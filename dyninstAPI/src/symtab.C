/*
 *  Copyright 1993 Jeff Hollingsworth.  All rights reserved.
 *
 */

#ifndef lint
static char Copyright[] = "@(#) Copyright (c) 1993 Jeff Hollingsowrth\
    All rights reserved.";

static char rcsid[] = "@(#) $Header: /home/jaw/CVSROOT_20081103/CVSROOT/core/dyninstAPI/src/symtab.C,v 1.18 1994/11/09 18:40:38 rbi Exp $";
#endif

/*
 * symtab.C - generic symbol routines.  Implements an ADT for a symbol
 *   table.  The make calls to machine and a.out specific routines to handle
 *   the implementation dependent parts.
 *
 * $Log: symtab.C,v $
 * Revision 1.18  1994/11/09 18:40:38  rbi
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

extern "C" {
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
}

// TODO - machine independent instruction functions
#include "symtab.h"
#include "arch-sparc.h"
#include "util/h/kludges.h"
#include "util/h/Object.h"
#include <strstream.h>
#include <fstream.h>
#include "util.h"
#include "dyninstP.h"
#include "util/h/String.h"
#include "inst.h"


dictionary_hash<string, image*> image::allImages(string::hash);

resource *moduleRoot=NULL;
extern "C" char *cplus_demangle(char *, int);

bool heapIsOk(Object &obj);

/* imported from platform specific library list.  This is lists all
   library functions we are interested in instrumenting. */

module *image::newModule(const string &name, const Address addr)
{
    module *ret;

    // modules can be defines several times in C++ due to templates and
    //   in-line member functions.
    if (ret = findModule(name))
      return(ret);

    ret = new module;

    ostrstream os;
    os << name << ends;
    char *out = os.str();
    char *sl = strrchr(out, '/');
    if (sl) {
      *sl = (char)0;
      ret->fullName = out;
      ret->fileName = sl+1;
    } else {
      ostrstream os1;
      os1 << "/DEFAULT" << out << ends;
      char *s = os1.str();
      ret->fullName = s;
      ret->fileName = out;
      delete s;
    }
    delete out;

    ret->language = langUnknown;
    ret->addr = addr;
    ret->exec = this;
    modsByFileName[ret->fileName] = ret;
    modsByFullName[ret->fullName] = ret;
    return(ret);
}

