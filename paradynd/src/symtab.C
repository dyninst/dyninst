/*
 *  Copyright 1993 Jeff Hollingsworth.  All rights reserved.
 *
 */

#ifndef lint
static char Copyright[] = "@(#) Copyright (c) 1993 Jeff Hollingsowrth\
    All rights reserved.";

static char rcsid[] = "@(#) /p/paradyn/CVSROOT/core/paradynd/src/symtab.C,v 1.26 1995/05/30 05:05:05 krisna Exp";
#endif

/*
 * symtab.C - generic symbol routines.  Implements an ADT for a symbol
 *   table.  The make calls to machine and a.out specific routines to handle
 *   the implementation dependent parts.
 *
 * $Log: symtab.C,v $
 * Revision 1.38  1996/04/06 21:25:33  hollings
 * Fixed inst free to work on AIX (really any platform with split I/D heaps).
 * Removed the Line class.
 * Removed a debugging printf for multiple function returns.
 *
 * Revision 1.37  1996/03/25  22:58:15  hollings
 * Support functions that have multiple exit points.
 *
 * Revision 1.36  1996/03/01  22:36:00  mjrg
 * Added a type to resources.
 * Changes to the MDL to handle the resource hierarchy better.
 *
 * Revision 1.35  1995/12/20 16:10:47  tamches
 * removed ref to sorted() (vector class)
 *
 * Revision 1.34  1995/12/15 22:27:04  mjrg
 * Merged paradynd and paradyndPVM
 * Get module name for functions from symbol table in solaris
 * Fixed code generation for multiple instrumentation statements
 * Changed syntax of MDL resource lists
 *
 * Revision 1.33  1995/11/28 15:56:59  naim
 * Minor fix. Changing char[number] by string - naim
 *
 * Revision 1.32  1995/11/22  00:02:26  mjrg
 * Updates for paradyndPVM on solaris
 * Fixed problem with wrong daemon getting connection to paradyn
 * Removed -f and -t arguments to paradyn
 * Added cleanUpAndExit to clean up and exit from pvm before we exit paradynd
 * Fixed bug in my previous commit
 *
 * Revision 1.31  1995/11/03 21:17:38  naim
 * Fixing minor warning - naim
 *
 * Revision 1.30  1995/11/03  00:06:13  newhall
 * changes to support changing the sampling rate: dynRPC::setSampleRate changes
 *     the value of DYNINSTsampleMultiple, implemented image::findInternalSymbol
 * fix so that SIGKILL is not being forwarded to CM5 applications.
 *
 * Revision 1.29  1995/10/26  21:07:28  tamches
 * removed some warnings
 *
 * Revision 1.28  1995/09/26 20:34:44  naim
 * Minor fix: change all msg char[100] by string msg everywhere, since this can
 * cause serious troubles. Adding some error messages too.
 *
 * Revision 1.27  1995/08/24  15:04:36  hollings
 * AIX/SP-2 port (including option for split instruction/data heaps)
 * Tracing of rexec (correctly spawns a paradynd if needed)
 * Added rtinst function to read getrusage stats (can now be used in metrics)
 * Critical Path
 * Improved Error reporting in MDL sematic checks
 * Fixed MDL Function call statement
 * Fixed bugs in TK usage (strings passed where UID expected)
 *
 * Revision 1.26  1995/05/30  05:05:05  krisna
 * upgrade from solaris-2.3 to solaris-2.4.
 * architecture-os based include protection of header files.
 * removed architecture-os dependencies in generic sources.
 * changed ST_* symbol names to PDST_* (to avoid conflict on HPUX)
 *
 * Revision 1.25  1995/05/18  10:42:40  markc
 * Added code to build procedure lists for the mdl
 *
 * Revision 1.24  1995/02/21  22:03:32  markc
 * Added slightly better error recovery, with messages!  Paradynd reports back
 * when it attempts to run an unusable executable.  It no longer aborts.
 *
 * Revision 1.23  1995/02/16  08:54:23  markc
 * Corrected error in comments -- I put a "star slash" in the comment.
 *
 * Revision 1.22  1995/02/16  08:34:58  markc
 * Changed igen interfaces to use strings/vectors rather than char igen-arrays
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
#include "arch.h"
#include "util/h/Object.h"
#include <fstream.h>
#include "util.h"
#include "dyninstP.h"
#include "util/h/String.h"
#include "inst.h"
#include "main.h"
#include "util/h/Timer.h"
#include "init.h"
#include "showerror.h"

vector<watch_data> image::watch_vec;
vector<image*> image::allImages;

extern "C" char *cplus_demangle(char *, int);

/* imported from platform specific library list.  This is lists all
   library functions we are interested in instrumenting. */

