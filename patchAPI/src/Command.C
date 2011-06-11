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

void BatchCommand::add(CommandPtr c) {
  to_do_.push_back(c);
}

void BatchCommand::remove(CommandList::iterator c) {
  to_do_.erase(c);
}

bool BatchCommand::run() {
  for (CommandList::iterator i = to_do_.begin(); i != to_do_.end(); i++) {
    to_do_.erase(i);
    done_.push_front(*i);
    if (!(*i)->run()) return false;
  }
  return true;
}

bool BatchCommand::undo() {
  for (CommandList::iterator i = done_.begin(); i != done_.end(); i++) {
    done_.erase(i);
    if (!(*i)->undo()) return false;
  }
  return true;
}

bool BatchCommand::run() {
  // The instrumentation engine
  // add()
  for (CommandList::iterator i = to_do_.begin(); i != to_do_.end(); i++) {
    to_do_.erase(i);
    done_.push_front(*i);
    if (!(*i)->run()) return false;
  }
  return true;
}

bool PushFrontCommand::run() {
  return true;
}

bool PushFrontCommand::undo() {
  return true;
}

bool PushBackCommand::run() {
  return true;
}

bool PushBackCommand::undo() {
  return true;
}

bool RemoveSnippetCommand::run() {
  return true;
}

bool RemoveSnippetCommand::undo() {
  return true;
}

bool RemoveCallCommand::run() {
  return true;
}

bool RemoveCallCommand::undo() {
  return true;
}

bool ReplaceCallCommand::run() {
  return true;
}

bool ReplaceCallCommand::undo() {
  return true;
}

bool ReplaceFuncCommand::run() {
  return true;
}

bool ReplaceFuncCommand::undo() {
  return true;
}

