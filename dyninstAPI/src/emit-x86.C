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
 * $Id: emit-x86.C,v 1.2 2005/06/13 19:14:33 gquinn Exp $
 */

#include "dyninstAPI/src/emit-x86.h"
#include "dyninstAPI/src/inst-x86.h"
#include "dyninstAPI/src/arch-x86.h"

void Emitter32::emitIf(Register expr_reg, Register target, unsigned char*& insn)
{
    // sub EAX, EAX
    emitOpRegReg(0x29, EAX, EAX, insn);

    // cmp -(expr*4)[EBP], EAX
    emitOpRegRM(0x3B, EAX, EBP, -(expr_reg*4), insn);

    // je dest
    *insn++ = 0x0F;
    *insn++ = 0x84;
    *((int *)insn) = target;
    insn += sizeof(int);
}

void Emitter32::emitOp(unsigned opcode, Register dest, Register src1, Register src2, unsigned char*& insn)
{
      emitMovRMToReg(EAX, EBP, -(src1*4), insn);
      emitOpRegRM(opcode, EAX, EBP, -(src2*4), insn);
      emitMovRegToRM(EBP, -(dest*4), EAX, insn);
}

void Emitter32::emitRelOp(unsigned op, Register dest, Register src1, Register src2, unsigned char*& insn)
{
    emitOpRegReg(0x29, ECX, ECX, insn);           // clear ECX
    emitMovRMToReg(EAX, EBP, -(src1*4), insn);    // mov eax, -(src1*4)[ebp]
    emitOpRegRM(0x3B, EAX, EBP, -(src2*4), insn); // cmp eax, -(src2*4)[ebp]
    unsigned char opcode = jccOpcodeFromRelOp(op);
    *insn++ = opcode; *insn++ = 1;                // jcc 1
    emitSimpleInsn(0x40+ECX, insn);               // inc ECX
    emitMovRegToRM(EBP, -(dest*4), ECX, insn);    // mov -(dest*4)[ebp], ecx
}

void Emitter32::emitDiv(Register dest, Register src1, Register src2, unsigned char*& insn)
{
    // mov eax, src1
    // cdq   ; edx = sign extend of eax
    // idiv eax, src2 ; eax = edx:eax div src2, edx = edx:eax mod src2
    // mov dest, eax
    emitMovRMToReg(EAX, EBP, -(src1*4), insn);
    emitSimpleInsn(0x99, insn);
    emitOpRegRM(0xF7, 0x7 /*opcode extension*/, EBP, -(src2*4), insn);
    emitMovRegToRM(EBP, -(dest*4), EAX, insn);
}

void Emitter32::emitOpImm(unsigned opcode1, unsigned opcode2, Register dest, Register src1, RegValue src2imm,
			  unsigned char*& insn)
{
    if (src1 != dest) {
	emitMovRMToReg(EAX, EBP, -(src1*4), insn);
	emitMovRegToRM(EBP, -(dest*4), EAX, insn);
    }
    emitOpRMImm(opcode1, opcode2, EBP, -(dest*4), src2imm, insn);
}

void Emitter32::emitRelOpImm(unsigned op, Register dest, Register src1, RegValue src2imm, unsigned char*& insn)
{
   emitOpRegReg(0x29, ECX, ECX, insn);           // clear ECX
   emitMovRMToReg(EAX, EBP, -(src1*4), insn);    // mov eax, -(src1*4)[ebp]
   emitOpRegImm(0x3D, EAX, src2imm, insn);       // cmp eax, src2
   unsigned char opcode = jccOpcodeFromRelOp(op);
   *insn++ = opcode; *insn++ = 1;                // jcc 1
   emitSimpleInsn(0x40+ECX, insn);               // inc ECX
   emitMovRegToRM(EBP, -(dest*4), ECX, insn);    // mov -(dest*4)[ebp], ecx   
}

// where is this defined?
extern bool isPowerOf2(int value, int &result);

void Emitter32::emitTimesImm(Register dest, Register src1, RegValue src2imm, unsigned char*& insn)
{
    int result = -1;

    if (isPowerOf2(src2imm, result) && result <= MAX_IMM8) {
	if (src1 != dest) {
	    emitMovRMToReg(EAX, EBP, -(src1*4), insn);
	    emitMovRegToRM(EBP, -(dest*4), EAX, insn);
	}
	// sal dest, result
	emitOpRMImm8(0xC1, 4, EBP, -(dest*4), result, insn);
    }
    else {
	// imul EAX, -(src1*4)[ebp], src2imm
	emitOpRegRMImm(0x69, EAX, EBP, -(src1*4), src2imm, insn);
	emitMovRegToRM(EBP, -(dest*4), EAX, insn);
    } 
}

