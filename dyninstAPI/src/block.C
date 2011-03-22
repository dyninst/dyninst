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

#include "process.h"
#include "instPoint.h"

#include "mapped_object.h"
#include "mapped_module.h"
#include "InstructionDecoder.h"
#include <set>
#include <sstream>
#include "Relocation/Transformers/Movement-analysis.h"
using namespace Dyninst;
using namespace Dyninst::ParseAPI;

block_instance::block_instance(parse_block *ib, func_instance *func) 
  : highlevel_block(NULL),
    func_(func),
    block_(ib),
    srclist_(srcs_),
    trglist_(trgs_)
{};

// Fork constructor
block_instance::block_instance(const block_instance *parent, func_instance *func) :
    highlevel_block(NULL),
    func_(func),
    block_(parent->block_),
    srclist_(srcs_),
    trglist_(trgs_)
{
}

block_instance::~block_instance() {}

Address block_instance::start() const {
    return block_->start() + func_->baseAddr();
}

Address block_instance::end() const {
    return block_->end() + func_->baseAddr();
}

Address block_instance::last() const {
    return block_->lastInsnAddr() + func_->baseAddr();
}

unsigned block_instance::size() const {
    return block_->size();
}

func_instance *block_instance::func() const { 
    return func_;
}

AddressSpace *block_instance::addrSpace() const { 
    return func()->proc(); 
}

bool block_instance::isEntry() const { 
   return (this == func_->entryBlock());
}

std::string block_instance::format() const {
    stringstream ret;
    ret << "BB(" 
        << hex << start()
        << ".." 
        << end() << dec << endl;
    return ret.str();
}

#if defined(cap_instruction_api) 
void block_instance::getInsns(InsnInstances &instances) const {
  instances.clear();
  llb()->getInsnInstances(instances);
  for (unsigned i = 0; i < instances.size(); ++i) {
      instances[i].second += func()->baseAddr();
  }
}

std::string block_instance::disassemble() const {
    stringstream ret;
    InsnInstances instances;
    getInsns(instances);
    for (unsigned i = 0; i < instances.size(); ++i) {
        ret << "\t" << hex << instances[i].second << ": " << instances[i].first->format() << dec << endl;
    }
    return ret.str();
}
#endif

void *block_instance::getPtrToInstruction(Address addr) const {
    if (addr < start()) return NULL;
    if (addr > end()) return NULL;
    return func()->obj()->getPtrToInstruction(addr);
}


block_instance *block_instance::getFallthrough() {
   for (edgelist::iterator iter = targets().begin(); iter != targets().end(); ++iter) {
      if ((*iter)->type() == FALLTHROUGH ||
          (*iter)->type() == CALL_FT ||
          (*iter)->type() == COND_NOT_TAKEN) { 
         return (*iter)->trg();
      }
   }
   return NULL;
}

block_instance *block_instance::getTarget() {
   for (edgelist::iterator iter = targets().begin(); iter != targets().end(); ++iter) {
      if ((*iter)->type() == CALL ||
          (*iter)->type() == DIRECT ||
          (*iter)->type() == COND_TAKEN) { 
         return (*iter)->trg();
      }
   }
   return NULL;
}

void block_instance::setHighLevelBlock(BPatch_basicBlock *newb)
{
   highlevel_block = newb;
}

BPatch_basicBlock *block_instance::getHighLevelBlock() const {
   return highlevel_block;
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
         // We need to copy interprocedural edges to ensure that we de-overlap
         // code in functions. We do this here. 
         if ((*iter)->interproc()) {
            createInterproceduralEdges(*iter, false, srcs_);
         }
         else {
            // Can lazily create the source block since it's in
            // our function.
            edge_instance *newEdge = edge_instance::create(*iter, NULL, this);
            srcs_.push_back(newEdge);
         }
      }
   }
   return srclist_;
}

const block_instance::edgelist &block_instance::targets() {
   if (trgs_.empty()) {
      for (ParseAPI::Block::edgelist::iterator iter = block_->targets().begin();
           iter != block_->targets().end(); ++iter) {
         // We need to copy interprocedural edges to ensure that we de-overlap
         // code in functions. We do this here.
        //XXX: this doesn't work!
        //         if ((*iter)->interproc()) {
         if ((*iter)->interproc()) {
            createInterproceduralEdges(*iter, true, trgs_);
         }
         else {
            // Can lazily create the source block since it's in
            // our function.
           edge_instance *newEdge = edge_instance::create(*iter, this, NULL);
           trgs_.push_back(newEdge);
         }
      }
   }
   return trglist_;
}

