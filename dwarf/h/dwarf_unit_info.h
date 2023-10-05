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

namespace Dyninst { namespace DwarfDyninst {
  // We purposefully don't include DW_TAG_skeleton_unit here as
  // libdw should merge those into a single CU for us.
  inline bool is_full_unit(Dwarf_Die die) { return dwarf_tag(&die) == DW_TAG_compile_unit; }
  inline bool is_partial_unit(Dwarf_Die die) { return dwarf_tag(&die) == DW_TAG_partial_unit; }
  inline bool is_type_unit(Dwarf_Die die) { return dwarf_tag(&die) == DW_TAG_type_unit; }
  inline bool is_imported_unit(Dwarf_Die die) { return dwarf_tag(&die) == DW_TAG_imported_unit; }
}}

#endif
