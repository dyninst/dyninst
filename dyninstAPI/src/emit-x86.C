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
 * $Id: emit-x86.C,v 1.7 2005/09/09 19:19:48 gquinn Exp $
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

extern registerSpace* regSpace;

codeBufIndex_t Emitter32::emitIf(Register expr_reg, Register target, codeGen &gen)
{
    // sub REGNUM_EAX, REGNUM_EAX
    emitOpRegReg(0x29, REGNUM_EAX, REGNUM_EAX, gen);

    // cmp -(expr*4)[REGNUM_EBP], REGNUM_EAX
    emitOpRegRM(0x3B, REGNUM_EAX, REGNUM_EBP, -(expr_reg*4), gen);

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
      emitMovRMToReg(REGNUM_EAX, REGNUM_EBP, -(src1*4), gen);
      emitOpRegRM(opcode, REGNUM_EAX, REGNUM_EBP, -(src2*4), gen);
      emitMovRegToRM(REGNUM_EBP, -(dest*4), REGNUM_EAX, gen);
}

void Emitter32::emitRelOp(unsigned op, Register dest, Register src1, Register src2, codeGen &gen)
{
    emitOpRegReg(0x29, REGNUM_ECX, REGNUM_ECX, gen);           // clear REGNUM_ECX
    emitMovRMToReg(REGNUM_EAX, REGNUM_EBP, -(src1*4), gen);    // mov eax, -(src1*4)[ebp]
    emitOpRegRM(0x3B, REGNUM_EAX, REGNUM_EBP, -(src2*4), gen); // cmp eax, -(src2*4)[ebp]
    unsigned char opcode = jccOpcodeFromRelOp(op);
    GET_PTR(insn, gen);
    *insn++ = opcode; *insn++ = 1;                // jcc 1
    SET_PTR(insn, gen);
    emitSimpleInsn(0x40+REGNUM_ECX, gen);               // inc REGNUM_ECX
    emitMovRegToRM(REGNUM_EBP, -(dest*4), REGNUM_ECX, gen);    // mov -(dest*4)[ebp], ecx
}

void Emitter32::emitDiv(Register dest, Register src1, Register src2, codeGen &gen)
{
    // mov eax, src1
    // cdq   ; edx = sign extend of eax
    // idiv eax, src2 ; eax = edx:eax div src2, edx = edx:eax mod src2
    // mov dest, eax
    emitMovRMToReg(REGNUM_EAX, REGNUM_EBP, -(src1*4), gen);
    emitSimpleInsn(0x99, gen);
    emitOpRegRM(0xF7, 0x7 /*opcode extension*/, REGNUM_EBP, -(src2*4), gen);
    emitMovRegToRM(REGNUM_EBP, -(dest*4), REGNUM_EAX, gen);
}

void Emitter32::emitOpImm(unsigned opcode1, unsigned opcode2, Register dest, Register src1, RegValue src2imm,
			  codeGen &gen)
{
    if (src1 != dest) {
	emitMovRMToReg(REGNUM_EAX, REGNUM_EBP, -(src1*4), gen);
	emitMovRegToRM(REGNUM_EBP, -(dest*4), REGNUM_EAX, gen);
    }
    emitOpRMImm(opcode1, opcode2, REGNUM_EBP, -(dest*4), src2imm, gen);
}

