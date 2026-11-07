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


#include "patching/block.h"
#include "patching/function.h"
#include "codegen/codegen.h"
#include "codegen/emitters/amdgpu/emit-amdgpu.h"
#include "dyninstAPI/src/registerSpace/registerSpace.h"
#include "registerSpace/registerSlot.h"
#include "mapped_object.h"
#include "AmdgpuKernelDescriptor.h"
#include "amdgpu-abi-sgpr.h"
#include "amdgpu-kernel-meta.h"
#include <cstdint>
#include <cstdio>
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
  // S_BRANCH target = (PC_of_branch + 4) + SIMM16*4, so SIMM16 = disp/4 - 1
  // (the -1 is the +4 in dword units; same for forward and backward jumps).
  long wordOffset = disp / 4 - 1;

  Emitter *emitter = gen.emitter();

  if (wordOffset >= INT16_MIN && wordOffset <= INT16_MAX) {
    emitter->emitShortJump(wordOffset, gen);
  } else {
    auto as = gen.addrSpace();
    block_instance *blockInstance = as->findBlockByEntry(from);
    assert(blockInstance);

    func_instance *funcInstance = blockInstance->entryOfFunc();
    if (!funcInstance) {
      // FIXME:
      // This happens because springboard generates jumps twice and in the second round the basic block
      // doesn't have a parent function.
      //
      // The branches are inserted in the first time the code is generated.
      return;
    }

    // The jump is placed at the start of the block, so its scratch registers must be dead
    // there. Registers that are live into the block but dead at its end (e.g. a workgroup ID
    // the block reads and then overwrites) must not be picked.
    instPoint *blockEntryPoint = instPoint::blockEntry(funcInstance, blockInstance);
    assert(blockEntryPoint);

    registerSpace *regSpace = registerSpace::actualRegSpace(blockEntryPoint);
    assert(regSpace);

    // Reserve the SGPRs the call path keeps live so the springboard's s_getpc veneer does not
    // clobber them. Dyninst's point liveness marks the ORIGINAL live inputs (kernarg etc.), but it
    // does not know (a) that enabling scratch shifts the system SGPRs up, nor (b) that emitCall
    // reserves a spill block just below the special regs (reservedBase = grantTop-10). Read the
    // caller kernel's ABI layout + grant from the canonical KernelMeta and mark both ranges live so
    // the long-jump allocation (below) places the pairs clear of them. (Root-caused on gfx908: the
    // springboard grabbed s[8:9], the pair emitCall reserves for the kernarg pointer.)
    {
      mapped_object *fobj = funcInstance->obj();
      Dyninst::DyninstAPI::KernelMeta *km =
          fobj ? fobj->getAmdgpuKernelMeta(funcInstance->symTabName()) : nullptr;
      if (km) {
        const Dyninst::DyninstAPI::AbiSgprLayout &L = km->layout();
        // ARCHITECTED flat scratch (gfx942/CDNA3): no Private Segment Buffer, so the kernarg ptr is
        // the FIRST user SGPRs (s[0:1]) and IS read by the entry prologue; scratchEnabled() is false.
        // Reserve the full ABI live-in range [0, liveSgprEnd) there so the kernarg pointer survives.
        // gfx908: kernarg sits at s[4:5]+ and s[0:3] (Private Segment Buffer) is dead at entry, so
        // the system-SGPR-only reservation [userSgprCount, liveSgprEnd) is correct.
        const bool architected = km->kd.supportsArchitectedFlatScratch();
        if (L.scratchEnabled() || architected) {
          const uint32_t liveStart = architected ? 0u : L.userSgprCount;
          for (uint32_t n = liveStart; n < L.liveSgprEnd; n++) {
            // A codegen Register is the SGPR operand code, which is the number itself.
            Dyninst::Register r = n;
            if (registerSlot *slot = (*regSpace)[r])
              slot->liveState = registerSlot::live;
          }
          // Also reserve emitCall's SGPR spill block: reservedBase = grantTop-10, and the top 6
          // SGPRs are special (VCC + XNACK gap + FLAT_SCRATCH per LLVM getNumExtraSGPRs), so the
          // 4-reg spill block sits at [grantTop-10, grantTop-6). These survive the whole kernel, so
          // the long-jump pairs must not land on them.
          const uint32_t grantTop = km->grantedSgprTop();
          if (grantTop >= 10) {
            for (uint32_t n = grantTop - 10; n < grantTop - 6; n++) {
              Dyninst::Register r = n;
              if (registerSlot *slot = (*regSpace)[r])
                slot->liveState = registerSlot::live;
            }
          }
        }
      }
    }

    // The long jump needs two independent 64-bit SGPR pairs (the s_getpc/s_setpc PC pair and the
    // diff-constant pair). Allocate two pairs rather than one contiguous 4-register block: each is
    // independently even-aligned, and 2+2 non-contiguous SGPRs can be found where 4 contiguous cannot.
    Dyninst::Register pcPair = regSpace->allocateRegister(gen, registerSlot::SGPR, /* numRegs */ 2, /* alignment */ 2);
    Dyninst::Register diffPair = regSpace->allocateRegister(gen, registerSlot::SGPR, /* numRegs */ 2, /* alignment */ 2);
    if (pcPair == Dyninst::Null_Register || diffPair == Dyninst::Null_Register) {
      fprintf(stderr, "AMDGPU generateBranch: no SGPR pairs (pair-aligned) for a long jump from %#lx to %#lx\n",
              static_cast<unsigned long>(from), static_cast<unsigned long>(to));
      assert(false && "no SGPR pairs for a long jump");
    }
    emitter->emitLongJump(pcPair, diffPair, from, to, gen);
    regSpace->freeRegister(diffPair, /* numRegs */ 2);
    regSpace->freeRegister(pcPair, /* numRegs */ 2);
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
  // S_BRANCH target = (PC_of_branch + 4) + SIMM16*4, so SIMM16 = disp/4 - 1.
  int16_t wordOffset = disp / 4 - 1;

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
  Dyninst::amdgpu_gfx908::setSImm16SopP(1, rawInst); // GPU does (1)*4 + 4 and computes target = X+8 i.e B

  // Now copy this at the end of the codegen buffer
  gen.copy((void *)&rawInst, sizeof rawInst);

  // We are at offset X+4, we want to jump to X+12, i.e C
  // Now emit jump C
  Emitter *emitter = gen.emitter();
  emitter->emitShortJump(1, gen); // GPU does (1)*4 + 4 and computes target = <X+4> + 8 = X+12 i.e C

  // Now emit jump A, the original target.
  // S_BRANCH target = (PC_of_branch + 4) + SIMM16*4, so SIMM16 = disp/4 - 1;
  // without the -1 the branch lands one instruction past the target (e.g. a loop
  // back-edge would skip the loop head's first instruction).
  long from = gen.currAddr();
  long disp = target - from;
  int16_t wordOffset = disp / 4 - 1;

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
