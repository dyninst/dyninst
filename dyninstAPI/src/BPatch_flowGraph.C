#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

#include "common/h/Types.h"
#include "common/h/Vector.h"
#include "common/h/Dictionary.h"

#include "util.h"
#include "process.h"
#include "symtab.h"
#include "instPoint.h"

#if defined(sparc_sun_solaris2_4) || defined(mips_sgi_irix6_4)
#include "AddressHandle.h"
#endif

#include "LineInformation.h"

#include "BPatch_flowGraph.h"

const int BPatch_flowGraph::WHITE = 0;
const int BPatch_flowGraph::GRAY  = 1;
const int BPatch_flowGraph::BLACK = 2;

//constructor of the class. It creates the CFG and
//deletes the unreachable code.
BPatch_flowGraph::BPatch_flowGraph(BPatch_function *bpf)
	: bpFunction(bpf),loops(NULL),isDominatorInfoReady(false),
	  isSourceBlockInfoReady(false)
{
	//fill the information of the basic blocks after creating
	//them. The dominator information will also be filled
	createBasicBlocks();
	findAndDeleteUnreachable();
}

//contsructor of class. It finds the bpatch function which has the name
//given as a parameter and calls the other constructor using the bpatch 
//function value.
BPatch_flowGraph::BPatch_flowGraph(BPatch_image *bpim,char *functionName){
	BPatch_function *bpf = NULL;
	bpf = bpim->findBPFunction(functionName);
	if(bpf)
		*(this) = *(new BPatch_flowGraph(bpf));
	else{
		cerr << "ERROR : BPatch_flowGraph::BPatch_flowGraph : ";
		cerr << functionName <<" does not have BPatch_function object\n" ;
	}
}

//destructor for the class 
BPatch_flowGraph::~BPatch_flowGraph(){
	int i;
	if(loops){
		BPatch_basicBlockLoop** lelements = 
			new BPatch_basicBlockLoop*[loops->size()];
		loops->elements(lelements);
		for(i=0;i<loops->size();i++)
			delete lelements[i];
		delete[] lelements;
		delete loops;
	}

	BPatch_basicBlock** belements =new BPatch_basicBlock*[allBlocks.size()];
	allBlocks.elements(belements);
	for(i=0;i<allBlocks.size();i++)
		delete belements[i];
	delete[] belements;
}

//this is the method that returns the set of entry points
//basic blocks, to the control flow graph. Actually, there must be
//only one entry point to each control flow graph but the definition
//given API specifications say there might be more.
void BPatch_flowGraph::getEntryBasicBlock(BPatch_Vector<BPatch_basicBlock*>& ebb){
	
	BPatch_basicBlock** belements =new BPatch_basicBlock*[entryBlock.size()];
	entryBlock.elements(belements);
	for(int i=0;i<entryBlock.size();i++)
		ebb.push_back(belements[i]);
	delete[] belements;
}

//this method returns the set of basic blocks that are the
//exit basic blocks from the control flow graph. That is those
//are the basic blocks that contains the instruction for
//returning from the function
void BPatch_flowGraph::getExitBasicBlock(BPatch_Vector<BPatch_basicBlock*>& nbb){

	BPatch_basicBlock** belements =new BPatch_basicBlock*[exitBlock.size()];
	exitBlock.elements(belements);
	for(int i=0;i<exitBlock.size();i++)
		nbb.push_back(belements[i]);
	delete[] belements;
}

