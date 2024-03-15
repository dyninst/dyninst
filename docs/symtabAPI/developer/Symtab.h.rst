.. _`sec-dev:Symtab.h`:

Symtab.h
########

.. cpp:namespace:: Dyninst::SymtabAPI::dev

.. cpp:class:: Symtab : public LookupInterface, public AnnotatableSparse

  .. cpp:member:: private std::string member_name_

      This will be either the name from the MappedFile _or_ the name of the ".a" file when
      the Symtab is created during static re-writing (see :cpp:func:`Archive::parseMember`).

  .. cpp:function:: private static boost::shared_ptr<typeCollection> setupStdTypes()
  .. cpp:function:: private static boost::shared_ptr<builtInTypeCollection> setupBuiltinTypes()
  .. cpp:member:: private dyn_rwlock symbols_rwlock
  .. cpp:member:: private Offset member_offset_
  .. cpp:member:: private Archive * parentArchive_
  .. cpp:member:: private MappedFile *mf
  .. cpp:member:: private Offset preferedBase_
  .. cpp:member:: private Offset imageOffset_
  .. cpp:member:: private unsigned imageLen_
  .. cpp:member:: private Offset dataOffset_
  .. cpp:member:: private unsigned dataLen_
  .. cpp:member:: private bool is_a_outfalse
  .. cpp:member:: private Offset main_call_addr_
  .. cpp:member:: private unsigned address_width_
  .. cpp:member:: private std::string interpreter_name_
  .. cpp:member:: private Offset entry_address_
  .. cpp:member:: private Offset base_address_
  .. cpp:member:: private Offset load_address_
  .. cpp:member:: private ObjectType object_type_obj_Unknown
  .. cpp:member:: private bool is_eel_false
  .. cpp:member:: private std::vector<Segment> segments_
  .. cpp:member:: private static std::vector<Symtab *> allSymtabs
  .. cpp:member:: private std::string defaultNamespacePrefix
  .. cpp:member:: private unsigned no_of_sections
  .. cpp:member:: private std::vector<Region *> regions_
  .. cpp:member:: private std::vector<Region *> codeRegions_
  .. cpp:member:: private std::vector<Region *> dataRegions_
  .. cpp:member:: private dyn_hash_map <Offset, Region *> regionsByEntryAddr
  .. cpp:member:: private unsigned newSectionInsertPoint

    Point where new loadable sections will be inserted

  .. cpp:member:: private unsigned no_of_symbols

    symbols

  .. cpp:member:: private bool sorted_everyFunctionfalse

    We also need per-Aggregate indices

  .. cpp:member:: private std::vector<Function *> everyFunction
  .. cpp:member:: private std::vector<Variable *> everyVariable

    Similar for Variables

  .. cpp:member:: private std::vector<relocationEntry > relocation_table_
  .. cpp:member:: private std::vector<ExceptionBlock *> excpBlocks
  .. cpp:member:: private std::vector<std::string> deps_
  .. cpp:member:: private std::vector<Archive *> linkingResources_

    This set is used during static linking to satisfy dependencies

  .. cpp:function:: private bool getExplicitSymtabRefs(std::set<Symtab *> &refs)

    This set represents Symtabs referenced by a new external Symbol

  .. cpp:member:: private std::set<Symtab *> explicitSymtabRefs_
  .. cpp:member:: private bool hasRel_false
  .. cpp:member:: private bool hasRela_false
  .. cpp:member:: private bool hasReldyn_false
  .. cpp:member:: private bool hasReladyn_false
  .. cpp:member:: private bool hasRelplt_false
  .. cpp:member:: private bool hasRelaplt_false
  .. cpp:member:: private bool isStaticBinary_false
  .. cpp:member:: private bool isDefensiveBinary_false
  .. cpp:member:: private std::map <std::string, std::string> dynLibSubs

    Don't use obj_private, use getObject() instead.   dynamic library name substitutions


  .. cpp:member:: const std::unique_ptr<symtab_impl> impl

      Hide implementation details that are complex or add large dependencies

  .. cpp:function:: static Symtab *findOpenSymtab(std::string filename)

      Finds a previously-opened symtab with name ``name``.

  .. cpp:function:: static bool closeSymtab(Symtab *s)

      Destroys ``s`` and removes it from the cache of symbtabs.

  .. cpp:function:: bool hasStackwalkDebugInfo()

  .. cpp:function:: bool getRegValueAtFrame(Address pc, Dyninst::MachRegister reg, Dyninst::MachRegisterVal &reg_result, \
                                            MemRegReader *reader)

  .. cpp:function:: bool addRegion(Offset vaddr, void *data, unsigned int dataSize, std::string name, Region::RegionType rType_, bool loadable = false, unsigned long memAlign = sizeof(unsigned), bool tls = false)

      Creates a new region using the specified parameters and adds it to the file.

  .. cpp:function:: bool addRegion(Region *newreg)

      Adds the provided region to the file.

  .. cpp:function:: bool getAllNewRegions(std::vector<Region *>&ret)

      This method finds all the new regions added to the object file. Returns
      ``true`` with ``ret`` containing the regions if there is at least one
      new region that is added to the object file or else returns ``false``.

  .. cpp:function:: void fixup_code_and_data(Offset newImageOffset, Offset newImageLength, Offset newDataOffset, Offset newDataLength)
  .. cpp:function:: bool fixup_RegionAddr(const char* name, Offset memOffset, long memSize)
  .. cpp:function:: bool updateRegion(const char* name, void *buffer, unsigned size)
  .. cpp:function:: bool updateCode(void *buffer, unsigned size)
  .. cpp:function:: bool updateData(void *buffer, unsigned size)
  .. cpp:function:: bool updateFuncBindingTable(Offset stub_addr, Offset plt_addr)

  .. cpp:function:: bool addSymbol(Symbol *newsym)

      This method adds a new symbol ``newsym`` to all of the internal data
      structures. The primary name of the ``newsym`` must be a mangled name.
      Returns ``true`` on success and ``false`` on failure. A new copy of
      ``newsym`` is not made. ``newsym`` must not be deallocated after adding
      it to symtabAPI. We suggest using ``createFunction`` or
      ``createVariable`` when possible.

  .. cpp:function:: bool addSymbol(Symbol *newSym, Symbol *referringSymbol)

      This method adds a new dynamic symbol ``newsym`` which refers to
      ``referringSymbol`` to all of the internal data structures. ``newsym``
      must represent a dynamic symbol. The primary name of the newsym must be
      a mangled name. All the required version names are allocated
      automatically. Also if the ``referringSymbol`` belongs to a shared
      library which is not currently a dependency, the shared library is added
      to the list of dependencies implicitly. Returns ``true`` on success and
      ``false`` on failure. A new copy of ``newsym`` is not made. ``newsym``
      must not be deallocated after adding it to symtabAPI.

  .. cpp:function:: Function *createFunction(std::string name, Offset offset, size_t size, Module *mod = NULL)

      This method creates a ``Function`` and updates all necessary data
      structures (including creating Symbols, if necessary). The function has
      the provided mangled name, offset, and size, and is added to the Module
      ``mod``. Symbols representing the function are added to the static and
      dynamic symbol tables. Returns the pointer to the new ``Function`` on
      success or ``NULL`` on failure.

  .. cpp:function:: Variable *createVariable(std::string name, Offset offset, size_t size, Module *mod = NULL)

      This method creates a ``Variable`` and updates all necessary data
      structures (including creating Symbols, if necessary). The variable has
      the provided mangled name, offset, and size, and is added to the Module
      ``mod``. Symbols representing the variable are added to the static and
      dynamic symbol tables. Returns the pointer to the new ``Variable`` on
      success or ``NULL`` on failure.

  .. cpp:function:: bool deleteFunction(Function *func)

      This method deletes the ``Function`` ``func`` from all of symtab’s data
      structures. It will not be available for further queries. Return
      ``true`` on success and ``false`` if ``func`` is not owned by the
      ``Symtab``.

  .. cpp:function:: bool deleteVariable(Variable *var)

      This method deletes the variable ``var`` from all of symtab’s data
      structures. It will not be available for further queries. Return
      ``true`` on success and ``false`` if ``var`` is not owned by the
      ``Symtab``.

  .. cpp:function:: void setTruncateLinePaths(bool value)
  .. cpp:function:: bool getTruncateLinePaths()
  .. cpp:function:: std::string getDefaultNamespacePrefix() const

  .. cpp:function:: Module* findModuleByOffset(Offset offset) const

      Returns the module at the offset ``offset`` in the debug section (e.g., .debug_info).

  .. cpp:function:: Module *getDefaultModule() const


  .. cpp:function:: bool addType(Type *typ)

      Adds a new type ``type`` to symtabAPI. Return ``true`` on success.

  .. cpp:function:: static boost::shared_ptr<builtInTypeCollection>& builtInTypes()
  .. cpp:function:: static boost::shared_ptr<typeCollection>& stdTypes()

  .. cpp:function:: static void getAllstdTypes(std::vector<boost::shared_ptr<Type>>&)
  .. cpp:function:: static std::vector<Type*>* getAllstdTypes()

      Returns all the standard types that normally occur in a program.


  .. cpp:function:: static void getAllbuiltInTypes(std::vector<boost::shared_ptr<Type>>&)
  .. cpp:function:: static std::vector<Type*>* getAllbuiltInTypes()

      Returns all the built-in types defined in the binary.

  .. cpp:function:: virtual boost::shared_ptr<Type> findType(unsigned type_id, Type::do_share_t)

      The same as :cpp:func:`Type* findType(unsigned i)`.

  .. cpp:function:: Type* findType(unsigned i)

      Returns the type at index ``i``.

      Returns ``false`` if no type was found.

  .. cpp:function:: bool addLine(string lineSource, unsigned int lineNo, unsigned int lineOffset, Offset lowInclusiveAddr, Offset highExclusiveAddr)

      This method adds a new line to the line map. ``lineSource`` represents
      the source file name. ``lineNo`` represents the line number. Returns
      ``true`` on success and ``false`` on error.

  .. cpp:function:: bool addAddressRange(Offset lowInclusiveAddr, Offset highExclusiveAddr, string lineSource, unsigned int lineNo, unsigned int lineOffset = 0);

      This method adds an address range
      ``[lowInclusiveAddr, highExclusiveAddr)`` for the line with line number
      ``lineNo`` in source file ``lineSource`` at offset ``lineOffset``.
      Returns ``true`` on success and ``false`` on error.

  .. cpp:function:: bool emitSymbols(Object *linkedFile, std::string filename, unsigned flag = 0)

  .. cpp:function:: bool emit(std::string filename, unsigned flag = 0)

      Creates a new file using the specified name that contains all changes made by the user.

  .. cpp:function:: void addDynLibSubstitution(std::string oldName, std::string newName)
  .. cpp:function:: std::string getDynLibSubstitution(std::string name)

  .. cpp:function:: Offset getFreeOffset(unsigned size)

      Find a contiguous region of unused space within the file (which may be
      at the end of the file) of the specified size and return an offset to
      the start of the region. Useful for allocating new regions.

  .. cpp:function:: bool addLibraryPrereq(std::string libname)

      Add a library dependence to the file such that when the file is loaded,
      the library will be loaded as well. Cannot be used for static binaries.

  .. cpp:function:: bool addSysVDynamic(long name, long value)
  .. cpp:function:: bool addLinkingResource(Archive *library)
  .. cpp:function:: bool getLinkingResources(std::vector<Archive *> &libs)
  .. cpp:function:: bool addExternalSymbolReference(Symbol *externalSym, Region *localRegion, relocationEntry localRel)
  .. cpp:function:: bool addTrapHeader_win(Address ptr)
  .. cpp:function:: bool updateRelocations(Address start, Address end, Symbol *oldsym, Symbol *newsym)
  .. cpp:function:: bool removeLibraryDependency(std::string lib)
  .. cpp:function:: void rebase(Offset offset)
  .. cpp:function:: Object *getObject()
  .. cpp:function:: const Object *getObject() const
  .. cpp:function:: void dumpModRanges()
  .. cpp:function:: void dumpFuncRanges()
  .. cpp:function:: Module *getOrCreateModule(const std::string &modName, const Offset modAddr)
  .. cpp:function:: Offset getElfDynamicOffset()
  .. cpp:function:: bool delSymbol(Symbol *sym)
  .. cpp:function:: void getSegmentsSymReader(std::vector<SymSegment> &segs)
  .. cpp:function:: bool deleteSymbol(Symbol *sym)

      This method deletes the symbol ``sym`` from all of symtab’s data
      structures. It will not be available for further queries. Return
      ``true`` on success and ``false`` if func is not owned by the
      ``Symtab``.

  .. cpp:function:: static boost::shared_ptr<Type>& type_Error()
  .. cpp:function:: static boost::shared_ptr<Type>& type_Untyped()
  .. cpp:function:: bool getFuncBindingTable(std::vector<relocationEntry> &fbt) const
  .. cpp:function:: bool findPltEntryByTarget(Address target_address, relocationEntry &result) const
  .. cpp:function:: Offset getTOCoffset(Function *func = NULL) const
  .. cpp:function:: Offset getTOCoffset(Offset off) const
  .. cpp:function:: Offset fileToDiskOffset(Dyninst::Offset) const
  .. cpp:function:: Offset fileToMemOffset(Dyninst::Offset) const
  .. cpp:function:: bool canBeShared()



