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

//#warning "This file is not implemented yet!"

#include "common/src/headers.h"
#include "dyninstAPI/h/BPatch_memoryAccess_NP.h"
#include "dyninstAPI/src/image.h"
#include "dyninstAPI/src/dynProcess.h"
#include "dyninstAPI/src/inst.h"
#include "dyninstAPI/src/instP.h"
#include "dyninstAPI/src/inst-aarch64.h"
#include "common/src/arch.h"
#include "dyninstAPI/src/codegen.h"
#include "dyninstAPI/src/ast.h"
#include "dyninstAPI/src/util.h"
#include "common/src/stats.h"
#include "dyninstAPI/src/os.h"
#include "dyninstAPI/src/instPoint.h" // class instPoint
#include "dyninstAPI/src/debug.h"
#include "common/src/debugOstream.h"
#include "dyninstAPI/src/baseTramp.h"
#include "dyninstAPI/h/BPatch.h"
#include "dyninstAPI/src/BPatch_collections.h"
#include "dyninstAPI/src/registerSpace.h"
#include "dyninstAPI/src/binaryEdit.h"
#include "dyninstAPI/src/function.h"
#include "dyninstAPI/src/mapped_object.h"

#include "parseAPI/h/CFG.h"

#include "emitter.h"
#include "emit-aarch64.h"

#include <boost/assign/list_of.hpp>
using namespace boost::assign;
#include <sstream>

#include "dyninstAPI/h/BPatch_memoryAccess_NP.h"

extern bool isPowerOf2(int value, int &result);

#define DISTANCE(x, y)   ((x<y) ? (y-x) : (x-y))

Address getMaxBranch() {
    return MAX_BRANCH_OFFSET;
}

std::unordered_map<std::string, unsigned> funcFrequencyTable;

void initDefaultPointFrequencyTable() {
    assert(0); //Not implemented
}

/************************************* Register Space **************************************/

void registerSpace::initialize32() {
    assert(!"No 32-bit implementation for the ARM architecture!");
}

void registerSpace::initialize64() {
    static bool done = false;
    if (done)
        return;

    pdvector < registerSlot * > registers;

    //GPRs
    for (unsigned idx = r0; idx <= r28; idx++) {
        char name[32];
        if (idx < 10)
            sprintf(name, "r%1d", idx - r0);
        else
            sprintf(name, "r%2d", idx - r0);
        registers.push_back(new registerSlot(idx,
                                             name,
                                             false,
                                             registerSlot::liveAlways,
                                             registerSlot::GPR));
    }
    //Mark r29 (frame pointer) and r30 (link register) as off-limits
    registers.push_back(new registerSlot(r29, "r29", true, registerSlot::liveAlways, registerSlot::GPR));
    registers.push_back(new registerSlot(r30, "r30", true, registerSlot::liveAlways, registerSlot::GPR));

    //SPRs
    registers.push_back(new registerSlot(lr, "lr", true, registerSlot::liveAlways, registerSlot::SPR));
    registers.push_back(new registerSlot(sp, "sp", true, registerSlot::liveAlways, registerSlot::SPR));
    registers.push_back(new registerSlot(pstate, "nzcv", true, registerSlot::liveAlways, registerSlot::SPR));
    registers.push_back(new registerSlot(fpcr, "fpcr", true, registerSlot::liveAlways, registerSlot::SPR));
    registers.push_back(new registerSlot(fpsr, "fpsr", true, registerSlot::liveAlways, registerSlot::SPR));

    //FPRs
    for (unsigned idx = fpr0; idx <= fpr31; idx++) {
        char name[32];
        sprintf(name, "fpr%d", idx - fpr0);
        registers.push_back(new registerSlot(idx,
                                             name,//TODO mov SP to FP
                                             false,
                                             registerSlot::liveAlways,
                                             registerSlot::FPR));
    }

    registerSpace::createRegisterSpace64(registers);
    done = true;
}

void registerSpace::initialize() {
    initialize64();
}

/************************************************************************************************/
/************************************************************************************************/

/********************************* EmitterAARCH64SaveRegs ***************************************/

/********************************* Private methods *********************************************/

void EmitterAARCH64SaveRegs::saveSPR(codeGen &gen, Register scratchReg, int sprnum, int stkOffset) {
    //TODO move map to common location
    map<int, int> sysRegCodeMap = map_list_of(SPR_NZCV, 0x5A10)(SPR_FPCR, 0x5A20)(SPR_FPSR, 0x5A21);
    if(!sysRegCodeMap.count(sprnum))
        assert(!"Invalid/unknown system register passed to saveSPR()!");

    instruction insn;
    insn.clear();

    //Set opcode for MRS instruction
    INSN_SET(insn, 20, 31, MRSOp);
    //Set destination register
    INSN_SET(insn, 0, 4, scratchReg & 0x1F);
    //Set bits representing source system register
    INSN_SET(insn, 5, 19, sysRegCodeMap[sprnum]);
    insnCodeGen::generate(gen, insn);

    insnCodeGen::generateMemAccess32or64(gen, insnCodeGen::Store, scratchReg, REG_SP, stkOffset, false);
}

