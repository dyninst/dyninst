// Not typeset.  The examples beside this file are printed by the manual
// without the context they take for granted -- the headers they need and
// the handle the first of them returns -- so this supplies that and pulls
// each one in, to be compiled by the docs build like any other example.

#include "Symtab.h"
#include "Function.h"
#include "Module.h"
#include "Variable.h"
#include "Type.h"
#include "Symbol.h"
#include "LineInformation.h"

#include <string>
#include <vector>

using namespace Dyninst;
using namespace SymtabAPI;

// An example is an excerpt: it names a variable to show it exists and stops
// there.  That warning fires on it by construction, so it is off for the
// included text and on everywhere else.
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-variable"

void parseFile()
{
#include "ParseFile.C"
}

void lookupAndUpdate()
{
  Symtab *obj = nullptr;
#include "LookupAndUpdate.C"
}

void addVariable()
{
  Symtab *obj = nullptr;
#include "AddVariable.C"
}

void createType()
{
  Symtab *obj = nullptr;
#include "CreateType.C"
}

void lineNumbers()
{
  Symtab *obj = nullptr;
#include "LineNumbers.C"
}

void localVariables()
{
  Symtab *obj = nullptr;
#include "LocalVariables.C"
}

void iterateLineInfo()
{
  Symtab *obj = nullptr;
#include "IterateLineInfo.C"
}

void retrieveType()
{
  Symtab *obj = nullptr;
#include "RetrieveType.C"
}

#pragma GCC diagnostic pop
