.. _sec:symtabAPI:

API Reference - Symbol Table Interface
======================================

This section describes the symbol table interface for the SymtabAPI
library. Currently this interface has the following capabilities:

-  Parsing the symbols in a binary, either on disk or in memory

-  Querying for symbols

-  Updating existing symbol information

-  Adding new symbols

-  Exporting symbols in standard formats

-  Accessing relocation and exception information

-  Accessing and modifying header information

The symbol table information is represented by the Symtab, Symbol,
Archive, and Region classes. Module, Function, and Variable provide
abstractions that support common use patterns. Finally, LocalVar
represents function-local variables and parameters.
