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

// $Id: mapped_object.C,v 1.39 2008/09/03 06:08:44 jaw Exp $

#include <string>
#include <cctype>
#include <algorithm>

#include "dyninstAPI/src/mapped_object.h"
#include "dyninstAPI/src/mapped_module.h"
#include "dyninstAPI/src/image.h"
#include "dyninstAPI/src/function.h"
#include "dyninstAPI/h/BPatch_function.h"
#include "dyninstAPI/src/debug.h"
#include "dyninstAPI/src/addressSpace.h"
#include "dyninstAPI/src/dynProcess.h"
#include "symtabAPI/h/Symtab.h"
#include "InstructionDecoder.h"
#include "Parsing.h"
#include "instPoint.h"
#include <boost/tuple/tuple.hpp>
#include "BPatch_image.h"
#include "PatchCFG.h"
#include "PCProcess.h"
#include "compiler_annotations.h"

using namespace Dyninst;
using namespace Dyninst::ParseAPI;
using namespace Dyninst::ProcControlAPI;
#if defined(os_windows)
#define FS_FIELD_SEPERATOR '\\'
#else
#define FS_FIELD_SEPERATOR '/'
#endif
// Whee hasher...


// triggered when parsing needs to check if the underlying data has changed
bool codeBytesUpdateCB(void *objCB, Address targ)
{
    mapped_object *obj = (mapped_object*) objCB;
    return obj->updateCodeBytesIfNeeded(targ);
}

mapped_object::mapped_object(fileDescriptor fileDesc,
      image *img,
      AddressSpace *proc,
      BPatch_hybridMode mode):
  DynObject(img->codeObject(), proc, fileDesc.code()),
  desc_(fileDesc),
  fullName_(img->getObject()->file()),
  dirty_(false),
  dirtyCalled_(false),
  image_(img),
  dlopenUsed(false),
  proc_(proc),
  analyzed_(false),
  analysisMode_(mode),
  pagesUpdated_(true),
  codeByteUpdates_(0),
  memEnd_(-1),
  memoryImg_(false)
{
// Set occupied range (needs to be ranges)
   dataBase_ = fileDesc.data();

#if defined(os_linux)
   // Handling for non-fPIE
   if (codeBase_ == image_->imageOffset()) {
      // Normal, non-PIE executable, so set the codeBase to 0. 
      codeBase_ = 0;
      dataBase_ = 0;
   }
#endif
   
   startup_printf("[%s:%d] Creating new mapped_object %s/%s\n",
                  FILE__, __LINE__, fullName_.c_str(), getFileDesc().member().c_str());
   startup_printf("[%s:%d] \tcodeBase 0x%lx, codeOffset 0x%lx, size %lu\n",
                  FILE__, __LINE__, codeBase_, image_->imageOffset(), image_->imageLength());
   startup_printf("[%s:%d] \tdataBase 0x%lx, dataOffset 0x%lx, size %lu\n",
                  FILE__, __LINE__, dataBase_, image_->dataOffset(), image_->dataLength());
   startup_printf("[%s:%d] \tfileDescriptor: code at 0x%lx, data 0x%lx\n",
                  FILE__, __LINE__, fileDesc.code(), fileDesc.data());
   startup_printf("[%s:%d] \tCode: 0x%lx to 0x%lx\n",
                  FILE__, __LINE__, codeAbs(), codeAbs() + imageSize());
   startup_printf("[%s:%d] \tData: 0x%lx to 0x%lx\n",
                  FILE__, __LINE__, dataAbs(), dataAbs() + dataSize());
   image_->getObject()->rebase(codeBase_);


   // Sets "fileName_"
   set_short_name();

   if (analysisMode_ != BPatch_normalMode && fileName() == "dyninstAPI_RT.dll") {
       startup_cerr << "Warning, running dyninstAPI_RT.dll in normal mode.\n";
       analysisMode_ = BPatch_normalMode;
   }
   tocBase = 0;
}

mapped_object *mapped_object::createMappedObject(Library::const_ptr lib,
                                                 AddressSpace *p,
                                                 BPatch_hybridMode analysisMode,
                                                 bool parseGaps) {
   fileDescriptor desc(lib->getAbsoluteName(),
                       lib->getLoadAddress(),
                       p->usesDataLoadAddress() ? lib->getDataLoadAddress() : lib->getLoadAddress());
   return createMappedObject(desc, p, analysisMode, parseGaps);
}
   

mapped_object *mapped_object::createMappedObject(fileDescriptor &desc,
                                                 AddressSpace *p,
                                                 BPatch_hybridMode analysisMode,
                                                 bool parseGaps) {
   if (!p) return NULL;
   if ( BPatch_defensiveMode == analysisMode ) {
       // parsing in the gaps in defensive mode is a bad idea because
       // we mark all binary regions as possible code-containing areas
       parseGaps = false;
   }
   assert(desc.file() != "");
   startup_printf("%s[%d]:  about to parseImage\n", FILE__, __LINE__);
   startup_printf("%s[%d]: name %s, codeBase 0x%lx, dataBase 0x%lx\n",
                  FILE__, __LINE__, desc.file().c_str(), desc.code(), desc.data());
   image *img = image::parseImage( desc, analysisMode, parseGaps);
   if (!img)  {
      startup_printf("%s[%d]:  failed to parseImage\n", FILE__, __LINE__);
      return NULL;
   }


   if (img->isDyninstRTLib()) {
       parseGaps = false;
   }

   // Adds exported functions and variables..
   startup_printf("%s[%d]:  creating mapped object\n", FILE__, __LINE__);
   mapped_object *obj = new mapped_object(desc, img, p, analysisMode);
   if (img->codeObject()->cs()->getArch() == Arch_ppc64) { 
  const CodeObject::funclist & allFuncs = img->codeObject()->funcs();
  CodeObject::funclist::const_iterator fit = allFuncs.begin();
  for( ; fit != allFuncs.end(); ++fit) {
      parse_func * f = (parse_func*)*fit;
      if (f->getNoPowerPreambleFunc() != NULL) {
          func_instance * preambleFunc = obj->findFunction(f);
          func_instance * noPreambleFunc = obj->findFunction(f->getNoPowerPreambleFunc());
          preambleFunc->setNoPowerPreambleFunc(noPreambleFunc);
          noPreambleFunc->setPowerPreambleFunc(preambleFunc);
          if (obj->getTOCBaseAddress() == 0 && f->getPowerTOCBaseAddress() > 0) {
              obj->setTOCBaseAddress(f->getPowerTOCBaseAddress());
          }
      }
  }
   }
   if (BPatch_defensiveMode == analysisMode) {
       img->register_codeBytesUpdateCB(obj);
   }
   startup_printf("%s[%d]:  leaving createMappedObject(%s)\n", FILE__, __LINE__, desc.file().c_str());

  return obj;
}

mapped_object::mapped_object(const mapped_object *s, AddressSpace *child) :
   codeRange(),
   DynObject(s, child, s->codeBase_),
   desc_(s->desc_),
   fullName_(s->fullName_),
   fileName_(s->fileName_),
   dataBase_(s->dataBase_),
   dirty_(s->dirty_),
   dirtyCalled_(s->dirtyCalled_),
   image_(s->image_),
   dlopenUsed(s->dlopenUsed),
   proc_(child),
   analyzed_(s->analyzed_),
   analysisMode_(s->analysisMode_),
   pagesUpdated_(true),
   codeByteUpdates_(0),
   memEnd_(s->memEnd_),
   memoryImg_(s->memoryImg_)
{
   // Let's do modules
   for (unsigned k = 0; k < s->everyModule.size(); k++) {
      // Doesn't copy things like line info. Ah, well.
      mapped_module *parMod = s->everyModule[k];
      mapped_module *mod = mapped_module::createMappedModule(this, parMod->pmod());
      assert(mod);
      everyModule.push_back(mod);
   }

   copyCFG(const_cast<mapped_object*>(s));

   for (auto iter = s->everyUniqueVariable.begin();
        iter != s->everyUniqueVariable.end(); ++iter) {
      int_variable *parVar = iter->second;
      assert(parVar->mod());
      mapped_module *mod = getOrCreateForkedModule(parVar->mod());
      int_variable *newVar = new int_variable(parVar,
            mod);
      addVariable(newVar);
   }

   assert(BPatch_defensiveMode != analysisMode_);

   image_ = s->image_->clone();
   tocBase = s->tocBase;
}


