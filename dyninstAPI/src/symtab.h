
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
#include "dyninstAPI/src/dyninst.h"
#include "dyninstAPI/src/arch.h"
#include "dyninstAPI/src/util.h"
#include "util/h/String.h"
#include "paradynd/src/resource.h"
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

class module;
class image;
class lineTable;
class process;

// if a function needs to be relocated when it's instrumented then we need
// to keep track of new instrumentation points for this function on a per
// process basis (there is no guarentee that two processes are going to
// relocated this function to the same location in the heap)
class relocatedFuncInfo {
public:
    relocatedFuncInfo(process *p,Address na):proc_(p),
	   	      addr_(na),funcEntry_(0),installed_(false){}
    ~relocatedFuncInfo(){proc_ = 0;}
    Address address(){ return addr_;}
    const process *getProcess(){ return proc_;}
    const vector<instPoint*> &funcReturns(){ return funcReturns_;}
    const vector<instPoint*> &funcCallSites(){ return calls_;}
    const instPoint *funcEntry(){ return funcEntry_;}
    void addFuncEntry(instPoint *e){ if(e) funcEntry_ = e; }
    void addFuncReturn(instPoint *r){ if(r) funcReturns_ += r; }
    void addFuncCall(instPoint *c){ if(c) calls_ += c; }
    bool isInstalled(){ return installed_; }
    void setInstalled() { installed_ = true; }
private:
    const process *proc_;		// process assoc. with the relocation
    Address addr_;			// function's relocated address
    instPoint *funcEntry_;		// function entry point
    bool installed_;			// if true, function has been relocated
    vector<instPoint*> funcReturns_;    // return point(s)
    vector<instPoint*> calls_;          // pointer to the calls
};

//Todo: move this class to machine dependent file?
class pdFunction {
 public:

    pdFunction(const string symbol, const string &pretty, module *f, 
		Address adr, const unsigned size, const unsigned tg, 
		const image *owner, bool &err);
    ~pdFunction() { /* TODO */ }

    bool findInstPoints(const image *owner);
    void checkCallPoints();
    bool defineInstPoint();
    Address newCallPoint(Address adr, const instruction code, 
			 const image *owner, bool &err);
    string symTabName() const { return symTabName_;}
    string prettyName() const { return prettyName_;}
    const module *file() const { return file_;}
    unsigned size() const {return size_;}
    // passing in a value of 0 for p will return the original address
    // otherwise, if the process is relocated it will return the new address
    Address getAddress(const process *p){
        if(p && relocatable_) { 
	  for(u_int i=0; i < relocatedByProcess.size(); i++){
	    if((relocatedByProcess[i])->getProcess() == p) 
		return (relocatedByProcess[i])->address();
	} }
	return addr_;
    }
    const instPoint *funcEntry(process *p) const {
        if(relocatable_) { 
	  for(u_int i=0; i < relocatedByProcess.size(); i++){
	    if((relocatedByProcess[i])->getProcess() == p) 
		return (relocatedByProcess[i])->funcEntry();
	} }
	return funcEntry_;
    }
    const vector<instPoint*> &funcExits(process *p) const {
        if(relocatable_) {
	  for(u_int i=0; i < relocatedByProcess.size(); i++){
	    if((relocatedByProcess[i])->getProcess() == p) 
		return (relocatedByProcess[i])->funcReturns();
	} }
	return funcReturns;
    }
    const vector<instPoint*> &funcCalls(process *p) const {
        if(relocatable_) {
	  for(u_int i=0; i < relocatedByProcess.size(); i++){
	    if((relocatedByProcess[i])->getProcess() == p) 
		return (relocatedByProcess[i])->funcCallSites();
	} }
	return calls;
    }
    bool isInstalled(process *p){
        if(relocatable_) {
	  for(u_int i=0; i < relocatedByProcess.size(); i++){
	    if((relocatedByProcess[i])->getProcess() == p) 
		return (relocatedByProcess[i])->isInstalled();
	} }
	return false;
    }
    void setInstalled(process *p){
        if(relocatable_) {
	  for(u_int i=0; i < relocatedByProcess.size(); i++){
	    if((relocatedByProcess[i])->getProcess() == p) 
	        (relocatedByProcess[i])->setInstalled();
	} }
    }
    inline void tagAsLib() { tag_ |= TAG_LIB_FUNC;}
    inline void untagAsLib() { tag_ &= ~TAG_LIB_FUNC;}
    inline bool isTagSimilar(const unsigned comp) const { return(tag_ & comp);}
    bool isLibTag() const { return (tag_ & TAG_LIB_FUNC);}
    unsigned tag() const { return tag_; }
    bool isLeafFunc() {return leaf;}
    bool isTrapFunc() {return isTrap;}

#if defined(hppa1_1_hp_hpux)
    instruction entryPoint;
    instruction exitPoint;
    vector<instPoint*> lr;
#endif
#if defined(sparc_sun_sunos4_1_3) || defined(sparc_sun_solaris2_4)   

