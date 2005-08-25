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

// $Id: inst-x86.h,v 1.19 2005/08/25 22:45:40 bernat Exp $

#ifndef INST_X86_H
#define INST_X86_H

#include "dyninstAPI/src/ast.h"

// some x86 definitions


/*
   We don't use the machine registers to store temporaries,
   but "virtual registers" that are located on the stack.
   The stack frame for a tramp is:

     ebp->    saved ebp (4 bytes)
     ebp-4:   128-byte space for 32 virtual registers (32*4 bytes)
     ebp-132: saved registers (8*4 bytes)
     ebp-164: saved flags registers (4 bytes)
     ebp-168: (MT only) thread index

     The temporaries are assigned numbers from 1 so that it is easier
     to refer to them: -(reg*4)[ebp]. So the first reg is -4[ebp].

     We are using a fixed number of temporaries now (32), but we could 
     change to using an arbitrary number.

*/

#define TRAMP_FRAME_SIZE (128)
#define NUM_VIRTUAL_REGISTERS (32)   /* number of virtual registers */

/*
   Function arguments are in the stack and are addressed with a displacement
   from EBP. EBP points to the saved EBP, EBP+4 is the saved return address,
   EBP+8 is the first parameter.
   TODO: what about far calls?
 */
#define FUNC_PARAM_OFFSET (8+(10*4))
#define CALLSITE_PARAM_OFFSET (4+(10*4))

// offset from EBP of the saved EAX for a tramp
#define SAVED_EAX_OFFSET (10*4-4)
#define SAVED_EFLAGS_OFFSET (SAVED_EAX_OFFSET+4)

// Undefine REG_MT_POS, basically
#define REG_MT_POS NUM_VIRTUAL_REGISTERS

//#ifndef DEBUG_FUNC_RELOC
//#define DEBUG_FUNC_RELOC
//#endif

// Macro for single x86/x86_64 register access
// Register names for use with ptrace calls, not instruction generation.
#if defined(__x86_64__) && __WORDSIZE == 64
#define PTRACE_REG_15		r15
#define PTRACE_REG_14		r14
#define PTRACE_REG_13		r13
#define PTRACE_REG_12		r12
#define PTRACE_REG_BP		rbp
#define PTRACE_REG_BX		rbx
#define PTRACE_REG_11		r11
#define PTRACE_REG_10		r10
#define PTRACE_REG_9		r9
#define PTRACE_REG_8		r8
#define PTRACE_REG_AX		rax
#define PTRACE_REG_CX		rcx
#define PTRACE_REG_DX		rdx
#define PTRACE_REG_SI		rsi
#define PTRACE_REG_DI		rdi
#define PTRACE_REG_ORIG_AX	orig_rax
#define PTRACE_REG_IP		rip
#define PTRACE_REG_CS		cs
#define PTRACE_REG_FLAGS	eflags
#define PTRACE_REG_SP		rsp
#define PTRACE_REG_SS		ss
#define PTRACE_REG_FS_BASE	fs_base
#define PTRACE_REG_GS_BASE	gs_base
#define PTRACE_REG_DS		ds
#define PTRACE_REG_ES		es
#define PTRACE_REG_FS		fs
#define PTRACE_REG_GS		gs

#else
#define PTRACE_REG_BX		ebx
#define PTRACE_REG_CX		ecx
#define PTRACE_REG_DX		edx
#define PTRACE_REG_SI		esi
#define PTRACE_REG_DI		edi
#define PTRACE_REG_BP		ebp
#define PTRACE_REG_AX		eax
#define PTRACE_REG_DS		xds
#define PTRACE_REG_ES		xes
#define PTRACE_REG_FS		xfs
#define PTRACE_REG_GS		xgs
#define PTRACE_REG_ORIG_AX	orig_eax
#define PTRACE_REG_IP		eip
#define PTRACE_REG_CS		xcs
#define PTRACE_REG_FLAGS	eflags
#define PTRACE_REG_SP		esp
#define PTRACE_REG_SS		xss

#endif

// The general machine registers.
// These values are taken from the Pentium manual and CANNOT be changed.

