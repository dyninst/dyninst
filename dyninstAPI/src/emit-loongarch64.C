#include "registers/loongarch64_regs.h"
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

#include "dyninstAPI/src/emit-loongarch64.h"
#include "dyninstAPI/src/codegen.h"
#include "dyninstAPI/src/registerSpace.h"
#include "dyninstAPI/src/inst-loongarch64.h"
#include "common/src/arch-loongarch64.h"
#include "common/h/dyn_regs.h"

using namespace Dyninst::loongarch64;



bool EmitterLOONGARCH64::emitLoadFrameAddr(codeGen &, Address, Register) {
    assert(0 && "emitLoadFrameAddr not implemented");
    return false;
}

Address EmitterLOONGARCH64::emitMovePCToReg(codeGen &, Register) {
    assert(0 && "emitMovePCToReg not implemented");
    return 0;
}

bool EmitterLOONGARCH64::emitBTSaves(baseTramp *, codeGen &, registerSpace *) {
    assert(0 && "emitBTSaves not implemented");
    return false;
}

bool EmitterLOONGARCH64::emitBTRestores(baseTramp *, codeGen &, registerSpace *) {
    assert(0 && "emitBTRestores not implemented");
    return false;
}

bool EmitterLOONGARCH64::emitBTCalls(baseTramp *, codeGen &, bool, codeBufIndex_t &) {
    assert(0 && "emitBTCalls not implemented");
    return false;
}

bool EmitterLOONGARCH64::emitBTCallCleanup(baseTramp *, codeGen &, bool) {
    assert(0 && "emitBTCallCleanup not implemented");
    return false;
}

bool EmitterLOONGARCH64::emitBTRestores(baseTramp *, codeGen &) {
    assert(0 && "emitBTRestores not implemented");
    return false;
}

bool EmitterLOONGARCH64::emitCallInstruction(codeGen &, bool, int, Address) {
    assert(0 && "emitCallInstruction not implemented");
    return false;
}

bool EmitterLOONGARCH64::emitCallCleanup(codeGen &, bool, int) {
    assert(0 && "emitCallCleanup not implemented");
    return false;
}

bool EmitterLOONGARCH64::emitpltCall(codeGen &, SymtabAPI::Function *, Address) {
    assert(0 && "emitpltCall not implemented");
    return false;
}

bool EmitterLOONGARCH64::emitASaveRegisters(baseTramp *, codeGen &, registerSpace *) {
    assert(0 && "emitASaveRegisters not implemented");
    return false;
}

bool EmitterLOONGARCH64::emitARestores(baseTramp *, codeGen &, registerSpace *) {
    assert(0 && "emitARestores not implemented");
    return false;
}

Register EmitterLOONGARCH64::emitRNodePre(codeGen &, Register, bool, Register) {
    assert(0 && "emitRNodePre not implemented");
    return Null_Register;
}

bool EmitterLOONGARCH64::emitRNodePost(codeGen &, Register, bool) {
    assert(0 && "emitRNodePost not implemented");
    return false;
}

bool EmitterLOONGARCH64::emitReturnOp(codeGen &, Register) {
    assert(0 && "emitReturnOp not implemented");
    return false;
}

bool EmitterLOONGARCH64::emitGetRetVal(codeGen &, bool, Register) {
    assert(0 && "emitGetRetVal not implemented");
    return false;
}

bool EmitterLOONGARCH64::emitGetParam(codeGen &, Register, instPoint *, bool, Register) {
    assert(0 && "emitGetParam not implemented");
    return false;
}

bool EmitterLOONGARCH64::emitFuncJump(codeGen &, Address, instPoint *, bool) {
    assert(0 && "emitFuncJump not implemented");
    return false;
}

bool EmitterLOONGARCH64::emitPush(codeGen &, Register, bool) {
    assert(0 && "emitPush not implemented");
    return false;
}

bool EmitterLOONGARCH64::emitPop(codeGen &, Register, bool) {
    assert(0 && "emitPop not implemented");
    return false;
}

bool EmitterLOONGARCH64::emitAdjustStackPointer(int, codeGen &) {
    assert(0 && "emitAdjustStackPointer not implemented");
    return false;
}

