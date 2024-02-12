.. _`sec:dwarf_names.h`:

dwarf_names.h
#############

.. cpp:namespace:: Dyninst::DwarfDyninst::detail

.. cpp:function:: inline std::string die_name(Dwarf_Die die)
.. cpp:function:: inline std::string absolute_path(std::string const &filename, std::string const &base)

  Returns the absolute path of ``filename`` relative to ``base``

  We could use ``boost::filesystem::absolute`` here, but we don't need to pay the cost of its flexibility
  for multiple path separators since DWARF is currently only on Unix-like platforms.

.. cpp:function:: inline std::string comp_dir_name(Dwarf_Die cuDie)

  Returns the compilation directory for the CU

  Returns an empty string if not found

.. cpp:function:: inline std::string die_offset(Dwarf_Die die)

  Returns a string representation of the DIEs offset


.. cpp:namespace:: Dyninst::DwarfDyninst

.. cpp:function:: inline bool is_anonymous_die(Dwarf_Die die)

  Checks if the die is anonymous

  True if it has no DW_AT_name attribute. This only checks if the immediate die has a name. We don't care if any
  of its parents have a name.

.. cpp:function:: inline bool is_artificial_die(Dwarf_Die die)

  Checks if ``die`` has been marked as artificial.

  From the DWARF5 standard (2.11 Artificial Entries):

    A compiler may wish to generate debugging information entries for objects or types that were
    not actually declared in the source of the application.

.. cpp:function:: inline std::string die_name(Dwarf_Die die)

  Returns the name of the die referred to by ``die``.

  If ``die`` is artificial, a unique name is returned. If this case is important to the caller, then
  :cpp:func:`is_artificial_die` should be checked. Anonymous DIEs are purposefully left unnamed because
  of explicit checks in :cpp:func:`Dyninst::SymtabAPI::DwarfWalker::nameDefined`.
