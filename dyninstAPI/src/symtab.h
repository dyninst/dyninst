/*
 * Copyright (c) 1996-2004 Barton P. Miller
 * 
 * We provide the Paradyn Parallel Performance Tools (below
 * described as "Paradyn") on an AS IS basis, and do not warrant its
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
 
// $Id: symtab.h,v 1.161 2004/08/08 21:46:14 lharris Exp $

#ifndef SYMTAB_HDR
#define SYMTAB_HDR
//#define REGEX_CHARSET "^*[]|?"
#define REGEX_CHARSET "^*|?"
extern "C" {
#include <sys/types.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#if !defined(i386_unknown_nt4_0) && !defined(mips_unknown_ce2_11)
#include <regex.h>
#endif
}

#include "common/h/Vector.h"
#include "common/h/Dictionary.h"
#include "common/h/List.h"
#include "dyninstAPI/src/Object.h"
#include "dyninstAPI/src/dyninst.h"
#include "dyninstAPI/src/arch.h"
#include "dyninstAPI/src/util.h"
#include "dyninstAPI/src/LineInformation.h"
#include "dyninstAPI/h/BPatch_Vector.h"
#include "dyninstAPI/h/BPatch_basicBlock.h"
#include "common/h/String.h"
#include "dyninstAPI/src/codeRange.h"

#ifndef BPATCH_LIBRARY
#include "paradynd/src/resource.h"

#define CHECK_ALL_CALL_POINTS  // we depend on this for Paradyn
#endif

#include "common/h/Types.h"
#include "common/h/Symbol.h"
#include "dyninstAPI/src/inst.h"

#ifndef mips_unknown_ce2_11 //ccw 8 apr 2001
#include "dyninstAPI/src/FunctionExpansionRecord.h"
class LocalAlterationSet;
#endif


typedef bool (*functionNameSieve_t)(const char *test,void *data);
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
	       lang_Fortran_with_pretty_debug,
	       lang_CMFortran
	       } supportedLanguages;

enum { EntryPt, CallPt, ReturnPt, OtherPt };
class point_ {
public:
   point_(): point(0), index(0), type(0) {};
   point_(instPoint *p, unsigned i, unsigned t): point(p), index(i), type(t)
       {  };
   instPoint *point;
   unsigned index;
   unsigned type;
};

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
class Frame;
class ExceptionBlock;
struct pdFuncCmp;

// if a function needs to be relocated when it's instrumented then we need
// to keep track of new instrumentation points for this function on a per
// process basis (there is no guarentee that two processes are going to
// relocated this function to the same location in the heap)
class relocatedFuncInfo : public codeRange {
 public:
   relocatedFuncInfo(process *p, Address na, unsigned s, pd_Function *f):
      proc_(p), addr_(na), size_(s), funcEntry_(0), func_(f) { }
        
   ~relocatedFuncInfo(){proc_ = 0;}
   Address get_address() const { return addr_;}
   unsigned get_size() const { return size_;}
   pd_Function *func() { return func_;}
    
   const process *getProcess(){ return proc_;}
   void setProcess(process *proc) { proc_ = proc; }
   instPoint *funcEntry() {
      return funcEntry_;
   }
   const pdvector<instPoint*> &funcReturns() {
      return funcReturns_;
   }
   const pdvector<instPoint*> &funcCallSites() {
      return calls_;
   }
   const pdvector<instPoint*> &funcArbitraryPoints() {
      return arbitraryPoints_;
   }

   void addFuncEntry(instPoint *e) {
      if(e) funcEntry_ = e;
   }
   void addFuncReturn(instPoint *r) {
      if(r) funcReturns_.push_back(r);
   }
   void addFuncCall(instPoint *c) {
      if(c) calls_.push_back(c);
   }
   void addArbitraryPoint(instPoint *r) {
      if(r) arbitraryPoints_.push_back(r);
   }

 private:
   const process *proc_;		// process assoc. with the relocation
   Address addr_;			// function's relocated address
   unsigned size_;             // Bulked-up size    
   instPoint *funcEntry_;		// function entry point
   pdvector<instPoint*> funcReturns_;    // return point(s)
   pdvector<instPoint*> calls_;          // pointer to the calls
   pdvector<instPoint*> arbitraryPoints_;          // pointer to the calls
   pd_Function *func_;         // "Parent" function pointer
};


class pdmodule;
class module;

class function_base : public codeRange {
 public:
   static pdstring emptyString;

   function_base(const pdstring &symbol, Address adr, const unsigned size) :
#if defined( ia64_unknown_linux2_4 )
		/* This appears to be the only constructor, so this should be
		   sufficient to make sure we don't think we have an AST when
		   we don't. */
		framePointerCalculator( NULL ), usedFPregs( NULL ),
