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

/*
 * emit-x86.h - x86 & AMD64 code generators
 * $Id: emit-x86.h,v 1.32 2008/09/11 20:14:14 mlam Exp $
 */

#ifndef _EMIT_X86_H
#define _EMIT_X86_H

#include <assert.h>
#include <vector>
#include "codegen/RegControl.h"
#include "common/src/arch-x86.h"
#include "patching/instPoint.h"
#include "trampolines/baseTramp.h"
#include "function_cache.h"

#include "dyninstAPI/src/emitter.h"
#include "codegen/emitters/x86/Emitterx86.h"
#include "codegen/emitters/x86/IA32/EmitterIA32.h"
#include "codegen/emitters/x86/IA32/EmitterIA32Dyn.h"
#include "codegen/emitters/x86/IA32/EmitterIA32Stat.h"

class codeGen;
class registerSpace;

class registerSlot;

using codeGenASTPtr = Dyninst::DyninstAPI::codeGenASTPtr;

// Emitter moved to emitter.h - useful on other platforms as well

// some useful 64-bit codegen functions
void emitMovRegToReg64(Dyninst::Register dest, Dyninst::Register src, bool is_64, codeGen &gen);
void emitMovPCRMToReg64(Dyninst::Register dest, int offset, int size, codeGen &gen, bool deref_result);
void emitMovImmToReg64(Dyninst::Register dest, long imm, bool is_64, codeGen &gen);
void emitPushReg64(Dyninst::Register src, codeGen &gen);
void emitPopReg64(Dyninst::Register dest, codeGen &gen);
void emitMovImmToRM64(Dyninst::Register base, int disp, int imm, bool is_64, codeGen &gen);
void emitAddRM64(Dyninst::Register dest, int imm, bool is_64, codeGen &gen);
void emitOpRegImm64(unsigned opcode, unsigned opcode_ext, Dyninst::Register rm_reg, int imm,
		    bool is_64, codeGen &gen);

#if defined(DYNINST_CODEGEN_ARCH_X86_64)
class EmitterAMD64 : public Dyninst::DyninstAPI::Emitterx86 {

public:
    virtual ~EmitterAMD64() {}
    codegen_arch::codeBufIndex_t emitIf(Dyninst::Register expr_reg, Dyninst::Register target, Dyninst::DyninstAPI::RegControl rc, codeGen &gen);
    void emitOp(unsigned op, Dyninst::Register dest, Dyninst::Register src1, Dyninst::Register src2, codeGen &gen);
    void emitRelOp(unsigned op, Dyninst::Register dest, Dyninst::Register src1, Dyninst::Register src2, codeGen &gen, bool s);
    void emitDiv(Dyninst::Register dest, Dyninst::Register src1, Dyninst::Register src2, codeGen &gen, bool s);
    void emitOpImm(unsigned opcode1, unsigned opcode2, Dyninst::Register dest, Dyninst::Register src1, Dyninst::RegValue src2imm,
			   codeGen &gen);
    void emitRelOpImm(unsigned op, Dyninst::Register dest, Dyninst::Register src1, Dyninst::RegValue src2imm, codeGen &gen, bool s);
    void emitTimesImm(Dyninst::Register dest, Dyninst::Register src1, Dyninst::RegValue src1imm, codeGen &gen);
    void emitDivImm(Dyninst::Register dest, Dyninst::Register src1, Dyninst::RegValue src1imm, codeGen &gen, bool s);
    void emitLoad(Dyninst::Register dest, Dyninst::Address addr, int size, codeGen &gen);
    void emitLoadConst(Dyninst::Register dest, Dyninst::Address imm, codeGen &gen);
    void emitLoadIndir(Dyninst::Register dest, Dyninst::Register addr_reg, int size, codeGen &gen);
    bool emitCallRelative(Dyninst::Register, Dyninst::Address, Dyninst::Register, codeGen &) {assert (0); return false; }
    bool emitLoadRelative(Dyninst::Register dest, Dyninst::Address offset, Dyninst::Register base, int size, codeGen &gen);
    bool emitLoadRelativeSegReg(Dyninst::Register dest, Dyninst::Address offset, Dyninst::Register base, int size, codeGen &gen);
    void emitLoadFrameAddr(Dyninst::Register dest, Dyninst::Address offset, codeGen &gen);

