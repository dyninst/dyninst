#ifndef DWARFDYNINST_DWARF_CU_INFO_HPP
#define DWARFDYNINST_DWARF_CU_INFO_HPP

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
}}

#endif
