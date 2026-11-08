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

#ifndef DYNINST_AMDGPU_REG_ADAPTER_H
#define DYNINST_AMDGPU_REG_ADAPTER_H

// AMDGPU codegen register adapter: route register classification and ISA-operand derivation through
// MachRegister, so AMDGPU logic does not rely on the codegen `Dyninst::Register`'s `kind`/`count` fields.
// MachRegister already encodes all three pieces:
//   id    -> the register number (val() low byte)
//   kind  -> the category bits (regClass(): SGPR vs VGPR; so SGPR-5 != VGPR-5)
//   block -> the size field (BITS_32 = 1 dword, BITS_64 = pair, ... BITS_512 = 16)
// The codegen Register -> MachRegister build is PER-GEN and lives where the gen is known by type: each
// EmitterAmdgpu<Gen>::toMachRegister(), naming only its own gen's namespace constants (no macro, no arch
// switch). Register-space code derives ISA operand codes through convertRegID (RegisterConversion-amdgpu.C),
// which dispatches on the register's own arch. This adapter operates on an ALREADY-built MachRegister
// (which carries its own arch), so everything here is gen-neutral: classify/encode from that.

#include <cstdint>

#include "dyn_register.h"
#include "registers/MachRegister.h"
#include "registers/abstract_regs.h"
#include "registerSpace/RegisterConversion.h"
#include "dyn_regs.h" // the per-gen MachRegister namespaces (amdgpu_gfx908/90a/940/950)

namespace Dyninst {
namespace amdgpu_adapter {

// AMDGPU SGPR / VGPR (incl. accumulator VGPR) classification from a MachRegister's own arch + category.
// This lives in the codegen adapter, NOT on the general MachRegister: "SGPR vs VGPR" is an AMDGPU-codegen
// question with no cross-arch meaning. Each gen compares against its OWN namespace's category constant (as
// MachRegister::size() does), so no gen is privileged and a gen that renumbers a category is its own case.
inline bool amdgpuIsSgpr(MachRegister mr) {
  switch (mr.getArchitecture()) {
  case Dyninst::Arch_amdgpu_gfx908: return mr.regClass() == Dyninst::amdgpu_gfx908::SGPR;
  case Dyninst::Arch_amdgpu_gfx90a: return mr.regClass() == Dyninst::amdgpu_gfx90a::SGPR;
  case Dyninst::Arch_amdgpu_gfx940: return mr.regClass() == Dyninst::amdgpu_gfx940::SGPR;
  case Dyninst::Arch_amdgpu_gfx950: return mr.regClass() == Dyninst::amdgpu_gfx950::SGPR;
  default: return false;
  }
}
inline bool amdgpuIsVgpr(MachRegister mr) {
  switch (mr.getArchitecture()) {
  case Dyninst::Arch_amdgpu_gfx908:
    return mr.regClass() == Dyninst::amdgpu_gfx908::VGPR || mr.regClass() == Dyninst::amdgpu_gfx908::ACC_VGPR;
  case Dyninst::Arch_amdgpu_gfx90a:
    return mr.regClass() == Dyninst::amdgpu_gfx90a::VGPR || mr.regClass() == Dyninst::amdgpu_gfx90a::ACC_VGPR;
  case Dyninst::Arch_amdgpu_gfx940:
    return mr.regClass() == Dyninst::amdgpu_gfx940::VGPR || mr.regClass() == Dyninst::amdgpu_gfx940::ACC_VGPR;
  case Dyninst::Arch_amdgpu_gfx950:
    return mr.regClass() == Dyninst::amdgpu_gfx950::VGPR || mr.regClass() == Dyninst::amdgpu_gfx950::ACC_VGPR;
  default: return false;
  }
}

// Register number (0..) from a MachRegister.
inline uint32_t amdgpuRegNum(MachRegister mr) {
  return static_cast<uint32_t>(mr.getBaseRegister().val() & 0xff);
}

// Number of 32-bit dwords the register spans (MachRegister::size() is bytes: 4B = 1 dword).
inline uint32_t amdgpuNumDwords(MachRegister mr) { return mr.size() / 4u; }

// The ISA scalar/vector operand code: SGPR -> number; VGPR -> 256 + number; special -> its operand
// code (the existing MachRegister -> codegen-id derivation).
inline uint32_t amdgpuOperandCode(MachRegister mr) {
  if (amdgpuIsSgpr(mr))
    return amdgpuRegNum(mr);
  if (amdgpuIsVgpr(mr))
    return 256u + amdgpuRegNum(mr);
  return static_cast<uint32_t>(convertRegID(mr).getId());
}

} // namespace amdgpu_adapter
} // namespace Dyninst

#endif // DYNINST_AMDGPU_REG_ADAPTER_H
