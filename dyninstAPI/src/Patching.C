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

#include "Patching.h"
#include "block.h"
#include "function.h"
#include "mapped_object.h"

using namespace Dyninst;
using namespace PatchAPI;

void DynPatchCallback::split_block_cb(PatchBlock *first, PatchBlock *second)
{
    // no splitting needed on block_instance itself
    // 1) split mapped_object data structures 
    // 2) split function data structures

    // 1)
    mapped_object *obj = SCAST_MO(first->object());
    obj->splitBlock(SCAST_BI(first),SCAST_BI(second));

    // 2)
    block_instance *b1 = SCAST_BI(first);
    std::vector<func_instance*> funcs;
    b1->getFuncs(std::back_inserter(funcs));
    for (vector<func_instance*>::iterator fit = funcs.begin();
         fit != funcs.end();
         fit++)
    {
        (*fit)->split_block_cb(b1, SCAST_BI(second));
    }
    //KEVINTODO: clean up BPatch-level items from here instead of top-down
}

void DynPatchCallback::add_block_cb(PatchFunction *func, PatchBlock *block)
{
    SCAST_FI(func)->add_block_cb(SCAST_BI(block));
}

void DynPatchCallback::remove_block_cb(PatchFunction *f, PatchBlock *b)
{
   SCAST_FI(f)->removeBlock(SCAST_BI(b));
}

void DynPatchCallback::destroy_cb(Point *p) 
{
    static_cast<instPoint*>(p)->func()->obj()->remove(static_cast<instPoint*>(p));
}

void DynPatchCallback::destroy_cb(PatchAPI::PatchBlock *b)
{
    SCAST_MO(b->obj())->destroy(SCAST_BI(b));
}
void DynPatchCallback::destroy_cb(PatchEdge *, PatchObject *)
{
    // nothing to do
}
void DynPatchCallback::destroy_cb(PatchAPI::PatchFunction *f)
{
    SCAST_MO(f->obj())->destroy(SCAST_FI(f));
}
void DynPatchCallback::destroy_cb(PatchAPI::PatchObject *)
{
    assert(0 && "implement me");
}