void EmitterAARCH64SaveRegs::saveRegister(codeGen &gen, Register reg, int save_off) {
    insnCodeGen::generateMemAccess32or64(gen, insnCodeGen::Store, reg, REG_SP, save_off, true);
}


void EmitterAARCH64SaveRegs::saveFPRegister(codeGen &gen, Register reg, int save_off) {
    //Always performing save of the full FP register
    insnCodeGen::generateMemAccessFP(gen, insnCodeGen::Store, reg, REG_SP, save_off, 0, true);
}

/********************************* Public methods *********************************************/

unsigned EmitterAARCH64SaveRegs::saveGPRegisters(baseTramp *bt, codeGen &gen, registerSpace *theRegSpace, int &offset, int numReqGPRs) {
    unsigned ret = 0;
    if(numReqGPRs == -1)
        numReqGPRs = theRegSpace->numGPRs();

    pdvector<registerSlot *> &regs = theRegSpace->trampRegs();

    for(int idx = 0; idx < regs.size() && ret < numReqGPRs; idx++) {
        registerSlot *reg = regs[idx];
        if(bt->definedRegs[reg->encoding()]) {
            saveRegister(gen, reg->number, -GPRSIZE_64);
            theRegSpace->markSavedRegister(reg->number, offset);

            offset += GPRSIZE_64;
            ret++;
        }
    }

    return ret;
}

unsigned EmitterAARCH64SaveRegs::saveFPRegisters(codeGen &gen, registerSpace *theRegSpace, int &offset) {
    unsigned ret = 0;

    for(int idx = 0; idx < theRegSpace->numFPRs(); idx++) {
        registerSlot *reg = theRegSpace->FPRs()[idx];

        if(reg->liveState == registerSlot::live) {
            saveFPRegister(gen, reg->number, -FPRSIZE);
            reg->liveState = registerSlot::spilled;

            offset += FPRSIZE;
            ret++;
        }
    }

    return ret;
}

unsigned EmitterAARCH64SaveRegs::saveSPRegisters(codeGen &gen, registerSpace *theRegSpace, int &offset, bool force_save) {
    int ret = 0;

    pdvector<registerSlot *> spRegs;
    map<registerSlot *, int> regMap;

    registerSlot *regNzcv = (*theRegSpace)[registerSpace::pstate];
    assert(regNzcv);
    regMap[regNzcv] = SPR_NZCV;
    if(force_save || regNzcv->liveState == registerSlot::live)
        spRegs.push_back(regNzcv);

    registerSlot *regFpcr = (*theRegSpace)[registerSpace::fpcr];
    assert(regFpcr);
    regMap[regFpcr] = SPR_FPCR;
    if(force_save || regFpcr->liveState == registerSlot::live)
        spRegs.push_back(regFpcr);

    registerSlot *regFpsr = (*theRegSpace)[registerSpace::fpsr];
    assert(regFpsr);
    regMap[regFpsr] = SPR_FPSR;
    if(force_save || regFpsr->liveState == registerSlot::live)
        spRegs.push_back(regFpsr);

    for(pdvector<registerSlot *>::iterator itr = spRegs.begin(); itr != spRegs.end(); itr++) {
        registerSlot *cur = *itr;
        saveSPR(gen, theRegSpace->getScratchRegister(gen, true), regMap[cur], -GPRSIZE_32);
        theRegSpace->markSavedRegister(cur->number, offset);

        offset += GPRSIZE_32;
        ret++;
    }

    return ret;
}

void EmitterAARCH64SaveRegs::createFrame(codeGen &gen) {
    //Save link register
    Register linkRegister = gen.rs()->getRegByName("r30");
    saveRegister(gen, linkRegister, -GPRSIZE_64);

    //Save frame pointer
    Register framePointer = gen.rs()->getRegByName("r29");
    saveRegister(gen, framePointer, -GPRSIZE_64);

    //Move stack pointer to frame pointer
    Register stackPointer = gen.rs()->getRegByName("sp");
    insnCodeGen::generateMoveSP(gen, stackPointer, framePointer, true);
}

/***********************************************************************************************/
/***********************************************************************************************/

/********************************* EmitterAARCH64RestoreRegs ************************************/

/********************************* Public methods *********************************************/

unsigned EmitterAARCH64RestoreRegs::restoreGPRegisters(codeGen &gen, registerSpace *theRegSpace) {
    unsigned ret = 0;

    for(int idx = theRegSpace->numGPRs() - 1; idx >= 0; idx--) {
        registerSlot *reg = theRegSpace->GPRs()[idx];

        if(reg->liveState == registerSlot::spilled) {
            restoreRegister(gen, reg->number, GPRSIZE_64);
            ret++;
        }
    }

    return ret;
}

