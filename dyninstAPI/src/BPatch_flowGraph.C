#define BPATCH_FILE

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

#include "common/h/Types.h"
#include "common/h/Vector.h"
#include "common/h/Dictionary.h"
#include "common/h/Pair.h"
#include "common/h/String.h"

#include "util.h"
#include "process.h"
#include "symtab.h"
#include "instPoint.h"

#include "InstrucIter.h"

#include "LineInformation.h"

#include "BPatch_flowGraph.h"


BPatch_loopTreeNode::BPatch_loopTreeNode(BPatch_basicBlockLoop *l, 
					 const char *n) {
    loop = l;
    hierarchicalName = NULL;
    if (n != NULL) {
	hierarchicalName = new char[strlen(n)+1]; 
	strcpy(hierarchicalName, n);
    }
}
 

const char * 
BPatch_loopTreeNode::getCalleeName(unsigned int i) 
{
    assert(i < callees.size());
    assert(callees[i] != NULL);
    return callees[i]->prettyName().c_str();
}

const char * 
BPatch_loopTreeNode::name()
{
    assert(loop != NULL);
    return (const char *)hierarchicalName; 
}

unsigned int
BPatch_loopTreeNode::numCallees() { 
    return callees.size(); 
}

BPatch_loopTreeNode::~BPatch_loopTreeNode() {
    delete loop;

    for (unsigned int i = 0; i < children.size(); i++)
	delete children[i];

    delete[] hierarchicalName;
    // don't delete callees!
}


class TarjanDominator;

const int BPatch_flowGraph::WHITE = 0;
const int BPatch_flowGraph::GRAY  = 1;
const int BPatch_flowGraph::BLACK = 2;

// constructor of the class. It creates the CFG and
// deletes the unreachable code.
BPatch_flowGraph::BPatch_flowGraph(function_base *func, 
                                   process *proc, 
                                   pdmodule *mod, 
                                   bool &valid)
  : func(func), proc(proc), mod(mod), 
    loops(NULL), loopRoot(NULL), isDominatorInfoReady(false), isPostDominatorInfoReady(false), isSourceBlockInfoReady(false) 
{
   // fill the information of the basic blocks after creating
   // them. The dominator information will also be filled
   valid = true;
   
   if (!createBasicBlocks()) {
      valid = false;
      return;
   }
   
#if defined(i386_unknown_linux2_0) ||\
    defined(i386_unknown_solaris2_5) ||\
    defined(i386_unknown_nt4_0) 
   
   findAndDeleteUnreachable();

   // this may be a leaf function - if so, we won't have figure out what
   // the exit blocks are.  But we can assume that all blocks that don't
   // have any targets are exits of a sort

   if (exitBlock.empty()) {
     BPatch_basicBlock **blocks = new BPatch_basicBlock *[allBlocks.size()];
     allBlocks.elements(blocks);
     for (unsigned int i=0; i < allBlocks.size(); i++) {
       if (blocks[i]->targets.empty()) {
        exitBlock.insert(blocks[i]);
        blocks[i]->isExitBasicBlock = true;
       }
     }
     delete[] blocks;
   }
#endif
}

BPatch_flowGraph::~BPatch_flowGraph()
{
    unsigned int i;
   if (loops) {
      BPatch_basicBlockLoop** lelements = 
         new BPatch_basicBlockLoop* [loops->size()];
      
      loops->elements(lelements);
      
      for (i=0; i < loops->size (); i++)
         delete lelements [i];
      
      delete[] lelements;
      delete loops;
   }
   
   BPatch_basicBlock** belements = new BPatch_basicBlock* [allBlocks.size()];
   
   allBlocks.elements (belements);
   
   for (i=0; i < allBlocks.size(); i++)
      delete belements [i];
   
   delete[] belements;

   delete loopRoot;
}

void
BPatch_flowGraph::getAllBasicBlocks(BPatch_Set<BPatch_basicBlock*>& abb)
{
   BPatch_basicBlock** belements =
      new BPatch_basicBlock* [allBlocks.size()];
   
   allBlocks.elements(belements);
   
   for (unsigned int i=0;i<allBlocks.size(); i++)
      abb += belements[i];
   
   delete[] belements;
}

// this is the method that returns the set of entry points
// basic blocks, to the control flow graph. Actually, there must be
// only one entry point to each control flow graph but the definition
// given API specifications say there might be more.
void
BPatch_flowGraph::getEntryBasicBlock(BPatch_Vector<BPatch_basicBlock*>& ebb) 
{
   BPatch_basicBlock** belements = new BPatch_basicBlock* [entryBlock.size()];
   
   entryBlock.elements(belements);
   
   for (unsigned int i=0;i<entryBlock.size(); i++)
      ebb.push_back(belements[i]);
   
   delete[] belements;
}

// this method returns the set of basic blocks that are the
// exit basic blocks from the control flow graph. That is those
// are the basic blocks that contains the instruction for
// returning from the function
void 
BPatch_flowGraph::getExitBasicBlock(BPatch_Vector<BPatch_basicBlock*>& nbb)
{
   BPatch_basicBlock** belements = new BPatch_basicBlock* [exitBlock.size()];
   
   exitBlock.elements(belements);
   
   for (unsigned int i=0;i<exitBlock.size(); i++)
      nbb.push_back(belements[i]);
   
   delete[] belements;
}

