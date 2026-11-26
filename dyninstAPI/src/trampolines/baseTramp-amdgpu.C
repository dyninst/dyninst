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

#include "trampolines/baseTramp-amdgpu.h"
#include "ABI.h"
#include "amdgpu-gfx908-details.h"
#include "arch-amdgpu.h"
#include "codegen/codegen.h"
#include "patching/instPoint.h"
#include "registerSpace/registerSpace.h"
#include "registers/AMDGPU/amdgpu_gfx908_regs.h"

// The registers a snippet uses come from the register space, which only hands out registers
// that are dead at the point, so they need no save. SCC is different: the snippet body never
// allocates it, but writes it as a side effect of every SALU add, logical operation and compare,
// and there is no other SCC to use instead. So when SCC is live at the point, it is copied to a
// dead SGPR before the body and written back after it:
//
//   s_cselect_b32 sN, 1, 0     ; sN = SCC
//   <snippet body>
//   s_and_b32     sN, sN, 1    ; SCC = (sN != 0)

namespace {
  // SOP2 source encodings of the inline constants 0 and 1.
  constexpr Dyninst::Register inlineConst0{128};
  constexpr Dyninst::Register inlineConst1{129};
}

bool baseTramp_amdgpu::generateSaves(codeGen &gen, registerSpace *rs) {
  sccSave_ = Dyninst::Null_Register;

  instPoint *iP = point();
  if (!iP) {
    return true;
  }

  // instPoint::liveRegisters() is indexed by the 64-bit ABI's register map.
  int const sccIndex = ABI::getABI(8)->getIndex(Dyninst::amdgpu_gfx908::src_scc);
  assert(sccIndex >= 0 && "src_scc must be in the AMDGPU liveness register map");
  if (!iP->liveRegisters()[sccIndex]) {
    return true;
  }

  sccSave_ = rs->allocateGprBlock(Dyninst::RegKind::SCALAR, 1, NS_amdgpu::SINGLE_ALIGNMENT);
  assert(sccSave_ != Dyninst::Null_Register && "no free SGPR to preserve the live SCC");

  AmdgpuGfx908::emitSop2(AmdgpuGfx908::S_CSELE3CT_B32, sccSave_.getId(), inlineConst1, inlineConst0, gen);
  return true;
}

bool baseTramp_amdgpu::generateRestores(codeGen &gen, registerSpace *rs) {
  if (sccSave_ == Dyninst::Null_Register) {
    return true;
  }

  AmdgpuGfx908::emitSop2(AmdgpuGfx908::S_AND_B32, sccSave_.getId(), sccSave_.getId(), inlineConst1, gen);
  rs->freeRegister(sccSave_);
  sccSave_ = Dyninst::Null_Register;
  return true;
}
