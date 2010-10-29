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

#include "dyntypes.h"

#include "CFG.h"

namespace Dyninst {
namespace InsnAdapter {

class IA_IAPI : public InstructionAdapter
{
    friend class image_func;
    friend class IA_platformDetails;
    friend class IA_x86Details;
    friend class IA_powerDetails;
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
        virtual std::pair<bool, Address> getCFT() const;
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
        bool isFrameSetupInsn(Dyninst::InstructionAPI::Instruction::Ptr i) const;
        virtual bool isReturn() const;
        bool isFakeCall() const;
        const char *isIATcall() const;
        bool isLinkerStub() const;


        Dyninst::InstructionAPI::InstructionDecoder & dec;
        std::map<Address, Dyninst::InstructionAPI::Instruction::Ptr> allInsns;
        Dyninst::InstructionAPI::Instruction::Ptr curInsn() const;
        std::map<Address, Dyninst::InstructionAPI::Instruction::Ptr>::iterator curInsnIter;
        mutable bool validCFT;
        mutable std::pair<bool, Address> cachedCFT;
        mutable bool validLinkerStubState;
        mutable bool cachedLinkerStubState;
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
