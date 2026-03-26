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

#ifndef DYNINST_COMMON_ENCODING_X86_H
#define DYNINST_COMMON_ENCODING_X86_H

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

/* XOR */
#define XOR_RM16_R16 (0x31)
#define XOR_RM32_R32 (0x31)
#define XOR_R8_RM8 (0x32)
#define XOR_R16_RM16 (0x33)
#define XOR_R32_RM32 (0x33)

/* CMP */
#define CMP_GV_EV (0x3B)

/* TEST */
#define TEST_EV_GV (0x85)

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

const unsigned char SYSCALL[] = {0x0F, 0x05};

/* limits */
#define MAX_IMM8 (127)

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

void ia32_set_mode_64(bool mode);

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

// Check AMD64 APM appendix A
// operand types - idem, but I invented quite a few to make implicit operands explicit.
// ADDED: op_y
// ADDED: op_o // octoword (128 bits), irrespective of the effective operand size
enum { op_a=1, op_b, op_c, op_d, op_dq, op_o, op_p, op_pd, op_pi, op_ps, op_q, // 10
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

/* Implicit mask getters */
#define sGetImplicitOPs(b) ((b) & 0xFFFF0000)
#define sGetImplicitOP(b, i) ((b) & (1 << (16 + (i))))

/* Masks */
#define FPOS 17

#define EXTENDED_0x81_ADD 0
#define EXTENDED_0x83_AND 4

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

#if defined(DYNINST_CODEGEN_ARCH_X86_64)
// size of instruction seqeunce to get anywhere in address space
// without touching any registers
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

#endif