unsigned EmitterAARCH64RestoreRegs::restoreFPRegisters(codeGen &gen, registerSpace *theRegSpace) {
    unsigned ret = 0;

    for(int idx = theRegSpace->numFPRs() - 1; idx >= 0; idx--) {
        registerSlot *reg = theRegSpace->FPRs()[idx];

        if(reg->liveState == registerSlot::spilled) {
            restoreFPRegister(gen, reg->number, FPRSIZE);
            ret++;
        }
    }

    return ret;
}

unsigned EmitterAARCH64RestoreRegs::restoreSPRegisters(codeGen &gen, registerSpace *theRegSpace, int force_save) {
    int ret = 0;

    pdvector<registerSlot *> spRegs;
    map<registerSlot *, int> regMap;

    registerSlot *regNzcv = (*theRegSpace)[registerSpace::pstate];
    assert(regNzcv);
    regMap[regNzcv] = SPR_NZCV;
    if(force_save || regNzcv->liveState == registerSlot::spilled)
        spRegs.push_back(regNzcv);

    registerSlot *regFpcr = (*theRegSpace)[registerSpace::fpcr];
    assert(regFpcr);
    regMap[regFpcr] = SPR_FPCR;
    if(force_save || regFpcr->liveState == registerSlot::spilled)
        spRegs.push_back(regFpcr);

    registerSlot *regFpsr = (*theRegSpace)[registerSpace::fpsr];
    assert(regFpsr);
    regMap[regFpsr] = SPR_FPSR;
    if(force_save || regFpsr->liveState == registerSlot::spilled)
        spRegs.push_back(regFpsr);

    for(pdvector<registerSlot *>::iterator itr = spRegs.begin(); itr != spRegs.end(); itr++) {
        registerSlot *cur = *itr;
        restoreSPR(gen, theRegSpace->getScratchRegister(gen, true), regMap[cur], GPRSIZE_32);
        ret++;
    }

    return ret;
}

void EmitterAARCH64RestoreRegs::tearFrame(codeGen &gen) {
    //Restore frame pointer
    Register framePointer = gen.rs()->getRegByName("r29");
    restoreRegister(gen, framePointer, GPRSIZE_64);

    //Restore link register
    Register linkRegister = gen.rs()->getRegByName("r30");
    restoreRegister(gen, linkRegister, GPRSIZE_64);
}


/********************************* Private methods *********************************************/

void EmitterAARCH64RestoreRegs::restoreSPR(codeGen &gen, Register scratchReg, int sprnum, int stkOffset) {
    insnCodeGen::generateMemAccess32or64(gen, insnCodeGen::Load, scratchReg, REG_SP, stkOffset, false);

    //TODO move map to common location
    map<int, int> sysRegCodeMap = map_list_of(SPR_NZCV, 0x5A10)(SPR_FPCR, 0x5A20)(SPR_FPSR, 0x5A21);
    if (!sysRegCodeMap.count(sprnum))
        assert(!"Invalid/unknown system register passed to restoreSPR()!");

    instruction insn;
    insn.clear();

    //Set opcode for MSR (register) instruction
    INSN_SET(insn, 20, 31, MSROp);
    //Set source register
    INSN_SET(insn, 0, 4, scratchReg & 0x1F);
    //Set bits representing destination system register
    INSN_SET(insn, 5, 19, sysRegCodeMap[sprnum]);
    insnCodeGen::generate(gen, insn);
}

void EmitterAARCH64RestoreRegs::restoreRegister(codeGen &gen, Register reg, int save_off) {
    insnCodeGen::generateMemAccess32or64(gen, insnCodeGen::Load, reg, REG_SP, save_off, true);
}

void EmitterAARCH64RestoreRegs::restoreFPRegister(codeGen &gen, Register reg, int save_off) {
    insnCodeGen::generateMemAccessFP(gen, insnCodeGen::Load, reg, REG_SP, save_off, 0, true);
}

/***********************************************************************************************/
/***********************************************************************************************/

/*********************************** Base Tramp ***********************************************/

bool baseTramp::generateSaves(codeGen &gen,
                              registerSpace *) {
    EmitterAARCH64SaveRegs saveRegs;

    int offset = 0;

    baseTramp *bt = this;
    bool saveFrame = !bt || bt->needsFrame();
    if(saveFrame) {
        saveRegs.createFrame(gen);
        offset += (GPRSIZE_64 * 2);
    }
    bt->createdFrame = saveFrame;

    bt->definedRegs = gen.getRegsDefined();
    assert(!bt->definedRegs.empty());
    saveRegs.saveGPRegisters(bt, gen, gen.rs(), offset);

    bool saveFPRs = BPatch::bpatch->isForceSaveFPROn() ||
                   (BPatch::bpatch->isSaveFPROn()      &&
                    gen.rs()->anyLiveFPRsAtEntry()     &&
                    bt->saveFPRs());

    if(saveFPRs)
        saveRegs.saveFPRegisters(gen, gen.rs(), offset);
    bt->savedFPRs = saveFPRs;

    saveRegs.saveSPRegisters(gen, gen.rs(), offset, false);

    return true;
}

