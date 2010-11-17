/*
 * Copyright (c) 1996-2009 Barton P. Miller
 * 
 * We provide the Paradyn Parallel Performance Tools (below
 * described as "Paradyn") on an AS IS basis, and do not warrant its
 * validity or performance.  We reserve the right to update, modify,
 * or discontinue this software at any time.  We shall have no
 * obligation to supply such updates or modifications or any other
 * form of support to you.
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
#include "dyninstAPI/src/symtab.h"
#include "dyninstAPI/src/function.h"
#include "dyninstAPI/src/debug.h"
#include "symtabAPI/h/Symtab.h"
#include "process.h"
#include "InstructionDecoder.h"
#include "Parsing.h"
#include "instPoint.h"
#include "MemoryEmulator/memEmulator.h"
#include <boost/tuple/tuple.hpp>

using namespace Dyninst;
using namespace Dyninst::ParseAPI;

#define FS_FIELD_SEPERATOR '/'

// Whee hasher...

unsigned imgFuncHash(const image_func * const &func) {
    return addrHash4((Address) func);
}
unsigned imgVarHash(const image_variable * const &func) 
{
    return addrHash4((Address) func);
}

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
   desc_(fileDesc),
   fullName_(fileDesc.file()), 
   everyUniqueFunction(imgFuncHash),
   everyUniqueVariable(imgVarHash),
   allFunctionsByMangledName(::Dyninst::stringhash),
   allFunctionsByPrettyName(::Dyninst::stringhash),
   allVarsByMangledName(::Dyninst::stringhash),
   allVarsByPrettyName(::Dyninst::stringhash),
   dirty_(false),
   dirtyCalled_(false),
   image_(img),
   dlopenUsed(false),
   proc_(proc),
   analyzed_(false),
   analysisMode_(mode),
   pagesUpdated_(true),
   memEnd_(-1)
{ 
   // Set occupied range (needs to be ranges)
   codeBase_ = fileDesc.code();
   dataBase_ = fileDesc.data();
#if defined(os_windows)
   codeBase_ = fileDesc.loadAddr();
   dataBase_ = fileDesc.loadAddr();
#endif
#if defined(os_aix)
   // AIX defines "virtual" addresses for an a.out inside the file as
   // well as when the system loads the object. As far as I can tell,
   // this only happens for the a.out (for shobjs the file-addresses
   // are 0).  The file-provided addresses are correct, but the
   // OS-provided addresses are not. So if the file includes
   // addresses, use those.  If it doesn't, all of the offsets are
   // from a "section start" that isn't our start. Getting a headache
   // yet? So _that_ needs to be adjusted. We've stored these values
   // in the Object file. We could also adjust all addresses of
   // symbols, but...
   if (image_->imageOffset() >= codeBase_) {
      codeBase_ = 0;
   }
   else if (image_->imageOffset() <= 0x1fffffff) {
      // GCC-ism. This is a shared library with a a.out-like codeOffset.
      // We need to make our base the difference between the two...
      codeBase_ -= image_->imageOffset();
      SymtabAPI::Region *sec;
      image_->getObject()->findRegion(sec, ".text");
      //fprintf(stderr, "codeBase 0x%x, rawPtr 0x%x, BaseOffset 0x%x, size %d\n",
      //	codeBase_, (Address)sec->getPtrToRawData() , image_->getObject()->getBaseAddress());
      codeBase_ += ((Address)sec->getPtrToRawData() - image_->getObject()->getBaseOffset());	
      //      codeBase_ += image_->getObject()->text_reloc();
   }
   else {
      // codeBase_ is the address that the chunk was loaded at; the actual interesting
      // bits start within the chunk. So add in text_reloc (actually, "offset from start
      // of file to interesting bits"). 
      // Non-GCC shared libraries.
      //codeBase_ += image_->getObject()->text_reloc();
      SymtabAPI::Region *sec;
      image_->getObject()->findRegion(sec, ".text");
      //fprintf(stderr, "codeBase 0x%x, rawPtr 0x%x, BaseOffset 0x%x, size %d\n",
      //	codeBase_, (Address)sec->getPtrToRawData() , image_->getObject()->getBaseOffset());
      codeBase_ += ((Address)sec->getPtrToRawData()-image_->getObject()->getBaseOffset());
   }
   if (image_->dataOffset() >= dataBase_) {
      dataBase_ = 0;
   }
   else if (image_->dataOffset() <= 0x2fffffff) {
      // More GCC-isms. 
      dataBase_ -= image_->dataOffset();
   }
   else {
      // *laughs* You'd think this was the same way, right?
      // Well, you're WRONG!
      // For some reason we don't need to add in the data_reloc_...
      //dataBase_ += image_->getObject()->data_reloc();
   }
#endif

#if 0
   fprintf(stderr, "Creating new mapped_object %s/%s\n",
         fullName_.c_str(), getFileDesc().member().c_str());
   fprintf(stderr, "codeBase 0x%x, codeOffset 0x%x, size %d\n",
         codeBase_, image_->imageOffset(), image_->imageLength());
   fprintf(stderr, "dataBase 0x%x, dataOffset 0x%x, size %d\n",
         dataBase_, image_->dataOffset(), image_->dataLength());
   fprintf(stderr, "fileDescriptor: code at 0x%x, data 0x%x\n",
         fileDesc.code(), fileDesc.data());
   fprintf(stderr, "Code: 0x%lx to 0x%lx\n",
         codeAbs(), codeAbs() + imageSize());
   fprintf(stderr, "Data: 0x%lx to 0x%lx\n",
         dataAbs(), dataAbs() + dataSize());
#endif


   // Sets "fileName_"
   set_short_name();
}

mapped_object *mapped_object::createMappedObject(fileDescriptor &desc,
      AddressSpace *p,
      BPatch_hybridMode analysisMode, 
      bool parseGaps) 
{

   if (!p) return NULL;

   if (BPatch_defensiveMode == analysisMode) {
       // parsing in the gaps in defensive mode is a bad idea because
       // we mark all binary regions as possible code-containing areas
       parseGaps = false;
   }

   startup_printf("%s[%d]:  about to parseImage\n", FILE__, __LINE__);
   startup_printf("%s[%d]: name %s, codeBase 0x%lx, dataBase 0x%lx\n",
                  FILE__, __LINE__, desc.file().c_str(), desc.code(), desc.data());
   image *img = image::parseImage( desc, analysisMode, parseGaps );
   if (!img)  {
      startup_printf("%s[%d]:  failed to parseImage\n", FILE__, __LINE__);
      return NULL;
   }

#if defined(os_linux) && defined(arch_x86_64)
   //Our x86_64 is actually reporting negative load addresses.  Go fig.
   // On Linux/x86_64 with 32-bit mutatees this causes problems because we
   // treat the load address as a unsigned 64 bit integer, and things don't
   // correctly wrap.
   //
   // We'll detect this by noticing that the dynamic entry doesn't match up
   // and then correct.
   if (desc.dynamic() &&
       p->getAddressWidth() == 4 && 
       img->getObject()->getElfDynamicOffset() + desc.code() != desc.dynamic())
   {
      Address new_load_addr;
      new_load_addr = desc.dynamic() - img->getObject()->getElfDynamicOffset();
      startup_printf("[%s:%u] - Incorrect binary load address %lx, changing " 
              "to %lx\n", FILE__, __LINE__, (unsigned long) desc.code(), 
              (unsigned long) new_load_addr);
      desc.setCode(new_load_addr);
      desc.setData(new_load_addr);
   }
#endif
   if (!desc.isSharedObject()) {
      //We've seen a case where the a.out is a shared object (RHEL4's
      // version of ssh).  Check if the shared object flag is set in the
      // binary (which is different from the isSharedObject()) call above.
      // If so, we need to update the load address.
      if (p->proc() &&
            (img->getObject()->getObjectType() == SymtabAPI::obj_SharedLib)) {
         //Executable is a shared lib
         p->proc()->setAOutLoadAddress(desc);
      }
      // Search for main, if we can't find it, and we're creating the process, 
      // and not attaching to it, we can find it by instrumenting libc.so
      // Currently this has only been implemented for linux 
#if defined(os_linux)
      // More specifically, x86 and x86_64 linux
#if defined(arch_x86) || defined(arch_x86_64)

      vector <SymtabAPI::Function *> main;
      if (p->proc() && 
          (p->proc()->getTraceState() == noTracing_ts) &&
          !p->proc()->wasCreatedViaAttach() &&
          (!img->getObject()->findFunctionsByName(main,"main") &&
           !img->getObject()->findFunctionsByName(main,"_main"))) {
          fprintf(stderr, "[%s][%d] Module: %s in process %d:\n"
               "\t  is not a shared object so it should contain a symbol for \n"
               "\t  function main. Initial attempt to locate main failed,\n"
               "\t  possibly due to the lack of a .text section\n",
               __FILE__,__LINE__,desc.file().c_str(), p->proc()->getPid());
         p->proc()->setTraceSysCalls(true);
         p->proc()->setTraceState(libcOpenCall_ts);
      }

#endif // arch_x86 || arch_x86_64
#endif // os_linux
   }

   // Adds exported functions and variables..
   startup_printf("%s[%d]:  creating mapped object\n", FILE__, __LINE__);
   mapped_object *obj = new mapped_object(desc, img, p, analysisMode);
   if (BPatch_defensiveMode == analysisMode) {
       img->register_codeBytesUpdateCB(obj);
   }
   startup_printf("%s[%d]:  leaving createMappedObject(%s)\n", FILE__, __LINE__, desc.file().c_str());

   return obj;
}

mapped_object::mapped_object(const mapped_object *s, process *child) :
   codeRange(),
   desc_(s->desc_),
   fullName_(s->fullName_),
   fileName_(s->fileName_),
   codeBase_(s->codeBase_),
   dataBase_(s->dataBase_),
   everyUniqueFunction(imgFuncHash),
   everyUniqueVariable(imgVarHash),
   allFunctionsByMangledName(::Dyninst::stringhash),
   allFunctionsByPrettyName(::Dyninst::stringhash),
   allVarsByMangledName(::Dyninst::stringhash),
   allVarsByPrettyName(::Dyninst::stringhash),
   dirty_(s->dirty_),
   dirtyCalled_(s->dirtyCalled_),
   image_(s->image_),
   dlopenUsed(s->dlopenUsed),
   proc_(child),
   analyzed_(s->analyzed_),
   analysisMode_(s->analysisMode_),
   pagesUpdated_(true)
{
   // Let's do modules
   for (unsigned k = 0; k < s->everyModule.size(); k++) {
      // Doesn't copy things like line info. Ah, well.
      mapped_module *parMod = s->everyModule[k];
      mapped_module *mod = mapped_module::createMappedModule(this, parMod->pmod());
      assert(mod);
      everyModule.push_back(mod);
   }

   const pdvector<int_function *> parFuncs = s->everyUniqueFunction.values();
   for (unsigned i = 0; i < parFuncs.size(); i++) {
      int_function *parFunc = parFuncs[i];
      assert(parFunc->mod());
      mapped_module *mod = getOrCreateForkedModule(parFunc->mod());
      int_function *newFunc = new int_function(parFunc,
            mod,
            child);
      addFunction(newFunc);
   }

   const pdvector<int_variable *> parVars = s->everyUniqueVariable.values();
   for (unsigned j = 0; j < parVars.size(); j++) {
      int_variable *parVar = parVars[j];
      assert(parVar->mod());
      mapped_module *mod = getOrCreateForkedModule(parVar->mod());
      int_variable *newVar = new int_variable(parVar,
            mod);
      addVariable(newVar);
   }

   assert(BPatch_defensiveMode != analysisMode_);

   image_ = s->image_->clone();
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

   pdvector<int_function *> funcs = everyUniqueFunction.values();
   for (unsigned j = 0; j < funcs.size(); j++) {
      delete funcs[j];
   }
   everyUniqueFunction.clear();

   pdvector<int_variable *> vars = everyUniqueVariable.values();
   for (unsigned k = 0; k < vars.size(); k++) {
      delete vars[k];
   }
   everyUniqueVariable.clear();

   pdvector<pdvector<int_function *> * > mangledFuncs = allFunctionsByMangledName.values();
   for (unsigned i = 0; i < mangledFuncs.size(); i++) {
      delete mangledFuncs[i];
   }
   allFunctionsByMangledName.clear();

   pdvector<pdvector<int_function *> * > prettyFuncs = allFunctionsByPrettyName.values();
   for (unsigned i = 0; i < prettyFuncs.size(); i++) {
      delete prettyFuncs[i];
   }
   allFunctionsByPrettyName.clear();

   pdvector<pdvector<int_variable *> * > mV = allVarsByMangledName.values();
   for (unsigned i = 0; i < mV.size(); i++) {
      delete mV[i];
   }
   allVarsByMangledName.clear();

   pdvector<pdvector<int_variable *> * > pV = allVarsByPrettyName.values();
   for (unsigned i = 0; i < pV.size(); i++) {
      delete pV[i];
   }
   allVarsByPrettyName.clear();

   // codeRangesByAddr_ is static
    // Remainder are static
   image::removeImage(image_);
}

bool mapped_object::analyze() 
{
    if (analyzed_) return true;
  // Create a process-specific version of the image; all functions and
  // variables at an absolute address (and modifiable).
  
  // At some point, we should do better handling of base
  // addresses. Right now we assume we have one per mapped object; AIX
  // is a special case with two (one for functions, one for
  // variables).
  
  if (!image_) return false;

  image_->analyzeIfNeeded();

  analyzed_ = true;

  // We already have exported ones. Force analysis (if needed) and get
  // the functions we created via analysis.
  CodeObject::funclist & allFuncs = parse_img()->getAllFunctions();
  CodeObject::funclist::iterator fit = allFuncs.begin();
  for( ; fit != allFuncs.end(); ++fit) {
  // For each function, we want to add our base address
      if((*fit)->src() != HINT)
        findFunction((image_func*)*fit);
  }
  
  // Remember: variables don't.
  pdvector<image_variable *> unmappedVars = image_->getCreatedVariables();
  for (unsigned vi = 0; vi < unmappedVars.size(); vi++) {
      findVariable(unmappedVars[vi]);
  }
  return true;
}

// TODO: this should probably not be a mapped_object method, but since
// for now it is only used by mapped_objects it is
// from a string that is a complete path name to a function in a module
// (ie. "/usr/lib/libc.so.1/write") return a string with the function
// part removed.  return 0 on error
char *mapped_object::getModulePart(std::string &full_path_name) {
    
    char *whole_name = P_strdup(full_path_name.c_str());
    char *next=0;
    char *last=next;
    if((last = P_strrchr(whole_name, '/'))){
        next = whole_name;
        for(u_int i=0;(next!=last)&&(i<full_path_name.length()); i++){
	    next++;
	    if(next == last){
		u_int size = i+2;
	        char *temp_str = new char[size];
	        if(P_strncpy(temp_str,whole_name,size-1)){
                    temp_str[size-1] = '\0';
		    delete whole_name;
		    return temp_str;
		    temp_str = 0;
                } 
            }
        }
    }
    delete whole_name;
    return 0;
}

mapped_module *mapped_object::findModule(string m_name, bool wildcard)
{
   parsing_printf("findModule for %s (substr match %d)\n",
         m_name.c_str(), wildcard);
   std::string tmp = m_name.c_str();	  
   for (unsigned i = 0; i < everyModule.size(); i++) {
      if (everyModule[i]->fileName() == m_name ||
            everyModule[i]->fullName() == m_name ||
            (wildcard &&
             (wildcardEquiv(tmp, everyModule[i]->fileName()) ||
              wildcardEquiv(tmp, everyModule[i]->fullName())))) {
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
      fprintf(stderr, "%s[%d]: WARNING: lookup for module in wrong mapped object! %p != %p\n", FILE__, __LINE__, pdmod->imExec(), parse_img()); 
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

const pdvector<int_function *> *mapped_object::findFuncVectorByPretty(const std::string &funcname)
{
   if (funcname.c_str() == 0) return NULL;
   // First, check the underlying image.
   const pdvector<image_func *> *img_funcs = parse_img()->findFuncVectorByPretty(funcname);
   if (img_funcs == NULL) {
      return NULL;
   }

   assert(img_funcs->size());
   // Fast path:
   if (allFunctionsByPrettyName.defines(funcname)) {
      // Okay, we've pulled in some of the functions before (this can happen as a
      // side effect of adding functions). But did we get them all?
       pdvector<int_function *> *map_funcs = allFunctionsByPrettyName[funcname];
       if (map_funcs->size() == img_funcs->size()) {
           // We're allocating at the lower level....
           delete img_funcs;
           return map_funcs;
       }
   }
   
   // Slow path: check each img_func, add those we don't already have, and return.
   for (unsigned i = 0; i < img_funcs->size(); i++) {
       image_func *func = (*img_funcs)[i];
       if (!everyUniqueFunction.defines(func)) {
           findFunction(func);
       }
       assert(everyUniqueFunction[func]);
   }
   delete img_funcs;
   return allFunctionsByPrettyName[funcname];
} 

const pdvector <int_function *> *mapped_object::findFuncVectorByMangled(const std::string &funcname)
{
    if (funcname.c_str() == 0) return NULL;
    
    // First, check the underlying image.
    const pdvector<image_func *> *img_funcs = parse_img()->findFuncVectorByMangled(funcname);
    if (img_funcs == NULL) return NULL;

    assert(img_funcs->size());
    // Fast path:
    if (allFunctionsByMangledName.defines(funcname)) {
        // Okay, we've pulled in some of the functions before (this can happen as a
        // side effect of adding functions). But did we get them all?
        pdvector<int_function *> *map_funcs = allFunctionsByMangledName[funcname];
        if (map_funcs->size() == img_funcs->size())
            // We're allocating at the lower level...
            delete img_funcs;
            return map_funcs;
    }
    
    // Slow path: check each img_func, add those we don't already have, and return.
    for (unsigned i = 0; i < img_funcs->size(); i++) {
        image_func *func = (*img_funcs)[i];
        if (!everyUniqueFunction.defines(func)) {
            findFunction(func);
        }
        assert(everyUniqueFunction[func]);
    }
    delete img_funcs;
    return allFunctionsByMangledName[funcname];
} 


const pdvector<int_variable *> *mapped_object::findVarVectorByPretty(const std::string &varname)
{
    if (varname.c_str() == 0) return NULL;
    
    // First, check the underlying image.
    const pdvector<image_variable *> *img_vars = parse_img()->findVarVectorByPretty(varname);
    if (img_vars == NULL) return NULL;
    
    assert(img_vars->size());
    // Fast path:
    if (allVarsByPrettyName.defines(varname)) {
        // Okay, we've pulled in some of the variabletions before (this can happen as a
        // side effect of adding variabletions). But did we get them all?
        pdvector<int_variable *> *map_variables = allVarsByPrettyName[varname];
        if (map_variables->size() == img_vars->size()) {
            delete img_vars;
            return map_variables;
        }
    }
    
    // Slow path: check each img_variable, add those we don't already have, and return.
    for (unsigned i = 0; i < img_vars->size(); i++) {
        image_variable *var = (*img_vars)[i];
        if (!everyUniqueVariable.defines(var)) {
            findVariable(var);
        }
        assert(everyUniqueVariable[var]);
    }
    delete img_vars;
    return allVarsByPrettyName[varname];
} 

const pdvector <int_variable *> *mapped_object::findVarVectorByMangled(const std::string &varname)
{
  if (varname.c_str() == 0) return NULL;

  // First, check the underlying image.
  const pdvector<image_variable *> *img_vars = parse_img()->findVarVectorByMangled(varname);
  if (img_vars == NULL) return NULL;

  assert(img_vars->size());
  // Fast path:
  if (allVarsByMangledName.defines(varname)) {
      // Okay, we've pulled in some of the variabletions before (this can happen as a
      // side effect of adding variabletions). But did we get them all?
      pdvector<int_variable *> *map_variables = allVarsByMangledName[varname];
      if (map_variables->size() == img_vars->size()) {
          delete img_vars;
          return map_variables;
      }
  }

  // Slow path: check each img_variable, add those we don't already have, and return.
  for (unsigned i = 0; i < img_vars->size(); i++) {
      image_variable *var = (*img_vars)[i];
      if (!everyUniqueVariable.defines(var)) {
          findVariable(var);
      }
      assert(everyUniqueVariable[var]);
  }
  delete img_vars;
  return allVarsByMangledName[varname];
} 

//Returns one variable, doesn't search other mapped_objects.  Use carefully.
const int_variable *mapped_object::getVariable(const std::string &varname) {
    const pdvector<int_variable *> *vars = NULL; 
    vars = findVarVectorByPretty(varname);
    if (!vars) vars = findVarVectorByMangled(varname);
    if (vars) {
        assert(vars->size() > 0);
        return (*vars)[0];
    }
    return NULL;
}

codeRange *mapped_object::findCodeRangeByAddress(const Address &addr)  {
    // Quick bounds check...
    if (addr < codeAbs()) { 
        return NULL; 
    }
    if (addr >= (codeAbs() + imageSize())) {
        return NULL;
    }

    codeRange *range = NULL;
    if (hybridMode() != BPatch_normalMode && codeRangesByAddr_.find(addr, range)) {
        if (range->is_basicBlockInstance()->block()->llb()->isShared()) {
            mal_printf("WARNING: mapped_obj lookup by addr %lx returning shared "
                       "block [%lx %lx)\n", addr, range->get_address(), 
                       range->get_address() + range->get_size());
        }
        return range;
    }
    // reset range, which may have been modified
    // by codeRange::find
    range = NULL;

    // Duck into the image class to see if anything matches
    set<ParseAPI::Function*> stab;
    parse_img()->findFuncs(addr - codeBase(),stab);
    if(!stab.empty()) {
        // FIXME what if there are multiple functions at this point?
        image_func * img_func = (image_func*)*stab.begin();
        int_function *func = findFunction(img_func);
        assert(func);
        func->blocks(); // Adds to codeRangesByAddr_...
        // And repeat...
        bool res = codeRangesByAddr_.find(addr, range);
        if (!res) {
            // Possible: we do a basic-block level search at this point, and a gap (or non-symtab parsing)
            // may skip an address.
            return NULL;
        }
    }
    
    return range;
}

int_function *mapped_object::findFuncByAddr(const Address &addr) {
    codeRange *range = findCodeRangeByAddress(addr);
    if (!range) return NULL;
    return range->is_function();
}

const pdvector<mapped_module *> &mapped_object::getModules() {
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

bool mapped_object::getAllFunctions(pdvector<int_function *> &funcs) {
    unsigned start = funcs.size();

    CodeObject::funclist &img_funcs = parse_img()->getAllFunctions();
    CodeObject::funclist::iterator fit = img_funcs.begin();
    for( ; fit != img_funcs.end(); ++fit) {
        if(!everyUniqueFunction.defines((image_func*)*fit)) {
            findFunction((image_func*)*fit);
        }
        funcs.push_back(everyUniqueFunction[(image_func*)*fit]);
    }
    return funcs.size() > start;
}

bool mapped_object::getAllVariables(pdvector<int_variable *> &vars) {
    unsigned start = vars.size();

    const pdvector<image_variable *> &img_vars = parse_img()->getAllVariables();
    
    for (unsigned i = 0; i < img_vars.size(); i++) {
        if (!everyUniqueVariable.defines(img_vars[i])) {
            findVariable(img_vars[i]);
        }
        vars.push_back(everyUniqueVariable[img_vars[i]]);
    }
    return vars.size() > start;
}

// Enter a function in all the appropriate tables
int_function *mapped_object::findFunction(image_func *img_func) {
    if (!img_func) {
        fprintf(stderr, "Warning: findFunction with null img_func\n");
        return NULL;
    }
    assert(img_func->getSymtabFunction());

    mapped_module *mod = findModule(img_func->pdmod());
    if (!mod) {
        fprintf(stderr, "%s[%d]: ERROR: cannot find module %p\n", FILE__, __LINE__, img_func->pdmod());
        fprintf(stderr, "%s[%d]:  ERROR:  Cannot find module %s\n", FILE__, __LINE__, img_func->pdmod()->fileName().c_str());
    }
    assert(mod);
    

    if (everyUniqueFunction.defines(img_func)) {
        return everyUniqueFunction[img_func];
    }

    int_function *func = new int_function(img_func, 
                                          codeBase_,
                                          mod);
    addFunction(func);
    return func;
}

void mapped_object::addFunctionName(int_function *func,
                                    const std::string newName,
                                    nameType_t nameType) {
    // DEBUG
    pdvector<int_function *> *funcsByName = NULL;
    
    if (nameType & mangledName) {
        if (!allFunctionsByMangledName.find(newName,
                                            funcsByName)) {
            funcsByName = new pdvector<int_function *>;
            allFunctionsByMangledName[newName] = funcsByName;
        }
    }
    if (nameType & prettyName) {
        if (!allFunctionsByPrettyName.find(newName,
                                           funcsByName)) {
            funcsByName = new pdvector<int_function *>;
            allFunctionsByPrettyName[newName] = funcsByName;
        }
    }
    if (nameType & typedName) {
        return; 
        /*
          // TODO add?
        if (!allFunctionsByPrettyName.find(newName,
                                           funcsByName)) {
            funcsByName = new pdvector<int_function *>;
            allFunctionsByPrettyName[newName] = funcsByName;
        }
        */
    }

    assert(funcsByName != NULL);
    funcsByName->push_back(func);
}
    

