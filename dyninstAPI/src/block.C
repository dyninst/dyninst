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



int_block::int_block(image_basicBlock *ib, int_function *func) 
  : highlevel_block(NULL),
    func_(func),
    block_(ib),
    srclist_(srcs_),
    trglist_(trgs_)
{};

// Fork constructor
int_block::int_block(const int_block *parent, int_function *func) :
    highlevel_block(NULL),
    func_(func),
    block_(parent->block_),
    srclist_(srcs_),
    trglist_(trgs_)
{
}

int_block::~int_block() {}

Address int_block::start() const {
    return block_->start() + func_->baseAddr();
}

Address int_block::end() const {
    return block_->end() + func_->baseAddr();
}

Address int_block::last() const {
    return block_->lastInsnAddr() + func_->baseAddr();
}

unsigned int_block::size() const {
    return block_->size();
}

int_function *int_block::func() const { 
    return func_;
}

AddressSpace *int_block::addrSpace() const { 
    return func()->proc(); 
}

bool int_block::isEntry() const { 
   return (this == func_->entryBlock());
}

std::string int_block::format() const {
    stringstream ret;
    ret << "BB(" 
        << hex << start()
        << ".." 
        << end() << dec << endl;
    return ret.str();
}

#if defined(cap_instruction_api) 
void int_block::getInsns(InsnInstances &instances) const {
  instances.clear();
  llb()->getInsnInstances(instances);
  for (unsigned i = 0; i < instances.size(); ++i) {
      instances[i].second += func()->baseAddr();
  }
}

std::string int_block::disassemble() const {
    stringstream ret;
    InsnInstances instances;
    getInsns(instances);
    for (unsigned i = 0; i < instances.size(); ++i) {
        ret << "\t" << hex << instances[i].second << ": " << instances[i].first->format() << dec << endl;
    }
    return ret.str();
}
#endif

void *int_block::getPtrToInstruction(Address addr) const {
    if (addr < start()) return NULL;
    if (addr > end()) return NULL;
    return func()->obj()->getPtrToInstruction(addr);
}


int_block *int_block::getFallthrough() {
   for (edgelist::iterator iter = targets().begin(); iter != targets().end(); ++iter) {
      if ((*iter)->type() == FALLTHROUGH ||
          (*iter)->type() == CALL_FT ||
          (*iter)->type() == COND_NOT_TAKEN) { 
         return (*iter)->trg();
      }
   }
   return NULL;
}

int_block *int_block::getTarget() {
   for (edgelist::iterator iter = targets().begin(); iter != targets().end(); ++iter) {
      if ((*iter)->type() == CALL ||
          (*iter)->type() == DIRECT ||
          (*iter)->type() == COND_TAKEN) { 
         return (*iter)->trg();
      }
   }
   return NULL;
}

void int_block::setHighLevelBlock(BPatch_basicBlock *newb)
{
   highlevel_block = newb;
}

BPatch_basicBlock *int_block::getHighLevelBlock() const {
   return highlevel_block;
}

bool int_block::containsCall()
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

bool int_block::containsDynamicCall() {
   Block::edgelist & out_edges = llb()->targets();
   Block::edgelist::iterator eit = out_edges.begin();
   for( ; eit != out_edges.end(); ++eit) {
      if ( CALL == (*eit)->type() && ((*eit)->sinkEdge())) {
         return true;
      }
   }
   return false;
}

int int_block::id() const {
    return llb()->id();
}

using namespace Dyninst::Relocation;
void int_block::triggerModified() {
    // Relocation info caching...
   //PCSensitiveTransformer::invalidateCache(this);
}

const int_block::edgelist &int_block::sources() {
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
            int_edge *newEdge = int_edge::create(*iter, NULL, this);
            srcs_.push_back(newEdge);
         }
      }
   }
   return srclist_;
}

const int_block::edgelist &int_block::targets() {
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
           int_edge *newEdge = int_edge::create(*iter, this, NULL);
           trgs_.push_back(newEdge);
         }
      }
   }
   return trglist_;
}

