/*
 * Copyright (c) 1996 Barton P. Miller
 * 
 * We provide the Paradyn Parallel Performance Tools (below
 * described as Paradyn") on an AS IS basis, and do not warrant its
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

// $Id: symtab.C,v 1.176 2003/08/05 21:49:23 hollings Exp $

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

// TODO - machine independent instruction functions
#include "dyninstAPI/src/symtab.h"
#include "dyninstAPI/src/arch.h"
#include "dyninstAPI/src/instPoint.h"
#include "dyninstAPI/src/Object.h"
#include <fstream>
#include "dyninstAPI/src/util.h"
#include "dyninstAPI/src/dyninstP.h"
#include "common/h/String.h"
#include "dyninstAPI/src/inst.h"
#include "common/h/Timer.h"
#include "dyninstAPI/src/showerror.h"
#include "common/h/debugOstream.h"
#include "common/h/pathName.h"          // extract_pathname_tail()
#include "dyninstAPI/src/process.h"   // process::getBaseAddress()

#ifndef BPATCH_LIBRARY
#include "paradynd/src/mdld.h"
#include "paradynd/src/main.h"
#include "paradynd/src/init.h"
#include "common/h/Dictionary.h"
#else
extern pdvector<sym_data> syms_to_find;
#endif

#include "LineInformation.h"

#if defined(TIMED_PARSE)
#include <sys/time.h>
#endif
// All debug_ostream vrbles are defined in process.C (for no particular reason)
extern unsigned enable_pd_sharedobj_debug;

extern bool parseCompilerType(Object *);

#if ENABLE_DEBUG_CERR == 1
#define sharedobj_cerr if (enable_pd_sharedobj_debug) cerr
#else
#define sharedobj_cerr if (0) cerr
#endif /* ENABLE_DEBUG_CERR == 1 */

pdvector<image*> image::allImages;

pdstring function_base::emptyString("");

// coming to dyninstAPI/src/symtab.hC
// needed in metric.C
bool function_base::match(function_base *fb)
{
  if (this == fb)
    return true;
  else
    return ((symTabName_ == fb->symTabName_) &&
	    (prettyName_ == fb->prettyName_) &&
	    (line_       == fb->line_) &&
	    (addr_       == fb->addr_) &&
	    (size_       == fb->size_));
}

/*
  Debuggering info for function_base....
 */
ostream & function_base::operator<<(ostream &s) const {
  
    unsigned i=0;
    s << "Mangled name(s): " << symTabName_[0];
    for(i = 1; i < symTabName_.size(); i++) {
        s << ", " << symTabName_[i];
    }

    s << "\nPretty name(s): " << prettyName_[0];
    for(i = 1; i < prettyName_.size(); i++) {
        s << ", " << prettyName_[i];
    }
      s << "\nline_ = " << line_ << " addr_ = "<< addr_ << " size_ = ";
      s << size_ << endl;
  
    return s;
}

ostream &operator<<(ostream &os, function_base &f) {
    return f.operator<<(os);
}

/*
 * function_base::getMangledName
 *
 * Copies the mangled name of the function into a buffer, up to a given maximum
 * length.  Returns a pointer to the beginning of the buffer that was
 * passed in.
 *
 * s            The buffer into which the name will be copied.
 * len          The size of the buffer.
 */
char *function_base::getMangledName(char *s, int len) const
{
    pdstring name = symTabName();
    strncpy(s, name.c_str(), len);

    return s;
}

/* imported from platform specific library list.  This is lists all
   library functions we are interested in instrumenting. */

pdmodule *image::newModule(const pdstring &name, const Address addr, supportedLanguages lang)
{
    pdmodule *ret = NULL;
    // modules can be defined several times in C++ due to templates and
    //   in-line member functions.
#ifndef BPATCH_LIBRARY
    if ((ret = findModule(name, TRUE))) {
      return(ret);
    }
#else
    // seperate out references to excluded modules .. a bit pedantic, but here we go...
    if ((ret = findModule(name))) {
      return(ret);
    }
#endif
    pdstring fullNm, fileNm;
    char *out = P_strdup(name.c_str());
    char *sl = P_strrchr(out, '/');
    if (sl) {
      *sl = (char)0;
      fullNm = out;
      fileNm = sl+1;
    } else {
      fullNm = pdstring("/default/") + out;
      fileNm = out;
    }
    free(out);

    ret = new pdmodule(lang, addr, fullNm, fileNm, this);

#ifndef BPATCH_LIBRARY
    // if module was excluded,stuff it into excludedMods (and dont
    //  index it in modeByFileName && modsByFullName.
    if (module_is_excluded(ret)) {
       excludedMods += ret;
    } else {
      modsByFileName[ret->fileName()] = ret;
      modsByFullName[ret->fullName()] = ret;
      includedMods.push_back(ret);
    }
#else
    modsByFileName[ret->fileName()] = ret;
    modsByFullName[ret->fullName()] = ret;
    _mods.push_back(ret);
#endif /* BPATCH_LIBRARY */

    return(ret);
}


// TODO -- is this g++ specific
bool buildDemangledName(const pdstring &mangled, pdstring &use, bool nativeCompiler, 
			supportedLanguages lang)
{
 /* The C++ demangling function demangles MPI__Allgather (and other MPI__
  * functions with start with A) into the MPI constructor.  In order to
  * prevent this a hack needed to be made, and this seemed the cleanest
  * approach.
  */
  if(!mangled.prefixed_by("MPI__")) {
    char *tempName = P_strdup(mangled.c_str());
    char demangled[1000];
    //cerr << "build demangled name. language = " << lang <<endl;
    if ((lang == lang_Fortran || lang == lang_CMFortran || lang == lang_Fortran_with_pretty_debug) 
	&& tempName[strlen(tempName)-1] == '_') {
      strcpy(demangled, tempName)[strlen(tempName)-1] = '\0';
      //cerr << "generating fortran name:  mangled = " << mangled << "demangled = " << demangled << endl;
      free(tempName);
      use = demangled;
      return true;
    }
    
    int result = P_cplus_demangle(tempName, demangled, 1000, nativeCompiler);
    
    if(result==0) {
      use = demangled;
      free(tempName);
      return true;
    } else {
      //cerr << __FILE__ << __LINE__ << ":  Cannot demangle " << tempName << endl;
      free(tempName); 
      return false;
      
    }
  }
  return(false);
}

void image::addInstruFunction(pd_Function *func, pdmodule *mod,
			      const Address addr,
#ifdef BPATCH_LIBRARY
                  bool /* excluded */
#else
                  bool excluded
#endif // BPATCH_LIBRARY
                  ) {
  pdvector<pd_Function*> *funcsByPrettyEntry = NULL;
  pdvector<pd_Function*> *funcsByMangledEntry = NULL;
  
  // any functions whose instrumentation info could be determined 
  //  get added to instrumentableFunctions, and mod->funcs.
  instrumentableFunctions.push_back(func); 
  mod->funcs.push_back(func);

#ifndef BPATCH_LIBRARY
  if (excluded) {
    excludedFunctions[func->prettyName()] = func;
    return;
    } 
  includedFunctions.push_back(func);
#endif

  funcsByAddr[addr] = func;
  if (!funcsByPretty.find(func->prettyName(), funcsByPrettyEntry)) {
    funcsByPrettyEntry = new pdvector<pd_Function*>;
    funcsByPretty[func->prettyName()] = funcsByPrettyEntry;
  }
  // several functions may have the same demangled name, and each one
  // will appear in a different module
  assert(funcsByPrettyEntry);
  (*funcsByPrettyEntry).push_back(func);
  
  if (!funcsByMangled.find(func->symTabName(), funcsByMangledEntry)) {
    funcsByMangledEntry = new pdvector<pd_Function*>;
    funcsByMangled[func->symTabName()] = funcsByMangledEntry;
  }
  // several functions may have the same demangled name, and each one
  // will appear in a different module
  //printf("%s[%d]:  check duplicates here.", __FILE__, __LINE__);
  assert(funcsByMangledEntry);
  for (unsigned int i = 0; i < funcsByMangledEntry->size(); ++i) {
    if ((*funcsByMangledEntry)[i] == func) {
      cerr << __FILE__ << __LINE__<< ": found duplicate of mangled function name: " 
	   << func->symTabName() << ".  Discarding. "<<endl;
    }
    else
      (*funcsByMangledEntry).push_back(func);
  }
}

// A helper routine for removeInstrumentableFunc -- removes function from specified hash
void image::removeFuncFromNameHash(pd_Function * /* func */, 
				   pdstring & /* fname */,
				   dictionary_hash<pdstring, pdvector<pd_Function *> > * /* func_hash */ )
{
#ifdef NOTDEF
  pdvector<pd_Function*> *funcsEntry = NULL;

  if (func_hash->find(fname, funcsEntry)) {
   assert(funcsEntry);
   if (!funcsEntry->size()) {
     // no funcs map onto this name, weird case, this is only a sanity check
     cerr << __FILE__ << __LINE__ << ":  WEIRD situation that should not happen detected..."
	  << "  zero-size vector of functions matches function name " << fname << endl;
     func_hash->undef(fname);
   }
   else if (funcsEntry->size() == 1) {
     //  the easy (and usual) case, function name has only one match
     //  do we need to do anything to verify that this match is indeed the right function?  
     //  lets say "no" and clear the whole thing out.
     func_hash->undef(fname);
     delete funcsEntry;
   }
   else {
     // this name matches more than one function, get rid of all elements of the vector
     // that have the same address
     pdvector<pd_Function *> *newFuncsEntry = NULL;
     for (unsigned int i = 0; i < funcsEntry->size(); ++i) {
       if (func->addr() != (*funcsEntry)[i]->addr()) {
	 if (! newFuncsEntry) newFuncsEntry = new pdvector<pd_Function *>;
	 newFuncsEntry->push_back((*funcsEntry)[i]);
       }
     }
     if (newFuncsEntry) {
       if (newFuncsEntry->size() == funcsEntry->size()) {
	 // something is odd here, we kept all of the functions, meaning that none of the
	 // found functions shared an address with the one that we want to remove.
	 // is this bad?
	 cerr << __FILE__ << __LINE__ << ":  WEIRD situation that should not happen detected..."
	  << "  could not find address match in name vector for name " << fname << endl;
       }
       else {
	 // replace the old vector with the new one and destroy the old
	 func_hash->undef(fname);
	 (*func_hash)[fname] = newFuncsEntry;
       }
     }
     else {
       cerr << __FILE__ << __LINE__ << ":  WEIRD situation that should not happen detected..."
	    << "  could not find address match in name vector for name " << fname << endl;
       }
   }
  }
  else {
    // function not found by name. this case is not to be expected
    // However, since it may not be critically important, the below warning can probably be 
    // removed if it is getting in the way.
       cerr << __FILE__ << __LINE__ << ":  WEIRD situation that should not happen detected..."
	    << "  could not find instrumentable function " << fname << endl;
  }
#endif
}

// if a function in the "instrumentable" data structs is later determined
// to be uninstrumentable, this function provides an atomic operation for 
// removing it from said structs.
int image::removeFuncFromInstrumentable(pd_Function * /* func */)
{
#ifdef NOTDEF
  assert(func);
  pdstring prettyName = func->prettyName();
  pdstring mangledName = func->symTabName();
  pdmodule *mod = func->file();

  // The ugly part of this whole process is this first linear search
  // If we need to remove more than a handful of functions, this should either be
  // optimized (eg sorted), or the whole process re-thought-out

  for (unsigned int i = 0; i < instrumentableFunctions.size(); ++i) {
    if (func == instrumentableFunctions[i]) {
      // replace func with the last element in the vector and resize
      instrumentableFunctions[i] = instrumentableFunctions[instrumentableFunctions.size()-1];
      instrumentableFunctions.pop_back();
    }
  }

  // remove from module lists too...
  if (mod) {
    if (!mod->removeInstruFunc(func)) {
      cerr << __FILE__ << __LINE__ << ":  WEIRD situation that should not happen detected..."
	   << "  could not remove function from module:    " << prettyName << endl;
    }
  }
  else {
    cerr << __FILE__ << __LINE__ << ":  WEIRD situation that should not happen detected..."
	 << "  function without module found while making un-instrumentable:    " << prettyName << endl;
  }

#ifndef BPATCH_LIBRARY
  if (excludedFunctions.defines(prettyName)) {
    excludedFunctions.undef(prettyName);
    // if function was excluded, it won't be in any of the other structs (right???)
    // so we are done
    return 0;
  }
  else {
    if (includedFunctions.defines(prettyName)) 
      includedFunctions.undef(prettyName);
    else {
      // not sure if a warning is needed here, but if the function was not in
      // excludedFunctions, in should be in includedFunctions -- I think
      cerr << __FILE__ << __LINE__ << ":  WEIRD situation that should not happen detected..."
	   << "  not-excluded function was not in included list:  " << prettyName << endl;
    }
  }
#endif


  removeFuncFromNameHash(func, prettyName, &funcsByPretty);
  removeFuncFromNameHash(func, mangledName, &funcsByMangled);

  if (funcsByAddr.defines(func->addr()))
    funcsByAddr.undef(func->addr());
  else {
    cerr << __FILE__ << __LINE__ << ":  WEIRD situation that should not happen detected..."
	 << "  no record of known function found in funcsByAddr:  " << prettyName << endl;
  }
#endif
  return 0;
}


void image::addNotInstruFunc(pd_Function *func, pdmodule *mod) {
    notInstruFunctions[func->prettyName()] = func;
    mod->notInstruFuncs.push_back(func);
}

#ifdef DEBUG_TIME
static timer loadTimer;
static FILE *timeOut=0;
#endif /* DEBUG_TIME */

// makeOneFunction(): find name of enclosing module and define function symbol
//
// module information comes from one of three sources:
//   #1 - debug format (stabs, DWARF, etc.)
//   #2 - file format (ELF, COFF)
//   #3 - file name (a.out, libXXX.so)
// (in order of decreasing reliability)
pd_Function *image::makeOneFunction(pdvector<Symbol> &mods,
				    const Symbol &lookUp) 
{
  // find module name
  Address modAddr = 0;
  pdstring modName = lookUp.module();
  
  if (modName == "") {
    modName = name_ + "_module";
  } else if (modName == "DEFAULT_MODULE") {
    pdstring modName_3 = modName;
    findModByAddr(lookUp, mods, modName, modAddr, modName_3);
  }
  
  pdmodule *use = getOrCreateModule(modName, modAddr);
  assert(use);


  pd_Function *func = new pd_Function(lookUp.name(), use, lookUp.addr(), lookUp.size());
  assert(func);

  return func;
}

