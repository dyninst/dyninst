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
 * emit-x86.C - x86 & AMD64 code generators
 * $Id: emit-x86.C,v 1.38 2006/12/04 23:39:06 legendre Exp $
 */

#include <assert.h>
#include <stdio.h>
#include "common/h/Types.h"
#include "dyninstAPI/src/arch.h"
#include "dyninstAPI/src/emit-x86.h"
#include "dyninstAPI/src/inst-x86.h"
#include "dyninstAPI/src/showerror.h"
#include "dyninstAPI/src/ast.h"
#include "dyninstAPI/src/process.h"

// get_index...
#include "dyninstAPI/src/dyn_thread.h"

#include "InstrucIter.h"

const int Emitter32::mt_offset = -4;
#if defined(arch_x86_64)
const int Emitter64::mt_offset = -8;
#endif

bool Emitter32::emitMoveRegToReg(Register src, Register dest, codeGen &gen) {
    emitMovRegToReg(dest, src, gen);
    return true;
}

codeBufIndex_t Emitter32::emitIf(Register expr_reg, Register target, codeGen &gen)
{
    // sub REGNUM_EAX, REGNUM_EAX
    emitOpRegReg(0x29, REGNUM_EAX, REGNUM_EAX, gen);

    // cmp -(expr*4)[REGNUM_EBP], REGNUM_EAX
    emitOpRegRM(0x3B, REGNUM_EAX, REGNUM_EBP, -1*(expr_reg*4), gen);

    // Retval: where the jump is in this sequence
    codeBufIndex_t retval = gen.getIndex();

    // Jump displacements are from the end of the insn, not start. The
    // one we're emitting has a size of 6.
    int disp = target - 6;

    GET_PTR(insn, gen);
    // je dest
    *insn++ = 0x0F;
    *insn++ = 0x84;
    *((int *)insn) = disp;
    insn += sizeof(int);
    SET_PTR(insn, gen);

    return retval;
}

void Emitter32::emitOp(unsigned opcode, Register dest, Register src1, Register src2, codeGen &gen)
{
      emitMovRMToReg(REGNUM_EAX, REGNUM_EBP, -1*(src1*4), gen);
      emitOpRegRM(opcode, REGNUM_EAX, REGNUM_EBP, -1*(src2*4), gen);
      emitMovRegToRM(REGNUM_EBP, -1*(dest*4), REGNUM_EAX, gen);
}

void Emitter32::emitRelOp(unsigned op, Register dest, Register src1, Register src2, codeGen &gen)
{
    emitOpRegReg(0x29, REGNUM_ECX, REGNUM_ECX, gen);           // clear REGNUM_ECX
    emitMovRMToReg(REGNUM_EAX, REGNUM_EBP, -1*(src1*4), gen);    // mov eax, -(src1*4)[ebp]
    emitOpRegRM(0x3B, REGNUM_EAX, REGNUM_EBP, -1*(src2*4), gen); // cmp eax, -(src2*4)[ebp]
    unsigned char opcode = jccOpcodeFromRelOp(op);
    GET_PTR(insn, gen);
    *insn++ = opcode; *insn++ = 1;                // jcc 1
    SET_PTR(insn, gen);
    emitSimpleInsn(0x40+REGNUM_ECX, gen);               // inc REGNUM_ECX
    emitMovRegToRM(REGNUM_EBP, -1*(dest*4), REGNUM_ECX, gen);    // mov -(dest*4)[ebp], ecx
}

void Emitter32::emitDiv(Register dest, Register src1, Register src2, codeGen &gen)
{
    // mov eax, src1
    // cdq   ; edx = sign extend of eax
    // idiv eax, src2 ; eax = edx:eax div src2, edx = edx:eax mod src2
    // mov dest, eax
    emitMovRMToReg(REGNUM_EAX, REGNUM_EBP, -1*(src1*4), gen);
    emitSimpleInsn(0x99, gen);
    emitOpRegRM(0xF7, 0x7 /*opcode extension*/, REGNUM_EBP, -1*(src2*4), gen);
    emitMovRegToRM(REGNUM_EBP, -1*(dest*4), REGNUM_EAX, gen);
}

void Emitter32::emitOpImm(unsigned opcode1, unsigned opcode2, Register dest, Register src1, RegValue src2imm,
			  codeGen &gen)
{
    if (src1 != dest) {
	emitMovRMToReg(REGNUM_EAX, REGNUM_EBP, -1*(src1*4), gen);
	emitMovRegToRM(REGNUM_EBP, -1*(dest*4), REGNUM_EAX, gen);
    }
    emitOpRMImm(opcode1, opcode2, REGNUM_EBP, -1*(dest*4), src2imm, gen);
}

void Emitter32::emitRelOpImm(unsigned op, Register dest, Register src1, RegValue src2imm, codeGen &gen)
{
   emitOpRegReg(0x29, REGNUM_ECX, REGNUM_ECX, gen);           // clear REGNUM_ECX
   emitMovRMToReg(REGNUM_EAX, REGNUM_EBP, -1*(src1*4), gen);    // mov eax, -(src1*4)[ebp]
   emitOpRegImm(0x3D, REGNUM_EAX, src2imm, gen);       // cmp eax, src2
   unsigned char opcode = jccOpcodeFromRelOp(op);
   GET_PTR(insn, gen);
   *insn++ = opcode; *insn++ = 1;                // jcc 1
   SET_PTR(insn, gen);
   emitSimpleInsn(0x40+REGNUM_ECX, gen);               // inc REGNUM_ECX
   emitMovRegToRM(REGNUM_EBP, -1*(dest*4), REGNUM_ECX, gen);    // mov -(dest*4)[ebp], ecx   
}

// where is this defined?
extern bool isPowerOf2(int value, int &result);

void Emitter32::emitTimesImm(Register dest, Register src1, RegValue src2imm, codeGen &gen)
{
    int result = -1;

    if (src2imm == 1)
	return;

    if (isPowerOf2(src2imm, result) && result <= MAX_IMM8) {
	if (src1 != dest) {
	    emitMovRMToReg(REGNUM_EAX, REGNUM_EBP, -1*(src1*4), gen);
	    emitMovRegToRM(REGNUM_EBP, -1*(dest*4), REGNUM_EAX, gen);
	}
	// sal dest, result
	emitOpRMImm8(0xC1, 4, REGNUM_EBP, -1*(dest*4), result, gen);
    }
    else {
	// imul REGNUM_EAX, -(src1*4)[ebp], src2imm
	emitOpRegRMImm(0x69, REGNUM_EAX, REGNUM_EBP, -1*(src1*4), src2imm, gen);
	emitMovRegToRM(REGNUM_EBP, -1*(dest*4), REGNUM_EAX, gen);
    } 
}

void Emitter32::emitDivImm(Register dest, Register src1, RegValue src2imm, codeGen &gen)
{
    int result = -1;
    if (isPowerOf2(src2imm, result) && result <= MAX_IMM8) {
	if (src1 != dest) {
	    emitMovRMToReg(REGNUM_EAX, REGNUM_EBP, -1*(src1*4), gen);
	    emitMovRegToRM(REGNUM_EBP, -1*(dest*4), REGNUM_EAX, gen);
	}
	// sar dest, result
	emitOpRMImm8(0xC1, 7, REGNUM_EBP, -1*(dest*4), result, gen);
    }
    else {
	// dest = src1 div src2imm
	// mov eax, src1
	// cdq   ; edx = sign extend of eax
	// mov ebx, src2imm
	// idiv eax, ebx ; eax = edx:eax div src2, edx = edx:eax mod src2
	// mov dest, eax
	emitMovRMToReg(REGNUM_EAX, REGNUM_EBP, -1*(src1*4), gen);
	emitSimpleInsn(0x99, gen);
	emitMovImmToReg(REGNUM_EBX, src2imm, gen);
	// idiv eax, ebx
	emitOpRegReg(0xF7, 0x7 /*opcode extension*/, REGNUM_EBX, gen); 
	emitMovRegToRM(REGNUM_EBP, -1*(dest*4), REGNUM_EAX, gen);
    }
}

void Emitter32::emitLoad(Register dest, Address addr, int size, codeGen &gen)
{
   if (size == 1) {
      emitMovMBToReg(REGNUM_EAX, addr, gen);               // movsbl eax, addr
   } else if (size == 2) {
      emitMovMWToReg(REGNUM_EAX, addr, gen);               // movswl eax, addr
   } else {
      emitMovMToReg(REGNUM_EAX, addr, gen);               // mov eax, addr
   }
   emitMovRegToRM(REGNUM_EBP, -1*(dest*4), REGNUM_EAX, gen);    // mov -(dest*4)[ebp], eax
}

void Emitter32::emitLoadConst(Register dest, Address imm, codeGen &gen)
{
    emitMovImmToRM(REGNUM_EBP, -1*(dest*4), imm, gen);
}

void Emitter32::emitLoadIndir(Register dest, Register addr_reg, codeGen &gen)
{
    emitMovRMToReg(REGNUM_EAX, REGNUM_EBP, -1*(addr_reg*4), gen); // mov eax, -(addr_reg*4)[ebp]
    emitMovRMToReg(REGNUM_EAX, REGNUM_EAX, 0, gen);         // mov eax, [eax]
    emitMovRegToRM(REGNUM_EBP, -1*(dest*4), REGNUM_EAX, gen); // mov -(dest*4)[ebp], eax
}

