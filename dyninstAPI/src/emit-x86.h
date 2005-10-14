/*
 * Copyright (c) 1996-2004 Barton P. Miller
 * 
 * We provide the Paradyn Parallel Performance Tools (below
 * described as "Paradyn") on an AS IS basis, and do not warrant its
 * validity or performance.  We reserve the right to update, modify,
 * or discontinue this software at any time.  We shall have no
 * obligation to supply such updates or modifications or any other
 * form of support to you.
 * 
 * This license is for research uses.  For such uses, there is no
 * charge. We define "research use" to mean you may freely use it
 * inside your organization for whatever purposes you see fit. But you
 * may not re-distribute Paradyn or parts of Paradyn, in any form
 * source or binary (including derivatives), electronic or otherwise,
 * to any other organization or entity without our permission.
 * 
 * (for other uses, please contact us at paradyn@cs.wisc.edu)
 * 
 * All warranties, including without limitation, any warranty of
 * merchantability or fitness for a particular purpose, are hereby
 * excluded.
 * 
 * By your use of Paradyn, you understand and agree that we (or any
 * other person or entity with proprietary rights in Paradyn) are
 * under no obligation to provide either maintenance services,
 * update services, notices of latent defects, or correction of
 * defects for Paradyn.
 * 
 * Even if advised of the possibility of such damages, under no
 * circumstances shall we (or any other person or entity with
 * proprietary rights in the software licensed hereunder) be liable
 * to you or any third party for direct, indirect, or consequential
 * damages of any character regardless of type of action, including,
 * without limitation, loss of profits, loss of use, loss of good
 * will, or computer failure or malfunction.  You agree to indemnify
 * us (and any other person or entity with proprietary rights in the
 * software licensed hereunder) for any and all liability it may
 * incur to third parties resulting from your use of Paradyn.
 */

/*
 * emit-x86.h - x86 & AMD64 code generators
 * $Id: emit-x86.h,v 1.4 2005/10/14 16:37:44 legendre Exp $
 */

#ifndef _EMIT_X86_H
#define _EMIT_X86_H

#include "common/h/headers.h"
#include "dyninstAPI/src/ast.h"
#include "dyninstAPI/src/instPoint.h"
#include "dyninstAPI/src/arch-x86.h"
#include "dyninstAPI/src/baseTramp.h"

class codeGen;

// class for encapsulating
// platform dependent code generation functions
class Emitter {

 public:
    virtual codeBufIndex_t emitIf(Register expr_reg, Register target, codeGen &gen) = 0;
    virtual void emitOp(unsigned opcode, Register dest, Register src1, Register src2, codeGen &gen) = 0;
    virtual void emitOpImm(unsigned opcode1, unsigned opcode2, Register dest, Register src1, RegValue src2imm,
			   codeGen &gen) = 0;
    virtual void emitRelOp(unsigned op, Register dest, Register src1, Register src2, codeGen &gen) = 0;
    virtual void emitRelOpImm(unsigned op, Register dest, Register src1, RegValue src2imm, codeGen &gen) = 0;
    virtual void emitDiv(Register dest, Register src1, Register src2, codeGen &gen) = 0;
    virtual void emitTimesImm(Register dest, Register src1, RegValue src2imm, codeGen &gen) = 0;
    virtual void emitDivImm(Register dest, Register src1, RegValue src2imm, codeGen &gen) = 0;
    virtual void emitLoad(Register dest, Address addr, int size, codeGen &gen) = 0;
    virtual void emitLoadConst(Register dest, Address imm, codeGen &gen) = 0;
    virtual void emitLoadIndir(Register dest, Register addr_reg, codeGen &gen) = 0;
    virtual void emitLoadFrameRelative(Register dest, Address offset, codeGen &gen) = 0;
    virtual void emitLoadFrameAddr(Register dest, Address offset, codeGen &gen) = 0;
    virtual void emitLoadPreviousStackFrameRegister(Address register_num, Register dest, codeGen &gen) = 0;
    virtual void emitStore(Address addr, Register src, codeGen &gen) = 0;
    virtual void emitStoreIndir(Register addr_reg, Register src, codeGen &gen) = 0;
    virtual void emitStoreFrameRelative(Address offset, Register src, Register scratch, codeGen &gen) = 0;
    virtual Register emitCall(opCode op, registerSpace *rs, codeGen &gen, const pdvector<AstNode *> &operands,
			      process *proc, bool noCost, Address callee_addr, const pdvector<AstNode *> &ifForks,
			      const instPoint *location) = 0;
    virtual void emitGetRetVal(Register dest, codeGen &gen) = 0;
    virtual void emitGetParam(Register dest, Register param_num, instPointType_t pt_type, codeGen &gen) = 0;
    virtual void emitFuncJump(Address addr, instPointType_t ptType, codeGen &gen) = 0;
    virtual bool emitBTSaves(baseTramp* bt, codeGen &gen) = 0;
    virtual bool emitBTRestores(baseTramp* bt, codeGen &gen) = 0;
    virtual bool emitBTMTCode(baseTramp* bt, codeGen &gen) = 0;
    virtual bool emitBTGuardPreCode(baseTramp* bt, codeGen &gen, codeBufIndex_t& guardJumpIndex) = 0;
    virtual bool emitBTGuardPostCode(baseTramp* bt, codeGen &gen, codeBufIndex_t& guardTargetIndex) = 0;
    virtual bool emitBTCostCode(baseTramp* bt, codeGen &gen, unsigned& costValue) = 0;
};

// current set of code generation functions
extern Emitter* x86_emitter;

// switches code generator to 32-bit mode
void emit32();

// switches code generator to 64-bit mode
void emit64();

