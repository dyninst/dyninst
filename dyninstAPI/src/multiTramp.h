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

// $Id: multiTramp.h,v 1.5 2005/08/08 22:39:29 bernat Exp $

#if !defined(MULTI_TRAMP_H)
#define MULTI_TRAMP_H

#include "common/h/Dictionary.h"
#include "codeRange.h"
#include "arch.h"
#include "instP.h"

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


class generatedCodeObject : public codeRange {
 public:
    // Do we need to regenerate this code? Used to determine
    // whether a level of hierarchy needs to be replaced
    virtual bool hasChanged() { return false; }

    // And how much space do you need? (worst case scenario)
    virtual unsigned maxSizeRequired() = 0;
    
    // Prep everything
    // This may be a noop if we've already generated and nothing
    // changed. In that case, we bump offset and keep going.

    virtual bool generateCode(codeGen &gen,
                              Address baseInMutatee) = 0;

    // Aaaand a helper function; returns true if we're already
    // done
    bool alreadyGenerated(codeGen &gen, Address baseInMutatee);
    // And some global setting
    bool generateSetup(codeGen &gen, Address baseInMutatee);

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
    virtual bool installCode() { return true; }

    // If we generated or installed, then decided something didn't work.
    virtual void invalidateCode();
    
    // Link new code into the application
    virtual bool linkCode() { return true; } 

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

    // And modification times
    bool generated_;
    bool installed_;
    bool linked_;

    bool generated() const { return generated_; }
    bool installed() const { return installed_; }
    bool linked() const { return linked_; }

    // Due to our one-pass generation structure, we might
    // want to pin one of these at a particular offset from wherever
    // we start.
    Address pinnedOffset;

    // Amount of space actually used.
    unsigned size_;    

    // And where we ended up
    Address addrInMutatee_;

    Address get_address_cr() const { return addrInMutatee_; }
    unsigned get_size_cr() const { return size_; }

    bool objIsChild(generatedCodeObject *obj);

    generatedCodeObject() :
        generated_(false),
        installed_(false),
        linked_(false),
        pinnedOffset(0),
        size_(0),
        addrInMutatee_(0),
        previous_(NULL),
        fallthrough_(NULL),
        target_(NULL) 
        {}
    generatedCodeObject(const generatedCodeObject *old,
                        process *) :
        generated_(old->generated_),
        installed_(old->installed_),
        linked_(old->linked_),
        pinnedOffset(old->pinnedOffset),
        size_(old->size_),
        addrInMutatee_(old->addrInMutatee_),
        previous_(NULL),
        fallthrough_(NULL),
        target_(NULL)// These must be set later
        {}

    // These form mini-CFGs. These pointers allow
    // us to walk 'em safely.

    generatedCodeObject *previous_;
    generatedCodeObject *fallthrough_;
    generatedCodeObject *target_;
  
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
                      Address baseInMutatee);

    unsigned maxSizeRequired();

    generatedCodeObject *replaceCode(generatedCodeObject *newParent);

    Address target() { return target_; }

    virtual Address uninstrumentedAddr() const { return target_; }
    
 private:
    multiTramp *multi_;
    Address target_;
};

class relocatedInstruction : public generatedCodeObject {
 private:
    relocatedInstruction() {};
 public:
    relocatedInstruction(instruction & i,
                         Address o,
                         multiTramp *m) :
        generatedCodeObject(),
        insn(i),
#if defined(arch_sparc)
        ds_insn(0),
        agg_insn(0),
        hasDS(false),
        hasAgg(false),
#endif
        origAddr(o), 
        multiT(m), targetOverride_(0) {}
    relocatedInstruction(relocatedInstruction *prev,
                         multiTramp *m) :
        generatedCodeObject(),
        insn(prev->insn),
#if defined(arch_sparc)
        ds_insn(prev->ds_insn),
        agg_insn(prev->agg_insn),
        hasDS(prev->hasDS),
        hasAgg(prev->hasAgg),
#endif
        origAddr(prev->origAddr),
        multiT(m),
        targetOverride_(prev->targetOverride_) {}

    relocatedInstruction(const relocatedInstruction *parRI,
                         multiTramp *cMT,
                         process *child);

    instruction insn;

#if defined(arch_sparc)
    // We wrap delay slots; not going to allow instrumentation
    // of them (as it's a pain)
    instruction ds_insn;
    instruction agg_insn;
    void setDS(instruction insn) { ds_insn = insn; hasDS = true; }
    void setAgg(instruction insn) { agg_insn = insn; hasAgg = true; }
    bool hasDS;
    bool hasAgg;
#endif

    Address origAddr;
    multiTramp *multiT;

    Address relocAddr() const;

    unsigned maxSizeRequired();

    bool generateCode(codeGen &gen,
                      Address baseInMutatee);

    // Change the target of a jump 
    void overrideTarget(Address newTarget);

    // And get the original target
    Address originalTarget() const;

