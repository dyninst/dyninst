/*
 * See the dyninst/COPYRIGHT file for copyright information.
 * 
 * We provide the Paradyn Tools (below described as "Paradyn")
 * on an AS IS basis, and do not warrant its validity or performance.
 * We reserve the right to update, modify, or discontinue this
 * software at any time.  We shall have no obligation to supply such
 * updates or modifications or any other form of support to you.
 * 
 * By your use of Paradyn, you understand and agree that we (or any
 * other person or entity with proprietary rights in Paradyn) are
 * under no obligation to provide either maintenance services,
 * update services, notices of latent defects, or correction of
 * defects for Paradyn.
 * 
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

#define BPATCH_FILE

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <string>

#include "common/src/Types.h"
#include "common/src/Vector.h"
#include <unordered_map>
#include "common/src/Pair.h"

#include "BPatch_process.h"
#include "BPatch_edge.h"
#include "BPatch_point.h"

#include "util.h"
#include "image.h"
#include "instPoint.h"
#include "debug.h"

#include "Instruction.h"

#include "BPatch_statement.h"

#include "BPatch_flowGraph.h"
#include "mapped_object.h"
#include "dominator.h"
#include "function.h"

// constructor of the class. It creates the CFG and
// deletes the unreachable code.
BPatch_flowGraph::BPatch_flowGraph(BPatch_function *func,
                                   bool &valid)
  : func_(func), addSpace(func->getAddSpace()), mod(func->getModule()),
    loops(NULL), loopRoot(NULL), isDominatorInfoReady(false),
    isPostDominatorInfoReady(false), isSourceBlockInfoReady(false)
{
  // fill the information of the basic blocks after creating
  // them. The dominator information will also be filled
  isValid_ = valid = true;

  if (!createBasicBlocks()) {
    fprintf(stderr, "Failed to make basic blocks!\n");
    isValid_ = valid = false;
    return;
  }
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
      if (!loop->hasBlock(edges[j]->getTarget())) {
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
    fprintf(stderr,"%s findLoopInstPoints 0x%p\n",
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
      if (!loop->hasBlock(edges[i]->getSource())) {
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
    block_instance *llHead = loop->getLoopHead()->lowlevel_block();

    // TODO FIXME: if we want this to work right we need an underlying loop
    // representation...
    BPatch_point *p = getAddSpace()->findOrCreateBPPoint(func_,
                                                         instPoint::blockEntry(loop->getLoopHead()->ifunc(),
                                                                               llHead),
                                                         BPatch_locBasicBlockEntry);
    p->overrideType(BPatch_locLoopStartIter);
    p->setLoop(loop);
    points->push_back(p);

    break;
  }

  case BPatch_locLoopEndIter: {
    if (DEBUG_LOOP) fprintf(stderr,"loop end iter\n");

    // point for the backedge of this loop
    BPatch_edge *edge = loop->getBackEdge();
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

BPatch_flowGraph::~BPatch_flowGraph()
{
  if (loops) {
     for (std::set<BPatch_loop *>::iterator iter = loops->begin();
          iter != loops->end(); ++iter) {
        delete *iter;
     }
     delete loops;
  }

  for (std::set<BPatch_basicBlock *>::iterator iter = allBlocks.begin();
       iter != allBlocks.end(); ++iter) {
     delete *iter;
  }
  
  if (loopRoot) {
     delete loopRoot;
  }

  func_->cfg = NULL;
}

bool
BPatch_flowGraph::getAllBasicBlocks(std::set<BPatch_basicBlock*>& abb)
{
   std::copy(allBlocks.begin(), allBlocks.end(), std::inserter(abb, abb.end()));
   return true;
}

bool
BPatch_flowGraph::getAllBasicBlocks(BPatch_Set<BPatch_basicBlock*>& abb) {
   std::set<BPatch_basicBlock *> tmp;
   getAllBasicBlocks(tmp);
   std::copy(tmp.begin(), tmp.end(), std::inserter(abb.int_set, abb.int_set.end()));
   return true;
}

// this is the method that returns the set of entry points
// basic blocks, to the control flow graph. Actually, there must be
// only one entry point to each control flow graph but the definition
// given API specifications say there might be more.
bool
BPatch_flowGraph::getEntryBasicBlock(BPatch_Vector<BPatch_basicBlock*>& ebb)
{
  ebb.push_back(findBlock(ll_func()->entryBlock()));

  return true;
}

// this method returns the set of basic blocks that are the
// exit basic blocks from the control flow graph. That is those
// are the basic blocks that contains the instruction for
// returning from the function
bool
BPatch_flowGraph::getExitBasicBlock(BPatch_Vector<BPatch_basicBlock*>& nbb)
{
  /*   for (func_instance::BlockSet::const_iterator iter = ll_func()->exitBlocks().begin();
       iter != ll_func()->exitBlocks().end(); ++iter) { */
  for (PatchFunction::Blockset::const_iterator iter = ll_func()->exitBlocks().begin();
       iter != ll_func()->exitBlocks().end(); ++iter) {
    nbb.push_back(findBlock(SCAST_BI(*iter)));
  }
  return true;
}

