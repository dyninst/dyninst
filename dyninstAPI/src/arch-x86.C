/*
 * Copyright (c) 1996 Barton P. Miller
 * 
 * We provide the Paradyn Parallel Performance Tools (below
 * described as Paradyn") on an AS IS basis, and do not warrant its
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

// $Id: arch-x86.C,v 1.16 2002/06/07 20:17:54 gaburici Exp $
// x86 instruction decoder

#include <assert.h>
#include <stdio.h>
#include "common/h/Types.h"
#include "arch-x86.h"
#include "arch-ia32.h"

// opcode descriptors (from the Pentium manual)
enum {
  Grp1=0, Grp2, Grp3a, Grp3b, Grp4, Grp5, Grp6, Grp7, Grp8, Grp9, 
  oneB, twoB, coprocEsc,
  PREFIX_SEG_OVR, PREFIX_OPR_SZ, PREFIX_ADDR_SZ, PREFIX_INSTR,
  Ill
};

// operand descriptor (from the Pentium manual)
enum {
  noOperand = 0,
  const1,
  CS, DS, ES, FS, GS, SS,
  AL, AH, BL, BH, CL, CH, DL, DH,
  AX, BX, CX, DX, SI, DI, BP, SP,
  eAX, eBX, eCX, eDX, eSI, eDI, eBP, eSP,
  rEAX, rEBX, rECX, rEDX, rESI, rEDI, rEBP, rESP,

  Ap,
  Cd,
  Dd,
  Eb, Ep, Ev, Ew,
  Fv,
  Gb, Gv, Gw,
  Ib, Iv, Iw,
  Jb, Jv,
  M, Ma, Mp, Mq, Ms,
  Ob, Ov,
  Rd,
  Sw, 
  Xb, Xv,
  Yb, Yv
};

// an entry of the opcode tables map
struct x86_insn {
  char *name;              // name of the instruction (for debbuging only)
  unsigned escape_code;    // which opcode tables to look at (eg. oneB, twoB, etc)
  bool hasModRM;           // true if the instruction has a MOD/RM byte
  unsigned operands[3];    // operand descriptos
  unsigned insnType;       // the type of the instruction (e.g. (IS_CALL | REL_W))
};


/* groupMap: opcodes determined by bits 5,4,3 of the MOD/RM byte.
   For some instructions in this table, the operands are defined in the
   one or two byte opcode map, for others the operands are defined here.
   Get operands from one or two byte map: Grp1, Grp2, Grp8
   Get operands from ModRM map: Grp3a, Grp3b, Grp4, Grp5, Grp6, Grp7, Grp 9

*/
static x86_insn groupMap[10][8] = {
 { /* group 1 - only opcode is defined here,
      operands are defined in the one or two byte maps below */
  { "add", 0, true, { 0, 0, 0 }, 0 },
  { "or", 0, true, { 0, 0, 0 }, 0 },
  { "adc", 0, true, { 0, 0, 0 }, 0 },
  { "sbb", 0, true, { 0, 0, 0 }, 0 },
  { "and", 0, true, { 0, 0, 0 }, 0 },
  { "sub", 0, true, { 0, 0, 0 }, 0 },
  { "xor", 0, true, { 0, 0, 0 }, 0 },
  { "cmp", 0, true, { 0, 0, 0 }, 0 }
 },

 {  /* group 2 - only opcode is defined here, 
       operands are defined in the one or two byte maps below */
  { "rol", 0, true, { 0, 0, 0 }, 0 },
  { "ror", 0, true, { 0, 0, 0 }, 0 },
  { "rcl", 0, true, { 0, 0, 0 }, 0 },
  { "rcr", 0, true, { 0, 0, 0 }, 0 },
  { "shl sal", 0, true, { 0, 0, 0 }, 0 },
  { "shr", 0, true, { 0, 0, 0 }, 0 },
  { 0, Ill, 0, { 0, 0, 0 }, 0 },
  { "sar", 0, true, { 0, 0, 0 }, 0 }
 },

 { /* group 3a - operands are defined here */
  { "test", 0, true, { Eb, Ib, 0 }, 0 },
  { 0, Ill, 0, { 0, 0, 0 }, 0 },
  { "not", 0, true, { Eb, 0, 0 }, 0 },
  { "neg", 0, true, { Eb, 0, 0 }, 0 },
  { "mul", 0, true, { AL, Eb, 0 }, 0 },
  { "imul", 0, true, { AL, Eb, 0 }, 0 },
  { "div", 0, true, { AL, Eb, 0 }, 0 },
  { "idiv", 0, true, { AL, Eb, 0 }, 0 }
 },

 { /* group 3b - operands are defined here */
  { "test", 0, true, { Ev, Iv, 0 }, 0 },
  { 0, Ill, 0, { 0, 0, 0 }, 0 },
  { "not", 0, true, { Ev, 0, 0 }, 0 },
  { "neg", 0, true, { Ev, 0, 0 }, 0 },
  { "mul", 0, true, { eAX, Ev, 0 }, 0 },
  { "imul", 0, true, { eAX, Ev, 0 }, 0 },
  { "div", 0, true, { eAX, Ev, 0 }, 0 },
  { "idiv", 0, true, { eAX, Ev, 0 }, 0 }
 },

 { /* group 4 - operands are defined here */
  { "inc", 0, true, { Eb, 0, 0 }, 0 },
  { "dec", 0, true, { Eb, 0, 0 }, 0 },
  { 0, Ill, 0, { 0, 0, 0 }, 0 },
  { 0, Ill, 0, { 0, 0, 0 }, 0 },
  { 0, Ill, 0, { 0, 0, 0 }, 0 },
  { 0, Ill, 0, { 0, 0, 0 }, 0 },
  { 0, Ill, 0, { 0, 0, 0 }, 0 },
  { 0, Ill, 0, { 0, 0, 0 }, 0 },
 },

 { /* group 5 - operands are defined here */
  { "inc", 0, true, { Ev, 0, 0 }, 0 },
  { "dec", 0, true, { Ev, 0, 0 }, 0 },
  { "call", 0, true, { Ev, 0, 0 }, (IS_CALL | INDIR) },
  { "call", 0, true, { Ep, 0, 0 }, (IS_CALL | INDIR) },
  { "jmp", 0, true, { Ev, 0, 0 } , (IS_JUMP | INDIR) },
  { "jmp", 0, true, { Ep, 0, 0 }, (IS_JUMP | INDIR) },
  { "push", 0, true, { Ev, 0, 0, }, 0 },
  { 0, Ill, 0, { 0, 0, 0 }, 0 },
 },

 { /* group 6 - operands are defined here */
  { "sldt", 0, true, { Ew, 0, 0 }, 0 },
  { "str", 0, true, { Ew, 0, 0 }, 0 },
  { "lldt", 0, true, { Ew, 0, 0 }, 0 },
  { "ltr", 0, true, { Ew, 0, 0 }, 0 },
  { "verr", 0, true, { Ew, 0, 0 }, 0 },
  { "verw", 0, true, { Ew, 0, 0 }, 0 },
  { 0, Ill, 0, { 0, 0, 0 }, 0 },
  { 0, Ill, 0, { 0, 0, 0 }, 0 },
 },

 { /* group 7 - operands are defined here */
  { "sgdt", 0, true, { Ms, 0, 0 }, 0 },
  { "sidt", 0, true, { Ms, 0, 0 }, 0 },
  { "lgdt", 0, true, { Ms, 0, 0 }, 0 },
  { "lidt", 0, true, { Ms, 0, 0 }, 0 },
  { "smsw", 0, true, { Ew, 0, 0 }, 0 },
  { 0, Ill, 0, { 0, 0, 0 }, 0 },
  { "lmsw", 0, true, { Ew, 0, 0 }, 0 },
  { "invlpg", 0, true, { 0, 0, 0 }, 0 },
 },

 { /* group 8 - only opcode is defined here, 
     operands are defined in the one or two byte maps below */
  { 0, Ill, 0, { 0, 0, 0 }, 0 },
  { 0, Ill, 0, { 0, 0, 0 }, 0 },
  { 0, Ill, 0, { 0, 0, 0 }, 0 },
  { 0, Ill, 0, { 0, 0, 0 }, 0 },
  { "bt", 0, true, { 0, 0, 0 }, 0 },
  { "bts", 0, true, { 0, 0, 0 }, 0 },
  { "btr", 0, true, { 0, 0, 0 }, 0 },
  { "btc", 0, true, { 0, 0, 0 }, 0 },
 },

 { /* group 9 - operands are defined here */
  { 0, Ill, 0, { 0, 0, 0 }, 0 },
  { "cmpxch8b", 0, true, { Mq, 0, 0 }, 0 },
  { 0, Ill, 0, { 0, 0, 0 }, 0 },
  { 0, Ill, 0, { 0, 0, 0 }, 0 },
  { 0, Ill, 0, { 0, 0, 0 }, 0 },
  { 0, Ill, 0, { 0, 0, 0 }, 0 },
  { 0, Ill, 0, { 0, 0, 0 }, 0 },
  { 0, Ill, 0, { 0, 0, 0 }, 0 }
 },
};