bool EmitterLOONGARCH64::emitStoreConst(codeGen &, Address, int, Register, bool, Register) {
    assert(0 && "emitStoreConst not implemented");
    return false;
}

bool EmitterLOONGARCH64::emitStore(codeGen &, Register, Register, int, bool, Register) {
    assert(0 && "emitStore not implemented");
    return false;
}

bool EmitterLOONGARCH64::emitStoreFrameRelative(codeGen &, Register, Register, int, bool, Register) {
    assert(0 && "emitStoreFrameRelative not implemented");
    return false;
}

bool EmitterLOONGARCH64::emitLoad(codeGen &, Register, Register, int, bool, Register) {
    assert(0 && "emitLoad not implemented");
    return false;
}

bool EmitterLOONGARCH64::emitLoadFrameRelative(codeGen &, Register, Address, Register) {
    assert(0 && "emitLoadFrameRelative not implemented");
    return false;
}

bool EmitterLOONGARCH64::emitLoadFrameAddr(codeGen &, Register, Address, Register) {
    assert(0 && "emitLoadFrameAddr not implemented");
    return false;
}

bool EmitterLOONGARCH64::emitLoadOrigRegRelative(codeGen &, Register, Register, int, Register, bool) {
    assert(0 && "emitLoadOrigRegRelative not implemented");
    return false;
}

bool EmitterLOONGARCH64::emitLoadOrigRegReference(codeGen &, Register, Register, int, Register) {
    assert(0 && "emitLoadOrigRegReference not implemented");
    return false;
}

bool EmitterLOONGARCH64::emitLoadOrigFrameRelative(codeGen &, Register, Address, Register) {
    assert(0 && "emitLoadOrigFrameRelative not implemented");
    return false;
}

bool EmitterLOONGARCH64::emitLoadOrigRegister(codeGen &, Register, Register) {
    assert(0 && "emitLoadOrigRegister not implemented");
    return false;
}

bool EmitterLOONGARCH64::emitReadOrigFrameRelative(codeGen &, Register, Register, Register) {
    assert(0 && "emitReadOrigFrameRelative not implemented");
    return false;
}

bool EmitterLOONGARCH64::emitBasicBlockGuard(codeGen &, Register, Register, Register, Register, Register, Register) {
    assert(0 && "emitBasicBlockGuard not implemented");
    return false;
}

bool EmitterLOONGARCH64::emitDataAccess(baseTramp *, codeGen &, Register, Register, Register, Register, Register, Register, bool, Address) {
    assert(0 && "emitDataAccess not implemented");
    return false;
}

bool EmitterLOONGARCH64::emitDataAccess(baseTramp *, codeGen &, Register, Register, Register, Register, Register, Register, bool, Register) {
    assert(0 && "emitDataAccess not implemented");
    return false;
}

bool EmitterLOONGARCH64::emitDataAccess(baseTramp *, codeGen &, Register, Register, Register, Register, Register, Register, bool, Register, Address) {
    assert(0 && "emitDataAccess not implemented");
    return false;
}

bool EmitterLOONGARCH64::emitIntrinsic(codeGen &, Register, int, Register, Register, Register, Register, bool, Register) {
    assert(0 && "emitIntrinsic not implemented");
    return false;
}

bool EmitterLOONGARCH64::emitFuncCall(codeGen &, Register, int, Register, Register, Register, Register, bool, Register) {
    assert(0 && "emitFuncCall not implemented");
    return false;
}

Register EmitterLOONGARCH64::emitFuncCall(codeGen &, func_instance *, std::vector<AstNodePtr> &, bool, Register) {
    assert(0 && "emitFuncCall not implemented");
    return Null_Register;
}

bool EmitterLOONGARCH64::emitFuncCall(func_instance *, codeGen &, std::vector<Register> &, bool, Register) {
    assert(0 && "emitFuncCall not implemented");
    return false;
}

bool EmitterLOONGARCH64::emitTOCJump(codeGen &, Address) {
    assert(0 && "emitTOCJump not implemented");
    return false;
}

bool EmitterLOONGARCH64::emitTOCCall(codeGen &, Address) {
    assert(0 && "emitTOCCall not implemented");
    return false;
}

Register EmitterLOONGARCH64::emitCallWithTOC(codeGen &, Address, Register, bool, Register) {
    assert(0 && "emitCallWithTOC not implemented");
    return Null_Register;
}

