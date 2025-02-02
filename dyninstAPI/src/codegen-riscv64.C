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

// TODO implement RISC-V codegen
// "Casting" methods. We use a "base + offset" model, but often need to
// turn that into "current instruction pointer".
codeBuf_t *insnCodeGen::insnPtr(codeGen &gen) {
    return (instructUnion *)gen.cur_ptr();
}

#if 0
// Same as above, but increment offset to point at the next insn.
codeBuf_t *insnCodeGen::ptrAndInc(codeGen &gen) {
  // MAKE SURE THAT ret WILL STAY VALID!
  gen.realloc(gen.used() + sizeof(instruction));

  instructUnion *ret = insnPtr(gen);
  gen.moveIndex(instruction::size());
  return ret;
}
#endif

void insnCodeGen::generate(codeGen &gen, instruction &insn) {
    unsigned raw, size;
    if (insn.isCompressed()) {
        raw = insn.asShort();
        size = 2;
    } else {
        raw = insn.asInt();
        size = 4;
    }
    gen.copy(&raw, size);
}

void insnCodeGen::generate(codeGen &gen, instruction &insn, unsigned position) {
    unsigned raw, size;
    if (insn.isCompressed()) {
        raw = insn.asShort();
        size = 2;
    } else {
        raw = insn.asInt();
        size = 4;
    }
    gen.insert(&raw, size, position);
}

void insnCodeGen::generateIllegal(codeGen &gen) { // instP.h
    instruction insn;
    generate(gen,insn);
}

void insnCodeGen::generateTrap(codeGen &gen) {
    instruction insn(BREAK_POINT_INSN);
    generate(gen,insn);
}

void insnCodeGen::generateJump(codeGen &gen, Dyninst::RegValue offset) {
    assert(offset & 1 == 0);
    assert(offset >= -0x100000 && offset < 0x100000);

    if (offset >= -4096 && offset < 4096) {
        // use c.j
        generateCJump(gen, offset);
        return;
    }
    generateJumpAndLink(gen, 0, offset);
}

void insnCodeGen::generateJumpAndLink(codeGen &gen, Dyninst::Register rd, Dyninst::RegValue offset) {
    assert(offset & 1 == 0);
    assert(offset >= -0x100000 && offset < 0x100000);

    if (rd == 1 /* the default link register is ra (x1) */ && offset >= -4096 && offset < 4096) {
        // use c.jal
        generateCJumpAndLink(gen, offset);
        return;
    }

    instruction insn{};
    INSN_SET(insn, 31, 31, (offset & 0x100000) >> 20); // offset[20]
    INSN_SET(insn, 30, 21, (offset & 0x7fe) >> 1);     // offset[10:1]
    INSN_SET(insn, 20, 20, (offset & 0x800) >> 11);    // offset[11]
    INSN_SET(insn, 19, 12, (offset & 0xff000) >> 12);  // offset[19:12]
    INSN_SET(insn, 11, 7, rd);                         // rd
    INSN_SET(insn, 6, 0, 0x6f);                        // 1101111
    insnCodeGen::generate(gen, insn);
}

void insnCodeGen::generateJumpRegister(codeGen &gen, Dyninst::Register rs, Dyninst::RegValue offset) {
    assert(offset >= -2048 && offset < 2048);

    if (offset == 0) {
        // use c.j
        generateCJumpRegister(gen, rs);
        return;
    }
    generateJumpAndLinkRegister(gen, 0, rs, offset);
}

void insnCodeGen::generateJumpAndLinkRegister(codeGen &gen, Dyninst::Register rd, Dyninst::Register rs, Dyninst::RegValue offset) {
    assert(offset >= -2048 && offset < 2048);

    if (offset == 0 && rd == 1) {
        // use c.jal
        generateCJumpAndLinkRegister(gen, rs);
        return;
    }

    instruction insn{};
    INSN_SET(insn, 31, 20, offset); // offset
    INSN_SET(insn, 19, 15, rs);     // rs
    INSN_SET(insn, 14, 12, 0x0);    // 000
    INSN_SET(insn, 11, 7, rd);      // rd
    INSN_SET(insn, 6, 0, 0x67);     // 1100111
    insnCodeGen::generate(gen, insn);
}

