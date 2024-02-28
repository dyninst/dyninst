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

#include "debug.h"
#include "interrupts.h"

namespace di = Dyninst::InstructionAPI;

namespace x86 {
  bool isSoftwareInterrupt(di::Instruction const& ins) {
    auto id = ins.getOperation().getID();
    switch(id) {
      case e_int:
      case e_int1:
      case e_into:
      case e_int3:
        return true;
      default:
        return false;
    }
  }
}

namespace ppc {
  bool isSoftwareInterrupt(di::Instruction const&) {
    return false;
  }
}

namespace aarch64 {
  bool isSoftwareInterrupt(di::Instruction const&) {
    return false;
  }
}

bool di::isSoftwareInterrupt(Instruction const& ins) {
  switch(ins.getArch()) {
    case Arch_x86:
    case Arch_x86_64:
      return ::x86::isSoftwareInterrupt(ins);
    case Arch_ppc32:
    case Arch_ppc64:
      return ::ppc::isSoftwareInterrupt(ins);
    case Arch_aarch64:
      return ::aarch64::isSoftwareInterrupt(ins);
    case Arch_none:
    case Arch_aarch32:
    case Arch_cuda:
    case Arch_amdgpu_gfx908:
    case Arch_amdgpu_gfx90a:
    case Arch_amdgpu_gfx940:
    case Arch_intelGen9:
      return false;
  }
  return false;
}
