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

// $Id: arch-x86.h,v 1.67 2008/10/28 18:42:39 bernat Exp $
// x86 instruction declarations

#ifndef _ARCH_X86_H
#define _ARCH_X86_H

#include "dyntypes.h"
#include <assert.h>
#include <stdio.h>
#include <set>
#include <map>
#include <vector>
#include "entryIDs.h"
#include "registers/MachRegister.h"
#include "common/src/ia32_locations.h"
#include "dyn_register.h"


namespace NS_x86 {

/* operand types */
/* signed char required for correct immediate value interpretation */
typedef signed char byte_t;   /* a byte operand */
typedef short word_t;  /* a word (16-bit) operand */
typedef int dword_t;   /* a double word (32-bit) operand */

// The general machine registers.
// These values are taken from the Pentium manual and CANNOT be changed.

// 32-bit
#define REGNUM_EAX 0
#define REGNUM_ECX 1
#define REGNUM_EDX 2
#define REGNUM_EBX 3
#define REGNUM_ESP 4
#define REGNUM_EBP 5
#define REGNUM_ESI 6
#define REGNUM_EDI 7

// 64-bit
enum AMD64_REG_NUMBERS {
    REGNUM_RAX = 0,
    REGNUM_RCX,
    REGNUM_RDX,
    REGNUM_RBX,
    REGNUM_RSP,
    REGNUM_RBP,
    REGNUM_RSI,
    REGNUM_RDI,
    REGNUM_R8,
    REGNUM_R9,
    REGNUM_R10,
    REGNUM_R11,
    REGNUM_R12,
    REGNUM_R13,
    REGNUM_R14,
    REGNUM_R15,
    REGNUM_DUMMYFPR,
    REGNUM_OF,
    REGNUM_SF,
    REGNUM_ZF,
    REGNUM_AF,
    REGNUM_PF,
    REGNUM_CF,
    REGNUM_TF,
    REGNUM_IF,
    REGNUM_DF,
    REGNUM_NT,
    REGNUM_RF,
    REGNUM_MM0,
    REGNUM_MM1,
    REGNUM_MM2,
    REGNUM_MM3,
    REGNUM_MM4,
    REGNUM_MM5,
    REGNUM_MM6,
    REGNUM_MM7,
    REGNUM_K0,
    REGNUM_K1,
    REGNUM_K2,
    REGNUM_K3,
    REGNUM_K4,
    REGNUM_K5,
    REGNUM_K6,
    REGNUM_K7,
    REGNUM_XMM0,
    REGNUM_XMM1,
    REGNUM_XMM2,
    REGNUM_XMM3,
    REGNUM_XMM4,
    REGNUM_XMM5,
    REGNUM_XMM6,
    REGNUM_XMM7,
    REGNUM_XMM8,
    REGNUM_XMM9,
    REGNUM_XMM10,
    REGNUM_XMM11,
    REGNUM_XMM12,
    REGNUM_XMM13,
    REGNUM_XMM14,
    REGNUM_XMM15,
    REGNUM_XMM16,
    REGNUM_XMM17,
    REGNUM_XMM18,
    REGNUM_XMM19,
    REGNUM_XMM20,
    REGNUM_XMM21,
    REGNUM_XMM22,
    REGNUM_XMM23,
    REGNUM_XMM24,
    REGNUM_XMM25,
    REGNUM_XMM26,
    REGNUM_XMM27,
    REGNUM_XMM28,
    REGNUM_XMM29,
    REGNUM_XMM30,
    REGNUM_XMM31,
    REGNUM_YMM0,
    REGNUM_YMM1,
    REGNUM_YMM2,
    REGNUM_YMM3,
    REGNUM_YMM4,
    REGNUM_YMM5,
    REGNUM_YMM6,
    REGNUM_YMM7,
    REGNUM_YMM8,
    REGNUM_YMM9,
    REGNUM_YMM10,
    REGNUM_YMM11,
    REGNUM_YMM12,
    REGNUM_YMM13,
    REGNUM_YMM14,
    REGNUM_YMM15,
    REGNUM_YMM16,
    REGNUM_YMM17,
    REGNUM_YMM18,
    REGNUM_YMM19,
    REGNUM_YMM20,
    REGNUM_YMM21,
    REGNUM_YMM22,
    REGNUM_YMM23,
    REGNUM_YMM24,
    REGNUM_YMM25,
    REGNUM_YMM26,
    REGNUM_YMM27,
    REGNUM_YMM28,
    REGNUM_YMM29,
    REGNUM_YMM30,
    REGNUM_YMM31,
    REGNUM_ZMM0,
    REGNUM_ZMM1,
    REGNUM_ZMM2,
    REGNUM_ZMM3,
    REGNUM_ZMM4,
    REGNUM_ZMM5,
    REGNUM_ZMM6,
    REGNUM_ZMM7,
    REGNUM_ZMM8,
    REGNUM_ZMM9,
    REGNUM_ZMM10,
    REGNUM_ZMM11,
    REGNUM_ZMM12,
    REGNUM_ZMM13,
    REGNUM_ZMM14,
    REGNUM_ZMM15,
    REGNUM_ZMM16,
    REGNUM_ZMM17,
    REGNUM_ZMM18,
    REGNUM_ZMM19,
    REGNUM_ZMM20,
    REGNUM_ZMM21,
    REGNUM_ZMM22,
    REGNUM_ZMM23,
    REGNUM_ZMM24,
    REGNUM_ZMM25,
    REGNUM_ZMM26,
    REGNUM_ZMM27,
    REGNUM_ZMM28,
    REGNUM_ZMM29,
    REGNUM_ZMM30,
    REGNUM_ZMM31,
    REGNUM_EFLAGS,
    REGNUM_FS,
    REGNUM_GS,
    REGNUM_IGNORED
}
;

#if defined(arch_x86_64)
#define maxGPR 16
#else
#define maxGPR 8
#endif 

#define READ_OP 0
#define WRITE_OP 1

/* operand sizes */
#define byteSzB (1)    /* size of a byte operand */
#define wordSzB (2)    /* size of a word operand */
#define dwordSzB (4)   /* size of a dword operand */
#define qwordSzB (8)   /* size of a qword operand */
#define dqwordSzB (16)   /* size of a double qword (oword) operand */

/* The following values are or'ed together to form an instruction type descriptor */
/* the instruction types of interest */
#define IS_CALL (1<<1)   /* call instruction */
#define IS_RET  (1<<2)   /* near return instruction */
#define IS_RETF (1<<3)   /* far return instruction */
#define IS_JUMP (1<<4)   /* jump instruction */
#define IS_JCC  (1<<5)   /* conditional jump instruction */
#define ILLEGAL (1<<6)   /* illegal instruction */
#define PRVLGD  (1<<7)   /* privileged */
#define IS_RETC (1<<8)   /* return and pop bytes off of stack*/
#define IS_NOP  (1<<9)   /* Nop, Lea--lea is only sometime a return, be sure to double check */

/* addressing modes for calls and jumps */
#define REL_B   (1<<10)  /* relative address, byte offset */
#define REL_W   (1<<11)  /* relative address, word offset */
#define REL_D   (1<<12)  /* relative address, dword offset */
#define REL_X   (1<<13)  /* relative address, word or dword offset */
#define INDIR   (1<<14)  /* indirect (register or memory) address */
#define PTR_WW  (1<<15)  /* 4-byte pointer */
#define PTR_WD  (1<<16)  /* 6-byte pointer */
#define PTR_WX  (1<<17)  /* 4 or 6-byte pointer */
#define REL_D_DATA (1<<18) /* AMD64 RIP-relative data addressing */

/* prefixes */
#define PREFIX_INST   (1<<20) /* instruction prefix */
#define PREFIX_SEG    (1<<21) /* segment override prefix */
#define PREFIX_OPR    (1<<22) /* operand size override */
#define PREFIX_ADDR   (1<<23) /* address size override */
#define PREFIX_REX    (1<<24) /* AMD64 REX prefix */
#define PREFIX_OPCODE (1<<25) /* prefix is part of opcode (SSE) */
#define PREFIX_AVX    (1<<26) /* VEX2 prefix (two byte) */
#define PREFIX_AVX2   (1<<27) /* VEX3 prefix (three byte) */
#define PREFIX_AVX512 (1<<28) /* EVEX prefix (four byte) */

/* end of instruction type descriptor values */

/* opcodes of some one byte opcode instructions */
/* ADD */
#define ADD_EB_GB (0x00)
#define ADD_EV_GV (0x01)
#define ADD_GB_EB (0x02)
#define ADD_GV_EV (0x03)
#define ADD_AL_LB (0x04)
#define ADD_RAX_LZ (0x05)

#define PUSHES    (0x06) /* Invalid in 64 bit mode */
#define POPES     (0x07) /* Invalid in 64 bit mode */

/* OR */
#define OR_EB_GB (0x08)
#define OR_EV_GV (0x09)
#define OR_GB_EB (0x0A)
#define OR_GV_EV (0x0B)
#define OR_AL_LB (0x0C)
#define OR_RAX_LZ (0x0D)

#define PUSHCS    (0x0E) /* Invalid in 64 bit mode */
#define TWO_BYTE_OPCODE (0x0F)

/* ADC */
#define ADC_EB_GB (0x10)
#define ADC_EV_GV (0x11)
#define ADC_GB_EB (0x12)
#define ADC_GV_EV (0x13)
#define ADC_AL_LB (0x14)
#define ADC_RAX_LZ (0x15)

#define PUSHSS    (0x16) /* Invalid in 64 bit mode */
#define POPSS     (0x17) /* Invalid in 64 bit mode */

/* SBB */
#define SBB_EB_GB (0x18)
#define SBB_EV_GV (0x19)
#define SBB_GB_EB (0x1A)
#define SBB_GV_EV (0x1B)
#define SBB_AL_LB (0x1C)
#define SBB_RAX_LZ (0x1D)

#define PUSH_DS  (0x1E) /* Invalid in 64 bit mode */
#define POP_DS   (0X1F) /* Invalid in 64 bit mode */

/* AND */
#define AND_EB_GB (0x20)
#define AND_EV_GV (0x21)
#define AND_GB_EB (0x22)
#define AND_GV_EV (0x23)
#define AND_AL_LB (0x24)
#define AND_RAX_LZ (0x25)

#define SEG_ES (0x26) /* Null prefix in 64-bit mode */
#define DAA    (0x27) /* Invalid in 64-bit mode */

/* SUB */
#define SUB_EB_GB (0x28)
#define SUB_EV_GV (0x29)
#define SUB_GB_EB (0x2A)
#define SUB_GV_EV (0x2B)
#define SUB_AL_LB (0x2C)
#define SUB_RAX_LZ (0x2D)

//(0x2E)
//   (0x2F)

/* XOR */
#define XOR_EB_GB (0x30)
#define XOR_EV_GV (0x31)
#define XOR_GB_EB (0x32)
#define XOR_GV_EV (0x33)
#define XOR_AL_LB (0x34)
#define XOR_RAX_LZ (0x35)

#define XOR_RM16_R16 (0x31)
#define XOR_RM32_R32 (0x31)
#define XOR_R8_RM8 (0x32)
#define XOR_R16_RM16 (0x33)
#define XOR_R32_RM32 (0x33)

#define SEG_SS (0x36) /* Null prefix in 64 bit mode */
#define AAA (0x37)    /* Invalid in 64-bit mode */


/* CMP */
#define CMP_EB_GB (0x38)
#define CMP_EV_GV (0x39)
#define CMP_GB_EB (0x3A)
#define CMP_GV_EV (0x3B)
#define CMP_AL_LB (0x3C)
#define CMP_RAX_LZ (0x3D)

#define TEST_EV_GV (0x85)
//   (0x3E)
//   (0x3F)

/* INC - REX Prefixes in 64 bit mode*/
#define INC_EAX  (0x40)
#define INC_ECX  (0x41)
#define INC_EDX  (0x42)
#define INC_EBX  (0x43)
#define INC_ESP  (0x44)
#define INC_EBP  (0x45)
#define INC_ESI  (0x46)
#define INC_EDI  (0x47)

/* DEC - REX Prefixes in 64 bit mode */
#define DEC_EAX  (0x48)
#define DEC_ECX  (0x49)
#define DEC_EDX  (0x50)
#define DEC_EBX  (0x51)
#define DEC_ESP  (0x52)
#define DEC_EBP  (0x53)
#define DEC_ESI  (0x54)
#define DEC_EDI  (0x55)

/* PUSH */
#define PUSHEAX  (0x50)
#define PUSHECX  (0x51)
#define PUSHEDX  (0x52)
#define PUSHEBX  (0x53)
#define PUSHESP  (0x54)
#define PUSHEBP  (0x55)
#define PUSHESI  (0x56)
#define PUSHEDI  (0x57)

/* POP */
#define POP_EAX  (0x58)
#define POP_ECX  (0x59)
#define POP_EDX  (0x5A)
#define POP_EBX  (0x5b)
#define POP_ESP  (0x5c)
#define POP_EBP  (0x5d)
#define POP_EBI  (0x5e)
#define POP_EDI  (0x5f)

#define PUSHAD   (0x60)
#define POPAD    (0x61)

#define JE_R8    (0x74)
#define JNE_R8   (0x75)
#define JL_R8    (0x7C)
#define JLE_R8   (0x7E)
#define JG_R8    (0x7F)
#define JGE_R8   (0x7D)

#define JB_R8    (0x72)
#define JBE_R8   (0x76)
#define JA_R8    (0x77)
#define JAE_R8   (0x73)

#define MOVREGMEM_REG (0x8b) 
#define MOV_R8_TO_RM8 (0x88)     //move r8 to r/m8
#define MOV_R16_TO_RM16 (0x89)   //move r16 to r/m16
#define MOV_R32_TO_RM32 (0x89)   //move r32 to r/m32
#define MOV_RM8_TO_R8 (0x8A)
#define MOV_RM16_TO_R16 (0x8b)
#define MOV_RM32_TO_R32 (0x8b)

#define NOP      (0x90)
#define PUSHFD   (0x9C)
#define POPFD    (0x9D)

#define JCXZ     (0xE3)

#define FSAVE    (0x9BDD)
#define FSAVE_OP (6)

#define FRSTOR   (0xDD)
#define FRSTOR_OP (4)

const unsigned char SYSCALL[] = {0x0F, 0x05};

/* limits */
#define MIN_IMM8 (-128)
#define MAX_IMM8 (127)
#define MIN_IMM16 (-32768)
#define MAX_IMM16 (32767)

// Size of floating point information saved by FSAVE
#define FSAVE_STATE_SIZE 108

// Prefix groups
enum {
  RepGroup = 0
};

#ifndef VEX_PREFIX_MASKS
#define VEX_PREFIX_MASKS

/**
 * Enum that differentiates different types of VEX prefixed instructions.
 * This is also used as the demultiplexer for the sseVexMult table so the
 * bindings should not be changed.
 */
enum VEX_TYPE
{
    VEX_TYPE_NONE=0, VEX_TYPE_VEX2, VEX_TYPE_VEX3, VEX_TYPE_EVEX
};

/* Masks to help decode vex prefixes */

/** VEX 3 masks (2nd byte) */
#define PREFIX_VEX3 ((unsigned char)0xC4)
#define PREFIX_VEX2 ((unsigned char)0xC5)
#define VEX3_REXX   (1 << 6)
#define VEX3_REXB   (1 << 5)
#define VEX3_M      ((1 << 5) - 1)
#define VEX3_W      (1 << 7)

/* VEX2 and VEX3 share these bits on their final byte  */
#define VEX_VVVV   (((1 << 4) - 1) << 3)
#define VEX_L      (1 << 2)
#define VEX_PP     (0x03)
#define VEX_REXR   (1 << 7)

/* VEX mask helper macros */
#define VEXGET_VVVV(b)  (unsigned char)((~((unsigned char) \
            ((b & VEX_VVVV) >> 3))) & 0xF)

