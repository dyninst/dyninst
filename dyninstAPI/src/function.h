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
 
// $Id: function.h,v 1.8 2005/04/18 20:55:36 legendre Exp $

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

class process;

class BPatch_flowGraph;
class BPatch_loopTreeNode;
class BPatch_basicBlock;
class BPatch_basicBlockLoop;

class instPoint;

class relocatedFuncInfo;
class LocalAlteration;
class LocalAlterationSet;

class Frame;

class int_function : public codeRange {
 public:
   static pdstring emptyString;

   // Almost everything gets filled in later.
   int_function(const pdstring &symbol, 
		Address offset, 
		const unsigned size, 
		pdmodule *m);


   ~int_function();

   ////////////////////////////////////////////////
   // Basic output functions
   ////////////////////////////////////////////////

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

   Address getOffset() const {return offset_;}
   
   // coderange needs a get_address...
   Address get_address() const { return getOffset();}

   // Should be operator==
   bool match(int_function *p);

   // extra debuggering info....
   ostream & operator<<(ostream &s) const;
   friend ostream &operator<<(ostream &os, int_function &f);
   pdmodule *pdmod() const { return mod_;}
   void changeModule(pdmodule *mod);

   ////////////////////////////////////////////////
   // CFG and other function body methods
   ////////////////////////////////////////////////

   pdvector< BPatch_basicBlock* >* blocks() const{ return blockList; }

   bool hasNoStackFrame() const {return noStackFrame;}
   bool makesNoCalls() const {return makesNoCalls_;}
   bool savesFramePointer() const {return savesFP_;}

   BPatch_flowGraph * getCFG(process * proc);
   BPatch_loopTreeNode * getLoopTree(process * proc);

   ////////////////////////////////////////////////
   // Instpoints!
   ////////////////////////////////////////////////

   const instPoint *funcEntry();
   const pdvector<instPoint*> &funcExits();
   const pdvector<instPoint*> &funcCalls();
   const pdvector<instPoint*> &funcArbitraryPoints();
   
   // Defined in inst-<arch>.C
   bool findInstPoints( const image* );
   bool findInstPoints( pdvector<Address >& , const image* );
   void checkCallPoints();
   Address newCallPoint(Address adr, const instruction code, 
			const image *owner, bool &emarr);
   
   void canFuncBeInstrumented( bool b ) { isInstrumentable_ = b; };

   bool isTrapFunc() {return isTrap;}
   bool isInstrumentable() { return isInstrumentable_; }


#if defined(arch_x86) || defined(arch_x86_64)
   //Replaces the function with a 'return val' statement.
   // currently needed only on Linux/x86
   // Defined in inst-x86.C
   bool setReturnValue(process *p, int val);
   // Update if symtab is incorrect
   void updateFunctionEnd( Address addr, image* owner );  
   
   bool hasJumpToFirstFiveBytes() {
     return has_jump_to_first_five_bytes;
   }
   // ----------------------------------------------------------------------
#endif

   ////////////////////////////////////////////////
   // Relocation
   ////////////////////////////////////////////////

#if defined(cap_relocation)
   bool mayNeedRelocation() {return mayNeedRelocation_;}
   bool needsRelocation() {return needs_relocation_;}
   void markAsNeedingRelocation(bool value) { needs_relocation_ = value; }
   bool canBeRelocated() { return canBeRelocated_; }
#endif

#if defined(arch_sparc)
   // Record a call point in a function that will be, or currently is
   // being relocated.
   // (if location != 0 && reloc_info != 0, then this is called when 
   // the the function is actually being relocated)
   void addCallPoint(const instruction instr, unsigned &callId,
                     relocatedFuncInfo *reloc_info, instPoint *&point, 
                     const instPoint *&location);
#endif

#if defined(cap_relocation)

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

