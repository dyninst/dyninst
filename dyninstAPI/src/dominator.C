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

#include "dyninstAPI/src/util.h"
#include "dyninstAPI/src/dominator.h"
#include "dyninstAPI/h/BPatch_basicBlock.h"
#include "dyninstAPI/h/BPatch_flowGraph.h"
#include <iostream>
#include <set>
using namespace std;

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
      dc->map_[bb->blockNo()] = this;
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
      if (p) {
          pred.push_back(p);
      }
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
   fg(flowgraph),
   currentDepthNo(0)
{
   //First initialize nullNode since dominatorBB's ctor uses it
   nullNode = NULL;
   nullNode = new dominatorBB(NULL, this);
   nullNode->ancestor = nullNode->child = nullNode;
   nullNode->size = 0;

   //Create a new dominatorBB object for each basic block
   entryBlock = new dominatorBB(NULL, this);
   all_blocks.push_back(entryBlock);

   std::set<BPatch_basicBlock *>::iterator iter;
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
         immDom->immediateDominates = new std::set<BPatch_basicBlock *>;
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
         immDom->immediatePostDominates = new std::set<BPatch_basicBlock *>;
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
      
      block->semiDom->bucket.insert(block);

      link(parent, block);
      
      while (!parent->bucket.empty())
      {
         dominatorBB *u, *v;

         //parent->bucket.extract(v);
#if 0
         std::set<dominatorBB *>::iterator iter = parent->bucket.begin();
         std::advance(iter, parent->bucket.size() / 2);
         v = *iter;
         parent->bucket.erase(iter);
#endif
         v = *(parent->bucket.begin());
         parent->bucket.erase(parent->bucket.begin());

         u = v->eval();
         if (u->sdno() < v->sdno())
            v->immDom = u;
         else
            v->immDom = parent;
      }
   }

   for (i = 1; i < sorted_blocks.size(); i++) {
      dominatorBB *block = sorted_blocks[i];
      if (block->immDom != block->semiDom) {
         if (block->immDom) 
            block->immDom = block->immDom->immDom;
      }
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
   auto iter = map_.find(bb->blockNo());
   if (iter == map_.end()) return NULL;
   return iter->second;
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
