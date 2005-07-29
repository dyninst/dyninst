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
 * $Id: emit-x86.C,v 1.3 2005/07/29 19:18:28 bernat Exp $
 */

#include <assert.h>
#include <stdio.h>
#include "common/h/Types.h"
#include "dyninstAPI/src/arch.h"
#include "dyninstAPI/src/emit-x86.h"
#include "dyninstAPI/src/inst-x86.h"

codeBufIndex_t Emitter32::emitIf(Register expr_reg, Register target, codeGen &gen)
{
    // sub EAX, EAX
    emitOpRegReg(0x29, EAX, EAX, gen);

    // cmp -(expr*4)[EBP], EAX
    emitOpRegRM(0x3B, EAX, EBP, -(expr_reg*4), gen);

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
      emitMovRMToReg(EAX, EBP, -(src1*4), gen);
      emitOpRegRM(opcode, EAX, EBP, -(src2*4), gen);
      emitMovRegToRM(EBP, -(dest*4), EAX, gen);
}

void Emitter32::emitRelOp(unsigned op, Register dest, Register src1, Register src2, codeGen &gen)
{
    emitOpRegReg(0x29, ECX, ECX, gen);           // clear ECX
    emitMovRMToReg(EAX, EBP, -(src1*4), gen);    // mov eax, -(src1*4)[ebp]
    emitOpRegRM(0x3B, EAX, EBP, -(src2*4), gen); // cmp eax, -(src2*4)[ebp]
    unsigned char opcode = jccOpcodeFromRelOp(op);
    GET_PTR(insn, gen);
    *insn++ = opcode; *insn++ = 1;                // jcc 1
    SET_PTR(insn, gen);
    emitSimpleInsn(0x40+ECX, gen);               // inc ECX
    emitMovRegToRM(EBP, -(dest*4), ECX, gen);    // mov -(dest*4)[ebp], ecx
}

void Emitter32::emitDiv(Register dest, Register src1, Register src2, codeGen &gen)
{
    // mov eax, src1
    // cdq   ; edx = sign extend of eax
    // idiv eax, src2 ; eax = edx:eax div src2, edx = edx:eax mod src2
    // mov dest, eax
    emitMovRMToReg(EAX, EBP, -(src1*4), gen);
    emitSimpleInsn(0x99, gen);
    emitOpRegRM(0xF7, 0x7 /*opcode extension*/, EBP, -(src2*4), gen);
    emitMovRegToRM(EBP, -(dest*4), EAX, gen);
}

void Emitter32::emitOpImm(unsigned opcode1, unsigned opcode2, Register dest, Register src1, RegValue src2imm,
			  codeGen &gen)
{
    if (src1 != dest) {
	emitMovRMToReg(EAX, EBP, -(src1*4), gen);
	emitMovRegToRM(EBP, -(dest*4), EAX, gen);
    }
    emitOpRMImm(opcode1, opcode2, EBP, -(dest*4), src2imm, gen);
}

void Emitter32::emitRelOpImm(unsigned op, Register dest, Register src1, RegValue src2imm, codeGen &gen)
{
   emitOpRegReg(0x29, ECX, ECX, gen);           // clear ECX
   emitMovRMToReg(EAX, EBP, -(src1*4), gen);    // mov eax, -(src1*4)[ebp]
   emitOpRegImm(0x3D, EAX, src2imm, gen);       // cmp eax, src2
   unsigned char opcode = jccOpcodeFromRelOp(op);
   GET_PTR(insn, gen);
   *insn++ = opcode; *insn++ = 1;                // jcc 1
   SET_PTR(insn, gen);
   emitSimpleInsn(0x40+ECX, gen);               // inc ECX
   emitMovRegToRM(EBP, -(dest*4), ECX, gen);    // mov -(dest*4)[ebp], ecx   
}

// where is this defined?
extern bool isPowerOf2(int value, int &result);