bool baseTramp::generateRestores(codeGen &gen,
                                 registerSpace *) {
    EmitterAARCH64RestoreRegs restoreRegs;

    restoreRegs.restoreSPRegisters(gen, gen.rs(), false);

    baseTramp *bt = this;
    if(bt->savedFPRs)
        restoreRegs.restoreFPRegisters(gen, gen.rs());

    restoreRegs.restoreGPRegisters(gen, gen.rs());

    if(bt->createdFrame)
        restoreRegs.tearFrame(gen);

    return true;
}

/***********************************************************************************************/
/***********************************************************************************************/

//TODO: 32-/64-bit regs?
void emitImm(opCode op, Register src1, RegValue src2imm, Register dest, codeGen &gen, bool noCost, registerSpace * /* rs */) {
    switch(op) {
        case plusOp:
        case minusOp: {
            if(src2imm >= -(1 << 11) && src2imm < (long int)((1 << 11) - 1))
                insnCodeGen::generateAddSubImmediate(gen, op == plusOp ? insnCodeGen::Add : insnCodeGen::Sub, 0, src2imm, src1, dest, false);
            else if(src2imm >= MIN_IMM16 && src2imm < MAX_IMM16) {
                Register rm = insnCodeGen::moveValueToReg(gen, src2imm);
                insnCodeGen::generateAddSubShifted(gen, op == plusOp ? insnCodeGen::Add : insnCodeGen::Sub, 0, 0, rm, src1, dest, true);
            }
        }
            break;
        case timesOp: {
            Register rm = insnCodeGen::moveValueToReg(gen, src2imm);
            insnCodeGen::generateMul(gen, rm, src1, dest, true);
        }
            break;
        case divOp:
            break;
        case orOp: {
            Register rm = insnCodeGen::moveValueToReg(gen, src2imm);
            insnCodeGen::generateBitwiseOpShifted(gen, insnCodeGen::Or, 0, rm, 0, src1, dest, true);
        }
            break;
        case andOp:  {
            Register rm = insnCodeGen::moveValueToReg(gen, src2imm);
            insnCodeGen::generateBitwiseOpShifted(gen, insnCodeGen::And, 0, rm, 0, src1, dest, true);
        }
            break;
    }
}

void cleanUpAndExit(int status);

/* Recursive function that goes to where our instrumentation is calling
to figure out what registers are clobbered there, and in any function
that it calls, to a certain depth ... at which point we clobber everything

Update-12/06, njr, since we're going to a cached system we are just going to
look at the first level and not do recursive, since we would have to also
store and reexamine every call out instead of doing it on the fly like before*/
bool EmitterAARCH64::clobberAllFuncCall(registerSpace *rs,
                                        func_instance *callee) {
    if(!callee)
        return true;

    stats_codegen.startTimer(CODEGEN_LIVENESS_TIMER);

    if(callee->ifunc()->isLeafFunc()) {
        std::set<Register> *gpRegs = callee->ifunc()->usedGPRs();
        for(std::set<Register>::iterator itr = gpRegs->begin(); itr != gpRegs->end(); itr++)
            rs->GPRs()[*itr]->beenUsed = true;

        std::set<Register> *fpRegs = callee->ifunc()->usedFPRs();
        for(std::set<Register>::iterator itr = fpRegs->begin(); itr != fpRegs->end(); itr++)
            rs->FPRs()[*itr]->beenUsed = true;
    } else {
        for(int idx = 0; idx < rs->numGPRs(); idx++)
            rs->GPRs()[idx]->beenUsed = true;
        for(int idx = 0; idx < rs->numFPRs(); idx++)
            rs->FPRs()[idx]->beenUsed = true;
    }

    stats_codegen.stopTimer(CODEGEN_LIVENESS_TIMER);

    return false;
}

Register emitFuncCall(opCode, codeGen &, pdvector <AstNodePtr> &, bool, Address) {
    assert(0);
    return 0;
}

Register emitFuncCall(opCode op,
                      codeGen &gen,
                      pdvector <AstNodePtr> &operands, bool noCost,
                      func_instance *callee) {
    return gen.emitter()->emitCall(op, gen, operands, noCost, callee);
}

Register EmitterAARCH64::emitCallReplacement(opCode ocode,
                                             codeGen &gen,
                                             bool /* noCost */,
                                             func_instance *callee) {
    assert(0); //Not implemented
    return 0;
}

// There are four "axes" going on here:
// 32 bit vs 64 bit
// Instrumentation vs function call replacement
// Static vs. dynamic