#endif
		line_(0), addr_(adr), size_(size) {

       symTabName_.push_back(symbol);

#if defined(arch_x86)
       blockList = new pdvector< BPatch_basicBlock* >;
#endif
   }

   virtual ~function_base() {
#if defined(arch_x86)
       delete blockList;
#endif
   }

   /* The next two asserts should necver be reached, function_base has no
    * default constructor which leaves the pdstring vectors empty, the if
    * is more or less a sanity check, if the asserts here are ever reached
    * then something really bad must have happened.  Since we will never
    * make it past the assert, in order to remove the compiler warnings
    * we add the return to return the same pdstring from the first part
    * of the if statement
    */
   const pdstring &symTabName() const { 
      if (symTabName_.size() > 0) return symTabName_[0];
      else return emptyString;
   }
   const pdstring &prettyName() const {
      if (prettyName_.size() > 0) return prettyName_[0];
      else return emptyString;
   }
   pdvector<pdstring> symTabNameVector() { return symTabName_; }
   pdvector<pdstring> prettyNameVector() { return prettyName_; }
   void addSymTabName(pdstring name) { symTabName_.push_back(name); }
   void addPrettyName(pdstring name) { prettyName_.push_back(name); }

   unsigned get_size() const {return size_;}
   Address get_address() const {return addr_;}
   pdvector< BPatch_basicBlock* >* blocks() const{ return blockList; }
   bool match(function_base *p);

   virtual Address getAddress(const process *p) = 0;
   virtual Address getEffectiveAddress(const process *p) const = 0;
   virtual instPoint *funcEntry(const process *p) const = 0;
   virtual const pdvector<instPoint*> &funcExits(const process *p) const = 0;
   virtual const pdvector<instPoint*> &funcCalls(const process *p)  = 0; 
   virtual const pdvector<instPoint*> &funcArbitraryPoints(const process *p) const = 0; 
   virtual bool hasNoStackFrame() const = 0;
   // formerly "isLeafFunc()" but that led to confusion, since people assign two
   // different meanings to "leaf" fns: (1) has no stack frame, (2) makes no calls.
   // By renaming this fn, we make clear that we're returning (1), not (2).
   virtual bool makesNoCalls() const = 0;
   virtual bool savesFramePointer() const = 0;

   // extra debuggering info....
   ostream & operator<<(ostream &s) const;
   friend ostream &operator<<(ostream &os, function_base &f);

   char *getMangledName(char *s, int len) const;

#if defined(arch_x86)
   //Replaces the function with a 'return val' statement.
   // currently needed only on Linux/x86
   bool setReturnValue(process *p, int val);
#endif
#if defined(ia64_unknown_linux2_4)
	// We need to know where all the alloc instructions in the
	// function are to do a reasonable job of register allocation
	// in the base tramp.  (I'd put this in pd_Function, but
	// pointFunc() returns a function_base...)
	pdvector< Address > allocs;
	
	// Since the IA-64 ABI does not define a frame pointer register,
	// we use DWARF debug records (DW_AT_frame_base entries) to 
	// construct an AST which calculates the frame pointer.  I
	// put this pointer here, rather than in the BPatch_function
	// that parseDwarf.C uses because instPoints store pd_Functions,
	// not BPatch_functions.
	AstNode * framePointerCalculator;

	// Place to store the results of doFloatingPointStaticAnalysis().
	// This way, if they are ever needed in a mini-tramp, emitFuncJump()
	// for example, the expensive operation doesn't need to happen again.
	bool * usedFPregs;
#endif

 protected:
   pdvector<pdstring> symTabName_;	/* name as it appears in the symbol table */
   pdvector<pdstring> prettyName_;	/* user's view of name (i.e. de-mangled) */
   int line_;			/* first line of function */
   Address addr_;		/* address of the start of the func */
   unsigned size_;             /* the function size, in bytes, used to
                                  define the function boundaries. This may not
                                  be exact, and may not be used on all 
                                  platforms. */
   image* img;
 
   pdvector< BPatch_basicBlock* >* blockList;
};


class instPoint;
class BPatch_basicBlockLoop;
class BPatch_flowGraph;
class BPatch_loopTreeNode;


class pd_Function : public function_base {
 public:
   pd_Function(const pdstring &symbol, pdmodule *f, 
               Address adr, const unsigned size);
	
   ~pd_Function() { 
      // delete the rewritten version of the function if it was relocated
      if (relocatedCode) delete relocatedCode;  
      if (originalCode) delete originalCode;   
      if (instructions) delete instructions;   
      /* TODO */ 
   }

   BPatch_flowGraph * getCFG(process * proc);

   void getOuterLoops(BPatch_Vector<BPatch_basicBlockLoop *> &loops, 
                      process * proc);

   BPatch_loopTreeNode * getLoopTree(process * proc);

   void printLoops(process * proc);

   void updateFunctionEnd( Address addr, image* owner );
   bool findInstPoints( const image* );
   bool findInstPoints( pdvector<Address >& , const image* );
   void canFuncBeRelocated( bool& );
   void canFuncBeInstrumented( bool b ) { isInstrumentable_ = b; };
   bool isInstrumentableByFunctionName();
   void checkCallPoints();
   bool defineInstPoint();
   pdmodule *file() const { return file_;}
   Address newCallPoint(
                        Address adr,
                        const instruction code, 
                        const image *owner,
                        bool &err);

