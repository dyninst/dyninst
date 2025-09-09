Class Variable
--------------

The class represents a collection of symbols that have the same address
and represent data.

=================== ====================
==========================================================================
Method name         Return type          Method description
=================== ====================
==========================================================================
getOffset           Offset               Offset associated with this variable.
getSize             unsigned             Size of this variable in the symbol table.
mangled_names_begin Aggregate::name_iter Beginning of a range of unique names of symbols pointing to this variable.
mangled_names_end   Aggregate::name_iter End of a range of unique names of symbols pointing to this variable.
getType             Type \*              Type of this variable, if known.
getModule           const Module \*      Module that contains this variable.
getRegion           Region \*            Region that contains this variable.
=================== ====================
==========================================================================

bool getSymbols(vector<Symbol \*> &syms) const

bool setModule (Module \*module)

bool setSize (unsigned size)

bool setOffset (Offset offset)

bool addMangledName(string name, bool isPrimary)

bool addPrettyName(string name, bool isPrimary)

bool addTypedName(string name, bool isPrimary)

bool setType(Type \*type)
