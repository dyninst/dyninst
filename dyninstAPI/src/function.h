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
 
// $Id: function.h,v 1.20 2005/10/31 22:42:53 rutar Exp $

#ifndef FUNCTION_H
#define FUNCTION_H

#include "common/h/String.h"
#include "common/h/Vector.h"
#include "common/h/Types.h"
#include "common/h/Pair.h"
#include "codeRange.h"
#include "arch.h" // instruction
#include "util.h"

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

class instPointInstance;

#include "dyninstAPI/h/BPatch_Set.h"

class instPoint;

class Frame;

class functionReplacement;

class int_function;
class int_basicBlock;
class bitArray;

class funcMod;

// A specific instance (relocated version) of a basic block
// It's really a semi-smart struct...
class bblInstance : public codeRange {
    friend class int_basicBlock;
    friend class int_function;

    // "We'll set things up later" constructor. Private because only int_basicBlock
    // should be playing in here
    bblInstance(int_basicBlock *parent, int version);
 public:
    bblInstance(Address start, Address last, Address end, int_basicBlock *parent, int version);
    bblInstance(const bblInstance *parent, int_basicBlock *block);
    ~bblInstance();

    Address firstInsnAddr() const { return firstInsnAddr_; }
    Address lastInsnAddr() const { return lastInsnAddr_; }
    // Just to be obvious -- this is the end addr of the block
    Address endAddr() const { return blockEndAddr_; }
    Address getSize() const { return blockEndAddr_ - firstInsnAddr_; }

    // And equivalence in addresses...
    Address equivAddr(bblInstance *otherBBL, Address otherAddr) const;

    // As a note: do _NOT_ create an address-based comparison of this
    // class unless you just need some sort of ordering. We may create these
    // blocks in some random place; depending on address is just plain dumb.

    Address get_address_cr() const { return firstInsnAddr(); }
    unsigned get_size_cr() const { return getSize(); }
    void *getPtrToInstruction(Address addr) const;
    
    process *proc() const;
    int_function *func() const;
    int_basicBlock *block() const;
    int version() const;

#if defined(cap_relocation)
    // Get the most space necessary to relocate this basic block,
    // using worst-case analysis.
    bool relocationSetup(bblInstance *orig, pdvector<funcMod *> &mods);
    unsigned sizeRequired();
    // Pin a block to a particular address; in theory, we can only
    // do these for blocks that can be entered via jumps, but for now
    // we do it all the time.
    void setStartAddr(Address addr);

    // And generate a moved copy into ze basic block
    bool generate();
    // And write the block into the addr space
    bool install();
    // Check for safety...
    bool check(pdvector<Address> &checkPCs);
    // Link things up
    bool link(pdvector<codeRange *> &overwrittenObjs);
#endif

 private:
#if defined(cap_relocation)
    class reloc_info_t {
    public:
       dictionary_hash<Address, Address> changedAddrs_;
       pdvector<instruction *> insns_;
       unsigned maxSize_;
       bblInstance *origInstance_;
       pdvector<funcMod *> appliedMods_;
       codeGen generatedBlock_; // Kept so we can quickly iterate over the block
       // in the future.
       functionReplacement *jumpToBlock_; // Kept here between generate->link

       reloc_info_t();
       reloc_info_t(reloc_info_t *parent, int_basicBlock *block);
       ~reloc_info_t();
    };

    //Setter functions for relocation information
    dictionary_hash<Address, Address> &changedAddrs();
    pdvector<instruction *> &insns();
    unsigned &maxSize();
    bblInstance *&origInstance();
    pdvector<funcMod *> &appliedMods();
    codeGen &generatedBlock();
    functionReplacement *&jumpToBlock();

    //Getter functions for relocation information
    dictionary_hash<Address, Address> &getChangedAddrs() const;
    pdvector<instruction *> &getInsns() const;
    unsigned getMaxSize() const;
    bblInstance *getOrigInstance() const;
    pdvector<funcMod *> &getAppliedMods() const;
    codeGen &getGeneratedBlock() const;
    functionReplacement *getJumpToBlock() const;

