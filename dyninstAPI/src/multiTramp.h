/*
 * Copyright (c) 1996-2011 Barton P. Miller
 * 
 * We provide the Paradyn Parallel Performance Tools (below
 * described as "Paradyn") on an AS IS basis, and do not warrant its
 * validity or performance.  We reserve the right to update, modify,
 * or discontinue this software at any time.  We shall have no
 * obligation to supply such updates or modifications or any other
 * form of support to you.
 * 
 * By your use of Paradyn, you understand and agree that we (or any
 * other person or entity with proprietary rights in Paradyn) are
 * under no obligation to provide either maintenance services,
 * update services, notices of latent defects, or correction of
 * defects for Paradyn.
 * 
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

// $Id: multiTramp.h,v 1.29 2008/10/28 18:42:48 bernat Exp $

#if !defined(MULTI_TRAMP_H)
#define MULTI_TRAMP_H

#include "common/h/Dictionary.h"
#include "codeRange.h"
#include "codegen.h"
#include "instP.h"
#include "mapped_object.h"
#include "ast.h"

// A chunk of code (instruction vector), a "do we care where it is",
// and a mechanism to update the code if it moves.

// Superclass of the baseTrampInstance, miniTrampInstance, and
// relocatedInsn classes... lets me play games with virtual functions.

class instPointInstance;
class baseTramp;
class miniTramp;
class codeRange;
class process;
class int_function;
class multiTramp;
class AddressSpace; // process superclass

#if defined( cap_unwind )
#include <libunwind.h>
#define UNW_INFO_TYPE unw_dyn_region_info_t
#else
#define UNW_INFO_TYPE void
#endif

class generatedCodeObject : public codeRange {
 public:
    // Do we need to regenerate this code? Used to determine
    // whether a level of hierarchy needs to be replaced
    virtual bool hasChanged() { return hasChanged_; }

    // And how much space do you need? (worst case scenario)
    virtual unsigned maxSizeRequired() = 0;
    
    // Prep everything
    // This may be a noop if we've already generated and nothing
    // changed. In that case, we bump offset and keep going.

    virtual bool generateCode(codeGen &gen,
                              Address baseInMutatee,
                              UNW_INFO_TYPE * * unwindInformation) = 0;

    // And some global setting up state
    bool generateSetup(codeGen &gen, Address baseInMutatee);

    // Collected code
    bool alreadyGenerated(codeGen &gen, Address baseInMutatee);

    // And actually write things in. This call is "recursive",
    // in that it will call into subsidiary objects (e.g.,
    // baseTramp->miniTramps) to write themselves. When the
    // call completes all subsidiaries must also be written
    // into the address space. However, they can do so however
    // they feel is best. If there isn't anything to do (the 
    // object is in-lined) no writing will occur, but sub-objects
    // will be recursed into. On the other hand, some last-second
    // overwrites can happen (jumps between things)

    // Each level is guaranteed to be installed before the sublevels
    // are called; any updates must be written into the addr space
    // directly

    // Any new code is not linked by this call; installCode can be
    // safely run on multiple points without any new code actually being
    // executed by the application.
    virtual bool installCode() { installed_ = true; return true; }

    // If we generated or installed, then decided something didn't work.
    virtual void invalidateCode();
    
    // Link new code into the application
    virtual bool linkCode() { linked_ = true; return true; } 

    // Begin the deletion process; argument is the child that
    // was just removed
    // Default for those that don't care.
    // Called from the bottom up or top down; check type of
    // argument to figure out which. Defaults to "do nothing"
    virtual void removeCode(generatedCodeObject *) { return; }

    // If you want this to work, specialize it.
    virtual bool safeToFree(codeRange *) { return true; }
    
    // Do the final freeing; let parent know we're gone if necessary.
    // May call the destructor.
    // Override if you allocate space.
    virtual void freeCode() { assert(0); return; }

    // Make a copy if necessary, or just return ourselves.
    virtual generatedCodeObject *replaceCode(generatedCodeObject *newParent) = 0;

    virtual bool shouldGenerate() { return true; }

    // For stack unwinding: give us the "uninstrumented" address (AKA
    // where we would have been) - since all instrumentation occurs at
    // a 'single' point, we can do this.

    virtual Address uninstrumentedAddr() const = 0;

    virtual std::string getTypeString() const;
    
 protected:
    // And modification times
    bool generated_;
    bool installed_;
    bool linked_;
    bool hasChanged_;
 public:
    bool generated() const { return generated_; }
    bool installed() const { return installed_; }
    bool linked() const { return linked_; }
    void markChanged(bool newValue) { hasChanged_ = newValue; }
    void markGenerated(bool newValue) { generated_ = newValue; }
    void markInstalled(bool newValue) { installed_ = newValue; }
    void markLinked(bool newValue) { linked_ = newValue; }

    // Due to our one-pass generation structure, we might
    // want to pin one of these at a particular offset from wherever
    // we start.
    Address pinnedOffset;

    // Amount of space actually used.
    unsigned size_;    

    // And where we ended up
    Address addrInMutatee_;

    Address get_address() const { return addrInMutatee_; }
    void set_address(Address a) { addrInMutatee_ = a; }
    unsigned get_size() const { return size_; }

    bool objIsChild(generatedCodeObject *obj);

    generatedCodeObject() :
        generated_(false),
        installed_(false),
        linked_(false),
        hasChanged_(true),
        pinnedOffset(0),
        size_(0),
        addrInMutatee_(0),
        previous_(NULL),
        fallthrough_(NULL),
        target_(NULL) 
        {}
    generatedCodeObject(const generatedCodeObject *old,
                        AddressSpace *) :
        generated_(old->generated_),
        installed_(old->installed_),
        linked_(old->linked_),
        hasChanged_(old->hasChanged_),
        pinnedOffset(old->pinnedOffset),
        size_(old->size_),
        addrInMutatee_(old->addrInMutatee_),
        previous_(NULL),
        fallthrough_(NULL),
        target_(NULL)// These must be set later
        {}

      virtual ~generatedCodeObject() { }
    // These form mini-CFGs. These pointers allow
    // us to walk 'em safely.

    generatedCodeObject *previous_;
    generatedCodeObject *fallthrough_;
    generatedCodeObject *target_;

    generatedCodeObject *nextObj(); //Gives priority to fallthrough

    // And assignment methods. We do this so we can assign
    // a node n to itself; for example, a shared baseTramp. 
    // In this case, the assignment disappears.
    // Returns the "next object to use".
    // Virtual: relocatedInstruction might platform-override this.
    virtual generatedCodeObject *setPrevious(generatedCodeObject *obj);
    virtual generatedCodeObject *setFallthrough(generatedCodeObject *obj);
    virtual generatedCodeObject *setTarget(generatedCodeObject *obj);
};

class multiTramp;

// Model the "we're at the end of the tramp, jump back" object
class trampEnd : public generatedCodeObject {
 private: 
    trampEnd() {};
 public:
    // Must always give target
    trampEnd(multiTramp *multi, Address target);
    trampEnd(const trampEnd *parEnd, multiTramp *cMT, process *child);

    bool generateCode(codeGen &gen,
                      Address baseInMutatee,
                      UNW_INFO_TYPE * * unwindInformation);

    unsigned maxSizeRequired();

    generatedCodeObject *replaceCode(generatedCodeObject *newParent);

    Address target() { return target_; }
    void changeTarget(Address newTarg );

    virtual Address uninstrumentedAddr() const { return target_; }
    
    void *getPtrToInstruction(Address addr) const;
    virtual std::string getTypeString() const;
    
 private:
    multiTramp *multi_;
    Address target_;
};

class relocatedInstruction;

// Wrapper for "original code pieces" (or replacements),
// AKA not instrumentation.
// Things that have an original address
class relocatedCode : public generatedCodeObject {
 public:
    relocatedCode() :
        generatedCodeObject() {};

    relocatedCode(const relocatedCode *old,
                  AddressSpace *proc) :
        generatedCodeObject(old, proc) 
        {};
    
    virtual Address relocAddr() const = 0;

    virtual const relocatedInstruction *relocInsn() const = 0;
};

class relocatedInstruction : public relocatedCode {
 private:
    relocatedInstruction() {};
 public:
    relocatedInstruction(instruction *i,
                         Address o, // The original location in the untouched mutatee
                         Address f, // Where we're coming from (function relocation)
                         Address t, // Target (if already set)
                         multiTramp *m);
    relocatedInstruction(relocatedInstruction *prev, multiTramp *m);
    relocatedInstruction(const relocatedInstruction *parRI,
                         multiTramp *cMT,
                         process *child);
    // Like the first constructor (with an instruction argument), but filling it in
    // from the original address.  We're smart, we can do that.
#if defined(cap_instruction_api)
    relocatedInstruction(const unsigned char *insnPtr, Address o,
			 Address f,
			 Address t,
			 multiTramp *m);
#endif    
    ~relocatedInstruction();

    instruction *insn;

#if defined(arch_sparc)
    // We wrap delay slots; not going to allow instrumentation
    // of them (as it's a pain)
    instruction *ds_insn;
    instruction *agg_insn;
#endif

    Address origAddr_;
    Address fromAddr_;
    Address targetAddr_;
    multiTramp *multiT;

    Address relocAddr() const;

    unsigned maxSizeRequired();

    bool generateCode(codeGen &gen,
                      Address baseInMutatee,
                      UNW_INFO_TYPE * * unwindInformation);

    // Change the target of a jump 
    void overrideTarget(patchTarget *target);

    // And get the original target
    Address originalTarget() const;

    const relocatedInstruction *relocInsn() const { return this; }

    generatedCodeObject *replaceCode(generatedCodeObject *newParent);
    
    virtual Address uninstrumentedAddr() const { return fromAddr_; }

    void *getPtrToInstruction(Address addr) const;
    virtual std::string getTypeString() const;
    
 private:
    // TODO: move to vector of instructions
    patchTarget *targetOverride_;
};

// Preliminary code to replace an instruction with an AST. 


// Good luck....
// Implemented in replacedInstruction.C ; header file is here
// so that we can build off generatedCodeObject.

class replacedInstruction : public relocatedCode {
 private: 
    replacedInstruction() {};
 public:
    // We take a relocatedInstruction in; first it's
    // moved into the multiTramp, then it's replaced by
    // something else.
    replacedInstruction(const relocatedInstruction *i,
                        AstNodePtr ast,
                        instPoint *p, // Needed for memory instrumentation
                        multiTramp *m);
    // Update constructor
    replacedInstruction(replacedInstruction *prev, multiTramp *m);

    // Fork constructor
    replacedInstruction(const replacedInstruction *parRI,
                        multiTramp *cMT,
                        process *child);

    ~replacedInstruction();

    const relocatedInstruction *relocInsn() const { return oldInsn_; }
    AstNodePtr ast() const { return ast_; }
    instPoint *point() const { return point_; }
    multiTramp *multi() const { return multiT_; }
    AddressSpace *proc() const;

    Address relocAddr() const { return get_address(); }
    Address uninstrumentedAddr() { return relocInsn()->uninstrumentedAddr(); }

    // Overridden from generatedCodeObject
    generatedCodeObject *replaceCode(generatedCodeObject *newParent);
    bool generateCode(codeGen &gen,
                      Address baseInMutatee,
                      UNW_INFO_TYPE **unwindInformation);
    unsigned maxSizeRequired();
    Address uninstrumentedAddr() const { return oldInsn_->uninstrumentedAddr(); }
    Address get_address() const { return addrInMutatee_; };
    unsigned get_size() const { return size_; };

    bool safeToFree(codeRange *range);


    const relocatedInstruction *oldInsn_;
    AstNodePtr ast_;
    instPoint *point_;
    multiTramp *multiT_;
    
    Address addr_;
    unsigned size_;
};

// Code generation. Code generation is really just a special case
// of a baby control flow graph, and we treat it as such. This is
// going to look very similar to the various CFG classes, and it's not
// accidental.

class generatedCFG_t;

class generatedCFG_t {    
    // See previous: STUPID WINDOWS COMPILER
#if defined(os_windows)
 public:
#endif
    generatedCodeObject *start_;
    
    generatedCodeObject *copy_int(generatedCodeObject *obj,
                                  generatedCodeObject *par,
                                  multiTramp *newMulti,
                                  pdvector<generatedCodeObject *> &unused);
    generatedCodeObject *fork_int(const generatedCodeObject *parObj,
                                  generatedCodeObject *childPrev,
                                  multiTramp *childMulti,
                                  process *child);

 public:
    class iterator {
        pdvector<generatedCodeObject *> stack_;
        generatedCodeObject *cur_;
    public:
        iterator(const generatedCFG_t &cfg) : cur_(cfg.start_) {}
        iterator() : cur_(NULL) {};
        void initialize(generatedCFG_t &cfg);
        // (int) : post-increment
        void find(generatedCFG_t &cfg, generatedCodeObject *pointer);
        generatedCodeObject *operator++(int);
        generatedCodeObject *operator*();
    };
    
 public:
    generatedCFG_t() : start_(NULL) {};
    // FORK!
    generatedCFG_t(const generatedCFG_t &par, 
                   multiTramp *cMT,
                   process *child);

    ~generatedCFG_t() {};

    // Called by multiTramp::replaceCode
    void replaceCode(generatedCFG_t &oldCFG,
                     multiTramp *newMulti,
                     pdvector<generatedCodeObject *> &unused);

    void setStart(generatedCodeObject *s) {start_ = s; }
    generatedCodeObject *start() { return start_; }

    // Empty it out; assumes all nodes deleted elsewhere
    void destroy();

    // Get the last thing added; for construction.
    generatedCodeObject *back();
};
      
// Multipoint: represents the physical instruction(s) overwritten with
// a jump. There will be multiple instPoints to one multiTramp. It
// also now owns a lot of the classless utility functions (such as
// installBaseTramp and generateBranchToTramp).

// Note: there can also be multiple multiTramps to a single instPoint
// if a function has been relocated.

#if defined( cap_unwind )
#include <libunwind.h>
#endif /* defined( cap_unwind ) */

