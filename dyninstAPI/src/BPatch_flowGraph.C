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

#include "BPatch_edge.h"

#include "util.h"
#include "process.h"
#include "symtab.h"
#include "instPoint.h"

#include "InstrucIter.h"

#include "LineInformation.h"

#include "BPatch_flowGraph.h"
#include "mapped_object.h"
#include "dominator.h"
#include "function.h"

// constructor of the class. It creates the CFG and
// deletes the unreachable code.
BPatch_flowGraph::BPatch_flowGraph(BPatch_function *func, 
                                   bool &valid)
    : func_(func), bproc(func->getProc()), mod(func->getModule()), 
      loops(NULL), loopRoot(NULL), isDominatorInfoReady(false), 
      isPostDominatorInfoReady(false), isSourceBlockInfoReady(false) 
{
   // fill the information of the basic blocks after creating
   // them. The dominator information will also be filled
   valid = true;

   unsigned tmpSize = func->getSize();
   if(!tmpSize){
      fprintf(stderr, "Func has no size!\n");
      valid = false;
      return;
   }

   if (!createBasicBlocks()) {
      fprintf(stderr, "Failed to make basic blocks!\n");
      valid = false;
      return;
   }

#if defined(rs6000_ibm_aix4_1) || defined(arch_x86_64)
   
   // LIVENESS ANALYSIS CODE STARTS
   

   BPatch_basicBlock **blocks = new BPatch_basicBlock *[allBlocks.size()];
   allBlocks.elements(blocks);

   // Initializes the gen kill set for all blocks in the CFG
   for (unsigned int i = 0; i < allBlocks.size(); i++)
     {
       (blocks[i]->lowlevel_block())->initRegisterGenKill();
     }
   
   bool change = true;
   
   //  Does fixed point iteration to figure out the in out sets 
   do {
     change = false;
     for (unsigned int i = 0; i < allBlocks.size(); i++) {
       if ((blocks[i]->lowlevel_block())->updateRegisterInOut(false)) 
	 change = true;
     }
   } while (change);
  
   change = true;

   // Same thing for floating point
   do {
     change = false;
     for (unsigned int i = 0; i < allBlocks.size(); i++) {
       if ((blocks[i]->lowlevel_block())->updateRegisterInOut(true)) 
	 change = true;
     }
   } while (change);

   delete[] blocks;
   
   // LIVENESS ANALYSIS CODE STOPS
  

#endif 

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
		BPatch_point *bP = edges[j]->getPoint();
		if (!bP) {
                    fprintf(stderr, "ERROR: exit edge had no inst point\n");
		}
                else {
                    bP->overrideType(BPatch_locLoopExit);
                    bP->setLoop(loop);
                    points->push_back(bP);
                }
		
            }
    }
}

BPatch_Vector<BPatch_point*> *
BPatch_flowGraph::findLoopInstPointsInt(const BPatch_procedureLocation loc, 
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
		ll_func()->prettyName().c_str(), loop);

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
		BPatch_point *iP = edges[i]->getPoint();
		if (!iP) {
		  fprintf(stderr, "ERROR: failed to find loop entry point!\n");
		} 
                else {
                   iP->overrideType(BPatch_locLoopEntry);
                   iP->setLoop(loop);
                   points->push_back(iP);
                }
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
	if (!points->size())
	  fprintf(stderr, "ERROR: failed to find loop exit points!\n");

        break;
    }

    case BPatch_locLoopStartIter: {
        if (DEBUG_LOOP) fprintf(stderr,"loop start iter\n");

        // instrument the head of the loop
        BPatch_point *p;
        void *addr = (void*)loop->getLoopHead()->getStartAddress();
        p = BPatch_point::createInstructionInstPoint(getBProcess(), addr, func_);
        p->overrideType(BPatch_locLoopStartIter);
	p->setLoop(loop);
	points->push_back(p);

        break;
    }

    case BPatch_locLoopEndIter: {
        if (DEBUG_LOOP) fprintf(stderr,"loop end iter\n");

        // point for the backedge of this loop 
        BPatch_edge *edge = loop->backEdge;
        if (DEBUG_LOOP) edge->dump();
	BPatch_point *iP = edge->getPoint();
	iP->overrideType(BPatch_locLoopEndIter);
	iP->setLoop(loop);
        points->push_back(iP);

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

void BPatch_flowGraph::BPatch_flowGraph_dtor()
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
   func_->cfg = NULL;
}

