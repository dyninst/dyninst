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

#include "arch-riscv64.h"
#include "codeRange.h"
#include "common/src/dyn_register.h"
#include "dyntypes.h"

class AddressSpace;

class codeGen;

class insnCodeGen {
private:
  // Basic RISC-V instruction type generation
  static int makeUTypeInsn(codeGen &gen, Dyninst::Register rd,
                           Dyninst::RegValue imm, unsigned immop);

  static int makeITypeInsn(codeGen &gen, Dyninst::Register rd,
                           Dyninst::Register rs, Dyninst::RegValue imm,
                           unsigned funct3, unsigned opcode);

  static int makeRTypeInsn(codeGen &gen, Dyninst::Register rd,
                           Dyninst::Register rs1, Dyninst::Register rs2,
                           unsigned funct7, unsigned funct3, unsigned opcode);

  static int makeBTypeInsn(codeGen &gen, Dyninst::Register rs1,
                           Dyninst::Register rs2, Dyninst::RegValue imm,
                           unsigned funct3, unsigned opcode);

  static int makeJTypeInsn(codeGen &gen, Dyninst::Register rd,
                           Dyninst::RegValue imm, unsigned opcode);

  static int makeSTypeInsn(codeGen &gen, Dyninst::Register rs1,
                           Dyninst::Register rs2, Dyninst::RegValue imm,
                           unsigned funct3, unsigned opcode);

public:
  static void generate(codeGen &gen, NS_riscv64::instruction &insn);

  static void generate(codeGen &gen, NS_riscv64::instruction &insn,
                       unsigned position);

  static void write(codeGen &gen, NS_riscv64::instruction &insn) {
    generate(gen, insn);
  }

  static int generateTrap(codeGen &gen);

  static int generateIllegal(codeGen &gen);

  static int generateNOOP(codeGen &gen, unsigned size);

  static int generateCall(codeGen &gen, Dyninst::Address from,
                          Dyninst::Address to);

  static int generateBranch(codeGen &gen, long jump_off, bool link = false);

  static int generateBranch(codeGen &gen, Dyninst::Address from,
                            Dyninst::Address to, bool link = false);

  static int generateShortBranch(codeGen &gen, Dyninst::Address from,
                                 Dyninst::Address to, bool isCall);

  static int generateLongBranch(codeGen &gen, Dyninst::Address from,
                                Dyninst::Address to, bool isCall);

  static int generateSpringBoardBranch(codeGen &gen, Dyninst::Address from,
                                       Dyninst::Address to);

  // Using the process trap mapping for a branch
  static int generateBranchViaTrap(codeGen &gen, Dyninst::Address from,
                                   Dyninst::Address to);

  static int generateCondBranch(codeGen &gen, int bCondOp,
                                Dyninst::Register rs1, Dyninst::Register rs2,
                                Dyninst::Address from, Dyninst::Address to);

  // LDR/STR (immediate)
  static int generateMemLoad(codeGen &gen, Dyninst::Register rd,
                             Dyninst::Register rs, Dyninst::RegValue offset,
                             Dyninst::RegValue size, bool isUnsigned,
                             bool useCompressed);

  static int generateMemStore(codeGen &gen, Dyninst::Register rs1,
                              Dyninst::Register rs2, Dyninst::RegValue offset,
                              Dyninst::RegValue size, bool useCompressed);

  static int generateLd(codeGen &gen, Dyninst::Register rd,
                        Dyninst::Register rs, Dyninst::RegValue offset,
                        Dyninst::RegValue size, bool isUnsigned,
                        bool useCompressed);

  static int generateSt(codeGen &gen, Dyninst::Register rs1,
                        Dyninst::Register rs2, Dyninst::RegValue offset,
                        Dyninst::RegValue size, bool useCompressed);

  static int generateMemLoadFp(codeGen &gen, Dyninst::Register rd,
                               Dyninst::Register rs, Dyninst::RegValue offset,
                               Dyninst::RegValue size, bool useCompressed);