void insnCodeGen::generateCall(codeGen &gen, Dyninst::Address from, Dyninst::Address to) {
    generateBranch(gen, from, to, true);
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
        loadImmIntoReg(gen, scratch, to);
        // generate jalr
        generateJumpAndLinkRegister(gen, scratch, scratch, 0);
        return;
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

    loadImmIntoReg(gen, scratch, to);
    generateJumpRegister(gen, scratch, 0);
}

void insnCodeGen::generateBranch(codeGen &gen, long disp, bool link) {
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
        generateJumpAndLink(gen, GPR_RA, disp);
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
        generateJump(gen, disp);
    }
}

void insnCodeGen::generateBranch(codeGen &gen, Dyninst::Address from, Dyninst::Address to, bool link) {
    long disp = (to - from);
    if (labs(disp) > MAX_BRANCH_OFFSET) {
        generateLongBranch(gen, from, to, link);
    }
    else {
        generateBranch(gen, disp, link);
    }
}

void insnCodeGen::generateBranchViaTrap(codeGen &gen, Dyninst::Address from, Dyninst::Address to, bool isCall) {
    // TODO
}

void insnCodeGen::generateConditionalBranch(codeGen& gen, Dyninst::Address to, unsigned opcode, bool s)
{
    // TODO
}


void insnCodeGen::generateMul(codeGen &gen, Dyninst::Register rm, Dyninst::Register rn, Dyninst::Register rd, bool is64bit) {
    // TODO
}

//#sasha is rm or rn the denominator?
void insnCodeGen::generateDiv(
        codeGen &gen, Dyninst::Register rm, Dyninst::Register rn, Dyninst::Register rd, bool is64bit, bool s)
{
    // TODO
}

void insnCodeGen::generateBitwiseOpShifted(
        codeGen &gen, insnCodeGen::BitwiseOp op, int shift, Dyninst::Register rm, int imm6,
        Dyninst::Register rn, Dyninst::Register rd, bool is64bit)
{
    // TODO
}

void insnCodeGen::generateLoadReg(codeGen &, Dyninst::Register,
                                  Dyninst::Register, Dyninst::Register)
{
    // TODO
}

void insnCodeGen::generateStoreReg(codeGen &, Dyninst::Register,
                                   Dyninst::Register, Dyninst::Register)
{
    // TODO
}

void insnCodeGen::generateLoadReg64(codeGen &, Dyninst::Register,
                                    Dyninst::Register, Dyninst::Register)
{
    // TODO
}

void insnCodeGen::generateStoreReg64(codeGen &, Dyninst::Register,
                                     Dyninst::Register, Dyninst::Register)
{
    // TODO
}

void insnCodeGen::generateMove(codeGen &gen, int imm16, int shift, Dyninst::Register rd, MoveOp movOp)
{
    // TODO
}

void insnCodeGen::generateMove(
        codeGen &gen, Dyninst::Register rd, Dyninst::Register rm, bool is64bit)
{
    // TODO
}

void insnCodeGen::generateMoveSP(codeGen &gen, Dyninst::Register rn, Dyninst::Register rd, bool is64bit) {
    // TODO
}


Dyninst::Register insnCodeGen::moveValueToReg(codeGen &gen, long int val, std::vector<Dyninst::Register> *exclude) {
    // TODO
}

// Generate memory access through Load or Store
// Instructions generated:
//     LDR/STR (immediate) for 32-bit or 64-bit
//     LDRB/STRB (immediate) for 8-bit
//     LDRH/STRH  (immediate) for 16-bit
//
// Encoding classes allowed: Post-index, Pre-index and Unsigned Offset
void insnCodeGen::generateMemLoad(codeGen &gen, LoadStore accType,
        Dyninst::Register rd, Dyninst::Register rs, Dyninst::RegValue offset, Dyninst::RegValue size, bool isUnsigned)
{
    assert(size == 1 || size == 2 || size == 4 || size == 8);
    assert(!(size == 8 && isUnsigned)); // no ldu instruction
    assert(offset >= -2048 && offset < 2048);

    instruction insn{};

    Dyninst::RegValue memop{};
    switch (size) {
        case 1: memop = 0x0; break; // lb = 000
        case 2: memop = 0x1; break; // lh = 001
        case 4: memop = 0x2; break; // lw = 010
        case 8: memop = 0x4; break; // ld = 011
        default: break;             // not gonna happen
    }
    if (isUnsigned) {
        memop |= 0x4; // lbu = 100, lhu = 101, lwu = 110
    }

    INSN_SET(insn, 31, 20, offset); // offset
    INSN_SET(insn, 19, 15, rs);     // rs
    INSN_SET(insn, 14, 12, memop);  // memop
    INSN_SET(insn, 11, 7, rd);      // rd
    INSN_SET(insn, 6, 0, 0x3);      // 0000011

    insnCodeGen::generate(gen, insn);
}