Notes
=====

An Elf Object that can be loaded into memory to form an executable’s
image has one of two types: ET_EXEC and ET_DYN. ET_EXEC type objects are
executables that are loaded at a fixed address determined at link time.
ET_DYN type objects historically were shared libraries that are loaded
at an arbitrary location in memory and are position independent code
(PIC). The ET_DYN object type was reused for position independent
executables (PIE) that allows the executable to be loaded at an
arbitrary location in memory. Although generally not the case an object
can be both a PIE executable and a shared library. Examples of these
include libc.so and the dynamic linker library (ld.so). These objects
are generally used as a shared library so ``isExec()`` will classify
these based on their typical usage. The methods below use heuristics to
classify ET_DYN object types correctly based on the properties of the
Elf Object, and will correctly classify most objects. Due to the
inherent ambiguity of ET_DYN object types, the heuristics may fail to
classify some libraries that are also executables as an executable. This
can happen in object is a shared library and an executable, and its
entry point happens to be at the start of the .text section.

``isExecutable()`` is equivalent to elfutils’ ``elfclassify --program``
test with the refinement of the soname value and entry point tests.
Pseudocode for the algorithm is shown below:

-  **if** (**not** loadable()) **return** *false*

-  **if** (object type is ET_EXEC) **return** *true*

