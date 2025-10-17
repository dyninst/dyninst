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

#include <stdlib.h>
#include "dyninstAPI/src/codegen.h"
#include "dyninstAPI/src/debug.h"
#include "dyninstAPI/src/instPoint.h"
#include "dyninstAPI/src/registerSpace.h"
#include "dyninstAPI/src/addressSpace.h"
#include "dyninstAPI/src/inst-riscv64.h"
#include "dyninstAPI/src/emit-riscv64.h"
#include "dyninstAPI/src/function.h"

void insnCodeGen::generate(codeGen &gen, instruction &insn) {
    insn.flushInsnBuff();
    gen.copy(insn.ptr(), insn.size());
}

void insnCodeGen::generate(codeGen &gen, instruction &insn, unsigned position) {
    insn.flushInsnBuff();
    gen.insert(insn.ptr(), insn.size(), position);
}

// Basic RISC-V instruction type generation

void insnCodeGen::makeUTypeInsn(codeGen &gen,
                                    Dyninst::Register rd,
                                    Dyninst::RegValue imm,
                                    unsigned opcode)
{
    instruction insn;

    INSN_BUFF_SET(insn, 12, 31, imm);  // imm
    INSN_BUFF_SET(insn, 7, 11, rd);    // rd
    INSN_BUFF_SET(insn, 0, 6, opcode); // opcode

    insnCodeGen::generate(gen, insn);
}

void insnCodeGen::makeITypeInsn(codeGen &gen,
                                    Dyninst::Register rd,
                                    Dyninst::Register rs,
                                    Dyninst::RegValue imm,
                                    unsigned funct3,
                                    unsigned opcode)
{
    instruction insn;

    INSN_BUFF_SET(insn, 20, 31, imm);    // imm
    INSN_BUFF_SET(insn, 15, 19, rs);     // rs
    INSN_BUFF_SET(insn, 12, 14, funct3); // 111
    INSN_BUFF_SET(insn, 7, 11, rd);      // rd
    INSN_BUFF_SET(insn, 0, 6, opcode);   // opcode

    insnCodeGen::generate(gen, insn);
}

void insnCodeGen::makeRTypeInsn(codeGen &gen,
                                Dyninst::Register rd,
                                Dyninst::Register rs1,
                                Dyninst::Register rs2,
                                unsigned funct7,
                                unsigned funct3,
                                unsigned opcode)
{
    instruction insn;

    INSN_BUFF_SET(insn, 25, 31, funct7); // funct7
    INSN_BUFF_SET(insn, 20, 24, rs2);    // rs2
    INSN_BUFF_SET(insn, 15, 19, rs1);    // rs1
    INSN_BUFF_SET(insn, 12, 14, funct3); // funct3
    INSN_BUFF_SET(insn, 7, 11, rd);      // rd
    INSN_BUFF_SET(insn, 0, 6, opcode);   // opcode

    insnCodeGen::generate(gen, insn);
}

void insnCodeGen::makeBTypeInsn(codeGen &gen,
                                Dyninst::Register rs1,
                                Dyninst::Register rs2,
                                Dyninst::RegValue imm,
                                unsigned funct3,
                                unsigned opcode)
{
    instruction insn;

    INSN_BUFF_SET(insn, 31, 31, (imm >> 11) & 0x1); // imm[11]
    INSN_BUFF_SET(insn, 25, 30, (imm >> 4) & 0x3f); // imm[9:4]
    INSN_BUFF_SET(insn, 20, 24, rs2);               // rs2
    INSN_BUFF_SET(insn, 15, 19, rs1);               // rs1
    INSN_BUFF_SET(insn, 12, 14, funct3);            // funct3
    INSN_BUFF_SET(insn, 8, 11, imm & 0xf);          // imm[3:0]
    INSN_BUFF_SET(insn, 7, 7, (imm >> 10) & 0x1);   // imm[10]
    INSN_BUFF_SET(insn, 0, 6, opcode);              // opcode

    insnCodeGen::generate(gen, insn);
}

void insnCodeGen::makeJTypeInsn(codeGen &gen,
                                Dyninst::Register rd,
                                Dyninst::RegValue imm,
                                unsigned opcode)
{
    instruction insn;

    INSN_BUFF_SET(insn, 31, 31, (imm >> 19) & 0x1);  // imm[19]
    INSN_BUFF_SET(insn, 21, 30, (imm & 0x3ff));      // imm[9:0]
    INSN_BUFF_SET(insn, 20, 20, (imm >> 10) & 0x1);  // imm[10]
    INSN_BUFF_SET(insn, 12, 19, (imm >> 11) & 0xff); // imm[18:11]
    INSN_BUFF_SET(insn, 7, 11, rd);                  // rd
    INSN_BUFF_SET(insn, 0, 6, opcode);               // opcode

    insnCodeGen::generate(gen, insn);
}

void insnCodeGen::makeSTypeInsn(codeGen &gen,
                                    Dyninst::Register rs1,
                                    Dyninst::Register rs2,
                                    Dyninst::RegValue imm,
                                    unsigned funct3,
                                    unsigned opcode)
{
    instruction insn;

    INSN_BUFF_SET(insn, 25, 31, (imm >> 5) & 0x7f); // imm[11:5]
    INSN_BUFF_SET(insn, 20, 24, rs2);                // rs2
    INSN_BUFF_SET(insn, 15, 19, rs1);                // rs1
    INSN_BUFF_SET(insn, 12, 14, funct3);             // funct3
    INSN_BUFF_SET(insn, 7, 11, imm & 0x1f);          // imm[4:0]
    INSN_BUFF_SET(insn, 0, 6, opcode);               // opcode = 0100011 (store)

    insnCodeGen::generate(gen, insn);
}

// Must have methods
// These methods are called throughout the DyninstAPI

void insnCodeGen::generateIllegal(codeGen &gen)
{ 
    instruction insn;
    generate(gen, insn);
}

void insnCodeGen::generateTrap(codeGen &gen)
{
    if (gen.getUseRVC()) {
        instruction insn;
        INSN_BUFF_SET(insn, 0, 15, CBREAK_POINT_INSN);
        generate(gen, insn);
        return;
    }
    instruction insn;
    INSN_BUFF_SET(insn, 0, 31, BREAK_POINT_INSN);
    generate(gen, insn);
}

void insnCodeGen::generateNOOP(codeGen &gen,
                               unsigned size)
{
    int num = 0;
    if (gen.getUseRVC()) {
        assert ((size % RVC_INSN_SIZE) == 0);
        num = (size / RVC_INSN_SIZE);
    }
    else {
        assert ((size % RV_INSN_SIZE) == 0);
        num = (size / RV_INSN_SIZE);
    }
    for (int i = 0; i < num; i++) {
        generateNop(gen, gen.getUseRVC());
    }
}


void insnCodeGen::generateCall(codeGen &gen,
                               Dyninst::Address from,
                               Dyninst::Address to)
{
    generateBranch(gen, from, to);
}

void insnCodeGen::generateShortBranch(codeGen &gen,
                                      Dyninst::Address from,
                                      Dyninst::Address to,
                                      bool isCall)
{
    long disp = to - from;
    assert(disp >= JAL_IMM_MIN && disp < JAL_IMM_MAX);

    if (isCall) {
        generateJal(gen, GPR_RA, disp, gen.getUseRVC());
    }
    else {
        generateJ(gen, disp, gen.getUseRVC());
    }
}

void insnCodeGen::generateLongBranch(codeGen &gen,
                                     Dyninst::Address from,
                                     Dyninst::Address to,
                                     bool isCall)
{
    long disp = (to - from);

    if (isCall) {
        // use the link register ra (x1) as scratch since it will be overwritten at return

        // load pc to ra
        if (disp >= UTYPE_IMM_MIN && disp < UTYPE_IMM_MAX) {
            Dyninst::RegValue top = (disp >> UTYPE_IMM_SHIFT) & UTYPE_IMM_MASK;
            Dyninst::RegValue offset = disp & ITYPE_IMM_MASK;
            if (offset & 0x800) {
                top = (top + 1) & UTYPE_IMM_MASK;
            }
            generateAuipc(gen, GPR_RA, top, gen.getUseRVC());
            generateAddi(gen, GPR_RA, GPR_RA, offset, gen.getUseRVC());
        }
        else {
            Dyninst::Register scratch = gen.rs()->getScratchRegister(gen, true);
            if (scratch == Null_Register) {
                generateBranchViaTrap(gen, from, to);
                return;
            }
            generateAuipc(gen, GPR_RA, 0, gen.getUseRVC());
            generateCalcImm(gen, scratch, disp, true, gen.getUseRVC());
            generateAdd(gen, GPR_RA, GPR_RA, scratch, gen.getUseRVC());
        }
        // generate jalr
        generateJalr(gen, GPR_RA, GPR_RA, 0, gen.getUseRVC());
    }
    else {
        Dyninst::Register scratch = Null_Register;

        instPoint *point = gen.point();
        //AddressSpace *as = gen.addrSpace();
        if (point) {
            registerSpace *rs = registerSpace::actualRegSpace(point);
            gen.setRegisterSpace(rs);
            scratch = rs->getScratchRegister(gen, true);
        }
        /*
        if (as) {
            registerSpace *rs = registerSpace::getRegisterSpace(as);
            gen.setRegisterSpace(rs);
            scratch = rs->getScratchRegister(gen, true);
        }
        */

        if (scratch == Null_Register) {
            generateBranchViaTrap(gen, from, to);
            return;
        }

        if (disp >= UTYPE_IMM_MIN && disp < UTYPE_IMM_MAX) {
            Dyninst::RegValue top = (disp >> UTYPE_IMM_SHIFT) & UTYPE_IMM_MASK;
            Dyninst::RegValue offset = disp & ITYPE_IMM_MASK;
            if (offset & 0x800) {
                top = (top + 1) & UTYPE_IMM_MASK;
            }
            generateAuipc(gen, scratch, top, gen.getUseRVC());
            generateAddi(gen, scratch, scratch, offset, gen.getUseRVC());
        }
        else {
            std::vector<Register> exclude;
            exclude.push_back(scratch);

            Dyninst::Register scratch2 = gen.rs()->getScratchRegister(gen, exclude, true);
            if (scratch2 == Null_Register) {
                generateBranchViaTrap(gen, from, to);
                return;
            }
            generateAuipc(gen, GPR_RA, 0, gen.getUseRVC());
            generateCalcImm(gen, scratch2, disp, true, gen.getUseRVC());
            generateAdd(gen, scratch, scratch, scratch2, gen.getUseRVC());
        }

        generateJr(gen, scratch, 0, gen.getUseRVC());
    }
}

