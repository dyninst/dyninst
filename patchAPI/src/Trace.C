/* Utility */

// Let's do the block class first.

// A Trace is a container for a set of instructions and
// instrumentation that we generate out as a single unit. To make life
// easier, it's currently matching the (implied) single-control-flow
// path assumption. That means that edge instrumentation must go into
// a separate Trace. Darn.
//
// A Trace consists of three logical types of elements: instructions,
// instrumentation, and flow controllers.
//
// Instruction: a reference to an original code instruction that is
// being moved as part of this block.

// Instrumentation: a reference to a (currently) AtomTrampInstance
// that contains an entire, single-entry, single-exit sequence of
// instrumentation.

// Flow controllers: an abstraction of control flow instructions
// (branches/calls) that end basic blocks. One effect of moving code
// around is that control flow gets b0rked pretty badly, so we toss
// the original instruction and regenerate it later.

#include "Trace.h"
#include "BufferMgr.h"

using namespace Dyninst;
using namespace PatchAPI;

int Trace::TraceID = 0;

Trace::Ptr Trace::create(Widget::Ptr a, Address addr, PatchFunction *f) {
  if (!a) return Ptr();
  Ptr newTrace = Ptr(new Trace(a, addr, f));
  return newTrace;
}

// Returns false only on catastrophic failure.

// Returns false on catastrophic failure
// Sets changed to true if something about this block
// changed in this generation; e.g. if we should re-run the
// fixpoint algorithm.
//
// Our fixpoint is to minimize the size of the generated code w.r.t.
// branches. So we "change" if the address of the first instruction changes
// (since it might be a target) or if the size of the block changes...
//
// So basically if the incoming start address is different or if the
// block size changed.

bool Trace::generate(BufferMgr &buffer) {
  patch_cerr << ws12 << "Generating block " << id() << " orig @ " << hex
             << origAddr() << dec << endl;

  // Register ourselves with the BufferMgr and get a label
  label_ = buffer.getLabel();

  // Simple form: iterate over every Widget, in order, and generate it.
  for (WidgetList::iterator iter = elements_.begin();
       iter != elements_.end(); ++iter) {
    if (!(*iter)->generate(this,
                           buffer)) {
      return false;
      // This leaves the block in an inconsistent state and should only be used
      // for fatal failures.
    }
  }

  return true;
}

std::string Trace::format() const {
  stringstream ret;
  ret << "Trace("
      << std::hex << origAddr() << std::dec
      << "/" << id()
      << ") {" << endl;
  for (WidgetList::const_iterator iter = elements_.begin();
       iter != elements_.end(); ++iter) {
    ret << "  " << (*iter)->format() << endl;
  }
  ret << "}" << endl;
  return ret.str();
}
