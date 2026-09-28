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

// The handle the first example creates; the rest take it as given.
static Symtab *obj;

static void parseFile()
{
#include "ParseFile.C"
}

static void lookupAndUpdate()
{
#include "LookupAndUpdate.C"
}

static void addVariable()
{
#include "AddVariable.C"
}

static void createType()
{
#include "CreateType.C"
}

static void lineNumbers()
{
#include "LineNumbers.C"
}

static void localVariables()
{
#include "LocalVariables.C"
}

static void iterateLineInfo()
{
#include "IterateLineInfo.C"
}

static void retrieveType()
{
#include "RetrieveType.C"
}
