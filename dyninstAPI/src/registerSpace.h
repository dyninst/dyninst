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

    typedef enum { invalid, GPR, FPR, SPR, SGPR, VGPR, realReg} regType_t;
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
#if defined(arch_amdgpu)
    // AMDGPU Vega register enumeration. Note that THIS enumeration is for Dyninst's abstraction, and the enumerations are NOT meant to map to the architectural register number here.
    // We map these registers to MachRegisters in RegisterConversion-amdgpu-vega.C
    // For information on architectural registers see MachRegister in dyn_regs.h
       typedef enum {
       s0,
       s1,
       s2,
       s3,
       s4,
       s5,
       s6,
       s7,
       s8,
       s9,
       s10,
       s11,
       s12,
       s13,
       s14,
       s15,
       s16,
       s17,
       s18,
       s19,
       s20,
       s21,
       s22,
       s23,
       s24,
       s25,
       s26,
       s27,
       s28,
       s29,
       s30,
       s31,
       s32,
       s33,
       s34,
       s35,
       s36,
       s37,
       s38,
       s39,
       s40,
       s41,
       s42,
       s43,
       s44,
       s45,
       s46,
       s47,
       s48,
       s49,
       s50,
       s51,
       s52,
       s53,
       s54,
       s55,
       s56,
       s57,
       s58,
       s59,
       s60,
       s61,
       s62,
       s63,
       s64,
       s65,
       s66,
       s67,
       s68,
       s69,
       s70,
       s71,
       s72,
       s73,
       s74,
       s75,
       s76,
       s77,
       s78,
       s79,
       s80,
       s81,
       s82,
       s83,
       s84,
       s85,
       s86,
       s87,
       s88,
       s89,
       s90,
       s91,
       s92,
       s93,
       s94,
       s95,
       s96,
       s97,
       s98,
       s99,
       s100,
       s101,
       flat_scratch_lo,
       flat_scratch_hi,
       xnack_mask_lo,
       xnack_mask_hi,
       vcc_lo,
       vcc_hi,
       ttmp0,
       ttmp1,
       ttmp2,
       ttmp3,
       ttmp4,
       ttmp5,
       ttmp6,
       ttmp7,
       ttmp8,
       ttmp9,
       ttmp10,
       ttmp11,
       ttmp12,
       ttmp13,
       ttmp14,
       ttmp15,
       m0,
       exec_lo,
       exec_hi,

       // now do vector registers
       v0,
       v1,
       v2,
       v3,
       v4,
       v5,
       v6,
       v7,
       v8,
       v9,
       v10,
       v11,
       v12,
       v13,
       v14,
       v15,
       v16,
       v17,
       v18,
       v19,
       v20,
       v21,
       v22,
       v23,
       v24,
       v25,
       v26,
       v27,
       v28,
       v29,
       v30,
       v31,
       v32,
       v33,
       v34,
       v35,
       v36,
       v37,
       v38,
       v39,
       v40,
       v41,
       v42,
       v43,
       v44,
       v45,
       v46,
       v47,
       v48,
       v49,
       v50,
       v51,
       v52,
       v53,
       v54,
       v55,
       v56,
       v57,
       v58,
       v59,
       v60,
       v61,
       v62,
       v63,
       v64,
       v65,
       v66,
       v67,
       v68,
       v69,
       v70,
       v71,
       v72,
       v73,
       v74,
       v75,
       v76,
       v77,
       v78,
       v79,
       v80,
       v81,
       v82,
       v83,
       v84,
       v85,
       v86,
       v87,
       v88,
       v89,
       v90,
       v91,
       v92,
       v93,
       v94,
       v95,
       v96,
       v97,
       v98,
       v99,
       v100,
       v101,
       v102,
       v103,
       v104,
       v105,
       v106,
       v107,
       v108,
       v109,
       v110,
       v111,
       v112,
       v113,
       v114,
       v115,
       v116,
       v117,
       v118,
       v119,
       v120,
       v121,
       v122,
       v123,
       v124,
       v125,
       v126,
       v127,
       v128,
       v129,
       v130,
       v131,
       v132,
       v133,
       v134,
       v135,
       v136,
       v137,
       v138,
       v139,
       v140,
       v141,
       v142,
       v143,
       v144,
       v145,
       v146,
       v147,
       v148,
       v149,
       v150,
       v151,
       v152,
       v153,
       v154,
       v155,
       v156,
       v157,
       v158,
       v159,
       v160,
       v161,
       v162,
       v163,
       v164,
       v165,
       v166,
       v167,
       v168,
       v169,
       v170,
       v171,
       v172,
       v173,
       v174,
       v175,
       v176,
       v177,
       v178,
       v179,
       v180,
       v181,
       v182,
       v183,
       v184,
       v185,
       v186,
       v187,
       v188,
       v189,
       v190,
       v191,
       v192,
       v193,
       v194,
       v195,
       v196,
       v197,
       v198,
       v199,
       v200,
       v201,
       v202,
       v203,
       v204,
       v205,
       v206,
       v207,
       v208,
       v209,
       v210,
       v211,
       v212,
       v213,
       v214,
       v215,
       v216,
       v217,
       v218,
       v219,
       v220,
       v221,
       v222,
       v223,
       v224,
       v225,
       v226,
       v227,
       v228,
       v229,
       v230,
       v231,
       v232,
       v233,
       v234,
       v235,
       v236,
       v237,
       v238,
       v239,
       v240,
       v241,
       v242,
       v243,
       v244,
       v245,
       v246,
       v247,
       v248,
       v249,
       v250,
       v251,
       v252,
       v253,
       v254,
       v255,

       ignored
     } amdgpuRegisters_t;

    static unsigned GPR(Dyninst::Register x) { return x; }
    static unsigned FPR(Dyninst::Register x) { return x; }
    int framePointer() { return s33; }
#endif

    // Create a map of register names to register numbers
    std::map<std::string, Dyninst::Register> registersByName;
    // The reverse map can be handled by doing a rs[x]->name

    Dyninst::Register getRegByName(const std::string name);
    std::string getRegByNumber(Dyninst::Register num);
    void getAllRegisterNames(std::vector<std::string> &ret);


};

#endif
