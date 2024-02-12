.. _`sec:dwarf_cu_info.h`:

dwarf_cu_info.h
###############

.. cpp:namespace:: Dyninst::DwarfDyninst

.. cpp:function:: inline bool is_cudie(Dwarf_Die die)

.. cpp:function:: inline bool is_parseable_unit(Dwarf_Die die)

  Checks if ``die`` corresponds to a DWARF unit that should be parsed

  DW_TAG_imported_unit may need to be included here, but is currently handled separately
  in :cpp:func:`Dyninst::SymtabAPI::DwarfWalker::parse_int`.

.. cpp:function:: inline std::string cu_dirname(Dwarf_Die cuDie)

  Returns the name of the directory where the source file was compiled.

  If present, it is always an absolute path. Returns an empty string if not found.

.. cpp:function:: inline std::string cu_name(Dwarf_Die cuDie)

  Returns the name of the compilation unit (CU) referred to by `cuDie`

  This is the name of the source file used to create the CU (e.g., test.cpp) made into an absolute path
  (if present, see below). The user MUST ensure that ``cuDie`` refers to a CU (see :cpp:func:`dwarf_is_cudie`
  above). Otherwise, the returned name is nonsense. It would be very weird- but technically possible- for
  a CU die to be artificial. In that case, a unique name is generated. If the CU has no name- which would be
  even weirder- a unique one is created for it.

.. cpp:function:: inline Dwarf_Die *find_cu(Dwarf *dbg, Dwarf_Addr addr, Dwarf_Die *result)

  Returns the compilation unit (CU) die that starts at the address `addr`.

