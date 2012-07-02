#include "DynInstrumenter.h"
#include "BPatch_point.h"
#include "BPatch_addressSpace.h"
#include "../function.h"
#include "../parse-cfg.h"

using Dyninst::PatchAPI::DynInstrumenter;
using Dyninst::PatchAPI::DynInsertSnipCommand;
using Dyninst::PatchAPI::DynRemoveSnipCommand;
using Dyninst::PatchAPI::DynReplaceFuncCommand;
using Dyninst::PatchAPI::DynModifyCallCommand;
using Dyninst::PatchAPI::DynRemoveCallCommand;

/* Instrumenter Command, which is called implicitly by Patcher's run()  */
bool DynInstrumenter::run() {
  DynAddrSpace* das = dynamic_cast<DynAddrSpace*>(as_);
  std::set<AddressSpace*> seen;
  bool ret = true;
  for (DynAddrSpace::AsSet::iterator i = das->asSet().begin();
       i != das->asSet().end(); i++) {
    AddressSpace* as = *i;
    if (std::find(seen.begin(), seen.end(), as) == seen.end()) {
       seen.insert(as);
       if (!as->relocate()) { ret = false; }
    }
  }
  return ret;
}

bool DynInstrumenter::undo() {
  // TODO(wenbin)
  return true;
}

/* Insert Snippet Command */

DynInsertSnipCommand::DynInsertSnipCommand(instPoint* pt, callOrder order,
                                           AstNodePtr ast, bool recursive) {
   inst_ = (order == orderFirstAtPoint) ? pt->pushFront(ast) : pt->pushBack(ast);
   if (inst_ && recursive)
      inst_->disableRecursiveGuard();
}

DynInsertSnipCommand* DynInsertSnipCommand::create(instPoint* pt, callOrder order,
                                                   AstNodePtr ast, bool recursive) {
   return new DynInsertSnipCommand(pt, order, ast, recursive);
}

bool DynInsertSnipCommand::run() {
  return true;
}

bool DynInsertSnipCommand::undo() {
  /* TODO(wenbin) */
   return true;
}

/* Remove Snippet Command */
DynRemoveSnipCommand::DynRemoveSnipCommand(Instance::Ptr inst) : inst_(inst) {
}

DynRemoveSnipCommand* DynRemoveSnipCommand::create(Instance::Ptr inst) {
   return new DynRemoveSnipCommand(inst);
}

bool DynRemoveSnipCommand::run() {
   uninstrument(inst_);
   return true;
}

bool DynRemoveSnipCommand::undo() {
  /* TODO(wenbin) */
  return true;
}

/* Replace Function Command */
DynReplaceFuncCommand::DynReplaceFuncCommand(AddressSpace* as,
                          func_instance* old_func,
                          func_instance* new_func)
      : as_(as), old_func_(old_func), new_func_(new_func) {
}

DynReplaceFuncCommand* DynReplaceFuncCommand::create(AddressSpace* as,
                      func_instance* old_func,
                      func_instance* new_func) {
  return (new DynReplaceFuncCommand(as, old_func, new_func));
}

bool DynReplaceFuncCommand::run() {
  as_->replaceFunction(old_func_, new_func_);
  return true;
}

bool DynReplaceFuncCommand::undo() {
  as_->revertReplacedFunction(old_func_);
  return true;
}

/* Modify Call Command */
DynModifyCallCommand::DynModifyCallCommand(AddressSpace* as,
                          block_instance* block,
                          func_instance* new_func,
                          func_instance* context)
      : as_(as), block_(block), new_func_(new_func), context_(context) {
}

DynModifyCallCommand* DynModifyCallCommand::create(AddressSpace* as,
                      block_instance* block,
                      func_instance* new_func,
                      func_instance* context) {
      return new DynModifyCallCommand(as, block, new_func, context);
    }

bool DynModifyCallCommand::run() {
  as_->modifyCall(block_, new_func_, context_);
  return true;
}

bool DynModifyCallCommand::undo() {
  as_->revertCall(block_, context_);
  return true;
}

/* Remove Call Command */
DynRemoveCallCommand::DynRemoveCallCommand(AddressSpace* as,
                         block_instance* block,
                         func_instance* context)
      : as_(as), block_(block), context_(context) {
}

DynRemoveCallCommand* DynRemoveCallCommand::create(AddressSpace* as,
                      block_instance* block,
                      func_instance* context) {
      return new DynRemoveCallCommand(as, block, context);
}


bool DynRemoveCallCommand::run() {
  as_->removeCall(block_, context_);
  return true;
}

bool DynRemoveCallCommand::undo() {
  as_->revertCall(block_, context_);
  return true;
}

bool DynInstrumenter::isInstrumentable(PatchFunction* f) {
  func_instance* func = static_cast<func_instance*>(f);
  if (func) {
    return func->ifunc()->isInstrumentable();
  }
  return false;
}