//Add an extra pretty name to a known function (needed for handling
//overloaded functions in paradyn)
void image::addTypedPrettyName( pd_Function *func, const char *typedName) {
   pdvector<pd_Function*> *funcsByPrettyEntry = NULL;
   pdstring typed(typedName);

   if (!funcsByPretty.find(typed, funcsByPrettyEntry)) {
      funcsByPrettyEntry = new pdvector<pd_Function*>;
      funcsByPretty[typed] = funcsByPrettyEntry;
   }
   assert(funcsByPrettyEntry);
   (*funcsByPrettyEntry).push_back(func);
}


/*
 * Add another name for the current function to the names vector in
 * the function object.  We also need to add the extra names to the
 * lookup hash tables
 */
void image::addMultipleFunctionNames(pd_Function *dup)
					
{
  // Obtain the original function at the same address:
  pd_Function *orig = funcsByAddr[dup->addr()];
  assert(orig); //sanity check

  pdstring mangled_name = dup->symTabName();
  pdstring pretty_name = dup->prettyName();

  /* add the new names to the existing function */
  orig->addSymTabName(mangled_name);
  orig->addPrettyName(pretty_name);

  /* now we add the names and the function object to the hash tables */
  //  Mangled Hash:
  pdvector<pd_Function*> *funcsByMangledEntry = NULL;
  if (!funcsByMangled.find(mangled_name, funcsByMangledEntry)) {
    funcsByMangledEntry = new pdvector<pd_Function*>;
    funcsByMangled[mangled_name] = funcsByMangledEntry;
  }

  assert(funcsByMangledEntry);
  (*funcsByMangledEntry).push_back(orig); // might need to check/eliminate duplicates here??

  // Pretty Hash:
  pdvector<pd_Function*> *funcsByPrettyEntry = NULL;
  if(!funcsByPretty.find(pretty_name, funcsByPrettyEntry)) {
    funcsByPrettyEntry = new pdvector<pd_Function*>;
    funcsByPretty[pretty_name] = funcsByPrettyEntry;
  }
    
  assert(funcsByPrettyEntry);
  (*funcsByPrettyEntry).push_back(orig); // might need to check/eliminate duplicates here??

#ifdef NOTDEF
  /* build the mangeled and pretty names so that we can add those to the
   * lookup tables
   */
  pdstring name = lookUp.name();
  pdstring mangled_name = name;
  const char *p = P_strchr(name.c_str(), ':');
  if (p) {
     unsigned nchars = p - name.c_str();
     mangled_name = pdstring(name.c_str(), nchars);
  }

  pdstring demangled;
  if (!buildDemangledName(mangled_name, demangled, nativeCompiler, func->file()->language())) 
    demangled = mangled_name;

  /* add the names to the vectors in the function object */
  func->addSymTabName(mangled_name);
  func->addPrettyName(demangled);

  /* now we add the names and the function object to the hash tables */
  pdvector<pd_Function*> *funcsByPrettyEntry = NULL;
  pdvector<pd_Function*> *funcsByMangledEntry = NULL;

  if(!funcsByPretty.find(demangled, funcsByPrettyEntry)) {
    funcsByPrettyEntry = new pdvector<pd_Function*>;
    funcsByPretty[demangled] = funcsByPrettyEntry;
  }
    
  assert(funcsByPrettyEntry);
  (*funcsByPrettyEntry).push_back(func);

  if (!funcsByMangled.find(mangled_name, funcsByMangledEntry)) {
    funcsByMangledEntry = new pdvector<pd_Function*>;
    funcsByMangled[mangled_name] = funcsByMangledEntry;
  }

  assert(funcsByMangledEntry);
  (*funcsByMangledEntry).push_back(func);
#endif
}

/*
 * Add all the functions (*) in the list of symbols to our data
 * structures. 
 *
 * We do a search for a "main" symbol (a couple of variants), and
 * if found we flag this image as the executable (a.out). 
 */

bool image::addAllFunctions(pdvector<Symbol> &mods, 
			    pdvector<pd_Function *> *raw_funcs)
{
#if defined(TIMED_PARSE)
  struct timeval starttime;
  gettimeofday(&starttime, NULL);
#endif

  Symbol lookUp;
  pdstring symString;

  // is_a_out is a member variable
  Symbol mainFuncSymbol;  //Keeps track of info on "main" function

  //Checking "main" function names in same order as in the inst-*.C files
  if (linkedFile.get_symbol(symString="main",     lookUp) ||
      linkedFile.get_symbol(symString="_main",    lookUp)
#if defined(i386_unknown_nt4_0)
      ||
      linkedFile.get_symbol(symString="WinMain",  lookUp) ||
      linkedFile.get_symbol(symString="_WinMain", lookUp) ||
      linkedFile.get_symbol(symString="wWinMain", lookUp) ||
      linkedFile.get_symbol(symString="_wWinMain", lookUp)
#endif
      ) {
    mainFuncSymbol = lookUp;
    is_a_out = true;

    if (lookUp.type() == Symbol::PDST_FUNCTION) {
      if (!isValidAddress(lookUp.addr())) {
         pdstring msg;
         char tempBuffer[40];
         sprintf(tempBuffer,"0x%lx",lookUp.addr());
         msg = pdstring("Function ") + lookUp.name() + pdstring(" has bad address ")
	  + pdstring(tempBuffer);
         statusLine(msg.c_str());
         showErrorCallback(29, msg);
         return false;
      }
      pd_Function *main_pdf = makeOneFunction(mods, lookUp);
      assert(main_pdf);
      raw_funcs->push_back(main_pdf);
    }
  }
  else
    is_a_out = false;

  // Checking for libdyninstRT (DYNINSTinit())
  if (linkedFile.get_symbol(symString="DYNINSTinit",  lookUp) ||
      linkedFile.get_symbol(symString="_DYNINSTinit", lookUp))
    is_libdyninstRT = true;
  else
    is_libdyninstRT = false;
 
#if !defined(BPATCH_LIBRARY) //ccw 19 apr 2002 : SPLIT
  if (linkedFile.get_symbol(symString="PARADYNinit",  lookUp) ||
      linkedFile.get_symbol(symString="PARADYNinit", lookUp))
    is_libparadynRT = true;
  else
    is_libparadynRT = false;
 
#endif
 
  // JAW 02-03 -- restructured below slightly to get rid of multiple loops
  // through entire symbol list
  pdvector<Symbol> kludge_symbols;
  
  // find the real functions -- those with the correct type in the symbol table
  for(SymbolIter symIter(linkedFile); symIter;symIter++) {
    const Symbol &lookUp = symIter.currval();
    const char *np = lookUp.name().c_str();
    if (linkedFile.isEEL() && np[0] == '.')
         /* ignore these EEL symbols; we don't understand their values */
	 continue; 
    if (is_a_out && 
	(lookUp.addr() == mainFuncSymbol.addr()) &&
	(lookUp.name() == mainFuncSymbol.name()))
      // We already added main(), so skip it now
      continue;
#ifdef NOTDEF
    //  This is now done later while building the "real" maps.
    //  We will have some duplication/aliasing while building up the raw (unclassed) map
    // but these will be weeded out later according to the same criteria.
    if (funcsByAddr.defines(lookUp.addr())) {
      // We have already seen a function at this addr. add a second name
      // for this function.
      addMultipleFunctionNames(lookUp);
      continue;
    }
#endif

    if (lookUp.type() == Symbol::PDST_FUNCTION) {
      pdstring msg;
      char tempBuffer[40];
      if (!isValidAddress(lookUp.addr())) {
         sprintf(tempBuffer,"0x%lx",lookUp.addr());
         msg = pdstring("Function ") + lookUp.name() + pdstring(" has bad address ")
            + pdstring(tempBuffer);
         statusLine(msg.c_str());
         showErrorCallback(29, msg);
         return false;
      }
      pd_Function *new_func = makeOneFunction(mods, lookUp);
      if (!new_func)
         cerr << __FILE__ << __LINE__ << ":  makeOneFunction returned NULL!" << endl;
      else
         raw_funcs->push_back(new_func);
    }

    if (lookUp.type() ==  Symbol::PDST_OBJECT) {
      //  JAW: This is here for legacy purposes, I do not know if it still applies:
      if (lookUp.kludge()) {
	//logLine(P_strdup(symString.c_str()));
	// Figure out where this happens
	
	// now find the pseudo functions -- this gets ugly
	// kludge has been set if the symbol could be a function
	// WHERE DO WE USE THESE KLUDGES? WHAT PLATFORM???
	
	cerr << "Found <KLUDGE> function " << lookUp.name().c_str() 
	     << ".  All <KLUDGE>  functions currently ignored!  see " 
	     << __FILE__ << __LINE__ << endl;
	//kludge_symbols.push_back(lookUp);
	
      }
    }
  }

#ifdef NOTDEF
  // go through vector of kludge functions found and add ones that are not already def'd. 
  for (unsigned int i = 0; i < kludge_symbols.size(); ++i) {
    if (funcsByAddr.defines(kludge_symbols[i].addr()))
      // Already defined a symbol at this addr
      continue;
    addOneFunction(mods, lookUp);
  }
#endif

#if defined(TIMED_PARSE)
  struct timeval endtime;
  gettimeofday(&endtime, NULL);
  unsigned long lstarttime = starttime.tv_sec * 1000 * 1000 + starttime.tv_usec;
  unsigned long lendtime = endtime.tv_sec * 1000 * 1000 + endtime.tv_usec;
  unsigned long difftime = lendtime - lstarttime;
  double dursecs = difftime/(1000 );
  cout << __FILE__ << ":" << __LINE__ <<": addAllFunctions took "<<dursecs <<" msecs" << endl;
#endif
  return true;
}

bool image::addAllVariables()
{
/* Eventually we'll have to do this on all platforms (because we'll retrieve
 * the type information here).
 */
#if defined( i386_unknown_nt4_0 ) ||\
    defined( mips_unknown_ce2_11 ) ||\
    defined( i386_unknown_linux2_4 )

#if defined(TIMED_PARSE)
  struct timeval starttime;
  gettimeofday(&starttime, NULL);
#endif

  pdstring mangledName; 
  Symbol symInfo;

  for(SymbolIter symIter(linkedFile); symIter; symIter++) {
     const pdstring &mangledName = symIter.currkey();
     const Symbol &symInfo = symIter.currval();

    if (symInfo.type() == Symbol::PDST_OBJECT) {
       char unmangledName[1000];
       int result = P_cplus_demangle((char*)mangledName.c_str(), unmangledName,
                                     1000, nativeCompiler);
       if(result == 1) {
          strcpy(unmangledName, mangledName.c_str());
       }
       if (varsByPretty.defines(unmangledName)) {
	  (*(varsByPretty[unmangledName])).push_back(pdstring(mangledName));
       } else {
	  pdvector<pdstring> *varEntry = new pdvector<pdstring>;
	  (*varEntry).push_back(pdstring(mangledName));
	  varsByPretty[unmangledName] = varEntry;
       }
    }
  }

#if defined(TIMED_PARSE)
  struct timeval endtime;
  gettimeofday(&endtime, NULL);
  unsigned long lstarttime = starttime.tv_sec * 1000 * 1000 + starttime.tv_usec;
  unsigned long lendtime = endtime.tv_sec * 1000 * 1000 + endtime.tv_usec;
  unsigned long difftime = lendtime - lstarttime;
  double dursecs = difftime/(1000 );
  cout << __FILE__ << ":" << __LINE__ <<": addAllVariables took "<<dursecs <<" msecs" << endl;
#endif

#endif
  return true;
}

/*
 * will search for symbol NAME or _NAME
 * returns false on failure 
 */
bool image::findInternalSymbol(const pdstring &name, 
			       const bool warn, 
			       internalSym &ret_sym)
{
   Symbol lookUp;

   if(linkedFile.get_symbol(name,lookUp)){
      ret_sym = internalSym(lookUp.addr(),name); 
      return true;
   }
   else {
       pdstring new_sym;
       new_sym = pdstring("_") + name;
       if(linkedFile.get_symbol(new_sym,lookUp)){
          ret_sym = internalSym(lookUp.addr(),name); 
          return true;
       }
   } 
   if(warn){
      pdstring msg;
      msg = pdstring("Unable to find symbol: ") + name;
      statusLine(msg.c_str());
      showErrorCallback(28, msg);
   }
   return false;
}

bool image::findInternalByPrefix(const pdstring &prefix, 
				 pdvector<Symbol> &found) const
{
  bool flag = false;
  /*
    Go through all defined symbols and return those which
    match the prefix given 
  */
  for(SymbolIter symIter(linkedFile); symIter;symIter++) {
    const Symbol &lookUp = symIter.currval();
    if (!strncmp(prefix.c_str(), lookUp.name().c_str(),
		 strlen(prefix.c_str())))
      {
	found.push_back(lookUp);
	flag = true;
      }
  }
  //if (!flag) {
  //  cerr << __FILE__ << __LINE__ << ":  findInternalByPrefix("<<prefix
  //	 <<") failed, num symbols: " << linkedFile.nsymbols() << endl;
  //}
  return flag;
}
#ifndef BPATCH_LIBRARY
pdmodule *image::findModule(const pdstring &name, bool find_if_excluded)
#else
pdmodule *image::findModule(const pdstring &name)
#endif
{
  //cerr << "image::findModule " << name << " , " << find_if_excluded
  //     << " called" << endl;

  if (modsByFileName.defines(name)) {
    //cerr << " (image::findModule) found module in modsByFileName" << endl;
    return (modsByFileName[name]);
  }
  else if (modsByFullName.defines(name)) {
    //cerr << " (image::findModule) found module in modsByFullName" << endl;
    return (modsByFullName[name]);
  }

#ifndef BPATCH_LIBRARY
  unsigned i;

  // if also looking for excluded functions, check through 
  //  excludedFunction list to see if any match function by FullName
  //  or FileName....
  if (find_if_excluded) {
      for(i=0;i<excludedMods.size();i++) {
         if ((excludedMods[i]->fileName() == name) ||
             (excludedMods[i]->fullName() == name)) {
	  //cerr << " (image::findModule) found module in excludedMods" << endl;
	  return excludedMods[i];
	}
      }
  }
#endif
  //cerr << " (image::findModule) did not find module, returning NULL" << endl;
  return NULL;
}

#ifndef BPATCH_LIBRARY
/* 
 * return 0 if symbol <symname> exists in image, non-zero if it does not
 */
bool image::symbolExists(const pdstring &symname)
{
  pdvector<pd_Function *> *pdfv;
  if (NULL != (pdfv = findFuncVectorByPretty(symname)) && pdfv->size())
    return true;

  if (NULL != (findFuncByMangled(symname)))
    return true;

  return false;
}