// twoByteMap: two byte opcode instructions (first byte is 0x0F)
static x86_insn twoByteMap[256] = {
  /* 00 */
  { 0, Grp6, true, { 0, 0, 0 }, 0 }, 
  { 0, 0, false, { 0, 0, 0 }, 0 },
  { "lar", twoB, true, { Gv, Ew, 0 }, 0  },
  { "lsl", twoB, true, { Gv, Ew, 0 }, 0  },
  {0,Ill,0,{0,0,0},0},
  {0,Ill,0,{0,0,0},0},
  { "clts", twoB, false, { 0, 0, 0 }, 0 },
  {0,Ill,0,{0,0,0},0},
  /* 08 */
  { "invd", twoB, false, { 0, 0, 0 }, 0 },
  { "wbinvd", twoB, false, { 0, 0, 0 }, 0 },
  {0,Ill,0,{0,0,0},0},
  {0,Ill,0,{0,0,0},0},
  {0,Ill,0,{0,0,0},0},
  {0,Ill,0,{0,0,0},0},
  {0,Ill,0,{0,0,0},0},
  {0,Ill,0,{0,0,0},0},
  /* 10 */
  {0,Ill,0,{0,0,0},0}, {0,Ill,0,{0,0,0},0}, {0,Ill,0,{0,0,0},0}, {0,Ill,0,{0,0,0},0},
  {0,Ill,0,{0,0,0},0}, {0,Ill,0,{0,0,0},0}, {0,Ill,0,{0,0,0},0}, {0,Ill,0,{0,0,0},0},
  /* 18 */
  {0,Ill,0,{0,0,0},0}, {0,Ill,0,{0,0,0},0}, {0,Ill,0,{0,0,0},0}, {0,Ill,0,{0,0,0},0},
  {0,Ill,0,{0,0,0},0}, {0,Ill,0,{0,0,0},0}, {0,Ill,0,{0,0,0},0}, {0,Ill,0,{0,0,0},0},
  /* 20 */
  { "mov", twoB, true, { Rd, Cd, 0 }, 0 },
  { "mov", twoB, true, { Rd, Dd, 0 }, 0 },
  { "mov", twoB, true, { Cd, Rd, 0 }, 0 },
  { "mov", twoB, true, { Dd, Rd, 0 }, 0 },
  {0,Ill,0,{0,0,0},0}, {0,Ill,0,{0,0,0},0}, {0,Ill,0,{0,0,0},0}, {0,Ill,0,{0,0,0},0},
  /* 28 */
  {0,Ill,0,{0,0,0},0}, {0,Ill,0,{0,0,0},0}, {0,Ill,0,{0,0,0},0}, {0,Ill,0,{0,0,0},0},
  {0,Ill,0,{0,0,0},0}, {0,Ill,0,{0,0,0},0}, {0,Ill,0,{0,0,0},0}, {0,Ill,0,{0,0,0},0},
  /* 30 */
  { "wrmsr", twoB, false, { 0, 0, 0 }, 0 },
  { "rdtsc", twoB, false, { 0, 0, 0 }, 0 },
  { "rdmsr", twoB, false, { 0, 0, 0 }, 0 },
  {0,Ill,0,{0,0,0},0},
  {0,Ill,0,{0,0,0},0}, {0,Ill,0,{0,0,0},0}, {0,Ill,0,{0,0,0},0}, {0,Ill,0,{0,0,0},0},
  /* 38 */
  {0,Ill,0,{0,0,0},0}, {0,Ill,0,{0,0,0},0}, {0,Ill,0,{0,0,0},0}, {0,Ill,0,{0,0,0},0},
  {0,Ill,0,{0,0,0},0}, {0,Ill,0,{0,0,0},0}, {0,Ill,0,{0,0,0},0}, {0,Ill,0,{0,0,0},0},
  /* 40 */
  { "cmovo", twoB, true, { Gv, Ev, 0 }, 0 },
  { "cmovno", twoB, true, { Gv, Ev, 0 }, 0 },
  { "cmovnae", twoB, true, { Gv, Ev, 0 }, 0 },
  { "cmovnb", twoB, true, { Gv, Ev, 0 }, 0 },
  { "cmove", twoB, true, { Gv, Ev, 0 }, 0 },
  { "cmovne", twoB, true, { Gv, Ev, 0 }, 0 },
  { "cmovbe", twoB, true, { Gv, Ev, 0 }, 0 },
  { "cmovnbe", twoB, true, { Gv, Ev, 0 }, 0 },
  /* 48 */
  { "cmovs", twoB, true, { Gv, Ev, 0 }, 0 },
  { "cmovns", twoB, true, { Gv, Ev, 0 }, 0 },
  { "cmovpe", twoB, true, { Gv, Ev, 0 }, 0 },
  { "cmovpo", twoB, true, { Gv, Ev, 0 }, 0 },
  { "cmovnge", twoB, true, { Gv, Ev, 0 }, 0 },
  { "cmovnl", twoB, true, { Gv, Ev, 0 }, 0 },
  { "cmovng", twoB, true, { Gv, Ev, 0 }, 0 },
  { "cmovnl", twoB, true, { Gv, Ev, 0 }, 0 },
  /* 50 */
  {0,Ill,0,{0,0,0},0}, {0,Ill,0,{0,0,0},0}, {0,Ill,0,{0,0,0},0}, {0,Ill,0,{0,0,0},0},
  {0,Ill,0,{0,0,0},0}, {0,Ill,0,{0,0,0},0}, {0,Ill,0,{0,0,0},0}, {0,Ill,0,{0,0,0},0},
  /* 58 */
  {0,Ill,0,{0,0,0},0}, {0,Ill,0,{0,0,0},0}, {0,Ill,0,{0,0,0},0}, {0,Ill,0,{0,0,0},0},
  {0,Ill,0,{0,0,0},0}, {0,Ill,0,{0,0,0},0}, {0,Ill,0,{0,0,0},0}, {0,Ill,0,{0,0,0},0},
  /* 60 */
  {0,Ill,0,{0,0,0},0}, {0,Ill,0,{0,0,0},0}, {0,Ill,0,{0,0,0},0}, {0,Ill,0,{0,0,0},0},
  {0,Ill,0,{0,0,0},0}, {0,Ill,0,{0,0,0},0}, {0,Ill,0,{0,0,0},0}, {0,Ill,0,{0,0,0},0},
  /* 68 */
  {0,Ill,0,{0,0,0},0}, {0,Ill,0,{0,0,0},0}, {0,Ill,0,{0,0,0},0}, {0,Ill,0,{0,0,0},0},
  {0,Ill,0,{0,0,0},0}, {0,Ill,0,{0,0,0},0}, {0,Ill,0,{0,0,0},0}, {0,Ill,0,{0,0,0},0},
  /* 70 */
  {0,Ill,0,{0,0,0},0}, {0,Ill,0,{0,0,0},0}, {0,Ill,0,{0,0,0},0}, {0,Ill,0,{0,0,0},0},
  {0,Ill,0,{0,0,0},0}, {0,Ill,0,{0,0,0},0}, {0,Ill,0,{0,0,0},0}, {0,Ill,0,{0,0,0},0},
  /* 78 */
  {0,Ill,0,{0,0,0},0}, {0,Ill,0,{0,0,0},0}, {0,Ill,0,{0,0,0},0}, {0,Ill,0,{0,0,0},0},
  {0,Ill,0,{0,0,0},0}, {0,Ill,0,{0,0,0},0}, {0,Ill,0,{0,0,0},0}, {0,Ill,0,{0,0,0},0},
  /* 80 */
  { "jo", twoB, false, { Jv, 0, 0 }, (IS_JCC | REL_X) },
  { "jno", twoB, false, { Jv, 0, 0 }, (IS_JCC | REL_X) },
  { "jb", twoB, false, { Jv, 0, 0 }, (IS_JCC | REL_X) },
  { "jnb", twoB, false, { Jv, 0, 0 }, (IS_JCC | REL_X) },
  { "jz", twoB, false, { Jv, 0, 0 }, (IS_JCC | REL_X) },
  { "jnz", twoB, false, { Jv, 0, 0 }, (IS_JCC | REL_X) },
  { "jbe", twoB, false, { Jv, 0, 0 }, (IS_JCC | REL_X) },
  { "jnbe", twoB, false, { Jv, 0, 0 }, (IS_JCC | REL_X) },
  /* 88 */
  { "js", twoB, false, { Jv, 0, 0 }, (IS_JCC | REL_X) },
  { "jns", twoB, false, { Jv, 0, 0 }, (IS_JCC | REL_X) },
  { "jp", twoB, false, { Jv, 0, 0 }, (IS_JCC | REL_X) },
  { "jnp", twoB, false, { Jv, 0, 0 }, (IS_JCC | REL_X) },
  { "jl", twoB, false, { Jv, 0, 0 }, (IS_JCC | REL_X) },
  { "jnl", twoB, false, { Jv, 0, 0 }, (IS_JCC | REL_X) },
  { "jle", twoB, false, { Jv, 0, 0 }, (IS_JCC | REL_X) },
  { "jnle", twoB, false, { Jv, 0, 0 }, (IS_JCC | REL_X) },
  /* 90 */
  { "seto", twoB, true, { Eb, 0, 0 }, 0 },
  { "setno", twoB, true, { Eb, 0, 0 }, 0 },
  { "setb", twoB, true, { Eb, 0, 0 }, 0 },
  { "setnb", twoB, true, { Eb, 0, 0 }, 0 },
  { "setz", twoB, true, { Eb, 0, 0 }, 0 },
  { "setnz", twoB, true, { Eb, 0, 0 }, 0 },
  { "setbe", twoB, true, { Eb, 0, 0 }, 0 },
  { "setnbe", twoB, true, { Eb, 0, 0 }, 0 },
  /* 98 */
  { "sets", twoB, true, { Eb, 0, 0 }, 0 },
  { "setns", twoB, true, { Eb, 0, 0 }, 0 },
  { "setp", twoB, true, { Eb, 0, 0 }, 0 },
  { "setnp", twoB, true, { Eb, 0, 0 }, 0 },
  { "setl", twoB, true, { Eb, 0, 0 }, 0 },
  { "setnl", twoB, true, { Eb, 0, 0 }, 0 },
  { "setle", twoB, true, { Eb, 0, 0 }, 0 },
  { "setnle", twoB, true, { Eb, 0, 0 }, 0 },
  /* A0 */
  { "push", twoB, false, { FS, 0, 0 }, 0 },
  { "pop", twoB, false, { FS, 0, 0 }, 0 },
  { "cpuid", twoB, false, { 0, 0, 0 }, 0 },
  { "bt", twoB, true, { Ev, Gv, 0 }, 0 },
  { "shld", twoB, true, { Ev, Gv, Ib }, 0 },
  { "shld", twoB, true, { Ev, Gv, CL }, 0 },
  {0,Ill,0,{0,0,0},0}, 
  {0,Ill,0,{0,0,0},0},
  /* A8 */
  { "push", twoB, false, { GS, 0, 0 }, 0 },
  { "pop", twoB, false, { GS, 0, 0 }, 0 },
  { "rsm", twoB, false, { 0, 0, 0 }, 0 },
  { "bts", twoB, true, { Ev, Gv, 0 }, 0 },
  { "shrd", twoB, true, { Ev, Gv, Ib }, 0 },
  { "shrd", twoB, true, { Ev, Gv, CL }, 0 },
  {0,Ill,0,{0,0,0},0}, 
  { "imul", twoB, true, { Gv, Ev, 0 }, 0 },
  /* B0 */
  { "cmpxch", twoB, true, { Eb, Gb, 0 }, 0 },
  { "cmpxch", twoB, true, { Ev, Gv, 0 }, 0 },
  { "lss", twoB, true, { Mp, 0, 0 }, 0 },
  { "btr", twoB, true, { Ev, Gv, 0 }, 0 },
  { "lfs", twoB, true, { Mp, 0, 0 }, 0 },
  { "lgs", twoB, true, { Mp, 0, 0 }, 0 },
  { "movzx", twoB, true, { Gv, Eb, 0 }, 0 },
  { "movzx", twoB, true, { Gv, Ew, 0 }, 0 },
  /* B8 */
  {0,Ill,0,{0,0,0},0},
  {0,Ill,0,{0,0,0},0},
  { 0, Grp8, true, { Ev, Ib, 0 }, 0 },
  { "btc", twoB, true, { Ev, Gv, 0 }, 0 },
  { "bsf", twoB, true, { Gv, Ev, 0 }, 0 },
  { "bsr", twoB, true, { Gv, Ev, 0 }, 0 },
  { "movsx", twoB, true, { Gv, Eb, 0 }, 0 },
  { "movsx", twoB, true, { Gv, Ew, 0 }, 0 },
  /* C0 */
  { "xadd", twoB, true, { Eb, Gb, 0 }, 0 },
  { "xadd", twoB, true, { Ev, Gv, 0 }, 0 },
  {0,Ill,0,{0,0,0},0},
  {0,Ill,0,{0,0,0},0},
  {0,Ill,0,{0,0,0},0},
  {0,Ill,0,{0,0,0},0},
  {0,Ill,0,{0,0,0},0},
  { 0, Grp9, true, { 0, 0, 0 }, 0 },
  /* C8 */
  { "bswap", twoB, false, { rEAX, 0, 0 }, 0 }, 
  { "bswap", twoB, false, { rECX, 0, 0 }, 0 }, 
  { "bswap", twoB, false, { rEDX, 0, 0 }, 0 }, 
  { "bswap", twoB, false, { rEBX, 0, 0 }, 0 }, 
  { "bswap", twoB, false, { rESP, 0, 0 }, 0 },
  { "bswap", twoB, false, { rEBP, 0, 0 }, 0 }, 
  { "bswap", twoB, false, { rESI, 0, 0 }, 0 }, 
  { "bswap", twoB, false, { rEDI, 0, 0 }, 0 }, 
  /* D0 */
  {0,Ill,0,{0,0,0},0}, {0,Ill,0,{0,0,0},0}, {0,Ill,0,{0,0,0},0}, {0,Ill,0,{0,0,0},0},
  {0,Ill,0,{0,0,0},0}, {0,Ill,0,{0,0,0},0}, {0,Ill,0,{0,0,0},0}, {0,Ill,0,{0,0,0},0},
  /* D8 */
  {0,Ill,0,{0,0,0},0}, {0,Ill,0,{0,0,0},0}, {0,Ill,0,{0,0,0},0}, {0,Ill,0,{0,0,0},0},
  {0,Ill,0,{0,0,0},0}, {0,Ill,0,{0,0,0},0}, {0,Ill,0,{0,0,0},0}, {0,Ill,0,{0,0,0},0},
  /* E0 */
  {0,Ill,0,{0,0,0},0}, {0,Ill,0,{0,0,0},0}, {0,Ill,0,{0,0,0},0}, {0,Ill,0,{0,0,0},0},
  {0,Ill,0,{0,0,0},0}, {0,Ill,0,{0,0,0},0}, {0,Ill,0,{0,0,0},0}, {0,Ill,0,{0,0,0},0},
  /* E8 */
  {0,Ill,0,{0,0,0},0}, {0,Ill,0,{0,0,0},0}, {0,Ill,0,{0,0,0},0}, {0,Ill,0,{0,0,0},0},
  {0,Ill,0,{0,0,0},0}, {0,Ill,0,{0,0,0},0}, {0,Ill,0,{0,0,0},0}, {0,Ill,0,{0,0,0},0},
  /* F0 */
  {0,Ill,0,{0,0,0},0}, {0,Ill,0,{0,0,0},0}, {0,Ill,0,{0,0,0},0}, {0,Ill,0,{0,0,0},0},
  {0,Ill,0,{0,0,0},0}, {0,Ill,0,{0,0,0},0}, {0,Ill,0,{0,0,0},0}, {0,Ill,0,{0,0,0},0},
  /* F8 */
  {0,Ill,0,{0,0,0},0}, {0,Ill,0,{0,0,0},0}, {0,Ill,0,{0,0,0},0}, {0,Ill,0,{0,0,0},0},
  {0,Ill,0,{0,0,0},0}, {0,Ill,0,{0,0,0},0}, {0,Ill,0,{0,0,0},0}, {0,Ill,0,{0,0,0},0},
};