mapped_object::~mapped_object()
{
   // desc_ is static
   // fullName_ is static
   // fileName_ is static
   // codeBase_ is static
   // dataBase_ is static

   for (unsigned i = 0; i < everyModule.size(); i++)
      delete everyModule[i];
   everyModule.clear();

   for (auto iter = everyUniqueVariable.begin();
        iter != everyUniqueVariable.end(); ++iter) {
      delete iter->second;
   }
   everyUniqueVariable.clear();

   for (auto fm_iter = allFunctionsByMangledName.begin(); 
        fm_iter != allFunctionsByMangledName.end(); ++fm_iter) {
      delete fm_iter->second;
   }
   allFunctionsByMangledName.clear();

   for (auto fp_iter = allFunctionsByPrettyName.begin(); 
        fp_iter != allFunctionsByPrettyName.end(); ++fp_iter) {
      delete fp_iter->second;
   }
   allFunctionsByPrettyName.clear();

   for (auto vm_iter = allVarsByMangledName.begin(); 
        vm_iter != allVarsByMangledName.end(); ++vm_iter) {
      delete vm_iter->second;
   }
   allVarsByMangledName.clear();

   for (auto vp_iter = allVarsByPrettyName.begin(); 
        vp_iter != allVarsByPrettyName.end(); ++vp_iter) {
      delete vp_iter->second;
   }
   allVarsByPrettyName.clear();

   // codeRangesByAddr_ is static
    // Remainder are static
   image::removeImage(image_);
}

Address mapped_object::codeAbs() const {
  return codeBase_ + imageOffset();
}

Address mapped_object::dataAbs() const {
  return dataBase_ + dataOffset();
}

bool mapped_object::isCode(Address addr) const {
   Address offset;
   offset = addr - codeBase();

   return parse_img()->getObject()->isCode(offset);
}

bool mapped_object::isData(Address addr) const {
   Address offset;
   offset = addr - codeBase();

   return parse_img()->getObject()->isData(offset);
}

bool mapped_object::analyze()
{
    if (analyzed_) return true;
  // Create a process-specific version of the image; all functions and
  // variables at an absolute address (and modifiable).

  // At some point, we should do better handling of base
  // addresses. Right now we assume we have one per mapped object

  if (!image_) return false;

  image_->analyzeIfNeeded();

  analyzed_ = true;

  // TODO: CLEANUP, shouldn't need this for loop, calling findFunction forces 
  // PatchAPI to create function objects, destroying lazy function creation
  // We already have exported ones. Force analysis (if needed) and get
  // the functions we created via analysis.
  const CodeObject::funclist & allFuncs = parse_img()->getAllFunctions();
  CodeObject::funclist::const_iterator fit = allFuncs.begin();
  for( ; fit != allFuncs.end(); ++fit) {
      parse_func * f = (parse_func*)*fit;
  // For each function, we want to add our base address
      if((*fit)->src() != HINT)
        findFunction(f);
      
  }

  // Remember: variables don't.
  std::vector<image_variable *> unmappedVars = image_->getCreatedVariables();
  for (unsigned vi = 0; vi < unmappedVars.size(); vi++) {
      findVariable(unmappedVars[vi]);
  }
  return true;
}

mapped_module *mapped_object::findModule(string m_name, bool wildcard)
{
   parsing_printf("findModule for %s (substr match %d)\n",
         m_name.c_str(), wildcard);
   std::string tmp = m_name.c_str();
   for (unsigned i = 0; i < everyModule.size(); i++) {
      if (everyModule[i]->fileName() == m_name ||
            (wildcard &&
             (wildcardEquiv(tmp, everyModule[i]->fileName())))) {
         //parsing_printf("... found!\n");
         return everyModule[i];
      }
   }
   // Create a new one IF there's one in the child pd_module

   pdmodule *pdmod = image_->findModule(m_name, wildcard);
   if (pdmod) {
      mapped_module *mod = mapped_module::createMappedModule(this,
            pdmod);
      everyModule.push_back(mod);
      //parsing_printf("... made new module!\n");
      return mod;
   }
   else {
      //parsing_printf("... error, no module found...\n");
      return NULL;
   }
}


mapped_module *mapped_object::findModule(pdmodule *pdmod)
{
   if (!pdmod) {
      fprintf(stderr, "%s[%d]:  please call this findModule with nonNULL parameter\n", FILE__, __LINE__);
      return NULL;
   }

   assert(pdmod);

   if (pdmod->imExec() != parse_img()) {
      fprintf(stderr, "%s[%d]: WARNING: lookup for module in wrong mapped object! %p != %p\n", FILE__, __LINE__, (void*)pdmod->imExec(), (void*)parse_img());
      fprintf(stderr, "%s[%d]:  \t\t %s \n", FILE__, __LINE__, parse_img()->name().c_str());
      fprintf(stderr, "%s[%d]:  \t %s != \n", FILE__, __LINE__, pdmod->imExec()->name().c_str());
      return NULL;
   }

   //parsing_printf("findModule for pdmod %s\n",
   //pdmod->fullName().c_str());

   for (unsigned i = 0; i < everyModule.size(); i++) {
      if (everyModule[i]->pmod() == pdmod) {
         //parsing_printf("... found at index %d\n", i);
         return everyModule[i];
      }
   }

   mapped_module *mod = mapped_module::createMappedModule(this,
         pdmod);
   if (mod) {
      //parsing_printf("... created new module\n");
      everyModule.push_back(mod);
      return mod;
   }
   else
      return NULL;
}

// fill in "short_name" data member.  Use last component of "name" data
// member with FS_FIELD_SEPERATOR ("/") as field seperator....
void mapped_object::set_short_name() {
   const char *name_string = fullName_.c_str();
   const char *ptr = strrchr(name_string, FS_FIELD_SEPERATOR);
   if (ptr != NULL) {
      fileName_ = ptr+1;
   } else {
      fileName_ = fullName_;
   }
}

const std::vector<func_instance *> *mapped_object::findFuncVectorByPretty(const std::string &funcname)
{
   if (funcname.empty()) return NULL;
   // First, check the underlying image.
   const std::vector<parse_func *> *img_funcs = parse_img()->findFuncVectorByPretty(funcname);
   if (img_funcs == NULL) {
      return NULL;
   }

   assert(img_funcs->size());
   // Fast path:
   auto iter = allFunctionsByPrettyName.find(funcname);
   if (iter != allFunctionsByPrettyName.end() && iter->second != nullptr) {
      // Okay, we've pulled in some of the functions before (this can happen as a
      // side effect of adding functions). But did we get them all?
      std::vector<func_instance *> *map_funcs = iter->second;
      if (map_funcs->size() == img_funcs->size()) {
         // We're allocating at the lower level....
         delete img_funcs;
         return map_funcs;
      }
   }

   // Slow path: check each img_func, add those we don't already have, and return.
   for (unsigned i = 0; i < img_funcs->size(); i++) {
       parse_func *func = (*img_funcs)[i];
       if (funcs_.find(func) == funcs_.end()) {
           findFunction(func);
       }
       assert(funcs_[func]);
   }
   delete img_funcs;
   return allFunctionsByPrettyName[funcname];
}

const std::vector <func_instance *> *mapped_object::findFuncVectorByMangled(const std::string &funcname)
{
    if (funcname.empty()) return NULL;

    // First, check the underlying image.
    const std::vector<parse_func *> *img_funcs = parse_img()->findFuncVectorByMangled(funcname);
    if (img_funcs == NULL) {
       return NULL;
    }

    assert(img_funcs->size());
    // Fast path:
    auto iter = allFunctionsByMangledName.find(funcname);
    if (iter != allFunctionsByMangledName.end() && iter->second != nullptr) {
        // Okay, we've pulled in some of the functions before (this can happen as a
        // side effect of adding functions). But did we get them all?
       std::vector<func_instance *> *map_funcs = iter->second;
       if (map_funcs->size() == img_funcs->size()) {
          // We're allocating at the lower level...
          delete img_funcs;
          return map_funcs;
       }
    }
    
    // Slow path: check each img_func, add those we don't already have, and return.
    for (unsigned i = 0; i < img_funcs->size(); i++) {
       parse_func *func = (*img_funcs)[i];
       if (funcs_.find(func) == funcs_.end()) {
          findFunction(func);
       }
       assert(funcs_[func]);
    }
    delete img_funcs;
    return allFunctionsByMangledName[funcname];
}


const std::vector<int_variable *> *mapped_object::findVarVectorByPretty(const std::string &varname)
{
    if (varname.empty()) return NULL;

    // First, check the underlying image.
    const std::vector<image_variable *> *img_vars = parse_img()->findVarVectorByPretty(varname);
    if (img_vars == NULL) return NULL;

    assert(img_vars->size());
    // Fast path:
    auto iter = allVarsByPrettyName.find(varname);
    if (iter != allVarsByPrettyName.end() && iter->second != nullptr) {
       // Okay, we've pulled in some of the variabletions before (this can happen as a
       // side effect of adding variabletions). But did we get them all?
       std::vector<int_variable *> *map_variables = iter->second;
       if (map_variables->size() == img_vars->size()) {
          delete img_vars;
          return map_variables;
       }
    }
    
    // Slow path: check each img_variable, add those we don't already have, and return.
    for (unsigned i = 0; i < img_vars->size(); i++) {
        image_variable *var = (*img_vars)[i];
        auto iter2 = everyUniqueVariable.find(var);
        if (iter2 == everyUniqueVariable.end()) {
           findVariable(var);
        }
        assert(everyUniqueVariable[var]);
    }
    delete img_vars;
    return allVarsByPrettyName[varname];
}

