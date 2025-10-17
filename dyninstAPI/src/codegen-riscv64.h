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
#include "arch-riscv64.h"

class AddressSpace;

class codeGen;

class insnCodeGen {
public:

    //static instructUnion *ptrAndInc(codeGen &gen);

    // Basic RISC-V instruction type generation
    static void makeUTypeInsn(codeGen &gen,
                              Dyninst::Register rd,
                              Dyninst::RegValue imm,
                              unsigned immop);

    static void makeITypeInsn(codeGen &gen,
                              Dyninst::Register rd,
                              Dyninst::Register rs,
                              Dyninst::RegValue imm,
                              unsigned funct3,
                              unsigned opcode);

    static void makeRTypeInsn(codeGen &gen,
                              Dyninst::Register rd,
                              Dyninst::Register rs1,
                              Dyninst::Register rs2,
                              unsigned funct7,
                              unsigned funct3,
                              unsigned opcode);

    static void makeBTypeInsn(codeGen &gen,
                              Dyninst::Register rs1,
                              Dyninst::Register rs2,
                              Dyninst::RegValue imm,
                              unsigned funct3,
                              unsigned opcode);

    static void makeJTypeInsn(codeGen &gen,
                              Dyninst::Register rd,
                              Dyninst::RegValue imm,
                              unsigned opcode);

    static void makeSTypeInsn(codeGen &gen,
                              Dyninst::Register rs1,
                              Dyninst::Register rs2,
                              Dyninst::RegValue imm,
                              unsigned funct3, unsigned opcode);

    // All of these write into a buffer
    static void generateTrap(codeGen &gen);

    static void generateIllegal(codeGen &gen);

    static void generateNOOP(codeGen &gen,
                             unsigned size);

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
    
    static void generateShortBranch(codeGen &gen,
                                    Dyninst::Address from,
                                    Dyninst::Address to,
                                    bool isCall);

    static void generateLongBranch(codeGen &gen,
                                   Dyninst::Address from,
                                   Dyninst::Address to,
                                   bool isCall);

    static void generateSpringBoardBranch(codeGen &gen,
                                          Dyninst::Address from,
                                          Dyninst::Address to);

    // Using the process trap mapping for a branch
    static void generateBranchViaTrap(codeGen &gen,
                                      Dyninst::Address from,
                                      Dyninst::Address to);

    static void generateCondBranch(codeGen &gen,
                                   int bCondOp,
                                   Dyninst::Register rs1,
                                   Dyninst::Register rs2,
                                   Dyninst::Address from,
                                   Dyninst::Address to);

    // LDR/STR (immediate)
    static bool generateMemLoad(codeGen &gen,
                                Dyninst::Register rd,
                                Dyninst::Register rs,
                                Dyninst::RegValue offset,
                                Dyninst::RegValue size,
                                bool isUnsigned,
                                bool useRVC);

    static bool generateMemStore(codeGen &gen,
                                 Dyninst::Register rs1,
                                 Dyninst::Register rs2,
                                 Dyninst::RegValue offset,
                                 Dyninst::RegValue size,
                                 bool useRVC);

    static bool generateLd(codeGen &gen,
                           Dyninst::Register rd,
                           Dyninst::Register rs,
                           Dyninst::RegValue offset,
                           Dyninst::RegValue size,
                           bool isUnsigned,
                           bool useRVC);

    static bool generateSt(codeGen &gen,
                           Dyninst::Register rs1,
                           Dyninst::Register rs2,
                           Dyninst::RegValue offset,
                           Dyninst::RegValue size,
                           bool useRVC);

    static bool generateMemLoadFp(codeGen &gen,
                                  Dyninst::Register rd,
                                  Dyninst::Register rs,
                                  Dyninst::RegValue offset,
                                  Dyninst::RegValue size,
                                  bool useRVC);

    static bool generateMemStoreFp(codeGen &gen,
                                   Dyninst::Register rs1,
                                   Dyninst::Register rs2,
                                   Dyninst::RegValue offset,
                                   Dyninst::RegValue size,
                                   bool useRVC);

    static bool saveRegister(codeGen &gen,
                             Dyninst::Register r,
                             int sp_offset,
                             bool useRVC);