void Emitter32::emitLoadOrigFrameRelative(Register dest, Address offset, codeGen &gen)
{
    // eax = [ebp]	- saved bp
    // dest = [eax](offset)
    emitMovRMToReg(REGNUM_EAX, REGNUM_EBP, 0, gen);       // mov (%ebp), %eax 
    emitMovRMToReg(REGNUM_EAX, REGNUM_EAX, offset, gen);    // mov <offset>(%eax), %eax 
    emitMovRegToRM(REGNUM_EBP, -1*(dest*4), REGNUM_EAX, gen);    // mov -(dest*4)[ebp], eax
}

bool Emitter32::emitLoadRelative(Register dest, Address offset, Register base, codeGen &gen)
{
    assert(0);
    return false;
    // Unimplemented
    // eax = [ebp]	- saved bp
    // dest = [eax](offset)
    emitMovRMToReg(REGNUM_EAX, REGNUM_EBP, 0, gen);       // mov (%ebp), %eax 
    emitMovRMToReg(REGNUM_EAX, REGNUM_EAX, offset, gen);    // mov <offset>(%eax), %eax 
    emitMovRegToRM(REGNUM_EBP, -1*(dest*4), REGNUM_EAX, gen);    // mov -(dest*4)[ebp], eax
}

void Emitter32::emitLoadOrigRegRelative(Register dest, Address offset,
                                        Register base, codeGen &gen,
                                        bool store)
{
    GET_GPR(base,gen);  // loads value of stored register 'base' into EAX
    // either load the address or the contents at that address
    if(store) 
    {
        // dest = [reg](offset)
        emitMovRMToReg(REGNUM_EAX, REGNUM_EAX, offset, gen);
    }
    else
    {
        // dest = [reg] + offset
        emitAddRegImm32(REGNUM_EAX, offset, gen);
    }
    emitMovRegToRM(REGNUM_EBP, -1*(dest*4), REGNUM_EAX, gen);
} 

void Emitter32::emitLoadFrameAddr(Register dest, Address offset, codeGen &gen)
{
    emitMovRMToReg(REGNUM_EAX, REGNUM_EBP, 0, gen);       // mov (%ebp), %eax 
    emitAddRegImm32(REGNUM_EAX, offset, gen);        // add #<offset>, %eax
    emitMovRegToRM(REGNUM_EBP, -1*(dest*4), REGNUM_EAX, gen);    // mov -(dest*4)[ebp], eax
}



void Emitter32::emitLoadOrigRegister(Address register_num, Register dest, codeGen &gen)
{
    //Previous stack frame register is stored on the stack,
    //it was stored there at the begining of the base tramp.
    
    //Calculate the register's offset from the frame pointer in REGNUM_EBP
    unsigned offset = SAVED_EAX_OFFSET - (register_num * 4);

    emitMovRMToReg(REGNUM_EAX, REGNUM_EBP, offset, gen); //mov eax, offset[ebp]
    emitMovRegToRM(REGNUM_EBP, -1*(dest*4), REGNUM_EAX, gen); //mov dest, 0[eax]
}

void Emitter32::emitStore(Address addr, Register src, codeGen &gen)
{
      emitMovRMToReg(REGNUM_EAX, REGNUM_EBP, -1*(src*4), gen);    // mov eax, -(src*4)[ebp]
      emitMovRegToM(addr, REGNUM_EAX, gen);               // mov addr, eax
}

void Emitter32::emitStoreIndir(Register addr_reg, Register src, codeGen &gen)
{
    emitMovRMToReg(REGNUM_EAX, REGNUM_EBP, -1*(src*4), gen);   // mov eax, -(src*4)[ebp]
    emitMovRMToReg(REGNUM_ECX, REGNUM_EBP, -1*(addr_reg*4), gen);   // mov ecx, -(addr_reg*4)[ebp]
    emitMovRegToRM(REGNUM_ECX, 0, REGNUM_EAX, gen);           // mov [ecx], eax
}

void Emitter32::emitStoreFrameRelative(Address offset, Register src, Register scratch, codeGen &gen)
{
      // scratch = [ebp]	- saved bp
      // (offset)[scratch] = src
      emitMovRMToReg(scratch, REGNUM_EBP, 0, gen);    	    // mov scratch, (ebp)
      emitMovRMToReg(REGNUM_EAX, REGNUM_EBP, -1*(src*4), gen);    // mov eax, -(src*4)[ebp]
      emitMovRegToRM(scratch, offset, REGNUM_EAX, gen);        // mov (offset)[scratch], eax
}

void Emitter32::emitGetRetVal(Register dest, codeGen &gen)
{
    emitMovRMToReg(REGNUM_EAX, REGNUM_EBP, SAVED_EAX_OFFSET, gen);
    emitMovRegToRM(REGNUM_EBP, -1*(dest*4), REGNUM_EAX, gen);
}

void Emitter32::emitGetParam(Register dest, Register param_num, instPointType_t pt_type, codeGen &gen)
{
    // Parameters are addressed by a positive offset from ebp,
    // the first is PARAM_OFFSET[ebp]
    if(pt_type == callSite) {
	emitMovRMToReg(REGNUM_EAX, REGNUM_EBP, CALLSITE_PARAM_OFFSET + param_num*4, gen);
	emitMovRegToRM(REGNUM_EBP, -1*(dest*4), REGNUM_EAX, gen);
    } else {
	// assert(pt_type == functionEntry)
	emitMovRMToReg(REGNUM_EAX, REGNUM_EBP, FUNC_PARAM_OFFSET + param_num*4, gen);
	emitMovRegToRM(REGNUM_EBP, -1*(dest*4), REGNUM_EAX, gen);
    }
}

bool Emitter32::emitBTSaves(baseTramp* bt, codeGen &gen)
{

    // These crank the saves forward
    emitSimpleInsn(PUSHFD, gen);
    emitSimpleInsn(PUSHAD, gen);
    // For now, we'll do all saves then do the guard. Could inline
    // Return addr for stack frame walking; for lack of a better idea,
    // we grab the original instPoint address
    if (bt->instP()) {
        emitPushImm(bt->instP()->addr(), gen);
    }
    else {
        assert(bt->rpcMgr_);
    }

    emitSimpleInsn(PUSH_EBP, gen);
    emitMovRegToReg(REGNUM_EBP, REGNUM_ESP, gen);
    
    if (bt->isConservative() && gen.rs()->getSPFlag() && BPatch::bpatch->isSaveFPROn()) {
      if (gen.rs()->hasXMM)
	{
	  // Allocate space for temporaries
	  emitOpRegImm(EXTENDED_0x81_SUB, REGNUM_ESP, TRAMP_FRAME_SIZE, gen);

	  // need to save the floating point state (x87, MMX, SSE)
	  // we do this on the stack, but the problem is that the save
	  // area must be 16-byte aligned. the following sequence does
	  // the job:
	  //   mov %esp, %eax          ; copy the current stack pointer
	  //   sub $512, %esp          ; allocate space
	  //   and $0xfffffff0, %esp   ; make sure we're aligned (allocates some more space)
	  //   fxsave (%esp)           ; save the state
	  //   push %eax               ; save the old stack pointer
	  
	  emitMovRegToReg(REGNUM_EAX, REGNUM_ESP, gen);
	  emitOpRegImm(EXTENDED_0x81_SUB, REGNUM_ESP, 512, gen);
	  emitOpRegImm(EXTENDED_0x81_AND, REGNUM_ESP, -16, gen);
	  
	  // fxsave (%rsp) ; 0x0f 0xae 0x04 0x24
	  GET_PTR(insn, gen);
	  *insn++ = 0x0f;
	  *insn++ = 0xae;
	  *insn++ = 0x04;
	  *insn++ = 0x24;
	  SET_PTR(insn, gen);
	  
	  emitSimpleInsn(0x50 + REGNUM_EAX, gen); /* Push EAX */
	}
      else
	{
	  // Allocate space for temporaries and floating points
	  emitOpRegImm(EXTENDED_0x81_SUB, REGNUM_ESP, TRAMP_FRAME_SIZE+FSAVE_STATE_SIZE, gen);
	  emitOpRegRM(FSAVE, FSAVE_OP, REGNUM_EBP, -(TRAMP_FRAME_SIZE) - FSAVE_STATE_SIZE, gen);
	}
    } else {
        // Allocate space for temporaries
        emitOpRegImm(EXTENDED_0x81_SUB, REGNUM_ESP, TRAMP_FRAME_SIZE, gen);
    }
    return true;
}

bool Emitter32::emitBTRestores(baseTramp* bt, codeGen &gen)
{
    if (bt->isConservative() && gen.rs()->getSPFlag() && BPatch::bpatch->isSaveFPROn()) {
      if (gen.rs()->hasXMM)
	{
	  // pop the old ESP value into EAX
	  emitSimpleInsn(0x58 + REGNUM_EAX, gen);

	  // restore saved FP state
	  // fxrstor (%rsp) ; 0x0f 0xae 0x04 0x24
	  GET_PTR(insn, gen);
	  *insn++ = 0x0f;
	  *insn++ = 0xae;
	  *insn++ = 0x0c;
	  *insn++ = 0x24;
	  SET_PTR(insn, gen);
	  
	  // restore stack pointer (deallocates FP save area)
	  emitMovRegToReg(REGNUM_ESP, REGNUM_EAX, gen);
	}
      else
	emitOpRegRM(FRSTOR, FRSTOR_OP, REGNUM_EBP, -TRAMP_FRAME_SIZE - FSAVE_STATE_SIZE, gen);
    }
    emitSimpleInsn(LEAVE, gen);
    if (bt->rpcMgr_ == NULL)
	emitSimpleInsn(POP_EAX, gen);
    emitSimpleInsn(POPAD, gen);
    emitSimpleInsn(POPFD, gen);
    return true;
}

