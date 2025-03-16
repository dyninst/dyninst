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

#ifndef _CODEGEN_RISCV64_H
#define _CODEGEN_RISCV64_H

#include <vector>
#include "dyntypes.h"
#include "common/src/dyn_register.h"

// TODO implement RISC-V codegen

class AddressSpace;

class codeGen;

class insnCodeGen {
public:

    static instructUnion *insnPtr(codeGen &gen);
    //static instructUnion *ptrAndInc(codeGen &gen);

    // All of these write into a buffer
    static void generateTrap(codeGen &gen);

    static void generateIllegal(codeGen &gen);

    static void generateCall(codeGen &gen,
                             Dyninst::Address from,
                             Dyninst::Address to);

    static void generateBranch(codeGen &gen,
                               long jump_off,
                               bool link = false);

    static void generateBranch(codeGen &gen,
                               Dyninst::Address from,
                               Dyninst::Address to,
                               bool link = false);

    static void generateLongBranch(codeGen &gen,
                                   Dyninst::Address from,
                                   Dyninst::Address to,
                                   bool isCall);

    // Using the process trap mapping for a branch
    static void generateBranchViaTrap(codeGen &gen,
                                      Dyninst::Address from,
                                      Dyninst::Address to,
                                      bool isCall);

    // Generate conditional branch
    static void generateBranchEqual(codeGen &gen, Dyninst::Register rs1, Dyninst::Register rs2, Dyninst::RegValue imm);
    static void generateBranchNotEqual(codeGen &gen, Dyninst::Register rs1, Dyninst::Register rs2, Dyninst::RegValue imm);
    static void generateBranchLessThan(codeGen &gen, Dyninst::Register rs1, Dyninst::Register rs2, Dyninst::RegValue imm);
    static void generateBranchGreaterThanEqual(codeGen &gen, Dyninst::Register rs1, Dyninst::Register rs2, Dyninst::RegValue imm);
    static void generateBranchLessThanUnsigned(codeGen &gen, Dyninst::Register rs1, Dyninst::Register rs2, Dyninst::RegValue imm);
    static void generateBranchGreaterThanEqualUnsigned(codeGen &gen, Dyninst::Register rs1, Dyninst::Register rs2, Dyninst::RegValue imm);

    // LDR/STR (immediate)
    static void generateMemLoad(codeGen &gen, Dyninst::Register rd,
            Dyninst::Register rs, Dyninst::RegValue offset, Dyninst::RegValue size, bool isUnsigned);

    static void generateMemStore(codeGen &gen, Dyninst::Register rs1,
            Dyninst::Register rs2, Dyninst::RegValue offset, Dyninst::RegValue size);

    static void generateMemLoadFp(codeGen &gen, Dyninst::Register rd,
            Dyninst::Register rs, Dyninst::RegValue offset, Dyninst::RegValue size);

    static void generateMemStoreFp(codeGen &gen, Dyninst::Register rs1,
            Dyninst::Register rs2, Dyninst::RegValue offset, Dyninst::RegValue size);

    static void saveRegister(codeGen &gen, Dyninst::Register r, int sp_offset);

    static void restoreRegister(codeGen &gen, Dyninst::Register r, int sp_offset);

    /** TODO **/
    static void loadPartialImmIntoReg(codeGen &gen, Dyninst::Register rt,
                                      long value);

    static Dyninst::Register moveValueToReg(codeGen &gen, long int val, std::vector<Dyninst::Register> *exclude = NULL);

    static void loadImmIntoReg(codeGen &gen, Dyninst::Register rd, Dyninst::RegValue value);

    static void generate(codeGen &gen, instruction &insn);

    // Copy instruction at position in codeGen buffer
    static void generate(codeGen &gen, instruction &insn, unsigned position);

    static void write(codeGen &gen, instruction &insn) { generate(gen, insn); }

    static bool generate(codeGen &gen,
                         instruction &insn,
                         AddressSpace *proc,
                         Dyninst::Address origAddr,
                         Dyninst::Address newAddr,
                         patchTarget *fallthroughOverride = NULL,
                         patchTarget *targetOverride = NULL);

    //TODO
    // Routines to create/remove a new stack frame for getting scratch registers
    static int createStackFrame(codeGen &gen, int numRegs, std::vector <Dyninst::Register> &freeReg,
            std::vector <Dyninst::Register> &excludeReg);

    //TODO
    static void removeStackFrame(codeGen &gen);


    static bool modifyJump(Dyninst::Address target,
                           NS_riscv64::instruction &insn,
                           codeGen &gen);

    static bool modifyJcc(Dyninst::Address target,
                          NS_riscv64::instruction &insn,
                          codeGen &gen);

    static bool modifyCall(Dyninst::Address target,
                           NS_riscv64::instruction &insn,
                           codeGen &gen);

    static bool modifyData(Dyninst::Address target,
                           NS_riscv64::instruction &insn,
                           codeGen &gen);

    // Basic RISC-V instruction type generation
    static void generateUTypeInsn(codeGen &gen, Dyninst::Register rd, Dyninst::RegValue imm, unsigned immop);
    static void generateITypeInsn(codeGen &gen, Dyninst::Register rd, Dyninst::Register rs, Dyninst::RegValue imm, unsigned funct3, unsigned opcode);
    static void generateRTypeInsn(codeGen &gen, Dyninst::Register rd, Dyninst::Register rs1, Dyninst::Register rs2, unsigned funct7, unsigned funct3, unsigned opcode);
    static void generateBTypeInsn(codeGen &gen, Dyninst::Register rs1, Dyninst::Register rs2, Dyninst::RegValue imm, unsigned funct3, unsigned opcode);
    static void generateJTypeInsn(codeGen &gen, Dyninst::Register rd, Dyninst::RegValue imm, unsigned opcode);
    static void generateSTypeInsn(codeGen &gen, Dyninst::Register rs1, Dyninst::Register rs2, Dyninst::RegValue imm, unsigned funct3, unsigned opcode);

