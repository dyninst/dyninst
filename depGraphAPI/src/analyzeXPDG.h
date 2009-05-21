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