bool Emitter32::emitBTMTCode(baseTramp* bt, codeGen &gen)
{
    AstNode *threadPOS;
    pdvector<AstNode *> dummy;
    Register src = Null_Register;
    // registers cleanup

    assert(gen.rs() != NULL);
    //regSpace->resetSpace();

    dyn_thread *thr = gen.thread();
    if (!bt->threaded() && !bt->guarded()) {
       //Emit nothing
    }
    else if (!bt->threaded()) {
        /* Get the hashed value of the thread */
        emitVload(loadConstOp, 0, REG_MT_POS, REG_MT_POS, gen, false);
    }
    else if (thr) {
        // Override 'normal' index value...
        emitVload(loadConstOp, thr->get_index(), REG_MT_POS, REG_MT_POS, gen, false);
    }    
    else {
        threadPOS = AstNode::funcCallNode("DYNINSTthreadIndex", dummy, bt->proc());
        Address unused;
        threadPOS->generateCode(gen,
                                false, // noCost 
                                unused,
                                src);
        // AST generation uses a base pointer and a current address; the lower-level
        // code uses a current pointer. Convert between the two.
        LOAD_VIRTUAL32(src, gen);
        SAVE_VIRTUAL32(REG_MT_POS, gen);
    }
    return true;
}

bool Emitter32::emitBTGuardPreCode(baseTramp* bt, codeGen &gen, codeBufIndex_t& guardJumpIndex)
{
    assert(bt->guarded());

    // The jump gets filled in later; allocate space for it now
    // and stick where it is in guardJumpOffset
    Address guard_flag_address = bt->proc()->trampGuardBase();
    if (!guard_flag_address) {
        return false;
    }
    
   /* guard-on code before pre instr */
   /* Code generated:
    * --------------
    * cmpl   $0x0, (guard flag address)
    * je     <after guard-off code>
    * movl   $0x0, (guard flag address)
    */

   /*            CMP_                       (memory address)__  0 */
    if (bt->threaded()) 
    {
       // Load the index into REGNUM_EAX
       LOAD_VIRTUAL32(REG_MT_POS, gen);
       // Shift by sizeof(int) and add guard_flag_address
       emitLEA(Null_Register, REGNUM_EAX, 2, guard_flag_address, REGNUM_EAX, gen);
       // Compare to 0
       emitOpRMImm8(0x83, 0x07, REGNUM_EAX, 0, 0, gen);
    }
    else {
        emitOpRMImm8(0x83, 0x07, Null_Register, guard_flag_address, 0, gen);
    }
    guardJumpIndex = gen.getIndex();

    emitJcc(0x04, 0, gen);

    bt->guardBranchSize = gen.getDisplacement(guardJumpIndex, gen.getIndex());

    if (bt->threaded())
    {
       emitMovImmToRM(REGNUM_EAX, 0, 0, gen);
    }
    else {
       emitMovImmToMem( guard_flag_address, 0, gen );
    }
    
    return true;
}

bool Emitter32::emitBTGuardPostCode(baseTramp* bt, codeGen &gen, codeBufIndex_t& guardTargetIndex)
{
    assert(bt->guarded());
    Address guard_flag_address = bt->proc()->trampGuardBase();
    if (!guard_flag_address) {
        return false;
    }
    
    assert(guard_flag_address);
    /* guard-off code after pre instr */
   /* Code generated:
    * --------------
    * movl   $0x1, (guard flag address)
    */
   if (bt->threaded())
   {
       // Load the index into REGNUM_EAX
       LOAD_VIRTUAL32(REG_MT_POS, gen);
       // Shift by sizeof(int) and add guard_flag_address
       emitLEA(Null_Register, REGNUM_EAX, 2, guard_flag_address, REGNUM_EAX, gen);
       emitMovImmToRM(REGNUM_EAX, 0, 1, gen);
   }
   else {
       emitMovImmToMem( guard_flag_address, 1, gen );
   }
   guardTargetIndex = gen.getIndex();

   return true;
}

bool Emitter32::emitBTCostCode(baseTramp* bt, codeGen &gen, unsigned& costUpdateOffset)
{
    Address costAddr = bt->proc()->getObservedCostAddr();
    if (!costAddr) return false;

    costUpdateOffset = gen.used();
    // Dummy for now; we update at generation time.
    emitAddMemImm32(costAddr, 0, gen); 
    return true;
}

int Emitter32::Register_DWARFtoMachineEnc(int n)
{
    return n;   // no mapping for 32-bit targets
}

Emitter32 emitter32;

//
// 64-bit code generation helper functions
//

static void emitRex(bool is_64, Register* r, Register* x, Register* b, codeGen &gen)
{
    unsigned char rex = 0x40;

    // need rex for 64-bit ops in most cases
    if (is_64)
       rex |= 0x08;

    // need rex for use of new registers
    // if a new register is used, we mask off the high bit before
    // returning since we account for it in the rex prefix
    
    // "R" register - extension to ModRM reg field
    if (r && *r & 0x08) {
       rex |= 0x04;
       *r &= 0x07;
    }
    
    // "X" register - extension to SIB index field
    if (x && *x & 0x08) {
       rex |= 0x02;
       *x &= 0x07;
    }

    // "B" register - extension to ModRM r/m field, SIB base field,
    // or opcode reg field
    if (b && *b & 0x08) {
       rex |= 0x01;
       *b &= 0x07;
    }
    
    // emit the rex, if needed
    // (note that some other weird cases not covered here
    //  need a "blank" rex, like using %sil or %dil)
    if (rex & 0x0f)
       emitSimpleInsn(rex, gen);
}

void Emitter32::emitStoreImm(Address addr, int imm, codeGen &gen, bool /*noCost*/) 
{
   emitMovImmToMem(addr, imm, gen);
}

void emitAddMem(Address addr, int imm, codeGen &gen) {
   //This add needs to encode "special" due to an exception
   // to the normal encoding rules and issues caused by AMD64's
   // pc-relative data addressing mode.  Our helper functions will
   // not correctly emit what we want, and we want this very specific
   // mode for the add instruction.  So I'm just writing raw bytes.
   GET_PTR(insn, gen);
   if (imm == 1) 
      *insn++ = 0xFF; //incl 
   else
      *insn++ = 0x81; //addl
               
   *insn++ = 0x04; *insn++ = 0x25; //Add to an absolute memory address
   
   *((int *)insn) = addr; //Write address
   insn += sizeof(int);

   if (imm != 1) {
      *((int*)insn) = imm; //Write immediate value to add
      insn += sizeof(int);
   }

   SET_PTR(insn, gen);
}

void Emitter32::emitAddSignedImm(Address addr,int imm, codeGen &gen,
                                 bool /*noCost*/)
{
   emitAddMem(addr, imm, gen);
}


void emitMovImmToReg64(Register dest, long imm, bool is_64, codeGen &gen)
{
    Register tmp_dest = dest;
    emitRex(is_64, NULL, NULL, &tmp_dest, gen);
    if (is_64) {
	GET_PTR(insn, gen);
	*insn++ = 0xB8 + tmp_dest;
	*((long *)insn) = imm;
	insn += sizeof(long);
	SET_PTR(insn, gen);
    }
    else
	emitMovImmToReg(tmp_dest, imm, gen);
}


#if defined(arch_x86_64)

bool isImm64bit(Address imm) {
   return (imm >> 32);
}

void emitMovRegToReg64(Register dest, Register src, bool is_64, codeGen &gen)
{
	Register tmp_dest = dest;
	Register tmp_src = src;
	emitRex(is_64, &tmp_dest, NULL, &tmp_src, gen);
	emitMovRegToReg(tmp_dest, tmp_src, gen);
}

void emitLEA64(Register base, Register index, unsigned int scale, int disp,
	       Register dest, bool is_64, codeGen &gen)
{
    Register tmp_base = base;
    Register tmp_index = index;
    Register tmp_dest = dest;
    emitRex(is_64, &tmp_dest,
	    tmp_index == Null_Register ? NULL : &tmp_index,
	    tmp_base == Null_Register ? NULL : &tmp_base,
	    gen);
    emitLEA(tmp_base, tmp_index, scale, disp, tmp_dest, gen);
}

static void emitMovRMToReg64(Register dest, Register base, int disp, bool is_64, codeGen &gen)
{
    Register tmp_dest = dest;
    Register tmp_base = base;
    emitRex(is_64, &tmp_dest, NULL, &tmp_base, gen);
    emitMovRMToReg(tmp_dest, tmp_base, disp, gen);
}

static void emitMovRegToRM64(Register base, int disp, Register src, bool is_64, codeGen &gen)
{
    Register tmp_base = base;
    Register tmp_src = src;
    emitRex(is_64, &tmp_src, NULL, &tmp_base, gen);
    emitMovRegToRM(tmp_base, disp, tmp_src, gen);
}