void Emitter32::emitRelOpImm(unsigned op, Register dest, Register src1, RegValue src2imm, codeGen &gen)
{
   emitOpRegReg(0x29, REGNUM_ECX, REGNUM_ECX, gen);           // clear REGNUM_ECX
   emitMovRMToReg(REGNUM_EAX, REGNUM_EBP, -(src1*4), gen);    // mov eax, -(src1*4)[ebp]
   emitOpRegImm(0x3D, REGNUM_EAX, src2imm, gen);       // cmp eax, src2
   unsigned char opcode = jccOpcodeFromRelOp(op);
   GET_PTR(insn, gen);
   *insn++ = opcode; *insn++ = 1;                // jcc 1
   SET_PTR(insn, gen);
   emitSimpleInsn(0x40+REGNUM_ECX, gen);               // inc REGNUM_ECX
   emitMovRegToRM(REGNUM_EBP, -(dest*4), REGNUM_ECX, gen);    // mov -(dest*4)[ebp], ecx   
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
	    emitMovRMToReg(REGNUM_EAX, REGNUM_EBP, -(src1*4), gen);
	    emitMovRegToRM(REGNUM_EBP, -(dest*4), REGNUM_EAX, gen);
	}
	// sal dest, result
	emitOpRMImm8(0xC1, 4, REGNUM_EBP, -(dest*4), result, gen);
    }
    else {
	// imul REGNUM_EAX, -(src1*4)[ebp], src2imm
	emitOpRegRMImm(0x69, REGNUM_EAX, REGNUM_EBP, -(src1*4), src2imm, gen);
	emitMovRegToRM(REGNUM_EBP, -(dest*4), REGNUM_EAX, gen);
    } 
}

