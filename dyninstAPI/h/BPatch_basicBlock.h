/*
 * See the dyninst/COPYRIGHT file for copyright information.
 * 
 * We provide the Paradyn Tools (below described as "Paradyn")
 * on an AS IS basis, and do not warrant its validity or performance.
 * We reserve the right to update, modify, or discontinue this
 * software at any time.  We shall have no obligation to supply such
 * updates or modifications or any other form of support to you.
 * 
 * By your use of Paradyn, you understand and agree that we (or any
 * other person or entity with proprietary rights in Paradyn) are
 * under no obligation to provide either maintenance services,
 * update services, notices of latent defects, or correction of
 * defects for Paradyn.
 * 
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

#ifndef _BPatch_basicBlock_h_
#define _BPatch_basicBlock_h_

#include <iosfwd>
#include <set>
#include <utility>
#include <vector>
#include "BPatch_dll.h"
#include "BPatch_Vector.h"
#include "BPatch_Set.h"
#include "BPatch_sourceBlock.h" 
#include "BPatch_instruction.h"
#include "Instruction.h"
#include "BPatch_enums.h"
#include "dyntypes.h"
//#include "BPatch_edge.h"

class image;
class func_instance;
class instPoint;
class block_instance;
class BPatch_point;
class BPatch_edge;
class BPatch_function;
class BPatch_flowGraph;
class BPatch_basicBlock;

/* Currently all this bitarray stuff is just for power, 
   but could be extended as we do liveness stuff for other platforms */

namespace Dyninst {
  namespace ParseAPI {
    class Block;
    BPATCH_DLL_EXPORT Block *convert(const BPatch_basicBlock *);
  }
  namespace PatchAPI {
    class PatchBlock;
    BPATCH_DLL_EXPORT PatchBlock *convert(const BPatch_basicBlock *);
  }
}


namespace std {
template <>
   struct less<BPatch_basicBlock *> {
   BPATCH_DLL_EXPORT bool operator()(const BPatch_basicBlock * const &l, const BPatch_basicBlock * const &r) const;
};
}

template <>
struct comparison <BPatch_basicBlock *> {
   BPATCH_DLL_EXPORT bool operator()(const BPatch_basicBlock * const &x, 
                   const BPatch_basicBlock * const &y) const;
};

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

struct BPATCH_DLL_EXPORT insnPredicate
{
  using result_type = bool;
  using argument_type = Dyninst::InstructionAPI::Instruction;
  virtual result_type operator()(argument_type arg) = 0;
  virtual ~insnPredicate() {}
    
};

class BPATCH_DLL_EXPORT BPatch_basicBlock {
  friend class BPatch_flowGraph;
  friend class TarjanDominator;
  friend class dominatorCFG;
  friend class func_instance;
  friend class BPatch_instruction;
  friend std::ostream& operator<<(std::ostream&,BPatch_basicBlock&);
  friend Dyninst::ParseAPI::Block *Dyninst::ParseAPI::convert(const BPatch_basicBlock *);
  friend Dyninst::PatchAPI::PatchBlock *Dyninst::PatchAPI::convert(const BPatch_basicBlock *);


 private:
  /** the internal basic block structure **/
  block_instance *iblock;

  /** the flow graph that contains this basic block */
  BPatch_flowGraph *flowGraph;
   
   /** set of basic blocks that this basicblock dominates immediately*/
   std::set<BPatch_basicBlock*>* immediateDominates;
   
  /** basic block which is the immediate dominator of the basic block */
  BPatch_basicBlock *immediateDominator;
   
   /** same as previous two fields, but for postdominator tree */
   std::set<BPatch_basicBlock*> *immediatePostDominates;
   BPatch_basicBlock *immediatePostDominator;
   
  /** the source block(source lines) that basic block corresponds*/
  BPatch_Vector<BPatch_sourceBlock*> *sourceBlocks;
   
  /** the instructions within this block */
  BPatch_Vector<BPatch_instruction*> *instructions;
   
   /** the incoming edges */
   std::set<BPatch_edge*> incomingEdges;
 
   /** the outgoing edges */
   std::set<BPatch_edge*> outgoingEdges;

 public:
  BPatch_flowGraph *fg() const { return flowGraph; }
  block_instance *block() const { return iblock; }
  BPatch_function *func() const;
  func_instance *ifunc() const;

 protected:

  /** constructor of class */
  BPatch_basicBlock(block_instance *ib, BPatch_flowGraph *fg);


   
  BPatch_Vector<BPatch_point*>*
  findPointByPredicate(insnPredicate& f);

 public:
   
  // Internal functions. Don't use these unless you know what you're
  // doing.
  block_instance *lowlevel_block()  { return iblock; }

  void setlowlevel_block(block_instance *b)  { iblock = b; }
  void  getAllPoints(std::vector<BPatch_point*>& allPoints);
  BPatch_point *convertPoint(instPoint *pt);
  BPatch_function *getCallTarget();
  // end internal functions

  BPatch_flowGraph * getFlowGraph() const;

