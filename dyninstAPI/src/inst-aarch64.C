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

    std::vector < registerSlot * > registers;

    //GPRs
    for (unsigned idx = r0; idx <= r28; idx++) {
        char name[32];
        sprintf(name, "r%u", idx - r0);
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
        sprintf(name, "fpr%u", idx - fpr0);
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

void EmitterAARCH64SaveRegs::saveSPR(codeGen &gen, Register scratchReg, int sprnum, int stkOffset)
{
    assert(scratchReg!=Null_Register);

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

    insnCodeGen::generateMemAccess(gen, insnCodeGen::Store, scratchReg,
            REG_SP, stkOffset, 4, insnCodeGen::Pre);
}


void EmitterAARCH64SaveRegs::saveFPRegister(codeGen &gen, Register reg, int save_off) {
    //Always performing save of the full FP register
    insnCodeGen::generateMemAccessFP(gen, insnCodeGen::Store, reg, REG_SP, save_off, 0, true);

}

/********************************* Public methods *********************************************/

unsigned EmitterAARCH64SaveRegs::saveGPRegisters(
        codeGen &gen, registerSpace *theRegSpace, int offset, int numReqGPRs)
{
    int ret = 0;
    if(numReqGPRs == -1) numReqGPRs = theRegSpace->numGPRs();

    for(int idx = 0; idx < numReqGPRs; idx++) {
        registerSlot *reg = theRegSpace->GPRs()[idx];
	// We always save FP and LR for stack walking out of instrumentation
        if (reg->liveState == registerSlot::live || reg->number == REG_FP || reg->number == REG_LR) {
            int offset_from_sp = offset + (reg->encoding() * gen.width());
            insnCodeGen::saveRegister(gen, reg->number, offset_from_sp);
            theRegSpace->markSavedRegister(reg->number, offset_from_sp);
            ret++;
        }
    }

    return ret;
}

unsigned EmitterAARCH64SaveRegs::saveFPRegisters(
        codeGen &gen, registerSpace *theRegSpace, int offset)
{
    unsigned ret = 0;

    for(int idx = 0; idx < theRegSpace->numFPRs(); idx++) {
        registerSlot *reg = theRegSpace->FPRs()[idx];

        //if(reg->liveState == registerSlot::live) {
            int offset_from_sp = offset + (reg->encoding() * FPRSIZE_64);
            saveFPRegister(gen, reg->number, offset_from_sp);
            //reg->liveState = registerSlot::spilled;
            theRegSpace->markSavedRegister(reg->number, offset_from_sp);
            ret++;
        //}
    }

    return ret;
}

unsigned EmitterAARCH64SaveRegs::saveSPRegisters(
        codeGen &gen, registerSpace *theRegSpace, int offset, bool force_save)
{
    int ret = 0;

    std::vector<registerSlot *> spRegs;
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

    for(std::vector<registerSlot *>::iterator itr = spRegs.begin(); itr != spRegs.end(); itr++) {
        registerSlot *cur = *itr;
        saveSPR(gen, theRegSpace->getScratchRegister(gen, true), regMap[cur], -4*GPRSIZE_32);
        theRegSpace->markSavedRegister(cur->number, offset);

        offset += 4*GPRSIZE_32;
        ret++;
    }

    return ret;
}

void EmitterAARCH64SaveRegs::createFrame(codeGen &gen) {
    //Save link register
    Register linkRegister = gen.rs()->getRegByName("r30");
    insnCodeGen::saveRegister(gen, linkRegister, -2*GPRSIZE_64);

    //Save frame pointer
    Register framePointer = gen.rs()->getRegByName("r29");
    insnCodeGen::saveRegister(gen, framePointer, -2*GPRSIZE_64);

    //Move stack pointer to frame pointer
    Register stackPointer = gen.rs()->getRegByName("sp");
    insnCodeGen::generateMoveSP(gen, stackPointer, framePointer, true);
}

/***********************************************************************************************/
/***********************************************************************************************/

/********************************* EmitterAARCH64RestoreRegs ************************************/

/********************************* Public methods *********************************************/

unsigned EmitterAARCH64RestoreRegs::restoreGPRegisters(
        codeGen &gen, registerSpace *theRegSpace, int offset)
{
    unsigned ret = 0;

    for(int idx = theRegSpace->numGPRs()-1; idx >=0; idx--) {
        registerSlot *reg = theRegSpace->GPRs()[idx];

        if(reg->liveState == registerSlot::spilled) {
            //#sasha this should be GPRSIZE_64 and not gen.width
            int offset_from_sp = offset + (reg->encoding() * gen.width());
            insnCodeGen::restoreRegister(gen, reg->number, offset_from_sp);
            ret++;
        }
    }

    return ret;
}

unsigned EmitterAARCH64RestoreRegs::restoreFPRegisters(
        codeGen &gen, registerSpace *theRegSpace, int offset)
{
    unsigned ret = 0;

    for(int idx = theRegSpace->numFPRs() - 1; idx >= 0; idx--) {
        registerSlot *reg = theRegSpace->FPRs()[idx];

        //if(reg->liveState == registerSlot::spilled) {
            int offset_from_sp = offset + (reg->encoding() * FPRSIZE_64);
            restoreFPRegister(gen, reg->number, offset_from_sp);
            ret++;
        //}
    }

    return ret;
}

unsigned EmitterAARCH64RestoreRegs::restoreSPRegisters(
        codeGen &gen, registerSpace *theRegSpace, int, int force_save)
{
    int ret = 0;

    std::vector<registerSlot *> spRegs;
    map<registerSlot *, int> regMap;


    registerSlot *regFpsr = (*theRegSpace)[registerSpace::fpsr];
    assert(regFpsr);
    regMap[regFpsr] = SPR_FPSR;
    if(force_save || regFpsr->liveState == registerSlot::spilled)
        spRegs.push_back(regFpsr);

    registerSlot *regFpcr = (*theRegSpace)[registerSpace::fpcr];
    assert(regFpcr);
    regMap[regFpcr] = SPR_FPCR;
    if(force_save || regFpcr->liveState == registerSlot::spilled)
        spRegs.push_back(regFpcr);

    registerSlot *regNzcv = (*theRegSpace)[registerSpace::pstate];
    assert(regNzcv);
    regMap[regNzcv] = SPR_NZCV;
    if(force_save || regNzcv->liveState == registerSlot::spilled)
        spRegs.push_back(regNzcv);

    for(std::vector<registerSlot *>::iterator itr = spRegs.begin(); itr != spRegs.end(); itr++) {
        registerSlot *cur = *itr;
        restoreSPR(gen, theRegSpace->getScratchRegister(gen, true), regMap[cur], 4*GPRSIZE_32);
        ret++;
    }

    return ret;
}

void EmitterAARCH64RestoreRegs::tearFrame(codeGen &gen) {
    //Restore frame pointer
    Register framePointer = gen.rs()->getRegByName("r29");
    insnCodeGen::restoreRegister(gen, framePointer, 2*GPRSIZE_64);

    //Restore link register
    Register linkRegister = gen.rs()->getRegByName("r30");
    insnCodeGen::restoreRegister(gen, linkRegister, 2*GPRSIZE_64);
}


/********************************* Private methods *********************************************/

void EmitterAARCH64RestoreRegs::restoreSPR(codeGen &gen, Register scratchReg, int sprnum, int stkOffset)
{
    insnCodeGen::generateMemAccess(gen, insnCodeGen::Load, scratchReg, REG_SP, stkOffset, 4);

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

void EmitterAARCH64RestoreRegs::restoreFPRegister(codeGen &gen, Register reg, int save_off) {
    insnCodeGen::generateMemAccessFP(gen, insnCodeGen::Load, reg, REG_SP, save_off, 0, true);
}

/***********************************************************************************************/
/***********************************************************************************************/

/*
 * Emit code to push down the stack
 */
void pushStack(codeGen &gen)
{
    if (gen.width() == 8)
        insnCodeGen::generateAddSubImmediate(gen, insnCodeGen::Sub, 0,
                TRAMP_FRAME_SIZE_64, REG_SP, REG_SP, true);
    else
        assert(0); // 32 bit not implemented
}

void popStack(codeGen &gen)
{
    if (gen.width() == 8)
        insnCodeGen::generateAddSubImmediate(gen, insnCodeGen::Add, 0,
                TRAMP_FRAME_SIZE_64, REG_SP, REG_SP, true);
    else
        assert(0); // 32 bit not implemented
}

/*********************************** Base Tramp ***********************************************/
bool baseTramp::generateSaves(codeGen &gen, registerSpace *)
{
    regalloc_printf("========== baseTramp::generateSaves\n");

    // Make a stack frame.
    pushStack(gen);

    EmitterAARCH64SaveRegs saveRegs;
    unsigned int width = gen.width();

    saveRegs.saveGPRegisters(gen, gen.rs(), TRAMP_GPR_OFFSET(width));
    // After saving GPR, we move SP to FP to create the instrumentation frame.
    // Note that Dyninst instrumentation frame has a different structure
    // compared to stack frame created by the compiler.
    //
    // Dyninst instrumentation frame makes sure that FP and SP are the same.
    // So, during stack walk, the FP retrived from the previous frame is 
    // the SP of the current instrumentation frame.
    //
    // Note: If the implementation of the instrumentation frame layout
    // needs to be changed, DyninstDynamicStepperImpl::getCallerFrameArch
    // in stackwalk/src/aarch64-swk.C also likely needs to be changed accordingly
    insnCodeGen::generateMoveSP(gen, REG_SP, REG_FP, true);
    gen.markRegDefined(REG_FP);

    bool saveFPRs = BPatch::bpatch->isForceSaveFPROn() ||
                   (BPatch::bpatch->isSaveFPROn()      &&
                    gen.rs()->anyLiveFPRsAtEntry()     &&
                    this->saveFPRs());

    if(saveFPRs) saveRegs.saveFPRegisters(gen, gen.rs(), TRAMP_FPR_OFFSET(width));
    this->savedFPRs = saveFPRs;

    saveRegs.saveSPRegisters(gen, gen.rs(), TRAMP_SPR_OFFSET(width), false);
    //gen.rs()->debugPrint();

    return true;
}

bool baseTramp::generateRestores(codeGen &gen, registerSpace *)
{
    EmitterAARCH64RestoreRegs restoreRegs;
    unsigned int width = gen.width();

    restoreRegs.restoreSPRegisters(gen, gen.rs(), TRAMP_SPR_OFFSET(width), false);

    if(this->savedFPRs)
        restoreRegs.restoreFPRegisters(gen, gen.rs(), TRAMP_FPR_OFFSET(width));

    restoreRegs.restoreGPRegisters(gen, gen.rs(), TRAMP_GPR_OFFSET(width));

    // Tear down the stack frame.
    popStack(gen);

    return true;
}

/***********************************************************************************************/
/***********************************************************************************************/

//TODO: 32-/64-bit regs?
void emitImm(opCode op, Register src1, RegValue src2imm, Register dest, 
        codeGen &gen, bool /*noCost*/, registerSpace * /* rs */, bool s)
{
    switch(op) {
        case plusOp:
        case minusOp:
            {
                Register rm = insnCodeGen::moveValueToReg(gen, src2imm);
                insnCodeGen::generateAddSubShifted(gen,
                        op == plusOp ? insnCodeGen::Add : insnCodeGen::Sub,
                        0, 0, rm, src1, dest, true);
            }
            break;
        case timesOp:
            {
                Register rm = insnCodeGen::moveValueToReg(gen, src2imm);
                insnCodeGen::generateMul(gen, rm, src1, dest, true);
            }
            break;
        case divOp:
            {
                Register rm = insnCodeGen::moveValueToReg(gen, src2imm);
                insnCodeGen::generateDiv(gen, rm, src1, dest, true, s);
            }
            break;
        case xorOp:
            {
                Register rm = insnCodeGen::moveValueToReg(gen, src2imm);
                insnCodeGen::generateBitwiseOpShifted(gen, insnCodeGen::Eor, 0, rm, 0, src1, dest, true);
            }
            break;
        case orOp:
            {
                Register rm = insnCodeGen::moveValueToReg(gen, src2imm);
                insnCodeGen::generateBitwiseOpShifted(gen, insnCodeGen::Or, 0, rm, 0, src1, dest, true);
            }
            break;
        case andOp:
            {
                Register rm = insnCodeGen::moveValueToReg(gen, src2imm);
                insnCodeGen::generateBitwiseOpShifted(gen, insnCodeGen::And, 0, rm, 0, src1, dest, true);
            }
            break;
        case eqOp:
            {
                Register scratch = gen.rs()->getScratchRegister(gen);
                emitVload(loadConstOp, src2imm, 0, scratch, gen, true);
                emitV(op, src1, scratch, dest, gen, true);
            }
            break;
        case neOp:
        case lessOp:
        case leOp:
        case greaterOp:
        case geOp:
            // note that eqOp could be grouped here too.
            // There's two ways to implement this.
            gen.codeEmitter()->emitRelOpImm(op, dest, src1, src2imm, gen, s);
            return;
        default:
            assert(0); // not implemented or not valid
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
        for(std::set<Register>::iterator itr = fpRegs->begin(); itr != fpRegs->end(); itr++) {
            if (*itr <= rs->FPRs().size())
              rs->FPRs()[*itr]->beenUsed = true;
            else
              // parse_func::calcUsedRegs includes the subtype; we only want the regno
              rs->FPRs()[*itr & 0xff]->beenUsed = true;
        }
    } else {
        for(int idx = 0; idx < rs->numGPRs(); idx++)
            rs->GPRs()[idx]->beenUsed = true;
        for(int idx = 0; idx < rs->numFPRs(); idx++)
            rs->FPRs()[idx]->beenUsed = true;
    }

    stats_codegen.stopTimer(CODEGEN_LIVENESS_TIMER);

    return false;
}

Register emitFuncCall(opCode, codeGen &, std::vector <AstNodePtr> &, bool, Address) {
    assert(0);
    return 0;
}

Register emitFuncCall(opCode op,
                      codeGen &gen,
                      std::vector <AstNodePtr> &operands, bool noCost,
                      func_instance *callee) {
    return gen.emitter()->emitCall(op, gen, operands, noCost, callee);
}

Register EmitterAARCH64::emitCallReplacement(opCode,
                                             codeGen &,
                                             bool,
                                             func_instance *) {
    assert(0); //Not implemented
    return 0;
}

// There are four "axes" going on here:
// 32 bit vs 64 bit
// Instrumentation vs function call replacement
// Static vs. dynamic

Register EmitterAARCH64::emitCall(opCode op,
                                  codeGen &gen,
                                  const std::vector<AstNodePtr> &operands,
                                  bool,
                                  func_instance *callee) 
{
    //#sasha This function implementation is experimental.

    if (op != callOp) {
        cerr << "ERROR: emitCall with op == " << op << endl;
    }
    assert(op == callOp);

    std::vector<Register> srcs;
    std::vector<Register> saves;

    //  Sanity check for NULL address arg
    if (!callee) 
    {
        char msg[256];
        sprintf(msg, "%s[%d]:  internal error:  emitFuncCall called w/out"
                "callee argument", __FILE__, __LINE__);
        showErrorCallback(80, msg);
        assert(0);
    }

    vector<int> savedRegs;

    // save r0-r7
    for(int id = 0; id < gen.rs()->numGPRs(); id++)
    {
        registerSlot *reg = gen.rs()->GPRs()[id];

        // We must save if:
        // refCount > 0 (and not a source register)
        // keptValue == true (keep over the call)
        // liveState == live (technically, only if not saved by the callee)

        if ((reg->refCount > 0) || reg->keptValue || (reg->liveState == registerSlot::live))
        {
            insnCodeGen::saveRegister(gen, registerSpace::r0 + id,
                    -2*GPRSIZE_64, insnCodeGen::Post);
            savedRegs.push_back(reg->number);
        }
    }

    // Passing operands to registers
    for(size_t id = 0; id < operands.size(); id++)
    {
        Register reg = Null_Register;
        if (gen.rs()->allocateSpecificRegister(gen, registerSpace::r0 + id, true))
            reg = registerSpace::r0 + id;

        Address unnecessary = ADDR_NULL;
        if (!operands[id]->generateCode_phase2(gen, false, unnecessary, reg))
            assert(0);
        assert(reg!=Null_Register);
    }

    assert(gen.rs());

    // Address of function to call in scratch register
    Register scratch = gen.rs()->getScratchRegister(gen);
    assert(scratch != Null_Register && "cannot get a scratch register");
    gen.markRegDefined(scratch);

    if (gen.addrSpace()->edit() != NULL
	&& (gen.func()->obj() != callee->obj()
	    || gen.addrSpace()->needsPIC())) {
        // gen.as.edit() checks if we are in rewriter mode
        Address dest = getInterModuleFuncAddr(callee, gen);

        // emit ADR instruction

        long disp = dest - gen.currAddr();
        instruction insn;
        insn.clear();
        INSN_SET(insn, 31, 31, 0);
        INSN_SET(insn, 28, 28, 1);
        INSN_SET(insn, 5, 23, disp >> 2);
        INSN_SET(insn, 0, 4, scratch);
        insnCodeGen::generate(gen, insn);

        insnCodeGen::generateMemAccess(gen, insnCodeGen::Load, scratch, scratch, 0, 8, insnCodeGen::Offset);
    } else {
        insnCodeGen::loadImmIntoReg(gen, scratch, callee->addr());
    }

    instruction branchInsn;
    branchInsn.clear();

    //Set bits which are 0 for both BR and BLR
    INSN_SET(branchInsn, 0, 4, 0);
    INSN_SET(branchInsn, 10, 15, 0);

    //Set register
    INSN_SET(branchInsn, 5, 9, scratch);

    //Set other bits. Basically, these are the opcode bits.
    //The only difference between BR and BLR is that bit 21 is 1 for BLR.
    INSN_SET(branchInsn, 16, 31, BRegOp);
    INSN_SET(branchInsn, 21, 21, 1);
    insnCodeGen::generate(gen, branchInsn);

    /*
     * Restoring registers
     */

    // r7-r0
    for (signed int ui = savedRegs.size()-1; ui >= 0; ui--) {
        insnCodeGen::restoreRegister(gen, registerSpace::r0 + savedRegs[ui],
                2*GPRSIZE_64, insnCodeGen::Post);
    }

    return 0;
}


codeBufIndex_t emitA(opCode op, Register src1, Register, long dest,
        codeGen &gen, RegControl rc, bool)
{
    codeBufIndex_t retval = 0;

    switch (op) {
        case ifOp: 
            {
                // if src1 == 0 jump to dest
                // src1 is a temporary
                // dest is a target address
                retval = gen.codeEmitter()->emitIf(src1, dest, rc, gen);
                break;
            }
        case branchOp:
            {
                insnCodeGen::generateBranch(gen, dest);
                retval = gen.getIndex();
                break;
            }
        default:
            assert(0);        // op not implemented or not expected for this emit!
    }

    return retval;
}

Register emitR(opCode op, Register src1, Register src2, Register dest,
               codeGen &gen, bool /*noCost*/,
               const instPoint *, bool /*for_MT*/)
{
    registerSlot *regSlot = NULL;
    unsigned addrWidth = gen.width();

    switch(op){
        case getRetValOp:
            regSlot = (*(gen.rs()))[registerSpace::r0];
            break;
        case getParamOp:
            // src1 is the number of the argument
            // dest is a register where we can store the value
            //gen.codeEmitter()->emitGetParam(dest, src1, location->type(), op,
            //        false, gen);

            if(src1 <= 7) {
                // src1 is 0..7 - it's a parameter order number, not a register
                regSlot = (*(gen.rs()))[registerSpace::r0 + src1];
                break;

            } else {
                int stkOffset = TRAMP_FRAME_SIZE_64 + (src1 - 8) * sizeof(long);
                // printf("TRAMP_FRAME_SIZE_64: %d\n", TRAMP_FRAME_SIZE_64);
                // printf("stdOffset = TRAMP_xxx_64 + (argc - 8) * 8 = { %d }\n", stkOffset);
                // TODO: PARAM_OFFSET(addrWidth) is currently not used
                // should delete that macro if it's useless

                if (src2 != Null_Register) insnCodeGen::saveRegister(gen, src2, stkOffset);
                insnCodeGen::restoreRegister(gen, dest, stkOffset);

                return dest;
            }
            break;
        default:
            assert(0);
    }

    assert(regSlot);
    Register reg = regSlot->number;

    switch(regSlot->liveState) {
        case registerSlot::spilled:
            {
                int offset = TRAMP_GPR_OFFSET(addrWidth);
                // its on the stack so load it.
                //if (src2 != Null_Register) saveRegister(gen, src2, reg, offset);
                insnCodeGen::restoreRegister(gen, dest, offset + (reg * gen.width()));
                return(dest);
            }
        case registerSlot::live:
            {
                // its still in a register so return the register it is in.
                cerr << "emitR state:" << reg << " live" << endl;
                assert(0);
                return(reg);
            }
        case registerSlot::dead:
            {
                cerr << "emitR state" << reg << ": dead" << endl;
                // Uhhh... wha?
                assert(0);
            }
    }
    return reg;
}

void emitJmpMC(int /*condition*/, int /*offset*/, codeGen &) {
    assert(0); //Not implemented
    // Not needed for memory instrumentation, otherwise TBD
}


// VG(03/15/02): Restore mutatee value of GPR reg to dest GPR
static inline void restoreGPRtoGPR(codeGen &gen,
                                   Register reg, Register dest) {
    int frame_size, gpr_size, gpr_off;

	frame_size = TRAMP_FRAME_SIZE_64;
    gpr_size   = GPRSIZE_64;
    gpr_off    = TRAMP_GPR_OFFSET_64;	
	
	//Stack Point Register
	if(reg == 31) {
	    insnCodeGen::generateAddSubImmediate(gen, insnCodeGen::Add, 0, frame_size, REG_SP, dest, true);	
	}
	else {
		insnCodeGen::restoreRegister(gen, dest, gpr_off + reg*gpr_size);
	}
	
	return;
}

// VG(03/15/02): Restore mutatee value of XER to dest GPR
static inline void restoreXERtoGPR(codeGen &, Register) {
    assert(0); //Not implemented
}

// VG(03/15/02): Move bits 25:31 of GPR reg to GPR dest
static inline void moveGPR2531toGPR(codeGen &,
                                    Register, Register) {
    assert(0); //Not implemented
}

// VG(11/16/01): Emit code to add the original value of a register to
// another. The original value may need to be restored from stack...
// VG(03/15/02): Made functionality more obvious by adding the above functions
static inline void emitAddOriginal(Register src, Register acc,
                                   codeGen &gen, bool noCost) {
    emitV(plusOp, src, acc, acc, gen, noCost, 0);
}


//Not correctly implemented
void MovePCToReg(Register dest, codeGen &gen) {
    instruction insn;
    insn.clear();

    INSN_SET(insn, 28, 28, 1);
    INSN_SET(insn, 0, 4, dest);

    insnCodeGen::generate(gen, insn);
    return;
}

// Yuhan(02/04/19): Load in destination the effective address given
// by the address descriptor. Used for memory access stuff.
void emitASload(const BPatch_addrSpec_NP *as, Register dest, int stackShift,
                codeGen &gen,
                bool) {

    // Haven't implemented non-zero shifts yet
    assert(stackShift == 0);
    long int imm = as->getImm();
    int ra  = as->getReg(0);
    int rb  = as->getReg(1);
    int sc  = as->getScale();
    gen.markRegDefined(dest);
    if(ra > -1) {
        if(ra == 32) {
	    // Special case where the actual address is store in imm.
	    // Need to change this for rewriting PIE or shared libraries
	    insnCodeGen::loadImmIntoReg(gen, dest, static_cast<Address>(imm));
	    return;
	}
	else {
	    restoreGPRtoGPR(gen, ra, dest);
	}
    } else {
        insnCodeGen::loadImmIntoReg(gen, dest, static_cast<Address>(0));
    }
    if(rb > -1) {
        std::vector<Register> exclude;
	exclude.push_back(dest);
        Register scratch = gen.rs()->getScratchRegister(gen, exclude);
        assert(scratch != Null_Register && "cannot get a scratch register");
        gen.markRegDefined(scratch);
        restoreGPRtoGPR(gen, rb, scratch);
    	// call adds, save 2^scale * rb to dest
	insnCodeGen::generateAddSubShifted(gen, insnCodeGen::Add, 0, sc, scratch, dest, dest, true);
    }
	
    // emit code to load the immediate (constant offset) into dest; this
    // writes at gen+base and updates base, we must update insn..
    if (imm) 
        insnCodeGen::generateAddSubImmediate(gen, insnCodeGen::Add, 0, imm, dest, dest, true);	
}

void emitCSload(const BPatch_addrSpec_NP *, Register, codeGen &,
                bool) {
    assert(0); //Not implemented
}

void emitVload(opCode op, Address src1, Register src2, Register dest,
               codeGen &gen, bool /*noCost*/,
               registerSpace * /*rs*/, int size,
               const instPoint * /* location */, AddressSpace *)
{
    switch(op)
    {
        case loadConstOp:
            // dest is a temporary
            // src1 is an immediate value
            // dest = src1:imm32
            gen.codeEmitter()->emitLoadConst(dest, src1, gen);
            break;
        case loadOp:
            // dest is a temporary
            // src1 is the address of the operand
            // dest = [src1]
            gen.codeEmitter()->emitLoad(dest, src1, size, gen);
            break;
        case loadRegRelativeAddr:
            // (readReg(src2) + src1)
            // dest is a temporary
            // src2 is the register
            // src1 is the offset from the address in src2
            gen.codeEmitter()->emitLoadOrigRegRelative(dest, src1, src2, gen, false);
            break;
        case loadRegRelativeOp:
            // *(readReg(src2) + src1)
            // dest is a temporary
            // src2 is the register
            // src1 is the offset from the address in src2
            gen.codeEmitter()->emitLoadOrigRegRelative(dest, src1, src2, gen, true);
            break;
        default:
            assert(0); //Not implemented
            break;
    }
}

void emitVstore(opCode op, Register src1, Register /*src2*/, Address dest,
        codeGen &gen, bool,
        registerSpace * /* rs */, int size,
        const instPoint * /* location */, AddressSpace *)
{
    if (op ==  storeOp) {
        // [dest] = src1
        // dest has the address where src1 is to be stored
        // src1 is a temporary
        // src2 is a "scratch" register, we don't need it in this architecture
        gen.codeEmitter()->emitStore(dest, src1, size, gen);
    }else{
        assert(0); //Not implemented
    }
    return;
}

void emitV(opCode op, Register src1, Register src2, Register dest,
        codeGen &gen, bool /*noCost*/,
           registerSpace * /*rs*/, int size,
           const instPoint * /* location */, AddressSpace *proc, bool s) 
{
    switch(op){
        case plusOp:
        case minusOp:
        case timesOp:
        case orOp:
        case andOp:
        case xorOp:
            gen.codeEmitter()->emitOp(op, dest, src1, src2, gen);
            break;
        case divOp:
	    insnCodeGen::generateDiv(gen, src2, src1, dest, true, s);
	    break;
        case lessOp:
        case leOp:
        case greaterOp:
        case geOp:
        case eqOp:
        case neOp:
            gen.codeEmitter()->emitRelOp(op, dest, src1, src2, gen, s);
            break;
        case loadIndirOp:
            size = !size ? proc->getAddressWidth() : size;
            // same as loadOp, but the value to load is already in a register
            gen.codeEmitter()->emitLoadIndir(dest, src1, size, gen);
            break;
        case storeIndirOp:
            size = !size ? proc->getAddressWidth() : size;
            gen.codeEmitter()->emitStoreIndir(dest, src1, size, gen);
            break;
        default:
            //std::cout << "operation not implemented= " << op << endl;
            assert(0); // Not implemented
            break;
    }
    return;
}

//
// I don't know how to compute cycles for AARCH64 instructions due to
//   multiple functional units.  However, we can compute the number of
//   instructions and hope that is fairly close. - jkh 1/30/96
//
int getInsnCost(opCode) {
    assert(0); //Not implemented
    return 0;
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

// This is used for checking wether immediate value should be encoded
// into a instruction. In fact, only being used for loading constant
// value into a register, and in ARMv8 there are 16 bits for immediate
// values in the instruction MOV.
// value here is never a negative value since constant values are saved
// as void* in the AST operand.
bool doNotOverflow(int64_t value)
{
    if ((value >= 0) && (value <= 0xFFFF)) return true;
    else return false;
}


// hasBeenBound: returns true if the runtime linker has bound the
// function symbol corresponding to the relocation entry in at the address
// specified by entry and base_addr.  If it has been bound, then the callee
// function is returned in "target_pdf", else it returns false.
bool PCProcess::hasBeenBound(const SymtabAPI::relocationEntry &entry,
                             func_instance *&target_pdf, Address base_addr)
{
	if (isTerminated()) return false;

	// if the relocationEntry has not been bound yet, then the value
	// at rel_addr is the address of the instruction immediately following
	// the first instruction in the PLT entry (which is at the target_addr)
	// The PLT entries are never modified, instead they use an indirrect
	// jump to an address stored in the _GLOBAL_OFFSET_TABLE_.  When the
	// function symbol is bound by the runtime linker, it changes the address
	// in the _GLOBAL_OFFSET_TABLE_ corresponding to the PLT entry

	Address got_entry = entry.rel_addr() + base_addr;
	Address bound_addr = 0;
	if (!readDataSpace((const void*)got_entry, sizeof(Address),
				&bound_addr, true)){
		sprintf(errorLine, "read error in PCProcess::hasBeenBound addr 0x%x, pid=%d\n (readDataSpace returns 0)",(unsigned)got_entry,getPid());
		logLine(errorLine);
		//print_read_error_info(entry, target_pdf, base_addr);
		fprintf(stderr, "%s[%d]: %s\n", FILE__, __LINE__, errorLine);
		return false;
	}

   //fprintf(stderr, "%s[%d]:  hasBeenBound:  %p ?= %p ?\n", FILE__, __LINE__, bound_addr, entry.target_addr() + 6 + base_addr);
	if ( !( bound_addr == (entry.target_addr()+6+base_addr)) ) {
	  // the callee function has been bound by the runtime linker
	  // find the function and return it
	  target_pdf = findFuncByEntry(bound_addr);
	  if(!target_pdf){
	    return false;
	  }
	  return true;
	}
	return false;
}

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
                                        bool)
{
    gen.codeEmitter()->emitLoadOrigRegister(register_num, dest, gen);
}

void emitStorePreviousStackFrameRegister(Address,
                                         Register,
                                         codeGen &,
                                         int,
                                         bool) {
    assert(0);
}

// First AST node: target of the call
// Second AST node: source of the call
// This can handle indirect control transfers as well
bool AddressSpace::getDynamicCallSiteArgs(InstructionAPI::Instruction i,
					  Address addr,
					  std::vector<AstNodePtr> &args)
{
    using namespace Dyninst::InstructionAPI;
    Register branch_target = registerSpace::ignored;

    for(Instruction::cftConstIter curCFT = i.cft_begin();
            curCFT != i.cft_end(); ++curCFT)
    {
        auto target_reg = dynamic_cast<RegisterAST *>(curCFT->target.get());
        if(!target_reg) return false;
        branch_target = target_reg->getID() & 0x1f;
        break;
    }

    if(branch_target == registerSpace::ignored) return false;

    //jumping to Xn (BLR Xn)
    args.push_back(AstNode::operandNode(AstNode::operandType::origRegister,(void *)(long)branch_target));
    args.push_back(AstNode::operandNode(AstNode::operandType::Constant, (void *) addr));

    //inst_printf("%s[%d]:  Inserting dynamic call site instrumentation for %s\n",
    //        FILE__, __LINE__, cft->format(insn.getArch()).c_str());
    return true;
}

bool writeFunctionPtr(AddressSpace *p, Address addr, func_instance *f) {
    Address val_to_write = f->addr();
    return p->writeDataSpace((void *) addr, sizeof(Address), &val_to_write);
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

bool EmitterAARCH64::emitCallRelative(Register, Address, Register, codeGen &) {
    assert(0); //Not implemented
    return true;
}

bool EmitterAARCH64::emitLoadRelative(Register dest, Address offset, Register baseReg, int size, codeGen &gen) {
    signed long long sOffset = (signed long long) offset;
    if(sOffset >=-256 && sOffset <=255)
        insnCodeGen::generateMemAccess(gen, insnCodeGen::Load, dest,
                baseReg, sOffset, size, insnCodeGen::Pre);
    else{
        std::vector<Register> exclude;
        exclude.push_back(baseReg);
        // mov sOffset to a reg
        auto addReg = insnCodeGen::moveValueToReg(gen, labs(offset), &exclude);
        // add/sub sOffset to baseReg
        insnCodeGen::generateAddSubShifted(gen,
                sOffset>0?insnCodeGen::Add:insnCodeGen::Sub,
                0, 0, addReg, baseReg, baseReg, true);
        insnCodeGen::generateMemAccess(gen, insnCodeGen::Load, dest,
                baseReg, 0, size, insnCodeGen::Pre);
    }

    gen.markRegDefined(dest);
    return true;
}


void EmitterAARCH64::emitStoreRelative(Register source, Address offset, Register base, int size, codeGen &gen) {
    if((signed long long)offset <=255 && (signed long long)offset >=-256)
        insnCodeGen::generateMemAccess(gen, insnCodeGen::Store, source,
                base, offset, size, insnCodeGen::Pre);
    else{
        assert(0 && "offset in emitStoreRelative not in (-256,255)");
        //insnCodeGen::generateMemAccess(gen, insnCodeGen::Store, source,
        //        base, offset, size, insnCodeGen::Pre);
    }
}

bool EmitterAARCH64::emitMoveRegToReg(registerSlot *,
                                      registerSlot *,
                                      codeGen &) {
    assert(0); //Not implemented
    return true;
}

/*
bool EmitterAARCH6432Stat::emitPIC(codeGen& gen, Address origAddr, Address relocAddr) {

      Register scratchPCReg = gen.rs()->getScratchRegister(gen, true);
      std::vector<Register> excludeReg;
      excludeReg.push_back(scratchPCReg);
      Register scratchReg = gen.rs()->getScratchRegister(gen, excludeReg, true);
      bool newStackFrame = false;
      int stack_size = 0;
      int gpr_off, fpr_off, ctr_off;
      //fprintf(stderr, " emitPIC origAddr 0x%lx reloc 0x%lx Registers PC %d scratch %d \n", origAddr, relocAddr, scratchPCReg, scratchReg);
      if ((scratchPCReg == Null_Register) || (scratchReg == Null_Register)) {
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
	      assert(scratchPCReg != Null_Register);
	      excludeReg.clear();
	      excludeReg.push_back(scratchPCReg);
	      scratchReg = gen.rs()->getScratchRegister(gen, excludeReg, true);
	      assert(scratchReg != Null_Register);
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
	assert(scratch != Null_Register);
	instruction::loadImmIntoReg(gen, scratch, origRet);
	insnCodeGen::generateMoveToLR(gen, scratch);
	return true;

}
*/

bool EmitterAARCH64Stat::emitPLTCommon(func_instance *, bool, codeGen &) {
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
  if (scratchReg == Null_Register) {
    std::vector<Register> freeReg;
    std::vector<Register> excludeReg;
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

bool EmitterAARCH64Dyn::emitTOCCommon(block_instance *, bool, codeGen &) {
    assert(0); //Not implemented
    return true;
}

bool EmitterAARCH64Stat::emitPLTCall(func_instance *callee, codeGen &gen) {
    Address dest = getInterModuleFuncAddr(callee, gen);
    long varOffset = dest - gen.currAddr();

    Register baseReg = gen.rs()->getScratchRegister(gen, true);
    assert(baseReg != Null_Register && "cannot get a scratch register");
    emitMovePCToReg(baseReg, gen);

    std::vector<Register> exclude;
    exclude.push_back(baseReg);
    // mov offset to a reg
    auto addReg = insnCodeGen::moveValueToReg(gen, labs(varOffset), &exclude);
    // add/sub offset to baseReg
    insnCodeGen::generateAddSubShifted(gen,
            (signed long long) varOffset>0?insnCodeGen::Add:insnCodeGen::Sub,
            0, 0, addReg, baseReg, baseReg, true);
    insnCodeGen::generateMemAccess(gen, insnCodeGen::Load, baseReg,
            baseReg, 0, 8, insnCodeGen::Offset);

    // call instruction
    instruction branchInsn;
    branchInsn.clear();
    //Set bits which are 0 for both BR and BLR
    INSN_SET(branchInsn, 0, 4, 0);
    INSN_SET(branchInsn, 10, 15, 0);
    //Set register
    INSN_SET(branchInsn, 5, 9, baseReg);
    //Set other bits. Basically, these are the opcode bits.
    //The only difference between BR and BLR is that bit 21 is 1 for BLR.
    INSN_SET(branchInsn, 16, 31, BRegOp);
    INSN_SET(branchInsn, 21, 21, 1);
    insnCodeGen::generate(gen, branchInsn);

    return true;
}

bool EmitterAARCH64Stat::emitPLTJump(func_instance *callee, codeGen &gen) {
    Address dest = getInterModuleFuncAddr(callee, gen);
    long varOffset = dest - gen.currAddr();

    Register baseReg = gen.rs()->getScratchRegister(gen, true);
    assert(baseReg != Null_Register && "cannot get a scratch register");
    emitMovePCToReg(baseReg, gen);

    std::vector<Register> exclude;
    exclude.push_back(baseReg);
    // mov offset to a reg
    auto addReg = insnCodeGen::moveValueToReg(gen, labs(varOffset), &exclude);
    // add/sub offset to baseReg
    insnCodeGen::generateAddSubShifted(gen,
            (signed long long) varOffset>0?insnCodeGen::Add:insnCodeGen::Sub,
            0, 0, addReg, baseReg, baseReg, true);
    insnCodeGen::generateMemAccess(gen, insnCodeGen::Load, baseReg,
            baseReg, 0, 8, insnCodeGen::Offset);

    // jump instruction
    instruction branchInsn;
    branchInsn.clear();
    //Set bits which are 0 for both BR and BLR
    INSN_SET(branchInsn, 0, 4, 0);
    INSN_SET(branchInsn, 10, 15, 0);
    //Set register
    INSN_SET(branchInsn, 5, 9, baseReg);
    //Set other bits. Basically, these are the opcode bits.
    //The only difference between BR and BLR is that bit 21 is 1 for BLR.
    INSN_SET(branchInsn, 16, 31, BRegOp);
    insnCodeGen::generate(gen, branchInsn);

    return true;
}

bool EmitterAARCH64Stat::emitTOCCall(block_instance *block, codeGen &gen) {
    assert(0); //Not implemented
    return emitTOCCommon(block, true, gen);
}

bool EmitterAARCH64Stat::emitTOCJump(block_instance *block, codeGen &gen) {
    assert(0); //Not implemented
    return emitTOCCommon(block, false, gen);
}

bool EmitterAARCH64Stat::emitTOCCommon(block_instance *, bool, codeGen &) {
    assert(0); //Not implemented
    return false;
}

bool EmitterAARCH64Stat::emitCallInstruction(codeGen &,
                                             func_instance *,
                                             bool, Address) {
    assert(0); //Not implemented
    return true;
}

// Generates call instruction sequence for all AARCH64-based systems
// under dynamic instrumentation.
//
// This should be able to stomp on the link register (LR) and TOC
// register (r2), as they were saved by Emitter::emitCall() as necessary.
bool EmitterAARCH64::emitCallInstruction(codeGen &, func_instance *, bool, Address) {
    assert(0); //Not implemented
    return true;
}

void EmitterAARCH64::emitLoadShared(opCode op, Register dest, const image_variable *var,
        bool is_local, int size, codeGen &gen, Address offset)
{
    // create or retrieve jump slot
    Address addr;
    int stackSize = 0;

    if(!var) {
        addr = offset;
    }
    else if(!is_local) {
        addr = getInterModuleVarAddr(var, gen);
    }
    else {
        addr = (Address)var->getOffset();
    }

    // load register with address from jump slot
    Register baseReg = gen.rs()->getScratchRegister(gen, true);
    assert(baseReg != Null_Register && "cannot get a scratch register");

    emitMovePCToReg(baseReg, gen);
    Address varOffset = addr - gen.currAddr() + 4;

    if (op ==loadOp ) {
        if(!is_local && (var != NULL)){
            emitLoadRelative(dest, varOffset, baseReg, gen.width(), gen);
            // Deference the pointer to get the variable
            // emitLoadRelative(dest, 0, dest, size, gen);
            // Offset mode to load back to itself
            insnCodeGen::generateMemAccess(gen, insnCodeGen::Load, dest, dest, 0, 8,
                    insnCodeGen::Offset);
        } else {
            emitLoadRelative(dest, varOffset, baseReg, size, gen);
        }
    } else { //loadConstop
        if(!is_local && (var != NULL)){
            emitLoadRelative(dest, varOffset, baseReg, gen.width(), gen);
        } else {
            std::vector<Register> exclude;
            exclude.push_back(baseReg);
            auto addReg = insnCodeGen::moveValueToReg(gen, labs(varOffset), &exclude);
            insnCodeGen::generateAddSubShifted(gen,
                    (signed long long) varOffset>0?insnCodeGen::Add:insnCodeGen::Sub,
                    0, 0, addReg, baseReg, baseReg, true);
            insnCodeGen::generateMove(gen, dest, baseReg, true);
        }
    }

    assert(stackSize <= 0 && "stack not empty at the end");
}

void EmitterAARCH64::emitStoreShared(Register source, const image_variable *var,
        bool is_local, int size, codeGen &gen)
{
    // create or retrieve jump slot
    Address addr;
    int stackSize = 0;
    if(!is_local) {
        addr = getInterModuleVarAddr(var, gen);
    }
    else {
        addr = (Address)var->getOffset();
    }

    // load register with address from jump slot
    Register baseReg = gen.rs()->getScratchRegister(gen, true);
    assert(baseReg != Null_Register && "cannot get a scratch register");

    emitMovePCToReg(baseReg, gen);
    Address varOffset = addr - gen.currAddr() + 4;

    if(!is_local) {
        std::vector<Register> exclude;
        exclude.push_back(baseReg);
        Register scratchReg1 = gen.rs()->getScratchRegister(gen, exclude, true);
        assert(scratchReg1 != Null_Register && "cannot get a scratch register");
        emitLoadRelative(scratchReg1, varOffset, baseReg, gen.width(), gen);
        emitStoreRelative(source, 0, scratchReg1, size, gen);
    } else {
        std::vector<Register> exclude;
        exclude.push_back(baseReg);
        // mov offset to a reg
        auto addReg = insnCodeGen::moveValueToReg(gen, labs(varOffset), &exclude);
        // add/sub offset to baseReg
        insnCodeGen::generateAddSubShifted(gen,
                (signed long long) varOffset>0?insnCodeGen::Add:insnCodeGen::Sub,
                0, 0, addReg, baseReg, baseReg, true);
        insnCodeGen::generateMemAccess(gen, insnCodeGen::Store, source,
                baseReg, 0, size, insnCodeGen::Pre);
    }

    assert(stackSize <= 0 && "stack not empty at the end");
}

Address Emitter::getInterModuleVarAddr(const image_variable *var, codeGen &gen) {
    AddressSpace *addrSpace = gen.addrSpace();
    if (!addrSpace)
        assert(0 && "No AddressSpace associated with codeGen object");

    BinaryEdit *binEdit = addrSpace->edit();
    Address relocation_address;

    unsigned int jump_slot_size;
    switch (addrSpace->getAddressWidth()) {
        case 4: jump_slot_size = 4; break;
        case 8: jump_slot_size = 8; break;
        default: assert(0 && "Encountered unknown address width");
    }

    if (!binEdit || !var) {
        assert(!"Invalid variable load (variable info is missing)");
    }

    // find the Symbol corresponding to the int_variable
    std::vector<SymtabAPI::Symbol *> syms;
    var->svar()->getSymbols(syms);

    if (syms.size() == 0) {
        char msg[256];
        snprintf(msg, sizeof(msg), "%s[%d]:  internal error:  cannot find symbol %s"
                , __FILE__, __LINE__, var->symTabName().c_str());
        showErrorCallback(80, msg);
        assert(0);
    }

    // try to find a dynamic symbol
    // (take first static symbol if none are found)
    SymtabAPI::Symbol *referring = syms[0];
    for (unsigned k=0; k<syms.size(); k++) {
        if (syms[k]->isInDynSymtab()) {
            referring = syms[k];
            break;
        }
    }

    // have we added this relocation already?
    relocation_address = binEdit->getDependentRelocationAddr(referring);

    if (!relocation_address) {
        // inferiorMalloc addr location and initialize to zero
        relocation_address = binEdit->inferiorMalloc(jump_slot_size);
        unsigned char dat[8] = {0};
        binEdit->writeDataSpace((void*)relocation_address, jump_slot_size, dat);

        // add write new relocation symbol/entry
        binEdit->addDependentRelocation(relocation_address, referring);
    }

    return relocation_address;
}

Address EmitterAARCH64::emitMovePCToReg(Register dest, codeGen &gen) {
    instruction insn;
    insn.clear();

    INSN_SET(insn, 28, 28, 1);
    INSN_SET(insn, 0, 4, dest);

    insnCodeGen::generate(gen, insn);
    Address ret = gen.currAddr();
    return ret;
}

Address Emitter::getInterModuleFuncAddr(func_instance *func, codeGen &gen) {
    // from POWER64 getInterModuleFuncAddr

    AddressSpace *addrSpace = gen.addrSpace();
    if (!addrSpace)
        assert(0 && "No AddressSpace associated with codeGen object");

    BinaryEdit *binEdit = addrSpace->edit();
    Address relocation_address;
    
    unsigned int jump_slot_size;
    switch (addrSpace->getAddressWidth()) {
    case 4: jump_slot_size =  4; break; // l: not needed
    case 8: 
      jump_slot_size = 24;
      break;
    default: assert(0 && "Encountered unknown address width");
    }

    if (!binEdit || !func) {
        assert(!"Invalid function call (function info is missing)");
    }

    // find the Symbol corresponding to the func_instance
    std::vector<SymtabAPI::Symbol *> syms;
    func->ifunc()->func()->getSymbols(syms);

    if (syms.size() == 0) {
        char msg[256];
        snprintf(msg, sizeof(msg), "%s[%d]:  internal error:  cannot find symbol %s"
                , __FILE__, __LINE__, func->symTabName().c_str());
        showErrorCallback(80, msg);
        assert(0);
    }

    // try to find a dynamic symbol
    // (take first static symbol if none are found)
    SymtabAPI::Symbol *referring = syms[0];
    for (unsigned k=0; k<syms.size(); k++) {
        if (syms[k]->isInDynSymtab()) {
            referring = syms[k];
            break;
        }
    }
    // have we added this relocation already?
    relocation_address = binEdit->getDependentRelocationAddr(referring);

    if (!relocation_address) {
        // inferiorMalloc addr location and initialize to zero
        relocation_address = binEdit->inferiorMalloc(jump_slot_size);
        unsigned char dat[24] = {0};
        binEdit->writeDataSpace((void*)relocation_address, jump_slot_size, dat);
        // add write new relocation symbol/entry
        binEdit->addDependentRelocation(relocation_address, referring);
    }
    return relocation_address;
}




