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

#include "IA_riscv64.h"
#include "Register.h"
#include "Dereference.h"
#include "Immediate.h"
#include "BinaryFunction.h"

#include "common/src/arch.h"
#include "registers/riscv64_regs.h"

#include "parseAPI/src/debug_parse.h"

#include <deque>
#include <iostream>
#include <sstream>
#include <functional>
#include <algorithm>
#include <set>

#include <boost/shared_ptr.hpp>

using namespace Dyninst;
using namespace InstructionAPI;
using namespace Dyninst::ParseAPI;
using namespace Dyninst::InsnAdapter;

IA_riscv64::IA_riscv64(Dyninst::InstructionAPI::InstructionDecoder dec_,
               Address start_,
	       Dyninst::ParseAPI::CodeObject* o,
	       Dyninst::ParseAPI::CodeRegion* r,
	       Dyninst::InstructionSource *isrc,
	       Dyninst::ParseAPI::Block * curBlk_):
	           IA_IAPI(dec_, start_, o, r, isrc, curBlk_) {
}
IA_riscv64::IA_riscv64(const IA_riscv64& rhs): IA_IAPI(rhs) {}

IA_riscv64* IA_riscv64::clone() const {
    return new IA_riscv64(*this);
}

bool IA_riscv64::isFrameSetupInsn(Instruction i) const
{
    // sd ra, 8(sp)     (c.sdsp ra, 0x8(sp))
    // sd s0, 0(sp)     (c.sdsp s0, sp)

    entryID eid = i.getOperation().getID();
    if (eid == riscv64_op_c_sdsp || eid == riscv64_op_sd) {
        RegisterAST::Ptr op0 = boost::dynamic_pointer_cast<RegisterAST>(i.getOperand(0).getValue());
        Dereference::Ptr op1 = boost::dynamic_pointer_cast<Dereference>(i.getOperand(1).getValue());

        std::vector<Expression::Ptr> derefChildren;
        op1->getChildren(derefChildren);
        BinaryFunction::Ptr deref = boost::dynamic_pointer_cast<BinaryFunction>(derefChildren[0]);
        std::vector<Expression::Ptr> addFunc;
        deref->getChildren(addFunc);
        RegisterAST::Ptr addFunc0 = boost::dynamic_pointer_cast<RegisterAST>(addFunc[0]);
        Immediate::Ptr addFunc1 = boost::dynamic_pointer_cast<Immediate>(addFunc[1]);

        MachRegister reg0 = op0->getID();
        MachRegister memReg1 = addFunc0->getID();
        int memOffset1 = addFunc1->eval().val.s32val;

        if (reg0 == riscv64::ra && memReg1 == riscv64::sp && memOffset1 == 0x8) {
            return true;
        }
        if (reg0 == riscv64::s0 && memReg1 == riscv64::sp && memOffset1 == 0x0) {
            return true;
        }
    }
    return false;
}

bool IA_riscv64::isNop() const
{
    Instruction insn = curInsn();
    entryID eid = insn.getOperation().getID();

    if (eid == riscv64_op_c_nop) {
        return true;
    }
    else if (eid == riscv64_op_addi) {
        RegisterAST::Ptr op0 = boost::dynamic_pointer_cast<RegisterAST>(insn.getOperand(0).getValue());
        RegisterAST::Ptr op1 = boost::dynamic_pointer_cast<RegisterAST>(insn.getOperand(1).getValue());
        Immediate::Ptr op2 = boost::dynamic_pointer_cast<Immediate>(insn.getOperand(2).getValue());
        if (op0->getID() == riscv64::zero && op1->getID() == riscv64::zero && op2->eval().val.s32val == 0) {
            return true;
        }
    }
    return false;
}

bool IA_riscv64::isThunk() const
{
    return false;
}

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
bool IA_riscv64::isTailCall(const Function* context, EdgeTypeEnum type, unsigned int,
        const std::set<Address>& knownTargets ) const
{
    // TODO tail call in RISC-V
    return false;
}
#pragma GCC diagnostic pop

bool IA_riscv64::savesFP() const
{
    Instruction insn = curInsn();
    entryID eid = insn.getOperation().getID();
    if (eid == riscv64_op_addi) {
        // addi sp, sp, -n
        RegisterAST::Ptr op0 = boost::dynamic_pointer_cast<RegisterAST>(insn.getOperand(0).getValue());
        RegisterAST::Ptr op1 = boost::dynamic_pointer_cast<RegisterAST>(insn.getOperand(1).getValue());
        Immediate::Ptr op2 = boost::dynamic_pointer_cast<Immediate>(insn.getOperand(2).getValue());

        MachRegister reg0 = op0->getID();
        MachRegister reg1 = op1->getID();
        int offset2 = op1->eval().val.s32val;

        if (reg0 == riscv64::sp && reg1 == riscv64::sp && offset2 < 0) {
            return true;
        }
    }
    else if (eid == riscv64_op_c_addi || eid == riscv64_op_c_addi16sp) {
        // c.addi16sp sp, n
        RegisterAST::Ptr op0 = boost::dynamic_pointer_cast<RegisterAST>(insn.getOperand(0).getValue());
        Immediate::Ptr op1 = boost::dynamic_pointer_cast<Immediate>(insn.getOperand(1).getValue());

        MachRegister reg0 = op0->getID();
        int offset1 = op1->eval().val.s32val;

        if (reg0 == riscv64::sp && offset1 < 0) {
            return true;
        }
    }
    return false;
}