// oneByteMap: one byte opcode map
static x86_insn oneByteMap[256] = {
  /* 00 */
  { "add", oneB, true, { Eb, Gb, 0 }, 0 },
  { "add", oneB, true, { Ev, Gv, 0 }, 0 },
  { "add", oneB, true, { Gb, Eb, 0 }, 0 },
  { "add", oneB, true, { Gv, Ev, 0 }, 0 },
  { "add", oneB, false, { AL, Ib, 0 }, 0 },
  { "add", oneB, false, { eAX, Iv, 0 }, 0 },
  { "push", oneB, false, { ES, 0, 0 }, 0 },
  { "pop", oneB, false, { ES, 0, 0 }, 0 },
  /* 08 */
  { "or", oneB, true, { Eb, Gb, 0 }, 0 },
  { "or", oneB, true, { Ev, Gv, 0 }, 0 },
  { "or", oneB, true, { Gb, Eb, 0 }, 0 },
  { "or", oneB, true, { Gv, Ev, 0 }, 0 },
  { "or", oneB, false, { AL, Ib, 0 }, 0 },
  { "or", oneB, false, { eAX, Iv, 0 }, 0 },
  { "push", oneB, false, { ES, 0, 0 }, 0 },
  { 0 , twoB, false, { 0, 0, 0 }, 0 },
  /* 10 */
  { "adc", oneB, true, { Eb, Gb, 0 }, 0 },
  { "adc", oneB, true, { Ev, Gv, 0 }, 0 },
  { "adc", oneB, true, { Gb, Eb, 0 }, 0 },
  { "adc", oneB, true, { Gv, Ev, 0 }, 0 },
  { "adc", oneB, false, { AL, Ib, 0 }, 0 },
  { "adc", oneB, false, { eAX, Iv, 0 }, 0 },
  { "push", oneB, false, { SS, 0, 0 }, 0 },
  { "pop", oneB, false, { SS, 0, 0 }, 0 },
  /* 18 */
  { "sbb", oneB, true, { Eb, Gb, 0 }, 0 },
  { "sbb", oneB, true, { Ev, Gv, 0 }, 0 },
  { "sbb", oneB, true, { Gb, Eb, 0 }, 0 },
  { "sbb", oneB, true, { Gv, Ev, 0 }, 0 },
  { "sbb", oneB, false, { AL, Ib, 0 }, 0 },
  { "sbb", oneB, false, { eAX, Iv, 0 }, 0 },
  { "push", oneB, false, { DS, 0, 0 }, 0 },
  { "pop" , oneB, false, { DS, 0, 0 }, 0 },
  /* 20 */
  { "and", oneB, true, { Eb, Gb, 0 }, 0 },
  { "and", oneB, true, { Ev, Gv, 0 }, 0 },
  { "and", oneB, true, { Gb, Eb, 0 }, 0 },
  { "and", oneB, true, { Gv, Ev, 0 }, 0 },
  { "and", oneB, false, { AL, Ib, 0 }, 0 },
  { "and", oneB, false, { eAX, Iv, 0 }, 0 },
  { 0, PREFIX_SEG_OVR, false, { 0, 0, 0 }, 0 },
  { "daa", oneB, false, { 0, 0, 0 }, 0 },
  /* 28 */
  { "sub", oneB, true, { Eb, Gb, 0 }, 0 },
  { "sub", oneB, true, { Ev, Gv, 0 }, 0 },
  { "sub", oneB, true, { Gb, Eb, 0 }, 0 },
  { "sub", oneB, true, { Gv, Ev, 0 }, 0 },
  { "sub", oneB, false, { AL, Ib, 0 }, 0 },
  { "sub", oneB, false, { eAX, Iv, 0 }, 0 },
  { 0, PREFIX_SEG_OVR, false, { 0, 0, 0 }, 0 },
  { "das" , oneB, false, { 0, 0, 0 }, 0 },
  /* 30 */
  { "xor", oneB, true, { Eb, Gb, 0 }, 0 },
  { "xor", oneB, true, { Ev, Gv, 0 }, 0 },
  { "xor", oneB, true, { Gb, Eb, 0 }, 0 },
  { "xor", oneB, true, { Gv, Ev, 0 }, 0 },
  { "xor", oneB, false, { AL, Ib, 0 }, 0 },
  { "xor", oneB, false, { eAX, Iv, 0 }, 0 },
  { 0, PREFIX_SEG_OVR, false, { 0, 0, 0 }, 0 },
  { "aaa", oneB, false, { 0, 0, 0 }, 0 },
  /* 38 */
  { "cmp", oneB, true, { Eb, Gb, 0 }, 0 },
  { "cmp", oneB, true, { Ev, Gv, 0 }, 0 },
  { "cmp", oneB, true, { Gb, Eb, 0 }, 0 },
  { "cmp", oneB, true, { Gv, Ev, 0 }, 0 },
  { "cmp", oneB, false, { AL, Ib, 0 }, 0 },
  { "cmp", oneB, false, { eAX, Iv, 0 }, 0 },
  { 0, PREFIX_SEG_OVR, false, { 0, 0, 0 }, 0 },
  { "aas" , oneB, false, { 0, 0, 0 }, 0 },
  /* 40 */
  { "inc", oneB, false, { eAX, 0, 0 }, 0 }, 
  { "inc", oneB, false, { eCX, 0, 0 }, 0 },
  { "inc", oneB, false, { eDX, 0, 0 }, 0 },
  { "inc", oneB, false, { eBX, 0, 0 }, 0 },
  { "inc", oneB, false, { eSP, 0, 0 }, 0 },
  { "inc", oneB, false, { eBP, 0, 0 }, 0 },
  { "inc", oneB, false, { eSI, 0, 0 }, 0 },
  { "inc", oneB, false, { eDI, 0, 0 }, 0 },
  /* 48 */
  { "dec", oneB, false, { eAX, 0, 0 }, 0 }, 
  { "dec", oneB, false, { eCX, 0, 0 }, 0 },
  { "dec", oneB, false, { eDX, 0, 0 }, 0 },
  { "dec", oneB, false, { eBX, 0, 0 }, 0 },
  { "dec", oneB, false, { eSP, 0, 0 }, 0 },
  { "dec", oneB, false, { eBP, 0, 0 }, 0 },
  { "dec", oneB, false, { eSI, 0, 0 }, 0 },
  { "dec", oneB, false, { eDI, 0, 0 }, 0 },
  /* 50 */
  { "push", oneB, false, { eAX, 0, 0 }, 0 }, 
  { "push", oneB, false, { eCX, 0, 0 }, 0 },
  { "push", oneB, false, { eDX, 0, 0 }, 0 },
  { "push", oneB, false, { eBX, 0, 0 }, 0 },
  { "push", oneB, false, { eSP, 0, 0 }, 0 },
  { "push", oneB, false, { eBP, 0, 0 }, 0 },
  { "push", oneB, false, { eSI, 0, 0 }, 0 },
  { "push", oneB, false, { eDI, 0, 0 }, 0 },
  /* 58 */
  { "pop", oneB, false, { eAX, 0, 0 }, 0 }, 
  { "pop", oneB, false, { eCX, 0, 0 }, 0 },
  { "pop", oneB, false, { eDX, 0, 0 }, 0 },
  { "pop", oneB, false, { eBX, 0, 0 }, 0 },
  { "pop", oneB, false, { eSP, 0, 0 }, 0 },
  { "pop", oneB, false, { eBP, 0, 0 }, 0 },
  { "pop", oneB, false, { eSI, 0, 0 }, 0 },
  { "pop", oneB, false, { eDI, 0, 0 }, 0 },
  /* 60 */
  { "pusha(d)", oneB, false, { 0, 0, 0 }, 0 }, 
  { "popa(d)", oneB, false, { 0, 0, 0 }, 0 },
  { "bound", oneB, true, { Gv, Ma, 0 }, 0 },
  { "arpl", oneB, true, { Ew, Gw, 0 }, 0 },
  { 0, PREFIX_SEG_OVR, false, { 0, 0, 0 }, 0 },
  { 0, PREFIX_SEG_OVR, false, { 0, 0, 0 }, 0 },
  { 0, PREFIX_OPR_SZ, false, { 0, 0, 0 }, 0 }, /* operand size prefix */
  { 0, PREFIX_ADDR_SZ, false, { 0, 0, 0 }, 0 }, /* address size prefix */
  /* 68 */
  { "push", oneB, false, { Iv, 0, 0 }, 0 },
  { "imul", oneB, true, { Gv, Ev, Iv }, 0 },
  { "push", oneB, false, { Ib, 0, 0 }, 0 },
  { "imul", oneB, true, { Gv, Ev, Ib }, 0 },
  { "insb", oneB, false, { Yb, DX, 0 }, 0 },
  { "insw/d", oneB, false, { Yv, DX, 0 }, 0 },
  { "outsb", oneB, false, { DX, Xb, 0 }, 0 },
  { "outsw/d", oneB, false, { DX, Xv, 0 }, 0 },
  /* 70 */
  { "jo", oneB, false, { Jb, 0, 0 }, (IS_JCC | REL_B) },
  { "jno", oneB, false, { Jb, 0, 0 }, (IS_JCC | REL_B) },
  { "jb/jnaej/j", oneB, false, { Jb, 0, 0 }, (IS_JCC | REL_B) },
  { "jnb/jae/j", oneB, false, { Jb, 0, 0 }, (IS_JCC | REL_B) },
  { "jz", oneB, false, { Jb, 0, 0 }, (IS_JCC | REL_B) },
  { "jnz", oneB, false, { Jb, 0, 0 }, (IS_JCC | REL_B) },
  { "jbe", oneB, false, { Jb, 0, 0 }, (IS_JCC | REL_B) },
  { "jnbe", oneB, false, { Jb, 0, 0 }, (IS_JCC | REL_B) },
  /* 78 */
  { "js", oneB, false, { Jb, 0, 0 }, (IS_JCC | REL_B) },
  { "jns", oneB, false, { Jb, 0, 0 }, (IS_JCC | REL_B) },
  { "jp", oneB, false, { Jb, 0, 0 }, (IS_JCC | REL_B) },
  { "jnp", oneB, false, { Jb, 0, 0 }, (IS_JCC | REL_B) },
  { "jl", oneB, false, { Jb, 0, 0 }, (IS_JCC | REL_B) },
  { "jnl", oneB, false, { Jb, 0, 0 }, (IS_JCC | REL_B) },
  { "jle", oneB, false, { Jb, 0, 0 }, (IS_JCC | REL_B) },
  { "jnle", oneB, false, { Jb, 0, 0 }, (IS_JCC | REL_B) },
  /* 80 */
  { 0, Grp1, true, { Eb, Ib, 0 }, 0 },
  { 0, Grp1, true, { Ev, Iv, 0 }, 0 },
  { 0, Ill, 0, { 0, 0, 0 }, 0 },
  { 0, Grp1, true, { Ev, Ib, 0 }, 0 },
  { "test", oneB, true, { Eb, Gb, 0 }, 0 },
  { "test", oneB, true, { Ev, Gv, 0 }, 0 },
  { "xchg", oneB, true, { Eb, Gb, 0 }, 0 },
  { "xchg", oneB, true, { Ev, Gv, 0 }, 0 },
  /* 88 */
  { "mov", oneB, true, { Eb, Gb, 0 }, 0 },
  { "mov", oneB, true, { Ev, Gv, 0 }, 0 },
  { "mov", oneB, true, { Gb, Eb, 0 }, 0 },
  { "mov", oneB, true, { Gv, Ev, 0 }, 0 },
  { "mov", oneB, true, { Ew, Sw, 0 }, 0 },
  { "lea", oneB, true, { Gv, M, 0 }, 0 },
  { "mov", oneB, true, { Sw, Ew, 0 }, 0 },
  { "pop", oneB, true, { Ev, 0, 0, }, 0 },
  /* 90 */
  { "nop", oneB, false, { 0, 0, 0 }, 0 },
  { "xchg", oneB, false, { eCX, 0, 0 }, 0 },
  { "xchg", oneB, false, { eDX, 0, 0 }, 0 },
  { "xchg", oneB, false, { eBX, 0, 0 }, 0 },
  { "xchg", oneB, false, { eSP, 0, 0 }, 0 },
  { "xchg", oneB, false, { eBP, 0, 0 }, 0 },
  { "xchg", oneB, false, { eSI, 0, 0 }, 0 },
  { "xchg", oneB, false, { eDI, 0, 0 }, 0 },
  /* 98 */
  { "cbw", oneB, false, { 0, 0, 0 }, 0 },
  { "cwd/cdq", oneB, false, { 0, 0, 0 }, 0 },
  { "call", oneB, false, { Ap, 0, 0}, IS_CALL | PTR_WX },
  { "wait", oneB, false, { 0, 0, 0 }, 0 },
  { "pushf", oneB, false, { Fv, 0, 0 }, 0 },
  { "pop", oneB, false, { Fv, 0, 0 }, 0 },
  { "sahf", oneB, false, { 0, 0, 0 }, 0 },
  { "lahf", oneB, false, { 0, 0, 0 }, 0 },
  /* A0 */
  { "mov", oneB, false, { AL, Ob, 0 }, 0 },
  { "mov", oneB, false, { eAX, Ov, 0 }, 0 },
  { "mov", oneB, false, { Ob, AL, 0 }, 0 },
  { "mov", oneB, false, { Ov, eAX, 0 }, 0 },
  { "movsb", oneB, false, { Xb, Yb, 0 }, 0 },
  { "movsw", oneB, false, { Xv, Yv, 0 }, 0 },
  { "cmpsb", oneB, false, { Xb, Yb, 0 }, 0 },
  { "cmpsw", oneB, false, { Xv, Yv, 0 }, 0 },
  /* A8 */
  { "test", oneB, false, { AL, Ib, 0 }, 0 },
  { "test", oneB, false, { eAX, Iv, 0 }, 0 },
  { "stopsb", oneB, false, { Yb, AL, 0 }, 0 },
  { "stopsw/d", oneB, false, { Yv, eAX, 0 }, 0 },
  { "lodsb", oneB, false, { AL, Xb, 0 }, 0 },
  { "lodsw", oneB, false, { eAX, Xv, 0 }, 0 },
  { "scasb", oneB, false, { AL, Yb, 0 }, 0 },
  { "scasw/d", oneB, false, { eAX, Yv, 0 }, 0 },
  /* B0 */
  { "mov", oneB, false, { AL, Ib, 0 }, 0 },
  { "mov", oneB, false, { CL, Ib, 0 }, 0 },
  { "mov", oneB, false, { DL, Ib, 0 }, 0 },
  { "mov", oneB, false, { BL, Ib, 0 }, 0 },
  { "mov", oneB, false, { AH, Ib, 0 }, 0 },
  { "mov", oneB, false, { CH, Ib, 0 }, 0 },
  { "mov", oneB, false, { DH, Ib, 0 }, 0 },
  { "mov", oneB, false, { BH, Ib, 0 }, 0 },
  /* B8 */
  { "mov", oneB, false, { eAX, Iv, 0 }, 0 },
  { "mov", oneB, false, { eCX, Iv, 0 }, 0 },
  { "mov", oneB, false, { eDX, Iv, 0 }, 0 },
  { "mov", oneB, false, { eBX, Iv, 0 }, 0 },
  { "mov", oneB, false, { eSP, Iv, 0 }, 0 },
  { "mov", oneB, false, { eBP, Iv, 0 }, 0 },
  { "mov", oneB, false, { eSI, Iv, 0 }, 0 },
  { "mov", oneB, false, { eDI, Iv, 0 }, 0 },
  /* C0 */
  { 0, Grp2, true, { Eb, Ib, 0 }, 0 },
  { 0, Grp2, true, { Ev, Ib, 0 }, 0 },
  { "ret near", oneB, false, { Iw, 0, 0 }, (IS_RET) },
  { "ret near", oneB, false, { 0, 0, 0 }, (IS_RET) },
  { "les", oneB, true, { Gv, Mp, 0 }, 0 },
  { "lds", oneB, true, { Gv, Mp, 0 }, 0 },
  { "mov", oneB, true, { Eb, Ib, 0 }, 0 },
  { "mov", oneB, true, { Ev, Iv, 0 }, 0 },
  /* C8 */
  { "enter", oneB, false, { Iw, Ib, 0 }, 0 },
  { "leave", oneB, false, { 0, 0, 0 }, 0 },
  { "ret far", oneB, false, { Iw, 0, 0 }, (IS_RETF) },
  { "ret far", oneB, false, { 0, 0, 0 }, (IS_RETF) },
  { "int 3", oneB, false, { 0, 0, 0 }, 0 },
  { "int", oneB, false, { Ib, 0, 0 }, 0 },
  { "into", oneB, false, { 0, 0, 0 }, 0 },
  { "iret", oneB, false, { 0, 0, 0 }, (IS_RET) },
  /* D0 */
  { 0, Grp2, true, { Eb, const1, 0 }, 0 },
  { 0, Grp2, true, { Ev, const1, 0 }, 0 },
  { 0, Grp2, true, { Eb, CL, 0 }, 0 },
  { 0, Grp2, true, { Ev, CL, 0 }, 0 },
  { "aam", oneB, false, { 0, 0, 0 }, 0 },
  { "aad", oneB, false, { 0, 0, 0 }, 0 },
  { 0, Ill, 0, { 0, 0, 0 }, 0 },
  { "xlat", oneB, false, { 0, 0, 0 }, 0 },
  /* D8 */
  { 0, coprocEsc, true, { 0, 0, 0 }, 0 },
  { 0, coprocEsc, true, { 0, 0, 0 }, 0 },
  { 0, coprocEsc, true, { 0, 0, 0 }, 0 },
  { 0, coprocEsc, true, { 0, 0, 0 }, 0 },
  { 0, coprocEsc, true, { 0, 0, 0 }, 0 },
  { 0, coprocEsc, true, { 0, 0, 0 }, 0 },
  { 0, coprocEsc, true, { 0, 0, 0 }, 0 },
  { 0, coprocEsc, true, { 0, 0, 0 }, 0 },
  /* E0 */
  { "loopn", oneB, false, { Jb, 0, 0 }, 0 },
  { "loope", oneB, false, { Jb, 0, 0 }, 0 },
  { "loop", oneB, false, { Jb, 0, 0 }, 0 },
  { "jcxz/jec", oneB, false, { Jb, 0, 0 }, (IS_JCC | REL_B) },
  { "in", oneB, false, { AL, Ib, 0 }, 0 },
  { "in", oneB, false, { eAX, Ib, 0 }, 0 },
  { "out", oneB, false, { Ib, AL, 0 }, 0 },
  { "out", oneB, false, { Ib, eAX, 0 }, 0 },
  /* E8 */
  { "call", oneB, false, { Jv, 0, 0 }, (IS_CALL | REL_X) },
  { "jmp", oneB, false, { Jv, 0, 0 }, (IS_JUMP | REL_X) },
  { "jmp", oneB, false, { Ap, 0, 0 }, (IS_JUMP | PTR_WX) },
  { "jmp", oneB, false, { Jb, 0, 0 }, (IS_JUMP | REL_B) },
  { "in", oneB, false, { AL, DX, 0 }, 0 },
  { "in", oneB, false, { eAX, DX, 0 }, 0 },
  { "out", oneB, false, { DX, AL, 0 }, 0 },
  { "out", oneB, false, { DX, eAX, 0 }, 0 },
  /* F0 */
  { 0, PREFIX_INSTR, false, { 0, 0, 0 }, 0 },
  { 0, 0, false, { 0, 0, 0 }, 0 },
  { 0, PREFIX_INSTR, false, { 0, 0, 0 }, 0 },
  { 0, PREFIX_INSTR, false, { 0, 0, 0 }, 0 },
  { "hlt", oneB, false, { 0, 0, 0 }, 0 },
  { "cmc", oneB, false, { 0, 0, 0 }, 0 },
  { 0, Grp3a, true, { Eb, 0, 0 }, 0 },
  { 0, Grp3b, true, { Ev, 0, 0 }, 0 },
  /* F8 */
  { "clc", oneB, false, { 0, 0, 0 }, 0 },
  { "stc", oneB, false, { 0, 0, 0 }, 0 },
  { "cli", oneB, false, { 0, 0, 0 }, 0 },
  { "sti", oneB, false, { 0, 0, 0 }, 0 },
  { "cld", oneB, false, { 0, 0, 0 }, 0 },
  { "std", oneB, false, { 0, 0, 0 }, 0 },
  { 0, Grp4, true, { 0, 0, 0 }, 0 },
  { 0, Grp5, true, { 0, 0, 0 }, 0 }
};