//this methods returns the loop objects that exist in the control flow
//grap. It retuns a set. And if ther is no loop then it returns empty
//set. not NULL. 
void BPatch_flowGraph::getLoops(BPatch_Vector<BPatch_basicBlockLoop*>& lbb){
	int i;

	if(!loops){
		//creating the loops field which was NULL initially
		loops = new BPatch_Set<BPatch_basicBlockLoop*>;

		//filling the dominator info since to find the loops
		//structure we need the dominator info
		fillDominatorInfo();

		//create an array of all blocks size to store the mapping from
		//basic block to set of basic blocks as back edges
		BPatch_Set<BPatch_basicBlock*>** backEdges = 
			new BPatch_Set<BPatch_basicBlock*>*[allBlocks.size()];
		for(i=0;i<allBlocks.size();i++)
			backEdges[i] = NULL;

		//using dfs we find the back edeges which define the
		//natural loop
		findBackEdges(backEdges);

		//a map from basic block number to basic block pointer
		//which will be used to get the basic block pointer
		//from its number from the map. I am using this way
		//as I do not want to include dictionary_hash in include files
		//or use other class(empty) to get around this problem.
		//this does not give any drawback for efficency or space.

		BPatch_basicBlock** bnoToBBptr =
				new BPatch_basicBlock*[allBlocks.size()];

		BPatch_basicBlock** elements =
			new BPatch_basicBlock*[allBlocks.size()];
		allBlocks.elements(elements);
		for(i=0;i<allBlocks.size();i++)
			bnoToBBptr[elements[i]->blockNumber] = elements[i];
		delete[] elements;

		//now using the map find the basic blocks inside the loops
		fillLoopInfo(backEdges,bnoToBBptr);

		//delete the temporary storages since it is done.
		for(i=0;i<allBlocks.size();i++)
			delete backEdges[i];
		delete[] backEdges;
		delete[] bnoToBBptr;
	}

	BPatch_basicBlockLoop** lelements = 
		new BPatch_basicBlockLoop*[loops->size()];
	loops->elements(lelements);
	for(i=0;i<loops->size();i++)
		lbb.push_back(lelements[i]);
	delete[] lelements;
}

