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

    enum IndexMode{
        Post,
        Pre,
        Offset
    };

    static instructUnion *insnPtr(codeGen &gen);
    //static instructUnion *ptrAndInc(codeGen &gen);

    // All of these write into a buffer
    static void generateTrap(codeGen &gen);

    static void generateIllegal(codeGen &gen);

    static void generateBranch(codeGen &gen,
                               long jump_off,
                               bool link = false);

    static void generateBranch(codeGen &gen,
                               Dyninst::Address from,
                               Dyninst::Address to,
                               bool link = false);

    static void generateCall(codeGen &gen,
                             Dyninst::Address from,
                             Dyninst::Address to);

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
    // immd in the range -256 to 255
    static void generateMemAccess(codeGen &gen, LoadStore accType, Dyninst::Register r1,
            Dyninst::Register r2, int immd, unsigned size, IndexMode im=Post);

    static void generateMemAccessFP(codeGen &gen, LoadStore accType, Dyninst::Register rt,
            Dyninst::Register rn, int immd, int size, bool is128bit, IndexMode im=Offset);

    static inline void loadImmIntoReg(codeGen &gen, Dyninst::Register rd, Dyninst::RegValue value)
    {
        // If the value is 6 bits wide (-32 <= value < 32), we use the c.li instruction
        if (value >= -0x20 && value < 0x20) {
            generateCli(gen, rd, value);
            return;
        }

        // If the value is 12 bits wide (-2048 <= value < 2048), we use the addi instruction (mv pseudo instruction)
        if (value >= -0x800 && value < 0x800) {
            generateMv(gen, rd, value);
            return;
        }

        // If the value is larger than 12 bits but less than 32 bits,
        // the value must be loaded in two steps using lui and addi
        if (value >= -0x80000000 && value < 0x80000000) {
            Dyninst::RegValue lui_imm = (value & 0xfffff000) >> 12;
            Dyninst::RegValue addi_imm = value & 0xfff;
            // If the most significant bit of addi_imm is 1 (addi_imm is negative), we should add 1 to lui_imm
            if (addi_imm & 0x800) {
                lui_imm = (lui_imm + 1) & 0xfffff;
            }
            generateLui(gen, rd, lui_imm);
            generateAddi(gen, rd, rd, addi_imm);
            return;
        }

        // If the value is a 64 bit long, the sequence of instructions is more complicated
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

        generateLui(gen, rd, lui_imm);
        generateAddi(gen, rd, rd, addi_imm);
        generateSlli(gen, rd, rd, 11);
        generateOri(gen, rd, rd, ori_imm1);
        generateSlli(gen, rd, rd, 11);
        generateOri(gen, rd, rd, ori_imm2);
        generateSlli(gen, rd, rd, 10);
        generateOri(gen, rd, rd, ori_imm3);

        return;
    }

    static void saveRegister(codeGen &gen, Dyninst::Register r, int sp_offset, IndexMode im=Offset);

    static void restoreRegister(codeGen &gen, Dyninst::Register r, int sp_offset, IndexMode im=Offset);

    /** TODO **/
    static void generateLoadReg(codeGen &gen, Dyninst::Register rt,
                                Dyninst::Register ra, Dyninst::Register rb);

    static void generateStoreReg(codeGen &gen, Dyninst::Register rs,
                                 Dyninst::Register ra, Dyninst::Register rb);

    static void generateLoadReg64(codeGen &gen, Dyninst::Register rt,
                                  Dyninst::Register ra, Dyninst::Register rb);

    static void generateStoreReg64(codeGen &gen, Dyninst::Register rs,
                                   Dyninst::Register ra, Dyninst::Register rb);

    static void generateLShift(codeGen &gen, Dyninst::Register rs,
                               int shift, Dyninst::Register ra);

    static void generateRShift(codeGen &gen, Dyninst::Register rs,
                               int shift, Dyninst::Register ra);

    static void generateLShift64(codeGen &gen, Dyninst::Register rs,
                                 int shift, Dyninst::Register ra);

    static void generateRShift64(codeGen &gen, Dyninst::Register rs,
                                 int shift, Dyninst::Register ra);

    static void generateRelOp(codeGen &gen, int cond,
                              int mode, Dyninst::Register rs1,
                              Dyninst::Register rs2, Dyninst::Register rd);

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

    static void generateAddSubShifted(
            codeGen &gen, ArithOp op, int shift, int imm6, Dyninst::Register rm, Dyninst::Register rn, Dyninst::Register rd, bool is64bit);

    static void generateAddSubImmediate(
            codeGen &gen, ArithOp op, int shift, int imm12, Dyninst::Register rn, Dyninst::Register rd, bool is64bit);

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


    static void generateNOOP(codeGen &gen, unsigned size = 4);

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

    static void generateAddi(codeGen &gen, Dyninst::Register rd, Dyninst::Register rs1, Dyninst::RegValue imm);
    static void generateCli(codeGen &gen, Dyninst::Register rd, Dyninst::RegValue imm);
    static void generateSlli(codeGen &gen, Dyninst::Register rd, Dyninst::Register rs1, Dyninst::RegValue imm);
    static void generateOri(codeGen &gen, Dyninst::Register rd, Dyninst::RegValue imm);
    static void generateLui(codeGen &gen, Dyninst::Register rd, Dyninst::RegValue imm);
    static inline void generateMv(codeGen &gen, Dyninst::Register rd, Dyninst::RegValue imm) {
        generateAddi(gen, rd, 0, imm);
    }
};

#endif