void Emitter32::emitTimesImm(Register dest, Register src1, RegValue src2imm, codeGen &gen)
{
    int result = -1;

    if (isPowerOf2(src2imm, result) && result <= MAX_IMM8) {
	if (src1 != dest) {
	    emitMovRMToReg(EAX, EBP, -(src1*4), gen);
	    emitMovRegToRM(EBP, -(dest*4), EAX, gen);
	}
	// sal dest, result
	emitOpRMImm8(0xC1, 4, EBP, -(dest*4), result, gen);
    }
    else {
	// imul EAX, -(src1*4)[ebp], src2imm
	emitOpRegRMImm(0x69, EAX, EBP, -(src1*4), src2imm, gen);
	emitMovRegToRM(EBP, -(dest*4), EAX, gen);
    } 
}

void Emitter32::emitDivImm(Register dest, Register src1, RegValue src2imm, codeGen &gen)
{
    int result = -1;
    if (isPowerOf2(src2imm, result) && result <= MAX_IMM8) {
	if (src1 != dest) {
	    emitMovRMToReg(EAX, EBP, -(src1*4), gen);
	    emitMovRegToRM(EBP, -(dest*4), EAX, gen);
	}
	// sar dest, result
	emitOpRMImm8(0xC1, 7, EBP, -(dest*4), result, gen);
    }
    else {
	// dest = src1 div src2imm
	// mov eax, src1
	// cdq   ; edx = sign extend of eax
	// mov ebx, src2imm
	// idiv eax, ebx ; eax = edx:eax div src2, edx = edx:eax mod src2
	// mov dest, eax
	emitMovRMToReg(EAX, EBP, -(src1*4), gen);
	emitSimpleInsn(0x99, gen);
	emitMovImmToReg(EBX, src2imm, gen);
	// idiv eax, ebx
	emitOpRegReg(0xF7, 0x7 /*opcode extension*/, EBX, gen); 
	emitMovRegToRM(EBP, -(dest*4), EAX, gen);
    }
}

void Emitter32::emitLoad(Register dest, Address addr, int size, codeGen &gen)
{
    if (size == 1) {
	emitMovMBToReg(EAX, addr, gen);               // movsbl eax, addr
    } else if (size == 2) {
	emitMovMWToReg(EAX, addr, gen);               // movswl eax, addr
    } else {
	emitMovMToReg(EAX, addr, gen);               // mov eax, addr
    }
    emitMovRegToRM(EBP, -(dest*4), EAX, gen);    // mov -(dest*4)[ebp], eax
}

void Emitter32::emitLoadConst(Register dest, Address imm, codeGen &gen)
{
    emitMovImmToRM(EBP, -(dest*4), imm, gen);
}

void Emitter32::emitLoadIndir(Register dest, Register addr_reg, codeGen &gen)
{
    emitMovRMToReg(EAX, EBP, -(addr_reg*4), gen); // mov eax, -(addr_reg*4)[ebp]
    emitMovRMToReg(EAX, EAX, 0, gen);         // mov eax, [eax]
    emitMovRegToRM(EBP, -(dest*4), EAX, gen); // mov -(dest*4)[ebp], eax
}

void Emitter32::emitLoadFrameRelative(Register dest, Address offset, codeGen &gen)
{
    // eax = [ebp]	- saved bp
    // dest = [eax](offset)
    emitMovRMToReg(EAX, EBP, 0, gen);       // mov (%ebp), %eax 
    emitMovRMToReg(EAX, EAX, offset, gen);    // mov <offset>(%eax), %eax 
    emitMovRegToRM(EBP, -(dest*4), EAX, gen);    // mov -(dest*4)[ebp], eax
}

void Emitter32::emitLoadFrameAddr(Register dest, Address offset, codeGen &gen)
{
    emitMovRMToReg(EAX, EBP, 0, gen);       // mov (%ebp), %eax 
    emitAddRegImm32(EAX, offset, gen);        // add #<offset>, %eax
    emitMovRegToRM(EBP, -(dest*4), EAX, gen);    // mov -(dest*4)[ebp], eax
}

void Emitter32::emitStore(Address addr, Register src, codeGen &gen)
{
      emitMovRMToReg(EAX, EBP, -(src*4), gen);    // mov eax, -(src*4)[ebp]
      emitMovRegToM(addr, EAX, gen);               // mov addr, eax
}

