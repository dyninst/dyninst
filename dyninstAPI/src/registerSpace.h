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

class RealRegister {
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
   int alloc_num;
    const Dyninst::Register number;
    const std::string name;

    typedef enum { deadAlways, deadABI, liveAlways } initialLiveness_t;
    const initialLiveness_t initialState;

    bool offLimits;

    typedef enum { invalid, GPR, FPR, SPR, realReg} regType_t;
    const regType_t type;

    int refCount;

    typedef enum { live, spilled, dead } livenessState_t;
    livenessState_t liveState;

    bool keptValue;
    bool beenUsed;

    typedef enum { unspilled, framePointer } spillReference_t;
    spillReference_t spilledState;
    int saveOffset;

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

    // registerSlot(const registerSlot &r)

    void debugPrint(const char *str = NULL);

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
    static registerSpace *globalRegSpace_;
    static registerSpace *globalRegSpace64_;

    static void createRegSpaceInt(std::vector<registerSlot *> &regs,
                                  registerSpace *regSpace);

 public:
    static registerSpace *conservativeRegSpace(AddressSpace *proc);
    static registerSpace *optimisticRegSpace(AddressSpace *proc);
    static registerSpace *irpcRegSpace(AddressSpace *proc);
    static registerSpace *actualRegSpace(instPoint *iP);
    static registerSpace *savedRegSpace(AddressSpace *proc);

    static registerSpace *getRegisterSpace(AddressSpace *proc);
    static registerSpace *getRegisterSpace(unsigned addr_width);

    registerSpace();

    static void createRegisterSpace(std::vector<registerSlot *> &registers);
    static void createRegisterSpace64(std::vector<registerSlot *> &registers);

    ~registerSpace();

    bool readProgramRegister(codeGen &gen, Dyninst::Register source,
                             Dyninst::Register destination,
                             unsigned size);

    bool writeProgramRegister(codeGen &gen, Dyninst::Register destination,
                              Dyninst::Register source,
                              unsigned size);


    Dyninst::Register allocateRegister(codeGen &gen, bool noCost, bool realReg = false);
    bool allocateSpecificRegister(codeGen &gen, Dyninst::Register r, bool noCost = true);


    Dyninst::Register getScratchRegister(codeGen &gen, bool noCost = true, bool realReg = false);
    Dyninst::Register getScratchRegister(codeGen &gen, std::vector<Dyninst::Register> &excluded, bool noCost = true, bool realReg = false);


    bool trySpecificRegister(codeGen &gen, Dyninst::Register reg, bool noCost = true);

    bool saveAllRegisters(codeGen &gen, bool noCost);
    bool restoreAllRegisters(codeGen &gen, bool noCost);

    bool markSavedRegister(Dyninst::Register num, int offsetFromFP);
    bool markSavedRegister(RealRegister num, int offsetFromFP);

    bool markKeptRegister(Dyninst::Register num);

    bool checkVolatileRegisters(codeGen &gen, registerSlot::livenessState_t);
    bool saveVolatileRegisters(codeGen &gen);
    bool restoreVolatileRegisters(codeGen &gen);

    void freeRegister(Dyninst::Register k);
    void forceFreeRegister(Dyninst::Register k);
    void unKeepRegister(Dyninst::Register k);

    void cleanSpace();

    bool isFreeRegister(Dyninst::Register k);

    bool isRegStartsLive(Dyninst::Register reg);
    int fillDeadRegs(Dyninst::Register * deadRegs, int num);

    void incRefCount(Dyninst::Register k);

    bool markReadOnly(Dyninst::Register k);
    bool readOnlyRegister(Dyninst::Register k);
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

    std::vector <registerSlot *> &trampRegs();

    registerSlot *physicalRegs(Dyninst::Register reg) { return physicalRegisters_[reg]; }

    registerSlot *operator[](Dyninst::Register);

    bool anyLiveGPRsAtEntry() const;
    bool anyLiveFPRsAtEntry() const;
    bool anyLiveSPRsAtEntry() const;

 public:
    RealRegister loadVirtual(registerSlot *virt_r, codeGen &gen);
    RealRegister loadVirtual(Dyninst::Register virt_r, codeGen &gen);

    void loadVirtualToSpecific(registerSlot *virt_r, RealRegister real_r, codeGen &gen);
    void loadVirtualToSpecific(Dyninst::Register virt_r, RealRegister real_r, codeGen &gen);

    void makeRegisterAvail(RealRegister r, codeGen &gen);

    void noteVirtualInReal(Dyninst::Register v_r, RealRegister r_r);
    void noteVirtualInReal(registerSlot *v_r, RealRegister r_r);

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
    int instFrameSize_;

    std::vector<regState_t *> regStateStack;

    std::vector<RealRegsState>& regState();
    int& timeline();

    std::set<registerSlot *> regs_been_spilled;

    void initRealRegSpace();

    RealRegister findReal(registerSlot *virt_r, bool &already_setup);
    void spillReal(RealRegister r, codeGen &gen);
    void loadReal(RealRegister r, registerSlot *v_r, codeGen &gen);
    void freeReal(RealRegister r);

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

    std::unordered_map<Dyninst::Register, registerSlot *> registers_;

    std::map<Dyninst::Register, registerSlot *> physicalRegisters_;

    std::vector<registerSlot *> GPRs_;
    std::vector<registerSlot *> FPRs_;
    std::vector<registerSlot *> SPRs_;

    std::vector<registerSlot *> realRegisters_;

    static void initialize();
    static void initialize32();
    static void initialize64();


    registerSpace &operator=(const registerSpace &src);

    typedef enum {arbitrary, ABI_boundary, allSaved} rs_location_t;

    void specializeSpace(rs_location_t state);

    void specializeSpace(const bitArray &);
    bool checkLive(Dyninst::Register reg, const bitArray &liveRegs);

    unsigned addr_width;

 public:
    static bool hasXMM;

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
    std::map<std::string, Dyninst::Register> registersByName;

    Dyninst::Register getRegByName(const std::string name);
    std::string getRegByNumber(Dyninst::Register num);
    void getAllRegisterNames(std::vector<std::string> &ret);


};

#endif
