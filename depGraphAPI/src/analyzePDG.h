/*
 * Copyright (c) 1996-2011 Barton P. Miller
 * 
 * We provide the Paradyn Parallel Performance Tools (below
 * described as "Paradyn") on an AS IS basis, and do not warrant its
 * validity or performance.  We reserve the right to update, modify,
 * or discontinue this software at any time.  We shall have no
 * obligation to supply such updates or modifications or any other
 * form of support to you.
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

#ifndef INTRAFUNCTIONPDGCREATOR_H_
#define INTRAFUNCTIONPDGCREATOR_H_

#include <map>
#include "Graph.h"
#include "Node.h"
#include "PDG.h"

class BPatch_function;

namespace Dyninst {
namespace DepGraphAPI {

// The tool that creates the Program Dependence Graph (PDG) associated with a given 
// function (currently BPatch_function). It creates a Data Dependency Graph (DDG)
// and a Control Dependency Graph (CDG). These graphs are merged to create an PDG. 
class PDGAnalyzer {
    friend class xPDGAnalyzer;

private:
    typedef BPatch_function Function;
    typedef BPatch_basicBlock Block;
    typedef std::set<Block *> BlockSet;
public:
    typedef Node::Ptr NodePtr;
    typedef std::map<Node*, NodePtr> NodeMap;

private:
    // Program Dependence Graph.
    PDG::Ptr pdg;
    
    // Function to be analyzed.
    Function *func_;

    // Gets the DDG for this function and copies it into PDG.
    void mergeWithDDG();

    // Gets the CDG for this function, converts it into sub-instruction level, and merges with the PDG.
    void mergeWithCDG();

protected:
    
    // Creates edges from given source to all nodes between targetsBegin and targetsEnd
    static void createEdges(Graph::Ptr graph, Node::Ptr source,
            NodeIterator targetsBegin, NodeIterator targetsEnd);
public:
    // Creates a PDGAnalyzer object that will be used to create a PDG for the given function.
    PDGAnalyzer(Function *f);

    // Analyze the function fed to the constructor and return its Program Dependence Graph. 
    PDG::Ptr analyze();
};

};
};

#endif /* INTRAFUNCTIONPDGCREATOR_H_ */
