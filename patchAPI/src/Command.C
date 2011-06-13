#include "Point.h"
#include "Command.h"
#include "Snippet.h"
#include "PatchMgr.h"
#include "Instrumenter.h"

using Dyninst::PatchAPI::Point;
using Dyninst::PatchAPI::Patcher;
using Dyninst::PatchAPI::Command;
using Dyninst::PatchAPI::CommandPtr;
using Dyninst::PatchAPI::SnippetPtr;
using Dyninst::PatchAPI::PatchBlock;
using Dyninst::PatchAPI::InstancePtr;
using Dyninst::PatchAPI::BatchCommand;
using Dyninst::PatchAPI::PatchFunction;
using Dyninst::PatchAPI::InstrumenterPtr;
using Dyninst::PatchAPI::BatchCommandPtr;
using Dyninst::PatchAPI::PushBackCommand;
using Dyninst::PatchAPI::PushFrontCommand;
using Dyninst::PatchAPI::RemoveCallCommand;
using Dyninst::PatchAPI::ReplaceCallCommand;
using Dyninst::PatchAPI::ReplaceFuncCommand;
using Dyninst::PatchAPI::RemoveSnippetCommand;

/* Basic Command */

bool Command::commit() {
  if (!run()) {
    undo();
  }
  return true;
}

BatchCommandPtr BatchCommand::create() {
  BatchCommandPtr ret = BatchCommandPtr(new BatchCommand);
  return ret;
}

/* Batch Command */

void BatchCommand::add(CommandPtr c) {
  to_do_.push_back(c);
}

void BatchCommand::remove(CommandList::iterator c) {
  to_do_.erase(c);
}

bool BatchCommand::run() {
  std::set<CommandList::iterator> remove_set;
  for (CommandList::iterator i = to_do_.begin(); i != to_do_.end();) {
    done_.push_front(*i);
    if (!(*i)->run()) { return false; }
    // Be careful! We are modifying the iterator during the loop ...
    i = to_do_.erase(i);
  }
  return true;
}

bool BatchCommand::undo() {
  for (CommandList::iterator i = done_.begin(); i != done_.end();) {
    if (!(*i)->undo()) return false;
    i = done_.erase(i);
  }
  return true;
}

/* Public Interface: Patcher, which accepts instrumentation requests from users. */

bool Patcher::run() {

  // We implicitly add the instrumentation engine
  add(mgr_->instrumenter());

  // The "common" BatchCommand stuffs
  for (CommandList::iterator i = to_do_.begin(); i != to_do_.end();) {
    done_.push_front(*i);
    if (!(*i)->run()) return false;
    i = to_do_.erase(i);
  }
  return true;
}

/* Public Interface: Insert Snippet by pushing the the front of
   snippet instance list */

bool PushFrontCommand::run() {
  instance_ = pt_->pushFront(snip_);
  return true;
}

bool PushFrontCommand::undo() {
  return pt_->remove(instance_);
}

/* Public Interface: Insert Snippet by pushing the the end of
   snippet instance list */

bool PushBackCommand::run() {
  instance_ = pt_->pushBack(snip_);
  return true;
}

bool PushBackCommand::undo() {
  return pt_->remove(instance_);
}

/* Public Interface: Remove Snippet */

bool RemoveSnippetCommand::run() {
  return instance_->destroy();
}

bool RemoveSnippetCommand::undo() {
  // TODO(wenbin)
  return true;
}

/* Public Interface: Remove Function Call */

bool RemoveCallCommand::run() {
  return mgr_->instrumenter()->removeCall(call_block_, context_);
}


bool RemoveCallCommand::undo() {
  return mgr_->instrumenter()->revertModifiedCall(call_block_, context_);
}

/* Public Interface: Replace Function Call */

bool ReplaceCallCommand::run() {
  return mgr_->instrumenter()->modifyCall(call_block_, new_callee_, context_);
}

bool ReplaceCallCommand::undo() {
  return mgr_->instrumenter()->revertModifiedCall(call_block_, context_);
}

/* Public Interface: Replace Function */

bool ReplaceFuncCommand::run() {
  return mgr_->instrumenter()->replaceFunction(old_func_, new_func_);
}

bool ReplaceFuncCommand::undo() {
  return mgr_->instrumenter()->revertReplacedFunction(old_func_);
}

