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

// $Id: registerSpace.h,v 1.18 2008/06/19 19:53:37 legendre Exp $

#ifndef REGISTER_SPACE_H
#define REGISTER_SPACE_H

#include <stdio.h>
#include <string>
#include <map>
#include <set>
#include "common/h/Vector.h"
#include "common/h/Dictionary.h"
#include "common/h/Types.h"
#include "inst.h" // callWhen...

#if defined(cap_liveness)
#include "bitArray.h"
#endif

class codeGen;
class instPoint;
class process;
class AddressSpace;
class image_basicBlock;

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

// Terminology:
// "Live" : contains a value outside of instrumentation, and so must be saved before use
// "Used" : used by instrumentation code. 

class RealRegister {
   //This is currently only used on x86_32 to represent the 
   // virtual/real register difference.  'Register' still refers
   // to virtual registers on this platform.  Contained in a struct
   // so that no one can accidently cast a Register into a RealRegister
   friend class registerSpace;
   signed int r;
 public:
   RealRegister() { r = 0; }
   explicit RealRegister(int reg) { r = reg; }
   int reg() const { return r; }
};

#if defined(arch_x86_64)
#include "inst-x86.h"
#endif

class registerSlot {
 public:
   int alloc_num; //MATT TODO: Remove
    const Register number;    // what register is it, using our Register enum
    const std::string name;

    typedef enum { deadAlways, deadABI, liveAlways } initialLiveness_t;
    const initialLiveness_t initialState;

    // Are we off limits for allocation in this particular instance?
    const bool offLimits; 

    typedef enum { invalid, GPR, FPR, SPR, realReg} regType_t;
    const regType_t type; 

    ////////// Code generation

    int refCount;      	// == 0 if free

    typedef enum { live, spilled, dead } livenessState_t;
    livenessState_t liveState;

    bool keptValue;     // Are we keeping this (as long as we can) to save
    // the pre-calculated value? Note: refCount can be 0 and
    // this still set.

    bool beenUsed;      // Has this register been used by generated code?

    // New version of "if we were saved, then where?" It's a pair - true/false,
    // then offset from the "zeroed" stack pointer. 
    typedef enum { unspilled, framePointer } spillReference_t;
    spillReference_t spilledState;
    int saveOffset; // Offset where this register can be
                    // retrieved.
    // AMD-64: this is the number of words
    // POWER: this is the number of bytes
    // I know it's inconsistent, but it's easier this way since POWER
    // has some funky math.

    //////// Member functions

    unsigned encoding() const;

    void cleanSlot();

    void markUsed(bool incRefCount) {
        assert(offLimits == false);
        assert(refCount == 0);
        assert(liveState != live);
        
        if (incRefCount) 
            refCount = 1;
        beenUsed = true;
    }
    
    // Default is just fine
    // registerSlot(const registerSlot &r)

    void debugPrint(const char *str = NULL);

    // Don't want to use this...
    registerSlot() :
       alloc_num(0),
        number(REG_NULL),
        name("DEFAULT REGISTER"),
        initialState(deadAlways),
        offLimits(true),
        type(invalid)
        {};

    registerSlot(Register num,
                 std::string name_,
                 bool offLimits_,
                 initialLiveness_t initial,
                 regType_t type_) : 
       alloc_num(0),
        number(num),
        name(name_),
        initialState(initial),
        offLimits(offLimits_),
        type(type_),
        refCount(0),
        liveState(live),
        keptValue(false),
        beenUsed(false),
        spilledState(unspilled),
        saveOffset(-1) {}

};

class instPoint;

typedef struct {
   bool is_allocatable;
   bool been_used;
   int last_used;
   registerSlot *contains;
} RealRegsState;


class regState_t {
 public:
   regState_t();
   int pc_rel_offset;
   int timeline;
   int stack_height;
   std::vector<RealRegsState> registerStates;
};

class registerSpace {
   friend class baseTrampInstance;
 private:
    // A global mapping of register names to slots
    static registerSpace *globalRegSpace_;
    static registerSpace *globalRegSpace64_;