class instArea;

class multiTramp : public generatedCodeObject {
    friend class instArea;
  static unsigned id_ctr; // All hail the unique ID

 public:
  unsigned id() const { return id_; }

 private:
  unsigned id_;
  // Blah blah private constructor.
  multiTramp();
  // First-time constructor
  multiTramp(Address addr,
             unsigned size,
             int_function *func);
  // If we're regenerating for some reason, 
  // we use the old one for info rather than having to 
  // redo everything.
  multiTramp(multiTramp *ot);

  // Keep a mapping so we can find multiTramps
  //static dictionary_hash<multiTrampID, multiTramp *> multiTrampDict;
  // Now in the process class so we can handle fork easily

 public:
  // And a factory method. Creates a multiTramp that covers the point
  // given, but may not be aligned with it (platform dependent). The
  // tramp is not actually generated and installed until
  // generateMultiTramp is called.
  // Also registers the multiTramp with the process.
  // Returns a handle that can be used to get the multiTramp; allows
  // us to replace them "under the hood"

  static int findOrCreateMultiTramp(Address pointAddr, 
                                    bblInstance *bbl); 

  // MultiTramps span several instructions. By default, they cover a
  // basic block on non-IA64 platforms; due to the inefficiency of our
  // relocation algorithm, only bundles are covered on that
  // platform. The instPoint logic needs to know what will be covered
  // by a multiTramp to properly decide who can share baseTramps.
  // Returns startAddr and size
  static bool getMultiTrampFootprint(Address instAddr,
                                     AddressSpace *proc,
                                     Address &startAddr,
                                     unsigned &size,
				     bool &basicBlock);

