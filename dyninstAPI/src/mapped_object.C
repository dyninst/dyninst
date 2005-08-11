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

// $Id: mapped_object.C,v 1.4 2005/08/11 21:20:20 bernat Exp $

#include "dyninstAPI/src/mapped_object.h"
#include "dyninstAPI/src/symtab.h"
#include "common/h/String.h"
#include "dyninstAPI/src/showerror.h"
#include "process.h"

#define FS_FIELD_SEPERATOR '/'

mapped_object::mapped_object(fileDescriptor fileDesc,
                             image *img,
			     process *proc):
    desc_(fileDesc),
    fullName_(fileDesc.file()), 
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
    
    const pdvector<image_func *> &exportedFuncs = image_->getExportedFunctions();
    //fprintf(stderr, "%d exported functions\n", exportedFuncs.size());
    for (unsigned fi = 0; fi < exportedFuncs.size(); fi++) {
        mapped_module *mod = findModule(exportedFuncs[fi]->pdmod());
        
        int_function *newFunc = new int_function(exportedFuncs[fi],
                                                 codeBase_,
                                                 mod);
        everyUniqueFunction.push_back(newFunc);
        addFunctionToIndices(newFunc);

        mod->addFunction(newFunc);
    }
    const pdvector<image_variable *> &exportedVars = image_->getExportedVariables();
    //fprintf(stderr, "%d exported variables\n", exportedVars.size());
    for (unsigned vi = 0; vi < exportedVars.size(); vi++) {
        mapped_module *mod = findModule(exportedVars[vi]->pdmod());
        int_variable *newVar = new int_variable(exportedVars[vi],
                                                dataBase_,
                                                mod);
        everyUniqueVariable.push_back(newVar);
        addVarToIndices(newVar);

        mod->addVariable(newVar);
    }
}

mapped_object *mapped_object::createMappedObject(fileDescriptor desc,
                                                 process *p) {
                                                 
    if (!p) return NULL;
    
    image *img = image::parseImage(desc);
    if (!img) return NULL;
    // Adds exported functions and variables..
    mapped_object *obj = new mapped_object(desc, img, p);

    // TODO: delay this
    obj->analyze(); 
    return obj;
}

mapped_object::mapped_object(const mapped_object *s, process *child) :
  codeRange(),
  desc_(s->desc_),
  fullName_(s->fullName_),
  fileName_(s->fileName_),
  codeBase_(s->codeBase_),
  dataBase_(s->dataBase_),
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

    for (unsigned i = 0; i < s->everyUniqueFunction.size(); i++) {
        int_function *parFunc = s->everyUniqueFunction[i];
        assert(parFunc->mod());
        mapped_module *mod = getOrCreateForkedModule(parFunc->mod());
        int_function *newFunc = new int_function(parFunc,
                                                 mod);
        everyUniqueFunction.push_back(newFunc);
        addFunctionToIndices(newFunc);
        mod->addFunction(newFunc);
    }

    for (unsigned j = 0; j < s->everyUniqueVariable.size(); j++) {
        int_variable *parVar = s->everyUniqueVariable[j];
        assert(parVar->mod());
        mapped_module * mod = getOrCreateForkedModule(parVar->mod());

        int_variable *newVar = new int_variable(parVar, mod);
        everyUniqueVariable.push_back(newVar);
        addVarToIndices(newVar);
        mod->addVariable(newVar);
    }
    image_ = s->image_->clone();
}

mapped_object::~mapped_object() {
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
      mapped_module *mod = findModule(unmappedFuncs[fi]->pdmod());
      assert(mod);
      // Note: we don't want to use codeBase_ because the function may be offset 
      int_function *newFunc = new int_function(unmappedFuncs[fi],
                                               codeBase_,
                                               mod);
      everyUniqueFunction.push_back(newFunc);
      addFunctionToIndices(newFunc);
      mod->addFunction(newFunc);
  }
  
  // Remember: variables don't.
  pdvector<image_variable *> unmappedVars = image_->getCreatedVariables();
  for (unsigned vi = 0; vi < unmappedVars.size(); vi++) {
      mapped_module *mod = findModule(unmappedVars[vi]->pdmod());
      assert(mod);
      int_variable *newVar = new int_variable(unmappedVars[vi],
                                              dataBase_,
                                              mod);
      everyUniqueVariable.push_back(newVar);
      addVarToIndices(newVar);
      mod->addVariable(newVar);
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
	    } }
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


mapped_module *mapped_object::findModule(const pdmodule *pdmod) {
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
  if (allFunctionsByPrettyName.defines(funcname)) {
      return allFunctionsByPrettyName[funcname];
  }
  else
      return NULL;
} 

const pdvector <int_function *> *mapped_object::findFuncVectorByMangled(const pdstring &funcname)
{
  if (funcname.c_str() == 0) return NULL;
  if (allFunctionsByMangledName.defines(funcname)) {
      return allFunctionsByMangledName[funcname];
  }
  else
      return NULL;
} 


