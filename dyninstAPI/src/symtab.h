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

// $Id: symtab.h,v 1.100 2001/10/04 20:04:45 buck Exp $

#ifndef SYMTAB_HDR
#define SYMTAB_HDR

extern "C" {
#include <sys/types.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
}

#include "common/h/Pair.h"
#include "common/h/Vector.h"
#include "common/h/Dictionary.h"
#include "common/h/list.h"
#include "dyninstAPI/src/Object.h"
#include "dyninstAPI/src/dyninst.h"
#include "dyninstAPI/src/arch.h"
#include "dyninstAPI/src/util.h"
#include "common/h/String.h"

#ifndef BPATCH_LIBRARY
#include "paradynd/src/resource.h"
#endif

#include "common/h/Types.h"
#include "common/h/Symbol.h"
#include "dyninstAPI/src/inst.h"

#ifndef mips_unknown_ce2_11 //ccw 8 apr 2001
#include "dyninstAPI/src/FunctionExpansionRecord.h"
class LocalAlterationSet;
#endif

#define RH_SEPERATOR '/'

/*
 * List of supported languages.
 *
 */
typedef enum { lang_Unknown,
	       lang_Assembly,
	       lang_C,
	       lang_CPlusPlus,
	       lang_GnuCPlusPlus,
	       lang_Fortran,
	       lang_CMFortran
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

class image;
class lineTable;
class process;
class pd_Function;

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
    const vector<instPoint*> &funcArbitraryPoints(){ return arbitraryPoints_;}
    const instPoint *funcEntry(){ return funcEntry_;}
    void addFuncEntry(instPoint *e){ if(e) funcEntry_ = e; }
    void addFuncReturn(instPoint *r){ if(r) funcReturns_.push_back(r); }
    void addFuncCall(instPoint *c){ if(c) calls_.push_back(c); }
    void addArbitraryPoint(instPoint *r){ if(r) arbitraryPoints_.push_back(r); }
    bool isInstalled(){ return installed_; }
    void setInstalled() { installed_ = true; }
private:
    const process *proc_;		// process assoc. with the relocation
    Address addr_;			// function's relocated address
    instPoint *funcEntry_;		// function entry point
    bool installed_;			// if true, function has been relocated
    vector<instPoint*> funcReturns_;    // return point(s)
    vector<instPoint*> calls_;          // pointer to the calls
    vector<instPoint*> arbitraryPoints_;          // pointer to the calls
};


class pdmodule;
class module;

class function_base {
public:
    function_base(const string &symbol, const string &pretty,
		Address adr, const unsigned size):
		line_(0), addr_(adr),size_(size) { 
		symTabName_.push_back(symbol); prettyName_.push_back(pretty); }
    virtual ~function_base() { /* TODO */ }

    /* The next two asserts should necver be reached, function_base has no
     * default constructor which leaves the string vectors empty, the if
     * is more or less a sanity check, if the asserts here are ever reached
     * then something really bad must have happened.  Since we will never
     * make it past the assert, in order to remove the compiler warnings
     * we add the return to return the same string from the first part
     * of the if statement
     */
    const string &symTabName() const { 
 	if (symTabName_.size() > 0) return symTabName_[0];
	else assert(0 && "symtab name"); return symTabName_[0]; }
    const string &prettyName() const {
 	if (prettyName_.size() > 0) return prettyName_[0];
	else assert(0 && "pretty name"); return prettyName_[0]; } 
    vector<string> symTabNameVector() { return symTabName_; }
    vector<string> prettyNameVector() { return prettyName_; }
    void addSymTabName(string name) { symTabName_.push_back(name); }
    void addPrettyName(string name) { prettyName_.push_back(name); }
    unsigned size() const {return size_;}
    Address addr() const {return addr_;}

    bool match(function_base *p);

    virtual Address getAddress(const process *p) = 0;
    virtual Address getEffectiveAddress(const process *p) const = 0;
    virtual const instPoint *funcEntry(process *p) const = 0;
    virtual const vector<instPoint*> &funcExits(process *p) const = 0;
    virtual const vector<instPoint*> &funcCalls(process *p) const = 0; 
    virtual const vector<instPoint*> &funcArbitraryPoints(process *p) const = 0; 
    virtual bool hasNoStackFrame() const = 0;
       // formerly "isLeafFunc()" but that led to confusion, since people assign two
       // different meanings to "leaf" fns: (1) has no stack frame, (2) makes no calls.
       // By renaming this fn, we make clear that we're returning (1), not (2).