void Emitter32::emitDivImm(Register dest, Register src1, RegValue src2imm, unsigned char*& insn)
{
    int result = -1;
    if (isPowerOf2(src2imm, result) && result <= MAX_IMM8) {
	if (src1 != dest) {
	    emitMovRMToReg(EAX, EBP, -(src1*4), insn);
	    emitMovRegToRM(EBP, -(dest*4), EAX, insn);
	}
	// sar dest, result
	emitOpRMImm8(0xC1, 7, EBP, -(dest*4), result, insn);
    }
    else {
	// dest = src1 div src2imm
	// mov eax, src1
	// cdq   ; edx = sign extend of eax
	// mov ebx, src2imm
	// idiv eax, ebx ; eax = edx:eax div src2, edx = edx:eax mod src2
	// mov dest, eax
	emitMovRMToReg(EAX, EBP, -(src1*4), insn);
	emitSimpleInsn(0x99, insn);
	emitMovImmToReg(EBX, src2imm, insn);
	// idiv eax, ebx
	emitOpRegReg(0xF7, 0x7 /*opcode extension*/, EBX, insn); 
	emitMovRegToRM(EBP, -(dest*4), EAX, insn);
    }
}

void Emitter32::emitLoad(Register dest, Address addr, int size, unsigned char*& insn)
{
    if (size == 1) {
	emitMovMBToReg(EAX, addr, insn);               // movsbl eax, addr
    } else if (size == 2) {
	emitMovMWToReg(EAX, addr, insn);               // movswl eax, addr
    } else {
	emitMovMToReg(EAX, addr, insn);               // mov eax, addr
    }
    emitMovRegToRM(EBP, -(dest*4), EAX, insn);    // mov -(dest*4)[ebp], eax
}

void Emitter32::emitLoadConst(Register dest, Address imm, unsigned char*& insn)
{
    emitMovImmToRM(EBP, -(dest*4), imm, insn);
}

void Emitter32::emitLoadIndir(Register dest, Register addr_reg, unsigned char*& insn)
{
    emitMovRMToReg(EAX, EBP, -(addr_reg*4), insn); // mov eax, -(addr_reg*4)[ebp]
    emitMovRMToReg(EAX, EAX, 0, insn);         // mov eax, [eax]
    emitMovRegToRM(EBP, -(dest*4), EAX, insn); // mov -(dest*4)[ebp], eax
}

void Emitter32::emitLoadFrameRelative(Register dest, Address offset, unsigned char*& insn)
{
    // eax = [ebp]	- saved bp
    // dest = [eax](offset)
    emitMovRMToReg(EAX, EBP, 0, insn);       // mov (%ebp), %eax 
    emitMovRMToReg(EAX, EAX, offset, insn);    // mov <offset>(%eax), %eax 
    emitMovRegToRM(EBP, -(dest*4), EAX, insn);    // mov -(dest*4)[ebp], eax
}

void Emitter32::emitLoadFrameAddr(Register dest, Address offset, unsigned char*& insn)
{
    emitMovRMToReg(EAX, EBP, 0, insn);       // mov (%ebp), %eax 
    emitAddRegImm32(EAX, offset, insn);        // add #<offset>, %eax
    emitMovRegToRM(EBP, -(dest*4), EAX, insn);    // mov -(dest*4)[ebp], eax
}

void Emitter32::emitStore(Address addr, Register src, unsigned char*& insn)
{
      emitMovRMToReg(EAX, EBP, -(src*4), insn);    // mov eax, -(src*4)[ebp]
      emitMovRegToM(addr, EAX, insn);               // mov addr, eax
}

void Emitter32::emitStoreIndir(Register addr_reg, Register src, unsigned char*& insn)
{
    emitMovRMToReg(EAX, EBP, -(src*4), insn);   // mov eax, -(src*4)[ebp]
    emitMovRMToReg(ECX, EBP, -(addr_reg*4), insn);   // mov ecx, -(addr_reg*4)[ebp]
    emitMovRegToRM(ECX, 0, EAX, insn);           // mov [ecx], eax
}

void Emitter32::emitStoreFrameRelative(Address offset, Register src, Register scratch, unsigned char*& insn)
{
      // scratch = [ebp]	- saved bp
      // (offset)[scratch] = src
      emitMovRMToReg(scratch, EBP, 0, insn);    	    // mov scratch, (ebp)
      emitMovRMToReg(EAX, EBP, -(src*4), insn);    // mov eax, -(src*4)[ebp]
      emitMovRegToRM(scratch, offset, EAX, insn);        // mov (offset)[scratch], eax
}

