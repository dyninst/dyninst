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

#ifndef PATCHAPI_H_DYNINST_DYNADDRSPACE_H_
#define PATCHAPI_H_DYNINST_DYNADDRSPACE_H_

#include <set>
#include <stddef.h>
#include "DynCommon.h"

class AddressSpace;

namespace Dyninst {
namespace PatchAPI {

class DynAddrSpace : public AddrSpace {
  public:
      static DynAddrSpace* create();
      bool loadLibrary(DynObject*);
      bool removeAddrSpace(AddressSpace *);

      typedef std::set<AddressSpace*> AsSet;
      AsSet& asSet() { return as_set_; }
      //typedef std::map<ParseAPI::CodeObject*, AddressSpace*> CoAsMap;
      //CoAsMap& coas_map() { return coas_map_; }
      
      virtual bool write(PatchObject*, Address to, Address from, size_t size);
      virtual Address malloc(PatchObject*, size_t size, Address near);
      virtual bool realloc(PatchObject*, Address orig, size_t size);
      virtual bool free(PatchObject*, Address orig);
      
      bool isRecursive() { return recursive_; }
      void setRecursive(bool r) { recursive_ = r; }

  protected:
    DynAddrSpace();
    DynAddrSpace(AddrSpace* par);

    //CoAsMap coas_map_;
    AsSet as_set_;
    bool recursive_;
};

}
}
#endif  // PATCHAPI_H_DYNINST_DYNADDRSPACE_H_