    // extra debuggering info....
    ostream & operator<<(ostream &s) const;
    friend ostream &operator<<(ostream &os, function_base &f);
private:
    vector<string> symTabName_;	/* name as it appears in the symbol table */
    vector<string> prettyName_;	/* user's view of name (i.e. de-mangled) */
    int line_;			/* first line of function */
    Address addr_;		/* address of the start of the func */
    unsigned size_;             /* the function size, in bytes, used to
				   define the function boundaries. This may not
				   be exact, and may not be used on all 
				   platforms. */
    
};


class instPoint;
class pd_Function : public function_base {
 public:
    pd_Function(const string &symbol, const string &pretty, pdmodule *f, 
		Address adr, const unsigned size, 
		const image *owner, bool &err);
    ~pd_Function() { delete relocatedCode;  // delete the rewritten version 
                     delete originalCode;   // of the function if it was 
                     delete instructions;   // relocated      
                               /* TODO */ }

    bool findInstPoints(const image *owner);
    void checkCallPoints();
    bool defineInstPoint();
    pdmodule *file() const { return file_;}
    Address newCallPoint(Address adr, const instruction code, 
			 const image *owner, bool &err);

    // passing in a value of 0 for p will return the original address
    // otherwise, if the process is relocated it will return the new address
    Address getAddress(const process *p){
        if(p && relocatable_) { 
	  for(u_int i=0; i < relocatedByProcess.size(); i++){
	    if((relocatedByProcess[i])->getProcess() == p) 
		return (relocatedByProcess[i])->address();
	} }
	return addr();
    }
    Address getEffectiveAddress(const process *p) const;
    const instPoint *funcEntry(process *p) const {
        if(p && relocatable_) { 
	  for(u_int i=0; i < relocatedByProcess.size(); i++){
	    if((relocatedByProcess[i])->getProcess() == p) 
		return (relocatedByProcess[i])->funcEntry();
	} }
	return funcEntry_;
    }
    const vector<instPoint*> &funcExits(process *p) const {
        if(p && relocatable_) {
	  for(u_int i=0; i < relocatedByProcess.size(); i++){
	    if((relocatedByProcess[i])->getProcess() == p) 
		return (relocatedByProcess[i])->funcReturns();
	} }
	return funcReturns;
    }
    const vector<instPoint*> &funcArbitraryPoints(process *p) const {
        if(p && relocatable_) {
	  for(u_int i=0; i < relocatedByProcess.size(); i++){
	    if((relocatedByProcess[i])->getProcess() == p) 
		return (relocatedByProcess[i])->funcArbitraryPoints();
	} }
	return arbitraryPoints;
    }
    void addArbitraryPoint(instPoint* insp,process* p){
	if(insp) arbitraryPoints.push_back(insp);

	// Cheesy get-rid-of-compiler-warning
	process *unused = p;
	unused = 0;

#if defined(i386_unknown_nt4_0) || \
    defined(i386_unknown_linux2_0) || \
    defined(sparc_sun_solaris2_4)

        if(insp && p && relocatable_)
	  for(u_int i=0; i < relocatedByProcess.size(); i++)
	    if((relocatedByProcess[i])->getProcess() == p) {
		addArbitraryPoint(insp,p,relocatedByProcess[i]);
		return;
	    }
#endif
    }

#if defined(i386_unknown_nt4_0) || \
    defined(i386_unknown_linux2_0) || \
    defined(sparc_sun_solaris2_4)

    void addArbitraryPoint(instPoint*,process*,relocatedFuncInfo*);

#endif