// 32-bit
#define REGNUM_EAX (0)
#define REGNUM_ECX (1)
#define REGNUM_EDX (2)
#define REGNUM_EBX (3)
#define REGNUM_ESP (4)
#define REGNUM_EBP (5)
#define REGNUM_ESI (6)
#define REGNUM_EDI (7)

// 64-bit
#define REGNUM_RAX (0)
#define REGNUM_RCX (1)
#define REGNUM_RDX (2)
#define REGNUM_RBX (3)
#define REGNUM_RSP (4)
#define REGNUM_RBP (5)
#define REGNUM_RSI (6)
#define REGNUM_RDI (7)
#define REGNUM_R8 (8)
#define REGNUM_R9 (9)
#define REGNUM_R10 (10)
#define REGNUM_R11 (11)
#define REGNUM_R12 (12)
#define REGNUM_R13 (13)
#define REGNUM_R14 (14)
#define REGNUM_R15 (15)

// on AMD64, we sometimes can't reach our minitramp from the basetramp with
// a direct jump. we always can on x86
#if defined(arch_x86_64)
#define BASE_TO_MINI_JMP_SIZE 12
#else
#define BASE_TO_MINI_JMP_SIZE JUMP_REL32_SZ
#endif

// 32 and 64-bit register spaces
extern registerSpace* regSpace32;
#if defined(arch_x86_64)
extern registerSpace* regSpace64;
#endif

class codeGen;

// Define access method for saved register (GPR)
#define GET_GPR(x, insn) emitMovRMToReg(REGNUM_EAX, REGNUM_EBP, SAVED_EAX_OFFSET-(x*4), insn)

// Define access method for virtual registers (stack-based)
#define LOAD_VIRTUAL(x, insn) emitMovRMToReg(REGNUM_EAX, REGNUM_EBP, -(x*4), insn)
#define SAVE_VIRTUAL(x, insn) emitMovRegToRM(REGNUM_EBP, -(x*4), REGNUM_EAX, insn)

// low-level code generation functions
void emitOpRegReg(unsigned opcode, Register dest, Register src, codeGen &gen);
void emitOpRegRM(unsigned opcode, Register dest, Register base, int disp, codeGen &gen);
void emitOpRegImm(int opcode, Register dest, int imm, codeGen &gen);
void emitOpRegRMImm(unsigned opcode, Register dest, Register base, int disp, int imm, codeGen &gen);
void emitOpRMImm(unsigned opcode1, unsigned opcode2, Register base, int disp, int imm, codeGen &gen);
void emitOpRMImm8( unsigned opcode1, unsigned opcode2, Register base, int disp, char imm, codeGen &gen);

void emitMovRegToReg(Register dest, Register src, codeGen &gen);
void emitMovMToReg(Register dest, int disp, codeGen &gen);
void emitMovMBToReg(Register dest, int disp, codeGen &gen);
void emitMovMWToReg(Register dest, int disp, codeGen &gen);
void emitMovRegToM(int disp, Register src, codeGen &gen);
void emitMovImmToReg(Register dest, int imm, codeGen &gen);
void emitMovImmToRM(Register base, int disp, int imm, codeGen &gen);
void emitMovRegToRM(Register base, int disp, Register src, codeGen &gen);
void emitMovRMToReg(Register dest, Register base, int disp, codeGen &gen);
void emitMovImmToMem(Address maddr, int imm, codeGen &gen);

void emitPushImm(unsigned int imm, codeGen &gen);

void emitSimpleInsn(unsigned opcode, codeGen &gen);

void emitAddRegImm32(Register dest, int imm, codeGen &gen);
void emitAddMemImm32(Address dest, int imm, codeGen &gen);

void emitLEA(Register base, Register index, unsigned int scale,
	     RegValue disp, Register dest, codeGen &gen);


void emitJump(unsigned disp32, codeGen &gen);
void emitJccR8(int condition_code, char jump_offset,
               codeGen &gen);
void emitPushImm(unsigned int imm, codeGen &gen);
void emitAddMemImm32(Address dest, int imm, codeGen &gen);
void emitCallRel32(unsigned disp32, codeGen &gen);

void emitJmpMC(int condition, int offset, codeGen &gen);
// helper functions for emitters
unsigned char jccOpcodeFromRelOp(unsigned op);

#endif