   // passing in a value of 0 for p will return the original address
   // otherwise, if the process is relocated it will return the new address
   Address getAddress(const process *p){
      if(p && needs_relocation_) { 
         for(u_int i=0; i < relocatedByProcess.size(); i++){
            if((relocatedByProcess[i])->getProcess() == p) 
               return (relocatedByProcess[i])->get_address();
         } }
      return get_address();
   }
   Address getEffectiveAddress(const process *p) const;
   instPoint *funcEntry(const process *p) const {
      if(p && needs_relocation_) { 
         for(u_int i=0; i < relocatedByProcess.size(); i++){
            if((relocatedByProcess[i])->getProcess() == p) {
               return (relocatedByProcess[i])->funcEntry();
            }
         } }
      return funcEntry_;
   }
   const pdvector<instPoint*> &funcExits(const process *p) const {
      if(p && needs_relocation_) {
         for(u_int i=0; i < relocatedByProcess.size(); i++){
            if((relocatedByProcess[i])->getProcess() == p) 
               return (relocatedByProcess[i])->funcReturns();
         } }
      return funcReturns;
   }
   const pdvector<instPoint*> &funcArbitraryPoints(const process *p) const {
      if(p && needs_relocation_) {
         for(u_int i=0; i < relocatedByProcess.size(); i++){
            if((relocatedByProcess[i])->getProcess() == p) 
               return (relocatedByProcess[i])->funcArbitraryPoints();
         } }
      return arbitraryPoints;
   }
   void addArbitraryPoint(instPoint* insp, process* p) {
      if(insp) arbitraryPoints.push_back(insp);

      // Cheesy get-rid-of-compiler-warning
      process *unused = p;
      unused = 0;

#if defined(i386_unknown_nt4_0) || \
    defined(i386_unknown_linux2_0) || \
    defined(sparc_sun_solaris2_4)
    
      if(insp && p && needs_relocation_)
         for(u_int i=0; i < relocatedByProcess.size(); i++)
            if((relocatedByProcess[i])->getProcess() == p) {
               addArbitraryPoint(insp, p, relocatedByProcess[i]);
               return;
            }
#endif
   }

#if defined(i386_unknown_nt4_0) || \
    defined(i386_unknown_linux2_0) || \
    defined(sparc_sun_solaris2_4)

   void addArbitraryPoint(instPoint*, process*, relocatedFuncInfo*);
#endif

   const pdvector<instPoint*> &funcCalls(const process *p) {
      if (!call_points_have_been_checked) checkCallPoints();

      if(p && needs_relocation_) {
         for(u_int i=0; i < relocatedByProcess.size(); i++){
            if((relocatedByProcess[i])->getProcess() == p) 
               return (relocatedByProcess[i])->funcCallSites();
         } }
      return calls;
   }
   bool hasBeenRelocated(process *p){
      if(p && needs_relocation_) {
         for(u_int i=0; i < relocatedByProcess.size(); i++) {
            if((relocatedByProcess[i])->getProcess() == p) 
               return true;
         }
      }
      return false;
   }

        //see the note in process::addASharedObject (process.C) for an
        //explaination, this is part of the test4 exec on linux fix.
   void unrelocatedByProcess(process *p){ //ccw 20 apr 2004
         for(u_int i=0; i < relocatedByProcess.size(); i++) {
            if((relocatedByProcess[i])->getProcess() == p)
                (relocatedByProcess[i])->setProcess(NULL);
         }
   }

   bool hasNoStackFrame() const {return noStackFrame;}
   // formerly "isLeafFunc()" but that led to confusion, since people assign two
   // different meanings to "leaf" fns: (1) has no stack frame, (2) makes no calls.
   // By renaming this fn, we make clear that we're returning (1), not (2).
   bool makesNoCalls() const {return makesNoCalls_;}
   // A function can have no stack frame, but still save the frame pointer.
   //  i.e. on x86 you could have 'push %ebp', but no 'mov %esp,%ebp'
   bool savesFramePointer() const { return savesFP_; }

   bool isTrapFunc() {return isTrap;}
   bool mayNeedRelocation() {return mayNeedRelocation_;}
   bool needsRelocation() {return needs_relocation_;}
   void markAsNeedingRelocation(bool value) { needs_relocation_ = value; }
   bool canBeRelocated() { return canBeRelocated_; }

   bool isInstrumentable() { return isInstrumentable_; }
    
#ifndef BPATCH_LIBRARY
   // Fill in <callees> vector with pointers to all other pd functions
   //  statically determined to be called from any call sites in 
   //  this function.
   // Returns false if unable to fill in that information....
   bool getStaticCallees(process *proc, pdvector <pd_Function *> &callees);
#endif

   bool is_on_stack(process *proc,
                    const pdvector<pdvector<Frame> > &stackWalks);
   bool think_is_long_running(process *proc,
                              const pdvector<pdvector<Frame> > &stackWalks);

#if defined(sparc_sun_sunos4_1_3) || defined(sparc_sun_solaris2_4)   

   bool checkInstPoints(const image *owner);
   bool findInstPoints(const image *owner, Address adr, process *proc);

   // Record a call point in a function that will be, or currently is
   // being relocated.
   // (if location != 0 && reloc_info != 0, then this is called when 
   // the the function is actually being relocated)
   void addCallPoint(const instruction instr, unsigned &callId,
                     relocatedFuncInfo *reloc_info, instPoint *&point, 
                     const instPoint *&location);

   bool PA_attachTailCalls(LocalAlterationSet *temp_alteration_set);