const pdvector<int_variable *> *mapped_object::findVarVectorByPretty(const pdstring &varname)
{
  if (varname.c_str() == 0) return NULL;
  if (allVarsByPrettyName.defines(varname))
      return allVarsByPrettyName[varname];
  else
      return NULL;
} 

const pdvector <int_variable *> *mapped_object::findVarVectorByMangled(const pdstring &varname)
{
  if (varname.c_str() == 0) return NULL;
  if (allVarsByMangledName.defines(varname))
      return allVarsByMangledName[varname];
  else
      return NULL;
} 

int_function *mapped_object::findFuncByAddr(const Address &addr)  {
    codeRange *range;
    if (!funcsByAddr_.find(addr, range)) {
        return NULL;
    }
    return range->is_function();
}

int_function *mapped_module::findFuncByAddr(const Address &addr)  {
    int_function *obj_func = obj()->findFuncByAddr(addr);
    if (!obj_func) return NULL;
    if (obj_func->mod() == this)
        return obj_func;
    else
        return NULL;
}

const pdvector<mapped_module *> &mapped_object::getModules() {
    return everyModule;
}

const pdvector<int_function *> &mapped_object::getAllFunctions() {
    return everyUniqueFunction;
}

const pdvector<int_function *> &mapped_module::getAllFunctions() {
    return everyUniqueFunction;
}

const pdvector<int_variable *> &mapped_object::getAllVariables() {
    return everyUniqueVariable;
}

void mapped_module::addFunction(int_function *func) {
    // Just the everything vector... the by-name lists are
    // kept in the mapped_object and filtered if we do a lookup.
    everyUniqueFunction.push_back(func);
}

void mapped_module::addVariable(int_variable *var) {
    everyUniqueVariable.push_back(var);
}

// Enter a function in all the appropriate tables
void mapped_object::addFunctionToIndices(int_function *func) {
  if (!func) return;
  
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

  // And addresses...
  funcsByAddr_.insert(func);
}  

// Enter a function in all the appropriate tables
void mapped_object::addVarToIndices(int_variable *var) {
  if (!var) return;
  
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
}  

/////////// Dinky functions This way we don't have to cross-include
//every header file in the world

process *mapped_object::proc() const { return proc_; }

bool mapped_object::isSharedLib() const {
    return desc_.isSharedObject();
}

const pdstring &mapped_module::fileName() const { 
    return pmod()->fileName(); 
}
const pdstring &mapped_module::fullName() const { 
    return pmod()->fullName(); 
}

mapped_object *mapped_module::obj() const { 
    return obj_;
}

bool mapped_module::isNativeCompiler() const {
    // This should probably be per-module info at some point; some
    // .o's might be compiled native, and others not.
    return pmod()->exec()->isNativeCompiler();
}

supportedLanguages mapped_module::language() const { 
    return pmod()->language(); 
}

bool mapped_module::findFuncVectorByMangled(const pdstring &funcname,
                                            pdvector<int_function *> &funcs)
{
    // For efficiency sake, we grab the image vector and strip out the
    // functions we want.
    // We could also keep them all in modules and ditch the image-wide search; 
    // the problem is that BPatch goes by module and internal goes by image. 
    unsigned orig_size = funcs.size();

    const pdvector<int_function *> *obj_funcs = obj()->findFuncVectorByMangled(funcname);
    if (!obj_funcs) {
        return false;
    }
    for (unsigned i = 0; i < obj_funcs->size(); i++) {
        if ((*obj_funcs)[i]->mod() == this)
            funcs.push_back((*obj_funcs)[i]);
    }
    return funcs.size() > orig_size;
}

bool mapped_module::findFuncVectorByPretty(const pdstring &funcname,
                                           pdvector<int_function *> &funcs)
{
    // For efficiency sake, we grab the image vector and strip out the
    // functions we want.
    // We could also keep them all in modules and ditch the image-wide search; 
    // the problem is that BPatch goes by module and internal goes by image. 
    unsigned orig_size = funcs.size();

    const pdvector<int_function *> *obj_funcs = obj()->findFuncVectorByPretty(funcname);
    if (!obj_funcs) return false;

    for (unsigned i = 0; i < obj_funcs->size(); i++) {
        if ((*obj_funcs)[i]->mod() == this)
            funcs.push_back((*obj_funcs)[i]);
    }
    return funcs.size() > orig_size;
}


const pdmodule *mapped_module::pmod() const { return internal_mod_;}

const pdstring mapped_object::debugString() const {
    pdstring debug;
    debug = fileName_ + ":" + pdstring(codeBase_) + "/" + pdstring(codeSize()); 
    return debug;
}

void mapped_module::dumpMangled(pdstring prefix) const {
    // No reason to have this process specific... it just dumps
    // function names.
    pmod()->dumpMangled(prefix);
}

