#define BPATCH_FILE

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

#include "InstrucIter.h"

#include "LineInformation.h"

#include "BPatch_flowGraph.h"

class TarjanDominator;

const int BPatch_flowGraph::WHITE = 0;
const int BPatch_flowGraph::GRAY  = 1;
const int BPatch_flowGraph::BLACK = 2;

//constructor of the class. It creates the CFG and
//deletes the unreachable code.
BPatch_flowGraph::BPatch_flowGraph(BPatch_function *bpf, bool &valid)
	: bpFunction(bpf),loops(NULL),isDominatorInfoReady(false),
	  isSourceBlockInfoReady(false)
{
	//fill the information of the basic blocks after creating
	//them. The dominator information will also be filled
	valid = true;
	if (!createBasicBlocks()) {
	    valid = false;
	    return;
	}
	findAndDeleteUnreachable();
}

//contsructor of class. It finds the bpatch function which has the name
//given as a parameter and calls the other constructor using the bpatch 
//function value.
BPatch_flowGraph::BPatch_flowGraph(BPatch_image *bpim,char *functionName, bool &valid){

	BPatch_Vector<BPatch_function *> bpfv;

	if (NULL == bpim->findFunction(functionName, bpfv)) {
	  cerr << "ERROR : BPatch_flowGraph::BPatch_flowGraph : ";
	  cerr << functionName <<" does not have BPatch_function object\n" ;
	}

	if (bpfv.size() > 1) {
	  cerr << "WARNING:  BPatch_flowGraph found " << bpfv.size() << "functions called "
	       << functionName << " using the first" <<endl;
	}

	if (NULL != bpfv[0])
	  *(this) = *(new BPatch_flowGraph(bpfv[0], valid));
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
	if(bpFunction)
		bpFunction->cfg = NULL;
}

