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
#include "dyninstAPI/src/emit-amdgpu.h"
#include "dyninstAPI/src/registerSpace/registerSpace.h"
#include "registerSpace/registerSlot.h"
#include "mapped_object.h"
#include "AmdgpuKernelDescriptor.h"
#include "amdgpu-abi-sgpr.h"
#include "external/amdgpu/AMDGPUEFlags.h"
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

    instPoint *blockExitPoint = instPoint::blockExit(funcInstance, blockInstance);
    assert(blockExitPoint);

    registerSpace *regSpace = registerSpace::actualRegSpace(blockExitPoint);
    assert(regSpace);

    // The entry springboard's long-jump grabs a 4-SGPR block via s_getpc. Dyninst's
    // liveness marks the ORIGINAL live inputs (kernarg etc.) live so the allocator
    // skips them, but when we ENABLE scratch on a kernel that shipped without it
    // (DYNINST_SPILL_SCRATCH), the flat_scratch_init user-SGPR pair shifts the
    // SYSTEM SGPRs up (wgid/wave_offset land at s[user_sgpr_count..]). Those hold
    // live per-wave inputs at kernel entry, but the original liveness doesn't know
    // they moved, so allocateGprBlock would hand them to the springboard and the
    // s_getpc would clobber the scratch wave-offset BEFORE the prologue captures it
    // (→ all waves share one scratch base → multi-wave corruption). Reserve that
    // live system-SGPR range so the springboard allocates above it.
    if (getenv("DYNINST_SPILL_SCRATCH")) {
      mapped_object *fobj = funcInstance->obj();
      int_symbol kdSym;
      if (fobj &&
          fobj->getSymbolInfo(funcInstance->symTabName() + ".kd", kdSym)) {
        const size_t kdSize = sizeof(llvm::amdhsa::kernel_descriptor_t);
        uint8_t kdBytes[sizeof(llvm::amdhsa::kernel_descriptor_t)];
        if (as->readDataSpace(reinterpret_cast<const void *>(kdSym.getAddr()),
                              static_cast<u_int>(kdSize), kdBytes, true)) {
          Dyninst::AmdgpuKernelDescriptor kd(kdBytes, kdSize,
                                             EF_AMDGPU_MACH_AMDGCN_GFX908);
          Dyninst::DyninstAPI::AbiSgprLayout L =
              Dyninst::DyninstAPI::computeAbiSgprLayout(kd);
          if (L.scratchEnabled()) {
            for (uint32_t n = L.userSgprCount; n < L.liveSgprEnd; n++) {
              Dyninst::Register r = Dyninst::Register::makeScalarRegister(
                  Dyninst::OperandRegId(n), Dyninst::BlockSize(1));
              registerSlot *slot = (*regSpace)[r];
              if (slot)
                slot->liveState = registerSlot::live;
            }
            // Also reserve the trampoline's SGPR spill block. In scratch mode the
            // grant is sized tight (maximizeSgprAllocationIfKernel) and emitCall
            // places EXEC-save + SADDR just BELOW the special regs: reservedBase =
            // grantTop-10. The wavefront SGPR count INCLUDES the special regs, and
            // when FLAT_SCRATCH is used that's SIX at the top (VCC + XNACK slot +
            // FLAT_SCRATCH, per LLVM getNumExtraSGPRs) — even with xnack- the xnack
            // pair stays a reserved gap, so treat all 6 (grantTop-6..grantTop-1) as
            // off-limits and put the 4-reg block below them. These must survive the
            // whole kernel, so the springboard/call-target block this long-jump
            // allocates must not land on them. (With the old maxed grant the block sat
            // at s[92:95], far above anything the allocator picked; a tight grant
            // brings it into reach, so reserve it.)
            const uint32_t grantTop =
                ((kd.getCOMPUTE_PGM_RSRC1_GranulatedWavefrontSgprCount() / 2) + 1) * 16;
            if (grantTop >= 10) {
              for (uint32_t n = grantTop - 10; n < grantTop - 6; n++) {
                Dyninst::Register r = Dyninst::Register::makeScalarRegister(
                    Dyninst::OperandRegId(n), Dyninst::BlockSize(1));
                registerSlot *slot = (*regSpace)[r];
                if (slot)
                  slot->liveState = registerSlot::live;
              }
            }
          }
        }
      }
    }

    // We relax the alignment to pair because we will use the 4 registers as 2 pairs.
    Dyninst::Register regBlock = regSpace->allocateGprBlock(Dyninst::RegKind::SCALAR, /* numRegs */4, NS_amdgpu::PAIR_ALIGNMENT);
    emitter->emitLongJump(regBlock, from, to, gen);
    regSpace->freeGprBlock(regBlock);
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
  AmdgpuGfx908::setSImm16SopP(1, rawInst); // GPU does (1)*4 + 4 and computes target = X+8 i.e B

  // Now copy this at the end of the codegen buffer
  gen.copy((void *)&rawInst, sizeof rawInst);

  // We are at offset X+4, we want to jump to X+12, i.e C
  // Now emit jump C
  Emitter *emitter = gen.emitter();
  emitter->emitShortJump(1, gen); // GPU does (1)*4 + 4 and computes target = <X+4> + 8 = X+12 i.e C

  // Now emit jump A, the original target.
  // S_BRANCH computes its destination as (PC_of_branch + 4) + SIMM16*4, so the
  // encoded SIMM16 must be (disp/4 - 1), NOT disp/4. Without the -1 the branch
  // lands ONE instruction (4 bytes) past the target. For a loop back-edge that
  // skips the loop head's first instruction — e.g. it dropped the induction
  // variable's decrement out of the loop, giving an infinite loop.
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