  /** BPatch_basicBlock::getSources   */
  /** method that returns the predecessors of the basic block */

  void getSources(BPatch_Vector<BPatch_basicBlock*> &srcs);

  /** BPatch_basicBlock::getTargets   */
  /** method that returns the successors  of the basic block */

  void getTargets(BPatch_Vector<BPatch_basicBlock*> &targets);

  /** BPatch_basicBlock::dominates   */
  /** returns true if argument is dominated by this basic block */
  
  bool dominates(BPatch_basicBlock *block);

  /** BPatch_basicBlock::getImmediateDominiator   */
  /** return the immediate dominator of a basic block */

  BPatch_basicBlock* getImmediateDominator();

  /** BPatch_basicBlock::getImmediateDominates   */
  /** method that returns the basic blocks immediately dominated by   */
  /** the basic block */

  void getImmediateDominates(BPatch_Vector<BPatch_basicBlock*> &blocks);

  /** BPatch_basicBlock::getAllDominates   */
  /** method that returns all basic blocks dominated by the basic block */

  void getAllDominates(BPatch_Set<BPatch_basicBlock*> &blocks);
  void getAllDominates(std::set<BPatch_basicBlock*> &blocks);

  /** the previous four methods, but for postdominators */

  /** BPatch_basicBlock::postdominates   */

  bool postdominates(BPatch_basicBlock *block);

  /** BPatch_basicBlock::getImmediatePostDominator   */

  BPatch_basicBlock* getImmediatePostDominator();

  /** BPatch_basicBlock::getImmediatePostDominates   */

  void getImmediatePostDominates(BPatch_Vector<BPatch_basicBlock*> &blocks);

  /** BPatch_basicBlock::getAllPostDominates   */

  void getAllPostDominates(BPatch_Set<BPatch_basicBlock*> &blocks);
  void getAllPostDominates(std::set<BPatch_basicBlock*> &blocks);
	
  /** BPatch_basicBlock::getSourceBlocks   */
  /** returns the source block corresponding to the basic block */

  bool getSourceBlocks(BPatch_Vector<BPatch_sourceBlock*> &blocks);

  /** BPatch_basicBlock::getBlockNumber   */
  /** returns the block id */

  int getBlockNumber();

  /** BPatch_basicBlock::setEmtryBlock   */
  /** sets whether this block is an entry block (or not) */

  /** BPatch_basicBlock::isEntryBlock   */

  bool isEntryBlock() const;

  /** BPatch_basicBlock::isExitBlock   */

  bool isExitBlock() const;

  /** BPatch_basicBlock::size   */

  unsigned size() const;

  /** BPatch_basicBlock::getStartAddress   */
  //these always return absolute address

  unsigned long getStartAddress() const;

  /** BPatch_basicBlock::getLastInsnAddress   */

  unsigned long getLastInsnAddress() const;
       
  /** BPatch_basicBlock::getEndAddress    */
        
  unsigned long  getEndAddress() const;

  /** BPatch_basicBlock::~BPatch_basicBlock   */
  /** destructor of class */

  ~BPatch_basicBlock();
        
  /** BPatch_basicBlock::getAddressRange   */
  /** return the start and end addresses of the basic block */

  bool getAddressRange(void*& _startAddress, void*& _endAddress);

  /** BPatch_basicBlock::findEntryPoint   */
  /** return point at the start of the basic block */

  BPatch_point*  findEntryPoint();

  /** BPatch_basicBlock::findExitPoint   */
  /** return point at the start of the basic block */
   
  BPatch_point*  findExitPoint();

  /** BPatch_basicBlock::findPoint   */
  /** return a set of points within the basic block */

  BPatch_Vector<BPatch_point*> * findPoint(const BPatch_Set<BPatch_opCode>& ops);
  BPatch_Vector<BPatch_point*> * findPoint(const std::set<BPatch_opCode>& ops);

  BPatch_Vector<BPatch_point*> * findPoint(bool(*filter)(Dyninst::InstructionAPI::Instruction));
   
  BPatch_point *  findPoint(Dyninst::Address addr);

  /** BPatch_basicBlock::getInstructions   */
  /** return the instructions that belong to the block */

  bool  getInstructions(std::vector<Dyninst::InstructionAPI::Instruction>& insns);
  bool  getInstructions(std::vector<std::pair<Dyninst::InstructionAPI::Instruction, Dyninst::Address> >& insnInstances);


  /** BPatch_basicBlock::getIncomingEdges   */
  /** returns the incoming edges */

  void getIncomingEdges(BPatch_Vector<BPatch_edge*> &inc);
        
  /** BPatch_basicBlock::getOutgoingEdges   */
  /** returns the outgoming edges */


  void getOutgoingEdges(BPatch_Vector<BPatch_edge*> &out);


  operator Dyninst::ParseAPI::Block *() const;
  operator Dyninst::PatchAPI::PatchBlock *() const;

  int blockNo() const;

};

BPATCH_DLL_EXPORT std::ostream& operator<<(std::ostream&,BPatch_basicBlock&);

#endif /* _BPatch_basicBlock_h_ */
