// Not typeset.  The plugin examples beside this file are printed as class
// bodies alone, so this supplies the headers they derive from and pulls
// each one in for the docs build to compile.

#include "Command.h"
#include "Instrumenter.h"
#include "PatchCFG.h"
#include "Point.h"
#include "Snippet.h"

using namespace Dyninst;
using namespace Dyninst::PatchAPI;

static void mySnippet()
{
#include "MySnippet.C"
}

static void myInstrumenter()
{
#include "MyInstrumenter.C"
}