  static multiTramp *getMulti(int id, AddressSpace *proc);

  // Replacement method. Used when the physical multiTramp
  // doesn't agree with the abstract version. Since we re-allocate,
  // we go ahead and make a new one.
  // TODO: two-phase remove step.
  static bool replaceMultiTramp(multiTramp *oldMulti, bool &deleteReplaced);

  // Fork constructor. Must copy over all sub-objects as well.
  multiTramp(const multiTramp *parentMulti, process *child);

  // Error codes!
  typedef enum { mtSuccess, mtAllocFailed, mtTryRelocation, mtError } mtErrorCode_t;

  // Entry point for code generation: from an instPointInstance
  mtErrorCode_t generateMultiTramp();
  mtErrorCode_t installMultiTramp();
  mtErrorCode_t linkMultiTramp();

  // Temporarily install or remove
  bool disable();
  bool enable();

  ~multiTramp();

  int_function *func() const;
  
  baseTrampInstance *getBaseTrampInstance(instPointInstance *point,
					  callWhen when) const;
  // This has "byAddr" added because the Address and callWhen types
  // overlap; no accidentally calling the wrong one.
  baseTrampInstance *getBaseTrampInstanceByAddr(Address addr) const;

  // codeRange stuff
  Address get_address() const { return trampAddr_; }
  unsigned get_size() const { return trampSize_; }
  void *get_local_ptr() const { return generatedMultiT_.start_ptr(); }

