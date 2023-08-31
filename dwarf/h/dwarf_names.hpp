#ifndef DWARFDYNINST_DWARF_NAMES_HPP
#define DWARFDYNINST_DWARF_NAMES_HPP

#include <dwarf.h>
#include <elfutils/libdw.h>
#include <iomanip>
#include <sstream>
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

    /* Retrieve the type unit signature
     *
     *  A die that represents a type unit (see 3.1.4 Type Unit Entries in the DWARF
     *  standard) has no name (i.e., no DW_AT_name). Instead, it is referred to by
     *  the hashed contents of its reference signature (see 7.32 Type Signature
     *  Computation in the standard).
     */
    inline char const* type_unit_signature(Dwarf_Die die) {
      Dwarf_Attribute attr{};
      bool is_type_unit = dwarf_attr_integrate(&die, DW_TAG_type_unit, &attr);
      if (!is_type_unit)
        return {};
      if (dwarf_whatform(&attr) != DW_FORM_ref_sig8)
        return {};
      char const *name = dwarf_formstring(&attr);
      if (!name)
        return {};
      return name;
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
   *   is returned. If the DIE is associated with a type info entry,
   *   the returned name is the string representation of the signature
   *   (see 7.32 Type Signature Computation of the DWARF standard for
   *   details).
   *
   *   If these cases are important to the caller, then `is_artificial_die`,
   *   `is_anonymous_die`, or `is_typecu` (from dwarf_cu_info.hpp) should
   *   be checked before calling this function.
   */
  inline std::string die_name(Dwarf_Die die) {
    auto name = detail::die_name(die);

    // There is no standard for naming artificial DIEs, so we just
    // append the DIE's location to it.
    if (is_artificial_die(die)) {
      name += "(" + detail::die_offset(die) + ")";
      return name;
    }

    char const* sig_name = detail::type_unit_signature(die);
    if (sig_name) {
      return sig_name;
    }

    // This needs to be done last because type unit CUs don't have names
    if (is_anonymous_die(die)) {
      // No name, make a unique one
      return "{ANONYMOUS}(" + detail::die_offset(die) + ")";
    }

    return name;
  }
}}

#endif