Register EmitterAARCH64::emitCall(opCode ocode,
                                  codeGen &gen,
                                  const pdvector <AstNodePtr> &operands,
                                  bool noCost,
                                  func_instance *callee) {
    assert(0); //Not implemented
    return NULL;
}


codeBufIndex_t emitA(opCode op, Register src1, Register /*src2*/, long dest,
                     codeGen &gen, RegControl, bool /*noCost*/) {
    assert(0); //Not implemented
    return NULL;
}

Register emitR(opCode op, Register src1, Register src2, Register dest,
               codeGen &gen, bool /*noCost*/,
               const instPoint * /*location*/, bool /*for_MT*/) {

    assert(0);
    return REG_NULL;
}

void emitJmpMC(int /*condition*/, int /*offset*/, codeGen &) {
    assert(0); //Not implemented
    // Not needed for memory instrumentation, otherwise TBD
}


// VG(11/16/01): Say if we have to restore a register to get its original value
// VG(03/15/02): Sync'd with the new AIX tramp
static inline bool needsRestore(Register x) {
    assert(0); //Not implemented
    return false;
}

// VG(03/15/02): Restore mutatee value of GPR reg to dest GPR
static inline void restoreGPRtoGPR(codeGen &gen,
                                   Register reg, Register dest) {
    assert(0); //Not implemented
}

// VG(03/15/02): Restore mutatee value of XER to dest GPR
static inline void restoreXERtoGPR(codeGen &gen, Register dest) {
    assert(0); //Not implemented
}

// VG(03/15/02): Move bits 25:31 of GPR reg to GPR dest
static inline void moveGPR2531toGPR(codeGen &gen,
                                    Register reg, Register dest) {
    assert(0); //Not implemented
}

// VG(11/16/01): Emit code to add the original value of a register to
// another. The original value may need to be restored from stack...
// VG(03/15/02): Made functionality more obvious by adding the above functions
static inline void emitAddOriginal(Register src, Register acc,
                                   codeGen &gen, bool noCost) {
    assert(0); //Not implemented
}

// VG(11/07/01): Load in destination the effective address given
// by the address descriptor. Used for memory access stuff.
void emitASload(const BPatch_addrSpec_NP *as, Register dest, int stackShift,
                codeGen &gen,
                bool noCost) {
    assert(0); //Not implemented
}

void emitCSload(const BPatch_addrSpec_NP *as, Register dest, codeGen &gen,
                bool noCost) {
    assert(0); //Not implemented
}

void emitVload(opCode op, Address src1, Register src2, Register dest,
               codeGen &gen, bool /*noCost*/,
               registerSpace * /*rs*/, int size,
               const instPoint * /* location */, AddressSpace *proc) {
    assert(0); //Not implemented
}

void emitVstore(opCode op, Register src1, Register /*src2*/, Address dest,
                codeGen &gen, bool noCost,
                registerSpace * /* rs */, int size,
                const instPoint * /* location */, AddressSpace *proc) {
    assert(0); //Not implemented
    return;
}

void emitV(opCode op, Register src1, Register src2, Register dest,
           codeGen &gen, bool /*noCost*/,
           registerSpace * /*rs*/, int size,
           const instPoint * /* location */, AddressSpace *proc) {
    assert(0); //not implemented
    return;
}

//
// I don't know how to compute cycles for AARCH64 instructions due to
//   multiple functional units.  However, we can compute the number of
//   instructions and hope that is fairly close. - jkh 1/30/96
//
int getInsnCost(opCode op) {
    assert(0); //Not implemented
    return NULL;
}