   bool PA_attachBasicBlockEndRewrites(LocalAlterationSet *temp_alteration_set,
                                       Address baseAddress,
                                       Address firstAddress,
                                       process *proc);
   //
   // NEW routines for function code rewrites using peephole alterations....
   //
   bool readFunctionCode(const image *owner, instruction *into);
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
   pdvector<Address> sp_ret;     // offset of insn that restores $sp
   int             frame_size; // stack frame size ($sp frame only)
   // $fp-style frame (rare)
   Address         fp_mod;     // offset of insn that modifies $fp
   bool            uses_fp;    // does this fn use $s8 as a frame pointer?

   typedef struct inactiveRange {
      int popOffset;
      int retOffset;
   } InactiveFrameRange;

   pdvector<InactiveFrameRange> inactiveRanges;

#elif defined(i386_unknown_solaris2_5) || defined(i386_unknown_nt4_0) || defined(i386_unknown_linux2_0) || defined(ia64_unknown_linux2_4) /* Temporary duplication - TLM */

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

   void updateForFork(process *childProcess, const process *parentProcess);

#if defined(i386_unknown_solaris2_5) || defined(i386_unknown_nt4_0) || defined(i386_unknown_linux2_0) || defined(sparc_sun_solaris2_4) || defined(ia64_unknown_linux2_4) /* Temporary duplication - TLM. */

   bool isNearBranchInsn(const instruction insn);

   bool fillInRelocInstPoints(const image *owner, process *proc,
                              instPoint *&location, 
                              relocatedFuncInfo *reloc_info, Address mutatee,
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

   relocatedFuncInfo *findAndApplyAlterations(const image *owner, 
                                instPoint *&location,
                                u_int &newAdr,
                                process *proc,
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
                         unsigned numberOfInstructions,
                         process *proc);

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

   bool relocateFunction(process *proc, instPoint *&location);

   void sorted_ips_vector(pdvector<instPoint*>&fill_in);

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

   instPoint *find_overlap(pdvector<instPoint*> v, Address targetAddress);
#endif

   void setNumInstructions(unsigned num) { numInstructions = num; }
   unsigned getNumInstructions() { return numInstructions; }
 
   instruction *getInstructions() { return instructions; }

   // only needed on x86 ---------------------------------------------------
   bool hasJumpToFirstFiveBytes() {
      return has_jump_to_first_five_bytes;
   }
   // ----------------------------------------------------------------------

#ifndef BPATCH_LIBRARY
   void SetFuncResource(resource *r) {
      assert(r != NULL); 
      funcResource = r;
   }

   pdstring ResourceFullName() {
      assert(funcResource); 
      return funcResource->full_name();
   }

   bool FuncResourceSet() {
      return (funcResource != NULL);
   }
#endif

 private:
   BPatch_flowGraph * flowGraph;

   pdmodule *file_;		/* pointer to file that defines func. */
   instPoint *funcEntry_;	/* place to instrument entry (often not addr) */
   pdvector<instPoint*> funcReturns;	/* return point(s). */
   pdvector<instPoint*> calls;		/* pointer to the calls */
   pdvector<instPoint*> arbitraryPoints;		/* pointer to the calls */
#ifndef BPATCH_LIBRARY
   resource *funcResource;
#endif
    
   // these are for relocated functions
   bool needs_relocation_;		   // true if func will be relocated when instr
   bool mayNeedRelocation_;       // True if func needs to be relocated to
   // enable call sequence transfers to
   // base trampolines 
   bool canBeRelocated_;           // True if nothing prevents us from
   // relocating function

   unsigned char *relocatedCode;  // points to copy of rewritten function    
   unsigned char *originalCode;   // points to copy of original function

   unsigned numInstructions;      // num instructions in original func
   instruction *instructions;     // instructions that correspond to the 
   // original function 

   bool noStackFrame; // formerly "leaf".  True iff this fn has no stack frame.
   bool makesNoCalls_;
   bool savesFP_;

   bool isTrap; 		// true if function contains a trap instruct
   bool isInstrumentable_;     // true if the function is instrumentable
   pdvector<relocatedFuncInfo *> relocatedByProcess; // one element per process

   bool call_points_have_been_checked; // true if checkCallPoints has been called.

   bool has_jump_to_first_five_bytes;

   // the vector "calls" should not be accessed until this is true.
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
   module(supportedLanguages lang, Address adr, pdstring &fullNm,
          pdstring &fileNm): fileName_(fileNm), fullName_(fullNm), 
      language_(lang), addr_(adr){}
   virtual ~module(){}

   pdstring fileName() const { return fileName_; }
   pdstring fullName() const { return fullName_; }
   supportedLanguages language() const { return language_;}
   void setLanguage(supportedLanguages lang) {language_ = lang;}
   Address addr() const { return addr_; }

   virtual pdvector<function_base *> *
      findFunction (const pdstring &name,pdvector<function_base *> *found) = 0;
   // virtual pdvector<function_base *> *
   // findFunctionFromAll(const pdstring &name,
   //		  pdvector<function_base *> *found) = 0;
		
   virtual void define(process *proc) = 0;    // defines module to paradyn
   virtual pdvector<function_base *> *getFunctions() = 0;

 private:
   pdstring fileName_;                   // short file 
   pdstring fullName_;                   // full path to file 
   supportedLanguages language_;
   Address addr_;                      // starting address of module
};

class pdmodule: public module {
   friend class image;
 public:

