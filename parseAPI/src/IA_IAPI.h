/*
 * Copyright (c) 1996-2009 Barton P. Miller
 * 
 * We provide the Paradyn Parallel Performance Tools (below
 * described as "Paradyn") on an AS IS basis, and do not warrant its
 * validity or performance.  We reserve the right to update, modify,
 * or discontinue this software at any time.  We shall have no
 * obligation to supply such updates or modifications or any other
 * form of support to you.
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

#if !defined(IA_IAPI_H)
#define IA_IAPI_H

#include <boost/tuple/tuple.hpp>

#include "InstructionAdapter.h"
#include "InstructionDecoder.h"
#include "Instruction.h"

#include "dynutil/h/dyntypes.h"

#include "parseAPI/h/CFG.h"

namespace Dyninst {
namespace InsnAdapter {

class IA_IAPI : public InstructionAdapter
{
  friend class image_func;
    public:
        IA_IAPI(Dyninst::InstructionAPI::InstructionDecoder &dec_,
                Address start_, 
                Dyninst::ParseAPI::CodeObject* o,
                Dyninst::ParseAPI::CodeRegion* r,
                Dyninst::InstructionSource *isrc);
        virtual ~IA_IAPI() {
        }
        Dyninst::InstructionAPI::Instruction::Ptr getInstruction();
    
        virtual bool hasCFT() const;
        virtual size_t getSize() const;
        virtual bool isFrameSetupInsn() const;
        virtual bool isAbortOrInvalidInsn() const;
        virtual void
                getNewEdges(std::vector<std::pair< Address, 
                                Dyninst::ParseAPI::EdgeTypeEnum> >&outEdges, 
                Dyninst::ParseAPI::Function * context,
                Dyninst::ParseAPI::Block * currBlk,
                unsigned int num_insns,
                dyn_hash_map<Address, std::string> *pltFuncs) const;
        virtual InstrumentableLevel getInstLevel(Dyninst::ParseAPI::Function *, unsigned int num_insns ) const;
        virtual bool isDynamicCall() const;
        virtual bool isAbsoluteCall() const;
        virtual bool simulateJump() const;
        virtual void advance();
        virtual bool retreat();
        virtual bool isNop() const;
        virtual bool isLeave() const;
        virtual bool isDelaySlot() const;
        virtual bool isRelocatable(InstrumentableLevel lvl) const;
        virtual bool isTailCall(Dyninst::ParseAPI::Function *,unsigned int) const;
        virtual Address getCFT() const;
        virtual bool isStackFramePreamble() const;
        virtual bool savesFP() const;
        virtual bool cleansStack() const;
        virtual bool isConditional() const;
        virtual bool isBranch() const;
        virtual bool isInterruptOrSyscall() const;
        virtual bool isSyscall() const;
        virtual bool isInterrupt() const;
        virtual bool isCall() const;
        virtual bool isReturnAddrSave() const;
        virtual bool isNopJump() const;
private:
        virtual bool isRealCall() const;
        virtual bool isThunk() const;
        bool parseJumpTable(Dyninst::ParseAPI::Block* currBlk,
             std::vector<std::pair< Address, Dyninst::ParseAPI::EdgeTypeEnum > >& outEdges) const;
        bool isIPRelativeBranch() const;
        bool isMovAPSTable(std::vector<std::pair< Address, Dyninst::ParseAPI::EdgeTypeEnum > >& outEdges) const;
        std::pair<Address, Address> findThunkAndOffset(Dyninst::ParseAPI::Block * start) const;
        bool isTableInsn(Dyninst::InstructionAPI::Instruction::Ptr i) const;
        std::map<Address, Dyninst::InstructionAPI::Instruction::Ptr>::const_iterator findTableInsn() const;
        boost::tuple<Dyninst::InstructionAPI::Instruction::Ptr,
        Dyninst::InstructionAPI::Instruction::Ptr,
        bool> findMaxSwitchInsn(Dyninst::ParseAPI::Block *start) const;
        Address findThunkInBlock(Dyninst::ParseAPI::Block * curBlock, Address& thunkOffset) const;
        bool computeTableBounds(Dyninst::InstructionAPI::Instruction::Ptr maxSwitchInsn,
                                Dyninst::InstructionAPI::Instruction::Ptr branchInsn,
                                Dyninst::InstructionAPI::Instruction::Ptr tableInsn,
                                bool foundJCCAlongTaken,
                                unsigned& tableSize,
                                unsigned& tableStride) const;
        bool fillTableEntries(Address thunkOffset,
                              Address tableBase,
                              unsigned tableSize,
                              unsigned tableStride,
                              int offsetMultiplier,
                              std::vector<std::pair< Address, Dyninst::ParseAPI::EdgeTypeEnum> >& outEdges) const;
        Address getTableAddress(Dyninst::InstructionAPI::Instruction::Ptr tableInsn,
                                Address tableInsnAddr,
                                Address thunkOffset) const;
        bool isFrameSetupInsn(Dyninst::InstructionAPI::Instruction::Ptr i) const;
        virtual bool isReturn() const;
        bool isFakeCall() const;
        bool isIATcall() const;


        Dyninst::InstructionAPI::InstructionDecoder & dec;
        std::map<Address, Dyninst::InstructionAPI::Instruction::Ptr> allInsns;
        Dyninst::InstructionAPI::Instruction::Ptr curInsn() const;
        std::map<Address, Dyninst::InstructionAPI::Instruction::Ptr>::iterator curInsnIter;
        mutable bool validCFT;
        mutable Address cachedCFT;
        mutable std::pair<bool, bool> hascftstatus;
        mutable std::pair<bool, bool> tailCall;
        static std::map<Architecture, Dyninst::InstructionAPI::RegisterAST::Ptr> framePtr;
        static std::map<Architecture, Dyninst::InstructionAPI::RegisterAST::Ptr> stackPtr;
        static std::map<Architecture, Dyninst::InstructionAPI::RegisterAST::Ptr> thePC;
        static std::map<Address, bool> thunkAtTarget;
        static void initASTs();
};

}
}


#endif // !defined(IA_IAPI_H)
