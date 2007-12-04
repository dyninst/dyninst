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
 
// $Id: function.h,v 1.45 2007/12/04 17:58:22 bernat Exp $

#ifndef FUNCTION_H
#define FUNCTION_H

#include "common/h/String.h"
#include "common/h/Vector.h"
#include "common/h/Types.h"
#include "common/h/Pair.h"
#include "codeRange.h"
#include "arch.h" // instruction
#include "util.h"
#include "image-func.h"

#include "bitArray.h"

class process;
class mapped_module;
class mapped_object;

class BPatch_flowGraph;
class BPatch_loopTreeNode;
class BPatch_basicBlock;
class BPatch_basicBlockLoop;

class instPointInstance;

#if defined(arch_ia64)
#include "dyninstAPI/h/BPatch_Set.h"
#endif

#include "dyninstAPI/src/ast.h"

class instPoint;

class Frame;

class functionReplacement;

class int_function;
class int_basicBlock;
class funcMod;

typedef enum callType {
  unknown_call,
  cdecl_call,
  stdcall_call,
  fastcall_call,
  thiscall_call
} callType;

// A specific instance (relocated version) of a basic block
// It's really a semi-smart struct...
class bblInstance : public codeRange {
    friend class int_basicBlock;
    friend class int_function;
    friend class multiTramp;

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
    void *get_local_ptr() const;


    const void *getPtrToOrigInstruction(Address addr) const;
    unsigned getRelocInsnSize(Address addr) const;

    void getOrigInstructionInfo(Address addr, void *&ptr, Address &origAddr, unsigned &origSize) const;

    //process *proc() const;
    AddressSpace *proc() const;
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

#if defined(cap_relocation)
    class reloc_info_t {
    public:       
       unsigned maxSize_;
       bblInstance *origInstance_;
       pdvector<funcMod *> appliedMods_;
       codeGen generatedBlock_; // Kept so we can quickly iterate over the block
       // in the future.
       functionReplacement *jumpToBlock_; // Kept here between generate->link

       struct relocInsn {
	 // Where we ended up; can derive size from next - this one
	 Address origAddr;
	 Address relocAddr;
	 instruction *origInsn;
	 const void *origPtr;
	 Address relocTarget;
	 unsigned relocSize;
       };
       
       pdvector<relocInsn *> relocs_;

       reloc_info_t();
       reloc_info_t(reloc_info_t *parent, int_basicBlock *block);
       ~reloc_info_t();
    };

 private:

    //Setter functions for relocation information
    pdvector<reloc_info_t::relocInsn *> &relocs();
    unsigned &maxSize();
    bblInstance *&origInstance();
    pdvector<funcMod *> &appliedMods();
    codeGen &generatedBlock();
    functionReplacement *&jumpToBlock();


    pdvector<reloc_info_t::relocInsn *> &get_relocs() const;

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
    int_basicBlock(const image_basicBlock *ib, Address baseAddr, int_function *func, int id);
    int_basicBlock(const int_basicBlock *parent, int_function *func, int id);
    ~int_basicBlock();

        // just because a block is an entry block doesn't mean it is
        // an entry block that this block's function cares about
    bool isEntryBlock() const;
    bool isExitBlock() const { return ib_->isExitBlock(); }

        // int_basicBlocks are not shared, but their underlying blocks
        // may be
    bool hasSharedBase() const { return ib_->isShared(); }

    const image_basicBlock * llb() const { return ib_; }
    
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
    EdgeTypeEnum getSourceEdgeType(int_basicBlock *source) const;
    EdgeTypeEnum getTargetEdgeType(int_basicBlock *target) const;

    int_basicBlock *getFallthrough() const;

    int id() const { return id_; }

    // True if we need to put a jump in here. If we're an entry block
    // or the target of an indirect jump (for now).
    bool needsJumpToNewVersion();

    // A block will need relocation (when instrumenting) if
    // its underlying image_basicBlock is shared (this is independant
    // of relocation requirements dependant on size available)
    bool needsRelocation() const;

    int_function *func() const { return func_; }
    //process *proc() const;
    AddressSpace *proc() const;

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


    int_function *func_;
    const image_basicBlock *ib_;

    int id_;

    // A single "logical" basic block may correspond to multiple
    // physical areas of code. In particular, we may relocate the
    // block (either replaced or duplicated).
    pdvector<bblInstance *> instances_;
};

class int_function {
  friend class int_basicBlock;
 public:
   //static pdstring emptyString;

   // Almost everything gets filled in later.
   int_function(image_func *f,
		Address baseAddr,
                mapped_module *mod);

   int_function(const int_function *parent,
                mapped_module *child_mod,
                process *childP);

   ~int_function();

   ////////////////////////////////////////////////
   // Passthrough functions.
   ////////////////////////////////////////////////
   // To minimize wasted memory (since there will be many copies of
   // this function) we make most methods passthroughs to the original
   // parsed version.

   const string &symTabName() const;
   const string &prettyName() const { return ifunc_->prettyName(); };
   const string &typedName() const { return ifunc_->typedName(); };
   
   const vector<string>& symTabNameVector() const { return ifunc_->symTabNameVector(); }
   const vector<string>& prettyNameVector() const { return ifunc_->prettyNameVector(); }
   const vector<string>& typedNameVector() const { return ifunc_->typedNameVector(); }

   // Debuggering functions
   void debugPrint() const;

   // And add...
   // Don't make the pdstring a reference; we want a copy.
   void addSymTabName(const pdstring name, bool isPrimary = false);
   void addPrettyName(const pdstring name, bool isPrimary = false);

   // May change when we relocate...
   Address getAddress() const {return addr_;}
   // Don't use this...
   unsigned getSize_NP();