    const vector<instPoint*> &funcCalls(process *p) const {
        if(p && relocatable_) {
	  for(u_int i=0; i < relocatedByProcess.size(); i++){
	    if((relocatedByProcess[i])->getProcess() == p) 
		return (relocatedByProcess[i])->funcCallSites();
	} }
	return calls;
    }
    bool isInstalled(process *p){
        if(p && relocatable_) {
	  for(u_int i=0; i < relocatedByProcess.size(); i++){
	    if((relocatedByProcess[i])->getProcess() == p) 
		return (relocatedByProcess[i])->isInstalled();
	} }
	return false;
    }
    void setInstalled(process *p){
        if(p && relocatable_) {
	  for(u_int i=0; i < relocatedByProcess.size(); i++){
	    if((relocatedByProcess[i])->getProcess() == p) 
	        (relocatedByProcess[i])->setInstalled();
	} }
    }
    bool hasNoStackFrame() const {return noStackFrame;}
       // formerly "isLeafFunc()" but that led to confusion, since people assign two
       // different meanings to "leaf" fns: (1) has no stack frame, (2) makes no calls.
       // By renaming this fn, we make clear that we're returning (1), not (2).

    bool isTrapFunc() {return isTrap;}
    bool needsRelocation() {return relocatable_;}
    void setRelocatable(bool value) { relocatable_ = value; }

    bool isInstrumentable() { return isInstrumentable_; }
    
#ifndef BPATCH_LIBRARY
    // Fill in <callees> vector with pointers to all other pd functions
    //  statically determined to be called from any call sites in 
    //  this function.
    // Returns false if unable to fill in that information....
    bool getStaticCallees(process *proc, vector <pd_Function *> &callees);
#endif

#if defined(sparc_sun_sunos4_1_3) || defined(sparc_sun_solaris2_4)   

    bool checkInstPoints(const image *owner);
    bool findInstPoints(const image *owner, Address adr, process *proc);
    // Add a new call point to a function that will be, or currently is
    // being relocated (if location != 0 && reloc_info != 0,  then this is
    // called when the the function is actually being relocated
    Address newCallPoint(Address &adr, const instruction code,
                         const image *owner, bool &err, unsigned &id, 
			 Address &addr, relocatedFuncInfo *reloc_info,
			 instPoint *&point, const instPoint *&location);

    bool PA_attachTailCalls(LocalAlterationSet *temp_alteration_set);

    //
    // NEW routines for function code rewrites using peephole alterations....
    //
    bool readFunctionCode(const image *owner, instruction *into);
    int moveOutOfDelaySlot(int offset, instruction loadedCode[],
	  int codeSize);
#elif defined(mips_sgi_irix6_4)  || defined(mips_unknown_ce2_11) //ccw 29 mar 2001
    bool    checkInstPoints();
    Address findTarget(instPoint *p);
    Address findBranchTarget(instPoint *p, instruction i);
    Address findJumpTarget(instPoint *p, instruction i);
    Address findIndirectJumpTarget(instPoint *p, instruction i);
    void    setVectorIds();

    // stack frame info
    Address findStackFrame(const image *owner);
    // register saves into frame
    struct regSave_t {
      int           slot;       // stack frame ($fp) offset of saved register
      bool          dword;      // is register saved as 64-bit doubleword?
      Address       insn;       // offset of insn that saves this register
    } reg_saves[NUM_REGS];
    // $sp-style frame (common)
    Address         sp_mod;     // offset of insn that modifies $sp
    vector<Address> sp_ret;     // offset of insn that restores $sp
    int             frame_size; // stack frame size ($sp frame only)
    // $fp-style frame (rare)
    Address         fp_mod;     // offset of insn that modifies $fp
    bool            uses_fp;    // does this fn use $s8 as a frame pointer?

    typedef struct inactiveRange {
       int popOffset;
       int retOffset;
       } InactiveFrameRange;

       vector<InactiveFrameRange> inactiveRanges;

#elif defined(i386_unknown_solaris2_5) || defined(i386_unknown_nt4_0) || defined(i386_unknown_linux2_0)

    void instrAroundPt(instPoint *p, instruction allInstr[], int numBefore, 
                       int numAfter, unsigned type, int index);

    int getArrayOffset(Address adr, instruction code[]);

    bool isTrueCallInsn(const instruction insn);
 
    bool canUseExtraSlot(instPoint *&ip) const;

    bool usesTrap(instPoint *&ip);

#elif defined(alpha_dec_osf4_0)
    int             frame_size; // stack frame size
#endif

#if defined(sparc_sun_solaris2_4)
    bool is_o7_live(){ return o7_live; }
#endif

#if defined(i386_unknown_solaris2_5) || defined(i386_unknown_nt4_0) || defined(i386_unknown_linux2_0) || defined(sparc_sun_solaris2_4)

