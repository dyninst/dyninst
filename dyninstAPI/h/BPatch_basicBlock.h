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
	friend class pd_Function;
	friend std::ostream& operator<<(std::ostream&,BPatch_basicBlock&);

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
	unsigned long lastInsnAddress;

    /** this is the absolute end of the basic block
      * i.e. the first Address that is outside the block  
	  * needed on x86 because of variable length instructions
      */
    unsigned long endAddr;

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
 
    void setEntryBlock( bool b ) { isEntryBasicBlock = b; }
	void setExitBlock( bool b ) { isExitBasicBlock = b; }
    bool isExitBlock(){ return isExitBasicBlock; }
    bool isEntryBlock(){ return isEntryBasicBlock; }

    unsigned size() const;

    //these always return absolute address
	unsigned long getStartAddress() const;
	unsigned long getLastInsnAddress() const;
       
    //basic blocks on x86 store relative addresses
    //we manipulate them with these functions
    unsigned long getRelStart() const;
    unsigned long getRelLast() const;
    unsigned long getRelEnd() const;
    void setRelStart( unsigned long sAddr ){ startAddress = sAddr; }
	void setRelLast( unsigned long eAddr ){ lastInsnAddress = eAddr; }
    void setRelEnd( unsigned long eAddr ) { endAddr = eAddr; }


    void addSource( BPatch_basicBlock* b ){ sources += b; }
	void addTarget( BPatch_basicBlock* b ){ targets += b; }
    void removeSource( BPatch_basicBlock* b ){ sources.remove( b ); }
    void removeTarget( BPatch_basicBlock* b ){ targets.remove( b ); };

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

public:

// internal use only
	/** constructor of class */
	BPatch_basicBlock(BPatch_flowGraph*, int);

	/** constructor of class */
	BPatch_basicBlock();

	/** method to set the block id */
	void setBlockNumber(int);
};

#endif /* _BPatch_basicBlock_h_ */
