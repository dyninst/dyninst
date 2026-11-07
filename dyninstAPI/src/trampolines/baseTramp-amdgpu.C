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
#include "codegen/emitters/amdgpu/emit-amdgpu.h"
#include "arch-amdgpu.h"
#include "dyn_register.h"
#include "ABI.h"
#include "registers/AMDGPU/amdgpu_gfx908_regs.h"

using namespace Dyninst;

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
  Register tmp = rs->allocateRegister(gen, registerSlot::SGPR, 1, /* alignment */ 1);
  if (tmp == Null_Register)
    return true;  // no free scratch SGPR: cannot save (high-pressure; left for the SpillContext work)
  sccSaveReg_ = tmp;

  // s_cselect_b32 tmp, 1, 0  ->  tmp = SCC ? 1 : 0   (reads SCC into an SGPR)
  getAmdgpuEmitter(gen, iP)->emitSccSave(sccSaveReg_, gen);
  return true;
}

bool baseTramp_amdgpu::generateRestores(codeGen &gen, registerSpace * /*rs*/) {
  if (sccSaveReg_ == NO_SCC_SAVE)
    return true;

  // sccSaveReg_ is only set when generateSaves had a non-null point, so point() is valid here.
  instPoint *iP = point();
  assert(iP && "baseTramp_amdgpu::generateRestores: SCC saved but no point to restore at");

  // s_and_b32 tmp, tmp, 1  ->  tmp = tmp & 1 ; SCC = (tmp != 0) = the original SCC
  getAmdgpuEmitter(gen, iP)->emitSccRestore(sccSaveReg_, gen);
  sccSaveReg_ = NO_SCC_SAVE;
  return true;
}