bool
BPatch_flowGraph::getAllBasicBlocksInt(BPatch_Set<BPatch_basicBlock*>& abb)
{
   BPatch_basicBlock** belements =
      new BPatch_basicBlock* [allBlocks.size()];
   
   allBlocks.elements(belements);
   
   for (unsigned int i=0;i<allBlocks.size(); i++)
      abb += belements[i];
   
   delete[] belements;
   return true;
}

// this is the method that returns the set of entry points
// basic blocks, to the control flow graph. Actually, there must be
// only one entry point to each control flow graph but the definition
// given API specifications say there might be more.
bool
BPatch_flowGraph::getEntryBasicBlockInt(BPatch_Vector<BPatch_basicBlock*>& ebb) 
{
   BPatch_basicBlock *bb;
   int_function *func = ll_func();
   for (unsigned i=0; i<func->blocks().size(); i++)
   {
      if (func->blocks()[i]->isEntryBlock())
      {
         bb = (BPatch_basicBlock *) func->blocks()[i]->getHighLevelBlock();
         ebb.push_back(bb);
      }
   }
   return true;
}

// this method returns the set of basic blocks that are the
// exit basic blocks from the control flow graph. That is those
// are the basic blocks that contains the instruction for
// returning from the function
bool 
BPatch_flowGraph::getExitBasicBlockInt(BPatch_Vector<BPatch_basicBlock*>& nbb)
{
   BPatch_basicBlock *bb;
   int_function *func = ll_func();
   for (unsigned i=0; i<func->blocks().size(); i++)
   {
      if (func->blocks()[i]->isExitBlock())
      {
         bb = (BPatch_basicBlock *) func->blocks()[i]->getHighLevelBlock();
         nbb.push_back(bb);
      }
   }
   return true;
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
            
            unsigned long l1start = l1->backEdge->target->getStartAddress();
	    unsigned long l2start = l2->backEdge->target->getStartAddress();   
	    unsigned long l1end   = l1->backEdge->source->getLastInsnAddress();
	    unsigned long l2end   = l2->backEdge->source->getLastInsnAddress();
            
            // if l2 is inside l1
            if (l1start < l2start && l2end < l1end) {
                // l1 contains l2
                l1->containedLoops += l2;
                
                // l2 has no parent, l1 is best so far
                if (!l2->parent) 
                    l2->parent = l1;
                else 
                   // if l1 is closer to l2 than l2's existing parent
                    if (l1start > l2->parent->getLoopHead()->getStartAddress()) 
                        l2->parent = l1;
            }
        }
    }
    
    delete[] allLoops;
}

// this methods returns the loop objects that exist in the control flow
// grap. It returns a set. And if there are no loops, then it returns the empty
// set. not NULL. 
void BPatch_flowGraph::getLoopsByNestingLevel(BPatch_Vector<BPatch_loop*>& lbb,
                                              bool outerMostOnly)
{
   if (!loops) {
      fillDominatorInfo();
      createEdges();
      createLoops();
   }
    
   BPatch_loop **lelements = new BPatch_loop* [loops->size()];
   
   loops->elements(lelements);
   
   for (unsigned i=0; i < loops->size(); i++)
   {
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
   }
   delete[] lelements;
   return;
}


// get all the loops in this flow graph
bool 
BPatch_flowGraph::getLoopsInt(BPatch_Vector<BPatch_basicBlockLoop*>& lbb)
{
    getLoopsByNestingLevel(lbb, false);
    return true;
}

// get the outermost loops in this flow graph
bool 
BPatch_flowGraph::getOuterLoopsInt(BPatch_Vector<BPatch_basicBlockLoop*>& lbb)
{
    getLoopsByNestingLevel(lbb, true);
    return true;
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
    assert(ll_func());
    const pdvector< int_basicBlock* > &iblocks	= ll_func()->blocks();
    for( unsigned int i = 0; i < iblocks.size(); i++ )
    {
       BPatch_basicBlock *newblock = new BPatch_basicBlock(iblocks[i], this);
       allBlocks += newblock;
    }
    return true;
}


// This function must be called only after basic blocks have been created
// by calling createBasicBlocks. It computes the source block for each
// basic block. For now, a source block is represented by the starting
// and ending line numbers in the source block for the basic block.

