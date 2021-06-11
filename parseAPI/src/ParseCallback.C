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

#include "ParseCallback.h"

using namespace Dyninst;
using namespace ParseAPI;

template class std::vector<std::pair<Dyninst::ParseAPI::Block*, Dyninst::ParseAPI::Block*> >;

ParseCallbackManager::~ParseCallbackManager() {
   for (iterator iter = begin(); iter != end(); ++iter) {
      delete *iter;
   }
}

void ParseCallbackManager::batch_begin() {
   assert(!inBatch_); // No recursive batches
   inBatch_ = true;
}

void ParseCallbackManager::batch_end(CFGFactory *fact) {
   assert(inBatch_);
   // And now we do work. Hard work. 
   // Collect up all the info we've copied and send it up to the user

   // Inform about changes
   for (std::vector<BlockMod>::iterator iter = blockMods_.begin();
        iter != blockMods_.end(); ++iter) {
      if (iter->action == removed)
         remove_edge_cb(iter->block, iter->edge, iter->type);
      else
         add_edge_cb(iter->block, iter->edge, iter->type);
   }
   blockMods_.clear();

   for (std::vector<EdgeMod>::iterator iter = edgeMods_.begin();
        iter != edgeMods_.end(); ++iter) {
      modify_edge_cb(iter->edge, iter->block, iter->action);
   }
   edgeMods_.clear();

   for (std::vector<FuncMod>::iterator iter = funcMods_.begin();
        iter != funcMods_.end(); ++iter) {
      if (iter->action == removed)
         remove_block_cb(iter->func, iter->block);
      else
         add_block_cb(iter->func, iter->block);
   }
   funcMods_.clear();

   for (std::vector<BlockSplit>::iterator iter = blockSplits_.begin();
        iter != blockSplits_.end(); ++iter) {
      split_block_cb(iter->first, iter->second);
   }
   blockSplits_.clear();

   // destroy (KEVINTODO: test this, shouldn't delete, but I think it does)
   for (std::vector<Edge *>::iterator iter = destroyedEdges_.begin();
        iter != destroyedEdges_.end(); ++iter) {
      destroy_cb(*iter);
   }
   for (std::vector<Block *>::iterator iter = destroyedBlocks_.begin();
        iter != destroyedBlocks_.end(); ++iter) {
      destroy_cb(*iter);
   }
   for (std::vector<Function *>::iterator iter = destroyedFunctions_.begin();
        iter != destroyedFunctions_.end(); ++iter) {
      destroy_cb(*iter);
   }

   // now that we're done with callbacks, delete dangling objects
   for (std::vector<Edge *>::iterator iter = destroyedEdges_.begin();
        iter != destroyedEdges_.end(); ++iter) {
      fact->destroy_edge(*iter, destroyed_cb);
   }
   destroyedEdges_.clear();
   for (std::vector<Block *>::iterator iter = destroyedBlocks_.begin();
        iter != destroyedBlocks_.end(); ++iter) {
      fact->destroy_block(*iter);
   }
   destroyedBlocks_.clear();
   for (std::vector<Function *>::iterator iter = destroyedFunctions_.begin();
        iter != destroyedFunctions_.end(); ++iter) {
      fact->destroy_func(*iter);
   }
   destroyedFunctions_.clear();
   inBatch_ = false;
}

void ParseCallbackManager::destroy(Block *b, CFGFactory *fact) {
   if (inBatch_) destroyedBlocks_.push_back(b);
   else {
      destroy_cb(b);
      fact->destroy_block(b);
   }
}

void ParseCallbackManager::destroy(Edge *e, CFGFactory *fact) {
   if (inBatch_) destroyedEdges_.push_back(e);
   else {
      destroy_cb(e);
      fact->destroy_edge(e, destroyed_cb);
   }
}

void ParseCallbackManager::destroy(Function *f, CFGFactory *fact) {
   if (inBatch_) destroyedFunctions_.push_back(f);
   else {
      destroy_cb(f);
      fact->destroy_func(f);
   }
}

void ParseCallbackManager::removeEdge(Block *b, Edge *e, ParseCallback::edge_type_t t) {
   if (inBatch_) blockMods_.push_back(BlockMod(b, e, t, removed));
   else remove_edge_cb(b, e, t);
}

void ParseCallbackManager::addEdge(Block *b, Edge *e, ParseCallback::edge_type_t t) {
   if (inBatch_) blockMods_.push_back(BlockMod(b, e, t, added));
   else add_edge_cb(b, e, t);
}

void ParseCallbackManager::modifyEdge(Edge *e, Block *b, ParseCallback::edge_type_t t) {
   if (inBatch_) edgeMods_.push_back(EdgeMod(e, b, t));
   else modify_edge_cb(e, b, t);
}

void ParseCallbackManager::removeBlock(Function *f, Block *b) {
   if (inBatch_) funcMods_.push_back(FuncMod(f, b, removed));
   else remove_block_cb(f, b);
}

