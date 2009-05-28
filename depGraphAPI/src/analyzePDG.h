/*
 * Copyright (c) 1996-2009 Barton P. Miller
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