    // modifyInstPoint: change the value of the instPoint if it is wrong: 
    // if this instPoint is from a function that was just relocated, then
    // it may not have the correct address.  This routine finds the correct
    // address for the instPoint
    void modifyInstPoint(const instPoint *&location, process *proc);

    bool isNearBranchInsn(const instruction insn);

    bool fillInRelocInstPoints(const image *owner, process *proc,
                               instPoint *&location, 
                               relocatedFuncInfo *&reloc_info, Address mutatee,
                               Address mutator, instruction oldCode[],
                               Address newAdr, instruction newCode[],
                               LocalAlterationSet &alteration_set);

    int relocateInstructionWithFunction(bool setDisp, 
                                        instruction *insn, 
                                        Address origAddr, 
                                        Address targetAddr, 
                                        Address oldFunctionAddr, 
                                        unsigned originalCodeSize);

    int patchOffset(bool setDisp, LocalAlterationSet *alteration_set, 
                    instruction& insn, Address adr, 
                    Address firstAddress, unsigned originalCodeSize);

    bool findAndApplyAlterations(const image *owner, 
	                         instPoint *&location,
      			         u_int &newAdr,
                                 process *proc,
                                 relocatedFuncInfo *&reloc_info, 
                                 unsigned &size_change);

    int findAlterations(const image *owner, process *proc, 
                        instruction *&oldCode,
                        LocalAlterationSet &normalized_alteration_set,
                        Address &mutator, Address &mutatee);

    int relocatedSizeChange(const image *owner, process *proc);

    bool loadCode(const image *owner, process *proc, instruction *&oldCode, 
                  unsigned &totalSize, Address &firstAddress);

    bool expandInstPoints(const image *owner, 
                          LocalAlterationSet *temp_alteration_set, 
                          LocalAlterationSet &normalized_alteration_set, 
                          Address baseAddress, Address mutator, 
                          Address mutatee, instruction oldCode[], 
                          unsigned numberOfInstructions);

    bool PA_attachGeneralRewrites(const image *owner, 
                                  LocalAlterationSet *temp_alteration_set, 
                                  Address baseAddress, Address firstAddress,
                                  instruction loadedCode[], 
                                  unsigned numInstructions, int codeSize);


    bool PA_attachOverlappingInstPoints(LocalAlterationSet *temp_alteration_set,
                                        Address baseAddress,
                                        Address firstAddress,
	                                instruction loadedCode[], int codeSize);

    bool PA_attachBranchOverlaps(LocalAlterationSet *temp_alteration_set, 
                                 Address baseAddress, Address firstAddress,
                                 instruction loadedCode[],
                                 unsigned  numberOfInstructions, int codeSize);

    bool discoverAlterations(LocalAlterationSet *temp_alteration_set, 
                             LocalAlterationSet &normalized_alteration_set,
			     Address baseAddress, Address firstAddress, 
                             instruction originalCode[], int originalCodeSize); 

    bool applyAlterations(LocalAlterationSet &normalized_alteration_set,
			  Address mutator, Address mutatee, Address newAdr, 
                          instruction originalCode[], 
			  unsigned originalCodeSize, instruction newCode[]);

    bool updateAlterations(LocalAlterationSet *temp_alteration_set,
                           LocalAlterationSet &normalized_alteration_set,
                           instruction *oldCode, 
                           Address baseAddress, Address firstAddress,
                           int &totalSizeChange);

    bool relocateFunction(process *proc, instPoint *&location, bool &deferred);

    void sorted_ips_vector(vector<instPoint*>&fill_in);

    void copyInstruction(instruction &newInsn, instruction &oldInsn,  
                                               unsigned &codeOffset);

    int expandInstructions(LocalAlterationSet &alteration_set, 
                           instruction &insn, 
                           Address offset,
                           instruction &newCodeInsn);

    int fixRelocatedInstruction(bool setDisp, instruction *insn, 
                                Address origAddr, Address targetAddr);

    bool branchInsideRange(instruction insn, Address branchAddress, 
                       Address firstAddress, Address lastAddress);

    bool trueCallInsideRange(instruction insn, Address callAddress, 
                         Address firstAddress, Address lastAddress);

    instPoint *find_overlap(vector<instPoint*> v, Address targetAddress);
#endif

