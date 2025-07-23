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

#include <stdio.h>
#include <set>
#include <map>
#include <vector>
#include "Architecture.h"
#include "entryIDs.h"
#include "dyntypes.h"
#include "dyn_register.h"
#include "arch-x86.h"
#include "arch-aarch64.h"
#include "arch-power.h"
#include "patch.h"

#include "common/src/ia32_locations.h"

#ifndef _INSN_CODEGEN_H
#define _INSN_CODEGEN_H

// Code generation

class codeGen;
class AddressSpace;

class insnCodeGen {
 public:

  static void generateBranch(codeGen &gen, int disp);
  //static void generateBranch(codeGen &gen, Dyninst::Address from, Dyninst::Address to);
  static void generateBranch(codeGen &gen,
                               long jump_off,
                               bool link = false);

  static void generateBranch(codeGen &gen,
                               Dyninst::Address from,
                               Dyninst::Address to,
                               bool link = false);

  static void generateBranch64(codeGen &gen, Dyninst::Address to);
  static void generateBranch32(codeGen &gen, Dyninst::Address to);
  static void generateCall(codeGen &gen, Dyninst::Address from, Dyninst::Address to);

  #if defined(DYNINST_CODEGEN_ARCH_AARCH64)
    static void generateNOOP(codeGen &gen, unsigned size = 4);
  #else //if defined(DYNINST_CODEGEN_ARCH_X86_64)
    static void generateNOOP(codeGen &gen, unsigned size = 1);
  #endif

  static void generateIllegal(codeGen &gen);
  static void generateTrap(codeGen &gen);

  static void generate(codeGen &gen, NS_x86::instruction & insn);
  static void generate(codeGen &gen, NS_power::instruction & insn);
  static void generate(codeGen &gen, NS_aarch64::instruction & insn);
  // Copy instruction at position in codeGen buffer
  static void generate(codeGen &gen, NS_aarch64::instruction &insn, unsigned position);
  static bool generate(codeGen &gen,
                       NS_aarch64::instruction &insn,
                       AddressSpace *proc,
                       Dyninst::Address origAddr,
                       Dyninst::Address newAddr,
                       patchTarget *fallthroughOverride = NULL,
                       patchTarget *targetOverride = NULL);

  // And generate an equivalent stream somewhere else...
  // fallthroughOverride and targetOverride are used for
  // making the behavior of jumps change. It won't work for 
  // jumptables; that should be cleared up sometime.
  static bool generate(codeGen &gen,
                NS_x86::instruction & insn,
                AddressSpace *addrSpace,
                Dyninst::Address origAddr,
                Dyninst::Address newAddr,
                patchTarget *fallthroughOverride = NULL,
                patchTarget *targetOverride = NULL);

  static bool generateMem(codeGen &gen,
                   NS_x86::instruction & insn,
                   Dyninst::Address origAddr,
                   Dyninst::Address newAddr,
                   Dyninst::Register newLoadReg,
                   Dyninst::Register newStoreReg);

  static bool modifyJump(Dyninst::Address target,
                         NS_x86::instruction &insn, 
                         codeGen &gen);
  static bool modifyJcc(Dyninst::Address target,
                        NS_x86::instruction &insn, 
                         codeGen &gen);
  static bool modifyCall(Dyninst::Address target,
                         NS_x86::instruction &insn, 
                         codeGen &gen);
  static bool modifyData(Dyninst::Address target,
                         NS_x86::instruction &insn, 
                         codeGen &gen);

  static bool modifyData(Dyninst::Address target,
                           NS_aarch64::instruction &insn,
                           codeGen &gen);

  static bool modifyData(Dyninst::Address target,
                           NS_power::instruction &insn,
                           codeGen &gen);

 
  static bool modifyDisp(signed long newDisp,
                         NS_x86::instruction &insn,
                         codeGen &gen, Dyninst::Architecture arch, Dyninst::Address addr);

  static bool modifyJump(Dyninst::Address target,
                         NS_aarch64::instruction &insn,
                         codeGen &gen);

  static bool modifyJump(Dyninst::Address target,
                         NS_power::instruction &insn,
                         codeGen &gen);


  static bool modifyJcc(Dyninst::Address target,
                        NS_aarch64::instruction &insn,
                        codeGen &gen);

  static bool modifyJcc(Dyninst::Address target,
                        NS_power::instruction &insn,
                        codeGen &gen);


  static bool modifyCall(Dyninst::Address target,
                         NS_aarch64::instruction &insn,
                         codeGen &gen);

  static bool modifyCall(Dyninst::Address target,
                         NS_power::instruction &insn,
                         codeGen &gen);
  // Generate conditional branch

  // Generate conditional branch
  static void generateConditionalBranch(codeGen& gen, Dyninst::Address to, unsigned opcode, bool s);

  static bool generateMem(codeGen &gen,
                          NS_aarch64::instruction &insn,
                          Dyninst::Address origAddr,
                          Dyninst::Address newAddr,
                          Dyninst::Register newLoadReg,
                          Dyninst::Register newStoreReg);

   static bool generateMem(codeGen &gen,
                          NS_power::instruction &insn,
                          Dyninst::Address origAddr,
                          Dyninst::Address newAddr,
                          Dyninst::Register newLoadReg,
                          Dyninst::Register newStoreReg);

  
 

};



#endif