void Emitter32::emitStoreIndir(Register addr_reg, Register src, codeGen &gen)
{
    emitMovRMToReg(EAX, EBP, -(src*4), gen);   // mov eax, -(src*4)[ebp]
    emitMovRMToReg(ECX, EBP, -(addr_reg*4), gen);   // mov ecx, -(addr_reg*4)[ebp]
    emitMovRegToRM(ECX, 0, EAX, gen);           // mov [ecx], eax
}

void Emitter32::emitStoreFrameRelative(Address offset, Register src, Register scratch, codeGen &gen)
{
      // scratch = [ebp]	- saved bp
      // (offset)[scratch] = src
      emitMovRMToReg(scratch, EBP, 0, gen);    	    // mov scratch, (ebp)
      emitMovRMToReg(EAX, EBP, -(src*4), gen);    // mov eax, -(src*4)[ebp]
      emitMovRegToRM(scratch, offset, EAX, gen);        // mov (offset)[scratch], eax
}

void Emitter32::emitGetRetVal(Register dest, codeGen &gen)
{
    emitMovRMToReg(EAX, EBP, SAVED_EAX_OFFSET, gen);
    emitMovRegToRM(EBP, -(dest*4), EAX, gen);
}

void Emitter32::emitGetParam(Register dest, Register param_num, instPointType_t pt_type, codeGen &gen)
{
    // Parameters are addressed by a positive offset from ebp,
    // the first is PARAM_OFFSET[ebp]
    if(pt_type == callSite) {
	emitMovRMToReg(EAX, EBP, CALLSITE_PARAM_OFFSET + param_num*4, gen);
	emitMovRegToRM(EBP, -(dest*4), EAX, gen);
    } else {
	// assert(pt_type == functionEntry)
	emitMovRMToReg(EAX, EBP, FUNC_PARAM_OFFSET + param_num*4, gen);
	emitMovRegToRM(EBP, -(dest*4), EAX, gen);
    }
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
    if (rex & 0x0f) {
        GET_PTR(insn, gen);
	*insn++ = rex;
        SET_PTR(insn, gen);
    }
}

// FIXME: better name, since this generates W=0 and W=1 REX prefixes
// FIXME: better yet, make these wrappers macro-based or something
static void emitMovRegToReg64(Register dest, Register src, bool is_64, codeGen &gen)
{
	Register tmp_dest = dest;
	Register tmp_src = src;
	emitRex(is_64, &tmp_dest, NULL, &tmp_src, gen);
	emitMovRegToReg(tmp_dest, tmp_src, gen);
}

static void emitMovImmToReg64(Register dest, long imm, bool is_64, codeGen &gen)
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

static void emitOpRegImm64(unsigned opcode, unsigned opcode_ext, Register rm_reg, int imm,
			   bool is_64, codeGen &gen)
{
    Register tmp_rm_reg = rm_reg;
    emitRex(is_64, NULL, NULL, &tmp_rm_reg, gen);
    
    GET_PTR(insn, gen);
    *insn++ = opcode;
    *insn++ = 0xC0 | (opcode_ext & 0x7 << 3) | tmp_rm_reg;
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
    emitRex(is_64, NULL, NULL, &dest, gen);
    GET_PTR(insn, gen);
    *insn++ = opcode;
    *insn++ = 0xC0 | (opcode_ext & 0x7 << 3) | tmp_dest;
    *insn++ = imm;
    SET_PTR(insn, gen);
}

static void emitIncReg64(Register dest, bool /*is_64*/, codeGen &gen)
{
    emitRex(false, NULL, NULL, &dest, gen);
    GET_PTR(insn, gen);
    *insn++ = 0xFF;
    *insn++ = 0xC0 + dest;
    SET_PTR(insn, gen);
}

