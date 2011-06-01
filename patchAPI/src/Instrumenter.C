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

bool
Instrumenter::replaceFunction(PatchFunction* oldfunc, PatchFunction *newfunc) {
  functionReplacements_[oldfunc] = newfunc;
  return true;
}

bool
Instrumenter::revertReplacedFunction(PatchFunction* oldfunc) {
  functionReplacements_.erase(oldfunc);
  return true;
}

bool
Instrumenter::wrapFunction(PatchFunction* oldfunc, PatchFunction *newfunc) {
  functionWraps_[oldfunc] = newfunc;
  return true;
}

bool
Instrumenter::revertWrappedFunction(PatchFunction* oldfunc) {
  functionWraps_.erase(oldfunc);
  return true;
}
