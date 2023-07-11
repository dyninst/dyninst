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
#include "common/src/headers.h"
#include "common/src/arch.h"
#include "dyninstAPI/src/instPoint.h"
#include "dyninstAPI/src/baseTramp.h"

#include "dyninstAPI/src/emitter.h"
class codeGen;
class registerSpace;

class registerSlot;

// Emitter moved to emitter.h - useful on other platforms as well

class Emitterx86 : public Emitter {
    public:
        virtual ~Emitterx86() {}

        virtual bool emitLoadRelativeSegReg(Register dest, Address offset, Register base, int size, codeGen &gen) = 0;

        virtual bool emitXorRegRM(Register dest, Register base, int disp, codeGen& gen) = 0;
        virtual bool emitXorRegReg(Register dest, Register base, codeGen& gen) = 0;
        virtual bool emitXorRegImm(Register dest, int imm, codeGen& gen) = 0;
        virtual bool emitXorRegSegReg(Register dest, Register base, int disp, codeGen& gen) = 0;

        virtual void emitLEA(Register base, Register index, unsigned int scale, int disp, Register dest, codeGen& gen) = 0;

        virtual bool emitCallInstruction(codeGen &, func_instance *, Register) = 0;
};

// 32-bit class declared here since its implementation is in both inst-x86.C and emit-x86.C
class EmitterIA32 : public Emitterx86 {

public:
    virtual ~EmitterIA32() {}
    static const int mt_offset;
    codeBufIndex_t emitIf(Register expr_reg, Register target, RegControl rc, codeGen &gen);
    void emitOp(unsigned opcode, Register dest, Register src1, Register src2, codeGen &gen);
    void emitRelOp(unsigned op, Register dest, Register src1, Register src2, codeGen &gen, bool s);
    void emitDiv(Register dest, Register src1, Register src2, codeGen &gen, bool s);
    void emitOpImm(unsigned opcode1, unsigned opcode2, Register dest, Register src1, RegValue src2imm,
			   codeGen &gen);
    void emitRelOpImm(unsigned op, Register dest, Register src1, RegValue src2imm, codeGen &gen, bool s);
    void emitTimesImm(Register dest, Register src1, RegValue src1imm, codeGen &gen);
    void emitDivImm(Register dest, Register src1, RegValue src1imm, codeGen &gen, bool s);
    void emitLoad(Register dest, Address addr, int size, codeGen &gen);
    void emitLoadConst(Register dest, Address imm, codeGen &gen);
    void emitLoadIndir(Register dest, Register addr_reg, int size, codeGen &gen);
    bool emitCallRelative(Register, Address, Register, codeGen &) {assert (0); return false; }
    bool emitLoadRelative(Register dest, Address offset, Register base, int size, codeGen &gen);
    bool emitLoadRelativeSegReg(Register dest, Address offset, Register base, int size, codeGen &gen);
    void emitLoadShared(opCode op, Register dest, const image_variable *var, bool is_local,int size, codeGen &gen, Address offset);
    void emitLoadFrameAddr(Register dest, Address offset, codeGen &gen);
    void emitLoadOrigFrameRelative(Register dest, Address offset, codeGen &gen);
    void emitLoadOrigRegRelative(Register dest, Address offset, Register base, codeGen &gen, bool store);
    void emitLoadOrigRegister(Address register_num, Register dest, codeGen &gen);

    void emitStoreOrigRegister(Address register_num, Register dest, codeGen &gen);

    void emitStore(Address addr, Register src, int size, codeGen &gen);
    void emitStoreIndir(Register addr_reg, Register src, int size, codeGen &gen);
    void emitStoreFrameRelative(Address offset, Register src, Register scratch, int size, codeGen &gen);
    void emitStoreRelative(Register source, Address offset, Register base, int size, codeGen &gen);
    void emitStoreShared(Register source, const image_variable *var, bool is_local,int size, codeGen &gen);

    bool clobberAllFuncCall(registerSpace *rs,func_instance *callee);
    void setFPSaveOrNot(const int * liveFPReg,bool saveOrNot);
    // We can overload this for the stat/dyn case
    virtual Register emitCall(opCode op, codeGen &gen,
                              const std::vector<AstNodePtr> &operands,
                              bool noCost, func_instance *callee);
    //virtual bool emitPIC(codeGen& /*gen*/, Address, Address )=0;
    int emitCallParams(codeGen &gen, 
                       const std::vector<AstNodePtr> &operands,
                       func_instance *target, 
                       std::vector<Register> &extra_saves,
                       bool noCost);
    bool emitCallCleanup(codeGen &gen, func_instance *target, 
                         int frame_size, std::vector<Register> &extra_saves);
    void emitGetRetVal(Register dest, bool addr_of, codeGen &gen);
    void emitGetRetAddr(Register dest, codeGen &gen);
    void emitGetParam(Register dest, Register param_num, instPoint::Type pt_type, opCode op, bool addr_of, codeGen &gen);
    void emitASload(int ra, int rb, int sc, long imm, Register dest, int stackShift, codeGen &gen);
    void emitCSload(int ra, int rb, int sc, long imm, Register dest, codeGen &gen);
    void emitPushFlags(codeGen &gen);
    void emitRestoreFlags(codeGen &gen, unsigned offset);
    void emitRestoreFlagsFromStackSlot(codeGen &gen);
    void emitStackAlign(int offset, codeGen &gen);
    bool emitBTSaves(baseTramp* bt, codeGen &gen);
    bool emitBTRestores(baseTramp* bt, codeGen &gen);
    void emitLoadEffectiveAddress(Register base, Register index, unsigned int scale, int disp,
				  Register dest, codeGen &gen);
    void emitStoreImm(Address addr, int imm, codeGen &gen, bool noCost);
    void emitAddSignedImm(Address addr, int imm, codeGen &gen, bool noCost);
    int Register_DWARFtoMachineEnc(int n);
    bool emitPush(codeGen &gen, Register pushee);
    bool emitPop(codeGen &gen, Register popee);