void image::postProcess(const pdstring pifname)
{
  FILE *Fil;
  pdstring fname, errorstr;
  char key[5000];
  char tmp1[5000], abstraction[500];
  resource *parent;

  return;

  /* What file to open? */
  if (!(pifname == (char*)NULL)) {
    fname = pifname;
  } else {
    fname = desc_->file() + ".pif";
  }

  /* Open the file */
  Fil = P_fopen(fname.c_str(), "r");

  if (Fil == NULL) {
    errorstr = pdstring("Tried to open PIF file ") + fname;
    errorstr += pdstring(", but could not (continuing)\n");
    logLine(P_strdup(errorstr.c_str()));
    showErrorCallback(35, errorstr); 
    return;
  }

  /* Process the file */
  while (!feof(Fil)) {
    fscanf(Fil, "%s", key);
    switch (key[0]) {
    case 'M':
      /* Ignore mapping information for right now */
      fgets(tmp1, 5000, Fil);
      break;
    case 'R':
#ifndef BPATCH_LIBRARY
      /* Create a new resource */
      fscanf(Fil, "%s {", abstraction);
      parent = rootResource;
      do {
	fscanf(Fil, "%s", tmp1);
        if (tmp1[0] != '}') {
          parent = resource::newResource(parent, NULL,
					 abstraction,
					 tmp1, timeStamp::ts1970(),
					 nullString, // uniqifier
					 MDL_T_STRING,
					 true);
        } else {
	  parent = NULL;
	}
      } while (parent != NULL);
#endif
      break;
    default:
      errorstr = pdstring("Ignoring bad line key (") + fname;
      errorstr += pdstring(") in file %s\n");
      logLine(P_strdup(errorstr.c_str()));
      fgets(tmp1, 5000, Fil);
      break;
    }
  }
  return;
}
#endif

void image::defineModules() {

  pdstring pds; pdmodule *mod;
  dictionary_hash_iter<pdstring, pdmodule*> mi(modsByFileName);

  while (mi.next(pds, mod)){
    mod->define();
  }

#ifndef BPATCH_LIBRARY
  unsigned i;

  for(i=0;i<excludedMods.size();i++) {
    mod = excludedMods[i];
    mod->define();
  }


#ifdef DEBUG_MDL
  std::ostringstream osb(std::ios::out);
  osb << "IMAGE_" << name() << "__" << getpid() << std::ends;
  ofstream of(osb, std::ios::app);

  of << "INCLUDED FUNCTIONS\n";
  for (unsigned ni=0; ni<includedFunctions.size(); ni++) {
    of << includedFunctions[ni]->prettyName() << "\t\t" 
       << includedFunctions[ni]->addr() << "\t\t" << endl;
  }
  
#endif

  
#endif
}



//  Comments on what this does would be nice....
//  Appears to run over a pdmodule, after all code in it has been processed
//   and parsed into functions, and define a resource for the module + a 
//   resource for every function found in the module (apparently includes 
//   excluded functions, but not uninstrumentable ones)....
//  Can't directly register call graph relationships here as resources
//   are being defined, because need all resources defined to 
//   do that....


void pdmodule::define() {
#ifdef DEBUG_MODS
  std::ostringstream osb(std::ios::out);
  osb << "MODS_" << exec->name() << "__" << getpid() << std::ends;
  ofstream of(osb, std::ios::app);
#endif

  unsigned f_size = funcs.size();

  for (unsigned f=0; f<f_size; f++) {
#ifndef BPATCH_LIBRARY
    pd_Function *pdf = funcs[f];
#ifdef DEBUG_MODS
    of << fileName << ":  " << pdf->prettyName() <<  "  "
        << pdf->addr() << endl;
#endif
    // ignore line numbers for now 

    //if (!(pdf->isLibTag())) {
    if (1) {
      // see if we have created module yet.
      if (!modResource) {
	modResource = resource::newResource(moduleRoot, this,
					    nullString, // abstraction
					    fileName(), // name
					  timeStamp::ts1970(), // creation time
					    pdstring(), // unique-ifier
					    MDL_T_MODULE,
					    false);
      }

      //check if the function is overloaded, and store types with the name
      //in the case that it is.  This way, we can differentiate
      //between overloaded functions in the paradyn front-end.
      bool useTyped = false;
      pdvector <pd_Function *> *pdfv;
      char prettyWithTypes[1000];
      if (NULL != (pdfv = exec()->findFuncVectorByPretty(pdf->prettyName()))
          && pdfv->size() > 1) {
         char *tempName = P_strdup(pdf->symTabName().c_str());
         int result = P_cplus_demangle(tempName, prettyWithTypes,
                                       1000, exec()->isNativeCompiler(), true /* include types */ );
         if (result == 0) {
            useTyped= true;
            //add another "pretty name" to the dictionary
            exec()->addTypedPrettyName(pdf, prettyWithTypes);
            pdf->addPrettyName(pdstring(prettyWithTypes));
         }
         free(tempName);
      }

      pdf->SetFuncResource(resource::newResource(modResource, pdf,
						 nullString, // abstraction
						  useTyped ? prettyWithTypes : pdf->prettyName(), 
						 timeStamp::ts1970(),
						 nullString, // uniquifier
						 MDL_T_PROCEDURE,
						 false));		   
      
    }
#endif
  }

#ifndef BPATCH_LIBRARY
  resource::send_now();
#endif
}

#ifndef BPATCH_LIBRARY
// as per getAllFunctions, but filters out those excluded with 
// e.g. mdl "exclude" command....
// to clarify a function should NOT be returned from here if its 
//  module is excluded (with exclude "/Code/module") or if it
//  is excluded (with exclude "/Code/module/function"). 
const pdvector<pd_Function*> &image::getIncludedFunctions() {
    unsigned int i;    
    includedFunctions.resize(0);
    pdvector<function_base *> *temp;
    pdvector<pd_Function *> *temp2;

    //cerr << "image::getIncludedFunctions called, name = " << name () << endl;
    //cerr << " includedMods = " << endl;
    //print_module_vector_by_short_name(pdstring("  "), &includedMods);
    for (i=0;i<includedMods.size();i++) {
         temp = includedMods[i]->getIncludedFunctions();
         temp2 = (pdvector<pd_Function *> *) temp;
         includedFunctions += *temp2;
    }

    //cerr << " (image::getIncludedFunctions) returning : " << endl;
    //print_func_vector_by_pretty_name(pdstring("  "),
    //		 (vector<function_base*>*)&includedFunctions);

    // what about shared objects????
    return includedFunctions;
}



// get all functions in module which are not "excluded" (e.g.
//  with mdl "exclude" command.  
// Assed to provide support for mdl "exclude" on functions in
//  statically linked objects.
//  mcheyney 970727

pdvector<function_base *> *pdmodule::getIncludedFunctions() {
    // laxy construction of some_funcs, as per sharedobject class....
    // cerr << "pdmodule " << fileName() << " :: getIncludedFunctions called "
    //      << endl;
    if (some_funcs_inited == TRUE) {
        //cerr << "  about to return : " << endl;
        //print_func_vector_by_pretty_name(pdstring("  "), (pdvector<function_base *>*)&some_funcs);
        return (pdvector<function_base *>*)&some_funcs;
    }
#ifdef BPATCH_LIBRARY  //BPatch Library doesn't know about excluded funcs 
    some_funcs = funcs;
#else
    some_funcs.resize(0);
    
    if (filter_excluded_functions(funcs, some_funcs, fileName()) == FALSE) {
        //cerr << "  about to return NULL";
	return NULL;
    }
#endif
    some_funcs_inited = TRUE;
    
    //cerr << "  about to return : " << endl;
    //print_func_vector_by_pretty_name(pdstring("  "),(pdvector<function_base *>*) &some_funcs);
    return (pdvector<function_base *>*)&some_funcs;
}
#endif

// Tests if a symbol starts at a given point
bool image::hasSymbolAtPoint(Address point) const
{
    return knownSymAddrs.defines(point);
}

const pdvector<pd_Function*> &image::getAllFunctions() 
{
    return instrumentableFunctions;
}

#ifdef BPATCH_LIBRARY
const pdvector <pdmodule*> &image::getModules() 
{
  return _mods;
}
#else



const pdvector <pdmodule*> &image::getAllModules() 
{
    // reinit all modules to empty vector....
    allMods.resize(0);

    // and add includedModules && excludedModules to it....
    VECTOR_APPEND(allMods, includedMods);
    VECTOR_APPEND(allMods, excludedMods);

    //cerr << "image::getAllModules called" << endl;
    //cerr << " about to return sum of includedMods and excludedMods" << endl;
    //cerr << " includedMods : " << endl;
    //print_module_vector_by_short_name(pdstring("  "), &includedMods);
    //cerr << " excludedMods : " << endl;
    //print_module_vector_by_short_name(pdstring("  "), &excludedMods);
    
    return allMods;
}



const pdvector <pdmodule*> &image::getIncludedModules() {
    //cerr << "image::getIncludedModules called" << endl;
    //cerr << " about to return includedMods = " << endl;
    //print_module_vector_by_short_name(pdstring("  "), &includedMods);

    return includedMods;
}

const pdvector <pdmodule*> &image::getExcludedModules() {
    //cerr << "image::getIncludedModules called" << endl;
    //cerr << " about to return includedMods = " << endl;
    //print_module_vector_by_short_name(pdstring("  "), &includedMods);

   return excludedMods;
}
#endif

void print_module_vector_by_short_name(pdstring prefix ,
				       pdvector<pdmodule*> *mods) {
    unsigned int i;
    pdmodule *mod;
    for(i=0;i<mods->size();i++) {
        mod = ((*mods)[i]);
	cerr << prefix << mod->fileName() << endl;
    }
}

void print_func_vector_by_pretty_name(pdstring prefix,
				      pdvector<function_base *>*funcs) {
    unsigned int i;
    function_base *func;
    for(i=0;i<funcs->size();i++) {
      func = ((*funcs)[i]);
      cerr << prefix << func->prettyName() << endl;
    }
}

#ifndef BPATCH_LIBRARY
// rip module name out of constraint....
// Assumes that constraint is of form module/function, or
// module.... 
pdstring getModuleName(pdstring constraint) {
    pdstring ret;

    const char *data = constraint.c_str();
    const char *first_slash = P_strchr(data, RH_SEPERATOR);
    
    // no "/", assume string holds module name....
    if (first_slash == NULL) {
	return constraint;
    }    
    // has "/", assume everything up to "/" is module name....
    return pdstring(data, first_slash - data);
}

// rip function name out of constraint....
// Assumes that constraint is of form module/function, or
// module.... 
pdstring getFunctionName(pdstring constraint) {
    pdstring ret;

    const char *data = constraint.c_str();
    const char *first_slash = P_strchr(data, RH_SEPERATOR);
    
    // no "/", assume constraint is module only....
    if (first_slash == NULL) {
	return pdstring("");
    }    
    // has "/", assume everything after "/" is function....
    return pdstring(first_slash+1);
}




// mcheyney, Oct. 6, 1997
static dictionary_hash<pdstring, pdstring> func_constraint_hash(pdstring::hash);
static bool cache_func_constraint_hash() {
    static bool func_constraint_hash_loaded = FALSE;

    // strings holding exclude constraints....
    pdvector<pdstring> func_constraints;
    // if unble to get list of excluded functions, assume all functions
    //  are NOT excluded!!!!
    if(mdl_get_lib_constraints(func_constraints) == FALSE) {
	return FALSE;
    }
    func_constraint_hash_loaded = TRUE;

    unsigned i;
    for(i=0;i<func_constraints.size();i++) {
	func_constraint_hash[func_constraints[i]] = func_constraints[i];
    }
    return TRUE;
}

// mcheyney, Oct. 3, 1997
// Return boolean value indicating whether function is found to
//  be excluded via "exclude module_name/function_name" (but NOT
//  via "exclude module_name").
bool function_is_excluded(pd_Function *func, pdstring module_name) {
    static bool func_constraint_hash_loaded = FALSE;

    pdstring function_name = func->prettyName();
    pdstring full_name = module_name + pdstring("/") + function_name;

    if (func_constraint_hash_loaded == FALSE) {
        if (!cache_func_constraint_hash()) {
	    return FALSE;
        }
    }

    if (func_constraint_hash.defines(full_name)) {
        return TRUE;
    }
    return FALSE;
}


bool module_is_excluded(pdmodule *module) {
    static bool func_constraint_hash_loaded = FALSE;

    pdstring full_name = module->fileName();

    if (func_constraint_hash_loaded == FALSE) {
        if (!cache_func_constraint_hash()) {
	    return FALSE;
        }
    }

    if (func_constraint_hash.defines(full_name)) {
        return TRUE;
    }
    return FALSE;
}

//
// mcheyney, Sep 28, 1997
// Take a list of functions (in vector <all_funcs>.  Copy all
//  of those functions which are not excluded (via "exclude" 
//  {module==module_name}/{function==function_name) into
//  <some_functions>.  DONT filter out those excluded via
//  exclude module==module_name...., eh????
// Returns status of mdl_get_lib_constraints() call.
//  If this status == FALSE< some_funcs is not modified....
// We assume that all_funcs is generally longer than the list
//  of constrained functions.  As such, internally proc. copies
//  all funcs into some funcs, then runs over excluded funcs
//  removing any matches, as opposed to doing to checking 
//  while adding to some_funcs....

bool filter_excluded_functions(pdvector<pd_Function*> all_funcs,
    pdvector<pd_Function*>& some_funcs, pdstring module_name) {
    unsigned i;

    pdstring full_name;
    static bool func_constraint_hash_loaded = FALSE;

    if (func_constraint_hash_loaded == FALSE) {
        if (!cache_func_constraint_hash()) {
	    return FALSE;
        }
    }

    // run over all_funcs, check if module/function is caught
    //  by an exclude....
    for(i=0;i<all_funcs.size();i++) {
        full_name = module_name + pdstring("/") + all_funcs[i]->prettyName();
	if (!(func_constraint_hash.defines(full_name))) {
            some_funcs += all_funcs[i];
        }
    }
    
    return TRUE;
}

#endif /* BPATCH_LIBRARY */

// identify module name from symbol address (binary search)
// based on module tags found in file format (ELF/COFF)
void image::findModByAddr (const Symbol &lookUp, pdvector<Symbol> &mods,
			   pdstring &modName, Address &modAddr, 
			   const pdstring &defName)
{
  if (mods.size() == 0) {
    modAddr = 0;
    modName = defName;
    return;
  }

  Address symAddr = lookUp.addr();
  int index;
  int start = 0;
  int end = mods.size() - 1;
  int last = end;
  bool found = false;
  while ((start <= end) && !found) {
    index = (start+end)/2;
    if ((index == last) ||
	((mods[index].addr() <= symAddr) && 
	 (mods[index+1].addr() > symAddr))) {
      modName = mods[index].name();
      modAddr = mods[index].addr();      
      found = true;
    } else if (symAddr < mods[index].addr()) {
      end = index - 1;
    } else {
      start = index + 1;
    }
  }
  if (!found) { 
    // must be (start > end)
    modAddr = 0;
    modName = defName;
    //modName = mods[0].name();
    //modAddr = mods[0].addr();
  }
}

unsigned int int_addrHash(const Address& addr) {
  return (unsigned int)addr;
}