    reloc_info_t *reloc_info;
#endif

    Address firstInsnAddr_;
    Address lastInsnAddr_;
    Address blockEndAddr_;
    int_basicBlock *block_;
    int version_;
};

class int_basicBlock {
    friend class int_function;
 public:
    int_basicBlock(const image_basicBlock *ib, Address baseAddr, int_function *func);
    int_basicBlock(const int_basicBlock *parent, int_function *func);
    ~int_basicBlock();

    bool isEntryBlock() const { return ib_->isEntryBlock(); }
    bool isExitBlock() const { return ib_->isExitBlock(); }
    
    static int compare(int_basicBlock *&b1,
                       int_basicBlock *&b2) {
        // First instance: original bbl.
        if (b1->instances_[0]->firstInsnAddr() < b2->instances_[0]->firstInsnAddr())
            return -1;
        if (b2->instances_[0]->firstInsnAddr() < b1->instances_[0]->firstInsnAddr())
            return 1;
        assert(b1 == b2);
        return 0;
    }

    const pdvector<bblInstance *> &instances() const;
    bblInstance *origInstance() const;
    bblInstance *instVer(unsigned index) const;
    void removeVersion(unsigned index);

    void debugPrint();

    void getSources(pdvector<int_basicBlock *> &ins) const;
    void getTargets(pdvector<int_basicBlock *> &outs) const;

    int_basicBlock *getFallthrough() const;

    int id() const { return ib_->id(); }

    int_function *func() const { return func_; }
    process *proc() const;

#if defined(arch_ia64)
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
#endif

    // Liveness functions for AIX && AMD64
#if defined(os_aix) || defined(arch_x86_64)
    
    /** Initializes the gen/kill sets for register liveness analysis */
    bool initRegisterGenKill();
   
    /** Used in the fixed point iteration part of liveness */
    bool updateRegisterInOut(bool isFP);
   
    /** Returns in set for GPR */
    bitArray * getInSet();
   
    /** Returns in set for FPR */
    bitArray * getInFPSet();
   
    /** Puts the live registers from bitArrays to integers stored by inst point */
    int liveRegistersIntoSet(int *& liveReg, int *& liveFPReg,
			     unsigned long address);

    /** Puts the live SP registers from bitArrays to integers stored by inst point */
    int liveSPRegistersIntoSet(int *& liveSPReg,
			       unsigned long address);
#endif     

    void setHighLevelBlock(void *newb);
    void *getHighLevelBlock() const;

 private:
    void *highlevel_block; //Should point to a BPatch_basicBlock, if they've
                           //been created.
#if defined(arch_ia64)
    BPatch_Set<int_basicBlock *> *dataFlowIn;
    BPatch_Set<int_basicBlock *> *dataFlowOut;
    int_basicBlock *dataFlowGen;
    int_basicBlock *dataFlowKill;
#endif

#if defined(os_aix) || defined(arch_x86_64)
   /* Liveness analysis variables */
   /** gen registers */
   bitArray * gen;
   bitArray * genFP;
   
   /** kill registers */
   bitArray * kill;
   bitArray * killFP;
   
   /** in registers */
   bitArray * in;
   bitArray * inFP;
   
   /** out registers */
   bitArray * out;
   bitArray * outFP;
#endif


    int_function *func_;
    const image_basicBlock *ib_;

    // A single "logical" basic block may correspond to multiple
    // physical areas of code. In particular, we may relocate the
    // block (either replaced or duplicated).
    pdvector<bblInstance *> instances_;
};

class int_function {
  friend class int_basicBlock;
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
   const pdvector<pdstring> &symTabNameVector() const { return ifunc_->symTabNameVector(); }
   const pdvector<pdstring> &prettyNameVector() const { return ifunc_->prettyNameVector(); }

   // May change when we relocate...
   Address getAddress() const {return addr_;}
   // Don't use this...
   unsigned getSize_NP();