module *image::newModule(const string &name, const Address addr)
{
    module *ret;
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
    delete out;

    ret = new module(langUnknown, addr, fullNm, fileNm, this);
    modsByFileName[ret->fileName()] = ret;
    modsByFullName[ret->fullName()] = ret;
    mods += ret;
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
		    const unsigned tag, pdFunction *&retFunc)
{
  pdFunction *func;
  retFunc = NULL;
  // KLUDGE
  if ((func = findFunction(addr)))
    return false;

  if (!mod) {
    logLine("Error function without module\n");
    showErrorCallback(34, "Error function without module");
    return false;
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
  retFunc = func;
  if (err) {
    delete func;
    retFunc = NULL;
    return false;
  }
  
  funcsByAddr[addr] = func;
  mod->funcs += func;

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
  // TODO -- better method to detect same image/offset --> offset only for CM5
  unsigned i_size = allImages.size();
  for (unsigned u=0; u<i_size; u++)
    if (file == allImages[u]->file())
      return allImages[u];

  /*
   * load the symbol table. (This is the a.out format specific routine).
   */
  statusLine("Process executable file");
  bool err;

#ifdef DEBUG_TIME
  loadTimer.clear(); loadTimer.start();
  if (!timeOut) timeOut = fopen("/tmp/paradynd_times", "w");
  if (!timeOut) P_abort();
#endif /* DEBUG_TIME */

  // TODO -- kill process here
  image *ret = new image(file, err);
  if (err || !ret) {
    if (ret)
      delete ret;
    return NULL;
  }

  // Add to master image list.
  image::allImages += ret;

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
 * returns 0 on failure 
 */
internalSym *image::findInternalSymbol(const string name, const bool warn){
   Symbol lookUp;
   internalSym *ret_sym;

   if(linkedFile.get_symbol(name,lookUp)){
      ret_sym = new internalSym(lookUp.addr(),name); 
      return ret_sym;
   }
   else {
       string new_sym;
       new_sym = string("_") + name;
       if(linkedFile.get_symbol(new_sym,lookUp)){
          ret_sym = new internalSym(lookUp.addr(),name); 
          return ret_sym;
       }
   } 
   if(warn){
      string msg;
      msg = string("Unable to find symbol: ") + name;
      statusLine(msg.string_of());
      showErrorCallback(28, msg);
   }
   return 0;
}

Address image::findInternalAddress(const string name, const bool warn, bool &err)
{

  err = false;

  if (!iSymsMap.defines(name)) {
    if (warn) {
      string msg;
      msg = string("Unable to find symbol: ") + name;
      statusLine(msg.string_of());
      showErrorCallback(28, msg);
    }
    err = true;
    return 0;
  } 
  else
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

  unsigned i_size = image::allImages.size();
  for (unsigned u=0; u<i_size; u++) {
    ret = image::allImages[u];
    mod = ret->findModule(res->part_name());
    if (mod) {
      // suppress all procedures.
      mod->changeLibFlag(setSuppress);
    } else {
      // more than one function may have this name --> templates, statics
      vector<pdFunction*> pdfA;
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
  string fname, errorstr;
  char key[5000];
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
      /* Create a new resource */
      fscanf(Fil, "%s {", abstraction);
      parent = rootResource;
      do {
	fscanf(Fil, "%s", tmp1);
        if (tmp1[0] != '}') {
          parent = resource::newResource(parent, NULL, abstraction, tmp1, 0.0, "", MDL_T_STRING);
        } else {
	  parent = NULL;
	}
      } while (parent != NULL);
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
  string pds; module *mod;
  dictionary_hash_iter<string, module*> mi(modsByFileName);

  statusLine("defining modules");
  tp->resourceBatchMode(true);
  while (mi.next(pds, mod))
    mod->define();
  statusLine("ready");
  tp->resourceBatchMode(false);

#ifdef DEBUG_MDL
#include <strstream.h>
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

void module::define() {
  resource *modResource = NULL;
#ifdef DEBUG_MODS
#include <strstream.h>
  char buffer[100];
  ostrstream osb(buffer, 100, ios::out);
  osb << "MODS_" << exec->name() << "__" << getpid() << ends;
  ofstream of(buffer, ios::app);
#endif

  unsigned f_size = funcs.size();
  for (unsigned f=0; f<f_size; f++) {
    pdFunction *pdf = funcs[f];
#ifdef DEBUG_MODS
    of << fileName << ":  " << pdf->prettyName() <<  "  " <<
      pdf->isLibTag() << "  " << pdf->addr() << endl;
#endif
    // ignore line numbers for now 
    if (!(pdf->isLibTag())) {
      // see if we have created module yet.
      if (!modResource) {
	modResource = resource::newResource(moduleRoot, this, nullString, 
					    fileName(), 0.0, "", MDL_T_MODULE);
      }
      resource::newResource(modResource, pdf, nullString, pdf->prettyName(), 0.0, "", MDL_T_PROCEDURE);
    }
  }
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
			   const Symbol &lookUp, pdFunction  *&retFunc) {
  // TODO mdc
  // find the module
  // this is a "user" symbol
  string modName = lookUp.module();
  Address modAddr = 0;
  
  string progName = name_ + "_module";

#ifdef sparc_sun_solaris2_4
  // In solaris there is no address for modules in the symbol table, 
  // so the binary search will not work. The module field in a symbol
  // already has the correct module name for a symbol, if it can be
  // obtained from the symbol table, otherwise the module is an empty
  // string.
  if (modName == "")
    modName = progName;
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
			    module *lib, module *dyn, 
			    const bool startB, const Address startAddr,
			    const bool endB, const Address endAddr) {

  Address boundary_start, boundary_end;
  Symbol lookUp;
  string symString;
  SymbolIter symIter(linkedFile);

  if (!linkedFile.get_symbol(symString="DYNINSTfirst", lookUp) &&
      !linkedFile.get_symbol(symString="_DYNINSTfirst", lookUp)) {
    statusLine("Internal symbol DYNINSTfirst not found");
    showErrorCallback(31, "Internal symbol DYNINSTfirst not found");
    return false;
  } else
    boundary_start = lookUp.addr();

  if (!linkedFile.get_symbol(symString="DYNINSTend", lookUp) &&
      !linkedFile.get_symbol(symString="_DYNINSTend", lookUp)) {
    statusLine("Internal symbol DYNINSTend not found");
    showErrorCallback(32, "Internal symbol DYNINSTend not found");
    return false;
  } else
    boundary_end = lookUp.addr();

  // TODO - find main and exit since other symbols may be mapped to their
  // addresses which makes things confusing
  // The rule is - the first function to define an address, gets it
  if (!findKnownFunctions(linkedFile, lib, dyn, startB, startAddr, 
			  endB, endAddr, boundary_start, boundary_end, mods)) {
    return false;
  }

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
      pdFunction *pdf;
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
      pdFunction *pdf;
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

// TODO - this should find all of the known functions in case 
// one of these functions has several names in the symbol table
// We want to declare the function by its known name and tag it
// before it is declared by its unknown name - which would make
// it untagged
//

bool image::findKnownFunctions(Object &linkedFile, module *lib, module *dyn,
			       const bool startB, const Address startAddr,
			       const bool endB, const Address endAddr,
			       const Address boundary_start, 
			       const Address boundary_end,
			       vector<Symbol> &mods) {
  Symbol sym;
  string s;
  // TODO -- this will be redundant until the mdl replacement is final
  unsigned wv_size = watch_vec.size();
  for (unsigned wv=0; wv<wv_size; wv++) {
    if (watch_vec[wv].is_func) {
      unsigned non_size = watch_vec[wv].non_prefix.size();
      for (unsigned non=0; non<non_size; non++) {
	pdFunction *pdf;
	string non_string(watch_vec[wv].non_prefix[non]);
	string under_string(string("_") + non_string);

	if (linkedFile.get_symbol(non_string, sym) ||
	    linkedFile.get_symbol(under_string, sym)) {
	  if (funcsByAddr.defines(sym.addr())) {
	    pdf = funcsByAddr[sym.addr()];
	    (*watch_vec[wv].funcs) += pdf;
	  } else if (watch_vec[wv].is_lib ||
		     inLibrary(sym.addr(), boundary_start, boundary_end, startAddr,
			       startB, endAddr, endB)) {
	    addInternalSymbol(sym.name(), sym.addr());
	    if (defineFunction(dyn, sym, TAG_LIB_FUNC, pdf)) {
	      assert(pdf); mdlLib += pdf; (*watch_vec[wv].funcs) += pdf;
	    }
	  } else {
	    if (addOneFunction(mods, lib, dyn, sym, pdf)) {
	      assert(pdf); mdlNormal += pdf; (*watch_vec[wv].funcs) += pdf;
	    }
	  }
	}
	// TODO -- get duplicates off of this list -- the watch_vec list
	// Or let anyone who puts duplicate functions on a list suffer?
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

// Please note that this is now machine independent-almost.  Let us keep it that way
// 
image::image(const string &fileName, bool &err)
:   funcsByAddr(uiHash),
    modsByFileName(string::hash),
    modsByFullName(string::hash),
    file_(fileName),
    linkedFile(fileName, pd_log_perror),
    iSymsMap(string::hash),
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
    string msg;
    msg = string("Unable to open executable file: ") + fileName;
    statusLine(msg.string_of());
    err = true;
    showErrorCallback(27, msg); 
    return;
  }

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

  const char *nm = fileName.string_of();
  const char *pos = P_strrchr(nm, '/');

  err = false;
  if (pos)
    name_ = pos + 1;
  else
    name_ = fileName;

  // TODO  -  dynamic ?
  if (!heapIsOk(syms_to_find)) {
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

    if (lookUp.type() == Symbol::PDST_MODULE) {
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
//  assert(mods.sorted(symCompare));

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

  // must use loop+1 not mods.size()-1 since it is an unsigned compare
  //  which could go negative - jkh 5/29/95
  for (loop=0; loop < mods.size(); loop++) {
    if ((loop+1 < mods.size()) && mods[loop].addr() == mods[loop+1].addr()) {
      if (!mods[loop].kludge())
	mods[loop+1] = mods[loop];
    } else
      uniq += mods[loop];
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
  if (!addAllFunctions(uniq, libModule, dynModule, startBound, startUserAddr,
		       endBound, endUserAddr)) {
    err = true;
    return;
  }
  statusLine("checking call points");
  checkAllCallPoints();
  statusLine("ready");

#ifdef DEBUG_TIME
  restTimer.stop();
  fprintf(timeOut, "It took %f:user %f:system %f:wall seconds to process after load %s\n",
	  restTimer.usecs(), restTimer.ssecs(),
	  restTimer.wsecs(), fileName.string_of());
#endif /* DEBUG_TIME */

  // TODO -- remove duplicates -- see earlier note
  dictionary_hash<unsigned, unsigned> addr_dict(uiHash);
  vector<pdFunction*> temp_vec;
  unsigned f_size = mdlLib.size(), index;
  for (index=0; index<f_size; index++) {
    if (!addr_dict.defines((unsigned)mdlLib[index]->addr())) {
      addr_dict[(unsigned)mdlLib[index]->addr()] = 1;
      temp_vec += mdlLib[index];
    }
  }
  mdlLib = temp_vec;
  temp_vec.resize(0); addr_dict.clear();
  f_size = mdlNormal.size();
  for (index=0; index<f_size; index++) {
    if (!addr_dict.defines((unsigned)mdlNormal[index]->addr())) {
      addr_dict[(unsigned)mdlNormal[index]->addr()] = 1;
      temp_vec += mdlNormal[index];
    }
  }
  mdlNormal = temp_vec;
}

void module::checkAllCallPoints() {
  unsigned fsize = funcs.size();
  for (unsigned f=0; f<fsize; f++)
    funcs[f]->checkCallPoints();
}

void image::checkAllCallPoints() {
  dictionary_hash_iter<string, module*> di(modsByFullName);
  string s; module *mod;
  while (di.next(s, mod))
    mod->checkAllCallPoints();
}

// passing in tags allows a function to be tagged as TAG_LIB_FUNC even
// if its entry is not in the tag dictionary of known functions
bool image::defineFunction(module *use, const Symbol &sym, const unsigned tags,
			   pdFunction *&retFunc) {
  const char *str = (sym.name()).string_of();

  // TODO - skip the underscore
  if (*str == '_') 
    str++;

  unsigned dictTags = findTags(str);
  return (newFunc(use, str, sym.addr(), tags | dictTags, retFunc));
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
    const char *lastSlash = P_strrchr(str, '/');
    if (lastSlash)
      return (newModule(++lastSlash, modAddr));
    else
      return (newModule(modName, modAddr));
  }
}

// KLUDGE TODO - internal functions are tagged with TAG_LIB_FUNC
// but they won't have tags in the tag dict, so this happens...
bool image::defineFunction(module *libModule, const Symbol &sym,
			   const string &modName, const Address modAddr,
			   pdFunction *&retFunc) {
  const char *str = (sym.name()).string_of();

  // TODO - skip the underscore
  if (*str == '_') 
    str++;
  unsigned tags = findTags(str);

  if (TAG_LIB_FUNC & tags)
    return (newFunc(libModule, str, sym.addr(), tags | TAG_LIB_FUNC, retFunc));
  else {
    module *use = getOrCreateModule(modName, modAddr);
    assert(use);
    return (newFunc(use, str, sym.addr(), tags, retFunc));
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
  funcEntry_(NULL)
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

    bool done;

    // check for return insn and as a side affect decide if we are at the
    //   end of the function.
    if (isReturnInsn(owner, adr, done)) {
      // define the return point
      funcReturns += new instPoint(this, instr, owner, adr, false);

      // see if this return is the last one 
      if (done) return;
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

// Store the mapping function:    name --> set of mdl resource lists
void image::update_watch_map(unsigned index, vector<string> *vs,
			     vector<string>& prefix,
			     vector<string>& non_prefix) {
  unsigned vs_size = vs->size();
  for (unsigned v=0; v<vs_size; v++) {
    unsigned len = (*vs)[v].length();

    // Check for wildcard character
    if (len && (((*vs)[v].string_of())[len-1] == '*')) {
      char *buffer = new char[len+1];
      P_strcpy(buffer, (*vs)[v].string_of());
      assert(buffer[len-1] == '*');
      buffer[len-1] = '\0';
      prefix += buffer;
      delete [] buffer;
    } else {
      non_prefix += (*vs)[v];
    }
  }
}

void image::watch_functions(string& name, vector<string> *vs, bool is_lib,
			    vector<pdFunction*> *update) {
  unsigned wv_size = watch_vec.size();
  bool found = false; unsigned found_index=0;
  for (unsigned wv=0; wv<wv_size; wv++)
    if (watch_vec[wv].name == name) {
      delete watch_vec[wv].funcs;
      delete watch_vec[wv].mods;
      found = true;
      found_index = wv;
    }

  if (!found) {
    watch_data wd;
    found_index = watch_vec.size();
    watch_vec += wd;
  }

  update_watch_map(found_index, vs, watch_vec[found_index].prefix, 
		   watch_vec[found_index].non_prefix);

  watch_vec[found_index].name = name;
  watch_vec[found_index].is_lib = is_lib;
  watch_vec[found_index].funcs = update;
  watch_vec[found_index].mods = NULL;
  watch_vec[found_index].is_func = true;
}
