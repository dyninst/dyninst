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
#include "DynInstrumenter.h"
#include "BPatch_point.h"
#include "BPatch_addressSpace.h"
#include "../function.h"
#include "../parse-cfg.h"
#include "Snippet.h"

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
DynRemoveSnipCommand::DynRemoveSnipCommand(Dyninst::PatchAPI::Instance::Ptr inst) : inst_(inst) {
}

DynRemoveSnipCommand* DynRemoveSnipCommand::create(Dyninst::PatchAPI::Instance::Ptr inst) {
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
