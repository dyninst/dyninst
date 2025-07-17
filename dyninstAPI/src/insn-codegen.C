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

#include <assert.h>
#include <stdio.h>
#include "boost/assign/list_of.hpp"
#include "boost/assign/std/vector.hpp"
#include "boost/assign/std/set.hpp"
#include <map>
#include <string>
#include "common/src/ia32_locations.h"
#include "codegen.h"
#include "debug.h"
#include "addressSpace.h"

#include "InstructionDecoder.h"
#include "Instruction.h"

#include "codegen-x86.h"
#include "codegen-power.h"
#include "codegen-aarch64.h"
#include "emit-x86.h"
#include "inst-x86.h"
#include "pcrel.h"

#include "StackMod/StackAccess.h"

#include "unaligned_memory_access.h"

using namespace std;
using namespace boost::assign;
using namespace Dyninst::InstructionAPI;


void insnCodeGen::generateIllegal(codeGen &gen) {
#if defined(DYNINST_CODEGEN_ARCH_X86_64)
  insnCodeGenX86::generateIllegal(gen);
#elif defined(DYNINST_CODEGEN_ARCH_AARCH64)
  insnCodeGenAarch64::generateIllegal(gen);
#elif defined(DYNINST_CODEGEN_ARCH_POWER)
  insnCodeGenPower::generateIllegal(gen);
#endif
}

void insnCodeGen::generateTrap(codeGen &gen) {
#if defined(DYNINST_CODEGEN_ARCH_X86_64)
  insnCodeGenX86::generateTrap(gen);
#elif defined(DYNINST_CODEGEN_ARCH_AARCH64)
  insnCodeGenAarch64::generateTrap(gen);
#elif defined(DYNINST_CODEGEN_ARCH_POWER)
  insnCodeGenPower::generateTrap(gen);
#endif
}

/*
 * change the insn at addr to be a branch to newAddr.
 *   Used to add multiple tramps to a point.
 */

void insnCodeGen::generateBranch(codeGen &gen,
                                 Dyninst::Address fromAddr, Dyninst::Address toAddr, bool link)
{
#if defined(DYNINST_CODEGEN_ARCH_X86_64)
  insnCodeGenX86::generateBranch(gen,fromAddr,toAddr); (void) link;
#elif defined(DYNINST_CODEGEN_ARCH_AARCH64)
  insnCodeGenAarch64::generateBranch(gen,fromAddr,toAddr,link);
#elif defined(DYNINST_CODEGEN_ARCH_POWER)
  insnCodeGenPower::generateBranch(gen,fromAddr,toAddr,link);
#endif
}

void insnCodeGen::generateBranch(codeGen &gen,
                                 int disp32)
{
#if defined(DYNINST_CODEGEN_ARCH_X86_64)
  insnCodeGenX86::generateBranch(gen,disp32);
#elif defined(DYNINST_CODEGEN_ARCH_AARCH64)
  insnCodeGenAarch64::generateBranch(gen,disp32);
#elif defined(DYNINST_CODEGEN_ARCH_POWER)
  insnCodeGenPower::generateBranch(gen,disp32);
#endif
}

void insnCodeGen::generateCall(codeGen &gen,
                               Dyninst::Address from,
                               Dyninst::Address target)
{

#if defined(DYNINST_CODEGEN_ARCH_X86_64)
  insnCodeGenX86::generateCall(gen,from,target);
#elif defined(DYNINST_CODEGEN_ARCH_AARCH64)
  insnCodeGenAarch64::generateCall(gen,from,target);
#elif defined(DYNINST_CODEGEN_ARCH_POWER)
  insnCodeGenPower::generateCall(gen,from,target);
#endif
}

void insnCodeGen::generateNOOP(codeGen &gen, unsigned size) {
#if defined(DYNINST_CODEGEN_ARCH_X86_64)
  insnCodeGenX86::generateNOOP(gen,size);
#elif defined(DYNINST_CODEGEN_ARCH_AARCH64)
  insnCodeGenAarch64::generateNOOP(gen,size);
#elif defined(DYNINST_CODEGEN_ARCH_POWER)
  insnCodeGenPower::generateNOOP(gen,size);
#endif
}

