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

#ifndef _BPatch_loopTreeNode_h_
#define _BPatch_loopTreeNode_h_

#include "BPatch_dll.h"
#include "BPatch_Vector.h"

class BPatch_basicBlockLoop;
class function_base;

/** A class to represent the tree of nested loops and 
 *  callees (functions) in the control flow graph.
 *  @see BPatch_basicBlockLoop
 *  @see BPatch_flowGraph
 */
class BPATCH_DLL_EXPORT BPatch_loopTreeNode {
    friend class BPatch_flowGraph;

 public:
    /** Create a loop tree node for BPatch_basicBlockLoop with name n */
    BPatch_loopTreeNode(BPatch_basicBlockLoop *l, const char *n);

    ~BPatch_loopTreeNode();

    /** A loop node contains a single BPatch_basicBlockLoop instance */
    BPatch_basicBlockLoop *loop;

    /** The BPatch_loopTreeNode instances nested within this loop. */
    BPatch_Vector<BPatch_loopTreeNode *> children;

    /** Return the name of this loop. */
    const char *name();

    // A vector of functions called within the body of this loop (and
    // not the body of sub loops). 
    BPatch_Vector<function_base *> callees;

    // Return the function name of the ith callee. 
    const char *getCalleeName(unsigned int i);
    
    // Return the number of callees contained in this loop's body. 
    unsigned int numCallees();

    // find loop by hierarchical name
    BPatch_basicBlockLoop *findLoop(const char *name);

 private:

    /** name which indicates this loop's relative nesting */
    char *hierarchicalName;
};


#endif /* _BPatch_loopTreeNode_h_ */