    void setNumInstructions(unsigned num) { numInstructions = num; }
    unsigned getNumInstructions() { return numInstructions; }
 
    instruction *getInstructions() { return instructions; }



#ifndef BPATCH_LIBRARY
    void SetFuncResource(resource *r) {
      assert(r != NULL); 
      funcResource = r;
    }

    string ResourceFullName() {
      assert(funcResource); 
      return funcResource->full_name();
    }

    bool FuncResourceSet() {
      return (funcResource != NULL);
    }
#endif

  private:
    pdmodule *file_;		/* pointer to file that defines func. */
    instPoint *funcEntry_;	/* place to instrument entry (often not addr) */
    vector<instPoint*> funcReturns;	/* return point(s). */
    vector<instPoint*> calls;		/* pointer to the calls */
    vector<instPoint*> arbitraryPoints;		/* pointer to the calls */
#ifndef BPATCH_LIBRARY
    resource *funcResource;
#endif
    
    // these are for relocated functions
    bool relocatable_;		   // true if func will be relocated when instr

    unsigned char *relocatedCode;  // points to copy of rewritten function    
    unsigned char *originalCode;   // points to copy of original function

    unsigned numInstructions;      // num instructions in original func
    instruction *instructions;     // instructions that correspond to the 
                                   // original function 

    bool noStackFrame; // formerly "leaf".  True iff this fn has no stack frame.

    bool isTrap; 		// true if function contains a trap instruct
    bool isInstrumentable_;     // true if the function is instrumentable
    vector<relocatedFuncInfo *> relocatedByProcess; // one element per process

#if defined(sparc_sun_solaris2_4)
    bool o7_live;
#endif
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
    module(){}
    module(supportedLanguages lang, Address adr, string &fullNm,
	   string &fileNm): fileName_(fileNm), fullName_(fullNm), 
		language_(lang), addr_(adr){}
    virtual ~module(){}

    string fileName() const { return fileName_; }
    string fullName() const { return fullName_; }
    supportedLanguages language() const { return language_;}
    Address addr() const { return addr_; }

    virtual function_base *findFunction (const string &name) = 0;
    virtual function_base *findFunctionFromAll(const string &name) = 0;
    virtual void define() = 0;    // defines module to paradyn
    virtual vector<function_base *> *getFunctions() = 0;

#ifndef BPATCH_LIBRARY
    virtual vector<function_base *> *getIncludedFunctions() = 0;
#endif

private:
    string fileName_;                   // short file 
    string fullName_;                   // full path to file 
    supportedLanguages language_;
    Address addr_;                      // starting address of module
};

class pdmodule: public module {
friend class image;
public:
  pdmodule(supportedLanguages lang, Address adr, string &fullNm,
	   string &fileNm, image *e): module(lang,adr,fullNm,fileNm),
#ifndef BPATCH_LIBRARY
    modResource(0),
#endif
    exec_(e){
    some_funcs_inited = FALSE;
  }
  ~pdmodule() { /* TODO */ }

  void setLineAddr(unsigned line, Address addr) {
	lines_.setLineAddr(line, addr);}
  bool getLineAddr(unsigned line, Address &addr) { 
       return (lines_.getLineAddr(line, addr)); }

  image *exec() const { return exec_; }
  void mapLines() { }           // line number info is not used now
  void checkAllCallPoints();
  void define();    // defines module to paradyn

#ifndef BPATCH_LIBRARY
  void FillInCallGraphStatic(process *proc);
      // fill in statically determined part of caller-callee relationship
      //  for paradyn....
#endif

  vector<function_base *> *getFunctions() { return (vector<function_base *>*)&funcs;} 
  vector<function_base *> *getIncludedFunctions();
  function_base *findFunction (const string &name);
  function_base *findFunctionFromAll(const string &name);

private:
#ifndef BPATCH_LIBRARY
  resource *modResource;
#endif
  image *exec_;                      // what executable it came from 
  lineDict lines_;
  //  list of all found functions in module....
  vector<pd_Function*> funcs;
  vector<pd_Function*> notInstruFuncs;
  // added as part of exclude support for statically linked objects.
  //  mcheyny, 970928
  //  list of non-excluded found functions in module....
  vector<pd_Function*> some_funcs;
  bool some_funcs_inited;
};


extern bool mdl_get_lib_constraints(vector<string> &);