// this methods returns the loop objects that exist in the control flow
// grap. It retuns a set. And if ther is no loop then it returns empty
// set. not NULL. 
void 
BPatch_flowGraph::getLoopsByNestingLevel(
			     BPatch_Vector<BPatch_basicBlockLoop*>& lbb,
			     bool outerMostOnly)
{
    unsigned int i;
    
    if (!loops) {
	loops = new BPatch_Set<BPatch_basicBlockLoop*>;
      
	fillDominatorInfo();
      
	// need dominator info to find back edges
	fillPostDominatorInfo();

	// mapping from basic block to set of basic blocks as back edges
	BPatch_Set<BPatch_basicBlock*>** backEdges = 
	    new BPatch_Set<BPatch_basicBlock*>* [allBlocks.size()];
      
	for (i=0; i < allBlocks.size(); i++)
	    backEdges[i] = NULL;
      
	// find back edeges which define natural loops
	findBackEdges(backEdges);

	// a map from basic block number to basic block pointer
	// which will be used to get the basic block pointer
	// from its number from the map. I am using this way
	// as I do not want to include dictionary_hash in include files
	// or use other class(empty) to get around this problem.
	// this does not give any drawback for efficency or space.
      
	BPatch_basicBlock** bnoToBBptr =
	    new BPatch_basicBlock*[allBlocks.size()];

	BPatch_basicBlock** elements =
	    new BPatch_basicBlock*[allBlocks.size()];

	allBlocks.elements(elements);

	for (i=0; i < allBlocks.size(); i++)
	    bnoToBBptr[elements[i]->blockNumber] = elements[i];

	delete[] elements;
      
	// now using the map find the basic blocks inside the loops
	fillLoopInfo(backEdges, bnoToBBptr);

	for (i=0; i < allBlocks.size(); i++)
	    delete backEdges[i];

	delete[] backEdges;
	delete[] bnoToBBptr;
    }

    BPatch_basicBlockLoop** lelements = 
	new BPatch_basicBlockLoop* [loops->size()];
   
    loops->elements(lelements);
   
    for (i=0; i < loops->size(); i++)
	// if we are only getting the outermost loops
	if (outerMostOnly) {
	    // if this loop has no parent then it is outermost
	    if (NULL == lelements[i]->parent) {
		lbb.push_back(lelements[i]);
	    }
	}
        else {
	    lbb.push_back(lelements[i]);
	}

    delete[] lelements;
}


// get all the loops in this flow graph
void 
BPatch_flowGraph::getLoops(BPatch_Vector<BPatch_basicBlockLoop*>& lbb)
{
    getLoopsByNestingLevel(lbb, false);
}