void mapped_object::getInferiorHeaps(pdvector<foundHeapDesc> &foundHeaps) const {
    
    for (unsigned vi = 0; vi < everyUniqueVariable.size(); vi++) {
        int_variable *v = everyUniqueVariable[vi];
        for (unsigned vis = 0; vis < v->symTabNameVector().size(); vis++) {
            if (v->symTabNameVector()[vis].prefixed_by("DYNINSTstaticHeap") ||
                v->symTabNameVector()[vis].prefixed_by("_DYNINSTstaticHeap")) {
                foundHeapDesc tmp;
                tmp.name = v->symTabNameVector()[vis];
                tmp.addr = v->getAddress();
                foundHeaps.push_back(tmp);
            }
        }
    }
    // Sometimes these masquerade as functions
    for (unsigned fi = 0; fi < everyUniqueFunction.size(); fi++) {
        int_function *f = everyUniqueFunction[fi];
        for (unsigned fis = 0; fis < f->symTabNameVector().size(); fis++) {
            if (f->symTabNameVector()[fis].prefixed_by("DYNINSTstaticHeap") ||
                f->symTabNameVector()[fis].prefixed_by("_DYNINSTstaticHeap")) {
                foundHeapDesc tmp;
                tmp.name = f->symTabNameVector()[fis];
                tmp.addr = f->getAddress();
                foundHeaps.push_back(tmp);
            }
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
    

void *mapped_object::getPtrToOrigInstruction(Address addr) const {
    if (addr < codeAbs()) return NULL;
    if (addr >= (codeAbs() + codeSize())) return NULL;

    // Only subtract off the codeBase, not the codeBase plus
    // codeOffset -- the image class includes the codeOffset.
    Address offset = addr - codeBase();
    return image_->getPtrToOrigInstruction(offset);
}

void *mapped_object::getPtrToData(Address addr) const {
    assert(addr >= dataAbs());
    assert(addr < (dataAbs() + dataSize()));

    // Don't go from the code base... the image adds back in the code
    // offset.
    Address offset = addr - dataBase();
    return image_->getPtrToData(offset);
}

mapped_module::mapped_module(mapped_object *obj,
                             const pdmodule *pdmod) :
    internal_mod_(pdmod),
    obj_(obj),
    lineInfoValid_(false)
{
}

mapped_module *mapped_module::createMappedModule(mapped_object *obj,
                                                 const pdmodule *pdmod) {
    assert(obj);
    assert(pdmod);
    assert(pdmod->exec() == obj->parse_img());
    mapped_module *mod = new mapped_module(obj, pdmod);
    // Do things?

    return mod;
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

pdstring *mapped_module::processDirectories(pdstring *fn) const {
    // This is black magic... assume Todd (I think) knew what
    // he was doing....
	if(!fn)
		return NULL;

	if(!strstr(fn->c_str(),"/./") &&
	   !strstr(fn->c_str(),"/../"))
		return fn;

	pdstring* ret = NULL;
	char suffix[10] = "";
	char prefix[10] = "";
	char* pPath = new char[strlen(fn->c_str())+1];

	strcpy(pPath,fn->c_str());

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

	ret = new pdstring;
	*ret += prefix;
	for(int i=0;i<count;i++){
		*ret += pPathLocs[i];
		if(i != (count-1))
			*ret += "/";
	}
	*ret += suffix;

	delete[] pPath;
	delete fn;
	return ret;
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

process *mapped_module::proc() const { return obj()->proc(); }


// Line information is processed for all modules in an image at once; so we have to do
// them at the same time. For now we're process-specific processing them. This code
// can move (back) into symtab.C if there is a mechanism for a process-specific
// line information structure.


// Parses symtab for file and line info. Should not be called before
// parseTypes. The ptr to lineInformation should be NULL before this is called.
#if !defined(rs6000_ibm_aix4_1) \
 && !defined(mips_sgi_irix6_4) \
 && !defined(alpha_dec_osf4_0) \
 && !defined(i386_unknown_nt4_0) \
 && !defined( USES_DWARF_DEBUG )
 
/* Parse everything in the file on disk, and cache that we've done so,
   because our modules may not bear any relation to the name source files. */
void mapped_module::parseFileLineInfo() {
	static dictionary_hash< pdstring, bool > haveParsedFileMap( pdstring::hash );
	
	image * fileOnDisk = obj()->parse_img();
	assert( fileOnDisk != NULL );
	const Object & elfObject = fileOnDisk->getObject();

	const char * fileName = elfObject.getFileName();
	if( haveParsedFileMap.defines( fileName ) ) { return; } 

	/* We haven't parsed this file already, so iterate over its stab entries. */
	stab_entry * stabEntry = elfObject.get_stab_info();
	assert( stabEntry != NULL );
	const char * nextStabString = stabEntry->getStringBase();
	
	const char * currentSourceFile = NULL;
	mapped_module * currentModule = NULL;
	Address currentFunctionBase = 0;
	unsigned int previousLineNo = 0;
	Address previousLineAddress = 0;
	bool isPreviousValid = false;
	
	Address baseAddress = obj()->codeBase();
	
	for( unsigned int i = 0; i < stabEntry->count(); i++ ) {
		switch( stabEntry->type( i ) ) {
		
			case N_UNDF: /* start of an object file */ {
				if( isPreviousValid ) {
					/* DEBUG */ fprintf( stderr, "%s[%d]: unterminated N_SLINE at start of object file.  Line number information will be lost.\n", __FILE__, __LINE__ );
					}
			
				stabEntry->setStringBase( nextStabString );
				nextStabString = stabEntry->getStringBase() + stabEntry->val( i );
				
				currentSourceFile = NULL;
				isPreviousValid = false;
				} break;
				
			case N_SO: /* compilation source or file name */ {
				if( isPreviousValid ) {
					/* Add the previous N_SLINE. */
					Address currentLineAddress = stabEntry->val( i );
					
					// /* DEBUG */ fprintf( stderr, "%s[%d]: adding %s:%d [0x%lx, 0x%lx) to module %s.\n", __FILE__, __LINE__, currentSourceFile, previousLineNo, previousLineAddress, currentLineAddress, currentModule->fileName().c_str() );
					currentModule->lineInfo_.addLine( currentSourceFile, previousLineNo, previousLineAddress, currentLineAddress  );
					}
				
				const char * sourceFile = stabEntry->name( i );
				currentSourceFile = strrchr( sourceFile, '/' );
				if( currentSourceFile == NULL ) { currentSourceFile = sourceFile; }
				else { ++currentSourceFile; }
				// /* DEBUG */ fprintf( stderr, "%s[%d]: using file name '%s'\n", __FILE__, __LINE__, currentSourceFile );
				
				isPreviousValid = false;
				} break;
				
			case N_SOL: /* file name (possibly an include file) */ {
				if( isPreviousValid ) {
					/* Add the previous N_SLINE. */
					Address currentLineAddress = stabEntry->val( i );
					// /* DEBUG */ fprintf( stderr, "%s[%d]: adding %s:%d [0x%lx, 0x%lx) to module %s.\n", __FILE__, __LINE__, currentSourceFile, previousLineNo, previousLineAddress, currentLineAddress, currentModule->fileName().c_str() );
					currentModule->lineInfo_.addLine( currentSourceFile, previousLineNo, previousLineAddress, currentLineAddress  );
					}
					
				const char * sourceFile = stabEntry->name( i );
				currentSourceFile = strrchr( sourceFile, '/' );
				if( currentSourceFile == NULL ) { currentSourceFile = sourceFile; }
				else { ++currentSourceFile; }
				// /* DEBUG */ fprintf( stderr, "%s[%d]: using file name '%s'\n", __FILE__, __LINE__, currentSourceFile );
				
				isPreviousValid = false;
				} break;
				
			case N_FUN: /* a function */ {
				if( * stabEntry->name( i ) == 0 ) {
					/* An end-of-function marker.  The value is the size of the function. */
					if( isPreviousValid ) {
						/* Add the previous N_SLINE. */
						Address currentLineAddress = currentFunctionBase + stabEntry->val( i );
						// /* DEBUG */ fprintf( stderr, "%s[%d]: adding %s:%d [0x%lx, 0x%lx) in module %s.\n", __FILE__, __LINE__, currentSourceFile, previousLineNo, previousLineAddress, currentLineAddress, currentModule->fileName().c_str() );
						currentModule->lineInfo_.addLine( currentSourceFile, previousLineNo, previousLineAddress, currentLineAddress  );
						}
					
					/* We've added the previous N_SLINE and don't currently have a module. */
					isPreviousValid = false;
					currentModule = NULL;
					break;
					} /* end if the N_FUN is an end-of-function-marker. */
							
				if( isPreviousValid ) {
					Address currentLineAddress = stabEntry->val( i );
					// /* DEBUG */ fprintf( stderr, "%s[%d]: adding %s:%d [0x%lx, 0x%lx) in module %s.\n", __FILE__, __LINE__, currentSourceFile, previousLineNo, previousLineAddress, currentLineAddress, currentModule->fileName().c_str() );
					currentModule->lineInfo_.addLine( currentSourceFile, previousLineNo, previousLineAddress, currentLineAddress  );
					}
					
				currentFunctionBase = stabEntry->val( i );
				currentFunctionBase += baseAddress;
				
				int_function * currentFunction = obj()->findFuncByAddr( currentFunctionBase );
				if( currentFunction == NULL ) {
					/* DEBUG */ fprintf( stderr, "%s[%d]: failed to find function containing address 0x%lx; line number information will be lost.\n", __FILE__, __LINE__, currentFunctionBase );
					currentModule = NULL;
					}
				else {
					currentModule = currentFunction->mod();
					assert( currentModule != NULL );
					}
											
				isPreviousValid = false;
				} break;
				
			case N_SLINE: {
				if( currentModule ) {
					Address currentLineAddress = currentFunctionBase + stabEntry->val( i );
					unsigned int currentLineNo = stabEntry->desc( i );
					
					if( isPreviousValid ) {
						// /* DEBUG */ fprintf( stderr, "%s[%d]: adding %s:%d [0x%lx, 0x%lx) in module %s.\n", __FILE__, __LINE__, currentSourceFile, previousLineNo, previousLineAddress, currentLineAddress, currentModule->fileName().c_str() );
						currentModule->lineInfo_.addLine( currentSourceFile, previousLineNo, previousLineAddress, currentLineAddress  );
						}
						
					previousLineAddress = currentLineAddress;
					previousLineNo = currentLineNo;
					isPreviousValid = true;
					} /* end if we've a module to which to add line information */
				} break;
						
			} /* end switch on the ith stab entry's type */
		} /* end iteration over stab entries. */
	
	haveParsedFileMap[ fileName ] = true;
	} /* end parseFileLineInfo() */
	
#endif 



#if defined(rs6000_ibm_aix4_1)

#include <linenum.h>
#include <syms.h>
#include <set>

/* FIXME: hack. */
Address trueBaseAddress = 0;

void mapped_module::parseFileLineInfo() {
	static std::set< image * > haveParsedFileMap;
	
        image * fileOnDisk = obj()->parse_img();
	assert( fileOnDisk != NULL );
	if( haveParsedFileMap.count( fileOnDisk ) != 0 ) { return; }
	// /* DEBUG */ fprintf( stderr, "%s[%d]: Considering image at 0x%lx\n", __FILE__, __LINE__, fileOnDisk );

	/* FIXME: hack.  Should be argument to parseLineInformation(), which should in turn be merged
	   back into here so it can tell how far to extend the range of the last line information point. */
	Address baseAddress = obj()->codeBase();

	trueBaseAddress = baseAddress;

	const Object & xcoffObject = fileOnDisk->getObject();

	/* We haven't parsed this file already, so iterate over its stab entries. */
	char * stabstr = NULL;
	int nstabs = 0;
	SYMENT * syms = 0;
	char * stringpool = NULL;
	xcoffObject.get_stab_info( stabstr, nstabs, syms, stringpool );

	int nlines = 0;
	char * lines = NULL;
	unsigned long linesfdptr;
	xcoffObject.get_line_info( nlines, lines, linesfdptr );

	/* I'm not sure why the original code thought it should copy (short) names (through here). */
	char temporaryName[256];
	char * funcName = NULL;
	char * currentSourceFile = NULL;
        char *moduleName = NULL;

	/* Iterate over STAB entries. */
	for( int i = 0; i < nstabs; i++ ) {
		/* sizeof( SYMENT ) is 20, not 18, as it should be. */
		SYMENT * sym = (SYMENT *)( (unsigned)syms + (i * SYMESZ) );

                /* Get the name (period) */
                if (!sym->n_zeroes) {
                    moduleName = &stringpool[sym->n_offset];
                } else {
                    memset(temporaryName, 0, 9);
                    strncpy(temporaryName, sym->n_name, 8);
                    moduleName = temporaryName;
                }

	
		/* Extract the current source file from the C_FILE entries. */
		if( sym->n_sclass == C_FILE ) {
                    if (!strcmp(moduleName, ".file")) {
                        // The actual name is in an aux record.

                        int j;
                        /* has aux record with additional information. */
                        for (j=1; j <= sym->n_numaux; j++) {
                            union auxent *aux = (union auxent *) ((char *) sym + j * SYMESZ);
                            if (aux->x_file._x.x_ftype == XFT_FN) {
                                // this aux record contains the file name.
                                if (!aux->x_file._x.x_zeroes) {
                                    moduleName = &stringpool[aux->x_file._x.x_offset];
                                } else {
                                    // x_fname is 14 bytes
                                    memset(temporaryName, 0, 15);
                                    strncpy(temporaryName, aux->x_file.x_fname, 14);
                                    moduleName = temporaryName;
                                }
                            }
                        }
                    }
			
                    currentSourceFile = strrchr( moduleName, '/' );
                    if( currentSourceFile == NULL ) { currentSourceFile = moduleName; }
                    else { ++currentSourceFile; }
                    
                    /* We're done with this entry. */
                    continue;
                } /* end if C_FILE */
	
		/* This apparently compensates for a bug in the naming of certain entries. */
		char * nmPtr = NULL;
		if( 	! sym->n_zeroes && (
				( sym->n_sclass & DBXMASK ) ||
				( sym->n_sclass == C_BINCL ) ||
				( sym->n_sclass == C_EINCL )
				) ) {
			if( sym->n_offset < 3 ) {
				if( sym->n_offset == 2 && stabstr[ 0 ] ) {
					nmPtr = & stabstr[ 0 ];
					} else {
					nmPtr = & stabstr[ sym->n_offset ];
					}
				} else if( ! stabstr[ sym->n_offset - 3 ] ) {
				nmPtr = & stabstr[ sym->n_offset ];
				} else {
				/* has off by two error */
				nmPtr = & stabstr[ sym->n_offset - 2 ];
				} 
			} else {
			// names 8 or less chars on inline, not in stabstr
			memset( temporaryName, 0, 9 );
			strncpy( temporaryName, sym->n_name, 8 );
			nmPtr = temporaryName;
			} /* end bug compensation */

		/* Now that we've compensated for buggy naming, actually
		   parse the line information. */
		if(	( sym->n_sclass == C_BINCL ) 
			|| ( sym->n_sclass == C_EINCL )
			|| ( sym->n_sclass == C_FUN ) ) {
			if( funcName ) {
				free( funcName );
				funcName = NULL;
				}
			funcName = strdup( nmPtr );

			pdstring pdCSF( currentSourceFile );
			parseLineInformation( proc(), & pdCSF, funcName, (SYMENT *)sym, linesfdptr, lines, nlines );
			} /* end if we're actually parsing line information */
		} /* end iteration over STAB entries. */

	if( funcName != NULL ) { 
		free( funcName );
		}		
	haveParsedFileMap.insert( fileOnDisk );
	} /* end parseFileLineInfo() */

void mapped_module::parseLineInformation(process * proc,
                                         pdstring * currentSourceFile,
                                         char * symbolName,
                                         SYMENT * sym,
                                         Address linesfdptr,
                                         char * lines,
                                         int nlines ) {
    union auxent * aux;
    pdvector<IncludeFileInfo> includeFiles;
    
    /* if it is beginning of include files then update the data structure
       that keeps the beginning of the include files. If the include files contain
       information about the functions and lines we have to keep it */
    if( sym->n_sclass == C_BINCL ) {
        includeFiles.push_back( IncludeFileInfo( (sym->n_value - linesfdptr)/LINESZ, symbolName ) );
    }
    /* similiarly if the include file contains function codes and line information
       we have to keep the last line information entry for this include file */
    else if( sym->n_sclass == C_EINCL ) {
        if( includeFiles.size() > 0 ) {
            includeFiles[includeFiles.size()-1].end = (sym->n_value-linesfdptr)/LINESZ;
        }
    }
    /* if the enrty is for a function than we have to collect all info
       about lines of the function */
    else if( sym->n_sclass == C_FUN ) {
        /* I have no idea what the old code did, except not work very well.
           Somebody who understands XCOFF should look at this. */
        int initialLine = 0;
        int initialLineIndex = 0;
        Address funcStartAddress = 0;
        
        for( int j = -1; ; --j ) {
            SYMENT * extSym = (SYMENT *)( ((Address)sym) + (j * SYMESZ) );
            if( extSym->n_sclass == C_EXT ) {
                aux = (union auxent *)( ((Address)extSym) + SYMESZ );
#ifndef __64BIT__
                initialLineIndex = ( aux->x_sym.x_fcnary.x_fcn.x_lnnoptr - linesfdptr )/LINESZ;
#endif
                funcStartAddress = extSym->n_value;
                break;
            } /* end if C_EXT found */
        } /* end search for C_EXT */
        
        /* access the line information now using the C_FCN entry*/
        SYMENT * bfSym = (SYMENT *)( ((Address)sym) + SYMESZ );
        if( bfSym->n_sclass != C_FCN ) {
            bperr("unable to process line info for %s\n", symbolName);
            return;
        }
        
        aux = (union auxent *)( ((Address)bfSym) + SYMESZ );
        initialLine = aux->x_sym.x_misc.x_lnsz.x_lnno;
        
        pdstring whichFile = *currentSourceFile;
        for( unsigned int j = 0; j < includeFiles.size(); j++ ) {
            if(	( includeFiles[j].begin <= (unsigned)initialLineIndex )
            	&& ( includeFiles[j].end >= (unsigned)initialLineIndex ) ) {
                whichFile = includeFiles[j].name;
                break;
            }
        } /* end iteration of include files */
        
        char * canonicalSourceFile = strrchr( whichFile.c_str(), '/' );
        if( canonicalSourceFile == NULL ) { canonicalSourceFile = const_cast< char * >( whichFile.c_str() ); }
        else { ++canonicalSourceFile; }
        
        int_function * currentFunction = obj()->findFuncByAddr( funcStartAddress + trueBaseAddress );
        if( currentFunction == NULL ) {
            /* Some addresses point to gdb-inaccessible memory; others have symbols (gdb will disassemble them)
               but the contents look like garbage, and may be data with symbol names.  (Who knows why.) */
            // fprintf( stderr, "%s[%d]: failed to find function containing address 0x%lx; line number information will be lost.\n", __FILE__, __LINE__, funcStartAddress + trueBaseAddress );
            return;
        }
        mapped_module * currentModule = currentFunction->mod();
        assert( currentModule != NULL );
        LineInformation & currentLineInformation = currentModule->lineInfo_;
        
        unsigned int previousLineNo = 0;
        Address previousLineAddr = 0;
        bool isPreviousValid = false;
        
        /* Iterate over this entry's lines. */
        for( int j = initialLineIndex + 1; j < nlines; j++ ) {
            LINENO * lptr = (LINENO *)( lines + (j * LINESZ) );
            if( ! lptr->l_lnno ) { break; }
            unsigned int lineNo = lptr->l_lnno + initialLine - 1;
            Address lineAddr = lptr->l_addr.l_paddr + trueBaseAddress;
            
            if( isPreviousValid ) {
                // /* DEBUG */ fprintf( stderr, "%s[%d]: adding %s:%d [0x%lx, 0x%lx).\n", __FILE__, __LINE__, canonicalSourceFile, previousLineNo, previousLineAddr, lineAddr );
                currentLineInformation.addLine( canonicalSourceFile, previousLineNo, previousLineAddr, lineAddr );
            }
            
            previousLineNo = lineNo;
            previousLineAddr = lineAddr;
            isPreviousValid = true;
        } /* end iteration over line information */
        
        if( isPreviousValid ) {
            /* Add the instruction (always 4 bytes on power) pointed at by the last entry.  We'd like to add a
               bigger range, but it's not clear how.  (If the function has inlined code, we won't know about
               it until we see the next section, so claiming "until the end of the function" will give bogus results.) */
            // /* DEBUG */ fprintf( stderr, "%s[%d]: adding %s:%d [0x%lx, 0x%lx).\n", __FILE__, __LINE__, canonicalSourceFile, previousLineNo, previousLineAddr, previousLineAddr + 4 );
            currentLineInformation.addLine( canonicalSourceFile, previousLineNo, previousLineAddr, previousLineAddr + 4 );
        }
    } /* end if we found a C_FUN symbol */
} /* end parseLineInformation() */

#endif

/* mips-sgi-irix6.5 uses DWARF debug, but the rest of the code
   isn't set up to take advantage of this. */
#if defined(USES_DWARF_DEBUG) && !defined(mips_sgi_irix6_4)

#include "elf.h"
#include "libelf.h"
#include "dwarf.h"
#include "libdwarf.h"  

#include "LineInformation.h"

extern void pd_dwarf_handler( Dwarf_Error, Dwarf_Ptr );
void mapped_module::parseFileLineInfo() {
	static dictionary_hash< pdstring, bool > haveParsedFileMap( pdstring::hash );
	
	/* Determine if we've parsed this file already. */
	image * moduleImage = obj()->parse_img();
	assert( moduleImage != NULL );
	const Object & moduleObject = moduleImage->getObject();	
	const char * fileName = moduleObject.getFileName();

    if( haveParsedFileMap.defines( fileName ) ) { return; }
	
	/* We have not parsed this file already, so wind up libdwarf. */
	int fd = open( fileName, O_RDONLY );
	assert( fd != -1 );
	
	Dwarf_Debug dbg;
	int status = dwarf_init(	fd, DW_DLC_READ, & pd_dwarf_handler,
								moduleObject.getErrFunc(),
								& dbg, NULL );
	assert( status != DW_DLV_ERROR );
	if( status == DW_DLV_NO_ENTRY ) { close( fd ); return; }
	
	/* Itereate over the CU headers. */
	Dwarf_Unsigned header;
	while( dwarf_next_cu_header( dbg, NULL, NULL, NULL, NULL, & header, NULL ) == DW_DLV_OK ) {
		/* Acquire the CU DIE. */
		Dwarf_Die cuDIE;
		status = dwarf_siblingof( dbg, NULL, & cuDIE, NULL);
		assert( status == DW_DLV_OK );

		/* Acquire this CU's source lines. */
		Dwarf_Line * lineBuffer;
		Dwarf_Signed lineCount;
		status = dwarf_srclines( cuDIE, & lineBuffer, & lineCount, NULL );
		assert( status != DW_DLV_ERROR );
		
		/* It's OK for a CU not to have line information. */
		if( status != DW_DLV_OK ) {
			/* Free this CU's DIE. */
			dwarf_dealloc( dbg, cuDIE, DW_DLA_DIE );
			continue;
			}
		assert( status == DW_DLV_OK );
		
		/* The 'lines' returned are actually interval markers; the code
		   generated from lineNo runs from lineAddr up to but not including
		   the lineAddr of the next line. */			   
		bool isPreviousValid = false;
		Dwarf_Unsigned previousLineNo = 0;
		Dwarf_Addr previousLineAddr = 0x0;
		char * previousLineSource = NULL;
		
		Address baseAddr = obj()->codeBase();
		
		/* Iterate over this CU's source lines. */
		for( int i = 0; i < lineCount; i++ ) {
			/* Acquire the line number, address, source, and end of sequence flag. */
			Dwarf_Unsigned lineNo;
			status = dwarf_lineno( lineBuffer[i], & lineNo, NULL );
			assert( status == DW_DLV_OK );			
				
			Dwarf_Addr lineAddr;
			status = dwarf_lineaddr( lineBuffer[i], & lineAddr, NULL );
			assert( status == DW_DLV_OK );
			lineAddr += baseAddr;
			
			char * lineSource;
			status = dwarf_linesrc( lineBuffer[i], & lineSource, NULL );
			assert( status == DW_DLV_OK );
						
			Dwarf_Bool isEndOfSequence;
			status = dwarf_lineendsequence( lineBuffer[i], & isEndOfSequence, NULL );
			assert( status == DW_DLV_OK );
			
			if( isPreviousValid ) {
				/* If we're talking about the same (source file, line number) tuple,
				   and it isn't the end of the sequence, we can coalesce the range.
				   (The end of sequence marker marks discontinuities in the ranges.) */
				if( lineNo == previousLineNo && strcmp( lineSource, previousLineSource ) == 0 && ! isEndOfSequence ) {
					/* Don't update the prev* values; just keep going until we hit the end of a sequence or
					   a new sourcefile. */
					continue;
					} /* end if we can coalesce this range */
                                
				/* Determine into which mapped_module this line information should be inserted. */
				int_function * currentFunction = obj()->findFuncByAddr( previousLineAddr );
				if( currentFunction == NULL ) {
					// /* DEBUG */ fprintf( stderr, "%s[%d]: failed to find function containing address 0x%lx; line number information will be lost.\n", __FILE__, __LINE__, lineAddr );
					}
				else {
					mapped_module * currentModule = currentFunction->mod();
					assert( currentModule != NULL );
					
					char * canonicalLineSource = strrchr( previousLineSource, '/' );
					if( canonicalLineSource == NULL ) { canonicalLineSource = previousLineSource; }
					else { ++canonicalLineSource; }
					
					/* The line 'canonicalLineSource:previousLineNo' has an address range of [previousLineAddr, lineAddr). */
					currentModule->lineInfo_.addLine( canonicalLineSource, previousLineNo, previousLineAddr, lineAddr );
				
					// /* DEBUG */ fprintf( stderr, "%s[%d]: inserted address range [0x%lx, 0x%lx) for source '%s:%u' into module '%s'.\n", __FILE__, __LINE__, previousLineAddr, lineAddr, canonicalLineSource, previousLineNo, currentModule->fileName().c_str() );
					} /* end if we found the function by its address */
				} /* end if the previous* variables are valid */
				
			/* If the current line ends the sequence, invalidate previous; otherwise, update. */
			if( isEndOfSequence ) {
				dwarf_dealloc( dbg, lineSource, DW_DLA_STRING );
				
				isPreviousValid = false;
				}
			else {
				if( isPreviousValid ) { dwarf_dealloc( dbg, previousLineSource, DW_DLA_STRING ); }

				previousLineNo = lineNo;
				previousLineSource = lineSource;
				previousLineAddr = lineAddr;
							
				isPreviousValid = true;
				} /* end if line was not the end of a sequence */
			} /* end iteration over source line entries. */
		
		/* Free this CU's source lines. */
		for( int i = 0; i < lineCount; i++ ) {
			dwarf_dealloc( dbg, lineBuffer[i], DW_DLA_LINE );
			}
		dwarf_dealloc( dbg, lineBuffer, DW_DLA_LIST );
		
		/* Free this CU's DIE. */
		dwarf_dealloc( dbg, cuDIE, DW_DLA_DIE );
		} /* end CU header iteration */

	/* Wind down libdwarf. */
	Elf * dwarfElf;
	status = dwarf_get_elf( dbg, & dwarfElf, NULL );
	assert( status == DW_DLV_OK );
	                                              
	status = dwarf_finish( dbg, NULL );
	assert( status == DW_DLV_OK );
	close( fd );
	
	/* Note that we've parsed this file. */
	haveParsedFileMap[ fileName ] = true;
	} /* end parseFileLineInfo() */
#elif defined(arch_alpha)
void mapped_module::parseFileLineInfo() {
    // We don't do anything here

}
#elif defined(os_windows)
void mapped_module::parseFileLineInfo() {
    // Or here, I believe
}
#endif


LineInformation &mapped_module::getLineInformation() {
    if (!lineInformation()) {
        parseFileLineInfo();
        lineInfoValid_ = true;
    }
    return lineInfo_;
}