const std::vector <int_variable *> *mapped_object::findVarVectorByMangled(const std::string &varname)
{
  if (varname.empty()) return NULL;

  // First, check the underlying image.
  const std::vector<image_variable *> *img_vars = parse_img()->findVarVectorByMangled(varname);
  if (img_vars == NULL) return NULL;

  assert(img_vars->size());
  // Fast path:

  auto iter = allVarsByMangledName.find(varname);
  if (iter != allVarsByMangledName.end() && iter->second != nullptr) {
      // Okay, we've pulled in some of the variabletions before (this can happen as a
      // side effect of adding variables). But did we get them all?
     std::vector<int_variable *> *map_variables = iter->second;
      if (map_variables->size() == img_vars->size()) {
         delete img_vars;
         return map_variables;
      }
  }
  
  // Slow path: check each img_variable, add those we don't already have, and return.
  for (unsigned i = 0; i < img_vars->size(); i++) {
     image_variable *var = (*img_vars)[i];
     auto iter2 = everyUniqueVariable.find(var);
     if (iter2 == everyUniqueVariable.end()) {
        findVariable(var);
     }
     assert(everyUniqueVariable[var]);
  }
  delete img_vars;
  return allVarsByMangledName[varname];
}

//Returns one variable, doesn't search other mapped_objects.  Use carefully.
const int_variable *mapped_object::getVariable(const std::string &varname) {
    const std::vector<int_variable *> *vars = NULL;
    vars = findVarVectorByPretty(varname);
    if (!vars) vars = findVarVectorByMangled(varname);
    if (vars) {
        assert(vars->size() > 0);
        return (*vars)[0];
    }
    return NULL;
}

block_instance *mapped_object::findBlockByEntry(Address addr)
{
    std::set<block_instance *> allBlocks;
    if (!findBlocksByAddr(addr, allBlocks)) return NULL;
    for (std::set<block_instance *>::iterator iter = allBlocks.begin();
        iter != allBlocks.end(); ++iter)
    {
        if ((*iter)->start() == addr)
        {
           return *iter;
        }
    }
    return NULL;
}


bool mapped_object::findBlocksByAddr(const Address addr, std::set<block_instance *> &return_blocks)
{
    // Quick bounds check...
    if (addr < codeAbs()) {
        return false;
    }
    if (addr >= (codeAbs() + imageSize())) {
        return false;
    }

    // Duck into the image class to see if anything matches
    set<ParseAPI::Block *> stab;
    parse_img()->findBlocksByAddr(addr - codeBase(), stab);
    if (stab.empty()) return false;

    for (set<ParseAPI::Block *>::iterator llb_iter = stab.begin();
        llb_iter != stab.end(); ++llb_iter)
    {
        // For each block b \in stab
        //   For each func f \in b.funcs()
        //     Let i_f = up_map(f)
        //       add up_map(b, i_f)
        std::vector<ParseAPI::Function *> ll_funcs;
        (*llb_iter)->getFuncs(ll_funcs);
        for (std::vector<ParseAPI::Function *>::iterator llf_iter = ll_funcs.begin();
            llf_iter != ll_funcs.end(); ++llf_iter) {
           block_instance *block = findBlock(*llb_iter);
           assert(block);
           return_blocks.insert(block);
        }
    }
    return !return_blocks.empty();
}

bool mapped_object::findFuncsByAddr(const Address addr, std::set<func_instance *> &return_funcs)
{
    bool ret = false;
    // Quick and dirty implementation
    std::set<block_instance *> found_blocks;
    if (!findBlocksByAddr(addr, found_blocks)) return false;
    for (std::set<block_instance *>::iterator iter = found_blocks.begin();
         iter != found_blocks.end(); ++iter) {
       (*iter)->getFuncs(std::inserter(return_funcs, return_funcs.end()));
       ret = true;
    }
    return ret;
}

func_instance *mapped_object::findFuncByEntry(const Address addr) {
   std::set<func_instance *> found_funcs;
   if (!findFuncsByAddr(addr, found_funcs)) return NULL;
   for (std::set<func_instance *>::iterator iter = found_funcs.begin();
        iter != found_funcs.end(); ++iter) {
      if ((*iter)->entryBlock()->start() == addr) return *iter;
   }
   return NULL;
}


const std::vector<mapped_module *> &mapped_object::getModules() {
    // everyModule may be out of date...
    std::vector<pdmodule *> pdmods;
    parse_img()->getModules(pdmods);
    if (everyModule.size() == pdmods.size())
        return everyModule;
    for (unsigned i = 0; i < pdmods.size(); i++) {
        findModule(pdmods[i]);
    }

    return everyModule;
}

bool mapped_object::getAllFunctions(std::vector<func_instance *> &return_funcs) {
    unsigned start = return_funcs.size();

    const CodeObject::funclist &img_funcs = parse_img()->getAllFunctions();
    CodeObject::funclist::const_iterator fit = img_funcs.begin();
    for( ; fit != img_funcs.end(); ++fit) {
        if(funcs_.find((parse_func*)*fit) == funcs_.end()) {
            findFunction((parse_func*)*fit);
        }
        return_funcs.push_back(SCAST_FI(funcs_[*fit]));
    }
    return return_funcs.size() > start;
}

bool mapped_object::getAllVariables(std::vector<int_variable *> &vars) {
    unsigned start = vars.size();

    const std::vector<image_variable *> &img_vars = parse_img()->getAllVariables();

    for (unsigned i = 0; i < img_vars.size(); i++) {
       auto iter = everyUniqueVariable.find(img_vars[i]);
       if (iter == everyUniqueVariable.end()) {
          findVariable(img_vars[i]);
       }
       vars.push_back(everyUniqueVariable[img_vars[i]]);
    }
    return vars.size() > start;
}

func_instance *mapped_object::findFunction(ParseAPI::Function *papi_func) {
  return SCAST_FI(getFunc(papi_func));
}

void mapped_object::addFunctionName(func_instance *func,
                                    const std::string newName,
                                    func_index_t &index) {
   std::vector<func_instance *> *funcsByName = NULL;
   
   auto iter = index.find(newName); 
   if (iter != index.end()) {
      funcsByName = iter->second;
   }
   else {
      funcsByName = new std::vector<func_instance *>;
      index[newName] = funcsByName;
   }

   assert(funcsByName != NULL);
   if(std::find(funcsByName->begin(),
		funcsByName->end(),
		func) == funcsByName->end()) 
   {
     funcsByName->push_back(func);
   }
}



void mapped_object::addFunction(func_instance *func) {
    // Possibly multiple demangled (pretty) names...
    // And multiple functions (different addr) with the same pretty
    // name. So we have a many::many mapping...
  for (auto pretty_iter = func->pretty_names_begin();
       pretty_iter != func->pretty_names_end();
       ++pretty_iter)
  {
        addFunctionName(func, pretty_iter->c_str(), allFunctionsByPrettyName);
  }
  for(auto symtab_iter = func->symtab_names_begin();
      symtab_iter != func->symtab_names_end();
      ++symtab_iter)
  {
    
    // And multiple symtab names...
        addFunctionName(func, symtab_iter->c_str(), allFunctionsByMangledName);
    }

    func->mod()->addFunction(func);
}

// Enter a function in all the appropriate tables
int_variable *mapped_object::findVariable(image_variable *img_var) {
    if (!img_var) return NULL;

    auto iter = everyUniqueVariable.find(img_var);
    if (iter != everyUniqueVariable.end()) { return iter->second; }

    mapped_module *mod = findModule(img_var->pdmod());
    assert(mod);

    int_variable *var = new int_variable(img_var, dataBase_, mod);
    addVariable(var);
    return var;
}

