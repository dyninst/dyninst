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
#include "dyninstAPI/src/inst-riscv64.h"
#include "common/src/arch-riscv64.h"
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
#include "RegisterConversion.h"
#include "parseAPI/h/CFG.h"

#include "emitter.h"
#include "emit-riscv64.h"

#include <boost/assign/list_of.hpp>
using namespace boost::assign;
#include <sstream>

#include "dyninstAPI/h/BPatch_memoryAccess_NP.h"

extern bool isPowerOf2(int value, int &result);

#define DISTANCE(x, y)   ((x<y) ? (y-x) : (x-y))

Address getMaxBranch() {
    return JAL_IMM_MAX;
}

std::unordered_map<std::string, unsigned> funcFrequencyTable;

void initDefaultPointFrequencyTable() {
    assert(0); //Not implemented
}

/************************************* Register Space **************************************/

void registerSpace::initialize32() {
    assert(!"No 32-bit implementation for the RISCV architecture!");
}

void registerSpace::initialize64() {
    static bool done = false;
    if (done)
        return;

    std::vector < registerSlot * > registers;

    // RISC-V GPRs
    for (unsigned idx = GPR_T0; idx <= GPR_T2; idx++) {
        char name[32];
        sprintf(name, "r%u", idx);
        registers.push_back(new registerSlot(idx, name, false,
                    registerSlot::deadABI, registerSlot::GPR));
    }
    for (unsigned idx = GPR_T3; idx <= GPR_T6; idx++) {
        char name[32];
        sprintf(name, "r%u", idx);
        registers.push_back(new registerSlot(idx, name, false,
                    registerSlot::deadABI, registerSlot::GPR));
    }
    for (unsigned idx = GPR_A0; idx <= GPR_A7; idx++) {
        char name[32];
        sprintf(name, "r%u", idx);
        registers.push_back(new registerSlot(idx, name, false,
                    registerSlot::liveAlways, registerSlot::GPR));
    }
    for (unsigned idx = GPR_S11; idx >= GPR_S2; idx--) {
        char name[32];
        sprintf(name, "r%u", idx);
        registers.push_back(new registerSlot(idx, name, false,
                    registerSlot::liveAlways, registerSlot::GPR));
    }
    for (unsigned idx = GPR_S1; idx >= GPR_S0; idx--) {
        char name[32];
        sprintf(name, "r%u", idx);
        registers.push_back(new registerSlot(idx, name, false,
                    registerSlot::liveAlways, registerSlot::GPR));
    }
    registers.push_back(new registerSlot(GPR_ZERO, "r0", true,
                registerSlot::liveAlways, registerSlot::GPR));
    registers.push_back(new registerSlot(GPR_RA, "r1", true,
                registerSlot::liveAlways, registerSlot::GPR));
    registers.push_back(new registerSlot(GPR_SP, "r2", true,
                registerSlot::liveAlways, registerSlot::GPR));
    registers.push_back(new registerSlot(GPR_GP, "r3", true,
                registerSlot::liveAlways, registerSlot::GPR));
    registers.push_back(new registerSlot(GPR_TP, "r4", true,
                registerSlot::liveAlways, registerSlot::GPR));

    // RISC-V FPRs
    for (unsigned idx = fpr0; idx <= fpr31; idx++) {
        char name[32];
        sprintf(name, "fpr%u", idx - fpr0);
        registers.push_back(new registerSlot(idx, name, false,
                    registerSlot::liveAlways, registerSlot::FPR));
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

void EmitterRISCV64SaveRestoreRegs::saveSPR(codeGen &/*gen*/, Register /*scratchReg*/, int /*sprnum*/, int /*stkOffset*/)
{
    // TODO RISC-V speical purpose register currently not supported
}


void EmitterRISCV64SaveRestoreRegs::saveFPRegister(codeGen &gen, Register reg, int save_off) {
    // TODO
    //     //Always performing save of the full FP register
    insnCodeGen::generateMemStoreFp(gen, REG_SP, reg, save_off, 8, gen.getUseRVC());
}

/********************************* Public methods *********************************************/

unsigned EmitterRISCV64SaveRestoreRegs::getStackHeight(codeGen &/*gen*/, registerSpace *theRegSpace) {
    unsigned height = 0;
    for (int idx = 0; idx < theRegSpace->numGPRs(); idx++) {
        registerSlot *reg = theRegSpace->GPRs()[idx];
        // Prevent storing: zero, sp, gp, and tp
        if (reg->number >= GPR_X0 && reg->number <= GPR_X4) {
            continue;
        }
        if (reg->liveState == registerSlot::spilled) {
            height++;
        }
    }
    return height;
}

int EmitterRISCV64SaveRestoreRegs::getHeightOf(codeGen &/*gen*/, registerSpace *theRegSpace, Register regNum) {
    if (theRegSpace->GPRs()[regNum]->liveState != registerSlot::spilled ||
            (regNum >= GPR_X0 && regNum <= GPR_X4)) {
        return -1;
    }
    int idx = 0;
    for (Register i = 0; i <= regNum; i++) {
        registerSlot *regSlot = theRegSpace->GPRs()[i];
        if (regSlot->liveState == registerSlot::spilled) {
            idx++;
        }
    }
    return idx;
}

unsigned EmitterRISCV64SaveRestoreRegs::saveGPRegisters(
        codeGen &gen, registerSpace *theRegSpace, baseTramp *bt, int offset)
{
    std::vector<registerSlot *> regs;
    for (int idx = 0; idx < theRegSpace->numGPRs(); idx++) {
        registerSlot *reg = theRegSpace->GPRs()[idx];
        // Prevent storing: zero, sp, gp, and tp
        if (reg->number >= GPR_X0 && reg->number <= GPR_X4) {
            continue;
        }
        if (bt->definedRegs.size() == 0 || bt->definedRegs[reg->encoding()]) {
            regs.push_back(reg);
        }
    }

    pushStack(gen, regs.size() * gen.width());

    for (unsigned idx = 0; idx < regs.size(); idx++) {
        registerSlot *reg = regs[idx];
        int offset_from_sp = offset + idx * gen.width();
        insnCodeGen::saveRegister(gen, reg->number, offset_from_sp, gen.getUseRVC());
        theRegSpace->markSavedRegister(reg->number, offset_from_sp);
    }

    return regs.size();
}

unsigned EmitterRISCV64SaveRestoreRegs::saveFPRegisters(
        codeGen &gen, registerSpace *theRegSpace, int offset)
{
    unsigned ret = 0;

    for (int idx = 0; idx < theRegSpace->numFPRs(); idx++) {
        registerSlot *reg = theRegSpace->FPRs()[idx];

        // no liveness for floating points currently, so save all
        int offset_from_sp = offset + (reg->encoding() * (idx < 32 ? FPRSIZE_32 : FPRSIZE_64));
        saveFPRegister(gen, reg->number, offset_from_sp);
        theRegSpace->markSavedRegister(reg->number, offset_from_sp);
        ret++;
    }

    return ret;
}

unsigned EmitterRISCV64SaveRestoreRegs::saveSPRegisters(
        codeGen &/*gen*/, registerSpace * /*theRegSpace*/, int /*offset*/, bool /*force_save*/)
{
    // TODO RISC-V speical purpose register currently not supported
    return 0;
}

void EmitterRISCV64SaveRestoreRegs::createFrame(codeGen &gen) {
    // Dyninst-style stack frame

    // Save link register
    Register linkRegister = gen.rs()->getRegByName("r1");
    insnCodeGen::saveRegister(gen, linkRegister, -2 * GPRSIZE_64, gen.getUseRVC());

    // Save frame pointer
    Register framePointer = gen.rs()->getRegByName("r8");
    insnCodeGen::saveRegister(gen, framePointer, -2 * GPRSIZE_64, gen.getUseRVC());

    // Move stack pointer to frame pointer
    Register stackPointer = gen.rs()->getRegByName("r2");
    insnCodeGen::generateMove(gen, framePointer, stackPointer, gen.getUseRVC());
}

/***********************************************************************************************/
/***********************************************************************************************/

/********************************* EmitterRISCV64SaveRestoreRegs ************************************/

/********************************* Public methods *********************************************/

unsigned EmitterRISCV64SaveRestoreRegs::restoreGPRegisters(
        codeGen &gen, registerSpace *theRegSpace, int offset)
{
    std::vector<registerSlot *> regs;

    for (int idx = 0; idx < theRegSpace->numGPRs(); idx++) {
        // Do not restore zero (x0), sp (x2)
        registerSlot *reg = theRegSpace->GPRs()[idx];
        if (reg->number == GPR_ZERO || reg->number == GPR_SP ||
                reg->number == GPR_GP || reg->number == GPR_TP) {
            continue;
        }
        if (reg->liveState == registerSlot::spilled) {
            regs.push_back(reg);
        }
    }
    for (int idx = regs.size() - 1; idx >= 0; idx--) {
        registerSlot *reg = regs[idx];
        int offset_from_sp = offset + idx * GPRSIZE_64;
        insnCodeGen::restoreRegister(gen, reg->number, offset_from_sp, gen.getUseRVC());
    }
    // Tear down the stack frame.
    popStack(gen, regs.size() * gen.width());

    return regs.size();
}

unsigned EmitterRISCV64SaveRestoreRegs::restoreFPRegisters(
        codeGen &gen, registerSpace *theRegSpace, int offset)
{
    unsigned ret = 0;

    for (int idx = theRegSpace->numFPRs() - 1; idx >= 0; idx--) {
        registerSlot *reg = theRegSpace->FPRs()[idx];

        // no liveness for floating points currently, so save all
        int offset_from_sp = offset + (reg->encoding() * (idx < 32 ? FPRSIZE_32 : FPRSIZE_64));
        restoreFPRegister(gen, reg->number, offset_from_sp);
        ret++;
    }

    return ret;
}

unsigned EmitterRISCV64SaveRestoreRegs::restoreSPRegisters(
        codeGen &/*gen*/, registerSpace * /*theRegSpace*/, int /*offset*/, int /*force_save*/)
{
    int ret = 0;
    // TODO RISC-V speical purpose register currently not supported
    return ret;
}

void EmitterRISCV64SaveRestoreRegs::tearFrame(codeGen &gen) {
    // Restore frame pointer
    Register framePointer = gen.rs()->getRegByName("r8");
    insnCodeGen::restoreRegister(gen, framePointer, 2 * GPRSIZE_64, gen.getUseRVC());

    // Restore link register
    Register linkRegister = gen.rs()->getRegByName("r1");
    insnCodeGen::restoreRegister(gen, linkRegister, 2 * GPRSIZE_64, gen.getUseRVC());
}


/********************************* Private methods *********************************************/

void EmitterRISCV64SaveRestoreRegs::restoreSPR(codeGen &/*gen*/, Register /*scratchReg*/, int /*sprnum*/, int /*stkOffset*/)
{
    // TODO RISC-V speical purpose register currently not supported
}

void EmitterRISCV64SaveRestoreRegs::restoreFPRegister(codeGen &gen, Register reg, int save_off) {
    insnCodeGen::generateMemLoadFp(gen, reg, REG_SP, save_off, 8, gen.getUseRVC());
}

/***********************************************************************************************/
/***********************************************************************************************/

/*
 * Emit code to push down the stack
 */
void pushStack(codeGen &gen, int size)
{
    insnCodeGen::generateAddImm(gen, REG_SP, REG_SP, -size, gen.getUseRVC());
}

void popStack(codeGen &gen, int size)
{
    insnCodeGen::generateAddImm(gen, REG_SP, REG_SP, size, gen.getUseRVC());
}

/*********************************** Base Tramp ***********************************************/
bool baseTramp::generateSaves(codeGen &gen, registerSpace *)
{
    regalloc_printf("========== baseTramp::generateSaves\n");

    EmitterRISCV64SaveRestoreRegs saveRestoreRegs;
    unsigned int width = gen.width();

    saveRestoreRegs.saveGPRegisters(gen, gen.rs(), this, 0);

    return true;
}

bool baseTramp::generateRestores(codeGen &gen, registerSpace *)
{
    EmitterRISCV64SaveRestoreRegs restoreRegs;
    unsigned int width = gen.width();

    restoreRegs.restoreGPRegisters(gen, gen.rs(), 0);

    return true;
}

/***********************************************************************************************/
/***********************************************************************************************/

void emitImm(opCode op, Register src1, RegValue src2imm, Register dest, codeGen &gen, bool /*noCost*/, registerSpace * /* rs */, bool s)
{
    switch (op) {
        case plusOp: {
            if (-src2imm >= -0x800 && -src2imm < 0x800) {
                insnCodeGen::generateAddImm(gen, dest, src1, src2imm & 0xfff, gen.getUseRVC());
            }
            else {
                Register scratch = insnCodeGen::moveValueToReg(gen, src2imm);
                insnCodeGen::generateAdd(gen, dest, src1, scratch, gen.getUseRVC());
            }
            break;
        }
        case minusOp: {
            if (-src2imm >= -0x800 && -src2imm < 0x800) {
                insnCodeGen::generateAddImm(gen, dest, src1, (-src2imm) & 0xfff, gen.getUseRVC());
            }
            else {
                Register scratch = insnCodeGen::moveValueToReg(gen, -src2imm);
                insnCodeGen::generateAnd(gen, dest, src1, scratch, gen.getUseRVC());
            }
            break;
        }
        case timesOp: {
            // No multiply immediate in RISC-V
            Register scratch = insnCodeGen::moveValueToReg(gen, src2imm);
            insnCodeGen::generateMul(gen, dest, src1, scratch, gen.getUseRVC());
            break;
        }
        case divOp: {
            // No multiply immediate in RISC-V
            Register scratch = insnCodeGen::moveValueToReg(gen, src2imm);
            insnCodeGen::generateDiv(gen, dest, src1, scratch, gen.getUseRVC());
            break;
        }
        case xorOp: {
            if (-src2imm >= -0x800 && -src2imm < 0x800) {
                insnCodeGen::generateXori(gen, dest, src1, src2imm & 0xfff, gen.getUseRVC());
            }
            else {
                Register scratch = insnCodeGen::moveValueToReg(gen, src2imm);
                insnCodeGen::generateXor(gen, dest, src1, scratch, gen.getUseRVC());
            }
            break;
        }
        case orOp: {
            if (-src2imm >= -0x800 && -src2imm < 0x800) {
                insnCodeGen::generateOri(gen, dest, src1, src2imm & 0xfff, gen.getUseRVC());
            }
            else {
                Register scratch = insnCodeGen::moveValueToReg(gen, src2imm);
                insnCodeGen::generateOr(gen, dest, src1, scratch, gen.getUseRVC());
            }
            break;
        }
        case andOp: {
            if (-src2imm >= -0x800 && -src2imm < 0x800) {
                insnCodeGen::generateAndi(gen, dest, src1, src2imm & 0xfff, gen.getUseRVC());
            }
            else {
                Register scratch = insnCodeGen::moveValueToReg(gen, src2imm);
                insnCodeGen::generateAnd(gen, dest, src1, scratch, gen.getUseRVC());
            }
            break;
        }
        case eqOp:
        case neOp:
        case lessOp:
        case leOp:
        case greaterOp:
        case geOp:
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
*/
bool EmitterRISCV64::clobberAllFuncCall(registerSpace *rs,
                                        func_instance *callee) {
    if (!callee) {
        return true;
    }

    if (callee->ifunc()->isLeafFunc()) {
        std::set<Register> *gpRegs = callee->ifunc()->usedGPRs();
        for (std::set<Register>::iterator itr = gpRegs->begin(); itr != gpRegs->end(); itr++) {
            rs->GPRs()[*itr]->beenUsed = true;
        }

        std::set<Register> *fpRegs = callee->ifunc()->usedFPRs();
        for (std::set<Register>::iterator itr = fpRegs->begin(); itr != fpRegs->end(); itr++) {
            if (*itr <= rs->FPRs().size()) {
                rs->FPRs()[*itr]->beenUsed = true;
            }
            else {
                rs->FPRs()[*itr & 0xff]->beenUsed = true;
            }
        }
    } else {
        for (int idx = 0; idx < rs->numGPRs(); idx++) {
            rs->GPRs()[idx]->beenUsed = true;
        }
        for (int idx = 0; idx < rs->numFPRs(); idx++) {
            rs->FPRs()[idx]->beenUsed = true;
        }
    }

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

Register EmitterRISCV64::emitCallReplacement(opCode,
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

Register EmitterRISCV64::emitCall(opCode op,
                                  codeGen &gen,
                                  const std::vector<AstNodePtr> &operands,
                                  bool,
                                  func_instance * callee) 
{
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

    // We first save all necessary registers
    vector<int> savedRegs;
    for (int i = 0; i < gen.rs()->numGPRs(); i++) {
        registerSlot *reg = gen.rs()->GPRs()[i];
        // Ignore zero, ra, sp, gp, tp
        if (reg->number == GPR_ZERO || reg->number == GPR_SP ||
                reg->number == GPR_GP || reg->number == GPR_TP) {
            continue;
        }
        // Ignore callee saved registers
        if ((reg->number >= GPR_S0 && reg->number <= GPR_S1) ||
                (reg->number >= GPR_S2 && reg->number <= GPR_S11)) {
            continue;
        }
        // Save caller saved registers if either
        // - refCount > 0 (and not a source register)
        // - keptValue == true (keep over the call)
        // - liveState == live (technically, only if not saved by the callee)
        if (reg->number == GPR_RA || (reg->refCount > 0) || reg->keptValue || (reg->liveState == registerSlot::live)) {
            savedRegs.push_back(reg->number);
        }
    }
    insnCodeGen::generateAddImm(gen, REG_SP, REG_SP, -savedRegs.size() * GPRSIZE_64, gen.getUseRVC());
    for (size_t id = 0; id < savedRegs.size(); id++) {
        insnCodeGen::saveRegister(gen, savedRegs[id], id * GPRSIZE_64, gen.getUseRVC());
    }

    // Generate code that handles operand registers
    for (size_t id = 0; id < operands.size(); id++) {
        Register reg = GPR_A0 + id;
        gen.markRegDefined(reg);

        Address unnecessary = ADDR_NULL;
        if (!operands[id]->generateCode_phase2(gen, false, unnecessary, reg)) {
            assert(0);
        }
        assert(reg != Null_Register);
    }

    // Move the function call address into a scratch register
    Register dest = gen.rs()->getScratchRegister(gen);
    assert(dest != Null_Register && "cannot get a dest register");
    gen.markRegDefined(dest);

    if (gen.addrSpace()->edit() != NULL && (gen.func()->obj() != callee->obj() || gen.addrSpace()->needsPIC())) {
        long disp = getInterModuleFuncAddr(callee, gen) - gen.currAddr();
        insnCodeGen::generateLoadImm(gen, dest, disp, true, true, gen.getUseRVC());
        insnCodeGen::generateMemLoad(gen, dest, dest, 0, GPRSIZE_64, true, gen.getUseRVC());
    } else {
        insnCodeGen::loadImmIntoReg(gen, dest, callee->addr(), gen.getUseRVC());
    }

    //// Generate jalr
    insnCodeGen::generateJalr(gen, GPR_RA, dest, 0, gen.getUseRVC());

    // Finally, we restore all necessary registers except for the return register
    for (size_t id = 0; id < savedRegs.size(); id++) {
        if (savedRegs[id] != GPR_A0 && savedRegs[id] != GPR_A1) {
            insnCodeGen::restoreRegister(gen, savedRegs[id], id * GPRSIZE_64, gen.getUseRVC());
        }
    }
    insnCodeGen::generateAddImm(gen, REG_SP, REG_SP, savedRegs.size() * GPRSIZE_64, gen.getUseRVC());
    return riscv64::a0;
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
                insnCodeGen::generateBranch(gen, dest, gen.getUseRVC());
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

    switch(op){
        case getRetValOp:
            regSlot = (*(gen.rs()))[registerSpace::r10];
            break;
        case getParamOp:
            if(src1 <= 7) {
                // src1 is 0..7 - it's a parameter order number, not a register
                regSlot = (*(gen.rs()))[registerSpace::r10 + src1];
                break;

            } else {
                int stkOffset = TRAMP_FRAME_SIZE_64 + (src1 - 8) * sizeof(long);
                // printf("TRAMP_FRAME_SIZE_64: %d\n", TRAMP_FRAME_SIZE_64);
                // printf("stdOffset = TRAMP_xxx_64 + (argc - 8) * 8 = { %d }\n", stkOffset);
                // TODO: PARAM_OFFSET(addrWidth) is currently not used
                // should delete that macro if it's useless

                if (src2 != Null_Register) insnCodeGen::saveRegister(gen, src2, stkOffset, gen.getUseRVC());
                insnCodeGen::restoreRegister(gen, dest, stkOffset, gen.getUseRVC());

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
                EmitterRISCV64SaveRestoreRegs saveRestoreRegs;
                int idx = saveRestoreRegs.getHeightOf(gen, gen.rs(), reg);
                insnCodeGen::restoreRegister(gen, dest, idx * gen.width(), gen.getUseRVC());
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
    if (reg == GPR_SP) {
        insnCodeGen::generateAddImm(gen, frame_size, REG_SP, dest, gen.getUseRVC());
    }
    else {
        insnCodeGen::restoreRegister(gen, dest, gpr_off + reg * gpr_size, gen.getUseRVC());
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
void MovePCToReg(Register /*dest*/, codeGen &/*gen*/) {
    // Not used currently
    assert(0);
    return;
}

// Yuhan(02/04/19): Load in destination the effective address given
// by the address descriptor. Used for memory access stuff.
void emitASload(const BPatch_addrSpec_NP *as, Register dest, int stackShift,
                codeGen &gen,
                bool) {
    assert(stackShift == 0);
    long int imm = as->getImm();
    int ra  = as->getReg(0);
    int rb  = as->getReg(1);
    int sc  = as->getScale();
    gen.markRegDefined(dest);
    if (ra > -1) {
        if (ra == 64) {
            insnCodeGen::loadImmIntoReg(gen, dest, imm, gen.getUseRVC());
            return;
        } else {
            restoreGPRtoGPR(gen, ra, dest);
        }
    } else {
        insnCodeGen::loadImmIntoReg(gen, dest, 0, gen.getUseRVC());
    }
    if(rb > -1) {
        std::vector<Register> exclude;
        exclude.push_back(dest);
        Register scratch = gen.rs()->getScratchRegister(gen, exclude);
        assert(scratch != Null_Register && "cannot get a scratch register");
        gen.markRegDefined(scratch);
        restoreGPRtoGPR(gen, rb, scratch);
        if (sc > 0) {
            insnCodeGen::generateSlli(gen, dest, scratch, sc, gen.getUseRVC());
        }
        insnCodeGen::generateAddImm(gen, dest, scratch, dest, gen.getUseRVC());
    }
    if (imm) {
        insnCodeGen::generateAddImm(gen, dest, imm, dest, gen.getUseRVC());
    }
    return;
}

void emitCSload(const BPatch_addrSpec_NP *, Register, codeGen &,
                bool) {
    // Not used currently
    assert(0);
    return;
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
    if (op == storeOp) {
        // [dest] = src1
        // dest has the address where src1 is to be stored
        // src1 is a temporary
        // src2 is a "scratch" register, we don't need it in this architecture
        gen.codeEmitter()->emitStore(dest, src1, size, gen);
    } else {
        assert(0); //Not implemented
    }
    return;
}

void emitV(opCode op, Register src1, Register src2, Register dest,
        codeGen &gen, bool /*noCost*/,
           registerSpace * /*rs*/, int size,
           const instPoint * /* location */, AddressSpace *proc, bool s) 
{
    switch (op) {
        case plusOp:
        case minusOp:
        case timesOp:
        case orOp:
        case andOp:
        case xorOp:
            gen.codeEmitter()->emitOp(op, dest, src1, src2, gen);
            break;
        case divOp:
            insnCodeGen::generateDiv(gen, src2, src1, dest, gen.getUseRVC());
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
// I don't know how to compute cycles for RISCV64 instructions due to
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
bool AddressSpace::getDynamicCallSiteArgs(InstructionAPI::Instruction /*i*/,
                      Address /*addr*/,
                      std::vector<AstNodePtr> &/*args*/)
{
    // Not used currently
    assert(0);
    return true;
}

bool writeFunctionPtr(AddressSpace *p, Address addr, func_instance *f) {
    Address val_to_write = f->addr();
    return p->writeDataSpace((void *) addr, sizeof(Address), &val_to_write);
}

Emitter *AddressSpace::getEmitter() {
    static EmitterRISCV64Stat emitter64Stat;
    static EmitterRISCV64Dyn emitter64Dyn;

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

bool EmitterRISCV64::emitCallRelative(Register, Address, Register, codeGen &) {
    // Not used currently
    return true;
}

bool EmitterRISCV64::emitLoadRelative(Register dest, Address offset, Register baseReg, int size, codeGen &gen) {
    signed long long sOffset = (signed long long) offset;
    // If the offset is small enough (-0x800 <= offset < 0x800), use lb/lw/ld,... directly
    // TODO att it back
    //if (sOffset >= -0x800 && sOffset < 0x800) {
        //insnCodeGen::generateMemLoad(gen, dest, baseReg, offset, size, true, gen.getUseRVC());
    //}
    //else {
        std::vector<Register> exclude;
        exclude.push_back(baseReg);
        auto addReg = insnCodeGen::moveValueToReg(gen, sOffset, &exclude);
        insnCodeGen::generateAdd(gen, baseReg, addReg, baseReg, gen.getUseRVC());
        insnCodeGen::generateMemLoad(gen, dest, baseReg, 0, size, true, gen.getUseRVC());
    //}
    gen.markRegDefined(dest);
    return true;
}


void EmitterRISCV64::emitStoreRelative(Register source, Address offset, Register baseReg, int size, codeGen &gen) {
    signed long long sOffset = (signed long long) offset;
    // If the offset is small enough (-0x800 <= offset < 0x800), use lb/lw/ld,... directly
    // TODO att it back
    //if (sOffset >= -0x800 && sOffset < 0x800) {
        //insnCodeGen::generateMemStore(gen, source, baseReg, offset, size, gen.getUseRVC());
    //}
    //else {
        std::vector<Register> exclude;
        exclude.push_back(baseReg);
        auto addReg = insnCodeGen::moveValueToReg(gen, sOffset, &exclude);
        insnCodeGen::generateAdd(gen, baseReg, addReg, baseReg, gen.getUseRVC());
        insnCodeGen::generateMemStore(gen, baseReg, source, 0, size, gen.getUseRVC());
    //}
}

bool EmitterRISCV64::emitMoveRegToReg(registerSlot *,
                                      registerSlot *,
                                      codeGen &) {
    assert(0); //Not implemented
    return true;
}

/*
bool EmitterRISCV6432Stat::emitPIC(codeGen& gen, Address origAddr, Address relocAddr) {

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

bool EmitterRISCV64Stat::emitPIC(codeGen& gen, Address origAddr, Address relocAddr) {
    assert(0);
    return false;
}
bool EmitterRISCV64Dyn::emitPIC(codeGen &gen, Address origAddr, Address relocAddr) {

    Address origRet = origAddr + 4;
    Register scratch = gen.rs()->getScratchRegister(gen, true);
    assert(scratch != Null_Register);
    instruction::loadImmIntoReg(gen, scratch, origRet);
    insnCodeGen::generateMoveToLR(gen, scratch);
    return true;

}
*/

bool EmitterRISCV64Stat::emitPLTCommon(func_instance *, bool, codeGen &) {
    // Not used currently
    assert(0);
    return true;
}

#if 0
bool EmitterRISCV64Stat::emitPLTCommon(func_instance *callee, bool call, codeGen &gen) {
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

bool EmitterRISCV64Dyn::emitTOCCommon(block_instance *, bool, codeGen &) {
    // Not used currently
    assert(0);
    return true;
}

bool EmitterRISCV64Stat::emitPLTCall(func_instance *callee, codeGen &gen) {
    Address disp = getInterModuleFuncAddr(callee, gen) - gen.currAddr();
    insnCodeGen::generateLoadImm(gen, GPR_RA, disp, true, true, gen.getUseRVC());
    insnCodeGen::generateMemLoad(gen, GPR_RA, GPR_RA, 0, GPRSIZE_64, true, gen.getUseRVC());
    insnCodeGen::generateJalr(gen, GPR_RA, GPR_RA, 0, gen.getUseRVC());

    return true;
}

bool EmitterRISCV64Stat::emitPLTJump(func_instance *callee, codeGen &gen) {
    // Move the function call address into a scratch register
    Address disp = getInterModuleFuncAddr(callee, gen) - gen.currAddr();
    Register dest = gen.rs()->getScratchRegister(gen);
    if (dest == Null_Register) {
        // Spill ra, generate call, restore ra, return back
        bool useRVC;
        useRVC = insnCodeGen::generateAddImm(gen, REG_SP, REG_SP, -GPRSIZE_64, gen.getUseRVC());
        disp -= useRVC ? RVC_INSN_SIZE : RV_INSN_SIZE;
        useRVC = insnCodeGen::saveRegister(gen, GPR_RA, 0, true);
        disp -= useRVC ? RVC_INSN_SIZE : RV_INSN_SIZE;
        insnCodeGen::generateLoadImm(gen, GPR_RA, disp, true, true, gen.getUseRVC());
        insnCodeGen::generateMemLoad(gen, GPR_RA, GPR_RA, 0, GPRSIZE_64, true, gen.getUseRVC());
        insnCodeGen::generateJalr(gen, GPR_RA, GPR_RA, 0, gen.getUseRVC());
        insnCodeGen::restoreRegister(gen, GPR_RA, 0, true);
        insnCodeGen::generateAddImm(gen, REG_SP, REG_SP, GPRSIZE_64, gen.getUseRVC());
        insnCodeGen::generateJr(gen, GPR_RA, 0, gen.getUseRVC());
    }
    else {
        gen.markRegDefined(dest);
        insnCodeGen::generateLoadImm(gen, dest, disp, true, true, gen.getUseRVC());
        insnCodeGen::generateMemLoad(gen, dest, dest, 0, GPRSIZE_64, true, gen.getUseRVC());
        insnCodeGen::generateJr(gen, dest, 0, gen.getUseRVC());
    }
    return true;
}

bool EmitterRISCV64Stat::emitTOCCall(block_instance * /*block*/, codeGen & /*gen*/) {
    // Not used currently
    assert(0);
}

bool EmitterRISCV64Stat::emitTOCJump(block_instance * /*block*/, codeGen & /*gen*/) {
    // Not used currently
    assert(0);
}

bool EmitterRISCV64Stat::emitTOCCommon(block_instance * /*block*/, bool, codeGen & /*gen*/) {
    // Not used currently
    assert(0);
    return false;
}

bool EmitterRISCV64Stat::emitCallInstruction(codeGen &, func_instance *, bool, Address) {
    // Not used currently
    assert(0);
    return true;
}

// Generates call instruction sequence for all RISCV64-based systems
// under dynamic instrumentation.
//
// This should be able to stomp on the link register (LR) and TOC
// register (r2), as they were saved by Emitter::emitCall() as necessary.
bool EmitterRISCV64::emitCallInstruction(codeGen &, func_instance *, bool, Address) {
    // Not used currently
    assert(0);
    return true;
}

void EmitterRISCV64::emitLoadShared(opCode op, Register dest, const image_variable *var,
        bool is_local, int size, codeGen &gen, Address offset)
{
    // create or retrieve jump slot
    Address addr;
    int stackSize = 0;

    if (var == NULL) {
        addr = offset;
    }
    else if (!is_local) {
        addr = getInterModuleVarAddr(var, gen);
    }
    else {
        addr = (Address)var->getOffset();
    }


    // Load register with address from jump slot
    //Register baseReg = gen.rs()->getScratchRegister(gen, true);
    //assert(baseReg != Null_Register && "cannot get a scratch register");
    Address varOffset = addr - gen.currAddr();

    if (op == loadOp) {
        if (!is_local && (var != NULL)){
            emitMovePCToReg(dest, gen);
            emitLoadRelative(dest, varOffset, dest, gen.width(), gen);
            // Deference the pointer to get the variable
            insnCodeGen::generateMemLoad(gen, dest, dest, 0, 8, true, gen.getUseRVC());
        }
        else {
            emitMovePCToReg(dest, gen);
            emitLoadRelative(dest, varOffset, dest, size, gen);
        }
        
    }
    else if (op == loadConstOp) {
        if (!is_local && (var != NULL)){
            emitMovePCToReg(dest, gen);
            emitLoadRelative(dest, varOffset, dest, gen.width(), gen);
        }
        else {
            // Load effective address
            insnCodeGen::generateLoadImm(gen, dest, varOffset, true, true, gen.getUseRVC());
        }
    }
    else {
        assert("Invalid op in emitLoadShared");
    }

    assert(stackSize <= 0 && "stack not empty at the end");
    return;
}

void EmitterRISCV64::emitStoreShared(Register source, const image_variable *var,
        bool is_local, int size, codeGen &gen)
{
    // Create or retrieve jump slot
    Address addr;
    int stackSize = 0;
    if (!is_local) {
        addr = getInterModuleVarAddr(var, gen);
    }
    else {
        addr = (Address)var->getOffset();
    }

    // Load register with address from jump slot
    Register baseReg = gen.rs()->getScratchRegister(gen, true);
    assert(baseReg != Null_Register && "cannot get a scratch register");

    Address varOffset = addr - gen.currAddr();

    if (!is_local) {
        std::vector<Register> exclude;
        exclude.push_back(baseReg);
        Register addrReg = gen.rs()->getScratchRegister(gen, exclude, true);
        assert(addrReg != Null_Register && "cannot get a scratch register");

        emitMovePCToReg(baseReg, gen);
        emitLoadRelative(addrReg, varOffset, baseReg, gen.width(), gen);
        emitStoreRelative(source, 0, addrReg, size, gen);
    } else {
        // mov offset to a reg
        //exclude.push_back(baseReg);
        //Register addReg = gen.rs()->getScratchRegister(gen, exclude, true);
        //assert(addReg != Null_Register && "cannot get a scratch register");
        //emitMovePCToReg(baseReg, gen);

        //exclude.push_back(addReg);
        //insnCodeGen::generateLoadImm(gen, addReg, varOffset, false, true, gen.getUseRVC());
        // add/sub offset to baseReg
        //insnCodeGen::generateAdd(gen, baseReg, addReg, baseReg, gen.getUseRVC());
        //insnCodeGen::generateMemStore(gen, baseReg, source, 0, size, gen.getUseRVC());

        //std::vector<Register> exclude;
        //exclude.push_back(baseReg);
        //Register addrReg = gen.rs()->getScratchRegister(gen, exclude, true);
        //assert(addrReg != Null_Register && "cannot get a scratch register");

        emitMovePCToReg(baseReg, gen);
        emitStoreRelative(source, varOffset, baseReg, size, gen);
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

Address EmitterRISCV64::emitMovePCToReg(Register dest, codeGen &gen) {
    // auipc rd, 0
    insnCodeGen::generateAuipc(gen, dest, 0, gen.getUseRVC());
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

regState_t::regState_t() : 
    pc_rel_offset(-1), 
    timeline(0), 
    stack_height(0) 
{
    for (unsigned i=0; i<8; i++) {
        RealRegsState r;
        r.is_allocatable = (i != GPR_ZERO && i != GPR_RA && i != GPR_SP &&
                i != GPR_GP && i != GPR_TP);
        r.been_used = false;
        r.last_used = 0;
        r.contains = NULL;
        registerStates.push_back(r);
    }
}

void registerSpace::initRealRegSpace()
{
    for (unsigned i=0; i<regStateStack.size(); i++) {
        if (regStateStack[i])
            delete regStateStack[i];
    }
    regStateStack.clear();

    regState_t *new_regState = new regState_t();
    regStateStack.push_back(new_regState);
    regs_been_spilled.clear();

    pc_rel_reg = Null_Register;
    pc_rel_use_count = 0;
}
