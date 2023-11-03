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
/* Public Interface */

#include "Point.h"
#include "PatchMgr.h"
#include "PatchObject.h"
#include "PatchCFG.h"
#include "PatchCallback.h"

using namespace Dyninst;
using namespace Dyninst::PatchAPI;

using Dyninst::PatchAPI::Instance;
using Dyninst::PatchAPI::InstancePtr;
using Dyninst::PatchAPI::Point;
using Dyninst::PatchAPI::PatchFunction;
using Dyninst::PatchAPI::PointMaker;
using Dyninst::ParseAPI::CodeObject;
using Dyninst::ParseAPI::CodeSource;
using Dyninst::ParseAPI::CodeRegion;
using Dyninst::InstructionAPI::InstructionDecoder;
using Dyninst::PatchAPI::SnippetPtr;
using Dyninst::PatchAPI::SnippetType;
using Dyninst::PatchAPI::PatchMgrPtr;
using Dyninst::PatchAPI::SnippetState;
using Dyninst::PatchAPI::PatchBlock;
using Dyninst::PatchAPI::PatchEdge;

InstancePtr
Instance::create(Point* point, SnippetPtr snippet,
    SnippetType type, SnippetState state) {
  InstancePtr ret = InstancePtr(new Instance(point, snippet));
  if (!ret) return InstancePtr();
  ret->state_ = state;
  ret->type_ = type;
  return ret;
}

bool
Instance::destroy() {
  if (point_) {
    bool ret = point_->remove(shared_from_this());
    return ret;
  }
  return false;
}

/* If the Point is PreCall or PostCall */
PatchFunction*
Point::getCallee() {
  if (type() != PreCall && type() != PostCall) return NULL;
  PatchBlock* b = the_block_;
  PatchBlock::edgelist::const_iterator it = b->targets().begin();
  for (; it != b->targets().end(); ++it) {
    if ((*it)->type() == ParseAPI::CALL ||
        ((((*it)->type() == ParseAPI::DIRECT || (*it)->type() == ParseAPI::COND_TAKEN)) && (*it)->interproc())) {
      PatchBlock* trg = (*it)->trg();
      return obj()->getFunc(obj()->co()->findFuncByEntry(trg->block()->region(),
                                                         trg->block()->start()));
    }
  }
  return NULL;
}

/* Associate this point with the block(s) and function(s)
   that contain it */
void
Point::initCodeStructure() {
  assert(mgr_);

  cb()->create(this);
}

PatchObject *Point::obj() const {
   if (the_func_) { 
      return the_func_->obj();
   }
   else if (the_block_) {
      return the_block_->obj();
   }
   else if (the_edge_) {
      return the_edge_->src()->obj();
   }
   return NULL;
}

/* for single instruction */
Point::Point(Point::Type type, PatchMgrPtr mgr, PatchBlock *b, Dyninst::Address a, InstructionAPI::Instruction i, PatchFunction *f)
   :addr_(a), type_(type), mgr_(mgr), the_block_(b), the_edge_(NULL), the_func_(f), insn_(i) {

  initCodeStructure();
}

/* for a block */
Point::Point(Type type, PatchMgrPtr mgr, PatchBlock* blk, PatchFunction *f)
  : addr_(0), type_(type), mgr_(mgr), the_block_(blk), the_edge_(NULL), the_func_(f) {
  initCodeStructure();
}

/* for an edge */
Point::Point(Type type, PatchMgrPtr mgr, PatchEdge* edge, PatchFunction *f)
  : addr_(0), type_(type), mgr_(mgr), the_block_(NULL), the_edge_(edge), the_func_(f) {
  initCodeStructure();
}

/* for a function */
Point::Point(Type type, PatchMgrPtr mgr, PatchFunction* func) : 
   addr_(0), type_(type), mgr_(mgr),
   the_block_(NULL), the_edge_(NULL), the_func_(func) {
  initCodeStructure();
}

/* for a call or exit site */
Point::Point(Type type, PatchMgrPtr mgr, PatchFunction* func, PatchBlock *b) : 
   addr_(0), type_(type), mgr_(mgr),
   the_block_(b), the_edge_(NULL), the_func_(func) {
  initCodeStructure();
}


/* old_instance, old_instance, <---new_instance */
InstancePtr
Point::pushBack(SnippetPtr snippet) {
  InstancePtr instance = Instance::create(this, snippet);
  if (!instance) return instance;
  instanceList_.push_back(instance);
  instance->set_state(INSERTED);
  return instance;
}

