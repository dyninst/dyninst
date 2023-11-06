/*
 * See the dyninst/COPYRIGHT file for copyright information.
 *
 * We provide the Paradyn Tools (below described as "Paradyn")
 * on an AS IS basis, and do not warrant its validity or performance.
 * We reserve the right to update, modify, or discontinue this
 * software at any time.  We shall have no obligation to supply such
 * updates or modifications or any other form of support to you.
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
#include <assert.h>
#include <vector>
#include <map>
#include <set>
#include <unordered_map>
#include "dyn_register.h"
#include "inst.h" // callWhen...

#include "bitArray.h"

class codeGen;
class instPoint;
class AddressSpace;
class parse_block;
class baseTramp;

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
   // virtual/real register difference.  'Dyninst::Register' still refers
   // to virtual registers on this platform.  Contained in a struct
   // so that no one can accidently cast a Dyninst::Register into a RealRegister
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
    const Dyninst::Register number;    // what register is it, using our Dyninst::Register enum
    const std::string name;

    typedef enum { deadAlways, deadABI, liveAlways } initialLiveness_t;
    const initialLiveness_t initialState;

    // Are we off limits for allocation in this particular instance?
    bool offLimits;

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
        number(Dyninst::Null_Register),
        name("DEFAULT REGISTER"),
        initialState(deadAlways),
        offLimits(true),
        type(invalid),
        refCount(0),
        liveState(live),
        keptValue(false),
        beenUsed(false),
        spilledState(unspilled),
        saveOffset(-1)
        {}

    registerSlot(Dyninst::Register num,
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
   friend class baseTramp;
 private:
    // A global mapping of register names to slots
    static registerSpace *globalRegSpace_;
    static registerSpace *globalRegSpace64_;

    static void createRegSpaceInt(std::vector<registerSlot *> &regs,
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
    static registerSpace *actualRegSpace(instPoint *iP);
    // DO NOT DELETE THESE.
    static registerSpace *savedRegSpace(AddressSpace *proc);

    static registerSpace *getRegisterSpace(AddressSpace *proc);
    static registerSpace *getRegisterSpace(unsigned addr_width);

    registerSpace();

    static void createRegisterSpace(std::vector<registerSlot *> &registers);
    static void createRegisterSpace64(std::vector<registerSlot *> &registers);

    ~registerSpace();

    // Read the value in register souce from wherever we've stored it in
    // memory (including the register itself), and stick it in actual register
    // destination. So the source is the label, and destination is an actual.
    // Size is a legacy parameter for places where we don't have register information
    bool readProgramRegister(codeGen &gen, Dyninst::Register source,
                             Dyninst::Register destination,
                             unsigned size);

    // And the reverse
    bool writeProgramRegister(codeGen &gen, Dyninst::Register destination,
                              Dyninst::Register source,
                              unsigned size);


    Dyninst::Register allocateRegister(codeGen &gen, bool noCost, bool realReg = false);
    bool allocateSpecificRegister(codeGen &gen, Dyninst::Register r, bool noCost = true);


    // Like allocate, but don't keep it around; if someone else tries to
    // allocate they might get this one.
    Dyninst::Register getScratchRegister(codeGen &gen, bool noCost = true, bool realReg = false);
    // Like the above, but excluding a set of registers (that we don't want
    // to touch)
    Dyninst::Register getScratchRegister(codeGen &gen, std::vector<Dyninst::Register> &excluded, bool noCost = true, bool realReg = false);


    bool trySpecificRegister(codeGen &gen, Dyninst::Register reg, bool noCost = true);

    bool saveAllRegisters(codeGen &gen, bool noCost);
    bool restoreAllRegisters(codeGen &gen, bool noCost);

    // For now, we save registers elsewhere and mark them here.
    bool markSavedRegister(Dyninst::Register num, int offsetFromFP);
    bool markSavedRegister(RealRegister num, int offsetFromFP);

    //
    bool markKeptRegister(Dyninst::Register num);

    // Things that will be modified implicitly by anything else we
    // generate - condition registers, etc.
    bool checkVolatileRegisters(codeGen &gen, registerSlot::livenessState_t);
    bool saveVolatileRegisters(codeGen &gen);
    bool restoreVolatileRegisters(codeGen &gen);

    // Free the specified register (decrement its refCount)
    void freeRegister(Dyninst::Register k);
    // Free the register even if its refCount is greater that 1
    void forceFreeRegister(Dyninst::Register k);
    // And mark a register as not being kept any more
    void unKeepRegister(Dyninst::Register k);


    // Mark all registers as unallocated, but keep live/dead info
    void cleanSpace();

    // Check to see if the register is free
    // DO NOT USE THIS!!!! to tell if you can use a register as
    // a scratch register; do that with trySpecificRegister
    // or allocateSpecificRegister. This is _ONLY_ to determine
    // if a register should be saved (e.g., over a call).
    bool isFreeRegister(Dyninst::Register k);

    // Checks to see if register starts live
    bool isRegStartsLive(Dyninst::Register reg);
    int fillDeadRegs(Dyninst::Register * deadRegs, int num);

    // Bump up the reference count. Occasionally, we underestimate it
    // and call this routine to correct this.
    void incRefCount(Dyninst::Register k);

    // Reset when the regSpace is reset - marked offlimits for
    // allocation.
    bool markReadOnly(Dyninst::Register k);
    bool readOnlyRegister(Dyninst::Register k);
    // Make sure that no registers remain allocated, except "to_exclude"
    // Used for assertion checking.
    void checkLeaks(Dyninst::Register to_exclude);

    int getAddressWidth() { return addr_width; }
    void debugPrint();
    void printAllocedRegisters();

    int numGPRs() const { return GPRs_.size(); }
    int numFPRs() const { return FPRs_.size(); }
    int numSPRs() const { return SPRs_.size(); }
    int numRegisters() const { return registers_.size(); }

    std::vector <registerSlot *> &GPRs() { return GPRs_; }
    std::vector <registerSlot *> &FPRs() { return FPRs_; }
    std::vector <registerSlot *> &SPRs() { return SPRs_; }

    std::vector <registerSlot *> &realRegs();

    std::vector <registerSlot *> &trampRegs(); //realRegs() on x86-32, GPRs on all others

    registerSlot *physicalRegs(Dyninst::Register reg) { return physicalRegisters_[reg]; }

    registerSlot *operator[](Dyninst::Register);

    // For platforms with "save all" semantics...
    bool anyLiveGPRsAtEntry() const;
    bool anyLiveFPRsAtEntry() const;
    bool anyLiveSPRsAtEntry() const;


    /**
     * The following set of 'public' and 'private' methods and data deal with
     * virtual registers, currently used only on x86.  The above 'Dyninst::Register' class
     * allocates and uses virtual registers, these methods provide mappings from
     * virtual registers to real registers.
     **/
 public:
    //Put VReg into RReg
    RealRegister loadVirtual(registerSlot *virt_r, codeGen &gen);
    RealRegister loadVirtual(Dyninst::Register virt_r, codeGen &gen);

    //Put VReg into specific real register
    void loadVirtualToSpecific(registerSlot *virt_r, RealRegister real_r, codeGen &gen);
    void loadVirtualToSpecific(Dyninst::Register virt_r, RealRegister real_r, codeGen &gen);

    //Spill away any virtual register in a real so that the real
    // can be used freely.  Careful with this, no guarentee it won't
    // be reallocated in the next step.
    void makeRegisterAvail(RealRegister r, codeGen &gen);

    //Tell the tracker that we've manually put some virtual into a real
    void noteVirtualInReal(Dyninst::Register v_r, RealRegister r_r);
    void noteVirtualInReal(registerSlot *v_r, RealRegister r_r);

    //Like loadVirtual, but don't load orig value first
    RealRegister loadVirtualForWrite(Dyninst::Register virt_r, codeGen &gen);
    RealRegister loadVirtualForWrite(registerSlot *virt_r, codeGen &gen);

    void markVirtualDead(Dyninst::Register num);
    bool spilledAnything();

    Dyninst::Register pc_rel_reg;
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

    registerSlot &getRegisterSlot(Dyninst::Register reg);

    registerSlot *findRegister(Dyninst::Register reg);
    registerSlot *findRegister(RealRegister reg);

    bool spillRegister(Dyninst::Register reg, codeGen &gen, bool noCost);
    bool stealRegister(Dyninst::Register reg, codeGen &gen, bool noCost);

    bool restoreRegister(Dyninst::Register reg, codeGen &gen, bool noCost);
    bool popRegister(Dyninst::Register reg, codeGen &gen, bool noCost);

    bool markSavedRegister(registerSlot *num, int offsetFromFP);

    int currStackPointer;

    // This structure is permanently tainted by its association with
    // virtual registers...
    std::unordered_map<Dyninst::Register, registerSlot *> registers_;

    std::map<Dyninst::Register, registerSlot *> physicalRegisters_;

    // And convenience vectors
    std::vector<registerSlot *> GPRs_;
    std::vector<registerSlot *> FPRs_;
    std::vector<registerSlot *> SPRs_;

    // Used on platforms that have "virtual" registers to provide a mapping
    // for real (e.g., architectural) registers
    std::vector<registerSlot *> realRegisters_;

    static void initialize();
    static void initialize32();
    static void initialize64();


    registerSpace &operator=(const registerSpace &src);

    typedef enum {arbitrary, ABI_boundary, allSaved} rs_location_t;

    // Specialize liveness as represented by a bit array
    void specializeSpace(rs_location_t state);

    void specializeSpace(const bitArray &);
    bool checkLive(Dyninst::Register reg, const bitArray &liveRegs);

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
    static unsigned GPR(Dyninst::Register x) { return x; }
    static unsigned FPR(Dyninst::Register x) { return x - fpr0; }
    static unsigned SPR(Dyninst::Register x);
    int framePointer() { return r1; }