#define VEXGET_L(b)     (char)((b & VEX_L) >> 2)
#define VEXGET_R(b)     (((unsigned char)((~b & (1 << 7)) >> 7)) & 0x01)
#define VEXGET_PP(b)    (char)(b & VEX_PP)

#define VEX3GET_W(b)    (char)((b & VEX3_W) >> 7)
#define VEX3GET_M(b)    (b & VEX3_M)
#define VEX3GET_X(b)    (((unsigned char)(~b) & (unsigned char)(1 << 6)) >> 6)
#define VEX3GET_B(b)    (((unsigned char)(~b) & (unsigned char)(1 << 5)) >> 5)
#define VEX3GET_M(b)    (b & VEX3_M)

/** EVEX masks */
#define PREFIX_EVEX ((unsigned char)0x62)

#define EVEXGET_W(b) VEX3GET_W(b)
#define EVEXGET_L1(b) (unsigned char)((1 << 5) & (b))
#define EVEXGET_L2(b) (unsigned char)((1 << 6) & (b))
#define EVEXGET_LL(b) (unsigned char)(((b) >> 5) & 0x03)
#define EVEXGET_PP(b) (unsigned char)(3 & (b))
#define EVEXGET_MM(b) (unsigned char)(3 & (b))
#define EVEXGET_AAA(b) (unsigned char)(7 & (b))
#define EVEXGET_r(b) (((unsigned char)(~b) & (unsigned char)(1 << 7)) >> 7)
#define EVEXGET_R(b) (((unsigned char)(~b) & (unsigned char)(1 << 4)) >> 4)
#define EVEXGET_b(b) (((unsigned char)(~b) & (unsigned char)(1 << 5)) >> 5)
#define EVEXGET_x(b) (((unsigned char)(~b) & (unsigned char)(1 << 6)) >> 6)

