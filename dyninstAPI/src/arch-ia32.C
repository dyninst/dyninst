// $Id: arch-ia32.C,v 1.3 2002/06/13 00:37:06 gaburici Exp $

// Official documentation used:    - IA-32 Intel Architecture Software Developer Manual
//                                   volume 2: Instruction Set Reference
// Unofficial documentation used:  - www.sandpile.org/ia32
//                                 - NASM documentation

#include <assert.h>
#include <stdio.h>
#include "common/h/Types.h"
#include "arch-ia32.h"

// tables and pseudotables
enum {
  t_ill=0, t_oneB, t_twoB, t_prefixedSSE, t_coprocEsc, t_grp, t_sse, t_grpsse, t_3dnow, t_done=99
};

#define oneB t_done, 0
#define twoB t_done, 0

// groups
enum {
  Grp1=0, Grp2, Grp3a, Grp3b, Grp4, Grp5, Grp6, Grp7, Grp8, Grp9,
  Grp11, Grp12, Grp13, Grp14, Grp15, Grp16, GrpAMD
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

// addressing methods (see appendix A-2)
enum { am_A=1, am_C, am_D, am_E, am_F, am_G, am_I, am_J, am_M, am_O,
       am_P, am_Q, am_R, am_S, am_T, am_V, am_W, am_X, am_Y, am_reg };

// operand types (idem, but I invented lea for consistency; op_ didn't seem a good idea)
// beware that not all operand types in the tables are explained!!! Guess what they mean...
enum { op_a=1, op_b, op_c, op_d, op_dq, op_p, op_pd, op_pi, op_ps, 
       op_q, op_s,  op_sd, op_ss, op_si, op_v, op_w, op_lea };

// registers [only fancy names, not used right now]
enum { r_AH=100, r_BH, r_CH, r_DH, r_AL, r_BL, r_CL, r_DL, 
       r_DX,
       r_eAX, r_eBX, r_eCX, r_eDX,
       r_EAX, r_EBX, r_ECX, r_EDX,
       r_DS, r_ES, r_FS, r_GS, r_SS,
       r_eSP, r_eBP, r_eSI, r_eDI,
       r_ESP, r_EBP, r_ESI, r_EDI };

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
#define Jb   { am_J, op_b }
#define Jv   { am_J, op_v }
#define Ma   { am_M, op_a }
#define Mlea { am_M, op_lea }
#define Mp   { am_M, op_p }
#define Ms   { am_M, op_s }
#define Mq   { am_M, op_q }
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

#define AH  { am_reg, r_AH }
#define BH  { am_reg, r_BH }
#define CH  { am_reg, r_CH }
#define DH  { am_reg, r_DH }
#define AL  { am_reg, r_AL }
#define BL  { am_reg, r_BL }
#define CL  { am_reg, r_CL }
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


struct ia32_operand {
  unsigned int admet;  // addressing method
  unsigned int optype; // operand type;
};

// An instruction table entry
struct ia32_entry {
  char *name;                // name of the instruction (for debbuging only)
  unsigned int otable;       // which opcode table is next; if t_done it is the current one
  unsigned char tabidx;      // at what index to look, 0 if it easy to deduce from opcode
  bool hasModRM;             // true if the instruction has a MOD/RM byte
  ia32_operand operands[3];  // operand descriptors
  unsigned legacyType;       // legacy type of the instruction (e.g. (IS_CALL | REL_W))
};


// oneByteMap: one byte opcode map
static ia32_entry oneByteMap[256] = {
  /* 00 */
  { "add",  t_done, 0, true, { Eb, Gb, Zz }, 0 },
  { "add",  t_done, 0, true, { Ev, Gv, Zz }, 0 },
  { "add",  t_done, 0, true, { Gb, Eb, Zz }, 0 },
  { "add",  t_done, 0, true, { Gv, Ev, Zz }, 0 },
  { "add",  t_done, 0, false, { AL, Ib, Zz }, 0 },
  { "add",  t_done, 0, false, { eAX, Iv, Zz }, 0 },
  { "push", t_done, 0, false, { ES, Zz, Zz }, 0 },
  { "pop",  t_done, 0, false, { ES, Zz, Zz }, 0 },
  /* 08 */
  { "or",   t_done, 0, true, { Eb, Gb, Zz }, 0 },
  { "or",   t_done, 0, true, { Ev, Gv, Zz }, 0 },
  { "or",   t_done, 0, true, { Gb, Eb, Zz }, 0 },
  { "or",   t_done, 0, true, { Gv, Ev, Zz }, 0 },
  { "or",   t_done, 0, false, { AL, Ib, Zz }, 0 },
  { "or",   t_done, 0, false, { eAX, Iv, Zz }, 0 },
  { "push", t_done, 0, false, { ES, Zz, Zz }, 0 },
  { 0,      t_twoB, 0, false, { Zz, Zz, Zz }, 0 },
  /* 10 */
  { "adc",  t_done, 0, true, { Eb, Gb, Zz }, 0 },
  { "adc",  t_done, 0, true, { Ev, Gv, Zz }, 0 },
  { "adc",  t_done, 0, true, { Gb, Eb, Zz }, 0 },
  { "adc",  t_done, 0, true, { Gv, Ev, Zz }, 0 },
  { "adc",  t_done, 0, false, { AL, Ib, Zz }, 0 },
  { "adc",  t_done, 0, false, { eAX, Iv, Zz }, 0 },
  { "push", t_done, 0, false, { SS, Zz, Zz }, 0 },
  { "pop",  t_done, 0, false, { SS, Zz, Zz }, 0 },
  /* 18 */
  { "sbb",  t_done, 0, true, { Eb, Gb, Zz }, 0 },
  { "sbb",  t_done, 0, true, { Ev, Gv, Zz }, 0 },
  { "sbb",  t_done, 0, true, { Gb, Eb, Zz }, 0 },
  { "sbb",  t_done, 0, true, { Gv, Ev, Zz }, 0 },
  { "sbb",  t_done, 0, false, { AL, Ib, Zz }, 0 },
  { "sbb",  t_done, 0, false, { eAX, Iv, Zz }, 0 },
  { "push", t_done, 0, false, { DS, Zz, Zz }, 0 },
  { "pop" , t_done, 0, false, { DS, Zz, Zz }, 0 },
  /* 20 */
  { "and", t_done, 0, true, { Eb, Gb, Zz }, 0 },
  { "and", t_done, 0, true, { Ev, Gv, Zz }, 0 },
  { "and", t_done, 0, true, { Gb, Eb, Zz }, 0 },
  { "and", t_done, 0, true, { Gv, Ev, Zz }, 0 },
  { "and", t_done, 0, false, { AL, Ib, Zz }, 0 },
  { "and", t_done, 0, false, { eAX, Iv, Zz }, 0 },
  { 0, t_ill, 0, false, { Zz, Zz, Zz }, 0 }, // PREFIX_SEG_OVR
  { "daa", t_done, 0, false, { Zz, Zz, Zz }, 0 },
  /* 28 */
  { "sub", t_done, 0, true, { Eb, Gb, Zz }, 0 },
  { "sub", t_done, 0, true, { Ev, Gv, Zz }, 0 },
  { "sub", t_done, 0, true, { Gb, Eb, Zz }, 0 },
  { "sub", t_done, 0, true, { Gv, Ev, Zz }, 0 },
  { "sub", t_done, 0, false, { AL, Ib, Zz }, 0 },
  { "sub", t_done, 0, false, { eAX, Iv, Zz }, 0 },
  { 0, t_ill, 0, false, { Zz, Zz, Zz }, 0 }, // PREFIX_SEG_OVR
  { "das" , t_done, 0, false, { Zz, Zz, Zz }, 0 },
  /* 30 */
  { "xor", t_done, 0, true, { Eb, Gb, Zz }, 0 },
  { "xor", t_done, 0, true, { Ev, Gv, Zz }, 0 },
  { "xor", t_done, 0, true, { Gb, Eb, Zz }, 0 },
  { "xor", t_done, 0, true, { Gv, Ev, Zz }, 0 },
  { "xor", t_done, 0, false, { AL, Ib, Zz }, 0 },
  { "xor", t_done, 0, false, { eAX, Iv, Zz }, 0 },
  { 0, t_ill, 0, false, { Zz, Zz, Zz }, 0 }, // PREFIX_SEG_OVR
  { "aaa", t_done, 0, false, { Zz, Zz, Zz }, 0 },
  /* 38 */
  { "cmp", t_done, 0, true, { Eb, Gb, Zz }, 0 },
  { "cmp", t_done, 0, true, { Ev, Gv, Zz }, 0 },
  { "cmp", t_done, 0, true, { Gb, Eb, Zz }, 0 },
  { "cmp", t_done, 0, true, { Gv, Ev, Zz }, 0 },
  { "cmp", t_done, 0, false, { AL, Ib, Zz }, 0 },
  { "cmp", t_done, 0, false, { eAX, Iv, Zz }, 0 },
  { 0,     t_ill,  0, false, { Zz, Zz, Zz }, 0 }, // PREFIX_SEG_OVR
  { "aas", t_done, 0, false, { Zz, Zz, Zz }, 0 },
  /* 40 */
  { "inc", t_done, 0, false, { eAX, Zz, Zz }, 0 }, 
  { "inc", t_done, 0, false, { eCX, Zz, Zz }, 0 },
  { "inc", t_done, 0, false, { eDX, Zz, Zz }, 0 },
  { "inc", t_done, 0, false, { eBX, Zz, Zz }, 0 },
  { "inc", t_done, 0, false, { eSP, Zz, Zz }, 0 },
  { "inc", t_done, 0, false, { eBP, Zz, Zz }, 0 },
  { "inc", t_done, 0, false, { eSI, Zz, Zz }, 0 },
  { "inc", t_done, 0, false, { eDI, Zz, Zz }, 0 },
  /* 48 */
  { "dec", t_done, 0, false, { eAX, Zz, Zz }, 0 }, 
  { "dec", t_done, 0, false, { eCX, Zz, Zz }, 0 },
  { "dec", t_done, 0, false, { eDX, Zz, Zz }, 0 },
  { "dec", t_done, 0, false, { eBX, Zz, Zz }, 0 },
  { "dec", t_done, 0, false, { eSP, Zz, Zz }, 0 },
  { "dec", t_done, 0, false, { eBP, Zz, Zz }, 0 },
  { "dec", t_done, 0, false, { eSI, Zz, Zz }, 0 },
  { "dec", t_done, 0, false, { eDI, Zz, Zz }, 0 },
  /* 50 */
  { "push", t_done, 0, false, { eAX, Zz, Zz }, 0 }, 
  { "push", t_done, 0, false, { eCX, Zz, Zz }, 0 },
  { "push", t_done, 0, false, { eDX, Zz, Zz }, 0 },
  { "push", t_done, 0, false, { eBX, Zz, Zz }, 0 },
  { "push", t_done, 0, false, { eSP, Zz, Zz }, 0 },
  { "push", t_done, 0, false, { eBP, Zz, Zz }, 0 },
  { "push", t_done, 0, false, { eSI, Zz, Zz }, 0 },
  { "push", t_done, 0, false, { eDI, Zz, Zz }, 0 },
  /* 58 */
  { "pop", t_done, 0, false, { eAX, Zz, Zz }, 0 }, 
  { "pop", t_done, 0, false, { eCX, Zz, Zz }, 0 },
  { "pop", t_done, 0, false, { eDX, Zz, Zz }, 0 },
  { "pop", t_done, 0, false, { eBX, Zz, Zz }, 0 },
  { "pop", t_done, 0, false, { eSP, Zz, Zz }, 0 },
  { "pop", t_done, 0, false, { eBP, Zz, Zz }, 0 },
  { "pop", t_done, 0, false, { eSI, Zz, Zz }, 0 },
  { "pop", t_done, 0, false, { eDI, Zz, Zz }, 0 },
  /* 60 */
  { "pusha(d)", t_done, 0, false, { Zz, Zz, Zz }, 0 }, 
  { "popa(d)",  t_done, 0, false, { Zz, Zz, Zz }, 0 },
  { "bound",    t_done, 0, true, { Gv, Ma, Zz }, 0 },
  { "arpl",     t_done, 0, true, { Ew, Gw, Zz }, 0 },
  { 0,          t_ill,  0, false, { Zz, Zz, Zz }, 0 }, // PREFIX_SEG_OVR
  { 0,          t_ill,  0, false, { Zz, Zz, Zz }, 0 }, // PREFIX_SEG_OVR
  { 0,   t_prefixedSSE, 2, false, { Zz, Zz, Zz }, 0 }, /* operand size prefix (PREFIX_OPR_SZ)*/
  { 0,          t_ill,  0, false, { Zz, Zz, Zz }, 0 }, /* address size prefix (PREFIX_ADDR_SZ)*/
  /* 68 */
  { "push",    t_done, 0, false, { Iv, Zz, Zz }, 0 },
  { "imul",    t_done, 0, true, { Gv, Ev, Iv }, 0 },
  { "push",    t_done, 0, false, { Ib, Zz, Zz }, 0 },
  { "imul",    t_done, 0, true, { Gv, Ev, Ib }, 0 },
  { "insb",    t_done, 0, false, { Yb, DX, Zz }, 0 },
  { "insw/d",  t_done, 0, false, { Yv, DX, Zz }, 0 },
  { "outsb",   t_done, 0, false, { DX, Xb, Zz }, 0 },
  { "outsw/d", t_done, 0, false, { DX, Xv, Zz }, 0 },
  /* 70 */
  { "jo",         t_done, 0, false, { Jb, Zz, Zz }, (IS_JCC | REL_B) },
  { "jno",        t_done, 0, false, { Jb, Zz, Zz }, (IS_JCC | REL_B) },
  { "jb/jnaej/j", t_done, 0, false, { Jb, Zz, Zz }, (IS_JCC | REL_B) },
  { "jnb/jae/j",  t_done, 0, false, { Jb, Zz, Zz }, (IS_JCC | REL_B) },
  { "jz",         t_done, 0, false, { Jb, Zz, Zz }, (IS_JCC | REL_B) },
  { "jnz",        t_done, 0, false, { Jb, Zz, Zz }, (IS_JCC | REL_B) },
  { "jbe",        t_done, 0, false, { Jb, Zz, Zz }, (IS_JCC | REL_B) },
  { "jnbe",       t_done, 0, false, { Jb, Zz, Zz }, (IS_JCC | REL_B) },
  /* 78 */
  { "js",   t_done, 0, false, { Jb, Zz, Zz }, (IS_JCC | REL_B) },
  { "jns",  t_done, 0, false, { Jb, Zz, Zz }, (IS_JCC | REL_B) },
  { "jp",   t_done, 0, false, { Jb, Zz, Zz }, (IS_JCC | REL_B) },
  { "jnp",  t_done, 0, false, { Jb, Zz, Zz }, (IS_JCC | REL_B) },
  { "jl",   t_done, 0, false, { Jb, Zz, Zz }, (IS_JCC | REL_B) },
  { "jnl",  t_done, 0, false, { Jb, Zz, Zz }, (IS_JCC | REL_B) },
  { "jle",  t_done, 0, false, { Jb, Zz, Zz }, (IS_JCC | REL_B) },
  { "jnle", t_done, 0, false, { Jb, Zz, Zz }, (IS_JCC | REL_B) },
  /* 80 */
  { 0, t_grp, Grp1, true, { Eb, Ib, Zz }, 0 },
  { 0, t_grp, Grp1, true, { Ev, Iv, Zz }, 0 },
  { 0, t_grp, Grp1, true, { Eb, Ib, Zz }, 0 }, // this was Ill in the old decoder and in gdb 5.2.
                                               // the book says Grp1 however;sandpile.org agrees.
  { 0, t_grp, Grp1, true, { Ev, Ib, Zz }, 0 },
  { "test", t_done, 0, true, { Eb, Gb, Zz }, 0 },
  { "test", t_done, 0, true, { Ev, Gv, Zz }, 0 },
  { "xchg", t_done, 0, true, { Eb, Gb, Zz }, 0 },
  { "xchg", t_done, 0, true, { Ev, Gv, Zz }, 0 },
  /* 88 */
  { "mov", t_done, 0, true, { Eb, Gb, Zz }, 0 },
  { "mov", t_done, 0, true, { Ev, Gv, Zz }, 0 },
  { "mov", t_done, 0, true, { Gb, Eb, Zz }, 0 },
  { "mov", t_done, 0, true, { Gv, Ev, Zz }, 0 },
  { "mov", t_done, 0, true, { Ew, Sw, Zz }, 0 },
  { "lea", t_done, 0, true, { Gv, Mlea, Zz }, 0 }, // this is just M in the book
  { "mov", t_done, 0, true, { Sw, Ew, Zz }, 0 },
  { "pop", t_done, 0, true, { Ev, Zz, Zz }, 0 },
  /* 90 */
  { "nop",  t_done, 0, false, { Zz, Zz, Zz }, 0 },
  { "xchg", t_done, 0, false, { eCX, Zz, Zz }, 0 },
  { "xchg", t_done, 0, false, { eDX, Zz, Zz }, 0 },
  { "xchg", t_done, 0, false, { eBX, Zz, Zz }, 0 },
  { "xchg", t_done, 0, false, { eSP, Zz, Zz }, 0 },
  { "xchg", t_done, 0, false, { eBP, Zz, Zz }, 0 },
  { "xchg", t_done, 0, false, { eSI, Zz, Zz }, 0 },
  { "xchg", t_done, 0, false, { eDI, Zz, Zz }, 0 },
  /* 98 */
  { "cbw",     t_done, 0, false, { Zz, Zz, Zz }, 0 },
  { "cwd/cdq", t_done, 0, false, { Zz, Zz, Zz }, 0 },
  { "call",    t_done, 0, false, { Ap, Zz, Zz }, IS_CALL | PTR_WX },
  { "wait",    t_done, 0, false, { Zz, Zz, Zz }, 0 },
  { "pushf",   t_done, 0, false, { Fv, Zz, Zz }, 0 },
  { "pop",     t_done, 0, false, { Fv, Zz, Zz }, 0 },
  { "sahf",    t_done, 0, false, { Zz, Zz, Zz }, 0 },
  { "lahf",    t_done, 0, false, { Zz, Zz, Zz }, 0 },
  /* A0 */
  { "mov",   t_done, 0, false, { AL, Ob, Zz }, 0 },
  { "mov",   t_done, 0, false, { eAX, Ov, Zz }, 0 },
  { "mov",   t_done, 0, false, { Ob, AL, Zz }, 0 },
  { "mov",   t_done, 0, false, { Ov, eAX, Zz }, 0 },
  { "movsb", t_done, 0, false, { Xb, Yb, Zz }, 0 },
  { "movsw", t_done, 0, false, { Xv, Yv, Zz }, 0 },
  { "cmpsb", t_done, 0, false, { Xb, Yb, Zz }, 0 },
  { "cmpsw", t_done, 0, false, { Xv, Yv, Zz }, 0 },
  /* A8 */
  { "test",     t_done, 0, false, { AL, Ib, Zz }, 0 },
  { "test",     t_done, 0, false, { eAX, Iv, Zz }, 0 },
  { "stopsb",   t_done, 0, false, { Yb, AL, Zz }, 0 },
  { "stopsw/d", t_done, 0, false, { Yv, eAX, Zz }, 0 },
  { "lodsb",    t_done, 0, false, { AL, Xb, Zz }, 0 },
  { "lodsw",    t_done, 0, false, { eAX, Xv, Zz }, 0 },
  { "scasb",    t_done, 0, false, { AL, Yb, Zz }, 0 },
  { "scasw/d",  t_done, 0, false, { eAX, Yv, Zz }, 0 },
  /* B0 */
  { "mov", t_done, 0, false, { AL, Ib, Zz }, 0 },
  { "mov", t_done, 0, false, { CL, Ib, Zz }, 0 },
  { "mov", t_done, 0, false, { DL, Ib, Zz }, 0 },
  { "mov", t_done, 0, false, { BL, Ib, Zz }, 0 },
  { "mov", t_done, 0, false, { AH, Ib, Zz }, 0 },
  { "mov", t_done, 0, false, { CH, Ib, Zz }, 0 },
  { "mov", t_done, 0, false, { DH, Ib, Zz }, 0 },
  { "mov", t_done, 0, false, { BH, Ib, Zz }, 0 },
  /* B8 */
  { "mov", t_done, 0, false, { eAX, Iv, Zz }, 0 },
  { "mov", t_done, 0, false, { eCX, Iv, Zz }, 0 },
  { "mov", t_done, 0, false, { eDX, Iv, Zz }, 0 },
  { "mov", t_done, 0, false, { eBX, Iv, Zz }, 0 },
  { "mov", t_done, 0, false, { eSP, Iv, Zz }, 0 },
  { "mov", t_done, 0, false, { eBP, Iv, Zz }, 0 },
  { "mov", t_done, 0, false, { eSI, Iv, Zz }, 0 },
  { "mov", t_done, 0, false, { eDI, Iv, Zz }, 0 },
  /* C0 */
  { 0, t_grp, Grp2, true, { Eb, Ib, Zz }, 0 },
  { 0, t_grp, Grp2, true, { Ev, Ib, Zz }, 0 },
  { "ret near", t_done, 0, false, { Iw, Zz, Zz }, (IS_RET) },
  { "ret near", t_done, 0, false, { Zz, Zz, Zz }, (IS_RET) },
  { "les",      t_done, 0, true, { Gv, Mp, Zz }, 0 },
  { "lds",      t_done, 0, true, { Gv, Mp, Zz }, 0 },
  { 0, t_grp, Grp11, true, { Eb, Ib, Zz }, 0 },
  { 0, t_grp, Grp11, true, { Ev, Iv, Zz }, 0 },
  /* C8 */
  { "enter",   t_done, 0, false, { Iw, Ib, Zz }, 0 },
  { "leave",   t_done, 0, false, { Zz, Zz, Zz }, 0 },
  { "ret far", t_done, 0, false, { Iw, Zz, Zz }, (IS_RETF) },
  { "ret far", t_done, 0, false, { Zz, Zz, Zz }, (IS_RETF) },
  { "int 3",   t_done, 0, false, { Zz, Zz, Zz }, 0 },
  { "int",     t_done, 0, false, { Ib, Zz, Zz }, 0 },
  { "into",    t_done, 0, false, { Zz, Zz, Zz }, 0 },
  { "iret",    t_done, 0, false, { Zz, Zz, Zz }, (IS_RET) },
  /* D0 */
  { 0, t_grp, Grp2, true, { Eb, Zz, Zz }, 0 }, // const1
  { 0, t_grp, Grp2, true, { Ev, Zz, Zz }, 0 }, // --"--
  { 0, t_grp, Grp2, true, { Eb, CL, Zz }, 0 },
  { 0, t_grp, Grp2, true, { Ev, CL, Zz }, 0 },
  { "aam",  t_done, 0, false, { Zz, Zz, Zz }, 0 },
  { "aad",  t_done, 0, false, { Zz, Zz, Zz }, 0 },
  { "salc", t_done, 0, false, { Zz, Zz, Zz }, 0 }, // sandpile.org gives this as SALC; undocumeted
  { "xlat", t_done, 0, false, { Zz, Zz, Zz }, 0 },
  /* D8 */
  { 0, t_coprocEsc, 0, true, { Zz, Zz, Zz }, 0 },
  { 0, t_coprocEsc, 0, true, { Zz, Zz, Zz }, 0 },
  { 0, t_coprocEsc, 0, true, { Zz, Zz, Zz }, 0 },
  { 0, t_coprocEsc, 0, true, { Zz, Zz, Zz }, 0 },
  { 0, t_coprocEsc, 0, true, { Zz, Zz, Zz }, 0 },
  { 0, t_coprocEsc, 0, true, { Zz, Zz, Zz }, 0 },
  { 0, t_coprocEsc, 0, true, { Zz, Zz, Zz }, 0 },
  { 0, t_coprocEsc, 0, true, { Zz, Zz, Zz }, 0 },
  /* E0 */
  { "loopn",    t_done, 0, false, { Jb, Zz, Zz }, 0 },
  { "loope",    t_done, 0, false, { Jb, Zz, Zz }, 0 },
  { "loop",     t_done, 0, false, { Jb, Zz, Zz }, 0 },
  { "jcxz/jec", t_done, 0, false, { Jb, Zz, Zz }, (IS_JCC | REL_B) },
  { "in",       t_done, 0, false, { AL, Ib, Zz }, 0 },
  { "in",       t_done, 0, false, { eAX, Ib, Zz }, 0 },
  { "out",      t_done, 0, false, { Ib, AL, Zz }, 0 },
  { "out",      t_done, 0, false, { Ib, eAX, Zz }, 0 },
  /* E8 */
  { "call", t_done, 0, false, { Jv, Zz, Zz }, (IS_CALL | REL_X) },
  { "jmp",  t_done, 0, false, { Jv, Zz, Zz }, (IS_JUMP | REL_X) },
  { "jmp",  t_done, 0, false, { Ap, Zz, Zz }, (IS_JUMP | PTR_WX) },
  { "jmp",  t_done, 0, false, { Jb, Zz, Zz }, (IS_JUMP | REL_B) },
  { "in",   t_done, 0, false, { AL, DX, Zz }, 0 },
  { "in",   t_done, 0, false, { eAX, DX, Zz }, 0 },
  { "out",  t_done, 0, false, { DX, AL, Zz }, 0 },
  { "out",  t_done, 0, false, { DX, eAX, Zz }, 0 },
  /* F0 */
  { 0,      t_ill,  0, false, { Zz, Zz, Zz }, 0 }, // PREFIX_INSTR
  { "int1", t_done, 0, false, { Zz, Zz, Zz }, 0 }, // INT1/ICEBP on sandpile; undocumented
  { 0, t_prefixedSSE, 3, false, { Zz, Zz, Zz }, 0 },
  { 0, t_prefixedSSE, 1, false, { Zz, Zz, Zz }, 0 },
  { "hlt",  t_done, 0, false, { Zz, Zz, Zz }, 0 },
  { "cmc",  t_done, 0, false, { Zz, Zz, Zz }, 0 },
  { 0, t_grp, Grp3a, true, { Zz, Zz, Zz }, 0 },
  { 0, t_grp, Grp3b, true, { Zz, Zz, Zz }, 0 },
  /* F8 */
  { "clc", t_done, 0, false, { Zz, Zz, Zz }, 0 },
  { "stc", t_done, 0, false, { Zz, Zz, Zz }, 0 },
  { "cli", t_done, 0, false, { Zz, Zz, Zz }, 0 },
  { "sti", t_done, 0, false, { Zz, Zz, Zz }, 0 },
  { "cld", t_done, 0, false, { Zz, Zz, Zz }, 0 },
  { "std", t_done, 0, false, { Zz, Zz, Zz }, 0 },
  { 0, t_grp, Grp4, true, { Zz, Zz, Zz }, 0 },
  { 0, t_grp, Grp5, true, { Zz, Zz, Zz }, 0 }
};


// twoByteMap: two byte opcode instructions (first byte is 0x0F)
static ia32_entry twoByteMap[256] = {
  /* 00 */
  { 0, t_grp, Grp6, true, { Zz, Zz, Zz }, 0 },
  { 0, t_grp, Grp7, false, { Zz, Zz, Zz }, 0 },
  { "lar",        t_done, 0, true, { Gv, Ew, Zz }, 0  },
  { "lsl",        t_done, 0, true, { Gv, Ew, Zz }, 0  },
  { 0,            t_ill,  0, false, { Zz, Zz, Zz },0},
  { "syscall",    t_done, 0, false, { Zz, Zz, Zz }, 0 }, // AMD
  { "clts",       t_done, 0, false, { Zz, Zz, Zz }, 0 },
  { "sysret",     t_done, 0, false, { Zz, Zz, Zz }, 0 }, // AMD
  /* 08 */
  { "invd",   t_done, 0, false, { Zz, Zz, Zz }, 0 },
  { "wbinvd", t_done, 0, false, { Zz, Zz, Zz }, 0 },
  { 0,        t_ill,  0, false, { Zz, Zz, Zz }, 0 },
  { "ud2",    t_ill,  0, 0, { Zz, Zz, Zz }, 0 },
  { 0,        t_ill,  0, 0, { Zz, Zz, Zz }, 0 },
  { "prefetch(w)", t_done,  0, true,  { Zz, Zz, Zz }, 0 }, // AMD
  { "femms",       t_done,  0, false, { Zz, Zz, Zz }, 0 }, // AMD 
  { 0,             t_3dnow, 0, true,  { Zz, Zz, Zz }, 0 }, // AMD 3DNOW! suffixes
  /* 10 */
  { 0, t_sse, SSE10, true, { Zz, Zz, Zz }, 0 },
  { 0, t_sse, SSE11, true, { Zz, Zz, Zz }, 0 },
  { 0, t_sse, SSE12, true, { Zz, Zz, Zz }, 0 },
  { 0, t_sse, SSE13, true, { Zz, Zz, Zz }, 0 },
  { 0, t_sse, SSE14, true, { Zz, Zz, Zz }, 0 },
  { 0, t_sse, SSE15, true, { Zz, Zz, Zz }, 0 },
  { 0, t_sse, SSE16, true, { Zz, Zz, Zz }, 0 },
  { 0, t_sse, SSE17, true, { Zz, Zz, Zz }, 0 },
  /* 18 */
  { 0, t_grp, Grp16, 0, { Zz, Zz, Zz }, 0 },
  { 0, t_ill, 0, 0, { Zz, Zz, Zz }, 0 },
  { 0, t_ill, 0, 0, { Zz, Zz, Zz }, 0 },
  { 0, t_ill, 0, 0, { Zz, Zz, Zz }, 0 },
  { 0, t_ill, 0, 0, { Zz, Zz, Zz }, 0 },
  { 0, t_ill, 0, 0, { Zz, Zz, Zz }, 0 },
  { 0, t_ill, 0, 0, { Zz, Zz, Zz }, 0 },
  { 0, t_ill, 0, 0, { Zz, Zz, Zz }, 0 },
  /* 20 */
  { "mov", t_done, 0, true, { Rd, Cd, Zz }, 0 },
  { "mov", t_done, 0, true, { Rd, Dd, Zz }, 0 },
  { "mov", t_done, 0, true, { Cd, Rd, Zz }, 0 },
  { "mov", t_done, 0, true, { Dd, Rd, Zz }, 0 },
  { "mov", t_done, 0, true, { Rd, Td, Zz }, 0 },
  { 0,     t_ill,  0, 0, { Zz, Zz, Zz }, 0 },
  { "mov", t_done, 0, true, { Td, Rd, Zz }, 0 },
  { 0,     t_ill,  0, 0, { Zz, Zz, Zz }, 0 },
  /* 28 */
  { 0, t_sse, SSE28, true, { Zz, Zz, Zz }, 0 },
  { 0, t_sse, SSE29, true, { Zz, Zz, Zz }, 0 },
  { 0, t_sse, SSE2A, true, { Zz, Zz, Zz }, 0 },
  { 0, t_sse, SSE2B, true, { Zz, Zz, Zz }, 0 },
  { 0, t_sse, SSE2C, true, { Zz, Zz, Zz }, 0 },
  { 0, t_sse, SSE2D, true, { Zz, Zz, Zz }, 0 },
  { 0, t_sse, SSE2E, true, { Zz, Zz, Zz }, 0 },
  { 0, t_sse, SSE2F, true, { Zz, Zz, Zz }, 0 },
  /* 30 */
  { "wrmsr", t_done, 0, false, { Zz, Zz, Zz }, 0 },
  { "rdtsc", t_done, 0, false, { Zz, Zz, Zz }, 0 },
  { "rdmsr", t_done, 0, false, { Zz, Zz, Zz }, 0 },
  { "rdpmc", t_done, 0, false, { Zz, Zz, Zz }, 0 },
  { "sysenter", t_done, 0, false, { Zz, Zz, Zz }, 0 },
  { "sysexit",  t_done, 0, false, { Zz, Zz, Zz }, 0 },
  {0,t_ill, 0,0,{ Zz, Zz, Zz },0}, 
  {0,t_ill, 0,0,{ Zz, Zz, Zz },0},
  /* 38 */
  {0,t_ill, 0,0,{ Zz, Zz, Zz },0},
  {0,t_ill, 0,0,{ Zz, Zz, Zz },0}, 
  {0,t_ill, 0,0,{ Zz, Zz, Zz },0}, 
  {0,t_ill, 0,0,{ Zz, Zz, Zz },0},
  {0,t_ill, 0,0,{ Zz, Zz, Zz },0},
  {0,t_ill, 0,0,{ Zz, Zz, Zz },0},
  {0,t_ill, 0,0,{ Zz, Zz, Zz },0},
  {0,t_ill, 0,0,{ Zz, Zz, Zz },0},
  /* 40 */
  { "cmovo",   t_done, 0, true, { Gv, Ev, Zz }, 0 },
  { "cmovno",  t_done, 0, true, { Gv, Ev, Zz }, 0 },
  { "cmovnae", t_done, 0, true, { Gv, Ev, Zz }, 0 },
  { "cmovnb",  t_done, 0, true, { Gv, Ev, Zz }, 0 },
  { "cmove",   t_done, 0, true, { Gv, Ev, Zz }, 0 },
  { "cmovne",  t_done, 0, true, { Gv, Ev, Zz }, 0 },
  { "cmovbe",  t_done, 0, true, { Gv, Ev, Zz }, 0 },
  { "cmovnbe", t_done, 0, true, { Gv, Ev, Zz }, 0 },
  /* 48 */
  { "cmovs",   t_done, 0, true, { Gv, Ev, Zz }, 0 },
  { "cmovns",  t_done, 0, true, { Gv, Ev, Zz }, 0 },
  { "cmovpe",  t_done, 0, true, { Gv, Ev, Zz }, 0 },
  { "cmovpo",  t_done, 0, true, { Gv, Ev, Zz }, 0 },
  { "cmovnge", t_done, 0, true, { Gv, Ev, Zz }, 0 },
  { "cmovnl",  t_done, 0, true, { Gv, Ev, Zz }, 0 },
  { "cmovng",  t_done, 0, true, { Gv, Ev, Zz }, 0 },
  { "cmovnl",  t_done, 0, true, { Gv, Ev, Zz }, 0 },
  /* 50 */
  { 0, t_sse, SSE50, true, { Zz, Zz, Zz }, 0 },
  { 0, t_sse, SSE51, true, { Zz, Zz, Zz }, 0 },
  { 0, t_sse, SSE52, true, { Zz, Zz, Zz }, 0 },
  { 0, t_sse, SSE53, true, { Zz, Zz, Zz }, 0 },
  { 0, t_sse, SSE54, true, { Zz, Zz, Zz }, 0 },
  { 0, t_sse, SSE55, true, { Zz, Zz, Zz }, 0 },
  { 0, t_sse, SSE56, true, { Zz, Zz, Zz }, 0 },
  { 0, t_sse, SSE57, true, { Zz, Zz, Zz }, 0 },
  /* 58 */
  { 0, t_sse, SSE58, true, { Zz, Zz, Zz }, 0 },
  { 0, t_sse, SSE59, true, { Zz, Zz, Zz }, 0 },
  { 0, t_sse, SSE5A, true, { Zz, Zz, Zz }, 0 },
  { 0, t_sse, SSE5B, true, { Zz, Zz, Zz }, 0 },
  { 0, t_sse, SSE5C, true, { Zz, Zz, Zz }, 0 },
  { 0, t_sse, SSE5D, true, { Zz, Zz, Zz }, 0 },
  { 0, t_sse, SSE5E, true, { Zz, Zz, Zz }, 0 },
  { 0, t_sse, SSE5F, true, { Zz, Zz, Zz }, 0 },
  /* 60 */
  { 0, t_sse, SSE60, true, { Zz, Zz, Zz }, 0 },
  { 0, t_sse, SSE61, true, { Zz, Zz, Zz }, 0 },
  { 0, t_sse, SSE62, true, { Zz, Zz, Zz }, 0 },
  { 0, t_sse, SSE63, true, { Zz, Zz, Zz }, 0 },
  { 0, t_sse, SSE64, true, { Zz, Zz, Zz }, 0 },
  { 0, t_sse, SSE65, true, { Zz, Zz, Zz }, 0 },
  { 0, t_sse, SSE66, true, { Zz, Zz, Zz }, 0 },
  { 0, t_sse, SSE67, true, { Zz, Zz, Zz }, 0 },
  /* 68 */
  { 0, t_sse, SSE68, true, { Zz, Zz, Zz }, 0 },
  { 0, t_sse, SSE69, true, { Zz, Zz, Zz }, 0 },
  { 0, t_sse, SSE6A, true, { Zz, Zz, Zz }, 0 },
  { 0, t_sse, SSE6B, true, { Zz, Zz, Zz }, 0 },
  { 0, t_sse, SSE6C, true, { Zz, Zz, Zz }, 0 },
  { 0, t_sse, SSE6D, true, { Zz, Zz, Zz }, 0 },
  { 0, t_sse, SSE6E, true, { Zz, Zz, Zz }, 0 },
  { 0, t_sse, SSE6F, true, { Zz, Zz, Zz }, 0 },
  /* 70 */
  { 0, t_sse, SSE70, true, { Zz, Zz, Zz }, 0 },
  { 0, t_grp, Grp12, false, { Zz, Zz, Zz }, 0 },
  { 0, t_grp, Grp13, false, { Zz, Zz, Zz }, 0 },
  { 0, t_grp, Grp14, false, { Zz, Zz, Zz }, 0 },
  { 0, t_sse, SSE74, true, { Zz, Zz, Zz }, 0 },
  { 0, t_sse, SSE75, true, { Zz, Zz, Zz }, 0 },
  { 0, t_sse, SSE76, true, { Zz, Zz, Zz }, 0 },
  { "emms", t_done, 0, false, { Zz, Zz, Zz }, 0 },
  /* 78 */
  { "mmxud", t_ill, 0, 0, { Zz, Zz, Zz }, 0},
  { "mmxud", t_ill, 0, 0, { Zz, Zz, Zz }, 0},
  { "mmxud", t_ill, 0, 0, { Zz, Zz, Zz }, 0},
  { "mmxud", t_ill, 0, 0, { Zz, Zz, Zz }, 0},
  { "mmxud", t_ill, 0, 0, { Zz, Zz, Zz }, 0},
  { "mmxud", t_ill, 0, 0, { Zz, Zz, Zz }, 0},
  { 0, t_sse, SSE7E, 0, { Zz, Zz, Zz }, 0 },
  { 0, t_sse, SSE7F, 0, { Zz, Zz, Zz }, 0 },
  /* 80 */
  { "jo",   t_done, 0, false, { Jv, Zz, Zz }, (IS_JCC | REL_X) },
  { "jno",  t_done, 0, false, { Jv, Zz, Zz }, (IS_JCC | REL_X) },
  { "jb",   t_done, 0, false, { Jv, Zz, Zz }, (IS_JCC | REL_X) },
  { "jnb",  t_done, 0, false, { Jv, Zz, Zz }, (IS_JCC | REL_X) },
  { "jz",   t_done, 0, false, { Jv, Zz, Zz }, (IS_JCC | REL_X) },
  { "jnz",  t_done, 0, false, { Jv, Zz, Zz }, (IS_JCC | REL_X) },
  { "jbe",  t_done, 0, false, { Jv, Zz, Zz }, (IS_JCC | REL_X) },
  { "jnbe", t_done, 0, false, { Jv, Zz, Zz }, (IS_JCC | REL_X) },
  /* 88 */
  { "js",   t_done, 0, false, { Jv, Zz, Zz }, (IS_JCC | REL_X) },
  { "jns",  t_done, 0, false, { Jv, Zz, Zz }, (IS_JCC | REL_X) },
  { "jp",   t_done, 0, false, { Jv, Zz, Zz }, (IS_JCC | REL_X) },
  { "jnp",  t_done, 0, false, { Jv, Zz, Zz }, (IS_JCC | REL_X) },
  { "jl",   t_done, 0, false, { Jv, Zz, Zz }, (IS_JCC | REL_X) },
  { "jnl",  t_done, 0, false, { Jv, Zz, Zz }, (IS_JCC | REL_X) },
  { "jle",  t_done, 0, false, { Jv, Zz, Zz }, (IS_JCC | REL_X) },
  { "jnle", t_done, 0, false, { Jv, Zz, Zz }, (IS_JCC | REL_X) },
  /* 90 */
  { "seto",   t_done, 0, true, { Eb, Zz, Zz }, 0 },
  { "setno",  t_done, 0, true, { Eb, Zz, Zz }, 0 },
  { "setb",   t_done, 0, true, { Eb, Zz, Zz }, 0 },
  { "setnb",  t_done, 0, true, { Eb, Zz, Zz }, 0 },
  { "setz",   t_done, 0, true, { Eb, Zz, Zz }, 0 },
  { "setnz",  t_done, 0, true, { Eb, Zz, Zz }, 0 },
  { "setbe",  t_done, 0, true, { Eb, Zz, Zz }, 0 },
  { "setnbe", t_done, 0, true, { Eb, Zz, Zz }, 0 },
  /* 98 */
  { "sets",   t_done, 0, true, { Eb, Zz, Zz }, 0 },
  { "setns",  t_done, 0, true, { Eb, Zz, Zz }, 0 },
  { "setp",   t_done, 0, true, { Eb, Zz, Zz }, 0 },
  { "setnp",  t_done, 0, true, { Eb, Zz, Zz }, 0 },
  { "setl",   t_done, 0, true, { Eb, Zz, Zz }, 0 },
  { "setnl",  t_done, 0, true, { Eb, Zz, Zz }, 0 },
  { "setle",  t_done, 0, true, { Eb, Zz, Zz }, 0 },
  { "setnle", t_done, 0, true, { Eb, Zz, Zz }, 0 },
  /* A0 */
  { "push",   t_done, 0, false, { FS, Zz, Zz }, 0 },
  { "pop",    t_done, 0, false, { FS, Zz, Zz }, 0 },
  { "cpuid",  t_done, 0, false, { Zz, Zz, Zz }, 0 },
  { "bt",     t_done, 0, true, { Ev, Gv, Zz }, 0 },
  { "shld",   t_done, 0, true, { Ev, Gv, Ib }, 0 },
  { "shld",   t_done, 0, true, { Ev, Gv, CL }, 0 },
  { 0, t_ill, 0, 0, { Zz, Zz, Zz }, 0}, 
  { 0, t_ill, 0, 0, { Zz, Zz, Zz }, 0},
  /* A8 */
  { "push", t_done, 0, false, { GS, Zz, Zz }, 0 },
  { "pop",  t_done, 0, false, { GS, Zz, Zz }, 0 },
  { "rsm",  t_done, 0, false, { Zz, Zz, Zz }, 0 },
  { "bts",  t_done, 0, true, { Ev, Gv, Zz }, 0 },
  { "shrd", t_done, 0, true, { Ev, Gv, Ib }, 0 },
  { "shrd", t_done, 0, true, { Ev, Gv, CL }, 0 },
  { 0, t_grp, Grp15, 0, { Zz, Zz, Zz }, 0 }, 
  { "imul", t_done, 0, true, { Gv, Ev, Zz }, 0 },
  /* B0 */
  { "cmpxch", t_done, 0, true, { Eb, Gb, Zz }, 0 },
  { "cmpxch", t_done, 0, true, { Ev, Gv, Zz }, 0 },
  { "lss", t_done, 0, true, { Mp, Zz, Zz }, 0 },
  { "btr", t_done, 0, true, { Ev, Gv, Zz }, 0 },
  { "lfs", t_done, 0, true, { Mp, Zz, Zz }, 0 },
  { "lgs", t_done, 0, true, { Mp, Zz, Zz }, 0 },
  { "movzx", t_done, 0, true, { Gv, Eb, Zz }, 0 },
  { "movzx", t_done, 0, true, { Gv, Ew, Zz }, 0 },
  /* B8 */
  {0,t_ill, 0,0,{ Zz, Zz, Zz },0},
  { "ud2grp10", t_ill, 0, 0, { Zz, Zz, Zz }, 0 },
  { 0, t_grp, Grp8, true, { Ev, Ib, Zz }, 0 },
  { "btc", t_done, 0, true, { Ev, Gv, Zz }, 0 },
  { "bsf", t_done, 0, true, { Gv, Ev, Zz }, 0 },
  { "bsr", t_done, 0, true, { Gv, Ev, Zz }, 0 },
  { "movsx", t_done, 0, true, { Gv, Eb, Zz }, 0 },
  { "movsx", t_done, 0, true, { Gv, Ew, Zz }, 0 },
  /* C0 */
  { "xadd", t_done, 0, true, { Eb, Gb, Zz }, 0 },
  { "xadd", t_done, 0, true, { Ev, Gv, Zz }, 0 },
  { 0, t_sse, SSEC2, true, { Zz, Zz, Zz }, 0 },
  { "movnti" , t_done, 0, 0, { Ed, Gd, Zz }, 0 },
  { 0, t_sse, SSEC4, true, { Zz, Zz, Zz }, 0 },
  { 0, t_sse, SSEC5, true, { Zz, Zz, Zz }, 0 },
  { 0, t_sse, SSEC6, true, { Zz, Zz, Zz }, 0 },
  { 0, t_grp, Grp9,  true, { Zz, Zz, Zz }, 0 },
  /* C8 */
  { "bswap", t_done, 0, false, { EAX, Zz, Zz }, 0 }, 
  { "bswap", t_done, 0, false, { ECX, Zz, Zz }, 0 }, 
  { "bswap", t_done, 0, false, { EDX, Zz, Zz }, 0 }, 
  { "bswap", t_done, 0, false, { EBX, Zz, Zz }, 0 }, 
  { "bswap", t_done, 0, false, { ESP, Zz, Zz }, 0 },
  { "bswap", t_done, 0, false, { EBP, Zz, Zz }, 0 }, 
  { "bswap", t_done, 0, false, { ESI, Zz, Zz }, 0 }, 
  { "bswap", t_done, 0, false, { EDI, Zz, Zz }, 0 }, 
  /* D0 */
  { 0, t_ill, 0, false, { Zz, Zz, Zz }, 0 },
  { 0, t_sse, SSED1, true, { Zz, Zz, Zz }, 0 },
  { 0, t_sse, SSED2, true, { Zz, Zz, Zz }, 0 },
  { 0, t_sse, SSED3, true, { Zz, Zz, Zz }, 0 },
  { 0, t_sse, SSED4, true, { Zz, Zz, Zz }, 0 },
  { 0, t_sse, SSED5, true, { Zz, Zz, Zz }, 0 },
  { 0, t_sse, SSED6, true, { Zz, Zz, Zz }, 0 },
  { 0, t_sse, SSED7, true, { Zz, Zz, Zz }, 0 },
  /* D8 */
  { 0, t_sse, SSED8, true, { Zz, Zz, Zz }, 0 },
  { 0, t_sse, SSED9, true, { Zz, Zz, Zz }, 0 },
  { 0, t_sse, SSEDA, true, { Zz, Zz, Zz }, 0 },
  { 0, t_sse, SSEDB, true, { Zz, Zz, Zz }, 0 },
  { 0, t_sse, SSEDC, true, { Zz, Zz, Zz }, 0 },
  { 0, t_sse, SSEDD, true, { Zz, Zz, Zz }, 0 },
  { 0, t_sse, SSEDE, true, { Zz, Zz, Zz }, 0 },
  { 0, t_sse, SSEDF, true, { Zz, Zz, Zz }, 0 },
  /* E0 */
  { 0, t_sse, SSEE0, true, { Zz, Zz, Zz }, 0 },
  { 0, t_sse, SSEE1, true, { Zz, Zz, Zz }, 0 },
  { 0, t_sse, SSEE2, true, { Zz, Zz, Zz }, 0 },
  { 0, t_sse, SSEE3, true, { Zz, Zz, Zz }, 0 },
  { 0, t_sse, SSEE4, true, { Zz, Zz, Zz }, 0 },
  { 0, t_sse, SSEE5, true, { Zz, Zz, Zz }, 0 },
  { 0, t_sse, SSEE6, true, { Zz, Zz, Zz }, 0 },
  { 0, t_sse, SSEE7, true, { Zz, Zz, Zz }, 0 },
  /* E8 */
  { 0, t_sse, SSEE8, true, { Zz, Zz, Zz }, 0 },
  { 0, t_sse, SSEE9, true, { Zz, Zz, Zz }, 0 },
  { 0, t_sse, SSEEA, true, { Zz, Zz, Zz }, 0 },
  { 0, t_sse, SSEEB, true, { Zz, Zz, Zz }, 0 },
  { 0, t_sse, SSEEC, true, { Zz, Zz, Zz }, 0 },
  { 0, t_sse, SSEED, true, { Zz, Zz, Zz }, 0 },
  { 0, t_sse, SSEEE, true, { Zz, Zz, Zz }, 0 },
  { 0, t_sse, SSEEF, true, { Zz, Zz, Zz }, 0 },
  /* F0 */
  { 0, t_ill, 0, false, { Zz, Zz, Zz }, 0 },
  { 0, t_sse, SSEF1, true, { Zz, Zz, Zz }, 0 },
  { 0, t_sse, SSEF2, true, { Zz, Zz, Zz }, 0 },
  { 0, t_sse, SSEF3, true, { Zz, Zz, Zz }, 0 },
  { 0, t_sse, SSEF4, true, { Zz, Zz, Zz }, 0 },
  { 0, t_sse, SSEF5, true, { Zz, Zz, Zz }, 0 },
  { 0, t_sse, SSEF6, true, { Zz, Zz, Zz }, 0 },
  { 0, t_sse, SSEF7, true, { Zz, Zz, Zz }, 0 },
  /* F8 */
  { 0, t_sse, SSEF8, true, { Zz, Zz, Zz }, 0 },
  { 0, t_sse, SSEF9, true, { Zz, Zz, Zz }, 0 },
  { 0, t_sse, SSEFA, true, { Zz, Zz, Zz }, 0 },
  { 0, t_sse, SSEFB, true, { Zz, Zz, Zz }, 0 },
  { 0, t_sse, SSEFC, true, { Zz, Zz, Zz }, 0 },
  { 0, t_sse, SSEFD, true, { Zz, Zz, Zz }, 0 },
  { 0, t_sse, SSEFE, true, { Zz, Zz, Zz }, 0 },
  { 0, t_ill, 0, false, { Zz, Zz, Zz }, 0}
};

static ia32_entry groupMap[][8] = {
 { /* group 1 - only opcode is defined here,
      operands are defined in the one or two byte maps below */
  { "add", t_done, 0, true, { Zz, Zz, Zz }, 0 },
  { "or",  t_done, 0, true, { Zz, Zz, Zz }, 0 },
  { "adc", t_done, 0, true, { Zz, Zz, Zz }, 0 },
  { "sbb", t_done, 0, true, { Zz, Zz, Zz }, 0 },
  { "and", t_done, 0, true, { Zz, Zz, Zz }, 0 },
  { "sub", t_done, 0, true, { Zz, Zz, Zz }, 0 },
  { "xor", t_done, 0, true, { Zz, Zz, Zz }, 0 },
  { "cmp", t_done, 0, true, { Zz, Zz, Zz }, 0 }
 },

 {  /* group 2 - only opcode is defined here, 
       operands are defined in the one or two byte maps below */
  { "rol", t_done, 0, true, { Zz, Zz, Zz }, 0 },
  { "ror", t_done, 0, true, { Zz, Zz, Zz }, 0 },
  { "rcl", t_done, 0, true, { Zz, Zz, Zz }, 0 },
  { "rcr", t_done, 0, true, { Zz, Zz, Zz }, 0 },
  { "shl/sal", t_done, 0, true, { Zz, Zz, Zz }, 0 },
  { "shr", t_done, 0, true, { Zz, Zz, Zz }, 0 },
  { 0, t_ill, 0, true, { Zz, Zz, Zz }, 0 },
  { "sar", t_done, 0, true, { Zz, Zz, Zz }, 0 }
 },

 { /* group 3a - operands are defined here */
  { "test", t_done, 0, true, { Eb, Ib, Zz }, 0 },
  { 0, t_ill, 0, true, { Zz, Zz, Zz }, 0 },
  { "not",  t_done, 0, true, { Eb, Zz, Zz }, 0 },
  { "neg",  t_done, 0, true, { Eb, Zz, Zz }, 0 },
  { "mul",  t_done, 0, true, { AL, Eb, Zz }, 0 },
  { "imul", t_done, 0, true, { AL, Eb, Zz }, 0 },
  { "div",  t_done, 0, true, { AL, Eb, Zz }, 0 },
  { "idiv", t_done, 0, true, { AL, Eb, Zz }, 0 }
 },

 { /* group 3b - operands are defined here */
  { "test", t_done, 0, true, { Ev, Iv, Zz }, 0 },
  { 0, t_ill, 0, true, { Zz, Zz, Zz }, 0 },
  { "not",  t_done, 0, true, { Ev, Zz, Zz }, 0 },
  { "neg",  t_done, 0, true, { Ev, Zz, Zz }, 0 },
  { "mul",  t_done, 0, true, { eAX, Ev, Zz }, 0 },
  { "imul", t_done, 0, true, { eAX, Ev, Zz }, 0 },
  { "div",  t_done, 0, true, { eAX, Ev, Zz }, 0 },
  { "idiv", t_done, 0, true, { eAX, Ev, Zz }, 0 }
 },

 { /* group 4 - operands are defined here */
  { "inc", t_done, 0, true, { Eb, Zz, Zz }, 0 },
  { "dec", t_done, 0, true, { Eb, Zz, Zz }, 0 },
  { 0, t_ill, 0, true, { Zz, Zz, Zz }, 0 },
  { 0, t_ill, 0, true, { Zz, Zz, Zz }, 0 },
  { 0, t_ill, 0, true, { Zz, Zz, Zz }, 0 },
  { 0, t_ill, 0, true, { Zz, Zz, Zz }, 0 },
  { 0, t_ill, 0, true, { Zz, Zz, Zz }, 0 },
  { 0, t_ill, 0, true, { Zz, Zz, Zz }, 0 },
 },

 { /* group 5 - operands are defined here */
  { "inc",  t_done, 0, true, { Ev, Zz, Zz }, 0 },
  { "dec",  t_done, 0, true, { Ev, Zz, Zz }, 0 },
  { "call", t_done, 0, true, { Ev, Zz, Zz }, (IS_CALL | INDIR) },
  { "call", t_done, 0, true, { Ep, Zz, Zz }, (IS_CALL | INDIR) },
  { "jmp",  t_done, 0, true, { Ev, Zz, Zz }, (IS_JUMP | INDIR) },
  { "jmp",  t_done, 0, true, { Ep, Zz, Zz }, (IS_JUMP | INDIR) },
  { "push", t_done, 0, true, { Ev, Zz, Zz }, 0 },
  { 0, t_ill, 0, true, { Zz, Zz, Zz }, 0 },
 },

 { /* group 6 - operands are defined here */
  { "sldt", t_done, 0, true, { Ew, Zz, Zz }, 0 },
  { "str",  t_done, 0, true, { Ew, Zz, Zz }, 0 },
  { "lldt", t_done, 0, true, { Ew, Zz, Zz }, 0 },
  { "ltr",  t_done, 0, true, { Ew, Zz, Zz }, 0 },
  { "verr", t_done, 0, true, { Ew, Zz, Zz }, 0 },
  { "verw", t_done, 0, true, { Ew, Zz, Zz }, 0 },
  { 0, t_ill, 0, true, { Zz, Zz, Zz }, 0 },
  { 0, t_ill, 0, true, { Zz, Zz, Zz }, 0 },
 },

 { /* group 7 - operands are defined here */
  { "sgdt", t_done, 0, true, { Ms, Zz, Zz }, 0 },
  { "sidt", t_done, 0, true, { Ms, Zz, Zz }, 0 },
  { "lgdt", t_done, 0, true, { Ms, Zz, Zz }, 0 },
  { "lidt", t_done, 0, true, { Ms, Zz, Zz }, 0 },
  { "smsw", t_done, 0, true, { Ew, Zz, Zz }, 0 },
  { 0, t_ill, 0, true, { Zz, Zz, Zz }, 0 },
  { "lmsw", t_done, 0, true, { Ew, Zz, Zz }, 0 },
  { "invlpg", t_done, 0, true, { Zz, Zz, Zz }, 0 },
 },

 { /* group 8 - only opcode is defined here, 
     operands are defined in the one or two byte maps below */
  { 0, t_ill, 0, true, { Zz, Zz, Zz }, 0 },
  { 0, t_ill, 0, true, { Zz, Zz, Zz }, 0 },
  { 0, t_ill, 0, true, { Zz, Zz, Zz }, 0 },
  { 0, t_ill, 0, true, { Zz, Zz, Zz }, 0 },
  { "bt",  t_done, 0, true, { Zz, Zz, Zz }, 0 },
  { "bts", t_done, 0, true, { Zz, Zz, Zz }, 0 },
  { "btr", t_done, 0, true, { Zz, Zz, Zz }, 0 },
  { "btc", t_done, 0, true, { Zz, Zz, Zz }, 0 },
 },

 { /* group 9 - operands are defined here */
  { 0, t_ill, 0, true, { Zz, Zz, Zz }, 0 },
  { "cmpxch8b", t_done, 0, true, { Mq, Zz, Zz }, 0 },
  { 0, t_ill, 0, true, { Zz, Zz, Zz }, 0 },
  { 0, t_ill, 0, true, { Zz, Zz, Zz }, 0 },
  { 0, t_ill, 0, true, { Zz, Zz, Zz }, 0 },
  { 0, t_ill, 0, true, { Zz, Zz, Zz }, 0 },
  { 0, t_ill, 0, true, { Zz, Zz, Zz }, 0 },
  { 0, t_ill, 0, true, { Zz, Zz, Zz }, 0 }
 },

 /* group 10 is all illegal */

 { /* group 11, opcodes defined in one byte map */
   { "mov", t_done, 0, true, { Zz, Zz, Zz }, 0 },
   { 0, t_ill, 0, true, { Zz, Zz, Zz }, 0 },
   { 0, t_ill, 0, true, { Zz, Zz, Zz }, 0 },
   { 0, t_ill, 0, true, { Zz, Zz, Zz }, 0 },
   { 0, t_ill, 0, true, { Zz, Zz, Zz }, 0 },
   { 0, t_ill, 0, true, { Zz, Zz, Zz }, 0 },
   { 0, t_ill, 0, true, { Zz, Zz, Zz }, 0 },
   { 0, t_ill, 0, true, { Zz, Zz, Zz }, 0 },
 }

};


// Groups 12-16 are split by mod={mem,11}. Some spill over into SSE groups!
// Notation: G12SSE010B = group 12, SSE, reg=010; B means mod=11
// Use A if Intel decides to put SSE instructions for mod=mem
static ia32_entry groupMap2[][2][8] = {
  { /* group 12 */
    {
      { 0, t_ill, 0, true, { Zz, Zz, Zz }, 0 },
      { 0, t_ill, 0, true, { Zz, Zz, Zz }, 0 },
      { 0, t_ill, 0, true, { Zz, Zz, Zz }, 0 },
      { 0, t_ill, 0, true, { Zz, Zz, Zz }, 0 },
      { 0, t_ill, 0, true, { Zz, Zz, Zz }, 0 },
      { 0, t_ill, 0, true, { Zz, Zz, Zz }, 0 },
      { 0, t_ill, 0, true, { Zz, Zz, Zz }, 0 },
      { 0, t_ill, 0, true, { Zz, Zz, Zz }, 0 },
    },
    {
      { 0, t_ill, 0, true, { Zz, Zz, Zz }, 0 },
      { 0, t_ill, 0, true, { Zz, Zz, Zz }, 0 },
      { 0, t_grpsse, G12SSE010B, true, { Zz, Zz, Zz }, 0 },
      { 0, t_ill, 0, true, { Zz, Zz, Zz }, 0 },
      { 0, t_grpsse, G12SSE100B, true, { Zz, Zz, Zz }, 0 },
      { 0, t_ill, 0, true, { Zz, Zz, Zz }, 0 },
      { 0, t_grpsse, G12SSE110B, true, { Zz, Zz, Zz }, 0 },
      { 0, t_ill, 0, true, { Zz, Zz, Zz }, 0 },
    }
  },
  { /* group 13 */
    {
      { 0, t_ill, 0, true, { Zz, Zz, Zz }, 0 },
      { 0, t_ill, 0, true, { Zz, Zz, Zz }, 0 },
      { 0, t_ill, 0, true, { Zz, Zz, Zz }, 0 },
      { 0, t_ill, 0, true, { Zz, Zz, Zz }, 0 },
      { 0, t_ill, 0, true, { Zz, Zz, Zz }, 0 },
      { 0, t_ill, 0, true, { Zz, Zz, Zz }, 0 },
      { 0, t_ill, 0, true, { Zz, Zz, Zz }, 0 },
      { 0, t_ill, 0, true, { Zz, Zz, Zz }, 0 },
    },
    {
      { 0, t_ill, 0, true, { Zz, Zz, Zz }, 0 },
      { 0, t_ill, 0, true, { Zz, Zz, Zz }, 0 },
      { 0, t_grpsse, G13SSE010B, true, { Zz, Zz, Zz }, 0 },
      { 0, t_ill, 0, true, { Zz, Zz, Zz }, 0 },
      { 0, t_grpsse, G13SSE100B, true, { Zz, Zz, Zz }, 0 },
      { 0, t_ill, 0, true, { Zz, Zz, Zz }, 0 },
      { 0, t_grpsse, G13SSE110B, true, { Zz, Zz, Zz }, 0 },
      { 0, t_ill, 0, true, { Zz, Zz, Zz }, 0 },
    }
  },
  { /* group 14 */
    {
      { 0, t_ill, 0, true, { Zz, Zz, Zz }, 0 },
      { 0, t_ill, 0, true, { Zz, Zz, Zz }, 0 },
      { 0, t_ill, 0, true, { Zz, Zz, Zz }, 0 },
      { 0, t_ill, 0, true, { Zz, Zz, Zz }, 0 },
      { 0, t_ill, 0, true, { Zz, Zz, Zz }, 0 },
      { 0, t_ill, 0, true, { Zz, Zz, Zz }, 0 },
      { 0, t_ill, 0, true, { Zz, Zz, Zz }, 0 },
      { 0, t_ill, 0, true, { Zz, Zz, Zz }, 0 },
    },
    {
      { 0, t_ill, 0, true, { Zz, Zz, Zz }, 0 },
      { 0, t_ill, 0, true, { Zz, Zz, Zz }, 0 },
      { 0, t_grpsse, G14SSE010B, true, { Zz, Zz, Zz }, 0 },
      { 0, t_grpsse, G14SSE011B, true, { Zz, Zz, Zz }, 0 },
      { 0, t_ill, 0, true, { Zz, Zz, Zz }, 0 },
      { 0, t_ill, 0, true, { Zz, Zz, Zz }, 0 },
      { 0, t_grpsse, G14SSE110B, true, { Zz, Zz, Zz }, 0 },
      { 0, t_grpsse, G14SSE111B, true, { Zz, Zz, Zz }, 0 },
    }
  },
  { /* group 15 */
    {
      { "fxsave",  t_done, 0, true, { Zz, Zz, Zz }, 0 },
      { "fxrstor", t_done, 0, true, { Zz, Zz, Zz }, 0 },
      { "ldmxcsr", t_done, 0, true, { Zz, Zz, Zz }, 0 },
      { "stmxcsr", t_done, 0, true, { Zz, Zz, Zz }, 0 },
      { 0, t_ill, 0, true, { Zz, Zz, Zz }, 0 },
      { 0, t_ill, 0, true, { Zz, Zz, Zz }, 0 },
      { 0, t_ill, 0, true, { Zz, Zz, Zz }, 0 },
      { "clflush", t_done, 0, true, { Zz, Zz, Zz }, 0 },
    },
    {
      { 0, t_ill, 0, true, { Zz, Zz, Zz }, 0 },
      { 0, t_ill, 0, true, { Zz, Zz, Zz }, 0 },
      { 0, t_ill, 0, true, { Zz, Zz, Zz }, 0 },
      { 0, t_ill, 0, true, { Zz, Zz, Zz }, 0 },
      { 0, t_ill, 0, true, { Zz, Zz, Zz }, 0 },
      { "lfence", t_done, 0, true, { Zz, Zz, Zz }, 0 },
      { "mfence", t_done, 0, true, { Zz, Zz, Zz }, 0 },
      { "sfence", t_done, 0, true, { Zz, Zz, Zz }, 0 },
    }
  },
  { /* group 16 */
    {
      { "prefetchNTA", t_done, 0, true, { Zz, Zz, Zz }, 0 },
      { "prefetchT0",  t_done, 0, true, { Zz, Zz, Zz }, 0 },
      { "prefetchT1",  t_done, 0, true, { Zz, Zz, Zz }, 0 },
      { "prefetchT2",  t_done, 0, true, { Zz, Zz, Zz }, 0 },
      { 0, t_ill, 0, true, { Zz, Zz, Zz }, 0 },
      { 0, t_ill, 0, true, { Zz, Zz, Zz }, 0 },
      { 0, t_ill, 0, true, { Zz, Zz, Zz }, 0 },
      { 0, t_ill, 0, true, { Zz, Zz, Zz }, 0 },
    },
    {
      { 0, t_ill, 0, true, { Zz, Zz, Zz }, 0 },
      { 0, t_ill, 0, true, { Zz, Zz, Zz }, 0 },
      { 0, t_ill, 0, true, { Zz, Zz, Zz }, 0 },
      { 0, t_ill, 0, true, { Zz, Zz, Zz }, 0 },
      { 0, t_ill, 0, true, { Zz, Zz, Zz }, 0 },
      { 0, t_ill, 0, true, { Zz, Zz, Zz }, 0 },
      { 0, t_ill, 0, true, { Zz, Zz, Zz }, 0 },
      { 0, t_ill, 0, true, { Zz, Zz, Zz }, 0 },
    }
  },
  { /* AMD prefetch group */
    {
      { "prefetch",   t_done, 0, true, { Zz, Zz, Zz }, 0 },
      { "prefetchw",  t_done, 0, true, { Zz, Zz, Zz }, 0 },
      { 0, t_ill, 0, true, { Zz, Zz, Zz }, 0 }, // this is reserved, not illegal, ugh...
      { 0, t_ill, 0, true, { Zz, Zz, Zz }, 0 }, // this is reserved, not illegal, ugh...
      { 0, t_ill, 0, true, { Zz, Zz, Zz }, 0 }, // this is reserved, not illegal, ugh...
      { 0, t_ill, 0, true, { Zz, Zz, Zz }, 0 }, // this is reserved, not illegal, ugh...
      { 0, t_ill, 0, true, { Zz, Zz, Zz }, 0 }, // this is reserved, not illegal, ugh...
      { 0, t_ill, 0, true, { Zz, Zz, Zz }, 0 }, // this is reserved, not illegal, ugh...
    },
    {
      { 0, t_ill, 0, true, { Zz, Zz, Zz }, 0 },
      { 0, t_ill, 0, true, { Zz, Zz, Zz }, 0 },
      { 0, t_ill, 0, true, { Zz, Zz, Zz }, 0 },
      { 0, t_ill, 0, true, { Zz, Zz, Zz }, 0 },
      { 0, t_ill, 0, true, { Zz, Zz, Zz }, 0 },
      { 0, t_ill, 0, true, { Zz, Zz, Zz }, 0 },
      { 0, t_ill, 0, true, { Zz, Zz, Zz }, 0 },
      { 0, t_ill, 0, true, { Zz, Zz, Zz }, 0 },
    }
  }
};

/* rows are not, F3, 66, F2 prefixed in this order (see book) */
static ia32_entry sseMap[][4] = {
  { /* SSE10 */
    { "movups", t_done, 0, true, { Vps, Wps, Zz }, 0 },
    { "movss",  t_done, 0, true, { Vss, Wss, Zz }, 0 },
    { "movupd", t_done, 0, true, { Vpd, Wpd, Zz }, 0 },
    { "movsd",  t_done, 0, true, { Vsd, Wsd, Zz }, 0 },
  },
  { /* SSE11 */
    { "movups", t_done, 0, true, { Wps, Vps, Zz }, 0 },
    { "movss",  t_done, 0, true, { Wss, Vss, Zz }, 0 },
    { "movupd", t_done, 0, true, { Wpd, Vpd, Zz }, 0 },
    { "movsd",  t_done, 0, true, { Vsd, Wsd, Zz }, 0 }, // FIXME: bug in book?????
  },
  { /* SSE12 */
    { "movlps/movhlps", t_done, 0, true, { Wq, Vq, Zz }, 0 }, // FIXME: wierd 1st op
    { 0, t_ill, 0, false, { Zz, Zz, Zz }, 0 },
    { "movlpd", t_done, 0, true, { Vq, Ws, Zz }, 0 },
    { 0, t_ill, 0, false, { Zz, Zz, Zz }, 0 },
  },
  { /* SSE13 */
    { "movlps", t_done, 0, true, { Vq, Wq, Zz }, 0 },
    { 0, t_ill, 0, false, { Zz, Zz, Zz }, 0 },
    { "movlpd", t_done, 0, true, { Vq, Wq, Zz }, 0 },
    { 0, t_ill, 0, false, { Zz, Zz, Zz }, 0 },
  },
  { /* SSE14 */
    { "unpcklps", t_done, 0, true, { Vps, Wq, Zz }, 0 },
    { 0, t_ill, 0, false, { Zz, Zz, Zz }, 0 },
    { "unpcklpd", t_done, 0, true, { Vpd, Wq, Zz }, 0 },
    { 0, t_ill, 0, false, { Zz, Zz, Zz }, 0 },
  },
  { /* SSE15 */
    { "unpckhps", t_done, 0, true, { Vps, Wq, Zz }, 0 },
    { 0, t_ill, 0, false, { Zz, Zz, Zz }, 0 },
    { "unpckhpd", t_done, 0, true, { Vpd, Wq, Zz }, 0 },
    { 0, t_ill, 0, false, { Zz, Zz, Zz }, 0 },
  },
  { /* SSE16 */
    { "movhps/movlhps", t_done, 0, true, { Vq, Wq, Zz }, 0 }, // FIXME: wierd 2nd op
    { 0, t_ill, 0, false, { Zz, Zz, Zz }, 0 },
    { "movhpd", t_done, 0, true, { Vq, Wq, Zz }, 0 },
    { 0, t_ill, 0, false, { Zz, Zz, Zz }, 0 },
  },
  { /* SSE17 */
    { "movhps", t_done, 0, true, { Wq, Vq, Zz }, 0 },
    { 0, t_ill, 0, false, { Zz, Zz, Zz }, 0 },
    { "movhpd", t_done, 0, true, { Wq, Vq, Zz }, 0 },
    { 0, t_ill, 0, false, { Zz, Zz, Zz }, 0 },
  },
  { /* SSE28 */
    { "movaps", t_done, 0, true, { Vps, Wps, Zz }, 0 },
    { 0, t_ill, 0, false, { Zz, Zz, Zz }, 0 },
    { "movapd", t_done, 0, true, { Vpd, Wpd, Zz }, 0 },
    { 0, t_ill, 0, false, { Zz, Zz, Zz }, 0 },
  },
  { /* SSE29 */
    { "movaps", t_done, 0, true, { Wps, Vps, Zz }, 0 },
    { 0, t_ill, 0, false, { Zz, Zz, Zz }, 0 },
    { "movapd", t_done, 0, true, { Wpd, Vpd, Zz }, 0 },
    { 0, t_ill, 0, false, { Zz, Zz, Zz }, 0 },
  },
  { /* SSE2A */
    { "cvtpi2ps", t_done, 0, true, { Vps, Qq, Zz }, 0 },
    { "cvtsi2ss", t_done, 0, true, { Vss, Ed, Zz }, 0 },
    { "cvtpi2pd", t_done, 0, true, { Vpd, Qdq, Zz }, 0 },
    { "cvtsi2sd", t_done, 0, true, { Vsd, Ed, Zz }, 0 },
  },
  { /* SSE2B */
    { "movntps", t_done, 0, true, { Wps, Vps, Zz }, 0 },
    { 0, t_ill, 0, false, { Zz, Zz, Zz }, 0 },
    { "movntps", t_done, 0, true, { Wpd, Vpd, Zz }, 0 }, // should be movntpd ???
    { 0, t_ill, 0, false, { Zz, Zz, Zz }, 0 },
  },
  { /* SSE2C */
    { "cvttps2pi", t_done, 0, true, { Qq, Wps, Zz }, 0 },
    { "cvttss2si", t_done, 0, true, { Gd, Wss, Zz }, 0 },
    { "cvttpd2pi", t_done, 0, true, { Qdq, Wpd, Zz }, 0 },
    { "cvttsd2si", t_done, 0, true, { Gd, Wsd, Zz }, 0 },
  },
  { /* SSE2D */
    { "cvtps2pi", t_done, 0, true, { Qq, Wps, Zz }, 0 },
    { "cvtss2si", t_done, 0, true, { Gd, Wss, Zz }, 0 },
    { "cvtpd2pi", t_done, 0, true, { Qdq, Wpd, Zz }, 0 },
    { "cvtsd2si", t_done, 0, true, { Gd, Wsd, Zz }, 0 },
  },
  { /* SSE2E */
    { "ucomiss", t_done, 0, true, { Vss, Wss, Zz }, 0 },
    { 0, t_ill, 0, false, { Zz, Zz, Zz }, 0 },
    { "ucomisd", t_done, 0, true, { Vsd, Wsd, Zz }, 0 },
    { 0, t_ill, 0, false, { Zz, Zz, Zz }, 0 },
  },
  { /* SSE2F */
    { "comiss", t_done, 0, true, { Vps, Wps, Zz }, 0 },
    { 0, t_ill, 0, false, { Zz, Zz, Zz }, 0 },
    { "comisd", t_done, 0, true, { Vsd, Wsd, Zz }, 0 },
    { 0, t_ill, 0, false, { Zz, Zz, Zz }, 0 },
  },
  { /* SSE50 */
    { "movmskps", t_done, 0, true, { Ed, Vps, Zz }, 0 },
    { 0, t_ill, 0, false, { Zz, Zz, Zz }, 0 },
    { "movmskpd", t_done, 0, true, { Ed, Vpd, Zz }, 0 },
    { 0, t_ill, 0, false, { Zz, Zz, Zz }, 0 },
  },
  { /* SSE51 */
    { "sqrtps", t_done, 0, true, { Vps, Wps, Zz }, 0 },
    { "sqrtss", t_done, 0, true, { Vss, Wss, Zz }, 0 },
    { "sqrtpd", t_done, 0, true, { Vpd, Wpd, Zz }, 0 },
    { "sqrtsd", t_done, 0, true, { Vsd, Wsd, Zz }, 0 },
  },
  { /* SSE52 */
    { "rsqrtps", t_done, 0, true, { Vps, Wps, Zz }, 0 },
    { 0, t_ill, 0, false, { Zz, Zz, Zz }, 0 },
    { "rsqrtss", t_done, 0, true, { Vss, Wss, Zz }, 0 },
    { 0, t_ill, 0, false, { Zz, Zz, Zz }, 0 },
  },
  { /* SSE53 */
    { "rcpps", t_done, 0, true, { Vps, Wps, Zz }, 0 },
    { 0, t_ill, 0, false, { Zz, Zz, Zz }, 0 },
    { "rcpss", t_done, 0, true, { Vss, Wss, Zz }, 0 },
    { 0, t_ill, 0, false, { Zz, Zz, Zz }, 0 },
  },
  { /* SSE54 */
    { "andps", t_done, 0, true, { Vps, Wps, Zz }, 0 },
    { 0, t_ill, 0, false, { Zz, Zz, Zz }, 0 },
    { "andpd", t_done, 0, true, { Vpd, Wpd, Zz }, 0 },
    { 0, t_ill, 0, false, { Zz, Zz, Zz }, 0 },
  },
  { /* SSE55 */
    { "andnps", t_done, 0, true, { Vps, Wps, Zz }, 0 },
    { 0, t_ill, 0, false, { Zz, Zz, Zz }, 0 },
    { "andnpd", t_done, 0, true, { Vpd, Wpd, Zz }, 0 },
    { 0, t_ill, 0, false, { Zz, Zz, Zz }, 0 },
  },
  { /* SSE56 */
    { "orps", t_done, 0, true, { Vps, Wps, Zz }, 0 },
    { 0, t_ill, 0, false, { Zz, Zz, Zz }, 0 },
    { "orpd", t_done, 0, true, { Vpd, Wpd, Zz }, 0 },
    { 0, t_ill, 0, false, { Zz, Zz, Zz }, 0 },
  },
  { /* SSE57 */
    { "xorps", t_done, 0, true, { Vps, Wps, Zz }, 0 },
    { 0, t_ill, 0, false, { Zz, Zz, Zz }, 0 },
    { "xorpd", t_done, 0, true, { Vpd, Wpd, Zz }, 0 },
    { 0, t_ill, 0, false, { Zz, Zz, Zz }, 0 },
  },
  { /* SSE58 */
    { "addps", t_done, 0, true, { Vps, Wps, Zz }, 0 },
    { "addss", t_done, 0, true, { Vss, Wss, Zz }, 0 },
    { "addpd", t_done, 0, true, { Vpd, Wpd, Zz }, 0 },
    { "addsd", t_done, 0, true, { Vsd, Wsd, Zz }, 0 },
  },
  { /* SSE59 */
    { "mulps", t_done, 0, true, { Vps, Wps, Zz }, 0 },
    { "mulss", t_done, 0, true, { Vss, Wss, Zz }, 0 },
    { "mulpd", t_done, 0, true, { Vpd, Wpd, Zz }, 0 },
    { "mulsd", t_done, 0, true, { Vsd, Wsd, Zz }, 0 },
  },
  { /* SSE5A */
    { "cvtps2pd", t_done, 0, true, { Vpd, Wps, Zz }, 0 },
    { "cvtss2sd", t_done, 0, true, { Vss, Wss, Zz }, 0 },
    { "cvtpd2ps", t_done, 0, true, { Vps, Wpd, Zz }, 0 }, // FIXME: book bug ???
    { "cvtsd2ss", t_done, 0, true, { Vsd, Wsd, Zz }, 0 },
  },
  { /* SSE5B */
    { "cvtdq2ps", t_done, 0, true, { Vps, Wdq, Zz }, 0 },
    { "cvttps2dq", t_done, 0, true, { Vdq, Wps, Zz }, 0 }, // book has this and next swapped!!! 
    { "cvtps2dq", t_done, 0, true, { Vdq, Wps, Zz }, 0 },  // FIXME: book bug ???
    { 0, t_ill, 0, false, { Zz, Zz, Zz }, 0 },
  },
  { /* SSE5C */
    { "subps", t_done, 0, true, { Vps, Wps, Zz }, 0 },
    { "subss", t_done, 0, true, { Vss, Wss, Zz }, 0 },
    { "subpd", t_done, 0, true, { Vpd, Wpd, Zz }, 0 },
    { "subsd", t_done, 0, true, { Vsd, Wsd, Zz }, 0 },
  },
  { /* SSE5D */
    { "minps", t_done, 0, true, { Vps, Wps, Zz }, 0 },
    { "minss", t_done, 0, true, { Vss, Wss, Zz }, 0 },
    { "minpd", t_done, 0, true, { Vpd, Wpd, Zz }, 0 },
    { "minsd", t_done, 0, true, { Vsd, Wsd, Zz }, 0 },
  },
  { /* SSE5E */
    { "divps", t_done, 0, true, { Vps, Wps, Zz }, 0 },
    { "divss", t_done, 0, true, { Vss, Wss, Zz }, 0 },
    { "divpd", t_done, 0, true, { Vpd, Wpd, Zz }, 0 },
    { "divsd", t_done, 0, true, { Vsd, Wsd, Zz }, 0 },
  },
  { /* SSE5F */
    { "maxps", t_done, 0, true, { Vps, Wps, Zz }, 0 },
    { "maxss", t_done, 0, true, { Vss, Wss, Zz }, 0 },
    { "maxpd", t_done, 0, true, { Vpd, Wpd, Zz }, 0 },
    { "maxsd", t_done, 0, true, { Vsd, Wsd, Zz }, 0 },
  },
  { /* SSE60 */
    { "punpcklbw", t_done, 0, true, { Pq, Qd, Zz }, 0 },
    { 0, t_ill, 0, false, { Zz, Zz, Zz }, 0 },
    { "punpcklbw", t_done, 0, true, { Vdq, Wdq, Zz }, 0 },
    { 0, t_ill, 0, false, { Zz, Zz, Zz }, 0 },
  },
  { /* SSE61 */
    { "punpcklwd", t_done, 0, true, { Pq, Qd, Zz }, 0 },
    { 0, t_ill, 0, false, { Zz, Zz, Zz }, 0 },
    { "punpcklwd", t_done, 0, true, { Vdq, Wdq, Zz }, 0 },
    { 0, t_ill, 0, false, { Zz, Zz, Zz }, 0 },
  },
  { /* SSE62 */
    { "punpcklqd", t_done, 0, true, { Pq, Qd, Zz }, 0 },
    { 0, t_ill, 0, false, { Zz, Zz, Zz }, 0 },
    { "punpcklqd", t_done, 0, true, { Vdq, Wdq, Zz }, 0 },
    { 0, t_ill, 0, false, { Zz, Zz, Zz }, 0 },
  },
  { /* SSE63 */
    { "packsswb", t_done, 0, true, { Pq, Qq, Zz }, 0 },
    { 0, t_ill, 0, false, { Zz, Zz, Zz }, 0 },
    { "packsswb", t_done, 0, true, { Vdq, Wdq, Zz }, 0 },
    { 0, t_ill, 0, false, { Zz, Zz, Zz }, 0 },
  },
  { /* SSE64 */
    { "pcmpgtb", t_done, 0, true, { Pq, Qq, Zz }, 0 },
    { 0, t_ill, 0, false, { Zz, Zz, Zz }, 0 },
    { "pcmpgtb", t_done, 0, true, { Vdq, Wdq, Zz }, 0 },
    { 0, t_ill, 0, false, { Zz, Zz, Zz }, 0 },
  },
  { /* SSE65 */
    { "pcmpgtw", t_done, 0, true, { Pq, Qq, Zz }, 0 },
    { 0, t_ill, 0, false, { Zz, Zz, Zz }, 0 },
    { "pcmpgtw", t_done, 0, true, { Vdq, Wdq, Zz }, 0 },
    { 0, t_ill, 0, false, { Zz, Zz, Zz }, 0 },
  },
  { /* SSE66 */
    { "pcmpgdt", t_done, 0, true, { Pq, Qq, Zz }, 0 },
    { 0, t_ill, 0, false, { Zz, Zz, Zz }, 0 },
    { "pcmpgdt", t_done, 0, true, { Vdq, Wdq, Zz }, 0 },
    { 0, t_ill, 0, false, { Zz, Zz, Zz }, 0 },
  },
  { /* SSE67 */
    { "packuswb", t_done, 0, true, { Pq, Qq, Zz }, 0 },
    { 0, t_ill, 0, false, { Zz, Zz, Zz }, 0 },
    { "packuswb", t_done, 0, true, { Vdq, Wdq, Zz }, 0 },
    { 0, t_ill, 0, false, { Zz, Zz, Zz }, 0 },
  },
  { /* SSE68 */
    { "punpckhbw", t_done, 0, true, { Pq, Qq, Zz }, 0 },
    { 0, t_ill, 0, false, { Zz, Zz, Zz }, 0 },
    { "punpckhbw", t_done, 0, true, { Pdq, Qdq, Zz }, 0 },
    { 0, t_ill, 0, false, { Zz, Zz, Zz }, 0 },
  },
  { /* SSE69 */
    { "punpckhwd", t_done, 0, true, { Pq, Qq, Zz }, 0 },
    { 0, t_ill, 0, false, { Zz, Zz, Zz }, 0 },
    { "punpckhwd", t_done, 0, true, { Pdq, Qdq, Zz }, 0 },
    { 0, t_ill, 0, false, { Zz, Zz, Zz }, 0 },
  },
  { /* SSE6A */
    { "punpckhdq", t_done, 0, true, { Pq, Qq, Zz }, 0 },
    { 0, t_ill, 0, false, { Zz, Zz, Zz }, 0 },
    { "punpckhdq", t_done, 0, true, { Pdq, Qdq, Zz }, 0 },
    { 0, t_ill, 0, false, { Zz, Zz, Zz }, 0 },
  },
  { /* SSE6B */
    { "packssdw", t_done, 0, true, { Pq, Qq, Zz }, 0 },
    { 0, t_ill, 0, false, { Zz, Zz, Zz }, 0 },
    { "packssdw", t_done, 0, true, { Pdq, Qdq, Zz }, 0 },
    { 0, t_ill, 0, false, { Zz, Zz, Zz }, 0 },
  },
  { /* SSE6C */
    { 0, t_ill, 0, false, { Zz, Zz, Zz }, 0 },
    { 0, t_ill, 0, false, { Zz, Zz, Zz }, 0 },
    { "punpcklqld", t_done, 0, true, { Vdq, Wdq, Zz }, 0 },
    { 0, t_ill, 0, false, { Zz, Zz, Zz }, 0 },
  },
  { /* SSE6D */
    { 0, t_ill, 0, false, { Zz, Zz, Zz }, 0 },
    { 0, t_ill, 0, false, { Zz, Zz, Zz }, 0 },
    { "punpckhqd", t_done, 0, true, { Vdq, Wdq, Zz }, 0 },
    { 0, t_ill, 0, false, { Zz, Zz, Zz }, 0 },
  },
  { /* SSE6E */
    { "movd", t_done, 0, true, { Pd, Ed, Zz }, 0 },
    { 0, t_ill, 0, false, { Zz, Zz, Zz }, 0 },
    { "movd", t_done, 0, true, { Vdq, Ed, Zz }, 0 },
    { 0, t_ill, 0, false, { Zz, Zz, Zz }, 0 },
  },
  { /* SSE6F */
    { "movq", t_done, 0, true, { Pq, Qq, Zz }, 0 },
    { "movdqu", t_done, 0, false, { Vdq, Wdq, Zz }, 0 }, // book has this and next swapped!!!
    { "movdqa", t_done, 0, true, { Vdq, Wdq, Zz }, 0 },
    { 0, t_ill, 0, false, { Zz, Zz, Zz }, 0 },
  },
  { /* SSE70 */
    { "pshufw", t_done, 0, true, { Pq, Qq, Ib }, 0 },
    { "pshufhw", t_done, 0, true, { Vdq, Wdq, Ib }, 0 }, // book has this and next swapped!!!
    { "pshufd", t_done, 0, true, { Vdq, Wdq, Ib }, 0 },
    { "pshuflw", t_done, 0, true, { Vdq, Wdq, Ib }, 0 },
  },
  { /* SSE74 */
    { "pcmpeqb", t_done, 0, true, { Pq, Qq, Zz }, 0 },
    { 0, t_ill, 0, false, { Zz, Zz, Zz }, 0 },
    { "pcmpeqb", t_done, 0, true, { Vdq, Wdq, Zz }, 0 },
    { 0, t_ill, 0, false, { Zz, Zz, Zz }, 0 },
  },
  { /* SSE75 */
    { "pcmpeqw", t_done, 0, true, { Pq, Qq, Zz }, 0 },
    { 0, t_ill, 0, false, { Zz, Zz, Zz }, 0 },
    { "pcmpeqw", t_done, 0, true, { Vdq, Wdq, Zz }, 0 },
    { 0, t_ill, 0, false, { Zz, Zz, Zz }, 0 },
  },
  { /* SSE76 */
    { "pcmpeqd", t_done, 0, true, { Pq, Qq, Zz }, 0 },
    { 0, t_ill, 0, false, { Zz, Zz, Zz }, 0 },
    { "pcmpeqd", t_done, 0, true, { Vdq, Wdq, Zz }, 0 },
    { 0, t_ill, 0, false, { Zz, Zz, Zz }, 0 },
  },
  { /* SSE7E */
    { "movd", t_done, 0, true, { Ed, Pd, Zz }, 0 },
    { "movq", t_done, 0, true, { Vq, Wq, Zz }, 0 }, // book has this and next swapped!!!
    { "movd", t_done, 0, true, { Ed, Vdq, Zz }, 0 },
    { 0, t_ill, 0, false, { Zz, Zz, Zz }, 0 },
  },
  { /* SSE7F */
    { "movq", t_done, 0, true, { Qq, Pq, Zz }, 0 },
    { "movdqu", t_done, 0, true, { Wdq, Vdq, Zz }, 0 }, // book has this and next swapped!!!
    { "movdqa", t_done, 0, true, { Wdq, Vdq, Zz }, 0 },
    { 0, t_ill, 0, false, { Zz, Zz, Zz }, 0 },
  },
  { /* SSEC2 */
    { "cmpps", t_done, 0, true, { Vps, Wps, Ib }, 0 },
    { "cmpss", t_done, 0, true, { Vss, Wss, Ib }, 0 },
    { "cmppd", t_done, 0, true, { Vpd, Wpd, Ib }, 0 },
    { "cmpsd", t_done, 0, true, { Vsd, Wsd, Ib }, 0 },
  },
  { /* SSEC4 */
    { "pinsrw", t_done, 0, true, { Pq, Ed, Ib }, 0 },
    { 0, t_ill, 0, false, { Zz, Zz, Zz }, 0 },
    { "pinsrw", t_done, 0, true, { Vdq, Ed, Ib }, 0 },
    { 0, t_ill, 0, false, { Zz, Zz, Zz }, 0 },
  },
  { /* SSEC5 */
    { "pextrw", t_done, 0, true, { Gd, Pq, Ib }, 0 },
    { 0, t_ill, 0, false, { Zz, Zz, Zz }, 0 },
    { "pextrw", t_done, 0, true, { Gd, Vdq, Ib }, 0 },
    { 0, t_ill, 0, false, { Zz, Zz, Zz }, 0 },
  },
  { /* SSEC6 */
    { "shufps", t_done, 0, true, { Vps, Wps, Ib }, 0 },
    { 0, t_ill, 0, false, { Zz, Zz, Zz }, 0 },
    { "shufpd", t_done, 0, true, { Vpd, Wpd, Ib }, 0 },
    { 0, t_ill, 0, false, { Zz, Zz, Zz }, 0 },
  },
  { /* SSED1 */
    { "psrlw", t_done, 0, true, { Pq, Qq, Zz }, 0 },
    { 0, t_ill, 0, false, { Zz, Zz, Zz }, 0 },
    { "psrlw", t_done, 0, true, { Vdq, Wdq, Zz }, 0 },
    { 0, t_ill, 0, false, { Zz, Zz, Zz }, 0 },
  },
  { /* SSED2 */
    { "psrld", t_done, 0, true, { Pq, Qq, Zz }, 0 },
    { 0, t_ill, 0, false, { Zz, Zz, Zz }, 0 },
    { "psrld", t_done, 0, true, { Vdq, Wdq, Zz }, 0 },
    { 0, t_ill, 0, false, { Zz, Zz, Zz }, 0 },
  },
  { /* SSED3 */
    { "psrlq", t_done, 0, true, { Pq, Qq, Zz }, 0 },
    { 0, t_ill, 0, false, { Zz, Zz, Zz }, 0 },
    { "psrlq", t_done, 0, true, { Vdq, Wdq, Zz }, 0 },
    { 0, t_ill, 0, false, { Zz, Zz, Zz }, 0 },
  },
  { /* SSED4 */
    { "paddq", t_done, 0, true, { Pq, Qq, Zz }, 0 },
    { 0, t_ill, 0, false, { Zz, Zz, Zz }, 0 },
    { "psrlq", t_done, 0, true, { Vdq, Wdq, Zz }, 0 },
    { 0, t_ill, 0, false, { Zz, Zz, Zz }, 0 },
  },
  { /* SSED5 */
    { "pmullw", t_done, 0, true, { Pq, Qq, Zz }, 0 },
    { 0, t_ill, 0, false, { Zz, Zz, Zz }, 0 },
    { "pmullw", t_done, 0, true, { Vdq, Wdq, Zz }, 0 },
    { 0, t_ill, 0, false, { Zz, Zz, Zz }, 0 },
  },
  { /* SSED6 */
    { 0, t_ill, 0, false, { Zz, Zz, Zz }, 0 },
    { "movq2dq", t_done, 0, true, { Vdq, Qq, Zz }, 0 }, // lines jumbled in book
    { "movq", t_done, 0, true, { Wq, Vq, Zz }, 0 },
    { "movdq2q", t_done, 0, true, { Pq, Wq, Zz }, 0 },
  },
  { /* SSED7 */
    { "pmovmskb", t_done, 0, true, { Gd, Pq, Zz }, 0 },
    { 0, t_ill, 0, false, { Zz, Zz, Zz }, 0 },
    { "pmovmskb", t_done, 0, true, { Gd, Vdq, Zz }, 0 },
    { 0, t_ill, 0, false, { Zz, Zz, Zz }, 0 },
  },
  { /* SSED8 */
    { "psubusb", t_done, 0, true, { Pq, Qq, Zz }, 0 },
    { 0, t_ill, 0, false, { Zz, Zz, Zz }, 0 },
    { "psubusb", t_done, 0, true, { Vdq, Wdq, Zz }, 0 },
    { 0, t_ill, 0, false, { Zz, Zz, Zz }, 0 },
  },
  { /* SSED9 */
    { "psubusw", t_done, 0, true, { Pq, Qq, Zz }, 0 },
    { 0, t_ill, 0, false, { Zz, Zz, Zz }, 0 },
    { "psubusw", t_done, 0, true, { Vdq, Wdq, Zz }, 0 },
    { 0, t_ill, 0, false, { Zz, Zz, Zz }, 0 },
  },
  { /* SSEDA */
    { "pminub", t_done, 0, true, { Pq, Qq, Zz }, 0 },
    { 0, t_ill, 0, false, { Zz, Zz, Zz }, 0 },
    { "pminub", t_done, 0, true, { Vdq, Wdq, Zz }, 0 },
    { 0, t_ill, 0, false, { Zz, Zz, Zz }, 0 },
  },
  { /* SSEDB */
    { "pand", t_done, 0, true, { Pq, Qq, Zz }, 0 },
    { 0, t_ill, 0, false, { Zz, Zz, Zz }, 0 },
    { "pand", t_done, 0, true, { Vdq, Wdq, Zz }, 0 },
    { 0, t_ill, 0, false, { Zz, Zz, Zz }, 0 },
  },
  { /* SSEDC */
    { "paddusb", t_done, 0, true, { Pq, Qq, Zz }, 0 },
    { 0, t_ill, 0, false, { Zz, Zz, Zz }, 0 },
    { "paddusb", t_done, 0, true, { Vdq, Wdq, Zz }, 0 },
    { 0, t_ill, 0, false, { Zz, Zz, Zz }, 0 },
  },
  { /* SSEDD */
    { "paddusw", t_done, 0, true, { Pq, Qq, Zz }, 0 },
    { 0, t_ill, 0, false, { Zz, Zz, Zz }, 0 },
    { "paddusw", t_done, 0, true, { Vdq, Wdq, Zz }, 0 },
    { 0, t_ill, 0, false, { Zz, Zz, Zz }, 0 },
  },
  { /* SSEDE */
    { "pmaxub", t_done, 0, true, { Pq, Qq, Zz }, 0 },
    { 0, t_ill, 0, false, { Zz, Zz, Zz }, 0 },
    { "pmaxub", t_done, 0, true, { Vdq, Wdq, Zz }, 0 },
    { 0, t_ill, 0, false, { Zz, Zz, Zz }, 0 },
  },
  { /* SSEDF */
    { "pandn", t_done, 0, true, { Pq, Qq, Zz }, 0 },
    { 0, t_ill, 0, false, { Zz, Zz, Zz }, 0 },
    { "pandn", t_done, 0, true, { Vdq, Wdq, Zz }, 0 },
    { 0, t_ill, 0, false, { Zz, Zz, Zz }, 0 },
  },
  { /* SSEE0 */
    { "pavgb", t_done, 0, true, { Pq, Qq, Zz }, 0 },
    { 0, t_ill, 0, false, { Zz, Zz, Zz }, 0 },
    { "pavgb", t_done, 0, true, { Vdq, Wdq, Zz }, 0 },
    { 0, t_ill, 0, false, { Zz, Zz, Zz }, 0 },
  },
  { /* SSEE1 */
    { "psraw", t_done, 0, true, { Pq, Qq, Zz }, 0 },
    { 0, t_ill, 0, false, { Zz, Zz, Zz }, 0 },
    { "psraw", t_done, 0, true, { Vdq, Wdq, Zz }, 0 },
    { 0, t_ill, 0, false, { Zz, Zz, Zz }, 0 },
  },
  { /* SSEE2 */
    { "psrad", t_done, 0, true, { Pq, Qq, Zz }, 0 },
    { 0, t_ill, 0, false, { Zz, Zz, Zz }, 0 },
    { "psrad", t_done, 0, true, { Vdq, Wdq, Zz }, 0 },
    { 0, t_ill, 0, false, { Zz, Zz, Zz }, 0 },
  },
  { /* SSEE3 */
    { "pavgw", t_done, 0, true, { Pq, Qq, Zz }, 0 },
    { 0, t_ill, 0, false, { Zz, Zz, Zz }, 0 },
    { "pavgw", t_done, 0, true, { Vdq, Wdq, Zz }, 0 },
    { 0, t_ill, 0, false, { Zz, Zz, Zz }, 0 },
  },
  { /* SSEE4 */
    { "pmulhuw", t_done, 0, true, { Pq, Qq, Zz }, 0 },
    { 0, t_ill, 0, false, { Zz, Zz, Zz }, 0 },
    { "pmulhuw", t_done, 0, true, { Vdq, Wdq, Zz }, 0 },
    { 0, t_ill, 0, false, { Zz, Zz, Zz }, 0 },
  },
  { /* SSEE5 */
    { "pmulhw", t_done, 0, true, { Pq, Qq, Zz }, 0 },
    { 0, t_ill, 0, false, { Zz, Zz, Zz }, 0 },
    { "pmulhw", t_done, 0, true, { Vdq, Wdq, Zz }, 0 },
    { 0, t_ill, 0, false, { Zz, Zz, Zz }, 0 },
  },
  { /* SSEE6 */
    { 0, t_ill, 0, false, { Zz, Zz, Zz }, 0 },
    { "cvtdq2pd", t_done, 0, true, { Vpd, Wdq, Zz }, 0 }, // lines jumbled in book
    { "cvttpd2dq", t_done, 0, true, { Vdq, Wpd, Zz }, 0 },
    { "cvtpd2dq", t_done, 0, true, { Vdq, Wpd, Zz }, 0 },
  },
  { /* SSEE7 */
    { "movntq", t_done, 0, true, { Wq, Vq, Zz }, 0 },
    { 0, t_ill, 0, false, { Zz, Zz, Zz }, 0 },
    { "movntdq", t_done, 0, true, { Wdq, Vdq, Zz }, 0 },
    { 0, t_ill, 0, false, { Zz, Zz, Zz }, 0 },
  },
  { /* SSEE8 */
    { "psubsb", t_done, 0, true, { Pq, Qq, Zz }, 0 },
    { 0, t_ill, 0, false, { Zz, Zz, Zz }, 0 },
    { "psubsb", t_done, 0, true, { Vdq, Wdq, Zz }, 0 },
    { 0, t_ill, 0, false, { Zz, Zz, Zz }, 0 },
  },
  { /* SSEE9 */
    { "psubsw", t_done, 0, true, { Pq, Qq, Zz }, 0 },
    { 0, t_ill, 0, false, { Zz, Zz, Zz }, 0 },
    { "psubsw", t_done, 0, true, { Vdq, Wdq, Zz }, 0 },
    { 0, t_ill, 0, false, { Zz, Zz, Zz }, 0 },
  },
  { /* SSEEA */
    { "pminsw", t_done, 0, true, { Pq, Qq, Zz }, 0 },
    { 0, t_ill, 0, false, { Zz, Zz, Zz }, 0 },
    { "pminsw", t_done, 0, true, { Vdq, Wdq, Zz }, 0 },
    { 0, t_ill, 0, false, { Zz, Zz, Zz }, 0 },
  },
  { /* SSEEB */
    { "por", t_done, 0, true, { Pq, Qq, Zz }, 0 },
    { 0, t_ill, 0, false, { Zz, Zz, Zz }, 0 },
    { "por", t_done, 0, true, { Vdq, Wdq, Zz }, 0 },
    { 0, t_ill, 0, false, { Zz, Zz, Zz }, 0 },
  },
  { /* SSEEC */
    { "paddsb", t_done, 0, true, { Pq, Qq, Zz }, 0 },
    { 0, t_ill, 0, false, { Zz, Zz, Zz }, 0 },
    { "paddsb", t_done, 0, true, { Vdq, Wdq, Zz }, 0 },
    { 0, t_ill, 0, false, { Zz, Zz, Zz }, 0 },
  },
  { /* SSEED */
    { "paddsw", t_done, 0, true, { Pq, Qq, Zz }, 0 },
    { 0, t_ill, 0, false, { Zz, Zz, Zz }, 0 },
    { "paddsw", t_done, 0, true, { Vdq, Wdq, Zz }, 0 },
    { 0, t_ill, 0, false, { Zz, Zz, Zz }, 0 },
  },
  { /* SSEEE */
    { "pmaxsw", t_done, 0, true, { Pq, Qq, Zz }, 0 },
    { 0, t_ill, 0, false, { Zz, Zz, Zz }, 0 },
    { "pmaxsw", t_done, 0, true, { Vdq, Wdq, Zz }, 0 },
    { 0, t_ill, 0, false, { Zz, Zz, Zz }, 0 },
  },
  { /* SSEEF */
    { "pxor", t_done, 0, true, { Pq, Qq, Zz }, 0 },
    { 0, t_ill, 0, false, { Zz, Zz, Zz }, 0 },
    { "pxor", t_done, 0, true, { Vdq, Wdq, Zz }, 0 },
    { 0, t_ill, 0, false, { Zz, Zz, Zz }, 0 },
  },
  { /* SSEF1 */
    { "psllw", t_done, 0, true, { Pq, Qq, Zz }, 0 },
    { 0, t_ill, 0, false, { Zz, Zz, Zz }, 0 },
    { "psllw", t_done, 0, true, { Vdq, Wdq, Zz }, 0 },
    { 0, t_ill, 0, false, { Zz, Zz, Zz }, 0 },
  },
  { /* SSEF2 */
    { "pslld", t_done, 0, true, { Pq, Qq, Zz }, 0 },
    { 0, t_ill, 0, false, { Zz, Zz, Zz }, 0 },
    { "pslld", t_done, 0, true, { Vdq, Wdq, Zz }, 0 },
    { 0, t_ill, 0, false, { Zz, Zz, Zz }, 0 },
  },
  { /* SSEF3 */
    { "psllq", t_done, 0, true, { Pq, Qq, Zz }, 0 },
    { 0, t_ill, 0, false, { Zz, Zz, Zz }, 0 },
    { "psllq", t_done, 0, true, { Vdq, Wdq, Zz }, 0 },
    { 0, t_ill, 0, false, { Zz, Zz, Zz }, 0 },
  },
  { /* SSEF4 */
    { "pmuludq", t_done, 0, true, { Pq, Qq, Zz }, 0 },
    { 0, t_ill, 0, false, { Zz, Zz, Zz }, 0 },
    { "pmuludq", t_done, 0, true, { Vdq, Wdq, Zz }, 0 },
    { 0, t_ill, 0, false, { Zz, Zz, Zz }, 0 },
  },
  { /* SSEF5 */
    { "pmaddwd", t_done, 0, true, { Pq, Qq, Zz }, 0 },
    { 0, t_ill, 0, false, { Zz, Zz, Zz }, 0 },
    { "pmaddwd", t_done, 0, true, { Vdq, Wdq, Zz }, 0 },
    { 0, t_ill, 0, false, { Zz, Zz, Zz }, 0 },
  },
  { /* SSEF6 */
    { "psadbw", t_done, 0, true, { Pq, Qq, Zz }, 0 },
    { 0, t_ill, 0, false, { Zz, Zz, Zz }, 0 },
    { "psadbw", t_done, 0, true, { Vdq, Wdq, Zz }, 0 },
    { 0, t_ill, 0, false, { Zz, Zz, Zz }, 0 },
  },
  { /* SSEF7 */
    { "maskmovq", t_done, 0, true, { Ppi, Qpi, Zz }, 0 },
    { 0, t_ill, 0, false, { Zz, Zz, Zz }, 0 },
    { "maskmovqu", t_done, 0, true, { Vdq, Wdq, Zz }, 0 },
    { 0, t_ill, 0, false, { Zz, Zz, Zz }, 0 },
  },
  { /* SSEF8 */
    { "psubb", t_done, 0, true, { Pq, Qq, Zz }, 0 },
    { 0, t_ill, 0, false, { Zz, Zz, Zz }, 0 },
    { "psubb", t_done, 0, true, { Vdq, Wdq, Zz }, 0 },
    { 0, t_ill, 0, false, { Zz, Zz, Zz }, 0 },
  },
  { /* SSEF9 */
    { "psubw", t_done, 0, true, { Pq, Qq, Zz }, 0 },
    { 0, t_ill, 0, false, { Zz, Zz, Zz }, 0 },
    { "psubw", t_done, 0, true, { Vdq, Wdq, Zz }, 0 },
    { 0, t_ill, 0, false, { Zz, Zz, Zz }, 0 },
  },
  { /* SSEFA */
    { "psubd", t_done, 0, true, { Pq, Qq, Zz }, 0 },
    { 0, t_ill, 0, false, { Zz, Zz, Zz }, 0 },
    { "psubd", t_done, 0, true, { Vdq, Wdq, Zz }, 0 },
    { 0, t_ill, 0, false, { Zz, Zz, Zz }, 0 },
  },
  { /* SSEFB */ // FIXME: Same????
    { "psubd", t_done, 0, true, { Pq, Qq, Zz }, 0 },
    { 0, t_ill, 0, false, { Zz, Zz, Zz }, 0 },
    { "psubd", t_done, 0, true, { Vdq, Wdq, Zz }, 0 },
    { 0, t_ill, 0, false, { Zz, Zz, Zz }, 0 },
  },
  { /* SSEFC */
    { "paddb", t_done, 0, true, { Pq, Qq, Zz }, 0 },
    { 0, t_ill, 0, false, { Zz, Zz, Zz }, 0 },
    { "paddb", t_done, 0, true, { Vdq, Wdq, Zz }, 0 },
    { 0, t_ill, 0, false, { Zz, Zz, Zz }, 0 },
  },
  { /* SSEFD */
    { "paddw", t_done, 0, true, { Pq, Qq, Zz }, 0 },
    { 0, t_ill, 0, false, { Zz, Zz, Zz }, 0 },
    { "paddw", t_done, 0, true, { Vdq, Wdq, Zz }, 0 },
    { 0, t_ill, 0, false, { Zz, Zz, Zz }, 0 },
  },
  { /* SSEFE */
    { "paddd", t_done, 0, true, { Pq, Qq, Zz }, 0 },
    { 0, t_ill, 0, false, { Zz, Zz, Zz }, 0 },
    { "paddd", t_done, 0, true, { Vdq, Wdq, Zz }, 0 },
    { 0, t_ill, 0, false, { Zz, Zz, Zz }, 0 },
  }
};

/* rows are none or 66 prefixed in this order (see book) */
static ia32_entry ssegrpMap[][2] = {
  /* G12SSE010B */
  {
    { "psrlw", t_done, 0, true, { Pq, Ib, Zz }, 0 },
    { "psrlw", t_done, 0, true, { Pdq, Ib, Zz }, 0 }
  },
  /* G12SSE100B */
  {
    { "psraw", t_done, 0, true, { Pq, Ib, Zz }, 0 },
    { "psraw", t_done, 0, true, { Pdq, Ib, Zz }, 0 }
  },
  /* G12SSE110B */
  {
    { "psllw", t_done, 0, true, { Pq, Ib, Zz }, 0 },
    { "psllw", t_done, 0, true, { Pdq, Ib, Zz }, 0 }
  },
  /* G13SSE010B */
  {
    { "psrld", t_done, 0, true, { Pq, Ib, Zz }, 0 },
    { "psrld", t_done, 0, true, { Wdq, Ib, Zz }, 0 }
  },
  /* G13SSE100B */
  {
    { "psrad", t_done, 0, true, { Pq, Ib, Zz }, 0 },
    { "psrad", t_done, 0, true, { Wdq, Ib, Zz }, 0 }
  },
  /* G13SSE110B */
  {
    { "pslld", t_done, 0, true, { Pq, Ib, Zz }, 0 },
    { "pslld", t_done, 0, true, { Wdq, Ib, Zz }, 0 }
  },
  /* G14SSE010B */
  {
    { "psrlq", t_done, 0, true, { Pq, Ib, Zz }, 0 },
    { "psrlq", t_done, 0, true, { Wdq, Ib, Zz }, 0 }
  },
  /* G14SSE011B */
  {
    { 0, t_ill, 0, true, { Zz, Zz, Zz }, 0 },
    { "psrldq", t_done, 0, true, { Wdq, Ib, Zz }, 0 }
  },
  /* G14SSE110B */
  {
    { "psllq", t_done, 0, true, { Pq, Ib, Zz }, 0 },
    { "psllq", t_done, 0, true, { Wdq, Ib, Zz }, 0 }
  },
  /* G14SSE111B */
  {
    { 0, t_ill, 0, true, { Zz, Zz, Zz }, 0 },
    { "pslldq", t_done, 0, true, { Wdq, Ib, Zz }, 0 }
  }
};

static unsigned int ia32_decode_modrm(const unsigned int addrSzAttr, const unsigned char* addr);


template <unsigned int capa>
ia32_instruction& ia32_decode(const unsigned char* addr, ia32_instruction& instruct)
{
  ia32_prefixes& pref = instruct.prf;
  unsigned int table, nxtab;
  unsigned int idx, sseidx = 0;
  ia32_entry *gotit = NULL;

  ia32_decode_prefixes(addr, pref);
  instruct.size = pref.getCount();
  addr += instruct.size;

  table = t_oneB;
  idx = addr[0];
  gotit = &oneByteMap[idx];
  nxtab = gotit->otable;
  instruct.size += 1;
  addr += 1;

  while(nxtab != t_done) {
    table = nxtab;
    switch(table) {
    case t_twoB:
      idx = addr[0];
      gotit = &twoByteMap[idx];
      nxtab = gotit->otable;
      instruct.size += 1;
      addr += 1;
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
        case Grp1:
        case Grp2:
        case Grp8:
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
      }
      break;
    }
    case t_grpsse:
      sseidx >>= 1;
      idx = gotit->tabidx;
      gotit = &ssegrpMap[idx][sseidx];
      break;
    case t_coprocEsc:
      instruct.legacy_type = 0;
      return ia32_decode_FP(pref, addr, instruct);
    case t_3dnow:
      // 3D now opcodes are given as suffix: ModRM [SIB] [displacement] opcode
      // Right now we don't care what the actual opcode is, so there's no table
      instruct.size += 1;
      break;
    case t_ill:
      instruct.legacy_type = ILLEGAL;
      return instruct;
    default:
      assert(!"wrong table");
    }
  }

  // addr points after the opcode, and the size has been adjusted accordingly
  // also the SSE index is set; now 'table' points to the correct instruction map

  assert(gotit != NULL);
  instruct.legacy_type = gotit->legacyType;

  ia32_decode_operands(pref, *gotit, addr, instruct); // all but FP

  return instruct;
}