#if 0
// What does this do???
void registerSpace::saveClobberInfo(const instPoint *location)
{
  registerSlot *regSlot = NULL;
  registerSlot *regFPSlot = NULL;
  if (location == NULL)
    return;
  if (location->actualGPRLiveSet_ != NULL && location->actualFPRLiveSet_ != NULL)
    {

      // REG guard registers, if live, must be saved
      if (location->actualGPRLiveSet_[ REG_GUARD_ADDR ] == LIVE_REG)
    location->actualGPRLiveSet_[ REG_GUARD_ADDR ] = LIVE_CLOBBERED_REG;

      if (location->actualGPRLiveSet_[ REG_GUARD_OFFSET ] == LIVE_REG)
    location->actualGPRLiveSet_[ REG_GUARD_OFFSET ] = LIVE_CLOBBERED_REG;

      // GPR and FPR scratch registers, if live, must be saved
      if (location->actualGPRLiveSet_[ REG_SCRATCH ] == LIVE_REG)
    location->actualGPRLiveSet_[ REG_SCRATCH ] = LIVE_CLOBBERED_REG;

      if (location->actualFPRLiveSet_[ REG_SCRATCH ] == LIVE_REG)
    location->actualFPRLiveSet_[ REG_SCRATCH ] = LIVE_CLOBBERED_REG;

      // Return func call register, since we make a call because
      // of multithreading (regardless if it's threaded) from BT
      // we must save return register
      if (location->actualGPRLiveSet_[ 3 ] == LIVE_REG)
    location->actualGPRLiveSet_[ 3 ] = LIVE_CLOBBERED_REG;


      for (u_int i = 0; i < getRegisterCount(); i++)
    {
      regSlot = getRegSlot(i);

      if (  location->actualGPRLiveSet_[ (int) registers[i].number ] == LIVE_REG )
        {
          if (!registers[i].beenClobbered)
        location->actualGPRLiveSet_[ (int) registers[i].number ] = LIVE_UNCLOBBERED_REG;
          else
        location->actualGPRLiveSet_[ (int) registers[i].number ] = LIVE_CLOBBERED_REG;
        }


      if (  location->actualGPRLiveSet_[ (int) registers[i].number ] == LIVE_UNCLOBBERED_REG )
        {
          if (registers[i].beenClobbered)
        location->actualGPRLiveSet_[ (int) registers[i].number ] = LIVE_CLOBBERED_REG;
        }
    }

      for (u_int i = 0; i < getFPRegisterCount(); i++)
    {
      regFPSlot = getFPRegSlot(i);

      if (  location->actualFPRLiveSet_[ (int) fpRegisters[i].number ] == LIVE_REG )
        {
          if (!fpRegisters[i].beenClobbered)
        location->actualFPRLiveSet_[ (int) fpRegisters[i].number ] = LIVE_UNCLOBBERED_REG;
          else
        location->actualFPRLiveSet_[ (int) fpRegisters[i].number ] = LIVE_CLOBBERED_REG;
        }

      if (  location->actualFPRLiveSet_[ (int) fpRegisters[i].number ] == LIVE_UNCLOBBERED_REG )
        {
          if (fpRegisters[i].beenClobbered)
        location->actualFPRLiveSet_[ (int) fpRegisters[i].number ] = LIVE_CLOBBERED_REG;
        }
    }
    }
}
#endif


bool doNotOverflow(int value) {
    assert(0); //Not implemented
    // we are assuming that we have 15 bits to store the immediate operand.
    if ((value <= 32767) && (value >= -32768)) return (true);
    else return (false);
}

#if !defined(os_vxworks)

// hasBeenBound: returns true if the runtime linker has bound the
// function symbol corresponding to the relocation entry in at the address
// specified by entry and base_addr.  If it has been bound, then the callee
// function is returned in "target_pdf", else it returns false.
bool PCProcess::hasBeenBound(const SymtabAPI::relocationEntry &entry,
                             func_instance *&target_pdf, Address base_addr) {
    assert(0); //Not implemented
    return false;
}

#endif

bool PCProcess::bindPLTEntry(const SymtabAPI::relocationEntry &, Address,
                             func_instance *, Address) {
    assert(0); //Not implemented
    assert(0 && "TODO!");
    return false;
}

void emitLoadPreviousStackFrameRegister(Address register_num,
                                        Register dest,
                                        codeGen &gen,
                                        int /*size*/,
                                        bool noCost) {
    assert(0); //Not implemented
}

void emitStorePreviousStackFrameRegister(Address,
                                         Register,
                                         codeGen &,
                                         int,
                                         bool) {
    assert(0);
}

using namespace Dyninst::InstructionAPI;

bool AddressSpace::getDynamicCallSiteArgs(InstructionAPI::Instruction::Ptr i,
                                          Address addr,
                                          pdvector <AstNodePtr> &args) {
    assert(0); //Not implemented
    return false;
}

bool writeFunctionPtr(AddressSpace *p, Address addr, func_instance *f) {
    assert(0); //Not implemented
    return false;
}

Emitter *AddressSpace::getEmitter() {
    static EmitterAARCH64Stat emitter64Stat;
    static EmitterAARCH64Dyn emitter64Dyn;

    if (proc())
        return &emitter64Dyn;

    return &emitter64Stat;
}

#define GET_IP      0x429f0005
#define MFLR_30     0x7fc802a6
#define ADDIS_30_30 0x3fde0000
#define ADDI_30_30  0x3bde0000
#define LWZ_11_30   0x817e0000
#define ADDIS_11_30 0x3d7e0000

/*
 * If the target stub_addr is a glink stub, try to determine the actual
 * function called (through the GOT) and fill in that information.
 *
 * The function at stub_addr may not have been created when this method
 * is called.
 *
 * XXX Is this a candidate to move into general parsing code, or is
 *     this properly a Dyninst-only technique?
 */

/*
bool image::updatePltFunc(parse_func *caller_func, Address stub_addr)
{
	assert(0); //Not implemented
    return true;
}
*/

bool EmitterAARCH64::emitCallRelative(Register dest, Address offset, Register base, codeGen &gen) {
    assert(0); //Not implemented
    return true;
}

bool EmitterAARCH64::emitLoadRelative(Register dest, Address offset, Register base, int size, codeGen &gen) {
    assert(0); //Not implemented
    return true;
}


