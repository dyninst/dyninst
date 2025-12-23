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

#include "dyninstAPI/src/codegen.h"
#include "dyninstAPI/src/emit-amdgpu.h"
#include <cstdint>
#include <stdlib.h>

void insnCodeGen::generateTrap(codeGen & /* gen */) {
  // BinaryEdit::addTrap calls this.
  // We currently only do static binary rewriting for AMDGPU and don't need to generate trap
  // instructions.
}

void insnCodeGen::generateIllegal(codeGen & /* gen */) {
  assert(!"Not implemented for AMDGPU");
}

void insnCodeGen::generateBranch(codeGen &gen, Dyninst::Address from, Dyninst::Address to,
                                 bool /* link */) {
  long disp = (to - from);
  long wordOffset = disp / 4;

  Emitter *emitter = gen.emitter();

  if (wordOffset >= INT16_MIN && wordOffset <= INT16_MAX) {
    emitter->emitShortJump(wordOffset, gen);
  } else {
    // TODO: Right now hardcoding s90. But this needs to use 2 pairs of dead registers.
    emitter->emitLongJump(90, from, to, gen);
  }
}

void insnCodeGen::generateCall(codeGen &gen, Dyninst::Address from, Dyninst::Address to) {
  generateBranch(gen, from, to, true);
}

void insnCodeGen::generateNOOP(codeGen &gen, unsigned size) {
  assert((size % instruction::size()) == 0);

  Emitter *emitter = gen.emitter();

  while (size) {
    emitter->emitNops(1, gen);
    size -= instruction::size();
  }
}

// There's nothing to modify here. Generate an emit a jump to target.
bool insnCodeGen::modifyJump(Dyninst::Address target, NS_amdgpu::instruction & /* insn */,
                             codeGen &gen) {
  long disp = target - gen.currAddr();
  Emitter *emitter = gen.emitter();
  int16_t wordOffset = disp / 4;

  assert(wordOffset > INT16_MIN && wordOffset < INT16_MAX &&
         "wordOffset must fit in a 16-bit signed value");
  emitter->emitShortJump(wordOffset, gen);

  return true;
}

/*
 * <Addr X> conditionalJump A
 * <Addr C> nextInstruction  // C = X+4
 *
 * The above is transformed to:
 *
 * <Addr X>   conditionalJump <B>
 * <Addr X+4> jump C
 * <Addr B>   jump A           // B = X+8
 * <Addr C>   nextInstruction  // C = X+12
 *
 */
bool insnCodeGen::modifyJcc(Dyninst::Address target, NS_amdgpu::instruction &insn, codeGen &gen) {
  // We start by modifying the current conditional jump instruction in-place:
  //  <Addr X> conditionalJump A  ---> <Addr X> conditionalJump B
  assert(insn.size() == 4 && "Conditional branch on AMDGPU must be 4 bytes long");
  uint32_t rawInst = insn.asInt();

  // This is a SOPP instruction, so simply set the the SIMM16 field appropriately.
  AmdgpuGfx908::setSImm16SopP(1, rawInst); // GPU does (1)*4 + 4 and computes target = X+8 i.e B

  // Now copy this at the end of the codegen buffer
  gen.copy((void *)&rawInst, sizeof rawInst);

  // We are at offset X+4, we want to jump to X+12, i.e C
  // Now emit jump C
  Emitter *emitter = gen.emitter();
  emitter->emitShortJump(1, gen); // GPU does (1)*4 + 4 and computes target = <X+4> + 8 = X+12 i.e C

  // Now emit jump A, the original target
  long from = gen.currAddr();
  long disp = target - from;
  int16_t wordOffset = disp / 4;

  assert(wordOffset > INT16_MIN && wordOffset < INT16_MAX &&
         "wordOffset must fit in a 16-bit signed value");
  emitter->emitShortJump(wordOffset, gen);

  return true;
}

bool insnCodeGen::modifyCall(Dyninst::Address /* target */, NS_amdgpu::instruction & /* insn */, codeGen & /* gen */) {
  // if (insn.isUncondBranch())
  //   return modifyJump(target, insn, gen);
  // else
  //   return modifyJcc(target, insn, gen);
  assert(!"Not implemented for AMDGPU");
  return false;
}

bool insnCodeGen::modifyData(Dyninst::Address /* target */, NS_amdgpu::instruction & /* insn */,
                             codeGen & /* gen */) {
  assert(!"Not implemented for AMDGPU");
  return false;
}
