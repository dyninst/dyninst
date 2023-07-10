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

    // BPatch_object::name
    // Returns the file name of the object
    std::string name();

    // BPatch_object::pathName
    // Returns the full pathname of the object
    std::string pathName();

    
    // BPatch_object::offsetToAddr
    // Converts a file offset into an absolute address suitable for use in looking up
    // functions or points. 
    // For dynamic instrumentation, this is an address in memory.
    // For binary rewriting, this is an offset that can be treated as an address.
    // Returns E_OUT_OF_BOUNDS (-1) on failure
    Dyninst::Address fileOffsetToAddr(const Dyninst::Offset offset);

    // BPatch_object::regions
    // Returns a vector of address ranges occupied by this object
    // May be multiple if there are multiple disjoint ranges, such as
    // separate code and data or multiple code regions
    // Ranges are returned as (base, size, type) tuples. 
    void regions(std::vector<Region> &regions);

    // BPatch_object::modules
    // Returns a vector of BPatch_modules logically contained in this
    // object. 
    // By design, shared libraries contain a single module; executable files contain one or more. 
    void modules(std::vector<BPatch_module *> &modules);

    // BPatch_object::findFunction
    // Returns a vector of functions matching the provided name
    // Maps this operation over its contained modules
    // For backwards compatibility, returns a pointer to the vector argument. 
    std::vector<BPatch_function *> * findFunction(std::string name,
						  std::vector<BPatch_function *> &funcs,
						  bool notify_on_failure =true,
						  bool regex_case_sensitive =true,
						  bool incUninstrumentable =false,
						  bool dont_use_regex = false);

    //  BPatch_object::findPoints
    //
    //  Returns a vector of BPatch_points that correspond with the provided address, one
    //  per function that includes an instruction at that address. Will have one element
    //  if there is not overlapping code. 
    bool findPoints(Dyninst::Address addr, std::vector<BPatch_point *> &points);

    // BPatch_object::addModsAllProcs
    // Apply stack modifications in mods to all functions in the current
    // object.  Perform error checking, handle stack alignment requirements, and
    // generate any modifications required for cleanup at function exit.
    // Atomically adds all modifications in mods; if any mod is found to be
    // unsafe, none of the modifications are applied.  If interproc is true,
    // interprocedural analysis is used for more precise evaluation of
    // modification safety (i.e. modifications that are actually safe are more
    // likely to be correctly identified as safe, but analysis will take
    // longer).  depthLimit specifies the maximum depth allowed for
    // interprocedural analysis, and is only used if interproc is true.  Note
    // that depthLimit 0 will still analyze interprocedural edges within the
    // current object; it just won't analyze edges between this object and
    // another object.
    //
    // Returns in modResults a vector of (function, instrumented) pairs where
    // instrumented is true if stack modifications were successfully added.
    void addModsAllFuncs(const std::set<StackMod *> &mods, bool interproc,
        std::vector<std::pair<BPatch_function *, bool> > &modResults,
        unsigned depthLimit = 0);

    BPatchSnippetHandle*  insertInitCallback(BPatch_snippet& callback);

    BPatchSnippetHandle*  insertFiniCallback(BPatch_snippet& callback);

};

#endif
