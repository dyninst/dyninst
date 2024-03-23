.. _`sec-dev:symtab_impl.hpp`:

symtab_impl.hpp
###############

.. cpp:namespace:: Dyninst::SymtabAPI::dev

.. cpp:struct:: symtab_impl final

  .. cpp:member:: indexed_symbols everyDefinedSymbol
  .. cpp:member:: indexed_symbols undefDynSyms
  .. cpp:member:: indexed_modules modules
  .. cpp:member:: std::once_flag funcRangesAreParsed
  .. cpp:member:: std::once_flag types_parsed
  .. cpp:member:: dyn_c_hash_map<Offset, Function *> funcsByOffset

    Since Functions are unique by address, we require this structure to efficiently track them.

  .. cpp:type:: VarsByOffsetMap = dyn_c_hash_map<Offset, std::vector<Variable *>>
  .. cpp:member:: VarsByOffsetMap varsByOffset
  .. cpp:type:: ModRangeLookup = IBSTree<ModRange>
  .. cpp:member:: ModRangeLookup mod_lookup_
  .. cpp:type:: FuncRangeLookup = IBSTree<FuncRange>
  .. cpp:member:: FuncRangeLookup func_lookup
  .. cpp:member:: Module* default_module
  .. cpp:function:: Module* getContainingModule(Offset offset) const
