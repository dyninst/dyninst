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

#include "InstructionAdapter.h"
#include "Instruction.h"
#include "InstructionDecoder.h"
#include <boost/tuple/tuple.hpp>

class IA_IAPI : public InstructionAdapter
{
  friend class image_func;
    public:
        IA_IAPI(dyn_detail::boost::shared_ptr<Dyninst::InstructionAPI::InstructionDecoder> dec_,
                Address start_, image_func* f);
        IA_IAPI(dyn_detail::boost::shared_ptr<Dyninst::InstructionAPI::InstructionDecoder> dec_,
                Address start_, image * im);
        virtual ~IA_IAPI() {
        }
        Dyninst::InstructionAPI::Instruction::Ptr getInstruction();
    
        virtual bool hasCFT() const;
        virtual size_t getSize() const;
        virtual bool isFrameSetupInsn() const;
        virtual bool isAbortOrInvalidInsn() const;
        virtual bool isAllocInsn() const;
        virtual void
                getNewEdges(pdvector<std::pair< Address, EdgeTypeEnum> >&
                outEdges, image_basicBlock* currBlk,
                unsigned int num_insns,
                dictionary_hash<Address, std::string> *pltFuncs) const;
        virtual InstrumentableLevel getInstLevel( unsigned int num_insns ) const;
        virtual bool isDynamicCall() const;
        virtual bool isAbsoluteCall() const;
        virtual bool simulateJump() const;
        virtual void advance();
        virtual bool isNop() const;
        virtual bool isLeave() const;
        virtual bool isDelaySlot() const;
        virtual bool isRelocatable(InstrumentableLevel lvl) const;
        virtual bool isTailCall(unsigned int) const;
        virtual bool checkEntry() const;
        virtual Address getCFT() const;
        virtual bool isStackFramePreamble(int& frameSize) const;
        virtual bool savesFP() const;
        virtual bool cleansStack() const;
        virtual bool isConditional() const;
        virtual bool isBranch() const;
        virtual bool isInterruptOrSyscall() const;
        virtual bool isSyscall() const;
        virtual bool isInterrupt() const;
    private:
        virtual bool isRealCall() const;
        virtual bool isThunk() const;
        bool parseJumpTable(image_basicBlock* currBlk,
                            pdvector<std::pair< Address, EdgeTypeEnum > >& outEdges) const;
        bool isIPRelativeBranch() const;
        bool isMovAPSTable(pdvector<std::pair< Address, EdgeTypeEnum > >& outEdges) const;
        std::pair<Address, Address> findThunkAndOffset(image_basicBlock* start) const;
        bool isTableInsn(Dyninst::InstructionAPI::Instruction::Ptr i) const;
        std::map<Address, Dyninst::InstructionAPI::Instruction::Ptr>::const_iterator findTableInsn() const;
        boost::tuple<Dyninst::InstructionAPI::Instruction::Ptr,
        Dyninst::InstructionAPI::Instruction::Ptr,
        bool> findMaxSwitchInsn(image_basicBlock *start) const;
        Address findThunkInBlock(image_basicBlock* curBlock, Address& thunkOffset) const;
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
                              pdvector<std::pair< Address, EdgeTypeEnum> >& outEdges) const;
        Address getTableAddress(Dyninst::InstructionAPI::Instruction::Ptr tableInsn,
                                Address thunkOffset) const;
        bool isFrameSetupInsn(Dyninst::InstructionAPI::Instruction::Ptr i) const;
        virtual bool isReturn() const;
        virtual bool isCall() const;




        dyn_detail::boost::shared_ptr<Dyninst::InstructionAPI::InstructionDecoder> dec;
        std::map<Address, Dyninst::InstructionAPI::Instruction::Ptr> allInsns;
        Dyninst::InstructionAPI::Instruction::Ptr curInsn() const;
        std::map<Address, Dyninst::InstructionAPI::Instruction::Ptr>::const_iterator curInsnIter;
        mutable bool validCFT;
        mutable Address cachedCFT;
        mutable std::pair<bool, bool> hascftstatus;
        mutable std::pair<bool, bool> tailCall;
        Dyninst::InstructionAPI::RegisterAST::Ptr framePtr;
        Dyninst::InstructionAPI::RegisterAST::Ptr stackPtr;
        Dyninst::InstructionAPI::RegisterAST::Ptr thePC;
        static std::map<Address, bool> thunkAtTarget;
};


#endif // !defined(IA_IAPI_H)
