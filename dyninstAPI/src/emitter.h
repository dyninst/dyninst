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
 * $Id: emitter.h,v 1.5 2006/12/01 01:33:15 legendre Exp $
 */

#ifndef _EMITTER_H
#define _EMITTER_H

#include "common/h/headers.h"
#include "dyninstAPI/src/instPoint.h"
#include "dyninstAPI/src/baseTramp.h"

class codeGen;
class AstNode;
class registerSpace;
class baseTramp;

// class for encapsulating
// platform dependent code generation functions
class Emitter {

 public:
    virtual ~Emitter() {};
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
    virtual bool emitLoadRelative(Register dest, Address offset, Register base, codeGen &gen) = 0;
    virtual void emitLoadFrameAddr(Register dest, Address offset, codeGen &gen) = 0;

    // These implicitly use the stored original/non-inst value
    virtual void emitLoadOrigFrameRelative(Register dest, Address offset, codeGen &gen) = 0;
    virtual void emitLoadOrigRegRelative(Register dest, Address offset, Register base, codeGen &gen, bool store) = 0;
    virtual void emitLoadOrigRegister(Address register_num, Register dest, codeGen &gen) = 0;

    virtual void emitStore(Address addr, Register src, codeGen &gen) = 0;
    virtual void emitStoreIndir(Register addr_reg, Register src, codeGen &gen) = 0;
    virtual void emitStoreFrameRelative(Address offset, Register src, Register scratch, codeGen &gen) = 0;
    virtual bool emitMoveRegToReg(Register src, Register dest, codeGen &gen) = 0;

    virtual Register emitCall(opCode op, codeGen &gen, const pdvector<AstNode *> &operands,
			      bool noCost, int_function *callee) = 0;
    virtual void emitGetRetVal(Register dest, codeGen &gen) = 0;
    virtual void emitGetParam(Register dest, Register param_num, instPointType_t pt_type, codeGen &gen) = 0;
    virtual void emitFuncJump(Address addr, instPointType_t ptType, codeGen &gen) = 0;
    virtual void emitASload(int ra, int rb, int sc, long imm, Register dest, codeGen &gen) = 0;
    virtual void emitCSload(int ra, int rb, int sc, long imm, Register dest, codeGen &gen) = 0;
    virtual void emitPushFlags(codeGen &gen) = 0;
    virtual void emitRestoreFlags(codeGen &gen, unsigned offset) = 0;
    // Built-in offset...
    virtual void emitRestoreFlagsFromStackSlot(codeGen &gen) = 0;
    virtual bool emitBTSaves(baseTramp* bt, codeGen &gen) = 0;
    virtual bool emitBTRestores(baseTramp* bt, codeGen &gen) = 0;
    virtual bool emitBTMTCode(baseTramp* bt, codeGen &gen) = 0;
    virtual bool emitBTGuardPreCode(baseTramp* bt, codeGen &gen, codeBufIndex_t& guardJumpIndex) = 0;
    virtual bool emitBTGuardPostCode(baseTramp* bt, codeGen &gen, codeBufIndex_t& guardTargetIndex) = 0;
    virtual bool emitBTCostCode(baseTramp* bt, codeGen &gen, unsigned& costValue) = 0;
    virtual void emitStoreImm(Address addr, int imm, codeGen &gen, bool noCost) = 0;
    virtual void emitAddSignedImm(Address addr, int imm, codeGen &gen, bool noCost) = 0;
    virtual int Register_DWARFtoMachineEnc(int n) = 0;
    virtual bool emitPush(codeGen &, Register) = 0;
    virtual bool emitPop(codeGen &, Register) = 0;
    virtual bool emitAdjustStackPointer(int index, codeGen &gen) = 0;
    
};

// current set of code generation functions
extern Emitter* code_emitter;

#endif
