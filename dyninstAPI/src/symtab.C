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

// $Id: symtab.C,v 1.130 2001/08/30 21:31:24 bernat Exp $

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

// TODO - machine independent instruction functions
#include "dyninstAPI/src/symtab.h"
#include "dyninstAPI/src/arch.h"
#include "dyninstAPI/src/instPoint.h"
#include "dyninstAPI/src/Object.h"
#include <fstream.h>
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
extern vector<sym_data> syms_to_find;
#endif

// All debug_ostream vrbles are defined in process.C (for no particular reason)
extern debug_ostream sharedobj_cerr;

vector<image*> image::allImages;

#if defined(i386_unknown_nt4_0) || (defined mips_unknown_ce2_11) //ccw 20 july 2000 : 29 mar 2001
extern char *cplus_demangle(char *, int);
#else
extern "C" char *cplus_demangle(char *, int);
#endif

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

/* imported from platform specific library list.  This is lists all
   library functions we are interested in instrumenting. */

pdmodule *image::newModule(const string &name, const Address addr)
{
    pdmodule *ret;
    // modules can be defined several times in C++ due to templates and
    //   in-line member functions.
    if ((ret = findModule(name, TRUE)))
      return(ret);

    string fullNm, fileNm;
    char *out = P_strdup(name.string_of());
    char *sl = P_strrchr(out, '/');
    if (sl) {
      *sl = (char)0;
      fullNm = out;
      fileNm = sl+1;
    } else {
      fullNm = string("/default/") + out;
      fileNm = out;
    }
    free(out);

    ret = new pdmodule(lang_Unknown, addr, fullNm, fileNm, this);

#ifndef BPATCH_LIBRARY
    // if module was excluded,stuff it into excludedMods (and dont
    //  index it in modeByFileName && modsByFullName.
    if (module_is_excluded(ret)) {
        excludedMods += ret;
    } else {
#endif /* BPATCH_LIBRARY */
        modsByFileName[ret->fileName()] = ret;
        modsByFullName[ret->fullName()] = ret;
	includedMods.push_back(ret);
#ifndef BPATCH_LIBRARY
    }
#endif /* BPATCH_LIBRARY */

    return(ret);
}


// TODO -- is this g++ specific
bool buildDemangledName(const string &mangled, string &use)
{
 /* The C++ demangling function demangles MPI__Allgather (and other MPI__
  * functions with start with A) into the MPI constructor.  In order to
  * prevent this a hack needed to be made, and this seemed the cleanest
  * approach.
  */
  if(!mangled.prefixed_by("MPI__")) {
    char *tempName = P_strdup(mangled.string_of());
    char *demangled = cplus_demangle(tempName, 0);
    
    if (demangled) {
      use = demangled;
      free(tempName);
      free(demangled);
      return true;
    } else {
      free(tempName);
      return false;
    }
  }
  return(false);
}

// err is true if the function can't be defined
// COMMENTS????
// looks like this function attempts to do several things:
//  1) Check if can find the function (WHERE DOES IT LOOK????).
//  2) strip out something from function name which looks like
//    possible scoping info (everything up to?? ":")??
//  3) try to demangle this (descoped??) name.
//  4) make new pd_Function under origional name and demangled name.
//  5) Insert into (data members) funcsByAdd && mods->funcs.
//  n) insert into (data member) funcsByPretty (indexed under 
//    demangled name).
bool image::newFunc(pdmodule *mod, const string &name, 
		    const Address addr, 
		    const unsigned size) {
  pd_Function *func;

  // KLUDGE
  if ((func = findFuncByAddr(addr))){
    //string temp = name;
    //temp += string(" findFunction succeeded\n");
    //logLine(P_strdup(temp.string_of()));
    return false;
  }

  if (!mod) {
    logLine("Error function without module\n");
    showErrorCallback(34, "Error function without module");
    return false;
  }
  
  string mangled_name = name;
  const char *p = P_strchr(name.string_of(), ':');
  if (p) {
     unsigned nchars = p - name.string_of();
     mangled_name = string(name.string_of(), nchars);
  }
  
  string demangled;
  if (!buildDemangledName(mangled_name, demangled)) 
    demangled = mangled_name;

  bool err=false;

  func = new pd_Function(name, demangled, mod, addr, size, this, err);
  assert(func);
  //cout << name << " pretty: " << demangled << " addr :" << addr <<endl;
  // if there was an error in determining the instrumentation info for
  //  function, add it to notInstruFunction.
  if (err) {
    //delete func;
    addNotInstruFunc(func);
    return false;
  }
 
  addInstruFunction(func, mod, addr,
#ifdef BPATCH_LIBRARY
		    false);
#else
		    function_is_excluded(func, mod->fileName()));
#endif
  return true;
}