bool BPatch_flowGraph::createSourceBlocksInt() {
    if( isSourceBlockInfoReady ) { return true; }
    
    /* Iterate over every basic block, looking for the lines corresponding to the
       addresses it contains. */
    BPatch_basicBlock ** elements = new BPatch_basicBlock * [ allBlocks.size() ];
    allBlocks.elements( elements );
    
    for( unsigned int i = 0; i < allBlocks.size(); i++ ) {
        BPatch_basicBlock * currentBlock = elements[i];
	
        std::vector< LineInformation::LineNoTuple > lines;
        InstrucIter insnIterator( currentBlock );
        
        for( ; insnIterator.hasMore(); ++insnIterator ) {
            if( getBProcess()->getSourceLines( * insnIterator, lines ) ) {
                // /* DEBUG */ fprintf( stderr, "%s[%d]: 0x%lx\n", __FILE__, __LINE__, * insnIterator );
            }
        }
        
        if( lines.size() != 0 ) {
            if( ! currentBlock->sourceBlocks ) {
                currentBlock->sourceBlocks = new BPatch_Vector< BPatch_sourceBlock * >();
            }
            
            BPatch_Set< unsigned short > lineNos;
            const char * currentSourceFile = lines[0].first;
            for( unsigned int j = 0; j < lines.size(); j++ ) {
                if( strcmp( currentSourceFile, lines[j].first ) == 0 ) {
                    lineNos.insert( lines[j].second );
                    continue;
                }
                
                // /* DEBUG */ fprintf( stderr, "%s[%d]: Inserting %s:", __FILE__, __LINE__, currentSourceFile );
                // /* DEBUG */ BPatch_Set< unsigned short >::iterator iter = lineNos.begin();
                // /* DEBUG */ for( ; iter != lineNos.end(); iter++ ) { fprintf( stderr, " %d", * iter ); }
                // /* DEBUG */ fprintf( stderr, "\n" );
                BPatch_sourceBlock * sourceBlock = new BPatch_sourceBlock( currentSourceFile, lineNos );
                currentBlock->sourceBlocks->push_back( sourceBlock );
                
                /* Wonder why there isn't a clear().  (For that matter, why there isn't a const_iterator
                   or a prefix increment operator for the iterator.) */
                lineNos = BPatch_Set< unsigned short >();
                currentSourceFile = lines[j].first;
                lineNos.insert( lines[j].second );
            }
            if( lineNos.size() != 0 ) {
                // /* DEBUG */ fprintf( stderr, "%s[%d]: Inserting %s:", __FILE__, __LINE__, currentSourceFile );
                // /* DEBUG */ BPatch_Set< unsigned short >::iterator iter = lineNos.begin();
                // /* DEBUG */ for( ; iter != lineNos.end(); iter++ ) { fprintf( stderr, " %d", * iter ); }
                // /* DEBUG */ fprintf( stderr, "\n" );
                
                BPatch_sourceBlock * sourceBlock = new BPatch_sourceBlock( currentSourceFile, lineNos );
                currentBlock->sourceBlocks->push_back( sourceBlock );
            }				
            
        } /* end if we found anything */
    } /* end iteration over all basic blocks */
    
    delete [] elements;
    return true;
} /* end createSourceBlocks() */

//this method fill the dominator information of each basic block
//looking at the control flow edges. It uses a fixed point calculation
//to find the immediate dominator of the basic blocks and the set of
//basic blocks that are immediately dominated by this one.
//Before calling this method all the dominator information
//is going to give incorrect results. So first this function must
//be called to process dominator related fields and methods.
void BPatch_flowGraph::fillDominatorInfoInt()
{
  if(isDominatorInfoReady)
    return;
  isDominatorInfoReady = true;
  
  dominatorCFG domcfg(this);
  domcfg.calcDominators();
}
 
void BPatch_flowGraph::fillPostDominatorInfoInt()
{
  if(isPostDominatorInfoReady)
    return;
  isPostDominatorInfoReady = true;
  
  dominatorCFG domcfg(this);
  domcfg.calcPostDominators();
}


