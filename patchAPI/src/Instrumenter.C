/* Plugin */

#include "Instrumenter.h"
#include "PatchCFG.h"

using Dyninst::PatchAPI::InstrumenterPtr;
using Dyninst::PatchAPI::Instrumenter;
using Dyninst::PatchAPI::InstanceSet;

/* Default implementation of Instrumenter */

InstrumenterPtr
Instrumenter::create(AddrSpacePtr as) {
  InstrumenterPtr ret = InstrumenterPtr(new Instrumenter(as));
  if (!ret) return InstrumenterPtr();
  return ret;
}

bool
Instrumenter::process() {
  return true;
}
