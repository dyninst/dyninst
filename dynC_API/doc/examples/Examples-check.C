// Not typeset.  1-DynC.tex builds one instrumentation task out of raw BPatch
// snippets, a step per listing, so that the dynC version beside it can be
// compared against something.  The steps only make sense in order and share
// the variables they create, so they are included here in that order, inside
// one function, with the handles the prose takes as given.

#include "BPatch.h"
#include "BPatch_process.h"
#include "BPatch_image.h"
#include "BPatch_function.h"
#include "BPatch_point.h"
#include "BPatch_snippet.h"

#include <string>
#include <vector>

// The address space the prose has already attached to, and the functions it
// is walking when the steps below run.
static BPatch_process *appProc;
static BPatch_image *appImage;
static std::vector<BPatch_function *> functions;
static unsigned i;

// An example is an excerpt: it names a thing to show it exists and stops
// there.  These two warnings fire on that by construction, so they are off
// for the included text and on everywhere else.
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-variable"
#pragma GCC diagnostic ignored "-Wunused-parameter"

void rawSnippetWalkthrough()
{
#include "LookupPrintf.C"
#include "Patterns.C"
#include "SnippetVectors.C"
#include "CounterVariable.C"
#include "FunctionName.C"
#include "EntryArgs.C"
#include "ExitArgs.C"
#include "PrintfCalls.C"
#include "IncrementCounter.C"
#include "AddIncrement.C"
#include "InsertSnippets.C"
}

#pragma GCC diagnostic pop
