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

#include "AmdgpuPrologue.h"
#include "emit-amdgpu.h"

static Register generateLinearBlockId(codeGen &gen) {
  using namespace NS_amdgpu::RegisterConstants;
  EmitterAmdgpuGfx908 emitter;

  // For a 2D grid, it is :
  // LinearBlockId = blockIdx.y * gridDim.x + blockIdx.x
  // gridDim.x is hidden_block_count_x at offset 36
  // blockIdx.x is contained in s6
  // blockIdx.y is contained in s7
  //
  //
  // Say we use s60 = gridDim.x
  // Say we use s61 = s60 * s7
  // LinearBlockId = s62 = s61 + s6
  // Return s62
  //

  // kernarg base = s[4:5]
  Register kernargBaseReg = Register::makeScalarRegister(OperandRegId(4), BlockSize(2));

  emitter.emitLoadRelative(s60, /* offset */32, kernargBaseReg, /* size */ 1, gen);
  emitter.emitOp(timesOp, s61, s60, s7, gen);
  emitter.emitOp(plusOp, s62, s61, s6, gen);
  return s62;
};

static Register generateWavesPerBlock(codeGen &gen) {
  using namespace NS_amdgpu::RegisterConstants;

  EmitterAmdgpuGfx908 emitter;
  //
  // // Waves per block is at index 296
  // // kernarg base = s[4:5]
  // Register kernargBaseReg = Register::makeScalarRegister(OperandRegId(4), BlockSize(2));
  //
  // // s[64:65] = wavesPerBlockPtr
  // // s[64:65] = load s[4:5] + 296
  // // wavesPerBlockReg = s63 = load s[64:65]
  // // Return s63
  //
  // Register s64_65 = Register::makeScalarRegister(OperandRegId(64), BlockSize(2));
  // emitter.emitLoadRelative(s64_65, [> offset */296, kernargBaseReg, /* size <]2, gen);
  // emitter.emitLoadIndir(s63, s64_65, [>size<]1, gen);
  //
  // VectorAdd has 4 wavefronts per block -- hardcoding right now
  emitter.emitMovLiteral(s63, 4, gen);
  return s63;
}

static Register generateLocalWavefrontId(codeGen &gen) {
  using namespace NS_amdgpu::RegisterConstants;

  EmitterAmdgpuGfx908 emitter;

  // LocalWavefrontId = threadId.y * blockDim.x + threadId.x
  // v0 = threadId.x
  // v1 = threadId.y
  //
  // blockDim.x is at offset 44 in the kernarg buffer, is of 2 bytes.
  // blockDim.y is at offset 46.
  // s64 = load s[4:5] + 44
  // s64 = blockDim.y_blockDim.x
  // s65 = s64 & 0x0000FFFF
  //
  // v200 = v1 * s65
  // v201 = v200 + v0
  // v201 contains all global thread ids
  //
  // s66 = readFirstLane v201
  // s66 = s66 / 64
  // return s66
  //

  Register kernargBaseReg = Register::makeScalarRegister(OperandRegId(4), BlockSize(2));
  emitter.emitLoadRelative(s64, /* offset */44, kernargBaseReg, /* size */ 1, gen);
  emitter.emitOpImmSimple(andOp, s65, s64, 0x0000FFFF, gen);
  emitter.emitVMulLoU32(v200, v1, s65, gen);
  emitter.emitOp(plusOp, v201, v200, v0, gen);
  emitter.emitReadFirstLane(v201, s66, gen);
  emitter.emitScalarLogicalRightShift(s66, s66, /*shiftAmt*/6, gen);

  // v201 = v201 & vectorLength
  // v201 = v201 * sizeOfVariable
  const uint32_t vectorLength = 64;
  const uint32_t sizeOfVariable = sizeof (unsigned);
  emitter.emitOpImmSimple(andOp, v201, v201, vectorLength - 1, gen);
  emitter.emitOpImmSimple(timesOp, v201, v201, sizeOfVariable, gen);
  return s66;
}

static Register generateGlobalWavefrontId(Register linearBlockIdReg, Register wavesPerBlockReg, codeGen &gen) {
  using namespace NS_amdgpu::RegisterConstants;

  EmitterAmdgpuGfx908 emitter;

  // globalWavefrontId = linearBlockIdReg * wavesPerBlockReg + localWavefrontId;
  // we use s66 = localWavefrontId;
  // we use s67 = linearBlockIdReg * wavesPerBlockReg
  // s68 = s66 + s67
  // return s68
  Register localWavefrontIdReg = generateLocalWavefrontId(gen);
  emitter.emitOp(timesOp, s67, linearBlockIdReg, wavesPerBlockReg, gen);
  emitter.emitOp(plusOp, s68, s66, s67, gen);
  return s68;
}

bool AmdgpuPrologue::generate(Dyninst::PatchAPI::Point * /* point */, Dyninst::Buffer &buffer) {
  // To avoid any code duplication or refactoring right now, we use a 'codeGen'
  // object to generate the code and copy what we get there into the
  // 'Dyninst::Buffer' object passed here.

  // We need 12 bytes for the prologue (a s_load_dwordx2, followed by a waitcnt)
  //
  codeGen gen(150);
  EmitterAmdgpuGfx908 emitter;

  // The original prologue
  emitter.emitLoadRelative(dest_, offset_, base_, /* size= */ 2, gen);

  // Additional stuff for a global waveId
  Register linearBlockIdReg = generateLinearBlockId(gen);
  Register wavesPerBlockReg = generateWavesPerBlock(gen);
  Register globalWavefrontIdReg = generateGlobalWavefrontId(linearBlockIdReg, wavesPerBlockReg, gen);
  buffer.copy(gen.start_ptr(), gen.used());

  return true;
}