void mapped_object::addVariable(int_variable *var) {

    // Possibly multiple demangled (pretty) names...
    // And multiple functions (different addr) with the same pretty
    // name. So we have a many::many mapping...
  
  for (auto pretty_iter = var->pretty_names_begin();
         pretty_iter != var->pretty_names_end();
         pretty_iter++) {
        string pretty_name = *pretty_iter;
        std::vector<int_variable *> *varsByPrettyEntry = NULL;

        // Ensure a vector exists
        auto iter = allVarsByPrettyName.find(pretty_name);
        if (iter == allVarsByPrettyName.end()) {
           varsByPrettyEntry = new std::vector<int_variable *>;
           allVarsByPrettyName[pretty_name] = varsByPrettyEntry;
        }
        else {
           varsByPrettyEntry = iter->second;
        }
	if(std::find(varsByPrettyEntry->begin(),
		     varsByPrettyEntry->end(),
		     var) == varsByPrettyEntry->end())
	{
	  varsByPrettyEntry->push_back(var);
	}
	
    }

    
    // And multiple symtab names...
    for (auto symtab_iter = var->symtab_names_begin();
         symtab_iter != var->symtab_names_end();
         symtab_iter++) {
      string symtab_name = *symtab_iter;
      std::vector<int_variable *> *varsBySymTabEntry = NULL;

        // Ensure a vector exist
        auto iter = allVarsByMangledName.find(symtab_name);
        if (iter == allVarsByMangledName.end()) {
           varsBySymTabEntry = new std::vector<int_variable *>;
           allVarsByMangledName[symtab_name] = varsBySymTabEntry;
        }
        else {
           varsBySymTabEntry = iter->second;
        }
	if(std::find(varsBySymTabEntry->begin(),
		     varsBySymTabEntry->end(),
		     var) == varsBySymTabEntry->end())
	{
	  varsBySymTabEntry->push_back(var);
	}
    }

    everyUniqueVariable[var->ivar()] = var;

    var->mod()->addVariable(var);
}

/////////// Dinky functions

// This way we don't have to cross-include every header file in the
// world.

AddressSpace *mapped_object::proc() const { return proc_; }

bool mapped_object::isSharedLib() const
{
    if (isMemoryImg()) return false;

    return parse_img()->isSharedLibrary();
}

bool mapped_object::isStaticExec() const
{
    return parse_img()->getObject()->isStaticBinary();
}

const std::string mapped_object::debugString() const
{
    std::string debug;
    debug = std::string(fileName_.c_str()) + ":"
       + utos(codeBase_)
       + "/" + utos(imageSize());
    return debug;
}

// Search an object for heapage
bool mapped_object::getInfHeapList(std::vector<heapDescriptor> &infHeaps) {
    vector<pair<string,Address> > foundHeaps;

    getInferiorHeaps(foundHeaps);

    for (u_int j = 0; j < foundHeaps.size(); j++) {
        // The string layout is: DYNINSTstaticHeap_size_type_unique
        // Can't allocate a variable-size array on NT, so malloc
        // that sucker
        char *temp_str = (char *)malloc(strlen(foundHeaps[j].first.c_str())+1);
        strcpy(temp_str, foundHeaps[j].first.c_str());
        char *garbage_str = strtok(temp_str, "_"); // Don't care about beginning
        assert(!strcmp("DYNINSTstaticHeap", garbage_str));
        // Name is as is.
        // If address is zero, then skip (error condition)
        if (foundHeaps[j].second == 0) {
            cerr << "Skipping heap " << foundHeaps[j].first.c_str()
                 << "with address 0" << endl;
            continue;
        }
        // Size needs to be parsed out (second item)
        // Just to make life difficult, the heap can have an optional
        // trailing letter (k,K,m,M,g,G) which indicates that it's in
        // kilobytes, megabytes, or gigabytes. Why gigs? I was bored.
        char *heap_size_str = strtok(NULL, "_"); // Second element, null-terminated
        unsigned heap_size = (unsigned) atol(heap_size_str);
        if (heap_size == 0)
            /* Zero size or error, either way this makes no sense for a heap */
        {
            free(temp_str);
            continue;
        }
        switch (heap_size_str[strlen(heap_size_str)-1]) {
        case 'g':
        case 'G':
            heap_size *= 1024;
	    DYNINST_FALLTHROUGH;
        case 'm':
        case 'M':
            heap_size *= 1024;
	    DYNINST_FALLTHROUGH;
        case 'k':
        case 'K':
            heap_size *= 1024;
	    break;
        default:
            break;
        }

        // Type needs to be parsed out. Can someone clean this up?
        inferiorHeapType heap_type;
        char *heap_type_str = strtok(NULL, "_");

        if (!strcmp(heap_type_str, "anyHeap"))
            heap_type = anyHeap;
        else if (!strcmp(heap_type_str, "lowmemHeap"))
            heap_type = lowmemHeap;
        else if (!strcmp(heap_type_str, "dataHeap"))
            heap_type = dataHeap;
        else if (!strcmp(heap_type_str, "textHeap"))
            heap_type = textHeap;
        else if (!strcmp(heap_type_str, "uncopiedHeap"))
            heap_type = uncopiedHeap;
        else {
            cerr << "Unknown heap string " << heap_type_str << " read from file!" << endl;
            free(temp_str);
            continue;
        }
        infHeaps.push_back(heapDescriptor(foundHeaps[j].first.c_str(),
                                          foundHeaps[j].second,
                                          heap_size, heap_type));
        free(temp_str);
    }
    return foundHeaps.size() > 0;
}

unsigned mapped_object::memoryEnd() 
{ 
    if ((long)memEnd_ != -1) {
        return memEnd_;
    }
    memEnd_ = 0;
    vector<SymtabAPI::Region*> regs;
    parse_img()->getObject()->getMappedRegions(regs);
    for (unsigned ridx=0; ridx < regs.size(); ridx++) {
        if (memEnd_ < regs[ridx]->getMemOffset() + regs[ridx]->getMemSize()) {
            memEnd_ = regs[ridx]->getMemOffset() + regs[ridx]->getMemSize();
        }
    }
    memEnd_ += codeBase();
    return memEnd_;
}


// This gets called once per image. Poke through to the internals;
// all we care about, amusingly, is symbol table information.

void mapped_object::getInferiorHeaps(vector<pair<string, Address> > &foundHeaps)
{
    vector<pair<string, Address> > code_heaps;
    vector<pair<string, Address> > data_heaps;

    if (!parse_img()->getInferiorHeaps(code_heaps, data_heaps)) {
        return;
    }


    // We have a bunch of offsets, now add in the base addresses
    for (unsigned i = 0; i < code_heaps.size(); i++) {
		foundHeaps.push_back(pair<string,Address>(code_heaps[i].first,
                                                  code_heaps[i].second + codeBase()));
    }
    for (unsigned i = 0; i < data_heaps.size(); i++) {
        foundHeaps.push_back(pair<string,Address>(data_heaps[i].first,
                                                  data_heaps[i].second + dataBase()));
    }
}


void *mapped_object::getPtrToInstruction(Address addr) const {
   if (!isCode(addr)) return NULL;
   
   Address offset;
   if (proc()->getAddressWidth() == 8) {
      offset = addr - codeBase();
   }
   else {
      offset = ((unsigned) addr) - ((unsigned) codeBase());
   }

  return image_->codeObject()->cs()->getPtrToInstruction(offset);
}

void *mapped_object::getPtrToData(Address addr) const
{
   assert(addr >= dataAbs());
   assert(addr < (dataAbs() + dataSize()));

   // Don't go from the code base... the image adds back in the code
   // offset.
   Address offset = addr - dataBase();
   return image_->codeObject()->cs()->getPtrToData(offset);
}

// mapped objects may contain multiple Symtab::Regions, this function
// should not be used, but must be included in the class because this
// function is a subclass of codeRange
void *mapped_object::get_local_ptr() const
{
    assert(0);// if you crash here, blame me. -kevin
    return NULL;
    //   return image_->getObject()->image_ptr();
}


bool mapped_object::getSymbolInfo(const std::string &n, int_symbol &info)
{
    using SymtabAPI::Symbol;

    assert(image_);

    Symbol *lowlevel_sym = image_->symbol_info(n);
    if (!lowlevel_sym) {
        lowlevel_sym = image_->symbol_info(std::string("_") + n);
    }

    if (!lowlevel_sym) return false;

    if (lowlevel_sym->getType() == Symbol::ST_OBJECT)
        info = int_symbol(lowlevel_sym, dataBase_);
    else
        info = int_symbol(lowlevel_sym, codeBase_);

    return true;
}

mapped_module *mapped_object::getOrCreateForkedModule(mapped_module *parMod)
{
   // Okay. We're forking, and this is the child mapped_object.
   // And now, given a parent module, we need to find the right one
   // in our little baby modules.

   // Since we've already copied modules, we can just do a name lookup.
   mapped_module *childModule = findModule(parMod->fileName(), false);
   assert(childModule);
   return childModule;

}

mapped_module* mapped_object::getDefaultModule()
{
  // Make sure the everyModule vector is initialized
  getModules();

  assert(everyModule.size() > 0);
  return everyModule[0];

}


