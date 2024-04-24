.. _`sec:Module.h`:

Module.h
########

.. cpp:namespace:: Dyninst::SymtabAPI

.. cpp:type:: SimpleInterval<Offset, Module*> ModRange

.. cpp:class:: Module : public LookupInterface

  **Represents the concept of a single source file**

  .. cpp:function:: Module()

      Creates an empty module with no associated file.

  .. cpp:function:: Module(supportedLanguages lang, Offset addr, std::string fullNm, Symtab *img)

      Creates a module in ``img`` associated with the file ``fullNm``, starting at offset ``addr``, and
      the source language ``lang``.

  .. cpp:function:: bool operator==(Module &mod)

      Checks if this module is equal to ``mod``.

      Two modules are equal if they have the same :cpp:func:`fileName`, :cpp:func:`address <addr>`,
      and :cpp:func:`language`.

  .. cpp:function:: const std::string &fileName() const

      Alias for :cpp:func:`fullName`.

  .. cpp:function:: const std::string &fullName() const

      Name of the source file represented by the module.

  .. cpp:function:: supportedLanguages language() const

      The source language used by the module.

  .. cpp:function:: Offset addr() const

      Returns the offset of the start of the module as reported by the symbol table assuming contiguous modules.

  .. cpp:function:: bool isShared() const

      Checks if the module is for a shared library.

  .. cpp:function:: virtual bool findSymbol(std::vector<Symbol*> &ret, const std::string &name, Symbol::SymbolType sType = Symbol::ST_UNKNOWN, \
                                            NameType nameType = anyName, bool isRegex = false, bool checkCase = false, bool includeUndefined = false)

      Searches for a symbols withname ``name``, type ``sType``, and type of name ``nameType`` and stores them in ``ret``. If ``isRegex`` is ``true``,
      then ``name`` is treated as a regular expression. If ``checkCase`` and ``isRegex`` are ``true``, then ``name`` is compared using a case-sensitive match.
      If ``includeUndefined`` is ``true``, then symbols without a definition are also included.

      Returns ``true`` if at least one symbol was found.

  .. cpp:function:: virtual bool getAllSymbolsByType(std::vector<Symbol*> &ret, Symbol::SymbolType sType)

      Returns all symbols in ``ret`` with type ``sType``.

      Returns ``true`` if at least one symbol was found.

  .. cpp:function:: virtual bool getAllSymbols(std::vector<Symbol*> &ret)

      Returns in ``ret`` all symbols contained in this module.

      Returns ``true`` if at least one symbol was found.

  .. cpp:function:: std::vector<Function*> getAllFunctions() const

      Returns all functions in this module.

  .. cpp:function:: bool findVariablesByOffset(std::vector<Variable*> &ret, const Offset offset)

      Searches for variables with offset ``offset`` and stores them in ``ret``.

      Returns ``true`` if at least one variable was found.

  .. cpp:function:: bool findVariablesByName(std::vector<Variable*> &ret, const std::string &name,\
                                             NameType nameType = anyName, bool isRegex = false, bool checkCase = true)

      Searches for all variables in the same was as :cpp:func:`findSymbol`.

  .. cpp:function:: virtual bool findType(boost::shared_ptr<Type> &type, std::string name)

      Searches for a contained type with name ``name`` and stores it in ``type``.

      Returns ``true`` if at least one type was found.

  .. cpp:function:: bool findType(Type*& t, std::string n)

      Searches for a contained type with name ``n`` and stores it in ``t``.

      Returns ``true`` if at least one type was found.

  .. cpp:function:: virtual bool findVariableType(boost::shared_ptr<Type> &type, std::string name)

  .. cpp:function:: bool findVariableType(Type *&t, std::string n)

      This method looks up a global variable with name ``name`` and returns
      its type attribute. Returns ``true`` if a variable is found or returns
      ``false`` with ``type`` set to ``NULL``.

  .. cpp:function:: void getAllTypes(std::vector<boost::shared_ptr<Type>>& types)

      Returns in ``types`` all types in this module.

  .. cpp:function:: std::vector<Type*> *getAllTypes()

      Returns all types in this module.

  .. cpp:function:: void getAllGlobalVars(std::vector<std::pair<std::string, boost::shared_ptr<Type>>> &vars)

      Returns in ``vars`` all global variables in the module.

      The first element of the returned type is the name of the variable, and the second is its type.

  .. cpp:function:: std::vector<std::pair<std::string, Type*>> *getAllGlobalVars()

        Returns all global variables in the module.

        The first element of the returned type is the name of the variable, and the second is its type.

  .. cpp:function:: typeCollection *getModuleTypes()

        Returns all types contained in this module.

  .. cpp:function:: bool findLocalVariable(std::vector<localVar*> &vars, std::string name)

      Returns in ``vars`` the local variable with name ``name``.

      Returns ``true`` if at least one variable was found.

  .. cpp:function:: bool getAddressRanges(std::vector<AddressRange> &ranges, std::string lineSource, \
                                          unsigned int LineNo)

      Returns in ``ranges`` the address ranges corresponding to the line with line number ``lineNo``
      in the source file ``lineSource``.

      Returns ``true`` if at least one range was found.

  .. cpp:function:: bool getSourceLines(std::vector<Statement::Ptr> &lines, Offset addressInRange)

      Returns in ``lines`` the source file names and line numbers covering the address ``addressInRange``.

      Returns ``true`` if at least one range was found.

  .. cpp:function:: bool getSourceLines(std::vector<LineNoTuple> &lines, Offset addressInRange)

      Returns in ``lines`` the source file names and line numbers covering the address ``addressInRange``.

      Returns ``true`` if at least one range was found.

  .. cpp:function:: bool getStatements(std::vector<Statement::Ptr> &statements)

      Returns in ``statements`` all statements in this module.

  .. cpp:function:: LineInformation *getLineInformation()

      Returns the line map for this module.

  .. cpp:function:: LineInformation *parseLineInformation()

      Parses the line map information for this module.

      By default, module construction does not parse the line information until it is needed
      or explicitly requested (by call this function).

