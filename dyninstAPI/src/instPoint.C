/*
 * Copyright (c) 1996-2009 Barton P. Miller
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
#include "dyninstAPI/src/image-func.h"
#include "common/h/arch.h"
#include "dyninstAPI/src/mapped_object.h"
#include "dyninstAPI/src/emitter.h"
#if defined(arch_x86_64)
// For 32/64-bit mode knowledge
#include "dyninstAPI/src/emit-x86.h"
#endif

// Creation methods!

instPoint *instPoint::funcEntry(int_function *f) {
   return new instPoint(FunctionEntry, f);
}

instPoint *instPoint::funcExit(int_block *b) {
   if (!b->isExit()) return NULL;
   return new instPoint(FunctionExit, b);
}

instPoint *instPoint::blockEntry(int_block *b) {
   return new instPoint(BlockEntry, b);
}

instPoint *instPoint::edge(int_edge *e) {
   return new instPoint(Edge, e);
}

instPoint *instPoint::preInsn(int_block *b,
                              Instruction::Ptr insn,
                              Address a,
                              bool trusted) {
   if (!trusted || !insn) {
      if (!checkInsn(b, insn, a)) return NULL;
   }
   
   return new instPoint(PreInsn, b, insn, a);
}

instPoint *instPoint::postInsn(int_block *b,
                               Instruction::Ptr insn, 
                               Address a, 
                               bool trusted) {
   if (!trusted || !insn) {
      if (!checkInsn(b, insn, a)) return NULL;
   }
   return new instPoint(PostInsn, b, insn, a);
}

instPoint *instPoint::preCall(int_block *b) {
   if (!b->containsCall()) return NULL;
   return new instPoint(PreCall, b);
}

instPoint *instPoint::postCall(int_block *b) {
   if (!b->containsCall()) return NULL;
   return new instPoint(PostCall, b);
}

instPoint::instPoint(Type t, int_function *f) :
   type_(t),
   func_(f),
   block_(NULL),
   edge_(NULL),
   addr_(0),
   recursive_(false),
   baseTramp_(NULL) {};

instPoint::instPoint(Type t, int_block *b) :
   type_(t), func_(NULL), block_(b), edge_(NULL), addr_(0), recursive_(false), baseTramp_(NULL) {};

instPoint::instPoint(Type t, int_edge *e) :
   type_(t), func_(NULL), block_(NULL), edge_(e), addr_(0), recursive_(false), baseTramp_(NULL) {};

instPoint::instPoint(Type t, int_block *b, Instruction::Ptr insn, Address a) :
   type_(t), func_(NULL), block_(b), edge_(NULL), insn_(insn), addr_(a), recursive_(false), baseTramp_(NULL) {};

instPoint *instPoint::fork(instPoint *, AddressSpace *) {
   assert(0);
   return NULL;
}


instPoint::~instPoint() {
   // Delete miniTramps? 
   // Uninstrument?
   for (iterator iter = begin(); iter != end(); ++iter)
      delete *iter;
   tramps_.clear();
};


instPoint::iterator instPoint::begin() { return tramps_.begin(); }
instPoint::iterator instPoint::end() { return tramps_.end(); }
instPoint::const_iterator instPoint::begin() const { return tramps_.begin(); }
instPoint::const_iterator instPoint::end() const { return tramps_.end(); }
bool instPoint::empty() const { return tramps_.empty(); }

AddressSpace *instPoint::proc() const { 
   return func()->proc();
}

int_function *instPoint::func() const { 
   if (func_) return func_;
   if (block_) return block_->func();
   if (edge_) return edge_->func();
   assert(0); return NULL;
}

miniTramp *instPoint::push_front(AstNodePtr ast) {
   miniTramp *newTramp = new miniTramp(ast, this);
   tramps_.push_front(newTramp);
   markModified();

   return newTramp;
}

miniTramp *instPoint::push_back(AstNodePtr ast) {
   miniTramp *newTramp = new miniTramp(ast, this);
   tramps_.push_back(newTramp);

   markModified();

   return newTramp;
}

miniTramp *instPoint::insert(iterator loc, AstNodePtr ast) {
   miniTramp *newTramp = new miniTramp(ast, this);
   tramps_.insert(loc, newTramp);

   markModified();

   return newTramp;
}

miniTramp *instPoint::insert(callOrder order, AstNodePtr ast) {
   if (order == orderFirstAtPoint) return push_front(ast);
   else return push_back(ast);
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

bool instPoint::checkInsn(int_block *b,
                          Instruction::Ptr &insn,
                          Address a) {
   int_block::InsnInstances insns;
   b->getInsns(insns);
   for (unsigned i = 0; i < insns.size(); ++i) {
      if (a == insns[i].second) {
         // Check equality...
         insn = insns[i].first;
         return true;
      }
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
int_block *instPoint::nextExecutedBlock() const {
   switch (type_) {
      case FunctionEntry:
         return func_->entryBlock();
      case Edge:
         return edge_->trg();
      case PreInsn:
      case PostInsn:
      case FunctionExit:
      case BlockEntry:
      case PreCall:
         return block_;
      case PostCall:
        return block_->getFallthrough();
      default:
         return NULL;
   }
}   

Address instPoint::nextExecutedAddr() const {
   // As the above, but our best guess at an address
   switch (type_) {
      case FunctionEntry:
         return func_->addr();
      case FunctionExit:
         // Not correct, but as close as we can get
         return block_->last(); 
      case BlockEntry:
         return block_->start();
      case Edge:
         return edge_->trg()->start();
      case PreInsn:
         return addr_;
      case PostInsn:
         // This gets weird for things like jumps...
         return addr_ + insn_->size();
      case PreCall:
         return block_->last();
      case PostCall: {
         int_block *ft = block_->getFallthrough();
         if (ft) 
            return ft->start();
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
