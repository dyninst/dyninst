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

#include "common/src/arch-riscv64.h"
#include "registers/riscv64_regs.h"
#include "parseAPI/src/debug_parse.h"

#include <set>
#include "Register.h"
#include "Dereference.h"
#include "SymbolicExpression.h"
#include "SymEval.h"

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
    // sd linkreg, 8(sp)  (c.sdsp linkreg, 0x8(sp))
    // sd s0, 0(sp)       (c.sdsp s0, sp)

    entryID eid = i.getOperation().getID();
    if (eid == riscv64_op_sd) {
        RegisterAST::Ptr op0 = boost::dynamic_pointer_cast<RegisterAST>(i.getOperand(0).getValue());
        Dereference::Ptr op1 = boost::dynamic_pointer_cast<Dereference>(i.getOperand(1).getValue());

        std::vector<Expression::Ptr> derefChildren = op1->getSubexpressions();
        BinaryFunction::Ptr deref = boost::dynamic_pointer_cast<BinaryFunction>(derefChildren[0]);
        std::vector<Expression::Ptr> addFunc = deref->getSubexpressions();
        RegisterAST::Ptr addFunc0 = boost::dynamic_pointer_cast<RegisterAST>(addFunc[0]);
        Immediate::Ptr addFunc1 = boost::dynamic_pointer_cast<Immediate>(addFunc[1]);

        MachRegister memReg1 = addFunc0->getID();
        int memOffset1 = addFunc1->eval().val.s32val;

        if (memReg1 == riscv64::sp && memOffset1 == 0x8) {
            return true;
        }
        if (memReg1 == riscv64::sp && memOffset1 == 0x0) {
            return true;
        }
    }
    return false;
}

bool IA_riscv64::isIndirectJump() const
{
    Instruction ci = curInsn();
    if (ci.isBranch() || ci.allowsFallThrough() || ci.isReturn()) {
        return false;
    }
    bool valid;
    Address target;
    boost::tie(valid, target) = getCFT();
    if (valid) {
        return false;
    }
    parsing_printf("... indirect jump at 0x%lx, delay parsing it\n", current);
    return true;
}