void
BPatch_flowGraph::createLoops()
{
   loops = new std::set<BPatch_loop*>;

   for (std::set<BPatch_edge *>::iterator iter = backEdges.begin();
        iter != backEdges.end(); ++iter) {
      assert((*iter) != NULL);
      BPatch_loop *loop = new BPatch_loop(*iter, this);

      // find all basic blocks in the loop and keep a map used
      // to find the nest structure
      findBBForBackEdge(*iter, loop->basicBlocks);

      loops->insert(loop);
  }

  std::vector<BPatch_loop*> dupLoops;

  for (std::set<BPatch_loop *>::iterator iter_1 = loops->begin();
       iter_1 != loops->end(); ++iter_1) {
     for (std::set<BPatch_loop *>::iterator iter_2 = loops->begin(); 
          iter_2 != loops->end(); ++iter_2) {
        // Skip the first one, of course
        if (iter_1 == iter_2) continue;

        // We can't do this as a triangle (e.g, begin <= iter_1 < end, 
        // iter_1 <= iter_2 < end) because we are checking
        // whether l1 contains l2, not the reverse as well. 
        // Also, the set is pointer-sorted, so there's no 
        // structure at all. 

        BPatch_loop *l1 = *iter_1;
        BPatch_loop *l2 = *iter_2;


        // Note that address ranges (as were previously used here)
        // between the target of the back edge and the last instruction
        // of the source of the back edge are insufficient to determine
        // whether a block lies within a loop, given all possible
        // layouts of loops in the address space. Instead, check
        // for set membership.
        //
        // If loop A contains both the head block and the source
        // of the back edge of loop B, it contains loop B (for
        // nested natural loops)
        if(l1->hasBlock(l2->getLoopHead()) &&
           l1->hasBlock(l2->getBackEdge()->getSource()))
        {
           // l1 contains l2
           l1->containedLoops.insert(l2);
           if( l2->hasBlock(l1->getLoopHead()) &&
               l2->hasBlock(l1->getBackEdge()->getSource()) )
           {
              if (l1 < l2) { // merge l2 into l1 if i < j
                 dupLoops.push_back(l2);
                 std::vector<BPatch_edge*> l2edges;
                 l2->getBackEdges(l2edges);
                 l1->addBackEdges(l2edges);
              }
              // Otherwise we'll catch this coming the other way. 
              // For future readers: there _has_ to be a better way
              // of doing this, and I wish you good luck. -- bernat

           }
           else
           {
              // l2 has no parent, l1 is best so far
              if(!l2->parent)
              {
                 l2->parent = l1;
              }
              else
              {
                 // if l1 is closer to l2 than l2's existing parent
                 if(l2->parent->hasBlock(l1->getLoopHead()) &&
                    l2->parent->hasBlock(l1->getBackEdge()->getSource()))
                 {
                    l2->parent = l1;
                 }
              }
            }
        }
     }
  }

  // remove duplicate loops
  for (unsigned idx=0; idx < dupLoops.size(); idx++) {
     // Erase on a key element returns number removed;
     // if this is 1 we removed it from the set and thus need
     // to free the memory.
     if (loops->erase(dupLoops[idx]) != 0) {
        delete dupLoops[idx];
     }
  }
}

