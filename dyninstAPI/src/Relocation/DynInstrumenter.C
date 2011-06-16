#include "DynInstrumenter.h"
#include "BPatch_point.h"
#include "BPatch_addressSpace.h"

using Dyninst::PatchAPI::DynInstrumenter;
using Dyninst::PatchAPI::DynInsertSnipCommand;
using Dyninst::PatchAPI::DynRemoveSnipCommand;
using Dyninst::PatchAPI::DynReplaceFuncCommand;
using Dyninst::PatchAPI::DynModifyCallCommand;
using Dyninst::PatchAPI::DynRemoveCallCommand;

/* Instrumenter Command, which is called implicitly by Patcher's run()  */
bool DynInstrumenter::run() {
  DynAddrSpacePtr das = DYN_CAST(DynAddrSpace, as_);
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
  mini_ = pt->insert(order, ast, recursive);
}

DynInsertSnipCommand::Ptr DynInsertSnipCommand::create(instPoint* pt, callOrder order,
                      AstNodePtr ast, bool recursive) {
  return Ptr(new DynInsertSnipCommand(pt, order, ast, recursive));
}

bool DynInsertSnipCommand::run() {
  return true;
}

bool DynInsertSnipCommand::undo() {
  /* TODO(wenbin) */
   return true;
}

/* Remove Snippet Command */
DynRemoveSnipCommand::DynRemoveSnipCommand(miniTramp* mini) : mini_(mini) {
}

DynRemoveSnipCommand::Ptr DynRemoveSnipCommand::create(miniTramp* mini) {
  return Ptr(new DynRemoveSnipCommand(mini));
}

bool DynRemoveSnipCommand::run() {
  mini_->instP()->erase(mini_);
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

DynReplaceFuncCommand::Ptr DynReplaceFuncCommand::create(AddressSpace* as,
                      func_instance* old_func,
                      func_instance* new_func) {
  return Ptr(new DynReplaceFuncCommand(as, old_func, new_func));
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

DynModifyCallCommand::Ptr DynModifyCallCommand::create(AddressSpace* as,
                      block_instance* block,
                      func_instance* new_func,
                      func_instance* context) {
      return Ptr(new DynModifyCallCommand(as, block, new_func, context));
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

DynRemoveCallCommand::Ptr DynRemoveCallCommand::create(AddressSpace* as,
                      block_instance* block,
                      func_instance* context) {
      return Ptr(new DynRemoveCallCommand(as, block, context));
}


bool DynRemoveCallCommand::run() {
  as_->removeCall(block_, context_);
  return true;
}

bool DynRemoveCallCommand::undo() {
  as_->revertCall(block_, context_);
  return true;
}
