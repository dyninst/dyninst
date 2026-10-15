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

#include <iterator>

//clang-format: off
#define DYN_DEFINE_REGS
#include "dyn_regs.h"
#undef DYN_DEFINE_REGS
//clang-format: on

// The same lists again, as tables of each register's value and name, in
// the order dyn_regs.h includes them.  The names are only needed when
// something asks for one, so the registry is built from these then.
#undef DEF_REGISTER
#undef DEF_REGISTER_ALIAS
#define DEF_REGISTER(name, value) {i##name, #name},
#define DEF_REGISTER_ALIAS(name, target) {i##name, #name},

//clang-format: off
namespace Dyninst {
namespace {
  const registers::reg_row register_table[] = {
#include "registers/abstract_regs.def"
  };
}
namespace x86 { namespace {
  const registers::reg_row register_table[] = {
#include "registers/x86_regs.def"
  };
}}
namespace x86_64 { namespace {
  const registers::reg_row register_table[] = {
#include "registers/x86_64_regs.def"
  };
}}
namespace ppc32 { namespace {
  const registers::reg_row register_table[] = {
#include "registers/ppc32_regs.def"
  };
}}
namespace ppc64 { namespace {
  const registers::reg_row register_table[] = {
#include "registers/ppc64_regs.def"
  };
}}
namespace aarch64 { namespace {
  const registers::reg_row register_table[] = {
#include "registers/aarch64_regs.def"
  };
}}
namespace riscv64 { namespace {
  const registers::reg_row register_table[] = {
#include "registers/riscv64_regs.def"
  };
}}
namespace amdgpu_gfx908 { namespace {
  const registers::reg_row register_table[] = {
#include "registers/AMDGPU/amdgpu_gfx908_regs.def"
  };
}}
namespace amdgpu_gfx90a { namespace {
  const registers::reg_row register_table[] = {
#include "registers/AMDGPU/amdgpu_gfx90a_regs.def"
  };
}}
namespace amdgpu_gfx940 { namespace {
  const registers::reg_row register_table[] = {
#include "registers/AMDGPU/amdgpu_gfx940_regs.def"
  };
}}
namespace amdgpu_gfx950 { namespace {
  const registers::reg_row register_table[] = {
#include "registers/AMDGPU/amdgpu_gfx950_regs.def"
  };
}}
namespace cuda { namespace {
  const registers::reg_row register_table[] = {
#include "registers/cuda_regs.def"
  };
}}
}
//clang-format: on
#undef DEF_REGISTER
#undef DEF_REGISTER_ALIAS

namespace {
  struct table_ref {
    const Dyninst::registers::reg_row *first, *last;
  };

  // Definition order, which decides the order of each architecture's
  // registers and, for an alias, which name a value reports.
  const table_ref all_tables[] = {
    {std::begin(Dyninst::register_table), std::end(Dyninst::register_table)},
    {std::begin(Dyninst::x86::register_table), std::end(Dyninst::x86::register_table)},
    {std::begin(Dyninst::x86_64::register_table), std::end(Dyninst::x86_64::register_table)},
    {std::begin(Dyninst::ppc32::register_table), std::end(Dyninst::ppc32::register_table)},
    {std::begin(Dyninst::ppc64::register_table), std::end(Dyninst::ppc64::register_table)},
    {std::begin(Dyninst::aarch64::register_table), std::end(Dyninst::aarch64::register_table)},
    {std::begin(Dyninst::riscv64::register_table), std::end(Dyninst::riscv64::register_table)},
    {std::begin(Dyninst::amdgpu_gfx908::register_table), std::end(Dyninst::amdgpu_gfx908::register_table)},
    {std::begin(Dyninst::amdgpu_gfx90a::register_table), std::end(Dyninst::amdgpu_gfx90a::register_table)},
    {std::begin(Dyninst::amdgpu_gfx940::register_table), std::end(Dyninst::amdgpu_gfx940::register_table)},
    {std::begin(Dyninst::amdgpu_gfx950::register_table), std::end(Dyninst::amdgpu_gfx950::register_table)},
    {std::begin(Dyninst::cuda::register_table), std::end(Dyninst::cuda::register_table)},
  };
}

Dyninst::registers::registry &Dyninst::registers::get_registry()
{
  // A function-local static is initialized exactly once, even when
  // several threads ask at the same time.
  static registry *reg_info = []()  {
    registry *r = new registry;
    for (const table_ref &t : all_tables)  {
      for (const reg_row *row = t.first; row != t.last; ++row)  {
        MachRegister m(row->value);
        r->names.emplace(row->value, row->name);
        r->all_regs[m.getArchitecture()].push_back(m);
      }
    }
    return r;
  }();
  return *reg_info;
}