void insnCodeGen::generateSpringBoardBranch(codeGen &gen,
                                            Dyninst::Address from,
                                            Dyninst::Address to)
{
    std::cout << "From: " << std::hex << from << std::endl;
    std::cout << "To: " << std::hex << to << std::endl;
    std::cout << "Diff: " << std::hex << to - from << std::endl;
    long disp = to - from;
    if (disp >= UTYPE_IMM_MIN && disp < UTYPE_IMM_MAX) {
        Dyninst::RegValue top = (disp >> UTYPE_IMM_SHIFT) & UTYPE_IMM_MASK;
        Dyninst::RegValue offset = disp & ITYPE_IMM_MASK;
        std::cout << "Top: " << std::hex << top << std::endl;
        std::cout << "Offset: " << std::hex << offset << std::endl;
        std::cout << std::endl;
        if (offset & 0x800) {
            top = (top + 1) & UTYPE_IMM_MASK;
        }
        generateAuipc(gen, GPR_T0, top, gen.getUseRVC());
        generateAddi(gen, GPR_T0, GPR_T0, offset, gen.getUseRVC());
    }
    else {
        generateAuipc(gen, GPR_T0, 0, gen.getUseRVC());
        generateCalcImm(gen, GPR_T1, disp, true, gen.getUseRVC());
        generateAdd(gen, GPR_T0, GPR_T0, GPR_T1, gen.getUseRVC());
    }

    generateJr(gen, GPR_T0, 0, gen.getUseRVC());
}

void insnCodeGen::generateBranch(codeGen &gen,
                                 long disp,
                                 bool link)
{
    if (link) {
        generateJal(gen, GPR_RA, disp, gen.getUseRVC());
    }
    else {
        generateJ(gen, disp, gen.getUseRVC());
    }
}

void insnCodeGen::generateBranch(codeGen &gen,
                                 Dyninst::Address from,
                                 Dyninst::Address to,
                                 bool link)
{
    long disp = (to - from);
    // If disp is within the range of 21-bits signed integer, we use jal
    if (disp >= JAL_IMM_MIN && disp < JAL_IMM_MAX) {
        generateShortBranch(gen, from, to, link);
    }
    // Otherwise, we generate long branch
    else {
        generateLongBranch(gen, from, to, link);
    }
}

void insnCodeGen::generateBranchViaTrap(codeGen &gen,
                                        Dyninst::Address from,
                                        Dyninst::Address to)
{
    if (gen.addrSpace()) {
        gen.addrSpace()->trapMapping.addTrapMapping(from, to, true);
        insnCodeGen::generateTrap(gen);
    }
    else {
        assert(0);
    }

}

void insnCodeGen::generateCondBranch(codeGen &gen,
                                     int bCondOp,
                                     Dyninst::Register rs1,
                                     Dyninst::Register rs2,
                                     Dyninst::Address from,
                                     Dyninst::Address to)
{
    long disp = (to - from);
    if (disp < BTYPE_IMM_MIN || disp >= BTYPE_IMM_MAX) {
        Dyninst::Register scratch = Null_Register;
        instPoint *point = gen.point();
        //AddressSpace *as = gen.addrSpace();
        if (point) {
            registerSpace *rs = registerSpace::actualRegSpace(point);
            gen.setRegisterSpace(rs);
            scratch = rs->getScratchRegister(gen, true);
        }

        /*
        if (as) {
            registerSpace *rs = registerSpace::getRegisterSpace(as);
            gen.setRegisterSpace(rs);
            scratch = rs->getScratchRegister(gen, true);
        }
        */

        // If no scratch register is available, use generate branch via trap
        if (scratch == Null_Register) {
            generateCmpBranch(gen, bCondOp, rs1, rs2, RV_INSN_SIZE, false);
            generateBranchViaTrap(gen, from, to);
            return;
        }
        if (disp >= UTYPE_IMM_MIN && disp < UTYPE_IMM_MAX) {
            Dyninst::RegValue top = (disp >> UTYPE_IMM_SHIFT) & UTYPE_IMM_MASK;
            Dyninst::RegValue offset = disp & UTYPE_IMM_MASK;
            if (offset & 0x800) {
                top = (top + 1) & UTYPE_IMM_MASK;
            }
            generateCmpBranch(gen, bCondOp, rs1, rs2, 3 * RV_INSN_SIZE, false);
            generateAuipc(gen, scratch, top, false);
            generateAddi(gen, scratch, GPR_RA, offset, false);
            generateJr(gen, scratch, 0, false);
            return;
        }
        // The li instruction will be expanded into 8 4-bytes instructions without any optimization
        // So we should generate
        //    bxx rs1, rs2, (1 auipc, 8 (for li), 1 for add, 1 (for jalr)) * 4 = 44
        //    auipc scratch1 
        //    li scratch2, disp
        //    add scratch1, scratch2
        //    jalr x0, scratch, 0
        generateCmpBranch(gen, bCondOp, rs1, rs2, 11 * RV_INSN_SIZE, false);

        std::vector<Register> exclude;
        exclude.push_back(scratch);
        Dyninst::Register scratch2 = gen.rs()->getScratchRegister(gen, true);
        if (scratch2 == Null_Register) {
            fprintf(stderr, " %s[%d] No scratch register available to generate add instruction!", FILE__, __LINE__);
            assert(0);
        }
        generateAuipc(gen, scratch, 0, false);
        generateCalcImm(gen, scratch2, disp, false, false);
        generateAdd(gen, scratch, scratch, scratch2, false);

        generateJr(gen, scratch, 0, false);
    }
    else {
        generateCmpBranch(gen, bCondOp, rs1, rs2, disp, gen.getUseRVC());
    }
}

Dyninst::Register insnCodeGen::moveValueToReg(codeGen &gen,
                                              long int val,
                                              std::vector<Dyninst::Register> *exclude)
{
    Dyninst::Register scratch;
    if (exclude) {
        scratch = gen.rs()->getScratchRegister(gen, *exclude, true);
    }
    else {
        scratch = gen.rs()->getScratchRegister(gen, true);
    }

    if (scratch == Null_Register) {
        fprintf(stderr, " %s[%d] No scratch register available to generate add instruction!", FILE__, __LINE__);
        assert(0);
    }

    //generateLi(gen, scratch, static_cast<Dyninst::RegValue>(val), true, gen.getUseRVC());
    bool useRVC = gen.getUseRVC();
    if (useRVC) {
        // c.li
        if (val >= CLI_IMM_MIN && val < CLI_IMM_MAX) {
            generateCLi(gen, scratch, val & CLI_IMM_MASK);
            return true;
        }

        // addi
        if (val >= ITYPE_IMM_MIN && val < ITYPE_IMM_MAX) {
            generateAddi(gen, scratch, 0, val & ITYPE_IMM_MASK, useRVC);
            return true;
        }

        // TODO: move this out of if
        // If val is larger than 12 bits but less than 32 bits,
        // val must be loaded in two steps using lui and addi
        if (val >= INT_MIN && val <= INT_MAX) {
            Dyninst::RegValue lui_imm = (val >> UTYPE_IMM_SHIFT) & UTYPE_IMM_MASK;
            Dyninst::RegValue addi_imm = val & ITYPE_IMM_MASK;
            // If the most significant bit of addi_imm is 1 (addi_imm is negative), we should add 1 to lui_imm
            if (addi_imm & 0x800) {
                lui_imm = (lui_imm + 1) & UTYPE_IMM_MASK;
            }
            // 12-bit sign extend
            if (lui_imm & 0x80000) {
                lui_imm |= 0xfffffffffff00000;
            }
            generateLui(gen, scratch, lui_imm, useRVC);
            generateAddi(gen, scratch, scratch, addi_imm, useRVC);
            return true;
        }
    }
    generateCalcImm(gen, scratch, val, true, useRVC);

    return scratch;
}

bool insnCodeGen::loadImmIntoReg(codeGen &gen,
                                 Dyninst::Register rd,
                                 Dyninst::RegValue value,
                                 bool useRVC)
{
    return generateLi(gen, rd, value, true, useRVC);
}

bool insnCodeGen::modifyCall(Dyninst::Address target,
                             NS_riscv64::instruction &insn,
                             codeGen &gen)
{
    if (insn.isUncondBranch()) {
        return modifyJump(target, insn, gen);
    }
    else {
        return modifyJcc(target, insn, gen);
    }
}

bool insnCodeGen::modifyJump(Dyninst::Address target,
                             NS_riscv64::instruction &insn,
                             codeGen &gen)
{
    generateBranch(gen, gen.currAddr(), target, insn.isCall());
    return true;
}

bool insnCodeGen::modifyJcc(Dyninst::Address target,
                            NS_riscv64::instruction &insn,
                            codeGen &gen)
{
    Dyninst::Register rs1 = insn.getCondBranchReg1();
    Dyninst::Register rs2 = insn.getCondBranchReg2();
    int bCondOp = insn.getCondBranchOp();

    generateCondBranch(gen, bCondOp, rs1, rs2, gen.currAddr(), target);

    return true;
}

// Other useful code generation methods

// Nop generation
bool insnCodeGen::generateNop(codeGen &gen,
                              bool useRVC)
{
    if (useRVC) {
        generateCNop(gen);
        return true;
    }
    generateMove(gen, GPR_ZERO, GPR_ZERO, false);
    return false;
}

// Conditional Branch Generation

void insnCodeGen::generateCmpBranch(codeGen &gen,
                                    int bCond,
                                    Dyninst::Register rs1,
                                    Dyninst::Register rs2,
                                    Dyninst::RegValue imm,
                                    bool useRVC)
{
    Dyninst::RegValue shiftedImm = (imm >> BTYPE_IMM_SHIFT) & BTYPE_IMM_MASK;
    switch (bCond) {
        case B_COND_EQ: {
            generateBeq(gen, rs1, rs2, shiftedImm, useRVC);
            break;
        }
        case B_COND_NE: {
            generateBne(gen, rs1, rs2, shiftedImm, useRVC);
            break;
        }
        case B_COND_LT: {
            generateBlt(gen, rs1, rs2, shiftedImm, useRVC);
            break;
        }
        case B_COND_GE: {
            generateBge(gen, rs1, rs2, shiftedImm, useRVC);
            break;
        }
        case B_COND_LTU: {
            generateBltu(gen, rs1, rs2, shiftedImm, useRVC);
            break;
        }
        case B_COND_GEU: {
            generateBgeu(gen, rs1, rs2, shiftedImm, useRVC);
            break;
        }
        default: {
            // Not gonna happen
            assert(0);
        }
    }
}

bool insnCodeGen::generateMemLoad(codeGen &gen,
                                  Dyninst::Register rd,
                                  Dyninst::Register rs,
                                  Dyninst::RegValue offset,
                                  Dyninst::RegValue size,
                                  bool isUnsigned,
                                  bool useRVC) {
    return generateLd(gen, rd, rs, offset, size, isUnsigned, useRVC);
}

