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

// Every example after the first works through a handle that the first one
// opens.  It arrives as a parameter rather than a local because a local
// would need a value, and a null one lets an optimising compiler see a null
// 'this' at each call through it and report -Wnonnull.  A parameter is
// opaque: there is no caller in this file to give it a value.
void parseFile()
{
#include "ParseFile.C"
}

void lookupAndUpdate(Symtab *obj)
{
#include "LookupAndUpdate.C"
}

void addVariable(Symtab *obj)
{
#include "AddVariable.C"
}

void createType(Symtab *obj)
{
#include "CreateType.C"
}

void lineNumbers(Symtab *obj)
{
#include "LineNumbers.C"
}

void localVariables(Symtab *obj)
{
#include "LocalVariables.C"
}

void iterateLineInfo(Symtab *obj)
{
#include "IterateLineInfo.C"
}

void retrieveType(Symtab *obj)
{
#include "RetrieveType.C"
}

#pragma GCC diagnostic pop