// Grabs all block_instances corresponding to the region (horribly inefficient)
bool mapped_object::findBlocksByRange(Address startAddr,
                                      Address endAddr,
                                      list<block_instance*> &rangeBlocks)//output
{
   std::set<ParseAPI::Block *> papiBlocks;
   for (Address cur = startAddr; cur < endAddr; ++cur) {
      Address papiCur = cur - codeBase();
      parse_img()->codeObject()->findBlocks(NULL, papiCur, papiBlocks);
   }

   for (std::set<ParseAPI::Block *>::iterator iter = papiBlocks.begin();
        iter != papiBlocks.end(); ++iter) {
      // For each parseAPI block, up-map it to a block_instance
      block_instance *bbl = this->findBlock(*iter);
      assert(bbl);
      rangeBlocks.push_back(bbl);
   }
   return !rangeBlocks.empty();
}

void mapped_object::findFuncsByRange(Address startAddr,
                                      Address endAddr,
                                      std::set<func_instance*> &pageFuncs)
{
   std::list<block_instance *> bbls;
   findBlocksByRange(startAddr, endAddr, bbls);
   for (std::list<block_instance *>::iterator iter = bbls.begin();
        iter != bbls.end(); ++iter) {
      (*iter)->getFuncs(std::inserter(pageFuncs, pageFuncs.end()));
   }
}

/* Re-trigger parsing in the object.  This function should
 * only be invoked if all funcEntryAddrs lie within the boundaries of
 * the object.
 *
 * Copies over the raw data if a funcEntryAddr lies in between
 * the region's disk size and memory size, also copies raw data
 * if the memory around the entry point has changed
 *
 * A true return value means that new functions were parsed
*/
bool mapped_object::parseNewFunctions(vector<Address> &funcEntryAddrs)
{

    bool reparsedObject = false;
    Address baseAddress = codeBase();
    SymtabAPI::Region *reg;
    std::set<SymtabAPI::Region*> visitedRegions;

    // code page bytes may need updating
    if (BPatch_defensiveMode == analysisMode_) {
        setCodeBytesUpdated(false);
    }

    assert(!parse_img()->hasNewBlocks());

    // update regions if necessary, check that functions not parsed already
    vector<Address>::iterator curEntry = funcEntryAddrs.begin();
    while (curEntry != funcEntryAddrs.end()) {
        Address entryOffset = (*curEntry)-baseAddress;
        reg = parse_img()->getObject()->findEnclosingRegion(entryOffset);
        if (reg != NULL) {

            if (parse_img()->codeObject()->defensiveMode() &&
                visitedRegions.end() == visitedRegions.find(reg))
            {
                updateCodeBytesIfNeeded(*curEntry);
                visitedRegions.insert(reg);
            }

            if (parse_img()->findFuncByEntry(entryOffset)) {
                fprintf(stderr,"WARNING: tried to parse at %lx, where a "
                        "function entry exists already %s[%d]\n",
                        *curEntry, FILE__,__LINE__);
                curEntry = funcEntryAddrs.erase(curEntry);
            }
            else {
                curEntry++;
            }

        }
        else {
            fprintf(stderr,"ERROR: passed invalid address %lx to "
                    "parseNewFunctions %s[%d]\n", *curEntry,FILE__,__LINE__);
            assert(0);
            curEntry++;
        }
    }

    // parse at funcEntryAddrs
    curEntry = funcEntryAddrs.begin();
    set<ParseAPI::Function*> tmpfuncs;
    while (curEntry != funcEntryAddrs.end()) {
        Address entryOffset = (*curEntry)  - baseAddress;
        parse_img()->codeObject()->parse( entryOffset, true );

        if ( ! parse_img()->findFuncs(entryOffset, tmpfuncs) ) {
            // parse failed, this can happen when the function is just a
            // jump or return instruction, but it doesn't mean that we
            // didn't do any parsing
            fprintf(stderr,"WARNING, failed to parse function at %lx, "
                    "%s[%d]\n", *curEntry, FILE__, __LINE__);
        }
        else {
            reparsedObject = true;
            tmpfuncs.clear();
        }
        curEntry++;
    }

    assert(consistency(&(*addrSpace())));
    return reparsedObject;
}


/* 0. The target and source must be in the same mapped region, make sure memory
 *    for the target is up to date
 * 1. Parse from target address, add new edge at image layer
 * 2. Register all newly created functions as a result of new edge parsing
 * 3. Add image blocks as block_instances
 * 4. fix up mapping of split blocks with points
 * 5. Add image points, as instPoints
*/
bool mapped_object::parseNewEdges(const std::vector<edgeStub> &stubs)
{
    using namespace SymtabAPI;
    using namespace ParseAPI;

    vector<ParseAPI::CodeObject::NewEdgeToParse> edgesInThisObject;

/* 0. Make sure memory for the target is up to date */

    // Do various checks and set edge types, if necessary
    for (unsigned idx=0; idx < stubs.size(); idx++) {
        mapped_object *targ_obj = proc()->findObject(stubs[idx].trg);
        assert(targ_obj);

        // update target region if needed
        if (BPatch_defensiveMode == hybridMode())
        {
          targ_obj->updateCodeBytesIfNeeded(stubs[idx].trg);
        }

        EdgeTypeEnum edgeType = stubs[idx].type;

        // Determine if this stub already has been parsed
        // Which means looking up a block at the target address
        if (targ_obj->findBlockByEntry(stubs[idx].trg)) {
           cerr << "KEVINTEST: VERIFY THAT I WORK: parsing edge for target that already exists" << endl;
          //continue; //KEVINTODO: don't we maybe want to add the edge anyway?
        }

        // Otherwise we don't have a target block, so we need to make one.
        if (stubs[idx].type == ParseAPI::NOEDGE)
        {
            using namespace InstructionAPI;
            // And we don't know what type of edge this is. Lovely. Let's
            // figure it out from the instruction class, since that's
            // the easy way to do things.
            block_instance::Insns insns;
            stubs[idx].src->getInsns(insns);
            InstructionAPI::Instruction cf = insns[stubs[idx].src->last()];
            assert(cf.isValid());
            switch (cf.getCategory()) {
            case c_CallInsn:
                if (stubs[idx].trg == stubs[idx].src->end())
                {
                    edgeType = CALL_FT;
                }
                else
                {
                    edgeType = CALL;
                }
                break;
            case c_ReturnInsn:
                //edgeType = RET;
                // The above doesn't work according to Nate
                edgeType = INDIRECT;
                break;
            case c_BranchInsn:
                if (cf.readsMemory())
                {
                    edgeType = INDIRECT;
                }
                else if (!cf.allowsFallThrough())
                {
                    edgeType = DIRECT;
                }
                else if (stubs[idx].trg == stubs[idx].src->end())
                {
                    edgeType = COND_NOT_TAKEN;
                }
                else
                {
                    edgeType = COND_TAKEN;
                }
                break;
            default:
                edgeType = FALLTHROUGH;
                break;
            }
        }

		/* 1. Parse from target address, add new edge at image layer  */
		CodeObject::NewEdgeToParse newEdge(stubs[idx].src->llb(),
            stubs[idx].trg - targ_obj->codeBase(),
            stubs[idx].checked, 
            edgeType);
		if (this != targ_obj) {
			std::vector<ParseAPI::CodeObject::NewEdgeToParse> newEdges;
			newEdges.push_back(newEdge);
			targ_obj->parse_img()->codeObject()->parseNewEdges(newEdges);
		}
		else {
			edgesInThisObject.push_back(newEdge);
		}
	}
 
	/* 2. Parse intra-object edges, after removing any edges that 
          would be duplicates at the image-layer */
	parse_img()->codeObject()->parseNewEdges(edgesInThisObject);

    // build list of potentially modified functions
    vector<ParseAPI::Function*> modIFuncs;
    vector<func_instance*> modFuncs;
    for(unsigned sidx=0; sidx < stubs.size(); sidx++) {
        if (stubs[sidx].src != NULL) {
            stubs[sidx].src->llb()->getFuncs(modIFuncs);
        }
    }

    for (unsigned fidx=0; fidx < modIFuncs.size(); fidx++)
    {
       func_instance *func = findFunction(modIFuncs[fidx]);
       modFuncs.push_back(func);

       //func->ifunc()->invalidateCache();//KEVINTEST: used to call this, which might have been important

       modFuncs[fidx]->triggerModified();
       modFuncs[fidx]->blocks();
       modFuncs[fidx]->callBlocks();
       modFuncs[fidx]->exitBlocks();
    }

    assert(consistency(&(*addrSpace())));
    return true;
}


/* 1. Copy the entire region in from the mutatee,
 * 2. if memory emulation is not on, copy blocks back in from the
 * mapped file, since we don't want to copy instrumentation into
 * the mutatee.
 */