// Add each back edge in the flow graph to the given set. A back edge
// in a flow graph is an edge whose head dominates its tail. 
void BPatch_flowGraph::createEdges()
{
   BPatch_Vector<BPatch_basicBlock *> targs;
   /*
    * Indirect jumps are NOT currently handled correctly
    */
   
   backEdges = new BPatch_Set<BPatch_edge*>;
   
   BPatch_basicBlock **blks = new BPatch_basicBlock*[allBlocks.size()];
   allBlocks.elements(blks);
   
   // for each block in the graph
   for (unsigned i = 0; i < allBlocks.size(); i++) {
      BPatch_basicBlock *source = blks[i];

      targs.clear();
      source->getTargets(targs);
      unsigned numTargs = targs.size();

      // create edges

      Address lastinsnaddr = source->getLastInsnAddress();
      if (lastinsnaddr == 0) {
         fprintf(stderr, "ERROR: 0 addr for block end!\n");
         continue;
      }

      if (numTargs == 1) {
         BPatch_edge *edge;
         edge = new BPatch_edge(source, targs[0], this);

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
            new BPatch_edge(source, targs[0], this); 

         BPatch_edge *edge1 = 
            new BPatch_edge(source, targs[1], this); 

         //             fprintf(stderr, "t2 %2d %2d\n",source->blockNo(),
         //                     targs[0]->blockNo());
         // 	    fprintf(stderr, "t2 %2d %2d\n",source->blockNo(),
         //                     targs[1]->blockNo());

         source->outgoingEdges += edge0;
         source->outgoingEdges += edge1;

         targs[0]->incomingEdges += edge0;
         targs[1]->incomingEdges += edge1;

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
   }
   delete[] blks;
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
   pdvector<int_basicBlock *> blocks;
   BPatch_basicBlock *pred;

   bbSet += backEdge->target;

   if (!bbSet.contains(backEdge->source)) {
      bbSet += backEdge->source;
      stack->push(backEdge->source);
   }

   while (!stack->empty()) {
      BPatch_basicBlock* bb = stack->pop();
      
      blocks.clear();
      bb->iblock->getSources(blocks);
      for (unsigned i=0; i<blocks.size(); i++)
      {
         pred = (BPatch_basicBlock*) blocks[i]->getHighLevelBlock();
         if (!bbSet.contains(pred)) {
            bbSet += pred;
            stack->push(pred);
         }
      }
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

    const pdvector<instPoint*> &instPs = ll_func()->funcCalls();

    for (unsigned i = 0; i < instPs.size(); i++) {
	int_function *f = instPs[i]->findCallee();
        
	if (f != NULL) 
	    insertCalleeIntoLoopHierarchy(f, instPs[i]->addr());
// 	else 
// 	    fprintf(stderr, "BPatch_flowGraph::createLoopHierarchy "
//                     "couldn't find callee by inst point.\n");
    }
}


// try to insert func into the appropriate spot in the loop tree based on
// address ranges. if succesful return true, return false otherwise.
bool 
BPatch_flowGraph::dfsInsertCalleeIntoLoopHierarchy(BPatch_loopTreeNode *node, 
						   int_function *callee,
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
BPatch_flowGraph::insertCalleeIntoLoopHierarchy(int_function *callee,
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
BPatch_flowGraph::getLoopTreeInt() 
{ 
    if (loopRoot == NULL) 
	createLoopHierarchy();
    return loopRoot; 
}


BPatch_loop *
BPatch_flowGraph::findLoopInt(const char *name) 
{ 
    return getLoopTree()->findLoop(name);
}


void 
BPatch_flowGraph::dfsPrintLoops(BPatch_loopTreeNode *n) {

  if (n->loop != NULL) {
      //    pdpair<u_short, u_short> mm = getLoopMinMaxSourceLines(n->loop);
      //  printf("%s (source %d-%d)\n", n->name(), mm.first, mm.second);
        
      printf("%s %s\n", n->name(),ll_func()->prettyName().c_str());
  }

  for (unsigned i = 0; i < n->children.size(); i++) {
      dfsPrintLoops(n->children[i]);
  }
}


void 
BPatch_flowGraph::printLoopsInt()
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
                blocks[i]->getStartAddress(), 
                blocks[i]->getEndAddress());
        
    }

}

bool BPatch_flowGraph::containsDynamicCallsitesInt()
{
   return (ll_func()->getNumDynamicCalls() > 0);
}

process *BPatch_flowGraph::ll_proc() const { 
    return bproc->lowlevel_process();
}

int_function *BPatch_flowGraph::ll_func() const {
    return func_->lowlevel_func();
}
