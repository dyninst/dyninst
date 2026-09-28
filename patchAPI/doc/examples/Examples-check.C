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

// An example is an excerpt: it names a thing to show it exists and stops
// there.  These two warnings fire on that by construction, so they are off
// for the included text and on everywhere else.
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-variable"
#pragma GCC diagnostic ignored "-Wunused-parameter"

void mySnippet()
{
#include "MySnippet.C"
}

void myInstrumenter()
{
#include "MyInstrumenter.C"
}

#pragma GCC diagnostic pop
