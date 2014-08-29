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

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <string>

#include <unordered_map>
#include <stack>
#include "CFG.h"
#include "Loop.h"
#include "LoopAnalyzer.h"
#include "dominator.h"

using namespace Dyninst;
using namespace Dyninst::ParseAPI;

// constructor of the class. It creates the CFG and
LoopAnalyzer::LoopAnalyzer(Function *f)
  : func(f) 
{
}


bool LoopAnalyzer::IsDuplicateLoop(Loop *l2) {
    for (auto lit = loops.begin(); lit != loops.end(); ++lit) {
        Loop *l1 = *lit;
        if(l1->hasBlock(l2->getLoopHead()) && l1->hasBlock(l2->getBackEdge()->src())
	   && l2->hasBlock(l1->getLoopHead()) && l2->hasBlock(l1->getBackEdge()->src()))
	   return true;
    }
    return false;
}

void
LoopAnalyzer::createLoops()
{
   loops.clear();
   for (auto iter = backEdges.begin(); iter != backEdges.end(); ++iter) {
      assert((*iter) != NULL);
      Loop *loop = new Loop(*iter, func);

      // find all basic blocks in the loop and keep a map used
      // to find the nest structure
      findBBForBackEdge(*iter, loop->basicBlocks);

      if (IsDuplicateLoop(loop)) {
          delete loop;
      } else {
          loops.insert(loop);
      }
  }

  func->_loops.insert(loops.begin(), loops.end());


  //Build loop nesting relations
  for (auto iter_1 = loops.begin(); iter_1 != loops.end(); ++iter_1) {
     for (auto iter_2 = loops.begin(); iter_2 != loops.end(); ++iter_2) {
        // Skip the first one, of course
        if (iter_1 == iter_2) continue;

        // We can't do this as a triangle (e.g, begin <= iter_1 < end, 
        // iter_1 <= iter_2 < end) because we are checking
        // whether l1 contains l2, not the reverse as well. 
        // Also, the set is pointer-sorted, so there's no 
        // structure at all. 

        Loop *l1 = *iter_1;
        Loop *l2 = *iter_2;


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
           l1->hasBlock(l2->getBackEdge()->src()))
        {
           // l1 contains l2
           l1->containedLoops.insert(l2);
           if( l2->hasBlock(l1->getLoopHead()) &&
               l2->hasBlock(l1->getBackEdge()->src()) )
           {

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
                    l2->parent->hasBlock(l1->getBackEdge()->src()))
                 {
                    l2->parent = l1;
                 }
              }
            }
        }
     }
  }

}


//this method fill the dominator information of each basic block
//looking at the control flow edges. It uses a fixed point calculation
//to find the immediate dominator of the basic blocks and the set of
//basic blocks that are immediately dominated by this one.
//Before calling this method all the dominator information
//is going to give incorrect results. So first this function must
//be called to process dominator related fields and methods.
void LoopAnalyzer::fillDominatorInfo()
{

  dominatorCFG domcfg(func);
  domcfg.calcDominators();
}

void LoopAnalyzer::fillPostDominatorInfo()
{
  dominatorCFG domcfg(func);
  domcfg.calcPostDominators();
}

// Adds each back edge in the flow graph to the given set. A back edge
// in a flow graph is an edge whose head dominates its tail.
void LoopAnalyzer::createBackEdges()
{
  /*
   * Indirect jumps are NOT currently handled correctly
   */

   for (auto bit = func->blocks().begin(); bit != func->blocks().end(); ++bit) {
      Block *source = *bit;
      for (auto eit = source->targets().begin(); eit != source->targets().end(); ++eit) {
          Edge *e = *eit;	  
          if (e->trg()->dominates(func,source))
	      backEdges.insert(e);

      }
   }
}

// this method is used to find the basic blocks contained by the loop
// defined by a backedge. The tail of the backedge is the starting point and
// the predecessors of the tail is inserted as a member of the loop.
// then the predecessors of the newly inserted blocks are also inserted
// until the head of the backedge is in the set(reached).