    static bool restoreRegister(codeGen &gen,
                                Dyninst::Register r,
                                int sp_offset,
                                bool useRVC);

    static bool modifyData(Dyninst::Address target,
			               NS_riscv64::instruction &insn,
			               codeGen &gen);

    static bool generateAddImm(codeGen &gen,
                               Dyninst::Register rd,
                               Dyninst::Register rs,
                               Dyninst::RegValue sImm,
                               bool useRVC);

    static bool generateLoadImm(codeGen &gen,
                                Dyninst::Register rd,
                                Dyninst::RegValue sImm,
                                bool isRel,
                                bool optimize,
                                bool useRVC);

    static bool generateCalcImm(codeGen &gen,
                                Dyninst::Register rd,
                                Dyninst::RegValue sImm,
                                bool optimize,
                                bool useRVC);

    static Dyninst::Register moveValueToReg(codeGen &gen,
                                            long int val,
                                            std::vector<Dyninst::Register> *exclude = NULL);

    static bool generateNop(codeGen &gen,
                            bool useRVC);

    static void generateCmpBranch(codeGen &gen,
                                  int bCond,
                                  Dyninst::Register rs1,
                                  Dyninst::Register rs2,
                                  Dyninst::RegValue imm,
                                  bool useRVC);

    static bool loadImmIntoReg(codeGen &gen,
                               Dyninst::Register rd,
                               Dyninst::RegValue value,
                               bool useRVC);
                               
    static bool modifyCall(Dyninst::Address target,
                           NS_riscv64::instruction &insn,
                           codeGen &gen);

    static bool modifyJump(Dyninst::Address target,
                           NS_riscv64::instruction &insn,
                           codeGen &gen);

    static bool modifyJcc(Dyninst::Address target,
			              NS_riscv64::instruction &insn,
			              codeGen &gen);

    static void generate(codeGen &gen, NS_riscv64::instruction &insn);

    // Copy instruction at position in codeGen buffer
    static void generate(codeGen &gen, NS_riscv64::instruction &insn, unsigned position);

    static void write(codeGen &gen, NS_riscv64::instruction &insn) { generate(gen, insn); }

    static bool generate(codeGen &gen,
                         NS_riscv64::instruction &insn,
                         AddressSpace *proc,
                         Dyninst::Address origAddr,
                         Dyninst::Address newAddr,
                         patchTarget *fallthroughOverride = NULL,
                         patchTarget *targetOverride = NULL);

    // RISC-V Instruction Generation

    static bool generateAddi(codeGen &gen,
                             Dyninst::Register rd,
                             Dyninst::Register rs1,
                             Dyninst::RegValue imm,
                             bool useRVC);

    static void generateCSw(codeGen &gen,
                            Dyninst::Register rd,
                            Dyninst::Register rs1,
                            Dyninst::RegValue imm);

    static void generateCSwsp(codeGen &gen,
                              Dyninst::Register rs2,
                              Dyninst::RegValue imm);

    static void generateCSd(codeGen &gen,
                            Dyninst::Register rd,
                            Dyninst::Register rs1,
                            Dyninst::RegValue imm);

    static void generateCSdsp(codeGen &gen,
                              Dyninst::Register rs2,
                              Dyninst::RegValue imm);

    static void generateCFsw(codeGen &gen,
                             Dyninst::Register rd,
                             Dyninst::Register rs1,
                             Dyninst::RegValue imm);

    static void generateCFswsp(codeGen &gen,
                               Dyninst::Register rs2,
                               Dyninst::RegValue imm);

    static void generateCFsd(codeGen &gen,
                             Dyninst::Register rd,
                             Dyninst::Register rs1,
                             Dyninst::RegValue imm);

    static void generateCFsdsp(codeGen &gen,
                               Dyninst::Register rs2,
                               Dyninst::RegValue imm);

    static bool generateSlli(codeGen &gen,
                             Dyninst::Register rd,
                             Dyninst::Register rs1,
                             Dyninst::RegValue imm,
                             bool useRVC);

    static bool generateSrli(codeGen &gen,
                             Dyninst::Register rd,
                             Dyninst::Register rs1,
                             Dyninst::RegValue imm,
                             bool useRVC);