void Emitter32::emitDivImm(Register dest, Register src1, RegValue src2imm, codeGen &gen)
{
    int result = -1;
    if (isPowerOf2(src2imm, result) && result <= MAX_IMM8) {
	if (src1 != dest) {
	    emitMovRMToReg(REGNUM_EAX, REGNUM_EBP, -(src1*4), gen);
	    emitMovRegToRM(REGNUM_EBP, -(dest*4), REGNUM_EAX, gen);
	}
	// sar dest, result
	emitOpRMImm8(0xC1, 7, REGNUM_EBP, -(dest*4), result, gen);
    }
    else {
	// dest = src1 div src2imm
	// mov eax, src1
	// cdq   ; edx = sign extend of eax
	// mov ebx, src2imm
	// idiv eax, ebx ; eax = edx:eax div src2, edx = edx:eax mod src2
	// mov dest, eax
	emitMovRMToReg(REGNUM_EAX, REGNUM_EBP, -(src1*4), gen);
	emitSimpleInsn(0x99, gen);
	emitMovImmToReg(REGNUM_EBX, src2imm, gen);
	// idiv eax, ebx
	emitOpRegReg(0xF7, 0x7 /*opcode extension*/, REGNUM_EBX, gen); 
	emitMovRegToRM(REGNUM_EBP, -(dest*4), REGNUM_EAX, gen);
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
    emitMovRegToRM(REGNUM_EBP, -(dest*4), REGNUM_EAX, gen);    // mov -(dest*4)[ebp], eax
}

void Emitter32::emitLoadConst(Register dest, Address imm, codeGen &gen)
{
    emitMovImmToRM(REGNUM_EBP, -(dest*4), imm, gen);
}

void Emitter32::emitLoadIndir(Register dest, Register addr_reg, codeGen &gen)
{
    emitMovRMToReg(REGNUM_EAX, REGNUM_EBP, -(addr_reg*4), gen); // mov eax, -(addr_reg*4)[ebp]
    emitMovRMToReg(REGNUM_EAX, REGNUM_EAX, 0, gen);         // mov eax, [eax]
    emitMovRegToRM(REGNUM_EBP, -(dest*4), REGNUM_EAX, gen); // mov -(dest*4)[ebp], eax
}

void Emitter32::emitLoadFrameRelative(Register dest, Address offset, codeGen &gen)
{
    // eax = [ebp]	- saved bp
    // dest = [eax](offset)
    emitMovRMToReg(REGNUM_EAX, REGNUM_EBP, 0, gen);       // mov (%ebp), %eax 
    emitMovRMToReg(REGNUM_EAX, REGNUM_EAX, offset, gen);    // mov <offset>(%eax), %eax 
    emitMovRegToRM(REGNUM_EBP, -(dest*4), REGNUM_EAX, gen);    // mov -(dest*4)[ebp], eax
}

void Emitter32::emitLoadFrameAddr(Register dest, Address offset, codeGen &gen)
{
    emitMovRMToReg(REGNUM_EAX, REGNUM_EBP, 0, gen);       // mov (%ebp), %eax 
    emitAddRegImm32(REGNUM_EAX, offset, gen);        // add #<offset>, %eax
    emitMovRegToRM(REGNUM_EBP, -(dest*4), REGNUM_EAX, gen);    // mov -(dest*4)[ebp], eax
}

void Emitter32::emitLoadPreviousStackFrameRegister(Address register_num, Register dest, codeGen &gen)
{
    //Previous stack frame register is stored on the stack,
    //it was stored there at the begining of the base tramp.
    
    //Calculate the register's offset from the frame pointer in REGNUM_EBP
    unsigned offset = SAVED_EAX_OFFSET - (register_num * 4);

    emitMovRMToReg(REGNUM_EAX, REGNUM_EBP, offset, gen); //mov eax, offset[ebp]
    emitMovRegToRM(REGNUM_EBP, -(dest*4), REGNUM_EAX, gen); //mov dest, 0[eax]
}

void Emitter32::emitStore(Address addr, Register src, codeGen &gen)
{
      emitMovRMToReg(REGNUM_EAX, REGNUM_EBP, -(src*4), gen);    // mov eax, -(src*4)[ebp]
      emitMovRegToM(addr, REGNUM_EAX, gen);               // mov addr, eax
}

void Emitter32::emitStoreIndir(Register addr_reg, Register src, codeGen &gen)
{
    emitMovRMToReg(REGNUM_EAX, REGNUM_EBP, -(src*4), gen);   // mov eax, -(src*4)[ebp]
    emitMovRMToReg(REGNUM_ECX, REGNUM_EBP, -(addr_reg*4), gen);   // mov ecx, -(addr_reg*4)[ebp]
    emitMovRegToRM(REGNUM_ECX, 0, REGNUM_EAX, gen);           // mov [ecx], eax
}

void Emitter32::emitStoreFrameRelative(Address offset, Register src, Register scratch, codeGen &gen)
{
      // scratch = [ebp]	- saved bp
      // (offset)[scratch] = src
      emitMovRMToReg(scratch, REGNUM_EBP, 0, gen);    	    // mov scratch, (ebp)
      emitMovRMToReg(REGNUM_EAX, REGNUM_EBP, -(src*4), gen);    // mov eax, -(src*4)[ebp]
      emitMovRegToRM(scratch, offset, REGNUM_EAX, gen);        // mov (offset)[scratch], eax
}

void Emitter32::emitGetRetVal(Register dest, codeGen &gen)
{
    emitMovRMToReg(REGNUM_EAX, REGNUM_EBP, SAVED_EAX_OFFSET, gen);
    emitMovRegToRM(REGNUM_EBP, -(dest*4), REGNUM_EAX, gen);
}

void Emitter32::emitGetParam(Register dest, Register param_num, instPointType_t pt_type, codeGen &gen)
{
    // Parameters are addressed by a positive offset from ebp,
    // the first is PARAM_OFFSET[ebp]
    if(pt_type == callSite) {
	emitMovRMToReg(REGNUM_EAX, REGNUM_EBP, CALLSITE_PARAM_OFFSET + param_num*4, gen);
	emitMovRegToRM(REGNUM_EBP, -(dest*4), REGNUM_EAX, gen);
    } else {
	// assert(pt_type == functionEntry)
	emitMovRMToReg(REGNUM_EAX, REGNUM_EBP, FUNC_PARAM_OFFSET + param_num*4, gen);
	emitMovRegToRM(REGNUM_EBP, -(dest*4), REGNUM_EAX, gen);
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
    if (bt->preInstP)
        emitPushImm(bt->preInstP->addr(), gen);
    else if (bt->postInstP) {
        emitPushImm(bt->postInstP->addr(), gen);
    }
    else {
        assert(bt->rpcMgr_);
    }

    emitSimpleInsn(PUSH_EBP, gen);
    emitMovRegToReg(REGNUM_EBP, REGNUM_ESP, gen);
    
    if (bt->isConservative()) {
        // Allocate space for temporaries and floating points
        emitOpRegImm(5, REGNUM_ESP, TRAMP_FRAME_SIZE+FSAVE_STATE_SIZE, gen);
        emitOpRegRM(FSAVE, FSAVE_OP, REGNUM_EBP, -(TRAMP_FRAME_SIZE) - FSAVE_STATE_SIZE, gen);
    } else {
        // Allocate space for temporaries
        emitOpRegImm(5, REGNUM_ESP, TRAMP_FRAME_SIZE, gen);
    }
    return true;
}

bool Emitter32::emitBTRestores(baseTramp* bt, codeGen &gen)
{
    if (bt->isConservative()) {
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
    regSpace->resetSpace();
    
    /* Get the hashed value of the thread */
    if (bt->threaded())
       threadPOS = new AstNode("DYNINSTthreadIndex", dummy);
    else
       threadPOS = new AstNode("DYNINSTreturnZero", dummy);
    
    src = threadPOS->generateCode(bt->proc(), regSpace, gen,
                                  false, // noCost 
                                  true); // root node
    
    
    // AST generation uses a base pointer and a current address; the lower-level
    // code uses a current pointer. Convert between the two.
    LOAD_VIRTUAL(src, gen);
    SAVE_VIRTUAL(REG_MT_POS, gen);
    
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
    inst_printf("guard_flag_addr 0x%x\n", guard_flag_address);
    if (bt->threaded()) 
    {
       inst_printf("Generating MT code\n");
       // Load the index into REGNUM_EAX
       LOAD_VIRTUAL(REG_MT_POS, gen);
       // Shift by sizeof(int) and add guard_flag_address
       emitLEA(Null_Register, REGNUM_EAX, 2, guard_flag_address, REGNUM_EAX, gen);
       // Compare to 0
       emitOpRMImm8(0x83, 0x07, REGNUM_EAX, 0, 0, gen);
    }
    else {
        inst_printf("Emitting normal code\n");
        emitOpRMImm8(0x83, 0x07, Null_Register, guard_flag_address, 0, gen);
    }
    guardJumpIndex = gen.getIndex();

    // We might have to 5-byte around if there's a lot of inlined minitramps
    // When we fix this, we might use a smaller jump (and leave the rest as noops)
    bt->guardBranchSize = JUMP_REL32_SZ;
    instruction::generateNOOP(gen, JUMP_REL32_SZ);
    
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
       inst_printf("Generating MT guard post code\n");
       // Load the index into REGNUM_EAX
       LOAD_VIRTUAL(REG_MT_POS, gen);
       // Shift by sizeof(int) and add guard_flag_address
       emitLEA(Null_Register, REGNUM_EAX, 2, guard_flag_address, REGNUM_EAX, gen);
       emitMovImmToRM(REGNUM_EAX, 0, 1, gen);
   }
   else {
       inst_printf("Generating non-MT guard post code\n");
       emitMovImmToMem( guard_flag_address, 1, gen );
   }
   guardTargetIndex = gen.getIndex();

   return true;
}

bool Emitter32::emitBTCostCode(baseTramp* bt, codeGen &gen, unsigned& costUpdateOffset)
{
    Address costAddr = bt->proc()->getObservedCostAddr();
    inst_printf("costAddr is 0x%x\n", costAddr);
    if (!costAddr) return false;

    costUpdateOffset = gen.used();
    // Dummy for now; we update at generation time.
    emitAddMemImm32(costAddr, 0, gen); 
    return true;
}

Emitter32 emitter32;

#if defined(arch_x86_64)

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

void emitMovRegToReg64(Register dest, Register src, bool is_64, codeGen &gen)
{
	Register tmp_dest = dest;
	Register tmp_src = src;
	emitRex(is_64, &tmp_dest, NULL, &tmp_src, gen);
	emitMovRegToReg(tmp_dest, tmp_src, gen);
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

static void emitPushReg64(Register src, codeGen &gen)
{
    emitRex(false, NULL, NULL, &src, gen);
    emitSimpleInsn(0x50 + src, gen);
}

static void emitPopReg64(Register dest, codeGen &gen)
{
    emitRex(false, NULL, NULL, &dest, gen);    
    emitSimpleInsn(0x58 + dest, gen);
}

class Emitter64 : public Emitter {

public:
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

codeBufIndex_t Emitter64::emitIf(Register expr_reg, Register target, codeGen &gen)
{
    // sub RAX, RAX
    emitOpRegReg64(0x29, REGNUM_RAX, REGNUM_RAX, true, gen);

    // cmp %expr_reg, RAX
    emitOpRegReg64(0x3B, REGNUM_RAX, expr_reg, true, gen);

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
    // push RDX if it's in use, since we will need it
    bool save_rdx = false;
    if (!regSpace->isFreeRegister(REGNUM_RDX)) {
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
	if (!regSpace->isFreeRegister(REGNUM_RDX)) {
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
	
	// mov $addr, %rax
	emitMovImmToReg64(REGNUM_RAX, addr, true, gen);
	
	// mov (%rax), %dest
	emitMovRMToReg64(dest, REGNUM_RAX, 0, size == 8, gen);
    }
}

void Emitter64::emitLoadConst(Register dest, Address imm, codeGen &gen)
{
    emitMovImmToReg64(dest, imm, true, gen);
}

void Emitter64::emitLoadIndir(Register dest, Register addr_src, codeGen &gen)
{
    // FIXME: we assume int (size == 4) for now
    emitMovRMToReg64(dest, addr_src, 0, false, gen);
}

void Emitter64::emitLoadFrameRelative(Register dest, Address offset, codeGen &gen)
{
    // FIXME: we assume int (size == 4) for now

    // mov (%rbp), %rax
    emitMovRMToReg64(REGNUM_RAX, REGNUM_RBP, 0, true, gen);

    // mov offset(%rax), %dest
    emitMovRMToReg64(dest, REGNUM_RAX, offset, false, gen);
}

void Emitter64::emitLoadFrameAddr(Register dest, Address offset, codeGen &gen)
{
    // mov (%rbp), %dest
    emitMovRMToReg64(dest, REGNUM_RBP, 0, true, gen);

    // add $offset, %dest
    emitOpRegImm64(0x81, 0x0, dest, offset, true, gen);
}

#define SAVED_RAX_OFFSET 17

void Emitter64::emitLoadPreviousStackFrameRegister(Address register_num, Register dest, codeGen &gen)
{
    emitMovRMToReg64(dest, REGNUM_RBP, (SAVED_RAX_OFFSET - register_num) * 8, true, gen);
}

void Emitter64::emitStore(Address addr, Register src, codeGen &gen)
{
    // FIXME: we assume int (size == 4) for now

    // mov $addr, %rax
    emitMovImmToReg64(REGNUM_RAX, addr, true, gen);

    // mov %src, (%rax)
    emitMovRegToRM64(REGNUM_RAX, 0, src, false, gen);
}

void Emitter64::emitStoreIndir(Register addr_reg, Register src, codeGen &gen)
{
    // FIXME: we assume int (size == 4) for now
    emitMovRegToRM64(addr_reg, 0, src, false, gen);
}

void Emitter64::emitStoreFrameRelative(Address offset, Register src, Register /*scratch*/, codeGen &gen)
{
    // FIXME: we assume int (size == 4) for now

    // mov (%rbp), %rax
    emitMovRMToReg64(REGNUM_RAX, REGNUM_RBP, 0, true, gen);
    
    // mov %src, offset(%rax)
    emitMovRegToRM64(REGNUM_RAX, offset, src, false, gen);
}

static Register amd64_arg_regs[] = {REGNUM_RDI, REGNUM_RSI, REGNUM_RDX, REGNUM_RCX, REGNUM_R8, REGNUM_R9};
#define AMD64_ARG_REGS (sizeof(amd64_arg_regs) / sizeof(Register))
Register Emitter64::emitCall(opCode op, registerSpace *rs, codeGen &gen, const pdvector<AstNode *> &operands,
			     process *proc, bool noCost, Address callee_addr, const pdvector<AstNode *> &ifForks,
			     const instPoint *location)
{
    assert(op == callOp);
    pdvector <Register> srcs;
    
    //  Sanity check for NULL address arg
    if (!callee_addr) {
	char msg[256];
	sprintf(msg, "%s[%d]:  internal error:  emitFuncCall called w/out"
		"callee_addr argument", __FILE__, __LINE__);
	showErrorCallback(80, msg);
	assert(0);
    }

    // generate code for arguments
    for (unsigned u = 0; u < operands.size(); u++)
	srcs.push_back((Register)operands[u]->generateCode_phase2(proc, rs, gen,
								  noCost, 
								  ifForks, 
								  location));
    
    // here we save the argument registers we're going to use
    // then we use the stack to shuffle everything into the right place
    // FIXME: optimize this a bit - this initial implementation is very conservative
    const unsigned num_args = srcs.size();
    assert(num_args <= AMD64_ARG_REGS);
    for (unsigned u = 0; u < num_args; u++)
	emitPushReg64(amd64_arg_regs[u], gen);
    for (unsigned u = 0; u < num_args; u++)
	emitPushReg64(srcs[u], gen);
    for (int i = num_args - 1; i >= 0; i--)
	emitPopReg64(amd64_arg_regs[i], gen);

    // make the call (using an indirect call)
    emitMovImmToReg64(REGNUM_EAX, callee_addr, true, gen);
    emitSimpleInsn(0xff, gen); // group 5
    emitSimpleInsn(0xd0, gen); // mod = 11, reg = 2 (call Ev), r/m = 0 (RAX)
    
    // restore argument registers
    for (int i = num_args - 1; i >= 0; i--)
	emitPopReg64(amd64_arg_regs[i], gen);   
    
    // allocate a (virtual) register to store the return value
    Register ret = rs->allocateRegister(gen, noCost);
    emitMovRegToReg64(ret, REGNUM_EAX, true, gen);
    
    return ret;
}

// FIXME: comment here on the stack layout
void Emitter64::emitGetRetVal(Register dest, codeGen &gen)
{
    emitMovRMToReg64(dest, REGNUM_RBP, SAVED_RAX_OFFSET * 8, true, gen);
}

void Emitter64::emitGetParam(Register dest, Register param_num, instPointType_t /*pt_type*/, codeGen &gen)
{
    assert(param_num <= 6);
    emitMovRMToReg64(dest, REGNUM_RBP, (SAVED_RAX_OFFSET - amd64_arg_regs[param_num]) * 8, true, gen);
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

    // restore saved registers (POP R15, POP R14, ...)
    for (int reg = 15; reg >= 0; reg--) {
	//if (reg == REGNUM_RSP) continue;
	emitPopReg64(reg, gen);
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
    
    // save all registers (PUSH RAX, PUSH RCX, ...)
    for (int reg = 0; reg < 16; reg++) {
	//if (reg == REGNUM_RSP) continue;
	emitPushReg64(reg, gen);
    }

    // push a return address for stack walking
    if (bt->preInstP) {
	emitMovImmToReg64(REGNUM_RAX, bt->preInstP->addr(), true, gen);
	emitPushReg64(REGNUM_RAX, gen);
    }
    else if (bt->postInstP) {
	emitMovImmToReg64(REGNUM_RAX, bt->postInstP->addr(), true, gen);
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

    if (bt->isConservative()) {
	
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
	emitOpRegImm64(0x81, 5, REGNUM_RSP, 512, true, gen);
	emitOpRegImm64(0x81, 4, REGNUM_RSP, -16, true, gen);

	// fxsave (%rsp) ; 0x0f 0xae 0x04 0x24
	REGET_PTR(buffer, gen);
	*buffer++ = 0x0f;
	*buffer++ = 0xae;
	*buffer++ = 0x04;
	*buffer++ = 0x24;
	SET_PTR(buffer, gen);

	emitPushReg64(REGNUM_RAX, gen);
    }

    return true;
}

bool Emitter64::emitBTRestores(baseTramp* bt, codeGen &gen)
{
    if (bt->isConservative()) {

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

    // tear down the stack frame (LEAVE)
    emitSimpleInsn(0xC9, gen);

    // pop "fake" return address
    if (!bt->rpcMgr_)
	emitPopReg64(REGNUM_RAX, gen);

    // restore saved registers (POP R15, POP R14, ...)
    for (int reg = 15; reg >= 0; reg--) {
	//if (reg == REGNUM_RSP) continue;
	emitPopReg64(reg, gen);
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

bool Emitter64::emitBTMTCode(baseTramp* /*bt*/, codeGen& /*gen*/)
{
    return true;
}

bool Emitter64::emitBTGuardPreCode(baseTramp* bt, codeGen &gen, unsigned& guardJumpIndex)
{
    assert(bt->guarded());

    // The jump gets filled in later; allocate space for it now
    // and stick where it is in guardJumpOffset
    Address guard_flag_address = bt->proc()->trampGuardBase();
    if (!guard_flag_address) {
        return false;
    }
    
    inst_printf("guard_flag_addr 0x%p\n", (void*)guard_flag_address);
    if (bt->threaded()) 
    {
       assert(0);
    }
    else {
        inst_printf("Emitting normal code\n");
        emitMovImmToReg64(REGNUM_RAX, guard_flag_address, true, gen);
        emitOpMemImm64(0x81, 0x7, REGNUM_RAX, 0, false, gen);
    }
    
    guardJumpIndex = gen.getIndex();
    
    // We might have to 5-byte around if there's a lot of inlined minitramps
    // When we fix this, we might use a smaller jump (and leave the rest as noops)
    bt->guardBranchSize = JUMP_REL32_SZ;
    instruction::generateNOOP(gen, JUMP_REL32_SZ);
    
    if (bt->threaded()) 
    {
	    assert(0);
    }
    else {
	emitMovImmToRM(REGNUM_RAX, 0, 0, gen);
    }
    
    return true;
}

bool Emitter64::emitBTGuardPostCode(baseTramp* bt, codeGen &gen, codeBufIndex_t &guardTargetIndex)
{
    assert(bt->guarded());

    Address guard_flag_address = bt->proc()->trampGuardBase();
    if (!guard_flag_address) {
        return false;
    }
    assert(guard_flag_address);

   if (bt->threaded())
   {
       assert(0);
   }
   else {
       inst_printf("Generating non-MT guard post code\n");
       emitMovImmToReg64(REGNUM_RAX, guard_flag_address, true, gen);
       emitMovImmToRM(REGNUM_EAX, 0, 1, gen);
   }
   guardTargetIndex = gen.getIndex();

   return true;
}

bool Emitter64::emitBTCostCode(baseTramp* bt, codeGen &gen, unsigned& costUpdateOffset)
{
    Address costAddr = bt->proc()->getObservedCostAddr();
    inst_printf("costAddr is 0x%x\n", costAddr);
    if (!costAddr) return false;
    costUpdateOffset = gen.used();    

    emitMovImmToReg64(REGNUM_RAX, costAddr, true, gen);
    
    // this will be overwritten with: add $cost, (%rax) ;[0x81 0x00 <cost>] - 6 bytes
    instruction::generateNOOP(gen, 6);

    return true;
}

Emitter64 emitter64;

// change code generator to 32-bit mode
void emit32()
{
    x86_emitter = &emitter32;
    regSpace = regSpace32;
}

// change code generator to 64-bit mode
void emit64()
{
    x86_emitter = &emitter64;
    regSpace = regSpace64;
}

#endif

Emitter* x86_emitter = &emitter32;
