/*
 *  Copyright 1993 Jeff Hollingsworth.  All rights reserved.
 *
 */

#ifndef lint
static char Copyright[] = "@(#) Copyright (c) 1993 Jeff Hollingsowrth\
    All rights reserved.";

static char rcsid[] = "@(#) $Header: /home/jaw/CVSROOT_20081103/CVSROOT/core/paradynd/src/Attic/symtab.C,v 1.22 1995/02/16 08:34:58 markc Exp $";
#endif

/*
 * symtab.C - generic symbol routines.  Implements an ADT for a symbol
 *   table.  The make calls to machine and a.out specific routines to handle
 *   the implementation dependent parts.
 *
 * $Log: symtab.C,v $
 * Revision 1.22  1995/02/16 08:34:58  markc
 * Changed igen interfaces to use strings/vectors rather than char*/igen-arrays
 * Changed igen interfaces to use bool, not Boolean.
 * Cleaned up symbol table parsing - favor properly labeled symbol table objects
 * Updated binary search for modules
 * Moved machine dependnent ptrace code to architecture specific files.
 * Moved machine dependent code out of class process.
 * Removed almost all compiler warnings.
 * Use "posix" like library to remove compiler warnings
 *
 * Revision 1.21  1994/12/15  07:39:53  markc
 * make resourceBatch request prior to defining resources.
 *
 * Revision 1.20  1994/11/10  21:01:21  markc
 * Turn off creation of MODS file.
 * Don't assume a .o is a module until all of the real modules have been seen.
 *
 * Revision 1.19  1994/11/10  18:58:19  jcargill
 * The "Don't Blame Me Either" commit
 *
 * Revision 1.18  1994/11/09  18:40:38  rbi
 * the "Don't Blame Me" commit
 *
 * Revision 1.17  1994/11/02  11:17:23  markc
 * Made symbol table parsing machine independent.
 *
 * Revision 1.15  1994/10/13  07:25:04  krisna
 * solaris porting and updates
 *
 * Revision 1.14  1994/10/04  21:40:12  jcargill
 * Removed requirement that functions have valid line-number information to
 * be consider user functions.
 *
 * Revision 1.13  1994/09/30  19:47:16  rbi
 * Basic instrumentation for CMFortran
 *
 * Revision 1.12  1994/09/22  02:26:40  markc
 * Made structs classes
 *
 * Revision 1.11  1994/08/08  20:13:47  hollings
 * Added suppress instrumentation command.
 *
 * Revision 1.10  1994/08/02  18:25:07  hollings
 * fixed modules to use list template for lists of functions.
 *
 * Revision 1.9  1994/07/28  22:40:48  krisna
 * changed definitions/declarations of xalloc functions to conform to alloc.
 *
 * Revision 1.8  1994/07/22  19:21:10  hollings
 * removed mistaken divid by 1Meg for predicted cost.
 *
 * Revision 1.7  1994/07/20  23:23:41  hollings
 * added insn generated metric.
 *
 * Revision 1.6  1994/07/12  19:26:15  jcargill
 * Added internal prefix for TRACELIB
 *
 * Revision 1.5  1994/06/29  02:52:51  hollings
 * Added metricDefs-common.{C,h}
 * Added module level performance data
 * cleanedup types of inferrior addresses instrumentation defintions
 * added firewalls for large branch displacements due to text+data over 2meg.
 * assorted bug fixes.
 *
 * Revision 1.4  1994/06/27  21:28:23  rbi
 * Abstraction-specific resources and mapping info
 *
 * Revision 1.3  1994/06/27  18:57:15  hollings
 * removed printfs.  Now use logLine so it works in the remote case.
 * added internalMetric class.
 * added extra paramter to metric info for aggregation.
 *
 * Revision 1.2  1994/05/16  22:31:55  hollings
 * added way to request unique resource name.
 *
 * Revision 1.1  1994/01/27  20:31:45  hollings
 * Iinital version of paradynd speaking dynRPC igend protocol.
 *
 * Revision 1.8  1993/12/13  19:57:20  hollings
 * added nearest line match for function to support Fortran convention of making
 * first sline record be the first executable statement in the function.
 *
 * Revision 1.7  1993/10/12  20:10:35  hollings
 * zero memory on a realloc.
 *
 * Revision 1.6  1993/10/04  21:39:08  hollings
 * made missing internal symbols a fatal condition.
 *
 * Revision 1.5  1993/08/23  23:16:05  hollings
 * added third parameter to findInternalSymbol.
 *
 * Revision 1.4  1993/08/20  22:02:39  hollings
 * removed unused local variables.
 *
 * Revision 1.3  1993/07/13  18:32:46  hollings
 * new include file syntax.
 * added name demangler.
 *
 * Revision 1.2  1993/06/08  20:14:34  hollings
 * state prior to bc net ptrace replacement.
 *
 * Revision 1.1  1993/03/19  22:45:45  hollings
 * Initial revision
 *
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