-  **if** (has an interpreter (PT_INTERP segment exists)) **return**
   *true*

-  **if** (PIE flag is set in FLAGS_1 of the PT_DYNAMIC segment)
   **return** *true*

-  **if** (DT_DEBUG tag exists in PT_DYNAMIC segment) **return** *true*

-  **if** (has a soname and its value is “linux-gate.so.1”) **return**
   *false*

-  **if** (entry point is in range .text section offset plus 1 to the
   end of the .text section) **return** *true*

-  **if** (has a soname and its value starts with “ld-linux”) **return**
   *true*

-  **otherwise return** *false*

``isSharedLibrary()`` is equivalent to elfutils’
``elfclassify --library``. Pseudocode for the algorithm is shown below:

-  **if** (**not** loadable()) **return** *false*

-  **if** (object type is ET_EXEC) **return** *false*

-  **if** (there is no PT_DYNAMIC segment) **return** *false*

-  **if** (PIE flag is set in FLAGS_1 of the PT_DYNAMIC segment)
   **return** *false*

-  **if** (DT_DEBUG tag exists in PT_DYNAMIC segment) **return** *false*

-  **otherwise return** *true*

Elf files can also store data that is neither an executable nor a shared
library including object files, core files and debug symbol files. To
distinguish these cases the ``loadable()`` function is defined using the
pseudocode shown below and returns true is the file can loaded into a
process’s address space:

-  **if** (object type is neither ET_EXEC nor ET_DYN) **return** *false*

-  **if** (there is are no program segments with the PT_LOAD flag set)
   **return** *false*

-  **if** (contains no sections) **return** *true*

-  **if** (contains a section with the SHF_ALLOC flag set and a section
   type of neither SHT_NOTE nor SHT_NOBITS) **return** *true*

-  **otherwise return** *false*