#ifndef _BPatch_basicBlockLoop_h
#define _BPatch_basicBlockLoop_h

#include <stdlib.h>
#include "BPatch_dll.h"
#include "BPatch_Vector.h"
#include "BPatch_Set.h"
#include "BPatch_basicBlock.h"

/** class to represent the loops composed of machine code basic 
  * blocks in the executable (Natural loops)
  *
  * @see BPatch_basicBlock
  * @see BPatch_flowGraph
  */

class BPatch_variableExpr;

class BPATCH_DLL_EXPORT BPatch_basicBlockLoop  {
	friend class BPatch_flowGraph;
	friend ostream& operator<<(ostream&,BPatch_basicBlockLoop&);

private:
	/** head basic block which dominates all basic blocks in the loop */
	BPatch_basicBlock* loopHead;

	/** set of loops that are contained (nested) in this loop. */
	BPatch_Set<BPatch_basicBlockLoop*> containedLoops;

	/** the basic blocks in the loop */
	BPatch_Set<BPatch_basicBlock*> basicBlocks;

	/** set of basic blocks having an edge to the head of the loop */
	BPatch_Set<BPatch_basicBlock*> backEdges;

public:
	/** If loop which directly encloses this loop. NULL if no such loop */
	BPatch_basicBlockLoop* parent;

	/** returns tail of back edges to loop head */
	void getBackEdges(BPatch_Vector<BPatch_basicBlock*>&);

	/** returns vector of contained loops */
	void getContainedLoops(BPatch_Vector<BPatch_basicBlockLoop*>&);	

	/** returns vector of outer contained loops */
	void getOuterLoops(BPatch_Vector<BPatch_basicBlockLoop*>&);	

	/** returns all basic blocks in the loop */
	void getLoopBasicBlocks(BPatch_Vector<BPatch_basicBlock*>&);

	/** returns all basic blocks in this loop, exluding the blocks
	    of its sub loops. */
	void getLoopBasicBlocksExclusive(BPatch_Vector<BPatch_basicBlock*>&);

	/** returns true if this loop is a descendant of the given loop */
	bool hasAncestor(BPatch_basicBlockLoop*);

	/** returns the head basic block of the loop */
	BPatch_basicBlock* getLoopHead();

	/** method that returns the variables used as iterator */
	/** not implemented yet */
	BPatch_Set<BPatch_variableExpr*>* getLoopIterators();

	/** destructor for the class */
	~BPatch_basicBlockLoop() {}

private:
// internal use only
	/** constructor of class */
	BPatch_basicBlockLoop();

	/** constructor of the class */
	BPatch_basicBlockLoop(BPatch_basicBlock*);

	/** get either contained or outer loops, determined by outerMostOnly */
	void getLoops(BPatch_Vector<BPatch_basicBlockLoop*>&, 
		      bool outerMostOnly);
};

#endif /*_BPatch_basicBlockLoop_h_*/
