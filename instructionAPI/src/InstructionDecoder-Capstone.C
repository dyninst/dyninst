#include "InstructionDecoder-Capstone.h"
#include "string"

#include <boost/assign/list_of.hpp>

using boost::assign::map_list_of;

namespace Dyninst {

namespace InstructionAPI {


/**************  Architecture independent functions  **********************/
dyn_tls bool InstructionDecoder_Capstone::handle_init = false;
dyn_tls std::map<std::string, std::string>* InstructionDecoder_Capstone::opcode_alias = nullptr;
dyn_tls dyn_hash_map<entryID, std::string>* InstructionDecoder_Capstone::opcode_str = nullptr;
dyn_tls cs_insn* InstructionDecoder_Capstone::capstone_ins_no_detail = nullptr;
dyn_tls cs_insn* InstructionDecoder_Capstone::capstone_ins_with_detail = nullptr;
dyn_tls csh InstructionDecoder_Capstone::handle_no_detail;
dyn_tls csh InstructionDecoder_Capstone::handle_with_detail;


InstructionDecoder_Capstone::InstructionDecoder_Capstone(Architecture a):
    InstructionDecoderImpl(a)
    {}


bool InstructionDecoder_Capstone::openCapstoneHandle() {
    if (handle_init) return true;
    cs_err ret1, ret2;    
    handle_init = true;

    initializeOpcodeData();



    switch (m_Arch) {
        case Arch_x86: 
            ret1 = cs_open(CS_ARCH_X86, CS_MODE_32, &handle_no_detail);
            ret2 = cs_open(CS_ARCH_X86, CS_MODE_32, &handle_with_detail);
            break;
        case Arch_x86_64:
            ret1 = cs_open(CS_ARCH_X86, CS_MODE_64, &handle_no_detail);
            ret2 = cs_open(CS_ARCH_X86, CS_MODE_64, &handle_with_detail);
            break;
        case Arch_ppc32:
            ret1 = cs_open(CS_ARCH_PPC, CS_MODE_32, &handle_no_detail);
            ret2 = cs_open(CS_ARCH_PPC, CS_MODE_32, &handle_with_detail);
            break;
        case Arch_ppc64:
            ret1 = cs_open(CS_ARCH_PPC, CS_MODE_64, &handle_no_detail);
            ret2 = cs_open(CS_ARCH_PPC, CS_MODE_64, &handle_with_detail);
            break;
        case Arch_aarch64:
            ret1 = cs_open(CS_ARCH_ARM64, CS_MODE_ARM, &handle_no_detail);
            ret2 = cs_open(CS_ARCH_ARM64, CS_MODE_ARM, &handle_with_detail);
            break;
        default:
            ret1 = ret2 = CS_ERR_ARCH;
            break;
    }

    if (ret1 == CS_ERR_OK && ret2 == CS_ERR_OK) {
        cs_option(handle_no_detail, CS_OPT_DETAIL, CS_OPT_OFF); 
        capstone_ins_no_detail = cs_malloc(handle_no_detail);
        cs_option(handle_with_detail, CS_OPT_DETAIL, CS_OPT_ON); 
        capstone_ins_with_detail = cs_malloc(handle_with_detail);
        return true;
    }
    return false;
}

void InstructionDecoder_Capstone::doDelayedDecode(const Instruction* insn) {
    if (!openCapstoneHandle()) {
        return;
    }
    const unsigned char* code = (const unsigned char*) insn->ptr();
    size_t codeSize = insn->size();
    uint64_t cap_addr = 0;
    if (cs_disasm_iter(handle_with_detail, &code, &codeSize, &cap_addr, capstone_ins_with_detail)) {
        if (m_Arch == Arch_x86 || m_Arch == Arch_x86_64)
            decodeOperands_x86(insn, capstone_ins_with_detail->detail);
        else if (m_Arch == Arch_ppc32 || m_Arch == Arch_ppc64)
            decodeOperands_ppc(insn, capstone_ins_with_detail->detail);
        else if (m_Arch == Arch_aarch64) 
            decodeOperands_aarch64(insn, capstone_ins_with_detail->detail); 
    }
}
bool InstructionDecoder_Capstone::decodeOperands(const Instruction* insn) {
    return false;
}

std::string InstructionDecoder_Capstone::mnemonicNormalization(std::string m) {
    if (opcode_alias->find(m) == opcode_alias->end())
        return m;
    else
        return (*opcode_alias)[m];
}

void InstructionDecoder_Capstone::decodeOpcode(InstructionDecoder::buffer& buf) {
    if (!openCapstoneHandle()) {
        m_Operation = Operation(e_No_Entry, "INVALID", m_Arch);
        return;
    }
    const unsigned char* code = buf.start;
    size_t codeSize = buf.end - buf.start;
    uint64_t cap_addr = 0;
    if (cs_disasm_iter(handle_no_detail, &code, &codeSize, &cap_addr, capstone_ins_no_detail)) {
        entryID e = opcodeTranslation(capstone_ins_no_detail->id);

        std::string opcodeString = "INVALID";
        auto strit = opcode_str->find(e);
        if (strit != opcode_str->end())
            opcodeString = strit->second;
        opcodeString = mnemonicNormalization(opcodeString);

        m_Operation = Operation(e, opcodeString, m_Arch);
        buf.start += capstone_ins_no_detail->size;
	} else
        m_Operation = Operation(e_No_Entry, "INVALID", m_Arch);
}

entryID InstructionDecoder_Capstone::opcodeTranslation(unsigned int cap_id) {
    if (m_Arch == Arch_x86 || m_Arch == Arch_x86_64)
        return opcodeTranslation_x86(cap_id);
    else if (m_Arch == Arch_ppc32 || m_Arch == Arch_ppc64)
        return opcodeTranslation_ppc(cap_id);
    else if (m_Arch == Arch_aarch64) 
        return opcodeTranslation_aarch64(cap_id);
    else {
        fprintf(stderr, "Unsupported architecture\n");
        return e_No_Entry; 
    }
}

Result_Type InstructionDecoder_Capstone::operandSizeTranslation(uint8_t cap_size) {
    switch (cap_size) {
        case 1:  return u8;
        case 2:  return u16;
        case 4:  return u32;
        case 8:  return u64;
        case 10: return m80;
        case 12: return m96;
        case 14: return m14;
        case 16: return dbl128; 
        case 24: return m192;
        case 28: return m224;
        case 32: return m256;
        case 48: return m384;
        case 64: return m512;
        default:
            return invalid_type;
    }
}

bool InstructionDecoder_Capstone::checkCapstoneGroup(cs_detail *d, uint8_t g) {
    for (uint8_t i = 0; i < d->groups_count; ++i)
        if (d->groups[i] == g)
            return true;
    return false;
}


void InstructionDecoder_Capstone::initializeOpcodeData() {
    
    opcode_alias = new std::map<std::string, std::string>();
    (*opcode_alias)["ja"] = "jnbe";
    (*opcode_alias)["jae"] = "jnb";
    (*opcode_alias)["je"] = "jz";
    (*opcode_alias)["jne"] = "jnz";
    (*opcode_alias)["jg"] = "jnle";
    (*opcode_alias)["jge"] = "jnl";

    opcode_str = new dyn_hash_map<entryID, std::string>();
    (*opcode_str) =  map_list_of
    (e_aaa,"aaa")
    (e_aad,"aad")
    (e_aam,"aam")
    (e_aas,"aas")
    (e_fabs,"fabs")
    (e_adc,"adc")
    (e_adcx,"adcx")
    (e_add,"add")
    (e_addpd,"addpd")
    (e_addps,"addps")
    (e_addsd,"addsd")
    (e_addss,"addss")
    (e_addsubpd,"addsubpd")
    (e_addsubps,"addsubps")
    (e_fadd,"fadd")
    (e_fiadd,"fiadd")
    (e_adox,"adox")
    (e_aesdeclast,"aesdeclast")
    (e_aesdec,"aesdec")
    (e_aesenclast,"aesenclast")
    (e_aesenc,"aesenc")
    (e_aesimc,"aesimc")
    (e_aeskeygenassist,"aeskeygenassist")
    (e_and,"and")
    (e_andn,"andn")
    (e_andnpd,"andnpd")
    (e_andnps,"andnps")
    (e_andpd,"andpd")
    (e_andps,"andps")
    (e_arpl,"arpl")
    (e_bextr,"bextr")
    (e_blcfill,"blcfill")
    (e_blci,"blci")
    (e_blcic,"blcic")
    (e_blcmsk,"blcmsk")
    (e_blcs,"blcs")
    (e_blendpd,"blendpd")
    (e_blendps,"blendps")
    (e_blendvpd,"blendvpd")
    (e_blendvps,"blendvps")
    (e_blsfill,"blsfill")
    (e_blsi,"blsi")
    (e_blsic,"blsic")
    (e_blsmsk,"blsmsk")
    (e_blsr,"blsr")
    (e_bndcl,"bndcl")
    (e_bndcn,"bndcn")
    (e_bndcu,"bndcu")
    (e_bndldx,"bndldx")
    (e_bndmk,"bndmk")
    (e_bndmov,"bndmov")
    (e_bndstx,"bndstx")
    (e_bound,"bound")
    (e_bsf,"bsf")
    (e_bsr,"bsr")
    (e_bswap,"bswap")
    (e_bt,"bt")
    (e_btc,"btc")
    (e_btr,"btr")
    (e_bts,"bts")
    (e_bzhi,"bzhi")
    (e_call,"call")
    (e_cbw,"cbw")
    (e_cdq,"cdq")
    (e_cdqe,"cdqe")
    (e_fchs,"fchs")
    (e_clac,"clac")
    (e_clc,"clc")
    (e_cld,"cld")
    (e_cldemote,"cldemote")
    (e_clflush,"clflush")
    (e_clflushopt,"clflushopt")
    (e_clgi,"clgi")
    (e_cli,"cli")
    (e_clrssbsy,"clrssbsy")
    (e_clts,"clts")
    (e_clwb,"clwb")
    (e_clzero,"clzero")
    (e_cmc,"cmc")
    (e_cmova,"cmova")
    (e_cmovae,"cmovae")
    (e_cmovb,"cmovb")
    (e_cmovbe,"cmovbe")
    (e_fcmovbe,"fcmovbe")
    (e_fcmovb,"fcmovb")
    (e_cmove,"cmove")
    (e_fcmove,"fcmove")
    (e_cmovg,"cmovg")
    (e_cmovge,"cmovge")
    (e_cmovl,"cmovl")
    (e_cmovle,"cmovle")
    (e_fcmovnbe,"fcmovnbe")
    (e_fcmovnb,"fcmovnb")
    (e_cmovne,"cmovne")
    (e_fcmovne,"fcmovne")
    (e_cmovno,"cmovno")
    (e_cmovnp,"cmovnp")
    (e_fcmovnu,"fcmovnu")
    (e_fcmovnp,"fcmovnp")
    (e_cmovns,"cmovns")
    (e_cmovo,"cmovo")
    (e_cmovp,"cmovp")
    (e_fcmovu,"fcmovu")
    (e_cmovs,"cmovs")
    (e_cmp,"cmp")
    (e_cmppd,"cmppd")
    (e_cmpps,"cmpps")
    (e_cmpsb,"cmpsb")
    (e_cmpsd,"cmpsd")
    (e_cmpsq,"cmpsq")
    (e_cmpss,"cmpss")
    (e_cmpsw,"cmpsw")
    (e_cmpxchg16b,"cmpxchg16b")
    (e_cmpxchg,"cmpxchg")
    (e_cmpxchg8b,"cmpxchg8b")
    (e_comisd,"comisd")
    (e_comiss,"comiss")
    (e_fcomp,"fcomp")
    (e_fcompi,"fcompi")
    (e_fcomi,"fcomi")
    (e_fcom,"fcom")
    (e_fcos,"fcos")
    (e_cpuid,"cpuid")
    (e_cqo,"cqo")
    (e_crc32,"crc32")
    (e_cvtdq2pd,"cvtdq2pd")
    (e_cvtdq2ps,"cvtdq2ps")
    (e_cvtpd2dq,"cvtpd2dq")
    (e_cvtpd2ps,"cvtpd2ps")
    (e_cvtps2dq,"cvtps2dq")
    (e_cvtps2pd,"cvtps2pd")
    (e_cvtsd2si,"cvtsd2si")
    (e_cvtsd2ss,"cvtsd2ss")
    (e_cvtsi2sd,"cvtsi2sd")
    (e_cvtsi2ss,"cvtsi2ss")
    (e_cvtss2sd,"cvtss2sd")
    (e_cvtss2si,"cvtss2si")
    (e_cvttpd2dq,"cvttpd2dq")
    (e_cvttps2dq,"cvttps2dq")
    (e_cvttsd2si,"cvttsd2si")
    (e_cvttss2si,"cvttss2si")
    (e_cwd,"cwd")
    (e_cwde,"cwde")
    (e_daa,"daa")
    (e_das,"das")
    (e_data16,"data16")
    (e_dec,"dec")
    (e_div,"div")
    (e_divpd,"divpd")
    (e_divps,"divps")
    (e_fdivr,"fdivr")
    (e_fidivr,"fidivr")
    (e_fdivrp,"fdivrp")
    (e_divsd,"divsd")
    (e_divss,"divss")
    (e_fdiv,"fdiv")
    (e_fidiv,"fidiv")
    (e_fdivp,"fdivp")
    (e_dppd,"dppd")
    (e_dpps,"dpps")
    (e_encls,"encls")
    (e_enclu,"enclu")
    (e_enclv,"enclv")
    (e_endbr32,"endbr32")
    (e_endbr64,"endbr64")
    (e_enter,"enter")
    (e_extractps,"extractps")
    (e_extrq,"extrq")
    (e_f2xm1,"f2xm1")
    (e_lcall,"lcall")
    (e_ljmp,"ljmp")
    (e_jmp,"jmp")
    (e_fbld,"fbld")
    (e_fbstp,"fbstp")
    (e_fcompp,"fcompp")
    (e_fdecstp,"fdecstp")
    (e_nop,"nop")
    (e_femms,"femms")
    (e_nop,"nop")
    (e_ffree,"ffree")
    (e_ffreep,"ffreep")
    (e_ficom,"ficom")
    (e_ficomp,"ficomp")
    (e_fincstp,"fincstp")
    (e_fldcw,"fldcw")
    (e_fldenv,"fldenv")
    (e_fldl2e,"fldl2e")
    (e_fldl2t,"fldl2t")
    (e_fldlg2,"fldlg2")
    (e_fldln2,"fldln2")
    (e_fldpi,"fldpi")
    (e_fnclex,"fnclex")
    (e_fninit,"fninit")
    (e_fnop,"fnop")
    (e_fnstcw,"fnstcw")
    (e_fnstsw,"fnstsw")
    (e_fpatan,"fpatan")
    (e_fstpnce,"fstpnce")
    (e_fprem,"fprem")
    (e_fprem1,"fprem1")
    (e_fptan,"fptan")
    (e_frndint,"frndint")
    (e_frstor,"frstor")
    (e_fnsave,"fnsave")
    (e_fscale,"fscale")
    (e_fsetpm,"fsetpm")
    (e_fsincos,"fsincos")
    (e_fnstenv,"fnstenv")
    (e_fxam,"fxam")
    (e_fxrstor,"fxrstor")
    (e_fxrstor64,"fxrstor64")
    (e_fxsave,"fxsave")
    (e_fxsave64,"fxsave64")
    (e_fxtract,"fxtract")
    (e_fyl2x,"fyl2x")
    (e_fyl2xp1,"fyl2xp1")
    (e_getsec,"getsec")
    (e_gf2p8affineinvqb,"gf2p8affineinvqb")
    (e_gf2p8affineqb,"gf2p8affineqb")
    (e_gf2p8mulb,"gf2p8mulb")
    (e_haddpd,"haddpd")
    (e_haddps,"haddps")
    (e_hlt,"hlt")
    (e_hsubpd,"hsubpd")
    (e_hsubps,"hsubps")
    (e_idiv,"idiv")
    (e_fild,"fild")
    (e_imul,"imul")
    (e_in,"in")
    (e_inc,"inc")
    (e_incsspd,"incsspd")
    (e_incsspq,"incsspq")
    (e_insb,"insb")
    (e_insertps,"insertps")
    (e_insertq,"insertq")
    (e_insd,"insd")
    (e_insw,"insw")
    (e_int,"int")
    (e_int1,"int1")
    (e_int3,"int3")
    (e_into,"into")
    (e_invd,"invd")
    (e_invept,"invept")
    (e_invlpg,"invlpg")
    (e_invlpga,"invlpga")
    (e_invpcid,"invpcid")
    (e_invvpid,"invvpid")
    (e_iret,"iret")
    (e_iretd,"iretd")
    (e_iretq,"iretq")
    (e_fisttp,"fisttp")
    (e_fist,"fist")
    (e_fistp,"fistp")
    (e_jnb,"jnb")
    (e_jnbe,"jnbe")
    (e_jbe,"jbe")
    (e_jb,"jb")
    (e_jcxz,"jcxz")
    (e_jecxz,"jecxz")
    (e_jz,"jz")
    (e_jnl,"jnl")
    (e_jnle,"jnle")
    (e_jle,"jle")
    (e_jl,"jl")
    (e_jnz,"jnz")
    (e_jno,"jno")
    (e_jnp,"jnp")
    (e_jns,"jns")
    (e_jo,"jo")
    (e_jp,"jp")
    (e_jrcxz,"jrcxz")
    (e_js,"js")
    (e_kaddb,"kaddb")
    (e_kaddd,"kaddd")
    (e_kaddq,"kaddq")
    (e_kaddw,"kaddw")
    (e_kandb,"kandb")
    (e_kandd,"kandd")
    (e_kandnb,"kandnb")
    (e_kandnd,"kandnd")
    (e_kandnq,"kandnq")
    (e_kandnw,"kandnw")
    (e_kandq,"kandq")
    (e_kandw,"kandw")
    (e_kmovb,"kmovb")
    (e_kmovd,"kmovd")
    (e_kmovq,"kmovq")
    (e_kmovw,"kmovw")
    (e_knotb,"knotb")
    (e_knotd,"knotd")
    (e_knotq,"knotq")
    (e_knotw,"knotw")
    (e_korb,"korb")
    (e_kord,"kord")
    (e_korq,"korq")
    (e_kortestb,"kortestb")
    (e_kortestd,"kortestd")
    (e_kortestq,"kortestq")
    (e_kortestw,"kortestw")
    (e_korw,"korw")
    (e_kshiftlb,"kshiftlb")
    (e_kshiftld,"kshiftld")
    (e_kshiftlq,"kshiftlq")
    (e_kshiftlw,"kshiftlw")
    (e_kshiftrb,"kshiftrb")
    (e_kshiftrd,"kshiftrd")
    (e_kshiftrq,"kshiftrq")
    (e_kshiftrw,"kshiftrw")
    (e_ktestb,"ktestb")
    (e_ktestd,"ktestd")
    (e_ktestq,"ktestq")
    (e_ktestw,"ktestw")
    (e_kunpckbw,"kunpckbw")
    (e_kunpckdq,"kunpckdq")
    (e_kunpckwd,"kunpckwd")
    (e_kxnorb,"kxnorb")
    (e_kxnord,"kxnord")
    (e_kxnorq,"kxnorq")
    (e_kxnorw,"kxnorw")
    (e_kxorb,"kxorb")
    (e_kxord,"kxord")
    (e_kxorq,"kxorq")
    (e_kxorw,"kxorw")
    (e_lahf,"lahf")
    (e_lar,"lar")
    (e_lddqu,"lddqu")
    (e_ldmxcsr,"ldmxcsr")
    (e_lds,"lds")
    (e_fldz,"fldz")
    (e_fld1,"fld1")
    (e_fld,"fld")
    (e_lea,"lea")
    (e_leave,"leave")
    (e_les,"les")
    (e_lfence,"lfence")
    (e_lfs,"lfs")
    (e_lgdt,"lgdt")
    (e_lgs,"lgs")
    (e_lidt,"lidt")
    (e_lldt,"lldt")
    (e_llwpcb,"llwpcb")
    (e_lmsw,"lmsw")
    (e_lock,"lock")
    (e_lodsb,"lodsb")
    (e_lodsd,"lodsd")
    (e_lodsq,"lodsq")
    (e_lodsw,"lodsw")
    (e_loop,"loop")
    (e_loope,"loope")
    (e_loopne,"loopne")
    (e_retf,"retf")
    (e_retfq,"retfq")
    (e_lsl,"lsl")
    (e_lss,"lss")
    (e_ltr,"ltr")
    (e_lwpins,"lwpins")
    (e_lwpval,"lwpval")
    (e_lzcnt,"lzcnt")
    (e_maskmovdqu,"maskmovdqu")
    (e_maxpd,"maxpd")
    (e_maxps,"maxps")
    (e_maxsd,"maxsd")
    (e_maxss,"maxss")
    (e_mfence,"mfence")
    (e_minpd,"minpd")
    (e_minps,"minps")
    (e_minsd,"minsd")
    (e_minss,"minss")
    (e_cvtpd2pi,"cvtpd2pi")
    (e_cvtpi2pd,"cvtpi2pd")
    (e_cvtpi2ps,"cvtpi2ps")
    (e_cvtps2pi,"cvtps2pi")
    (e_cvttpd2pi,"cvttpd2pi")
    (e_cvttps2pi,"cvttps2pi")
    (e_emms,"emms")
    (e_maskmovq,"maskmovq")
    (e_movd,"movd")
    (e_movq,"movq")
    (e_movdq2q,"movdq2q")
    (e_movntq,"movntq")
    (e_movq2dq,"movq2dq")
    (e_pabsb,"pabsb")
    (e_pabsd,"pabsd")
    (e_pabsw,"pabsw")
    (e_packssdw,"packssdw")
    (e_packsswb,"packsswb")
    (e_packuswb,"packuswb")
    (e_paddb,"paddb")
    (e_paddd,"paddd")
    (e_paddq,"paddq")
    (e_paddsb,"paddsb")
    (e_paddsw,"paddsw")
    (e_paddusb,"paddusb")
    (e_paddusw,"paddusw")
    (e_paddw,"paddw")
    (e_palignr,"palignr")
    (e_pandn,"pandn")
    (e_pand,"pand")
    (e_pavgb,"pavgb")
    (e_pavgw,"pavgw")
    (e_pcmpeqb,"pcmpeqb")
    (e_pcmpeqd,"pcmpeqd")
    (e_pcmpeqw,"pcmpeqw")
    (e_pcmpgtb,"pcmpgtb")
    (e_pcmpgtd,"pcmpgtd")
    (e_pcmpgtw,"pcmpgtw")
    (e_pextrw,"pextrw")
    (e_phaddd,"phaddd")
    (e_phaddsw,"phaddsw")
    (e_phaddw,"phaddw")
    (e_phsubd,"phsubd")
    (e_phsubsw,"phsubsw")
    (e_phsubw,"phsubw")
    (e_pinsrw,"pinsrw")
    (e_pmaddubsw,"pmaddubsw")
    (e_pmaddwd,"pmaddwd")
    (e_pmaxsw,"pmaxsw")
    (e_pmaxub,"pmaxub")
    (e_pminsw,"pminsw")
    (e_pminub,"pminub")
    (e_pmovmskb,"pmovmskb")
    (e_pmulhrsw,"pmulhrsw")
    (e_pmulhuw,"pmulhuw")
    (e_pmulhw,"pmulhw")
    (e_pmullw,"pmullw")
    (e_pmuludq,"pmuludq")
    (e_por,"por")
    (e_psadbw,"psadbw")
    (e_pshufb,"pshufb")
    (e_pshufw,"pshufw")
    (e_psignb,"psignb")
    (e_psignd,"psignd")
    (e_psignw,"psignw")
    (e_pslld,"pslld")
    (e_psllq,"psllq")
    (e_psllw,"psllw")
    (e_psrad,"psrad")
    (e_psraw,"psraw")
    (e_psrld,"psrld")
    (e_psrlq,"psrlq")
    (e_psrlw,"psrlw")
    (e_psubb,"psubb")
    (e_psubd,"psubd")
    (e_psubq,"psubq")
    (e_psubsb,"psubsb")
    (e_psubsw,"psubsw")
    (e_psubusb,"psubusb")
    (e_psubusw,"psubusw")
    (e_psubw,"psubw")
    (e_punpckhbw,"punpckhbw")
    (e_punpckhdq,"punpckhdq")
    (e_punpckhwd,"punpckhwd")
    (e_punpcklbw,"punpcklbw")
    (e_punpckldq,"punpckldq")
    (e_punpcklwd,"punpcklwd")
    (e_pxor,"pxor")
    (e_monitorx,"monitorx")
    (e_monitor,"monitor")
    (e_montmul,"montmul")
    (e_mov,"mov")
    (e_movabs,"movabs")
    (e_movapd,"movapd")
    (e_movaps,"movaps")
    (e_movbe,"movbe")
    (e_movddup,"movddup")
    (e_movdir64b,"movdir64b")
    (e_movdiri,"movdiri")
    (e_movdqa,"movdqa")
    (e_movdqu,"movdqu")
    (e_movhlps,"movhlps")
    (e_movhpd,"movhpd")
    (e_movhps,"movhps")
    (e_movlhps,"movlhps")
    (e_movlpd,"movlpd")
    (e_movlps,"movlps")
    (e_movmskpd,"movmskpd")
    (e_movmskps,"movmskps")
    (e_movntdqa,"movntdqa")
    (e_movntdq,"movntdq")
    (e_movnti,"movnti")
    (e_movntpd,"movntpd")
    (e_movntps,"movntps")
    (e_movntsd,"movntsd")
    (e_movntss,"movntss")
    (e_movsb,"movsb")
    (e_movsd,"movsd")
    (e_movshdup,"movshdup")
    (e_movsldup,"movsldup")
    (e_movsq,"movsq")
    (e_movss,"movss")
    (e_movsw,"movsw")
    (e_movsx,"movsx")
    (e_movsxd,"movsxd")
    (e_movupd,"movupd")
    (e_movups,"movups")
    (e_movzx,"movzx")
    (e_mpsadbw,"mpsadbw")
    (e_mul,"mul")
    (e_mulpd,"mulpd")
    (e_mulps,"mulps")
    (e_mulsd,"mulsd")
    (e_mulss,"mulss")
    (e_mulx,"mulx")
    (e_fmul,"fmul")
    (e_fimul,"fimul")
    (e_fmulp,"fmulp")
    (e_mwaitx,"mwaitx")
    (e_mwait,"mwait")
    (e_neg,"neg")
    (e_nop,"nop")
    (e_not,"not")
    (e_or,"or")
    (e_orpd,"orpd")
    (e_orps,"orps")
    (e_out,"out")
    (e_outsb,"outsb")
    (e_outsd,"outsd")
    (e_outsw,"outsw")
    (e_packusdw,"packusdw")
    (e_pause,"pause")
    (e_pavgusb,"pavgusb")
    (e_pblendvb,"pblendvb")
    (e_pblendw,"pblendw")
    (e_pclmulqdq,"pclmulqdq")
    (e_pcmpeqq,"pcmpeqq")
    (e_pcmpestri,"pcmpestri")
    (e_pcmpestrm,"pcmpestrm")
    (e_pcmpgtq,"pcmpgtq")
    (e_pcmpistri,"pcmpistri")
    (e_pcmpistrm,"pcmpistrm")
    (e_pconfig,"pconfig")
    (e_pdep,"pdep")
    (e_pext,"pext")
    (e_pextrb,"pextrb")
    (e_pextrd,"pextrd")
    (e_pextrq,"pextrq")
    (e_pf2id,"pf2id")
    (e_pf2iw,"pf2iw")
    (e_pfacc,"pfacc")
    (e_pfadd,"pfadd")
    (e_pfcmpeq,"pfcmpeq")
    (e_pfcmpge,"pfcmpge")
    (e_pfcmpgt,"pfcmpgt")
    (e_pfmax,"pfmax")
    (e_pfmin,"pfmin")
    (e_pfmul,"pfmul")
    (e_pfnacc,"pfnacc")
    (e_pfpnacc,"pfpnacc")
    (e_pfrcpit1,"pfrcpit1")
    (e_pfrcpit2,"pfrcpit2")
    (e_pfrcp,"pfrcp")
    (e_pfrsqit1,"pfrsqit1")
    (e_pfrsqrt,"pfrsqrt")
    (e_pfsubr,"pfsubr")
    (e_pfsub,"pfsub")
    (e_phminposuw,"phminposuw")
    (e_pi2fd,"pi2fd")
    (e_pi2fw,"pi2fw")
    (e_pinsrb,"pinsrb")
    (e_pinsrd,"pinsrd")
    (e_pinsrq,"pinsrq")
    (e_pmaxsb,"pmaxsb")
    (e_pmaxsd,"pmaxsd")
    (e_pmaxud,"pmaxud")
    (e_pmaxuw,"pmaxuw")
    (e_pminsb,"pminsb")
    (e_pminsd,"pminsd")
    (e_pminud,"pminud")
    (e_pminuw,"pminuw")
    (e_pmovsxbd,"pmovsxbd")
    (e_pmovsxbq,"pmovsxbq")
    (e_pmovsxbw,"pmovsxbw")
    (e_pmovsxdq,"pmovsxdq")
    (e_pmovsxwd,"pmovsxwd")
    (e_pmovsxwq,"pmovsxwq")
    (e_pmovzxbd,"pmovzxbd")
    (e_pmovzxbq,"pmovzxbq")
    (e_pmovzxbw,"pmovzxbw")
    (e_pmovzxdq,"pmovzxdq")
    (e_pmovzxwd,"pmovzxwd")
    (e_pmovzxwq,"pmovzxwq")
    (e_pmuldq,"pmuldq")
    (e_pmulhrw,"pmulhrw")
    (e_pmulld,"pmulld")
    (e_pop,"pop")
    (e_popaw,"popaw")
    (e_popal,"popal")
    (e_popcnt,"popcnt")
    (e_popf,"popf")
    (e_popfd,"popfd")
    (e_popfq,"popfq")
    (e_prefetch,"prefetch")
    (e_prefetchnta,"prefetchnta")
    (e_prefetcht0,"prefetcht0")
    (e_prefetcht1,"prefetcht1")
    (e_prefetcht2,"prefetcht2")
    (e_prefetchw,"prefetchw")
    (e_prefetchwt1,"prefetchwt1")
    (e_pshufd,"pshufd")
    (e_pshufhw,"pshufhw")
    (e_pshuflw,"pshuflw")
    (e_pslldq,"pslldq")
    (e_psrldq,"psrldq")
    (e_pswapd,"pswapd")
    (e_ptest,"ptest")
    (e_ptwrite,"ptwrite")
    (e_punpckhqdq,"punpckhqdq")
    (e_punpcklqdq,"punpcklqdq")
    (e_push,"push")
    (e_pushaw,"pushaw")
    (e_pushal,"pushal")
    (e_pushf,"pushf")
    (e_pushfd,"pushfd")
    (e_pushfq,"pushfq")
    (e_rcl,"rcl")
    (e_rcpps,"rcpps")
    (e_rcpss,"rcpss")
    (e_rcr,"rcr")
    (e_rdfsbase,"rdfsbase")
    (e_rdgsbase,"rdgsbase")
    (e_rdmsr,"rdmsr")
    (e_rdpid,"rdpid")
    (e_rdpkru,"rdpkru")
    (e_rdpmc,"rdpmc")
    (e_rdrand,"rdrand")
    (e_rdseed,"rdseed")
    (e_rdsspd,"rdsspd")
    (e_rdsspq,"rdsspq")
    (e_rdtsc,"rdtsc")
    (e_rdtscp,"rdtscp")
    (e_repne,"repne")
    (e_rep,"rep")
    (e_ret_near,"ret_near")
    (e_rex64,"rex64")
    (e_rol,"rol")
    (e_ror,"ror")
    (e_rorx,"rorx")
    (e_roundpd,"roundpd")
    (e_roundps,"roundps")
    (e_roundsd,"roundsd")
    (e_roundss,"roundss")
    (e_rsm,"rsm")
    (e_rsqrtps,"rsqrtps")
    (e_rsqrtss,"rsqrtss")
    (e_rstorssp,"rstorssp")
    (e_sahf,"sahf")
    (e_sal,"sal")
    (e_salc,"salc")
    (e_sar,"sar")
    (e_sarx,"sarx")
    (e_saveprevssp,"saveprevssp")
    (e_sbb,"sbb")
    (e_scasb,"scasb")
    (e_scasd,"scasd")
    (e_scasq,"scasq")
    (e_scasw,"scasw")
    (e_setae,"setae")
    (e_seta,"seta")
    (e_setbe,"setbe")
    (e_setb,"setb")
    (e_sete,"sete")
    (e_setge,"setge")
    (e_setg,"setg")
    (e_setle,"setle")
    (e_setl,"setl")
    (e_setne,"setne")
    (e_setno,"setno")
    (e_setnp,"setnp")
    (e_setns,"setns")
    (e_seto,"seto")
    (e_setp,"setp")
    (e_setssbsy,"setssbsy")
    (e_sets,"sets")
    (e_sfence,"sfence")
    (e_sgdt,"sgdt")
    (e_sha1msg1,"sha1msg1")
    (e_sha1msg2,"sha1msg2")
    (e_sha1nexte,"sha1nexte")
    (e_sha1rnds4,"sha1rnds4")
    (e_sha256msg1,"sha256msg1")
    (e_sha256msg2,"sha256msg2")
    (e_sha256rnds2,"sha256rnds2")
    (e_shl,"shl")
    (e_shld,"shld")
    (e_shlx,"shlx")
    (e_shr,"shr")
    (e_shrd,"shrd")
    (e_shrx,"shrx")
    (e_shufpd,"shufpd")
    (e_shufps,"shufps")
    (e_sidt,"sidt")
    (e_fsin,"fsin")
    (e_skinit,"skinit")
    (e_sldt,"sldt")
    (e_slwpcb,"slwpcb")
    (e_smsw,"smsw")
    (e_sqrtpd,"sqrtpd")
    (e_sqrtps,"sqrtps")
    (e_sqrtsd,"sqrtsd")
    (e_sqrtss,"sqrtss")
    (e_fsqrt,"fsqrt")
    (e_stac,"stac")
    (e_stc,"stc")
    (e_std,"std")
    (e_stgi,"stgi")
    (e_sti,"sti")
    (e_stmxcsr,"stmxcsr")
    (e_stosb,"stosb")
    (e_stosd,"stosd")
    (e_stosq,"stosq")
    (e_stosw,"stosw")
    (e_str,"str")
    (e_fst,"fst")
    (e_fstp,"fstp")
    (e_sub,"sub")
    (e_subpd,"subpd")
    (e_subps,"subps")
    (e_fsubr,"fsubr")
    (e_fisubr,"fisubr")
    (e_fsubrp,"fsubrp")
    (e_subsd,"subsd")
    (e_subss,"subss")
    (e_fsub,"fsub")
    (e_fisub,"fisub")
    (e_fsubp,"fsubp")
    (e_swapgs,"swapgs")
    (e_syscall,"syscall")
    (e_sysenter,"sysenter")
    (e_sysexit,"sysexit")
    (e_sysexitq,"sysexitq")
    (e_sysret,"sysret")
    (e_sysretq,"sysretq")
    (e_t1mskc,"t1mskc")
    (e_test,"test")
    (e_tpause,"tpause")
    (e_ftst,"ftst")
    (e_tzcnt,"tzcnt")
    (e_tzmsk,"tzmsk")
    (e_ucomisd,"ucomisd")
    (e_ucomiss,"ucomiss")
    (e_fucompi,"fucompi")
    (e_fucomi,"fucomi")
    (e_fucompp,"fucompp")
    (e_fucomp,"fucomp")
    (e_fucom,"fucom")
    (e_ud0,"ud0")
    (e_ud1,"ud1")
    (e_ud2,"ud2")
    (e_umonitor,"umonitor")
    (e_umwait,"umwait")
    (e_unpckhpd,"unpckhpd")
    (e_unpckhps,"unpckhps")
    (e_unpcklpd,"unpcklpd")
    (e_unpcklps,"unpcklps")
    (e_v4fmaddps,"v4fmaddps")
    (e_v4fmaddss,"v4fmaddss")
    (e_v4fnmaddps,"v4fnmaddps")
    (e_v4fnmaddss,"v4fnmaddss")
    (e_vaddpd,"vaddpd")
    (e_vaddps,"vaddps")
    (e_vaddsd,"vaddsd")
    (e_vaddss,"vaddss")
    (e_vaddsubpd,"vaddsubpd")
    (e_vaddsubps,"vaddsubps")
    (e_vaesdeclast,"vaesdeclast")
    (e_vaesdec,"vaesdec")
    (e_vaesenclast,"vaesenclast")
    (e_vaesenc,"vaesenc")
    (e_vaesimc,"vaesimc")
    (e_vaeskeygenassist,"vaeskeygenassist")
    (e_valignd,"valignd")
    (e_valignq,"valignq")
    (e_vandnpd,"vandnpd")
    (e_vandnps,"vandnps")
    (e_vandpd,"vandpd")
    (e_vandps,"vandps")
    (e_vblendmpd,"vblendmpd")
    (e_vblendmps,"vblendmps")
    (e_vblendpd,"vblendpd")
    (e_vblendps,"vblendps")
    (e_vblendvpd,"vblendvpd")
    (e_vblendvps,"vblendvps")
    (e_vbroadcastf128,"vbroadcastf128")
    (e_vbroadcastf32x2,"vbroadcastf32x2")
    (e_vbroadcastf32x4,"vbroadcastf32x4")
    (e_vbroadcastf32x8,"vbroadcastf32x8")
    (e_vbroadcastf64x2,"vbroadcastf64x2")
    (e_vbroadcastf64x4,"vbroadcastf64x4")
    (e_vbroadcasti128,"vbroadcasti128")
    (e_vbroadcasti32x2,"vbroadcasti32x2")
    (e_vbroadcasti32x4,"vbroadcasti32x4")
    (e_vbroadcasti32x8,"vbroadcasti32x8")
    (e_vbroadcasti64x2,"vbroadcasti64x2")
    (e_vbroadcasti64x4,"vbroadcasti64x4")
    (e_vbroadcastsd,"vbroadcastsd")
    (e_vbroadcastss,"vbroadcastss")
    (e_vcmp,"vcmp")
    (e_vcmppd,"vcmppd")
    (e_vcmpps,"vcmpps")
    (e_vcmpsd,"vcmpsd")
    (e_vcmpss,"vcmpss")
    (e_vcomisd,"vcomisd")
    (e_vcomiss,"vcomiss")
    (e_vcompresspd,"vcompresspd")
    (e_vcompressps,"vcompressps")
    (e_vcvtdq2pd,"vcvtdq2pd")
    (e_vcvtdq2ps,"vcvtdq2ps")
    (e_vcvtpd2dq,"vcvtpd2dq")
    (e_vcvtpd2ps,"vcvtpd2ps")
    (e_vcvtpd2qq,"vcvtpd2qq")
    (e_vcvtpd2udq,"vcvtpd2udq")
    (e_vcvtpd2uqq,"vcvtpd2uqq")
    (e_vcvtph2ps,"vcvtph2ps")
    (e_vcvtps2dq,"vcvtps2dq")
    (e_vcvtps2pd,"vcvtps2pd")
    (e_vcvtps2ph,"vcvtps2ph")
    (e_vcvtps2qq,"vcvtps2qq")
    (e_vcvtps2udq,"vcvtps2udq")
    (e_vcvtps2uqq,"vcvtps2uqq")
    (e_vcvtqq2pd,"vcvtqq2pd")
    (e_vcvtqq2ps,"vcvtqq2ps")
    (e_vcvtsd2si,"vcvtsd2si")
    (e_vcvtsd2ss,"vcvtsd2ss")
    (e_vcvtsd2usi,"vcvtsd2usi")
    (e_vcvtsi2sd,"vcvtsi2sd")
    (e_vcvtsi2ss,"vcvtsi2ss")
    (e_vcvtss2sd,"vcvtss2sd")
    (e_vcvtss2si,"vcvtss2si")
    (e_vcvtss2usi,"vcvtss2usi")
    (e_vcvttpd2dq,"vcvttpd2dq")
    (e_vcvttpd2qq,"vcvttpd2qq")
    (e_vcvttpd2udq,"vcvttpd2udq")
    (e_vcvttpd2uqq,"vcvttpd2uqq")
    (e_vcvttps2dq,"vcvttps2dq")
    (e_vcvttps2qq,"vcvttps2qq")
    (e_vcvttps2udq,"vcvttps2udq")
    (e_vcvttps2uqq,"vcvttps2uqq")
    (e_vcvttsd2si,"vcvttsd2si")
    (e_vcvttsd2usi,"vcvttsd2usi")
    (e_vcvttss2si,"vcvttss2si")
    (e_vcvttss2usi,"vcvttss2usi")
    (e_vcvtudq2pd,"vcvtudq2pd")
    (e_vcvtudq2ps,"vcvtudq2ps")
    (e_vcvtuqq2pd,"vcvtuqq2pd")
    (e_vcvtuqq2ps,"vcvtuqq2ps")
    (e_vcvtusi2sd,"vcvtusi2sd")
    (e_vcvtusi2ss,"vcvtusi2ss")
    (e_vdbpsadbw,"vdbpsadbw")
    (e_vdivpd,"vdivpd")
    (e_vdivps,"vdivps")
    (e_vdivsd,"vdivsd")
    (e_vdivss,"vdivss")
    (e_vdppd,"vdppd")
    (e_vdpps,"vdpps")
    (e_verr,"verr")
    (e_verw,"verw")
    (e_vexp2pd,"vexp2pd")
    (e_vexp2ps,"vexp2ps")
    (e_vexpandpd,"vexpandpd")
    (e_vexpandps,"vexpandps")
    (e_vextractf128,"vextractf128")
    (e_vextractf32x4,"vextractf32x4")
    (e_vextractf32x8,"vextractf32x8")
    (e_vextractf64x2,"vextractf64x2")
    (e_vextractf64x4,"vextractf64x4")
    (e_vextracti128,"vextracti128")
    (e_vextracti32x4,"vextracti32x4")
    (e_vextracti32x8,"vextracti32x8")
    (e_vextracti64x2,"vextracti64x2")
    (e_vextracti64x4,"vextracti64x4")
    (e_vextractps,"vextractps")
    (e_vfixupimmpd,"vfixupimmpd")
    (e_vfixupimmps,"vfixupimmps")
    (e_vfixupimmsd,"vfixupimmsd")
    (e_vfixupimmss,"vfixupimmss")
    (e_vfmadd132pd,"vfmadd132pd")
    (e_vfmadd132ps,"vfmadd132ps")
    (e_vfmadd132sd,"vfmadd132sd")
    (e_vfmadd132ss,"vfmadd132ss")
    (e_vfmadd213pd,"vfmadd213pd")
    (e_vfmadd213ps,"vfmadd213ps")
    (e_vfmadd213sd,"vfmadd213sd")
    (e_vfmadd213ss,"vfmadd213ss")
    (e_vfmadd231pd,"vfmadd231pd")
    (e_vfmadd231ps,"vfmadd231ps")
    (e_vfmadd231sd,"vfmadd231sd")
    (e_vfmadd231ss,"vfmadd231ss")
    (e_vfmaddpd,"vfmaddpd")
    (e_vfmaddps,"vfmaddps")
    (e_vfmaddsd,"vfmaddsd")
    (e_vfmaddss,"vfmaddss")
    (e_vfmaddsub132pd,"vfmaddsub132pd")
    (e_vfmaddsub132ps,"vfmaddsub132ps")
    (e_vfmaddsub213pd,"vfmaddsub213pd")
    (e_vfmaddsub213ps,"vfmaddsub213ps")
    (e_vfmaddsub231pd,"vfmaddsub231pd")
    (e_vfmaddsub231ps,"vfmaddsub231ps")
    (e_vfmaddsubpd,"vfmaddsubpd")
    (e_vfmaddsubps,"vfmaddsubps")
    (e_vfmsub132pd,"vfmsub132pd")
    (e_vfmsub132ps,"vfmsub132ps")
    (e_vfmsub132sd,"vfmsub132sd")
    (e_vfmsub132ss,"vfmsub132ss")
    (e_vfmsub213pd,"vfmsub213pd")
    (e_vfmsub213ps,"vfmsub213ps")
    (e_vfmsub213sd,"vfmsub213sd")
    (e_vfmsub213ss,"vfmsub213ss")
    (e_vfmsub231pd,"vfmsub231pd")
    (e_vfmsub231ps,"vfmsub231ps")
    (e_vfmsub231sd,"vfmsub231sd")
    (e_vfmsub231ss,"vfmsub231ss")
    (e_vfmsubadd132pd,"vfmsubadd132pd")
    (e_vfmsubadd132ps,"vfmsubadd132ps")
    (e_vfmsubadd213pd,"vfmsubadd213pd")
    (e_vfmsubadd213ps,"vfmsubadd213ps")
    (e_vfmsubadd231pd,"vfmsubadd231pd")
    (e_vfmsubadd231ps,"vfmsubadd231ps")
    (e_vfmsubaddpd,"vfmsubaddpd")
    (e_vfmsubaddps,"vfmsubaddps")
    (e_vfmsubpd,"vfmsubpd")
    (e_vfmsubps,"vfmsubps")
    (e_vfmsubsd,"vfmsubsd")
    (e_vfmsubss,"vfmsubss")
    (e_vfnmadd132pd,"vfnmadd132pd")
    (e_vfnmadd132ps,"vfnmadd132ps")
    (e_vfnmadd132sd,"vfnmadd132sd")
    (e_vfnmadd132ss,"vfnmadd132ss")
    (e_vfnmadd213pd,"vfnmadd213pd")
    (e_vfnmadd213ps,"vfnmadd213ps")
    (e_vfnmadd213sd,"vfnmadd213sd")
    (e_vfnmadd213ss,"vfnmadd213ss")
    (e_vfnmadd231pd,"vfnmadd231pd")
    (e_vfnmadd231ps,"vfnmadd231ps")
    (e_vfnmadd231sd,"vfnmadd231sd")
    (e_vfnmadd231ss,"vfnmadd231ss")
    (e_vfnmaddpd,"vfnmaddpd")
    (e_vfnmaddps,"vfnmaddps")
    (e_vfnmaddsd,"vfnmaddsd")
    (e_vfnmaddss,"vfnmaddss")
    (e_vfnmsub132pd,"vfnmsub132pd")
    (e_vfnmsub132ps,"vfnmsub132ps")
    (e_vfnmsub132sd,"vfnmsub132sd")
    (e_vfnmsub132ss,"vfnmsub132ss")
    (e_vfnmsub213pd,"vfnmsub213pd")
    (e_vfnmsub213ps,"vfnmsub213ps")
    (e_vfnmsub213sd,"vfnmsub213sd")
    (e_vfnmsub213ss,"vfnmsub213ss")
    (e_vfnmsub231pd,"vfnmsub231pd")
    (e_vfnmsub231ps,"vfnmsub231ps")
    (e_vfnmsub231sd,"vfnmsub231sd")
    (e_vfnmsub231ss,"vfnmsub231ss")
    (e_vfnmsubpd,"vfnmsubpd")
    (e_vfnmsubps,"vfnmsubps")
    (e_vfnmsubsd,"vfnmsubsd")
    (e_vfnmsubss,"vfnmsubss")
    (e_vfpclasspd,"vfpclasspd")
    (e_vfpclassps,"vfpclassps")
    (e_vfpclasssd,"vfpclasssd")
    (e_vfpclassss,"vfpclassss")
    (e_vfrczpd,"vfrczpd")
    (e_vfrczps,"vfrczps")
    (e_vfrczsd,"vfrczsd")
    (e_vfrczss,"vfrczss")
    (e_vgatherdpd,"vgatherdpd")
    (e_vgatherdps,"vgatherdps")
    (e_vgatherpf0dpd,"vgatherpf0dpd")
    (e_vgatherpf0dps,"vgatherpf0dps")
    (e_vgatherpf0qpd,"vgatherpf0qpd")
    (e_vgatherpf0qps,"vgatherpf0qps")
    (e_vgatherpf1dpd,"vgatherpf1dpd")
    (e_vgatherpf1dps,"vgatherpf1dps")
    (e_vgatherpf1qpd,"vgatherpf1qpd")
    (e_vgatherpf1qps,"vgatherpf1qps")
    (e_vgatherqpd,"vgatherqpd")
    (e_vgatherqps,"vgatherqps")
    (e_vgetexppd,"vgetexppd")
    (e_vgetexpps,"vgetexpps")
    (e_vgetexpsd,"vgetexpsd")
    (e_vgetexpss,"vgetexpss")
    (e_vgetmantpd,"vgetmantpd")
    (e_vgetmantps,"vgetmantps")
    (e_vgetmantsd,"vgetmantsd")
    (e_vgetmantss,"vgetmantss")
    (e_vgf2p8affineinvqb,"vgf2p8affineinvqb")
    (e_vgf2p8affineqb,"vgf2p8affineqb")
    (e_vgf2p8mulb,"vgf2p8mulb")
    (e_vhaddpd,"vhaddpd")
    (e_vhaddps,"vhaddps")
    (e_vhsubpd,"vhsubpd")
    (e_vhsubps,"vhsubps")
    (e_vinsertf128,"vinsertf128")
    (e_vinsertf32x4,"vinsertf32x4")
    (e_vinsertf32x8,"vinsertf32x8")
    (e_vinsertf64x2,"vinsertf64x2")
    (e_vinsertf64x4,"vinsertf64x4")
    (e_vinserti128,"vinserti128")
    (e_vinserti32x4,"vinserti32x4")
    (e_vinserti32x8,"vinserti32x8")
    (e_vinserti64x2,"vinserti64x2")
    (e_vinserti64x4,"vinserti64x4")
    (e_vinsertps,"vinsertps")
    (e_vlddqu,"vlddqu")
    (e_vldmxcsr,"vldmxcsr")
    (e_vmaskmovdqu,"vmaskmovdqu")
    (e_vmaskmovpd,"vmaskmovpd")
    (e_vmaskmovps,"vmaskmovps")
    (e_vmaxpd,"vmaxpd")
    (e_vmaxps,"vmaxps")
    (e_vmaxsd,"vmaxsd")
    (e_vmaxss,"vmaxss")
    (e_vmcall,"vmcall")
    (e_vmclear,"vmclear")
    (e_vmfunc,"vmfunc")
    (e_vminpd,"vminpd")
    (e_vminps,"vminps")
    (e_vminsd,"vminsd")
    (e_vminss,"vminss")
    (e_vmlaunch,"vmlaunch")
    (e_vmload,"vmload")
    (e_vmmcall,"vmmcall")
    (e_vmovq,"vmovq")
    (e_vmovapd,"vmovapd")
    (e_vmovaps,"vmovaps")
    (e_vmovddup,"vmovddup")
    (e_vmovd,"vmovd")
    (e_vmovdqa32,"vmovdqa32")
    (e_vmovdqa64,"vmovdqa64")
    (e_vmovdqa,"vmovdqa")
    (e_vmovdqu16,"vmovdqu16")
    (e_vmovdqu32,"vmovdqu32")
    (e_vmovdqu64,"vmovdqu64")
    (e_vmovdqu8,"vmovdqu8")
    (e_vmovdqu,"vmovdqu")
    (e_vmovhlps,"vmovhlps")
    (e_vmovhpd,"vmovhpd")
    (e_vmovhps,"vmovhps")
    (e_vmovlhps,"vmovlhps")
    (e_vmovlpd,"vmovlpd")
    (e_vmovlps,"vmovlps")
    (e_vmovmskpd,"vmovmskpd")
    (e_vmovmskps,"vmovmskps")
    (e_vmovntdqa,"vmovntdqa")
    (e_vmovntdq,"vmovntdq")
    (e_vmovntpd,"vmovntpd")
    (e_vmovntps,"vmovntps")
    (e_vmovsd,"vmovsd")
    (e_vmovshdup,"vmovshdup")
    (e_vmovsldup,"vmovsldup")
    (e_vmovss,"vmovss")
    (e_vmovupd,"vmovupd")
    (e_vmovups,"vmovups")
    (e_vmpsadbw,"vmpsadbw")
    (e_vmptrld,"vmptrld")
    (e_vmptrst,"vmptrst")
    (e_vmread,"vmread")
    (e_vmresume,"vmresume")
    (e_vmrun,"vmrun")
    (e_vmsave,"vmsave")
    (e_vmulpd,"vmulpd")
    (e_vmulps,"vmulps")
    (e_vmulsd,"vmulsd")
    (e_vmulss,"vmulss")
    (e_vmwrite,"vmwrite")
    (e_vmxoff,"vmxoff")
    (e_vmxon,"vmxon")
    (e_vorpd,"vorpd")
    (e_vorps,"vorps")
    (e_vp4dpwssds,"vp4dpwssds")
    (e_vp4dpwssd,"vp4dpwssd")
    (e_vpabsb,"vpabsb")
    (e_vpabsd,"vpabsd")
    (e_vpabsq,"vpabsq")
    (e_vpabsw,"vpabsw")
    (e_vpackssdw,"vpackssdw")
    (e_vpacksswb,"vpacksswb")
    (e_vpackusdw,"vpackusdw")
    (e_vpackuswb,"vpackuswb")
    (e_vpaddb,"vpaddb")
    (e_vpaddd,"vpaddd")
    (e_vpaddq,"vpaddq")
    (e_vpaddsb,"vpaddsb")
    (e_vpaddsw,"vpaddsw")
    (e_vpaddusb,"vpaddusb")
    (e_vpaddusw,"vpaddusw")
    (e_vpaddw,"vpaddw")
    (e_vpalignr,"vpalignr")
    (e_vpandd,"vpandd")
    (e_vpandnd,"vpandnd")
    (e_vpandnq,"vpandnq")
    (e_vpandn,"vpandn")
    (e_vpandq,"vpandq")
    (e_vpand,"vpand")
    (e_vpavgb,"vpavgb")
    (e_vpavgw,"vpavgw")
    (e_vpblendd,"vpblendd")
    (e_vpblendmb,"vpblendmb")
    (e_vpblendmd,"vpblendmd")
    (e_vpblendmq,"vpblendmq")
    (e_vpblendmw,"vpblendmw")
    (e_vpblendvb,"vpblendvb")
    (e_vpblendw,"vpblendw")
    (e_vpbroadcastb,"vpbroadcastb")
    (e_vpbroadcastd,"vpbroadcastd")
    (e_vpbroadcastmb2q,"vpbroadcastmb2q")
    (e_vpbroadcastmw2d,"vpbroadcastmw2d")
    (e_vpbroadcastq,"vpbroadcastq")
    (e_vpbroadcastw,"vpbroadcastw")
    (e_vpclmulqdq,"vpclmulqdq")
    (e_vpcmov,"vpcmov")
    (e_vpcmp,"vpcmp")
    (e_vpcmpb,"vpcmpb")
    (e_vpcmpd,"vpcmpd")
    (e_vpcmpeqb,"vpcmpeqb")
    (e_vpcmpeqd,"vpcmpeqd")
    (e_vpcmpeqq,"vpcmpeqq")
    (e_vpcmpeqw,"vpcmpeqw")
    (e_vpcmpestri,"vpcmpestri")
    (e_vpcmpestrm,"vpcmpestrm")
    (e_vpcmpgtb,"vpcmpgtb")
    (e_vpcmpgtd,"vpcmpgtd")
    (e_vpcmpgtq,"vpcmpgtq")
    (e_vpcmpgtw,"vpcmpgtw")
    (e_vpcmpistri,"vpcmpistri")
    (e_vpcmpistrm,"vpcmpistrm")
    (e_vpcmpq,"vpcmpq")
    (e_vpcmpub,"vpcmpub")
    (e_vpcmpud,"vpcmpud")
    (e_vpcmpuq,"vpcmpuq")
    (e_vpcmpuw,"vpcmpuw")
    (e_vpcmpw,"vpcmpw")
    (e_vpcom,"vpcom")
    (e_vpcomb,"vpcomb")
    (e_vpcomd,"vpcomd")
    (e_vpcompressb,"vpcompressb")
    (e_vpcompressd,"vpcompressd")
    (e_vpcompressq,"vpcompressq")
    (e_vpcompressw,"vpcompressw")
    (e_vpcomq,"vpcomq")
    (e_vpcomub,"vpcomub")
    (e_vpcomud,"vpcomud")
    (e_vpcomuq,"vpcomuq")
    (e_vpcomuw,"vpcomuw")
    (e_vpcomw,"vpcomw")
    (e_vpconflictd,"vpconflictd")
    (e_vpconflictq,"vpconflictq")
    (e_vpdpbusds,"vpdpbusds")
    (e_vpdpbusd,"vpdpbusd")
    (e_vpdpwssds,"vpdpwssds")
    (e_vpdpwssd,"vpdpwssd")
    (e_vperm2f128,"vperm2f128")
    (e_vperm2i128,"vperm2i128")
    (e_vpermb,"vpermb")
    (e_vpermd,"vpermd")
    (e_vpermi2b,"vpermi2b")
    (e_vpermi2d,"vpermi2d")
    (e_vpermi2pd,"vpermi2pd")
    (e_vpermi2ps,"vpermi2ps")
    (e_vpermi2q,"vpermi2q")
    (e_vpermi2w,"vpermi2w")
    (e_vpermil2pd,"vpermil2pd")
    (e_vpermilpd,"vpermilpd")
    (e_vpermil2ps,"vpermil2ps")
    (e_vpermilps,"vpermilps")
    (e_vpermpd,"vpermpd")
    (e_vpermps,"vpermps")
    (e_vpermq,"vpermq")
    (e_vpermt2b,"vpermt2b")
    (e_vpermt2d,"vpermt2d")
    (e_vpermt2pd,"vpermt2pd")
    (e_vpermt2ps,"vpermt2ps")
    (e_vpermt2q,"vpermt2q")
    (e_vpermt2w,"vpermt2w")
    (e_vpermw,"vpermw")
    (e_vpexpandb,"vpexpandb")
    (e_vpexpandd,"vpexpandd")
    (e_vpexpandq,"vpexpandq")
    (e_vpexpandw,"vpexpandw")
    (e_vpextrb,"vpextrb")
    (e_vpextrd,"vpextrd")
    (e_vpextrq,"vpextrq")
    (e_vpextrw,"vpextrw")
    (e_vpgatherdd,"vpgatherdd")
    (e_vpgatherdq,"vpgatherdq")
    (e_vpgatherqd,"vpgatherqd")
    (e_vpgatherqq,"vpgatherqq")
    (e_vphaddbd,"vphaddbd")
    (e_vphaddbq,"vphaddbq")
    (e_vphaddbw,"vphaddbw")
    (e_vphadddq,"vphadddq")
    (e_vphaddd,"vphaddd")
    (e_vphaddsw,"vphaddsw")
    (e_vphaddubd,"vphaddubd")
    (e_vphaddubq,"vphaddubq")
    (e_vphaddubw,"vphaddubw")
    (e_vphaddudq,"vphaddudq")
    (e_vphadduwd,"vphadduwd")
    (e_vphadduwq,"vphadduwq")
    (e_vphaddwd,"vphaddwd")
    (e_vphaddwq,"vphaddwq")
    (e_vphaddw,"vphaddw")
    (e_vphminposuw,"vphminposuw")
    (e_vphsubbw,"vphsubbw")
    (e_vphsubdq,"vphsubdq")
    (e_vphsubd,"vphsubd")
    (e_vphsubsw,"vphsubsw")
    (e_vphsubwd,"vphsubwd")
    (e_vphsubw,"vphsubw")
    (e_vpinsrb,"vpinsrb")
    (e_vpinsrd,"vpinsrd")
    (e_vpinsrq,"vpinsrq")
    (e_vpinsrw,"vpinsrw")
    (e_vplzcntd,"vplzcntd")
    (e_vplzcntq,"vplzcntq")
    (e_vpmacsdd,"vpmacsdd")
    (e_vpmacsdqh,"vpmacsdqh")
    (e_vpmacsdql,"vpmacsdql")
    (e_vpmacssdd,"vpmacssdd")
    (e_vpmacssdqh,"vpmacssdqh")
    (e_vpmacssdql,"vpmacssdql")
    (e_vpmacsswd,"vpmacsswd")
    (e_vpmacssww,"vpmacssww")
    (e_vpmacswd,"vpmacswd")
    (e_vpmacsww,"vpmacsww")
    (e_vpmadcsswd,"vpmadcsswd")
    (e_vpmadcswd,"vpmadcswd")
    (e_vpmadd52huq,"vpmadd52huq")
    (e_vpmadd52luq,"vpmadd52luq")
    (e_vpmaddubsw,"vpmaddubsw")
    (e_vpmaddwd,"vpmaddwd")
    (e_vpmaskmovd,"vpmaskmovd")
    (e_vpmaskmovq,"vpmaskmovq")
    (e_vpmaxsb,"vpmaxsb")
    (e_vpmaxsd,"vpmaxsd")
    (e_vpmaxsq,"vpmaxsq")
    (e_vpmaxsw,"vpmaxsw")
    (e_vpmaxub,"vpmaxub")
    (e_vpmaxud,"vpmaxud")
    (e_vpmaxuq,"vpmaxuq")
    (e_vpmaxuw,"vpmaxuw")
    (e_vpminsb,"vpminsb")
    (e_vpminsd,"vpminsd")
    (e_vpminsq,"vpminsq")
    (e_vpminsw,"vpminsw")
    (e_vpminub,"vpminub")
    (e_vpminud,"vpminud")
    (e_vpminuq,"vpminuq")
    (e_vpminuw,"vpminuw")
    (e_vpmovb2m,"vpmovb2m")
    (e_vpmovd2m,"vpmovd2m")
    (e_vpmovdb,"vpmovdb")
    (e_vpmovdw,"vpmovdw")
    (e_vpmovm2b,"vpmovm2b")
    (e_vpmovm2d,"vpmovm2d")
    (e_vpmovm2q,"vpmovm2q")
    (e_vpmovm2w,"vpmovm2w")
    (e_vpmovmskb,"vpmovmskb")
    (e_vpmovq2m,"vpmovq2m")
    (e_vpmovqb,"vpmovqb")
    (e_vpmovqd,"vpmovqd")
    (e_vpmovqw,"vpmovqw")
    (e_vpmovsdb,"vpmovsdb")
    (e_vpmovsdw,"vpmovsdw")
    (e_vpmovsqb,"vpmovsqb")
    (e_vpmovsqd,"vpmovsqd")
    (e_vpmovsqw,"vpmovsqw")
    (e_vpmovswb,"vpmovswb")
    (e_vpmovsxbd,"vpmovsxbd")
    (e_vpmovsxbq,"vpmovsxbq")
    (e_vpmovsxbw,"vpmovsxbw")
    (e_vpmovsxdq,"vpmovsxdq")
    (e_vpmovsxwd,"vpmovsxwd")
    (e_vpmovsxwq,"vpmovsxwq")
    (e_vpmovusdb,"vpmovusdb")
    (e_vpmovusdw,"vpmovusdw")
    (e_vpmovusqb,"vpmovusqb")
    (e_vpmovusqd,"vpmovusqd")
    (e_vpmovusqw,"vpmovusqw")
    (e_vpmovuswb,"vpmovuswb")
    (e_vpmovw2m,"vpmovw2m")
    (e_vpmovwb,"vpmovwb")
    (e_vpmovzxbd,"vpmovzxbd")
    (e_vpmovzxbq,"vpmovzxbq")
    (e_vpmovzxbw,"vpmovzxbw")
    (e_vpmovzxdq,"vpmovzxdq")
    (e_vpmovzxwd,"vpmovzxwd")
    (e_vpmovzxwq,"vpmovzxwq")
    (e_vpmuldq,"vpmuldq")
    (e_vpmulhrsw,"vpmulhrsw")
    (e_vpmulhuw,"vpmulhuw")
    (e_vpmulhw,"vpmulhw")
    (e_vpmulld,"vpmulld")
    (e_vpmullq,"vpmullq")
    (e_vpmullw,"vpmullw")
    (e_vpmultishiftqb,"vpmultishiftqb")
    (e_vpmuludq,"vpmuludq")
    (e_vpopcntb,"vpopcntb")
    (e_vpopcntd,"vpopcntd")
    (e_vpopcntq,"vpopcntq")
    (e_vpopcntw,"vpopcntw")
    (e_vpord,"vpord")
    (e_vporq,"vporq")
    (e_vpor,"vpor")
    (e_vpperm,"vpperm")
    (e_vprold,"vprold")
    (e_vprolq,"vprolq")
    (e_vprolvd,"vprolvd")
    (e_vprolvq,"vprolvq")
    (e_vprord,"vprord")
    (e_vprorq,"vprorq")
    (e_vprorvd,"vprorvd")
    (e_vprorvq,"vprorvq")
    (e_vprotb,"vprotb")
    (e_vprotd,"vprotd")
    (e_vprotq,"vprotq")
    (e_vprotw,"vprotw")
    (e_vpsadbw,"vpsadbw")
    (e_vpscatterdd,"vpscatterdd")
    (e_vpscatterdq,"vpscatterdq")
    (e_vpscatterqd,"vpscatterqd")
    (e_vpscatterqq,"vpscatterqq")
    (e_vpshab,"vpshab")
    (e_vpshad,"vpshad")
    (e_vpshaq,"vpshaq")
    (e_vpshaw,"vpshaw")
    (e_vpshlb,"vpshlb")
    (e_vpshldd,"vpshldd")
    (e_vpshldq,"vpshldq")
    (e_vpshldvd,"vpshldvd")
    (e_vpshldvq,"vpshldvq")
    (e_vpshldvw,"vpshldvw")
    (e_vpshldw,"vpshldw")
    (e_vpshld,"vpshld")
    (e_vpshlq,"vpshlq")
    (e_vpshlw,"vpshlw")
    (e_vpshrdd,"vpshrdd")
    (e_vpshrdq,"vpshrdq")
    (e_vpshrdvd,"vpshrdvd")
    (e_vpshrdvq,"vpshrdvq")
    (e_vpshrdvw,"vpshrdvw")
    (e_vpshrdw,"vpshrdw")
    (e_vpshufbitqmb,"vpshufbitqmb")
    (e_vpshufb,"vpshufb")
    (e_vpshufd,"vpshufd")
    (e_vpshufhw,"vpshufhw")
    (e_vpshuflw,"vpshuflw")
    (e_vpsignb,"vpsignb")
    (e_vpsignd,"vpsignd")
    (e_vpsignw,"vpsignw")
    (e_vpslldq,"vpslldq")
    (e_vpslld,"vpslld")
    (e_vpsllq,"vpsllq")
    (e_vpsllvd,"vpsllvd")
    (e_vpsllvq,"vpsllvq")
    (e_vpsllvw,"vpsllvw")
    (e_vpsllw,"vpsllw")
    (e_vpsrad,"vpsrad")
    (e_vpsraq,"vpsraq")
    (e_vpsravd,"vpsravd")
    (e_vpsravq,"vpsravq")
    (e_vpsravw,"vpsravw")
    (e_vpsraw,"vpsraw")
    (e_vpsrldq,"vpsrldq")
    (e_vpsrld,"vpsrld")
    (e_vpsrlq,"vpsrlq")
    (e_vpsrlvd,"vpsrlvd")
    (e_vpsrlvq,"vpsrlvq")
    (e_vpsrlvw,"vpsrlvw")
    (e_vpsrlw,"vpsrlw")
    (e_vpsubb,"vpsubb")
    (e_vpsubd,"vpsubd")
    (e_vpsubq,"vpsubq")
    (e_vpsubsb,"vpsubsb")
    (e_vpsubsw,"vpsubsw")
    (e_vpsubusb,"vpsubusb")
    (e_vpsubusw,"vpsubusw")
    (e_vpsubw,"vpsubw")
    (e_vpternlogd,"vpternlogd")
    (e_vpternlogq,"vpternlogq")
    (e_vptestmb,"vptestmb")
    (e_vptestmd,"vptestmd")
    (e_vptestmq,"vptestmq")
    (e_vptestmw,"vptestmw")
    (e_vptestnmb,"vptestnmb")
    (e_vptestnmd,"vptestnmd")
    (e_vptestnmq,"vptestnmq")
    (e_vptestnmw,"vptestnmw")
    (e_vptest,"vptest")
    (e_vpunpckhbw,"vpunpckhbw")
    (e_vpunpckhdq,"vpunpckhdq")
    (e_vpunpckhqdq,"vpunpckhqdq")
    (e_vpunpckhwd,"vpunpckhwd")
    (e_vpunpcklbw,"vpunpcklbw")
    (e_vpunpckldq,"vpunpckldq")
    (e_vpunpcklqdq,"vpunpcklqdq")
    (e_vpunpcklwd,"vpunpcklwd")
    (e_vpxord,"vpxord")
    (e_vpxorq,"vpxorq")
    (e_vpxor,"vpxor")
    (e_vrangepd,"vrangepd")
    (e_vrangeps,"vrangeps")
    (e_vrangesd,"vrangesd")
    (e_vrangess,"vrangess")
    (e_vrcp14pd,"vrcp14pd")
    (e_vrcp14ps,"vrcp14ps")
    (e_vrcp14sd,"vrcp14sd")
    (e_vrcp14ss,"vrcp14ss")
    (e_vrcp28pd,"vrcp28pd")
    (e_vrcp28ps,"vrcp28ps")
    (e_vrcp28sd,"vrcp28sd")
    (e_vrcp28ss,"vrcp28ss")
    (e_vrcpps,"vrcpps")
    (e_vrcpss,"vrcpss")
    (e_vreducepd,"vreducepd")
    (e_vreduceps,"vreduceps")
    (e_vreducesd,"vreducesd")
    (e_vreducess,"vreducess")
    (e_vrndscalepd,"vrndscalepd")
    (e_vrndscaleps,"vrndscaleps")
    (e_vrndscalesd,"vrndscalesd")
    (e_vrndscaless,"vrndscaless")
    (e_vroundpd,"vroundpd")
    (e_vroundps,"vroundps")
    (e_vroundsd,"vroundsd")
    (e_vroundss,"vroundss")
    (e_vrsqrt14pd,"vrsqrt14pd")
    (e_vrsqrt14ps,"vrsqrt14ps")
    (e_vrsqrt14sd,"vrsqrt14sd")
    (e_vrsqrt14ss,"vrsqrt14ss")
    (e_vrsqrt28pd,"vrsqrt28pd")
    (e_vrsqrt28ps,"vrsqrt28ps")
    (e_vrsqrt28sd,"vrsqrt28sd")
    (e_vrsqrt28ss,"vrsqrt28ss")
    (e_vrsqrtps,"vrsqrtps")
    (e_vrsqrtss,"vrsqrtss")
    (e_vscalefpd,"vscalefpd")
    (e_vscalefps,"vscalefps")
    (e_vscalefsd,"vscalefsd")
    (e_vscalefss,"vscalefss")
    (e_vscatterdpd,"vscatterdpd")
    (e_vscatterdps,"vscatterdps")
    (e_vscatterpf0dpd,"vscatterpf0dpd")
    (e_vscatterpf0dps,"vscatterpf0dps")
    (e_vscatterpf0qpd,"vscatterpf0qpd")
    (e_vscatterpf0qps,"vscatterpf0qps")
    (e_vscatterpf1dpd,"vscatterpf1dpd")
    (e_vscatterpf1dps,"vscatterpf1dps")
    (e_vscatterpf1qpd,"vscatterpf1qpd")
    (e_vscatterpf1qps,"vscatterpf1qps")
    (e_vscatterqpd,"vscatterqpd")
    (e_vscatterqps,"vscatterqps")
    (e_vshuff32x4,"vshuff32x4")
    (e_vshuff64x2,"vshuff64x2")
    (e_vshufi32x4,"vshufi32x4")
    (e_vshufi64x2,"vshufi64x2")
    (e_vshufpd,"vshufpd")
    (e_vshufps,"vshufps")
    (e_vsqrtpd,"vsqrtpd")
    (e_vsqrtps,"vsqrtps")
    (e_vsqrtsd,"vsqrtsd")
    (e_vsqrtss,"vsqrtss")
    (e_vstmxcsr,"vstmxcsr")
    (e_vsubpd,"vsubpd")
    (e_vsubps,"vsubps")
    (e_vsubsd,"vsubsd")
    (e_vsubss,"vsubss")
    (e_vtestpd,"vtestpd")
    (e_vtestps,"vtestps")
    (e_vucomisd,"vucomisd")
    (e_vucomiss,"vucomiss")
    (e_vunpckhpd,"vunpckhpd")
    (e_vunpckhps,"vunpckhps")
    (e_vunpcklpd,"vunpcklpd")
    (e_vunpcklps,"vunpcklps")
    (e_vxorpd,"vxorpd")
    (e_vxorps,"vxorps")
    (e_vzeroall,"vzeroall")
    (e_vzeroupper,"vzeroupper")
    (e_wait,"wait")
    (e_wbinvd,"wbinvd")
    (e_wbnoinvd,"wbnoinvd")
    (e_wrfsbase,"wrfsbase")
    (e_wrgsbase,"wrgsbase")
    (e_wrmsr,"wrmsr")
    (e_wrpkru,"wrpkru")
    (e_wrssd,"wrssd")
    (e_wrssq,"wrssq")
    (e_wrussd,"wrussd")
    (e_wrussq,"wrussq")
    (e_xabort,"xabort")
    (e_xacquire,"xacquire")
    (e_xadd,"xadd")
    (e_xbegin,"xbegin")
    (e_xchg,"xchg")
    (e_fxch,"fxch")
    (e_xcryptcbc,"xcryptcbc")
    (e_xcryptcfb,"xcryptcfb")
    (e_xcryptctr,"xcryptctr")
    (e_xcryptecb,"xcryptecb")
    (e_xcryptofb,"xcryptofb")
    (e_xend,"xend")
    (e_xgetbv,"xgetbv")
    (e_xlatb,"xlatb")
    (e_xor,"xor")
    (e_xorpd,"xorpd")
    (e_xorps,"xorps")
    (e_xrelease,"xrelease")
    (e_xrstor,"xrstor")
    (e_xrstor64,"xrstor64")
    (e_xrstors,"xrstors")
    (e_xrstors64,"xrstors64")
    (e_xsave,"xsave")
    (e_xsave64,"xsave64")
    (e_xsavec,"xsavec")
    (e_xsavec64,"xsavec64")
    (e_xsaveopt,"xsaveopt")
    (e_xsaveopt64,"xsaveopt64")
    (e_xsaves,"xsaves")
    (e_xsaves64,"xsaves64")
    (e_xsetbv,"xsetbv")
    (e_xsha1,"xsha1")
    (e_xsha256,"xsha256")
    (e_xstore,"xstore")
    (e_xtest,"xtest")
    (e_No_Entry, "No_Entry");


}


/******************************  x86 functions  *************************************/

void InstructionDecoder_Capstone::decodeOperands_x86(const Instruction* insn, cs_detail *d) {
    bool isCFT = false;
    bool isCall = false;
    bool isConditional = false;
    InsnCategory cat = insn->getCategory();
     if (cat == c_ReturnInsn) {
        Expression::Ptr ret_addr = makeDereferenceExpression(makeRegisterExpression(x86_64::rsp), u64);
        insn->addSuccessor(ret_addr, false, true, false, false);
        return;
    }
    if(cat == c_BranchInsn || cat == c_CallInsn) {
        isCFT = true;
        if(cat == c_CallInsn) {
            isCall = true;
        }
    }

    if(cat == c_BranchInsn && insn->getOperation().getID() != e_jmp) {
        isConditional = true;
    }
    bool isRela = checkCapstoneGroup(d, (uint8_t)CS_GRP_BRANCH_RELATIVE);
    bool err = false;
    cs_x86* detail = &(d->x86);
    for (uint8_t i = 0; i < detail->op_count; ++i) {
        cs_x86_op* operand = &(detail->operands[i]);
        if (operand->type == X86_OP_REG) {
            Expression::Ptr regAST = makeRegisterExpression(registerTranslation_x86(operand->reg));
            if (isCFT) {
                // if a call or a jump has a register as an operand, 
                // it should not be a conditional jump
                assert(!isConditional);
                insn->addSuccessor(regAST, isCall, true, false, false);
            } else {
                insn->appendOperand(regAST, 
                        (operand->access & CS_AC_READ) != 0,
                        (operand->access & CS_AC_WRITE) != 0, 
                        false);
            }
            //TODO: correctly mark implicit registers
        } else if (operand->type == X86_OP_IMM) {
            Expression::Ptr immAST = Immediate::makeImmediate(Result(s64, operand->imm));
            if (isCFT) {
                // It looks like that Capstone automatically adjust the offset with the instruction length
                Expression::Ptr IP(makeRegisterExpression(MachRegister::getPC(m_Arch)));
                if (isRela) {
                    Expression::Ptr target(makeAddExpression(IP ,immAST, u64));
                    insn->addSuccessor(target, isCall, false, isConditional, false);
                } else {
                    insn->addSuccessor(immAST, isCall, false, isConditional, false);
                }
                if (isConditional)
                    insn->addSuccessor(IP, false, false, true, true);
            } else {
                insn->appendOperand(immAST, false, false, false);
            }
        } else if (operand->type == X86_OP_MEM) {
             Expression::Ptr effectiveAddr;
             x86_op_mem * mem = &(operand->mem);
             // TODO: handle segment registers
             if (mem->base != X86_REG_INVALID) {
                 effectiveAddr = makeRegisterExpression(registerTranslation_x86(mem->base));
             }
             if (mem->index != X86_REG_INVALID) {
                 Expression::Ptr indexAST = makeRegisterExpression(registerTranslation_x86(mem->index));
                 indexAST = makeMultiplyExpression(indexAST,
                             Immediate::makeImmediate(Result(u8, mem->scale)), 
                             u64);
                 if (effectiveAddr)
                     effectiveAddr = makeAddExpression(effectiveAddr, indexAST, u64);
                 else
                     effectiveAddr = indexAST;
             }
             Expression::Ptr immAST = Immediate::makeImmediate(Result(s64, mem->disp));
             if (effectiveAddr)
                 effectiveAddr = makeAddExpression(effectiveAddr, immAST , u64);
             else
                 effectiveAddr = immAST;
             Result_Type type = operandSizeTranslation(operand->size);
             if (type == invalid_type) {
                 err = true;
             }
             Expression::Ptr memAST;
             if (insn->getOperation().getID() == e_lea)
                 memAST = effectiveAddr;
             else
                 memAST = makeDereferenceExpression(effectiveAddr, type);
             if (isCFT) {
                 assert(!isConditional);
                 insn->addSuccessor(memAST, isCall, true, false, false);
             } else {
                 insn->appendOperand(memAST, 
                         (operand->access & CS_AC_READ) != 0,
                         (operand->access & CS_AC_WRITE) != 0, 
                         false);
             }
        } else {
            fprintf(stderr, "Unhandled capstone operand type %d\n", operand->type);
        }
    }
    if (err) fprintf(stderr, "\tinstruction %s\n", insn->format().c_str()); 

}

entryID InstructionDecoder_Capstone::opcodeTranslation_x86(unsigned int cap_id) {
    switch (cap_id) {
        case X86_INS_AAA:
            return e_aaa;
        case X86_INS_AAD:
            return e_aad;
        case X86_INS_AAM:
            return e_aam;
        case X86_INS_AAS:
            return e_aas;
        case X86_INS_FABS:
            return e_fabs;
        case X86_INS_ADC:
            return e_adc;
        case X86_INS_ADCX:
            return e_adcx;
        case X86_INS_ADD:
            return e_add;
        case X86_INS_ADDPD:
            return e_addpd;
        case X86_INS_ADDPS:
            return e_addps;
        case X86_INS_ADDSD:
            return e_addsd;
        case X86_INS_ADDSS:
            return e_addss;
        case X86_INS_ADDSUBPD:
            return e_addsubpd;
        case X86_INS_ADDSUBPS:
            return e_addsubps;
        case X86_INS_FADD:
            return e_fadd;
        case X86_INS_FIADD:
            return e_fiadd;
        case X86_INS_ADOX:
            return e_adox;
        case X86_INS_AESDECLAST:
            return e_aesdeclast;
        case X86_INS_AESDEC:
            return e_aesdec;
        case X86_INS_AESENCLAST:
            return e_aesenclast;
        case X86_INS_AESENC:
            return e_aesenc;
        case X86_INS_AESIMC:
            return e_aesimc;
        case X86_INS_AESKEYGENASSIST:
            return e_aeskeygenassist;
        case X86_INS_AND:
            return e_and;
        case X86_INS_ANDN:
            return e_andn;
        case X86_INS_ANDNPD:
            return e_andnpd;
        case X86_INS_ANDNPS:
            return e_andnps;
        case X86_INS_ANDPD:
            return e_andpd;
        case X86_INS_ANDPS:
            return e_andps;
        case X86_INS_ARPL:
            return e_arpl;
        case X86_INS_BEXTR:
            return e_bextr;
        case X86_INS_BLCFILL:
            return e_blcfill;
        case X86_INS_BLCI:
            return e_blci;
        case X86_INS_BLCIC:
            return e_blcic;
        case X86_INS_BLCMSK:
            return e_blcmsk;
        case X86_INS_BLCS:
            return e_blcs;
        case X86_INS_BLENDPD:
            return e_blendpd;
        case X86_INS_BLENDPS:
            return e_blendps;
        case X86_INS_BLENDVPD:
            return e_blendvpd;
        case X86_INS_BLENDVPS:
            return e_blendvps;
        case X86_INS_BLSFILL:
            return e_blsfill;
        case X86_INS_BLSI:
            return e_blsi;
        case X86_INS_BLSIC:
            return e_blsic;
        case X86_INS_BLSMSK:
            return e_blsmsk;
        case X86_INS_BLSR:
            return e_blsr;
        case X86_INS_BNDCL:
            return e_bndcl;
        case X86_INS_BNDCN:
            return e_bndcn;
        case X86_INS_BNDCU:
            return e_bndcu;
        case X86_INS_BNDLDX:
            return e_bndldx;
        case X86_INS_BNDMK:
            return e_bndmk;
        case X86_INS_BNDMOV:
            return e_bndmov;
        case X86_INS_BNDSTX:
            return e_bndstx;
        case X86_INS_BOUND:
            return e_bound;
        case X86_INS_BSF:
            return e_bsf;
        case X86_INS_BSR:
            return e_bsr;
        case X86_INS_BSWAP:
            return e_bswap;
        case X86_INS_BT:
            return e_bt;
        case X86_INS_BTC:
            return e_btc;
        case X86_INS_BTR:
            return e_btr;
        case X86_INS_BTS:
            return e_bts;
        case X86_INS_BZHI:
            return e_bzhi;
        case X86_INS_CALL:
            return e_call;
        case X86_INS_CBW:
            return e_cbw;
        case X86_INS_CDQ:
            return e_cdq;
        case X86_INS_CDQE:
            return e_cdqe;
        case X86_INS_FCHS:
            return e_fchs;
        case X86_INS_CLAC:
            return e_clac;
        case X86_INS_CLC:
            return e_clc;
        case X86_INS_CLD:
            return e_cld;
        case X86_INS_CLDEMOTE:
            return e_cldemote;
        case X86_INS_CLFLUSH:
            return e_clflush;
        case X86_INS_CLFLUSHOPT:
            return e_clflushopt;
        case X86_INS_CLGI:
            return e_clgi;
        case X86_INS_CLI:
            return e_cli;
        case X86_INS_CLRSSBSY:
            return e_clrssbsy;
        case X86_INS_CLTS:
            return e_clts;
        case X86_INS_CLWB:
            return e_clwb;
        case X86_INS_CLZERO:
            return e_clzero;
        case X86_INS_CMC:
            return e_cmc;
        case X86_INS_CMOVA:
            return e_cmova;
        case X86_INS_CMOVAE:
            return e_cmovae;
        case X86_INS_CMOVB:
            return e_cmovb;
        case X86_INS_CMOVBE:
            return e_cmovbe;
        case X86_INS_FCMOVBE:
            return e_fcmovbe;
        case X86_INS_FCMOVB:
            return e_fcmovb;
        case X86_INS_CMOVE:
            return e_cmove;
        case X86_INS_FCMOVE:
            return e_fcmove;
        case X86_INS_CMOVG:
            return e_cmovg;
        case X86_INS_CMOVGE:
            return e_cmovge;
        case X86_INS_CMOVL:
            return e_cmovl;
        case X86_INS_CMOVLE:
            return e_cmovle;
        case X86_INS_FCMOVNBE:
            return e_fcmovnbe;
        case X86_INS_FCMOVNB:
            return e_fcmovnb;
        case X86_INS_CMOVNE:
            return e_cmovne;
        case X86_INS_FCMOVNE:
            return e_fcmovne;
        case X86_INS_CMOVNO:
            return e_cmovno;
        case X86_INS_CMOVNP:
            return e_cmovnp;
        case X86_INS_FCMOVNU:
            return e_fcmovnu;
        case X86_INS_FCMOVNP:
            return e_fcmovnp;
        case X86_INS_CMOVNS:
            return e_cmovns;
        case X86_INS_CMOVO:
            return e_cmovo;
        case X86_INS_CMOVP:
            return e_cmovp;
        case X86_INS_FCMOVU:
            return e_fcmovu;
        case X86_INS_CMOVS:
            return e_cmovs;
        case X86_INS_CMP:
            return e_cmp;
        case X86_INS_CMPPD:
            return e_cmppd;
        case X86_INS_CMPPS:
            return e_cmpps;
        case X86_INS_CMPSB:
            return e_cmpsb;
        case X86_INS_CMPSD:
            return e_cmpsd;
        case X86_INS_CMPSQ:
            return e_cmpsq;
        case X86_INS_CMPSS:
            return e_cmpss;
        case X86_INS_CMPSW:
            return e_cmpsw;
        case X86_INS_CMPXCHG16B:
            return e_cmpxchg16b;
        case X86_INS_CMPXCHG:
            return e_cmpxchg;
        case X86_INS_CMPXCHG8B:
            return e_cmpxchg8b;
        case X86_INS_COMISD:
            return e_comisd;
        case X86_INS_COMISS:
            return e_comiss;
        case X86_INS_FCOMP:
            return e_fcomp;
        case X86_INS_FCOMPI:
            return e_fcompi;
        case X86_INS_FCOMI:
            return e_fcomi;
        case X86_INS_FCOM:
            return e_fcom;
        case X86_INS_FCOS:
            return e_fcos;
        case X86_INS_CPUID:
            return e_cpuid;
        case X86_INS_CQO:
            return e_cqo;
        case X86_INS_CRC32:
            return e_crc32;
        case X86_INS_CVTDQ2PD:
            return e_cvtdq2pd;
        case X86_INS_CVTDQ2PS:
            return e_cvtdq2ps;
        case X86_INS_CVTPD2DQ:
            return e_cvtpd2dq;
        case X86_INS_CVTPD2PS:
            return e_cvtpd2ps;
        case X86_INS_CVTPS2DQ:
            return e_cvtps2dq;
        case X86_INS_CVTPS2PD:
            return e_cvtps2pd;
        case X86_INS_CVTSD2SI:
            return e_cvtsd2si;
        case X86_INS_CVTSD2SS:
            return e_cvtsd2ss;
        case X86_INS_CVTSI2SD:
            return e_cvtsi2sd;
        case X86_INS_CVTSI2SS:
            return e_cvtsi2ss;
        case X86_INS_CVTSS2SD:
            return e_cvtss2sd;
        case X86_INS_CVTSS2SI:
            return e_cvtss2si;
        case X86_INS_CVTTPD2DQ:
            return e_cvttpd2dq;
        case X86_INS_CVTTPS2DQ:
            return e_cvttps2dq;
        case X86_INS_CVTTSD2SI:
            return e_cvttsd2si;
        case X86_INS_CVTTSS2SI:
            return e_cvttss2si;
        case X86_INS_CWD:
            return e_cwd;
        case X86_INS_CWDE:
            return e_cwde;
        case X86_INS_DAA:
            return e_daa;
        case X86_INS_DAS:
            return e_das;
        case X86_INS_DATA16:
            return e_data16;
        case X86_INS_DEC:
            return e_dec;
        case X86_INS_DIV:
            return e_div;
        case X86_INS_DIVPD:
            return e_divpd;
        case X86_INS_DIVPS:
            return e_divps;
        case X86_INS_FDIVR:
            return e_fdivr;
        case X86_INS_FIDIVR:
            return e_fidivr;
        case X86_INS_FDIVRP:
            return e_fdivrp;
        case X86_INS_DIVSD:
            return e_divsd;
        case X86_INS_DIVSS:
            return e_divss;
        case X86_INS_FDIV:
            return e_fdiv;
        case X86_INS_FIDIV:
            return e_fidiv;
        case X86_INS_FDIVP:
            return e_fdivp;
        case X86_INS_DPPD:
            return e_dppd;
        case X86_INS_DPPS:
            return e_dpps;
        case X86_INS_ENCLS:
            return e_encls;
        case X86_INS_ENCLU:
            return e_enclu;
        case X86_INS_ENCLV:
            return e_enclv;
        case X86_INS_ENDBR32:
            return e_endbr32;
        case X86_INS_ENDBR64:
            return e_endbr64;
        case X86_INS_ENTER:
            return e_enter;
        case X86_INS_EXTRACTPS:
            return e_extractps;
        case X86_INS_EXTRQ:
            return e_extrq;
        case X86_INS_F2XM1:
            return e_f2xm1;
        case X86_INS_LCALL:
            return e_lcall;
        case X86_INS_LJMP:
            return e_ljmp;
        case X86_INS_JMP:
            return e_jmp;
        case X86_INS_FBLD:
            return e_fbld;
        case X86_INS_FBSTP:
            return e_fbstp;
        case X86_INS_FCOMPP:
            return e_fcompp;
        case X86_INS_FDECSTP:
            return e_fdecstp;
        case X86_INS_FDISI8087_NOP:
            return e_nop;
        case X86_INS_FEMMS:
            return e_femms;
        case X86_INS_FENI8087_NOP:
            return e_nop;
        case X86_INS_FFREE:
            return e_ffree;
        case X86_INS_FFREEP:
            return e_ffreep;
        case X86_INS_FICOM:
            return e_ficom;
        case X86_INS_FICOMP:
            return e_ficomp;
        case X86_INS_FINCSTP:
            return e_fincstp;
        case X86_INS_FLDCW:
            return e_fldcw;
        case X86_INS_FLDENV:
            return e_fldenv;
        case X86_INS_FLDL2E:
            return e_fldl2e;
        case X86_INS_FLDL2T:
            return e_fldl2t;
        case X86_INS_FLDLG2:
            return e_fldlg2;
        case X86_INS_FLDLN2:
            return e_fldln2;
        case X86_INS_FLDPI:
            return e_fldpi;
        case X86_INS_FNCLEX:
            return e_fnclex;
        case X86_INS_FNINIT:
            return e_fninit;
        case X86_INS_FNOP:
            return e_fnop;
        case X86_INS_FNSTCW:
            return e_fnstcw;
        case X86_INS_FNSTSW:
            return e_fnstsw;
        case X86_INS_FPATAN:
            return e_fpatan;
        case X86_INS_FSTPNCE:
            return e_fstpnce;
        case X86_INS_FPREM:
            return e_fprem;
        case X86_INS_FPREM1:
            return e_fprem1;
        case X86_INS_FPTAN:
            return e_fptan;
        case X86_INS_FRNDINT:
            return e_frndint;
        case X86_INS_FRSTOR:
            return e_frstor;
        case X86_INS_FNSAVE:
            return e_fnsave;
        case X86_INS_FSCALE:
            return e_fscale;
        case X86_INS_FSETPM:
            return e_fsetpm;
        case X86_INS_FSINCOS:
            return e_fsincos;
        case X86_INS_FNSTENV:
            return e_fnstenv;
        case X86_INS_FXAM:
            return e_fxam;
        case X86_INS_FXRSTOR:
            return e_fxrstor;
        case X86_INS_FXRSTOR64:
            return e_fxrstor64;
        case X86_INS_FXSAVE:
            return e_fxsave;
        case X86_INS_FXSAVE64:
            return e_fxsave64;
        case X86_INS_FXTRACT:
            return e_fxtract;
        case X86_INS_FYL2X:
            return e_fyl2x;
        case X86_INS_FYL2XP1:
            return e_fyl2xp1;
        case X86_INS_GETSEC:
            return e_getsec;
        case X86_INS_GF2P8AFFINEINVQB:
            return e_gf2p8affineinvqb;
        case X86_INS_GF2P8AFFINEQB:
            return e_gf2p8affineqb;
        case X86_INS_GF2P8MULB:
            return e_gf2p8mulb;
        case X86_INS_HADDPD:
            return e_haddpd;
        case X86_INS_HADDPS:
            return e_haddps;
        case X86_INS_HLT:
            return e_hlt;
        case X86_INS_HSUBPD:
            return e_hsubpd;
        case X86_INS_HSUBPS:
            return e_hsubps;
        case X86_INS_IDIV:
            return e_idiv;
        case X86_INS_FILD:
            return e_fild;
        case X86_INS_IMUL:
            return e_imul;
        case X86_INS_IN:
            return e_in;
        case X86_INS_INC:
            return e_inc;
        case X86_INS_INCSSPD:
            return e_incsspd;
        case X86_INS_INCSSPQ:
            return e_incsspq;
        case X86_INS_INSB:
            return e_insb;
        case X86_INS_INSERTPS:
            return e_insertps;
        case X86_INS_INSERTQ:
            return e_insertq;
        case X86_INS_INSD:
            return e_insd;
        case X86_INS_INSW:
            return e_insw;
        case X86_INS_INT:
            return e_int;
        case X86_INS_INT1:
            return e_int1;
        case X86_INS_INT3:
            return e_int3;
        case X86_INS_INTO:
            return e_into;
        case X86_INS_INVD:
            return e_invd;
        case X86_INS_INVEPT:
            return e_invept;
        case X86_INS_INVLPG:
            return e_invlpg;
        case X86_INS_INVLPGA:
            return e_invlpga;
        case X86_INS_INVPCID:
            return e_invpcid;
        case X86_INS_INVVPID:
            return e_invvpid;
        case X86_INS_IRET:
            return e_iret;
        case X86_INS_IRETD:
            return e_iretd;
        case X86_INS_IRETQ:
            return e_iretq;
        case X86_INS_FISTTP:
            return e_fisttp;
        case X86_INS_FIST:
            return e_fist;
        case X86_INS_FISTP:
            return e_fistp;
        case X86_INS_JAE:
            return e_jnb;
        case X86_INS_JA:
            return e_jnbe;
        case X86_INS_JBE:
            return e_jbe;
        case X86_INS_JB:
            return e_jb;
        case X86_INS_JCXZ:
            return e_jcxz;
        case X86_INS_JECXZ:
            return e_jecxz;
        case X86_INS_JE:
            return e_jz;
        case X86_INS_JGE:
            return e_jnl;
        case X86_INS_JG:
            return e_jnle;
        case X86_INS_JLE:
            return e_jle;
        case X86_INS_JL:
            return e_jl;
        case X86_INS_JNE:
            return e_jnz;
        case X86_INS_JNO:
            return e_jno;
        case X86_INS_JNP:
            return e_jnp;
        case X86_INS_JNS:
            return e_jns;
        case X86_INS_JO:
            return e_jo;
        case X86_INS_JP:
            return e_jp;
        case X86_INS_JRCXZ:
            return e_jrcxz;
        case X86_INS_JS:
            return e_js;
        case X86_INS_KADDB:
            return e_kaddb;
        case X86_INS_KADDD:
            return e_kaddd;
        case X86_INS_KADDQ:
            return e_kaddq;
        case X86_INS_KADDW:
            return e_kaddw;
        case X86_INS_KANDB:
            return e_kandb;
        case X86_INS_KANDD:
            return e_kandd;
        case X86_INS_KANDNB:
            return e_kandnb;
        case X86_INS_KANDND:
            return e_kandnd;
        case X86_INS_KANDNQ:
            return e_kandnq;
        case X86_INS_KANDNW:
            return e_kandnw;
        case X86_INS_KANDQ:
            return e_kandq;
        case X86_INS_KANDW:
            return e_kandw;
        case X86_INS_KMOVB:
            return e_kmovb;
        case X86_INS_KMOVD:
            return e_kmovd;
        case X86_INS_KMOVQ:
            return e_kmovq;
        case X86_INS_KMOVW:
            return e_kmovw;
        case X86_INS_KNOTB:
            return e_knotb;
        case X86_INS_KNOTD:
            return e_knotd;
        case X86_INS_KNOTQ:
            return e_knotq;
        case X86_INS_KNOTW:
            return e_knotw;
        case X86_INS_KORB:
            return e_korb;
        case X86_INS_KORD:
            return e_kord;
        case X86_INS_KORQ:
            return e_korq;
        case X86_INS_KORTESTB:
            return e_kortestb;
        case X86_INS_KORTESTD:
            return e_kortestd;
        case X86_INS_KORTESTQ:
            return e_kortestq;
        case X86_INS_KORTESTW:
            return e_kortestw;
        case X86_INS_KORW:
            return e_korw;
        case X86_INS_KSHIFTLB:
            return e_kshiftlb;
        case X86_INS_KSHIFTLD:
            return e_kshiftld;
        case X86_INS_KSHIFTLQ:
            return e_kshiftlq;
        case X86_INS_KSHIFTLW:
            return e_kshiftlw;
        case X86_INS_KSHIFTRB:
            return e_kshiftrb;
        case X86_INS_KSHIFTRD:
            return e_kshiftrd;
        case X86_INS_KSHIFTRQ:
            return e_kshiftrq;
        case X86_INS_KSHIFTRW:
            return e_kshiftrw;
        case X86_INS_KTESTB:
            return e_ktestb;
        case X86_INS_KTESTD:
            return e_ktestd;
        case X86_INS_KTESTQ:
            return e_ktestq;
        case X86_INS_KTESTW:
            return e_ktestw;
        case X86_INS_KUNPCKBW:
            return e_kunpckbw;
        case X86_INS_KUNPCKDQ:
            return e_kunpckdq;
        case X86_INS_KUNPCKWD:
            return e_kunpckwd;
        case X86_INS_KXNORB:
            return e_kxnorb;
        case X86_INS_KXNORD:
            return e_kxnord;
        case X86_INS_KXNORQ:
            return e_kxnorq;
        case X86_INS_KXNORW:
            return e_kxnorw;
        case X86_INS_KXORB:
            return e_kxorb;
        case X86_INS_KXORD:
            return e_kxord;
        case X86_INS_KXORQ:
            return e_kxorq;
        case X86_INS_KXORW:
            return e_kxorw;
        case X86_INS_LAHF:
            return e_lahf;
        case X86_INS_LAR:
            return e_lar;
        case X86_INS_LDDQU:
            return e_lddqu;
        case X86_INS_LDMXCSR:
            return e_ldmxcsr;
        case X86_INS_LDS:
            return e_lds;
        case X86_INS_FLDZ:
            return e_fldz;
        case X86_INS_FLD1:
            return e_fld1;
        case X86_INS_FLD:
            return e_fld;
        case X86_INS_LEA:
            return e_lea;
        case X86_INS_LEAVE:
            return e_leave;
        case X86_INS_LES:
            return e_les;
        case X86_INS_LFENCE:
            return e_lfence;
        case X86_INS_LFS:
            return e_lfs;
        case X86_INS_LGDT:
            return e_lgdt;
        case X86_INS_LGS:
            return e_lgs;
        case X86_INS_LIDT:
            return e_lidt;
        case X86_INS_LLDT:
            return e_lldt;
        case X86_INS_LLWPCB:
            return e_llwpcb;
        case X86_INS_LMSW:
            return e_lmsw;
        case X86_INS_LOCK:
            return e_lock;
        case X86_INS_LODSB:
            return e_lodsb;
        case X86_INS_LODSD:
            return e_lodsd;
        case X86_INS_LODSQ:
            return e_lodsq;
        case X86_INS_LODSW:
            return e_lodsw;
        case X86_INS_LOOP:
            return e_loop;
        case X86_INS_LOOPE:
            return e_loope;
        case X86_INS_LOOPNE:
            return e_loopne;
        case X86_INS_RETF:
            return e_retf;
        case X86_INS_RETFQ:
            return e_retfq;
        case X86_INS_LSL:
            return e_lsl;
        case X86_INS_LSS:
            return e_lss;
        case X86_INS_LTR:
            return e_ltr;
        case X86_INS_LWPINS:
            return e_lwpins;
        case X86_INS_LWPVAL:
            return e_lwpval;
        case X86_INS_LZCNT:
            return e_lzcnt;
        case X86_INS_MASKMOVDQU:
            return e_maskmovdqu;
        case X86_INS_MAXPD:
            return e_maxpd;
        case X86_INS_MAXPS:
            return e_maxps;
        case X86_INS_MAXSD:
            return e_maxsd;
        case X86_INS_MAXSS:
            return e_maxss;
        case X86_INS_MFENCE:
            return e_mfence;
        case X86_INS_MINPD:
            return e_minpd;
        case X86_INS_MINPS:
            return e_minps;
        case X86_INS_MINSD:
            return e_minsd;
        case X86_INS_MINSS:
            return e_minss;
        case X86_INS_CVTPD2PI:
            return e_cvtpd2pi;
        case X86_INS_CVTPI2PD:
            return e_cvtpi2pd;
        case X86_INS_CVTPI2PS:
            return e_cvtpi2ps;
        case X86_INS_CVTPS2PI:
            return e_cvtps2pi;
        case X86_INS_CVTTPD2PI:
            return e_cvttpd2pi;
        case X86_INS_CVTTPS2PI:
            return e_cvttps2pi;
        case X86_INS_EMMS:
            return e_emms;
        case X86_INS_MASKMOVQ:
            return e_maskmovq;
        case X86_INS_MOVD:
            return e_movd;
        case X86_INS_MOVQ:
            return e_movq;
        case X86_INS_MOVDQ2Q:
            return e_movdq2q;
        case X86_INS_MOVNTQ:
            return e_movntq;
        case X86_INS_MOVQ2DQ:
            return e_movq2dq;
        case X86_INS_PABSB:
            return e_pabsb;
        case X86_INS_PABSD:
            return e_pabsd;
        case X86_INS_PABSW:
            return e_pabsw;
        case X86_INS_PACKSSDW:
            return e_packssdw;
        case X86_INS_PACKSSWB:
            return e_packsswb;
        case X86_INS_PACKUSWB:
            return e_packuswb;
        case X86_INS_PADDB:
            return e_paddb;
        case X86_INS_PADDD:
            return e_paddd;
        case X86_INS_PADDQ:
            return e_paddq;
        case X86_INS_PADDSB:
            return e_paddsb;
        case X86_INS_PADDSW:
            return e_paddsw;
        case X86_INS_PADDUSB:
            return e_paddusb;
        case X86_INS_PADDUSW:
            return e_paddusw;
        case X86_INS_PADDW:
            return e_paddw;
        case X86_INS_PALIGNR:
            return e_palignr;
        case X86_INS_PANDN:
            return e_pandn;
        case X86_INS_PAND:
            return e_pand;
        case X86_INS_PAVGB:
            return e_pavgb;
        case X86_INS_PAVGW:
            return e_pavgw;
        case X86_INS_PCMPEQB:
            return e_pcmpeqb;
        case X86_INS_PCMPEQD:
            return e_pcmpeqd;
        case X86_INS_PCMPEQW:
            return e_pcmpeqw;
        case X86_INS_PCMPGTB:
            return e_pcmpgtb;
        case X86_INS_PCMPGTD:
            return e_pcmpgtd;
        case X86_INS_PCMPGTW:
            return e_pcmpgtw;
        case X86_INS_PEXTRW:
            return e_pextrw;
        case X86_INS_PHADDD:
            return e_phaddd;
        case X86_INS_PHADDSW:
            return e_phaddsw;
        case X86_INS_PHADDW:
            return e_phaddw;
        case X86_INS_PHSUBD:
            return e_phsubd;
        case X86_INS_PHSUBSW:
            return e_phsubsw;
        case X86_INS_PHSUBW:
            return e_phsubw;
        case X86_INS_PINSRW:
            return e_pinsrw;
        case X86_INS_PMADDUBSW:
            return e_pmaddubsw;
        case X86_INS_PMADDWD:
            return e_pmaddwd;
        case X86_INS_PMAXSW:
            return e_pmaxsw;
        case X86_INS_PMAXUB:
            return e_pmaxub;
        case X86_INS_PMINSW:
            return e_pminsw;
        case X86_INS_PMINUB:
            return e_pminub;
        case X86_INS_PMOVMSKB:
            return e_pmovmskb;
        case X86_INS_PMULHRSW:
            return e_pmulhrsw;
        case X86_INS_PMULHUW:
            return e_pmulhuw;
        case X86_INS_PMULHW:
            return e_pmulhw;
        case X86_INS_PMULLW:
            return e_pmullw;
        case X86_INS_PMULUDQ:
            return e_pmuludq;
        case X86_INS_POR:
            return e_por;
        case X86_INS_PSADBW:
            return e_psadbw;
        case X86_INS_PSHUFB:
            return e_pshufb;
        case X86_INS_PSHUFW:
            return e_pshufw;
        case X86_INS_PSIGNB:
            return e_psignb;
        case X86_INS_PSIGND:
            return e_psignd;
        case X86_INS_PSIGNW:
            return e_psignw;
        case X86_INS_PSLLD:
            return e_pslld;
        case X86_INS_PSLLQ:
            return e_psllq;
        case X86_INS_PSLLW:
            return e_psllw;
        case X86_INS_PSRAD:
            return e_psrad;
        case X86_INS_PSRAW:
            return e_psraw;
        case X86_INS_PSRLD:
            return e_psrld;
        case X86_INS_PSRLQ:
            return e_psrlq;
        case X86_INS_PSRLW:
            return e_psrlw;
        case X86_INS_PSUBB:
            return e_psubb;
        case X86_INS_PSUBD:
            return e_psubd;
        case X86_INS_PSUBQ:
            return e_psubq;
        case X86_INS_PSUBSB:
            return e_psubsb;
        case X86_INS_PSUBSW:
            return e_psubsw;
        case X86_INS_PSUBUSB:
            return e_psubusb;
        case X86_INS_PSUBUSW:
            return e_psubusw;
        case X86_INS_PSUBW:
            return e_psubw;
        case X86_INS_PUNPCKHBW:
            return e_punpckhbw;
        case X86_INS_PUNPCKHDQ:
            return e_punpckhdq;
        case X86_INS_PUNPCKHWD:
            return e_punpckhwd;
        case X86_INS_PUNPCKLBW:
            return e_punpcklbw;
        case X86_INS_PUNPCKLDQ:
            return e_punpckldq;
        case X86_INS_PUNPCKLWD:
            return e_punpcklwd;
        case X86_INS_PXOR:
            return e_pxor;
        case X86_INS_MONITORX:
            return e_monitorx;
        case X86_INS_MONITOR:
            return e_monitor;
        case X86_INS_MONTMUL:
            return e_montmul;
        case X86_INS_MOV:
            return e_mov;
        case X86_INS_MOVABS:
            return e_movabs;
        case X86_INS_MOVAPD:
            return e_movapd;
        case X86_INS_MOVAPS:
            return e_movaps;
        case X86_INS_MOVBE:
            return e_movbe;
        case X86_INS_MOVDDUP:
            return e_movddup;
        case X86_INS_MOVDIR64B:
            return e_movdir64b;
        case X86_INS_MOVDIRI:
            return e_movdiri;
        case X86_INS_MOVDQA:
            return e_movdqa;
        case X86_INS_MOVDQU:
            return e_movdqu;
        case X86_INS_MOVHLPS:
            return e_movhlps;
        case X86_INS_MOVHPD:
            return e_movhpd;
        case X86_INS_MOVHPS:
            return e_movhps;
        case X86_INS_MOVLHPS:
            return e_movlhps;
        case X86_INS_MOVLPD:
            return e_movlpd;
        case X86_INS_MOVLPS:
            return e_movlps;
        case X86_INS_MOVMSKPD:
            return e_movmskpd;
        case X86_INS_MOVMSKPS:
            return e_movmskps;
        case X86_INS_MOVNTDQA:
            return e_movntdqa;
        case X86_INS_MOVNTDQ:
            return e_movntdq;
        case X86_INS_MOVNTI:
            return e_movnti;
        case X86_INS_MOVNTPD:
            return e_movntpd;
        case X86_INS_MOVNTPS:
            return e_movntps;
        case X86_INS_MOVNTSD:
            return e_movntsd;
        case X86_INS_MOVNTSS:
            return e_movntss;
        case X86_INS_MOVSB:
            return e_movsb;
        case X86_INS_MOVSD:
            return e_movsd;
        case X86_INS_MOVSHDUP:
            return e_movshdup;
        case X86_INS_MOVSLDUP:
            return e_movsldup;
        case X86_INS_MOVSQ:
            return e_movsq;
        case X86_INS_MOVSS:
            return e_movss;
        case X86_INS_MOVSW:
            return e_movsw;
        case X86_INS_MOVSX:
            return e_movsx;
        case X86_INS_MOVSXD:
            return e_movsxd;
        case X86_INS_MOVUPD:
            return e_movupd;
        case X86_INS_MOVUPS:
            return e_movups;
        case X86_INS_MOVZX:
            return e_movzx;
        case X86_INS_MPSADBW:
            return e_mpsadbw;
        case X86_INS_MUL:
            return e_mul;
        case X86_INS_MULPD:
            return e_mulpd;
        case X86_INS_MULPS:
            return e_mulps;
        case X86_INS_MULSD:
            return e_mulsd;
        case X86_INS_MULSS:
            return e_mulss;
        case X86_INS_MULX:
            return e_mulx;
        case X86_INS_FMUL:
            return e_fmul;
        case X86_INS_FIMUL:
            return e_fimul;
        case X86_INS_FMULP:
            return e_fmulp;
        case X86_INS_MWAITX:
            return e_mwaitx;
        case X86_INS_MWAIT:
            return e_mwait;
        case X86_INS_NEG:
            return e_neg;
        case X86_INS_NOP:
            return e_nop;
        case X86_INS_NOT:
            return e_not;
        case X86_INS_OR:
            return e_or;
        case X86_INS_ORPD:
            return e_orpd;
        case X86_INS_ORPS:
            return e_orps;
        case X86_INS_OUT:
            return e_out;
        case X86_INS_OUTSB:
            return e_outsb;
        case X86_INS_OUTSD:
            return e_outsd;
        case X86_INS_OUTSW:
            return e_outsw;
        case X86_INS_PACKUSDW:
            return e_packusdw;
        case X86_INS_PAUSE:
            return e_pause;
        case X86_INS_PAVGUSB:
            return e_pavgusb;
        case X86_INS_PBLENDVB:
            return e_pblendvb;
        case X86_INS_PBLENDW:
            return e_pblendw;
        case X86_INS_PCLMULQDQ:
            return e_pclmulqdq;
        case X86_INS_PCMPEQQ:
            return e_pcmpeqq;
        case X86_INS_PCMPESTRI:
            return e_pcmpestri;
        case X86_INS_PCMPESTRM:
            return e_pcmpestrm;
        case X86_INS_PCMPGTQ:
            return e_pcmpgtq;
        case X86_INS_PCMPISTRI:
            return e_pcmpistri;
        case X86_INS_PCMPISTRM:
            return e_pcmpistrm;
        case X86_INS_PCONFIG:
            return e_pconfig;
        case X86_INS_PDEP:
            return e_pdep;
        case X86_INS_PEXT:
            return e_pext;
        case X86_INS_PEXTRB:
            return e_pextrb;
        case X86_INS_PEXTRD:
            return e_pextrd;
        case X86_INS_PEXTRQ:
            return e_pextrq;
        case X86_INS_PF2ID:
            return e_pf2id;
        case X86_INS_PF2IW:
            return e_pf2iw;
        case X86_INS_PFACC:
            return e_pfacc;
        case X86_INS_PFADD:
            return e_pfadd;
        case X86_INS_PFCMPEQ:
            return e_pfcmpeq;
        case X86_INS_PFCMPGE:
            return e_pfcmpge;
        case X86_INS_PFCMPGT:
            return e_pfcmpgt;
        case X86_INS_PFMAX:
            return e_pfmax;
        case X86_INS_PFMIN:
            return e_pfmin;
        case X86_INS_PFMUL:
            return e_pfmul;
        case X86_INS_PFNACC:
            return e_pfnacc;
        case X86_INS_PFPNACC:
            return e_pfpnacc;
        case X86_INS_PFRCPIT1:
            return e_pfrcpit1;
        case X86_INS_PFRCPIT2:
            return e_pfrcpit2;
        case X86_INS_PFRCP:
            return e_pfrcp;
        case X86_INS_PFRSQIT1:
            return e_pfrsqit1;
        case X86_INS_PFRSQRT:
            return e_pfrsqrt;
        case X86_INS_PFSUBR:
            return e_pfsubr;
        case X86_INS_PFSUB:
            return e_pfsub;
        case X86_INS_PHMINPOSUW:
            return e_phminposuw;
        case X86_INS_PI2FD:
            return e_pi2fd;
        case X86_INS_PI2FW:
            return e_pi2fw;
        case X86_INS_PINSRB:
            return e_pinsrb;
        case X86_INS_PINSRD:
            return e_pinsrd;
        case X86_INS_PINSRQ:
            return e_pinsrq;
        case X86_INS_PMAXSB:
            return e_pmaxsb;
        case X86_INS_PMAXSD:
            return e_pmaxsd;
        case X86_INS_PMAXUD:
            return e_pmaxud;
        case X86_INS_PMAXUW:
            return e_pmaxuw;
        case X86_INS_PMINSB:
            return e_pminsb;
        case X86_INS_PMINSD:
            return e_pminsd;
        case X86_INS_PMINUD:
            return e_pminud;
        case X86_INS_PMINUW:
            return e_pminuw;
        case X86_INS_PMOVSXBD:
            return e_pmovsxbd;
        case X86_INS_PMOVSXBQ:
            return e_pmovsxbq;
        case X86_INS_PMOVSXBW:
            return e_pmovsxbw;
        case X86_INS_PMOVSXDQ:
            return e_pmovsxdq;
        case X86_INS_PMOVSXWD:
            return e_pmovsxwd;
        case X86_INS_PMOVSXWQ:
            return e_pmovsxwq;
        case X86_INS_PMOVZXBD:
            return e_pmovzxbd;
        case X86_INS_PMOVZXBQ:
            return e_pmovzxbq;
        case X86_INS_PMOVZXBW:
            return e_pmovzxbw;
        case X86_INS_PMOVZXDQ:
            return e_pmovzxdq;
        case X86_INS_PMOVZXWD:
            return e_pmovzxwd;
        case X86_INS_PMOVZXWQ:
            return e_pmovzxwq;
        case X86_INS_PMULDQ:
            return e_pmuldq;
        case X86_INS_PMULHRW:
            return e_pmulhrw;
        case X86_INS_PMULLD:
            return e_pmulld;
        case X86_INS_POP:
            return e_pop;
        case X86_INS_POPAW:
            return e_popaw;
        case X86_INS_POPAL:
            return e_popal;
        case X86_INS_POPCNT:
            return e_popcnt;
        case X86_INS_POPF:
            return e_popf;
        case X86_INS_POPFD:
            return e_popfd;
        case X86_INS_POPFQ:
            return e_popfq;
        case X86_INS_PREFETCH:
            return e_prefetch;
        case X86_INS_PREFETCHNTA:
            return e_prefetchnta;
        case X86_INS_PREFETCHT0:
            return e_prefetcht0;
        case X86_INS_PREFETCHT1:
            return e_prefetcht1;
        case X86_INS_PREFETCHT2:
            return e_prefetcht2;
        case X86_INS_PREFETCHW:
            return e_prefetchw;
        case X86_INS_PREFETCHWT1:
            return e_prefetchwt1;
        case X86_INS_PSHUFD:
            return e_pshufd;
        case X86_INS_PSHUFHW:
            return e_pshufhw;
        case X86_INS_PSHUFLW:
            return e_pshuflw;
        case X86_INS_PSLLDQ:
            return e_pslldq;
        case X86_INS_PSRLDQ:
            return e_psrldq;
        case X86_INS_PSWAPD:
            return e_pswapd;
        case X86_INS_PTEST:
            return e_ptest;
        case X86_INS_PTWRITE:
            return e_ptwrite;
        case X86_INS_PUNPCKHQDQ:
            return e_punpckhqdq;
        case X86_INS_PUNPCKLQDQ:
            return e_punpcklqdq;
        case X86_INS_PUSH:
            return e_push;
        case X86_INS_PUSHAW:
            return e_pushaw;
        case X86_INS_PUSHAL:
            return e_pushal;
        case X86_INS_PUSHF:
            return e_pushf;
        case X86_INS_PUSHFD:
            return e_pushfd;
        case X86_INS_PUSHFQ:
            return e_pushfq;
        case X86_INS_RCL:
            return e_rcl;
        case X86_INS_RCPPS:
            return e_rcpps;
        case X86_INS_RCPSS:
            return e_rcpss;
        case X86_INS_RCR:
            return e_rcr;
        case X86_INS_RDFSBASE:
            return e_rdfsbase;
        case X86_INS_RDGSBASE:
            return e_rdgsbase;
        case X86_INS_RDMSR:
            return e_rdmsr;
        case X86_INS_RDPID:
            return e_rdpid;
        case X86_INS_RDPKRU:
            return e_rdpkru;
        case X86_INS_RDPMC:
            return e_rdpmc;
        case X86_INS_RDRAND:
            return e_rdrand;
        case X86_INS_RDSEED:
            return e_rdseed;
        case X86_INS_RDSSPD:
            return e_rdsspd;
        case X86_INS_RDSSPQ:
            return e_rdsspq;
        case X86_INS_RDTSC:
            return e_rdtsc;
        case X86_INS_RDTSCP:
            return e_rdtscp;
        case X86_INS_REPNE:
            return e_repne;
        case X86_INS_REP:
            return e_rep;
        case X86_INS_RET:
            return e_ret_near;
        case X86_INS_REX64:
            return e_rex64;
        case X86_INS_ROL:
            return e_rol;
        case X86_INS_ROR:
            return e_ror;
        case X86_INS_RORX:
            return e_rorx;
        case X86_INS_ROUNDPD:
            return e_roundpd;
        case X86_INS_ROUNDPS:
            return e_roundps;
        case X86_INS_ROUNDSD:
            return e_roundsd;
        case X86_INS_ROUNDSS:
            return e_roundss;
        case X86_INS_RSM:
            return e_rsm;
        case X86_INS_RSQRTPS:
            return e_rsqrtps;
        case X86_INS_RSQRTSS:
            return e_rsqrtss;
        case X86_INS_RSTORSSP:
            return e_rstorssp;
        case X86_INS_SAHF:
            return e_sahf;
        case X86_INS_SAL:
            return e_sal;
        case X86_INS_SALC:
            return e_salc;
        case X86_INS_SAR:
            return e_sar;
        case X86_INS_SARX:
            return e_sarx;
        case X86_INS_SAVEPREVSSP:
            return e_saveprevssp;
        case X86_INS_SBB:
            return e_sbb;
        case X86_INS_SCASB:
            return e_scasb;
        case X86_INS_SCASD:
            return e_scasd;
        case X86_INS_SCASQ:
            return e_scasq;
        case X86_INS_SCASW:
            return e_scasw;
        case X86_INS_SETAE:
            return e_setae;
        case X86_INS_SETA:
            return e_seta;
        case X86_INS_SETBE:
            return e_setbe;
        case X86_INS_SETB:
            return e_setb;
        case X86_INS_SETE:
            return e_sete;
        case X86_INS_SETGE:
            return e_setge;
        case X86_INS_SETG:
            return e_setg;
        case X86_INS_SETLE:
            return e_setle;
        case X86_INS_SETL:
            return e_setl;
        case X86_INS_SETNE:
            return e_setne;
        case X86_INS_SETNO:
            return e_setno;
        case X86_INS_SETNP:
            return e_setnp;
        case X86_INS_SETNS:
            return e_setns;
        case X86_INS_SETO:
            return e_seto;
        case X86_INS_SETP:
            return e_setp;
        case X86_INS_SETSSBSY:
            return e_setssbsy;
        case X86_INS_SETS:
            return e_sets;
        case X86_INS_SFENCE:
            return e_sfence;
        case X86_INS_SGDT:
            return e_sgdt;
        case X86_INS_SHA1MSG1:
            return e_sha1msg1;
        case X86_INS_SHA1MSG2:
            return e_sha1msg2;
        case X86_INS_SHA1NEXTE:
            return e_sha1nexte;
        case X86_INS_SHA1RNDS4:
            return e_sha1rnds4;
        case X86_INS_SHA256MSG1:
            return e_sha256msg1;
        case X86_INS_SHA256MSG2:
            return e_sha256msg2;
        case X86_INS_SHA256RNDS2:
            return e_sha256rnds2;
        case X86_INS_SHL:
            return e_shl;
        case X86_INS_SHLD:
            return e_shld;
        case X86_INS_SHLX:
            return e_shlx;
        case X86_INS_SHR:
            return e_shr;
        case X86_INS_SHRD:
            return e_shrd;
        case X86_INS_SHRX:
            return e_shrx;
        case X86_INS_SHUFPD:
            return e_shufpd;
        case X86_INS_SHUFPS:
            return e_shufps;
        case X86_INS_SIDT:
            return e_sidt;
        case X86_INS_FSIN:
            return e_fsin;
        case X86_INS_SKINIT:
            return e_skinit;
        case X86_INS_SLDT:
            return e_sldt;
        case X86_INS_SLWPCB:
            return e_slwpcb;
        case X86_INS_SMSW:
            return e_smsw;
        case X86_INS_SQRTPD:
            return e_sqrtpd;
        case X86_INS_SQRTPS:
            return e_sqrtps;
        case X86_INS_SQRTSD:
            return e_sqrtsd;
        case X86_INS_SQRTSS:
            return e_sqrtss;
        case X86_INS_FSQRT:
            return e_fsqrt;
        case X86_INS_STAC:
            return e_stac;
        case X86_INS_STC:
            return e_stc;
        case X86_INS_STD:
            return e_std;
        case X86_INS_STGI:
            return e_stgi;
        case X86_INS_STI:
            return e_sti;
        case X86_INS_STMXCSR:
            return e_stmxcsr;
        case X86_INS_STOSB:
            return e_stosb;
        case X86_INS_STOSD:
            return e_stosd;
        case X86_INS_STOSQ:
            return e_stosq;
        case X86_INS_STOSW:
            return e_stosw;
        case X86_INS_STR:
            return e_str;
        case X86_INS_FST:
            return e_fst;
        case X86_INS_FSTP:
            return e_fstp;
        case X86_INS_SUB:
            return e_sub;
        case X86_INS_SUBPD:
            return e_subpd;
        case X86_INS_SUBPS:
            return e_subps;
        case X86_INS_FSUBR:
            return e_fsubr;
        case X86_INS_FISUBR:
            return e_fisubr;
        case X86_INS_FSUBRP:
            return e_fsubrp;
        case X86_INS_SUBSD:
            return e_subsd;
        case X86_INS_SUBSS:
            return e_subss;
        case X86_INS_FSUB:
            return e_fsub;
        case X86_INS_FISUB:
            return e_fisub;
        case X86_INS_FSUBP:
            return e_fsubp;
        case X86_INS_SWAPGS:
            return e_swapgs;
        case X86_INS_SYSCALL:
            return e_syscall;
        case X86_INS_SYSENTER:
            return e_sysenter;
        case X86_INS_SYSEXIT:
            return e_sysexit;
        case X86_INS_SYSEXITQ:
            return e_sysexitq;
        case X86_INS_SYSRET:
            return e_sysret;
        case X86_INS_SYSRETQ:
            return e_sysretq;
        case X86_INS_T1MSKC:
            return e_t1mskc;
        case X86_INS_TEST:
            return e_test;
        case X86_INS_TPAUSE:
            return e_tpause;
        case X86_INS_FTST:
            return e_ftst;
        case X86_INS_TZCNT:
            return e_tzcnt;
        case X86_INS_TZMSK:
            return e_tzmsk;
        case X86_INS_UCOMISD:
            return e_ucomisd;
        case X86_INS_UCOMISS:
            return e_ucomiss;
        case X86_INS_FUCOMPI:
            return e_fucompi;
        case X86_INS_FUCOMI:
            return e_fucomi;
        case X86_INS_FUCOMPP:
            return e_fucompp;
        case X86_INS_FUCOMP:
            return e_fucomp;
        case X86_INS_FUCOM:
            return e_fucom;
        case X86_INS_UD0:
            return e_ud0;
        case X86_INS_UD1:
            return e_ud1;
        case X86_INS_UD2:
            return e_ud2;
        case X86_INS_UMONITOR:
            return e_umonitor;
        case X86_INS_UMWAIT:
            return e_umwait;
        case X86_INS_UNPCKHPD:
            return e_unpckhpd;
        case X86_INS_UNPCKHPS:
            return e_unpckhps;
        case X86_INS_UNPCKLPD:
            return e_unpcklpd;
        case X86_INS_UNPCKLPS:
            return e_unpcklps;
        case X86_INS_V4FMADDPS:
            return e_v4fmaddps;
        case X86_INS_V4FMADDSS:
            return e_v4fmaddss;
        case X86_INS_V4FNMADDPS:
            return e_v4fnmaddps;
        case X86_INS_V4FNMADDSS:
            return e_v4fnmaddss;
        case X86_INS_VADDPD:
            return e_vaddpd;
        case X86_INS_VADDPS:
            return e_vaddps;
        case X86_INS_VADDSD:
            return e_vaddsd;
        case X86_INS_VADDSS:
            return e_vaddss;
        case X86_INS_VADDSUBPD:
            return e_vaddsubpd;
        case X86_INS_VADDSUBPS:
            return e_vaddsubps;
        case X86_INS_VAESDECLAST:
            return e_vaesdeclast;
        case X86_INS_VAESDEC:
            return e_vaesdec;
        case X86_INS_VAESENCLAST:
            return e_vaesenclast;
        case X86_INS_VAESENC:
            return e_vaesenc;
        case X86_INS_VAESIMC:
            return e_vaesimc;
        case X86_INS_VAESKEYGENASSIST:
            return e_vaeskeygenassist;
        case X86_INS_VALIGND:
            return e_valignd;
        case X86_INS_VALIGNQ:
            return e_valignq;
        case X86_INS_VANDNPD:
            return e_vandnpd;
        case X86_INS_VANDNPS:
            return e_vandnps;
        case X86_INS_VANDPD:
            return e_vandpd;
        case X86_INS_VANDPS:
            return e_vandps;
        case X86_INS_VBLENDMPD:
            return e_vblendmpd;
        case X86_INS_VBLENDMPS:
            return e_vblendmps;
        case X86_INS_VBLENDPD:
            return e_vblendpd;
        case X86_INS_VBLENDPS:
            return e_vblendps;
        case X86_INS_VBLENDVPD:
            return e_vblendvpd;
        case X86_INS_VBLENDVPS:
            return e_vblendvps;
        case X86_INS_VBROADCASTF128:
            return e_vbroadcastf128;
        case X86_INS_VBROADCASTF32X2:
            return e_vbroadcastf32x2;
        case X86_INS_VBROADCASTF32X4:
            return e_vbroadcastf32x4;
        case X86_INS_VBROADCASTF32X8:
            return e_vbroadcastf32x8;
        case X86_INS_VBROADCASTF64X2:
            return e_vbroadcastf64x2;
        case X86_INS_VBROADCASTF64X4:
            return e_vbroadcastf64x4;
        case X86_INS_VBROADCASTI128:
            return e_vbroadcasti128;
        case X86_INS_VBROADCASTI32X2:
            return e_vbroadcasti32x2;
        case X86_INS_VBROADCASTI32X4:
            return e_vbroadcasti32x4;
        case X86_INS_VBROADCASTI32X8:
            return e_vbroadcasti32x8;
        case X86_INS_VBROADCASTI64X2:
            return e_vbroadcasti64x2;
        case X86_INS_VBROADCASTI64X4:
            return e_vbroadcasti64x4;
        case X86_INS_VBROADCASTSD:
            return e_vbroadcastsd;
        case X86_INS_VBROADCASTSS:
            return e_vbroadcastss;
        case X86_INS_VCMP:
            return e_vcmp;
        case X86_INS_VCMPPD:
            return e_vcmppd;
        case X86_INS_VCMPPS:
            return e_vcmpps;
        case X86_INS_VCMPSD:
            return e_vcmpsd;
        case X86_INS_VCMPSS:
            return e_vcmpss;
        case X86_INS_VCOMISD:
            return e_vcomisd;
        case X86_INS_VCOMISS:
            return e_vcomiss;
        case X86_INS_VCOMPRESSPD:
            return e_vcompresspd;
        case X86_INS_VCOMPRESSPS:
            return e_vcompressps;
        case X86_INS_VCVTDQ2PD:
            return e_vcvtdq2pd;
        case X86_INS_VCVTDQ2PS:
            return e_vcvtdq2ps;
        case X86_INS_VCVTPD2DQ:
            return e_vcvtpd2dq;
        case X86_INS_VCVTPD2PS:
            return e_vcvtpd2ps;
        case X86_INS_VCVTPD2QQ:
            return e_vcvtpd2qq;
        case X86_INS_VCVTPD2UDQ:
            return e_vcvtpd2udq;
        case X86_INS_VCVTPD2UQQ:
            return e_vcvtpd2uqq;
        case X86_INS_VCVTPH2PS:
            return e_vcvtph2ps;
        case X86_INS_VCVTPS2DQ:
            return e_vcvtps2dq;
        case X86_INS_VCVTPS2PD:
            return e_vcvtps2pd;
        case X86_INS_VCVTPS2PH:
            return e_vcvtps2ph;
        case X86_INS_VCVTPS2QQ:
            return e_vcvtps2qq;
        case X86_INS_VCVTPS2UDQ:
            return e_vcvtps2udq;
        case X86_INS_VCVTPS2UQQ:
            return e_vcvtps2uqq;
        case X86_INS_VCVTQQ2PD:
            return e_vcvtqq2pd;
        case X86_INS_VCVTQQ2PS:
            return e_vcvtqq2ps;
        case X86_INS_VCVTSD2SI:
            return e_vcvtsd2si;
        case X86_INS_VCVTSD2SS:
            return e_vcvtsd2ss;
        case X86_INS_VCVTSD2USI:
            return e_vcvtsd2usi;
        case X86_INS_VCVTSI2SD:
            return e_vcvtsi2sd;
        case X86_INS_VCVTSI2SS:
            return e_vcvtsi2ss;
        case X86_INS_VCVTSS2SD:
            return e_vcvtss2sd;
        case X86_INS_VCVTSS2SI:
            return e_vcvtss2si;
        case X86_INS_VCVTSS2USI:
            return e_vcvtss2usi;
        case X86_INS_VCVTTPD2DQ:
            return e_vcvttpd2dq;
        case X86_INS_VCVTTPD2QQ:
            return e_vcvttpd2qq;
        case X86_INS_VCVTTPD2UDQ:
            return e_vcvttpd2udq;
        case X86_INS_VCVTTPD2UQQ:
            return e_vcvttpd2uqq;
        case X86_INS_VCVTTPS2DQ:
            return e_vcvttps2dq;
        case X86_INS_VCVTTPS2QQ:
            return e_vcvttps2qq;
        case X86_INS_VCVTTPS2UDQ:
            return e_vcvttps2udq;
        case X86_INS_VCVTTPS2UQQ:
            return e_vcvttps2uqq;
        case X86_INS_VCVTTSD2SI:
            return e_vcvttsd2si;
        case X86_INS_VCVTTSD2USI:
            return e_vcvttsd2usi;
        case X86_INS_VCVTTSS2SI:
            return e_vcvttss2si;
        case X86_INS_VCVTTSS2USI:
            return e_vcvttss2usi;
        case X86_INS_VCVTUDQ2PD:
            return e_vcvtudq2pd;
        case X86_INS_VCVTUDQ2PS:
            return e_vcvtudq2ps;
        case X86_INS_VCVTUQQ2PD:
            return e_vcvtuqq2pd;
        case X86_INS_VCVTUQQ2PS:
            return e_vcvtuqq2ps;
        case X86_INS_VCVTUSI2SD:
            return e_vcvtusi2sd;
        case X86_INS_VCVTUSI2SS:
            return e_vcvtusi2ss;
        case X86_INS_VDBPSADBW:
            return e_vdbpsadbw;
        case X86_INS_VDIVPD:
            return e_vdivpd;
        case X86_INS_VDIVPS:
            return e_vdivps;
        case X86_INS_VDIVSD:
            return e_vdivsd;
        case X86_INS_VDIVSS:
            return e_vdivss;
        case X86_INS_VDPPD:
            return e_vdppd;
        case X86_INS_VDPPS:
            return e_vdpps;
        case X86_INS_VERR:
            return e_verr;
        case X86_INS_VERW:
            return e_verw;
        case X86_INS_VEXP2PD:
            return e_vexp2pd;
        case X86_INS_VEXP2PS:
            return e_vexp2ps;
        case X86_INS_VEXPANDPD:
            return e_vexpandpd;
        case X86_INS_VEXPANDPS:
            return e_vexpandps;
        case X86_INS_VEXTRACTF128:
            return e_vextractf128;
        case X86_INS_VEXTRACTF32X4:
            return e_vextractf32x4;
        case X86_INS_VEXTRACTF32X8:
            return e_vextractf32x8;
        case X86_INS_VEXTRACTF64X2:
            return e_vextractf64x2;
        case X86_INS_VEXTRACTF64X4:
            return e_vextractf64x4;
        case X86_INS_VEXTRACTI128:
            return e_vextracti128;
        case X86_INS_VEXTRACTI32X4:
            return e_vextracti32x4;
        case X86_INS_VEXTRACTI32X8:
            return e_vextracti32x8;
        case X86_INS_VEXTRACTI64X2:
            return e_vextracti64x2;
        case X86_INS_VEXTRACTI64X4:
            return e_vextracti64x4;
        case X86_INS_VEXTRACTPS:
            return e_vextractps;
        case X86_INS_VFIXUPIMMPD:
            return e_vfixupimmpd;
        case X86_INS_VFIXUPIMMPS:
            return e_vfixupimmps;
        case X86_INS_VFIXUPIMMSD:
            return e_vfixupimmsd;
        case X86_INS_VFIXUPIMMSS:
            return e_vfixupimmss;
        case X86_INS_VFMADD132PD:
            return e_vfmadd132pd;
        case X86_INS_VFMADD132PS:
            return e_vfmadd132ps;
        case X86_INS_VFMADD132SD:
            return e_vfmadd132sd;
        case X86_INS_VFMADD132SS:
            return e_vfmadd132ss;
        case X86_INS_VFMADD213PD:
            return e_vfmadd213pd;
        case X86_INS_VFMADD213PS:
            return e_vfmadd213ps;
        case X86_INS_VFMADD213SD:
            return e_vfmadd213sd;
        case X86_INS_VFMADD213SS:
            return e_vfmadd213ss;
        case X86_INS_VFMADD231PD:
            return e_vfmadd231pd;
        case X86_INS_VFMADD231PS:
            return e_vfmadd231ps;
        case X86_INS_VFMADD231SD:
            return e_vfmadd231sd;
        case X86_INS_VFMADD231SS:
            return e_vfmadd231ss;
        case X86_INS_VFMADDPD:
            return e_vfmaddpd;
        case X86_INS_VFMADDPS:
            return e_vfmaddps;
        case X86_INS_VFMADDSD:
            return e_vfmaddsd;
        case X86_INS_VFMADDSS:
            return e_vfmaddss;
        case X86_INS_VFMADDSUB132PD:
            return e_vfmaddsub132pd;
        case X86_INS_VFMADDSUB132PS:
            return e_vfmaddsub132ps;
        case X86_INS_VFMADDSUB213PD:
            return e_vfmaddsub213pd;
        case X86_INS_VFMADDSUB213PS:
            return e_vfmaddsub213ps;
        case X86_INS_VFMADDSUB231PD:
            return e_vfmaddsub231pd;
        case X86_INS_VFMADDSUB231PS:
            return e_vfmaddsub231ps;
        case X86_INS_VFMADDSUBPD:
            return e_vfmaddsubpd;
        case X86_INS_VFMADDSUBPS:
            return e_vfmaddsubps;
        case X86_INS_VFMSUB132PD:
            return e_vfmsub132pd;
        case X86_INS_VFMSUB132PS:
            return e_vfmsub132ps;
        case X86_INS_VFMSUB132SD:
            return e_vfmsub132sd;
        case X86_INS_VFMSUB132SS:
            return e_vfmsub132ss;
        case X86_INS_VFMSUB213PD:
            return e_vfmsub213pd;
        case X86_INS_VFMSUB213PS:
            return e_vfmsub213ps;
        case X86_INS_VFMSUB213SD:
            return e_vfmsub213sd;
        case X86_INS_VFMSUB213SS:
            return e_vfmsub213ss;
        case X86_INS_VFMSUB231PD:
            return e_vfmsub231pd;
        case X86_INS_VFMSUB231PS:
            return e_vfmsub231ps;
        case X86_INS_VFMSUB231SD:
            return e_vfmsub231sd;
        case X86_INS_VFMSUB231SS:
            return e_vfmsub231ss;
        case X86_INS_VFMSUBADD132PD:
            return e_vfmsubadd132pd;
        case X86_INS_VFMSUBADD132PS:
            return e_vfmsubadd132ps;
        case X86_INS_VFMSUBADD213PD:
            return e_vfmsubadd213pd;
        case X86_INS_VFMSUBADD213PS:
            return e_vfmsubadd213ps;
        case X86_INS_VFMSUBADD231PD:
            return e_vfmsubadd231pd;
        case X86_INS_VFMSUBADD231PS:
            return e_vfmsubadd231ps;
        case X86_INS_VFMSUBADDPD:
            return e_vfmsubaddpd;
        case X86_INS_VFMSUBADDPS:
            return e_vfmsubaddps;
        case X86_INS_VFMSUBPD:
            return e_vfmsubpd;
        case X86_INS_VFMSUBPS:
            return e_vfmsubps;
        case X86_INS_VFMSUBSD:
            return e_vfmsubsd;
        case X86_INS_VFMSUBSS:
            return e_vfmsubss;
        case X86_INS_VFNMADD132PD:
            return e_vfnmadd132pd;
        case X86_INS_VFNMADD132PS:
            return e_vfnmadd132ps;
        case X86_INS_VFNMADD132SD:
            return e_vfnmadd132sd;
        case X86_INS_VFNMADD132SS:
            return e_vfnmadd132ss;
        case X86_INS_VFNMADD213PD:
            return e_vfnmadd213pd;
        case X86_INS_VFNMADD213PS:
            return e_vfnmadd213ps;
        case X86_INS_VFNMADD213SD:
            return e_vfnmadd213sd;
        case X86_INS_VFNMADD213SS:
            return e_vfnmadd213ss;
        case X86_INS_VFNMADD231PD:
            return e_vfnmadd231pd;
        case X86_INS_VFNMADD231PS:
            return e_vfnmadd231ps;
        case X86_INS_VFNMADD231SD:
            return e_vfnmadd231sd;
        case X86_INS_VFNMADD231SS:
            return e_vfnmadd231ss;
        case X86_INS_VFNMADDPD:
            return e_vfnmaddpd;
        case X86_INS_VFNMADDPS:
            return e_vfnmaddps;
        case X86_INS_VFNMADDSD:
            return e_vfnmaddsd;
        case X86_INS_VFNMADDSS:
            return e_vfnmaddss;
        case X86_INS_VFNMSUB132PD:
            return e_vfnmsub132pd;
        case X86_INS_VFNMSUB132PS:
            return e_vfnmsub132ps;
        case X86_INS_VFNMSUB132SD:
            return e_vfnmsub132sd;
        case X86_INS_VFNMSUB132SS:
            return e_vfnmsub132ss;
        case X86_INS_VFNMSUB213PD:
            return e_vfnmsub213pd;
        case X86_INS_VFNMSUB213PS:
            return e_vfnmsub213ps;
        case X86_INS_VFNMSUB213SD:
            return e_vfnmsub213sd;
        case X86_INS_VFNMSUB213SS:
            return e_vfnmsub213ss;
        case X86_INS_VFNMSUB231PD:
            return e_vfnmsub231pd;
        case X86_INS_VFNMSUB231PS:
            return e_vfnmsub231ps;
        case X86_INS_VFNMSUB231SD:
            return e_vfnmsub231sd;
        case X86_INS_VFNMSUB231SS:
            return e_vfnmsub231ss;
        case X86_INS_VFNMSUBPD:
            return e_vfnmsubpd;
        case X86_INS_VFNMSUBPS:
            return e_vfnmsubps;
        case X86_INS_VFNMSUBSD:
            return e_vfnmsubsd;
        case X86_INS_VFNMSUBSS:
            return e_vfnmsubss;
        case X86_INS_VFPCLASSPD:
            return e_vfpclasspd;
        case X86_INS_VFPCLASSPS:
            return e_vfpclassps;
        case X86_INS_VFPCLASSSD:
            return e_vfpclasssd;
        case X86_INS_VFPCLASSSS:
            return e_vfpclassss;
        case X86_INS_VFRCZPD:
            return e_vfrczpd;
        case X86_INS_VFRCZPS:
            return e_vfrczps;
        case X86_INS_VFRCZSD:
            return e_vfrczsd;
        case X86_INS_VFRCZSS:
            return e_vfrczss;
        case X86_INS_VGATHERDPD:
            return e_vgatherdpd;
        case X86_INS_VGATHERDPS:
            return e_vgatherdps;
        case X86_INS_VGATHERPF0DPD:
            return e_vgatherpf0dpd;
        case X86_INS_VGATHERPF0DPS:
            return e_vgatherpf0dps;
        case X86_INS_VGATHERPF0QPD:
            return e_vgatherpf0qpd;
        case X86_INS_VGATHERPF0QPS:
            return e_vgatherpf0qps;
        case X86_INS_VGATHERPF1DPD:
            return e_vgatherpf1dpd;
        case X86_INS_VGATHERPF1DPS:
            return e_vgatherpf1dps;
        case X86_INS_VGATHERPF1QPD:
            return e_vgatherpf1qpd;
        case X86_INS_VGATHERPF1QPS:
            return e_vgatherpf1qps;
        case X86_INS_VGATHERQPD:
            return e_vgatherqpd;
        case X86_INS_VGATHERQPS:
            return e_vgatherqps;
        case X86_INS_VGETEXPPD:
            return e_vgetexppd;
        case X86_INS_VGETEXPPS:
            return e_vgetexpps;
        case X86_INS_VGETEXPSD:
            return e_vgetexpsd;
        case X86_INS_VGETEXPSS:
            return e_vgetexpss;
        case X86_INS_VGETMANTPD:
            return e_vgetmantpd;
        case X86_INS_VGETMANTPS:
            return e_vgetmantps;
        case X86_INS_VGETMANTSD:
            return e_vgetmantsd;
        case X86_INS_VGETMANTSS:
            return e_vgetmantss;
        case X86_INS_VGF2P8AFFINEINVQB:
            return e_vgf2p8affineinvqb;
        case X86_INS_VGF2P8AFFINEQB:
            return e_vgf2p8affineqb;
        case X86_INS_VGF2P8MULB:
            return e_vgf2p8mulb;
        case X86_INS_VHADDPD:
            return e_vhaddpd;
        case X86_INS_VHADDPS:
            return e_vhaddps;
        case X86_INS_VHSUBPD:
            return e_vhsubpd;
        case X86_INS_VHSUBPS:
            return e_vhsubps;
        case X86_INS_VINSERTF128:
            return e_vinsertf128;
        case X86_INS_VINSERTF32X4:
            return e_vinsertf32x4;
        case X86_INS_VINSERTF32X8:
            return e_vinsertf32x8;
        case X86_INS_VINSERTF64X2:
            return e_vinsertf64x2;
        case X86_INS_VINSERTF64X4:
            return e_vinsertf64x4;
        case X86_INS_VINSERTI128:
            return e_vinserti128;
        case X86_INS_VINSERTI32X4:
            return e_vinserti32x4;
        case X86_INS_VINSERTI32X8:
            return e_vinserti32x8;
        case X86_INS_VINSERTI64X2:
            return e_vinserti64x2;
        case X86_INS_VINSERTI64X4:
            return e_vinserti64x4;
        case X86_INS_VINSERTPS:
            return e_vinsertps;
        case X86_INS_VLDDQU:
            return e_vlddqu;
        case X86_INS_VLDMXCSR:
            return e_vldmxcsr;
        case X86_INS_VMASKMOVDQU:
            return e_vmaskmovdqu;
        case X86_INS_VMASKMOVPD:
            return e_vmaskmovpd;
        case X86_INS_VMASKMOVPS:
            return e_vmaskmovps;
        case X86_INS_VMAXPD:
            return e_vmaxpd;
        case X86_INS_VMAXPS:
            return e_vmaxps;
        case X86_INS_VMAXSD:
            return e_vmaxsd;
        case X86_INS_VMAXSS:
            return e_vmaxss;
        case X86_INS_VMCALL:
            return e_vmcall;
        case X86_INS_VMCLEAR:
            return e_vmclear;
        case X86_INS_VMFUNC:
            return e_vmfunc;
        case X86_INS_VMINPD:
            return e_vminpd;
        case X86_INS_VMINPS:
            return e_vminps;
        case X86_INS_VMINSD:
            return e_vminsd;
        case X86_INS_VMINSS:
            return e_vminss;
        case X86_INS_VMLAUNCH:
            return e_vmlaunch;
        case X86_INS_VMLOAD:
            return e_vmload;
        case X86_INS_VMMCALL:
            return e_vmmcall;
        case X86_INS_VMOVQ:
            return e_vmovq;
        case X86_INS_VMOVAPD:
            return e_vmovapd;
        case X86_INS_VMOVAPS:
            return e_vmovaps;
        case X86_INS_VMOVDDUP:
            return e_vmovddup;
        case X86_INS_VMOVD:
            return e_vmovd;
        case X86_INS_VMOVDQA32:
            return e_vmovdqa32;
        case X86_INS_VMOVDQA64:
            return e_vmovdqa64;
        case X86_INS_VMOVDQA:
            return e_vmovdqa;
        case X86_INS_VMOVDQU16:
            return e_vmovdqu16;
        case X86_INS_VMOVDQU32:
            return e_vmovdqu32;
        case X86_INS_VMOVDQU64:
            return e_vmovdqu64;
        case X86_INS_VMOVDQU8:
            return e_vmovdqu8;
        case X86_INS_VMOVDQU:
            return e_vmovdqu;
        case X86_INS_VMOVHLPS:
            return e_vmovhlps;
        case X86_INS_VMOVHPD:
            return e_vmovhpd;
        case X86_INS_VMOVHPS:
            return e_vmovhps;
        case X86_INS_VMOVLHPS:
            return e_vmovlhps;
        case X86_INS_VMOVLPD:
            return e_vmovlpd;
        case X86_INS_VMOVLPS:
            return e_vmovlps;
        case X86_INS_VMOVMSKPD:
            return e_vmovmskpd;
        case X86_INS_VMOVMSKPS:
            return e_vmovmskps;
        case X86_INS_VMOVNTDQA:
            return e_vmovntdqa;
        case X86_INS_VMOVNTDQ:
            return e_vmovntdq;
        case X86_INS_VMOVNTPD:
            return e_vmovntpd;
        case X86_INS_VMOVNTPS:
            return e_vmovntps;
        case X86_INS_VMOVSD:
            return e_vmovsd;
        case X86_INS_VMOVSHDUP:
            return e_vmovshdup;
        case X86_INS_VMOVSLDUP:
            return e_vmovsldup;
        case X86_INS_VMOVSS:
            return e_vmovss;
        case X86_INS_VMOVUPD:
            return e_vmovupd;
        case X86_INS_VMOVUPS:
            return e_vmovups;
        case X86_INS_VMPSADBW:
            return e_vmpsadbw;
        case X86_INS_VMPTRLD:
            return e_vmptrld;
        case X86_INS_VMPTRST:
            return e_vmptrst;
        case X86_INS_VMREAD:
            return e_vmread;
        case X86_INS_VMRESUME:
            return e_vmresume;
        case X86_INS_VMRUN:
            return e_vmrun;
        case X86_INS_VMSAVE:
            return e_vmsave;
        case X86_INS_VMULPD:
            return e_vmulpd;
        case X86_INS_VMULPS:
            return e_vmulps;
        case X86_INS_VMULSD:
            return e_vmulsd;
        case X86_INS_VMULSS:
            return e_vmulss;
        case X86_INS_VMWRITE:
            return e_vmwrite;
        case X86_INS_VMXOFF:
            return e_vmxoff;
        case X86_INS_VMXON:
            return e_vmxon;
        case X86_INS_VORPD:
            return e_vorpd;
        case X86_INS_VORPS:
            return e_vorps;
        case X86_INS_VP4DPWSSDS:
            return e_vp4dpwssds;
        case X86_INS_VP4DPWSSD:
            return e_vp4dpwssd;
        case X86_INS_VPABSB:
            return e_vpabsb;
        case X86_INS_VPABSD:
            return e_vpabsd;
        case X86_INS_VPABSQ:
            return e_vpabsq;
        case X86_INS_VPABSW:
            return e_vpabsw;
        case X86_INS_VPACKSSDW:
            return e_vpackssdw;
        case X86_INS_VPACKSSWB:
            return e_vpacksswb;
        case X86_INS_VPACKUSDW:
            return e_vpackusdw;
        case X86_INS_VPACKUSWB:
            return e_vpackuswb;
        case X86_INS_VPADDB:
            return e_vpaddb;
        case X86_INS_VPADDD:
            return e_vpaddd;
        case X86_INS_VPADDQ:
            return e_vpaddq;
        case X86_INS_VPADDSB:
            return e_vpaddsb;
        case X86_INS_VPADDSW:
            return e_vpaddsw;
        case X86_INS_VPADDUSB:
            return e_vpaddusb;
        case X86_INS_VPADDUSW:
            return e_vpaddusw;
        case X86_INS_VPADDW:
            return e_vpaddw;
        case X86_INS_VPALIGNR:
            return e_vpalignr;
        case X86_INS_VPANDD:
            return e_vpandd;
        case X86_INS_VPANDND:
            return e_vpandnd;
        case X86_INS_VPANDNQ:
            return e_vpandnq;
        case X86_INS_VPANDN:
            return e_vpandn;
        case X86_INS_VPANDQ:
            return e_vpandq;
        case X86_INS_VPAND:
            return e_vpand;
        case X86_INS_VPAVGB:
            return e_vpavgb;
        case X86_INS_VPAVGW:
            return e_vpavgw;
        case X86_INS_VPBLENDD:
            return e_vpblendd;
        case X86_INS_VPBLENDMB:
            return e_vpblendmb;
        case X86_INS_VPBLENDMD:
            return e_vpblendmd;
        case X86_INS_VPBLENDMQ:
            return e_vpblendmq;
        case X86_INS_VPBLENDMW:
            return e_vpblendmw;
        case X86_INS_VPBLENDVB:
            return e_vpblendvb;
        case X86_INS_VPBLENDW:
            return e_vpblendw;
        case X86_INS_VPBROADCASTB:
            return e_vpbroadcastb;
        case X86_INS_VPBROADCASTD:
            return e_vpbroadcastd;
        case X86_INS_VPBROADCASTMB2Q:
            return e_vpbroadcastmb2q;
        case X86_INS_VPBROADCASTMW2D:
            return e_vpbroadcastmw2d;
        case X86_INS_VPBROADCASTQ:
            return e_vpbroadcastq;
        case X86_INS_VPBROADCASTW:
            return e_vpbroadcastw;
        case X86_INS_VPCLMULQDQ:
            return e_vpclmulqdq;
        case X86_INS_VPCMOV:
            return e_vpcmov;
        case X86_INS_VPCMP:
            return e_vpcmp;
        case X86_INS_VPCMPB:
            return e_vpcmpb;
        case X86_INS_VPCMPD:
            return e_vpcmpd;
        case X86_INS_VPCMPEQB:
            return e_vpcmpeqb;
        case X86_INS_VPCMPEQD:
            return e_vpcmpeqd;
        case X86_INS_VPCMPEQQ:
            return e_vpcmpeqq;
        case X86_INS_VPCMPEQW:
            return e_vpcmpeqw;
        case X86_INS_VPCMPESTRI:
            return e_vpcmpestri;
        case X86_INS_VPCMPESTRM:
            return e_vpcmpestrm;
        case X86_INS_VPCMPGTB:
            return e_vpcmpgtb;
        case X86_INS_VPCMPGTD:
            return e_vpcmpgtd;
        case X86_INS_VPCMPGTQ:
            return e_vpcmpgtq;
        case X86_INS_VPCMPGTW:
            return e_vpcmpgtw;
        case X86_INS_VPCMPISTRI:
            return e_vpcmpistri;
        case X86_INS_VPCMPISTRM:
            return e_vpcmpistrm;
        case X86_INS_VPCMPQ:
            return e_vpcmpq;
        case X86_INS_VPCMPUB:
            return e_vpcmpub;
        case X86_INS_VPCMPUD:
            return e_vpcmpud;
        case X86_INS_VPCMPUQ:
            return e_vpcmpuq;
        case X86_INS_VPCMPUW:
            return e_vpcmpuw;
        case X86_INS_VPCMPW:
            return e_vpcmpw;
        case X86_INS_VPCOM:
            return e_vpcom;
        case X86_INS_VPCOMB:
            return e_vpcomb;
        case X86_INS_VPCOMD:
            return e_vpcomd;
        case X86_INS_VPCOMPRESSB:
            return e_vpcompressb;
        case X86_INS_VPCOMPRESSD:
            return e_vpcompressd;
        case X86_INS_VPCOMPRESSQ:
            return e_vpcompressq;
        case X86_INS_VPCOMPRESSW:
            return e_vpcompressw;
        case X86_INS_VPCOMQ:
            return e_vpcomq;
        case X86_INS_VPCOMUB:
            return e_vpcomub;
        case X86_INS_VPCOMUD:
            return e_vpcomud;
        case X86_INS_VPCOMUQ:
            return e_vpcomuq;
        case X86_INS_VPCOMUW:
            return e_vpcomuw;
        case X86_INS_VPCOMW:
            return e_vpcomw;
        case X86_INS_VPCONFLICTD:
            return e_vpconflictd;
        case X86_INS_VPCONFLICTQ:
            return e_vpconflictq;
        case X86_INS_VPDPBUSDS:
            return e_vpdpbusds;
        case X86_INS_VPDPBUSD:
            return e_vpdpbusd;
        case X86_INS_VPDPWSSDS:
            return e_vpdpwssds;
        case X86_INS_VPDPWSSD:
            return e_vpdpwssd;
        case X86_INS_VPERM2F128:
            return e_vperm2f128;
        case X86_INS_VPERM2I128:
            return e_vperm2i128;
        case X86_INS_VPERMB:
            return e_vpermb;
        case X86_INS_VPERMD:
            return e_vpermd;
        case X86_INS_VPERMI2B:
            return e_vpermi2b;
        case X86_INS_VPERMI2D:
            return e_vpermi2d;
        case X86_INS_VPERMI2PD:
            return e_vpermi2pd;
        case X86_INS_VPERMI2PS:
            return e_vpermi2ps;
        case X86_INS_VPERMI2Q:
            return e_vpermi2q;
        case X86_INS_VPERMI2W:
            return e_vpermi2w;
        case X86_INS_VPERMIL2PD:
            return e_vpermil2pd;
        case X86_INS_VPERMILPD:
            return e_vpermilpd;
        case X86_INS_VPERMIL2PS:
            return e_vpermil2ps;
        case X86_INS_VPERMILPS:
            return e_vpermilps;
        case X86_INS_VPERMPD:
            return e_vpermpd;
        case X86_INS_VPERMPS:
            return e_vpermps;
        case X86_INS_VPERMQ:
            return e_vpermq;
        case X86_INS_VPERMT2B:
            return e_vpermt2b;
        case X86_INS_VPERMT2D:
            return e_vpermt2d;
        case X86_INS_VPERMT2PD:
            return e_vpermt2pd;
        case X86_INS_VPERMT2PS:
            return e_vpermt2ps;
        case X86_INS_VPERMT2Q:
            return e_vpermt2q;
        case X86_INS_VPERMT2W:
            return e_vpermt2w;
        case X86_INS_VPERMW:
            return e_vpermw;
        case X86_INS_VPEXPANDB:
            return e_vpexpandb;
        case X86_INS_VPEXPANDD:
            return e_vpexpandd;
        case X86_INS_VPEXPANDQ:
            return e_vpexpandq;
        case X86_INS_VPEXPANDW:
            return e_vpexpandw;
        case X86_INS_VPEXTRB:
            return e_vpextrb;
        case X86_INS_VPEXTRD:
            return e_vpextrd;
        case X86_INS_VPEXTRQ:
            return e_vpextrq;
        case X86_INS_VPEXTRW:
            return e_vpextrw;
        case X86_INS_VPGATHERDD:
            return e_vpgatherdd;
        case X86_INS_VPGATHERDQ:
            return e_vpgatherdq;
        case X86_INS_VPGATHERQD:
            return e_vpgatherqd;
        case X86_INS_VPGATHERQQ:
            return e_vpgatherqq;
        case X86_INS_VPHADDBD:
            return e_vphaddbd;
        case X86_INS_VPHADDBQ:
            return e_vphaddbq;
        case X86_INS_VPHADDBW:
            return e_vphaddbw;
        case X86_INS_VPHADDDQ:
            return e_vphadddq;
        case X86_INS_VPHADDD:
            return e_vphaddd;
        case X86_INS_VPHADDSW:
            return e_vphaddsw;
        case X86_INS_VPHADDUBD:
            return e_vphaddubd;
        case X86_INS_VPHADDUBQ:
            return e_vphaddubq;
        case X86_INS_VPHADDUBW:
            return e_vphaddubw;
        case X86_INS_VPHADDUDQ:
            return e_vphaddudq;
        case X86_INS_VPHADDUWD:
            return e_vphadduwd;
        case X86_INS_VPHADDUWQ:
            return e_vphadduwq;
        case X86_INS_VPHADDWD:
            return e_vphaddwd;
        case X86_INS_VPHADDWQ:
            return e_vphaddwq;
        case X86_INS_VPHADDW:
            return e_vphaddw;
        case X86_INS_VPHMINPOSUW:
            return e_vphminposuw;
        case X86_INS_VPHSUBBW:
            return e_vphsubbw;
        case X86_INS_VPHSUBDQ:
            return e_vphsubdq;
        case X86_INS_VPHSUBD:
            return e_vphsubd;
        case X86_INS_VPHSUBSW:
            return e_vphsubsw;
        case X86_INS_VPHSUBWD:
            return e_vphsubwd;
        case X86_INS_VPHSUBW:
            return e_vphsubw;
        case X86_INS_VPINSRB:
            return e_vpinsrb;
        case X86_INS_VPINSRD:
            return e_vpinsrd;
        case X86_INS_VPINSRQ:
            return e_vpinsrq;
        case X86_INS_VPINSRW:
            return e_vpinsrw;
        case X86_INS_VPLZCNTD:
            return e_vplzcntd;
        case X86_INS_VPLZCNTQ:
            return e_vplzcntq;
        case X86_INS_VPMACSDD:
            return e_vpmacsdd;
        case X86_INS_VPMACSDQH:
            return e_vpmacsdqh;
        case X86_INS_VPMACSDQL:
            return e_vpmacsdql;
        case X86_INS_VPMACSSDD:
            return e_vpmacssdd;
        case X86_INS_VPMACSSDQH:
            return e_vpmacssdqh;
        case X86_INS_VPMACSSDQL:
            return e_vpmacssdql;
        case X86_INS_VPMACSSWD:
            return e_vpmacsswd;
        case X86_INS_VPMACSSWW:
            return e_vpmacssww;
        case X86_INS_VPMACSWD:
            return e_vpmacswd;
        case X86_INS_VPMACSWW:
            return e_vpmacsww;
        case X86_INS_VPMADCSSWD:
            return e_vpmadcsswd;
        case X86_INS_VPMADCSWD:
            return e_vpmadcswd;
        case X86_INS_VPMADD52HUQ:
            return e_vpmadd52huq;
        case X86_INS_VPMADD52LUQ:
            return e_vpmadd52luq;
        case X86_INS_VPMADDUBSW:
            return e_vpmaddubsw;
        case X86_INS_VPMADDWD:
            return e_vpmaddwd;
        case X86_INS_VPMASKMOVD:
            return e_vpmaskmovd;
        case X86_INS_VPMASKMOVQ:
            return e_vpmaskmovq;
        case X86_INS_VPMAXSB:
            return e_vpmaxsb;
        case X86_INS_VPMAXSD:
            return e_vpmaxsd;
        case X86_INS_VPMAXSQ:
            return e_vpmaxsq;
        case X86_INS_VPMAXSW:
            return e_vpmaxsw;
        case X86_INS_VPMAXUB:
            return e_vpmaxub;
        case X86_INS_VPMAXUD:
            return e_vpmaxud;
        case X86_INS_VPMAXUQ:
            return e_vpmaxuq;
        case X86_INS_VPMAXUW:
            return e_vpmaxuw;
        case X86_INS_VPMINSB:
            return e_vpminsb;
        case X86_INS_VPMINSD:
            return e_vpminsd;
        case X86_INS_VPMINSQ:
            return e_vpminsq;
        case X86_INS_VPMINSW:
            return e_vpminsw;
        case X86_INS_VPMINUB:
            return e_vpminub;
        case X86_INS_VPMINUD:
            return e_vpminud;
        case X86_INS_VPMINUQ:
            return e_vpminuq;
        case X86_INS_VPMINUW:
            return e_vpminuw;
        case X86_INS_VPMOVB2M:
            return e_vpmovb2m;
        case X86_INS_VPMOVD2M:
            return e_vpmovd2m;
        case X86_INS_VPMOVDB:
            return e_vpmovdb;
        case X86_INS_VPMOVDW:
            return e_vpmovdw;
        case X86_INS_VPMOVM2B:
            return e_vpmovm2b;
        case X86_INS_VPMOVM2D:
            return e_vpmovm2d;
        case X86_INS_VPMOVM2Q:
            return e_vpmovm2q;
        case X86_INS_VPMOVM2W:
            return e_vpmovm2w;
        case X86_INS_VPMOVMSKB:
            return e_vpmovmskb;
        case X86_INS_VPMOVQ2M:
            return e_vpmovq2m;
        case X86_INS_VPMOVQB:
            return e_vpmovqb;
        case X86_INS_VPMOVQD:
            return e_vpmovqd;
        case X86_INS_VPMOVQW:
            return e_vpmovqw;
        case X86_INS_VPMOVSDB:
            return e_vpmovsdb;
        case X86_INS_VPMOVSDW:
            return e_vpmovsdw;
        case X86_INS_VPMOVSQB:
            return e_vpmovsqb;
        case X86_INS_VPMOVSQD:
            return e_vpmovsqd;
        case X86_INS_VPMOVSQW:
            return e_vpmovsqw;
        case X86_INS_VPMOVSWB:
            return e_vpmovswb;
        case X86_INS_VPMOVSXBD:
            return e_vpmovsxbd;
        case X86_INS_VPMOVSXBQ:
            return e_vpmovsxbq;
        case X86_INS_VPMOVSXBW:
            return e_vpmovsxbw;
        case X86_INS_VPMOVSXDQ:
            return e_vpmovsxdq;
        case X86_INS_VPMOVSXWD:
            return e_vpmovsxwd;
        case X86_INS_VPMOVSXWQ:
            return e_vpmovsxwq;
        case X86_INS_VPMOVUSDB:
            return e_vpmovusdb;
        case X86_INS_VPMOVUSDW:
            return e_vpmovusdw;
        case X86_INS_VPMOVUSQB:
            return e_vpmovusqb;
        case X86_INS_VPMOVUSQD:
            return e_vpmovusqd;
        case X86_INS_VPMOVUSQW:
            return e_vpmovusqw;
        case X86_INS_VPMOVUSWB:
            return e_vpmovuswb;
        case X86_INS_VPMOVW2M:
            return e_vpmovw2m;
        case X86_INS_VPMOVWB:
            return e_vpmovwb;
        case X86_INS_VPMOVZXBD:
            return e_vpmovzxbd;
        case X86_INS_VPMOVZXBQ:
            return e_vpmovzxbq;
        case X86_INS_VPMOVZXBW:
            return e_vpmovzxbw;
        case X86_INS_VPMOVZXDQ:
            return e_vpmovzxdq;
        case X86_INS_VPMOVZXWD:
            return e_vpmovzxwd;
        case X86_INS_VPMOVZXWQ:
            return e_vpmovzxwq;
        case X86_INS_VPMULDQ:
            return e_vpmuldq;
        case X86_INS_VPMULHRSW:
            return e_vpmulhrsw;
        case X86_INS_VPMULHUW:
            return e_vpmulhuw;
        case X86_INS_VPMULHW:
            return e_vpmulhw;
        case X86_INS_VPMULLD:
            return e_vpmulld;
        case X86_INS_VPMULLQ:
            return e_vpmullq;
        case X86_INS_VPMULLW:
            return e_vpmullw;
        case X86_INS_VPMULTISHIFTQB:
            return e_vpmultishiftqb;
        case X86_INS_VPMULUDQ:
            return e_vpmuludq;
        case X86_INS_VPOPCNTB:
            return e_vpopcntb;
        case X86_INS_VPOPCNTD:
            return e_vpopcntd;
        case X86_INS_VPOPCNTQ:
            return e_vpopcntq;
        case X86_INS_VPOPCNTW:
            return e_vpopcntw;
        case X86_INS_VPORD:
            return e_vpord;
        case X86_INS_VPORQ:
            return e_vporq;
        case X86_INS_VPOR:
            return e_vpor;
        case X86_INS_VPPERM:
            return e_vpperm;
        case X86_INS_VPROLD:
            return e_vprold;
        case X86_INS_VPROLQ:
            return e_vprolq;
        case X86_INS_VPROLVD:
            return e_vprolvd;
        case X86_INS_VPROLVQ:
            return e_vprolvq;
        case X86_INS_VPRORD:
            return e_vprord;
        case X86_INS_VPRORQ:
            return e_vprorq;
        case X86_INS_VPRORVD:
            return e_vprorvd;
        case X86_INS_VPRORVQ:
            return e_vprorvq;
        case X86_INS_VPROTB:
            return e_vprotb;
        case X86_INS_VPROTD:
            return e_vprotd;
        case X86_INS_VPROTQ:
            return e_vprotq;
        case X86_INS_VPROTW:
            return e_vprotw;
        case X86_INS_VPSADBW:
            return e_vpsadbw;
        case X86_INS_VPSCATTERDD:
            return e_vpscatterdd;
        case X86_INS_VPSCATTERDQ:
            return e_vpscatterdq;
        case X86_INS_VPSCATTERQD:
            return e_vpscatterqd;
        case X86_INS_VPSCATTERQQ:
            return e_vpscatterqq;
        case X86_INS_VPSHAB:
            return e_vpshab;
        case X86_INS_VPSHAD:
            return e_vpshad;
        case X86_INS_VPSHAQ:
            return e_vpshaq;
        case X86_INS_VPSHAW:
            return e_vpshaw;
        case X86_INS_VPSHLB:
            return e_vpshlb;
        case X86_INS_VPSHLDD:
            return e_vpshldd;
        case X86_INS_VPSHLDQ:
            return e_vpshldq;
        case X86_INS_VPSHLDVD:
            return e_vpshldvd;
        case X86_INS_VPSHLDVQ:
            return e_vpshldvq;
        case X86_INS_VPSHLDVW:
            return e_vpshldvw;
        case X86_INS_VPSHLDW:
            return e_vpshldw;
        case X86_INS_VPSHLD:
            return e_vpshld;
        case X86_INS_VPSHLQ:
            return e_vpshlq;
        case X86_INS_VPSHLW:
            return e_vpshlw;
        case X86_INS_VPSHRDD:
            return e_vpshrdd;
        case X86_INS_VPSHRDQ:
            return e_vpshrdq;
        case X86_INS_VPSHRDVD:
            return e_vpshrdvd;
        case X86_INS_VPSHRDVQ:
            return e_vpshrdvq;
        case X86_INS_VPSHRDVW:
            return e_vpshrdvw;
        case X86_INS_VPSHRDW:
            return e_vpshrdw;
        case X86_INS_VPSHUFBITQMB:
            return e_vpshufbitqmb;
        case X86_INS_VPSHUFB:
            return e_vpshufb;
        case X86_INS_VPSHUFD:
            return e_vpshufd;
        case X86_INS_VPSHUFHW:
            return e_vpshufhw;
        case X86_INS_VPSHUFLW:
            return e_vpshuflw;
        case X86_INS_VPSIGNB:
            return e_vpsignb;
        case X86_INS_VPSIGND:
            return e_vpsignd;
        case X86_INS_VPSIGNW:
            return e_vpsignw;
        case X86_INS_VPSLLDQ:
            return e_vpslldq;
        case X86_INS_VPSLLD:
            return e_vpslld;
        case X86_INS_VPSLLQ:
            return e_vpsllq;
        case X86_INS_VPSLLVD:
            return e_vpsllvd;
        case X86_INS_VPSLLVQ:
            return e_vpsllvq;
        case X86_INS_VPSLLVW:
            return e_vpsllvw;
        case X86_INS_VPSLLW:
            return e_vpsllw;
        case X86_INS_VPSRAD:
            return e_vpsrad;
        case X86_INS_VPSRAQ:
            return e_vpsraq;
        case X86_INS_VPSRAVD:
            return e_vpsravd;
        case X86_INS_VPSRAVQ:
            return e_vpsravq;
        case X86_INS_VPSRAVW:
            return e_vpsravw;
        case X86_INS_VPSRAW:
            return e_vpsraw;
        case X86_INS_VPSRLDQ:
            return e_vpsrldq;
        case X86_INS_VPSRLD:
            return e_vpsrld;
        case X86_INS_VPSRLQ:
            return e_vpsrlq;
        case X86_INS_VPSRLVD:
            return e_vpsrlvd;
        case X86_INS_VPSRLVQ:
            return e_vpsrlvq;
        case X86_INS_VPSRLVW:
            return e_vpsrlvw;
        case X86_INS_VPSRLW:
            return e_vpsrlw;
        case X86_INS_VPSUBB:
            return e_vpsubb;
        case X86_INS_VPSUBD:
            return e_vpsubd;
        case X86_INS_VPSUBQ:
            return e_vpsubq;
        case X86_INS_VPSUBSB:
            return e_vpsubsb;
        case X86_INS_VPSUBSW:
            return e_vpsubsw;
        case X86_INS_VPSUBUSB:
            return e_vpsubusb;
        case X86_INS_VPSUBUSW:
            return e_vpsubusw;
        case X86_INS_VPSUBW:
            return e_vpsubw;
        case X86_INS_VPTERNLOGD:
            return e_vpternlogd;
        case X86_INS_VPTERNLOGQ:
            return e_vpternlogq;
        case X86_INS_VPTESTMB:
            return e_vptestmb;
        case X86_INS_VPTESTMD:
            return e_vptestmd;
        case X86_INS_VPTESTMQ:
            return e_vptestmq;
        case X86_INS_VPTESTMW:
            return e_vptestmw;
        case X86_INS_VPTESTNMB:
            return e_vptestnmb;
        case X86_INS_VPTESTNMD:
            return e_vptestnmd;
        case X86_INS_VPTESTNMQ:
            return e_vptestnmq;
        case X86_INS_VPTESTNMW:
            return e_vptestnmw;
        case X86_INS_VPTEST:
            return e_vptest;
        case X86_INS_VPUNPCKHBW:
            return e_vpunpckhbw;
        case X86_INS_VPUNPCKHDQ:
            return e_vpunpckhdq;
        case X86_INS_VPUNPCKHQDQ:
            return e_vpunpckhqdq;
        case X86_INS_VPUNPCKHWD:
            return e_vpunpckhwd;
        case X86_INS_VPUNPCKLBW:
            return e_vpunpcklbw;
        case X86_INS_VPUNPCKLDQ:
            return e_vpunpckldq;
        case X86_INS_VPUNPCKLQDQ:
            return e_vpunpcklqdq;
        case X86_INS_VPUNPCKLWD:
            return e_vpunpcklwd;
        case X86_INS_VPXORD:
            return e_vpxord;
        case X86_INS_VPXORQ:
            return e_vpxorq;
        case X86_INS_VPXOR:
            return e_vpxor;
        case X86_INS_VRANGEPD:
            return e_vrangepd;
        case X86_INS_VRANGEPS:
            return e_vrangeps;
        case X86_INS_VRANGESD:
            return e_vrangesd;
        case X86_INS_VRANGESS:
            return e_vrangess;
        case X86_INS_VRCP14PD:
            return e_vrcp14pd;
        case X86_INS_VRCP14PS:
            return e_vrcp14ps;
        case X86_INS_VRCP14SD:
            return e_vrcp14sd;
        case X86_INS_VRCP14SS:
            return e_vrcp14ss;
        case X86_INS_VRCP28PD:
            return e_vrcp28pd;
        case X86_INS_VRCP28PS:
            return e_vrcp28ps;
        case X86_INS_VRCP28SD:
            return e_vrcp28sd;
        case X86_INS_VRCP28SS:
            return e_vrcp28ss;
        case X86_INS_VRCPPS:
            return e_vrcpps;
        case X86_INS_VRCPSS:
            return e_vrcpss;
        case X86_INS_VREDUCEPD:
            return e_vreducepd;
        case X86_INS_VREDUCEPS:
            return e_vreduceps;
        case X86_INS_VREDUCESD:
            return e_vreducesd;
        case X86_INS_VREDUCESS:
            return e_vreducess;
        case X86_INS_VRNDSCALEPD:
            return e_vrndscalepd;
        case X86_INS_VRNDSCALEPS:
            return e_vrndscaleps;
        case X86_INS_VRNDSCALESD:
            return e_vrndscalesd;
        case X86_INS_VRNDSCALESS:
            return e_vrndscaless;
        case X86_INS_VROUNDPD:
            return e_vroundpd;
        case X86_INS_VROUNDPS:
            return e_vroundps;
        case X86_INS_VROUNDSD:
            return e_vroundsd;
        case X86_INS_VROUNDSS:
            return e_vroundss;
        case X86_INS_VRSQRT14PD:
            return e_vrsqrt14pd;
        case X86_INS_VRSQRT14PS:
            return e_vrsqrt14ps;
        case X86_INS_VRSQRT14SD:
            return e_vrsqrt14sd;
        case X86_INS_VRSQRT14SS:
            return e_vrsqrt14ss;
        case X86_INS_VRSQRT28PD:
            return e_vrsqrt28pd;
        case X86_INS_VRSQRT28PS:
            return e_vrsqrt28ps;
        case X86_INS_VRSQRT28SD:
            return e_vrsqrt28sd;
        case X86_INS_VRSQRT28SS:
            return e_vrsqrt28ss;
        case X86_INS_VRSQRTPS:
            return e_vrsqrtps;
        case X86_INS_VRSQRTSS:
            return e_vrsqrtss;
        case X86_INS_VSCALEFPD:
            return e_vscalefpd;
        case X86_INS_VSCALEFPS:
            return e_vscalefps;
        case X86_INS_VSCALEFSD:
            return e_vscalefsd;
        case X86_INS_VSCALEFSS:
            return e_vscalefss;
        case X86_INS_VSCATTERDPD:
            return e_vscatterdpd;
        case X86_INS_VSCATTERDPS:
            return e_vscatterdps;
        case X86_INS_VSCATTERPF0DPD:
            return e_vscatterpf0dpd;
        case X86_INS_VSCATTERPF0DPS:
            return e_vscatterpf0dps;
        case X86_INS_VSCATTERPF0QPD:
            return e_vscatterpf0qpd;
        case X86_INS_VSCATTERPF0QPS:
            return e_vscatterpf0qps;
        case X86_INS_VSCATTERPF1DPD:
            return e_vscatterpf1dpd;
        case X86_INS_VSCATTERPF1DPS:
            return e_vscatterpf1dps;
        case X86_INS_VSCATTERPF1QPD:
            return e_vscatterpf1qpd;
        case X86_INS_VSCATTERPF1QPS:
            return e_vscatterpf1qps;
        case X86_INS_VSCATTERQPD:
            return e_vscatterqpd;
        case X86_INS_VSCATTERQPS:
            return e_vscatterqps;
        case X86_INS_VSHUFF32X4:
            return e_vshuff32x4;
        case X86_INS_VSHUFF64X2:
            return e_vshuff64x2;
        case X86_INS_VSHUFI32X4:
            return e_vshufi32x4;
        case X86_INS_VSHUFI64X2:
            return e_vshufi64x2;
        case X86_INS_VSHUFPD:
            return e_vshufpd;
        case X86_INS_VSHUFPS:
            return e_vshufps;
        case X86_INS_VSQRTPD:
            return e_vsqrtpd;
        case X86_INS_VSQRTPS:
            return e_vsqrtps;
        case X86_INS_VSQRTSD:
            return e_vsqrtsd;
        case X86_INS_VSQRTSS:
            return e_vsqrtss;
        case X86_INS_VSTMXCSR:
            return e_vstmxcsr;
        case X86_INS_VSUBPD:
            return e_vsubpd;
        case X86_INS_VSUBPS:
            return e_vsubps;
        case X86_INS_VSUBSD:
            return e_vsubsd;
        case X86_INS_VSUBSS:
            return e_vsubss;
        case X86_INS_VTESTPD:
            return e_vtestpd;
        case X86_INS_VTESTPS:
            return e_vtestps;
        case X86_INS_VUCOMISD:
            return e_vucomisd;
        case X86_INS_VUCOMISS:
            return e_vucomiss;
        case X86_INS_VUNPCKHPD:
            return e_vunpckhpd;
        case X86_INS_VUNPCKHPS:
            return e_vunpckhps;
        case X86_INS_VUNPCKLPD:
            return e_vunpcklpd;
        case X86_INS_VUNPCKLPS:
            return e_vunpcklps;
        case X86_INS_VXORPD:
            return e_vxorpd;
        case X86_INS_VXORPS:
            return e_vxorps;
        case X86_INS_VZEROALL:
            return e_vzeroall;
        case X86_INS_VZEROUPPER:
            return e_vzeroupper;
        case X86_INS_WAIT:
            return e_wait;
        case X86_INS_WBINVD:
            return e_wbinvd;
        case X86_INS_WBNOINVD:
            return e_wbnoinvd;
        case X86_INS_WRFSBASE:
            return e_wrfsbase;
        case X86_INS_WRGSBASE:
            return e_wrgsbase;
        case X86_INS_WRMSR:
            return e_wrmsr;
        case X86_INS_WRPKRU:
            return e_wrpkru;
        case X86_INS_WRSSD:
            return e_wrssd;
        case X86_INS_WRSSQ:
            return e_wrssq;
        case X86_INS_WRUSSD:
            return e_wrussd;
        case X86_INS_WRUSSQ:
            return e_wrussq;
        case X86_INS_XABORT:
            return e_xabort;
        case X86_INS_XACQUIRE:
            return e_xacquire;
        case X86_INS_XADD:
            return e_xadd;
        case X86_INS_XBEGIN:
            return e_xbegin;
        case X86_INS_XCHG:
            return e_xchg;
        case X86_INS_FXCH:
            return e_fxch;
        case X86_INS_XCRYPTCBC:
            return e_xcryptcbc;
        case X86_INS_XCRYPTCFB:
            return e_xcryptcfb;
        case X86_INS_XCRYPTCTR:
            return e_xcryptctr;
        case X86_INS_XCRYPTECB:
            return e_xcryptecb;
        case X86_INS_XCRYPTOFB:
            return e_xcryptofb;
        case X86_INS_XEND:
            return e_xend;
        case X86_INS_XGETBV:
            return e_xgetbv;
        case X86_INS_XLATB:
            return e_xlatb;
        case X86_INS_XOR:
            return e_xor;
        case X86_INS_XORPD:
            return e_xorpd;
        case X86_INS_XORPS:
            return e_xorps;
        case X86_INS_XRELEASE:
            return e_xrelease;
        case X86_INS_XRSTOR:
            return e_xrstor;
        case X86_INS_XRSTOR64:
            return e_xrstor64;
        case X86_INS_XRSTORS:
            return e_xrstors;
        case X86_INS_XRSTORS64:
            return e_xrstors64;
        case X86_INS_XSAVE:
            return e_xsave;
        case X86_INS_XSAVE64:
            return e_xsave64;
        case X86_INS_XSAVEC:
            return e_xsavec;
        case X86_INS_XSAVEC64:
            return e_xsavec64;
        case X86_INS_XSAVEOPT:
            return e_xsaveopt;
        case X86_INS_XSAVEOPT64:
            return e_xsaveopt64;
        case X86_INS_XSAVES:
            return e_xsaves;
        case X86_INS_XSAVES64:
            return e_xsaves64;
        case X86_INS_XSETBV:
            return e_xsetbv;
        case X86_INS_XSHA1:
            return e_xsha1;
        case X86_INS_XSHA256:
            return e_xsha256;
        case X86_INS_XSTORE:
            return e_xstore;
        case X86_INS_XTEST:
            return e_xtest;
        default:
            return e_No_Entry;
    }

}

MachRegister InstructionDecoder_Capstone::registerTranslation_x86(x86_reg cap_reg) {
    switch (cap_reg) {
        case X86_REG_AH:
            return x86_64::ah;
        case X86_REG_AL:
            return x86_64::al;
        case X86_REG_AX:
            return x86_64::ax;
        case X86_REG_BH:
            return x86_64::bh;
        case X86_REG_BL:
            return x86_64::bl;
        case X86_REG_BP:
            return x86_64::bp;
        case X86_REG_BPL:
            return x86_64::bpl;
        case X86_REG_BX:
            return x86_64::bx;
        case X86_REG_CH:
            return x86_64::ch;
        case X86_REG_CL:
            return x86_64::cl;
        case X86_REG_CS:
            return x86_64::cs;
        case X86_REG_CX:
            return x86_64::cx;
        case X86_REG_DH:
            return x86_64::dh;
        case X86_REG_DI:
            return x86_64::di;
        case X86_REG_DIL:
            return x86_64::dil;
        case X86_REG_DL:
            return x86_64::dl;
        case X86_REG_DS:
            return x86_64::ds;
        case X86_REG_DX:
            return x86_64::dx;
        case X86_REG_EAX:
            return x86_64::eax;
        case X86_REG_EBP:
            return x86_64::ebp;
        case X86_REG_EBX:
            return x86_64::ebx;
        case X86_REG_ECX:
            return x86_64::ecx;
        case X86_REG_EDI:
            return x86_64::edi;
        case X86_REG_EDX:
            return x86_64::edx;
        case X86_REG_EFLAGS:
            return x86_64::flags;
        case X86_REG_EIP:
            return x86_64::eip;
//        case X86_REG_EIZ:
//            return x86_64::eiz;
        case X86_REG_ES:
            return x86_64::es;
        case X86_REG_ESI:
            return x86_64::esi;
        case X86_REG_ESP:
            return x86_64::esp;
//        case X86_REG_FPSW:
//            return x86_64::fpsw;
        case X86_REG_FS:
            return x86_64::fs;
        case X86_REG_GS:
            return x86_64::gs;
        case X86_REG_IP:
            return x86_64::rip;
        case X86_REG_RAX:
            return x86_64::rax;
        case X86_REG_RBP:
            return x86_64::rbp;
        case X86_REG_RBX:
            return x86_64::rbx;
        case X86_REG_RCX:
            return x86_64::rcx;
        case X86_REG_RDI:
            return x86_64::rdi;
        case X86_REG_RDX:
            return x86_64::rdx;
        case X86_REG_RIP:
            return x86_64::rip;
//        case X86_REG_RIZ:
//            return x86_64::riz;
        case X86_REG_RSI:
            return x86_64::rsi;
        case X86_REG_RSP:
            return x86_64::rsp;
        case X86_REG_SI:
            return x86_64::si;
        case X86_REG_SIL:
            return x86_64::sil;
        case X86_REG_SP:
            return x86_64::sp;
        case X86_REG_SPL:
            return x86_64::spl;
        case X86_REG_SS:
            return x86_64::ss;
        case X86_REG_CR0:
            return x86_64::cr0;
        case X86_REG_CR1:
            return x86_64::cr1;
        case X86_REG_CR2:
            return x86_64::cr2;
        case X86_REG_CR3:
            return x86_64::cr3;
        case X86_REG_CR4:
            return x86_64::cr4;
        case X86_REG_CR5:
            return x86_64::cr5;
        case X86_REG_CR6:
            return x86_64::cr6;
        case X86_REG_CR7:
            return x86_64::cr7;
      /*
        case X86_REG_CR8:
            return x86_64::cr8;
        case X86_REG_CR9:
            return x86_64::cr9;
        case X86_REG_CR10:
            return x86_64::cr10;
        case X86_REG_CR11:
            return x86_64::cr11;
        case X86_REG_CR12:
            return x86_64::cr12;
        case X86_REG_CR13:
            return x86_64::cr13;
        case X86_REG_CR14:
            return x86_64::cr14;
        case X86_REG_CR15:
            return x86_64::cr15;
       */
        case X86_REG_DR0:
            return x86_64::dr0;
        case X86_REG_DR1:
            return x86_64::dr1;
        case X86_REG_DR2:
            return x86_64::dr2;
        case X86_REG_DR3:
            return x86_64::dr3;
        case X86_REG_DR4:
            return x86_64::dr4;
        case X86_REG_DR5:
            return x86_64::dr5;
        case X86_REG_DR6:
            return x86_64::dr6;
        case X86_REG_DR7:
            return x86_64::dr7;
       /*
        case X86_REG_DR8:
            return x86_64::dr8;
        case X86_REG_DR9:
            return x86_64::dr9;
        case X86_REG_DR10:
            return x86_64::dr10;
        case X86_REG_DR11:
            return x86_64::dr11;
        case X86_REG_DR12:
            return x86_64::dr12;
        case X86_REG_DR13:
            return x86_64::dr13;
        case X86_REG_DR14:
            return x86_64::dr14;
        case X86_REG_DR15:
            return x86_64::dr15;
        case X86_REG_FP0:
            return x86_64::fp0;
        case X86_REG_FP1:
            return x86_64::fp1;
        case X86_REG_FP2:
            return x86_64::fp2;
        case X86_REG_FP3:
            return x86_64::fp3;
        case X86_REG_FP4:
            return x86_64::fp4;
        case X86_REG_FP5:
            return x86_64::fp5;
        case X86_REG_FP6:
            return x86_64::fp6;
        case X86_REG_FP7:
            return x86_64::fp7;
       */
        case X86_REG_K0:
            return x86_64::k0;
        case X86_REG_K1:
            return x86_64::k1;
        case X86_REG_K2:
            return x86_64::k2;
        case X86_REG_K3:
            return x86_64::k3;
        case X86_REG_K4:
            return x86_64::k4;
        case X86_REG_K5:
            return x86_64::k5;
        case X86_REG_K6:
            return x86_64::k6;
        case X86_REG_K7:
            return x86_64::k7;
        case X86_REG_MM0:
            return x86_64::mm0;
        case X86_REG_MM1:
            return x86_64::mm1;
        case X86_REG_MM2:
            return x86_64::mm2;
        case X86_REG_MM3:
            return x86_64::mm3;
        case X86_REG_MM4:
            return x86_64::mm4;
        case X86_REG_MM5:
            return x86_64::mm5;
        case X86_REG_MM6:
            return x86_64::mm6;
        case X86_REG_MM7:
            return x86_64::mm7;
        case X86_REG_R8:
            return x86_64::r8;
        case X86_REG_R9:
            return x86_64::r9;
        case X86_REG_R10:
            return x86_64::r10;
        case X86_REG_R11:
            return x86_64::r11;
        case X86_REG_R12:
            return x86_64::r12;
        case X86_REG_R13:
            return x86_64::r13;
        case X86_REG_R14:
            return x86_64::r14;
        case X86_REG_R15:
            return x86_64::r15;
        case X86_REG_ST0:
            return x86_64::st0;
        case X86_REG_ST1:
            return x86_64::st1;
        case X86_REG_ST2:
            return x86_64::st2;
        case X86_REG_ST3:
            return x86_64::st3;
        case X86_REG_ST4:
            return x86_64::st4;
        case X86_REG_ST5:
            return x86_64::st5;
        case X86_REG_ST6:
            return x86_64::st6;
        case X86_REG_ST7:
            return x86_64::st7;
        case X86_REG_XMM0:
            return x86_64::xmm0;
        case X86_REG_XMM1:
            return x86_64::xmm1;
        case X86_REG_XMM2:
            return x86_64::xmm2;
        case X86_REG_XMM3:
            return x86_64::xmm3;
        case X86_REG_XMM4:
            return x86_64::xmm4;
        case X86_REG_XMM5:
            return x86_64::xmm5;
        case X86_REG_XMM6:
            return x86_64::xmm6;
        case X86_REG_XMM7:
            return x86_64::xmm7;
        case X86_REG_XMM8:
            return x86_64::xmm8;
        case X86_REG_XMM9:
            return x86_64::xmm9;
        case X86_REG_XMM10:
            return x86_64::xmm10;
        case X86_REG_XMM11:
            return x86_64::xmm11;
        case X86_REG_XMM12:
            return x86_64::xmm12;
        case X86_REG_XMM13:
            return x86_64::xmm13;
        case X86_REG_XMM14:
            return x86_64::xmm14;
        case X86_REG_XMM15:
            return x86_64::xmm15;
        case X86_REG_XMM16:
            return x86_64::xmm16;
        case X86_REG_XMM17:
            return x86_64::xmm17;
        case X86_REG_XMM18:
            return x86_64::xmm18;
        case X86_REG_XMM19:
            return x86_64::xmm19;
        case X86_REG_XMM20:
            return x86_64::xmm20;
        case X86_REG_XMM21:
            return x86_64::xmm21;
        case X86_REG_XMM22:
            return x86_64::xmm22;
        case X86_REG_XMM23:
            return x86_64::xmm23;
        case X86_REG_XMM24:
            return x86_64::xmm24;
        case X86_REG_XMM25:
            return x86_64::xmm25;
        case X86_REG_XMM26:
            return x86_64::xmm26;
        case X86_REG_XMM27:
            return x86_64::xmm27;
        case X86_REG_XMM28:
            return x86_64::xmm28;
        case X86_REG_XMM29:
            return x86_64::xmm29;
        case X86_REG_XMM30:
            return x86_64::xmm30;
        case X86_REG_XMM31:
            return x86_64::xmm31;
        case X86_REG_YMM0:
            return x86_64::ymm0;
        case X86_REG_YMM1:
            return x86_64::ymm1;
        case X86_REG_YMM2:
            return x86_64::ymm2;
        case X86_REG_YMM3:
            return x86_64::ymm3;
        case X86_REG_YMM4:
            return x86_64::ymm4;
        case X86_REG_YMM5:
            return x86_64::ymm5;
        case X86_REG_YMM6:
            return x86_64::ymm6;
        case X86_REG_YMM7:
            return x86_64::ymm7;
        case X86_REG_YMM8:
            return x86_64::ymm8;
        case X86_REG_YMM9:
            return x86_64::ymm9;
        case X86_REG_YMM10:
            return x86_64::ymm10;
        case X86_REG_YMM11:
            return x86_64::ymm11;
        case X86_REG_YMM12:
            return x86_64::ymm12;
        case X86_REG_YMM13:
            return x86_64::ymm13;
        case X86_REG_YMM14:
            return x86_64::ymm14;
        case X86_REG_YMM15:
            return x86_64::ymm15;
        case X86_REG_YMM16:
            return x86_64::ymm16;
        case X86_REG_YMM17:
            return x86_64::ymm17;
        case X86_REG_YMM18:
            return x86_64::ymm18;
        case X86_REG_YMM19:
            return x86_64::ymm19;
        case X86_REG_YMM20:
            return x86_64::ymm20;
        case X86_REG_YMM21:
            return x86_64::ymm21;
        case X86_REG_YMM22:
            return x86_64::ymm22;
        case X86_REG_YMM23:
            return x86_64::ymm23;
        case X86_REG_YMM24:
            return x86_64::ymm24;
        case X86_REG_YMM25:
            return x86_64::ymm25;
        case X86_REG_YMM26:
            return x86_64::ymm26;
        case X86_REG_YMM27:
            return x86_64::ymm27;
        case X86_REG_YMM28:
            return x86_64::ymm28;
        case X86_REG_YMM29:
            return x86_64::ymm29;
        case X86_REG_YMM30:
            return x86_64::ymm30;
        case X86_REG_YMM31:
            return x86_64::ymm31;
        case X86_REG_ZMM0:
            return x86_64::zmm0;
        case X86_REG_ZMM1:
            return x86_64::zmm1;
        case X86_REG_ZMM2:
            return x86_64::zmm2;
        case X86_REG_ZMM3:
            return x86_64::zmm3;
        case X86_REG_ZMM4:
            return x86_64::zmm4;
        case X86_REG_ZMM5:
            return x86_64::zmm5;
        case X86_REG_ZMM6:
            return x86_64::zmm6;
        case X86_REG_ZMM7:
            return x86_64::zmm7;
        case X86_REG_ZMM8:
            return x86_64::zmm8;
        case X86_REG_ZMM9:
            return x86_64::zmm9;
        case X86_REG_ZMM10:
            return x86_64::zmm10;
        case X86_REG_ZMM11:
            return x86_64::zmm11;
        case X86_REG_ZMM12:
            return x86_64::zmm12;
        case X86_REG_ZMM13:
            return x86_64::zmm13;
        case X86_REG_ZMM14:
            return x86_64::zmm14;
        case X86_REG_ZMM15:
            return x86_64::zmm15;
        case X86_REG_ZMM16:
            return x86_64::zmm16;
        case X86_REG_ZMM17:
            return x86_64::zmm17;
        case X86_REG_ZMM18:
            return x86_64::zmm18;
        case X86_REG_ZMM19:
            return x86_64::zmm19;
        case X86_REG_ZMM20:
            return x86_64::zmm20;
        case X86_REG_ZMM21:
            return x86_64::zmm21;
        case X86_REG_ZMM22:
            return x86_64::zmm22;
        case X86_REG_ZMM23:
            return x86_64::zmm23;
        case X86_REG_ZMM24:
            return x86_64::zmm24;
        case X86_REG_ZMM25:
            return x86_64::zmm25;
        case X86_REG_ZMM26:
            return x86_64::zmm26;
        case X86_REG_ZMM27:
            return x86_64::zmm27;
        case X86_REG_ZMM28:
            return x86_64::zmm28;
        case X86_REG_ZMM29:
            return x86_64::zmm29;
        case X86_REG_ZMM30:
            return x86_64::zmm30;
        case X86_REG_ZMM31:
            return x86_64::zmm31;
        case X86_REG_R8B:
            return x86_64::r8b;
        case X86_REG_R9B:
            return x86_64::r9b;
        case X86_REG_R10B:
            return x86_64::r10b;
        case X86_REG_R11B:
            return x86_64::r11b;
        case X86_REG_R12B:
            return x86_64::r12b;
        case X86_REG_R13B:
            return x86_64::r13b;
        case X86_REG_R14B:
            return x86_64::r14b;
        case X86_REG_R15B:
            return x86_64::r15b;
        case X86_REG_R8D:
            return x86_64::r8d;
        case X86_REG_R9D:
            return x86_64::r9d;
        case X86_REG_R10D:
            return x86_64::r10d;
        case X86_REG_R11D:
            return x86_64::r11d;
        case X86_REG_R12D:
            return x86_64::r12d;
        case X86_REG_R13D:
            return x86_64::r13d;
        case X86_REG_R14D:
            return x86_64::r14d;
        case X86_REG_R15D:
            return x86_64::r15d;
        case X86_REG_R8W:
            return x86_64::r8w;
        case X86_REG_R9W:
            return x86_64::r9w;
        case X86_REG_R10W:
            return x86_64::r10w;
        case X86_REG_R11W:
            return x86_64::r11w;
        case X86_REG_R12W:
            return x86_64::r12w;
        case X86_REG_R13W:
            return x86_64::r13w;
        case X86_REG_R14W:
            return x86_64::r14w;
        case X86_REG_R15W:
            return x86_64::r15w;
            /*
        case X86_REG_BND0:
            return x86_64::bnd0;
        case X86_REG_BND1:
            return x86_64::bnd1;
        case X86_REG_BND2:
            return x86_64::bnd2;
        case X86_REG_BND3:
            return x86_64::bnd3;
            */
        default:
            return InvalidReg;
    }
}



/******************************  ppc functions  *************************************/

void InstructionDecoder_Capstone::decodeOperands_ppc(const Instruction* insn, cs_detail *d) {

}

entryID InstructionDecoder_Capstone::opcodeTranslation_ppc(unsigned int cap_id) {
    return e_No_Entry; 
}


/******************************  aarch64 functions  *************************************/

void InstructionDecoder_Capstone::decodeOperands_aarch64(const Instruction* insn, cs_detail *d) {

}

entryID InstructionDecoder_Capstone::opcodeTranslation_aarch64(unsigned int cap_id) {
    return e_No_Entry; 
}

}; // namespace InstructionAPI

}; // namespace Dyninst