class Emitter64 : public Emitter {

public:
    virtual codeBufIndex_t emitIf(Register expr_reg, Register target, codeGen &gen);
    virtual void emitOp(unsigned op, Register dest, Register src1, Register src2, codeGen &gen);
    virtual void emitRelOp(unsigned op, Register dest, Register src1, Register src2, codeGen &gen);
    virtual void emitDiv(Register dest, Register src1, Register src2, codeGen &gen);
    virtual void emitOpImm(unsigned opcode1, unsigned opcode2, Register dest, Register src1, RegValue src2imm,
			   codeGen &gen);
    virtual void emitRelOpImm(unsigned op, Register dest, Register src1, RegValue src2imm, codeGen &gen);
    virtual void emitTimesImm(Register dest, Register src1, RegValue src1imm, codeGen &gen);
    virtual void emitDivImm(Register dest, Register src1, RegValue src1imm, codeGen &gen);
    virtual void emitLoad(Register dest, Address addr, int size, codeGen &gen);
    virtual void emitLoadConst(Register dest, Address imm, codeGen &gen);
    virtual void emitLoadIndir(Register dest, Register addr_reg, codeGen &gen);
    virtual void emitLoadFrameRelative(Register dest, Address offset, codeGen &gen);
    virtual void emitLoadFrameAddr(Register dest, Address offset, codeGen &gen);
    virtual void emitStore(Address addr, Register src, codeGen &gen);
    virtual void emitStoreIndir(Register addr_reg, Register src, codeGen &gen);
    virtual void emitStoreFrameRelative(Address offset, Register src, Register scratch, codeGen &gen);
    virtual Register emitCall(opCode op, registerSpace *rs, codeGen &gen, const pdvector<AstNode *> &operands,
                              process *proc, bool noCost, Address callee_addr, const pdvector<AstNode *> &ifForks,
                              const instPoint *location);
    virtual void emitGetRetVal(Register dest, codeGen &gen);
    virtual void emitGetParam(Register dest, Register param_num, instPointType_t pt_type, codeGen &gen);
};

codeBufIndex_t Emitter64::emitIf(Register expr_reg, Register target, codeGen &gen)
{
    // sub EAX, EAX
    emitOpRegReg(0x29, EAX, EAX, gen);

    // cmp %expr_reg, EAX
    emitOpRegReg64(0x3B, EAX, expr_reg, false, gen);

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
    Register src = 0;
    if (src1 == dest)
	src = src2;
    else if (src2 == dest)
	src = src1;
    else {
	emitMovRegToReg64(dest, src, false, gen);
	src = src2;
    }
    emitOpRegReg64(opcode, dest, src, false, gen);
}

void Emitter64::emitOpImm(unsigned opcode1, unsigned opcode2, Register dest, Register src1, RegValue src2imm,
			  codeGen &gen)
{
    // copy src1 to dest if needed
    if (src1 != dest) {
	emitMovRegToReg64(dest, src1, false, gen);
    }

    emitOpRegImm64(opcode1, opcode2, dest, src2imm, false, gen);
}

void Emitter64::emitRelOp(unsigned op, Register dest, Register src1, Register src2, codeGen &gen)
{
    // sub %dest, %dest
    emitOpRegReg64(0x29, dest, dest, false, gen);

    // cmp %src2, %src1
    emitOpRegReg64(0x39, src1, src2, false, gen);

    // jcc 2 (since inc insstruction will be two bytes)
    GET_PTR(insn, gen);
    unsigned char jcc_opcode = jccOpcodeFromRelOp(op);
    *insn++ = jcc_opcode;
    *insn++ = 2;
    SET_PTR(insn, gen);
    
    // inc %dest
    emitIncReg64(dest, false, gen);
}

void Emitter64::emitRelOpImm(unsigned op, Register dest, Register src1, RegValue src2imm,
			     codeGen &gen)
{
    // sub %dest, %dest
    emitOpRegReg64(0x29, dest, dest, false, gen);

    // cmp $src2imm, %src1
    emitOpRegImm64(0x81, 7, src1, src2imm, false, gen);

    // jcc 2 (inc instruction that follows will be two bytes)
    GET_PTR(insn, gen);
    unsigned char opcode = jccOpcodeFromRelOp(op);
    *insn++ = opcode; *insn++ = 1;
    SET_PTR(insn, gen);

    // inc %dest
    emitIncReg64(dest, false, gen);
}

void Emitter64::emitDiv(Register dest, Register src1, Register src2, codeGen &gen)
{
    // FIXME: need to add a EDX save/restore pair here

    // mov %src1, %eax
    emitMovRegToReg64(src1, EAX, false, gen);

    // cdq (sign extend EAX into EDX)
    emitSimpleInsn(0x99, gen);

    // idiv %src2
    emitOpRegReg64(0xF7, 0x7, src2, false, gen);

    // mov %eax, %dest
    emitMovRegToReg64(dest, EAX, false, gen);
}

