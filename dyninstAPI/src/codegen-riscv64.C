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
        raw = insn.getShort();
        size = 2;
    } else {
        raw = insn.getInt();
        size = 4;
    }
    gen.copy(&raw, size);
}

void insnCodeGen::generate(codeGen &gen, instruction &insn, unsigned position) {
    unsigned raw, size;
    if (insn.isCompressed()) {
        raw = insn.getShort();
        size = 2;
    } else {
        raw = insn.getInt();
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

void insnCodeGen::generateBranch(codeGen &gen, long disp, bool link) {
    // TODO
}

void insnCodeGen::generateBranch(codeGen &gen, Dyninst::Address from, Dyninst::Address to, bool link) {
    // TODO
}

void insnCodeGen::generateCall(codeGen &gen, Dyninst::Address from, Dyninst::Address to) {
    // TODO
}

void insnCodeGen::generateLongBranch(codeGen &gen,
                                     Dyninst::Address from,
                                     Dyninst::Address to,
                                     bool isCall) 
{
    // TODO
}

void insnCodeGen::generateBranchViaTrap(codeGen &gen, Dyninst::Address from, Dyninst::Address to, bool isCall) {
    // TODO
}

void insnCodeGen::generateConditionalBranch(codeGen& gen, Dyninst::Address to, unsigned opcode, bool s)
{
    // TODO
}


void insnCodeGen::generateAddSubShifted(
        codeGen &gen, insnCodeGen::ArithOp op, int shift, int imm6, Dyninst::Register rm,
        Dyninst::Register rn, Dyninst::Register rd, bool is64bit)
{
    // TODO
}

void insnCodeGen::generateAddSubImmediate(
        codeGen &gen, insnCodeGen::ArithOp op, int shift, int imm12, Dyninst::Register rn, Dyninst::Register rd, bool is64bit)
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

void insnCodeGen::generateCMove(codeGen &gen, Dyninst::Register rd, Dyninst::RegValue imm) {

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
void insnCodeGen::generateMemAccess(codeGen &gen, LoadStore accType,
        Dyninst::Register r1, Dyninst::Register r2, int immd, unsigned size, IndexMode im)
{
    // TODO
}

// This is for generating STR/LDR (SIMD&FP) (immediate) for indexing modes of Post, Pre and Offset
void insnCodeGen::generateMemAccessFP(codeGen &gen, LoadStore accType,
        Dyninst::Register rt, Dyninst::Register rn, int immd, int size, bool is128bit, IndexMode im)
{
    // TODO
}

// rlwinm ra,rs,n,0,31-n
void insnCodeGen::generateLShift(codeGen &, Dyninst::Register, int, Dyninst::Register)
{
    // TODO
}

// rlwinm ra,rs,32-n,n,31
void insnCodeGen::generateRShift(codeGen &, Dyninst::Register, int, Dyninst::Register)
{
    // TODO
}

// sld ra, rs, rb
void insnCodeGen::generateLShift64(codeGen &, Dyninst::Register, int, Dyninst::Register)
{
    // TODO
}

// srd ra, rs, rb
void insnCodeGen::generateRShift64(codeGen &, Dyninst::Register, int, Dyninst::Register)
{
    // TODO
}

//
// generate an instruction that does nothing and has to side affect except to
//   advance the program counter.
//
void insnCodeGen::generateNOOP(codeGen &gen, unsigned size) {
    // TODO
}

void insnCodeGen::generateRelOp(codeGen &, int, int, Dyninst::Register,
                                Dyninst::Register, Dyninst::Register)
{
    // TODO
}


void insnCodeGen::saveRegister(codeGen &gen, Dyninst::Register r, int sp_offset, IndexMode im)
{
    // TODO
}


void insnCodeGen::restoreRegister(codeGen &gen, Dyninst::Register r, int sp_offset, IndexMode im)
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

void insnCodeGen::generateAddImm(codeGen &gen, Dyninst::Register rd, Dyninst::Register rs, Dyninst::RegValue imm) {
    // If rd == rs == zero && imm == 0, the instruction is essentially NOP (c.nop)
    if (rd == 0 && rs == 0 && imm == 0) {
        generateNop(gen);
        return;
    }

    // If imm is 6 bits wide (-32 <= value < 32) and rs == zero
    // we use the c.li instruction
    if (value >= -0x20 && value < 0x20 && rs == 0) {
        generateCLoadImm(gen, rd, imm);
    }

    // If imm == 0, we use the c.mv instruction
    if (imm == 0) {
        generateCMove(gen, rd, rs);
    }

    // If rs == sp && x8 <= rd < x16 && 0 <= imm < 1024 && imm % 4 == 0
    // we use c.addi4spn
    if (rs == 2 && rd >= 8 && rd < 16 && imm >= 0 && imm < 1024 && imm % 4 == 0) {
        generateCAddImmScale4SPn(gen, rd, imm >> 2);
    }

    // If rd == rs == sp && -512 <= imm < 512 && imm % 16 == 0
    // we use c.addi16sp
    if (rs == 2 && rd >= 8 && rd < 16 && imm >= 0 && imm < 1024 && imm % 4 == 0) {
        generateCAddImmScale16SP(gen, imm >> 4);
    }

    // If imm is 6 bits wide (-32 <= value < 32) and imm != 0 and rd != zero
    // we use the c.addi instruction
    if (value >= -0x20 && value < 0x20) {
        generateCAddImm(gen, rd, value);
        return;
    }
}
void insnCodeGen::generateShiftImm(codeGen &gen, Dyninst::Register rd, Dyninst::Register rs, Dyninst::RegValue imm) {

}
void insnCodeGen::generateOrImm(codeGen &gen, Dyninst::Register rd, Dyninst::RegValue imm) {

}
void insnCodeGen::generateLoadUpperImm(codeGen &gen, Dyninst::Register rd, Dyninst::RegValue imm) {
    // If imm is 6 bits wide (-32 <= value < 32), we use the c.lui instruction
    if (value >= -0x20 && value < 0x20) {
        generateCLoadUpperImm(gen, rd, value);
        return;
    }
    INSN_SET(31, 12, imm); // imm[31:12]
    INSN_SET(11, 7, rd);   // rd
    INSN_SET(6, 2, 0xdu);  // opcode[6:2] = 01101
    INSN_SET(1, 0, 0x3u);  // opcode[1:0] = 11
    insnCodeGen::generate(gen, insn);
}

void insnCodeGen::generateLoadImm(codeGen &gen, Dyninst::Register rd, Dyninst::Register rs, Dyninst::RegValue imm) {
    // If imm is 6 bits wide (-32 <= value < 32), we use the c.li instruction
    if (value >= -0x20 && value < 0x20) {
        generateCLoadImm(gen, rd, value);
        return;
    }

    // If imm is 12 bits wide (-2048 <= value < 2048), we use the addi instruction
    if (value >= -0x800 && value < 0x800) {
        generateAddImm(gen, rd, 0, value);
        return;
    }

    // If imm is larger than 12 bits but less than 32 bits,
    // imm must be loaded in two steps using lui and addi
    if (value >= -0x80000000 && value < 0x80000000) {
        Dyninst::RegValue lui_imm = (value & 0xfffff000) >> 12;
        Dyninst::RegValue addi_imm = value & 0xfff;
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
    // However, for the sake of simplicity, we will use a much simpler algorithm
    // Assume that we want to perform li t0, 0xdeadbeefcafebabe
    // We break the integer into the following
    //   lui t0, 0xdeadc
    //   addiw t0, t0, -0x111
    //   slli t0, t0, 11
    //   ori t0, t0, 0x657
    //   slli t0, t0, 11
    //   ori t0, t0, 0x7ae
    //   slli t0, t0, 10
    //   ori t0, t0, 0x2be
    // There's of course room for improvement

    // Top 32 bits
    Dyninst::RegValue lui_imm = (value & 0xfffff00000000000) >> 44;
    Dyninst::RegValue addi_imm = (value & 0xfff00000000) >> 32;
    // If the most significant bit of addi_imm is 1 (addi_imm is negative), we should add 1 to lui_imm
    if (addi_imm & 0x800) {
        lui_imm = (lui_imm + 1) & 0xfffff;
    }

    // Bottom 32 bits
    // We cannot use lui again because it will overwrite top 32 bits
    // We instead use a series of slli and ori to construct the bottom 32 bits
    Dyninst::RegValue ori_imm1 = (value & 0xffe00000) >> 21;
    Dyninst::RegValue ori_imm2 = (value & 0x1ffc00) >> 10;
    Dyninst::RegValue ori_imm3 = (value & 0x3ff);

    generateLoadUpperImm(gen, rd, lui_imm);
    generateAddImm(gen, rd, rd, addi_imm);
    generateShiftImm(gen, rd, rd, 11);
    generateOrImm(gen, rd, rd, ori_imm1);
    generateShiftImm(gen, rd, rd, 11);
    generateOrImm(gen, rd, rd, ori_imm2);
    generateShiftImm(gen, rd, rd, 10);
    generateOrImm(gen, rd, rd, ori_imm3);
    
    return;
}

void generateCAddImm(codeGen &gen, Dyninst::Register rd, Dyninst::RegValue imm) {
    INSN_C_SET(15, 13, 0x0);        // func3 = 000
    INSN_C_SET(12, 12, imm & 0x20); // imm[5] != 0
    INSN_C_SET(11, 7, rd);          // rsi/rd != 0
    INSN_C_SET(6, 2, imm & 0x1f);   // imm[4:0] != 0
    INSN_C_SET(1, 0, 0x1);          // opcode = 01
    insnCodeGen::generate(gen, insn);
}

void generateCAddImmScale4SPn(codeGen &gen, Dyninst::Register rd, Dyninst::RegValue imm) {
    INSN_C_SET(15, 13, 0x0);              // func3 = 000
    INSN_C_SET(12, 11, (imm & 0xc) >> 2); // imm[3:2]
    INSN_C_SET(10, 7, (imm & 0xf0) >> 4); // imm[7:4]
    INSN_C_SET(6, 6, (imm & 0x1));        // imm[0]
    INSN_C_SET(5, 5, (imm & 0x2) >> 1);   // imm[1]
    INSN_C_SET(4, 2, rd);                 // rd
    INSN_C_SET(1, 0, 0x0);                // opcode = 00
    insnCodeGen::generate(gen, insn);
}

void generateCAddImmScale16SP(codeGen &gen, Dyninst::RegValue imm) {
    INSN_C_SET(15, 13, 0x3);               // func3 = 011
    INSN_C_SET(12, 12, (imm & 0x20) >> 5); // imm[5]
    INSN_C_SET(11, 7, 0x2);                // 00010
    INSN_C_SET(6, 6, (imm & 0x1));         // imm[0]
    INSN_C_SET(5, 5, (imm & 0x4) >> 2);    // imm[2]
    INSN_C_SET(4, 3, (imm & 0x18) >> 3);   // imm[4:3]
    INSN_C_SET(2, 2, (imm & 0x2) >> 1);    // imm[1]
    INSN_C_SET(1, 0, 0x1);                 // opcode = 01
    insnCodeGen::generate(gen, insn);
}

void generateCLoadImm(codeGen &gen, Dyninst::Register rd, Dyninst::RegValue imm) {
    INSN_C_SET(15, 13, 0x2);        // func3 = 010
    INSN_C_SET(12, 12, imm & 0x20); // imm[5]
    INSN_C_SET(11, 7, rd);          // rd
    INSN_C_SET(6, 2, imm & 0x1f);   // imm[4:0]
    INSN_C_SET(1, 0, 0x1);          // opcode = 01
    insnCodeGen::generate(gen, insn);
}

void generateCLoadUpperImm(codeGen &gen, Dyninst::Register rd, Dyninst::RegValue imm) {
    INSN_C_SET(15, 13, 0x3);        // func3 = 011
    INSN_C_SET(12, 12, imm & 0x20); // imm[5]
    INSN_C_SET(11, 7, rd);          // rd
    INSN_C_SET(6, 2, imm & 0x1f);   // imm[4:0]
    INSN_C_SET(1, 0, 0x1);          // opcode = 01
    insnCodeGen::generate(gen, insn);
}

void generateCMove(codeGen &gen, Dyninst::Register rd, Dyninst::Register rs) {
    INSN_C_SET(15, 13, 0x4); // func3 = 100
    INSN_C_SET(12, 12, 0x0); // 0
    INSN_C_SET(11, 7, rd);   // rd
    INSN_C_SET(6, 2, rs);    // rs
    INSN_C_SET(1, 0, 0x2);   // opcode = 10
    insnCodeGen::generate(gen, insn);
}

void generateCNop(codeGen &gen) {
    INSN_C_SET(15, 0, 0x1);
    insnCodeGen::generate(gen, insn);
}

void generateMove(codeGen &gen, Dyninst::Register rd, Dyninst::Register rs) {
    generateCMove(gen, rd, rs);
}

void generateNop(codeGen &gen) {
    generateCNop(gen);
}