void image::addInstruFunction(pd_Function *func, pdmodule *mod,
			      const Address addr, bool excluded) {
    vector<pd_Function*> *funcsByPrettyEntry = NULL;
    vector<pd_Function*> *funcsByMangledEntry = NULL;

    // any functions whose instrumentation info could be determined 
    //  get added to instrumentableFunctions, and mod->funcs.
    instrumentableFunctions.push_back(func);
    mod->funcs.push_back(func);

    if (excluded) {
	excludedFunctions[func->prettyName()] = func;
    } else {
        includedFunctions.push_back(func);
	funcsByAddr[addr] = func;
	if (!funcsByPretty.find(func->prettyName(), funcsByPrettyEntry)) {
            funcsByPrettyEntry = new vector<pd_Function*>;
            funcsByPretty[func->prettyName()] = funcsByPrettyEntry;
	}
	// several functions may have the same demangled name, and each one
        // will appear in a different module
        assert(funcsByPrettyEntry);
        (*funcsByPrettyEntry).push_back(func);

	if (!funcsByMangled.find(func->symTabName(), funcsByMangledEntry)) {
            funcsByMangledEntry = new vector<pd_Function*>;
            funcsByMangled[func->symTabName()] = funcsByMangledEntry;
	}
	// several functions may have the same demangled name, and each one
        // will appear in a different module
        assert(funcsByMangledEntry);
        (*funcsByMangledEntry).push_back(func);
    }
}

void image::addNotInstruFunc(pd_Function *func) {
    notInstruFunctions[func->prettyName()] = func;
}

#ifdef DEBUG_TIME
static timer loadTimer;
static FILE *timeOut=0;
#endif /* DEBUG_TIME */

// addOneFunction(): find name of enclosing module and define function symbol
//
// module information comes from one of three sources:
//   #1 - debug format (stabs, DWARF, etc.)
//   #2 - file format (ELF, COFF)
//   #3 - file name (a.out, libXXX.so)
// (in order of decreasing reliability)
bool image::addOneFunction(vector<Symbol> &mods,
			   const Symbol &lookUp) 
{
  // find module name
  Address modAddr = 0;
  string modName = lookUp.module();

  if (modName == "") {
    modName = name_ + "_module";
  } else if (modName == "DEFAULT_MODULE") {
    string modName_3 = modName;
    findModByAddr(lookUp, mods, modName, modAddr, modName_3);
  }
  
  pdmodule *use = getOrCreateModule(modName, modAddr);
  assert(use);
  return newFunc(use, lookUp.name(), lookUp.addr(), lookUp.size());
}

/*
 * Add another name for the current function to the names vector in
 * the function object.  We also need to add the extra names to the
 * lookup hash tables
 */
void image::addMultipleFunctionNames(vector<Symbol> &mods,
					const Symbol &lookUp)
{
  pd_Function *func = findFuncByAddr(lookUp.addr());

  /* sanity check, make sure we have actually seen this address before */
  assert(func);

  /* build the mangeled and pretty names so that we can add those to the
   * lookup tables
   */
  string name = lookUp.name();
  string mangled_name = name;
  const char *p = P_strchr(name.string_of(), ':');
  if (p) {
     unsigned nchars = p - name.string_of();
     mangled_name = string(name.string_of(), nchars);
  }

  string demangled;
  if (!buildDemangledName(mangled_name, demangled)) 
    demangled = mangled_name;

  /* add the names to the vectors in the function object */
  func->addSymTabName(mangled_name);
  func->addPrettyName(demangled);

  /* now we add the names and the function object to the hash tables */
  vector<pd_Function*> *funcsByPrettyEntry = NULL;
  vector<pd_Function*> *funcsByMangledEntry = NULL;

  if(!funcsByPretty.find(demangled, funcsByPrettyEntry)) {
    funcsByPrettyEntry = new vector<pd_Function*>;
    funcsByPretty[demangled] = funcsByPrettyEntry;
  }
    
  assert(funcsByPrettyEntry);
  (*funcsByPrettyEntry).push_back(func);

  if (!funcsByMangled.find(mangled_name, funcsByMangledEntry)) {
    funcsByMangledEntry = new vector<pd_Function*>;
    funcsByMangled[mangled_name] = funcsByMangledEntry;
  }

  assert(funcsByMangledEntry);
  (*funcsByMangledEntry).push_back(func);
}

/*
 * Add all the functions (*) in the list of symbols to our data
 * structures. 
 *
 * We do a search for a "main" symbol (a couple of variants), and
 * if found we flag this image as the executable (a.out). 
 */