static void emitOpRegReg64(unsigned opcode, Register dest, Register src, bool is_64, codeGen &gen)
{
    Register tmp_dest = dest;
    Register tmp_src = src;
    emitRex(is_64, &tmp_dest, NULL, &tmp_src, gen);
    emitOpRegReg(opcode, tmp_dest, tmp_src, gen);
}

static void emitOpRegRM64(unsigned opcode, Register dest, Register base, int disp, bool is_64, codeGen &gen)
{
    Register tmp_dest = dest;
    Register tmp_base = base;
    emitRex(is_64, &tmp_dest, NULL, &tmp_base, gen);
    emitOpRegRM(opcode, tmp_dest, tmp_base, disp, gen);
}

static void emitOpRegImm64(unsigned opcode, unsigned opcode_ext, Register rm_reg, int imm,
			   bool is_64, codeGen &gen)
{
    Register tmp_rm_reg = rm_reg;
    emitRex(is_64, NULL, NULL, &tmp_rm_reg, gen);

    GET_PTR(insn, gen);
    *insn++ = opcode;
    *insn++ = 0xC0 | ((opcode_ext & 0x7) << 3) | tmp_rm_reg;
    *((int *)insn) = imm;
    insn+= sizeof(int);
    SET_PTR(insn, gen);
}

// operation on memory location specified with a base register
// (does not work for RSP, RBP, R12, R13)
static void emitOpMemImm64(unsigned opcode, unsigned opcode_ext, Register base,
			  int imm, bool is_64, codeGen &gen)
{
    Register tmp_base = base;
    emitRex(is_64, NULL, NULL, &tmp_base, gen);

    GET_PTR(insn, gen);
    *insn++ = opcode;
    *insn++ = ((opcode_ext & 0x7) << 3) | tmp_base;
    *((int *)insn) = imm;
    insn+= sizeof(int);
    SET_PTR(insn, gen);
}

static void emitOpRegRegImm64(unsigned opcode, Register dest, Register src1, int imm,
			      bool is_64, codeGen &gen)
{
    emitOpRegReg64(opcode, dest, src1, is_64, gen);
    GET_PTR(insn, gen);
    *((int *)insn) = imm;
    insn+= sizeof(int);
    SET_PTR(insn, gen);
}

static void emitOpRegImm8_64(unsigned opcode, unsigned opcode_ext, Register dest,
			     char imm, bool is_64, codeGen &gen)
{
    Register tmp_dest = dest;
    emitRex(is_64, NULL, NULL, &tmp_dest, gen);
    GET_PTR(insn, gen);
    *insn++ = opcode;
    *insn++ = 0xC0 | ((opcode_ext & 0x7) << 3) | tmp_dest;
    *insn++ = imm;
    SET_PTR(insn, gen);
}

void emitPushReg64(Register src, codeGen &gen)
{
    emitRex(false, NULL, NULL, &src, gen);
    emitSimpleInsn(0x50 + src, gen);
}

void emitPopReg64(Register dest, codeGen &gen)
{
    emitRex(false, NULL, NULL, &dest, gen);    
    emitSimpleInsn(0x58 + dest, gen);
}

void emitMovImmToRM64(Register base, int disp, int imm, bool is_64, 
                      codeGen &gen) 
{
   GET_PTR(insn, gen);
   if (base == Null_Register) {
      *insn++ = 0xC7;
      *insn++ = 0x84;
      *insn++ = 0x25;
      *((int*)insn) = disp;
      insn += sizeof(int);
   }
   else {
      emitRex(is_64, &base, NULL, NULL, gen);
      *insn++ = 0xC7;
      SET_PTR(insn, gen);
      emitAddressingMode(base, disp, 0, gen);
      REGET_PTR(insn, gen);
   }
   *((int*)insn) = imm;
   insn += sizeof(int);
   SET_PTR(insn, gen);
}

void emitAddRM64(Register dest, int imm, bool is_64, codeGen &gen)
{
   GET_PTR(insn, gen);
   if (imm == 1) {
      *insn++ = 0xFF;
      emitRex(is_64, &dest, NULL, NULL, gen);
      emitAddressingMode(dest, 0, 0, gen);
   }
   REGET_PTR(insn, gen);
   *((int*)insn) = imm;
   insn += sizeof(int);
   SET_PTR(insn, gen);   
}


bool Emitter64::emitMoveRegToReg(Register src, Register dest, codeGen &gen) {
    emitMovRegToReg64(dest, src, true, gen);
    return true;
}


codeBufIndex_t Emitter64::emitIf(Register expr_reg, Register target, codeGen &gen)
{
    Register scratchReg = gen.rs()->getScratchRegister(gen, true);

    // sub RAX, RAX
    emitOpRegReg64(0x29, scratchReg, scratchReg, true, gen);

    assert(scratchReg != expr_reg);

    // cmp %expr_reg, RAX
    emitOpRegReg64(0x3B, scratchReg, expr_reg, true, gen);

    // Retval: where the jump is in this sequence
    codeBufIndex_t retval = gen.getIndex();

    // Jump displacements are from the end of the insn, not start. The
    // one we're emitting has a size of 6.
    int disp = target - 6;

    // jz target
    GET_PTR(insn, gen);
    *insn++ = 0x0F;
    *insn++ = 0x84;
    *((int *)insn) = disp;
    insn += sizeof(int);
    SET_PTR(insn, gen);

    return retval;
}

void Emitter64::emitOp(unsigned opcode, Register dest, Register src1, Register src2, codeGen &gen)
{
    // TODO: optimize this further for ops where order doesn't matter
    if (src1 != dest)
	emitMovRegToReg64(dest, src1, true, gen);
    emitOpRegReg64(opcode, dest, src2, true, gen);
}

void Emitter64::emitOpImm(unsigned opcode1, unsigned opcode2, Register dest, Register src1, RegValue src2imm,
			  codeGen &gen)
{
    if (src1 != dest) {
	emitMovRegToReg64(dest, src1, true, gen);
    }
    emitOpRegImm64(opcode1, opcode2, dest, src2imm, true, gen);
}

void Emitter64::emitRelOp(unsigned op, Register dest, Register src1, Register src2, codeGen &gen)
{
    // cmp %src2, %src1
    emitOpRegReg64(0x39, src2, src1, true, gen);

    // mov $0, $dest ; done now in case src1 == dest or src2 == dest
    // (we can do this since mov doesn't mess w/ flags)
    emitMovImmToReg64(dest, 0, false, gen);

    // jcc by two or three, depdending on size of mov
    unsigned char jcc_opcode = jccOpcodeFromRelOp(op);
    GET_PTR(insn, gen);
    *insn++ = jcc_opcode;
    codeBuf_t* disp = insn;
    insn++;
    codeBuf_t* after_jcc_insn = insn;
    
    // mov $1,  %dest
    SET_PTR(insn, gen);
    emitMovImmToReg64(dest, 1, false, gen);
    REGET_PTR(insn, gen);

    // write in the correct displacement
    *disp = (insn - after_jcc_insn);

    SET_PTR(insn, gen);
}

void Emitter64::emitRelOpImm(unsigned op, Register dest, Register src1, RegValue src2imm,
			     codeGen &gen)
{
    // cmp $src2imm, %src1
    emitOpRegImm64(0x81, 7, src1, src2imm, true, gen);

    // mov $0, $dest ; done now in case src1 == dest
    // (we can do this since mov doesn't mess w/ flags)
    emitMovImmToReg64(dest, 0, false, gen);

    // jcc by two or three, depdending on size of mov
    unsigned char opcode = jccOpcodeFromRelOp(op);
    GET_PTR(insn, gen);
    *insn++ = opcode;
    codeBuf_t* disp = insn;
    insn++;
    codeBuf_t* after_jcc_insn = insn;

    // mov $1,  %dest
    SET_PTR(insn, gen);
    emitMovImmToReg64(dest, 1, false, gen);
    REGET_PTR(insn, gen);

    // write in the correct displacement
    *disp = (insn - after_jcc_insn);

    SET_PTR(insn, gen);
}

void Emitter64::emitDiv(Register dest, Register src1, Register src2, codeGen &gen)
{
    // TODO: fix so that we don't always use RAX

    // push RDX if it's in use, since we will need it
    bool save_rdx = false;
    if (!gen.rs()->isFreeRegister(REGNUM_RDX)) {
	save_rdx = true;
	emitPushReg64(REGNUM_RDX, gen);
    }

    // mov %src1, %rax
    emitMovRegToReg64(REGNUM_RAX, src1, true, gen);

    // cqo (sign extend RAX into RDX)
    emitSimpleInsn(0x48, gen); // REX.W
    emitSimpleInsn(0x99, gen);

    // idiv %src2
    emitOpRegReg64(0xF7, 0x7, src2, true, gen);

    // mov %rax, %dest
    emitMovRegToReg64(dest, REGNUM_RAX, true, gen);

    // pop rdx if it needed to be saved
    if (save_rdx)
	emitPopReg64(REGNUM_RDX, gen);
}

void Emitter64::emitTimesImm(Register dest, Register src1, RegValue src2imm, codeGen &gen)
{
    int result = -1;

    if (isPowerOf2(src2imm, result) && result <= MAX_IMM8) {

	// immediate is a power of two - use a shift

	// mov %src1, %dest (if needed)
	if (src1 != dest) {
	    emitMovRegToReg64(dest, src1, true, gen);
	}
	// sal dest, result
	emitOpRegImm8_64(0xC1, 4, dest, result, true, gen);
    }
    else {

	// imul %dest, %src1, $src2imm
	emitOpRegRegImm64(0x69, dest, src1, src2imm, true, gen);
    } 
}

