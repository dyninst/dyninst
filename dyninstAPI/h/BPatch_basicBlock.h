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

#include "BPatch_point.h"
#include "BPatch_dll.h"
#include "BPatch_Vector.h"
#include "BPatch_Set.h"
#include "BPatch_sourceBlock.h" 
#include "BPatch_point.h"
#include "BPatch_instruction.h"
#include "BPatch_eventLock.h"
#include "BPatch_edge.h"

class image;
class int_function;

/** class for machine code basic blocks. We assume the user can not 
  * create basic blocks using its constructor. It is not safe. 
  * basic blocks are used for reading purposes not for inserting
  * a new code to the machine executable other than instrumentation code
  *
  * @see BPatch_flowGraph
  * @see BPatch_sourceBlock
  * @see BPatch_basicBlockLoop
  */
class BPatch_flowGraph;
#ifdef DYNINST_CLASS_NAME
#undef DYNINST_CLASS_NAME
#endif
#define DYNINST_CLASS_NAME BPatch_basicBlock

class BPATCH_DLL_EXPORT BPatch_basicBlock : public BPatch_eventLock {
	friend class BPatch_flowGraph;
	friend class TarjanDominator;
	friend class InstrucIter;
	friend class int_function;
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

 	/** the incoming edges */
 	BPatch_Set<BPatch_edge*> incomingEdges;
 
 	/** the outgoing edges */
 	BPatch_Set<BPatch_edge*> outgoingEdges;


#if defined( arch_ia64 )

protected:

	/* These may be fairly general, */
	BPatch_Set< BPatch_basicBlock * > * dataFlowIn;
	BPatch_Set< BPatch_basicBlock * > * dataFlowOut;
	
	/* but these are certainly not.   Notionally,
	   all four of these could be factored out into an
	   associative array (or four), from basicBlock pointers
	   to data flow data, but for now, I won't do that. */
	BPatch_basicBlock * dataFlowGen;
	BPatch_basicBlock * dataFlowKill;
	
public:

        // const accessor functions


        //  BPatch_basicBlock::getDataFlowIn
        //
        API_EXPORT(Int, (),

        BPatch_Set< BPatch_basicBlock * > *,getDataFlowIn,() CONST_EXPORT);
        
        //  BPatch_basicBlock::getDataFlowOut
        //
        API_EXPORT(Int, (),

        BPatch_Set< BPatch_basicBlock * > *,getDataFlowOut,() CONST_EXPORT);

        //  BPatch_basicBlock::getDataFlowGen
        //
        API_EXPORT(Int, (),

        BPatch_basicBlock *,getDataFlowGen,() CONST_EXPORT);

        //  BPatch_basicBlock::getDataFlowKill
        //
        API_EXPORT(Int, (),

        BPatch_basicBlock *,getDataFlowKill,() CONST_EXPORT)

        //  BPatch_basicBlock::setDataFlowIn
        //
        API_EXPORT_V(Int, (dfi),

        void,setDataFlowIn,(BPatch_Set< BPatch_basicBlock * > *dfi));

        //  BPatch_basicBlock::setDataFlowOut
        //
        API_EXPORT_V(Int, (dfo),

        void,setDataFlowOut,(BPatch_Set< BPatch_basicBlock * > *dfo));

        //  BPatch_basicBlock::setDataFlowGen
        //
        API_EXPORT_V(Int, (dfg),

        void,setDataFlowGen,(BPatch_basicBlock *dfg));

        //  BPatch_basicBlock::setDataFlowKill
        //
        API_EXPORT_V(Int, (dfk),

        void,setDataFlowKill,(BPatch_basicBlock *dfk));
	
#endif /* defined( arch_ia64 ) */

public:
	/** BPatch_basicBlock::getSources   */
	/** method that returns the predecessors of the basic block */

        API_EXPORT_V(Int, (srcs),

        void,getSources,(BPatch_Vector<BPatch_basicBlock*> &srcs));

	/** BPatch_basicBlock::addTargets   */
	/** method that returns the successors  of the basic block */

        API_EXPORT_V(Int, (targets),

        void,getTargets,(BPatch_Vector<BPatch_basicBlock*> &targets));

	/** BPatch_basicBlock::dominates   */
	/** returns true if argument is dominated by this basic block */

        API_EXPORT(Int, (block),

        bool,dominates,(BPatch_basicBlock *block));

        /** BPatch_basicBlock::getImmediateDominiator   */
        /** return the immediate dominator of a basic block */

        API_EXPORT(Int, (),

        BPatch_basicBlock*,getImmediateDominator,());

	/** BPatch_basicBlock::getImmediateDominates   */
	/** method that returns the basic blocks immediately dominated by   */
	/** the basic block */

        API_EXPORT_V(Int, (blocks),
        
        void,getImmediateDominates,(BPatch_Vector<BPatch_basicBlock*> &blocks));

	/** BPatch_basicBlock::getAllDominates   */
	/** method that returns all basic blocks dominated by the basic block */

        API_EXPORT_V(Int, (blocks),

        void,getAllDominates,(BPatch_Set<BPatch_basicBlock*> &blocks));

	/** the previous four methods, but for postdominators */

	/** BPatch_basicBlock::postdominates   */

        API_EXPORT(Int, (block),

        bool,postdominates,(BPatch_basicBlock *block));

	/** BPatch_basicBlock::getImmediatePostDominator   */

        API_EXPORT(Int, (),

        BPatch_basicBlock*,getImmediatePostDominator,());

	/** BPatch_basicBlock::getImmediatePostDominates   */

        API_EXPORT_V(Int, (blocks),

        void,getImmediatePostDominates,(BPatch_Vector<BPatch_basicBlock*> &blocks));

