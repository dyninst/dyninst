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
#include "codegen/RegControl.h"
#include "dyninstAPI/h/BPatch_memoryAccess_NP.h"
#include "dyninstAPI/src/image.h"
#include "dyninstAPI/src/dynProcess.h"
#include "dyninstAPI/src/inst-aarch64.h"
#include "common/src/arch-aarch64.h"
#include "dyninstAPI/src/codegen.h"
#include "dyninstAPI/src/util.h"
#include "common/src/stats.h"
#include "dyninstAPI/src/os.h"
#include "dyninstAPI/src/instPoint.h" // class instPoint
#include "dyninstAPI/src/debug.h"
#include "dyninstAPI/h/BPatch.h"
#include "BPatch_collections.h"
#include "registerSpace.h"
#include "dyninstAPI/src/binaryEdit.h"
#include "dyninstAPI/src/function.h"
#include "dyninstAPI/src/mapped_object.h"
#include "RegisterConversion.h"
#include "parseAPI/h/CFG.h"
#include "codegen/emitters/aarch64/EmitterAarch64Dyn.h"
#include "Instruction.h"
#include "Register.h"
#include "registers/aarch64_regs.h"

#include "emitter.h"
#include "emit-aarch64.h"

#include <boost/assign/list_of.hpp>
using namespace boost::assign;
#include <sstream>

#include "dyninstAPI/h/BPatch_memoryAccess_NP.h"

using codeGenASTPtr = Dyninst::DyninstAPI::codeGenASTPtr;
using operandAST = Dyninst::DyninstAPI::operandAST;


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

    registerSlot *regNzcv = (*theRegSpace)[registerSpace::nzcv];
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
        saveSPR(gen, theRegSpace->getScratchRegister(gen), regMap[cur], -4*GPRSIZE_32);
        theRegSpace->markSavedRegister(cur->number, offset);

        offset += 4*GPRSIZE_32;
        ret++;
    }

    return ret;
}

void EmitterAARCH64SaveRegs::createFrame(codeGen &gen) {
    //Save link register
    Register linkRegister = convertRegID(Dyninst::aarch64::lr);
    insnCodeGen::saveRegister(gen, linkRegister, -2*GPRSIZE_64);

    //Save frame pointer
    Register framePointer = convertRegID(Dyninst::aarch64::fp);
    insnCodeGen::saveRegister(gen, framePointer, -2*GPRSIZE_64);

    //Move stack pointer to frame pointer
    Register stackPointer = convertRegID(Dyninst::aarch64::sp);
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

    registerSlot *regNzcv = (*theRegSpace)[registerSpace::nzcv];
    assert(regNzcv);
    regMap[regNzcv] = SPR_NZCV;
    if(force_save || regNzcv->liveState == registerSlot::spilled)
        spRegs.push_back(regNzcv);

    for(std::vector<registerSlot *>::iterator itr = spRegs.begin(); itr != spRegs.end(); itr++) {
        registerSlot *cur = *itr;
        restoreSPR(gen, theRegSpace->getScratchRegister(gen), regMap[cur], 4*GPRSIZE_32);
        ret++;
    }

    return ret;
}

void EmitterAARCH64RestoreRegs::tearFrame(codeGen &gen) {
    //Restore frame pointer
    Register framePointer = convertRegID(Dyninst::aarch64::fp);
    insnCodeGen::restoreRegister(gen, framePointer, 2*GPRSIZE_64);

    //Restore link register
    Register linkRegister = convertRegID(Dyninst::aarch64::lr);
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

/***********************************************************************************************/
/***********************************************************************************************/

void cleanUpAndExit(int status);

//Not correctly implemented
void MovePCToReg(Register dest, codeGen &gen) {
    instruction insn;
    insn.clear();

    INSN_SET(insn, 28, 28, 1);
    INSN_SET(insn, 0, 4, dest);

    insnCodeGen::generate(gen, insn);
    return;
}

void emitLoadPreviousStackFrameRegister(Address register_num,
                                        Register dest,
                                        codeGen &gen,
                                        int /*size*/)
{
    gen.codeEmitter()->emitLoadOrigRegister(register_num, dest, gen);
}

// First AST node: target of the call
// Second AST node: source of the call
// This can handle indirect control transfers as well
bool AddressSpace::getDynamicCallSiteArgs(InstructionAPI::Instruction i,
					  Address addr,
					  std::vector<codeGenASTPtr> &args)
{
    namespace di = Dyninst::InstructionAPI;

    auto cft = i.getControlFlowTarget();
    auto target_reg = boost::dynamic_pointer_cast<di::RegisterAST>(cft);
    if(!target_reg) return false;
    auto branch_target = convertRegID(target_reg);

    if(branch_target == registerSpace::ignored) return false;

    //jumping to Xn (BLR Xn)
    args.push_back(operandAST::origRegister((void *)(long)branch_target));
    args.push_back(operandAST::Constant((void *) addr));

    return true;
}

Emitter *AddressSpace::getEmitter() {
    static EmitterAARCH64Stat emitter64Stat;
    static Dyninst::DyninstAPI::EmitterAarch64Dyn emitter64Dyn;

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

bool EmitterAARCH64Stat::emitPLTCommon(func_instance *, bool, codeGen &) {
    assert(0); //Not implemented
    return true;
}

bool EmitterAARCH64Stat::emitPLTCall(func_instance *callee, codeGen &gen) {
    Address dest = getInterModuleFuncAddr(callee, gen);
    long varOffset = dest - gen.currAddr();

    Register baseReg = gen.rs()->getScratchRegister(gen);
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

    Register baseReg = gen.rs()->getScratchRegister(gen);
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