void Emitter64::emitDivImm(Register dest, Register src1, RegValue src2imm, codeGen &gen)
{
    int result = -1;
    if (isPowerOf2(src2imm, result) && result <= MAX_IMM8) {

	// divisor is a power of two - use a shift instruction

	// mov %src1, %dest (if needed)
	if (src1 != dest) {
	    emitMovRegToReg64(dest, src1, true, gen);
	}

	// sar $result, %dest
	emitOpRegImm8_64(0xC1, 7, dest, result, true, gen);
    }
    else {

	// push RDX if it's in use, since we will need it
	bool save_rdx = false;
	if (!gen.rs()->isFreeRegister(REGNUM_RDX)) {
	    save_rdx = true;
	    emitPushReg64(REGNUM_RDX, gen);
	}

	// need to put dividend in RDX:RAX
	// mov %src1, %rax
	// cqo
	emitMovRegToReg64(REGNUM_EAX, src1, true, gen);
	emitSimpleInsn(0x48, gen); // REX.W
	emitSimpleInsn(0x99, gen);

	// push immediate operand on the stack (no IDIV $imm)
	emitPushImm(src2imm, gen);

	// idiv (%rsp)
	emitOpRegRM64(0xF7, 0x7 /* opcode extension */, REGNUM_RSP, 0, true, gen);

	// mov %rax, %dest ; set the result
	emitMovRegToReg64(dest, REGNUM_RAX, true, gen);

	// pop the immediate off the stack
	// add $8, %rsp
	emitOpRegImm8_64(0x83, 0x0, REGNUM_RSP, 8, true, gen);

	// pop rdx if it needed to be saved
	if (save_rdx)
	    emitPopReg64(REGNUM_RDX, gen);
    }
}

void Emitter64::emitLoad(Register dest, Address addr, int size, codeGen &gen)
{

    if (size == 1) {
	assert(0);
    } else if (size == 2) {
	assert(0);
    } else {
	assert(size == 4 || size == 8);
	Register scratch = gen.rs()->getScratchRegister(gen);

	// mov $addr, %rax
	emitMovImmToReg64(scratch, addr, true, gen);
	
	// mov (%rax), %dest
	emitMovRMToReg64(dest, scratch, 0, size == 8, gen);
    }
}

void Emitter64::emitLoadConst(Register dest, Address imm, codeGen &gen)
{
    emitMovImmToReg64(dest, imm, true, gen);
}

void Emitter64::emitLoadIndir(Register dest, Register addr_src, codeGen &gen)
{
    emitMovRMToReg64(dest, addr_src, 0, false, gen);
}

void Emitter64::emitLoadOrigFrameRelative(Register dest, Address offset, codeGen &gen)
{
    Register scratch = gen.rs()->getScratchRegister(gen);
    // mov (%rbp), %rax
    emitMovRMToReg64(scratch, REGNUM_RBP, 0, true, gen);

    // mov offset(%rax), %dest
    emitMovRMToReg64(dest, scratch, offset, false, gen);
}

bool Emitter64::emitLoadRelative(Register dest, Address offset, Register base, codeGen &gen)
{
    // mov offset(%base), %dest
    emitMovRMToReg64(dest, base, offset*gen.proc()->getAddressWidth(), false, gen);
    return true;
}

void Emitter64::emitLoadFrameAddr(Register dest, Address offset, codeGen &gen)
{
    // mov (%rbp), %dest
    emitMovRMToReg64(dest, REGNUM_RBP, 0, true, gen);

    // add $offset, %dest
    emitOpRegImm64(0x81, 0x0, dest, offset, true, gen);
}

void Emitter64::emitLoadOrigRegRelative(Register dest, Address offset,
                                        Register base, codeGen &gen,
                                        bool store)
{
    Register scratch = gen.rs()->getScratchRegister(gen);
    // either load the address or the contents at that address
    if(store) 
    {
        // load the stored register 'base' into RAX
        emitLoadOrigRegister(base, scratch, gen);
        // move offset(%rax), %dest
        emitMovRMToReg64(dest, scratch, offset, false, gen);
    }
    else
    {
        // load the stored register 'base' into dest
        emitLoadOrigRegister(base, dest, gen);
        // add $offset, %dest
        emitOpRegImm64(0x81, 0x0, dest, offset, true, gen);
    }
} 
// this is the distance on the basetramp stack frame from the
// start of the GPR save region to where the base pointer is,
// in 8-byte quadwords
#define GPR_SAVE_REGION_OFFSET 17

void Emitter64::emitLoadOrigRegister(Address register_num, Register dest, codeGen &gen)
{

    gen.rs()->readRegister(gen, register_num, dest);
}

void Emitter64::emitStore(Address addr, Register src, codeGen &gen)
{
    Register scratch = gen.rs()->getScratchRegister(gen);

    // FIXME: we assume int (size == 4) for now

    // mov $addr, %rax
    emitMovImmToReg64(scratch, addr, true, gen);

    // mov %src, (%rax)
    emitMovRegToRM64(scratch, 0, src, false, gen);
}

void Emitter64::emitStoreIndir(Register addr_reg, Register src, codeGen &gen)
{
    // FIXME: we assume int (size == 4) for now
    emitMovRegToRM64(addr_reg, 0, src, false, gen);
}

void Emitter64::emitStoreFrameRelative(Address offset, Register src, Register /*scratch*/, codeGen &gen)
{
    Register scratch = gen.rs()->getScratchRegister(gen);
    // FIXME: we assume int (size == 4) for now

    // mov (%rbp), %rax
    emitMovRMToReg64(scratch, REGNUM_RBP, 0, true, gen);
    
    // mov %src, offset(%rax)
    emitMovRegToRM64(scratch, offset, src, false, gen);
}


void Emitter64::setFPSaveOrNot(const int * liveFPReg,bool saveOrNot)
{
  if (liveFPReg != NULL)
    {
      if (liveFPReg[0] == 0 && saveOrNot)
	{
	  int * temp = const_cast<int *>(liveFPReg);
	  temp[0] = 1;
	}
    }
}



/* Recursive function that goes to where our instrumentation is calling
to figure out what registers are clobbered there, and in any function
that it calls, to a certain depth ... at which point we clobber everything*/
bool Emitter64::clobberAllFuncCall( registerSpace *rs,
		   process *proc, 
		   int_function *callee,
		    int level)
		   
{
    if (callee == NULL) return false;
    InstrucIter ah(callee);
    
    //while there are still instructions to check for in the
    //address space of the function
    
    while (ah.hasMore()) 
        {
            if (ah.isFPWrite())
                return true;
            if (ah.isACallInstruction()) {
                if (level >= 1)
                    return true;
		instPoint *ip = callee->findInstPByAddr(*ah);
		if (!ip)
		  return true;
		int_function *call = ip->findCallee();
		if (!call || clobberAllFuncCall(rs, proc, call, level+1))
		    return true;
            }
            ah++;
        }
   return false;
}


static Register amd64_arg_regs[] = {REGNUM_RDI, REGNUM_RSI, REGNUM_RDX, REGNUM_RCX, REGNUM_R8, REGNUM_R9};
#define AMD64_ARG_REGS (sizeof(amd64_arg_regs) / sizeof(Register))
Register Emitter64::emitCall(opCode op, codeGen &gen, const pdvector<AstNode *> &operands,
			     bool noCost, int_function *callee)
{

    assert(op == callOp);
    pdvector <Register> srcs;
   
    //  Sanity check for NULL address arg
    if (!callee) {
	char msg[256];
	sprintf(msg, "%s[%d]:  internal error:  emitFuncCall called w/out"
		"callee argument", __FILE__, __LINE__);
	showErrorCallback(80, msg);
	assert(0);
    }


    // generate code for arguments
    for (unsigned u = 0; u < operands.size(); u++) {
        Address unused = ADDR_NULL;
        Register reg = REG_NULL;
	if (!operands[u]->generateCode_phase2(gen,
                                              noCost, 
                                              unused,
                                              reg)) assert(0);
        srcs.push_back(reg);
    }

    // here we save the argument registers we're going to use
    // then we use the stack to shuffle everything into the right place
    // FIXME: optimize this a bit - this initial implementation is very conservative
    const unsigned num_args = srcs.size();
    assert(num_args <= AMD64_ARG_REGS);
    for (unsigned u = 0; u < num_args; u++)
      {
	emitPushReg64(amd64_arg_regs[u], gen);

      }
    for (unsigned u = 0; u < num_args; u++)
      {
	emitPushReg64(srcs[u], gen);

      }
    for (int i = num_args - 1; i >= 0; i--)
      {
	emitPopReg64(amd64_arg_regs[i], gen);

      }

    // make the call (using an indirect call)
    emitMovImmToReg64(REGNUM_EAX, callee->getAddress(), true, gen);
    emitSimpleInsn(0xff, gen); // group 5
    emitSimpleInsn(0xd0, gen); // mod = 11, reg = 2 (call Ev), r/m = 0 (RAX)
    
    // restore argument registers
    for (int i = num_args - 1; i >= 0; i--)
	emitPopReg64(amd64_arg_regs[i], gen);   
    
    // allocate a (virtual) register to store the return value
    Register ret = gen.rs()->allocateRegister(gen, noCost);
    emitMovRegToReg64(ret, REGNUM_EAX, true, gen);

    // Figure out if we need to save FPR in base tramp
    bool useFPR = clobberAllFuncCall(gen.rs(), gen.proc(), callee, 0);

    if (gen.point() != NULL)
        setFPSaveOrNot(gen.point()->liveFPRegisters(), useFPR);

    return ret;
}

