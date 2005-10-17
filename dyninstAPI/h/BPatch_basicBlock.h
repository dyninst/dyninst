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
#include "BPatch_instruction.h"
#include "BPatch_eventLock.h"
//#include "BPatch_edge.h"

class image;
class int_function;
class int_basicBlock;
class bitArray;

/* Currently all this bitarray stuff is just for power, 
   but could be extended as we do liveness stuff for other platforms */


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
	friend class dominatorCFG;
	friend class InstrucIter;
	friend class int_function;
	friend std::ostream& operator<<(std::ostream&,BPatch_basicBlock&);

 private:
   /** the internal basic block structure **/
   int_basicBlock *iblock;

	/** the flow graph that contains this basic block */
	BPatch_flowGraph *flowGraph;

	/** set of basic blocks that this basicblock dominates immediately*/
	BPatch_Set<BPatch_basicBlock*>* immediateDominates;

	/** basic block which is the immediate dominator of the basic block */
	BPatch_basicBlock *immediateDominator;

	/** same as previous two fields, but for postdominator tree */
	BPatch_Set<BPatch_basicBlock*> *immediatePostDominates;
	BPatch_basicBlock *immediatePostDominator;

	/** the source block(source lines) that basic block corresponds*/
	BPatch_Vector<BPatch_sourceBlock*> *sourceBlocks;

	/** the instructions within this block */
	BPatch_Vector<BPatch_instruction*> *instructions;

 	/** the incoming edges */
 	BPatch_Set<BPatch_edge*> incomingEdges;
 
 	/** the outgoing edges */
 	BPatch_Set<BPatch_edge*> outgoingEdges;


	/* Liveness analysis variables */
	/** gen registers */
	bitArray * gen;
	bitArray * genFP;

	/** kill registers */
	bitArray * kill;
	bitArray * killFP;
	
	/** in registers */
	bitArray * in;
	bitArray * inFP;

	/** out registers */
	bitArray * out;
	bitArray * outFP;

 protected:

   /** constructor of class */
   BPatch_basicBlock(int_basicBlock *ib, BPatch_flowGraph *fg);

 public:
   
   // Internal functions. Don't use these unless you know what you're
   // doing.
   const int_basicBlock *lowlevel_block() const { return iblock; }


	/** BPatch_basicBlock::getSources   */
	/** method that returns the predecessors of the basic block */

   API_EXPORT_V(Int, (srcs),
                void,getSources,(BPatch_Vector<BPatch_basicBlock*> &srcs));

	/** BPatch_basicBlock::getTargets   */
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

   /** BPatch_basicBlock::setEmtryBlock   */
   /** sets whether this block is an entry block (or not) */

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
       
   /** BPatch_basicBlock::getEndAddress    */
        
   API_EXPORT(Int, (),
              unsigned long, getEndAddress, () CONST_EXPORT);

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

   /** BPatch_basicBlock::initRegisterGenKill */
   /** Initializes the gen/kill sets for register liveness analysis */
   API_EXPORT(Int, (),
	      
	      bool,initRegisterGenKill,());
   
   /** BPatch_basicBlock::updateRegisternOut */
   /** Initializes the gen/kill sets for register liveness analysis */
   API_EXPORT(Int, (isFP),
	      
	      bool,updateRegisterInOut,(bool isFP));
   
   /** BPatch_basicBlock::getInSet*/

   API_EXPORT(Int, (),
	      
	      bitArray *, getInSet, ());
   
   
   /** BPatch_basicBlock::getInFPSet **/
   API_EXPORT(Int, (),
	      
	      bitArray *, getInFPSet, ());
   
   /** BPatch_basicBlock::printAll **/
   API_EXPORT(Int, (),
	      
	      bool,printAll,());
   
   API_EXPORT(Int, (liveReg, liveFPReg, address),
	      
	      int, liveRegistersIntoSet, (int *& liveReg, int *& liveFPReg,
				    unsigned long address));

   API_EXPORT(Int, (liveSPReg, address),
	      
	      int, liveSPRegistersIntoSet, (int *& liveSPReg,
					    unsigned long address));
   

   int blockNo() const;
    
   struct compare {
      int operator()(const BPatch_basicBlock *b1, 
                     const BPatch_basicBlock *b2) const 
      {
         if (b1->blockNo() < b2->blockNo())
            return -1;
         if (b1->blockNo() > b2->blockNo())
            return 1;
         return 0;
      }
   };
};

#endif /* _BPatch_basicBlock_h_ */
