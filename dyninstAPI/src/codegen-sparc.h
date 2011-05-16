/*
 * Copyright (c) 1996-2011 Barton P. Miller
 * 
 * We provide the Paradyn Parallel Performance Tools (below
 * described as "Paradyn") on an AS IS basis, and do not warrant its
 * validity or performance.  We reserve the right to update, modify,
 * or discontinue this software at any time.  We shall have no
 * obligation to supply such updates or modifications or any other
 * form of support to you.
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

#if !defined(arch_sparc)
#error "invalid architecture-os inclusion"
#endif

#ifndef _CODEGEN_SPARC_H_
#define _CODEGEN_SPARC_H_

//#include "common/h/Types.h"

class codeGen;
class AddressSpace;

class insnCodeGen {
 public:
    static instructUnion *insnPtr(codeGen &gen);
    static instructUnion *ptrAndInc(codeGen &gen);

    static void generateTrap(codeGen &gen);
    static void generateIllegal(codeGen &gen);

    static void generateNOOP(codeGen &gen,
                             unsigned size = 4);
    static void generateBranch(codeGen &gen,
                               int jump_off);
    static void generateBranch(codeGen &gen,
                               Address from,
                               Address to);
    static void generateInterFunctionBranch(codeGen &gen,
                                            Address from,
                                            Address to) {
        return generateBranch(gen, from, to);
    }


    static void generateCall(codeGen &gen,
                             Address fromAddr,
                             Address toAddr);
    static void generateJmpl(codeGen &gen,
                             int rs1,
                             int jump_off,
                             int rd);
    static void generateCondBranch(codeGen &gen,
                                   int jump_off,
                                   unsigned condition,
                                   bool annul);
    static void generateAnnulledBranch(codeGen &gen,
                                       int jump_off);
    static void generateSimple(codeGen &gen,
                               int op,
                               Register rs1,
                               Register rs2,
                               Register rd);
    static void generateImm(codeGen &gen,
                            int op,
                            Register rs1,
                            int immd,
                            Register rd);
    static void generateImmRelOp(codeGen &gen,
                               int cond,
                               Register rs1,
                               int immd,
                                 Register rd);
    static void generateRelOp(codeGen &gen,
                              int cond,
                              Register rs1,
                              Register rs2,
                              Register rd);
    static void generateSetHi(codeGen &gen,
                              int src1,
                              int dest);
    static void generateStore(codeGen &gen,
                              int rd,
                              int rs1,
                              int store_off);
    static void generateLShift(codeGen &gen,
                               int rs1,
                               int shift,
                               int rd);
    static void generateRShift(codeGen &gen,
                               int rs1,
                               int shift,
                               int rd);
    static void generateLoad(codeGen &gen,
                             int rs1,
                             int load_off,
                             int rd);
    static void generateStoreD(codeGen &gen,
                               int rd,
                               int rs1,
                               int shift);
    static void generateLoadD(codeGen &gen,
                               int rs1,
                               int load_off,
                               int rd);
    static void generateLoadB(codeGen &gen,
                              int rs1,
                              int load_off,
                              int rd);    
    static void generateLoadH(codeGen &gen,
                              int rs1,
                              int load_off,
                              int rd);
    static void generateStoreFD(codeGen &gen,
                                int rd,
                                int rs1,
                                int shift);
    static void generateLoadFD(codeGen &gen,
                               int rs1,
                               int load_off,
                               int rd);
    static void generateFlushw(codeGen &gen);
    static void generateTrapRegisterSpill(codeGen &gen);
  
    static void write(codeGen &gen, instruction &insn);
    static void generate(codeGen &gen, instruction &isnsn);

    static bool generate(codeGen &gen,
                  instruction &insn,
                  AddressSpace *proc,
                  Address origAddr,
                  Address newAddr,
                  patchTarget *fallthroughOverride = NULL,
                  patchTarget *targetOverride = NULL);

    static bool generateMem(codeGen &gen,
                     instruction &insn,
                     Address origAddr,
                     Address newAddr,
                     Register newLoadReg,
                     Register newStoreReg);
};

#endif