    generatedCodeObject *replaceCode(generatedCodeObject *newParent);
    
    virtual Address uninstrumentedAddr() const { return origAddr; }

 private:
    // TODO: move to vector of instructions
    Address targetOverride_;
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

class multiTramp : public generatedCodeObject {
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
             int_function *func,
             bool allowTrap);
  // If we're regenerating for some reason, 
  // we use the old one for info rather than having to 
  // redo everything.
  multiTramp(multiTramp *ot);

  // Keep a mapping so we can find multiTramps
  //static dictionary_hash<multiTrampID, multiTramp *> multiTrampDict;
  // Now in the process class so we can handle fork easily

  // Replacement method. Used when the physical multiTramp
  // doesn't agree with the abstract version. Since we re-allocate,
  // we go ahead and make a new one.
  // TODO: two-phase remove step.
  static bool replaceMultiTramp(multiTramp *oldMulti, bool &deleteReplaced);

 public:
  // And a factory method. Creates a multiTramp that covers the point
  // given, but may not be aligned with it (platform dependent). The
  // tramp is not actually generated and installed until
  // generateMultiTramp is called.
  // Also registers the multiTramp with the process.
  // Returns a handle that can be used to get the multiTramp; allows
  // us to replace them "under the hood"

  static int findOrCreateMultiTramp(Address pointAddr, 
                                    process *proc,
                                    bool allowTrap);

  // MultiTramps span several instructions. By default, they cover a
  // basic block on non-IA64 platforms; due to the inefficiency of our
  // relocation algorithm, only bundles are covered on that
  // platform. The instPoint logic needs to know what will be covered
  // by a multiTramp to properly decide who can share baseTramps.
  // Returns startAddr and size
  static bool getMultiTrampFootprint(Address instAddr,
                                     process *proc,
                                     Address &startAddr,
                                     unsigned &size);

  static multiTramp *getMulti(int id, process *proc);

  // Fork constructor. Must copy over all sub-objects as well.
  multiTramp(const multiTramp *parentMulti, process *child);

  // Entry point for code generation: from an instPointInstance
  bool generateMultiTramp();
  bool installMultiTramp();
  bool linkMultiTramp();

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
  Address get_address_cr() const { return trampAddr_; }
  unsigned get_size_cr() const { return trampSize_; }

  Address getAddress() const { return trampAddr_; }

  Address instAddr() const { return instAddr_; }
  unsigned instSize() const { return instSize_; }

  // If we're really out of ideas for getting to a multitramp. Ouch.
  bool usesTrap() const { return usesTrap_; }

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
  bool catchupRequired(Address pc, miniTramp *newMT, codeRange *range = NULL);

  process *proc() const;

  ///////////////// Generation
  bool generateCode(codeGen &gen,
                    Address baseInMutatee);

  unsigned maxSizeRequired();
  bool hasChanged();
  bool installCode();
  void invalidateCode();
  bool linkCode();
  void removeCode(generatedCodeObject *subObject);
    // STUPID WINDOWS COMPILER
  generatedCodeObject *replaceCode(generatedCodeObject *newParent);
  bool safeToFree(codeRange *range);
  void freeCode();

 private:
  Address instAddr_; 
  Address trampAddr_;  // Where we are
  unsigned trampSize_; // Size of the generated multiTramp
  unsigned instSize_; // Size of the original instrumented area

  int_function *func_;
  process *proc_;
  // A list of tramp structures we created a baseTramp stores
  // offsets for various points. We keep it separate to avoid clogging
  // up this structure.
  pdvector<generatedCodeObject *> deletedObjs;

  // A mini-CFG for code generation
  generatedCFG_t generatedCFG_;
  // And track all the instructions we cover.
  dictionary_hash<Address, relocatedInstruction *> insns_;
  void updateInsnDict();


  codeGen generatedMultiT_;
  codeGen jumpBuf_;

  // And what was there before we copied it.
  void *savedCodeBuf_;

  friend class instPointInstance;
  void addInstInstance(instPointInstance *instInstance);

  // Go and check to see if there are any new instInstances...
  void updateInstInstances();

  bool generateBranchToTramp(codeGen &gen);

  bool usesTrap_;
  bool canUseTrap_;
  bool changedSinceLastGeneration_;

  void setFirstInsn(generatedCodeObject *obj) { generatedCFG_.setStart(obj); }
};

// Funfun... we need to look up multiTramps by both their instrumentation range and 
// the instrumented range; but current codeRange can only do one. So we make a wrapper.

// This reports the instAddr_ and instSize_ members

class instArea : public codeRange {
 public:
    instArea(multiTramp *m) : multi(m) {}
    multiTramp *multi;
    Address get_address_cr() const { assert(multi); return multi->instAddr(); }
    unsigned get_size_cr() const { assert(multi); return multi->instSize(); }
};



/////////////////
// Sticking this here for now
class replacedFunctionCall {
 public:
    Address callAddr;
    codeGen newCall;
    codeGen oldCall;
};

#endif