//this is the main method to create the basic blocks and the
//the edges between basic blocks. The assumption of the
//method is as follows: It assumes existence of four machine dependent
//functions as given in AddressHandle.h.
//after finding the leaders, for each leader a basic block is created and
//then the predecessors and successors of the basic blocks are inserted
//to the basic blocks by passing from the function address space again, one
//instruction at a time, and using maps from basic blocks to leaders, and
//leaders to basic blocks. 
//The basic block of which the
//leader is the start address of the function is assumed to be the entry block
//to control flow graph. This makes only one basic block to be in the
//entryBlocks field of the controlflow grpah. If it is possible
//to enter a function from many points some modification is needed
//to insert all entry basic blocks to the relevant field of the class.
void BPatch_flowGraph::createBasicBlocks(){

#if defined(sparc_sun_solaris2_4) || defined(mips_sgi_irix6_4)
	int tbs = 0,i,j;

	Address effectiveAddress = (Address) (bpFunction->getBaseAddr());
	Address relativeAddress = (Address) (bpFunction->getBaseAddrRelative());
	long long diffAddress = effectiveAddress;
	diffAddress -= relativeAddress;

	char functionName[100];
        bpFunction->getName(functionName,99);functionName[99]='\0';

	//initializing the variables to use. Creating an address handle
	//a set of leaders and a map from leaders to the basic blocks.

	AddressHandle ah(bpFunction->proc->getImage(),relativeAddress,
			 bpFunction->getSize());

	Address baddr = relativeAddress;
	Address maddr = relativeAddress + bpFunction->getSize();
	Address taddr;

	BPatch_Set<Address> leaders;
	dictionary_hash<Address,BPatch_basicBlock*> leaderToBlock(addrHash);
	
	//start inserting the leader information. The initial address of the
	//function is inserted as a leader and a basic block is created for it
	//and inserted into the map.

	leaders += relativeAddress;
	leaderToBlock[relativeAddress] = new BPatch_basicBlock(this, tbs++);
	allBlocks += leaderToBlock[relativeAddress];

	//while there are still instructions to check for in the
	//address space of the function
	instruction inst;


	for(;ah.hasMore();){

		//get the inctruction and the address
		inst = ah.getInstruction();
		Address pos = ah++;

		//if it is a conditional branch 
		if(isLocalCondBranch(inst)){

			//if also it is inside the function space
			//then insert the target address as a leader
			//and create the basic block for the leader
			taddr = getBranchTargetAddress(inst,pos);
			if((baddr <= taddr) && (taddr < maddr) && 
			   !leaders.contains(taddr)) {
				leaders += taddr;
				leaderToBlock[taddr] =
				    new BPatch_basicBlock(this, tbs++);
				allBlocks += leaderToBlock[taddr];
			}

			if(AddressHandle::delayInstructionSupported())
				//if the dleay instruction is supported by the
				//architecture then skip one more instruction
				++ah;

			//if the next address is still in the address
			//space of the function then it is also a leader
			//since the condition may not be met
			taddr = *ah;
			if((taddr < maddr) && !leaders.contains(taddr)){
				leaders += taddr;
				leaderToBlock[taddr] =
				    new BPatch_basicBlock(this, tbs++);
				allBlocks += leaderToBlock[taddr];
			}
		}
		else if(isLocalJump(inst)) {
			//if it is unconditional jump then find the
			//target address and insert it as a leader and create
			//a basic block for it.
			taddr = getBranchTargetAddress(inst,pos);
			if((baddr <= taddr) && (taddr < maddr) && 
			   !leaders.contains(taddr)) {
				leaders += taddr;
				leaderToBlock[taddr] =
				    new BPatch_basicBlock(this, tbs++);
				allBlocks += leaderToBlock[taddr];
			}

			if(AddressHandle::delayInstructionSupported())
				//if the dleay instruction is supported by the
				//architecture then skip one more instruction
				++ah;
		}
		else if(isLocalIndirectJump(inst)){
			//cerr << "********* INDIRECT ***********\n";
			AddressHandle ah2(ah);
			BPatch_Set<Address> possTargets; 
			ah2.getMultipleJumpTargets(possTargets);
			Address* telements = new Address[possTargets.size()];
			possTargets.elements(telements);
			for(i=0;i<possTargets.size();i++){
				taddr = telements[i];
				if((baddr <= taddr) && (taddr < maddr) &&
				   !leaders.contains(taddr)) {
					leaders += taddr;
					leaderToBlock[taddr] =
					    new BPatch_basicBlock(this, tbs++);
					allBlocks += leaderToBlock[taddr];
				}
			}
			delete[] telements;

			if(AddressHandle::delayInstructionSupported())
				//if the dleay instruction is supported by the
				//architecture then skip one more instruction
				++ah;
			
		}
	}


	//to process the leaders easily sort the leaders according to their
	//values, that is according to the address value they have
	Address* elements = new Address[leaders.size()];
	leaders.elements(elements);

	//insert the first leaders corresponding basic block as a entry
	//block to the control flow graph.

	leaderToBlock[elements[0]]->isEntryBasicBlock = true;
	entryBlock += leaderToBlock[elements[0]];

	//for each leader take the address value and continue procesing
	//the instruction till the next leader or the end of the
	//function space is seen
	for(i=0;i<leaders.size();i++){
		//set the value of address handle to be the value of the leader
		ah.setCurrentAddress(elements[i]);
		BPatch_basicBlock * bb = leaderToBlock[elements[i]];
		bb->startAddress = (Address)(elements[i]+diffAddress);

		//while the address handle has instructions to process
		while(ah.hasMore()){
			//get the current instruction
			inst = ah.getInstruction();
			Address pos = *ah;

			//if the next leaders instruction is seen and it is not
			//the end of the function yet, find out whether
			//the successors of the basic block already has
			//some information. If not then it has no branch
			//instruction as a last instruction. So the next
			//leaders basic block must be in the successor list
			//and break processing for the current leader
			if((i < (leaders.size()-1)) && (pos == elements[i+1])){
				bb->endAddress = (Address)(ah.prevAddress() + diffAddress);
				if(bb->targets.size() == 0){
					bb->targets += leaderToBlock[pos];
					leaderToBlock[pos]->sources += bb;	
				}
				break;
			}

			//increment the address handle;
			ah++;

			//if the instruction is conditional branch then
			//find the target address and find the corresponding 
			//leader and basic block for it. Then insert the found
			//basic block to the successor list of current leader's
			//basic block, and insert current basic block to the
			//predecessor field of the other one. Do the
			//same thing for the following ( or the other one
			//if delay instruction is supported) as a leader.
			if(isLocalCondBranch(inst)){
				taddr = getBranchTargetAddress(inst,pos);
				if((baddr <= taddr) && (taddr < maddr)){
					bb->targets += leaderToBlock[taddr];
					leaderToBlock[taddr]->sources += bb;
				} 
				else
					exitBlock += bb;

				if(AddressHandle::delayInstructionSupported())
					//if the delay instruction is supported
					++ah;

				taddr = *ah;
				if(taddr < maddr){
					bb->targets += leaderToBlock[taddr];
					leaderToBlock[taddr]->sources += bb;
				}
			}
			else if(isLocalJump(inst)){
				//if the branch is unconditional then only
				//find the target and leader and basic block 
				//coressponding to the leader. And update 
				//predecessor and successor fields of the 
				//basic blocks.
				taddr = getBranchTargetAddress(inst,pos);
				if((baddr <= taddr) && (taddr < maddr)){
					bb->targets += leaderToBlock[taddr];
					leaderToBlock[taddr]->sources += bb;
				}
				else 
					exitBlock += bb;

				if(AddressHandle::delayInstructionSupported())
					//if the delay instruction is supported
					++ah;
			}
			else if(isReturn(inst)){
				exitBlock += bb;
				bb->isExitBasicBlock = true;
			}
			else if(isLocalIndirectJump(inst)){
				AddressHandle ah2(ah);
				BPatch_Set<Address> possTargets; 
				ah2.getMultipleJumpTargets(possTargets);
				Address* telements = new Address[possTargets.size()];
				possTargets.elements(telements);
				for(j=0;j<possTargets.size();j++){
					taddr = telements[j];
					if((baddr <= taddr) && (taddr < maddr)){
						bb->targets += leaderToBlock[taddr];
						leaderToBlock[taddr]->sources += bb;
					}
					else
						exitBlock += bb;
				}
				delete[] telements;

				if(AddressHandle::delayInstructionSupported())
					//if the dleay instruction is supported by the
					//architecture then skip one more instruction
					++ah;
			}
		}
		//if the while loop terminated due to recahing the
		//end of the address space of the function then set the
		//end addresss of the basic block to the last instruction's
		//address in the address space.
		if(i == (leaders.size()-1))
			bb->endAddress = (Address)(ah.prevAddress()+diffAddress);

	}
	delete[] elements;
#endif

}

