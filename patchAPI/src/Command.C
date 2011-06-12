#include "Command.h"

using Dyninst::PatchAPI::Command;
using Dyninst::PatchAPI::CommandPtr;
using Dyninst::PatchAPI::BatchCommand;
using Dyninst::PatchAPI::BatchCommandPtr;
using Dyninst::PatchAPI::PushFrontCommand;
using Dyninst::PatchAPI::PushBackCommand;
using Dyninst::PatchAPI::RemoveSnippetCommand;
using Dyninst::PatchAPI::RemoveCallCommand;
using Dyninst::PatchAPI::ReplaceCallCommand;
using Dyninst::PatchAPI::ReplaceFuncCommand;
using Dyninst::PatchAPI::Patcher;

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
  // The instrumentation engine
  // add()
  for (CommandList::iterator i = to_do_.begin(); i != to_do_.end(); i++) {
    to_do_.erase(i);
    done_.push_front(*i);
    if (!(*i)->run()) return false;
  }
  return true;
}

/* Public Interface: Insert Snippet by pushing the the front of
   snippet instance list */

bool PushFrontCommand::run() {
  return true;
}

bool PushFrontCommand::undo() {
  return true;
}

/* Public Interface: Insert Snippet by pushing the the end of
   snippet instance list */

bool PushBackCommand::run() {
  return true;
}

bool PushBackCommand::undo() {
  return true;
}

/* Public Interface: Remove Snippet */

bool RemoveSnippetCommand::run() {
  return true;
}

bool RemoveSnippetCommand::undo() {
  return true;
}

bool RemoveCallCommand::run() {
  return true;
}

/* Public Interface: Remove Function Call */

bool RemoveCallCommand::undo() {
  return true;
}

bool ReplaceCallCommand::run() {
  return true;
}

/* Public Interface: Replace Function Call */

bool ReplaceCallCommand::undo() {
  return true;
}

bool ReplaceFuncCommand::run() {
  return true;
}

/* Public Interface: Replace Function */

bool ReplaceFuncCommand::undo() {
  return true;
}

