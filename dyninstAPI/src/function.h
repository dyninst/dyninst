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
 
// $Id: function.h,v 1.1 2005/01/19 17:41:05 bernat Exp $

#ifndef FUNCTION_H
#define FUNCTION_H

#include "common/h/String.h"
#include "common/h/Vector.h"
#include "common/h/Types.h"
#include "common/h/Pair.h"
#include "codeRange.h"
#include "arch.h" // instruction

#if !defined(BPATCH_LIBRARY)
#include "paradynd/src/resource.h"
#endif

class pdmodule;
class BPatch_flowGraph;
class BPatch_loopTreeNode;
class BPatch_basicBlock;
class process;
class instPoint;
class image;


class function_base : public codeRange {
 public:
   static pdstring emptyString;

   function_base(const pdstring &symbol, 
                 Address adr, 
                 const unsigned size, 
                 pdmodule *f) :
#if defined( ia64_unknown_linux2_4 )
       /* This appears to be the only constructor, so this should be
          sufficient to make sure we don't think we have an AST when
          we don't. */
       framePointerCalculator( NULL ), usedFPregs( NULL ),
#endif
       line_(0), addr_(adr), size_(size), file_(f), flowGraph(NULL) {
       
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
   virtual codeRange *copy() const = 0;
   pdvector< BPatch_basicBlock* >* blocks() const{ return blockList; }
   bool match(function_base *p);

   virtual Address getAddress(const process *p) const = 0;
   virtual Address getEffectiveAddress(const process *p) const = 0;
   virtual instPoint *funcEntry(const process *p) const = 0;
   virtual const pdvector<instPoint*> &funcExits(const process *p) const = 0;
   virtual const pdvector<instPoint*> &funcCalls(const process *p)= 0; 
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
   // Defined in inst-x86.C
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

   pdmodule *file() const { return file_;}
   BPatch_flowGraph * getCFG(process * proc);
   BPatch_loopTreeNode * getLoopTree(process * proc);

 protected:
   pdvector<pdstring> symTabName_;	/* name as it appears in the symbol table */
   pdvector<pdstring> prettyName_;	/* user's view of name (i.e. de-mangled) */
   int line_;			/* first line of function */
   Address addr_;		/* address of the start of the func */
   unsigned size_;             /* the function size, in bytes, used to
                                  define the function boundaries. This may not
                                  be exact, and may not be used on all 
                                  platforms. */
   pdmodule *file_;		/* pointer to file that defines func. */

   image* img;
 
   pdvector< BPatch_basicBlock* >* blockList;

   BPatch_flowGraph *flowGraph;

};


class process;
class relocatedFuncInfo;
class LocalAlteration;
class LocalAlterationSet;
class Frame;

class pd_Function : public function_base {
 public:
   pd_Function(const pdstring &symbol, pdmodule *f, 
               Address adr, const unsigned size);
	
   ~pd_Function();

   void cleanProcessSpecific(process *p);

   codeRange *copy() const;

#if defined(arch_x86)
   // Defined in inst-x86.C
   void updateFunctionEnd( Address addr, image* owner );
   void canFuncBeRelocated( bool& );
   bool isInstrumentableByFunctionName();
#endif
   // Defined in inst-<arch>.C
   bool findInstPoints( const image* );
   bool findInstPoints( pdvector<Address >& , const image* );
   void checkCallPoints();
   Address newCallPoint(Address adr, const instruction code, 
                        const image *owner, bool &err);

   void canFuncBeInstrumented( bool b ) { isInstrumentable_ = b; };

   // Undefined? bool defineInstPoint();

   void addArbitraryPoint(instPoint* insp, process* p);

#if defined(i386_unknown_nt4_0) || \
    defined(i386_unknown_linux2_0) || \
    defined(sparc_sun_solaris2_4)
   void addArbitraryPoint(instPoint*, process*, relocatedFuncInfo*);
#endif

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

   Address getAddress(const process *p) const;
   Address getEffectiveAddress(const process *p) const;
   instPoint *funcEntry(const process *p) const;
   const pdvector<instPoint *> &funcExits(const process *p) const;
   const pdvector<instPoint *> &funcArbitraryPoints(const process *p) const;
   const pdvector<instPoint *> &funcCalls(const process *p);
   bool hasBeenRelocated(const process *p) const;
 private:

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

#endif /* FUNCTION_H */