   pdmodule(supportedLanguages lang, Address adr, pdstring &fullNm,
            pdstring &fileNm, image *e): module(lang,adr,fullNm,fileNm),
      lineInformation(NULL),

#ifndef BPATCH_LIBRARY
      modResource(0),
#endif
      exec_(e), 

      allInstrumentableFunctionsByPrettyName( pdstring::hash ),
      allInstrumentableFunctionsByMangledName( pdstring::hash ),
      allUninstrumentableFunctionsByPrettyName( pdstring::hash ),
      allUninstrumentableFunctionsByMangledName( pdstring::hash )  
      {
      }

   ~pdmodule();

   void setLineAddr(unsigned line, Address addr) {
      lines_.setLineAddr(line, addr);}
   bool getLineAddr(unsigned line, Address &addr) { 
      return (lines_.getLineAddr(line, addr)); }

   image *exec() const { return exec_; }
   void mapLines() { }           // line number info is not used now
#ifdef CHECK_ALL_CALL_POINTS
   // JAW -- checking all call points is expensive and may not be necessary
   //    --  if we can do this on-the-fly
   void checkAllCallPoints();
#endif
   void define(process *proc);    // defines module to paradyn

   void updateForFork(process *childProcess, const process *parentProcess);

   pdvector<function_base *> * getFunctions();
   const pdvector<pd_Function *> * getPD_Functions();

   pdvector<function_base *> *findFunction (const pdstring &name, 
                                            pdvector<function_base *> *found);
 
   pdvector<function_base *> *findFunctionFromAll(const pdstring &name, 
                                                  pdvector<function_base *> *found, 
                                                  bool regex_case_sensitive=true);

   function_base *findFunctionByMangled(const pdstring &name);
   pdvector<function_base *> *findUninstrumentableFunction(const pdstring &name,
                                                           pdvector<function_base *> *found);
   // remove record of function from internal vector of instrumentable funcs
   // (used when a function needs to be reclassified as non-instrumentable);
   bool removeInstruFunc(pd_Function *pdf);
   bool isShared() const;
#ifndef BPATCH_LIBRARY
   resource *getResource() { return modResource; }
#endif
   void dumpMangled(char * prefix);

   LineInformation* lineInformation;
   pdstring* processDirectories(pdstring* fn);

#if defined(rs6000_ibm_aix4_1)

   void parseLineInformation(process* proc, 
                             pdstring* currentSourceFile,
                             char* symbolName,
                             SYMENT *sym,
                             Address linesfdptr,char* lines,int nlines);
#endif

#if !defined(mips_sgi_irix6_4) && !defined(alpha_dec_osf4_0) && !defined(i386_unknown_nt4_0)
   void parseFileLineInfo(process *proc);
#endif

   LineInformation* getLineInformation(process *proc);
   void initLineInformation();
   void cleanupLineInformation();


 private:

#ifndef BPATCH_LIBRARY
   resource *modResource;
#endif
   image *exec_;                      // what executable it came from 
   lineDict lines_;
   //  list of all found functions in module....
   // pdvector<pd_Function*> funcs;
   // pdvector<pd_Function*> notInstruFuncs;

   //bool shared_;                      // if image it belongs to is shared lib

 public:

   void addInstrumentableFunction( pd_Function * function );
   void addUninstrumentableFunction( pd_Function * function );

 private:

   typedef dictionary_hash< pdstring, pd_Function * >::iterator FunctionsByMangledNameIterator;
   typedef dictionary_hash< pdstring, pdvector< pd_Function * > * >::iterator FunctionsByPrettyNameIterator;
   dictionary_hash< pdstring, pdvector< pd_Function * > * > allInstrumentableFunctionsByPrettyName;
   dictionary_hash< pdstring, pd_Function * > allInstrumentableFunctionsByMangledName;
   dictionary_hash< pdstring, pdvector< pd_Function * > * > allUninstrumentableFunctionsByPrettyName;
   dictionary_hash< pdstring, pd_Function * > allUninstrumentableFunctionsByMangledName;

};




void print_func_vector_by_pretty_name(pdstring prefix,
                                      pdvector<function_base *>*funcs);
void print_module_vector_by_short_name(pdstring prefix,
                                       pdvector<pdmodule*> *mods);
pdstring getModuleName(pdstring constraint);
pdstring getFunctionName(pdstring constraint);

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
   internalSym(const Address adr, const pdstring &nm) : name(nm), addr(adr) { }
   Address getAddr() const { return addr;}

 private:
   pdstring name;            /* name as it appears in the symbol table. */
   Address addr;      /* absolute address of the symbol */
};


int rawfuncscmp( pd_Function*& pdf1, pd_Function*& pdf2 );

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
class image : public codeRange {
   friend class process;
   friend class pd_Function;

   //
   // ****  PUBLIC MEMBER FUBCTIONS  ****
   //
 public:
   static image *parseImage(const pdstring file);
   static image *parseImage(fileDescriptor *desc, Address newBaseAddr = 0); 
   // And to get rid of them if we need to re-parse
   static void removeImage(image *img);
   static void removeImage(const pdstring file);
   static void removeImage(fileDescriptor *desc);
   void parseStaticCallTargets( pdvector< Address >& callTargets,
                                pdvector< pd_Function* > *raw_funcs,
                                pdmodule* mod );