void print_func_vector_by_pretty_name(string prefix,
				      vector<function_base *>*funcs);
void print_module_vector_by_short_name(string prefix,
                                      vector<pdmodule*> *mods);
string getModuleName(string constraint);
string getFunctionName(string constraint);
//used by both sharedobject and pdmodule class....
bool filter_excluded_functions(vector<pd_Function*> all_funcs,
    vector<pd_Function*>& some_funcs, string module_name);
bool function_is_excluded(pd_Function *func, string module_name);
bool module_is_excluded(pdmodule *module);


/*
 * symbols we need to find from our RTinst library.  This is how we know
 *   were our inst primatives got loaded as well as the data space variables
 *   we use to put counters/timers and inst trampolines.  An array of these
 *   is placed in the image structure.
 *
 */
class internalSym {
public:
  internalSym() { }
  internalSym(const Address adr, const string &nm) : name(nm), addr(adr) { }
  Address getAddr() const { return addr;}

private:
  string name;            /* name as it appears in the symbol table. */
  Address addr;      /* absolute address of the symbol */
};


// modsByFileName
// modsbyFullName
// includedMods
// excludedMods
// allMods
// includedFunctions
// excludedFunctions
// instrumentableFunctions
// notInstruFunctions
// funcsByAddr
// funcsByPretty
// file_
// name_
// codeOffset_
// codeLen_
// dataOffset_
// dataLen_
// linkedFile
// iSymsMap
// allImages
// varsByPretty
// knownJumpTargets
// COMMENTS????
//  Image class contains information about statically and dynamically linked code 
//  belonging to a process....
class image {
   friend class process;
   friend class pd_Function;

   //
   // ****  PUBLIC MEMBER FUBCTIONS  ****
   //
public:
  static image *parseImage(const string file);
  static image *parseImage(fileDescriptor *desc);

  image(fileDescriptor *desc, bool &err);
  ~image() { /* TODO */ }

  // Check the list of symbols returned by the parser, return
  // name/addr pair
  bool findInternalSymbol(const string &name, const bool warn, internalSym &iSym);

  // Check the list of symbols returned by the parser, return
  // all which start with the given prefix
  bool findInternalByPrefix(const string &prefix, vector<Symbol> &found) const;

  
  //Address findInternalAddress (const string &name, const bool warn, bool &err);

  // find the named module  
  pdmodule *findModule(const string &name, bool find_if_excluded = FALSE);

  // Note to self later: find is a const operation, [] isn't, for
  // dictionary access. If we need to make the findFuncBy* methods
  // consts, we can move from [] to find()

  // Find the vector of functions associated with a (demangled) name
  vector <pd_Function *> *findFuncVectorByPretty(const string &name);
  // Find a (single) function by pretty (demangled) name. Picks one if more than
  // one exists. Probably shouldn't exist.
  pd_Function *findFuncByPretty(const string &name);
  // Find a function by mangled (original) name. Guaranteed unique
  pd_Function *findFuncByMangled(const string &name);
  // Look for the function in the non instrumentable list
  pd_Function *findNonInstruFunc(const string &name);
  // Look for the function in the excluded list
  pd_Function *findExcludedFunc(const string &name);
  // Looks only for an instrumentable, non-excluded function
  pd_Function *findFuncByName(const string &name);
  // Looks for the name in all lists (inc. excluded and non-instrumentable)
  pd_Function *findOneFunctionFromAll(const string &name);

  // Given an address, do an exhaustive search for that function
  pd_Function *findFuncByAddr(const Address &addr, const process *p = 0) const;

  // Break apart the above
  pd_Function *findFuncByEntryAddr(const Address &addr, const process *p = 0) const;
  pd_Function *findFuncByRelocAddr(const Address &addr, const process *p = 0) const;
  pd_Function *findFuncByOrigAddr(const Address &addr, const process *p = 0) const;

  void findModByAddr (const Symbol &lookUp, vector<Symbol> &mods,
		      string &modName, Address &modAddr, 
		      const string &defName);

  // report modules to paradyn
  void defineModules();

#ifndef BPATCH_LIBRARY
 // report statically determinable caller-callee relationship to paradyn....
  void FillInCallGraphStatic(process *proc);
#endif

  bool symbolExists(const string &); /* Does the symbol exist in the image? */
  void postProcess(const string);          /* Load .pif file */


