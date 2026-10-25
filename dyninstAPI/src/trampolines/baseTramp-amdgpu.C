/*
 * See the dyninst/COPYRIGHT file for copyright information.
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

#include "trampolines/baseTramp-amdgpu.h"
#include "codegen/codegen.h"
#include "registerSpace/registerSpace.h"
#include "patching/instPoint.h"
#include "emit-amdgpu.h"
#include "amdgpu-gfx908-details.h"
#include "arch-amdgpu.h"
#include "dyn_register.h"
#include "ABI.h"
#include "registers/AMDGPU/amdgpu_gfx908_regs.h"

using namespace Dyninst;
using namespace AmdgpuGfx908;

// AMDGPU inline-constant source encodings: 0 -> 128, k(1..64) -> 128+k.
static constexpr unsigned INLINE_0 = 128;
static constexpr unsigned INLINE_1 = 129;
static constexpr unsigned NO_SCC_SAVE = 0xFFFFFFFFu;

bool baseTramp_amdgpu::generateSaves(codeGen &gen, registerSpace *rs) {
  sccSaveReg_ = NO_SCC_SAVE;

  instPoint *iP = point();
  if (!iP)
    return true;

  // Is SCC live at this point? liveRegisters() is indexed by the (GFX9-codegen) gfx908 ABI, the same
  // ABI that resolves src_scc's index, so the lookup is consistent.
  int sccIdx = ABI::getABI(8)->getIndex(amdgpu_gfx908::src_scc);
  if (sccIdx < 0)
    return true;  // SCC not in the liveness index (older build): nothing we can decide
  bitArray live = iP->liveRegisters();
  if (!live[sccIdx])
    return true;  // SCC dead across the snippet: no need to preserve it

  // SCC is live: stash it in a scratch SGPR before the snippet runs.
  Register tmp = rs->allocateGprBlock(RegKind::SCALAR, 1, NS_amdgpu::SINGLE_ALIGNMENT);
  if (tmp == Null_Register)
    return true;  // no free scratch SGPR: cannot save (high-pressure; left for the SpillContext work)
  sccSaveReg_ = tmp.getId();

  // s_cselect_b32 tmp, 1, 0  ->  tmp = SCC ? 1 : 0   (reads SCC into an SGPR)
  emitSop2Raw(S_CSELE3CT_B32, sccSaveReg_, INLINE_1, INLINE_0, gen);
  return true;
}

bool baseTramp_amdgpu::generateRestores(codeGen &gen, registerSpace * /*rs*/) {
  if (sccSaveReg_ == NO_SCC_SAVE)
    return true;

  // s_and_b32 tmp, tmp, 1  ->  tmp = tmp & 1 ; SCC = (tmp != 0) = the original SCC
  emitSop2Raw(S_AND_B32, sccSaveReg_, sccSaveReg_, INLINE_1, gen);
  sccSaveReg_ = NO_SCC_SAVE;
  return true;
}
