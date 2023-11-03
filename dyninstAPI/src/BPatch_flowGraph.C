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

#include <unordered_map>

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
#include "function.h"
#include "block.h"

#include "PatchCFG.h"

using namespace Dyninst::PatchAPI;

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
   *
   *  2014-10-14 Xiaozhu:
   *  Case (2) becomes irrelevant because under the new loop detection
   *  algorithm, there is going to be only one loop identified.
   *  Case (1) is still a problem.
   */

  if (DEBUG_LOOP)
    fprintf(stderr,"%s findLoopInstPoints 0x%p\n",
            ll_func()->prettyName().c_str(), (void*)loop);

  BPatch_Vector<BPatch_point*> *points = new BPatch_Vector<BPatch_point *>;

  switch (loc) {

  case BPatch_locLoopEntry: {
    if (DEBUG_LOOP) fprintf(stderr,"loop entry\n");

    // return inst points for each edge e where e's target is this
    // loop's entries and e's source is not a block in this loop or
    // its subloops.
    BPatch_Vector<BPatch_basicBlock*> entries;
    loop->getLoopEntries(entries);
    for (auto bit = entries.begin(); bit != entries.end(); ++bit) {
      BPatch_Vector<BPatch_edge*> edges;
      (*bit)->getIncomingEdges(edges);
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

    // instrument the entris of the loop
    BPatch_Vector<BPatch_basicBlock*> entries;
    loop->getLoopEntries(entries);
    for (auto bit = entries.begin(); bit != entries.end(); ++bit) {
      block_instance *llHead = (*bit)->lowlevel_block();

      BPatch_point *p = getAddSpace()->findOrCreateBPPoint(func_,
                                                           instPoint::blockEntry((*bit)->ifunc(),
                                                                                 llHead),
                                                           BPatch_locBasicBlockEntry);
      p->overrideType(BPatch_locLoopStartIter);
      p->setLoop(loop);
      points->push_back(p);
    }

    break;
  }

  case BPatch_locLoopEndIter: {
    if (DEBUG_LOOP) fprintf(stderr,"loop end iter\n");

    // point for the backedge of this loop
    BPatch_Vector<BPatch_edge*> edges;
    loop->getBackEdges(edges);
    for (auto edge : edges) {
      if (DEBUG_LOOP) edge->dump();
      BPatch_point *iP = edge->getPoint();
      iP->overrideType(BPatch_locLoopEndIter);
      iP->setLoop(loop);
      points->push_back(iP);
    }
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
    loops = new set<BPatch_basicBlockLoop*>;
    vector<PatchLoop*> patch_loops;    
    ll_func()->getLoops(patch_loops);
    
    // Create all the PatchLoop objects in the function
    for (auto lit = patch_loops.begin(); lit != patch_loops.end(); ++lit) {
        BPatch_basicBlockLoop* pl = new BPatch_basicBlockLoop(this, *lit);
	_loop_map[*lit] = pl;
        loops->insert(pl);
    }

    // Build nesting relations among loops
    for (auto lit = patch_loops.begin(); lit != patch_loops.end(); ++lit) {
         PatchLoop* l = *lit;
         BPatch_basicBlockLoop *pl = _loop_map[l];
	 // set parent pointer
         if (l->parent != NULL)
	     pl->parent = _loop_map[l->parent];
	 // set contained loop vector
         vector<PatchLoop*> containedLoops;
	 l->getContainedLoops(containedLoops);
	 for (auto lit2 = containedLoops.begin(); lit2 != containedLoops.end(); ++lit2)
	     pl->containedLoops.insert(_loop_map[*lit2]);
    }     

}

// this methods returns the loop objects that exist in the control flow
// grap. It returns a set. And if there are no loops, then it returns the empty
// set. not NULL.
void BPatch_flowGraph::getLoopsByNestingLevel(BPatch_Vector<BPatch_loop*>& lbb,
                                              bool outerMostOnly)
{
  if (!loops) {
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
//the edges between basic blocks.
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
    std::vector<std::pair< Instruction, Address > > insnsByAddr;
    currentBlock->getInstructions(insnsByAddr);
    for(auto cur = insnsByAddr.begin();
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
  // Fill immediate dominator info
  for (auto bit = allBlocks.begin(); bit != allBlocks.end(); ++bit) {
      PatchBlock* b = (*bit)->lowlevel_block();
      PatchBlock* imd = ll_func()->getImmediateDominator(b);
      if (imd == NULL)
          (*bit)->immediateDominator = NULL;
      else
          (*bit)->immediateDominator = findBlock(SCAST_BI(imd));
  }
  // Fill immediate dominates info
  for (auto bit = allBlocks.begin(); bit != allBlocks.end(); ++bit) {
      PatchBlock* b = (*bit)->lowlevel_block();
      set<PatchBlock*> dominates;
      ll_func()->getImmediateDominates(b, dominates);
      (*bit)->immediateDominates = new set<BPatch_basicBlock*>;
      for (auto dit = dominates.begin(); dit != dominates.end(); ++dit)
          (*bit)->immediateDominates->insert(findBlock(SCAST_BI(*dit)));
  }

  isDominatorInfoReady = true;

}

void BPatch_flowGraph::fillPostDominatorInfo()
{
  if(isPostDominatorInfoReady)
    return;
  // Fill immediate post-dominator info
  for (auto bit = allBlocks.begin(); bit != allBlocks.end(); ++bit) {
      PatchBlock* b = (*bit)->lowlevel_block();
      PatchBlock* imd = ll_func()->getImmediatePostDominator(b);
      if (imd == NULL)
          (*bit)->immediateDominator = NULL;
      else
          (*bit)->immediateDominator = findBlock(SCAST_BI(imd));
  }
  // Fill immediate post-dominates info
  for (auto bit = allBlocks.begin(); bit != allBlocks.end(); ++bit) {
      PatchBlock* b = (*bit)->lowlevel_block();
      set<PatchBlock*> postDominates;
      ll_func()->getImmediatePostDominates(b, postDominates);
      (*bit)->immediatePostDominates = new set<BPatch_basicBlock*>;
      for (auto dit = postDominates.begin(); dit != postDominates.end(); ++dit)
          (*bit)->immediatePostDominates->insert(findBlock(SCAST_BI(*dit)));
  }

  isPostDominatorInfoReady = true;

}


BPatch_loopTreeNode *BPatch_flowGraph::getLoopTree()
{
  if (loopRoot == NULL) {
      if (!loops) createLoops();
      loopRoot = new BPatch_loopTreeNode(this, ll_func()->getLoopTree(), _loop_map);
  }
  return loopRoot;
}


BPatch_loop *BPatch_flowGraph::findLoop(const char *name)
{
  return getLoopTree()->findLoop(name);
}


void BPatch_flowGraph::dfsPrintLoops(BPatch_loopTreeNode *n)
{
  if (n->loop != NULL) {
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
      fprintf(stderr,"[%d 0x%p 0x%p]\n",
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