void mapped_object::expandCodeBytes(SymtabAPI::Region *reg)
{
    assert(reg);
    void *mappedPtr = reg->getPtrToRawData();
    Address regStart = reg->getMemOffset();
    ParseAPI::Block *cur = NULL;
    ParseAPI::CodeObject *cObj = parse_img()->codeObject();
    ParseAPI::CodeRegion *parseReg = NULL;
    Address copySize = reg->getMemSize();
    void* regBuf = malloc(copySize);
    Address initializedEnd = regStart + copySize;

    set<ParseAPI::CodeRegion*> parseRegs;
    cObj->cs()->findRegions(regStart, parseRegs);
    parseReg = * parseRegs.begin();
    parseRegs.clear();

    // 1. copy memory into regBuf
    Address readAddr = regStart + codeBase();
    if (!proc()->readDataSpace((void*)readAddr,
                               copySize,
                               regBuf,
                               true))
    {
        fprintf(stderr, "%s[%d] Failed to read from region [%lX %lX]\n",
                __FILE__, __LINE__, (long)regStart+codeBase(), copySize);
        assert(0);
    }
    mal_printf("EXTEND_CB: copied to [%lx %lx)\n", codeBase()+regStart, codeBase()+regStart+copySize);


    // 2. copy code bytes back into the regBuf to wipe out instrumentation
    //    and set regBuf to be the data for the region

        // find the first block in the region
        set<ParseAPI::Block*> analyzedBlocks;
        cObj->findBlocks(parseReg, regStart, analyzedBlocks);
        if (analyzedBlocks.size()) {
            cur = * analyzedBlocks.begin();
        } else {
            cur = cObj->findNextBlock(parseReg, regStart);
        }

        // copy code ranges from old mapped data into regBuf
        while (cur != NULL &&
               cur->start() < initializedEnd)
        {
            if ( ! memcpy((void*)((Address)regBuf + cur->start() - regStart),
                          (void*)((Address)mappedPtr + cur->start() - regStart),
                          cur->size()) )
            {
                assert(0);
            }
            mal_printf("EX: uncopy [%lx %lx)\n", codeBase()+cur->start(),codeBase()+cur->end());
            // advance to the next block
            Address prevEnd = cur->end();
            cur = cObj->findBlockByEntry(parseReg,prevEnd);
            if (!cur) {
                cur = cObj->findNextBlock(parseReg,prevEnd);
            }
        }
        mal_printf("Expand region: %lx blocks copied back into mapped file\n",
                   analyzedBlocks.size());

    if (reg->isDirty()) {
        // if isDirty is true, the pointer was created via malloc
        // and we can free it.  If not, isDirty is part of a mapped
        // file and we can't free it
        free( mappedPtr );
    }

    // swap out rawDataPtr for the mapped file
    static_cast<SymtabCodeSource*>(cObj->cs())->
        resizeRegion( reg, reg->getMemSize() );
    reg->setPtrToRawData( regBuf , copySize );

    // expand this mapped_object's codeRange
    if (codeBase() + reg->getMemOffset() + reg->getMemSize()
        >
        codeAbs() + get_size())
    {
        parse_img()->setImageLength( codeBase()
                                     + reg->getMemOffset()
                                     + reg->getMemSize()
                                     - codeAbs() );

    }
}

// 1. use other update functions to update non-code areas of mapped files,
//    expanding them if we overwrote into unmapped areas
// 2. copy overwritten regions into the mapped objects
void mapped_object::updateCodeBytes(const list<pair<Address,Address> > &owRanges)
{
// 1. use other update functions to update non-code areas of mapped files,
//    expanding them if we wrote in un-initialized memory
    using namespace SymtabAPI;
    std::set<Region *> expandRegs;// so we don't update regions more than once
    Address baseAddress = codeBase();

    // figure out which regions need expansion and which need updating
    list<pair<Address,Address> >::const_iterator rIter = owRanges.begin();
    for(; rIter != owRanges.end(); rIter++) {
        Address lastChangeOffset = (*rIter).second -1 -baseAddress;
        Region *curReg = parse_img()->getObject()->findEnclosingRegion
                                                    ( lastChangeOffset );
        if ( lastChangeOffset - curReg->getMemOffset() >= curReg->getDiskSize() ) {
            expandRegs.insert(curReg);
        }
    }
    // expand and update regions
    for (set<Region*>::iterator regIter = expandRegs.begin();
         regIter != expandRegs.end(); regIter++)
    {
        expandCodeBytes(*regIter);
    }
    std::vector<Region *> allregions;
    parse_img()->getObject()->getCodeRegions(allregions);
    for (unsigned int ridx=0; ridx < allregions.size(); ridx++)
    {
        Region *curreg = allregions[ridx];
        if (expandRegs.end() == expandRegs.find(curreg)) {
            updateCodeBytes(curreg); // KEVINOPTIMIZE: major overkill here, only update regions that had unprotected pages
        }
    }

// 2. copy overwritten regions into the mapped objects
    for(rIter = owRanges.begin(); rIter != owRanges.end(); rIter++)
    {
        Address readAddr = rIter->first;

        Region *reg = parse_img()->getObject()->findEnclosingRegion
            ( (*rIter).first - baseAddress );
        unsigned char* regPtr = (unsigned char*)reg->getPtrToRawData()
            + (*rIter).first - baseAddress - reg->getMemOffset();

        if (!proc()->readDataSpace((void*)readAddr,
                                   (*rIter).second - (*rIter).first,
                                   regPtr,
                                   true) )
        {
            assert(0);
        }
        if (0) {
            mal_printf("OW_CB: copied to [%lx %lx): ", rIter->first,rIter->second);
            for (unsigned idx=0; idx < rIter->second - rIter->first; idx++) {
                mal_printf("%2x ", (unsigned) regPtr[idx]);
            }
            mal_printf("\n");
        }
    }
    pagesUpdated_ = true;
}

// this is a helper function
//
// update mapped data for whole object, or just one region, if specified
//
// Read unprotected pages into the mapped file
// (not analyzed code regions so we don't get instrumentation in our parse)
void mapped_object::updateCodeBytes(SymtabAPI::Region * symReg)
{
    assert(NULL != symReg);

    Address base = codeBase();
    ParseAPI::CodeObject *cObj = parse_img()->codeObject();
    std::vector<SymtabAPI::Region *> regions;

    Block *curB = NULL;
    set<ParseAPI::Block *> analyzedBlocks;
    set<ParseAPI::CodeRegion*> parseRegs;

    void *mappedPtr = symReg->getPtrToRawData();
    Address regStart = symReg->getMemOffset();

    cObj->cs()->findRegions(regStart, parseRegs);
    ParseAPI::CodeRegion *parseReg = * parseRegs.begin();
    parseRegs.clear();

    // find the first block in the region
    cObj->findBlocks(parseReg, regStart, analyzedBlocks);
    if (analyzedBlocks.size()) {
        curB = * analyzedBlocks.begin();
        analyzedBlocks.clear();
    } else {
        curB = cObj->findNextBlock(parseReg, regStart);
    }

    Address prevEndAddr = regStart;
    while ( curB != NULL &&
            curB->start() < regStart + symReg->getDiskSize() )
    {
        // if there's a gap between previous and current block
        if (prevEndAddr < curB->start()) {
            // update the mapped file
            Address readAddr = prevEndAddr + base;
            if (!proc()->readDataSpace(
                    (void*)readAddr,
                    curB->start() - prevEndAddr,
                    (void*)((Address)mappedPtr + prevEndAddr - regStart),
                    true))
            {
                assert(0);//read failed
            }
            //mal_printf("UPDATE_CB: copied to [%lx %lx)\n", prevEndAddr+base,curB->start()+base);
        }

        // advance curB to last adjacent block and set prevEndAddr
        prevEndAddr = curB->end();
        Block *ftBlock = cObj->findBlockByEntry(parseReg,prevEndAddr);
        while (ftBlock) {
            curB = ftBlock;
            prevEndAddr = curB->end();
            ftBlock = cObj->findBlockByEntry(parseReg,prevEndAddr);
        }

        curB = cObj->findNextBlock(parseReg, prevEndAddr);

    }
    // read in from prevEndAddr to the end of the region
	// (will read in whole region if there are no ranges in the region)
    if (prevEndAddr < regStart + symReg->getDiskSize()) {
        Address readAddr = prevEndAddr + base;
        if (!proc()->readDataSpace(
                (void*)readAddr,
                regStart + symReg->getDiskSize() - prevEndAddr,
                (void*)((Address)mappedPtr + prevEndAddr - regStart),
                true))
        {
            assert(0);// read failed
        }
    }
    // change all region pages with REPROTECTED status to PROTECTED status
    Address page_size = proc()->proc()->getMemoryPageSize();
    Address curPage = (regStart / page_size) * page_size + base;
    Address regEnd = base + regStart + symReg->getDiskSize();
    for (; protPages_.end() == protPages_.find(curPage)  && curPage < regEnd;
         curPage += page_size) {};
    for (map<Address,WriteableStatus>::iterator pit = protPages_.find(curPage);
         pit != protPages_.end() && pit->first < regEnd;
         pit++)
    {
        pit->second = PROTECTED;
    }
}

