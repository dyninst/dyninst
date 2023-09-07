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

#ifndef DWARFDYNINST_DWARF_CU_INFO_HPP
#define DWARFDYNINST_DWARF_CU_INFO_HPP

#include "dwarf_names.h"
#include <dwarf.h>
#include <elfutils/libdw.h>
#include <string>

namespace Dyninst { namespace DwarfDyninst {

  inline bool is_fullcu(Dwarf_Die die) { return dwarf_tag(&die) == DW_TAG_compile_unit; }
  inline bool is_partialcu(Dwarf_Die die) { return dwarf_tag(&die) == DW_TAG_partial_unit; }
  inline bool is_typecu(Dwarf_Die die) { return dwarf_tag(&die) == DW_TAG_type_unit; }

  inline bool is_cudie(Dwarf_Die die) {
    // If there is not an inner CU attribute, then it's not a CU
    if (!die.cu)
      return false;

    // These are best guess. Ideally, we'd like to interrogate
    // the internals of the die, but that's not currently possible
    // with libdw. The internal function there is `is_cudie`.
    //
    // We purposefully don't include DW_TAG_skeleton_unit here as
    // libdw should merge those into a single CU for us.
    return is_fullcu(die) || is_partialcu(die) || is_typecu(die);
  }

  /*
   * The name of the compilation unit (CU) referred to by `cuDie`
   *
   *   This is the name of the source file used to create the CU (e.g., test.cpp)
   *   made into an absolute path (if present, see below).
   *
   * The user MUST ensure that `cuDie` refers to a CU (see `dwarf_is_cudie` above). Otherwise,
   * the returned name is nonsense. It would be very weird- but technically possible- for
   * a CU die to be artificial. In that case, a unique name is generated.
   *
   * If the CU has no name- which would be even weirder- a unique one is created for it.
   *
   */
  inline std::string cu_name(Dwarf_Die cuDie) {

    // This takes care of the DW_AT_artificial case for us.
    auto name = DwarfDyninst::die_name(cuDie);

    // Make the name absolute wrt the compilation directory
    return detail::absolute_path(name, detail::comp_dir_name(cuDie));
  }
}}

#endif
