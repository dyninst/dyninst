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
 
// $Id: function.h,v 1.10 2005/07/29 19:18:32 bernat Exp $

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

#include "image-func.h"

class process;
class mapped_module;
class mapped_object;

class BPatch_flowGraph;
class BPatch_loopTreeNode;
class BPatch_basicBlock;
class BPatch_basicBlockLoop;

#include "dyninstAPI/h/BPatch_Set.h"

class instPoint;

class Frame;

class int_function;

class relocShift : public codeRange {
    friend class int_function;
 public:
    unsigned get_shift() const { return shift;}
    Address get_address_cr() const { return offset;}
    unsigned get_size_cr() const { return size;}
 private:
    Address offset;
    unsigned size;
    unsigned shift;
};

class int_basicBlock : public codeRange {
    friend class int_function;
 public:
    int_basicBlock(const image_basicBlock *ib, Address baseAddr, int_function *func);

    Address firstInsnAddr() const { return firstInsnAddr_; }
    Address lastInsnAddr() const { return lastInsnAddr_; }
    // Just to be obvious -- this is the end addr of the block
    Address endAddr() const { return blockEndAddr_; }
    Address getSize() const { return blockEndAddr_ - firstInsnAddr_; }
    
    bool isEntryBlock() const { return isEntryBlock_; }
    bool isExitBlock() const { return isExitBlock_; }

    static int compare(int_basicBlock *&b1,
                       int_basicBlock *&b2) {
        if (b1->firstInsnAddr() < b2->firstInsnAddr())
            return -1;
        if (b2->firstInsnAddr() < b1->firstInsnAddr())
            return 1;
        assert(b1 == b2);
        return 0;
    }

    void debugPrint();

    void getSources(pdvector<int_basicBlock *> &ins) const;
    void getTargets(pdvector<int_basicBlock *> &outs) const;

    int_basicBlock *getFallthrough() const;

    Address get_address_cr() const { return firstInsnAddr(); }
    unsigned get_size_cr() const { return getSize(); }

    void addTarget(int_basicBlock *target);
    void addSource(int_basicBlock *source);

    void removeTarget(int_basicBlock * target);
    void removeSource(int_basicBlock * source);

    int id() const { return blockNumber_; }

    int_function *func() const { return func_; }
    process *proc() const;

    // Data flow... for register analysis. Right now just used for 
    // IA64 alloc calculations
    // We need a set...
    void setDataFlowIn(BPatch_Set<int_basicBlock *> *in);
    void setDataFlowOut(BPatch_Set<int_basicBlock *> *out);
    void setDataFlowGen(int_basicBlock *gen);
    void setDataFlowKill(int_basicBlock *kill);

    BPatch_Set<int_basicBlock *> *getDataFlowOut();
    BPatch_Set<int_basicBlock *> *getDataFlowIn();
    int_basicBlock *getDataFlowGen();
    int_basicBlock *getDataFlowKill();

 private:
    Address firstInsnAddr_;
    Address lastInsnAddr_;
    Address blockEndAddr_;

    bool isEntryBlock_;
    bool isExitBlock_;

    int blockNumber_;

    pdvector<int_basicBlock *> targets_;
    pdvector<int_basicBlock *> sources_;

    BPatch_Set<int_basicBlock *> *dataFlowIn;
    BPatch_Set<int_basicBlock *> *dataFlowOut;
    int_basicBlock *dataFlowGen;
    int_basicBlock *dataFlowKill;


    int_function *func_;
    const image_basicBlock *ib_;
};

class int_function : public codeRange {
  friend class funcIterator;
 public:
   static pdstring emptyString;

   // Almost everything gets filled in later.
   int_function(image_func *f,
		Address baseAddr,
                mapped_module *mod);

   int_function(const int_function *parent,
                mapped_module *child_mod);

   ~int_function();

   ////////////////////////////////////////////////
   // Passthrough functions.
   ////////////////////////////////////////////////
   // To minimize wasted memory (since there will be many copies of
   // this function) we make most methods passthroughs to the original
   // parsed version.

   const pdstring &symTabName() const;
   const pdstring &prettyName() const { return ifunc_->prettyName(); };
   const pdvector<pdstring> &symTabNameVector() { return ifunc_->symTabNameVector(); }
   const pdvector<pdstring> &prettyNameVector() { return ifunc_->prettyNameVector(); }

