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

// $Id: baseTramp.h,v 1.8 2005/09/09 18:06:35 legendre Exp $

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
    unsigned trampPostOffset;
    Address trampPreAddr() const { return trampAddr_; }
    Address trampPostAddr() const { return trampPreAddr() + trampPostOffset; }
    //bool empty;
    
    baseTramp *baseT;
    multiTramp *multiT;

    bool isInInstance(Address pc);
    bool isInInstru(Address pc);
    
    // codeRange...
    Address get_address_cr() const { return trampPreAddr(); }
    unsigned get_size_cr() const; 
    void *getPtrToInstruction(Address addr) const;


    Address uninstrumentedAddr() const;

    process *proc() const;
    
    unsigned maxSizeRequired();
    //const pdvector<instruction> &getCode();
    bool shouldGenerate();

    bool generateCode(codeGen &gen,
                      Address baseInMutatee);

    bool installCode();

    // Displacement is platform-independent; from start
    // of branch insn to start of target insn.
    bool finalizeGuardBranch(codeGen &gen,
                             int displacement);

    // Have we changed (and do we need regeneration/
    // a new baseTrampInstance?)
    bool hasChanged();
    
    void invalidateCode();
    
    // Patch in jumps
    bool linkCode();

    generatedCodeObject *replaceCode(generatedCodeObject *newParent);

    // The subObject wants to be gone, do we delete us as well?
    void removeCode(generatedCodeObject *subObject);

    // Given the current range, can we safely clean up this
    // area?
    bool safeToFree(codeRange *range);

    // Utility function to drop a correct jump
    // from BT to MT into a buffer
    void generateBranchToMT(codeGen &gen);

    // Update the list of miniTrampInstances
    void updateMTInstances();

    // Null out an MTI pointer
    void deleteMTI(miniTrampInstance *mti);

    ~baseTrampInstance();

    Address miniTrampReturnAddr();

    bool isEmpty();

    void updateTrampCost(unsigned cost);

    instPoint *findInstPointByAddr(Address addr);

    unsigned genVersion;

    pdvector<miniTrampInstance *> mtis;

    // We need to keep these around to tell whether it's
    // safe to delete yet.
    pdvector<miniTrampInstance *> deletedMTIs;

    bool hadMini;
};

class baseTramp {
    friend class baseTrampInstance;
 public:
    unsigned preSize;
    unsigned postSize;

    unsigned saveStartOffset;
    unsigned saveEndOffset; // Last save instruction

    unsigned guardLoadOffset;
    unsigned guardBranchSize;

    unsigned costUpdateOffset;
    unsigned costSize;

    unsigned instStartOffset;
    unsigned instSize; // So we can correctly null out the jump if
    // there are no miniTramps

    // The following are offsets from bti->trampPostAddr
    unsigned restoreStartOffset;
    unsigned restoreEndOffset;

    unsigned trampEnd; // First non-BT instruction

    // For internal generation
    codeBufIndex_t guardBranchIndex;
    codeBufIndex_t costValueOffset; // If there are multiple insns, this
                              // does the math

    // Index in bt->postTramp_ codeGen
    codeBufIndex_t guardTargetIndex;


    int cost; // Current cost in cycles;
    Address costAddr; // For quick updates

    // FIXME
    int * clobberedGPR;
    int * clobberedFPR;
    int totalClobbered;

#if defined(arch_power)
    registerSpace *theRegSpace;
#endif

    // For flexible generation...
    Register trampGuardFlagAddr;
    Register trampGuardFlagValue;

    // And if we do unwind info
#if defined( arch_ia64 )
    unw_dyn_region_info_t * baseTrampRegion;
    Register addressRegister;
    Register valueRegister;
#endif

    Address origInstAddr(); // For faking an in-function address

    // Inst point for which we are pre-instrumentation
    instPoint *preInstP;
    // And post-instrumentation
    instPoint *postInstP;
    // You know, a conservative tramp is equivalent to an iRPC...
    rpcMgr *rpcMgr_;

    process *proc() const;

    bool generateBT();
    bool generateSaves(codeGen &gen,
                       registerSpace *rs = NULL);
    bool generateRestores(codeGen &gen,
                          registerSpace *rs = NULL);
    bool generateMTCode(codeGen &gen,
                        registerSpace *rs = NULL);
    // guardJumpOffset: internal variable, distance
    // from the start of the guard code to where the
    // jump is. Allows us to overwrite the jump later.
    bool generateGuardPreCode(codeGen &gen,
                              codeBufIndex_t &guardJumpIndex,
                              registerSpace *rs = NULL);
    bool generateGuardPostCode(codeGen &gen,
                               codeBufIndex_t &guardTargetIndex,
                               registerSpace *rs = NULL);
    // This isn't an index; we're writing straight into memory
    // when this gets fixed.
    bool generateCostCode(codeGen &gen,
                          unsigned &costUpdateOffset,
                          registerSpace *rs = NULL);

    bool isMerged;

    bool isConservative();
    bool isCallsite();
    bool isEntryExit();

    unsigned getBTCost();

    bool inBasetramp( Address addr );
    bool inSavedRegion( Address addr );

    miniTramp *firstMini;
    miniTramp *lastMini;

    // Normal constructor
    baseTramp();
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
    bool correctBTJumps();

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

    codeGen preTrampCode_;
    codeGen postTrampCode_;
    bool valid;

    typedef enum {unset_BTR, recursive_BTR, guarded_BTR} guardState_t;
    guardState_t guardState_;

    bool suppress_threads_;
    bool threaded() const;
    void setThreaded(bool new_val);

    void setRecursive(bool trampRecursive);
    bool getRecursive() const;

    // Return one or the other; undefined which one if both are set.
    // Returns null if an rpcMgr tramp
    instPoint *point() const;

    // Easier to do logic this way...
    bool guarded() const { return (guardState_ == guarded_BTR); }

 private:

    unsigned instVersion_;
};

extern baseTramp baseTemplate;

#endif



