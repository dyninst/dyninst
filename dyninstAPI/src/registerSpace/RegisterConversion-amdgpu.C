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

// AMDGPU codegen Register <-> MachRegister conversion.
//
// Unlike x86/aarch64/ppc (whose register ids are irregular and need a hand-written map), the AMDGPU
// register space is a regular identity: an SGPR/VGPR's codegen id is its register number, which is also
// its MachRegister base number (s33 = number 33), and a special register's codegen id is its ISA scalar
// operand code (vcc_lo = 106, exec_lo = 126, ...). So we DERIVE the mapping instead of hand-listing it:
//  - SGPR/VGPR: codegen id = MachRegister base number, kind from the register class.
//  - specials (vcc/exec/flat_scratch/xnack/m0/ttmp): codegen id = ISA operand code, read from the
//    generated per-arch table amdgpu_gfx908_enc::specialRegs[] (the single source for those codes; it is
//    per-arch, so it tracks AMD dropping/renumbering a register, e.g. gfx1250, without edits here).
// This replaces the former 366-entry hand-written regToMachReg32 / reverseRegisterMap tables.
#include "registerSpace/RegisterConversion.h"

#include <cstdint>
#include <map>
#include <string>

#include "Register.h"
#include "registerSpace/registerSpace.h"
#include "registers/MachRegister.h"
#include "registers/abstract_regs.h"
#include "amdgpu_gfx908_encoding.h"
#include <iostream>

using namespace Dyninst;
using namespace Dyninst::InstructionAPI;
using namespace std;

namespace {

// The ISA scalar operand code for a special register named `nm`, from the generated per-arch table;
// returns false if `nm` is not a listed special (i.e. a plain SGPR/VGPR or an unhandled register).
bool amdgpuSpecialCode(const std::string &nm, uint32_t &codeOut) {
  for (const auto &sr : amdgpu_gfx908_enc::specialRegs) {
    if (nm == sr.name) {
      codeOut = sr.code;
      return true;
    }
  }
  return false;
}

} // namespace

Register convertRegID(MachRegister reg) {
  // Normalize any CDNA arch (gfx90a/942/950) to the gfx908 register identities we key on.
  MachRegister baseReg =
      MachRegister((reg.getBaseRegister().val() & ~reg.getArchitecture()) | Arch_amdgpu_gfx908);

  const unsigned category = baseReg.regClass();
  const uint32_t num = (uint32_t)(baseReg.val() & 0xffu);

  if (category == amdgpu_gfx908::v0.regClass())
    return Register(OperandRegId(num), RegKind::VECTOR, BlockSize(1));
  if (category == amdgpu_gfx908::s0.regClass())
    return Register(OperandRegId(num), RegKind::SCALAR, BlockSize(1));

  // Special registers (vcc/exec/flat_scratch/xnack/m0/ttmp) are scalar operands; their codegen id is
  // the ISA operand code, not their MachRegister base number.
  uint32_t code;
  if (amdgpuSpecialCode(baseReg.name(), code))
    return Register(OperandRegId(code), RegKind::SCALAR, BlockSize(1));

  // Not a register we encode (e.g. trash from analyzing bad code) -> invalid/ignored.
  return Register();
}

Register convertRegID(RegisterAST::Ptr toBeConverted) { return convertRegID(toBeConverted.get()); }

Register convertRegID(RegisterAST *toBeConverted) {
  if (!toBeConverted) {
    return Register();
  }
  return convertRegID(toBeConverted->getID());
}

MachRegister convertRegID(Register r, Dyninst::Architecture arch) {
  if (arch != Arch_amdgpu_gfx908) {
    assert(!"Invalid architecture");
    return InvalidReg;
  }

  const uint32_t id = r.getId();

  // VGPR / SGPR: the MachRegister base number is the codegen id, so build it from v0/s0 + id.
  if (r.getKind() == RegKind::VECTOR)
    return MachRegister(amdgpu_gfx908::v0.val() + (int32_t)id);
  if (id <= NS_amdgpu::MAX_SGPR_ID)
    return MachRegister(amdgpu_gfx908::s0.val() + (int32_t)id);

  // Special scalar (operand code >= 102): the MachRegister base number is NOT the code, so look it up
  // by matching the generated specialRegs[] codes against the registered MachRegisters for this arch.
  // Built once; no hand-listed constants.
  static const std::map<uint32_t, MachRegister> specialByCode = [] {
    std::map<uint32_t, MachRegister> m;
    for (const MachRegister &mr : MachRegister::getAllRegistersForArch(Arch_amdgpu_gfx908)) {
      uint32_t code;
      if (amdgpuSpecialCode(mr.name(), code))
        m.emplace(code, mr);
    }
    return m;
  }();

  std::map<uint32_t, MachRegister>::const_iterator found = specialByCode.find(id);
  if (found != specialByCode.end())
    return found->second;

  std::cerr << "convertRegID : register not handled";
  return InvalidReg;
}
