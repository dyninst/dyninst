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
  block_instance *iblock;

  BPatch_flowGraph *flowGraph;
   
   std::set<BPatch_basicBlock*>* immediateDominates;
   
  BPatch_basicBlock *immediateDominator;
   
   std::set<BPatch_basicBlock*> *immediatePostDominates;
   BPatch_basicBlock *immediatePostDominator;
   
  BPatch_Vector<BPatch_sourceBlock*> *sourceBlocks;
   
  BPatch_Vector<BPatch_instruction*> *instructions;
   
   std::set<BPatch_edge*> incomingEdges;
 
   std::set<BPatch_edge*> outgoingEdges;

 public:
  BPatch_flowGraph *fg() const { return flowGraph; }
  block_instance *block() const { return iblock; }
  BPatch_function *func() const;
  func_instance *ifunc() const;

 protected:

  BPatch_basicBlock(block_instance *ib, BPatch_flowGraph *fg);


   
  BPatch_Vector<BPatch_point*>*
  findPointByPredicate(insnPredicate& f);

 public:
   
  block_instance *lowlevel_block()  { return iblock; }

  void setlowlevel_block(block_instance *b)  { iblock = b; }
  void  getAllPoints(std::vector<BPatch_point*>& allPoints);
  BPatch_point *convertPoint(instPoint *pt);
  BPatch_function *getCallTarget();

  BPatch_flowGraph * getFlowGraph() const;

  void getSources(BPatch_Vector<BPatch_basicBlock*> &srcs);

  void getTargets(BPatch_Vector<BPatch_basicBlock*> &targets);

  bool dominates(BPatch_basicBlock *block);

  BPatch_basicBlock* getImmediateDominator();

  void getImmediateDominates(BPatch_Vector<BPatch_basicBlock*> &blocks);

  void getAllDominates(BPatch_Set<BPatch_basicBlock*> &blocks);
  void getAllDominates(std::set<BPatch_basicBlock*> &blocks);

  bool postdominates(BPatch_basicBlock *block);

  BPatch_basicBlock* getImmediatePostDominator();

  void getImmediatePostDominates(BPatch_Vector<BPatch_basicBlock*> &blocks);

  void getAllPostDominates(BPatch_Set<BPatch_basicBlock*> &blocks);
  void getAllPostDominates(std::set<BPatch_basicBlock*> &blocks);
	
  bool getSourceBlocks(BPatch_Vector<BPatch_sourceBlock*> &blocks);

  int getBlockNumber();

  bool isEntryBlock() const;

  bool isExitBlock() const;

  unsigned size() const;

  unsigned long getStartAddress() const;

  unsigned long getLastInsnAddress() const;
       
  unsigned long  getEndAddress() const;

  ~BPatch_basicBlock();
        
  bool getAddressRange(void*& _startAddress, void*& _endAddress);

  BPatch_point*  findEntryPoint();

  BPatch_point*  findExitPoint();

  BPatch_Vector<BPatch_point*> * findPoint(const BPatch_Set<BPatch_opCode>& ops);
  BPatch_Vector<BPatch_point*> * findPoint(const std::set<BPatch_opCode>& ops);

  BPatch_Vector<BPatch_point*> * findPoint(bool(*filter)(Dyninst::InstructionAPI::Instruction));
   
  BPatch_point *  findPoint(Dyninst::Address addr);

  bool  getInstructions(std::vector<Dyninst::InstructionAPI::Instruction>& insns);
  bool  getInstructions(std::vector<std::pair<Dyninst::InstructionAPI::Instruction, Dyninst::Address> >& insnInstances);


  void getIncomingEdges(BPatch_Vector<BPatch_edge*> &inc);
        

  void getOutgoingEdges(BPatch_Vector<BPatch_edge*> &out);


  operator Dyninst::ParseAPI::Block *() const;
  operator Dyninst::PatchAPI::PatchBlock *() const;

  int blockNo() const;

};

BPATCH_DLL_EXPORT std::ostream& operator<<(std::ostream&,BPatch_basicBlock&);

#endif /* _BPatch_basicBlock_h_ */
