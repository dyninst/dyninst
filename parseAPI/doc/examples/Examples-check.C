// Not typeset.  The examples beside this file are printed without the
// headers and declarations they assume, so this supplies them and pulls
// each one in for the docs build to compile.

#include "CFG.h"
#include "CodeObject.h"

#include <boost/iterator/filter_iterator.hpp>

#include <algorithm>
#include <iterator>
#include <set>
#include <vector>

using namespace Dyninst;
using namespace Dyninst::ParseAPI;

// The function whose blocks the example walks.
static Function *func;

// The per-edge action the example leaves to the reader.
struct do_stuff
{
  void operator()(Edge *) {}
};

// The block whose functions the getFuncs example asks for.
static Block *block;

// GetLoopInFunc is printed whole, so it is a definition rather than a body.
#include "LoopAnalysis.C"

void filteredIteration()
{
#include "FilteredIteration.C"
}

void blockFuncs()
{
#include "BlockFuncs.C"
}