// This function must be called only after basic blocks have been created
// by calling createBasicBlocks. It computes the source block for each
// basic block. For now, a source block is represented by the starting
// and ending line numbers in the source block for the basic block.
void BPatch_flowGraph::createSourceBlocks(){

#if defined(sparc_sun_solaris2_4) || defined(mips_sgi_irix6_4)
	if (isSourceBlockInfoReady)
		return;
	isSourceBlockInfoReady = true;

	BPatch_image* bpImage = bpFunction->mod->img;

	char functionName[100];
	bpFunction->getName(functionName,99);functionName[99]='\0';
	string fName(functionName);
	int i;

	//get the line information object which contains the information for 
	//this function

	LineInformation* lineInformation;
	FileLineInformation* fLineInformation = NULL; 

	BPatch_Vector<BPatch_module*>* appModules =  bpImage->getModules();
	for(i=0;i<appModules->size();i++){
		lineInformation = (*appModules)[i]->lineInformation;
		if(!lineInformation)
			continue;
		fLineInformation = lineInformation->getFunctionLineInformation(fName);
		if(fLineInformation)
			break;
	}

	if(!fLineInformation){
		cerr << "WARNING : Line information is missing >> Function : " ;
		cerr << functionName  << "\n";
		return;
	}

	Address effectiveAddress = (Address) (bpFunction->getBaseAddr());


	//now it is time to look the starting and ending line addresses
	//of the basic blocks in the control flow graph. To define
	//the line numbers we will use the beginAddress and endAddress
	//fields of the basic blocks in the control flow graph
	//and find the closest lines to these addresses.

	//get the address handle for the region
	AddressHandle ah(bpFunction->proc->getImage(),effectiveAddress,
			 bpFunction->getSize()); 

	//for every basic block in the control flow graph

	BPatch_basicBlock** elements = new BPatch_basicBlock*[allBlocks.size()];
	allBlocks.elements(elements);
	for(i=0;i<allBlocks.size();i++){
		BPatch_basicBlock *bb = elements[i];

		//set the address handle to the start address
		ah.setCurrentAddress(bb->startAddress);

		//create the set of unsigned integers for line numbers
		//in the basic block
		BPatch_Set<unsigned short> lSet;

		Address cAddr;
		//while the address is valid  go backwards and find the
		//entry in the mapping from address to line number for closest
		//if the address is coming after a line number information
		while(ah.hasMore()){
			cAddr = ah--;
			if(fLineInformation->getLineFromAddr(fName,lSet,cAddr))
				break;
		}

		//set the address handle to the start address
		ah.setCurrentAddress(bb->startAddress);

		//while the address is valid go forward and find the entry
		//in the mapping from address to line number for closest
		while(ah.hasMore()){
			cAddr = ah++;
			if(cAddr > bb->endAddress) 
				break;
			fLineInformation->getLineFromAddr(fName,lSet,cAddr);
		}
		
		//create the source block for the above address set
		//and assign it to the basic block field
		bb->sourceBlock = new BPatch_sourceBlock(lSet);
	}
	delete[] elements; 
#endif

}


