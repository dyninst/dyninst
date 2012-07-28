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
#include "PatchObject.h"
#include "PatchCFG.h"

using namespace Dyninst;
using namespace PatchAPI;

/* Default Implementation for CFGMaker */

PatchFunction*
CFGMaker::makeFunction(ParseAPI::Function* f, PatchObject* obj) {
  PatchFunction* ret = new PatchFunction(f, obj);
  if (!ret) {
    cerr << "ERROR: Cannot create PatchFunction for " << f->name() << "\n";
    assert(0);
  }
  return ret;
}

PatchFunction*
CFGMaker::copyFunction(PatchFunction* f, PatchObject* obj) {
  return (new PatchFunction(f, obj));
}

PatchBlock*
CFGMaker::makeBlock(ParseAPI::Block* b, PatchObject* obj) {
  PatchBlock* ret = new PatchBlock(b, obj);
  if (!ret) {
    cerr << "ERROR: Cannot create PatchBlock for 0x" << std::hex
         << b->start() << "\n" << std::dec;
    assert(0);
  }
  return ret;
}

PatchBlock*
CFGMaker::copyBlock(PatchBlock* b, PatchObject* obj) {
  return (new PatchBlock(b, obj));
}

PatchEdge*
CFGMaker::makeEdge(ParseAPI::Edge* e, PatchBlock* s, PatchBlock* t, PatchObject* o) {
  return (new PatchEdge(e,
                        s ? s : o->getBlock(e->src()),
                        t ? t : o->getBlock(e->trg())));
}

PatchEdge*
CFGMaker::copyEdge(PatchEdge* e, PatchObject* o) {
  return (new PatchEdge(e,
                        o->getBlock(e->src()->block()),
                        o->getBlock(e->trg()->block())));
}