   // Not defined here so we don't have to play header file magic
   // Not const; we can add names via the Dyninst layer
   image_func *ifunc();
   mapped_module *mod() const;
   mapped_object *obj() const;
   //process *proc() const;
   AddressSpace *proc() const;

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
   
   instPoint *findInstPByAddr(Address addr);

   // And for adding... we map by address to instPoint
   // as a single instPoint may have multiple instances
   void registerInstPointAddr(Address addr, instPoint *inst);
   void unregisterInstPointAddr(Address addr, instPoint *inst);

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
   bool needsRelocation() const { return ifunc_->needsRelocation(); }
   int version() const { return version_; }


   ////////////////////////////////////////////////
   // Code overlapping
   ////////////////////////////////////////////////
   // Get all functions that "share" the block. Actually, the
   // int_basicBlock will not be shared (they are per function),
   // but the underlying image_basicBlock records the sharing status. 
   // So dodge through to the image layer and find out that info. 
   // Returns true if such functions exist.

   bool getSharingFuncs(int_basicBlock *b,
                        pdvector<int_function *> &funcs);

   // The same, but for any function that overlaps with any of
   // our basic blocks.
   // OPTIMIZATION: we're not checking all blocks, only an exit
   // point; this _should_ work :) but needs to change if we
   // ever do flow-sensitive parsing
   bool getOverlappingFuncs(pdvector<int_function *> &funcs);


   ////////////////////////////////////////////////
   // Misc
   ////////////////////////////////////////////////


   const pdvector< int_parRegion* > &parRegions() { return parallelRegions_; }

   bool containsSharedBlocks() const { return ifunc_->containsSharedBlocks(); }

   unsigned getNumDynamicCalls();

    // Fill the <callers> vector with pointers to the statically-determined
    // list of functions that call this function.
    void getStaticCallers(pdvector <int_function *> &callers);

   codeRange *copy() const;
    
#if defined(arch_alpha)
   int frame_size() const { return ifunc_->frame_size; };

#endif

#if defined(sparc_sun_solaris2_4)
   bool is_o7_live(){ return ifunc_->is_o7_live(); }
#endif

   void updateForFork(process *childProcess, const process *parentProcess);

#if defined(arch_ia64)
   // We need to know where all the alloc instructions in the
   // function are to do a reasonable job of register allocation
   // in the base tramp.
   
 private:
   Address baseAddr_;
   pdvector< Address > cachedAllocs;
    
 public:
   // Accessor function that ensures that the cachedAllocs are
   // up-to-date.  Necessary because of delayed parsing.
   pdvector< Address > & getAllocs();
   
   // Since the IA-64 ABI does not define a frame pointer register,
   // we use DWARF debug records (DW_AT_frame_base entries) to 
   // construct an AST which calculates the frame pointer.
   AstNodePtr getFramePointerCalculator();
   
   // Place to store the results of doFloatingPointStaticAnalysis().
   // This way, if they are ever needed in a mini-tramp, emitFuncJump()
   // for example, the expensive operation doesn't need to happen again.
   bool * getUsedFPregs();
#endif

#if defined(cap_relocation)
   // These are defined in reloc-func.C to keep large chunks of
   // functionality separate!

   // Deceptively simple... take a list of requested changes,
   // and make a copy of the function somewhere out in space.
   // Defaults to the first version of the function = 0
   bool relocationGenerate(pdvector<funcMod *> &mods, 
                           int version,
                           pdvector<int_function *> &needReloc);
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

#if defined(os_windows) 
   //Calling convention for this function
   callType int_function::getCallingConvention();
   int getParamSize() { return paramSize; }
   void setParamSize(int s) { paramSize = s; }
#endif


 private:

   ///////////////////// Basic func info
   Address addr_; // Absolute address of the start of the function

   image_func *ifunc_;
   mapped_module *mod_; // This is really a dodge; translate a list of
			// image_funcs to int_funcs

   ///////////////////// CFG and function body
   pdvector< int_basicBlock* > blockList;

   // Added to support translation between function-specific int_basicBlocks
   // and potentially shared image_basicBlocks
   dictionary_hash<int, int> blockIDmap;

   //BPatch_flowGraph *flowGraph;

   ///////////////////// Instpoints 

   pdvector<instPoint*> entryPoints_;	/* place to instrument entry (often not addr) */
   pdvector<instPoint*> exitPoints_;	/* return point(s). */
   pdvector<instPoint*> callPoints_;	/* pointer to the calls */
   pdvector<instPoint*> arbitraryPoints_;	       /* pointer to the calls */

   dictionary_hash<Address, instPoint *> instPsByAddr_;

   //////////////////////////  Parallel Regions 
   pdvector<int_parRegion*> parallelRegions_; /* pointer to the parallel regions */


#if defined(cap_relocation)
   // Status tracking variables
   int generatedVersion_;
   int installedVersion_;
   int linkedVersion_;

   // We want to keep around expansions for instrumentation
   pdvector<funcMod *> enlargeMods_;

   // The actual relocation workhorse (the public method is just a facade)
   bool relocationGenerateInt(pdvector<funcMod *> &mods, 
                              int version,
                              pdvector<int_function *> &needReloc);
#endif

   // Used to sync with instPoints
   int version_;

   codeRangeTree blocksByAddr_;
   void addBBLInstance(bblInstance *instance);
   void deleteBBLInstance(bblInstance *instance);

#if defined(os_windows) 
   callType callingConv;
   int paramSize;
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

    bool generateFuncRepJump(pdvector<int_function *> &needReloc);
    bool generateFuncRepTrap(pdvector<int_function *> &needReloc);
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
    void *get_local_ptr() const { return jumpToRelocated.start_ptr(); }

    AddressSpace *proc() const;

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

    bool usesTrap_;
};



#endif /* FUNCTION_H */
