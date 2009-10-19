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

// $Id: baseTramp.h,v 1.24 2008/09/04 21:06:08 bill Exp $

// baseTramp class definition

#ifndef BASE_TRAMP_H
#define BASE_TRAMP_H

#include "common/h/Types.h"
#include "inst.h" // callWhen
#include "dyninstAPI/src/codeRange.h"
#include "instPoint.h"
#include "arch.h"
#include "multiTramp.h" // generatedCodeObject

class multiTramp;
class miniTramp;
class miniTrampInstance;
class baseTramp;
class instPointInstance;
class rpcMgr;

class generatedCodeObject;

// Todo: make things private/protected

// A baseTrampInstance represents a particular version of an
// overwritten instruction. Due to re+location and function cloning,
// there may be multiple baseTrampInstances for a particular logical
// baseTramp. This allows us to minimize the confusion of the
// necessary many:many mappings.

class baseTrampInstance : public generatedCodeObject { 
    friend class baseTramp;
 public:
    baseTrampInstance(baseTramp *tramp,
                      multiTramp *multi);
    // FORK!
    baseTrampInstance(const baseTrampInstance *pI,
                      baseTramp *cBT,
                      multiTramp *cMT,
                      process *child);


    Address trampAddr_;
    unsigned trampSize_;
    unsigned trampPostOffset;
    unsigned saveStartOffset;
    unsigned saveEndOffset;
    unsigned restoreStartOffset;
    unsigned restoreEndOffset;
    Address trampPreAddr() const;
    Address trampPostAddr() const;
    //bool empty;
    
    baseTramp *baseT;
    multiTramp *multiT;

    bool isInInstance(Address pc);
    bool isInInstru(Address pc);
    
    // codeRange...
    Address get_address() const;
    unsigned get_size() const; 
    void *getPtrToInstruction(Address addr) const;
    virtual std::string getTypeString() const;
    

    Address uninstrumentedAddr() const;

    AddressSpace *proc() const;
    
    unsigned maxSizeRequired();
    //const pdvector<instruction> &getCode();
    bool shouldGenerate();

    bool generateCode(codeGen &gen,
                      Address baseInMutatee,
                      UNW_INFO_TYPE * * unwindInformation);

    bool generateCodeInlined(codeGen &gen,
                             Address baseInMutatee,
                             UNW_INFO_TYPE **unwindInformation);

    bool installCode();

    // Displacement is platform-independent; from start
    // of branch insn to start of target insn.
    bool finalizeGuardBranch(codeGen &gen,
                             int displacement);

    
    void invalidateCode();
    
    // Patch in jumps
    bool linkCode();

    generatedCodeObject *replaceCode(generatedCodeObject *newParent);

    // The subObject wants to be gone, do we delete us as well?
    void removeCode(generatedCodeObject *subObject);

    // Given the current range, can we safely clean up this
    // area?
    bool safeToFree(codeRange *range);

    // Update the list of miniTrampInstances
    void updateMTInstances();

    // Null out an MTI pointer
    void deleteMTI(miniTrampInstance *mti);

    ~baseTrampInstance();

    int numDefinedRegs();
    Address miniTrampReturnAddr();

    bool isEmpty() const;

    void updateTrampCost(unsigned cost);

    instPoint *findInstPointByAddr(Address addr);

    bool shouldRegenBaseTramp(registerSpace *rs);

    unsigned genVersion;

    pdvector<miniTrampInstance *> mtis;

    // We need to keep these around to tell whether it's
    // safe to delete yet.
    pdvector<miniTrampInstance *> deletedMTIs;

    // We may remove ourselves from the baseTramp's list of instances
    // either directly (via removeCode) or during deletion -- but
    // don't do it twice!!!
    bool alreadyDeleted_;


