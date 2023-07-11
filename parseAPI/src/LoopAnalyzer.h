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

#ifndef _LOOPANALYZER_H_
#define _LOOPANALYZER_H_

#include <string>
#include <set>
#include <map>
#include "Annotatable.h"
#include "CFG.h"

using namespace std;

/** class which finds loops in a function 
  *
  */
namespace Dyninst {
namespace ParseAPI {

//  Implement WMZC algorithm to detect both natural loops 
//  and irreducible loops
//  Reference: "A New Algorithm for Identifying Loops in Decompilation"
//  by Tao Wei, Jian Mao, Wei Zou and Yu Chen
class LoopAnalyzer {
 
  
  const Function *func;
  std::map<Block*, set<Block*> > loop_tree;
  std::map<Block*, Loop*> loops;

  std::map<Block*, Block*> header;  
  std::map<Block*, int> DFSP_pos;
  std::set<Block*> visited;

  Block* WMZC_DFS(Block* b0, int pos);
  void WMZC_TagHead(Block* b, Block* h);
  void FillMoreBackEdges(Loop *loop);
  void dfsCreateLoopHierarchy(LoopTreeNode * parent,
                              vector<Loop *> &loops,
			      std::string level);

public:
  bool analyzeLoops();
  /** create the tree of loops/callees for this flow graph */
  void createLoopHierarchy();
 
  LoopAnalyzer (const Function *f);


  
 /** returns true if the cfg contains dynamic callsites */
//  bool containsDynamicCallsites();


 private:   
  static void findBBForBackEdge(Edge*, std::set<Block*>&);


  bool dfsInsertCalleeIntoLoopHierarchy(LoopTreeNode *node, 
                                        Function *func,
                                        unsigned long addr);

  void insertCalleeIntoLoopHierarchy(Function * func, unsigned long addr);

  void createLoops(Block* cur);

    };
}
}

#endif /* _LOOPANALYZER_H_ */