/* decodes an operand especified in the ModRM byte. Return the size in bytes of the
   operand field, not counting the ModRM byte.
*/
static unsigned doModRMOperand(const unsigned addrSzAttr, // the address size attr.
			       const unsigned Mod,        // mod field
			       const unsigned RM,         // RM field
			       const unsigned SIB)        // SIB byte
{
  if (addrSzAttr == 1) { // 16-bit addressing - use table 25-2 in the Pentium manual
    if ((Mod == 0 && RM == 6) || (Mod == 2)) {
      return wordSzB;
    }
    else if (Mod == 1) {
      return byteSzB;
    }
    else {
      return 0;
    }
  }
  else { // 32-bit addressing -  use table 25-3 in the Pentium manual
    if ((Mod == 0 && RM == 5) || (Mod == 2)) {
      return dwordSzB;
    }
    else if (Mod == 1) {
      return byteSzB;
    }
    else {
      // check SIB: there is a 32 bit displacement if Mod = 0 and 
      // the base field of SIB is 5
      if (Mod == 0 && RM == 4 && ((SIB & 0x7) == 5))
	return dwordSzB;
      else 
	return 0;
    }
  }
  // not reached
  assert(0);
}

/*
   decodes an operand of an instruction.
   Return the size in bytes of the operand fields. If the operand is encoded in the
   opcode or in the ModR/M byte, the size returned is zero.
*/
static unsigned doOperand(const unsigned operandType,   // type descriptor of operand
			  const unsigned operandSzAttr, // operand sz. attribute
			  const unsigned addrSzAttr,    // address sz. attribute
			  const unsigned Mod,           // MOD field
			  const unsigned RM,            // RM field
			  const unsigned SIB)           // SIB byte
{
  switch (operandType) {
  case Ap:
    /* direct address of operand encoded in instruction as a segment number (word)
       and an offset (word or dword)
     */
    return (wordSzB + (addrSzAttr * wordSzB));
    break;

  case Cd:
  case Dd:
    // ModRM byte selects operand
    return 0;
    break;
  
  case Eb:
  case Ep:
  case Ev:
  case Ew:
    /* the operand is either a register or a memory operand.
       The operand size doesn't matter here as the instruction does not have the
       operand itself, but only its address.
       Only the address size is used here.
    */
    return doModRMOperand(addrSzAttr, Mod, RM, SIB);
    break;

  case Fv:
    // operand is flag register
    return 0;
    break;

  case Gb:
  case Gv:
  case Gw:
    // operand encoded in modR/M byte
    return 0;
    break;

  case Ib: /* immediate byte */
    return byteSzB;
    break;
  case Iv: /* immediate word or dword */
    return (wordSzB * operandSzAttr);
    break;
  case Iw: /* immediate word */
    return wordSzB;
    break;

  case Jb: /* byte relative offset in instruction */
    return byteSzB;
    break;
  case Jv: /* word or dword relative offset in instruction */
    return (wordSzB * operandSzAttr);
    break;

  case M:
  case Ma:
  case Mp:
  case Mq:
  case Ms:
    /* Like E but the operand can be only a memory operand.
       The operand size doesn't matter here as the instruction does not have the
       operand itself, but only its address.
       Only the address size is used here.
    */
    return doModRMOperand(addrSzAttr, Mod, RM, SIB);
    break;

  case Ob:
  case Ov: 
    /* offset of operand encoded as word or dword */
    return (wordSzB * addrSzAttr);
    break;

  case Rd:
    return 0;
    break;

  case Sw:
    // operand defined in reg field of ModR/M byte
    return 0;
    break;

  case Xb:
  case Xv:
    return 0;
    break;

  case Yb:
  case Yv:
    return 0;
    break;

  case const1:
    return 0;
    break;

  default:
    if (operandType >= CS && operandType <= rESP)
      return 0;
  }
  // should not be reached
  assert(0);
  return 0;
}

