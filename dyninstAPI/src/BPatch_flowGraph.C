/*
 * Copyright (c) 1996-2004 Barton P. Miller
 * 
 * We provide the Paradyn Parallel Performance Tools (below
 * described as "Paradyn") on an AS IS basis, and do not warrant its
 * validity or performance.  We reserve the right to update, modify,
 * or discontinue this software at any time.  We shall have no
 * obligation to supply such updates or modifications or any other
 * form of support to you.
 * 
 * This license is for research uses.  For such uses, there is no
 * charge. We define "research use" to mean you may freely use it
 * inside your organization for whatever purposes you see fit. But you
 * may not re-distribute Paradyn or parts of Paradyn, in any form
 * source or binary (including derivatives), electronic or otherwise,
 * to any other organization or entity without our permission.
 * 
 * (for other uses, please contact us at paradyn@cs.wisc.edu)
 * 
 * All warranties, including without limitation, any warranty of
 * merchantability or fitness for a particular purpose, are hereby
 * excluded.
 * 
 * By your use of Paradyn, you understand and agree that we (or any
 * other person or entity with proprietary rights in Paradyn) are
 * under no obligation to provide either maintenance services,
 * update services, notices of latent defects, or correction of
 * defects for Paradyn.
 * 
 * Even if advised of the possibility of such damages, under no
 * circumstances shall we (or any other person or entity with
 * proprietary rights in the software licensed hereunder) be liable
 * to you or any third party for direct, indirect, or consequential
 * damages of any character regardless of type of action, including,
 * without limitation, loss of profits, loss of use, loss of good
 * will, or computer failure or malfunction.  You agree to indemnify
 * us (and any other person or entity with proprietary rights in the
 * software licensed hereunder) for any and all liability it may
 * incur to third parties resulting from your use of Paradyn.
 */

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
    loops(NULL), loopRoot(NULL), isDominatorInfoReady(false), 
    isPostDominatorInfoReady(false), isSourceBlockInfoReady(false) 
{
   // fill the information of the basic blocks after creating
   // them. The dominator information will also be filled
   valid = true;
   
   unsigned tmpSize = func->get_size();
   if(!tmpSize){
	valid = false;
	return;
   }

   if (!createBasicBlocks()) {
      valid = false;
      return;
   }
   
#if ! defined( arch_x86 )
   findAndDeleteUnreachable();
#endif /* defined( arch_x86 ) */

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

   // if are not still bale to find an exit block yet
   // then we do special handling and assign an exit basic block
   // to the CFG. This is common for functions composed of infinite
   // loops which no loop-breakers or returns frm the function.

   if (exitBlock.empty()) {
	 assignAnExitBlockIfNoneExists();
   }
}



extern BPatch_point *createInstructionEdgeInstPoint(process* proc, 
                                                    pd_Function *func,
                                                    BPatch_edge *edge);

extern void createEdgeTramp(process *proc, image *img, BPatch_edge *edge);


BPatch_point *BPatch_flowGraph::createInstPointAtEdge(BPatch_edge *edge)
{
    pd_Function *pfunc = (pd_Function *)func;

    assert(edge->point == NULL);

    if (edge->needsEdgeTramp()) {
        // if this edge already has an address to instrument (because
        // its buddy conditional edge has already been instrumented)
        // then we use it. otherwise we need to create an edge tramp,
        // this will set instAddr for this edge and its buddy
        // conditional edge
        if (!edge->instAddr) 
            createEdgeTramp(proc, pfunc->file()->exec(), edge);

        edge->point = createInstructionEdgeInstPoint(proc, pfunc, edge);
    }
    else {
        // the address of the last instruction of the source block 
        void *addr = (void *)edge->source->getRelLast();

        // edge->point = createInstPointAtAddr(addr);
        // XXX this is to remove dependency on BPatch_image, the following
        // function can be replaced with de-bpachified version in mdl.C

        edge->point = createInstructionInstPoint(proc, addr, NULL, NULL);
    }

    return edge->point;
}

bool DEBUG_LOOP = false;