void insnCodeGen::generateMemStore(codeGen &gen, LoadStore accType,
        Dyninst::Register rd, Dyninst::Register rs, Dyninst::RegValue offset, Dyninst::RegValue size)
{
    assert(size == 1 || size == 2 || size == 4 || size == 8);
    assert(offset >= -2048 && offset < 2048);

    instruction insn{};

    Dyninst::RegValue memop{};
    switch (size) {
        case 1: memop = 0x0; break; // lb = 000
        case 2: memop = 0x1; break; // lh = 001
        case 4: memop = 0x2; break; // lw = 010
        case 8: memop = 0x4; break; // ld = 011
        default: break;             // not gonna happen
    }

    INSN_SET(insn, 31, 25, (offset & 0xfe0) >> 5); // offset[11:5]
    INSN_SET(insn, 24, 20, rs);                    // rs
    INSN_SET(insn, 19, 15, rd);                    // rd
    INSN_SET(insn, 14, 12, 0x2);                   // 010
    INSN_SET(insn, 11, 7, (offset & 0x1f));        // offset[4:0]
    INSN_SET(insn, 6, 0, 0x23);                    // 0100011

    insnCodeGen::generate(gen, insn);
}

//
// generate an instruction that does nothing and has to side affect except to
//   advance the program counter.
//


void insnCodeGen::saveRegister(codeGen &gen, Dyninst::Register r, int sp_offset)
{
    // TODO
}


void insnCodeGen::restoreRegister(codeGen &gen, Dyninst::Register r, int sp_offset)
{
    // TODO
}


// Helper method.  Fills register with partial value to be completed
// by an operation with a 16-bit signed immediate.  Such as loads and
// stores.
void insnCodeGen::loadPartialImmIntoReg(codeGen &, Dyninst::Register, long)
{
    // TODO
}

int insnCodeGen::createStackFrame(codeGen &, int, std::vector<Dyninst::Register>& freeReg, std::vector<Dyninst::Register>&){
    // TODO
}

void insnCodeGen::removeStackFrame(codeGen &) {
    // TODO
}

bool insnCodeGen::generateMem(codeGen &,
                              instruction&,
                              Dyninst::Address,
                              Dyninst::Address,
                              Dyninst::Register,
                  Dyninst::Register) {
    // TODO
}

void insnCodeGen::generateMoveFromLR(codeGen &, Dyninst::Register) {
    // TODO
}

void insnCodeGen::generateMoveToLR(codeGen &, Dyninst::Register) {
    // TODO
}
void insnCodeGen::generateMoveToCR(codeGen &, Dyninst::Register) {
    // TODO
}

bool insnCodeGen::modifyJump(Dyninst::Address target,
                             NS_riscv64::instruction &insn,
                             codeGen &gen) {
    // TODO
    return true;
}

/* TODO and/or FIXME
 * The logic used by this function is common across architectures but is replicated 
 * in architecture-specific manner in all codegen-* files.
 * This means that the logic itself needs to be refactored into the (platform 
 * independent) codegen.C file. Appropriate architecture-specific,
 * bit-twiddling functions can then be defined if necessary in the codegen-* files 
 * and called as necessary by the common, refactored logic.
*/
bool insnCodeGen::modifyJcc(Dyninst::Address target,
			    NS_riscv64::instruction &insn,
			    codeGen &gen) {
    // TODO
    return true;
}