    //Information about code generated in this tramp
 private:
    bool hasOptInfo_;
    bool spilledRegisters_;
    bool hasLocalSpace_;
    bool hasStackFrame_;
    bool flags_saved_;
    bool saved_fprs_;
    bool saved_orig_addr_;
 public:
    bitArray definedRegs;
    bool hasOptInfo() { return hasOptInfo_; } 
    bool spilledRegisters() { assert(hasOptInfo_); return spilledRegisters_; }
    bool hasLocalSpace() { return hasLocalSpace_; }
    bool hasStackFrame() { return hasStackFrame_; }
    bool flagsSaved() { return flags_saved_; }
    bool savedFPRs() { return saved_fprs_; }
    bool savedOrigAddr() { return saved_orig_addr_; }
    void setHasOptInfo(bool v) { hasOptInfo_ = v; } 
    void setSpilledRegisters(bool v) { spilledRegisters_ = v; }
    void setHasLocalSpace(bool v) { hasLocalSpace_ = v; }
    void setHasStackFrame(bool v) { hasStackFrame_ = v; }
    void setFlagsSaved(bool v) { flags_saved_ = v; }
    void setSavedFPRs(bool v) { saved_fprs_ = v; }
    void setSavedOrigAddr(bool v) { saved_orig_addr_ = v; }
};

class baseTramp {
    friend class baseTrampInstance;
 public:
#if defined( cap_unwind )
   unw_dyn_region_info_t * baseTrampRegion;
#endif
    Address origInstAddr(); // For faking an in-function address

    // Our instPoint
    instPoint *instP_;

    instPoint *instP() const { return instP_; }

    // You know, a conservative tramp is equivalent to an iRPC...
    rpcMgr *rpcMgr_;

    AddressSpace *proc() const;

    void invalidateBT() { valid = false; };

    bool doOptimizations();
    bool generateSaves(codeGen &,
                       registerSpace *,
                       baseTrampInstance *inst);
    bool generateRestores(codeGen &,
                          registerSpace *,
                          baseTrampInstance *inst);

    bool isConservative();
    bool isCallsite();
    bool isEntryExit();

    void setCreateFrame(bool frame);
    bool createFrame();

    unsigned getBTCost();

    bool inBasetramp( Address addr );
    bool inSavedRegion( Address addr );

    miniTramp *firstMini;
    miniTramp *lastMini;

    // Normal constructor
    baseTramp(instPoint *iP, callWhen when);
    // Fork constructor
    baseTramp(const baseTramp *parentT, process *proc);

    // destructor
    ~baseTramp();

    pdvector<baseTrampInstance *> instances;

    // Hooks the minitramp into the appropriate place in the chains 'o minis,
    // and sets the miniTramp next and prev members
    bool addMiniTramp(miniTramp *newMT, callOrder order);

    bool removeMiniTramp(miniTramp *oldMT);

    // Remove the instrumentation skip jumps if necessary
    bool clearBaseBranch();
    
    // Logic to correct all basetramp jumps.
    // Unused?
    //bool correctBTJumps();

    // If all our minitramps are gone, clean up
    void deleteIfEmpty();

    // Get an instance (takes a multiTramp, looks up 
    // or makes a new one);
    baseTrampInstance *findOrCreateInstance(multiTramp *multi);

    // Remove an instance
    void unregisterInstance(baseTrampInstance *inst);

    // Generated base tramp...
    // Recursively create miniTramps... as a result, we need to 
    // split the generated code for a baseT in half as there
    // may be an arbitrary amount of space in between.

    bool valid;
    bool optimized_out_guards;

    typedef enum {unset_BTR, recursive_BTR, guarded_BTR} guardState_t;
    guardState_t guardState_;

    bool suppress_threads_;
    bool threaded() const;
    void setThreaded(bool new_val);

    void setRecursive(bool trampRecursive, bool force = false);
    bool getRecursive() const;

    // Easier to do logic this way...
    bool guarded() const { return (guardState_ == guarded_BTR); }

 private:
    
    bool createFrame_;
    unsigned instVersion_;
    callWhen when_;
};

extern baseTramp baseTemplate;

#define X86_REGS_SAVE_LIMIT 3

#endif