// FIXME: comment here on the stack layout
void Emitter64::emitGetRetVal(Register dest, codeGen &gen)
{
    emitLoadOrigRegister(REGNUM_RAX, dest, gen);
}

void Emitter64::emitGetParam(Register dest, Register param_num, instPointType_t /*pt_type*/, codeGen &gen)
{
    assert(param_num <= 6);
    emitLoadOrigRegister(amd64_arg_regs[param_num], dest, gen);
}

static void emitPushImm16_64(unsigned short imm, codeGen &gen)
{
    GET_PTR(insn, gen);

    // operand-size prefix
    *insn++ = 0x66;

    // PUSH imm opcode
    *insn++ = 0x68;

    // and the immediate
    *(unsigned short*)insn = imm;
    insn += 2;

    SET_PTR(insn, gen);
}

void Emitter64::emitFuncJump(Address addr, instPointType_t ptType, codeGen &gen)
{
    if (ptType == otherPoint) {
		// pop the old RSP value into RAX
		emitPopReg64(REGNUM_RAX, gen);
	
		// restore saved FP state
		// fxrstor (%rsp) ; 0x0f 0xae 0x04 0x24
		GET_PTR(insn, gen);
		*insn++ = 0x0f;
		*insn++ = 0xae;
		*insn++ = 0x0c;
		*insn++ = 0x24;
		SET_PTR(insn, gen);
	
		// restore stack pointer (deallocates FP save area)
		emitMovRegToReg64(REGNUM_RSP, REGNUM_RAX, true, gen);
    }

    // tear down the stack frame (LEAVE)
    emitSimpleInsn(0xC9, gen);

    // pop "fake" return address
    emitPopReg64(REGNUM_RAX, gen);

    if (gen.rs()->getSPFlag() && 0) {
       // restore saved registers (POP R15, POP R14, ...)
       for (int reg = 15; reg >= 0; reg--) {
          emitPopReg64(reg, gen);
       }
    }
    else {
       // Count the saved registers
       int num_saved = 0; // RAX, RSP, RBP always saved
       for(int i = gen.rs()->getRegisterCount()-1; i >= 0; i--) {
			registerSlot * reg = gen.rs()->getRegSlot(i);
    		if (reg->startsLive ||
				(reg->number == REGNUM_RAX) ||
				(reg->number == REGNUM_RBX) ||
				(reg->number == REGNUM_RSP) ||
				(reg->number == REGNUM_RBP)) {
					num_saved++;
         }
       }
	  
	  	// move SP up to end of GPR save area
	  	if (num_saved < 16) {
	      	emitOpRegImm8_64(0x83, 0x0, REGNUM_RSP, 8 * (16 - num_saved), true, gen);
	  	}

		// Save the live ones
		for(int i = gen.rs()->getRegisterCount()-1; i >= 0; i--) {
	    	registerSlot * reg = gen.rs()->getRegSlot(i);
    		if (reg->startsLive ||
				(reg->number == REGNUM_RAX) ||
				(reg->number == REGNUM_RBX) ||
				(reg->number == REGNUM_RSP) ||
				(reg->number == REGNUM_RBP)) {
				emitPopReg64(reg->number,gen);
			}
		}    
	}
    // restore flags (POPFQ)
    emitSimpleInsn(0x9D, gen);

    // restore stack pointer (use LEA to not affect flags)
    GET_PTR(insn, gen);
    *insn++ = 0x48; // REX.W
    *insn++ = 0x8D; // LEA opcode
    *insn++ = 0xA4; // ModRM: [SIB + disp32], %rsp
    *insn++ = 0x24; // SIB: base = RSP
    *(unsigned int*)insn = 128; // displacement: 128
    insn += 4;
    SET_PTR(insn, gen);

    // push the address to jump to...
    // (need to do this in 16-bit chunks - weird huh)
    emitPushImm16_64((unsigned short)(addr >> 48), gen);
    emitPushImm16_64((unsigned short)((addr & 0x0000ffffffffffff) >> 32), gen);
    emitPushImm16_64((unsigned short)((addr & 0x00000000ffffffff) >> 16), gen);
    emitPushImm16_64((unsigned short)(addr & 0x000000000000ffff), gen);
    
    // and return
    emitSimpleInsn(0xc3, gen);

    // And an illegal just in case we come back here... which we shouldn't.
    REGET_PTR(insn, gen);
    *insn++ = 0x0f;
    *insn++ = 0x0b;   
    SET_PTR(insn, gen);
}


void Emitter64::emitASload(int ra, int rb, int sc, long imm, Register dest, codeGen &gen)
{
    Register scratch = Null_Register;
    bool havera = ra > -1, haverb = rb > -1;

    // if ra is specified, move its inst-point value into our
    // destination register
    if(havera) {
        if (ra == mRIP) {
            // special case: rip-relative data addressing
            // the correct address has been stuffed in imm
            emitMovImmToReg64(dest, imm, true, gen);
            return;
        }
        emitLoadOrigRegister(ra, dest, gen);
    }
    
    // if rb is specified, move its inst-point value into RAX
    if(haverb) {
        scratch = gen.rs()->getScratchRegister(gen);
        emitLoadOrigRegister(rb, scratch, gen);
    }
    // emitLEA64 will not handle the [disp32] case properly, so
    // we special case that
    if (!havera && !haverb)
        emitMovImmToReg64(dest, imm, false, gen);
    else {
        emitLEA64((havera ? dest : Null_Register), (haverb ? scratch : Null_Register),
                  sc, (int)imm, dest, true, gen);
    }
}

void Emitter64::emitCSload(int ra, int rb, int sc, long imm, Register dest, codeGen &gen)
{
   // count is at most 1 register or constant or hack (aka pseudoregister)
   assert((ra == -1) &&
          ((rb == -1) ||
            ((imm == 0) && (rb == 1 /*REGNUM_ECX */ || rb >= IA32_EMULATE))));

   if(rb >= IA32_EMULATE) {
       
       // need to emulate repeated SCAS or CMPS to figure out byte count

       // TODO: firewall code to ensure that direction is up

       bool neg = false;
       unsigned char opcode_small, opcode_large;
       bool restore_rax = false;
       bool restore_rsi = false;

       bool rax_wasUsed = false;
       bool rsi_wasUsed = false;
       bool rdi_wasUsed = false;
       bool rcx_wasUsed = false;
      
       switch(rb) {
       case IA32_NESCAS:
           neg = true;
       case IA32_ESCAS:
	   opcode_small = 0xAE;
	   opcode_large = 0xAF;
	   restore_rax = true;
	   break;
       case IA32_NECMPS:
           neg = true;
       case IA32_ECMPS:
	   opcode_small = 0xA6;
	   opcode_large = 0xA7;
	   restore_rsi = true;
	   break;
       default:
           assert(!"Wrong emulation!");
       }
      
       // restore flags (needed for direction flag)
       code_emitter->emitRestoreFlagsFromStackSlot(gen);

       // restore needed registers to values at the inst point
       // (push current values on the stack in case they're in use)
       if (restore_rax) {
           // We often use RAX as a destination register - in this case,
           // it's allocated but by us. And we really don't want to save 
           // it and then restore...
           if (!gen.rs()->isFreeRegister(REGNUM_RAX) && (dest != REGNUM_RAX)) {
               rax_wasUsed = true;
               emitPushReg64(REGNUM_RAX, gen);
           }
	   emitLoadOrigRegister(REGNUM_RAX, REGNUM_RAX, gen);
       }
       if (restore_rsi) {
           if (!gen.rs()->isFreeRegister(REGNUM_RSI)) {
               rsi_wasUsed = true;
               emitPushReg64(REGNUM_RSI, gen);
           }
           emitLoadOrigRegister(REGNUM_RSI, REGNUM_RSI, gen);
       }
       if (!gen.rs()->isFreeRegister(REGNUM_RDI)) {
           rdi_wasUsed = true;
           emitPushReg64(REGNUM_RDI, gen);
       }
       emitLoadOrigRegister(REGNUM_RDI, REGNUM_RDI, gen);
       if (!gen.rs()->isFreeRegister(REGNUM_RCX)) {
           rcx_wasUsed = true;
           emitPushReg64(REGNUM_RCX, gen);
       }
       emitLoadOrigRegister(REGNUM_RCX, REGNUM_RCX, gen);

       // emulate the string instruction
       emitSimpleInsn(neg ? 0xF2 : 0xF3, gen); // rep(n)e
       if (sc == 0)
	  emitSimpleInsn(opcode_small, gen);
       else {
	   if (sc == 1)
	       emitSimpleInsn(0x66, gen); // operand size prefix
	   else if (sc == 3)
	       emitSimpleInsn(0x48, gen); // REX.W
	   emitSimpleInsn(opcode_large, gen);
       }

       // RCX has now been decremented by the number of repititions
       // load old RCX into RAX and compute difference
       emitLoadOrigRegister(REGNUM_RCX, dest, gen);
       emitOp(0x2B, dest, dest, REGNUM_RCX, gen);

       // restore registers we stomped on
       if (rcx_wasUsed)
           emitPopReg64(REGNUM_RCX, gen);
       if (rdi_wasUsed)
           emitPopReg64(REGNUM_RDI, gen);
       if (rsi_wasUsed)
	   emitPopReg64(REGNUM_RSI, gen);       
       if (rax_wasUsed)
           emitPopReg64(REGNUM_RAX, gen);
   }
   else if(rb > -1) {

       // count spec is simple register with scale
       // TODO: 16-bit pseudoregisters
       assert(rb < 16);

       // store the register into RAX
       Register scratch = gen.rs()->getScratchRegister(gen);
       emitLoadOrigRegister(rb, scratch, gen);

       // shift left by the given scale
       // emitTimesImm will do the right thing
       if(sc > 0)
	   emitTimesImm(dest, scratch, 1 << sc, gen);
   }
   else
       emitMovImmToReg64(dest, (int)imm, true, gen);       
}

