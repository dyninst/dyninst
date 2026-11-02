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

// M1 proof-of-concept adapter: route AMDGPU codegen's register classification and ISA-operand
// derivation through MachRegister, so the codegen `Dyninst::Register`'s `kind`/`count` fields are not
// relied on by AMDGPU logic. MachRegister already encodes all three pieces:
//   id    -> the register number (val() low byte)
//   kind  -> the category bits (regClass(): SGPR vs VGPR; so SGPR-5 != VGPR-5)
//   block -> the size field (BITS_32 = 1 dword, BITS_64 = pair, ... BITS_512 = 16)
// `Dyninst::Register` remains only as the shared Emitter-interface boundary type; amdgpuMachRegOf()
// adapts it to a MachRegister at entry, and everything downstream classifies/encodes from that.
//
// The adapter is arch-GENERAL: amdgpuMachRegOf() takes the architecture from the caller (the per-gen
// emitter), so it is not bound to gfx908 -- it dispatches to each gen's own register constants.

#include <cstdint>

#include "dyn_register.h"
#include "registers/MachRegister.h"
#include "registers/abstract_regs.h"
#include "registerSpace/RegisterConversion.h"
#include "dyn_regs.h" // the per-gen MachRegister namespaces (amdgpu_gfx908/90a/940/950)

namespace Dyninst {
namespace amdgpu_adapter {

// Adapt a codegen Register to a MachRegister of the GIVEN arch: scalar/vector class from the kind, number
// from the id, width (in 32-bit dwords) from the block count. This is the one place the codegen Register's
// fields are read. The AMDGPU register-value encoding lives in the AMDGPU codegen layer (here), not on the
// generic MachRegister. PER-GEN: each arch uses its OWN namespace's category + size-field constants (as
// MachRegister::size() does), so a gen that renumbers them is handled by its own case; `arch`
// selects the gen. Bit ranges are disjoint: arch 0xff000000 | category 0x00ff0000 | size 0x0000ff00 |
// number 0x000000ff.
inline MachRegister amdgpuMachRegOf(Register r, Dyninst::Architecture arch) {
  const bool isVec = r.isVector();
  const int32_t num = static_cast<int32_t>(r.getId() & 0xffu);
  const uint32_t dwords = r.getCount();
#define DYNINST_AMDGPU_MK_REG(NS)                                                                          \
  {                                                                                                        \
    const int32_t category = isVec ? Dyninst::NS::VGPR : Dyninst::NS::SGPR;                                 \
    int32_t sizeBits = Dyninst::NS::BITS_32;                                                                \
    switch (dwords) {                                                                                      \
    case 2: sizeBits = Dyninst::NS::BITS_64; break;                                                         \
    case 4: sizeBits = Dyninst::NS::BITS_128; break;                                                        \
    case 8: sizeBits = Dyninst::NS::BITS_256; break;                                                        \
    case 16: sizeBits = Dyninst::NS::BITS_512; break;                                                       \
    default: break; /* 1 (and unexpected) -> BITS_32 */                                                     \
    }                                                                                                      \
    return MachRegister(static_cast<int32_t>(arch) | category | sizeBits | num);                            \
  }
  switch (arch) {
  case Dyninst::Arch_amdgpu_gfx908: DYNINST_AMDGPU_MK_REG(amdgpu_gfx908)
  case Dyninst::Arch_amdgpu_gfx90a: DYNINST_AMDGPU_MK_REG(amdgpu_gfx90a)
  case Dyninst::Arch_amdgpu_gfx940: DYNINST_AMDGPU_MK_REG(amdgpu_gfx940)
  case Dyninst::Arch_amdgpu_gfx950: DYNINST_AMDGPU_MK_REG(amdgpu_gfx950)
  default: return MachRegister();
  }
#undef DYNINST_AMDGPU_MK_REG
}

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