void EmitterAARCH64::emitStoreRelative(Register source, Address offset, Register base, int size, codeGen &gen) {
    //return true;
    assert(0); //Not implemented
}

bool EmitterAARCH64::emitMoveRegToReg(registerSlot *src,
                                      registerSlot *dest,
                                      codeGen &gen) {
    assert(0); //Not implemented
    return true;
}

/*
bool EmitterAARCH6432Stat::emitPIC(codeGen& gen, Address origAddr, Address relocAddr) {

      Register scratchPCReg = gen.rs()->getScratchRegister(gen, true);
      pdvector<Register> excludeReg;
      excludeReg.push_back(scratchPCReg);
      Register scratchReg = gen.rs()->getScratchRegister(gen, excludeReg, true);
      bool newStackFrame = false;
      int stack_size = 0;
      int gpr_off, fpr_off, ctr_off;
      //fprintf(stderr, " emitPIC origAddr 0x%lx reloc 0x%lx Registers PC %d scratch %d \n", origAddr, relocAddr, scratchPCReg, scratchReg);
      if ((scratchPCReg == REG_NULL) || (scratchReg == REG_NULL)) {
		//fprintf(stderr, " Creating new stack frame for 0x%lx to 0x%lx \n", origAddr, relocAddr);

		newStackFrame = true;
		//create new stack frame
	        gpr_off = TRAMP_GPR_OFFSET_32;
	        fpr_off = TRAMP_FPR_OFFSET_32;
	        ctr_off = STK_CTR_32;

		// Make a stack frame.
	    	pushStack(gen);

    		// Save GPRs
	      stack_size = saveGPRegisters(gen, gen.rs(), gpr_off, 2);

	      scratchPCReg = gen.rs()->getScratchRegister(gen, true);
	      assert(scratchPCReg != REG_NULL);
	      excludeReg.clear();
	      excludeReg.push_back(scratchPCReg);
	      scratchReg = gen.rs()->getScratchRegister(gen, excludeReg, true);
	      assert(scratchReg != REG_NULL);
	      // relocaAddr has moved since we added instructions to setup a new stack frame
	      relocAddr = relocAddr + ((stack_size + 1)*(gen.width()));
              //fprintf(stderr, " emitPIC origAddr 0x%lx reloc 0x%lx stack size %d Registers PC %d scratch %d \n", origAddr, relocAddr, stack_size, scratchPCReg, scratchReg);

	}
	emitMovePCToReg(scratchPCReg, gen);
	Address varOffset = origAddr - relocAddr;
	emitCallRelative(scratchReg, varOffset, scratchPCReg, gen);
      	insnCodeGen::generateMoveToLR(gen, scratchReg);
	if(newStackFrame) {
	      // GPRs
	      restoreGPRegisters(gen, gen.rs(), gpr_off);
	      popStack(gen);
	}

      return 0;
}

bool EmitterAARCH64Stat::emitPIC(codeGen& gen, Address origAddr, Address relocAddr) {
	assert(0);
	return false;
}
bool EmitterAARCH64Dyn::emitPIC(codeGen &gen, Address origAddr, Address relocAddr) {

	Address origRet = origAddr + 4;
	Register scratch = gen.rs()->getScratchRegister(gen, true);
	assert(scratch != REG_NULL);
	instruction::loadImmIntoReg(gen, scratch, origRet);
	insnCodeGen::generateMoveToLR(gen, scratch);
	return true;

}
*/

bool EmitterAARCH64Stat::emitPLTCommon(func_instance *callee, bool call, codeGen &gen) {
    assert(0); //Not implemented
    return true;
}

