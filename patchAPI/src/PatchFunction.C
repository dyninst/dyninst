/* Public Interface */

#include "PatchCFG.h"
#include "PatchMgr.h"

using namespace Dyninst;
using namespace PatchAPI;

PatchFunction*
PatchFunction::create(ParseAPI::Function *f, PatchObject* obj) {
  return obj->getFunc(f);
}

PatchFunction::PatchFunction(ParseAPI::Function *f,
     PatchObject* o) : func_(f), obj_(o), addr_(obj_->codeBase() + func_->addr()) {}

PatchFunction::PatchFunction(const PatchFunction *parFunc, PatchObject* child)
  : func_(parFunc->func_), obj_(child), addr_(obj_->codeBase() + func_->addr()) {}

const PatchFunction::blockset&
PatchFunction::getAllBlocks() {
  if (!all_blocks_.empty()) return all_blocks_;
  // Otherwise we need to create them
  for (ParseAPI::Function::blocklist::iterator iter = func_->blocks().begin();
       iter != func_->blocks().end(); ++iter) {
    all_blocks_.insert(object()->getBlock(*iter));
  }
  return all_blocks_;
}

PatchBlock*
PatchFunction::getEntryBlock() {
  assert(object());
  assert(func_);

  ParseAPI::Block* ientry = func_->entry();
  if (!ientry) {
    // In case we haven't parsed yet ...
    getAllBlocks();
    ientry = func_->entry();
  }
  assert(ientry);
  return object()->getBlock(ientry);
}

const PatchFunction::blockset&
PatchFunction::getExitBlocks() {
  if (!exit_blocks_.empty()) return exit_blocks_;

  for (ParseAPI::Function::blocklist::iterator iter = func_->returnBlocks().begin();
       iter != func_->returnBlocks().end(); ++iter) {
    PatchBlock* pblk = object()->getBlock(*iter);
    exit_blocks_.insert(pblk);
  }
  return exit_blocks_;
}

const PatchFunction::blockset&
PatchFunction::getCallBlocks() {
  // Check the list...
  if (call_blocks_.empty()) {
    const ParseAPI::Function::edgelist &callEdges = func_->callEdges();
    for (ParseAPI::Function::edgelist::iterator iter = callEdges.begin();
         iter != callEdges.end(); ++iter) {
      ParseAPI::Block *src = (*iter)->src();
      PatchBlock *block = object()->getBlock(src);
      assert(block);
      call_blocks_.insert(block);
    }
  }
  return call_blocks_;
}

PatchFunction::~PatchFunction() {
}