void int_block::createInterproceduralEdges(ParseAPI::Edge *iedge, bool forwards, std::vector<int_edge 
*> &edges) {
  //   assert(iedge->interproc());
   
   // Let pT be the target block in the parseAPI
   // Let {f_1, ..., f_n} be the functions T is in
   // We create blocks t_i for each function f_i
   ParseAPI::Block *iblk = (forwards) ? iedge->trg() : iedge->src();
   if (!iblk) {
      assert(forwards); // Can't have sink in-edges

      edges.push_back(int_edge::create(iedge, this, NULL));
      return;
   }
   
   std::vector<ParseAPI::Function *> ifuncs;
   iblk->getFuncs(ifuncs);
   
   for (unsigned i = 0; i < ifuncs.size(); ++i) {
      int_function *pfunc = obj()->findFunction(ifuncs[i]);
      assert(pfunc);
      int_block *pblock = pfunc->findBlock(iblk);
      assert(pblock);
      int_edge *newEdge = NULL;
      if (forwards) 
         newEdge = int_edge::create(iedge, this, pblock);
      else 
         newEdge = int_edge::create(iedge, pblock, this);
      
      edges.push_back(newEdge);
   }
   return;
}

mapped_object *int_block::obj() const {
   return func()->obj();
}

int_function *int_block::callee() {
   return func()->findCallee(this);
}

std::string int_block::calleeName() {
   // How the heck do we do this again?
   return obj()->getCalleeName(this);
}

instPoint *int_block::entryPoint() {
   int_function::InstPointMap::iterator iter = func()->blockEntryPoints_.find(this);
   if (iter != func()->blockEntryPoints_.end()) return iter->second;
   
   instPoint *iP = instPoint::blockEntry(this);
   func()->blockEntryPoints_[this] = iP;
   return iP;
}

instPoint *int_block::preCallPoint() {
   int_function::InstPointMap::iterator iter = func()->preCallPoints_.find(this);
   if (iter != func()->preCallPoints_.end()) return iter->second;
   
   instPoint *iP = instPoint::preCall(this);
   func()->preCallPoints_[this] = iP;
   return iP;
}

instPoint *int_block::postCallPoint() {
   int_function::InstPointMap::iterator iter = func()->postCallPoints_.find(this);
   if (iter != func()->postCallPoints_.end()) return iter->second;
   
   instPoint *iP = instPoint::postCall(this);
   func()->postCallPoints_[this] = iP;
   return iP;
}

instPoint *int_block::preInsnPoint(Address addr) {
   assert(addr >= start());
   assert(addr < end());

   int_function::ArbitraryMap::iterator iter = func()->preInsnPoints_.find(this);
   if (iter != func()->preInsnPoints_.end()) {
      int_function::ArbitraryMapInt::iterator iter2 = iter->second.find(addr);
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

instPoint *int_block::preInsnPoint(Address addr, InstructionAPI::Instruction::Ptr insn) {
   int_function::ArbitraryMap::iterator iter = func()->preInsnPoints_.find(this);
   if (iter != func()->preInsnPoints_.end()) {
      int_function::ArbitraryMapInt::iterator iter2 = iter->second.find(addr);
      if (iter2 != iter->second.end()) {
         return iter2->second;
      }
   }
   
   instPoint *iP = instPoint::preInsn(this, insn, addr, true);
   func()->preInsnPoints_[this][addr] = iP;
   return iP;
}

instPoint *int_block::postInsnPoint(Address addr) {
   assert(addr > start());
   assert(addr < end());

   int_function::ArbitraryMap::iterator iter = func()->postInsnPoints_.find(this);
   if (iter != func()->postInsnPoints_.end()) {
      int_function::ArbitraryMapInt::iterator iter2 = iter->second.find(addr);
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

instPoint *int_block::postInsnPoint(Address addr, InstructionAPI::Instruction::Ptr insn) {
   int_function::ArbitraryMap::iterator iter = func()->postInsnPoints_.find(this);
   if (iter != func()->postInsnPoints_.end()) {
      int_function::ArbitraryMapInt::iterator iter2 = iter->second.find(addr);
      if (iter2 != iter->second.end()) {
         return iter2->second;
      }
   }

   instPoint *iP = instPoint::postInsn(this, insn, addr, true);
   func()->postInsnPoints_[this][addr] = iP;
   return iP;
}


instPoint *int_block::findPoint(instPoint::Type type) {
   return func()->findPoint(type, this);
}

const std::map<Address, instPoint *> &int_block::findPoints(instPoint::Type type) {
   return func()->findPoints(type, this);
}

