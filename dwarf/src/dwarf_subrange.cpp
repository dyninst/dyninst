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

#include "dwarf_subrange.h"

namespace {

Dwarf_Die *get_type(Dwarf_Die *die, Dwarf_Die *result) {
  Dwarf_Attribute scratch_attr;
  dwarf_attr_integrate(die, DW_AT_type, &scratch_attr);
  Dwarf_Die *type = dwarf_formref_die(&scratch_attr, result);

  if (!type || dwarf_peel_type(type, type) != 0)
    return nullptr;

  return type;
}

bool is_signed(Dwarf_Die *die) {
  Dwarf_Attribute attr;
  Dwarf_Die scratch;
  if (dwarf_attr(get_type(die, &scratch), DW_AT_encoding, &attr)) {
    Dwarf_Word encoding;
    if (dwarf_formudata(&attr, &encoding) == 0)
      return encoding == DW_ATE_signed || encoding == DW_ATE_signed_char;
  }
  return false;
}
} // namespace

namespace Dyninst {
namespace DwarfDyninst {

static dwarf_result upper_bound(Dwarf_Die *die) {
  Dwarf_Attribute attr;
  if (dwarf_attr_integrate(die, DW_AT_upper_bound, &attr)) {
    if (is_signed(die)) {
      Dwarf_Sword upper;
      if (dwarf_formsdata(&attr, &upper) != 0)
        return dwarf_error{};
      return upper;
    }
    Dwarf_Word unsigned_upper;
    if (dwarf_formudata(&attr, &unsigned_upper) != 0)
      return dwarf_error{};
    return unsigned_upper;
  }

  // Nothing was found, but there was no error
  return dwarf_result{};
}

static dwarf_result lower_bound(Dwarf_Die *die) {
  Dwarf_Attribute attr;
  if (dwarf_attr_integrate(die, DW_AT_lower_bound, &attr)) {
    if (is_signed(die)) {
      Dwarf_Sword lower;
      if (dwarf_formsdata(&attr, &lower) != 0)
        return dwarf_error{};
      return lower;
    }
    Dwarf_Word unsigned_lower;
    if (dwarf_formudata(&attr, &unsigned_lower) != 0)
      return dwarf_error{};
    return unsigned_lower;
  }

  // Nothing was found, but there was no error
  return dwarf_result{};
}

static dwarf_result lower_bound_by_language(Dwarf_Die *die) {
  int lang = dwarf_srclang(die);
  if (lang != -1) {
    Dwarf_Sword lower;
    if (dwarf_default_lower_bound(lang, &lower) != 0)
      return dwarf_error{};
    return lower;
  }

  // Nothing was found, but there was no error
  return dwarf_result{};
}

static dwarf_result length_from_count(Dwarf_Die *die) {
  Dwarf_Attribute attr;
  if (dwarf_attr_integrate(die, DW_AT_count, &attr)) {
    Dwarf_Word count;
    if (dwarf_formudata(&attr, &count) != 0)
      return dwarf_error{};
    return count;
  }

  // Nothing was found, but there was no error
  return dwarf_result{};
}

dwarf_bounds dwarf_subrange_bounds(Dwarf_Die *die) {
  /*
   * DWARF5 - Section 5.13 Subrange Type Entries
   *
   * The subrange entry may have the attributes DW_AT_lower_bound and
   * DW_AT_upper_bound to specify, respectively, the lower and upper
   * bound values of the subrange.
   */
  dwarf_bounds bounds{lower_bound(die), upper_bound(die)};

  // Don't continue if we encountered an error in either lookup
  if (!bounds.lower || !bounds.upper) {
    return bounds;
  }

  // If the lower bound value is missing, the value is assumed to
  // be a language-dependent default constant
  if (!bounds.lower.value) {
    bounds.lower = lower_bound_by_language(die);

    // If there was a dwarf error, bail out
    if (!bounds.lower) {
      return bounds;
    }
  }

  /*
   * The DW_AT_upper_bound attribute may be replaced by a DW_AT_count
   * attribute, whose value describes the number of elements in the
   * subrange rather than the value of the last element.
   */
  auto count = length_from_count(die);

  // If there was a dwarf error, explicitly mark the upper bound as bad
  if (!count) {
    bounds.upper = dwarf_error{};
    return bounds;
  }

  if (count.value) {
    auto const lb = [&bounds]() {
      // If we have a lower bound, use it
      if (bounds.lower.value) return bounds.lower.value.get();

      // Otherwise, assume the array is zero-based
      return static_cast<Dwarf_Word>(0);
    }();
    bounds.upper = dwarf_result{lb + count.value.get() - static_cast<Dwarf_Word>(1)};
  }

  // If the upper bound and count are missing, then the upper bound value is unknown.
  return bounds;
}

dwarf_result dwarf_subrange_length_from_enum(Dwarf_Die *die) {
  /* We have to find the DW_TAG_enumerator child with the
     highest value to know the array's element count.  */
  Dwarf_Die enum_child;
  int has_children = dwarf_child(die, &enum_child);
  if (has_children < 0)
    return dwarf_error{};
  if (has_children > 0) {
    Dwarf_Attribute attr;
    Dwarf_Word count{};
    do {
      if (dwarf_tag(&enum_child) == DW_TAG_enumerator) {
        dwarf_attr_integrate(&enum_child, DW_AT_const_value, &attr);
        Dwarf_Word value;
        if (dwarf_formudata(&attr, &value) != 0)
          return dwarf_error{};
        if (value >= count)
          count = value + 1;
      }
    } while (dwarf_siblingof(&enum_child, &enum_child) > 0);
    return count;
  }

  // Nothing was found, but there was no error
  return dwarf_result{};
}
} // namespace DwarfDyninst
} // namespace Dyninst