void 
BPatch_flowGraph::findLoopExitInstPoints(BPatch_loop *loop,
                                         BPatch_Vector<BPatch_point*> *points)
{
    BPatch_Vector<BPatch_basicBlock*> blocks;
    loop->getLoopBasicBlocks(blocks);
    
    // for each block in this loop (including its subloops)
    for (unsigned i = 0; i < blocks.size(); i++) {
        BPatch_Vector<BPatch_edge*> edges;
        blocks[i]->getOutgoingEdges(edges);
        
        // for each of its outgoing edges, if the edge's target is
        // outside this loop then this edge exits this loop
        for (unsigned j = 0; j < edges.size(); j++) 
            if (!loop->hasBlock(edges[j]->target)) {
		if (DEBUG_LOOP) edges[j]->dump();
                points->push_back(edges[j]->instPoint());
            }
    }
}

BPatch_Vector<BPatch_point*> *
BPatch_flowGraph::findLoopInstPoints(const BPatch_procedureLocation loc, 
                                     BPatch_loop *loop)
{
    /*
     * We need to detect and handle following cases:
     * 
     * (1) If a loop has no entry edge, e.g. the loop head is also
     * first basic block of the function, we need to create a loop
     * preheader.  we probably want to relocate the function at this
     * point.
     * 
     *        ___   
     *       v   |
     *   f: [_]--/
     *       |
     *      [ ]
     *
     *
     * (2) If a loop header is shared between two loops then add a new
     * nop node N, redirect the back edge of each loop to N and make N
     * jump to the header. this transforms the two loops into a single
     * loop.
     * 
     *      _              _
     * --->[_]<---        [_]<---
     * |  _/ \_  |       _/ \_  |
     * \-[_] [_]-/      [_] [_] |
     *                    \_/   |
     *                    [N]---/
     *
     *
     *  Also, loop instrumentation works on the control flow as it was
     *  _originally_ parsed. Function entry/exit instrumentation may
     *  have modified the this control flow, but this new
     *  instrumentation is not cognizant of these modifications. This
     *  instrumentation therefore may clobber any instrumentation that
     *  was added because it has an inaccurate view of the binary.
     *
     */

    if (DEBUG_LOOP) 
	fprintf(stderr,"%s findLoopInstPoints 0x%x\n",
		func->prettyName().c_str(), loop);

    BPatch_Vector<BPatch_point*> *points = new BPatch_Vector<BPatch_point *>;

    switch (loc) {

    case BPatch_locLoopEntry: {
	if (DEBUG_LOOP) fprintf(stderr,"loop entry\n");

        // return inst points for each edge e where e's target is this
        // loop's head and e's source is not a block in this loop or
        // its subloops. we assume a natural loop, that the loop is
        // always entered through the head
        BPatch_Vector<BPatch_edge*> edges;
        loop->getLoopHead()->getIncomingEdges(edges);
        for (unsigned i = 0; i < edges.size(); i++) {
            // hasBlock is inclusive, checks subloops
            if (!loop->hasBlock(edges[i]->source)) {
		if (DEBUG_LOOP) edges[i]->dump();
                points->push_back(edges[i]->instPoint());
            }
        }

        if (0 == points->size()) {
            fprintf(stderr,"Warning: request to instrument loop entry "
                    "of a loop w/o an entry edge.");
        }

        break;        
    }

    case BPatch_locLoopExit: {
	if (DEBUG_LOOP) fprintf(stderr,"loop exit\n");

        // return inst points for all edges e such that e->source is a
        // member of this loop (including subloops) and e->target is
        // not a member of this loop (including its subloops)
        findLoopExitInstPoints(loop, points);

        break;
    }

    case BPatch_locLoopStartIter: {
        if (DEBUG_LOOP) fprintf(stderr,"loop start iter\n");

        // instrument the head of the loop
        BPatch_point *p;
        void *addr = (void*)loop->getLoopHead()->getRelStart();
        p = createInstructionInstPoint(proc, addr, NULL, NULL);
        points->push_back(p);

        break;
    }

    case BPatch_locLoopEndIter: {
        if (DEBUG_LOOP) fprintf(stderr,"loop end iter\n");

        // point for the backedge of this loop 
        BPatch_edge *edge = loop->backEdge;
        if (DEBUG_LOOP) edge->dump();
        points->push_back(edge->instPoint());

        // and all edges which exit the loop
        findLoopExitInstPoints(loop, points);

        break;
    }

    default: {
        bperr("called findLoopInstPoints with non-loop location\n");
        assert(0);
    }

    }

    return points;
}