    bool checkInstPoints(const image *owner);
    bool findInstPoints(const image *owner, Address adr, process *proc);
    bool findNewInstPoints(const image *owner, 
			  const instPoint *&location, Address adr, 
			  process *proc,
			  vector<instruction> &extra_instrs, 
			  relocatedFuncInfo *reloc_info);
    bool relocateFunction(process *proc, const instPoint *&location,
			  vector<instruction> &extra_instrs);
    // Add a new call point to a function that will be, or currently is
    // being relocated (if location != 0 && reloc_info != 0,  then this is
    // called when the the function is actually being relocated
    Address newCallPoint(Address &adr, const instruction code, const 
                         image *owner, bool &err, int &id, Address &addr,
			 relocatedFuncInfo *reloc_info,
			 const instPoint *&location);
    // modifyInstPoint: change the value of the instPoint if it is wrong: 
    // if this instPoint is from a function that was just relocated, then
    // it may not have the correct address.  This routine finds the correct
    // address for the instPoint
    void modifyInstPoint(const instPoint *&location,process *proc);
#endif

  private:
    unsigned tag_;
    string symTabName_;		/* name as it appears in the symbol table */
    string prettyName_;		/* user's view of name (i.e. de-mangled) */
    int line_;			/* first line of function */
    module *file_;		/* pointer to file that defines func. */
    Address addr_;		/* address of the start of the func */
    unsigned size_;             /* the function size, in bytes, used to
				   define the function boundaries. This may not
				   be exact, and may not be used on all 
				   platforms. */
    instPoint *funcEntry_;	/* place to instrument entry (often not addr) */
    vector<instPoint*> funcReturns;	/* return point(s). */
    vector<instPoint*> calls;		/* pointer to the calls */

    // these are for relocated functions
    bool relocatable_;		// true if func will be relocated when instr
    bool leaf;  		// true if function is a leaf function
    bool isTrap; 		// true if function contains a trap instruct
    vector<relocatedFuncInfo *> relocatedByProcess; // one element per process
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



class image {
   friend class process;
public:
  static image *parseImage(const string file);
  static image *parseImage(const string file,u_int baseAddr);
  static void changeLibFlag(resource*, const bool);

  image(const string &file, bool &err);
  image(const string &file, u_int baseAddr, bool &err);
  ~image() { /* TODO */ }

  internalSym *findInternalSymbol(const string &name, const bool warn);
  Address findInternalAddress(const string &name, const bool warn, bool &err);

  // find the named module 
  module *findModule(const string &name);

  // find the function by name, address, or the first by name
  bool findFunction(const string &name, vector<pdFunction*> &flist);
  pdFunction *findFunction(const Address &addr);
  pdFunction *findOneFunction(const string &name);
  pdFunction *findOneFunctionFromAll(const string &name);

  pdFunction *findFunctionIn(const Address &addr,const process *p);

  // report modules to paradyn
  void defineModules();

  bool symbolExists(const string &); /* Does the symbol exist in the image? */
  void postProcess(const string);          /* Load .pif file */

  // data member access
  inline const Word get_instruction(Address adr) const;

  inline const unsigned char *getPtrToInstruction(Address adr) const;

