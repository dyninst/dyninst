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

// $Id: mapped_module.C,v 1.29 2008/06/19 19:53:30 legendre Exp $

#include "dyninstAPI/src/mapped_module.h"
#include "dyninstAPI/src/mapped_object.h"
#include "dyninstAPI/src/image.h"
#include "dyninstAPI/src/function.h"
#include "dyninstAPI/src/debug.h"
#include "dyninstAPI/src/addressSpace.h"
#include "dyninstAPI/src/dynProcess.h"
#include <iomanip>
#include <string>

const std::vector<func_instance *> &mapped_module::getAllFunctions() 
{
   std::vector<parse_func *> pdfuncs;
   internal_mod_->getFunctions(pdfuncs);

   if (everyUniqueFunction.size() == pdfuncs.size())
      return everyUniqueFunction;

   for (unsigned i = 0; i < pdfuncs.size(); i++) {
      // Will auto-create (and add to this module)
      obj()->findFunction(pdfuncs[i]);
   }

   return everyUniqueFunction;
}

const std::vector<int_variable *> &mapped_module::getAllVariables() 
{
   std::vector<image_variable *> img_vars;
   internal_mod_->getVariables(img_vars);

   if (everyUniqueVariable.size() == img_vars.size())
      return everyUniqueVariable;

   for (unsigned i = 0; i < img_vars.size(); i++) {
      obj()->findVariable(img_vars[i]);
   }

   return everyUniqueVariable;
}

// We rely on the mapped_object for pretty much everything...
void mapped_module::addFunction(func_instance *func) 
{
   // Just the everything vector... the by-name lists are
   // kept in the mapped_object and filtered if we do a lookup.
  if (std::find(everyUniqueFunction.begin(), everyUniqueFunction.end(), func) != everyUniqueFunction.end()) return;
   everyUniqueFunction.push_back(func);
}

void mapped_module::addVariable(int_variable *var) 
{
   everyUniqueVariable.push_back(var);
}

// We rely on the mapped_object for pretty much everything...
void mapped_module::remove(func_instance *func) 
{
   for (unsigned fIdx=0; fIdx < everyUniqueFunction.size(); fIdx++) {
       if (everyUniqueFunction[fIdx] == func) {
           if (fIdx != everyUniqueFunction.size()-1) {
               everyUniqueFunction[fIdx] = everyUniqueFunction.back();
           }
           everyUniqueFunction.pop_back();
           return;
       }
   }
   assert(0 && "Tried to remove function that's not in the module");
}

const string &mapped_module::fileName() const
{
   return pmod()->fileName();
}

mapped_object *mapped_module::obj() const 
{
   return obj_;
}

SymtabAPI::supportedLanguages mapped_module::language() const 
{
   return pmod()->language(); 
}

bool mapped_module::findFuncVectorByMangled(const std::string &funcname,
      std::vector<func_instance *> &funcs)
{
   // For efficiency sake, we grab the image vector and strip out the
   // functions we want.
   // We could also keep them all in modules and ditch the image-wide search; 
   // the problem is that BPatch goes by module and internal goes by image. 
   unsigned orig_size = funcs.size();

   const std::vector<func_instance *> *obj_funcs = obj()->findFuncVectorByMangled(funcname);
   if (!obj_funcs) {
      return false;
   }

   for (unsigned i = 0; i < obj_funcs->size(); i++) {
      if ((*obj_funcs)[i]->mod() == this)
         funcs.push_back((*obj_funcs)[i]);
   }

   return funcs.size() > orig_size;
}

bool mapped_module::findFuncVectorByPretty(const std::string &funcname,
      std::vector<func_instance *> &funcs)
{
   // For efficiency sake, we grab the image vector and strip out the
   // functions we want.
   // We could also keep them all in modules and ditch the image-wide search; 
   // the problem is that BPatch goes by module and internal goes by image. 
   unsigned orig_size = funcs.size();

   const std::vector<func_instance *> *obj_funcs = obj()->findFuncVectorByPretty(funcname);
   if (!obj_funcs) return false;

   for (unsigned i = 0; i < obj_funcs->size(); i++) {
      if ((*obj_funcs)[i]->mod() == this)
         funcs.push_back((*obj_funcs)[i]);
   }
   return funcs.size() > orig_size;
}


pdmodule *mapped_module::pmod() const 
{
   return internal_mod_;
}

void mapped_module::dumpMangled(std::string prefix) const 
{
   // No reason to have this process specific... it just dumps
   // function names.
   pmod()->dumpMangled(prefix);
}