void Emitter32::emitGetRetVal(Register dest, unsigned char*& insn)
{
    emitMovRMToReg(EAX, EBP, SAVED_EAX_OFFSET, insn);
    emitMovRegToRM(EBP, -(dest*4), EAX, insn);
}

void Emitter32::emitGetParam(Register dest, Register param_num, instPointType pt_type, unsigned char*& insn)
{
    // Parameters are addressed by a positive offset from ebp,
    // the first is PARAM_OFFSET[ebp]
    if(pt_type == callSite) {
	emitMovRMToReg(EAX, EBP, CALLSITE_PARAM_OFFSET + param_num*4, insn);
	emitMovRegToRM(EBP, -(dest*4), EAX, insn);
    } else {
	// assert(pt_type == functionEntry)
	emitMovRMToReg(EAX, EBP, FUNC_PARAM_OFFSET + param_num*4, insn);
	emitMovRegToRM(EBP, -(dest*4), EAX, insn);
    }
}

Emitter32 emitter32;

//
// 64-bit code generation helper functions
//

static void emitRex(bool is_64, Register* r, Register* x, Register* b, unsigned char*& insn)
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
	*insn++ = rex;
}

// FIXME: better name, since this generates W=0 and W=1 REX prefixes
// FIXME: better yet, make these wrappers macro-based or something
static void emitMovRegToReg64(Register dest, Register src, bool is_64, unsigned char*& insn)
{
	Register tmp_dest = dest;
	Register tmp_src = src;
	emitRex(is_64, &tmp_dest, NULL, &tmp_src, insn);
	emitMovRegToReg(tmp_dest, tmp_src, insn);
}

static void emitMovImmToReg64(Register dest, long imm, bool is_64, unsigned char*& insn)
{
    Register tmp_dest = dest;
    emitRex(is_64, NULL, NULL, &tmp_dest, insn);
    if (is_64) {
	*insn++ = 0xB8 + tmp_dest;
	*((long *)insn) = imm;
	insn += sizeof(long);
    }
    else
	emitMovImmToReg(tmp_dest, imm, insn);
}

static void emitMovRMToReg64(Register dest, Register base, int disp, bool is_64, unsigned char*& insn)
{
    Register tmp_dest = dest;
    Register tmp_base = base;
    emitRex(is_64, &tmp_dest, NULL, &tmp_base, insn);
    emitMovRMToReg(tmp_dest, tmp_base, disp, insn);
}

static void emitMovRegToRM64(Register base, int disp, Register src, bool is_64, unsigned char*& insn)
{
    Register tmp_base = base;
    Register tmp_src = src;
    emitRex(is_64, &tmp_src, NULL, &tmp_base, insn);
    emitMovRegToRM(tmp_base, disp, tmp_src, insn);
}

static void emitOpRegReg64(unsigned opcode, Register dest, Register src, bool is_64, unsigned char*& insn)
{
    Register tmp_dest = dest;
    Register tmp_src = src;
    emitRex(is_64, &tmp_dest, NULL, &tmp_src, insn);
    emitOpRegReg(opcode, tmp_dest, tmp_src, insn);
}

static void emitOpRegImm64(unsigned opcode, unsigned opcode_ext, Register rm_reg, int imm,
			   bool is_64, unsigned char*& insn)
{
    Register tmp_rm_reg = rm_reg;
    emitRex(is_64, NULL, NULL, &tmp_rm_reg, insn);

    *insn++ = opcode;
    *insn++ = 0xC0 | (opcode_ext & 0x7 << 3) | tmp_rm_reg;
    *((int *)insn) = imm;
    insn+= sizeof(int);
}

static void emitOpRegRegImm64(unsigned opcode, Register dest, Register src1, int imm,
			      bool is_64, unsigned char*& insn)
{
    emitOpRegReg64(opcode, dest, src1, is_64, insn);
    *((int *)insn) = imm;
    insn+= sizeof(int);
}

static void emitOpRegImm8_64(unsigned opcode, unsigned opcode_ext, Register dest,
			     char imm, bool is_64, unsigned char*& insn)
{
    Register tmp_dest = dest;
    emitRex(is_64, NULL, NULL, &dest, insn);
    *insn++ = opcode;
    *insn++ = 0xC0 | (opcode_ext & 0x7 << 3) | tmp_dest;
    *insn++ = imm;
}

static void emitIncReg64(Register dest, bool is_64, unsigned char*& insn)
{
    emitRex(false, NULL, NULL, &dest, insn);
    *insn++ = 0xFF;
    *insn++ = 0xC0 + dest;
}

