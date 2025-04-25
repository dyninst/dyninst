.. _FunctionBase:

Class FunctionBase
------------------

The class provides a common interface that can represent either a
regular function or an inlined function.

============= ===============
=======================================================================
Method name   Return type     Method description
============= ===============
=======================================================================
getModule     const Module \* Module this function belongs to.
getSize       unsigned        Size encoded in the symbol table; may not be actual function size.
getRegion     Region \*       Region containing this function.
getReturnType Type \*         Type representing the return type of the function.
getName       std::string     Returns primary name of the function (first mangled name or DWARF name)
============= ===============
=======================================================================

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

FunctionBase\* getInlinedParent()

const InlineCollection& getInlines()