void mapped_object::addFunction(int_function *func) {
    /*
    fprintf(stderr, "Adding function %s/%p: %d mangled, %d pretty, %d typed names\n",
            func->symTabName().c_str(),
            func,
            func->symTabNameVector().size(),
            func->prettyNameVector().size(),
            func->typedNameVector().size());
    */

    // Possibly multiple demangled (pretty) names...
    // And multiple functions (different addr) with the same pretty
    // name. So we have a many::many mapping...
    for (unsigned pretty_iter = 0; 
         pretty_iter < func->prettyNameVector().size();
         pretty_iter++) {
        string pretty_name = func->prettyNameVector()[pretty_iter];
        addFunctionName(func, pretty_name.c_str(), prettyName);
    }

    for (unsigned typed_iter = 0; 
         typed_iter < func->typedNameVector().size();
         typed_iter++) {
        string typed_name = func->typedNameVector()[typed_iter];
        addFunctionName(func, typed_name.c_str(), typedName);
    }
    
    // And multiple symtab names...
    for (unsigned symtab_iter = 0; 
         symtab_iter < func->symTabNameVector().size();
         symtab_iter++) {
        string symtab_name = func->symTabNameVector()[symtab_iter];
        addFunctionName(func, symtab_name.c_str(), mangledName);
    }  
    everyUniqueFunction[func->ifunc()] = func;
    func->mod()->addFunction(func);
}  

