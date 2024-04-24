.. _`sec:dwarf_unit_info.h`:

dwarf_unit_info.h
#################

.. cpp:namespace:: Dyninst::DwarfDyninst

We purposefully don't include DW_TAG_skeleton_unit here as libdw should merge
those into a single CU for us.

.. cpp:function:: inline bool is_full_unit(Dwarf_Die die) 
.. cpp:function:: inline bool is_partial_unit(Dwarf_Die die) 
.. cpp:function:: inline bool is_type_unit(Dwarf_Die die) 
.. cpp:function:: inline bool is_imported_unit(Dwarf_Die die) 
