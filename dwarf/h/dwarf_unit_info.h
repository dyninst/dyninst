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

#ifndef DWARF_UNIT_INFO_H
#define DWARF_UNIT_INFO_H

#include <dwarf.h>
#include <elfutils/libdw.h>

namespace Dyninst { namespace DwarfDyninst {
  // We purposefully don't include DW_TAG_skeleton_unit here as
  // libdw should merge those into a single CU for us.
  inline bool is_full_unit(Dwarf_Die die) { return dwarf_tag(&die) == DW_TAG_compile_unit; }
  inline bool is_partial_unit(Dwarf_Die die) { return dwarf_tag(&die) == DW_TAG_partial_unit; }
  inline bool is_type_unit(Dwarf_Die die) { return dwarf_tag(&die) == DW_TAG_type_unit; }
  inline bool is_imported_unit(Dwarf_Die die) { return dwarf_tag(&die) == DW_TAG_imported_unit; }

  /*
   * Is `die` in .debug_info rather than .debug_types?
   *
   * DIE offsets are relative to the section holding them, so an offset alone does not
   * identify a DIE: a .debug_info DIE and a .debug_types DIE in the same file routinely
   * share one. Anything keying DIEs by offset has to key by this as well.
   *
   * Note that this is not the same question as "is `die` in a type unit". Only DWARF4 has
   * a separate .debug_types section; DWARF5 moved type units into .debug_info, so a DWARF5
   * type unit is in .debug_info.
   */
  inline bool is_in_debug_info(Dwarf_Die die) {
    Dwarf_Half version{};
    uint8_t unit_type{};
    if(!die.cu || dwarf_cu_info(die.cu, &version, &unit_type, NULL, NULL, NULL, NULL, NULL) != 0) {
      return true; // Unknown; .debug_info is by far the common case.
    }
    return !(version < 5 && unit_type == DW_UT_type);
  }

  /*
   * Is `die` inside a type unit, in either .debug_types (DWARF4) or .debug_info (DWARF5)?
   */
  inline bool is_in_type_unit(Dwarf_Die die) {
    uint8_t unit_type{};
    if(!die.cu || dwarf_cu_info(die.cu, NULL, &unit_type, NULL, NULL, NULL, NULL, NULL) != 0) {
      return false;
    }
    return unit_type == DW_UT_type;
  }

  /*
   * The 8-byte type signature of the type unit headed by `die`, as DW_FORM_ref_sig8
   * spells it. Returns false, leaving `signature` untouched, if `die` does not head a
   * type unit.
   */
  inline bool type_unit_signature(Dwarf_Die die, uint64_t &signature) {
    uint8_t unit_type{};
    uint64_t unit_id{};
    if(!die.cu || dwarf_cu_info(die.cu, NULL, &unit_type, NULL, NULL, &unit_id, NULL, NULL) != 0) {
      return false;
    }
    if(unit_type != DW_UT_type) {
      return false;
    }
    signature = unit_id;
    return true;
  }
}}

#endif
