/*
 *  Copyright 1993 Jeff Hollingsworth.  All rights reserved.
 *
 */

#ifndef SYMTAB_HDR
#define SYMTAB_HDR

/*
 * symtab.h - interface to generic symbol table.
 *
 * $Log: symtab.h,v $
 * Revision 1.21  1996/04/26 19:58:29  lzheng
 * Moved some data structures used by HP only to the machine dependent file.
 * And an argument for constructor of instPoint to pass the error.
 *
 * Revision 1.20  1996/04/08 22:26:49  lzheng
 * Added some HP-specific structures and member functions, needed
 * for treating the call site, entry point, and exit points differently
 * on the HP.
 *
 * Revision 1.19  1996/03/25 22:58:18  hollings
 * Support functions that have multiple exit points.
 *
 * Revision 1.18  1995/11/29  18:45:26  krisna
 * added inlines for compiler. added templates
 *
 * Revision 1.17  1995/08/24 15:04:38  hollings
 * AIX/SP-2 port (including option for split instruction/data heaps)
 * Tracing of rexec (correctly spawns a paradynd if needed)
 * Added rtinst function to read getrusage stats (can now be used in metrics)
 * Critical Path
 * Improved Error reporting in MDL sematic checks
 * Fixed MDL Function call statement
 * Fixed bugs in TK usage (strings passed where UID expected)
 *
 * Revision 1.16  1995/05/30  05:05:08  krisna
 * upgrade from solaris-2.3 to solaris-2.4.
 * architecture-os based include protection of header files.
 * removed architecture-os dependencies in generic sources.
 * changed ST_* symbol names to PDST_* (to avoid conflict on HPUX)
 *
 * Revision 1.15  1995/05/18  10:42:42  markc
 * Added code to build procedure lists for the mdl
 *
 * Revision 1.14  1995/02/24  04:42:01  markc
 * Check if an address could be for an instruction before checking to see if it
 * is delayed, since we should not be checking instructions that are out of range.
 *
 * Revision 1.13  1995/02/21  22:03:36  markc
 * Added slightly better error recovery, with messages!  Paradynd reports back
 * when it attempts to run an unusable executable.  It no longer aborts.
 *
 * Revision 1.12  1995/02/16  08:54:25  markc
 * Corrected error in comments -- I put a "star slash" in the comment.
 *
 * Revision 1.11  1995/02/16  08:35:00  markc
 * Changed igen interfaces to use strings/vectors rather than char igen-arrays
 * Changed igen interfaces to use bool, not Boolean.
 * Cleaned up symbol table parsing - favor properly labeled symbol table objects
 * Updated binary search for modules
 * Moved machine dependnent ptrace code to architecture specific files.
 * Moved machine dependent code out of class process.
 * Removed almost all compiler warnings.
 * Use "posix" like library to remove compiler warnings
 *
 * Revision 1.10  1994/11/09  18:40:40  rbi
 * the "Don't Blame Me" commit
 *
 * Revision 1.9  1994/11/02  11:17:46  markc
 * Added class support for image, module, function.
 *
 * Revision 1.7  1994/09/30  19:47:17  rbi
 * Basic instrumentation for CMFortran
 *
 * Revision 1.6  1994/09/22  02:26:56  markc
 * Made structs classes
 *
 * Revision 1.5  1994/08/02  18:25:08  hollings
 * fixed modules to use list template for lists of functions.
 *
 * Revision 1.4  1994/07/22  19:21:11  hollings
 * removed mistaken divid by 1Meg for predicted cost.
 *
 * Revision 1.3  1994/07/20  23:23:43  hollings
 * added insn generated metric.
 *
 * Revision 1.2  1994/06/29  02:52:52  hollings
 * Added metricDefs-common.{C,h}
 * Added module level performance data
 * cleanedup types of inferrior addresses instrumentation defintions
 * added firewalls for large branch displacements due to text+data over 2meg.
 * assorted bug fixes.
 *
 * Revision 1.1  1994/01/27  20:31:46  hollings
 * Iinital version of paradynd speaking dynRPC igend protocol.
 *
 * Revision 1.4  1993/12/13  19:58:12  hollings
 * added sibling filed for functions that occur multiple times in the same
 * binary image (statics, and c++ template classes for example).
 *
 * Revision 1.3  1993/07/13  18:33:11  hollings
 * new include file syntax.
 *
 * Revision 1.2  1993/06/08  20:14:34  hollings
 * state prior to bc net ptrace replacement.
 *
 * Revision 1.1  1993/03/19  22:51:05  hollings
 * Initial revision
 *
 *
 */

extern "C" {
#include <sys/types.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
}