    bool emitAdjustStackPointer(int index, codeGen &gen);

    bool emitMoveRegToReg(Register src, Register dest, codeGen &gen);
    bool emitMoveRegToReg(registerSlot* /*src*/, registerSlot* /*dest*/, codeGen& /*gen*/) { assert(0); return true; }
    void emitLEA(Register base, Register index, unsigned int scale, int disp, Register dest, codeGen& gen);

    bool emitXorRegRM(Register dest, Register base, int disp, codeGen& gen);
    bool emitXorRegReg(Register dest, Register base, codeGen& gen);
    bool emitXorRegImm(Register dest, int imm, codeGen& gen);
    bool emitXorRegSegReg(Register dest, Register base, int disp, codeGen& gen);


 protected:
    virtual bool emitCallInstruction(codeGen &gen, func_instance *target, Register ret) = 0;

};

class EmitterIA32Dyn : public EmitterIA32 {
 public:
    ~EmitterIA32Dyn() {}
    
 protected:
    bool emitCallInstruction(codeGen &gen, func_instance *target, Register ret);
    //virtual bool emitPIC(codeGen& /*gen*/, Address, Address );
};

class EmitterIA32Stat : public EmitterIA32 {
 public:

    ~EmitterIA32Stat() {}

    virtual bool emitPLTCall(func_instance *dest, codeGen &gen);
    virtual bool emitPLTJump(func_instance *dest, codeGen &gen);
    
 protected:
    bool emitCallInstruction(codeGen &gen, func_instance *target, Register ret);
    //virtual bool emitPIC(codeGen& /*gen*/, Address, Address );
};

extern EmitterIA32Dyn emitterIA32Dyn;
extern EmitterIA32Stat emitterIA32Stat;


// some useful 64-bit codegen functions
void emitMovRegToReg64(Register dest, Register src, bool is_64, codeGen &gen);
void emitMovPCRMToReg64(Register dest, int offset, int size, codeGen &gen);
void emitMovImmToReg64(Register dest, long imm, bool is_64, codeGen &gen);
void emitPushReg64(Register src, codeGen &gen);
void emitPopReg64(Register dest, codeGen &gen);
void emitMovImmToRM64(Register base, int disp, int imm, codeGen &gen);
void emitAddMem64(Address addr, int imm, codeGen &gen);
void emitAddRM64(Address addr, int imm, codeGen &gen);
void emitOpRegImm64(unsigned opcode, unsigned opcode_ext, Register rm_reg, int imm,
		    bool is_64, codeGen &gen);

#if defined(arch_x86_64)
class EmitterAMD64 : public Emitterx86 {

public:
    virtual ~EmitterAMD64() {}
    static const int mt_offset;
    codeBufIndex_t emitIf(Register expr_reg, Register target, RegControl rc, codeGen &gen);
    void emitOp(unsigned op, Register dest, Register src1, Register src2, codeGen &gen);
    void emitRelOp(unsigned op, Register dest, Register src1, Register src2, codeGen &gen, bool s);
    void emitDiv(Register dest, Register src1, Register src2, codeGen &gen, bool s);
    void emitOpImm(unsigned opcode1, unsigned opcode2, Register dest, Register src1, RegValue src2imm,
			   codeGen &gen);
    void emitRelOpImm(unsigned op, Register dest, Register src1, RegValue src2imm, codeGen &gen, bool s);
    void emitTimesImm(Register dest, Register src1, RegValue src1imm, codeGen &gen);
    void emitDivImm(Register dest, Register src1, RegValue src1imm, codeGen &gen, bool s);
    void emitLoad(Register dest, Address addr, int size, codeGen &gen);
    void emitLoadConst(Register dest, Address imm, codeGen &gen);
    void emitLoadIndir(Register dest, Register addr_reg, int size, codeGen &gen);
    bool emitCallRelative(Register, Address, Register, codeGen &) {assert (0); return false; }
    bool emitLoadRelative(Register dest, Address offset, Register base, int size, codeGen &gen);
    bool emitLoadRelativeSegReg(Register dest, Address offset, Register base, int size, codeGen &gen);
    void emitLoadFrameAddr(Register dest, Address offset, codeGen &gen);