#define EVEXGET_VVVV(a, b)  ((((unsigned char)~(a) >> 3) & 0x0F)) | \
            (((unsigned char)~(b) & 0x08) << 1)
#define EVEXGET_V(b) (((unsigned char)~(b) & 0x08) << 1)

#endif

#ifndef PREFIX_LOCK
#define PREFIX_LOCK   (unsigned char)(0xF0)
#define PREFIX_REPNZ  (unsigned char)(0xF2)
#define PREFIX_REP    (unsigned char)(0xF3)

#define PREFIX_SEGCS  (unsigned char)(0x2E)
#define PREFIX_SEGSS  (unsigned char)(0x36)
#define PREFIX_SEGDS  (unsigned char)(0x3E)
#define PREFIX_SEGES  (unsigned char)(0x26)
#define PREFIX_SEGFS  (unsigned char)(0x64)
#define PREFIX_SEGGS  (unsigned char)(0x65)

#define PREFIX_XOP  (unsigned char)(0x8F)

#define PREFIX_BRANCH0 (unsigned char)(0x2E)
#define PREFIX_BRANCH1 (unsigned char)(0x3E)

#define PREFIX_SZOPER  (unsigned char)(0x66)
#define PREFIX_SZADDR  (unsigned char)(0x67)
#endif