bool insnCodeGen::modifyCall(Dyninst::Address target,
                             NS_riscv64::instruction &insn,
                             codeGen &gen) {
    if (insn.isUncondBranch())
        return modifyJump(target, insn, gen);
    else
        return modifyJcc(target, insn, gen);
}

bool insnCodeGen::modifyData(Dyninst::Address target,
        NS_riscv64::instruction &insn,
        codeGen &gen) 
{
    // TODO
    return true;
}

// Basic RISC-V instruction type generation

// U-type instruction

void insnCodeGen::generateUpperImmInsn(codeGen &gen, Dyninst::Register rd, Dyninst::Register rs, Dyninst::RegValue imm, int immop) {
    assert(imm >= -2048 && imm < 2048);

    instruction insn{};
    INSN_SET(insn, 31, 20, imm);   // imm
    INSN_SET(insn, 19, 15, rs);    // rs
    INSN_SET(insn, 14, 12, immop); // 111
    INSN_SET(insn, 11, 7, rd);     // rd
    INSN_SET(insn, 6, 0, 0x13);    // opcode = 0010011
    insnCodeGen::generate(gen, insn);
}

// I-type instruction

void insnCodeGen::generateImmInsn(codeGen &gen, Dyninst::Register rd, Dyninst::Register rs, Dyninst::RegValue imm, int immop) {
    assert(imm >= -2048 && imm < 2048);

    instruction insn{};
    INSN_SET(insn, 31, 20, imm);   // imm
    INSN_SET(insn, 19, 15, rs);    // rs
    INSN_SET(insn, 14, 12, immop); // 111
    INSN_SET(insn, 11, 7, rd);     // rd
    INSN_SET(insn, 6, 0, 0x13);    // opcode = 0010011
    insnCodeGen::generate(gen, insn);
}


void insnCodeGen::generateAddImm(codeGen &gen, Dyninst::Register rd, Dyninst::Register rs, Dyninst::RegValue imm) {
    assert(imm < 0x800 && imm >= -0x800);

    // If rd == rs == zero && imm == 0, the instruction is essentially NOP (c.nop)
    if (rd == 0 && rs == 0 && imm == 0) {
        generateNOOP(gen);
        return;
    }

    // If imm is 6 bits wide (-32 <= imm < 32) and rs == zero
    // we use the c.li instruction
    if (imm >= -0x20 && imm < 0x20 && rs == 0) {
        generateCLoadImm(gen, rd, imm);
        return;
    }

    // If imm == 0, we use the c.mv instruction
    if (imm == 0) {
        generateCMove(gen, rd, rs);
        return;
    }

    // If rs == sp && x8 <= rd < x16 && 0 <= imm < 1024 && imm % 4 == 0
    // we use c.addi4spn
    if (rs == GPR_SP && rd >= GPR_FP && rd < 16 && imm >= 0 && imm < 1024 && imm % 4 == 0) {
        generateCAddImmScale4SPn(gen, rd, imm >> 2);
        return;
    }

    // If rd == rs == sp && -512 <= imm < 512 && imm % 16 == 0
    // we use c.addi16sp
    if (rs == GPR_SP && rd >= GPR_FP && rd < 16 && imm >= 0 && imm < 1024 && imm % 4 == 0) {
        generateCAddImmScale16SP(gen, imm >> 4);
        return;
    }

    // If imm is 6 bits wide (-32 <= imm < 32) and imm != 0 and rd != zero
    // we use the c.addi instruction
    if (imm >= -0x20 && imm < 0x20) {
        generateCAddImm(gen, rd, imm);
        return;
    }

    // Otherwise, generate addi
    generateImmInsn(gen, rd, rs, imm, ADDImmOp);
}

void insnCodeGen::generateShiftLeftImm(codeGen &gen, Dyninst::Register rd, Dyninst::Register rs, Dyninst::RegValue imm) {
    assert(imm >= 0 && imm < 64);

    // If rd == rs, use c.slli
    if (rd == rs) {
        generateCShiftLeftImm(gen, rd, imm);
        return;
    }
    generateImmInsn(gen, rd, rs, imm, SLLImmOp);
}