    void emitLoadOrigFrameRelative(Dyninst::Register dest, Dyninst::Address offset, codeGen &gen);
    void emitLoadOrigRegRelative(Dyninst::Register dest, Dyninst::Address offset, Dyninst::Register base, codeGen &gen, bool store);
    void emitLoadOrigRegister(Dyninst::Address register_num, Dyninst::Register dest, codeGen &gen);
    void emitLoadShared(opCode op, Dyninst::Register dest, const image_variable *var, bool is_local,int size, codeGen &gen, Dyninst::Address offset);

    void emitStoreOrigRegister(Dyninst::Address register_num, Dyninst::Register dest, codeGen &gen);

    void emitStore(Dyninst::Address addr, Dyninst::Register src, int size, codeGen &gen);
    void emitStoreIndir(Dyninst::Register addr_reg, Dyninst::Register src, int size, codeGen &gen);
    void emitStoreFrameRelative(Dyninst::Address offset, Dyninst::Register src, Dyninst::Register scratch, int size, codeGen &gen);
    void emitStoreRelative(Dyninst::Register source, Dyninst::Address offset, Dyninst::Register base, int size, codeGen &gen);

    void emitStoreShared(Dyninst::Register source, const image_variable *var, bool is_local,int size, codeGen &gen);

    bool clobberAllFuncCall(registerSpace *rs, func_instance *callee);
    // See comment on 32-bit emitCall
    virtual Dyninst::Register emitCall(opCode op, codeGen &gen,
                              const std::vector<codeGenASTPtr> &operands,
                              func_instance *callee);
    void emitGetRetVal(Dyninst::Register dest, bool addr_of, codeGen &gen);
    void emitGetRetAddr(Dyninst::Register dest, codeGen &gen);
    void emitGetParam(Dyninst::Register dest, Dyninst::Register param_num, instPoint::Type pt_type, opCode op, bool addr_of, codeGen &gen);
    void emitASload(int ra, int rb, int sc, long imm, Dyninst::Register dest, int stackShift, codeGen &gen);
    void emitCSload(int ra, int rb, int sc, long imm, Dyninst::Register dest, codeGen &gen);
    void emitPushFlags(codeGen &gen);
    void emitRestoreFlags(codeGen &gen, unsigned offset);
    void emitRestoreFlagsFromStackSlot(codeGen &gen);
    void emitStackAlign(int offset, codeGen &gen);
    bool emitBTSaves(baseTramp* bt, codeGen &gen);
    bool emitBTRestores(baseTramp* bt, codeGen &gen);
    void emitStoreImm(Dyninst::Address addr, int imm, codeGen &gen);
    void emitAddSignedImm(Dyninst::Address addr, int imm, codeGen &gen);
    bool emitPush(codeGen &gen, Dyninst::Register pushee);
    bool emitPop(codeGen &gen, Dyninst::Register popee);

    bool emitAdjustStackPointer(int index, codeGen &gen);

    bool emitMoveRegToReg(Dyninst::Register src, Dyninst::Register dest, codeGen &gen);
    bool emitMoveRegToReg(registerSlot *src, registerSlot *dest, codeGen &gen);
    void emitLEA(Dyninst::Register base, Dyninst::Register index, unsigned int scale, int disp, Dyninst::Register dest, codeGen& gen);

    bool emitXorRegRM(Dyninst::Register dest, Dyninst::Register base, int disp, codeGen& gen);
    bool emitXorRegReg(Dyninst::Register dest, Dyninst::Register base, codeGen& gen);
    bool emitXorRegImm(Dyninst::Register dest, int imm, codeGen& gen);
    bool emitXorRegSegReg(Dyninst::Register dest, Dyninst::Register base, int disp, codeGen& gen);

 protected:
    virtual bool emitCallInstruction(codeGen &gen, func_instance *target, Dyninst::Register ret) = 0;

 private:
    // clobberAllFuncCall can be expensive, so don't re-analyze functions
    Dyninst::DyninstAPI::function_cache clobbered_functions;
};

class EmitterAMD64Dyn : public EmitterAMD64 {
 public:
    ~EmitterAMD64Dyn() {}

    bool emitCallInstruction(codeGen &gen, func_instance *target, Dyninst::Register ret);
};

class EmitterAMD64Stat : public EmitterAMD64 {
 public:
    ~EmitterAMD64Stat() {}
    
    virtual bool emitPLTCall(func_instance *dest, codeGen &gen);
    virtual bool emitPLTJump(func_instance *dest, codeGen &gen);

    bool emitCallInstruction(codeGen &gen, func_instance *target, Dyninst::Register ret);
};

#endif

#endif
