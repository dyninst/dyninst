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

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

// TODO - machine independent instruction functions
#include "dyninstAPI/src/symtab.h"
#include "dyninstAPI/src/arch.h"
#include "util/h/Object.h"
#include <fstream.h>
#include "dyninstAPI/src/util.h"
#include "dyninstAPI/src/dyninstP.h"
#include "util/h/String.h"
#include "dyninstAPI/src/inst.h"
#include "util/h/Timer.h"
#include "paradynd/src/showerror.h"
#include "util/h/debugOstream.h"

#ifndef BPATCH_LIBRARY
#include "paradynd/src/mdld.h"
#include "paradynd/src/main.h"
#include "paradynd/src/init.h"
#else
extern vector<sym_data> syms_to_find;
#endif

// All debug_ostream vrbles are defined in process.C (for no particular reason)
extern debug_ostream sharedobj_cerr;

vector<image*> image::allImages;

#if defined(i386_unknown_nt4_0)
extern char *cplus_demangle(char *, int);
#else
extern "C" char *cplus_demangle(char *, int);
#endif


/*
  Debuggering info for function_base....
 */
ostream & function_base::operator<<(ostream &s) const {
    s << "symTabName_ = " << symTabName_ << " prettyName_ = " \
      << prettyName_ \
      << " line_ = " << line_ << " addr_ = "<< addr_ << " size_ = " << size_ \
      << " tag_ = " << tag_ << endl;
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

    ret = new pdmodule(langUnknown, addr, fullNm, fileNm, this);

    // if module was excluded,stuff it into excludedMods (and dont
    //  index it in modeByFileName && modsByFullName.
    if (module_is_excluded(ret)) {
        excludedMods += ret;
    } else {
        modsByFileName[ret->fileName()] = ret;
        modsByFullName[ret->fullName()] = ret;
        includedMods += ret;
    }

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
bool image::newFunc(pdmodule *mod, const string &name, const Address addr, 
		    const unsigned size, const unsigned tag, 
		    pd_Function *&retFunc) {
  pd_Function *func;

  retFunc = NULL;
  // KLUDGE
  if ((func = findFunction(addr, TRUE))){
    string temp = name;
    temp += string(" findFunction succeeded\n");
    logLine(P_strdup(temp.string_of()));
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

  bool err;

  func = new pd_Function(name, demangled, mod, addr, size, tag, this, err);
  assert(func);
  //cout << name << " pretty: " << demangled << " addr :" << addr <<endl;
  retFunc = func;
  // if there was an error in determining the instrumentation info for
  //  function, add it to notInstruFunction.
  if (err) {
    //delete func;
    retFunc = NULL;
    addNotInstruFunc(func);
    return false;
  }
  
  addInstruFunction(func, mod, addr, \
		    function_is_excluded(func, mod->fileName()));
  return true;
}

void image::addInstruFunction(pd_Function *func, pdmodule *mod, \
			      const Address addr, bool excluded) {
    vector<pd_Function*> *funcsByPrettyEntry;

    // any functions whose instrumentation info could be determined 
    //  get added to instrumentableFunctions, and mod->funcs.
    instrumentableFunctions += func;
    mod->funcs += func;

    if (excluded) {
	excludedFunctions[func->prettyName()] = func;
    } else {
        includedFunctions += func;
	funcsByAddr[addr] = func;

	if (!funcsByPretty.find(func->prettyName(), funcsByPrettyEntry)) {
            funcsByPrettyEntry = new vector<pd_Function*>;
            funcsByPretty[func->prettyName()] = funcsByPrettyEntry;
	}
	// several functions may have the same demangled name, and each one
        // will appear in a different module
        assert(funcsByPrettyEntry);
        (*funcsByPrettyEntry) += func;
    }
}

void image::addNotInstruFunc(pd_Function *func) {
    notInstruFunctions[func->prettyName()] = func;
}

#ifdef DEBUG_TIME
static timer loadTimer;
static FILE *timeOut=0;
#endif /* DEBUG_TIME */

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
image *image::parseImage(const string file)
{
  /*
   * Check to see if we have parsed this image at this offeset before.
   */
  // TODO -- better method to detect same image/offset --> offset only for CM5

  unsigned numImages = allImages.size();

  for (unsigned u=0; u<numImages; u++)
    if (file == allImages[u]->file())
      return allImages[u];

  /*
   * load the symbol table. (This is the a.out format specific routine).
   */

  statusLine("Process executable file");
  bool err;

  // TODO -- kill process here
  image *ret = new image(file, err);
  if (err || !ret) {
    if (ret)
      delete ret;
    return NULL;
  }

  // Add to master image list.
  image::allImages += ret;

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

/*
 * load a shared object:
 *   1.) parse symbol table and identify rotuines.
 *   2.) scan executable to identify inst points.
 *
 */
image *image::parseImage(const string file,u_int baseAddr)
{
  /*
   * Check to see if we have parsed this image at this offeset before.
   */
  // TODO -- better method to detect same image/offset --> offset only for CM5

  unsigned theSize = allImages.size();

  for (unsigned u=0; u<theSize; u++)
    if (file == allImages[u]->file())
      return allImages[u];

  /*
   * load the symbol table. (This is the a.out format specific routine).
   */

  if(!baseAddr) statusLine("Processing an executable file");
  else  statusLine("Processing a shared object file");
  bool err;

  // TODO -- kill process here
  image *ret = new image(file, baseAddr,err);
  if (err || !ret) {
    if (ret)
      delete ret;
    logLine("error after new image in parseImage\n");
    return NULL;
  }

  // Add to master image list.
  image::allImages += ret;

  // define all modules.
  ret->defineModules();
  return(ret);
}

// COMMENTS??  COMMENTS??
// check symbol whose name is in <str>.  If it is prefixed by
// "_DYNINST" or "_TRACELIB", then assume that that it is an
// "internal" symbol, and add it to (private data member) 
// iSymsMap.
bool image::addInternalSymbol(const string &str, const Address symValue) {
  // Internal symbols are prefixed by { DYNINST, TRACELIB }
  // don't ignore the underscore
  static string dyn = "_DYNINST"; 
  static string tlib = "_TRACELIB";


  // normalize all symbols -- remove the leading "_"
  if (str.prefixed_by(dyn)) {
    const char *s = str.string_of(); s++;
    if (!iSymsMap.defines(s)){
      iSymsMap[s] = new internalSym(symValue, s);
    }
    return true;
  } else if (str.prefixed_by(tlib)) {
    const char *s = str.string_of(); s++;
    if (!iSymsMap.defines(s))
      iSymsMap[s] = new internalSym(symValue, s);
    return true;
  } else {
    if (!iSymsMap.defines(str))
      iSymsMap[str] = new internalSym(symValue, str);
    return true;
  }
  return false;
}

/*
 * will search for symbol NAME or _NAME
 * returns false on failure 
 */
bool image::findInternalSymbol(const string &name, const bool warn, internalSym &ret_sym){
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

Address image::findInternalAddress(const string &name, const bool warn, bool &err)
{

  err = false;

  internalSym *theSym; // filled in by find()
  if (!iSymsMap.find(name, theSym)) {
    // not found!
    if (warn) {
      string msg = string("Unable to find symbol: ") + name;
      statusLine(msg.string_of());
      showErrorCallback(28, msg);
    }
    err = true;
    return 0;
  } 
  else
    return (theSym->getAddr());
}

pdmodule *image::findModule(const string &name, bool find_if_excluded)
{
  unsigned i;

  //cerr << "image::findModule " << name << " , " << find_if_excluded \
  //       << " called" << endl;

  if (modsByFileName.defines(name)) {
    //cerr << " (image::findModule) found module in modsByFileName" << endl;
    return (modsByFileName[name]);
  }
  else if (modsByFullName.defines(name)) {
    //cerr << " (image::findModule) found module in modsByFuleName" << endl;
    return (modsByFullName[name]);
  }
  
  // if also looking for excluded functions, check through 
  //  excludedFunction list to see if any match function by FullName
  //  or FileName....
  if (find_if_excluded) {
      for(i=0;i<excludedMods.size();i++) {
          if ((excludedMods[i]->fileName() == name) || \
                  (excludedMods[i]->fullName() == name)) {
	      //cerr << " (image::findModule) found module in excludedMods" << endl;
              return excludedMods[i];
          }
      }
  }

  //cerr << " (image::findModule) did not find module, returning NULL" << endl;
  return NULL;
}

// TODO -- this is only being used in cases where only one function
// should exist -- should I assert that the vector size <= 1 ?
// mcheyney - should return NULL when function being searched for 
//  is excluded!!!!
pd_Function *image::findOneFunction(const string &name)
{
  string demangName;
  pd_Function *ret;

  //cerr << "image::findOneFunction " << name << " called" << endl;

  if (funcsByPretty.defines(name)) {
    vector<pd_Function*> *a = funcsByPretty[name];
    assert(a);
    if (!a->size())
      ret = NULL;
    else
      ret = ((*a)[0]);
  } else if (buildDemangledName(name, demangName)) {
    if (funcsByPretty.defines(demangName)) {
      vector<pd_Function*> *a = funcsByPretty[demangName];
      assert(a);
      if (!a->size())
	ret = NULL;
      else
	ret = ((*a)[0]);
    } else
      ret = NULL;
  } else
    ret = NULL;

  //cerr << "(image::findOneFunction " << name << " ) about to return";
  //if (ret == NULL) {
  //    cerr << " NULL" << endl;
  //} else {
  //    cerr << " non-NULL" << endl;
  //}

  return ret;
}

// This function supposely is only used to find function that
// is not instrumentable which may not be totally defined.
// Use with caution.  
// NEW - also can be used to find an excluded function....
pd_Function *image::findOneFunctionFromAll(const string &name) {
    pd_Function *ret;
    if ((ret = findOneFunction(name))) 
	return ret;

    //cerr << "image::findOneFunctionFromAll " << name << \
	  " called, unable to find function in hash table" << endl; 

    if (notInstruFunctions.defines(name)) {
        //cerr << "  (image::findOneFunctionFromAll) found in notInstruFunctions" \
	   << endl;
        return notInstruFunctions[name];
    }

    if (excludedFunctions.defines(name)) {
        //cerr << "  (image::findOneFunctionFromAll) found in excludedFunctions" \
	   << endl;
        return excludedFunctions[name];
    }
    return NULL;
}

bool image::findFunction(const string &name, vector<pd_Function*> &retList, \
        bool find_if_excluded) {

    bool found = FALSE;
    if (funcsByPretty.defines(name)) {
        retList = *funcsByPretty[name];
        found = TRUE;
    } 

    if (find_if_excluded) {
        // should APPEND to retList!!!!
        if (find_excluded_function(name, retList)) {
            found = TRUE;
        }
    }    
    return found;
}

pd_Function *image::findFunction(const Address &addr, bool find_if_excluded) 
{
  pd_Function *result; // filled in by find()
  if (funcsByAddr.find(addr, result))
     return result;
  if (find_if_excluded) {
     return find_excluded_function(addr);
  }
  return NULL;
}

// find (excluded only) function byt address.... 
pd_Function *image::find_excluded_function(const Address &addr) 
{
    pd_Function *this_func;
    string str;
    dictionary_hash_iter<string, pd_Function*> ex(excludedFunctions);
    while(ex.next(str, this_func)) {
        // ALERT ALERT - assumes that code in image is NOT
	//  relocated!!!!
	if (addr == this_func->getAddress(0)) {
	    return this_func;
	}
    }
    return NULL;
}

//  Question??  Currently only finding 1 excluded function reference by
//  name (e.g. findOneFunctionFromAll.  Want to keep multiple copies
// of same excluded function (as referenced by name)??
bool image::find_excluded_function(const string &name, \
        vector<pd_Function*> &retList) {
    bool found = FALSE;

    found = excludedFunctions.defines(name);
    if (found) {
        retList += excludedFunctions[name];
    }
    return found;
}

pd_Function *image::findFunctionInInstAndUnInst(const Address &addr,const process *p) const 
{
  pd_Function *pdf;
  vector <pd_Function *> values;
  unsigned i;

  if (funcsByAddr.find(addr, pdf))
     return pdf;
  else
     return NULL;
  
  // If not found, we are going to search them in the 
  // uninstrumentable function
  values = notInstruFunctions.values();
  for (i = 0; i < values.size(); i++) {
      pdf = values[i];
      if ((addr>=pdf->getAddress(p))&&(addr<=(pdf->getAddress(p)\
					      +pdf->size()))) 
	  return pdf;
  }

  // and in excludedFunctions....
  values = excludedFunctions.values();
  for (i = 0; i < values.size(); i++) {
      pdf = values[i];
      if ((addr>=pdf->getAddress(p))&&(addr<=(pdf->getAddress(p)\
					      +pdf->size()))) 
	  return pdf;
  }

  return NULL; 
}
 
pd_Function *image::findFunctionIn(const Address &addr,const process *p) const 
{
  pd_Function *pdf;

  // first, look in funcsByAddr - should contain instrumentable non-
  //  excluded functions....
  dictionary_hash_iter<Address, pd_Function*> mi(funcsByAddr);
  Address adr;
  while (mi.next(adr, pdf)) {
      if ((addr>=pdf->getAddress(p))&&(addr<=(pdf->getAddress(p)+pdf->size()))) 
	  return pdf;
  }
  
  // next, look in excludedFunctions...
  dictionary_hash_iter<string, pd_Function*> ex(excludedFunctions);
  string str;
  while (ex.next(str, pdf)) {
     if ((addr>=pdf->getAddress(p)) && \
	      (addr<=(pdf->getAddress(p)+pdf->size()))) 
	  return pdf;
  }

  dictionary_hash_iter<string, pd_Function *> ni(notInstruFunctions);
  while (ni.next(str, pdf)) {
      if ((addr>=pdf->getAddress(p)) && \
	      (addr<=(pdf->getAddress(p)+pdf->size()))) 
	  return pdf;
  }

  return NULL; 
}


#ifndef BPATCH_LIBRARY
void image::changeLibFlag(resource *res, const bool setSuppress)
{
  image *ret;
  pdmodule *mod;

  unsigned numImages = image::allImages.size();
  for (unsigned u=0; u<numImages; u++) {
    ret = image::allImages[u];
    // assume should work even if module excluded....
    mod = ret->findModule(res->part_name(), TRUE);
    if (mod) {
      // suppress all procedures.
      mod->changeLibFlag(setSuppress);
    } else {
      // more than one function may have this name --> templates, statics
      vector<pd_Function*> pdfA;
      if (ret->findFunction(res->part_name(), pdfA)) {
	for (unsigned i=0; i<pdfA.size(); ++i) {
	  if (setSuppress) 
	    pdfA[i]->tagAsLib();
	  else
	    pdfA[i]->untagAsLib();
	}
      }
    }
  }
}
#endif


/* 
 * return 0 if symbol <symname> exists in image, non-zero if it does not
 */
bool image::symbolExists(const string &symname)
{
  pd_Function *dummy = findOneFunction(symname);
  return (dummy != NULL);
}

void image::postProcess(const string pifname)
{
  FILE *Fil;
  string fname, errorstr;
  char key[5000];
  char tmp1[5000], abstraction[500];
#ifndef BPATCH_LIBRARY
  resource *parent;
#endif

  return;

  /* What file to open? */
  if (!(pifname == (char*)NULL)) {
    fname = pifname;
  } else {
    fname = file_ + ".pif";
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
					 tmp1, 0.0,
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
    of << includedFunctions[ni]->prettyName() << "\t\t" << includedFunctions[ni]->addr() << "\t\t" << << endl;
  }
  
#endif
}

void pdmodule::define() {
#ifndef BPATCH_LIBRARY
  resource *modResource = NULL;
#endif
#ifdef DEBUG_MODS
  char buffer[100];
  ostrstream osb(buffer, 100, ios::out);
  osb << "MODS_" << exec->name() << "__" << getpid() << ends;
  ofstream of(buffer, ios::app);
#endif

  unsigned f_size = funcs.size();

  for (unsigned f=0; f<f_size; f++) {
    pd_Function *pdf = funcs[f];
#ifdef DEBUG_MODS
    of << fileName << ":  " << pdf->prettyName() <<  "  " \
        << pdf->addr() << endl;
#endif
    // ignore line numbers for now 

#ifndef BPATCH_LIBRARY
    //if (!(pdf->isLibTag())) {
    if (1) {
      // see if we have created module yet.
      if (!modResource) {
	modResource = resource::newResource(moduleRoot, this,
					    nullString, // abstraction
					    fileName(), // name
					    0.0, // creation time
					    string(), // unique-ifier
					    MDL_T_MODULE,
					    false);
      }
      resource::newResource(modResource, pdf,
			    nullString, // abstraction
			    pdf->prettyName(), 0.0,
			    nullString, // uniquifier
			    MDL_T_PROCEDURE,
			    false);
    }
#endif
  }

#ifndef BPATCH_LIBRARY
  resource::send_now();
#endif
}

// get all functions in module which are not "excluded" (e.g.
//  with mdl "exclude" command.  
// Assed to provide support for mdl "exclude" on functions in
//  statically linked objects.
//  mcheyney 970727
vector<function_base *> *pdmodule::getIncludedFunctions() {
    // laxy construction of some_funcs, as per sharedobject class....
    // cerr << "pdmodule " << fileName() << " :: getIncludedFunctions called " \
       << endl;
    if (some_funcs_inited == TRUE) {
        //cerr << "  about to return : " << endl;
        print_func_vector_by_pretty_name(string("  "), (vector<function_base *>*)&some_funcs);
        return (vector<function_base *>*)&some_funcs;
    }
    some_funcs.resize(0);
    if (filter_excluded_functions(funcs, some_funcs, fileName()) == FALSE) {
        //cerr << "  about to return NULL";
	return NULL;
    }
    some_funcs_inited = TRUE;
    
    //cerr << "  about to return : " << endl;
    //print_func_vector_by_pretty_name(string("  "),(vector<function_base *>*) &some_funcs);
    return (vector<function_base *>*)&some_funcs;
}


// get all functions in module which are not "excluded" (e.g.
//  with mdl "exclude" command.  
// Assed to provide support for mdl "exclude" on functions in
//  statically linked objects.
//  mcheyney 970727
const vector<pd_Function *> &image::getIncludedFunctions() {
    //cerr << "image::getIncludedFunctions() called, about to return includedFunctions = " << endl;
    //print_func_vector_by_pretty_name(string("  "), \
    //        (vector<function_base*>*)&includedFunctions);
    return includedFunctions;
}

const vector<pd_Function*> &image::getAllFunctions() {
    //cerr << "pdmodule::getAllFunctions() called, about to return instrumentableFunctions = " << endl;
    //print_func_vector_by_pretty_name(string("  "), \
    //        (vector<function_base*>*)&instrumentableFunctions);
    return instrumentableFunctions;
}

const vector <pdmodule*> &image::getAllModules() {
    // reinit all modules to empty vector....
    allMods.resize(0);

    // and add includedModules && excludedModules to it....
    allMods += includedMods;
    allMods += excludedMods;

    //cerr << "image::getAllModules called" << endl;
    //cerr << " about to return sum of includedMods and excludedMods" << endl;
    //cerr << " includedMods : " << endl;
    //print_module_vector_by_short_name(string("  "), &includedMods);
    //cerr << " excludedMods : " << endl;
    //print_module_vector_by_short_name(string("  "), &excludedMods);
    
    return allMods;
}

const vector <pdmodule*> &image::getIncludedModules() {
    //cerr << "image::getIncludedModules called" << endl;
    //cerr << " about to return includedMods = " << endl;
    //print_module_vector_by_short_name(string("  "), &includedMods);

    return includedMods;
}

void print_module_vector_by_short_name(string prefix , \
				       vector<pdmodule*> *mods) {
    unsigned int i;
    pdmodule *mod;
    for(i=0;i<mods->size();i++) {
        mod = ((*mods)[i]);
	cerr << prefix << mod->fileName() << endl;
    }
}

void print_func_vector_by_pretty_name(string prefix, \
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


// mcheyney, Oct. 6, 1997
bool module_is_excluded(pdmodule *module) {
    unsigned i;
    string constraint_function_name;
    string constraint_module_name;
    string module_name = module->fileName();
    // strings holding exclude constraints....
    vector<string> func_constraints;
    string empty_string("");

    //cerr << "module_is_excluded " << module_name << " called" << endl;

    // if unble to get list of excluded functions, assume all modules
    //  are NOT excluded!!!!
    if(mdl_get_lib_constraints(func_constraints) == FALSE) {
        //cerr << " (module_is_excluded) unable to mdl_get_lib_constrants, returning FALSE" << endl;
        return FALSE;
    }

    // run through func_constraints, looking for a constraint
    //  where constraint_module_name == module_name && 
    //  constraint_function_name == ""....
    for(i=0;i<func_constraints.size();i++) {
        constraint_module_name = getModuleName(func_constraints[i]);
	if (constraint_module_name == module_name) {
	    constraint_function_name = getFunctionName(func_constraints[i]);
	    if (constraint_function_name == empty_string) {
	        //cerr << " (module_is_excluded) found matching constraint " << func_constraints[i] << " returning TRUE" << endl;
	        return TRUE;
	    }
	}
    }

    //cerr << " (module_is_excluded) didnt find matching constraint returning FALSE" << endl;
    return FALSE;
}

// mcheyney, Oct. 3, 1997
// Return boolean value indicating whether function is found to
//  be excluded (via "exclude module_name" or "exclude module_name/
//  function_name").
bool function_is_excluded(pd_Function *func, string module_name) {
    // strings holding exclude constraints....
    vector<string> func_constraints;
    string function_name = func->prettyName();
    string constraint_function_name;
    string constraint_module_name;
    unsigned i;

    //cerr << "function_is_excluded " << function_name << " , " << \
      module_name << " called" << endl;

    // if unble to get list of excluded functions, assume all functions
    //  are NOT excluded!!!!
    if(mdl_get_lib_constraints(func_constraints) == FALSE) {
        //cerr << " (function_is_excluded) unable to mdl_get_lib_constrants, returning FALSE" << endl;
        return FALSE;
    }

    // run through func_constraints, looking for a constraint of 
    //  form module_name/function_name....
    for(i=0;i<func_constraints.size();i++) {
        constraint_module_name = getModuleName(func_constraints[i]);
	if (constraint_module_name == module_name) { 
	    constraint_function_name = getFunctionName(func_constraints[i]);
	    if (constraint_function_name == function_name) {
	        //cerr << " (function_is_excluded) found matching constraint " << func_constraints[i] << " returning TRUE" << endl;
	        return TRUE;
	    }
	}
    }

    //cerr << " (function_is_excluded) didnt find matching constraint, returing FALSE" << endl;
    return FALSE;
}

//
// mcheyney, Sep 28, 1997
// Take a list of functions (in vector <all_funcs>.  Copy all
//  of those functions which are not excluded (via "exclude" 
//  {module==module_name}/{function==function_name) into
//  <some_functions>.
// Returns status of mdl_get_lib_constraints() call.
//  If this status == FALSE< some_funcs is not modified....
// We assume that all_funcs is generally longer than the list
//  of constrained functions.  As such, internally proc. copies
//  all funcs into some funcs, then runs over excluded funcs
//  removing any matches, as opposed to doing to checking 
//  while adding to some_funcs....
bool filter_excluded_functions(vector<pd_Function*> all_funcs, \
    vector<pd_Function*>& some_funcs, string module_name) {

    u_int i, j;
    // strings holding exclude constraints....
    vector<string> func_constraints;
    string constraint_module_name, constraint_function_name, empty_string;
    vector<string> tstrings;
    bool excluded;

    empty_string = string("");

    //cerr << "filter_excluded_functions called : " << endl;
    //cerr << " module_name = " << module_name << endl;
    //cerr << " all_funcs (by pretty name ) : " << endl;
    //for(i=0;i<all_funcs.size();i++) {
    //    cerr << "  " << all_funcs[i]->prettyName() << endl;
    //}
    

    // if you cannot get set of lib constraints, return FALSE w/o
    // modifying some_funcs....
    if(mdl_get_lib_constraints(func_constraints) == FALSE) {
        if (0) {
	    //cerr << " could not mdl_get_lib_constraints, about to return FALSE" \
	       << endl;
        }
        return FALSE;
    }

    // run through func_constraints, filtering all constraints
    //  not of form module/function, and all constraints of that
    //  form where constraint module name != module_name....
    for(i=0;i<func_constraints.size();i++) {
        constraint_module_name = getModuleName(func_constraints[i]);
	//cerr << "constraint = " << func_constraints[i] << endl;
	//cerr << " constraint_module_name = " << constraint_module_name \
	         << endl;
	
	if (module_name == constraint_module_name) {
	    constraint_function_name = getFunctionName(func_constraints[i]);
	    if (constraint_function_name != empty_string) { 
	        tstrings += getFunctionName(func_constraints[i]);
	    }
	}
    }
    func_constraints = tstrings;
    
    //cerr << "func_constraints = " << endl;
    //for(i=0;i<func_constraints.size();i++) {
    //    cerr << "  " << func_constraints[i] << endl;
    //}
   

    // run over functions in all_funcs.  If they dont match an excluded 
    //  function in module_name, then push them onto some_funcs....
    for(i=0;i<all_funcs.size();i++) {
        excluded = FALSE;
	for(j=0;j<func_constraints.size();j++) {
	    if (all_funcs[i]->prettyName() == func_constraints[j]) {
	        excluded = TRUE;
		break;
	    }
	}
	if (excluded == FALSE) {
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

// I commented this out since gcc says it ain't used --ari 10/97
// I then uncommented it because non-Solaris platforms need it --ssuen 10/10/97
static void binSearch (const Symbol &lookUp, vector<Symbol> &mods,
		       string &modName, Address &modAddr, const string &def) {
  int start=0, end=mods.size()-1, index;
  bool found=false;

  if (!mods.size()) {
    modAddr = 0;
    modName = def;
    return;
  }

  while ((start <= end) && !found) {
    index = (start+end)/2;

    if ((index == (((int)mods.size())-1)) ||
	((mods[index].addr() <= lookUp.addr()) && (mods[index+1].addr() > lookUp.addr()))) {
      modName = mods[index].name();
      modAddr = mods[index].addr();      
      found = true;
    } else if (lookUp.addr() < mods[index].addr()) {
      end = index - 1;
    } else {
      start = index + 1;
    }
  }
  if (!found) {
    modName = mods[0].name();
    modAddr = mods[0].addr();
  }
}

// COMMENTS????
// 
bool image::addOneFunction(vector<Symbol> &mods,  \
			   pdmodule *lib, \
			   pdmodule *, \
			   const Symbol &lookUp, pd_Function  *&retFunc) {
  // TODO mdc
  // find the module
  // this is a "user" symbol
  string modName = lookUp.module();
  Address modAddr = 0;
  
  string progName = name_ + "_module";

#if defined (sparc_sun_solaris2_4) || defined (i386_unknown_solaris2_5) 
  // In solaris there is no address for modules in the symbol table, 
  // so the binary search will not work. The module field in a symbol
  // already has the correct module name for a symbol, if it can be
  // obtained from the symbol table, otherwise the module is an empty
  // string.
  if (modName == "") {
    modName = progName;
  }
#else
  binSearch(lookUp, mods, modName, modAddr, progName);
#endif

  return (defineFunction(lib, lookUp, modName, modAddr, retFunc));
}

bool inLibrary(Address addr, Address boundary_start, Address boundary_end,
		      Address startAddr, bool startB,
		      Address endAddr, bool endB) {
  if ((addr >= boundary_start) && (addr <= boundary_end))
    return true;
  else if (startB) {
    if (addr <= startAddr)
      return true;
    else if (endB) {
      if (addr >= endAddr)
	return true;
      else 
	return false;
    } else 
      return false;
  } else if (endB) {
    if (addr >= endAddr)
      return true;
    else
      return false;
  } else
    return false;
}

#ifdef NOT_DEFINED
// as per getAllFunctions, but filters out those excluded with 
// e.g. mdl "exclude" command....
// Note that unlike several other paradynd classes, we do
// not allocate a seperate list of some functions , but rather
// filter the list of all functions whenevr queried for list 
// of included funtions.....
const vector<pd_Function*> &image::getIncludedFunctions() {
    unsigned int i;    
    includedFunctions.resize(0);
    vector<function_base *> *temp;
    vector<pd_Function *> *temp2;

    //cerr << "image::getIncludedFunctions called, name = " << name () << endl;
    //cerr << " mods = " << endl;
    //print_module_vector_by_short_name(string("  "), &mods);

    for (i=0;i<mods.size();i++) {
         temp = mods[i]->getIncludedFunctions();
         temp2 = (vector<pd_Function *> *) temp;
         includedFunctions += *temp2;
    }

    //cerr << " (image::getIncludedFunctions) returning : " << endl;
    //print_func_vector_by_pretty_name(string("  "), \
    //		 (vector<function_base*>*)&includedFunctions);

    // what about shared objects????
    return includedFunctions;
}
#endif

//  COMMENTS??  HELLO?
//  Here's a guess what this function tries to do:
//  a. look through (data member of type Object) linkedFile,
//   trying to file DYNINSTstart and DYNINSTend sections (
//   beginning + ending of instrumentation code).
//  b. ditto for main function (note variant names under which
//   these functions are searched for)....
//  c. iterate over all functions in linkedFile:
//    if the function has already been defined, OR shadows the
//     main function (has same address) - ignore it.
//    else - 
//     use (member function) inLibrary() to check
//     whether function is in range of instrumented code (between
//     DYNINSTstart and DYNINSTend)
//     if in range - addInternalSymbol().
//     else addOneFunction().
//
//  Questions : what do params mean????
bool image::addAllFunctions(vector<Symbol> &mods,
			    pdmodule *lib, pdmodule *dyn, 
			    const bool startB, const Address startAddr,
			    const bool endB, const Address endAddr) {

  Address boundary_start, boundary_end;
  Symbol lookUp;
  string symString;

#ifdef BPATCH_LIBRARY
  boundary_start = boundary_end = NULL;
#else
  if (!linkedFile.get_symbol(symString="DYNINSTfirst", lookUp) &&
      !linkedFile.get_symbol(symString="_DYNINSTfirst", lookUp)) {
    //statusLine("Internal symbol DYNINSTfirst not found");
    //showErrorCallback(31, "Internal symbol DYNINSTfirst not found");
    //return false;
    boundary_start = NULL;
  } else
    boundary_start = lookUp.addr();

  if (!linkedFile.get_symbol(symString="DYNINSTend", lookUp) &&
      !linkedFile.get_symbol(symString="_DYNINSTend", lookUp)) {
    //statusLine("Internal symbol DYNINSTend not found");
    //showErrorCallback(32, "Internal symbol DYNINSTend not found");
    //return false;
    boundary_end = NULL;
  } else
    boundary_end = lookUp.addr();
#endif

  Symbol mainFuncSymbol;  //Keeps track of info on "main" function

  //Checking "main" function names in same order as in the inst-*.C files
  if (linkedFile.get_symbol(symString="main",     lookUp) ||
      linkedFile.get_symbol(symString="_main",    lookUp) ||
      linkedFile.get_symbol(symString="WinMain",  lookUp) ||
      linkedFile.get_symbol(symString="_WinMain", lookUp)) 
    mainFuncSymbol = lookUp;

  // find the real functions -- those with the correct type in the symbol table
  for(SymbolIter symIter = linkedFile; symIter;symIter++) {
    const Symbol &lookUp = symIter.currval();

    if (funcsByAddr.defines(lookUp.addr()) ||
        ((lookUp.addr() == mainFuncSymbol.addr()) &&
         (lookUp.name() != mainFuncSymbol.name()))) {
      // This function has been defined 
      // *or*
      // This function has the same address as the "main" function but does 
      //   not have the same name as the "main" function.  Therefore, skip
      //   it and let the "main" function be eventually associated with this
      //   function address.  If we don't do this, paradynd will not have a
      //   "main" function to start work with.
      ;
    } else if (lookUp.type() == Symbol::PDST_FUNCTION) {
      if (!isValidAddress(lookUp.addr())) {
	string msg;
	char tempBuffer[40];
	sprintf(tempBuffer,"%x",lookUp.addr());
	msg = string("Function") + lookUp.name() + string("has bad address ") +
	      string(tempBuffer);
	statusLine(msg.string_of());
	showErrorCallback(29, msg);
	return false;
      }
      insert_function_internal_static(mods, lookUp, boundary_start,
              boundary_end, startAddr, startB, endAddr, endB, dyn, lib);
    }
  }

  // now find the pseudo functions -- this gets ugly
  // kludge has been set if the symbol could be a function
  for(SymbolIter symIter2 = linkedFile;symIter2;symIter2++) {
    lookUp = symIter2.currval();
    if (funcsByAddr.defines(lookUp.addr())) {
        // This function has been defined
        ;
    } else if ((lookUp.type() == Symbol::PDST_OBJECT) && lookUp.kludge()) {
        //logLine(P_strdup(symString.string_of()));
        // see comment under image::insert_function_internal_static,
        // below....
        insert_function_internal_static(mods, lookUp, boundary_start, \
              boundary_end, startAddr, startB, endAddr, endB, dyn, lib);
    }
  }
  return true;
}

bool image::addAllSharedObjFunctions(vector<Symbol> &mods,
			    pdmodule *lib, pdmodule *dyn) {

  Symbol lookUp;
  string symString;

  bool is_libdyninstRT = false; // true if this image is libdyninstRT
#if defined(i386_unknown_nt4_0)
  if (linkedFile.get_symbol(symString="_DYNINSTfirst", lookUp)
      || linkedFile.get_symbol(symString="_DYNINSTfirst", lookUp))
    is_libdyninstRT = true;
#endif

  // find the real functions -- those with the correct type in the symbol table
  for(SymbolIter symIter3 = linkedFile;symIter3;symIter3++) { 
    const Symbol &lookUp = symIter3.currval();
    if (funcsByAddr.defines(lookUp.addr())) {
      // This function has been defined
      ;
    } else if (lookUp.type() == Symbol::PDST_FUNCTION) {
      if (!isValidAddress(lookUp.addr())) {
	string msg;
	char tempBuffer[40];
	sprintf(tempBuffer,"%x",lookUp.addr());
	msg = string("Function") + lookUp.name() + string("has bad address ") +
	      string(tempBuffer);
	statusLine(msg.string_of());
	showErrorCallback(29, msg);
	return false;
      }
      // see comment under image::insert_functions_internal_dynamic,
      //  below....
      insert_function_internal_dynamic(mods, lookUp, dyn, lib, \
				       is_libdyninstRT);
    }
  }

  // now find the pseudo functions -- this gets ugly
  // kludge has been set if the symbol could be a function
  for(SymbolIter symIter4 = linkedFile;symIter4;symIter4++) {
    const Symbol &lookUp = symIter4.currval();
    if (funcsByAddr.defines(lookUp.addr())) {
      // This function has been defined
      ;
    } else if ((lookUp.type() == Symbol::PDST_OBJECT) && lookUp.kludge()) {
      pd_Function *pdf;
      addInternalSymbol(lookUp.name(), lookUp.addr());
      defineFunction(dyn, lookUp, TAG_LIB_FUNC, pdf);
    }
  }
  return true;
} 


bool image::addAllVariables()
{
/* Eventually we'll have to do this on all platforms (because we'll retrieve
 * the type information here).
 */
#ifdef i386_unknown_nt4_0
  string mangledName; 
  Symbol symInfo;
  SymbolIter symIter(linkedFile);

  while (symIter.next(mangledName, symInfo)) {
     const string &mangledName = symIter.currkey();
     const Symbol &symInfo = symIter.currval();

    if (symInfo.type() == Symbol::PDST_OBJECT) {
      char *name = cplus_demangle((char *)mangledName.string_of(), 0);
      const char *unmangledName;
      if (name) unmangledName = name;
      else unmangledName = mangledName.string_of();
      if (varsByPretty.defines(unmangledName)) {
	  *(varsByPretty[unmangledName]) += string(mangledName);
      } else {
	  vector<string> *varEntry = new vector<string>;
	  *varEntry += string(mangledName);
	  varsByPretty[unmangledName] = varEntry;
      }
      if (name) free(name);
    }
  }
#endif
  return true;
}

int symCompare(const void *s1, const void *s2) {
  const Symbol *sym1 = (const Symbol*)s1, *sym2 = (const Symbol*)s2;
  // TODO mdc
  return (sym1->addr() - sym2->addr());
}


unsigned int_addrHash(const unsigned &addr) {
  return addr;
}

// Please note that this is now machine independent-almost.  Let us keep it that way
// COMMENTS????
//  Guesses as to what this cod does....
//  image constructor:
//   construct an image corresponding to the executable file on path
//   fileName.  Fill in err based on success or failure....
image::image(const string &fileName, bool &err)
:   modsByFileName(string::hash),
    modsByFullName(string::hash),
    includedFunctions(0),
    excludedFunctions(string::hash),
    instrumentableFunctions(0),
    notInstruFunctions(string::hash),
    funcsByAddr(addrHash4),
    funcsByPretty(string::hash),
    file_(fileName),
    linkedFile(fileName, pd_log_perror),
    iSymsMap(string::hash),
    varsByPretty(string::hash),
    knownJumpTargets(int_addrHash, 8192)
{
    sharedobj_cerr << "image::image for non-sharedobj; file name=" << \
      file_ << endl;

    // shared_object and static object (a.out) constructors merged into
    //  common initialize routine.... 
    initialize(fileName, err, 0);
}

// 
// load a shared object
//
image::image(const string &fileName, u_int baseAddr, bool &err)
:   
    modsByFileName(string::hash),
    modsByFullName(string::hash),
    includedFunctions(0),
    excludedFunctions(string::hash),
    instrumentableFunctions(0),
    notInstruFunctions(string::hash),
    funcsByAddr(addrHash4),
    funcsByPretty(string::hash),
    file_(fileName),
    linkedFile(fileName, baseAddr,pd_log_perror),
    iSymsMap(string::hash),
    varsByPretty(string::hash),
    knownJumpTargets(int_addrHash, 8192)
{
sharedobj_cerr << "welcome to image::image for shared obj; file name=" << file_ << endl;

    // shared_object and static object (a.out) constructors merged into
    //  common initialize routine.... 
    initialize(fileName, err, 1, baseAddr);
}

static bool findStartSymbol(Object &lf, Address &adr) {
  Symbol lookUp;

  if (lf.get_symbol("DYNINSTstartUserCode", lookUp) ||
      lf.get_symbol("_DYNINSTstartUserCode", lookUp)) {
    adr = lookUp.addr();
    return true;
  } else
    return false;
}

static bool findEndSymbol(Object &lf, Address &adr) {
  Symbol lookUp;

  if (lf.get_symbol("DYNINSTendUserCode", lookUp) ||
      lf.get_symbol("_DYNINSTendUserCode", lookUp)) {
    adr = lookUp.addr();
    return true;
  } else
    return false;
}

/*
    image::initialize - called by image a.out and shared_object 
    constructor to do common initialization....

    paramaters - 

      shared_object - pass 0 to denote initializing from statically
	linked executable (a.out file), pass 1 to denote initializing
   	from shared library.
      base_addr - curr. used IFF shared_library == 1.
 */
void image::initialize(const string &fileName, bool &err, \
	bool shared_object, u_int) {

    // initialize (data members) codeOffset_, dataOffset_, \
    //  codeLen_, dataLen_.
    codeOffset_ = linkedFile.code_off();
    dataOffset_ = linkedFile.data_off();
    codeLen_ = linkedFile.code_len();
    dataLen_ = linkedFile.data_len();

#if defined(hppa1_1_hp_hpux)
  unwind   = linkedFile.unwind;
#endif

    // if unable to parse object file (somehow??), try to
    //  notify luser/calling process + return....    
    if (!codeLen_ || !linkedFile.code_ptr()) {
        string msg = string("Unable to open executable file: ") + fileName;
        statusLine(msg.string_of());
        msg += "\n";
        logLine(msg.string_of());
        err = true;
        showErrorCallback(27, msg); 
        return;
    }

#if !defined(i386_unknown_nt4_0) && !defined(USES_LIBDYNINSTRT_SO)
    // on architectures where statically linked programs to be run 
    //  w/ paradyn need to link with the DYNINST library, try to find
    //  the paradyn lib version # (DYNINSTversion or _DYNINSTversion
    //  symbol).
    if (!shared_object) {
        Symbol version;
        if (!linkedFile.get_symbol("DYNINSTversion", version) &&
                !linkedFile.get_symbol("_DYNINSTversion", version)) {
            statusLine("Could not find version number in instrumentation\n");
            showErrorCallback(33, "Could not find version number in instrumentation");
            err = true;
            return;
        }

        Word version_number = get_instruction(version.addr());
        if (version_number != 1) {
            string msg;
            msg = string("Incorrect version number, expected ") + string(1) + 
	         string("found ") + string(version_number);
            statusLine(msg.string_of());
            showErrorCallback(30, msg);
            err = true;
            return;
        }
    }
#endif

    string msg;
    // give luser some feedback....
    if (shared_object) {
	msg = string("Parsing shared object file : ") + fileName;
    } else {
	msg = string("Parsing static object file : ") + fileName;
    }
    statusLine(msg.string_of());

    const char *nm = fileName.string_of();
    const char *pos = P_strrchr(nm, '/');

    err = false;
    if (pos)
        name_ = pos + 1;
    else
        name_ = fileName;

    // use the *DUMMY_MODULE* until a module is defined
    pdmodule *dynModule = newModule(DYN_MODULE, 0);
    pdmodule *libModule = newModule(LIBRARY_MODULE, 0);
    // TODO -- define inst points in define function ?

    // The functions cannot be verified until all of them have been seen
    // because calls out of each function must be tagged as calls to user
    // functions or call to "library" functions

    Symbol lookUp;
    string symString;

    //
    // sort the modules by address into a vector to allow a binary search to 
    // determine the module that a symbol will map to -- this 
    // may be bsd specific....
    //
    vector <Symbol> tmods;

    for (SymbolIter symIter = linkedFile; symIter; symIter++) {
        const Symbol &lookUp = symIter.currval();
        if (lookUp.type() == Symbol::PDST_MODULE) {
            const string &lookUpName = lookUp.name();
            const char *str = lookUpName.string_of();
            assert(str);
            int ln = lookUpName.length();

            // directory definition -- ignored for now
            if (str[ln-1] != '/') {
	        tmods += lookUp;
	    }
        }
    }

    // sort the modules by address
    statusLine("sorting modules");
    tmods.sort(symCompare);
    //  assert(mods.sorted(symCompare));

    // remove duplicate entries -- some .o files may have the same 
    // address as .C files.  kludge is true for module symbols that 
    // I am guessing are modules
    vector<Symbol> uniq;
    unsigned loop=0;

    // must use loop+1 not mods.size()-1 since it is an unsigned compare
    //  which could go negative - jkh 5/29/95
    for (loop=0; loop < tmods.size(); loop++) {
        if ((loop+1 < tmods.size()) && 
	        (tmods[loop].addr() == tmods[loop+1].addr())) {
            if (!tmods[loop].kludge())
	        tmods[loop+1] = tmods[loop];
        } 
	else
          uniq += tmods[loop];
    }

    // define all of the functions
    statusLine("winnowing functions");

    // register functions differently depending on whether parsing 
    //  shared object or static executable.  
    // THIS MAY CHANGE WHEN REMOVE DYNINSTstart && DYNINSTend hacks....
    if (!shared_object) {
        // find the "user" code boundaries
        statusLine("finding user code boundaries");
        Address startUserAddr=0, endUserAddr=0;
        bool startBound = findStartSymbol(linkedFile, startUserAddr);
        bool endBound = findEndSymbol(linkedFile, endUserAddr);

	if (!addAllFunctions(uniq, libModule, dynModule, startBound, \
		startUserAddr, endBound, endUserAddr)) {
            err = true;
            return;
        }
    } else {
	// define all of the functions
        if (!addAllSharedObjFunctions(uniq, libModule, dynModule)) {
            err = true;
            return;
        }
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
    dictionary_hash<unsigned, unsigned> addr_dict(uiHash);
    vector<pd_Function*> temp_vec;

    // question??  Necessary to do same crap to includedFunctions &&
    //  excludedFunctions??
    unsigned f_size = instrumentableFunctions.size(), index;
    for (index=0; index<f_size; index++) {
        const unsigned the_address = (unsigned)instrumentableFunctions[index]\
	                                       ->getAddress(0);
        if (!addr_dict.defines(the_address)) {
            addr_dict[the_address] = 1;
            temp_vec += instrumentableFunctions[index];
        }
    }
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

// passing in tags allows a function to be tagged as TAG_LIB_FUNC even
// if its entry is not in the tag dictionary of known functions
bool image::defineFunction(pdmodule *use, const Symbol &sym, const unsigned tags,
			   pd_Function *&retFunc) {
  // We used to skip a leading underscore, but not anymore.
  // (I forgot why we ever did in the first place)

  unsigned dictTags = findTags(sym.name());
  return (newFunc(use, sym.name(), sym.addr(), sym.size(), tags | dictTags, retFunc));
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

// COMMENTS????
// looks like wrapper function for (member function) newFunc.
//  appears to do some fiddling with tags paramater to that
//  function.
// Kludge TODO - internal functions are tagged with TAG_LIB_FUNC
// but they won't have tags in the tag dict, so this happens...
bool image::defineFunction(pdmodule *libModule, const Symbol &sym,
			   const string &modName, const Address modAddr,
			   pd_Function *&retFunc) {

  // We used to skip a leading underscore, but not any more.

  unsigned tags = findTags(sym.name());

  if (TAG_LIB_FUNC & tags)
    return (newFunc(libModule, sym.name(), sym.addr(), sym.size(),
		    tags | TAG_LIB_FUNC, retFunc));
  else {
    pdmodule *use = getOrCreateModule(modName, modAddr);
    assert(use);
    return (newFunc(use, sym.name(), sym.addr(), sym.size(), tags, retFunc));
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
		       const unsigned tg, const image *owner, bool &err) : 
  function_base(symbol, pretty, adr, size,tg),
  file_(f),
  funcEntry_(0),
  relocatable_(false)
{
  err = findInstPoints(owner) == false;
}

// image::addAllFunctions dupliactes this section of code.
// As part of an effort to clean up the code a bit, I merged the
// duplicates into this 1 function.  
void image::insert_function_internal_static(vector<Symbol> &mods, \
	Symbol &lookUp, \
        const Address boundary_start,  const Address boundary_end, \
	const Address startAddr, bool startB, const Address endAddr, \
        bool endB, pdmodule *dyn, pdmodule *lib) {
    pd_Function *pdf;
    //  COMMENTS??
    //  If CONDITION??
    if (inLibrary(lookUp.addr(), boundary_start, boundary_end,
		    startAddr, startB, endAddr, endB)) {
        // look at symbol name.  If it looks like "_DYNINST*" or 
	// "_TRACELIN*", add it to iSymsMap data member....
	addInternalSymbol(lookUp.name(), lookUp.addr());
	defineFunction(dyn, lookUp, TAG_LIB_FUNC, pdf);
    } else {
	addOneFunction(mods, lib, dyn, lookUp, pdf);
    } 
}

// version of insert_function_internal_static (above) for 
// shared objects....
void image::insert_function_internal_dynamic(vector<Symbol> &mods,\
	Symbol &lookUp, \
        pdmodule *dyn, pdmodule *lib, bool is_libdyninstRT) {
    pd_Function *pdf;
    if (is_libdyninstRT) {
        addInternalSymbol(lookUp.name(), lookUp.addr());
	defineFunction(dyn, lookUp, TAG_LIB_FUNC, pdf);
    } else {
        addOneFunction(mods, lib, dyn, lookUp, pdf);
    }
}