  static int generateMemStoreFp(codeGen &gen, Dyninst::Register rs1,
                                Dyninst::Register rs2, Dyninst::RegValue offset,
                                Dyninst::RegValue size, bool useCompressed);

  static int generateLoadRelAddr(codeGen &gen, Dyninst::Register rd,
                                 Dyninst::RegValue offset, bool useCompressed);

  static int generateMemLoadRelAddr(codeGen &gen, Dyninst::Register rd,
                                    Dyninst::RegValue offset,
                                    Dyninst::RegValue size, bool isUnsigned,
                                    bool useCompressed);

  static int saveRegister(codeGen &gen, Dyninst::Register r, int sp_offset,
                          bool useCompressed);

  static int restoreRegister(codeGen &gen, Dyninst::Register r, int sp_offset,
                             bool useCompressed);

  static int modifyData(Dyninst::Address target, NS_riscv64::instruction &insn,
                        codeGen &gen);

  static int generateAddImm(codeGen &gen, Dyninst::Register rd,
                            Dyninst::Register rs, Dyninst::RegValue sImm,
                            bool useCompressed);

  static int generateLoadImm(codeGen &gen, Dyninst::Register rd,
                             Dyninst::RegValue sImm, bool isRel, bool optimize,
                             bool useCompressed);

  static int generateCalcImm(codeGen &gen, Dyninst::Register rd,
                             Dyninst::RegValue sImm, bool optimize,
                             bool useCompressed);

  static int generateNop(codeGen &gen, bool useCompressed);

  static int generateCmpBranch(codeGen &gen, int bCond, Dyninst::Register rs1,
                               Dyninst::Register rs2, Dyninst::RegValue imm,
                               bool useCompressed);

  static int loadImmIntoReg(codeGen &gen, Dyninst::Register rd,
                            Dyninst::RegValue value, bool useCompressed);

  static int modifyCall(Dyninst::Address target, NS_riscv64::instruction &insn,
                        codeGen &gen);

  static int modifyJump(Dyninst::Address target, NS_riscv64::instruction &insn,
                        codeGen &gen);

  static int modifyJcc(Dyninst::Address target, NS_riscv64::instruction &insn,
                       codeGen &gen);

  static bool generate(codeGen &gen, NS_riscv64::instruction &insn,
                       AddressSpace *proc, Dyninst::Address origAddr,
                       Dyninst::Address newAddr,
                       patchTarget *fallthroughOverride = NULL,
                       patchTarget *targetOverride = NULL);

  // RISC-V Instruction Generation

  static int generateAddi(codeGen &gen, Dyninst::Register rd,
                          Dyninst::Register rs1, Dyninst::RegValue imm,
                          bool useCompressed);

  static int generateCSw(codeGen &gen, Dyninst::Register rd,
                         Dyninst::Register rs1, Dyninst::RegValue imm);

  static int generateCSwsp(codeGen &gen, Dyninst::Register rs2,
                           Dyninst::RegValue imm);

  static int generateCSd(codeGen &gen, Dyninst::Register rd,
                         Dyninst::Register rs1, Dyninst::RegValue imm);

  static int generateCSdsp(codeGen &gen, Dyninst::Register rs2,
                           Dyninst::RegValue imm);

  static int generateCFsw(codeGen &gen, Dyninst::Register rd,
                          Dyninst::Register rs1, Dyninst::RegValue imm);

  static int generateCFswsp(codeGen &gen, Dyninst::Register rs2,
                            Dyninst::RegValue imm);

  static int generateCFsd(codeGen &gen, Dyninst::Register rd,
                          Dyninst::Register rs1, Dyninst::RegValue imm);

  static int generateCFsdsp(codeGen &gen, Dyninst::Register rs2,
                            Dyninst::RegValue imm);

  static int generateSlli(codeGen &gen, Dyninst::Register rd,
                          Dyninst::Register rs1, Dyninst::RegValue imm,
                          bool useCompressed);

