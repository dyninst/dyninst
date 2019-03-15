/*
 * See the dyninst/COPYRIGHT file for copyright information.
 * 
 * We provide the Paradyn Tools (below described as "Paradyn")
 * on an AS IS basis, and do not warrant its validity or performance.
 * We reserve the right to update, modify, or discontinue this
 * software at any time.  We shall have no obligation to supply such
 * updates or modifications or any other form of support to you.
 * 
 * By your use of Paradyn, you understand and agree that we (or any
 * other person or entity with proprietary rights in Paradyn) are
 * under no obligation to provide either maintenance services,
 * update services, notices of latent defects, or correction of
 * defects for Paradyn.
 * 
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */
#include "Point.h"
#include "Command.h"
#include "Snippet.h"
#include "PatchMgr.h"
#include "Instrumenter.h"

using Dyninst::PatchAPI::Point;
using Dyninst::PatchAPI::Patcher;
using Dyninst::PatchAPI::Command;
using Dyninst::PatchAPI::SnippetPtr;
using Dyninst::PatchAPI::PatchBlock;
using Dyninst::PatchAPI::InstancePtr;
using Dyninst::PatchAPI::BatchCommand;
using Dyninst::PatchAPI::PatchFunction;
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
    return false;
  }
  return true;
}

BatchCommand* BatchCommand::create() {
  BatchCommand* ret = new BatchCommand;
  return ret;
}

/* Batch Command */

void BatchCommand::add(Command* c) {
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
  Instrumenter* inst = mgr_->instrumenter();
  add(inst);


  CommandList::iterator i = to_do_.begin();
  while (i != to_do_.end()) {
     done_.push_front(*i);

    // Add all commands before instrumenter to instrumenter's user_commans_
    if (*i != inst) {
       inst->user_commands_.push_back(*i);
    }

    if (!(*i)->run())  {
       return false;
    }

    to_do_.erase(i++);
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

