// Not typeset.  FilteredIteration.C is printed as the body of a walk, so it
// names three things the surrounding text describes rather than defines: the
// function to start from, and the do_stuff functor a reader supplies.  This
// gives it those and pulls it in for the docs build to compile.

#include "CFG.h"
#include "CodeObject.h"

#include <boost/iterator/filter_iterator.hpp>

#include <algorithm>
#include <iterator>
#include <vector>

using namespace Dyninst;
using namespace Dyninst::ParseAPI;

// The function whose blocks the example walks.
static Function *func;

// The per-edge action the example leaves to the reader.
struct do_stuff
{
  // An example is an excerpt: it names a thing to show it exists and stops
// there.  These two warnings fire on that by construction, so they are off
// for the included text and on everywhere else.
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-variable"
#pragma GCC diagnostic ignored "-Wunused-parameter"

void operator()(Edge *) {}
};

void filteredIteration()
{
#include "FilteredIteration.C"
}

#pragma GCC diagnostic pop