  void *getPtrToInstruction(Address addr) const;

  Address getAddress() const { return trampAddr_; }

  Address trampAddr() const { return trampAddr_; }
  Address trampSize() const { return trampSize_; }

  Address instAddr() const { return instAddr_; }
  unsigned instSize() const { return instSize_; }
  unsigned branchSize() const { return branchSize_; }

  // Address in the multitramp -> equivalent in uninstrumented terms
  // Kind of the reverse of base tramp instances...
  Address instToUninstAddr(Address addr);
  // Returns 0 if the addr isn't within the tramp
  Address uninstToInstAddr(Address addr);

  // Returns the closest instPoint that we're at
  instPoint *findInstPointByAddr(Address addr);

  // Can't call this on a multiTramp as it covers multiple
  // instructions...
  Address uninstrumentedAddr() const { assert(0); return 0; }

  // Mmmm breaking logic up
  // Returns true if the PC is in the multiTramp (or sub-areas)
  // and after the given miniTramp. Returns false if it's before.
  // Asserts if the PC is not "in" the multiTramp at all.
  bool catchupRequired(Address pc, miniTramp *newMT, bool active, codeRange *range = NULL);

  AddressSpace *proc() const;

  ///////////////// Generation
  bool generateCode(codeGen &gen,
                    Address baseInMutatee,
                    UNW_INFO_TYPE * * unwindInformation);

