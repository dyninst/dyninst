Developer Notes
###############

Call hierarchy for :cpp:func:`createDefaultModule`
==================================================

This is not a complete call graph. It has been truncated for readability.

.. code-block:: shell

  Dyninst::SymtabAPI::Symtab::createDefaultModule() : void - /dyninst/symtabAPI/src/Symtab.C
    Dyninst::SymtabAPI::Symtab::getDefaultModule() : Module * - /dyninst/symtabAPI/src/Symtab-lookup.C
      BinaryEdit::makeInitAndFiniIfNeeded() : void - /dyninst/dyninstAPI/src/unix.C (2 matches)
      DynCFGFactory::mkfunc(Address, enum FuncSource, string, CodeObject *, CodeRegion *, InstructionSource *) : Function * - /dyninst/dyninstAPI/src/Parsing.C (2 matches)
      Dyninst::SymtabAPI::DwarfParseActions::setModuleFromName(string) : void - /dyninst/symtabAPI/src/dwarfWalker.C
        Dyninst::SymtabAPI::DwarfWalker::parseModule(Dwarf_Die, Module * &) : bool - /dyninst/symtabAPI/src/dwarfWalker.C
          Dyninst::SymtabAPI::DwarfWalker::parse() : bool - /dyninst/symtabAPI/src/dwarfWalker.C (2 matches)
            Dyninst::SymtabAPI::Object::parseTypeInfo() : void - /dyninst/symtabAPI/src/Object-elf.C
              Dyninst::SymtabAPI::Symtab::parseTypes() : void - /dyninst/symtabAPI/src/Symtab.C
                Dyninst::SymtabAPI::Symtab::parseTypesNow() : void - /dyninst/symtabAPI/src/Symtab.C
                  BPatch_module::parseTypes() : void - /dyninst/dyninstAPI/src/BPatch_module.C
                  BPatch_module::parseTypesIfNecessary() : bool - /dyninst/dyninstAPI/src/BPatch_module.C
                  Dyninst::SymtabAPI::FunctionBase::findLocalVariable(vector<localVar *,allocator<localVar *>> &, string) : bool - /dyninst/symtabAPI/src/Function.C
                  Dyninst::SymtabAPI::FunctionBase::getInlinedParent() : FunctionBase * - /dyninst/symtabAPI/src/Function.C
                  Dyninst::SymtabAPI::FunctionBase::getInlines() : const InlineCollection & - /dyninst/symtabAPI/src/Function.C
                  Dyninst::SymtabAPI::FunctionBase::getLocalVariables(vector<localVar *,allocator<localVar *>> &) : bool - /dyninst/symtabAPI/src/Function.C
                  Dyninst::SymtabAPI::FunctionBase::getParams(vector<localVar *,allocator<localVar *>> &) : bool - /dyninst/symtabAPI/src/Function.C
                  Dyninst::SymtabAPI::FunctionBase::getReturnType(enum Dyninst::SymtabAPI::Type::do_share_t) : shared_ptr<Type> - /dyninst/symtabAPI/src/Function.C
                  Dyninst::SymtabAPI::Module::getAllGlobalVars(vector<pair<string,shared_ptr<Type>>,allocator<pair<string,shared_ptr<Type>>>> &) : void - /dyninst/symtabAPI/src/Module.C
                  Dyninst::SymtabAPI::Module::getAllTypes(vector<shared_ptr<Type>,allocator<shared_ptr<Type>>> &) : void - /dyninst/symtabAPI/src/Module.C
                  Dyninst::SymtabAPI::Module::getModuleTypes() : typeCollection * - /dyninst/symtabAPI/src/Module.C
                  Dyninst::SymtabAPI::Symtab::findLocalVariable(vector<localVar *,allocator<localVar *>> &, string) : bool - /dyninst/symtabAPI/src/Symtab.C
                  Dyninst::SymtabAPI::Symtab::findType(shared_ptr<Type> &, string) : bool - /dyninst/symtabAPI/src/Symtab.C
                  Dyninst::SymtabAPI::Symtab::findType(unsigned int, enum Dyninst::SymtabAPI::Type::do_share_t) : shared_ptr<Type> - /dyninst/symtabAPI/src/Symtab.C
                  Dyninst::SymtabAPI::Symtab::findVariableType(shared_ptr<Type> &, string) : bool - /dyninst/symtabAPI/src/Symtab.C
                  Dyninst::SymtabAPI::Symtab::parseFunctionRanges() : bool - /dyninst/symtabAPI/src/Symtab-lookup.C
                  Dyninst::SymtabAPI::Variable::getType(enum Dyninst::SymtabAPI::Type::do_share_t) : shared_ptr<Type> - /dyninst/symtabAPI/src/Variable.C
      Dyninst::SymtabAPI::Symtab::addExternalSymbolReference(Symbol *, Region *, relocationEntry) : bool - /dyninst/symtabAPI/src/Symtab.C
      Dyninst::SymtabAPI::Symtab::addSymbol(Symbol *) : bool - /dyninst/symtabAPI/src/Symtab-edit.C
      Dyninst::SymtabAPI::Symtab::createFunction(string, Offset, size_t, Module *) : Function * - /dyninst/symtabAPI/src/Symtab-edit.C
      Dyninst::SymtabAPI::Symtab::createVariable(string, Offset, size_t, Module *) : Variable * - /dyninst/symtabAPI/src/Symtab-edit.C
      Dyninst::SymtabAPI::Symtab::fixSymModule(Symbol * &) : bool - /dyninst/symtabAPI/src/Symtab.C
        Dyninst::SymtabAPI::Symtab::fixSymModules(vector<Symbol *,allocator<Symbol *>> &) : bool - /dyninst/symtabAPI/src/Symtab.C
          Dyninst::SymtabAPI::Symtab::extractInfo(Object *) : bool - /dyninst/symtabAPI/src/Symtab.C
            Dyninst::SymtabAPI::Symtab::Symtab(string, bool, bool &) - /dyninst/symtabAPI/src/Symtab.C
            Dyninst::SymtabAPI::Symtab::Symtab(unsigned char *, size_t, const string &, bool, bool &) - /dyninst/symtabAPI/src/Symtab.C
      store_line_info(Symtab *, info_for_all_files_t *) : bool - /dyninst/symtabAPI/src/Object-nt.C
    Dyninst::SymtabAPI::Symtab::getOrCreateModule(const string &, unsigned long int) : Module * - /dyninst/symtabAPI/src/Symtab.C
      BinaryEdit::writeFile(const string &) : bool - /dyninst/dyninstAPI/src/binaryEdit.C
      Dyninst::SymtabAPI::Object::fix_global_symbol_modules_static_dwarf() : bool - /dyninst/symtabAPI/src/Object-elf.C
        Dyninst::SymtabAPI::Object::load_object(bool) : void - /dyninst/symtabAPI/src/Object-elf.C
          Dyninst::SymtabAPI::Object::Object(MappedFile *, bool, void (*)(const char *), bool, Symtab *) - /dyninst/symtabAPI/src/Object-elf.C (2 matches)
            Dyninst::SymtabAPI::Symtab::Symtab(string, bool, bool &) - /dyninst/symtabAPI/src/Symtab.C
            Dyninst::SymtabAPI::Symtab::Symtab(unsigned char *, size_t, const string &, bool, bool &) - /dyninst/symtabAPI/src/Symtab.C


``Symtab::member_name_``
========================
This will be either the name from the MappedFile _or_ the name of the ".a" file when the Symtab is created during static re-writing (see ``Archive::parseMember``).

Having multiple CUs per Module
==============================

Just like for functions (Symtab::Function, ParseAPI::Function), a Module
corresponds to a unique range of PC values with its own properties. They
cannot be merged together.

A Symtab::Module is a one-to-one mapping to a DWARF compilation unit
(CU). In DWARF4, we consider a CU to be an entry in the .debug_info
section with the tag DW_TAG_compile_unit. In DWARF5, we also include
entries with the tag DW_TAG_partial_unit as they can contain symbol
definitions; we assume libdw will merge all other split unit types for
us.
