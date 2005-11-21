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

// $Id: arch-x86.C,v 1.42 2005/11/21 17:16:12 jaw Exp $

// Official documentation used:    - IA-32 Intel Architecture Software Developer Manual (2001 ed.)
//                                 - AMD x86-64 Architecture Programmer's Manual (rev 3.00, 1/2002)
// Unofficial documentation used:  - www.sandpile.org/ia32
//                                 - NASM documentation

// Note: Unless specified "book" refers to Intel's manual

#include <assert.h>
#include <stdio.h>
#include "common/h/Types.h"
#include "arch.h"
#include "util.h"
#include "showerror.h"
#include "InstrucIter.h"

// tables and pseudotables
enum {
  t_ill=0, t_oneB, t_twoB, t_prefixedSSE, t_coprocEsc, t_grp, t_sse, t_grpsse, t_3dnow, t_done=99
};

// groups
enum {
  Grp1a=0, Grp1b, Grp1c, Grp1d, Grp2, Grp3a, Grp3b, Grp4, Grp5, Grp6, Grp7,
  Grp8, Grp9, Grp11, Grp12, Grp13, Grp14, Grp15, Grp16, GrpAMD
};

// SSE
enum {
  SSE10=0, SSE11, SSE12, SSE13, SSE14, SSE15, SSE16, SSE17,
  SSE28, SSE29, SSE2A, SSE2B, SSE2C, SSE2D, SSE2E, SSE2F,
  SSE50, SSE51, SSE52, SSE53, SSE54, SSE55, SSE56, SSE57,
  SSE58, SSE59, SSE5A, SSE5B, SSE5C, SSE5D, SSE5E, SSE5F,
  SSE60, SSE61, SSE62, SSE63, SSE64, SSE65, SSE66, SSE67,
  SSE68, SSE69, SSE6A, SSE6B, SSE6C, SSE6D, SSE6E, SSE6F,
  SSE70, SSE74, SSE75, SSE76,
  SSE7E, SSE7F,
  SSEC2, SSEC4, SSEC5, SSEC6,
  SSED1, SSED2, SSED3, SSED4, SSED5, SSED6, SSED7,
  SSED8, SSED9, SSEDA, SSEDB, SSEDC, SSEDD, SSEDE, SSEDF,
  SSEE0, SSEE1, SSEE2, SSEE3, SSEE4, SSEE5, SSEE6, SSEE7,
  SSEE8, SSEE9, SSEEA, SSEEB, SSEEC, SSEED, SSEEE, SSEEF,
  SSEF1, SSEF2, SSEF3, SSEF4, SSEF5, SSEF6, SSEF7,
  SSEF8, SSEF9, SSEFA, SSEFB, SSEFC, SSEFD, SSEFE
};

// SSE groups
enum {
  G12SSE010B=0, G12SSE100B, G12SSE110B,
  G13SSE010B, G13SSE100B, G13SSE110B,
  G14SSE010B, G14SSE011B, G14SSE110B, G14SSE111B,
};


#define Zz   { 0, 0 }
#define Ap   { am_A, op_p }
#define Cd   { am_C, op_d }
#define Dd   { am_D, op_d }
#define Eb   { am_E, op_b }
#define Ed   { am_E, op_d }
#define Ep   { am_E, op_p }
#define Ev   { am_E, op_v }
#define Ew   { am_E, op_w }
#define Fv   { am_F, op_v }
#define Gb   { am_G, op_b }
#define Gd   { am_G, op_d }
#define Gv   { am_G, op_v }
#define Gw   { am_G, op_w }
#define Ib   { am_I, op_b }
#define Iv   { am_I, op_v }
#define Iw   { am_I, op_w }
#define Iz   { am_I, op_z }
#define Jb   { am_J, op_b }
#define Jv   { am_J, op_v }
#define Jz   { am_J, op_z }
#define Ma   { am_M, op_a }
#define Mb   { am_M, op_b }
#define Mlea { am_M, op_lea }
#define Mp   { am_M, op_p }
#define Ms   { am_M, op_s }
#define Md   { am_M, op_d }
#define Mq   { am_M, op_q }
#define M512 { am_M, op_512 }
#define Ob   { am_O, op_b }
#define Ov   { am_O, op_v }
#define Pd   { am_P, op_d }
#define Pdq  { am_P, op_dq }
#define Ppi  { am_P, op_pi }
#define Pq   { am_P, op_q }
#define Qdq  { am_Q, op_dq }
#define Qd   { am_Q, op_d }
#define Qpi  { am_Q, op_pi }
#define Qq   { am_Q, op_q }
#define Rd   { am_R, op_d }
#define Td   { am_T, op_d }
#define Sw   { am_S, op_w }
#define Vdq  { am_V, op_dq }
#define Vpd  { am_V, op_pd }
#define Vps  { am_V, op_ps }
#define Vq   { am_V, op_q }
#define Vss  { am_V, op_ss }
#define Vsd  { am_V, op_sd }
#define Wdq  { am_W, op_dq }
#define Wpd  { am_W, op_pd }
#define Wps  { am_W, op_ps }
#define Wq   { am_W, op_q }
#define Ws   { am_W, op_s }
#define Wsd  { am_W, op_sd }
#define Wss  { am_W, op_ss }
#define Xb   { am_X, op_b }
#define Xv   { am_X, op_v }
#define Yb   { am_Y, op_b }
#define Yv   { am_Y, op_v }
#define STHb { am_stackH, op_b }
#define STPb { am_stackP, op_b }
#define STHv { am_stackH, op_v }
#define STPv { am_stackP, op_v }
#define STHw { am_stackH, op_w }
#define STPw { am_stackP, op_w }
#define STHd { am_stackH, op_d }
#define STPd { am_stackP, op_d }
#define STHa { am_stackH, op_allgprs }
#define STPa { am_stackP, op_allgprs }

#define STKb { am_stack, op_b }
#define STKv { am_stack, op_v }
#define STKw { am_stack, op_w }
#define STKd { am_stack, op_d }
#define STKa { am_stack, op_allgprs }


#define GPRS { am_allgprs, op_allgprs }

#define AH  { am_reg, r_AH }
#define AX  { am_reg, r_AX }
#define BH  { am_reg, r_BH }
#define CH  { am_reg, r_CH }
#define DH  { am_reg, r_DH }
#define AL  { am_reg, r_AL }
#define BL  { am_reg, r_BL }
#define CL  { am_reg, r_CL }
#define CS  { am_reg, r_CS }
#define DL  { am_reg, r_DL }
#define DX  { am_reg, r_DX }
#define eAX { am_reg, r_eAX }
#define eBX { am_reg, r_eBX }
#define eCX { am_reg, r_eCX }
#define eDX { am_reg, r_eDX }
#define EAX { am_reg, r_EAX }
#define EBX { am_reg, r_EBX }
#define ECX { am_reg, r_ECX }
#define EDX { am_reg, r_EDX }
#define DS  { am_reg, r_DS }
#define ES  { am_reg, r_ES }
#define FS  { am_reg, r_FS }
#define GS  { am_reg, r_GS }
#define SS  { am_reg, r_SS }
#define eSP { am_reg, r_eSP }
#define eBP { am_reg, r_eBP }
#define eSI { am_reg, r_eSI }
#define eDI { am_reg, r_eDI }
#define ESP { am_reg, r_ESP }
#define EBP { am_reg, r_EBP }
#define ESI { am_reg, r_ESI }
#define EDI { am_reg, r_EDI }
#define ECXEBX { am_reg, r_ECXEBX }
#define EDXEAX { am_reg, r_EDXEAX }


#define FPOS 16

enum {
  fNT=1,   // non-temporal
  fPREFETCHNT,
  fPREFETCHT0,
  fPREFETCHT1,
  fPREFETCHT2,
  fPREFETCHAMDE,
  fPREFETCHAMDW,
  fCALL,
  fNEARRET,
  fFARRET,
  fIRET,
  fENTER,
  fLEAVE,
  fXLAT,
  fIO,
  fSEGDESC,
  fCOND,
  fCMPXCH,
  fCMPXCH8,
  fINDIRCALL,
  fINDIRJUMP,
  fFXSAVE,
  fFXRSTOR,
  fCLFLUSH,
  fREP,   // only rep prefix allowed: ins, movs, outs, lods, stos
  fSCAS,
  fCMPS
};

// Modded table entry for push/pop, daa, das, aaa, aas, insb/w/d, outsb/w/d, xchg, cbw
// les, lds, aam, aad, loop(z/nz), cmpxch, lss, mul, imul, div, idiv, cmpxch8, [ld/st]mxcsr
// clflush, prefetch*