mapped_module::mapped_module(mapped_object *obj,
      pdmodule *pdmod) :
   internal_mod_(pdmod),
   obj_(obj)
{
}

mapped_module *mapped_module::createMappedModule(mapped_object *obj,
      pdmodule *pdmod) 
{
   assert(obj);
   assert(pdmod);
   assert(pdmod->imExec() == obj->parse_img());
   mapped_module *mod = new mapped_module(obj, pdmod);
   // Do things?

   return mod;
}


std::string mapped_module::processDirectories(const std::string &fn) const 
{
   // This is black magic... assume Todd (I think) knew what
   // he was doing....
   if(fn == "")
      return "";

   if(!strstr(fn.c_str(),"/./") &&
         !strstr(fn.c_str(),"/../"))
      return fn;

   std::string ret;
   char suffix[10] = "";
   char prefix[10] = "";
   char* pPath = new char[strlen(fn.c_str())+1];

   strcpy(pPath,fn.c_str());

   if(pPath[0] == '/')
      strcpy(prefix, "/");
   else
      strcpy(prefix, "");

   if(pPath[strlen(pPath)-1] == '/')
      strcpy(suffix, "/");
   else
      strcpy(suffix, "");

   int count = 0;
   char* pPathLocs[1024];
   char* p = strtok(pPath,"/");
   while(p){
      if(!strcmp(p,".")){
         p = strtok(NULL,"/");
         continue;
      }
      else if(!strcmp(p,"..")){
         count--;
         if(((count < 0) && (*prefix != '/')) || 
               ((count >= 0) && !strcmp(pPathLocs[count],"..")))
         {
            count++;
            pPathLocs[count++] = p;
         }
         if(count < 0) count = 0;
      }
      else
         pPathLocs[count++] = p;

      p = strtok(NULL,"/");
   }

   ret += prefix;
   for(int i=0;i<count;i++){
      ret += pPathLocs[i];
      if(i != (count-1))
         ret += "/";
   }
   ret += suffix;

   delete[] pPath;
   return ret;
}

AddressSpace *mapped_module::proc() const 
{
   return obj()->proc(); 
}

int_variable* mapped_module::createVariable(std::string name, Address addr, int size)
{
  Address base = obj()->dataBase();
  Offset offset = addr - base;
  
  image_variable* img_var = pmod()->imExec()->createImageVariable(offset, name, size, pmod());
  int_variable* ret = new int_variable(img_var, base, this);
  obj()->addVariable(ret);
  return ret;
}

bool mapped_module::findFuncsByAddr(const Address addr,
                                    std::set<func_instance *> &funcs) {
   std::set<func_instance *> allFuncs;
   unsigned size = funcs.size();
   if (!obj()->findFuncsByAddr(addr, allFuncs)) return false;
   for (std::set<func_instance *>::iterator iter = allFuncs.begin();
        iter != allFuncs.end(); ++iter) {
      if ((*iter)->mod() == this) funcs.insert(*iter);
   }
   return (funcs.size() > size);
}

#if 0
bool mapped_module::findBlocksByAddr(const Address addr,
                                     std::set<block_instance *> &blocks) {
   std::set<block_instance *> allBlocks;
   unsigned size = blocks.size();
   if (!obj()->findBlocksByAddr(addr, allBlocks)) return false;
   for (std::set<block_instance *>::iterator iter = allBlocks.begin();
        iter != allBlocks.end(); ++iter) {
      if ((*iter)->func()->mod() == this) blocks.insert(*iter);
   }
   return (blocks.size() > size);
}
#endif

void mapped_module::getAnalyzedCodePages(std::set<Address> & pages)
{
    assert(proc()->proc());
    using namespace ParseAPI;
    int pageSize = proc()->proc()->getMemoryPageSize();
    const std::vector<func_instance *> funcs = getAllFunctions();
    for (unsigned fidx=0; fidx < funcs.size(); fidx++) {
      /*        const func_instance::BlockSet&
            blocks = funcs[fidx]->blocks();
	    func_instance::BlockSet::const_iterator bIter;*/
      const PatchFunction::Blockset&
            blocks = funcs[fidx]->blocks();
      PatchFunction::Blockset::const_iterator bIter;
        for (bIter = blocks.begin(); 
            bIter != blocks.end(); 
            bIter++) 
        {
            Address bStart, bEnd, page;
            bStart = (*bIter)->start();
            bEnd = (*bIter)->end();
            page = bStart - (bStart % pageSize);
            while (page < bEnd) { // account for blocks spanning multiple pages
                pages.insert(page);
                page += pageSize;
            }
        }
    }
}


