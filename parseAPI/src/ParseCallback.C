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

#include "ParseCallback.h"

using namespace Dyninst;
using namespace ParseAPI;

void ParseCallback::batch_begin() {
   assert(!inBatch_); // No recursive batches
   inBatch_ = true;
}

void ParseCallback::batch_end(CFGFactory *fact) {
   assert(inBatch_);

   // And now we do work. Hard work. 
   // Collect up all the info we've copied and send it up to the user
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

   // Inform about changes
   for (std::vector<BlockMod>::iterator iter = blockMods_.begin();
        iter != blockMods_.end(); ++iter) {
      if (iter->action == removed)
         remove_edge_cb(iter->block, iter->edge, iter->type);
      else
         add_edge_cb(iter->block, iter->edge, iter->type);
   }
   blockMods_.clear();

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

   // now that we're done with callbacks, delete dangling objects
   for (std::vector<Edge *>::iterator iter = destroyedEdges_.begin();
        iter != destroyedEdges_.end(); ++iter) {
      fact->destroy_edge(*iter);
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
}

void ParseCallback::destroy(Block *b, CFGFactory *fact) {
   if (inBatch_) destroyedBlocks_.push_back(b);
   else {
      destroy_cb(b);
      fact->destroy_block(b);
   }
}

void ParseCallback::destroy(Edge *e, CFGFactory *fact) {
   if (inBatch_) destroyedEdges_.push_back(e);
   else {
      destroy_cb(e);
      fact->destroy_edge(e);
   }
}

void ParseCallback::destroy(Function *f, CFGFactory *fact) {
   if (inBatch_) destroyedFunctions_.push_back(f);
   else {
      destroy_cb(f);
      fact->destroy_func(f);
   }
}

void ParseCallback::removeEdge(Block *b, Edge *e, edge_type_t t) {
   if (inBatch_) blockMods_.push_back(BlockMod(b, e, t, removed));
   else remove_edge_cb(b, e, t);
}

void ParseCallback::addEdge(Block *b, Edge *e, edge_type_t t) {
   if (inBatch_) blockMods_.push_back(BlockMod(b, e, t, added));
   else add_edge_cb(b, e, t);
}

void ParseCallback::removeBlock(Function *f, Block *b) {
   if (inBatch_) funcMods_.push_back(FuncMod(f, b, removed));
   else remove_block_cb(f, b);
}

void ParseCallback::addBlock(Function *f, Block *b) {
   if (inBatch_) funcMods_.push_back(FuncMod(f, b, added));
   else add_block_cb(f, b);
}

void ParseCallback::splitBlock(Block *o, Block *n) {
   if (inBatch_) blockSplits_.push_back(BlockSplit(o, n));
   else split_block_cb(o, n);
}

   
   
   
