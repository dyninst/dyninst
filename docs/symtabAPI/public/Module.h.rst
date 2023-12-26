Module.h
========

.. cpp:namespace:: Dyninst::SymtabAPI

Class Module
------------

This class represents the concept of a single source file. Currently,
Modules are only identified for the executable file; each shared library
is made up of a single Module, ignoring any source file information that
may be present.

.. container:: center

   ============================== ==============================
   supportedLanguages             Meaning
   ============================== ==============================
   lang_Unknown                   Unknown source language
   lang_Assembly                  Raw assembly code
   lang_C                         C source code
   lang_CPlusPlus                 C++ source code
   lang_GnuCPlusPlus              C++ with GNU extensions
   lang_Fortran                   Fortran source code
   lang_Fortran_with_pretty_debug Fortran with debug annotations
   lang_CMFortran                 Fortran with CM extensions
   ============================== ==============================

.. list-table::
   :widths: 30  35 35
   :header-rows: 1

   * - Method name
     - Return type
     - Method description
   * - isShared
     - bool
     - True if the module is for a shared library, false for an executable.
   * - fullName
     - std::string &
     - Name, including path, of the source file represented by the module.
   * - fileName
     - std::string &
     - Name, not including path, of the source file represented by the module.
   * - language
     - supportedLanguages
     - The source language used by the Module.
   * - addr
     - Offset
     - Offset of the start of the module, as reported by the symbol table, assuming contiguous modules.
   * - exec
     - Symtab *
     - Symtab object that contains the module.
     
 
Function, Variable, Symbol lookup
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. code-block:: cpp

   bool findFunctionByEntryOffset(Function *&ret, const Offset offset)

This method returns the ``Function`` object that begins at ``offset``.
Returns ``true`` on success and ``false`` if there is no matching
function. The error value is set to ``No_Such_Function``.

.. code-block:: cpp

    typedef enum mangledName, prettyName, typedName, anyName NameType;
    bool findFunctionsByName(vector<Function> &ret, const string name, Symtab::NameType nameType = anyName, bool isRegex = false, bool checkCase = true)

This method finds and returns a vector of ``Functions`` whose names
match the given pattern. The ``nameType`` parameter determines which
names are searched: mangled, pretty, typed, or any. If the ``isRegex``
flag is set a regular expression match is performed with the symbol
names. ``checkCase`` is applicable only if ``isRegex`` has been set.
This indicates if the case be considered while performing regular
expression matching. ``ret`` contains the list of matching
``Function``\ s, if any. Returns ``true`` if it finds functions that
match the given name, otherwise returns ``false``. The error value is
set to ``No_Such_Function``.

.. code-block:: cpp

    bool getAllFunctions(vector<Function *> &ret)

This method returns all functions in the object file. Returns ``true``
on success and ``false`` if there are no modules. The error value is set
to ``No_Such_Function``.

.. code-block:: cpp

    bool findVariablesByOffset(std::vector<Variable *> &ret, const Offset offset)

This method returns a vector of ``Variable``\ s with the specified
offset. There may be more than one variable at an offset if they have
different sizes. Returns ``true`` on success and ``false`` if there is
no matching variable. The error value is set to ``No_Such_Variable``.

.. code-block:: cpp

    bool findVariablesByName(vector<Function> &ret, const string &name, Symtab::NameType nameType, bool isRegex = false, bool checkCase = true)

This method finds and returns a vector of ``Variable``\ s whose names
match the given pattern. The ``nameType`` parameter determines which
names are searched: mangled, pretty, typed, or any (note: a ``Variable``
may not have a typed name). If the ``isRegex`` flag is set a regular
expression match is performed with the symbol names. ``checkCase`` is
applicable only if ``isRegex`` has been set. This indicates if the case
be considered while performing regular expression matching. ``ret``
contains the list of matching ``Variables``, if any. Returns ``true`` if
it finds variables that match the given name, otherwise returns
``false``. The error value is set to ``No_Such_Variable``.

.. code-block:: cpp

    bool getAllSymbols(vector<Symbol *> &ret)


This method returns all symbols. Returns ``true`` on success and
``false`` if there are no symbols. The error value is set to
``No_Such_Symbol``.

.. code-block:: cpp

    bool getAllSymbolsByType(vector<Symbol *> &ret, Symbol::SymbolType sType)

This method returns all symbols whose type matches the given type
``sType``. Returns ``true`` on success and ``false`` if there are no
symbols with the given type. The error value is set to
``No_Such_Symbol``.

.. _line-number-information-1:

Line number information for Symtab
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. code-block:: cpp

    bool getAddressRanges(vector<pair<unsigned long, unsigned long> > & ranges, string lineSource, unsigned int lineNo)

This method returns the address ranges in ``ranges`` corresponding to
the line with line number ``lineNo`` in the source file ``lineSource``.
Searches only this module for the given source. Return ``true`` if at
least one address range corresponding to the line number was found and
returns false if none found.

.. code-block:: cpp

    bool getSourceLines(vector<Statement *> &lines, Offset addressInRange)

This method returns the source file names and line numbers corresponding
to the given address ``addressInRange``. Searches only this module for
the given source. Return ``true`` if at least one tuple corresponding to
the offset was found and returns ``false`` if none found. The
``Statement`` class used to be named ``LineNoTuple``; backwards
compatibility is provided via typedef.

.. code-block:: cpp

    LineInformation *getLineInformation() const

This method returns the line map (section `7.1 <#LineInformation>`__)
corresponding to the module. Returns ``NULL`` if there is no line
information existing for the module.

.. code-block:: cpp

    bool getStatements(std::vector<Statement *> &statements)

Returns all line information (section `7.2 <#Statement>`__) available
for the module.

.. _`subsubsec:typeInfo`:

Type information Symtab
~~~~~~~~~~~~~~~~~~~~~~~

.. code-block:: cpp

    bool findType(Type * &type, string name)

This method performs a look up and returns a handle to the named
``type``. This method searches all the built-in types, standard types
and user-defined types within the module. Returns ``true`` if a type is
found with type containing the handle to the type, else return
``false``.

.. code-block:: cpp

    bool findLocalVariable(vector<localVar *> &vars, string name)

The method returns a list of local variables within the module with name
``name``. Returns ``true`` with vars containing a list of ``localVar``
objects corresponding to the local variables if found or else returns
``false``.


.. code-block:: cpp
 
    bool findVariableType(Type *&type, std::string name)

This method looks up a global variable with name ``name`` and returns
its type attribute. Returns ``true`` if a variable is found or returns
``false`` with ``type`` set to ``NULL``.


Class Statement
---------------

A ``Statement`` is the base representation of line information.

=========== ============ ==========================================
Method name Return type  Method description
=========== ============ ==========================================
startAddr   Offset       Starting address of this line in the file.
endAddr     Offset       Ending address of this line in the file.
getFile     std::string  File that contains the line.
getLine     unsigned int Line number.
getColumn   unsigned int Starting column number.
=========== ============ ==========================================

For backwards compatibility, this class may also be referred to as a
``LineNoTuple``, and provides the following legacy member variables.
They should not be used and will be removed in a future version of
SymtabAPI.

====== ============= ========================
Member Return type   Method description
====== ============= ========================
first  const char *  Equivalent to getFile.
second unsigned int  Equivalent to getLine.
column unsigned int  Equivalent to getColumn.
====== ============= ========================