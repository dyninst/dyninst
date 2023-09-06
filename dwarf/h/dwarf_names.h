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

namespace Dyninst { namespace DwarfDyninst {

  namespace detail {

    inline std::string die_name(Dwarf_Die die) {
      auto name = dwarf_diename(&die);
      if (!name)
        return {}; // can't make a std::string from nullptr
      return name;
    }

    /* The absolute path of `filename` relative to `base`
     *
     *  We could use boost::filesystem::absolute here, but we don't need to pay the cost of
     *  its flexibility for multiple path separators since DWARF is currently only on
     *  Unix-like platforms.
     */
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

    /* The compilation directory for the CU
     *
     *  Returns an empty string if not found
     */
    inline std::string comp_dir_name(Dwarf_Die cuDie) {
      Dwarf_Attribute attr;
      const char *comp_dir = dwarf_formstring(dwarf_attr(&cuDie, DW_AT_comp_dir, &attr));
      if (!comp_dir)
        return {};
      return comp_dir;
    }

    /*
     *  Make a string representation of the DIEs offset
     */
    inline std::string die_offset(Dwarf_Die die) {
      auto off_die = dwarf_dieoffset(&die);
      std::stringstream suffix;
      suffix << std::hex << off_die;
      return suffix.str();
    }
  }

  /* Check if the die is anonymous
   *
   *   True if it has no DW_AT_name attribute
   *
   *   This only checks if the immediate die has a name. We
   *   don't care if any of its parents have a name.
   */
  inline bool is_anonymous_die(Dwarf_Die die) { return !dwarf_hasattr(&die, DW_AT_name); }

  /*  Detect if the current DIE has been marked as artificial
   *
   *   From the DWARF5 standard (2.11 Artificial Entries):
   *
   *     A compiler may wish to generate debugging information entries for objects
   *     or types that were not actually declared in the source of the application.
   */
  inline bool is_artificial_die(Dwarf_Die die) {
    bool has_art_attr = dwarf_hasattr(&die, DW_AT_artificial);

    // Some compilers name the DIE `<artificial>` and some provide the
    // DW_AT_artificial attribute, so check both
    return detail::die_name(die) == "<artificial>" || has_art_attr;
  }

  /* The name of the die referred to by `die`
   *
   *   If the `die` is artificial or anonymous, a unique name
   *   is returned. If these cases are important to the caller,
   *   then `is_artificial` or `is_anonymous` should be checked.
   */
  inline std::string die_name(Dwarf_Die die) {

    if (is_anonymous_die(die)) {
      // No name, make a unique one
      return "{ANONYMOUS}(" + detail::die_offset(die) + ")";
    }

    auto name = detail::die_name(die);

    // There is no standard for naming artificial DIEs, so we just
    // append the DIE's location to it.
    if (is_artificial_die(die)) {
      name += "(" + detail::die_offset(die) + ")";
      return name;
    }

    return name;
  }
}}

#endif
