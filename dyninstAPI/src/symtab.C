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


/* imported from platform specific library list.  This is lists all
   library functions we are interested in instrumenting. */

pdmodule *image::newModule(const string &name, const Address addr)
{
    pdmodule *ret;
    // modules can be defined several times in C++ due to templates and
    //   in-line member functions.
    if ((ret = findModule(name)))
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
    modsByFileName[ret->fileName()] = ret;
    modsByFullName[ret->fullName()] = ret;
    mods += ret;
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
bool image::newFunc(pdmodule *mod, const string &name, const Address addr, 
		    const unsigned size, const unsigned tag, 
		    pd_Function *&retFunc) {
  pd_Function *func;
  retFunc = NULL;
  // KLUDGE
  if ((func = findFunction(addr))){
    string temp = name;
    temp += string(" findFunction failed\n");
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
  //cout << name << " pretty: " << demangled << " addr :" << addr <<endl;
  retFunc = func;
  if (err) {
    //delete func;
    retFunc = NULL;
    notInstruFunction += func;
    return false;
  }
  
  funcsByAddr[addr] = func;
  mod->funcs += func;

  vector<pd_Function*> *funcsByPrettyEntry;
  if (!funcsByPretty.find(func->prettyName(), funcsByPrettyEntry)) {
    funcsByPrettyEntry = new vector<pd_Function*>;
    funcsByPretty[func->prettyName()] = funcsByPrettyEntry;
  }

  // several functions may have the same demangled name, and each one
  // will appear in a different module
//  vector<pdFunction*> *ap = funcsByPretty[func->prettyName()];
  assert(funcsByPrettyEntry);
  (*funcsByPrettyEntry) += func;
  return true;
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

pdmodule *image::findModule(const string &name)
{
  if (modsByFileName.defines(name))
    return (modsByFileName[name]);
  else if (modsByFullName.defines(name))
    return (modsByFullName[name]);
  else
    return NULL;
}

// TODO -- this is only being used in cases where only one function
// should exist -- should I assert that the vector size <= 1 ?
pd_Function *image::findOneFunction(const string &name)
{
  string demangName;

  if (funcsByPretty.defines(name)) {
    vector<pd_Function*> *a = funcsByPretty[name];
    assert(a);
    if (!a->size())
      return NULL;
    else
      return ((*a)[0]);
  } else if (buildDemangledName(name, demangName)) {
    if (funcsByPretty.defines(demangName)) {
      vector<pd_Function*> *a = funcsByPretty[demangName];
      assert(a);
      if (!a->size())
	return NULL;
      else
	return ((*a)[0]);
    } else
      return NULL;
  } else
    return NULL;
}

// This function supposely is only used to find function that
// is not instrumentable which may not be totally defined.
// Use with caution.  
pd_Function *image::findOneFunctionFromAll(const string &name) {

    pd_Function *ret;
    if ((ret = findOneFunction(name))) 
	return ret;
    else {
	for (unsigned i = 0; i < notInstruFunction.size(); i++) {
	    ret = notInstruFunction[i];
	    if (ret->prettyName() == name)
		return ret;
	}
    }
    return NULL;
}

bool image::findFunction(const string &name, vector<pd_Function*> &retList) {

  if (funcsByPretty.defines(name)) {
    retList = *funcsByPretty[name];
    return true;
  } else
    return false;
}

pd_Function *image::findFunction(const Address &addr) 
{
  pd_Function *result; // filled in by find()
  if (funcsByAddr.find(addr, result))
     return result;
  else
     return NULL;
}
  
pd_Function *image::findFunctionIn(const Address &addr,const process *p) const 
{
  pd_Function *pdf;

  dictionary_hash_iter<Address, pd_Function*> mi(funcsByAddr);
  Address adr;
  while (mi.next(adr, pdf)) {
      if ((addr>=pdf->getAddress(p))&&(addr<=(pdf->getAddress(p)+pdf->size()))) 
	  return pdf;
  }
  
  // If not found, we are going to search them in the 
  // uninstrumentable function
  for (unsigned i = 0; i < notInstruFunction.size(); i++) {
      pdf = notInstruFunction[i];
      if ((addr>=pdf->getAddress(p))&&(addr<=(pdf->getAddress(p)+pdf->size()))) 
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
    mod = ret->findModule(res->part_name());
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
  string pds; pdmodule *mod;
  dictionary_hash_iter<string, pdmodule*> mi(modsByFileName);

  while (mi.next(pds, mod)){
    mod->define();
  }

#ifdef DEBUG_MDL
  char buffer[100];
  ostrstream osb(buffer, 100, ios::out);
  osb << "IMAGE_" << name() << "__" << getpid() << ends;
  ofstream of(buffer, ios::app);
  unsigned n_size = mdlNormal.size();
  of << "NORMAL\n";
  for (unsigned ni=0; ni<n_size; ni++) {
    of << mdlNormal[ni]->prettyName() << "\t\t" << mdlNormal[ni]->addr() << "\t\t" << 
      mdlNormal[ni]->isLibTag() << endl;
  }
  n_size = mdlLib.size();
  of << "\n\nLIB\n";
  for (ni=0; ni<n_size; ni++) {
    of << mdlLib[ni]->prettyName() << "\t\t" << mdlLib[ni]->addr() << "\t\t" <<
      mdlLib[ni]->isLibTag() << endl;
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
    of << fileName << ":  " << pdf->prettyName() <<  "  " <<
      pdf->isLibTag() << "  " << pdf->addr() << endl;
#endif
    // ignore line numbers for now 

#ifndef BPATCH_LIBRARY
    if (!(pdf->isLibTag())) {
      // see if we have created module yet.
      if (!modResource) {
	modResource = resource::newResource(moduleRoot, this,
					    nullString, // abstraction
					    fileName(), // name
					    0.0, // creation time
					    string::nil, // unique-ifier
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

  resource::send_now();
}

static inline bool findStartSymbol(Object &lf, Address &adr) {
  Symbol lookUp;

  if (lf.get_symbol("DYNINSTstartUserCode", lookUp) ||
      lf.get_symbol("_DYNINSTstartUserCode", lookUp)) {
    adr = lookUp.addr();
    return true;
  } else
    return false;
}

static inline bool findEndSymbol(Object &lf, Address &adr) {
  Symbol lookUp;

  if (lf.get_symbol("DYNINSTendUserCode", lookUp) ||
      lf.get_symbol("_DYNINSTendUserCode", lookUp)) {
    adr = lookUp.addr();
    return true;
  } else
    return false;
}

// TODO this assumes an ordering in the symbol table wrt modules KLUDGE
static inline bool notInUserRange(const Address adr,
				  const bool start, const Address startAdr,
				  const bool end, const Address endAdr) {
  return ((start && (adr < startAdr)) || (end && (adr >= endAdr)));
}

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

bool image::addOneFunction(vector<Symbol> &mods, pdmodule *lib, pdmodule *dyn,
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

bool image::addAllFunctions(vector<Symbol> &mods,
			    pdmodule *lib, pdmodule *dyn, 
			    const bool startB, const Address startAddr,
			    const bool endB, const Address endAddr) {

  Address boundary_start, boundary_end;
  Symbol lookUp;
  string symString;
  SymbolIter symIter(linkedFile);

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
  while (symIter.next(symString, lookUp)) {

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
      pd_Function *pdf;
      if (inLibrary(lookUp.addr(), boundary_start, boundary_end,
		    startAddr, startB, endAddr, endB)) {
	addInternalSymbol(lookUp.name(), lookUp.addr());
	if (defineFunction(dyn, lookUp, TAG_LIB_FUNC, pdf)) {
	  assert(pdf); mdlLib += pdf;
	}
      } else {
	if (addOneFunction(mods, lib, dyn, lookUp, pdf)) {
	  assert(pdf); mdlNormal += pdf;
	}
      }
    }
  }

  // now find the pseudo functions -- this gets ugly
  // kludge has been set if the symbol could be a function
  symIter.reset();
  while (symIter.next(symString, lookUp)) {
    if (funcsByAddr.defines(lookUp.addr())) {
      // This function has been defined
      ;
    } else if ((lookUp.type() == Symbol::PDST_OBJECT) && lookUp.kludge()) {
      //logLine(P_strdup(symString.string_of()));
      pd_Function *pdf;
      if (inLibrary(lookUp.addr(), boundary_start, boundary_end,
		    startAddr, startB, endAddr, endB)) {
	addInternalSymbol(lookUp.name(), lookUp.addr());
	if (defineFunction(dyn, lookUp, TAG_LIB_FUNC, pdf)) {
	  assert(pdf); mdlLib += pdf;
	}
      } else {
	if (addOneFunction(mods, lib, dyn, lookUp, pdf)) {
	  assert(pdf); mdlNormal += pdf;
	}
      }
    }
  }
  return true;
}

bool image::addAllSharedObjFunctions(vector<Symbol> &mods,
			    pdmodule *lib, pdmodule *dyn) {

  Symbol lookUp;
  string symString;
  SymbolIter symIter(linkedFile);

  bool is_libdyninstRT = false; // true if this image is libdyninstRT
#if defined(i386_unknown_nt4_0)
  if (linkedFile.get_symbol(symString="_DYNINSTfirst", lookUp)
      || linkedFile.get_symbol(symString="_DYNINSTfirst", lookUp))
    is_libdyninstRT = true;
#endif

  // find the real functions -- those with the correct type in the symbol table
  while (symIter.next(symString, lookUp)) {

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
      pd_Function *pdf;
      if (is_libdyninstRT) {
	addInternalSymbol(lookUp.name(), lookUp.addr());
	if (defineFunction(dyn, lookUp, TAG_LIB_FUNC, pdf)) {
	  assert(pdf); mdlLib += pdf;
	}
      } else if (addOneFunction(mods, lib, dyn, lookUp, pdf)) {
        assert(pdf); mdlNormal += pdf;
      }
    }
  }

  // now find the pseudo functions -- this gets ugly
  // kludge has been set if the symbol could be a function
  symIter.reset();
  while (symIter.next(symString, lookUp)) {
    if (funcsByAddr.defines(lookUp.addr())) {
      // This function has been defined
      ;
    } else if ((lookUp.type() == Symbol::PDST_OBJECT) && lookUp.kludge()) {
      //logLine(P_strdup(symString.string_of()));
      pd_Function *pdf;
      addInternalSymbol(lookUp.name(), lookUp.addr());
      if (defineFunction(dyn, lookUp, TAG_LIB_FUNC, pdf)) {
          assert(pdf); mdlLib += pdf;
      }
    }
  }
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
// 
image::image(const string &fileName, bool &err)
:   funcsByAddr(addrHash4),
    modsByFileName(string::hash),
    modsByFullName(string::hash),
    file_(fileName),
    linkedFile(fileName, pd_log_perror),
    iSymsMap(string::hash),
    funcsByPretty(string::hash),
    knownJumpTargets(int_addrHash, 8192)
{
sharedobj_cerr << "image::image for non-sharedobj; file name=" << file_ << endl;

  codeOffset_ = linkedFile.code_off();
  dataOffset_ = linkedFile.data_off();
  codeLen_ = linkedFile.code_len();
  dataLen_ = linkedFile.data_len();

#if defined(hppa1_1_hp_hpux)
  unwind   = linkedFile.unwind;
#endif

  if (!codeLen_ || !linkedFile.code_ptr()) {
    string msg = string("Unable to open executable file: ") + fileName;
    statusLine(msg.string_of());

    msg += "\n";
    logLine(msg.string_of());

    err = true;

    showErrorCallback(27, msg); 
    return;
  }

#if !defined(i386_unknown_nt4_0)
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
#endif

  const char *nm = fileName.string_of();
  const char *pos = P_strrchr(nm, '/');

  err = false;
  if (pos)
    name_ = pos + 1;
  else
    name_ = fileName;

  // find the "user" code boundaries
  statusLine("finding user code boundaries");
  Address startUserAddr=0, endUserAddr=0;
  bool startBound = findStartSymbol(linkedFile, startUserAddr);
  bool endBound = findEndSymbol(linkedFile, endUserAddr);

  // use the *DUMMY_MODULE* until a module is defined
  pdmodule *dynModule = newModule(DYN_MODULE, 0);
  pdmodule *libModule = newModule(LIBRARY_MODULE, 0);
  // TODO -- define inst points in define function ?

  // The functions cannot be verified until all of them have been seen
  // because calls out of each function must be tagged as calls to user
  // functions or call to "library" functions

  Symbol lookUp;
  string symString;
  SymbolIter symIter(linkedFile);

  // sort the modules by address into a vector to allow a binary search to 
  // determine the module that a symbol will map to -- this may be bsd specific
  vector<Symbol> mods;

  while (symIter.next(symString, lookUp)) {

    if (lookUp.type() == Symbol::PDST_MODULE) {
      const string &lookUpName = lookUp.name();
      const char *str = lookUpName.string_of();
      assert(str);
      int ln = lookUpName.length();

      // directory definition -- ignored for now
      if (str[ln-1] != '/')
	mods += lookUp;
    }
  }

  // sort the modules by address
  statusLine("sorting modules");
  mods.sort(symCompare);
//  assert(mods.sorted(symCompare));

  // remove duplicate entries -- some .o files may have the same address as .C files
  // kludge is true for module symbols that I am guessing are modules
  vector<Symbol> uniq;
  unsigned loop=0;

  // must use loop+1 not mods.size()-1 since it is an unsigned compare
  //  which could go negative - jkh 5/29/95
  for (loop=0; loop < mods.size(); loop++) {
    if ((loop+1 < mods.size()) && 
	(mods[loop].addr() == mods[loop+1].addr())) {
      if (!mods[loop].kludge())
	mods[loop+1] = mods[loop];
    } else
      uniq += mods[loop];
  }

  // define all of the functions
  statusLine("winnowing functions");
  if (!addAllFunctions(uniq, libModule, dynModule, startBound, startUserAddr,
		       endBound, endUserAddr)) {
    err = true;
    return;
  }
  statusLine("checking call points");
  checkAllCallPoints();

//  statusLine("ready"); // this shouldn't be here, right? (cuz we're not really ready)

  // TODO -- remove duplicates -- see earlier note
  dictionary_hash<unsigned, unsigned> addr_dict(uiHash);
  vector<pd_Function*> temp_vec;
  unsigned f_size = mdlLib.size(), index;
  for (index=0; index<f_size; index++) {
    const unsigned the_address = (unsigned)mdlLib[index]->getAddress(0);
    if (!addr_dict.defines(the_address)) {
      addr_dict[the_address] = 1;
      temp_vec += mdlLib[index];
    }
  }
  mdlLib = temp_vec;
  temp_vec.resize(0); addr_dict.clear();
  f_size = mdlNormal.size();
  for (index=0; index<f_size; index++) {
    const unsigned the_address = (unsigned)mdlNormal[index]->getAddress(0);
    if (!addr_dict.defines(the_address)) {
      addr_dict[the_address] = 1;
      temp_vec += mdlNormal[index];
    }
  }
  mdlNormal = temp_vec;
}

// 
// load a shared object
//
image::image(const string &fileName, u_int baseAddr, bool &err)
:   funcsByAddr(addrHash4),
    modsByFileName(string::hash),
    modsByFullName(string::hash),
    file_(fileName),
    linkedFile(fileName, baseAddr,pd_log_perror),
    iSymsMap(string::hash),
    funcsByPretty(string::hash),
    knownJumpTargets(int_addrHash, 8192)
{
sharedobj_cerr << "welcome to image::image for shared obj; file name=" << file_ << endl;

  codeOffset_ = linkedFile.code_off();
  dataOffset_ = linkedFile.data_off();
  codeLen_ = linkedFile.code_len();
  dataLen_ = linkedFile.data_len();
  //logLine("IN image::image\n");

#if defined(hppa1_1_hp_hpux)
  unwind   = linkedFile.unwind;
#endif

  if (!codeLen_ || !linkedFile.code_ptr()) {
    string msg = string("Unable to open shared object file: ") + fileName;
    statusLine(msg.string_of());
    
    msg += "\n";
    logLine(msg.string_of());

    err = true;
    showErrorCallback(27, msg); 

    assert(false);

    return;
  }
  else {
    string msg = string("Parsing shared object file: ") + fileName;
    statusLine(msg.string_of());
  }

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
  SymbolIter symIter(linkedFile);

  // sort the modules by address into a vector to allow a binary search to 
  // determine the module that a symbol will map to -- this may be bsd specific
  vector<Symbol> mods;

  while (symIter.next(symString, lookUp)) {

    if (lookUp.type() == Symbol::PDST_MODULE) {
      const char *str = (lookUp.name()).string_of();
      assert(str);
      int ln = P_strlen(str);

      // directory definition -- ignored for now
      if (str[ln-1] != '/')
	mods += lookUp;
      // string temp = string("NEW MODULE: ");
      // temp += lookUp.name();
      // temp += string("\n");
      // logLine(P_strdup(temp.string_of()));
    }
  }

  // sort the modules by address
  mods.sort(symCompare);
//  assert(mods.sorted(symCompare));

  // remove duplicate entries -- some .o files may have the same address as .C files
  // kludge is true for module symbols that I am guessing are modules
  vector<Symbol> uniq;
  unsigned loop=0;

  // must use loop+1 not mods.size()-1 since it is an unsigned compare
  //  which could go negative - jkh 5/29/95
  for (loop=0; loop < mods.size(); loop++) {
    if ((loop+1 < mods.size()) && 
	 (mods[loop].addr() == mods[loop+1].addr())) {
      if (!mods[loop].kludge())
	mods[loop+1] = mods[loop];
    } else
      uniq += mods[loop];
  }

  // define all of the functions
  if (!addAllSharedObjFunctions(uniq, libModule, dynModule)) {
    err = true;
    return;
  }
  checkAllCallPoints();

  // TODO -- remove duplicates -- see earlier note
  dictionary_hash<unsigned, unsigned> addr_dict(uiHash);
  vector<pd_Function*> temp_vec;
  unsigned f_size = mdlLib.size(), index;
  for (index=0; index<f_size; index++) {
    const unsigned the_address = (unsigned)mdlLib[index]->getAddress(0);
    if (!addr_dict.defines(the_address)) {
      addr_dict[the_address] = 1;
      temp_vec += mdlLib[index];
    }
  }
  mdlLib = temp_vec;
  temp_vec.resize(0); addr_dict.clear();
  f_size = mdlNormal.size();
  for (index=0; index<f_size; index++) {
    const unsigned the_address = (unsigned)mdlNormal[index]->getAddress(0);
    if (!addr_dict.defines(the_address)) {
      addr_dict[the_address] = 1;
      temp_vec += mdlNormal[index];
    }
  }
  mdlNormal = temp_vec;

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

// KLUDGE TODO - internal functions are tagged with TAG_LIB_FUNC
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

