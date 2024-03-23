.. _`sec:dwarfExprParser.h`:

dwarfExprParser.h
#################

.. cpp:namespace:: Dyninst::DwarfDyninst

.. cpp:function:: int Register_DWARFtoMachineEnc32(int n)
.. cpp:function:: int Register_DWARFtoMachineEnc64(int n)
.. cpp:function:: bool decodeDwarfExpression(Dwarf_Op *expr, Dwarf_Sword listlen, long int *initialStackValue, Dyninst::VariableLocation &loc, Dyninst::Architecture arch)
.. cpp:function:: bool decodeDwarfExpression(Dwarf_Op *expr, Dwarf_Sword listlen, long int *initialStackValue, DwarfResult &res, Dyninst::Architecture arch)