// 32-bit class declared here since its implementation is in both inst-x86.C and emit-x86.C
class Emitter32 : public Emitter {

public:
    static const int mt_offset = -4;
    codeBufIndex_t emitIf(Register expr_reg, Register target, codeGen &gen);
    void emitOp(unsigned opcode, Register dest, Register src1, Register src2, codeGen &gen);
    void emitRelOp(unsigned op, Register dest, Register src1, Register src2, codeGen &gen);
    void emitDiv(Register dest, Register src1, Register src2, codeGen &gen);
    void emitOpImm(unsigned opcode1, unsigned opcode2, Register dest, Register src1, RegValue src2imm,
			   codeGen &gen);
    void emitRelOpImm(unsigned op, Register dest, Register src1, RegValue src2imm, codeGen &gen);
    void emitTimesImm(Register dest, Register src1, RegValue src1imm, codeGen &gen);
    void emitDivImm(Register dest, Register src1, RegValue src1imm, codeGen &gen);
    void emitLoad(Register dest, Address addr, int size, codeGen &gen);
    void emitLoadConst(Register dest, Address imm, codeGen &gen);
    void emitLoadIndir(Register dest, Register addr_reg, codeGen &gen);
    void emitLoadFrameRelative(Register dest, Address offset, codeGen &gen);
    void emitLoadFrameAddr(Register dest, Address offset, codeGen &gen);
    void emitLoadPreviousStackFrameRegister(Address register_num, Register dest, codeGen &gen);
    void emitStore(Address addr, Register src, codeGen &gen);
    void emitStoreIndir(Register addr_reg, Register src, codeGen &gen);
    void emitStoreFrameRelative(Address offset, Register src, Register scratch, codeGen &gen);
    Register emitCall(opCode op, registerSpace *rs, codeGen &gen,
			      const pdvector<AstNode *> &operands,
			      process *proc, bool noCost, Address callee_addr, const pdvector<AstNode *> &ifForks,
			      const instPoint *location);
    void emitGetRetVal(Register dest, codeGen &gen);
    void emitGetParam(Register dest, Register param_num, instPointType_t pt_type, codeGen &gen);
    void emitFuncJump(Address addr, instPointType_t ptType, codeGen &gen);
    bool emitBTSaves(baseTramp* bt, codeGen &gen);
    bool emitBTRestores(baseTramp* bt, codeGen &gen);
    bool emitBTMTCode(baseTramp* bt, codeGen &gen);
    bool emitBTGuardPreCode(baseTramp* bt, codeGen &gen, codeBufIndex_t& guardJumpIndex);
    bool emitBTGuardPostCode(baseTramp* bt, codeGen &gen, codeBufIndex_t& guardTargetIndex);
    bool emitBTCostCode(baseTramp* bt, codeGen& gen, unsigned& costValue);
};

// some useful 64-bit codegen functions
void emitMovRegToReg64(Register dest, Register src, bool is_64, codeGen &gen);
void emitMovImmToReg64(Register dest, long imm, bool is_64, codeGen &gen);

class Emitter64 : public Emitter {

public:
    static const int mt_offset = -8;
    codeBufIndex_t emitIf(Register expr_reg, Register target, codeGen &gen);
    void emitOp(unsigned op, Register dest, Register src1, Register src2, codeGen &gen);
    void emitRelOp(unsigned op, Register dest, Register src1, Register src2, codeGen &gen);
    void emitDiv(Register dest, Register src1, Register src2, codeGen &gen);
    void emitOpImm(unsigned opcode1, unsigned opcode2, Register dest, Register src1, RegValue src2imm,
			   codeGen &gen);
    void emitRelOpImm(unsigned op, Register dest, Register src1, RegValue src2imm, codeGen &gen);
    void emitTimesImm(Register dest, Register src1, RegValue src1imm, codeGen &gen);
    void emitDivImm(Register dest, Register src1, RegValue src1imm, codeGen &gen);
    void emitLoad(Register dest, Address addr, int size, codeGen &gen);
    void emitLoadConst(Register dest, Address imm, codeGen &gen);
    void emitLoadIndir(Register dest, Register addr_reg, codeGen &gen);
    void emitLoadFrameRelative(Register dest, Address offset, codeGen &gen);
    void emitLoadFrameAddr(Register dest, Address offset, codeGen &gen);
    void emitLoadPreviousStackFrameRegister(Address register_num, Register dest, codeGen &gen);
    void emitStore(Address addr, Register src, codeGen &gen);
    void emitStoreIndir(Register addr_reg, Register src, codeGen &gen);
    void emitStoreFrameRelative(Address offset, Register src, Register scratch, codeGen &gen);
    Register emitCall(opCode op, registerSpace *rs, codeGen &gen,
			      const pdvector<AstNode *> &operands,
			      process *proc, bool noCost, Address callee_addr, const pdvector<AstNode *> &ifForks,
			      const instPoint *location);
    void emitGetRetVal(Register dest, codeGen &gen);
    void emitGetParam(Register dest, Register param_num, instPointType_t pt_type, codeGen &gen);
    void emitFuncJump(Address addr, instPointType_t ptType, codeGen &gen);
    bool emitBTSaves(baseTramp* bt, codeGen &gen);
    bool emitBTRestores(baseTramp* bt, codeGen &gen);
    bool emitBTMTCode(baseTramp* bt, codeGen &gen);
    bool emitBTGuardPreCode(baseTramp* bt, codeGen &gen, codeBufIndex_t& guardJumpOffset);
    bool emitBTGuardPostCode(baseTramp* bt, codeGen &gen, codeBufIndex_t& guardTargetIndex);
    bool emitBTCostCode(baseTramp* bt, codeGen &gen, unsigned& costValue);
};

#endif