bool image::addAllFunctions(vector<Symbol> &mods)
{
  Symbol lookUp;
  string symString;

  // is_a_out is a member variable
  Symbol mainFuncSymbol;  //Keeps track of info on "main" function

  //Checking "main" function names in same order as in the inst-*.C files
  if (linkedFile.get_symbol(symString="main",     lookUp) ||
      linkedFile.get_symbol(symString="_main",    lookUp) ||
      linkedFile.get_symbol(symString="WinMain",  lookUp) ||
      linkedFile.get_symbol(symString="_WinMain", lookUp)) {
    mainFuncSymbol = lookUp;
    is_a_out = true;
  }
  else
    is_a_out = false;

  // Checking for libdyninstRT (DYNINSTinit())
  if (linkedFile.get_symbol(symString="DYNINSTinit",  lookUp) ||
      linkedFile.get_symbol(symString="_DYNINSTinit", lookUp))
    is_libdyninstRT = true;
  else
    is_libdyninstRT = false;
  
  // find the real functions -- those with the correct type in the symbol table
  for(SymbolIter symIter(linkedFile); symIter;symIter++) {
    const Symbol &lookUp = symIter.currval();
    if (funcsByAddr.defines(lookUp.addr())) {
      // We have already seen a function at this addr. add a second name
      // for this function.
      addMultipleFunctionNames(mods, lookUp);
      continue;
    }
    if (is_a_out && 
	(lookUp.addr() == mainFuncSymbol.addr()) &&
	(lookUp.name() != mainFuncSymbol.name()))
      // Wait for main to appear. Couldn't we just add main to start with?
      continue;
    if (lookUp.type() == Symbol::PDST_FUNCTION) {
      if (!isValidAddress(lookUp.addr())) {
	string msg;
	char tempBuffer[40];
	sprintf(tempBuffer,"0x%lx",lookUp.addr());
	msg = string("Function ") + lookUp.name() + string(" has bad address ")
	  + string(tempBuffer);
	statusLine(msg.string_of());
	showErrorCallback(29, msg);
	return false;
      }
      addOneFunction(mods, lookUp);
    }
  }

  // now find the pseudo functions -- this gets ugly
  // kludge has been set if the symbol could be a function
  // WHERE DO WE USE THESE KLUDGES? WHAT PLATFORM???
  for(SymbolIter symIter2(linkedFile);symIter2;symIter2++) {
    lookUp = symIter2.currval();
    if (funcsByAddr.defines(lookUp.addr()))
      // Already defined a symbol at this addr
      continue;
    if ((lookUp.type() == Symbol::PDST_OBJECT) && lookUp.kludge()) {
      //logLine(P_strdup(symString.string_of()));
      // Figure out where this happens
      cerr << "Found <KLUDGE> function " << lookUp.name().string_of() << endl;
      addOneFunction(mods, lookUp);
    }
  }
  return true;
}

bool image::addAllVariables()
{
/* Eventually we'll have to do this on all platforms (because we'll retrieve
 * the type information here).
 */
#if defined(i386_unknown_nt4_0)  || (defined mips_unknown_ce2_11) //ccw 20 july 2000 : 29 mar 2001
  string mangledName; 
  Symbol symInfo;

  for(SymbolIter symIter(linkedFile); symIter; symIter++) {
     const string &mangledName = symIter.currkey();
     const Symbol &symInfo = symIter.currval();

    if (symInfo.type() == Symbol::PDST_OBJECT) {
      char *name = cplus_demangle((char *)mangledName.string_of(), 0);
      const char *unmangledName;
      if (name) unmangledName = name;
      else unmangledName = mangledName.string_of();
      if (varsByPretty.defines(unmangledName)) {
	  (*(varsByPretty[unmangledName])).push_back(string(mangledName));
      } else {
	  vector<string> *varEntry = new vector<string>;
	  (*varEntry).push_back(string(mangledName));
	  varsByPretty[unmangledName] = varEntry;
      }
      if (name) free(name);
    }
  }
#endif
  return true;
}

/*
 * will search for symbol NAME or _NAME
 * returns false on failure 
 */
bool image::findInternalSymbol(const string &name, 
			       const bool warn, 
			       internalSym &ret_sym)
{
   Symbol lookUp;

   if(linkedFile.get_symbol(name,lookUp)){
      ret_sym = internalSym(lookUp.addr(),name); 
      return true;
   }
   else {
       string new_sym;
       new_sym = string("_") + name;
       if(linkedFile.get_symbol(new_sym,lookUp)){
          ret_sym = internalSym(lookUp.addr(),name); 
          return true;
       }
   } 
   if(warn){
      string msg;
      msg = string("Unable to find symbol: ") + name;
      statusLine(msg.string_of());
      showErrorCallback(28, msg);
   }
   return false;
}