    void emitLoadOrigFrameRelative(Register dest, Address offset, codeGen &gen);
    void emitLoadOrigRegRelative(Register dest, Address offset, Register base, codeGen &gen, bool store);
    void emitLoadOrigRegister(Address register_num, Register dest, codeGen &gen);
    void emitLoadShared(opCode op, Register dest, const image_variable *var, bool is_local,int size, codeGen &gen, Address offset);

    void emitStoreOrigRegister(Address register_num, Register dest, codeGen &gen);

    void emitStore(Address addr, Register src, int size, codeGen &gen);
    void emitStoreIndir(Register addr_reg, Register src, int size, codeGen &gen);
    void emitStoreFrameRelative(Address offset, Register src, Register scratch, int size, codeGen &gen);
    void emitStoreRelative(Register source, Address offset, Register base, int size, codeGen &gen);

    void emitStoreShared(Register source, const image_variable *var, bool is_local,int size, codeGen &gen);

    bool clobberAllFuncCall(registerSpace *rs, func_instance *callee);
    void setFPSaveOrNot(const int * liveFPReg,bool saveOrNot);
    // See comment on 32-bit emitCall
    virtual Register emitCall(opCode op, codeGen &gen,
                              const std::vector<AstNodePtr> &operands,
                              bool noCost, func_instance *callee);
    //virtual bool emitPIC(codeGen& /*gen*/, Address, Address )=0;
    void emitGetRetVal(Register dest, bool addr_of, codeGen &gen);
    void emitGetRetAddr(Register dest, codeGen &gen);
    void emitGetParam(Register dest, Register param_num, instPoint::Type pt_type, opCode op, bool addr_of, codeGen &gen);
    void emitASload(int ra, int rb, int sc, long imm, Register dest, int stackShift, codeGen &gen);
    void emitCSload(int ra, int rb, int sc, long imm, Register dest, codeGen &gen);
    void emitPushFlags(codeGen &gen);
    void emitRestoreFlags(codeGen &gen, unsigned offset);
    void emitRestoreFlagsFromStackSlot(codeGen &gen);
    void emitStackAlign(int offset, codeGen &gen);
    bool emitBTSaves(baseTramp* bt, codeGen &gen);
    bool emitBTRestores(baseTramp* bt, codeGen &gen);
    void emitStoreImm(Address addr, int imm, codeGen &gen, bool noCost);
    void emitAddSignedImm(Address addr, int imm, codeGen &gen, bool noCost);
    /* The DWARF register numbering does not correspond to the architecture's
       register encoding for 64-bit target binaries *only*. This method
       maps the number that DWARF reports for a register to the actual
       register number. */
    int Register_DWARFtoMachineEnc(int n);
    bool emitPush(codeGen &gen, Register pushee);
    bool emitPop(codeGen &gen, Register popee);

    bool emitAdjustStackPointer(int index, codeGen &gen);

    bool emitMoveRegToReg(Register src, Register dest, codeGen &gen);
    bool emitMoveRegToReg(registerSlot *src, registerSlot *dest, codeGen &gen);
    void emitLEA(Register base, Register index, unsigned int scale, int disp, Register dest, codeGen& gen);

    bool emitXorRegRM(Register dest, Register base, int disp, codeGen& gen);
    bool emitXorRegReg(Register dest, Register base, codeGen& gen);
    bool emitXorRegImm(Register dest, int imm, codeGen& gen);
    bool emitXorRegSegReg(Register dest, Register base, int disp, codeGen& gen);

 protected:
    virtual bool emitCallInstruction(codeGen &gen, func_instance *target, Register ret) = 0;

};

class EmitterAMD64Dyn : public EmitterAMD64 {
 public:
    ~EmitterAMD64Dyn() {}

    bool emitCallInstruction(codeGen &gen, func_instance *target, Register ret);
    //virtual bool emitPIC(codeGen& /*gen*/, Address, Address );
};

class EmitterAMD64Stat : public EmitterAMD64 {
 public:
    ~EmitterAMD64Stat() {}
    
    virtual bool emitPLTCall(func_instance *dest, codeGen &gen);
    virtual bool emitPLTJump(func_instance *dest, codeGen &gen);

    bool emitCallInstruction(codeGen &gen, func_instance *target, Register ret);
    //virtual bool emitPIC(codeGen& /*gen*/, Address, Address );
};

extern EmitterAMD64Dyn emitterAMD64Dyn;
extern EmitterAMD64Stat emitterAMD64Stat;

/* useful functions for inter-library function/variable references
 * (used in the binary rewriter) */
//Address getInterModuleFuncAddr(func_instance *func, codeGen& gen);
//Address getInterModuleVarAddr(const image_variable *var, codeGen& gen);

#endif

#endif