class Emitter64 : public Emitter {

public:
    virtual void emitIf(Register expr_reg, Register target, unsigned char*& insn);
    virtual void emitOp(unsigned op, Register dest, Register src1, Register src2, unsigned char*& insn);
    virtual void emitRelOp(unsigned op, Register dest, Register src1, Register src2, unsigned char*& insn);
    virtual void emitDiv(Register dest, Register src1, Register src2, unsigned char*& insn);
    virtual void emitOpImm(unsigned opcode1, unsigned opcode2, Register dest, Register src1, RegValue src2imm,
			   unsigned char*& insn);
    virtual void emitRelOpImm(unsigned op, Register dest, Register src1, RegValue src2imm, unsigned char*& insn);
    virtual void emitTimesImm(Register dest, Register src1, RegValue src1imm, unsigned char*& insn);
    virtual void emitDivImm(Register dest, Register src1, RegValue src1imm, unsigned char*& insn);
    virtual void emitLoad(Register dest, Address addr, int size, unsigned char*& insn);
    virtual void emitLoadConst(Register dest, Address imm, unsigned char*& insn);
    virtual void emitLoadIndir(Register dest, Register addr_reg, unsigned char*& insn);
    virtual void emitLoadFrameRelative(Register dest, Address offset, unsigned char*& insn);
    virtual void emitLoadFrameAddr(Register dest, Address offset, unsigned char*& insn);
    virtual void emitStore(Address addr, Register src, unsigned char*& insn);
    virtual void emitStoreIndir(Register addr_reg, Register src, unsigned char*& insn);
    virtual void emitStoreFrameRelative(Address offset, Register src, Register scratch, unsigned char*& insn);
    virtual Register emitCall(opCode op, registerSpace *rs, char *ibuf, Address &base, const pdvector<AstNode *> &operands,
			  process *proc, bool noCost, Address callee_addr, const pdvector<AstNode *> &ifForks,
			  const instPoint *location);
    virtual void emitGetRetVal(Register dest, unsigned char*& insn);
    virtual void emitGetParam(Register dest, Register param_num, instPointType pt_type, unsigned char*& insn);
};

void Emitter64::emitIf(Register expr_reg, Register target, unsigned char*& insn)
{
    // sub EAX, EAX
    emitOpRegReg(0x29, EAX, EAX, insn);

    // cmp %expr_reg, EAX
    emitOpRegReg64(0x3B, EAX, expr_reg, false, insn);

    // jz target
    *insn++ = 0x0F;
    *insn++ = 0x84;
    *((int *)insn) = target;
    insn += sizeof(int);
}

void Emitter64::emitOp(unsigned opcode, Register dest, Register src1, Register src2, unsigned char*& insn)
{
    Register src;
    if (src1 == dest)
	src = src2;
    else if (src2 == dest)
	src = src1;
    else {
	emitMovRegToReg64(dest, src, false, insn);
	src = src2;
    }
    emitOpRegReg64(opcode, dest, src, false, insn);
}

void Emitter64::emitOpImm(unsigned opcode1, unsigned opcode2, Register dest, Register src1, RegValue src2imm,
			  unsigned char*& insn)
{
    // copy src1 to dest if needed
    if (src1 != dest) {
	emitMovRegToReg64(dest, src1, false, insn);
    }

    emitOpRegImm64(opcode1, opcode2, dest, src2imm, false, insn);
}

void Emitter64::emitRelOp(unsigned op, Register dest, Register src1, Register src2, unsigned char*& insn)
{
    // sub %dest, %dest
    emitOpRegReg64(0x29, dest, dest, false, insn);

    // cmp %src2, %src1
    emitOpRegReg64(0x39, src1, src2, false, insn);

    // jcc 2 (since inc insstruction will be two bytes)
    unsigned char jcc_opcode = jccOpcodeFromRelOp(op);
    *insn++ = jcc_opcode;
    *insn++ = 2;
    
    // inc %dest
    emitIncReg64(dest, false, insn);
}

void Emitter64::emitRelOpImm(unsigned op, Register dest, Register src1, RegValue src2imm,
			     unsigned char*& insn)
{
    // sub %dest, %dest
    emitOpRegReg64(0x29, dest, dest, false, insn);

    // cmp $src2imm, %src1
    emitOpRegImm64(0x81, 7, src1, src2imm, false, insn);

    // jcc 2 (inc instruction that follows will be two bytes)
    unsigned char opcode = jccOpcodeFromRelOp(op);
    *insn++ = opcode; *insn++ = 1;

    // inc %dest
    emitIncReg64(dest, false, insn);
}