void BPatch_flowGraph::getAllBasicBlocks(BPatch_Set<BPatch_basicBlock*>& abb){
	BPatch_basicBlock** belements =
		new BPatch_basicBlock*[allBlocks.size()];
	allBlocks.elements(belements);
	for(int i=0;i<allBlocks.size();i++)
		abb += belements[i];
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
//functions as given in InstrucIter.h.
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
bool BPatch_flowGraph::createBasicBlocks(){

	int tbs = 0,i,j;

	Address effectiveAddress = (Address) (bpFunction->getBaseAddr());
	Address relativeAddress = (Address) (bpFunction->getBaseAddrRelative());
#if defined(i386_unknown_nt4_0) || defined(mips_unknown_ce2_11) //ccw 6 apr 2001
	long diffAddress = effectiveAddress;
#else
	long long diffAddress = effectiveAddress;
#endif
	diffAddress -= relativeAddress;

	//initializing the variables to use. Creating an address handle
	//a set of leaders and a map from leaders to the basic blocks.
	InstrucIter ah(bpFunction);

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
// 	instruction inst;


	for(;ah.hasMore();){
		//get the inctruction and the address
// 		inst = ah.getInstruction();
          InstrucIter inst(ah);
		Address pos = ah++;

		//if it is a conditional branch 
		if(inst.isACondBranchInstruction()){
			//if also it is inside the function space
			//then insert the target address as a leader
			//and create the basic block for the leader
			taddr = inst.getBranchTargetAddress(pos);
			if((baddr <= taddr) && (taddr < maddr) && 
			   !leaders.contains(taddr)) {
				leaders += taddr;
				leaderToBlock[taddr] =
				    new BPatch_basicBlock(this, tbs++);
				allBlocks += leaderToBlock[taddr];
			}

			if(InstrucIter::delayInstructionSupported())
			    if(!inst.isAnneal())
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
		else if(inst.isAJumpInstruction()) {
			//if it is unconditional jump then find the
			//target address and insert it as a leader and create
			//a basic block for it.
			taddr = inst.getBranchTargetAddress(pos);
			if((baddr <= taddr) && (taddr < maddr) && 
			   !leaders.contains(taddr)) {
				leaders += taddr;
				leaderToBlock[taddr] =
				    new BPatch_basicBlock(this, tbs++);
				allBlocks += leaderToBlock[taddr];
			}

			if(InstrucIter::delayInstructionSupported())
			     if(!inst.isAnneal())
				//if the dleay instruction is supported by the
				//architecture then skip one more instruction
				++ah;
#if defined(alpha_dec_osf4_0)
			taddr = *ah;
			if((taddr < maddr) && !leaders.contains(taddr)){
				leaders += taddr;
				leaderToBlock[taddr] =
				    new BPatch_basicBlock(this, tbs++);
				allBlocks += leaderToBlock[taddr];
			}
#endif
		}
#if defined(rs6000_ibm_aix4_1)
		else if(inst.isAIndirectJumpInstruction(InstrucIter(ah))){
#else
		else if(inst.isAIndirectJumpInstruction()){
#endif
			InstrucIter ah2(ah);
			BPatch_Set<Address> possTargets; 
#if defined(i386_unknown_linux2_0) ||\
    defined(i386_unknown_solaris2_5) ||\
    defined(i386_unknown_nt4_0)
			ah2.getMultipleJumpTargets(possTargets,ah);
#else
			ah2.getMultipleJumpTargets(possTargets);
#endif
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

			if(InstrucIter::delayInstructionSupported())
				//if the dleay instruction is supported by the
				//architecture then skip one more instruction
				++ah;
			
		}
#if defined(i386_unknown_linux2_0) ||\
    defined(i386_unknown_solaris2_5) ||\
    defined(i386_unknown_nt4_0) ||\
    defined(ia64_unknown_linux2_4) /* Temporary duplication - TLM */
		else if(inst.isAReturnInstruction()){
			if(InstrucIter::delayInstructionSupported())
				++ah;
			taddr = *ah;
			if((baddr <= taddr) && (taddr < maddr) && 
			   !leaders.contains(taddr)) {
				leaders += taddr;
				leaderToBlock[taddr] =
				    new BPatch_basicBlock(this, tbs++);
				allBlocks += leaderToBlock[taddr];
			}
		}
#endif
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
#if defined(i386_unknown_linux2_0) || defined(i386_unknown_nt4_0)
		// If the position of this leader does not match the start
		// of an instruction, then the leader must be invalid.
		// This can happen, for instance, when we parse a section
		// of data as if it were code, and some part of the data
		// looks like a branch.  If this happens, don't process
		// the leader (the associated BPatch_basicBlock will be
		// removed later when we remove unreachable blocks).
		if (!ah.isInstruction())
			continue;
#endif
		BPatch_basicBlock * bb = leaderToBlock[elements[i]];
		bb->startAddress = (Address)(elements[i]+diffAddress);

		//while the address handle has instructions to process
		while(ah.hasMore()){
			//get the current instruction
// 			inst = ah.getInstruction();
                  InstrucIter inst(ah);
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
			if(inst.isACondBranchInstruction()){
				taddr = inst.getBranchTargetAddress(pos);
				if((baddr <= taddr) && (taddr < maddr)){
					bb->targets += leaderToBlock[taddr];
					leaderToBlock[taddr]->sources += bb;
				} 
				else
					exitBlock += bb;

				if(InstrucIter::delayInstructionSupported())
				     if(!inst.isAnneal())
					//if the delay instruction is supported
					++ah;

				taddr = *ah;
				if(taddr < maddr){
					bb->targets += leaderToBlock[taddr];
					leaderToBlock[taddr]->sources += bb;
				}
			}
			else if(inst.isAJumpInstruction()){
				//if the branch is unconditional then only
				//find the target and leader and basic block 
				//coressponding to the leader. And update 
				//predecessor and successor fields of the 
				//basic blocks.
				taddr = inst.getBranchTargetAddress(pos);
				if((baddr <= taddr) && (taddr < maddr)){
					bb->targets += leaderToBlock[taddr];
					leaderToBlock[taddr]->sources += bb;
				}
				else 
					exitBlock += bb;

				if(InstrucIter::delayInstructionSupported())
				     if(!inst.isAnneal())
					//if the delay instruction is supported
					++ah;
			}
#if defined(rs6000_ibm_aix4_1)
			else if(inst.isAIndirectJumpInstruction(InstrucIter(ah))){
#else
			else if(inst.isAIndirectJumpInstruction()){
#endif
				InstrucIter ah2(ah);
				BPatch_Set<Address> possTargets; 
#if defined(i386_unknown_linux2_0) ||\
    defined(i386_unknown_solaris2_5) ||\
    defined(i386_unknown_nt4_0)
				ah2.getMultipleJumpTargets(possTargets,ah);
#else
				ah2.getMultipleJumpTargets(possTargets);
#endif
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

				if(InstrucIter::delayInstructionSupported())
					//if the dleay instruction is supported by the
					//architecture then skip one more instruction
					++ah;
			}
			else if(inst.isAReturnInstruction()){
				exitBlock += bb;
				bb->isExitBasicBlock = true;
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

	return true;
}

// This function must be called only after basic blocks have been created
// by calling createBasicBlocks. It computes the source block for each
// basic block. For now, a source block is represented by the starting
// and ending line numbers in the source block for the basic block.
void BPatch_flowGraph::createSourceBlocks(){

	bool lineInformationAnalyzed = false;

	if (isSourceBlockInfoReady)
		return;
	isSourceBlockInfoReady = true;

	BPatch_image* bpImage = bpFunction->mod->img;

	char functionName[1024];
	bpFunction->getMangledName(functionName, sizeof(functionName));
	string fName(functionName);
	unsigned int i,j,possibleFileIndex;

	//get the line information object which contains the information for 
	//this function

	LineInformation* lineInformation;
	FileLineInformation* fLineInformation = NULL; 
	FileLineInformation* possibleFiles[1024];

	BPatch_Vector<BPatch_module*>* appModules =  bpImage->getModules();
	for(i=0;i<appModules->size();i++){
		lineInformation = (*appModules)[i]->getLineInformation();
		if(!lineInformation)
			continue;

		if(!lineInformation->getFunctionLineInformation(fName,possibleFiles,1024))
			continue;

		for(possibleFileIndex=0;possibleFiles[possibleFileIndex];possibleFileIndex++){


			fLineInformation = possibleFiles[possibleFileIndex];

			lineInformationAnalyzed = true;

			const char* fileToBeProcessed = fLineInformation->getFileNamePtr();

			//now it is time to look the starting and ending line addresses
			//of the basic blocks in the control flow graph. To define
			//the line numbers we will use the beginAddress and endAddress
			//fields of the basic blocks in the control flow graph
			//and find the closest lines to these addresses.
			//get the address handle for the region

			// FIXME FIXME FIXME This address crap...
			InstrucIter ah(bpFunction, false); 
	
			//for every basic block in the control flow graph
	
			BPatch_basicBlock** elements = new BPatch_basicBlock*[allBlocks.size()];
			allBlocks.elements(elements);
			for(j=0;j<(unsigned)allBlocks.size();j++){
				BPatch_basicBlock *bb = elements[j];
	
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
			
				if(lSet.size() != 0){
					//create the source block for the above address set
					//and assign it to the basic block field

					if(!bb->sourceBlocks)
						bb->sourceBlocks = new BPatch_Vector<BPatch_sourceBlock*>();

					BPatch_sourceBlock* sb = new BPatch_sourceBlock(fileToBeProcessed,lSet);
					bb->sourceBlocks->push_back(sb);
				}
			}
			delete[] elements; 
		}
	}

	if(!lineInformationAnalyzed){
		cerr << "WARNING : Line information is missing >> Function : " ;
		cerr << fName  << "\n";
		return;
	}

}

/* class that calculates the dominators of a flow graph using
   tarjan's algorithms with results an almost linear complexity
   for graphs with less than 8 basic blocks */

class TarjanDominator {
private:
	int n,r;
	BPatch_basicBlock** numberToBlock;
	int *dom,*parent, *ancestor, *child, *vertex, *label, *semi,*size;
	BPatch_Set<int>** bucket;

	inline int bbToint(BPatch_basicBlock* bb){
		return bb->blockNumber + 1;
	}
	void dfs(int v,int* dfsNo){
		semi[v] = ++(*dfsNo);
		vertex[*dfsNo] = v;
		label[v] = v;
		ancestor[v] = 0;
		child[v] = 0;
		size[v] = 1;
		BPatch_basicBlock* bb = numberToBlock[v];
		BPatch_basicBlock** elements = new BPatch_basicBlock*[bb->targets.size()];
        	bb->targets.elements(elements);
        	for(int i=0;i<bb->targets.size();i++){
			int w = bbToint(elements[i]);
			if(semi[w] == 0){
				parent[w] = v;
				dfs(w,dfsNo);
			}
		}
		delete[] elements;
	}
	void COMPRESS(int v){
		if(ancestor[ancestor[v]] != 0){
			COMPRESS(ancestor[v]);
			if(semi[label[ancestor[v]]] < semi[label[v]])
				label[v] = label[ancestor[v]];
			ancestor[v] = ancestor[ancestor[v]];
		}
	}
	int EVAL(int v){
		if(ancestor[v] == 0)
			return label[v];
		COMPRESS(v);
		if(semi[label[ancestor[v]]] >= semi[label[v]])
			return label[v];
		return label[ancestor[v]];
	}
	void LINK(int v,int w){
		int s = w;
		while(semi[label[w]] < semi[label[child[s]]]){
			if((size[s]+size[child[child[s]]]) >= (2*size[child[s]])){
				ancestor[child[s]] = s;
				child[s] = child[child[s]];
			}
			else{
				size[child[s]] = size[s];
				ancestor[s] = child[s];
				s = child[s];
			}
		}
		label[s] = label[w];
		size[v] += size[w];
		if(size[v] < (2*size[w])){
			int tmp = s;
			child[v] = s;
			s = tmp;
		}
		while(s != 0){
			ancestor[s] = v;
			s = child[s];
		}
	}
public:
	TarjanDominator(int arg_size, BPatch_basicBlock* root,
                        BPatch_basicBlock** blocks) 
           : n(arg_size),r(bbToint(root)) 
	{
		int i;

		arg_size++;
		numberToBlock = new BPatch_basicBlock*[arg_size];
		numberToBlock[0] = NULL;
		for(i=0;i<n;i++)
			numberToBlock[bbToint(blocks[i])] = blocks[i];	

		dom = new int[arg_size];

		parent = new int[arg_size];
		ancestor = new int[arg_size];
		child = new int[arg_size];
		vertex = new int[arg_size];
		label = new int[arg_size];
		semi = new int[arg_size];
		this->size = new int[arg_size];
		bucket = new BPatch_Set<int>*[arg_size];

		for(i=0;i<arg_size;i++){
			bucket[i] = new BPatch_Set<int>;
			semi[i] = 0;	
		}
	}
	~TarjanDominator(){
		int i;
		delete[] numberToBlock;
		delete[] parent;
		delete[] ancestor;
		delete[] child;
		delete[] vertex;
		delete[] label;
		delete[] semi;
		delete[] size;
		delete[] dom;
		for(i=0;i<(n+1);i++)
			delete bucket[i];
		delete[] bucket;
	}
	void findDominators(){
		int i;
		int dfsNo = 0;
		dfs(r,&dfsNo);

		size[0] = 0;
		label[0] = 0;
		semi[0] = 0;

		for(i=n;i>1;i--){
			int w =  vertex[i];

			BPatch_basicBlock* bb = numberToBlock[w];
			BPatch_basicBlock** elements = new BPatch_basicBlock*[bb->sources.size()];
        		bb->sources.elements(elements);
        		for(int j=0;j<bb->sources.size();j++){
				int v = bbToint(elements[j]);
				int u = EVAL(v);
				if(semi[u] < semi[w])
					semi[w] = semi[u];
			}
			bucket[vertex[semi[w]]]->insert(w);
			LINK(parent[w],w);
			int v = 0;
			BPatch_Set<int>* bs = bucket[parent[w]];
			while(bs->extract(v)){
				int u = EVAL(v);
				dom[v] = ( semi[u] < semi[v] ) ? u : parent[w];
			}
		}
		for(i=2;i<=n;i++){
			int w = vertex[i];
			if(dom[w] != vertex[semi[w]])
				dom[w] = dom[dom[w]];
		}
		dom[r] = 0;

		for(i=1;i<=n;i++){
			int w = vertex[i];
			BPatch_basicBlock* bb = numberToBlock[w];
			bb->immediateDominator = numberToBlock[dom[w]];
			if(bb->immediateDominator){
				if(!bb->immediateDominator->immediateDominates)
					bb->immediateDominator->immediateDominates =
                                        	new BPatch_Set<BPatch_basicBlock*>;
                        	bb->immediateDominator->immediateDominates->insert(bb);
                	}
		}
	}
};

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
	BPatch_basicBlock** elements = NULL;

	if(isDominatorInfoReady)
		return;
	isDominatorInfoReady = true;

	/* if the number of basic blocks is greater than 8 then use
	   tarjan's fast dominator algorithm. Otherwise use the 
	   previous one */

	if(allBlocks.size() >= 8 ){
		elements = new BPatch_basicBlock*[allBlocks.size()];
		allBlocks.elements(elements);
		TarjanDominator tarjanDominator(allBlocks.size(),
						entryBlock.minimum(),
						elements);
		delete[] elements;
		tarjanDominator.findDominators();

		return;
	}

	/* end of the tarjan dominator claculation  if used */

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
	elements = new BPatch_basicBlock*[entryBlock.size()];
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

typedef struct SortTuple{
	Address address;
	BPatch_basicBlock* bb;
}SortTuple;

extern "C" int tupleSort(const void* arg1,const void* arg2){
   if(((const SortTuple*)arg1)->address > ((const SortTuple*)arg2)->address)
      return 1;

   if(((const SortTuple*)arg1)->address < ((const SortTuple*)arg2)->address)
      return -1;

   return 0;
}

void BPatch_flowGraph::findAndDeleteUnreachable()
{

	int i,j;

	int* bbToColor = new int[allBlocks.size()];
	for(i=0;i<allBlocks.size();i++)
		bbToColor[i] = WHITE;

	//a dfs based back edge discovery
	BPatch_basicBlock** elements = 
			new BPatch_basicBlock*[entryBlock.size()];
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
	for(i=0;i<toDelete.size();i++)
		delete elements[i];
	delete[] elements;
	delete[] bbToColor;

	int orderArraySize = allBlocks.size();
	SortTuple* orderArray = new SortTuple[orderArraySize];
	elements = new BPatch_basicBlock*[allBlocks.size()];
	allBlocks.elements(elements);
	for(i=0;i<orderArraySize;i++){
		orderArray[i].bb = elements[i];
		orderArray[i].address = elements[i]->startAddress;
        }
        qsort((void*)orderArray, orderArraySize, sizeof(SortTuple), tupleSort);
        for(i=0;i<orderArraySize;i++)
                orderArray[i].bb->setBlockNumber(i);
        delete[] orderArray;
        delete[] elements;
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
	~STACK() { free(data); }
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