void insnCodeGen::generateShiftRightLogicallyImm(codeGen &gen, Dyninst::Register rd, Dyninst::Register rs, Dyninst::RegValue imm) {
    assert(imm >= 0 && imm < 64);

    // If rd == rs, use c.srli
    if (rd == rs && rd >= 8 && rd < 16) {
        generateCShiftRightLogicallyImm(gen, rd, imm);
        return;
    }
    generateImmInsn(gen, rd, rs, imm, SRLImmOp);
}

void insnCodeGen::generateShiftRightArithmeticImm(codeGen &gen, Dyninst::Register rd, Dyninst::Register rs, Dyninst::RegValue imm) {
    // If rd == rs, use c.srai
    if (rd == rs && rd >= 8 && rd < 16) {
        generateCShiftRightArithmeticImm(gen, rd, imm);
        return;
    }
    imm |= 0x400;
    generateImmInsn(gen, rd, rs, imm, SRAImmOp);
}
void insnCodeGen::generateAndImm(codeGen &gen, Dyninst::Register rd, Dyninst::Register rs, Dyninst::RegValue imm) {
    assert(imm >= -2048 && imm < 2048);
    // If rd == rs, use c.andi
    if (rd == rs && rd >= 8 && rd < 16) {
        generateCAndImm(gen, rd, imm);
        return;
    }
    generateImmInsn(gen, rd, rs, imm, ANDImmOp);
}

void insnCodeGen::generateOrImm(codeGen &gen, Dyninst::Register rd, Dyninst::Register rs, Dyninst::RegValue imm) {
    assert(imm >= -2048 && imm < 2048);
    generateImmInsn(gen, rd, rs, imm, ORImmOp);
}

void insnCodeGen::generateXorImm(codeGen &gen, Dyninst::Register rd, Dyninst::Register rs, Dyninst::RegValue imm) {
    assert(imm >= -2048 && imm < 2048);
    generateImmInsn(gen, rd, rs, imm, XORImmOp);
}

void insnCodeGen::generateLoadUpperImm(codeGen &gen, Dyninst::Register rd, Dyninst::RegValue imm) {
    // If imm is 6 bits wide (-32 <= imm < 32), we use the c.lui instruction
    if (imm >= -0x20 && imm < 0x20) {
        generateCLoadUpperImm(gen, rd, imm);
        return;
    }

    // Otherwise, generate lui
    instruction insn;
    INSN_SET(insn, 31, 12, imm); // imm[31:12]
    INSN_SET(insn, 11, 7, rd);   // rd
    INSN_SET(insn, 6, 2, 0xdu);  // opcode[6:2] = 01101
    INSN_SET(insn, 1, 0, 0x3u);  // opcode[1:0] = 11
    insnCodeGen::generate(gen, insn);
}

