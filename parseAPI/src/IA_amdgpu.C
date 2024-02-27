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

#include "common/src/arch.h"

#include "parseAPI/src/debug_parse.h"

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
    return curInsn().getCategory() == c_ReturnInsn;
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