    static bool generateSrai(codeGen &gen,
                             Dyninst::Register rd,
                             Dyninst::Register rs1,
                             Dyninst::RegValue imm,
                             bool useRVC);

    static bool generateAndi(codeGen &gen,
                             Dyninst::Register rd,
                             Dyninst::Register rs1,
                             Dyninst::RegValue imm,
                             bool useRVC);

    static bool generateOri(codeGen &gen,
                            Dyninst::Register rd,
                            Dyninst::Register rs1,
                            Dyninst::RegValue imm,
                            bool useRVC);

    static bool generateXori(codeGen &gen,
                             Dyninst::Register rd,
                             Dyninst::Register rs1,
                             Dyninst::RegValue imm,
                             bool useRVC);

    static bool generateLi(codeGen &gen,
                           Dyninst::Register rd,
                           Dyninst::RegValue imm,
                           bool optimize,
                           bool useRVC);

    static bool generateLui(codeGen &gen,
                            Dyninst::Register rd,
                            Dyninst::RegValue offset,
                            bool useRVC);

    static bool generateAuipc(codeGen &gen,
                              Dyninst::Register rd,
                              Dyninst::RegValue offset,
                              bool useRVC);

    static bool generateAdd(codeGen &gen,
                            Dyninst::Register rd,
                            Dyninst::Register rs1,
                            Dyninst::Register rs2,
                            bool useRVC);

    static bool generateSub(codeGen &gen,
                            Dyninst::Register rd,
                            Dyninst::Register rs1,
                            Dyninst::Register rs2,
                            bool useRVC);

    static bool generateSll(codeGen &gen,
                            Dyninst::Register rd,
                            Dyninst::Register rs1,
                            Dyninst::Register rs2,
                            bool useRVC);

    static bool generateSrl(codeGen &gen,
                            Dyninst::Register rd,
                            Dyninst::Register rs1,
                            Dyninst::Register rs2,
                            bool useRVC);

    static bool generateSra(codeGen &gen,
                            Dyninst::Register rd,
                            Dyninst::Register rs1,
                            Dyninst::Register rs2,
                            bool useRVC);

    static bool generateAnd(codeGen &gen,
                            Dyninst::Register rd,
                            Dyninst::Register rs1,
                            Dyninst::Register rs2,
                            bool useRVC);

    static bool generateOr(codeGen &gen,
                           Dyninst::Register rd,
                           Dyninst::Register rs1,
                           Dyninst::Register rs2,
                           bool useRVC);

    static bool generateXor(codeGen &gen,
                            Dyninst::Register rd,
                            Dyninst::Register rs1,
                            Dyninst::Register rs2,
                            bool useRVC);

    static bool generateMul(codeGen &gen,
                            Dyninst::Register rd,
                            Dyninst::Register rm,
                            Dyninst::Register rn,
                            bool useRVC);

    static bool generateDiv(codeGen &gen,
                            Dyninst::Register rd,
                            Dyninst::Register rm,
                            Dyninst::Register rn,
                            bool useRVC);

    static bool generateMove(codeGen &gen,
                             Dyninst::Register rd,
                             Dyninst::Register rs,
                             bool useRVC);


    // Generate conditional branch
    static bool generateBeq(codeGen &gen,
                            Dyninst::Register rs1,
                            Dyninst::Register rs2,
                            Dyninst::RegValue imm,
                            bool useRVC);

    static bool generateBne(codeGen &gen,
                            Dyninst::Register rs1,
                            Dyninst::Register rs2,
                            Dyninst::RegValue imm,
                            bool useRVC);

    static bool generateBlt(codeGen &gen,
                            Dyninst::Register rs1,
                            Dyninst::Register rs2,
                            Dyninst::RegValue imm,
                            bool useRVC);

    static bool generateBge(codeGen &gen,
                            Dyninst::Register rs1,
                            Dyninst::Register rs2,
                            Dyninst::RegValue imm,
                            bool useRVC);

    static bool generateBltu(codeGen &gen,
                             Dyninst::Register rs1,
                             Dyninst::Register rs2,
                             Dyninst::RegValue imm,
                             bool useRVC);

