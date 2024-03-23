.. _`sec:Symbol.h`:

Symbol.h
########

.. cpp:namespace:: Dyninst::SymtabAPI

.. cpp:class:: Symbol : public AnnotatableSparse

  **A symbol in an binary file**

  This class holds the symbol information such as the mangled, pretty and typed
  names, the module in which it is present, type, linkage, offset and size.

  .. cpp:function:: static const char *symbolType2Str(SymbolType t)
  .. cpp:function:: static const char *symbolLinkage2Str(SymbolLinkage t)
  .. cpp:function:: static const char *symbolTag2Str(SymbolTag t)
  .. cpp:function:: static const char *symbolVisibility2Str(SymbolVisibility t)

  .. cpp:function:: Symbol()

  .. cpp:function:: Symbol(const std::string& name, SymbolType t, SymbolLinkage l, SymbolVisibility v, Offset o, \
                           Module *module = NULL, Region *r = NULL, unsigned s = 0, bool d = false, bool a = false, \
                           int index= -1, int strindex = -1, bool cs = false)

      Creates a symbol with a mangled name ``name``, type ``t``, linkage type ``l``, and visibility ``v`` at file offset ``o``.

      ``module`` is the source code module the symbol should belong to. ``region`` is the region the symbol belongs to;
      if left unset this will be determined if a new binary is generated. ``size`` is the size of the object the symbol
      refers to. When ``dynamic`` is ``true``, the symbol belongs to the dynamic symbol table(ELF) and may be used as the
      target of inter-module references. When ``absolute`` is ``true``, the ``o`` is treated as an absolute value rather
      than an offset. ``index`` is the index in the symbol table. If left unset, it will be determined when generating a
      new binary. ``strindex`` is the index in the string table that contains the symbol name. If left unset, it will be
      determined when generating a new binary. When ``commonStorage`` is ``true``, the symbol references common storage
      (only used for Fortran).

  .. cpp:function:: std::string getMangledName() const

      Raw name of the symbol in the symbol table, including name mangling.

  .. cpp:function:: std::string getPrettyName() const

      Demangled name of the symbol with parameters(for functions) removed.

  .. cpp:function:: std::string getTypedName() const

      Demangled name of the symbol including full function parameters.

  .. cpp:function:: Module *getModule() const

      The module, if any, that contains the symbol.

  .. cpp:function:: SymbolType getType() const

      Returns the type of the symbol.

  .. cpp:function:: SymbolLinkage getLinkage() const

      Return the linkage of the symbol.

  .. cpp:function:: Offset getOffset() const

      Return the offset of the symbol.

  .. cpp:function:: unsigned getSize() const

      Returns the size of the symbol.

  .. cpp:function:: Region *getRegion() const

      Returns the region containing the symbol.

  .. cpp:function:: bool isInDynSymtab() const

      Checks if the symbol is dynamic and can be used as the target of an intermodule reference.

  .. cpp:function:: bool isInSymtab() const

      Checks if the symbol is static.

  .. cpp:function:: bool isAbsolute() const

      Checks if the offset encoded in the symbol is an absolute value rather than offset.

  .. cpp:function:: bool isDebug() const

      Checks if the symbol is in the debug section.

  .. cpp:function:: bool isCommonStorage() const

      Checks if the symbol represents a common section (Fortran only).

  .. cpp:function:: bool isFunction() const

      Checks if the symbol refers to a function.

  .. cpp:function:: Function* getFunction() const

      Returns the function that contains this symbol, if it exists.

  .. cpp:function:: bool isVariable() const

      Checks if the symbol refers to a variable.

  .. cpp:function:: Variable* getVariable() const

      Returns the variable to which this symbol refers, if it exists.

  .. cpp:function:: SymbolVisibility getVisibility() const

      Returns the visibility of this symbol.

  .. cpp:function:: int getIndex() const

      Returns the index of the symbol within the symbol table.

  .. cpp:function:: int getStrIndex() const

      Returns the index of the symbol name in the string table.

  .. cpp:function:: SymbolTag tag() const

      Returns the tag of this symbol.


.. cpp:enum:: Symbol::SymbolType

  .. cpp:enumerator:: ST_UNKNOWN

    Unknown type

  .. cpp:enumerator:: ST_FUNCTION

    Function or other executable code sequence

  .. cpp:enumerator:: ST_OBJECT

    Variable or other data object

  .. cpp:enumerator:: ST_MODULE

    Source file declaration

  .. cpp:enumerator:: ST_SETION

    Region declaration

  .. cpp:enumerator:: ST_TLS

    Thread-local storage declaration

  .. cpp:enumerator:: ST_DELETED

    Deleted symbol

  .. cpp:enumerator:: ST_NOTYPE

    Miscellaneous symbol


.. cpp:enum:: Symbol::SymbolLinkage

  .. cpp:enumerator:: SL_UNKNOWN

    Unknown linkage

  .. cpp:enumerator:: SL_GLOBAL

    Process-global symbol

  .. cpp:enumerator:: SL_LOCAL

    Process-local(e.g., static) symbol

  .. cpp:enumerator:: SL_WEAK

    Alternate name for a function or variable


.. cpp:enum:: Symbol::SymbolVisibility

  .. cpp:enumerator:: SV_UNKNOWN
  .. cpp:enumerator:: SV_DEFAULT
  .. cpp:enumerator:: SV_INTERNAL
  .. cpp:enumerator:: SV_HIDDEN
  .. cpp:enumerator:: SV_PROTECTED


.. cpp:enum:: Symbol::SymbolTag

  .. cpp:enumerator:: TAG_UNKNOWN
  .. cpp:enumerator:: TAG_USER
  .. cpp:enumerator:: TAG_LIBRARY
  .. cpp:enumerator:: TAG_INTERNAL


.. cpp:struct:: Symbol::Ptr

  Stores a :cpp:class:`Symbol`\ \*.


.. cpp:class:: LookupInterface

  .. cpp:function:: LookupInterface()
  .. cpp:function:: virtual bool getAllSymbolsByType(std::vector<Symbol *> &ret, Symbol::SymbolType sType) = 0

  .. cpp:function:: virtual bool findSymbol(std::vector<Symbol *> &ret, const std::string& name, \
                                            Symbol::SymbolType sType = Symbol::SymbolType::ST_UNKNOWN, \
                                            NameType nameType = anyName, bool isRegex = false, \
                                            bool checkCase = false, bool includeUndefined = false) = 0

  .. cpp:function:: virtual bool findType(boost::shared_ptr<Type>& type, std::string name) = 0
  .. cpp:function:: bool findType(Type*& t, std::string n)
  .. cpp:function:: virtual bool findVariableType(boost::shared_ptr<Type>& type, std::string name)= 0
  .. cpp:function:: bool findVariableType(Type*& t, std::string n)
  .. cpp:function:: virtual ~LookupInterface()
