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

// AMDGPU codegen Register <-> MachRegister conversion, dispatched on the register's OWN architecture.
//
// Unlike x86/aarch64/ppc (whose register ids are irregular and need a hand-written map), the AMDGPU
// register space is a regular identity, so we DERIVE the mapping instead of hand-listing it:
//  - SGPR/VGPR: the codegen id is the ISA operand code -- the register number for an SGPR (s33 = 33),
//    VGPR_OPERAND_BASE + the register number for a VGPR (v5 = 261). The SGPR/VGPR category bits are
//    identical across every AMDGPU gen, so this part is arch-invariant.
//  - specials (vcc/exec/flat_scratch/xnack/m0/ttmp): the codegen id is the ISA scalar operand code
//    (vcc_lo = 106, exec_lo = 126, ...), read from the GENERATED PER-ARCH table <arch>::detail::specialRegs[].
//    That table is the single source for those codes and is per-gen, so a gen that renumbers or drops a
//    special (the first being gfx1250) is captured by vendoring its table — not by editing this file.
//
// We dispatch on the register's own architecture (reg.getArchitecture() / the `arch` argument); we do
// NOT normalize every gen to one arch. SGPR/VGPR conversion already works for every AMDGPU gen; the
// special-register table is wired per gen as each gen's encoding header is vendored. Only gfx908's table
// is vendored today (its header is also what the emitter uses); gfx90a/gfx942/gfx950 are byte-identical
// to it right now (verified vs llvm-mc) and get wired after the gfx942 rename lands, so the
// AmdgpuGfx942::enc namespace lines up with Arch_amdgpu_gfx942.
#include "registerSpace/RegisterConversion.h"

#include <cstdint>
#include <map>
#include <string>

#include "Architecture.h"
#include "Register.h"
#include "registerSpace/registerSpace.h"
#include "registers/MachRegister.h"
#include "registers/abstract_regs.h"
#include "codegen/emitters/amdgpu/gfx908/amdgpu-gfx908-encoding.h"
#include "codegen/emitters/amdgpu/gfx90a/amdgpu-gfx90a-encoding.h"
#include "codegen/emitters/amdgpu/gfx940/amdgpu-gfx940-encoding.h"
#include "codegen/emitters/amdgpu/gfx950/amdgpu-gfx950-encoding.h"
#include <iostream>

using namespace Dyninst;
using namespace Dyninst::InstructionAPI;
using namespace std;

namespace {

// The ISA scalar operand code for a special register named `nm` on `arch`, from THAT gen's generated
// table. Returns false if `nm` is not a listed special (a plain SGPR/VGPR) or if `arch`'s table is not
// vendored yet. Each arm scans its own gen's table so the codes stay per-gen (see the file header).
bool amdgpuSpecialCode(Architecture arch, const std::string &nm, uint32_t &codeOut) {
  switch (arch) {
    case Arch_amdgpu_gfx908:
      for (const auto &sr : Dyninst::amdgpu_gfx908::detail::specialRegs)
        if (nm == sr.name) {
          codeOut = sr.code;
          return true;
        }
      return false;
    case Arch_amdgpu_gfx90a:
      for (const auto &sr : Dyninst::amdgpu_gfx90a::detail::specialRegs)
        if (nm == sr.name) {
          codeOut = sr.code;
          return true;
        }
      return false;
    case Arch_amdgpu_gfx940:
      for (const auto &sr : Dyninst::amdgpu_gfx940::detail::specialRegs)
        if (nm == sr.name) {
          codeOut = sr.code;
          return true;
        }
      return false;
    case Arch_amdgpu_gfx950:
      for (const auto &sr : Dyninst::amdgpu_gfx950::detail::specialRegs)
        if (nm == sr.name) {
          codeOut = sr.code;
          return true;
        }
      return false;
    default:
      return false;
  }
}

// Per-arch reverse data, built once per architecture from that arch's OWN registered MachRegisters:
// the v0/s0 base encodings (for the SGPR/VGPR identity) and the special operand-code -> MachRegister map
// (intersecting the arch's registers with its generated specialRegs[] codes). No fixed-arch constants.
struct AmdgpuArchRegs {
  int32_t v0Val = 0;
  int32_t s0Val = 0;
  bool haveBase = false;
  std::map<uint32_t, MachRegister> specialByCode;
};

const AmdgpuArchRegs &amdgpuArchRegs(Architecture arch) {
  static std::map<Architecture, AmdgpuArchRegs> cache;
  std::map<Architecture, AmdgpuArchRegs>::iterator it = cache.find(arch);
  if (it != cache.end())
    return it->second;

  AmdgpuArchRegs d;
  bool haveV0 = false, haveS0 = false;
  for (const MachRegister &mr : MachRegister::getAllRegistersForArch(arch)) {
    const std::string nm = mr.name();
    if (nm == "v0") {
      d.v0Val = mr.val();
      haveV0 = true;
    } else if (nm == "s0") {
      d.s0Val = mr.val();
      haveS0 = true;
    }
    uint32_t code;
    if (amdgpuSpecialCode(arch, nm, code))
      d.specialByCode.emplace(code, mr);
  }
  d.haveBase = haveV0 && haveS0;
  return cache.emplace(arch, std::move(d)).first->second;
}

} // namespace

