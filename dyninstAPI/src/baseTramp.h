/*
 * Copyright (c) 1996-2009 Barton P. Miller
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

// $Id: baseTramp.h,v 1.24 2008/09/04 21:06:08 bill Exp $

// baseTramp class definition

#ifndef BASE_TRAMP_H
#define BASE_TRAMP_H

#include "common/h/Types.h"
#include "inst.h" // callWhen
#include "dyninstAPI/src/codeRange.h"
#include "instPoint.h"
//#include "arch.h"
#include "ast.h"

#include <list>

class miniTramp;
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

class baseTrampInstance { 
    friend class baseTramp;
 public:
    baseTrampInstance(baseTramp *tramp);

    // FORK!
    baseTrampInstance(const baseTrampInstance *pI,
                      baseTramp *cBT,
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

    bool isInInstance(Address pc);
    bool isInInstru(Address pc);
    
    // codeRange...
    Address get_address() const;
    unsigned get_size() const; 
    void *getPtrToInstruction(Address addr) const;
    std::string getTypeString() const;
    

    Address uninstrumentedAddr() const;

    AddressSpace *proc() const;
    
    unsigned maxSizeRequired();
    //const pdvector<instruction> &getCode();
    bool shouldGenerate();

    bool generateCode(codeGen &gen,
                      Address baseInMutatee);

    bool generateCodeInlined(codeGen &gen,
                             Address baseInMutatee);

    cfjRet_t checkForFuncJumps();

    ~baseTrampInstance();

    int numDefinedRegs();
    Address miniTrampReturnAddr();

    bool isEmpty() const;

    void updateTrampCost(unsigned cost);

    instPoint *findInstPointByAddr(Address addr);

    bool shouldRegenBaseTramp(registerSpace *rs);

    unsigned genVersion;

    // We may remove ourselves from the baseTramp's list of instances
    // either directly (via removeCode) or during deletion -- but
    // don't do it twice!!!
    bool alreadyDeleted_;

    SymtabAPI::Symbol *createBTSymbol();
 private:
    //Information about code generated in this tramp
    bool hasOptInfo_;
    bool spilledRegisters_;
    bool hasLocalSpace_;
    bool hasStackFrame_;
    bool flags_saved_;
    bool saved_fprs_;
    bool saved_orig_addr_;
    cfjRet_t hasFuncJump_;
    int trampStackHeight_;
 public:
    bitArray definedRegs;
    bool hasOptInfo() { return hasOptInfo_; } 
    bool spilledRegisters() { assert(hasOptInfo_); return spilledRegisters_; }
    bool hasLocalSpace() { return hasLocalSpace_; }
    bool hasStackFrame() { return hasStackFrame_; }
    bool flagsSaved() { return flags_saved_; }
    bool savedFPRs() { return saved_fprs_; }
    bool savedOrigAddr() { return saved_orig_addr_; }
    cfjRet_t hasFuncJump() { return hasFuncJump_; }
    int trampStackHeight() { return trampStackHeight_; }
    void setHasOptInfo(bool v) { hasOptInfo_ = v; } 
    void setSpilledRegisters(bool v) { spilledRegisters_ = v; }
    void setHasLocalSpace(bool v) { hasLocalSpace_ = v; }
    void setHasStackFrame(bool v) { hasStackFrame_ = v; }
    void setFlagsSaved(bool v) { flags_saved_ = v; }
    void setSavedFPRs(bool v) { saved_fprs_ = v; }
    void setSavedOrigAddr(bool v) { saved_orig_addr_ = v; }
    void setHasFuncJump(cfjRet_t v) { hasFuncJump_ = v; }
    void setTrampStackHeight(int v) { trampStackHeight_ = v; }
    int funcJumpSlotSize();

    int_function *func() const;
};

class baseTramp {
    friend class baseTrampInstance;
    friend class miniTramp;
 public:
    typedef std::list<miniTramp *>::iterator iterator;
    typedef std::list<miniTramp *>::const_iterator const_iterator;
    iterator begin() { return miniTramps_.begin(); };
    iterator end() { return miniTramps_.end(); };
    const_iterator begin() const { return miniTramps_.begin(); };
    const_iterator end() const { return miniTramps_.end(); };

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


    bool empty() const { return miniTramps_.empty(); };

    bool wasNonEmpty() const { return wasNonEmpty_; };

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

    // tells us how far up the stack to look for
    // saved register addresses 
    unsigned savedFlagSize;

    callWhen when() const { return when_; }

 private:
    
    bool createFrame_;
    callWhen when_;
    bool wasNonEmpty_; 

    std::list<miniTramp *> miniTramps_;
};

extern baseTramp baseTemplate;

#define X86_REGS_SAVE_LIMIT 3

#endif



