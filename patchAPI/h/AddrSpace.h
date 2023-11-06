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

/* Plugin Interface */

#ifndef PATCHAPI_H_ADDRSPACE_H_
#define PATCHAPI_H_ADDRSPACE_H_

#include <map>
#include <stddef.h>
#include <string>
#include "PatchCommon.h"
#include "dyntypes.h"

namespace Dyninst {
namespace PatchAPI {

/* Interface specification for the interation between a PatchMgr and
   the address space */

class PATCHAPI_EXPORT AddrSpace {
    friend class PatchMgr;
    friend class PatchFunction;

  public:
    static AddrSpace* create(PatchObject* obj);
    virtual ~AddrSpace();

    // Write data in mutatee's address space
    virtual bool write(PatchObject* /*obj*/, Dyninst::Address /*to*/,
                                       Dyninst::Address /*from*/, size_t /*size*/);

    // Memory allocation / reallocation / deallocation in mutatee's addressSpace
    virtual Dyninst::Address malloc(PatchObject* /*obj*/, size_t /*size*/,
                                           Dyninst::Address /*near*/);

    virtual bool realloc(PatchObject* /*obj*/, Dyninst::Address /*orig*/,
                                         size_t /*size*/);

    virtual bool free(PatchObject* /*obj*/, Dyninst::Address /*orig*/);

    // Load a binary oject into the address space
    virtual bool loadObject(PatchObject* obj);

    // Getters
    typedef std::map<const ParseAPI::CodeObject*, PatchObject*> ObjMap;
    ObjMap& objMap() { return obj_map_; }
    PatchObject* findObject(const ParseAPI::CodeObject*) const;
    template <class Iter> void objs(Iter iter); // EXPORTED
    PatchObject* executable() { return first_object_; }
    PatchMgrPtr mgr() const { return mgr_; }

    std::string format() const;

    bool consistency(const PatchMgr *mgr) const;

  protected:
    ObjMap obj_map_;
    PatchObject* first_object_{};
    PatchMgrPtr mgr_;

    bool init(PatchObject*);
    AddrSpace(): first_object_(NULL) {}
    explicit AddrSpace(AddrSpace*) {}
};

template <class Iter>
   void AddrSpace::objs(Iter iter) {
   for (ObjMap::iterator tmp = obj_map_.begin(); tmp != obj_map_.end(); ++tmp) {
      *iter = tmp->second;
      ++iter;
   }
}



}
}

#endif /* PATCHAPI_H_ADDRSPACE_H_ */
