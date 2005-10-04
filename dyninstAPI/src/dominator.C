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

#include "dyninstAPI/src/util.h"
#include "dyninstAPI/src/dominator.h"
#include "dyninstAPI/h/BPatch_basicBlock.h"
#include "dyninstAPI/h/BPatch_flowGraph.h"

dominatorBB::dominatorBB(BPatch_basicBlock *bb, dominatorCFG *dc) :
     dfs_no(-1),
     size(1),
     immDom(NULL),
     ancestor(dc->nullNode),
     parent(NULL),
     child(dc->nullNode),
     bpatchBlock(bb),
     dom_cfg(dc)
{
   semiDom = this;
   label = this;
   if (bb)
      dc->map[bb->blockNo()] = this;
}

dominatorBB::~dominatorBB() 
{
}

/**
 * Build a CFG on top of the BPatch_flowGraph that matches
 * the original, but with an extra entry block and using
 * dominatorBB's instead of BPatch_basicBlocks.
 **/
void dominatorBB::dominatorPredAndSucc() {
   unsigned i;

   if (!bpatchBlock)
      return;

   //Predecessors
   BPatch_Vector<BPatch_basicBlock*> blocks;
   bpatchBlock->getSources(blocks);
   for (i=0; i<blocks.size(); i++)
   {
      dominatorBB *p = dom_cfg->bpatchToDomBB(blocks[i]);
      assert(p);
      pred.push_back(p);
   }

   if (bpatchBlock->isEntryBlock() || !blocks.size()) {
      dom_cfg->entryBlock->succ.push_back(this);
      pred.push_back(dom_cfg->entryBlock);
   }

   //Successors
   blocks.clear();
   bpatchBlock->getTargets(blocks);
   for (i=0; i<blocks.size(); i++)
   {
      dominatorBB *s = dom_cfg->bpatchToDomBB(blocks[i]);
      assert(s);
      succ.push_back(s);
   }
}

/**
 * Build an inverted CFG for doing post-dominator analysis.  Running
 * the dominator analysis on this code will actually produce post-dominators
 **/
void dominatorBB::postDominatorPredAndSucc() {
   unsigned i;

   if (!bpatchBlock)
      return;

   //Predecessors
   BPatch_Vector<BPatch_basicBlock*> blocks;
   bpatchBlock->getTargets(blocks);
   for (i=0; i<blocks.size(); i++)
   {
      dominatorBB *p = dom_cfg->bpatchToDomBB(blocks[i]);
      assert(p);
      pred.push_back(p);
   }

   if (bpatchBlock->isExitBlock() || !blocks.size()) {
      dom_cfg->entryBlock->succ.push_back(this);
      pred.push_back(dom_cfg->entryBlock);
   }

   //Successors
   blocks.clear();
   bpatchBlock->getSources(blocks);
   for (i=0; i<blocks.size(); i++)
   {
      dominatorBB *s = dom_cfg->bpatchToDomBB(blocks[i]);
      assert(s);
      succ.push_back(s);
   }
}

dominatorBB *dominatorBB::eval() {
   if (ancestor == dom_cfg->nullNode)
      return label;
   compress();
   if (ancestor->label->sdno() >= label->sdno())
      return label;
   else
      return ancestor->label;
}

void dominatorBB::compress() {
   if (ancestor->ancestor == dom_cfg->nullNode)
      return;
   ancestor->compress();
   if (ancestor->label->sdno() < label->sdno())
      label = ancestor->label;
   ancestor = ancestor->ancestor;
}

int dominatorBB::sdno() { 
   return semiDom->dfs_no; 
}

dominatorCFG::dominatorCFG(BPatch_flowGraph *flowgraph) :
   map(uiHash),
   fg(flowgraph),
   currentDepthNo(0)
{
   nullNode = new dominatorBB(NULL, this);
   nullNode->ancestor = nullNode->child = nullNode;
   nullNode->size = 0;
   //Create a new dominatorBB object for each basic block
   entryBlock = new dominatorBB(NULL, this);
   all_blocks.push_back(entryBlock);

   BPatch_Set<BPatch_basicBlock *, BPatch_basicBlock::compare>::iterator iter;
   for (iter = fg->allBlocks.begin(); iter != fg->allBlocks.end(); iter++)
   {
      dominatorBB *newbb = new dominatorBB(*iter, this);
      all_blocks.push_back(newbb);
   }
}

dominatorCFG::~dominatorCFG() {
   for (unsigned i=0; i < all_blocks.size(); i++)
      delete all_blocks[i];
   delete nullNode;
}