    static void createRegSpaceInt(pdvector<registerSlot *> &regs,
                                  registerSpace *regSpace);

 public:
    // Pre-set unknown register state:
    // Everything is live...
    static registerSpace *conservativeRegSpace(AddressSpace *proc);
    // Everything is dead...
    static registerSpace *optimisticRegSpace(AddressSpace *proc);
    // IRPC-specific - everything live for now
    static registerSpace *irpcRegSpace(AddressSpace *proc);
    // Aaand instPoint-specific
    static registerSpace *actualRegSpace(instPoint *iP, callWhen location);
    // DO NOT DELETE THESE. 
    static registerSpace *savedRegSpace(AddressSpace *proc);

    static registerSpace *getRegisterSpace(AddressSpace *proc);
    static registerSpace *getRegisterSpace(unsigned addr_width);
    
    registerSpace();
    
    static void createRegisterSpace(pdvector<registerSlot *> &registers);
    static void createRegisterSpace64(pdvector<registerSlot *> &registers);
    
    ~registerSpace();

    // IA-64... it overrides the register space with particular register
    // numbers based on its rotating window mechanism.
    // Note: this screws with the default registerSpace, and so 
    // _really_ needs to be used continually or not at all.
    static void overwriteRegisterSpace(unsigned firstReg, unsigned lastReg);
    static void overwriteRegisterSpace64(unsigned firstReg, unsigned lastReg);

    // Read the value in register souce from wherever we've stored it in
    // memory (including the register itself), and stick it in actual register
    // destination. So the source is the label, and destination is an actual.
    // Size is a legacy parameter for places where we don't have register information
    // (SPARC/IA-64)
    bool readProgramRegister(codeGen &gen, Register source, 
                             Register destination,
                             unsigned size);

    // And the reverse
    bool writeProgramRegister(codeGen &gen, Register destination, 
                              Register source,
                              unsigned size);


    Register allocateRegister(codeGen &gen, bool noCost);
    bool allocateSpecificRegister(codeGen &gen, Register r, bool noCost = true);


    // Like allocate, but don't keep it around; if someone else tries to
    // allocate they might get this one. 
    Register getScratchRegister(codeGen &gen, bool noCost = true); 
    // Like the above, but excluding a set of registers (that we don't want
    // to touch)
    Register getScratchRegister(codeGen &gen, pdvector<Register> &excluded, bool noCost = true);


    bool trySpecificRegister(codeGen &gen, Register reg, bool noCost = true);

    bool saveAllRegisters(codeGen &gen, bool noCost);
    bool restoreAllRegisters(codeGen &gen, bool noCost);

    // For now, we save registers elsewhere and mark them here. 
    bool markSavedRegister(Register num, int offsetFromFP);
    bool markSavedRegister(RealRegister num, int offsetFromFP);

    // 
    bool markKeptRegister(Register num);

    // Things that will be modified implicitly by anything else we
    // generate - condition registers, etc.
    bool checkVolatileRegisters(codeGen &gen, registerSlot::livenessState_t);
    bool saveVolatileRegisters(codeGen &gen);
    bool restoreVolatileRegisters(codeGen &gen);

    // Free the specified register (decrement its refCount)
    void freeRegister(Register k);
    // Free the register even if its refCount is greater that 1
    void forceFreeRegister(Register k);
    // And mark a register as not being kept any more
    void unKeepRegister(Register k);


    // Mark all registers as unallocated, but keep live/dead info
    void cleanSpace();
    
    // Check to see if the register is free
    // DO NOT USE THIS!!!! to tell if you can use a register as
    // a scratch register; do that with trySpecificRegister
    // or allocateSpecificRegister. This is _ONLY_ to determine
    // if a register should be saved (e.g., over a call).
    bool isFreeRegister(Register k);
    
    // Checks to see if register starts live
    bool isRegStartsLive(Register reg);
    int fillDeadRegs(Register * deadRegs, int num);
    
