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

// This file was automatically generated

//#include "SymEval.h"
//#include "SymEvalPolicy.h"
#include "RoseInsnFactory.h"

#include "../rose/RegisterDescriptor.h"
#include "DynAST.h"

#include "external/rose/armv8InstructionEnum.h"
#include "external/rose/amdgpuInstructionEnum.h"
#include "../rose/x86InstructionSemantics.h"
#include "external/rose/powerpcInstructionEnum.h"

using namespace Dyninst;
using namespace Dyninst::InstructionAPI;
using namespace Dyninst::DataflowAPI;
    
X86InstructionKind RoseInsnX86Factory::convertKind(entryID opcode, prefixEntryID prefix) {
  switch (prefix) {
    case prefix_rep:
        switch (opcode) {
          case e_insb: return x86_rep_insb;
          case e_insd: return x86_rep_insd;
          case e_insw: return x86_rep_insw;
          case e_lodsb: return x86_rep_lodsb;
          case e_lodsd: return x86_rep_lodsd;
          case e_lodsw: return x86_rep_lodsw;
          case e_movsb: return x86_rep_movsb;
          case e_movsd: return x86_rep_movsd;
          case e_movsw: return x86_rep_movsw;
          case e_outsb: return x86_rep_outsb;
          case e_outsd: return x86_rep_outsd;
          case e_outsw: return x86_rep_outsw;
          case e_stosb: return x86_rep_stosb;
          case e_stosd: return x86_rep_stosd;
          case e_stosw: return x86_rep_stosw;
          case e_cmpsb: return x86_repe_cmpsb;
          case e_cmpsd: return x86_repe_cmpsd;
          case e_cmpsw: return x86_repe_cmpsw;
          case e_scasb: return x86_repe_scasb;
          case e_scasd: return x86_repe_scasd;
          case e_scasw: return x86_repe_scasw;
          default: return x86_unknown_instruction;
        }
    break;
    case prefix_repnz:
        switch (opcode) {
          case e_cmpsb: return x86_repne_cmpsb;
          case e_cmpsd: return x86_repne_cmpsd;
          case e_cmpsw: return x86_repne_cmpsw;
          case e_scasb: return x86_repne_scasb;
          case e_scasd: return x86_repne_scasd;
          case e_scasw: return x86_repne_scasw;
          default: return x86_unknown_instruction;
        }
    break;
    case prefix_none:
    default:
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
        case e_cbw:
            return x86_cbw;
        case e_cdq:
            return x86_cdq;
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
        case e_cwd:
            return x86_cwd;
        case e_cwde:
            return x86_cwde;
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
        case e_insd:
            return x86_insd;
        case e_insertq:
            return x86_insertq;
        case e_insw:
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
        case e_lodsd:
            return x86_lodsd;
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
        case e_movsw:
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
        case e_outsd:
            return x86_outsd;
        case e_outsw:
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
        case e_popa:
            return x86_popa;
        case e_popad:
            return x86_popad;
        case e_popf:
            return x86_popf;
        case e_popfd:
            return x86_popfd;
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
        case e_pusha:
            return x86_pusha;
        case e_pushad:
            return x86_pushad;
        case e_pushf:
            return x86_pushf;
        case e_pushfd:
            return x86_pushfd;
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
        case e_scasd:
            return x86_scasd;
        case e_scasw:
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
        case e_stosd:
            return x86_stosd;
        case e_stosw:
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
    break;
  }
}

