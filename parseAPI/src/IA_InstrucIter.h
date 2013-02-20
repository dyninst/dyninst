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

#if !defined(IA_INSTRUCITER_H)
#define IA_INSTRUCITER_H

#include "parseAPI/h/InstructionAdapter.h"
#include "parseAPI/h/CFG.h"

#include "InstrucIter.h"

namespace Dyninst {
namespace InsnAdapter {

class IA_InstrucIter : public InstructionAdapter
{
    public:
        IA_InstrucIter(InstrucIter from, 
                       Dyninst::ParseAPI::CodeObject * o,
                       Dyninst::ParseAPI::CodeRegion * r,
                       Dyninst::InstructionSource * isrc);
        virtual ~IA_InstrucIter() { }
        instruction getInstruction();

        virtual bool hasCFT() const;
        virtual size_t getSize() const;
        virtual bool isFrameSetupInsn() const;
        virtual bool isAbort() const;
        virtual bool isInvalidInsn() const;

        virtual void
                getNewEdges(std::vector<std::pair< Address, 
                Dyninst::ParseAPI::EdgeTypeEnum> >&
                outEdges, 
                Dyninst::ParseAPI::Function* context,
                Dyninst::ParseAPI::Block* currBlk,
                unsigned int num_insns,
                dyn_hash_map<Address, std::string> *pltFuncs) const;
        virtual bool isDynamicCall() const;
        virtual bool isAbsoluteCall() const;
        virtual bool simulateJump() const;
        virtual bool isRelocatable(InstrumentableLevel lvl) const;
        virtual void advance();
        virtual void retreat();
        virtual bool isNop() const;
        virtual bool isLeave() const;
        virtual bool isDelaySlot() const;
        virtual bool isTailCall(Dyninst::ParseAPI::Function*, Dyninst::ParseAPI::EdgeTypeEnum, unsigned int num_insns) const;
        virtual Address getCFT() const;
        virtual bool isStackFramePreamble() const;
        virtual bool savesFP() const;
        virtual bool cleansStack() const;
        virtual bool isConditional() const;
        virtual bool isBranch() const;
        virtual bool isInterruptOrSyscall() const;
        virtual bool isCall() const;
        virtual bool isReturnAddrSave() const;   
    private:
        virtual bool isRealCall() const;
        virtual bool isReturn() const;
        
        mutable InstrucIter ii;
        
};

}
}


#endif // !defined(IA_INSTRUCITER_H)
