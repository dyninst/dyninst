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

// $Id: arch-x86.C,v 1.26 2005/02/02 16:57:19 gquinn Exp $
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

/* decode instruction at address addr, return size of instruction */
unsigned get_instruction(const unsigned char* addr, unsigned &insnType)
{
  int r1;

  ia32_instruction i;
#if defined(i386_unknown_nt4_0) && _MSC_VER < 1300
  ia32_decode(0, addr, i);
#else
  ia32_decode<0>(addr, i);
#endif
  r1 = i.getSize();
  insnType = ia32_emulate_old_type(i);

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

const unsigned char*
skip_headers(const unsigned char* addr, bool& isWordAddr,bool& isWordOp);

// get the displacement of a jump or call
int displacement(const unsigned char *instr, unsigned type) {

  int disp = 0;
  bool temp1, temp2;
  //skip prefix
  instr = skip_headers( instr, temp1, temp2 );
  
  if (type & IS_JUMP) {
    if (type & REL_B) {
      disp = *(const char *)(instr+1);
    } else if (type & REL_W) {
      disp = *(const short *)(instr+1); // skip opcode
    } else if (type & REL_D) {
      disp = *(const int *)(instr+1);
    }
  } else if (type & IS_JCC) {
    if (type & REL_B) {
      disp = *(const char *)(instr+1);
    } else if (type & REL_W) {
      disp = *(const short *)(instr+2); // skip two byte opcode
    } else if (type & REL_D) {
      disp = *(const int *)(instr+2);   // skip two byte opcode
    }
  } else if (type & IS_CALL) {
    if (type & REL_W) {
      disp = *(const short *)(instr+1); // skip opcode
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
  bool temp1, temp2;

  if (!((type & IS_JUMP) || (type & IS_JCC) || (type & IS_CALL))) return 0; 

  instr = const_cast<unsigned char *>(skip_headers(instr, temp1, temp2));

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
    return ((Mod != 3) && (RM == 4));
}

bool insn_hasDisp8(unsigned ModRMbyte){
    unsigned Mod = (ModRMbyte >> 6) & 0x03;
    return (Mod == 1);
}

bool insn_hasDisp32(unsigned ModRMbyte){
    unsigned Mod = (ModRMbyte >> 6) & 0x03;
    /* unsigned Reg = (ModRMbyte >> 3) & 0x07; */
    unsigned RM = ModRMbyte & 0x07;
    return (Mod == 0 && RM == 5) || (Mod == 2);
}

bool isStackFramePreamble( instruction& insn1 )
{
    //look for the following sequences in the gaps
    //1.
    //55      push  %ebp
    //89 35   mov   %esp, %ebp
    
    //2.
    //55      push  %ebp
    //xx xx   xxx   xxxx, xxxx  (can be any instruction)
    //89 35   mov   %esp, %ebp               
    
    instruction insn2, insn3;
    insn2.getNextInstruction( insn1.ptr() + insn1.size() );       
    insn3.getNextInstruction( insn2.ptr() + insn2.size() );

    const unsigned char* p = insn1.ptr();
    const unsigned char* q = insn2.ptr();
    const unsigned char* r = insn3.ptr();

    if( insn1.size() != 1 )
        return false;
    
    if( p[ 0 ] != PUSHEBP )
        return false;
       
    if( insn2.size() == 2 && q[0] == 0x89 && q[1] == 0xe5 )
        return true;

    if( insn3.size() == 2 && r[0] == 0x89 && r[1] == 0xe5 )
        return true;
    
    return false;
}