/* new_instance--->, old_instance, old_instance */
InstancePtr
Point::pushFront(SnippetPtr snippet) {
  InstancePtr instance = Instance::create(this, snippet);
  if (!instance) return instance;
  instanceList_.push_front(instance);
  instance->set_state(INSERTED);
  return instance;
}

/* Test whether the type contains a specific type. */
bool
Point::TestType(Point::Type types, Point::Type trg) {
  if (types & trg) return true;
  return false;
}

/* Add a specific type to a set of types */
void
Point::AddType(Point::Type& types, Point::Type trg) {
  int trg_int = static_cast<int>(trg);
  int type_int = static_cast<int>(types);
  type_int |= trg_int;
  types = (Point::Type)type_int;
}

/* Remove a specific type from a set of types */
void
Point::RemoveType(Point::Type& types, Point::Type trg) {
  int trg_int = static_cast<int>(trg);
  int type_int = static_cast<int>(types);
  type_int &= (~trg_int);
  types = (Point::Type)type_int;
}

bool
Point::remove(InstancePtr instance) {
  if (instance == InstancePtr()) return false;
  InstanceList::iterator it = std::find(instanceList_.begin(),
                                 instanceList_.end(), instance);
  if (it != instanceList_.end()) {
    instanceList_.erase(it);
    instance->set_state(PENDING);
    return true;
  }
  return false;
}

size_t
Point::size() {
  return instanceList_.size();
}

void
Point::clear() {
  while (size() > 0) {
    InstancePtr i = instanceList_.back();
    i->destroy();
  }
}

/* 1, Clear all snippet instances
   2, Detach from PatchMgr object */
bool
Point::destroy() {
  clear();

  return true;
}

Point::~Point() {
  // Clear all instances associated with this point
  clear();
}

void Point::changeBlock(PatchBlock *block) {
   // TODO: callback from here
   PatchBlock *old = the_block_;
   the_block_ = block;
   cb()->change(this, old, block);
}

FuncPoints::~FuncPoints() {
   if (entry) delete entry;
   if (during) delete during;
   for (std::map<PatchBlock *, Point *>::iterator iter = exits.begin();
        iter != exits.end(); ++iter) {
      delete iter->second;
   }
   for (std::map<PatchBlock *, Point *>::iterator iter = preCalls.begin();
        iter != preCalls.end(); ++iter) {
      delete iter->second;
   }
   for (std::map<PatchBlock *, Point *>::iterator iter = postCalls.begin();
        iter != postCalls.end(); ++iter) {
      delete iter->second;
   }
}

BlockPoints::~BlockPoints() {
   if (entry) delete entry;
   if (during) delete during;
   if (exit) delete exit;
   for (InsnPoints::iterator iter = preInsn.begin(); iter != preInsn.end(); ++iter) {
      delete iter->second;
   }
   for (InsnPoints::iterator iter = postInsn.begin(); iter != postInsn.end(); ++iter) {
      delete iter->second;
   }
}

PatchCallback *Point::cb() const { 
   if (the_func_) return the_func_->cb();
   else if (the_block_) return the_block_->cb();
   else if (the_edge_) return the_edge_->cb();
   else return NULL;
}

bool Point::consistency() const {
   if (!obj()) return false;

   // Assert that our data matches our type.
   switch (type()) {
      case PreInsn:
      case PostInsn:
         if (!insn().isValid()) return false;
         if (!addr()) return false;
         if (!block()) return false;
         // Can have a function or not, that's okay
         if (edge()) return false;
         break;
      case BlockEntry:
      case BlockExit:
      case BlockDuring:
         if (insn().isValid()) return false;
         if (addr()) return false;
         if (!block()) return false;
         if (edge()) return false;
         break;
      case FuncEntry:
      case FuncDuring:
         if (insn().isValid()) return false;
         if (addr()) return false;
         if (block()) return false;
         if (edge()) return false;
         if (!func()) return false;
         break;
      case PreCall:
      case PostCall:
      case FuncExit:
         if (insn().isValid()) return false;
         if (addr()) return false;
         if (!block()) return false;
         if (edge()) return false;
         if (!func()) return false;
         break;
      case EdgeDuring:
         if (insn().isValid()) return false;
         if (addr()) return false;
         if (block()) return false;
         if (!edge()) return false;
         if (!func()) return false;
         break;
      default:
         return false;
   }
   return true;
}