template ia32_instruction& ia32_decode<0>(const unsigned char* addr, ia32_instruction& instruct);

ia32_instruction& ia32_decode_FP(const ia32_prefixes& pref, const unsigned char* addr,
                                 ia32_instruction& instruct)
{
  unsigned int nib = byteSzB; // modRM
  unsigned int addrSzAttr = (pref.getPrefix(3) == PREFIX_SZADDR ? 1 : 2); // 32-bit mode implicit

  if (addr[0] <= 0xBF) { // modrm
    nib += ia32_decode_modrm(addrSzAttr, addr);
  }
  instruct.size += nib;

  return instruct;
}

static unsigned int ia32_decode_modrm(const unsigned int addrSzAttr, const unsigned char* addr)
{
  unsigned char modrm = addr[0];
  unsigned char mod = modrm >> 6;
  unsigned char rm  = modrm & 7;
  //unsigned char reg = (modrm >> 3) & 7;

  if(addrSzAttr == 1) { // 16-bit, cannot have SIB
    switch(mod) {
    case 0:
      return rm==6 ? wordSzB : 0;
    case 1:
      return byteSzB;
    case 2:
      return wordSzB;
    case 3:
      return 0; // register
    default:
      assert(0);
    }
  }
  else { // 32-bit, may have SIB
    if(mod == 3)
      return 0; // only registers, no SIB
    bool hassib = rm == 4;
    unsigned int nsib = 0;
    unsigned char sib;
    unsigned char base = 0;
    if(hassib) {
      nsib = byteSzB;
      sib = addr[1];
      base = sib & 7;
    }
    switch(mod) {
    case 0: {
      /* this is tricky: there is a disp32 iff (1) rm == 5  or  (2) rm == 4 && base == 5 */
      unsigned char check5 = hassib ? base : rm;
      return nsib + ((check5 == 5) ? dwordSzB : 0);
    }
    case 1:
      return nsib + byteSzB;
    case 2:
      return nsib + dwordSzB;
    default:
      assert(0);
    }
  }
}