// TODO - machine independent instruction functions
#include "symtab.h"
#include "arch-sparc.h"
#include "util/h/Object.h"
#include <fstream.h>
#include "util.h"
#include "dyninstP.h"
#include "util/h/String.h"
#include "inst.h"
#include "main.h"
#include "util/h/Timer.h"
#include "init.h"

dictionary_hash<string, image*> image::allImages(string::hash);

resource *moduleRoot=NULL;
extern "C" char *cplus_demangle(char *, int);

/* imported from platform specific library list.  This is lists all
   library functions we are interested in instrumenting. */

module *image::newModule(const string &name, const Address addr)
{
    module *ret;
    // modules can be defined several times in C++ due to templates and
    //   in-line member functions.
    if (ret = findModule(name))
      return(ret);
    ret = new module;

    char *out = P_strdup(name.string_of());
    char *sl = P_strrchr(out, '/');
    if (sl) {
      *sl = (char)0;
      ret->fullName = out;
      ret->fileName = sl+1;
    } else {
      ret->fullName = string("/default/") + out;
      ret->fileName = out;
    }
    delete out;

    ret->language = langUnknown;
    ret->addr = addr;
    ret->exec = this;
    modsByFileName[ret->fileName] = ret;
    modsByFullName[ret->fullName] = ret;
    return(ret);
}


// TODO -- is this g++ specific
bool buildDemangledName(const string mangled, string &use)
{
    char *tempName = P_strdup(mangled.string_of());
    char *demangled = cplus_demangle(tempName, 0);

    if (demangled) {
      use = demangled;
      delete tempName;
      delete demangled;
      return true;
    } else {
      delete tempName;
      return false;
    }
}