bool EmitterLOONGARCH64::emitPIC(codeGen &, Address, Register) {
    assert(0 && "emitPIC not implemented");
    return false;
}

bool EmitterLOONGARCH64::clobberAllFuncCall(codeGen &, registerSpace *, func_instance *) {
    assert(0 && "clobberAllFuncCall not implemented");
    return false;
}

// Stat variants
Register EmitterLOONGARCH64Stat::emitCallWithTOC(codeGen &, Address, Register, bool, Register) {
    assert(0 && "Stat::emitCallWithTOC not implemented");
    return Null_Register;
}

// Dyn variants
bool EmitterLOONGARCH64Dyn::emitPIC(codeGen &, Address, Register) {
    assert(0 && "Dyn::emitPIC not implemented");
    return false;
}

Register EmitterLOONGARCH64Dyn::emitCallWithTOC(codeGen &, Address, Register, bool, Register) {
    assert(0 && "Dyn::emitCallWithTOC not implemented");
    return Null_Register;
}

// =====================================================================
// Emitter base class pure virtual method overrides
// These provide the base-class-signature implementations required by
// the Emitter abstract base class.
// =====================================================================

codeBufIndex_t EmitterLOONGARCH64::emitIf(Register expr_reg,
                                          Register target,
                                          RegControl /*rc*/,
                                          codeGen &gen) {
    // if (expr_reg != 0) jump to target
    // bne expr_reg, zero, 8
    // j target
    // The target is a code buffer index, not a PC address.
    // Use the same pattern as RISC-V: conditional skip over unconditional jump
    assert(0 && "emitIf not yet implemented for loongarch64");
    return gen.getIndex();
}

void EmitterLOONGARCH64::emitOp(unsigned /*opcode*/, Register /*dest*/,
                                Register /*src1*/, Register /*src2*/,
                                codeGen &/*gen*/) {
    assert(0 && "emitOp not yet implemented for loongarch64");
}

void EmitterLOONGARCH64::emitOpImm(unsigned /*opcode1*/, unsigned /*opcode2*/,
                                   Register /*dest*/, Register /*src1*/,
                                   RegValue /*src2imm*/, codeGen &/*gen*/) {
    assert(0 && "emitOpImm not yet implemented for loongarch64");
}

void EmitterLOONGARCH64::emitRelOp(unsigned /*op*/, Register /*dest*/,
                                   Register /*src1*/, Register /*src2*/,
                                   codeGen &/*gen*/, bool /*s*/) {
    assert(0 && "emitRelOp not yet implemented for loongarch64");
}

void EmitterLOONGARCH64::emitRelOpImm(unsigned /*op*/, Register /*dest*/,
                                      Register /*src1*/, RegValue /*src2imm*/,
                                      codeGen &/*gen*/, bool /*s*/) {
    assert(0 && "emitRelOpImm not yet implemented for loongarch64");
}

void EmitterLOONGARCH64::emitDiv(Register /*dest*/, Register /*src1*/,
                                 Register /*src2*/, codeGen &/*gen*/,
                                 bool /*s*/) {
    assert(0 && "emitDiv not yet implemented for loongarch64");
}

void EmitterLOONGARCH64::emitTimesImm(Register /*dest*/, Register /*src1*/,
                                      RegValue /*src2imm*/, codeGen &/*gen*/) {
    assert(0 && "emitTimesImm not yet implemented for loongarch64");
}

void EmitterLOONGARCH64::emitDivImm(Register /*dest*/, Register /*src1*/,
                                    RegValue /*src2imm*/, codeGen &/*gen*/,
                                    bool /*s*/) {
    assert(0 && "emitDivImm not yet implemented for loongarch64");
}

void EmitterLOONGARCH64::emitLoad(Register /*dest*/, Address /*addr*/,
                                  int /*size*/, codeGen &/*gen*/) {
    assert(0 && "emitLoad(dest, addr, size, gen) not yet implemented for loongarch64");
}

void EmitterLOONGARCH64::emitLoadConst(Register /*dest*/, Address /*imm*/,
                                       codeGen &/*gen*/) {
    assert(0 && "emitLoadConst not yet implemented for loongarch64");
}