// oneByteMap: one byte opcode map
static ia32_entry oneByteMap[256] = {
  /* 00 */
  { "add",  t_done, 0, true, { Eb, Gb, Zz }, 0, s1RW2R },
  { "add",  t_done, 0, true, { Ev, Gv, Zz }, 0, s1RW2R },
  { "add",  t_done, 0, true, { Gb, Eb, Zz }, 0, s1RW2R },
  { "add",  t_done, 0, true, { Gv, Ev, Zz }, 0, s1RW2R },
  { "add",  t_done, 0, false, { AL, Ib, Zz }, 0, s1RW2R },
  { "add",  t_done, 0, false, { eAX, Iz, Zz }, 0, s1RW2R },
  { "push", t_done, 0, false, { STHw, ES, eSP }, 0, s1W2R3RW },
  { "pop",  t_done, 0, false, { ES, STPw, eSP }, 0, s1W2R3RW },
  /* 08 */
  { "or",   t_done, 0, true, { Eb, Gb, Zz }, 0, s1RW2R },
  { "or",   t_done, 0, true, { Ev, Gv, Zz }, 0, s1RW2R },
  { "or",   t_done, 0, true, { Gb, Eb, Zz }, 0, s1RW2R },
  { "or",   t_done, 0, true, { Gv, Ev, Zz }, 0, s1RW2R },
  { "or",   t_done, 0, false, { AL, Ib, Zz }, 0, s1RW2R },
  { "or",   t_done, 0, false, { eAX, Iz, Zz }, 0, s1RW2R },
  { "push", t_done, 0, false, { STHw, CS, eSP }, 0, s1W2R3RW },
  { 0,      t_twoB, 0, false, { Zz, Zz, Zz }, 0, 0 },
  /* 10 */
  { "adc",  t_done, 0, true, { Eb, Gb, Zz }, 0, s1RW2R },
  { "adc",  t_done, 0, true, { Ev, Gv, Zz }, 0, s1RW2R },
  { "adc",  t_done, 0, true, { Gb, Eb, Zz }, 0, s1RW2R },
  { "adc",  t_done, 0, true, { Gv, Ev, Zz }, 0, s1RW2R },
  { "adc",  t_done, 0, false, { AL, Ib, Zz }, 0, s1RW2R },
  { "adc",  t_done, 0, false, { eAX, Iz, Zz }, 0, s1RW2R },
  { "push", t_done, 0, false, { STHw, SS, eSP }, 0, s1W2R3RW },
  { "pop",  t_done, 0, false, { SS, STPw, eSP }, 0, s1W2R3RW },
  /* 18 */
  { "sbb",  t_done, 0, true, { Eb, Gb, Zz }, 0, s1RW2R },
  { "sbb",  t_done, 0, true, { Ev, Gv, Zz }, 0, s1RW2R },
  { "sbb",  t_done, 0, true, { Gb, Eb, Zz }, 0, s1RW2R },
  { "sbb",  t_done, 0, true, { Gv, Ev, Zz }, 0, s1RW2R },
  { "sbb",  t_done, 0, false, { AL, Ib, Zz }, 0, s1RW2R },
  { "sbb",  t_done, 0, false, { eAX, Iz, Zz }, 0, s1RW2R },
  { "push", t_done, 0, false, { STHw, DS, eSP }, 0, s1W2R3RW },
  { "pop" , t_done, 0, false, { DS, STPw, eSP }, 0, s1W2R3RW },
  /* 20 */
  { "and", t_done, 0, true, { Eb, Gb, Zz }, 0, s1RW2R },
  { "and", t_done, 0, true, { Ev, Gv, Zz }, 0, s1RW2R },
  { "and", t_done, 0, true, { Gb, Eb, Zz }, 0, s1RW2R },
  { "and", t_done, 0, true, { Gv, Ev, Zz }, 0, s1RW2R },
  { "and", t_done, 0, false, { AL, Ib, Zz }, 0, s1RW2R },
  { "and", t_done, 0, false, { eAX, Iz, Zz }, 0, s1RW2R },
  { 0, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 }, // PREFIX_SEG_OVR
  { "daa", t_done, 0, false, { AL, Zz, Zz }, 0, s1RW },
  /* 28 */
  { "sub", t_done, 0, true, { Eb, Gb, Zz }, 0, s1RW2R },
  { "sub", t_done, 0, true, { Ev, Gv, Zz }, 0, s1RW2R },
  { "sub", t_done, 0, true, { Gb, Eb, Zz }, 0, s1RW2R },
  { "sub", t_done, 0, true, { Gv, Ev, Zz }, 0, s1RW2R },
  { "sub", t_done, 0, false, { AL, Ib, Zz }, 0, s1RW2R },
  { "sub", t_done, 0, false, { eAX, Iz, Zz }, 0, s1RW2R },
  { 0, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 }, // PREFIX_SEG_OVR
  { "das" , t_done, 0, false, { AL, Zz, Zz }, 0, s1RW },
  /* 30 */
  { "xor", t_done, 0, true, { Eb, Gb, Zz }, 0, s1RW2R },
  { "xor", t_done, 0, true, { Ev, Gv, Zz }, 0, s1RW2R },
  { "xor", t_done, 0, true, { Gb, Eb, Zz }, 0, s1RW2R },
  { "xor", t_done, 0, true, { Gv, Ev, Zz }, 0, s1RW2R },
  { "xor", t_done, 0, false, { AL, Ib, Zz }, 0, s1RW2R },
  { "xor", t_done, 0, false, { eAX, Iz, Zz }, 0, s1RW2R },
  { 0, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 }, // PREFIX_SEG_OVR
  { "aaa", t_done, 0, false, { AX, Zz, Zz }, 0, s1RW2R },
  /* 38 */
  { "cmp", t_done, 0, true, { Eb, Gb, Zz }, 0, s1R2R },
  { "cmp", t_done, 0, true, { Ev, Gv, Zz }, 0, s1R2R },
  { "cmp", t_done, 0, true, { Gb, Eb, Zz }, 0, s1R2R },
  { "cmp", t_done, 0, true, { Gv, Ev, Zz }, 0, s1R2R },
  { "cmp", t_done, 0, false, { AL, Ib, Zz }, 0, s1R2R },
  { "cmp", t_done, 0, false, { eAX, Iz, Zz }, 0, s1R2R },
  { 0,     t_ill,  0, false, { Zz, Zz, Zz }, 0, 0 }, // PREFIX_SEG_OVR
  { "aas", t_done, 0, false, { AX, Zz, Zz }, 0, s1RW },
  /* 40 */
  { "inc", t_done, 0, false, { eAX, Zz, Zz }, 0, s1RW },
  { "inc", t_done, 0, false, { eCX, Zz, Zz }, 0, s1RW },
  { "inc", t_done, 0, false, { eDX, Zz, Zz }, 0, s1RW },
  { "inc", t_done, 0, false, { eBX, Zz, Zz }, 0, s1RW },
  { "inc", t_done, 0, false, { eSP, Zz, Zz }, 0, s1RW },
  { "inc", t_done, 0, false, { eBP, Zz, Zz }, 0, s1RW },
  { "inc", t_done, 0, false, { eSI, Zz, Zz }, 0, s1RW },
  { "inc", t_done, 0, false, { eDI, Zz, Zz }, 0, s1RW },
  /* 48 */
  { "dec", t_done, 0, false, { eAX, Zz, Zz }, 0, s1RW },
  { "dec", t_done, 0, false, { eCX, Zz, Zz }, 0, s1RW },
  { "dec", t_done, 0, false, { eDX, Zz, Zz }, 0, s1RW },
  { "dec", t_done, 0, false, { eBX, Zz, Zz }, 0, s1RW },
  { "dec", t_done, 0, false, { eSP, Zz, Zz }, 0, s1RW },
  { "dec", t_done, 0, false, { eBP, Zz, Zz }, 0, s1RW },
  { "dec", t_done, 0, false, { eSI, Zz, Zz }, 0, s1RW },
  { "dec", t_done, 0, false, { eDI, Zz, Zz }, 0, s1RW },
  /* 50 */
  { "push", t_done, 0, false, { STHv, eAX, eSP }, 0, s1W2R3RW },
  { "push", t_done, 0, false, { STHv, eCX, eSP }, 0, s1W2R3RW },
  { "push", t_done, 0, false, { STHv, eDX, eSP }, 0, s1W2R3RW },
  { "push", t_done, 0, false, { STHv, eBX, eSP }, 0, s1W2R3RW },
  { "push", t_done, 0, false, { STHv, eSP, eSP }, 0, s1W2R3RW },
  { "push", t_done, 0, false, { STHv, eBP, eSP }, 0, s1W2R3RW },
  { "push", t_done, 0, false, { STHv, eSI, eSP }, 0, s1W2R3RW },
  { "push", t_done, 0, false, { STHv, eDI, eSP }, 0, s1W2R3RW },
  /* 58 */
  { "pop", t_done, 0, false, { eAX, STPv, eSP }, 0, s1W2R3RW },
  { "pop", t_done, 0, false, { eCX, STPv, eSP }, 0, s1W2R3RW },
  { "pop", t_done, 0, false, { eDX, STPv, eSP }, 0, s1W2R3RW },
  { "pop", t_done, 0, false, { eBX, STPv, eSP }, 0, s1W2R3RW },
  { "pop", t_done, 0, false, { eSP, STPv, eSP }, 0, s1W2R3RW },
  { "pop", t_done, 0, false, { eBP, STPv, eSP }, 0, s1W2R3RW },
  { "pop", t_done, 0, false, { eSI, STPv, eSP }, 0, s1W2R3RW },
  { "pop", t_done, 0, false, { eDI, STPv, eSP }, 0, s1W2R3RW },
  /* 60 */
  { "pusha(d)", t_done, 0, false, { STHa, GPRS, eSP }, 0, s1W2R3RW },
  { "popa(d)",  t_done, 0, false, { GPRS, STPa, eSP }, 0, s1W2R3RW },
  { "bound",    t_done, 0, true, { Gv, Ma, Zz }, 0, s1R2R },
  { "arpl",     t_done, 0, true, { Ew, Gw, Zz }, 0, s1R2R },
  { 0,          t_ill,  0, false, { Zz, Zz, Zz }, 0, 0 }, // PREFIX_SEG_OVR
  { 0,          t_ill,  0, false, { Zz, Zz, Zz }, 0, 0 }, // PREFIX_SEG_OVR
  { 0,   t_prefixedSSE, 2, false, { Zz, Zz, Zz }, 0, 0 }, /* operand size prefix (PREFIX_OPR_SZ)*/
  { 0,          t_ill,  0, false, { Zz, Zz, Zz }, 0, 0 }, /* address size prefix (PREFIX_ADDR_SZ)*/
  /* 68 */
  { "push",    t_done, 0, false, { STHv, Iz, eSP }, 0, s1W2R3RW },
  { "imul",    t_done, 0, true, { Gv, Ev, Iz }, 0, s1W2R3R },
  { "push",    t_done, 0, false, { STHb, Ib, eSP }, 0, s1W2R3RW },
  { "imul",    t_done, 0, true, { Gv, Ev, Ib }, 0, s1W2R3R },
  { "insb",    t_done, 0, false, { Yb, DX, Zz }, 0, s1W2R | (fREP << FPOS) }, // (e)SI/DI changed
  { "insw/d",  t_done, 0, false, { Yv, DX, Zz }, 0, s1W2R | (fREP << FPOS) },
  { "outsb",   t_done, 0, false, { DX, Xb, Zz }, 0, s1W2R | (fREP << FPOS) },
  { "outsw/d", t_done, 0, false, { DX, Xv, Zz }, 0, s1W2R | (fREP << FPOS) },
  /* 70 */
  { "jo",         t_done, 0, false, { Jb, Zz, Zz }, (IS_JCC | REL_B), s1R },
  { "jno",        t_done, 0, false, { Jb, Zz, Zz }, (IS_JCC | REL_B), s1R },
  { "jb/jnaej/j", t_done, 0, false, { Jb, Zz, Zz }, (IS_JCC | REL_B), s1R },
  { "jnb/jae/j",  t_done, 0, false, { Jb, Zz, Zz }, (IS_JCC | REL_B), s1R },
  { "jz",         t_done, 0, false, { Jb, Zz, Zz }, (IS_JCC | REL_B), s1R },
  { "jnz",        t_done, 0, false, { Jb, Zz, Zz }, (IS_JCC | REL_B), s1R },
  { "jbe",        t_done, 0, false, { Jb, Zz, Zz }, (IS_JCC | REL_B), s1R },
  { "jnbe",       t_done, 0, false, { Jb, Zz, Zz }, (IS_JCC | REL_B), s1R },
  /* 78 */
  { "js",   t_done, 0, false, { Jb, Zz, Zz }, (IS_JCC | REL_B), s1R },
  { "jns",  t_done, 0, false, { Jb, Zz, Zz }, (IS_JCC | REL_B), s1R },
  { "jp",   t_done, 0, false, { Jb, Zz, Zz }, (IS_JCC | REL_B), s1R },
  { "jnp",  t_done, 0, false, { Jb, Zz, Zz }, (IS_JCC | REL_B), s1R },
  { "jl",   t_done, 0, false, { Jb, Zz, Zz }, (IS_JCC | REL_B), s1R },
  { "jnl",  t_done, 0, false, { Jb, Zz, Zz }, (IS_JCC | REL_B), s1R },
  { "jle",  t_done, 0, false, { Jb, Zz, Zz }, (IS_JCC | REL_B), s1R },
  { "jnle", t_done, 0, false, { Jb, Zz, Zz }, (IS_JCC | REL_B), s1R },
  /* 80 */
  { 0, t_grp, Grp1a, true, { Zz, Zz, Zz }, 0, 0 },
  { 0, t_grp, Grp1b, true, { Zz, Zz, Zz }, 0, 0 },
  { 0, t_grp, Grp1c, true, { Zz, Zz, Zz }, 0, 0 }, // book says Grp1 however;sandpile.org agrees.
  { 0, t_grp, Grp1d, true, { Zz, Zz, Zz }, 0, 0 },
  { "test", t_done, 0, true, { Eb, Gb, Zz }, 0, s1R2R },
  { "test", t_done, 0, true, { Ev, Gv, Zz }, 0, s1R2R },
  { "xchg", t_done, 0, true, { Eb, Gb, Zz }, 0, s1RW2RW },
  { "xchg", t_done, 0, true, { Ev, Gv, Zz }, 0, s1RW2RW },
  /* 88 */
  { "mov", t_done, 0, true, { Eb, Gb, Zz }, 0, s1W2R },
  { "mov", t_done, 0, true, { Ev, Gv, Zz }, 0, s1W2R },
  { "mov", t_done, 0, true, { Gb, Eb, Zz }, 0, s1W2R },
  { "mov", t_done, 0, true, { Gv, Ev, Zz }, 0, s1W2R },
  { "mov", t_done, 0, true, { Ew, Sw, Zz }, 0, s1W2R },
  { "lea", t_done, 0, true, { Gv, Mlea, Zz }, 0, s1W }, // this is just M in the book
                                                        // AFAICT the 2nd operand is not accessed
  { "mov", t_done, 0, true, { Sw, Ew, Zz }, 0, s1W2R },
  { "pop", t_done, 0, true, { Ev, STPv, eSP }, 0, s1W2R3RW },
  /* 90 */
  { "nop",  t_done, 0, false, { Zz, Zz, Zz }, 0, sNONE }, // actually xchg eax,eax
  { "xchg", t_done, 0, false, { eCX, eAX, Zz }, 0, s1RW2RW },
  { "xchg", t_done, 0, false, { eDX, eAX, Zz }, 0, s1RW2RW },
  { "xchg", t_done, 0, false, { eBX, eAX, Zz }, 0, s1RW2RW },
  { "xchg", t_done, 0, false, { eSP, eAX, Zz }, 0, s1RW2RW },
  { "xchg", t_done, 0, false, { eBP, eAX, Zz }, 0, s1RW2RW },
  { "xchg", t_done, 0, false, { eSI, eAX, Zz }, 0, s1RW2RW },
  { "xchg", t_done, 0, false, { eDI, eAX, Zz }, 0, s1RW2RW },
  /* 98 */
  { "cbw/cwde", t_done, 0, false, { eAX, Zz, Zz }, 0, s1RW },
  { "cwd/cdq",  t_done, 0, false, { eDX, eAX, Zz }, 0, s1W2R },
  { "call",     t_done, 0, false, { Ap, Zz, Zz }, IS_CALL | PTR_WX, s1R },
  { "wait",     t_done, 0, false, { Zz, Zz, Zz }, 0, sNONE },
  { "pushf(d)", t_done, 0, false, { STHv, Fv, eSP }, 0, s1W2R3RW },
  { "popf(d)",  t_done, 0, false, { Fv, STPv, eSP }, 0, s1W2R3RW },
  { "sahf",     t_done, 0, false, { Zz, Zz, Zz }, 0, 0 }, // FIXME Intel
  { "lahf",     t_done, 0, false, { Zz, Zz, Zz }, 0, 0 }, // FIXME Intel
  /* A0 */
  { "mov",   t_done, 0, false, { AL, Ob, Zz },  0, s1W2R },
  { "mov",   t_done, 0, false, { eAX, Ov, Zz }, 0, s1W2R },
  { "mov",   t_done, 0, false, { Ob, AL, Zz },  0, s1W2R },
  { "mov",   t_done, 0, false, { Ov, eAX, Zz }, 0, s1W2R },
  // XXX: Xv is source, Yv is destination for movs, so they're swapped!
  { "movsb", t_done, 0, false, { Yb, Xb, Zz },  0, s1W2R | (fREP << FPOS) }, // (e)SI/DI changed
  { "movsw/d", t_done, 0, false, { Yv, Xv, Zz }, 0, s1W2R | (fREP << FPOS) },
  { "cmpsb", t_done, 0, false, { Xb, Yb, Zz },  0, s1R2R | (fCMPS << FPOS) },
  { "cmpsw", t_done, 0, false, { Xv, Yv, Zz },  0, s1R2R | (fCMPS << FPOS) },
  /* A8 */
  { "test",     t_done, 0, false, { AL, Ib, Zz },  0, s1R2R },
  { "test",     t_done, 0, false, { eAX, Iz, Zz }, 0, s1R2R },
  { "stosb",    t_done, 0, false, { Yb, AL, Zz },  0, s1W2R | (fREP << FPOS) },
  { "stosw/d",  t_done, 0, false, { Yv, eAX, Zz }, 0, s1W2R | (fREP << FPOS) },
  { "lodsb",    t_done, 0, false, { AL, Xb, Zz },  0, s1W2R | (fREP << FPOS) },
  { "lodsw",    t_done, 0, false, { eAX, Xv, Zz }, 0, s1W2R | (fREP << FPOS) },
  { "scasb",    t_done, 0, false, { AL, Yb, Zz },  0, s1R2R | (fSCAS << FPOS) },
  { "scasw/d",  t_done, 0, false, { eAX, Yv, Zz }, 0, s1R2R | (fSCAS << FPOS) },
  /* B0 */
  { "mov", t_done, 0, false, { AL, Ib, Zz }, 0, s1W2R },
  { "mov", t_done, 0, false, { CL, Ib, Zz }, 0, s1W2R },
  { "mov", t_done, 0, false, { DL, Ib, Zz }, 0, s1W2R },
  { "mov", t_done, 0, false, { BL, Ib, Zz }, 0, s1W2R },
  { "mov", t_done, 0, false, { AH, Ib, Zz }, 0, s1W2R },
  { "mov", t_done, 0, false, { CH, Ib, Zz }, 0, s1W2R },
  { "mov", t_done, 0, false, { DH, Ib, Zz }, 0, s1W2R },
  { "mov", t_done, 0, false, { BH, Ib, Zz }, 0, s1W2R },
  /* B8 */
  { "mov", t_done, 0, false, { eAX, Iv, Zz }, 0, s1W2R },
  { "mov", t_done, 0, false, { eCX, Iv, Zz }, 0, s1W2R },
  { "mov", t_done, 0, false, { eDX, Iv, Zz }, 0, s1W2R },
  { "mov", t_done, 0, false, { eBX, Iv, Zz }, 0, s1W2R },
  { "mov", t_done, 0, false, { eSP, Iv, Zz }, 0, s1W2R },
  { "mov", t_done, 0, false, { eBP, Iv, Zz }, 0, s1W2R },
  { "mov", t_done, 0, false, { eSI, Iv, Zz }, 0, s1W2R },
  { "mov", t_done, 0, false, { eDI, Iv, Zz }, 0, s1W2R },
  /* C0 */
  { 0, t_grp, Grp2, true, { Eb, Ib, Zz }, 0, s1RW2R },
  { 0, t_grp, Grp2, true, { Ev, Ib, Zz }, 0, s1RW2R },
  { "ret near", t_done, 0, false, { Iw, Zz, Zz }, (IS_RET), s1R | (fNEARRET << FPOS) },
  { "ret near", t_done, 0, false, { Zz, Zz, Zz }, (IS_RET), fNEARRET << FPOS },
  { "les",      t_done, 0, true, { ES, Gv, Mp }, 0, s1W2W3R },
  { "lds",      t_done, 0, true, { DS, Gv, Mp }, 0, s1W2W3R },
  { 0, t_grp, Grp11, true, { Eb, Ib, Zz }, 0, s1W2R },
  { 0, t_grp, Grp11, true, { Ev, Iz, Zz }, 0, s1W2R },
  /* C8 */
  { "enter",   t_done, 0, false, { Iw, Ib, Zz }, 0, fENTER << FPOS },
  { "leave",   t_done, 0, false, { Zz, Zz, Zz }, 0, fLEAVE << FPOS },
  { "ret far", t_done, 0, false, { Iw, Zz, Zz }, (IS_RETF), fFARRET << FPOS },
  { "ret far", t_done, 0, false, { Zz, Zz, Zz }, (IS_RETF), fFARRET << FPOS },
  { "int 3",   t_done, 0, false, { Zz, Zz, Zz }, 0, sNONE },
  { "int",     t_done, 0, false, { Ib, Zz, Zz }, 0, sNONE },
  { "into",    t_done, 0, false, { Zz, Zz, Zz }, 0, sNONE },
  { "iret",    t_done, 0, false, { Zz, Zz, Zz }, (IS_RET), fIRET << FPOS},
  /* D0 */
  { 0, t_grp, Grp2, true, { Eb, Zz, Zz }, 0, s1RW }, // const1
  { 0, t_grp, Grp2, true, { Ev, Zz, Zz }, 0, s1RW }, // --"--
  { 0, t_grp, Grp2, true, { Eb, CL, Zz }, 0, s1RW2R },
  { 0, t_grp, Grp2, true, { Ev, CL, Zz }, 0, s1RW2R },
  { "aam",  t_done, 0, false, { AX, Ib, Zz }, 0, s1RW2R },
  { "aad",  t_done, 0, false, { AX, Ib, Zz }, 0, s1RW2R },
  { "salc", t_done, 0, false, { Zz, Zz, Zz }, 0, sNONE }, // undocumeted
  { "xlat", t_done, 0, false, { Zz, Zz, Zz }, 0, fXLAT << FPOS }, // scream
  /* D8 */
  { 0, t_coprocEsc, 0, true, { Zz, Zz, Zz }, 0, 0 },
  { 0, t_coprocEsc, 0, true, { Zz, Zz, Zz }, 0, 0 },
  { 0, t_coprocEsc, 0, true, { Zz, Zz, Zz }, 0, 0 },
  { 0, t_coprocEsc, 0, true, { Zz, Zz, Zz }, 0, 0 },
  { 0, t_coprocEsc, 0, true, { Zz, Zz, Zz }, 0, 0 },
  { 0, t_coprocEsc, 0, true, { Zz, Zz, Zz }, 0, 0 },
  { 0, t_coprocEsc, 0, true, { Zz, Zz, Zz }, 0, 0 },
  { 0, t_coprocEsc, 0, true, { Zz, Zz, Zz }, 0, 0 },
  /* E0 */
  { "loopn",    t_done, 0, false, { Jb, eCX, Zz }, 0, s1R2R }, // aren't these conditional jumps?
  { "loope",    t_done, 0, false, { Jb, eCX, Zz }, 0, s1R2R },
  { "loop",     t_done, 0, false, { Jb, eCX, Zz }, 0, s1R2R },
  { "jcxz/jec", t_done, 0, false, { Jb, eCX, Zz }, (IS_JCC | REL_B), s1R2R },
  { "in",       t_done, 0, false, { AL, Ib, Zz }, 0, s1W2R | fIO << FPOS },
  { "in",       t_done, 0, false, { eAX, Ib, Zz }, 0, s1W2R | fIO << FPOS },
  { "out",      t_done, 0, false, { Ib, AL, Zz }, 0, s1W2R | fIO << FPOS },
  { "out",      t_done, 0, false, { Ib, eAX, Zz }, 0, s1W2R | fIO << FPOS },
  /* E8 */
  { "call", t_done, 0, false, { Jz, Zz, Zz }, (IS_CALL | REL_X), fCALL << FPOS },
  { "jmp",  t_done, 0, false, { Jz, Zz, Zz }, (IS_JUMP | REL_X), s1R },
  { "jmp",  t_done, 0, false, { Ap, Zz, Zz }, (IS_JUMP | PTR_WX), s1R },
  { "jmp",  t_done, 0, false, { Jb, Zz, Zz }, (IS_JUMP | REL_B), s1R },
  { "in",   t_done, 0, false, { AL, DX, Zz }, 0, s1W2R | (fIO << FPOS) },
  { "in",   t_done, 0, false, { eAX, DX, Zz }, 0, s1W2R | (fIO << FPOS) },
  { "out",  t_done, 0, false, { DX, AL, Zz }, 0, s1W2R | (fIO << FPOS) },
  { "out",  t_done, 0, false, { DX, eAX, Zz }, 0, s1W2R | (fIO << FPOS) },
  /* F0 */
  { 0,      t_ill,  0, false, { Zz, Zz, Zz }, 0, 0 }, // PREFIX_INSTR
  { "int1", t_done, 0, false, { Zz, Zz, Zz }, 0, sNONE }, // undocumented
  { 0, t_prefixedSSE, 3, false, { Zz, Zz, Zz }, 0, 0 },
  { 0, t_prefixedSSE, 1, false, { Zz, Zz, Zz }, 0, 0 },
  { "hlt",  t_done, 0, false, { Zz, Zz, Zz }, 0, sNONE },
  { "cmc",  t_done, 0, false, { Zz, Zz, Zz }, 0, sNONE },
  { 0, t_grp, Grp3a, true, { Zz, Zz, Zz }, 0, sNONE },
  { 0, t_grp, Grp3b, true, { Zz, Zz, Zz }, 0, sNONE },
  /* F8 */
  { "clc", t_done, 0, false, { Zz, Zz, Zz }, 0, sNONE },
  { "stc", t_done, 0, false, { Zz, Zz, Zz }, 0, sNONE },
  { "cli", t_done, 0, false, { Zz, Zz, Zz }, 0, sNONE },
  { "sti", t_done, 0, false, { Zz, Zz, Zz }, 0, sNONE },
  { "cld", t_done, 0, false, { Zz, Zz, Zz }, 0, sNONE },
  { "std", t_done, 0, false, { Zz, Zz, Zz }, 0, sNONE },
  { 0, t_grp, Grp4, true, { Zz, Zz, Zz }, 0, sNONE },
  { 0, t_grp, Grp5, true, { Zz, Zz, Zz }, 0, sNONE }
};