/* decodes the operands, return the total size in bytes of the operands */
static unsigned doOperands(unsigned operands[3],
		    const unsigned operandSzAttr, const unsigned addrSzAttr,
		    const unsigned Mod, const unsigned RM, const unsigned SIB)
{
  unsigned size = 0;
  if (operands[0]) {
    size += doOperand(operands[0], operandSzAttr, addrSzAttr, Mod, RM, SIB);
  }
  if (operands[1]) {
    size += doOperand(operands[1], operandSzAttr, addrSzAttr, Mod, RM, SIB);
  }
  if (operands[2]) {
    size += doOperand(operands[2], operandSzAttr, addrSzAttr, Mod, RM, SIB);
  }
  return size;
}


unsigned get_instruction2(const unsigned char* addr, unsigned &insnType)
{
  bool instrPrefix = false;
  bool addrSzPrefix = false;
  bool oprSzPrefix = false;
  bool segOvrPrefix = false;

  unsigned operandSzAttr = 2;
  unsigned addrSzAttr = 2;

  const unsigned char *nextb = addr;
  x86_insn *desc;
  x86_insn *descAux;
  unsigned code;
  unsigned ModRMbyte = 0;
  unsigned Mod=0;
  unsigned Reg=0;
  unsigned RM=0;
  unsigned SIB=0;
  bool hasSIB;

  insnType = 0;

  // check prefixes
  do {
    desc = &oneByteMap[*nextb++];
    code = desc->escape_code;
    if (code == PREFIX_INSTR)
      instrPrefix = true;
    else if (code == PREFIX_ADDR_SZ)
      addrSzPrefix = true;
    else if (code == PREFIX_OPR_SZ)
      oprSzPrefix = true;
    else if (code == PREFIX_SEG_OVR)
      segOvrPrefix = true;
    else
      // no more prefixes
      break;
  } while (true);

  if (oprSzPrefix)
    operandSzAttr = 1;
  if (addrSzPrefix)
    addrSzAttr = 1;

  /* if it is a two byte opcode instruction, fetch the next opcode byte 
     and decode using twoByteMap */
  if (code == twoB) {
    desc = &twoByteMap[*nextb++];
    code = desc->escape_code;
  }

  // check if the instruction has Mod/RM and SIB bytes
  if (desc->hasModRM) {
    ModRMbyte = *nextb++;
    Mod = ModRMbyte >> 6;
    RM = ModRMbyte & 0x07;
    Reg = (ModRMbyte >> 3) & 0x07;
    hasSIB = (Mod != 3) && (RM == 4);
    if (hasSIB) {
      SIB = *nextb++;
    }
  }  

  /* now decode the instruction */
  switch (code) {
  case oneB:
  case twoB:
    insnType = desc->insnType;
    if (insnType & REL_X) {
      // set the correct size for the operand
      if (operandSzAttr == 1)
	insnType |= REL_W;
      else
	insnType |= REL_D;
    }
    //assert(desc->name);
    //printf("%s ", desc->name);
    nextb += doOperands(desc->operands, operandSzAttr, addrSzAttr, Mod, RM, SIB);
    break;

  case Grp1:
  case Grp2:
  case Grp8:
    // operands defined in oneByteMap
    insnType = desc->insnType;
    descAux = &groupMap[code][Reg];
    //assert(descAux->name);
    //printf("%s ", descAux->name);
    nextb += doOperands(desc->operands, operandSzAttr, addrSzAttr, Mod, RM, SIB);
    break;

  case Grp3a:
  case Grp3b:
  case Grp4:
  case Grp5:
  case Grp6:
  case Grp7:
  case Grp9:
    // operands defined in groupMap
    descAux = &groupMap[code][Reg];
    insnType = descAux->insnType;
    //assert(descAux->name);
    //printf("%s ", descAux->name);
    nextb += doOperands(descAux->operands, operandSzAttr, addrSzAttr, Mod, RM, SIB);
    break;

  case coprocEsc:
    insnType = 0;
    //fprintf(stderr,"FP instruction %x\n", nextb);
    if (ModRMbyte <= 0xBF) {
      nextb += doModRMOperand(addrSzAttr, Mod, RM, SIB);
    }
    break;
    
  case Ill:
    insnType = ILLEGAL;
    break;

  default:
    // should not be reached
    assert(0);
    break;
  }

  // set the correct size for the operand in insnType
  if (insnType & REL_X) {
    if (operandSzAttr == 1)
      insnType |= REL_W;
    else
      insnType |= REL_D;
  }
  else if (insnType & PTR_WX) {
    if (operandSzAttr == 1)
      insnType |= PTR_WW;
    else
      insnType |= PTR_WD;
  }

  // set the prefix flags in insnType
  if (instrPrefix)
    insnType |= PREFIX_INST;
  if (addrSzPrefix)
    insnType |= PREFIX_ADDR;
  if (oprSzPrefix)
    insnType |= PREFIX_OPR;
  if (segOvrPrefix)
    insnType |= PREFIX_SEG;

  return (nextb - addr);
}

