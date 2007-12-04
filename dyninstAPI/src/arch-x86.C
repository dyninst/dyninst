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

// $Id: arch-x86.C,v 1.74 2007/12/04 21:47:13 bernat Exp $

// Official documentation used:    - IA-32 Intel Architecture Software Developer Manual (2001 ed.)
//                                 - AMD x86-64 Architecture Programmer's Manual (rev 3.00, 1/2002)
// Unofficial documentation used:  - www.sandpile.org/ia32
//                                 - NASM documentation

// Note: Unless specified "book" refers to Intel's manual

#include <assert.h>
#include <stdio.h>
#include "boost/assign/list_of.hpp"
#include "boost/assign/std/vector.hpp"
#include "boost/assign/std/set.hpp"
#include <map>
#include <string>
#include "common/h/Types.h"
#include "arch.h"
#include "util.h"
#include "debug.h"
#include "InstrucIter.h"
#include "dyninstAPI/src/emit-x86.h"
#include "process.h"
#include "inst-x86.h"

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



using namespace boost::assign;
using namespace std;


map<entryID, string> entryNames = map_list_of
  (e_aaa, "aaa")
  (e_aad, "aad")
  (e_aam, "aam")
  (e_aas, "aas")
  (e_adc, "adc")
  (e_add, "add")
  (e_addpd, "addpd")
  (e_addps, "addps")
  (e_addsd, "addsd")
  (e_addss, "addss")
  (e_and, "and")
  (e_andnpd, "andnpd")
  (e_andnps, "andnps")
  (e_andpd, "andpd")
  (e_andps, "andps")
  (e_arpl, "arpl")
  (e_bound, "bound")
  (e_bsf, "bsf")
  (e_bsr, "bsr")
  (e_bswap, "bswap")
  (e_bt, "bt")
  (e_btc, "btc")
  (e_btr, "btr")
  (e_bts, "bts")
  (e_call, "call")
  (e_cbw_cwde, "cbw/cwde")
  (e_clc, "clc")
  (e_cld, "cld")
  (e_clflush, "clflush")
  (e_cli, "cli")
  (e_clts, "clts")
  (e_cmc, "cmc")
  (e_cmovbe, "cmovbe")
  (e_cmove, "cmove")
  (e_cmovnae, "cmovnae")
  (e_cmovnb, "cmovnb")
  (e_cmovnbe, "cmovnbe")
  (e_cmovne, "cmovne")
  (e_cmovng, "cmovng")
  (e_cmovnge, "cmovnge")
  (e_cmovnl, "cmovnl")
  (e_cmovno, "cmovno")
  (e_cmovns, "cmovns")
  (e_cmovo, "cmovo")
  (e_cmovpe, "cmovpe")
  (e_cmovpo, "cmovpo")
  (e_cmovs, "cmovs")
  (e_cmp, "cmp")
  (e_cmppd, "cmppd")
  (e_cmpps, "cmpps")
  (e_cmpsb, "cmpsb")
  (e_cmpsd, "cmpsd")
  (e_cmpss, "cmpss")
  (e_cmpsw, "cmpsw")
  (e_cmpxch, "cmpxch")
  (e_cmpxch8b, "cmpxch8b")
  (e_comisd, "comisd")
  (e_comiss, "comiss")
  (e_cpuid, "cpuid")
  (e_cvtdq2pd, "cvtdq2pd")
  (e_cvtdq2ps, "cvtdq2ps")
  (e_cvtpd2dq, "cvtpd2dq")
  (e_cvtpd2pi, "cvtpd2pi")
  (e_cvtpd2ps, "cvtpd2ps")
  (e_cvtpi2pd, "cvtpi2pd")
  (e_cvtpi2ps, "cvtpi2ps")
  (e_cvtps2dq, "cvtps2dq")
  (e_cvtps2pd, "cvtps2pd")
  (e_cvtps2pi, "cvtps2pi")
  (e_cvtsd2si, "cvtsd2si")
  (e_cvtsd2ss, "cvtsd2ss")
  (e_cvtsi2sd, "cvtsi2sd")
  (e_cvtsi2ss, "cvtsi2ss")
  (e_cvtss2sd, "cvtss2sd")
  (e_cvtss2si, "cvtss2si")
  (e_cvttpd2dq, "cvttpd2dq")
  (e_cvttpd2pi, "cvttpd2pi")
  (e_cvttps2dq, "cvttps2dq")
  (e_cvttps2pi, "cvttps2pi")
  (e_cvttsd2si, "cvttsd2si")
  (e_cvttss2si, "cvttss2si")
  (e_cwd_cdq, "cwd/cdq")
  (e_daa, "daa")
  (e_das, "das")
  (e_dec, "dec")
  (e_div, "div")
  (e_divpd, "divpd")
  (e_divps, "divps")
  (e_divsd, "divsd")
  (e_divss, "divss")
  (e_emms, "emms")
  (e_enter, "enter")
  (e_femms, "femms")
  (e_fxrstor, "fxrstor")
  (e_fxsave, "fxsave")
  (e_hlt, "hlt")
  (e_idiv, "idiv")
  (e_imul, "imul")
  (e_in, "in")
  (e_inc, "inc")
  (e_insb, "insb")
  (e_insw_d, "insw/d")
  (e_int, "int")
  (e_int3, "int 3")
  (e_int1, "int1")
  (e_into, "into")
  (e_invd, "invd")
  (e_invlpg, "invlpg")
  (e_iret, "iret")
  (e_jb, "jb")
  (e_jb_jnaej_j, "jb/jnaej/j")
  (e_jbe, "jbe")
  (e_jcxz_jec, "jcxz/jec")
  (e_jl, "jl")
  (e_jle, "jle")
  (e_jmp, "jmp")
  (e_jnb, "jnb")
  (e_jnb_jae_j, "jnb/jae/j")
  (e_jnbe, "jnbe")
  (e_jnl, "jnl")
  (e_jnle, "jnle")
  (e_jno, "jno")
  (e_jnp, "jnp")
  (e_jns, "jns")
  (e_jnz, "jnz")
  (e_jo, "jo")
  (e_jp, "jp")
  (e_js, "js")
  (e_jz, "jz")
  (e_lahf, "lahf")
  (e_lar, "lar")
  (e_ldmxcsr, "ldmxcsr")
  (e_lds, "lds")
  (e_lea, "lea")
  (e_leave, "leave")
  (e_les, "les")
  (e_lfence, "lfence")
  (e_lfs, "lfs")
  (e_lgdt, "lgdt")
  (e_lgs, "lgs")
  (e_lidt, "lidt")
  (e_lldt, "lldt")
  (e_lmsw, "lmsw")
  (e_lodsb, "lodsb")
  (e_lodsw, "lodsw")
  (e_loop, "loop")
  (e_loope, "loope")
  (e_loopn, "loopn")
  (e_lsl, "lsl")
  (e_lss, "lss")
  (e_ltr, "ltr")
  (e_maskmovdqu, "maskmovdqu")
  (e_maskmovq, "maskmovq")
  (e_maxpd, "maxpd")
  (e_maxps, "maxps")
  (e_maxsd, "maxsd")
  (e_maxss, "maxss")
  (e_mfence, "mfence")
  (e_minpd, "minpd")
  (e_minps, "minps")
  (e_minsd, "minsd")
  (e_minss, "minss")
  (e_mmxud, "mmxud")
  (e_mov, "mov")
  (e_movapd, "movapd")
  (e_movaps, "movaps")
  (e_movd, "movd")
  (e_movdq2q, "movdq2q")
  (e_movdqa, "movdqa")
  (e_movdqu, "movdqu")
  (e_movhpd, "movhpd")
  (e_movhps, "movhps")
  (e_movhps_movlhps, "movhps/movlhps")
  (e_movlpd, "movlpd")
  (e_movlps, "movlps")
  (e_movlps_movhlps, "movlps/movhlps")
  (e_movmskpd, "movmskpd")
  (e_movmskps, "movmskps")
  (e_movntdq, "movntdq")
  (e_movnti, "movnti")
  (e_movntpd, "movntpd")
  (e_movntps, "movntps")
  (e_movntq, "movntq")
  (e_movq, "movq")
  (e_movq2dq, "movq2dq")
  (e_movsb, "movsb")
  (e_movsd, "movsd")
  (e_movss, "movss")
  (e_movsw_d, "movsw/d")
  (e_movsx, "movsx")
  (e_movupd, "movupd")
  (e_movups, "movups")
  (e_movzx, "movzx")
  (e_mul, "mul")
  (e_mulpd, "mulpd")
  (e_mulps, "mulps")
  (e_mulsd, "mulsd")
  (e_mulss, "mulss")
  (e_neg, "neg")
  (e_nop, "nop")
  (e_not, "not")
  (e_or, "or")
  (e_orpd, "orpd")
  (e_orps, "orps")
  (e_out, "out")
  (e_outsb, "outsb")
  (e_outsw_d, "outsw/d")
  (e_packssdw, "packssdw")
  (e_packsswb, "packsswb")
  (e_packuswb, "packuswb")
  (e_paddb, "paddb")
  (e_paddd, "paddd")
  (e_paddq, "paddq")
  (e_paddsb, "paddsb")
  (e_paddsw, "paddsw")
  (e_paddusb, "paddusb")
  (e_paddusw, "paddusw")
  (e_paddw, "paddw")
  (e_pand, "pand")
  (e_pandn, "pandn")
  (e_pavgb, "pavgb")
  (e_pavgw, "pavgw")
  (e_pcmpeqb, "pcmpeqb")
  (e_pcmpeqd, "pcmpeqd")
  (e_pcmpeqw, "pcmpeqw")
  (e_pcmpgdt, "pcmpgdt")
  (e_pcmpgtb, "pcmpgtb")
  (e_pcmpgtw, "pcmpgtw")
  (e_pextrw, "pextrw")
  (e_pinsrw, "pinsrw")
  (e_pmaddwd, "pmaddwd")
  (e_pmaxsw, "pmaxsw")
  (e_pmaxub, "pmaxub")
  (e_pminsw, "pminsw")
  (e_pminub, "pminub")
  (e_pmovmskb, "pmovmskb")
  (e_pmulhuw, "pmulhuw")
  (e_pmulhw, "pmulhw")
  (e_pmullw, "pmullw")
  (e_pmuludq, "pmuludq")
  (e_pop, "pop")
  (e_popa_d, "popa(d)")
  (e_popf_d, "popf(d)")
  (e_por, "por")
  (e_prefetch, "prefetch")
  (e_prefetchNTA, "prefetchNTA")
  (e_prefetchT0, "prefetchT0")
  (e_prefetchT1, "prefetchT1")
  (e_prefetchT2, "prefetchT2")
  (e_prefetch_w, "prefetch(w)")
  (e_prefetchw, "prefetchw")
  (e_psadbw, "psadbw")
  (e_pshufd, "pshufd")
  (e_pshufhw, "pshufhw")
  (e_pshuflw, "pshuflw")
  (e_pshufw, "pshufw")
  (e_pslld, "pslld")
  (e_pslldq, "pslldq")
  (e_psllq, "psllq")
  (e_psllw, "psllw")
  (e_psrad, "psrad")
  (e_psraw, "psraw")
  (e_psrld, "psrld")
  (e_psrldq, "psrldq")
  (e_psrlq, "psrlq")
  (e_psrlw, "psrlw")
  (e_psubb, "psubb")
  (e_psubd, "psubd")
  (e_psubsb, "psubsb")
  (e_psubsw, "psubsw")
  (e_psubusb, "psubusb")
  (e_psubusw, "psubusw")
  (e_psubw, "psubw")
  (e_punpckhbw, "punpckhbw")
  (e_punpckhdq, "punpckhdq")
  (e_punpckhqd, "punpckhqd")
  (e_punpckhwd, "punpckhwd")
  (e_punpcklbw, "punpcklbw")
  (e_punpcklqd, "punpcklqd")
  (e_punpcklqld, "punpcklqld")
  (e_punpcklwd, "punpcklwd")
  (e_push, "push")
  (e_pusha_d, "pusha(d)")
  (e_pushf_d, "pushf(d)")
  (e_pxor, "pxor")
  (e_rcl, "rcl")
  (e_rcpps, "rcpps")
  (e_rcpss, "rcpss")
  (e_rcr, "rcr")
  (e_rdmsr, "rdmsr")
  (e_rdpmc, "rdpmc")
  (e_rdtsc, "rdtsc")
  (e_ret_far, "ret far")
  (e_ret_near, "ret near")
  (e_rol, "rol")
  (e_ror, "ror")
  (e_rsm, "rsm")
  (e_rsqrtps, "rsqrtps")
  (e_rsqrtss, "rsqrtss")
  (e_sahf, "sahf")
  (e_salc, "salc")
  (e_sar, "sar")
  (e_sbb, "sbb")
  (e_scasb, "scasb")
  (e_scasw_d, "scasw/d")
  (e_setb, "setb")
  (e_setbe, "setbe")
  (e_setl, "setl")
  (e_setle, "setle")
  (e_setnb, "setnb")
  (e_setnbe, "setnbe")
  (e_setnl, "setnl")
  (e_setnle, "setnle")
  (e_setno, "setno")
  (e_setnp, "setnp")
  (e_setns, "setns")
  (e_setnz, "setnz")
  (e_seto, "seto")
  (e_setp, "setp")
  (e_sets, "sets")
  (e_setz, "setz")
  (e_sfence, "sfence")
  (e_sgdt, "sgdt")
  (e_shl_sal, "shl/sal")
  (e_shld, "shld")
  (e_shr, "shr")
  (e_shrd, "shrd")
  (e_shufpd, "shufpd")
  (e_shufps, "shufps")
  (e_sidt, "sidt")
  (e_sldt, "sldt")
  (e_smsw, "smsw")
  (e_sqrtpd, "sqrtpd")
  (e_sqrtps, "sqrtps")
  (e_sqrtsd, "sqrtsd")
  (e_sqrtss, "sqrtss")
  (e_stc, "stc")
  (e_std, "std")
  (e_sti, "sti")
  (e_stmxcsr, "stmxcsr")
  (e_stosb, "stosb")
  (e_stosw_d, "stosw/d")
  (e_str, "str")
  (e_sub, "sub")
  (e_subpd, "subpd")
  (e_subps, "subps")
  (e_subsd, "subsd")
  (e_subss, "subss")
  (e_syscall, "syscall")
  (e_sysenter, "sysenter")
  (e_sysexit, "sysexit")
  (e_sysret, "sysret")
  (e_test, "test")
  (e_ucomisd, "ucomisd")
  (e_ucomiss, "ucomiss")
  (e_ud2, "ud2")
  (e_ud2grp10, "ud2grp10")
  (e_unpckhpd, "unpckhpd")
  (e_unpckhps, "unpckhps")
  (e_unpcklpd, "unpcklpd")
  (e_unpcklps, "unpcklps")
  (e_verr, "verr")
  (e_verw, "verw")
  (e_wait, "wait")
  (e_wbinvd, "wbinvd")
  (e_wrmsr, "wrmsr")
  (e_xadd, "xadd")
  (e_xchg, "xchg")
  (e_xlat, "xlat")
  (e_xor, "xor")
  (e_xorpd, "xorpd")
  (e_xorps, "xorps");

const char* ia32_entry::name()
{
  map<entryID, string>::const_iterator found = entryNames.find(id);
  
  if(found != entryNames.end())
  {
    return found->second.c_str();
  }
  return NULL;
}

struct flagInfo
{
  flagInfo(const vector<RegisterID>& rf, const vector<RegisterID>& wf) : readFlags(rf), writtenFlags(wf) 
  {
  }
  vector<RegisterID> readFlags;
  vector<RegisterID> writtenFlags;
};


vector<RegisterID> standardFlags = list_of(r_OF)(r_SF)(r_ZF)(r_AF)(r_PF)(r_CF);

map<entryID, flagInfo> flagTable = map_list_of
(e_aaa, flagInfo(list_of(r_AF), standardFlags))
  (e_aad, flagInfo(vector<RegisterID>(), standardFlags))
  (e_aam, flagInfo(vector<RegisterID>(), standardFlags))
  (e_aas, flagInfo(list_of(r_AF), standardFlags))
  (e_adc, flagInfo(list_of(r_CF), standardFlags))
  (e_add, flagInfo(vector<RegisterID>(), standardFlags))
  (e_and, flagInfo(vector<RegisterID>(), standardFlags))
  (e_arpl, flagInfo(vector<RegisterID>(), list_of(r_ZF)))
  (e_bsf, flagInfo(vector<RegisterID>(), standardFlags))
  (e_bt, flagInfo(vector<RegisterID>(), standardFlags))
  (e_bts, flagInfo(vector<RegisterID>(), standardFlags))
  (e_btr, flagInfo(vector<RegisterID>(), standardFlags))
  (e_btc, flagInfo(vector<RegisterID>(), standardFlags))
  (e_clc, flagInfo(vector<RegisterID>(), list_of(r_CF)))
  (e_cld, flagInfo(vector<RegisterID>(), list_of(r_DF)))
  (e_cli, flagInfo(vector<RegisterID>(), list_of(r_IF)))
  (e_cmc, flagInfo(vector<RegisterID>(), list_of(r_CF)))
  (e_cmovbe, flagInfo(list_of(r_OF)(r_SF)(r_ZF)(r_PF)(r_CF), vector<RegisterID>()))
  (e_cmove, flagInfo(list_of(r_OF)(r_SF)(r_ZF)(r_PF)(r_CF), vector<RegisterID>()))
  (e_cmovnae, flagInfo(list_of(r_OF)(r_SF)(r_ZF)(r_PF)(r_CF), vector<RegisterID>()))
  (e_cmovnb, flagInfo(list_of(r_OF)(r_SF)(r_ZF)(r_PF)(r_CF), vector<RegisterID>()))
  (e_cmovnbe, flagInfo(list_of(r_OF)(r_SF)(r_ZF)(r_PF)(r_CF), vector<RegisterID>()))
  (e_cmovne, flagInfo(list_of(r_OF)(r_SF)(r_ZF)(r_PF)(r_CF), vector<RegisterID>()))
  (e_cmovng, flagInfo(list_of(r_OF)(r_SF)(r_ZF)(r_PF)(r_CF), vector<RegisterID>()))
  (e_cmovnge, flagInfo(list_of(r_OF)(r_SF)(r_ZF)(r_PF)(r_CF), vector<RegisterID>()))
  (e_cmovnl, flagInfo(list_of(r_OF)(r_SF)(r_ZF)(r_PF)(r_CF), vector<RegisterID>()))
  (e_cmovno, flagInfo(list_of(r_OF)(r_SF)(r_ZF)(r_PF)(r_CF), vector<RegisterID>()))
  (e_cmovns, flagInfo(list_of(r_OF)(r_SF)(r_ZF)(r_PF)(r_CF), vector<RegisterID>()))
  (e_cmovo, flagInfo(list_of(r_OF)(r_SF)(r_ZF)(r_PF)(r_CF), vector<RegisterID>()))
  (e_cmovpe, flagInfo(list_of(r_OF)(r_SF)(r_ZF)(r_PF)(r_CF), vector<RegisterID>()))
  (e_cmovpo, flagInfo(list_of(r_OF)(r_SF)(r_ZF)(r_PF)(r_CF), vector<RegisterID>()))
  (e_cmovs, flagInfo(list_of(r_OF)(r_SF)(r_ZF)(r_PF)(r_CF), vector<RegisterID>()))
  (e_cmp, flagInfo(vector<RegisterID>(), standardFlags))
  (e_cmpsb, flagInfo(vector<RegisterID>(), standardFlags))
  (e_cmpsd, flagInfo(vector<RegisterID>(), standardFlags))
  (e_cmpss, flagInfo(vector<RegisterID>(), standardFlags))
  (e_cmpsw, flagInfo(vector<RegisterID>(), standardFlags))
  (e_cmpxch, flagInfo(vector<RegisterID>(), standardFlags))
  (e_cmpxch8b, flagInfo(vector<RegisterID>(), list_of(r_ZF)))
  (e_comisd, flagInfo(vector<RegisterID>(), standardFlags))
  (e_comiss, flagInfo(vector<RegisterID>(), standardFlags))
  (e_daa, flagInfo(list_of(r_AF)(r_CF), standardFlags))
  (e_das, flagInfo(list_of(r_AF)(r_CF), standardFlags))
  (e_dec, flagInfo(vector<RegisterID>(), list_of(r_OF)(r_SF)(r_ZF)(r_AF)(r_PF)))
  (e_div, flagInfo(list_of(r_AF)(r_CF), standardFlags))
  // TODO: FCMOVcc (not in our entry table) (reads zf/pf/cf)
  // TODO: FCOMI/FCOMIP/FUCOMI/FUCOMIP (writes/zf/pf/cf)
  (e_idiv, flagInfo(vector<RegisterID>(), standardFlags))
  (e_imul, flagInfo(vector<RegisterID>(), standardFlags))
  (e_inc, flagInfo(vector<RegisterID>(), list_of(r_OF)(r_SF)(r_ZF)(r_AF)(r_PF)))
  (e_insb, flagInfo(list_of(r_DF), vector<RegisterID>()))
  (e_insw_d, flagInfo(list_of(r_DF), vector<RegisterID>()))
  (e_int, flagInfo(vector<RegisterID>(), list_of(r_TF)(r_NT)))
  (e_int3, flagInfo(vector<RegisterID>(), list_of(r_TF)(r_NT)))
  (e_into, flagInfo(list_of(r_OF), list_of(r_TF)(r_NT)))
  (e_ucomisd, flagInfo(vector<RegisterID>(), standardFlags))
  (e_ucomiss, flagInfo(vector<RegisterID>(), standardFlags))
  (e_iret, flagInfo(list_of(r_NT), list_of(r_OF)(r_SF)(r_ZF)(r_AF)(r_PF)(r_CF)(r_TF)(r_IF)(r_DF)))
  (e_jb, flagInfo(list_of(r_OF)(r_SF)(r_ZF)(r_PF)(r_CF), vector<RegisterID>()))
  (e_jb_jnaej_j, flagInfo(list_of(r_OF)(r_SF)(r_ZF)(r_PF)(r_CF), vector<RegisterID>()))
  (e_jbe, flagInfo(list_of(r_OF)(r_SF)(r_ZF)(r_PF)(r_CF), vector<RegisterID>()))
  (e_jcxz_jec, flagInfo(list_of(r_OF)(r_SF)(r_ZF)(r_PF)(r_CF), vector<RegisterID>()))
  (e_jl, flagInfo(list_of(r_OF)(r_SF)(r_ZF)(r_PF)(r_CF), vector<RegisterID>()))
  (e_jle, flagInfo(list_of(r_OF)(r_SF)(r_ZF)(r_PF)(r_CF), vector<RegisterID>()))
  (e_jnb, flagInfo(list_of(r_OF)(r_SF)(r_ZF)(r_PF)(r_CF), vector<RegisterID>()))
  (e_jnb_jae_j, flagInfo(list_of(r_OF)(r_SF)(r_ZF)(r_PF)(r_CF), vector<RegisterID>()))
  (e_jnbe, flagInfo(list_of(r_OF)(r_SF)(r_ZF)(r_PF)(r_CF), vector<RegisterID>()))
  (e_jnl, flagInfo(list_of(r_OF)(r_SF)(r_ZF)(r_PF)(r_CF), vector<RegisterID>()))
  (e_jnle, flagInfo(list_of(r_OF)(r_SF)(r_ZF)(r_PF)(r_CF), vector<RegisterID>()))
  (e_jno, flagInfo(list_of(r_OF)(r_SF)(r_ZF)(r_PF)(r_CF), vector<RegisterID>()))
  (e_jnp, flagInfo(list_of(r_OF)(r_SF)(r_ZF)(r_PF)(r_CF), vector<RegisterID>()))
  (e_jns, flagInfo(list_of(r_OF)(r_SF)(r_ZF)(r_PF)(r_CF), vector<RegisterID>()))
  (e_jnz, flagInfo(list_of(r_OF)(r_SF)(r_ZF)(r_PF)(r_CF), vector<RegisterID>()))
  (e_jo, flagInfo(list_of(r_OF)(r_SF)(r_ZF)(r_PF)(r_CF), vector<RegisterID>()))
  (e_jp, flagInfo(list_of(r_OF)(r_SF)(r_ZF)(r_PF)(r_CF), vector<RegisterID>()))
  (e_js, flagInfo(list_of(r_OF)(r_SF)(r_ZF)(r_PF)(r_CF), vector<RegisterID>()))
  (e_jz, flagInfo(list_of(r_OF)(r_SF)(r_ZF)(r_PF)(r_CF), vector<RegisterID>()))
  (e_lar, flagInfo(vector<RegisterID>(), list_of(r_ZF)))
  (e_loop, flagInfo(list_of(r_DF), vector<RegisterID>()))
  (e_loope, flagInfo(list_of(r_ZF), vector<RegisterID>()))
  (e_loopn, flagInfo(list_of(r_ZF), vector<RegisterID>()))
  (e_lsl, flagInfo(vector<RegisterID>(), list_of(r_ZF)))
  // I'd expect that mov control/debug/test gets handled when we do operand analysis
  // If it doesn't, fix later
  (e_mul, flagInfo(vector<RegisterID>(), standardFlags))
  (e_neg, flagInfo(vector<RegisterID>(), standardFlags))
  (e_or, flagInfo(vector<RegisterID>(), standardFlags))
  (e_outsb, flagInfo(list_of(r_DF), vector<RegisterID>()))
  (e_outsw_d, flagInfo(list_of(r_DF), vector<RegisterID>()))
  (e_popf_d, flagInfo(vector<RegisterID>(), list_of(r_OF)(r_SF)(r_ZF)(r_AF)(r_PF)(r_CF)(r_TF)(r_IF)(r_DF)(r_NT)))
  (e_rcl, flagInfo(list_of(r_CF), list_of(r_OF)(r_CF)))
  (e_rcr, flagInfo(list_of(r_CF), list_of(r_OF)(r_CF)))
  (e_rol, flagInfo(list_of(r_CF), list_of(r_OF)(r_CF)))
  (e_ror, flagInfo(list_of(r_CF), list_of(r_OF)(r_CF)))
  (e_rsm, flagInfo(vector<RegisterID>(), list_of(r_OF)(r_SF)(r_ZF)(r_AF)(r_PF)(r_CF)(r_TF)(r_IF)(r_DF)(r_NT)(r_RF)))
  (e_sahf, flagInfo(list_of(r_SF)(r_ZF)(r_AF)(r_PF)(r_CF), vector<RegisterID>()))
  (e_sar, flagInfo(vector<RegisterID>(), standardFlags))
  (e_shr, flagInfo(vector<RegisterID>(), standardFlags))
  (e_setb, flagInfo(list_of(r_OF)(r_SF)(r_ZF)(r_PF)(r_CF), vector<RegisterID>()))
  (e_setbe, flagInfo(list_of(r_OF)(r_SF)(r_ZF)(r_PF)(r_CF), vector<RegisterID>()))
  (e_setl, flagInfo(list_of(r_OF)(r_SF)(r_ZF)(r_PF)(r_CF), vector<RegisterID>()))
  (e_setle, flagInfo(list_of(r_OF)(r_SF)(r_ZF)(r_PF)(r_CF), vector<RegisterID>()))
  (e_setnb, flagInfo(list_of(r_OF)(r_SF)(r_ZF)(r_PF)(r_CF), vector<RegisterID>()))
  (e_setnbe, flagInfo(list_of(r_OF)(r_SF)(r_ZF)(r_PF)(r_CF), vector<RegisterID>()))
  (e_setnl, flagInfo(list_of(r_OF)(r_SF)(r_ZF)(r_PF)(r_CF), vector<RegisterID>()))
  (e_setnle, flagInfo(list_of(r_OF)(r_SF)(r_ZF)(r_PF)(r_CF), vector<RegisterID>()))
  (e_setno, flagInfo(list_of(r_OF)(r_SF)(r_ZF)(r_PF)(r_CF), vector<RegisterID>()))
  (e_setnp, flagInfo(list_of(r_OF)(r_SF)(r_ZF)(r_PF)(r_CF), vector<RegisterID>()))
  (e_setns, flagInfo(list_of(r_OF)(r_SF)(r_ZF)(r_PF)(r_CF), vector<RegisterID>()))
  (e_setnz, flagInfo(list_of(r_OF)(r_SF)(r_ZF)(r_PF)(r_CF), vector<RegisterID>()))
  (e_seto, flagInfo(list_of(r_OF)(r_SF)(r_ZF)(r_PF)(r_CF), vector<RegisterID>()))
  (e_setp, flagInfo(list_of(r_OF)(r_SF)(r_ZF)(r_PF)(r_CF), vector<RegisterID>()))
  (e_sets, flagInfo(list_of(r_OF)(r_SF)(r_ZF)(r_PF)(r_CF), vector<RegisterID>()))
  (e_setz, flagInfo(list_of(r_OF)(r_SF)(r_ZF)(r_PF)(r_CF), vector<RegisterID>()))
  (e_shld, flagInfo(vector<RegisterID>(), standardFlags))
  (e_shrd, flagInfo(vector<RegisterID>(), standardFlags))
  (e_shl_sal, flagInfo(vector<RegisterID>(), standardFlags))
  (e_stc, flagInfo(vector<RegisterID>(), list_of(r_CF)))
  (e_std, flagInfo(vector<RegisterID>(), list_of(r_DF)))
  (e_sti, flagInfo(vector<RegisterID>(), list_of(r_IF)))
  (e_stosb, flagInfo(list_of(r_DF), vector<RegisterID>()))
  (e_stosw_d, flagInfo(list_of(r_DF), vector<RegisterID>()))
  (e_sub, flagInfo(vector<RegisterID>(), standardFlags))
  (e_verr, flagInfo(vector<RegisterID>(), list_of(r_ZF)))
  (e_verw, flagInfo(vector<RegisterID>(), list_of(r_ZF)))
  (e_xadd, flagInfo(vector<RegisterID>(), standardFlags))
  (e_xor, flagInfo(vector<RegisterID>(), standardFlags))

