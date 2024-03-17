.. `sec:symtabapi-dev-intro`:

SymtabAPI
#########

.. toctree::
  :caption: Developer API
  :name: symtabapi-developer-api
  :hidden:
  :maxdepth: 1

  AddrLookup.h
  Aggregate.h
  annotations.h
  Archive.h
  Collections.h
  debug.h
  dwarfWalker.h
  emitElf.h
  emitElfStatic.h
  emitWin.h
  Function.h
  indexed_modules.h
  indexed_symbols.h
  LineInformation.h
  LinkMap.h
  Module.h
  Object-elf.h
  Object.h
  Object-nt.h
  Region.h
  relocationEntry.h
  Statement.h
  StringTable.h
  Symbol.h
  Symtab.h
  SymtabReader.h
  Type.h
  Type-mem.h
  Variable.h


A design goal with SymtabAPI is to allow users and tool
developers to easily extend or add symbol or debug information to the
library through a platform-independent interface. Often times it is
impossible to satify all the requirements of a tool that uses SymtabAPI,
as those requirements can vary from tool to tool. So by providing
extensible structures, SymtabAPI allows tools to modify any structure to
fit their own requirements. Also, tools frequently use more
sophisticated analyses to augment the information available from the
binary directly; it should be possible to make this extra information
available to the SymtabAPI library. An example of this is a tool
operating on a stripped binary. Although the symbols for the majority of
functions in the binary may be missing, many can be determined via more
sophisticated analysis. In our model, the tool would then inform the
SymtabAPI library of the presence of these functions; this information
would be incorporated and available for subsequent analysis. Other
examples of such extensions might involve creating and adding new types
or adding new local variables to certain functions. Adding a new format requires no changes to the
interface and hence will not affect any of the tools that use the
SymtabAPI.
