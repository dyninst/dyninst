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

#include <string>
#include <vector>
#include "BPatch_object.h"
#include "BPatch_image.h"
#include "mapped_object.h"
#include "BPatch_module.h"
#include "dyntypes.h"
#include "function.h"
#include "block.h"

const Dyninst::Address BPatch_object::E_OUT_OF_BOUNDS((Dyninst::Address) -1);

class BPatch_object_getMod {
  public:
   void operator()(mapped_module *m) { 
      BPatch_module *mod = img->findOrCreateModule(m);
      if (mod) mods.push_back(mod);
   }
   BPatch_object_getMod(BPatch_image *i, std::vector<BPatch_module *> &m) : img(i), mods(m) {};
   BPatch_image *img;
   std::vector<BPatch_module *> &mods;
};

BPatch_object::BPatch_object(mapped_object *o, BPatch_image *i) :
   img(i), obj(o) {
   // Fill in module list
   const pdvector<mapped_module *> &ll_mods = obj->getModules();
   BPatch_object_getMod gm(img, mods);
   std::for_each(ll_mods.begin(), ll_mods.end(), gm);
}

AddressSpace *BPatch_object::ll_as() { return obj->proc(); }

BPatch_addressSpace *BPatch_object::as() { return img->getAddressSpace(); }

std::string BPatch_object::name() {
   return obj->fileName();
}

std::string BPatch_object::pathName() {
   return obj->fullName();
}

Dyninst::Address BPatch_object::fileOffsetToAddr(const Dyninst::Offset fileOffset) {
   // File offset, so duck into SymtabAPI to turn it into a "mem offset" 
   // (aka ELF shifted) address

   Dyninst::SymtabAPI::Symtab *sym = Dyninst::SymtabAPI::convert(this);
   assert(sym);
   
   Dyninst::Offset memOffset = sym->fileToMemOffset(fileOffset);
   if (memOffset == (Dyninst::Offset) -1) return E_OUT_OF_BOUNDS;

   if (memOffset >= obj->imageOffset() &&
       memOffset < (obj->imageOffset() + obj->imageSize())) {
      return memOffset + obj->codeBase();
   }
   if (memOffset >= obj->dataOffset() &&
       memOffset < (obj->dataOffset() + obj->dataSize())) {
      return memOffset + obj->dataBase();
   }
   
   return E_OUT_OF_BOUNDS;
}

void BPatch_object::regions(std::vector<BPatch_object::Region> &regions) {
   regions.push_back(Region(obj->codeAbs(), obj->imageSize(), Region::CODE));
   regions.push_back(Region(obj->dataAbs(), obj->dataSize(), Region::DATA));
}

void BPatch_object::modules(std::vector<BPatch_module *> &modules) {
   modules.insert(modules.end(), mods.begin(), mods.end());
}

struct findFunc {
   void operator()(BPatch_module *mod) {
      mod->findFunction(name.c_str(), funcs, 
                        notify_on_failure, regex_case_sensitive,
                        incUninstrumentable, dont_use_regex);
   }

   findFunc(std::string n, 
            std::vector<BPatch_function *> &f,
            bool n_o_f, bool r, bool i, bool d) : 
      name(n), funcs(f), notify_on_failure(n_o_f),
      regex_case_sensitive(r), incUninstrumentable(i), dont_use_regex(d) {};

   std::string name;
   std::vector<BPatch_function *> &funcs;
   bool notify_on_failure;
   bool regex_case_sensitive;
   bool incUninstrumentable;
   bool dont_use_regex;
};
      
std::vector<BPatch_function *> *BPatch_object::findFunction(std::string name,
                                                               std::vector<BPatch_function *> &funcs,
                                                               bool notify_on_failure,
                                                               bool regex_case_sensitive,
                                                               bool incUninstrumentable,
                                                               bool dont_use_regex) {
   findFunc f(name, funcs, notify_on_failure, regex_case_sensitive,
              incUninstrumentable, dont_use_regex);
   std::for_each(mods.begin(), mods.end(), f);
   return &funcs;
}


bool BPatch_object::findPoints(Dyninst::Address addr,
                                  std::vector<BPatch_point *> &points) {
   block_instance *blk = obj->findOneBlockByAddr(addr);
   if (!blk) return false;

   std::vector<func_instance *> funcs;
   blk->getFuncs(std::back_inserter(funcs));
   for (unsigned i = 0; i < funcs.size(); ++i) {
      BPatch_module *bpmod = img->findOrCreateModule(funcs[i]->mod());
      BPatch_function *bpfunc = as()->findOrCreateBPFunc(funcs[i], bpmod);
      if (!bpfunc) continue;
      instPoint *p = instPoint::preInsn(funcs[i], blk, addr);
      if (!p) continue;
      BPatch_point *pbp = as()->findOrCreateBPPoint(bpfunc, p, BPatch_locInstruction);
      if (pbp) points.push_back(pbp);
   }
   return true;
}

std::string BPatch_object::Region::format() {
   std::stringstream ret;
   
   ret << "[" << hex << base << "," << (base + size) << ","
       << ((type == CODE) ? "CODE" : "DATA") << "]";
   return ret.str();
}

Dyninst::ParseAPI::CodeObject *Dyninst::ParseAPI::convert(const BPatch_object *o) {
   if (!o->obj) return NULL;
   return o->obj->parse_img()->codeObject();
}

Dyninst::PatchAPI::PatchObject *Dyninst::PatchAPI::convert(const BPatch_object *o) {
   if (!o) return NULL;
   return o->obj;
}

Dyninst::SymtabAPI::Symtab *Dyninst::SymtabAPI::convert(const BPatch_object *o) {
   if (!o) return NULL;
   return o->obj->parse_img()->getObject();
}