   bool parseFunction( pd_Function* pdf, pdvector< Address >& callTargets,
                       pdmodule* mod );
   image(fileDescriptor *desc, bool &err, Address newBaseAddr = 0); 
 protected:
   ~image() { /* TODO */ }
 public:
   image *clone() { refCount++; return this; }
   int destroy() {
      if (!--refCount)
         delete this; 
      return refCount; 
   }

   // Check the list of symbols returned by the parser, return
   // name/addr pair
   bool findInternalSymbol(const pdstring &name, const bool warn, internalSym &iSym);

   // Check the list of symbols returned by the parser, return
   // all which start with the given prefix
   bool findInternalByPrefix(const pdstring &prefix, pdvector<Symbol> &found) const;

  
   //Address findInternalAddress (const pdstring &name, const bool warn, bool &err);
   void updateForFork(process *childProcess, const process *parentProcess);

   // find the named module  
   pdmodule *findModule(const pdstring &name);
   pdmodule *findModule(function_base *func);

   // Note to self later: find is a const operation, [] isn't, for
   // dictionary access. If we need to make the findFuncBy* methods
   // consts, we can move from [] to find()

   // Find the vector of functions associated with a (demangled) name
   pdvector <pd_Function *> *findFuncVectorByPretty(const pdstring &name);
   //pdvector <pd_Function *> *findFuncVectorByMangled(const pdstring &name);
   pdvector <pd_Function *> *findFuncVectorByNotInstru(const pdstring &name);

   // Find the vector of functions determined by a filter function
   pdvector <pd_Function *> *findFuncVectorByPretty(functionNameSieve_t bpsieve, 
                                                    void *user_data, 
                                                    pdvector<pd_Function *> *found);
   pdvector <pd_Function *> *findFuncVectorByMangled(functionNameSieve_t bpsieve, 
                                                     void *user_data, 
                                                     pdvector<pd_Function *> *found);

   // Find a function by mangled (original) name. Guaranteed unique
   pd_Function *findFuncByMangled(const pdstring &name);
   // Look for the function in the non instrumentable list
   pd_Function *findNonInstruFunc(const pdstring &name);

   // Looks for the name in all lists (inc. excluded and non-instrumentable)
   pd_Function *findOnlyOneFunctionFromAll(const pdstring &name);
   pd_Function *findOnlyOneFunction(const pdstring &name);

#if !defined(i386_unknown_nt4_0) && !defined(mips_unknown_ce2_11) // no regex for M$
   // REGEX search functions for Pretty and Mangled function names:
   // Callers can either provide a pre-compiled regex struct, or a
   // string pattern which will then be compiled.  This is set up
   // like this to provide a way for higher level functions to 
   // scan different images with the same compiled pattern -- thus
   // avoiding unnecessary re-compilation overhead.
   //
   // EXPENSIVE TO USE!!  Linearly searches dictionary hashes.  --jaw 01-03
   int findFuncVectorByPrettyRegex(pdvector<pd_Function *>*, pdstring pattern,
                                   bool case_sensitive = TRUE);
   int findFuncVectorByPrettyRegex(pdvector<pd_Function *>*, regex_t *);
   int findFuncVectorByMangledRegex(pdvector<pd_Function *>*, pdstring pattern,
                                    bool case_sensitive = TRUE);
   int findFuncVectorByMangledRegex(pdvector<pd_Function *>*, regex_t *);
#endif

   // Given an address (offset into the image), find the function that occupies
   // that address
   pd_Function *findFuncByOffset(const Address &offset) const;
   pd_Function *findFuncByEntry(const Address &entry) const;
  
   void findModByAddr (const Symbol &lookUp, pdvector<Symbol> &mods,
                       pdstring &modName, Address &modAddr, 
                       const pdstring &defName);

   // report modules to paradyn
   void defineModules(process *proc);
  
   //Add an extra pretty name to a known function (needed for handling
   //overloaded functions in paradyn)
   void addTypedPrettyName(pd_Function *func, const char *typedName);
	
   bool symbolExists(const pdstring &); /* Does the symbol exist in the image? */
   void postProcess(const pdstring);          /* Load .pif file */


   void addJumpTarget(Address addr) {
      if (!knownJumpTargets.defines(addr)) knownJumpTargets[addr] = addr; 
   }

   bool isJumpTarget(Address addr) { 
      return knownJumpTargets.defines(addr); 
   }


   // data member access

   pdstring file() const {return desc_->file();}
   pdstring name() const { return name_;}
   pdstring pathname() const { return pathname_; }
   const fileDescriptor *desc() const { return desc_; }
   Address codeOffset() const { return codeOffset_;}
   Address get_address() const { return codeOffset(); }
   Address dataOffset() const { return dataOffset_;}
   Address dataLength() const { return (dataLen_ << 2);} 
   Address codeLength() const { return (codeLen_ << 2);} 
   unsigned get_size() const { return codeLength(); }
   Address codeValidStart() const { return codeValidStart_; }
   Address codeValidEnd() const { return codeValidEnd_; }
   Address dataValidStart() const { return dataValidStart_; }
   Address dataValidEnd() const { return dataValidEnd_; }
   const Object &getObject() const { return linkedFile; }

   Object &getObjectNC() { return linkedFile; } //ccw 27 july 2000 : this is a TERRIBLE hack : 29 mar 2001