	/** BPatch_basicBlock::getAllPostDominates   */

        API_EXPORT_V(Int, (blocks),

        void,getAllPostDominates,(BPatch_Set<BPatch_basicBlock*> &blocks));
	
	/** BPatch_basicBlock::getSourceBlocks   */
	/** returns the source block corresponding to the basic block */

        API_EXPORT(Int, (blocks),

        bool,getSourceBlocks,(BPatch_Vector<BPatch_sourceBlock*> &blocks));

	/** BPatch_basicBlock::getBlockNumber   */
	/** returns the block id */

        API_EXPORT(Int, (),

        int,getBlockNumber,());

	public:  int blockNo() { return getBlockNumber(); }
 
	/** BPatch_basicBlock::setEmtryBlock   */
        /** sets whether this block is an entry block (or not) */

        API_EXPORT(Int, (b),

        bool,setEntryBlock,(bool b));

	/** BPatch_basicBlock::setExitBlock   */
        /** sets whether this block is an exit block (or not) */

        API_EXPORT(Int, (b),

        bool,setExitBlock,(bool b));

        /** BPatch_basicBlock::isEntryBlock   */

        API_EXPORT(Int, (),

        bool,isEntryBlock,() CONST_EXPORT);

	/** BPatch_basicBlock::isExitBlock   */

        API_EXPORT(Int, (),

        bool,isExitBlock,() CONST_EXPORT);

	/** BPatch_basicBlock::size   */

        API_EXPORT(Int, (),

        unsigned,size,() CONST_EXPORT);

	/** BPatch_basicBlock::getStartAddress   */
        //these always return absolute address

        API_EXPORT(Int, (),

        unsigned long,getStartAddress,() CONST_EXPORT);

	/** BPatch_basicBlock::getLastInsnAddress   */

        API_EXPORT(Int, (),

        unsigned long,getLastInsnAddress,() CONST_EXPORT);
       
        //basic blocks on x86 store relative addresses
        //we manipulate them with these functions

	/** BPatch_basicBlock::getRelStart   */

        API_EXPORT(Int, (),

        unsigned long,getRelStart,() CONST_EXPORT);

	/** BPatch_basicBlock::getRelLast   */

        API_EXPORT(Int, (),

        unsigned long,getRelLast,() CONST_EXPORT);

	/** BPatch_basicBlock::getRelEnd   */

        API_EXPORT(Int, (),

        unsigned long,getRelEnd,() CONST_EXPORT);

	/** BPatch_basicBlock::setRelStart   */

        API_EXPORT(Int, (sAddr),

        unsigned long,setRelStart,(unsigned long sAddr));

	/** BPatch_basicBlock::setRelLast   */

        API_EXPORT(Int, (eAddr),

        unsigned long,setRelLast,(unsigned long eAddr));

	/** BPatch_basicBlock::setRelEnd   */

        API_EXPORT(Int, (eAddr),

        unsigned long,setRelEnd,(unsigned long eAddr));

	/** BPatch_basicBlock::addSource   */

        API_EXPORT(Int, (b),

        bool,addSource,(BPatch_basicBlock *b));

	/** BPatch_basicBlock::addTarget   */

        API_EXPORT(Int, (b),

        bool,addTarget,(BPatch_basicBlock *b));

	/** BPatch_basicBlock::removeSource   */

        API_EXPORT(Int, (b),

        bool,removeSource,(BPatch_basicBlock *b));

	/** BPatch_basicBlock::removeTarget   */

        API_EXPORT(Int, (b),

        bool,removeTarget,(BPatch_basicBlock *b));

	/** BPatch_basicBlock::~BPatch_basicBlock   */
	/** destructor of class */

        API_EXPORT_DTOR(_dtor, (),

        ~,BPatch_basicBlock,());
        
	/** BPatch_basicBlock::getAddressRange   */
	/** return the start and end addresses of the basic block */

        API_EXPORT(Int, (_startAddress, _endAddress),

        bool,getAddressRange,(void*& _startAddress, void*& _endAddress));

#ifdef IBM_BPATCH_COMPAT
        //  dummy placeholder.  I think this is only used by dpcl in a debug routine
        API_EXPORT(Int, (_startLine, _endLine),

        bool,getLineNumbers,(unsigned int &_startLine, unsigned int  &_endLine));       
#endif

	/** BPatch_basicBlock::findPoint   */
	/** return a set of points within the basic block */

        API_EXPORT(Int, (ops),

        BPatch_Vector<BPatch_point*> *,findPoint,(const BPatch_Set<BPatch_opCode>& ops));

	/** BPatch_basicBlock::getInstructions   */
	/** return the instructions that belong to the block */

        API_EXPORT(Int, (),

        BPatch_Vector<BPatch_instruction*> *,getInstructions,());

	/** BPatch_basicBlock::getIncomingEdges   */
 	/** returns the incoming edges */

        API_EXPORT_V(Int, (inc),

        void,getIncomingEdges,(BPatch_Vector<BPatch_edge*> &inc));
        
	/** BPatch_basicBlock::getOutgoingEdges   */
 	/** returns the outgoming edges */

        API_EXPORT_V(Int, (out),

        void,getOutgoingEdges,(BPatch_Vector<BPatch_edge*> &out));

public:

        // internal use only
        void dump();

	/** constructor of class */
	BPatch_basicBlock(BPatch_flowGraph*, int);

	/** constructor of class */
	BPatch_basicBlock();

	/** method to set the block id */
	void setBlockNumber(int);
};

#endif /* _BPatch_basicBlock_h_ */