    static bool generateBgeu(codeGen &gen,
                             Dyninst::Register rs1,
                             Dyninst::Register rs2,
                             Dyninst::RegValue imm,
                             bool useRVC);

    static bool generateJ(codeGen &gen,
                          Dyninst::RegValue offset,
                          bool useRVC);

    static bool generateJal(codeGen &gen,
                            Dyninst::Register rd,
                            Dyninst::RegValue offset,
                            bool useRVC);

    static bool generateJr(codeGen &gen,
                           Dyninst::Register rs,
                           Dyninst::RegValue offset,
                           bool useRVC);

    static bool generateJalr(codeGen &gen,
                             Dyninst::Register rd,
                             Dyninst::Register rs,
                             Dyninst::RegValue offset,
                             bool useRVC);

    // Compressed Instructions
    static void generateCAdd(codeGen &gen,
                             Dyninst::Register rd,
                             Dyninst::Register rs);

    static void generateCAddi(codeGen &gen,
                              Dyninst::Register rd,
                              Dyninst::RegValue imm);

    static void generateCAddi4spn(codeGen &gen,
                                  Dyninst::Register rd,
                                  Dyninst::RegValue imm);

    static void generateCAddi16sp(codeGen &gen,
                                  Dyninst::RegValue imm);

    static void generateCAnd(codeGen &gen,
                            Dyninst::Register rd,
                            Dyninst::Register rs);

    static void generateCAndi(codeGen &gen,
                              Dyninst::Register rd,
                              Dyninst::RegValue imm);

    static void generateCLw(codeGen &gen,
                            Dyninst::Register rd,
                            Dyninst::Register rs1,
                            Dyninst::RegValue imm);

    static void generateCLwsp(codeGen &gen,
                              Dyninst::Register rd,
                              Dyninst::RegValue imm);

    static void generateCLd(codeGen &gen,
                            Dyninst::Register rd,
                            Dyninst::Register rs1,
                            Dyninst::RegValue imm);

    static void generateCLdsp(codeGen &gen,
                              Dyninst::Register rd,
                              Dyninst::RegValue imm);

    static void generateCFlw(codeGen &gen,
                             Dyninst::Register rd,
                             Dyninst::Register rs1,
                             Dyninst::RegValue imm);

    static void generateCFlwsp(codeGen &gen,
                               Dyninst::Register rd,
                               Dyninst::RegValue imm);

    static void generateCFld(codeGen &gen,
                             Dyninst::Register rd,
                             Dyninst::Register rs1,
                             Dyninst::RegValue imm);

    static void generateCFldsp(codeGen &gen,
                               Dyninst::Register rd,
                               Dyninst::RegValue imm);

    static void generateCLi(codeGen &gen,
                            Dyninst::Register rd,
                            Dyninst::RegValue imm);

    static void generateCLui(codeGen &gen,
                             Dyninst::Register rd,
                             Dyninst::RegValue imm);

    static void generateCSlli(codeGen &gen,
                              Dyninst::Register rd,
                              Dyninst::RegValue uimm);

    static void generateCSrli(codeGen &gen,
                              Dyninst::Register rd,
                              Dyninst::RegValue uimm);

    static void generateCSrai(codeGen &gen,
                              Dyninst::Register rd,
                              Dyninst::RegValue uimm);

    static void generateCMv(codeGen &gen,
                            Dyninst::Register rd,
                            Dyninst::Register rs);

    static void generateCOr(codeGen &gen,
                            Dyninst::Register rd,
                            Dyninst::Register rs);

    static void generateCJ(codeGen &gen,
                           Dyninst::RegValue offset);

    static void generateCJal(codeGen &gen,
                             Dyninst::RegValue offset);

    static void generateCJr(codeGen &gen,
                            Dyninst::Register rs);

    static void generateCJalr(codeGen &gen,
                              Dyninst::Register rs);

    static void generateCBeqz(codeGen &gen,
                              Dyninst::Register rs,
                              Dyninst::RegValue offset);

    static void generateCBnez(codeGen &gen,
                              Dyninst::Register rs,
                              Dyninst::RegValue offset);

    static void generateCNop(codeGen &gen);

    static void generateCXor(codeGen &gen,
                             Dyninst::Register rd,
                             Dyninst::Register rs);
};

#endif