COMMON_EXPORT void ia32_set_mode_64(bool mode);

/**
 * AVX/AVX2/EVEX addressing modes (not in manual).
 *
 * am_HK operand is an EVEX masking register (k0 - k7) which is specified
 *        using the EVEX.vvvv bits.
 * am_VK the reg field of the R/M byte specifies an EVEX masking register.
 * am_WK the R/M field of the R/M byte specifies an EVEX masking register.
 *
 * am_XH same as am_H except the register is constrained to an XMM register,
 *        reguardless of the VEX.L field.
 * am_XV same as am_V except the register is contrained to an XMM register,
 *        reguardless of the VEX.L field.
 * am_XW same as am_W except the register is constrained to an XMM register,
 *        reguardless of the VEX.L field.
 *
 * am_YH same as am_H except the register is constrained to either an XMM
 *        or YMM register, based on the VEX.L bits field.
 * am_YV same as am_V except the register is constrained to either an XMM
 *        or YMM register, based on the VEX.L bits field.
 * am_YW same as am_W except the register is constrained to either an XMM
 *        or YMM register, based on the VEX.L bits field.
 */

// addressing methods (see appendix A-2)
// I've added am_reg (for registers implicitely encoded in instruciton), 
// and am_stackX for stack operands [this kinda' messy since there are actually two operands:
// the stack byte/word/dword and the (E)SP register itself - but is better than naught]
// added: am_reg, am_stack, am_allgprs
// ADDED: am_ImplImm for implicit immediates
// ADDED: am_RM, am_UM,
enum { am_A=1, am_B, am_C, am_D, am_E, am_F, am_G, am_H, am_I, am_J, // 1 -> 10
	am_L, am_M, am_N, am_O, am_P, am_Q, am_R, am_S, am_T, am_XU, am_YU,  // 11 -> 20
	am_U, am_UM, am_V, am_W, am_X, am_Y, am_reg, am_stackH, am_stackP, am_allgprs,
	am_tworeghack, am_ImplImm, am_RM, am_HK, am_VK, am_WK, am_XH, am_XV, am_XW, am_YH, 
	am_YV, am_YW }; // pusH and poP produce different addresses

// operand types - idem, but I invented quite a few to make implicit operands explicit.
// ADDED: op_y
enum { op_a=1, op_b, op_c, op_d, op_dq, op_p, op_pd, op_pi, op_ps, op_q, // 10
       op_qq, op_s, op_sd, op_ss, op_si, op_v, op_w, op_y, op_z, op_lea, op_allgprs, op_512,
       op_f, op_dbl, op_14, op_28, op_edxeax, op_ecxebx};


// tables and pseudotables
enum {
  t_ill=0, t_oneB, t_twoB, t_threeB, t_threeB2, t_prefixedSSE, t_coprocEsc, 
  t_grp, t_sse, t_sse_mult, t_sse_bis, t_sse_bis_mult, 
  t_sse_ter, t_sse_ter_mult, t_grpsse, t_3dnow, t_vexl, t_vexw, t_sse_vex_mult, t_fma4,
  t_xop_8_w, t_xop_9_w,
  t_done=99
};

// registers used for memory access
enum { mRAX=0, mRCX, mRDX, mRBX,
       mRSP, mRBP, mRSI, mRDI,
       mR8, mR9, mR10, mR11, 
       mR12,mR13, MR14, mR15, mRIP };

enum { mEAX=0, mECX, mEDX, mEBX,
       mESP, mEBP, mESI, mEDI };

enum { mAX=0, mCX, mDX, mBX,
       mSP, mBP, mSI, mDI };


// operand semantic - these make explicit all the implicit stuff in the Intel tables
// they are needed for memory access, but may be useful for other things: dataflow etc.
// Instructions that do not deal with memory are not tested, so caveat emptor...
// Also note that the stack is never specified as an operand in Intel tables, so it
// has to be dealt with here.

enum { sNONE=0, // the instruction does something that cannot be classified as read/write (by me)
       s1R,     // reads one operand, e.g. jumps
       s1W,     // e.g. lea
       s1RW,    // one operand read and written, e.g. inc
       s1R2R,   // reads two operands, e.g. cmp
       s1W2R,   // second operand read, first operand written (e.g. mov)
       s1RW2R,  // two operands read, first written (e.g. add)
       s1RW2RW, // e.g. xchg
       s1W2R3R, // e.g. imul
       s1W2W3R, // e.g. les
       s1W2RW3R, // some mul
       s1R2RW, // (stack) push
       s1W2RW, // pop
       s1W2R3RW, // additional push/pop
       s1RW2R3R, // shld/shrd
       s1RW2RW3R, // [i]div, cmpxch8b
       s1RW2R3RW, // v[p]gather[ps, pd, qq, qd]
       s1R2R3R,

/* Only 4 operands below here */
       s1W2R3R4R,
       s1RW2R3R4R 
}; // should be strictly less than 2^17 otherwise adjust FPOS in arch-x86.C