// this is the distance in 8-byte quadwords from the frame pointer
// in our basetramp's stack frame to the saved value of RFLAGS
// (1 qword for our false return address, 16 for the saved registers, 1 more for the flags)
#define SAVED_RFLAGS_OFFSET 18

void Emitter64::emitRestoreFlags(codeGen &gen, unsigned offset)
{
    if (offset)
        emitOpRMReg(PUSH_RM_OPC1, REGNUM_ESP, offset*8, PUSH_RM_OPC2, gen);
    emitSimpleInsn(0x9D, gen);
}

void Emitter64::emitPushFlags(codeGen &gen) {
    // save flags (PUSHFQ)
    emitSimpleInsn(0x9C, gen);
}

void Emitter64::emitRestoreFlagsFromStackSlot(codeGen &gen)
{
    emitRestoreFlags(gen, SAVED_RFLAGS_OFFSET);
}

bool Emitter64::emitBTSaves(baseTramp* bt, codeGen &gen)
{


   // skip past the red zone
   // (we use LEA to avoid overwriting the flags)
    GET_PTR(buffer, gen);
    *buffer++ = 0x48; // REX.W
    *buffer++ = 0x8D; // LEA opcode
    *buffer++ = 0x64; // ModRM: [SIB + disp8], %rsp
    *buffer++ = 0x24; // SIB: base = RSP
    *buffer++ = 0x80; // displacement: -128
    SET_PTR(buffer, gen);

    // save flags (PUSHFQ)
    emitSimpleInsn(0x9C, gen);

    // We use RAX implicitly all over the place... so mark it read-only
    //gen.rs()->markReadOnly(REGNUM_RAX);

	// We make a 128-byte skip (16*8) and push the flags register;
	// so the first register saved starts 17 slots down from the frame
	// pointer.

    if (gen.rs()->getSPFlag() && 0) {
		for (int reg = 0; reg < 16; reg++) {
	  		emitPushReg64(reg, gen);
          	gen.rs()->markSavedRegister(reg, 17-reg);
		}
    }
    else {
	
		//	printf("Saving registers ...\n");
		// Save the live ones
		int num_saved = 0; 
		for(u_int i = 0; i < gen.rs()->getRegisterCount(); i++) {
	    	registerSlot * reg = gen.rs()->getRegSlot(i);    
	    	if (reg->startsLive ||
				(reg->number == REGNUM_RAX) ||
				(reg->number == REGNUM_RBX) ||
				(reg->number == REGNUM_RSP) ||
				(reg->number == REGNUM_RBP)) {
				emitPushReg64(reg->number,gen);
				// We move the FP down to just under here, so we're actually
				// measuring _up_ from the FP. 
				assert((17-num_saved) > 0);
            	gen.rs()->markSavedRegister(reg->number, 17-num_saved);
				num_saved++;
	      	}
        	else {
            }
	}

	// we always allocate space on the stack for all the GPRs (helps stack walk)
	if (num_saved < 16) {
	    emitOpRegImm8_64(0x83, 0x0, REGNUM_RSP, -8 * (16 - num_saved), true, gen);
	}
	//printf("\n");
      }
    
    // push a return address for stack walking
    if (bt->instP()) {
	emitMovImmToReg64(REGNUM_RAX, bt->instP()->addr(), true, gen);
	emitPushReg64(REGNUM_RAX, gen);
    }
    else {
	assert(bt->rpcMgr_);
    }

    // set up a fresh stack frame
    // pushl %rbp        (0x55)
    // movl  %rsp, %rbp  (0x48 0x89 0xe5)
    emitSimpleInsn(0x55, gen);
    emitMovRegToReg64(REGNUM_RBP, REGNUM_RSP, true, gen);


    if (bt->isConservative() && BPatch::bpatch->isSaveFPROn()) {
       if (gen.rs()->getSPFlag())
       {
          // need to save the floating point state (x87, MMX, SSE)
          // we do this on the stack, but the problem is that the save
          // area must be 16-byte aligned. the following sequence does
          // the job:
          //   mov %rsp, %rax          ; copy the current stack pointer
          //   sub $512, %rsp          ; allocate space
          //   and $0xfffffff0, %rsp   ; make sure we're aligned (allocates some more space)
          //   fxsave (%rsp)           ; save the state
          //   push %rax               ; save the old stack pointer
          
          emitMovRegToReg64(REGNUM_RAX, REGNUM_RSP, true, gen);
          emitOpRegImm64(0x81, EXTENDED_0x81_SUB, REGNUM_RSP, 512, true, gen);
          emitOpRegImm64(0x81, EXTENDED_0x81_AND, REGNUM_RSP, -16, true, gen);
          
          // fxsave (%rsp) ; 0x0f 0xae 0x04 0x24
          REGET_PTR(buffer, gen);
          *buffer++ = 0x0f;
          *buffer++ = 0xae;
          *buffer++ = 0x04;
          *buffer++ = 0x24;
          SET_PTR(buffer, gen);
          
          emitPushReg64(REGNUM_RAX, gen);
       }
    }

    return true;
}

bool Emitter64::emitBTRestores(baseTramp* bt, codeGen &gen)
{
   if (bt->isConservative() && BPatch::bpatch->isSaveFPROn()) {
      if (gen.rs()->getSPFlag())
      {
         // pop the old RSP value into RAX
         emitPopReg64(REGNUM_RAX, gen);
         
         // restore saved FP state
         // fxrstor (%rsp) ; 0x0f 0xae 0x04 0x24
         GET_PTR(buffer, gen);
         *buffer++ = 0x0f;
         *buffer++ = 0xae;
         *buffer++ = 0x0c;
         *buffer++ = 0x24;
         SET_PTR(buffer, gen);
	  
         // restore stack pointer (deallocates FP save area)
         emitMovRegToReg64(REGNUM_RSP, REGNUM_RAX, true, gen);
      }
   }

   // tear down the stack frame (LEAVE)
   emitSimpleInsn(0xC9, gen);
   
   // pop "fake" return address
   if (!bt->rpcMgr_)
      emitPopReg64(REGNUM_RAX, gen);
   
   if (gen.rs()->getSPFlag() && 0)
   {
      // restore saved registers (POP R15, POP R14, ...)
      for (int reg = 15; reg >= 0; reg--) {
         emitPopReg64(reg, gen);
      }
   }
   else
   {
      // Count the saved registers
      int num_saved = 0; // RAX, RBX, RSP, RBP always saved
      for(int i = gen.rs()->getRegisterCount()-1; i >= 0; i--) {
         
         registerSlot * reg = gen.rs()->getRegSlot(i);
         if (reg->startsLive ||
             (reg->number == REGNUM_RAX) ||
             (reg->number == REGNUM_RBX) ||
             (reg->number == REGNUM_RSP) ||
             (reg->number == REGNUM_RBP)) {
		  		num_saved++;
	      }
      }
      
      // move SP up to end of GPR save area
      if (num_saved < 16) {
	      emitOpRegImm8_64(0x83, 0x0, REGNUM_RSP, 8 * (16 - num_saved), true, gen);
      }
      
      // restore saved registers
      for(int i = gen.rs()->getRegisterCount()-1; i >= 0; i--)
      {
         registerSlot * reg = gen.rs()->getRegSlot(i);
         
         if (reg->startsLive ||
             (reg->number == REGNUM_RAX) ||
             (reg->number == REGNUM_RBX) ||
             (reg->number == REGNUM_RSP) ||
             (reg->number == REGNUM_RBP)) {
            emitPopReg64(reg->number,gen);
	      }
      }
      
   }
   
   // restore flags (POPFQ)
   emitSimpleInsn(0x9D, gen);
   
   // restore stack pointer (use LEA to not affect flags)
   GET_PTR(buffer, gen);
   *buffer++ = 0x48; // REX.W
   *buffer++ = 0x8D; // LEA opcode
   *buffer++ = 0xA4; // ModRM: [SIB + disp32], %rsp
   *buffer++ = 0x24; // SIB: base = RSP
   *(unsigned int*)buffer = 128; // displacement: 128
   buffer += 4;
   SET_PTR(buffer, gen);
   
   return true;
}

