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
#include "PatchCallback.h"
#include "PatchCFG.h"
#include "Point.h"

using namespace std;
using namespace Dyninst;
using namespace PatchAPI;

void PatchCallback::batch_begin() {
   assert(!batching_);
   batching_ = true;
}

void PatchCallback::batch_end() {
   assert(batching_);

   // And now we do work. Hard work. 
   // Collect up all the info we've copied and send it up to the user
   for (std::vector<Point *>::iterator iter = destroyedPoints_.begin();
        iter != destroyedPoints_.end(); ++iter) {
      destroy_cb(*iter);
   }
   for (std::vector<std::pair<PatchEdge *,PatchObject*> >::iterator iter = destroyedEdges_.begin();
        iter != destroyedEdges_.end(); ++iter) {
      destroy_cb(iter->first, iter->second);
   }
   for (std::vector<PatchBlock *>::iterator iter = destroyedBlocks_.begin();
        iter != destroyedBlocks_.end(); ++iter) {
      destroy_cb(*iter);
   }
   for (std::vector<PatchFunction *>::iterator iter = destroyedFuncs_.begin();
        iter != destroyedFuncs_.end(); ++iter) {
      destroy_cb(*iter);
   }
   for (std::vector<PatchObject *>::iterator iter = destroyedObjects_.begin();
        iter != destroyedObjects_.end(); ++iter) {
      destroy_cb(*iter);
   }

   for (std::vector<PatchObject *>::iterator iter = createdObjects_.begin();
        iter != createdObjects_.end(); ++iter) {
      create_cb(*iter);
   }
   createdObjects_.clear();
   for (std::vector<PatchFunction *>::iterator iter = createdFuncs_.begin();
        iter != createdFuncs_.end(); ++iter) {
      create_cb(*iter);
   }
   createdFuncs_.clear();
   for (std::vector<PatchBlock *>::iterator iter = createdBlocks_.begin();
        iter != createdBlocks_.end(); ++iter) {
      create_cb(*iter);
   }
   createdBlocks_.clear();
   for (std::vector<PatchEdge *>::iterator iter = createdEdges_.begin();
        iter != createdEdges_.end(); ++iter) {
      create_cb(*iter);
   }
   createdEdges_.clear();
   for (std::vector<Point *>::iterator iter = createdPoints_.begin();
        iter != createdPoints_.end(); ++iter) {
      create_cb(*iter);
   }
   createdPoints_.clear();

   // Inform about changes
   for (std::vector<BlockMod>::iterator iter = blockMods_.begin();
        iter != blockMods_.end(); ++iter) {
      if (iter->mod == removed)
         remove_edge_cb(iter->block, iter->edge, iter->type);
      else
         add_edge_cb(iter->block, iter->edge, iter->type);
   }
   blockMods_.clear();

   for (std::vector<FuncMod>::iterator iter = funcMods_.begin();
        iter != funcMods_.end(); ++iter) {
      if (iter->mod == removed)
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

   for (std::vector<PointMod>::iterator iter = pointMods_.begin();
        iter != pointMods_.end(); ++iter) {
      change_cb(iter->point, iter->old_block, iter->new_block);
   }

   // now that we're done with callbacks, delete dangling objects
   for (std::vector<Point *>::iterator iter = destroyedPoints_.begin();
        iter != destroyedPoints_.end(); ++iter) {
      delete *iter;
   }
   destroyedPoints_.clear();
   for (std::vector<std::pair<PatchEdge *,PatchObject*> >::iterator iter = destroyedEdges_.begin();
        iter != destroyedEdges_.end(); ++iter) {
      delete iter->first;
   }
   destroyedEdges_.clear();
   for (std::vector<PatchBlock *>::iterator iter = destroyedBlocks_.begin();
        iter != destroyedBlocks_.end(); ++iter) {
      delete *iter;
   }
   destroyedBlocks_.clear();
   for (std::vector<PatchFunction *>::iterator iter = destroyedFuncs_.begin();
        iter != destroyedFuncs_.end(); ++iter) {
      delete *iter;
   }
   destroyedFuncs_.clear();
   for (std::vector<PatchObject *>::iterator iter = destroyedObjects_.begin();
        iter != destroyedObjects_.end(); ++iter) {
      delete *iter;
   }
   destroyedObjects_.clear();

   batching_ = false;
}

void PatchCallback::destroy(PatchBlock *b) {
   if (batching_) destroyedBlocks_.push_back(b);
   else {
      destroy_cb(b);
      delete b;
   }
}

void PatchCallback::destroy(PatchEdge *e, PatchObject *o) {
   if (batching_) { 
      pair<PatchEdge*, PatchObject*> deadEdge(e,o);
      destroyedEdges_.push_back(deadEdge);
   }
   else {
      destroy_cb(e, o);
      delete e;
   }
}

void PatchCallback::destroy(PatchFunction *e) {
   if (batching_) destroyedFuncs_.push_back(e);
   else {
      destroy_cb(e);
      delete e;
   }
}

void PatchCallback::destroy(PatchObject *e) {
   if (batching_) destroyedObjects_.push_back(e);
   else {
      destroy_cb(e);
      delete e;
   }
}

void PatchCallback::create(PatchBlock *b) {
   if (batching_) createdBlocks_.push_back(b);
   else create_cb(b);
}

void PatchCallback::create(PatchEdge *b) {
   if (batching_) createdEdges_.push_back(b);
   else create_cb(b);
}

void PatchCallback::create(PatchFunction *b) {
   if (batching_) createdFuncs_.push_back(b);
   else create_cb(b);
}

void PatchCallback::create(PatchObject *b) {
   if (batching_) createdObjects_.push_back(b);
   else create_cb(b);
}

void PatchCallback::split_block(PatchBlock *a, PatchBlock *b) {
   if (batching_) blockSplits_.push_back(std::make_pair(a, b));
   else split_block_cb(a, b);
}

void PatchCallback::remove_edge(PatchBlock *a, PatchEdge *b, edge_type_t c) {
   if (batching_) blockMods_.push_back(BlockMod(a, b, c, removed));
   else remove_edge_cb(a, b, c);
}

void PatchCallback::add_edge(PatchBlock *a, PatchEdge *b, edge_type_t c) {
   if (batching_) blockMods_.push_back(BlockMod(a, b, c, added));
   else add_edge_cb(a, b, c);
}

void PatchCallback::remove_block(PatchFunction *a, PatchBlock *b) {
   if (batching_) funcMods_.push_back(FuncMod(a, b, removed));
   else remove_block_cb(a, b);
}

void PatchCallback::add_block(PatchFunction *a, PatchBlock *b) {
   if (batching_) funcMods_.push_back(FuncMod(a, b, added));
   else add_block_cb(a, b);
}

void PatchCallback::destroy(Point *e) {
   if (batching_) destroyedPoints_.push_back(e);
   else {
      destroy_cb(e);
      delete e;
   }
}

void PatchCallback::create(Point *b) {
   if (batching_) createdPoints_.push_back(b);
   else create_cb(b);
}

void PatchCallback::change(Point *p, PatchBlock *ob, PatchBlock *nb) {
   if (batching_) pointMods_.push_back(PointMod(p, ob, nb));
   else change_cb(p, ob, nb);
}
                      