/* This should equal the first operand semantic where 4 operands are used. */
#define s4OP s1W2R3R4R

/* Implicit operand specifier */
#define s1I (1 << 16) /* 1st operand is implicit */
#define s2I (1 << 17) /* 2nd operand is implicit */
#define s3I (1 << 18) /* 3rd operand is implicit */
#define s4I (1 << 19) /* 4th operand is implicit */

/* Implicit mask getters */
#define sGetImplicitOP1(b) ((b) & s1I)
#define sGetImplicitOP2(b) ((b) & s2I)
#define sGetImplicitOP3(b) ((b) & s3I)
#define sGetImplicitOP4(b) ((b) & s4I)
#define sGetImplicitOPs(b) ((b) & 0xFFFF0000)
#define sGetImplicitOP(b, i) ((b) & (1 << (16 + (i))))

/* Implicit mask setters */
#define sSetImplicitOP1(b) ((b) | s1I)
#define sSetImplicitOP2(b) ((b) | s12)
#define sSetImplicitOP3(b) ((b) | s13)
#define sSetImplicitOP4(b) ((b) | s14)

/* Instruction decoration descriptors */
#define sGetDecoration(b) ((b) & 0xFFFF)
#define sSetDecoration(b, dec) (((b) & ~0xFFFF) | (dec))

enum { 
    s1D = 1, /* Take decoration from 1st operand */
    s1D2D /* Take decoration from 1st, 2nd operand in that order */
};

/* Masks */
#define FPOS 17

struct modRMByte {
  unsigned mod : 2;
  unsigned reg : 3;
  unsigned rm  : 3;
};

struct sIBByte {
  unsigned scale : 2;
  unsigned index : 3;
  unsigned base  : 3;
};

class ia32_instruction;

class ia32_prefixes
{
  friend COMMON_EXPORT bool ia32_decode_prefixes(const unsigned char *addr, ia32_instruction &insn, bool mode_64);
  friend bool ia32_decode_rex(const unsigned char* addr, ia32_prefixes&,
                              ia32_locations *loc);
 private:
  unsigned int count;
  // At most 4 prefixes are allowed for Intel 32-bit CPUs
  // There also 4 groups, so this array is 0 if no prefix
  // from that group is present, otherwise it contains the
  // prefix opcode
  // For 64-bit CPUs, an additional REX prefix is possible,
  // so this array is extended to 5 elements
  unsigned char prfx[5];
  unsigned char opcode_prefix;
  
 public:
  unsigned int getCount() const { return count; }
  unsigned char getPrefix(unsigned char group) const { assert(group <= 4); return prfx[group]; }
  bool rexW() const { return prfx[4] & 0x8; }
  bool rexR() const { return prfx[4] & 0x4; }
  bool rexX() const { return prfx[4] & 0x2; }
  bool rexB() const { return prfx[4] & 0x1; }
  unsigned char getOpcodePrefix() const { return opcode_prefix; }
  unsigned char getAddrSzPrefix() const { return prfx[3]; }
  unsigned char getOperSzPrefix() const { return prfx[2]; }

  /* Because VEX fields are based on the VEX type, they are decoded immediately. */
  bool vex_present; /* Does this instruction have a vex prefix?  */
  bool XOP;  /* whether this instrucxtion is an XOP instrucxtion */
  VEX_TYPE vex_type; /* If there is a vex prefix present, what type is it? */
  unsigned char vex_prefix[5]; /* Support up to EVEX (VEX-512) */
  int vex_sse_mult; /* index for sse multiplexer table */
  int vex_vvvv_reg; /* The register specified by this prefix. */
  int vex_ll; /* l bit for VEX2, VEX3 or ll for EVEX */
  int vex_pp; /* pp bits for VEX2, VEX3 or EVEX */
  int vex_m_mmmm; /* m-mmmm bits for VEX2, VEX3 or EVEX */
  int vex_w; /* w bit for VEX2, VEX3 or EVEX */
  int vex_V; /* V' modifier for EVEX */
  int vex_r; /* The VEX REXR bit for VEX2, VEX3 or EVEX*/
  int vex_R; /* The VEX REXR' bit for EVEX */
  int vex_x; /* The VEX REXX bit for VEX2, VEX3 or EVEX */
  int vex_b; /* The VEX REXB bit for VEX2, VEX3 or EVEX */
  int vex_aaa; /* Selects the vector mask register for EVEX */
};

// helper routine to tack-on rex bit when needed
inline int apply_rex_bit(int reg, bool rex_bit)
{
    if (rex_bit)
	return reg + 8;
    else
	return reg;
}

//VG(6/20/02): To support Paradyn without forcing it to include BPatch_memoryAccess, we
//             define this IA-32 "specific" class to encapsulate the same info - yuck

struct ia32_memacc
{
  bool is;
  bool read;
  bool write;
  bool nt;     // non-temporal, e.g. movntq...
  bool prefetch;

  int addr_size; // size of address in 16-bit words
  long imm;
  int scale;
  int regs[2]; // register encodings (in ISA order): 0-7
               // (E)AX, (E)CX, (E)DX, (E)BX
               // (E)SP, (E)BP, (E)SI, (E)DI

  int size;
  int sizehack;  // register (E)CX or string based
  int prefetchlvl; // prefetch level
  int prefetchstt; // prefetch state (AMD)

  ia32_memacc() : is(false), read(false), write(false), nt(false), 
       prefetch(false), addr_size(2), imm(0), scale(0), size(0), sizehack(0),
       prefetchlvl(-1), prefetchstt(-1)
  {
    regs[0] = -1;
    regs[1] = -1;
  }

