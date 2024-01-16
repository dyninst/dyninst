.. _`sec:Statement.h`:

Statement.h
###########

.. cpp:namespace:: Dyninst

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