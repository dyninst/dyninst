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

#include "PatchModifier.h"
#include "PatchCFG.h"
#include "PatchMgr.h"
#include "CFGModifier.h"

using namespace Dyninst;
using namespace PatchAPI;

bool PatchModifier::redirect(PatchEdge *edge, PatchBlock *target) {
   // Do we want edges to be in the same object? I don't think so.
   // However, same address space is probably a good idea ;)
   if (edge->source()->obj()->addrSpace() != target->obj()->addrSpace()) return false;

   // Current limitation: cannot retarget indirect edges (well, we can,
   // but don't expect it to work)
   // Also, changing catch edges would be awesome! ... but we can't. 
   if (edge->type() == ParseAPI::INDIRECT || 
       edge->type() == ParseAPI::CATCH ||
       edge->type() == ParseAPI::RET) return false;
   
   // I think this is all we do...
   if (!ParseAPI::CFGModifier::redirect(edge->edge(), target->block())) return false;
   
   assert(edge->source()->consistency());
   assert(edge->consistency());
   assert(target->consistency());

   return true;
}
   


PatchBlock *PatchModifier::split(PatchBlock *block, Address addr, bool trust, Address newlast) {
   if (!trust) {
      if (!block->getInsn(addr)) return NULL;
      newlast = -1; // If we don't trust the address, don't
      // trust the new last instruction addr.
   }

   if (newlast == (Address) -1) {
      // Determine via disassembly
      PatchBlock::Insns insns;
      block->getInsns(insns);
      PatchBlock::Insns::iterator iter = insns.find(addr);
      if (iter == insns.begin()) return NULL;
      --iter;
      newlast = iter->first;
   }      

   Address offset = addr - block->obj()->codeBase();
   
   ParseAPI::Block *split_int = ParseAPI::CFGModifier::split(block->block(), offset, true, newlast);
   if (!split_int) return NULL;

   // We want to return the new block so that folks have a handle; 
   // look it up. 
   PatchBlock *split = block->obj()->getBlock(split_int);

   // DEBUG BUILD
   assert(block->consistency());
   assert(split->consistency());

   return split;
}

PatchBlock *PatchModifier::insert(PatchObject *obj, void *start, unsigned size) {
   ParseAPI::CodeObject *co = obj->co();
   Address base = co->getFreeAddr();
   
   ParseAPI::Block *newBlock = ParseAPI::CFGModifier::insert(co, base, start, size);
   if (!newBlock) return NULL;

   return obj->getBlock(newBlock);
}

bool PatchModifier::remove(PatchBlock *block, bool force)
{
    vector<PatchFunction*> funcs;
    block->getFunctions(std::back_inserter(funcs));
    bool success = ParseAPI::CFGModifier::remove(block->block(), force);

    // DEBUG
    if (success) {
        for (unsigned int fidx = 0; fidx < funcs.size(); fidx++) {
            assert(funcs[fidx]->consistency());
        }
    }
    assert(0 && "KEVINTODO: is there more to do here?");
    return success;
}
