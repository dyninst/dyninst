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

#ifndef _EMITTER_LOONGARCH64_H
#define _EMITTER_LOONGARCH64_H

#include "common/src/headers.h"
#include "patching/instPoint.h"
#include "trampolines/baseTramp.h"
#include "ASTs/ast.h"

#include "dyninstAPI/src/emitter.h"

class codeGen;
class registerSpace;
class baseTramp;

// class for encapsulating
// platform dependent code generation functions
class EmitterLOONGARCH64 : public Emitter {

public:
    virtual ~EmitterLOONGARCH64() {}

    virtual bool emitCallRelative(Register, Address, Register, codeGen &);
    virtual bool emitLoadRelative(Register, Address, Register, int, codeGen &);
    virtual bool emitLoadFrameAddr(codeGen &, Address, Register);
    virtual Address emitMovePCToReg(codeGen &, Register);

    virtual bool emitBTSaves(baseTramp *, codeGen &, registerSpace *);
    virtual bool emitBTRestores(baseTramp *, codeGen &, registerSpace *);
    virtual bool emitBTCalls(baseTramp *, codeGen &, bool, codeBufIndex_t &);
    virtual bool emitBTCallCleanup(baseTramp *, codeGen &, bool);
    virtual bool emitBTRestores(baseTramp *, codeGen &);

    virtual bool emitCallInstruction(codeGen &, bool, int, Address);
    virtual bool emitCallCleanup(codeGen &, bool, int);
    virtual bool emitpltCall(codeGen &, SymtabAPI::Function *, Address);

    virtual bool emitASaveRegisters(baseTramp *, codeGen &, registerSpace *);
    virtual bool emitARestores(baseTramp *, codeGen &, registerSpace *);

    virtual Register emitRNodePre(codeGen &, Register, bool, Register);
    virtual bool emitRNodePost(codeGen &, Register, bool);
    virtual bool emitReturnOp(codeGen &, Register);
    virtual bool emitGetRetVal(codeGen &, bool, Register);
    virtual bool emitGetParam(codeGen &, Register, instPoint *, bool, Register);
    virtual bool emitFuncJump(codeGen &, Address, instPoint *, bool);

    virtual bool emitPush(codeGen &, Register, bool);
    virtual bool emitPop(codeGen &, Register, bool);
    virtual bool emitAdjustStackPointer(int, codeGen &);

    virtual bool emitStoreConst(codeGen &, Address, int, Register, bool, Register);
    virtual bool emitStore(codeGen &, Register, Register, int, bool, Register);
    virtual bool emitStoreFrameRelative(codeGen &, Register, Register, int, bool, Register);

    virtual bool emitLoad(codeGen &, Register, Register, int, bool, Register);
    virtual bool emitLoadFrameRelative(codeGen &, Register, Address, Register);
    virtual bool emitLoadFrameAddr(codeGen &, Register, Address, Register);
    virtual bool emitLoadOrigRegRelative(codeGen &, Register, Register, int, Register, bool);
    virtual bool emitLoadOrigRegReference(codeGen &, Register, Register, int, Register);
    virtual bool emitLoadOrigFrameRelative(codeGen &, Register, Address, Register);
    virtual bool emitLoadOrigRegister(codeGen &, Register, Register);
    virtual bool emitReadOrigFrameRelative(codeGen &, Register, Register, Register);

    virtual bool emitBasicBlockGuard(codeGen &, Register, Register, Register, Register, Register, Register);
    virtual bool emitDataAccess(baseTramp *, codeGen &, Register, Register, Register, Register, Register, Register, bool, Address);
    virtual bool emitDataAccess(baseTramp *, codeGen &, Register, Register, Register, Register, Register, Register, bool, Register);
    virtual bool emitDataAccess(baseTramp *, codeGen &, Register, Register, Register, Register, Register, Register, bool, Register, Address);

    virtual bool emitIntrinsic(codeGen &, Register, int, Register, Register, Register, Register, bool, Register);
    virtual bool emitFuncCall(codeGen &, Register, int, Register, Register, Register, Register, bool, Register);

    virtual Register emitFuncCall(codeGen &, func_instance *, std::vector<AstNodePtr> &, bool, Register);

    virtual bool emitFuncCall(func_instance *, codeGen &, std::vector<Register> &, bool, Register);

    virtual bool emitTOCJump(codeGen &, Address);
    virtual bool emitTOCCall(codeGen &, Address);

    virtual Register emitCallWithTOC(codeGen &, Address, Register, bool, Register);

    virtual bool emitPIC(codeGen &, Address, Register);

