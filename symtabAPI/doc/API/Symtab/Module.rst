.. _Module:

Class Module
------------

This class represents the concept of a single source file. Currently,
Modules are only identified for the executable file; each shared library
is made up of a single Module, ignoring any source file information that
may be present. We also create a single module, called , for each Symtab
that contains any symbols for which module information was unavailable.
This may be compiler template code, or files produced without module
information.

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

=========== ==================
================================================================================================
Method name Return type        Method description
=========== ==================
================================================================================================
isShared    bool               True if the module is for a shared library, false for an executable.
fullName    std::string &      Name, including path, of the source file represented by the module.
fileName    std::string &      Name, not including path, of the source file represented by the module.
language    supportedLanguages The source language used by the Module.
addr        Offset             Offset of the start of the module, as reported by the symbol table, assuming contiguous modules.
exec        Symtab \*          Symtab object that contains the module.
=========== ==================
================================================================================================

Function, Variable, Symbol lookup
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

typedef enum mangledName, prettyName, typedName, anyName NameType;

bool getAllFunctions(vector<Function \*> &ret)

bool findVariablesByOffset(std::vector<Variable \*> &ret, const Offset
offset)

bool findVariablesByName(vector<Function> &ret, const string &name,
Symtab::NameType nameType, bool isRegex = false, bool checkCase = true)

bool getAllSymbols(vector<Symbol \*> &ret)

bool getAllSymbolsByType(vector<Symbol \*> &ret, Symbol::SymbolType
sType)

Line number information
~~~~~~~~~~~~~~~~~~~~~~~

bool getAddressRanges(vector<pair<unsigned long, unsigned long> > &
ranges, string lineSource, unsigned int lineNo)

bool getSourceLines(vector<Statement \*> &lines, Offset addressInRange)

LineInformation \*getLineInformation() const

bool getStatements(std::vector<Statement \*> &statements)

.. _subsubsec:typeInfo:

Type information
~~~~~~~~~~~~~~~~

bool findType(Type \* &type, string name)

bool findLocalVariable(vector<localVar \*> &vars, string name)

bool findVariableType(Type \*&type, std::string name)