// Enter a function in all the appropriate tables
int_variable *mapped_object::findVariable(image_variable *img_var) {
    if (!img_var) return NULL;
    
    if (everyUniqueVariable.defines(img_var))
        return everyUniqueVariable[img_var];
    
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
    for (unsigned pretty_iter = 0; 
         pretty_iter < var->prettyNameVector().size();
         pretty_iter++) {
        string pretty_name = var->prettyNameVector()[pretty_iter];
        pdvector<int_variable *> *varsByPrettyEntry = NULL;
        
        // Ensure a vector exists
        if (!allVarsByPrettyName.find(pretty_name.c_str(),  
                                      varsByPrettyEntry)) {
            varsByPrettyEntry = new pdvector<int_variable *>;
            allVarsByPrettyName[pretty_name.c_str()] = varsByPrettyEntry;
        }
        
        (*varsByPrettyEntry).push_back(var);
    }
    
    // And multiple symtab names...
    for (unsigned symtab_iter = 0; 
         symtab_iter < var->symTabNameVector().size();
         symtab_iter++) {
        string symtab_name = var->symTabNameVector()[symtab_iter];
        pdvector<int_variable *> *varsBySymTabEntry = NULL;
        
        // Ensure a vector exists
        if (!allVarsByMangledName.find(symtab_name.c_str(),  
                                       varsBySymTabEntry)) {
            varsBySymTabEntry = new pdvector<int_variable *>;
            allVarsByMangledName[symtab_name.c_str()] = varsBySymTabEntry;
        }
        
        (*varsBySymTabEntry).push_back(var);
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
    return parse_img()->isSharedObj();
    // HELL NO
    //return desc_.isSharedObject();
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
#if !defined(os_aix)
        // AIX: see auxiliary lookup, below.
        return;
#endif
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
    
    // AIX: we scavenge space. Do that here.
    
#if defined(os_aix)
    // ...
    
    // a.out: from the end of the loader to 0x20000000
    // Anything in 0x2....: skip
    // Anything in 0xd....: to the next page
    
    Address start = 0;
    unsigned size = 0;
    
#if 0
    fprintf(stderr, "Looking for inferior heap in %s/%s, codeAbs 0x%x (0x%x/0x%x)\n",
            getFileDesc().file().c_str(),
            getFileDesc().member().c_str(),
            codeAbs(),
            codeBase(),
            codeOffset());
#endif
    
    if (codeAbs() >= 0xd0000000) {
        // This caused problems on sp3-01.cs.wisc.edu; apparently we were overwriting
        // necessary library information. For now I'm disabling it (10FEB06) until
        // we can get a better idea of what was going on.
#if 0
        start = codeAbs() + imageSize();
        start += instruction::size() - (start % (Address)instruction::size());
        size = PAGESIZE - (start % PAGESIZE);
#endif
    }
    else if (codeAbs() > 0x20000000) {
        // ...
    }
    else if (codeAbs() > 0x10000000) {
        // We also have the loader; there is no information on where
        // it goes (ARGH) so we pad the end of the code segment to
        // try and avoid it.
        
        SymtabAPI::Region *sec;
        image_->getObject()->findRegion(sec, ".loader");
        Address loader_end = codeAbs() + 
            //sec.getSecAddr() +
            image_->getObject()->getLoadOffset() +
            sec->getDiskSize();
        //Address loader_end = codeAbs() + 
        //    image_->getObject()->loader_off() +
        //    image_->getObject()->loader_len();
        // If we loaded it up in the data segment, don't use...
        if (loader_end > 0x20000000)
            loader_end = 0;
        Address code_end = codeAbs() + imageSize();
        
        start = (loader_end > code_end) ? loader_end : code_end;
        
        start += instruction::size() - (start % (Address)instruction::size());
        size = (0x20000000 - start);
    }
    
    
    if (start) {
        char name_scratch[1024];
        snprintf(name_scratch, 1023,
                 "DYNINSTstaticHeap_%i_uncopiedHeap_0x%lx_scratchpage_%s",
                 (unsigned) size,
                 start,
                 fileName().c_str());

        foundHeaps.push_back(pair<string,Address>(string(name_scratch),start)); 
    }
#endif
}
    

void *mapped_object::getPtrToInstruction(Address addr) const 
{
   if (addr < codeAbs()) {
      assert(0);
       return NULL;
   }
   if (addr >= (codeAbs() + imageSize())) {
      assert(0);
       return NULL;
   }

   // Only subtract off the codeBase, not the codeBase plus
   // codeOffset -- the image class includes the codeOffset.
   Address offset = addr - codeBase();
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
  mapped_module* ret = findModule("DEFAULT_MODULE");
  if(ret) return ret;

  // Make sure the everyModule vector is initialized
  getModules();
  
  assert(everyModule.size() > 0);
  return everyModule[0];
  
}


// splits int-layer blocks in response to block-splitting at the image-layer,
// adds the split image-layer blocks that are newly created, 
// and adjusts point->block pointers accordingly 
//
// KEVINTODO: this would be much cheaper if we stored pairs of split blocks, 
bool mapped_object::splitIntLayer()
{

#if ! defined (cap_instruction_api)
    // not implemented (or needed, for now) on non-instruction API platforms
    return false;
#else
    Address baseAddr = codeBase();
    using namespace InstructionAPI;
    // iterates through the blocks that were created during block splitting
    std::set< image_basicBlock* > splits = parse_img()->getSplitBlocks();
    set<image_basicBlock*>::iterator bIter;
    std::set<image_func*> splitfuncs;
    for (bIter = splits.begin(); bIter != splits.end(); bIter++) 
    {
        // foreach function corresponding to the block
        image_basicBlock *splitImgB = (*bIter); //latter half
        vector<Function *> funcs;
        splitImgB->getFuncs(funcs);
        for (std::vector<Function*>::iterator fIter = funcs.begin();
             fIter != funcs.end(); 
             fIter++) 
        {
            image_func *imgFunc = dynamic_cast<image_func*>(*fIter);
            splitfuncs.insert(imgFunc);
            int_function   * intFunc  = findFunction(imgFunc);
            int_basicBlock * splitIntB = intFunc->findBlockByOffsetInFunc
                ( splitImgB->firstInsnOffset() - imgFunc->getOffset() );

            // add block to new int_function if necessary
            if (!splitIntB || splitIntB->llb() != splitImgB) {
                // this will adjust the previous block's length if necessary
                intFunc->addMissingBlock(*splitImgB);
            }

            // splitIntB is null if its function is uninstrumentable 
            // (because it is very short or has indirect jumps)
            if (splitIntB) {

                // make point fixes
                instPoint *point = NULL;
                Address current = splitIntB->origInstance()->firstInsnAddr();
                InstructionDecoder dec
                    (getPtrToInstruction(current),
                     splitIntB->origInstance()->get_size(),
                     proc()->getArch());
                Instruction::Ptr insn;
                while(insn = dec.decode()) 
                {
                    point = intFunc->findInstPByAddr( current );
                    if ( point && point->block() != splitIntB ) {
                        point->setBlock( splitIntB );
                    } 
                    current += insn->size();
                }
                // we're at the last instruction, create a point if needed
                if ( !point && 
                     parse_img()->getInstPoint
                         (splitIntB->origInstance()->lastInsnAddr() - baseAddr) ) 
                {
                    intFunc->addMissingPoints();
                    point = intFunc->findInstPByAddr
                        ( splitIntB->origInstance()->lastInsnAddr() );

                    if (!point) {
                        fprintf(stderr,"WARNING: failed to find point for "
                                "block [%lx %lx] at the"
                                " block's lastInsnAddr = %lx %s[%d]\n", 
                                splitIntB->origInstance()->firstInsnAddr(), 
                                splitIntB->origInstance()->endAddr(),
                                splitIntB->origInstance()->lastInsnAddr(),
                                FILE__,__LINE__);
                    }
                }
            }
        }
    }

    // check arbitrary points in functions whose block boundaries may have changed 
    Address baseAddress = codeBase();
    for (std::set<image_func*>::iterator fIter = splitfuncs.begin();
            fIter != splitfuncs.end(); 
            fIter++) 
    {
        int_function *f = findFuncByAddr(baseAddress + (*fIter)->getOffset());
        const pdvector<instPoint*> & points = f->funcArbitraryPoints();
        for (pdvector<instPoint*>::const_iterator pIter = points.begin(); 
             pIter != points.end(); pIter++) 
        {
            Address pointAddr = (*pIter)->addr();
            bblInstance *bbi = (*pIter)->block()->origInstance();
            // fix block boundaries if necessary
            while (pointAddr <  bbi->firstInsnAddr()) 
            {
                bbi = bbi->block()->func()->findBlockInstanceByAddr(
                    bbi->firstInsnAddr() -1 );
                assert(bbi);
            } 
            while (pointAddr >= bbi->endAddr()) 
            {
                bbi = bbi->block()->func()->findBlockInstanceByAddr(
                    bbi->endAddr() );
                assert(bbi);
            }
            if (bbi != (*pIter)->block()->origInstance()) {
                mal_printf("updating block (which was split) for arbitrary"
                           " point %lx; %s[%d]\n",(*pIter)->addr(),
                           FILE__,__LINE__);
                (*pIter)->setBlock(bbi->block());
            }
        }
    }

    return true;

#endif
}

// Grabs all bblInstances corresponding to the region, taking special care 
// to get ALL bblInstances corresponding to an address if it is shared 
// between multiple functions
void mapped_object::findBBIsByRange(Address startAddr,
                                    Address endAddr,
                                    list<bblInstance*> &rangeBlocks)//output
{
    codeRange *range=NULL;
    Address nextAddr = startAddr;

    do {
        // add bblInstance range to output
        if (range != NULL) {
            bblInstance* bbi = range->is_basicBlockInstance();
            assert(bbi);
            rangeBlocks.push_back(bbi);
            if (bbi->block()->llb()->isShared()) {
                vector<ParseAPI::Function*> ifuncs;
                bbi->block()->llb()->getFuncs(ifuncs);
                for (unsigned fidx=0; fidx < ifuncs.size(); fidx++) {
                    rangeBlocks.push_back(proc()->
                        findFuncByInternalFunc((image_func*)ifuncs[fidx])->
                        findBlockInstanceByAddr(bbi->firstInsnAddr()));
                }
            }
        }

        // advance to the next range
        if ( ! codeRangesByAddr_.find(nextAddr, range) ) {
            if ( ! codeRangesByAddr_.successor(nextAddr, range) ) {
                range = NULL;
            }
        }
        if (range) {
            nextAddr = range->get_address() + range->get_size();
        }

    } while (range != NULL && range->get_address() < endAddr);
}

void mapped_object::findFuncsByRange(Address startAddr,
                                      Address endAddr,
                                      std::set<int_function*> &pageFuncs)
{
    codeRange *range=NULL;
    if ( ! codeRangesByAddr_.find(startAddr,range) &&
         ! codeRangesByAddr_.successor(startAddr,range) ) 
    {
        range = NULL;
    }
    while (range != NULL && 
           range->get_address() < endAddr)
    {
        bblInstance* bbi = range->is_basicBlockInstance();
        assert(bbi);
        pageFuncs.insert(bbi->func());
        // advance to the next region
        if ( ! codeRangesByAddr_.successor(
                    range->get_address() + range->get_size(), 
                    range) ) 
        {
           range = NULL;
        }
    }
}

// register functions found by recursive traversal parsing from 
// new entry points that are discovered after the initial parse
void mapped_object::registerNewFunctions()
{
    CodeObject::funclist newFuncs = parse_img()->getAllFunctions();
    CodeObject::funclist::iterator fit = newFuncs.begin();
    for( ; fit != newFuncs.end(); ++fit) {
        image_func *curFunc = (image_func*) *fit;
        if ( ! everyUniqueFunction.defines(curFunc) ) { 
            if(curFunc->src() == HINT)
                mal_printf("adding function of source type hint\n");
            findFunction(curFunc); // does all the work
        }
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

    assert( !parse_img()->hasSplitBlocks() && !parse_img()->hasNewBlocks());

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

    // add the functions we created to mapped_object datastructures
    registerNewFunctions();

    // split int layer
    if (parse_img()->hasSplitBlocks()) {
        splitIntLayer();
        parse_img()->clearSplitBlocks();
    }

    return reparsedObject;
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
    Address regStart = reg->getRegionAddr();
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
    if (proc()->isMemoryEmulated()) {
        bool valid = false;
        boost::tie(valid, readAddr) = proc()->getMemEm()->translate(readAddr);
        assert(valid);
    }
    if (!proc()->readDataSpace((void*)readAddr, 
                               copySize, 
                               regBuf, 
                               true)) 
    {
        fprintf(stderr, "%s[%d] Failed to read from region [%lX %lX]\n",
                __FILE__, __LINE__, (long)regStart+codeBase(), copySize);
        assert(0);
    }
    mal_printf("EX: copied to [%lx %lx)\n", codeBase()+regStart, codeBase()+regStart+copySize);

    if (proc()->isMemoryEmulated()) {
        mal_printf("Expand region: no blocks copied back into mapped file, memEm is on\n");
        return; // instrumentation is not a problem 
    }

    // 2. copy code bytes back into the regBuf before setting it as raw 
    //    data for region

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

    if (reg->isDirty()) {
        // if isDirty is true, the pointer was created via malloc 
        // and we can free it.  If not, isDirty is part of a mapped
        // file and we can't free it
        free( mappedPtr );
    }

    // KEVINTODO: find a cleaner solution than taking over the mapped files
    static_cast<SymtabCodeSource*>(cObj->cs())->
        resizeRegion( reg, reg->getMemSize() );
    reg->setPtrToRawData( regBuf , copySize );

    // expand this mapped_object's codeRange
    if (codeBase() + reg->getMemOffset() + reg->getMemSize() 
        > 
        codeAbs() + get_size())
    {
        proc()->removeOrigRange(this);
        parse_img()->setImageLength( codeBase() 
                                     + reg->getMemOffset()
                                     + reg->getMemSize()
                                     - codeAbs() );
        proc()->addOrigRange(this);
    }
    mal_printf("Expand region: %lx blocks copied back into mapped file\n", 
               analyzedBlocks.size());

    // KEVINTODO: what?  why is this necessary?, I've killed it for now, delete if no failures
    // 
    //// now update all of the other regions
    //std::vector<SymtabAPI::Region*> regions;
    //parse_img()->getObject()->getCodeRegions(regions);
    //for(unsigned rIdx=0; rIdx < regions.size(); rIdx++) {
    //    SymtabAPI::Region *curReg = regions[rIdx];
    //    if (curReg != reg) {
    //        updateCodeBytes(curReg);
    //    }
    //}
}

// 1. use other update functions to update non-code areas of mapped files, 
//    expanding them if we overwrote into unmapped areas
// 2. copy overwritten regions into the mapped objects
void mapped_object::updateCodeBytes(const list<pair<Address,Address> > &owRanges)
{
    bool memEmulation = proc()->isMemoryEmulated();
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
        if ( lastChangeOffset - curReg->getRegionAddr() >= curReg->getDiskSize() ) {
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
            updateCodeBytes(curreg); // KEVINTODO: major overkill here, only update regions that had unprotected pages
        }
    }

// 2. copy overwritten regions into the mapped objects
    for(rIter = owRanges.begin(); rIter != owRanges.end(); rIter++) 
    {
        Address readAddr = rIter->first;
        if (memEmulation) {
            bool valid = false;
            boost::tie(valid, readAddr) = proc()->getMemEm()->translate(readAddr);
            assert(valid);
        }

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
        mal_printf("OW: copied to [%lx %lx): ", rIter->first,rIter->second);
        for (unsigned idx=0; idx < rIter->second - rIter->first; idx++) {
            mal_printf("%2x ", (unsigned) regPtr[idx]);
        }
        mal_printf("\n");
    }
    pagesUpdated_ = true;
}

// this is a helper function
// 
// update mapped data for whole object, or just one region, if specified
//
// Read unprotected pages into the mapped file
// (not analyzed code regions so we don't get instrumentation in our parse)
void mapped_object::updateCodeBytes(SymtabAPI::Region * reg)
{
    Address base = codeBase();
    ParseAPI::CodeObject *cObj = parse_img()->codeObject();

    std::vector<SymtabAPI::Region *> regions;
    if (NULL == reg) {
        parse_img()->getObject()->getCodeRegions(regions);
    } else {
        regions.push_back(reg);
    }

    Block *curB = NULL;
    set<ParseAPI::Block *> analyzedBlocks;
    set<ParseAPI::CodeRegion*> parseRegs;
    for(unsigned rIdx=0; rIdx < regions.size(); rIdx++) {

        SymtabAPI::Region *symReg = regions[rIdx];
        void *mappedPtr = symReg->getPtrToRawData();
        Address regStart = symReg->getRegionAddr();

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
                if (proc()->isMemoryEmulated()) {
                    bool valid = false;
                    boost::tie(valid, readAddr) = proc()->getMemEm()->translate(readAddr);
                    assert(valid);
                }
                if (!proc()->readDataSpace(
                        (void*)readAddr, 
                        curB->start() - prevEndAddr, 
                        (void*)((Address)mappedPtr + prevEndAddr - regStart),
                        true)) 
                {
                    assert(0);//read failed
                }
                mal_printf("UP: copied to [%lx %lx)\n", prevEndAddr+base,curB->start()+base);
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
            if (proc()->isMemoryEmulated()) {
                bool valid = false;
                boost::tie(valid, readAddr) = proc()->getMemEm()->translate(readAddr);
                assert(valid);
            }
            if (!proc()->readDataSpace(
                    (void*)readAddr, 
                    regStart + symReg->getDiskSize() - prevEndAddr, 
                    (void*)((Address)mappedPtr + prevEndAddr - regStart), 
                    true)) 
            {
                assert(0);// read failed
            }
        }
        mal_printf("UP: copied to [%lx %lx)\n", prevEndAddr+base, 
                   base+symReg->getDiskSize());
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
            - ( (entry - base) - creg->symRegion()->getRegionAddr() );
    }

    // read until first difference, then see if the difference is to known
    // in which case the difference is due to instrumentation, as we would 
    // have otherwise detected the overwrite
    Address page_size = proc()->proc()->getMemoryPageSize();
    comparison_size = ( comparison_size <  page_size) 
                      ? comparison_size : page_size;
    regBuf = malloc(comparison_size);
    Address readAddr = entry;
    if (proc()->isMemoryEmulated()) {
        bool valid = false;
		Address translated = 0;
		boost::tie(valid, translated) = proc()->getMemEm()->translate(readAddr);
		if (valid) readAddr = translated;
	}

    mal_printf("%s[%d] Comparing %lx bytes starting at %lx\n",
            FILE__,__LINE__,comparison_size,entry);
    if (!proc()->readDataSpace((void*)readAddr, comparison_size, regBuf, true)) {
        assert(0); 
    }
    void *mappedPtr = (void*)
                      ((Address)creg->symRegion()->getPtrToRawData() +
                        (entry - base - creg->symRegion()->getRegionAddr()) );
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
        base + reg->getRegionAddr() + reg->getDiskSize();
    if (proc()->isMemoryEmulated()) {
        bool valid = false;
        boost::tie(valid, compareStart) = proc()->getMemEm()->translate(compareStart);
        assert(valid);
    }
#if defined(cap_instruction_api)
    unsigned compareSize = InstructionAPI::InstructionDecoder::maxInstructionLength;
#else
    unsigned compareSize = 2 * proc()->getAddressWidth(); 
#endif
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
    assert( BPatch_defensiveMode == analysisMode_ );

    Address pageAddr = entry - 
        (entry % proc()->proc()->getMemoryPageSize());

    if ( pagesUpdated_ ) {
        return false;
    }

    if (protPages_.end() != protPages_.find(pageAddr)) {
        return false;
    }

    bool expand = isExpansionNeeded(entry);
    if ( ! expand ) {
        if ( ! isUpdateNeeded(entry) ) {
            return false;
        }
    }

    SymtabAPI::Region * reg = parse_img()->getObject()->findEnclosingRegion
        (entry - codeBase());
    mal_printf("%s[%d] updating region [%lx %lx] for entry point %lx\n", 
               FILE__,__LINE__,
               reg->getRegionAddr(), 
               reg->getRegionAddr()+reg->getDiskSize(),
               entry);
    
    if ( expand ) {
        expandCodeBytes(reg);
    } 
    else {
        updateCodeBytes(reg);
    }
    pagesUpdated_ = true;
    return true;
}

