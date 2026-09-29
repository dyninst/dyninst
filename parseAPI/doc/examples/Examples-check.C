// Not typeset.  FilteredIteration.C is printed as the body of a walk, so it
// names two things the surrounding text describes rather than defines: the
// function to start from, and the do_stuff functor a reader supplies.  This
// gives it those and pulls it in for the docs build to compile.

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

void filteredIteration()
{
#include "FilteredIteration.C"
}
