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

    enum MoveOp {
        MovOp_MOVK = 0xE5,
        MovOp_MOVN = 0x25,
        MovOp_MOVZ = 0xA5
    };

    enum LoadStore {
        Load,
        Store
    };

    enum ArithOp {
        Add,
        Sub
    };

    enum BitwiseOp {
        Or,
        And,
        Eor
    };

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
    static void generateConditionalBranch(codeGen& gen, Dyninst::Address to, unsigned opcode, bool s);

    // LDR/STR (immediate)
    static void generateMemLoad(codeGen &gen, LoadStore accType, Dyninst::Register r1,
            Dyninst::Register r2, Dyninst::RegValue offset, Dyninst::RegValue size, bool isUnsigned);

    static void generateMemStore(codeGen &gen, LoadStore accType, Dyninst::Register rs1,
            Dyninst::Register rs2, Dyninst::RegValue offset, Dyninst::RegValue size);

    static inline void loadImmIntoReg(codeGen &gen, Dyninst::Register rd, Dyninst::RegValue value)
    {
        generateLoadImm(gen, rd, GPR_ZERO, value);
    }

    static void saveRegister(codeGen &gen, Dyninst::Register r, int sp_offset);

    static void restoreRegister(codeGen &gen, Dyninst::Register r, int sp_offset);

    /** TODO **/
    static void generateLoadReg(codeGen &gen, Dyninst::Register rt,
                                Dyninst::Register ra, Dyninst::Register rb);

    static void generateStoreReg(codeGen &gen, Dyninst::Register rs,
                                 Dyninst::Register ra, Dyninst::Register rb);

    static void generateLoadReg64(codeGen &gen, Dyninst::Register rt,
                                  Dyninst::Register ra, Dyninst::Register rb);

    static void generateStoreReg64(codeGen &gen, Dyninst::Register rs,
                                   Dyninst::Register ra, Dyninst::Register rb);

    static void loadPartialImmIntoReg(codeGen &gen, Dyninst::Register rt,
                                      long value);

    static void generateMoveFromLR(codeGen &gen, Dyninst::Register rt);

    static void generateMoveToLR(codeGen &gen, Dyninst::Register rs);

    static void generateMoveToCR(codeGen &gen, Dyninst::Register rs);

    static bool generateMem(codeGen &gen,
                            instruction &insn,
                            Dyninst::Address origAddr,
                            Dyninst::Address newAddr,
                            Dyninst::Register newLoadReg,
                            Dyninst::Register newStoreReg);

    /** *** **/

    static void generateMul(codeGen &gen, Dyninst::Register rm, Dyninst::Register rn, Dyninst::Register rd, bool is64bit);

    static void generateDiv(codeGen &gen, Dyninst::Register rm, Dyninst::Register rn, Dyninst::Register rd, bool is64bit, bool s);

    static void generateBitwiseOpShifted(codeGen &gen, BitwiseOp op, int shift,
            Dyninst::Register rm, int imm6, Dyninst::Register rn, Dyninst::Register rd, bool is64bit);

    // This is for MOVK, MOVN, and MOVZ. For MOV use the other generateMove()
    static void generateMove(codeGen &gen, int imm16, int shift, Dyninst::Register rd, MoveOp movOp);

    // This is for MOV, which is an alias for ORR. See ARMv8 Documentation.
    static void generateMove(codeGen &gen, Dyninst::Register rd, Dyninst::Register rm, bool is64bit = true);

    static void generateMoveSP(codeGen &gen, Dyninst::Register rn, Dyninst::Register rd, bool is64bit);

    static Dyninst::Register moveValueToReg(codeGen &gen, long int val, std::vector<Dyninst::Register> *exclude = NULL);

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
    static void generateShiftLeftImm(codeGen &gen, Dyninst::Register rd, Dyninst::Register rs1, Dyninst::RegValue imm);
    static void generateShiftRightLogicallyImm(codeGen &gen, Dyninst::Register rd, Dyninst::Register rs1, Dyninst::RegValue imm);
    static void generateShiftRightArithmeticImm(codeGen &gen, Dyninst::Register rd, Dyninst::Register rs1, Dyninst::RegValue imm);

    static void generateAndImm(codeGen &gen, Dyninst::Register rd, Dyninst::Register rs1, Dyninst::RegValue imm);
    static void generateOrImm(codeGen &gen, Dyninst::Register rd, Dyninst::Register rs1, Dyninst::RegValue imm);
    static void generateXorImm(codeGen &gen, Dyninst::Register rd, Dyninst::Register rs1, Dyninst::RegValue imm);
    static void generateLoadImm(codeGen &gen, Dyninst::Register rd, Dyninst::Register rs1, Dyninst::RegValue imm);
    static void generateLoadUpperImm(codeGen &gen, Dyninst::Register rd, Dyninst::RegValue imm);

    static void generateJump(codeGen &gen, Dyninst::RegValue offset);
    static void generateJumpAndLink(codeGen &gen, Dyninst::Register rd, Dyninst::RegValue offset);
    static void generateJumpRegister(codeGen &gen, Dyninst::Register rs, Dyninst::RegValue offset);
    static void generateJumpAndLinkRegister(codeGen &gen, Dyninst::Register rd, Dyninst::Register rs, Dyninst::RegValue offset);

    // Compressed Instructions
    static void generateCAddImm(codeGen &gen, Dyninst::Register rd, Dyninst::RegValue imm);
    static void generateCAddImmScale4SPn(codeGen &gen, Dyninst::Register rd, Dyninst::RegValue imm);
    static void generateCAddImmScale16SP(codeGen &gen, Dyninst::RegValue imm);
    static void generateCLoadImm(codeGen &gen, Dyninst::Register rd, Dyninst::RegValue imm);
    static void generateCLoadUpperImm(codeGen &gen, Dyninst::Register rd, Dyninst::RegValue imm);
    static void generateCMove(codeGen &gen, Dyninst::Register rd, Dyninst::Register rs);
    static void generateCShiftLeftImm(codeGen &gen, Dyninst::Register rd, Dyninst::RegValue uimm);
    static void generateCShiftRightLogicallyImm(codeGen &gen, Dyninst::Register rd, Dyninst::RegValue uimm);
    static void generateCShiftRightArithmeticImm(codeGen &gen, Dyninst::Register rd, Dyninst::RegValue uimm);
    static void generateCAndImm(codeGen &gen, Dyninst::Register rd, Dyninst::RegValue imm);
    static void generateCNOOP(codeGen &gen);
    static void generateCJump(codeGen &gen, Dyninst::RegValue offset);
    static void generateCJumpAndLink(codeGen &gen, Dyninst::RegValue offset);
    static void generateCJumpRegister(codeGen &gen, Dyninst::Register rs);
    static void generateCJumpAndLinkRegister(codeGen &gen, Dyninst::Register rs);
    static void generateMove(codeGen &gen, Dyninst::Register rd, Dyninst::Register rs);
    static void generateNOOP(codeGen &gen);
};

#endif