//this method fill the dominator information of each basic block
//looking at the control flow edges. It uses a fixed point calculation
//to find the immediate dominator of the basic blocks and the set of
//basic blocks that are immediately dominated by this one.
//Before calling this method all the dominator information
//is going to give incorrect results. So first this function must
//be called to process dominator related fields and methods.
void BPatch_flowGraph::fillDominatorInfo(){

	int i,j,k;
	BPatch_basicBlock* bb;
	BPatch_Set<BPatch_basicBlock*>* domSet;

	if(isDominatorInfoReady)
		return;
	isDominatorInfoReady = true;

        BPatch_Set<BPatch_basicBlock*>** blockToDominator = 
			new BPatch_Set<BPatch_basicBlock*>*[allBlocks.size()];

	//for each basic block set the set of dominators to 
	//all basic blocks in the control flow. For the entry basic blocks
	//the set contains only the basic block itself initially, and through
	//the algorithm since they can not be dominated by any other basic 
	//block.

	int tmp = 0;
	BPatch_basicBlock** belements = 
		new BPatch_basicBlock*[allBlocks.size()];
	int* bbToColor = new int[allBlocks.size()];
	for(i=0;i<allBlocks.size();i++){
		bbToColor[i] = WHITE;
		belements[i] = NULL;
	}

	//a dfs order of basic blocks
	BPatch_basicBlock** elements = new BPatch_basicBlock*[entryBlock.size()];
	entryBlock.elements(elements);
	for(i=0;i<entryBlock.size();i++)
		if(bbToColor[elements[i]->blockNumber] == WHITE)
			dfsVisit(elements[i],bbToColor,NULL,&tmp,belements);
	delete[] elements;
	delete[] bbToColor;

	for(i=0;i<allBlocks.size();i++){
		bb = belements[i];
		domSet = new BPatch_Set<BPatch_basicBlock*>;
		if(entryBlock.contains(bb))
			//if enntry basic block then insert itself
			domSet->insert(bb);
		else 
			*domSet |= allBlocks;

		blockToDominator[bb->blockNumber] = domSet; 
	}

	bool done = true ;
	//fix-point calculation start here. When the fix point is reached for 
	//the set then done will be true and no more iterations will be done
	do{
		done = true;
		for(i=0;i<allBlocks.size();i++){
			bb = belements[i];

			//the processing is done for basic blocks that are not
			//entry points to the control flow graph. Since
			//entry basic blocks can not be dominated by other 
			//basic blocks
			if(!entryBlock.contains(bb))
			{
				BPatch_Set<BPatch_basicBlock*>* oldSet = 
					blockToDominator[bb->blockNumber];
				BPatch_Set<BPatch_basicBlock*>* newSet = 
					new BPatch_Set<BPatch_basicBlock*>;

				//initialize the new set
				if(bb->sources.size() != 0)
					*newSet |= allBlocks;
				assert(bb->sources.size());

				//intersect the dominators of the predecessors
				//since a block is dominated by the common 
				//blocks that dominate each predecessor of the
				//basic block.

				BPatch_basicBlock** selements = 
					new BPatch_basicBlock*[bb->sources.size()];
				bb->sources.elements(selements); 
				for(int j=0;j<bb->sources.size();j++)
					*newSet &= *blockToDominator[selements[j]->blockNumber];
				delete[] selements;

				//add the basic block itself. Since every block
				//is dominated by itself.
				*newSet += bb;
				blockToDominator[bb->blockNumber] = newSet;
	
				//if the previous set for basic block is the 
				//same with the new one generated then this
				//basic blocks dominator set reached the fix 
				//point. But if at least one basic block did 
				//not reach the fix point the iteration will
				//be done once more.
				if(*oldSet != *newSet)
					done = false;
				delete oldSet;
			}
		}
	}while(!done);

	//since the output of the previous set is the set of basic blocks
	//that dominate a basic block we have to use this information
	//and change it to the set of basic blocks that the basic block
	//dominates and using this information we have to fill the
	//immediate dominates field of each basic block. 

	//at this pointdominator tree construction is easy since
	//all the necessary information is ready. Finding immediate
	//dominators is enough for many purposes. To find the
	//immediate dominator of each block the dominators of basic 
	//blocks will be used. 

	//initially we initialize a mapping from a basic block to a basic block set
	//to keep track of the immediate dominators. We initialize sets corresponding
	//to a basic block with its dominator set except itself

	for(i=0;i<allBlocks.size();i++)
		blockToDominator[belements[i]->blockNumber]->remove(belements[i]);

	//then for each node we eliminate the unnecessary basic blocks to reach 
	//the immediate dominator. What we do it we look at the dominators of 
	//the basic block. Then for each dominator we take an element and iterate 
	//on the remaining ones. If the dominatos of the 
	//chosen elemnt contains one of the others then we delete the latter one 
	//from the set since it can not be immediate dominator. At the end of 
	//the loops the each set will contain immediate dominator

	for(i=0;i<allBlocks.size();i++)
		if(!entryBlock.contains(belements[i])){
			BPatch_basicBlock *currBlock = belements[i];

			BPatch_Set<BPatch_basicBlock*> ts1(*blockToDominator[currBlock->blockNumber]);
			BPatch_basicBlock** it1 = new BPatch_basicBlock*[ts1.size()];
			ts1.elements(it1);

			for(j=0;j<ts1.size();j++){
				BPatch_basicBlock *bb1 = it1[j];
				BPatch_Set<BPatch_basicBlock*> ts2(*blockToDominator[currBlock->blockNumber]);
				ts2.remove(bb1);
				BPatch_basicBlock** it2 = new BPatch_basicBlock*[ts2.size()];
				ts2.elements(it2);
				for(k=0;k<ts2.size();k++){
					BPatch_basicBlock *bb2 = it2[k];
					if(blockToDominator[bb1->blockNumber]->contains(bb2))
						blockToDominator[currBlock->blockNumber]->remove(bb2);
				}
				delete[] it2;
			}

			delete[] it1;
		}

	//using the previous imformation we will initialize the immediateDominator field 
	//of each basic blocks in control flow graph
	//if for a basic block another basic blocks dominator set
	//contains the former one, then we insert the latter to the
	//dominates field of the first one.


	for(i=0;i<allBlocks.size();i++){	
		bb = belements[i];
		if(entryBlock.contains(bb) ||
		  !blockToDominator[bb->blockNumber]->extract(bb->immediateDominator))
			bb->immediateDominator = NULL;
		assert(!blockToDominator[bb->blockNumber]->size());
		delete blockToDominator[bb->blockNumber];
		if(!entryBlock.contains(bb)){
			BPatch_basicBlock* dominator = bb->immediateDominator;
			domSet = dominator->immediateDominates;
			if(!domSet){
				domSet = new BPatch_Set<BPatch_basicBlock*>;
				dominator->immediateDominates = domSet;
			}
			domSet->insert(bb);
		}
	}
	delete[] blockToDominator;
	delete[] belements;
}



