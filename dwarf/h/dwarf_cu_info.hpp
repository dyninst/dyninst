#ifndef DWARFDYNINST_DWARF_CU_INFO_HPP
#define DWARFDYNINST_DWARF_CU_INFO_HPP

#include "dwarf_names.hpp"
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

    // This takes care of the anonymous/unnamed die and DW_AT_artificial cases for us.
    auto name = DwarfDyninst::die_name(cuDie);

    // Make the name absolute wrt the compilation directory
    return detail::absolute_path(name, detail::comp_dir_name(cuDie));
  }
}}

#endif