/*
 * Remove a parsed executable from the global list. Used if the old handle
 * is no longer valid.
 */
void image::removeImage(image *img)
{
    pdvector<image*> newImages;
    for (unsigned i = 0; i < allImages.size(); i++)
        if (allImages[i] != img)
            newImages.push_back(allImages[i]);
    
    allImages = newImages;
}

void image::removeImage(const pdstring file)
{
    pdvector<image*> newImages;
    for (unsigned i = 0; i < allImages.size(); i++)
        if (allImages[i]->file() != file)
            newImages.push_back(allImages[i]);
    
    allImages = newImages;
}

void image::removeImage(fileDescriptor *desc)
{
    pdvector<image*> newImages;
    for (unsigned i = 0; i < allImages.size(); i++)
        // Never bothered to implement a != operator
        if (!(*(allImages[i]->desc()) == *desc))
            newImages.push_back(allImages[i]);
    
    allImages = newImages;
}

/*
 * load an executable:
 *   1.) parse symbol table and identify rotuines.
 *   2.) scan executable to identify inst points.
 *
 *  offset is normally zero except on CM-5 where we have two images in one
 *    file.  The offset passed to parseImage is the logical offset (0/1), not
 *    the physical point in the file.  This makes it faster to "parse" multiple
 *    copies of the same image since we don't have to stat and read to find the
 *    physical offset. 
 */

image *image::parseImage(fileDescriptor *desc, Address newBaseAddr)
{
  /*
   * Check to see if we have parsed this image at this offset before.
   * We only match if the entire file descriptor matches, which can
   * can be filename matching or filename/offset matching.
   */
  unsigned numImages = allImages.size();
  
  // AIX: it's possible that we're reparsing a file with better information
  // about it. If so, yank the old one out of the images vector -- replace
  // it, basically.
  for (unsigned u=0; u<numImages; u++)
    if ((*desc) == *(allImages[u]->desc()))
      return allImages[u];

  /*
   * load the symbol table. (This is the a.out format specific routine).
   */

  if(desc->isSharedObject()) 
    statusLine("Processing a shared object file");
  else  
    statusLine("Processing an executable file");

  bool err=false;
  
  // TODO -- kill process here
  image *ret = new image(desc, err, newBaseAddr); 

  if (err || !ret) {
      if (ret) {
	  delete ret;
      }
    return NULL;
  }

  bool beenReplaced = false;

#ifdef rs6000_ibm_aix4_1 
  // On AIX, we might have a "stub" image instead of the
  // actual image we want. So we check to see if we do,
  // and if so copy over the list. In normal practice,
  // the stub will be the first and only entry.
  for (unsigned i=0; i<numImages; i++)
    if (allImages[i]->desc()->addr() == (unsigned) -1) {
      image *imageTemp = allImages[i];
      allImages[i]=ret;
      beenReplaced = true;
      delete imageTemp;
  }
  // Add to master image list.
#endif
  if (beenReplaced == false) // short-circuit on non-AIX
    image::allImages.push_back(ret);

  // define all modules.
#ifndef BPATCH_LIBRARY
  tp->resourceBatchMode(true);
#endif

  statusLine("defining modules");

  ret->defineModules();

  statusLine("ready"); // this shouldn't be here, right? (cuz we're not done, right?)

#ifndef BPATCH_LIBRARY
  tp->resourceBatchMode(false);
#endif

  return ret;
}


//  a helper routine that selects a language based on information from the symtab
supportedLanguages pickLanguage(pdstring &working_module, char *working_options, 
				supportedLanguages working_lang)
{
  supportedLanguages lang = lang_Unknown;
  static int sticky_fortran_modifier_flag = 0;

  // (2) -- check suffixes -- try to keep most common suffixes near the top of the checklist
  if (working_module.suffixed_by(".c", 2)) lang = lang_C;
  else if (working_module.suffixed_by(".C",2)) lang = lang_CPlusPlus;
  else if (working_module.suffixed_by(".cpp",4)) lang = lang_CPlusPlus;
  else if (working_module.suffixed_by(".F",2)) lang = lang_Fortran; 
  else if (working_module.suffixed_by(".f",2)) lang = lang_Fortran; 
  else if (working_module.suffixed_by(".cc",3)) lang = lang_C;
  else if (working_module.suffixed_by(".a",2)) lang = lang_Assembly; // is this right?
  else if (working_module.suffixed_by(".S",2)) lang = lang_Assembly; 
  else if (working_module.suffixed_by(".s",2)) lang = lang_Assembly; 
  else {
    //(3) -- try to use options string -- if we have 'em
    if (working_options) {
      //  NOTE:  a binary is labeled "gcc2_compiled" even if compiled w/g77 -- thus this is
      //         quite inaccurate to make such assumptions
      if (strstr(working_options, "gcc")) 
	lang = lang_C; 
      else if (strstr(working_options, "g++")) 
	lang = lang_CPlusPlus; 
    }
  }

  //  This next section tries to determine the version of the debug info generator for a
  //  Sun fortran compiler.  Some leave the underscores on names in the debug info, and some
  //  have the "pretty" names, we need to detect this in order to properly read the debug.
  if (working_lang == lang_Fortran) {
    if (sticky_fortran_modifier_flag) {
      //cerr << __FILE__ << __LINE__ << ": UPDATE: lang_Fortran->lang_Fortran_with_pretty_debug." << endl;
      working_lang = lang_Fortran_with_pretty_debug;
    }
    else if (working_options) {
      char *dbg_gen = NULL;
      //      cerr << __FILE__ << __LINE__ << ":  OPT: " << working_options << endl; 
      if (NULL != (dbg_gen = strstr(working_options, "DBG_GEN="))) {
	//cerr << __FILE__ << __LINE__ << ":  OPT: " << dbg_gen << endl; 
	// Sun fortran compiler (probably), need to examine version
	char *dbg_gen_ver_maj = dbg_gen + strlen("DBG_GEN=");
	//cerr << __FILE__ << __LINE__ << ":  OPT: " << dbg_gen_ver_maj << endl; 
	char *next_dot = strchr(dbg_gen_ver_maj, '.');
	if (NULL != next_dot) {
	  next_dot = '\0';  //terminate major version number string
	  int ver_maj = atoi(dbg_gen_ver_maj);
	  //cerr <<"Major Debug Ver. "<<ver_maj<< endl;
	  if (ver_maj < 3) {
	    working_lang = lang_Fortran_with_pretty_debug;
	    sticky_fortran_modifier_flag = 1;
	    //cerr << __FILE__ << __LINE__ << ": UPDATE: lang_Fortran->lang_Fortran_with_pretty_debug.  "
	    //	 <<"Major Debug Ver. "<<ver_maj<< endl;
	  }
	}
      }
    }
  }
  
  return lang;
}
void image::getModuleLanguageInfo(dictionary_hash<pdstring, supportedLanguages> *mod_langs)
{
#if defined(sparc_sun_solaris2_4) || \
    defined(i386_unknown_solaris2_5) || \
    defined(i386_unknown_linux2_0) || \
    defined(ia64_unknown_linux2_4) /* Temporary duplication -- TLM. */
   // check .stabs section to get language info for modules:
   int stab_nsyms;
   char *stabstr_nextoffset;
   const char *stabstrs = 0;

   char *ptr;
   pdstring mod_string;

   // This ugly flag is set when certain (sun) fortran compilers are detected.
   // If it is set at any point during the following iteration, this routine
   // ends with "backtrack mode" and reiterates through all chosen languages, changing
   // lang_Fortran to lang_Fortran_with_pretty_debug.
   //
   // This may be ugly, but it is set up this way since the information that is used
   // to determine whether this flag is set comes from the N_OPT field, which 
   // seems to come only once per image.  The kludge is that we assume that all
   // fortran sources in the module have this property (they probably do, but
   // could conceivably be mixed (???)).
   int fortran_kludge_flag = 0;

   // "state variables" we use to accumulate potentially useful information
   //  A final module<->language decision is not made until we have arrived at the
   //  next module entry, at which point we use any and all info we have to 
   //  make the most sensible guess
   pdstring working_module;
   supportedLanguages working_lang = lang_Unknown;
   char *working_options = NULL, *working_name = NULL;

   struct stab_entry *stabptr = NULL;
#if defined(TIMED_PARSE)
   struct timeval starttime;
   gettimeofday(&starttime, NULL);
#endif

   //Using the Object to get the pointers to the .stab and .stabstr
   // XXX - Elf32 specific needs to be in seperate file -- jkh 3/18/99
   linkedFile.get_stab_info((void **) &stabptr, stab_nsyms, 
                            (void **) &stabstr_nextoffset);

   for(int i=0;i<stab_nsyms;i++){
      if (stabptr[i].type == N_UNDF) {/* start of object file */
         /* value contains offset of the next string table for next module */
         // assert(stabptr[i].name == 1);
         stabstrs = stabstr_nextoffset;
         stabstr_nextoffset = const_cast<char *>(stabstrs + stabptr[i].val);
      }
      else if (stabptr[i].type == N_OPT){
         //  We can use the compiler option string (in a pinch) to guess at the source file language
         //  There is possibly more useful information encoded somewhere around here, but I lack
         //  an immediate reference....
         if (working_name)
            working_options = const_cast<char *> (&stabstrs[stabptr[i].name]); 
      }
      else if ((stabptr[i].type == N_SO)  || (stabptr[i].type == N_ENDM)){ /* compilation source or file name */
         // We have arrived at the next source file, finish up with the last one and reset state
         // before starting next


         //   XXXXXXXXXXX  This block is mirrored near the end of routine, if you edit it,
         //   XXXXXXXXXXX  change it there too.
         if  (working_name) {
            working_lang = pickLanguage(working_module, working_options, working_lang);
            if (working_lang == lang_Fortran_with_pretty_debug)
               fortran_kludge_flag = 1;
            (*mod_langs)[working_module] = working_lang;

         }
         //   XXXXXXXXXXX
	
         // reset "state" here
         working_lang = lang_Unknown;
         working_options = NULL;

         //  Now:  out with the old, in with the new

         if (stabptr[i].type == N_ENDM) {
            // special case:
            // which is most likely both broken (and ignorable ???)
            working_name = "DEFAULT_MODULE";
         }
         else {
            working_name = const_cast<char*>(&stabstrs[stabptr[i].name]);
            ptr = strrchr(working_name, '/');
            if (ptr) {
               ptr++;
               working_name = ptr;
            }
         }
         working_module = pdstring(working_name);

         if (mod_langs->defines(working_module) && (*mod_langs)[working_module] != lang_Unknown) {
            //  we already have a module with this name in the map.  If it has been given
            //  a language assignment (not lang_Unknown), we can just skip ahead
            working_name = NULL;
            working_options = NULL;
            continue;
         } 
         else {
            //cerr << __FILE__ << __LINE__ << ":  Module: " <<working_module<< " has language "<< stabptr[i].desc << endl;  
            switch (stabptr[i].desc) {
              case N_SO_FORTRAN: 
                 working_lang = lang_Fortran;
                 break;
              case N_SO_F90:
                 working_lang = lang_Fortran;  // not sure if this should be different from N_SO_FORTRAN
                 break;
              case N_SO_AS:
                 working_lang = lang_Assembly;
                 break;
              case N_SO_ANSI_C:
              case N_SO_C:
                 working_lang = lang_C;
                 break;
              case N_SO_CC:
                 working_lang = lang_CPlusPlus;
                 break;
              default:
                 //  currently uncovered options are lang_CMFortran, and lang_GnuCPlusPlus
                 //  do we need to make this kind of distinction here?
                 working_lang = lang_Unknown;
                 break;
            }
	
         } 
      } // end N_SO section
#ifdef NOTDEF
      else {
         //  This is here only to trace the parse, for my edification and knowledge, should be removed
         //  Throw away most known symbols here
         if ( (N_FUN != stabptr[i].type) &&
              (N_GSYM != stabptr[i].type) &&
              (N_STSYM != stabptr[i].type) &&
              (N_LCSYM != stabptr[i].type) &&
              (N_ROSYM != stabptr[i].type) &&
              (N_SLINE != stabptr[i].type) &&
              (N_SOL != stabptr[i].type) &&
              (N_ENTRY != stabptr[i].type) &&
              (N_BCOMM != stabptr[i].type) &&
              (N_ECOMM != stabptr[i].type)) {
            char hexbuf[10];
            sprintf(hexbuf, "%p",stabptr[i].type );
            cerr << __FILE__ << __LINE__ << ":  got " << hexbuf << endl;
         }
      }
#endif
   } // for loop

   //  Need to make sure we finish up with the module we were last collecting information 
   //  about

   //   XXXXXXXXXXX  see note above (find the X's)
   if  (working_name) {
      working_lang = pickLanguage(working_module, working_options, working_lang);	  
      if (working_lang == lang_Fortran_with_pretty_debug)
         fortran_kludge_flag = 1;
      (*mod_langs)[working_module] = working_lang;
   }
   //   XXXXXXXXXXX

   if (fortran_kludge_flag) {
      // go through map and change all lang_Fortran to lang_Fortran_with_pretty_symtab
      dictionary_hash_iter<pdstring, supportedLanguages> iter(*mod_langs);
      pdstring aname;
      supportedLanguages alang;
      while (iter.next(aname, alang)) {
         if (lang_Fortran == alang) {
            (*mod_langs)[aname] = lang_Fortran_with_pretty_debug;
            cerr << __FILE__ << __LINE__ << ": UPDATE: lang_Fortran->lang_Fortran_with_pretty_debug.  " << endl;
         }
      }
   }
#if defined(TIMED_PARSE)
   struct timeval endtime;
   gettimeofday(&endtime, NULL);
   unsigned long lstarttime = starttime.tv_sec * 1000 * 1000 + starttime.tv_usec;
   unsigned long lendtime = endtime.tv_sec * 1000 * 1000 + endtime.tv_usec;
   unsigned long difftime = lendtime - lstarttime;
   double dursecs = difftime/(1000 );
   cout << __FILE__ << ":" << __LINE__ <<": getModuleLanguageInfo took "<<dursecs <<" msecs" << endl;
#endif


#endif // stabs platforms

   //
   // eCoff Platforms
   //
#if defined(alpha_dec_osf4_0)

   LDFILE *ldptr;
   pCHDRR symtab;

   ldptr = ldopen((char *)linkedFile.GetFile().c_str(), NULL);
   symtab = SYMTAB(ldptr);
   for (int i = 0; i < symtab->hdr.ifdMax; ++i) {
      pCFDR file = symtab->pcfd + i;
      char *tmp = ((symtab->psym + file->pfd->isymBase)->iss + 
                   (symtab->pss + file->pfd->issBase));
      pdstring modname = (strrchr(tmp, '/') ? strrchr(tmp, '/') + 1 : tmp);

      if (file->pfd->csym && modname.length()) {
         switch (file->pfd->lang) {
           case langAssembler:
              //		fprintf(stderr, "Setting %s to 'lang_Assembly'\n", modname.c_str());
              (*mod_langs)[modname] = lang_Assembly;
              break;

           case langC:
           case langStdc:
              //		fprintf(stderr, "Setting %s to 'lang_C'\n", modname.c_str());
              (*mod_langs)[modname] = lang_C;
              break;

           case langCxx:
           case langDECCxx:
              //		fprintf(stderr, "Setting %s to 'lang_CPlusPlus'\n", modname.c_str());
              (*mod_langs)[modname] = lang_CPlusPlus;
              break;

           case langFortran:
           case langFortran90:
              //		fprintf(stderr, "Setting %s to 'lang_Fortran'\n", modname.c_str());
              (*mod_langs)[modname] = lang_Fortran;
              break;

           default:
              //		fprintf(stderr, "Setting %s to 'lang_Unknown'\n", modname.c_str());
              (*mod_langs)[modname] = lang_Unknown;
         }

      }
   }
   ldaclose(ldptr);

#endif // eCoff Platforms

}