// This function tries to assign an exit block to a CFG
// that originally does not have eny exit blocks.
// Common for functions with infinite loops and no returns.
// this function should be used as the last resort
// for asssigning an exit block to CFG for later post dominator
// calculations to proceed.
// The implementation of this function can be changed.
// Currently we chose the basic block which has the lowest
// id and the target of a backedge. Sort of approximating the
// outermost loop.

void BPatch_flowGraph::assignAnExitBlockIfNoneExists(){
   if (!loops)
     fillDominatorInfo();

	/** processedBlocks entries show ; 
	  -1 ==> not processed
	  n >= 0 ==> depth in dominator tree
	  */

	int* processedBlocks = new int[allBlocks.size()];
	BPatch_basicBlock** depthMinimums = new BPatch_basicBlock*[allBlocks.size()];
	for(unsigned i=0;i<allBlocks.size();i++){
		processedBlocks[i] = -1;
		depthMinimums[i] = NULL;
	}

	int depth = 0;

	BPatch_Set<BPatch_basicBlock*> currentDepth;

	currentDepth |= entryBlock;

	while(!currentDepth.empty()){
		BPatch_Set<BPatch_basicBlock*> nextDepth;
		BPatch_Set<BPatch_basicBlock*> tmpSet;
		while(!currentDepth.empty()){
			BPatch_basicBlock* bb = NULL;
			currentDepth.extract(bb);
			if(processedBlocks[bb->blockNumber] > -1)
				continue;
			processedBlocks[bb->blockNumber] =  depth;
			if(bb->immediateDominates)
				nextDepth |= *(bb->immediateDominates);
			tmpSet += bb;
		}

		int minDepth = allBlocks.size();
		while(!tmpSet.empty()){
			BPatch_basicBlock* bb = NULL;
			tmpSet.extract(bb);
			BPatch_Set<BPatch_basicBlock*> bbtargets = bb->targets;
			while(!bbtargets.empty()){
				BPatch_basicBlock* tobb = NULL;
				bbtargets.extract(tobb);
				if(processedBlocks[tobb->blockNumber] == -1)
					continue;
				if(minDepth > processedBlocks[tobb->blockNumber]){
					minDepth = processedBlocks[tobb->blockNumber];
					depthMinimums[depth] = tobb;
				}
			}
		}
		depth++;
		currentDepth |= nextDepth;
	}

	for(int j=(int)(allBlocks.size())-1;j>=0;j--)
		if(depthMinimums[j]){
			exitBlock += depthMinimums[j];
			depthMinimums[j]->isExitBasicBlock = true;
			break;
		}

	delete[] processedBlocks;
	delete[] depthMinimums;
}

