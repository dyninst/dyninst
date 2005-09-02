/*
 * Copyright (c) 1996-2004 Barton P. Miller
 * 
 * We provide the Paradyn Parallel Performance Tools (below
 * described as "Paradyn") on an AS IS basis, and do not warrant its
 * validity or performance.  We reserve the right to update, modify,
 * or discontinue this software at any time.  We shall have no
 * obligation to supply such updates or modifications or any other
 * form of support to you.
 * 
 * This license is for research uses.  For such uses, there is no
 * charge. We define "research use" to mean you may freely use it
 * inside your organization for whatever purposes you see fit. But you
 * may not re-distribute Paradyn or parts of Paradyn, in any form
 * source or binary (including derivatives), electronic or otherwise,
 * to any other organization or entity without our permission.
 * 
 * (for other uses, please contact us at paradyn@cs.wisc.edu)
 * 
 * All warranties, including without limitation, any warranty of
 * merchantability or fitness for a particular purpose, are hereby
 * excluded.
 * 
 * By your use of Paradyn, you understand and agree that we (or any
 * other person or entity with proprietary rights in Paradyn) are
 * under no obligation to provide either maintenance services,
 * update services, notices of latent defects, or correction of
 * defects for Paradyn.
 * 
 * Even if advised of the possibility of such damages, under no
 * circumstances shall we (or any other person or entity with
 * proprietary rights in the software licensed hereunder) be liable
 * to you or any third party for direct, indirect, or consequential
 * damages of any character regardless of type of action, including,
 * without limitation, loss of profits, loss of use, loss of good
 * will, or computer failure or malfunction.  You agree to indemnify
 * us (and any other person or entity with proprietary rights in the
 * software licensed hereunder) for any and all liability it may
 * incur to third parties resulting from your use of Paradyn.
 */

// $Id: mapped_object.C,v 1.7 2005/09/02 16:32:14 bernat Exp $

#include "dyninstAPI/src/mapped_object.h"
#include "dyninstAPI/src/mapped_module.h"
#include "dyninstAPI/src/symtab.h"
#include "common/h/String.h"
#include "dyninstAPI/src/showerror.h"
#include "process.h"

#define FS_FIELD_SEPERATOR '/'

// Whee hasher...
unsigned imgFuncHash(const image_func * const &func) {
    return addrHash4((Address) func);
}
unsigned imgVarHash(const image_variable * const &func) {
    return addrHash4((Address) func);
}



mapped_object::mapped_object(fileDescriptor fileDesc,
                             image *img,
			     process *proc):
    desc_(fileDesc),
    fullName_(fileDesc.file()), 
    everyUniqueFunction(imgFuncHash),
    everyUniqueVariable(imgVarHash),
    allFunctionsByMangledName(pdstring::hash),
    allFunctionsByPrettyName(pdstring::hash),
    allVarsByMangledName(pdstring::hash),
    allVarsByPrettyName(pdstring::hash),
    dirty_(false),
    dirtyCalled_(false),
    image_(img),
    dlopenUsed(false),
    proc_(proc),
    analyzed_(false)
{ 
    // Set occupied range (needs to be ranges)
    codeBase_ = fileDesc.code();
    dataBase_ = fileDesc.data();

#if defined(arch_power)
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
    if (image_->codeOffset() >= codeBase_) {
        codeBase_ = 0;
    }
    else {
        codeBase_ += image_->getObject().text_reloc();
    }
    if (image_->dataOffset() >= dataBase_) {
        dataBase_ = 0;
    }
    else {
        // *laughs* You'd think this was the same way, right?
        // Well, you're WRONG!
        // For some reason we don't need to add in the data_reloc_...
        //dataBase_ += image_->getObject().data_reloc();
    }
#endif

#if 0
    fprintf(stderr, "Creating new mapped_object %s/%s\n",
            fullName_.c_str(), getFileDesc().member().c_str());
    fprintf(stderr, "codeBase 0x%x, codeOffset 0x%x, size %d\n",
            codeBase_, image_->codeOffset(), image_->codeLength());
    fprintf(stderr, "dataBase 0x%x, dataOffset 0x%x, size %d\n",
            dataBase_, image_->dataOffset(), image_->dataLength());
    fprintf(stderr, "fileDescriptor: code at 0x%x, data 0x%x\n",
            fileDesc.code(), fileDesc.data());
#endif

    // Sets "fileName_"
    set_short_name();
    
#if 0
    // Let's try delayed parsing, shall we?

    const pdvector<image_func *> &exportedFuncs = image_->getExportedFunctions();
    //fprintf(stderr, "%d exported functions\n", exportedFuncs.size());
    for (unsigned fi = 0; fi < exportedFuncs.size(); fi++) {
        addFunction(exportedFuncs[fi]);
    }
    const pdvector<image_variable *> &exportedVars = image_->getExportedVariables();
    //fprintf(stderr, "%d exported variables\n", exportedVars.size());
    for (unsigned vi = 0; vi < exportedVars.size(); vi++) {
        addVariable(exportedVars[vi]);

    }
#endif
}