  void addJumpTarget(Address addr) {
    if (!knownJumpTargets.defines(addr)) knownJumpTargets[addr] = addr; 
  }

  bool isJumpTarget(Address addr) { 
    return knownJumpTargets.defines(addr); 
  }


  // data member access

  string file() const {return desc_->file();}
  string name() const { return name_;}
  const fileDescriptor *desc() const { return desc_; }
  Address codeOffset() { return codeOffset_;}
  Address dataOffset() { return dataOffset_;}
  Address dataLength() { return (dataLen_ << 2);} 
  Address codeLength() { return (codeLen_ << 2);} 
  const Object &getObject() const { return linkedFile; }

  Object &getObjectNC() { return linkedFile; } //ccw 27 july 2000 : this is a TERRIBLE hack : 29 mar 2001

  bool isDyninstRTLib() const { return is_libdyninstRT; }
  bool isAOut() const { return is_a_out; }

  inline bool isCode(const Address &where) const;
  inline bool isData(const Address &where) const;
  inline bool isValidAddress(const Address &where) const;
  inline const Word get_instruction(Address adr) const;
  inline const unsigned char *getPtrToInstruction(Address adr) const;

  // Return symbol table information
  inline bool symbol_info(const string& symbol_name, Symbol& ret);

  // Called from the mdl -- lists of functions to look for
  static void watch_functions(string& name, vector<string> *vs, bool is_lib,
			      vector<pd_Function*> *updateDict);

  // origionally return mdlNormal;....
  // Note that (unlike name), this returns ONLY functions for which
  // necessary instrumentation info could be found)!!!!
  const vector<pd_Function*> &getAllFunctions();

  // get all modules, including excluded ones....
  const vector<pdmodule *> &getAllModules();

#ifndef BPATCH_LIBRARY
  const vector<pd_Function*> &getIncludedFunctions();
  const vector<pdmodule *> &getIncludedModules();
#endif 

  //
  //  ****  PUBLIC DATA MEMBERS  ****
  //

  Address get_main_call_addr() const { return main_call_addr_; }
 
  private:

  // Call tree:
  //
  // AddOneFunction:
  //    Gets/Creates a module name ("DEFAULT_MODULE")
  //    newFunc
  // newFunc
  //    new pd_Function()
  //    if (problem determining instr)
  //      addNotInstruFunction
  //    else
  //      addInstruFunction
  //        function_is_excluded (MDL exclusion via "exclude <mod>/<func>", BUT NOT "exclude <mod>)
  // addInstruFunction
  //   if (excluded) [from above]
  //     add to excludedFunctions (hash)
  //   else
  //     add to includedFunctions
  //     add to funcsByAddr
  //     add to funcsByPretty (if not already in there)
  //     add to funcsByMangled (if not already in there)
  // addNotInstruFunction
  //   add to notInstruFunctions (by pretty name)

  // Add a function to (if excluded) excluded list, otherwise to includedFunctions,
  // funcsByAddr, funcsByPretty, funcsByMangled
  void addInstruFunction(pd_Function *func, pdmodule *mod,
        const Address addr, bool excluded);

  // Add a function which could not be instrumented.  Sticks it in
  // notInstruFuncs (list)
  void addNotInstruFunc(pd_Function *func, pdmodule *mod);

  // Determines if a function is instrumentable, and calls add(Not)InstruFunction
  bool newFunc(pdmodule *, const string &name, const Address addr, 
	       const unsigned size);

  bool addOneFunction(vector<Symbol> &mods,
		      const Symbol &lookUp);

  void addMultipleFunctionNames(vector<Symbol> &mods,
				const Symbol &lookUp);

  //
  //  ****  PRIVATE MEMBERS FUNCTIONS  ****
  //

  // private methods for findind an excluded function by name or
  //  address....
  bool find_excluded_function(const string &name,
      vector<pd_Function*> &retList);
  pd_Function *find_excluded_function(const Address &addr);

  void checkAllCallPoints();

#if 0
  bool addInternalSymbol(const string &str, const Address symValue);
#endif
  // creates the module if it does not exist
  pdmodule *getOrCreateModule (const string &modName, const Address modAddr);
  pdmodule *newModule(const string &name, const Address addr);

  bool addAllFunctions(vector<Symbol> &mods);