//  setModuleLanguages is only called after modules have been defined.
//  it attempts to set each module's language, information which is needed
//  before names can be demangled.
void image::setModuleLanguages(dictionary_hash<pdstring, supportedLanguages> *mod_langs)
{
  if (!mod_langs->size()) return;  // cannot do anything here
  //  this case will arise on non-stabs platforms until language parsing can be introduced at this level
  pdvector<pdmodule *> *modlist;
  pdmodule *currmod = NULL;
#ifdef BPATCH_LIBRARY
  modlist = &_mods;
#else
  modlist = &allMods;
#endif
  //int dump = 0;

  for (unsigned int i = 0;  i < modlist->size(); ++i) {
    currmod = (*modlist)[i];
    supportedLanguages currLang;

    if (currmod->isShared()) continue;  // need to find some way to get shared object languages?
    if(mod_langs->find(currmod->fileName(), currLang)) {
      currmod->setLanguage(currLang);
    }
    else {
      //cerr << __FILE__ << __LINE__ << ":  module " << currmod->fileName() 
      //	   << " not found in module<->language map" << endl;
      //dump = 1;
      //  here we should probably try to guess, based on filename conventions
    }
  }
#ifdef NOTDEF
  // REMOVE!!  this is for debuggering
  if (dump) {
    dictionary_hash_iter<pdstring, supportedLanguages> iter(*mod_langs);
    pdstring aname;
    supportedLanguages alang;
    cerr << __FILE__ << __LINE__ << ": contents of module<->language map:" << endl;
    while (iter.next(aname, alang)) {
      cerr << aname << " : " << alang << endl;
    }
  }
#endif
}

// buildFunctionMaps() iterates through pd_Functions and constructs demangled names.
// Demangling was moved here (names used to be demangled as pd_Functions were
// built) so that language information could be obtained _after_ the functions and modules
// were built, but before name demangling takes place.  Thus we can use language information
// during the demangling process.
//
// After name demangling is done, each function's inst points are found and the function
// is classified as either instrumentable or non-instrumentable and filed accordingly 
bool image::buildFunctionMaps(pdvector<pd_Function *> *raw_funcs)
{
  //cerr << "buldFunctionMaps" << endl;
  pd_Function *pdf;
  pdmodule *mod;
  ////cerr << "Inside buildFunctionMaps: raw_funcs->size = " <<raw_funcs->size()<< endl;
  unsigned int num_raw_funcs = raw_funcs->size();
  // build a demangled name for each raw (unclassed) function found in the parse
  for (unsigned int i = 0; i < num_raw_funcs; ++i) {
    assert(NULL != (pdf = (*raw_funcs)[i]));
    assert (NULL != (mod = pdf->file()));
    Address addr = pdf->addr();
    
    pdstring name = pdf->symTabName();
    pdstring mangled_name = name;

    // strip scoping information from mangled name before demangling:
    const char *p = P_strchr(mangled_name.c_str(), ':');
    if (p) {
      unsigned nchars = p - name.c_str();
      mangled_name = pdstring(name.c_str(), nchars);
    }
    
    pdstring demangled;

    if (!buildDemangledName(mangled_name, demangled, nativeCompiler, mod->language())) 
      demangled = mangled_name;

    // let the function in on its new name first
    //  WARNING:  do we need to check for duplicates here???
    pdf->addPrettyName(demangled);

    // check to see that this function is not an alias
    if (funcsByAddr.defines(addr)) {
      // We have already seen a function at this addr. add a second name
      // for this function.  Then delete it
      addMultipleFunctionNames(pdf);
      delete pdf;
      continue;
    }

    // check to see if function is instrumentable
    if (!pdf->findInstPoints(this)) {
      // function is not instrumentable, add to "bad" pile
      addNotInstruFunc(pdf, pdf->file());
      continue;
    }

    // then build up the maps & vectors as appropriate
#ifdef BPATCH_LIBRARY
    addInstruFunction(pdf, mod, addr,false);
#else
    addInstruFunction(pdf, mod, addr,function_is_excluded(pdf, mod->fileName()));
#endif
    
  }

  // can this ever fail???
  return true;
}

// Constructor for the image object. The fileDescriptor simply
// wraps (in the normal case) the object name and a relocation
// address (0 for a.out file). On the following platforms, we
// are handling a special case:
//   AIX: objects can possibly have a name like /lib/libc.so:shr.o
//          since libraries are archives
//        Both text and data sections have a relocation address


image::image(fileDescriptor *desc, bool &err, Address newBaseAddr)
  : 
  desc_(desc),
  is_libdyninstRT(false),
#ifndef BPATCH_LIBRARY
  is_libparadynRT(false),
#endif
  is_a_out(false),
  main_call_addr_(0),
  nativeCompiler(false),    
  linkedFile(desc, newBaseAddr,pd_log_perror),//ccw jun 2002
  knownJumpTargets(int_addrHash, 8192),
#ifndef BPATCH_LIBRARY
  includedFunctions(0),
  excludedFunctions(pdstring::hash),
  includedMods(0),
  excludedMods(0),
  allMods(0),
#else
  _mods(0),
#endif
  instrumentableFunctions(0),
  knownSymAddrs(addrHash4),
  funcsByAddr(addrHash4),
  funcsByPretty(pdstring::hash),
  funcsByMangled(pdstring::hash),
  notInstruFunctions(pdstring::hash),
  modsByFileName(pdstring::hash),
  modsByFullName(pdstring::hash),
  varsByPretty(pdstring::hash),
  refCount(1)
{
  err = false;
  sharedobj_cerr << "image::image for file name="
		 << desc->file() << endl;

  name_ = extract_pathname_tail(desc->file());

  pathname_ = desc->file();

  // on some platforms (e.g. Windows) we try to parse
  // the image too soon, before we have a process we can
  // work with reliably.  If so, we must recognize it
  // and reparse at some later time.
  if( linkedFile.have_deferred_parsing() )
  {
    // nothing else to do here
    return;
  }

  // initialize (data members) codeOffset_, dataOffset_,
  //  codeLen_, dataLen_.
  codeOffset_ = linkedFile.code_off();
  dataOffset_ = linkedFile.data_off();
  codeLen_ = linkedFile.code_len();
  dataLen_ = linkedFile.data_len();
    
  // if unable to parse object file (somehow??), try to
  //  notify luser/calling process + return....    
  if (!codeLen_ || !linkedFile.code_ptr()) {
    pdstring msg = pdstring("Parsing problem with executable file: ") + desc->file();
    statusLine(msg.c_str());
    msg += "\n";
    logLine(msg.c_str());
    err = true;
#ifndef mips_unknown_ce2_11 //ccw 29 mar 2001

#if defined(BPATCH_LIBRARY)
    BPatch_reportError(BPatchWarning, 27, msg.c_str()); 
#else
    showErrorCallback(27, msg); 
#endif
#endif
    return; 
  }
  

  pdstring msg;
  // give luser some feedback....
  msg = pdstring("Parsing object file: ") + desc->file();

  statusLine(msg.c_str());

  // use the *DUMMY_MODULE* until a module is defined
  //pdmodule *dynModule = newModule(DYN_MODULE, 0);
  //pdmodule *libModule = newModule(LIBRARY_MODULE, 0);
  // TODO -- define inst points in define function ?
  
  // The functions cannot be verified until all of them have been seen
  // because calls out of each function must be tagged as calls to user
  // functions or call to "library" functions
  
  //
  // sort the modules by address into a vector to allow a binary search to 
  // determine the module that a symbol will map to -- this 
  // may be bsd specific....
  //
  pdvector <Symbol> tmods;
  
  for (SymbolIter symIter(linkedFile); symIter; symIter++) {
    const Symbol &lookUp = symIter.currval();
    if (lookUp.type() == Symbol::PDST_MODULE) {
      
      const pdstring &lookUpName = lookUp.name();
      const char *str = lookUpName.c_str();
      assert(str);
      int ln = lookUpName.length();
      
      // directory definition -- ignored for now
      if (str[ln-1] != '/') {
	tmods.push_back(lookUp);
      }
    }
    // As a side project, fill knownSymAddrs
    knownSymAddrs[lookUp.addr()] = 0; // 0 is a dummy value
  }
  
  // sort the modules by address
  statusLine("sorting modules");
  VECTOR_SORT(tmods, symbol_compare);
  
  // remove duplicate entries -- some .o files may have the same 
  // address as .C files.  kludge is true for module symbols that 
  // I am guessing are modules
  pdvector<Symbol> uniq;

  unsigned int num_zeros = 0;
  // must use loop+1 not mods.size()-1 since it is an unsigned compare
  //  which could go negative - jkh 5/29/95
  for (unsigned loop=0; loop < tmods.size(); loop++) {
    if (tmods[loop].addr() == 0) num_zeros++;
    if ((loop+1 < tmods.size()) && 
	(tmods[loop].addr() == tmods[loop+1].addr())) {
      if (!tmods[loop].kludge())
	tmods[loop+1] = tmods[loop];
    } 
    else
      uniq.push_back(tmods[loop]);
  }
  // avoid case where all (ELF) module symbols have address zero
  if (num_zeros == tmods.size()) uniq.resize(0);



#if defined(sparc_sun_solaris2_4) || defined(i386_unknown_solaris2_5) || defined(rs6000_ibm_aix4_1)
  // make sure we're using the right demangler

  nativeCompiler = parseCompilerType(&linkedFile);

#endif

  // define all of the functions
  statusLine("winnowing functions");
  
  // a vector to hold all created functions until they are properly classified
  pdvector<pd_Function *> raw_funcs; 

  // define all of the functions, this also defines all of the moldules
  if (!addAllFunctions(uniq, &raw_funcs)) {
    err = true;
    return;
  }

  // wait until all modules are defined before applying languages to them
  // we want to do it this way so that module information comes from the function
  // symbols, first and foremost, to avoid any internal module-function mismatching.

  // get Information on the language each modules is written in (prior to making modules)
  dictionary_hash<pdstring, supportedLanguages> mod_langs(pdstring::hash);
  getModuleLanguageInfo(&mod_langs);
  setModuleLanguages(&mod_langs);

  // Once languages are assigned, we can build demangled names (in the wider sense of demangling
  // which includes stripping _'s from fortran names -- this is why language information must
  // be determined before this step).
  
  if (!buildFunctionMaps(&raw_funcs)) {
    err = true;
    return;
  }

  // ALERT ALERT - Calling on both shared_object && !shared_object path.
  //  In origional code, only called in !shared_object case.  Was this 
  //  a bug????
  // XXX should have a statusLine("retrieving variable information") here,
  //     but it's left out for now since addAllVariables only does something
  //     when BPATCH_LIBRARY is defined
  addAllVariables();
  
#ifdef CHECK_ALL_CALL_POINTS
  statusLine("checking call points");
  checkAllCallPoints();
#endif

  // TODO -- remove duplicates -- see earlier note
  dictionary_hash<Address, unsigned> addr_dict(addrHash4);
  pdvector<pd_Function*> temp_vec;
  
  // question??  Necessary to do same crap to includedFunctions &&
  //  excludedFunctions??
  unsigned f_size = instrumentableFunctions.size(), index;
  for (index=0; index<f_size; index++) {
    const Address the_address = 
      instrumentableFunctions[index]->getAddress(0);
    if (!addr_dict.defines(the_address)) {
      addr_dict[the_address] = 1;
      temp_vec.push_back(instrumentableFunctions[index]);
    }
  }
  // Memory leak, eh?
  instrumentableFunctions = temp_vec;
}

void pdmodule::updateForFork(process *childProcess, 
			     const process *parentProcess) {
  for(unsigned i=0; i<funcs.size(); i++) {
    funcs[i]->updateForFork(childProcess, parentProcess);
  }
  for(unsigned j=0; j<notInstruFuncs.size(); j++) {
    notInstruFuncs[j]->updateForFork(childProcess, parentProcess);
  }
#ifndef BPATCH_LIBRARY
  for(unsigned k=0; k<some_funcs.size(); k++) {
    some_funcs[k]->updateForFork(childProcess, parentProcess);
  }
#endif
}

#ifdef CHECK_ALL_CALL_POINTS
void pdmodule::checkAllCallPoints() {
#if defined(TIMED_PARSE)
  struct timeval starttime;
  gettimeofday(&starttime, NULL);
#endif
  unsigned fsize = funcs.size();
  for (unsigned f=0; f<fsize; f++)
      funcs[f]->checkCallPoints();
#if defined(TIMED_PARSE)
  struct timeval endtime;
  gettimeofday(&endtime, NULL);
  unsigned long lstarttime = starttime.tv_sec * 1000 * 1000 + starttime.tv_usec;
  unsigned long lendtime = endtime.tv_sec * 1000 * 1000 + endtime.tv_usec;
  unsigned long difftime = lendtime - lstarttime;
  double dursecs = difftime/(1000 );
  cout << __FILE__ << ":" << __LINE__ <<": checkAllCallPoints took "<<dursecs <<" msecs" << endl;
#endif
}
#endif

#define PDMODULE_FIND(name, src, dest, type) {  \
  for (unsigned int f=0; f<src.size(); f++) {\
    pdvector<pdstring> funcNames = src[f]->type##NameVector(); \
    for (unsigned i = 0; i < funcNames.size(); i++) \
      if (funcNames[i] == name) {\
	dest->push_back(src[f]); break;}\
  }}

inline pdvector<function_base *> *
pdmodule::findFunction(const pdstring &name, pdvector<function_base *> *found)
{
  // pretty first
  PDMODULE_FIND(name, funcs, found, pretty)
  if (found->size()) return found;

  // then mangled
  PDMODULE_FIND(name, funcs, found, symTab)
  if (found->size()) return found;

  return NULL;
}