//method that finds the unreachable basic blocks and then deletes
//the structures allocated for that block. If the argument is 
//NULL then it deletes unreachable code. 
void BPatch_flowGraph::findBackEdges(BPatch_Set<BPatch_basicBlock*>** backEdges)
{
	int i;
	int* bbToColor = new int[allBlocks.size()];
	for(i=0;i<allBlocks.size();i++)
		bbToColor[i] = WHITE;
	
	//a dfs based back edge discovery
	BPatch_basicBlock** elements = 
		new BPatch_basicBlock*[entryBlock.size()];
	entryBlock.elements(elements);
	for(i=0;i<entryBlock.size();i++)
		if(bbToColor[elements[i]->blockNumber] == WHITE)
			dfsVisit(elements[i],bbToColor,backEdges);
	delete[] elements;
	delete[] bbToColor;
}

//method that implements the depth first visit operation
void BPatch_flowGraph::dfsVisit(BPatch_basicBlock* bb,
			        int* bbToColor,
				BPatch_Set<BPatch_basicBlock*>** backEdges,
				int* which,
				BPatch_basicBlock** order)
{
	if(order) order[(*which)++] = bb;	
	bbToColor[bb->blockNumber] = GRAY;
	BPatch_basicBlock** elements =  
			new BPatch_basicBlock*[bb->targets.size()];
	bb->targets.elements(elements);
	for(int i=0;i<bb->targets.size();i++)
		if(bbToColor[elements[i]->blockNumber] == WHITE)
			dfsVisit(elements[i],bbToColor,backEdges,which,order);
		else if(backEdges && 
			(bbToColor[elements[i]->blockNumber] == GRAY))
		{
			BPatch_Set<BPatch_basicBlock*>* newSet = backEdges[bb->blockNumber];
			if(!newSet){
				newSet = new BPatch_Set<BPatch_basicBlock*>;
				backEdges[bb->blockNumber] = newSet;
			}
			newSet->insert(elements[i]);
		}
	bbToColor[bb->blockNumber] = BLACK;
	delete[] elements;
}
void BPatch_flowGraph::findAndDeleteUnreachable()
{

	int i,j;

	int* bbToColor = new int[allBlocks.size()];
	for(i=0;i<allBlocks.size();i++)
		bbToColor[i] = WHITE;

	//a dfs based back edge discovery
	BPatch_basicBlock** elements = new BPatch_basicBlock*[entryBlock.size()];
	entryBlock.elements(elements);
	for(i=0;i<entryBlock.size();i++)
		if(bbToColor[elements[i]->blockNumber] == WHITE)
			dfsVisit(elements[i],bbToColor,NULL);

	delete[] elements;

	BPatch_Set<BPatch_basicBlock*> toDelete;
	elements =  new BPatch_basicBlock*[allBlocks.size()];
	allBlocks.elements(elements);
	int oldCount = allBlocks.size();
	for(i=0;i<oldCount;i++){
		BPatch_basicBlock* bb = elements[i];
		if(bbToColor[bb->blockNumber] == WHITE){

			BPatch_basicBlock** selements = 
				new BPatch_basicBlock*[bb->sources.size()];
			bb->sources.elements(selements);
			int count = bb->sources.size();
			for(j=0;j<count;j++)
				selements[j]->targets.remove(bb);
			delete[] selements;

			selements = new BPatch_basicBlock*[bb->targets.size()];
			bb->targets.elements(selements);
			count = bb->targets.size();
			for(j=0;j<count;j++)
				selements[j]->sources.remove(bb);
			delete[] selements;

			allBlocks.remove(bb);
			exitBlock.remove(bb);
			toDelete += bb;
		}
	}
	delete[] elements;
	elements = new BPatch_basicBlock*[toDelete.size()];
	toDelete.elements(elements);	
	for(i=0;i<toDelete.size();i++){
		delete elements[i];
	}
	delete[] elements;
	elements =  new BPatch_basicBlock*[allBlocks.size()];
	allBlocks.elements(elements);
	for(i=0;i<allBlocks.size();i++)
		elements[i]->setBlockNumber(i);
	delete[] elements;
	delete[] bbToColor;
}


