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
 
#include "function.h"
#include "parse-cfg.h"
#include "process.h"

#include "mapped_object.h"
#include "mapped_module.h"
#include "InstructionDecoder.h"
#include <set>
#include <sstream>
#include "Relocation/Transformers/Movement-analysis.h"
using namespace Dyninst;
using namespace Dyninst::ParseAPI;

block_instance::block_instance(ParseAPI::Block *ib, 
                               mapped_object *obj) 
   : obj_(obj),
     block_(static_cast<parse_block *>(ib))
{
   // We create edges lazily
};

// Fork constructor
block_instance::block_instance(const block_instance *parent, mapped_object *childObj) 
   : obj_(childObj),
     block_(parent->block_) {
   // We also need to copy edges.
   // Thing is, those blocks may not exist yet...
   // So we wait, and do edges after all blocks have
   // been created
}

block_instance::~block_instance() {}

Address block_instance::start() const {
   return block_->start() + obj_->codeBase();
}

Address block_instance::end() const {
   return block_->end() + obj_->codeBase();
}

Address block_instance::last() const {
   return block_->lastInsnAddr() + obj_->codeBase();
}

unsigned block_instance::size() const {
   return block_->size();
}

AddressSpace *block_instance::addrSpace() const { 
    return obj()->proc(); 
}

std::string block_instance::format() const {
    stringstream ret;
    ret << "BB[" 
        << hex << start()
        << "," 
        << end() << dec
        << "]" << endl;
    return ret.str();
}

void block_instance::getInsns(Insns &instances) const {
   instances.clear();
   llb()->getInsns(instances, obj()->codeBase());
}

InstructionAPI::Instruction::Ptr block_instance::getInsn(Address a) const {
   Insns insns;
   getInsns(insns);
   return insns[a];
}

std::string block_instance::disassemble() const {
    stringstream ret;
    Insns instances;
    getInsns(instances);
    for (Insns::iterator iter = instances.begin(); 
         iter != instances.end(); ++iter) {
       ret << "\t" << hex << iter->first << ": " << iter->second->format() << dec << endl;
    }
    return ret.str();
}

void *block_instance::getPtrToInstruction(Address addr) const {
    if (addr < start()) return NULL;
    if (addr > end()) return NULL;
    return obj()->getPtrToInstruction(addr);
}


edge_instance *block_instance::getFallthrough() {
   for (edgelist::const_iterator iter = targets().begin(); iter != targets().end(); ++iter) {
      if ((*iter)->type() == FALLTHROUGH ||
          (*iter)->type() == CALL_FT ||
          (*iter)->type() == COND_NOT_TAKEN) { 
         return *iter;
      }
   }
   return NULL;
}

block_instance *block_instance::getFallthroughBlock() {
   edge_instance *ft = getFallthrough();
   if (ft &&
       !ft->sinkEdge()) 
      return ft->trg();
   else
      return NULL;
}

edge_instance *block_instance::getTarget() {
   for (edgelist::const_iterator iter = targets().begin(); iter != targets().end(); ++iter) {
      if ((*iter)->type() == CALL ||
          (*iter)->type() == DIRECT ||
          (*iter)->type() == COND_TAKEN) { 
         return *iter;
      }
   }
   return NULL;
}

block_instance *block_instance::getTargetBlock() {
   edge_instance *t = getFallthrough();
   if (t &&
       !t->sinkEdge()) 
      return t->trg();
   else
      return NULL;
}


bool block_instance::containsCall()
{
    Block::edgelist & out_edges = llb()->targets();
    Block::edgelist::iterator eit = out_edges.begin();
    for( ; eit != out_edges.end(); ++eit) {
        if ( CALL == (*eit)->type() ) {
            return true;
        }
    }
    return false;
}

bool block_instance::containsDynamicCall() {
   Block::edgelist & out_edges = llb()->targets();
   Block::edgelist::iterator eit = out_edges.begin();
   for( ; eit != out_edges.end(); ++eit) {
      if ( CALL == (*eit)->type() && ((*eit)->sinkEdge())) {
         return true;
      }
   }
   return false;
}

int block_instance::id() const {
    return llb()->id();
}

using namespace Dyninst::Relocation;
void block_instance::triggerModified() {
    // Relocation info caching...
   //PCSensitiveTransformer::invalidateCache(this);
}

const block_instance::edgelist &block_instance::sources() {
   if (srcs_.empty()) {
      // Create edges
      for (ParseAPI::Block::edgelist::iterator iter = block_->sources().begin();
           iter != block_->sources().end(); ++iter) {
         // edge_instance takes care of looking up whether we've already
         // created this thing. 
         edge_instance *newEdge = obj()->findEdge(*iter, NULL, this);
         srcs_.push_back(newEdge);
      }
   }
   return srcs_;
}

const block_instance::edgelist &block_instance::targets() {
   if (trgs_.empty()) {
      for (ParseAPI::Block::edgelist::iterator iter = block_->targets().begin();
           iter != block_->targets().end(); ++iter) {
         edge_instance *newEdge = obj()->findEdge(*iter, this, NULL);
         trgs_.push_back(newEdge);
     }
   }
   return trgs_;
}

std::string block_instance::calleeName() {
   // How the heck do we do this again?
   return obj()->getCalleeName(this);
}

void block_instance::updateCallTarget(func_instance *func) {
   // Update a sink-typed call edge to
   // have an inter-module target
   edge_instance *e = getTarget();
   assert(e->sinkEdge());
   e->trg_ = func->entryBlock();
}


func_instance *block_instance::entryOfFunc() const {
   parse_block *b = static_cast<parse_block *>(llb());
   parse_func *e = b->getEntryFunc();
   if (!e) return NULL;
   return obj()->findFunction(e);
}

bool block_instance::isFuncExit() const {
   parse_block *b = static_cast<parse_block *>(llb());
   return b->isExitBlock();
}

func_instance *block_instance::findFunction(ParseAPI::Function *p) {
   return obj()->findFunction(p);
}

void block_instance::destroy(block_instance *b) {
   // Put things here that should go away when we destroy a block. 
   // Iterate through functions...

   std::vector<ParseAPI::Function *> pFuncs;
   b->llb()->getFuncs(pFuncs);
   for (unsigned i = 0; i < pFuncs.size(); ++i) {
      func_instance *func = b->findFunction(pFuncs[i]);
      func->destroyBlock(b);
   }
   
   delete b;
}

   