#include "util/h/Pair.h"
#include "util/h/Vector.h"
#include "util/h/Dictionary.h"
#include "util/h/Object.h"
#include "util/h/list.h"
#include "dyninst.h"
#include "arch.h"
#include "util.h"
#include "util/h/String.h"
#include "resource.h"
#include "util/h/Types.h"
#include "util/h/Symbol.h"

/*
 * List of supported languages.
 *
 */
typedef enum { langUnknown,
	       langAssembly,
	       langC,
	       langCPlusPlus,
	       langGnuCPlusPlus,
	       langFortran,
	       langCMFortran
	       } supportedLanguages;

 
/* contents of line number field if line is unknown */
#define UNKNOWN_LINE	0

#define TAG_LIB_FUNC	0x1
#define TAG_IO_OUT	0x2
#define TAG_IO_IN       0x4
#define TAG_MSG_SEND	0x8
#define TAG_MSG_RECV    0x10
#define TAG_SYNC_FUNC	0x20
#define TAG_CPU_STATE	0x40	/* does the func block waiting for ext. event */
#define TAG_MSG_FILT    0x80

#define DYN_MODULE "DYN_MODULE"
#define EXTRA_MODULE "EXTRA_MODULE"
#define USER_MODULE "USER_MODULE"
#define LIBRARY_MODULE	"LIBRARY_MODULE"

class pdFunction;
class instPoint;
class module;
class image;
class internalSym;
class lineTable;

// test if the passed instruction is a return instruction.
extern bool isReturnInsn(const image *owner, Address adr, bool &lastOne);


class pdFunction {
 public:
    pdFunction(const string symbol, const string &pretty, module *f, Address adr,
	       const unsigned tg, const image *owner, bool &err);
    ~pdFunction() { /* TODO */ }
    void checkCallPoints();
    bool defineInstPoint();
#if defined(hppa1_1_hp_hpux)
    Address newCallPoint(const Address adr, const instruction code, const 
                         image *owner, 
			 bool &err, instPointType pointType=noneType);
#else 
    Address newCallPoint(const Address adr, const instruction code, const 
                         image *owner, bool &err);
#endif

    string symTabName() const { return symTabName_;}
    string prettyName() const { return prettyName_;}
    const module *file() const { return file_;}
    Address addr() const { return addr_;}
    instPoint *funcEntry() const { return funcEntry_;}
    vector<instPoint*> funcReturns;	/* return point(s). */
    vector<instPoint*> calls;		/* pointer to the calls */
    inline void tagAsLib() { tag_ |= TAG_LIB_FUNC;}
    inline void untagAsLib() { tag_ &= ~TAG_LIB_FUNC;}
    inline bool isTagSimilar(const unsigned comp) const { return(tag_ & comp);}
    bool isLibTag() const { return (tag_ & TAG_LIB_FUNC);}
    unsigned tag() const { return tag_; }
#if defined(hppa1_1_hp_hpux)
    instruction entryPoint;
    instruction exitPoint;
    struct loadr *lr; 
    int szOfLr;
#endif

  private:
    unsigned tag_;
    string symTabName_;		/* name as it appears in the symbol table */
    string prettyName_;		/* user's view of name (i.e. de-mangled) */
    int line_;			/* first line of function */
    module *file_;		/* pointer to file that defines func. */
    Address addr_;		/* address of the start of the func */
    instPoint *funcEntry_;	/* place to instrument entry (often not addr) */
};

class instPoint {
public:
#if defined(hppa1_1_hp_hpux)
  instPoint(pdFunction *f, const instruction &instr, const image *owner,
	    const Address adr, const bool delayOK, bool &err,
	    instPointType pointType = noneType);
#else
  instPoint(pdFunction *f, const instruction &instr, const image *owner,
	    const Address adr, const bool delayOK);
#endif

  ~instPoint() {  /* TODO */ }
#ifdef notdef
  Address addr() const { return addr_; } 
  instruction originalInstruction() const { return originalInstruction_;} 
  instruction delaySlotInsn() const { return delaySlotInsn_; }
  instruction aggregateInsn() const { return aggregateInsn_; }
  bool inDelaySlot() const { return inDelaySlot_;}
  bool isDelayed() const { return isDelayed_;}
  bool callIndirect() const { return callIndirect_;}
  bool callAggregate() const { return callAggregate_;}
  pdFunction *callee() const { return callee_; }
  pdFunction *func() const { return func_;}
private:
#endif

  // can't set this in the constructor because call points can't be classified until
  // all functions have been seen -- this might be cleaned up
  void set_callee(pdFunction *to) { callee = to; }