bool insnCodeGen::generateMemStore(codeGen &gen,
                                   Dyninst::Register rs1,
                                   Dyninst::Register rs2,
                                   Dyninst::RegValue offset,
                                   Dyninst::RegValue size,
                                   bool useRVC) {
    return generateSt(gen, rs1, rs2, offset, size, useRVC);
}

// Generate memory access through Load or Store
bool insnCodeGen::generateLd(codeGen &gen,
                             Dyninst::Register rd,
                             Dyninst::Register rs,
                             Dyninst::RegValue offset,
                             Dyninst::RegValue size,
                             bool isUnsigned,
                             bool useRVC)
{
    assert(size == 1 || size == 2 || size == 4 || size == 8);

    if (useRVC) {
        if (!isUnsigned && size == 4) {
            // c.lw
            if (rd >= GPR_X8 && rd < GPR_X16 && rs >= GPR_X8 && rs < GPR_X16 &&
                    offset % size == 0 && offset >= CLW_IMM_MIN && offset < CLW_IMM_MAX) {
                generateCLw(gen, rd - GPR_X8, rs - GPR_X8, (offset >> CLW_SHIFT) & CLW_MASK);
                return true;
            }
            // c.lwsp
            if (rs == GPR_SP && offset % size == 0 &&
                    offset >= CLWSP_IMM_MIN && offset < CLWSP_IMM_MAX) {
                generateCLwsp(gen, rd, (offset >> CLWSP_SHIFT) & CLWSP_MASK);
                return true;
            }
        }
        if (!isUnsigned && size == 8) {
            // c.ld
            if (rd >= GPR_X8 && rd < GPR_X16 && rs >= GPR_X8 && rs < GPR_X16 &&
                    offset % size == 0 && offset >= CLD_IMM_MIN && offset < CLD_IMM_MAX) {
                generateCLd(gen, rd - GPR_X8, rs - GPR_X8, (offset >> CLD_SHIFT) & CLD_MASK);
                return true;
            }
            // c.ldsp
            if (rs == GPR_SP && offset % size == 0 &&
                    offset >= CLDSP_IMM_MIN && offset < CLDSP_IMM_MAX) {
                generateCLdsp(gen, rd, (offset >> CLDSP_SHIFT) & CLDSP_MASK);
                return true;
            }
        }
    }

    // There is no "ldu" instruction, but treat "ldu" as ld
    // Uncomment the following line to not treat "ldu" as ld
    //assert(!(size == 8 && isUnsigned))

    Dyninst::RegValue funct3{};
    if (!isUnsigned) {
        switch (size) {
            case 1: funct3 = LBFunct3; break; // lb = 000
            case 2: funct3 = LHFunct3; break; // lh = 001
            case 4: funct3 = LWFunct3; break; // lw = 010
            case 8: funct3 = LDFunct3; break; // ld = 011
            default: break;                   // not gonna happen
        }
    }
    else {
        switch (size) {
            case 1: funct3 = LBUFunct3; break; // lbu = 100
            case 2: funct3 = LHUFunct3; break; // lhu = 101
            case 4: funct3 = LWUFunct3; break; // lwu = 110
            case 8: funct3 = LDFunct3; break;  // Treat "ldu" as "ld"
            default: break;                    // not gonna happen
        }
    }
    makeITypeInsn(gen, rd, rs, offset & ITYPE_IMM_MASK, funct3, LOADOp);
    return false;
}

bool insnCodeGen::generateSt(codeGen &gen,
                             Dyninst::Register rs1,
                             Dyninst::Register rs2,
                             Dyninst::RegValue offset,
                             Dyninst::RegValue size,
                             bool useRVC)
{
    assert(size == 1 || size == 2 || size == 4 || size == 8);

    if (useRVC) {
        if (size == 4) {
            // c.sw
            if (rs1 >= GPR_X8 && rs1 < GPR_X16 && rs2 >= GPR_X8 && rs2 < GPR_X16
                    && offset % size == 0 && offset >= CSW_IMM_MIN && offset < CSW_IMM_MAX) {
                generateCSw(gen, rs1 - GPR_X8, rs2 - GPR_X8, (offset >> CSW_SHIFT) & CSW_MASK);
                return true;
            }
            // c.swsp
            if (rs1 == GPR_SP && offset % size == 0 &&
                    offset >= CSWSP_IMM_MIN && offset < CSWSP_IMM_MAX) {
                generateCSwsp(gen, rs2, (offset >> CSWSP_SHIFT) & CSWSP_MASK);
                return true;
            }
        }
        if (size == 8) {
            // c.sd
            if (rs1 >= GPR_X8 && rs1 < GPR_X16 && rs2 >= GPR_X8 && rs2 < GPR_X16 &&
                    offset % size == 0 && offset >= CSD_IMM_MIN && offset < CSD_IMM_MAX) {
                generateCSd(gen, rs1 - GPR_X8, rs2 - GPR_X8, (offset >> CSD_SHIFT) & CSD_MASK);
                return true;
            }
            // c.sdsp
            if (rs1 == GPR_SP && offset % size == 0 &&
                    offset >= CSDSP_IMM_MIN && offset < CSDSP_IMM_MAX) {
                generateCSdsp(gen, rs2, (offset >> CSDSP_SHIFT) & CSDSP_MASK);
                return true;
            }
        }
    }

    Dyninst::RegValue funct3{};
    switch (size) {
        case 1: funct3 = SBFunct3; break; // sb = 000
        case 2: funct3 = SHFunct3; break; // sh = 001
        case 4: funct3 = SWFunct3; break; // sw = 010
        case 8: funct3 = SDFunct3; break; // sd = 011
        default: break;                   // not gonna happen
    }

    makeSTypeInsn(gen, rs1, rs2, offset & STYPE_IMM_MASK, funct3, STOREOp);
    return false;
}

bool insnCodeGen::generateMemLoadFp(codeGen & gen,
                                    Dyninst::Register rd,
                                    Dyninst::Register rs,
                                    Dyninst::RegValue offset,
                                    Dyninst::RegValue size,
                                    bool useRVC)
{
    if (useRVC) {
        if (size == 4) {
            // c.lw
            if (rd >= GPR_X8 && rd < GPR_X16 && rs >= GPR_X8 && rs < GPR_X16 &&
                    offset % size == 0 && offset >= CFLW_IMM_MIN && offset < CFLW_IMM_MAX) {
                generateCFlw(gen, rd - GPR_X8, rs - GPR_X8, (offset >> CFLW_SHIFT) & CFLW_MASK);
                return true;
            }
            // c.lwsp
            if (rs == GPR_SP && offset % size == 0 &&
                    offset >= CFLWSP_IMM_MIN && offset < CFLWSP_IMM_MAX) {
                generateCFlwsp(gen, rd, (offset >> CFLWSP_SHIFT) & CFLWSP_MASK);
                return true;
            }
        }
        if (size == 8) {
            // c.ld
            if (rd >= GPR_X8 && rd < GPR_X16 && rs >= GPR_X8 && rs < GPR_X16 &&
                    offset % size == 0 && offset >= CFLD_IMM_MIN && offset < CFLD_IMM_MAX) {
                generateCFld(gen, rd - GPR_X8, rs - GPR_X8, (offset >> CFLD_SHIFT) & CFLD_MASK);
                return true;
            }
            // c.ldsp
            if (rs == GPR_SP && offset % size == 0 &&
                    offset >= CFLDSP_IMM_MIN && offset < CFLDSP_IMM_MAX) {
                generateCFldsp(gen, rd, (offset >> CFLDSP_SHIFT) & CFLDSP_MASK);
                return true;
            }
        }
    }
    makeITypeInsn(gen, rd, rs, offset & ITYPE_IMM_MASK, FLDFunct3, FLDOp);
    return false;
}

bool insnCodeGen::generateMemStoreFp(codeGen &gen,
                                     Dyninst::Register rs1,
                                     Dyninst::Register rs2,
                                     Dyninst::RegValue offset,
                                     Dyninst::RegValue size,
                                     bool useRVC)
{
    if (useRVC) {
        if (size == 4) {
            // c.sw
            if (rs1 >= GPR_X8 && rs1 < GPR_X16 && rs2 >= GPR_X8 && rs2 < GPR_X16
                    && offset % size == 0 && offset >= CFSW_IMM_MIN && offset < CFSW_IMM_MAX) {
                generateCFsw(gen, rs1 - GPR_X8, rs2 - GPR_X8, (offset >> CFSW_SHIFT) & CFSW_MASK);
                return true;
            }
            // c.swsp
            if (rs1 == GPR_SP && offset % size == 0 &&
                    offset >= CFSWSP_IMM_MIN && offset < CFSWSP_IMM_MAX) {
                generateCFswsp(gen, rs2, (offset >> CFSWSP_SHIFT) & CFSWSP_MASK);
                return true;
            }
        }
        if (size == 8) {
            // c.sd
            if (rs1 >= GPR_X8 && rs1 < GPR_X16 && rs2 >= GPR_X8 && rs2 < GPR_X16 &&
                    offset % size == 0 && offset >= CFSD_IMM_MIN && offset < CFSD_IMM_MAX) {
                generateCFsd(gen, rs1 - GPR_X8, rs2 - GPR_X8, (offset >> CFSD_SHIFT) & CFSD_MASK);
                return true;
            }
            // c.sdsp
            if (rs1 == GPR_SP && offset % size == 0 &&
                    offset >= CFSDSP_IMM_MIN && offset < CFSDSP_IMM_MAX) {
                generateCFsdsp(gen, rs2, (offset >> CFSDSP_SHIFT) & CFSDSP_MASK);
                return true;
            }
        }
    }
    makeSTypeInsn(gen, rs1, rs2, offset & STYPE_IMM_MASK, FSDFunct3, FSDOp);
    return false;
}

//
// generate an instruction that does nothing and has to side affect except to
//   advance the program counter.
//

bool insnCodeGen::saveRegister(codeGen &gen,
                               Dyninst::Register r,
                               int sp_offset,
                               bool useRVC)
{
    return generateMemStore(gen, REG_SP, r, sp_offset, gen.width(), useRVC);
}


bool insnCodeGen::restoreRegister(codeGen &gen,
                                  Dyninst::Register r,
                                  int sp_offset,
                                  bool useRVC)
{
    return generateMemLoad(gen, r, REG_SP, sp_offset, gen.width(), true, useRVC);
}

bool insnCodeGen::modifyData(Dyninst::Address target,
                             NS_riscv64::instruction &insn,
                             codeGen &gen)
{
    assert(insn.isAuipc());
    
    long auipcOff = insn.getAuipcOffset();
    Register rd = insn.getAuipcReg();

    // The displacement is target - currAddr
    // But we also need to minus the instruction length of auipc
    // As well as adding the offset of auipc back
    long disp = (target - gen.currAddr() - RV_INSN_SIZE) + auipcOff;
    generateLoadImm(gen, rd, disp, true, true, gen.getUseRVC());
    return true;
}

