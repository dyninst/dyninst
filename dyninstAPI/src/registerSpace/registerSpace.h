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

#ifndef REGISTER_SPACE_H
#define REGISTER_SPACE_H

#include <map>
#include <set>
#include <string>
#include <unordered_map>
#include <vector>

#include "bitArray.h"
#include "dyn_register.h"
#include "inst.h"
#include "RealRegister.h"
#include "registerSlot.h"

#if defined(DYNINST_CODEGEN_ARCH_X86_64)
#include "inst-x86.h"
#endif

class codeGen;
class instPoint;
class AddressSpace;
class baseTramp;

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
    // Everything is dead...
    static registerSpace *optimisticRegSpace(AddressSpace *proc);
    // IRPC-specific - everything live for now
    static registerSpace *irpcRegSpace(AddressSpace *proc);
    // Aaand instPoint-specific
    static registerSpace *actualRegSpace(instPoint *iP);

    static registerSpace *getRegisterSpace(AddressSpace *proc);
    static registerSpace *getRegisterSpace(unsigned addr_width);

    registerSpace() = default;
    registerSpace(const registerSpace &) = delete;
    ~registerSpace();
    registerSpace &operator=(const registerSpace &src) = delete;

    static void createRegisterSpace(std::vector<registerSlot *> &registers);
    static void createRegisterSpace64(std::vector<registerSlot *> &registers);

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

    // For now, we save registers elsewhere and mark them here.
    bool markSavedRegister(Dyninst::Register num, int offsetFromFP);
    bool markSavedRegister(RealRegister num, int offsetFromFP);

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
    // a scratch register; do that with allocateSpecificRegister.
    // This is _ONLY_ to determine
    // if a register should be saved (e.g., over a call).
    bool isFreeRegister(Dyninst::Register k);

    // Checks to see if register starts live
    bool isRegStartsLive(Dyninst::Register reg);
    int fillDeadRegs(Dyninst::Register * deadRegs, int num);

    // Bump up the reference count. Occasionally, we underestimate it
    // and call this routine to correct this.
    void incRefCount(Dyninst::Register k);

    int getAddressWidth() { return addr_width; }
    void debugPrint();

    int numGPRs() const { return GPRs_.size(); }
    int numFPRs() const { return FPRs_.size(); }
    int numSPRs() const { return SPRs_.size(); }

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

    Dyninst::Register pc_rel_reg{Dyninst::Null_Register};
    int pc_rel_use_count{};
    int& pc_rel_offset();
    void incStack(int val);
    int getInstFrameSize();
    void setInstFrameSize(int val);

    int getStackHeight();
    void setStackHeight(int val);

    void unifyTopRegStates(codeGen &gen);
    void pushNewRegState();

 private:
    int instFrameSize_{};// How much stack space we allocate for
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

    registerSlot *findRegister(Dyninst::Register reg);
    registerSlot *findRegister(RealRegister reg);

    bool stealRegister(Dyninst::Register reg, codeGen &gen, bool noCost);

    bool markSavedRegister(registerSlot *num, int offsetFromFP);

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

    typedef enum {arbitrary, ABI_boundary, allSaved} rs_location_t;

    // Specialize liveness as represented by a bit array
    void specializeSpace(rs_location_t state);

    void specializeSpace(const bitArray &);
    bool checkLive(Dyninst::Register reg, const bitArray &liveRegs);

    unsigned addr_width{};

    // Everything is live...
    static registerSpace *conservativeRegSpace(AddressSpace *proc);

 public:
#if defined(DYNINST_CODEGEN_ARCH_POWER)
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
#if defined(DYNINST_CODEGEN_ARCH_I386) || defined(DYNINST_CODEGEN_ARCH_X86_64)
    int framePointer();
#endif
#if defined(DYNINST_CODEGEN_ARCH_AARCH64)
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
                   lr, sp, pc, nzcv, fpcr, fpsr, ignored } aarch64Registers_t;
    static unsigned GPR(Dyninst::Register x) { return x; }
    static unsigned FPR(Dyninst::Register x) { return x - fpr0; }
    int framePointer() { return r29; }
#endif
#if defined(DYNINST_CODEGEN_ARCH_AMDGPU_GFX908)
    static unsigned GPR(Dyninst::Register x) { return x; }
    static unsigned FPR(Dyninst::Register x) { return x; }
    static unsigned SPR(Dyninst::Register x) { return x; }
    static unsigned SGPR(Dyninst::Register x) { return x; }
    static unsigned VGPR(Dyninst::Register x) { return x; }
    static unsigned AGPR(Dyninst::Register x) { return x; }

    int framePointer() { return RegisterConstants::s33; }
#endif

};

void emitLoadPreviousStackFrameRegister(Dyninst::Address register_num, Dyninst::Register dest,
                                        codeGen &gen, int size, bool noCost);

#endif