//this method is used find the basic blocks contained by the loop
//defined by a backedge. The tail of the backedge is the starting point and
//the predecessors of the tail is inserted as a member of the loop.
//then the predecessors of the newly inserted blocks are also inserted
//until the head of the backedge is in the set(reached).

void BPatch_flowGraph::findBBForBackEdge(
			BPatch_basicBlock* from,
			BPatch_basicBlock* to,
		        BPatch_Set<BPatch_basicBlock*>& bbSet)
{
typedef struct STACK {
	unsigned size;
	int top;
	BPatch_basicBlock** data;

	STACK() : size(0),top(-1),data(NULL) {}
	~STACK() { delete[] data; }
	bool empty() { return (top < 0); }
	void push(BPatch_basicBlock* b) {
		if(!size) 
		    data = (BPatch_basicBlock**) malloc(
				sizeof(BPatch_basicBlock*)*(++size));
		else if(top == ((int)size-1))
		    data = (BPatch_basicBlock**)realloc(
				data,sizeof(BPatch_basicBlock*)*(++size));
		top++;
		data[top] = b;
	}
	BPatch_basicBlock* pop() {
		if(empty()) return NULL;
		return data[top--];
	}
} STACK;

	STACK* stack = new STACK;

	bbSet += to;
	if(!bbSet.contains(from)){
		bbSet += from;
		stack->push(from);
	}
	while(!stack->empty()){
		BPatch_basicBlock* bb = stack->pop();
		BPatch_basicBlock** elements = 
			new BPatch_basicBlock*[bb->sources.size()];
		bb->sources.elements(elements);
		for(int i=0;i<bb->sources.size();i++)
			if(!bbSet.contains(elements[i])){
				bbSet += elements[i];
				stack->push(elements[i]);
			}
		delete[] elements;
	}
	delete stack;
}