// this methods returns the loop objects that exist in the control flow
// grap. It returns a set. And if there are no loops, then it returns the empty
// set. not NULL.
void BPatch_flowGraph::getLoopsByNestingLevel(BPatch_Vector<BPatch_loop*>& lbb,
                                              bool outerMostOnly)
{
  if (!loops) {
    fillDominatorInfo();
    createBackEdges();
    createLoops();
  }

  for (std::set<BPatch_loop *>::iterator iter = loops->begin();
       iter != loops->end(); ++iter) {
     // if we are only getting the outermost loops
     if (outerMostOnly && 
         (*iter)->parent != NULL) continue;

     lbb.push_back(*iter);
  }
  return;
}


// get all the loops in this flow graph
bool
BPatch_flowGraph::getLoops(BPatch_Vector<BPatch_basicBlockLoop*>& lbb)
{
  getLoopsByNestingLevel(lbb, false);
  return true;
}

// get the outermost loops in this flow graph
bool
BPatch_flowGraph::getOuterLoops(BPatch_Vector<BPatch_basicBlockLoop*>& lbb)
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
  // create blocks from block_instances
  const PatchFunction::Blockset&
    iblocks = ll_func()->blocks();
  PatchFunction::Blockset::const_iterator ibIter;
  //for( unsigned int i = 0; i < iblocks.size(); i++ )
  for (ibIter = iblocks.begin();
       ibIter != iblocks.end();
       ibIter++)
    {
      block_instance* iblk = SCAST_BI(*ibIter);
      BPatch_basicBlock *newblock = findBlock(iblk);
      assert(newblock);
      allBlocks.insert(newblock);

      // Insert source/target edges
      /*       const block_instance::edgelist &srcs = iblk->sources();
      //const block_instance::edgelist &srcs = (*ibIter)->sources();
      for (block_instance::edgelist::const_iterator iter = srcs.begin(); iter != srcs.end(); ++iter) {
      */
      const PatchBlock::edgelist &srcs = iblk->sources();
      //const block_instance::edgelist &srcs = (*ibIter)->sources();
      for (PatchBlock::edgelist::const_iterator iter = srcs.begin(); iter != srcs.end(); ++iter) {
        edge_instance* iedge = SCAST_EI(*iter);
        // Skip interprocedural edges
        if (iedge->interproc()) continue;
        BPatch_edge *e = findEdge(iedge);
        newblock->incomingEdges.insert(e);
      }
      // Insert source/target edges
      const PatchBlock::edgelist &trgs = iblk->targets();
      for (PatchBlock::edgelist::const_iterator iter = trgs.begin(); iter != trgs.end(); ++iter) {
        edge_instance* iedge = SCAST_EI(*iter);
        // Skip interprocedural edges
        if (iedge->interproc()) continue;
        BPatch_edge *e = findEdge(iedge);
        newblock->outgoingEdges.insert(e);
      }
    }
  return true;
}


// This function must be called only after basic blocks have been created
// by calling createBasicBlocks. It computes the source block for each
// basic block. For now, a source block is represented by the starting
// and ending line numbers in the source block for the basic block.