#if 0
bool EmitterAARCH64Stat::emitPLTCommon(func_instance *callee, bool call, codeGen &gen) {
  // In PPC64 Linux, function descriptors are used in place of direct
  // function pointers.  The descriptors have the following layout:
  //
  // Function Descriptor --> + 0: <Function Text Address>
  //                         + 8: <TOC Pointer Value>
  //                         +16: <Environment Pointer [Optional]>
  //
  // Additionally, this should be able to stomp on the link register (LR)
  // and TOC register (r2), as they were saved by Emitter::emitCall() if
  // necessary.
  //
  // So here's a brief sketch of the code this function generates:
  //
  //   Set up new branch target in LR from function descriptor
  //   Set up new TOC in R2 from function descriptor + 8
  //   Call
  bool isStaticBinary = false;

  if(gen.addrSpace()->edit()->getMappedObject()->parse_img()->getObject()->isStaticBinary()) {
    isStaticBinary = true;
  }

  const unsigned TOCreg = 2;
  const unsigned wordsize = gen.width();
  assert(wordsize == 8);
  Address dest = getInterModuleFuncAddr(callee, gen);
  Address caller_toc = 0;
  Address toc_anchor = gen.addrSpace()->getTOCoffsetInfo(callee);
  // Instead of saving the TOC (if we can't), just reset it afterwards.
  if (gen.func()) {
    caller_toc = gen.addrSpace()->getTOCoffsetInfo(gen.func());
  }
  else if (gen.point()) {
    caller_toc = gen.addrSpace()->getTOCoffsetInfo(gen.point()->func());
  }
  else {
    // Don't need it, and this might be an iRPC
  }

  if(isStaticBinary)
    caller_toc = 0;

  //Offset destOff = dest - gen.currAddr();
  Offset destOff = dest - caller_toc;

  //    insnCodeGen::loadPartialImmIntoReg(gen, TOCreg, destOff);
  // Broken to see if any of this generates intellible code.

  Register scratchReg = 3; // = gen.rs()->getScratchRegister(gen, true);
  int stackSize = 0;
  if (scratchReg == REG_NULL) {
    pdvector<Register> freeReg;
    pdvector<Register> excludeReg;
    stackSize = insnCodeGen::createStackFrame(gen, 1, freeReg, excludeReg);
    assert (stackSize == 1);
    scratchReg = freeReg[0];
  }
  insnCodeGen::loadImmIntoReg<Offset?(gen, scratchReg, destOff);

  if(!isStaticBinary) {
    insnCodeGen::generateLoadReg64(gen, scratchReg, scratchReg, TOCreg);

    insnCodeGen::generateMemAccess64(gen, LDop, LDxop,
                     TOCreg, scratchReg, 8);
  }
  insnCodeGen::generateMemAccess64(gen, LDop, LDxop,
                   scratchReg, scratchReg, 0);

  insnCodeGen::generateMoveToCR(gen, scratchReg);

  if (stackSize > 0)
    insnCodeGen::removeStackFrame(gen);


  instruction branch_insn(call ? BCTRLraw : BCTRraw);
  insnCodeGen::generate(gen, branch_insn);

  return true;
}
#endif

bool EmitterAARCH64Dyn::emitTOCCommon(block_instance *block, bool call, codeGen &gen) {
    assert(0); //Not implemented
    return true;
}

// TODO 32/64-bit?
bool EmitterAARCH64Stat::emitPLTCall(func_instance *callee, codeGen &gen) {
    assert(0); //Not implemented
    return emitPLTCommon(callee, true, gen);
}

bool EmitterAARCH64Stat::emitPLTJump(func_instance *callee, codeGen &gen) {
    assert(0); //Not implemented
    return emitPLTCommon(callee, false, gen);
}

bool EmitterAARCH64Stat::emitTOCCall(block_instance *block, codeGen &gen) {
    assert(0); //Not implemented
    return emitTOCCommon(block, true, gen);
}

bool EmitterAARCH64Stat::emitTOCJump(block_instance *block, codeGen &gen) {
    assert(0); //Not implemented
    return emitTOCCommon(block, false, gen);
}

bool EmitterAARCH64Stat::emitTOCCommon(block_instance *block, bool call, codeGen &gen) {
    assert(0); //Not implemented
    return false;
}

bool EmitterAARCH64Stat::emitCallInstruction(codeGen &gen,
                                             func_instance *callee,
                                             bool setTOC, Address) {
    assert(0); //Not implemented
    return true;
}

// Generates call instruction sequence for all AARCH64-based systems
// under dynamic instrumentation.
//
// This should be able to stomp on the link register (LR) and TOC
// register (r2), as they were saved by Emitter::emitCall() as necessary.
bool EmitterAARCH64::emitCallInstruction(codeGen &gen, func_instance *callee, bool setTOC, Address toc_anchor) {
    assert(0); //Not implemented
    return true;
}

void EmitterAARCH64::emitLoadShared(opCode op, Register dest, const image_variable *var, bool is_local, int size,
                                    codeGen &gen, Address offset) {
    assert(0); //Not implemented
    return;
}

void
EmitterAARCH64::emitStoreShared(Register source, const image_variable *var, bool is_local, int size, codeGen &gen) {
    assert(0); //Not implemented
    return;
}

Address Emitter::getInterModuleVarAddr(const image_variable *var, codeGen &gen) {
    assert(0); //Not implemented
    AddressSpace *addrSpace = gen.addrSpace();
    if (!addrSpace)
        assert(0 && "No AddressSpace associated with codeGen object");

    BinaryEdit *binEdit = addrSpace->edit();
    Address relocation_address;
    return relocation_address;
}

Address EmitterAARCH64::emitMovePCToReg(Register dest, codeGen &gen) {
    assert(0); //Not implemented
    insnCodeGen::generateBranch(gen, gen.currAddr(), gen.currAddr() + 4, true); // blrl
    Address ret = gen.currAddr();
    return ret;
}

Address Emitter::getInterModuleFuncAddr(func_instance *func, codeGen &gen) {
    assert(0); //Not implemented
    return NULL;
}