;


bool ia32_entry::flagsUsed(std::set<RegisterID>& flagsRead, std::set<RegisterID>& flagsWritten)
{
  map<entryID, flagInfo>::const_iterator found = flagTable.find(id);
  if(found == flagTable.end())
  {
    return false;
  }
  // No entries for something that touches no flags, so always return true if we had an entry
  const flagInfo& foundInfo(found->second);

  copy(foundInfo.readFlags.begin(), foundInfo.readFlags.end(), inserter(flagsRead, flagsRead.begin()));
  copy(foundInfo.writtenFlags.begin(), foundInfo.writtenFlags.end(), inserter(flagsWritten,flagsWritten.begin()));
  return true;
}



// Modded table entry for push/pop, daa, das, aaa, aas, insb/w/d, outsb/w/d, xchg, cbw
// les, lds, aam, aad, loop(z/nz), cmpxch, lss, mul, imul, div, idiv, cmpxch8, [ld/st]mxcsr
// clflush, prefetch*


// oneByteMap: one byte opcode map
static ia32_entry oneByteMap[256] = {
  /* 00 */
  { e_add,  t_done, 0, true, { Eb, Gb, Zz }, 0, s1RW2R },
  { e_add,  t_done, 0, true, { Ev, Gv, Zz }, 0, s1RW2R },
  { e_add,  t_done, 0, true, { Gb, Eb, Zz }, 0, s1RW2R },
  { e_add,  t_done, 0, true, { Gv, Ev, Zz }, 0, s1RW2R },
  { e_add,  t_done, 0, false, { AL, Ib, Zz }, 0, s1RW2R },
  { e_add,  t_done, 0, false, { eAX, Iz, Zz }, 0, s1RW2R },
  { e_push, t_done, 0, false, { STHw, ES, eSP }, 0, s1W2R3RW },
  { e_pop,  t_done, 0, false, { ES, STPw, eSP }, 0, s1W2R3RW },
  /* 08 */
  { e_or,   t_done, 0, true, { Eb, Gb, Zz }, 0, s1RW2R },
  { e_or,   t_done, 0, true, { Ev, Gv, Zz }, 0, s1RW2R },
  { e_or,   t_done, 0, true, { Gb, Eb, Zz }, 0, s1RW2R },
  { e_or,   t_done, 0, true, { Gv, Ev, Zz }, 0, s1RW2R },
  { e_or,   t_done, 0, false, { AL, Ib, Zz }, 0, s1RW2R },
  { e_or,   t_done, 0, false, { eAX, Iz, Zz }, 0, s1RW2R },
  { e_push, t_done, 0, false, { STHw, CS, eSP }, 0, s1W2R3RW },
  { e_No_Entry,      t_twoB, 0, false, { Zz, Zz, Zz }, 0, 0 },
  /* 10 */
  { e_adc,  t_done, 0, true, { Eb, Gb, Zz }, 0, s1RW2R },
  { e_adc,  t_done, 0, true, { Ev, Gv, Zz }, 0, s1RW2R },
  { e_adc,  t_done, 0, true, { Gb, Eb, Zz }, 0, s1RW2R },
  { e_adc,  t_done, 0, true, { Gv, Ev, Zz }, 0, s1RW2R },
  { e_adc,  t_done, 0, false, { AL, Ib, Zz }, 0, s1RW2R },
  { e_adc,  t_done, 0, false, { eAX, Iz, Zz }, 0, s1RW2R },
  { e_push, t_done, 0, false, { STHw, SS, eSP }, 0, s1W2R3RW },
  { e_pop,  t_done, 0, false, { SS, STPw, eSP }, 0, s1W2R3RW },
  /* 18 */
  { e_sbb,  t_done, 0, true, { Eb, Gb, Zz }, 0, s1RW2R },
  { e_sbb,  t_done, 0, true, { Ev, Gv, Zz }, 0, s1RW2R },
  { e_sbb,  t_done, 0, true, { Gb, Eb, Zz }, 0, s1RW2R },
  { e_sbb,  t_done, 0, true, { Gv, Ev, Zz }, 0, s1RW2R },
  { e_sbb,  t_done, 0, false, { AL, Ib, Zz }, 0, s1RW2R },
  { e_sbb,  t_done, 0, false, { eAX, Iz, Zz }, 0, s1RW2R },
  { e_push, t_done, 0, false, { STHw, DS, eSP }, 0, s1W2R3RW },
  { e_pop , t_done, 0, false, { DS, STPw, eSP }, 0, s1W2R3RW },
  /* 20 */
  { e_and, t_done, 0, true, { Eb, Gb, Zz }, 0, s1RW2R },
  { e_and, t_done, 0, true, { Ev, Gv, Zz }, 0, s1RW2R },
  { e_and, t_done, 0, true, { Gb, Eb, Zz }, 0, s1RW2R },
  { e_and, t_done, 0, true, { Gv, Ev, Zz }, 0, s1RW2R },
  { e_and, t_done, 0, false, { AL, Ib, Zz }, 0, s1RW2R },
  { e_and, t_done, 0, false, { eAX, Iz, Zz }, 0, s1RW2R },
  { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 }, // PREFIX_SEG_OVR
  { e_daa, t_done, 0, false, { AL, Zz, Zz }, 0, s1RW },
  /* 28 */
  { e_sub, t_done, 0, true, { Eb, Gb, Zz }, 0, s1RW2R },
  { e_sub, t_done, 0, true, { Ev, Gv, Zz }, 0, s1RW2R },
  { e_sub, t_done, 0, true, { Gb, Eb, Zz }, 0, s1RW2R },
  { e_sub, t_done, 0, true, { Gv, Ev, Zz }, 0, s1RW2R },
  { e_sub, t_done, 0, false, { AL, Ib, Zz }, 0, s1RW2R },
  { e_sub, t_done, 0, false, { eAX, Iz, Zz }, 0, s1RW2R },
  { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 }, // PREFIX_SEG_OVR
  { e_das , t_done, 0, false, { AL, Zz, Zz }, 0, s1RW },
  /* 30 */
  { e_xor, t_done, 0, true, { Eb, Gb, Zz }, 0, s1RW2R },
  { e_xor, t_done, 0, true, { Ev, Gv, Zz }, 0, s1RW2R },
  { e_xor, t_done, 0, true, { Gb, Eb, Zz }, 0, s1RW2R },
  { e_xor, t_done, 0, true, { Gv, Ev, Zz }, 0, s1RW2R },
  { e_xor, t_done, 0, false, { AL, Ib, Zz }, 0, s1RW2R },
  { e_xor, t_done, 0, false, { eAX, Iz, Zz }, 0, s1RW2R },
  { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 }, // PREFIX_SEG_OVR
  { e_aaa, t_done, 0, false, { AX, Zz, Zz }, 0, s1RW2R },
  /* 38 */
  { e_cmp, t_done, 0, true, { Eb, Gb, Zz }, 0, s1R2R },
  { e_cmp, t_done, 0, true, { Ev, Gv, Zz }, 0, s1R2R },
  { e_cmp, t_done, 0, true, { Gb, Eb, Zz }, 0, s1R2R },
  { e_cmp, t_done, 0, true, { Gv, Ev, Zz }, 0, s1R2R },
  { e_cmp, t_done, 0, false, { AL, Ib, Zz }, 0, s1R2R },
  { e_cmp, t_done, 0, false, { eAX, Iz, Zz }, 0, s1R2R },
  { e_No_Entry,     t_ill,  0, false, { Zz, Zz, Zz }, 0, 0 }, // PREFIX_SEG_OVR
  { e_aas, t_done, 0, false, { AX, Zz, Zz }, 0, s1RW },
  /* 40 */
  { e_inc, t_done, 0, false, { eAX, Zz, Zz }, 0, s1RW },
  { e_inc, t_done, 0, false, { eCX, Zz, Zz }, 0, s1RW },
  { e_inc, t_done, 0, false, { eDX, Zz, Zz }, 0, s1RW },
  { e_inc, t_done, 0, false, { eBX, Zz, Zz }, 0, s1RW },
  { e_inc, t_done, 0, false, { eSP, Zz, Zz }, 0, s1RW },
  { e_inc, t_done, 0, false, { eBP, Zz, Zz }, 0, s1RW },
  { e_inc, t_done, 0, false, { eSI, Zz, Zz }, 0, s1RW },
  { e_inc, t_done, 0, false, { eDI, Zz, Zz }, 0, s1RW },
  /* 48 */
  { e_dec, t_done, 0, false, { eAX, Zz, Zz }, 0, s1RW },
  { e_dec, t_done, 0, false, { eCX, Zz, Zz }, 0, s1RW },
  { e_dec, t_done, 0, false, { eDX, Zz, Zz }, 0, s1RW },
  { e_dec, t_done, 0, false, { eBX, Zz, Zz }, 0, s1RW },
  { e_dec, t_done, 0, false, { eSP, Zz, Zz }, 0, s1RW },
  { e_dec, t_done, 0, false, { eBP, Zz, Zz }, 0, s1RW },
  { e_dec, t_done, 0, false, { eSI, Zz, Zz }, 0, s1RW },
  { e_dec, t_done, 0, false, { eDI, Zz, Zz }, 0, s1RW },
  /* 50 */
  { e_push, t_done, 0, false, { STHv, eAX, eSP }, 0, s1W2R3RW },
  { e_push, t_done, 0, false, { STHv, eCX, eSP }, 0, s1W2R3RW },
  { e_push, t_done, 0, false, { STHv, eDX, eSP }, 0, s1W2R3RW },
  { e_push, t_done, 0, false, { STHv, eBX, eSP }, 0, s1W2R3RW },
  { e_push, t_done, 0, false, { STHv, eSP, eSP }, 0, s1W2R3RW },
  { e_push, t_done, 0, false, { STHv, eBP, eSP }, 0, s1W2R3RW },
  { e_push, t_done, 0, false, { STHv, eSI, eSP }, 0, s1W2R3RW },
  { e_push, t_done, 0, false, { STHv, eDI, eSP }, 0, s1W2R3RW },
  /* 58 */
  { e_pop, t_done, 0, false, { eAX, STPv, eSP }, 0, s1W2R3RW },
  { e_pop, t_done, 0, false, { eCX, STPv, eSP }, 0, s1W2R3RW },
  { e_pop, t_done, 0, false, { eDX, STPv, eSP }, 0, s1W2R3RW },
  { e_pop, t_done, 0, false, { eBX, STPv, eSP }, 0, s1W2R3RW },
  { e_pop, t_done, 0, false, { eSP, STPv, eSP }, 0, s1W2R3RW },
  { e_pop, t_done, 0, false, { eBP, STPv, eSP }, 0, s1W2R3RW },
  { e_pop, t_done, 0, false, { eSI, STPv, eSP }, 0, s1W2R3RW },
  { e_pop, t_done, 0, false, { eDI, STPv, eSP }, 0, s1W2R3RW },
  /* 60 */
  { e_pusha_d, t_done, 0, false, { STHa, GPRS, eSP }, 0, s1W2R3RW },
  { e_popa_d,  t_done, 0, false, { GPRS, STPa, eSP }, 0, s1W2R3RW },
  { e_bound,    t_done, 0, true, { Gv, Ma, Zz }, 0, s1R2R },
  { e_arpl,     t_done, 0, true, { Ew, Gw, Zz }, 0, s1R2R },
  { e_No_Entry,          t_ill,  0, false, { Zz, Zz, Zz }, 0, 0 }, // PREFIX_SEG_OVR
  { e_No_Entry,          t_ill,  0, false, { Zz, Zz, Zz }, 0, 0 }, // PREFIX_SEG_OVR
  { e_No_Entry,   t_prefixedSSE, 2, false, { Zz, Zz, Zz }, 0, 0 }, /* operand size prefix (PREFIX_OPR_SZ)*/
  { e_No_Entry,          t_ill,  0, false, { Zz, Zz, Zz }, 0, 0 }, /* address size prefix (PREFIX_ADDR_SZ)*/
  /* 68 */
  { e_push,    t_done, 0, false, { STHv, Iz, eSP }, 0, s1W2R3RW },
  { e_imul,    t_done, 0, true, { Gv, Ev, Iz }, 0, s1W2R3R },
  { e_push,    t_done, 0, false, { STHb, Ib, eSP }, 0, s1W2R3RW },
  { e_imul,    t_done, 0, true, { Gv, Ev, Ib }, 0, s1W2R3R },
  { e_insb,    t_done, 0, false, { Yb, DX, Zz }, 0, s1W2R | (fREP << FPOS) }, // (e)SI/DI changed
  { e_insw_d,  t_done, 0, false, { Yv, DX, Zz }, 0, s1W2R | (fREP << FPOS) },
  { e_outsb,   t_done, 0, false, { DX, Xb, Zz }, 0, s1W2R | (fREP << FPOS) },
  { e_outsw_d, t_done, 0, false, { DX, Xv, Zz }, 0, s1W2R | (fREP << FPOS) },
  /* 70 */
  { e_jo,         t_done, 0, false, { Jb, Zz, Zz }, (IS_JCC | REL_B), s1R },
  { e_jno,        t_done, 0, false, { Jb, Zz, Zz }, (IS_JCC | REL_B), s1R },
  { e_jb_jnaej_j, t_done, 0, false, { Jb, Zz, Zz }, (IS_JCC | REL_B), s1R },
  { e_jnb_jae_j,  t_done, 0, false, { Jb, Zz, Zz }, (IS_JCC | REL_B), s1R },
  { e_jz,         t_done, 0, false, { Jb, Zz, Zz }, (IS_JCC | REL_B), s1R },
  { e_jnz,        t_done, 0, false, { Jb, Zz, Zz }, (IS_JCC | REL_B), s1R },
  { e_jbe,        t_done, 0, false, { Jb, Zz, Zz }, (IS_JCC | REL_B), s1R },
  { e_jnbe,       t_done, 0, false, { Jb, Zz, Zz }, (IS_JCC | REL_B), s1R },
  /* 78 */
  { e_js,   t_done, 0, false, { Jb, Zz, Zz }, (IS_JCC | REL_B), s1R },
  { e_jns,  t_done, 0, false, { Jb, Zz, Zz }, (IS_JCC | REL_B), s1R },
  { e_jp,   t_done, 0, false, { Jb, Zz, Zz }, (IS_JCC | REL_B), s1R },
  { e_jnp,  t_done, 0, false, { Jb, Zz, Zz }, (IS_JCC | REL_B), s1R },
  { e_jl,   t_done, 0, false, { Jb, Zz, Zz }, (IS_JCC | REL_B), s1R },
  { e_jnl,  t_done, 0, false, { Jb, Zz, Zz }, (IS_JCC | REL_B), s1R },
  { e_jle,  t_done, 0, false, { Jb, Zz, Zz }, (IS_JCC | REL_B), s1R },
  { e_jnle, t_done, 0, false, { Jb, Zz, Zz }, (IS_JCC | REL_B), s1R },
  /* 80 */
  { e_No_Entry, t_grp, Grp1a, true, { Zz, Zz, Zz }, 0, 0 },
  { e_No_Entry, t_grp, Grp1b, true, { Zz, Zz, Zz }, 0, 0 },
  { e_No_Entry, t_grp, Grp1c, true, { Zz, Zz, Zz }, 0, 0 }, // book says Grp1 however;sandpile.org agrees.
  { e_No_Entry, t_grp, Grp1d, true, { Zz, Zz, Zz }, 0, 0 },
  { e_test, t_done, 0, true, { Eb, Gb, Zz }, 0, s1R2R },
  { e_test, t_done, 0, true, { Ev, Gv, Zz }, 0, s1R2R },
  { e_xchg, t_done, 0, true, { Eb, Gb, Zz }, 0, s1RW2RW },
  { e_xchg, t_done, 0, true, { Ev, Gv, Zz }, 0, s1RW2RW },
  /* 88 */
  { e_mov, t_done, 0, true, { Eb, Gb, Zz }, 0, s1W2R },
  { e_mov, t_done, 0, true, { Ev, Gv, Zz }, 0, s1W2R },
  { e_mov, t_done, 0, true, { Gb, Eb, Zz }, 0, s1W2R },
  { e_mov, t_done, 0, true, { Gv, Ev, Zz }, 0, s1W2R },
  { e_mov, t_done, 0, true, { Ew, Sw, Zz }, 0, s1W2R },
  { e_lea, t_done, 0, true, { Gv, Mlea, Zz }, 0, s1W2R }, // this is just M in the book
                                                        // AFAICT the 2nd operand is not accessed
  { e_mov, t_done, 0, true, { Sw, Ew, Zz }, 0, s1W2R },
  { e_pop, t_done, 0, true, { Ev, STPv, eSP }, 0, s1W2R3RW },
  /* 90 */
  { e_nop,  t_done, 0, false, { Zz, Zz, Zz }, 0, sNONE }, // actually xchg eax,eax
  { e_xchg, t_done, 0, false, { eCX, eAX, Zz }, 0, s1RW2RW },
  { e_xchg, t_done, 0, false, { eDX, eAX, Zz }, 0, s1RW2RW },
  { e_xchg, t_done, 0, false, { eBX, eAX, Zz }, 0, s1RW2RW },
  { e_xchg, t_done, 0, false, { eSP, eAX, Zz }, 0, s1RW2RW },
  { e_xchg, t_done, 0, false, { eBP, eAX, Zz }, 0, s1RW2RW },
  { e_xchg, t_done, 0, false, { eSI, eAX, Zz }, 0, s1RW2RW },
  { e_xchg, t_done, 0, false, { eDI, eAX, Zz }, 0, s1RW2RW },
  /* 98 */
  { e_cbw_cwde, t_done, 0, false, { eAX, Zz, Zz }, 0, s1RW },
  { e_cwd_cdq,  t_done, 0, false, { eDX, eAX, Zz }, 0, s1W2R },
  { e_call,     t_done, 0, false, { Ap, Zz, Zz }, IS_CALL | PTR_WX, s1R },
  { e_wait,     t_done, 0, false, { Zz, Zz, Zz }, 0, sNONE },
  { e_pushf_d, t_done, 0, false, { STHv, Fv, eSP }, 0, s1W2R3RW },
  { e_popf_d,  t_done, 0, false, { Fv, STPv, eSP }, 0, s1W2R3RW },
  { e_sahf,     t_done, 0, false, { Zz, Zz, Zz }, 0, 0 }, // FIXME Intel
  { e_lahf,     t_done, 0, false, { Zz, Zz, Zz }, 0, 0 }, // FIXME Intel
  /* A0 */
  { e_mov,   t_done, 0, false, { AL, Ob, Zz },  0, s1W2R },
  { e_mov,   t_done, 0, false, { eAX, Ov, Zz }, 0, s1W2R },
  { e_mov,   t_done, 0, false, { Ob, AL, Zz },  0, s1W2R },
  { e_mov,   t_done, 0, false, { Ov, eAX, Zz }, 0, s1W2R },
  // XXX: Xv is source, Yv is destination for movs, so they're swapped!
  { e_movsb, t_done, 0, false, { Yb, Xb, Zz },  0, s1W2R | (fREP << FPOS) }, // (e)SI/DI changed
  { e_movsw_d, t_done, 0, false, { Yv, Xv, Zz }, 0, s1W2R | (fREP << FPOS) },
  { e_cmpsb, t_done, 0, false, { Xb, Yb, Zz },  0, s1R2R | (fCMPS << FPOS) },
  { e_cmpsw, t_done, 0, false, { Xv, Yv, Zz },  0, s1R2R | (fCMPS << FPOS) },
  /* A8 */
  { e_test,     t_done, 0, false, { AL, Ib, Zz },  0, s1R2R },
  { e_test,     t_done, 0, false, { eAX, Iz, Zz }, 0, s1R2R },
  { e_stosb,    t_done, 0, false, { Yb, AL, Zz },  0, s1W2R | (fREP << FPOS) },
  { e_stosw_d,  t_done, 0, false, { Yv, eAX, Zz }, 0, s1W2R | (fREP << FPOS) },
  { e_lodsb,    t_done, 0, false, { AL, Xb, Zz },  0, s1W2R | (fREP << FPOS) },
  { e_lodsw,    t_done, 0, false, { eAX, Xv, Zz }, 0, s1W2R | (fREP << FPOS) },
  { e_scasb,    t_done, 0, false, { AL, Yb, Zz },  0, s1R2R | (fSCAS << FPOS) },
  { e_scasw_d,  t_done, 0, false, { eAX, Yv, Zz }, 0, s1R2R | (fSCAS << FPOS) },
  /* B0 */
  { e_mov, t_done, 0, false, { AL, Ib, Zz }, 0, s1W2R },
  { e_mov, t_done, 0, false, { CL, Ib, Zz }, 0, s1W2R },
  { e_mov, t_done, 0, false, { DL, Ib, Zz }, 0, s1W2R },
  { e_mov, t_done, 0, false, { BL, Ib, Zz }, 0, s1W2R },
  { e_mov, t_done, 0, false, { AH, Ib, Zz }, 0, s1W2R },
  { e_mov, t_done, 0, false, { CH, Ib, Zz }, 0, s1W2R },
  { e_mov, t_done, 0, false, { DH, Ib, Zz }, 0, s1W2R },
  { e_mov, t_done, 0, false, { BH, Ib, Zz }, 0, s1W2R },
  /* B8 */
  { e_mov, t_done, 0, false, { eAX, Iv, Zz }, 0, s1W2R },
  { e_mov, t_done, 0, false, { eCX, Iv, Zz }, 0, s1W2R },
  { e_mov, t_done, 0, false, { eDX, Iv, Zz }, 0, s1W2R },
  { e_mov, t_done, 0, false, { eBX, Iv, Zz }, 0, s1W2R },
  { e_mov, t_done, 0, false, { eSP, Iv, Zz }, 0, s1W2R },
  { e_mov, t_done, 0, false, { eBP, Iv, Zz }, 0, s1W2R },
  { e_mov, t_done, 0, false, { eSI, Iv, Zz }, 0, s1W2R },
  { e_mov, t_done, 0, false, { eDI, Iv, Zz }, 0, s1W2R },
  /* C0 */
  { e_No_Entry, t_grp, Grp2, true, { Eb, Ib, Zz }, 0, s1RW2R },
  { e_No_Entry, t_grp, Grp2, true, { Ev, Ib, Zz }, 0, s1RW2R },
  { e_ret_near, t_done, 0, false, { Iw, Zz, Zz }, (IS_RET | IS_RETC), s1R | (fNEARRET << FPOS) },
  { e_ret_near, t_done, 0, false, { Zz, Zz, Zz }, (IS_RET), fNEARRET << FPOS },
  { e_les,      t_done, 0, true, { ES, Gv, Mp }, 0, s1W2W3R },
  { e_lds,      t_done, 0, true, { DS, Gv, Mp }, 0, s1W2W3R },
  { e_No_Entry, t_grp, Grp11, true, { Eb, Ib, Zz }, 0, s1W2R },
  { e_No_Entry, t_grp, Grp11, true, { Ev, Iz, Zz }, 0, s1W2R },
  /* C8 */
  { e_enter,   t_done, 0, false, { Iw, Ib, Zz }, 0, fENTER << FPOS },
  { e_leave,   t_done, 0, false, { Zz, Zz, Zz }, 0, fLEAVE << FPOS },
  { e_ret_far, t_done, 0, false, { Iw, Zz, Zz }, (IS_RETF | IS_RETC), fFARRET << FPOS },
  { e_ret_far, t_done, 0, false, { Zz, Zz, Zz }, (IS_RETF), fFARRET << FPOS },
  { e_int3,   t_done, 0, false, { Zz, Zz, Zz }, 0, sNONE },
  { e_int,     t_done, 0, false, { Ib, Zz, Zz }, 0, sNONE },
  { e_into,    t_done, 0, false, { Zz, Zz, Zz }, 0, sNONE },
  { e_iret,    t_done, 0, false, { Zz, Zz, Zz }, (IS_RET), fIRET << FPOS},
  /* D0 */
  { e_No_Entry, t_grp, Grp2, true, { Eb, Zz, Zz }, 0, s1RW }, // const1
  { e_No_Entry, t_grp, Grp2, true, { Ev, Zz, Zz }, 0, s1RW }, // --"--
  { e_No_Entry, t_grp, Grp2, true, { Eb, CL, Zz }, 0, s1RW2R },
  { e_No_Entry, t_grp, Grp2, true, { Ev, CL, Zz }, 0, s1RW2R },
  { e_aam,  t_done, 0, false, { AX, Ib, Zz }, 0, s1RW2R },
  { e_aad,  t_done, 0, false, { AX, Ib, Zz }, 0, s1RW2R },
  { e_salc, t_done, 0, false, { Zz, Zz, Zz }, 0, sNONE }, // undocumeted
  { e_xlat, t_done, 0, false, { Zz, Zz, Zz }, 0, fXLAT << FPOS }, // scream
  /* D8 */
  { e_No_Entry, t_coprocEsc, 0, true, { Zz, Zz, Zz }, 0, 0 },
  { e_No_Entry, t_coprocEsc, 0, true, { Zz, Zz, Zz }, 0, 0 },
  { e_No_Entry, t_coprocEsc, 0, true, { Zz, Zz, Zz }, 0, 0 },
  { e_No_Entry, t_coprocEsc, 0, true, { Zz, Zz, Zz }, 0, 0 },
  { e_No_Entry, t_coprocEsc, 0, true, { Zz, Zz, Zz }, 0, 0 },
  { e_No_Entry, t_coprocEsc, 0, true, { Zz, Zz, Zz }, 0, 0 },
  { e_No_Entry, t_coprocEsc, 0, true, { Zz, Zz, Zz }, 0, 0 },
  { e_No_Entry, t_coprocEsc, 0, true, { Zz, Zz, Zz }, 0, 0 },
  /* E0 */
  { e_loopn,    t_done, 0, false, { Jb, eCX, Zz }, 0, s1R2R }, // aren't these conditional jumps?
  { e_loope,    t_done, 0, false, { Jb, eCX, Zz }, 0, s1R2R },
  { e_loop,     t_done, 0, false, { Jb, eCX, Zz }, 0, s1R2R },
  { e_jcxz_jec, t_done, 0, false, { Jb, eCX, Zz }, (IS_JCC | REL_B), s1R2R },
  { e_in,       t_done, 0, false, { AL, Ib, Zz }, 0, s1W2R | fIO << FPOS },
  { e_in,       t_done, 0, false, { eAX, Ib, Zz }, 0, s1W2R | fIO << FPOS },
  { e_out,      t_done, 0, false, { Ib, AL, Zz }, 0, s1W2R | fIO << FPOS },
  { e_out,      t_done, 0, false, { Ib, eAX, Zz }, 0, s1W2R | fIO << FPOS },
  /* E8 */
  { e_call, t_done, 0, false, { Jz, Zz, Zz }, (IS_CALL | REL_X), fCALL << FPOS },
  { e_jmp,  t_done, 0, false, { Jz, Zz, Zz }, (IS_JUMP | REL_X), s1R },
  { e_jmp,  t_done, 0, false, { Ap, Zz, Zz }, (IS_JUMP | PTR_WX), s1R },
  { e_jmp,  t_done, 0, false, { Jb, Zz, Zz }, (IS_JUMP | REL_B), s1R },
  { e_in,   t_done, 0, false, { AL, DX, Zz }, 0, s1W2R | (fIO << FPOS) },
  { e_in,   t_done, 0, false, { eAX, DX, Zz }, 0, s1W2R | (fIO << FPOS) },
  { e_out,  t_done, 0, false, { DX, AL, Zz }, 0, s1W2R | (fIO << FPOS) },
  { e_out,  t_done, 0, false, { DX, eAX, Zz }, 0, s1W2R | (fIO << FPOS) },
  /* F0 */
  { e_No_Entry,      t_ill,  0, false, { Zz, Zz, Zz }, 0, 0 }, // PREFIX_INSTR
  { e_int1, t_done, 0, false, { Zz, Zz, Zz }, 0, sNONE }, // undocumented
  { e_No_Entry, t_prefixedSSE, 3, false, { Zz, Zz, Zz }, 0, 0 },
  { e_No_Entry, t_prefixedSSE, 1, false, { Zz, Zz, Zz }, 0, 0 },
  { e_hlt,  t_done, 0, false, { Zz, Zz, Zz }, PRVLGD, sNONE },
  { e_cmc,  t_done, 0, false, { Zz, Zz, Zz }, 0, sNONE },
  { e_No_Entry, t_grp, Grp3a, true, { Zz, Zz, Zz }, 0, sNONE },
  { e_No_Entry, t_grp, Grp3b, true, { Zz, Zz, Zz }, 0, sNONE },
  /* F8 */
  { e_clc, t_done, 0, false, { Zz, Zz, Zz }, 0, sNONE },
  { e_stc, t_done, 0, false, { Zz, Zz, Zz }, 0, sNONE },
  { e_cli, t_done, 0, false, { Zz, Zz, Zz }, 0, sNONE },
  { e_sti, t_done, 0, false, { Zz, Zz, Zz }, 0, sNONE },
  { e_cld, t_done, 0, false, { Zz, Zz, Zz }, 0, sNONE },
  { e_std, t_done, 0, false, { Zz, Zz, Zz }, 0, sNONE },
  { e_No_Entry, t_grp, Grp4, true, { Zz, Zz, Zz }, 0, sNONE },
  { e_No_Entry, t_grp, Grp5, true, { Zz, Zz, Zz }, 0, sNONE }
};