void Emitter64::emitTimesImm(Register dest, Register src1, RegValue src2imm, codeGen &gen)
{
    int result = -1;

    if (isPowerOf2(src2imm, result) && result <= MAX_IMM8) {

	// immediate is a power of two - use a shift

	// mov %src1, %dest (if needed)
	if (src1 != dest) {
	    emitMovRegToReg64(dest, src1, false, gen);
	}
	// sal dest, result
	emitOpRegImm8_64(0xC1, 4, EBP, dest, result, gen);
    }
    else {

	// imul %dest, %src1, $src2imm
	emitOpRegRegImm64(0x69, dest, src1, src2imm, false, gen);
    } 
}

void Emitter64::emitDivImm(Register dest, Register src1, RegValue src2imm, codeGen &gen)
{
    int result = -1;
    if (isPowerOf2(src2imm, result) && result <= MAX_IMM8) {

	// divisor is a power of two - use a shift instruction

	// mov %src1, %dest (if needed)
	if (src1 != dest) {
	    emitMovRegToReg64(dest, src1, false, gen);
	}

	// sar $result, %dest
	emitOpRegImm8_64(0xC1, 7, dest, result, false, gen);
    }
    else {

	assert(0);

	// STILL NEED TO IMPLEMENT THIS
	// THERE IS NO IDIV $imm INSTRUCTION SO WE NEED TO GRAB A REGISTER
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
	emitMovImmToReg64(EAX, addr, true, gen);
	
	// mov (%rax), %dest
	emitMovRMToReg64(dest, EAX, 0, size == 8, gen);
    }
}

void Emitter64::emitLoadConst(Register dest, Address imm, codeGen &gen)
{
    emitMovImmToReg64(dest, imm, false, gen);
}

void Emitter64::emitLoadIndir(Register dest, Register addr_src, codeGen &gen)
{
    emitMovRMToReg64(dest, addr_src, 0, false, gen);
}

void Emitter64::emitLoadFrameRelative(Register dest, Address /*offset*/, codeGen &gen)
{
    // mov (%rbp), %rax
    emitMovRMToReg64(EAX, EBP, 0, true, gen);

    // mov offset(%rax), %dest
    emitMovRMToReg64(dest, EAX, 0, false, gen);
}

void Emitter64::emitLoadFrameAddr(Register dest, Address offset, codeGen &gen)
{
    // mov (%rbp), %dest
    emitMovRMToReg64(dest, EBP, 0, true, gen);
}

void Emitter64::emitStore(Address addr, Register src, codeGen &gen)
{
    // mov $addr, %rax
    emitMovImmToReg64(EAX, addr, true, gen);

    // mov %src, (%dest)
    emitMovRegToRM64(EAX, 0, src, false, gen);
}

void Emitter64::emitStoreIndir(Register addr_reg, Register src, codeGen &gen)
{
    emitMovRegToRM64(addr_reg, 0, src, false, gen);
}

void Emitter64::emitStoreFrameRelative(Address offset, Register src, Register scratch, codeGen &gen)
{
    // mov (%rbp), %rax
    emitMovRMToReg64(EAX, EBP, 0, true, gen);
    
    // mov %src, offset(%rax)
    emitMovRegToRM64(EAX, 0, src, false, gen);
}

Register Emitter64::emitCall(opCode op, registerSpace *rs, codeGen &gen, 
                             const pdvector<AstNode *> &operands,
			     process *proc, bool noCost, Address callee_addr, 
                             const pdvector<AstNode *> &ifForks,
			     const instPoint *location)
{
    // not yet implemented
    assert(0);
    return 0;
}

void Emitter64::emitGetRetVal(Register dest, codeGen &gen)
{
    // not yet implemented
    assert(0);
}

void Emitter64::emitGetParam(Register dest, Register param_num, instPointType_t pt_type, codeGen &gen)
{
    // not yet implemented
    assert(0);
}

Emitter64 emitter64;

Emitter* x86_emitter = &emitter32;

// change code generator to 32-bit mode
void emit32()
{
    x86_emitter = &emitter32;
}

// change code generator to 64-bit mode
void emit64()
{
    x86_emitter = &emitter64;
}