/* decode instruction at address addr, return size of instruction */
unsigned get_instruction(const unsigned char* addr, unsigned &insnType)
{
  int r1;
  unsigned insnType1;

  ia32_instruction i;
  ia32_decode<0>(addr, i);
  r1 = i.getSize();
  insnType1 = ia32_emulate_old_type(i);

#ifdef DEBUG_CMPDEC
  int r2;
  unsigned insnType2;

  r2 = get_instruction2(addr, insnType2);
  if(r1 != r2) {
    fprintf(stderr, "[L] %d!=%d @ %p:", r1, r2, addr);
    for(int i=0; i<10; ++i)
      fprintf(stderr, " %x", addr[i]);
    fprintf(stderr, "\n");
  }
  if(insnType1 != insnType2) {
    fprintf(stderr, "[T] %x!=%x @ %p:", insnType1, insnType2, addr);
    for(int i=0; i<10; ++i)
      fprintf(stderr, " %x", addr[i]);
    fprintf(stderr, "\n");
  }
#endif

  insnType = insnType1;
  return r1;
}


/* CODE ADDED FOR FUNCTION RELOCATION */

/*************************************************************************/
/*************************************************************************/

// find the target of a jump or call

Address get_target(const unsigned char *instr, unsigned type, unsigned size,
                                                              Address addr) {
  int disp = displacement(instr, type);
  return (Address)(addr + size + disp);
}


