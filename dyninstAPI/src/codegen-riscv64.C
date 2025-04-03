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

// "Casting" methods. We use a "base + offset" model, but often need to
// turn that into "current instruction pointer".
codeBuf_t *insnCodeGen::insnPtr(codeGen &gen) {
    return (instructUnion *)gen.cur_ptr();
}

void insnCodeGen::generate(codeGen &gen, instruction &insn) {
    // Call `flushInsnBuffer` to flush the instruction buffer into the
    // 2-byte code buffer `code_buff` short-by-short.
    for (unsigned i = 0; i < insn.size() / RV_MIN_INSN_SIZE; i++) {
        insn.flushInsnBuff(i * RV_MIN_INSN_SIZE * 8);
        gen.copy(insn.ptr(), RV_MIN_INSN_SIZE);
    }
}

void insnCodeGen::generate(codeGen &gen, instruction &insn, unsigned position) {
    // Call `flushInsnBuffer` to flush the instruction buffer into the
    // 2-byte code buffer `code_buff` short-by-short
    for (unsigned i = 0; i < insn.size() / RV_MIN_INSN_SIZE; i++) {
        insn.flushInsnBuff(i * RV_MIN_INSN_SIZE * 8);
        gen.insert(insn.ptr(), RV_MIN_INSN_SIZE, position);
    }
}

// Basic RISC-V instruction type generation

void insnCodeGen::makeUTypeInsn(codeGen &gen,
                                    Dyninst::Register rd,
                                    Dyninst::RegValue imm,
                                    unsigned opcode)
{
    assert(imm >= -0x80000 && imm < 0x80000);

    instruction insn;
    insn.setIsRVC(false);

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
    assert(imm >= -0x800 && imm < 0x800);

    instruction insn;
    insn.setIsRVC(false);

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
    insn.setIsRVC(false);

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
    assert(imm >= -0x1000 && imm < 0x1000);        // 13-bit signed immediate

    instruction insn;
    insn.setIsRVC(false);

    INSN_BUFF_SET(insn, 31, 31, (imm >> 12) & 0x1); // imm[12]
    INSN_BUFF_SET(insn, 25, 30, (imm >> 5) & 0x3f); // imm[10:5]
    INSN_BUFF_SET(insn, 20, 24, rs2);               // rs2
    INSN_BUFF_SET(insn, 15, 19, rs1);               // rs1
    INSN_BUFF_SET(insn, 12, 14, funct3);            // funct3
    INSN_BUFF_SET(insn, 8, 11, (imm >> 1) & 0xf);   // imm[4:1]
    INSN_BUFF_SET(insn, 7, 7, (imm >> 11) & 0x1);   // imm[11]
    INSN_BUFF_SET(insn, 0, 6, opcode);              // opcode

    insnCodeGen::generate(gen, insn);
}

void insnCodeGen::makeJTypeInsn(codeGen &gen,
                                Dyninst::Register rd,
                                Dyninst::RegValue imm,
                                unsigned opcode)
{
    assert(imm >= -0x100000 && imm < 0x100000); // 21-bit signed immediate

    instruction insn;
    insn.setIsRVC(false);


    INSN_BUFF_SET(insn, 31, 31, (imm >> 20) & 0x1);  // imm[20]
    INSN_BUFF_SET(insn, 21, 30, (imm >> 1) & 0x3ff); // imm[10:1]
    INSN_BUFF_SET(insn, 20, 20, (imm >> 11) & 0x1);  // imm[11]
    INSN_BUFF_SET(insn, 12, 19, (imm >> 12) & 0xff); // imm[19:12]
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
    assert(imm >= -800 && imm < 0x800);         // 12-bit signed immediate

    instruction insn;
    insn.setIsRVC(false);

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
    insn.setIsRVC(true);
    generate(gen, insn);
}