   // May change when we relocate...
   unsigned getSize();
   Address getAddress() const {return addr_;}

   // And we always want an original This will always return the
   // original address of the function, no matter how many times it
   // has been relocated.
   Address getOriginalAddress() const;
   
   int_function *getOriginalFunc() const;

   // Not defined here so we don't have to play header file magic
   image_func *ifunc() const;
   mapped_module *mod() const;
   mapped_object *obj() const;
   process *proc() const;

   // coderange needs a get_address...
   Address get_address_cr() const { return getAddress(); }
   unsigned get_size_cr() const { return size_; }

   // Necessary for BPatch_set which needs a structure with a ()
   // operator. Odd.
   struct cmpAddr {
     int operator() (const int_function *f1, const int_function *f2) const {
       if (f1->getAddress() > f2->getAddress())
	 return 1;
       else if (f1->getAddress() < f2->getAddress())
	 return -1;
       else
	 return 0;
     }
   };

   // Should be operator==
   bool match(int_function *p) const;
   bool match(image_func *f) const { return (f == ifunc_); }

   // extra debuggering info....
   ostream & operator<<(ostream &s) const;
   friend ostream &operator<<(ostream &os, int_function &f);

   ////////////////////////////////////////////////
   // Process-dependent (inter-module) parsing
   ////////////////////////////////////////////////
   void checkCallPoints();

   ////////////////////////////////////////////////
   // CFG and other function body methods
   ////////////////////////////////////////////////

   const pdvector< int_basicBlock* > &blocks();

   // Perform a lookup (either linear or log(n)).
   int_basicBlock *findBlockByAddr(Address addr);
   int_basicBlock *findBlockByOffset(Address offset) { return findBlockByAddr(offset + getAddress()); }

   bool hasNoStackFrame() const {return ifunc_->hasNoStackFrame();}
   bool makesNoCalls() const {return ifunc_->makesNoCalls();}
   bool savesFramePointer() const {return ifunc_->savesFramePointer();}

   //BPatch_flowGraph * getCFG();
   //BPatch_loopTreeNode * getLoopTree();

   ////////////////////////////////////////////////
   // Instpoints!
   ////////////////////////////////////////////////

   void addArbitraryPoint(instPoint *insp);

   const pdvector<instPoint*> &funcEntries();
   // Note: the vector is constant, the instPoints aren't.
   const pdvector<instPoint*> &funcExits();
   const pdvector<instPoint*> &funcCalls();
   const pdvector<instPoint*> &funcArbitraryPoints();
   
   bool isInstrumentable() const { return ifunc_->isInstrumentable(); }


#if defined(arch_x86) || defined(arch_x86_64)
   //Replaces the function with a 'return val' statement.
   // currently needed only on Linux/x86
   // Defined in inst-x86.C
   bool setReturnValue(int val);

   //bool hasJumpToFirstFiveBytes() { return ifunc_->hasJumpToFirstFiveBytes(); }
   // ----------------------------------------------------------------------
#endif

   ////////////////////////////////////////////////
   // Relocation
   ////////////////////////////////////////////////

   bool needsRelocation() const {return needs_relocation_;}
   void markAsNeedingRelocation(bool value) { needs_relocation_ = value; }
   bool canBeRelocated() const { return canBeRelocated_; }
   // Given an address in a different version, find the equivalent address here.
   Address equivAddr(int_function *other_func, Address addrInOther) const;
   
   Address offsetInOriginal(Address offsetInSelf) const;
   Address offsetInSelf(Address offsetInOriginal) const;
   // If we've relocated since the last time someone asked
   unsigned version() const { return version_; }

   void *getPtrToOrigInstruction(Address addr) const;

#if defined(cap_relocation0)
   bool fillInRelocInstPoints(int_function *original_func);

   int relocateInstructionWithFunction(bool setDisp, 
                                       instruction *insn, 
                                       Address origAddr, 
                                       Address targetAddr, 
                                       Address oldFunctionAddr, 
                                       unsigned originalCodeSize);

   int patchOffset(bool setDisp, LocalAlterationSet *alteration_set, 
                   instruction& insn, Address adr, 
                   Address firstAddress, unsigned originalCodeSize);

   relocatedFuncInfo *findAndApplyAlterations(instPoint *&location,
					      u_int &newAdr,
					      unsigned &size_change);

   int calculateAlterations(instruction *&oldCode,
			    LocalAlterationSet &normalized_alteration_set,
			    Address &mutator, Address &mutatee);
   