// get the displacement of a jump or call

int displacement(const unsigned char *instr, unsigned type) {

  int disp = 0;

  if (type & IS_JUMP) {
    if (type & REL_B) {
      disp = *(const char *)(instr+1);
    } else if (type & REL_W) {
      disp = *(const short *)(instr+2); // skip prefix and opcode
    } else if (type & REL_D) {
      disp = *(const int *)(instr+1);
    }
  } else if (type & IS_JCC) {
    if (type & REL_B) {
      disp = *(const char *)(instr+1);
    } else if (type & REL_W) {
      disp = *(const short *)(instr+3); // skip prefix and two byte opcode
    } else if (type & REL_D) {
      disp = *(const int *)(instr+2);   // skip two byte opcode
    }
  } else if (type & IS_CALL) {
    if (type & REL_W) {
      disp = *(const short *)(instr+2); // skip prefix and opcode
    } else if (type & REL_D) {
      disp = *(const int *)(instr+1);
    }
  }
  
  return disp;
}


// get the displacement of a relative jump or call

int get_disp(instruction *insn) {
  return displacement(insn->ptr(), insn->type());
}


// if setDisp is true set the target of a relative jump or call insn
// otherwise, return the number of bytes needed to expand the present 
// instruction so that its target is in the range of its displacement. 
// In cases where we have a short near jump out of the function, we always 
// want to expand it to a 4 byte jump