  static int generateSrli(codeGen &gen, Dyninst::Register rd,
                          Dyninst::Register rs1, Dyninst::RegValue imm,
                          bool useCompressed);

  static int generateSrai(codeGen &gen, Dyninst::Register rd,
                          Dyninst::Register rs1, Dyninst::RegValue imm,
                          bool useCompressed);

  static int generateAndi(codeGen &gen, Dyninst::Register rd,
                          Dyninst::Register rs1, Dyninst::RegValue imm,
                          bool useCompressed);

  static int generateOri(codeGen &gen, Dyninst::Register rd,
                         Dyninst::Register rs1, Dyninst::RegValue imm,
                         bool useCompressed);

  static int generateXori(codeGen &gen, Dyninst::Register rd,
                          Dyninst::Register rs1, Dyninst::RegValue imm,
                          bool useCompressed);

  static int generateLi(codeGen &gen, Dyninst::Register rd,
                        Dyninst::RegValue imm, bool optimize,
                        bool useCompressed);

  static int generateLui(codeGen &gen, Dyninst::Register rd,
                         Dyninst::RegValue offset, bool useCompressed);

  static int generateAuipc(codeGen &gen, Dyninst::Register rd,
                           Dyninst::RegValue offset, bool useCompressed);

  static int generateAdd(codeGen &gen, Dyninst::Register rd,
                         Dyninst::Register rs1, Dyninst::Register rs2,
                         bool useCompressed);

  static int generateSub(codeGen &gen, Dyninst::Register rd,
                         Dyninst::Register rs1, Dyninst::Register rs2,
                         bool useCompressed);

  static int generateSll(codeGen &gen, Dyninst::Register rd,
                         Dyninst::Register rs1, Dyninst::Register rs2,
                         bool useCompressed);

  static int generateSrl(codeGen &gen, Dyninst::Register rd,
                         Dyninst::Register rs1, Dyninst::Register rs2,
                         bool useCompressed);

  static int generateSra(codeGen &gen, Dyninst::Register rd,
                         Dyninst::Register rs1, Dyninst::Register rs2,
                         bool useCompressed);

  static int generateAnd(codeGen &gen, Dyninst::Register rd,
                         Dyninst::Register rs1, Dyninst::Register rs2,
                         bool useCompressed);

  static int generateOr(codeGen &gen, Dyninst::Register rd,
                        Dyninst::Register rs1, Dyninst::Register rs2,
                        bool useCompressed);

  static int generateXor(codeGen &gen, Dyninst::Register rd,
                         Dyninst::Register rs1, Dyninst::Register rs2,
                         bool useCompressed);

  static int generateMul(codeGen &gen, Dyninst::Register rd,
                         Dyninst::Register rm, Dyninst::Register rn,
                         bool useCompressed);

  static int generateDiv(codeGen &gen, Dyninst::Register rd,
                         Dyninst::Register rm, Dyninst::Register rn,
                         bool useCompressed);

  static int generateDivu(codeGen &gen, Dyninst::Register rd,
                          Dyninst::Register rm, Dyninst::Register rn,
                          bool useCompressed);

  static int generateMove(codeGen &gen, Dyninst::Register rd,
                          Dyninst::Register rs, bool useCompressed);

  // Generate conditional branch
  static int generateBeq(codeGen &gen, Dyninst::Register rs1,
                         Dyninst::Register rs2, Dyninst::RegValue imm,
                         bool useCompressed);

  static int generateBne(codeGen &gen, Dyninst::Register rs1,
                         Dyninst::Register rs2, Dyninst::RegValue imm,
                         bool useCompressed);

  static int generateBlt(codeGen &gen, Dyninst::Register rs1,
                         Dyninst::Register rs2, Dyninst::RegValue imm,
                         bool useCompressed);

  static int generateBge(codeGen &gen, Dyninst::Register rs1,
                         Dyninst::Register rs2, Dyninst::RegValue imm,
                         bool useCompressed);

  static int generateBltu(codeGen &gen, Dyninst::Register rs1,
                          Dyninst::Register rs2, Dyninst::RegValue imm,
                          bool useCompressed);