// checks if update is needed by looking in the gap between the previous
// and next block for changes to the underlying bytes
//
// should only be called if we've already checked that we're not on an
// analyzed page that's been protected from overwrites, as this
// check would not be needed
bool mapped_object::isUpdateNeeded(Address entry)
{
    using namespace ParseAPI;
    bool updateNeeded = false;
    void* regBuf = NULL;
    Address base = codeBase();

    assert( BPatch_defensiveMode == hybridMode() );

    set<CodeRegion*> cregs;
    CodeObject *co = parse_img()->codeObject();
    co->cs()->findRegions(entry-base, cregs);
    assert( ! co->cs()->regionsOverlap() );
    if (0 == cregs.size()) {
        mal_printf("Object update request has invalid addr[%lx] %s[%d]\n",
                   entry, FILE__,__LINE__);
        return false;
    }
    SymtabCodeRegion *creg = static_cast<SymtabCodeRegion*>( * cregs.begin() );

    // update the range tree, if necessary
    set<ParseAPI::Block *> analyzedBlocks;
    if (parse_img()->findBlocksByAddr(entry-base, analyzedBlocks)) {
        return false; // don't need to update if target is in analyzed code
    }

    // see if the underlying bytes have changed
    //
    // read until the next basic block or until the end of the region
    // to make sure nothing has changed, otherwise we'll want to read
    // the section in again
    Block *nextBlk = co->findNextBlock(creg, entry-base);
    unsigned comparison_size = 0;
    if (nextBlk) {
        comparison_size = nextBlk->start() - (entry-base);
    } else {
        comparison_size = creg->symRegion()->getDiskSize()
            - ( (entry - base) - creg->symRegion()->getMemOffset() );
    }

    // read until first difference, then see if the difference is to known
    // in which case the difference is due to instrumentation, as we would
    // have otherwise detected the overwrite
    Address page_size = proc()->proc()->getMemoryPageSize();
    comparison_size = ( comparison_size <  page_size)
                      ? comparison_size : page_size;
    regBuf = malloc(comparison_size);
    Address readAddr = entry;

   // mal_printf("%s[%d] Comparing %lx bytes starting at %lx\n",
      //      FILE__,__LINE__,comparison_size,entry);
    if (!proc()->readDataSpace((void*)readAddr, comparison_size, regBuf, true)) {
        assert(0);
    }
    void *mappedPtr = (void*)
                      ((Address)creg->symRegion()->getPtrToRawData() +
                        (entry - base - creg->symRegion()->getMemOffset()) );
    //compare
    if (0 != memcmp(mappedPtr,regBuf,comparison_size) ) {
        updateNeeded = true;
    }
    free(regBuf);
    regBuf = NULL;

    return updateNeeded;
}

// checks to see if expansion is needed
bool mapped_object::isExpansionNeeded(Address entry)
{
    using namespace SymtabAPI;
    Address base = codeBase();
    Region * reg = parse_img()->getObject()->findEnclosingRegion(entry - base);

    if (reg->getMemSize() <= reg->getDiskSize()) {
        return false;
    }

    if ( ! parse_img()->getObject()->isCode(entry - base) ) {
        return true;
    }

    if (expansionCheckedRegions_.end() !=
        expansionCheckedRegions_.find(reg)) {
        return false;
    }
    expansionCheckedRegions_.insert(reg);

    // if there is uninitialized space in the region,
    // see if the first few bytes have been updated
    Address compareStart =
        base + reg->getMemOffset() + reg->getDiskSize();
    unsigned compareSize = InstructionAPI::InstructionDecoder::maxInstructionLength;

    Address uninitSize = reg->getMemSize() - reg->getDiskSize();
    if (compareSize > uninitSize) {
        compareSize = uninitSize;
    }
    unsigned char* regBuf = (unsigned char*) malloc(compareSize);
    if (!proc()->readDataSpace((void*)compareStart,compareSize,regBuf,true)) {
        fprintf(stderr, "%s[%d] Failed to read from region [%lX %lX]\n",
                __FILE__, __LINE__, compareStart, compareStart+compareSize);
        assert(0);
    }
    // compare to zero if the region has not been expanded yet
    bool allZeros = true;
    for (unsigned idx=0; allZeros && idx < compareSize; idx++) {
        if (0 != regBuf[idx]) {
            allZeros = false;
        }
    }
    if (allZeros) {
        return false;
    } else {
        return true;
    }
}

// updates the raw code bytes by fetching from memory, if needed
//
// updates if we haven't updated since the last time code could have
// changed, and if the entry address is on an unprotected code page,
// or if the address is in an uninitialized memory,
bool mapped_object::updateCodeBytesIfNeeded(Address entry)
{

    // we may want to switch between normal and defensive
    // instrumentation/parsing for trusted modules. simply return if we're
    // presently in normal mode.
    if (analysisMode_ != BPatch_defensiveMode) {
        return true;
    }

    Address pageAddr = entry -
        (entry % proc()->proc()->getMemoryPageSize());

    if ( pagesUpdated_ ) {
		//cerr << "\t No pages have been updated in mapped_object, ret false" << endl;
        return false;
    }

    if (protPages_.end() != protPages_.find(pageAddr) &&
        PROTECTED == protPages_[pageAddr])
    {
		//cerr << "\t Address corresponds to protected page, ret false" << endl;
        return false;
    }

    bool expand = isExpansionNeeded(entry);
    if ( ! expand ) {
        if ( ! isUpdateNeeded(entry) ) {
			//cerr << "\t Expansion false and no update needed, ret false" << endl;
            return false;
        }
    }

    SymtabAPI::Region * reg = parse_img()->getObject()->findEnclosingRegion
        (entry - codeBase());
    mal_printf("%s[%d] updating region [%lx %lx] for entry point %lx\n",
               FILE__,__LINE__,
               reg->getMemOffset(),
               reg->getMemOffset()+reg->getDiskSize(),
               entry);

    if ( expand ) {
        expandCodeBytes(reg);
    }
    else {
        updateCodeBytes(reg);
    }

    codeByteUpdates_++;
    pagesUpdated_ = true;
    return true;
}

void mapped_object::remove(func_instance *func) {
    
    // clear out module- and BPatch-level data structures 
    BPatch_addressSpace* bpAS = (BPatch_addressSpace*)proc()->up_ptr();
    BPatch_module *bpmod = bpAS->getImage()->findModule(func->mod());
    BPatch_function *bpfunc = bpAS->findOrCreateBPFunc(SCAST_FI(func), bpmod);
    bpfunc->removeCFG();
    bpmod->remove(bpfunc);
    func->mod()->remove(func);

    // remove from func_instance vector
    funcs_.erase(func->ifunc());

    // remove symtab names
    for (auto name_iter = func->symtab_names_begin();
         name_iter != func->symtab_names_end(); 
         ++name_iter) {
       auto map_iter = allFunctionsByMangledName.find(*name_iter);
       if (map_iter == allFunctionsByMangledName.end()) continue;
       
       std::vector<func_instance *> &name_vec = *(map_iter->second);
       for (unsigned i = 0; i < name_vec.size(); ++i) {
          if (name_vec[i] == func) {
             name_vec[i] = name_vec.back();
             name_vec.pop_back();
             if (name_vec.empty()) {
                delete map_iter->second;
                allFunctionsByMangledName.erase(map_iter);
             }
             break;
          }
       }
    }

    // remove pretty names
    for (auto name_iter = func->pretty_names_begin();
         name_iter != func->pretty_names_end(); 
         ++name_iter) {
       auto map_iter = allFunctionsByPrettyName.find(*name_iter);
       if (map_iter == allFunctionsByPrettyName.end()) continue;
       
       std::vector<func_instance *> &name_vec = *(map_iter->second);
       for (unsigned i = 0; i < name_vec.size(); ++i) {
          if (name_vec[i] == func) {
             name_vec[i] = name_vec.back();
             name_vec.pop_back();
             if (name_vec.empty()) {
                delete map_iter->second;
                allFunctionsByPrettyName.erase(map_iter);
             }
             break;
          }
       }
    }

}

void mapped_object::remove(instPoint *point)
{
    BPatch_addressSpace* bpAS = (BPatch_addressSpace*)proc()->up_ptr();
    BPatch_module *bpmod = bpAS->getImage()->findModule(point->func()->mod());
    bpmod->remove(point);
}

// does not delete
void mapped_object::destroy(PatchAPI::PatchBlock *b) {
   calleeNames_.erase(SCAST_BI(b));
}

