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

#include "registers/MachRegisterCache.h"

//clang-format: off
#define DYN_DEFINE_REGS
#include "dyn_regs.h"
#undef DYN_DEFINE_REGS
//clang-format: on

// The same lists again, as one table of each register's value and name, in
// the order dyn_regs.h includes them.  That order decides the order of each
// architecture's registers and, for an alias, which name a value reports.
// The names are only needed when something asks for one, so the registry is
// built from this table then.
//
// Each list is expanded with its architecture's namespace in DYN_REG_NS,
// since the same register name is defined by several architectures.
#undef DEF_REGISTER
#undef DEF_REGISTER_ALIAS
#define DEF_REGISTER(name, value) {DYN_REG_NS::i##name, #name},
#define DEF_REGISTER_ALIAS(name, target) {DYN_REG_NS::i##name, #name},

namespace {
  //clang-format: off
  const Dyninst::registers::reg_row register_table[] = {
#define DYN_REG_NS Dyninst
#include "registers/abstract_regs.def"
#undef DYN_REG_NS
#define DYN_REG_NS Dyninst::x86
#include "registers/x86_regs.def"
#undef DYN_REG_NS
#define DYN_REG_NS Dyninst::x86_64
#include "registers/x86_64_regs.def"
#undef DYN_REG_NS
#define DYN_REG_NS Dyninst::ppc32
#include "registers/ppc32_regs.def"
#undef DYN_REG_NS
#define DYN_REG_NS Dyninst::ppc64
#include "registers/ppc64_regs.def"
#undef DYN_REG_NS
#define DYN_REG_NS Dyninst::aarch64
#include "registers/aarch64_regs.def"
#undef DYN_REG_NS
#define DYN_REG_NS Dyninst::riscv64
#include "registers/riscv64_regs.def"
#undef DYN_REG_NS
#define DYN_REG_NS Dyninst::amdgpu_gfx908
#include "registers/AMDGPU/amdgpu_gfx908_regs.def"
#undef DYN_REG_NS
#define DYN_REG_NS Dyninst::amdgpu_gfx90a
#include "registers/AMDGPU/amdgpu_gfx90a_regs.def"
#undef DYN_REG_NS
#define DYN_REG_NS Dyninst::amdgpu_gfx940
#include "registers/AMDGPU/amdgpu_gfx940_regs.def"
#undef DYN_REG_NS
#define DYN_REG_NS Dyninst::amdgpu_gfx950
#include "registers/AMDGPU/amdgpu_gfx950_regs.def"
#undef DYN_REG_NS
#define DYN_REG_NS Dyninst::cuda
#include "registers/cuda_regs.def"
#undef DYN_REG_NS
  };
  //clang-format: on
}
#undef DEF_REGISTER
#undef DEF_REGISTER_ALIAS

Dyninst::registers::registry const &Dyninst::registers::get_registry()
{
  // A function-local static is initialized exactly once, even when
  // several threads ask at the same time.
  static const registry reg_info = []()  {
    registry r;
    for (const reg_row &row : register_table)  {
      MachRegister m(row.value);
      r.names.emplace(row.value, row.name);
      r.all_regs[m.getArchitecture()].push_back(m);
    }
    return r;
  }();
  return reg_info;
}