void EmitterLOONGARCH64::emitLoadShared(opCode /*op*/, Register /*dest*/,
                                        const image_variable * /*var*/,
                                        bool /*is_local*/, int /*size*/,
                                        codeGen &/*gen*/, Address /*offset*/) {
    assert(0 && "emitLoadShared not yet implemented for loongarch64");
}

void EmitterLOONGARCH64::emitLoadFrameAddr(Register /*dest*/, Address /*offset*/,
                                           codeGen &/*gen*/) {
    assert(0 && "emitLoadFrameAddr(dest, offset, gen) not yet implemented for loongarch64");
}

void EmitterLOONGARCH64::emitLoadOrigFrameRelative(Register /*dest*/,
                                                   Address /*offset*/,
                                                   codeGen &/*gen*/) {
    assert(0 && "emitLoadOrigFrameRelative(dest, offset, gen) not yet implemented for loongarch64");
}

void EmitterLOONGARCH64::emitLoadOrigRegRelative(Register /*dest*/,
                                                  Address /*offset*/,
                                                  Register /*base*/,
                                                  codeGen &/*gen*/,
                                                  bool /*store*/) {
    assert(0 && "emitLoadOrigRegRelative(dest, offset, base, gen, store) not yet implemented for loongarch64");
}

void EmitterLOONGARCH64::emitLoadOrigRegister(Address /*register_num*/,
                                               Register /*dest*/,
                                               codeGen &/*gen*/) {
    assert(0 && "emitLoadOrigRegister(register_num, dest, gen) not yet implemented for loongarch64");
}

void EmitterLOONGARCH64::emitStoreOrigRegister(Address /*register_num*/,
                                                Register /*dest*/,
                                                codeGen &/*gen*/) {
    assert(0 && "emitStoreOrigRegister not yet implemented for loongarch64");
}

void EmitterLOONGARCH64::emitStore(Address /*addr*/, Register /*src*/,
                                   int /*size*/, codeGen &/*gen*/) {
    assert(0 && "emitStore(addr, src, size, gen) not yet implemented for loongarch64");
}

void EmitterLOONGARCH64::emitLoadIndir(Register, Register, int, codeGen &) {
    assert(0 && "emitLoadIndir not implemented for loongarch64");
}

void EmitterLOONGARCH64::emitStoreIndir(Register /*addr_reg*/, Register /*src*/,
                                        int /*size*/, codeGen &/*gen*/) {
    assert(0 && "emitStoreIndir(addr_reg, src, size, gen) not yet implemented for loongarch64");
}

void EmitterLOONGARCH64::emitStoreFrameRelative(Address /*offset*/,
                                                 Register /*src*/,
                                                 Register /*scratch*/,
                                                 int /*size*/,
                                                 codeGen &/*gen*/) {
    assert(0 && "emitStoreFrameRelative(offset, src, scratch, size, gen) not yet implemented for loongarch64");
}

bool EmitterLOONGARCH64::emitCallRelative(Register, Address, Register, codeGen &) {
    assert(0 && "emitCallRelative not implemented for loongarch64");
    return false;
}

bool EmitterLOONGARCH64::emitLoadRelative(Register, Address, Register, int, codeGen &) {
    assert(0 && "emitLoadRelative not implemented for loongarch64");
    return false;
}

void EmitterLOONGARCH64::emitStoreRelative(Register /*source*/, Address /*offset*/,
                                           Register /*base*/, int /*size*/,
                                           codeGen &/*gen*/) {
    assert(0 && "emitStoreRelative(source, offset, base, size, gen) not yet implemented for loongarch64");
}

void EmitterLOONGARCH64::emitStoreShared(Register /*source*/,
                                          const image_variable * /*var*/,
                                          bool /*is_local*/, int /*size*/,
                                          codeGen &/*gen*/) {
    assert(0 && "emitStoreShared not yet implemented for loongarch64");
}

bool EmitterLOONGARCH64::emitMoveRegToReg(Register /*src*/, Register /*dest*/,
                                          codeGen &/*gen*/) {
    assert(0 && "emitMoveRegToReg(Register, Register, codeGen) not yet implemented for loongarch64");
    return false;
}