   bool isDyninstRTLib() const { return is_libdyninstRT; }

#if !defined(BPATCH_LIBRARY) //ccw 19 apr 2002 : SPLIT
   //  bool is_libparadynRT;
   bool isParadynRTLib() const { return is_libparadynRT; }
#endif

   bool isAOut() const { return is_a_out; }

   inline bool isCode(const Address &where) const;
   inline bool isData(const Address &where) const;
   inline bool isValidAddress(const Address &where) const;
   inline bool isAllocedCode(const Address &where) const;
   inline bool isAllocedData(const Address &where) const;
   inline bool isAllocedAddress(const Address &where) const;
   inline const Word get_instruction(Address adr) const;
   inline const unsigned char *getPtrToInstruction(Address adr) const;

   inline bool isNativeCompiler() const { return nativeCompiler; }

   // Return symbol table information
   inline bool symbol_info(const pdstring& symbol_name, Symbol& ret);


   const pdvector<pd_Function*> &getAllFunctions();
  
   // Tests if a symbol starts at a given point
   bool hasSymbolAtPoint(Address point) const;

#ifndef BPATCH_LIBRARY

   // get all modules, including excluded ones....
   const pdvector<pdmodule *> &getAllModules();

   // Called from the mdl -- lists of functions to look for
   static void watch_functions(pdstring& name, pdvector<pdstring> *vs, bool is_lib,
                               pdvector<pd_Function*> *updateDict);
#else

#endif 
   const pdvector<pdmodule*> &getModules();

   //
   //  ****  PUBLIC DATA MEMBERS  ****
   //

   Address get_main_call_addr() const { return main_call_addr_; }
 private:

   // Adds a function to the following lists
   // funcsByPretty
   // funcsByMangled
   // funcsByAddr
   // if (excluded) excludedFunctions
   // else includedFunctions
   void addInstruFunction(pd_Function *func, pdmodule *mod,
                          const Address addr);

   // Remove a function from the lists of instrumentable functions, once already inserted.
   int removeFuncFromInstrumentable(pd_Function *func);

   // Add a function which could not be instrumented.  Sticks it in
   // notInstruFuncs (list)
   void addNotInstruFunc(pd_Function *func, pdmodule *mod);

   pd_Function *makeOneFunction(pdvector<Symbol> &mods,
                                const Symbol &lookUp);

   // addMultipleFunctionNames is called after the argument pd_Function
   // has been found to have a conflicting address a function that has already
   // been found and inserted into the funcsByAddr map.  We assume that this
   // is a case of function name aliasing, so we merely take the names from the duplicate
   // function and add them as additional names for the one that was already found.
   void addMultipleFunctionNames(pd_Function *dup);
				

   //
   //  ****  PRIVATE MEMBERS FUNCTIONS  ****
   //

   // private methods for findind an excluded function by name or
   //  address....
   //bool find_excluded_function(const pdstring &name,
   //    pdvector<pd_Function*> &retList);
   //pd_Function *find_excluded_function(const Address &addr);

   // A helper routine for removeInstrumentableFunc -- removes function from specified hash
   void removeFuncFromNameHash(pd_Function *func, pdstring &fname,
                               dictionary_hash<pdstring, pdvector<pd_Function *> > *func_hash);

#ifdef CHECK_ALL_CALL_POINTS
   void checkAllCallPoints();
#endif

#if 0
   bool addInternalSymbol(const pdstring &str, const Address symValue);
#endif
   // creates the module if it does not exist
   pdmodule *getOrCreateModule (const pdstring &modName, const Address modAddr);
   pdmodule *newModule(const pdstring &name, const Address addr, supportedLanguages lang);

   bool addAllFunctions(pdvector<Symbol> &mods, pdvector<pd_Function *> *raw_funcs);

   bool addAllVariables();
   void getModuleLanguageInfo(dictionary_hash<pdstring, supportedLanguages> *mod_langs);
   void setModuleLanguages(dictionary_hash<pdstring, supportedLanguages> *mod_langs);

   bool buildFunctionMaps(pdvector<pd_Function *> *);
   //
   //  ****  PRIVATE DATA MEMBERS  ****
   //

   fileDescriptor *desc_; /* file descriptor (includes name) */
   pdstring name_;		 /* filename part of file, no slashes */
   pdstring pathname_;      /* file name with path */

   Address codeOffset_;
   unsigned codeLen_;
   Address dataOffset_;
   unsigned dataLen_;

   Address codeValidStart_;
   Address codeValidEnd_;
   Address dataValidStart_;
   Address dataValidEnd_;

   bool is_libdyninstRT;
#if !defined(BPATCH_LIBRARY) //ccw 19 apr 2002 : SPLIT
   bool is_libparadynRT;
#endif
   bool is_a_out;
   Address main_call_addr_; // address of call to main()

   bool nativeCompiler;

   // data from the symbol table 
   Object linkedFile;

   //dictionary_hash <pdstring, internalSym*> iSymsMap;   // internal RTinst symbols

   // A vector of all images. Used to avoid duplicating
   // an "image" that already exists.
   static pdvector<image*> allImages;

   // knownJumpTargets: the addresses in this image that are known to 
   // be targets of jumps. It is used to check points with multiple 
   // instructions.
   // This is a subset of the addresses that are actually targets of jumps.
   dictionary_hash<Address, Address> knownJumpTargets;