   int relocatedSizeChange();

   // Manually load and parse the code stream for the function.
   // Should probably be eliminated; performs straight-through parsing
   // that is really pretty ugly.
   bool loadCode();

   bool expandInstPoints(LocalAlterationSet *temp_alteration_set, 
                         LocalAlterationSet &normalized_alteration_set, 
                         Address baseAddress, Address mutator, 
                         Address mutatee, instruction oldCode[], 
                         unsigned numberOfInstructions);

   bool PA_attachGeneralRewrites(LocalAlterationSet *temp_alteration_set, 
                                 Address baseAddress, Address firstAddress,
                                 instruction loadedCode[], 
                                 unsigned numInstructions, int codeSize);


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

   bool relocateFunction(instPoint *&location);

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

#endif /*cap_relocation*/

   bool hasBeenRelocated() const { return (relocatedFuncs_.size() != 0); }

   ////////////////////////////////////////////////
   // Misc
   ////////////////////////////////////////////////

   unsigned getNumDynamicCalls();

#ifndef BPATCH_LIBRARY
   // Fill in <callees> vector with pointers to all other pd functions
   //  statically determined to be called from any call sites in 
   //  this function.
   // Returns false if unable to fill in that information....
   bool getStaticCallees(pdvector <int_function *> &callees);
#endif

   codeRange *copy() const;
    
#if 0

   bool PA_attachTailCalls(LocalAlterationSet *temp_alteration_set);

   bool PA_attachBasicBlockEndRewrites(LocalAlterationSet *temp_alteration_set,
                                       Address baseAddress,
                                       Address firstAddress);
   //
   // NEW routines for function code rewrites using peephole alterations....
   //
   bool readFunctionCode(const image *owner, instruction *into);

#elif defined(arch_alpha)
   int frame_size() const { return ifunc_->frame_size; };

#endif

#if defined(sparc_sun_solaris2_4)
   bool is_o7_live(){ return ifunc_->is_o7_live(); }
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
   Address addr_; // Absolute address
   unsigned size_;             /* the function size, in bytes, used to
                                  define the function boundaries. This may not
                                  be exact, and may not be used on all 
                                  platforms. */
   image_func *ifunc_;
   mapped_module *mod_; // This is really a dodge; translate a list of
			// image_funcs to int_funcs

   ///////////////////// CFG and function body
   pdvector< int_basicBlock* > blockList;
   BPatch_flowGraph *flowGraph;

   ///////////////////// Instpoints 

   pdvector<instPoint*> entryPoints_;	/* place to instrument entry (often not addr) */
   pdvector<instPoint*> exitPoints_;	/* return point(s). */
   pdvector<instPoint*> callPoints_;	/* pointer to the calls */
   pdvector<instPoint*> arbitraryPoints_;	       /* pointer to the calls */

   pdvector<int_function *>relocatedFuncs_; // Pointer to relocated
                                            // versions (which may
                                            // have been
                                            // since relocated)
   // Does not include shift
   codeRangeTree relocOffsetsFwd_; // When we relocate, instructions
                               // move. This assumes that they stay at an
                               // offset from the original (not, say,
                               // reorganized completely).
                               // Offsets are from the previous version.
   // Includes shift
   codeRangeTree relocOffsetsBkwd_; // We need to be able to calculate
                                // both directions -- given an addr in
                                // me, find the equivalent in the
                                // original; and given an addr in the
                                // original, find the equiv in
                                // me. AFAIK, we need two trees to do
                                // this, one which includes the shift
                                // (so I can look up an addr in "me")
                                // and one that does _not_ (so I can
                                // look up an addr in the original).

   int_function *previousFunc_;  // And back pointer
   // these are for relocated functions
   bool needs_relocation_;	   // true if func will be relocated when instr

   bool canBeRelocated_;           // True if nothing prevents us from
   // relocating function

   void *code_;

   // Misc

   unsigned version_;

#ifndef BPATCH_LIBRARY
   resource *funcResource;
#endif
};

// Breadth-first iterator
class funcIterator {
  friend class int_function;
 public:
  funcIterator(int_function *startFunc);
  int_function *operator*();
  // postfix increment
  int_function *operator++(int);

 private:
  pdvector<int_function *> unusedFuncs_;
  unsigned index;
};

#endif /* FUNCTION_H */