// twoByteMap: two byte opcode instructions (first byte is 0x0F)
static ia32_entry twoByteMap[256] = {
  /* 00 */
  // Syscall/sysret are somewhat hacked
  // Syscall on amd64 will also read CS/SS for where to call in 32-bit mode
  { e_No_Entry, t_grp, Grp6, true, { Zz, Zz, Zz }, 0, 0 },
  { e_No_Entry, t_grp, Grp7, false, { Zz, Zz, Zz }, 0, 0 },
  { e_lar,        t_done, 0, true, { Gv, Ew, Zz }, 0, s1W2R | (fSEGDESC << FPOS) },
  { e_lsl,        t_done, 0, true, { Gv, Ew, Zz }, 0, s1W2R | (fSEGDESC << FPOS) },
  { e_No_Entry,            t_ill,  0, false, { Zz, Zz, Zz }, 0, 0 },
  { e_syscall,    t_done, 0, false, { eCX, Zz, Zz }, 0, s1W }, // AMD: writes return address to eCX; for liveness, treat as hammering all
  { e_clts,       t_done, 0, false, { Zz, Zz, Zz }, 0, sNONE },
  { e_sysret,     t_done, 0, false, { eCX, Zz, Zz }, 0, s1R }, // AMD; reads return address from eCX; unlikely to occur in Dyninst use cases but we'll be paranoid
  /* 08 */
  { e_invd,   t_done, 0, false, { Zz, Zz, Zz }, 0, sNONE }, // only in priviledge 0, so ignored
  { e_wbinvd, t_done, 0, false, { Zz, Zz, Zz }, 0, sNONE }, // idem
  { e_No_Entry,        t_ill,  0, false, { Zz, Zz, Zz }, 0, 0 },
  { e_ud2,    t_ill,  0, 0, { Zz, Zz, Zz }, 0, 0 },
  { e_No_Entry,        t_ill,  0, 0, { Zz, Zz, Zz }, 0, 0 },
  { e_prefetch_w, t_grp, GrpAMD, true, { Zz, Zz, Zz }, 0, 0 },    // AMD prefetch group
  { e_femms,       t_done,  0, false, { Zz, Zz, Zz }, 0, sNONE },  // AMD specific
  // semantic is bogus for the 1st operand - but correct for the 2nd,
  // which is the only one that can be a memory operand :)
  // fixing the 1st operand requires an extra table for the 3dnow instructions...
  { e_No_Entry,             t_3dnow, 0, true,  { Pq, Qq, Zz }, 0, s1RW2R }, // AMD 3DNow! suffixes
  /* 10 */
  { e_No_Entry, t_sse, SSE10, true, { Zz, Zz, Zz }, 0, 0 },
  { e_No_Entry, t_sse, SSE11, true, { Zz, Zz, Zz }, 0, 0 },
  { e_No_Entry, t_sse, SSE12, true, { Zz, Zz, Zz }, 0, 0 },
  { e_No_Entry, t_sse, SSE13, true, { Zz, Zz, Zz }, 0, 0 },
  { e_No_Entry, t_sse, SSE14, true, { Zz, Zz, Zz }, 0, 0 },
  { e_No_Entry, t_sse, SSE15, true, { Zz, Zz, Zz }, 0, 0 },
  { e_No_Entry, t_sse, SSE16, true, { Zz, Zz, Zz }, 0, 0 },
  { e_No_Entry, t_sse, SSE17, true, { Zz, Zz, Zz }, 0, 0 },
  /* 18 */
  { e_No_Entry, t_grp, Grp16, 0, { Zz, Zz, Zz }, 0, 0 },
  { e_No_Entry, t_ill, 0, 0, { Zz, Zz, Zz }, 0, 0 },
  { e_No_Entry, t_ill, 0, 0, { Zz, Zz, Zz }, 0, 0 },
  { e_No_Entry, t_ill, 0, 0, { Zz, Zz, Zz }, 0, 0 },
  { e_No_Entry, t_ill, 0, 0, { Zz, Zz, Zz }, 0, 0 },
  { e_No_Entry, t_ill, 0, 0, { Zz, Zz, Zz }, 0, 0 },
  { e_No_Entry, t_ill, 0, 0, { Zz, Zz, Zz }, 0, 0 },
  { e_No_Entry, t_ill, 0, 0, { Zz, Zz, Zz }, 0, 0 },
  /* 20 */
  { e_mov, t_done, 0, true, { Rd, Cd, Zz }, 0, s1W2R },
  { e_mov, t_done, 0, true, { Rd, Dd, Zz }, 0, s1W2R },
  { e_mov, t_done, 0, true, { Cd, Rd, Zz }, 0, s1W2R },
  { e_mov, t_done, 0, true, { Dd, Rd, Zz }, 0, s1W2R },
  { e_mov, t_done, 0, true, { Rd, Td, Zz }, 0, s1W2R },
  { e_No_Entry,     t_ill,  0, 0, { Zz, Zz, Zz }, 0, 0 },
  { e_mov, t_done, 0, true, { Td, Rd, Zz }, 0, s1W2R },
  { e_No_Entry,     t_ill,  0, 0, { Zz, Zz, Zz }, 0, 0 },
  /* 28 */
  { e_No_Entry, t_sse, SSE28, true, { Zz, Zz, Zz }, 0, 0 },
  { e_No_Entry, t_sse, SSE29, true, { Zz, Zz, Zz }, 0, 0 },
  { e_No_Entry, t_sse, SSE2A, true, { Zz, Zz, Zz }, 0, 0 },
  { e_No_Entry, t_sse, SSE2B, true, { Zz, Zz, Zz }, 0, 0 },
  { e_No_Entry, t_sse, SSE2C, true, { Zz, Zz, Zz }, 0, 0 },
  { e_No_Entry, t_sse, SSE2D, true, { Zz, Zz, Zz }, 0, 0 },
  { e_No_Entry, t_sse, SSE2E, true, { Zz, Zz, Zz }, 0, 0 },
  { e_No_Entry, t_sse, SSE2F, true, { Zz, Zz, Zz }, 0, 0 },
  /* 30 */
  { e_wrmsr, t_done, 0, false, { Zz, Zz, Zz }, 0, sNONE },
  { e_rdtsc, t_done, 0, false, { Zz, Zz, Zz }, 0, sNONE},
  { e_rdmsr, t_done, 0, false, { Zz, Zz, Zz }, 0, sNONE },
  { e_rdpmc, t_done, 0, false, { Zz, Zz, Zz }, 0, sNONE },
  { e_sysenter, t_done, 0, false, { Zz, Zz, Zz }, 0, sNONE }, // XXX: fixme for kernel work
  { e_sysexit,  t_done, 0, false, { Zz, Zz, Zz }, 0, sNONE }, // XXX: fixme for kernel work
  { e_No_Entry, t_ill, 0, 0, { Zz, Zz, Zz }, 0, 0 }, 
  { e_No_Entry, t_ill, 0, 0, { Zz, Zz, Zz }, 0 ,0 },
  /* 38 */
  { e_No_Entry, t_ill, 0, 0, { Zz, Zz, Zz }, 0 ,0 },
  { e_No_Entry, t_ill, 0, 0, { Zz, Zz, Zz }, 0 ,0 },
  { e_No_Entry, t_ill, 0, 0, { Zz, Zz, Zz }, 0 ,0 },
  { e_No_Entry, t_ill, 0, 0, { Zz, Zz, Zz }, 0 ,0 },
  { e_No_Entry, t_ill, 0, 0, { Zz, Zz, Zz }, 0 ,0 },
  { e_No_Entry, t_ill, 0, 0, { Zz, Zz, Zz }, 0 ,0 },
  { e_No_Entry, t_ill, 0, 0, { Zz, Zz, Zz }, 0 ,0 },
  { e_No_Entry, t_ill, 0, 0, { Zz, Zz, Zz }, 0 ,0 },
  /* 40 */
  { e_cmovo,   t_done, 0, true, { Gv, Ev, Zz }, 0, s1W2R | (fCOND << FPOS) },
  { e_cmovno,  t_done, 0, true, { Gv, Ev, Zz }, 0, s1W2R | (fCOND << FPOS) },
  { e_cmovnae, t_done, 0, true, { Gv, Ev, Zz }, 0, s1W2R | (fCOND << FPOS) },
  { e_cmovnb,  t_done, 0, true, { Gv, Ev, Zz }, 0, s1W2R | (fCOND << FPOS) },
  { e_cmove,   t_done, 0, true, { Gv, Ev, Zz }, 0, s1W2R | (fCOND << FPOS) },
  { e_cmovne,  t_done, 0, true, { Gv, Ev, Zz }, 0, s1W2R | (fCOND << FPOS) },
  { e_cmovbe,  t_done, 0, true, { Gv, Ev, Zz }, 0, s1W2R | (fCOND << FPOS) },
  { e_cmovnbe, t_done, 0, true, { Gv, Ev, Zz }, 0, s1W2R | (fCOND << FPOS) },
  /* 48 */
  { e_cmovs,   t_done, 0, true, { Gv, Ev, Zz }, 0, s1W2R | (fCOND << FPOS) },
  { e_cmovns,  t_done, 0, true, { Gv, Ev, Zz }, 0, s1W2R | (fCOND << FPOS) },
  { e_cmovpe,  t_done, 0, true, { Gv, Ev, Zz }, 0, s1W2R | (fCOND << FPOS) },
  { e_cmovpo,  t_done, 0, true, { Gv, Ev, Zz }, 0, s1W2R | (fCOND << FPOS) },
  { e_cmovnge, t_done, 0, true, { Gv, Ev, Zz }, 0, s1W2R | (fCOND << FPOS) },
  { e_cmovnl,  t_done, 0, true, { Gv, Ev, Zz }, 0, s1W2R | (fCOND << FPOS) },
  { e_cmovng,  t_done, 0, true, { Gv, Ev, Zz }, 0, s1W2R | (fCOND << FPOS) },
  { e_cmovnl,  t_done, 0, true, { Gv, Ev, Zz }, 0, s1W2R | (fCOND << FPOS) },
  /* 50 */
  { e_No_Entry, t_sse, SSE50, true, { Zz, Zz, Zz }, 0, 0 },
  { e_No_Entry, t_sse, SSE51, true, { Zz, Zz, Zz }, 0, 0 },
  { e_No_Entry, t_sse, SSE52, true, { Zz, Zz, Zz }, 0, 0 },
  { e_No_Entry, t_sse, SSE53, true, { Zz, Zz, Zz }, 0, 0 },
  { e_No_Entry, t_sse, SSE54, true, { Zz, Zz, Zz }, 0, 0 },
  { e_No_Entry, t_sse, SSE55, true, { Zz, Zz, Zz }, 0, 0 },
  { e_No_Entry, t_sse, SSE56, true, { Zz, Zz, Zz }, 0, 0 },
  { e_No_Entry, t_sse, SSE57, true, { Zz, Zz, Zz }, 0, 0 },
  /* 58 */
  { e_No_Entry, t_sse, SSE58, true, { Zz, Zz, Zz }, 0, 0 },
  { e_No_Entry, t_sse, SSE59, true, { Zz, Zz, Zz }, 0, 0 },
  { e_No_Entry, t_sse, SSE5A, true, { Zz, Zz, Zz }, 0, 0 },
  { e_No_Entry, t_sse, SSE5B, true, { Zz, Zz, Zz }, 0, 0 },
  { e_No_Entry, t_sse, SSE5C, true, { Zz, Zz, Zz }, 0, 0 },
  { e_No_Entry, t_sse, SSE5D, true, { Zz, Zz, Zz }, 0, 0 },
  { e_No_Entry, t_sse, SSE5E, true, { Zz, Zz, Zz }, 0, 0 },
  { e_No_Entry, t_sse, SSE5F, true, { Zz, Zz, Zz }, 0, 0 },
  /* 60 */
  { e_No_Entry, t_sse, SSE60, true, { Zz, Zz, Zz }, 0, 0 },
  { e_No_Entry, t_sse, SSE61, true, { Zz, Zz, Zz }, 0, 0 },
  { e_No_Entry, t_sse, SSE62, true, { Zz, Zz, Zz }, 0, 0 },
  { e_No_Entry, t_sse, SSE63, true, { Zz, Zz, Zz }, 0, 0 },
  { e_No_Entry, t_sse, SSE64, true, { Zz, Zz, Zz }, 0, 0 },
  { e_No_Entry, t_sse, SSE65, true, { Zz, Zz, Zz }, 0, 0 },
  { e_No_Entry, t_sse, SSE66, true, { Zz, Zz, Zz }, 0, 0 },
  { e_No_Entry, t_sse, SSE67, true, { Zz, Zz, Zz }, 0, 0 },
  /* 68 */
  { e_No_Entry, t_sse, SSE68, true, { Zz, Zz, Zz }, 0, 0 },
  { e_No_Entry, t_sse, SSE69, true, { Zz, Zz, Zz }, 0, 0 },
  { e_No_Entry, t_sse, SSE6A, true, { Zz, Zz, Zz }, 0, 0 },
  { e_No_Entry, t_sse, SSE6B, true, { Zz, Zz, Zz }, 0, 0 },
  { e_No_Entry, t_sse, SSE6C, true, { Zz, Zz, Zz }, 0, 0 },
  { e_No_Entry, t_sse, SSE6D, true, { Zz, Zz, Zz }, 0, 0 },
  { e_No_Entry, t_sse, SSE6E, true, { Zz, Zz, Zz }, 0, 0 },
  { e_No_Entry, t_sse, SSE6F, true, { Zz, Zz, Zz }, 0, 0 },
  /* 70 */
  { e_No_Entry, t_sse, SSE70, true, { Zz, Zz, Zz }, 0, 0 },
  { e_No_Entry, t_grp, Grp12, false, { Zz, Zz, Zz }, 0, 0 },
  { e_No_Entry, t_grp, Grp13, false, { Zz, Zz, Zz }, 0, 0 },
  { e_No_Entry, t_grp, Grp14, false, { Zz, Zz, Zz }, 0, 0 },
  { e_No_Entry, t_sse, SSE74, true, { Zz, Zz, Zz }, 0, 0 },
  { e_No_Entry, t_sse, SSE75, true, { Zz, Zz, Zz }, 0, 0 },
  { e_No_Entry, t_sse, SSE76, true, { Zz, Zz, Zz }, 0, 0 },
  { e_emms, t_done, 0, false, { Zz, Zz, Zz }, 0, sNONE },
  /* 78 */
  { e_mmxud, t_ill, 0, 0, { Zz, Zz, Zz }, 0, 0 },
  { e_mmxud, t_ill, 0, 0, { Zz, Zz, Zz }, 0, 0 },
  { e_mmxud, t_ill, 0, 0, { Zz, Zz, Zz }, 0, 0 },
  { e_mmxud, t_ill, 0, 0, { Zz, Zz, Zz }, 0, 0 },
  { e_mmxud, t_ill, 0, 0, { Zz, Zz, Zz }, 0, 0 },
  { e_mmxud, t_ill, 0, 0, { Zz, Zz, Zz }, 0, 0 },
  { e_No_Entry, t_sse, SSE7E, 0, { Zz, Zz, Zz }, 0, 0 },
  { e_No_Entry, t_sse, SSE7F, 0, { Zz, Zz, Zz }, 0, 0 },
  /* 80 */
  { e_jo,   t_done, 0, false, { Jz, Zz, Zz }, (IS_JCC | REL_X), s1R | (fCOND << FPOS) },
  { e_jno,  t_done, 0, false, { Jz, Zz, Zz }, (IS_JCC | REL_X), s1R | (fCOND << FPOS) },
  { e_jb,   t_done, 0, false, { Jz, Zz, Zz }, (IS_JCC | REL_X), s1R | (fCOND << FPOS) },
  { e_jnb,  t_done, 0, false, { Jz, Zz, Zz }, (IS_JCC | REL_X), s1R | (fCOND << FPOS) },
  { e_jz,   t_done, 0, false, { Jz, Zz, Zz }, (IS_JCC | REL_X), s1R | (fCOND << FPOS) },
  { e_jnz,  t_done, 0, false, { Jz, Zz, Zz }, (IS_JCC | REL_X), s1R | (fCOND << FPOS) },
  { e_jbe,  t_done, 0, false, { Jz, Zz, Zz }, (IS_JCC | REL_X), s1R | (fCOND << FPOS) },
  { e_jnbe, t_done, 0, false, { Jz, Zz, Zz }, (IS_JCC | REL_X), s1R | (fCOND << FPOS) },
  /* 88 */
  { e_js,   t_done, 0, false, { Jz, Zz, Zz }, (IS_JCC | REL_X), s1R | (fCOND << FPOS) },
  { e_jns,  t_done, 0, false, { Jz, Zz, Zz }, (IS_JCC | REL_X), s1R | (fCOND << FPOS) },
  { e_jp,   t_done, 0, false, { Jz, Zz, Zz }, (IS_JCC | REL_X), s1R | (fCOND << FPOS) },
  { e_jnp,  t_done, 0, false, { Jz, Zz, Zz }, (IS_JCC | REL_X), s1R | (fCOND << FPOS) },
  { e_jl,   t_done, 0, false, { Jz, Zz, Zz }, (IS_JCC | REL_X), s1R | (fCOND << FPOS) },
  { e_jnl,  t_done, 0, false, { Jz, Zz, Zz }, (IS_JCC | REL_X), s1R | (fCOND << FPOS) },
  { e_jle,  t_done, 0, false, { Jz, Zz, Zz }, (IS_JCC | REL_X), s1R | (fCOND << FPOS) },
  { e_jnle, t_done, 0, false, { Jz, Zz, Zz }, (IS_JCC | REL_X), s1R | (fCOND << FPOS) },
  /* 90 */
  { e_seto,   t_done, 0, true, { Eb, Zz, Zz }, 0, s1W | (fCOND << FPOS) },
  { e_setno,  t_done, 0, true, { Eb, Zz, Zz }, 0, s1W | (fCOND << FPOS) },
  { e_setb,   t_done, 0, true, { Eb, Zz, Zz }, 0, s1W | (fCOND << FPOS) },
  { e_setnb,  t_done, 0, true, { Eb, Zz, Zz }, 0, s1W | (fCOND << FPOS) },
  { e_setz,   t_done, 0, true, { Eb, Zz, Zz }, 0, s1W | (fCOND << FPOS) },
  { e_setnz,  t_done, 0, true, { Eb, Zz, Zz }, 0, s1W | (fCOND << FPOS) },
  { e_setbe,  t_done, 0, true, { Eb, Zz, Zz }, 0, s1W | (fCOND << FPOS) },
  { e_setnbe, t_done, 0, true, { Eb, Zz, Zz }, 0, s1W | (fCOND << FPOS) },
  /* 98 */
  { e_sets,   t_done, 0, true, { Eb, Zz, Zz }, 0, s1W | (fCOND << FPOS) },
  { e_setns,  t_done, 0, true, { Eb, Zz, Zz }, 0, s1W | (fCOND << FPOS) },
  { e_setp,   t_done, 0, true, { Eb, Zz, Zz }, 0, s1W | (fCOND << FPOS) },
  { e_setnp,  t_done, 0, true, { Eb, Zz, Zz }, 0, s1W | (fCOND << FPOS) },
  { e_setl,   t_done, 0, true, { Eb, Zz, Zz }, 0, s1W | (fCOND << FPOS) },
  { e_setnl,  t_done, 0, true, { Eb, Zz, Zz }, 0, s1W | (fCOND << FPOS) },
  { e_setle,  t_done, 0, true, { Eb, Zz, Zz }, 0, s1W | (fCOND << FPOS) },
  { e_setnle, t_done, 0, true, { Eb, Zz, Zz }, 0, s1W | (fCOND << FPOS) },
  /* A0 */
  { e_push,   t_done, 0, false, { STHw, FS, eSP }, 0, s1W2R3RW },
  { e_pop,    t_done, 0, false, { FS, STPw, eSP }, 0, s1W2R3RW },
  { e_cpuid,  t_done, 0, false, { Zz, Zz, Zz }, 0, sNONE },
  { e_bt,     t_done, 0, true, { Ev, Gv, Zz }, 0, s1R2R },
  { e_shld,   t_done, 0, true, { Ev, Gv, Ib }, 0, s1RW2R3R },
  { e_shld,   t_done, 0, true, { Ev, Gv, CL }, 0, s1RW2R3R },
  { e_No_Entry, t_ill, 0, 0, { Zz, Zz, Zz }, 0, 0 }, 
  { e_No_Entry, t_ill, 0, 0, { Zz, Zz, Zz }, 0, 0 },
  /* A8 */
  { e_push, t_done, 0, false, { STHw, GS, eSP }, 0, s1W2R3RW },
  { e_pop,  t_done, 0, false, { GS, STPw, eSP }, 0, s1W2R3RW },
  { e_rsm,  t_done, 0, false, { Zz, Zz, Zz }, 0, sNONE },
  { e_bts,  t_done, 0, true, { Ev, Gv, Zz }, 0, s1RW2R },
  { e_shrd, t_done, 0, true, { Ev, Gv, Ib }, 0, s1RW2R3R },
  { e_shrd, t_done, 0, true, { Ev, Gv, CL }, 0, s1RW2R3R },
  { e_No_Entry, t_grp, Grp15, 0, { Zz, Zz, Zz }, 0, 0 }, 
  { e_imul, t_done, 0, true, { Gv, Ev, Zz }, 0, s1RW2R },
  /* B0 */
  // Assuming this is used with LOCK prefix, the destination gets a write anyway
  // This is not the case without lock prefix, but I ignore that case
  // Also, given that the 3rd operand is a register I ignore that it may be written
  { e_cmpxch, t_done, 0, true, { Eb, Gb, AL }, 0, s1RW2R3R | (fCMPXCH << FPOS) },
  { e_cmpxch, t_done, 0, true, { Ev, Gv, eAX }, 0, s1RW2R3R | (fCMPXCH << FPOS) },
  { e_lss, t_done, 0, true, { SS, Gv, Mp }, 0, s1W2W3R },
  { e_btr, t_done, 0, true, { Ev, Gv, Zz }, 0, s1RW2R },
  { e_lfs, t_done, 0, true, { FS, Gv, Mp }, 0, s1W2W3R },
  { e_lgs, t_done, 0, true, { GS, Gv, Mp }, 0, s1W2W3R },
  { e_movzx, t_done, 0, true, { Gv, Eb, Zz }, 0, s1W2R },
  { e_movzx, t_done, 0, true, { Gv, Ew, Zz }, 0, s1W2R },
  /* B8 */
  { e_No_Entry, t_ill, 0, 0, { Zz, Zz, Zz }, 0, 0 },
  { e_ud2grp10, t_ill, 0, 0, { Zz, Zz, Zz }, 0, sNONE },
  { e_No_Entry, t_grp, Grp8, true, { Zz, Zz, Zz }, 0, 0 },
  { e_btc, t_done, 0, true, { Ev, Gv, Zz }, 0, s1RW2R },
  { e_bsf, t_done, 0, true, { Gv, Ev, Zz }, 0, s1W2R },
  { e_bsr, t_done, 0, true, { Gv, Ev, Zz }, 0, s1W2R },
  { e_movsx, t_done, 0, true, { Gv, Eb, Zz }, 0, s1W2R },
  { e_movsx, t_done, 0, true, { Gv, Ew, Zz }, 0, s1W2R },
  /* C0 */
  { e_xadd, t_done, 0, true, { Eb, Gb, Zz }, 0, s1RW2RW },
  { e_xadd, t_done, 0, true, { Ev, Gv, Zz }, 0, s1RW2RW },
  { e_No_Entry, t_sse, SSEC2, true, { Zz, Zz, Zz }, 0, 0 },
  { e_movnti , t_done, 0, 0, { Ev, Gv, Zz }, 0, s1W2R | (fNT << FPOS) },
  { e_No_Entry, t_sse, SSEC4, true, { Zz, Zz, Zz }, 0, 0 },
  { e_No_Entry, t_sse, SSEC5, true, { Zz, Zz, Zz }, 0, 0 },
  { e_No_Entry, t_sse, SSEC6, true, { Zz, Zz, Zz }, 0, 0 },
  { e_No_Entry, t_grp, Grp9,  true, { Zz, Zz, Zz }, 0, 0 },
  /* C8 */
  { e_bswap, t_done, 0, false, { EAX, Zz, Zz }, 0, s1RW }, 
  { e_bswap, t_done, 0, false, { ECX, Zz, Zz }, 0, s1RW },
  { e_bswap, t_done, 0, false, { EDX, Zz, Zz }, 0, s1RW }, 
  { e_bswap, t_done, 0, false, { EBX, Zz, Zz }, 0, s1RW }, 
  { e_bswap, t_done, 0, false, { ESP, Zz, Zz }, 0, s1RW },
  { e_bswap, t_done, 0, false, { EBP, Zz, Zz }, 0, s1RW }, 
  { e_bswap, t_done, 0, false, { ESI, Zz, Zz }, 0, s1RW }, 
  { e_bswap, t_done, 0, false, { EDI, Zz, Zz }, 0, s1RW }, 
  /* D0 */
  { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
  { e_No_Entry, t_sse, SSED1, true, { Zz, Zz, Zz }, 0, 0 },
  { e_No_Entry, t_sse, SSED2, true, { Zz, Zz, Zz }, 0, 0 },
  { e_No_Entry, t_sse, SSED3, true, { Zz, Zz, Zz }, 0, 0 },
  { e_No_Entry, t_sse, SSED4, true, { Zz, Zz, Zz }, 0, 0 },
  { e_No_Entry, t_sse, SSED5, true, { Zz, Zz, Zz }, 0, 0 },
  { e_No_Entry, t_sse, SSED6, true, { Zz, Zz, Zz }, 0, 0 },
  { e_No_Entry, t_sse, SSED7, true, { Zz, Zz, Zz }, 0, 0 },
  /* D8 */
  { e_No_Entry, t_sse, SSED8, true, { Zz, Zz, Zz }, 0, 0 },
  { e_No_Entry, t_sse, SSED9, true, { Zz, Zz, Zz }, 0, 0 },
  { e_No_Entry, t_sse, SSEDA, true, { Zz, Zz, Zz }, 0, 0 },
  { e_No_Entry, t_sse, SSEDB, true, { Zz, Zz, Zz }, 0, 0 },
  { e_No_Entry, t_sse, SSEDC, true, { Zz, Zz, Zz }, 0, 0 },
  { e_No_Entry, t_sse, SSEDD, true, { Zz, Zz, Zz }, 0, 0 },
  { e_No_Entry, t_sse, SSEDE, true, { Zz, Zz, Zz }, 0, 0 },
  { e_No_Entry, t_sse, SSEDF, true, { Zz, Zz, Zz }, 0, 0 },
  /* E0 */
  { e_No_Entry, t_sse, SSEE0, true, { Zz, Zz, Zz }, 0, 0 },
  { e_No_Entry, t_sse, SSEE1, true, { Zz, Zz, Zz }, 0, 0 },
  { e_No_Entry, t_sse, SSEE2, true, { Zz, Zz, Zz }, 0, 0 },
  { e_No_Entry, t_sse, SSEE3, true, { Zz, Zz, Zz }, 0, 0 },
  { e_No_Entry, t_sse, SSEE4, true, { Zz, Zz, Zz }, 0, 0 },
  { e_No_Entry, t_sse, SSEE5, true, { Zz, Zz, Zz }, 0, 0 },
  { e_No_Entry, t_sse, SSEE6, true, { Zz, Zz, Zz }, 0, 0 },
  { e_No_Entry, t_sse, SSEE7, true, { Zz, Zz, Zz }, 0, 0 },
  /* E8 */
  { e_No_Entry, t_sse, SSEE8, true, { Zz, Zz, Zz }, 0, 0 },
  { e_No_Entry, t_sse, SSEE9, true, { Zz, Zz, Zz }, 0, 0 },
  { e_No_Entry, t_sse, SSEEA, true, { Zz, Zz, Zz }, 0, 0 },
  { e_No_Entry, t_sse, SSEEB, true, { Zz, Zz, Zz }, 0, 0 },
  { e_No_Entry, t_sse, SSEEC, true, { Zz, Zz, Zz }, 0, 0 },
  { e_No_Entry, t_sse, SSEED, true, { Zz, Zz, Zz }, 0, 0 },
  { e_No_Entry, t_sse, SSEEE, true, { Zz, Zz, Zz }, 0, 0 },
  { e_No_Entry, t_sse, SSEEF, true, { Zz, Zz, Zz }, 0, 0 },
  /* F0 */
  { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
  { e_No_Entry, t_sse, SSEF1, true, { Zz, Zz, Zz }, 0, 0 },
  { e_No_Entry, t_sse, SSEF2, true, { Zz, Zz, Zz }, 0, 0 },
  { e_No_Entry, t_sse, SSEF3, true, { Zz, Zz, Zz }, 0, 0 },
  { e_No_Entry, t_sse, SSEF4, true, { Zz, Zz, Zz }, 0, 0 },
  { e_No_Entry, t_sse, SSEF5, true, { Zz, Zz, Zz }, 0, 0 },
  { e_No_Entry, t_sse, SSEF6, true, { Zz, Zz, Zz }, 0, 0 },
  { e_No_Entry, t_sse, SSEF7, true, { Zz, Zz, Zz }, 0, 0 },
  /* F8 */
  { e_No_Entry, t_sse, SSEF8, true, { Zz, Zz, Zz }, 0, 0 },
  { e_No_Entry, t_sse, SSEF9, true, { Zz, Zz, Zz }, 0, 0 },
  { e_No_Entry, t_sse, SSEFA, true, { Zz, Zz, Zz }, 0, 0 },
  { e_No_Entry, t_sse, SSEFB, true, { Zz, Zz, Zz }, 0, 0 },
  { e_No_Entry, t_sse, SSEFC, true, { Zz, Zz, Zz }, 0, 0 },
  { e_No_Entry, t_sse, SSEFD, true, { Zz, Zz, Zz }, 0, 0 },
  { e_No_Entry, t_sse, SSEFE, true, { Zz, Zz, Zz }, 0, 0 },
  { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 }
};

static ia32_entry groupMap[][8] = {
  { /* group 1a */
    { e_add, t_done, 0, true, { Eb, Ib, Zz }, 0, s1RW2R },
    { e_or,  t_done, 0, true, { Eb, Ib, Zz }, 0, s1RW2R },
    { e_adc, t_done, 0, true, { Eb, Ib, Zz }, 0, s1RW2R },
    { e_sbb, t_done, 0, true, { Eb, Ib, Zz }, 0, s1RW2R },
    { e_and, t_done, 0, true, { Eb, Ib, Zz }, 0, s1RW2R },
    { e_sub, t_done, 0, true, { Eb, Ib, Zz }, 0, s1RW2R },
    { e_xor, t_done, 0, true, { Eb, Ib, Zz }, 0, s1RW2R },
    { e_cmp, t_done, 0, true, { Eb, Ib, Zz }, 0, s1R2R },
  },
  { /* group 1b */
    { e_add, t_done, 0, true, { Ev, Iz, Zz }, 0, s1RW2R },
    { e_or,  t_done, 0, true, { Ev, Iz, Zz }, 0, s1RW2R },
    { e_adc, t_done, 0, true, { Ev, Iz, Zz }, 0, s1RW2R },
    { e_sbb, t_done, 0, true, { Ev, Iz, Zz }, 0, s1RW2R },
    { e_and, t_done, 0, true, { Ev, Iz, Zz }, 0, s1RW2R },
    { e_sub, t_done, 0, true, { Ev, Iz, Zz }, 0, s1RW2R },
    { e_xor, t_done, 0, true, { Ev, Iz, Zz }, 0, s1RW2R },
    { e_cmp, t_done, 0, true, { Ev, Iz, Zz }, 0, s1R2R },
  },
  { /* group 1c */
    { e_add, t_done, 0, true, { Eb, Ib, Zz }, 0, s1RW2R },
    { e_or,  t_done, 0, true, { Eb, Ib, Zz }, 0, s1RW2R },
    { e_adc, t_done, 0, true, { Eb, Ib, Zz }, 0, s1RW2R },
    { e_sbb, t_done, 0, true, { Eb, Ib, Zz }, 0, s1RW2R },
    { e_and, t_done, 0, true, { Eb, Ib, Zz }, 0, s1RW2R },
    { e_sub, t_done, 0, true, { Eb, Ib, Zz }, 0, s1RW2R },
    { e_xor, t_done, 0, true, { Eb, Ib, Zz }, 0, s1RW2R },
    { e_cmp, t_done, 0, true, { Eb, Ib, Zz }, 0, s1R2R },
  },
  { /* group 1d */
    { e_add, t_done, 0, true, { Ev, Ib, Zz }, 0, s1RW2R },
    { e_or,  t_done, 0, true, { Ev, Ib, Zz }, 0, s1RW2R },
    { e_adc, t_done, 0, true, { Ev, Ib, Zz }, 0, s1RW2R },
    { e_sbb, t_done, 0, true, { Ev, Ib, Zz }, 0, s1RW2R },
    { e_and, t_done, 0, true, { Ev, Ib, Zz }, 0, s1RW2R },
    { e_sub, t_done, 0, true, { Ev, Ib, Zz }, 0, s1RW2R },
    { e_xor, t_done, 0, true, { Ev, Ib, Zz }, 0, s1RW2R },
    { e_cmp, t_done, 0, true, { Ev, Ib, Zz }, 0, s1R2R },
  },


 {  /* group 2 - only opcode is defined here, 
       operands are defined in the one or two byte maps above */
  { e_rol, t_done, 0, true, { Zz, Zz, Zz }, 0, 0 },
  { e_ror, t_done, 0, true, { Zz, Zz, Zz }, 0, 0 },
  { e_rcl, t_done, 0, true, { Zz, Zz, Zz }, 0, 0 },
  { e_rcr, t_done, 0, true, { Zz, Zz, Zz }, 0, 0 },
  { e_shl_sal, t_done, 0, true, { Zz, Zz, Zz }, 0, 0 },
  { e_shr, t_done, 0, true, { Zz, Zz, Zz }, 0, 0 },
  { e_No_Entry, t_ill, 0, true, { Zz, Zz, Zz }, 0, 0 },
  { e_sar, t_done, 0, true, { Zz, Zz, Zz }, 0, 0 }
 },

 { /* group 3a - operands are defined here */
  { e_test, t_done, 0, true, { Eb, Ib, Zz }, 0, s1R2R },
  { e_No_Entry, t_ill, 0, true, { Zz, Zz, Zz }, 0, 0 },
  { e_not,  t_done, 0, true, { Eb, Zz, Zz }, 0, s1RW },
  { e_neg,  t_done, 0, true, { Eb, Zz, Zz }, 0, s1RW },
  { e_mul,  t_done, 0, true, { AX, AL, Eb }, 0, s1W2RW3R },
  { e_imul, t_done, 0, true, { AX, AL, Eb }, 0, s1W2R3R },
  { e_div,  t_done, 0, true, { AX, AL, Eb }, 0, s1RW2R3R },
  { e_idiv, t_done, 0, true, { AX, AL, Eb }, 0, s1W2R3R }
 },

 { /* group 3b - operands are defined here */
  { e_test, t_done, 0, true, { Ev, Iz, Zz }, 0, s1R2R },
  { e_No_Entry, t_ill, 0, true, { Zz, Zz, Zz }, 0, 0 },
  { e_not,  t_done, 0, true, { Ev, Zz, Zz }, 0, s1RW },
  { e_neg,  t_done, 0, true, { Ev, Zz, Zz }, 0, s1RW },
  { e_mul,  t_done, 0, true, { eDX, eAX, Ev }, 0, s1W2RW3R },
  { e_imul, t_done, 0, true, { eDX, eAX, Ev }, 0, s1W2RW3R },
  { e_div,  t_done, 0, true, { eDX, eAX, Ev }, 0, s1RW2RW3R },
  { e_idiv, t_done, 0, true, { eDX, eAX, Ev }, 0, s1RW2RW3R }
 },

 { /* group 4 - operands are defined here */
  { e_inc, t_done, 0, true, { Eb, Zz, Zz }, 0, s1RW },
  { e_dec, t_done, 0, true, { Eb, Zz, Zz }, 0, s1RW },
  { e_No_Entry, t_ill, 0, true, { Zz, Zz, Zz }, 0, 0 },
  { e_No_Entry, t_ill, 0, true, { Zz, Zz, Zz }, 0, 0 },
  { e_No_Entry, t_ill, 0, true, { Zz, Zz, Zz }, 0, 0 },
  { e_No_Entry, t_ill, 0, true, { Zz, Zz, Zz }, 0, 0 },
  { e_No_Entry, t_ill, 0, true, { Zz, Zz, Zz }, 0, 0 },
  { e_No_Entry, t_ill, 0, true, { Zz, Zz, Zz }, 0, 0 },
 },

 { /* group 5 - operands are defined here */
  { e_inc,  t_done, 0, true, { Ev, Zz, Zz }, 0, s1RW },
  { e_dec,  t_done, 0, true, { Ev, Zz, Zz }, 0, s1RW },
  { e_call, t_done, 0, true, { Ev, Zz, Zz }, (IS_CALL | INDIR), s1R | (fINDIRCALL << FPOS) },
  { e_call, t_done, 0, true, { Ep, Zz, Zz }, (IS_CALL | INDIR), s1R | (fINDIRCALL << FPOS) },
  { e_jmp,  t_done, 0, true, { Ev, Zz, Zz }, (IS_JUMP | INDIR), s1R | (fINDIRJUMP << FPOS) },
  { e_jmp,  t_done, 0, true, { Ep, Zz, Zz }, (IS_JUMP | INDIR), s1R | (fINDIRJUMP << FPOS) },
  { e_push, t_done, 0, true, { Ev, Zz, Zz }, 0, 0 },
  { e_No_Entry, t_ill, 0, true, { Zz, Zz, Zz }, 0, 0 },
 },

 { /* group 6 - operands are defined here */
   // these need to be fixed for kernel mode accesses
  { e_sldt, t_done, 0, true, { Ew, Zz, Zz }, 0, s1W },
  { e_str,  t_done, 0, true, { Ew, Zz, Zz }, 0, s1W },
  { e_lldt, t_done, 0, true, { Ew, Zz, Zz }, 0, s1R },
  { e_ltr,  t_done, 0, true, { Ew, Zz, Zz }, 0, s1R },
  { e_verr, t_done, 0, true, { Ew, Zz, Zz }, 0, s1R },
  { e_verw, t_done, 0, true, { Ew, Zz, Zz }, 0, s1R },
  { e_No_Entry, t_ill, 0, true, { Zz, Zz, Zz }, 0, 0 },
  { e_No_Entry, t_ill, 0, true, { Zz, Zz, Zz }, 0, 0 },
 },

 { /* group 7 - operands are defined here */
   // idem
  { e_sgdt, t_done, 0, true, { Ms, Zz, Zz }, 0, s1W },
  { e_sidt, t_done, 0, true, { Ms, Zz, Zz }, 0, s1W },
  { e_lgdt, t_done, 0, true, { Ms, Zz, Zz }, 0, s1R },
  { e_lidt, t_done, 0, true, { Ms, Zz, Zz }, 0, s1R },
  { e_smsw, t_done, 0, true, { Ew, Zz, Zz }, 0, s1W },
  { e_No_Entry, t_ill, 0, true, { Zz, Zz, Zz }, 0, 0 },
  { e_lmsw, t_done, 0, true, { Ew, Zz, Zz }, 0, s1R },
  { e_invlpg, t_done, 0, true, { Zz, Zz, Zz }, 0, sNONE }, // 64-bit: swapgs also uses this encoding (w/ mod=11)
 },

 { /* group 8 - only opcode is defined here, 
     operands are defined in the one or two byte maps below */
  { e_No_Entry, t_ill, 0, true, { Zz, Zz, Zz }, 0, 0 },
  { e_No_Entry, t_ill, 0, true, { Zz, Zz, Zz }, 0, 0 },
  { e_No_Entry, t_ill, 0, true, { Zz, Zz, Zz }, 0, 0 },
  { e_No_Entry, t_ill, 0, true, { Zz, Zz, Zz }, 0, 0 },
  { e_bt,  t_done, 0, true, { Ev, Ib, Zz }, 0, s1R2R },
  { e_bts, t_done, 0, true, { Ev, Ib, Zz }, 0, s1RW2R },
  { e_btr, t_done, 0, true, { Ev, Ib, Zz }, 0, s1RW2R },
  { e_btc, t_done, 0, true, { Ev, Ib, Zz }, 0, s1RW2R },
 },

 { /* group 9 - operands are defined here */
  { e_No_Entry, t_ill, 0, true, { Zz, Zz, Zz }, 0, 0 },
  // see comments for cmpxch
  { e_cmpxch8b, t_done, 0, true, { EDXEAX, Mq, ECXEBX }, 0, s1RW2RW3R | (fCMPXCH8 << FPOS) },
  { e_No_Entry, t_ill, 0, true, { Zz, Zz, Zz }, 0, 0 },
  { e_No_Entry, t_ill, 0, true, { Zz, Zz, Zz }, 0, 0 },
  { e_No_Entry, t_ill, 0, true, { Zz, Zz, Zz }, 0, 0 },
  { e_No_Entry, t_ill, 0, true, { Zz, Zz, Zz }, 0, 0 },
  { e_No_Entry, t_ill, 0, true, { Zz, Zz, Zz }, 0, 0 },
  { e_No_Entry, t_ill, 0, true, { Zz, Zz, Zz }, 0, 0 }
 },

 /* group 10 is all illegal */

 { /* group 11, opcodes defined in one byte map */
   { e_mov, t_done, 0, true, { Zz, Zz, Zz }, 0, sNONE },
   { e_No_Entry, t_ill, 0, true, { Zz, Zz, Zz }, 0, 0 },
   { e_No_Entry, t_ill, 0, true, { Zz, Zz, Zz }, 0, 0 },
   { e_No_Entry, t_ill, 0, true, { Zz, Zz, Zz }, 0, 0 },
   { e_No_Entry, t_ill, 0, true, { Zz, Zz, Zz }, 0, 0 },
   { e_No_Entry, t_ill, 0, true, { Zz, Zz, Zz }, 0, 0 },
   { e_No_Entry, t_ill, 0, true, { Zz, Zz, Zz }, 0, 0 },
   { e_No_Entry, t_ill, 0, true, { Zz, Zz, Zz }, 0, 0 },
 }

};


// Groups 12-16 are split by mod={mem,11}. Some spill over into SSE groups!
// Notation: G12SSE010B = group 12, SSE, reg=010; B means mod=11
// Use A if Intel decides to put SSE instructions for mod=mem
static ia32_entry groupMap2[][2][8] = {
  { /* group 12 */
    {
      { e_No_Entry, t_ill, 0, true, { Zz, Zz, Zz }, 0, 0 },
      { e_No_Entry, t_ill, 0, true, { Zz, Zz, Zz }, 0, 0 },
      { e_No_Entry, t_ill, 0, true, { Zz, Zz, Zz }, 0, 0 },
      { e_No_Entry, t_ill, 0, true, { Zz, Zz, Zz }, 0, 0 },
      { e_No_Entry, t_ill, 0, true, { Zz, Zz, Zz }, 0, 0 },
      { e_No_Entry, t_ill, 0, true, { Zz, Zz, Zz }, 0, 0 },
      { e_No_Entry, t_ill, 0, true, { Zz, Zz, Zz }, 0, 0 },
      { e_No_Entry, t_ill, 0, true, { Zz, Zz, Zz }, 0, 0 },
    },
    {
      { e_No_Entry, t_ill, 0, true, { Zz, Zz, Zz }, 0, 0 },
      { e_No_Entry, t_ill, 0, true, { Zz, Zz, Zz }, 0, 0 },
      { e_No_Entry, t_grpsse, G12SSE010B, true, { Zz, Zz, Zz }, 0, 0 },
      { e_No_Entry, t_ill, 0, true, { Zz, Zz, Zz }, 0, 0 },
      { e_No_Entry, t_grpsse, G12SSE100B, true, { Zz, Zz, Zz }, 0, 0 },
      { e_No_Entry, t_ill, 0, true, { Zz, Zz, Zz }, 0, 0 },
      { e_No_Entry, t_grpsse, G12SSE110B, true, { Zz, Zz, Zz }, 0, 0 },
      { e_No_Entry, t_ill, 0, true, { Zz, Zz, Zz }, 0, 0 },
    }
  },
  { /* group 13 */
    {
      { e_No_Entry, t_ill, 0, true, { Zz, Zz, Zz }, 0, 0 },
      { e_No_Entry, t_ill, 0, true, { Zz, Zz, Zz }, 0, 0 },
      { e_No_Entry, t_ill, 0, true, { Zz, Zz, Zz }, 0, 0 },
      { e_No_Entry, t_ill, 0, true, { Zz, Zz, Zz }, 0, 0 },
      { e_No_Entry, t_ill, 0, true, { Zz, Zz, Zz }, 0, 0 },
      { e_No_Entry, t_ill, 0, true, { Zz, Zz, Zz }, 0, 0 },
      { e_No_Entry, t_ill, 0, true, { Zz, Zz, Zz }, 0, 0 },
      { e_No_Entry, t_ill, 0, true, { Zz, Zz, Zz }, 0, 0 },
    },
    {
      { e_No_Entry, t_ill, 0, true, { Zz, Zz, Zz }, 0, 0 },
      { e_No_Entry, t_ill, 0, true, { Zz, Zz, Zz }, 0, 0 },
      { e_No_Entry, t_grpsse, G13SSE010B, true, { Zz, Zz, Zz }, 0, 0 },
      { e_No_Entry, t_ill, 0, true, { Zz, Zz, Zz }, 0, 0 },
      { e_No_Entry, t_grpsse, G13SSE100B, true, { Zz, Zz, Zz }, 0, 0 },
      { e_No_Entry, t_ill, 0, true, { Zz, Zz, Zz }, 0, 0 },
      { e_No_Entry, t_grpsse, G13SSE110B, true, { Zz, Zz, Zz }, 0, 0 },
      { e_No_Entry, t_ill, 0, true, { Zz, Zz, Zz }, 0, 0 },
    }
  },
  { /* group 14 */
    {
      { e_No_Entry, t_ill, 0, true, { Zz, Zz, Zz }, 0, 0 },
      { e_No_Entry, t_ill, 0, true, { Zz, Zz, Zz }, 0, 0 },
      { e_No_Entry, t_ill, 0, true, { Zz, Zz, Zz }, 0, 0 },
      { e_No_Entry, t_ill, 0, true, { Zz, Zz, Zz }, 0, 0 },
      { e_No_Entry, t_ill, 0, true, { Zz, Zz, Zz }, 0, 0 },
      { e_No_Entry, t_ill, 0, true, { Zz, Zz, Zz }, 0, 0 },
      { e_No_Entry, t_ill, 0, true, { Zz, Zz, Zz }, 0, 0 },
      { e_No_Entry, t_ill, 0, true, { Zz, Zz, Zz }, 0, 0 },
    },
    {
      { e_No_Entry, t_ill, 0, true, { Zz, Zz, Zz }, 0, 0 },
      { e_No_Entry, t_ill, 0, true, { Zz, Zz, Zz }, 0, 0 },
      { e_No_Entry, t_grpsse, G14SSE010B, true, { Zz, Zz, Zz }, 0, 0 },
      { e_No_Entry, t_grpsse, G14SSE011B, true, { Zz, Zz, Zz }, 0, 0 },
      { e_No_Entry, t_ill, 0, true, { Zz, Zz, Zz }, 0, 0 },
      { e_No_Entry, t_ill, 0, true, { Zz, Zz, Zz }, 0, 0 },
      { e_No_Entry, t_grpsse, G14SSE110B, true, { Zz, Zz, Zz }, 0, 0 },
      { e_No_Entry, t_grpsse, G14SSE111B, true, { Zz, Zz, Zz }, 0, 0 },
    }
  },
  { /* group 15 */
    {
      { e_fxsave,  t_done, 0, true, { M512, Zz, Zz }, 0, s1W | (fFXSAVE << FPOS) },
      { e_fxrstor, t_done, 0, true, { M512, Zz, Zz }, 0, s1R | (fFXRSTOR << FPOS) },
      { e_ldmxcsr, t_done, 0, true, { Md, Zz, Zz }, 0, s1R },
      { e_stmxcsr, t_done, 0, true, { Md, Zz, Zz }, 0, s1W },
      { e_No_Entry, t_ill, 0, true, { Zz, Zz, Zz }, 0, 0 },
      { e_No_Entry, t_ill, 0, true, { Zz, Zz, Zz }, 0, 0 },
      { e_No_Entry, t_ill, 0, true, { Zz, Zz, Zz }, 0, 0 },
      { e_clflush, t_done, 0, true, { Mb, Zz, Zz }, 0, s1W | (fCLFLUSH << FPOS) },
    },
    {
      { e_No_Entry, t_ill, 0, true, { Zz, Zz, Zz }, 0, 0 },
      { e_No_Entry, t_ill, 0, true, { Zz, Zz, Zz }, 0, 0 },
      { e_No_Entry, t_ill, 0, true, { Zz, Zz, Zz }, 0, 0 },
      { e_No_Entry, t_ill, 0, true, { Zz, Zz, Zz }, 0, 0 },
      { e_No_Entry, t_ill, 0, true, { Zz, Zz, Zz }, 0, 0 },
      { e_lfence, t_done, 0, true, { Zz, Zz, Zz }, 0, sNONE },
      { e_mfence, t_done, 0, true, { Zz, Zz, Zz }, 0, sNONE },
      { e_sfence, t_done, 0, true, { Zz, Zz, Zz }, 0, sNONE },
    }
  },
  { /* group 16 */
    {
      { e_prefetchNTA, t_done, 0, true, { Mb, Zz, Zz }, 0, s1R | (fPREFETCHNT << FPOS) },
      { e_prefetchT0,  t_done, 0, true, { Mb, Zz, Zz }, 0, s1R | (fPREFETCHT0 << FPOS) },
      { e_prefetchT1,  t_done, 0, true, { Mb, Zz, Zz }, 0, s1R | (fPREFETCHT1 << FPOS) },
      { e_prefetchT2,  t_done, 0, true, { Mb, Zz, Zz }, 0, s1R | (fPREFETCHT1 << FPOS) },
      { e_No_Entry, t_ill, 0, true, { Zz, Zz, Zz }, 0, 0 },
      { e_No_Entry, t_ill, 0, true, { Zz, Zz, Zz }, 0, 0 },
      { e_No_Entry, t_ill, 0, true, { Zz, Zz, Zz }, 0, 0 },
      { e_No_Entry, t_ill, 0, true, { Zz, Zz, Zz }, 0, 0 },
    },
    {
      { e_No_Entry, t_ill, 0, true, { Zz, Zz, Zz }, 0, 0 },
      { e_No_Entry, t_ill, 0, true, { Zz, Zz, Zz }, 0, 0 },
      { e_No_Entry, t_ill, 0, true, { Zz, Zz, Zz }, 0, 0 },
      { e_No_Entry, t_ill, 0, true, { Zz, Zz, Zz }, 0, 0 },
      { e_No_Entry, t_ill, 0, true, { Zz, Zz, Zz }, 0, 0 },
      { e_No_Entry, t_ill, 0, true, { Zz, Zz, Zz }, 0, 0 },
      { e_No_Entry, t_ill, 0, true, { Zz, Zz, Zz }, 0, 0 },
      { e_No_Entry, t_ill, 0, true, { Zz, Zz, Zz }, 0, 0 },
    }
  },
  { /* AMD prefetch group */
    {
      { e_prefetch,   t_done, 0, true, { Mb, Zz, Zz }, 0, s1R | (fPREFETCHAMDE << FPOS) },
      { e_prefetchw,  t_done, 0, true, { Mb, Zz, Zz }, 0, s1R | (fPREFETCHAMDW << FPOS) },
      { e_No_Entry, t_ill, 0, true, { Zz, Zz, Zz }, 0, 0 }, // this is reserved, not illegal, ugh...
      { e_No_Entry, t_ill, 0, true, { Zz, Zz, Zz }, 0, 0 }, // this is reserved, not illegal, ugh...
      { e_No_Entry, t_ill, 0, true, { Zz, Zz, Zz }, 0, 0 }, // this is reserved, not illegal, ugh...
      { e_No_Entry, t_ill, 0, true, { Zz, Zz, Zz }, 0, 0 }, // this is reserved, not illegal, ugh...
      { e_No_Entry, t_ill, 0, true, { Zz, Zz, Zz }, 0, 0 }, // this is reserved, not illegal, ugh...
      { e_No_Entry, t_ill, 0, true, { Zz, Zz, Zz }, 0, 0 }, // this is reserved, not illegal, ugh...
    },
    {
      { e_No_Entry, t_ill, 0, true, { Zz, Zz, Zz }, 0, 0 },
      { e_No_Entry, t_ill, 0, true, { Zz, Zz, Zz }, 0, 0 },
      { e_No_Entry, t_ill, 0, true, { Zz, Zz, Zz }, 0, 0 },
      { e_No_Entry, t_ill, 0, true, { Zz, Zz, Zz }, 0, 0 },
      { e_No_Entry, t_ill, 0, true, { Zz, Zz, Zz }, 0, 0 },
      { e_No_Entry, t_ill, 0, true, { Zz, Zz, Zz }, 0, 0 },
      { e_No_Entry, t_ill, 0, true, { Zz, Zz, Zz }, 0, 0 },
      { e_No_Entry, t_ill, 0, true, { Zz, Zz, Zz }, 0, 0 },
    }
  }
};

/* rows are not, F3, 66, F2 prefixed in this order (see book) */
static ia32_entry sseMap[][4] = {
  { /* SSE10 */
    { e_movups, t_done, 0, true, { Vps, Wps, Zz }, 0, s1W2R },
    { e_movss,  t_done, 0, true, { Vss, Wss, Zz }, 0, s1W2R },
    { e_movupd, t_done, 0, true, { Vpd, Wpd, Zz }, 0, s1W2R },
    { e_movsd,  t_done, 0, true, { Vsd, Wsd, Zz }, 0, s1W2R },
  },
  { /* SSE11 */
    { e_movups, t_done, 0, true, { Wps, Vps, Zz }, 0, s1W2R },
    { e_movss,  t_done, 0, true, { Wss, Vss, Zz }, 0, s1W2R },
    { e_movupd, t_done, 0, true, { Wpd, Vpd, Zz }, 0, s1W2R },
    { e_movsd,  t_done, 0, true, { Vsd, Wsd, Zz }, 0, s1W2R }, // FIXME: bug in book?????
  },
  { /* SSE12 */
    { e_movlps_movhlps, t_done, 0, true, { Wq, Vq, Zz }, 0, s1W2R }, // FIXME: wierd 1st op
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
    { e_movlpd, t_done, 0, true, { Vq, Ws, Zz }, 0, s1W2R },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
  },
  { /* SSE13 */
    { e_movlps, t_done, 0, true, { Vq, Wq, Zz }, 0, s1W2R },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
    { e_movlpd, t_done, 0, true, { Vq, Wq, Zz }, 0, s1W2R },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
  },
  { /* SSE14 */
    { e_unpcklps, t_done, 0, true, { Vps, Wq, Zz }, 0, s1RW2R },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
    { e_unpcklpd, t_done, 0, true, { Vpd, Wq, Zz }, 0, s1RW2R },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
  },
  { /* SSE15 */
    { e_unpckhps, t_done, 0, true, { Vps, Wq, Zz }, 0, s1RW2R },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
    { e_unpckhpd, t_done, 0, true, { Vpd, Wq, Zz }, 0, s1RW2R },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
  },
  { /* SSE16 */
    { e_movhps_movlhps, t_done, 0, true, { Vq, Wq, Zz }, 0, s1W2R }, // FIXME: wierd 2nd op
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
    { e_movhpd, t_done, 0, true, { Vq, Wq, Zz }, 0, s1W2R },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
  },
  { /* SSE17 */
    { e_movhps, t_done, 0, true, { Wq, Vq, Zz }, 0, s1W2R },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
    { e_movhpd, t_done, 0, true, { Wq, Vq, Zz }, 0, s1W2R },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
  },
  { /* SSE28 */
    { e_movaps, t_done, 0, true, { Vps, Wps, Zz }, 0, s1W2R },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
    { e_movapd, t_done, 0, true, { Vpd, Wpd, Zz }, 0, s1W2R },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
  },
  { /* SSE29 */
    { e_movaps, t_done, 0, true, { Wps, Vps, Zz }, 0, s1W2R },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
    { e_movapd, t_done, 0, true, { Wpd, Vpd, Zz }, 0, s1W2R },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
  },
  { /* SSE2A */
    { e_cvtpi2ps, t_done, 0, true, { Vps, Qq, Zz }, 0, s1W2R },
    { e_cvtsi2ss, t_done, 0, true, { Vss, Ev, Zz }, 0, s1W2R },
    { e_cvtpi2pd, t_done, 0, true, { Vpd, Qdq, Zz }, 0, s1W2R },
    { e_cvtsi2sd, t_done, 0, true, { Vsd, Ev, Zz }, 0, s1W2R },
  },
  { /* SSE2B */
    { e_movntps, t_done, 0, true, { Wps, Vps, Zz }, 0, s1W2R | (fNT << FPOS) },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
    { e_movntpd, t_done, 0, true, { Wpd, Vpd, Zz }, 0, s1W2R | (fNT << FPOS) }, // bug in book
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
  },
  { /* SSE2C */
    { e_cvttps2pi, t_done, 0, true, { Qq, Wps, Zz }, 0, s1W2R },
    { e_cvttss2si, t_done, 0, true, { Gv, Wss, Zz }, 0, s1W2R },
    { e_cvttpd2pi, t_done, 0, true, { Qdq, Wpd, Zz }, 0, s1W2R },
    { e_cvttsd2si, t_done, 0, true, { Gv, Wsd, Zz }, 0, s1W2R },
  },
  { /* SSE2D */
    { e_cvtps2pi, t_done, 0, true, { Qq, Wps, Zz }, 0, s1W2R },
    { e_cvtss2si, t_done, 0, true, { Gv, Wss, Zz }, 0, s1W2R },
    { e_cvtpd2pi, t_done, 0, true, { Qdq, Wpd, Zz }, 0, s1W2R },
    { e_cvtsd2si, t_done, 0, true, { Gv, Wsd, Zz }, 0, s1W2R },
  },
  { /* SSE2E */
    { e_ucomiss, t_done, 0, true, { Vss, Wss, Zz }, 0, s1R2R },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
    { e_ucomisd, t_done, 0, true, { Vsd, Wsd, Zz }, 0, s1R2R },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
  },
  { /* SSE2F */
    { e_comiss, t_done, 0, true, { Vps, Wps, Zz }, 0, s1R2R },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
    { e_comisd, t_done, 0, true, { Vsd, Wsd, Zz }, 0, s1R2R },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
  },
  { /* SSE50 */
    { e_movmskps, t_done, 0, true, { Ed, Vps, Zz }, 0, s1W2R },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
    { e_movmskpd, t_done, 0, true, { Ed, Vpd, Zz }, 0, s1W2R },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
  },
  { /* SSE51 */
    { e_sqrtps, t_done, 0, true, { Vps, Wps, Zz }, 0, s1W2R },
    { e_sqrtss, t_done, 0, true, { Vss, Wss, Zz }, 0, s1W2R },
    { e_sqrtpd, t_done, 0, true, { Vpd, Wpd, Zz }, 0, s1W2R },
    { e_sqrtsd, t_done, 0, true, { Vsd, Wsd, Zz }, 0, s1W2R },
  },
  { /* SSE52 */
    { e_rsqrtps, t_done, 0, true, { Vps, Wps, Zz }, 0, s1W2R },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
    { e_rsqrtss, t_done, 0, true, { Vss, Wss, Zz }, 0, s1W2R },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
  },
  { /* SSE53 */
    { e_rcpps, t_done, 0, true, { Vps, Wps, Zz }, 0, s1W2R },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
    { e_rcpss, t_done, 0, true, { Vss, Wss, Zz }, 0, s1W2R },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
  },
  { /* SSE54 */
    { e_andps, t_done, 0, true, { Vps, Wps, Zz }, 0, s1RW2R },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
    { e_andpd, t_done, 0, true, { Vpd, Wpd, Zz }, 0, s1RW2R },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
  },
  { /* SSE55 */
    { e_andnps, t_done, 0, true, { Vps, Wps, Zz }, 0, s1RW2R },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
    { e_andnpd, t_done, 0, true, { Vpd, Wpd, Zz }, 0, s1RW2R },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
  },
  { /* SSE56 */
    { e_orps, t_done, 0, true, { Vps, Wps, Zz }, 0, s1RW2R },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
    { e_orpd, t_done, 0, true, { Vpd, Wpd, Zz }, 0, s1RW2R },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
  },
  { /* SSE57 */
    { e_xorps, t_done, 0, true, { Vps, Wps, Zz }, 0, s1RW2R },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
    { e_xorpd, t_done, 0, true, { Vpd, Wpd, Zz }, 0, s1RW2R },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
  },
  { /* SSE58 */
    { e_addps, t_done, 0, true, { Vps, Wps, Zz }, 0, s1RW2R },
    { e_addss, t_done, 0, true, { Vss, Wss, Zz }, 0, s1RW2R },
    { e_addpd, t_done, 0, true, { Vpd, Wpd, Zz }, 0, s1RW2R },
    { e_addsd, t_done, 0, true, { Vsd, Wsd, Zz }, 0, s1RW2R },
  },
  { /* SSE59 */
    { e_mulps, t_done, 0, true, { Vps, Wps, Zz }, 0, s1RW2R },
    { e_mulss, t_done, 0, true, { Vss, Wss, Zz }, 0, s1RW2R },
    { e_mulpd, t_done, 0, true, { Vpd, Wpd, Zz }, 0, s1RW2R },
    { e_mulsd, t_done, 0, true, { Vsd, Wsd, Zz }, 0, s1RW2R },
  },
  { /* SSE5A */
    { e_cvtps2pd, t_done, 0, true, { Vpd, Wps, Zz }, 0, s1W2R },
    { e_cvtss2sd, t_done, 0, true, { Vss, Wss, Zz }, 0, s1W2R },
    { e_cvtpd2ps, t_done, 0, true, { Vps, Wpd, Zz }, 0, s1W2R }, // FIXME: book bug ???
    { e_cvtsd2ss, t_done, 0, true, { Vsd, Wsd, Zz }, 0, s1W2R },
  },
  { /* SSE5B */
    { e_cvtdq2ps, t_done, 0, true, { Vps, Wdq, Zz }, 0, s1W2R },
    { e_cvttps2dq, t_done, 0, true, { Vdq, Wps, Zz }, 0, s1W2R }, // book has this/next swapped!!! 
    { e_cvtps2dq, t_done, 0, true, { Vdq, Wps, Zz }, 0, s1W2R },  // FIXME: book bug ???
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
  },
  { /* SSE5C */
    { e_subps, t_done, 0, true, { Vps, Wps, Zz }, 0, s1RW2R },
    { e_subss, t_done, 0, true, { Vss, Wss, Zz }, 0, s1RW2R },
    { e_subpd, t_done, 0, true, { Vpd, Wpd, Zz }, 0, s1RW2R },
    { e_subsd, t_done, 0, true, { Vsd, Wsd, Zz }, 0, s1RW2R },
  },
  { /* SSE5D */
    { e_minps, t_done, 0, true, { Vps, Wps, Zz }, 0, s1RW2R },
    { e_minss, t_done, 0, true, { Vss, Wss, Zz }, 0, s1RW2R },
    { e_minpd, t_done, 0, true, { Vpd, Wpd, Zz }, 0, s1RW2R },
    { e_minsd, t_done, 0, true, { Vsd, Wsd, Zz }, 0, s1RW2R },
  },
  { /* SSE5E */
    { e_divps, t_done, 0, true, { Vps, Wps, Zz }, 0, s1RW2R },
    { e_divss, t_done, 0, true, { Vss, Wss, Zz }, 0, s1RW2R },
    { e_divpd, t_done, 0, true, { Vpd, Wpd, Zz }, 0, s1RW2R },
    { e_divsd, t_done, 0, true, { Vsd, Wsd, Zz }, 0, s1RW2R },
  },
  { /* SSE5F */
    { e_maxps, t_done, 0, true, { Vps, Wps, Zz }, 0, s1RW2R },
    { e_maxss, t_done, 0, true, { Vss, Wss, Zz }, 0, s1RW2R },
    { e_maxpd, t_done, 0, true, { Vpd, Wpd, Zz }, 0, s1RW2R },
    { e_maxsd, t_done, 0, true, { Vsd, Wsd, Zz }, 0, s1RW2R },
  },
  { /* SSE60 */
    { e_punpcklbw, t_done, 0, true, { Pq, Qd, Zz }, 0, s1RW2R },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
    { e_punpcklbw, t_done, 0, true, { Vdq, Wdq, Zz }, 0, s1RW2R },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
  },
  { /* SSE61 */
    { e_punpcklwd, t_done, 0, true, { Pq, Qd, Zz }, 0, s1RW2R },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
    { e_punpcklwd, t_done, 0, true, { Vdq, Wdq, Zz }, 0, s1RW2R },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
  },
  { /* SSE62 */
    { e_punpcklqd, t_done, 0, true, { Pq, Qd, Zz }, 0, s1RW2R },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
    { e_punpcklqd, t_done, 0, true, { Vdq, Wdq, Zz }, 0, s1RW2R },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
  },
  { /* SSE63 */
    { e_packsswb, t_done, 0, true, { Pq, Qq, Zz }, 0, s1RW2R },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
    { e_packsswb, t_done, 0, true, { Vdq, Wdq, Zz }, 0, s1RW2R },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
  },
  { /* SSE64 */
    { e_pcmpgtb, t_done, 0, true, { Pq, Qq, Zz }, 0, s1R2R },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
    { e_pcmpgtb, t_done, 0, true, { Vdq, Wdq, Zz }, 0, s1R2R },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
  },
  { /* SSE65 */
    { e_pcmpgtw, t_done, 0, true, { Pq, Qq, Zz }, 0, s1R2R },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
    { e_pcmpgtw, t_done, 0, true, { Vdq, Wdq, Zz }, 0, s1R2R },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
  },
  { /* SSE66 */
    { e_pcmpgdt, t_done, 0, true, { Pq, Qq, Zz }, 0, s1R2R },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
    { e_pcmpgdt, t_done, 0, true, { Vdq, Wdq, Zz }, 0, s1R2R },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
  },
  { /* SSE67 */
    { e_packuswb, t_done, 0, true, { Pq, Qq, Zz }, 0, s1RW2R },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
    { e_packuswb, t_done, 0, true, { Vdq, Wdq, Zz }, 0, s1RW2R },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
  },
  { /* SSE68 */
    { e_punpckhbw, t_done, 0, true, { Pq, Qq, Zz }, 0, s1RW2R },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
    { e_punpckhbw, t_done, 0, true, { Pdq, Qdq, Zz }, 0, s1RW2R },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
  },
  { /* SSE69 */
    { e_punpckhwd, t_done, 0, true, { Pq, Qq, Zz }, 0, s1RW2R },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
    { e_punpckhwd, t_done, 0, true, { Pdq, Qdq, Zz }, 0, s1RW2R },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
  },
  { /* SSE6A */
    { e_punpckhdq, t_done, 0, true, { Pq, Qq, Zz }, 0, s1RW2R },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
    { e_punpckhdq, t_done, 0, true, { Pdq, Qdq, Zz }, 0, s1RW2R },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
  },
  { /* SSE6B */
    { e_packssdw, t_done, 0, true, { Pq, Qq, Zz }, 0, s1RW2R },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
    { e_packssdw, t_done, 0, true, { Pdq, Qdq, Zz }, 0, s1RW2R },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
  },
  { /* SSE6C */
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
    { e_punpcklqld, t_done, 0, true, { Vdq, Wdq, Zz }, 0, s1RW2R },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
  },
  { /* SSE6D */
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
    { e_punpckhqd, t_done, 0, true, { Vdq, Wdq, Zz }, 0, s1RW2R },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
  },
  { /* SSE6E */
    { e_movd, t_done, 0, true, { Pd, Ev, Zz }, 0, s1W2R },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
    { e_movd, t_done, 0, true, { Vdq, Ev, Zz }, 0, s1W2R },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
  },
  { /* SSE6F */
    { e_movq, t_done, 0, true, { Pq, Qq, Zz }, 0, s1W2R },
    { e_movdqu, t_done, 0, false, { Vdq, Wdq, Zz }, 0, s1W2R }, // book has this/next swapped!!!
    { e_movdqa, t_done, 0, true, { Vdq, Wdq, Zz }, 0, s1W2R },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
  },
  { /* SSE70 */
    { e_pshufw, t_done, 0, true, { Pq, Qq, Ib }, 0, s1W2R3R },
    { e_pshufhw, t_done, 0, true, { Vdq, Wdq, Ib }, 0, s1W2R3R }, // book has this/next swapped!!!
    { e_pshufd, t_done, 0, true, { Vdq, Wdq, Ib }, 0, s1W2R3R },
    { e_pshuflw, t_done, 0, true, { Vdq, Wdq, Ib }, 0, s1W2R3R },
  },
  { /* SSE74 */
    { e_pcmpeqb, t_done, 0, true, { Pq, Qq, Zz }, 0, s1R2R },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
    { e_pcmpeqb, t_done, 0, true, { Vdq, Wdq, Zz }, 0, s1R2R },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
  },
  { /* SSE75 */
    { e_pcmpeqw, t_done, 0, true, { Pq, Qq, Zz }, 0, s1R2R },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
    { e_pcmpeqw, t_done, 0, true, { Vdq, Wdq, Zz }, 0, s1R2R },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
  },
  { /* SSE76 */
    { e_pcmpeqd, t_done, 0, true, { Pq, Qq, Zz }, 0, s1R2R },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
    { e_pcmpeqd, t_done, 0, true, { Vdq, Wdq, Zz }, 0, s1R2R },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
  },
  { /* SSE7E */
    { e_movd, t_done, 0, true, { Ev, Pd, Zz }, 0, s1W2R },
    { e_movq, t_done, 0, true, { Vq, Wq, Zz }, 0, s1W2R }, // book has this and next swapped!!!
    { e_movd, t_done, 0, true, { Ev, Vdq, Zz }, 0, s1W2R },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
  },
  { /* SSE7F */
    { e_movq, t_done, 0, true, { Qq, Pq, Zz }, 0, s1W2R },
    { e_movdqu, t_done, 0, true, { Wdq, Vdq, Zz }, 0, s1W2R }, // book has this and next swapped!!!
    { e_movdqa, t_done, 0, true, { Wdq, Vdq, Zz }, 0, s1W2R },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
  },
  { /* SSEC2 */
    { e_cmpps, t_done, 0, true, { Vps, Wps, Ib }, 0, s1RW2R3R }, // comparison writes to dest!
    { e_cmpss, t_done, 0, true, { Vss, Wss, Ib }, 0, s1RW2R3R },
    { e_cmppd, t_done, 0, true, { Vpd, Wpd, Ib }, 0, s1RW2R3R },
    { e_cmpsd, t_done, 0, true, { Vsd, Wsd, Ib }, 0, s1RW2R3R },
  },
  { /* SSEC4 */
    { e_pinsrw, t_done, 0, true, { Pq, Ed, Ib }, 0, s1RW2R3R },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
    { e_pinsrw, t_done, 0, true, { Vdq, Ed, Ib }, 0, s1RW2R3R },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
  },
  { /* SSEC5 */
    { e_pextrw, t_done, 0, true, { Gd, Pq, Ib }, 0, s1W2R3R },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
    { e_pextrw, t_done, 0, true, { Gd, Vdq, Ib }, 0, s1W2R3R },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
  },
  { /* SSEC6 */
    { e_shufps, t_done, 0, true, { Vps, Wps, Ib }, 0, s1RW2R3R },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
    { e_shufpd, t_done, 0, true, { Vpd, Wpd, Ib }, 0, s1RW2R3R },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
  },
  { /* SSED1 */
    { e_psrlw, t_done, 0, true, { Pq, Qq, Zz }, 0, s1RW2R },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
    { e_psrlw, t_done, 0, true, { Vdq, Wdq, Zz }, 0, s1RW2R },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
  },
  { /* SSED2 */
    { e_psrld, t_done, 0, true, { Pq, Qq, Zz }, 0, s1RW2R },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
    { e_psrld, t_done, 0, true, { Vdq, Wdq, Zz }, 0, s1RW2R },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
  },
  { /* SSED3 */
    { e_psrlq, t_done, 0, true, { Pq, Qq, Zz }, 0, s1RW2R },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
    { e_psrlq, t_done, 0, true, { Vdq, Wdq, Zz }, 0, s1RW2R },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
  },
  { /* SSED4 */
    { e_paddq, t_done, 0, true, { Pq, Qq, Zz }, 0, s1RW2R },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
    { e_paddq, t_done, 0, true, { Vdq, Wdq, Zz }, 0, s1RW2R },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
  },
  { /* SSED5 */
    { e_pmullw, t_done, 0, true, { Pq, Qq, Zz }, 0, s1RW2R },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
    { e_pmullw, t_done, 0, true, { Vdq, Wdq, Zz }, 0, s1RW2R },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
  },
  { /* SSED6 */
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
    { e_movq2dq, t_done, 0, true, { Vdq, Qq, Zz }, 0, s1W2R }, // lines jumbled in book
    { e_movq, t_done, 0, true, { Wq, Vq, Zz }, 0, s1W2R },
    { e_movdq2q, t_done, 0, true, { Pq, Wq, Zz }, 0, s1W2R },
  },
  { /* SSED7 */
    { e_pmovmskb, t_done, 0, true, { Gd, Pq, Zz }, 0, s1W2R },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
    { e_pmovmskb, t_done, 0, true, { Gd, Vdq, Zz }, 0, s1W2R },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
  },
  { /* SSED8 */
    { e_psubusb, t_done, 0, true, { Pq, Qq, Zz }, 0, s1RW2R },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
    { e_psubusb, t_done, 0, true, { Vdq, Wdq, Zz }, 0, s1RW2R },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
  },
  { /* SSED9 */
    { e_psubusw, t_done, 0, true, { Pq, Qq, Zz }, 0, s1RW2R },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
    { e_psubusw, t_done, 0, true, { Vdq, Wdq, Zz }, 0, s1RW2R },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
  },
  { /* SSEDA */
    { e_pminub, t_done, 0, true, { Pq, Qq, Zz }, 0, s1RW2R },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
    { e_pminub, t_done, 0, true, { Vdq, Wdq, Zz }, 0, s1RW2R },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
  },
  { /* SSEDB */
    { e_pand, t_done, 0, true, { Pq, Qq, Zz }, 0, s1RW2R },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
    { e_pand, t_done, 0, true, { Vdq, Wdq, Zz }, 0, s1RW2R },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
  },
  { /* SSEDC */
    { e_paddusb, t_done, 0, true, { Pq, Qq, Zz }, 0, s1RW2R },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
    { e_paddusb, t_done, 0, true, { Vdq, Wdq, Zz }, 0, s1RW2R },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
  },
  { /* SSEDD */
    { e_paddusw, t_done, 0, true, { Pq, Qq, Zz }, 0, s1RW2R },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
    { e_paddusw, t_done, 0, true, { Vdq, Wdq, Zz }, 0, s1RW2R },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
  },
  { /* SSEDE */
    { e_pmaxub, t_done, 0, true, { Pq, Qq, Zz }, 0, s1RW2R },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
    { e_pmaxub, t_done, 0, true, { Vdq, Wdq, Zz }, 0, s1RW2R },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
  },
  { /* SSEDF */
    { e_pandn, t_done, 0, true, { Pq, Qq, Zz }, 0, s1RW2R },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
    { e_pandn, t_done, 0, true, { Vdq, Wdq, Zz }, 0, s1RW2R },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
  },
  { /* SSEE0 */
    { e_pavgb, t_done, 0, true, { Pq, Qq, Zz }, 0, s1RW2R },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
    { e_pavgb, t_done, 0, true, { Vdq, Wdq, Zz }, 0, s1RW2R },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
  },
  { /* SSEE1 */
    { e_psraw, t_done, 0, true, { Pq, Qq, Zz }, 0, s1RW2R },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
    { e_psraw, t_done, 0, true, { Vdq, Wdq, Zz }, 0, s1RW2R },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
  },
  { /* SSEE2 */
    { e_psrad, t_done, 0, true, { Pq, Qq, Zz }, 0, s1RW2R },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
    { e_psrad, t_done, 0, true, { Vdq, Wdq, Zz }, 0, s1RW2R },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
  },
  { /* SSEE3 */
    { e_pavgw, t_done, 0, true, { Pq, Qq, Zz }, 0, s1RW2R },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
    { e_pavgw, t_done, 0, true, { Vdq, Wdq, Zz }, 0, s1RW2R },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
  },
  { /* SSEE4 */
    { e_pmulhuw, t_done, 0, true, { Pq, Qq, Zz }, 0, s1RW2R },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
    { e_pmulhuw, t_done, 0, true, { Vdq, Wdq, Zz }, 0, s1RW2R },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
  },
  { /* SSEE5 */
    { e_pmulhw, t_done, 0, true, { Pq, Qq, Zz }, 0, s1RW2R },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
    { e_pmulhw, t_done, 0, true, { Vdq, Wdq, Zz }, 0, s1RW2R },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
  },
  { /* SSEE6 */
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
    { e_cvtdq2pd, t_done, 0, true, { Vpd, Wdq, Zz }, 0, s1W2R }, // lines jumbled in book
    { e_cvttpd2dq, t_done, 0, true, { Vdq, Wpd, Zz }, 0, s1W2R },
    { e_cvtpd2dq, t_done, 0, true, { Vdq, Wpd, Zz }, 0, s1W2R },
  },
  { /* SSEE7 */
    { e_movntq, t_done, 0, true, { Wq, Vq, Zz }, 0, s1W2R | (fNT << FPOS) },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
    { e_movntdq, t_done, 0, true, { Wdq, Vdq, Zz }, 0, s1W2R | (fNT << FPOS) },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
  },
  { /* SSEE8 */
    { e_psubsb, t_done, 0, true, { Pq, Qq, Zz }, 0, s1RW2R },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
    { e_psubsb, t_done, 0, true, { Vdq, Wdq, Zz }, 0, s1RW2R },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
  },
  { /* SSEE9 */
    { e_psubsw, t_done, 0, true, { Pq, Qq, Zz }, 0, s1RW2R },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
    { e_psubsw, t_done, 0, true, { Vdq, Wdq, Zz }, 0, s1RW2R },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
  },
  { /* SSEEA */
    { e_pminsw, t_done, 0, true, { Pq, Qq, Zz }, 0, s1RW2R },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
    { e_pminsw, t_done, 0, true, { Vdq, Wdq, Zz }, 0, s1RW2R },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
  },
  { /* SSEEB */
    { e_por, t_done, 0, true, { Pq, Qq, Zz }, 0, s1RW2R },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
    { e_por, t_done, 0, true, { Vdq, Wdq, Zz }, 0, s1RW2R },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
  },
  { /* SSEEC */
    { e_paddsb, t_done, 0, true, { Pq, Qq, Zz }, 0, s1RW2R },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
    { e_paddsb, t_done, 0, true, { Vdq, Wdq, Zz }, 0, s1RW2R },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
  },
  { /* SSEED */
    { e_paddsw, t_done, 0, true, { Pq, Qq, Zz }, 0, s1RW2R },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
    { e_paddsw, t_done, 0, true, { Vdq, Wdq, Zz }, 0, s1RW2R },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
  },
  { /* SSEEE */
    { e_pmaxsw, t_done, 0, true, { Pq, Qq, Zz }, 0, s1RW2R },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
    { e_pmaxsw, t_done, 0, true, { Vdq, Wdq, Zz }, 0, s1RW2R },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
  },
  { /* SSEEF */
    { e_pxor, t_done, 0, true, { Pq, Qq, Zz }, 0, s1RW2R },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
    { e_pxor, t_done, 0, true, { Vdq, Wdq, Zz }, 0, s1RW2R },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
  },
  { /* SSEF1 */
    { e_psllw, t_done, 0, true, { Pq, Qq, Zz }, 0, s1RW2R },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
    { e_psllw, t_done, 0, true, { Vdq, Wdq, Zz }, 0, s1RW2R },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
  },
  { /* SSEF2 */
    { e_pslld, t_done, 0, true, { Pq, Qq, Zz }, 0, s1RW2R },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
    { e_pslld, t_done, 0, true, { Vdq, Wdq, Zz }, 0, s1RW2R },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
  },
  { /* SSEF3 */
    { e_psllq, t_done, 0, true, { Pq, Qq, Zz }, 0, s1RW2R },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
    { e_psllq, t_done, 0, true, { Vdq, Wdq, Zz }, 0, s1RW2R },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
  },
  { /* SSEF4 */
    { e_pmuludq, t_done, 0, true, { Pq, Qq, Zz }, 0, s1RW2R },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
    { e_pmuludq, t_done, 0, true, { Vdq, Wdq, Zz }, 0, s1RW2R },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
  },
  { /* SSEF5 */
    { e_pmaddwd, t_done, 0, true, { Pq, Qq, Zz }, 0, s1RW2R },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
    { e_pmaddwd, t_done, 0, true, { Vdq, Wdq, Zz }, 0, s1RW2R },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
  },
  { /* SSEF6 */
    { e_psadbw, t_done, 0, true, { Pq, Qq, Zz }, 0, s1RW2R },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
    { e_psadbw, t_done, 0, true, { Vdq, Wdq, Zz }, 0, s1RW2R },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
  },
  { /* SSEF7 */
    { e_maskmovq, t_done, 0, true, { Ppi, Qpi, Zz }, 0, s1W2R | (fNT << FPOS) },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
    { e_maskmovdqu, t_done, 0, true, { Vdq, Wdq, Zz }, 0, s1W2R | (fNT << FPOS) }, // bug in book
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
  },
  { /* SSEF8 */
    { e_psubb, t_done, 0, true, { Pq, Qq, Zz }, 0, s1RW2R },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
    { e_psubb, t_done, 0, true, { Vdq, Wdq, Zz }, 0, s1RW2R },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
  },
  { /* SSEF9 */
    { e_psubw, t_done, 0, true, { Pq, Qq, Zz }, 0, s1RW2R },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
    { e_psubw, t_done, 0, true, { Vdq, Wdq, Zz }, 0, s1RW2R },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
  },
  { /* SSEFA */
    { e_psubd, t_done, 0, true, { Pq, Qq, Zz }, 0, s1RW2R },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
    { e_psubd, t_done, 0, true, { Vdq, Wdq, Zz }, 0, s1RW2R },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
  },
  { /* SSEFB */ // FIXME: Same????
    { e_psubd, t_done, 0, true, { Pq, Qq, Zz }, 0, s1RW2R },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
    { e_psubd, t_done, 0, true, { Vdq, Wdq, Zz }, 0, s1RW2R },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
  },
  { /* SSEFC */
    { e_paddb, t_done, 0, true, { Pq, Qq, Zz }, 0, s1RW2R },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
    { e_paddb, t_done, 0, true, { Vdq, Wdq, Zz }, 0, s1RW2R },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
  },
  { /* SSEFD */
    { e_paddw, t_done, 0, true, { Pq, Qq, Zz }, 0, s1RW2R },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
    { e_paddw, t_done, 0, true, { Vdq, Wdq, Zz }, 0, s1RW2R },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
  },
  { /* SSEFE */
    { e_paddd, t_done, 0, true, { Pq, Qq, Zz }, 0, s1RW2R },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
    { e_paddd, t_done, 0, true, { Vdq, Wdq, Zz }, 0, s1RW2R },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
  }
};

/* rows are none or 66 prefixed in this order (see book) */
static ia32_entry ssegrpMap[][2] = {
  /* G12SSE010B */
  {
    { e_psrlw, t_done, 0, true, { Pq, Ib, Zz }, 0, s1RW2R },
    { e_psrlw, t_done, 0, true, { Pdq, Ib, Zz }, 0, s1RW2R }
  },
  /* G12SSE100B */
  {
    { e_psraw, t_done, 0, true, { Pq, Ib, Zz }, 0, s1RW2R },
    { e_psraw, t_done, 0, true, { Pdq, Ib, Zz }, 0, s1RW2R }
  },
  /* G12SSE110B */
  {
    { e_psllw, t_done, 0, true, { Pq, Ib, Zz }, 0, s1RW2R },
    { e_psllw, t_done, 0, true, { Pdq, Ib, Zz }, 0, s1RW2R }
  },
  /* G13SSE010B */
  {
    { e_psrld, t_done, 0, true, { Pq, Ib, Zz }, 0, s1RW2R },
    { e_psrld, t_done, 0, true, { Wdq, Ib, Zz }, 0, s1RW2R }
  },
  /* G13SSE100B */
  {
    { e_psrad, t_done, 0, true, { Pq, Ib, Zz }, 0, s1RW2R },
    { e_psrad, t_done, 0, true, { Wdq, Ib, Zz }, 0, s1RW2R }
  },
  /* G13SSE110B */
  {
    { e_pslld, t_done, 0, true, { Pq, Ib, Zz }, 0, s1RW2R },
    { e_pslld, t_done, 0, true, { Wdq, Ib, Zz }, 0, s1RW2R }
  },
  /* G14SSE010B */
  {
    { e_psrlq, t_done, 0, true, { Pq, Ib, Zz }, 0, s1RW2R },
    { e_psrlq, t_done, 0, true, { Wdq, Ib, Zz }, 0, s1RW2R }
  },
  /* G14SSE011B */
  {
    { e_No_Entry, t_ill, 0, true, { Zz, Zz, Zz }, 0, 0 },
    { e_psrldq, t_done, 0, true, { Wdq, Ib, Zz }, 0, s1RW2R }
  },
  /* G14SSE110B */
  {
    { e_psllq, t_done, 0, true, { Pq, Ib, Zz }, 0, s1RW2R },
    { e_psllq, t_done, 0, true, { Wdq, Ib, Zz }, 0, s1RW2R }
  },
  /* G14SSE111B */
  {
    { e_No_Entry, t_ill, 0, true, { Zz, Zz, Zz }, 0, 0 },
    { e_pslldq, t_done, 0, true, { Wdq, Ib, Zz }, 0, s1RW2R }
  }
};

static bool mode_64 = false;

void ia32_set_mode_64(bool mode) {
  mode_64 = mode;
}

bool ia32_is_mode_64() {
  return mode_64;
}

ia32_entry movsxd = { e_movsxd, t_done, 0, true, { Gv, Ed, Zz }, 0, s1W2R };

static void ia32_translate_for_64(ia32_entry** gotit_ptr)
{
    if (*gotit_ptr == &oneByteMap[0x63]) // APRL redefined to MOVSXD
	*gotit_ptr = &movsxd;
}

/* full decoding version: supports memory access information */
static unsigned int ia32_decode_modrm(const unsigned int addrSzAttr,
                                      const unsigned char* addr,
                                      ia32_memacc* macadr,
                                      const ia32_prefixes* pref,
                                      ia32_locations *pos);

/* shortcut version: just decodes instruction size info */
static unsigned int ia32_decode_modrm(const unsigned int addrSzAttr,
				      const unsigned char* addr)
{
    return ia32_decode_modrm(addrSzAttr, addr, NULL, NULL, NULL);
}

void ia32_memacc::print()
{
    fprintf(stderr, "base: %d, index: %d, scale:%d, disp: %ld (%lx), size: %d, addr_size: %d\n",
	    regs[0], regs[1], scale, imm, imm, size, addr_size);
}


ia32_instruction& ia32_decode(unsigned int capa, const unsigned char* addr, ia32_instruction& instruct)
{
  ia32_prefixes& pref = instruct.prf;
  unsigned int table, nxtab;
  unsigned int idx, sseidx = 0;
  ia32_entry *gotit = NULL;
  int condbits = 0;

  if(capa & IA32_DECODE_MEMACCESS)
    assert(instruct.mac != NULL);

  if (!ia32_decode_prefixes(addr, pref, instruct.loc)) {
    instruct.size = 1;
    instruct.legacy_type = ILLEGAL;
    return instruct;
  }

  if (instruct.loc) instruct.loc->num_prefixes = pref.getCount();
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
	  if(capa & IA32_DECODE_CONDITION) {
        ; // FIXME: translation to tttn & set it
	  }
      instruct = ia32_decode_FP(idx, pref, addr, instruct, gotit, instruct.mac);
      return instruct;
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

  assert(gotit != NULL);
  instruct.legacy_type = gotit->legacyType;

  // addr points after the opcode, and the size has been adjusted accordingly
  if (instruct.loc) instruct.loc->opcode_size = instruct.size - pref.getCount();
  if (instruct.loc) instruct.loc->opcode_position = pref.getCount();

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

#if 0
    // debug output for memory access decoding
    for (int i = 0; i < 3; i++) {
	if (instruct.mac[i].is) {
	    fprintf(stderr, "%d)", i);
	    if (instruct.mac[i].read)
		fprintf(stderr, " read");
	    if (instruct.mac[i].write)
		fprintf(stderr, " write");
	    if (instruct.mac[i].nt)
		fprintf(stderr, " nt");
	    if (instruct.mac[i].sizehack)
		fprintf(stderr, " sizehack");
	    fprintf(stderr, "\n");
	    instruct.mac[i].print();
	}
    }
#endif

  }


  if(capa & IA32_DECODE_CONDITION) {
    int hack = gotit->opsema >> FPOS;
    if(hack == fCOND)
      instruct.cond->set(condbits);
  }

  instruct.entry = gotit;
  return instruct;
}

ia32_instruction& ia32_decode_FP(unsigned int opcode, const ia32_prefixes& pref,
                                 const unsigned char* addr, ia32_instruction& instruct,
                                 ia32_entry * entry, ia32_memacc *mac)
{
  unsigned int nib = byteSzB; // modRM
  unsigned int addrSzAttr = (pref.getPrefix(3) == PREFIX_SZADDR ? 1 : 2); // 32-bit mode implicit
  unsigned int operSzAttr = (pref.getPrefix(2) == PREFIX_SZOPER ? 1 : 2); // 32-bit mode implicit

  if (addr[0] <= 0xBF) { // modrm
    if (instruct.loc) {
       instruct.loc->modrm_position = instruct.loc->opcode_position +
          instruct.loc->opcode_size;
       instruct.loc->modrm_operand = 0;
    }
    nib += ia32_decode_modrm(addrSzAttr, addr, mac, &pref, instruct.loc);
    // also need to check for AMD64 rip-relative data addressing
    // occurs when mod == 0 and r/m == 101
    if (mode_64)
       if ((addr[0] & 0xc7) == 0x05)
          instruct.rip_relative_data = true;
    // operand size has to be determined from opcode
    if(mac)
      {
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
      } // if mac
  } // if modrm
  
  instruct.size += nib;
  instruct.entry = entry;
  
  return instruct;
}

#define MODRM_MOD(x) ((x) >> 6)
#define MODRM_RM(x) ((x) & 7)
#define MODRM_REG(x) (((x) & (7 << 3)) >> 3)
#define MODRM_SET_MOD(x, y) ((x) |= ((y) << 6))
#define MODRM_SET_RM(x, y) ((x) |= (y))
#define MODRM_SET_REG(x, y) ((x) |= ((y) << 3))

static unsigned int ia32_decode_modrm(const unsigned int addrSzAttr,
                                      const unsigned char* addr,
                                      ia32_memacc* macadr,
                                      const ia32_prefixes* pref,
                                      ia32_locations *loc)
{
 
  unsigned char modrm = addr[0];
  unsigned char mod = MODRM_MOD(modrm);
  unsigned char rm  = MODRM_RM(modrm);
  unsigned char reg = MODRM_REG(modrm);
  if (loc) {
     loc->modrm_byte = modrm;
     loc->modrm_mod = mod;
     loc->modrm_rm = rm;
     loc->modrm_reg = reg;
     loc->address_size = addrSzAttr;
  }
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
        case 6: { 
           // disp16
           const short int *pdisp16 = (const short int*)addr;
           //bperr( "16bit addr - case6!\n");
           macadr->set16(-1, -1, *pdisp16);
           if (loc) { 
              loc->disp_position = loc->modrm_position + 1;
              loc->disp_size = 2;
           }
           break; 
        }
        case 7:
          macadr->set16(mBX, -1, 0);
          break;
        }
      return rm==6 ? wordSzB : 0;
    case 1:
      //bperr( "16bit addr - case1!\n");
      if(macadr) {
        const char *pdisp8 = (const char*)addr;
        if (loc) { 
           loc->disp_position = loc->modrm_position + 1;
           loc->disp_size = 1;
        }
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
        if (loc) { 
           loc->disp_position = loc->modrm_position + 1;
           loc->disp_size = 2;
        }
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
      if (loc) { 
         loc->sib_position = loc->modrm_position + 1;
         loc->sib_byte = sib;
      }
      ++addr;
      base = sib & 7;
      if(macadr) {
        scale = sib >> 6;
        index = (sib >> 3) & 7;

	// stack pointer can't be used as an index - supports base-only addressing w/ SIB
        if(index == 4 && !pref->rexX())
          index = -1;
      }
    }
    switch(mod) {
    case 0: {
      /* this is tricky: there is a disp32 iff (1) rm == 5  or  (2) hassib && base == 5 */
      unsigned char check5 = hassib ? base : rm;
      if(macadr)
        switch (rm) {
        case 0:
          macadr->set(apply_rex_bit(mEAX, pref->rexB()), 0, addrSzAttr);
          break;
        case 1:
          macadr->set(apply_rex_bit(mECX, pref->rexB()), 0, addrSzAttr);
          break;
        case 2:
          macadr->set(apply_rex_bit(mEDX, pref->rexB()), 0, addrSzAttr);
          break;
        case 3:
          macadr->set(apply_rex_bit(mEBX, pref->rexB()), 0, addrSzAttr);
          break;
        case 4: // SIB
          if(base == 5) 
          { 
             // disp32[index<<scale]
             const int *pdisp32 = (const int*)addr;
             if (loc) { 
                loc->disp_position = loc->sib_position + 1;
                loc->disp_size = 4;
             }
             macadr->set_sib(-1, scale, apply_rex_bit(index, pref->rexX()), 
                             *pdisp32, addrSzAttr);
          }
          else
            macadr->set_sib(apply_rex_bit(base, pref->rexB()), scale, 
                            apply_rex_bit(index, pref->rexX()),
                            0, addrSzAttr);
          break;
        case 5: { 
           // disp32 (or [RIP + disp32] for 64-bit mode)
           if (loc) { 
              loc->disp_position = loc->modrm_position + 1;
              loc->disp_size = 4;
           }
           const int *pdisp32 = (const int*)addr;
           if (mode_64)
              macadr->set(mRIP, *pdisp32, addrSzAttr);
           else
              macadr->set(-1, *pdisp32, addrSzAttr);
           break; 
        }
        case 6:
           macadr->set(apply_rex_bit(mESI, pref->rexB()), 0, addrSzAttr);
           break;
        case 7:
           macadr->set(apply_rex_bit(mEDI, pref->rexB()), 0, addrSzAttr);
           break;
      }
      return nsib + ((check5 == 5) ? dwordSzB : 0);
    }
    case 1:
      if(macadr) {
        const char *pdisp8 = (const char*)addr;
        switch (rm) {
        case 0:
	  macadr->set(apply_rex_bit(mEAX, pref->rexB()), *pdisp8, addrSzAttr);
          break;
        case 1:
          macadr->set(apply_rex_bit(mECX, pref->rexB()), *pdisp8, addrSzAttr);
          break;
        case 2:
          macadr->set(apply_rex_bit(mEDX, pref->rexB()), *pdisp8, addrSzAttr);
          break;
        case 3:
          macadr->set(apply_rex_bit(mEBX, pref->rexB()), *pdisp8, addrSzAttr);
          break;
        case 4:
          // disp8[EBP + index<<scale] happens naturally here when base=5
           if (loc) { 
              loc->disp_position = loc->sib_position + 1;
              loc->disp_size = 1;
           }           
          macadr->set_sib(apply_rex_bit(base, pref->rexB()), scale, apply_rex_bit(index, pref->rexX()),
			  *pdisp8, addrSzAttr);
          break;
        case 5:
          macadr->set(apply_rex_bit(mEBP, pref->rexB()), *pdisp8, addrSzAttr);
          break;
        case 6:
          macadr->set(apply_rex_bit(mESI, pref->rexB()), *pdisp8, addrSzAttr);
          break;
        case 7:
          macadr->set(apply_rex_bit(mEDI, pref->rexB()), *pdisp8, addrSzAttr);
          break;
        }
      }
      return nsib + byteSzB;
    case 2:
      if(macadr) {
        const int *pdisp32 = (const int*)addr;
        switch (rm) {
        case 0:
          macadr->set(apply_rex_bit(mEAX, pref->rexB()), *pdisp32, addrSzAttr);
          break;
        case 1:
          macadr->set(apply_rex_bit(mECX, pref->rexB()), *pdisp32, addrSzAttr);
          break;
        case 2:
          macadr->set(apply_rex_bit(mEDX, pref->rexB()), *pdisp32, addrSzAttr);
          break;
        case 3:
          macadr->set(apply_rex_bit(mEBX, pref->rexB()), *pdisp32, addrSzAttr);
          break;
        case 4:
          // disp32[EBP + index<<scale] happens naturally here when base=5
           if (loc) { 
              loc->disp_position = loc->sib_position + 1;
              loc->disp_size = 4;
           }
          macadr->set_sib(apply_rex_bit(base, pref->rexB()), scale, apply_rex_bit(index, pref->rexX()),
			  *pdisp32, addrSzAttr);
          break;
        case 5:
          macadr->set(apply_rex_bit(mEBP, pref->rexB()), *pdisp32, addrSzAttr);
          break;
        case 6:
          macadr->set(apply_rex_bit(mESI, pref->rexB()), *pdisp32, addrSzAttr);
          break;
        case 7:
          macadr->set(apply_rex_bit(mEDI, pref->rexB()), *pdisp32, addrSzAttr);
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


int getOperSz(const ia32_prefixes &pref) 
{
   if (pref.rexW()) return 4;
   else if (pref.getPrefix(2) == PREFIX_SZOPER) return 1;
   else return 2;
}

unsigned int ia32_decode_operands (const ia32_prefixes& pref, 
                                   const ia32_entry& gotit, 
                                   const unsigned char* addr, 
                                   ia32_instruction& instruct,
                                   ia32_memacc *mac)
{
  ia32_locations *loc = instruct.loc;
  unsigned int nib = 0 /* # of bytes in instruction */;

  int addrSzAttr = (pref.getPrefix(3) == PREFIX_SZADDR ? 1 : 2);

  if (mode_64)
    addrSzAttr *= 2;

  int operSzAttr = getOperSz(pref);

  if(gotit.hasModRM)
    nib += byteSzB;

  for(unsigned int i=0; i<3; ++i) {
    const ia32_operand& op = gotit.operands[i];
    if(op.admet) {
      // At most two operands can be memory, the third is register or immediate
      //assert(i<2 || op.admet == am_reg || op.admet == am_I);
      switch(op.admet) {
      case am_A: /* address = segment + offset (word or dword or qword) */
        nib += wordSzB;
        if(mac)
          bperr( "x86: segment selector ignored [am_A].\n");
      case am_O: /* operand offset */
        nib += wordSzB * addrSzAttr;
        if(mac) {
          int offset;
          switch(addrSzAttr) {
          case 1: // 16-bit offset
              offset = *((const short int*)addr);
              break;
          case 2: // 32-bit offset
              offset = *((const int*)addr);
              break;
          case 4: // 64-bit
	      offset = *((const long*)addr);
              break;
          default:
              assert(0);
              break;
          }
          mac[i].set(-1, offset, addrSzAttr);
          mac[i].size = type2size(op.optype, operSzAttr);
        }
        break;
      case am_C:   /* control register */
      case am_D:   /* debug register */
      case am_F:   /* flags register */
      case am_G:   /* general purpose register, selecteb by reg field */
      case am_P:   /* MMX register */
      case am_R:   /* general purpose register, selected by r/m field */
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
         if (loc) {
            loc->modrm_position = loc->opcode_size + loc->opcode_position;
            loc->modrm_operand = i;
         }
         if(mac) {
            nib += ia32_decode_modrm(addrSzAttr, addr, &mac[i], &pref, loc);
            mac[i].size = type2size(op.optype, operSzAttr);
            if (loc) loc->address_size = mac[i].size;
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
      case am_J: { /* instruction pointer offset */
         int imm_size = type2size(op.optype, operSzAttr);
         if (loc) {
            loc->imm_position = nib + loc->opcode_position + loc->opcode_size;
            loc->imm_size = imm_size;
         }
         nib += imm_size;
         break;
      }
      /* TODO: rep prefixes, deal with them here? */
      case am_X: /* memory at DS:(E)SI*/
        if(mac)
          mac[i].setXY(mESI, type2size(op.optype, operSzAttr), addrSzAttr);
        break;
      case am_Y: /* memory at ES:(E)DI*/
        if(mac)
          mac[i].setXY(mEDI, type2size(op.optype, operSzAttr), addrSzAttr);
        break;
      case am_stackH: /* stack push */
        if(mac) {
          // assuming 32-bit (64-bit for AMD64) stack segment
	  // AMD64: push defaults to 64-bit operand size
	  if (mode_64 && operSzAttr == 2)
	      operSzAttr = 4;
          mac[i].set(mESP, -2 * operSzAttr, addrSzAttr);
          mac[i].size = type2size(op.optype, operSzAttr);
        }
        break;
      case am_stackP: /* stack pop */
        if(mac) {
          // assuming 32-bit (64-bit for AMD64) stack segment
	  // AMD64: pop defaults to 64-bit operand size
	  if (mode_64 && operSzAttr == 2)
	      operSzAttr = 4;
          mac[i].set(mESP, 0, addrSzAttr);
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
bool ia32_decode_prefixes(const unsigned char* addr, ia32_prefixes& pref,
                          ia32_locations *loc)
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

  bool result = true;
  if (mode_64)
     result = ia32_decode_rex(addr - 1, pref, loc);
  if (loc) loc->num_prefixes = pref.count;

  return result;
}

#define REX_ISREX(x) (((x) >> 4) == 4)
#define REX_W(x) ((x) & 0x8)
#define REX_R(x) ((x) & 0x4)
#define REX_X(x) ((x) & 0x2)
#define REX_B(x) ((x) & 0x1)

#define REX_INIT(x) ((x) = 0x40)
#define REX_SET_W(x, v) ((x) |= ((v) ? 0x8 : 0))
#define REX_SET_R(x, v) ((x) |= ((v) ? 0x4 : 0))
#define REX_SET_X(x, v) ((x) |= ((v) ? 0x2 : 0))
#define REX_SET_B(x, v) ((x) |= ((v) ? 0x1 : 0))

bool ia32_decode_rex(const unsigned char* addr, ia32_prefixes& pref,
                     ia32_locations *loc)
{
   if (REX_ISREX(addr[0])) {
      ++pref.count;
      pref.prfx[4] = addr[0];

      if (loc) {
         loc->rex_byte = addr[0];
         loc->rex_w = REX_W(addr[0]);
         loc->rex_r = REX_R(addr[0]);
         loc->rex_x = REX_X(addr[0]);
         loc->rex_b = REX_B(addr[0]);
         loc->rex_position = pref.count - 1;
      }
      
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
  ia32_decode(0, addr, i);

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
  } else if (type & REL_D_DATA) {
    // Some SIMD instructions have a mandatory 0xf2 prefix; the call to skip_headers
    // doesn't skip it and I don't feel confident in changing that - bernat, 22MAY06
      // 0xf3 as well...
      // Some 3-byte opcodes start with 0x66... skip
      if (*instr == 0x66) instr++;
      if (*instr == 0xf2) instr++;
      else if (*instr == 0xf3) instr++;
    // And the "0x0f is a 2-byte opcode" skip
    if (*instr == 0x0F) instr++;
    // Skip the instr opcode and the MOD/RM byte
    disp = *(const int *)(instr+2);
  }  

  return disp;
}


// get the displacement of a relative jump or call

int get_disp(instruction *insn) {
  return displacement(insn->ptr(), insn->type());
}

int count_prefixes(unsigned insnType) {
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
  return nPrefixes;
}

unsigned copy_prefixes(const unsigned char *&origInsn, unsigned char *&newInsn, unsigned insnType) {
  // Skip prefixes
  unsigned nPrefixes = count_prefixes(insnType);

  //inst_printf(".... %d bytes in prefixes\n", nPrefixes);
  for (unsigned u = 0; u < nPrefixes; u++)
    *newInsn++ = *origInsn++;
  return nPrefixes;
}

bool convert_to_rel8(const unsigned char*&origInsn, unsigned char *&newInsn) {
  if (*origInsn == 0x0f)
    origInsn++;

  // We think that an opcode in the 0x8? range can be mapped to an equivalent
  // opcode in the 0x7? range...

  if ((*origInsn >= 0x70) &&
      (*origInsn < 0x80)) {
    *newInsn++ = *origInsn++;
    return true;
  }

  if ((*origInsn >= 0x80) &&
      (*origInsn < 0x90)) {
    *newInsn++ = *origInsn++ - 0x10;
    return true;
  }

  if (*origInsn == 0xE3) {
    *newInsn++ = *origInsn++;
    return true;
  }

  // Oops...
  fprintf(stderr, "Unhandled jump conversion case: opcode is 0x%x\n", *origInsn);
  assert(0 && "Unhandled jump conversion case!");
  return false;
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
    
    unsigned Mod1_1 =  ( q[ 1 ] >> 3 ) & 0x07;
    unsigned Mod1_2 =  q[ 1 ] & 0x07;
    unsigned Mod2_1 =  ( r[ 1 ] >> 3 ) & 0x07;
    unsigned Mod2_2 =  r[ 1 ] & 0x07;

    if( insn1.size() != 1 )
    {
        return false;  //shouldn't need this, but you never know
    }
    
    if( p[ 0 ] == PUSHEBP  )
    {   
        // Looking for mov %esp -> %ebp in one of the two
        // following instructions.  There are two ways to encode 
        // mov %esp -> %ebp: as '0x8b 0xec' or as '0x89 0xe5'.  
        if( insn2.isMoveRegMemToRegMem() && 
            ((Mod1_1 == 0x05 && Mod1_2 == 0x04) ||
             (Mod1_1 == 0x04 && Mod1_2 == 0x05)))
            return true;

        if( insn3.isMoveRegMemToRegMem() && 
            ((Mod2_1 == 0x05 && Mod2_2 == 0x04) ||
             (Mod2_1 == 0x04 && Mod2_2 == 0x05)))
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
      return generateBranch64(gen, toAddr);
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

// Unified the 64-bit push between branch and call
void instruction::generatePush64(codeGen &gen, Address val) {
  GET_PTR(insn, gen);
  for (int i = 3; i >= 0; i--) {
    short word = (short) (val >> (16 * i)) & 0xffff;
    *insn++ = 0x66; // operand size override
    *insn++ = 0x68; // push immediate (16-bits b/c of prefix)
    *(short *)insn = word;
    insn += 2;
  }
  SET_PTR(insn, gen);
}

void instruction::generateBranch64(codeGen &gen, Address to)
{
    // "long jump" - generates sequence to jump to any 64-bit address
    // pushes the value on the stack (using 4 16-bit pushes) the uses a 'RET'

  generatePush64(gen, to);

  GET_PTR(insn, gen);
  *insn++ = 0xC3; // RET
  SET_PTR(insn, gen);

}

void instruction::generateCall(codeGen &gen,
                               Address from,
                               Address target) {
  // TODO 64bit fixme
  long disp = target - (from + CALL_REL32_SZ);
  
  if (is_disp32(disp)) {
    GET_PTR(insn, gen);
    *insn++ = 0xE8;
    *((int *)insn) = (int) disp;
    insn += sizeof(int);
    SET_PTR(insn, gen);
  }
  else {
    // Wheee....
    // We use a technique similar to our 64-bit
    // branch; push the return addr onto the stack (from),
    // then push to, then return. 
    
    // This looks like
    // A: push D
    // B: push <...>
    // C: return
    // D:

#if defined(arch_x86_64)
    // So we need to know where D is off of "from"
    generatePush64(gen, from+CALL_ABS64_SZ);
    generateBranch64(gen, target);
#else
    assert(0 && "Impossible case");
#endif
  }
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

    // longJumpSize == size of code needed to get
    // anywhere in memory space.
#if defined(arch_x86_64)
    const int longJumpSize = JUMP_ABS64_SZ;
#else
    const int longJumpSize = JUMP_REL32_SZ;
#endif


    // Assumption: rewriting calls into immediates will
    // not be larger than rewriting a call into a call...

    if (!((type() & REL_B) ||
	  (type() & REL_W) ||
	  (type() & REL_D) ||
	  (type() & REL_D_DATA))) {
      return size();
    }
    
    // Now that the easy case is out of the way...
    
    if (type() & IS_JUMP) {
      // Worst-case: prefixes + max-displacement branch
      return count_prefixes(type()) + longJumpSize;
    }
    if (type() & IS_JCC) {
      // Jump conditional; jump past jump; long jump
      return count_prefixes(type()) + 2 + 2 + longJumpSize;
    }
    if (type() & IS_CALL) {
      // Worst case is approximated by two long jumps (AMD64) or a REL32 (x86)
#if defined(arch_x86_64)
      return 2*JUMP_ABS64_SZ+count_prefixes(type());
#else
      return JUMP_REL32_SZ+count_prefixes(type());
#endif
    }
#if defined(arch_x86_64)
    if (type() & REL_D_DATA) {
      // Worst-case: replace RIP with push of IP, use, and cleanup
      // 8: unknown; previous constant
      return count_prefixes(type()) + size() + 8;
    }
#endif

    assert(0);
    return 0;
}

bool instruction::generate(codeGen &gen,
                           AddressSpace *addrSpace,
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

   Address newDisp = 0;

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
         assert(sizeof(unsigned int) == 4);
         newInsn += sizeof(unsigned int);
         assert((newInsn - insnBuf) == 5);
         done = true;
      }
      else if (addrSpace->isValidAddress(target)) {
         // Get us an instrucIter
	InstrucIter callTarget(target, addrSpace);
	instruction firstInsn = callTarget.getInstruction();
	callTarget++;
         instruction secondInsn = callTarget.getInstruction();
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
      else {
	fprintf(stderr, "Warning: call at 0x%lx (reloc 0x%lx) did not have a valid calculated target addr 0x%lx\n",
		origAddr, newAddr, target);
      }
   }
   if (done) {
     // Reset the generator and return
     SET_PTR(newInsn, gen);
     return true;
   }

   if (!((insnType & REL_B) ||
	 (insnType & REL_W) ||
	 (insnType & REL_D) ||
	 (insnType & REL_D_DATA))) {
     // Normal insn, not addr relative
     for (unsigned u = 0; u < insnSz; u++)
       *newInsn++ = *origInsn++;
     SET_PTR(newInsn, gen);
     return true;
   }

   // We're addr-relative...
   
   Address newTarget = 0; // 64-bit jumps are absolute

   if (targetOverride == 0) {
     newTarget = origAddr + size() + get_disp(this);
   }
   else {
     newTarget = targetOverride;
   }

   newDisp = newTarget - newAddr;
   
   if (insnType & IS_JUMP) {
     // Replace it....

     copy_prefixes(origInsn, newInsn, insnType);
     
     SET_PTR(newInsn, gen);
     generateBranch(gen, newAddr, newTarget);
     return true;
   }

   if (insnType & IS_JCC) {
     // We use a three-step branch system that looks like so:
     // jump conditional <A> 
     // becomes
     // jump conditional <B>
     // jump <C>
     // B: jump <A>
     // C: ... next insn
     // We could optimize, but this works and is clear to read.

     unsigned nPrefixes = copy_prefixes(origInsn, newInsn, insnType);

     // Moves as appropriate...
     convert_to_rel8(origInsn, newInsn);
     // We now want a 2-byte branch past the branch at B
     *newInsn++ = 2;

     // Now for the branch at B- <jumpSize> unconditional branch
     *newInsn++ = 0xEB; *newInsn++ = (char) jumpSize(newDisp);

     // And the branch at C
     SET_PTR(newInsn, gen);
     // Original address is a little skewed... 
     // We've moved past the original address (to the tune of nPrefixes + 2 (JCC) + 2 (J))
     generateBranch(gen, newAddr + nPrefixes + 2 + 2, newTarget);
     return true;
   }

   if (insnType & IS_CALL) {
     unsigned nPrefixes = copy_prefixes(origInsn, newInsn, insnType);
     SET_PTR(newInsn, gen);
     generateCall(gen, newAddr + nPrefixes, newTarget);
     return true;
   }

#if defined(arch_x86_64)
   if (insnType & REL_D_DATA) {
     // We may need to change these from 32-bit relative
     // to 64-bit absolute. This happens with the jumps and calls
     // as well, but it's better encapsulated there.

     // We have three options:
     // A) 64-bit absolute version
     // b) 32-bit absolute version (where 32-bit relative would fail but we're low)
     // c) 32-bit relative (AKA "original").


     unsigned nPrefixes = count_prefixes(insnType);

     // count opcode bytes (1 or 2)
     unsigned nOpcodeBytes = 1;
     if (*(origInsn + nPrefixes) == 0x0F)
       nOpcodeBytes = 2;

     bool is_data_abs64 = false;
     Register pointer_reg = (Register)-1;

     if (!(is_disp32(newDisp+insnSz) || is_addr32(newTarget))) {
       assert(!is_disp32(newDisp));
       assert(!is_addr32(newDisp));
       // Case A: replace with 64-bit.
       is_data_abs64 = true;
       unsigned char mod_rm = *(origInsn + nPrefixes + nOpcodeBytes);
       pointer_reg = (mod_rm & 0x38) != 0 ? 0 : 3;
       SET_PTR(newInsn, gen);
       emitPushReg64(pointer_reg, gen);
       emitMovImmToReg64(pointer_reg, newTarget, true, gen);
       REGET_PTR(newInsn, gen);
     }

     const unsigned char* origInsnStart = origInsn;

     // In other cases, we can rewrite the insn directly; in the 64-bit case, we
     // still need to copy the insn
     copy_prefixes(origInsn, newInsn, insnType);

     if (*origInsn == 0x0F) {
       *newInsn++ = *origInsn++;
     }
     
     // And the normal opcode
     *newInsn++ = *origInsn++;

     if (is_data_abs64) {
       // change ModRM byte to use [pointer_reg]: requires
       // us to change last three bits (the r/m field)
       // to the value of pointer_reg
       unsigned char mod_rm = *origInsn++;
       assert(pointer_reg != (Register)-1);
       mod_rm = (mod_rm & 0xf8) + pointer_reg;
       *newInsn++ = mod_rm;
     }
     else if (is_disp32(newDisp+insnSz)) {
       // Whee easy case
       *newInsn++ = *origInsn++;
       // Size doesn't change....
       *((int *)newInsn) = (int)(newDisp - insnSz);
       newInsn += 4;
     }
     else if (is_addr32(newTarget)) {
       assert(!is_disp32(newDisp+insnSz));

       unsigned char mod_rm = *origInsn++;
       
       // change ModRM byte to use SIB addressing (r/m == 4)
       mod_rm = (mod_rm & 0xf8) + 4;
       *newInsn++ = mod_rm;
       
       // SIB == 0x25 specifies [disp32] addressing when mod == 0
       *newInsn++ = 0x25;
       
       // now throw in the displacement (the absolute 32-bit address)
       *((int *)newInsn) = (int)(newTarget);
       newInsn += 4;
     }
     else {
       // Should never be reached...
       assert(0);
     }

     // there may be an immediate after the displacement for RIP-relative
     // so we copy over the rest of the instruction here
     origInsn += 4;
     while (origInsn - origInsnStart < (int)insnSz)
       *newInsn++ = *origInsn++;

     SET_PTR(newInsn, gen);
     
     if (is_data_abs64) {
       // Cleanup on aisle pointer_reg...
       assert(pointer_reg != (Register)-1);
       emitPopReg64(pointer_reg, gen);
     }
     return true;
   }
#endif
   // Should we get here?

   assert(0);
   return false;
}

#if defined(arch_x86)
unsigned instruction::jumpSize(long /*disp*/ ) {
  return JUMP_REL32_SZ;
}

unsigned instruction::jumpSize(Address /*from*/, Address /*to*/) {
    return JUMP_REL32_SZ;
}
#else

unsigned instruction::jumpSize(long disp) {
  if (is_disp32(disp))
    return JUMP_REL32_SZ;
  else
    return JUMP_ABS64_SZ;
}

unsigned instruction::jumpSize(Address from, Address to) {
    long disp = to - (from + JUMP_REL32_SZ);
    return jumpSize(disp);
}
#endif

unsigned instruction::maxJumpSize() {
#if defined(arch_x86)
    return JUMP_REL32_SZ;
#else
    return JUMP_ABS64_SZ;
#endif
}

bool instruction::isCmp() const {
    if(*op_ptr_ == CMP_EB_GB || *op_ptr_ == CMP_EV_GV ||
       *op_ptr_ == CMP_GB_EB || *op_ptr_ == CMP_GV_EV ||
       *op_ptr_ == CMP_AL_LB || *op_ptr_ == CMP_RAX_LZ)
    {
        return true;
    }

    if(*op_ptr_ == 0x80 || *op_ptr_ == 0x81 || *op_ptr_ == 0x83) 
    {
        // group 1 opcodes -- operation depends on reg (bits 3-5) of
        // modr/m byte
        const unsigned char *p = op_ptr_+1;
        if( (*p & (7<<3)) == (7<<3))
            return true;
    }

    return false;
}

#define SIB_SET_REG(x, y) ((x) |= ((y) & 7))
#define SIB_SET_INDEX(x, y) ((x) |= (((y) & 7) << 3))
#define SIB_SET_SS(x, y) ((x) | (((y) & 3) << 6))

typedef struct parsed_instr_t {
   int num_prefixes;
   int opcode_size;
   int disp_position;
   int disp_size;
   int imm_position;
   int imm_size;

   unsigned char sib_byte;
   unsigned char modrm_byte;
   int sib_position; 
   int modrm_position;

   int address_size;
   int modrm_operand;

   int rex_position;
   int rex_byte;
   int rex_w;
   int rex_r;
   int rex_x;
   int rex_b;

   ia32_instruction orig_instr;
   ia32_entry *entry;
} parse_instr_t;

bool instruction::getUsedRegs(pdvector<int> &regs) {
   const unsigned char *insn_ptr = ptr();

   struct ia32_memacc memacc[3];
   struct ia32_condition cond;
   struct ia32_locations loc;
   ia32_entry *entry;
   ia32_instruction orig_instr(memacc, &cond, &loc);
   ia32_decode(IA32_DECODE_MEMACCESS | IA32_DECODE_CONDITION,
               insn_ptr, orig_instr);
   entry = orig_instr.getEntry();

   if (orig_instr.getPrefix()->getPrefix(1) != 0)
      //The instruction accesses memory via segment registers.  Disallow.
      return false;
   
   if (loc.modrm_position == -1)
      //Only supporting MOD/RM instructions now
      return false; 

   if (loc.address_size == 1)
      //Don't support 16-bit instructions yet
      return false;

   if (loc.modrm_reg == 4 && !loc.rex_r)
      //The non-memory access register is %rsp/%esp, we can't work with
      // this register due to our register saving techniques.
      return false;

   if (loc.modrm_mod == 3)
      //This instruction doesn't use the MOD/RM to access memory
      return false;

   for (unsigned i=0; i<3; i++) {
      const ia32_operand& op = entry->operands[i];
      if (op.admet == am_O) {
         //The MOD/RM specifies a register that's used
         int regused = loc.modrm_reg;
         if (loc.address_size == 4) {
            regused |= loc.rex_r << 4;
         }
         regs.push_back(regused);
      }
      else if (op.admet == am_reg) {
         //The instruction implicitely references a memory instruction
         switch (op.optype) {
            case r_AH:   
            case r_AL:   
            case r_eAX:
            case r_EAX:
               regs.push_back(REGNUM_RAX);
               if (loc.rex_byte) regs.push_back(REGNUM_R8);
               break;
            case r_BH:
            case r_BL:
            case r_eBX:
            case r_EBX:
               regs.push_back(REGNUM_RBX);
               if (loc.rex_byte) regs.push_back(REGNUM_R11);
               break;
            case r_CH:   
            case r_CL:   
            case r_eCX:
            case r_ECX:
               regs.push_back(REGNUM_RCX);
               if (loc.rex_byte) regs.push_back(REGNUM_R9);
               break;
            case r_DL:
            case r_DH:
            case r_eDX:
            case r_EDX:
               regs.push_back(REGNUM_RDX);
               if (loc.rex_byte) regs.push_back(REGNUM_R10);
               break;
            case r_eSP:
            case r_ESP:
               regs.push_back(REGNUM_RSP);
               if (loc.rex_byte) regs.push_back(REGNUM_R12);
               break;
            case r_eBP:
            case r_EBP:
               regs.push_back(REGNUM_RBP);
               if (loc.rex_byte) regs.push_back(REGNUM_R13);
               break;
            case r_eSI:
            case r_ESI:
               regs.push_back(REGNUM_RSI);
               if (loc.rex_byte) regs.push_back(REGNUM_R14);
               break;
            case r_EDI:
            case r_eDI:
               regs.push_back(REGNUM_RDI);
               if (loc.rex_byte) regs.push_back(REGNUM_R15);
               break;
            case r_EDXEAX:
               regs.push_back(REGNUM_RAX);
               regs.push_back(REGNUM_RDX);
               break;
            case r_ECXEBX:
               regs.push_back(REGNUM_RBX);
               regs.push_back(REGNUM_RCX);
               break;
         }
      }
   }
   return true;
}

/**
 * The comments and naming schemes in this function assume some familiarity with
 * the IA32/IA32e instruction encoding.  If you don't understand this, I suggest
 * you start with Chapter 2 of:
 *  _IA-32 Intel Architecture Software Developer's Manual, Volume 2a_
 * and appendix A of:
 *  _IA-32 Intel Architecture Software Developer's Manual, Volume 2b_
 *
 * This function takes an instruction that accesses memory, and emits a 
 * copy of that instruction that has the load/store replaces with a load/store
 * through a register.  For example, if this function were called with 'loadExpr = r12'
 * on the instruction 'mov 12(%rax)->%rbx', we would emit 'mov (%r12)->%rbx'.
 * Note that we strip off any displacements, indexs, etc...  The register is assumed
 * to contain the final address that will be loaded/stored.
 **/
bool instruction::generateMem(codeGen &gen,
                              Address /*origAddr*/, 
                              Address /*newAddr*/,
                              Register loadExpr,
                              Register storeExpr) 
{
   /**********
    * Check parameters
    **********/
   Register newreg = Null_Register;
   if (loadExpr != Null_Register && storeExpr != Null_Register) 
      return false; //Can only do one memory replace per instruction now
   else if (loadExpr == Null_Register && storeExpr == Null_Register) 
      return false; //None specified
   else if (loadExpr != Null_Register && storeExpr == Null_Register) 
      newreg = loadExpr;
   else if (loadExpr == Null_Register && storeExpr != Null_Register) 
      newreg = storeExpr;

   /********************************
    * Section 1.  Read the instruction into our internal data structures.
    ********************************/
   GET_PTR(insnBuf, gen);
   const unsigned char *insn_ptr = ptr();

   struct ia32_memacc memacc[3];
   struct ia32_condition cond;
   struct ia32_locations loc;

   ia32_entry *entry;
   ia32_instruction orig_instr(memacc, &cond, &loc);
   ia32_decode(IA32_DECODE_MEMACCESS | IA32_DECODE_CONDITION,
               insn_ptr, orig_instr);
   entry = orig_instr.getEntry();

   if (gen.addrSpace()->getAddressWidth() != 8)
      //Currently works only on IA-32e
      return false; 

   if (orig_instr.getPrefix()->getPrefix(1) != 0)
      //The instruction accesses memory via segment registers.  Disallow.
      return false;
   
   if (loc.modrm_position == -1)
      //Only supporting MOD/RM instructions now
      return false; 

   if (loc.address_size == 1)
      //Don't support 16-bit instructions yet
      return false;

   if (loc.modrm_reg == 4 && !loc.rex_r) {
      //The non-memory access register is %rsp/%esp, we can't work with
      // this register due to our register saving techniques.
      return false;
   }

   if (loc.modrm_mod == 3) {
      //This instruction doesn't use the MOD/RM to access memory
      return false;
   }
   
   /*********************************
    * Section 2.  We understand the instruction.  Output it
    *********************************/
   int emit_displacement = 0;
   int emit_mod = 0;
   int emit_sib = 0;
   int new_sib = 0;
   
   unsigned char *walker = insnBuf;
   
   /**
    * Handle two special cases, where we emit a memory instruction
    * that loads from/to RBP/R13 or RSP/R12
    **/
   if (newreg == REGNUM_RBP || newreg == REGNUM_R13) {
      //You can't encode rbp or r13 normally i.e. 'mov (%r13)->%rax'
      // Instead we encode using a displacement, so 'mov 0(%r13)->%rax'
      emit_displacement = 1;
      emit_mod = 1; //1 encodes the 0(%reg) format
   }
   
   if (newreg == REGNUM_RSP || newreg == REGNUM_R12) {
      //You can't encode rsp or r12 normally.  We'll emit a SIB instruction.
      // So instead of 'mov (%r12)->%rax' we'll emit 'mov (%r12,0,0)->%rax
      emit_sib = 1;
      SIB_SET_REG(new_sib, newreg & 7);
      SIB_SET_INDEX(new_sib, newreg & 4); //4 encodes to no-index
      //SIB_SET_SS(new_sib, 0); //Gives gcc warning statement w/ no effect
      loc.rex_x = 0; //If we emit a rex, don't extend the index.
   }
   
   /**
    * Emit prefixes
    **/
   unsigned char new_rex = 0;
   
   //Emit all prefixes except for rex
   for (int i=0; i<loc.num_prefixes; i++) {
      if (i != loc.rex_position)
         *walker++ = insn_ptr[i];
   }
   
   //Emit the rex
   if (loc.rex_position != -1 || (newreg & 8)) {
      //If there was no REX byte, and the high bit of the new register
      // is set, then we'll need to make a rex byte to encode that high bit.
      loc.rex_b = newreg & 8;
      REX_INIT(new_rex);
      REX_SET_W(new_rex, loc.rex_w);
      REX_SET_R(new_rex, loc.rex_r);
      REX_SET_X(new_rex, loc.rex_x);
      REX_SET_B(new_rex, loc.rex_b); 
      *walker++ = new_rex;
   }
   
   /**
    * Copy opcode
    **/
   for (int i=loc.num_prefixes; i<loc.num_prefixes+(int)loc.opcode_size; i++) {
      *walker++ = insn_ptr[i];
   }
   
   /**
    * Emit MOD/RM byte
    **/
   unsigned char new_modrm = 0;
   MODRM_SET_MOD(new_modrm, emit_mod); 
   MODRM_SET_RM(new_modrm, newreg & 7); //The new register replacing the memaccess
   // Only the bottom 3 bits go here
   MODRM_SET_REG(new_modrm, loc.modrm_reg); //The bottom old register
   *walker++ = new_modrm;
   
   /**
    * Emit SIB byte
    **/
   if (emit_sib) {
      *walker++ = new_sib;
   }
   
   /**
    * Emit displacement
    **/
   if (emit_displacement) {
      *walker++ = 0x0; //We only need 0 displacements now.
   }
   
   /**
    * Emit immediate
    **/
   for (unsigned i=0; i<loc.imm_size; i++)
   {
      *walker++ = insn_ptr[loc.imm_position + i];
   }

   //Debug output.  Fix the end of testdump.c, compile it, the do an
   // objdump -D
   /*   static FILE *f = NULL;
   if (f == NULL)
   {
      f = fopen("testdump.c", "w+");
      if (!f)
         perror("Couldn't open");
      fprintf(f, "char buffer[] = {\n");
   }
   fprintf(f, "144, 144, 144, 144, 144, 144, 144, 144, 144,\n");
   for (unsigned i=0; i<orig_instr.getSize(); i++) {
      fprintf(f, "%u, ", (unsigned) insn_ptr[i]);
   }
   fprintf(f, "\n");
   for (int i=0; i<(walker-insnBuf); i++) {
      fprintf(f, "%u, ", (unsigned) insnBuf[i]);
   }
   fprintf(f, "\n");
   fprintf(f, "144, 144, 144, 144, 144, 144, 144, 144, 144,\n");*/
   SET_PTR(walker, gen);
   return true;
}
