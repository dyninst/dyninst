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

#ifndef _BPatch_object_h_
#define _BPatch_object_h_

#include <vector>
#include <string>
#include <set>
#include <utility>
#include "dyntypes.h"
#include "BPatch_dll.h"
#include "StackMod.h"

class mapped_object;
class BPatch_addressSpace;
class AddressSpace;
class BPatch_image;
class BPatch_module;
class BPatch_object;
class BPatch_function;
class BPatch_point;
class BPatchSnippetHandle;
class BPatch_snippet;
namespace Dyninst { 
   namespace ParseAPI { 
      class CodeObject; 
      BPATCH_DLL_EXPORT CodeObject *convert(const BPatch_object *);
   }
   namespace PatchAPI {
      class PatchObject;
      BPATCH_DLL_EXPORT PatchObject *convert(const BPatch_object *);
   }
   namespace SymtabAPI {
      class Symtab;
      BPATCH_DLL_EXPORT Symtab *convert(const BPatch_object *);
   }
}


class BPATCH_DLL_EXPORT BPatch_object {

    friend Dyninst::ParseAPI::CodeObject *Dyninst::ParseAPI::convert(const BPatch_object *);
    friend Dyninst::PatchAPI::PatchObject *Dyninst::PatchAPI::convert(const BPatch_object *);
    friend Dyninst::SymtabAPI::Symtab *Dyninst::SymtabAPI::convert(const BPatch_object *);
    friend class BPatch_image;
    friend class BPatch_module;

    BPatch_image        *img;
    mapped_object      	*obj;
    std::vector<BPatch_module *> mods;

    BPatch_object(mapped_object *o, BPatch_image *i);

    AddressSpace *ll_as();
    BPatch_addressSpace *as();

  public:

    struct Region {
       typedef enum {
          UNKNOWN,
          CODE,
          DATA } type_t;

       Dyninst::Address base;
       unsigned long size;
       type_t type;

       std::string format();
       
    Region() : base(0), size(0), type(UNKNOWN) {}
    Region(Dyninst::Address b, unsigned long s, type_t t) : base(b), size(s), type(t) {}
    };       

    static const Dyninst::Address E_OUT_OF_BOUNDS;

    std::string name();

    std::string pathName();

    
    Dyninst::Address fileOffsetToAddr(const Dyninst::Offset offset);

    void regions(std::vector<Region> &regions);

    void modules(std::vector<BPatch_module *> &modules);

    std::vector<BPatch_function *> * findFunction(std::string name,
						  std::vector<BPatch_function *> &funcs,
						  bool notify_on_failure =true,
						  bool regex_case_sensitive =true,
						  bool incUninstrumentable =false,
						  bool dont_use_regex = false);

    bool findPoints(Dyninst::Address addr, std::vector<BPatch_point *> &points);

    void addModsAllFuncs(const std::set<StackMod *> &mods, bool interproc,
        std::vector<std::pair<BPatch_function *, bool> > &modResults,
        unsigned depthLimit = 0);

    BPatchSnippetHandle*  insertInitCallback(BPatch_snippet& callback);

    BPatchSnippetHandle*  insertFiniCallback(BPatch_snippet& callback);

};

#endif