   // Not defined here so we don't have to play header file magic
   const image_func *ifunc() const;
   mapped_module *mod() const;
   mapped_object *obj() const;
   process *proc() const;

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
   bblInstance *findBlockInstanceByAddr(Address addr);


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

   bool canBeRelocated() const { return ifunc_->canBeRelocated(); }
   int version() const { return version_; }

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
    
#if defined(arch_alpha)
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

#if defined(cap_relocation)
   // These are defined in reloc-func.C to keep large chunks of
   // functionality separate!

   // Deceptively simple... take a list of requested changes,
   // and make a copy of the function somewhere out in space.
   // Defaults to the first version of the function = 0
   bool relocationGenerate(pdvector<funcMod *> &mods, int version = 0);
   // The above gives us a set of basic blocks that have little
   // code segments. Install them in the address space....
   bool relocationInstall();
   // Check whether we can overwrite...
   bool relocationCheck(pdvector<Address> &checkPCs);
   // And overwrite code with jumps to the relocated version
   bool relocationLink(pdvector<codeRange *> &overwritten_objs);

   // Cleanup code: remove (back to) the latest installed version...
   bool relocationInvalidate();

   // A top-level function; for each instPoint, see if we need to 
   // relocate the function.
   bool expandForInstrumentation();

   pdvector<funcMod *> &enlargeMods() { return enlargeMods_; }

#endif

 private:

   ///////////////////// Basic func info
   Address addr_; // Absolute address of the start of the function

   image_func *ifunc_;
   mapped_module *mod_; // This is really a dodge; translate a list of
			// image_funcs to int_funcs

   ///////////////////// CFG and function body
   pdvector< int_basicBlock* > blockList;
   //BPatch_flowGraph *flowGraph;

   ///////////////////// Instpoints 

   pdvector<instPoint*> entryPoints_;	/* place to instrument entry (often not addr) */
   pdvector<instPoint*> exitPoints_;	/* return point(s). */
   pdvector<instPoint*> callPoints_;	/* pointer to the calls */
   pdvector<instPoint*> arbitraryPoints_;	       /* pointer to the calls */

#if defined(cap_relocation)
   // Status tracking variables
   int generatedVersion_;
   int installedVersion_;
   int linkedVersion_;

   // We want to keep around expansions for instrumentation
   pdvector<funcMod *> enlargeMods_;
#endif

   // Used to sync with instPoints
   int version_;


   codeRangeTree blocksByAddr_;
   void addBBLInstance(bblInstance *instance);
   void deleteBBLInstance(bblInstance *instance);

#ifndef BPATCH_LIBRARY
   resource *funcResource;
#endif
};

// All-purpose; use for function relocation, actual function
// replacement, etc.
class functionReplacement : public codeRange {

 public:
    functionReplacement(int_basicBlock *sourceBlock,
                        int_basicBlock *targetBlock,
                        unsigned sourceVersion = 0,
                        unsigned targetVersion = 0);
    ~functionReplacement() {};

    unsigned maxSizeRequired();

    bool generateFuncRep();
    bool installFuncRep();
    bool checkFuncRep(pdvector<Address> &checkPCs);
    bool linkFuncRep(pdvector<codeRange *> &overwrittenObjs);
    void removeFuncRep();
    
    bool overwritesMultipleBlocks();

    int_basicBlock *source();
    int_basicBlock *target();
    unsigned sourceVersion();
    unsigned targetVersion();

    Address get_address_cr() const;
    unsigned get_size_cr() const;

 private:
    codeGen jumpToRelocated;
    codeGen origInsns;

    int_basicBlock *sourceBlock_;
    int_basicBlock *targetBlock_;

    // If no relocation, these will be zero.
    unsigned sourceVersion_;
    unsigned targetVersion_;

    // We may need more room than there is in a block;
    // this makes things "interesting".
    bool overwritesMultipleBlocks_; 
};



#endif /* FUNCTION_H */
