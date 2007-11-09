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

// $Id: registerSpace.h,v 1.11 2007/11/09 20:11:03 bernat Exp $

#ifndef REGISTER_SPACE_H
#define REGISTER_SPACE_H

#include <stdio.h>
#include "common/h/Vector.h"
#include "common/h/Dictionary.h"
#include "common/h/String.h"
#include "common/h/Types.h"
#if defined(ia64_unknown_linux2_4)
#include "inst-ia64.h"
#endif

class codeGen;
class instPoint;
class process;
class AddressSpace;

// A class to retain information about where the original register can be found. It can be in one of the following states: 
// 1) Unsaved, and available via the register itself;
// 2) Saved in a frame, e.g., a base tramp;
// 3) Pushed on the stack at a relative offset from the current stack pointer.
// 4) TODO: we could subclass this and make "get me the current value" a member function; not sure it's really worth it for the minimal amount of memory multiple types will use.

// We also need a better way of tracking what state a register is in. Here's some possibilities, not at all mutually independent:

// Live at the start of instrumentation, or dead;
// Used during the generation of a subexpression
// Currently reserved by another AST, but we could recalculate if necessary
// At a function call node, is it carrying a value?

class registerSlot {
 public:
    Register number;    // what register is it
    int refCount;      	// == 0 if free
    bool needsSaving;	// Does this need to be saved at an emitted function call
    bool mustRestore;   // Did we tap this to be used in generated code (+ started live)
    bool startsLive;	// starts life as a live register; if dead, then we can freely use it. 
    bool beenClobbered; // Have we overwritten it (implying restore at end)
    // Unsure how beenClobbered is different from mustRestore...
	bool keptValue;     // Are we keeping this (as long as we can) to save
	                    // the pre-calculated value? Note: refCount can be 0 and
	                    // this still set.

    // Are we off limits for allocation in this particular instance?
    bool offLimits; 

    // New version of "if we were saved, then where?" It's a pair - true/false,
    // then offset from the "zeroed" stack pointer. 
    typedef enum { unspilled, stackPointer, framePointer } spillReference_t;
    spillReference_t origValueSpilled_;
    int saveOffset_; // Offset from base pointer or stack pointer
    
    typedef enum { invalid, GPR, FPR, SPR } regType_t;
    regType_t type_; 


    static registerSlot deadReg(Register number);
    static registerSlot liveReg(Register number);
    static registerSlot thrIndexReg(Register number);

    void cleanSlot();

    void resetSlot();

    registerSlot() : 
        number((Register) -1),
        refCount(0),
        needsSaving(false),
        mustRestore(false),
        startsLive(false),
        beenClobbered(false),
		keptValue(false),
        offLimits(false),
        origValueSpilled_(unspilled),
        saveOffset_(0),
        type_(invalid)
        {}

    registerSlot(const registerSlot &r) :
        number(r.number),
        refCount(r.refCount),
        needsSaving(r.needsSaving),
        mustRestore(r.mustRestore),
        startsLive(r.startsLive),
        beenClobbered(r.beenClobbered),
		keptValue(r.keptValue),
        offLimits(r.offLimits),
        origValueSpilled_(r.origValueSpilled_),
        saveOffset_(r.saveOffset_),
        type_(r.type_)
        {
        }
	void debugPrint(char *str = NULL);

};

class instPoint;

class registerSpace {
	friend void initRegisters();

 private:
	// A global mapping of register names to slots
	static registerSpace *globalRegSpace_;

	// Pre-set commonly used register spaces
	// Everyone live...
   static registerSpace *conservativeRegSpace_;
	// Function boundary...
	static registerSpace *optimisticRegSpace_;
	// And the one we should use.
	static registerSpace *actualRegSpace_;
	// Oh, and everything _dead_ for out-of-line BTs.
	static registerSpace *savedRegSpace_;

 public:
    // Legacy...
    //static registerSpace *regSpace();

   // Pre-set unknown register state:
	// Everything is live...
   static registerSpace *conservativeRegSpace(AddressSpace *proc);
   // Everything is dead...
   static registerSpace *optimisticRegSpace(AddressSpace *proc);
   // IRPC-specific - everything live for now
   static registerSpace *irpcRegSpace(AddressSpace *proc);
   // Aaand instPoint-specific
   static registerSpace *actualRegSpace(instPoint *iP);
   // DO NOT DELETE THESE. 
   static registerSpace *savedRegSpace(AddressSpace *proc);

   registerSpace();

	registerSpace(const registerSpace &);
	// Specialize a register space given a particular list of
	// dead registers (everyone else is assumed live);
	// Returns a copy.
	static registerSpace *specializeRegisterSpace(Register *deadRegs,
												  const unsigned int numDead);
	static registerSpace *createAllLive(Register *liveRegs,
										const unsigned int liveCount);
	static registerSpace *createAllDead(Register *deadRegs,
										const unsigned int deadCount);

    // Inits the values for the clobbered variables for the floating point registers
    void initFloatingPointRegisters(const unsigned int count, Register *fp);

    // Inits the values for any tracked SPRegisters
    // Platform specific
    void initSpecialPurposeRegisters();
    
    ~registerSpace();

    // Read the value in register souce from wherever we've stored it in
    // memory (including the register itself), and stick it in actual register
    // destination. So the source is the label, and destination is an actual.
    bool readRegister(codeGen &gen, Register source, Register destination);

    Register allocateRegister(codeGen &gen, bool noCost);
    bool allocateSpecificRegister(codeGen &gen, Register r, bool noCost);