int set_disp(bool setDisp, instruction *insn, int newOffset, bool outOfFunc) {

  unsigned char *instr = (const_cast<unsigned char *> (insn->ptr()));
  unsigned type = insn->type();

  if (!((type & IS_JUMP) || (type & IS_JCC) || (type & IS_CALL))) return 0; 
  
  if (type & IS_CALL) {
    if (type & REL_W) {
      if (outOfFunc) {
        return 2;  // go from 3 to 5 bytes;
      } else { 
          if (setDisp) {
            instr++;
            *((short *) instr) = (short) newOffset;
          }
          else return 0;
      }
    } else { 
        if (type & REL_D) {
          if (setDisp) {
            instr++;
            *((int *) instr) = (int) newOffset;
          }
          else return 0;
	}
    }
    return 0;
  }
 
  // assert((type & IS_JUMP) || (type & IS_JCC));

  if (type & REL_B) {
    if (*instr == JCXZ) {
      if (newOffset > 127 || newOffset < -128 || outOfFunc) {
	return 7;  // go from 2 to 9 bytes
      } else {
          if (setDisp) { 
            instr++;
            *((char *) instr) = (char) newOffset;
	  }
          else return 0;
      }
    } else {
        if (type & IS_JCC) {
          if (newOffset > 127 || newOffset < -128 || outOfFunc) {
 	    return 4;  // go from 2 to 6 bytes
          } else {
              if (setDisp) {
                instr++; 
                *((char *) instr) = (char) newOffset;
	      }
              else return 0;
	  }
	} else {
            if (type & IS_JUMP) {
              if (newOffset > 127 || newOffset < -128 || outOfFunc) {
	        return 3;  // go from 2 to 5 bytes
              } else {
		  if (setDisp) {
                    instr++;
                    *((char *) instr) = (char) newOffset;
	          }
                  else return 0;
	      }
	    }
	} 
    }   
  } else {
      if (type & REL_W) {
        if (newOffset > 32767 || newOffset < -32768 || outOfFunc) {
          return 1;  // drop the PREFIX_OPR, add 2 bytes for disp
	} else {
	    if (setDisp) {
              if (type & PREFIX_OPR)
                instr++;
              if (type & PREFIX_SEG)
                instr++;
              instr++; 
              *((short *) instr) = (short) newOffset;
 	    } else return 0;
	}
      } else {
	  if (setDisp) {
            if (type & REL_D) {
              if (type & PREFIX_SEG)
                instr++;
              if (*instr == 0x0F)
                instr++;
              instr++; 
              *((int *) instr) = (int) newOffset;
	    } 
          } else return 0;               
      }
  }
  return 0;
}

int addressOfMachineInsn(instruction *insn) {
  return (int)insn->ptr();
}

int sizeOfMachineInsn(instruction *insn) {
  return insn->size();
}

/*********************************************************************/
/*********************************************************************/



//This function decodes the addressing modes used for call instructions
//For documentation on these mode, and how to decode them, see tables
//2-2 and 2-3 from the intel software developers manual.
int get_instruction_operand(const unsigned char *ptr, Register& base_reg,
			    Register& index_reg, int& displacement, 
			    unsigned& scale, unsigned &Mod){
  x86_insn *desc;
  unsigned code;
  unsigned ModRMbyte;
  unsigned RM=0;
  unsigned REG;
  unsigned SIBbyte = 0;
  bool hasSIB;
  //Initialize default values to an appropriately devilish number 
  base_reg = 666;
  index_reg = 666;
  displacement = 0;
  scale = 0;
  Mod = 0;

  do {
    desc = &oneByteMap[*ptr++];
    code = desc->escape_code;
    if (code == PREFIX_INSTR 
	|| code == PREFIX_ADDR_SZ
        || code == PREFIX_OPR_SZ
	|| code == PREFIX_SEG_OVR){
      
      //We do not have support for handling the prefixes of x86 instructions
      return -1;
    }
    else
      // no more prefixes
      break;
  } while (true);
  
  if (code != Grp5) {
    //We should never get here, there should never be a non group5
    //indirect call instruction
    return -1;
  }
  
  //If the instruction has a mod/rm byte
  if (desc->hasModRM) {
    ModRMbyte = *ptr++;
    Mod = ModRMbyte >> 6;
    RM = ModRMbyte & 0x07;
    REG = (ModRMbyte >> 3) & 0x07;
    hasSIB = (Mod != 3) && (RM == 4);
    if (hasSIB) {
      SIBbyte = *ptr++;
    }

    if(REG == 2){
      if (Mod == 3){ //Standard call where address is in register
	base_reg = (Register) RM;
	return REGISTER_DIRECT;
      }
      else if (Mod == 2){
	displacement = *( ( const unsigned int * ) ptr );
	ptr+= wordSzB;
	if(hasSIB){
	  decode_SIB(SIBbyte, scale, index_reg, base_reg);
	  return SIB;
	}	
	else {
	  base_reg = (Register) RM;
	  return REGISTER_INDIRECT_DISPLACED; 
	}
      }
      else if(Mod == 1){//call with 8-bit signed displacement
	displacement = *( const char * )ptr;
	ptr+= byteSzB;
	if(hasSIB){
	  decode_SIB(SIBbyte, scale, index_reg, base_reg);
	  return SIB;
	}
	else {
	  base_reg = (Register) RM;
	  return REGISTER_INDIRECT_DISPLACED; 
	}
      }
      else if(Mod == 0){
	if(hasSIB){
	  decode_SIB(SIBbyte, scale, index_reg, base_reg);
	  if(base_reg == 5){
	    displacement = *( ( const unsigned int * )ptr );
	    ptr+= wordSzB;
	  }
	  else 
	    displacement = 0;
	  return SIB;
	}
	else if(RM == 5){ //The disp32 field from Table 2-2 of IA guide
	  displacement = *( ( const unsigned int * ) ptr );
	  ptr+= wordSzB;
	  return DISPLACED;
	}
	else {
	  base_reg = (Register) RM;
	  return REGISTER_INDIRECT;
	}
      }
    }
    else if(REG==3){ 
      //The call instruction is a far call, for which there
      //is no monitoring code
      return -1;
    }
    else assert(0);
  }
  

  return -1;
}  

//Determine appropriate scale, index, and base given SIB byte.
void decode_SIB(unsigned sib, unsigned& scale, Register& index_reg, Register& base_reg){
  scale = sib >> 6;
  
  //scale = 2^scale
  if(scale == 0)
    scale = 1;
  else if(scale == 1)
    scale = 2;
  else if(scale == 2)
    scale = 4;
  else if(scale == 3)
    scale = 8;

  index_reg = (sib >> 3) & 0x07;
  base_reg = sib & 0x07;
}

const unsigned char*
skip_headers(const unsigned char* addr, bool& isWordAddr,bool& isWordOp)
{
  isWordAddr = false;
  isWordOp = false;

  ia32_prefixes prefs;

  ia32_decode_prefixes(addr, prefs);
  if(prefs.getPrefix(2) == PREFIX_SZOPER)
    isWordOp = true;
  if(prefs.getPrefix(3) == PREFIX_SZADDR)
    isWordAddr = true;
  return addr+prefs.getCount();
}

bool insn_hasSIB(unsigned ModRMbyte,unsigned& Mod,unsigned& Reg,unsigned& RM){
    Mod = (ModRMbyte >> 6) & 0x03;
    Reg = (ModRMbyte >> 3) & 0x07;
    RM = ModRMbyte & 0x07;
    return ((Mod != 3) && ((RM == 4) || (RM == 5)));
}