Register convertRegID(MachRegister reg) {
  const MachRegister baseReg = reg.getBaseRegister();
  const Architecture arch = baseReg.getArchitecture();

  // SGPR/VGPR: arch-invariant category + register-number identity (no normalization to a fixed arch).
  // The category bits are the same value on every AMDGPU gen, so any gen's v0/s0 is a valid probe.
  const unsigned category = baseReg.regClass();
  const uint32_t num = (uint32_t)(baseReg.val() & 0xffu);
  if (category == amdgpu_gfx908::v0.regClass())
    return NS_amdgpu::VGPR_OPERAND_BASE + num;
  if (category == amdgpu_gfx908::s0.regClass())
    return num;

  // Specials: codegen id = ISA operand code, from this register's own arch table.
  uint32_t code;
  if (amdgpuSpecialCode(arch, baseReg.name(), code))
    return code;

  // Not an encodable register (trash from bad code), or an arch whose special table is not vendored yet.
  return Null_Register;
}

Register convertRegID(RegisterAST::Ptr toBeConverted) { return convertRegID(toBeConverted.get()); }

Register convertRegID(RegisterAST *toBeConverted) {
  if (!toBeConverted) {
    return Null_Register;
  }
  return convertRegID(toBeConverted->getID());
}

MachRegister convertRegID(Register r, Dyninst::Architecture arch) {
  if (!isArchitectureAMDGPU(arch)) {
    assert(!"convertRegID: non-AMDGPU architecture");
    return InvalidReg;
  }

  const AmdgpuArchRegs &d = amdgpuArchRegs(arch);
  const uint32_t id = r;

  // VGPR / SGPR: the codegen id is the operand code, so the MachRegister is this arch's v0/s0 + number.
  if (id >= NS_amdgpu::VGPR_OPERAND_BASE && id <= NS_amdgpu::VGPR_OPERAND_BASE + NS_amdgpu::MAX_VGPR_ID) {
    if (d.haveBase)
      return MachRegister(d.v0Val + (int32_t)(id - NS_amdgpu::VGPR_OPERAND_BASE));
  } else if (id <= NS_amdgpu::MAX_SGPR_ID) {
    if (d.haveBase)
      return MachRegister(d.s0Val + (int32_t)id);
  } else {
    // Special scalar (operand code >= 102): look the code up in this arch's generated table.
    std::map<uint32_t, MachRegister>::const_iterator found = d.specialByCode.find(id);
    if (found != d.specialByCode.end())
      return found->second;
  }

  std::cerr << "convertRegID : register not handled\n";
  return InvalidReg;
}