    // Bump up the reference count. Occasionally, we underestimate it
    // and call this routine to correct this.
    void incRefCount(Register k);
    
    // Reset when the regSpace is reset - marked offlimits for 
    // allocation.
    bool markReadOnly(Register k);
    bool readOnlyRegister(Register k);
    // Make sure that no registers remain allocated, except "to_exclude"
    // Used for assertion checking.
    void checkLeaks(Register to_exclude);
    
    int getAddressWidth() { return addr_width; }
    void debugPrint();
    void printAllocedRegisters();

    int numGPRs() const { return GPRs_.size(); }
    int numFPRs() const { return FPRs_.size(); }
    int numSPRs() const { return SPRs_.size(); }
    int numRegisters() const { return registers_.size(); }

    pdvector <registerSlot *> &GPRs() { return GPRs_; }
    pdvector <registerSlot *> &FPRs() { return FPRs_; }
    pdvector <registerSlot *> &SPRs() { return SPRs_; }

    pdvector <registerSlot *> &realRegs();

    pdvector <registerSlot *> &trampRegs(); //realRegs() on x86-32, GPRs on all others

    registerSlot *operator[](Register);

    // For platforms with "save all" semantics...
    bool anyLiveGPRsAtEntry() const;
    bool anyLiveFPRsAtEntry() const;
    bool anyLiveSPRsAtEntry() const;

    // And for the bitarrays we use to track these
    static bitArray getBitArray();

    /**
     * The following set of 'public' and 'private' methods and data deal with
     * virtual registers, currently used only on x86.  The above 'Register' class
     * allocates and uses virtual registers, these methods provide mappings from 
     * virtual registers to real registers.
     **/
 public:
    //Put VReg into RReg
    RealRegister loadVirtual(registerSlot *virt_r, codeGen &gen); 
    RealRegister loadVirtual(Register virt_r, codeGen &gen);

    //Put VReg into specific real register
    void loadVirtualToSpecific(registerSlot *virt_r, RealRegister real_r, codeGen &gen);
    void loadVirtualToSpecific(Register virt_r, RealRegister real_r, codeGen &gen);

    //Spill away any virtual register in a real so that the real
    // can be used freely.  Careful with this, no guarentee it won't
    // be reallocated in the next step.
    void makeRegisterAvail(RealRegister r, codeGen &gen);

    //Tell the tracker that we've manually put some virtual into a real
    void noteVirtualInReal(Register v_r, RealRegister r_r);
    void noteVirtualInReal(registerSlot *v_r, RealRegister r_r);

    //Like loadVirtual, but don't load orig value first
    RealRegister loadVirtualForWrite(Register virt_r, codeGen &gen);
    RealRegister loadVirtualForWrite(registerSlot *virt_r, codeGen &gen);

    void markVirtualDead(Register num);
    bool spilledAnything();

    Register pc_rel_reg;
    int pc_rel_use_count;
    int& pc_rel_offset();
    void incStack(int val);
    int getInstFrameSize();
    void setInstFrameSize(int val);
    int getStackHeight();
    void setStackHeight(int val);

    void unifyTopRegStates(codeGen &gen);
    void pushNewRegState();

 private:
    int instFrameSize_;  // How much stack space we allocate for
                         // instrumentation before a frame is set up.

    std::vector<regState_t *> regStateStack;
    int cur_register_state;

    std::vector<RealRegsState>& regState();
    int& timeline();

    std::set<registerSlot *> regs_been_spilled;

    void initRealRegSpace();

    //High-level functions that track data structures and call code gen
    RealRegister findReal(registerSlot *virt_r, bool &already_setup);
    void spillReal(RealRegister r, codeGen &gen);
    void loadReal(RealRegister r, registerSlot *v_r, codeGen &gen);
    void freeReal(RealRegister r);

    //low-level functions for code gen
    void spillToVReg(RealRegister reg, registerSlot *v_reg, codeGen &gen);
    void movVRegToReal(registerSlot *v_reg, RealRegister r, codeGen &gen); 
    void movRegToReg(RealRegister dest, RealRegister src, codeGen &gen);

