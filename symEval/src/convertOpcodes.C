/*
 * Copyright (c) 1996-2007 Barton P. Miller
 * 
 * We provide the Paradyn Parallel Performance Tools (below
 * described as "Paradyn") on an AS IS basis, and do not warrant its
 * validity or performance.  We reserve the right to update, modify,
 * or discontinue this software at any time.  We shall have no
 * obligation to supply such updates or modifications or any other
 * form of support to you.
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

// This file was automatically generated

#include "SymEval.h"
#include "SymEvalPolicy.h"

#include "AST.h"

#include "../rose/x86InstructionSemantics.h"
#include "external/rose/powerpcInstructionEnum.h"

using namespace Dyninst;
using namespace Dyninst::InstructionAPI;
using namespace Dyninst::SymbolicEvaluation;

template class SymEval<Arch_x86>;
template class SymEval<Arch_ppc32>;

    
SymEvalArchTraits<Arch_x86>::InstructionKind_t SymEvalArchTraits<Arch_x86>::convert(entryID opcode)
{
    switch (opcode) {
        case e_jb:
            return x86_jb;
        case e_jb_jnaej_j:
            return x86_jb;
        case e_jbe:
            return x86_jbe;
        case e_jcxz_jec:
            return x86_jcxz;
        case e_jl:
            return x86_jl;
        case e_jle:
            return x86_jle;
        case e_jmp:
            return x86_jmp;
        case e_jmpe:
            return x86_jmpe;
        case e_jnb:
            return x86_jae;
        case e_jnb_jae_j:
            return x86_jae;
        case e_jnbe:
            return x86_ja;
        case e_jnl:
            return x86_jge;
        case e_jnle:
            return x86_jg;
        case e_jno:
            return x86_jno;
        case e_jnp:
            return x86_jpo;
        case e_jns:
            return x86_jns;
        case e_jnz:
            return x86_jne;
        case e_jo:
            return x86_jo;
        case e_jp:
            return x86_jpe;
        case e_js:
            return x86_js;
        case e_jz:
            return x86_je;
        case e_loop:
            return x86_loop;
        case e_loope:
            return x86_loopz;
        case e_loopn:
            return x86_loopnz;
        case e_call:
            return x86_call;
        case e_cmp:
            return x86_cmp;
        case e_cmppd:
            return x86_cmppd;
        case e_cmpps:
            return x86_cmpps;
        case e_cmpsb:
            return x86_cmpsb;
        case e_cmpsd:
            return x86_cmpsd;
        case e_cmpss:
            return x86_cmpss;
        case e_cmpsw:
            return x86_cmpsw;
        case e_cmpxch:
            return x86_cmpxchg;
        case e_cmpxch8b:
            return x86_cmpxchg8b;
        case e_ret_far:
            return x86_retf;
        case e_ret_near:
            return x86_ret;
        case e_prefetch:
            return x86_prefetch;
        case e_prefetchNTA:
            return x86_prefetchnta;
        case e_prefetchT0:
            return x86_prefetcht0;
        case e_prefetchT1:
            return x86_prefetcht1;
        case e_prefetchT2:
            return x86_prefetcht2;
        case e_prefetch_w:
            return x86_prefetchw;
        case e_prefetchw:
            return x86_prefetchw;
        case e_No_Entry:
            return x86_unknown_instruction;
        case e_aaa:
            return x86_aaa;
        case e_aad:
            return x86_aad;
        case e_aam:
            return x86_aam;
        case e_aas:
            return x86_aas;
        case e_adc:
            return x86_adc;
        case e_add:
            return x86_add;
        case e_addpd:
            return x86_addpd;
        case e_addps:
            return x86_addps;
        case e_addsd:
            return x86_addsd;
        case e_addss:
            return x86_addss;
        case e_addsubpd:
            return x86_addsubpd;
        case e_addsubps:
            return x86_addsubps;
        case e_and:
            return x86_and;
        case e_andnpd:
            return x86_andnpd;
        case e_andnps:
            return x86_andnps;
        case e_andpd:
            return x86_andpd;
        case e_andps:
            return x86_andps;
        case e_arpl:
            return x86_arpl;
        case e_bound:
            return x86_bound;
        case e_bsf:
            return x86_bsf;
        case e_bsr:
            return x86_bsr;
        case e_bswap:
            return x86_bswap;
        case e_bt:
            return x86_bt;
        case e_btc:
            return x86_btc;
        case e_btr:
            return x86_btr;
        case e_bts:
            return x86_bts;
        case e_cbw_cwde:
            return x86_cbw;
        case e_clc:
            return x86_clc;
        case e_cld:
            return x86_cld;
        case e_clflush:
            return x86_clflush;
        case e_cli:
            return x86_cli;
        case e_clts:
            return x86_clts;
        case e_cmc:
            return x86_cmc;
        case e_cmovbe:
            return x86_cmovbe;
        case e_cmove:
            return x86_cmove;
        case e_cmovnae:
            return x86_cmovb;
        case e_cmovnb:
            return x86_cmovae;
        case e_cmovnbe:
            return x86_cmova;
        case e_cmovne:
            return x86_cmovne;
        case e_cmovng:
            return x86_cmovle;
        case e_cmovnge:
            return x86_cmovl;
        case e_cmovnl:
            return x86_cmovge;
        case e_cmovno:
            return x86_cmovno;
        case e_cmovns:
            return x86_cmovns;
        case e_cmovo:
            return x86_cmovo;
        case e_cmovpe:
            return x86_cmovpe;
        case e_cmovpo:
            return x86_cmovpo;
        case e_cmovs:
            return x86_cmovs;
        case e_comisd:
            return x86_comisd;
        case e_comiss:
            return x86_comiss;
        case e_cpuid:
            return x86_cpuid;
        case e_cvtdq2pd:
            return x86_cvtdq2pd;
        case e_cvtdq2ps:
            return x86_cvtdq2ps;
        case e_cvtpd2dq:
            return x86_cvtpd2dq;
        case e_cvtpd2pi:
            return x86_cvtpd2pi;
        case e_cvtpd2ps:
            return x86_cvtpd2ps;
        case e_cvtpi2pd:
            return x86_cvtpi2pd;
        case e_cvtpi2ps:
            return x86_cvtpi2ps;
        case e_cvtps2dq:
            return x86_cvtps2dq;
        case e_cvtps2pd:
            return x86_cvtps2pd;
        case e_cvtps2pi:
            return x86_cvtps2pi;
        case e_cvtsd2si:
            return x86_cvtsd2si;
        case e_cvtsd2ss:
            return x86_cvtsd2ss;
        case e_cvtsi2sd:
            return x86_cvtsi2sd;
        case e_cvtsi2ss:
            return x86_cvtsi2ss;
        case e_cvtss2sd:
            return x86_cvtss2sd;
        case e_cvtss2si:
            return x86_cvtss2si;
        case e_cvttpd2dq:
            return x86_cvttpd2dq;
        case e_cvttpd2pi:
            return x86_cvttpd2pi;
        case e_cvttps2dq:
            return x86_cvttps2dq;
        case e_cvttps2pi:
            return x86_cvttps2pi;
        case e_cvttsd2si:
            return x86_cvttsd2si;
        case e_cvttss2si:
            return x86_cvttss2si;
        case e_cwd_cdq:
            return x86_cwd;
        case e_daa:
            return x86_daa;
        case e_das:
            return x86_das;
        case e_dec:
            return x86_dec;
        case e_div:
            return x86_div;
        case e_divpd:
            return x86_divpd;
        case e_divps:
            return x86_divps;
        case e_divsd:
            return x86_divsd;
        case e_divss:
            return x86_divss;
        case e_emms:
            return x86_emms;
        case e_enter:
            return x86_enter;
        case e_extrq:
            return x86_extrq;
        case e_fadd:
            return x86_fadd;
        case e_fbld:
            return x86_fbld;
        case e_fbstp:
            return x86_fbstp;
        case e_fcom:
            return x86_fcom;
        case e_fcomp:
            return x86_fcomp;
        case e_fdiv:
            return x86_fdiv;
        case e_fdivr:
            return x86_fdivr;
        case e_femms:
            return x86_femms;
        case e_fiadd:
            return x86_fiadd;
        case e_ficom:
            return x86_ficom;
        case e_ficomp:
            return x86_ficomp;
        case e_fidiv:
            return x86_fidiv;
        case e_fidivr:
            return x86_fidivr;
        case e_fild:
            return x86_fild;
        case e_fimul:
            return x86_fimul;
        case e_fist:
            return x86_fist;
        case e_fistp:
            return x86_fistp;
        case e_fisttp:
            return x86_fisttp;
        case e_fisub:
            return x86_fisub;
        case e_fisubr:
            return x86_fisubr;
        case e_fld:
            return x86_fld;
        case e_fldcw:
            return x86_fldcw;
        case e_fldenv:
            return x86_fldenv;
        case e_fmul:
            return x86_fmul;
        case e_fnop:
            return x86_fnop;
        case e_frstor:
            return x86_frstor;
        case e_fsave:
            return x86_fnsave;
        case e_fst:
            return x86_fst;
        case e_fstcw:
            return x86_fnstcw;
        case e_fstenv:
            return x86_fnstenv;
        case e_fstp:
            return x86_fstp;
        case e_fstsw:
            return x86_fnstsw;
        case e_fsub:
            return x86_fsub;
        case e_fsubr:
            return x86_fsubr;
        case e_fucomp:
            return x86_fucomp;
        case e_fxrstor:
            return x86_fxrstor;
        case e_fxsave:
            return x86_fxsave;
        case e_haddpd:
            return x86_haddpd;
        case e_haddps:
            return x86_haddps;
        case e_hlt:
            return x86_hlt;
        case e_hsubpd:
            return x86_hsubpd;
        case e_hsubps:
            return x86_hsubps;
        case e_idiv:
            return x86_idiv;
        case e_imul:
            return x86_imul;
        case e_in:
            return x86_in;
        case e_inc:
            return x86_inc;
        case e_insb:
            return x86_insb;
        case e_insertq:
            return x86_insertq;
        case e_insw_d:
            return x86_insw;
        case e_int:
            return x86_int;
        case e_int3:
            return x86_int3;
        case e_int1:
            return x86_int1;
        case e_into:
            return x86_into;
        case e_invd:
            return x86_invd;
        case e_invlpg:
            return x86_invlpg;
        case e_iret:
            return x86_iret;
        case e_lahf:
            return x86_lahf;
        case e_lar:
            return x86_lar;
        case e_lddqu:
            return x86_lddqu;
        case e_ldmxcsr:
            return x86_ldmxcsr;
        case e_lds:
            return x86_lds;
        case e_lea:
            return x86_lea;
        case e_leave:
            return x86_leave;
        case e_les:
            return x86_les;
        case e_lfence:
            return x86_lfence;
        case e_lfs:
            return x86_lfs;
        case e_lgdt:
            return x86_lgdt;
        case e_lgs:
            return x86_lgs;
        case e_lidt:
            return x86_lidt;
        case e_lldt:
            return x86_lldt;
        case e_lmsw:
            return x86_lmsw;
        case e_lodsb:
            return x86_lodsb;
        case e_lodsw:
            return x86_lodsw;
        case e_lsl:
            return x86_lsl;
        case e_lss:
            return x86_lss;
        case e_ltr:
            return x86_ltr;
        case e_maskmovdqu:
            return x86_maskmovq;
        case e_maskmovq:
            return x86_maskmovq;
        case e_maxpd:
            return x86_maxpd;
        case e_maxps:
            return x86_maxps;
        case e_maxsd:
            return x86_maxsd;
        case e_maxss:
            return x86_maxss;
        case e_mfence:
            return x86_mfence;
        case e_minpd:
            return x86_minpd;
        case e_minps:
            return x86_minps;
        case e_minsd:
            return x86_minsd;
        case e_minss:
            return x86_minss;
        case e_mmxud:
            return x86_unknown_instruction;
        case e_mov:
            return x86_mov;
        case e_movapd:
            return x86_movapd;
        case e_movaps:
            return x86_movaps;
        case e_movd:
            return x86_movd;
        case e_movddup:
            return x86_movddup;
        case e_movdq2q:
            return x86_movdq2q;
        case e_movdqa:
            return x86_movdqa;
        case e_movdqu:
            return x86_movdqu;
        case e_movhpd:
            return x86_movhpd;
        case e_movhps:
            return x86_movhps;
        case e_movhps_movlhps:
            return x86_movhps;
        case e_movlpd:
            return x86_movlpd;
        case e_movlps:
            return x86_movlps;
        case e_movlps_movhlps:
            return x86_movlps;
        case e_movmskpd:
            return x86_movmskpd;
        case e_movmskps:
            return x86_movmskps;
        case e_movntdq:
            return x86_movntdq;
        case e_movnti:
            return x86_movnti;
        case e_movntpd:
            return x86_movntpd;
        case e_movntps:
            return x86_movntps;
        case e_movntq:
            return x86_movntq;
        case e_movntsd:
            return x86_movntsd;
        case e_movntss:
            return x86_movntss;
        case e_movq:
            return x86_movq;
        case e_movq2dq:
            return x86_movq2dq;
        case e_movsb:
            return x86_movsb;
        case e_movsd:
            return x86_movsd;
        case e_movshdup:
            return x86_movshdup;
        case e_movsldup:
            return x86_movsldup;
        case e_movss:
            return x86_movss;
        case e_movsw_d:
            return x86_movsw;
        case e_movsx:
            return x86_movsx;
        case e_movsxd:
            return x86_movsxd;
        case e_movupd:
            return x86_movupd;
        case e_movups:
            return x86_movups;
        case e_movzx:
            return x86_movzx;
        case e_mul:
            return x86_mul;
        case e_mulpd:
            return x86_mulpd;
        case e_mulps:
            return x86_mulps;
        case e_mulsd:
            return x86_mulsd;
        case e_mulss:
            return x86_mulss;
        case e_neg:
            return x86_neg;
        case e_nop:
            return x86_nop;
        case e_not:
            return x86_not;
        case e_or:
            return x86_or;
        case e_orpd:
            return x86_orpd;
        case e_orps:
            return x86_orps;
        case e_out:
            return x86_out;
        case e_outsb:
            return x86_outsb;
        case e_outsw_d:
            return x86_outsw;
        case e_packssdw:
            return x86_packssdw;
        case e_packsswb:
            return x86_packsswb;
        case e_packuswb:
            return x86_packuswb;
        case e_paddb:
            return x86_paddb;
        case e_paddd:
            return x86_paddd;
        case e_paddq:
            return x86_paddq;
        case e_paddsb:
            return x86_paddsb;
        case e_paddsw:
            return x86_paddsw;
        case e_paddusb:
            return x86_paddusb;
        case e_paddusw:
            return x86_paddusw;
        case e_paddw:
            return x86_paddw;
        case e_pand:
            return x86_pand;
        case e_pandn:
            return x86_pandn;
        case e_pavgb:
            return x86_pavgb;
        case e_pavgw:
            return x86_pavgw;
        case e_pcmpeqb:
            return x86_pcmpeqb;
        case e_pcmpeqd:
            return x86_pcmpeqd;
        case e_pcmpeqw:
            return x86_pcmpeqw;
        case e_pcmpgdt:
            return x86_pcmpgtd;
        case e_pcmpgtb:
            return x86_pcmpgtb;
        case e_pcmpgtw:
            return x86_pcmpgtw;
        case e_pextrw:
            return x86_pextrw;
        case e_pinsrw:
            return x86_pinsrw;
        case e_pmaddwd:
            return x86_pmaddwd;
        case e_pmaxsw:
            return x86_pmaxsw;
        case e_pmaxub:
            return x86_pmaxub;
        case e_pminsw:
            return x86_pminsw;
        case e_pminub:
            return x86_pminub;
        case e_pmovmskb:
            return x86_pmovmskb;
        case e_pmulhuw:
            return x86_pmulhuw;
        case e_pmulhw:
            return x86_pmulhw;
        case e_pmullw:
            return x86_pmullw;
        case e_pmuludq:
            return x86_pmuludq;
        case e_pop:
            return x86_pop;
        case e_popa_d:
            return x86_popa;
        case e_popf_d:
            return x86_popf;
        case e_popcnt:
            return x86_popcnt;
        case e_por:
            return x86_por;
        case e_psadbw:
            return x86_psadbw;
        case e_pshufd:
            return x86_pshufd;
        case e_pshufhw:
            return x86_pshufhw;
        case e_pshuflw:
            return x86_pshuflw;
        case e_pshufw:
            return x86_pshufw;
        case e_pslld:
            return x86_pslld;
        case e_pslldq:
            return x86_pslldq;
        case e_psllq:
            return x86_psllq;
        case e_psllw:
            return x86_psllw;
        case e_psrad:
            return x86_psrad;
        case e_psraw:
            return x86_psraw;
        case e_psrld:
            return x86_psrld;
        case e_psrldq:
            return x86_psrldq;
        case e_psrlq:
            return x86_psrlq;
        case e_psrlw:
            return x86_psrlw;
        case e_psubb:
            return x86_psubb;
        case e_psubd:
            return x86_psubd;
        case e_psubsb:
            return x86_psubsb;
        case e_psubsw:
            return x86_psubsw;
        case e_psubusb:
            return x86_psubusb;
        case e_psubusw:
            return x86_psubusw;
        case e_psubw:
            return x86_psubw;
        case e_punpckhbw:
            return x86_punpckhbw;
        case e_punpckhdq:
            return x86_punpckhdq;
        case e_punpckhqd:
            return x86_punpckhdq;
        case e_punpckhwd:
            return x86_punpckhwd;
        case e_punpcklbw:
            return x86_punpcklbw;
        case e_punpcklqd:
            return x86_punpckldq;
        case e_punpcklqld:
            return x86_punpcklqdq;
        case e_punpcklwd:
            return x86_punpcklwd;
        case e_push:
            return x86_push;
        case e_pusha_d:
            return x86_pusha;
        case e_pushf_d:
            return x86_pushf;
        case e_pxor:
            return x86_pxor;
        case e_rcl:
            return x86_rcl;
        case e_rcpps:
            return x86_rcpps;
        case e_rcpss:
            return x86_rcpss;
        case e_rcr:
            return x86_rcr;
        case e_rdmsr:
            return x86_rdmsr;
        case e_rdpmc:
            return x86_rdpmc;
        case e_rdtsc:
            return x86_rdtsc;
        case e_rol:
            return x86_rol;
        case e_ror:
            return x86_ror;
        case e_rsm:
            return x86_rsm;
        case e_rsqrtps:
            return x86_rsqrtps;
        case e_rsqrtss:
            return x86_rsqrtss;
        case e_sahf:
            return x86_sahf;
        case e_salc:
            return x86_salc;
        case e_sar:
            return x86_sar;
        case e_sbb:
            return x86_sbb;
        case e_scasb:
            return x86_scasb;
        case e_scasw_d:
            return x86_scasw;
        case e_setb:
            return x86_setb;
        case e_setbe:
            return x86_setbe;
        case e_setl:
            return x86_setl;
        case e_setle:
            return x86_setle;
        case e_setnb:
            return x86_setae;
        case e_setnbe:
            return x86_seta;
        case e_setnl:
            return x86_setge;
        case e_setnle:
            return x86_setg;
        case e_setno:
            return x86_setno;
        case e_setnp:
            return x86_setpo;
        case e_setns:
            return x86_setns;
        case e_setnz:
            return x86_setne;
        case e_seto:
            return x86_seto;
        case e_setp:
            return x86_setpe;
        case e_sets:
            return x86_sets;
        case e_setz:
            return x86_sete;
        case e_sfence:
            return x86_sfence;
        case e_sgdt:
            return x86_sgdt;
        case e_shl_sal:
            return x86_shl;
        case e_shld:
            return x86_shld;
        case e_shr:
            return x86_shr;
        case e_shrd:
            return x86_shrd;
        case e_shufpd:
            return x86_shufpd;
        case e_shufps:
            return x86_shufps;
        case e_sidt:
            return x86_sidt;
        case e_sldt:
            return x86_sldt;
        case e_smsw:
            return x86_smsw;
        case e_sqrtpd:
            return x86_sqrtpd;
        case e_sqrtps:
            return x86_sqrtps;
        case e_sqrtsd:
            return x86_sqrtsd;
        case e_sqrtss:
            return x86_sqrtss;
        case e_stc:
            return x86_stc;
        case e_std:
            return x86_std;
        case e_sti:
            return x86_sti;
        case e_stmxcsr:
            return x86_stmxcsr;
        case e_stosb:
            return x86_stosb;
        case e_stosw_d:
            return x86_stosw;
        case e_str:
            return x86_str;
        case e_sub:
            return x86_sub;
        case e_subpd:
            return x86_subpd;
        case e_subps:
            return x86_subps;
        case e_subsd:
            return x86_subsd;
        case e_subss:
            return x86_subss;
        case e_syscall:
            return x86_syscall;
        case e_sysenter:
            return x86_sysenter;
        case e_sysexit:
            return x86_sysexit;
        case e_sysret:
            return x86_sysret;
        case e_test:
            return x86_test;
        case e_ucomisd:
            return x86_ucomisd;
        case e_ucomiss:
            return x86_ucomiss;
        case e_ud:
            return x86_unknown_instruction;
        case e_ud2:
            return x86_ud2;
        case e_ud2grp10:
            return x86_ud2;
        case e_unpckhpd:
            return x86_unpckhpd;
        case e_unpckhps:
            return x86_unpckhps;
        case e_unpcklpd:
            return x86_unpcklpd;
        case e_unpcklps:
            return x86_unpcklps;
        case e_verr:
            return x86_verr;
        case e_verw:
            return x86_verw;
        case e_vmread:
            return x86_vmread;
        case e_vmwrite:
            return x86_vmwrite;
        case e_wait:
            return x86_wait;
        case e_wbinvd:
            return x86_wbinvd;
        case e_wrmsr:
            return x86_wrmsr;
        case e_xadd:
            return x86_xadd;
        case e_xchg:
            return x86_xchg;
        case e_xlat:
            return x86_xlatb;
        case e_xor:
            return x86_xor;
        case e_xorpd:
            return x86_xorpd;
        case e_xorps:
            return x86_xorps;
        case e_fp_generic:
            return x86_unknown_instruction;
        case e_3dnow_generic:
            return x86_unknown_instruction;
        default:
            return x86_unknown_instruction;
        }
}

SymEvalArchTraits<Arch_ppc32>::InstructionKind_t SymEvalArchTraits<Arch_ppc32>::convert(entryID opcode)
{
    switch(opcode)
    {
        case power_op_stfdu: return powerpc_stfdu;
        case power_op_fadd: return powerpc_fadd;
        case power_op_xoris: return powerpc_xoris;
        case power_op_mulhwu: return powerpc_mulhwu;
        case power_op_stbux: return powerpc_stbux;
        case power_op_cmpl: return powerpc_cmpl;
        case power_op_subf: return powerpc_subf;
        case power_op_svcs: return powerpc_sc;
        case power_op_fmuls: return powerpc_fmuls;
        case power_op_subfic: return powerpc_subfic;
        case power_op_mcrfs: return powerpc_mcrfs;
        case power_op_divs: return powerpc_divw;
        case power_op_lwzx: return powerpc_lwzx;
        case power_op_fctiw: return powerpc_fctiw;
        case power_op_mtcrf: return powerpc_mtcrf;
        case power_op_srq: return powerpc_unknown_instruction;
        case power_op_sraw: return powerpc_sraw;
        case power_op_lfdx: return powerpc_lfdx;
        case power_op_stdcx_rc: return powerpc_stdcx_record;
        case power_op_nor: return powerpc_nor;
        case power_op_crandc: return powerpc_crandc;
        case power_op_stdu: return powerpc_stdu;
        case power_op_addme: return powerpc_addme;
        case power_op_fmul: return powerpc_fmul;
        case power_op_sthbrx: return powerpc_sthbrx;
        case power_op_mtspr: return powerpc_mtspr;
        case power_op_lfsx: return powerpc_lfsx;
        case power_op_lbzx: return powerpc_lbzx;
        case power_op_nand: return powerpc_nand;
        case power_op_fnmadds: return powerpc_fnmadds;
        case power_op_fnmadd: return powerpc_fnmadd;
        case power_op_mulhw: return powerpc_mulhw;
        case power_op_sradi: return powerpc_sradi;
        case power_op_fnmsubs: return powerpc_fnmsubs;
        case power_op_addze: return powerpc_addze;
        case power_op_mulld: return powerpc_mulld;
        case power_op_si: return powerpc_unknown_instruction; // there's not a subtract immediate available? huh?
        case power_op_lfs: return powerpc_lfs;
        case power_op_andc: return powerpc_andc;
        case power_op_eciwx: return powerpc_eciwx;
        case power_op_rfid: return powerpc_rfid;
        case power_op_divw: return powerpc_divw;
        case power_op_creqv: return powerpc_creqv;
        case power_op_fctiwz: return powerpc_fctiwz;
        case power_op_crnor: return powerpc_crnor;
        case power_op_lbzux: return powerpc_lbzux;
        case power_op_td: return powerpc_td;
        case power_op_dcbi: return powerpc_dcbi;
        case power_op_cli: return powerpc_unknown_instruction;
        case power_op_div: return powerpc_unknown_instruction;
        case power_op_add: return powerpc_add;
        case power_op_extsh: return powerpc_extsh;
        case power_op_divd: return powerpc_divd;
        case power_op_fmsub: return powerpc_fmsub;
        case power_op_stbx: return powerpc_stbx;
        case power_op_nabs: return powerpc_unknown_instruction;
        case power_op_isync: return powerpc_isync;
        case power_op_mfsri: return powerpc_unknown_instruction;
        case power_op_stfdx: return powerpc_stfdx;
        case power_op_fsqrt: return powerpc_fsqrt;
        case power_op_dcbz: return powerpc_dcbz;
        case power_op_dcbst: return powerpc_dcbst;
        case power_op_stswi: return powerpc_stswi;
        case power_op_mulli: return powerpc_mulli;
        case power_op_stfs: return powerpc_stfs;
        case power_op_clf: return powerpc_unknown_instruction;
        case power_op_fnmsub: return powerpc_fnmsub;
        case power_op_lhz: return powerpc_lhz;
        case power_op_ecowx: return powerpc_ecowx;
        case power_op_fres: return powerpc_fres;
        case power_op_stwu: return powerpc_stwu;
        case power_op_lhau: return powerpc_lhau;
        case power_op_slq: return powerpc_unknown_instruction;
        case power_op_srawi: return powerpc_srawi;
        case power_op_divwu: return powerpc_divwu;
        case power_op_addis: return powerpc_addis;
        case power_op_mfmsr: return powerpc_mfmsr;
        case power_op_mulhd: return powerpc_mulhd;
        case power_op_fdivs: return powerpc_fdivs;
        case power_op_abs: return powerpc_unknown_instruction;
        case power_op_lwzu: return powerpc_lwzu;
        case power_op_tlbli: return powerpc_unknown_instruction; // PPC 603 only
        case power_op_orc: return powerpc_orc;
        case power_op_mtfsf: return powerpc_mtfsf;
        case power_op_lswx: return powerpc_lswx;
        case power_op_stb: return powerpc_stb;
        case power_op_andis_rc: return powerpc_andis_record;
        case power_op_fsel: return powerpc_fsel;
        case power_op_xori: return powerpc_xori;
        case power_op_lwax: return powerpc_lwax;
        case power_op_tdi: return powerpc_tdi;
        case power_op_rlwimi: return powerpc_rlwimi;
        case power_op_stw: return powerpc_stw;
        case power_op_rldcr: return powerpc_rldcr;
        case power_op_sraq: return powerpc_unknown_instruction;
        case power_op_fmr: return powerpc_fmr;
        case power_op_tlbld: return powerpc_unknown_instruction; // PPC 603 only
        case power_op_doz: return powerpc_unknown_instruction;
        case power_op_lbz: return powerpc_lbz;
        case power_op_stdux: return powerpc_stdux;
        case power_op_mtfsfi: return powerpc_mtfsfi;
        case power_op_srea: return powerpc_unknown_instruction;
        case power_op_lscbx: return powerpc_unknown_instruction;
        case power_op_rlwinm: return powerpc_rlwinm;
        case power_op_sld: return powerpc_sld;
        case power_op_addc: return powerpc_addc;
        case power_op_lfqux: return powerpc_unknown_instruction; // POWER2 only; shouldn't this exist for double hummer?
        case power_op_sleq: return powerpc_unknown_instruction;
        case power_op_extsb: return powerpc_extsb;
        case power_op_ld: return powerpc_ld;
        case power_op_ldu: return powerpc_ldu;
        case power_op_fctidz: return powerpc_fctidz;
        case power_op_lfq: return powerpc_unknown_instruction; // again, POWER2 only
        case power_op_lwbrx: return powerpc_lwbrx;
        case power_op_fsqrts: return powerpc_fsqrts;
        case power_op_srd: return powerpc_srd;
        case power_op_lfdu: return powerpc_lfdu;
        case power_op_stfsux: return powerpc_stfsux;
        case power_op_lhzu: return powerpc_lhzu;
        case power_op_crnand: return powerpc_crnand;
        case power_op_icbi: return powerpc_icbi;
        case power_op_rlwnm: return powerpc_rlwnm;
        case power_op_rldcl: return powerpc_rldcl;
        case power_op_stwcx_rc: return powerpc_stwcx_record;
        case power_op_lhzx: return powerpc_lhzx;
        case power_op_stfsx: return powerpc_stfsx;
        case power_op_rlmi: return powerpc_unknown_instruction;
        case power_op_twi: return powerpc_twi;
        case power_op_srliq: return powerpc_unknown_instruction;
        case power_op_tlbie: return powerpc_tlbie;
        case power_op_mfcr: return powerpc_mfcr;
        case power_op_tlbsync: return powerpc_tlbsync;
        case power_op_extsw: return powerpc_extsw;
        case power_op_rldicl: return powerpc_rldicl;
        case power_op_bclr: return powerpc_bclr;
        case power_op_rfsvc: return powerpc_unknown_instruction;
        case power_op_mcrxr: return powerpc_mcrxr;
        case power_op_clcs: return powerpc_unknown_instruction;
        case power_op_srad: return powerpc_srad;
        case power_op_subfc: return powerpc_subfc;
        case power_op_mfsrin: return powerpc_mfsrin;
        case power_op_rfi: return powerpc_rfi;
        case power_op_sreq: return powerpc_unknown_instruction;
        case power_op_frsqrte: return powerpc_frsqrte;
        case power_op_mffs: return powerpc_mffs;
        case power_op_lwz: return powerpc_lwz;
        case power_op_lfqu: return powerpc_unknown_instruction; // power2
        case power_op_and: return powerpc_and;
        case power_op_stswx: return powerpc_stswx;
        case power_op_stfd: return powerpc_stfd;
        case power_op_fmsubs: return powerpc_fmsubs;
        case power_op_bcctr: return powerpc_bcctr;
        case power_op_lhaux: return powerpc_lhaux;
        case power_op_ldux: return powerpc_ldux;
        case power_op_fctid: return powerpc_fctid;
        case power_op_frsp: return powerpc_frsp;
        case power_op_slw: return powerpc_slw;
        case power_op_cmpli: return powerpc_cmpli;
        case power_op_sync: return powerpc_sync;
        case power_op_cntlzw: return powerpc_cntlzw;
        case power_op_maskg: return powerpc_unknown_instruction;
        case power_op_divdu: return powerpc_divdu;
        case power_op_xor: return powerpc_xor;
        case power_op_fadds: return powerpc_fadds;
        case power_op_fneg: return powerpc_fneg;
        case power_op_lwaux: return powerpc_lwaux;
        case power_op_fsub: return powerpc_fsub;
        case power_op_stfqux: return powerpc_unknown_instruction; // power2
        case power_op_srlq: return powerpc_unknown_instruction;
        case power_op_lfqx: return powerpc_unknown_instruction; // power2
        case power_op_dcbt: return powerpc_dcbt;
        case power_op_sliq: return powerpc_unknown_instruction;
        case power_op_fcmpo: return powerpc_fcmpo;
        case power_op_lhax: return powerpc_lhax;
        case power_op_cror: return powerpc_cror;
        case power_op_dozi: return powerpc_unknown_instruction;
        case power_op_crand: return powerpc_crand;
        case power_op_stfsu: return powerpc_stfsu;
        case power_op_lha: return powerpc_lha;
        case power_op_mcrf: return powerpc_mcrf;
        case power_op_fdiv: return powerpc_fdiv;
        case power_op_ori: return powerpc_ori;
        case power_op_fmadd: return powerpc_fmadd;
        case power_op_stmw: return powerpc_stmw;
        case power_op_lwarx: return powerpc_lwarx;
        case power_op_sle: return powerpc_unknown_instruction;
        case power_op_fsubs: return powerpc_fsubs;
        case power_op_stdx: return powerpc_stdx;
        case power_op_stwx: return powerpc_stwx;
        case power_op_sthux: return powerpc_sthux;
        case power_op_stwbrx: return powerpc_stwbrx;
        case power_op_sthu: return powerpc_sthu;
        case power_op_dclst: return powerpc_unknown_instruction;
        case power_op_fcmpu: return powerpc_fcmpu;
        case power_op_subfme: return powerpc_subfme;
        case power_op_stfiwx: return powerpc_stfiwx;
        case power_op_mul: return powerpc_unknown_instruction;
        case power_op_bc: return powerpc_bc;
        case power_op_stwux: return powerpc_stwux;
        case power_op_sllq: return powerpc_unknown_instruction;
        case power_op_mullw: return powerpc_mullw;
        case power_op_cmpi: return powerpc_cmpi;
        case power_op_rldicr: return powerpc_rldicr;
        case power_op_sth: return powerpc_sth;
        case power_op_sre: return powerpc_unknown_instruction;
        case power_op_slliq: return powerpc_unknown_instruction;
        case power_op_rldic: return powerpc_rldic;
        case power_op_fnabs: return powerpc_fnabs;
        case power_op_sc: return powerpc_sc;
        case power_op_addic_rc: return powerpc_addic_record;
        case power_op_rldimi: return powerpc_rldimi;
        case power_op_stfqu: return powerpc_unknown_instruction; // power2
        case power_op_neg: return powerpc_neg;
        case power_op_oris: return powerpc_oris;
        case power_op_lfsux: return powerpc_lfsux;
        case power_op_mtfsb1: return powerpc_mtfsb1;
        case power_op_dcbtst: return powerpc_dcbtst;
        case power_op_subfe: return powerpc_subfe;
        case power_op_b: return powerpc_b;
        case power_op_lwzux: return powerpc_lwzux;
        case power_op_rac: return powerpc_unknown_instruction;
        case power_op_lfdux: return powerpc_lfdux;
        case power_op_lbzu: return powerpc_lbzu;
        case power_op_lhzux: return powerpc_lhzux;
        case power_op_lhbrx: return powerpc_lhbrx;
        case power_op_lfsu: return powerpc_lfsu;
        case power_op_srw: return powerpc_srw;
        case power_op_crxor: return powerpc_crxor;
        case power_op_stfdux: return powerpc_stfdux;
        case power_op_lmw: return powerpc_lmw;
        case power_op_adde: return powerpc_adde;
        case power_op_mfsr: return powerpc_mfsr;
        case power_op_sraiq: return powerpc_unknown_instruction;
        case power_op_rrib: return powerpc_unknown_instruction;
        case power_op_addi: return powerpc_addi;
        case power_op_sthx: return powerpc_sthx;
        case power_op_stfqx: return powerpc_unknown_instruction; // power2
        case power_op_andi_rc: return powerpc_andi_record;
        case power_op_or: return powerpc_or;
        case power_op_dcbf: return powerpc_dcbf;
        case power_op_fcfid: return powerpc_fcfid;
        case power_op_fmadds: return powerpc_fmadds;
        case power_op_mtfsb0: return powerpc_mtfsb0;
        case power_op_lswi: return powerpc_lswi;
        case power_op_mulhdu: return powerpc_mulhdu;
        case power_op_ldarx: return powerpc_ldarx;
        case power_op_eieio: return powerpc_eieio;
        case power_op_cntlzd: return powerpc_cntlzd;
        case power_op_subfze: return powerpc_subfze;
        case power_op_fabs: return powerpc_fabs;
        case power_op_tw: return powerpc_tw;
        case power_op_eqv: return powerpc_eqv;
        case power_op_stfq: return powerpc_unknown_instruction; // power2
        case power_op_maskir: return powerpc_unknown_instruction;
        case power_op_sriq: return powerpc_unknown_instruction;
        case power_op_mfspr: return powerpc_mfspr;
        case power_op_ldx: return powerpc_ldx;
        case power_op_crorc: return powerpc_crorc;
        case power_op_lfd: return powerpc_lfd;
        case power_op_cmp: return powerpc_cmp;
        case power_op_stbu: return powerpc_stbu;
        case power_op_stfpdux: return powerpc_stfpdux;
        case power_op_stfpdx: return powerpc_stfpdx;
        case power_op_stfpsux: return powerpc_stfpsux;
        case power_op_stfpsx: return powerpc_stfpsx;
        case power_op_stfxdux: return powerpc_stfxdux;
        case power_op_stfxdx: return powerpc_stfxdx;
        case power_op_stfxsux: return powerpc_stfxsux;
        case power_op_stfxsx: return powerpc_stfxsx;
        case power_op_stfsdux: return powerpc_stfsdux;
        case power_op_stfsdx: return powerpc_stfsdx;
        case power_op_stfssux: return powerpc_stfssux;
        case power_op_stfssx: return powerpc_stfssx;
        case power_op_stfpiwx: return powerpc_stfpiwx;
        case power_op_lfpdux: return powerpc_lfpdux;
        case power_op_lfpdx: return powerpc_lfpdx;
        case power_op_lfpsux: return powerpc_lfpsux;
        case power_op_lfpsx: return powerpc_lfpsx;
        case power_op_lfxdux: return powerpc_lfxdux;
        case power_op_lfxdx: return powerpc_lfxdx;
        case power_op_lfxsux: return powerpc_lfxsux;
        case power_op_lfxsx: return powerpc_lfxsx;
        case power_op_lfsdux: return powerpc_lfsdux;
        case power_op_lfsdx: return powerpc_lfsdx;
        case power_op_lfssux: return powerpc_lfssux;
        case power_op_lfssx: return powerpc_lfssx;
        case power_op_fxcxnms: return powerpc_fxcxnms;
        case power_op_fxcxma: return powerpc_fxcxma;
        case power_op_fxcxnsma: return powerpc_fxcxnsma;
        case power_op_fxcxnpma: return powerpc_fxcxnpma;
        case power_op_fxcsnsma: return powerpc_fxcsnsma;
        case power_op_fxcpnsma: return powerpc_fxcpnsma;
        case power_op_fxcsnpma: return powerpc_fxcsnpma;
        case power_op_fxcpnpma: return powerpc_fxcpnpma;
        case power_op_fsmtp: return powerpc_fsmtp;
        case power_op_fsmfp: return powerpc_fsmfp;
        case power_op_fpctiwz: return powerpc_fpctiwz;
        case power_op_fpctiw: return powerpc_fpctiw;
        case power_op_fxmr: return powerpc_fxmr;
        case power_op_fpsel: return powerpc_fpsel;
        case power_op_fpmul: return powerpc_fpmul;
        case power_op_fxmul: return powerpc_fxmul;
        case power_op_fxpmul: return powerpc_fxpmul;
        case power_op_fxsmul: return powerpc_fxsmul;
        case power_op_fpadd: return powerpc_fpadd;
        case power_op_fpsub: return powerpc_fpsub;
        case power_op_fpre: return powerpc_fpre;
        case power_op_fprsqrte: return powerpc_fprsqrte;
        case power_op_fpmadd: return powerpc_fpmadd;
        case power_op_fxmadd: return powerpc_fxmadd;
        case power_op_fxcpmadd: return powerpc_fxcpmadd;
        case power_op_fxcsmadd: return powerpc_fxcsmadd;
        case power_op_fpnmadd: return powerpc_fpnmadd;
        case power_op_fxnmadd: return powerpc_fxnmadd;
        case power_op_fxcpnmadd: return powerpc_fxcpnmadd;
        case power_op_fxcsnmadd: return powerpc_fxcsnmadd;
        case power_op_fpmsub: return powerpc_fpmsub;
        case power_op_fxmsub: return powerpc_fxmsub;
        case power_op_fxcpmsub: return powerpc_fxcpmsub;
        case power_op_fxcsmsub: return powerpc_fxcsmsub;
        case power_op_fpnmsub: return powerpc_fpnmsub;
        case power_op_fxnmsub: return powerpc_fxnmsub;
        case power_op_fxcpnmsub: return powerpc_fxcpnmsub;
        case power_op_fxcsnmsub: return powerpc_fxcsnmsub;
        case power_op_fpmr: return powerpc_fpmr;
        case power_op_fpabs: return powerpc_fpabs;
        case power_op_fpneg: return powerpc_fpneg;
        case power_op_fprsp: return powerpc_fprsp;
        case power_op_fpnabs: return powerpc_fpnabs;
        case power_op_fsmr: return powerpc_fsmr;
        case power_op_fscmp: return powerpc_unknown_instruction; // someone screwed up their double hummer implementation!
        case power_op_fsabs: return powerpc_fsabs;
        case power_op_fsneg: return powerpc_fsneg;
        case power_op_fsnabs: return powerpc_fsnabs;
        case power_op_lwa: return powerpc_lwa;
        default:
            return powerpc_unknown_instruction;
    }
}
