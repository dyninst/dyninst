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
/* Public Interface */

#include "AddrSpace.h"
#include "PatchObject.h"
#include "PatchMgr.h"

using namespace std;
using Dyninst::PatchAPI::AddrSpace;
using Dyninst::PatchAPI::PatchObject;
using Dyninst::PatchAPI::PatchMgr;


/* Use an PatchObject (a.out) to initialize the AddrSpace */

bool
AddrSpace::init(PatchObject* obj) {
  first_object_ = obj;
  loadObject(obj);
  return true;
}

AddrSpace*
AddrSpace::create(PatchObject* obj) {
  AddrSpace* ret = new AddrSpace();
  if (!ret) return NULL;
  ret->init(obj);
  return ret;
}

bool
AddrSpace::loadObject(PatchObject* obj) {
  obj_map_[obj->co()] = obj;
  obj->setAddrSpace(this);
  return true;
}

AddrSpace::~AddrSpace() {
  std::cerr << obj_map_.size() << " objects\n";
  for (ObjMap::iterator i = obj_map_.begin(); i != obj_map_.end(); i++) {
    PatchObject* obj = i->second;
    delete obj;
  }
}

bool
AddrSpace::write(PatchObject* /*obj*/, Dyninst::Address /*to*/,
                 Dyninst::Address /*from*/, size_t /*size*/) {
  return false;
}

Dyninst::Address
AddrSpace::malloc(PatchObject* /*obj*/, size_t /*size*/,
                  Dyninst::Address /*near*/) {
  return false;
}

bool
AddrSpace::realloc(PatchObject* /*obj*/, Dyninst::Address /*orig*/,
                   size_t /*size*/) {
  return false;
}

bool
AddrSpace::free(PatchObject* /*obj*/, Dyninst::Address /*orig*/) {
  return false;
}

std::string AddrSpace::format() const { 
   stringstream ret;
   ret << hex << this << dec << endl;
   return ret.str();
}

PatchObject *AddrSpace::findObject(const ParseAPI::CodeObject *co) const
{
    ObjMap::const_iterator oit = obj_map_.find(co);
    if (oit != obj_map_.end()) {
        return oit->second;
    }
    return NULL;
}

bool AddrSpace::consistency(const PatchMgr *m) const
{
   if (mgr_.get() != m) return false;
   for (ObjMap::const_iterator iter = obj_map_.begin();
        iter != obj_map_.end(); ++iter) {
      if (!iter->second->consistency(this)) {
         cerr << "Error: " << iter->second->format() << " failed consistency!" << endl;
         return false;
      }
   }
   return true;
}

