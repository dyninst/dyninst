.. _`sec:Symtab.h`:

Symtab.h
########

.. rubric:: Quick map

The Symtab interface is extensive. Use the links below to jump to a particular topic.

* :ref:`Symbols <topic:symtabapi-symbols>`
* :ref:`Functions <topic:symtabapi-functions>`
* :ref:`Variables <topic:symtabapi-variables>`
* :ref:`Modules <topic:symtabapi-modules>`
* :ref:`Regions <topic:symtabapi-regions>`
* :ref:`Exceptions <topic:symtabapi-exceptions>`
* :ref:`Line Information <topic:symtabapi-lineinfo>`
* :ref:`Type Information <topic:symtabapi-typeinfo>`
* :ref:`Object Information <topic:symtabapi-object-info>`

.. cpp:namespace:: Dyninst::SymtabAPI

.. cpp:type:: Dyninst::ProcessReader MemRegReader

.. cpp:class:: Symtab : public LookupInterface, public AnnotatableSparse

  **An object file either on-disk or in-memory**

  This class is responsible for the parsing of the :cpp:class:`Object` file information and holding the data that can be accessed
  through lookup functions.

  .. cpp:function:: Symtab()
  .. cpp:function:: Symtab(unsigned char *mem_image, size_t image_size, const std::string &name, bool defensive_binary, bool &err)

      Creates a symtab representing an in-memory object with contents in ``mem_image`` of
      ``size`` **bytes**.  If ``name`` is provided, it can be used to find this symtab in the lookup interfaces.

      See :ref:`sec:symtabapi-defensive` for a discussion on ``defensive_binary``. It is only applicable on Windows.

      Sets ``err`` to ``false`` on failure.

  .. cpp:function:: ~Symtab()

  .. important::
  
    The Symtab class is neither copyable nor movable.
     
  .. cpp:function:: Symtab(Symtab const&) = delete
  .. cpp:function:: Symtab& operator=(Symtab const&) = delete
  .. cpp:function:: Symtab(Symtab&&) = delete
  .. cpp:function:: Symtab& operator=(Symtab&&) = delete

  ......

  .. _`topic:symtabapi-creation`:

  .. rubric:: Symtab creation

  These are the preferred mechanisms for creating a Symtab from a file or in-memory image.

  .. cpp:function:: static bool openFile(Symtab *&obj, std::string filename, def_t defensive_binary = NotDefensive)

      Creates a symtab in ``obj`` representing an object file on disk at the path ``filename``.

      See :ref:`sec:symtabapi-defensive` for a discussion on ``defensive_binary``. It is only applicable on Windows.

      Returns ``false`` on failure.

  .. cpp:function:: static bool openFile(Symtab *&obj, void *mem_image, size_t size, std::string name, \
                                         def_t defensive_binary = NotDefensive)

      Creates a symtab in ``obj`` representing an in-memory object with contents in ``mem_image`` of
      ``size`` **bytes**.  If ``name`` is provided, it can be used to find this symtab in the lookup interfaces.

      See :ref:`sec:symtabapi-defensive` for a discussion on ``defensive_binary``. It is only applicable on Windows.

      Returns ``false`` on failure.

  ......

  .. _`topic:symtabapi-symbols`:

  .. rubric:: Symbols

  .. cpp:function:: unsigned getNumberOfSymbols() const

      Returns the total number of symbols in both the static and dynamic tables.

  .. cpp:function:: virtual bool findSymbol(std::vector<Symbol *> &ret, const std::string& name, \
                                            Symbol::SymbolType sType = Symbol::ST_UNKNOWN, NameType nameType = anyName, \
                                            bool isRegex = false, bool checkCase = false, bool includeUndefined = false)

      Returns in ``ret`` the symbols with name ``name`` with type ``sType``.

      If ``nameType`` changes the interpretation of ``name``. When ``isRegex`` is ``true``, the contents of ``name`` is treated
      as a regular expression for name-matching. ``checkCase`` enables or disables case-sensitive regex matching.
      ``includeUndefined`` enables finding symbols that do not have a definition.

      Returns ``false`` if no symbol was found.

  .. cpp:function:: virtual bool getAllSymbols(std::vector<Symbol *> &ret)

      Returns all symbols contained in the underlying object.

      Returns ``false`` if there are no symbols.

  .. cpp:function:: virtual bool getAllSymbolsByType(std::vector<Symbol *> &ret, Symbol::SymbolType sType)

      Returns in ``ret`` the symbols with type ``sType``.

      Returns ``false`` if no symbols were found.

  .. cpp:function:: std::vector<Symbol *> findSymbolByOffset(Offset offset)

      Returns all symbols located at the offset ``offset``.

      .. warning:: The returned symbols should not be modified.

  .. cpp:function:: bool getAllUndefinedSymbols(std::vector<Symbol *> &ret)

      Returns symbols without a definition.

      Undefined symbols are those that reference symbols in other files (e.g., external functions or variables)
      and have no definition in the current object. It is currently used for finding the object files in an archive
      that have definitions of these symbols.

      Returns ``false`` if no symbols were found.

  .. cpp:function:: bool getAllDefinedSymbols(std::vector<Symbol *> &ret)

      The opposite of :cpp:func:`getAllUndefinedSymbols`.

  ......

  .. _`topic:symtabapi-functions`:

  .. rubric:: Functions

  .. cpp:function:: bool findFuncByEntryOffset(Function *&ret, const Offset offset)

      Returns the function starting at the offset ``offset``.

      Returns ``false`` if no function is found.

  .. cpp:function:: bool findFunctionsByName(std::vector<Function *> &ret, const std::string name, \
                                             NameType nameType = anyName, bool isRegex = false, bool checkCase = true)

      Equivalent to ``findSymbol(ret, name, Symbol::ST_FUNCTION, nameType, isRegex, checkCase, includeUndefined)``.

  .. cpp:function:: bool getAllFunctions(std::vector<Function *>&ret)

      Returns in ``ret`` all functions contained in the underlying object.

      Returns ``false`` if there are no functions.

  .. cpp:function:: const std::vector<Function*>& getAllFunctionsRef() const

      The same as :cpp:func:`getAllFunctions`.

  .. cpp:function:: bool getContainingFunction(Offset offset, Function* &func)

      Returns in ``func`` the function, if any, that contains the offset ``offset``.

      This function does not perform any parsing and relies on the symbol table for information. It may return
      incorrect information if the symbol table is wrong or if functions are either non-contiguous or overlapping.

      Returns ``false`` if no function was found.

  .. cpp:function:: bool getContainingInlinedFunction(Offset offset, FunctionBase* &func)

      Returns in ``func`` the function that contains the inlined function starting at offset ``offset``.

  .. cpp:function:: bool parseFunctionRanges()

      Parses the function ranges for the object file.

      By default, Symtab uses a lazy evaluation technique to reduce memory consumption when information is not
      needed.

  ......

  .. _`topic:symtabapi-variables`:

  .. rubric:: Variables

  .. cpp:function:: bool findVariablesByOffset(std::vector<Variable *> &ret, const Offset offset)

      Returns in ``ret`` the variables located at ``offset``.

      There may be more than one variable at an offset if they have different sizes.

      Returns ``false`` on error.

  .. cpp:function:: bool findVariablesByName(std::vector<Variable *> &ret, const std::string name, \
                                             NameType nameType = anyName, bool isRegex = false, bool checkCase = true)

      Equivalent to ``findSymbol(ret, name, Symbol::ST_VARIABLE, nameType, isRegex, checkCase, includeUndefined)``.

  .. cpp:function:: bool getAllVariables(std::vector<Variable *> &ret)

      Returns in ``ret`` all variables contained in the underlying object.

      Returns ``false`` if there are no variables.

  .. cpp:function:: bool findLocalVariable(std::vector<localVar *>&vars, std::string name)

      Returns in ``vars`` all local variables with name ``name``.

      Returns ``false`` if no variable was found.

  ......

  .. _`topic:symtabapi-modules`:

  .. rubric:: Modules

  .. cpp:function:: bool getAllModules(vector<module*>& ret)

      Returns in ``ret`` all modules contained in the underlying object.

      Returns ``false`` if there are no modules.

  .. cpp:function:: std::vector<Module*> findModulesByName(std::string const& name) const

      Finds all modules with name ``name``.

      ``name`` should be the full path name as returned by :cpp:func:`file()`.

      .. warning:: Multiple modules may have the same name!

  .. cpp:function:: bool findModuleByOffset(Module *& ret, Offset off)

      .. deprecated:: 12.3

        Use :cpp:func:`Module* findModuleByOffset(Offset) const`.

  .. cpp:function:: Module* findModuleByOffset(Offset offset) const

      Returns the module at the offset ``offset`` in the debug section (e.g., .debug_info).

  .. cpp:function:: Module* getContainingModule(Offset offset) const

      Returns the module that contains source information for the address range that includes the offset ``offset``.

  ......

  .. _`topic:symtabapi-regions`:

  .. rubric:: Regions

  Regions are logical areas of the binary that represent different structures
  like data or code. For ELF binaries, these correspond to sections (e.g., .data, .text., etc.).

  .. cpp:function:: unsigned getNumberOfRegions() const

      Returns the number of regions.

  .. cpp:function:: bool getCodeRegions(std::vector<Region *>&ret)

      Returns in ``ret`` all code regions in the object file.

      Returns ``false`` if no code regions were found.

  .. cpp:function:: bool getDataRegions(std::vector<Region *>&ret)

      Returns in ``ret`` all data regions in the object file.

      Returns ``false`` if no data regions were found.

  .. cpp:function:: bool getAllRegions(std::vector<Region *>&ret)

      Returns in ``ret`` all regions in the object file.

      Returns ``false`` if no regions were found.

  .. cpp:function:: bool findRegion(Region *&ret, std::string regname)

      Returns in ``ret`` the region with name ``regname``.

      Returns ``false`` if no region was found.

  .. cpp:function:: bool findRegion(Region *&ret, const Offset addr, const unsigned long size)

      Returns in ``ret`` the region with a memory offset of ``addr`` and a size of ``size`` **bytes**.

      Returns ``false`` if no region was found.

  .. cpp:function:: bool findRegionByEntry(Region *&ret, const Offset offset)

      Returns in ``ret`` the region with a memory offset of ``addr``.

      Returns ``false`` if no region was found.

  .. cpp:function:: Region* findEnclosingRegion(const Offset offset)

      Returns the region with a virtual address range containing ``offset``.

      Returns ``NULL`` if no region was found.

  .. cpp:function:: bool isCode(const Offset where) const

      Checks if the offset ``where`` belongs to a region containing executable code.

  .. cpp:function:: bool isData(const Offset where) const

      Checks if the offset ``where`` belongs to a region containing data (read-only or writable).

  .. cpp:function:: bool isValidOffset(const Offset where) const

      Checks if the offset ``where`` is valid.

      A valid offset must be aligned and correspond to either a code or data offset.

  .. cpp:function:: bool getMappedRegions(std::vector<Region *> &mappedRegs) const

      Returns in ``mappedRegs`` all loadable regions in the object file.

      Returns ``false`` if no region was found.

  ......

  .. _`topic:symtabapi-exceptions`:

  .. rubric:: Exceptions

  Currently, only C++ exceptions are supported.

  .. cpp:function:: bool findException(ExceptionBlock &excp, Offset addr)

      Returns in ``excp`` the exception block at offset ``addr``.

      Returns ``false`` if no block was found.

  .. cpp:function:: bool getAllExceptions(std::vector<ExceptionBlock *> &exceptions)

      Returns in ``exceptions`` all exception blocks.

      Returns ``false`` if no block was found.

  .. cpp:function:: bool findCatchBlock(ExceptionBlock &excp, Offset addr, unsigned size = 0)

      Returns in ``excp`` the block contained in the address range ``[addr, addr+size]``.

      Returns ``false`` if no block was found.

  ......

  .. _`topic:symtabapi-lineinfo`:

  .. rubric:: Source Line Information

  .. cpp:function:: bool getAddressRanges(std::vector<AddressRange> &ranges, std::string lineSource, unsigned int LineNo)

      Returns in ``ranges`` the address ranges corresponding to the source line number ``lineNo`` in the file ``lineSource``.

      Returns ``false`` if no ranges were found.

  .. cpp:function:: bool getSourceLines(std::vector<Statement::Ptr> &lines, Offset addressInRange)

      Returns in ``lines`` the source file names and line numbers from which code at the address
      ``addressInRange`` was generated.

      Returns ``false`` if no sources were found.

  .. cpp:function:: bool getSourceLines(std::vector<LineNoTuple> &lines, Offset addressInRange)

      The same as :cpp:func:`bool getSourceLines(std::vector<Statement::Ptr> &lines, Offset addressInRange)`.

      For backwards compatibility only.

  ......

  .. _`topic:symtabapi-typeinfo`:

  .. rubric:: Type Information

  .. cpp:function:: virtual bool findType(boost::shared_ptr<Type>& type, std::string name)

      The same as :cpp:func:`bool findType(Type*& t, std::string n)`.

  .. cpp:function:: bool findType(Type*& t, std::string n)

      Returns in ``t`` the type named ``n``.

      Returns ``false`` if no type was found.

  .. cpp:function:: virtual bool findVariableType(boost::shared_ptr<Type>& type, std::string name)

      The same as :cpp:func:`bool findVariableType(Type*& t, std::string n)`.

  .. cpp:function:: bool findVariableType(Type*& t, std::string n)

      Returns in ``t`` the global variable with name ``name``.

      Returns ``false`` if no variable was found.

  .. cpp:function:: void parseTypesNow()

      Parses the the types for the object file.

      By default, Symtab uses a lazy evaluation technique to reduce memory consumption when information is not
      needed.

  ......

  .. _`topic:symtabapi-object-info`:

  .. rubric:: Object Information

  General information about the object used to create the symtab.

  .. cpp:function:: std::string file() const

      Returns the full path to the opened file or provided name for the memory image.

  .. cpp:function:: std::string name() const

      An alias for :cpp:func:`file`.

  .. cpp:function:: bool isExec() const

      Checks if this is an executable that is not a shared library.

      An object may be both an executable and a shared library.

  .. cpp:function:: bool isExecutable() const

      Checks if this is an executable.

      An object may be both an executable and a shared library.

  .. cpp:function:: bool isSharedLibrary() const

      Checks if this is a shared library.

      An object may be both an executable and a shared library.

  .. cpp:function:: bool isStripped()

      Checks if this is a stripped binary (i.e., it has no symbols or debug information).

  .. cpp:function:: bool isStaticBinary() const

      Checks if this is a statically-linked binary.

  .. cpp:function:: ObjectType getObjectType() const

      This method queries information on the type of the object file.

  .. cpp:function:: Dyninst::Architecture getArchitecture() const

      Representation of the system architecture for the binary.

  .. cpp:function:: std::string memberName() const

      If this symtab represents an archive (.a) file, returns the name of the object file.

  .. cpp:function:: Offset imageOffset() const

      Offset at the first code segment from the start of the binary.

  .. cpp:function:: Offset imageLength() const

      Size of the primary code-containing region, typically .text.

  .. cpp:function:: char *mem_image() const

      Returns the raw memory image for the Symtab; not valid for disk files.

  .. cpp:function:: Offset dataOffset() const

      Offset at the first data segment from the start of the binary.

  .. cpp:function:: Offset dataLength() const

      Size of the primary data-containing region, typically .data.

  .. cpp:function:: unsigned getAddressWidth() const

      Size (in bytes) of a pointer value in the Symtab; 4 for 32-bit binaries and 8 for 64-bit binaries.

  .. cpp:function:: Offset getLoadOffset() const

    The suggested load offset of the file; typically 0 for shared libraries.

  .. cpp:function:: Offset getEntryOffset() const

      The entry point (where execution beings) of the binary.

  .. cpp:function:: Offset getBaseOffset() const

      Returns the OS-specified base offset of the file (Windows only).

  .. cpp:function:: bool hasRel() const

      Checks if the object holds relocation entries without explicit addends.

      Only valid for ELF binaries.

  .. cpp:function:: bool hasRela() const

      Checks if the object holds relocation entries with explicit addends.

      Only valid for ELF binaries.

  .. cpp:function:: bool hasReldyn() const

      Checks if the object has a dynamic relocation table with implicit addends.

      Only valid for ELF binaries.

  .. cpp:function:: bool hasReladyn() const

      Checks if the object has a dynamic relocation table with explicit addends.

      Only valid for ELF binaries.

  .. cpp:function:: bool hasRelplt() const

      Checks if the object holds a dynamic relocation referred to by a Procedure Linkage Table
      with implicit addends.

      Only valid for ELF binaries.

  .. cpp:function:: bool hasRelaplt() const

      Checks if the object holds a dynamic relocation referred to by a Procedure Linkage Table
      with explicit addends.

      Only valid for ELF binaries.

  .. cpp:function:: bool getSegments(std::vector<Segment> &segs) const

      Returns in ``segs`` the segments contained in the object.

  .. cpp:function:: Offset preferedBase() const

      Returns the virtual address at which the binary is preferred to be loaded.

      Only valid on Windows.

  .. cpp:function:: Offset getInitOffset()

      Returns the location of the ``.init`` section.

      Only valid for ELF binaries.

  .. cpp:function:: Offset getFiniOffset()

      Returns the location of the ``.fini`` section.

      Only valid for ELF binaries.

  .. cpp:function:: const char* getInterpreterName() const

      Returns the name of the interpreter.

      Only valid for ELF binaries on Linux.

  .. cpp:function:: Address getLoadAddress()

      Returns the address where the object will be loaded into memory.

  .. cpp:function:: bool isDefensiveBinary() const

      Checks if the object is in defensive mode.

      See :ref:`sec:symtabapi-defensive` for details.

  .. cpp:function:: std::vector<std::string> &getDependencies()

      Returns the set of link dependencies for the object.

  .. cpp:function:: Archive *getParentArchive() const

      If the object came from an archive file, returns that file.


.. cpp:enum:: Symtab::def_t

  .. cpp:enumerator:: NotDefensive
  .. cpp:enumerator:: Defensive


Notes
=====

Deprecated error-handling interface

.. code:: cpp

  static SymtabError getLastSymtabError()
  static void setSymtabError(SymtabError new_err)
  static std::string printError(SymtabError serr)

