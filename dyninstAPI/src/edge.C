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
#include "block.h"
#include "function.h"
#include "mapped_object.h"

using namespace Dyninst;
using namespace Dyninst::ParseAPI;

edge_instance::edge_instance(ParseAPI::Edge *e, block_instance *s, block_instance *t)
  : PatchEdge(e, s, t) {
}

// For forked process
edge_instance::edge_instance(const edge_instance *parent,
                             mapped_object *child)
  : PatchEdge(parent,
              parent->src_?child->findBlock(parent->src()->llb()):NULL,
              parent->trg_?child->findBlock(parent->trg()->llb()):NULL) {
}

edge_instance::~edge_instance() {
}


AddressSpace *edge_instance::proc() {
   return src()->proc();
}

block_instance *edge_instance::src() const {
  return SCAST_BI(src_);
}

block_instance *edge_instance::trg() const {
  return SCAST_BI(trg_);
}