bool IA_riscv64::isStackFramePreamble() const
{
    // addi s0, sp, -n  (c.addi sp, -n)
    // sd ra, 8(sp)     (c.sdsp ra, 0x8(sp))
    // sd s0, 0(sp)     (c.sdsp s0, sp)
    // addi s0, sp, n   (c.addi4spn s0, sp, n)

    Instruction insn = curInsn();
    if (!savesFP()) { // check c.addi sp, -16
        return false;
    }
    entryID eid = insn.getOperation().getID();
    InstructionDecoder tmp(dec);
    for (int i = 0; i < 2; ++i) {
        insn = tmp.decode();
        if (!isFrameSetupInsn(insn)) {
            return false;
        }
    }

    // TODO: no need to check c.addi2spn ?
    return true;
}

bool IA_riscv64::cleansStack() const
{
    Instruction insn = curInsn();
    entryID eid = insn.getOperation().getID();

    // addi sp, sp, n
    if (eid == riscv64_op_addi) {
        // addi sp, sp, n
        RegisterAST::Ptr op0 = boost::dynamic_pointer_cast<RegisterAST>(insn.getOperand(0).getValue());
        RegisterAST::Ptr op1 = boost::dynamic_pointer_cast<RegisterAST>(insn.getOperand(1).getValue());
        Immediate::Ptr op2 = boost::dynamic_pointer_cast<Immediate>(insn.getOperand(2).getValue());

        MachRegister reg0 = op0->getID();
        MachRegister reg1 = op1->getID();
        int offset2 = op1->eval().val.s32val;

        if (reg0 == riscv64::sp && reg1 == riscv64::sp && offset2 > 0) {
            return true;
        }
    }
    else if (eid == riscv64_op_c_addi || eid == riscv64_op_c_addi16sp) {
        // c.addi sp, n
        // c.addi16sp sp, n
        RegisterAST::Ptr op0 = boost::dynamic_pointer_cast<RegisterAST>(insn.getOperand(0).getValue());
        Immediate::Ptr op1 = boost::dynamic_pointer_cast<Immediate>(insn.getOperand(1).getValue());

        MachRegister reg0 = op0->getID();
        int offset1 = op1->eval().val.s32val;

        if (reg0 == riscv64::sp && offset1 > 0) {
            return true;
        }
    }
    return false;
}

bool IA_riscv64::sliceReturn(ParseAPI::Block*, Address, ParseAPI::Function *) const
{
    return true;
}

bool IA_riscv64::isReturnAddrSave(Address&) const
{
  return false;
}

bool IA_riscv64::isReturn(Dyninst::ParseAPI::Function *, Dyninst::ParseAPI::Block*) const
{
    Instruction insn = curInsn();
    entryID eid = insn.getOperation().getID();
    if (eid == riscv64_op_jalr) {
        // jalr zero, ra, 0
        RegisterAST::Ptr op0 = boost::dynamic_pointer_cast<RegisterAST>(insn.getOperand(0).getValue());
        RegisterAST::Ptr op1 = boost::dynamic_pointer_cast<RegisterAST>(insn.getOperand(1).getValue());
        Immediate::Ptr op2 = boost::dynamic_pointer_cast<Immediate>(insn.getOperand(2).getValue());

        MachRegister reg0 = op0->getID();
        MachRegister reg1 = op1->getID();
        int offset2 = op2->eval().val.s32val;

        if (reg0 == riscv64::zero && reg1 == riscv64::ra && offset2 == 0) {
            return true;
        }
    }
    else if (eid == riscv64_op_c_jr) {
        // jalr zero, ra, 0
        RegisterAST::Ptr op0 = boost::dynamic_pointer_cast<RegisterAST>(insn.getOperand(0).getValue());

        MachRegister reg0 = op0->getID();

        if (reg0 == riscv64::ra) {
            return true;
        }
    }
    return false;
}

bool IA_riscv64::isFakeCall() const
{
    return false;
}

bool IA_riscv64::isIATcall(std::string &) const
{
    return false;
}

bool IA_riscv64::isLinkerStub() const
{
  // Disabling this code because it ends with an
    // incorrect CFG.
    return false;
}

#if 0
ParseAPI::StackTamper
IA_riscv64::tampersStack(ParseAPI::Function *, Address &) const
{
    return TAMPER_NONE;
}
#endif

bool IA_riscv64::isNopJump() const
{
    return false;
}