void mapped_object::removeFunction(int_function *func) {
    // remove from int_function vectore
    everyUniqueFunction.undef(func->ifunc());
    // remove pretty names
    pdvector<int_function *> *funcsByName = NULL;
    for (unsigned pretty_iter = 0; 
         pretty_iter < func->prettyNameVector().size();
         pretty_iter++) 
    {
        allFunctionsByPrettyName.find
            (func->prettyNameVector()[pretty_iter], funcsByName);
        if (funcsByName) {
            for (unsigned fIdx=0; fIdx < funcsByName->size(); fIdx++) {
                if (func == (*funcsByName)[fIdx]) {
                    unsigned lastIdx = funcsByName->size() -1;
                    (*funcsByName)[fIdx] = (*funcsByName)[lastIdx];
                    funcsByName->pop_back();
                    if (funcsByName->size() == 0) {
                        allFunctionsByPrettyName.undef
                            (func->symTabNameVector()[pretty_iter]);
                    }
                }
            }
        }
    }
    // remove typed names
    for (unsigned typed_iter = 0; 
         typed_iter < func->typedNameVector().size();
         typed_iter++) 
    {
        allFunctionsByPrettyName.find
            (func->typedNameVector()[typed_iter], funcsByName);
        if (funcsByName) {
            for (unsigned fIdx=0; fIdx < funcsByName->size(); fIdx++) {
                if (func == (*funcsByName)[fIdx]) {
                    unsigned lastIdx = funcsByName->size() -1;
                    (*funcsByName)[fIdx] = (*funcsByName)[lastIdx];
                    funcsByName->pop_back();
                    if (funcsByName->size() == 0) {
                        allFunctionsByPrettyName.undef
                            (func->symTabNameVector()[typed_iter]);
                    }
                }
            }
        }
    }
    // remove symtab names
    for (unsigned symtab_iter = 0; 
         symtab_iter < func->symTabNameVector().size();
         symtab_iter++) 
    {
        allFunctionsByMangledName.find
            (func->symTabNameVector()[symtab_iter], funcsByName);
        if (funcsByName) {
            for (unsigned fIdx=0; fIdx < funcsByName->size(); fIdx++) {
                if (func == (*funcsByName)[fIdx]) {
                    unsigned lastIdx = funcsByName->size() -1;
                    (*funcsByName)[fIdx] = (*funcsByName)[lastIdx];
                    funcsByName->pop_back();
                    if (funcsByName->size() == 0) {
                        allFunctionsByMangledName.undef
                            (func->symTabNameVector()[symtab_iter]);
                    }
                }
            }
        }
    }  
}

