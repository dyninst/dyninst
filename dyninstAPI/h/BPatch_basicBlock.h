#ifndef _BPatch_basicBlock_h_
#define _BPatch_basicBlock_h_

#include "BPatch_dll.h"
#include "BPatch_Vector.h"
#include "BPatch_Set.h"
#include "BPatch_sourceBlock.h" 
#include "BPatch_point.h"
#include "BPatch_instruction.h"

/** class for machine code basic blocks. We assume the user can not 
  * create basic blocks using its constructor. It is not safe. 
  * basic blocks are used for reading purposes not for inserting
  * a new code to the machine executable other than instrumentation code
  *
  * @see BPatch_flowGraph
  * @see BPatch_sourceBlock
  * @see BPatch_basicBlockLoop
  */

class BPATCH_DLL_EXPORT BPatch_basicBlock {
	friend class BPatch_flowGraph;
	friend class TarjanDominator;
	friend class InstrucIter;

	friend ostream& operator<<(ostream&,BPatch_basicBlock&);

private:
	/** the flow graph that contains this basic block */
	BPatch_flowGraph *flowGraph;

	/** the ID of the block.It is unique in the CFG.
          * starts at 0 and goes up to (number of basic blocks - 1)
	  */
	int blockNumber;

	/** a flag to keep whether a basic block is the entry to CFG or not */
	bool isEntryBasicBlock;

	/** a flag to keep whether a basic block is the exit from CFG or not */
	bool isExitBasicBlock;

	/** the start address of the basic blocks. */
	unsigned long startAddress;

	/** the end address of the basic blocks
	  * address of the last instruction in the block 
	  */
	unsigned long endAddress;

	/** set of basic blocks that this basicblock dominates immediately*/
	BPatch_Set<BPatch_basicBlock*>* immediateDominates;

	/** basic block which is the immediate dominator of the basic block */
	BPatch_basicBlock* immediateDominator;

	/** same as previous two fields, but for postdominator tree */

	BPatch_Set<BPatch_basicBlock*>* immediatePostDominates;
	BPatch_basicBlock* immediatePostDominator;

	/** the set of basic blocks that are predecessors in the CFG */
	BPatch_Set<BPatch_basicBlock*> sources;

	/** the set of basic blocks that are successors in the CFG */
	BPatch_Set<BPatch_basicBlock*> targets;

	/** the source block(source lines) that basic block corresponds*/
	BPatch_Vector<BPatch_sourceBlock*>* sourceBlocks;

	/** the instructions within this block */
	BPatch_Vector<BPatch_instruction*>* instructions;

public:

	/** method that returns the predecessors of the basic block */
	void getSources(BPatch_Vector<BPatch_basicBlock*>&);

	/** method that returns the successors  of the basic block */
	void getTargets(BPatch_Vector<BPatch_basicBlock*>&);

	/** returns true if argument is dominated by this basic block */
	bool dominates(BPatch_basicBlock*);

	/** return the immediate dominator of a basic block */
	BPatch_basicBlock* getImmediateDominator();

	/** method that returns the basic blocks immediately dominated by 
	  * the basic block 
          */
	void getImmediateDominates(BPatch_Vector<BPatch_basicBlock*>&);

	/** method that returns all basic blocks dominated by the basic block */
	void getAllDominates(BPatch_Set<BPatch_basicBlock*>&);

	/** the previous four methods, but for postdominators */

	bool postdominates(BPatch_basicBlock*);
	BPatch_basicBlock* getImmediatePostDominator();
	void getImmediatePostDominates(BPatch_Vector<BPatch_basicBlock*>&);
	void getAllPostDominates(BPatch_Set<BPatch_basicBlock*>&);
	
	/** returns the source block corresponding to the basic block */
	void getSourceBlocks(BPatch_Vector<BPatch_sourceBlock*>&);

	/** returns the block id */
	int getBlockNumber();

	/** destructor of class */
	~BPatch_basicBlock();

	/** return the start and end addresses of the basic block */
	bool getAddressRange(void*& _startAddress, void*& _endAddress);
#ifdef IBM_BPATCH_COMPAT
        //  dummy placeholder.  I think this is only used by dpcl in a debug routine
	bool getLineNumbers(unsigned int _startLine, unsigned int  _endLine) {return false;}
#endif

	/** return a set of points within the basic block */
	BPatch_Vector<BPatch_point*> *findPoint(const BPatch_Set<BPatch_opCode>& ops);

	/** return the instructions that belong to the block */
	BPatch_Vector<BPatch_instruction*> *getInstructions();

private:

// internal use only
	/** constructor of class */
	BPatch_basicBlock(BPatch_flowGraph*, int);

	/** constructor of class */
	BPatch_basicBlock();

	/** method to set the block id */
	void setBlockNumber(int);
};

#endif /* _BPatch_basicBlock_h_ */