#if !defined(i386_unknown_nt4_0) && !defined(mips_unknown_ce2_11) // no regex for M$
//  This big ugly macro performs a regex search of src, and puts matches in dest.
//  type is either "pretty" or "symTab" and specifies which vectors of
//  names are used.  This is here because otherwise there'd be a lot of search
//  functions here with nearly the same code.

#define REGEX_FIND(pat, src, dest, type) { \
  for (unsigned int i = 0; i < src.size(); ++i) { \
    pdvector<pdstring> funcNames = src[i]->type##NameVector(); \
    for (unsigned int j =  0; j <funcNames.size(); ++j) { \
      int err; \
      if (0 == (err = regexec( (regex_t *)pat, funcNames[j].c_str(), 1, NULL, 0 ))){ \
	dest->push_back(src[i]); \
	break; \
      } \
      if (REG_NOMATCH == err) \
	continue; \
      char errbuf[80]; \
      regerror( err, (regex_t *)pat, errbuf, 80 ); \
      cerr << __FILE__ << ":" << __LINE__ << ":  REGEXEC ERROR: "<< errbuf << endl; \
      return NULL; \
    } \
  } }
#endif

pdvector<function_base *> *
pdmodule::findFunctionFromAll(const pdstring &name,
			      pdvector<function_base *> *found, 
			      bool regex_case_sensitive) 
{
  if (NULL == strpbrk(name.c_str(), REGEX_CHARSET)) {

    if (NULL != findFunction(name, found) 
	&& found->size())
      return found;

    // could not find in instrumentable, check non-instrumentable
    PDMODULE_FIND(name, notInstruFuncs, found, pretty);
    if (found->size()) return found;

    PDMODULE_FIND(name, notInstruFuncs, found, symTab);
    if (found->size()) return found;
  
    return NULL;
  }

#if !defined(i386_unknown_nt4_0) && !defined(mips_unknown_ce2_11) // no regex for M$
  // regex falls through
  regex_t comp_pat;
  int err, cflags = REG_NOSUB | REG_EXTENDED;
  
  if( !regex_case_sensitive )
    cflags |= REG_ICASE;
  
  if (0 != (err = regcomp( &comp_pat, name.c_str(), cflags ))) {
    char errbuf[80];
    regerror( err, &comp_pat, errbuf, 80 );
    cerr << __FILE__ << ":" << __LINE__ << ":  REGEXEC ERROR: "<< errbuf << endl;
    return NULL;
  }

  REGEX_FIND(&comp_pat, funcs, found, pretty);
  if (found->size()) goto regex_done;

  REGEX_FIND(&comp_pat, funcs, found, symTab);
  if (found->size()) goto regex_done;

  // no matches found yet, check notInstruFuncs

  REGEX_FIND(&comp_pat, notInstruFuncs, found, pretty);
  if (found->size()) goto regex_done;

  REGEX_FIND(&comp_pat, notInstruFuncs, found, symTab);
  if (found->size()) goto regex_done;

 regex_done:
  regfree(&comp_pat);
  if (found->size()) return found;
#endif
  
  return NULL;
}

function_base *pdmodule::findFunctionByMangled(const pdstring &name)
{
  pdvector<function_base *> found;
  pdvector<function_base *> *foundp = &found;

  PDMODULE_FIND(name, funcs, foundp, symTab);
  if (!found.size()) return NULL;
  if (found.size() > 1) return NULL; // fail if more than one found 
  return found[0];

}

void pdmodule::dumpMangled(char * prefix) 
{
  cerr << fileName() << "::dumpMangled("<< prefix << "): " << endl;
  for (unsigned int i = 0; i < funcs.size(); ++i) {
    if (prefix) {
      if (!strncmp(funcs[i]->symTabName().c_str(), prefix, strlen(prefix) -1))
	cerr << funcs[i]->symTabName() << " ";
      else {
	//printf("%s is not a prefix of %s\n", prefix, funcs[i]->symTabName().c_str());
      }
    }
    else
      cerr << funcs[i]->symTabName() << " ";
  }
  cerr << endl;
}

pdvector<function_base *> *pdmodule::findUninstrumentableFunction(const pdstring &name,
								 pdvector<function_base *> *found)
{
  PDMODULE_FIND(name, notInstruFuncs, found, pretty);
  return found;
}

// remove record of function from internal vector of instrumentable funcs
// (used when a function needs to be reclassified as non-instrumentable);
bool pdmodule::removeInstruFunc(pd_Function *pdf)
{
  bool ret = false;
  unsigned int i;

  for (i = 0; i < funcs.size(); ++i) {
    if (funcs[i] == pdf) {
      funcs[i] = funcs[funcs.size() -1];
      funcs.pop_back();
      ret = true;
      break;
    }
  }

#ifndef BPATCH_LIBRARY
  for (i = 0; i < some_funcs.size(); ++i) {
    if (some_funcs[i] == pdf) {
      some_funcs[i] = some_funcs[some_funcs.size() -1];
      some_funcs.pop_back();
      break;
    }
  }
#endif
  return ret;
}
#ifdef CHECK_ALL_CALL_POINTS
void image::checkAllCallPoints() {
  dictionary_hash_iter<pdstring, pdmodule*> di(modsByFullName);
  pdstring s; pdmodule *mod;
  while (di.next(s, mod))
    mod->checkAllCallPoints();
}
#endif
pdmodule *image::getOrCreateModule(const pdstring &modName, 
				   const Address modAddr) {
  const char *str = modName.c_str();
  int len = modName.length();
  assert(len>0);

  // TODO ignore directory definitions for now
  if (str[len-1] == '/') {
    return NULL;
  } else {
    // TODO - navigate ".." and "."
    const char *lastSlash = P_strrchr(str, '/');
    if (lastSlash)
      return (newModule(++lastSlash, modAddr, lang_Unknown));
    else
      return (newModule(modName, modAddr, lang_Unknown));
  }
}

// Verify that this is code
// Find the call points
// Find the return address
// TODO -- use an instruction object to remove
// Sets err to false on error, true on success
//
// Note - this must define funcEntry and funcReturn
// 
pd_Function::pd_Function(const pdstring &symbol,
			 pdmodule *f, Address adr, const unsigned size) :
  function_base(symbol, adr, size),
  file_(f),
  funcEntry_(0),
#ifndef BPATCH_LIBRARY
  funcResource(0),
#endif
  relocatable_(false),
  call_points_have_been_checked(false)
{
  //  We used to call findInstPoints here, but since this function sometimes makes
  //  use of a pretty name to determine if the function should not be instrumented
  //  it had to be relocated to a later point in the parsing process...  after 
  //  names have been demangled.  
#ifdef NOTDEF 
 err = findInstPoints(owner) == false;

  isInstrumentable_ = !err;
#endif
  relocatedCode = NULL;
  originalCode = NULL;
  instructions = NULL;
}

// This method returns the address at which this function resides
// in the process P, even if it is dynamic, even if it has been
// relocated.  getAddress() (see symtab.h) does not always do this:
// If the function is in a shlib and it has not been relocated,
// getAddress() only returns the relative offset of the function
// within its shlib.  We think that getAddress() should be fixed
// to always return the effective address, but we are not sure
// whether that would boggle any of its 75 callers.  Until that is
// cleared up, call this method. -zandy, Apr-26-1999
Address pd_Function::getEffectiveAddress(const process *p) const {
     assert(p);
     // Even if the function has been relocated, call it at its
     // original address since the call will be redirected to the
     // right place anyway.
     Address base;
     p->getBaseAddress(file()->exec(), base);
     return base + addr();
}

void pd_Function::updateForFork(process *childProcess, 
				const process *parentProcess) {
  if(relocatable_) {
    for(u_int i=0; i < relocatedByProcess.size(); i++) {
      if((relocatedByProcess[i])->getProcess() == parentProcess) {
	  relocatedFuncInfo *childRelocInfo = 
	    new relocatedFuncInfo(*relocatedByProcess[i]);
	  childRelocInfo->setProcess(childProcess);
	  relocatedByProcess.push_back(childRelocInfo);
      }
    }
  }
}


/*********************************************************************/
/**** Function lookup (by name or address) routines               ****/
/*********************************************************************/

/* Look up a function by the given address. Four-part method:
 * First, do a quick scan of funcsByAddr to see if we get a match
 *   -- will match function entry point
 * Second, do a complete search of funcsByAddr, using original address
 * Finally, check excludedFuncs and nonInstruFuncs
 */

pd_Function *image::findFuncByAddr(const Address &addr, 
				   const process *p) const
{
  pd_Function *pdf;

  // Quick check of funcsByAddr
  if (funcsByAddr.find(addr, pdf))
    return pdf;
  
  // Slow check of funcsByAddr
  dictionary_hash_iter<Address, pd_Function*> mi(funcsByAddr);
  Address adr;
  while (mi.next(adr, pdf)) {
    if ( p && // getEffectiveAddress asserts p 
	(addr>=pdf->getEffectiveAddress(p)) &&
	 (addr<(pdf->getEffectiveAddress(p)+pdf->size()))
	 )
      return pdf;
    if ( (addr>=pdf->getAddress(p)) && 
	 (addr < (pdf->getAddress(p)+pdf->size()))
	 )
      return pdf;
  }

  pdstring str;
#ifndef BPATCH_LIBRARY
  // next, look in excludedFunctions...
  dictionary_hash_iter<pdstring, pd_Function*> ex(excludedFunctions);
 
  while (ex.next(str, pdf)) {
    if ( p &&
	 (addr>=pdf->getEffectiveAddress(p)) &&
	 (addr<(pdf->getEffectiveAddress(p)+pdf->size()))
	 )
      return pdf;
    if ( (addr>=pdf->getAddress(p)) && 
	 (addr < (pdf->getAddress(p)+pdf->size()))
	 )
      return pdf;
  }
#endif

  dictionary_hash_iter<pdstring, pd_Function *> ni(notInstruFunctions);
  while (ni.next(str, pdf)) {
    if ( p &&
	 (addr>=pdf->getEffectiveAddress(p)) &&
	 (addr<(pdf->getEffectiveAddress(p)+pdf->size()))
	 )
      return pdf;
    if ( (addr>=pdf->getAddress(p)) && 
	 (addr < (pdf->getAddress(p)+pdf->size()))
	 ) 
      return pdf;
  }
  return NULL; 
}

// This function assumes the given address is an offset within
// the file.

pd_Function *image::findFuncByEntryAddr(const Address &addr, 
					const process * /* p */) const
{
  pd_Function *pdf;

  if (funcsByAddr.find(addr, pdf))
    return pdf;

  return NULL; 
}

pd_Function *image::findFuncByRelocAddr(const Address &addr, 
					const process *p) const
{
  pd_Function *pdf;

  // Slow check of funcsByAddr
  dictionary_hash_iter<Address, pd_Function*> mi(funcsByAddr);
  Address adr;
  while (mi.next(adr, pdf)) {
    if ( (addr>=pdf->getAddress(p)) && 
	 (addr < (pdf->getAddress(p)+pdf->size()))
	 )
      return pdf;
  }

  pdstring str;
#ifndef BPATCH_LIBRARY
  // next, look in excludedFunctions...
  dictionary_hash_iter<pdstring, pd_Function*> ex(excludedFunctions);

  while (ex.next(str, pdf)) {
    if ( (addr>=pdf->getAddress(p)) && 
	 (addr < (pdf->getAddress(p)+pdf->size()))
	 )
      return pdf;
  }
#endif

  dictionary_hash_iter<pdstring, pd_Function *> ni(notInstruFunctions);
  while (ni.next(str, pdf)) {
    if ( (addr>=pdf->getAddress(p)) && 
	 (addr < (pdf->getAddress(p)+pdf->size()))
	 ) 
      return pdf;
  }
  return NULL; 
}

pd_Function *image::findFuncByOrigAddr(const Address &addr, 
				       const process *p) const
{
  pd_Function *pdf;
  
  // Slow check of funcsByAddr
  dictionary_hash_iter<Address, pd_Function*> mi(funcsByAddr);
  Address adr;
  while (mi.next(adr, pdf)) {
    if ( p && // getEffectiveAddress asserts p 
	(addr>=pdf->getEffectiveAddress(p)) &&
	 (addr<(pdf->getEffectiveAddress(p)+pdf->size()))
	 )
      return pdf;
  }

  pdstring str;
#ifndef BPATCH_LIBRARY
  // next, look in excludedFunctions...
  dictionary_hash_iter<pdstring, pd_Function*> ex(excludedFunctions);

  while (ex.next(str, pdf)) {
    if ( p &&
	 (addr>=pdf->getEffectiveAddress(p)) &&
	 (addr<(pdf->getEffectiveAddress(p)+pdf->size()))
	 )
      return pdf;
  }
#endif

  dictionary_hash_iter<pdstring, pd_Function *> ni(notInstruFunctions);
  while (ni.next(str, pdf)) {
    if ( p &&
	 (addr>=pdf->getEffectiveAddress(p)) &&
	 (addr<(pdf->getEffectiveAddress(p)+pdf->size()))
	 )
      return pdf;
  }
  return NULL; 
}

// Return the vector of functions associated with a pretty (demangled) name
// Very well might be more than one!

pdvector <pd_Function *> *image::findFuncVectorByPretty(const pdstring &name)
{
#ifdef IBM_BPATCH_COMPAT_STAB_DEBUG
  fprintf(stderr, "%s[%d]:  inside findFuncVectorByPretty\n", __FILE__, __LINE__);
  fflush(NULL);
#endif
  if (funcsByPretty.defines(name))
    return funcsByPretty[name];
  return NULL;
}

// Return a single function associated with a demangled name. Arbitrarily
// pick the first one.

/*
pd_Function *image::findFuncByPretty(const pdstring &name)
{
  pdvector <pd_Function *> *a;

  if (funcsByPretty.defines(name)) {
    a = funcsByPretty[name];
    return ((*a)[0]);
  }
  return NULL;
}
*/

pd_Function *image::findFuncByMangled(const pdstring &name)
{
  pdvector <pd_Function *> *a;

  if (funcsByMangled.defines(name)) {
    a = funcsByMangled[name];
    return ((*a)[0]);
  }
  return NULL;
}

#ifndef BPATCH_LIBRARY
pd_Function *image::findExcludedFunc(const pdstring &name)
{
  if (excludedFunctions.defines(name))
    return excludedFunctions[name];
  return NULL;
}
#endif

pd_Function *image::findNonInstruFunc(const pdstring &name)
{
  if (notInstruFunctions.defines(name)) 
    return notInstruFunctions[name];
  return NULL;
}


// TODO -- this is only being used in cases where only one function
// should exist -- should I assert that the vector size <= 1 ?
// mcheyney - should return NULL when function being searched for 
//  is excluded!!!!
// Checks for function in pretty name hash first, then in
//  mangled name hash to better handle many (mangled name) to one 
//  (pretty name) mapping in C++ function names....
/*
pd_Function *image::findFuncByName(const pdstring &name)
{
  pd_Function *pdf;

  if ( ( pdf = findFuncByPretty(name))) return pdf;

  if ( ( pdf = findFuncByMangled(name))) return pdf;

  return NULL;
}
*/

