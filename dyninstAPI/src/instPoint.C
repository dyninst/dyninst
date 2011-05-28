/*
 * Copyright (c) 1996-2011 Barton P. Miller
 *
 * We provide the Paradyn Parallel Performance Tools (below
 * described as "Paradyn") on an AS IS basis, and do not warrant its
 * validity or performance.  We reserve the right to update, modify,
 * or discontinue this software at any time.  We shall have no
 * obligation to supply such updates or modifications or any other
 * form of support to you.
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

// $Id: instPoint.C,v 1.55 2008/09/08 16:44:03 bernat Exp $
// instPoint code


#include <assert.h>
#include "dyninstAPI/src/symtab.h"
#include "dyninstAPI/src/process.h"
#include "dyninstAPI/src/inst.h"
#include "dyninstAPI/src/instP.h"
#include "dyninstAPI/src/ast.h"
#include "dyninstAPI/src/util.h"
#include "common/h/stats.h"
#include "dyninstAPI/src/debug.h"
#include "dyninstAPI/src/instPoint.h"
#include "dyninstAPI/src/miniTramp.h"
#include "dyninstAPI/src/baseTramp.h"

#include "instructionAPI/h/InstructionDecoder.h"
using namespace Dyninst::InstructionAPI;

#include "dyninstAPI/src/function.h"
#include "dyninstAPI/src/parse-cfg.h"
#include "common/h/arch.h"
#include "dyninstAPI/src/mapped_object.h"
#include "dyninstAPI/src/emitter.h"
#if defined(arch_x86_64)
// For 32/64-bit mode knowledge
#include "dyninstAPI/src/emit-x86.h"
#endif

instPoint *instPoint::funcEntry(func_instance *f) {
  return f->funcEntryPoint(true);
}

instPoint *instPoint::funcExit(func_instance *f, block_instance *b) {
  return f->funcExitPoint(b, true);
}

instPoint *instPoint::blockEntry(func_instance *f, block_instance *b) {
  return f->blockEntryPoint(b, true);
}

instPoint *instPoint::blockExit(func_instance *f, block_instance *b) {
  return f->blockExitPoint(b, true);
}

instPoint *instPoint::preCall(func_instance *f, block_instance *b) {
  return f->preCallPoint(b, true);
}

instPoint *instPoint::postCall(func_instance *f, block_instance *b) {
  return f->postCallPoint(b, true);
}

instPoint *instPoint::edge(func_instance *f, edge_instance *e) {
  //   return f->findPoint(EdgeDuring, e, true);
  return f->edgePoint(e, true);
}

instPoint *instPoint::preInsn(func_instance *f,
                              block_instance *b,
                              Address a,
                              InstructionAPI::Instruction::Ptr ptr,
                              bool trusted) {
  return f->preInsnPoint(b, a, ptr, trusted, true);
}

instPoint *instPoint::postInsn(func_instance *f,
                               block_instance *b,
                               Address a,
                               InstructionAPI::Instruction::Ptr ptr,
                               bool trusted) {
  return f->postInsnPoint(b, a, ptr, trusted, true);
}


instPoint::instPoint(Address       addr,
                     Type          t,
                     PatchMgrPtr   mgr,
                     func_instance *f) :
  Point(addr, t, mgr, f),
  baseTramp_(NULL) {
  func_ = SCAST_FI(the_func_);
  block_ = SCAST_BI(the_block_);
  edge_ = SCAST_EI(the_edge_);
};

instPoint::instPoint(Address       addr,
                     Type          t,
                     PatchMgrPtr   mgr,
                     block_instance *b) :
  Point(addr, t, mgr, b),
  baseTramp_(NULL) {
  func_ = SCAST_FI(the_func_);
  block_ = SCAST_BI(the_block_);
  edge_ = SCAST_EI(the_edge_);
};

instPoint::instPoint(Address       addr,
                     Type          t,
                     PatchMgrPtr   mgr,
                     Address*      scope) :
  Point(addr, t, mgr, scope),
  baseTramp_(NULL) {
  func_ = SCAST_FI(the_func_);
  block_ = SCAST_BI(the_block_);
  edge_ = SCAST_EI(the_edge_);
};

instPoint::instPoint(Address       addr,
                     Type          t,
                     PatchMgrPtr   mgr,
                     edge_instance *e) :
  Point(addr, t, mgr, e),
  baseTramp_(NULL) {
  func_ = SCAST_FI(the_func_);
  block_ = SCAST_BI(the_block_);
  edge_ = SCAST_EI(the_edge_);
};


// If there is a logical "pair" (e.g., before/after) of instPoints return them.
// The return result is a pair of <before, after>
std::pair<instPoint *, instPoint *> instPoint::getInstpointPair(instPoint *i) {
  patch_cerr << "getInstpointPair\n";
   switch(i->type()) {
      case None:
         assert(0);
         return std::pair<instPoint *, instPoint *>(NULL, NULL);
      case PreInsn:
         return std::pair<instPoint *, instPoint *>(i,
                                                    postInsn(i->func(),
                                                             i->block(),
                                                             i->insnAddr(),
                                                             i->insn(),
                                                             true));
      case PostInsn:
         return std::pair<instPoint *, instPoint *>(preInsn(i->func(),
                                                            i->block(),
                                                            i->insnAddr(),
                                                            i->insn(),
                                                            true),
                                                    i);
      case PreCall:
         return std::pair<instPoint *, instPoint *>(i,
                                                    postCall(i->func(),
                                                             i->block()));
      case PostCall:
         return std::pair<instPoint *, instPoint *>(preCall(i->func(),
                                                            i->block()),
                                                    i);
      default:
         return std::pair<instPoint *, instPoint *>(i, NULL);
   }
   assert(0);
   return std::pair<instPoint *, instPoint *>(NULL, NULL);
}

instPoint *instPoint::fork(instPoint *parent, AddressSpace *child) {
   // Return the equivalent instPoint within the child process
   func_instance *f = parent->func_ ? child->findFunction(parent->func_->ifunc()) : NULL;
   block_instance *b = parent->block_ ? child->findBlock(parent->block_->llb()) : NULL;
   edge_instance *e = parent->edge_ ? child->findEdge(parent->edge_->edge()) : NULL;
   Instruction::Ptr i = parent->insn_;
   Address a = parent->addr_;

   instPoint *point = NULL;

   switch(parent->type_) {
      case None:
         assert(0);
         break;
      case FuncEntry:
         point = funcEntry(f);
         break;
      case FuncExit:
         point = funcExit(f, b);
         break;
      case BlockEntry:
         point = blockEntry(f, b);
         break;
      case BlockExit:
         point = blockExit(f, b);
         break;
      case EdgeDuring:
         point = edge(f, e);
         break;
      case PreInsn:
         point = preInsn(f, b, a, i, true);
         break;
      case PostInsn:
         point = postInsn(f, b, a, i, true);
         break;
      case PreCall:
         point = preCall(f, b);
         break;
      case PostCall:
         point = postCall(f, b);
         break;
      case OtherPoint:
   case InsnTaken:
   case BlockDuring:
   case FuncDuring:
   case LoopStart:
   case LoopEnd:
   case LoopIterStart:
   case LoopIterEnd:
   case InsnTypes:
   case BlockTypes:
   case FuncTypes:
   case LoopTypes:
   case CallTypes:
         assert(0);
         break;
   }
   assert(point->empty() ||
          point->size() == parent->size());
   if (point->empty()) {
      for (const_iterator iter = parent->begin(); iter != parent->end(); ++iter) {
         point->push_back((*iter)->ast(), (*iter)->recursive());
      }
   }

   point->liveRegs_ = parent->liveRegs_;

   return point;
}


instPoint::~instPoint() {
   // Delete miniTramps?
   // Uninstrument?
   for (iterator iter = begin(); iter != end(); ++iter)
      delete *iter;
   tramps_.clear();
   if (baseTramp_) delete baseTramp_;

};


instPoint::iterator instPoint::begin() { return tramps_.begin(); }
instPoint::iterator instPoint::end() { return tramps_.end(); }
instPoint::const_iterator instPoint::begin() const { return tramps_.begin(); }
instPoint::const_iterator instPoint::end() const { return tramps_.end(); }
bool instPoint::empty() const { return tramps_.empty(); }
unsigned instPoint::size() const { return tramps_.size(); }

AddressSpace *instPoint::proc() const {
   return func()->proc();
}

func_instance *instPoint::func() const {
   if (func_) return func_;
   return NULL;
}

miniTramp *instPoint::push_front(AstNodePtr ast, bool recursive) {
   miniTramp *newTramp = new miniTramp(ast, this, recursive);
   tramps_.push_front(newTramp);
   markModified();

   return newTramp;
}

miniTramp *instPoint::push_back(AstNodePtr ast, bool recursive) {
   miniTramp *newTramp = new miniTramp(ast, this, recursive);
   tramps_.push_back(newTramp);

   markModified();

   return newTramp;
}

miniTramp *instPoint::insert(iterator loc, AstNodePtr ast, bool recursive) {
   miniTramp *newTramp = new miniTramp(ast, this, recursive);
   tramps_.insert(loc, newTramp);

   markModified();

   return newTramp;
}

miniTramp *instPoint::insert(callOrder order, AstNodePtr ast, bool recursive) {
   if (order == orderFirstAtPoint) return push_front(ast, recursive);
   else return push_back(ast, recursive);
}

void instPoint::erase(iterator loc) {

   markModified();

   tramps_.erase(loc);
}

void instPoint::erase(miniTramp *m) {
   for (iterator iter = begin(); iter != end(); ++iter) {
      if ((*iter) == m) {
         markModified();
         tramps_.erase(iter);
         return;
      }
   }
}

bool instPoint::checkInsn(block_instance *b,
                          Instruction::Ptr &insn,
                          Address a) {
   block_instance::Insns insns;
   b->getInsns(insns);
   block_instance::Insns::iterator iter = insns.find(a);
   if (iter != insns.end()) {
      insn = iter->second;
      return true;
   }
   return false;
}


baseTramp *instPoint::tramp() {
   if (!baseTramp_) {
      baseTramp_ = baseTramp::create(this);
   }

   return baseTramp_;
}

// Returns the current block (if there is one)
// or the next block we're going to execute (if not).
// In some cases we may not know; function exit points
// and the like. In this case we return the current block
// as a "well, this is what we've got..."
block_instance *instPoint::nextExecutedBlock() const {
   switch (type_) {
      case FuncEntry:
         return func_->entryBlock();
      case EdgeDuring:
         return edge_->trg();
      case PreInsn:
      case PostInsn:
      case FuncExit:
      case BlockEntry:
      case PreCall:
         return block_;
      case PostCall: {
         edge_instance *ftE = block_->getFallthrough();
         if (ftE &&
             (!ftE->sinkEdge()))
            return ftE->trg();
         else
            return NULL;
      }
      default:
         return NULL;
   }
}

Address instPoint::nextExecutedAddr() const {
   // As the above, but our best guess at an address
   switch (type_) {
      case FuncEntry:
         return func_->addr();
      case FuncExit:
         // Not correct, but as close as we can get
         return block_->last();
      case BlockEntry:
         return block_->start();
      case EdgeDuring:
         return edge_->trg()->start();
      case PreInsn:
         return addr_;
      case PostInsn:
         // This gets weird for things like jumps...
         return addr_ + insn_->size();
      case PreCall:
         return block_->last();
      case PostCall: {
         edge_instance *ftE = block_->getFallthrough();
         if (ftE &&
             (!ftE->sinkEdge()))
            return ftE->trg()->start();
         else
            return block_->end();
      }
      default:
         return 0;
   }
}

void instPoint::markModified() {
   proc()->addModifiedFunction(func());
}

BlockInstpoints::~BlockInstpoints() {
   if (entry) delete entry;
   if (exit) delete exit;
   if (preCall) delete preCall;
   if (postCall) delete postCall;
   for (InsnInstpoints::iterator iter = preInsn.begin();
        iter != preInsn.end(); ++iter) {
      if (iter->second) delete iter->second;
   }

   for (InsnInstpoints::iterator iter = postInsn.begin();
        iter != postInsn.end(); ++iter) {
      if (iter->second) delete iter->second;
   }
}

FuncInstpoints::~FuncInstpoints() {
   if (entry) delete entry;
   for (std::map<block_instance *, instPoint *>::iterator iter = exits.begin();
        iter != exits.end(); ++iter) {
      if (iter->second) delete iter->second;
   }
}

std::string instPoint::format() const {
   stringstream ret;
   ret << "iP(";
   switch(type_) {
      case FuncEntry:
         ret << "FEntry";
         break;
      case FuncExit:
         ret << "FExit";
         break;
      case BlockEntry:
         ret << "BEntry";
         break;
      case BlockExit:
         ret << "BExit";
         break;
      case EdgeDuring:
         ret << "E";
         break;
      case PreInsn:
         ret << "PreI";
         break;
      case PostInsn:
         ret << "PostI";
         break;
      case PreCall:
         ret << "PreC";
         break;
      case PostCall:
         ret << "PostC";
         break;
      default:
         ret << "???";
         break;
   }
   if (func_) {
      ret << ", Func(" << func_->name() << ")";
   }
   if (block_) {
      ret << ", Block(" << hex << block_->start() << dec << ")";
   }
   if (edge_) {
      ret << ", Edge";
   }
   if (addr_) {
      ret << ", Addr(" << hex << addr_ << dec << ")";
   }
   if (insn_) {
      ret << ", Insn(" << insn_->format() << ")";
   }
   ret << ")";
   return ret.str();
}