  //////////////// Helper to generateCode, Do the actual work of calling generate()
  bool generateCodeWorker(unsigned size_required, UNW_INFO_TYPE **unwind_region);

  // The most that we can need to get to a multitramp...
  unsigned maxSizeRequired();
  // the space needed for a jump to this particular multitramp...
  // (unsigned) -1: can't make it, so sorry.
  unsigned sizeDesired() const { return branchSize_; };
  bool usesTrap() const { return usedTrap_; };

  bool hasChanged();
  bool installCode();
  void invalidateCode();
  bool linkCode();
  void removeCode(generatedCodeObject *subObject);
    // STUPID WINDOWS COMPILER
  generatedCodeObject *replaceCode(generatedCodeObject *newParent);
  bool safeToFree(codeRange *range);
  void freeCode();
  
  // flags to help us recognize active instrumentation and not remove it
  void setIsActive( bool value );
  bool getIsActive() { return isActive_; }//true if multi is on the call stack
  bool getPartlyGone() { return partlyGone_; }//true if multi has been unlinked
  multiTramp *getStompMulti() { return stompMulti_; }
  Address getFuncBaseInMutatee() { return funcBaseInMutatee_; }
  trampEnd* getTrampEnd() { return trampEnd_; } 
  // point trampEnd to relocated code, for use when the function is updated
  // while instrumentation is active
  void updateTrampEnd(instPoint *point);
  void setTrampEnd(trampEnd &newTramp);
  bool hasMultipleBaseTramps();