bool Emitter64::emitBTMTCode(baseTramp* bt, codeGen& gen)
{
    AstNode *threadPOS;
    pdvector<AstNode *> dummy;
    Register src = Null_Register;
    // registers cleanup
    //gen.rs()->resetSpace();
    
    dyn_thread *thr = gen.thread();
    if (!bt->threaded() && !bt->guarded()) {
       return true;
    }
    else if (!bt->threaded()) {
        /* Get the hashed value of the thread */
        //emitVload(loadConstOp, 0, REG_MT_POS, REG_MT_POS, gen, false);
        emitMovImmToReg64(REGNUM_RAX, 0, true, gen);
    }
    else if (thr) {
        // Override 'normal' index value...
        //emitVload(loadConstOp, thr->get_index(), REG_MT_POS, REG_MT_POS, gen, false);
        emitMovImmToReg64(REGNUM_RAX, thr->get_index(), true, gen);
    }    
    else {
        Address unused;
        threadPOS = AstNode::funcCallNode("DYNINSTthreadIndex", dummy, bt->proc());
        src = threadPOS->generateCode(gen,
                                      false, // noCost 
                                      unused,
                                      src);
        // AST generation uses a base pointer and a current address; the lower-level
        // code uses a current pointer. Convert between the two.
    }

    // assert(index is in _RAX)

    emitMovRegToRM64(REGNUM_RBP, mt_offset, REGNUM_RAX, true, gen);

    return true;
}

bool Emitter64::emitBTGuardPreCode(baseTramp* bt, codeGen &gen, unsigned& guardJumpIndex)
{
    assert(bt->guarded());

    Register scratch1 = gen.rs()->getScratchRegister(gen);
    Register scratch2 = gen.rs()->getScratchRegister(gen);

    // The jump gets filled in later; allocate space for it now
    // and stick where it is in guardJumpOffset
    Address guard_flag_address = bt->proc()->trampGuardBase();
    if (!guard_flag_address) {
        return false;
    }
    
    if (bt->threaded()) 
    {
       inst_printf("Generating MT code\n");
       // Load the index into REGNUM_EAX
       emitMovRMToReg64(scratch1, REGNUM_RBP, mt_offset, true, gen);
       if (!is_disp32(guard_flag_address))
       {
          // We can't use a 64 bit value in the lea, so first store it in rbx
          emitMovImmToReg64(scratch2, guard_flag_address, true, gen);
          // Shift by sizeof(int) and add guard_flag_address
          emitLEA64(scratch2, scratch1, 2, 0x0, REGNUM_RAX, true, gen);
       }
       else
       {
          //Store tramp guard address in RAX
          emitLEA64(Null_Register, scratch1, 2, guard_flag_address, scratch1, true, gen);
       }
    }
    else {
        emitMovImmToReg64(scratch1, guard_flag_address, true, gen);
    }
    // Compare to 0
    emitOpMemImm64(0x81, 0x7, scratch1, 0, false, gen);
    
    guardJumpIndex = gen.getIndex();
    
    // We might have to 5-byte around if there's a lot of inlined minitramps
    // When we fix this, we might use a smaller jump (and leave the rest as noops)
    emitJcc(0x4, 0, gen, true);
    bt->guardBranchSize = gen.getIndex() - guardJumpIndex;
    
    emitMovImmToRM(scratch1, 0, 0, gen);
    
    return true;
}

bool Emitter64::emitBTGuardPostCode(baseTramp* bt, codeGen &gen, codeBufIndex_t &guardTargetIndex)
{
    assert(bt->guarded());
    Register scratch1 = gen.rs()->getScratchRegister(gen);
    Register scratch2 = gen.rs()->getScratchRegister(gen);

    Address guard_flag_address = bt->proc()->trampGuardBase();
    if (!guard_flag_address) {
        return false;
    }
    assert(guard_flag_address);

   if (bt->threaded())
   {
       inst_printf("Generating MT guard post code\n");
       // Load the index into REGNUM_RAX
       emitMovRMToReg64(scratch1, REGNUM_RBP, mt_offset, true, gen);
       //       LOAD_VIRTUAL64(REG_MT_POS, gen);
       // Shift by sizeof(int) and add guard_flag_address
       if (!is_disp32(guard_flag_address))
       {
          // We can't use a 64 bit value in the lea, so first store it in ebx
          emitMovImmToReg64(scratch2, guard_flag_address, true, gen);
          // Shift by sizeof(int) and add guard_flag_address
          emitLEA64(scratch2, scratch1, 2, 0x0, scratch1, true, gen);
       }
       else
       {
          //Store tramp guard address in RAX
          emitLEA64(Null_Register, scratch1, 2, guard_flag_address, scratch1, true, gen);
       }
   }
   else {
       emitMovImmToReg64(scratch1, guard_flag_address, true, gen);
   }
   emitMovImmToRM(scratch1, 0, 1, gen);
   guardTargetIndex = gen.getIndex();

   return true;
}

bool Emitter64::emitBTCostCode(baseTramp* bt, codeGen &gen, unsigned& costUpdateOffset)
{
   return false;
    Address costAddr = bt->proc()->getObservedCostAddr();
    if (!costAddr) return false;
    costUpdateOffset = gen.used();    

    // FIXME: we unconditionally use RAX here since the cost is written in later
    emitMovImmToReg64(REGNUM_RAX, costAddr, true, gen);
    
    // this will be overwritten with: add $cost, (%rax) ;[0x81 0x00 <cost>] - 6 bytes
    instruction::generateNOOP(gen, 6);

    return true;
}

void Emitter64::emitStoreImm(Address addr, int imm, codeGen &gen, bool noCost) 
{
   if (!isImm64bit(addr)) {
      emitMovImmToRM(Null_Register, addr, imm, gen);
   }
   else {
      Register r = gen.rs()->allocateRegister(gen, noCost);      
      emitMovImmToReg64(r, addr, true, gen);
      emitMovImmToRM64(r, 0, imm, true, gen);
      gen.rs()->freeRegister(r);
   }
}

void Emitter64::emitAddSignedImm(Address addr,int imm, codeGen &gen,bool noCost)
{
   if (!isImm64bit(addr)) {
      emitAddMem(addr, imm, gen);
   }
   else {
      Register r = gen.rs()->allocateRegister(gen, noCost);      
      emitMovImmToReg64(r, addr, true, gen);
      emitAddRM64(r, imm, true, gen);
      gen.rs()->freeRegister(r);
   }
}

// on 64-bit x86_64 targets, the DWARF register number does not
// correspond to the machine encoding. See the AMD-64 ABI.
#define REG_CNT 53
/* I stole this wholesale from gcc's gcc/config/i386/i386.c -- nater */
static int const amd64_register_map[] =
{ 
  0, 2, 1, 3, 6, 7, 5, 4,       /* general regs */
  11, 12, 13, 14, 15, 16, 17, 18,   /* fp regs */
  -1, 9, -1, -1, -1,            /* arg, flags, fpsr, dir, frame */
  21, 22, 23, 24, 25, 26, 27, 28,   /* SSE registers */
  29, 30, 31, 32, 33, 34, 35, 36,   /* MMX registers */
  -1, -1, -1, -1, -1, -1, -1, -1,   /* extended integer registers */
  -1, -1, -1, -1, -1, -1, -1, -1,   /* extended SSE registers */
};
int Emitter64::Register_DWARFtoMachineEnc(int n)
{
    return amd64_register_map[n];

}

bool Emitter64::emitPush(codeGen &gen, Register reg) {
    emitPushReg64(reg, gen);
    return true;
}
   
bool Emitter64::emitPop(codeGen &gen, Register reg) {
    emitPopReg64(reg, gen);
    return true;
}

bool Emitter64::emitAdjustStackPointer(int index, codeGen &gen) {
	// The index will be positive for "needs popped" and negative
	// for "needs pushed". However, positive + SP works, so don't
	// invert.
	int popVal = index * gen.proc()->getAddressWidth();
	emitOpRegImm64(0x81, EXTENDED_0x81_ADD, REGNUM_ESP, popVal, true, gen);
	return true;
}

Emitter64 emitter64;

// change code generator to 32-bit mode
void emit32()
{
    code_emitter = &emitter32;

	registerSpace::conservativeRegSpace_ = conservativeRegSpace32;
	registerSpace::optimisticRegSpace_ = optimisticRegSpace32;
	registerSpace::actualRegSpace_ = actualRegSpace32;		
	registerSpace::savedRegSpace_ = savedRegSpace32;
}

// change code generator to 64-bit mode
void emit64()
{
    code_emitter = &emitter64;

	registerSpace::conservativeRegSpace_ = conservativeRegSpace64;
	registerSpace::optimisticRegSpace_ = optimisticRegSpace64;
	registerSpace::actualRegSpace_ = actualRegSpace64;		
	registerSpace::savedRegSpace_ = savedRegSpace64;
}

registerSpace *globalRegSpace32 = NULL;
registerSpace *conservativeRegSpace32 = NULL;
registerSpace *optimisticRegSpace32 = NULL;
registerSpace *actualRegSpace32 = NULL;
registerSpace *savedRegSpace32 = NULL;
registerSpace *globalRegSpace64 = NULL;
registerSpace *conservativeRegSpace64 = NULL;
registerSpace *optimisticRegSpace64 = NULL;
registerSpace *actualRegSpace64 = NULL;
registerSpace *savedRegSpace64 = NULL;


#endif

// emitter defaults to 32-bit
Emitter* code_emitter = &emitter32;