void LoopAnalyzer::findBBForBackEdge(Edge* backEdge, std::set<Block*>& bbSet)
{
  std::stack<Block *> work;

  Block *pred;

  bbSet.insert(backEdge->trg());

  if (bbSet.find(backEdge->src()) == bbSet.end()) {
     bbSet.insert(backEdge->src());
     work.push(backEdge->src());
  }

  while (!work.empty()) {
    Block* bb = work.top();
    work.pop();

   for (auto eit = bb->sources().begin(); eit != bb->sources().end(); ++eit) {
       if ((*eit)->interproc()) continue;
       pred = (*eit)->src();
       if (bbSet.find(pred) == bbSet.end()) {
          bbSet.insert(pred);
          work.push(pred);
       }
    }
  }
}

// sort blocks by address ascending
void bsort_loops_addr_asc(vector<Loop*> &v)
{
  if (v.size()==0) return;
  for (unsigned i=0; i < v.size()-1; i++)
    for (unsigned j=0; j < v.size()-1-i; j++)
      if (v[j+1]->getLoopHead()->start()
          < v[j]->getLoopHead()->start()) {
        Loop *tmp = v[j];
        v[j] = v[j+1];
        v[j+1] = tmp;
      }
}


struct loop_sort {
   bool operator()(Loop *l, Loop *r) const {
      return l->getLoopHead()->start() < r->getLoopHead()->start(); 
   }
};

void LoopAnalyzer::dfsCreateLoopHierarchy(LoopTreeNode * parent,
                            vector<Loop *> &loops,
                            std::string level)
{
  for (unsigned int i = 0; i < loops.size(); i++) {
    // loop name is hierarchical level
    std::string clevel = (level != "")
      ? level + "." + utos(i+1)
      : utos(i+1);

    // add new tree nodes to parent
    LoopTreeNode * child =
      new LoopTreeNode(loops[i], (std::string("loop_"+clevel)).c_str());

    parent->children.push_back(child);

    // recurse with this child's outer loops
    vector<Loop*> outerLoops;
    loops[i]->getOuterLoops(outerLoops);
    loop_sort l;
    std::sort(outerLoops.begin(), outerLoops.end(), l);
    dfsCreateLoopHierarchy(child, outerLoops, clevel);
  }
}


void LoopAnalyzer::createLoopHierarchy()
{
  LoopTreeNode* loopRoot = new LoopTreeNode(NULL, NULL);
  func->_loop_root = loopRoot;

  vector<Loop *> outerLoops;
  for (auto lit = loops.begin(); lit != loops.end(); ++lit)
      if ((*lit)->parent == NULL)
          outerLoops.push_back(*lit);

  loop_sort l;
  std::sort(outerLoops.begin(), outerLoops.end(), l);
  // Recursively build the loop nesting tree
  dfsCreateLoopHierarchy(loopRoot, outerLoops, string(""));

  // Enumerate every basic blocks in the functions to find all create 
  // call information for each loop
  for (auto bit = func->blocks().begin(); bit != func->blocks().end(); ++bit) {
    Block *b = *bit;
    for (auto eit = b->targets().begin(); eit != b->targets().end(); ++eit) {
      // Can tail call happen here?
      if ((*eit)->type() == CALL) {
          Block *target = (*eit)->trg();
	  vector<Function*> callees;
	  target->getFuncs(callees);
	  for (auto fit = callees.begin(); fit != callees.end(); ++fit)
	      insertCalleeIntoLoopHierarchy(*fit, b->last());
      }
    }
  }
}


// try to insert func into the appropriate spot in the loop tree based on
// address ranges. if succesful return true, return false otherwise.
bool LoopAnalyzer::dfsInsertCalleeIntoLoopHierarchy(LoopTreeNode *node,
                                                        Function *callee,
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


void LoopAnalyzer::insertCalleeIntoLoopHierarchy(Function *callee,
                                                     unsigned long addr)
{
  // try to insert func into the loop hierarchy
  bool success = dfsInsertCalleeIntoLoopHierarchy(func->_loop_root, callee, addr);

  // if its not in a loop make it a child of the root
  if (!success) {
    func->_loop_root->callees.push_back(callee);
  }
}



bool LoopAnalyzer::analyzeLoops() {
    fillDominatorInfo();
    createBackEdges();
    createLoops();
    createLoopHierarchy();
    return true;
}