// This function supposely is only used to find function that
// is not instrumentable which may not be totally defined.
// Use with caution.  
// NEW - also can be used to find an excluded function....
pd_Function *image::findOnlyOneFunctionFromAll(const pdstring &name) {
  
    pd_Function *found,*ret = NULL;
    pdvector<pd_Function *> pdfv;

    if (NULL != (found = findOnlyOneFunction(name)))
      ret = found;

    if (NULL != (found = findNonInstruFunc(name))) {
      // fail if we already found a match
      if (ret) return NULL;
      ret = found;
    }

#ifndef BPATCH_LIBRARY
    if (NULL != (found = findExcludedFunc(name))) {
      // fail if we already found a match
      if (ret) return NULL;
      ret = found;
    }
#endif

    return ret;
}

//  image::findOnlyOneFunction(const pdstring &name)
//  
//  searches for function with name <name> and fails if it finds more than
//  one match.
//
//  In order to be as comprehensive as possible, if no <name> is found in the 
//  "pretty" list, a search on the mangled list is performed.  Due to
//  duplication between many pretty and mangled names, this function does not
//  care about the same name appearing in both the pretty and mangled lists.
pd_Function *image::findOnlyOneFunction(const pdstring &name) {
  pdvector<pd_Function *> *pdfv;
  pd_Function *pdf;

  if (NULL != (pdfv=findFuncVectorByPretty(name)) && pdfv->size()) {
    // fail if more than one match
    if (pdfv->size() > 1) {
      cerr << __FILE__ << ":" << __LINE__ << ": findOnlyOneFunction(" << name
	   << ")...  found more than one... failing... " << endl;
      return NULL;
    }
    return (*pdfv)[0];
  }

  if (NULL != (pdf = findFuncByMangled(name))) {
    return pdf;
  }
  
  return NULL;
}

void image::updateForFork(process *childProcess, const process *parentProcess)
{
#ifdef BPATCH_LIBRARY
  for(unsigned i=0; i<_mods.size(); i++) {
    _mods[i]->updateForFork(childProcess, parentProcess);
  }
#else
  for(unsigned i=0; i<includedMods.size(); i++) {
    includedMods[i]->updateForFork(childProcess, parentProcess);
  }
  for(unsigned j=0; j<excludedMods.size(); j++) {
    excludedMods[j]->updateForFork(childProcess, parentProcess);
  }
#endif
}

#if !defined(i386_unknown_nt4_0) && !defined(mips_unknown_ce2_11) // no regex for M$
int image::findFuncVectorByPrettyRegex(pdvector<pd_Function *> *found, pdstring pattern,
				       bool case_sensitive)
{
  // This function compiles regex patterns and then calls its overloaded variant,
  // which does not.  This behavior is desirable so we can avoid re-compiling
  // expressions for broad searches (across images) -- allowing for higher level
  // functions to compile expressions just once. jaw 01-03

  regex_t comp_pat;
  int err, cflags = REG_NOSUB;
  
  if( !case_sensitive )
    cflags |= REG_ICASE;
  
  if (0 == (err = regcomp( &comp_pat, pattern.c_str(), cflags ))) {
    int ret = findFuncVectorByPrettyRegex(found, &comp_pat); 
    regfree(&comp_pat);
    return ret;  
  }
  
  // errors fall through
  char errbuf[80];
  regerror( err, &comp_pat, errbuf, 80 );
  cerr << __FILE__ << ":" << __LINE__ << ":  REGEXEC ERROR: "<< errbuf << endl;
  
  return -1;
}

int image::findFuncVectorByMangledRegex(pdvector<pd_Function *> *found, pdstring pattern,
					bool case_sensitive)
{
  // Almost identical to its "Pretty" counterpart

  regex_t comp_pat;
  int err, cflags = REG_NOSUB;
  
  if( !case_sensitive )
    cflags |= REG_ICASE;
  
  if (0 == (err = regcomp( &comp_pat, pattern.c_str(), cflags ))) {
    int ret = findFuncVectorByMangledRegex(found, &comp_pat); 
    regfree(&comp_pat);
    return ret;
  }
  // errors fall through
  char errbuf[80];
  regerror( err, &comp_pat, errbuf, 80 );
  cerr << __FILE__ << ":" << __LINE__ << ":  REGEXEC ERROR: "<< errbuf << endl;
  
  return -1;
}
#endif

pdvector<pd_Function *> *image::findFuncVectorByPretty(functionNameSieve_t bpsieve, 
						       void *user_data,
						       pdvector<pd_Function *> *found)
{
  pdvector<pdvector<pd_Function *> *> result;
  //result = funcsByPretty.linear_filter(bpsieve, user_data);
  dictionary_hash_iter <pdstring, pdvector<pd_Function*>*> iter(funcsByPretty);
  pdstring fname;
  pdvector<pd_Function *> *fmatches;
  while (iter.next(fname, fmatches)) {
    if ((*bpsieve)(iter.currkey().c_str(), user_data)) {
      result.push_back(fmatches);
    }
  }

  // Need to consolodate vector of vectors into just one vector  
  // This is wasteful in general, but hopefully result.size() is small
  // Besides, a more efficient approach would probably require different
  // data structs
  for (unsigned int i = 0; i < result.size(); ++i) 
    for (unsigned int j = 0; j < result[i]->size(); ++j) 
      found->push_back((*result[i])[j]);

  if (found->size()) return found;

  return NULL;
}

pdvector<pd_Function *> *image::findFuncVectorByMangled(functionNameSieve_t bpsieve, 
							void *user_data,
							pdvector<pd_Function *> *found)
{
  pdvector<pdvector<pd_Function *> *> result;
  // result = funcsByMangled.linear_filter(bpsieve, user_data);

  dictionary_hash_iter <pdstring, pdvector<pd_Function*>*> iter(funcsByMangled);
  pdstring fname;
  pdvector<pd_Function *> *fmatches;
  while (iter.next(fname, fmatches)) {
    if ((*bpsieve)(iter.currkey().c_str(), user_data)) {
      result.push_back(fmatches);
    }
  }


  // Need to consolodate vector of vectors into just one vector  
  // This is wasteful in general, but hopefully result.size() is small
  // Besides, a more efficient approach would probably require different
  // data structs
  for (unsigned int i = 0; i < result.size(); ++i) 
    for (unsigned int j = 0; j < result[i]->size(); ++j) 
      found->push_back((*result[i])[j]);

  if (found->size()) return found;

  return NULL;
}

#if !defined(i386_unknown_nt4_0) && !defined(mips_unknown_ce2_11) // no regex for M$
bool regex_filter_func(const pdstring &test, void *comp_pat)
{

  int err;

  if (0 == (err = regexec( (regex_t *)comp_pat, test.c_str(), 1, NULL, 0 ))){
    //cerr << test.c_str() << "does match" <<endl;
    return TRUE; // no error, match
  }
  if (REG_NOMATCH == err) {
    //cerr << test.c_str() << "does not match" <<endl;
    return FALSE; // no error, but no match
  }
  char errbuf[80];
  regerror( err, (regex_t *)comp_pat, errbuf, 80 );
  cerr << __FILE__ << ":" << __LINE__ << ":  REGEXEC ERROR: "<< errbuf << endl;

  return FALSE; // error, no match
}

int image::findFuncVectorByPrettyRegex(pdvector<pd_Function *> *found, regex_t *comp_pat)
{
  //  This is a bit ugly in that it has to iterate through the entire dict hash 
  //  to find matches.  If this turns out to be a popular way of searching for
  //  functions (in particular "name*"), we might consider adding a data struct that
  //  preserves the pdstring ordering realation to make this O(1)-ish in that special case.
  //  jaw 01-03

  pdvector<pdvector<pd_Function *> *> result;
  //  result = funcsByPretty.linear_filter(&regex_filter_func, (void *) comp_pat);

  dictionary_hash_iter <pdstring, pdvector<pd_Function*>*> iter(funcsByPretty);
  pdstring fname;
  pdvector<pd_Function *> *fmatches;
  while (iter.next(fname, fmatches)) {
    if ((*regex_filter_func)(iter.currkey().c_str(), comp_pat)) {
      result.push_back(fmatches);
    }
  }

  // Need to consolodate vector of vectors into just one vector  
  // This is wasteful in general, but hopefully result.size() is small
  // Besides, a more efficient approach would probably require different
  // data structs
  cerr <<"pretty regex result.size() = " << result.size() <<endl;
  for (unsigned int i = 0; i < result.size(); ++i) 
    for (unsigned int j = 0; j < result[i]->size(); ++j) 
      found->push_back((*result[i])[j]);
  
  return found->size();
}

int image::findFuncVectorByMangledRegex(pdvector<pd_Function *> *found, regex_t *comp_pat)
{
  // almost identical to its "Pretty" counterpart.

  pdvector<pdvector<pd_Function *> *> result;
  //result = funcsByMangled.linear_filter(&regex_filter_func, (void *) comp_pat);
  //cerr <<"mangled regex result.size() = " << result.size() <<endl;
  dictionary_hash_iter <pdstring, pdvector<pd_Function*>*> iter(funcsByMangled);
  pdstring fname;
  pdvector<pd_Function *> *fmatches;
  while (iter.next(fname, fmatches)) {
    if ((*regex_filter_func)(iter.currkey().c_str(), comp_pat)) {
      result.push_back(fmatches);
    }
  }

  for (unsigned int i = 0; i < result.size(); ++i) 
    for (unsigned int j = 0; j < result[i]->size(); ++j) 
      found->push_back((*result[i])[j]);
  
  return found->size();
}
#endif // !windows

pdmodule *image::findModule(function_base *func) {
#ifndef BPATCH_LIBRARY
  for(unsigned i=0; i<includedMods.size(); i++) {
    pdmodule *curMod = includedMods[i];
    pdvector<function_base *> bpfv;
    if (NULL != curMod->findFunction(func->prettyName(), &bpfv) && bpfv.size()) 
      return curMod;
  }
#else
  for(unsigned i=0; i<_mods.size(); i++) {
    pdvector<function_base *> bpfv;
    if (NULL != _mods[i]->findFunction(func->prettyName(), &bpfv) && bpfv.size()) 
      return _mods[i];
  }
#endif
  return NULL;
}

// Returns TRUE if module belongs to a shared library, and FALSE otherwise

bool pdmodule::isShared() const { 
  return !exec_->isAOut();
}


