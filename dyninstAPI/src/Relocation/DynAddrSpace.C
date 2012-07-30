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
/* Plugin Implementation */

#include "DynAddrSpace.h"
#include "DynObject.h"

using Dyninst::ParseAPI::CodeObject;
using Dyninst::PatchAPI::DynAddrSpace;
using Dyninst::PatchAPI::DynObject;
using Dyninst::PatchAPI::PatchObject;

bool DynAddrSpace::loadLibrary(DynObject* obj) {
  if (obj) {
    loadObject(obj);
    as_set_.insert(obj->as());
    return false;
  }
  return true;
}

bool DynAddrSpace::removeAddrSpace(AddressSpace *as) {
   as_set_.erase(as);
   return true;
}

DynAddrSpace::DynAddrSpace()
  : AddrSpace(), recursive_(false) {
}

DynAddrSpace* DynAddrSpace::create() {
  DynAddrSpace* ret = new DynAddrSpace();
  if (!ret) return NULL;
  return ret;
}

bool DynAddrSpace::write(PatchObject* obj, Address to, Address from, size_t size) {
  DynObject* dobj = dynamic_cast<DynObject*>(obj);
  return dobj->as()->writeDataSpace(reinterpret_cast<void*>(to), size,
                                     reinterpret_cast<void*>(from));
}

Address DynAddrSpace::malloc(PatchObject* obj, size_t size, Address /*near*/) {
  DynObject* dobj = dynamic_cast<DynObject*>(obj);
  return dobj->as()->inferiorMalloc(size);
}

bool DynAddrSpace::realloc(PatchObject* obj, Address orig, size_t size) {
  DynObject* dobj = dynamic_cast<DynObject*>(obj);
  return dobj->as()->inferiorRealloc(orig, size);
}

bool DynAddrSpace::free(PatchObject* obj, Address orig) {
  DynObject* dobj = dynamic_cast<DynObject*>(obj);
  dobj->as()->inferiorFree(orig);
  return true;
}