   pdvector<pdmodule *> _mods;
   pdvector<pd_Function*> instrumentableFunctions;

   // The dictionary of all symbol addresses in the image. We use it as a hack
   // on x86 to scavenge some bytes past a function exit for the exit-point
   // instrumentation
   dictionary_hash<Address, unsigned> knownSymAddrs;
   //
   // Hash Tables of Functions....
   //

   // functions by address for all modules.  Only contains instrumentable
   //  funtions.
   codeRangeTree funcsByRange;
   // Keep this one as well for O(1) entry lookups
   dictionary_hash <Address, pd_Function *> funcsByEntryAddr;
   // note, a prettyName is not unique, it may map to a function appearing
   // in several modules.  Also only contains instrumentable functions....
   dictionary_hash <pdstring, pdvector<pd_Function*>*> funcsByPretty;
   // Hash table holding functions by mangled name.
   // Should contain same functions as funcsByPretty....
   dictionary_hash <pdstring, pdvector<pd_Function*>*> funcsByMangled;
   // The functions that can't be instrumented
   // Note that notInstruFunctions holds list of functions for which
   //  necessary instrumentation data could NOT be found....
   dictionary_hash <pdstring, pd_Function*> notInstruFunctions;

   // TODO -- get rid of one of these
   // Note : as of 971001 (mcheyney), these hash tables only 
   //  hold entries in includedMods --> this implies that
   //  it may sometimes be necessary to do a linear sort
   //  through excludedMods if searching for a module which
   //  was excluded....
   dictionary_hash <pdstring, pdmodule *> modsByFileName;
   dictionary_hash <pdstring, pdmodule*> modsByFullName;
   // Variables indexed by pretty (non-mangled) name
   dictionary_hash <pdstring, pdvector<pdstring>*> varsByPretty;
 
   int refCount;
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
   libraryFunc(const pdstring n, unsigned t) : name(n), tags(t) { }
   unsigned getTags() const { return tags;}

 private:
   pdstring name;
   unsigned tags;
};

/**
 * Used to represent something like a C++ try/catch block.  
 * Currently only used on Linux/x86
 **/
class ExceptionBlock {
 public:
   ExceptionBlock(Address tStart, unsigned tSize, Address cStart) :
     tryStart_(tStart), trySize_(tSize), catchStart_(cStart), hasTry_(true) {}
   ExceptionBlock(Address cStart) :
      tryStart_(0), trySize_(0), catchStart_(cStart), hasTry_(false) {}
   ExceptionBlock(const ExceptionBlock &eb) : tryStart_(eb.tryStart_), 
      trySize_(eb.trySize_), catchStart_(eb.catchStart_), hasTry_(eb.hasTry_) {}
   ExceptionBlock() : tryStart_(0), trySize_(0), catchStart_(0), hasTry_(false) {}
   ~ExceptionBlock() {}

   bool hasTry() const { return hasTry_; }
   Address tryStart() const { return tryStart_; }
   Address tryEnd() const { return tryStart_ + trySize_; }
   Address trySize() const { return trySize_; }
   Address catchStart() const { return catchStart_; }
   Address contains(Address a) const 
      { return (a >= tryStart_ && a < tryStart_ + trySize_); }

 private:
   Address tryStart_;
   unsigned trySize_;
   Address catchStart_;
   bool hasTry_;
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

inline bool image::isAllocedAddress(const Address &where) const{
   return (isAligned(where) && (isAllocedCode(where) || isAllocedData(where)));
}

inline bool image::isCode(const Address &where)  const{
   return (linkedFile.code_ptr() && 
           (where >= codeOffset_) && (where < (codeOffset_+(codeLen_<<2))));
}

inline bool image::isData(const Address &where)  const{
   return (linkedFile.data_ptr() && 
           (where >= dataOffset_) && (where < (dataOffset_+(dataLen_<<2))));
}

inline bool image::isAllocedCode(const Address &where)  const{
   return (linkedFile.code_ptr() && 
           (where >= codeValidStart_) && (where < codeValidEnd_));
}

inline bool image::isAllocedData(const Address &where)  const{
   return (linkedFile.data_ptr() && 
           (where >= dataValidStart_) && (where < dataValidEnd_));
}

inline bool image::symbol_info(const pdstring& symbol_name, Symbol &ret_sym) {

   if (linkedFile.get_symbol(symbol_name, ret_sym))
      return true;

   if (varsByPretty.defines(symbol_name)) {
      pdvector<pdstring> *mangledNames = varsByPretty[symbol_name];
      assert(mangledNames && mangledNames->size() == 1);
      if (linkedFile.get_symbol((*mangledNames)[0], ret_sym))
         return true;
   }

   return false;
}


typedef pd_Function *pdFuncPtr;

struct pdFuncCmp
{
    int operator()( const pdFuncPtr &pdf1, const pdFuncPtr &pdf2 ) const
    {
        if( pdf1->get_address() > pdf2->get_address() )
            return 1;
        if( pdf1->get_address() < pdf2->get_address() )
            return -1;
        return 0;
    }
};

int instPointCompare( instPoint*& ip1, instPoint*& ip2 );
int basicBlockCompare( BPatch_basicBlock*& bb1, BPatch_basicBlock*& bb2 );

#endif