pdstring* pdmodule::processDirectories(pdstring* fn){
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


LineInformation*
pdmodule::getLineInformation(process *proc)
{
#if !defined(mips_sgi_irix6_4) && \
    !defined(alpha_dec_osf4_0) && \
    !defined(i386_unknown_nt4_0)  
  if (!lineInformation) parseFileLineInfo(proc);
#endif

  if (!lineInformation)
    cerr << __FILE__ << ":" << __LINE__ << ": lineInfo == NULL" << endl;
  return lineInformation;
}

void pdmodule::initLineInformation()
{
    lineInformation = new LineInformation(fileName());
}

void pdmodule::cleanupLineInformation()
{
    lineInformation->cleanEmptyFunctions();
}

pdmodule::~pdmodule()
{
    if(lineInformation) delete lineInformation;
}

// Parses symtab for file and line info. Should not be called before
// parseTypes. The ptr to lineInformation should be NULL before this is called.
#if !defined(rs6000_ibm_aix4_1) && !defined(mips_sgi_irix6_4) && !defined(alpha_dec_osf4_0) && !defined(i386_unknown_nt4_0)
void pdmodule::parseFileLineInfo(process * proc) 
{
  int i;
  char *modName;
  image * imgPtr=NULL;
  char *ptr;
  int stab_nsyms;
  char *stabstr_nextoffset;
  const char *stabstrs = 0;
  struct stab_entry *stabptr = NULL;
  int parseActive = false; 

  if (lineInformation) {
    cerr << __FILE__ << ":" << __LINE__ << ": Internal error, not fatal, probabl:, duplicated call to"
	 << "parseFileLineInfo()...  ignoring" << endl;
  }

  initLineInformation();
 

#if defined(TIMED_PARSE)
  struct timeval starttime;
  gettimeofday(&starttime, NULL);
  unsigned int fun_count = 0;
  double fun_dur = 0;
  unsigned int src_count = 0;
  double src_dur = 0;
  unsigned int sol_count = 0;
  double sol_dur = 0;
  unsigned int sline_count = 0;
  double sline_dur = 0;
  struct timeval t1, t2;
#endif

  imgPtr = exec();

  const Object &objPtr = imgPtr->getObject();

  //Using the Object to get the pointers to the .stab and .stabstr
  // XXX - Elf32 specific needs to be in seperate file -- jkh 3/18/99
  objPtr.get_stab_info((void **) &stabptr, stab_nsyms, 
		       (void **) &stabstr_nextoffset);


  //these variables are used to keep track of the source files
  //and function names being processes at a moment

  pdstring* currentFunctionName = NULL;
  Address currentFunctionBase = 0;
  pdstring* currentSourceFile = NULL;
  pdstring* absoluteDirectory = NULL;
  FunctionInfo* currentFuncInfo = NULL;
  FileLineInformation* currentFileInfo = NULL;

  for(i=0;i<stab_nsyms;i++){
    // if (stabstrs) printf("parsing #%d, %s\n", stabptr[i].type, &stabstrs[stabptr[i].name]);
    switch(stabptr[i].type){

    case N_UNDF: /* start of object file */
	    /* value contains offset of the next string table for next module */
	    // assert(stabptr[i].name == 1);
	    stabstrs = stabstr_nextoffset;
	    stabstr_nextoffset = const_cast<char *>(stabstrs) + stabptr[i].val;

	    //N_UNDF is the start of object file. It is time to 
	    //clean source file name at this moment.
	    if(currentSourceFile){
	  	delete currentSourceFile;
		currentSourceFile = NULL;
		delete absoluteDirectory;
		absoluteDirectory = NULL;
		currentFileInfo = NULL;
		currentFuncInfo = NULL;
	    }
	    break;

    case N_SO: /* compilation source or file name */
      /* printf("Resetting CURRENT FUNCTION NAME FOR NEXT OBJECT FILE\n");*/
#ifdef TIMED_PARSE
      src_count++;
      gettimeofday(&t1, NULL);
#endif
      //residue from separating parseLineInfo from parseTypes?
      //current_func_name = NULL; // reset for next object file
      //current_mangled_func_name = NULL; // reset for next object file
      //current_func = NULL;

	    //  JAW -- not sure we need this block here
            modName = const_cast<char *>(&stabstrs[stabptr[i].name]);
            ptr = strrchr(modName, '/');
            if (ptr) {
                ptr++;
		modName = ptr;
	    }

	    if (!strcmp(modName, fileName().c_str())) 
	      parseActive = true;
	    else
	      parseActive = false;
	    //time to create the source file name to be used
	    //for latter processing of line information
	    if(!currentSourceFile){
		currentSourceFile = new pdstring(&stabstrs[stabptr[i].name]);
	    	absoluteDirectory = new pdstring(*currentSourceFile);
	    }
	    else if(!strlen(&stabstrs[stabptr[i].name])){
		delete currentSourceFile;
		currentSourceFile = NULL;
		delete absoluteDirectory;
		absoluteDirectory = NULL;
	    }
	    else
		*currentSourceFile += &stabstrs[stabptr[i].name];

	    currentSourceFile = processDirectories(currentSourceFile);
 

#ifdef TIMED_PARSE
	    gettimeofday(&t2, NULL);
	    src_dur += (t2.tv_sec - t1.tv_sec)*1000.0 + (t2.tv_usec - t1.tv_usec)/1000.0;
	    //src_dur += (t2.tv_sec/1000 + t2.tv_usec*1000) - (t1.tv_sec/1000 + t1.tv_usec*1000) ;
#endif
           break;
    default:
      break;
    }

    if( parseActive || isShared())
      switch(stabptr[i].type){
      case N_SOL:
#ifdef TIMED_PARSE
	sol_count++;
	gettimeofday(&t1, NULL);
#endif
	if(absoluteDirectory){
	  const char* newSuffix = &stabstrs[stabptr[i].name];
	  if(newSuffix[0] == '/'){
	    delete currentSourceFile;
	    currentSourceFile = new pdstring;
	  }
	  else{
	    char* tmp = new char[absoluteDirectory->length()+1];
	    strcpy(tmp,absoluteDirectory->c_str());
	    char* p=strrchr(tmp,'/');
	    if(p) 
	      *(++p)='\0';
	    delete currentSourceFile;
	    currentSourceFile = new pdstring(tmp);
	    delete[] tmp;
	  }
	  (*currentSourceFile) += newSuffix;
	  currentSourceFile = processDirectories(currentSourceFile);
	  if(currentFunctionName)
	    lineInformation->insertSourceFileName(
						  *currentFunctionName,
						  *currentSourceFile,
						  &currentFileInfo,&currentFuncInfo);
	}
	else{
	  currentSourceFile = new pdstring(&stabstrs[stabptr[i].name]);
	  currentSourceFile = processDirectories(currentSourceFile);
	  if(currentFunctionName)
	    lineInformation->insertSourceFileName(
						  *currentFunctionName,
						  *currentSourceFile,
						  &currentFileInfo,&currentFuncInfo);
	}
#ifdef TIMED_PARSE
	gettimeofday(&t2, NULL);
	sol_dur += (t2.tv_sec - t1.tv_sec)*1000.0 + (t2.tv_usec - t1.tv_usec)/1000.0;
	//sol_dur += (t2.tv_sec/1000 + t2.tv_usec*1000) - (t1.tv_sec/1000 + t1.tv_usec*1000); 
#endif
	break;
      case N_SLINE:
#ifdef TIMED_PARSE
	sline_count++;
	gettimeofday(&t1, NULL);
#endif
	//if the stab information is a line information
	//then insert an entry to the line info object
	if(!currentFunctionName) break;
	if(currentFileInfo)
	  currentFileInfo->insertLineAddress(currentFuncInfo,
					     stabptr[i].desc,
					     stabptr[i].val+currentFunctionBase);
#ifdef TIMED_PARSE
	gettimeofday(&t2, NULL);
	sline_dur += (t2.tv_sec - t1.tv_sec)*1000.0 + (t2.tv_usec - t1.tv_usec)/1000.0;
	//sline_dur += (t2.tv_sec/1000 + t2.tv_usec*1000) - (t1.tv_sec/1000 + t1.tv_usec*1000) ;
#endif
	break;
    case N_FUN:
#ifdef TIMED_PARSE
      fun_count++;
      gettimeofday(&t1, NULL);
#endif
      //residue from separating parseLineInfo from parseTypes?
      //current_func = NULL;

      //if it is a function stab then we have to insert an entry 
      //to initialize the entries in the line information object
      int currentEntry = i;
      int funlen = strlen(&stabstrs[stabptr[currentEntry].name]);
      ptr = new char[funlen+1];
      strcpy(ptr,(const char *)&stabstrs[stabptr[currentEntry].name]);
      while(strlen(ptr) != 0 && ptr[strlen(ptr)-1] == '\\'){
	ptr[strlen(ptr)-1] = '\0';
	currentEntry++;
	strcat(ptr,(const char *)&stabstrs[stabptr[currentEntry].name]);
      }
      
      char* colonPtr = NULL;
      if(currentFunctionName) delete currentFunctionName;
      if(!ptr || !(colonPtr = strchr(ptr,':')))
	currentFunctionName = NULL;
      else {
	char* tmp = new char[colonPtr-ptr+1];
	strncpy(tmp,ptr,colonPtr-ptr);
	tmp[colonPtr-ptr] = '\0';
	currentFunctionName = new pdstring(tmp);
	
	currentFunctionBase = 0;
	Symbol info;
	if (!proc->getSymbolInfo(*currentFunctionName,
				 info,currentFunctionBase))
	  {
	    pdstring fortranName = *currentFunctionName + pdstring("_");
	    if (proc->getSymbolInfo(fortranName,info,
				    currentFunctionBase))
	      {
		delete currentFunctionName;
		currentFunctionName = new pdstring(fortranName);
	      }
	  }
	
	currentFunctionBase += info.addr();
	
	delete[] tmp;		
	if(currentSourceFile)
	  lineInformation->insertSourceFileName(
						*currentFunctionName,
						*currentSourceFile,
						&currentFileInfo,&currentFuncInfo);
      }
      delete[] ptr;
#ifdef TIMED_PARSE
      gettimeofday(&t2, NULL);
      fun_dur += (t2.tv_sec - t1.tv_sec)*1000.0 + (t2.tv_usec - t1.tv_usec)/1000.0;
      //fun_dur += (t2.tv_sec/1000 + t2.tv_usec*1000) - (t1.tv_sec/1000 + t1.tv_usec*1000);
#endif
      break;
      }
  }

  cleanupLineInformation();

#if defined(TIMED_PARSE)
  struct timeval endtime;
  gettimeofday(&endtime, NULL);
  unsigned long lstarttime = starttime.tv_sec * 1000 * 1000 + starttime.tv_usec;
  unsigned long lendtime = endtime.tv_sec * 1000 * 1000 + endtime.tv_usec;
  unsigned long difftime = lendtime - lstarttime;
  double dursecs = difftime/(1000 );
  cout << __FILE__ << ":" << __LINE__ <<": parseTypes("<< fileName()
       <<") took "<<dursecs <<" msecs" << endl;
  cout << "Breakdown:" << endl;
  cout << "     Functions: " << fun_count << " took " << fun_dur << "msec" << endl;
  cout << "     Sources: " << src_count << " took " << src_dur << "msec" << endl;
  cout << "     SOL?s: " << sol_count << " took " << sol_dur << "msec" << endl;
  cout << "     Sliness: " << sline_count << " took " << sline_dur << "msec" << endl;
  cout << "     Total: " << sline_dur + sol_dur + fun_dur + src_dur 
       << " msec" << endl;
  //lineInformation->print();
  cout << "Max addr per line = " << max_addr_per_line << ".  Max line per addr = "
       << max_line_per_addr << endl;
#endif
}
#endif 



#if defined(rs6000_ibm_aix4_1)

#include <linenum.h>
#include <syms.h>

void 
pdmodule::parseLineInformation(process* proc, 
			       pdstring* currentSourceFile,
			       char* symbolName,
			       SYMENT *sym,
			       Address linesfdptr,char* lines,int nlines)
{
      union auxent *aux;
      pdvector<IncludeFileInfo> includeFiles;

      /* if it is beginning of include files then update the data structure 
         that keeps the beginning of the include files. If the include files contain 
         information about the functions and lines we have to keep it */
      if (sym->n_sclass == C_BINCL){
		includeFiles.push_back(IncludeFileInfo((sym->n_value-linesfdptr)/LINESZ, symbolName));
      }
      /* similiarly if the include file contains function codes and line information
         we have to keep the last line information entry for this include file */
      else if (sym->n_sclass == C_EINCL){
		if (includeFiles.size() > 0) {
			includeFiles[includeFiles.size()-1].end = (sym->n_value-linesfdptr)/LINESZ;
		}
      }
      /* if the enrty is for a function than we have to collect all info
         about lines of the function */
      else if (sym->n_sclass == C_FUN){
		assert(currentSourceFile);
		/* creating the string for function name */
		char* ce = strchr(symbolName,':'); if(ce) *ce = '\0';
		pdstring currentFunctionName(symbolName);

		/* getting the real function base address from the symbols*/
    		Address currentFunctionBase=0;
		Symbol info;
		proc->getSymbolInfo(currentFunctionName,info,
				    currentFunctionBase);
		currentFunctionBase += info.addr();

		/* getting the information about the function from C_EXT */
		int initialLine = 0;
		int initialLineIndex = 0;
		Address funcStartAddress = 0;
		for(int j=-1;;j--){
			SYMENT *extsym = (SYMENT*)(((unsigned)sym)+j*SYMESZ);
			if(extsym->n_sclass == C_EXT){
				aux = (union auxent*)((char*)extsym+SYMESZ);
#ifndef __64BIT__
				initialLineIndex = (aux->x_sym.x_fcnary.x_fcn.x_lnnoptr-linesfdptr)/LINESZ;
#endif
				funcStartAddress = extsym->n_value;
				break;
			}
		}

		/* access the line information now using the C_FCN entry*/
		SYMENT *bfsym = (SYMENT*)(((unsigned)sym)+SYMESZ);

		if (bfsym->n_sclass != C_FCN) {
		    printf("unable to process line info for %s\n", symbolName);
		    return;
		}

		aux = (union auxent*)((char*)bfsym+SYMESZ);
		initialLine = aux->x_sym.x_misc.x_lnsz.x_lnno;

		pdstring whichFile = *currentSourceFile;
		/* find in which file is it */
		for(unsigned int j=0;j<includeFiles.size();j++)
			if((includeFiles[j].begin <= (unsigned)initialLineIndex) &&
			   (includeFiles[j].end >= (unsigned)initialLineIndex)){
				whichFile = includeFiles[j].name;
				break;
			}

		FunctionInfo* currentFuncInfo = NULL;
		FileLineInformation* currentFileInfo = NULL;
		lineInformation->insertSourceFileName(currentFunctionName,whichFile,
						&currentFileInfo,&currentFuncInfo);

		for(unsigned int j=initialLineIndex+1;j<nlines;j++){
			LINENO* lptr = (LINENO*)(lines+j*LINESZ);
			if(!lptr->l_lnno)
				break;
			if(currentFileInfo)
	    			currentFileInfo->insertLineAddress(
					currentFuncInfo,
					lptr->l_lnno+initialLine-1,
					(lptr->l_addr.l_paddr-funcStartAddress)+currentFunctionBase);
		}
      }
}

// This was copied from BPatch_module::parseTypes, the type parsing code was
// removed. When paradyn uses parseTypes this can go away.
void pdmodule::parseFileLineInfo(process* proc)
{
    int i, j;
    int nlines;
    int nstabs;
    char* lines;
    SYMENT *syms;
    SYMENT *tsym;
    char *stringPool;
    char tempName[9];
    char *stabstr=NULL;
    union auxent *aux;
    image * imgPtr=NULL;
    char* funcName = NULL;
    unsigned long linesfdptr;
    pdstring* currentSourceFile = NULL;

    initLineInformation();  

    imgPtr = exec();

    const Object &objPtr = imgPtr->getObject();

    objPtr.get_stab_info(stabstr, nstabs, ((Address&) syms), stringPool); 

    objPtr.get_line_info(nlines,lines,linesfdptr); 

    bool parseActive = true;
    for (i=0; i < nstabs; i++) {
	// do the pointer addition by hand since sizeof(struct syment)
	//   seems to be 20 not 18 as it should be 
      SYMENT *sym = (SYMENT *) (((unsigned) syms) + i * SYMESZ);

      if (sym->n_sclass == C_FILE) {
	 char *moduleName;
	 if (!sym->n_zeroes) {
	    moduleName = &stringPool[sym->n_offset];
	 } else {
	    memset(tempName, 0, 9);
	    strncpy(tempName, sym->n_name, 8);
	    moduleName = tempName;
	 }
	 // look in aux records 
	 for (j=1; j <= sym->n_numaux; j++) {
	    aux = (union auxent *) ((char *) sym + j * SYMESZ);
	    if (aux->x_file._x.x_ftype == XFT_FN) {
		if (!aux->x_file._x.x_zeroes) {
                     moduleName = &stringPool[aux->x_file._x.x_offset];
                } else {
                     // x_fname is 14 bytes
                     memset(moduleName, 0, 15);
                     strncpy(moduleName, aux->x_file.x_fname, 14);
                }
	    }
	 }

	 if(currentSourceFile) delete currentSourceFile;
	 currentSourceFile = new pdstring(moduleName);
	 currentSourceFile = processDirectories(currentSourceFile);

	 if (strrchr(moduleName, '/')) {
	     moduleName = strrchr(moduleName, '/');
	     moduleName++;
	 }

	 if (!strcmp(moduleName, fileName().c_str())) {
		parseActive = true;
	 } else {
		parseActive = false;
	 }
      }

      if (!parseActive) continue;

      char *nmPtr;
      if (!sym->n_zeroes && ((sym->n_sclass & DBXMASK) ||
			     (sym->n_sclass == C_BINCL) ||
			     (sym->n_sclass == C_EINCL))) {
	  if (sym->n_offset < 3) {
	      if (sym->n_offset == 2 && stabstr[0]) {
		  nmPtr = &stabstr[0];
	      } else {
		  nmPtr = &stabstr[sym->n_offset];
	      }
	  } else if (!stabstr[sym->n_offset-3]) {
	      nmPtr = &stabstr[sym->n_offset];
	  } else {
	      /* has off by two error */
	      nmPtr = &stabstr[sym->n_offset-2];
	  }
      } else {
	  // names 8 or less chars on inline, not in stabstr
	  memset(tempName, 0, 9);
	  strncpy(tempName, sym->n_name, 8);
	  nmPtr = tempName;
      }

      if ((sym->n_sclass == C_BINCL) ||
	  (sym->n_sclass == C_EINCL) ||
	  (sym->n_sclass == C_FUN)) {
		if (funcName) { 
		    free(funcName);
		    funcName = NULL;
		}
		funcName = strdup(nmPtr);

		parseLineInformation(proc, currentSourceFile,  funcName, sym, 
				     linesfdptr, lines, nlines);
      }

    }

    cleanupLineInformation();
}


#endif