  bool addAllVariables();


  //
  //  ****  PRIVATE DATA MEMBERS  ****
  //

  fileDescriptor *desc_; /* file descriptor (includes name) */
  string name_;		/* filename part of file, no slashes */

  Address codeOffset_;
  unsigned codeLen_;
  Address dataOffset_;
  unsigned dataLen_;

  bool is_libdyninstRT;
  bool is_a_out;
  Address main_call_addr_; // address of call to main()

  // data from the symbol table 
  Object linkedFile;

  //dictionary_hash <string, internalSym*> iSymsMap;   // internal RTinst symbols

  // A vector of all images. Used to avoid duplicating
  // an "image" that already exists.
  static vector<image*> allImages;

  // knownJumpTargets: the addresses in this image that are known to 
  // be targets of jumps. It is used to check points with multiple 
  // instructions.
  // This is a subset of the addresses that are actually targets of jumps.
  dictionary_hash<Address, Address> knownJumpTargets;

  // list of modules which have not been excluded.
  vector<pdmodule *> includedMods;
  // list of excluded module.  includedMods && excludedMods
  //  should be disjoint!!!!
  vector<pdmodule *> excludedMods;
  // list of all modules, should = includedMods + excludedMods;
  // Not actually created until getAllModules called....
  vector<pdmodule *> allMods;

  // list of all functions for which necessary instrumentation data
  //  could be found which are NOT excluded....
  vector<pd_Function*> includedFunctions;
  // includedFunctions + excludedFunctions (but not notInstruFunctions)....
  vector<pd_Function*> instrumentableFunctions;


  //
  // Hash Tables of Functions....
  //

  // functions by address for all modules.  Only contains instrumentable
  //  funtions.
  dictionary_hash <Address, pd_Function*> funcsByAddr;
  // note, a prettyName is not unique, it may map to a function appearing
  // in several modules.  Also only contains instrumentable functions....
  dictionary_hash <string, vector<pd_Function*>*> funcsByPretty;
  // Hash table holding functions by mangled name.
  // Should contain same functions as funcsByPretty....
  dictionary_hash <string, vector<pd_Function*>*> funcsByMangled;
  // The functions that can't be instrumented
  // Note that notInstruFunctions holds list of functions for which
  //  necessary instrumentation data could NOT be found....
  dictionary_hash <string, pd_Function*> notInstruFunctions;
  // hash table of all functions for which necessary instrumentation data
  //  could be found which ARE excluded....
  dictionary_hash <string, pd_Function*> excludedFunctions;
  // TODO -- get rid of one of these
  // Note : as of 971001 (mcheyney), these hash tables only 
  //  hold entries in includedMods --> this implies that
  //  it may sometimes be necessary to do a linear sort
  //  through excludedMods if searching for a module which
  //  was excluded....
  dictionary_hash <string, pdmodule *> modsByFileName;
  dictionary_hash <string, pdmodule*> modsByFullName;
  // Variables indexed by pretty (non-mangled) name
  dictionary_hash <string, vector<string>*> varsByPretty;
 

};


#ifndef BPATCH_LIBRARY
// forward declarations....
void CallGraphSetEntryFuncCallback(string exe_name, string r);
void CallGraphFillDone(string exe_name);
void CallGraphAddProgramCallback(string exe_name);
#endif
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

#ifndef BPATCH_LIBRARY
// TODO -- remove this
extern resource *moduleRoot;
#endif

inline bool lineDict::getLineAddr (const unsigned line, Address &adr) {
  if (!lineMap.defines(line)) {
    return false;
  } else {
    adr = lineMap[line];
    return true;
  }
}

inline function_base *pdmodule::findFunction (const string &name) {
  unsigned fsize = funcs.size();
  for (unsigned f=0; f<fsize; f++) {
    if (funcs[f]->symTabName() == name)
      return funcs[f];
    else if (funcs[f]->prettyName() == name)
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
  if (linkedFile.get_symbol(symbol_name, ret_sym))
    return true;

  if (varsByPretty.defines(symbol_name)) {
    vector<string> *mangledNames = varsByPretty[symbol_name];
    assert(mangledNames && mangledNames->size() == 1);
    if (linkedFile.get_symbol((*mangledNames)[0], ret_sym))
      return true;
  }

  return false;
}

#endif

