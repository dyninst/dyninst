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

#ifndef _BPatch_basicBlockLoop_h
#define _BPatch_basicBlockLoop_h

#include <stdlib.h>
#include "BPatch_dll.h"
#include "BPatch_Vector.h"
#include "BPatch_Set.h"
#include "BPatch_basicBlock.h"
#include "BPatch_flowGraph.h" 

/** class to represent the loops composed of machine code basic 
  * blocks in the executable (Natural loops)
  *
  * @see BPatch_basicBlock
  * @see BPatch_flowGraph
  */

class BPatch_variableExpr;

class BPATCH_DLL_EXPORT BPatch_basicBlockLoop  {
	friend class BPatch_flowGraph;
	friend std::ostream& operator<<(std::ostream&,BPatch_basicBlockLoop&);

private:
        // the flow graph this loop is part of
        BPatch_flowGraph *flowGraph;

	/** set of loops that are contained (nested) in this loop. */
	BPatch_Set<BPatch_basicBlockLoop*> containedLoops;

	/** the basic blocks in the loop */
	BPatch_Set<BPatch_basicBlock*> basicBlocks;

	BPatch_edge *backEdge;

public:
	/** If loop which directly encloses this loop. NULL if no such loop */
	BPatch_basicBlockLoop* parent;

	/** Return true if the given address is within the range of
	    this loop's basicBlocks */
	bool containsAddress(unsigned long addr);

        /** return the back edge which defines this loop */
        BPatch_edge *getBackEdge() { return backEdge; }

	/** returns vector of contained loops */
	void getContainedLoops(BPatch_Vector<BPatch_basicBlockLoop*>&);	

	/** returns vector of outer contained loops */
	void getOuterLoops(BPatch_Vector<BPatch_basicBlockLoop*>&);	

	/** returns all basic blocks in the loop */
	void getLoopBasicBlocks(BPatch_Vector<BPatch_basicBlock*>&);

	/** returns all basic blocks in this loop, exluding the blocks
	    of its sub loops. */
	void getLoopBasicBlocksExclusive(BPatch_Vector<BPatch_basicBlock*>&);

        /** does this loop or its subloops contain the given block? */
        bool hasBlock(BPatch_basicBlock *);

        /** does this loop contain the given block? */
        bool hasBlockExclusive(BPatch_basicBlock *);

	/** returns true if this loop is a descendant of the given loop */
	bool hasAncestor(BPatch_basicBlockLoop*);

	/** returns the flow graph this loop is in */
	BPatch_flowGraph* getFlowGraph();

	/** returns the head basic block of the loop */
	BPatch_basicBlock* getLoopHead();

	/** method that returns the variables used as iterator */
	/** not implemented yet */
	BPatch_Set<BPatch_variableExpr*>* getLoopIterators();

	/** destructor for the class */
	~BPatch_basicBlockLoop() { }

private:
// internal use only
	/** constructor of class */
	BPatch_basicBlockLoop(BPatch_flowGraph *);

	/** constructor of the class */
	BPatch_basicBlockLoop(BPatch_edge *, BPatch_flowGraph *);

	/** get either contained or outer loops, determined by outerMostOnly */
	void getLoops(BPatch_Vector<BPatch_basicBlockLoop*>&, 
		      bool outerMostOnly);
};

#endif /*_BPatch_basicBlockLoop_h_*/