// remove an element from range, these are always original bblInstance's
void mapped_object::removeRange(codeRange *range) {
    codeRange *foundrange = NULL;
    if (codeRangesByAddr_.find(range->get_address(), foundrange) && 
        range == foundrange) 
    {
        codeRangesByAddr_.remove(range->get_address());
    }
}

bool mapped_object::isSystemLib(const std::string &objname)
{
   std::string lowname = objname;
   std::transform(lowname.begin(),lowname.end(),lowname.begin(), 
                  (int(*)(int))std::tolower);
    
   if (std::string::npos != lowname.find("libdyninstapi_rt"))
      return true;

#if defined(os_solaris)
   // Solaris 2.8... we don't grab the initial func always,
   // so fix up this code as well...
   if (std::string::npos != lowname.find("libthread"))
      return true;
#endif

#if defined(os_linux)
   if (std::string::npos != lowname.find("libc.so"))
      return true;
   if (std::string::npos != lowname.find("libpthread"))
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
   if (std::string::npos != lowname.find("ntdll.dll"))
      return true;
   if (std::string::npos != lowname.find("msvcrt") && 
       std::string::npos != lowname.find(".dll"))
      return true;
   if (std::string::npos == lowname.find("\\"))
       return true; //KEVINTODO: can't leave this in, but for now, anything without a path
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
    protPages_.insert(pageAddr);
}

void mapped_object::removeProtectedPage(Address pageAddr)
{
    protPages_.erase(pageAddr);
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
       (defined(arch_x86) || defined(arch_x86_64)) )
int_function *mapped_object::findGlobalConstructorFunc(const std::string &) {
    assert(!"Not implemented");
    return NULL;
}

int_function *mapped_object::findGlobalDestructorFunc(const std::string &) {
    assert(!"Not implemented");
    return NULL;
}
#endif

