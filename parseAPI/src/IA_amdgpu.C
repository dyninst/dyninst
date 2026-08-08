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

#include "IA_amdgpu.h"
#include "Dereference.h"
#include "Immediate.h"
#include "BinaryFunction.h"

#include "parseAPI/src/IndirectAnalyzer.h"
#include "parseAPI/src/debug_parse.h"

#include "registers/AMDGPU/amdgpu_gfx908_regs.h"
#include "registers/AMDGPU/amdgpu_gfx90a_regs.h"
#include "registers/AMDGPU/amdgpu_gfx940_regs.h"

#include <deque>
#include <iostream>
#include <sstream>
#include <functional>
#include <algorithm>
#include <set>
#include "Register.h"

using namespace Dyninst;
using namespace InstructionAPI;
using namespace Dyninst::ParseAPI;
using namespace Dyninst::InsnAdapter;

IA_amdgpu::IA_amdgpu(Dyninst::InstructionAPI::InstructionDecoder dec_,
               Address start_, 
	       Dyninst::ParseAPI::CodeObject* o,
	       Dyninst::ParseAPI::CodeRegion* r,
	       Dyninst::InstructionSource *isrc,
	       Dyninst::ParseAPI::Block * curBlk_):
	           IA_IAPI(dec_, start_, o, r, isrc, curBlk_) {
}		   
IA_amdgpu::IA_amdgpu(const IA_amdgpu& rhs): IA_IAPI(rhs) {}

IA_amdgpu* IA_amdgpu::clone() const {
    return new IA_amdgpu(*this);
}

bool IA_amdgpu::isFrameSetupInsn(Instruction ) const
{
    return false;
}


bool IA_amdgpu::isNop() const
{
    Instruction ci = curInsn();
    auto id = ci.getOperation().getID();
    if(id == amdgpu_gfx908_op_S_NOP || 
        id == amdgpu_gfx90a_op_S_NOP || id == amdgpu_gfx940_op_S_NOP)
        return true;
    return false;
}

bool IA_amdgpu::isThunk() const 
{
    return false;
}

bool IA_amdgpu::isTailCall(const Function*, EdgeTypeEnum , unsigned int,
        const std::set<Address>&  ) const
{
   return false;
}

bool IA_amdgpu::savesFP() const
{
    return false;
}

bool IA_amdgpu::isStackFramePreamble() const
{
    return false;
}

bool IA_amdgpu::cleansStack() const
{
   return false;
}

bool IA_amdgpu::sliceReturn(ParseAPI::Block* , Address , ParseAPI::Function * ) const
{
    return true;
}

bool IA_amdgpu::isReturnAddrSave(Address& ) const
{
  return false;
}

bool IA_amdgpu::isReturn(Dyninst::ParseAPI::Function * , Dyninst::ParseAPI::Block*) const
{
    Instruction ci = curInsn();
    auto id = ci.getOperation().getID();
    if (id == amdgpu_gfx908_op_S_SETPC_B64 ||
        id == amdgpu_gfx90a_op_S_SETPC_B64 ||
        id == amdgpu_gfx940_op_S_SETPC_B64) {
        std::set<RegisterAST::Ptr> reads;
        ci.getReadSet(reads);
        for (auto const& r : reads) {
            MachRegister mr = r->getID();
            if (mr == amdgpu_gfx908::s30 ||
                mr == amdgpu_gfx90a::s30 ||
                mr == amdgpu_gfx940::s30)
                return true;
        }
        return false;
    }
    return ci.isReturn();
}

bool IA_amdgpu::isFakeCall() const
{
    return false;
}

bool IA_amdgpu::isIATcall(std::string &) const
{
    return false;
}

bool IA_amdgpu::isLinkerStub() const
{
  // Disabling this code because it ends with an
    // incorrect CFG.
    return false;
}

bool IA_amdgpu::isNopJump() const
{
    return false;
}

bool IA_amdgpu::isMultiInsnJump() const
{
    switch (curInsn().getOperation().getID()) {
        case amdgpu_gfx908_op_S_SWAPPC_B64:
        case amdgpu_gfx90a_op_S_SWAPPC_B64:
        case amdgpu_gfx940_op_S_SWAPPC_B64:
            return true;
        default:
            return false;
    }
}

bool IA_amdgpu::isSoftwareException() const
{
    Instruction ci = curInsn();
    auto id = ci.getOperation().getID();
    if(id != amdgpu_gfx908_op_S_TRAP &&
       id != amdgpu_gfx90a_op_S_TRAP &&
       id != amdgpu_gfx940_op_S_TRAP)
        return IA_IAPI::isSoftwareException();

    // s_trap takes a 16-bit immediate whose low 8 bits are the trap ID.
    //
    // The per-trap-ID behavior is defined by the AMDGPU trap handler ABI in the
    // LLVM AMDGPU backend documentation:
    //   https://llvm.org/docs/AMDGPUUsage.html#trap-handler-abi
    // (see the "AMDGPU Trap Handler for AMDHSA OS Code Object V3 / V4 and Above"
    // tables). Summarizing the relevant trap IDs:
    //   - 0x01 (debugger breakpoint): the wave is halted and the debugger
    //     resumes it, re-executing the overwritten instruction. Execution
    //     continues, so the fall-through must be preserved.
    //   - 0x02 (llvm.trap): the wave is halted and its queue is put into the
    //     error state, terminating the dispatch's waves. It does not return,
    //     so this is the only s_trap that ends the block with no fall-through.
    //   - 0x03 (llvm.debugtrap): a no-op when no debugger is attached, otherwise
    //     reported to the debugger which resumes the wave. Either way execution
    //     continues, so the fall-through must be preserved.
    // All other trap IDs are reserved and are left to fall through.
    Immediate::Ptr imm =
        boost::dynamic_pointer_cast<Immediate>(ci.getOperand(0).getValue());
    if(!imm)
        return false;
    Result trapId = imm->eval();
    return trapId.defined && ((trapId.val.s16val & 0xff) == 0x02);
}