  string file() const {return file_;}
  string name() const { return name_;}
  Address codeOffset() { return codeOffset_;}
  Address dataOffset() { return dataOffset_;}
  Address dataLength() { return (dataLen_ << 2);} 
  Address codeLength() { return (codeLen_ << 2);} 
  inline bool isCode(const Address &where) const;
  inline bool isData(const Address &where) const;

  // functions by address for all modules
  dictionary_hash <Address, pdFunction*> funcsByAddr;

  // TODO -- get rid of one of these
  dictionary_hash <string, module *> modsByFileName;
  dictionary_hash <string, module*> modsByFullName;

  inline bool isValidAddress(const Address &where) const;

  // Return symbol table information
  inline bool symbol_info(const string& symbol_name, Symbol& ret);

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

  vector <pdFunction *> notInstruFunction;
  // The functions that we are not going to instrument 

  bool newFunc(module *, const string name, const Address addr, const unsigned size,
	       const unsigned tags, pdFunction *&retFunc);

  void checkAllCallPoints();

  bool addInternalSymbol(const string &str, const Address symValue);

  // creates the module if it does not exist
  module *getOrCreateModule (const string &modName, const Address modAddr);
  module *newModule(const string &name, const Address addr);

  bool addOneFunction(vector<Symbol> &mods, module *lib, module *dyn,
		      const Symbol &lookUp, pdFunction *&retFunc);

  bool addAllFunctions(vector<Symbol> &mods,
		       module *lib, module *dyn,
		       const bool startB, const Address startAddr,
		       const bool endB, const Address endAddr);

  bool addAllSharedObjFunctions(vector<Symbol> &mods,
		       module *lib, module *dyn);


  // if useLib = true or the functions' tags signify a library function
  // the function is put in the library module
  bool defineFunction(module *use, const Symbol &sym, const unsigned tags,
		      pdFunction *&retFunc);
  bool defineFunction(module *lib, const Symbol &sym,
		      const string &modName, const Address modAdr,
		      pdFunction *&retFunc);

  bool heapIsOk(const vector<sym_data>&);

  // knownJumpTargets: the addresses in this image that are known to 
  // be targets of jumps. It is used to check points with multiple 
  // instructions.
  // This is a subset of the addresses that are actually targets of jumps.
  dictionary_hash<Address, Address> knownJumpTargets;

public:
  void addJumpTarget(Address addr) {
    if (!knownJumpTargets.defines(addr)) knownJumpTargets[addr] = addr; 
  }

  bool isJumpTarget(Address addr) { 
    return knownJumpTargets.defines(addr); 
  }

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

inline const Word image::get_instruction(Address adr) const{
  // TODO remove assert
  // assert(isValidAddress(adr));
  if(!isValidAddress(adr)){
      // logLine("address not valid in get_instruction\n");
      return 0;
  }

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

// return a pointer to the instruction at address adr
inline const unsigned char *image::getPtrToInstruction(Address adr) const {
  assert(isValidAddress(adr));
  if (isCode(adr)) {
    adr -= codeOffset_;
    const unsigned char *inst = (const unsigned char *)linkedFile.code_ptr();
    return (&inst[adr]);
  } else if (isData(adr)) {
    adr -= dataOffset_;
    const unsigned char *inst = (const unsigned char *)linkedFile.data_ptr();
    return (&inst[adr]);
  } else {
    abort();
    return 0;
  }
}


// Address must be in code or data range since some code may end up
// in the data segment
inline bool image::isValidAddress(const Address &where) const{
  return (isAligned(where) && (isCode(where) || isData(where)));
}

inline bool image::isCode(const Address &where)  const{
  return (linkedFile.code_ptr() && 
	  (where >= codeOffset_) && (where < (codeOffset_+(codeLen_<<2))));
}

inline bool image::isData(const Address &where)  const{
   return (linkedFile.data_ptr() && 
	  (where >= dataOffset_) && (where < (dataOffset_+(dataLen_<<2))));
}

inline bool image::symbol_info(const string& symbol_name, Symbol &ret_sym) {
  if (!linkedFile.get_symbol(symbol_name, ret_sym))
    return false;
  else
    return true;
}

#endif
