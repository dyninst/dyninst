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

// $Id: inst-x86.h,v 1.16 2005/06/09 17:20:55 gquinn Exp $

#ifndef INST_X86_H
#define INST_X86_H

// some x86 definitions
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
#define EAX (0)
#define ECX (1)
#define EDX (2)
#define EBX (3)
#define ESP (4)
#define EBP (5)
#define ESI (6)
#define EDI (7)

// Define access method for saved register (GPR)
#define GET_GPR(x, insn) emitMovRMToReg(EAX, EBP, SAVED_EAX_OFFSET-(x*4), insn)

// Define access method for virtual registers (stack-based)
#define LOAD_VIRTUAL(x, insn) emitMovRMToReg(EAX, EBP, -(x*4), insn)
#define SAVE_VIRTUAL(x, insn) emitMovRegToRM(EBP, -(x*4), EAX, insn)

// low-level code generation functions
void emitOpRegReg(unsigned opcode, Register dest, Register src, unsigned char *&insn);
void emitOpRegRM(unsigned opcode, Register dest, Register base, int disp, unsigned char *&insn);
void emitOpRegImm(int opcode, Register dest, int imm, unsigned char *&insn);
void emitOpRegRMImm(unsigned opcode, Register dest, Register base, int disp, int imm, unsigned char *&insn);
void emitOpRMImm(unsigned opcode1, unsigned opcode2, Register base, int disp, int imm, unsigned char *&insn);
void emitOpRMImm8( unsigned opcode1, unsigned opcode2, Register base, int disp, char imm, unsigned char * & insn );

void emitMovRegToReg(Register dest, Register src, unsigned char *&insn);
void emitMovMToReg(Register dest, int disp, unsigned char *&insn);
void emitMovMBToReg(Register dest, int disp, unsigned char *&insn);
void emitMovMWToReg(Register dest, int disp, unsigned char *&insn);
void emitMovRegToM(int disp, Register src, unsigned char *&insn);
void emitMovImmToReg(Register dest, int imm, unsigned char *&insn);
void emitMovImmToRM(Register base, int disp, int imm, unsigned char *&insn);
void emitMovRegToRM(Register base, int disp, Register src, unsigned char *&insn);
void emitMovRMToReg(Register dest, Register base, int disp, unsigned char *&insn);

void emitSimpleInsn(unsigned opcode, unsigned char *&insn);

void emitAddRegImm32(Register dest, int imm, unsigned char *&insn);

// helper functions for emitters
unsigned char jccOpcodeFromRelOp(unsigned op);

#endif