//this method find all loops in the control flow graph.
//The loops are defined by backedges and the backedge defines
//a loop if the head of the backedge dominates the tail of the backedge.
//Then after finding all loops then the basic blocks in the loops
//are found and the nesting structure of the loops are found and the
//relevant fields of the loops are filled by the information discovered.
void BPatch_flowGraph::fillLoopInfo(BPatch_Set<BPatch_basicBlock*>** backEdges,
				    BPatch_basicBlock** bToP)
{
	//for each back edge find the dominator relationship and if the
	//head basic block dominates the tail then find the loop basic blocks

	for(int bNo=0;bNo<allBlocks.size();bNo++){
		if(!backEdges[bNo])
			continue;

		BPatch_Set<BPatch_basicBlock*>* toBlocks = backEdges[bNo];
		BPatch_basicBlock* bbFrom = bToP[bNo];
		BPatch_basicBlock** elements = 
			new BPatch_basicBlock*[toBlocks->size()];
		toBlocks->elements(elements);
		for(int i=0;i<toBlocks->size();i++){
			BPatch_basicBlock* bbTo = elements[i];
			if(bbTo->dominates(bbFrom)){
				//then check whether
				//it dominates the current
				//basic block. If so then
				//it defines a loop. Otherwise
				//there is no regular loop
				//create the loop
				BPatch_basicBlockLoop* l = 
					new BPatch_basicBlockLoop(bbTo);

				//initialize some fields of the
				//loop object
				l->backEdges += bbFrom;
				(*loops) += l;

				//find all basic blocks in the
				//loop and keep a map used
				//to find the nest structure 
				findBBForBackEdge(bbFrom,bbTo,l->basicBlocks);
			}
		}
		delete[] elements;
	}

	//create two iterators on the elements of the map which is from
	//loop object to the set of basic blocks
	//for each loop object pair check whether one is subset of the
	//other. That is the set of basic blocks in the loop. If one is
	//subset of the other one then the smaller one is nested in the
	//bigger one so insert the smaller one to the containedLoops 
	//field of the loop object.

	BPatch_basicBlockLoop** it = 
			new BPatch_basicBlockLoop*[loops->size()];
	loops->elements(it);
	for(int i=0;i<loops->size();i++)
		for(int j=0;j<loops->size();j++)
			if(i != j){
				BPatch_basicBlockLoop* l1 = it[i];
				BPatch_basicBlockLoop* l2 = it[j];
				BPatch_Set<BPatch_basicBlock*> diff(l1->basicBlocks);
				diff -= l2->basicBlocks;
				if(diff.empty())
					l2->containedLoops += l1;
			}
	delete[] it;
}

//print method
ostream& operator<<(ostream& os,BPatch_flowGraph& fg){
	int i;
	os << "Begin CFG :\n";
	BPatch_basicBlock** belements = 
		new BPatch_basicBlock*[fg.allBlocks.size()];
	fg.allBlocks.elements(belements);
	for(i=0;i<fg.allBlocks.size();i++)
		os << *belements[i];
	delete[] belements;

	if(fg.loops){
		BPatch_basicBlockLoop** lelements = 
			new BPatch_basicBlockLoop*[fg.loops->size()];
		fg.loops->elements(lelements);	
		for(i=0;i<fg.loops->size();i++)
			os << *lelements[i];
		delete[] lelements;
	}
	os << "End CFG :\n";
	return os;
}