// twoByteMap: two byte opcode instructions (first byte is 0x0F)
static ia32_entry twoByteMap[256] = {
  /* 00 */
  { 0, t_grp, Grp6, true, { Zz, Zz, Zz }, 0, 0 },
  { 0, t_grp, Grp7, false, { Zz, Zz, Zz }, 0, 0 },
  { "lar",        t_done, 0, true, { Gv, Ew, Zz }, 0, s1W2R | (fSEGDESC << FPOS) },
  { "lsl",        t_done, 0, true, { Gv, Ew, Zz }, 0, s1W2R | (fSEGDESC << FPOS) },
  { 0,            t_ill,  0, false, { Zz, Zz, Zz }, 0, 0 },
  { "syscall",    t_done, 0, false, { Zz, Zz, Zz }, 0, sNONE }, // AMD; XXX: fixme for kernel work
  { "clts",       t_done, 0, false, { Zz, Zz, Zz }, 0, sNONE },
  { "sysret",     t_done, 0, false, { Zz, Zz, Zz }, 0, sNONE }, // AMD; XXX: fixme for kernel work
  /* 08 */
  { "invd",   t_done, 0, false, { Zz, Zz, Zz }, 0, sNONE }, // only in priviledge 0, so ignored
  { "wbinvd", t_done, 0, false, { Zz, Zz, Zz }, 0, sNONE }, // idem
  { 0,        t_ill,  0, false, { Zz, Zz, Zz }, 0, 0 },
  { "ud2",    t_ill,  0, 0, { Zz, Zz, Zz }, 0, 0 },
  { 0,        t_ill,  0, 0, { Zz, Zz, Zz }, 0, 0 },
  { "prefetch(w)", t_grp, GrpAMD, true, { Zz, Zz, Zz }, 0, 0 },    // AMD prefetch group
  { "femms",       t_done,  0, false, { Zz, Zz, Zz }, 0, sNONE },  // AMD specific
  // semantic is bogus for the 1st operand - but correct for the 2nd,
  // which is the only one that can be a memory operand :)
  // fixing the 1st operand requires an extra table for the 3dnow instructions...
  { 0,             t_3dnow, 0, true,  { Pq, Qq, Zz }, 0, s1RW2R }, // AMD 3DNow! suffixes
  /* 10 */
  { 0, t_sse, SSE10, true, { Zz, Zz, Zz }, 0, 0 },
  { 0, t_sse, SSE11, true, { Zz, Zz, Zz }, 0, 0 },
  { 0, t_sse, SSE12, true, { Zz, Zz, Zz }, 0, 0 },
  { 0, t_sse, SSE13, true, { Zz, Zz, Zz }, 0, 0 },
  { 0, t_sse, SSE14, true, { Zz, Zz, Zz }, 0, 0 },
  { 0, t_sse, SSE15, true, { Zz, Zz, Zz }, 0, 0 },
  { 0, t_sse, SSE16, true, { Zz, Zz, Zz }, 0, 0 },
  { 0, t_sse, SSE17, true, { Zz, Zz, Zz }, 0, 0 },
  /* 18 */
  { 0, t_grp, Grp16, 0, { Zz, Zz, Zz }, 0, 0 },
  { 0, t_ill, 0, 0, { Zz, Zz, Zz }, 0, 0 },
  { 0, t_ill, 0, 0, { Zz, Zz, Zz }, 0, 0 },
  { 0, t_ill, 0, 0, { Zz, Zz, Zz }, 0, 0 },
  { 0, t_ill, 0, 0, { Zz, Zz, Zz }, 0, 0 },
  { 0, t_ill, 0, 0, { Zz, Zz, Zz }, 0, 0 },
  { 0, t_ill, 0, 0, { Zz, Zz, Zz }, 0, 0 },
  { 0, t_ill, 0, 0, { Zz, Zz, Zz }, 0, 0 },
  /* 20 */
  { "mov", t_done, 0, true, { Rd, Cd, Zz }, 0, s1W2R },
  { "mov", t_done, 0, true, { Rd, Dd, Zz }, 0, s1W2R },
  { "mov", t_done, 0, true, { Cd, Rd, Zz }, 0, s1W2R },
  { "mov", t_done, 0, true, { Dd, Rd, Zz }, 0, s1W2R },
  { "mov", t_done, 0, true, { Rd, Td, Zz }, 0, s1W2R },
  { 0,     t_ill,  0, 0, { Zz, Zz, Zz }, 0, 0 },
  { "mov", t_done, 0, true, { Td, Rd, Zz }, 0, s1W2R },
  { 0,     t_ill,  0, 0, { Zz, Zz, Zz }, 0, 0 },
  /* 28 */
  { 0, t_sse, SSE28, true, { Zz, Zz, Zz }, 0, 0 },
  { 0, t_sse, SSE29, true, { Zz, Zz, Zz }, 0, 0 },
  { 0, t_sse, SSE2A, true, { Zz, Zz, Zz }, 0, 0 },
  { 0, t_sse, SSE2B, true, { Zz, Zz, Zz }, 0, 0 },
  { 0, t_sse, SSE2C, true, { Zz, Zz, Zz }, 0, 0 },
  { 0, t_sse, SSE2D, true, { Zz, Zz, Zz }, 0, 0 },
  { 0, t_sse, SSE2E, true, { Zz, Zz, Zz }, 0, 0 },
  { 0, t_sse, SSE2F, true, { Zz, Zz, Zz }, 0, 0 },
  /* 30 */
  { "wrmsr", t_done, 0, false, { Zz, Zz, Zz }, 0, sNONE },
  { "rdtsc", t_done, 0, false, { Zz, Zz, Zz }, 0, sNONE},
  { "rdmsr", t_done, 0, false, { Zz, Zz, Zz }, 0, sNONE },
  { "rdpmc", t_done, 0, false, { Zz, Zz, Zz }, 0, sNONE },
  { "sysenter", t_done, 0, false, { Zz, Zz, Zz }, 0, sNONE }, // XXX: fixme for kernel work
  { "sysexit",  t_done, 0, false, { Zz, Zz, Zz }, 0, sNONE }, // XXX: fixme for kernel work
  { 0, t_ill, 0, 0, { Zz, Zz, Zz }, 0, 0 }, 
  { 0, t_ill, 0, 0, { Zz, Zz, Zz }, 0 ,0 },
  /* 38 */
  { 0, t_ill, 0, 0, { Zz, Zz, Zz }, 0 ,0 },
  { 0, t_ill, 0, 0, { Zz, Zz, Zz }, 0 ,0 },
  { 0, t_ill, 0, 0, { Zz, Zz, Zz }, 0 ,0 },
  { 0, t_ill, 0, 0, { Zz, Zz, Zz }, 0 ,0 },
  { 0, t_ill, 0, 0, { Zz, Zz, Zz }, 0 ,0 },
  { 0, t_ill, 0, 0, { Zz, Zz, Zz }, 0 ,0 },
  { 0, t_ill, 0, 0, { Zz, Zz, Zz }, 0 ,0 },
  { 0, t_ill, 0, 0, { Zz, Zz, Zz }, 0 ,0 },
  /* 40 */
  { "cmovo",   t_done, 0, true, { Gv, Ev, Zz }, 0, s1W2R | (fCOND << FPOS) },
  { "cmovno",  t_done, 0, true, { Gv, Ev, Zz }, 0, s1W2R | (fCOND << FPOS) },
  { "cmovnae", t_done, 0, true, { Gv, Ev, Zz }, 0, s1W2R | (fCOND << FPOS) },
  { "cmovnb",  t_done, 0, true, { Gv, Ev, Zz }, 0, s1W2R | (fCOND << FPOS) },
  { "cmove",   t_done, 0, true, { Gv, Ev, Zz }, 0, s1W2R | (fCOND << FPOS) },
  { "cmovne",  t_done, 0, true, { Gv, Ev, Zz }, 0, s1W2R | (fCOND << FPOS) },
  { "cmovbe",  t_done, 0, true, { Gv, Ev, Zz }, 0, s1W2R | (fCOND << FPOS) },
  { "cmovnbe", t_done, 0, true, { Gv, Ev, Zz }, 0, s1W2R | (fCOND << FPOS) },
  /* 48 */
  { "cmovs",   t_done, 0, true, { Gv, Ev, Zz }, 0, s1W2R | (fCOND << FPOS) },
  { "cmovns",  t_done, 0, true, { Gv, Ev, Zz }, 0, s1W2R | (fCOND << FPOS) },
  { "cmovpe",  t_done, 0, true, { Gv, Ev, Zz }, 0, s1W2R | (fCOND << FPOS) },
  { "cmovpo",  t_done, 0, true, { Gv, Ev, Zz }, 0, s1W2R | (fCOND << FPOS) },
  { "cmovnge", t_done, 0, true, { Gv, Ev, Zz }, 0, s1W2R | (fCOND << FPOS) },
  { "cmovnl",  t_done, 0, true, { Gv, Ev, Zz }, 0, s1W2R | (fCOND << FPOS) },
  { "cmovng",  t_done, 0, true, { Gv, Ev, Zz }, 0, s1W2R | (fCOND << FPOS) },
  { "cmovnl",  t_done, 0, true, { Gv, Ev, Zz }, 0, s1W2R | (fCOND << FPOS) },
  /* 50 */
  { 0, t_sse, SSE50, true, { Zz, Zz, Zz }, 0, 0 },
  { 0, t_sse, SSE51, true, { Zz, Zz, Zz }, 0, 0 },
  { 0, t_sse, SSE52, true, { Zz, Zz, Zz }, 0, 0 },
  { 0, t_sse, SSE53, true, { Zz, Zz, Zz }, 0, 0 },
  { 0, t_sse, SSE54, true, { Zz, Zz, Zz }, 0, 0 },
  { 0, t_sse, SSE55, true, { Zz, Zz, Zz }, 0, 0 },
  { 0, t_sse, SSE56, true, { Zz, Zz, Zz }, 0, 0 },
  { 0, t_sse, SSE57, true, { Zz, Zz, Zz }, 0, 0 },
  /* 58 */
  { 0, t_sse, SSE58, true, { Zz, Zz, Zz }, 0, 0 },
  { 0, t_sse, SSE59, true, { Zz, Zz, Zz }, 0, 0 },
  { 0, t_sse, SSE5A, true, { Zz, Zz, Zz }, 0, 0 },
  { 0, t_sse, SSE5B, true, { Zz, Zz, Zz }, 0, 0 },
  { 0, t_sse, SSE5C, true, { Zz, Zz, Zz }, 0, 0 },
  { 0, t_sse, SSE5D, true, { Zz, Zz, Zz }, 0, 0 },
  { 0, t_sse, SSE5E, true, { Zz, Zz, Zz }, 0, 0 },
  { 0, t_sse, SSE5F, true, { Zz, Zz, Zz }, 0, 0 },
  /* 60 */
  { 0, t_sse, SSE60, true, { Zz, Zz, Zz }, 0, 0 },
  { 0, t_sse, SSE61, true, { Zz, Zz, Zz }, 0, 0 },
  { 0, t_sse, SSE62, true, { Zz, Zz, Zz }, 0, 0 },
  { 0, t_sse, SSE63, true, { Zz, Zz, Zz }, 0, 0 },
  { 0, t_sse, SSE64, true, { Zz, Zz, Zz }, 0, 0 },
  { 0, t_sse, SSE65, true, { Zz, Zz, Zz }, 0, 0 },
  { 0, t_sse, SSE66, true, { Zz, Zz, Zz }, 0, 0 },
  { 0, t_sse, SSE67, true, { Zz, Zz, Zz }, 0, 0 },
  /* 68 */
  { 0, t_sse, SSE68, true, { Zz, Zz, Zz }, 0, 0 },
  { 0, t_sse, SSE69, true, { Zz, Zz, Zz }, 0, 0 },
  { 0, t_sse, SSE6A, true, { Zz, Zz, Zz }, 0, 0 },
  { 0, t_sse, SSE6B, true, { Zz, Zz, Zz }, 0, 0 },
  { 0, t_sse, SSE6C, true, { Zz, Zz, Zz }, 0, 0 },
  { 0, t_sse, SSE6D, true, { Zz, Zz, Zz }, 0, 0 },
  { 0, t_sse, SSE6E, true, { Zz, Zz, Zz }, 0, 0 },
  { 0, t_sse, SSE6F, true, { Zz, Zz, Zz }, 0, 0 },
  /* 70 */
  { 0, t_sse, SSE70, true, { Zz, Zz, Zz }, 0, 0 },
  { 0, t_grp, Grp12, false, { Zz, Zz, Zz }, 0, 0 },
  { 0, t_grp, Grp13, false, { Zz, Zz, Zz }, 0, 0 },
  { 0, t_grp, Grp14, false, { Zz, Zz, Zz }, 0, 0 },
  { 0, t_sse, SSE74, true, { Zz, Zz, Zz }, 0, 0 },
  { 0, t_sse, SSE75, true, { Zz, Zz, Zz }, 0, 0 },
  { 0, t_sse, SSE76, true, { Zz, Zz, Zz }, 0, 0 },
  { "emms", t_done, 0, false, { Zz, Zz, Zz }, 0, sNONE },
  /* 78 */
  { "mmxud", t_ill, 0, 0, { Zz, Zz, Zz }, 0, 0 },
  { "mmxud", t_ill, 0, 0, { Zz, Zz, Zz }, 0, 0 },
  { "mmxud", t_ill, 0, 0, { Zz, Zz, Zz }, 0, 0 },
  { "mmxud", t_ill, 0, 0, { Zz, Zz, Zz }, 0, 0 },
  { "mmxud", t_ill, 0, 0, { Zz, Zz, Zz }, 0, 0 },
  { "mmxud", t_ill, 0, 0, { Zz, Zz, Zz }, 0, 0 },
  { 0, t_sse, SSE7E, 0, { Zz, Zz, Zz }, 0, 0 },
  { 0, t_sse, SSE7F, 0, { Zz, Zz, Zz }, 0, 0 },
  /* 80 */
  { "jo",   t_done, 0, false, { Jz, Zz, Zz }, (IS_JCC | REL_X), s1R | (fCOND << FPOS) },
  { "jno",  t_done, 0, false, { Jz, Zz, Zz }, (IS_JCC | REL_X), s1R | (fCOND << FPOS) },
  { "jb",   t_done, 0, false, { Jz, Zz, Zz }, (IS_JCC | REL_X), s1R | (fCOND << FPOS) },
  { "jnb",  t_done, 0, false, { Jz, Zz, Zz }, (IS_JCC | REL_X), s1R | (fCOND << FPOS) },
  { "jz",   t_done, 0, false, { Jz, Zz, Zz }, (IS_JCC | REL_X), s1R | (fCOND << FPOS) },
  { "jnz",  t_done, 0, false, { Jz, Zz, Zz }, (IS_JCC | REL_X), s1R | (fCOND << FPOS) },
  { "jbe",  t_done, 0, false, { Jz, Zz, Zz }, (IS_JCC | REL_X), s1R | (fCOND << FPOS) },
  { "jnbe", t_done, 0, false, { Jz, Zz, Zz }, (IS_JCC | REL_X), s1R | (fCOND << FPOS) },
  /* 88 */
  { "js",   t_done, 0, false, { Jz, Zz, Zz }, (IS_JCC | REL_X), s1R | (fCOND << FPOS) },
  { "jns",  t_done, 0, false, { Jz, Zz, Zz }, (IS_JCC | REL_X), s1R | (fCOND << FPOS) },
  { "jp",   t_done, 0, false, { Jz, Zz, Zz }, (IS_JCC | REL_X), s1R | (fCOND << FPOS) },
  { "jnp",  t_done, 0, false, { Jz, Zz, Zz }, (IS_JCC | REL_X), s1R | (fCOND << FPOS) },
  { "jl",   t_done, 0, false, { Jz, Zz, Zz }, (IS_JCC | REL_X), s1R | (fCOND << FPOS) },
  { "jnl",  t_done, 0, false, { Jz, Zz, Zz }, (IS_JCC | REL_X), s1R | (fCOND << FPOS) },
  { "jle",  t_done, 0, false, { Jz, Zz, Zz }, (IS_JCC | REL_X), s1R | (fCOND << FPOS) },
  { "jnle", t_done, 0, false, { Jz, Zz, Zz }, (IS_JCC | REL_X), s1R | (fCOND << FPOS) },
  /* 90 */
  { "seto",   t_done, 0, true, { Eb, Zz, Zz }, 0, s1W | (fCOND << FPOS) },
  { "setno",  t_done, 0, true, { Eb, Zz, Zz }, 0, s1W | (fCOND << FPOS) },
  { "setb",   t_done, 0, true, { Eb, Zz, Zz }, 0, s1W | (fCOND << FPOS) },
  { "setnb",  t_done, 0, true, { Eb, Zz, Zz }, 0, s1W | (fCOND << FPOS) },
  { "setz",   t_done, 0, true, { Eb, Zz, Zz }, 0, s1W | (fCOND << FPOS) },
  { "setnz",  t_done, 0, true, { Eb, Zz, Zz }, 0, s1W | (fCOND << FPOS) },
  { "setbe",  t_done, 0, true, { Eb, Zz, Zz }, 0, s1W | (fCOND << FPOS) },
  { "setnbe", t_done, 0, true, { Eb, Zz, Zz }, 0, s1W | (fCOND << FPOS) },
  /* 98 */
  { "sets",   t_done, 0, true, { Eb, Zz, Zz }, 0, s1W | (fCOND << FPOS) },
  { "setns",  t_done, 0, true, { Eb, Zz, Zz }, 0, s1W | (fCOND << FPOS) },
  { "setp",   t_done, 0, true, { Eb, Zz, Zz }, 0, s1W | (fCOND << FPOS) },
  { "setnp",  t_done, 0, true, { Eb, Zz, Zz }, 0, s1W | (fCOND << FPOS) },
  { "setl",   t_done, 0, true, { Eb, Zz, Zz }, 0, s1W | (fCOND << FPOS) },
  { "setnl",  t_done, 0, true, { Eb, Zz, Zz }, 0, s1W | (fCOND << FPOS) },
  { "setle",  t_done, 0, true, { Eb, Zz, Zz }, 0, s1W | (fCOND << FPOS) },
  { "setnle", t_done, 0, true, { Eb, Zz, Zz }, 0, s1W | (fCOND << FPOS) },
  /* A0 */
  { "push",   t_done, 0, false, { STHw, FS, eSP }, 0, s1W2R3RW },
  { "pop",    t_done, 0, false, { FS, STPw, eSP }, 0, s1W2R3RW },
  { "cpuid",  t_done, 0, false, { Zz, Zz, Zz }, 0, sNONE },
  { "bt",     t_done, 0, true, { Ev, Gv, Zz }, 0, s1R2R },
  { "shld",   t_done, 0, true, { Ev, Gv, Ib }, 0, s1RW2R3R },
  { "shld",   t_done, 0, true, { Ev, Gv, CL }, 0, s1RW2R3R },
  { 0, t_ill, 0, 0, { Zz, Zz, Zz }, 0, 0 }, 
  { 0, t_ill, 0, 0, { Zz, Zz, Zz }, 0, 0 },
  /* A8 */
  { "push", t_done, 0, false, { STHw, GS, eSP }, 0, s1W2R3RW },
  { "pop",  t_done, 0, false, { GS, STPw, eSP }, 0, s1W2R3RW },
  { "rsm",  t_done, 0, false, { Zz, Zz, Zz }, 0, sNONE },
  { "bts",  t_done, 0, true, { Ev, Gv, Zz }, 0, s1RW2R },
  { "shrd", t_done, 0, true, { Ev, Gv, Ib }, 0, s1RW2R3R },
  { "shrd", t_done, 0, true, { Ev, Gv, CL }, 0, s1RW2R3R },
  { 0, t_grp, Grp15, 0, { Zz, Zz, Zz }, 0, 0 }, 
  { "imul", t_done, 0, true, { Gv, Ev, Zz }, 0, s1RW2R },
  /* B0 */
  // Assuming this is used with LOCK prefix, the destination gets a write anyway
  // This is not the case without lock prefix, but I ignore that case
  // Also, given that the 3rd operand is a register I ignore that it may be written
  { "cmpxch", t_done, 0, true, { Eb, Gb, AL }, 0, s1RW2R3R | (fCMPXCH << FPOS) },
  { "cmpxch", t_done, 0, true, { Ev, Gv, eAX }, 0, s1RW2R3R | (fCMPXCH << FPOS) },
  { "lss", t_done, 0, true, { SS, Gv, Mp }, 0, s1W2W3R },
  { "btr", t_done, 0, true, { Ev, Gv, Zz }, 0, s1RW2R },
  { "lfs", t_done, 0, true, { FS, Gv, Mp }, 0, s1W2W3R },
  { "lgs", t_done, 0, true, { GS, Gv, Mp }, 0, s1W2W3R },
  { "movzx", t_done, 0, true, { Gv, Eb, Zz }, 0, s1W2R },
  { "movzx", t_done, 0, true, { Gv, Ew, Zz }, 0, s1W2R },
  /* B8 */
  { 0, t_ill, 0, 0, { Zz, Zz, Zz }, 0, 0 },
  { "ud2grp10", t_ill, 0, 0, { Zz, Zz, Zz }, 0, sNONE },
  { 0, t_grp, Grp8, true, { Zz, Zz, Zz }, 0, 0 },
  { "btc", t_done, 0, true, { Ev, Gv, Zz }, 0, s1RW2R },
  { "bsf", t_done, 0, true, { Gv, Ev, Zz }, 0, s1W2R },
  { "bsr", t_done, 0, true, { Gv, Ev, Zz }, 0, s1W2R },
  { "movsx", t_done, 0, true, { Gv, Eb, Zz }, 0, s1W2R },
  { "movsx", t_done, 0, true, { Gv, Ew, Zz }, 0, s1W2R },
  /* C0 */
  { "xadd", t_done, 0, true, { Eb, Gb, Zz }, 0, s1RW2RW },
  { "xadd", t_done, 0, true, { Ev, Gv, Zz }, 0, s1RW2RW },
  { 0, t_sse, SSEC2, true, { Zz, Zz, Zz }, 0, 0 },
  { "movnti" , t_done, 0, 0, { Ed, Gd, Zz }, 0, s1W2R | (fNT << FPOS) },
  { 0, t_sse, SSEC4, true, { Zz, Zz, Zz }, 0, 0 },
  { 0, t_sse, SSEC5, true, { Zz, Zz, Zz }, 0, 0 },
  { 0, t_sse, SSEC6, true, { Zz, Zz, Zz }, 0, 0 },
  { 0, t_grp, Grp9,  true, { Zz, Zz, Zz }, 0, 0 },
  /* C8 */
  { "bswap", t_done, 0, false, { EAX, Zz, Zz }, 0, s1RW }, 
  { "bswap", t_done, 0, false, { ECX, Zz, Zz }, 0, s1RW },
  { "bswap", t_done, 0, false, { EDX, Zz, Zz }, 0, s1RW }, 
  { "bswap", t_done, 0, false, { EBX, Zz, Zz }, 0, s1RW }, 
  { "bswap", t_done, 0, false, { ESP, Zz, Zz }, 0, s1RW },
  { "bswap", t_done, 0, false, { EBP, Zz, Zz }, 0, s1RW }, 
  { "bswap", t_done, 0, false, { ESI, Zz, Zz }, 0, s1RW }, 
  { "bswap", t_done, 0, false, { EDI, Zz, Zz }, 0, s1RW }, 
  /* D0 */
  { 0, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
  { 0, t_sse, SSED1, true, { Zz, Zz, Zz }, 0, 0 },
  { 0, t_sse, SSED2, true, { Zz, Zz, Zz }, 0, 0 },
  { 0, t_sse, SSED3, true, { Zz, Zz, Zz }, 0, 0 },
  { 0, t_sse, SSED4, true, { Zz, Zz, Zz }, 0, 0 },
  { 0, t_sse, SSED5, true, { Zz, Zz, Zz }, 0, 0 },
  { 0, t_sse, SSED6, true, { Zz, Zz, Zz }, 0, 0 },
  { 0, t_sse, SSED7, true, { Zz, Zz, Zz }, 0, 0 },
  /* D8 */
  { 0, t_sse, SSED8, true, { Zz, Zz, Zz }, 0, 0 },
  { 0, t_sse, SSED9, true, { Zz, Zz, Zz }, 0, 0 },
  { 0, t_sse, SSEDA, true, { Zz, Zz, Zz }, 0, 0 },
  { 0, t_sse, SSEDB, true, { Zz, Zz, Zz }, 0, 0 },
  { 0, t_sse, SSEDC, true, { Zz, Zz, Zz }, 0, 0 },
  { 0, t_sse, SSEDD, true, { Zz, Zz, Zz }, 0, 0 },
  { 0, t_sse, SSEDE, true, { Zz, Zz, Zz }, 0, 0 },
  { 0, t_sse, SSEDF, true, { Zz, Zz, Zz }, 0, 0 },
  /* E0 */
  { 0, t_sse, SSEE0, true, { Zz, Zz, Zz }, 0, 0 },
  { 0, t_sse, SSEE1, true, { Zz, Zz, Zz }, 0, 0 },
  { 0, t_sse, SSEE2, true, { Zz, Zz, Zz }, 0, 0 },
  { 0, t_sse, SSEE3, true, { Zz, Zz, Zz }, 0, 0 },
  { 0, t_sse, SSEE4, true, { Zz, Zz, Zz }, 0, 0 },
  { 0, t_sse, SSEE5, true, { Zz, Zz, Zz }, 0, 0 },
  { 0, t_sse, SSEE6, true, { Zz, Zz, Zz }, 0, 0 },
  { 0, t_sse, SSEE7, true, { Zz, Zz, Zz }, 0, 0 },
  /* E8 */
  { 0, t_sse, SSEE8, true, { Zz, Zz, Zz }, 0, 0 },
  { 0, t_sse, SSEE9, true, { Zz, Zz, Zz }, 0, 0 },
  { 0, t_sse, SSEEA, true, { Zz, Zz, Zz }, 0, 0 },
  { 0, t_sse, SSEEB, true, { Zz, Zz, Zz }, 0, 0 },
  { 0, t_sse, SSEEC, true, { Zz, Zz, Zz }, 0, 0 },
  { 0, t_sse, SSEED, true, { Zz, Zz, Zz }, 0, 0 },
  { 0, t_sse, SSEEE, true, { Zz, Zz, Zz }, 0, 0 },
  { 0, t_sse, SSEEF, true, { Zz, Zz, Zz }, 0, 0 },
  /* F0 */
  { 0, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
  { 0, t_sse, SSEF1, true, { Zz, Zz, Zz }, 0, 0 },
  { 0, t_sse, SSEF2, true, { Zz, Zz, Zz }, 0, 0 },
  { 0, t_sse, SSEF3, true, { Zz, Zz, Zz }, 0, 0 },
  { 0, t_sse, SSEF4, true, { Zz, Zz, Zz }, 0, 0 },
  { 0, t_sse, SSEF5, true, { Zz, Zz, Zz }, 0, 0 },
  { 0, t_sse, SSEF6, true, { Zz, Zz, Zz }, 0, 0 },
  { 0, t_sse, SSEF7, true, { Zz, Zz, Zz }, 0, 0 },
  /* F8 */
  { 0, t_sse, SSEF8, true, { Zz, Zz, Zz }, 0, 0 },
  { 0, t_sse, SSEF9, true, { Zz, Zz, Zz }, 0, 0 },
  { 0, t_sse, SSEFA, true, { Zz, Zz, Zz }, 0, 0 },
  { 0, t_sse, SSEFB, true, { Zz, Zz, Zz }, 0, 0 },
  { 0, t_sse, SSEFC, true, { Zz, Zz, Zz }, 0, 0 },
  { 0, t_sse, SSEFD, true, { Zz, Zz, Zz }, 0, 0 },
  { 0, t_sse, SSEFE, true, { Zz, Zz, Zz }, 0, 0 },
  { 0, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 }
};

static ia32_entry groupMap[][8] = {
  { /* group 1a */
    { "add", t_done, 0, true, { Eb, Ib, Zz }, 0, s1RW2R },
    { "or",  t_done, 0, true, { Eb, Ib, Zz }, 0, s1RW2R },
    { "adc", t_done, 0, true, { Eb, Ib, Zz }, 0, s1RW2R },
    { "sbb", t_done, 0, true, { Eb, Ib, Zz }, 0, s1RW2R },
    { "and", t_done, 0, true, { Eb, Ib, Zz }, 0, s1RW2R },
    { "sub", t_done, 0, true, { Eb, Ib, Zz }, 0, s1RW2R },
    { "xor", t_done, 0, true, { Eb, Ib, Zz }, 0, s1RW2R },
    { "cmp", t_done, 0, true, { Eb, Ib, Zz }, 0, s1R2R },
  },
  { /* group 1b */
    { "add", t_done, 0, true, { Ev, Iz, Zz }, 0, s1RW2R },
    { "or",  t_done, 0, true, { Ev, Iz, Zz }, 0, s1RW2R },
    { "adc", t_done, 0, true, { Ev, Iz, Zz }, 0, s1RW2R },
    { "sbb", t_done, 0, true, { Ev, Iz, Zz }, 0, s1RW2R },
    { "and", t_done, 0, true, { Ev, Iz, Zz }, 0, s1RW2R },
    { "sub", t_done, 0, true, { Ev, Iz, Zz }, 0, s1RW2R },
    { "xor", t_done, 0, true, { Ev, Iz, Zz }, 0, s1RW2R },
    { "cmp", t_done, 0, true, { Ev, Iz, Zz }, 0, s1R2R },
  },
  { /* group 1c */
    { "add", t_done, 0, true, { Eb, Ib, Zz }, 0, s1RW2R },
    { "or",  t_done, 0, true, { Eb, Ib, Zz }, 0, s1RW2R },
    { "adc", t_done, 0, true, { Eb, Ib, Zz }, 0, s1RW2R },
    { "sbb", t_done, 0, true, { Eb, Ib, Zz }, 0, s1RW2R },
    { "and", t_done, 0, true, { Eb, Ib, Zz }, 0, s1RW2R },
    { "sub", t_done, 0, true, { Eb, Ib, Zz }, 0, s1RW2R },
    { "xor", t_done, 0, true, { Eb, Ib, Zz }, 0, s1RW2R },
    { "cmp", t_done, 0, true, { Eb, Ib, Zz }, 0, s1R2R },
  },
  { /* group 1d */
    { "add", t_done, 0, true, { Ev, Ib, Zz }, 0, s1RW2R },
    { "or",  t_done, 0, true, { Ev, Ib, Zz }, 0, s1RW2R },
    { "adc", t_done, 0, true, { Ev, Ib, Zz }, 0, s1RW2R },
    { "sbb", t_done, 0, true, { Ev, Ib, Zz }, 0, s1RW2R },
    { "and", t_done, 0, true, { Ev, Ib, Zz }, 0, s1RW2R },
    { "sub", t_done, 0, true, { Ev, Ib, Zz }, 0, s1RW2R },
    { "xor", t_done, 0, true, { Ev, Ib, Zz }, 0, s1RW2R },
    { "cmp", t_done, 0, true, { Ev, Ib, Zz }, 0, s1R2R },
  },


 {  /* group 2 - only opcode is defined here, 
       operands are defined in the one or two byte maps above */
  { "rol", t_done, 0, true, { Zz, Zz, Zz }, 0, 0 },
  { "ror", t_done, 0, true, { Zz, Zz, Zz }, 0, 0 },
  { "rcl", t_done, 0, true, { Zz, Zz, Zz }, 0, 0 },
  { "rcr", t_done, 0, true, { Zz, Zz, Zz }, 0, 0 },
  { "shl/sal", t_done, 0, true, { Zz, Zz, Zz }, 0, 0 },
  { "shr", t_done, 0, true, { Zz, Zz, Zz }, 0, 0 },
  { 0, t_ill, 0, true, { Zz, Zz, Zz }, 0, 0 },
  { "sar", t_done, 0, true, { Zz, Zz, Zz }, 0, 0 }
 },

 { /* group 3a - operands are defined here */
  { "test", t_done, 0, true, { Eb, Ib, Zz }, 0, s1R2R },
  { 0, t_ill, 0, true, { Zz, Zz, Zz }, 0, 0 },
  { "not",  t_done, 0, true, { Eb, Zz, Zz }, 0, s1RW },
  { "neg",  t_done, 0, true, { Eb, Zz, Zz }, 0, s1RW },
  { "mul",  t_done, 0, true, { AX, AL, Eb }, 0, s1W2RW3R },
  { "imul", t_done, 0, true, { AX, AL, Eb }, 0, s1W2R3R },
  { "div",  t_done, 0, true, { AX, AL, Eb }, 0, s1RW2R3R },
  { "idiv", t_done, 0, true, { AX, AL, Eb }, 0, s1W2R3R }
 },

 { /* group 3b - operands are defined here */
  { "test", t_done, 0, true, { Ev, Iz, Zz }, 0, s1R2R },
  { 0, t_ill, 0, true, { Zz, Zz, Zz }, 0, 0 },
  { "not",  t_done, 0, true, { Ev, Zz, Zz }, 0, s1RW },
  { "neg",  t_done, 0, true, { Ev, Zz, Zz }, 0, s1RW },
  { "mul",  t_done, 0, true, { eDX, eAX, Ev }, 0, s1W2RW3R },
  { "imul", t_done, 0, true, { eDX, eAX, Ev }, 0, s1W2RW3R },
  { "div",  t_done, 0, true, { eDX, eAX, Ev }, 0, s1RW2RW3R },
  { "idiv", t_done, 0, true, { eDX, eAX, Ev }, 0, s1RW2RW3R }
 },

 { /* group 4 - operands are defined here */
  { "inc", t_done, 0, true, { Eb, Zz, Zz }, 0, s1RW },
  { "dec", t_done, 0, true, { Eb, Zz, Zz }, 0, s1RW },
  { 0, t_ill, 0, true, { Zz, Zz, Zz }, 0, 0 },
  { 0, t_ill, 0, true, { Zz, Zz, Zz }, 0, 0 },
  { 0, t_ill, 0, true, { Zz, Zz, Zz }, 0, 0 },
  { 0, t_ill, 0, true, { Zz, Zz, Zz }, 0, 0 },
  { 0, t_ill, 0, true, { Zz, Zz, Zz }, 0, 0 },
  { 0, t_ill, 0, true, { Zz, Zz, Zz }, 0, 0 },
 },

 { /* group 5 - operands are defined here */
  { "inc",  t_done, 0, true, { Ev, Zz, Zz }, 0, s1RW },
  { "dec",  t_done, 0, true, { Ev, Zz, Zz }, 0, s1RW },
  { "call", t_done, 0, true, { Ev, Zz, Zz }, (IS_CALL | INDIR), s1R | (fINDIRCALL << FPOS) },
  { "call", t_done, 0, true, { Ep, Zz, Zz }, (IS_CALL | INDIR), s1R | (fINDIRCALL << FPOS) },
  { "jmp",  t_done, 0, true, { Ev, Zz, Zz }, (IS_JUMP | INDIR), s1R | (fINDIRJUMP << FPOS) },
  { "jmp",  t_done, 0, true, { Ep, Zz, Zz }, (IS_JUMP | INDIR), s1R | (fINDIRJUMP << FPOS) },
  { "push", t_done, 0, true, { Ev, Zz, Zz }, 0, 0 },
  { 0, t_ill, 0, true, { Zz, Zz, Zz }, 0, 0 },
 },

 { /* group 6 - operands are defined here */
   // these need to be fixed for kernel mode accesses
  { "sldt", t_done, 0, true, { Ew, Zz, Zz }, 0, s1W },
  { "str",  t_done, 0, true, { Ew, Zz, Zz }, 0, s1W },
  { "lldt", t_done, 0, true, { Ew, Zz, Zz }, 0, s1R },
  { "ltr",  t_done, 0, true, { Ew, Zz, Zz }, 0, s1R },
  { "verr", t_done, 0, true, { Ew, Zz, Zz }, 0, s1R },
  { "verw", t_done, 0, true, { Ew, Zz, Zz }, 0, s1R },
  { 0, t_ill, 0, true, { Zz, Zz, Zz }, 0, 0 },
  { 0, t_ill, 0, true, { Zz, Zz, Zz }, 0, 0 },
 },

 { /* group 7 - operands are defined here */
   // idem
  { "sgdt", t_done, 0, true, { Ms, Zz, Zz }, 0, s1W },
  { "sidt", t_done, 0, true, { Ms, Zz, Zz }, 0, s1W },
  { "lgdt", t_done, 0, true, { Ms, Zz, Zz }, 0, s1R },
  { "lidt", t_done, 0, true, { Ms, Zz, Zz }, 0, s1R },
  { "smsw", t_done, 0, true, { Ew, Zz, Zz }, 0, s1W },
  { 0, t_ill, 0, true, { Zz, Zz, Zz }, 0, 0 },
  { "lmsw", t_done, 0, true, { Ew, Zz, Zz }, 0, s1R },
  { "invlpg", t_done, 0, true, { Zz, Zz, Zz }, 0, sNONE }, // 64-bit: swapgs also uses this encoding (w/ mod=11)
 },

 { /* group 8 - only opcode is defined here, 
     operands are defined in the one or two byte maps below */
  { 0, t_ill, 0, true, { Zz, Zz, Zz }, 0, 0 },
  { 0, t_ill, 0, true, { Zz, Zz, Zz }, 0, 0 },
  { 0, t_ill, 0, true, { Zz, Zz, Zz }, 0, 0 },
  { 0, t_ill, 0, true, { Zz, Zz, Zz }, 0, 0 },
  { "bt",  t_done, 0, true, { Ev, Ib, Zz }, 0, s1R2R },
  { "bts", t_done, 0, true, { Ev, Ib, Zz }, 0, s1RW2R },
  { "btr", t_done, 0, true, { Ev, Ib, Zz }, 0, s1RW2R },
  { "btc", t_done, 0, true, { Ev, Ib, Zz }, 0, s1RW2R },
 },

 { /* group 9 - operands are defined here */
  { 0, t_ill, 0, true, { Zz, Zz, Zz }, 0, 0 },
  // see comments for cmpxch
  { "cmpxch8b", t_done, 0, true, { EDXEAX, Mq, ECXEBX }, 0, s1RW2RW3R | (fCMPXCH8 << FPOS) },
  { 0, t_ill, 0, true, { Zz, Zz, Zz }, 0, 0 },
  { 0, t_ill, 0, true, { Zz, Zz, Zz }, 0, 0 },
  { 0, t_ill, 0, true, { Zz, Zz, Zz }, 0, 0 },
  { 0, t_ill, 0, true, { Zz, Zz, Zz }, 0, 0 },
  { 0, t_ill, 0, true, { Zz, Zz, Zz }, 0, 0 },
  { 0, t_ill, 0, true, { Zz, Zz, Zz }, 0, 0 }
 },

 /* group 10 is all illegal */

 { /* group 11, opcodes defined in one byte map */
   { "mov", t_done, 0, true, { Zz, Zz, Zz }, 0, sNONE },
   { 0, t_ill, 0, true, { Zz, Zz, Zz }, 0, 0 },
   { 0, t_ill, 0, true, { Zz, Zz, Zz }, 0, 0 },
   { 0, t_ill, 0, true, { Zz, Zz, Zz }, 0, 0 },
   { 0, t_ill, 0, true, { Zz, Zz, Zz }, 0, 0 },
   { 0, t_ill, 0, true, { Zz, Zz, Zz }, 0, 0 },
   { 0, t_ill, 0, true, { Zz, Zz, Zz }, 0, 0 },
   { 0, t_ill, 0, true, { Zz, Zz, Zz }, 0, 0 },
 }

};


// Groups 12-16 are split by mod={mem,11}. Some spill over into SSE groups!
// Notation: G12SSE010B = group 12, SSE, reg=010; B means mod=11
// Use A if Intel decides to put SSE instructions for mod=mem
static ia32_entry groupMap2[][2][8] = {
  { /* group 12 */
    {
      { 0, t_ill, 0, true, { Zz, Zz, Zz }, 0, 0 },
      { 0, t_ill, 0, true, { Zz, Zz, Zz }, 0, 0 },
      { 0, t_ill, 0, true, { Zz, Zz, Zz }, 0, 0 },
      { 0, t_ill, 0, true, { Zz, Zz, Zz }, 0, 0 },
      { 0, t_ill, 0, true, { Zz, Zz, Zz }, 0, 0 },
      { 0, t_ill, 0, true, { Zz, Zz, Zz }, 0, 0 },
      { 0, t_ill, 0, true, { Zz, Zz, Zz }, 0, 0 },
      { 0, t_ill, 0, true, { Zz, Zz, Zz }, 0, 0 },
    },
    {
      { 0, t_ill, 0, true, { Zz, Zz, Zz }, 0, 0 },
      { 0, t_ill, 0, true, { Zz, Zz, Zz }, 0, 0 },
      { 0, t_grpsse, G12SSE010B, true, { Zz, Zz, Zz }, 0, 0 },
      { 0, t_ill, 0, true, { Zz, Zz, Zz }, 0, 0 },
      { 0, t_grpsse, G12SSE100B, true, { Zz, Zz, Zz }, 0, 0 },
      { 0, t_ill, 0, true, { Zz, Zz, Zz }, 0, 0 },
      { 0, t_grpsse, G12SSE110B, true, { Zz, Zz, Zz }, 0, 0 },
      { 0, t_ill, 0, true, { Zz, Zz, Zz }, 0, 0 },
    }
  },
  { /* group 13 */
    {
      { 0, t_ill, 0, true, { Zz, Zz, Zz }, 0, 0 },
      { 0, t_ill, 0, true, { Zz, Zz, Zz }, 0, 0 },
      { 0, t_ill, 0, true, { Zz, Zz, Zz }, 0, 0 },
      { 0, t_ill, 0, true, { Zz, Zz, Zz }, 0, 0 },
      { 0, t_ill, 0, true, { Zz, Zz, Zz }, 0, 0 },
      { 0, t_ill, 0, true, { Zz, Zz, Zz }, 0, 0 },
      { 0, t_ill, 0, true, { Zz, Zz, Zz }, 0, 0 },
      { 0, t_ill, 0, true, { Zz, Zz, Zz }, 0, 0 },
    },
    {
      { 0, t_ill, 0, true, { Zz, Zz, Zz }, 0, 0 },
      { 0, t_ill, 0, true, { Zz, Zz, Zz }, 0, 0 },
      { 0, t_grpsse, G13SSE010B, true, { Zz, Zz, Zz }, 0, 0 },
      { 0, t_ill, 0, true, { Zz, Zz, Zz }, 0, 0 },
      { 0, t_grpsse, G13SSE100B, true, { Zz, Zz, Zz }, 0, 0 },
      { 0, t_ill, 0, true, { Zz, Zz, Zz }, 0, 0 },
      { 0, t_grpsse, G13SSE110B, true, { Zz, Zz, Zz }, 0, 0 },
      { 0, t_ill, 0, true, { Zz, Zz, Zz }, 0, 0 },
    }
  },
  { /* group 14 */
    {
      { 0, t_ill, 0, true, { Zz, Zz, Zz }, 0, 0 },
      { 0, t_ill, 0, true, { Zz, Zz, Zz }, 0, 0 },
      { 0, t_ill, 0, true, { Zz, Zz, Zz }, 0, 0 },
      { 0, t_ill, 0, true, { Zz, Zz, Zz }, 0, 0 },
      { 0, t_ill, 0, true, { Zz, Zz, Zz }, 0, 0 },
      { 0, t_ill, 0, true, { Zz, Zz, Zz }, 0, 0 },
      { 0, t_ill, 0, true, { Zz, Zz, Zz }, 0, 0 },
      { 0, t_ill, 0, true, { Zz, Zz, Zz }, 0, 0 },
    },
    {
      { 0, t_ill, 0, true, { Zz, Zz, Zz }, 0, 0 },
      { 0, t_ill, 0, true, { Zz, Zz, Zz }, 0, 0 },
      { 0, t_grpsse, G14SSE010B, true, { Zz, Zz, Zz }, 0, 0 },
      { 0, t_grpsse, G14SSE011B, true, { Zz, Zz, Zz }, 0, 0 },
      { 0, t_ill, 0, true, { Zz, Zz, Zz }, 0, 0 },
      { 0, t_ill, 0, true, { Zz, Zz, Zz }, 0, 0 },
      { 0, t_grpsse, G14SSE110B, true, { Zz, Zz, Zz }, 0, 0 },
      { 0, t_grpsse, G14SSE111B, true, { Zz, Zz, Zz }, 0, 0 },
    }
  },
  { /* group 15 */
    {
      { "fxsave",  t_done, 0, true, { M512, Zz, Zz }, 0, s1W | (fFXSAVE << FPOS) },
      { "fxrstor", t_done, 0, true, { M512, Zz, Zz }, 0, s1R | (fFXRSTOR << FPOS) },
      { "ldmxcsr", t_done, 0, true, { Md, Zz, Zz }, 0, s1R },
      { "stmxcsr", t_done, 0, true, { Md, Zz, Zz }, 0, s1W },
      { 0, t_ill, 0, true, { Zz, Zz, Zz }, 0, 0 },
      { 0, t_ill, 0, true, { Zz, Zz, Zz }, 0, 0 },
      { 0, t_ill, 0, true, { Zz, Zz, Zz }, 0, 0 },
      { "clflush", t_done, 0, true, { Mb, Zz, Zz }, 0, s1W | (fCLFLUSH << FPOS) },
    },
    {
      { 0, t_ill, 0, true, { Zz, Zz, Zz }, 0, 0 },
      { 0, t_ill, 0, true, { Zz, Zz, Zz }, 0, 0 },
      { 0, t_ill, 0, true, { Zz, Zz, Zz }, 0, 0 },
      { 0, t_ill, 0, true, { Zz, Zz, Zz }, 0, 0 },
      { 0, t_ill, 0, true, { Zz, Zz, Zz }, 0, 0 },
      { "lfence", t_done, 0, true, { Zz, Zz, Zz }, 0, sNONE },
      { "mfence", t_done, 0, true, { Zz, Zz, Zz }, 0, sNONE },
      { "sfence", t_done, 0, true, { Zz, Zz, Zz }, 0, sNONE },
    }
  },
  { /* group 16 */
    {
      { "prefetchNTA", t_done, 0, true, { Mb, Zz, Zz }, 0, s1R | (fPREFETCHNT << FPOS) },
      { "prefetchT0",  t_done, 0, true, { Mb, Zz, Zz }, 0, s1R | (fPREFETCHT0 << FPOS) },
      { "prefetchT1",  t_done, 0, true, { Mb, Zz, Zz }, 0, s1R | (fPREFETCHT1 << FPOS) },
      { "prefetchT2",  t_done, 0, true, { Mb, Zz, Zz }, 0, s1R | (fPREFETCHT1 << FPOS) },
      { 0, t_ill, 0, true, { Zz, Zz, Zz }, 0, 0 },
      { 0, t_ill, 0, true, { Zz, Zz, Zz }, 0, 0 },
      { 0, t_ill, 0, true, { Zz, Zz, Zz }, 0, 0 },
      { 0, t_ill, 0, true, { Zz, Zz, Zz }, 0, 0 },
    },
    {
      { 0, t_ill, 0, true, { Zz, Zz, Zz }, 0, 0 },
      { 0, t_ill, 0, true, { Zz, Zz, Zz }, 0, 0 },
      { 0, t_ill, 0, true, { Zz, Zz, Zz }, 0, 0 },
      { 0, t_ill, 0, true, { Zz, Zz, Zz }, 0, 0 },
      { 0, t_ill, 0, true, { Zz, Zz, Zz }, 0, 0 },
      { 0, t_ill, 0, true, { Zz, Zz, Zz }, 0, 0 },
      { 0, t_ill, 0, true, { Zz, Zz, Zz }, 0, 0 },
      { 0, t_ill, 0, true, { Zz, Zz, Zz }, 0, 0 },
    }
  },
  { /* AMD prefetch group */
    {
      { "prefetch",   t_done, 0, true, { Mb, Zz, Zz }, 0, s1R | (fPREFETCHAMDE << FPOS) },
      { "prefetchw",  t_done, 0, true, { Mb, Zz, Zz }, 0, s1R | (fPREFETCHAMDW << FPOS) },
      { 0, t_ill, 0, true, { Zz, Zz, Zz }, 0, 0 }, // this is reserved, not illegal, ugh...
      { 0, t_ill, 0, true, { Zz, Zz, Zz }, 0, 0 }, // this is reserved, not illegal, ugh...
      { 0, t_ill, 0, true, { Zz, Zz, Zz }, 0, 0 }, // this is reserved, not illegal, ugh...
      { 0, t_ill, 0, true, { Zz, Zz, Zz }, 0, 0 }, // this is reserved, not illegal, ugh...
      { 0, t_ill, 0, true, { Zz, Zz, Zz }, 0, 0 }, // this is reserved, not illegal, ugh...
      { 0, t_ill, 0, true, { Zz, Zz, Zz }, 0, 0 }, // this is reserved, not illegal, ugh...
    },
    {
      { 0, t_ill, 0, true, { Zz, Zz, Zz }, 0, 0 },
      { 0, t_ill, 0, true, { Zz, Zz, Zz }, 0, 0 },
      { 0, t_ill, 0, true, { Zz, Zz, Zz }, 0, 0 },
      { 0, t_ill, 0, true, { Zz, Zz, Zz }, 0, 0 },
      { 0, t_ill, 0, true, { Zz, Zz, Zz }, 0, 0 },
      { 0, t_ill, 0, true, { Zz, Zz, Zz }, 0, 0 },
      { 0, t_ill, 0, true, { Zz, Zz, Zz }, 0, 0 },
      { 0, t_ill, 0, true, { Zz, Zz, Zz }, 0, 0 },
    }
  }
};

/* rows are not, F3, 66, F2 prefixed in this order (see book) */
static ia32_entry sseMap[][4] = {
  { /* SSE10 */
    { "movups", t_done, 0, true, { Vps, Wps, Zz }, 0, s1W2R },
    { "movss",  t_done, 0, true, { Vss, Wss, Zz }, 0, s1W2R },
    { "movupd", t_done, 0, true, { Vpd, Wpd, Zz }, 0, s1W2R },
    { "movsd",  t_done, 0, true, { Vsd, Wsd, Zz }, 0, s1W2R },
  },
  { /* SSE11 */
    { "movups", t_done, 0, true, { Wps, Vps, Zz }, 0, s1W2R },
    { "movss",  t_done, 0, true, { Wss, Vss, Zz }, 0, s1W2R },
    { "movupd", t_done, 0, true, { Wpd, Vpd, Zz }, 0, s1W2R },
    { "movsd",  t_done, 0, true, { Vsd, Wsd, Zz }, 0, s1W2R }, // FIXME: bug in book?????
  },
  { /* SSE12 */
    { "movlps/movhlps", t_done, 0, true, { Wq, Vq, Zz }, 0, s1W2R }, // FIXME: wierd 1st op
    { 0, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
    { "movlpd", t_done, 0, true, { Vq, Ws, Zz }, 0, s1W2R },
    { 0, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
  },
  { /* SSE13 */
    { "movlps", t_done, 0, true, { Vq, Wq, Zz }, 0, s1W2R },
    { 0, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
    { "movlpd", t_done, 0, true, { Vq, Wq, Zz }, 0, s1W2R },
    { 0, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
  },
  { /* SSE14 */
    { "unpcklps", t_done, 0, true, { Vps, Wq, Zz }, 0, s1RW2R },
    { 0, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
    { "unpcklpd", t_done, 0, true, { Vpd, Wq, Zz }, 0, s1RW2R },
    { 0, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
  },
  { /* SSE15 */
    { "unpckhps", t_done, 0, true, { Vps, Wq, Zz }, 0, s1RW2R },
    { 0, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
    { "unpckhpd", t_done, 0, true, { Vpd, Wq, Zz }, 0, s1RW2R },
    { 0, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
  },
  { /* SSE16 */
    { "movhps/movlhps", t_done, 0, true, { Vq, Wq, Zz }, 0, s1W2R }, // FIXME: wierd 2nd op
    { 0, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
    { "movhpd", t_done, 0, true, { Vq, Wq, Zz }, 0, s1W2R },
    { 0, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
  },
  { /* SSE17 */
    { "movhps", t_done, 0, true, { Wq, Vq, Zz }, 0, s1W2R },
    { 0, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
    { "movhpd", t_done, 0, true, { Wq, Vq, Zz }, 0, s1W2R },
    { 0, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
  },
  { /* SSE28 */
    { "movaps", t_done, 0, true, { Vps, Wps, Zz }, 0, s1W2R },
    { 0, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
    { "movapd", t_done, 0, true, { Vpd, Wpd, Zz }, 0, s1W2R },
    { 0, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
  },
  { /* SSE29 */
    { "movaps", t_done, 0, true, { Wps, Vps, Zz }, 0, s1W2R },
    { 0, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
    { "movapd", t_done, 0, true, { Wpd, Vpd, Zz }, 0, s1W2R },
    { 0, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
  },
  { /* SSE2A */
    { "cvtpi2ps", t_done, 0, true, { Vps, Qq, Zz }, 0, s1W2R },
    { "cvtsi2ss", t_done, 0, true, { Vss, Ed, Zz }, 0, s1W2R },
    { "cvtpi2pd", t_done, 0, true, { Vpd, Qdq, Zz }, 0, s1W2R },
    { "cvtsi2sd", t_done, 0, true, { Vsd, Ed, Zz }, 0, s1W2R },
  },
  { /* SSE2B */
    { "movntps", t_done, 0, true, { Wps, Vps, Zz }, 0, s1W2R | (fNT << FPOS) },
    { 0, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
    { "movntpd", t_done, 0, true, { Wpd, Vpd, Zz }, 0, s1W2R | (fNT << FPOS) }, // bug in book
    { 0, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
  },
  { /* SSE2C */
    { "cvttps2pi", t_done, 0, true, { Qq, Wps, Zz }, 0, s1W2R },
    { "cvttss2si", t_done, 0, true, { Gd, Wss, Zz }, 0, s1W2R },
    { "cvttpd2pi", t_done, 0, true, { Qdq, Wpd, Zz }, 0, s1W2R },
    { "cvttsd2si", t_done, 0, true, { Gd, Wsd, Zz }, 0, s1W2R },
  },
  { /* SSE2D */
    { "cvtps2pi", t_done, 0, true, { Qq, Wps, Zz }, 0, s1W2R },
    { "cvtss2si", t_done, 0, true, { Gd, Wss, Zz }, 0, s1W2R },
    { "cvtpd2pi", t_done, 0, true, { Qdq, Wpd, Zz }, 0, s1W2R },
    { "cvtsd2si", t_done, 0, true, { Gd, Wsd, Zz }, 0, s1W2R },
  },
  { /* SSE2E */
    { "ucomiss", t_done, 0, true, { Vss, Wss, Zz }, 0, s1R2R },
    { 0, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
    { "ucomisd", t_done, 0, true, { Vsd, Wsd, Zz }, 0, s1R2R },
    { 0, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
  },
  { /* SSE2F */
    { "comiss", t_done, 0, true, { Vps, Wps, Zz }, 0, s1R2R },
    { 0, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
    { "comisd", t_done, 0, true, { Vsd, Wsd, Zz }, 0, s1R2R },
    { 0, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
  },
  { /* SSE50 */
    { "movmskps", t_done, 0, true, { Ed, Vps, Zz }, 0, s1W2R },
    { 0, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
    { "movmskpd", t_done, 0, true, { Ed, Vpd, Zz }, 0, s1W2R },
    { 0, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
  },
  { /* SSE51 */
    { "sqrtps", t_done, 0, true, { Vps, Wps, Zz }, 0, s1W2R },
    { "sqrtss", t_done, 0, true, { Vss, Wss, Zz }, 0, s1W2R },
    { "sqrtpd", t_done, 0, true, { Vpd, Wpd, Zz }, 0, s1W2R },
    { "sqrtsd", t_done, 0, true, { Vsd, Wsd, Zz }, 0, s1W2R },
  },
  { /* SSE52 */
    { "rsqrtps", t_done, 0, true, { Vps, Wps, Zz }, 0, s1W2R },
    { 0, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
    { "rsqrtss", t_done, 0, true, { Vss, Wss, Zz }, 0, s1W2R },
    { 0, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
  },
  { /* SSE53 */
    { "rcpps", t_done, 0, true, { Vps, Wps, Zz }, 0, s1W2R },
    { 0, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
    { "rcpss", t_done, 0, true, { Vss, Wss, Zz }, 0, s1W2R },
    { 0, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
  },
  { /* SSE54 */
    { "andps", t_done, 0, true, { Vps, Wps, Zz }, 0, s1RW2R },
    { 0, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
    { "andpd", t_done, 0, true, { Vpd, Wpd, Zz }, 0, s1RW2R },
    { 0, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
  },
  { /* SSE55 */
    { "andnps", t_done, 0, true, { Vps, Wps, Zz }, 0, s1RW2R },
    { 0, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
    { "andnpd", t_done, 0, true, { Vpd, Wpd, Zz }, 0, s1RW2R },
    { 0, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
  },
  { /* SSE56 */
    { "orps", t_done, 0, true, { Vps, Wps, Zz }, 0, s1RW2R },
    { 0, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
    { "orpd", t_done, 0, true, { Vpd, Wpd, Zz }, 0, s1RW2R },
    { 0, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
  },
  { /* SSE57 */
    { "xorps", t_done, 0, true, { Vps, Wps, Zz }, 0, s1RW2R },
    { 0, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
    { "xorpd", t_done, 0, true, { Vpd, Wpd, Zz }, 0, s1RW2R },
    { 0, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
  },
  { /* SSE58 */
    { "addps", t_done, 0, true, { Vps, Wps, Zz }, 0, s1RW2R },
    { "addss", t_done, 0, true, { Vss, Wss, Zz }, 0, s1RW2R },
    { "addpd", t_done, 0, true, { Vpd, Wpd, Zz }, 0, s1RW2R },
    { "addsd", t_done, 0, true, { Vsd, Wsd, Zz }, 0, s1RW2R },
  },
  { /* SSE59 */
    { "mulps", t_done, 0, true, { Vps, Wps, Zz }, 0, s1RW2R },
    { "mulss", t_done, 0, true, { Vss, Wss, Zz }, 0, s1RW2R },
    { "mulpd", t_done, 0, true, { Vpd, Wpd, Zz }, 0, s1RW2R },
    { "mulsd", t_done, 0, true, { Vsd, Wsd, Zz }, 0, s1RW2R },
  },
  { /* SSE5A */
    { "cvtps2pd", t_done, 0, true, { Vpd, Wps, Zz }, 0, s1W2R },
    { "cvtss2sd", t_done, 0, true, { Vss, Wss, Zz }, 0, s1W2R },
    { "cvtpd2ps", t_done, 0, true, { Vps, Wpd, Zz }, 0, s1W2R }, // FIXME: book bug ???
    { "cvtsd2ss", t_done, 0, true, { Vsd, Wsd, Zz }, 0, s1W2R },
  },
  { /* SSE5B */
    { "cvtdq2ps", t_done, 0, true, { Vps, Wdq, Zz }, 0, s1W2R },
    { "cvttps2dq", t_done, 0, true, { Vdq, Wps, Zz }, 0, s1W2R }, // book has this/next swapped!!! 
    { "cvtps2dq", t_done, 0, true, { Vdq, Wps, Zz }, 0, s1W2R },  // FIXME: book bug ???
    { 0, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
  },
  { /* SSE5C */
    { "subps", t_done, 0, true, { Vps, Wps, Zz }, 0, s1RW2R },
    { "subss", t_done, 0, true, { Vss, Wss, Zz }, 0, s1RW2R },
    { "subpd", t_done, 0, true, { Vpd, Wpd, Zz }, 0, s1RW2R },
    { "subsd", t_done, 0, true, { Vsd, Wsd, Zz }, 0, s1RW2R },
  },
  { /* SSE5D */
    { "minps", t_done, 0, true, { Vps, Wps, Zz }, 0, s1RW2R },
    { "minss", t_done, 0, true, { Vss, Wss, Zz }, 0, s1RW2R },
    { "minpd", t_done, 0, true, { Vpd, Wpd, Zz }, 0, s1RW2R },
    { "minsd", t_done, 0, true, { Vsd, Wsd, Zz }, 0, s1RW2R },
  },
  { /* SSE5E */
    { "divps", t_done, 0, true, { Vps, Wps, Zz }, 0, s1RW2R },
    { "divss", t_done, 0, true, { Vss, Wss, Zz }, 0, s1RW2R },
    { "divpd", t_done, 0, true, { Vpd, Wpd, Zz }, 0, s1RW2R },
    { "divsd", t_done, 0, true, { Vsd, Wsd, Zz }, 0, s1RW2R },
  },
  { /* SSE5F */
    { "maxps", t_done, 0, true, { Vps, Wps, Zz }, 0, s1RW2R },
    { "maxss", t_done, 0, true, { Vss, Wss, Zz }, 0, s1RW2R },
    { "maxpd", t_done, 0, true, { Vpd, Wpd, Zz }, 0, s1RW2R },
    { "maxsd", t_done, 0, true, { Vsd, Wsd, Zz }, 0, s1RW2R },
  },
  { /* SSE60 */
    { "punpcklbw", t_done, 0, true, { Pq, Qd, Zz }, 0, s1RW2R },
    { 0, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
    { "punpcklbw", t_done, 0, true, { Vdq, Wdq, Zz }, 0, s1RW2R },
    { 0, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
  },
  { /* SSE61 */
    { "punpcklwd", t_done, 0, true, { Pq, Qd, Zz }, 0, s1RW2R },
    { 0, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
    { "punpcklwd", t_done, 0, true, { Vdq, Wdq, Zz }, 0, s1RW2R },
    { 0, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
  },
  { /* SSE62 */
    { "punpcklqd", t_done, 0, true, { Pq, Qd, Zz }, 0, s1RW2R },
    { 0, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
    { "punpcklqd", t_done, 0, true, { Vdq, Wdq, Zz }, 0, s1RW2R },
    { 0, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
  },
  { /* SSE63 */
    { "packsswb", t_done, 0, true, { Pq, Qq, Zz }, 0, s1RW2R },
    { 0, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
    { "packsswb", t_done, 0, true, { Vdq, Wdq, Zz }, 0, s1RW2R },
    { 0, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
  },
  { /* SSE64 */
    { "pcmpgtb", t_done, 0, true, { Pq, Qq, Zz }, 0, s1R2R },
    { 0, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
    { "pcmpgtb", t_done, 0, true, { Vdq, Wdq, Zz }, 0, s1R2R },
    { 0, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
  },
  { /* SSE65 */
    { "pcmpgtw", t_done, 0, true, { Pq, Qq, Zz }, 0, s1R2R },
    { 0, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
    { "pcmpgtw", t_done, 0, true, { Vdq, Wdq, Zz }, 0, s1R2R },
    { 0, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
  },
  { /* SSE66 */
    { "pcmpgdt", t_done, 0, true, { Pq, Qq, Zz }, 0, s1R2R },
    { 0, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
    { "pcmpgdt", t_done, 0, true, { Vdq, Wdq, Zz }, 0, s1R2R },
    { 0, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
  },
  { /* SSE67 */
    { "packuswb", t_done, 0, true, { Pq, Qq, Zz }, 0, s1RW2R },
    { 0, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
    { "packuswb", t_done, 0, true, { Vdq, Wdq, Zz }, 0, s1RW2R },
    { 0, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
  },
  { /* SSE68 */
    { "punpckhbw", t_done, 0, true, { Pq, Qq, Zz }, 0, s1RW2R },
    { 0, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
    { "punpckhbw", t_done, 0, true, { Pdq, Qdq, Zz }, 0, s1RW2R },
    { 0, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
  },
  { /* SSE69 */
    { "punpckhwd", t_done, 0, true, { Pq, Qq, Zz }, 0, s1RW2R },
    { 0, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
    { "punpckhwd", t_done, 0, true, { Pdq, Qdq, Zz }, 0, s1RW2R },
    { 0, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
  },
  { /* SSE6A */
    { "punpckhdq", t_done, 0, true, { Pq, Qq, Zz }, 0, s1RW2R },
    { 0, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
    { "punpckhdq", t_done, 0, true, { Pdq, Qdq, Zz }, 0, s1RW2R },
    { 0, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
  },
  { /* SSE6B */
    { "packssdw", t_done, 0, true, { Pq, Qq, Zz }, 0, s1RW2R },
    { 0, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
    { "packssdw", t_done, 0, true, { Pdq, Qdq, Zz }, 0, s1RW2R },
    { 0, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
  },
  { /* SSE6C */
    { 0, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
    { 0, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
    { "punpcklqld", t_done, 0, true, { Vdq, Wdq, Zz }, 0, s1RW2R },
    { 0, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
  },
  { /* SSE6D */
    { 0, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
    { 0, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
    { "punpckhqd", t_done, 0, true, { Vdq, Wdq, Zz }, 0, s1RW2R },
    { 0, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
  },
  { /* SSE6E */
    { "movd", t_done, 0, true, { Pd, Ed, Zz }, 0, s1W2R },
    { 0, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
    { "movd", t_done, 0, true, { Vdq, Ed, Zz }, 0, s1W2R },
    { 0, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
  },
  { /* SSE6F */
    { "movq", t_done, 0, true, { Pq, Qq, Zz }, 0, s1W2R },
    { "movdqu", t_done, 0, false, { Vdq, Wdq, Zz }, 0, s1W2R }, // book has this/next swapped!!!
    { "movdqa", t_done, 0, true, { Vdq, Wdq, Zz }, 0, s1W2R },
    { 0, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
  },
  { /* SSE70 */
    { "pshufw", t_done, 0, true, { Pq, Qq, Ib }, 0, s1W2R3R },
    { "pshufhw", t_done, 0, true, { Vdq, Wdq, Ib }, 0, s1W2R3R }, // book has this/next swapped!!!
    { "pshufd", t_done, 0, true, { Vdq, Wdq, Ib }, 0, s1W2R3R },
    { "pshuflw", t_done, 0, true, { Vdq, Wdq, Ib }, 0, s1W2R3R },
  },
  { /* SSE74 */
    { "pcmpeqb", t_done, 0, true, { Pq, Qq, Zz }, 0, s1R2R },
    { 0, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
    { "pcmpeqb", t_done, 0, true, { Vdq, Wdq, Zz }, 0, s1R2R },
    { 0, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
  },
  { /* SSE75 */
    { "pcmpeqw", t_done, 0, true, { Pq, Qq, Zz }, 0, s1R2R },
    { 0, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
    { "pcmpeqw", t_done, 0, true, { Vdq, Wdq, Zz }, 0, s1R2R },
    { 0, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
  },
  { /* SSE76 */
    { "pcmpeqd", t_done, 0, true, { Pq, Qq, Zz }, 0, s1R2R },
    { 0, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
    { "pcmpeqd", t_done, 0, true, { Vdq, Wdq, Zz }, 0, s1R2R },
    { 0, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
  },
  { /* SSE7E */
    { "movd", t_done, 0, true, { Ed, Pd, Zz }, 0, s1W2R },
    { "movq", t_done, 0, true, { Vq, Wq, Zz }, 0, s1W2R }, // book has this and next swapped!!!
    { "movd", t_done, 0, true, { Ed, Vdq, Zz }, 0, s1W2R },
    { 0, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
  },
  { /* SSE7F */
    { "movq", t_done, 0, true, { Qq, Pq, Zz }, 0, s1W2R },
    { "movdqu", t_done, 0, true, { Wdq, Vdq, Zz }, 0, s1W2R }, // book has this and next swapped!!!
    { "movdqa", t_done, 0, true, { Wdq, Vdq, Zz }, 0, s1W2R },
    { 0, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
  },
  { /* SSEC2 */
    { "cmpps", t_done, 0, true, { Vps, Wps, Ib }, 0, s1RW2R3R }, // comparison writes to dest!
    { "cmpss", t_done, 0, true, { Vss, Wss, Ib }, 0, s1RW2R3R },
    { "cmppd", t_done, 0, true, { Vpd, Wpd, Ib }, 0, s1RW2R3R },
    { "cmpsd", t_done, 0, true, { Vsd, Wsd, Ib }, 0, s1RW2R3R },
  },
  { /* SSEC4 */
    { "pinsrw", t_done, 0, true, { Pq, Ed, Ib }, 0, s1RW2R3R },
    { 0, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
    { "pinsrw", t_done, 0, true, { Vdq, Ed, Ib }, 0, s1RW2R3R },
    { 0, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
  },
  { /* SSEC5 */
    { "pextrw", t_done, 0, true, { Gd, Pq, Ib }, 0, s1W2R3R },
    { 0, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
    { "pextrw", t_done, 0, true, { Gd, Vdq, Ib }, 0, s1W2R3R },
    { 0, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
  },
  { /* SSEC6 */
    { "shufps", t_done, 0, true, { Vps, Wps, Ib }, 0, s1RW2R3R },
    { 0, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
    { "shufpd", t_done, 0, true, { Vpd, Wpd, Ib }, 0, s1RW2R3R },
    { 0, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
  },
  { /* SSED1 */
    { "psrlw", t_done, 0, true, { Pq, Qq, Zz }, 0, s1RW2R },
    { 0, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
    { "psrlw", t_done, 0, true, { Vdq, Wdq, Zz }, 0, s1RW2R },
    { 0, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
  },
  { /* SSED2 */
    { "psrld", t_done, 0, true, { Pq, Qq, Zz }, 0, s1RW2R },
    { 0, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
    { "psrld", t_done, 0, true, { Vdq, Wdq, Zz }, 0, s1RW2R },
    { 0, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
  },
  { /* SSED3 */
    { "psrlq", t_done, 0, true, { Pq, Qq, Zz }, 0, s1RW2R },
    { 0, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
    { "psrlq", t_done, 0, true, { Vdq, Wdq, Zz }, 0, s1RW2R },
    { 0, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
  },
  { /* SSED4 */
    { "paddq", t_done, 0, true, { Pq, Qq, Zz }, 0, s1RW2R },
    { 0, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
    { "paddq", t_done, 0, true, { Vdq, Wdq, Zz }, 0, s1RW2R },
    { 0, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
  },
  { /* SSED5 */
    { "pmullw", t_done, 0, true, { Pq, Qq, Zz }, 0, s1RW2R },
    { 0, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
    { "pmullw", t_done, 0, true, { Vdq, Wdq, Zz }, 0, s1RW2R },
    { 0, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
  },
  { /* SSED6 */
    { 0, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
    { "movq2dq", t_done, 0, true, { Vdq, Qq, Zz }, 0, s1W2R }, // lines jumbled in book
    { "movq", t_done, 0, true, { Wq, Vq, Zz }, 0, s1W2R },
    { "movdq2q", t_done, 0, true, { Pq, Wq, Zz }, 0, s1W2R },
  },
  { /* SSED7 */
    { "pmovmskb", t_done, 0, true, { Gd, Pq, Zz }, 0, s1W2R },
    { 0, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
    { "pmovmskb", t_done, 0, true, { Gd, Vdq, Zz }, 0, s1W2R },
    { 0, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
  },
  { /* SSED8 */
    { "psubusb", t_done, 0, true, { Pq, Qq, Zz }, 0, s1RW2R },
    { 0, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
    { "psubusb", t_done, 0, true, { Vdq, Wdq, Zz }, 0, s1RW2R },
    { 0, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
  },
  { /* SSED9 */
    { "psubusw", t_done, 0, true, { Pq, Qq, Zz }, 0, s1RW2R },
    { 0, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
    { "psubusw", t_done, 0, true, { Vdq, Wdq, Zz }, 0, s1RW2R },
    { 0, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
  },
  { /* SSEDA */
    { "pminub", t_done, 0, true, { Pq, Qq, Zz }, 0, s1RW2R },
    { 0, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
    { "pminub", t_done, 0, true, { Vdq, Wdq, Zz }, 0, s1RW2R },
    { 0, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
  },
  { /* SSEDB */
    { "pand", t_done, 0, true, { Pq, Qq, Zz }, 0, s1RW2R },
    { 0, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
    { "pand", t_done, 0, true, { Vdq, Wdq, Zz }, 0, s1RW2R },
    { 0, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
  },
  { /* SSEDC */
    { "paddusb", t_done, 0, true, { Pq, Qq, Zz }, 0, s1RW2R },
    { 0, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
    { "paddusb", t_done, 0, true, { Vdq, Wdq, Zz }, 0, s1RW2R },
    { 0, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
  },
  { /* SSEDD */
    { "paddusw", t_done, 0, true, { Pq, Qq, Zz }, 0, s1RW2R },
    { 0, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
    { "paddusw", t_done, 0, true, { Vdq, Wdq, Zz }, 0, s1RW2R },
    { 0, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
  },
  { /* SSEDE */
    { "pmaxub", t_done, 0, true, { Pq, Qq, Zz }, 0, s1RW2R },
    { 0, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
    { "pmaxub", t_done, 0, true, { Vdq, Wdq, Zz }, 0, s1RW2R },
    { 0, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
  },
  { /* SSEDF */
    { "pandn", t_done, 0, true, { Pq, Qq, Zz }, 0, s1RW2R },
    { 0, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
    { "pandn", t_done, 0, true, { Vdq, Wdq, Zz }, 0, s1RW2R },
    { 0, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
  },
  { /* SSEE0 */
    { "pavgb", t_done, 0, true, { Pq, Qq, Zz }, 0, s1RW2R },
    { 0, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
    { "pavgb", t_done, 0, true, { Vdq, Wdq, Zz }, 0, s1RW2R },
    { 0, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
  },
  { /* SSEE1 */
    { "psraw", t_done, 0, true, { Pq, Qq, Zz }, 0, s1RW2R },
    { 0, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
    { "psraw", t_done, 0, true, { Vdq, Wdq, Zz }, 0, s1RW2R },
    { 0, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
  },
  { /* SSEE2 */
    { "psrad", t_done, 0, true, { Pq, Qq, Zz }, 0, s1RW2R },
    { 0, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
    { "psrad", t_done, 0, true, { Vdq, Wdq, Zz }, 0, s1RW2R },
    { 0, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
  },
  { /* SSEE3 */
    { "pavgw", t_done, 0, true, { Pq, Qq, Zz }, 0, s1RW2R },
    { 0, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
    { "pavgw", t_done, 0, true, { Vdq, Wdq, Zz }, 0, s1RW2R },
    { 0, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
  },
  { /* SSEE4 */
    { "pmulhuw", t_done, 0, true, { Pq, Qq, Zz }, 0, s1RW2R },
    { 0, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
    { "pmulhuw", t_done, 0, true, { Vdq, Wdq, Zz }, 0, s1RW2R },
    { 0, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
  },
  { /* SSEE5 */
    { "pmulhw", t_done, 0, true, { Pq, Qq, Zz }, 0, s1RW2R },
    { 0, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
    { "pmulhw", t_done, 0, true, { Vdq, Wdq, Zz }, 0, s1RW2R },
    { 0, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
  },
  { /* SSEE6 */
    { 0, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
    { "cvtdq2pd", t_done, 0, true, { Vpd, Wdq, Zz }, 0, s1W2R }, // lines jumbled in book
    { "cvttpd2dq", t_done, 0, true, { Vdq, Wpd, Zz }, 0, s1W2R },
    { "cvtpd2dq", t_done, 0, true, { Vdq, Wpd, Zz }, 0, s1W2R },
  },
  { /* SSEE7 */
    { "movntq", t_done, 0, true, { Wq, Vq, Zz }, 0, s1W2R | (fNT << FPOS) },
    { 0, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
    { "movntdq", t_done, 0, true, { Wdq, Vdq, Zz }, 0, s1W2R | (fNT << FPOS) },
    { 0, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
  },
  { /* SSEE8 */
    { "psubsb", t_done, 0, true, { Pq, Qq, Zz }, 0, s1RW2R },
    { 0, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
    { "psubsb", t_done, 0, true, { Vdq, Wdq, Zz }, 0, s1RW2R },
    { 0, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
  },
  { /* SSEE9 */
    { "psubsw", t_done, 0, true, { Pq, Qq, Zz }, 0, s1RW2R },
    { 0, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
    { "psubsw", t_done, 0, true, { Vdq, Wdq, Zz }, 0, s1RW2R },
    { 0, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
  },
  { /* SSEEA */
    { "pminsw", t_done, 0, true, { Pq, Qq, Zz }, 0, s1RW2R },
    { 0, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
    { "pminsw", t_done, 0, true, { Vdq, Wdq, Zz }, 0, s1RW2R },
    { 0, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
  },
  { /* SSEEB */
    { "por", t_done, 0, true, { Pq, Qq, Zz }, 0, s1RW2R },
    { 0, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
    { "por", t_done, 0, true, { Vdq, Wdq, Zz }, 0, s1RW2R },
    { 0, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
  },
  { /* SSEEC */
    { "paddsb", t_done, 0, true, { Pq, Qq, Zz }, 0, s1RW2R },
    { 0, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
    { "paddsb", t_done, 0, true, { Vdq, Wdq, Zz }, 0, s1RW2R },
    { 0, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
  },
  { /* SSEED */
    { "paddsw", t_done, 0, true, { Pq, Qq, Zz }, 0, s1RW2R },
    { 0, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
    { "paddsw", t_done, 0, true, { Vdq, Wdq, Zz }, 0, s1RW2R },
    { 0, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
  },
  { /* SSEEE */
    { "pmaxsw", t_done, 0, true, { Pq, Qq, Zz }, 0, s1RW2R },
    { 0, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
    { "pmaxsw", t_done, 0, true, { Vdq, Wdq, Zz }, 0, s1RW2R },
    { 0, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
  },
  { /* SSEEF */
    { "pxor", t_done, 0, true, { Pq, Qq, Zz }, 0, s1RW2R },
    { 0, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
    { "pxor", t_done, 0, true, { Vdq, Wdq, Zz }, 0, s1RW2R },
    { 0, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
  },
  { /* SSEF1 */
    { "psllw", t_done, 0, true, { Pq, Qq, Zz }, 0, s1RW2R },
    { 0, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
    { "psllw", t_done, 0, true, { Vdq, Wdq, Zz }, 0, s1RW2R },
    { 0, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
  },
  { /* SSEF2 */
    { "pslld", t_done, 0, true, { Pq, Qq, Zz }, 0, s1RW2R },
    { 0, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
    { "pslld", t_done, 0, true, { Vdq, Wdq, Zz }, 0, s1RW2R },
    { 0, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
  },
  { /* SSEF3 */
    { "psllq", t_done, 0, true, { Pq, Qq, Zz }, 0, s1RW2R },
    { 0, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
    { "psllq", t_done, 0, true, { Vdq, Wdq, Zz }, 0, s1RW2R },
    { 0, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
  },
  { /* SSEF4 */
    { "pmuludq", t_done, 0, true, { Pq, Qq, Zz }, 0, s1RW2R },
    { 0, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
    { "pmuludq", t_done, 0, true, { Vdq, Wdq, Zz }, 0, s1RW2R },
    { 0, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
  },
  { /* SSEF5 */
    { "pmaddwd", t_done, 0, true, { Pq, Qq, Zz }, 0, s1RW2R },
    { 0, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
    { "pmaddwd", t_done, 0, true, { Vdq, Wdq, Zz }, 0, s1RW2R },
    { 0, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
  },
  { /* SSEF6 */
    { "psadbw", t_done, 0, true, { Pq, Qq, Zz }, 0, s1RW2R },
    { 0, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
    { "psadbw", t_done, 0, true, { Vdq, Wdq, Zz }, 0, s1RW2R },
    { 0, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
  },
  { /* SSEF7 */
    { "maskmovq", t_done, 0, true, { Ppi, Qpi, Zz }, 0, s1W2R | (fNT << FPOS) },
    { 0, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
    { "maskmovdqu", t_done, 0, true, { Vdq, Wdq, Zz }, 0, s1W2R | (fNT << FPOS) }, // bug in book
    { 0, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
  },
  { /* SSEF8 */
    { "psubb", t_done, 0, true, { Pq, Qq, Zz }, 0, s1RW2R },
    { 0, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
    { "psubb", t_done, 0, true, { Vdq, Wdq, Zz }, 0, s1RW2R },
    { 0, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
  },
  { /* SSEF9 */
    { "psubw", t_done, 0, true, { Pq, Qq, Zz }, 0, s1RW2R },
    { 0, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
    { "psubw", t_done, 0, true, { Vdq, Wdq, Zz }, 0, s1RW2R },
    { 0, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
  },
  { /* SSEFA */
    { "psubd", t_done, 0, true, { Pq, Qq, Zz }, 0, s1RW2R },
    { 0, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
    { "psubd", t_done, 0, true, { Vdq, Wdq, Zz }, 0, s1RW2R },
    { 0, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
  },
  { /* SSEFB */ // FIXME: Same????
    { "psubd", t_done, 0, true, { Pq, Qq, Zz }, 0, s1RW2R },
    { 0, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
    { "psubd", t_done, 0, true, { Vdq, Wdq, Zz }, 0, s1RW2R },
    { 0, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
  },
  { /* SSEFC */
    { "paddb", t_done, 0, true, { Pq, Qq, Zz }, 0, s1RW2R },
    { 0, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
    { "paddb", t_done, 0, true, { Vdq, Wdq, Zz }, 0, s1RW2R },
    { 0, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
  },
  { /* SSEFD */
    { "paddw", t_done, 0, true, { Pq, Qq, Zz }, 0, s1RW2R },
    { 0, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
    { "paddw", t_done, 0, true, { Vdq, Wdq, Zz }, 0, s1RW2R },
    { 0, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
  },
  { /* SSEFE */
    { "paddd", t_done, 0, true, { Pq, Qq, Zz }, 0, s1RW2R },
    { 0, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
    { "paddd", t_done, 0, true, { Vdq, Wdq, Zz }, 0, s1RW2R },
    { 0, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
  }
};

/* rows are none or 66 prefixed in this order (see book) */
static ia32_entry ssegrpMap[][2] = {
  /* G12SSE010B */
  {
    { "psrlw", t_done, 0, true, { Pq, Ib, Zz }, 0, s1RW2R },
    { "psrlw", t_done, 0, true, { Pdq, Ib, Zz }, 0, s1RW2R }
  },
  /* G12SSE100B */
  {
    { "psraw", t_done, 0, true, { Pq, Ib, Zz }, 0, s1RW2R },
    { "psraw", t_done, 0, true, { Pdq, Ib, Zz }, 0, s1RW2R }
  },
  /* G12SSE110B */
  {
    { "psllw", t_done, 0, true, { Pq, Ib, Zz }, 0, s1RW2R },
    { "psllw", t_done, 0, true, { Pdq, Ib, Zz }, 0, s1RW2R }
  },
  /* G13SSE010B */
  {
    { "psrld", t_done, 0, true, { Pq, Ib, Zz }, 0, s1RW2R },
    { "psrld", t_done, 0, true, { Wdq, Ib, Zz }, 0, s1RW2R }
  },
  /* G13SSE100B */
  {
    { "psrad", t_done, 0, true, { Pq, Ib, Zz }, 0, s1RW2R },
    { "psrad", t_done, 0, true, { Wdq, Ib, Zz }, 0, s1RW2R }
  },
  /* G13SSE110B */
  {
    { "pslld", t_done, 0, true, { Pq, Ib, Zz }, 0, s1RW2R },
    { "pslld", t_done, 0, true, { Wdq, Ib, Zz }, 0, s1RW2R }
  },
  /* G14SSE010B */
  {
    { "psrlq", t_done, 0, true, { Pq, Ib, Zz }, 0, s1RW2R },
    { "psrlq", t_done, 0, true, { Wdq, Ib, Zz }, 0, s1RW2R }
  },
  /* G14SSE011B */
  {
    { 0, t_ill, 0, true, { Zz, Zz, Zz }, 0, 0 },
    { "psrldq", t_done, 0, true, { Wdq, Ib, Zz }, 0, s1RW2R }
  },
  /* G14SSE110B */
  {
    { "psllq", t_done, 0, true, { Pq, Ib, Zz }, 0, s1RW2R },
    { "psllq", t_done, 0, true, { Wdq, Ib, Zz }, 0, s1RW2R }
  },
  /* G14SSE111B */
  {
    { 0, t_ill, 0, true, { Zz, Zz, Zz }, 0, 0 },
    { "pslldq", t_done, 0, true, { Wdq, Ib, Zz }, 0, s1RW2R }
  }
};

static bool mode_64 = false;

void ia32_set_mode_64(bool mode) {
  mode_64 = mode;
}

bool ia32_is_mode_64() {
  return mode_64;
}

ia32_entry movsxd = { "movsxd", t_done, 0, true, { Gv, Ed, Zz }, 0, s1W2R };

static void ia32_translate_for_64(ia32_entry** gotit_ptr)
{
    if (*gotit_ptr == &oneByteMap[0x63]) // APRL redefined to MOVSXD
	*gotit_ptr = &movsxd;
}

static unsigned int ia32_decode_modrm(const unsigned int addrSzAttr,
                                      const unsigned char* addr,
                                      ia32_memacc* macadr = NULL);


#if defined(i386_unknown_nt4_0) && _MSC_VER < 1300
ia32_instruction& ia32_decode(unsigned int capa, const unsigned char* addr, ia32_instruction& instruct)
#else
template <unsigned int capa>
ia32_instruction& ia32_decode(const unsigned char* addr, ia32_instruction& instruct)
#endif
{
  ia32_prefixes& pref = instruct.prf;
  unsigned int table, nxtab;
  unsigned int idx, sseidx = 0;
  ia32_entry *gotit = NULL;
  int condbits;

  if(capa & IA32_DECODE_MEMACCESS)
    assert(instruct.mac != NULL);

  if (!ia32_decode_prefixes(addr, pref)) {
    instruct.size = 1;
    instruct.legacy_type = ILLEGAL;
    return instruct;
  }
    
  instruct.size = pref.getCount();
  addr += instruct.size;

  table = t_oneB;
  idx = addr[0];
  gotit = &oneByteMap[idx];
  nxtab = gotit->otable;
  instruct.size += 1;
  addr += 1;

  if(capa & IA32_DECODE_CONDITION) {
    assert(instruct.cond != NULL);
    condbits = idx & 0x0F;
  }

  // ensure mac/cond processing not done until we implement it for 64-bit mode
  if (mode_64) {
    assert(instruct.mac == NULL);
    assert(instruct.cond == NULL);
  }

  while(nxtab != t_done) {
    table = nxtab;
    switch(table) {
    case t_twoB:
      idx = addr[0];
      gotit = &twoByteMap[idx];
      nxtab = gotit->otable;
      instruct.size += 1;
      addr += 1;
      if(capa & IA32_DECODE_CONDITION)
        condbits = idx & 0x0F;
      break;
    case t_prefixedSSE:
      sseidx = gotit->tabidx;
      assert(addr[0] == 0x0F);
      idx = addr[1];
      gotit = &twoByteMap[idx];
      nxtab = gotit->otable;
      instruct.size += 2;
      addr += 2;
      break;
    case t_sse:
      idx = gotit->tabidx;
      gotit = &sseMap[idx][sseidx];
      nxtab = gotit->otable;
      break;
    case t_grp: {
      idx = gotit->tabidx;
      unsigned int reg  = (addr[0] >> 3) & 7;
      if(idx < Grp12)
        switch(idx) {
        case Grp2:
        case Grp11:
          // leave table unchanged because operands are in not defined in group map
          nxtab = groupMap[idx][reg].otable;
          assert(nxtab==t_done || nxtab==t_ill);
          break;
        default:
          gotit = &groupMap[idx][reg];
          nxtab = gotit->otable;
        }
      else {
        unsigned int mod = addr[0] >> 6;
        gotit = &groupMap2[idx-Grp12][mod==3][reg];
        nxtab = gotit->otable;
      }
      break;
    }
    case t_grpsse:
      sseidx >>= 1;
      idx = gotit->tabidx;
      gotit = &ssegrpMap[idx][sseidx];
      nxtab = gotit->otable;
      break;
    case t_coprocEsc:
      instruct.legacy_type = 0;
      if(capa & IA32_DECODE_CONDITION)
        ; // FIXME: translation to tttn & set it
      return ia32_decode_FP(idx, pref, addr, instruct, instruct.mac);
    case t_3dnow:
      // 3D now opcodes are given as suffix: ModRM [SIB] [displacement] opcode
      // Right now we don't care what the actual opcode is, so there's no table
      instruct.size += 1;
      nxtab = t_done;
      break;
    case t_ill:
      instruct.legacy_type = ILLEGAL;
      return instruct;
    default:
      assert(!"wrong table");
    }
  }

  // addr points after the opcode, and the size has been adjusted accordingly

  assert(gotit != NULL);
  instruct.legacy_type = gotit->legacyType;

  // make adjustments for instruction redefined in 64-bit mode
  if (mode_64)
    ia32_translate_for_64(&gotit);

  ia32_decode_operands(pref, *gotit, addr, instruct, instruct.mac); // all but FP

  if(capa & IA32_DECODE_MEMACCESS) {
    int sema = gotit->opsema & ((1<<FPOS)-1);
    int hack = gotit->opsema >> FPOS;
    switch(sema) {
    case sNONE:
      break;
    case s1R:
      switch(hack) {
      case fPREFETCHNT:
        instruct.mac[0].prefetch = true;
        instruct.mac[0].prefetchlvl = 0;
        break;
      case fPREFETCHT0:
        instruct.mac[0].prefetch = true;
        instruct.mac[0].prefetchlvl = 1;
        break;
      case fPREFETCHT1:
        instruct.mac[0].prefetch = true;
        instruct.mac[0].prefetchlvl = 2;
        break;
      case fPREFETCHT2:
        instruct.mac[0].prefetch = true;
        instruct.mac[0].prefetchlvl = 3;
        break;
      case fPREFETCHAMDE:
        instruct.mac[0].prefetch = true;
        instruct.mac[0].prefetchstt = 0;
        break;
      case fPREFETCHAMDW:
        instruct.mac[0].prefetch = true;
        instruct.mac[0].prefetchstt = 1;
        break;
      default:
        instruct.mac[0].read = true;
      }
      break;
    case s1W:
      instruct.mac[0].write = true;
      break;
    case s1RW:
      instruct.mac[0].read = true;
      instruct.mac[0].write = true;
      instruct.mac[0].nt = hack == fNT;
      break;
    case s1R2R:
      instruct.mac[0].read = true;
      instruct.mac[1].read = true;
      break;
    case s1W2R:
      instruct.mac[0].write = true;
      instruct.mac[0].nt = hack == fNT; // all NTs are s1W2R
      instruct.mac[1].read = true;
      break;
    case s1RW2R:
      instruct.mac[0].read = true;
      instruct.mac[0].write = true;
      instruct.mac[1].read = true;
      break;
    case s1RW2RW:
      instruct.mac[0].read = true;
      instruct.mac[0].write = true;
      instruct.mac[1].read = true;
      instruct.mac[1].write = true;
      break;
    case s1W2R3R:
      instruct.mac[0].write = true;
      instruct.mac[1].read = true;
      instruct.mac[2].read = true;
      break;
    case s1W2W3R:
      instruct.mac[0].write = true;
      instruct.mac[1].write = true;
      instruct.mac[2].read = true;
      break;
    case s1W2RW3R:
      instruct.mac[0].write = true;
      instruct.mac[1].read = true;
      instruct.mac[1].write = true;
      instruct.mac[2].read = true;
      break;
    case s1W2R3RW:
      instruct.mac[0].write = true;
      instruct.mac[1].read = true;
      instruct.mac[2].read = true;
      instruct.mac[2].write = true;
      break;
    case s1RW2R3R:
      instruct.mac[0].read = true;
      instruct.mac[0].write = true;
      instruct.mac[1].read = true;
      instruct.mac[2].read = true;
      break;
    case s1RW2RW3R:
      instruct.mac[0].write = true;
      instruct.mac[0].read = true;
      instruct.mac[1].read = true;
      instruct.mac[1].write = true;
      instruct.mac[2].read = true;
      break;
    }

    switch(pref.getPrefix(0)) {
    case PREFIX_REPNZ:
      switch(hack) {
      case fSCAS:
        instruct.mac[1].sizehack = shREPNESCAS;
        break;
      case fCMPS:
        instruct.mac[0].sizehack = shREPNECMPS;
        instruct.mac[1].sizehack = shREPNECMPS;
        break;
      default:
        bperr( "IA32 DECODER: unexpected repnz prefix ignored...\n");
      }
      break;
    case PREFIX_REP:
      switch(hack) {
      case fSCAS:
        instruct.mac[1].sizehack = shREPESCAS;
        break;
      case fCMPS:
        instruct.mac[0].sizehack = shREPECMPS;
        instruct.mac[1].sizehack = shREPECMPS;
        break;
      case fREP:
        instruct.mac[0].sizehack = shREP;
        instruct.mac[1].sizehack = shREP;
        break;
      default:
        bperr( "IA32 DECODER: unexpected rep prefix ignored...\n");
      }
      break;
    case 0:
    case PREFIX_LOCK:
      break;
    default:
      bperr( "IA32 WARNING: unknown type 0 prefix!\n");
      break;
    }
  }

  if(capa & IA32_DECODE_CONDITION) {
    int hack = gotit->opsema >> FPOS;
    if(hack == fCOND)
      instruct.cond->set(condbits);
  }

  instruct.entry = gotit;
  return instruct;
}

#if !defined(i386_unknown_nt4_0) || _MSC_VER >= 1300 
// MS VS 6.0 compiler gives internal compiler error on this
template ia32_instruction& ia32_decode<0>(const unsigned char* addr, ia32_instruction& instruct);
// FIXME: remove
template ia32_instruction& ia32_decode<IA32_DECODE_MEMACCESS>
                             (const unsigned char* addr,ia32_instruction& instruct);
template ia32_instruction& ia32_decode<(IA32_DECODE_MEMACCESS|IA32_DECODE_CONDITION)>
                                      (const unsigned char* addr,ia32_instruction& instruct);
#endif

ia32_instruction& ia32_decode_FP(unsigned int opcode, const ia32_prefixes& pref,
                                 const unsigned char* addr, ia32_instruction& instruct,
                                 ia32_memacc *mac)
{
  unsigned int nib = byteSzB; // modRM
  unsigned int addrSzAttr = (pref.getPrefix(3) == PREFIX_SZADDR ? 1 : 2); // 32-bit mode implicit
  unsigned int operSzAttr = (pref.getPrefix(2) == PREFIX_SZOPER ? 1 : 2); // 32-bit mode implicit

  if (addr[0] <= 0xBF) { // modrm
    nib += ia32_decode_modrm(addrSzAttr, addr, mac);
    // operand size has to be determined from opcode
    if(mac)
      switch(opcode) {
      case 0xD8: // all single real
        mac->size = 4;
        mac->read = true;
        break;
      case 0xD9: {
        unsigned char modrm = addr[0];
        unsigned char reg = (modrm >> 3) & 7;
        switch(reg) {
        case 0:
          mac->size = 4;
          mac->read = true;
          break;
        case 2:
        case 3:
          mac->size = 4;
          mac->write = true;
          break;
        case 1:
          instruct.legacy_type = ILLEGAL;
          break;
        case 4:
          mac->size = 14 * operSzAttr;
          mac->read = true;
          break;
        case 5:
        mac->read = true;
        mac->size = 2;
        break;
        case 6:
          mac->size = 14 * operSzAttr;
          mac->write = true;
          break;
        case 7:
          mac->write = true;
          mac->size = 2;
          break;
        }
        break; }
      case 0xDA:  // all double real
        mac->size = 8;
        mac->read = true;
        break;
      case 0xDB: {
        unsigned char modrm = addr[0];
        unsigned char reg = (modrm >> 3) & 7;
        switch(reg) {
        case 0:
          mac->size = dwordSzB;
          mac->read = true;
          break;
        case 2:
        case 3:
          mac->size = dwordSzB;
          mac->write = true;
          break;
        case 1:
        case 4:
        case 6:
          instruct.legacy_type = ILLEGAL;
          break;
        case 5:
          mac->size = 10; // extended real
          mac->read = true;
          break;
        case 7:
          mac->size = 10; // extended real
          mac->write = true;
          break;
        }
        break; }
      case 0xDC:   // all double real
        mac->size = 8;
        mac->read = true;
        break;
      case 0xDD: {
        unsigned char modrm = addr[0];
        unsigned char reg = (modrm >> 3) & 7;
        switch(reg) {
        case 0:
          mac->size = 8;
          mac->read = true;
          break;
        case 2:
        case 3:
          mac->size = 8;
          mac->write = true;
          break;
        case 1:
        case 5:
          instruct.legacy_type = ILLEGAL;
          break;
        case 4:
          mac->size = operSzAttr == 2 ? 108 : 98;
          mac->read = true;
          break;
        case 6:
          mac->size = operSzAttr == 2 ? 108 : 98;
          mac->write = true;
          break;
        case 7:
          mac->size = 2;
          mac->write = true;
          break;    
        }
        break; }
      case 0xDE: // all word integer
        mac->size = wordSzB;
        mac->write = true;
        break;
      case 0xDF: {
        unsigned char modrm = addr[0];
        unsigned char reg = (modrm >> 3) & 7;
        switch(reg) {
        case 0:
          mac->size = wordSzB;
          mac->read = true;
          break;
        case 2:
        case 3:
          mac->size = wordSzB;
          mac->write = true;
          break;
        case 1:
          instruct.legacy_type = ILLEGAL;
          break;
        case 4:
          mac->size = 10;
          mac->read = true;
          break;
        case 5:
          mac->size = 8;
          mac->read = true;
          break;
        case 6:
          mac->size = 10;
          mac->write = true;
          break;
        case 7:
          mac->size = 8;
          mac->write = true;
          break;
        }
        break; }
      } // switch(opcode)
  } // if modrm
  
  instruct.size += nib;

  return instruct;
}

static unsigned int ia32_decode_modrm(const unsigned int addrSzAttr,
                                      const unsigned char* addr,
                                      ia32_memacc* macadr)
{
 
  unsigned char modrm = addr[0];
  unsigned char mod = modrm >> 6;
  unsigned char rm  = modrm & 7;
  //unsigned char reg = (modrm >> 3) & 7;
  ++addr;

  if(addrSzAttr == 1) { // 16-bit, cannot have SIB
    //bperr( "16bit addr!\n");
    switch(mod) {
    case 0:
      //bperr( "16bit addr - case 0, rm = %d!\n", rm);
      if(macadr)
        switch (rm) {
        case 0: // [BX+SI]
          macadr->set16(mBX, mSI, 0);
          break;
        case 1:
          macadr->set16(mBX, mDI, 0);
          break;
        case 2:
          macadr->set16(mBP, mSI, 0);
          break;
        case 3:
          macadr->set16(mBP, mDI, 0);
          break;
        case 4:
          macadr->set16(mSI, -1, 0);
          break;
        case 5:
          macadr->set16(mDI, -1, 0);
          break;
        case 6: { // disp16
          const short int *pdisp16 = (const short int*)addr;
          //disp16 = (short)addr[0] + ((short)addr[1]) << 8;
          //bperr( "16bit addr - case6!\n");
          macadr->set16(-1, -1, *pdisp16);
          break; }
        case 7:
          macadr->set16(mBX, -1, 0);
          break;
        }
      return rm==6 ? wordSzB : 0;
    case 1:
      //bperr( "16bit addr - case1!\n");
      if(macadr) {
        const char *pdisp8 = (const char*)addr;
        switch (rm) {
        case 0: 
          macadr->set16(mBX, mSI, *pdisp8);
          break;
        case 1:
          macadr->set16(mBX, mDI, *pdisp8);
          break;
        case 2:
          macadr->set16(mBP, mSI, *pdisp8);
          break;
        case 3:
          macadr->set16(mBP, mDI, *pdisp8);
          break;
        case 4:
          macadr->set16(mSI, -1, *pdisp8);
          break;
        case 5:
          macadr->set16(mDI, -1, *pdisp8);
          break;
        case 6:
          macadr->set16(mBP, -1, *pdisp8);
          break;
        case 7:
          macadr->set16(mBX, -1, *pdisp8);
          break;
        }
      }
      return byteSzB;
    case 2:
      //bperr( "16bit addr - case2!\n");
      if(macadr) {
        const short int *pdisp16 = (const short int*)addr;
        switch (rm) {
        case 0: 
          macadr->set16(mBX, mSI, *pdisp16);
          break;
        case 1:
          macadr->set16(mBX, mDI, *pdisp16);
          break;
        case 2:
          macadr->set16(mBP, mSI, *pdisp16);
          break;
        case 3:
          macadr->set16(mBP, mDI, *pdisp16);
          break;
        case 4:
          macadr->set16(mSI, -1, *pdisp16);
          break;
        case 5:
          macadr->set16(mDI, -1, *pdisp16);
          break;
        case 6:
          macadr->set16(mBP, -1, *pdisp16);
          break;
        case 7:
          macadr->set16(mBX, -1, *pdisp16);
          break;
        }
      }

      return wordSzB;
    case 3:
      return 0; // register
    default:
      assert(0);
    }
  }
  else { // 32-bit or 64-bit, may have SIB
    if(mod == 3)
      return 0; // only registers, no SIB
    bool hassib = rm == 4;
    unsigned int nsib = 0;
    unsigned char sib;
    int base = 0, scale = -1, index = -1;  // prevent g++ from whining.
    if(hassib) {
      nsib = byteSzB;
      sib = addr[0];
      ++addr;
      base = sib & 7;
      if(macadr) {
        scale = sib >> 6;
        index = (sib >> 3) & 7;
        if(index == 4)
          index = -1;
      }
    }
    switch(mod) {
    case 0: {
      /* this is tricky: there is a disp32 iff (1) rm == 5  or  (2) rm == 4 && base == 5 */
      /* note: this doesn't change for 64-bit mode, only diff is disp32 is added to RIP */
      unsigned char check5 = hassib ? base : rm;
      if(macadr)
        switch (rm) {
        case 0:
          macadr->set32(mEAX, 0);
          break;
        case 1:
          macadr->set32(mECX, 0);
          break;
        case 2:
          macadr->set32(mEDX, 0);
          break;
        case 3:
          macadr->set32(mEBX, 0);
          break;
        case 4:
          if(base == 5) { // disp32[index<<scale]
            const int *pdisp32 = (const int*)addr;
            macadr->set32sib(-1, scale, index, *pdisp32);
          }
          else
            macadr->set32sib(base, scale, index, 0);
          break;
        case 5: { // disp32
          const int *pdisp32 = (const int*)addr;
          macadr->set32(-1, *pdisp32);
          break; }
        case 6:
          macadr->set32(mESI, 0);
          break;
        case 7:
          macadr->set32(mEDI, 0);
          break;
        }
      return nsib + ((check5 == 5) ? dwordSzB : 0);
    }
    case 1:
      if(macadr) {
        const char *pdisp8 = (const char*)addr;
        switch (rm) {
        case 0:
          macadr->set32(mEAX, *pdisp8);
          break;
        case 1:
          macadr->set32(mECX, *pdisp8);
          break;
        case 2:
          macadr->set32(mEDX, *pdisp8);
          break;
        case 3:
          macadr->set32(mEBX, *pdisp8);
          break;
        case 4:
          // disp8[EBP + index<<scale] happens naturally here when base=5
          macadr->set32sib(base, scale, index, *pdisp8);
          break;
        case 5:
          macadr->set32(mEBP, *pdisp8);
          break;
        case 6:
          macadr->set32(mESI, *pdisp8);
          break;
        case 7:
          macadr->set32(mEDI, *pdisp8);
          break;
        }
      }
      return nsib + byteSzB;
    case 2:
      if(macadr) {
        const int *pdisp32 = (const int*)addr;
        switch (rm) {
        case 0:
          macadr->set32(mEAX, *pdisp32);
          break;
        case 1:
          macadr->set32(mECX, *pdisp32);
          break;
        case 2:
          macadr->set32(mEDX, *pdisp32);
          break;
        case 3:
          macadr->set32(mEBX, *pdisp32);
          break;
        case 4:
          // disp32[EBP + index<<scale] happens naturally here when base=5
          macadr->set32sib(base, scale, index, *pdisp32);
          break;
        case 5:
          macadr->set32(mEBP, *pdisp32);
          break;
        case 6:
          macadr->set32(mESI, *pdisp32);
          break;
        case 7:
          macadr->set32(mEDI, *pdisp32);
          break;
        }
      }
      return nsib + dwordSzB;
    default:
      assert(0);
    }
  }
  return 0; // MS compiler from VS 6.0 wants this
}


static inline int type2size(unsigned int optype, unsigned int operSzAttr)
{
  switch(optype) {
  case op_a:
    return 2 * wordSzB * operSzAttr;
  case op_b:
    return byteSzB;
  case op_c:
    assert(!"Where is this used, Intel?");
    return byteSzB * operSzAttr;
  case op_d:
    return dwordSzB;
  case op_dq:
    return dqwordSzB;
  case op_p:
    return wordSzB + wordSzB * operSzAttr; // XXX book says operand size...
  case op_pd:  // Intel "forgot" to define this in book, but uses it
    return dqwordSzB;
  case op_pi:
    return qwordSzB;
  case op_ps:
    return dqwordSzB;
  case op_q:
    return qwordSzB;
  case op_s:
    return 6;
  case op_sd:  // another Intel amnesia case
    return qwordSzB;
  case op_ss:
    return dwordSzB;
  case op_si:
    assert(!"Where is this used, Intel?");
    return dwordSzB;
  case op_v:
    return wordSzB * operSzAttr;
  case op_w:
    return wordSzB;
  case op_z:
      return operSzAttr == 1 ? wordSzB : dwordSzB;
  case op_lea:
    //    assert(!"Should not be evaluated");
    // We might be called, if we don't know this is an lea ahead of time
    // It's okay to return 0 here, because we really don't load/store
    return 0;
  case op_allgprs:
    return 8 * wordSzB * operSzAttr;
  case op_512:
    return 512;
  default:
    assert(!"No such type");
    return -1;
  }
}


unsigned int ia32_decode_operands (const ia32_prefixes& pref, const ia32_entry& gotit, 
                                   const unsigned char* addr, ia32_instruction& instruct,
                                   ia32_memacc *mac)
{
  unsigned int nib = 0 /* # of bytes in instruction */;
  
  unsigned int addrSzAttr = (pref.getPrefix(3) == PREFIX_SZADDR ? 1 : 2);
  if (mode_64)
    addrSzAttr *= 2;

  unsigned int operSzAttr;
  if (pref.rexW()) operSzAttr = 4;
  else if (pref.getPrefix(2) == PREFIX_SZOPER) operSzAttr = 1;
  else operSzAttr = 2;

  if(gotit.hasModRM)
    nib += byteSzB;

  for(unsigned int i=0; i<3; ++i) {
    const ia32_operand& op = gotit.operands[i];
    if(op.admet) {
      // At most two operands can be memory, the third is register or immediate
      //assert(i<2 || op.admet == am_reg || op.admet == am_I);
      switch(op.admet) {
      case am_A: /* address = segment + offset (word or dword) */
        nib += wordSzB;
        if(mac)
          bperr( "IA32: segment selector ignored [am_A].\n");
      case am_O: /* operand offset */
        nib += wordSzB * addrSzAttr;
        if(mac) {
          int offset;
          assert(addrSzAttr < 3);
          if(addrSzAttr == 1) // 16-bit offset
            offset = *((const short int*)addr);
          else // 32-bit offset
            offset = *((const int*)addr);
          mac[i].set32(-1, offset);
          mac[i].size = type2size(op.optype, operSzAttr);
        }
        break;
      case am_C:   /* control register */
      case am_D:   /* debug register */
      case am_F:   /* flags register */
      case am_G:   /* general purpose register, selecteb by reg field */
      case am_P:   /* MMX register */
      case am_R:   /* general purpose register, selected by mod field */
      case am_S:   /* segment register */
      case am_T:   /* test register */
      case am_V:   /* XMM register */
      case am_reg: /* register implicitely encoded in opcode */
      case am_allgprs:
        break;
      case am_E: /* register or memory location, so decoding needed */
      case am_M: /* memory operand, decoding needed; size includes modRM byte */
      case am_Q: /* MMX register or memory location */
      case am_W: /* XMM register or memory location */
        if(mac) {
          nib += ia32_decode_modrm(addrSzAttr, addr, &mac[i]);
          mac[i].size = type2size(op.optype, operSzAttr);
        }
        else
          nib += ia32_decode_modrm(addrSzAttr, addr);

	// also need to check for AMD64 rip-relative data addressing
	// occurs when mod == 0 and r/m == 101
	if (mode_64)
	    if ((addr[0] & 0xc7) == 0x05)
		instruct.rip_relative_data = true;

        break;
      case am_I: /* immediate data */
      case am_J: /* instruction pointer offset */
        nib += type2size(op.optype, operSzAttr);
        break;
      /* TODO: rep prefixes, deal with them here? */
      case am_X: /* memory at DS:(E)SI*/
        if(mac)
          mac[i].setXY(mESI, type2size(op.optype, operSzAttr), addrSzAttr == 1);
        break;
      case am_Y: /* memory at ES:(E)DI*/
        if(mac)
          mac[i].setXY(mEDI, type2size(op.optype, operSzAttr), addrSzAttr == 1);
        break;
      case am_stackH: /* stack push */
        if(mac) {
          // assuming 32-bit stack segment
          mac[i].set32(mESP, -2 * operSzAttr);
          mac[i].size = type2size(op.optype, operSzAttr);
        }
        break;
      case am_stackP: /* stack pop */
        if(mac) {
          // assuming 32-bit stack segment
          mac[i].set32(mESP, 0);
          mac[i].size = type2size(op.optype, operSzAttr);
        }
        break;
      default:
        assert(0);
      }
    }
    else
      break;
  }
  instruct.size += nib;
  return nib;
}


static const unsigned char sse_prefix[256] = {
  /*       0 1 2 3 4 5 6 7 8 9 A B C D E F  */
  /* 0x */ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  /* 1x */ 1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,
  /* 2x */ 0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,
  /* 3x */ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  /* 4x */ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  /* 5x */ 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
  /* 6x */ 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
  /* 7x */ 1,1,1,1,1,1,1,0,0,0,0,0,0,0,1,1, // Grp12-14 are SSE groups
  /* 8x */ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  /* 9x */ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  /* Ax */ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  /* Bx */ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  /* Cx */ 0,0,1,0,1,1,1,0,0,0,0,0,0,0,0,0,
  /* Dx */ 0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
  /* Ex */ 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
  /* Fx */ 0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0
};


// FIXME: lookahead might blow up...
bool ia32_decode_prefixes(const unsigned char* addr, ia32_prefixes& pref)
{
  pref.count = 0;
  pref.prfx[0] = pref.prfx[1] = pref.prfx[2] = pref.prfx[3] = pref.prfx[4] = 0;
  pref.opcode_prefix = 0;
  bool in_prefix = true;

  while(in_prefix) {
    switch(addr[0]) {
    case PREFIX_REPNZ:
    case PREFIX_REP:
	if(addr[1]==0x0F && sse_prefix[addr[2]]) {
	    pref.opcode_prefix = addr[0];
	    break;
	}
    case PREFIX_LOCK:
      ++pref.count;
      pref.prfx[0] = addr[0];
      break;
    case PREFIX_SEGCS:
    case PREFIX_SEGSS:
    case PREFIX_SEGDS:
    case PREFIX_SEGES:
    case PREFIX_SEGFS:
    case PREFIX_SEGGS:
      ++pref.count;
      pref.prfx[1] = addr[0];
      break;
    case PREFIX_SZOPER:
	if(addr[1]==0x0F && sse_prefix[addr[2]]) {
	    pref.opcode_prefix = addr[0];
	    break;
	}
	++pref.count;
	pref.prfx[2] = addr[0];
      break;
    case PREFIX_SZADDR:
      ++pref.count;
      pref.prfx[3] = addr[0];
      break;
    default:
      in_prefix=false;
    }
    ++addr;
  }
  
  if (mode_64)
      return ia32_decode_rex(addr - 1, pref);

  return true;
}

bool ia32_decode_rex(const unsigned char* addr, ia32_prefixes& pref)
{
    if ((addr[0] & (unsigned char)0xF0) == (unsigned char)0x40) {
	++pref.count;
	pref.prfx[4] = addr[0];

	// it is an error to have legacy prefixes after a REX prefix
	// in particular, ia32_decode will get confused if a prefix
	// that could be used as an SSE opcode extension follows our
	// REX
	if (addr[1] == PREFIX_SZOPER || addr[1] == PREFIX_REPNZ || addr[1] == PREFIX_REP)
	  return false;
    }

    return true;
}

unsigned int ia32_emulate_old_type(ia32_instruction& instruct)
{
  const ia32_prefixes& pref = instruct.prf;
  unsigned int& insnType = instruct.legacy_type;
  unsigned int operSzAttr = (pref.getPrefix(2) == PREFIX_SZOPER ? 1 : 2); // 32-bit mode implicit

  if (pref.getPrefix(0)) // no distinction between these
    insnType |= PREFIX_INST;
  if (pref.getPrefix(3) == PREFIX_SZADDR)
    insnType |= PREFIX_ADDR;
  if (pref.getPrefix(2) == PREFIX_SZOPER)
    insnType |= PREFIX_OPR;
  if (pref.getPrefix(1))
    insnType |= PREFIX_SEG; // no distinction between segments
  if (mode_64 && pref.getPrefix(4))
    insnType |= PREFIX_REX;
  if (pref.getOpcodePrefix())
      insnType |= PREFIX_OPCODE;

  // this still works for AMD64, since there is no "REL_Q"
  // actually, it will break if there is both a REX.W and operand
  // size prefix, since REX.W takes precedence (FIXME)
  if (insnType & REL_X) {
    if (operSzAttr == 1)
      insnType |= REL_W;
    else
      insnType |= REL_D;
  }
  else if (insnType & PTR_WX) {
    if (operSzAttr == 1)
      insnType |= PTR_WW;
    else
      insnType |= PTR_WD;
  }

  // AMD64 rip-relative data
  if (instruct.hasRipRelativeData())
      insnType |= REL_D_DATA;

  return insnType;
}

/* decode instruction at address addr, return size of instruction */
unsigned get_instruction(const unsigned char* addr, unsigned &insnType,
			 const unsigned char** op_ptr)
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
  if (op_ptr)
      *op_ptr = addr + i.getPrefixCount();

  return r1;
}

long addressOfMachineInsn(instruction *insn) {
  return (long)insn->ptr();
}

int sizeOfMachineInsn(instruction *insn) {
  return insn->size();
}

// find the target of a jump or call
Address get_target(const unsigned char *instr, unsigned type, unsigned size,
		   Address addr) {
  int disp = displacement(instr, type);
  return (Address)(addr + size + disp);
}

// get the displacement of a jump or call
int displacement(const unsigned char *instr, unsigned type) {

  int disp = 0;
  //skip prefix
  instr = skip_headers( instr );
  
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
skip_headers(const unsigned char* addr, ia32_prefixes* prefs)
{
  ia32_prefixes default_prefs;
  if (prefs == NULL)
      prefs = &default_prefs;
  ia32_decode_prefixes(addr, *prefs);
  return addr + prefs->getCount();
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

// haven't made this ready for 64-bit yet - gq
bool isFunctionPrologue( instruction& insn1 )
{
    if( insn1.size() != 1 )
        return false;
    if( isStackFramePreamble( insn1 ) )
        return true;

    instruction insn2, insn3;
    insn2.setInstruction( insn1.ptr() + insn1.size() );       
    insn3.setInstruction( insn2.ptr() + insn2.size() );
    
    const unsigned char* p = insn1.ptr();
    const unsigned char* q = insn2.ptr();
    const unsigned char* r = insn3.ptr();
    
    unsigned Mod1 =  ( q[ 1 ] >> 3 ) & 0x07;
    unsigned Mod2 =  ( r[ 1 ] >> 3 ) & 0x07;  
    
    //Things fall apart; the center cannot hold; Mere anarchy is loosed upon 
    //the world...
    //[W.B. Yeats]
    
    //<centre>
    //look for
    //1. push esi
    //   move esi, xxx
    //
    //2. push esi
    //   xor  esi, esi
    if( p[ 0 ] == PUSHESI )
    {   
        if( insn2.isMoveRegMemToRegMem() && Mod1 == 0x06 )
            return true;
          
        if( insn3.isMoveRegMemToRegMem() && Mod2 == 0x06 )
            return true;
        
        if( insn2.isXORRegMemRegMem() && Mod1 == 0x06 )
            return true;
        
        if( insn3.isXORRegMemRegMem() && Mod2 == 0x06 )
            return true;
    }
    //</centre>
    return false;
}

bool isStackFramePreamble( instruction& insn1 )
{       
    instruction insn2, insn3;
    insn2.setInstruction( insn1.ptr() + insn1.size() );       
    insn3.setInstruction( insn2.ptr() + insn2.size() );

    const unsigned char* p = insn1.op_ptr();
    const unsigned char* q = insn2.op_ptr();
    const unsigned char* r = insn3.op_ptr();
    
    unsigned Mod1 =  ( q[ 1 ] >> 3 ) & 0x07;
    unsigned Mod2 =  ( r[ 1 ] >> 3 ) & 0x07;  

    if( insn1.size() != 1 )
    {
        return false;  //shouldn't need this, but you never know
    }
    
    if( p[ 0 ] == PUSHEBP  )
    {   
        if( insn2.isMoveRegMemToRegMem() && Mod1 == 0x05 )
            return true;
          
        if( insn3.isMoveRegMemToRegMem() && Mod2 == 0x05 )
            return true;
    }
    
    return false;
}

int set_disp(bool setDisp, instruction *insn, int newOffset, bool outOfFunc) {

  unsigned char *instr = (const_cast<unsigned char *> (insn->ptr()));
  unsigned type = insn->type();

  if (!((type & IS_JUMP) || (type & IS_JCC) || (type & IS_CALL))) return 0; 

  instr = const_cast<unsigned char *>(skip_headers(instr));

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

// This function decodes the addressing modes used for call instructions
// For documentation on these mode, and how to decode them, see tables
// 2-2 and 2-3 from the intel software developers manual.
//     02/05 GQ: modified to talk with newer decoding tables
int get_instruction_operand(const unsigned char *ptr, Register& base_reg,
			    Register& index_reg, int& displacement, 
			    unsigned& scale, unsigned &Mod){
  ia32_entry *desc;
  
  unsigned ModRMbyte;
  unsigned RM=0;
  unsigned REG;
  unsigned SIBbyte = 0;
  bool hasSIB;
  ia32_prefixes prefs;

  // Initialize default values to an appropriately devilish number 
  base_reg = 666;
  index_reg = 666;
  displacement = 0;
  scale = 0;
  Mod = 0;
  
  ptr = skip_headers(ptr, &prefs);  
  desc = &oneByteMap[*ptr++];

  if (desc->tabidx != Grp5) {
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

    if(REG == 2){ // call to 32-bit near pointer
      if (Mod == 3){ //Standard call where address is in register
	base_reg = (Register) RM;
	if (prefs.rexB()) base_reg += 8;
	return REGISTER_DIRECT;
      }
      else if (Mod == 2){
	displacement = *( ( const unsigned int * ) ptr );
	ptr+= wordSzB;
	if(hasSIB){
	  decode_SIB(SIBbyte, scale, index_reg, base_reg);
	  if (prefs.rexB()) base_reg += 8;
	  if (prefs.rexX()) index_reg += 8;
	  return SIB;
	}	
	else {
	  base_reg = (Register) RM;
	  if (prefs.rexB()) base_reg += 8;
	  return REGISTER_INDIRECT_DISPLACED; 
	}
      }
      else if(Mod == 1){//call with 8-bit signed displacement
	displacement = *( const char * )ptr;
	ptr+= byteSzB;
	if(hasSIB){
	  decode_SIB(SIBbyte, scale, index_reg, base_reg);
	  if (prefs.rexB()) base_reg += 8;
	  if (prefs.rexX()) index_reg += 8;
	  return SIB;
	}
	else {
	  base_reg = (Register) RM;
	  if (prefs.rexB()) base_reg += 8;
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
	  if (prefs.rexB()) base_reg += 8;
	  if (prefs.rexX()) index_reg += 8;
	  return SIB;
	}
	else if(RM == 5){ //The disp32 field from Table 2-2 of IA guide
	  displacement = *( ( const unsigned int * ) ptr );
	  ptr+= wordSzB;
	  return DISPLACED;
	}
	else {
	  base_reg = (Register) RM;
	  if (prefs.rexB()) base_reg += 8;
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

// We keep an array-let that represents various fixed
// insns
unsigned char illegalRep[2] = {0x0f, 0x0b};
unsigned char trapRep[1] = {0xCC};

instruction *instruction::copy() const {
    // Or should we copy? I guess it depends on who allocated
    // the memory...
    return new instruction(*this);
}

void instruction::generateIllegal(codeGen &gen) {
    instruction insn;
    insn.setInstruction(illegalRep);
    insn.generate(gen);
}

void instruction::generateTrap(codeGen &gen) {
    instruction insn;
    insn.setInstruction(trapRep);
    insn.generate(gen);
}


    
/*
 * change the insn at addr to be a branch to newAddr.
 *   Used to add multiple tramps to a point.
 */

void instruction::generateBranch(codeGen &gen,
                                 Address fromAddr, Address toAddr) {

  long disp = toAddr - (fromAddr + JUMP_REL32_SZ);

#if defined(arch_x86_64)
  if (!is_disp32(disp)) {

    // need to use an indirect jmp
    GET_PTR(insn, gen);
    for (int i = 3; i >= 0; i--) {
      short word = (toAddr >> (16 * i)) & 0xffff;
      *insn++ = 0x66; // operand size override
      *insn++ = 0x68; // push immediate (16-bits w/ prefix)
      *(short *)insn = word;
      insn += 2;
    }
    *insn++ = 0xC3; // RET
    SET_PTR(insn, gen);
    return;
  }
#endif
  
  generateBranch(gen, disp);
}

void instruction::generateBranch(codeGen &gen,
                                 int disp32) {
    if (disp32 >= 0)
        assert ((unsigned)disp32 < unsigned(1<<31));
    else
        assert ((unsigned)(-disp32) < unsigned(1<<31));
    
    GET_PTR(insn, gen);
    *insn++ = 0xE9;
    *((int *)insn) = disp32;
    insn += sizeof(int);
    SET_PTR(insn, gen);
    return;
}

void instruction::generateCall(codeGen &gen,
                               Address from,
                               Address target) {
    // TODO 64bit fixme
    int disp = target - (from + CALL_REL32_SZ);
    GET_PTR(insn, gen);
    *insn++ = 0xE8;
    *((int *)insn) = disp;
    insn += sizeof(int);
    SET_PTR(insn, gen);
}

void instruction::generateNOOP(codeGen &gen, unsigned size) {
    // Be more efficient here...
    while (size) {
        GET_PTR(insn, gen);
        *insn++ = NOP;
        SET_PTR(insn, gen);
        size -= sizeof(unsigned char);
    }
}

void instruction::generate(codeGen &gen) {
    assert(ptr_);
    assert(size_);
    memcpy(gen.cur_ptr(), ptr_, size_);
    gen.moveIndex(size_);
}

unsigned instruction::spaceToRelocate() const {
    // List of instructions that might be painful:
    // jumps (displacement will change)
    // call (displacement will change)
    // PC-relative ops

    // TODO: pc-relative ops

   const unsigned char *origInsn = ptr();

   unsigned maxSize = 0;
   unsigned nPrefixes = 0;


   if ((type() && REL_B) ||
       (type() && REL_W) ||
       (type() && REL_D)) {
       if (type() & PREFIX_OPR)
           nPrefixes++;
       if (type() & PREFIX_SEG)
           nPrefixes++;
       if (type() & PREFIX_OPCODE)
           nPrefixes++;
       if (type() & PREFIX_REX)
           nPrefixes++;
       if (type() & PREFIX_INST)
           nPrefixes++;
       if (type() & PREFIX_ADDR)
           nPrefixes++;
   }

   // Skip prefixes or none of the later matches will work.
   for (unsigned u = 0; u < nPrefixes; u++)
       *origInsn++;

   inst_printf("%d bytes in prefixes\n", nPrefixes);

   if (type() & REL_B) {
       /* replace with rel32 instruction, opcode is one byte. */
       if (*origInsn == JCXZ) {
           // Replace with:
           // jcxz 2    2 bytes
           //  jmp 5    2 bytes
           // jmp rel32 5 bytes
           
           return 9 + nPrefixes;
      }
      else {
          if (type() & IS_JCC) {
              /* Change a Jcc rel8 to Jcc rel32.  Must generate a new opcode: a
                 0x0F followed by (old opcode + 16) */
              inst_printf("... JCC, returning %d\n", 6+nPrefixes);
              return 6 + nPrefixes;
          }
          else if (type() & IS_JUMP) {
              /* change opcode to 0xE9 */
              inst_printf(".... JUMP, returning %d\n", 5+nPrefixes);
              return 5 + nPrefixes;
          }
          assert(0);
      }
   }
   else if (type() & REL_W) {
       inst_printf("... REL_W\n");
      /* opcode is unchanged, just relocate the displacement */
      if (*origInsn == (unsigned char)0x0F)
          maxSize++;

      // opcode, 32-bit displacement
      maxSize += 5;

      return maxSize + nPrefixes;
   } else if (type() & REL_D) {
       inst_printf("... REL_D\n");
       /* opcode is unchanged, just relocate the displacement */
       if (*origInsn == 0x0F)
           maxSize++;
       
       maxSize += 5;
       return maxSize + nPrefixes;
   }
   else {
       return size();
   }
   assert(0);
   return 0;
}

bool instruction::generate(codeGen &gen,
                           process *proc,
                           Address origAddr, // Could be kept in the instruction class..
                           Address newAddr,
                           Address /*fallthroughOverride*/,
                           Address targetOverride) {
    // We grab the maximum space we might need
    GET_PTR(insnBuf, gen);
    
    /* 
       Relative address instructions need to be modified. The relative address
       can be a 8, 16, or 32-byte displacement relative to the next instruction.
       Since we are relocating the instruction to a different area, we have
       to replace 8 and 16-byte displacements with 32-byte displacements.
       
       All relative address instructions are one or two-byte opcode followed
       by a displacement relative to the next instruction:
       
       CALL rel16 / CALL rel32
       Jcc rel8 / Jcc rel16 / Jcc rel32
       JMP rel8 / JMP rel16 / JMP rel32
       
       The only two-byte opcode instructions are the Jcc rel16/rel32,
       all others have one byte opcode.
       
       The instruction JCXZ/JECXZ rel8 does not have an equivalent with rel32
       displacement. We must generate code to emulate this instruction:
       
       JCXZ rel8
       
       becomes
       
       A0: JCXZ 2 (jump to A4)
       A2: JMP 5  (jump to A9)
       A4: JMP rel32 (relocated displacement)
       A9: ...
       
   */

    const unsigned char *origInsn = ptr();
    unsigned insnType = type();
    unsigned insnSz = size();
    // This moves as we emit code
    unsigned char *newInsn = insnBuf;

    int oldDisp;
    int newDisp;

    bool done = false;
    
    // Check to see if we're doing the "get my PC" via a call
    // We do this first as those aren't "real" jumps.
    if (isCall() && !isCallIndir()) {
        // A possibility...
        // Two types: call(0) (AKA call(me+5));
        // or call to a move/return combo.

        // First, call(me)
        Address target = getTarget(origAddr);
        // Big if tree: "if we go to the next insn"
        // Also could do with an instrucIter... or even have
        // parsing label it for us. 
        // TODO: label in parsing (once)
        
        if (target == (origAddr + size())) {
            *newInsn = 0x68; // What opcode is this?
            newInsn++;

            Address EIP = origAddr + size();
            unsigned int *temp = (unsigned int *) newInsn;
            *temp = EIP;
            // No 9-byte jumps...
            assert(sizeof(unsigned int *) == 4);
            newInsn += sizeof(unsigned int *);
            assert((newInsn - insnBuf) == 5);
            done = true;
        }
        else {
            // Get us an instrucIter
            InstrucIter callTarget(target, proc);
            instruction firstInsn = callTarget.getInstruction();
            instruction secondInsn = callTarget.getNextInstruction();
            if (firstInsn.isMoveRegMemToRegMem() &&
                secondInsn.isReturn()) {
                // We need to fake this by figuring out the register
                // target (assuming we're moving stack->reg),
                // and constructing an immediate with the value of the
                // original address of the call (+size)
                // This was copied from function relocation code... I 
                // don't understand most of it -- bernat
                const unsigned char *ptr = firstInsn.ptr();
                unsigned char modrm = *(ptr + 1);
                unsigned char reg = (modrm >> 3) & 0x3;
                // Source register... 
                if ((modrm == 0x0c) || (modrm == 0x1c)) {
                    // Check source register (%esp == 0x24)
                    if ((*(ptr + 2) == 0x24)) {
                        // Okay, put the PC into the 'reg'
                        Address EIP = origAddr + size();
                        *newInsn = 0xb8 + reg; // Opcode???
                        newInsn++;
                        unsigned int *temp = (unsigned int *)newInsn;
                        *temp = EIP;
                        //assert(sizeof(unsigned int *)==4);
                        //newInsn += sizeof(unsigned int *);
                        newInsn += 4;  // fix for AMD64
                        done = true;
                    }
                }
            }
        }
    }
    if (!done) {
        // If call-specialization didn't take care of us
        if (insnType & REL_B) {
            // Skip prefixes
            unsigned nPrefixes = 0;
            if (insnType & PREFIX_OPR)
                nPrefixes++;
            if (insnType & PREFIX_SEG)
                nPrefixes++;
            if (insnType & PREFIX_OPCODE)
               nPrefixes++;
            if (insnType & PREFIX_REX)
                nPrefixes++;
            if (insnType & PREFIX_INST)
                nPrefixes++;
            if (insnType & PREFIX_ADDR)
                nPrefixes++;
            inst_printf(".... %d bytes in prefixes\n", nPrefixes);
            for (unsigned u = 0; u < nPrefixes; u++)
                *newInsn++ = *origInsn++;


            /* replace with rel32 instruction, opcode is one byte. */
            if (*origInsn == JCXZ) {
                if (targetOverride == 0) {
                    oldDisp = (int)*(const char *)(origInsn+1);
                    newDisp = (origAddr + size()) // end of instruction...
                        + oldDisp - (newAddr + 9);
                }
                else {
                    // Override the target to go somewhere else
                    oldDisp = 0;
                    newDisp = targetOverride - (newAddr + 9);
                }
                *newInsn++ = *origInsn; *(newInsn++) = 2; // jcxz 2
                *newInsn++ = 0xEB; *newInsn++ = 5;        // jmp 5
                *newInsn++ = 0xE9;                        // jmp rel32
                *((int *)newInsn) = newDisp;
                newInsn += sizeof(int);
            }
            else {
                unsigned newSz=UINT_MAX;
                if (insnType & IS_JCC) {
                    /* Change a Jcc rel8 to Jcc rel32.  Must generate a new opcode: a
                       0x0F followed by (old opcode + 16) */
                    unsigned char opcode = *origInsn++;
                    *newInsn++ = 0x0F;
                    *newInsn++ = opcode + 0x10;
                    newSz = 6 + nPrefixes;
                }
                else if (insnType & IS_JUMP) {
                    /* change opcode to 0xE9 */
                    origInsn++;
                    *newInsn++ = 0xE9;
                    newSz = 5 + nPrefixes;
                }
                assert(newSz!=UINT_MAX);
                if (targetOverride == 0) {
                    oldDisp = (int)*(const char *)origInsn;
                    newDisp = (origAddr + size()) + oldDisp - (newAddr + newSz);
                }
                else {
                    oldDisp = 0;
                    newDisp = targetOverride - (newAddr + newSz);
                }
                *((int *)newInsn) = newDisp;
                newInsn += sizeof(int);
            }
        }
        else if (insnType & REL_W) {
            // Skip prefixes
            unsigned nPrefixes = 0;
            if (insnType & PREFIX_OPR)
                nPrefixes++;
            if (insnType & PREFIX_SEG)
                nPrefixes++;
            if (insnType & PREFIX_OPCODE)
               nPrefixes++;
            if (insnType & PREFIX_REX)
                nPrefixes++;
            if (insnType & PREFIX_INST)
                nPrefixes++;
            if (insnType & PREFIX_ADDR)
                nPrefixes++;

            for (unsigned u = 0; u < nPrefixes; u++)
                *newInsn++ = *origInsn++;

            /* opcode is unchanged, just relocate the displacement */
            if (*origInsn == (unsigned char)0x0F)
                *newInsn++ = *origInsn++;
            *newInsn++ = *origInsn++;
            if (targetOverride == 0) {
                oldDisp = *((const short *)origInsn);
                newDisp = (origAddr + 5) + oldDisp - (newAddr + 3);
            }
            else {
                oldDisp = 0;
                newDisp = targetOverride -  (newAddr + 3);
            }
            *((int *)newInsn) = newDisp;
            newInsn += sizeof(int);
        } else if (insnType & REL_D || insnType & REL_D_DATA) {

            // Skip prefixes
            unsigned nPrefixes = 0;
            if (insnType & PREFIX_OPR)
                nPrefixes++;
            if (insnType & PREFIX_SEG)
                nPrefixes++;
            if (insnType & PREFIX_OPCODE)
               nPrefixes++;
            if (insnType & PREFIX_REX)
                nPrefixes++;
            if (insnType & PREFIX_INST)
                nPrefixes++;
            if (insnType & PREFIX_ADDR)
                nPrefixes++;

            for (unsigned u = 0; u < nPrefixes; u++)
                *newInsn++ = *origInsn++;
            
            /* opcode is unchanged, just relocate the displacement */
            if (*origInsn == 0x0F)
                *newInsn++ = *origInsn++;
            *newInsn++ = *origInsn++;

	    // skip over ModRM byte for RIP-relative data
	    if (insnType & REL_D_DATA)
		*newInsn++ = *origInsn++;

            if (targetOverride == 0) {
                oldDisp = *((const int *)origInsn);
                newDisp = (origAddr + insnSz) + oldDisp - (newAddr + insnSz);
            }
            else {
                oldDisp = 0;
                newDisp = targetOverride - (newAddr + insnSz);
            }
            *((int *)newInsn) = newDisp;
            newInsn += sizeof(int);

	    // copy the rest of the instruction for RIP-relative data (there may be an immediate)
	    // we can do this since the insn size will always be the same
	    if (insnType & REL_D_DATA) {
		origInsn += sizeof(int);
		while (newInsn - insnBuf < (int)insnSz)
		    *newInsn++ = *origInsn++;
	    }
        }
        else {
            /* instruction is unchanged */
            for (unsigned u = 0; u < insnSz; u++)
                *newInsn++ = *origInsn++;
        }
    }

    SET_PTR(newInsn, gen);
    return true;
}

int instruction::jumpSize(Address /*from*/, Address /*to*/) {

#if defined(arch_x86)
    return JUMP_REL32_SZ;
#else
    long disp = to - (from + JUMP_REL32_SZ);
    if (is_disp32(disp))
      return JUMP_REL32_SZ;
    else
      return JUMP_ABS64_SZ;
#endif
}

unsigned instruction::maxJumpSize() {
#if defined(arch_x86)
    return JUMP_REL32_SZ;
#else
    return JUMP_ABS64_SZ;
#endif
}