PowerpcInstructionKind RoseInsnPPCFactory::convertKind(entryID opcode,
						       std::string mnem)
{
  PowerpcInstructionKind ret = powerpc_unknown_instruction;
    switch(opcode)
    {
        case power_op_stfdu: ret = powerpc_stfdu; break;
        case power_op_fadd: ret = powerpc_fadd; break;
        case power_op_xoris: ret = powerpc_xoris; break;
        case power_op_mulhwu: ret = powerpc_mulhwu; break;
        case power_op_stbux: ret = powerpc_stbux; break;
        case power_op_cmpl: ret = powerpc_cmpl; break;
        case power_op_subf: ret = powerpc_subf; break;
        case power_op_svcs: ret = powerpc_sc; break;
        case power_op_fmuls: ret = powerpc_fmuls; break;
        case power_op_subfic: ret = powerpc_subfic; break;
        case power_op_mcrfs: ret = powerpc_mcrfs; break;
        case power_op_divs: ret = powerpc_divw; break;
        case power_op_lwzx: ret = powerpc_lwzx; break;
        case power_op_fctiw: ret = powerpc_fctiw; break;
        case power_op_mtcrf: ret = powerpc_mtcrf; break;
        case power_op_srq: ret = powerpc_unknown_instruction; break;
        case power_op_sraw: ret = powerpc_sraw; break;
        case power_op_lfdx: ret = powerpc_lfdx; break;
        case power_op_stdcx_rc: ret = powerpc_stdcx_record; break;
        case power_op_nor: ret = powerpc_nor; break;
        case power_op_crandc: ret = powerpc_crandc; break;
        case power_op_stdu: ret = powerpc_stdu; break;
        case power_op_addme: ret = powerpc_addme; break;
        case power_op_fmul: ret = powerpc_fmul; break;
        case power_op_sthbrx: ret = powerpc_sthbrx; break;
        case power_op_mtspr: ret = powerpc_mtspr; break;
        case power_op_lfsx: ret = powerpc_lfsx; break;
        case power_op_lbzx: ret = powerpc_lbzx; break;
        case power_op_nand: ret = powerpc_nand; break;
        case power_op_fnmadds: ret = powerpc_fnmadds; break;
        case power_op_fnmadd: ret = powerpc_fnmadd; break;
        case power_op_mulhw: ret = powerpc_mulhw; break;
        case power_op_sradi: ret = powerpc_sradi; break;
        case power_op_fnmsubs: ret = powerpc_fnmsubs; break;
        case power_op_addze: ret = powerpc_addze; break;
        case power_op_mulld: ret = powerpc_mulld; break;
        case power_op_addic: ret = powerpc_addc; break;
        case power_op_lfs: ret = powerpc_lfs; break;
        case power_op_andc: ret = powerpc_andc; break;
        case power_op_eciwx: ret = powerpc_eciwx; break;
        case power_op_rfid: ret = powerpc_rfid; break;
        case power_op_divw: ret = powerpc_divw; break;
        case power_op_creqv: ret = powerpc_creqv; break;
        case power_op_fctiwz: ret = powerpc_fctiwz; break;
        case power_op_crnor: ret = powerpc_crnor; break;
        case power_op_lbzux: ret = powerpc_lbzux; break;
        case power_op_td: ret = powerpc_td; break;
        case power_op_dcbi: ret = powerpc_dcbi; break;
        case power_op_cli: ret = powerpc_unknown_instruction; break;
        case power_op_div: ret = powerpc_unknown_instruction; break;
        case power_op_add: ret = powerpc_add; break;
        case power_op_extsh: ret = powerpc_extsh; break;
        case power_op_divd: ret = powerpc_divd; break;
        case power_op_fmsub: ret = powerpc_fmsub; break;
        case power_op_stbx: ret = powerpc_stbx; break;
        case power_op_nabs: ret = powerpc_unknown_instruction; break;
        case power_op_isync: ret = powerpc_isync; break;
        case power_op_mfsri: ret = powerpc_unknown_instruction; break;
        case power_op_stfdx: ret = powerpc_stfdx; break;
        case power_op_fsqrt: ret = powerpc_fsqrt; break;
        case power_op_dcbz: ret = powerpc_dcbz; break;
        case power_op_dcbst: ret = powerpc_dcbst; break;
        case power_op_stswi: ret = powerpc_stswi; break;
        case power_op_mulli: ret = powerpc_mulli; break;
        case power_op_stfs: ret = powerpc_stfs; break;
        case power_op_clf: ret = powerpc_unknown_instruction; break;
        case power_op_fnmsub: ret = powerpc_fnmsub; break;
        case power_op_lhz: ret = powerpc_lhz; break;
        case power_op_ecowx: ret = powerpc_ecowx; break;
        case power_op_fres: ret = powerpc_fres; break;
        case power_op_stwu: ret = powerpc_stwu; break;
        case power_op_lhau: ret = powerpc_lhau; break;
        case power_op_slq: ret = powerpc_unknown_instruction; break;
        case power_op_srawi: ret = powerpc_srawi; break;
        case power_op_divwu: ret = powerpc_divwu; break;
        case power_op_addis: ret = powerpc_addis; break;
        case power_op_mfmsr: ret = powerpc_mfmsr; break;
        case power_op_mulhd: ret = powerpc_mulhd; break;
        case power_op_fdivs: ret = powerpc_fdivs; break;
        case power_op_abs: ret = powerpc_unknown_instruction; break;
        case power_op_lwzu: ret = powerpc_lwzu; break;
        case power_op_tlbli: ret = powerpc_unknown_instruction; break; // PPC 603 only
        case power_op_orc: ret = powerpc_orc; break;
        case power_op_mtfsf: ret = powerpc_mtfsf; break;
        case power_op_lswx: ret = powerpc_lswx; break;
        case power_op_stb: ret = powerpc_stb; break;
        case power_op_andis_rc: ret = powerpc_andis_record; break;
        case power_op_fsel: ret = powerpc_fsel; break;
        case power_op_xori: ret = powerpc_xori; break;
        case power_op_lwax: ret = powerpc_lwax; break;
        case power_op_tdi: ret = powerpc_tdi; break;
        case power_op_rlwimi: ret = powerpc_rlwimi; break;
        case power_op_stw: ret = powerpc_stw; break;
        case power_op_rldcr: ret = powerpc_rldcr; break;
        case power_op_sraq: ret = powerpc_unknown_instruction; break;
        case power_op_fmr: ret = powerpc_fmr; break;
        case power_op_tlbld: ret = powerpc_unknown_instruction; break; // PPC 603 only
        case power_op_doz: ret = powerpc_unknown_instruction; break;
        case power_op_lbz: ret = powerpc_lbz; break;
        case power_op_stdux: ret = powerpc_stdux; break;
        case power_op_mtfsfi: ret = powerpc_mtfsfi; break;
        case power_op_srea: ret = powerpc_unknown_instruction; break;
        case power_op_lscbx: ret = powerpc_unknown_instruction; break;
        case power_op_rlwinm: ret = powerpc_rlwinm; break;
        case power_op_sld: ret = powerpc_sld; break;
        case power_op_addc: ret = powerpc_addc; break;
        case power_op_lfqux: ret = powerpc_unknown_instruction; break; // POWER2 only; break; shouldn't this exist for double hummer?
        case power_op_sleq: ret = powerpc_unknown_instruction; break;
        case power_op_extsb: ret = powerpc_extsb; break;
        case power_op_ld: ret = powerpc_ld; break;
        case power_op_ldu: ret = powerpc_ldu; break;
        case power_op_fctidz: ret = powerpc_fctidz; break;
        case power_op_lfq: ret = powerpc_unknown_instruction; break; // again, POWER2 only
        case power_op_lwbrx: ret = powerpc_lwbrx; break;
        case power_op_fsqrts: ret = powerpc_fsqrts; break;
        case power_op_srd: ret = powerpc_srd; break;
        case power_op_lfdu: ret = powerpc_lfdu; break;
        case power_op_stfsux: ret = powerpc_stfsux; break;
        case power_op_lhzu: ret = powerpc_lhzu; break;
        case power_op_crnand: ret = powerpc_crnand; break;
        case power_op_icbi: ret = powerpc_icbi; break;
        case power_op_rlwnm: ret = powerpc_rlwnm; break;
        case power_op_rldcl: ret = powerpc_rldcl; break;
        case power_op_stwcx_rc: ret = powerpc_stwcx_record; break;
        case power_op_lhzx: ret = powerpc_lhzx; break;
        case power_op_stfsx: ret = powerpc_stfsx; break;
        case power_op_rlmi: ret = powerpc_unknown_instruction; break;
        case power_op_twi: ret = powerpc_twi; break;
        case power_op_srliq: ret = powerpc_unknown_instruction; break;
        case power_op_tlbie: ret = powerpc_tlbie; break;
        case power_op_mfcr: ret = powerpc_mfcr; break;
        case power_op_tlbsync: ret = powerpc_tlbsync; break;
        case power_op_extsw: ret = powerpc_extsw; break;
        case power_op_rldicl: ret = powerpc_rldicl; break;
        case power_op_bclr: ret = powerpc_bclr; break;
        case power_op_rfsvc: ret = powerpc_unknown_instruction; break;
        case power_op_mcrxr: ret = powerpc_mcrxr; break;
        case power_op_clcs: ret = powerpc_unknown_instruction; break;
        case power_op_srad: ret = powerpc_srad; break;
        case power_op_subfc: ret = powerpc_subfc; break;
        case power_op_mfsrin: ret = powerpc_mfsrin; break;
        case power_op_rfi: ret = powerpc_rfi; break;
        case power_op_sreq: ret = powerpc_unknown_instruction; break;
        case power_op_frsqrte: ret = powerpc_frsqrte; break;
        case power_op_mffs: ret = powerpc_mffs; break;
        case power_op_lwz: ret = powerpc_lwz; break;
        case power_op_lfqu: ret = powerpc_unknown_instruction; break; // power2
        case power_op_and: ret = powerpc_and; break;
        case power_op_stswx: ret = powerpc_stswx; break;
        case power_op_stfd: ret = powerpc_stfd; break;
        case power_op_fmsubs: ret = powerpc_fmsubs; break;
        case power_op_bcctr: ret = powerpc_bcctr; break;
        case power_op_lhaux: ret = powerpc_lhaux; break;
        case power_op_ldux: ret = powerpc_ldux; break;
        case power_op_fctid: ret = powerpc_fctid; break;
        case power_op_frsp: ret = powerpc_frsp; break;
        case power_op_slw: ret = powerpc_slw; break;
        case power_op_cmpli: ret = powerpc_cmpli; break;
        case power_op_sync: ret = powerpc_sync; break;
        case power_op_cntlzw: ret = powerpc_cntlzw; break;
        case power_op_maskg: ret = powerpc_unknown_instruction; break;
        case power_op_divdu: ret = powerpc_divdu; break;
        case power_op_xor: ret = powerpc_xor; break;
        case power_op_fadds: ret = powerpc_fadds; break;
        case power_op_fneg: ret = powerpc_fneg; break;
        case power_op_lwaux: ret = powerpc_lwaux; break;
        case power_op_fsub: ret = powerpc_fsub; break;
        case power_op_stfqux: ret = powerpc_unknown_instruction; break; // power2
        case power_op_srlq: ret = powerpc_unknown_instruction; break;
        case power_op_lfqx: ret = powerpc_unknown_instruction; break; // power2
        case power_op_dcbt: ret = powerpc_dcbt; break;
        case power_op_sliq: ret = powerpc_unknown_instruction; break;
        case power_op_fcmpo: ret = powerpc_fcmpo; break;
        case power_op_lhax: ret = powerpc_lhax; break;
        case power_op_cror: ret = powerpc_cror; break;
        case power_op_dozi: ret = powerpc_unknown_instruction; break;
        case power_op_crand: ret = powerpc_crand; break;
        case power_op_stfsu: ret = powerpc_stfsu; break;
        case power_op_lha: ret = powerpc_lha; break;
        case power_op_mcrf: ret = powerpc_mcrf; break;
        case power_op_fdiv: ret = powerpc_fdiv; break;
        case power_op_ori: ret = powerpc_ori; break;
        case power_op_fmadd: ret = powerpc_fmadd; break;
        case power_op_stmw: ret = powerpc_stmw; break;
        case power_op_lwarx: ret = powerpc_lwarx; break;
        case power_op_sle: ret = powerpc_unknown_instruction; break;
        case power_op_fsubs: ret = powerpc_fsubs; break;
        case power_op_stdx: ret = powerpc_stdx; break;
        case power_op_stwx: ret = powerpc_stwx; break;
        case power_op_sthux: ret = powerpc_sthux; break;
        case power_op_stwbrx: ret = powerpc_stwbrx; break;
        case power_op_sthu: ret = powerpc_sthu; break;
        case power_op_dclst: ret = powerpc_unknown_instruction; break;
        case power_op_fcmpu: ret = powerpc_fcmpu; break;
        case power_op_subfme: ret = powerpc_subfme; break;
        case power_op_stfiwx: ret = powerpc_stfiwx; break;
        case power_op_mul: ret = powerpc_unknown_instruction; break;
        case power_op_bc: ret = powerpc_bc; break;
        case power_op_stwux: ret = powerpc_stwux; break;
        case power_op_sllq: ret = powerpc_unknown_instruction; break;
        case power_op_mullw: ret = powerpc_mullw; break;
        case power_op_cmpi: ret = powerpc_cmpi; break;
        case power_op_rldicr: ret = powerpc_rldicr; break;
        case power_op_sth: ret = powerpc_sth; break;
        case power_op_sre: ret = powerpc_unknown_instruction; break;
        case power_op_slliq: ret = powerpc_unknown_instruction; break;
        case power_op_rldic: ret = powerpc_rldic; break;
        case power_op_fnabs: ret = powerpc_fnabs; break;
        case power_op_sc: ret = powerpc_sc; break;
        case power_op_addic_rc: ret = powerpc_addic_record; break;
        case power_op_rldimi: ret = powerpc_rldimi; break;
        case power_op_stfqu: ret = powerpc_unknown_instruction; break; // power2
        case power_op_neg: ret = powerpc_neg; break;
        case power_op_oris: ret = powerpc_oris; break;
        case power_op_lfsux: ret = powerpc_lfsux; break;
        case power_op_mtfsb1: ret = powerpc_mtfsb1; break;
        case power_op_dcbtst: ret = powerpc_dcbtst; break;
        case power_op_subfe: ret = powerpc_subfe; break;
        case power_op_b: ret = powerpc_b; break;
        case power_op_lwzux: ret = powerpc_lwzux; break;
        case power_op_rac: ret = powerpc_unknown_instruction; break;
        case power_op_lfdux: ret = powerpc_lfdux; break;
        case power_op_lbzu: ret = powerpc_lbzu; break;
        case power_op_lhzux: ret = powerpc_lhzux; break;
        case power_op_lhbrx: ret = powerpc_lhbrx; break;
        case power_op_lfsu: ret = powerpc_lfsu; break;
        case power_op_srw: ret = powerpc_srw; break;
        case power_op_crxor: ret = powerpc_crxor; break;
        case power_op_stfdux: ret = powerpc_stfdux; break;
        case power_op_lmw: ret = powerpc_lmw; break;
        case power_op_adde: ret = powerpc_adde; break;
        case power_op_mfsr: ret = powerpc_mfsr; break;
        case power_op_sraiq: ret = powerpc_unknown_instruction; break;
        case power_op_rrib: ret = powerpc_unknown_instruction; break;
        case power_op_addi: ret = powerpc_addi; break;
        case power_op_sthx: ret = powerpc_sthx; break;
        case power_op_stfqx: ret = powerpc_unknown_instruction; break; // power2
        case power_op_andi_rc: ret = powerpc_andi_record; break;
        case power_op_or: ret = powerpc_or; break;
        case power_op_dcbf: ret = powerpc_dcbf; break;
        case power_op_fcfid: ret = powerpc_fcfid; break;
        case power_op_fmadds: ret = powerpc_fmadds; break;
        case power_op_mtfsb0: ret = powerpc_mtfsb0; break;
        case power_op_lswi: ret = powerpc_lswi; break;
        case power_op_mulhdu: ret = powerpc_mulhdu; break;
        case power_op_ldarx: ret = powerpc_ldarx; break;
        case power_op_eieio: ret = powerpc_eieio; break;
        case power_op_cntlzd: ret = powerpc_cntlzd; break;
        case power_op_subfze: ret = powerpc_subfze; break;
        case power_op_fabs: ret = powerpc_fabs; break;
        case power_op_tw: ret = powerpc_tw; break;
        case power_op_eqv: ret = powerpc_eqv; break;
        case power_op_stfq: ret = powerpc_unknown_instruction; break; // power2
        case power_op_maskir: ret = powerpc_unknown_instruction; break;
        case power_op_sriq: ret = powerpc_unknown_instruction; break;
        case power_op_mfspr: ret = powerpc_mfspr; break;
        case power_op_ldx: ret = powerpc_ldx; break;
        case power_op_crorc: ret = powerpc_crorc; break;
        case power_op_lfd: ret = powerpc_lfd; break;
        case power_op_cmp: ret = powerpc_cmp; break;
        case power_op_stbu: ret = powerpc_stbu; break;
        case power_op_stfpdux: ret = powerpc_stfpdux; break;
        case power_op_stfpdx: ret = powerpc_stfpdx; break;
        case power_op_stfpsux: ret = powerpc_stfpsux; break;
        case power_op_stfpsx: ret = powerpc_stfpsx; break;
        case power_op_stfxdux: ret = powerpc_stfxdux; break;
        case power_op_stfxdx: ret = powerpc_stfxdx; break;
        case power_op_stfxsux: ret = powerpc_stfxsux; break;
        case power_op_stfxsx: ret = powerpc_stfxsx; break;
        case power_op_stfsdux: ret = powerpc_stfsdux; break;
        case power_op_stfsdx: ret = powerpc_stfsdx; break;
        case power_op_stfssux: ret = powerpc_stfssux; break;
        case power_op_stfssx: ret = powerpc_stfssx; break;
        case power_op_stfpiwx: ret = powerpc_stfpiwx; break;
        case power_op_lfpdux: ret = powerpc_lfpdux; break;
        case power_op_lfpdx: ret = powerpc_lfpdx; break;
        case power_op_lfpsux: ret = powerpc_lfpsux; break;
        case power_op_lfpsx: ret = powerpc_lfpsx; break;
        case power_op_lfxdux: ret = powerpc_lfxdux; break;
        case power_op_lfxdx: ret = powerpc_lfxdx; break;
        case power_op_lfxsux: ret = powerpc_lfxsux; break;
        case power_op_lfxsx: ret = powerpc_lfxsx; break;
        case power_op_lfsdux: ret = powerpc_lfsdux; break;
        case power_op_lfsdx: ret = powerpc_lfsdx; break;
        case power_op_lfssux: ret = powerpc_lfssux; break;
        case power_op_lfssx: ret = powerpc_lfssx; break;
        case power_op_fxcxnms: ret = powerpc_fxcxnms; break;
        case power_op_fxcxma: ret = powerpc_fxcxma; break;
        case power_op_fxcxnsma: ret = powerpc_fxcxnsma; break;
        case power_op_fxcxnpma: ret = powerpc_fxcxnpma; break;
        case power_op_fxcsnsma: ret = powerpc_fxcsnsma; break;
        case power_op_fxcpnsma: ret = powerpc_fxcpnsma; break;
        case power_op_fxcsnpma: ret = powerpc_fxcsnpma; break;
        case power_op_fxcpnpma: ret = powerpc_fxcpnpma; break;
        case power_op_fsmtp: ret = powerpc_fsmtp; break;
        case power_op_fsmfp: ret = powerpc_fsmfp; break;
        case power_op_fpctiwz: ret = powerpc_fpctiwz; break;
        case power_op_fpctiw: ret = powerpc_fpctiw; break;
        case power_op_fxmr: ret = powerpc_fxmr; break;
        case power_op_fpsel: ret = powerpc_fpsel; break;
        case power_op_fpmul: ret = powerpc_fpmul; break;
        case power_op_fxmul: ret = powerpc_fxmul; break;
        case power_op_fxpmul: ret = powerpc_fxpmul; break;
        case power_op_fxsmul: ret = powerpc_fxsmul; break;
        case power_op_fpadd: ret = powerpc_fpadd; break;
        case power_op_fpsub: ret = powerpc_fpsub; break;
        case power_op_fpre: ret = powerpc_fpre; break;
        case power_op_fprsqrte: ret = powerpc_fprsqrte; break;
        case power_op_fpmadd: ret = powerpc_fpmadd; break;
        case power_op_fxmadd: ret = powerpc_fxmadd; break;
        case power_op_fxcpmadd: ret = powerpc_fxcpmadd; break;
        case power_op_fxcsmadd: ret = powerpc_fxcsmadd; break;
        case power_op_fpnmadd: ret = powerpc_fpnmadd; break;
        case power_op_fxnmadd: ret = powerpc_fxnmadd; break;
	/*
        case power_op_qvfcfids: ret = powerpc_qvfcfids; break;
        case power_op_qvlfsx: ret = powerpc_qvlfsx; break;
        case power_op_qvlfsux: ret = powerpc_qvlfsux; break;
        case power_op_qvlfcsx: ret = powerpc_qvlfcsx; break;
        case power_op_qvlfcsux: ret = powerpc_qvlfcsux; break;
        case power_op_qvlfdx: ret = powerpc_qvlfdx; break;
        case power_op_qvlfdux: ret = powerpc_qvlfdux; break;
        case power_op_qvlfcdx: ret = powerpc_qvlfcdx; break;
        case power_op_qvlfcdux: ret = powerpc_qvlfcdux; break;
        case power_op_qvlfiwax: ret = powerpc_qvlfiwax; break;
        case power_op_qvlfiwzx: ret = powerpc_qvlfiwzx; break;
        case power_op_qvlpcldx: ret = powerpc_qvlpcldx; break;
        case power_op_qvlpclsx: ret = powerpc_qvlpclsx; break;
        case power_op_qvlpcrdx: ret = powerpc_qvlpcrdx; break;
        case power_op_qvlpcrsx: ret = powerpc_qvlpcrsx; break;
        case power_op_qvstfsx: ret = powerpc_qvstfsx; break;
        case power_op_qvstfsux: ret = powerpc_qvstfsux; break;
        case power_op_qvstfsxi: ret = powerpc_qvstfsxi; break;
        case power_op_qvstfdx: ret = powerpc_qvstfdx; break;
        case power_op_qvstfdux: ret = powerpc_qvstfdux; break;
        case power_op_qvstfsuxi: ret = powerpc_qvstfsuxi; break;
        case power_op_qvstfdxi: ret = powerpc_qvstfdx; break;
        case power_op_qvstfduxi: ret = powerpc_qvstfdux; break;
        case power_op_qvstfcsx: ret = powerpc_qvstfcsx; break;
        case power_op_qvstfcsux: ret = powerpc_qvstfcsux; break;
        case power_op_qvstfcsxi: ret = powerpc_qvstfcsxi; break;
        case power_op_qvstfcdx: ret = powerpc_qvstfcdx; break;
        case power_op_qvstfcdux: ret = powerpc_qvstfcdux; break;
        case power_op_qvstfcsuxi: ret = powerpc_qvstfcsuxi; break;
        case power_op_qvstfcdxi: ret = powerpc_qvstfcdx; break;
        case power_op_qvstfcduxi: ret = powerpc_qvstfcdux; break;
        case power_op_qvfmr: ret = powerpc_qvfmr; break;
        case power_op_qvfcpsgn: ret = powerpc_qvfcpsgn; break;
        case power_op_qvfneg: ret = powerpc_qvfneg; break;
        case power_op_qvfabs: ret = powerpc_qvfabs; break;
        case power_op_qvfnabs: ret = powerpc_qvfnabs; break;
        case power_op_qvfadd: ret = powerpc_qvfadd; break;
        case power_op_qvfadds: ret = powerpc_qvfadds; break;
        case power_op_qvfsub: ret = powerpc_qvfsub; break;
        case power_op_qvfsubs: ret = powerpc_qvfsubs; break;
        case power_op_qvfmul: ret = powerpc_qvfmul; break;
        case power_op_qvfmuls: ret = powerpc_qvfmuls; break;
        case power_op_qvfre: ret = powerpc_qvfre; break;
        case power_op_qvfres: ret = powerpc_qvfres; break;
        case power_op_qvfrsqrte: ret = powerpc_qvfrsqrte; break;
        case power_op_qvfrsqrtes: ret = powerpc_qvfrsqrtes; break;
        case power_op_qvfmadd: ret = powerpc_qvfmadd; break;
        case power_op_qvfmadds: ret = powerpc_qvfmadds; break;
        case power_op_qvfmsub: ret = powerpc_qvfmsub; break;
        case power_op_qvfmsubs: ret = powerpc_qvfmsubs; break;
        case power_op_qvfnmadd: ret = powerpc_qvfnmadd; break;
        case power_op_qvfnmadds: ret = powerpc_qvfnmadds; break;
        case power_op_qvfnmsub: ret = powerpc_qvfnmsub; break;
        case power_op_qvfnmsubs: ret = powerpc_qvfnmsubs; break;
        case power_op_qvfxmadd: ret = powerpc_qvfxmadd; break;
        case power_op_qvfxmadds: ret = powerpc_qvfxmadds; break;
        case power_op_qvfxxnpmadd: ret = powerpc_qvfxxnpmadd; break;
        case power_op_qvfxxnpmadds: ret = powerpc_qvfxxnpmadds; break;
        case power_op_qvfxxmadd: ret = powerpc_qvfxxmadd; break;
        case power_op_qvfxxmadds: ret = powerpc_qvfxxmadds; break;
        case power_op_qvfxxcpnmadd: ret = powerpc_qvfxxcpnmadd; break;
        case power_op_qvfxxcpnmadds: ret = powerpc_qvfxxcpnmadds; break;
        case power_op_qvfxmul: ret = powerpc_qvfxmul; break;
        case power_op_qvfxmuls: ret = powerpc_qvfxmuls; break;
        case power_op_qvfrsp: ret = powerpc_qvfrsp; break;
        case power_op_qvfctid: ret = powerpc_qvfctid; break;
        case power_op_qvfctidz: ret = powerpc_qvfctidz; break;
        case power_op_qvfctidu: ret = powerpc_qvfctidu; break;
        case power_op_qvfctiduz: ret = powerpc_qvfctiduz; break;
        case power_op_qvfctiw: ret = powerpc_qvfctiw; break;
        case power_op_qvfctiwu: ret = powerpc_qvfctiwu; break;
        case power_op_qvfctiwz: ret = powerpc_qvfctiwz; break;
        case power_op_qvfctiwuz: ret = powerpc_qvfctiwuz; break;
        case power_op_qvfcfid: ret = powerpc_qvfcfid; break;
        case power_op_qvfcfidu: ret = powerpc_qvfcfidu; break;
        case power_op_qvfrin: ret = powerpc_qvfrin; break;
        case power_op_qvfriz: ret = powerpc_qvfriz; break;
        case power_op_qvfrip: ret = powerpc_qvfrip; break;
        case power_op_qvfrim: ret = powerpc_qvfrim; break;
        case power_op_qvftstnan: ret = powerpc_qvftstnan; break;
        case power_op_qvfcmpgt: ret = powerpc_qvfcmpgt; break;
        case power_op_qvfcmpeq: ret = powerpc_qvfcmpeq; break;
        case power_op_qvfsel: ret = powerpc_qvfsel; break;
        case power_op_qvfcmplt: ret = powerpc_qvfcmplt; break;
        case power_op_qvfaligni: ret = powerpc_qvfaligni; break;
        case power_op_qvfperm: ret = powerpc_qvfperm; break;
        case power_op_qvesplati: ret = powerpc_qvesplati; break;
        case power_op_qvgpci: ret = powerpc_qvgpci; break;
        case power_op_qvlstdux: ret = powerpc_qvlstdux; break;
        case power_op_qvlstduxi: ret = powerpc_qvlstduxi; break;
        case power_op_qvflogical: ret = powerpc_qvflogical; break;
	*/
        case power_op_fxcpnmadd: ret = powerpc_fxcpnmadd; break;
        case power_op_fxcsnmadd: ret = powerpc_fxcsnmadd; break;
        case power_op_fpmsub: ret = powerpc_fpmsub; break;
        case power_op_fxmsub: ret = powerpc_fxmsub; break;
        case power_op_fxcpmsub: ret = powerpc_fxcpmsub; break;
        case power_op_fxcsmsub: ret = powerpc_fxcsmsub; break;
        case power_op_fpnmsub: ret = powerpc_fpnmsub; break;
        case power_op_fxnmsub: ret = powerpc_fxnmsub; break;
        case power_op_fxcpnmsub: ret = powerpc_fxcpnmsub; break;
        case power_op_fxcsnmsub: ret = powerpc_fxcsnmsub; break;
//        case power_op_qvstfiwx: ret = powerpc_qvstfiwx; break;
        case power_op_fpmr: ret = powerpc_fpmr; break;
        case power_op_fpabs: ret = powerpc_fpabs; break;
        case power_op_fpneg: ret = powerpc_fpneg; break;
        case power_op_fprsp: ret = powerpc_fprsp; break;
        case power_op_fpnabs: ret = powerpc_fpnabs; break;
        case power_op_fsmr: ret = powerpc_fsmr; break;
        case power_op_fscmp: ret = powerpc_unknown_instruction; break; // someone screwed up their double hummer implementation!
        case power_op_fsabs: ret = powerpc_fsabs; break;
        case power_op_fsneg: ret = powerpc_fsneg; break;
        case power_op_fsnabs: ret = powerpc_fsnabs; break;
        case power_op_lwa: ret = powerpc_lwa; break;
        default:
            ret = powerpc_unknown_instruction; break;
    }
    if(ret != powerpc_stwcx_record && ret != powerpc_unknown_instruction &&
       ret != powerpc_andis_record && ret != powerpc_andi_record &&
       mnem.find(".") != std::string::npos) {
      ret = (PowerpcInstructionKind)((int)ret + 1);
    }
    return ret;
}

