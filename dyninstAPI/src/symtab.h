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

#ifndef SYMTAB_HDR
#define SYMTAB_HDR

/*
 * symtab.h - interface to generic symbol table.
 *
 * $Log: symtab.h,v $
 * Revision 1.27  1996/09/05 16:32:20  lzheng
 * Removing some warning and clean up the code a little bit.
 *
 * Revision 1.26  1996/08/20 19:01:00  lzheng
 * Implementation of moving multiple instructions sequence
 * Added a few variable. (need to change them to private member later)
 *
 * Revision 1.25  1996/08/16 21:20:01  tamches
 * updated copyright for release 1.1
 *
 * Revision 1.24  1996/07/09 04:06:53  lzheng
 * Add infomation of unwind table to assist the stack walking on HPUX.
 *
 * Revision 1.23  1996/05/09 19:21:44  lzheng
 * Minor fix to remove one ifdef for HPUX
 *
 * Revision 1.22  1996/04/29 22:18:51  mjrg
 * Added size to functions (get size from symbol table)
 * Use size to define function boundary
 * Find multiple return points for sparc
 * Instrument branches and jumps out of a function as return points (sparc)
 * Recognize tail-call optimizations and instrument them as return points (sparc)
 * Move instPoint to machine dependent files
 *
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

//Todo: move this class to machine dependent file?
class pdFunction {
 public:
    pdFunction(const string symbol, const string &pretty, module *f, Address adr,
	       const unsigned size,
	       const unsigned tg, const image *owner, bool &err);
    ~pdFunction() { /* TODO */ }

    bool findInstPoints(const image *owner);

    void checkCallPoints();
    bool defineInstPoint();
    Address newCallPoint(Address adr, const instruction code, 
			 const image *owner, bool &err);
    string symTabName() const { return symTabName_;}
    string prettyName() const { return prettyName_;}
    const module *file() const { return file_;}
    Address addr() const { return addr_;}
    unsigned size() const {return size_;}
    instPoint *funcEntry() const { return funcEntry_;}
    instPoint *funcEntry_;	/* place to instrument entry (often not addr) */
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
    vector<instPoint*> lr;
#endif
#if defined(sparc_sun_sunos4_1_3) || defined(sparc_sun_solaris2_4)   
    // ToDo : change those variables to the private.
    bool leaf;
    bool isTrap;  
    bool relocation;
    bool notInstalled;
    bool checkInstPoints(const image *owner);
    bool findInstPoints(const image *owner, Address adr, process *proc);
    void relocateFunction(process *proc);
    Address newCallPoint(Address &adr, const instruction code, const 
                         image *owner, bool &err, int &id, Address &addr);
    Address newAddr;
#endif

  private:
    unsigned tag_;
    string symTabName_;		/* name as it appears in the symbol table */
    string prettyName_;		/* user's view of name (i.e. de-mangled) */
    int line_;			/* first line of function */
    module *file_;		/* pointer to file that defines func. */
    Address addr_;		/* address of the start of the func */
    /*instPoint *funcEntry_;	 place to instrument entry (often not addr) */

    unsigned size_;             /* the function size, in bytes, used to
				   define the function boundaries. This may not
				   be exact, and may not be used on all 
				   platforms. */

};

class instPoint;
#ifdef notdef
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

bool conditionalPoint;
int target;
};
#endif

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

  pdFunction *findFunctionIn(const Address &addr);

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
  // static void destroy(pdFunction *pdf) { }
  // static void destroy(module *mod) { }

  vector<pdFunction*> mdlLib;
  vector<pdFunction*> mdlNormal;
  vector<module*> mods;

#if defined(hppa1_1_hp_hpux)
  vector<unwind_table_entry> unwind;
#endif

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

  bool newFunc(module *, const string name, const Address addr, const unsigned size,
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