bool image::findInternalByPrefix(const string &prefix, 
				 vector<Symbol> &found) const
{
  bool flag = false;
  /*
    Go through all defined symbols and return those which
    match the prefix given 
  */
  for(SymbolIter symIter(linkedFile); symIter;symIter++) {
    const Symbol &lookUp = symIter.currval();
    if (!strncmp(prefix.string_of(), lookUp.name().string_of(),
		 strlen(prefix.string_of())))
      {
	found.push_back(lookUp);
	flag = true;
      }
  }
  return flag;
}

pdmodule *image::findModule(const string &name, bool find_if_excluded)
{
  unsigned i;

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

  //cerr << " (image::findModule) did not find module, returning NULL" << endl;
  return NULL;
}

/* 
 * return 0 if symbol <symname> exists in image, non-zero if it does not
 */
bool image::symbolExists(const string &symname)
{
  pd_Function *dummy = findFuncByName(symname);
  return (dummy != NULL);
}

#ifndef BPATCH_LIBRARY
void image::postProcess(const string pifname)
{
  FILE *Fil;
  string fname, errorstr;
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
  Fil = P_fopen(fname.string_of(), "r");

  if (Fil == NULL) {
    errorstr = string("Tried to open PIF file ") + fname;
    errorstr += string(", but could not (continuing)\n");
    logLine(P_strdup(errorstr.string_of()));
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
      errorstr = string("Ignoring bad line key (") + fname;
      errorstr += string(") in file %s\n");
      logLine(P_strdup(errorstr.string_of()));
      fgets(tmp1, 5000, Fil);
      break;
    }
  }
  return;
}
#endif

void image::defineModules() {
  unsigned i;

  string pds; pdmodule *mod;
  dictionary_hash_iter<string, pdmodule*> mi(modsByFileName);

  while (mi.next(pds, mod)){
    mod->define();
  }

  for(i=0;i<excludedMods.size();i++) {
    mod = excludedMods[i];
    mod->define();
  }

#ifdef DEBUG_MDL
  char buffer[100];
  ostrstream osb(buffer, 100, ios::out);
  osb << "IMAGE_" << name() << "__" << getpid() << ends;
  ofstream of(buffer, ios::app);

  of << "INCLUDED FUNCTIONS\n";
  for (unsigned ni=0; ni<includedFunctions.size(); ni++) {
    of << includedFunctions[ni]->prettyName() << "\t\t" 
       << includedFunctions[ni]->addr() << "\t\t" << endl;
  }
  
#endif
}

#ifndef BPATCH_LIBRARY
void image::FillInCallGraphStatic(process *proc)
{
  unsigned i;
  string pds;
  pdmodule *mod;
  dictionary_hash_iter<string, pdmodule*> mi(modsByFileName);
  string buffer;

  // define call graph relations for all non-excluded modules.
  while (mi.next(pds, mod)){
    buffer = "building call graph module: " +
      mod->fileName();
    statusLine(buffer.string_of());
    mod->FillInCallGraphStatic(proc);
  }
  // also define call graph relations for all excluded modules.
  //  Call graph gets information about both included and excluded 
  //  modules and functions, and allows the DM and PC to decide whether
  //  to look at a given function based on whether the function is 
  //  included or excluded....
  for(i=0;i<excludedMods.size();i++) {
    mod = excludedMods[i];
    buffer = "building call graph module: " +
      mod->fileName();
    statusLine(buffer.string_of());
    mod->FillInCallGraphStatic(proc);
  }
}
#endif

//  Comments on what this does would be nice....
//  Appears to run over a pdmodule, after all code in it has been processed
//   and parsed into functions, and define a resource for the module + a 
//   resource for every function found in the module (apparently includes 
//   excluded functions, but not uninstrumentable ones)....
//  Can't directly register call graph relationships here as resources
//   are being defined, because need all resources defined to 
//   do that....