BPatch_flowGraph::~BPatch_flowGraph()
{
    unsigned int i;

    if (loops) {
      BPatch_loop **lelements = new BPatch_loop *[loops->size()];
      
      loops->elements(lelements);
      
      for (i=0; i < loops->size(); i++)
         delete lelements [i];
      
      delete[] lelements;
      delete loops;
    }
   
   BPatch_basicBlock **belements = new BPatch_basicBlock *[allBlocks.size()];
   
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


void 
BPatch_flowGraph::createLoops()
{
    loops = new BPatch_Set<BPatch_loop*>;
    
    BPatch_edge **allBackEdges = new BPatch_edge*[backEdges->size()];
    backEdges->elements(allBackEdges);

    // for each back edge
    unsigned i;
    for (i = 0; i < backEdges->size(); i++) {
        assert(allBackEdges[i] != NULL);
        BPatch_loop *loop = new BPatch_loop(allBackEdges[i], this);
	    
        // find all basic blocks in the loop and keep a map used
        // to find the nest structure 
        findBBForBackEdge(allBackEdges[i], loop->basicBlocks);

        (*loops) += loop;
    }

    delete[] allBackEdges;

    BPatch_loop **allLoops = new BPatch_loop*[loops->size()];
    loops->elements(allLoops);
    
    // for each pair of loops l1,l2
    for (i = 0; i < loops->size(); i++) {
        for (unsigned j=0; j < loops->size(); j++) {
            if (i==j) continue;
            
            BPatch_loop *l1 = allLoops[i];
            BPatch_loop *l2 = allLoops[j];
            
            unsigned long l1start = l1->backEdge->target->getRelStart();
	    unsigned long l2start = l2->backEdge->target->getRelStart();   
	    unsigned long l1end   = l1->backEdge->source->getRelLast();
	    unsigned long l2end   = l2->backEdge->source->getRelLast();
            
            // if l2 is inside l1
            if (l1start < l2start && l2end < l1end) {
                // l1 contains l2
                l1->containedLoops += l2;
                
                // l2 has no parent, l1 is best so far
                if (!l2->parent) 
                    l2->parent = l1;
                else 
                   // if l1 is closer to l2 than l2's existing parent
                    if (l1start > l2->parent->getLoopHead()->getRelStart()) 
                        l2->parent = l1;
            }
        }
    }
    
    delete[] allLoops;
}

// this methods returns the loop objects that exist in the control flow
// grap. It retuns a set. And if ther is no loop then it returns empty
// set. not NULL. 
void 
BPatch_flowGraph::getLoopsByNestingLevel(BPatch_Vector<BPatch_loop*>& lbb,
                                         bool outerMostOnly)
{
    if (!loops) {
        fillDominatorInfo();
        fillPostDominatorInfo();
        createEdges();
        createLoops();
    }
    
    BPatch_loop **lelements = new BPatch_loop* [loops->size()];
   
    loops->elements(lelements);
   
    for (unsigned i=0; i < loops->size(); i++)
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
BPatch_flowGraph::getLoops(BPatch_Vector<BPatch_loop*>& lbb)
{
    getLoopsByNestingLevel(lbb, false);
}

// get the outermost loops in this flow graph
void 
BPatch_flowGraph::getOuterLoops(BPatch_Vector<BPatch_loop*>& lbb)
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

#if defined( arch_x86 )
 
    pdvector<BPatch_basicBlock *> *blocks = func->blocks();
    
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

#else /* [not] defined( arch_x86 ) */

   // assign sequential block numbers to basic blocks
   int bno = 0;

   Address effectiveAddress = (Address)
      ((void *)func->getEffectiveAddress(proc));
   
   Address relativeAddress = (Address) ((void *)func->get_address());

   long long diffAddress = effectiveAddress;

   diffAddress -= relativeAddress;

   //initializing the variables to use. Creating an address handle
   //a set of leaders and a map from leaders to the basic blocks.
   InstrucIter ah(func, proc, mod);

   Address baddr = relativeAddress;

   Address maddr = relativeAddress + func->get_size();

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
      else if (inst.isAIndirectJumpInstruction(InstrucIter(ah)))
#else
      else if (inst.isAIndirectJumpInstruction())
#endif
      {
         InstrucIter ah2(ah);
         BPatch_Set<Address> possTargets; 
         ah2.getMultipleJumpTargets(possTargets);
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

#if defined(ia64_unknown_linux2_4) 
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
      
      else if( inst.getInstruction()->getType() == IA64_instruction::ALLOC ) {
        if( ! leaders.contains( pos ) ) { 
        	// /* DEBUG */ fprintf( stderr, "%s[%d]: alloc at 0x%lx is a (new) leader.\n", __FILE__, __LINE__, pos );
        	leaders += pos;
        	leaderToBlock[pos] = new BPatch_basicBlock( this, bno++ );
        	allBlocks += leaderToBlock[pos];
        	}
      	} /* end if an alloc instruction */
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
		bb->lastInsnAddress = (Address)(ah.prevAddress()+diffAddress);
                bb->setRelEnd((Address)((*ah)+diffAddress));

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
            else if (inst.isAIndirectJumpInstruction(InstrucIter(ah)))
#else
            else if (inst.isAIndirectJumpInstruction())
#endif
            {
               InstrucIter ah2(ah);
               BPatch_Set<Address> possTargets; 

               ah2.getMultipleJumpTargets(possTargets);
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
         {
            bb->lastInsnAddress = (Address)(ah.prevAddress()+diffAddress);
            bb->setRelEnd((Address)((*ah)+diffAddress));
         }         
   
   }
   delete[] elements;
         
   return true;
#endif /* defined( arch_x86 ) */
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
             while (ah.hasPrev()) {
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
                if (cAddr > bb->lastInsnAddress) 
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
  if (tempb->immediateDominates)
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

  if (tempb->immediateDominates) 
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
  
  /* First find basic blocks that are not reachable from 
     one of the exit block. their color are white after
     the following DFS with sources
   */

  int* bbToColor = new int[allBlocks.size()];
  for(i=0;i<allBlocks.size();i++)
    bbToColor[i] = WHITE;

  elements = new BPatch_basicBlock*[exitBlock.size()];
  exitBlock.elements(elements);
  for(i=0;i<exitBlock.size();i++)
    if(bbToColor[elements[i]->blockNumber] == WHITE)
       dfsVisitWithSources(elements[i],bbToColor);

  delete[] elements;

  elements = new BPatch_basicBlock*[allBlocks.size()+1];
  allBlocks.elements(elements);

  BPatch_Set<BPatch_basicBlock*> extraBlockSet;	
  for (i = 0; i < allBlocks.size(); i++){
	if(exitBlock.contains(elements[i]))
		continue;
	if(bbToColor[elements[i]->blockNumber] == WHITE){
		/** a block that does not reach the exit block
          * root of the forest so include this 
          * for processing for post dom calculation
          */
		dfsVisitWithSources(elements[i],bbToColor);
		extraBlockSet += elements[i];
	}
  }

  delete[] bbToColor;

  int maxBlk = -1;
  for (i = 0; i < allBlocks.size(); i++)
    if (maxBlk < elements[i]->blockNumber)
      maxBlk = elements[i]->blockNumber;

  tempb = new BPatch_basicBlock(this, maxBlk+1);
  elements[allBlocks.size()] = tempb;
  BPatch_Set<BPatch_basicBlock*> exits = exitBlock;

  /** add the basic blocks that do not reach exit blocks 
	to the exit list */

  exits |= extraBlockSet;

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
  if (tempb->immediatePostDominates)
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


// Add each back edge in the flow graph to the given set. A back edge
// in a flow graph is an edge whose head dominates its tail. 
void BPatch_flowGraph::createEdges()
{
    /*
     * Indirect jumps are NOT currently handled correctly
     */

    backEdges = new BPatch_Set<BPatch_edge*>;

    BPatch_basicBlock **blks = new BPatch_basicBlock*[allBlocks.size()];
    allBlocks.elements(blks);

    // for each block in the graph
    for (unsigned i = 0; i < allBlocks.size(); i++) {
        BPatch_basicBlock *source = blks[i];

        // source's targets
	BPatch_basicBlock **targs = 
	    new BPatch_basicBlock*[source->targets.size()];

	source->targets.elements(targs);

        unsigned numTargs = source->targets.size();

        // create edges
        image *img = mod->exec();
        const unsigned char *relocp;

        relocp = img->getPtrToInstruction((unsigned long)source->getRelLast());

        if (numTargs == 1) {
            BPatch_edge *edge;
            edge = new BPatch_edge(source, targs[0], this, relocp);

            targs[0]->incomingEdges += edge;
            source->outgoingEdges += edge;

//             fprintf(stderr, "t1 %2d %2d\n",source->blockNo(),
//                     targs[0]->blockNo());
            if (targs[0]->dominates(source))
                (*backEdges) += edge;
        }
        else if (numTargs == 2) {
            //XXX could be an indirect jump with two targets

            // conditional jumps create two edges from a block
            BPatch_edge *edge0 = 
                new BPatch_edge(source, targs[0], this, relocp); 

            BPatch_edge *edge1 = 
                new BPatch_edge(source, targs[1], this, relocp); 

//             fprintf(stderr, "t2 %2d %2d\n",source->blockNo(),
//                     targs[0]->blockNo());
// 	    fprintf(stderr, "t2 %2d %2d\n",source->blockNo(),
//                     targs[1]->blockNo());

            source->outgoingEdges += edge0;
            source->outgoingEdges += edge1;

            targs[0]->incomingEdges += edge0;
            targs[1]->incomingEdges += edge1;

            edge0->conditionalBuddy = edge1;
            edge1->conditionalBuddy = edge0;

            if (targs[0]->dominates(source))
                (*backEdges) += edge0;

            if (targs[1]->dominates(source))
                (*backEdges) += edge1;

            // taken and fall-through edge should not both be back edges
// 	    if (targs[0]->dominates(source) && targs[1]->dominates(source)) {
//                 bperr("Both edge targets can not dominate the source.\n");
//  		edge0->dump();
//  		edge1->dump();
//  	    }

            //assert(!(targs[0]->dominates(source) && 
	    //targs[1]->dominates(source)));
        }
        else {
            //XXX indirect jumps, set conditional buddy?
        }

	delete[] targs;
    }


    delete[] blks;
}


// Perform a dfs of the graph of blocks bb starting at bbToColor. A 
// condition of this function is that the blocks are marked as not yet visited
// (WHITE). This function marks blocks as pre- (GRAY) and post-visited (BLACK).
void BPatch_flowGraph::dfsVisitWithTargets(BPatch_basicBlock* bb, int* bbToColor)
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
	    dfsVisitWithTargets(elements[i], bbToColor);
	}
    }

    // post-visit this block
    bbToColor[bb->blockNumber] = BLACK;

    delete[] elements;
}

void BPatch_flowGraph::dfsVisitWithSources(BPatch_basicBlock* bb, int* bbToColor)
{
    // pre-visit this block
    bbToColor[bb->blockNumber] = GRAY;

    BPatch_basicBlock** elements =  
	new BPatch_basicBlock*[bb->sources.size()];

    bb->sources.elements(elements);

    // for each of the block's sucessors 
    for (unsigned int i=0; i < bb->sources.size(); i++) {
	// if sucessor not yet visited then pre-visit it
	if (bbToColor[elements[i]->blockNumber] == WHITE) {
	    dfsVisitWithSources(elements[i], bbToColor);
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
			dfsVisitWithTargets(elements[i],bbToColor);

	delete[] elements;

	BPatch_Set<BPatch_basicBlock*> toDelete;
	elements =  new BPatch_basicBlock*[allBlocks.size()];
	allBlocks.elements(elements);
	unsigned int oldCount = allBlocks.size();

	// for each basic block B
	for (i=0;i<oldCount;i++) {
		BPatch_basicBlock* bb = elements[i];

		// if the block B was NOT visited during a dfs
		if (bbToColor[bb->blockNumber] == WHITE) {

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
//                     fprintf(stderr, "bb %u [0x%x 0x%x]\n",
//                             bb->blockNo(),
//                             bb->getRelStart(),
//                             bb->getRelEnd() );

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

void BPatch_flowGraph::findBBForBackEdge(BPatch_edge* backEdge,
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

    bbSet += backEdge->target;

    if (!bbSet.contains(backEdge->source)) {
	bbSet += backEdge->source;
	stack->push(backEdge->source);
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



// return a pair of the min and max source lines for this loop
pdpair<u_short, u_short> 
getLoopMinMaxSourceLines(BPatch_loop * loop) 
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


// sort blocks by address ascending
void bsort_loops_addr_asc(BPatch_Vector<BPatch_loop*> &v) 
{
    if (v.size()==0) return;
    for (unsigned i=0; i < v.size()-1; i++) 
	for (unsigned j=0; j < v.size()-1-i; j++)
	    if (v[j+1]->getLoopHead()->getStartAddress() 
                < v[j]->getLoopHead()->getStartAddress()) {    
		BPatch_loop *tmp = v[j]; 
		v[j] = v[j+1];
		v[j+1] = tmp;
            }
}


void 
dfsCreateLoopHierarchy(BPatch_loopTreeNode * parent,
		       BPatch_Vector<BPatch_loop *> &loops, 
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
				    (pdstring("loop_"+clevel)).c_str());

	parent->children.push_back(child);

	// recurse with this child's outer loops
	BPatch_Vector<BPatch_loop*> outerLoops;
	loops[i]->getOuterLoops(outerLoops);
        bsort_loops_addr_asc(outerLoops);

	dfsCreateLoopHierarchy(child, outerLoops, clevel);
    }
}


void 
BPatch_flowGraph::createLoopHierarchy()
{
    loopRoot = new BPatch_loopTreeNode(NULL, NULL);

    BPatch_Vector<BPatch_loop *> outerLoops;
    getOuterLoops(outerLoops);

    bsort_loops_addr_asc(outerLoops);

    dfsCreateLoopHierarchy(loopRoot, outerLoops, "");

    const pdvector<instPoint*> &instPs = func->funcCalls(proc);

    for (unsigned i = 0; i < instPs.size(); i++) {
	function_base *f;

	bool found = proc->findCallee(*(instPs[i]), f);

	if (found && f != NULL) 
	    insertCalleeIntoLoopHierarchy(f, instPs[i]->pointAddr());
// 	else 
// 	    fprintf(stderr, "BPatch_flowGraph::createLoopHierarchy "
//                     "couldn't find callee by inst point.\n");
    }
}


// try to insert func into the appropriate spot in the loop tree based on
// address ranges. if succesful return true, return false otherwise.
bool 
BPatch_flowGraph::dfsInsertCalleeIntoLoopHierarchy(BPatch_loopTreeNode *node, 
						   function_base *callee,
						   unsigned long addr)
{
    // if this node contains func then insert it
    if ((node->loop != NULL) && node->loop->containsAddress(addr)) {
	node->callees.push_back(callee);
	return true;
    }

    // otherwise recur with each of node's children
    bool success = false;

    for (unsigned int i = 0; i < node->children.size(); i++) 
	success = success || 
	    dfsInsertCalleeIntoLoopHierarchy(node->children[i], callee, addr);
    
    return success;
}


void 
BPatch_flowGraph::insertCalleeIntoLoopHierarchy(function_base *callee,
						unsigned long addr)
{
    // try to insert func into the loop hierarchy
    bool success = dfsInsertCalleeIntoLoopHierarchy(loopRoot, callee, addr);

    // if its not in a loop make it a child of the root
    if (!success) {
	loopRoot->callees.push_back(callee);
    }
}


BPatch_loopTreeNode *
BPatch_flowGraph::getLoopTree() 
{ 
    if (loopRoot == NULL) 
	createLoopHierarchy();
    return loopRoot; 
}


BPatch_loop *
BPatch_flowGraph::findLoop(const char *name) 
{ 
    return getLoopTree()->findLoop(name);
}


void 
BPatch_flowGraph::dfsPrintLoops(BPatch_loopTreeNode *n) {

  if (n->loop != NULL) {
      //    pdpair<u_short, u_short> mm = getLoopMinMaxSourceLines(n->loop);
      //  printf("%s (source %d-%d)\n", n->name(), mm.first, mm.second);
        
      printf("%s %s\n", n->name(),func->prettyName().c_str());
  }

  for (unsigned i = 0; i < n->children.size(); i++) {
      dfsPrintLoops(n->children[i]);
  }
}


void 
BPatch_flowGraph::printLoops()
{    
    dfsPrintLoops(getLoopTree());
}


void 
BPatch_flowGraph::dump()
{    
    BPatch_basicBlock **blocks = new BPatch_basicBlock *[allBlocks.size()];
    allBlocks.elements(blocks);
    for (unsigned i=0; i < allBlocks.size(); i++) {
        fprintf(stderr,"[%u 0x%x 0x%x]\n",
                blocks[i]->blockNo(),
                blocks[i]->getRelStart(), 
                blocks[i]->getRelEnd());
        
    }

}