PowerpcInstructionKind RoseInsnPPCFactory::makeRoseBranchOpcode(entryID iapi_opcode, bool isAbsolute, bool isLink) {
  switch(iapi_opcode) {
  case power_op_b:
    if(isAbsolute && isLink) return powerpc_bla;
    if(isAbsolute) return powerpc_ba;
    if(isLink) return powerpc_bl;
    return powerpc_b;
  case power_op_bc:
    if(isAbsolute && isLink) return powerpc_bcla;
    if(isAbsolute) return powerpc_bca;
    if(isLink) return powerpc_bcl;
    return powerpc_bc;
  case power_op_bcctr:
    assert(!isAbsolute);
    if(isLink) return powerpc_bcctrl;
    return powerpc_bcctr;
  case power_op_bclr:
    assert(!isAbsolute);
    if(isLink) return powerpc_bclrl;
    return powerpc_bclr;
  default:
    assert(!"makeRoseBranchOpcode called with unknown branch opcode!");
    return powerpc_unknown_instruction;
  }
}


ARMv8InstructionKind RoseInsnArmv8Factory::convertKind(entryID opcode) {
    switch(opcode) {
        case aarch64_op_INVALID: return rose_aarch64_op_INVALID;
        case aarch64_op_extended: return rose_aarch64_op_extended;
        case aarch64_op_abs_advsimd: return rose_aarch64_op_abs_advsimd;
        case aarch64_op_adc: return rose_aarch64_op_adc;
        case aarch64_op_adcs: return rose_aarch64_op_adcs;
        case aarch64_op_add_addsub_ext: return rose_aarch64_op_add_addsub_ext;
        case aarch64_op_add_addsub_imm: return rose_aarch64_op_add_addsub_imm;
        case aarch64_op_add_addsub_shift: return rose_aarch64_op_add_addsub_shift;
        case aarch64_op_add_advsimd: return rose_aarch64_op_add_advsimd;
        case aarch64_op_addhn_advsimd: return rose_aarch64_op_addhn_advsimd;
        case aarch64_op_addp_advsimd_pair: return rose_aarch64_op_addp_advsimd_pair;
        case aarch64_op_addp_advsimd_vec: return rose_aarch64_op_addp_advsimd_vec;
        case aarch64_op_adds_addsub_ext: return rose_aarch64_op_adds_addsub_ext;
        case aarch64_op_adds_addsub_imm: return rose_aarch64_op_adds_addsub_imm;
        case aarch64_op_adds_addsub_shift: return rose_aarch64_op_adds_addsub_shift;
        case aarch64_op_addv_advsimd: return rose_aarch64_op_addv_advsimd;
        case aarch64_op_adr: return rose_aarch64_op_adr;
        case aarch64_op_adrp: return rose_aarch64_op_adrp;
        case aarch64_op_aesd_advsimd: return rose_aarch64_op_aesd_advsimd;
        case aarch64_op_aese_advsimd: return rose_aarch64_op_aese_advsimd;
        case aarch64_op_aesimc_advsimd: return rose_aarch64_op_aesimc_advsimd;
        case aarch64_op_aesmc_advsimd: return rose_aarch64_op_aesmc_advsimd;
        case aarch64_op_and_advsimd: return rose_aarch64_op_and_advsimd;
        case aarch64_op_and_log_imm: return rose_aarch64_op_and_log_imm;
        case aarch64_op_and_log_shift: return rose_aarch64_op_and_log_shift;
        case aarch64_op_ands_log_imm: return rose_aarch64_op_ands_log_imm;
        case aarch64_op_ands_log_shift: return rose_aarch64_op_ands_log_shift;
        case aarch64_op_asr_asrv: return rose_aarch64_op_asr_asrv;
        case aarch64_op_asr_sbfm: return rose_aarch64_op_asr_sbfm;
        case aarch64_op_asrv: return rose_aarch64_op_asrv;
        case aarch64_op_at_sys: return rose_aarch64_op_at_sys;
        case aarch64_op_b_cond: return rose_aarch64_op_b_cond;
        case aarch64_op_b_uncond: return rose_aarch64_op_b_uncond;
        case aarch64_op_bfi_bfm: return rose_aarch64_op_bfi_bfm;
        case aarch64_op_bfm: return rose_aarch64_op_bfm;
        case aarch64_op_bfxil_bfm: return rose_aarch64_op_bfxil_bfm;
        case aarch64_op_bic_advsimd_imm: return rose_aarch64_op_bic_advsimd_imm;
        case aarch64_op_bic_advsimd_reg: return rose_aarch64_op_bic_advsimd_reg;
        case aarch64_op_bic_log_shift: return rose_aarch64_op_bic_log_shift;
        case aarch64_op_bics: return rose_aarch64_op_bics;
        case aarch64_op_bif_advsimd: return rose_aarch64_op_bif_advsimd;
        case aarch64_op_bit_advsimd: return rose_aarch64_op_bit_advsimd;
        case aarch64_op_bl: return rose_aarch64_op_bl;
        case aarch64_op_blr: return rose_aarch64_op_blr;
        case aarch64_op_br: return rose_aarch64_op_br;
        case aarch64_op_brk: return rose_aarch64_op_brk;
        case aarch64_op_bsl_advsimd: return rose_aarch64_op_bsl_advsimd;
        case aarch64_op_cbnz: return rose_aarch64_op_cbnz;
        case aarch64_op_cbz: return rose_aarch64_op_cbz;
        case aarch64_op_ccmn_imm: return rose_aarch64_op_ccmn_imm;
        case aarch64_op_ccmn_reg: return rose_aarch64_op_ccmn_reg;
        case aarch64_op_ccmp_imm: return rose_aarch64_op_ccmp_imm;
        case aarch64_op_ccmp_reg: return rose_aarch64_op_ccmp_reg;
        case aarch64_op_cinc_csinc: return rose_aarch64_op_cinc_csinc;
        case aarch64_op_cinv_csinv: return rose_aarch64_op_cinv_csinv;
        case aarch64_op_clrex: return rose_aarch64_op_clrex;
        case aarch64_op_cls_advsimd: return rose_aarch64_op_cls_advsimd;
        case aarch64_op_cls_int: return rose_aarch64_op_cls_int;
        case aarch64_op_clz_advsimd: return rose_aarch64_op_clz_advsimd;
        case aarch64_op_clz_int: return rose_aarch64_op_clz_int;
        case aarch64_op_cmeq_advsimd_reg: return rose_aarch64_op_cmeq_advsimd_reg;
        case aarch64_op_cmeq_advsimd_zero: return rose_aarch64_op_cmeq_advsimd_zero;
        case aarch64_op_cmge_advsimd_reg: return rose_aarch64_op_cmge_advsimd_reg;
        case aarch64_op_cmge_advsimd_zero: return rose_aarch64_op_cmge_advsimd_zero;
        case aarch64_op_cmgt_advsimd_reg: return rose_aarch64_op_cmgt_advsimd_reg;
        case aarch64_op_cmgt_advsimd_zero: return rose_aarch64_op_cmgt_advsimd_zero;
        case aarch64_op_cmhi_advsimd: return rose_aarch64_op_cmhi_advsimd;
        case aarch64_op_cmhs_advsimd: return rose_aarch64_op_cmhs_advsimd;
        case aarch64_op_cmle_advsimd: return rose_aarch64_op_cmle_advsimd;
        case aarch64_op_cmlt_advsimd: return rose_aarch64_op_cmlt_advsimd;
        case aarch64_op_cmn_adds_addsub_ext: return rose_aarch64_op_cmn_adds_addsub_ext;
        case aarch64_op_cmn_adds_addsub_imm: return rose_aarch64_op_cmn_adds_addsub_imm;
        case aarch64_op_cmn_adds_addsub_shift: return rose_aarch64_op_cmn_adds_addsub_shift;
        case aarch64_op_cmp_subs_addsub_ext: return rose_aarch64_op_cmp_subs_addsub_ext;
        case aarch64_op_cmp_subs_addsub_imm: return rose_aarch64_op_cmp_subs_addsub_imm;
        case aarch64_op_cmp_subs_addsub_shift: return rose_aarch64_op_cmp_subs_addsub_shift;
        case aarch64_op_cmtst_advsimd: return rose_aarch64_op_cmtst_advsimd;
        case aarch64_op_cneg_csneg: return rose_aarch64_op_cneg_csneg;
        case aarch64_op_cnt_advsimd: return rose_aarch64_op_cnt_advsimd;
        case aarch64_op_crc32: return rose_aarch64_op_crc32;
        case aarch64_op_crc32c: return rose_aarch64_op_crc32c;
        case aarch64_op_csel: return rose_aarch64_op_csel;
        case aarch64_op_cset_csinc: return rose_aarch64_op_cset_csinc;
        case aarch64_op_csetm_csinv: return rose_aarch64_op_csetm_csinv;
        case aarch64_op_csinc: return rose_aarch64_op_csinc;
        case aarch64_op_csinv: return rose_aarch64_op_csinv;
        case aarch64_op_csneg: return rose_aarch64_op_csneg;
        case aarch64_op_dc_sys: return rose_aarch64_op_dc_sys;
        case aarch64_op_dcps1: return rose_aarch64_op_dcps1;
        case aarch64_op_dcps2: return rose_aarch64_op_dcps2;
        case aarch64_op_dcps3: return rose_aarch64_op_dcps3;
        case aarch64_op_dmb: return rose_aarch64_op_dmb;
        case aarch64_op_drps: return rose_aarch64_op_drps;
        case aarch64_op_dsb: return rose_aarch64_op_dsb;
        case aarch64_op_dup_advsimd_elt: return rose_aarch64_op_dup_advsimd_elt;
        case aarch64_op_dup_advsimd_gen: return rose_aarch64_op_dup_advsimd_gen;
        case aarch64_op_eon: return rose_aarch64_op_eon;
        case aarch64_op_eor_advsimd: return rose_aarch64_op_eor_advsimd;
        case aarch64_op_eor_log_imm: return rose_aarch64_op_eor_log_imm;
        case aarch64_op_eor_log_shift: return rose_aarch64_op_eor_log_shift;
        case aarch64_op_eret: return rose_aarch64_op_eret;
        case aarch64_op_ext_advsimd: return rose_aarch64_op_ext_advsimd;
        case aarch64_op_extr: return rose_aarch64_op_extr;
        case aarch64_op_fabd_advsimd: return rose_aarch64_op_fabd_advsimd;
        case aarch64_op_fabs_advsimd: return rose_aarch64_op_fabs_advsimd;
        case aarch64_op_fabs_float: return rose_aarch64_op_fabs_float;
        case aarch64_op_facge_advsimd: return rose_aarch64_op_facge_advsimd;
        case aarch64_op_facgt_advsimd: return rose_aarch64_op_facgt_advsimd;
        case aarch64_op_fadd_advsimd: return rose_aarch64_op_fadd_advsimd;
        case aarch64_op_fadd_float: return rose_aarch64_op_fadd_float;
        case aarch64_op_faddp_advsimd_pair: return rose_aarch64_op_faddp_advsimd_pair;
        case aarch64_op_faddp_advsimd_vec: return rose_aarch64_op_faddp_advsimd_vec;
        case aarch64_op_fccmp_float: return rose_aarch64_op_fccmp_float;
        case aarch64_op_fccmpe_float: return rose_aarch64_op_fccmpe_float;
        case aarch64_op_fcmeq_advsimd_reg: return rose_aarch64_op_fcmeq_advsimd_reg;
        case aarch64_op_fcmeq_advsimd_zero: return rose_aarch64_op_fcmeq_advsimd_zero;
        case aarch64_op_fcmge_advsimd_reg: return rose_aarch64_op_fcmge_advsimd_reg;
        case aarch64_op_fcmge_advsimd_zero: return rose_aarch64_op_fcmge_advsimd_zero;
        case aarch64_op_fcmgt_advsimd_reg: return rose_aarch64_op_fcmgt_advsimd_reg;
        case aarch64_op_fcmgt_advsimd_zero: return rose_aarch64_op_fcmgt_advsimd_zero;
        case aarch64_op_fcmle_advsimd: return rose_aarch64_op_fcmle_advsimd;
        case aarch64_op_fcmlt_advsimd: return rose_aarch64_op_fcmlt_advsimd;
        case aarch64_op_fcmp_float: return rose_aarch64_op_fcmp_float;
        case aarch64_op_fcmpe_float: return rose_aarch64_op_fcmpe_float;
        case aarch64_op_fcsel_float: return rose_aarch64_op_fcsel_float;
        case aarch64_op_fcvt_float: return rose_aarch64_op_fcvt_float;
        case aarch64_op_fcvtas_advsimd: return rose_aarch64_op_fcvtas_advsimd;
        case aarch64_op_fcvtas_float: return rose_aarch64_op_fcvtas_float;
        case aarch64_op_fcvtau_advsimd: return rose_aarch64_op_fcvtau_advsimd;
        case aarch64_op_fcvtau_float: return rose_aarch64_op_fcvtau_float;
        case aarch64_op_fcvtl_advsimd: return rose_aarch64_op_fcvtl_advsimd;
        case aarch64_op_fcvtms_advsimd: return rose_aarch64_op_fcvtms_advsimd;
        case aarch64_op_fcvtms_float: return rose_aarch64_op_fcvtms_float;
        case aarch64_op_fcvtmu_advsimd: return rose_aarch64_op_fcvtmu_advsimd;
        case aarch64_op_fcvtmu_float: return rose_aarch64_op_fcvtmu_float;
        case aarch64_op_fcvtn_advsimd: return rose_aarch64_op_fcvtn_advsimd;
        case aarch64_op_fcvtns_advsimd: return rose_aarch64_op_fcvtns_advsimd;
        case aarch64_op_fcvtns_float: return rose_aarch64_op_fcvtns_float;
        case aarch64_op_fcvtnu_advsimd: return rose_aarch64_op_fcvtnu_advsimd;
        case aarch64_op_fcvtnu_float: return rose_aarch64_op_fcvtnu_float;
        case aarch64_op_fcvtps_advsimd: return rose_aarch64_op_fcvtps_advsimd;
        case aarch64_op_fcvtps_float: return rose_aarch64_op_fcvtps_float;
        case aarch64_op_fcvtpu_advsimd: return rose_aarch64_op_fcvtpu_advsimd;
        case aarch64_op_fcvtpu_float: return rose_aarch64_op_fcvtpu_float;
        case aarch64_op_fcvtxn_advsimd: return rose_aarch64_op_fcvtxn_advsimd;
        case aarch64_op_fcvtzs_advsimd_fix: return rose_aarch64_op_fcvtzs_advsimd_fix;
        case aarch64_op_fcvtzs_advsimd_int: return rose_aarch64_op_fcvtzs_advsimd_int;
        case aarch64_op_fcvtzs_float_fix: return rose_aarch64_op_fcvtzs_float_fix;
        case aarch64_op_fcvtzs_float_int: return rose_aarch64_op_fcvtzs_float_int;
        case aarch64_op_fcvtzu_advsimd_fix: return rose_aarch64_op_fcvtzu_advsimd_fix;
        case aarch64_op_fcvtzu_advsimd_int: return rose_aarch64_op_fcvtzu_advsimd_int;
        case aarch64_op_fcvtzu_float_fix: return rose_aarch64_op_fcvtzu_float_fix;
        case aarch64_op_fcvtzu_float_int: return rose_aarch64_op_fcvtzu_float_int;
        case aarch64_op_fdiv_advsimd: return rose_aarch64_op_fdiv_advsimd;
        case aarch64_op_fdiv_float: return rose_aarch64_op_fdiv_float;
        case aarch64_op_fmadd_float: return rose_aarch64_op_fmadd_float;
        case aarch64_op_fmax_advsimd: return rose_aarch64_op_fmax_advsimd;
        case aarch64_op_fmax_float: return rose_aarch64_op_fmax_float;
        case aarch64_op_fmaxnm_advsimd: return rose_aarch64_op_fmaxnm_advsimd;
        case aarch64_op_fmaxnm_float: return rose_aarch64_op_fmaxnm_float;
        case aarch64_op_fmaxnmp_advsimd_pair: return rose_aarch64_op_fmaxnmp_advsimd_pair;
        case aarch64_op_fmaxnmp_advsimd_vec: return rose_aarch64_op_fmaxnmp_advsimd_vec;
        case aarch64_op_fmaxnmv_advsimd: return rose_aarch64_op_fmaxnmv_advsimd;
        case aarch64_op_fmaxp_advsimd_pair: return rose_aarch64_op_fmaxp_advsimd_pair;
        case aarch64_op_fmaxp_advsimd_vec: return rose_aarch64_op_fmaxp_advsimd_vec;
        case aarch64_op_fmaxv_advsimd: return rose_aarch64_op_fmaxv_advsimd;
        case aarch64_op_fmin_advsimd: return rose_aarch64_op_fmin_advsimd;
        case aarch64_op_fmin_float: return rose_aarch64_op_fmin_float;
        case aarch64_op_fminnm_advsimd: return rose_aarch64_op_fminnm_advsimd;
        case aarch64_op_fminnm_float: return rose_aarch64_op_fminnm_float;
        case aarch64_op_fminnmp_advsimd_pair: return rose_aarch64_op_fminnmp_advsimd_pair;
        case aarch64_op_fminnmp_advsimd_vec: return rose_aarch64_op_fminnmp_advsimd_vec;
        case aarch64_op_fminnmv_advsimd: return rose_aarch64_op_fminnmv_advsimd;
        case aarch64_op_fminp_advsimd_pair: return rose_aarch64_op_fminp_advsimd_pair;
        case aarch64_op_fminp_advsimd_vec: return rose_aarch64_op_fminp_advsimd_vec;
        case aarch64_op_fminv_advsimd: return rose_aarch64_op_fminv_advsimd;
        case aarch64_op_fmla_advsimd_elt: return rose_aarch64_op_fmla_advsimd_elt;
        case aarch64_op_fmla_advsimd_vec: return rose_aarch64_op_fmla_advsimd_vec;
        case aarch64_op_fmls_advsimd_elt: return rose_aarch64_op_fmls_advsimd_elt;
        case aarch64_op_fmls_advsimd_vec: return rose_aarch64_op_fmls_advsimd_vec;
        case aarch64_op_fmov_advsimd: return rose_aarch64_op_fmov_advsimd;
        case aarch64_op_fmov_float: return rose_aarch64_op_fmov_float;
        case aarch64_op_fmov_float_gen: return rose_aarch64_op_fmov_float_gen;
        case aarch64_op_fmov_float_imm: return rose_aarch64_op_fmov_float_imm;
        case aarch64_op_fmsub_float: return rose_aarch64_op_fmsub_float;
        case aarch64_op_fmul_advsimd_elt: return rose_aarch64_op_fmul_advsimd_elt;
        case aarch64_op_fmul_advsimd_vec: return rose_aarch64_op_fmul_advsimd_vec;
        case aarch64_op_fmul_float: return rose_aarch64_op_fmul_float;
        case aarch64_op_fmulx_advsimd_elt: return rose_aarch64_op_fmulx_advsimd_elt;
        case aarch64_op_fmulx_advsimd_vec: return rose_aarch64_op_fmulx_advsimd_vec;
        case aarch64_op_fneg_advsimd: return rose_aarch64_op_fneg_advsimd;
        case aarch64_op_fneg_float: return rose_aarch64_op_fneg_float;
        case aarch64_op_fnmadd_float: return rose_aarch64_op_fnmadd_float;
        case aarch64_op_fnmsub_float: return rose_aarch64_op_fnmsub_float;
        case aarch64_op_fnmul_float: return rose_aarch64_op_fnmul_float;
        case aarch64_op_frecpe_advsimd: return rose_aarch64_op_frecpe_advsimd;
        case aarch64_op_frecps_advsimd: return rose_aarch64_op_frecps_advsimd;
        case aarch64_op_frecpx_advsimd: return rose_aarch64_op_frecpx_advsimd;
        case aarch64_op_frinta_advsimd: return rose_aarch64_op_frinta_advsimd;
        case aarch64_op_frinta_float: return rose_aarch64_op_frinta_float;
        case aarch64_op_frinti_advsimd: return rose_aarch64_op_frinti_advsimd;
        case aarch64_op_frinti_float: return rose_aarch64_op_frinti_float;
        case aarch64_op_frintm_advsimd: return rose_aarch64_op_frintm_advsimd;
        case aarch64_op_frintm_float: return rose_aarch64_op_frintm_float;
        case aarch64_op_frintn_advsimd: return rose_aarch64_op_frintn_advsimd;
        case aarch64_op_frintn_float: return rose_aarch64_op_frintn_float;
        case aarch64_op_frintp_advsimd: return rose_aarch64_op_frintp_advsimd;
        case aarch64_op_frintp_float: return rose_aarch64_op_frintp_float;
        case aarch64_op_frintx_advsimd: return rose_aarch64_op_frintx_advsimd;
        case aarch64_op_frintx_float: return rose_aarch64_op_frintx_float;
        case aarch64_op_frintz_advsimd: return rose_aarch64_op_frintz_advsimd;
        case aarch64_op_frintz_float: return rose_aarch64_op_frintz_float;
        case aarch64_op_frsqrte_advsimd: return rose_aarch64_op_frsqrte_advsimd;
        case aarch64_op_frsqrts_advsimd: return rose_aarch64_op_frsqrts_advsimd;
        case aarch64_op_fsqrt_advsimd: return rose_aarch64_op_fsqrt_advsimd;
        case aarch64_op_fsqrt_float: return rose_aarch64_op_fsqrt_float;
        case aarch64_op_fsub_advsimd: return rose_aarch64_op_fsub_advsimd;
        case aarch64_op_fsub_float: return rose_aarch64_op_fsub_float;
        case aarch64_op_hint: return rose_aarch64_op_hint;
        case aarch64_op_hlt: return rose_aarch64_op_hlt;
        case aarch64_op_hvc: return rose_aarch64_op_hvc;
        case aarch64_op_ic_sys: return rose_aarch64_op_ic_sys;
        case aarch64_op_ins_advsimd_elt: return rose_aarch64_op_ins_advsimd_elt;
        case aarch64_op_ins_advsimd_gen: return rose_aarch64_op_ins_advsimd_gen;
        case aarch64_op_isb: return rose_aarch64_op_isb;
        case aarch64_op_ld1_advsimd_mult: return rose_aarch64_op_ld1_advsimd_mult;
        case aarch64_op_ld1_advsimd_sngl: return rose_aarch64_op_ld1_advsimd_sngl;
        case aarch64_op_ld1r_advsimd: return rose_aarch64_op_ld1r_advsimd;
        case aarch64_op_ld2_advsimd_mult: return rose_aarch64_op_ld2_advsimd_mult;
        case aarch64_op_ld2_advsimd_sngl: return rose_aarch64_op_ld2_advsimd_sngl;
        case aarch64_op_ld2r_advsimd: return rose_aarch64_op_ld2r_advsimd;
        case aarch64_op_ld3_advsimd_mult: return rose_aarch64_op_ld3_advsimd_mult;
        case aarch64_op_ld3_advsimd_sngl: return rose_aarch64_op_ld3_advsimd_sngl;
        case aarch64_op_ld3r_advsimd: return rose_aarch64_op_ld3r_advsimd;
        case aarch64_op_ld4_advsimd_mult: return rose_aarch64_op_ld4_advsimd_mult;
        case aarch64_op_ld4_advsimd_sngl: return rose_aarch64_op_ld4_advsimd_sngl;
        case aarch64_op_ld4r_advsimd: return rose_aarch64_op_ld4r_advsimd;
        case aarch64_op_ldar: return rose_aarch64_op_ldar;
        case aarch64_op_ldarb: return rose_aarch64_op_ldarb;
        case aarch64_op_ldarh: return rose_aarch64_op_ldarh;
        case aarch64_op_ldaxp: return rose_aarch64_op_ldaxp;
        case aarch64_op_ldaxr: return rose_aarch64_op_ldaxr;
        case aarch64_op_ldaxrb: return rose_aarch64_op_ldaxrb;
        case aarch64_op_ldaxrh: return rose_aarch64_op_ldaxrh;
        case aarch64_op_ldnp_fpsimd: return rose_aarch64_op_ldnp_fpsimd;
        case aarch64_op_ldnp_gen: return rose_aarch64_op_ldnp_gen;
        case aarch64_op_ldp_fpsimd: return rose_aarch64_op_ldp_fpsimd;
        case aarch64_op_ldp_gen: return rose_aarch64_op_ldp_gen;
        case aarch64_op_ldpsw: return rose_aarch64_op_ldpsw;
        case aarch64_op_ldr_imm_fpsimd: return rose_aarch64_op_ldr_imm_fpsimd;
        case aarch64_op_ldr_imm_gen: return rose_aarch64_op_ldr_imm_gen;
        case aarch64_op_ldr_lit_fpsimd: return rose_aarch64_op_ldr_lit_fpsimd;
        case aarch64_op_ldr_lit_gen: return rose_aarch64_op_ldr_lit_gen;
        case aarch64_op_ldr_reg_fpsimd: return rose_aarch64_op_ldr_reg_fpsimd;
        case aarch64_op_ldr_reg_gen: return rose_aarch64_op_ldr_reg_gen;
        case aarch64_op_ldrb_imm: return rose_aarch64_op_ldrb_imm;
        case aarch64_op_ldrb_reg: return rose_aarch64_op_ldrb_reg;
        case aarch64_op_ldrh_imm: return rose_aarch64_op_ldrh_imm;
        case aarch64_op_ldrh_reg: return rose_aarch64_op_ldrh_reg;
        case aarch64_op_ldrsb_imm: return rose_aarch64_op_ldrsb_imm;
        case aarch64_op_ldrsb_reg: return rose_aarch64_op_ldrsb_reg;
        case aarch64_op_ldrsh_imm: return rose_aarch64_op_ldrsh_imm;
        case aarch64_op_ldrsh_reg: return rose_aarch64_op_ldrsh_reg;
        case aarch64_op_ldrsw_imm: return rose_aarch64_op_ldrsw_imm;
        case aarch64_op_ldrsw_lit: return rose_aarch64_op_ldrsw_lit;
        case aarch64_op_ldrsw_reg: return rose_aarch64_op_ldrsw_reg;
        case aarch64_op_ldtr: return rose_aarch64_op_ldtr;
        case aarch64_op_ldtrb: return rose_aarch64_op_ldtrb;
        case aarch64_op_ldtrh: return rose_aarch64_op_ldtrh;
        case aarch64_op_ldtrsb: return rose_aarch64_op_ldtrsb;
        case aarch64_op_ldtrsh: return rose_aarch64_op_ldtrsh;
        case aarch64_op_ldtrsw: return rose_aarch64_op_ldtrsw;
        case aarch64_op_ldur_fpsimd: return rose_aarch64_op_ldur_fpsimd;
        case aarch64_op_ldur_gen: return rose_aarch64_op_ldur_gen;
        case aarch64_op_ldurb: return rose_aarch64_op_ldurb;
        case aarch64_op_ldurh: return rose_aarch64_op_ldurh;
        case aarch64_op_ldursb: return rose_aarch64_op_ldursb;
        case aarch64_op_ldursh: return rose_aarch64_op_ldursh;
        case aarch64_op_ldursw: return rose_aarch64_op_ldursw;
        case aarch64_op_ldxp: return rose_aarch64_op_ldxp;
        case aarch64_op_ldxr: return rose_aarch64_op_ldxr;
        case aarch64_op_ldxrb: return rose_aarch64_op_ldxrb;
        case aarch64_op_ldxrh: return rose_aarch64_op_ldxrh;
        case aarch64_op_lsl_lslv: return rose_aarch64_op_lsl_lslv;
        case aarch64_op_lsl_ubfm: return rose_aarch64_op_lsl_ubfm;
        case aarch64_op_lslv: return rose_aarch64_op_lslv;
        case aarch64_op_lsr_lsrv: return rose_aarch64_op_lsr_lsrv;
        case aarch64_op_lsr_ubfm: return rose_aarch64_op_lsr_ubfm;
        case aarch64_op_lsrv: return rose_aarch64_op_lsrv;
        case aarch64_op_madd: return rose_aarch64_op_madd;
        case aarch64_op_mla_advsimd_elt: return rose_aarch64_op_mla_advsimd_elt;
        case aarch64_op_mla_advsimd_vec: return rose_aarch64_op_mla_advsimd_vec;
        case aarch64_op_mls_advsimd_elt: return rose_aarch64_op_mls_advsimd_elt;
        case aarch64_op_mls_advsimd_vec: return rose_aarch64_op_mls_advsimd_vec;
        case aarch64_op_mneg_msub: return rose_aarch64_op_mneg_msub;
        case aarch64_op_mov_add_addsub_imm: return rose_aarch64_op_mov_add_addsub_imm;
        case aarch64_op_mov_dup_advsimd_elt: return rose_aarch64_op_mov_dup_advsimd_elt;
        case aarch64_op_mov_ins_advsimd_elt: return rose_aarch64_op_mov_ins_advsimd_elt;
        case aarch64_op_mov_ins_advsimd_gen: return rose_aarch64_op_mov_ins_advsimd_gen;
        case aarch64_op_mov_movn: return rose_aarch64_op_mov_movn;
        case aarch64_op_mov_movz: return rose_aarch64_op_mov_movz;
        case aarch64_op_mov_orr_advsimd_reg: return rose_aarch64_op_mov_orr_advsimd_reg;
        case aarch64_op_mov_orr_log_imm: return rose_aarch64_op_mov_orr_log_imm;
        case aarch64_op_mov_orr_log_shift: return rose_aarch64_op_mov_orr_log_shift;
        case aarch64_op_mov_umov_advsimd: return rose_aarch64_op_mov_umov_advsimd;
        case aarch64_op_movi_advsimd: return rose_aarch64_op_movi_advsimd;
        case aarch64_op_movk: return rose_aarch64_op_movk;
        case aarch64_op_movn: return rose_aarch64_op_movn;
        case aarch64_op_movz: return rose_aarch64_op_movz;
        case aarch64_op_mrs: return rose_aarch64_op_mrs;
        case aarch64_op_msr_imm: return rose_aarch64_op_msr_imm;
        case aarch64_op_msr_reg: return rose_aarch64_op_msr_reg;
        case aarch64_op_msub: return rose_aarch64_op_msub;
        case aarch64_op_mul_advsimd_elt: return rose_aarch64_op_mul_advsimd_elt;
        case aarch64_op_mul_advsimd_vec: return rose_aarch64_op_mul_advsimd_vec;
        case aarch64_op_mul_madd: return rose_aarch64_op_mul_madd;
        case aarch64_op_mvn_not_advsimd: return rose_aarch64_op_mvn_not_advsimd;
        case aarch64_op_mvn_orn_log_shift: return rose_aarch64_op_mvn_orn_log_shift;
        case aarch64_op_mvni_advsimd: return rose_aarch64_op_mvni_advsimd;
        case aarch64_op_neg_advsimd: return rose_aarch64_op_neg_advsimd;
        case aarch64_op_neg_sub_addsub_shift: return rose_aarch64_op_neg_sub_addsub_shift;
        case aarch64_op_negs_subs_addsub_shift: return rose_aarch64_op_negs_subs_addsub_shift;
        case aarch64_op_ngc_sbc: return rose_aarch64_op_ngc_sbc;
        case aarch64_op_ngcs_sbcs: return rose_aarch64_op_ngcs_sbcs;
        case aarch64_op_nop_hint: return rose_aarch64_op_nop_hint;
        case aarch64_op_not_advsimd: return rose_aarch64_op_not_advsimd;
        case aarch64_op_orn_advsimd: return rose_aarch64_op_orn_advsimd;
        case aarch64_op_orn_log_shift: return rose_aarch64_op_orn_log_shift;
        case aarch64_op_orr_advsimd_imm: return rose_aarch64_op_orr_advsimd_imm;
        case aarch64_op_orr_advsimd_reg: return rose_aarch64_op_orr_advsimd_reg;
        case aarch64_op_orr_log_imm: return rose_aarch64_op_orr_log_imm;
        case aarch64_op_orr_log_shift: return rose_aarch64_op_orr_log_shift;
        case aarch64_op_pmul_advsimd: return rose_aarch64_op_pmul_advsimd;
        case aarch64_op_pmull_advsimd: return rose_aarch64_op_pmull_advsimd;
        case aarch64_op_prfm_imm: return rose_aarch64_op_prfm_imm;
        case aarch64_op_prfm_lit: return rose_aarch64_op_prfm_lit;
        case aarch64_op_prfm_reg: return rose_aarch64_op_prfm_reg;
        case aarch64_op_prfum: return rose_aarch64_op_prfum;
        case aarch64_op_raddhn_advsimd: return rose_aarch64_op_raddhn_advsimd;
        case aarch64_op_rbit_advsimd: return rose_aarch64_op_rbit_advsimd;
        case aarch64_op_rbit_int: return rose_aarch64_op_rbit_int;
        case aarch64_op_ret: return rose_aarch64_op_ret;
        case aarch64_op_rev: return rose_aarch64_op_rev;
        case aarch64_op_rev16_advsimd: return rose_aarch64_op_rev16_advsimd;
        case aarch64_op_rev16_int: return rose_aarch64_op_rev16_int;
        case aarch64_op_rev32_advsimd: return rose_aarch64_op_rev32_advsimd;
        case aarch64_op_rev32_int: return rose_aarch64_op_rev32_int;
        case aarch64_op_rev64_advsimd: return rose_aarch64_op_rev64_advsimd;
        case aarch64_op_ror_extr: return rose_aarch64_op_ror_extr;
        case aarch64_op_ror_rorv: return rose_aarch64_op_ror_rorv;
        case aarch64_op_rorv: return rose_aarch64_op_rorv;
        case aarch64_op_rshrn_advsimd: return rose_aarch64_op_rshrn_advsimd;
        case aarch64_op_rsubhn_advsimd: return rose_aarch64_op_rsubhn_advsimd;
        case aarch64_op_saba_advsimd: return rose_aarch64_op_saba_advsimd;
        case aarch64_op_sabal_advsimd: return rose_aarch64_op_sabal_advsimd;
        case aarch64_op_sabd_advsimd: return rose_aarch64_op_sabd_advsimd;
        case aarch64_op_sabdl_advsimd: return rose_aarch64_op_sabdl_advsimd;
        case aarch64_op_sadalp_advsimd: return rose_aarch64_op_sadalp_advsimd;
        case aarch64_op_saddl_advsimd: return rose_aarch64_op_saddl_advsimd;
        case aarch64_op_saddlp_advsimd: return rose_aarch64_op_saddlp_advsimd;
        case aarch64_op_saddlv_advsimd: return rose_aarch64_op_saddlv_advsimd;
        case aarch64_op_saddw_advsimd: return rose_aarch64_op_saddw_advsimd;
        case aarch64_op_sbc: return rose_aarch64_op_sbc;
        case aarch64_op_sbcs: return rose_aarch64_op_sbcs;
        case aarch64_op_sbfiz_sbfm: return rose_aarch64_op_sbfiz_sbfm;
        case aarch64_op_sbfm: return rose_aarch64_op_sbfm;
        case aarch64_op_sbfx_sbfm: return rose_aarch64_op_sbfx_sbfm;
        case aarch64_op_scvtf_advsimd_fix: return rose_aarch64_op_scvtf_advsimd_fix;
        case aarch64_op_scvtf_advsimd_int: return rose_aarch64_op_scvtf_advsimd_int;
        case aarch64_op_scvtf_float_fix: return rose_aarch64_op_scvtf_float_fix;
        case aarch64_op_scvtf_float_int: return rose_aarch64_op_scvtf_float_int;
        case aarch64_op_sdiv: return rose_aarch64_op_sdiv;
        case aarch64_op_sev_hint: return rose_aarch64_op_sev_hint;
        case aarch64_op_sevl_hint: return rose_aarch64_op_sevl_hint;
        case aarch64_op_sha1c_advsimd: return rose_aarch64_op_sha1c_advsimd;
        case aarch64_op_sha1h_advsimd: return rose_aarch64_op_sha1h_advsimd;
        case aarch64_op_sha1m_advsimd: return rose_aarch64_op_sha1m_advsimd;
        case aarch64_op_sha1p_advsimd: return rose_aarch64_op_sha1p_advsimd;
        case aarch64_op_sha1su0_advsimd: return rose_aarch64_op_sha1su0_advsimd;
        case aarch64_op_sha1su1_advsimd: return rose_aarch64_op_sha1su1_advsimd;
        case aarch64_op_sha256h2_advsimd: return rose_aarch64_op_sha256h2_advsimd;
        case aarch64_op_sha256h_advsimd: return rose_aarch64_op_sha256h_advsimd;
        case aarch64_op_sha256su0_advsimd: return rose_aarch64_op_sha256su0_advsimd;
        case aarch64_op_sha256su1_advsimd: return rose_aarch64_op_sha256su1_advsimd;
        case aarch64_op_shadd_advsimd: return rose_aarch64_op_shadd_advsimd;
        case aarch64_op_shl_advsimd: return rose_aarch64_op_shl_advsimd;
        case aarch64_op_shll_advsimd: return rose_aarch64_op_shll_advsimd;
        case aarch64_op_shrn_advsimd: return rose_aarch64_op_shrn_advsimd;
        case aarch64_op_shsub_advsimd: return rose_aarch64_op_shsub_advsimd;
        case aarch64_op_sli_advsimd: return rose_aarch64_op_sli_advsimd;
        case aarch64_op_smaddl: return rose_aarch64_op_smaddl;
        case aarch64_op_smax_advsimd: return rose_aarch64_op_smax_advsimd;
        case aarch64_op_smaxp_advsimd: return rose_aarch64_op_smaxp_advsimd;
        case aarch64_op_smaxv_advsimd: return rose_aarch64_op_smaxv_advsimd;
        case aarch64_op_smc: return rose_aarch64_op_smc;
        case aarch64_op_smin_advsimd: return rose_aarch64_op_smin_advsimd;
        case aarch64_op_sminp_advsimd: return rose_aarch64_op_sminp_advsimd;
        case aarch64_op_sminv_advsimd: return rose_aarch64_op_sminv_advsimd;
        case aarch64_op_smlal_advsimd_elt: return rose_aarch64_op_smlal_advsimd_elt;
        case aarch64_op_smlal_advsimd_vec: return rose_aarch64_op_smlal_advsimd_vec;
        case aarch64_op_smlsl_advsimd_elt: return rose_aarch64_op_smlsl_advsimd_elt;
        case aarch64_op_smlsl_advsimd_vec: return rose_aarch64_op_smlsl_advsimd_vec;
        case aarch64_op_smnegl_smsubl: return rose_aarch64_op_smnegl_smsubl;
        case aarch64_op_smov_advsimd: return rose_aarch64_op_smov_advsimd;
        case aarch64_op_smsubl: return rose_aarch64_op_smsubl;
        case aarch64_op_smulh: return rose_aarch64_op_smulh;
        case aarch64_op_smull_advsimd_elt: return rose_aarch64_op_smull_advsimd_elt;
        case aarch64_op_smull_advsimd_vec: return rose_aarch64_op_smull_advsimd_vec;
        case aarch64_op_smull_smaddl: return rose_aarch64_op_smull_smaddl;
        case aarch64_op_sqabs_advsimd: return rose_aarch64_op_sqabs_advsimd;
        case aarch64_op_sqadd_advsimd: return rose_aarch64_op_sqadd_advsimd;
        case aarch64_op_sqdmlal_advsimd_elt: return rose_aarch64_op_sqdmlal_advsimd_elt;
        case aarch64_op_sqdmlal_advsimd_vec: return rose_aarch64_op_sqdmlal_advsimd_vec;
        case aarch64_op_sqdmlsl_advsimd_elt: return rose_aarch64_op_sqdmlsl_advsimd_elt;
        case aarch64_op_sqdmlsl_advsimd_vec: return rose_aarch64_op_sqdmlsl_advsimd_vec;
        case aarch64_op_sqdmulh_advsimd_elt: return rose_aarch64_op_sqdmulh_advsimd_elt;
        case aarch64_op_sqdmulh_advsimd_vec: return rose_aarch64_op_sqdmulh_advsimd_vec;
        case aarch64_op_sqdmull_advsimd_elt: return rose_aarch64_op_sqdmull_advsimd_elt;
        case aarch64_op_sqdmull_advsimd_vec: return rose_aarch64_op_sqdmull_advsimd_vec;
        case aarch64_op_sqneg_advsimd: return rose_aarch64_op_sqneg_advsimd;
        case aarch64_op_sqrdmulh_advsimd_elt: return rose_aarch64_op_sqrdmulh_advsimd_elt;
        case aarch64_op_sqrdmulh_advsimd_vec: return rose_aarch64_op_sqrdmulh_advsimd_vec;
        case aarch64_op_sqrshl_advsimd: return rose_aarch64_op_sqrshl_advsimd;
        case aarch64_op_sqrshrn_advsimd: return rose_aarch64_op_sqrshrn_advsimd;
        case aarch64_op_sqrshrun_advsimd: return rose_aarch64_op_sqrshrun_advsimd;
        case aarch64_op_sqshl_advsimd_imm: return rose_aarch64_op_sqshl_advsimd_imm;
        case aarch64_op_sqshl_advsimd_reg: return rose_aarch64_op_sqshl_advsimd_reg;
        case aarch64_op_sqshlu_advsimd: return rose_aarch64_op_sqshlu_advsimd;
        case aarch64_op_sqshrn_advsimd: return rose_aarch64_op_sqshrn_advsimd;
        case aarch64_op_sqshrun_advsimd: return rose_aarch64_op_sqshrun_advsimd;
        case aarch64_op_sqsub_advsimd: return rose_aarch64_op_sqsub_advsimd;
        case aarch64_op_sqxtn_advsimd: return rose_aarch64_op_sqxtn_advsimd;
        case aarch64_op_sqxtun_advsimd: return rose_aarch64_op_sqxtun_advsimd;
        case aarch64_op_srhadd_advsimd: return rose_aarch64_op_srhadd_advsimd;
        case aarch64_op_sri_advsimd: return rose_aarch64_op_sri_advsimd;
        case aarch64_op_srshl_advsimd: return rose_aarch64_op_srshl_advsimd;
        case aarch64_op_srshr_advsimd: return rose_aarch64_op_srshr_advsimd;
        case aarch64_op_srsra_advsimd: return rose_aarch64_op_srsra_advsimd;
        case aarch64_op_sshl_advsimd: return rose_aarch64_op_sshl_advsimd;
        case aarch64_op_sshll_advsimd: return rose_aarch64_op_sshll_advsimd;
        case aarch64_op_sshr_advsimd: return rose_aarch64_op_sshr_advsimd;
        case aarch64_op_ssra_advsimd: return rose_aarch64_op_ssra_advsimd;
        case aarch64_op_ssubl_advsimd: return rose_aarch64_op_ssubl_advsimd;
        case aarch64_op_ssubw_advsimd: return rose_aarch64_op_ssubw_advsimd;
        case aarch64_op_st1_advsimd_mult: return rose_aarch64_op_st1_advsimd_mult;
        case aarch64_op_st1_advsimd_sngl: return rose_aarch64_op_st1_advsimd_sngl;
        case aarch64_op_st2_advsimd_mult: return rose_aarch64_op_st2_advsimd_mult;
        case aarch64_op_st2_advsimd_sngl: return rose_aarch64_op_st2_advsimd_sngl;
        case aarch64_op_st3_advsimd_mult: return rose_aarch64_op_st3_advsimd_mult;
        case aarch64_op_st3_advsimd_sngl: return rose_aarch64_op_st3_advsimd_sngl;
        case aarch64_op_st4_advsimd_mult: return rose_aarch64_op_st4_advsimd_mult;
        case aarch64_op_st4_advsimd_sngl: return rose_aarch64_op_st4_advsimd_sngl;
        case aarch64_op_stlr: return rose_aarch64_op_stlr;
        case aarch64_op_stlrb: return rose_aarch64_op_stlrb;
        case aarch64_op_stlrh: return rose_aarch64_op_stlrh;
        case aarch64_op_stlxp: return rose_aarch64_op_stlxp;
        case aarch64_op_stlxr: return rose_aarch64_op_stlxr;
        case aarch64_op_stlxrb: return rose_aarch64_op_stlxrb;
        case aarch64_op_stlxrh: return rose_aarch64_op_stlxrh;
        case aarch64_op_stnp_fpsimd: return rose_aarch64_op_stnp_fpsimd;
        case aarch64_op_stnp_gen: return rose_aarch64_op_stnp_gen;
        case aarch64_op_stp_fpsimd: return rose_aarch64_op_stp_fpsimd;
        case aarch64_op_stp_gen: return rose_aarch64_op_stp_gen;
        case aarch64_op_str_imm_fpsimd: return rose_aarch64_op_str_imm_fpsimd;
        case aarch64_op_str_imm_gen: return rose_aarch64_op_str_imm_gen;
        case aarch64_op_str_reg_fpsimd: return rose_aarch64_op_str_reg_fpsimd;
        case aarch64_op_str_reg_gen: return rose_aarch64_op_str_reg_gen;
        case aarch64_op_strb_imm: return rose_aarch64_op_strb_imm;
        case aarch64_op_strb_reg: return rose_aarch64_op_strb_reg;
        case aarch64_op_strh_imm: return rose_aarch64_op_strh_imm;
        case aarch64_op_strh_reg: return rose_aarch64_op_strh_reg;
        case aarch64_op_sttr: return rose_aarch64_op_sttr;
        case aarch64_op_sttrb: return rose_aarch64_op_sttrb;
        case aarch64_op_sttrh: return rose_aarch64_op_sttrh;
        case aarch64_op_stur_fpsimd: return rose_aarch64_op_stur_fpsimd;
        case aarch64_op_stur_gen: return rose_aarch64_op_stur_gen;
        case aarch64_op_sturb: return rose_aarch64_op_sturb;
        case aarch64_op_sturh: return rose_aarch64_op_sturh;
        case aarch64_op_stxp: return rose_aarch64_op_stxp;
        case aarch64_op_stxr: return rose_aarch64_op_stxr;
        case aarch64_op_stxrb: return rose_aarch64_op_stxrb;
        case aarch64_op_stxrh: return rose_aarch64_op_stxrh;
        case aarch64_op_sub_addsub_ext: return rose_aarch64_op_sub_addsub_ext;
        case aarch64_op_sub_addsub_imm: return rose_aarch64_op_sub_addsub_imm;
        case aarch64_op_sub_addsub_shift: return rose_aarch64_op_sub_addsub_shift;
        case aarch64_op_sub_advsimd: return rose_aarch64_op_sub_advsimd;
        case aarch64_op_subhn_advsimd: return rose_aarch64_op_subhn_advsimd;
        case aarch64_op_subs_addsub_ext: return rose_aarch64_op_subs_addsub_ext;
        case aarch64_op_subs_addsub_imm: return rose_aarch64_op_subs_addsub_imm;
        case aarch64_op_subs_addsub_shift: return rose_aarch64_op_subs_addsub_shift;
        case aarch64_op_suqadd_advsimd: return rose_aarch64_op_suqadd_advsimd;
        case aarch64_op_svc: return rose_aarch64_op_svc;
        case aarch64_op_sxtb_sbfm: return rose_aarch64_op_sxtb_sbfm;
        case aarch64_op_sxth_sbfm: return rose_aarch64_op_sxth_sbfm;
        case aarch64_op_sxtl_sshll_advsimd: return rose_aarch64_op_sxtl_sshll_advsimd;
        case aarch64_op_sxtw_sbfm: return rose_aarch64_op_sxtw_sbfm;
        case aarch64_op_sys: return rose_aarch64_op_sys;
        case aarch64_op_sysl: return rose_aarch64_op_sysl;
        case aarch64_op_tbl_advsimd: return rose_aarch64_op_tbl_advsimd;
        case aarch64_op_tbnz: return rose_aarch64_op_tbnz;
        case aarch64_op_tbx_advsimd: return rose_aarch64_op_tbx_advsimd;
        case aarch64_op_tbz: return rose_aarch64_op_tbz;
        case aarch64_op_tlbi_sys: return rose_aarch64_op_tlbi_sys;
        case aarch64_op_trn1_advsimd: return rose_aarch64_op_trn1_advsimd;
        case aarch64_op_trn2_advsimd: return rose_aarch64_op_trn2_advsimd;
        case aarch64_op_tst_ands_log_imm: return rose_aarch64_op_tst_ands_log_imm;
        case aarch64_op_tst_ands_log_shift: return rose_aarch64_op_tst_ands_log_shift;
        case aarch64_op_uaba_advsimd: return rose_aarch64_op_uaba_advsimd;
        case aarch64_op_uabal_advsimd: return rose_aarch64_op_uabal_advsimd;
        case aarch64_op_uabd_advsimd: return rose_aarch64_op_uabd_advsimd;
        case aarch64_op_uabdl_advsimd: return rose_aarch64_op_uabdl_advsimd;
        case aarch64_op_uadalp_advsimd: return rose_aarch64_op_uadalp_advsimd;
        case aarch64_op_uaddl_advsimd: return rose_aarch64_op_uaddl_advsimd;
        case aarch64_op_uaddlp_advsimd: return rose_aarch64_op_uaddlp_advsimd;
        case aarch64_op_uaddlv_advsimd: return rose_aarch64_op_uaddlv_advsimd;
        case aarch64_op_uaddw_advsimd: return rose_aarch64_op_uaddw_advsimd;
        case aarch64_op_ubfiz_ubfm: return rose_aarch64_op_ubfiz_ubfm;
        case aarch64_op_ubfm: return rose_aarch64_op_ubfm;
        case aarch64_op_ubfx_ubfm: return rose_aarch64_op_ubfx_ubfm;
        case aarch64_op_ucvtf_advsimd_fix: return rose_aarch64_op_ucvtf_advsimd_fix;
        case aarch64_op_ucvtf_advsimd_int: return rose_aarch64_op_ucvtf_advsimd_int;
        case aarch64_op_ucvtf_float_fix: return rose_aarch64_op_ucvtf_float_fix;
        case aarch64_op_ucvtf_float_int: return rose_aarch64_op_ucvtf_float_int;
        case aarch64_op_udiv: return rose_aarch64_op_udiv;
        case aarch64_op_uhadd_advsimd: return rose_aarch64_op_uhadd_advsimd;
        case aarch64_op_uhsub_advsimd: return rose_aarch64_op_uhsub_advsimd;
        case aarch64_op_umaddl: return rose_aarch64_op_umaddl;
        case aarch64_op_umax_advsimd: return rose_aarch64_op_umax_advsimd;
        case aarch64_op_umaxp_advsimd: return rose_aarch64_op_umaxp_advsimd;
        case aarch64_op_umaxv_advsimd: return rose_aarch64_op_umaxv_advsimd;
        case aarch64_op_umin_advsimd: return rose_aarch64_op_umin_advsimd;
        case aarch64_op_uminp_advsimd: return rose_aarch64_op_uminp_advsimd;
        case aarch64_op_uminv_advsimd: return rose_aarch64_op_uminv_advsimd;
        case aarch64_op_umlal_advsimd_elt: return rose_aarch64_op_umlal_advsimd_elt;
        case aarch64_op_umlal_advsimd_vec: return rose_aarch64_op_umlal_advsimd_vec;
        case aarch64_op_umlsl_advsimd_elt: return rose_aarch64_op_umlsl_advsimd_elt;
        case aarch64_op_umlsl_advsimd_vec: return rose_aarch64_op_umlsl_advsimd_vec;
        case aarch64_op_umnegl_umsubl: return rose_aarch64_op_umnegl_umsubl;
        case aarch64_op_umov_advsimd: return rose_aarch64_op_umov_advsimd;
        case aarch64_op_umsubl: return rose_aarch64_op_umsubl;
        case aarch64_op_umulh: return rose_aarch64_op_umulh;
        case aarch64_op_umull_advsimd_elt: return rose_aarch64_op_umull_advsimd_elt;
        case aarch64_op_umull_advsimd_vec: return rose_aarch64_op_umull_advsimd_vec;
        case aarch64_op_umull_umaddl: return rose_aarch64_op_umull_umaddl;
        case aarch64_op_uqadd_advsimd: return rose_aarch64_op_uqadd_advsimd;
        case aarch64_op_uqrshl_advsimd: return rose_aarch64_op_uqrshl_advsimd;
        case aarch64_op_uqrshrn_advsimd: return rose_aarch64_op_uqrshrn_advsimd;
        case aarch64_op_uqshl_advsimd_imm: return rose_aarch64_op_uqshl_advsimd_imm;
        case aarch64_op_uqshl_advsimd_reg: return rose_aarch64_op_uqshl_advsimd_reg;
        case aarch64_op_uqshrn_advsimd: return rose_aarch64_op_uqshrn_advsimd;
        case aarch64_op_uqsub_advsimd: return rose_aarch64_op_uqsub_advsimd;
        case aarch64_op_uqxtn_advsimd: return rose_aarch64_op_uqxtn_advsimd;
        case aarch64_op_urecpe_advsimd: return rose_aarch64_op_urecpe_advsimd;
        case aarch64_op_urhadd_advsimd: return rose_aarch64_op_urhadd_advsimd;
        case aarch64_op_urshl_advsimd: return rose_aarch64_op_urshl_advsimd;
        case aarch64_op_urshr_advsimd: return rose_aarch64_op_urshr_advsimd;
        case aarch64_op_ursqrte_advsimd: return rose_aarch64_op_ursqrte_advsimd;
        case aarch64_op_ursra_advsimd: return rose_aarch64_op_ursra_advsimd;
        case aarch64_op_ushl_advsimd: return rose_aarch64_op_ushl_advsimd;
        case aarch64_op_ushll_advsimd: return rose_aarch64_op_ushll_advsimd;
        case aarch64_op_ushr_advsimd: return rose_aarch64_op_ushr_advsimd;
        case aarch64_op_usqadd_advsimd: return rose_aarch64_op_usqadd_advsimd;
        case aarch64_op_usra_advsimd: return rose_aarch64_op_usra_advsimd;
        case aarch64_op_usubl_advsimd: return rose_aarch64_op_usubl_advsimd;
        case aarch64_op_usubw_advsimd: return rose_aarch64_op_usubw_advsimd;
        case aarch64_op_uxtb_ubfm: return rose_aarch64_op_uxtb_ubfm;
        case aarch64_op_uxth_ubfm: return rose_aarch64_op_uxth_ubfm;
        case aarch64_op_uxtl_ushll_advsimd: return rose_aarch64_op_uxtl_ushll_advsimd;
        case aarch64_op_uzp1_advsimd: return rose_aarch64_op_uzp1_advsimd;
        case aarch64_op_uzp2_advsimd: return rose_aarch64_op_uzp2_advsimd;
        case aarch64_op_wfe_hint: return rose_aarch64_op_wfe_hint;
        case aarch64_op_wfi_hint: return rose_aarch64_op_wfi_hint;
        case aarch64_op_xtn_advsimd: return rose_aarch64_op_xtn_advsimd;
        case aarch64_op_yield_hint: return rose_aarch64_op_yield_hint;
        case aarch64_op_zip1_advsimd: return rose_aarch64_op_zip1_advsimd;
        case aarch64_op_zip2_advsimd: return rose_aarch64_op_zip2_advsimd;
        default: return rose_aarch64_op_INVALID;
    }
}
// TODO:
// This function should just translate each instructionAPI opcode to a rose equivalent version
AmdgpuVegaInstructionKind RoseInsnAmdgpuVegaFactory::convertKind(entryID opcode) {
    switch(opcode) {
        case amdgpu_op_s_setpc_b64 : return rose_amdgpu_op_s_setpc_b64;
        case amdgpu_op_s_swappc_b64 : return rose_amdgpu_op_s_swappc_b64;
        case amdgpu_op_s_getpc_b64 : return rose_amdgpu_op_s_getpc_b64;
        case amdgpu_op_s_add_u32 : return rose_amdgpu_op_s_add_u32;
        case amdgpu_op_s_addc_u32 : return rose_amdgpu_op_s_addc_u32;
        default: return rose_amdgpu_op_INVALID;
    }
}