    unsigned savedFlagSize;

 private:

    registerSpace(const registerSpace &);
    
    registerSlot &getRegisterSlot(Register reg);

    registerSlot *findRegister(Register reg); 
    registerSlot *findRegister(RealRegister reg);

    bool spillRegister(Register reg, codeGen &gen, bool noCost);
    bool stealRegister(Register reg, codeGen &gen, bool noCost);

    bool restoreRegister(Register reg, codeGen &gen, bool noCost); 
    bool popRegister(Register reg, codeGen &gen, bool noCost);

    bool markSavedRegister(registerSlot *num, int offsetFromFP);
    
    int currStackPointer; 

    typedef dictionary_hash_iter<Register, registerSlot *> regDictIter;
    dictionary_hash<Register, registerSlot *> registers_;

    // And convenience vectors
    pdvector<registerSlot *> GPRs_;
    pdvector<registerSlot *> FPRs_;
    pdvector<registerSlot *> SPRs_;

    // Used on platforms that have "virtual" registers to provide a mapping
    // for real (e.g., architectural) registers
    pdvector<registerSlot *> realRegisters_;

    static void initialize();
    static void initialize32();
    static void initialize64();


    registerSpace &operator=(const registerSpace &src);

    typedef enum {arbitrary, ABI_boundary, allSaved} rs_location_t;

    // Specialize liveness as represented by a bit array
    void specializeSpace(rs_location_t state);

#if defined(cap_liveness)
    void specializeSpace(const bitArray &);
#endif

    unsigned addr_width;

 public:
    static bool hasXMM;  // for Intel architectures, XMM registers
    
 public:
#if defined(arch_power)
    typedef enum { r0, r1, r2, r3, r4, r5, r6, r7, r8, r9, r10, r11, r12,
                   r13, r14, r15, r16, r17, r18, r19, r20, r21, r22, r23,
                   r24, r25, r26, r27, r28, r29, r30, r31,
                   fpr0, fpr1, fpr2, fpr3, fpr4, fpr5, fpr6, 
                   fpr7, fpr8, fpr9, fpr10, fpr11, fpr12, fpr13,
                   fpr14, fpr15, fpr16, fpr17, fpr18, fpr19, fpr20,
                   fpr21, fpr22, fpr23, fpr24, fpr25, fpr26, fpr27,
                   fpr28, fpr29, fpr30, fpr31,
                   xer, lr, ctr, mq, cr, lastReg, ignored } powerRegisters_t;
    static unsigned GPR(Register x) { return x; }
    static unsigned FPR(Register x) { return x - fpr0; }
    static unsigned SPR(Register x);
    int framePointer() { return r1; }
#endif
#if defined(arch_x86) || defined(arch_x86_64)
    int framePointer();
#endif
    // Create a map of register names to register numbers
    std::map<std::string, Register> registersByName;
    // The reverse map can be handled by doing a rs[x]->name

    Register getRegByName(const std::string name);
    std::string getRegByNumber(Register num);
    void getAllRegisterNames(std::vector<std::string> &ret);

    // Bit vectors that represent the ABI behavior at call points
    // and exits. 

#if defined(cap_liveness)

    const bitArray &getCallReadRegisters() const;
    const bitArray &getCallWrittenRegisters() const;
    const bitArray &getReturnReadRegisters() const;
    // No such thing as return written...

    // Syscall!
    const bitArray &getSyscallReadRegisters() const;
    const bitArray &getSyscallWrittenRegisters() const;

    const bitArray &getAllRegs() const;
 private:
    static bitArray callRead_;
    static bitArray callRead64_;

    static bitArray callWritten_;
    static bitArray callWritten64_;

    static bitArray returnRead_;
    static bitArray returnRead64_;

    static bitArray syscallRead_;
    static bitArray syscallRead64_;

    static bitArray syscallWritten_;
    static bitArray syscallWritten64_;

    static bitArray allRegs_;
    static bitArray allRegs64_;

#endif


};

#endif