// err is true if the function can't be defined
bool image::newFunc(module *mod, const string name, const Address addr,
		    const unsigned tag, bool &err)
{
  pdFunction *func;
  // KLUDGE
  if (func = findFunction(addr))
    return false;

  if (!mod) {
    logLine("Error function without module\n");
    P_abort();
  }
  
  char *out = P_strdup(name.string_of());
  char *p = P_strchr(out, ':');
  if (p) *p = '\0';

  string demangled;
  if (!buildDemangledName(out, demangled)) 
    demangled = out;
  delete out;

  bool err;
  func = new pdFunction(name, demangled, mod, addr, tag, this, err);
  if (err)
    return false;
  
  funcsByAddr[addr] = func;
  
  mod->funcMap[func->prettyName()] = func;

  if (!funcsByPretty.defines(func->prettyName())) {
    vector<pdFunction*> *a1 = new vector<pdFunction*>;
    funcsByPretty[func->prettyName()] = a1;      
  }

  // several functions may have the same demangled name, and each one
  // will appear in a different module
  vector<pdFunction*> *ap = funcsByPretty[func->prettyName()];
  assert(ap);
  (*ap) += func;
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
#ifdef DEBUG_TIME
  timer parseTimer;
  parseTimer.start();
#endif /* DEBUG_TIME */

  /*
   * Check to see if we have parsed this image at this offeset before.
   */
  string pds; image *ip;
  dictionary_hash_iter<string, image*> ii(image::allImages);
  // TODO -- better method to detect same image/offset --> offset only for CM5
  while (ii.next(pds, ip))
    if (file == ip->file())
      return ip;

  /*
   * load the symbol table. (This is the a.out format specific routine).
   */
  statusLine("loading symbol table");
  bool err;

#ifdef DEBUG_TIME
  loadTimer.clear(); loadTimer.start();
  if (!timeOut) timeOut = fopen("/tmp/paradynd_times", "w");
  if (!timeOut) P_abort();
#endif /* DEBUG_TIME */

  image *ret = new image(file, err);
  if (err || !ret)
    return NULL;

  // Add to master image list.
  image::allImages[ret->name()] = ret;

  if (!moduleRoot)
    moduleRoot = newResource(rootResource, NULL, (char*)NULL, "Procedure", 0.0, "");

#ifdef DEBUG_TIME
  char sline[256];
  timer defTimer;
  defTimer.start();
#endif /* DEBUG_TIME */

  // define all modules.
  ret->defineModules();

#ifdef DEBUG_TIME
  defTimer.stop();
  fprintf(timeOut, "It took %f:user %f:system %f:wall seconds to define resources %s\n",
	  defTimer.usecs(), defTimer.ssecs(),
	  defTimer.wsecs(), file.string_of());
  timer postTimer;
  postTimer.start();
#endif /* DEBUG_TIME */

  //
  // Optional post-processing step.  
  if (ret->symbolExists("CMRT_init"))
    ret->postProcess((char*)NULL);

#ifdef DEBUG_TIME
  postTimer.stop();
  fprintf(timeOut, "It took %f:user %f:system %f:wall seconds to post process %s\n",
	  postTimer.usecs(), postTimer.ssecs(),
	  postTimer.wsecs(), file.string_of());
  parseTimer.stop();
  fprintf(timeOut, "It took %f:user %f:system %f:wall seconds to process %s\n",
	  parseTimer.usecs(), parseTimer.ssecs(),
	  parseTimer.wsecs(), file.string_of());
#endif /* DEBUG_TIME */
  
  return(ret);
}

void image::addInternalSymbol(const string &str, const Address symValue) {
  // Internal symbols are prefixed by { DYNINST, TRACELIB }
  // don't ignore the underscore
  static string dyn = "_DYNINST"; 
  static string tlib = "_TRACELIB";

  // normalize all symbols -- remove the leading "_"
  if (str.prefixed_by(dyn)) {
    const char *s = str.string_of(); s++;
    if (!iSymsMap.defines(s))
      iSymsMap[s] = new internalSym(symValue, s);
    return;
  } else if (str.prefixed_by(tlib)) {
    const char *s = str.string_of(); s++;
    if (!iSymsMap.defines(s))
      iSymsMap[s] = new internalSym(symValue, s);
    return;
  } else {
    if (!iSymsMap.defines(str))
      iSymsMap[str] = new internalSym(symValue, str);
    return;
  }
}

Address image::findInternalAddress(const string name, const bool warn, bool &err)
{

  err = false;

  if (!iSymsMap.defines(name)) {
    if (warn) {
      cout << "unable to find internal symbol " << name << endl;
      P_abort();
    }
    err = true;
    return 0;
  } else
    return (iSymsMap[name]->getAddr());
}

