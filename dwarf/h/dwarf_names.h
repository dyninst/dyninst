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

#ifndef DWARFDYNINST_DWARF_NAMES_HPP
#define DWARFDYNINST_DWARF_NAMES_HPP

#include <dwarf.h>
#include <elfutils/libdw.h>
#include <iomanip>
#include <string>
#include <sstream>
#include "dwarf_unit_info.h"

namespace Dyninst { namespace DwarfDyninst {

  namespace detail {

    inline std::string die_name(Dwarf_Die die) {
      auto name = dwarf_diename(&die);
      if (!name)
        return {}; // can't make a std::string from nullptr
      return name;
    }

    inline std::string absolute_path(std::string const &filename, std::string const &base) {
      // If base is empty, don't make any conversion
      if (base.empty()) {
        return filename;
      }

      if (filename.find('/') == 0UL) {
        // It starts with a leading slash, so assume it's already absolute
        return filename;
      }
      return base + "/" + filename;
    }

    inline std::string comp_dir_name(Dwarf_Die cuDie) {
      Dwarf_Attribute attr;
      const char *comp_dir = dwarf_formstring(dwarf_attr(&cuDie, DW_AT_comp_dir, &attr));
      if (!comp_dir)
        return {};
      return comp_dir;
    }

    inline std::string die_offset(Dwarf_Die die) {
      auto off_die = dwarf_dieoffset(&die);
      std::stringstream suffix;
      suffix << std::hex << off_die;
      return "0x" + suffix.str();
    }
  }

  inline bool is_anonymous_die(Dwarf_Die die) { return !dwarf_hasattr(&die, DW_AT_name); }

  inline bool is_artificial_die(Dwarf_Die die) {
    bool has_art_attr = dwarf_hasattr(&die, DW_AT_artificial);

    // Some compilers name the DIE `<artificial>` and some provide the
    // DW_AT_artificial attribute, so check both
    return detail::die_name(die) == "<artificial>" || has_art_attr;
  }

  inline std::string die_name(Dwarf_Die die) {

    auto name = detail::die_name(die);

    // For artificial DIEs or partial units, append the DIE's location to its name (if any).
    // For C++ member functions, compilers will sometimes add a DW_AT_name called 'this',
    // and we don't want to mangle that.
    if (name.empty() && (is_artificial_die(die) || is_partial_unit(die))) {
      name += "(" + detail::die_offset(die) + ")";
      return name;
    }

    return name;
  }
}}

#endif
