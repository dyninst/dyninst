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

#ifndef _CODEGEN_POWER_H
#define _CODEGEN_POWER_H

class AddressSpace;
class codeGen;

class insnCodeGen {
 public:
  static instructUnion *insnPtr(codeGen &gen);
  // static instructUnion *ptrAndInc(codeGen &gen);

  // All of these write into a buffer
  static void generateTrap(codeGen &gen);
  static void generateIllegal(codeGen &gen);

  static void generateBranch(codeGen &gen, long jump_off, bool link = false);
  static void generateBranch(codeGen &gen, Address from, Address to,
                             bool link = false);
  static void generateCall(codeGen &gen, Address from, Address to);

  // This is a register-stomping, full-range branch. Uses one GPR
  // and either LR or CTR. New addition: use liveness information to
  // calculate which registers to use; otherwise, trap.

  static void generateLongBranch(codeGen &gen, Address from, Address to,
                                 bool isCall);

  // A specialization of the above that assumes R0/CTR are dead.

  static void generateInterFunctionBranch(codeGen &gen, Address from,
                                          Address to, bool link = false);

  // Using the process trap mapping for a branch
  static void generateBranchViaTrap(codeGen &gen, Address from, Address to,
                                    bool isCall);

  static void generateLoadReg(codeGen &gen, Register rt, Register ra,
                              Register rb);
  static void generateStoreReg(codeGen &gen, Register rs, Register ra,
                               Register rb);
  static void generateLoadReg64(codeGen &gen, Register rt, Register ra,
                                Register rb);
  static void generateStoreReg64(codeGen &gen, Register rs, Register ra,
                                 Register rb);
  static void generateAddReg(codeGen &gen, int op, Register rt, Register ra,
                             Register rb);
  static void generateImm(codeGen &gen, int op, Register rt, Register ra,
                          int immd);
  static void generateMemAccess64(codeGen &gen, int op, int xop, Register r1,
                                  Register r2, int immd);
  static void generateLShift(codeGen &gen, Register rs, int shift, Register ra);
  static void generateRShift(codeGen &gen, Register rs, int shift, Register ra);
  static void generateLShift64(codeGen &gen, Register rs, int shift,
                               Register ra);
  static void generateRShift64(codeGen &gen, Register rs, int shift,
                               Register ra);
  static void generateNOOP(codeGen &gen, unsigned size = 4);

  static void generateSimple(codeGen &gen, int op, Register src1, Register src2,
                             Register dest);
  static void generateRelOp(codeGen &gen, int cond, int mode, Register rs1,
                            Register rs2, Register rd);
  static void loadImmIntoReg(codeGen &gen, Register rt, long value);
  static void loadPartialImmIntoReg(codeGen &gen, Register rt, long value);

  static void generateMoveFromLR(codeGen &gen, Register rt);
  static void generateMoveToLR(codeGen &gen, Register rs);
  static void generateMoveToCR(codeGen &gen, Register rs);

  static void generate(codeGen &gen, instruction &insn);
  static void write(codeGen &gen, instruction &insn) { generate(gen, insn); }

  static bool generate(codeGen &gen, instruction &insn, AddressSpace *proc,
                       Address origAddr, Address newAddr,
                       patchTarget *fallthroughOverride = NULL,
                       patchTarget *targetOverride = NULL);

  static bool generateMem(codeGen &gen, instruction &insn, Address origAddr,
                          Address newAddr, Register newLoadReg,
                          Register newStoreReg);

  // Routines to create/remove a new stack frame for getting scratch registers
  static int createStackFrame(codeGen &gen, int numRegs,
                              pdvector<Register> &freeReg,
                              pdvector<Register> &excludeReg);
  static void removeStackFrame(codeGen &gen);

  static bool modifyJump(Address target, NS_power::instruction &insn,
                         codeGen &gen);
  static bool modifyJcc(Address target, NS_power::instruction &insn,
                        codeGen &gen);
  static bool modifyCall(Address target, NS_power::instruction &insn,
                         codeGen &gen);
  static bool modifyData(Address target, NS_power::instruction &insn,
                         codeGen &gen);
};

#endif
