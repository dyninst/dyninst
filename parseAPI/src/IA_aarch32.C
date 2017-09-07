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

#include "IA_IAPI.h"

#include "Register.h"
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

using namespace Dyninst;
using namespace InstructionAPI;
using namespace Dyninst::ParseAPI;
using namespace Dyninst::InsnAdapter;

bool IA_IAPI::isFrameSetupInsn(Instruction::Ptr i) const
{
    return false;
}

bool IA_IAPI::isNop() const
{
    return false;
}

bool IA_IAPI::isThunk() const
{
    return false;
}

bool IA_IAPI::isTailCall(Function* context,
                         EdgeTypeEnum type,
                         unsigned int,
                         const std::set<Address>& knownTargets ) const
{
    return false;
}

bool IA_IAPI::savesFP() const
{
    return false;
}

bool IA_IAPI::isStackFramePreamble() const
{
    return false;
}

bool IA_IAPI::cleansStack() const
{
    return false;
}

bool IA_IAPI::sliceReturn(ParseAPI::Block* bit, Address ret_addr, ParseAPI::Function * func) const
{
    return true;
}

bool IA_IAPI::isReturnAddrSave(Address& retAddr) const
{
  return false;
}

bool IA_IAPI::isReturn(Dyninst::ParseAPI::Function * context, Dyninst::ParseAPI::Block* currBlk) const
{
    return false;
}

bool IA_IAPI::isFakeCall() const
{
    return false;
}

bool IA_IAPI::isIATcall(std::string &) const
{
    return false;
}

bool IA_IAPI::isLinkerStub() const
{
  // Disabling this code because it ends with an
    // incorrect CFG.
    return false;
}

#if 0
ParseAPI::StackTamper
IA_IAPI::tampersStack(ParseAPI::Function *, Address &) const
{
    return TAMPER_NONE;
}
#endif

bool IA_IAPI::isNopJump() const
{
    return false;
}
