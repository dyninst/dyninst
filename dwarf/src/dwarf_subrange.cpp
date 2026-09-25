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

static bool constant_value(Dwarf_Attribute *attr, bool as_signed,
                           Dwarf_Word &value) {
  if (as_signed) {
    Dwarf_Sword signed_value;
    if (dwarf_formsdata(attr, &signed_value) != 0)
      return false;
    value = signed_value;
    return true;
  }
  return dwarf_formudata(attr, &value) == 0;
}

/*
 * DWARF5 - Section 2.19 Static and Dynamic Values of Attributes
 *
 * The bound and count attributes of a subrange may be a constant, a DWARF
 * expression, or a reference to a DIE that describes a constant, describes a
 * variable holding the value, or computes it. A constant, directly or through
 * a reference, is the value; the other cases are runtime values, and are
 * reported as found but with no value.
 */
static dwarf_result subrange_attr(Dwarf_Die *die, unsigned int name,
                                  bool as_signed) {
  Dwarf_Attribute attr;
  if (!dwarf_attr_integrate(die, name, &attr)) {
    // Nothing was found, but there was no error
    return dwarf_result{};
  }

  Dwarf_Word value;
  if (constant_value(&attr, as_signed, value))
    return value;

  // The constant decoders fail the same way for a runtime value and for a
  // form that is invalid here, so check for the runtime forms explicitly.
  Dwarf_Block block;
  if (dwarf_formblock(&attr, &block) == 0)
    return dwarf_result{};

  // A reference that can't be resolved, such as one into a supplementary
  // (dwz) file that isn't available, is an error.
  Dwarf_Die ref;
  if (!dwarf_formref_die(&attr, &ref))
    return dwarf_error{};

  Dwarf_Attribute ref_value;
  if (dwarf_attr_integrate(&ref, DW_AT_const_value, &ref_value) &&
      constant_value(&ref_value, is_signed(&ref), value))
    return value;
  return dwarf_result{};
}

static dwarf_result upper_bound(Dwarf_Die *die) {
  return subrange_attr(die, DW_AT_upper_bound, is_signed(die));
}

static dwarf_result lower_bound(Dwarf_Die *die) {
  return subrange_attr(die, DW_AT_lower_bound, is_signed(die));
}

static dwarf_result lower_bound_by_language(Dwarf_Die *die) {
  // DW_AT_language is only on the unit DIE, and dwarf_srclang reads the DIE
  // it is given.
  Dwarf_Die cu_die;
  int lang = dwarf_srclang(dwarf_diecu(die, &cu_die, nullptr, nullptr));
  Dwarf_Sword lower;
  if (lang != -1 && dwarf_default_lower_bound(lang, &lower) == 0)
    return lower;

  // No language, or one libdw has no default for; the caller decides
  return dwarf_result{};
}

static dwarf_result length_from_count(Dwarf_Die *die) {
  return subrange_attr(die, DW_AT_count, false);
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
