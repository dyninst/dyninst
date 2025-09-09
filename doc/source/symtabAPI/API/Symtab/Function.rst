.. _Function:

Class Function
--------------

The class represents a collection of symbols that have the same address
and a type of . When appropriate, use this representation instead of the
underlying objects.

=================== ====================
==========================================================================
Method name         Return type          Method description
=================== ====================
==========================================================================
getModule           const Module \*      Module this function belongs to.
getOffset           Offset               Offset in the file associated with the function.
getSize             unsigned             Size encoded in the symbol table; may not be actual function size.
mangled_names_begin Aggregate::name_iter Beginning of a range of unique names of symbols pointing to this function.
mangled_names_end   Aggregate::name_iter End of a range of unique names of symbols pointing to this function.
pretty_names_begin  Aggregate::name_iter As above, but prettified with the demangler.
pretty_names_end    Aggregate::name_iter As above, but prettified with the demangler.
typed_names_begin   Aggregate::name_iter As above, but including full type strings.
typed_names_end     Aggregate::name_iter As above, but including full type strings.
getRegion           Region \*            Region containing this function.
getReturnType       Type \*              Type representing the return type of the function.
=================== ====================
==========================================================================

bool getSymbols(vector<Symbol \*> &syms) const

bool setModule (Module \*module)

bool setSize (unsigned size)

bool setOffset (Offset offset)

bool addMangledName(string name, bool isPrimary)

bool addPrettyName(string name, bool isPrimary)

bool addTypedName(string name, bool isPrimary)

bool getLocalVariables(vector<localVar \*> &vars)

std::vector<VariableLocation> &getFramePtr()

bool getParams(vector<localVar \*> &params)

bool findLocalVariable(vector<localVar \*> &vars, string name)