void Emitter64::emitDiv(Register dest, Register src1, Register src2, unsigned char*& insn)
{
    // FIXME: need to add a EDX save/restore pair here

    // mov %src1, %eax
    emitMovRegToReg64(src1, EAX, false, insn);

    // cdq (sign extend EAX into EDX)
    emitSimpleInsn(0x99, insn);

    // idiv %src2
    emitOpRegReg64(0xF7, 0x7, src2, false, insn);

    // mov %eax, %dest
    emitMovRegToReg64(dest, EAX, false, insn);
}

void Emitter64::emitTimesImm(Register dest, Register src1, RegValue src2imm, unsigned char*& insn)
{
    int result = -1;

    if (isPowerOf2(src2imm, result) && result <= MAX_IMM8) {

	// immediate is a power of two - use a shift

	// mov %src1, %dest (if needed)
	if (src1 != dest) {
	    emitMovRegToReg64(dest, src1, false, insn);
	}
	// sal dest, result
	emitOpRegImm8_64(0xC1, 4, EBP, dest, result, insn);
    }
    else {

	// imul %dest, %src1, $src2imm
	emitOpRegRegImm64(0x69, dest, src1, src2imm, false, insn);
    } 
}

void Emitter64::emitDivImm(Register dest, Register src1, RegValue src2imm, unsigned char*& insn)
{
    int result = -1;
    if (isPowerOf2(src2imm, result) && result <= MAX_IMM8) {

	// divisor is a power of two - use a shift instruction

	// mov %src1, %dest (if needed)
	if (src1 != dest) {
	    emitMovRegToReg64(dest, src1, false, insn);
	}

	// sar $result, %dest
	emitOpRegImm8_64(0xC1, 7, dest, result, false, insn);
    }
    else {

	assert(0);

	// STILL NEED TO IMPLEMENT THIS
	// THERE IS NO IDIV $imm INSTRUCTION SO WE NEED TO GRAB A REGISTER
    }
}

void Emitter64::emitLoad(Register dest, Address addr, int size, unsigned char*& insn)
{
    if (size == 1) {
	assert(0);
    } else if (size == 2) {
	assert(0);
    } else {
	assert(size == 4 || size == 8);
	
	// mov $addr, %rax
	emitMovImmToReg64(EAX, addr, true, insn);
	
	// mov (%rax), %dest
	emitMovRMToReg64(dest, EAX, 0, size == 8, insn);
    }
}

void Emitter64::emitLoadConst(Register dest, Address imm, unsigned char*& insn)
{
    emitMovImmToReg64(dest, imm, false, insn);
}

void Emitter64::emitLoadIndir(Register dest, Register addr_src, unsigned char*& insn)
{
    emitMovRMToReg64(dest, addr_src, 0, false, insn);
}

void Emitter64::emitLoadFrameRelative(Register dest, Address offset, unsigned char*& insn)
{
    // mov (%rbp), %rax
    emitMovRMToReg64(EAX, EBP, 0, true, insn);

    // mov offset(%rax), %dest
    emitMovRMToReg64(dest, EAX, 0, false, insn);
}

void Emitter64::emitLoadFrameAddr(Register dest, Address offset, unsigned char*& insn)
{
    // mov (%rbp), %dest
    emitMovRMToReg64(dest, EBP, 0, true, insn);
}

void Emitter64::emitStore(Address addr, Register src, unsigned char*& insn)
{
    // mov $addr, %rax
    emitMovImmToReg64(EAX, addr, true, insn);

    // mov %src, (%dest)
    emitMovRegToRM64(EAX, 0, src, false, insn);
}

void Emitter64::emitStoreIndir(Register addr_reg, Register src, unsigned char*& insn)
{
    emitMovRegToRM64(addr_reg, 0, src, false, insn);
}

void Emitter64::emitStoreFrameRelative(Address offset, Register src, Register scratch, unsigned char*& insn)
{
    // mov (%rbp), %rax
    emitMovRMToReg64(EAX, EBP, 0, true, insn);
    
    // mov %src, offset(%rax)
    emitMovRegToRM64(EAX, 0, src, false, insn);
}

Register Emitter64::emitCall(opCode op, registerSpace *rs, char *ibuf, Address &base, const pdvector<AstNode *> &operands,
			     process *proc, bool noCost, Address callee_addr, const pdvector<AstNode *> &ifForks,
			     const instPoint *location)
{
    // not yet implemented
    assert(0);
    return 0;
}

void Emitter64::emitGetRetVal(Register dest, unsigned char*& insn)
{
    // not yet implemented
    assert(0);
}

void Emitter64::emitGetParam(Register dest, Register param_num, instPointType pt_type, unsigned char*& insn)
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