void ParseCallbackManager::addBlock(Function *f, Block *b) {
   
   if (inBatch_) funcMods_.push_back(FuncMod(f, b, added));
   else add_block_cb(f, b);
}

void ParseCallbackManager::splitBlock(Block *o, Block *n) {
   if (inBatch_) blockSplits_.push_back(BlockSplit(o, n));
   else split_block_cb(o, n);
}

void ParseCallbackManager::interproc_cf(Function *f, Block *b, Address a, ParseCallback::interproc_details *d) {
   for (iterator iter = begin(); iter != end(); ++iter)
      (*iter)->interproc_cf(f, b, a, d);
}

void ParseCallbackManager::instruction_cb(Function *f, Block *b, Address a, ParseCallback::insn_details *d) {
   for (iterator iter = begin(); iter != end(); ++iter)
      (*iter)->instruction_cb(f, b, a, d);
}

void ParseCallbackManager::overlapping_blocks(Block *a, Block *b) {
   for (iterator iter = begin(); iter != end(); ++iter)
      (*iter)->overlapping_blocks(a, b);
}

void ParseCallbackManager::newfunction_retstatus(Function *f) {
   boost::lock_guard <Function> g(*f);
   for (iterator iter = begin(); iter != end(); ++iter)
      (*iter)->newfunction_retstatus(f);
}

void ParseCallbackManager::patch_nop_jump(Address a) {
   for (iterator iter = begin(); iter != end(); ++iter)
      (*iter)->patch_nop_jump(a);
}

bool ParseCallbackManager::updateCodeBytes(Address a) {
   bool ret = true;
   for (iterator iter = begin(); iter != end(); ++iter)
      if (!(*iter)->updateCodeBytes(a)) ret = false;
   return ret;
}

void ParseCallbackManager::abruptEnd_cf(Address a, Block *b, ParseCallback::default_details *d) {
   for (iterator iter = begin(); iter != end(); ++iter)
      (*iter)->abruptEnd_cf(a, b, d);
}

bool ParseCallbackManager::absAddr(Address abs, Address &load, CodeObject *&obj) {
   bool ret = true;
   for (iterator iter = begin(); iter != end(); ++iter)
      if (!(*iter)->absAddr(abs, load, obj)) ret = false;
   return ret;
}

bool ParseCallbackManager::hasWeirdInsns(const Function *f) {
   bool ret = true;
   for (iterator iter = begin(); iter != end(); ++iter)
      if (!(*iter)->hasWeirdInsns(f)) ret = false;
   return ret;
}

void ParseCallbackManager::foundWeirdInsns(Function *f) {
   for (iterator iter = begin(); iter != end(); ++iter)
      (*iter)->foundWeirdInsns(f);
}

void ParseCallbackManager::split_block_cb(Block *a, Block *b) {
   for (iterator iter = begin(); iter != end(); ++iter)
      (*iter)->split_block_cb(a, b);
}

void ParseCallbackManager::discover_function(Function* f) {
   for (iterator iter = begin(); iter != end(); ++iter)
      (*iter)->function_discovery_cb(f);
}

void ParseCallbackManager::destroy_cb(Block *b) {
   for (iterator iter = begin(); iter != end(); ++iter)
      (*iter)->destroy_cb(b);
}

void ParseCallbackManager::destroy_cb(Edge *e) {
   for (iterator iter = begin(); iter != end(); ++iter)
      (*iter)->destroy_cb(e);
}

void ParseCallbackManager::destroy_cb(Function *f) {
   for (iterator iter = begin(); iter != end(); ++iter)
      (*iter)->destroy_cb(f);
}

void ParseCallbackManager::remove_edge_cb(Block *b, Edge *e, ParseCallback::edge_type_t t) {
   for (iterator iter = begin(); iter != end(); ++iter)
      (*iter)->remove_edge_cb(b, e, t);
}

void ParseCallbackManager::add_edge_cb(Block *b, Edge *e, ParseCallback::edge_type_t t) {
   for (iterator iter = begin(); iter != end(); ++iter)
      (*iter)->add_edge_cb(b, e, t);
}

void ParseCallbackManager::modify_edge_cb(Edge *e, Block *b, ParseCallback::edge_type_t t) {
   for (iterator iter = begin(); iter != end(); ++iter)
      (*iter)->modify_edge_cb(e, b, t);
}

void ParseCallbackManager::remove_block_cb(Function *f, Block *b) {
   for (iterator iter = begin(); iter != end(); ++iter)
      (*iter)->remove_block_cb(f, b);
}


void ParseCallbackManager::add_block_cb(Function *f, Block *b) {
   for (iterator iter = begin(); iter != end(); ++iter)
      (*iter)->add_block_cb(f, b);
}

void ParseCallbackManager::registerCallback(ParseCallback *cb) {
   cbs_.push_back(cb);
}

void ParseCallbackManager::unregisterCallback(ParseCallback *cb) {
   for (iterator iter = cbs_.begin(); iter != cbs_.end(); ++iter) {
      if (*iter == cb) {
         cbs_.erase(iter);
         return;
      }
   }
}

   