  Address addr;                   /* address of inst point */
  instruction originalInstruction;    /* original instruction */
#if defined(hppa1_1_hp_hpux)          /* next instruction for HP-UX only */
  instruction nextInstruction;
  instPointType ipType;
#endif
  instruction delaySlotInsn;  /* original instruction */
  instruction aggregateInsn;  /* aggregate insn */
  bool inDelaySlot;            /* Is the instruction in a delay slot */
  bool isDelayed;		/* is the instruction a delayed instruction */
  bool callIndirect;		/* is it a call whose target is rt computed ? */
  bool callAggregate;		/* calling a func that returns an aggregate
				   we need to reolcate three insns in this case
				   */
  pdFunction *callee;		/* what function is called */
  pdFunction *func;		/* what function we are inst */
};


/* Stores source code to address in text association for modules */
class lineDict {
public:
  lineDict() : lineMap(uiHash) { }
  ~lineDict() { /* TODO */ }
  void setLineAddr (unsigned line, Address addr) { lineMap[line] = addr; }
  inline bool getLineAddr (const unsigned line, Address &adr);

private:
  dictionary_hash<unsigned, Address> lineMap;
};

class module {
public:
  inline module(supportedLanguages lang, Address adr, string &fullNm,
		string &fileNm, image *e);
  ~module() { /* TODO */ }

  void setLineAddr(unsigned line, Address addr) { lines_.setLineAddr(line, addr); }
  bool getLineAddr(unsigned line, Address &addr) { 
                                         return (lines_.getLineAddr(line, addr)); }

  void define();                // defines module to paradyn

  inline void changeLibFlag(const bool setSuppress);
  inline pdFunction *findFunction (const string &name);
  void mapLines() { }           // line number info is not used now
  void checkAllCallPoints();

  string fileName() const { return fileName_; }
  string fullName() const { return fullName_; }
  supportedLanguages language() const { return language_;}
  Address addr() const { return addr_; }
  image *exec() const { return exec_; }

  // Note -- why by address?, this structure is rarely used
  // the MDL should be the most frequent user and it needs this data structure
  // to be the same type as the function dictionary in class image
  vector<pdFunction*> funcs;

private:

  string fileName_;                   // short file 
  string fullName_;                   // full path to file 
  supportedLanguages language_;
  Address addr_;                      // starting address of module
  image *exec_;                      // what executable it came from 
  lineDict lines_;
};


/*
 * symbols we need to find from our RTinst library.  This is how we know
 *   were our inst primatives got loaded as well as the data space variables
 *   we use to put counters/timers and inst trampolines.  An array of these
 *   is placed in the image structure.
 *
 */
class internalSym {
public:
  internalSym(const Address adr, const string &nm) : name(nm), addr(adr) { }
  Address getAddr() const { return addr;}

private:
  string name;            /* name as it appears in the symbol table. */
  Address addr;      /* absolute address of the symbol */
};

typedef struct watch_data {
  string name;
  bool is_lib;
  vector<pdFunction*> *funcs;
  vector<module*> *mods;
  bool is_func;
  vector<string> prefix;
  vector<string> non_prefix;
} watch_data;


class image {
public:
  static image *parseImage(const string file);
  static void changeLibFlag(resource*, const bool);

  image(const string &file, bool &err);
  ~image() { /* TODO */ }

  internalSym *findInternalSymbol(const string name, const bool warn);
  Address findInternalAddress(const string name, const bool warn, bool &err);

  // find the named module 
  module *findModule(const string &name);

  // find the function by name, address, or the first by name
  bool findFunction(const string &name, vector<pdFunction*> &flist);
  pdFunction *findFunction(const Address &addr);
  pdFunction *findOneFunction(const string &name);

  // report modules to paradyn
  void defineModules();

  bool symbolExists(const string); /* Does the symbol exist in the image? */
  void postProcess(const string);          /* Load .pif file */

  // data member access
  inline Word get_instruction(Address adr) const;

  string file() const {return file_;}
  string name() const { return name_;}
  Address codeOffset() const { return codeOffset_;}
  Address dataOffset() const { return dataOffset_;}

  // functions by address for all modules
  dictionary_hash <Address, pdFunction*> funcsByAddr;

  // TODO -- get rid of one of these
  dictionary_hash <string, module *> modsByFileName;
  dictionary_hash <string, module*> modsByFullName;

  inline bool isValidAddress(const Address &where) const;

  // Return symbol table information
  inline bool symbol_info(string& symbol_name, Symbol& ret);

  // Called from the mdl -- lists of functions to look for
  static void watch_functions(string& name, vector<string> *vs, bool is_lib,
			      vector<pdFunction*> *updateDict);

  // called from function/module destructor, removes the pointer from the watch list
  // TODO
  static void destroy(pdFunction *pdf) { }
  static void destroy(module *mod) { }

  vector<pdFunction*> mdlLib;
  vector<pdFunction*> mdlNormal;
  vector<module*> mods;

private:
  string file_;		/* image file name */
  string name_;		/* filename part of file, no slashes */

