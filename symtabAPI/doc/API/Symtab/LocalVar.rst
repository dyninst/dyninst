.. _localVar:

Class localVar
--------------

This represents a local variable or parameter of a function.

=========== ===========
======================================================
Method name Return type Method description
=========== ===========
======================================================
getName     string &    Name of the local variable or parameter.
getType     Type \*     Type associated with the variable.
getFileName string &    File where the variable was declared, if known.
getLineNum  int         Line number where the variable was declared, if known.
=========== ===========
======================================================

vector<VariableLocation> &getLocationLists()