bool EmitterLOONGARCH64::emitMoveRegToReg(registerSlot * /*src*/,
                                          registerSlot * /*dest*/,
                                          codeGen &/*gen*/) {
    assert(0 && "emitMoveRegToReg(registerSlot*, registerSlot*, codeGen) not yet implemented for loongarch64");
    return false;
}

Register EmitterLOONGARCH64::emitCall(opCode /*op*/, codeGen &/*gen*/,
                                      const std::vector<AstNodePtr> & /*operands*/,
                                      bool /*noCost*/, func_instance * /*callee*/) {
    assert(0 && "emitCall(opCode, gen, operands, noCost, callee) not yet implemented for loongarch64");
    return Null_Register;
}

void EmitterLOONGARCH64::emitGetRetVal(Register /*dest*/, bool /*addr_of*/,
                                        codeGen &/*gen*/) {
    assert(0 && "emitGetRetVal(dest, addr_of, gen) not yet implemented for loongarch64");
}

void EmitterLOONGARCH64::emitGetRetAddr(Register /*dest*/, codeGen &/*gen*/) {
    assert(0 && "emitGetRetAddr not yet implemented for loongarch64");
}

void EmitterLOONGARCH64::emitGetParam(Register /*dest*/, Register /*param_num*/,
                                      instPoint::Type /*pt_type*/, opCode /*op*/,
                                      bool /*addr_of*/, codeGen &/*gen*/) {
    assert(0 && "emitGetParam(dest, param_num, pt_type, op, addr_of, gen) not yet implemented for loongarch64");
}

void EmitterLOONGARCH64::emitASload(int /*ra*/, int /*rb*/, int /*sc*/,
                                     long /*imm*/, Register /*dest*/,
                                     int /*stackShift*/, codeGen &/*gen*/) {
    assert(0 && "emitASload(ra, rb, sc, imm, dest, stackShift, gen) not yet implemented for loongarch64");
}

void EmitterLOONGARCH64::emitCSload(int /*ra*/, int /*rb*/, int /*sc*/,
                                     long /*imm*/, Register /*dest*/,
                                     codeGen &/*gen*/) {
    assert(0 && "emitCSload(ra, rb, sc, imm, dest, gen) not yet implemented for loongarch64");
}

void EmitterLOONGARCH64::emitPushFlags(codeGen &/*gen*/) {
    assert(0 && "emitPushFlags not yet implemented for loongarch64");
}

void EmitterLOONGARCH64::emitRestoreFlags(codeGen &/*gen*/, unsigned /*offset*/) {
    assert(0 && "emitRestoreFlags not yet implemented for loongarch64");
}

void EmitterLOONGARCH64::emitRestoreFlagsFromStackSlot(codeGen &/*gen*/) {
    assert(0 && "emitRestoreFlagsFromStackSlot not yet implemented for loongarch64");
}

bool EmitterLOONGARCH64::emitBTSaves(baseTramp * /*bt*/, codeGen &/*gen*/) {
    assert(0 && "emitBTSaves(bt, gen) not yet implemented for loongarch64");
    return false;
}


void EmitterLOONGARCH64::emitStoreImm(Address /*addr*/, int /*imm*/,
                                       codeGen &/*gen*/, bool /*noCost*/) {
    assert(0 && "emitStoreImm not yet implemented for loongarch64");
}

void EmitterLOONGARCH64::emitAddSignedImm(Address /*addr*/, int /*imm*/,
                                           codeGen &/*gen*/, bool /*noCost*/) {
    assert(0 && "emitAddSignedImm not yet implemented for loongarch64");
}

bool EmitterLOONGARCH64::emitPush(codeGen &/*gen*/, Register /*reg*/) {
    assert(0 && "emitPush(codeGen, Register) not yet implemented for loongarch64");
    return false;
}

bool EmitterLOONGARCH64::emitPop(codeGen &/*gen*/, Register /*reg*/) {
    assert(0 && "emitPop(codeGen, Register) not yet implemented for loongarch64");
    return false;
}


bool EmitterLOONGARCH64::clobberAllFuncCall(registerSpace * /*rs*/,
                                             func_instance * /*callee*/) {
    assert(0 && "clobberAllFuncCall(rs, callee) not yet implemented for loongarch64");
    return false;
}