void insnCodeGen::generateLoadImm(codeGen &gen, Dyninst::Register rd, Dyninst::Register rs, Dyninst::RegValue imm) {
    // If imm is 6 bits wide (-32 <= imm < 32), we use the c.li instruction
    if (imm >= -0x20 && imm < 0x20) {
        generateCLoadImm(gen, rd, imm);
        return;
    }

    // If imm is 12 bits wide (-2048 <= imm < 2048), we use the addi instruction
    if (imm >= -0x800 && imm < 0x800) {
        generateAddImm(gen, rd, 0, imm);
        return;
    }

    // If imm is larger than 12 bits but less than 32 bits,
    // imm must be loaded in two steps using lui and addi
    if (imm >= -0x80000000LL && imm < 0x80000000LL) {
        Dyninst::RegValue lui_imm = (imm & 0xfffff000) >> 12;
        Dyninst::RegValue addi_imm = imm & 0xfff;
        // If the most significant bit of addi_imm is 1 (addi_imm is negative), we should add 1 to lui_imm
        if (addi_imm & 0x800) {
            lui_imm = (lui_imm + 1) & 0xfffff;
        }
        generateLoadUpperImm(gen, rd, lui_imm);
        generateAddImm(gen, rd, rd, addi_imm);
        return;
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
    Dyninst::RegValue lui_imm = (imm & 0xfffff00000000000) >> 44;
    Dyninst::RegValue addi_imm0 = (imm & 0xfff00000000) >> 32;
    // Lower 32 bits
    Dyninst::RegValue slli_imm1 = 12;
    Dyninst::RegValue addi_imm1 = (imm & 0xfff00000) >> 20;
    Dyninst::RegValue slli_imm2 = 12;
    Dyninst::RegValue addi_imm2 = (imm & 0xfff00) >> 8;
    Dyninst::RegValue slli_imm3 = 8;
    Dyninst::RegValue addi_imm3 = (imm & 0xff);

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
    generateLoadUpperImm(gen, rd, lui_imm);

    // If any of the following immediates are zero, there's no point of generating it
    if (addi_imm0 != 0) {
        generateAddImm(gen, rd, rd, addi_imm0);
    }
    if (slli_imm1 != 0) {
        generateShiftLeftImm(gen, rd, rd, slli_imm1);
    }
    if (addi_imm1 != 0) {
        generateAddImm(gen, rd, rd, addi_imm1);
    }
    if (slli_imm2 != 0) {
        generateShiftLeftImm(gen, rd, rd, slli_imm2);
    }
    if (addi_imm2 != 0) {
        generateAddImm(gen, rd, rd, addi_imm2);
    }
    if (slli_imm3 != 0) {
        generateShiftLeftImm(gen, rd, rd, slli_imm3);
    }
    if (addi_imm3 != 0) {
        generateAddImm(gen, rd, rd, addi_imm3);
    }
}

void insnCodeGen::generateCAddImm(codeGen &gen, Dyninst::Register rd, Dyninst::RegValue imm) {
    instruction insn;
    INSN_C_SET(insn, 15, 13, 0x0);        // func3 = 000
    INSN_C_SET(insn, 12, 12, imm & 0x20); // imm[5] != 0
    INSN_C_SET(insn, 11, 7, rd);          // rsi/rd != 0
    INSN_C_SET(insn, 6, 2, imm & 0x1f);   // imm[4:0] != 0
    INSN_C_SET(insn, 1, 0, 0x1);          // opcode = 01
    insnCodeGen::generate(gen, insn);
}

void insnCodeGen::generateCAddImmScale4SPn(codeGen &gen, Dyninst::Register rd, Dyninst::RegValue imm) {
    instruction insn;
    INSN_C_SET(insn, 15, 13, 0x0);              // func3 = 000
    INSN_C_SET(insn, 12, 11, (imm & 0xc) >> 2); // imm[3:2]
    INSN_C_SET(insn, 10, 7, (imm & 0xf0) >> 4); // imm[7:4]
    INSN_C_SET(insn, 6, 6, (imm & 0x1));        // imm[0]
    INSN_C_SET(insn, 5, 5, (imm & 0x2) >> 1);   // imm[1]
    INSN_C_SET(insn, 4, 2, rd);                 // rd
    INSN_C_SET(insn, 1, 0, 0x0);                // opcode = 00
    insnCodeGen::generate(gen, insn);
}

void insnCodeGen::generateCAddImmScale16SP(codeGen &gen, Dyninst::RegValue imm) {
    instruction insn;
    INSN_C_SET(insn, 15, 13, 0x3);               // func3 = 011
    INSN_C_SET(insn, 12, 12, (imm & 0x20) >> 5); // imm[5]
    INSN_C_SET(insn, 11, 7, 0x2);                // 00010
    INSN_C_SET(insn, 6, 6, (imm & 0x1));         // imm[0]
    INSN_C_SET(insn, 5, 5, (imm & 0x4) >> 2);    // imm[2]
    INSN_C_SET(insn, 4, 3, (imm & 0x18) >> 3);   // imm[4:3]
    INSN_C_SET(insn, 2, 2, (imm & 0x2) >> 1);    // imm[1]
    INSN_C_SET(insn, 1, 0, 0x1);                 // opcode = 01
    insnCodeGen::generate(gen, insn);
}

void insnCodeGen::generateCLoadImm(codeGen &gen, Dyninst::Register rd, Dyninst::RegValue imm) {
    instruction insn;
    INSN_C_SET(insn, 15, 13, 0x2);        // func3 = 010
    INSN_C_SET(insn, 12, 12, imm & 0x20); // imm[5]
    INSN_C_SET(insn, 11, 7, rd);          // rd
    INSN_C_SET(insn, 6, 2, imm & 0x1f);   // imm[4:0]
    INSN_C_SET(insn, 1, 0, 0x1);          // opcode = 01
    insnCodeGen::generate(gen, insn);
}

void insnCodeGen::generateCLoadUpperImm(codeGen &gen, Dyninst::Register rd, Dyninst::RegValue imm) {
    instruction insn;
    INSN_C_SET(insn, 15, 13, 0x3);        // func3 = 011
    INSN_C_SET(insn, 12, 12, imm & 0x20); // imm[5]
    INSN_C_SET(insn, 11, 7, rd);          // rd
    INSN_C_SET(insn, 6, 2, imm & 0x1f);   // imm[4:0]
    INSN_C_SET(insn, 1, 0, 0x1);          // opcode = 01
    insnCodeGen::generate(gen, insn);
}

void insnCodeGen::generateCMove(codeGen &gen, Dyninst::Register rd, Dyninst::Register rs) {
    instruction insn;
    INSN_C_SET(insn, 15, 13, 0x4); // func3 = 100
    INSN_C_SET(insn, 12, 12, 0x0); // 0
    INSN_C_SET(insn, 11, 7, rd);   // rd
    INSN_C_SET(insn, 6, 2, rs);    // rs
    INSN_C_SET(insn, 1, 0, 0x2);   // opcode = 10
    insnCodeGen::generate(gen, insn);
}

void insnCodeGen::generateCShiftLeftImm(codeGen &gen, Dyninst::Register rd, Dyninst::RegValue uimm) {
    assert(uimm >= 0 && uimm < 64);

    instruction insn;
    INSN_C_SET(insn, 15, 13, 0x2);                // func3 = 010
    INSN_C_SET(insn, 12, 12, (uimm & 0x20) >> 5); // uimm[5]
    INSN_C_SET(insn, 11, 7, rd);                  // rd
    INSN_C_SET(insn, 6, 2, uimm & 0x1f);          // uimm[4:0]
    INSN_C_SET(insn, 1, 0, 0x2);                  // opcode = 10
    insnCodeGen::generate(gen, insn);
}

void insnCodeGen::generateCShiftRightLogicallyImm(codeGen &gen, Dyninst::Register rd, Dyninst::RegValue uimm) {
    assert(rd >= 8 && rd < 16 && uimm >= 0 && uimm < 64);

    instruction insn;
    INSN_C_SET(insn, 15, 13, 0x4);                // func3 = 100
    INSN_C_SET(insn, 12, 12, (uimm & 0x20) >> 5); // uimm[5]
    INSN_C_SET(insn, 11, 10, 0x0);                // 00
    INSN_C_SET(insn, 9, 7, rd);                   // rd
    INSN_C_SET(insn, 6, 2, uimm & 0x1f);          // uimm[4:0]
    INSN_C_SET(insn, 1, 0, 0x1);                  // opcode = 01
    insnCodeGen::generate(gen, insn);
}

void insnCodeGen::generateCShiftRightArithmeticImm(codeGen &gen, Dyninst::Register rd, Dyninst::RegValue uimm) {
    assert(rd >= 8 && rd < 16 && uimm >= 0 && uimm < 64);

    instruction insn;
    INSN_C_SET(insn, 15, 13, 0x4);                // func3 = 100
    INSN_C_SET(insn, 12, 12, (uimm & 0x20) >> 5); // uimm[5]
    INSN_C_SET(insn, 11, 10, 0x1);                // 01
    INSN_C_SET(insn, 9, 7, rd);                   // rd
    INSN_C_SET(insn, 6, 2, uimm & 0x1f);          // uimm[4:0]
    INSN_C_SET(insn, 1, 0, 0x1);                  // opcode = 01
    insnCodeGen::generate(gen, insn);
}

void insnCodeGen::generateCAndImm(codeGen &gen, Dyninst::Register rd, Dyninst::RegValue imm) {
    assert(rd >= 8 && rd < 16 && imm >= -32 && imm < 32);

    instruction insn;
    INSN_C_SET(insn, 15, 13, 0x4);               // func3 = 100
    INSN_C_SET(insn, 12, 12, (imm & 0x20) >> 5); // imm[5]
    INSN_C_SET(insn, 11, 10, 0x10);              // 10
    INSN_C_SET(insn, 9, 7, rd);                  // rd
    INSN_C_SET(insn, 6, 2, imm & 0x1f);          // imm[4:0]
    INSN_C_SET(insn, 1, 0, 0x1);                 // opcode = 01
    insnCodeGen::generate(gen, insn);
}

void insnCodeGen::generateCJump(codeGen &gen, Dyninst::RegValue offset) {
    assert(offset & 1 == 0 && offset >= -4096 && offset < 4096);

    instruction insn{};
    INSN_C_SET(insn, 15, 13, 0x5);                    // func3 = 101
    INSN_C_SET(insn, 12, 12, (offset & 0x800) >> 11); // imm[11]
    INSN_C_SET(insn, 11, 11, (offset & 0x10) >> 4);   // imm[4]
    INSN_C_SET(insn, 10, 9, (offset & 0x300) >> 8);   // imm[9:8]
    INSN_C_SET(insn, 8, 8, (offset & 0x400) >> 10);   // imm[10]
    INSN_C_SET(insn, 7, 7, (offset & 0x40) >> 6);     // imm[6]
    INSN_C_SET(insn, 6, 6, (offset & 0x80) >> 7);     // imm[7]
    INSN_C_SET(insn, 5, 3, (offset & 0xe) >> 1);      // imm[3:1]
    INSN_C_SET(insn, 2, 2, (offset & 0x20) >> 5);     // imm[5]
    INSN_C_SET(insn, 1, 0, 0x1);                      // opcode = 01
    insnCodeGen::generate(gen, insn);
}

void insnCodeGen::generateCJumpAndLink(codeGen &gen, Dyninst::RegValue offset) {
    assert(offset & 1 == 0 && offset >= -4096 && offset < 4096);

    instruction insn{};
    INSN_C_SET(insn, 15, 13, 0x1);                    // func3 = 001
    INSN_C_SET(insn, 12, 12, (offset & 0x800) >> 11); // imm[11]
    INSN_C_SET(insn, 11, 11, (offset & 0x10) >> 4);   // imm[4]
    INSN_C_SET(insn, 10, 9, (offset & 0x300) >> 8);   // imm[9:8]
    INSN_C_SET(insn, 8, 8, (offset & 0x400) >> 10);   // imm[10]
    INSN_C_SET(insn, 7, 7, (offset & 0x40) >> 6);     // imm[6]
    INSN_C_SET(insn, 6, 6, (offset & 0x80) >> 7);     // imm[7]
    INSN_C_SET(insn, 5, 3, (offset & 0xe) >> 1);      // imm[3:1]
    INSN_C_SET(insn, 2, 2, (offset & 0x20) >> 5);     // imm[5]
    INSN_C_SET(insn, 1, 0, 0x1);                      // opcode = 01
    insnCodeGen::generate(gen, insn);
}

void insnCodeGen::generateCJumpRegister(codeGen &gen, Dyninst::Register rs) {
    instruction insn{};
    INSN_C_SET(insn, 15, 13, 0x4); // func3 = 100
    INSN_C_SET(insn, 12, 12, 0x0); // 0
    INSN_C_SET(insn, 11, 7, rs);   // rs
    INSN_C_SET(insn, 6, 0, 0x2);   // 0000010
    insnCodeGen::generate(gen, insn);
}

void insnCodeGen::generateCJumpAndLinkRegister(codeGen &gen, Dyninst::Register rs) {
    instruction insn{};
    INSN_C_SET(insn, 15, 13, 0x4); // func3 = 100
    INSN_C_SET(insn, 12, 12, 0x1); // 1
    INSN_C_SET(insn, 11, 7, rs);   // rs
    INSN_C_SET(insn, 6, 0, 0x2);   // 0000010
    insnCodeGen::generate(gen, insn);
}

void insnCodeGen::generateCNOOP(codeGen &gen) {
    instruction insn;
    INSN_C_SET(insn, 15, 0, 0x1);
    insnCodeGen::generate(gen, insn);
}

void insnCodeGen::generateNOOP(codeGen &gen) {
    generateCNOOP(gen);
}