bool insnCodeGen::generateAddImm(codeGen &gen,
                                 Dyninst::Register rd,
                                 Dyninst::Register rs,
                                 Dyninst::RegValue sImm,
                                 bool useRVC) {
    if (sImm >= ITYPE_IMM_MIN && sImm < ITYPE_IMM_MAX) {
        return generateAddi(gen, rd, rs, sImm & ITYPE_IMM_MASK, useRVC);
    }
    std::vector<Register> exclude;
    exclude.push_back(rd);
    exclude.push_back(rs);
    Register scratch = insnCodeGen::moveValueToReg(gen, sImm, &exclude);
    return insnCodeGen::generateAdd(gen, rd, rs, scratch, useRVC);
}

bool insnCodeGen::generateLoadImm(codeGen &gen,
                                  Dyninst::Register rd,
                                  Dyninst::RegValue sImm,
                                  bool isRel,
                                  bool optimize,
                                  bool useRVC)
{
    if (useRVC) {
        // TODO: move this out of if
        if (isRel) {
            if (sImm >= UTYPE_IMM_MIN && sImm < UTYPE_IMM_MAX) {
                Dyninst::RegValue top = (sImm >> UTYPE_IMM_SHIFT) & UTYPE_IMM_MASK;
                Dyninst::RegValue offset = sImm & ITYPE_IMM_MASK;
                if (offset & 0x800) {
                    top = (top + 1) & UTYPE_IMM_MASK;
                }
                generateAuipc(gen, rd, top, true);
                generateAddi(gen, rd, rd, offset, true);
                return true;
            }
        }
        else {
            // c.li
            if (sImm >= CLI_IMM_MIN && sImm < CLI_IMM_MAX) {
                generateCLi(gen, rd, sImm & CLI_IMM_MASK);
                return true;
            }

            // addi
            if (sImm >= ITYPE_IMM_MIN && sImm < ITYPE_IMM_MAX) {
                generateAddi(gen, rd, 0, sImm & ITYPE_IMM_MASK, useRVC);
                return true;
            }

            // TODO: move this out of if
            // If sImm is larger than 12 bits but less than 32 bits,
            // sImm must be loaded in two steps using lui and addi
            if (sImm >= INT_MIN && sImm <= INT_MAX) {
                Dyninst::RegValue lui_imm = (sImm >> UTYPE_IMM_SHIFT) & UTYPE_IMM_MASK;
                Dyninst::RegValue addi_imm = sImm & ITYPE_IMM_MASK;
                // If the most significant bit of addi_imm is 1 (addi_imm is negative), we should add 1 to lui_imm
                if (addi_imm & 0x800) {
                    lui_imm = (lui_imm + 1) & UTYPE_IMM_MASK;
                }
                // 12-bit sign extend
                if (lui_imm & 0x80000) {
                    lui_imm |= 0xfffffffffff00000;
                }
                generateLui(gen, rd, lui_imm, useRVC);
                generateAddi(gen, rd, rd, addi_imm, useRVC);
                return true;
            }
        }
    }

    if (isRel) {
        std::vector<Register> exclude;
        exclude.push_back(rd);

        Dyninst::Register scratch = gen.rs()->getScratchRegister(gen, exclude, true);
        if (scratch == Null_Register) {
            // Pick an arbitrary register, spill it, load immediate to the register, and restore it
            generateAuipc(gen, rd, 0, useRVC);
            Dyninst::Register tmp = (rd == GPR_T0) ? GPR_T1 : GPR_T0;
            saveRegister(gen, tmp, -8, true);
            generateCalcImm(gen, tmp, sImm, optimize, useRVC);
            generateAdd(gen, rd, rd, tmp, useRVC);
            restoreRegister(gen, tmp, -8, true);
        }
        else {
            generateAuipc(gen, rd, 0, useRVC);
            generateCalcImm(gen, scratch, sImm, optimize, useRVC);
            generateAdd(gen, rd, rd, scratch, useRVC);
        }
    }
    else {
        generateCalcImm(gen, rd, sImm, optimize, useRVC);
    }
    return false;
}

bool insnCodeGen::generateCalcImm(codeGen &gen,
                                  Dyninst::Register rd,
                                  Dyninst::RegValue sImm,
                                  bool optimize,
                                  bool useRVC)
{
    // If sImm is a 64 bit long, the sequence of instructions is more complicated
    // See the following functions for more information on how GCC generates immediate integers
    // https://gcc.gnu.org/git/?p=gcc.git;a=blob;f=gcc/config/riscv/riscv.cc;h=65e09842fde8b15b92a8399cea2493b5b239f93c;hb=HEAD#l828
    // But for the sake of simplicity, we will use a much simpler algorithm
    // Assume that we want to perform li t0, 0xdeadbeefcafebabe
    // We break the integer into the following
    //   lui  t0, 0xdeadc    / auipc t0, 0
    //   addi t0, t0, -0x110
    //   slli t0, t0, 12
    //   addi t0, t0, -0x250
    //   slli t0, t0, 12
    //   addi t0, t0, -0x186
    //   slli t0, t0, 8
    //   addi t0, t0, 0xef

    // Decompose the immediate into various chunks
    // For example, if sImm == 0xdeadbeefcafebabe:
    //  deadb     eef       caf       eba       be
    // ------- --------- --------- --------- ---------
    // lui_imm addi_imm0 addi_imm1 addi_imm2 addi_imm3

    // Upper 32 bits
    Dyninst::RegValue lui_imm = (sImm >> 44) & 0xfffff; // sImm[63:44]
    Dyninst::RegValue addi_imm0 = (sImm >> 32) & 0xfff; // sImm[43:32]
    // Lower 32 bits
    Dyninst::RegValue slli_imm1 = 11;
    Dyninst::RegValue addi_imm1 = (sImm >> 21) & 0x7ff; // sImm[31:21]
    Dyninst::RegValue slli_imm2 = 11;
    Dyninst::RegValue addi_imm2 = (sImm >> 10) & 0x7ff;  // sImm[20:10]
    Dyninst::RegValue slli_imm3 = 10;
    Dyninst::RegValue addi_imm3 = sImm & 0x3ff;         // sImm[9:0]

    // If the most significant bit of addi_imm is 1 (addi_imm is negative), we should add 1 to lui_imm
    if (addi_imm0 & 0x800) {
        lui_imm = (lui_imm + 1) & 0xfffff;
    }
    // 12-bit sign extend
    if (lui_imm & 0x80000) {
        lui_imm |= 0xfffffffffff00000;
    }

    // Optimization: if any of the addi sImmediates are zero, we can omit them
    // We should also adjust the number of bits to shift accordingly
    if (optimize) {
        if (addi_imm2 == 0) {
            slli_imm2 += slli_imm3;
            slli_imm3 = 0;
        }
        if (addi_imm1 == 0) {
            slli_imm1 += slli_imm2;
            slli_imm2 = 0;
        }
    }

    // lui must be generated
    generateLui(gen, rd, lui_imm, useRVC);

    // If any of the following sImmediates are zero, there's no point of generating it
    if (addi_imm0 != 0 || !optimize) {
        generateAddi(gen, rd, rd, addi_imm0, useRVC);
    }
    if (slli_imm1 != 0 || !optimize) {
        generateSlli(gen, rd, rd, slli_imm1, useRVC);
    }
    if (addi_imm1 != 0 || !optimize) {
        generateAddi(gen, rd, rd, addi_imm1, useRVC);
    }
    if (slli_imm2 != 0 || !optimize) {
        generateSlli(gen, rd, rd, slli_imm2, useRVC);
    }
    if (addi_imm2 != 0 || !optimize) {
        generateAddi(gen, rd, rd, addi_imm2, useRVC);
    }
    if (slli_imm3 != 0 || !optimize) {
        generateSlli(gen, rd, rd, slli_imm3, useRVC);
    }
    if (addi_imm3 != 0 || !optimize) {
        generateAddi(gen, rd, rd, addi_imm3, useRVC);
    }
    return false;
}

// RISC-V I-Type Instructions

bool insnCodeGen::generateAddi(codeGen &gen,
                               Dyninst::Register rd,
                               Dyninst::Register rs,
                               Dyninst::RegValue imm,
                               bool useRVC)
{
    // 12 bit sign extend
    int sImm = (imm & 0x800) ? (imm | 0xfffff000) : imm;

    if (useRVC) {
        // c.nop
        if (rd == 0 && rs == 0 && sImm == 0) {
            generateCNop(gen);
            return true;
        }

        // c.li
        if (sImm >= CLI_IMM_MIN && sImm < CLI_IMM_MAX && rs == 0) {
            generateCLi(gen, rd, imm & CLI_IMM_MASK);
            return true;
        }

        // c.mv
        if (sImm == 0) {
            generateCMv(gen, rd, rs);
            return true;
        }

        // c.addi4spn
        if (rs == GPR_SP && rd >= GPR_X8 && rd < GPR_X16 && sImm % 4 == 0 && 
                sImm >= CADDI4SPN_IMM_MIN && sImm < CADDI4SPN_IMM_MAX) {
            generateCAddi4spn(gen, rd, (imm >> CADDI4SPN_IMM_SHIFT) & CADDI4SPN_IMM_MASK);
            return true;
        }

        // c.addi16sp
        if (rd == GPR_SP && rs == GPR_SP && sImm % 16 == 0 &&
                sImm >= CADDI16SP_IMM_MIN && sImm < CADDI16SP_IMM_MAX) {
            generateCAddi16sp(gen, (imm >> CADDI16SP_IMM_SHIFT) & CADDI16SP_IMM_MASK);
            return true;
        }

        // c.addi
        if (rd == rs && sImm >= CADDI_IMM_MIN && sImm < CADDI_IMM_MAX) {
            generateCAddi(gen, rd, imm & CADDI_IMM_MASK);
            return true;
        }
    }
    // addi
    makeITypeInsn(gen, rd, rs, imm & ITYPE_IMM_MASK, ADDFunct3, IMMOp);
    return false;
}

bool insnCodeGen::generateSlli(codeGen &gen,
                               Dyninst::Register rd,
                               Dyninst::Register rs,
                               Dyninst::RegValue imm,
                               bool useRVC)
{
    if (useRVC) {
        // c.slli
        if (rd == rs && imm >= CSLLI_IMM_MIN && imm < CSLLI_IMM_MAX) {
            generateCSlli(gen, rd, imm & CSLLI_IMM_MASK);
            return false;
        }
    }
    makeITypeInsn(gen, rd, rs, imm & ITYPE_IMM_MASK, SLLFunct3, IMMOp);
    return true;
}