    // Like allocate, but don't keep it around; if someone else tries to
    // allocate they might get this one. 
    Register getScratchRegister(codeGen &gen, bool noCost = true); 
    // Like the above, but excluding a set of registers (that we don't want
    // to touch)
    Register getScratchRegister(codeGen &gen, pdvector<Register> &excluded, bool noCost = true);

    bool saveAllRegisters(codeGen &gen, bool noCost);
    bool restoreAllRegisters(codeGen &gen, bool noCost);

    // For now, we save registers elsewhere and mark them here. 
    bool markSavedRegister(Register num, int offsetFromSP);
    // 
    bool markKeptRegister(Register num);

    // Things that will be modified implicitly by anything else we
    // generate - condition registers, etc.
    bool saveVolatileRegisters(codeGen &gen);
    bool restoreVolatileRegisters(codeGen &gen);

    // Free the specified register (decrement its refCount)
    void freeRegister(Register k);
    // Free the register even if its refCount is greater that 1
    void forceFreeRegister(Register k);
	// And mark a register as not being kept any more
	void unKeepRegister(Register k);
	// Set a registerSpace back to defaults; equivalent to allocating a new
	// one
    void resetSpace();
	// Mark all registers as unallocated, but keep live/dead info
	void cleanSpace();
	
    void resetClobbers();
    void saveClobberInfo(const instPoint * location);
    void resetLiveDeadInfo(const int* liveRegs,
                           const int *, 
                           const int *);
    
    
    // Check to see if the register is free
    bool isFreeRegister(Register k);
    
    //Makes register unClobbered
    void unClobberRegister(Register reg);
    void unClobberFPRegister(Register reg);
    
    // Checks to see if register has been clobbered and clobbers it 
    // if it hasn't been clobbered yet, returns true if we clobber it
    // false if it has already been clobbered
    bool clobberRegister(Register reg);
    bool clobberFPRegister(Register reg);

    // The efficient mass-production version of the above. Sets all registers
    // to "clobbered"; that is, used in a function call and in need of saving
    // (if it's live).
    bool clobberAllRegisters();
    
    // Checks to see if given register has been clobbered, true if it has
    bool beenSaved(Register reg);
    bool beenSavedFP(Register reg);
    
    // Checks to see if register starts live
    bool isRegStartsLive(Register reg);
    int fillDeadRegs(Register * deadRegs, int num);
    
    // Manually set the reference count of the specified register
    // we need to do so when reusing an already-allocated register
    void fixRefCount(Register k, int iRefCount);
    
    // Bump up the reference count. Occasionally, we underestimate it
    // and call this routine to correct this.
    void incRefCount(Register k);
    
    u_int getRegisterCount() { return registers.size(); }
    u_int getFPRegisterCount() { return fpRegisters.size(); }
    
    registerSlot *getRegSlot(Register k) { return (&registers[k]); }
    registerSlot *getFPRegSlot(Register k) { return (&fpRegisters[k]); }

    enum saveState_t { unknown, killed, unused, live, dead};
    
    // A bit of logic... for now, "unknown" is optimistic.
    bool saveAllGPRs() const; 
    bool saveAllFPRs() const; 
    bool saveAllSPRs() const;
    
    void setSaveAllGPRs(bool val) { saveAllGPRs_ = val ? killed : unused; } 
    void setSaveAllFPRs(bool val) { saveAllFPRs_ = val ? killed : unused; } 
    void setSaveAllSPRs(bool val) { saveAllSPRs_ = val ? killed : unused; } 
    
    void copyInfo(registerSpace *rs) const;

    
    // Reset when the regSpace is reset - marked offlimits for 
    // allocation.
    bool markReadOnly(Register k);
    bool readOnlyRegister(Register k);
    // Make sure that no registers remain allocated, except "to_exclude"
    // Used for assertion checking.
    void checkLeaks(Register to_exclude);
    
    registerSpace &operator=(const registerSpace &src);

    void debugPrint();
    void printAllocedRegisters();

 private:

    unsigned GPRindex(unsigned index) const { return index; }
    unsigned FPRindex(unsigned index) const { return index+registers.size(); }
    unsigned SPRindex(unsigned index) const { return index+registers.size()+fpRegisters.size(); }
    
    registerSlot &getRegisterSlot(unsigned index);

    registerSlot *findRegister(Register reg); 

    bool spillRegister(unsigned index, codeGen &gen, bool noCost);
    bool stealRegister(unsigned index, codeGen &gen, bool noCost);

    bool restoreRegister(unsigned index, codeGen &gen, bool noCost); 
	bool popRegister(unsigned index, codeGen &gen, bool noCost);
    
    // Track available registers
    pdvector<registerSlot> registers;
    pdvector<registerSlot> fpRegisters;
    pdvector<registerSlot> spRegisters;
    
    int currStackPointer; 

    // And track what needs to be saved...
    saveState_t saveAllGPRs_;
    saveState_t saveAllFPRs_;
    saveState_t saveAllSPRs_;

 public:
    static bool hasXMM;  // for Intel architectures, XMM registers
    
    
#if defined(ia64_unknown_linux2_4)
    
 public:
    int originalLocals;
    int originalOutputs;
    int originalRotates;
    int sizeOfStack;
    
    // storageMap[] needs to be of type 'int' as opposed to
    // 'Register' becuase negative values may be used.
    int storageMap[ BP_R_MAX ];
#endif
    
};

#endif