  static int generateBgeu(codeGen &gen, Dyninst::Register rs1,
                          Dyninst::Register rs2, Dyninst::RegValue imm,
                          bool useCompressed);

  static int generateJ(codeGen &gen, Dyninst::RegValue offset,
                       bool useCompressed);

  static int generateJal(codeGen &gen, Dyninst::Register rd,
                         Dyninst::RegValue offset, bool useCompressed);

  static int generateJr(codeGen &gen, Dyninst::Register rs,
                        Dyninst::RegValue offset, bool useCompressed);

  static int generateJalr(codeGen &gen, Dyninst::Register rd,
                          Dyninst::Register rs, Dyninst::RegValue offset,
                          bool useCompressed);

  // Compressed Instructions
  static int generateCAdd(codeGen &gen, Dyninst::Register rd,
                          Dyninst::Register rs);

  static int generateCAddi(codeGen &gen, Dyninst::Register rd,
                           Dyninst::RegValue imm);

  static int generateCAddi4spn(codeGen &gen, Dyninst::Register rd,
                               Dyninst::RegValue imm);

  static int generateCAddi16sp(codeGen &gen, Dyninst::RegValue imm);

  static int generateCAnd(codeGen &gen, Dyninst::Register rd,
                          Dyninst::Register rs);

  static int generateCAndi(codeGen &gen, Dyninst::Register rd,
                           Dyninst::RegValue imm);

  static int generateCLw(codeGen &gen, Dyninst::Register rd,
                         Dyninst::Register rs1, Dyninst::RegValue imm);

  static int generateCLwsp(codeGen &gen, Dyninst::Register rd,
                           Dyninst::RegValue imm);

  static int generateCLd(codeGen &gen, Dyninst::Register rd,
                         Dyninst::Register rs1, Dyninst::RegValue imm);

  static int generateCLdsp(codeGen &gen, Dyninst::Register rd,
                           Dyninst::RegValue imm);

  static int generateCFlw(codeGen &gen, Dyninst::Register rd,
                          Dyninst::Register rs1, Dyninst::RegValue imm);

  static int generateCFlwsp(codeGen &gen, Dyninst::Register rd,
                            Dyninst::RegValue imm);

  static int generateCFld(codeGen &gen, Dyninst::Register rd,
                          Dyninst::Register rs1, Dyninst::RegValue imm);

  static int generateCFldsp(codeGen &gen, Dyninst::Register rd,
                            Dyninst::RegValue imm);

  static int generateCLi(codeGen &gen, Dyninst::Register rd,
                         Dyninst::RegValue imm);

  static int generateCLui(codeGen &gen, Dyninst::Register rd,
                          Dyninst::RegValue imm);

  static int generateCSlli(codeGen &gen, Dyninst::Register rd,
                           Dyninst::RegValue uimm);

  static int generateCSrli(codeGen &gen, Dyninst::Register rd,
                           Dyninst::RegValue uimm);

  static int generateCSrai(codeGen &gen, Dyninst::Register rd,
                           Dyninst::RegValue uimm);

  static int generateCMv(codeGen &gen, Dyninst::Register rd,
                         Dyninst::Register rs);

  static int generateCOr(codeGen &gen, Dyninst::Register rd,
                         Dyninst::Register rs);

  static int generateCJ(codeGen &gen, Dyninst::RegValue offset);

  static int generateCJal(codeGen &gen, Dyninst::RegValue offset);

  static int generateCJr(codeGen &gen, Dyninst::Register rs);

  static int generateCJalr(codeGen &gen, Dyninst::Register rs);

  static int generateCBeqz(codeGen &gen, Dyninst::Register rs,
                           Dyninst::RegValue offset);

  static int generateCBnez(codeGen &gen, Dyninst::Register rs,
                           Dyninst::RegValue offset);

  static int generateCNop(codeGen &gen);

  static int generateCXor(codeGen &gen, Dyninst::Register rd,
                          Dyninst::Register rs);
};

#endif