bool insnCodeGen::generateSrli(codeGen &gen,
                               Dyninst::Register rd,
                               Dyninst::Register rs,
                               Dyninst::RegValue imm,
                               bool useRVC) {
    if (useRVC) {
        // use c.srli
        if (rd == rs && rd >= GPR_X8 && rd < GPR_X16 &&
                imm >= CSRLI_IMM_MIN && imm < CSRLI_IMM_MAX) {
            generateCSrli(gen, rd - GPR_X8, imm);
            return false;
        }
    }
    makeITypeInsn(gen, rd, rs, imm & ITYPE_IMM_MASK, SRLFunct3, IMMOp);
    return true;
}

bool insnCodeGen::generateSrai(codeGen &gen,
                               Dyninst::Register rd,
                               Dyninst::Register rs,
                               Dyninst::RegValue imm,
                               bool useRVC)
{
    if (useRVC) {
        // c.srai
        if (rd == rs && rd >= GPR_X8 && rd < GPR_X16 &&
                imm >= CSRLI_IMM_MIN && imm < CSRLI_IMM_MAX) {
            generateCSrai(gen, rd - GPR_X8, imm);
            return false;
        }
    }
    // srai is essentially srli with bit 30 set to 1
    imm |= 0x400;

    makeITypeInsn(gen, rd, rs, imm & ITYPE_IMM_MASK, SRAFunct3, IMMOp);
    return true;
}
bool insnCodeGen::generateAndi(codeGen &gen,
                               Dyninst::Register rd,
                               Dyninst::Register rs,
                               Dyninst::RegValue imm,
                               bool useRVC)
{
    if (useRVC) {
        // c.andi
        if (rd == rs && rd >= GPR_X8 && rd < GPR_X16 &&
                imm >= CANDI_IMM_MIN && imm < CANDI_IMM_MAX) {
            generateCAndi(gen, rd - GPR_X8, imm);
            return false;
        }
    }
    makeITypeInsn(gen, rd, rs, imm & ITYPE_IMM_MASK, ANDFunct3, IMMOp);
    return true;
}

bool insnCodeGen::generateOri(codeGen &gen,
                              Dyninst::Register rd,
                              Dyninst::Register rs,
                              Dyninst::RegValue imm,
                              bool /*useRVC*/)
{
    // No available RVC optimization
    makeITypeInsn(gen, rd, rs, imm & ITYPE_IMM_MASK, ORFunct3, IMMOp);
    return false;
}

bool insnCodeGen::generateXori(codeGen &gen,
                               Dyninst::Register rd,
                               Dyninst::Register rs,
                               Dyninst::RegValue imm,
                               bool /*useRVC*/)
{
    // No available RVC optimization
    makeITypeInsn(gen, rd, rs, imm & ITYPE_IMM_MASK, XORFunct3, IMMOp);
    return false;
}

bool insnCodeGen::generateLui(codeGen &gen,
                              Dyninst::Register rd,
                              Dyninst::RegValue offset,
                              bool useRVC)
{
    Dyninst::RegValue imm = offset >> 12;
    if (useRVC) {
        // c.lui
        if ((imm >= CLUI_IMM_MIN1 && imm < CLUI_IMM_MAX1) ||
                (imm >= CLUI_IMM_MIN2 && imm < CLUI_IMM_MAX2)) {
            generateCLui(gen, rd, imm);
            return true;
        }
    }
    makeUTypeInsn(gen, rd, imm & UTYPE_IMM_MASK, LUIOp);
    return false;
}

bool insnCodeGen::generateAuipc(codeGen &gen,
                                Dyninst::Register rd,
                                Dyninst::RegValue offset,
                                bool /*useRVC*/)
{
    // No available RVC optimization
    makeUTypeInsn(gen, rd, offset & UTYPE_IMM_MASK, AUIPCOp);
    return false;
}

bool insnCodeGen::generateAdd(codeGen &gen,
                              Dyninst::Register rd,
                              Dyninst::Register rs1,
                              Dyninst::Register rs2,
                              bool useRVC)
{
    if (useRVC) {
        // c.add
        if (rs1 == rs2) {
            generateCAdd(gen, rd, rs1);
            return true;
        }
    }
    makeRTypeInsn(gen, rd, rs1, rs2, ADDFunct7, ADDFunct3, REGOp);
    return false;
}

bool insnCodeGen::generateSub(codeGen &gen,
                              Dyninst::Register rd,
                              Dyninst::Register rs1,
                              Dyninst::Register rs2,
                              bool /*useRVC*/)
{
    // No available RVC optimization
    makeRTypeInsn(gen, rd, rs1, rs2, SUBFunct7, SUBFunct3, REGOp);
    return false;
}

bool insnCodeGen::generateSll(codeGen &gen,
                              Dyninst::Register rd,
                              Dyninst::Register rs1,
                              Dyninst::Register rs2,
                              bool /*useRVC*/)
{
    // No available RVC optimization
    makeRTypeInsn(gen, rd, rs1, rs2, SLLFunct7, SLLFunct3, REGOp);
    return false;
}

bool insnCodeGen::generateSrl(codeGen &gen,
                              Dyninst::Register rd,
                              Dyninst::Register rs1,
                              Dyninst::Register rs2,
                              bool /*useRVC*/)
{
    // No available RVC optimization
    makeRTypeInsn(gen, rd, rs1, rs2, SRLFunct7, SRLFunct3, REGOp);
    return false;
}

bool insnCodeGen::generateSra(codeGen &gen,
                              Dyninst::Register rd,
                              Dyninst::Register rs1,
                              Dyninst::Register rs2,
                              bool /*useRVC*/)
{
    // No available RVC optimization
    makeRTypeInsn(gen, rd, rs1, rs2, SRAFunct7, SRAFunct3, REGOp);
    return false;
}

bool insnCodeGen::generateAnd(codeGen &gen,
                              Dyninst::Register rd,
                              Dyninst::Register rs1,
                              Dyninst::Register rs2,
                              bool useRVC)
{
    if (useRVC) {
        // c.and
        if (rs1 == rs2 && rs1 >= GPR_X8 && rs1 < GPR_X16) {
            generateCAnd(gen, rd, rs1 - GPR_X8);
            return true;
        }
    }
    makeRTypeInsn(gen, rd, rs1, rs2, ANDFunct7, ANDFunct3, REGOp);
    return false;
}

bool insnCodeGen::generateOr(codeGen &gen,
                             Dyninst::Register rd,
                             Dyninst::Register rs1,
                             Dyninst::Register rs2,
                             bool useRVC)
{
    if (useRVC) {
        // c.or
        if (rs1 == rs2 && rs1 >= GPR_X8 && rs1 < GPR_X16) {
            generateCOr(gen, rd - GPR_X8, rs1);
            return true;
        }
    }
    makeRTypeInsn(gen, rd, rs1, rs2, ORFunct7, ORFunct3, REGOp);
    return false;
}
bool insnCodeGen::generateXor(codeGen &gen,
                              Dyninst::Register rd,
                              Dyninst::Register rs1,
                              Dyninst::Register rs2,
                              bool useRVC)
{
    if (useRVC) {
        // c.xor
        if (rs1 == rs2 && rs1 >= GPR_X8 && rs1 < GPR_X16) {
            generateCXor(gen, rd - GPR_X8, rs1);
            return true;
        }
    }
    makeRTypeInsn(gen, rd, rs1, rs2, XORFunct7, XORFunct3, REGOp);
    return false;
}

bool insnCodeGen::generateMul(codeGen &gen,
                              Dyninst::Register rd,
                              Dyninst::Register rs1,
                              Dyninst::Register rs2,
                              bool /*useRVC*/)
{
    // No available RVC optimization
    makeRTypeInsn(gen, rd, rs1, rs2, MULFunct7, MULFunct3, REGOp);
    return false;
}

bool insnCodeGen::generateDiv(codeGen &gen,
                              Dyninst::Register rd,
                              Dyninst::Register rs1,
                              Dyninst::Register rs2,
                              bool /*useRVC*/)
{
    // No available RVC optimization
    makeRTypeInsn(gen, rd, rs1, rs2, DIVFunct7, DIVFunct3, REGOp);
    return false;
}


bool insnCodeGen::generateLi(codeGen &gen,
                             Dyninst::Register rd,
                             Dyninst::RegValue imm,
                             bool optimize,
                             bool useRVC)
{
    generateLoadImm(gen, rd, imm, false, optimize, useRVC);
    return false;
}


bool insnCodeGen::generateMove(codeGen &gen,
                               Dyninst::Register rd,
                               Dyninst::Register rs,
                               bool useRVC)
{
    if (useRVC) {
        generateCMv(gen, rd, rs);
        return true;
    }
    generateAdd(gen, rd, rs, 0, useRVC);
    return false;
}

bool insnCodeGen::generateBeq(codeGen &gen,
                              Dyninst::Register rs1,
                              Dyninst::Register rs2,
                              Dyninst::RegValue imm,
                              bool useRVC)
{
    // If both rs1 and rs2 are x0, it is equivalent to unconditional branch
    if (rs1 == GPR_ZERO && rs2 == GPR_ZERO) {
        generateBranch(gen, imm, false);
    }
    if (useRVC) {
        if ((rs1 == GPR_ZERO && rs2 >= GPR_X8 && rs2 < GPR_X16) &&
                imm >= CBEQZ_IMM_MIN && imm < CBEQZ_IMM_MAX) {
            int shiftedImm = (imm >> CBEQZ_IMM_SHIFT) & CBEQZ_IMM_MASK;
            generateCBeqz(gen, rs1 - GPR_X8, shiftedImm);
            return true;
        }
        if ((rs2 == GPR_ZERO && rs1 >= GPR_X8 && rs1 < GPR_X16) &&
                imm >= CBEQZ_IMM_MIN && imm < CBEQZ_IMM_MAX) {
            int shiftedImm = (imm >> CBEQZ_IMM_SHIFT) & CBEQZ_IMM_MASK;
            generateCBeqz(gen, rs2 - GPR_X8, shiftedImm);
            return true;
        }
    }
    makeBTypeInsn(gen, rs1, rs2, imm & BTYPE_IMM_MASK, BEQFunct3, BRANCHOp);
    return false;
}