void insnCodeGen::generateTrap(codeGen &gen)
{
    instruction insn;
    insn.setIsRVC(false);
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

void insnCodeGen::generateLongBranch(codeGen &gen,
                                     Dyninst::Address from,
                                     Dyninst::Address to,
                                     bool isCall)
{
    Dyninst::Register scratch = Null_Register;

    if (isCall) {
        // use the link register ra (x1) as scratch since it will be overwritten at return
        scratch = GPR_RA;
        // load disp to ra
        loadImmIntoReg(gen, scratch, to, gen.getUseRVC());
        // generate jalr
        generateJalr(gen, scratch, scratch, 0, gen.getUseRVC());
    }

    instPoint *point = gen.point();
    if (point) {
        registerSpace *rs = registerSpace::actualRegSpace(point);
        gen.setRegisterSpace(rs);

        scratch = rs->getScratchRegister(gen, true);
    }

    if (scratch == Null_Register) {
        //fprintf(stderr, " %s[%d] No registers. Calling generateBranchViaTrap...\n", FILE__, __LINE__);
        generateBranchViaTrap(gen, from, to, isCall);
        return;
    }

    loadImmIntoReg(gen, scratch, to, gen.getUseRVC());
    generateJr(gen, scratch, 0, gen.getUseRVC());
}

void insnCodeGen::generateBranch(codeGen &gen,
                                 long disp,
                                 bool link)
{
    if (link) {
        if (labs(disp) > MAX_BRANCH_LINK_OFFSET) {
            fprintf(stderr, "ABS OFF: 0x%lx, MAX: 0x%lx\n",
                    (unsigned long)labs(disp), (unsigned long) MAX_BRANCH_OFFSET);
            bperr( "Error: attempted a branch and link of 0x%lx\n", (unsigned long)disp);
            logLine("a branch and link too far\n");
            showErrorCallback(52, "Internal error: branch and link too far");
            bperr( "Attempted to make a branch and link of offset 0x%lx\n", (unsigned long)disp);
            assert(0);
        }
        generateJal(gen, GPR_RA, disp, gen.getUseRVC());
    }
    else {
        if (labs(disp) > MAX_BRANCH_OFFSET) {
            fprintf(stderr, "ABS OFF: 0x%lx, MAX: 0x%lx\n",
                    (unsigned long)labs(disp), (unsigned long) MAX_BRANCH_OFFSET);
            bperr( "Error: attempted a branch of 0x%lx\n", (unsigned long)disp);
            logLine("a branch too far\n");
            showErrorCallback(52, "Internal error: branch too far");
            bperr( "Attempted to make a branch of offset 0x%lx\n", (unsigned long)disp);
            assert(0);
        }
        generateJ(gen, disp, gen.getUseRVC());
    }
}

void insnCodeGen::generateBranch(codeGen &gen,
                                 Dyninst::Address from,
                                 Dyninst::Address to,
                                 bool link)
{
    long disp = (to - from);
    if (labs(disp) > MAX_BRANCH_OFFSET) {
        generateLongBranch(gen, from, to, link);
    }
    else {
        generateBranch(gen, disp, link);
    }
}

void insnCodeGen::generateBranchViaTrap(codeGen &/*gen*/,
                                        Dyninst::Address /*from*/,
                                        Dyninst::Address /*to*/,
                                        bool /*isCall*/)
{
    // TODO
}


Dyninst::Register insnCodeGen::moveValueToReg(codeGen &gen,
                                              long int val,
                                              std::vector<Dyninst::Register> *exclude)
{
    Dyninst::Register scratchReg;
    if (exclude) {
	    scratchReg = gen.rs()->getScratchRegister(gen, *exclude, true);
    }
    else {
	    scratchReg = gen.rs()->getScratchRegister(gen, true);
    }

    if (scratchReg == Null_Register) {
        fprintf(stderr, " %s[%d] No scratch register available to generate add instruction!", FILE__, __LINE__);
        assert(0);
    }

    loadImmIntoReg(gen, scratchReg, static_cast<Dyninst::RegValue>(val), gen.getUseRVC());

    return scratchReg;
}

bool insnCodeGen::loadImmIntoReg(codeGen &gen,
                                 Dyninst::Register rd,
                                 Dyninst::RegValue value,
                                 bool useRVC)
{
    return generateLi(gen, rd, value, useRVC);
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

// Generate memory access through Load or Store
bool insnCodeGen::generateMemLoad(codeGen &gen,
                                  Dyninst::Register rd,
                                  Dyninst::Register rs,
                                  Dyninst::RegValue offset,
                                  Dyninst::RegValue size,
                                  bool isUnsigned,
                                  bool useRVC)
{
    assert(size == 1 || size == 2 || size == 4 || size == 8);
    // no "ldu" instruction, but treat "ldu" as ld
    //assert(!(size == 8 && isUnsigned));
    assert(offset >= -0x800 && offset < 0x800);

    if (useRVC) {
        // TODO
    }

    Dyninst::RegValue funct3{};
    switch (size) {
        case 1: funct3 = 0x0; break; // lb = 000
        case 2: funct3 = 0x1; break; // lh = 001
        case 4: funct3 = 0x2; break; // lw = 010
        case 8: funct3 = 0x3; break; // ld = 011
        default: break;              // not gonna happen
    }
    if (isUnsigned && size != 8) {
        funct3 |= 0x4; // lbu = 100, lhu = 101, lwu = 110
    }

    // Load instructions are I-Type
    makeITypeInsn(gen, rd, rs, offset, funct3, LOADOp);
    return false;
}

bool insnCodeGen::generateMemStore(codeGen &gen,
                                   Dyninst::Register rs1,
                                   Dyninst::Register rs2,
                                   Dyninst::RegValue offset,
                                   Dyninst::RegValue size,
                                   bool useRVC)
{
    assert(size == 1 || size == 2 || size == 4 || size == 8);
    assert(offset >= -0x800 && offset < 0x800);

    if (useRVC) {
        // TODO
    }

    Dyninst::RegValue funct3{};
    switch (size) {
        case 1: funct3 = 0x0; break; // lb = 000
        case 2: funct3 = 0x1; break; // lh = 001
        case 4: funct3 = 0x2; break; // lw = 010
        case 8: funct3 = 0x3; break; // ld = 011
        default: break;              // not gonna happen
    }

    // Store instructions are S-Type
    makeSTypeInsn(gen, rs1, rs2, offset, funct3, STOREOp);
    return false;
}

bool insnCodeGen::generateMemLoadFp(codeGen & /*gen*/,
                                    Dyninst::Register /*rd*/,
                                    Dyninst::Register /*rs*/,
                                    Dyninst::RegValue /*offset*/,
                                    Dyninst::RegValue /*size*/,
                                    bool /*useRVC*/)
{
    // TODO Load instructions for FPRs
    return false;
}

bool insnCodeGen::generateMemStoreFp(codeGen &/*gen*/,
                                     Dyninst::Register /*rs1*/,
                                     Dyninst::Register /*rs2*/,
                                     Dyninst::RegValue /*offset*/,
                                     Dyninst::RegValue /*size*/,
                                     bool /*useRVC*/)
{
    // TODO Store instructions for FPRs
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
    return generateMemStore(gen, REG_SP, r, sp_offset, 8, useRVC);
}


bool insnCodeGen::restoreRegister(codeGen &gen,
                                  Dyninst::Register r,
                                  int sp_offset,
                                  bool useRVC)
{
    return generateMemLoad(gen, r, REG_SP, sp_offset, 8, true, useRVC);
}

bool insnCodeGen::modifyData(Dyninst::Address /*target*/,
			                 NS_riscv64::instruction &insn,
			                 codeGen &gen) {
    gen.copy(insn.ptr(), insn.size());
    return true;
}

// RISC-V I-Type Instructions

bool insnCodeGen::generateAddi(codeGen &gen,
                               Dyninst::Register rd,
                               Dyninst::Register rs,
                               Dyninst::RegValue imm,
                               bool useRVC)
{
    assert(imm < 0x800 && imm >= -0x800);

    if (useRVC) {
        // If rd == rs == zero && imm == 0, the instruction is essentially NOP (c.nop)
        if (rd == 0 && rs == 0 && imm == 0) {
            generateCNop(gen);
            return true;
        }

        // If imm is 6 bits wide (-32 <= imm < 32) and rs == zero
        // we use the c.li instruction
        if (imm >= -0x20 && imm < 0x20 && rs == 0) {
            generateCLoadImm(gen, rd, imm);
            return true;
        }

        // If imm == 0, we use the c.mv instruction
        if (imm == 0) {
            generateCMv(gen, rd, rs);
            return true;
        }

        // If rs == sp && x8 <= rd < x16 && 0 <= imm < 0x400 && imm % 4 == 0
        // we use c.addi4spn
        if (rs == GPR_SP && rd >= 8 && rd < 16 && imm >= 0 && imm < 0x400 && imm % 4 == 0) {
            generateCAddi4spn(gen, rd, imm >> 2);
            return true;
        }

        // If rd == rs == sp && -0x200 <= imm < 0x200 && imm % 16 == 0
        // we use c.addi16sp
        if (rd == rs && rs == GPR_SP && imm >= -0x200 && imm < 0x200 && imm % 16 == 0) {
            generateCAddi16sp(gen, imm >> 4);
            return true;
        }

        // If imm is 6 bits wide (-32 <= imm < 32) and imm != 0 and rd != zero
        // we use the c.addi instruction
        if (imm >= -0x20 && imm < 0x20) {
            generateCAddi(gen, rd, imm);
            return true;
        }
    }
    // Otherwise, generate addi
    makeITypeInsn(gen, rd, rs, imm, ADDFunct3, IMMOp);
    return false;
}

bool insnCodeGen::generateSlli(codeGen &gen,
                               Dyninst::Register rd,
                               Dyninst::Register rs,
                               Dyninst::RegValue imm,
                               bool useRVC)
{
    assert(imm >= 0 && imm < 64);

    if (useRVC) {
        // If rd == rs, use c.slli
        if (rd == rs) {
            generateCSlli(gen, rd, imm);
            return false;
        }
    }
    makeITypeInsn(gen, rd, rs, imm, SLLFunct3, IMMOp);
    return true;
}

bool insnCodeGen::generateSrli(codeGen &gen,
                               Dyninst::Register rd,
                               Dyninst::Register rs,
                               Dyninst::RegValue imm,
                               bool useRVC) {
    assert(imm >= 0 && imm < 64);

    if (useRVC) {
        // If rd == rs, use c.srli
        if (rd == rs && rd >= 8 && rd < 16) {
            generateCSrli(gen, rd, imm);
            return false;
        }
    }
    makeITypeInsn(gen, rd, rs, imm, SRLFunct3, IMMOp);
    return true;
}

bool insnCodeGen::generateSrai(codeGen &gen,
                               Dyninst::Register rd,
                               Dyninst::Register rs,
                               Dyninst::RegValue imm,
                               bool useRVC)
{
    if (useRVC) {
        // If rd == rs, use c.srai
        if (rd == rs && rd >= 8 && rd < 16) {
            generateCSrai(gen, rd, imm);
            return false;
        }
    }
    // srai is essentially srli with bit 30 set to 1
    imm |= 0x400;

    makeITypeInsn(gen, rd, rs, imm, SRAFunct3, IMMOp);
    return true;
}
bool insnCodeGen::generateAndi(codeGen &gen,
                               Dyninst::Register rd,
                               Dyninst::Register rs,
                               Dyninst::RegValue imm,
                               bool useRVC)
{
    assert(imm >= -0x800 && imm < 0x800);

    if (useRVC) {
        // If rd == rs, use c.andi
        if (rd == rs && rd >= 8 && rd < 16) {
            generateCAndi(gen, rd, imm);
            return false;
        }
    }
    makeITypeInsn(gen, rd, rs, imm, ANDFunct3, IMMOp);
    return true;
}

bool insnCodeGen::generateOri(codeGen &gen,
                              Dyninst::Register rd,
                              Dyninst::Register rs,
                              Dyninst::RegValue imm,
                              bool /*useRVC*/)
{
    assert(imm >= -0x800 && imm < 0x800);

    // No available RVC optimization
    makeITypeInsn(gen, rd, rs, imm, ORFunct3, IMMOp);
    return false;
}

bool insnCodeGen::generateXori(codeGen &gen,
                               Dyninst::Register rd,
                               Dyninst::Register rs,
                               Dyninst::RegValue imm,
                               bool /*useRVC*/)
{
    assert(imm >= -0x800 && imm < 0x800);

    // No available RVC optimization
    makeITypeInsn(gen, rd, rs, imm, XORFunct3, IMMOp);
    return false;
}

bool insnCodeGen::generateLui(codeGen &gen,
                              Dyninst::Register rd,
                              Dyninst::RegValue imm,
                              bool useRVC)
{
    if (useRVC) {
        // If imm is 6 bits wide (-32 <= imm < 32), we use the c.lui instruction
        if (imm >= -0x20 && imm < 0x20) {
            generateCLoadUpperImm(gen, rd, imm);
            return true;
        }
    }
    makeUTypeInsn(gen, rd, imm, LUIOp);
    return false;
}

bool insnCodeGen::generateAuipc(codeGen &gen,
                                Dyninst::Register rd,
                                Dyninst::RegValue offset,
                                bool /*useRVC*/)
{
    // No available RVC optimization

    makeUTypeInsn(gen, rd, offset, AUIPCOp);
    return false;
}

bool insnCodeGen::generateAdd(codeGen &gen,
                              Dyninst::Register rd,
                              Dyninst::Register rs1,
                              Dyninst::Register rs2,
                              bool useRVC)
{
    if (useRVC) {
        // If rd == rs, use c.add
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
        // If rd == rs, use c.and
        if (rs1 == rs2) {
            generateCAnd(gen, rd, rs1);
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
        // If rd == rs, use c.or
        if (rs1 == rs2) {
            generateCOr(gen, rd, rs1);
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
        // If rd == rs, use c.xor
        if (rs1 == rs2) {
            generateCXor(gen, rd, rs1);
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
                             bool useRVC)
{
    if (useRVC) {
        // If imm is 6 bits wide (-32 <= imm < 32), we use the c.li instruction
        if (imm >= -0x20 && imm < 0x20) {
            generateCLoadImm(gen, rd, imm);
            return true;
        }

        // If imm is 12 bits wide (-0x800 <= imm < 0x800), we use the addi instruction
        if (imm >= -0x800 && imm < 0x800) {
            generateAddi(gen, rd, 0, imm, useRVC);
            return true;
        }

        // If imm is larger than 12 bits but less than 32 bits,
        // imm must be loaded in two steps using lui and addi
        if (imm >= -0x80000000LL && imm < 0x80000000LL) {
            Dyninst::RegValue lui_imm = (imm >> 12) & 0xfffff;
            Dyninst::RegValue addi_imm = imm & 0xfff;
            // If the most significant bit of addi_imm is 1 (addi_imm is negative), we should add 1 to lui_imm
            if (addi_imm & 0x800) {
                lui_imm = (lui_imm + 1) & 0xfffff;
            }
            generateLui(gen, rd, lui_imm, useRVC);
            generateAddi(gen, rd, rd, addi_imm, useRVC);
            return true;
        }
    }

    // If imm is a 64 bit long, the sequence of instructions is more complicated
    // See the following functions for more information on how GCC generates immediate integers
    // https://gcc.gnu.org/git/?p=gcc.git;a=blob;f=gcc/config/riscv/riscv.cc;h=65e09842fde8b15b92a8399cea2493b5b239f93c;hb=HEAD#l828
    // But for the sake of simplicity, we will use a much simpler algorithm
    // Assume that we want to perform li t0, 0xdeadbeefcafebabe
    // We break the integer into the following
    //   lui t0, 0xdeadc
    //   addi t0, t0, -0x110
    //   slli t0, t0, 12
    //   addi t0, t0, -0x250
    //   slli t0, t0, 12
    //   addi t0, t0, -0x186
    //   slli t0, t0, 8
    //   addi t0, t0, 0xef

    // Decompose the immediate into various chunks
    // For example, if imm == 0xdeadbeefcafebabe:
    //  deadb     eef       caf       eba       be
    // ------- --------- --------- --------- ---------
    // lui_imm addi_imm0 addi_imm1 addi_imm2 addi_imm3

    // Upper 32 bits
    Dyninst::RegValue lui_imm = (imm >> 44) & 0xfffff; // imm[63:44]
    Dyninst::RegValue addi_imm0 = (imm >> 32) & 0xfff; // imm[43:32]
    // Lower 32 bits
    Dyninst::RegValue slli_imm1 = 12;
    Dyninst::RegValue addi_imm1 = (imm >> 20) & 0xfff; // imm[31:20]
    Dyninst::RegValue slli_imm2 = 12;
    Dyninst::RegValue addi_imm2 = (imm >> 8) & 0xfff;  // imm[19:8]
    Dyninst::RegValue slli_imm3 = 8;
    Dyninst::RegValue addi_imm3 = imm & 0xff;          // imm[7:0]

    // For lui and addi, the immediates are signed extended, so we need to adjust the decomposed immediates
    // according to their corresponding signedness.
    int carry = 0;

    // addi_imm2 does not need adjustment as addi_imm3 is 8 bits wide, meaning that it is 100% a positive value

    // addi_imm1 requires adjustment if the most significant bit of addi_imm2 is 1
    if (addi_imm2 & 0x800) {
        addi_imm1 = (addi_imm1 + carry + 1) & 0xfff;
        carry = (addi_imm1 == 0) ? 1 : 0;
    }

    // addi_imm0 requires adjustment if the most significant bit of addi_imm1 is 1
    if (addi_imm1 & 0x800) {
        addi_imm0 = (addi_imm0 + carry + 1) & 0xfff;
        carry = (addi_imm0 == 0) ? 1 : 0;
    }

    // lui_imm requires adjustment if the most significant bit of addi_imm0 is 1
    // Here, we don't need to worry about the carry and overflow
    // because the register will handle it for you naturally
    if (addi_imm0 & 0x800) {
        lui_imm = (lui_imm + carry + 1) & 0xfff;
    }

    addi_imm1 = 32;
    addi_imm2 = 0;
    addi_imm3 = 0;

    // Optimization: if any of the addi immediates are zero, we can omit them
    // We should also adjust the number of bits to shift accordingly
    if (addi_imm2 == 0) {
        slli_imm2 += slli_imm3;
        slli_imm3 = 0;
    }
    if (addi_imm1 == 0) {
        slli_imm1 += slli_imm2;
        slli_imm2 = 0;
    }

    // lui must be generated
    generateLui(gen, rd, lui_imm, useRVC);

    // If any of the following immediates are zero, there's no point of generating it
    if (addi_imm0 != 0) {
        generateAddi(gen, rd, rd, addi_imm0, useRVC);
    }
    if (slli_imm1 != 0) {
        generateSlli(gen, rd, rd, slli_imm1, useRVC);
    }
    if (addi_imm1 != 0) {
        generateAddi(gen, rd, rd, addi_imm1, useRVC);
    }
    if (slli_imm2 != 0) {
        generateSlli(gen, rd, rd, slli_imm2, useRVC);
    }
    if (addi_imm2 != 0) {
        generateAddi(gen, rd, rd, addi_imm2, useRVC);
    }
    if (slli_imm3 != 0) {
        generateSlli(gen, rd, rd, slli_imm3, useRVC);
    }
    if (addi_imm3 != 0) {
        generateAddi(gen, rd, rd, addi_imm3, useRVC);
    }
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
    assert(imm % 2 == 0);

    if (useRVC) {
        if ((rs1 == GPR_ZERO || rs2 == GPR_ZERO) && imm >= -0x100 && imm < 0x100) {
            if (rs1 != GPR_ZERO && rs2 == GPR_ZERO) {
                generateCBeqz(gen, rs1, imm);
                return true;
            }
            if (rs1 == GPR_ZERO && rs2 != GPR_ZERO) {
                generateCBeqz(gen, rs2, imm);
                return true;
            }
        }
    }
    makeBTypeInsn(gen, rs1, rs2, imm, BEQFunct3, BRANCHOp);
    return false;
}

bool insnCodeGen::generateBne(codeGen &gen,
                              Dyninst::Register rs1,
                              Dyninst::Register rs2,
                              Dyninst::RegValue imm,
                              bool useRVC)
{
    assert(imm % 2 == 0);

    if (useRVC) {
        if ((rs1 == GPR_ZERO || rs2 == GPR_ZERO) && imm >= -0x100 && imm < 0x100) {
            if (rs1 != GPR_ZERO && rs2 == GPR_ZERO) {
                generateCBnez(gen, rs1, imm);
                return true;
            }
            if (rs1 == GPR_ZERO && rs2 != GPR_ZERO) {
                generateCBnez(gen, rs2, imm);
                return true;
            }
        }
    }
    makeBTypeInsn(gen, rs1, rs2, imm, BNEFunct3, BRANCHOp);
    return false;
}

bool insnCodeGen::generateBlt(codeGen &gen,
                                         Dyninst::Register rs1,
                                         Dyninst::Register rs2,
                                         Dyninst::RegValue imm,
                                         bool /*useRVC*/)
{
    assert(imm % 2 == 0);

    // No available RVC optimization
    makeBTypeInsn(gen, rs1, rs2, imm, BLTFunct3, BRANCHOp);
    return false;
}

bool insnCodeGen::generateBge(codeGen &gen,
                              Dyninst::Register rs1,
                              Dyninst::Register rs2,
                              Dyninst::RegValue imm,
                              bool /*useRVC*/)
{
    assert(imm % 2 == 0);

    // No available RVC optimization
    makeBTypeInsn(gen, rs1, rs2, imm, BGEFunct3, BRANCHOp);
    return false;
}

bool insnCodeGen::generateBltu(codeGen &gen,
                               Dyninst::Register rs1,
                               Dyninst::Register rs2,
                               Dyninst::RegValue imm,
                               bool /*useRVC*/)
{
    assert(imm % 2 == 0);

    // No available RVC optimization
    makeBTypeInsn(gen, rs1, rs2, imm, BLTUFunct3, BRANCHOp);
    return false;
}

bool insnCodeGen::generateBgeu(codeGen &gen,
                               Dyninst::Register rs1,
                               Dyninst::Register rs2,
                               Dyninst::RegValue imm,
                               bool /*useRVC*/)
{
    assert(imm % 2 == 0);

    // No available RVC optimization
    makeBTypeInsn(gen, rs1, rs2, imm, BGEUFunct3, BRANCHOp);
    return false;
}

bool insnCodeGen::generateJ(codeGen &gen,
                            Dyninst::RegValue offset,
                            bool useRVC)
{
    assert((offset & 1) == 0);
    assert(offset >= -0x100000 && offset < 0x100000);

    if (useRVC) {
        if (offset >= -0x1000 && offset < 0x1000) {
            // use c.j
            generateCJ(gen, offset);
            return true;
        }
    }
    generateJal(gen, GPR_ZERO, offset, false);
    return false;
}

bool insnCodeGen::generateJal(codeGen &gen,
                              Dyninst::Register rd,
                              Dyninst::RegValue offset,
                              bool useRVC)
{
    assert((offset & 1) == 0);
    assert(offset >= -0x100000 && offset < 0x100000);

    if (useRVC) {
        if (rd == 1 /* the default link register is ra (x1) */ && offset >= -0x1000 && offset < 0x1000) {
            // use c.jal
            generateCJal(gen, offset);
            return true;
        }
    }

    makeJTypeInsn(gen, rd, offset, JALOp);
    return false;
}

bool insnCodeGen::generateJr(codeGen &gen,
                             Dyninst::Register rs,
                             Dyninst::RegValue offset,
                             bool useRVC)
{
    assert(offset >= -0x800 && offset < 0x800);

    if (useRVC) {
        if (offset == 0) {
            // use c.j
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
    assert(offset >= -0x800 && offset < 0x800);

    if (useRVC) {
        if (offset == 0 && rd == GPR_RA) {
            // use c.jal
            generateCJalr(gen, rs);
            return true;
        }
    }

    // JALR is an I-Type instruction
    makeITypeInsn(gen, rd, rs, offset, JALRFunct3, JALROp);
    return false;
}



void insnCodeGen::generateCAdd(codeGen &gen,
                               Dyninst::Register rd,
                               Dyninst::Register rs)
{
    instruction insn;
    insn.setIsRVC(true);
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
    insn.setIsRVC(true);
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
    insn.setIsRVC(true);
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
    insn.setIsRVC(true);

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
    insn.setIsRVC(true);

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
    assert(rd >= 8 && rd < 16 && imm >= -32 && imm < 32);

    instruction insn;
    insn.setIsRVC(true);

    INSN_BUFF_SET(insn, 13, 15, 0x4);            // func3 = 100
    INSN_BUFF_SET(insn, 12, 12, (imm >> 5) & 1); // imm[5]
    INSN_BUFF_SET(insn, 10, 11, 0x10);           // 10
    INSN_BUFF_SET(insn, 7, 9, rd);               // rd
    INSN_BUFF_SET(insn, 2, 6, imm & 0x1f);       // imm[4:0]
    INSN_BUFF_SET(insn, 0, 1, 0x1);               // opcode = 01

    insnCodeGen::generate(gen, insn);
}

void insnCodeGen::generateCLoadImm(codeGen &gen,
                                   Dyninst::Register rd,
                                   Dyninst::RegValue imm)
{
    instruction insn;
    insn.setIsRVC(true);
    INSN_BUFF_SET(insn, 13, 15, 0x2);            // func3 = 010
    INSN_BUFF_SET(insn, 12, 12, (imm >> 5) & 1); // imm[5]
    INSN_BUFF_SET(insn, 7, 11, rd);              // rd
    INSN_BUFF_SET(insn, 2, 6, imm & 0x1f);       // imm[4:0]
    INSN_BUFF_SET(insn, 0, 1, 0x1);              // opcode = 01
    insnCodeGen::generate(gen, insn);
}

void insnCodeGen::generateCLoadUpperImm(codeGen &gen,
                                        Dyninst::Register rd,
                                        Dyninst::RegValue imm)
{
    instruction insn;
    insn.setIsRVC(true);

    INSN_BUFF_SET(insn, 13, 15, 0x3);            // func3 = 011
    INSN_BUFF_SET(insn, 12, 12, (imm >> 5) & 1); // imm[5]
    INSN_BUFF_SET(insn, 7, 11, rd);              // rd
    INSN_BUFF_SET(insn, 2, 6, imm & 0x1f);       // imm[4:0]
    INSN_BUFF_SET(insn, 0, 1, 0x1);              // opcode = 01

    insnCodeGen::generate(gen, insn);
}

void insnCodeGen::generateCSlli(codeGen &gen, 
                                Dyninst::Register rd,
                                Dyninst::RegValue uimm)
{
    assert(uimm >= 0 && uimm < 64);

    instruction insn;
    insn.setIsRVC(true);

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
    assert(rd >= 8 && rd < 16 && uimm >= 0 && uimm < 64);

    instruction insn;
    insn.setIsRVC(true);

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
    assert(rd >= 8 && rd < 16 && uimm >= 0 && uimm < 64);

    instruction insn;
    insn.setIsRVC(true);

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
    insn.setIsRVC(true);

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
    insn.setIsRVC(true);

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
    assert((offset & 1) == 0 && offset >= -0x1000 && offset < 0x1000);

    instruction insn;
    insn.setIsRVC(true);

    INSN_BUFF_SET(insn, 13, 15, 0x5);                  // func3 = 101
    INSN_BUFF_SET(insn, 12, 12, (offset >> 11) & 0x1); // imm[11]
    INSN_BUFF_SET(insn, 11, 11, (offset >> 4) & 0x1);  // imm[4]
    INSN_BUFF_SET(insn, 9, 10, (offset >> 8) & 0x3);   // imm[9:8]
    INSN_BUFF_SET(insn, 8, 8, (offset >> 10) & 0x1);   // imm[10]
    INSN_BUFF_SET(insn, 7, 7, (offset >> 6) & 0x1);    // imm[6]
    INSN_BUFF_SET(insn, 6, 6, (offset >> 7) & 0x1);    // imm[7]
    INSN_BUFF_SET(insn, 3, 5, (offset >> 1) & 0x7);    // imm[3:1]
    INSN_BUFF_SET(insn, 2, 2, (offset >> 5) & 0x1);    // imm[5]
    INSN_BUFF_SET(insn, 0, 1, 0x1);                    // opcode = 01

    insnCodeGen::generate(gen, insn);
}

void insnCodeGen::generateCJal(codeGen &gen,
                               Dyninst::RegValue offset)
{
    assert((offset & 1) == 0 && offset >= -0x1000 && offset < 0x1000);

    instruction insn;
    insn.setIsRVC(true);

    INSN_BUFF_SET(insn, 12, 12, (offset >> 11) & 0x1); // imm[11]
    INSN_BUFF_SET(insn, 11, 11, (offset >> 4) & 0x1);  // imm[4]
    INSN_BUFF_SET(insn, 9, 10, (offset >> 8) & 0x3);   // imm[9:8]
    INSN_BUFF_SET(insn, 8, 8, (offset >> 10) & 0x1);   // imm[10]
    INSN_BUFF_SET(insn, 7, 7, (offset >> 6) & 0x1);    // imm[6]
    INSN_BUFF_SET(insn, 6, 6, (offset >> 7) & 0x1);    // imm[7]
    INSN_BUFF_SET(insn, 3, 5, (offset >> 1) & 0x7);    // imm[3:1]
    INSN_BUFF_SET(insn, 2, 2, (offset >> 5) & 0x1);    // imm[5]
    INSN_BUFF_SET(insn, 0, 1, 0x1);                    // opcode = 01

    insnCodeGen::generate(gen, insn);
}

void insnCodeGen::generateCJr(codeGen &gen,
                              Dyninst::Register rs)
{
    instruction insn;
    insn.setIsRVC(true);

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
    insn.setIsRVC(true);

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
    insn.setIsRVC(true);

    INSN_BUFF_SET(insn, 13, 15, 0x6);                 // func3 = 110
    INSN_BUFF_SET(insn, 12, 12, (offset >> 8) & 0x1); // offset[8]
    INSN_BUFF_SET(insn, 10, 11, (offset >> 3) & 0x3); // offset[4:3]
    INSN_BUFF_SET(insn, 7, 9, rs);                    // rs
    INSN_BUFF_SET(insn, 5, 6, (offset >> 6) & 0x3);   // offset[7:6]
    INSN_BUFF_SET(insn, 3, 4, (offset >> 1) & 0x3);   // offset[2:1]
    INSN_BUFF_SET(insn, 2, 2, (offset >> 5) & 0x1);   // offset[5]
    INSN_BUFF_SET(insn, 0, 1, 0x1);                   // 01

    insnCodeGen::generate(gen, insn);
}

void insnCodeGen::generateCBnez(codeGen &gen,
                                Dyninst::Register rs,
                                Dyninst::RegValue offset) {
    instruction insn;
    insn.setIsRVC(true);

    INSN_BUFF_SET(insn, 13, 15, 0x7);                 // func3 = 111
    INSN_BUFF_SET(insn, 12, 12, (offset >> 8) & 0x1); // offset[8]
    INSN_BUFF_SET(insn, 10, 11, (offset >> 3) & 0x3); // offset[4:3]
    INSN_BUFF_SET(insn, 7, 9, rs);                    // rs
    INSN_BUFF_SET(insn, 5, 6, (offset >> 6) & 0x3);   // offset[7:6]
    INSN_BUFF_SET(insn, 3, 4, (offset >> 1) & 0x3);   // offset[2:1]
    INSN_BUFF_SET(insn, 2, 2, (offset >> 5) & 0x1);   // offset[5]
    INSN_BUFF_SET(insn, 0, 1, 0x1);                   // 01

    insnCodeGen::generate(gen, insn);
}

void insnCodeGen::generateCNop(codeGen &gen) {
    instruction insn;
    insn.setIsRVC(true);

    INSN_BUFF_SET(insn, 0, 15, 0x1);

    insnCodeGen::generate(gen, insn);
}

void insnCodeGen::generateCXor(codeGen &gen,
                               Dyninst::Register rd,
                               Dyninst::Register rs)
{
    instruction insn;
    insn.setIsRVC(true);

    INSN_BUFF_SET(insn, 10, 15, 0x23); // 100011
    INSN_BUFF_SET(insn, 7, 9, rd);     // rd
    INSN_BUFF_SET(insn, 5, 6, 0x1);    // 01
    INSN_BUFF_SET(insn, 2, 4, rs);     // rs
    INSN_BUFF_SET(insn, 0, 1, 0x1);    // opcode = 01

    insnCodeGen::generate(gen, insn);
}
