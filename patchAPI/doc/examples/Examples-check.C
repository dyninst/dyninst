// Not typeset.  The plugin examples beside this file are printed as class
// bodies alone, so this supplies the headers they derive from and pulls
// each one in for the docs build to compile.

#include "Command.h"
#include "Instrumenter.h"
#include "PatchCFG.h"
#include "PatchMgr.h"
#include "Point.h"
#include "Snippet.h"

#include <vector>

using namespace Dyninst;
using namespace Dyninst::PatchAPI;

// An example is an excerpt: MySnippet::generate has a comment where a body
// would use its two parameters.  That warning fires on it by construction, so
// it is off for the included text and on everywhere else.
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"

// MySnippet.C is a class definition followed by an instance of it, so it goes
// at namespace scope rather than inside a function.  The namespace also keeps
// its snippet out of the way of the code patching example's, which the manual
// prints with the same name.
namespace snippet_plugin  {
#include "MySnippet.C"
}

void myInstrumenter()
{
#include "MyInstrumenter.C"
}

// FilterFunc is a class template, which cannot be defined inside a function.
#include "FilterFunc.C"

// The manager and points the code patching example is handed by the point
// finding example above it, which stays inline because it is written around
// an elision.
static PatchMgrPtr mgr;
static std::vector<Point *> pts;

void codePatching()
{
  using snippet_plugin::MySnippet;
#include "CodePatching.C"
}

#pragma GCC diagnostic pop