 private:
  Address instAddr_; 
  Address trampAddr_;  // Where we are
  unsigned trampSize_; // Size of the generated multiTramp
  unsigned instSize_; // Size of the original instrumented area
  unsigned branchSize_; // Size of the branch instruction(s) used to get to
                   // the multiTramp (in a perfect world). Also, MAXINT
                   // means that due to architecture limitations we
                   // can't put a branch in, and is a signal to either
                   // relocate or trap (or, likely, both).
  bool usedTrap_;

  int_function *func_;
  AddressSpace *proc_;
  // A list of tramp structures we created a baseTramp stores
  // offsets for various points. We keep it separate to avoid clogging
  // up this structure.
  pdvector<generatedCodeObject *> deletedObjs;

  // A mini-CFG for code generation
  generatedCFG_t generatedCFG_;
  // And track all the instructions we cover.
  dictionary_hash<Address, relocatedCode *> insns_;
  void updateInsnDict();

  // Insert jumps/traps/whatever from previous multiTramp to new multiTramp.
  // Allows correct instrumentation semantics - "if you add to a later point,
  // the new code will execute". We add to this vector from the previous 
  // multiTramp and put traps in when linking the new one. Then we nuke the
  // vector.
  // Format: <uninst addr, addr in old multiTramp>
  // We need to map old mT to new mT - however, the "new mT" isn't done until
  // we generate. So instead we go "uninst, oldMT", as we can map uninst -> newMT
  // via insns_
  pdvector<pdpair<Address, Address> > *previousInsnAddrs_;
  void constructPreviousInsnList(multiTramp *oldMulti);

  codeGen generatedMultiT_;
  codeGen jumpBuf_;

  // And what was there before we copied it.
  void *savedCodeBuf_;

  friend class instPointInstance;
  void addInstInstance(instPointInstance *instInstance);

  // Go and check to see if there are any new instInstances...
  void updateInstInstances();

  bool generateBranchToTramp(codeGen &gen);
  bool generateTrapToTramp(codeGen &gen);
  bool fillJumpBuf(codeGen &gen);
  
#if defined( cap_unwind )
  unw_dyn_info_t * unwindInformation;
#endif /* defined( cap_unwind ) */

  bool changedSinceLastGeneration_;

  trampEnd *trampEnd_;

  // flags for active instrumentation 
  bool isActive_;   // is the multitramp on the call stack 
  bool partlyGone_; // true if multiTramp has been disabled (unlinked)
  Address funcBaseInMutatee_; //track the base of the relocated function in 
                              //the mutatee, as updates to the function's 
                              //analysis can cause this information to be lost
  multiTramp *stompMulti_; // if the multiTramp was replaced with a new one, 
                           // store a pointer to it


  void setFirstInsn(generatedCodeObject *obj) { generatedCFG_.setStart(obj); }
};

// Funfun... we need to look up multiTramps by both their instrumentation range and 
// the instrumented range; but current codeRange can only do one. So we make a wrapper.

// This reports the instAddr_ and instSize_ members

class instArea : public codeRange {
 public:
    instArea(multiTramp *m) : multi(m) {}
    multiTramp *multi;
    Address get_address() const { assert(multi); return multi->instAddr(); }
    unsigned get_size() const { assert(multi); return multi->instSize(); }
    void *get_local_ptr() const { assert(multi); return multi->jumpBuf_.start_ptr(); }
};



/////////////////
// Sticking this here for now
class replacedFunctionCall : public codeRange {
 public:
    Address get_address() const { return callAddr; }
    unsigned get_size() const { return callSize; }
    void *get_local_ptr() const { return newCall.start_ptr(); }
    
    Address callAddr;
    unsigned callSize;
    Address newTargetAddr;
    codeGen newCall;
    codeGen oldCall;
};

#endif