   int calculateAlterations(const image *owner, process *proc, 
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

   bool PA_expandLoopBlocks(LocalAlterationSet *temp_alteration_set,
			    process *proc);

   bool discoverAlterations(LocalAlterationSet *temp_alteration_set, 
                            LocalAlterationSet &normalized_alteration_set,
                            Address baseAddress, Address firstAddress, 
                            instruction originalCode[], int originalCodeSize); 

   bool applyAlterations(LocalAlterationSet &normalized_alteration_set,
                         Address mutator, Address mutatee, Address newAdr, 
                         instruction originalCode[], 
                         unsigned originalCodeSize, instruction newCode[],
			 relocatedFuncInfo *reloc);

   bool updateAlterations(LocalAlterationSet *temp_alteration_set,
                          LocalAlterationSet &normalized_alteration_set,
                          instruction *oldCode, 
                          Address baseAddress, Address firstAddress,
                          int &totalSizeChange);

   void markNeededLoopRelocations(BPatch_basicBlockLoop *loop);

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

   void addArbitraryPoint(instPoint*, process*, relocatedFuncInfo*);

#endif /*cap_relocation*/

   void addArbitraryPoint(instPoint*, process*);

   /* Redefining to handle relocation */
   Address getAddress(const process *p) const;
   Address getEffectiveAddress(const process *p) const;
   Address getSize(const process *p) const;
   instPoint *funcEntry(const process *p) const;
   const pdvector<instPoint *> &funcExits(const process *p) const;
   const pdvector<instPoint *> &funcArbitraryPoints(const process *p) const;
   const pdvector<instPoint *> &funcCalls(const process *p);
   bool hasBeenRelocated(const process *p) const;
   const relocatedFuncInfo *getRelocRecord(const process *p) const;
   Address mapOrigToRelocOffset(Address origOffset, const process *p) const;

   void setNumInstructions(unsigned num) { numInstructions = num; }
   unsigned getNumInstructions() { return numInstructions; }
 
   instruction *getInstructions() { return instructions; }

   ////////////////////////////////////////////////
   // Misc
   ////////////////////////////////////////////////

   void cleanProcessSpecific(process *p);

#ifndef BPATCH_LIBRARY
   // Fill in <callees> vector with pointers to all other pd functions
   //  statically determined to be called from any call sites in 
   //  this function.
   // Returns false if unable to fill in that information....
   bool getStaticCallees(process *proc, pdvector <int_function *> &callees);
#endif

   codeRange *copy() const;
    
   bool is_on_stack(process *proc,
                    const pdvector<pdvector<Frame> > &stackWalks);
   bool think_is_long_running(process *proc,
                              const pdvector<pdvector<Frame> > &stackWalks);

#if defined(arch_sparc)

   bool checkInstPoints(const image *owner);
   //bool findInstPoints(const image *owner, Address adr, process *proc);

   bool PA_attachTailCalls(LocalAlterationSet *temp_alteration_set);

   bool PA_attachBasicBlockEndRewrites(LocalAlterationSet *temp_alteration_set,
                                       Address baseAddress,
                                       Address firstAddress,
                                       process *proc);
   //
   // NEW routines for function code rewrites using peephole alterations....
   //
   bool readFunctionCode(const image *owner, instruction *into);
#elif defined(arch_mips)
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

#elif defined(arch_ia64) || defined(arch_x86) || defined(arch_x86_64)

   void instrAroundPt(instPoint *p, instruction allInstr[], int numBefore, 
                      int numAfter, unsigned type, int index);

   int getArrayOffset(Address adr, instruction code[]);

   bool isTrueCallInsn(const instruction insn);
 
   bool canUseExtraSlot(instPoint *&ip) const;

   bool usesTrap(instPoint *&ip);

#elif defined(arch_alpha)
   int             frame_size; // stack frame size
#endif

#if defined(sparc_sun_solaris2_4)
   bool is_o7_live(){ return o7_live; }
#endif

   void updateForFork(process *childProcess, const process *parentProcess);

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

#if defined(arch_ia64)
   // We need to know where all the alloc instructions in the
   // function are to do a reasonable job of register allocation
   // in the base tramp.  
   pdvector< Address > allocs;
   
   // Since the IA-64 ABI does not define a frame pointer register,
   // we use DWARF debug records (DW_AT_frame_base entries) to 
   // construct an AST which calculates the frame pointer.
   AstNode * framePointerCalculator;
   
   // Place to store the results of doFloatingPointStaticAnalysis().
   // This way, if they are ever needed in a mini-tramp, emitFuncJump()
   // for example, the expensive operation doesn't need to happen again.
   bool * usedFPregs;
#endif

 private:

   ///////////////////// Basic func info
   pdvector<pdstring> symTabName_;	/* name as it appears in the symbol table */
   pdvector<pdstring> prettyName_;	/* user's view of name (i.e. de-mangled) */
   int line_;			/* first line of function */
   Address offset_;		/* address of the start of the func */
   unsigned size_;             /* the function size, in bytes, used to
                                  define the function boundaries. This may not
                                  be exact, and may not be used on all 
                                  platforms. */
   pdmodule *mod_;		/* pointer to file that defines func. */
   bool parsed_;                /* Set to true in findInstPoints */


   ///////////////////// CFG and function body
   pdvector< BPatch_basicBlock* >* blockList;
   BPatch_flowGraph *flowGraph;

   bool noStackFrame; // formerly "leaf".  True iff this fn has no stack frame.
   bool makesNoCalls_;
   bool savesFP_;
   bool call_points_have_been_checked; // true if checkCallPoints has been called.

   
   ///////////////////// Instpoints 

   instPoint *funcEntry_;	/* place to instrument entry (often not addr) */
   pdvector<instPoint*> funcReturns;	/* return point(s). */
   pdvector<instPoint*> calls;		/* pointer to the calls */
   pdvector<instPoint*> arbitraryPoints;		/* pointer to the calls */
   
   bool isTrap; 		// true if function contains a trap instruct
   bool isInstrumentable_;     // true if the function is instrumentable
   
   bool has_jump_to_first_five_bytes;
   


   // these are for relocated functions
   bool needs_relocation_;	   // true if func will be relocated when instr
   bool mayNeedRelocation_;       // True if func needs to be relocated to
   // enable call sequence transfers to
   // base trampolines 
   bool canBeRelocated_;           // True if nothing prevents us from
   // relocating function

   pdvector<BPatch_basicBlock *> blocksNeedingReloc;

   // FIXME: this is process-specific!
   // Works for now, since we assume 1) relocation happens only once per
   // function and 2) relocation between processes will be identical.
   // b0rked, but hey.
   unsigned char *relocatedCode;  // points to copy of rewritten function    
   unsigned char *originalCode;   // points to copy of original function

   pdvector<relocatedFuncInfo *> relocatedByProcess; // one element per process

   unsigned numInstructions;      // num instructions in original func
   instruction *instructions;     // instructions that correspond to the 
                                  // original function 
   instruction *relocatedInstructions;


   // Hacky way around parsing things -- we can stick things known to be 
   // unparseable in here.
   bool isInstrumentableByFunctionName();

   // Misc

#ifndef BPATCH_LIBRARY
   resource *funcResource;
#endif
   

   // the vector "calls" should not be accessed until this is true.
   bool o7_live;

};

typedef int_function *funcPtr;

struct funcCmp
{
    int operator()( const funcPtr &f1, const funcPtr &f2 ) const
    {
        if( f1->get_address() > f2->get_address() )
            return 1;
        if( f1->get_address() < f2->get_address() )
            return -1;
        return 0;
    }
};

#endif /* FUNCTION_H */