  Address codeOffset_;
  unsigned codeLen_;
  Address dataOffset_;
  unsigned dataLen_;

  // data from the symbol table 
  Object linkedFile;

  dictionary_hash <string, internalSym*> iSymsMap;   // internal RTinst symbols

  static vector<image*> allImages;

  dictionary_hash <string, vector<pdFunction*>*> funcsByPretty;
  // note, a prettyName is not unique, it may map to a function appearing
  // in several modules

  bool newFunc(module *, const string name, const Address addr,
	       const unsigned tags, pdFunction *&retFunc);

  void checkAllCallPoints();

  bool addInternalSymbol(const string &str, const Address symValue);

  // creates the module if it does not exist
  module *getOrCreateModule (const string &modName, const Address modAddr);
  module *newModule(const string &name, Address addr);

  bool findKnownFunctions(Object &linkedFile, module *lib, module *dyn,
			  const bool startB, const Address startAddr,
			  const bool endB, const Address endAddr,
			  const Address b_start, const Address b_end,
			  vector<Symbol> &mods);

  bool addOneFunction(vector<Symbol> &mods, module *lib, module *dyn,
		      const Symbol &lookUp, pdFunction *&retFunc);

  bool addAllFunctions(vector<Symbol> &mods,
		       module *lib, module *dyn,
		       const bool startB, const Address startAddr,
		       const bool endB, const Address endAddr);

  // if useLib = true or the functions' tags signify a library function
  // the function is put in the library module
  bool defineFunction(module *use, const Symbol &sym, const unsigned tags,
		      pdFunction *&retFunc);
  bool defineFunction(module *lib, const Symbol &sym,
		      const string &modName, const Address modAdr,
		      pdFunction *&retFunc);

  inline bool isCode(const Address &where) const;
  inline bool isData(const Address &where) const;
  bool heapIsOk(const vector<sym_data>&);
  
  static vector<watch_data> watch_vec;
  static void update_watch_map(unsigned index, vector<string> *vs,
			       vector<string>& pref,
			       vector<string>& non_pref);
};


/*
 * a definition of a library function that we may wish to identify.  This is
 *   how we describe it to the symbol table parser, not how it appears in
 *   the symbol table.  Library functions are placed in a pseudo module 
 *   named LIBRARY_MODULE. 
 *
 */


class libraryFunc {
public:
  libraryFunc(const string n, unsigned t) : name(n), tags(t) { }
  unsigned getTags() const { return tags;}

private:
  string name;
  unsigned tags;
};

// TODO -- remove this
extern resource *moduleRoot;

inline bool lineDict::getLineAddr (const unsigned line, Address &adr) {
  if (!lineMap.defines(line)) {
    return false;
  } else {
    adr = lineMap[line];
    return true;
  }
}

inline module::module(supportedLanguages lang, Address adr, string &fullNm,
	       string &fileNm, image *e) 
: fileName_(fileNm), fullName_(fullNm), language_(lang),
  addr_(adr), exec_(e) { }

inline void module::changeLibFlag(const bool setSuppress) {
  unsigned fsize = funcs.size();
  for (unsigned f=0; f<fsize; f++) {
    if (setSuppress)
      funcs[f]->tagAsLib();
    else
      funcs[f]->untagAsLib();    
  }
}

inline pdFunction *module::findFunction (const string &name) {
  unsigned fsize = funcs.size();
  for (unsigned f=0; f<fsize; f++) {
    if (funcs[f]->prettyName() == name)
      return funcs[f];
  }
  return NULL;
}

inline Word image::get_instruction(Address adr) const {
  // TODO remove assert
  assert(isValidAddress(adr));

  if (isCode(adr)) {
    adr -= codeOffset_;
    adr >>= 2;
    const Word *inst = linkedFile.code_ptr();
    return (inst[adr]);
  } else if (isData(adr)) {
    adr -= dataOffset_;
    adr >>= 2;
    const Word *inst = linkedFile.data_ptr();
    return (inst[adr]);
  } else {
    abort();
    return 0;
  }
}

// Address must be in code or data range since some code may end up
// in the data segment
inline bool image::isValidAddress(const Address &where) const {
  return (!(where & 0x3) &&
	  (isCode(where) || isData(where)));
}

inline bool image::isCode(const Address &where) const {
  return (linkedFile.code_ptr() && 
	  (where >= codeOffset_) && (where < (codeOffset_+(codeLen_<<2))));
}

inline bool image::isData(const Address &where) const {
  return (linkedFile.data_ptr() && 
	  (where >= dataOffset_) && (where < (dataOffset_+(dataLen_<<2))));
}

inline bool image::symbol_info(string& symbol_name, Symbol &ret_sym) {
  if (!linkedFile.get_symbol(symbol_name, ret_sym))
    return false;
  else
    return true;
}

#endif