  void set16(int reg0, int reg1, long disp)
  { 
    is = true;
    addr_size  = 1; 
    regs[0] = reg0; 
    regs[1] = reg1; 
    imm     = disp;
  }

  void set(int reg, long disp, int addr_sz)
  { 
    is = true;
    addr_size = addr_sz;
    regs[0] = reg; 
    imm     = disp;
  }

  void set_sib(int base, int scal, int indx, long disp, int addr_sz)
  {
    is = true;
    addr_size = addr_sz;
    regs[0] = base;
    regs[1] = indx;
    scale   = scal;
    imm     = disp;
  }

  void setXY(int reg, int _size, int _addr_size)
  {
    is = true;
    regs[0] = reg;
    size = _size;
    addr_size = _addr_size;
  }

  void print();
};

enum sizehacks {
  shREP=1,
  shREPECMPS,
  shREPESCAS,
  shREPNECMPS,
  shREPNESCAS
};

struct ia32_condition
{
  bool is;
  // TODO: add a field/hack for ECX [not needed for CMOVcc, but for Jcc]
  int tttn;

  ia32_condition() : is(false), tttn(-1) {}
  void set(int _tttn) { is = true; tttn = _tttn; }
};

struct ia32_operand {  // operand as given in Intel book tables
  unsigned int admet;  // addressing method
  unsigned int optype; // operand type;
};

// An instruction table entry
struct ia32_entry {
  COMMON_EXPORT const char* name(ia32_locations* locs = NULL);
  COMMON_EXPORT entryID getID(ia32_locations* locs = NULL) const;
  // returns true if any flags are read/written, false otherwise
  COMMON_EXPORT bool flagsUsed(std::set<Dyninst::MachRegister>& flagsRead, std::set<Dyninst::MachRegister>& flagsWritten,
		 ia32_locations* locs = NULL);
  entryID id;
  unsigned int otable;       // which opcode table is next; if t_done it is the current one
  unsigned char tabidx;      // at what index to look, 0 if it easy to deduce from opcode
  bool hasModRM;             // true if the instruction has a MOD/RM byte
  ia32_operand operands[4];  // operand descriptors
  unsigned int legacyType;   // legacy type of the instruction (e.g. (IS_CALL | REL_W))
  // code to decode memory access - this field should be seen as two 16 bit fields
  // the lower half gives operand semantics, e.g. s1RW2R, the upper half is a fXXX hack if needed
  // before hating me for this: it takes a LOT less time to add ONE field to ~2000 table lines!
  // The upper 3 bits of this field (bits 29, 30, 31) are specifiers for implicit operands.
  unsigned int opsema;  

  unsigned int impl_dec; /* Implicit operands and decoration descriptions */
};

using std::vector;
struct flagInfo
{
  flagInfo(const vector<Dyninst::MachRegister>& rf, const vector<Dyninst::MachRegister>& wf) : readFlags(rf), writtenFlags(wf)
  {
  }
  flagInfo() 
  {
  }
  
  vector<Dyninst::MachRegister> readFlags;
  vector<Dyninst::MachRegister> writtenFlags;
};

class ia32_instruction
{
  friend unsigned int ia32_decode_operands (const ia32_prefixes& pref, 
                                            const ia32_entry& gotit, 
                                            const char* addr, 
                                            ia32_instruction& instruct);
  friend COMMON_EXPORT bool ia32_decode_prefixes(const unsigned char *addr, ia32_instruction &insn, bool mode_64);
  friend COMMON_EXPORT ia32_instruction &
  ia32_decode(unsigned int capa, const unsigned char *addr, ia32_instruction &instruct, bool mode_64);
  friend COMMON_EXPORT int
  ia32_decode_opcode(unsigned int capa, const unsigned char *addr, ia32_instruction &instruct, ia32_entry **gotit_ret,
                       bool mode_64);
  friend unsigned int ia32_decode_operands(const ia32_prefixes &pref, const ia32_entry &gotit, const unsigned char *addr,
                                             ia32_instruction &instruct, ia32_memacc *mac, bool mode_64);
  friend ia32_instruction& ia32_decode_FP(const ia32_prefixes& pref, const unsigned char* addr,
                                          ia32_instruction& instruct);
  friend unsigned int ia32_emulate_old_type(ia32_instruction &instruct, bool mode_64);
  friend ia32_instruction &
  ia32_decode_FP(unsigned int opcode, const ia32_prefixes &pref, const unsigned char *addr, ia32_instruction &instruct,
                   ia32_entry *entry, ia32_memacc *mac, bool mode_64);

  unsigned int   size;
  ia32_prefixes  prf;
  ia32_memacc    *mac;
  ia32_condition *cond;
  ia32_entry     *entry;
  ia32_locations *loc;
  unsigned int   legacy_type;
  bool           rip_relative_data;


 public:
  ia32_instruction(ia32_memacc* _mac = NULL, ia32_condition* _cnd = NULL,
                   ia32_locations *loc_ = NULL)
    : size(0), prf(), mac(_mac), cond(_cnd), entry(NULL), loc(loc_),
      legacy_type(0), rip_relative_data(false)
  {}

  ia32_entry * getEntry() { return entry; }
  unsigned int getSize() const { return size; }
  unsigned int getPrefixCount() const { return prf.getCount(); }
  ia32_prefixes * getPrefix() { return &prf; }
  unsigned int getLegacyType() const { return legacy_type; }
  bool hasRipRelativeData() const { return rip_relative_data; }
  const ia32_memacc& getMac(int which) const { return mac[which]; }
  const ia32_condition& getCond() const { return *cond; }
  const ia32_locations& getLocationInfo() const { return *loc; }

  COMMON_EXPORT static dyn_hash_map<entryID, flagInfo> const& getFlagTable();
  static void initFlagTable(dyn_hash_map<entryID, flagInfo>&);
private:
    static dyn_hash_map<entryID, flagInfo> flagTable;
  
};