void pdmodule::define() {
#ifndef BPATCH_LIBRARY
  resource *moduleResource = NULL;
#endif
#ifdef DEBUG_MODS
  char buffer[100];
  ostrstream osb(buffer, 100, ios::out);
  osb << "MODS_" << exec->name() << "__" << getpid() << ends;
  ofstream of(buffer, ios::app);
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
      if (!moduleResource) {
	moduleResource = resource::newResource(moduleRoot, this,
					    nullString, // abstraction
					    fileName(), // name
					    timeStamp::ts1970(), // creation time
					    string(), // unique-ifier
					    MDL_T_MODULE,
					    false);
      }

      pdf->SetFuncResource(resource::newResource(moduleResource, pdf,
						 nullString, // abstraction
						 pdf->prettyName(), 
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

// Why is this in symtab.C?
#ifndef BPATCH_LIBRARY
// send message to data manager to specify the entry function for the
//  call graph corresponding to a given image.  r should hold the 
//  FULL resourcename of the entry function (e.g. "/Code/module.c/main")
void CallGraphSetEntryFuncCallback(string exe_name, string r)
{
    tp->CallGraphSetEntryFuncCallback(exe_name, r);
}

void CallGraphAddProgramCallback(string name){
  tp->CallGraphAddProgramCallback(name);
}

//send message to the data manager, notifying it that all of the statically
//determinable functions have been registered with the call graph. The
//data manager will then be able to create the call graph.
void CallGraphFillDone(string exe_name)
{
  tp->CallGraphFillDone(exe_name);
}

//send message to the data manager in order to register a function 
//in the call graph.
void AddCallGraphNodeCallback(string exe_name, string r)
{
    tp->AddCallGraphNodeCallback(exe_name, r);
}

//send a message to the data manager in order register a the function
//calls made by a function (whose name is stored in r).
void AddCallGraphStaticChildrenCallback(string exe_name, string r,
					const vector<string> children) 
{
    tp->AddCallGraphStaticChildrenCallback(exe_name, r, children);
}

// Called across all modules (in a given image) to define the call
//  graph relationship between functions.
// Must be called AFTER all functions in all modules (in image) are
//  registered as resource (e.g. w/ pdmodule::define())....
void pdmodule::FillInCallGraphStatic(process *proc) {
  pd_Function *pdf, *callee;
  vector <pd_Function *>callees;
  resource *r , *callee_as_resource;
  string callee_full_name;
  vector <string>callees_as_strings;
  vector <instPoint*> callPoints; 
  unsigned f, g,i, f_size = funcs.size();
  
  string resource_full_name;
  
  // for each INSTRUMENTABLE function in the module (including excluded 
  //  functions, but NOT uninstrumentable ones)....
  
  for (f=0;f<f_size;f++) {
    pdf = funcs[f];
    
    callees_as_strings.resize(0);
    
    // Translate from function name to resource *.
    // Note that this probably is NOT the correct translation, as 
    //  function names are not necessarily unique, but the paradyn
    //  code assumes that they are in several places (e.g. 
    //  resource::findResource)....
    resource_full_name = pdf->ResourceFullName();
    r = resource::findResource(resource_full_name);
    // functions registered under pretty name....
    assert(r != NULL);
    
    // get list of statically determined call destinations from pdf,
    //  using the process info to help fill in calls througb PLT
    //  entries....
    pdf->getStaticCallees(proc, callees); 
    
    // and convert them into a list of resources....
    for (g=0;g<callees.size();g++) {
      callee = callees[g];
      assert(callee);
      
      //if the funcResource is not set, then the function must be
      //uninstrumentable, so we don't want to notify the front end
      //of its existence
      if(callee->FuncResourceSet()){
	callee_full_name = callee->ResourceFullName();
	
	// if callee->funcResource has been set, then it should have 
	//  been registered as a resource.... 
	callee_as_resource = resource::findResource(callee_full_name);
	assert(callee_as_resource);
	callees_as_strings += callee_full_name;
	}
      
    }//end for
    
    // register that callee_resources holds list of resource*s 
    //  describing children of resource r....
    string exe_name = proc->getImage()->file();
    AddCallGraphNodeCallback(exe_name, resource_full_name);
    AddCallGraphStaticChildrenCallback(exe_name, resource_full_name,
				       callees_as_strings);

    //Locate the dynamic call sites within the function, and notify 
    //the front end as to their existence
    callPoints = pdf->funcCalls(proc);
    for(i = 0; i < callPoints.size(); i++){
      if(proc->isDynamicCallSite(callPoints[i])){
	tp->CallGraphAddDynamicCallSiteCallback(exe_name, resource_full_name);
	break;
      }
    }
  }
}
#endif // ndef BPATCH_LIBRARY

#ifndef BPATCH_LIBRARY
// as per getAllFunctions, but filters out those excluded with 
// e.g. mdl "exclude" command....
// to clarify a function should NOT be returned from here if its 
//  module is excluded (with exclude "/Code/module") or if it
//  is excluded (with exclude "/Code/module/function"). 
const vector<pd_Function*> &image::getIncludedFunctions() {
    unsigned int i;    
    includedFunctions.resize(0);
    vector<function_base *> *temp;
    vector<pd_Function *> *temp2;

    //cerr << "image::getIncludedFunctions called, name = " << name () << endl;
    //cerr << " includedMods = " << endl;
    //print_module_vector_by_short_name(string("  "), &includedMods);
    for (i=0;i<includedMods.size();i++) {
         temp = includedMods[i]->getIncludedFunctions();
         temp2 = (vector<pd_Function *> *) temp;
         includedFunctions += *temp2;
    }

    //cerr << " (image::getIncludedFunctions) returning : " << endl;
    //print_func_vector_by_pretty_name(string("  "),
    //		 (vector<function_base*>*)&includedFunctions);

    // what about shared objects????
    return includedFunctions;
}
#endif


// get all functions in module which are not "excluded" (e.g.
//  with mdl "exclude" command.  
// Assed to provide support for mdl "exclude" on functions in
//  statically linked objects.
//  mcheyney 970727
vector<function_base *> *pdmodule::getIncludedFunctions() {
    // laxy construction of some_funcs, as per sharedobject class....
    // cerr << "pdmodule " << fileName() << " :: getIncludedFunctions called "
    //      << endl;
    if (some_funcs_inited == TRUE) {
        //cerr << "  about to return : " << endl;
        //print_func_vector_by_pretty_name(string("  "), (vector<function_base *>*)&some_funcs);
        return (vector<function_base *>*)&some_funcs;
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
    //print_func_vector_by_pretty_name(string("  "),(vector<function_base *>*) &some_funcs);
    return (vector<function_base *>*)&some_funcs;
}


const vector<pd_Function*> &image::getAllFunctions() {
    //cerr << "pdmodule::getAllFunctions() called, about to return instrumentableFunctions = " << endl;
    //print_func_vector_by_pretty_name(string("  "),
    //        (vector<function_base*>*)&instrumentableFunctions);
    return instrumentableFunctions;
}

const vector <pdmodule*> &image::getAllModules() {
    // reinit all modules to empty vector....
    allMods.resize(0);

    // and add includedModules && excludedModules to it....
    VECTOR_APPEND(allMods, includedMods);
    VECTOR_APPEND(allMods, excludedMods);

    //cerr << "image::getAllModules called" << endl;
    //cerr << " about to return sum of includedMods and excludedMods" << endl;
    //cerr << " includedMods : " << endl;
    //print_module_vector_by_short_name(string("  "), &includedMods);
    //cerr << " excludedMods : " << endl;
    //print_module_vector_by_short_name(string("  "), &excludedMods);
    
    return allMods;
}


#ifndef BPATCH_LIBRARY
const vector <pdmodule*> &image::getIncludedModules() {
    //cerr << "image::getIncludedModules called" << endl;
    //cerr << " about to return includedMods = " << endl;
    //print_module_vector_by_short_name(string("  "), &includedMods);

    return includedMods;
}
#endif

void print_module_vector_by_short_name(string prefix ,
				       vector<pdmodule*> *mods) {
    unsigned int i;
    pdmodule *mod;
    for(i=0;i<mods->size();i++) {
        mod = ((*mods)[i]);
	cerr << prefix << mod->fileName() << endl;
    }
}

void print_func_vector_by_pretty_name(string prefix,
				      vector<function_base *>*funcs) {
    unsigned int i;
    function_base *func;
    for(i=0;i<funcs->size();i++) {
      func = ((*funcs)[i]);
      cerr << prefix << func->prettyName() << endl;
    }
}

// rip module name out of constraint....
// Assumes that constraint is of form module/function, or
// module.... 
string getModuleName(string constraint) {
    string ret;

    const char *data = constraint.string_of();
    const char *first_slash = P_strchr(data, RH_SEPERATOR);
    
    // no "/", assume string holds module name....
    if (first_slash == NULL) {
	return constraint;
    }    
    // has "/", assume everything up to "/" is module name....
    return string(data, first_slash - data);
}

// rip function name out of constraint....
// Assumes that constraint is of form module/function, or
// module.... 
string getFunctionName(string constraint) {
    string ret;

    const char *data = constraint.string_of();
    const char *first_slash = P_strchr(data, RH_SEPERATOR);
    
    // no "/", assume constraint is module only....
    if (first_slash == NULL) {
	return string("");
    }    
    // has "/", assume everything after "/" is function....
    return string(first_slash+1);
}


#ifndef BPATCH_LIBRARY

// mcheyney, Oct. 6, 1997
static dictionary_hash<string, string> func_constraint_hash(string::hash);
static bool cache_func_constraint_hash() {
    static bool func_constraint_hash_loaded = FALSE;

    // strings holding exclude constraints....
    vector<string> func_constraints;
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
bool function_is_excluded(pd_Function *func, string module_name) {
    static bool func_constraint_hash_loaded = FALSE;

    string function_name = func->prettyName();
    string full_name = module_name + string("/") + function_name;

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

    string full_name = module->fileName();

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

bool filter_excluded_functions(vector<pd_Function*> all_funcs,
    vector<pd_Function*>& some_funcs, string module_name) {
    unsigned i;

    string full_name;
    static bool func_constraint_hash_loaded = FALSE;

    if (func_constraint_hash_loaded == FALSE) {
        if (!cache_func_constraint_hash()) {
	    return FALSE;
        }
    }

    // run over all_funcs, check if module/function is caught
    //  by an exclude....
    for(i=0;i<all_funcs.size();i++) {
        full_name = module_name + string("/") + all_funcs[i]->prettyName();
	if (!(func_constraint_hash.defines(full_name))) {
            some_funcs += all_funcs[i];
        }
    }
    
    //cerr << " looks successful : about to return TRUE" << endl;
    //cerr << " some_funcs (by pretty name) " << endl;
    //for (i=0;i<some_funcs.size();i++) {
    //    cerr << "  " << some_funcs[i]->prettyName() << endl;
    //}
    
    return TRUE;
}

#endif /* BPATCH_LIBRARY */

// identify module name from symbol address (binary search)
// based on module tags found in file format (ELF/COFF)
void image::findModByAddr (const Symbol &lookUp, vector<Symbol> &mods,
			   string &modName, Address &modAddr, 
			   const string &defName)
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

image *image::parseImage(fileDescriptor *desc)
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
  image *ret = new image(desc, err);
  if (err || !ret) {
    if (ret)
      delete ret;
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

//  statusLine("ready"); // this shouldn't be here, right? (cuz we're not done, right?)

#ifndef BPATCH_LIBRARY
  tp->resourceBatchMode(false);
#endif

  return(ret);
}


// Constructor for the image object. The fileDescriptor simply
// wraps (in the normal case) the object name and a relocation
// address (0 for a.out file). On the following platforms, we
// are handling a special case:
//   AIX: objects can possibly have a name like /lib/libc.so:shr.o
//          since libraries are archives
//        Both text and data sections have a relocation address

image::image(fileDescriptor *desc, bool &err)
  : 
    desc_(desc),
    is_libdyninstRT(false),
    is_a_out(false),
    main_call_addr_(0),
    linkedFile(desc, pd_log_perror),
    knownJumpTargets(int_addrHash, 8192),
    includedMods(0),
    excludedMods(0),
    allMods(0),
    includedFunctions(0),
    instrumentableFunctions(0),
    funcsByAddr(addrHash4),
    funcsByPretty(string::hash),
    funcsByMangled(string::hash),
    notInstruFunctions(string::hash),
    excludedFunctions(string::hash),
    modsByFileName(string::hash),
    modsByFullName(string::hash),
    varsByPretty(string::hash)
{
  sharedobj_cerr << "image::image for file name="
		 << desc->file() << endl;

  // initialize (data members) codeOffset_, dataOffset_,
  //  codeLen_, dataLen_.
  codeOffset_ = linkedFile.code_off();
  dataOffset_ = linkedFile.data_off();
  codeLen_ = linkedFile.code_len();
  dataLen_ = linkedFile.data_len();
    
  // if unable to parse object file (somehow??), try to
  //  notify luser/calling process + return....    
  if (!codeLen_ || !linkedFile.code_ptr()) {
    string msg = string("Parsing problem with executable file: ") + desc->file();
    statusLine(msg.string_of());
    msg += "\n";
    logLine(msg.string_of());
    err = true;
#ifndef mips_unknown_ce2_11 //ccw 29 mar 2001

#if defined(BPATCH_LIBRARY)
    BPatch_reportError(BPatchWarning, 27, msg.string_of()); 
#else
    showErrorCallback(27, msg); 
#endif
#endif
    return;
  }
  
  string msg;
  // give luser some feedback....
  msg = string("Parsing object file: ") + desc->file();
  statusLine(msg.string_of());
  
  name_ = extract_pathname_tail(desc->file());
  err = false;
  
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
  vector <Symbol> tmods;
  
  for (SymbolIter symIter(linkedFile); symIter; symIter++) {
    const Symbol &lookUp = symIter.currval();
    if (lookUp.type() == Symbol::PDST_MODULE) {
      
      const string &lookUpName = lookUp.name();
      const char *str = lookUpName.string_of();
      assert(str);
      int ln = lookUpName.length();
      
      // directory definition -- ignored for now
      if (str[ln-1] != '/') {
	tmods.push_back(lookUp);
      }
    }
  }
  
  // sort the modules by address
  statusLine("sorting modules");
  VECTOR_SORT(tmods, symbol_compare);
  
  // remove duplicate entries -- some .o files may have the same 
  // address as .C files.  kludge is true for module symbols that 
  // I am guessing are modules
  vector<Symbol> uniq;
  
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
  
  // define all of the functions
  statusLine("winnowing functions");
  
  // define all of the functions
  if (!addAllFunctions(uniq)) {
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
  
  statusLine("checking call points");
  checkAllCallPoints();
  
  // TODO -- remove duplicates -- see earlier note
  dictionary_hash<Address, unsigned> addr_dict(addrHash4);
  vector<pd_Function*> temp_vec;
  
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

void pdmodule::checkAllCallPoints() {
  unsigned fsize = funcs.size();
  for (unsigned f=0; f<fsize; f++)
      funcs[f]->checkCallPoints();
}


void image::checkAllCallPoints() {
  dictionary_hash_iter<string, pdmodule*> di(modsByFullName);
  string s; pdmodule *mod;
  while (di.next(s, mod))
    mod->checkAllCallPoints();
}

pdmodule *image::getOrCreateModule(const string &modName, 
				      const Address modAddr) {
  const char *str = modName.string_of();
  int len = modName.length();
  assert(len>0);

  // TODO ignore directory definitions for now
  if (str[len-1] == '/') {
    return NULL;
  } else {
    // TODO - navigate ".." and "."
    const char *lastSlash = P_strrchr(str, '/');
    if (lastSlash)
      return (newModule(++lastSlash, modAddr));
    else
      return (newModule(modName, modAddr));
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
pd_Function::pd_Function(const string &symbol, const string &pretty, 
		       pdmodule *f, Address adr, const unsigned size, 
		       const image *owner, bool &err) : 
  function_base(symbol, pretty, adr, size),
  file_(f),
  funcEntry_(0),
#ifndef BPATCH_LIBRARY
  funcResource(0),
#endif
  relocatable_(false)
{
  err = findInstPoints(owner) == false;
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

  // next, look in excludedFunctions...
  dictionary_hash_iter<string, pd_Function*> ex(excludedFunctions);
  string str;
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
  
  dictionary_hash_iter<string, pd_Function *> ni(notInstruFunctions);
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

pd_Function *image::findFuncByEntryAddr(const Address &addr, 
					const process */*p*/) const
{
  pd_Function *pdf;

  // Quick check of funcsByAddr
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

  // next, look in excludedFunctions...
  dictionary_hash_iter<string, pd_Function*> ex(excludedFunctions);
  string str;
  while (ex.next(str, pdf)) {
    if ( (addr>=pdf->getAddress(p)) && 
	 (addr < (pdf->getAddress(p)+pdf->size()))
	 )
      return pdf;
  }
  
  dictionary_hash_iter<string, pd_Function *> ni(notInstruFunctions);
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

  // next, look in excludedFunctions...
  dictionary_hash_iter<string, pd_Function*> ex(excludedFunctions);
  string str;
  while (ex.next(str, pdf)) {
    if ( p &&
	 (addr>=pdf->getEffectiveAddress(p)) &&
	 (addr<(pdf->getEffectiveAddress(p)+pdf->size()))
	 )
      return pdf;
  }
  
  dictionary_hash_iter<string, pd_Function *> ni(notInstruFunctions);
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

vector <pd_Function *> *image::findFuncVectorByPretty(const string &name)
{
  if (funcsByPretty.defines(name))
    return funcsByPretty[name];
  return NULL;
}

// Return a single function associated with a demangled name. Arbitrarily
// pick the first one.

pd_Function *image::findFuncByPretty(const string &name)
{
  vector <pd_Function *> *a;

  if (funcsByPretty.defines(name)) {
    a = funcsByPretty[name];
    return ((*a)[0]);
  }
  return NULL;
}

pd_Function *image::findFuncByMangled(const string &name)
{
  vector <pd_Function *> *a;

  if (funcsByMangled.defines(name)) {
    a = funcsByMangled[name];
    return ((*a)[0]);
  }
  return NULL;
}

pd_Function *image::findExcludedFunc(const string &name)
{
  if (excludedFunctions.defines(name))
    return excludedFunctions[name];
  return NULL;
}

pd_Function *image::findNonInstruFunc(const string &name)
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
pd_Function *image::findFuncByName(const string &name)
{
  pd_Function *pdf;

  if ( ( pdf = findFuncByPretty(name))) return pdf;

  if ( ( pdf = findFuncByMangled(name))) return pdf;

  return NULL;
}

// This function supposely is only used to find function that
// is not instrumentable which may not be totally defined.
// Use with caution.  
// NEW - also can be used to find an excluded function....
pd_Function *image::findOneFunctionFromAll(const string &name) {
    pd_Function *ret = NULL;
    if ( (ret = findFuncByName(name))) 
      return ret;

    if ( (ret = findNonInstruFunc(name)))
      return ret;

    if ( (ret = findExcludedFunc(name)))
      return ret;

    return NULL;
}

#if 0
// Only looks for function by pretty name.
//  Should it also look by mangled name??
bool image::findFunction(const string &name, vector<pd_Function*> &retList,
        bool find_if_excluded) {

    bool found = FALSE;
    if (funcsByPretty.defines(name)) {
        retList = *funcsByPretty[name];
        found = TRUE;
    } 

    if (find_if_excluded) {
      // should APPEND to retList!!!!
      if (excludedFunctions.defines(name))
	retList.push_back(excludedFunctions[name]);
      found = TRUE;
    }
    return found;
}
#endif