bool IA_riscv64::isNop() const
{
    Instruction insn = curInsn();
    entryID eid = insn.getOperation().getID();

    if (eid == riscv64_op_addi) {
        MachRegister rd = boost::dynamic_pointer_cast<RegisterAST>(insn.getOperand(0).getValue())->getID();
        MachRegister rs = boost::dynamic_pointer_cast<RegisterAST>(insn.getOperand(1).getValue())->getID();
        int32_t imm = boost::dynamic_pointer_cast<Immediate>(insn.getOperand(2).getValue())->eval().val.s32val;
        if (rd == riscv64::zero && rs == riscv64::zero && imm == 0) {
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
    switch(type) {
        case COND_TAKEN:
        case DIRECT:
        case INDIRECT:
            type = DIRECT;
            break;
        case CALL:
        case RET:
        case COND_NOT_TAKEN:
        case FALLTHROUGH:
        case CALL_FT:
        default:
            return false;
    }
    parsing_printf("Checking for Tail Call from RISCV\n");
    context->obj()->cs()->incrementCounter(PARSE_TAILCALL_COUNT);

    if (tailCalls.find(type) != tailCalls.end()) {
        parsing_printf("\tReturning cached tail call check result: %d\n", tailCalls[type]);
        if (tailCalls[type]) {
            context->obj()->cs()->incrementCounter(PARSE_TAILCALL_FAIL);
            return true;
        }
        return false;
    }

    bool valid; Address addr;
    boost::tie(valid, addr) = getCFT();

    Function *callee = _obj->findFuncByEntry(_cr, addr);
    Block *target = _obj->findBlockByEntry(_cr, addr);

    if(curInsn().isBranch() &&
            valid &&
            callee && 
            callee != context &&
            // We can only trust entry points from hints
            callee->src() == HINT &&
            /* the target can either be not parsed or not within the current context */
            ((target == NULL) || (target && !context->contains(target)))
      )
    {
        parsing_printf("\tjump to 0x%lx, TAIL CALL\n", addr);
        tailCalls[type] = true;
        return true;
    }

    if (valid && addr > 0 && !context->region()->contains(addr)) {
        parsing_printf("\tjump to 0x%lx in other regions, TAIL CALL\n", addr);
        tailCalls[type] = true;
        return true;
    }

    if (curInsn().isBranch() &&
            valid &&
            !callee) {
        if (knownTargets.find(addr) != knownTargets.end()) {
            parsing_printf("\tjump to 0x%lx is known target in this function, NOT TAIL CALL\n", addr);
            tailCalls[type] = false;
            return false;
        }
    }
    if(allInsns.size() < 2) {
        parsing_printf("\ttoo few insns to detect tail call\n");
        context->obj()->cs()->incrementCounter(PARSE_TAILCALL_FAIL);
        tailCalls[type] = false;
        return false;
    }
    tailCalls[type] = false;
    context->obj()->cs()->incrementCounter(PARSE_TAILCALL_FAIL);
    return false;
}
#pragma GCC diagnostic pop

bool IA_riscv64::savesFP() const
{
    Instruction insn = curInsn();
    entryID eid = insn.getOperation().getID();
    if (eid == riscv64_op_addi) {
        // addi sp, sp, -n
        MachRegister rd = boost::dynamic_pointer_cast<RegisterAST>(insn.getOperand(0).getValue())->getID();
        MachRegister rs = boost::dynamic_pointer_cast<RegisterAST>(insn.getOperand(1).getValue())->getID();
        int32_t imm = boost::dynamic_pointer_cast<Immediate>(insn.getOperand(2).getValue())->eval().val.s32val;

        if (rd == riscv64::sp && rs == riscv64::sp && imm < 0) {
            return true;
        }
    }
    return false;
}

bool IA_riscv64::isStackFramePreamble() const
{
    // addi s0, sp, -n    (c.addi sp, -n)
    // sd linkreg, 8(sp)  (c.sdsp linkreg, 0x8(sp))
    // sd s0, 0(sp)       (c.sdsp s0, sp)

    Instruction insn = curInsn();
    // check c.addi sp, -16
    if (!savesFP()) {
        return false;
    }
    InstructionDecoder tmp(dec);
    for (int i = 0; i < 2; ++i) {
        insn = tmp.decode();
        // check c.sdsp ra, 0x8(sp) and c.sdsp s0, sp
        // The order of both stores does not matter
        if (!isFrameSetupInsn(insn)) {
            return false;
        }
    }

    return true;
}

bool IA_riscv64::cleansStack() const
{
    Instruction insn = curInsn();
    entryID eid = insn.getOperation().getID();

    // addi sp, sp, n
    if (eid == riscv64_op_addi) {
        // addi sp, sp, n
        MachRegister rd = boost::dynamic_pointer_cast<RegisterAST>(insn.getOperand(0).getValue())->getID();
        MachRegister rs = boost::dynamic_pointer_cast<RegisterAST>(insn.getOperand(1).getValue())->getID();
        int32_t imm = boost::dynamic_pointer_cast<Immediate>(insn.getOperand(2).getValue())->eval().val.s32val;

        if (rd == riscv64::sp && rs == riscv64::sp && imm > 0) {
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

bool IA_riscv64::isReturn(Dyninst::ParseAPI::Function *func, Dyninst::ParseAPI::Block* currBlk) const
{
    Instruction insn = curInsn();
    entryID eid = insn.getOperation().getID();

    if (eid == riscv64_op_jalr) {
        // jalr zero, reg, 0
        MachRegister rd = boost::dynamic_pointer_cast<RegisterAST>(insn.getOperand(0).getValue())->getID();
        MachRegister rs = boost::dynamic_pointer_cast<RegisterAST>(insn.getOperand(1).getValue())->getID();
        int32_t imm = boost::dynamic_pointer_cast<Immediate>(insn.getOperand(2).getValue())->eval().val.s32val;
        if (rd == riscv64::x0 && imm == 0) {
            MachRegister linkReg = rs;
            // If reg is ra, it is return
            if (linkReg == riscv64::ra) {
                return true;
            }

            // Otherwise, check whether it is non-ABI return
            std::map<Offset, InstructionAPI::Instruction> insns;
            func->entry()->getInsns(insns);

            // Check whether the jump register matches the stored link register in the function prologue
            auto iter = insns.begin();
            for (int i = 0; i < 2 && iter != insns.end(); i++) {
                Instruction frameInsn = iter->second;
                eid = frameInsn.getOperation().getID();
                if (eid == riscv64_op_sd) {
                    RegisterAST::Ptr op0 = boost::dynamic_pointer_cast<RegisterAST>(frameInsn.getOperand(0).getValue());
                    Dereference::Ptr op1 = boost::dynamic_pointer_cast<Dereference>(frameInsn.getOperand(1).getValue());

                    std::vector<Expression::Ptr> derefChildren = op1->getSubexpressions();
                    BinaryFunction::Ptr deref = boost::dynamic_pointer_cast<BinaryFunction>(derefChildren[0]);
                    std::vector<Expression::Ptr> addFunc = deref->getSubexpressions();
                    RegisterAST::Ptr addFunc0 = boost::dynamic_pointer_cast<RegisterAST>(addFunc[0]);
                    Immediate::Ptr addFunc1 = boost::dynamic_pointer_cast<Immediate>(addFunc[1]);

                    MachRegister memReg1 = addFunc0->getID();
                    int memOffset1 = addFunc1->eval().val.s32val;

                    if (memReg1 == riscv64::sp && memOffset1 == 0x8 && linkReg == op0->getID()) {
                        return true;
                    }
                }
                iter++;
            }
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

bool IA_riscv64::isNopJump() const
{
    return false;
}

bool IA_riscv64::isMultiInsnJump(Address *target, Function *context, Block *currBlk) const {
    Instruction insn = curInsn();

    // Basic Instruction Matching
    // This is used to prevent calling slicing all the time
    // Slicing is not as efficient as instruction pattern matching
    entryID curInsnID = insn.getOperation().getID();

    if (curInsnID == riscv64_op_jalr) {
        Address addr = current;

        MachRegister targetReg = boost::dynamic_pointer_cast<RegisterAST>(insn.getOperand(1).getValue())->getID();
        Address offset = boost::dynamic_pointer_cast<Immediate>(insn.getOperand(2).getValue())->eval().val.s32val;

        IA_riscv64* cloned = this->clone();
        cloned->retreat();
        Instruction pi = cloned->curInsn();

        // In RISC-V where sometimes the offset is too large to fit into the instruction's immediate field of jal,
        // Compilers will typically generate the following instruction sequence
        //   auipc	ra,0xffe00
        //   jalr	-42(ra)
        // The following code performs instruction pattern matching on such code snippet

        // If the previous instruction is indeed `auipc`, unwrap all operands and calculate the target address
        if (pi.getOperation().getID() == riscv64_op_auipc) {
            MachRegister auipcRd = boost::dynamic_pointer_cast<RegisterAST>(pi.getOperand(0).getValue())->getID();
            int32_t auipcImm = boost::dynamic_pointer_cast<Immediate>(pi.getOperand(1).getValue())->eval().val.s32val;

            if (auipcRd == targetReg) {
                // The offset of auipc is (imm << 12), so we add (imm << 12) to the target address
                addr += (auipcImm << 12);

                // Add the offset to relAddr
                addr += offset;
                // minus the length of the auipc instruction
                addr -= pi.size();

                *target = addr;
                return true;
            }
        }

        // For some RISC-V shared libraries, PLT entries are not generated for exported functions
        // Instead, it loads the address from .rela.dyn and calls it directly without going through PLT
        // An example code snippet that loads from .rela.dyn is as follows
        //   auipc   t1,0xa
        //   ld      t1,1498(t1)
        //   jalr    t1

        else if (pi.getOperation().getID() == riscv64_op_ld) {
            MachRegister ldRd = (boost::dynamic_pointer_cast<RegisterAST>(pi.getOperand(0).getValue()))->getID();
            vector<InstructionAST::Ptr> children1 =
                boost::dynamic_pointer_cast<Dereference>(pi.getOperand(1).getValue())->getSubexpressions();
            vector<InstructionAST::Ptr> children2 =
                boost::dynamic_pointer_cast<BinaryFunction>(children1[0])->getSubexpressions();
            MachRegister ldRs = boost::dynamic_pointer_cast<RegisterAST>(children2[0])->getID();
            int32_t ldImm = boost::dynamic_pointer_cast<Immediate>(children2[1])->eval().val.s32val;

            // The register operands should be the same as the target register
            if (ldRd == targetReg && ldRs == targetReg) {
                // Add the offset to the target address
                addr += ldImm;

                // If the previous second instruction is indeed `auipc`, unwrap all operands and check them
                cloned->retreat();
                Instruction pi2 = cloned->curInsn();
                if (pi2.getOperation().getID() == riscv64_op_auipc) {
                    MachRegister auipcRd = boost::dynamic_pointer_cast<RegisterAST>(pi2.getOperand(0).getValue())->getID();
                    int32_t auipcImm = boost::dynamic_pointer_cast<Immediate>(pi2.getOperand(1).getValue())->eval().val.s32val;

                    if (auipcRd == targetReg) {
                        // The offset of auipc is (imm << 12), so we add (imm << 12) to the target address
                        addr += (auipcImm << 12);
                        // minus the length of the auipc and ld instruction
                        addr -= pi.size() + pi2.size();
                        // Dereference addr and check if the address is in .rela.dyn
                        CodeSource *cs = currBlk->obj()->cs();
                        std::map< Address, std::pair<std::string, Address>> reladyn_linkage = cs->reladyn_linkage();
                        if (reladyn_linkage.find(addr) != reladyn_linkage.end()) {
                            *target = reladyn_linkage[addr].second;
                            return true;
                        }
                    }
                }
            }
        }

        // Add more instruction patterns here
        //
        // ...
        //

        // Unknown instruction pattern
        // Perform backward slicing as the last resort
        // This should not occur frequently. If it does, consider adding the instruction pattern.
        AssignmentConverter ac(true, false);
        vector<Assignment::Ptr> assignments;
        ac.convert(insn, currBlk->last(), context, currBlk, assignments);
        Slicer formatSlicer(assignments[0], currBlk, context, false, false);

        SymbolicExpression se;
        se.cs = currBlk->obj()->cs();
        se.cr = currBlk->region();

        Slicer::Predicates preds;
        Graph::Ptr slice = formatSlicer.backwardSlice(preds);
        DataflowAPI::Result_t symRet;
        DataflowAPI::SymEval::expand(slice, symRet);

        auto origAST = symRet[assignments[0]];
        if (origAST != NULL) {
            auto simplifiedAST = se.SimplifyAnAST(origAST, 0, false);
            if (simplifiedAST->getID() == AST::V_ConstantAST) {
                DataflowAPI::ConstantAST::Ptr constAST = boost::static_pointer_cast<DataflowAPI::ConstantAST>(simplifiedAST);
                uint32_t evalAddr = constAST->val().val;
                *target = evalAddr;
                return true;
            }
        }
    }
    return false;
}