// VG(02/07/2002): Information that the decoder can return is
//   #defined below. The decoder always returns the size of the 
//   instruction because that has to be determined anyway.
//   Please don't add things that should be external to the
//   decoder, e.g.: how may bytes a relocated instruction needs
//   IMHO that stuff should go into inst-x86...

#define IA32_DECODE_PREFIXES	(1<<0)
#define IA32_DECODE_MNEMONICS	(1<<1)
#define IA32_DECODE_OPERANDS	(1<<2)
#define IA32_DECODE_JMPS	    (1<<3)
#define IA32_DECODE_MEMACCESS	(1<<4)
#define IA32_DECODE_CONDITION 	(1<<5)

#define IA32_FULL_DECODER (IA32_DECODE_PREFIXES \
        | IA32_DECODE_MNEMONICS \
        | IA32_DECODE_OPERANDS \
        | IA32_DECODE_JMPS \
        | IA32_DECODE_MEMACCESS \
        | IA32_DECODE_CONDITION)
#define IA32_SIZE_DECODER 0

/* TODO: documentation*/
COMMON_EXPORT bool ia32_decode_prefixes(const unsigned char *addr, ia32_instruction &insn, bool mode_64);

/**
 * Decode just the opcode of the given instruction. This implies that
 * ia32_decode_prefixes has already been called on the given instruction
 * and addr has been moved past the prefix bytes. Returns zero on success,
 * non zero otherwise.
 */
COMMON_EXPORT int
ia32_decode_opcode(unsigned int capa, const unsigned char *addr, ia32_instruction &instruct, ia32_entry **gotit_ret,
                   bool mode_64);

/**
 * Do a complete decoding of the instruction at the given address. This
 * function calls ia32_decode_prefixes, ia32_decode_opcode and
 * ia32_decode_operands. Returns zero on success, non zero otherwise.
 * When there is a decoding failure, the state of the given instruction
 * is not defined. capabilities is a mask of the above flags (IA32_DECODE_*).
 * The mask determines what part of the instruction should be decoded.
 */
COMMON_EXPORT ia32_instruction &
ia32_decode(unsigned int capabilities, const unsigned char *addr, ia32_instruction &, bool mode_64);


enum dynamic_call_address_mode {
  REGISTER_DIRECT, REGISTER_INDIRECT,
  REGISTER_INDIRECT_DISPLACED, SIB, DISPLACED, 
  IP_INDIRECT_DISPLACED
};

/*
   get_instruction: get the instruction that starts at instr.
   return the size of the instruction and set instType to a type descriptor
*/
COMMON_EXPORT unsigned
get_instruction(const unsigned char *instr, unsigned &instType, const unsigned char **op_ptr, bool mode_64);

/* get the target of a jump or call */
COMMON_EXPORT Dyninst::Address get_target(const unsigned char *instr, unsigned type, unsigned size,
		   Dyninst::Address addr);

// Size of a jump rel32 instruction
#define JUMP_REL32_SZ (6)
// Maxium size of an emitted jump
#define JUMP_SZ (5)
// Size of a call rel32 instruction
#define CALL_REL32_SZ (5)
// >2gb displacement in 32 bit mode
#define CALL_ABS32_SZ (11)
#define JUMP_ABS32_SZ (6)
// Max size of a relocated thunk call
#define CALL_RELOC_THUNK (13)

#if defined(arch_x86_64)
// size of instruction seqeunce to get anywhere in address space
// without touching any registers
//#define JUMP_ABS64_SZ (17)
#define JUMP_ABS64_SZ (14)
// Jump is push/return; call is push/push/return, so subtract a return
#define CALL_ABS64_SZ (JUMP_ABS64_SZ+JUMP_ABS64_SZ-1)
#endif

#define PUSH_RM_OPC1 (0xFF)
#define PUSH_RM_OPC2 (6)
#define CALL_RM_OPC1 (0xFF)
#define CALL_RM_OPC2 (2)
#define JUMP_RM_OPC1 (0xFF)
#define JUMP_RM_OPC2 (4)
#define PUSH_EBP (0x50+REGNUM_EBP)
#define SUB_REG_IMM32 (5)
#define LEAVE (0xC9)

#define EXTENDED_0x81_ADD 0
#define EXTENDED_0x81_OR 1
#define EXTENDED_0x81_ADDC 2
#define EXTENDED_0x81_SHIFT 3
#define EXTENDED_0x81_AND 4
#define EXTENDED_0x81_SUB 5
#define EXTENDED_0x81_XOR 6
#define EXTENDED_0x81_CMP 7
#define EXTENDED_0x83_AND 4

unsigned int swapBytesIfNeeded(unsigned int i);

class instruction {
 public:
    instruction(): type_(0), size_(0), ptr_(0), op_ptr_(0) {}

  instruction(const unsigned char *p, unsigned type, unsigned sz, const unsigned char* op = 0):
      type_(type), size_(sz), ptr_(p), op_ptr_(op ? op : p) {}

  instruction(const instruction &insn)
  {
    type_ = insn.type_;
    size_ = insn.size_;
    ptr_ = insn.ptr_;
    op_ptr_ = insn.op_ptr_;
  }
  
  COMMON_EXPORT instruction *copy() const;

  instruction(const void *ptr, bool mode_64) :
      type_(0), size_(0), ptr_(NULL), op_ptr_(0) {
      setInstruction((const unsigned char *) ptr, 0, mode_64);
  }

  unsigned setInstruction(const unsigned char *p, Dyninst::Address, bool mode_64) {
      ptr_ = p;
      size_ = get_instruction(ptr_, type_, &op_ptr_, mode_64);
      return size_;
  }

  // if the instruction is a jump or call, return the target, else return zero
  Dyninst::Address getTarget(Dyninst::Address addr) const {
    return (Dyninst::Address)get_target(ptr_, type_, size_, addr);
  }

  // return the size of the instruction in bytes
  unsigned size() const { return size_; }

  // And the size necessary to reproduce this instruction
  // at some random point.
  COMMON_EXPORT unsigned spaceToRelocate() const;

  // return the type of the instruction
  unsigned type() const { return type_; }