bool insnCodeGen::generateBne(codeGen &gen,
                              Dyninst::Register rs1,
                              Dyninst::Register rs2,
                              Dyninst::RegValue imm,
                              bool useRVC)
{
    // If both rs1 and rs2 are x0, it is equivalent to unconditional branch
    if (rs1 == GPR_ZERO && rs2 == GPR_ZERO) {
        return generateJal(gen, GPR_ZERO, imm, gen.getUseRVC());
    }
    if (useRVC) {
        if ((rs1 == GPR_ZERO && rs2 >= GPR_X8 && rs2 < GPR_X16) &&
                imm >= CBNEZ_IMM_MIN && imm < CBNEZ_IMM_MAX) {
            int shiftedImm = (imm >> CBNEZ_IMM_SHIFT) & CBNEZ_IMM_MASK;
            generateCBnez(gen, rs1 - GPR_X8, shiftedImm);
            return true;
        }
        if ((rs2 == GPR_ZERO && rs1 >= GPR_X8 && rs1 < GPR_X16) &&
                imm >= CBNEZ_IMM_MIN && imm < CBNEZ_IMM_MAX) {
            int shiftedImm = (imm >> CBNEZ_IMM_SHIFT) & CBNEZ_IMM_MASK;
            generateCBnez(gen, rs2 - GPR_X8, shiftedImm);
            return true;
        }
    }
    makeBTypeInsn(gen, rs1, rs2, imm & BTYPE_IMM_MASK, BNEFunct3, BRANCHOp);
    return false;
}

bool insnCodeGen::generateBlt(codeGen &gen,
                                         Dyninst::Register rs1,
                                         Dyninst::Register rs2,
                                         Dyninst::RegValue imm,
                                         bool /*useRVC*/)
{
    // No available RVC optimization
    makeBTypeInsn(gen, rs1, rs2, imm & BTYPE_IMM_MASK, BLTFunct3, BRANCHOp);
    return false;
}

bool insnCodeGen::generateBge(codeGen &gen,
                              Dyninst::Register rs1,
                              Dyninst::Register rs2,
                              Dyninst::RegValue imm,
                              bool /*useRVC*/)
{
    // No available RVC optimization
    makeBTypeInsn(gen, rs1, rs2, imm & BTYPE_IMM_MASK, BGEFunct3, BRANCHOp);
    return false;
}

bool insnCodeGen::generateBltu(codeGen &gen,
                               Dyninst::Register rs1,
                               Dyninst::Register rs2,
                               Dyninst::RegValue imm,
                               bool /*useRVC*/)
{
    // No available RVC optimization
    makeBTypeInsn(gen, rs1, rs2, imm & BTYPE_IMM_MASK, BLTUFunct3, BRANCHOp);
    return false;
}

bool insnCodeGen::generateBgeu(codeGen &gen,
                               Dyninst::Register rs1,
                               Dyninst::Register rs2,
                               Dyninst::RegValue imm,
                               bool /*useRVC*/)
{
    // No available RVC optimization
    makeBTypeInsn(gen, rs1, rs2, imm & BTYPE_IMM_MASK, BGEUFunct3, BRANCHOp);
    return false;
}

bool insnCodeGen::generateJ(codeGen &gen,
                            Dyninst::RegValue offset,
                            bool useRVC)
{
    if (useRVC) {
        if (offset >= -CJ_IMM_MIN && offset < CJ_IMM_MAX) {
            // use c.j
            generateCJ(gen, (offset >> CJ_IMM_SHIFT) & CJ_IMM_MASK);
            return true;
        }
    }
    generateJal(gen, GPR_ZERO, offset, false);
    return false;
}

bool insnCodeGen::generateJal(codeGen &gen,
                              Dyninst::Register rd,
                              Dyninst::RegValue offset,
                              bool /*useRVC*/)
{
    // No optimization for RV64 (c.jal is RV32)
    Dyninst::RegValue shiftedImm = (offset >> JTYPE_IMM_SHIFT) & JTYPE_IMM_MASK;
    makeJTypeInsn(gen, rd, shiftedImm, JALOp);
    return false;
}

bool insnCodeGen::generateJr(codeGen &gen,
                             Dyninst::Register rs,
                             Dyninst::RegValue offset,
                             bool useRVC)
{
    if (useRVC) {
        if (offset == 0) {
            // c.j
            generateCJr(gen, rs);
            return true;
        }
    }
    generateJalr(gen, GPR_ZERO, rs, offset, false);
    return false;
}

bool insnCodeGen::generateJalr(codeGen &gen,
                               Dyninst::Register rd,
                               Dyninst::Register rs,
                               Dyninst::RegValue offset,
                               bool useRVC)
{
    if (useRVC) {
        if (offset == 0 && rd == GPR_RA) {
            // c.jalr
            generateCJalr(gen, rs);
            return true;
        }
    }

    // JALR is an I-Type instruction
    makeITypeInsn(gen, rd, rs, offset & ITYPE_IMM_MASK, JALRFunct3, JALROp);
    return false;
}



void insnCodeGen::generateCAdd(codeGen &gen,
                               Dyninst::Register rd,
                               Dyninst::Register rs)
{
    instruction insn;
    INSN_BUFF_SET(insn, 13, 15, 0x4); // func3 = 100
    INSN_BUFF_SET(insn, 12, 12, 0x1); // imm[5] != 0
    INSN_BUFF_SET(insn, 7, 11, rd);   // rsi/rd != 0
    INSN_BUFF_SET(insn, 2, 6, rs);    // imm[4:0] != 0
    INSN_BUFF_SET(insn, 0, 1, 0x1);   // opcode = 10
    insnCodeGen::generate(gen, insn);
}

void insnCodeGen::generateCAddi(codeGen &gen,
                                Dyninst::Register rd,
                                Dyninst::RegValue imm)
{
    instruction insn;
    INSN_BUFF_SET(insn, 13, 15, 0x0);            // func3 = 000
    INSN_BUFF_SET(insn, 12, 12, (imm >> 5) & 1); // imm[5] != 0
    INSN_BUFF_SET(insn, 7, 11, rd);              // rsi/rd != 0
    INSN_BUFF_SET(insn, 2, 6, imm & 0x1f);       // imm[4:0] != 0
    INSN_BUFF_SET(insn, 0, 1, 0x1);              // opcode = 01
    insnCodeGen::generate(gen, insn);
}

void insnCodeGen::generateCAddi4spn(codeGen &gen,
                                    Dyninst::Register rd,
                                    Dyninst::RegValue imm)
{
    instruction insn;
    INSN_BUFF_SET(insn, 13, 15, 0x0);              // func3 = 000
    INSN_BUFF_SET(insn, 11, 12, (imm >> 2) & 0x3); // imm[3:2]
    INSN_BUFF_SET(insn, 7, 10, (imm >> 4) & 0xf);  // imm[7:4]
    INSN_BUFF_SET(insn, 6, 6, (imm >> 0) & 0x1);   // imm[0]
    INSN_BUFF_SET(insn, 5, 5, (imm >> 1) & 0x1);   // imm[1]
    INSN_BUFF_SET(insn, 2, 4, rd);                 // rd
    INSN_BUFF_SET(insn, 0, 1, 0x0);                // opcode = 00
    insnCodeGen::generate(gen, insn);
}

void insnCodeGen::generateCAddi16sp(codeGen &gen,
                                    Dyninst::RegValue imm)
{
    instruction insn;

    INSN_BUFF_SET(insn, 13, 15, 0x3);              // func3 = 011
    INSN_BUFF_SET(insn, 12, 12, (imm >> 5) & 0x1); // imm[5]
    INSN_BUFF_SET(insn, 7, 11, 0x2);               // 00010
    INSN_BUFF_SET(insn, 6, 6, (imm >> 0) & 0x1);   // imm[0]
    INSN_BUFF_SET(insn, 5, 5, (imm >> 2) & 0x1);   // imm[2]
    INSN_BUFF_SET(insn, 3, 4, (imm >> 3) & 0x3);   // imm[4:3]
    INSN_BUFF_SET(insn, 2, 2, (imm >> 1) & 0x1);   // imm[1]
    INSN_BUFF_SET(insn, 0, 1, 0x1);                // opcode = 01

    insnCodeGen::generate(gen, insn);
}

void insnCodeGen::generateCAnd(codeGen &gen,
                               Dyninst::Register rd,
                               Dyninst::Register rs)
{
    instruction insn;

    INSN_BUFF_SET(insn, 10, 15, 0x23); // 100011
    INSN_BUFF_SET(insn, 7, 9, rd);     // rd
    INSN_BUFF_SET(insn, 5, 6, 0x3);    // 11
    INSN_BUFF_SET(insn, 2, 4, rs);     // rs
    INSN_BUFF_SET(insn, 0, 1, 0x1);    // opcode = 01

    insnCodeGen::generate(gen, insn);
}

void insnCodeGen::generateCAndi(codeGen &gen,
                                Dyninst::Register rd,
                                Dyninst::RegValue imm)
{
    instruction insn;

    INSN_BUFF_SET(insn, 13, 15, 0x4);            // func3 = 100
    INSN_BUFF_SET(insn, 12, 12, (imm >> 5) & 1); // imm[5]
    INSN_BUFF_SET(insn, 10, 11, 0x10);           // 10
    INSN_BUFF_SET(insn, 7, 9, rd);               // rd
    INSN_BUFF_SET(insn, 2, 6, imm & 0x1f);       // imm[4:0]
    INSN_BUFF_SET(insn, 0, 1, 0x1);               // opcode = 01

    insnCodeGen::generate(gen, insn);
}

void insnCodeGen::generateCLw(codeGen &gen,
                              Dyninst::Register rd,
                              Dyninst::Register rs1,
                              Dyninst::RegValue imm)
{
    instruction insn;
    INSN_BUFF_SET(insn, 13, 15, 0x2);              // func3 = 010
    INSN_BUFF_SET(insn, 10, 12, (imm >> 1) & 0x7); // imm[3:1]
    INSN_BUFF_SET(insn, 7, 9, rs1);                // rs1
    INSN_BUFF_SET(insn, 6, 6, imm & 0x1);          // imm[0]
    INSN_BUFF_SET(insn, 5, 5, (imm >> 4) & 0x1);   // imm[4]
    INSN_BUFF_SET(insn, 2, 4, rd);                 // rd
    INSN_BUFF_SET(insn, 0, 1, 0x0);                // opcode = 00
    insnCodeGen::generate(gen, insn);
}

void insnCodeGen::generateCLwsp(codeGen &gen,
                                Dyninst::Register rd,
                                Dyninst::RegValue imm)
{
    instruction insn;
    INSN_BUFF_SET(insn, 13, 15, 0x2);              // func3 = 010
    INSN_BUFF_SET(insn, 12, 12, (imm >> 3) & 0x1); // imm[3]
    INSN_BUFF_SET(insn, 7, 11, rd);                // rd
    INSN_BUFF_SET(insn, 4, 6, imm & 0x7);          // imm[2:0]
    INSN_BUFF_SET(insn, 2, 3, (imm >> 4) & 0x3);   // imm[5:4]
    INSN_BUFF_SET(insn, 0, 1, 0x2);                // opcode = 10
    insnCodeGen::generate(gen, insn);
}