void dominatorCFG::calcDominators() {
   //fill in predecessor and successors
   unsigned i;
   for (i=0; i<all_blocks.size(); i++)
      all_blocks[i]->dominatorPredAndSucc();

   //Perform main computation
   performComputation();

   //Store results
   for (i=0; i<all_blocks.size(); i++) 
   {
      dominatorBB *bb = all_blocks[i];
      if (!bb || !bb->bpatchBlock || 
          !bb->immDom || !bb->immDom->bpatchBlock)
         continue;

      BPatch_basicBlock *immDom = bb->immDom->bpatchBlock;
      BPatch_basicBlock *block = bb->bpatchBlock;

      block->immediateDominator = immDom;
      if (!immDom->immediateDominates)
         immDom->immediateDominates = new BPatch_Set<BPatch_basicBlock *>;
      immDom->immediateDominates->insert(block);
   }   
}

void dominatorCFG::calcPostDominators() {
   unsigned i;
   //fill in predecessor and successors
   for (i=0; i<all_blocks.size(); i++)
      all_blocks[i]->postDominatorPredAndSucc();

   if (!entryBlock->succ.size())
   {
      //The function doesn't have an exit block
      return;
   }

   //Perform main computation
   performComputation();

   //Store results
   for (i=0; i<all_blocks.size(); i++) 
   {
      dominatorBB *bb = all_blocks[i];
      if (!bb || !bb->bpatchBlock || 
          !bb->immDom || !bb->immDom->bpatchBlock)
         continue;

      BPatch_basicBlock *immDom = bb->immDom->bpatchBlock;
      BPatch_basicBlock *block = bb->bpatchBlock;

      block->immediatePostDominator = immDom;
      if (!immDom->immediatePostDominates)
         immDom->immediatePostDominates = new BPatch_Set<BPatch_basicBlock *>;
      immDom->immediatePostDominates->insert(block);
   }   
}

void dominatorCFG::performComputation() {
   unsigned i, j;
   depthFirstSearch(entryBlock);
   for (i = sorted_blocks.size()-1; i > 0; i--)
   {
      dominatorBB *block = sorted_blocks[i];
      dominatorBB *parent = block->parent;
      if (block->dfs_no == -1)
         //Easy to get when dealing with un-reachable code
         continue;

      for (j = 0; j < block->pred.size(); j++)
      {
         dominatorBB *pred = block->pred[j]->eval();
         if (pred->sdno() < block->sdno())
            block->semiDom = pred->semiDom;
      }

      block->semiDom->bucket += block;
      link(parent, block);
      
      while (!parent->bucket.empty())
      {
         dominatorBB *u, *v;
         parent->bucket.extract(v);
         u = v->eval();
         if (u->sdno() < v->sdno())
            v->immDom = u;
         else
            v->immDom = parent;
      }
   }

   for (i = 1; i < sorted_blocks.size(); i++) {
      dominatorBB *block = sorted_blocks[i];
      if (block->immDom != block->semiDom)
         block->immDom = block->immDom->immDom;
   }

   storeDominatorResults();
}

void dominatorCFG::storeDominatorResults() {
}

void dominatorCFG::link(dominatorBB *parent, dominatorBB *block) {
   dominatorBB *s = block;
   while (block->label->sdno() < s->child->label->sdno()) {
      if (s->size + s->child->child->size >= 2*s->child->size) {
         s->child->ancestor = s;
         s->child = s->child->child;
      }
      else {
         s->child->size = s->size;
         s->ancestor = s->child;
         s = s->child;
      }
   }

   s->label = block->label;
   parent->size += block->size;
   if (parent->size < 2 * block->size) {
      dominatorBB *tmp = s;
      s = parent->child;
      parent->child = tmp;
   }

   while (s != nullNode) {
      s->ancestor = parent;
      s = s->child;
   }
}

dominatorBB *dominatorCFG::bpatchToDomBB(BPatch_basicBlock *bb) {
   return map[bb->blockNo()];
}

void dominatorCFG::depthFirstSearch(dominatorBB *v) {
   v->dfs_no = currentDepthNo++;
   sorted_blocks.push_back(v);
   v->semiDom = v;
   for (unsigned i=0; i<v->succ.size(); i++) 
   {
      dominatorBB* successor = v->succ[i];
      if (successor->dfs_no != -1)
         continue;
      successor->parent = v;
      depthFirstSearch(successor);
   }
}