bool BPatch_flowGraph::createSourceBlocks() {
  if( isSourceBlockInfoReady ) { return true; }

  /* Iterate over every basic block, looking for the lines corresponding to the
     addresses it contains. */

  for (std::set<BPatch_basicBlock *>::iterator iter = allBlocks.begin();
       iter != allBlocks.end(); ++iter) {
     BPatch_basicBlock * currentBlock = *iter;

    BPatch_Vector<BPatch_statement> lines;
    using namespace Dyninst::InstructionAPI;
    std::vector<std::pair< Instruction::Ptr, Address > > insnsByAddr;
    currentBlock->getInstructions(insnsByAddr);
    for(std::vector<std::pair< Instruction::Ptr, Address > >::const_iterator cur = insnsByAddr.begin();
        cur != insnsByAddr.end();
        ++cur)
      {
        if( getAddSpace()->getSourceLines(cur->second, lines ) ) {
          // /* DEBUG */ fprintf( stderr, "%s[%d]: 0x%lx\n", __FILE__, __LINE__, cur->second );
        }
      }
    if( lines.size() != 0 ) {
      if( ! currentBlock->sourceBlocks ) {
        currentBlock->sourceBlocks = new BPatch_Vector< BPatch_sourceBlock * >();
      }

      std::set< unsigned short > lineNos;
      const char * currentSourceFile = lines[0].fileName();
      for( unsigned int j = 0; j < lines.size(); j++ ) {
        if( strcmp( currentSourceFile, lines[j].fileName() ) == 0 ) {
          lineNos.insert( (unsigned short) lines[j].lineNumber() );
          continue;
        }

        // /* DEBUG */ fprintf( stderr, "%s[%d]: Inserting %s:", __FILE__, __LINE__, currentSourceFile );
        // /* DEBUG */ std::set< unsigned short >::iterator iter = lineNos.begin();
        // /* DEBUG */ for( ; iter != lineNos.end(); iter++ ) { fprintf( stderr, " %d", * iter ); }
        // /* DEBUG */ fprintf( stderr, "\n" );
        BPatch_sourceBlock * sourceBlock = new BPatch_sourceBlock( currentSourceFile, lineNos );
        currentBlock->sourceBlocks->push_back( sourceBlock );

        lineNos.clear();
        currentSourceFile = lines[j].fileName();
        lineNos.insert( (unsigned short)lines[j].lineNumber() );
      }
      if( !lineNos.empty()) {
        // /* DEBUG */ fprintf( stderr, "%s[%d]: Inserting %s:", __FILE__, __LINE__, currentSourceFile );
        // /* DEBUG */ std::set< unsigned short >::iterator iter = lineNos.begin();
        // /* DEBUG */ for( ; iter != lineNos.end(); iter++ ) { fprintf( stderr, " %d", * iter ); }
        // /* DEBUG */ fprintf( stderr, "\n" );

        BPatch_sourceBlock * sourceBlock = new BPatch_sourceBlock( currentSourceFile, lineNos );
        currentBlock->sourceBlocks->push_back( sourceBlock );
      }

    } /* end if we found anything */
  } /* end iteration over all basic blocks */

  isSourceBlockInfoReady = true;
  return true;
} /* end createSourceBlocks() */

//this method fill the dominator information of each basic block
//looking at the control flow edges. It uses a fixed point calculation
//to find the immediate dominator of the basic blocks and the set of
//basic blocks that are immediately dominated by this one.
//Before calling this method all the dominator information
//is going to give incorrect results. So first this function must
//be called to process dominator related fields and methods.
void BPatch_flowGraph::fillDominatorInfo()
{
  if(isDominatorInfoReady)
    return;
  isDominatorInfoReady = true;

  dominatorCFG domcfg(this);
  domcfg.calcDominators();
}

void BPatch_flowGraph::fillPostDominatorInfo()
{
  if(isPostDominatorInfoReady)
    return;
  isPostDominatorInfoReady = true;

  dominatorCFG domcfg(this);
  domcfg.calcPostDominators();
}

// Adds each back edge in the flow graph to the given set. A back edge
// in a flow graph is an edge whose head dominates its tail.
void BPatch_flowGraph::createBackEdges()
{
  /*
   * Indirect jumps are NOT currently handled correctly
   */

   for (std::set<BPatch_basicBlock *>::iterator iter = allBlocks.begin();
        iter != allBlocks.end(); ++iter) {
      BPatch_basicBlock *source = *iter;

    BPatch_Vector<BPatch_edge *> outEdges;
    (*iter)->getOutgoingEdges(outEdges);
    unsigned numTargs = outEdges.size();

    // create edges

    Address lastinsnaddr = source->getLastInsnAddress();
    if (lastinsnaddr == 0) {
      fprintf(stderr, "ERROR: 0 addr for block end!\n");
      continue;
    }

    if (numTargs == 1) {
      //         BPatch_edge *edge = blks[i]->incomingEdges.find();
      //         edge = new BPatch_edge(source, targs[0], this);

      //         targs[0]->incomingEdges += edge;
      //         source->outgoingEdges += edge;

      //             fprintf(stderr, "t1 %2d %2d\n",source->blockNo(),
      //                     targs[0]->blockNo());
      if (outEdges[0]->getTarget()->dominates(source))
         backEdges.insert(outEdges[0]);
    }
    else if (numTargs == 2) {
      //XXX could be an indirect jump with two targets

      // conditional jumps create two edges from a block
      //         BPatch_edge *edge0 =
      //            new BPatch_edge(source, targs[0], this);

      //         BPatch_edge *edge1 =
      //            new BPatch_edge(source, targs[1], this);

      //             fprintf(stderr, "t2 %2d %2d\n",source->blockNo(),
      //                     targs[0]->blockNo());
      //            fprintf(stderr, "t2 %2d %2d\n",source->blockNo(),
      //                     targs[1]->blockNo());

      //         source->outgoingEdges += edge0;
      //         source->outgoingEdges += edge1;

      //         targs[0]->incomingEdges += edge0;
      //         targs[1]->incomingEdges += edge1;

      if (outEdges[0]->getTarget()->dominates(source))
         backEdges.insert(outEdges[0]);

      if (outEdges[1]->getTarget()->dominates(source))
         backEdges.insert(outEdges[1]);

      // taken and fall-through edge should not both be back edges
      //            if (targs[0]->dominates(source) && targs[1]->dominates(source)) {
      //                 bperr("Both edge targets can not dominate the source.\n");
      //                edge0->dump();
      //                edge1->dump();
      //            }

      //assert(!(targs[0]->dominates(source) &&
      //targs[1]->dominates(source)));
    }
    else {
      //XXX indirect jumps, set conditional buddy?
      // 7 Dec 06, tugrul
      // create edges for each target even if there are more than two
      // the last instruction of this block is an indirect jump (such as a switch statement)
      //        BPatch_edge *edge;
      for(unsigned j=0; j<numTargs; j++) {
        // create edge between source and this target
        //        edge = new BPatch_edge(source, targs[j], this);

        //        targs[j]->incomingEdges += edge;
        //        source->outgoingEdges += edge;

        // update backEdges if target already dominates source
        if (outEdges[j]->getTarget()->dominates(source))
           backEdges.insert(outEdges[j]);
      }
    }
  }
}