void insnCodeGen::generateCLd(codeGen &gen,
                              Dyninst::Register rd,
                              Dyninst::Register rs1,
                              Dyninst::RegValue imm)
{
    instruction insn;
    INSN_BUFF_SET(insn, 13, 15, 0x3);            // func3 = 011
    INSN_BUFF_SET(insn, 10, 12, imm & 0x7);      // imm[2:0]
    INSN_BUFF_SET(insn, 7, 9, rs1);              // rs1
    INSN_BUFF_SET(insn, 5, 6, (imm >> 3) & 0x3); // imm[4:3]
    INSN_BUFF_SET(insn, 2, 4, rd);               // rd
    INSN_BUFF_SET(insn, 0, 1, 0x0);              // opcode = 00
    insnCodeGen::generate(gen, insn);
}

void insnCodeGen::generateCLdsp(codeGen &gen,
                                Dyninst::Register rd,
                                Dyninst::RegValue imm)
{
    instruction insn;
    INSN_BUFF_SET(insn, 13, 15, 0x3);              // func3 = 011
    INSN_BUFF_SET(insn, 12, 12, (imm >> 2) & 0x1); // imm[2]
    INSN_BUFF_SET(insn, 7, 11, rd);                // rd
    INSN_BUFF_SET(insn, 5, 6, imm & 0x3);          // imm[1:0]
    INSN_BUFF_SET(insn, 2, 4, (imm >> 3) & 0x7);   // imm[5:3]
    INSN_BUFF_SET(insn, 0, 1, 0x2);                // opcode = 10
    insnCodeGen::generate(gen, insn);
}

void insnCodeGen::generateCFlw(codeGen &gen,
                               Dyninst::Register rd,
                               Dyninst::Register rs1,
                               Dyninst::RegValue imm)
{
    instruction insn;
    INSN_BUFF_SET(insn, 13, 15, 0x3);              // func3 = 011
    INSN_BUFF_SET(insn, 10, 12, (imm >> 1) & 0x7); // imm[3:1]
    INSN_BUFF_SET(insn, 7, 9, rs1);                // rs1
    INSN_BUFF_SET(insn, 6, 6, imm & 0x1);          // imm[0]
    INSN_BUFF_SET(insn, 5, 5, (imm >> 4) & 0x1);   // imm[4]
    INSN_BUFF_SET(insn, 2, 4, rd);                 // rd
    INSN_BUFF_SET(insn, 0, 1, 0x0);                // opcode = 00
    insnCodeGen::generate(gen, insn);
}

void insnCodeGen::generateCFlwsp(codeGen &gen,
                                 Dyninst::Register rd,
                                 Dyninst::RegValue imm)
{
    instruction insn;
    INSN_BUFF_SET(insn, 13, 15, 0x3);              // func3 = 011
    INSN_BUFF_SET(insn, 12, 12, (imm >> 3) & 0x1); // imm[3]
    INSN_BUFF_SET(insn, 7, 11, rd);                // rd
    INSN_BUFF_SET(insn, 4, 6, imm & 0x7);          // imm[2:0]
    INSN_BUFF_SET(insn, 2, 3, (imm >> 4) & 0x3);   // imm[5:4]
    INSN_BUFF_SET(insn, 0, 1, 0x2);                // opcode = 10
    insnCodeGen::generate(gen, insn);
}

void insnCodeGen::generateCFld(codeGen &gen,
                               Dyninst::Register rd,
                               Dyninst::Register rs1,
                               Dyninst::RegValue imm)
{
    instruction insn;
    INSN_BUFF_SET(insn, 13, 15, 0x1);            // func3 = 001
    INSN_BUFF_SET(insn, 10, 12, imm & 0x7);      // imm[2:0]
    INSN_BUFF_SET(insn, 7, 9, rs1);              // rs1
    INSN_BUFF_SET(insn, 5, 6, (imm >> 3) & 0x3); // imm[4:3]
    INSN_BUFF_SET(insn, 2, 4, rd);               // rd
    INSN_BUFF_SET(insn, 0, 1, 0x0);              // opcode = 00
    insnCodeGen::generate(gen, insn);
}

void insnCodeGen::generateCFldsp(codeGen &gen,
                                 Dyninst::Register rd,
                                 Dyninst::RegValue imm)
{
    instruction insn;
    INSN_BUFF_SET(insn, 13, 15, 0x1);              // func3 = 001
    INSN_BUFF_SET(insn, 12, 12, (imm >> 2) & 0x1); // imm[2]
    INSN_BUFF_SET(insn, 7, 11, rd);                // rd
    INSN_BUFF_SET(insn, 5, 6, imm & 0x3);          // imm[1:0]
    INSN_BUFF_SET(insn, 2, 4, (imm >> 3) & 0x7);   // imm[5:3]
    INSN_BUFF_SET(insn, 0, 1, 0x2);                // opcode = 10
    insnCodeGen::generate(gen, insn);
}



void insnCodeGen::generateCLi(codeGen &gen,
                              Dyninst::Register rd,
                              Dyninst::RegValue imm)
{
    instruction insn;
    INSN_BUFF_SET(insn, 13, 15, 0x2);            // func3 = 010
    INSN_BUFF_SET(insn, 12, 12, (imm >> 5) & 1); // imm[5]
    INSN_BUFF_SET(insn, 7, 11, rd);              // rd
    INSN_BUFF_SET(insn, 2, 6, imm & 0x1f);       // imm[4:0]
    INSN_BUFF_SET(insn, 0, 1, 0x1);              // opcode = 01
    insnCodeGen::generate(gen, insn);
}

void insnCodeGen::generateCLui(codeGen &gen,
                               Dyninst::Register rd,
                               Dyninst::RegValue imm)
{
    instruction insn;

    INSN_BUFF_SET(insn, 13, 15, 0x3);            // func3 = 011
    INSN_BUFF_SET(insn, 12, 12, (imm >> 5) & 1); // imm[5]
    INSN_BUFF_SET(insn, 7, 11, rd);              // rd
    INSN_BUFF_SET(insn, 2, 6, imm & 0x1f);       // imm[4:0]
    INSN_BUFF_SET(insn, 0, 1, 0x1);              // opcode = 01

    insnCodeGen::generate(gen, insn);
}

void insnCodeGen::generateCSw(codeGen &gen,
                              Dyninst::Register rd,
                              Dyninst::Register rs1,
                              Dyninst::RegValue imm)
{
    instruction insn;
    INSN_BUFF_SET(insn, 13, 15, 0x6);              // func3 = 110
    INSN_BUFF_SET(insn, 10, 12, (imm >> 1) & 0x7); // imm[3:1]
    INSN_BUFF_SET(insn, 7, 9, rs1);                // rs1
    INSN_BUFF_SET(insn, 6, 6, imm & 0x1);          // imm[0]
    INSN_BUFF_SET(insn, 5, 5, (imm >> 4) & 0x1);   // imm[4]
    INSN_BUFF_SET(insn, 2, 4, rd);                 // rd
    INSN_BUFF_SET(insn, 0, 1, 0x0);                // opcode = 00
    insnCodeGen::generate(gen, insn);
}

void insnCodeGen::generateCSwsp(codeGen &gen,
                                Dyninst::Register rs2,
                                Dyninst::RegValue imm)
{
    instruction insn;
    INSN_BUFF_SET(insn, 13, 15, 0x6);              // func3 = 110
    INSN_BUFF_SET(insn, 9, 12, imm & 0xf);         // imm[3:0]
    INSN_BUFF_SET(insn, 7, 8, (imm >> 4) & 0x3);   // imm[5:4]
    INSN_BUFF_SET(insn, 2, 6, rs2);                // rs2
    INSN_BUFF_SET(insn, 0, 1, 0x2);                // opcode = 10
    insnCodeGen::generate(gen, insn);
}

void insnCodeGen::generateCSd(codeGen &gen,
                              Dyninst::Register rd,
                              Dyninst::Register rs1,
                              Dyninst::RegValue imm)
{
    instruction insn;
    INSN_BUFF_SET(insn, 13, 15, 0x7);            // func3 = 111
    INSN_BUFF_SET(insn, 10, 12, imm & 0x7);      // imm[2:0]
    INSN_BUFF_SET(insn, 7, 9, rs1);              // rs1
    INSN_BUFF_SET(insn, 5, 6, (imm >> 3) & 0x3); // imm[4:3]
    INSN_BUFF_SET(insn, 2, 4, rd);               // rd
    INSN_BUFF_SET(insn, 0, 1, 0x0);              // opcode = 00
    insnCodeGen::generate(gen, insn);
}

void insnCodeGen::generateCSdsp(codeGen &gen,
                                Dyninst::Register rs2,
                                Dyninst::RegValue imm)
{
    instruction insn;
    INSN_BUFF_SET(insn, 13, 15, 0x7);              // func3 = 111
    INSN_BUFF_SET(insn, 10, 12, imm & 0x7);        // imm[2:0]
    INSN_BUFF_SET(insn, 7, 9, (imm >> 3) & 0x7);   // imm[5:3]
    INSN_BUFF_SET(insn, 2, 6, rs2);                // rs2
    INSN_BUFF_SET(insn, 0, 1, 0x2);                // opcode = 10
    insnCodeGen::generate(gen, insn);
}

void insnCodeGen::generateCFsw(codeGen &gen,
                               Dyninst::Register rd,
                               Dyninst::Register rs1,
                               Dyninst::RegValue imm)
{
    instruction insn;
    INSN_BUFF_SET(insn, 13, 15, 0x7);              // func3 = 111
    INSN_BUFF_SET(insn, 10, 12, (imm >> 1) & 0x7); // imm[3:1]
    INSN_BUFF_SET(insn, 7, 9, rs1);                // rs1
    INSN_BUFF_SET(insn, 6, 6, imm & 0x1);          // imm[0]
    INSN_BUFF_SET(insn, 5, 5, (imm >> 4) & 0x1);   // imm[4]
    INSN_BUFF_SET(insn, 2, 4, rd);                 // rd
    INSN_BUFF_SET(insn, 0, 1, 0x0);                // opcode = 00
    insnCodeGen::generate(gen, insn);
}

void insnCodeGen::generateCFswsp(codeGen &gen,
                                 Dyninst::Register rs2,
                                 Dyninst::RegValue imm)
{
    instruction insn;
    INSN_BUFF_SET(insn, 13, 15, 0x7);              // func3 = 111
    INSN_BUFF_SET(insn, 9, 12, imm & 0xf);         // imm[3:0]
    INSN_BUFF_SET(insn, 7, 8, (imm >> 4) & 0x3);   // imm[5:4]
    INSN_BUFF_SET(insn, 2, 6, rs2);                // rs2
    INSN_BUFF_SET(insn, 0, 1, 0x2);                // opcode = 10
    insnCodeGen::generate(gen, insn);
}

