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

// $Id: arch-x86.C,v 1.5 2008/09/04 21:06:48 bill Exp $

// Official documentation used:    - IA-32 Intel Architecture Software Developer Manual (2001 ed.)
//                                 - AMD x86-64 Architecture Programmer's Manual (rev 3.00, 1/2002)
// Unofficial documentation used:  - www.sandpile.org/ia32
//                                 - NASM documentation

// Note: Unless specified "book" refers to Intel's manual

// This include *must* come first in the file.
#include "common/src/Types.h"

#include <assert.h>
#include <stdio.h>
#include <map>
#include <string>
#include <iostream>

#include "boost/assign/list_of.hpp"
#include "boost/assign/std/vector.hpp"
#include "boost/assign/std/set.hpp"

#include "common/src/arch-x86.h"
#include "dyn_regs.h"

#if defined(os_vxworks)
#include "common/src/wtxKludges.h"
#endif

using namespace std;
using namespace boost::assign;

namespace NS_x86 {

unsigned int swapBytesIfNeeded(unsigned int i)
{
    return i;
}

// groups
enum {
  Grp1a=0, Grp1b, Grp1c, Grp1d, Grp2, Grp3a, Grp3b, Grp4, Grp5, Grp6, Grp7,
  Grp8, Grp9, Grp11, Grp12, Grp13, Grp14, Grp15, Grp16, Grp17, GrpAMD
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
  SSE78, SSE79, SSE7C, SSE7D, SSE7E, SSE7F,
  SSEB8,
  SSEC2, SSEC4, SSEC5, SSEC6,
  SSED0, SSED1, SSED2, SSED3, SSED4, SSED5, SSED6, SSED7,
  SSED8, SSED9, SSEDA, SSEDB, SSEDC, SSEDD, SSEDE, SSEDF,
  SSEE0, SSEE1, SSEE2, SSEE3, SSEE4, SSEE5, SSEE6, SSEE7,
  SSEE8, SSEE9, SSEEA, SSEEB, SSEEC, SSEED, SSEEE, SSEEF,
  SSEF0, SSEF1, SSEF2, SSEF3, SSEF4, SSEF5, SSEF6, SSEF7,
  SSEF8, SSEF9, SSEFA, SSEFB, SSEFC, SSEFD, SSEFE, SSEFF
};

// SSE BIS
enum {
	SSEB00=0, SSEB01, SSEB02, SSEB03, SSEB04, SSEB05, SSEB06, SSEB07,
	SSEB08, SSEB09,	SSEB0A, SSEB0B,
	SSEB10, SSEB14, SSEB15, SSEB17,
	SSEB1C, SSEB1D, SSEB1E,
	SSEB20, SSEB21, SSEB22, SSEB23, SSEB24, SSEB25, 
	SSEB28, SSEB29, SSEB2A, SSEB2B,
	SSEB30, SSEB31, SSEB32, SSEB33, SSEB34, SSEB35, SSEB37,
	SSEB38, SSEB39,	SSEB3A, SSEB3B, SSEB3C, SSEB3D, SSEB3E, SSEB3F,
	SSEB40, SSEB41,
	SSEBF0, SSEBF1
};

// SSE TER 
enum {
	SSET08=0, SSET09,
	SSET0A, SSET0B, SSET0C, SSET0D, SSET0E, SSET0F,
	SSET14, SSET15, SSET16, SSET17,
	SSET20, SSET21, SSET22,
	SSET40, SSET41, SSET42,
	SSET60, SSET61, SSET62, SSET63
};

// SSE groups
enum {
  G12SSE010B=0, G12SSE100B, G12SSE110B,
  G13SSE010B, G13SSE100B, G13SSE110B,
  G14SSE010B, G14SSE011B, G14SSE110B, G14SSE111B,
};

enum {
    GrpD8=0, GrpD9, GrpDA, GrpDB, GrpDC, GrpDD, GrpDE, GrpDF
};

#define Zz   { 0, 0 }
#define ImplImm { am_ImplImm, op_b }
#define Ap   { am_A, op_p }
#define Cd   { am_C, op_d }
#define Dd   { am_D, op_d }
#define Eb   { am_E, op_b }
#define Ed   { am_E, op_d }
#define Ef   { am_E, op_f }
#define Efd  { am_E, op_dbl }
#define Ep   { am_E, op_p }
#define Ev   { am_E, op_v }
#define Ew   { am_E, op_w }
#define Ey	 { am_E, op_y }
#define Fv   { am_F, op_v }
#define Gb   { am_G, op_b }
#define Gd   { am_G, op_d }
#define Gv   { am_G, op_v }
#define Gw   { am_G, op_w }
#define Gf   { am_G, op_f }
#define Gfd  { am_G, op_dbl }
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
#define Mdq   { am_M, op_dq }
#define M512 { am_M, op_512 }
#define Mf   { am_M, op_f }
#define Mfd  { am_M, op_dbl }
#define M14  { am_M, op_14 }
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
#define RMb  { am_RM, op_b }
#define RMw  { am_RM, op_w }
#define Td   { am_T, op_d }
#define UMd	 { am_UM, op_d }
#define Sw   { am_S, op_w }
#define Vd   { am_V, op_d }
#define Vdq  { am_V, op_dq }
#define Vpd  { am_V, op_pd }
#define Vps  { am_V, op_ps }
#define Vq   { am_V, op_q }
#define VRq  { am_VR, op_q }
#define VRdq { am_VR, op_dq }
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

#define AH  { am_reg, x86::iah }
#define AX  { am_reg, x86::iax }
#define BH  { am_reg, x86::ibh }
#define CH  { am_reg, x86::ich }
#define DH  { am_reg, x86::idh }
#define AL  { am_reg, x86::ial }
#define BL  { am_reg, x86::ibl }
#define CL  { am_reg, x86::icl }
#define CS  { am_reg, x86::ics }
#define DL  { am_reg, x86::idl }
#define DX  { am_reg, x86::idx }
#define eAX { am_reg, x86::ieax }
#define eBX { am_reg, x86::iebx }
#define eCX { am_reg, x86::iecx }
#define eDX { am_reg, x86::iedx }
#define EAX { am_reg, x86::ieax }
#define EBX { am_reg, x86::iebx }
#define ECX { am_reg, x86::iecx }
#define EDX { am_reg, x86::iedx }
#define DS  { am_reg, x86::ids }
#define ES  { am_reg, x86::ies }
#define FS  { am_reg, x86::ifs }
#define GS  { am_reg, x86::igs }
#define SS  { am_reg, x86::iss }
#define eSP { am_reg, x86::iesp }
#define eBP { am_reg, x86::iebp }
#define eSI { am_reg, x86::iesi }
#define eDI { am_reg, x86::iedi }
#define ESP { am_reg, x86::iesp }
#define EBP { am_reg, x86::iebp }
#define ESI { am_reg, x86::iesi }
#define EDI { am_reg, x86::iedi }
#define ECXEBX { am_tworeghack, op_ecxebx }
#define EDXEAX { am_tworeghack, op_edxeax }
#define rAX { am_reg, x86_64::irax }
#define rBX { am_reg, x86_64::irbx }
#define rCX { am_reg, x86_64::ircx }
#define rDX { am_reg, x86_64::irdx }
#define rSP { am_reg, x86_64::irsp }
#define rBP { am_reg, x86_64::irbp }
#define rSI { am_reg, x86_64::irsi }
#define rDI { am_reg, x86_64::irdi }
#define ST0 { am_reg, x86::ist0 }
#define ST1 { am_reg, x86::ist1 }
#define ST2 { am_reg, x86::ist2 }
#define ST3 { am_reg, x86::ist3 }
#define ST4 { am_reg, x86::ist4 }
#define ST5 { am_reg, x86::ist5 }
#define ST6 { am_reg, x86::ist6 }
#define ST7 { am_reg, x86::ist7 }
#define FPOS 17

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

COMMON_EXPORT dyn_hash_map<entryID, std::string> entryNames_IAPI = map_list_of
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
  (e_addsubpd, "addsubpd")
  (e_addsubps, "addsubps")
  (e_and, "and")
  (e_andnpd, "andnpd")
  (e_andnps, "andnps")
  (e_andpd, "andpd")
  (e_andps, "andps")
  (e_arpl, "arpl")
  (e_blendpd,"blendpd")
  (e_blendps, "blendps")
  (e_blendvpd, "blendvpd")
  (e_blendvps, "blendvps")
  (e_bound, "bound")
  (e_bsf, "bsf")
  (e_bsr, "bsr")
  (e_bswap, "bswap")
  (e_bt, "bt")
  (e_btc, "btc")
  (e_btr, "btr")
  (e_bts, "bts")
  (e_call, "call")
  (e_cbw, "cbw")
  (e_cdq, "cdq")
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
  (e_cmpsd_sse, "cmpsd")
  (e_cmpss, "cmpss")
  (e_cmpsw, "cmpsw")
  (e_cmpxch, "cmpxch")
  (e_cmpxch8b, "cmpxch8b")
  (e_comisd, "comisd")
  (e_comiss, "comiss")
  (e_cpuid, "cpuid")
  (e_crc32, "crc32")
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
  (e_cwd, "cwd")
  (e_cwde, "cwde")
  (e_daa, "daa")
  (e_das, "das")
  (e_dec, "dec")
  (e_div, "div")
  (e_divpd, "divpd")
  (e_divps, "divps")
  (e_divsd, "divsd")
  (e_divss, "divss")
  (e_dppd, "dppd")
  (e_dpps, "dpps")
  (e_emms, "emms")
  (e_enter, "enter")
  (e_extractps, "extractps")
  (e_extrq, "extrq")
  (e_fadd, "fadd")
  (e_faddp, "faddp")
  (e_fbld, "fbld")
  (e_fbstp, "fbstp")
  (e_fcom, "fcom")
  (e_fcomp, "fcomp")
  (e_fdiv, "fdiv")
  (e_fdivr, "fdivr")
  (e_femms, "femms")
  (e_fiadd, "fiadd")
  (e_ficom, "ficom")
  (e_ficomp, "ficomp")
  (e_fidiv, "fidiv")
  (e_fidivr, "fidivr")
  (e_fild, "fild")
  (e_fimul, "fimul")
  (e_fist, "fist")
  (e_fistp, "fistp")
  (e_fisttp, "fisttp")
  (e_fisub, "fisub")
  (e_fisubr, "fisubr")
  (e_fld, "fld")
  (e_fldcw, "fldcw")
  (e_fldenv, "fldenv")
  (e_fmul, "fmul")
  (e_fnop, "fnop")
  (e_frstor, "frstor")
  (e_fsave, "fsave")
  (e_fst, "fst")
  (e_fstcw, "fstcw")
  (e_fstenv, "fstenv")
  (e_fstp, "fstp")
  (e_fstsw, "fstsw")
  (e_fsub, "fsub")
  (e_fsubr, "fsubr")
  (e_fucomp, "fucomp")
  (e_fucompp, "fucompp")
  (e_fxrstor, "fxrstor")
  (e_fxsave, "fxsave")
  (e_haddpd, "haddpd")
  (e_haddps, "haddps")
  (e_hlt, "hlt")
  (e_hsubpd, "hsubpd")
  (e_hsubps, "hsubps")
  (e_idiv, "idiv")
  (e_imul, "imul")
  (e_in, "in")
  (e_inc, "inc")
  (e_insb, "insb")
  (e_insd, "insd")
  (e_insertps, "insertps")
  (e_insertq, "insertq")
  (e_insw, "insw")
  (e_int, "int")
  (e_int3, "int 3")
  (e_int1, "int1")
  (e_int80, "int 80")
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
  (e_lddqu, "lddqu")
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
  (e_lodsd, "lodsd")
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
  (e_movddup, "movddup")
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
  (e_movntdqa, "movntdqa")
  (e_movnti, "movnti")
  (e_movntpd, "movntpd")
  (e_movntps, "movntps")
  (e_movntq, "movntq")
  (e_movq, "movq")
  (e_movq2dq, "movq2dq")
  (e_movsb, "movsb")
  (e_movsd, "movsd")
  (e_movsd_sse, "movsd")
  (e_movshdup, "movshdup")
  (e_movsldup, "movsldup")
  (e_movss, "movss")
  (e_movsw, "movsw")
  (e_movsx, "movsx")
  (e_movsxd, "movsxd")
  (e_movupd, "movupd")
  (e_movups, "movups")
  (e_movzx, "movzx")
  (e_mpsadbw, "mpsadbw")
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
  (e_outsd, "outsd")
  (e_outsw, "outsw")
  (e_pabsb, "pabsb")
  (e_pabsd, "pabsd")
  (e_pabsw, "pabsw")
  (e_packssdw, "packssdw")
  (e_packsswb, "packsswb")
  (e_packusdw, "packusdw")
  (e_packuswb, "packuswb")
  (e_paddb, "paddb")
  (e_paddd, "paddd")
  (e_paddq, "paddq")
  (e_paddsb, "paddsb")
  (e_paddsw, "paddsw")
  (e_paddusb, "paddusb")
  (e_paddusw, "paddusw")
  (e_paddw, "paddw")
  (e_palignr, "palignr")
  (e_pand, "pand")
  (e_pandn, "pandn")
  (e_pavgb, "pavgb")
  (e_pavgw, "pavgw")
  (e_pblendvb, "pblendvb")
  (e_pblendw, "pblendw")
  (e_pcmpeqb, "pcmpeqb")
  (e_pcmpeqd, "pcmpeqd")
  (e_pcmpeqq, "pcmpeqq")
  (e_pcmpeqw, "pcmpeqw")
  (e_pcmpestri, "pcmpestri")
  (e_pcmpestrm, "pcmpestrm")
  (e_pcmpgdt, "pcmpgdt")
  (e_pcmpgtb, "pcmpgtb")
  (e_pcmpgtq, "pcmpgtq")
  (e_pcmpgtw, "pcmpgtw")
  (e_pcmpistri, "pcmpistri")
  (e_pcmpistrm, "pcmpistrm")
  (e_pextrb, "pextrb")
  (e_pextrd_pextrq, "pextrd/pextrq")
  (e_pextrw, "pextrw")
  (e_phaddd, "phaddd")
  (e_phaddsw, "phaddsw")
  (e_phaddw, "phaddw")
  (e_phminposuw, "phminposuw")
  (e_phsubd, "phsubd")
  (e_phsubsw, "phsubsw")
  (e_phsubw, "phsubw")
  (e_pinsrb, "pinsrb")
  (e_pinsrd_pinsrq, "pinsrd/pinsrq")
  (e_pinsrw, "pinsrw")
  (e_pmaddubsw, "pmaddubsw")
  (e_pmaddwd, "pmaddwd")
  (e_pmaxsb, "pmaxsb")
  (e_pmaxsd, "pmaxsd")
  (e_pmaxsw, "pmaxsw")
  (e_pmaxub, "pmaxub")
  (e_pmaxud, "pmaxud")
  (e_pmaxuw, "pmaxuw")
  (e_pminsb, "pminsb")
  (e_pminsd, "pminsd")
  (e_pminsw, "pminsw")
  (e_pminub, "pminub")
  (e_pminud, "pminud")
  (e_pminuw, "pminuw")
  (e_pmovmskb, "pmovmskb")
  (e_pmovsxbd, "pmovsxbd")
  (e_pmovsxbq, "pmovsxbq")
  (e_pmovsxbw, "pmovsxbw")
  (e_pmovsxdq, "pmovsxdq")
  (e_pmovsxwd, "pmovsxwd")
  (e_pmovsxwq, "pmovsxwq")
  (e_pmovzxbd, "pmovzxbd")
  (e_pmovzxbq, "pmovzxbq")
  (e_pmovzxbw, "pmovzxbw")
  (e_pmovzxdq, "pmovzxdq")
  (e_pmovzxwd, "pmovzxwd")
  (e_pmovzxwq, "pmovzxwq")
  (e_pmuldq, "pmuldq")
  (e_pmulhrsw, "pmulhrsw")
  (e_pmulhuw, "pmulhuw")
  (e_pmulhw, "pmulhw")
  (e_pmullw, "pmullw")
  (e_pmulld, "pmulld")
  (e_pmuludq, "pmuludq")
  (e_pop, "pop")
  (e_popa, "popa")
  (e_popad, "popad")
  (e_popcnt, "popcnt")
  (e_popf, "popf")
  (e_popfd, "popfd")
  (e_por, "por")
  (e_prefetch, "prefetch")
  (e_prefetchNTA, "prefetchNTA")
  (e_prefetchT0, "prefetchT0")
  (e_prefetchT1, "prefetchT1")
  (e_prefetchT2, "prefetchT2")
  (e_prefetch_w, "prefetch(w)")
  (e_prefetchw, "prefetchw")
  (e_psadbw, "psadbw")
  (e_pshufb, "pshufb")
  (e_pshufd, "pshufd")
  (e_pshufhw, "pshufhw")
  (e_pshuflw, "pshuflw")
  (e_pshufw, "pshufw")
  (e_psignb, "psignb")
  (e_psignd, "psignd")
  (e_psignw, "psignw")
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
  (e_ptest, "ptest")
  (e_punpckhbw, "punpckhbw")
  (e_punpckhdq, "punpckhdq")
  (e_punpckhqd, "punpckhqd")
  (e_punpckhwd, "punpckhwd")
  (e_punpcklbw, "punpcklbw")
  (e_punpcklqd, "punpcklqd")
  (e_punpcklqld, "punpcklqld")
  (e_punpcklwd, "punpcklwd")
  (e_push, "push")
  (e_pusha, "pusha")
  (e_pushad, "pushad")
  (e_pushf, "pushf")
  (e_pushfd, "pushfd")
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
  (e_roundpd, "roundpd")
  (e_roundps, "roundps")
  (e_roundsd, "roundsd")
  (e_roundss, "roundss")
  (e_rsm, "rsm")
  (e_rsqrtps, "rsqrtps")
  (e_rsqrtss, "rsqrtss")
  (e_sahf, "sahf")
  (e_salc, "salc")
  (e_sar, "sar")
  (e_sbb, "sbb")
  (e_scasb, "scasb")
  (e_scasd, "scasd")
  (e_scasw, "scasw")
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
  (e_stosd, "stosd")
  (e_stosw, "stosw")
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
  (e_xorps, "xorps")
  (e_fp_generic, "[FIXME: GENERIC FPU INSN]")
  (e_3dnow_generic, "[FIXME: GENERIC 3DNow INSN]")
        ;

dyn_hash_map<prefixEntryID, std::string> prefixEntryNames_IAPI = map_list_of
  (prefix_rep, "REP")
  (prefix_repnz, "REPNZ")
        ;

COMMON_EXPORT dyn_hash_map<entryID, flagInfo> const& ia32_instruction::getFlagTable()
{
  static dyn_hash_map<entryID, flagInfo> flagTable;
  if(flagTable.empty()) 
  {
    ia32_instruction::initFlagTable(flagTable);
  }
  return flagTable;
}
  
void ia32_instruction::initFlagTable(dyn_hash_map<entryID, flagInfo>& flagTable)
{
  static const vector<Dyninst::MachRegister> standardFlags = list_of(x86::of)(x86::sf)(x86::zf)(x86::af)(x86::pf)(x86::cf);

  flagTable[e_aaa] = flagInfo(list_of(x86::af), standardFlags);
  flagTable[e_aad] = flagInfo(vector<Dyninst::MachRegister>(), standardFlags);
  flagTable[e_aam] = flagInfo(vector<Dyninst::MachRegister>(), standardFlags);
  flagTable[e_aas] = flagInfo(list_of(x86::af), standardFlags);
  flagTable[e_adc] = flagInfo(list_of(x86::cf), standardFlags);
  flagTable[e_add] = flagInfo(vector<Dyninst::MachRegister>(), standardFlags);
  flagTable[e_and] = flagInfo(vector<Dyninst::MachRegister>(), standardFlags);
  flagTable[e_arpl] = flagInfo(vector<Dyninst::MachRegister>(), list_of(x86::zf));
  flagTable[e_bsf] = flagInfo(vector<Dyninst::MachRegister>(), standardFlags);
  flagTable[e_bsr] = flagInfo(vector<Dyninst::MachRegister>(), standardFlags);
  flagTable[e_bt] = flagInfo(vector<Dyninst::MachRegister>(), standardFlags);
  flagTable[e_bts] = flagInfo(vector<Dyninst::MachRegister>(), standardFlags);
  flagTable[e_btr] = flagInfo(vector<Dyninst::MachRegister>(), standardFlags);
  flagTable[e_btc] = flagInfo(vector<Dyninst::MachRegister>(), standardFlags);
  flagTable[e_clc] = flagInfo(vector<Dyninst::MachRegister>(), list_of(x86::cf));
  flagTable[e_cld] = flagInfo(vector<Dyninst::MachRegister>(), list_of(x86::df));
  flagTable[e_cli] = flagInfo(vector<Dyninst::MachRegister>(), list_of(x86::if_));
  flagTable[e_cmc] = flagInfo(vector<Dyninst::MachRegister>(), list_of(x86::cf));
  flagTable[e_cmovbe] = flagInfo(list_of(x86::of)(x86::sf)(x86::zf)(x86::pf)(x86::cf), vector<Dyninst::MachRegister>());
  flagTable[e_cmove] = flagInfo(list_of(x86::of)(x86::sf)(x86::zf)(x86::pf)(x86::cf), vector<Dyninst::MachRegister>());
  flagTable[e_cmovnae] = flagInfo(list_of(x86::of)(x86::sf)(x86::zf)(x86::pf)(x86::cf), vector<Dyninst::MachRegister>());
  flagTable[e_cmovnb] = flagInfo(list_of(x86::of)(x86::sf)(x86::zf)(x86::pf)(x86::cf), vector<Dyninst::MachRegister>());
  flagTable[e_cmovnbe] = flagInfo(list_of(x86::of)(x86::sf)(x86::zf)(x86::pf)(x86::cf), vector<Dyninst::MachRegister>());
  flagTable[e_cmovne] = flagInfo(list_of(x86::of)(x86::sf)(x86::zf)(x86::pf)(x86::cf), vector<Dyninst::MachRegister>());
  flagTable[e_cmovng] = flagInfo(list_of(x86::of)(x86::sf)(x86::zf)(x86::pf)(x86::cf), vector<Dyninst::MachRegister>());
  flagTable[e_cmovnge] = flagInfo(list_of(x86::of)(x86::sf)(x86::zf)(x86::pf)(x86::cf), vector<Dyninst::MachRegister>());
  flagTable[e_cmovnl] = flagInfo(list_of(x86::of)(x86::sf)(x86::zf)(x86::pf)(x86::cf), vector<Dyninst::MachRegister>());
  flagTable[e_cmovno] = flagInfo(list_of(x86::of)(x86::sf)(x86::zf)(x86::pf)(x86::cf), vector<Dyninst::MachRegister>());
  flagTable[e_cmovns] = flagInfo(list_of(x86::of)(x86::sf)(x86::zf)(x86::pf)(x86::cf), vector<Dyninst::MachRegister>());
  flagTable[e_cmovo] = flagInfo(list_of(x86::of)(x86::sf)(x86::zf)(x86::pf)(x86::cf), vector<Dyninst::MachRegister>());
  flagTable[e_cmovpe] = flagInfo(list_of(x86::of)(x86::sf)(x86::zf)(x86::pf)(x86::cf), vector<Dyninst::MachRegister>());
  flagTable[e_cmovpo] = flagInfo(list_of(x86::of)(x86::sf)(x86::zf)(x86::pf)(x86::cf), vector<Dyninst::MachRegister>());
  flagTable[e_cmovs] = flagInfo(list_of(x86::of)(x86::sf)(x86::zf)(x86::pf)(x86::cf), vector<Dyninst::MachRegister>());
  flagTable[e_cmp] = flagInfo(vector<Dyninst::MachRegister>(), standardFlags);
  flagTable[e_cmpsb] = flagInfo(vector<Dyninst::MachRegister>(), standardFlags);
  flagTable[e_cmpsd] = flagInfo(vector<Dyninst::MachRegister>(), standardFlags);
  flagTable[e_cmpss] = flagInfo(vector<Dyninst::MachRegister>(), standardFlags);
  flagTable[e_cmpsw] = flagInfo(vector<Dyninst::MachRegister>(), standardFlags);
  flagTable[e_cmpxch] = flagInfo(vector<Dyninst::MachRegister>(), standardFlags);
  flagTable[e_cmpxch8b] = flagInfo(vector<Dyninst::MachRegister>(), list_of(x86::zf));
  flagTable[e_comisd] = flagInfo(vector<Dyninst::MachRegister>(), standardFlags);
  flagTable[e_comiss] = flagInfo(vector<Dyninst::MachRegister>(), standardFlags);
  flagTable[e_daa] = flagInfo(list_of(x86::af)(x86::cf), standardFlags);
  flagTable[e_das] = flagInfo(list_of(x86::af)(x86::cf), standardFlags);
  flagTable[e_dec] = flagInfo(vector<Dyninst::MachRegister>(), list_of(x86::of)(x86::sf)(x86::zf)(x86::af)(x86::pf));
  flagTable[e_div] = flagInfo(list_of(x86::af)(x86::cf), standardFlags);
  // TODO: FCMOVcc (not in our entry table) (reads zf/pf/cf)
  // TODO: FCOMI/FCOMIP/FUCOMI/FUCOMIP (writes/zf/pf/cf)
  flagTable[e_idiv] = flagInfo(vector<Dyninst::MachRegister>(), standardFlags);
  flagTable[e_imul] = flagInfo(vector<Dyninst::MachRegister>(), standardFlags);
  flagTable[e_inc] = flagInfo(vector<Dyninst::MachRegister>(), list_of(x86::of)(x86::sf)(x86::zf)(x86::af)(x86::pf));
  flagTable[e_insb] = flagInfo(list_of(x86::df), vector<Dyninst::MachRegister>());
  flagTable[e_insw] = flagInfo(list_of(x86::df), vector<Dyninst::MachRegister>());
  flagTable[e_insd] = flagInfo(list_of(x86::df), vector<Dyninst::MachRegister>());
  flagTable[e_int] = flagInfo(vector<Dyninst::MachRegister>(), list_of(x86::tf)(x86::nt_));
  flagTable[e_int3] = flagInfo(vector<Dyninst::MachRegister>(), list_of(x86::tf)(x86::nt_));
  flagTable[e_int80] = flagInfo(vector<Dyninst::MachRegister>(), list_of(x86::tf)(x86::nt_));
  flagTable[e_into] = flagInfo(list_of(x86::of), list_of(x86::tf)(x86::nt_));
  flagTable[e_ucomisd] = flagInfo(vector<Dyninst::MachRegister>(), standardFlags);
  flagTable[e_ucomiss] = flagInfo(vector<Dyninst::MachRegister>(), standardFlags);
  flagTable[e_iret] = flagInfo(list_of(x86::nt_),
list_of(x86::of)(x86::sf)(x86::zf)(x86::af)(x86::pf)(x86::cf)(x86::tf)(x86::if_)(x86::df));
  flagTable[e_jb] = flagInfo(list_of(x86::of)(x86::sf)(x86::zf)(x86::pf)(x86::cf), vector<Dyninst::MachRegister>());
  flagTable[e_jb_jnaej_j] = flagInfo(list_of(x86::of)(x86::sf)(x86::zf)(x86::pf)(x86::cf), vector<Dyninst::MachRegister>());
  flagTable[e_jbe] = flagInfo(list_of(x86::of)(x86::sf)(x86::zf)(x86::pf)(x86::cf), vector<Dyninst::MachRegister>());
  flagTable[e_jl] = flagInfo(list_of(x86::of)(x86::sf)(x86::zf)(x86::pf)(x86::cf), vector<Dyninst::MachRegister>());
  flagTable[e_jle] = flagInfo(list_of(x86::of)(x86::sf)(x86::zf)(x86::pf)(x86::cf), vector<Dyninst::MachRegister>());
  flagTable[e_jnb] = flagInfo(list_of(x86::of)(x86::sf)(x86::zf)(x86::pf)(x86::cf), vector<Dyninst::MachRegister>());
  flagTable[e_jnb_jae_j] = flagInfo(list_of(x86::of)(x86::sf)(x86::zf)(x86::pf)(x86::cf), vector<Dyninst::MachRegister>());
  flagTable[e_jnbe] = flagInfo(list_of(x86::of)(x86::sf)(x86::zf)(x86::pf)(x86::cf), vector<Dyninst::MachRegister>());
  flagTable[e_jnl] = flagInfo(list_of(x86::of)(x86::sf)(x86::zf)(x86::pf)(x86::cf), vector<Dyninst::MachRegister>());
  flagTable[e_jnle] = flagInfo(list_of(x86::of)(x86::sf)(x86::zf)(x86::pf)(x86::cf), vector<Dyninst::MachRegister>());
  flagTable[e_jno] = flagInfo(list_of(x86::of)(x86::sf)(x86::zf)(x86::pf)(x86::cf), vector<Dyninst::MachRegister>());
  flagTable[e_jnp] = flagInfo(list_of(x86::of)(x86::sf)(x86::zf)(x86::pf)(x86::cf), vector<Dyninst::MachRegister>());
  flagTable[e_jns] = flagInfo(list_of(x86::of)(x86::sf)(x86::zf)(x86::pf)(x86::cf), vector<Dyninst::MachRegister>());
  flagTable[e_jnz] = flagInfo(list_of(x86::of)(x86::sf)(x86::zf)(x86::pf)(x86::cf), vector<Dyninst::MachRegister>());
  flagTable[e_jo] = flagInfo(list_of(x86::of)(x86::sf)(x86::zf)(x86::pf)(x86::cf), vector<Dyninst::MachRegister>());
  flagTable[e_jp] = flagInfo(list_of(x86::of)(x86::sf)(x86::zf)(x86::pf)(x86::cf), vector<Dyninst::MachRegister>());
  flagTable[e_js] = flagInfo(list_of(x86::of)(x86::sf)(x86::zf)(x86::pf)(x86::cf), vector<Dyninst::MachRegister>());
  flagTable[e_jz] = flagInfo(list_of(x86::of)(x86::sf)(x86::zf)(x86::pf)(x86::cf), vector<Dyninst::MachRegister>());
  flagTable[e_lar] = flagInfo(vector<Dyninst::MachRegister>(), list_of(x86::zf));
  flagTable[e_lodsb] = flagInfo(list_of(x86::df), vector<Dyninst::MachRegister>());
  flagTable[e_lodsd] = flagInfo(list_of(x86::df), vector<Dyninst::MachRegister>());
  flagTable[e_lodsw] = flagInfo(list_of(x86::df), vector<Dyninst::MachRegister>());
  flagTable[e_loope] = flagInfo(list_of(x86::zf), vector<Dyninst::MachRegister>());
  flagTable[e_loopn] = flagInfo(list_of(x86::zf), vector<Dyninst::MachRegister>());
  flagTable[e_lsl] = flagInfo(vector<Dyninst::MachRegister>(), list_of(x86::zf));
  // I'd expect that mov control/debug/test gets handled when we do operand analysis
  // If it doesn't, fix later
  flagTable[e_mul] = flagInfo(vector<Dyninst::MachRegister>(), standardFlags);
  flagTable[e_neg] = flagInfo(vector<Dyninst::MachRegister>(), standardFlags);
  flagTable[e_or] = flagInfo(vector<Dyninst::MachRegister>(), standardFlags);
  flagTable[e_outsb] = flagInfo(list_of(x86::df), vector<Dyninst::MachRegister>());
  flagTable[e_outsw] = flagInfo(list_of(x86::df), vector<Dyninst::MachRegister>());
  flagTable[e_outsd] = flagInfo(list_of(x86::df), vector<Dyninst::MachRegister>());
  flagTable[e_popf] = flagInfo(vector<Dyninst::MachRegister>(),
list_of(x86::of)(x86::sf)(x86::zf)(x86::af)(x86::pf)(x86::cf)(x86::tf)(x86::if_)(x86::df)(x86::nt_));
  flagTable[e_popfd] = flagInfo(vector<Dyninst::MachRegister>(),
list_of(x86::of)(x86::sf)(x86::zf)(x86::af)(x86::pf)(x86::cf)(x86::tf)(x86::if_)(x86::df)(x86::nt_));
  flagTable[e_rcl] = flagInfo(list_of(x86::cf), list_of(x86::of)(x86::cf));
  flagTable[e_rcr] = flagInfo(list_of(x86::cf), list_of(x86::of)(x86::cf));
  flagTable[e_rol] = flagInfo(list_of(x86::cf), list_of(x86::of)(x86::cf));
  flagTable[e_ror] = flagInfo(list_of(x86::cf), list_of(x86::of)(x86::cf));
  flagTable[e_rsm] = flagInfo(vector<Dyninst::MachRegister>(),
list_of(x86::of)(x86::sf)(x86::zf)(x86::af)(x86::pf)(x86::cf)(x86::tf)(x86::if_)(x86::df)(x86::nt_)(x86::rf));
  flagTable[e_sahf] = flagInfo(list_of(x86::sf)(x86::zf)(x86::af)(x86::pf)(x86::cf), vector<Dyninst::MachRegister>());
  flagTable[e_sar] = flagInfo(vector<Dyninst::MachRegister>(), standardFlags);
  flagTable[e_shr] = flagInfo(vector<Dyninst::MachRegister>(), standardFlags);
  flagTable[e_salc] = flagInfo(list_of(x86::cf), vector<Dyninst::MachRegister>());
  flagTable[e_sbb] = flagInfo(list_of(x86::cf), standardFlags);
  flagTable[e_setb] = flagInfo(list_of(x86::of)(x86::sf)(x86::zf)(x86::pf)(x86::cf), vector<Dyninst::MachRegister>());
  flagTable[e_setbe] = flagInfo(list_of(x86::of)(x86::sf)(x86::zf)(x86::pf)(x86::cf), vector<Dyninst::MachRegister>());
  flagTable[e_setl] = flagInfo(list_of(x86::of)(x86::sf)(x86::zf)(x86::pf)(x86::cf), vector<Dyninst::MachRegister>());
  flagTable[e_setle] = flagInfo(list_of(x86::of)(x86::sf)(x86::zf)(x86::pf)(x86::cf), vector<Dyninst::MachRegister>());
  flagTable[e_setnb] = flagInfo(list_of(x86::of)(x86::sf)(x86::zf)(x86::pf)(x86::cf), vector<Dyninst::MachRegister>());
  flagTable[e_setnbe] = flagInfo(list_of(x86::of)(x86::sf)(x86::zf)(x86::pf)(x86::cf), vector<Dyninst::MachRegister>());
  flagTable[e_setnl] = flagInfo(list_of(x86::of)(x86::sf)(x86::zf)(x86::pf)(x86::cf), vector<Dyninst::MachRegister>());
  flagTable[e_setnle] = flagInfo(list_of(x86::of)(x86::sf)(x86::zf)(x86::pf)(x86::cf), vector<Dyninst::MachRegister>());
  flagTable[e_setno] = flagInfo(list_of(x86::of)(x86::sf)(x86::zf)(x86::pf)(x86::cf), vector<Dyninst::MachRegister>());
  flagTable[e_setnp] = flagInfo(list_of(x86::of)(x86::sf)(x86::zf)(x86::pf)(x86::cf), vector<Dyninst::MachRegister>());
  flagTable[e_setns] = flagInfo(list_of(x86::of)(x86::sf)(x86::zf)(x86::pf)(x86::cf), vector<Dyninst::MachRegister>());
  flagTable[e_setnz] = flagInfo(list_of(x86::of)(x86::sf)(x86::zf)(x86::pf)(x86::cf), vector<Dyninst::MachRegister>());
  flagTable[e_seto] = flagInfo(list_of(x86::of)(x86::sf)(x86::zf)(x86::pf)(x86::cf), vector<Dyninst::MachRegister>());
  flagTable[e_setp] = flagInfo(list_of(x86::of)(x86::sf)(x86::zf)(x86::pf)(x86::cf), vector<Dyninst::MachRegister>());
  flagTable[e_sets] = flagInfo(list_of(x86::of)(x86::sf)(x86::zf)(x86::pf)(x86::cf), vector<Dyninst::MachRegister>());
  flagTable[e_setz] = flagInfo(list_of(x86::of)(x86::sf)(x86::zf)(x86::pf)(x86::cf), vector<Dyninst::MachRegister>());
  flagTable[e_shld] = flagInfo(vector<Dyninst::MachRegister>(), standardFlags);
  flagTable[e_shrd] = flagInfo(vector<Dyninst::MachRegister>(), standardFlags);
  flagTable[e_shl_sal] = flagInfo(vector<Dyninst::MachRegister>(), standardFlags);
  flagTable[e_stc] = flagInfo(vector<Dyninst::MachRegister>(), list_of(x86::cf));
  flagTable[e_std] = flagInfo(vector<Dyninst::MachRegister>(), list_of(x86::df));
  flagTable[e_sti] = flagInfo(vector<Dyninst::MachRegister>(), list_of(x86::if_));
  flagTable[e_stosb] = flagInfo(list_of(x86::df), vector<Dyninst::MachRegister>());
  flagTable[e_stosd] = flagInfo(list_of(x86::df), vector<Dyninst::MachRegister>());
  flagTable[e_stosw] = flagInfo(list_of(x86::df), vector<Dyninst::MachRegister>());
  flagTable[e_sub] = flagInfo(vector<Dyninst::MachRegister>(), standardFlags);
  flagTable[e_test] = flagInfo(vector<Dyninst::MachRegister>(), standardFlags);
  flagTable[e_verr] = flagInfo(vector<Dyninst::MachRegister>(), list_of(x86::zf));
  flagTable[e_verw] = flagInfo(vector<Dyninst::MachRegister>(), list_of(x86::zf));
  flagTable[e_xadd] = flagInfo(vector<Dyninst::MachRegister>(), standardFlags);
  flagTable[e_xor] = flagInfo(vector<Dyninst::MachRegister>(), standardFlags);
  flagTable[e_scasb] = flagInfo(list_of(x86::df), standardFlags);
  flagTable[e_scasw] = flagInfo(list_of(x86::df), standardFlags);
  flagTable[e_scasd] = flagInfo(list_of(x86::df), standardFlags);
  flagTable[e_pcmpestri] = flagInfo(vector<Dyninst::MachRegister>(), standardFlags);
  flagTable[e_pcmpestrm] = flagInfo(vector<Dyninst::MachRegister>(), standardFlags);
  flagTable[e_pcmpistri] = flagInfo(vector<Dyninst::MachRegister>(), standardFlags);
  flagTable[e_pcmpistrm] = flagInfo(vector<Dyninst::MachRegister>(), standardFlags);
  flagTable[e_popcnt] = flagInfo(list_of(x86::of)(x86::sf)(x86::zf)(x86::af)(x86::cf)(x86::pf), vector<Dyninst::MachRegister>());
  flagTable[e_ptest] = flagInfo(list_of(x86::of)(x86::sf)(x86::zf)(x86::af)(x86::cf)(x86::pf), vector<Dyninst::MachRegister>());
}

bool ia32_entry::flagsUsed(std::set<MachRegister>& flagsRead, std::set<MachRegister>& flagsWritten, ia32_locations* locs)
{
  dyn_hash_map<entryID, flagInfo>::const_iterator found = ia32_instruction::getFlagTable().find(getID(locs));
  if(found == ia32_instruction::getFlagTable().end())
  {
    return false;
  }
  // No entries for something that touches no flags, so always return true if we had an entry

  copy((found->second).readFlags.begin(), (found->second).readFlags.end(), inserter(flagsRead, flagsRead.begin()));
  copy((found->second).writtenFlags.begin(), (found->second).writtenFlags.end(), inserter(flagsWritten,flagsWritten.begin()));
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
  { e_push, t_done, 0, false, { ES, eSP, Zz }, 0, s1R2RW }, // Semantics rewritten to ignore stack "operand"
  { e_pop,  t_done, 0, false, { ES, eSP, Zz }, 0, s1W2RW },
  /* 08 */
  { e_or,   t_done, 0, 
true, { Eb, Gb, Zz }, 0, s1RW2R },
  { e_or,   t_done, 0, true, { Ev, Gv, Zz }, 0, s1RW2R },
  { e_or,   t_done, 0, true, { Gb, Eb, Zz }, 0, s1RW2R },
  { e_or,   t_done, 0, true, { Gv, Ev, Zz }, 0, s1RW2R },
  { e_or,   t_done, 0, false, { AL, Ib, Zz }, 0, s1RW2R },
  { e_or,   t_done, 0, false, { eAX, Iz, Zz }, 0, s1RW2R },
  { e_push, t_done, 0, false, { CS, eSP, Zz }, 0, s1R2RW },
  { e_No_Entry,      t_twoB, 0, false, { Zz, Zz, Zz }, 0, 0 },
  /* 10 */
  { e_adc,  t_done, 0, true, { Eb, Gb, Zz }, 0, s1RW2R },
  { e_adc,  t_done, 0, true, { Ev, Gv, Zz }, 0, s1RW2R },
  { e_adc,  t_done, 0, true, { Gb, Eb, Zz }, 0, s1RW2R },
  { e_adc,  t_done, 0, true, { Gv, Ev, Zz }, 0, s1RW2R },
  { e_adc,  t_done, 0, false, { AL, Ib, Zz }, 0, s1RW2R },
  { e_adc,  t_done, 0, false, { eAX, Iz, Zz }, 0, s1RW2R },
  { e_push, t_done, 0, false, { SS, eSP, Zz }, 0, s1R2RW },
  { e_pop,  t_done, 0, false, { SS, eSP, Zz }, 0, s1W2RW },
  /* 18 */
  { e_sbb,  t_done, 0, true, { Eb, Gb, Zz }, 0, s1RW2R },
  { e_sbb,  t_done, 0, true, { Ev, Gv, Zz }, 0, s1RW2R },
  { e_sbb,  t_done, 0, true, { Gb, Eb, Zz }, 0, s1RW2R },
  { e_sbb,  t_done, 0, true, { Gv, Ev, Zz }, 0, s1RW2R },
  { e_sbb,  t_done, 0, false, { AL, Ib, Zz }, 0, s1RW2R },
  { e_sbb,  t_done, 0, false, { eAX, Iz, Zz }, 0, s1RW2R },
  { e_push, t_done, 0, false, { DS, eSP, Zz }, 0, s1R2RW },
  { e_pop , t_done, 0, false, { DS, eSP, Zz }, 0, s1W2RW },
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
  { e_aaa, t_done, 0, false, { AX, Zz, Zz }, 0, s1RW },
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
  { e_push, t_done, 0, false, { rAX, eSP, Zz }, 0, s1R2RW },
  { e_push, t_done, 0, false, { rCX, eSP, Zz }, 0, s1R2RW },
  { e_push, t_done, 0, false, { rDX, eSP, Zz }, 0, s1R2RW },
  { e_push, t_done, 0, false, { rBX, eSP, Zz }, 0, s1R2RW },
  { e_push, t_done, 0, false, { rSP, eSP, Zz }, 0, s1R2RW },
  { e_push, t_done, 0, false, { rBP, eSP, Zz }, 0, s1R2RW },
  { e_push, t_done, 0, false, { rSI, eSP, Zz }, 0, s1R2RW },
  { e_push, t_done, 0, false, { rDI, eSP, Zz }, 0, s1R2RW },
  /* 58 */
  { e_pop, t_done, 0, false, { rAX, eSP, Zz }, 0, s1W2RW },
  { e_pop, t_done, 0, false, { rCX, eSP, Zz }, 0, s1W2RW },
  { e_pop, t_done, 0, false, { rDX, eSP, Zz }, 0, s1W2RW },
  { e_pop, t_done, 0, false, { rBX, eSP, Zz }, 0, s1W2RW },
  { e_pop, t_done, 0, false, { rSP, eSP, Zz }, 0, s1W2RW },
  { e_pop, t_done, 0, false, { rBP, eSP, Zz }, 0, s1W2RW },
  { e_pop, t_done, 0, false, { rSI, eSP, Zz }, 0, s1W2RW },
  { e_pop, t_done, 0, false, { rDI, eSP, Zz }, 0, s1W2RW },
  /* 60 */
  { e_pushad, t_done, 0, false, { GPRS, eSP, Zz }, 0, s1R2RW },
  { e_popad,  t_done, 0, false, { GPRS, eSP, Zz }, 0, s1W2RW },
  { e_bound,    t_done, 0, true, { Gv, Ma, Zz }, 0, s1R2R },
  { e_arpl,     t_done, 0, true, { Ew, Gw, Zz }, 0, s1R2R },
  { e_No_Entry,          t_ill,  0, false, { Zz, Zz, Zz }, 0, 0 }, // PREFIX_SEG_OVR
  { e_No_Entry,          t_ill,  0, false, { Zz, Zz, Zz }, 0, 0 }, // PREFIX_SEG_OVR
  { e_No_Entry,   t_prefixedSSE, 2, false, { Zz, Zz, Zz }, 0, 0 }, /* operand size prefix (PREFIX_OPR_SZ)*/
  { e_No_Entry,          t_ill,  0, false, { Zz, Zz, Zz }, 0, 0 }, /* address size prefix (PREFIX_ADDR_SZ)*/
  /* 68 */
  { e_push,    t_done, 0, false, { Iz, eSP, Zz }, 0, s1R2RW },
  { e_imul,    t_done, 0, true, { Gv, Ev, Iz }, 0, s1W2R3R },
  { e_push,    t_done, 0, false, { Ib, eSP, Zz }, 0, s1R2RW },
  { e_imul,    t_done, 0, true, { Gv, Ev, Ib }, 0, s1W2R3R },
  { e_insb,    t_done, 0, false, { Yb, DX, Zz }, 0, s1W2R | (fREP << FPOS) }, // (e)SI/DI changed
  { e_insd,  t_done, 0, false, { Yv, DX, Zz }, 0, s1W2R | (fREP << FPOS) },
  { e_outsb,   t_done, 0, false, { DX, Xb, Zz }, 0, s1W2R | (fREP << FPOS) },
  { e_outsd, t_done, 0, false, { DX, Xv, Zz }, 0, s1W2R | (fREP << FPOS) },
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
  { e_lea, t_done, 0, true, { Gv, Mlea, Zz }, IS_NOP, s1W2R }, // this is just M in the book
                                                        // AFAICT the 2nd operand is not accessed
  { e_mov, t_done, 0, true, { Sw, Ew, Zz }, 0, s1W2R },
  { e_pop, t_done, 0, true, { Ev, eSP, Zz }, 0, s1W2RW },
  /* 90 */
  { e_nop,  t_done, 0, false, { Zz, Zz, Zz }, IS_NOP, sNONE }, // actually xchg eax,eax
  { e_xchg, t_done, 0, false, { eCX, eAX, Zz }, 0, s1RW2RW },
  { e_xchg, t_done, 0, false, { eDX, eAX, Zz }, 0, s1RW2RW },
  { e_xchg, t_done, 0, false, { eBX, eAX, Zz }, 0, s1RW2RW },
  { e_xchg, t_done, 0, false, { eSP, eAX, Zz }, 0, s1RW2RW },
  { e_xchg, t_done, 0, false, { eBP, eAX, Zz }, 0, s1RW2RW },
  { e_xchg, t_done, 0, false, { eSI, eAX, Zz }, 0, s1RW2RW },
  { e_xchg, t_done, 0, false, { eDI, eAX, Zz }, 0, s1RW2RW },
  /* 98 */
  { e_cwde, t_done, 0, false, { eAX, Zz, Zz }, 0, s1RW },
  { e_cdq,  t_done, 0, false, { eDX, eAX, Zz }, 0, s1W2R },
  { e_call,     t_done, 0, false, { Ap, Zz, Zz }, IS_CALL | PTR_WX, s1R },
  { e_wait,     t_done, 0, false, { Zz, Zz, Zz }, 0, sNONE },
  { e_pushfd, t_done, 0, false, { Fv, eSP, Zz }, 0, s1R2RW },
  { e_popfd,  t_done, 0, false, { Fv, eSP, Zz }, 0, s1W2RW },
  { e_sahf,     t_done, 0, false, { Zz, Zz, Zz }, 0, 0 }, // FIXME Intel
  { e_lahf,     t_done, 0, false, { Zz, Zz, Zz }, 0, 0 }, // FIXME Intel
  /* A0 */
  { e_mov,   t_done, 0, false, { AL, Ob, Zz },  0, s1W2R },
  { e_mov,   t_done, 0, false, { eAX, Ov, Zz }, 0, s1W2R },
  { e_mov,   t_done, 0, false, { Ob, AL, Zz },  0, s1W2R },
  { e_mov,   t_done, 0, false, { Ov, eAX, Zz }, 0, s1W2R },
  // XXX: Xv is source, Yv is destination for movs, so they're swapped!
  { e_movsb, t_done, 0, false, { Yb, Xb, Zz },  0, s1W2R | (fREP << FPOS) }, // (e)SI/DI changed
  { e_movsd, t_done, 0, false, { Yv, Xv, Zz }, 0, s1W2R | (fREP << FPOS) },
  { e_cmpsb, t_done, 0, false, { Xb, Yb, Zz },  0, s1R2R | (fCMPS << FPOS) },
  { e_cmpsw, t_done, 0, false, { Xv, Yv, Zz },  0, s1R2R | (fCMPS << FPOS) },
  /* A8 */
  { e_test,     t_done, 0, false, { AL, Ib, Zz },  0, s1R2R },
  { e_test,     t_done, 0, false, { eAX, Iz, Zz }, 0, s1R2R },
  { e_stosb,    t_done, 0, false, { Yb, AL, Zz },  0, s1W2R | (fREP << FPOS) },
  { e_stosd,  t_done, 0, false, { Yv, eAX, Zz }, 0, s1W2R | (fREP << FPOS) },
  { e_lodsb,    t_done, 0, false, { AL, Xb, Zz },  0, s1W2R | (fREP << FPOS) },
  { e_lodsd,    t_done, 0, false, { eAX, Xv, Zz }, 0, s1W2R | (fREP << FPOS) },
  { e_scasb,    t_done, 0, false, { AL, Yb, Zz },  0, s1R2R | (fSCAS << FPOS) },
  { e_scasd,  t_done, 0, false, { eAX, Yv, Zz }, 0, s1R2R | (fSCAS << FPOS) },
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
  { e_enter,   t_done, 0, false, { Iw, Ib, Zz }, 0, s1R2R | (fENTER << FPOS) },
  { e_leave,   t_done, 0, false, { Zz, Zz, Zz }, 0, fLEAVE << FPOS },
  { e_ret_far, t_done, 0, false, { Iw, Zz, Zz }, (IS_RETF | IS_RETC), s1R | (fFARRET << FPOS) },
  { e_ret_far, t_done, 0, false, { Zz, Zz, Zz }, (IS_RETF), fFARRET << FPOS },
  { e_int3,   t_done, 0, false, { Zz, Zz, Zz }, 0, sNONE },
  { e_int,     t_done, 0, false, { Ib, Zz, Zz }, 0, s1R },
  { e_into,    t_done, 0, false, { Zz, Zz, Zz }, 0, sNONE },
  { e_iret,    t_done, 0, false, { Zz, Zz, Zz }, (IS_RET), fIRET << FPOS},
  /* D0 */
  { e_No_Entry, t_grp, Grp2, true, { Eb, ImplImm, Zz }, 0, s1RW2R }, // const1
  { e_No_Entry, t_grp, Grp2, true, { Ev, ImplImm, Zz }, 0, s1RW2R }, // --"--
  { e_No_Entry, t_grp, Grp2, true, { Eb, CL, Zz }, 0, s1RW2R },
  { e_No_Entry, t_grp, Grp2, true, { Ev, CL, Zz }, 0, s1RW2R },
  { e_aam,  t_done, 0, false, { AX, Ib, Zz }, 0, s1RW2R },
  { e_aad,  t_done, 0, false, { AX, Ib, Zz }, 0, s1RW2R },
  { e_salc, t_done, 0, false, { Zz, Zz, Zz }, 0, sNONE }, // undocumeted
  { e_xlat, t_done, 0, false, { Zz, Zz, Zz }, 0, fXLAT << FPOS }, // scream
  /* D8 */
  { e_No_Entry, t_coprocEsc, GrpD8, true, { Zz, Zz, Zz }, 0, 0 },
  { e_No_Entry, t_coprocEsc, GrpD9, true, { Zz, Zz, Zz }, 0, 0 },
  { e_No_Entry, t_coprocEsc, GrpDA, true, { Zz, Zz, Zz }, 0, 0 },
  { e_No_Entry, t_coprocEsc, GrpDB, true, { Zz, Zz, Zz }, 0, 0 },
  { e_No_Entry, t_coprocEsc, GrpDC, true, { Zz, Zz, Zz }, 0, 0 },
  { e_No_Entry, t_coprocEsc, GrpDD, true, { Zz, Zz, Zz }, 0, 0 },
  { e_No_Entry, t_coprocEsc, GrpDE, true, { Zz, Zz, Zz }, 0, 0 },
  { e_No_Entry, t_coprocEsc, GrpDF, true, { Zz, Zz, Zz }, 0, 0 },
  /* E0 */
  { e_loopn,    t_done, 0, false, { Jb, eCX, Zz }, (IS_JCC | REL_B), s1R2R }, 
  { e_loope,    t_done, 0, false, { Jb, eCX, Zz }, (IS_JCC | REL_B), s1R2R },
  { e_loop,     t_done, 0, false, { Jb, eCX, Zz }, (IS_JCC | REL_B), s1R2R },
  { e_jcxz_jec, t_done, 0, false, { Jb, eCX, Zz }, (IS_JCC | REL_B), s1R2R },
  { e_in,       t_done, 0, false, { AL, Ib, Zz }, 0, s1W2R | (fIO << FPOS) },
  { e_in,       t_done, 0, false, { eAX, Ib, Zz }, 0, s1W2R | (fIO << FPOS) },
  { e_out,      t_done, 0, false, { Ib, AL, Zz }, 0, s1W2R | (fIO << FPOS) },
  { e_out,      t_done, 0, false, { Ib, eAX, Zz }, 0, s1W2R | (fIO << FPOS) },
  /* E8 */
  { e_call, t_done, 0, false, { Jz, Zz, Zz }, (IS_CALL | REL_X), s1R | (fCALL << FPOS) },
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
  { e_No_Entry,    t_ill, 0, false, { Zz, Zz, Zz }, 0, 0},
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
  { e_nop, t_done, 0, true, { Ev, Zz, Zz }, IS_NOP, 0 }, // 19-1F according to sandpile and AMD are NOPs with an Ev operand
  { e_nop, t_done, 0, true, { Ev, Zz, Zz }, IS_NOP, 0 }, // Can we go out on a limb that the 'operand' of a NOP is never read?
  { e_nop, t_done, 0, true, { Ev, Zz, Zz }, IS_NOP, 0 }, // I think we can...so nullary operand semantics, but consume the
  { e_nop, t_done, 0, true, { Ev, Zz, Zz }, IS_NOP, 0 }, // mod/rm byte operand.
  { e_nop, t_done, 0, true, { Ev, Zz, Zz }, IS_NOP, 0 }, // -- BW 1/08
  { e_nop, t_done, 0, true, { Ev, Zz, Zz }, IS_NOP, 0 },
  { e_nop, t_done, 0, true, { Ev, Zz, Zz }, IS_NOP, 0 },
  /* 20 */
  { e_mov, t_done, 0, true, { Rd, Cd, Zz }, 0, s1W2R },
  { e_mov, t_done, 0, true, { Rd, Dd, Zz }, 0, s1W2R },
  { e_mov, t_done, 0, true, { Cd, Rd, Zz }, 0, s1W2R },
  { e_mov, t_done, 0, true, { Dd, Rd, Zz }, 0, s1W2R },
  { e_mov, t_done, 0, true, { Rd, Td, Zz }, 0, s1W2R }, // actually a SSE5A
  { e_No_Entry,     t_ill,  0, 0, { Zz, Zz, Zz }, 0, 0 }, // SSE5A
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
  { e_wrmsr, t_done, 0, false, { rAX, rDX, rCX }, 0, s1R2R3R },
  { e_rdtsc, t_done, 0, false, { rAX, rDX, Zz }, 0, s1W2W3R },
  { e_rdmsr, t_done, 0, false, { rAX, rDX, rCX }, 0, s1W2W3R },
  { e_rdpmc, t_done, 0, false, { rAX, rDX, rCX }, 0, s1W2W3R },
  { e_sysenter, t_done, 0, false, { Zz, Zz, Zz }, 0, sNONE }, // XXX: fixme for kernel work
  { e_sysexit,  t_done, 0, false, { Zz, Zz, Zz }, 0, sNONE }, // XXX: fixme for kernel work
  { e_No_Entry, t_ill, 0, 0, { Zz, Zz, Zz }, 0, 0 }, 
  { e_No_Entry, t_ill, 0, 0, { Zz, Zz, Zz }, 0 ,0 },
  /* 38 */
  { e_No_Entry, t_threeB, 0, 0, { Zz, Zz, Zz }, 0, 0 }, //3-Byte escape (Book Table A-4)
  { e_No_Entry, t_ill, 0, 0, { Zz, Zz, Zz }, 0 ,0 },
  { e_No_Entry, t_threeB2, 0, 0, { Zz, Zz, Zz }, 0, 0 }, //3-Byte escape (Book Table A-5)
  { e_No_Entry, t_ill, 0, 0, { Zz, Zz, Zz }, 0 ,0 },
  { e_No_Entry, t_ill, 0, 0, { Zz, Zz, Zz }, 0 ,0 },
  { e_No_Entry, t_ill, 0, 0, { Zz, Zz, Zz }, 0 ,0 },
  { e_No_Entry, t_ill, 0, 0, { Zz, Zz, Zz }, 0 ,0 },
  { e_No_Entry, t_ill, 0, 0, { Zz, Zz, Zz }, 0 ,0 },
  /* 40 */
  { e_cmovo,   t_done, 0, true, { Gv, Ev, Zz }, 0, s1RW2R | (fCOND << FPOS) },
  { e_cmovno,  t_done, 0, true, { Gv, Ev, Zz }, 0, s1RW2R | (fCOND << FPOS) },
  { e_cmovnae, t_done, 0, true, { Gv, Ev, Zz }, 0, s1RW2R | (fCOND << FPOS) },
  { e_cmovnb,  t_done, 0, true, { Gv, Ev, Zz }, 0, s1RW2R | (fCOND << FPOS) },
  { e_cmove,   t_done, 0, true, { Gv, Ev, Zz }, 0, s1RW2R | (fCOND << FPOS) },
  { e_cmovne,  t_done, 0, true, { Gv, Ev, Zz }, 0, s1RW2R | (fCOND << FPOS) },
  { e_cmovbe,  t_done, 0, true, { Gv, Ev, Zz }, 0, s1RW2R | (fCOND << FPOS) },
  { e_cmovnbe, t_done, 0, true, { Gv, Ev, Zz }, 0, s1RW2R | (fCOND << FPOS) },
  /* 48 */
  { e_cmovs,   t_done, 0, true, { Gv, Ev, Zz }, 0, s1RW2R | (fCOND << FPOS) },
  { e_cmovns,  t_done, 0, true, { Gv, Ev, Zz }, 0, s1RW2R | (fCOND << FPOS) },
  { e_cmovpe,  t_done, 0, true, { Gv, Ev, Zz }, 0, s1RW2R | (fCOND << FPOS) },
  { e_cmovpo,  t_done, 0, true, { Gv, Ev, Zz }, 0, s1RW2R | (fCOND << FPOS) },
  { e_cmovnge, t_done, 0, true, { Gv, Ev, Zz }, 0, s1RW2R | (fCOND << FPOS) },
  { e_cmovnl,  t_done, 0, true, { Gv, Ev, Zz }, 0, s1RW2R | (fCOND << FPOS) },
  { e_cmovng,  t_done, 0, true, { Gv, Ev, Zz }, 0, s1RW2R | (fCOND << FPOS) },
  { e_cmovnl,  t_done, 0, true, { Gv, Ev, Zz }, 0, s1RW2R | (fCOND << FPOS) },
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
  { e_No_Entry, t_sse, SSE78, 0, { Zz, Zz, Zz }, 0, 0 },
  { e_No_Entry, t_sse, SSE79, 0, { Zz, Zz, Zz }, 0, 0 },
  { e_No_Entry, t_ill, 0, 0, { Zz, Zz, Zz }, 0, 0 }, // SSE5A will go in 7A and 7B when it comes out
  { e_No_Entry, t_ill, 0, 0, { Zz, Zz, Zz }, 0, 0 },
  { e_No_Entry, t_sse, SSE7C, 0, { Zz, Zz, Zz }, 0, 0 },
  { e_No_Entry, t_sse, SSE7D, 0, { Zz, Zz, Zz }, 0, 0 },
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
  { e_push,   t_done, 0, false, { FS, eSP, Zz }, 0, s1R2RW },
  { e_pop,    t_done, 0, false, { FS, eSP, Zz }, 0, s1W2RW },
  { e_cpuid,  t_done, 0, false, { Zz, Zz, Zz }, 0, sNONE },
  { e_bt,     t_done, 0, true, { Ev, Gv, Zz }, 0, s1R2R },
  { e_shld,   t_done, 0, true, { Ev, Gv, Ib }, 0, s1RW2R3R },
  { e_shld,   t_done, 0, true, { Ev, Gv, CL }, 0, s1RW2R3R },
  { e_No_Entry, t_ill, 0, 0, { Zz, Zz, Zz }, 0, 0 }, 
  { e_No_Entry, t_ill, 0, 0, { Zz, Zz, Zz }, 0, 0 },
  /* A8 */
  { e_push, t_done, 0, false, { GS, eSP, Zz }, 0, s1R2RW },
  { e_pop,  t_done, 0, false, { GS, eSP, Zz }, 0, s1W2RW },
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
  { e_No_Entry, t_sse, SSEB8, 0, { Zz, Zz, Zz }, 0, 0 },
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
  { e_movnti , t_done, 0, true, { Ev, Gv, Zz }, 0, s1W2R | (fNT << FPOS) },
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
  { e_No_Entry, t_sse, SSED0, false, { Zz, Zz, Zz }, 0, 0 },
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
  { e_No_Entry, t_sse, SSEF0, false, { Zz, Zz, Zz }, 0, 0 },
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
  { e_No_Entry, t_sse, SSEFF, false, { Zz, Zz, Zz }, 0, 0 }
};

static ia32_entry threeByteMap[256] = {
		/* 00 */
		{ e_No_Entry, t_sse_bis, SSEB00, true, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_sse_bis, SSEB01, true, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_sse_bis, SSEB02, true, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_sse_bis, SSEB03, true, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_sse_bis, SSEB04, true, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_sse_bis, SSEB05, true, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_sse_bis, SSEB06, true, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_sse_bis, SSEB07, true, { Zz, Zz, Zz }, 0, 0 },
		/* 08*/
		{ e_No_Entry, t_sse_bis, SSEB08, true, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_sse_bis, SSEB09, true, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_sse_bis, SSEB0A, true, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_sse_bis, SSEB0B, true, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		/* 10 */
		{ e_No_Entry, t_sse_bis, SSEB10, true, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_sse_bis, SSEB14, true, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_sse_bis, SSEB15, true, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_sse_bis, SSEB17, true, { Zz, Zz, Zz }, 0, 0 },
		/* 18 */
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_sse_bis, SSEB1C, true, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_sse_bis, SSEB1D, true, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_sse_bis, SSEB1E, true, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		/* 20 */
		{ e_No_Entry, t_sse_bis, SSEB20, true, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_sse_bis, SSEB21, true, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_sse_bis, SSEB22, true, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_sse_bis, SSEB23, true, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_sse_bis, SSEB24, true, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_sse_bis, SSEB25, true, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		/* 28 */
		{ e_No_Entry, t_sse_bis, SSEB28, true, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_sse_bis, SSEB29, true, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_sse_bis, SSEB2A, true, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_sse_bis, SSEB2B, true, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		/* 30 */
		{ e_No_Entry, t_sse_bis, SSEB30, true, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_sse_bis, SSEB31, true, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_sse_bis, SSEB32, true, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_sse_bis, SSEB33, true, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_sse_bis, SSEB34, true, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_sse_bis, SSEB35, true, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_sse_bis, SSEB37, true, { Zz, Zz, Zz }, 0, 0 },
		/* 38 */
		{ e_No_Entry, t_sse_bis, SSEB38, true, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_sse_bis, SSEB39, true, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_sse_bis, SSEB3A, true, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_sse_bis, SSEB3B, true, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_sse_bis, SSEB3C, true, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_sse_bis, SSEB3D, true, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_sse_bis, SSEB3E, true, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_sse_bis, SSEB3F, true, { Zz, Zz, Zz }, 0, 0 },
		/* 40 */
		{ e_No_Entry, t_sse_bis, SSEB40, true, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_sse_bis, SSEB41, true, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		/* 48 */
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		/* 50 */
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		/* 58 */
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		/* 60 */
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		/* 68 */
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		/* 70 */
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		/* 78 */
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		/* 80 */
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		/* 88 */
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		/* 90 */
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		/* 98 */
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		/* A0 */
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		/* A8 */
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		/* B0 */
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },	
		/* B8 */
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		/* C0 */
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		/* C8 */
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		/* D0 */
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		/* D8 */
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		/* E0 */
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		/* E8 */
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		/* F0 */
		{ e_No_Entry, t_sse_bis, SSEBF0, true, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_sse_bis, SSEBF1, true, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_grp, Grp17, true, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		/* F8 */
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 }
};

static ia32_entry threeByteMap2[256] = {
		/* 00 */
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		/* 08*/
		{ e_No_Entry, t_sse_ter, SSET08, true, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_sse_ter, SSET09, true, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_sse_ter, SSET0A, true, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_sse_ter, SSET0B, true, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_sse_ter, SSET0C, true, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_sse_ter, SSET0D, true, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_sse_ter, SSET0E, true, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_sse_ter, SSET0F, true, { Zz, Zz, Zz }, 0, 0 },
		/* 10 */
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_sse_ter, SSET14, true, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_sse_ter, SSET15, true, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_sse_ter, SSET16, true, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_sse_ter, SSET17, true, { Zz, Zz, Zz }, 0, 0 },
		/* 18 */
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		/* 20 */
		{ e_No_Entry, t_sse_ter, SSET20, true, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_sse_ter, SSET21, true, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_sse_ter, SSET22, true, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		/* 28 */
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		/* 30 */
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		/* 38 */
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		/* 40 */
		{ e_No_Entry, t_sse_ter, SSET40, true, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_sse_ter, SSET41, true, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_sse_ter, SSET42, true, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		/* 48 */
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		/* 50 */
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		/* 58 */
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		/* 60 */
		{ e_No_Entry, t_sse_ter, SSET60, true, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_sse_ter, SSET61, true, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_sse_ter, SSET62, true, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_sse_ter, SSET63, true, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		/* 68 */
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		/* 70 */
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		/* 78 */
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		/* 80 */
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		/* 88 */
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		/* 90 */
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		/* 98 */
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		/* A0 */
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		/* A8 */
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		/* B0 */
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },	
		/* B8 */
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		/* C0 */
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		/* C8 */
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		/* D0 */
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		/* D8 */
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		/* E0 */
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		/* E8 */
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		/* F0 */
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		/* F8 */
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 }
};

static ia32_entry fpuMap[][2][8] = {
{
    { // D8
        { e_fadd,  t_done, 0, true, { ST0, Ef, Zz }, 0, s1RW2R },
        { e_fmul,  t_done, 0, true, { ST0, Ef, Zz }, 0, s1RW2R },
        { e_fcom,  t_done, 0, true, { ST0, Ef, Zz }, 0, s1RW2R },
        { e_fcomp, t_done, 0, true, { ST0, Ef, Zz }, 0, s1RW2R }, // stack pop
        { e_fsub,  t_done, 0, true, { ST0, Ef, Zz }, 0, s1RW2R },
        { e_fsubr, t_done, 0, true, { ST0, Ef, Zz }, 0, s1RW2R },
        { e_fdiv,  t_done, 0, true, { ST0, Ef, Zz }, 0, s1RW2R },
        { e_fdivr, t_done, 0, true, { ST0, Ef, Zz }, 0, s1RW2R }
    },
    { // D8 -- IMPORTANT NOTE: the C0-FF tables in the book are interleaved from how they
        // need to appear here (and for all FPU insns).  i.e. book rows 0, 4, 1, 5, 2, 6, 3, 7 are table rows
        // 0, 1, 2, 3, 4, 5, 6, 7.
        { e_fadd,  t_done, 0, true, { ST0, Ef, Zz }, 0, s1RW2R },
        { e_fmul,  t_done, 0, true, { ST0, Ef, Zz }, 0, s1RW2R },
        { e_fcom,  t_done, 0, true, { ST0, Ef, Zz }, 0, s1RW2R },
        { e_fcomp, t_done, 0, true, { ST0, Ef, Zz }, 0, s1RW2R }, // stack pop
        { e_fsub,  t_done, 0, true, { ST0, Ef, Zz }, 0, s1RW2R },
        { e_fsubr, t_done, 0, true, { ST0, Ef, Zz }, 0, s1RW2R },
        { e_fdiv,  t_done, 0, true, { ST0, Ef, Zz }, 0, s1RW2R },
        { e_fdivr, t_done, 0, true, { ST0, Ef, Zz }, 0, s1RW2R }
    },
},
{
    { // D9 
        { e_fld,    t_done, 0, true, { ST0, Ef, Zz }, 0, s1W2R }, // stack push
        { e_fnop,   t_done, 0, true, { Zz,  Zz, Zz }, 0, sNONE },
        { e_fst,    t_done, 0, true, { Ef, ST0, Zz }, 0, s1W2R },
        { e_fstp,   t_done, 0, true, { Ef, ST0, Zz }, 0, s1W2R }, // stack pop
        { e_fldenv, t_done, 0, true, { M14, Zz, Zz }, 0, s1R },
        { e_fldcw,  t_done, 0, true, { Ew,  Zz, Zz }, 0, s1R },
        { e_fstenv, t_done, 0, true, { M14, Zz, Zz }, 0, s1W },
        { e_fstcw,  t_done, 0, true, { Ew,  Zz, Zz }, 0, s1W }
    },
    { // D9 
        { e_fld,    t_done, 0, true, { ST0, Ef, Zz }, 0, s1W2R }, // stack push
        { e_fxch, t_done, 0, true, { ST0, Ef, Zz }, 0, s1RW2RW },
        { e_fnop,   t_done, 0, true, { Zz,  Zz, Zz }, 0, sNONE },
        { e_No_Entry,  t_done, 0, true, { Zz,  Zz, Zz }, 0, sNONE },
        { e_fchs,    t_done, 0, true, { ST0, Zz, Zz }, 0, s1RW }, // FIXME: using first of group as placeholder
        { e_fld1, t_done, 0, true, { ST0, Zz, Zz }, 0, s1RW }, // FIXME: using first of group
        { e_f2xm1,   t_done, 0, true, { ST0, ST1, Zz }, 0, s1RW2R }, // FIXME: using first of group as placeholder
        { e_fprem,  t_done, 0, true, { ST0, ST1, Zz }, 0, s1RW2R } // FIXME: using first of group
    },
},
{
    { // DA 
        { e_fiadd,  t_done, 0, true, { ST0, Ev, Zz }, 0, s1RW2R },
        { e_fimul,  t_done, 0, true, { ST0, Ev, Zz }, 0, s1RW2R },
        { e_ficom,  t_done, 0, true, { ST0, Ev, Zz }, 0, s1RW2R },
        { e_ficomp, t_done, 0, true, { ST0, Ev, Zz }, 0, s1RW2R }, // stack pop
        { e_fisub,  t_done, 0, true, { ST0, Ev, Zz }, 0, s1RW2R },
        { e_fisubr, t_done, 0, true, { ST0, Ev, Zz }, 0, s1RW2R },
        { e_fidiv,  t_done, 0, true, { ST0, Ev, Zz }, 0, s1RW2R },
        { e_fidivr, t_done, 0, true, { ST0, Ev, Zz }, 0, s1RW2R }
    },
    { // DA
        { e_fcmovb,  t_done, 0, true, { ST0, Ef, Zz }, 0, s1RW2R },
        { e_fcmove,  t_done, 0, true, { ST0, Ef, Zz }, 0, s1RW2R },
        { e_fcmovbe, t_done, 0, true, { ST0, Ef, Zz }, 0, s1RW2R },
        { e_fcmovu, t_done, 0, true,  { ST0, Ef, Zz }, 0, s1RW2R },
        { e_No_Entry,  t_done, 0, true, { Zz, Zz, Zz }, 0, sNONE },
        { e_fucompp,  t_done, 0, true, { ST0, ST1, Zz }, 0, s1RW2RW }, // double pop
        { e_No_Entry,  t_done, 0, true, { Zz, Zz, Zz }, 0, sNONE },
        { e_No_Entry,  t_done, 0, true, { Zz, Zz, Zz }, 0, sNONE },
    },
},
{
    { // DB 
      { e_fild,   t_done, 0, true, { ST0, Ev, Zz }, 0, s1W2R },
      { e_fisttp, t_done, 0, true, { Ev, ST0, Zz }, 0, s1W2R }, //stack pop
      { e_fist,   t_done, 0, true, { Ev, ST0, Zz }, 0, s1W2R },
      { e_fistp,  t_done, 0, true, { Ev, ST0, Zz }, 0, s1W2R }, // stack pop
      { e_No_Entry,  t_done, 0, true, { Zz, Zz, Zz }, 0, sNONE },
      { e_fld,    t_done, 0, true, { ST0, Ef, Zz }, 0, s1W2R },
      { e_No_Entry,  t_done, 0, true, { Zz, Zz, Zz }, 0, sNONE },
      { e_fstp,   t_done, 0, true, { Ef, ST0, Zz }, 0, s1W2R }
    },
    { // DB
        { e_fcmovnb,  t_done, 0, true, { ST0, Ef, Zz }, 0, s1RW2R },
        { e_fcmovne,  t_done, 0, true, { ST0, Ef, Zz }, 0, s1RW2R },
        { e_fcmovnbe, t_done, 0, true, { ST0, Ef, Zz }, 0, s1RW2R },
        { e_fcmovnu,  t_done, 0, true, { ST0, Ef, Zz }, 0, s1RW2R },
        { e_fp_generic,  t_done, 0, true, { Zz, Zz, Zz }, 0, sNONE }, // FIXME: needs FCLEX and FINIT in group
        { e_fucomi,  t_done, 0, true, { ST0, Ef, Zz }, 0, s1RW2R },
        { e_fcomi,  t_done, 0, true, { ST0, Ef, Zz }, 0, s1RW2R },
        { e_No_Entry,  t_done, 0, true, { Zz, Zz, Zz }, 0, sNONE },
    },
},
{
    { // DC
        { e_fadd,  t_done, 0, true, { ST0, Efd, Zz }, 0, s1RW2R },
        { e_fmul,  t_done, 0, true, { ST0, Efd, Zz }, 0, s1RW2R },
        { e_fcom,  t_done, 0, true, { ST0, Efd, Zz }, 0, s1RW2R },
        { e_fcomp, t_done, 0, true, { ST0, Efd, Zz }, 0, s1RW2R }, // stack pop
        { e_fsub,  t_done, 0, true, { ST0, Efd, Zz }, 0, s1RW2R },
        { e_fsubr, t_done, 0, true, { ST0, Efd, Zz }, 0, s1RW2R },
        { e_fdiv,  t_done, 0, true, { ST0, Efd, Zz }, 0, s1RW2R },
        { e_fdivr, t_done, 0, true, { ST0, Efd, Zz }, 0, s1RW2R }
    },
    { // DC
        { e_fadd,  t_done, 0, true, { ST0, Efd, Zz }, 0, s1RW2R },
        { e_fmul,  t_done, 0, true, { ST0, Efd, Zz }, 0, s1RW2R },
        { e_No_Entry,  t_done, 0, true, { Zz, Zz, Zz }, 0, sNONE },
        { e_No_Entry,  t_done, 0, true, { Zz, Zz, Zz }, 0, sNONE },
        { e_fsubr,  t_done, 0, true, { ST0, Efd, Zz }, 0, s1RW2R },
        { e_fsub,  t_done, 0, true, { ST0, Efd, Zz }, 0, s1RW2R },
        { e_fdivr,  t_done, 0, true, { ST0, Efd, Zz }, 0, s1RW2R },
        { e_fdiv,  t_done, 0, true, { ST0, Efd, Zz }, 0, s1RW2R },
    },
},
{
    { // DD TODO semantics check
        { e_fld,    t_done, 0, true, { ST0, Efd, Zz }, 0, s1W2R },
        { e_fisttp, t_done, 0, true, { Mq, ST0, Zz }, 0, s1W2R },
        { e_fst,    t_done, 0, true, { Efd, ST0, Zz }, 0, s1W2R },
        { e_fstp,   t_done, 0, true, { Efd, ST0, Zz }, 0, s1W2R }, // stack pop
        { e_frstor, t_done, 0, true, { M512, Zz, Zz }, 0, s1R },
        { e_fucomp, t_done, 0, true, { ST0, Efd, Zz }, 0, s1R2R }, // stack pop
        { e_fsave,  t_done, 0, true, { M512, Zz, Zz }, 0, s1W },
        { e_fstsw,  t_done, 0, true, { Ew, Zz, Zz }, 0, s1W }
    },
    { // DD TODO semantics check
        { e_ffree,    t_done, 0, true, { Efd, Zz, Zz }, 0, s1W },
        { e_No_Entry,  t_done, 0, true, { Zz, Zz, Zz }, 0, sNONE },
        { e_fst, t_done, 0, true, { Efd, ST0, Zz }, 0, s1W2R },
        { e_fstp, t_done, 0, true, { Efd, ST0, Zz }, 0, s1W2RW },
        { e_fucom,    t_done, 0, true, { ST0, Efd, Zz }, 0, s1R2R },
        { e_fucomp,    t_done, 0, true, { ST0, Efd, Zz }, 0, s1RW2R },
        { e_No_Entry,  t_done, 0, true, { Zz, Zz, Zz }, 0, sNONE },
        { e_No_Entry,  t_done, 0, true, { Zz, Zz, Zz }, 0, sNONE },
    },
},
{    
    { // DE 
        { e_fiadd,  t_done, 0, true, { ST0, Ev, Zz }, 0, s1RW2R },
        { e_fimul,  t_done, 0, true, { ST0, Ev, Zz }, 0, s1RW2R },
        { e_ficom,  t_done, 0, true, { ST0, Ev, Zz }, 0, s1RW2R },
        { e_ficomp, t_done, 0, true, { ST0, Ev, Zz }, 0, s1RW2R }, // stack pop
        { e_fisub,  t_done, 0, true, { ST0, Ev, Zz }, 0, s1RW2R },
        { e_fisubr, t_done, 0, true, { ST0, Ev, Zz }, 0, s1RW2R },
        { e_fidiv,  t_done, 0, true, { ST0, Ev, Zz }, 0, s1RW2R },
        { e_fidivr, t_done, 0, true, { ST0, Ev, Zz }, 0, s1RW2R }
    },
    { // DE
        { e_faddp,  t_done, 0, true, { ST0, Ef, Zz }, 0, s1RW2R },
        { e_fmulp,  t_done, 0, true, { ST0, Ef, Zz }, 0, s1RW2R },
        { e_No_Entry,  t_done, 0, true, { Zz, Zz, Zz }, 0, sNONE },
        { e_fcompp, t_done, 0, true, { ST0, ST1, Zz }, 0, s1RW2R },
        { e_fsubrp,  t_done, 0, true, { ST0, Ef, Zz }, 0, s1RW2R },
        { e_fsubp,  t_done, 0, true, { ST0, Ef, Zz }, 0, s1RW2R },
        { e_fdivrp, t_done, 0, true, { ST0, Ef, Zz }, 0, s1RW2R }, // stack pop
        { e_fdivp, t_done, 0, true, { ST0, Ef, Zz }, 0, s1RW2R }
    },
},
{
    { // DF TODO semantics/operand sizes
        { e_fild,   t_done, 0, true, { ST0, Ew, Zz }, 0, s1W2R },
        { e_fisttp, t_done, 0, true, { Ew, ST0, Zz }, 0, s1W2R },
        { e_fist,   t_done, 0, true, { Ew, ST0, Zz }, 0, s1W2R },
        { e_fistp,  t_done, 0, true, { Ew, ST0, Zz }, 0, s1W2R }, // stack pop
        { e_fbld,   t_done, 0, true, { ST0, Mq, Zz }, 0, s1W2R }, // BCD 80 bit
        { e_fild,   t_done, 0, true, { ST0, Ev, Zz }, 0, s1W2R },
        { e_fbstp,  t_done, 0, true, { Mq, ST0, Zz }, 0, s1RW2R },// BCD 80 bit
        { e_fistp,  t_done, 0, true, { Ev, ST0, Zz }, 0, s1W2R }
    },
    { // DF TODO semantics/operand sizes
        { e_No_Entry,  t_done, 0, true, { Zz, Zz, Zz }, 0, sNONE },
        { e_No_Entry,  t_done, 0, true, { Zz, Zz, Zz }, 0, sNONE },
        { e_No_Entry,  t_done, 0, true, { Zz, Zz, Zz }, 0, sNONE },
        { e_No_Entry,  t_done, 0, true, { Zz, Zz, Zz }, 0, sNONE },
        { e_fstsw,   t_done, 0, true, { AX, Zz, Zz }, 0, s1W },
        { e_fucomip,  t_done, 0, true, { ST0, Ef, Zz }, 0, s1RW2R }, // stack pop
        { e_fcomip,  t_done, 0, true, { ST0, Ef, Zz }, 0, s1RW2R }, // stack pop
        { e_No_Entry,  t_done, 0, true, { Zz, Zz, Zz }, 0, sNONE },
    }
}
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
  { e_test, t_done, 0, true, { Eb, Ib, Zz }, 0, s1R2R }, // book swears this is illegal, sandpile claims it's an aliased TEST
  { e_not,  t_done, 0, true, { Eb, Zz, Zz }, 0, s1RW },
  { e_neg,  t_done, 0, true, { Eb, Zz, Zz }, 0, s1RW },
  { e_mul,  t_done, 0, true, { AX, AL, Eb }, 0, s1W2RW3R },
  { e_imul, t_done, 0, true, { AX, AL, Eb }, 0, s1W2R3R },
  { e_div,  t_done, 0, true, { AX, AL, Eb }, 0, s1RW2R3R },
  { e_idiv, t_done, 0, true, { AX, AL, Eb }, 0, s1W2R3R }
 },

 { /* group 3b - operands are defined here */
  { e_test, t_done, 0, true, { Ev, Iz, Zz }, 0, s1R2R },
  { e_test, t_done, 0, true, { Ev, Iz, Zz }, 0, s1R2R }, // book swears this is illegal, sandpile claims it's an aliased TEST
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
  { e_push, t_done, 0, true, { Ev, eSP, Zz }, 0, s1R2RW },
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
  { /* group 17 */
    {
      { e_extrq, t_done, 0, true, { Vdq, Ib, Ib }, 0, s1RW2R3R },
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
    { e_movsd_sse,  t_done, 0, true, { Vsd, Wsd, Zz }, 0, s1W2R },
  },
  { /* SSE11 */
    { e_movups, t_done, 0, true, { Wps, Vps, Zz }, 0, s1W2R },
    { e_movss,  t_done, 0, true, { Wss, Vss, Zz }, 0, s1W2R },
    { e_movupd, t_done, 0, true, { Wpd, Vpd, Zz }, 0, s1W2R },
    { e_movsd_sse,  t_done, 0, true, { Wsd, Vsd, Zz }, 0, s1W2R }, // Book is wrong, this is a W/V
  },
  { /* SSE12 */
    { e_movlps_movhlps, t_done, 0, true, { Wq, Vq, Zz }, 0, s1W2R }, // FIXME: wierd 1st op
    { e_movsldup, t_done, 0, true, { Vdq, Wdq, Zz }, 0, s1W2R },
    { e_movlpd, t_done, 0, true, { Vq, Ws, Zz }, 0, s1W2R },
    { e_movddup, t_done, 0, true, { Vdq, Wq, Zz }, 0, s1W2R },
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
    { e_movshdup, t_done, 0, true, { Vdq, Wdq, Zz }, 0, s1W2R },
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
    { e_movntss, t_done, 0, true, { Md, Vd, Zz }, 0, s1W2R | (fNT << FPOS) },
    { e_movntpd, t_done, 0, true, { Wpd, Vpd, Zz }, 0, s1W2R | (fNT << FPOS) }, // bug in book
    { e_movntsd, t_done, 0, true, { Wq, Vq, Zz }, 0, s1W2R | (fNT << FPOS) },
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
    { e_rsqrtss, t_done, 0, true, { Vss, Wss, Zz }, 0, s1W2R },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
  },
  { /* SSE53 */
    { e_rcpps, t_done, 0, true, { Vps, Wps, Zz }, 0, s1W2R },
    { e_rcpss, t_done, 0, true, { Vss, Wss, Zz }, 0, s1W2R },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
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
    { e_movdqu, t_done, 0, true, { Vdq, Wdq, Zz }, 0, s1W2R }, // book has this/next swapped!!!
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
  { /* SSE78 */
    { e_vmread, t_done, 0, true, { Ed, Gd, Zz }, 0, 0 },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
    { e_No_Entry, t_grp, Grp17, false, { Zz, Zz, Zz }, 0, 0 },
    { e_insertq, t_done, 0, true, {Vdq, VRq, Iw}, 0, s1RW2R3R}, // This is actually 2 8-bit immediates, treat as 1 16-bit for decode
  },
  { /* SSE79 */
    { e_vmwrite, t_done, 0, true, { Ed, Gd, Zz }, 0, 0 },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
    { e_extrq, t_done, 0, true, {Vdq, VRq, Zz}, 0, s1RW2R},
    { e_insertq, t_done, 0, true, {Vdq, VRdq, Zz}, 0, s1RW2R},
  },
  { /* SSE7C */
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
    { e_haddpd, t_done, 0, true, { Vpd, Wpd, Zz }, 0, s1RW2R },
    { e_haddps, t_done, 0, true, { Vps, Wps, Zz }, 0, s1RW2R },
  },
  { /* SSE7D */
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
    { e_hsubpd, t_done, 0, true, { Vpd, Wpd, Zz }, 0, s1RW2R },
    { e_hsubps, t_done, 0, true, { Vps, Wps, Zz }, 0, s1RW2R },
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
  { /* SSEB8 */
    { e_jmpe, t_done, 0, false, { Jz, Zz, Zz }, 0, s1R },
    { e_popcnt, t_done, 0, true, { Gv, Ev, Zz }, 0, s1W2R },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
  },
  { /* SSEC2 */
    { e_cmpps, t_done, 0, true, { Vps, Wps, Ib }, 0, s1RW2R3R }, // comparison writes to dest!
    { e_cmpss, t_done, 0, true, { Vss, Wss, Ib }, 0, s1RW2R3R },
    { e_cmppd, t_done, 0, true, { Vpd, Wpd, Ib }, 0, s1RW2R3R },
    { e_cmpsd_sse, t_done, 0, true, { Vsd, Wsd, Ib }, 0, s1RW2R3R },
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
  { /* SSED0 */
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
    { e_addsubpd, t_done, 0, true, { Vdq, Wdq, Zz }, 0, s1RW2R },
    { e_addsubps, t_done, 0, true, { Vdq, Wdq, Zz }, 0, s1RW2R },
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
  { /* SSEF0 */
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
    { e_lddqu, t_done, 0, true, { Vdq, Mdq, Zz }, 0, s1W2R },
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
  },
  { /* SSEFF */
    { e_ud, t_done, 0, false, { Zz, Zz, Zz }, 0, 0 },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
    { e_ud, t_done, 0, false, { Zz, Zz, Zz }, 0, 0 },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
  }
};

/* rows are not, F3, 66, F2, 66&F2 prefixed in this order (see book) */
static ia32_entry sseMapBis[][5] = {
		{ /* SSEB00 */
				{ e_pshufb, t_done, 0, true, { Pq, Qq, Zz }, 0, s1RW2R },
				{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
				{ e_pshufb, t_done, 0, true, { Vdq, Wdq, Zz }, 0, s1RW2R },
				{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
				{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 }
		},
		{ /* SSEB01 */
				{ e_phaddw, t_done, 0, true, { Pq, Qq, Zz }, 0, s1RW2R },
				{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
				{ e_phaddw, t_done, 0, true, { Vdq, Wdq, Zz }, 0, s1RW2R },
				{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
				{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 }
		},
		{ /* SSEB02 */
				{ e_phaddd, t_done, 0, true, { Pq, Qq, Zz }, 0, s1RW2R },
				{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
				{ e_phaddd, t_done, 0, true, { Vdq, Wdq, Zz }, 0, s1RW2R },
				{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
				{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 }
		},
		{ /* SSEB03 */
				{ e_phaddsw, t_done, 0, true, { Pq, Qq, Zz }, 0, s1RW2R },
				{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
				{ e_phaddsw, t_done, 0, true, { Vdq, Wdq, Zz }, 0, s1RW2R },
				{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
				{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 }
		},
		{ /* SSEB04 */
				{ e_pmaddubsw, t_done, 0, true, { Pq, Qq, Zz }, 0, s1RW2R },
				{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
				{ e_pmaddubsw, t_done, 0, true, { Vdq, Wdq, Zz }, 0, s1RW2R },
				{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
				{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 }
		},
		{ /* SSEB05 */
				{ e_phsubw, t_done, 0, true, { Pq, Qq, Zz }, 0, s1RW2R },
				{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
				{ e_phsubw, t_done, 0, true, { Vdq, Wdq, Zz }, 0, s1RW2R },
				{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
				{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 }
		},
		{ /* SSEB02 */
				{ e_phsubd, t_done, 0, true, { Pq, Qq, Zz }, 0, s1RW2R },
				{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
				{ e_phsubd, t_done, 0, true, { Vdq, Wdq, Zz }, 0, s1RW2R },
				{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
				{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 }
		},
		{ /* SSEB07 */
				{ e_phsubsw, t_done, 0, true, { Pq, Qq, Zz }, 0, s1RW2R },
				{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
				{ e_phsubsw, t_done, 0, true, { Vdq, Wdq, Zz }, 0, s1RW2R },
				{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
				{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 }
		},
		{ /* SSEB08 */
				{ e_psignb, t_done, 0, true, { Pq, Qq, Zz }, 0, s1RW2R },
				{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
				{ e_psignb, t_done, 0, true, { Vdq, Wdq, Zz }, 0, s1RW2R },
				{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
				{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 }
		},
		{ /* SSEB09 */
				{ e_psignw, t_done, 0, true, { Pq, Qq, Zz }, 0, s1RW2R },
				{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
				{ e_psignw, t_done, 0, true, { Vdq, Wdq, Zz }, 0, s1RW2R },
				{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
				{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 }
		},
		{ /* SSEB0A */
				{ e_psignd, t_done, 0, true, { Pq, Qq, Zz }, 0, s1RW2R },
				{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
				{ e_psignd, t_done, 0, true, { Vdq, Wdq, Zz }, 0, s1RW2R },
				{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
				{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 }
		},
		{ /* SSEB0B */
				{ e_pmulhrsw, t_done, 0, true, { Pq, Qq, Zz }, 0, s1RW2R },
				{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
				{ e_pmulhrsw, t_done, 0, true, { Vdq, Wdq, Zz }, 0, s1RW2R },
				{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
				{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 }
		},
		{ /* SSEB10 */
				{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
				{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
				{ e_pblendvb, t_done, 0, true, { Vdq, Wdq, Zz }, 0, s1RW2R3R },
				{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
				{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 }
		},
		{ /* SSEB14 */
				{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
				{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
				{ e_blendvps, t_done, 0, true, { Vdq, Wdq, Zz }, 0, s1RW2R3R },
				{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
				{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 }
		},
		{ /* SSEB15 */
				{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
				{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
				{ e_blendvpd, t_done, 0, true, { Vdq, Wdq, Zz }, 0, s1RW2R3R },
				{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
				{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 }
		},
		{ /* SSEB17 */
				{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
				{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
				{ e_ptest, t_done, 0, true, { Vdq, Wdq, Zz }, 0, s1R2R },
				{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
				{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 }
		},
		{ /* SSEB1C */
				{ e_pabsb, t_done, 0, true, { Pq, Qq, Zz }, 0, s1W2R },
				{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
				{ e_pabsb, t_done, 0, true, { Vdq, Wdq, Zz }, 0, s1W2R },
				{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
				{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 }
		},
		{ /* SSEB1D */
				{ e_pabsw, t_done, 0, true, { Pq, Qq, Zz }, 0, s1W2R },
				{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
				{ e_pabsw, t_done, 0, true, { Vdq, Wdq, Zz }, 0, s1W2R },
				{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
				{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 }
		},
		{ /* SSEB1E */
				{ e_pabsd, t_done, 0, true, { Pq, Qq, Zz }, 0, s1W2R },
				{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
				{ e_pabsd, t_done, 0, true, { Vdq, Wdq, Zz }, 0, s1W2R },
				{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
				{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 }
		},
		{ /* SSEB20 */
				{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
				{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
				{ e_pmovsxbw, t_done, 0, true, { Vdq, Wdq, Zz }, 0, s1W2R },
				{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
				{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 }
		},
		{ /* SSEB21 */
				{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
				{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
				{ e_pmovsxbd, t_done, 0, true, { Vdq, Wdq, Zz }, 0, s1W2R },
				{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
				{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 }
		},
		{ /* SSEB22 */
				{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
				{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
				{ e_pmovsxbq, t_done, 0, true, { Vdq, Wdq, Zz }, 0, s1W2R },
				{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
				{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 }
		},
		{ /* SSEB23 */
				{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
				{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
				{ e_pmovsxwd, t_done, 0, true, { Vdq, Wdq, Zz }, 0, s1W2R },
				{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
				{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 }
		},
		{ /* SSEB24 */
				{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
				{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
				{ e_pmovsxwq, t_done, 0, true, { Vdq, Wdq, Zz }, 0, s1W2R },
				{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
				{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 }
		},
		{ /* SSEB25 */
				{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
				{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
				{ e_pmovsxdq, t_done, 0, true, { Vdq, Wdq, Zz }, 0, s1W2R },
				{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
				{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 }
		},
		{ /* SSEB28 */
				{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
				{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
				{ e_pmuldq, t_done, 0, true, { Vdq, Wdq, Zz }, 0, s1RW2R },
				{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
				{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 }
		},
		{ /* SSEB29 */
				{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
				{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
				{ e_pcmpeqq, t_done, 0, true, { Vdq, Wdq, Zz }, 0, s1RW2R },
				{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
				{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 }
		},
		{ /* SSEB2A */
				{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
				{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
				{ e_movntdqa, t_done, 0, true, { Mdq, Wdq, Zz }, 0, s1W2R },
				{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
				{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 }
		},
		{ /* SSEB2B */
				{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
				{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
				{ e_packusdw, t_done, 0, true, { Vdq, Wdq, Zz }, 0, s1RW2R },
				{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
				{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 }
		},
		{ /* SSEB30 */
				{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
				{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
				{ e_pmovzxbw, t_done, 0, true, { Vdq, Wdq, Zz }, 0, s1W2R },
				{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
				{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 }
		},
		{ /* SSEB31 */
				{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
				{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
				{ e_pmovzxbd, t_done, 0, true, { Vdq, Wdq, Zz }, 0, s1W2R },
				{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
				{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 }
		},
		{ /* SSEB32 */
				{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
				{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
				{ e_pmovzxbq, t_done, 0, true, { Vdq, Wdq, Zz }, 0, s1W2R },
				{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
				{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 }
		},
		{ /* SSEB33 */
				{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
				{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
				{ e_pmovzxwd, t_done, 0, true, { Vdq, Wdq, Zz }, 0, s1W2R },
				{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
				{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 }
		},
		{ /* SSEB34 */
				{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
				{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
				{ e_pmovzxwq, t_done, 0, true, { Vdq, Wdq, Zz }, 0, s1W2R },
				{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
				{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 }
		},
		{ /* SSEB35 */
				{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
				{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
				{ e_pmovzxdq, t_done, 0, true, { Vdq, Wdq, Zz }, 0, s1W2R },
				{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
				{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 }
		},
		{ /* SSEB37 */
				{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
				{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
				{ e_pcmpgtq, t_done, 0, true, { Vdq, Wdq, Zz }, 0, s1RW2R },
				{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
				{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 }
		},
		{ /* SSEB38 */
				{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
				{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
				{ e_pminsb, t_done, 0, true, { Vdq, Wdq, Zz }, 0, s1RW2R },
				{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
				{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 }
		},
		{ /* SSEB39 */
				{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
				{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
				{ e_pminsd, t_done, 0, true, { Vdq, Wdq, Zz }, 0, s1RW2R },
				{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
				{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 }
		},
		{ /* SSEB3A */
				{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
				{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
				{ e_pminuw, t_done, 0, true, { Vdq, Wdq, Zz }, 0, s1RW2R },
				{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
				{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 }
		},
		{ /* SSEB3B */
				{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
				{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
				{ e_pminud, t_done, 0, true, { Vdq, Wdq, Zz }, 0, s1RW2R },
				{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
				{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 }
		},
		{ /* SSEB3C */
				{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
				{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
				{ e_pmaxsb, t_done, 0, true, { Vdq, Wdq, Zz }, 0, s1RW2R },
				{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
				{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 }
		},
		{ /* SSEB3D */
				{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
				{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
				{ e_pmaxsd, t_done, 0, true, { Vdq, Wdq, Zz }, 0, s1RW2R },
				{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
				{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 }
		},
		{ /* SSEB3E */
				{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
				{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
				{ e_pmaxuw, t_done, 0, true, { Vdq, Wdq, Zz }, 0, s1RW2R },
				{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
				{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 }
		},
		{ /* SSEB3F */
				{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
				{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
				{ e_pmaxud, t_done, 0, true, { Vdq, Wdq, Zz }, 0, s1RW2R },
				{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
				{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 }
		},
		{ /* SSEB40 */
				{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
				{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
				{ e_pmulld, t_done, 0, true, { Vdq, Wdq, Zz }, 0, s1RW2R },
				{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
				{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 }
		},
		{ /* SSEB41 */
				{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
				{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
				{ e_phminposuw, t_done, 0, true, { Vdq, Wdq, Zz }, 0, s1W2R },
				{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
				{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 }
		},
		{ /* SSEBF0 */
				{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
				{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
				{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
				{ e_crc32, t_done, 0, true, { Gv, Eb, Zz }, 0, s1RW2R },
				{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 }
		},
		{ /* SSEBF1 */
				{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
				{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
				{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
				{ e_crc32, t_done, 0, true, { Gv, Ev, Zz }, 0, s1RW2R },
				{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 }
		}
		
};

/* rows are not, 66, F2 prefixed in this order (see book) */
static ia32_entry sseMapTer[][3] = {
		{ /* SSET08 */
				{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
				{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
				{ e_roundps, t_done, 0, true, { Vdq, Wdq, Ib }, 0, s1W2R3R },
		},
		{ /* SSET09 */
				{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
				{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
				{ e_roundpd, t_done, 0, true, { Vdq, Wdq, Ib }, 0, s1W2R3R },
		},
		{ /* SSET0A */
				{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
				{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
				{ e_roundss, t_done, 0, true, { Vss, Wss, Ib }, 0, s1W2R3R },
		},
		{ /* SSET0B */
				{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
				{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
				{ e_roundsd, t_done, 0, true, { Vsd, Wsd, Ib }, 0, s1W2R3R },
		},
		{ /* SSET0C */
				{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
				{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
				{ e_blendps, t_done, 0, true, { Vdq, Wdq, Ib }, 0, s1RW2R3R },
		},
		{ /* SSET0D */
				{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
				{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
				{ e_blendps, t_done, 0, true, { Vdq, Wdq, Ib }, 0, s1RW2R3R },
		},
		{ /* SSET0E */
				{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
				{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
				{ e_pblendw, t_done, 0, true, { Vdq, Wdq, Ib }, 0, s1RW2R3R },
		},		
		{ /* SSET0F */
				{ e_palignr, t_done, 0, true, { Pq, Qq, Ib }, 0, s1RW2R3R },
				{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
				{ e_palignr, t_done, 0, true, { Vdq, Wdq, Ib }, 0, s1RW2R3R },
		},
		{ /* SSET14 */
				{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
				{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
				{ e_pextrb, t_done, 0, true, { RMb, Vdq, Ib }, 0, s1W2R3R }, 
		},
		{ /* SSET15 */
				{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
				{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
				{ e_pextrw, t_done, 0, true, { RMw, Vdq, Ib }, 0, s1W2R3R }, 
		},
		{ /* SSET16 */
				{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
				{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
				{ e_pextrd_pextrq, t_done, 0, true, { Ey, Vdq, Ib }, 0, s1W2R3R },
		},
		{ /* SSET17 */
				{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
				{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
				{ e_extractps, t_done, 0, true, { Ed, Vdq, Ib }, 0, s1W2R3R }, 
		},
		{ /* SSET20 */
				{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
				{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
				{ e_pinsrb, t_done, 0, true, { Vdq, RMb, Ib }, 0, s1W2R3R }, 
		},
		{ /* SSET21 */
				{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
				{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
				{ e_insertps, t_done, 0, true, { Vdq, UMd, Ib }, 0, s1W2R3R },
		},
		{ /* SSET22 */
				{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
				{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
				{ e_pinsrd_pinsrq, t_done, 0, true, { Vdq, Ey, Ib }, 0, s1W2R3R },
		},
		{ /* SSET40 */
				{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
				{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
				{ e_dpps, t_done, 0, true, { Vdq, Wdq, Ib }, 0, s1RW2R3R }, 
		},
		{ /* SSET41 */
				{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
				{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
				{ e_dppd, t_done, 0, true, { Vdq, Wdq, Ib }, 0, s1RW2R3R }, 
		},
		{ /* SSET42 */
				{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
				{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
				{ e_mpsadbw, t_done, 0, true, { Vdq, Wdq, Ib }, 0, s1RW2R3R }, 
		},
		{ /* SSET60 */
				{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
				{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
				{ e_pcmpestrm, t_done, 0, true, { Vdq, Wdq, Ib }, 0, s1R2R3R }, 
		},
		{ /* SSET61 */
				{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
				{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
				{ e_pcmpestri, t_done, 0, true, { Vdq, Wdq, Ib }, 0, s1R2R3R }, 
		},
		{ /* SSET62 */
				{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
				{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
				{ e_pcmpistrm, t_done, 0, true, { Vdq, Wdq, Ib }, 0, s1R2R3R }, 
		},
		{ /* SSET63 */
				{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
				{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 },
				{ e_pcmpistri, t_done, 0, true, { Vdq, Wdq, Ib }, 0, s1R2R3R }, 
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
ia32_entry invalid = { e_No_Entry, t_ill, 0, true, { Zz, Zz, Zz }, 0, 0 };
		       
static void ia32_translate_for_64(ia32_entry** gotit_ptr)
{
    if (*gotit_ptr == &oneByteMap[0x63]) // APRL redefined to MOVSXD
	*gotit_ptr = &movsxd;
    if (*gotit_ptr == &oneByteMap[0x06] || // Invalid instructions in 64-bit mode: push es
	*gotit_ptr == &oneByteMap[0x07] || // pop es
	*gotit_ptr == &oneByteMap[0x0E] || // push cs
	*gotit_ptr == &oneByteMap[0x16] || // push ss
	*gotit_ptr == &oneByteMap[0x17] || // pop ss
	*gotit_ptr == &oneByteMap[0x1E] || // push ds
	*gotit_ptr == &oneByteMap[0x1F] || // pop ds
	*gotit_ptr == &oneByteMap[0x27] || // daa
	*gotit_ptr == &oneByteMap[0x2F] || // das
	*gotit_ptr == &oneByteMap[0x37] || // aaa
	*gotit_ptr == &oneByteMap[0x3F] || // aas
	*gotit_ptr == &oneByteMap[0x60] || // pusha
	*gotit_ptr == &oneByteMap[0x61] || // popa
	*gotit_ptr == &oneByteMap[0x62] || // bound gv, ma
	*gotit_ptr == &oneByteMap[0x82] || // group 1 eb/ib
	*gotit_ptr == &oneByteMap[0x9A] || // call ap
	*gotit_ptr == &oneByteMap[0xC4] || // les gz, mp
	*gotit_ptr == &oneByteMap[0xC5] || // lds gz, mp
	*gotit_ptr == &oneByteMap[0xCE] || // into
	*gotit_ptr == &oneByteMap[0xD4] || // aam ib
	*gotit_ptr == &oneByteMap[0xD5] || // aad ib
	*gotit_ptr == &oneByteMap[0xD6] || // salc
	*gotit_ptr == &oneByteMap[0xEA]) { // jump ap
      *gotit_ptr = &invalid;
    }
    
}

/* full decoding version: supports memory access information */
static unsigned int ia32_decode_modrm(const unsigned int addrSzAttr,
                                      const unsigned char* addr,
                                      ia32_memacc* macadr,
                                      const ia32_prefixes* pref,
                                      ia32_locations *pos);


void ia32_memacc::print()
{
    fprintf(stderr, "base: %d, index: %d, scale:%d, disp: %ld (%lx), size: %d, addr_size: %d\n",
	    regs[0], regs[1], scale, imm, imm, size, addr_size);
}

int getOperSz(const ia32_prefixes &pref) 
{
   if (pref.rexW()) return 4;
   else if (pref.getPrefix(2) == PREFIX_SZOPER) return 1;
   else return 2;
}

ia32_instruction& ia32_decode(unsigned int capa, const unsigned char* addr, ia32_instruction& instruct)
{
  ia32_prefixes& pref = instruct.prf;
  unsigned int table, nxtab;
  unsigned int idx = 0, sseidx = 0;
  ia32_entry *gotit = NULL;
  int condbits = 0;

  if(capa & IA32_DECODE_MEMACCESS)
    assert(instruct.mac != NULL);

  if (!ia32_decode_prefixes(addr, pref, instruct.loc)) {
    instruct.size = 1;
    instruct.legacy_type = ILLEGAL;
    return instruct;
  }

  if((pref.getOpcodePrefix()) && pref.getCount())
  {
    idx = pref.getOpcodePrefix();
  }

  if (instruct.loc) instruct.loc->num_prefixes = pref.getCount();
  instruct.size = pref.getCount();
  addr += instruct.size;

  table = t_oneB;
  if(idx == 0) {
    // consume opcode
    idx = addr[0];
    instruct.size += 1;
    addr += 1;
  } else {
    // opcode already consumed (prefix opcode)
  }

  gotit = &oneByteMap[idx];
  nxtab = gotit->otable;

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
    case t_threeB:
      idx = addr[0];
      gotit = &threeByteMap[idx];
      nxtab = gotit->otable;
      instruct.size += 1;
      addr += 1;
      if(capa & IA32_DECODE_CONDITION)
    	condbits = idx & 0x0F;
      break;
    case t_threeB2:
      idx = addr[0];
      gotit = &threeByteMap2[idx];
      nxtab = gotit->otable;
      instruct.size += 1;
      addr += 1;
      if(capa & IA32_DECODE_CONDITION)
    	condbits = idx & 0x0F;
      break;
    case t_prefixedSSE:
      sseidx = gotit->tabidx;
      if(addr[0] != 0x0F)
      {
          // all valid SSE insns will have 0x0F as their first byte after prefix
          instruct.size += 1;
          addr += 1;
          instruct.entry = &invalid;
          return instruct;
      }
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
    case t_sse_bis:
      idx = gotit->tabidx;
      gotit = &sseMapBis[idx][sseidx];
      nxtab = gotit->otable;
      break;
    case t_sse_ter:
      idx = gotit->tabidx;      
      gotit = &sseMapTer[idx][sseidx];
      nxtab = gotit->otable;
      break;
    case t_grp: {
      idx = gotit->tabidx;
      unsigned int reg  = (addr[0] >> 3) & 7;
      if(idx < Grp12)
        switch(idx) {
        case Grp2:
        case Grp11:
          /* leave table unchanged because operands are in not 
             defined in group map, unless this is an invalid index
             into the group, in which case we need the instruction
             to reflect its illegal status */
          if(groupMap[idx][reg].id == e_No_Entry)
            gotit = &groupMap[idx][reg];
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
      {
        instruct.legacy_type = 0;
        unsigned int reg  = (addr[0] >> 3) & 7;
        unsigned int mod = addr[0] >> 6;
        gotit = &fpuMap[gotit->tabidx][mod==3][reg];
        ia32_decode_FP(idx, pref, addr, instruct, gotit, instruct.mac);
        return instruct;
      }
    case t_3dnow:
      // 3D now opcodes are given as suffix: ModRM [SIB] [displacement] opcode
      // Right now we don't care what the actual opcode is, so there's no table
      instruct.size += 1;
      nxtab = t_done;
      break;
    case t_ill:
      instruct.legacy_type = ILLEGAL;
      instruct.entry = gotit;
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
	  break;
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
	break;
      }
      break;
    case 0:
    case PREFIX_LOCK:
      break;
    default:
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

  // flip id of opcodes overloaded on operand size prefix
  int operSzAttr = getOperSz(pref);
  if (1 == operSzAttr) {
    entryID newID = gotit->id;
    switch (gotit->id) {
    case e_cwde: newID = e_cbw; break;
    case e_cdq: newID = e_cwd; break;
    case e_insd: newID = e_insw; break;
    case e_lodsd: newID = e_lodsw; break;
    case e_movsd: newID = e_movsw; break;
    case e_outsd: newID = e_outsw; break;
    case e_popad: newID = e_popa; break;
    case e_popfd: newID = e_popf; break;
    case e_pushad: newID = e_pusha; break;
    case e_pushfd: newID = e_pushf; break;
    case e_scasd: newID = e_scasw; break;
    case e_stosd: newID = e_stosw; break;
    default: break;
    }
    gotit->id = newID;
  }

  instruct.entry = gotit;
  return instruct;
}

ia32_instruction& ia32_decode_FP(unsigned int opcode, const ia32_prefixes& pref,
                                 const unsigned char* addr, ia32_instruction& instruct,
                                 ia32_entry * entry, ia32_memacc *mac)
{
  // addr points after the opcode, and the size has been adjusted accordingly
  if (instruct.loc) instruct.loc->opcode_size = instruct.size - pref.getCount();
  if (instruct.loc) instruct.loc->opcode_position = pref.getCount();

  unsigned int nib = byteSzB; // modRM
  unsigned int addrSzAttr = (pref.getPrefix(3) == PREFIX_SZADDR ? 1 : 2); // 32-bit mode implicit
  unsigned int operSzAttr = (pref.getPrefix(2) == PREFIX_SZOPER ? 1 : 2); // 32-bit mode implicit

  // There *will* be a mod r/m byte at least as far as locs are concerned, though the mod=3 case does not
  // consume extra bytes.  So pull this out of the conditional.
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
  if (addr[0] <= 0xBF) { // modrm
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
         if (loc) { 
            loc->disp_position = loc->modrm_position + 1;
            loc->disp_size = 1;
         }
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
        if (loc) { 
           loc->disp_position = loc->modrm_position + 1;
           loc->disp_size = 4;
        }
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
  case op_y:
	  return operSzAttr == 4 ? qwordSzB : dwordSzB;
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
      assert(0);
      return 0;
//    RegisterAST reg(optype);
//    return reg.eval().size();
  }
}

unsigned int ia32_decode_operands (const ia32_prefixes& pref, 
                                   const ia32_entry& gotit, 
                                   const unsigned char* addr, 
                                   ia32_instruction& instruct,
                                   ia32_memacc *mac)
{
  ia32_locations *loc = instruct.loc;
  if (loc) loc->imm_cnt = 0;
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
		nib += wordSzB; // segment
        nib += wordSzB * addrSzAttr;  // + offset (1 or 2 words, depending on prefix)
	break;
      case am_O: /* operand offset */
        nib += wordSzB * addrSzAttr;
        if(mac) {
          int offset = 0;
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
      case am_RM:/* register or memory location, so decoding needed */
      case am_UM:/* XMM register or memory location */
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
            nib += ia32_decode_modrm(addrSzAttr, addr, NULL, &pref, loc);
         
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
            // sanity
            if(loc->imm_cnt > 1) {
                fprintf(stderr,"Oops, more than two immediate operands\n");
            } else {
                loc->imm_position[loc->imm_cnt] = 
                    nib + loc->opcode_position + loc->opcode_size;
                loc->imm_size[loc->imm_cnt] = imm_size;
                ++loc->imm_cnt;
            }
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
      case am_stackP: /* stack pop */
	assert(0 && "Wrong table!");
        break;
      case am_tworeghack:
      case am_ImplImm:
	// Don't do nuthin'
	break;
      default:
        assert(0 && "Bad addressing mode!");
      }
    }
    else
      break;
  }
  if((gotit.id == e_push) && mac)
  {
    // assuming 32-bit (64-bit for AMD64) stack segment
    // AMD64: push defaults to 64-bit operand size
    if (mode_64 && operSzAttr == 2)
      operSzAttr = 4;
    mac[1].set(mESP, -2 * operSzAttr, addrSzAttr);
    if(gotit.operands[0].admet == am_reg)
    {
        mac[1].size = type2size(op_v, operSzAttr);
    }
    else
    {
        mac[1].size = type2size(gotit.operands[0].optype, operSzAttr);
    }
    mac[1].write = true;
  }
  if((gotit.id == e_pop) && mac)
  {
    // assuming 32-bit (64-bit for AMD64) stack segment
    // AMD64: pop defaults to 64-bit operand size
    if (mode_64 && operSzAttr == 2)
      operSzAttr = 4;
    mac[1].set(mESP, 0, addrSzAttr);
    if(gotit.operands[0].admet == am_reg)
    {
        mac[1].size = type2size(op_v, operSzAttr);
    }
    else
    {
        mac[1].size = type2size(gotit.operands[0].optype, operSzAttr);
    }
    mac[1].read = true;
  }
  if((gotit.id == e_leave) && mac)
  {
    // assuming 32-bit (64-bit for AMD64) stack segment
    // AMD64: push defaults to 64-bit operand size
    if (mode_64 && operSzAttr == 2)
      operSzAttr = 4;
    mac[0].set(mESP, 0, addrSzAttr);
    mac[0].size = type2size(op_v, operSzAttr);
    mac[0].read = true;
  }
  if((gotit.id == e_ret_near || gotit.id == e_ret_far) && mac)
  {
    mac[0].set(mESP, 0, addrSzAttr);
    mac[0].size = type2size(op_v, addrSzAttr);
    mac[0].read = true;
  }
  if((gotit.id == e_call) && mac)
  {
	int index = 0;
	while((mac[index].regs[0] != -1) ||
		  (mac[index].regs[1] != -1) ||
		  (mac[index].scale != 0) ||
		  (mac[index].imm != 0)) {
		index++;
		assert(index < 3);
	}
	mac[index].set(mESP, -2 * addrSzAttr, addrSzAttr);
      mac[index].size = type2size(op_v, addrSzAttr);
      mac[index].write = true;
            
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
  /* 7x */ 1,1,1,1,1,1,1,0,1,1,0,0,1,1,1,1, // Grp12-14 are SSE groups
  /* 8x */ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  /* 9x */ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  /* Ax */ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  /* Bx */ 0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,
  /* Cx */ 0,0,1,0,1,1,1,0,0,0,0,0,0,0,0,0,
  /* Dx */ 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
  /* Ex */ 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
  /* Fx */ 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1
};

static const unsigned char sse_prefix_bis[256] = {
  /*       0 1 2 3 4 5 6 7 8 9 A B C D E F  */
  /* 0x */ 1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,
  /* 1x */ 1,0,0,0,1,1,0,1,0,0,0,0,1,1,1,0,
  /* 2x */ 1,1,1,1,1,1,0,0,1,1,1,1,0,0,0,0,
  /* 3x */ 1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,
  /* 4x */ 1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  /* 5x */ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  /* 6x */ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  /* 7x */ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  /* 8x */ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  /* 9x */ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  /* Ax */ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  /* Bx */ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  /* Cx */ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  /* Dx */ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  /* Ex */ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  /* Fx */ 1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
};

static const unsigned char sse_prefix_ter[256] = {
  /*       0 1 2 3 4 5 6 7 8 9 A B C D E F  */
  /* 0x */ 0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,
  /* 1x */ 0,0,0,0,1,1,1,1,0,0,0,0,0,0,0,0,
  /* 2x */ 1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,
  /* 3x */ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  /* 4x */ 1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,
  /* 5x */ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  /* 6x */ 1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,
  /* 7x */ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  /* 8x */ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  /* 9x */ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  /* Ax */ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  /* Bx */ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  /* Cx */ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  /* Dx */ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  /* Ex */ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  /* Fx */ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
};

#define REX_ISREX(x) (((x) >> 4) == 4)

bool is_sse_opcode(unsigned char byte1, unsigned char byte2, unsigned char byte3) {
	if((byte1==0x0F && sse_prefix[byte2])
			|| (byte1==0x0F && byte2==0x38 && sse_prefix_bis[byte3])
			|| (byte1==0x0F && byte2==0x3A && sse_prefix_ter[byte3]))
		return true;
	
	return false;
}

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
       if(mode_64 && REX_ISREX(addr[1]) && is_sse_opcode(addr[2],addr[3],addr[4])) {
          ++pref.count;
          pref.opcode_prefix = addr[0];
          break;
       }
       else if(is_sse_opcode(addr[1],addr[2],addr[3])) {
          ++pref.count;
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
       if(is_sse_opcode(addr[1],addr[2],addr[3])) {
          pref.opcode_prefix = addr[0];
          break;
       }
       if(mode_64 && REX_ISREX(addr[1]) && is_sse_opcode(addr[2],addr[3],addr[4])) {
          ++pref.count;
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
      {
         return false;
      }
   }

   return true;
}

unsigned int ia32_emulate_old_type(ia32_instruction& instruct)
{
  const ia32_prefixes& pref = instruct.prf;
  unsigned int& insnType = instruct.legacy_type;

  unsigned int operSzAttr = getOperSz(pref);

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

// find the target of a jump or call
Address get_target(const unsigned char *instr, unsigned type, unsigned size,
		   Address addr) {
#if defined(os_vxworks)
    Address ret;
    // FIXME requires vxworks in Dyninst
    if (relocationTarget(addr+1, &ret))
        return ret;
#endif
  int disp = displacement(instr, type);
  return (Address)(addr + size + disp);
}

// get the displacement of a jump or call
int displacement(const unsigned char *instr, unsigned type) {

  int disp = 0;
  //skip prefix
  instr = skip_headers( instr );

  if (type & REL_D_DATA) {
    // Some SIMD instructions have a mandatory 0xf2 prefix; the call to skip_headers
    // doesn't skip it and I don't feel confident in changing that - bernat, 22MAY06
      // 0xf3 as well...
      // Some 3-byte opcodes start with 0x66... skip
      if (*instr == 0x66) instr++;
      if (*instr == 0xf2) instr++;
      else if (*instr == 0xf3) instr++;
    // And the "0x0f is a 2-byte opcode" skip
    if (*instr == 0x0F) {
        instr++;
        // And the "0x38 or 0x3A is a 3-byte opcode" skip
        if(*instr == 0x38 || *instr == 0x3A)  instr++;
    }
    // Skip the instr opcode and the MOD/RM byte
    disp = *(const int *)(instr+2);
  } else if (type & IS_JUMP) {
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
  unsigned nPrefixes = count_prefixes(insnType);

  for (unsigned u = 0; u < nPrefixes; u++)
     *newInsn++ = *origInsn++;
  return nPrefixes;
}

//Copy all prefixes but the Operand-Size and Address-Size prefixes (0x66 and 0x67)
unsigned copy_prefixes_nosize(const unsigned char *&origInsn, unsigned char *&newInsn, 
                              unsigned insnType) 
{
  unsigned nPrefixes = count_prefixes(insnType);

  for (unsigned u = 0; u < nPrefixes; u++) {
     if (*origInsn == 0x66 || *origInsn == 0x67)
     {
        origInsn++;
        continue;
     }
     *newInsn++ = *origInsn++;
  }
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

bool convert_to_rel32(const unsigned char*&origInsn, unsigned char *&newInsn) {
  if (*origInsn == 0x0f)
    origInsn++;
  *newInsn++ = 0x0f;

  // We think that an opcode in the 0x7? range can be mapped to an equivalent
  // opcode in the 0x8? range...

  if ((*origInsn >= 0x70) &&
      (*origInsn < 0x80)) {
    *newInsn++ = *origInsn++ + 0x10;
    return true;
  }

  if ((*origInsn >= 0x80) &&
      (*origInsn < 0x90)) {
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

const char* ia32_entry::name(ia32_locations* loc)
{
  dyn_hash_map<entryID, string>::const_iterator found = entryNames_IAPI.find(id);
  if(found != entryNames_IAPI.end())
  {
    return found->second.c_str();
  }
  if(loc)
  {
    if(otable == t_grp && (tabidx == Grp2 || tabidx == Grp11))
    {
      int reg = loc->modrm_reg;
      found = entryNames_IAPI.find(groupMap[tabidx][reg].id);
      if(found != entryNames_IAPI.end())
      {
	return found->second.c_str();
      }
    }
  }
  return NULL;
}

entryID ia32_entry::getID(ia32_locations* l) const
{
  if((id != e_No_Entry) || (tabidx == t_done) || (l == NULL))
  {
    return id;
  }
  int reg = l->modrm_reg;
  switch(otable)
  {
  case t_grp:
    switch(tabidx)
    {
    case Grp2:
    case Grp11:
      return groupMap[tabidx][reg].id;
      break;
    default:
      break;
    }
  case t_coprocEsc:
      return e_fp_generic;
      case t_3dnow:
          return e_3dnow_generic;
      default:
    break;
  }
  return id;
}

Address get_immediate_operand(instruction *instr)
{
    ia32_memacc mac[3];
    ia32_condition cond;
    ia32_locations loc;
    ia32_instruction detail(mac,&cond,&loc);

    ia32_decode(IA32_FULL_DECODER,(const unsigned char *)(instr->ptr()),detail);

    if (loc.imm_cnt < 1)
      return 0;

    // now find the immediate value in the locations
    Address immediate = 0;

    switch(loc.imm_size[0]) {
        case 8:
            immediate = *(const unsigned long*)(instr->ptr()+loc.imm_position[0]);
            break;
        case 4:
            immediate = *(const unsigned int*)(instr->ptr()+loc.imm_position[0]);
            break;
        case 2:
            immediate = *(const unsigned short*)(instr->ptr()+loc.imm_position[0]);
            break;
        case 1:
            immediate = *(const unsigned char*)(instr->ptr()+loc.imm_position[0]);
            break;
        default:
//            fprintf(stderr,"%s[%u]:  invalid immediate size %d in insn\n",
//                FILE__,__LINE__,loc.imm_size[0]);
            break;
    }

    return immediate;
}

// get the displacement of a relative jump or call

int get_disp(instruction *insn) {
  return displacement(insn->ptr(), insn->type());
}

bool isStackFramePrecheck_gcc( const unsigned char *buffer )
{
   //Currently enabled entry bytes for gaps:
   //  0x55 - push %ebp
   static char gap_initial_bytes[] =
      { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
   return (gap_initial_bytes[*buffer] != 0);
}  

bool isStackFramePrecheck_msvs( const unsigned char *buffer )
{
   //Currently enabled entry bytes for gaps:
   //  0x55 - push %ebp
   static char gap_initial_bytes[] =
      { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
   return (gap_initial_bytes[*buffer] != 0);
}  

/*
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
*/

instruction *instruction::copy() const {
    // Or should we copy? I guess it depends on who allocated
    // the memory...
    return new instruction(*this);
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
    const int longJumpSize = JUMP_ABS32_SZ;
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
      unsigned size;
#if defined(arch_x86_64)
      size = 2*JUMP_ABS64_SZ+count_prefixes(type());
#else
      size = JUMP_SZ+count_prefixes(type());
#endif
      size = (size > CALL_RELOC_THUNK) ? size : CALL_RELOC_THUNK;
      return size;
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

#if defined(arch_x86_64)
unsigned instruction::jumpSize(long disp, unsigned addr_width) 
{
   if (addr_width == 8 && !is_disp32(disp))
      return JUMP_ABS64_SZ;
   return JUMP_SZ;
}
#else
unsigned instruction::jumpSize(long /*disp*/, unsigned /*addr_width*/) 
{
   return JUMP_SZ;
}
#endif

unsigned instruction::jumpSize(Address from, Address to, unsigned addr_width) 
{
    long disp = to - (from + JUMP_SZ);
    return jumpSize(disp, addr_width);
}

#if defined(arch_x86_64)
unsigned instruction::maxJumpSize(unsigned addr_width) 
{
   if (addr_width == 8)
      return JUMP_ABS64_SZ;
   return JUMP_SZ;
}
#else
unsigned instruction::maxJumpSize(unsigned /*addr_width*/) 
{
   return JUMP_SZ;
}
#endif

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

bool instruction::getUsedRegs(pdvector<int> &regs) {
   const unsigned char *insn_ptr = ptr();

   struct ia32_memacc memacc[3];
   struct ia32_condition cond;
   class ia32_locations loc;
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
         if (loc.address_size == 4 && loc.rex_r) {
             regused |= 0x8;
         }
         regs.push_back(regused);
      }
      else if (op.admet == am_reg) {
	//using namespace Dyninst::InstructionAPI;
         //The instruction implicitely references a memory instruction
         switch (op.optype) {
             case x86::iah:   
             case x86::ial:
             case x86::iax:   
             case x86::ieax:
               regs.push_back(REGNUM_RAX);
               if (loc.rex_byte) regs.push_back(REGNUM_R8);
               break;
             case x86::ibh:
             case x86::ibl:
             case x86::ibx:
             case x86::iebx:
               regs.push_back(REGNUM_RBX);
               if (loc.rex_byte) regs.push_back(REGNUM_R11);
               break;
             case x86::ich:
             case x86::icl:
             case x86::icx:
             case x86::iecx:
                 regs.push_back(REGNUM_RCX);
               if (loc.rex_byte) regs.push_back(REGNUM_R9);
               break;
             case x86::idh:
             case x86::idl:
             case x86::idx:
             case x86::iedx:
                 regs.push_back(REGNUM_RDX);
               if (loc.rex_byte) regs.push_back(REGNUM_R10);
               break;
             case x86::isp:
             case x86::iesp:
                regs.push_back(REGNUM_RSP);
               if (loc.rex_byte) regs.push_back(REGNUM_R12);
               break;
             case x86::ibp:
             case x86::iebp:
               regs.push_back(REGNUM_RBP);
               if (loc.rex_byte) regs.push_back(REGNUM_R13);
               break;
             case x86::isi:
             case x86::iesi:
               regs.push_back(REGNUM_RSI);
               if (loc.rex_byte) regs.push_back(REGNUM_R14);
               break;
             case x86::idi:
             case x86::iedi:
               regs.push_back(REGNUM_RDI);
               if (loc.rex_byte) regs.push_back(REGNUM_R15);
               break;
            case op_edxeax:
               regs.push_back(REGNUM_RAX);
               regs.push_back(REGNUM_RDX);
               break;
            case op_ecxebx:
               regs.push_back(REGNUM_RBX);
               regs.push_back(REGNUM_RCX);
               break;
         }
      }
   }
   return true;
}

int instruction::getStackDelta()
{
   ia32_instruction instruc;
   const unsigned char *p = ptr();
   ia32_decode(0, ptr(), instruc);

   if (instruc.getEntry()->id == e_push)
      return -4;
   if (instruc.getEntry()->id == e_pop)
      return 4;
   if (instruc.getEntry()->id == e_pusha)
      return (-2 * 9);
   if (instruc.getEntry()->id == e_pushad)
      return (-4 * 9);
   if (instruc.getEntry()->id == e_popa)
      return (2 * 9);
   if (instruc.getEntry()->id == e_popad)
      return (4 * 9);

   if (p[0] == 0x83 && p[1] == 0xec) {
      //Subtract byte
      return -1 * (signed char) p[2];
   }

   if (p[0] == 0x83 && p[1] == 0xc4) {
      //Add byte
      return (signed char) p[2];
   }
   
   return 0;
}

bool instruction::isNop() const
{ 

   if (!(type_ & IS_NOP)) //NOP or LEA
      return false;

   if (*op_ptr_ == NOP) {
      return true;
   }

   ia32_memacc mac[3];
   ia32_condition cnd;
   ia32_locations loc;

   ia32_instruction instruc(mac, &cnd, &loc);

   ia32_decode(IA32_FULL_DECODER, ptr(), instruc);


   if (instruc.getEntry()->id == e_nop) {
      return true;
   }

   if (loc.modrm_mod == 3) {
      return false;
   }
   if (loc.modrm_mod == 0 && loc.modrm_rm == 5) {
      return false;
   }

   if (loc.rex_x) {
      return false;
   }
   if ((!loc.rex_r) != (!loc.rex_b)) { // Logical exclusive or
      return false;
   }

   if (loc.disp_position != -1) {
      for (unsigned i=0; i<loc.disp_size; i++) {
         if (ptr_[i + loc.disp_position] != 0) {
            return false;
         }
      }
   }
   
   if (loc.modrm_rm == 4) {
      unsigned scale;
      Register index, base;
      decode_SIB(loc.sib_byte, scale, index, base);
      
      if (index != 4) {
         return false;
      }

      if (base != loc.modrm_reg) {
         return false;
      }
   }
   else if (loc.modrm_reg != loc.modrm_rm) {
      return false;
   }


   return true;
}

} // namespace arch_x86
