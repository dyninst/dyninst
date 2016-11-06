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

//#warning "The reg defines are not correct now!"
static RegisterAST::Ptr aarch64_R11(new RegisterAST(aarch64::x11));
static RegisterAST::Ptr aarch64_LR(new RegisterAST(aarch64::x30));
// SP is an independent reg in aarch64
static RegisterAST::Ptr aarch64_SP(new RegisterAST(aarch64::sp));

bool IA_IAPI::isFrameSetupInsn(Instruction::Ptr i) const {
  if (i->getOperation().getID() == aarch64_op_mov_add_addsub_imm) {
    if (i->readsMemory() || i->writesMemory()) {
      parsing_printf(
          "%s[%d]: discarding insn %s as stack frame preamble, not a reg-reg "
          "move\n",
          FILE__, __LINE__, i->format().c_str());

      return false;
    }
    if (i->isRead(stackPtr[_isrc->getArch()]) &&
        i->isWritten(framePtr[_isrc->getArch()])) {
      return true;
    }
  }
}

bool IA_IAPI::isNop() const {
  Instruction::Ptr ci = curInsn();

  if (ci->getOperation().getID() == aarch64_op_nop_hint) return true;

  return false;
}

bool IA_IAPI::isThunk() const { return false; }

bool IA_IAPI::isTailCall(Function* context, EdgeTypeEnum type, unsigned int,
                         const std::set<Address>&) const {
  switch (type) {
    case CALL:
    case COND_TAKEN:
    case DIRECT:
    case INDIRECT:
      type = DIRECT;
      break;
    case COND_NOT_TAKEN:
    case FALLTHROUGH:
    case CALL_FT:
    case RET:
    default:
      return false;
  }

  parsing_printf("Checking for Tail Call \n");
  context->obj()->cs()->incrementCounter(PARSE_TAILCALL_COUNT);
  parsing_printf("***type: %d, current: %d\n", type, tailCalls[type]);

  if (tailCalls.find(type) != tailCalls.end()) {
    parsing_printf("\tReturning cached tail call check result: %d\n",
                   tailCalls[type]);
    if (tailCalls[type]) {
      context->obj()->cs()->incrementCounter(PARSE_TAILCALL_FAIL);
      return true;
    }
    return false;
  }

  bool valid;
  Address addr;
  boost::tie(valid, addr) = getCFT();

  Function* callee = _obj->findFuncByEntry(_cr, addr);
  Block* target = _obj->findBlockByEntry(_cr, addr);

  if (curInsn()->getCategory() == c_BranchInsn && valid && callee &&
      callee != context && !context->contains(target)) {
    parsing_printf("\tjump to 0x%lx, TAIL CALL\n", addr);
    tailCalls[type] = true;
    return true;
  }

  if (allInsns.size() < 2) {
    parsing_printf("\ttoo few insns to detect tail call\n");
    context->obj()->cs()->incrementCounter(PARSE_TAILCALL_FAIL);
    tailCalls[type] = false;
    return false;
  }
  tailCalls[type] = false;
  context->obj()->cs()->incrementCounter(PARSE_TAILCALL_FAIL);
  return false;
}

bool IA_IAPI::savesFP() const {
  Instruction::Ptr insn = curInsn();
  RegisterAST::Ptr returnAddrReg(new RegisterAST(aarch64::x30));

  // stp x29, x30, [sp, imm]!
  if (insn->getOperation().getID() == aarch64_op_stp_gen &&
      insn->isRead(framePtr[_isrc->getArch()]) && insn->isRead(returnAddrReg) &&
      insn->isRead(stackPtr[_isrc->getArch()]) &&
      insn->isWritten(stackPtr[_isrc->getArch()]))
    return true;

  return false;
}

bool IA_IAPI::isStackFramePreamble() const {
  if (!savesFP()) return false;

  InstructionDecoder tmp(dec);
  if (isFrameSetupInsn(tmp.decode())) return true;

  return false;
}

bool IA_IAPI::cleansStack() const {
  Instruction::Ptr insn = curInsn();
  RegisterAST::Ptr returnAddrReg(new RegisterAST(aarch64::x30));

  // ldp x29, x30, [sp], imm
  if (insn->getOperation().getID() == aarch64_op_ldp_gen &&
      insn->isWritten(framePtr[_isrc->getArch()]) &&
      insn->isWritten(returnAddrReg) &&
      insn->isRead(stackPtr[_isrc->getArch()]) &&
      insn->isWritten(stackPtr[_isrc->getArch()]))
    return true;

  return false;
}

bool IA_IAPI::sliceReturn(ParseAPI::Block* bit, Address ret_addr,
                          ParseAPI::Function* func) const {
  return true;
}

bool IA_IAPI::isReturnAddrSave(Address& retAddr) const { return false; }

bool IA_IAPI::isReturn(Dyninst::ParseAPI::Function* context,
                       Dyninst::ParseAPI::Block* currBlk) const {
  return curInsn()->getCategory() == c_ReturnInsn;
}

bool IA_IAPI::isFakeCall() const { return false; }

bool IA_IAPI::isIATcall(std::string&) const { return false; }

bool IA_IAPI::isLinkerStub() const {
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

bool IA_IAPI::isNopJump() const { return false; }