void insnCodeGen::generate(codeGen &gen,instruction & insn) {
#if defined(DYNINST_CODEGEN_ARCH_X86_64)
  insnCodeGenX86::generate(gen,insn);
#elif defined(DYNINST_CODEGEN_ARCH_AARCH64)
  insnCodeGenAarch64::generate(gen,insn);
#elif defined(DYNINST_CODEGEN_ARCH_POWER)
  insnCodeGenPower::generate(gen,insn);
#endif
}


bool insnCodeGen::generateMem(codeGen &gen,
                              instruction & insn,
                              Dyninst::Address origAddr,
                              Dyninst::Address newAddr,
                              Dyninst::Register loadExpr,
                              Dyninst::Register storeExpr)
{
#if defined(DYNINST_CODEGEN_ARCH_X86_64)
  return insnCodeGenX86::generateMem(gen,insn,origAddr,newAddr,loadExpr,storeExpr);
#elif defined(DYNINST_CODEGEN_ARCH_AARCH64)
  return insnCodeGenAarch64::generateMem(gen,insn,origAddr,newAddr,loadExpr,storeExpr);
#elif defined(DYNINST_CODEGEN_ARCH_POWER)
  return insnCodeGenPower::generateMem(gen,insn,origAddr,newAddr,loadExpr,storeExpr);
#endif
}


bool insnCodeGen::modifyJump(Dyninst::Address targetAddr, instruction &insn, codeGen &gen) {
#if defined(DYNINST_CODEGEN_ARCH_X86_64)
  return insnCodeGenX86::modifyJump(targetAddr, insn, gen);
#elif defined(DYNINST_CODEGEN_ARCH_AARCH64)
  return insnCodeGenAarch64::modifyJump(targetAddr, insn, gen);
#elif defined(DYNINST_CODEGEN_ARCH_POWER)
  return insnCodeGenPower::modifyJump(targetAddr, insn, gen);
#endif
}

bool insnCodeGen::modifyJcc(Dyninst::Address targetAddr, instruction &insn, codeGen &gen) {
#if defined(DYNINST_CODEGEN_ARCH_X86_64)
  return insnCodeGenX86::modifyJcc(targetAddr, insn, gen);
#elif defined(DYNINST_CODEGEN_ARCH_AARCH64)
  return insnCodeGenAarch64::modifyJcc(targetAddr, insn, gen);
#elif defined(DYNINST_CODEGEN_ARCH_POWER)
  return insnCodeGenPower::modifyJcc(targetAddr, insn, gen);
#endif
}

bool insnCodeGen::modifyCall(Dyninst::Address targetAddr,instruction &insn, codeGen &gen) {
#if defined(DYNINST_CODEGEN_ARCH_X86_64)
  return insnCodeGenX86::modifyCall(targetAddr, insn, gen);
#elif defined(DYNINST_CODEGEN_ARCH_AARCH64)
  return insnCodeGenAarch64::modifyCall(targetAddr, insn, gen);
#elif defined(DYNINST_CODEGEN_ARCH_POWER)
  return insnCodeGenPower::modifyCall(targetAddr, insn, gen);
#endif
}

bool insnCodeGen::modifyData(Dyninst::Address targetAddr, instruction &insn, codeGen &gen)
{
#if defined(DYNINST_CODEGEN_ARCH_X86_64)
  return insnCodeGenX86::modifyData(targetAddr, insn, gen);
#elif defined(DYNINST_CODEGEN_ARCH_AARCH64)
  return insnCodeGenAarch64::modifyData(targetAddr, insn, gen);
#elif defined(DYNINST_CODEGEN_ARCH_POWER)
  return insnCodeGenPower::modifyData(targetAddr, insn, gen);
#endif
}

bool insnCodeGen::modifyDisp(signed long newDisp, NS_x86::instruction &insn, codeGen &gen, Dyninst::Architecture arch, Dyninst::Address addr) {
#if defined(DYNINST_CODEGEN_ARCH_X86_64)
  return insnCodeGenX86::modifyDisp(newDisp, insn, gen, arch, addr);
#else
  (void) newDisp; (void) insn; (void) gen; (void) arch; (void) addr;
  return true;
#endif
}
