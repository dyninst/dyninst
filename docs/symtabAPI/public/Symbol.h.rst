Symbol.h
========

.. cpp:namespace:: Dyninst::SymtabAPI

Class Symbol
------------

The ``Symbol`` class represents a symbol in the object file. This class
holds the symbol information such as the mangled, pretty and typed
names, the module in which it is present, type, linkage, offset and
size.

.. container:: center

   =========== ==========================================
   SymbolType  Meaning
   =========== ==========================================
   ST_UNKNOWN  Unknown type
   ST_FUNCTION Function or other executable code sequence
   ST_OBJECT   Variable or other data object
   ST_MODULE   Source file declaration
   ST_SETION   Region declaration
   ST_TLS      Thread-local storage declaration
   ST_DELETED  Deleted symbol
   ST_NOTYPE   Miscellaneous symbol
   =========== ==========================================

.. container:: center

   ============= =========================================
   SymbolLinkage Meaning
   ============= =========================================
   SL_UNKNOWN    Unknown linkage
   SL_GLOBAL     Process-global symbol
   SL_LOCAL      Process-local (e.g., static) symbol
   SL_WEAK       Alternate name for a function or variable
   ============= =========================================

The following two types are platform-specific:

.. code-block:: cpp

    typedef enum SV_UNKNOWN, SV_DEFAULT, SV_INTERNAL, SV_HIDDEN,
    SV_PROTECTED SymbolVisibility;

    typedef enum TAG_UNKNOWN, TAG_USER, TAG_LIBRARY, TAG_INTERNAL SymbolTag;

.. list-table::
   :widths: 30  35 35
   :header-rows: 1

   * - Method name
     - Return type
     - Method description
   * - getMangledName
     - string
     - Raw name of the symbol in the symbol table, including name mangling.
   * - getPrettyName
     - string
     - Demangled name of the symbol with parameters (for functions) removed.
   * - getTypedName
     - string
     - Demangled name of the symbol including full function parameters.
   * - getModule
     - Module *
     - The module, if any, that contains the symbol.
   * - getType
     - SymbolType
     - The symboltype (as defined above) of the symbol.
   * - getLinkage
     - SymbolLinkage
     - The linkage (as defined above) of the symbol.
   * - getVisibility
     - SymbolVisibility
     - The visibility (as defined above) of the symbol.
   * - tag
     - SymbolTag
     - The tag (as defined above) of the symbol.
   * - getOffset
     - Offset
     - The Offset of the object the symbol refers to.
   * - getSize
     - unsigned
     - The size of the object the symbol refers to.
   * - getRegion
     - Region *
     - The region containing the symbol.
   * - getIndex
     - int
     - The index of the symbol within the symbol table.
   * - getStrIndex
     - int
     - The index of the symbol name in the string table.
   * - IsInDynSymtab
     - bool
     - If true, the symbol is dynamic and can be used as the target of an intermodule reference. Implies isInSymtab is false.
   * - IsInSymtab
     - bool
     - If true, the symbol is static. Implies isInDynSymtab is false.
   * - IsAbsolute
     - bool
     - If true, the offset encoded in the symbol is an absolute value rather than offset.
   * - IsFunction
     - bool
     - If true, the symbol refers to a function.
   * - GetFunction
     - Funcion *
     - The Function that contains this symbol if such a Function exists.
   * - isVariable
     - bool
     - If true, the symbol refers to a variable.
   * - getVariable
     - Variable *
     - The Variable that contains the symbol if such a Variable exists.
   * - getSymtab
     - Symtab *
     - The Symtab that contains the symbol.
   * - getPtrOffset
     - Offset
     - For binaries with an OPD section, the offset in the OPD that contains the function pointer data structure for this symbol.
   * - getLocalTOC
     - Offset
     - For platforms with a TOC register, the expected TOC for this object referred to by this symbol.
   * - isCommonStorage
     - bool
     - True if the symbol represents a common section (Fortran).


.. code-block:: cpp

    SYMTAB_EXPORT Symbol(const std::string& name, SymbolType type,
    SymbolLinkage linkage, SymbolVisibility visibility, Offset offset,
    Module *module = NULL, Region *region = NULL, unsigned size = 0, bool
    dyamic = false, bool absolute = false, int index = -1, int strindex =
    -1, bool commonStorage = false)


Symbol creation interface:

name
   The mangled name of the symbol.

type
   The type of the symbol as specified above.

linkage
   The linkage of the symbol as specified above.

visibility
   The visibility of the symbol as specified above.

offset
   The offset within the file that the symbol refers to.

module
   The source code module the symbol should belong to; default is no
   module.

region
   The region the symbol belongs to; if left unset this will be
   determined if a new binary is generated.

size
   The size of the object the symbol refers to; defaults to 0.

dynamic
   If true, the symbol belongs to the dynamic symbol table (ELF) and may
   be used as the target of inter-module references.

absolute
   If true, the offset specified is treated as an absolute value rather
   than an offset.

index
   The index in the symbol table. If left unset, it will be determined
   when generating a new binary.

strindex
   The index in the string table that contains the symbol name. If left
   unset, it will be determined when generating a new binary.

commonStorage
   If true, the symbol references common storage (Fortran).

.. code-block:: cpp

    bool getVersionFileName(std::string &fileName)

This method retrieves the file name in which this symbol is present.
Returns ``false`` if this symbol does not have any version information
present otherwise returns ``true``.

.. code-block:: cpp
    
    bool getVersions(std::vector<std::string> *&vers)

This method retrieves all the version names for this symbol. Returns
``false`` if the symbol does not have any version information present.

.. code-block:: cpp
    
    bool getVersionNum(unsigned &verNum)

This method retrieves the version number of the symbol. Returns
``false`` if the symbol does not have any version information present.

Symbol modification
~~~~~~~~~~~~~~~~~~~

Most elements of a ``Symbol`` can be modified using the functions below.
Each returns ``true`` on success and ``false`` otherwise.

.. code-block:: cpp

    bool setSize (unsigned size) bool setOffset (Offset newOffset) bool
    setMangledName (string name) bool setType (SymbolType sType) bool
    setModule (Module *module) bool setRegion (Region *region) bool
    setDynamic (bool dyn) bool setAbsolute (bool absolute) bool
    setCommonStorage (bool common) bool setFunction (Function *func) bool
    setVariable (Variable *var) bool setIndex (int index) bool setStrIndex
    (int index) bool setPtrOffset (Offset ptr) bool setLocalTOC (Offset toc)
    bool setVersionNum (unsigned num) bool setVersionFileName (std::string
    &fileName) bool setVersions (std::vector<std::string> &vers)