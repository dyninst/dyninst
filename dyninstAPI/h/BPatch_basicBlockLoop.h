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
#include "BPatch_eventLock.h"
#include "BPatch_flowGraph.h" 

/** class to represent the loops composed of machine code basic 
  * blocks in the executable (Natural loops)
  *
  * @see BPatch_basicBlock
  * @see BPatch_flowGraph
  */

class BPatch_variableExpr;
class BPatch_loopTreeNode;
class pdstring;

#ifdef DYNINST_CLASS_NAME
#undef DYNINST_CLASS_NAME
#endif
#define DYNINST_CLASS_NAME BPatch_basicBlockLoop

class BPATCH_DLL_EXPORT BPatch_basicBlockLoop : public BPatch_eventLock {
	friend class BPatch_flowGraph;
	friend std::ostream& operator<<(std::ostream&,BPatch_basicBlockLoop&);
	friend void dfsCreateLoopHierarchy(BPatch_loopTreeNode * parent,
                                           BPatch_Vector<BPatch_basicBlockLoop *> &loops,
                                           pdstring level);

private:
	BPatch_edge *backEdge;

        // the flow graph this loop is part of
        BPatch_flowGraph *flowGraph;

	/** set of loops that are contained (nested) in this loop. */
	BPatch_Set<BPatch_basicBlockLoop*> containedLoops;

	/** the basic blocks in the loop */
	BPatch_Set<BPatch_basicBlock*> basicBlocks;


public:
	/** If loop which directly encloses this loop. NULL if no such loop */
	BPatch_basicBlockLoop* parent;

	/** BPatch_basicBlockLoop::containsAddress    */
	/** Return true if the given address is within the range of
	    this loop's basicBlocks */
        API_EXPORT(Int, (addr),

        bool,containsAddress,(unsigned long addr));

	/** BPatch_basicBlockLoop::getBackEdge    */
        /** return the back edge which defines this loop */
        API_EXPORT(Int, (),

        BPatch_edge *,getBackEdge,());

	/** BPatch_basicBlockLoop::getContainedLoops    */
	/** returns vector of contained loops */
        API_EXPORT(Int, (loops),

        bool,getContainedLoops,(BPatch_Vector<BPatch_basicBlockLoop*> &loops));

	/** BPatch_basicBlockLoop::getOuterLoops    */
	/** returns vector of outer contained loops */
        API_EXPORT(Int, (loops),

        bool,getOuterLoops,(BPatch_Vector<BPatch_basicBlockLoop*> &loops));

	/** BPatch_basicBlockLoop::getLoopBasicBlocks    */
	/** returns all basic blocks in the loop */
        API_EXPORT(Int, (blocks),

        bool,getLoopBasicBlocks,(BPatch_Vector<BPatch_basicBlock*> &blocks));

	/** BPatch_basicBlockLoop::getLoopBasicBlocksExclusive    */
	/** returns all basic blocks in this loop, exluding the blocks
	    of its sub loops. */
        API_EXPORT(Int, (blocks),

        bool,getLoopBasicBlocksExclusive,(BPatch_Vector<BPatch_basicBlock*> &blocks));

        /** does this loop or its subloops contain the given block? */
        API_EXPORT(Int, (b),

        bool,hasBlock,(BPatch_basicBlock *b));

        /** does this loop contain the given block? */
        API_EXPORT(Int, (b),

        bool,hasBlockExclusive,(BPatch_basicBlock *b));

	/** BPatch_basicBlockLoop::hasAncestor    */
	/** returns true if this loop is a descendant of the given loop */
        API_EXPORT(Int, (loop),

        bool,hasAncestor,(BPatch_basicBlockLoop *loop));

	/** returns the flow graph this loop is in */
        API_EXPORT(Int, (),

        BPatch_flowGraph *,getFlowGraph,());

	/** BPatch_basicBlockLoop::getLoopHead    */
	/** returns the head basic block of the loop */
        API_EXPORT(Int, (),

        BPatch_basicBlock *,getLoopHead,());

	/** BPatch_basicBlockLoop::getLoopIterators    */
	/** method that returns the variables used as iterator */
	/** not implemented yet */
        API_EXPORT(Int, (),

        BPatch_Set<BPatch_variableExpr*> *,getLoopIterators,());

	/** BPatch_basicBlockLoop::~BPatch_basicBlockLoop    */
	/** destructor for the class */

	public:  ~BPatch_basicBlockLoop() { }

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