module *image::findModule(const string &name)
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
pdFunction *image::findOneFunction(const string &name)
{
  string demangName;

  if (funcsByPretty.defines(name)) {
    vector<pdFunction*> *a = funcsByPretty[name];
    assert(a);
    if (!a->size())
      return NULL;
    else
      return ((*a)[0]);
  } else if (buildDemangledName(name, demangName)) {
    if (funcsByPretty.defines(demangName)) {
      vector<pdFunction*> *a = funcsByPretty[name];
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

bool image::findFunction(const string &name, vector<pdFunction*> &retList) {

  if (funcsByPretty.defines(name)) {
    retList = *funcsByPretty[name];
    return true;
  } else
    return false;
}

pdFunction *image::findFunction(const Address &addr)
{
  if (funcsByAddr.defines(addr))
    return (funcsByAddr[addr]);
  else 
    return NULL;
}
  
void image::changeLibFlag(resource *res, const bool setSuppress)
{
  image *ret;
  module *mod;

  dictionary_hash_iter<string, image*> ai(image::allImages);
  string pds;
  while (ai.next(pds, ret)) {
    mod = ret->findModule(res->getName());
    if (mod) {
      // suppress all procedures.
      mod->changeLibFlag(setSuppress);
    } else {
      // more thant one function may have this name --> templates, statics
      vector<pdFunction*> pdfA;
      if (ret->findFunction(res->getName(), pdfA)) {
	int i;
	for (i=0; i<pdfA.size(); ++i) {
	  if (setSuppress) 
	    pdfA[i]->tagAsLib();
	  else
	    pdfA[i]->untagAsLib();
	}
      }
    }
  }
}


/* 
 * return 0 if symbol <symname> exists in image, non-zero if it does not
 */
bool image::symbolExists(const string symname)
{
  pdFunction *dummy = findOneFunction(symname);
  return (dummy != NULL);
}

void image::postProcess(const string pifname)
{
  FILE *Fil;
  string fname;
  char temp[5000], errorstr[5000], key[5000];
  char tmp1[5000], abstraction[500];
  resource *parent;

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
    sprintf(errorstr, 
	    "Tried to open PIF file %s, but could not (continuing)\n", 
	    fname.string_of());
    logLine(errorstr);
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
      /* Create a new resource */
      fscanf(Fil, "%s {", abstraction);
      parent = rootResource;
      do {
	fscanf(Fil, "%s", tmp1);
        if (tmp1[0] != '}') {
          parent = newResource(parent, NULL, abstraction, tmp1, 0.0, "");
        } else {
	  parent = NULL;
	}
      } while (parent != NULL);
      break;
    default:
      sprintf(errorstr, 
	      "Ignoring bad line key (%s) in file %s\n", key, fname.string_of());
      logLine(errorstr);
      fgets(tmp1, 5000, Fil);
      break;
    }
  }
  return;
}

module::module() : funcMap(string::hash)
{
  compileInfo = NULL;
  fileName = NULL;
  fullName = NULL;
  language = langUnknown;
  addr = 0;
  exec = 0;
}

void image::defineModules() {
  string pds; module *mod;
  dictionary_hash_iter<string, module*> mi(modsByFileName);

  statusLine("defining modules");
  tp->resourceBatchMode(TRUE);
  while (mi.next(pds, mod))
    mod->define();
  statusLine("ready");
  tp->resourceBatchMode(FALSE);

}

#include <strstream.h>

void module::define() {
  pdFunction *pdf; string pds;
  resource *modResource = NULL;
#ifdef DEBUG_MODS
  char buffer[100];
  ostrstream osb(buffer, 100, ios::out);
  osb << "MODS_" << exec->name() << "__" << getpid() << ends;
  ofstream of(buffer, ios::app);
#endif

  dictionary_hash_iter<string, pdFunction*> fi(funcMap);
  while (fi.next(pds, pdf)) {
#ifdef DEBUG_MODS
    of << fileName << ":  " << pdf->prettyName() <<  "  " <<
      pdf->isLibTag() << "  " << pdf->addr() << endl;
#endif
    // ignore line numbers for now 
    if (!(pdf->isLibTag())) {
      // see if we have created module yet.
      if (!modResource) {
	modResource = newResource(moduleRoot, this, nullString, 
				  fileName, 0.0, "");
      }
      (void) newResource(modResource, pdf, nullString,
			 pdf->prettyName(), 0.0, "");
    }
  }
}

static inline bool findStartSymbol(Object &linkedFile, Address &adr) {
  Symbol lookUp;

  if (linkedFile.get_symbol("DYNINSTstartUserCode", lookUp) ||
      linkedFile.get_symbol("_DYNINSTstartUserCode", lookUp)) {
    adr = lookUp.addr();
    return true;
  } else
    return false;
}

static inline bool findEndSymbol(Object &linkedFile, Address &adr) {
  Symbol lookUp;

  if (linkedFile.get_symbol("DYNINSTendUserCode", lookUp) ||
      linkedFile.get_symbol("_DYNINSTendUserCode", lookUp)) {
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

    if ((index == (mods.size()-1)) ||
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

bool image::addOneFunction(vector<Symbol> &mods, module *lib, module *dyn,
			   const bool startB, const Address startAddr,
			   const bool endB, const Address endAddr,
			   const Symbol &lookUp) {
  bool defErr=true;
  if (notInUserRange(lookUp.addr(), startB, startAddr, endB, endAddr))
    defineFunction(lib, lookUp, TAG_LIB_FUNC, defErr);   // "library symbols" 
  else {
    // TODO mdc
    // find the module
    // this is a "user" symbol
    string modName = lookUp.module();
    Address modAddr = 0;

    string progName = name_ + "_module";
    module *progModule = newModule(progName, 0);

    binSearch(lookUp, mods, modName, modAddr, progName);
    defineFunction(lib, lookUp, defErr, modName, modAddr);
  }
  return defErr;
}

void image::addAllFunctions(vector<Symbol> &mods,
			    module *lib, module *dyn, 
			    const bool startB, const Address startAddr,
			    const bool endB, const Address endAddr) {
  bool defErr;

  // TODO - find main and exit since other symbols may be mapped to their
  // addresses which makes things confusing
  // The rule is - the first function to define an address, gets it
  findKnownFunctions(linkedFile, lib, dyn, startB, startAddr, endB, endAddr, defErr, mods);

  Address boundary;
  Symbol lookUp;
  string symString;
  SymbolIter symIter(linkedFile);

  if (!linkedFile.get_symbol(symString="DYNINSTfirst", lookUp) &&
      !linkedFile.get_symbol(symString="_DYNINSTfirst", lookUp)) {
    cout << "unable to find DYNINSTfirst " << endl;
    P_abort();
  } else
    boundary = lookUp.addr();
    
  // find the real functions -- those with the correct type in the symbol table
  while (symIter.next(symString, lookUp)) {
    if (lookUp.type() == Symbol::ST_FUNCTION) {
      assert(isValidAddress(lookUp.addr()));
      if (lookUp.addr() >= boundary) {
	addInternalSymbol(lookUp.name(), lookUp.addr());
	defineFunction(dyn, lookUp, TAG_LIB_FUNC, defErr);
      } else {
	addOneFunction(mods, lib, dyn, startB, startAddr, endB, endAddr, lookUp);
      }
    }
  }

  // now find the pseudo functions -- this gets ugly
  // kludge has been set if the symbol could be a function
  symIter.reset();
  while (symIter.next(symString, lookUp)) {
    if ((lookUp.type() == Symbol::ST_OBJECT) && lookUp.kludge()) {
      if (lookUp.addr() >= boundary) {
	addInternalSymbol(lookUp.name(), lookUp.addr());
	defineFunction(dyn, lookUp, TAG_LIB_FUNC, defErr);   // internal symbols
      } else {
	addOneFunction(mods, lib, dyn, startB, startAddr, endB, endAddr, lookUp);
      }
    }
  }
}

// TODO - this should find all of the known functions in case 
// one of these functions has several names in the symbol table
// We want to declare the function by its known name and tag it
// before it is declared by its unknown name - which would make
// it untagged
//

void image::findKnownFunctions(Object &linkedFile, module *lib, module *dyn,
			       const bool startB, const Address startAddr,
			       const bool endB, const Address endAddr,
			       bool &defErr, vector<Symbol> &mods) {
  Symbol sym;

  dictionary_hash_iter<string, unsigned> di(tagDict);
  unsigned u; string s;
  while (di.next(s, u)) {
    char name[200];
    name[0] = '_';
    P_strcpy(&name[1], s.string_of());
    assert(strlen(name) > 0);
    if (linkedFile.get_symbol(s, sym) || linkedFile.get_symbol(name, sym)) {
      if (u & TAG_LIB_FUNC)
	defineFunction(lib, sym, u, defErr);
      else
	defErr = addOneFunction(mods, lib, dyn, startB, startAddr, 
				endB, endAddr, sym);
    }
  }
}

int symCompare(const void *s1, const void *s2) {
  const Symbol *sym1 = (const Symbol*)s1, *sym2 = (const Symbol*)s2;
  // TODO mdc
  return (sym1->addr() - sym2->addr());
}

// Please not that this is now machine independent-almost.  Let us keep it that way
// 
image::image(const string &fileName, bool &err)
:   funcsByAddr(uiHash),
    file_(fileName),
    linkedFile(fileName, pd_log_perror),
    iSymsMap(string::hash),
    modsByFileName(string::hash),
    modsByFullName(string::hash),
    funcsByPretty(string::hash)
{
  #ifdef DEBUG_TIME
  loadTimer.stop();
  char sline[256];
  fprintf(timeOut, "It took %f:user %f:system %f:wall seconds to load %s\n",
	 loadTimer.usecs(), loadTimer.ssecs(),
	  loadTimer.wsecs(), fileName.string_of());
  timer restTimer;
  restTimer.start();
#endif /* DEBUG_TIME */

  codeOffset_ = linkedFile.code_off();
  dataOffset_ = linkedFile.data_off();
  codeLen_ = linkedFile.code_len();
  dataLen_ = linkedFile.data_len();

  if (!codeLen_ || !linkedFile.code_ptr()) {
    if (!codeLen_) logLine ("codeLen == 0\n");
    logLine("File is\n");
    logLine(fileName.string_of());
    if (!linkedFile.code_ptr()) logLine ("code_ptr == NULL\n");
    logLine("Could not open executable file\n");
    err = true;
    return;
  }
    
  const char *nm = fileName.string_of();
  char *pos = P_strrchr(nm, '/');

  err = false;
  if (pos)
    name_ = pos + 1;
  else
    name_ = fileName;

  // TODO  -  dynamic ?
  if (!heapIsOk(syms_to_find)) {
    // TODO debug
    logLine("Heap not ok\n");
    err = true; return;
  }

  // find the "user" code boundaries
  statusLine("finding user code boundaries");
  Address startUserAddr, endUserAddr;
  bool startBound = findStartSymbol(linkedFile, startUserAddr);
  bool endBound = findEndSymbol(linkedFile, endUserAddr);

  // use the *DUMMY_MODULE* until a module is defined
  module *dynModule = newModule(DYN_MODULE, 0);
  module *libModule = newModule(LIBRARY_MODULE, 0);
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

    if (lookUp.type() == Symbol::ST_MODULE) {
      const char *str = (lookUp.name()).string_of();
      assert(str);
      int ln = P_strlen(str);

      // directory definition -- ignored for now
      if (str[ln-1] != '/')
	mods += lookUp;
    }
  }

#ifdef DEBUG_MODS
  char buffer[100];
  ostrstream osb(buffer, 100, ios::out);
  osb << "UNSORTED_" << name() << "__" << getpid() << ends;
  ofstream of(buffer, ios::app);
  for (unsigned ve=0; ve<mods.size(); ve++)
    of << mods[ve].name() << "  " << mods[ve].addr() << endl;
  of.close();
#endif

  // sort the modules by address
  statusLine("sorting modules");
  mods.sort(symCompare);
  assert(mods.sorted(symCompare));

#ifdef DEBUG_MODS
  ostrstream osb1(buffer, 100, ios::out);
  osb1 << "SORTED_" << name() << "__" << getpid() << ends;
  ofstream of1(buffer, ios::app);
  for (unsigned ve1=0; ve1<mods.size(); ve1++)
    of1 << mods[ve1].name() << "  " << mods[ve1].addr() << endl;
  of1.close();
#endif

  // remove duplicate entries -- some .o files may have the same address as .C files
  // kludge is true for module symbols that I am guessing are modules
  vector<Symbol> uniq;
  unsigned loop=0;
  while (loop < (mods.size()-1)) {
    if (mods[loop].addr() == mods[loop+1].addr()) {
      if (!mods[loop].kludge())
	mods[loop+1] = mods[loop];
    } else
      uniq += mods[loop];
    loop++;
  }

#ifdef DEBUG_MODS
  ostrstream osb2(buffer, 100, ios::out);
  osb2 << "UNIQUE_" << name() << "__" << getpid() << ends;
  ofstream of2(buffer, ios::app);
  for (unsigned ve2=0; ve2<uniq.size(); ve2++)
    of2 << uniq[ve2].name() << "  " << uniq[ve2].addr() << endl;
  of2.close();
#endif
  
  // define all of the functions
  statusLine("winnowing functions");
  addAllFunctions(uniq, libModule, dynModule, startBound, startUserAddr,
		  endBound, endUserAddr);
  statusLine("checking call points");
  checkAllCallPoints();
  statusLine("ready");

#ifdef DEBUG_TIME
  restTimer.stop();
  fprintf(timeOut, "It took %f:user %f:system %f:wall seconds to process after load %s\n",
	  restTimer.usecs(), restTimer.ssecs(),
	  restTimer.wsecs(), fileName.string_of());
#endif /* DEBUG_TIME */
}

void module::checkAllCallPoints() {
  dictionary_hash_iter<string, pdFunction*> fi(funcMap);
  string s; pdFunction *pdf;
  while (fi.next(s, pdf)) 
    pdf->checkCallPoints();
}

void image::checkAllCallPoints() {
  dictionary_hash_iter<string, module*> di(modsByFullName);
  string s; module *mod;
  while (di.next(s, mod))
    mod->checkAllCallPoints();
}

// passing in tags allows a function to be tagged as TAG_LIB_FUNC even
// if its entry is not in the tag dictionary of known functions
void image::defineFunction(module *use, const Symbol &sym, const unsigned tags, bool &err) {
  const char *str;
  str = (sym.name()).string_of();

  // TODO - skip the underscore
  if (*str == '_') 
    str++;

  unsigned dictTags = findTags(str);
  newFunc(use, str, sym.addr(), tags | dictTags, err);
}

module *image::getOrCreateModule(const string &modName, const Address modAddr) {
  const char *str = modName.string_of();
  int len = modName.length();
  assert(len>0);

  // TODO ignore directory definitions for now
  if (str[len-1] == '/') {
    return NULL;
  } else {
    // TODO - navigate ".." and "."
    char *lastSlash = P_strrchr(str, '/');
    if (lastSlash)
      return (newModule(++lastSlash, modAddr));
    else
      return (newModule(modName, modAddr));
  }
}

// KLUDGE TODO - internal functions are tagged with TAG_LIB_FUNC
// but they won't have tags in the tag dict, so this happens...
void image::defineFunction(module *libModule, const Symbol &sym, bool &err,
			   const string &modName, const Address modAddr) {
  const char *str = (sym.name()).string_of();

  // TODO - skip the underscore
  if (*str == '_') 
    str++;
  unsigned tags = findTags(str);

  if (TAG_LIB_FUNC & tags)
    newFunc(libModule, str, sym.addr(), tags | TAG_LIB_FUNC, err);
  else {
    module *use = getOrCreateModule(modName, modAddr);
    assert(use);
    newFunc(use, str, sym.addr(), tags, err);
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
pdFunction::pdFunction(const string symbol, const string &pretty, module *f,
		       Address adr, const unsigned tg, const image *owner, bool &err)
: tag_(tg),
  symTabName_(symbol),
  prettyName_(pretty),
  line_(0),
  file_(f),
  addr_(adr),
  funcEntry_(NULL),
  funcReturn_(NULL)
{
  instruction instr;
  err = false;

  instr.raw = owner->get_instruction(adr);
  if (!IS_VALID_INSN(instr)) {
    err = true; return;
  }

  // TODO - why not automatic?
  // define the entry point
  funcEntry_ = new instPoint(this, instr, owner, adr, true);
  assert(funcEntry_);

  while (true) {
    instr.raw = owner->get_instruction(adr);

    // this does not account for jump tables
    // if (!IS_VALID_INSN(instr)) {
    // err = true; return;
    // } else 
    if (isInsnType(instr, RETmask, RETmatch) ||
	       isInsnType(instr, RETLmask, RETLmatch)) {
      // define the return point
      funcReturn_ = new instPoint(this, instr, owner, adr, false);
      assert(funcReturn_);
      // TODO -- move this to machine dependent area
      if ((instr.resti.simm13 != 8) && (instr.resti.simm13 != 12)) {
	logLine("*** FATAL Error:");
	sprintf(errorLine, " unsupported return at %x\n", funcReturn_->addr);
	logLine(errorLine);
	P_abort();
	err = false;
	return;
      }
      return;
    } else if (isCallInsn(instr)) {
      // define a call point
      // this may update address - sparc - aggregate return value
      // want to skip instructions
      adr = newCallPoint(adr, instr, owner, err);
    }
    // now do the next instruction
    adr += 4;
  }
}

// find all DYNINST symbols that are data symbols
bool image::heapIsOk(const vector<sym_data> &find_us) {
  Address curr, instHeapEnd;
  Symbol sym;
  string str;

  for (unsigned i=0; i<find_us.size(); i++) {
    str = find_us[i].name;
    if (!linkedFile.get_symbol(str, sym)) {
      string str1 = string("_") + str.string_of();
      if (!linkedFile.get_symbol(str1, sym) && find_us[i].must_find) {
	char msg[100];
        sprintf(msg, "Cannot find %s, exiting", str.string_of());
	statusLine(msg);
        P_abort();
      }
    }
    addInternalSymbol(str, sym.addr());
  }

  string ghb = GLOBAL_HEAP_BASE;
  if (!linkedFile.get_symbol(ghb, sym)) {
    ghb = U_GLOBAL_HEAP_BASE;
    if (!linkedFile.get_symbol(ghb, sym)) 
      P_abort();
  }
  instHeapEnd = sym.addr();
  addInternalSymbol(ghb, instHeapEnd);
  ghb = INFERIOR_HEAP_BASE;

  if (!linkedFile.get_symbol(ghb, sym)) {
    ghb = UINFERIOR_HEAP_BASE;
    if (!linkedFile.get_symbol(ghb, sym))
      P_abort();
  }
  curr = sym.addr();
  addInternalSymbol(ghb, curr);
  if (curr > instHeapEnd) instHeapEnd = curr;

  // check that we can get to our heap.
  if (instHeapEnd > getMaxBranch()) {
    logLine("*** FATAL ERROR: Program text + data too big for dyninst\n");
    sprintf(errorLine, "    heap ends at %x\n", instHeapEnd);
    logLine(errorLine);
    return false;
  } else if (instHeapEnd + SYN_INST_BUF_SIZE > getMaxBranch()) {
    logLine("WARNING: Program text + data could be too big for dyninst\n");
    return false;
  }
  return true;
}