  // return a pointer to the instruction
  const unsigned char *ptr() const { 
      return ptr_; 
  }

  // return a pointer to the instruction's opcode
  const unsigned char* op_ptr() const { return op_ptr_; }

  // Function relocation...
  static unsigned maxInterFunctionJumpSize(unsigned addr_width) { return maxJumpSize(addr_width); }

  // And tell us how much space we'll need...
  COMMON_EXPORT static unsigned jumpSize(Dyninst::Address from, Dyninst::Address to, unsigned addr_width);
  COMMON_EXPORT static unsigned jumpSize(long disp, unsigned addr_width);
  COMMON_EXPORT static unsigned maxJumpSize(unsigned addr_width);

    bool isCall() const { return type_ & IS_CALL; }
  bool isCallIndir() const { return (type_ & IS_CALL) && (type_ & INDIR); }
  bool isReturn() const { return (type_ & IS_RET) || (type_ & IS_RETF); }
  bool isRetFar() const { return type_ & IS_RETF; }
  bool isCleaningRet() const {return type_ & IS_RETC; }
  bool isJumpIndir() const { return (type_ & IS_JUMP) && (type_ & INDIR); }
  bool isJumpDir() const
    { return !(type_ & INDIR) && ((type_ & IS_JUMP) || (type_ & IS_JCC)); }
  bool isUncondJump() const
    { return ((type_ & IS_JUMP) && !(type_ & IS_JCC)); }

    bool isIndir() const { return type_ & INDIR; }
  bool isIllegal() const { return type_ & ILLEGAL; }
  bool isLeave() const { return *ptr_ == 0xC9; }  
  bool isPrivileged() const { return (type_ & PRVLGD); }
  bool isMoveRegMemToRegMem() const 
    { const unsigned char* p = op_ptr_ ? op_ptr_ : ptr_;
      return *p == MOV_R8_TO_RM8   || *p == MOV_R16_TO_RM16 ||
             *p == MOV_R32_TO_RM32 || *p ==  MOV_RM8_TO_R8  ||
             *p == MOV_RM16_TO_R16 || *p == MOV_RM32_TO_R32;   }
  bool isXORRegMemRegMem() const
      { const unsigned char* p = op_ptr_ ? op_ptr_ : ptr_;
        return
               *p == XOR_RM16_R16 ||
#if !defined(XOR_RM16_R16) || !defined(XOR_RM32_R32) || (XOR_RM16_R16) != (XOR_RM32_R32)
// XOR_RM16_R16 and XOR_RM32_R32 are macros with the same value so disable this
// clause as it is unnecessary and produces a compiler warning
               *p ==  XOR_RM32_R32 ||
#endif
               *p ==  XOR_R8_RM8  || *p ==  XOR_R16_RM16 ||
               *p == XOR_R32_RM32; }
  bool isANearBranch() const { return isJumpDir(); }

  bool isTrueCallInsn() const { return (isCall() && !isCallIndir()); }
  bool isSysCallInsn() const { return op_ptr_[0] == SYSCALL[0] &&
                                   op_ptr_[1] == SYSCALL[1]; }

  static bool isAligned(const Dyninst::Address ) { return true; }

    void print()
  {
      for (unsigned i = 0; i < size_; i++)
	  fprintf(stderr, " %02x", *(ptr_ + i));
      fprintf(stderr, "\n");
  }

private:
  unsigned type_;   // type of the instruction (e.g. IS_CALL | INDIR)
  unsigned size_;   // size in bytes
  const unsigned char *ptr_;       // pointer to the instruction
  const unsigned char *op_ptr_;    // pointer to the opcode
};

/** Only appropriate for call/jump functions **/
    int set_disp(bool setDisp, instruction *insn, int newOffset, bool outOfFunc);
int displacement(const unsigned char *instr, unsigned type);

/** Returns the immediate operand of an instruction **/

    COMMON_EXPORT int count_prefixes(unsigned insnType);

inline bool is_disp8(long disp) {
   return (disp >= -128 && disp < 127);
}

inline bool is_disp16(long disp) {
   return (disp >= -32768 && disp < 32767);
}

inline bool is_disp32(long disp) {
  return (disp <= INT32_MAX && disp >= INT32_MIN);
}
inline bool is_disp32(Dyninst::Address a1, Dyninst::Address a2) {
  return is_disp32(a2 - (a1 + JUMP_REL32_SZ));
}
inline bool is_addr32(Dyninst::Address addr) {
    return (addr < UINT32_MAX);
}

COMMON_EXPORT void decode_SIB(unsigned sib, unsigned& scale, 
        Dyninst::Register& index_reg, Dyninst::Register& base_reg);
COMMON_EXPORT const unsigned char* skip_headers(const unsigned char*, 
        ia32_instruction* = NULL);

/* addresses on x86 don't have to be aligned */
/* Address bounds of new dynamic heap segments.  On x86 we don't try
to allocate new segments near base tramps, so heap segments can be
allocated anywhere (the tramp address "x" is ignored). */
inline Dyninst::Address region_lo(const Dyninst::Address /*x*/) { return 0x00000000; }
inline Dyninst::Address region_hi(const Dyninst::Address /*x*/) { return 0xf0000000; }

#if defined(arch_x86_64)
// range functions for AMD64

inline Dyninst::Address region_lo_64(const Dyninst::Address x) { return x & 0xffffffff80000000; }
inline Dyninst::Address region_hi_64(const Dyninst::Address x) { return x | 0x000000007fffffff; }

#endif

COMMON_EXPORT bool insn_hasSIB(unsigned,unsigned&,unsigned&,unsigned&);
COMMON_EXPORT bool insn_hasDisp8(unsigned ModRM);
COMMON_EXPORT bool insn_hasDisp32(unsigned ModRM);

COMMON_EXPORT bool isStackFramePrecheck_msvs( const unsigned char *buffer );
COMMON_EXPORT bool isStackFramePrecheck_gcc( const unsigned char *buffer );

} // namespace arch_x86

#endif