void insnCodeGen::generateCFsd(codeGen &gen,
                               Dyninst::Register rd,
                               Dyninst::Register rs1,
                               Dyninst::RegValue imm)
{
    instruction insn;
    INSN_BUFF_SET(insn, 13, 15, 0x5);            // func3 = 101
    INSN_BUFF_SET(insn, 10, 12, imm & 0x7);      // imm[2:0]
    INSN_BUFF_SET(insn, 7, 9, rs1);              // rs1
    INSN_BUFF_SET(insn, 5, 6, (imm >> 3) & 0x3); // imm[4:3]
    INSN_BUFF_SET(insn, 2, 4, rd);               // rd
    INSN_BUFF_SET(insn, 0, 1, 0x0);              // opcode = 00
    insnCodeGen::generate(gen, insn);
}

void insnCodeGen::generateCFsdsp(codeGen &gen,
                                 Dyninst::Register rs2,
                                 Dyninst::RegValue imm)
{
    instruction insn;
    INSN_BUFF_SET(insn, 13, 15, 0x5);              // func3 = 101
    INSN_BUFF_SET(insn, 10, 12, imm & 0x7);        // imm[2:0]
    INSN_BUFF_SET(insn, 7, 9, (imm >> 3) & 0x7);   // imm[5:3]
    INSN_BUFF_SET(insn, 2, 6, rs2);                // rs2
    INSN_BUFF_SET(insn, 0, 1, 0x2);                // opcode = 10
    insnCodeGen::generate(gen, insn);
}


void insnCodeGen::generateCSlli(codeGen &gen, 
                                Dyninst::Register rd,
                                Dyninst::RegValue uimm)
{
    instruction insn;

    INSN_BUFF_SET(insn, 13, 15, 0x2);             // func3 = 010
    INSN_BUFF_SET(insn, 12, 12, (uimm >> 5) & 1); // uimm[5]
    INSN_BUFF_SET(insn, 7, 11, rd);               // rd
    INSN_BUFF_SET(insn, 2, 6, uimm & 0x1f);       // uimm[4:0]
    INSN_BUFF_SET(insn, 0, 1, 0x2);               // opcode = 10

    insnCodeGen::generate(gen, insn);
}

void insnCodeGen::generateCSrli(codeGen &gen,
                                Dyninst::Register rd,
                                Dyninst::RegValue uimm)
{
    instruction insn;

    INSN_BUFF_SET(insn, 13, 15, 0x4);             // func3 = 100
    INSN_BUFF_SET(insn, 12, 12, (uimm >> 5) & 1); // uimm[5]
    INSN_BUFF_SET(insn, 10, 11, 0x0);             // 00
    INSN_BUFF_SET(insn, 7, 9, rd);                // rd
    INSN_BUFF_SET(insn, 2, 6, uimm & 0x1f);       // uimm[4:0]
    INSN_BUFF_SET(insn, 0, 1, 0x1);               // opcode = 01

    insnCodeGen::generate(gen, insn);
}

void insnCodeGen::generateCSrai(codeGen &gen,
                                Dyninst::Register rd,
                                Dyninst::RegValue uimm)
{
    instruction insn;

    INSN_BUFF_SET(insn, 13, 15, 0x4);             // func3 = 100
    INSN_BUFF_SET(insn, 12, 12, (uimm >> 5) & 1); // uimm[5]
    INSN_BUFF_SET(insn, 10, 11, 0x1);             // 01
    INSN_BUFF_SET(insn, 7, 9, rd);                // rd
    INSN_BUFF_SET(insn, 2, 6, uimm & 0x1f);       // uimm[4:0]
    INSN_BUFF_SET(insn, 0, 1, 0x1);               // opcode = 01

    insnCodeGen::generate(gen, insn);
}

void insnCodeGen::generateCMv(codeGen &gen,
                              Dyninst::Register rd,
                              Dyninst::Register rs)
{
    instruction insn;

    INSN_BUFF_SET(insn, 13, 15, 0x4); // func3 = 100
    INSN_BUFF_SET(insn, 12, 12, 0x0); // 0
    INSN_BUFF_SET(insn, 7, 11, rd);   // rd
    INSN_BUFF_SET(insn, 2, 6, rs);    // rs
    INSN_BUFF_SET(insn, 0, 1, 0x2);   // opcode = 10

    insnCodeGen::generate(gen, insn);
}

void insnCodeGen::generateCOr(codeGen &gen,
                              Dyninst::Register rd,
                              Dyninst::Register rs)
{
    instruction insn;

    INSN_BUFF_SET(insn, 10, 15, 0x23); // 100011
    INSN_BUFF_SET(insn, 7, 9, rd);     // rd
    INSN_BUFF_SET(insn, 5, 6, 0x2);    // 10
    INSN_BUFF_SET(insn, 2, 4, rs);     // rs
    INSN_BUFF_SET(insn, 0, 1, 0x1);    // opcode = 01

    insnCodeGen::generate(gen, insn);
}

void insnCodeGen::generateCJ(codeGen &gen,
                             Dyninst::RegValue offset)
{
    instruction insn;

    INSN_BUFF_SET(insn, 13, 15, 0x5);                  // func3 = 101
    INSN_BUFF_SET(insn, 12, 12, (offset >> 10) & 0x1); // imm[10]
    INSN_BUFF_SET(insn, 11, 11, (offset >> 3) & 0x1);  // imm[3]
    INSN_BUFF_SET(insn, 9, 10, (offset >> 7) & 0x3);   // imm[8:7]
    INSN_BUFF_SET(insn, 8, 8, (offset >> 9) & 0x1);    // imm[9]
    INSN_BUFF_SET(insn, 7, 7, (offset >> 5) & 0x1);    // imm[5]
    INSN_BUFF_SET(insn, 6, 6, (offset >> 6) & 0x1);    // imm[6]
    INSN_BUFF_SET(insn, 3, 5, offset & 0x7);           // imm[2:0]
    INSN_BUFF_SET(insn, 2, 2, (offset >> 4) & 0x1);    // imm[4]
    INSN_BUFF_SET(insn, 0, 1, 0x1);                    // opcode = 01

    insnCodeGen::generate(gen, insn);
}

void insnCodeGen::generateCJal(codeGen &gen,
                               Dyninst::RegValue offset)
{
    instruction insn;

    INSN_BUFF_SET(insn, 12, 12, (offset >> 10) & 0x1); // imm[10]
    INSN_BUFF_SET(insn, 11, 11, (offset >> 3) & 0x1);  // imm[3]
    INSN_BUFF_SET(insn, 9, 10, (offset >> 7) & 0x3);   // imm[8:7]
    INSN_BUFF_SET(insn, 8, 8, (offset >> 9) & 0x1);    // imm[9]
    INSN_BUFF_SET(insn, 7, 7, (offset >> 5) & 0x1);    // imm[5]
    INSN_BUFF_SET(insn, 6, 6, (offset >> 6) & 0x1);    // imm[6]
    INSN_BUFF_SET(insn, 3, 5, offset & 0x7);           // imm[2:0]
    INSN_BUFF_SET(insn, 2, 2, (offset >> 4) & 0x1);    // imm[4]
    INSN_BUFF_SET(insn, 0, 1, 0x1);                    // opcode = 01

    insnCodeGen::generate(gen, insn);
}

void insnCodeGen::generateCJr(codeGen &gen,
                              Dyninst::Register rs)
{
    instruction insn;

    INSN_BUFF_SET(insn, 13, 15, 0x4); // func3 = 100
    INSN_BUFF_SET(insn, 12, 12, 0x0); // 0
    INSN_BUFF_SET(insn, 7, 11, rs);   // rs
    INSN_BUFF_SET(insn, 0, 6, 0x2);   // 0000010

    insnCodeGen::generate(gen, insn);
}

void insnCodeGen::generateCJalr(codeGen &gen,
                                Dyninst::Register rs)
{
    instruction insn;

    INSN_BUFF_SET(insn, 13, 15, 0x4); // func3 = 100
    INSN_BUFF_SET(insn, 12, 12, 0x1); // 1
    INSN_BUFF_SET(insn, 7, 11, rs);   // rs
    INSN_BUFF_SET(insn, 0, 6, 0x2);   // 0000010

    insnCodeGen::generate(gen, insn);
}

void insnCodeGen::generateCBeqz(codeGen &gen,
                                Dyninst::Register rs,
                                Dyninst::RegValue offset) {
    instruction insn;

    INSN_BUFF_SET(insn, 13, 15, 0x6);                 // func3 = 110
    INSN_BUFF_SET(insn, 12, 12, (offset >> 7) & 0x1); // offset[7]
    INSN_BUFF_SET(insn, 10, 11, (offset >> 2) & 0x3); // offset[3:2]
    INSN_BUFF_SET(insn, 7, 9, rs);                    // rs
    INSN_BUFF_SET(insn, 5, 6, (offset >> 5) & 0x3);   // offset[6:5]
    INSN_BUFF_SET(insn, 3, 4, offset & 0x3);          // offset[1:0]
    INSN_BUFF_SET(insn, 2, 2, (offset >> 4) & 0x1);   // offset[4]
    INSN_BUFF_SET(insn, 0, 1, 0x1);                   // 01

    insnCodeGen::generate(gen, insn);
}

void insnCodeGen::generateCBnez(codeGen &gen,
                                Dyninst::Register rs,
                                Dyninst::RegValue offset) {
    instruction insn;

    INSN_BUFF_SET(insn, 13, 15, 0x7);                 // func3 = 111
    INSN_BUFF_SET(insn, 12, 12, (offset >> 7) & 0x1); // offset[7]
    INSN_BUFF_SET(insn, 10, 11, (offset >> 2) & 0x3); // offset[3:2]
    INSN_BUFF_SET(insn, 7, 9, rs);                    // rs
    INSN_BUFF_SET(insn, 5, 6, (offset >> 5) & 0x3);   // offset[6:5]
    INSN_BUFF_SET(insn, 3, 4, offset & 0x3);          // offset[1:0]
    INSN_BUFF_SET(insn, 2, 2, (offset >> 4) & 0x1);   // offset[4]
    INSN_BUFF_SET(insn, 0, 1, 0x1);                   // 01

    insnCodeGen::generate(gen, insn);
}

void insnCodeGen::generateCNop(codeGen &gen) {
    instruction insn;

    INSN_BUFF_SET(insn, 0, 15, 0x1);

    insnCodeGen::generate(gen, insn);
}

void insnCodeGen::generateCXor(codeGen &gen,
                               Dyninst::Register rd,
                               Dyninst::Register rs)
{
    instruction insn;

    INSN_BUFF_SET(insn, 10, 15, 0x23); // 100011
    INSN_BUFF_SET(insn, 7, 9, rd);     // rd
    INSN_BUFF_SET(insn, 5, 6, 0x1);    // 01
    INSN_BUFF_SET(insn, 2, 4, rs);     // rs
    INSN_BUFF_SET(insn, 0, 1, 0x1);    // opcode = 01

    insnCodeGen::generate(gen, insn);
}
