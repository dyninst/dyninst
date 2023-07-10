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
 * $Id: emitter.h,v 1.10 2008/03/25 19:24:29 bernat Exp $
 */

#ifndef _EMITTER_H
#define _EMITTER_H

#include <assert.h>
#include <vector>
#include "common/src/headers.h"
#include "dyninstAPI/src/instPoint.h"
#include "dyninstAPI/src/baseTramp.h"
#include "dyninstAPI/src/ast.h"

class codeGen;
class registerSpace;
class baseTramp;

class registerSlot;

// class for encapsulating
// platform dependent code generation functions
class Emitter {

 public:
    virtual ~Emitter() {}
    virtual codeBufIndex_t emitIf(Register expr_reg, Register target, RegControl rc, codeGen &gen) = 0;
    virtual void emitOp(unsigned opcode, Register dest, Register src1, Register src2, codeGen &gen) = 0;
    virtual void emitOpImm(unsigned opcode1, unsigned opcode2, Register dest, Register src1, RegValue src2imm,
			   codeGen &gen) = 0;
    virtual void emitRelOp(unsigned op, Register dest, Register src1, Register src2, codeGen &gen, bool s) = 0;
    virtual void emitRelOpImm(unsigned op, Register dest, Register src1, RegValue src2imm, codeGen &gen, bool s) = 0;
    virtual void emitDiv(Register dest, Register src1, Register src2, codeGen &gen, bool s) = 0;
    virtual void emitTimesImm(Register dest, Register src1, RegValue src2imm, codeGen &gen) = 0;
    virtual void emitDivImm(Register dest, Register src1, RegValue src2imm, codeGen &gen, bool s) = 0;
    virtual void emitLoad(Register dest, Address addr, int size, codeGen &gen) = 0;
    virtual void emitLoadConst(Register dest, Address imm, codeGen &gen) = 0;
    virtual void emitLoadIndir(Register dest, Register addr_reg, int size, codeGen &gen) = 0;
    virtual bool emitCallRelative(Register, Address, Register, codeGen &) = 0;
    virtual bool emitLoadRelative(Register dest, Address offset, Register base, int size, codeGen &gen) = 0;
    virtual void emitLoadShared(opCode op, Register dest, const image_variable *var, bool is_local, int size, codeGen &gen, Address offset) = 0;

    virtual void emitLoadFrameAddr(Register dest, Address offset, codeGen &gen) = 0;

    // These implicitly use the stored original/non-inst value
    virtual void emitLoadOrigFrameRelative(Register dest, Address offset, codeGen &gen) = 0;
    virtual void emitLoadOrigRegRelative(Register dest, Address offset, Register base, codeGen &gen, bool store) = 0;
    virtual void emitLoadOrigRegister(Address register_num, Register dest, codeGen &gen) = 0;
    
    virtual void emitStoreOrigRegister(Address register_num, Register dest, codeGen &gen) = 0;

    virtual void emitStore(Address addr, Register src, int size, codeGen &gen) = 0;
    virtual void emitStoreIndir(Register addr_reg, Register src, int size, codeGen &gen) = 0;
    virtual void emitStoreFrameRelative(Address offset, Register src, Register scratch, int size, codeGen &gen) = 0;
    virtual void emitStoreRelative(Register source, Address offset, Register base, int size, codeGen &gen) = 0;
    virtual void emitStoreShared(Register source, const image_variable *var, bool is_local, int size, codeGen &gen) = 0;

    virtual bool emitMoveRegToReg(Register src, Register dest, codeGen &gen) = 0;
    virtual bool emitMoveRegToReg(registerSlot *src, registerSlot *dest, codeGen &gen) = 0;

    virtual Register emitCall(opCode op, codeGen &gen, const std::vector<AstNodePtr> &operands,
			      bool noCost, func_instance *callee) = 0;

    virtual void emitGetRetVal(Register dest, bool addr_of, codeGen &gen) = 0;
    virtual void emitGetRetAddr(Register dest, codeGen &gen) = 0;
    virtual void emitGetParam(Register dest, Register param_num, instPoint::Type pt_type, opCode op, bool addr_of, codeGen &gen) = 0;
    virtual void emitASload(int ra, int rb, int sc, long imm, Register dest, int stackShift, codeGen &gen) = 0;
    virtual void emitCSload(int ra, int rb, int sc, long imm, Register dest, codeGen &gen) = 0;
    virtual void emitPushFlags(codeGen &gen) = 0;
    virtual void emitRestoreFlags(codeGen &gen, unsigned offset) = 0;
    // Built-in offset...
    virtual void emitRestoreFlagsFromStackSlot(codeGen &gen) = 0;
    virtual bool emitBTSaves(baseTramp* bt, codeGen &gen) = 0;
    virtual bool emitBTRestores(baseTramp* bt, codeGen &gen) = 0;
    virtual void emitStoreImm(Address addr, int imm, codeGen &gen, bool noCost) = 0;
    virtual void emitAddSignedImm(Address addr, int imm, codeGen &gen, bool noCost) = 0;
    virtual bool emitPush(codeGen &, Register) = 0;
    virtual bool emitPop(codeGen &, Register) = 0;
    virtual bool emitAdjustStackPointer(int index, codeGen &gen) = 0;
    
    virtual bool clobberAllFuncCall(registerSpace *rs,func_instance *callee) = 0;

    Address getInterModuleFuncAddr(func_instance *func, codeGen& gen);
    Address getInterModuleVarAddr(const image_variable *var, codeGen& gen);
    //bool emitPIC(codeGen& /*gen*/, Address, Address );

    virtual bool emitPLTCall(func_instance *, codeGen &) { assert(0); return false;}
    virtual bool emitPLTJump(func_instance *, codeGen &) { assert(0); return false;}

    virtual bool emitTOCJump(block_instance *, codeGen &) { assert(0); return false; }
    virtual bool emitTOCCall(block_instance *, codeGen &) { assert(0); return false; }
};

#endif
