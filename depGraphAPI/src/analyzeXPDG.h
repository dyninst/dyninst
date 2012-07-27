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

#ifndef INTRAFUNCTIONXPDGCREATOR_H_
#define INTRAFUNCTIONXPDGCREATOR_H_

#include <map>
#include "Node.h"
#include "xPDG.h"

class BPatch_function;

using namespace std;

namespace Dyninst {
namespace DepGraphAPI {

/**
 * The tool that creates the Extended Program Dependence Graph (xPDG) associated with a given 
 * function (currently BPatch_function). It creates a Program Dependency Graph (PDG)
 * and a Flow Dependency Graph (FDG) using a PDGAnalyzer and
 * an FDGAnalyzer. These graphs are merged to create an xPDG.
 */
class xPDGAnalyzer {
    typedef BPatch_function Function;

public:
    typedef BPatch_basicBlock Block;
    typedef Node::Ptr NodePtr;
    typedef map<Node*, NodePtr> NodeMap;

private:
    // Extended Program Dependence Graph.
    xPDG::Ptr xpdg;

    // Function  to be analyzed.
    Function *func_;

    // Gets the PDG for this function and copies it into xPDG.
    void mergeWithPDG();
    
    // Gets the FDG for this function, converts it into sub-instruction level, and merges with the xPDG.
    void mergeWithFDG();

public:
    // Creates an xPDGAnalyzer object that will be used to create an xPDG for the given function.
    xPDGAnalyzer(Function *f);

    // Analyze the function fed to the constructor and return its Extended Program Dependence Graph.
    xPDG::Ptr analyze();
};

};
};

#endif /* INTRAFUNCTIONXPDGCREATOR_H_ */
