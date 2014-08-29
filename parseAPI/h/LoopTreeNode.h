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

#ifndef _LoopTreeNode_h_
#define _LoopTreeNode_h_

#include "CFG.h"
#include "Loop.h"

namespace Dyninst{
namespace ParseAPI{

class LoopAnalyzer;

/** A class to represent the tree of nested loops and 
 *  callees (functions) in the control flow graph.
 *  @see BPatch_basicBlockLoop
 *  @see BPatch_flowGraph
 */

class PARSER_EXPORT LoopTreeNode {
    friend class LoopAnalyzer;

 public:
    // A loop node contains a single Loop instance
    Loop *loop;

    // The LoopTreeNode instances nested within this loop.
    vector<LoopTreeNode *> children;

    //  LoopTreeNode::LoopTreeNode
    //  Create a loop tree node for Loop with name n 
    LoopTreeNode(Loop *l, const char *n);

    //  Destructor
    ~LoopTreeNode();

    //  LoopTreeNode::name
    //  Return the name of this loop. 
    const char * name(); 

    //  LoopTreeNode::getCalleeName
    //  Return the function name of the ith callee. 
    const char * getCalleeName(unsigned int i);

    //  LoopTreeNode::numCallees
    //  Return the number of callees contained in this loop's body. 
    unsigned int numCallees();

    //Returns a vector of the functions called by this loop.
    bool getCallees(vector<Function *> &v);
    

    //  BPatch_loopTreeNode::findLoop
    //  find loop by hierarchical name
    Loop * findLoop(const char *name);

 private:

    /** name which indicates this loop's relative nesting */
    char *hierarchicalName;

    // A vector of functions called within the body of this loop (and
    // not the body of sub loops). 
    vector<Function *> callees;

}; // class LoopTreeNode 

}  // namespace ParseAPI
}  // namespace Dyninst
#endif /* _BPatch_loopTreeNode_h_ */