bool buildDemangledName(const string mangled, string &use)
{
    char *tempName;
    ostrstream os;
    os << mangled << ends;
    tempName = os.str();

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

// When a function is seen more than once
bool image::moveFunction(module *mod, const string &name, const Address adr,
			 const unsigned tag, pdFunction *func) {
  char *p, *out; ostrstream os;
  os << name << ends;
  out = os.str();
  p = strchr(out, ':');
  if (p) *p = '\0';

  string demangled;
  if (!buildDemangledName(out, demangled)) 
    demangled = out;
  delete out;

  // remove from old module
  func->file->funcMap.undef(func->getPretty());

  func->symTabName = name;
  func->prettyName = demangled;

  // put in this module
  mod->funcMap[func->getPretty()] = func;
  func->file = mod;
  func->tag = tag;

  if (!funcsByPretty.defines(func->getPretty())) {
    vector<pdFunction*> *a1 = new vector<pdFunction*>;
    funcsByPretty[func->getPretty()] = a1;      
  }

  // several functions may have the same demangled name, and each one
  // will appear in a different module
  vector<pdFunction*> *ap = funcsByPretty[func->getPretty()];
  assert(ap);
  (*ap) += func;
  return true;
}

// err is true if the function can't be.defines
bool image::newFunc(module *mod, const string name, const Address addr,
		    const unsigned tag, bool &err)
{
  pdFunction *func;
  // KLUDGE
  if (func = findFunctionByAddr(addr)) {
    //if ((type == Symbol::ST_FUNCTION) && (func->type != Symbol::ST_FUNCTION)) {
    //   func->type = Symbol::ST_FUNCTION;
    // return (moveFunction(mod, name, addr, tag, func));
    // } else
    // return false;
    return false;
  }
  char *p;

  if (!mod) {
    logLine("Error function without module\n");
    abort();
  }
  
  char *out; ostrstream os;
  os << name << ends;
  out = os.str();
  p = strchr(out, ':');
  if (p) *p = '\0';

  string demangled;
  if (!buildDemangledName(out, demangled)) 
    demangled = out;
  delete out;

  bool err;
  func = new pdFunction(name, demangled, mod, addr, tag, this, err);
  if (err) {
    return false;
  }
  
  funcsByAddr[addr] = func;
  
  mod->funcMap[func->getPretty()] = func;

  if (!funcsByPretty.defines(func->getPretty())) {
    vector<pdFunction*> *a1 = new vector<pdFunction*>;
    funcsByPretty[func->getPretty()] = a1;      
  }

  // several functions may have the same demangled name, and each one
  // will appear in a different module
  vector<pdFunction*> *ap = funcsByPretty[func->getPretty()];
  assert(ap);
  (*ap) += func;
  return true;
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
image *parseImage(const string file)
{
    image *ret;
    /*
     * Check to see if we have parsed this image at this offeset before.
     */
    string pds; image *ip;
    dictionary_hash_iter<string, image*> ii(image::allImages);
    // TODO -- better method to detect same image/offset --> offset only for CM5
    while (ii.next(pds, ip))
      if (file == ip->getFile())
	return ip;

    /*
     * load the symbol table. (This is the a.out format specific routine).
     */
    statusLine("loading symbol table");
    ostrstream os;
    os << file << ends;
    char *temp = os.str();
    bool err;
    ret = new image(temp, err);
    delete temp;

    /* check that we loaded o.k. */    
    if (err || !ret)
      return NULL;

    // Add to master image list.

    image::allImages[ret->getName()] = ret;

    if (!moduleRoot)
      moduleRoot = newResource(rootResource, NULL, (char*)NULL, "Procedure", 0.0, false);

    // define all modules.
    ret->defineModules();

    //
    // Optional post-processing step.  
    if (ret->symbolExists("CMRT_init"))
      ret->postProcess((char*)NULL);

    return(ret);
}

// TODO - clean this up - mdc
bool isInternalSymbol(const string &str) {
  return (str.prefixed_by("DYNINST") ||
	  str.prefixed_by("_DYNINST") ||
	  str.prefixed_by("TRACELIB") ||
	  str.prefixed_by("_TRACELIB"));
}

bool image::addInternalSymbol(const string &str, const Address symValue) {
  // Internal symbols are prefixed by { DYNINST, TRACELIB }
  // don't ignore the underscore

  if (str.prefixed_by("DYNINST")) {
    if (!iSymsMap.defines(str))
      iSymsMap[str] = new internalSym(symValue, str);
    return true;
  } else if  (str.prefixed_by("_DYNINST")) {
    const char *s = str.string_of(); s++;
    if (!iSymsMap.defines(str))
      iSymsMap[s] = new internalSym(symValue, s);
    return true;
  } else if (str.prefixed_by("TRACELIB")) {
    if (!iSymsMap.defines(str))
      iSymsMap[str] = new internalSym(symValue, str);
    return true;
  } else if (str.prefixed_by("_TRACELIB")) {
    const char *s = str.string_of(); s++;
    if (!iSymsMap.defines(str))
      iSymsMap[s] = new internalSym(symValue, s);
    return true;
  }

  return false;
}

internalSym *image::findInternalSymbol(const string name, bool warn)
{

  ostrstream os(errorLine, 1024, ios::out);

  if (!iSymsMap.defines(name)) {
    if (warn) {
      os << "unable to find internal symbol " << name << endl;
      logLine(errorLine);
    }
    return NULL;
  }
  else 
    return (iSymsMap[name]);
}

Address image::findInternalAddress(const string name, const bool warn, bool &err)
{

  err = false;

  if (!iSymsMap.defines(name)) {
    if (warn) {
      cout << "unable to find internal symbol " << name << endl;
      abort();
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

// No more siblings
// TODO - should I support demangled names/ mangled names ?
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

pdFunction *image::findFunctionByAddr(const Address addr)
{
  if (funcsByAddr.defines(addr))
    return (funcsByAddr[addr]);
  else 
    return NULL;
}
  
void changeLibFlag(resource *res, bool setSuppress)
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
	    pdfA[i]->tag |= TAG_LIB_FUNC;
	  else
	    pdfA[i]->tag &= ~TAG_LIB_FUNC;
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
  pdFunction *dummy;

  dummy = findOneFunction(symname);
  return (dummy != NULL);
}

void image::postProcess(const string pifname)
{
  FILE *Fil;
  string fname;
  char errorstr[5000], key[5000];
  char tmp1[5000], abstraction[500];
  resource *parent;

  ostrstream os;
  char *fileBuf;
  /* What file to open? */
  if (!(pifname == (char*)NULL)) {
    os << pifname << ends;
    fileBuf = os.str();
    fname = fileBuf;
  } else {
    ostrstream os;
    os << file << ".pif" << ends;
    fileBuf = os.str();
    fname = fileBuf;
  }

  /* Open the file */
  Fil = fopen(fileBuf, "r");

  if (Fil == NULL) {
    sprintf(errorstr, 
	    "Tried to open PIF file %s, but could not (continuing)\n", 
	    fileBuf);
    logLine(errorstr);
    delete fileBuf;
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
          parent = newResource(parent, NULL, abstraction, tmp1, 0.0, false);
        } else {
	  parent = NULL;
	}
      } while (parent != NULL);
      break;
    default:
      sprintf(errorstr, 
	      "Ignoring bad line key (%s) in file %s\n", key, fileBuf);
      logLine(errorstr);
      fgets(tmp1, 5000, Fil);
      break;
    }
  }
  delete fileBuf;
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
  while (mi.next(pds, mod))
    mod->define();
  statusLine("ready");

}

void module::define() {
  pdFunction *pdf; string pds;
  resource *modResource = NULL;
  char buffer[100];
  ostrstream osb(buffer, 100, ios::out);
  osb << "MODS_" << exec->name << "__" << getpid() << ends;
  ofstream of(buffer, ios::app);

  dictionary_hash_iter<string, pdFunction*> fi(funcMap);
  while (fi.next(pds, pdf)) {
    of << fileName << ":  " << pdf->prettyName <<  "  " <<
      (pdf->tag & TAG_LIB_FUNC) << endl;
    if (!(pdf->tag & TAG_LIB_FUNC)) { // IGNORE LINE NUMBERS FOR NOW
      // see if we have created module yet.
      if (!modResource) {
	modResource = newResource(moduleRoot, this, nullString, 
				  fileName, 0.0, false);
      }
      (void) newResource(modResource, pdf, nullString,
			 pdf->prettyName, 0.0, false);
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

bool image::addOneFunction(vector<Symbol> &mods, module *lib, module *dyn,
			   const bool startB, const Address startAddr,
			   const bool endB, const Address endAddr,
			   const Symbol &lookUp) {
  bool defErr=true;
  if (isInternalSymbol(lookUp.name()))
    defineFunction(dyn, lookUp, TAG_LIB_FUNC, defErr);   // internal symbols
  else if (notInUserRange(lookUp.addr(), startB, startAddr, endB, endAddr))
    defineFunction(lib, lookUp, TAG_LIB_FUNC, defErr);   // "library symbols" 
  else {
    // TODO mdc
    // find the module
    // this is a "user" symbol
    string modName = lookUp.module();
    Address modAddr = 0;
    int i=1; bool loop=true;

    while ((i < mods.size()) && loop) {
      if (lookUp.addr() < (mods[i]).addr()) {
	modName = (mods[i-1]).name();
	modAddr = (mods[i-1]).addr();
	loop = false;
      }
      i++;
    }
    defineFunction(lib, lookUp, defErr, modName, modAddr);
  }
  return defErr;
}

void image::addAllFunctions(vector<Symbol> &mods, vector<Symbol> &almostF,
			    module *lib, module *dyn, 
			    const bool startB, const Address startAddr,
			    const bool endB, const Address endAddr) {
  bool defErr;
  int i;

  // TODO - find main and exit since other symbols may be mapped to their
  // addresses which makes things confusing
  // The rule is - the first function to define an address, gets it
  findKnownFunctions(linkedFile, lib, dyn, startB, startAddr, endB, endAddr, defErr, mods);
  
  for (i=0; i<almostF.size(); ++i)
    addOneFunction(mods, lib, dyn, startB, startAddr, endB, endAddr, almostF[i]);
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
    strcpy(&name[1], s.string_of());
    assert(strlen(name) > 0);
    string s1 = name;
    if (linkedFile.get_symbol(s, sym) || linkedFile.get_symbol(s1, sym)) {
      if (u & TAG_LIB_FUNC)
	defineFunction(lib, sym, u, defErr);
      else
	defErr = addOneFunction(mods, lib, dyn, startB, startAddr, 
				endB, endAddr, sym);
    }
  }
}

int symCompare(const void *s1, const void *s2) {
  const Symbol *sym1 = (Symbol*)s1, *sym2 = (Symbol*)s2;
  // TODO mdc
  return (sym1->addr() - sym2->addr());
}

// Please not that this is now machine independent-almost.  Let us keep it that way
// 
image::image(char *fileName, bool &err)
: iSymsMap(string::hash), funcsByAddr(uiHash),
  funcsByPretty(string::hash),
  modsByFileName(string::hash), modsByFullName(string::hash),
  linkedFile(fileName, pd_log_perror)
{
  char sline[256];
  int count=0;

  codeOffset = linkedFile.code_off();
  dataOffset = linkedFile.data_off();
  codeLen = linkedFile.code_len();
  dataLen = linkedFile.data_len();

  if (!codeLen || !linkedFile.code_ptr()) {
    if (!codeLen) logLine ("codeLen == 0\n");
    logLine("File is\n");
    logLine(fileName);
    if (!linkedFile.code_ptr()) logLine ("code_ptr == NULL\n");
    logLine("Could not open executable file\n");
    err = true;
    return;
  }
    
  file = fileName;
  char *nm = strrchr(fileName, '/');
  err = false;
  if (nm)
    name = nm + 1;
  else
    name = file;

  // TODO  -  dynamic ?

  Symbol lookUp;
  string symString;
  SymbolIter symIter(linkedFile);

  if (!heapIsOk(linkedFile)) {
    // TODO debug
    logLine("Heap not ok\n");
    err = true; return;
  }

  Address startUserAddr, endUserAddr;
  bool startBound=false, endBound=false;

  // find the "user" code boundaries
  statusLine("finding user code boundaries");
  startBound = findStartSymbol(linkedFile, startUserAddr);
  endBound = findEndSymbol(linkedFile, endUserAddr);

  // use the *DUMMY_MODULE* until a module is.defines
  module *dynModule = newModule(DYN_MODULE, 0);
  module *libModule = newModule(LIBRARY_MODULE, 0);

  // TODO -- define inst points in define function ?

  // The functions cannot be verified until all of them have been seen
  // since calls to "real" functions that have yet to be seen must be handled
  vector<Symbol> almostFuncs;
  dictionary_hash<Address, Symbol*> modDict(hash_address);

  symIter.reset();
  char nameBuf[300];

  while (symIter.next(symString, lookUp)) {

    count++;
    if ((count % 25) == 0) {
      sprintf(sline,"processed %d syms", count);
      statusLine(sline);
    }

    // TODO - mdc - this lookup is being done again, doesn't have to be
    addInternalSymbol(symString, lookUp.addr());

    switch (lookUp.type()) {
    case Symbol::ST_UNKNOWN:
      break;

    case Symbol::ST_FUNCTION:
      // push the function on the todo list

      // NASTY KLUDGE mdc?
      strcpy(nameBuf, (lookUp.name()).string_of());
      int len = strlen(nameBuf);
      if ((len > 2) && (nameBuf[len-1] == 'o') && (nameBuf[len-2] == '.')) {
	if (!modDict.defines(lookUp.addr())) {
	  Symbol *s = new Symbol(lookUp);
	  assert(s);
	  modDict[lookUp.addr()] = s;
	}
	// END NASTY KLUDGE
      } else if (isValidAddress(lookUp.addr()))
	almostFuncs += lookUp;      // must be word aligned

      break;

    case Symbol::ST_OBJECT:
      break;
      
    case Symbol::ST_MODULE:
      const char *str = (lookUp.name()).string_of();
      assert(str);
      int ln = strlen(str);
      // directory definition
      if (str[ln-1] == '/')
	break;
      else if (!modDict.defines(lookUp.addr())) {
	Symbol *s = new Symbol(lookUp);
	assert(s);
	modDict[lookUp.addr()] = s;
      }
      break;

    default:
      abort();
    }
  }

  Address adr; Symbol *s;
  dictionary_hash_iter<Address, Symbol*> di(modDict);
  vector<Symbol> mods(modDict.size());
  int i =0;
  while (di.next(adr, s)) {
    mods[i] = *s;
    i++;
    delete s;
  }
    
  // sort the modules by address
  statusLine("sorting modules");
  mods.sort(symCompare);

  // define all of the functions
  statusLine("winnowing functions");
  addAllFunctions(mods, almostFuncs, libModule, dynModule, startBound, startUserAddr,
		  endBound, endUserAddr);
  statusLine("checking call points");
  checkAllCallPoints();
  statusLine("ready");
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
  char nameBuf[300], *str;
  strcpy(nameBuf, (sym.name()).string_of());

  // TODO - skip the underscore
  str = nameBuf;
  if (nameBuf[0] == '_') 
    str++;

  unsigned dictTags = findTags(nameBuf);
  newFunc(use, str, sym.addr(), tags | dictTags, err);
}

module *image::getOrCreateModule(const string &modName, const Address modAddr) {
  char nameBuffer[300];
  strcpy(nameBuffer, modName.string_of());

  int len = strlen(nameBuffer);
  assert(len>0);

  // TODO ignore directory definitions for now
  if (nameBuffer[len-1] == '/') {
    abort();
    return NULL;
  } else {
    // TODO - navigate ".." and "."
    char *lastSlash = strrchr(nameBuffer, '/');
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
  char nameBuf[300], *str;
  strcpy(nameBuf, (sym.name()).string_of());

  str = nameBuf;
  // TODO - skip the underscore
  if (nameBuf[0] == '_') 
    str++;
  unsigned tags = findTags(str);

  if (TAG_LIB_FUNC & tags)
    newFunc(libModule, str, sym.addr(), tags | TAG_LIB_FUNC, err);
  else {
    module *use = getOrCreateModule(modName, modAddr);
    assert(use);
    newFunc(use, str, sym.addr(), tags, err);
  }

  if (err) {
    // ostrstream os(errorLine, 1024, ios::out);
    // os << "Error defining function " << str << " address " << addr << endl;
    // logLine(errorLine);
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
: symTabName(symbol), prettyName(pretty), line(0), file(f), addr(adr),
  funcEntry(NULL), funcReturn(NULL), tag(tg)
{
  instruction instr;
  err = false;

  instr.raw = owner->get_instruction(adr);
  if (!IS_VALID_INSN(instr)) {
    err = true; return;
  }

  // define the entry point
  funcEntry = new instPoint(this, instr, owner, adr, true);

  while (true) {
    instr.raw = owner->get_instruction(adr);

    if (!IS_VALID_INSN(instr)) {
      err = true; return;
    } else if (isInsnType(instr, RETmask, RETmatch) ||
	       isInsnType(instr, RETLmask, RETLmatch)) {
      // define the return point
      funcReturn = new instPoint(this, instr, owner, adr, false);
      assert(funcReturn);
      // TODO -- move this to machine dependent area
      if ((instr.resti.simm13 != 8) && (instr.resti.simm13 != 12)) {
	logLine("*** FATAL Error:");
	sprintf(errorLine, " unsupported return at %x\n", funcReturn->addr);
	logLine(errorLine);
	abort();
	err = false;
	return;
      }
      assert(funcEntry && funcReturn);
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

bool heapIsOk(Object &obj) {
  Address curr, instHeapEnd;

  Symbol sym;
  string ghb = GLOBAL_HEAP_BASE;
  if (!obj.get_symbol(ghb, sym)) {
    ghb = U_GLOBAL_HEAP_BASE;
    if (!obj.get_symbol(ghb, sym)) 
      abort();
  }
  instHeapEnd = sym.addr();

  ghb = INFERIOR_HEAP_BASE;

  if (!obj.get_symbol(ghb, sym)) {
    ghb = UINFERIOR_HEAP_BASE;
    if (!obj.get_symbol(ghb, sym))
      abort();
  }

  curr = sym.addr();

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
