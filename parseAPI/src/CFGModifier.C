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

#include "CFGModifier.h"
#include "CFG.h"
#include "InstructionSource.h"
#include "CodeObject.h"
#include "ParseCallback.h"
#include "ParseData.h"
#include "Parser.h"

using namespace Dyninst;
using namespace ParseAPI;

bool CFGModifier::redirect(Edge *edge, Block *target) {
   // What happens if we try a redirect to the sink? I'm
   // thinking fail, as it's a virtual block.
   if (!target) return false;
   if (!edge) return false;
   if (target->start() == numeric_limits<Address>::max()) return false;
   if (edge->trg() == target) return true;

   // Have to stay within the same CodeObject.
   if (edge->trg()->obj() != target->obj()) return false;

   // Okay, that's done. Let's rock.
   // Pull edge from the old target's source list;
   // Add it to the new target's source list;
   // Queue up callbacks.

   if (!edge->sinkEdge()) {
      Block *oldTarget = edge->trg();
      oldTarget->removeSource(edge);
      oldTarget->obj()->_pcb->removeEdge(oldTarget, edge, ParseCallback::source);
   }
   
   target->addSource(edge);
   target->obj()->_pcb->addEdge(target, edge, ParseCallback::source);
   return true;
}

bool CFGModifier::split(Block *b, Address a, bool trust) {
   if (!trust) {
      // Need to check whether this is a valid instruction offset
      // in the block. 
      assert(0 && "Implement me!");
      return false;
   }
   if (!b) return false;
   if (a < b->start()) return false;
   if (a > b->end()) return false;

   // This function is substantially similar to Parser.C's split_block;
   // however, there are minor differences and the more important point
   // that I don't want to mess with it. 
   
   // 1) Remove the old block from any lookup data structures
   // 2) Create a new "target" block, move all targets to the new
   //    block, and connect the two with a fallthrough edge 
   // 3) Add both blocks to the lookup data structures
   // 4) Add the new block to all functions that contain the original.

   std::vector<Function *> funcs;
   b->getFuncs(funcs);

   // 1)
   region_data *rd = b->obj()->parser->_parse_data->findRegion(b->region());
   assert(rd);
   rd->blocksByRange.remove(b);

   // 2a)
   Block *ret = b->obj()->_fact->_mkblock(funcs[0], b->region(), a);
   ret->_end = b->_end;
   ret->_lastInsn = b->_lastInsn;
   ret->_parsed = true;

   // 2b)
   for (vector<Edge *>::iterator iter = b->_targets.begin(); 
        iter != b->_targets.end(); ++iter) {
      b->obj()->_pcb->removeEdge(b, *iter, ParseCallback::target);
      (*iter)->_source = ret;
      ret->_targets.push_back(*iter);
      b->obj()->_pcb->addEdge(ret, *iter, ParseCallback::target);
   }

   // 2c)
   Edge *ft = b->obj()->_fact->_mkedge(b, ret, FALLTHROUGH);
   ft->_type._sink = false;
   b->_targets.push_back(ft);
   b->obj()->_pcb->addEdge(b, ft, ParseCallback::target);
   ret->_sources.push_back(ft);
   b->obj()->_pcb->addEdge(ret, ft, ParseCallback::source);

   // 3)
   rd->blocksByRange.insert(b);
   rd->blocksByRange.insert(ret);

   // 4)
   for (std::vector<Function *>::iterator iter = funcs.begin();
        iter != funcs.end(); ++iter) {
      (*iter)->_cache_valid = false;
      (*iter)->finalize();
      b->obj()->_pcb->addBlock(*iter, ret);
   }

   b->obj()->_pcb->splitBlock(b, ret);

   return true;
}

bool CFGModifier::remove(Block *b, bool force) {
   if (!b) return false;
   if (!b->_sources.empty()) {
      if (!force) return false;
      for (std::vector<Edge *>::iterator iter = b->_sources.begin();
           iter != b->_sources.end(); ++iter) {
         b->obj()->_pcb->removeEdge(b, *iter, ParseCallback::source);
         b->obj()->_pcb->removeEdge((*iter)->src(), *iter, ParseCallback::source);
         (*iter)->src()->removeTarget(*iter);
      }
   }

   // 1) Remove all target edges. 
   // Remove from:
   // 2) Containing functions;
   // 3) Parser data structures;

   // 1)
   for (std::vector<Edge *>::iterator iter = b->_targets.begin();
        iter != b->_targets.end(); ++iter) {
      b->obj()->_pcb->removeEdge(b, *iter, ParseCallback::target);
      if (!(*iter)->sinkEdge()) {
         (*iter)->trg()->removeSource(*iter);
         b->obj()->_pcb->removeEdge((*iter)->trg(), (*iter), ParseCallback::source);
      }
   }

   // 2)
   std::vector<Function *> funcs;
   b->getFuncs(funcs);
   for (std::vector<Function *>::iterator iter = funcs.begin(); 
        iter != funcs.end(); ++iter) {
      b->obj()->_pcb->removeBlock(*iter, b);
      (*iter)->_cache_valid = false;
      (*iter)->finalize();
   }
   
   // 3)
   region_data *rd = b->obj()->parser->_parse_data->findRegion(b->region());
   assert(rd);
   rd->blocksByRange.remove(b);

   // And destroy it.
   b->obj()->_pcb->destroy(b, b->obj()->fact());

   return true;
}

bool CFGModifier::remove(Function *f) {
   // Remove this function and all of its blocks; 
   // actually refcount the blocks one lower. 
   
   // Must have no in-edges. Technically, we can never remove a recursive function :)
   if (!f->entry()->_sources.empty()) return false;

   for (Function::blocklist::iterator iter = f->blocks().begin(); 
        iter != f->blocks().end(); ++iter) {
      (*iter)->removeFunc(f);
   };

   f->_start = 0;
   f->_name = "ERASED";
   f->_entry = NULL;

   f->_parsed = false;
   f->_cache_valid = false;
   f->_blocks.clear();

   vector<FuncExtent *>::iterator eit = f->_extents.begin();
   for( ; eit != f->_extents.end(); ++eit) {
      delete *eit;
   }

   f->_extents.clear();
   f->_bmap.clear();
   f->_call_edges.clear();
   f->_return_blocks.clear();

   f->obj()->_pcb->destroy(f, f->obj()->fact());

   return true;
}

bool CFGModifier::insert(CodeObject *obj, 
                         Address base, void *data, 
                         unsigned size, Architecture arch) {
   // As per Nate's suggestion, we're going to add this data as a new
   // Region in the CodeObject. 

   InsertedRegion *newRegion = new InsertedRegion(base, data, size, arch);

   obj->cs()->addRegion(newRegion);
   obj->parse(newRegion, base, true);
   return true;
}

InsertedRegion::InsertedRegion(Address b, void *d, unsigned s, Architecture arch) : 
   base_(b), buf_(d), size_(s), arch_(arch) {};