mapped_object *mapped_object::createMappedObject(fileDescriptor desc,
                                                 process *p) {
                                                 
    if (!p) return NULL;
    
    image *img = image::parseImage(desc);
    if (!img) return NULL;
    // Adds exported functions and variables..
    mapped_object *obj = new mapped_object(desc, img, p);

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
  allFunctionsByMangledName(pdstring::hash),
  allFunctionsByPrettyName(pdstring::hash),
  allVarsByMangledName(pdstring::hash),
  allVarsByPrettyName(pdstring::hash),
  dirty_(s->dirty_),
  dirtyCalled_(s->dirtyCalled_),
  image_(s->image_),
  dlopenUsed(s->dlopenUsed),
  proc_(child),
  analyzed_(s->analyzed_)
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
                                                 mod);
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
    
    image_ = s->image_->clone();
}

mapped_object::~mapped_object() {
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
    
    image::removeImage(image_);
}

bool mapped_object::analyze() {
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
  pdvector<image_func *> unmappedFuncs = image_->getCreatedFunctions();
  // For each function, we want to add our base address
  for (unsigned fi = 0; fi < unmappedFuncs.size(); fi++) {
      findFunction(unmappedFuncs[fi]);
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
char *mapped_object::getModulePart(pdstring &full_path_name) {
    
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

mapped_module *mapped_object::findModule(pdstring m_name, bool wildcard)
{
    //parsing_printf("findModule for %s (substr match %d)\n",
    //m_name.c_str(), wildcard);
    for (unsigned i = 0; i < everyModule.size(); i++) {
        if (everyModule[i]->fileName() == m_name ||
            everyModule[i]->fullName() == m_name ||
            (wildcard &&
             (m_name.wildcardEquiv(everyModule[i]->fileName()) ||
              m_name.wildcardEquiv(everyModule[i]->fullName())))) {
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


mapped_module *mapped_object::findModule(pdmodule *pdmod) {
    assert(pdmod);

    if (pdmod->exec() != parse_img()) {
        cerr << "WARNING: lookup for module in wrong mapped object!" << endl;
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

const pdvector<int_function *> *mapped_object::findFuncVectorByPretty(const pdstring &funcname)
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
  assert(allFunctionsByPrettyName.defines(funcname));
  return allFunctionsByPrettyName[funcname];
} 

const pdvector <int_function *> *mapped_object::findFuncVectorByMangled(const pdstring &funcname)
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
    assert(allFunctionsByMangledName.defines(funcname));
    return allFunctionsByMangledName[funcname];
} 


const pdvector<int_variable *> *mapped_object::findVarVectorByPretty(const pdstring &varname)
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
        if (map_variables->size() == img_vars->size())
            return map_variables;
    }
    
    // Slow path: check each img_variable, add those we don't already have, and return.
    for (unsigned i = 0; i < img_vars->size(); i++) {
        image_variable *var = (*img_vars)[i];
        if (!everyUniqueVariable.defines(var)) {
            findVariable(var);
        }
        assert(everyUniqueVariable[var]);
    }
    assert(allVarsByPrettyName.defines(varname));
    return allVarsByPrettyName[varname];
} 

const pdvector <int_variable *> *mapped_object::findVarVectorByMangled(const pdstring &varname)
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
      if (map_variables->size() == img_vars->size())
          return map_variables;
  }

  // Slow path: check each img_variable, add those we don't already have, and return.
  for (unsigned i = 0; i < img_vars->size(); i++) {
      image_variable *var = (*img_vars)[i];
      if (!everyUniqueVariable.defines(var)) {
          findVariable(var);
      }
      assert(everyUniqueVariable[var]);
  }
  assert(allVarsByMangledName.defines(varname));
  return allVarsByMangledName[varname];
} 

codeRange *mapped_object::findCodeRangeByAddress(const Address &addr)  {
    // Quick bounds check...
    if (addr < codeAbs()) { 
        return NULL; 
    }
    if (addr >= (codeAbs() + codeSize())) {
        return NULL;
    }

    codeRange *range;
    if (codeRangesByAddr_.find(addr, range)) {
        return range;
    }

    // Duck into the image class to see if anything matches
    codeRange *img_range = parse_img()->findCodeRangeByOffset(addr - codeBase());
    if (!img_range)
        return NULL;

    if (img_range->is_image_func()) {
        image_func *img_func = img_range->is_image_func();
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
        return range;
    }
    else {
        fprintf(stderr, "ERROR: unknown lookup type at %s/%d, findCodeRange(0x%lx)\n",
                __FILE__, __LINE__, addr);
    }
    return NULL;
}

int_function *mapped_object::findFuncByAddr(const Address &addr) {
    codeRange *range = findCodeRangeByAddress(addr);
    if (!range) return NULL;
    return range->is_function();
}

const pdvector<mapped_module *> &mapped_object::getModules() {
    // everyModule may be out of date...
    const pdvector<pdmodule *> &pdmods = parse_img()->getModules();
    if (everyModule.size() == pdmods.size())
        return everyModule;
    for (unsigned i = 0; i < pdmods.size(); i++) {
        findModule(pdmods[i]);
    }
    
    return everyModule;
}

bool mapped_object::getAllFunctions(pdvector<int_function *> &funcs) {
    unsigned start = funcs.size();

    const pdvector<image_func *> &img_funcs = parse_img()->getAllFunctions();
    for (unsigned i = 0; i < img_funcs.size(); i++) {
        if (!everyUniqueFunction.defines(img_funcs[i])) {
            findFunction(img_funcs[i]);
        }
        funcs.push_back(everyUniqueFunction[img_funcs[i]]);
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
    if (!img_func) return NULL;
    mapped_module *mod = findModule(img_func->pdmod());
    assert(mod);
    
    if (everyUniqueFunction.defines(img_func))
        return everyUniqueFunction[img_func];
    
    int_function *func = new int_function(img_func, 
                                          codeBase_,
                                          mod);
    addFunction(func);
    return func;
}

void mapped_object::addFunction(int_function *func) {
    // Possibly multiple demangled (pretty) names...
    // And multiple functions (different addr) with the same pretty
    // name. So we have a many::many mapping...
    for (unsigned pretty_iter = 0; 
         pretty_iter < func->prettyNameVector().size();
         pretty_iter++) {
        pdstring pretty_name = func->prettyNameVector()[pretty_iter];
        pdvector<int_function *> *funcsByPrettyEntry = NULL;
        
        // Ensure a vector exists
        if (!allFunctionsByPrettyName.find(pretty_name,			      
                                           funcsByPrettyEntry)) {
            funcsByPrettyEntry = new pdvector<int_function *>;
            allFunctionsByPrettyName[pretty_name] = funcsByPrettyEntry;
        }
        
        (*funcsByPrettyEntry).push_back(func);
    }
    
    // And multiple symtab names...
    for (unsigned symtab_iter = 0; 
         symtab_iter < func->symTabNameVector().size();
         symtab_iter++) {
        pdstring symtab_name = func->symTabNameVector()[symtab_iter];
        pdvector<int_function *> *funcsBySymTabEntry = NULL;
        
        // Ensure a vector exists
        if (!allFunctionsByMangledName.find(symtab_name,			      
                                            funcsBySymTabEntry)) {
            funcsBySymTabEntry = new pdvector<int_function *>;
            allFunctionsByMangledName[symtab_name] = funcsBySymTabEntry;
        }
        
        
        (*funcsBySymTabEntry).push_back(func);
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
    
    int_variable *var = new int_variable(img_var,
                                         dataBase_,
                                         mod);

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
        pdstring pretty_name = var->prettyNameVector()[pretty_iter];
        pdvector<int_variable *> *varsByPrettyEntry = NULL;
        
        // Ensure a vector exists
        if (!allVarsByPrettyName.find(pretty_name,			      
                                      varsByPrettyEntry)) {
            varsByPrettyEntry = new pdvector<int_variable *>;
            allVarsByPrettyName[pretty_name] = varsByPrettyEntry;
        }
        
        (*varsByPrettyEntry).push_back(var);
    }
    
    // And multiple symtab names...
    for (unsigned symtab_iter = 0; 
         symtab_iter < var->symTabNameVector().size();
         symtab_iter++) {
        pdstring symtab_name = var->symTabNameVector()[symtab_iter];
        pdvector<int_variable *> *varsBySymTabEntry = NULL;
        
        // Ensure a vector exists
        if (!allVarsByMangledName.find(symtab_name,			      
                                       varsBySymTabEntry)) {
            varsBySymTabEntry = new pdvector<int_variable *>;
            allVarsByMangledName[symtab_name] = varsBySymTabEntry;
        }
        
        (*varsBySymTabEntry).push_back(var);
    }  
    
    everyUniqueVariable[var->ivar()] = var;
    
    var->mod()->addVariable(var);
}  

/////////// Dinky functions

// This way we don't have to cross-include every header file in the
// world.

process *mapped_object::proc() const { return proc_; }

bool mapped_object::isSharedLib() const {
    return desc_.isSharedObject();
}

const pdstring mapped_object::debugString() const {
    pdstring debug;
    debug = fileName_ + ":" + pdstring(codeBase_) + "/" + pdstring(codeSize()); 
    return debug;
}

// This gets called once per image. Poke through to the internals;
// all we care about, amusingly, is symbol table information. 

void mapped_object::getInferiorHeaps(pdvector<foundHeapDesc> &foundHeaps) const {
    pdvector<Symbol> foundHeapSyms;

    

    parse_img()->findSymByPrefix("DYNINSTstaticHeap", foundHeapSyms);
    parse_img()->findSymByPrefix("_DYNINSTstaticHeap", foundHeapSyms);

    for (unsigned i = 0; i < foundHeapSyms.size(); i++) {
        foundHeapDesc foo;
        foo.name = foundHeapSyms[i].name();
        foo.addr = foundHeapSyms[i].addr();
        // foo.addr is now relative to the start of the heap; check the type of the symbol to 
        // determine whether it's a function (off codeBase_) or variable (off dataBase_)
        switch(foundHeapSyms[i].type()) {
        case Symbol::PDST_FUNCTION:
            foo.addr += codeBase_;
            foundHeaps.push_back(foo);
            break;
        case Symbol::PDST_OBJECT:
            foo.addr += dataBase_;
            foundHeaps.push_back(foo);
            break;
        default:
            // We don't know what this is, and can't tell the base. Skip it (but warn)
            fprintf(stderr, "Warning: skipping inferior heap with type %d, %s@0x%lx\n", foundHeapSyms[i].type(), foo.name.c_str(), foo.addr);
            break;
        }
    }
    
    // AIX: we scavenge space. Do that here.

#if defined(arch_power)
    // ...

    // a.out: from the end of the loader to 0x20000000
    // Anything in 0x2....: skip
    // Anything in 0xd....: to the next page

    foundHeapDesc tmp;
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
        start = codeAbs() + codeSize();
        start += instruction::size() - (start % (Address)instruction::size());
        size = PAGESIZE - (start % PAGESIZE);
    }
    else if (codeAbs() > 0x20000000) {
        // ...
    }
    else if (codeAbs() > 0x10000000) {
        // We also have the loader; there is no information on where
        // it goes (ARGH) so we pad the end of the code segment to
        // try and avoid it.
        start = codeAbs() + codeSize() + image_->getObject().loader_len();
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
        
        tmp.name = pdstring(name_scratch);
        tmp.addr = start;
        
        foundHeaps.push_back(tmp);
    }
#endif
}
    

void *mapped_object::getPtrToInstruction(Address addr) const {
    if (addr < codeAbs()) return NULL;
    if (addr >= (codeAbs() + codeSize())) return NULL;

    // Only subtract off the codeBase, not the codeBase plus
    // codeOffset -- the image class includes the codeOffset.
    Address offset = addr - codeBase();
    return image_->getPtrToInstruction(offset);
}

void *mapped_object::getPtrToData(Address addr) const {
    assert(addr >= dataAbs());
    assert(addr < (dataAbs() + dataSize()));

    // Don't go from the code base... the image adds back in the code
    // offset.
    Address offset = addr - dataBase();
    return image_->getPtrToData(offset);
}

bool mapped_object::getSymbolInfo(const pdstring &n, Symbol &info) {
    if (image_) {
        if (!image_->symbol_info(n, info)) {
            // Leading underscore...
            pdstring n1 = pdstring("_") + n;
            if (!image_->symbol_info(n1, info))
                return false;
        }

        // Shift to specialize; we check whether code or data, and
        // add the appropriate offset. On most platforms these are the
        // same, but may be different (cf. AIX)

        // Check symbol type.
        if (info.type() == Symbol::PDST_OBJECT) {
            info.setAddr(info.addr() + dataBase_);
        }
        else {
            info.setAddr(info.addr() + codeBase_);
        }

        return true;
    }
    assert(0);
    return false;
}

mapped_module *mapped_object::getOrCreateForkedModule(mapped_module *parMod) {
    // Okay. We're forking, and this is the child mapped_object.
    // And now, given a parent module, we need to find the right one
    // in our little baby modules.
    
    // Since we've already copied modules, we can just do a name lookup.
    mapped_module *childModule = findModule(parMod->fileName(), false);
    assert(childModule);
    return childModule;

}




