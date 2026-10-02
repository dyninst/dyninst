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

// The per-edge action the example leaves to the reader.
struct do_stuff
{
  void operator()(Edge *) {}
};

// GetLoopInFunc is printed whole, so it is a definition rather than a body.
#include "LoopAnalysis.C"

// The handles the examples take as given arrive as parameters rather than as
// file-scope variables: nothing here gives them a value, and a compiler that
// can see a null one reports every call through it as -Wnonnull.

// func is the function whose blocks the example walks.
void filteredIteration(Function *func)
{
#include "FilteredIteration.C"
}

// block is the one whose functions the getFuncs example asks for.
void blockFuncs(Block *block)
{
#include "BlockFuncs.C"
}