// get the outermost loops in this flow graph
void 
BPatch_flowGraph::getOuterLoops(BPatch_Vector<BPatch_basicBlockLoop*>& lbb)
{
    getLoopsByNestingLevel(lbb, true);
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
//to insert all entry basic blocks to the esrelevant field of the class.
bool BPatch_flowGraph::createBasicBlocks()
{ 
    //clean up this mess
#if defined(i386_unknown_linux2_0) ||\
    defined(i386_unknown_solaris2_5) ||\
    defined(i386_unknown_nt4_0) 
    
    pdvector< BPatch_basicBlock* >* blocks
	= func->blocks();
    
    unsigned int size = blocks->size();
       
    for( unsigned int ii = 0; ii < size; ii++ )
    {
	(*blocks)[ii]->blockNumber = ii;
	(*blocks)[ii]->flowGraph = this;
	if( (*blocks)[ii]->isEntryBasicBlock )
	    entryBlock += (*blocks)[ii];
	if( (*blocks)[ii]->isExitBasicBlock )
	    exitBlock += (*blocks)[ii];
	
	allBlocks += (*blocks)[ii];
    }
    return true;
#endif

   // assign sequential block numbers to basic blocks
   int bno = 0;

   Address effectiveAddress = (Address)
      ((void *)func->getEffectiveAddress(proc));
   
   Address relativeAddress = (Address) ((void *)func->addr());

#if defined(i386_unknown_nt4_0) || defined(mips_unknown_ce2_11) 
   long diffAddress = effectiveAddress;
#else
   long long diffAddress = effectiveAddress;
#endif

   diffAddress -= relativeAddress;

   //initializing the variables to use. Creating an address handle
   //a set of leaders and a map from leaders to the basic blocks.
   InstrucIter ah(func, proc, mod);

   Address baddr = relativeAddress;

   Address maddr = relativeAddress + func->size();

   Address taddr;

   BPatch_Set<Address> leaders;
   dictionary_hash<Address,BPatch_basicBlock*> leaderToBlock(addrHash);
   
   //start inserting the leader information. The initial address of the
   //function is inserted as a leader and a basic block is created for it
   //and inserted into the map.
   
   leaders += relativeAddress;
   leaderToBlock[relativeAddress] = new BPatch_basicBlock(this, bno++);
   allBlocks += leaderToBlock[relativeAddress];
   
   //while there are still instructions to check for in the
   //address space of the function
   //   instruction inst;


   while (ah.hasMore()) {
      //get the inctruction and the address
      //inst = ah.getInstruction();
      InstrucIter inst(ah);
      Address pos = ah++;

      //if it is a conditional branch 
      if (inst.isACondBranchInstruction()) {
         //if also it is inside the function space
         //then insert the target address as a leader
         //and create the basic block for the leader
         taddr = inst.getBranchTargetAddress(pos);

         if ((baddr <= taddr) && (taddr < maddr) && 
             !leaders.contains(taddr)) {
            leaders += taddr;

            leaderToBlock[taddr] =
               new BPatch_basicBlock(this, bno++);

            allBlocks += leaderToBlock[taddr];
         }

         //if the dleay instruction is supported by the
         //architecture then skip one more instruction         
         if (InstrucIter::delayInstructionSupported() && !inst.isAnneal())
            ++ah;
         
         //if the next address is still in the address
         //space of the function then it is also a leader
         //since the condition may not be met
         taddr = *ah;
         if ((taddr < maddr) && !leaders.contains(taddr)) {
            leaders += taddr;
            leaderToBlock[taddr] = new BPatch_basicBlock(this, bno++);
            allBlocks += leaderToBlock[taddr];
         }
      }
      else if (inst.isAJumpInstruction()) {
         //if it is unconditional jump then find the
         //target address and insert it as a leader and create
         //a basic block for it.
         taddr = inst.getBranchTargetAddress(pos);

         if ((baddr <= taddr) && (taddr < maddr) && !leaders.contains(taddr)) {
            leaders += taddr;
            leaderToBlock[taddr] = new BPatch_basicBlock(this, bno++);
            allBlocks += leaderToBlock[taddr];
         }

         //if the dleay instruction is supported by the
         //architecture then skip one more instruction
         if (InstrucIter::delayInstructionSupported() && !inst.isAnneal())
            ++ah;

#if defined(alpha_dec_osf4_0)
         taddr = *ah;
         if ((taddr < maddr) && !leaders.contains(taddr)) {
            leaders += taddr;
            leaderToBlock[taddr] = new BPatch_basicBlock(this, bno++);
            allBlocks += leaderToBlock[taddr];
         }
#endif
      }
#if defined(rs6000_ibm_aix4_1)
      else if (inst.isAIndirectJumpInstruction(InstrucIter(ah))) {
#else
      else if (inst.isAIndirectJumpInstruction()) {
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

         for (unsigned int i=0; i < possTargets.size(); i++) {
            taddr = telements[i];
            if (proc->getImage()->isAllocedAddress(taddr) &&
		(baddr <= taddr) && (taddr < maddr) && 
                !leaders.contains(taddr)) {
               leaders += taddr;
               leaderToBlock[taddr] = new BPatch_basicBlock(this, bno++);
               allBlocks += leaderToBlock[taddr];
            }
         }
         delete[] telements;

         //if the dleay instruction is supported by the
         //architecture then skip one more instruction
         if (InstrucIter::delayInstructionSupported())
            ++ah;
      }
#if defined(i386_unknown_linux2_0) ||\
    defined(i386_unknown_solaris2_5) ||\
    defined(i386_unknown_nt4_0) ||\
    defined(ia64_unknown_linux2_4) // Temporary duplication - TLM 
      else if (inst.isAReturnInstruction()) {
         if (InstrucIter::delayInstructionSupported())
            ++ah;

         taddr = *ah;

         if ((baddr <= taddr) && (taddr < maddr) && !leaders.contains(taddr)) {
            leaders += taddr;
            leaderToBlock[taddr] = new BPatch_basicBlock(this, bno++);
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
      for (unsigned int i=0; i < leaders.size(); i++) {
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
         
	 // was the previous instruction an unconditional jump?
	 bool prevInsIsUncondJump = false;

         //while the address handle has instructions to process
         while (ah.hasMore()) {
            InstrucIter inst(ah);
            Address pos = *ah;
            
            // if the next leaders instruction is seen and it is not
            // the end of the function yet
            if ((i < (leaders.size()-1)) && (pos == elements[i+1])) {
		// end of current block
		bb->endAddress = (Address)(ah.prevAddress() + diffAddress);

		// if the previous block has no targets inside the current 
		// function and is not an exit block and the previous 
		// instruction was not an unconditional jump then we infer
		// that there is a fall-through edge from bb to the next block
		// (which pos is the leader of). if the target of return
		// instructions and jumps outside the function were added to
		// the basic blocks vector of targets then checking that 
		// the vector of targets is empty would be enough
		if (bb->targets.size() == 0 && !bb->isExitBasicBlock
		    && !prevInsIsUncondJump) {
		    bb->targets += leaderToBlock[pos];
		    leaderToBlock[pos]->sources += bb;    

		    prevInsIsUncondJump = false; 
		}

		// continue to next leader
		break;
            }
            
            ah++;
            
	    prevInsIsUncondJump = false; 

            //if the instruction is conditional branch then
            //find the target address and find the corresponding 
            //leader and basic block for it. Then insert the found
            //basic block to the successor list of current leader's
            //basic block, and insert current basic block to the
            //predecessor field of the other one. Do the
            //same thing for the following ( or the other one
            //if delay instruction is supported) as a leader.
            if (inst.isACondBranchInstruction()) {
               taddr = inst.getBranchTargetAddress(pos);

               if ((baddr <= taddr) && (taddr < maddr)) {
                  bb->targets += leaderToBlock[taddr];
                  leaderToBlock[taddr]->sources += bb;
               } 
               else {
                  exitBlock += bb;
               }

               if (InstrucIter::delayInstructionSupported() && 
                   !inst.isAnneal())
                  ++ah;
               
               taddr = *ah;
               if (taddr < maddr) {
                  bb->targets += leaderToBlock[taddr];
                  leaderToBlock[taddr]->sources += bb;
               }
            }
            else if (inst.isAJumpInstruction()) {
               //if the branch is unconditional then only
               //find the target and leader and basic block 
               //coressponding to the leader. And update 
               //predecessor and successor fields of the 
               //basic blocks.
               taddr = inst.getBranchTargetAddress(pos);

               if ((baddr <= taddr) && (taddr < maddr)) {
                  bb->targets += leaderToBlock[taddr];
                  leaderToBlock[taddr]->sources += bb;
               }
               else {
		   exitBlock += bb;
               }

	       // flag this unconditional jump so that when we examine the
	       // next instruction and find that it is a leader we will know
	       // there is not a fall-through edge between these two blocks
	       prevInsIsUncondJump = true; 

               if (InstrucIter::delayInstructionSupported() && 
                   !inst.isAnneal())
                  ++ah;
            }
#if defined(rs6000_ibm_aix4_1)
            else if (inst.isAIndirectJumpInstruction(InstrucIter(ah))) {
#else
            else if (inst.isAIndirectJumpInstruction()) {
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

               for (unsigned int j=0; j < possTargets.size(); j++) {
                  taddr = telements[j];
		  if (proc->getImage()->isAllocedAddress(taddr)) {
		    if ((baddr <= taddr) && (taddr < maddr)) {
		      bb->targets += leaderToBlock[taddr];
		      leaderToBlock[taddr]->sources += bb;
		    }
		    else {
		      exitBlock += bb;
		    }
		  }
               }
               delete[] telements;

               //if the dleay instruction is supported by the
               //architecture then skip one more instruction
               if (InstrucIter::delayInstructionSupported())
                  ++ah;
            }
            else if (inst.isAReturnInstruction()) {
               exitBlock += bb;
               bb->isExitBasicBlock = true;
            }
         }
         //if the while loop terminated due to recahing the
         //end of the address space of the function then set the
         //end addresss of the basic block to the last instruction's
         //address in the address space.
         if (i == (leaders.size()-1))
            bb->endAddress = (Address)(ah.prevAddress()+diffAddress);
            
   }
   delete[] elements;
         
   return true;
}


// This function must be called only after basic blocks have been created
// by calling createBasicBlocks. It computes the source block for each
// basic block. For now, a source block is represented by the starting
// and ending line numbers in the source block for the basic block.
void
BPatch_flowGraph::createSourceBlocks() 
{
    unsigned int i;
    unsigned int j;
    unsigned int posFile;

   bool lineInformationAnalyzed = false;
  
   if (isSourceBlockInfoReady)
      return;
   
   isSourceBlockInfoReady = true;
   
   char functionName[1024];
   
   func->getMangledName(functionName, sizeof(functionName));
   
   pdstring fName(functionName);
  
   //get the line information object which contains the information for 
   //this function

   FileLineInformation* fLineInformation = NULL; 
   FileLineInformation* possibleFiles[1024];

   pdvector<module *> *appModules = proc->getAllModules();
   
   for (i = 0; i < appModules->size(); i++) {
      pdmodule* tmp = (pdmodule *)(*appModules)[i];
      LineInformation* lineInfo = tmp->getLineInformation(proc);

      //cerr << "module " << tmp->fileName() << endl;

      if (!lineInfo) {
         continue;
      }
      
      if (!lineInfo->getFunctionLineInformation(fName,possibleFiles,1024)) {
         continue;
      }

      for (posFile = 0; possibleFiles[posFile]; posFile++) {
         fLineInformation = possibleFiles[posFile];
        
         lineInformationAnalyzed = true;

         const char* fileToBeProcessed = fLineInformation->getFileNamePtr();

         //now it is time to look the starting and ending line addresses
         //of the basic blocks in the control flow graph. To define
         //the line numbers we will use the beginAddress and endAddress
         //fields of the basic blocks in the control flow graph
         //and find the closest lines to these addresses.
         //get the address handle for the region

         // FIXME FIXME FIXME This address crap...
         InstrucIter ah(func, proc, mod);

         //for every basic block in the control flow graph
         
         BPatch_basicBlock** elements = 
            new BPatch_basicBlock* [allBlocks.size()];

          allBlocks.elements(elements);

          for (j=0; j < (unsigned)allBlocks.size(); j++) {
             BPatch_basicBlock *bb = elements[j];
             
             ah.setCurrentAddress(bb->startAddress);
              
             BPatch_Set<unsigned short> lineNums;
             

             //while the address is valid  go backwards and find the
             //entry in the mapping from address to line number for closest
             //if the address is coming after a line number information
             while (ah.hasMore()) {
                Address cAddr = ah--;
                if (fLineInformation->getLineFromAddr(fName,lineNums,cAddr))
                   break;
             }
              
             //set the address handle to the start address
             ah.setCurrentAddress(bb->startAddress);
              
             //while the address is valid go forward and find the entry
             //in the mapping from address to line number for closest
             while (ah.hasMore()) {
                Address cAddr = ah++;
                if (cAddr > bb->endAddress) 
                   break;
                fLineInformation->getLineFromAddr(fName,lineNums,cAddr);
             }

              if (lineNums.size() != 0) {
                 //create the source block for the above address set
                 //and assign it to the basic block field
                 
                 if (!bb->sourceBlocks)
                    bb->sourceBlocks = 
                       new BPatch_Vector< BPatch_sourceBlock* >();
                 
                 BPatch_sourceBlock* sb = 
                    new BPatch_sourceBlock(fileToBeProcessed,lineNums);
                 
                 bb->sourceBlocks->push_back(sb);
              }
          }

          delete[] elements; 
      }
   }
   
   if (!lineInformationAnalyzed) {
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
        	for(unsigned int i=0;i<bb->targets.size();i++){
			int w = bbToint(elements[i]);
			if(semi[w] == 0){
				parent[w] = v;
				dfs(w,dfsNo);
			}
		}
		delete[] elements;
	}
	void dfsP(int v,int* dfsNo){
		semi[v] = ++(*dfsNo);
		vertex[*dfsNo] = v;
		label[v] = v;
		ancestor[v] = 0;
		child[v] = 0;
		size[v] = 1;
		BPatch_basicBlock* bb = numberToBlock[v];
		BPatch_basicBlock** elements = new BPatch_basicBlock*[bb->sources.size()];
        	bb->sources.elements(elements);
        	for(unsigned int i=0;i<bb->sources.size();i++){
			int w = bbToint(elements[i]);
			if(semi[w] == 0){
				parent[w] = v;
				dfsP(w,dfsNo);
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
			int tmp = child[v];
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
        		for(unsigned int j=0;j<bb->sources.size();j++){
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

	void findPostDominators(){
		int i;
		int dfsNo = 0;
		dfsP(r,&dfsNo);

		size[0] = 0;
		label[0] = 0;
		semi[0] = 0;

		for(i=n;i>1;i--){
			int w =  vertex[i];

			BPatch_basicBlock* bb = numberToBlock[w];
			BPatch_basicBlock** elements = new BPatch_basicBlock*[bb->targets.size()];
        		bb->targets.elements(elements);
        		for(unsigned int j=0;j<bb->targets.size();j++){
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
			bb->immediatePostDominator = numberToBlock[dom[w]];
			if(bb->immediatePostDominator){
				if(!bb->immediatePostDominator->immediatePostDominates)
					bb->immediatePostDominator->immediatePostDominates =
                                        	new BPatch_Set<BPatch_basicBlock*>;
                        	bb->immediatePostDominator->immediatePostDominates->insert(bb);
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

  unsigned int i;
  BPatch_basicBlock* bb, *tempb;
  BPatch_basicBlock** elements = NULL;
  
  if(isDominatorInfoReady)
    return;
  isDominatorInfoReady = true;
  
  /* Always use Tarjan algorithm, since performance gain is
     probably minimal for small (<8 block) graphs anyways */
  
  elements = new BPatch_basicBlock*[allBlocks.size()+1];
  allBlocks.elements(elements);

  int maxBlk = -1;
  for (i = 0; i < allBlocks.size(); i++)
    if (maxBlk < elements[i]->blockNumber)
      maxBlk = elements[i]->blockNumber;

  tempb = new BPatch_basicBlock(this, maxBlk+1);
  elements[allBlocks.size()] = tempb;
  BPatch_Set<BPatch_basicBlock*> entries = entryBlock;
  while (!entries.empty()) {
    bb = entries.minimum();
    tempb->targets.insert(bb);
    bb->sources.insert(tempb);
    entries.remove(bb);
  } 

  TarjanDominator tarjanDominator(allBlocks.size()+1,
                                  tempb,
                                  elements);
  tarjanDominator.findDominators();
  /* clean up references to our temp block */
  while (!tempb->immediateDominates->empty()) {
    bb = tempb->immediateDominates->minimum();
    bb->immediateDominator = NULL;
    tempb->immediateDominates->remove(bb);
  }
  while (!tempb->targets.empty()) {
    bb = tempb->targets.minimum();
    bb->sources.remove(tempb);
    tempb->targets.remove(bb);
  }

  delete tempb;
  delete[] elements;
}
 
void BPatch_flowGraph::fillPostDominatorInfo(){
   
  unsigned int i;
  BPatch_basicBlock* bb, *tempb;
  BPatch_basicBlock** elements = NULL;
  
  if(isPostDominatorInfoReady)
    return;
  isPostDominatorInfoReady = true;
  
  /* Always use Tarjan algorithm, since performance gain is
     probably minimal for small (<8 block) graphs anyways */
  
  elements = new BPatch_basicBlock*[allBlocks.size()+1];
  allBlocks.elements(elements);

  int maxBlk = -1;
  for (i = 0; i < allBlocks.size(); i++)
    if (maxBlk < elements[i]->blockNumber)
      maxBlk = elements[i]->blockNumber;

  tempb = new BPatch_basicBlock(this, maxBlk+1);
  elements[allBlocks.size()] = tempb;
  BPatch_Set<BPatch_basicBlock*> exits = exitBlock;
  while (!exits.empty()) {
    bb = exits.minimum();
    tempb->sources.insert(bb);
    bb->targets.insert(tempb);
    exits.remove(bb);
  }

  TarjanDominator tarjanDominator(allBlocks.size()+1,
                                  tempb,
                                  elements);
  tarjanDominator.findPostDominators();
  /* clean up references to our temp block */
  while (!tempb->immediatePostDominates->empty()) {
    bb = tempb->immediatePostDominates->minimum();
    bb->immediatePostDominator = NULL;
    tempb->immediatePostDominates->remove(bb);
  }
  while (!tempb->sources.empty()) {
    bb = tempb->sources.minimum();
    bb->targets.remove(tempb);
    tempb->sources.remove(bb);
  }
  
  delete tempb;
  delete[] elements;
}


// Add each back edge in the flow graph to the given set. An edge n -> d is
// defined to be a back edge if d dom n, where dom means every path from the
// initial node in the graph to n goes through d.
void BPatch_flowGraph::findBackEdges(BPatch_Set<BPatch_basicBlock*>** backEdges)
{
    // for each block in the graph
    BPatch_basicBlock **blks = new BPatch_basicBlock*[allBlocks.size()];
    allBlocks.elements(blks);

    for (unsigned int i = 0; i < allBlocks.size(); i++) {

	// for each of the block's targets
	BPatch_basicBlock **targs = 
	    new BPatch_basicBlock*[blks[i]->targets.size()];

	blks[i]->targets.elements(targs);

	for (unsigned int j = 0; j < blks[i]->targets.size(); j++) {

	    if (targs[j]->dominates(blks[i])) {
		BPatch_Set<BPatch_basicBlock*>* newSet = 
		    backEdges[blks[i]->blockNumber];

		if (!newSet) {
		    newSet = new BPatch_Set<BPatch_basicBlock*>;
		    backEdges[blks[i]->blockNumber] = newSet;
		}

		newSet->insert(targs[j]);
	    }
	}    

	delete[] targs;
    }


    delete[] blks;
}


// Perform a dfs of the graph of blocks bb starting at bbToColor. A 
// condition of this function is that the blocks are marked as not yet visited
// (WHITE). This function marks blocks as pre- (GRAY) and post-visited (BLACK).
void BPatch_flowGraph::dfsVisit(BPatch_basicBlock* bb, int* bbToColor)
{
    // pre-visit this block
    bbToColor[bb->blockNumber] = GRAY;

    BPatch_basicBlock** elements =  
	new BPatch_basicBlock*[bb->targets.size()];

    bb->targets.elements(elements);

    // for each of the block's sucessors 
    for (unsigned int i=0; i < bb->targets.size(); i++) {
	// if sucessor not yet visited then pre-visit it
	if (bbToColor[elements[i]->blockNumber] == WHITE) {
	    dfsVisit(elements[i], bbToColor);
	}
    }

    // post-visit this block
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


// Finds all blocks in the graph not reachable from the entry blocks and
// deletes them from the sets of blocks. The blocks are then renumbered 
// increasing with respect to their starting addresses.
void BPatch_flowGraph::findAndDeleteUnreachable()
{

	unsigned int i,j;

	int* bbToColor = new int[allBlocks.size()];
	for(i=0;i<allBlocks.size();i++)
		bbToColor[i] = WHITE;

	// perform a dfs on the graph of blocks starting from each enttry 
	// block, blocks not reached from the dfs remain colored WHITE and
	// are unreachable
	BPatch_basicBlock** elements = 
			new BPatch_basicBlock*[entryBlock.size()];
	entryBlock.elements(elements);
	for(i=0;i<entryBlock.size();i++)
		if(bbToColor[elements[i]->blockNumber] == WHITE)
			dfsVisit(elements[i],bbToColor);

	delete[] elements;

	BPatch_Set<BPatch_basicBlock*> toDelete;
	elements =  new BPatch_basicBlock*[allBlocks.size()];
	allBlocks.elements(elements);
	unsigned int oldCount = allBlocks.size();

	// for each basic block B
	for(i=0;i<oldCount;i++){
		BPatch_basicBlock* bb = elements[i];

		// if the block B was NOT visited during a dfs
		if(bbToColor[bb->blockNumber] == WHITE){

		    // for each of B's source blocks, remove B as a target
		    BPatch_basicBlock** selements = 
			new BPatch_basicBlock*[bb->sources.size()];
		    bb->sources.elements(selements);
		    unsigned int count = bb->sources.size();
		    for(j=0;j<count;j++)
			selements[j]->targets.remove(bb);
		    delete[] selements;
		    
		    // for each of B's target blocks, remove B as a source
		    selements = new BPatch_basicBlock*[bb->targets.size()];
		    bb->targets.elements(selements);
		    count = bb->targets.size();
		    for(j=0;j<count;j++)
			selements[j]->sources.remove(bb);
		    delete[] selements;
		    
		    // remove B from vec of all blocks
		    allBlocks.remove(bb);
		    exitBlock.remove(bb);
		    toDelete += bb;
		}
	}

	// delete all blocks add to toDelete
	delete[] elements;
	elements = new BPatch_basicBlock*[toDelete.size()];
	toDelete.elements(elements);	
	for(i=0;i<toDelete.size();i++)
		delete elements[i];
	delete[] elements;
	delete[] bbToColor;

	// renumber basic blocks to increase with starting addresses
	unsigned int orderArraySize = allBlocks.size();
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


// this method is used to find the basic blocks contained by the loop
// defined by a backedge. The tail of the backedge is the starting point and
// the predecessors of the tail is inserted as a member of the loop.
// then the predecessors of the newly inserted blocks are also inserted
// until the head of the backedge is in the set(reached).

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
	    if (!size) 
		data = (BPatch_basicBlock**) malloc( sizeof(BPatch_basicBlock*)*(++size));
	    else if(top == ((int)size-1))
		data = (BPatch_basicBlock**)realloc( data,sizeof(BPatch_basicBlock*)*(++size));
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

    if (!bbSet.contains(from)) {
	bbSet += from;
	stack->push(from);
    }

    while (!stack->empty()) {
	BPatch_basicBlock* bb = stack->pop();

	BPatch_basicBlock** elements = 
	    new BPatch_basicBlock*[bb->sources.size()];
	bb->sources.elements(elements);

	for(unsigned int i=0; i < bb->sources.size(); i++)
	    if (!bbSet.contains(elements[i])) {
		bbSet += elements[i];
		stack->push(elements[i]);
	    }

	delete[] elements;
    }
    delete stack;
}

// this method find all loops in the control flow graph.
// The loops are defined by backedges and the backedge defines
// a loop if the head of the backedge dominates the tail of the backedge.
// Then after finding all loops then the basic blocks in the loops
// are found and the nesting structure of the loops are found and the
// relevant fields of the loops are filled by the information discovered.
void BPatch_flowGraph::fillLoopInfo(BPatch_Set<BPatch_basicBlock*>** backEdges,
				    BPatch_basicBlock** bToP)
{
    // for each back edge source
    for (unsigned int bNo=0; bNo < allBlocks.size(); bNo++) {
	if (!backEdges[bNo])
	    continue;

	BPatch_Set<BPatch_basicBlock*>* toBlocks = backEdges[bNo];

	BPatch_basicBlock* bbFrom = bToP[bNo];

	BPatch_basicBlock** elements = 
	    new BPatch_basicBlock*[toBlocks->size()];

	toBlocks->elements(elements);

	// for each of the source targets
	for (unsigned int i=0; i < toBlocks->size(); i++) {
	    BPatch_basicBlock* bbTo = elements[i];

	    // then check whether it dominates the current basic block. 
	    // If so then it defines a loop. Otherwise there is no regular
	    // loop. create the loop
	    BPatch_basicBlockLoop* l = new BPatch_basicBlockLoop(bbTo);
	    
	    // initialize some fields of the loop object
	    l->backEdges += bbFrom;
	    (*loops) += l;
	    
	    // find all basic blocks in the loop and keep a map used
	    // to find the nest structure 
	    findBBForBackEdge(bbFrom, bbTo, l->basicBlocks);
	}

	delete[] elements;
    }

    // create two iterators on the elements of the map which is from
    // loop object to the set of basic blocks
    // for each loop object pair check whether one is subset of the
    // other. That is the set of basic blocks in the loop. If one is
    // subset of the other one then the smaller one is nested in the
    // bigger one so insert the smaller one to the containedLoops 
    // field of the loop object.

    BPatch_basicBlockLoop** allLoops = 
	new BPatch_basicBlockLoop*[loops->size()];

    loops->elements(allLoops);

    for (unsigned int i=0; i < loops->size(); i++)
	for (unsigned int j=0; j < loops->size(); j++)
	    if (i != j) {
		BPatch_basicBlockLoop* l1 = allLoops[i];
		BPatch_basicBlockLoop* l2 = allLoops[j];

		BPatch_Set<BPatch_basicBlock*> diff(l2->basicBlocks);
		diff -= l1->basicBlocks;

		if (diff.empty()) {
		    l1->containedLoops += l2;

		    // assign l2's parent. l1 may not be l2's real parent, 
		    // with each l1/l2 comparison we check if l1 is a closer
		    // ancestor to to l2 than l2's current parent, if so l1
		    // becomes l2's parent.

		    // l2 has no parent so l1 is closest so far.
		    if (NULL == l2->parent) {
			l2->parent = l1;
		    }
		    // else if l1 is a more direct ancestor of l2 than l2's 
		    // current parent. *this assumes allLoops is ordered with
		    // the outer most loops first, otherwise if l1 is l2's 
		    // parent but l2 already has parent l0 and l0 is an 
		    // ancestor of l1 (AND this link has not yet been set)
		    // then l2->parent will stay l0 when it should be l1.*
		    else if (l1->hasAncestor(l2->parent)) {
			l2->parent = l1;
		    }
		}
	    }

    delete[] allLoops;
}


#ifdef DEBUG 
//print method
ostream&
operator<<(ostream& os,BPatch_flowGraph& fg){
   os << "Begin CFG :\n";

   BPatch_basicBlock** belements = 
      new BPatch_basicBlock*[fg.allBlocks.size()];

   fg.allBlocks.elements(belements);

   for (unsigned int i=0; i < fg.allBlocks.size(); i++)
      os << *belements[i];

   delete[] belements;

   if (fg.loops) {
      BPatch_basicBlockLoop** lelements = 
         new BPatch_basicBlockLoop*[fg.loops->size()];

      fg.loops->elements(lelements);  

      for (unsigned int i=0;i<fg.loops->size(); i++)
         os << *lelements[i];

      delete[] lelements;
   }

   os << "End CFG :\n";
   return os;
}
#endif


// return a pair of the min and max source lines for this loop
pdpair<u_short, u_short> 
getLoopMinMaxSourceLines(BPatch_basicBlockLoop * loop) 
{
    BPatch_Vector<BPatch_basicBlock*> blocks;
    loop->getLoopBasicBlocks(blocks);
	
    BPatch_Vector<u_short> lines;
	
    for (u_int j = 0; j < blocks.size (); j++) {
	BPatch_Vector<BPatch_sourceBlock*> sourceBlocks;
	blocks[j]->getSourceBlocks(sourceBlocks);
	
	for (u_int k = 0; k < sourceBlocks.size (); k++) {
	    BPatch_Vector<u_short> sourceLines;
	    sourceBlocks[k]->getSourceLines(sourceLines);
	    for (u_int l = 0; l < sourceLines.size(); l++) 
		lines.push_back(sourceLines[l]);
	}
    }

    pdpair<u_short, u_short> mm = min_max_pdpair<u_short>(lines);

    return mm;
}


void 
dfsCreateLoopHierarchy(BPatch_loopTreeNode * parent,
		       BPatch_Vector<BPatch_basicBlockLoop *> &loops, 
		       pdstring level)
{
    for (unsigned int i = 0; i < loops.size (); i++) {
	// loop name is hierarchical level
	pdstring clevel = (level != "") 
	    ? level + "." + pdstring(i+1)
	    : pdstring(i+1);
	
	// add new tree nodes to parent
	BPatch_loopTreeNode * child = 
	    new BPatch_loopTreeNode(loops[i],
				    (pdstring("loop "+clevel)).c_str());

	parent->children.push_back(child);

	// recurse with this child's outer loops
	BPatch_Vector<BPatch_basicBlockLoop*> outerLoops;
	loops[i]->getOuterLoops(outerLoops);
	dfsCreateLoopHierarchy(child, outerLoops, clevel);
    }
}


void 
BPatch_flowGraph::createLoopHierarchy()
{
    loopRoot = new BPatch_loopTreeNode(NULL, NULL);
    BPatch_Vector<BPatch_basicBlockLoop *> loops;
    getOuterLoops(loops);
    dfsCreateLoopHierarchy(loopRoot, loops, "");

    const pdvector<instPoint*> &instPs = func->funcCalls(proc);

    for (unsigned i = 0; i < instPs.size(); i++) {
	function_base *f;

	bool found = proc->findCallee(*(instPs[i]), f);

	if (found && f != NULL) {
	    insertCalleeIntoLoopHierarchy(f, instPs[i]->pointAddr());
	}
	else {
	    //fprintf( stderr, "BPatch_flowGraph::createLoopHierarchy "
            //"couldn't find callee by inst point.\n");
	}
    }
}


// try to insert func into the appropriate spot in the loop tree based on
// address ranges. if succesful return true, return false otherwise.
bool 
BPatch_flowGraph::dfsInsertCalleeIntoLoopHierarchy(BPatch_loopTreeNode *node, 
						   function_base *func,
						   unsigned long addr)
{
    // if this node contains func then insert it
    if ((node->loop != NULL) && node->loop->containsAddress(addr)) {
	node->callees.push_back(func);
	return true;
    }

    // otherwise recur with each of node's children
    bool success = false;

    for (unsigned int i = 0; i < node->children.size(); i++) 
	success = success || 
	    dfsInsertCalleeIntoLoopHierarchy(node->children[i], func, addr);
    
    return success;
}


void 
BPatch_flowGraph::insertCalleeIntoLoopHierarchy(function_base * func,
						unsigned long addr)
{
    // try to insert func into the loop hierarchy
    bool success = dfsInsertCalleeIntoLoopHierarchy(loopRoot, func, addr);

    // if its not in a loop make it a child of the root
    if (!success) {
	loopRoot->callees.push_back(func);
    }
}


BPatch_loopTreeNode *
BPatch_flowGraph::getLoopTree() 
{ 
    if (loopRoot == NULL) 
	createLoopHierarchy();
    return loopRoot; 
}


void 
BPatch_flowGraph::dfsPrintLoops(BPatch_loopTreeNode *n) {
  if (n->loop != NULL) {
      pdpair<u_short, u_short> mm = getLoopMinMaxSourceLines(n->loop);
      
      fprintf(stderr, "%s (source %d-%d)\n", n->name(), mm.first, mm.second);
  }

  for (unsigned int i = 0; i < n->children.size(); i++) {
      dfsPrintLoops(n->children[i]);
  }
}


void 
BPatch_flowGraph::printLoops()
{    
    dfsPrintLoops(getLoopTree());
}