#endif
#if defined(arch_x86) || defined(arch_x86_64)
    int framePointer();
#endif
#if defined(arch_aarch64)
//#warning "Not verified yet!"
	//31 GPRs, 32 FPRs
    typedef enum { r0, r1, r2, r3, r4, r5, r6, r7, r8, r9, r10, r11, r12,
                   r13, r14, r15, r16, r17, r18, r19, r20, r21, r22, r23,
                   r24, r25, r26, r27, r28, r29, r30,
                   fpr0, fpr1, fpr2, fpr3, fpr4, fpr5, fpr6,
                   fpr7, fpr8, fpr9, fpr10, fpr11, fpr12, fpr13,
                   fpr14, fpr15, fpr16, fpr17, fpr18, fpr19, fpr20,
                   fpr21, fpr22, fpr23, fpr24, fpr25, fpr26, fpr27,
                   fpr28, fpr29, fpr30, fpr31,
                   lr, sp, pc, pstate, fpcr, fpsr, ignored } aarch64Registers_t;
    static unsigned GPR(Dyninst::Register x) { return x; }
    static unsigned FPR(Dyninst::Register x) { return x - fpr0; }
    int framePointer() { return r29; }
#endif
    // Create a map of register names to register numbers
    std::map<std::string, Dyninst::Register> registersByName;
    // The reverse map can be handled by doing a rs[x]->name

    Dyninst::Register getRegByName(const std::string name);
    std::string getRegByNumber(Dyninst::Register num);
    void getAllRegisterNames(std::vector<std::string> &ret);


};

#endif