    virtual bool clobberAllFuncCall(codeGen &, registerSpace *, func_instance *);

    // Pure virtual overrides from Emitter base class
    virtual codeBufIndex_t emitIf(Register expr_reg, Register target, RegControl rc, codeGen &gen);
    virtual void emitOp(unsigned opcode, Register dest, Register src1, Register src2, codeGen &gen);
    virtual void emitOpImm(unsigned opcode1, unsigned opcode2, Register dest, Register src1, RegValue src2imm,
                           codeGen &gen);
    virtual void emitRelOp(unsigned op, Register dest, Register src1, Register src2, codeGen &gen, bool s);
    virtual void emitRelOpImm(unsigned op, Register dest, Register src1, RegValue src2imm, codeGen &gen, bool s);
    virtual void emitDiv(Register dest, Register src1, Register src2, codeGen &gen, bool s);
    virtual void emitTimesImm(Register dest, Register src1, RegValue src2imm, codeGen &gen);
    virtual void emitDivImm(Register dest, Register src1, RegValue src2imm, codeGen &gen, bool s);
    virtual void emitLoad(Register dest, Address addr, int size, codeGen &gen);
    virtual void emitLoadConst(Register dest, Address imm, codeGen &gen);
    virtual void emitLoadIndir(Register dest, Register addr_reg, int size, codeGen &gen);
    virtual void emitLoadShared(opCode op, Register dest, const image_variable *var, bool is_local, int size, codeGen &gen, Address offset);
    virtual void emitLoadFrameAddr(Register dest, Address offset, codeGen &gen);
    virtual void emitLoadOrigFrameRelative(Register dest, Address offset, codeGen &gen);
    virtual void emitLoadOrigRegRelative(Register dest, Address offset, Register base, codeGen &gen, bool store);
    virtual void emitLoadOrigRegister(Address register_num, Register dest, codeGen &gen);
    virtual void emitStoreOrigRegister(Address register_num, Register dest, codeGen &gen);
    virtual void emitStore(Address addr, Register src, int size, codeGen &gen);
    virtual void emitStoreIndir(Register addr_reg, Register src, int size, codeGen &gen);
    virtual void emitStoreFrameRelative(Address offset, Register src, Register scratch, int size, codeGen &gen);
    virtual void emitStoreRelative(Register source, Address offset, Register base, int size, codeGen &gen);
    virtual void emitStoreShared(Register source, const image_variable *var, bool is_local, int size, codeGen &gen);
    virtual bool emitMoveRegToReg(Register src, Register dest, codeGen &gen);
    virtual bool emitMoveRegToReg(registerSlot *src, registerSlot *dest, codeGen &gen);
    virtual Register emitCall(opCode op, codeGen &gen, const std::vector<AstNodePtr> &operands,
                              bool noCost, func_instance *callee);
    virtual void emitGetRetVal(Register dest, bool addr_of, codeGen &gen);
    virtual void emitGetRetAddr(Register dest, codeGen &gen);
    virtual void emitGetParam(Register dest, Register param_num, instPoint::Type pt_type, opCode op, bool addr_of, codeGen &gen);
    virtual void emitASload(int ra, int rb, int sc, long imm, Register dest, int stackShift, codeGen &gen);
    virtual void emitCSload(int ra, int rb, int sc, long imm, Register dest, codeGen &gen);
    virtual void emitPushFlags(codeGen &gen);
    virtual void emitRestoreFlags(codeGen &gen, unsigned offset);
    virtual void emitRestoreFlagsFromStackSlot(codeGen &gen);
    virtual bool emitBTSaves(baseTramp* bt, codeGen &gen);
    virtual void emitStoreImm(Address addr, int imm, codeGen &gen, bool noCost);
    virtual void emitAddSignedImm(Address addr, int imm, codeGen &gen, bool noCost);
    virtual bool emitPush(codeGen &, Register);
    virtual bool emitPop(codeGen &, Register);
    virtual bool clobberAllFuncCall(registerSpace *rs, func_instance *callee);
};

class EmitterLOONGARCH64Stat : public EmitterLOONGARCH64 {
public:
    virtual ~EmitterLOONGARCH64Stat() {}
    virtual Register emitCallWithTOC(codeGen &, Address, Register, bool, Register);
};

class EmitterLOONGARCH64Dyn : public EmitterLOONGARCH64 {
public:
    virtual ~EmitterLOONGARCH64Dyn() {}
    virtual bool emitPIC(codeGen &, Address, Register);
    virtual Register emitCallWithTOC(codeGen &, Address, Register, bool, Register);
};

#endif // _EMITTER_LOONGARCH64_H