void block_instance::createInterproceduralEdges(ParseAPI::Edge *iedge, bool forwards, std::vector<edge_instance 
*> &edges) {
  //   assert(iedge->interproc());

   // This function has to handle two things
   // 1) ParseAPI has blocks contained in multiple
   // functions, which the int layer doesn't;
   // 2) Interprocedural control flow. 
   // 
   // We map 1:n in all cases except forward
   // calls, in which case we know which function
   // we're calling. 
   
   // Let pT be the target block in the parseAPI
   // Let {f_1, ..., f_n} be the functions T is in
   // We create blocks t_i for each function f_i
   ParseAPI::Block *iblk = (forwards) ? iedge->trg() : iedge->src();
   if (!iblk) {
      assert(forwards); // Can't have sink in-edges

      edges.push_back(edge_instance::create(iedge, this, NULL));
      return;
   }

   // TODO: figure out how to handle sink blocks better... right
   // now we're just ignoring them, and I'm not sure that's a
   // great idea. 

   std::vector<ParseAPI::Function *> ifuncs;
   if (forwards && iedge->type() == ParseAPI::CALL) {
      parse_block *tmp = static_cast<parse_block *>(iblk);
      parse_func *f = tmp->getEntryFunc();
      if (f) ifuncs.push_back(f);
   }
   else {
      iblk->getFuncs(ifuncs);
   }   

   for (unsigned i = 0; i < ifuncs.size(); ++i) {
      func_instance *pfunc = obj()->findFunction(ifuncs[i]);
      assert(pfunc);
      block_instance *pblock = pfunc->findBlock(iblk);
      assert(pblock);
      edge_instance *newEdge = NULL;
      if (forwards) 
         newEdge = edge_instance::create(iedge, this, pblock);
      else 
         newEdge = edge_instance::create(iedge, pblock, this);
      
      edges.push_back(newEdge);
   }
   return;
}

mapped_object *block_instance::obj() const {
   return func()->obj();
}

func_instance *block_instance::callee() {
   return func()->findCallee(this);
}

std::string block_instance::calleeName() {
   // How the heck do we do this again?
   return obj()->getCalleeName(this);
}

instPoint *block_instance::entryPoint() {
   func_instance::InstPointMap::iterator iter = func()->blockEntryPoints_.find(this);
   if (iter != func()->blockEntryPoints_.end()) return iter->second;
   
   instPoint *iP = instPoint::blockEntry(this);
   func()->blockEntryPoints_[this] = iP;
   return iP;
}

instPoint *block_instance::preCallPoint() {
   func_instance::InstPointMap::iterator iter = func()->preCallPoints_.find(this);
   if (iter != func()->preCallPoints_.end()) return iter->second;
   
   instPoint *iP = instPoint::preCall(this);
   func()->preCallPoints_[this] = iP;
   return iP;
}

instPoint *block_instance::postCallPoint() {
   func_instance::InstPointMap::iterator iter = func()->postCallPoints_.find(this);
   if (iter != func()->postCallPoints_.end()) return iter->second;
   
   instPoint *iP = instPoint::postCall(this);
   func()->postCallPoints_[this] = iP;
   return iP;
}

instPoint *block_instance::preInsnPoint(Address addr) {
   assert(addr >= start());
   assert(addr < end());

   func_instance::ArbitraryMap::iterator iter = func()->preInsnPoints_.find(this);
   if (iter != func()->preInsnPoints_.end()) {
      func_instance::ArbitraryMapInt::iterator iter2 = iter->second.find(addr);
      if (iter2 != iter->second.end()) {
         return iter2->second;
      }
   }
   
   InsnInstances insns;
   getInsns(insns);
   for (InsnInstances::iterator iter = insns.begin(); iter != insns.end(); ++iter) {
      if (iter->second == addr) {
         return preInsnPoint(iter->second, iter->first);
      }
   }
   return NULL;
}

instPoint *block_instance::preInsnPoint(Address addr, InstructionAPI::Instruction::Ptr insn) {
   func_instance::ArbitraryMap::iterator iter = func()->preInsnPoints_.find(this);
   if (iter != func()->preInsnPoints_.end()) {
      func_instance::ArbitraryMapInt::iterator iter2 = iter->second.find(addr);
      if (iter2 != iter->second.end()) {
         return iter2->second;
      }
   }
   
   instPoint *iP = instPoint::preInsn(this, insn, addr, true);
   func()->preInsnPoints_[this][addr] = iP;
   return iP;
}

instPoint *block_instance::postInsnPoint(Address addr) {
   assert(addr > start());
   assert(addr < end());

   func_instance::ArbitraryMap::iterator iter = func()->postInsnPoints_.find(this);
   if (iter != func()->postInsnPoints_.end()) {
      func_instance::ArbitraryMapInt::iterator iter2 = iter->second.find(addr);
      if (iter2 != iter->second.end()) {
         return iter2->second;
      }
   }

   InsnInstances insns;
   getInsns(insns);
   for (InsnInstances::iterator iter = insns.begin(); iter != insns.end(); ++iter) {
      if (iter->second == addr) {
         return postInsnPoint(addr, iter->first);
      }
   }
   return NULL;
}

instPoint *block_instance::postInsnPoint(Address addr, InstructionAPI::Instruction::Ptr insn) {
   func_instance::ArbitraryMap::iterator iter = func()->postInsnPoints_.find(this);
   if (iter != func()->postInsnPoints_.end()) {
      func_instance::ArbitraryMapInt::iterator iter2 = iter->second.find(addr);
      if (iter2 != iter->second.end()) {
         return iter2->second;
      }
   }

   instPoint *iP = instPoint::postInsn(this, insn, addr, true);
   func()->postInsnPoints_[this][addr] = iP;
   return iP;
}


instPoint *block_instance::findPoint(instPoint::Type type) {
   return func()->findPoint(type, this);
}

const std::map<Address, instPoint *> &block_instance::findPoints(instPoint::Type type) {
   return func()->findPoints(type, this);
}