unsigned int ia32_decode_operands (const ia32_prefixes& pref, const ia32_entry& gotit, 
                                   const unsigned char* addr, ia32_instruction& instruct)
{
  unsigned int nib = 0; /* # of bytes in instruction */
  unsigned int addrSzAttr = (pref.getPrefix(3) == PREFIX_SZADDR ? 1 : 2); // 32-bit mode implicit
  unsigned int operSzAttr = (pref.getPrefix(2) == PREFIX_SZOPER ? 1 : 2); // 32-bit mode implicit

  if(gotit.hasModRM)
    nib += byteSzB;

  for(unsigned int i=0; i<3; ++i) {
    const ia32_operand& op = gotit.operands[i];
    if(op.admet) {
      switch(op.admet) {
      case am_A: /* address = segment + offset (word or dword) */
        nib += wordSzB;
      case am_O: /* operand offset */
        nib += wordSzB * addrSzAttr;
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
        break;
      case am_E: /* register or memory location, so decoding needed */
      case am_M: /* memory operand, decoding needed; size includes modRM byte */
      case am_Q: /* MMX register or memory location */
      case am_W: /* XMM register or memory location */
        nib += ia32_decode_modrm(addrSzAttr, addr);
        break;
      case am_I: /* immediate data */
      case am_J: /* instruction pointer offset */
        switch(op.optype) {
        case op_b:
          nib += byteSzB;
          break;
        case op_v:
          nib += wordSzB * operSzAttr;
          break;
        case op_w:
          nib += wordSzB;
        }
        break;
        /* Don't forget these... */
      case am_X: /* memory at DS:(E)SI*/
      case am_Y: /* memory at ES:(E)DI*/
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
ia32_prefixes& ia32_decode_prefixes(const unsigned char* addr, ia32_prefixes& pref)
{
  pref.count = 0;
  pref.prfx[0] = pref.prfx[1] = pref.prfx[2] = pref.prfx[3] = 0;
  bool in_prefix = true;

  while(in_prefix) {
    switch(addr[0]) {
    case PREFIX_REPNZ:
    case PREFIX_REP:
      if(addr[1]==0x0F && sse_prefix[addr[2]])
        break;
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
      if(addr[1]==0x0F && sse_prefix[addr[2]])
        break;
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
  
  //printf("Got %d prefixes\n", pref.count);
  return pref;
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

  return insnType;
}
