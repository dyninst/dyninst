.. _LineNoInterface:

API Reference - Line Number Interface
=====================================

This section describes the line number interface for the SymtabAPI
library. Currently this interface has the following capabilities:

-  Look up address ranges for a given line number.

-  Look up source lines for a given address.

-  Add new line information. This information will be available for
   lookup, but will not be included with an emitted object file.

In order to look up or add line information, the user/application must
have already parsed the object file and should have a Symtab handle to
the object file. For more information on line information lookups
through the Symtab class refer to Section
`[sec:symtabAPI] <#sec:symtabAPI>`__. The rest of this section describes
the classes that are part of the line number interface.