// does not delete
void mapped_object::destroy(PatchAPI::PatchFunction *f) {
    remove(SCAST_FI(f));
}

void mapped_object::removeEmptyPages()
{
    // get all pages currently containing code from the mapped modules
    set<Address> curPages;
    vector<Address> emptyPages;
    const vector<mapped_module*> & mods = getModules();
    for (unsigned midx=0; midx < mods.size(); midx++) {
        mods[midx]->getAnalyzedCodePages(curPages);
    }
    // find entries in protPages_ that aren't in curPages, add to emptyPages
    for (map<Address,WriteableStatus>::iterator pit= protPages_.begin();
         pit != protPages_.end();
         pit++)
    {
        if (curPages.end() == curPages.find(pit->first)) {
            emptyPages.push_back(pit->first);
        }
    }
    // erase emptyPages from protPages
    for (unsigned pidx=0; pidx < emptyPages.size(); pidx++) {
        protPages_.erase(emptyPages[pidx]);
    }
}

bool mapped_object::isSystemLib(const std::string &objname)
{
   std::string lowname = objname;
   for(char &c : lowname) c = std::tolower(c);

   if (std::string::npos != lowname.find("libdyninstapi_rt"))
      return true;

#if defined(os_linux)
   if (std::string::npos != lowname.find("libc.so"))
      return true;
   if (std::string::npos != lowname.find("libpthread"))
      return true;
#endif

#if defined(os_freebsd)
   if(std::string::npos != lowname.find("libc.so"))
       return true;
   if(std::string::npos != lowname.find("libthr"))
       return true;
#endif

#if defined(os_windows)
   if (std::string::npos != lowname.find("windows\\system32\\") &&
       std::string::npos != lowname.find(".dll"))
       return true;
   if (std::string::npos != lowname.find("kernel32.dll"))
      return true;
   if (std::string::npos != lowname.find("user32.dll"))
      return true;
   if (std::string::npos != lowname.find("advapi32.dll"))
      return true;
   if (std::string::npos != lowname.find("ntdll.dll"))
      return true;
   if (std::string::npos != lowname.find("msvcrt") &&
       std::string::npos != lowname.find(".dll"))
      return true;
   if (std::string::npos != lowname.find(".dll"))
       return true; //KEVINTODO: find a reliable way of detecting windows system libraries
#endif

   return false;
}

bool mapped_object::isExploratoryModeOn()
{
    return BPatch_exploratoryMode == analysisMode_ ||
           BPatch_defensiveMode == analysisMode_;
}

void mapped_object::addProtectedPage(Address pageAddr)
{
    map<Address,WriteableStatus>::iterator iter = protPages_.find(pageAddr);
    if (protPages_.end() == iter) {
        protPages_[pageAddr] = PROTECTED;
    }
    else if (PROTECTED != iter->second) {
        iter->second = REPROTECTED;
    }
}

void mapped_object::removeProtectedPage(Address pageAddr)
{
    map<Address,WriteableStatus>::iterator iter = protPages_.find(pageAddr);
    if (iter == protPages_.end()) {
        // sanity check, make sure there isn't any code on the page, in which
        // case we're unprotecting a page that was originally set to be writeable
        Address pageOffset = pageAddr - codeBase();
        SymtabAPI::Region *reg = parse_img()->getObject()->findEnclosingRegion(pageOffset);
        assert(reg);
        set<CodeRegion*> cregs;
        parse_img()->codeObject()->cs()->findRegions(reg->getMemOffset(), cregs);
        if (!cregs.empty()) { // (if empty, pageAddr is in uninitialized memory)
            ParseAPI::Block *blk = parse_img()->codeObject()->findNextBlock
                (*cregs.begin(), pageOffset);
            Address pageEnd =  pageOffset + proc()->proc()->getMemoryPageSize();
            if (blk && blk->start() < pageEnd) {
                assert(0);
            }
        }
        return;
    }
    iter->second = UNPROTECTED;
}

void mapped_object::setCodeBytesUpdated(bool newval)
{
    if (BPatch_defensiveMode == analysisMode_) {
        if (false == newval && newval != pagesUpdated_) {
            expansionCheckedRegions_.clear();
        }
        pagesUpdated_ = newval;
    } else {
        cerr << "WARNING: requesting update of code bytes from memory "
             <<  "on non-defensive mapped object, ignoring request "
             << fileName().c_str() << " " << __FILE__ << __LINE__ << endl;
    }
}

#if !( (defined(os_linux) || defined(os_freebsd)) && \
       (defined(arch_x86) || defined(arch_x86_64) || defined(arch_power)\
        ||defined(arch_aarch64)\
       ) )
func_instance *mapped_object::findGlobalConstructorFunc(const std::string &) {
    assert(!"Not implemented");
    return NULL;
}

func_instance *mapped_object::findGlobalDestructorFunc(const std::string &) {
    assert(!"Not implemented");
    return NULL;
}
#endif

bool mapped_object::isEmulInsn(Address insnAddr)
{
    return ( emulInsns_.end() != emulInsns_.find(insnAddr) );
}


void mapped_object::setEmulInsnVal(Address insnAddr, void * val)
{
    assert(emulInsns_.end() != emulInsns_.find(insnAddr));
    emulInsns_[insnAddr] = pair<Register,void*>(emulInsns_[insnAddr].first,val);
}

Register mapped_object::getEmulInsnReg(Address insnAddr)
{
    assert(emulInsns_.end() != emulInsns_.find(insnAddr));
    return emulInsns_[insnAddr].first;
}

void mapped_object::addEmulInsn(Address insnAddr, Register effectiveAddrReg)
{
    emulInsns_[insnAddr] = pair<Register,void*>(effectiveAddrReg,(void *)0);
}

std::string mapped_object::getCalleeName(block_instance *b) {
   std::map<block_instance *, std::string>::iterator iter = calleeNames_.find(b);
   if (iter != calleeNames_.end()) return iter->second;

#if defined(os_windows)
   string calleeName;
   if (parse_img()->codeObject()->isIATcall(b->last() - codeBase(), calleeName)) {
      setCalleeName(b, calleeName);
      return calleeName;
   }
#endif

   return std::string();
}

void mapped_object::setCalleeName(block_instance *b, std::string s) {
   calleeNames_[b] = s;
}

// Missing
// findEdge
// findBlock
// findOneBlockByAddr
// splitBlock
// findFuncByEntry
// findBlock (again)

edge_instance *mapped_object::findEdge(ParseAPI::Edge *e,
                                       block_instance *src,
                                       block_instance *trg) {
  edge_instance *inst = SCAST_EI(getEdge(e, src, trg));
  return inst;
}

block_instance *mapped_object::findBlock(ParseAPI::Block *b) {
  return SCAST_BI(getBlock(b));
}

block_instance *mapped_object::findOneBlockByAddr(const Address addr) {
   std::set<block_instance *> possibles;
   findBlocksByAddr(addr, possibles);
   for (std::set<block_instance *>::iterator iter = possibles.begin();
        iter != possibles.end(); ++iter) {
      block_instance::Insns insns;
      (*iter)->getInsns(insns);
      if (insns.find(addr) != insns.end()) {
         return *iter;
      }
   }
   return NULL;
}

void mapped_object::splitBlock(block_instance * b1, 
                               block_instance * b2) 
{
    // fix block mappings in: map<block_instance *, std::string> calleeNames_
    map<block_instance *, std::string>::iterator nit = calleeNames_.find(b1);
    if (calleeNames_.end() != nit) {
        string name = nit->second;
        calleeNames_.erase(nit);
        calleeNames_[b2] = name;
    }
}

func_instance *mapped_object::findFuncByEntry(const block_instance *blk) {
  parse_block *llb = SCAST_PB(blk->llb());
  parse_func* f = llb->getEntryFunc();
  if (!f) return NULL;
  return findFunction(f);
}

func_instance *mapped_object::getCallee(const block_instance *b) const {
   std::map<const block_instance *, func_instance *>::const_iterator iter = callees_.find(b);
   if (iter == callees_.end()) return NULL;
   return iter->second;
}

void mapped_object::setCallee(const block_instance *b, func_instance *f) {
   callees_[b] = f;
}

#include "Symtab.h"

void mapped_object::replacePLTStub(SymtabAPI::Symbol *sym, func_instance *orig, Address newAddr) {
   // Let's play relocation games...
   vector<SymtabAPI::relocationEntry> fbt;
   bool ok = parse_img()->getObject()->getFuncBindingTable(fbt);
   if(!ok) return;
   
   
   for (unsigned i = 0; i < fbt.size(); ++i) {
      if (fbt[i].name() == sym->getMangledName()) {
         proc()->bindPLTEntry(fbt[i], codeBase(), orig, newAddr);
      }
   }
}

string mapped_object::fileName() const { 
  return parse_img()->getObject()->name();
  
}
