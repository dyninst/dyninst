/* Plugin */

#include "Instrumenter.h"
#include "PatchCFG.h"

using Dyninst::PatchAPI::InstrumenterPtr;
using Dyninst::PatchAPI::Instrumenter;
using Dyninst::PatchAPI::InstanceSet;
using Dyninst::PatchAPI::AddrSpacePtr;
using Dyninst::PatchAPI::PatchFunction;
using Dyninst::PatchAPI::PatchBlock;

/* Default implementation of Instrumenter */

InstrumenterPtr
Instrumenter::create(AddrSpacePtr as) {
  InstrumenterPtr ret = InstrumenterPtr(new Instrumenter(as));
  if (!ret) return InstrumenterPtr();
  return ret;
}
/*
bool
Instrumenter::process() {
  return true;
}
*/
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

bool
Instrumenter::modifyCall(PatchBlock *callBlock, PatchFunction *newCallee, PatchFunction *context) {
  callModifications_[callBlock][context] = newCallee;
  return true;
}

bool
Instrumenter::revertModifiedCall(PatchBlock *callBlock, PatchFunction *context) {
  if (callModifications_.find(callBlock) != callModifications_.end()) {
    callModifications_[callBlock].erase(context);
  }
  return true;
}

bool
Instrumenter::removeCall(PatchBlock *callBlock, PatchFunction *context) {
  modifyCall(callBlock, NULL, context);
  return true;
}