    static void generateAddImm(codeGen &gen, Dyninst::Register rd, Dyninst::Register rs1, Dyninst::RegValue imm);
    static void generateSubImm(codeGen &gen, Dyninst::Register rd, Dyninst::Register rs1, Dyninst::RegValue imm);
    static void generateShiftLeftImm(codeGen &gen, Dyninst::Register rd, Dyninst::Register rs1, Dyninst::RegValue imm);
    static void generateShiftRightLogicallyImm(codeGen &gen, Dyninst::Register rd, Dyninst::Register rs1, Dyninst::RegValue imm);
    static void generateShiftRightArithmeticImm(codeGen &gen, Dyninst::Register rd, Dyninst::Register rs1, Dyninst::RegValue imm);
    static void generateAndImm(codeGen &gen, Dyninst::Register rd, Dyninst::Register rs1, Dyninst::RegValue imm);
    static void generateOrImm(codeGen &gen, Dyninst::Register rd, Dyninst::Register rs1, Dyninst::RegValue imm);
    static void generateXorImm(codeGen &gen, Dyninst::Register rd, Dyninst::Register rs1, Dyninst::RegValue imm);
    static void generateLoadImm(codeGen &gen, Dyninst::Register rd, Dyninst::RegValue imm);
    static void generateLoadUpperImm(codeGen &gen, Dyninst::Register rd, Dyninst::RegValue imm);
    static void generateAuipc(codeGen &gen, Dyninst::Register rd, Dyninst::RegValue offset);

    static void generateNOOP(codeGen &gen, unsigned size);

    static void generateAdd(codeGen &gen, Dyninst::Register rd, Dyninst::Register rs1, Dyninst::Register rs2);
    static void generateSub(codeGen &gen, Dyninst::Register rd, Dyninst::Register rs1, Dyninst::Register rs2);
    static void generateShiftLeft(codeGen &gen, Dyninst::Register rd, Dyninst::Register rs1, Dyninst::Register rs2);
    static void generateShiftRightLogically(codeGen &gen, Dyninst::Register rd, Dyninst::Register rs1, Dyninst::Register rs2);
    static void generateShiftRightArithmetic(codeGen &gen, Dyninst::Register rd, Dyninst::Register rs1, Dyninst::Register rs2);
    static void generateAnd(codeGen &gen, Dyninst::Register rd, Dyninst::Register rs1, Dyninst::Register rs2);
    static void generateOr(codeGen &gen, Dyninst::Register rd, Dyninst::Register rs1, Dyninst::Register rs2);
    static void generateXor(codeGen &gen, Dyninst::Register rd, Dyninst::Register rs1, Dyninst::Register rs2);
    static void generateMul(codeGen &gen, Dyninst::Register rd, Dyninst::Register rm, Dyninst::Register rn);
    static void generateDiv(codeGen &gen, Dyninst::Register rd, Dyninst::Register rm, Dyninst::Register rn);
    static void generateMove(codeGen &gen, Dyninst::Register rd, Dyninst::Register rs);

    static void generateJump(codeGen &gen, Dyninst::RegValue offset);
    static void generateJumpAndLink(codeGen &gen, Dyninst::Register rd, Dyninst::RegValue offset);
    static void generateJumpRegister(codeGen &gen, Dyninst::Register rs, Dyninst::RegValue offset);
    static void generateJumpAndLinkRegister(codeGen &gen, Dyninst::Register rd, Dyninst::Register rs, Dyninst::RegValue offset);

    // Compressed Instructions
    static void generateCAdd(codeGen &gen, Dyninst::Register rd, Dyninst::Register rs);
    static void generateCAddImm(codeGen &gen, Dyninst::Register rd, Dyninst::RegValue imm);
    static void generateCAddImmScale4SPn(codeGen &gen, Dyninst::Register rd, Dyninst::RegValue imm);
    static void generateCAddImmScale16SP(codeGen &gen, Dyninst::RegValue imm);
    static void generateCAnd(codeGen &gen, Dyninst::Register rd, Dyninst::Register rs);
    static void generateCAndImm(codeGen &gen, Dyninst::Register rd, Dyninst::RegValue imm);
    static void generateCLoadImm(codeGen &gen, Dyninst::Register rd, Dyninst::RegValue imm);
    static void generateCLoadUpperImm(codeGen &gen, Dyninst::Register rd, Dyninst::RegValue imm);
    static void generateCShiftLeftImm(codeGen &gen, Dyninst::Register rd, Dyninst::RegValue uimm);
    static void generateCShiftRightLogicallyImm(codeGen &gen, Dyninst::Register rd, Dyninst::RegValue uimm);
    static void generateCShiftRightArithmeticImm(codeGen &gen, Dyninst::Register rd, Dyninst::RegValue uimm);
    static void generateCMove(codeGen &gen, Dyninst::Register rd, Dyninst::Register rs);
    static void generateCOr(codeGen &gen, Dyninst::Register rd, Dyninst::Register rs);
    static void generateCJump(codeGen &gen, Dyninst::RegValue offset);
    static void generateCJumpAndLink(codeGen &gen, Dyninst::RegValue offset);
    static void generateCJumpRegister(codeGen &gen, Dyninst::Register rs);
    static void generateCJumpAndLinkRegister(codeGen &gen, Dyninst::Register rs);
    static void generateCNop(codeGen &gen);
    static void generateCXor(codeGen &gen, Dyninst::Register rd, Dyninst::Register rs);
};

#endif