// this method is used to find the basic blocks contained by the loop
// defined by a backedge. The tail of the backedge is the starting point and
// the predecessors of the tail is inserted as a member of the loop.
// then the predecessors of the newly inserted blocks are also inserted
// until the head of the backedge is in the set(reached).

void BPatch_flowGraph::findBBForBackEdge(BPatch_edge* backEdge,
                                         std::set<BPatch_basicBlock*>& bbSet)
{
  std::stack<BPatch_basicBlock *> work;

  pdvector<block_instance *> blocks;
  BPatch_basicBlock *pred;

  bbSet.insert(backEdge->getTarget());

  if (bbSet.find(backEdge->getSource()) == bbSet.end()) {
     bbSet.insert(backEdge->getSource());
     work.push(backEdge->getSource());
  }

  while (!work.empty()) {
    BPatch_basicBlock* bb = work.top();
    work.pop();

    std::vector<BPatch_basicBlock *> srcs;
    bb->getSources(srcs);

    for (unsigned i=0; i < srcs.size(); i++) {
       pred = srcs[i];
       if (bbSet.find(pred) == bbSet.end()) {
          bbSet.insert(pred);
          work.push(pred);
       }
    }
  }
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


struct loop_sort {
   bool operator()(BPatch_loop *l, BPatch_loop *r) const {
      return l->getLoopHead()->getStartAddress() < r->getLoopHead()->getStartAddress(); 
   }
};

void dfsCreateLoopHierarchy(BPatch_loopTreeNode * parent,
                            BPatch_Vector<BPatch_loop *> &loops,
                            std::string level)
{
  for (unsigned int i = 0; i < loops.size (); i++) {
    // loop name is hierarchical level
    std::string clevel = (level != "")
      ? level + "." + utos(i+1)
      : utos(i+1);

    // add new tree nodes to parent
    BPatch_loopTreeNode * child =
      new BPatch_loopTreeNode(loops[i],
                              (std::string("loop_"+clevel)).c_str());

    parent->children.push_back(child);

    // recurse with this child's outer loops
    BPatch_Vector<BPatch_loop*> outerLoops;
    loops[i]->getOuterLoops(outerLoops);
    loop_sort l;
    std::sort(outerLoops.begin(), outerLoops.end(), l);

    dfsCreateLoopHierarchy(child, outerLoops, clevel);
  }
}


void BPatch_flowGraph::createLoopHierarchy()
{
  loopRoot = new BPatch_loopTreeNode(NULL, NULL);

  BPatch_Vector<BPatch_loop *> outerLoops;
  getOuterLoops(outerLoops);

  loop_sort l;
  std::sort(outerLoops.begin(), outerLoops.end(), l);

  dfsCreateLoopHierarchy(loopRoot, outerLoops, "");

  const PatchFunction::Blockset &blocks = ll_func()->blocks();
  for (PatchFunction::Blockset::const_iterator iter = blocks.begin(); iter != blocks.end(); ++iter) {
    block_instance* iblk = SCAST_BI(*iter);
    func_instance *callee = iblk->callee();
    if (callee) {
      insertCalleeIntoLoopHierarchy(callee, iblk->last());
    }
  }
}


// try to insert func into the appropriate spot in the loop tree based on
// address ranges. if succesful return true, return false otherwise.
bool BPatch_flowGraph::dfsInsertCalleeIntoLoopHierarchy(BPatch_loopTreeNode *node,
                                                        func_instance *callee,
                                                        unsigned long addr)
{
  // if this node contains func then insert it
  if ((node->loop != NULL) && node->loop->containsAddress(addr)) {
    node->callees.push_back(callee);
    return true;
  }

  // otherwise recur with each of node's children
  bool success = false;

  for (unsigned int i = 0; i < node->children.size(); i++) {
     success |= dfsInsertCalleeIntoLoopHierarchy(node->children[i], callee, addr);
  }

  return success;
}


void BPatch_flowGraph::insertCalleeIntoLoopHierarchy(func_instance *callee,
                                                     unsigned long addr)
{
  // try to insert func into the loop hierarchy
  bool success = dfsInsertCalleeIntoLoopHierarchy(loopRoot, callee, addr);

  // if its not in a loop make it a child of the root
  if (!success) {
    loopRoot->callees.push_back(callee);
  }
}


BPatch_loopTreeNode *BPatch_flowGraph::getLoopTree()
{
  if (loopRoot == NULL)
    createLoopHierarchy();
  return loopRoot;
}


BPatch_loop *BPatch_flowGraph::findLoop(const char *name)
{
  return getLoopTree()->findLoop(name);
}


void BPatch_flowGraph::dfsPrintLoops(BPatch_loopTreeNode *n)
{
  if (n->loop != NULL) {
    //    pdpair<u_short, u_short> mm = getLoopMinMaxSourceLines(n->loop);
    //  printf("%s (source %d-%d)\n", n->name(), mm.first, mm.second);

    printf("%s %s\n", n->name(),ll_func()->prettyName().c_str());
  }

  for (unsigned i = 0; i < n->children.size(); i++) {
    dfsPrintLoops(n->children[i]);
  }
}


void BPatch_flowGraph::printLoops()
{
  dfsPrintLoops(getLoopTree());
}


void BPatch_flowGraph::dump()
{
   for (std::set<BPatch_basicBlock *>::iterator iter = allBlocks.begin();
        iter != allBlocks.end(); ++iter) {
      fprintf(stderr,"[%u 0x%p 0x%p]\n",
              (*iter)->blockNo(),
              (void *)((*iter)->getStartAddress()),
              (void *)((*iter)->getEndAddress()));
      
   }
}

bool BPatch_flowGraph::containsDynamicCallsites()
{
  return (ll_func()->getNumDynamicCalls() > 0);
}

func_instance *BPatch_flowGraph::ll_func() const {
  return func_->lowlevel_func();
}

AddressSpace *BPatch_flowGraph::getllAddSpace() const
{
  return func_->lladdSpace;
}

void BPatch_flowGraph::invalidate()
{
  isValid_ = false;
}

bool BPatch_flowGraph::isValid() { return isValid_; }

BPatch_basicBlock *BPatch_flowGraph::findBlockByAddr(Dyninst::Address addr) {
   block_instance *blk = ll_func()->getBlock(addr);
   if (!blk) return NULL;
   return findBlock(blk);
}

BPatch_basicBlock *BPatch_flowGraph::findBlock(block_instance *inst) {
  std::map<const block_instance *, BPatch_basicBlock *>::const_iterator iter = blockMap_.find(inst);
  if (iter != blockMap_.end()) return iter->second;
  BPatch_basicBlock *block = new BPatch_basicBlock(inst, this);
  blockMap_[inst] = block;
  return block;
}

BPatch_edge *BPatch_flowGraph::findEdge(edge_instance *inst) {
  // This is more complicated.
  std::map<const edge_instance *, BPatch_edge *>::const_iterator iter = edgeMap_.find(inst);
  if (iter != edgeMap_.end()) return iter->second;

  BPatch_edge *edge = new BPatch_edge(inst, this);
  edgeMap_[inst] = edge;
  return edge;
}
