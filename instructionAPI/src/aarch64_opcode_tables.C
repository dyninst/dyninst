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
#define INVALID_ENTRY aarch64_insn_entry::main_insn_table[0]

#define fn(...) (&InstructionDecoder_aarch64::__VA_ARGS__)

const aarch64_insn_table aarch64_insn_entry::main_insn_table = {
    {aarch64_op_INVALID,"INVALID",{NULL},0,402653184},
    {aarch64_op_abs_advsimd,"abs",{fn(setSIMDMode),fn(OPRsize<23,22>),fn(OPRRn),fn(OPRRd),NULL},1579202560,4282383360},
    {aarch64_op_abs_advsimd,"abs",{fn(setSIMDMode),fn(OPRQ),fn(OPRsize<23,22>),fn(OPRRn),fn(OPRRd),NULL},237025280,3208641536},
    {aarch64_op_adc,"adc",{fn(OPRsf),fn(OPRRm),fn(OPRRn),fn(OPRRd),NULL},436207616,2145451008},
    {aarch64_op_adcs,"adcs",{fn(setFlags),fn(OPRsf),fn(OPRRm),fn(OPRRn),fn(OPRRd),NULL},973078528,2145451008},
    {aarch64_op_add_addsub_ext,"add",{fn(OPRsf),fn(OPRRm),fn(OPRoption<15,13>),fn(OPRimm<12,10>),fn(OPRRn),fn(OPRRd),NULL},186646528,2145386496},
    {aarch64_op_add_addsub_imm,"add",{fn(OPRsf),fn(OPRshift),fn(OPRimm<21,10>),fn(OPRRn),fn(OPRRd),NULL},285212672,2130706432},
    {aarch64_op_add_addsub_shift,"add",{fn(OPRsf),fn(OPRshift),fn(OPRRm),fn(OPRimm<15,10>),fn(OPRRn),fn(OPRRd),NULL},184549376,2132803584},
    {aarch64_op_add_advsimd,"add",{fn(setSIMDMode),fn(OPRsize<23,22>),fn(OPRRm),fn(OPRRn),fn(OPRRd),NULL},1579189248,4280351744},
    {aarch64_op_add_advsimd,"add",{fn(setSIMDMode),fn(OPRQ),fn(OPRsize<23,22>),fn(OPRRm),fn(OPRRn),fn(OPRRd),NULL},237011968,3206609920},
    {aarch64_op_addhn_advsimd,"addhn",{fn(setSIMDMode),fn(OPRQ),fn(OPRsize<23,22>),fn(OPRRm),fn(OPRRn),fn(OPRRd),NULL},236994560,3206609920},
    {aarch64_op_addp_advsimd_pair,"addp",{fn(setSIMDMode),fn(OPRsize<23,22>),fn(OPRRn),fn(OPRRd),NULL},1580316672,4282383360},
    {aarch64_op_addp_advsimd_vec,"addp",{fn(setSIMDMode),fn(OPRQ),fn(OPRsize<23,22>),fn(OPRRm),fn(OPRRn),fn(OPRRd),NULL},237026304,3206609920},
    {aarch64_op_adds_addsub_ext,"adds",{fn(setFlags),fn(OPRsf),fn(OPRRm),fn(OPRoption<15,13>),fn(OPRimm<12,10>),fn(OPRRn),fn(OPRRd),NULL},723517440,2145386496},
    {aarch64_op_adds_addsub_imm,"adds",{fn(setFlags),fn(OPRsf),fn(OPRshift),fn(OPRimm<21,10>),fn(OPRRn),fn(OPRRd),NULL},822083584,2130706432},
    {aarch64_op_adds_addsub_shift,"adds",{fn(setFlags),fn(OPRsf),fn(OPRshift),fn(OPRRm),fn(OPRimm<15,10>),fn(OPRRn),fn(OPRRd),NULL},721420288,2132803584},
    {aarch64_op_addv_advsimd,"addv",{fn(setSIMDMode),fn(OPRQ),fn(OPRsize<23,22>),fn(OPRRn),fn(OPRRd),NULL},238139392,3208641536},
    {aarch64_op_adr,"adr",{fn(OPRimm<30,29>),fn(OPRimm<23,5>),fn(OPRRd),NULL},268435456,2667577344},
    {aarch64_op_adrp,"adrp",{fn(OPRimm<30,29>),fn(OPRimm<23,5>),fn(OPRRd),NULL},2415919104,2667577344},
    {aarch64_op_aesd_advsimd,"aesd",{fn(setSIMDMode),fn(OPRRn),fn(OPRRd),NULL},1311266816,4294966272},
    {aarch64_op_aese_advsimd,"aese",{fn(setSIMDMode),fn(OPRRn),fn(OPRRd),NULL},1311262720,4294966272},
    {aarch64_op_aesimc_advsimd,"aesimc",{fn(setSIMDMode),fn(OPRRn),fn(OPRRd),NULL},1311275008,4294966272},
    {aarch64_op_aesmc_advsimd,"aesmc",{fn(setSIMDMode),fn(OPRRn),fn(OPRRd),NULL},1311270912,4294966272},
    {aarch64_op_and_advsimd,"and",{fn(setSIMDMode),fn(OPRQ),fn(OPRRm),fn(OPRRn),fn(OPRRd),NULL},236985344,3219192832},
    {aarch64_op_and_log_imm,"and",{fn(OPRsf),fn(OPRN<22,22>),fn(OPRimm<21,16>),fn(OPRimm<15,10>),fn(OPRRn),fn(OPRRd),NULL},301989888,2139095040},
    {aarch64_op_and_log_shift,"and",{fn(OPRsf),fn(OPRshift),fn(OPRRm),fn(OPRimm<15,10>),fn(OPRRn),fn(OPRRd),NULL},167772160,2132803584},
    {aarch64_op_ands_log_imm,"ands",{fn(setFlags),fn(OPRsf),fn(OPRN<22,22>),fn(OPRimm<21,16>),fn(OPRimm<15,10>),fn(OPRRn),fn(OPRRd),NULL},1912602624,2139095040},
    {aarch64_op_ands_log_shift,"ands",{fn(setFlags),fn(OPRsf),fn(OPRshift),fn(OPRRm),fn(OPRimm<15,10>),fn(OPRRn),fn(OPRRd),NULL},1778384896,2132803584},
    {aarch64_op_asr_asrv,"asr",{fn(OPRsf),fn(OPRRm),fn(OPRRn),fn(OPRRd),NULL},448800768,2145451008},
    {aarch64_op_asr_sbfm,"asr",{fn(OPRsf),fn(OPRN<22,22>),fn(OPRimm<21,16>),fn(OPRimm<15,10>),fn(OPRRn),fn(OPRRd),NULL},318798848,2139126784},
    {aarch64_op_asrv,"asrv",{fn(OPRsf),fn(OPRRm),fn(OPRRn),fn(OPRRd),NULL},448800768,2145451008},
    {aarch64_op_at_sys,"at",{fn(OPRop1),fn(OPRop2),fn(OPRRt),NULL},3574102016,4294508288},
    {aarch64_op_b_cond,"b",{fn(OPRimm<23,5>),fn(OPRcond<3,0>),NULL},1409286144,4278190096},
    {aarch64_op_b_uncond,"b",{fn(OPRimm<25,0>),NULL},335544320,4227858432},
    {aarch64_op_bfi_bfm,"bfi",{fn(OPRsf),fn(OPRN<22,22>),fn(OPRimm<21,16>),fn(OPRimm<15,10>),fn(OPRRn),fn(OPRRd),NULL},855638016,2139095040},
    {aarch64_op_bfm,"bfm",{fn(OPRsf),fn(OPRN<22,22>),fn(OPRimm<21,16>),fn(OPRimm<15,10>),fn(OPRRn),fn(OPRRd),NULL},855638016,2139095040},
    {aarch64_op_bfxil_bfm,"bfxil",{fn(OPRsf),fn(OPRN<22,22>),fn(OPRimm<21,16>),fn(OPRimm<15,10>),fn(OPRRn),fn(OPRRd),NULL},855638016,2139095040},
    {aarch64_op_bic_advsimd_imm,"bic",{fn(setSIMDMode),fn(OPRQ),fn(OPRa),fn(OPRb),fn(OPRc),fn(OPRcmode),fn(OPRd),fn(OPRe),fn(OPRf),fn(OPRg),fn(OPRh),fn(OPRRd),NULL},788534272,3220708352},
    {aarch64_op_bic_advsimd_reg,"bic",{fn(setSIMDMode),fn(OPRQ),fn(OPRRm),fn(OPRRn),fn(OPRRd),NULL},241179648,3219192832},
    {aarch64_op_bic_log_shift,"bic",{fn(OPRsf),fn(OPRshift),fn(OPRRm),fn(OPRimm<15,10>),fn(OPRRn),fn(OPRRd),NULL},169869312,2132803584},
    {aarch64_op_bics,"bics",{fn(setFlags),fn(OPRsf),fn(OPRshift),fn(OPRRm),fn(OPRimm<15,10>),fn(OPRRn),fn(OPRRd),NULL},1780482048,2132803584},
    {aarch64_op_bif_advsimd,"bif",{fn(setSIMDMode),fn(OPRQ),fn(OPRRm),fn(OPRRn),fn(OPRRd),NULL},786439168,3219192832},
    {aarch64_op_bit_advsimd,"bit",{fn(setSIMDMode),fn(OPRQ),fn(OPRRm),fn(OPRRn),fn(OPRRd),NULL},782244864,3219192832},
    {aarch64_op_bl,"bl",{fn(OPRimm<25,0>),NULL},2483027968,4227858432},
    {aarch64_op_blr,"blr",{fn(OPRRn),NULL},3594452992,4294966303},
    {aarch64_op_br,"br",{fn(OPRRn),NULL},3592355840,4294966303},
    {aarch64_op_brk,"brk",{fn(OPRimm<20,5>),NULL},3558866944,4292870175},
    {aarch64_op_bsl_advsimd,"bsl",{fn(setSIMDMode),fn(OPRQ),fn(OPRRm),fn(OPRRn),fn(OPRRd),NULL},778050560,3219192832},
    {aarch64_op_cbnz,"cbnz",{fn(OPRsf),fn(OPRimm<23,5>),fn(OPRRt),NULL},889192448,2130706432},
    {aarch64_op_cbz,"cbz",{fn(OPRsf),fn(OPRimm<23,5>),fn(OPRRt),NULL},872415232,2130706432},
    {aarch64_op_ccmn_imm,"ccmn",{fn(OPRsf),fn(OPRimm<20,16>),fn(OPRcond<15,12>),fn(OPRRn),fn(OPRnzcv),NULL},977274880,2145389584},
    {aarch64_op_ccmn_reg,"ccmn",{fn(OPRsf),fn(OPRRm),fn(OPRcond<15,12>),fn(OPRRn),fn(OPRnzcv),NULL},977272832,2145389584},
    {aarch64_op_ccmp_imm,"ccmp",{fn(OPRsf),fn(OPRimm<20,16>),fn(OPRcond<15,12>),fn(OPRRn),fn(OPRnzcv),NULL},2051016704,2145389584},
    {aarch64_op_ccmp_reg,"ccmp",{fn(OPRsf),fn(OPRRm),fn(OPRcond<15,12>),fn(OPRRn),fn(OPRnzcv),NULL},2051014656,2145389584},
    {aarch64_op_cinc_csinc,"cinc",{fn(OPRsf),fn(OPRcond<15,12>),fn(OPRRn),fn(OPRRd),NULL},444597248,2145389568},
    {aarch64_op_cinv_csinv,"cinv",{fn(OPRsf),fn(OPRcond<15,12>),fn(OPRRn),fn(OPRRd),NULL},1518338048,2145389568},
    {aarch64_op_clrex,"clrex",{fn(OPRCRm),NULL},3573755999,4294963455},
    {aarch64_op_cls_advsimd,"cls",{fn(setSIMDMode),fn(OPRQ),fn(OPRsize<23,22>),fn(OPRRn),fn(OPRRd),NULL},236996608,3208641536},
    {aarch64_op_cls_int,"cls",{fn(OPRsf),fn(OPRRn),fn(OPRRd),NULL},1522537472,2147482624},
    {aarch64_op_clz_advsimd,"clz",{fn(setSIMDMode),fn(OPRQ),fn(OPRsize<23,22>),fn(OPRRn),fn(OPRRd),NULL},773867520,3208641536},
    {aarch64_op_clz_int,"clz",{fn(OPRsf),fn(OPRRn),fn(OPRRd),NULL},1522536448,2147482624},
    {aarch64_op_cmeq_advsimd_reg,"cmeq",{fn(setSIMDMode),fn(OPRsize<23,22>),fn(OPRRm),fn(OPRRn),fn(OPRRd),NULL},2116062208,4280351744},
    {aarch64_op_cmeq_advsimd_reg,"cmeq",{fn(setSIMDMode),fn(OPRQ),fn(OPRsize<23,22>),fn(OPRRm),fn(OPRRn),fn(OPRRd),NULL},773884928,3206609920},
    {aarch64_op_cmeq_advsimd_zero,"cmeq",{fn(setSIMDMode),fn(OPRsize<23,22>),fn(OPRRn),fn(OPRRd),NULL},1579194368,4282383360},
    {aarch64_op_cmeq_advsimd_zero,"cmeq",{fn(setSIMDMode),fn(OPRQ),fn(OPRsize<23,22>),fn(OPRRn),fn(OPRRd),NULL},237017088,3208641536},
    {aarch64_op_cmge_advsimd_reg,"cmge",{fn(setSIMDMode),fn(OPRsize<23,22>),fn(OPRRm),fn(OPRRn),fn(OPRRd),NULL},1579170816,4280351744},
    {aarch64_op_cmge_advsimd_reg,"cmge",{fn(setSIMDMode),fn(OPRQ),fn(OPRsize<23,22>),fn(OPRRm),fn(OPRRn),fn(OPRRd),NULL},236993536,3206609920},
    {aarch64_op_cmge_advsimd_zero,"cmge",{fn(setSIMDMode),fn(OPRsize<23,22>),fn(OPRRn),fn(OPRRd),NULL},2116061184,4282383360},
    {aarch64_op_cmge_advsimd_zero,"cmge",{fn(setSIMDMode),fn(OPRQ),fn(OPRsize<23,22>),fn(OPRRn),fn(OPRRd),NULL},773883904,3208641536},
    {aarch64_op_cmgt_advsimd_reg,"cmgt",{fn(setSIMDMode),fn(OPRsize<23,22>),fn(OPRRm),fn(OPRRn),fn(OPRRd),NULL},1579168768,4280351744},
    {aarch64_op_cmgt_advsimd_reg,"cmgt",{fn(setSIMDMode),fn(OPRQ),fn(OPRsize<23,22>),fn(OPRRm),fn(OPRRn),fn(OPRRd),NULL},236991488,3206609920},
    {aarch64_op_cmgt_advsimd_zero,"cmgt",{fn(setSIMDMode),fn(OPRsize<23,22>),fn(OPRRn),fn(OPRRd),NULL},1579190272,4282383360},
    {aarch64_op_cmgt_advsimd_zero,"cmgt",{fn(setSIMDMode),fn(OPRQ),fn(OPRsize<23,22>),fn(OPRRn),fn(OPRRd),NULL},237012992,3208641536},
    {aarch64_op_cmhi_advsimd,"cmhi",{fn(setSIMDMode),fn(OPRsize<23,22>),fn(OPRRm),fn(OPRRn),fn(OPRRd),NULL},2116039680,4280351744},
    {aarch64_op_cmhi_advsimd,"cmhi",{fn(setSIMDMode),fn(OPRQ),fn(OPRsize<23,22>),fn(OPRRm),fn(OPRRn),fn(OPRRd),NULL},773862400,3206609920},
    {aarch64_op_cmhs_advsimd,"cmhs",{fn(setSIMDMode),fn(OPRsize<23,22>),fn(OPRRm),fn(OPRRn),fn(OPRRd),NULL},2116041728,4280351744},
    {aarch64_op_cmhs_advsimd,"cmhs",{fn(setSIMDMode),fn(OPRQ),fn(OPRsize<23,22>),fn(OPRRm),fn(OPRRn),fn(OPRRd),NULL},773864448,3206609920},
    {aarch64_op_cmle_advsimd,"cmle",{fn(setSIMDMode),fn(OPRsize<23,22>),fn(OPRRn),fn(OPRRd),NULL},2116065280,4282383360},
    {aarch64_op_cmle_advsimd,"cmle",{fn(setSIMDMode),fn(OPRQ),fn(OPRsize<23,22>),fn(OPRRn),fn(OPRRd),NULL},773888000,3208641536},
    {aarch64_op_cmlt_advsimd,"cmlt",{fn(setSIMDMode),fn(OPRsize<23,22>),fn(OPRRn),fn(OPRRd),NULL},1579198464,4282383360},
    {aarch64_op_cmlt_advsimd,"cmlt",{fn(setSIMDMode),fn(OPRQ),fn(OPRsize<23,22>),fn(OPRRn),fn(OPRRd),NULL},237021184,3208641536},
    {aarch64_op_cmn_adds_addsub_ext,"cmn",{fn(setFlags),fn(OPRsf),fn(OPRRm),fn(OPRoption<15,13>),fn(OPRimm<12,10>),fn(OPRRn),NULL},723517471,2145386527},
    {aarch64_op_cmn_adds_addsub_imm,"cmn",{fn(setFlags),fn(OPRsf),fn(OPRshift),fn(OPRimm<21,10>),fn(OPRRn),NULL},822083615,2130706463},
    {aarch64_op_cmn_adds_addsub_shift,"cmn",{fn(setFlags),fn(OPRsf),fn(OPRshift),fn(OPRRm),fn(OPRimm<15,10>),fn(OPRRn),NULL},721420319,2132803615},
    {aarch64_op_cmp_subs_addsub_ext,"cmp",{fn(setFlags),fn(OPRsf),fn(OPRRm),fn(OPRoption<15,13>),fn(OPRimm<12,10>),fn(OPRRn),NULL},1797259295,2145386527},
    {aarch64_op_cmp_subs_addsub_imm,"cmp",{fn(setFlags),fn(OPRsf),fn(OPRshift),fn(OPRimm<21,10>),fn(OPRRn),NULL},1895825439,2130706463},
    {aarch64_op_cmp_subs_addsub_shift,"cmp",{fn(setFlags),fn(OPRsf),fn(OPRshift),fn(OPRRm),fn(OPRimm<15,10>),fn(OPRRn),NULL},1795162143,2132803615},
    {aarch64_op_cmtst_advsimd,"cmtst",{fn(setSIMDMode),fn(OPRsize<23,22>),fn(OPRRm),fn(OPRRn),fn(OPRRd),NULL},1579191296,4280351744},
    {aarch64_op_cmtst_advsimd,"cmtst",{fn(setSIMDMode),fn(OPRQ),fn(OPRsize<23,22>),fn(OPRRm),fn(OPRRn),fn(OPRRd),NULL},237014016,3206609920},
    {aarch64_op_cneg_csneg,"cneg",{fn(OPRsf),fn(OPRcond<15,12>),fn(OPRRn),fn(OPRRd),NULL},1518339072,2145389568},
    {aarch64_op_cnt_advsimd,"cnt",{fn(setSIMDMode),fn(OPRQ),fn(OPRsize<23,22>),fn(OPRRn),fn(OPRRd),NULL},237000704,3208641536},
    {aarch64_op_crc32,"crc32",{fn(OPRsf),fn(OPRRm),fn(OPRsz<11,10>),fn(OPRRn),fn(OPRRd),NULL},448806912,2145447936},
    {aarch64_op_crc32c,"crc32c",{fn(OPRsf),fn(OPRRm),fn(OPRsz<11,10>),fn(OPRRn),fn(OPRRd),NULL},448811008,2145447936},
    {aarch64_op_csel,"csel",{fn(OPRsf),fn(OPRcond<15,12>),fn(OPRRm),fn(OPRRn),fn(OPRRd),NULL},444596224,2145389568},
    {aarch64_op_cset_csinc,"cset",{fn(OPRsf),fn(OPRcond<15,12>),fn(OPRRd),NULL},446629856,2147422176},
    {aarch64_op_csetm_csinv,"csetm",{fn(OPRsf),fn(OPRcond<15,12>),fn(OPRRd),NULL},1520370656,2147422176},
    {aarch64_op_csinc,"csinc",{fn(OPRsf),fn(OPRcond<15,12>),fn(OPRRm),fn(OPRRn),fn(OPRRd),NULL},444597248,2145389568},
    {aarch64_op_csinv,"csinv",{fn(OPRsf),fn(OPRcond<15,12>),fn(OPRRm),fn(OPRRn),fn(OPRRd),NULL},1518338048,2145389568},
    {aarch64_op_csneg,"csneg",{fn(OPRsf),fn(OPRcond<15,12>),fn(OPRRm),fn(OPRRn),fn(OPRRd),NULL},1518339072,2145389568},
    {aarch64_op_dc_sys,"dc",{fn(OPRop1),fn(OPRCRm),fn(OPRop2),fn(OPRRt),NULL},3574099968,4294504448},
    {aarch64_op_dcps1,"dcps1",{fn(OPRimm<20,5>),NULL},3567255553,4292870175},
    {aarch64_op_dcps2,"dcps2",{fn(OPRimm<20,5>),NULL},3567255554,4292870175},
    {aarch64_op_dcps3,"dcps3",{fn(OPRimm<20,5>),NULL},3567255555,4292870175},
    {aarch64_op_dmb,"dmb",{fn(OPRCRm),NULL},3573756095,4294963455},
    {aarch64_op_drps,"drps",{NULL},3602842592,4294967295},
    {aarch64_op_dsb,"dsb",{fn(OPRCRm),NULL},3573756063,4294963455},
    {aarch64_op_dup_advsimd_elt,"dup",{fn(setSIMDMode),fn(OPRimm<20,16>),fn(OPRRn),fn(OPRRd),NULL},1577059328,4292934656},
    {aarch64_op_dup_advsimd_elt,"dup",{fn(setSIMDMode),fn(OPRQ),fn(OPRimm<20,16>),fn(OPRRn),fn(OPRRd),NULL},234882048,3219192832},
    {aarch64_op_dup_advsimd_gen,"dup",{fn(setSIMDMode),fn(OPRQ),fn(OPRimm<20,16>),fn(OPRRn),fn(OPRRd),NULL},234884096,3219192832},
    {aarch64_op_eon,"eon",{fn(OPRsf),fn(OPRshift),fn(OPRRm),fn(OPRimm<15,10>),fn(OPRRn),fn(OPRRd),NULL},1243611136,2132803584},
    {aarch64_op_eor_advsimd,"eor",{fn(setSIMDMode),fn(OPRQ),fn(OPRRm),fn(OPRRn),fn(OPRRd),NULL},773856256,3219192832},
    {aarch64_op_eor_log_imm,"eor",{fn(OPRsf),fn(OPRN<22,22>),fn(OPRimm<21,16>),fn(OPRimm<15,10>),fn(OPRRn),fn(OPRRd),NULL},1375731712,2139095040},
    {aarch64_op_eor_log_shift,"eor",{fn(OPRsf),fn(OPRshift),fn(OPRRm),fn(OPRimm<15,10>),fn(OPRRn),fn(OPRRd),NULL},1241513984,2132803584},
    {aarch64_op_eret,"eret",{NULL},3600745440,4294967295},
    {aarch64_op_ext_advsimd,"ext",{fn(setSIMDMode),fn(OPRQ),fn(OPRRm),fn(OPRimm<14,11>),fn(OPRRn),fn(OPRRd),NULL},771751936,3219162112},
    {aarch64_op_extr,"extr",{fn(OPRsf),fn(OPRN<22,22>),fn(OPRRm),fn(OPRimm<15,10>),fn(OPRRn),fn(OPRRd),NULL},327155712,2141192192},
    {aarch64_op_fabd_advsimd,"fabd",{fn(setSIMDMode),fn(OPRsz<22,22>),fn(OPRRm),fn(OPRRn),fn(OPRRd),NULL},2124469248,4288740352},
    {aarch64_op_fabd_advsimd,"fabd",{fn(setSIMDMode),fn(OPRQ),fn(OPRsz<22,22>),fn(OPRRm),fn(OPRRn),fn(OPRRd),NULL},782291968,3214998528},
    {aarch64_op_fabs_advsimd,"fabs",{fn(setSIMDMode),fn(OPRQ),fn(OPRsz<22,22>),fn(OPRRn),fn(OPRRd),NULL},245430272,3217030144},
    {aarch64_op_fabs_float,"fabs",{fn(setFPMode),fn(OPRtype<23,22>),fn(OPRRn),fn(OPRRd),NULL},505462784,4290771968},
    {aarch64_op_facge_advsimd,"facge",{fn(setSIMDMode),fn(OPRsz<22,22>),fn(OPRRm),fn(OPRRn),fn(OPRRd),NULL},2116086784,4288740352},
    {aarch64_op_facge_advsimd,"facge",{fn(setSIMDMode),fn(OPRQ),fn(OPRsz<22,22>),fn(OPRRm),fn(OPRRn),fn(OPRRd),NULL},773909504,3214998528},
    {aarch64_op_facgt_advsimd,"facgt",{fn(setSIMDMode),fn(OPRsz<22,22>),fn(OPRRm),fn(OPRRn),fn(OPRRd),NULL},2124475392,4288740352},
    {aarch64_op_facgt_advsimd,"facgt",{fn(setSIMDMode),fn(OPRQ),fn(OPRsz<22,22>),fn(OPRRm),fn(OPRRn),fn(OPRRd),NULL},782298112,3214998528},
    {aarch64_op_fadd_advsimd,"fadd",{fn(setSIMDMode),fn(OPRQ),fn(OPRsz<22,22>),fn(OPRRm),fn(OPRRn),fn(OPRRd),NULL},237032448,3214998528},
    {aarch64_op_fadd_float,"fadd",{fn(setFPMode),fn(OPRtype<23,22>),fn(OPRRm),fn(OPRRn),fn(OPRRd),NULL},505423872,4288740352},
    {aarch64_op_faddp_advsimd_pair,"faddp",{fn(setSIMDMode),fn(OPRsz<22,22>),fn(OPRRn),fn(OPRRd),NULL},2117130240,4290771968},
    {aarch64_op_faddp_advsimd_vec,"faddp",{fn(setSIMDMode),fn(OPRQ),fn(OPRsz<22,22>),fn(OPRRm),fn(OPRRn),fn(OPRRd),NULL},773903360,3214998528},
    {aarch64_op_fccmp_float,"fccmp",{fn(setFPMode),fn(setFlags),fn(OPRtype<23,22>),fn(OPRRm),fn(OPRcond<15,12>),fn(OPRRn),fn(OPRnzcv),NULL},505414656,4288678928},
    {aarch64_op_fccmpe_float,"fccmpe",{fn(setFPMode),fn(setFlags),fn(OPRtype<23,22>),fn(OPRRm),fn(OPRcond<15,12>),fn(OPRRn),fn(OPRnzcv),NULL},505414672,4288678928},
    {aarch64_op_fcmeq_advsimd_reg,"fcmeq",{fn(setSIMDMode),fn(OPRsz<22,22>),fn(OPRRm),fn(OPRRn),fn(OPRRd),NULL},1579213824,4288740352},
    {aarch64_op_fcmeq_advsimd_reg,"fcmeq",{fn(setSIMDMode),fn(OPRQ),fn(OPRsz<22,22>),fn(OPRRm),fn(OPRRn),fn(OPRRd),NULL},237036544,3214998528},
    {aarch64_op_fcmeq_advsimd_zero,"fcmeq",{fn(setSIMDMode),fn(OPRsz<22,22>),fn(OPRRn),fn(OPRRd),NULL},1587599360,4290771968},
    {aarch64_op_fcmeq_advsimd_zero,"fcmeq",{fn(setSIMDMode),fn(OPRQ),fn(OPRsz<22,22>),fn(OPRRn),fn(OPRRd),NULL},245422080,3217030144},
    {aarch64_op_fcmge_advsimd_reg,"fcmge",{fn(setSIMDMode),fn(OPRsz<22,22>),fn(OPRRm),fn(OPRRn),fn(OPRRd),NULL},2116084736,4288740352},
    {aarch64_op_fcmge_advsimd_reg,"fcmge",{fn(setSIMDMode),fn(OPRQ),fn(OPRsz<22,22>),fn(OPRRm),fn(OPRRn),fn(OPRRd),NULL},773907456,3214998528},
    {aarch64_op_fcmge_advsimd_zero,"fcmge",{fn(setSIMDMode),fn(OPRsz<22,22>),fn(OPRRn),fn(OPRRd),NULL},2124466176,4290771968},
    {aarch64_op_fcmge_advsimd_zero,"fcmge",{fn(setSIMDMode),fn(OPRQ),fn(OPRsz<22,22>),fn(OPRRn),fn(OPRRd),NULL},782288896,3217030144},
    {aarch64_op_fcmgt_advsimd_reg,"fcmgt",{fn(setSIMDMode),fn(OPRsz<22,22>),fn(OPRRm),fn(OPRRn),fn(OPRRd),NULL},2124473344,4288740352},
    {aarch64_op_fcmgt_advsimd_reg,"fcmgt",{fn(setSIMDMode),fn(OPRQ),fn(OPRsz<22,22>),fn(OPRRm),fn(OPRRn),fn(OPRRd),NULL},782296064,3214998528},
    {aarch64_op_fcmgt_advsimd_zero,"fcmgt",{fn(setSIMDMode),fn(OPRsz<22,22>),fn(OPRRn),fn(OPRRd),NULL},1587595264,4290771968},
    {aarch64_op_fcmgt_advsimd_zero,"fcmgt",{fn(setSIMDMode),fn(OPRQ),fn(OPRsz<22,22>),fn(OPRRn),fn(OPRRd),NULL},245417984,3217030144},
    {aarch64_op_fcmle_advsimd,"fcmle",{fn(setSIMDMode),fn(OPRsz<22,22>),fn(OPRRn),fn(OPRRd),NULL},2124470272,4290771968},
    {aarch64_op_fcmle_advsimd,"fcmle",{fn(setSIMDMode),fn(OPRQ),fn(OPRsz<22,22>),fn(OPRRn),fn(OPRRd),NULL},782292992,3217030144},
    {aarch64_op_fcmlt_advsimd,"fcmlt",{fn(setSIMDMode),fn(OPRsz<22,22>),fn(OPRRn),fn(OPRRd),NULL},1587603456,4290771968},
    {aarch64_op_fcmlt_advsimd,"fcmlt",{fn(setSIMDMode),fn(OPRQ),fn(OPRsz<22,22>),fn(OPRRn),fn(OPRRd),NULL},245426176,3217030144},
    {aarch64_op_fcmp_float,"fcmp",{fn(setFPMode),fn(setFlags),fn(OPRtype<23,22>),fn(OPRRm),fn(OPRRn),fn(OPRopc),NULL},505421824,4288740375},
    {aarch64_op_fcmpe_float,"fcmpe",{fn(setFPMode),fn(setFlags),fn(OPRtype<23,22>),fn(OPRRm),fn(OPRRn),fn(OPRopc),NULL},505421840,4288740375},
    {aarch64_op_fcsel_float,"fcsel",{fn(setFPMode),fn(OPRtype<23,22>),fn(OPRRm),fn(OPRcond<15,12>),fn(OPRRn),fn(OPRRd),NULL},505416704,4288678912},
    {aarch64_op_fcvt_float,"fcvt",{fn(setFPMode),fn(OPRtype<23,22>),fn(OPRopc),fn(OPRRn),fn(OPRRd),NULL},505561088,4282285056},
    {aarch64_op_fcvtas_advsimd,"fcvtas",{fn(setSIMDMode),fn(OPRsz<22,22>),fn(OPRRn),fn(OPRRd),NULL},1579272192,4290771968},
    {aarch64_op_fcvtas_advsimd,"fcvtas",{fn(setSIMDMode),fn(OPRQ),fn(OPRsz<22,22>),fn(OPRRn),fn(OPRRd),NULL},237094912,3217030144},
    {aarch64_op_fcvtas_float,"fcvtas",{fn(setFPMode),fn(OPRsf),fn(OPRtype<23,22>),fn(OPRRn),fn(OPRRd),NULL},505675776,2143288320},
    {aarch64_op_fcvtau_advsimd,"fcvtau",{fn(setSIMDMode),fn(OPRsz<22,22>),fn(OPRRn),fn(OPRRd),NULL},2116143104,4290771968},
    {aarch64_op_fcvtau_advsimd,"fcvtau",{fn(setSIMDMode),fn(OPRQ),fn(OPRsz<22,22>),fn(OPRRn),fn(OPRRd),NULL},773965824,3217030144},
    {aarch64_op_fcvtau_float,"fcvtau",{fn(setFPMode),fn(OPRsf),fn(OPRtype<23,22>),fn(OPRRn),fn(OPRRd),NULL},505741312,2143288320},
    {aarch64_op_fcvtl_advsimd,"fcvtl",{fn(setSIMDMode),fn(OPRQ),fn(OPRsz<22,22>),fn(OPRRn),fn(OPRRd),NULL},237074432,3217030144},
    {aarch64_op_fcvtms_advsimd,"fcvtms",{fn(setSIMDMode),fn(OPRsz<22,22>),fn(OPRRn),fn(OPRRd),NULL},1579268096,4290771968},
    {aarch64_op_fcvtms_advsimd,"fcvtms",{fn(setSIMDMode),fn(OPRQ),fn(OPRsz<22,22>),fn(OPRRn),fn(OPRRd),NULL},237090816,3217030144},
    {aarch64_op_fcvtms_float,"fcvtms",{fn(setFPMode),fn(OPRsf),fn(OPRtype<23,22>),fn(OPRRn),fn(OPRRd),NULL},506462208,2143288320},
    {aarch64_op_fcvtmu_advsimd,"fcvtmu",{fn(setSIMDMode),fn(OPRsz<22,22>),fn(OPRRn),fn(OPRRd),NULL},2116139008,4290771968},
    {aarch64_op_fcvtmu_advsimd,"fcvtmu",{fn(setSIMDMode),fn(OPRQ),fn(OPRsz<22,22>),fn(OPRRn),fn(OPRRd),NULL},773961728,3217030144},
    {aarch64_op_fcvtmu_float,"fcvtmu",{fn(setFPMode),fn(OPRsf),fn(OPRtype<23,22>),fn(OPRRn),fn(OPRRd),NULL},506527744,2143288320},
    {aarch64_op_fcvtn_advsimd,"fcvtn",{fn(setSIMDMode),fn(OPRQ),fn(OPRsz<22,22>),fn(OPRRn),fn(OPRRd),NULL},237070336,3217030144},
    {aarch64_op_fcvtns_advsimd,"fcvtns",{fn(setSIMDMode),fn(OPRsz<22,22>),fn(OPRRn),fn(OPRRd),NULL},1579264000,4290771968},
    {aarch64_op_fcvtns_advsimd,"fcvtns",{fn(setSIMDMode),fn(OPRQ),fn(OPRsz<22,22>),fn(OPRRn),fn(OPRRd),NULL},237086720,3217030144},
    {aarch64_op_fcvtns_float,"fcvtns",{fn(setFPMode),fn(OPRsf),fn(OPRtype<23,22>),fn(OPRRn),fn(OPRRd),NULL},505413632,2143288320},
    {aarch64_op_fcvtnu_advsimd,"fcvtnu",{fn(setSIMDMode),fn(OPRsz<22,22>),fn(OPRRn),fn(OPRRd),NULL},2116134912,4290771968},
    {aarch64_op_fcvtnu_advsimd,"fcvtnu",{fn(setSIMDMode),fn(OPRQ),fn(OPRsz<22,22>),fn(OPRRn),fn(OPRRd),NULL},773957632,3217030144},
    {aarch64_op_fcvtnu_float,"fcvtnu",{fn(setFPMode),fn(OPRsf),fn(OPRtype<23,22>),fn(OPRRn),fn(OPRRd),NULL},505479168,2143288320},
    {aarch64_op_fcvtps_advsimd,"fcvtps",{fn(setSIMDMode),fn(OPRsz<22,22>),fn(OPRRn),fn(OPRRd),NULL},1587652608,4290771968},
    {aarch64_op_fcvtps_advsimd,"fcvtps",{fn(setSIMDMode),fn(OPRQ),fn(OPRsz<22,22>),fn(OPRRn),fn(OPRRd),NULL},245475328,3217030144},
    {aarch64_op_fcvtps_float,"fcvtps",{fn(setFPMode),fn(OPRsf),fn(OPRtype<23,22>),fn(OPRRn),fn(OPRRd),NULL},505937920,2143288320},
    {aarch64_op_fcvtpu_advsimd,"fcvtpu",{fn(setSIMDMode),fn(OPRsz<22,22>),fn(OPRRn),fn(OPRRd),NULL},2124523520,4290771968},
    {aarch64_op_fcvtpu_advsimd,"fcvtpu",{fn(setSIMDMode),fn(OPRQ),fn(OPRsz<22,22>),fn(OPRRn),fn(OPRRd),NULL},782346240,3217030144},
    {aarch64_op_fcvtpu_float,"fcvtpu",{fn(setFPMode),fn(OPRsf),fn(OPRtype<23,22>),fn(OPRRn),fn(OPRRd),NULL},506003456,2143288320},
    {aarch64_op_fcvtxn_advsimd,"fcvtxn",{fn(setSIMDMode),fn(OPRsz<22,22>),fn(OPRRn),fn(OPRRd),NULL},2116118528,4290771968},
    {aarch64_op_fcvtxn_advsimd,"fcvtxn",{fn(setSIMDMode),fn(OPRQ),fn(OPRsz<22,22>),fn(OPRRn),fn(OPRRd),NULL},773941248,3217030144},
    {aarch64_op_fcvtzs_advsimd_fix,"fcvtzs",{fn(setSIMDMode),fn(OPRimm<22,19>),fn(OPRimm<18,16>),fn(OPRRn),fn(OPRRd),NULL},1593900032,4286643200},
    {aarch64_op_fcvtzs_advsimd_fix,"fcvtzs",{fn(setSIMDMode),fn(OPRQ),fn(OPRimm<22,19>),fn(OPRimm<18,16>),fn(OPRRn),fn(OPRRd),NULL},251722752,3212901376},
    {aarch64_op_fcvtzs_advsimd_int,"fcvtzs",{fn(setSIMDMode),fn(OPRsz<22,22>),fn(OPRRn),fn(OPRRd),NULL},1587656704,4290771968},
    {aarch64_op_fcvtzs_advsimd_int,"fcvtzs",{fn(setSIMDMode),fn(OPRQ),fn(OPRsz<22,22>),fn(OPRRn),fn(OPRRd),NULL},245479424,3217030144},
    {aarch64_op_fcvtzs_float_fix,"fcvtzs",{fn(setFPMode),fn(OPRsf),fn(OPRtype<23,22>),fn(OPRscale),fn(OPRRn),fn(OPRRd),NULL},504889344,2143223808},
    {aarch64_op_fcvtzs_float_int,"fcvtzs",{fn(setFPMode),fn(OPRsf),fn(OPRtype<23,22>),fn(OPRRn),fn(OPRRd),NULL},506986496,2143288320},
    {aarch64_op_fcvtzu_advsimd_fix,"fcvtzu",{fn(setSIMDMode),fn(OPRimm<22,19>),fn(OPRimm<18,16>),fn(OPRRn),fn(OPRRd),NULL},2130770944,4286643200},
    {aarch64_op_fcvtzu_advsimd_fix,"fcvtzu",{fn(setSIMDMode),fn(OPRQ),fn(OPRimm<22,19>),fn(OPRimm<18,16>),fn(OPRRn),fn(OPRRd),NULL},788593664,3212901376},
    {aarch64_op_fcvtzu_advsimd_int,"fcvtzu",{fn(setSIMDMode),fn(OPRsz<22,22>),fn(OPRRn),fn(OPRRd),NULL},2124527616,4290771968},
    {aarch64_op_fcvtzu_advsimd_int,"fcvtzu",{fn(setSIMDMode),fn(OPRQ),fn(OPRsz<22,22>),fn(OPRRn),fn(OPRRd),NULL},782350336,3217030144},
    {aarch64_op_fcvtzu_float_fix,"fcvtzu",{fn(setFPMode),fn(OPRsf),fn(OPRtype<23,22>),fn(OPRscale),fn(OPRRn),fn(OPRRd),NULL},504954880,2143223808},
    {aarch64_op_fcvtzu_float_int,"fcvtzu",{fn(setFPMode),fn(OPRsf),fn(OPRtype<23,22>),fn(OPRRn),fn(OPRRd),NULL},507052032,2143288320},
    {aarch64_op_fdiv_advsimd,"fdiv",{fn(setSIMDMode),fn(OPRQ),fn(OPRsz<22,22>),fn(OPRRm),fn(OPRRn),fn(OPRRd),NULL},773913600,3214998528},
    {aarch64_op_fdiv_float,"fdiv",{fn(setFPMode),fn(OPRtype<23,22>),fn(OPRRm),fn(OPRRn),fn(OPRRd),NULL},505419776,4288740352},
    {aarch64_op_fmadd_float,"fmadd",{fn(setFPMode),fn(OPRtype<23,22>),fn(OPRRm),fn(OPRRa),fn(OPRRn),fn(OPRRd),NULL},520093696,4288708608},
    {aarch64_op_fmax_advsimd,"fmax",{fn(setSIMDMode),fn(OPRQ),fn(OPRsz<22,22>),fn(OPRRm),fn(OPRRn),fn(OPRRd),NULL},237040640,3214998528},
    {aarch64_op_fmax_float,"fmax",{fn(setFPMode),fn(OPRtype<23,22>),fn(OPRRm),fn(OPRRn),fn(OPRRd),NULL},505432064,4288740352},
    {aarch64_op_fmaxnm_advsimd,"fmaxnm",{fn(setSIMDMode),fn(OPRQ),fn(OPRsz<22,22>),fn(OPRRm),fn(OPRRn),fn(OPRRd),NULL},237028352,3214998528},
    {aarch64_op_fmaxnm_float,"fmaxnm",{fn(setFPMode),fn(OPRtype<23,22>),fn(OPRRm),fn(OPRRn),fn(OPRRd),NULL},505440256,4288740352},
    {aarch64_op_fmaxnmp_advsimd_pair,"fmaxnmp",{fn(setSIMDMode),fn(OPRsz<22,22>),fn(OPRRn),fn(OPRRd),NULL},2117126144,4290771968},
    {aarch64_op_fmaxnmp_advsimd_vec,"fmaxnmp",{fn(setSIMDMode),fn(OPRQ),fn(OPRsz<22,22>),fn(OPRRm),fn(OPRRn),fn(OPRRd),NULL},773899264,3214998528},
    {aarch64_op_fmaxnmv_advsimd,"fmaxnmv",{fn(setSIMDMode),fn(OPRQ),fn(OPRsz<22,22>),fn(OPRRn),fn(OPRRd),NULL},774948864,3217030144},
    {aarch64_op_fmaxp_advsimd_pair,"fmaxp",{fn(setSIMDMode),fn(OPRsz<22,22>),fn(OPRRn),fn(OPRRd),NULL},2117138432,4290771968},
    {aarch64_op_fmaxp_advsimd_vec,"fmaxp",{fn(setSIMDMode),fn(OPRQ),fn(OPRsz<22,22>),fn(OPRRm),fn(OPRRn),fn(OPRRd),NULL},773911552,3214998528},
    {aarch64_op_fmaxv_advsimd,"fmaxv",{fn(setSIMDMode),fn(OPRQ),fn(OPRsz<22,22>),fn(OPRRn),fn(OPRRd),NULL},774961152,3217030144},
    {aarch64_op_fmin_advsimd,"fmin",{fn(setSIMDMode),fn(OPRQ),fn(OPRsz<22,22>),fn(OPRRm),fn(OPRRn),fn(OPRRd),NULL},245429248,3214998528},
    {aarch64_op_fmin_float,"fmin",{fn(setFPMode),fn(OPRtype<23,22>),fn(OPRRm),fn(OPRRn),fn(OPRRd),NULL},505436160,4288740352},
    {aarch64_op_fminnm_advsimd,"fminnm",{fn(setSIMDMode),fn(OPRQ),fn(OPRsz<22,22>),fn(OPRRm),fn(OPRRn),fn(OPRRd),NULL},245416960,3214998528},
    {aarch64_op_fminnm_float,"fminnm",{fn(setFPMode),fn(OPRtype<23,22>),fn(OPRRm),fn(OPRRn),fn(OPRRd),NULL},505444352,4288740352},
    {aarch64_op_fminnmp_advsimd_pair,"fminnmp",{fn(setSIMDMode),fn(OPRsz<22,22>),fn(OPRRn),fn(OPRRd),NULL},2125514752,4290771968},
    {aarch64_op_fminnmp_advsimd_vec,"fminnmp",{fn(setSIMDMode),fn(OPRQ),fn(OPRsz<22,22>),fn(OPRRm),fn(OPRRn),fn(OPRRd),NULL},782287872,3214998528},
    {aarch64_op_fminnmv_advsimd,"fminnmv",{fn(setSIMDMode),fn(OPRQ),fn(OPRsz<22,22>),fn(OPRRn),fn(OPRRd),NULL},783337472,3217030144},
    {aarch64_op_fminp_advsimd_pair,"fminp",{fn(setSIMDMode),fn(OPRsz<22,22>),fn(OPRRn),fn(OPRRd),NULL},2125527040,4290771968},
    {aarch64_op_fminp_advsimd_vec,"fminp",{fn(setSIMDMode),fn(OPRQ),fn(OPRsz<22,22>),fn(OPRRm),fn(OPRRn),fn(OPRRd),NULL},782300160,3214998528},
    {aarch64_op_fminv_advsimd,"fminv",{fn(setSIMDMode),fn(OPRQ),fn(OPRsz<22,22>),fn(OPRRn),fn(OPRRd),NULL},783349760,3217030144},
    {aarch64_op_fmla_advsimd_elt,"fmla",{fn(setSIMDMode),fn(OPRsz<22,22>),fn(OPRL),fn(OPRM),fn(OPRRm),fn(OPRH),fn(OPRRn),fn(OPRRd),NULL},1602228224,4286641152},
    {aarch64_op_fmla_advsimd_elt,"fmla",{fn(setSIMDMode),fn(OPRQ),fn(OPRsz<22,22>),fn(OPRL),fn(OPRM),fn(OPRRm),fn(OPRH),fn(OPRRn),fn(OPRRd),NULL},260050944,3212899328},
    {aarch64_op_fmla_advsimd_vec,"fmla",{fn(setSIMDMode),fn(OPRQ),fn(OPRsz<22,22>),fn(OPRRm),fn(OPRRn),fn(OPRRd),NULL},237030400,3214998528},
    {aarch64_op_fmls_advsimd_elt,"fmls",{fn(setSIMDMode),fn(OPRsz<22,22>),fn(OPRL),fn(OPRM),fn(OPRRm),fn(OPRH),fn(OPRRn),fn(OPRRd),NULL},1602244608,4286641152},
    {aarch64_op_fmls_advsimd_elt,"fmls",{fn(setSIMDMode),fn(OPRQ),fn(OPRsz<22,22>),fn(OPRL),fn(OPRM),fn(OPRRm),fn(OPRH),fn(OPRRn),fn(OPRRd),NULL},260067328,3212899328},
    {aarch64_op_fmls_advsimd_vec,"fmls",{fn(setSIMDMode),fn(OPRQ),fn(OPRsz<22,22>),fn(OPRRm),fn(OPRRn),fn(OPRRd),NULL},245419008,3214998528},
    {aarch64_op_fmov_advsimd,"fmov",{fn(setSIMDMode),fn(OPRQ),fn(OPRop),fn(OPRa),fn(OPRb),fn(OPRc),fn(OPRd),fn(OPRe),fn(OPRf),fn(OPRg),fn(OPRh),fn(OPRRd),NULL},251720704,2683894784},
    {aarch64_op_fmov_float,"fmov",{fn(setFPMode),fn(OPRtype<23,22>),fn(OPRRn),fn(OPRRd),NULL},505430016,4290771968},
    {aarch64_op_fmov_float_gen,"fmov",{fn(setFPMode),fn(OPRsf),fn(OPRtype<23,22>),fn(OPRrmode),fn(OPRopcode),fn(OPRRn),fn(OPRRd),NULL},505806848,2134309888},
    {aarch64_op_fmov_float_imm,"fmov",{fn(setFPMode),fn(OPRtype<23,22>),fn(OPRimm<20,13>),fn(OPRRd),NULL},505417728,4288684000},
    {aarch64_op_fmsub_float,"fmsub",{fn(setFPMode),fn(OPRtype<23,22>),fn(OPRRm),fn(OPRRa),fn(OPRRn),fn(OPRRd),NULL},520126464,4288708608},
    {aarch64_op_fmul_advsimd_elt,"fmul",{fn(setSIMDMode),fn(OPRsz<22,22>),fn(OPRL),fn(OPRM),fn(OPRRm),fn(OPRH),fn(OPRRn),fn(OPRRd),NULL},1602260992,4286641152},
    {aarch64_op_fmul_advsimd_elt,"fmul",{fn(setSIMDMode),fn(OPRQ),fn(OPRsz<22,22>),fn(OPRL),fn(OPRM),fn(OPRRm),fn(OPRH),fn(OPRRn),fn(OPRRd),NULL},260083712,3212899328},
    {aarch64_op_fmul_advsimd_vec,"fmul",{fn(setSIMDMode),fn(OPRQ),fn(OPRsz<22,22>),fn(OPRRm),fn(OPRRn),fn(OPRRd),NULL},773905408,3214998528},
    {aarch64_op_fmul_float,"fmul",{fn(setFPMode),fn(OPRtype<23,22>),fn(OPRRm),fn(OPRRn),fn(OPRRd),NULL},505415680,4288740352},
    {aarch64_op_fmulx_advsimd_elt,"fmulx",{fn(setSIMDMode),fn(OPRsz<22,22>),fn(OPRL),fn(OPRM),fn(OPRRm),fn(OPRH),fn(OPRRn),fn(OPRRd),NULL},2139131904,4286641152},
    {aarch64_op_fmulx_advsimd_elt,"fmulx",{fn(setSIMDMode),fn(OPRQ),fn(OPRsz<22,22>),fn(OPRL),fn(OPRM),fn(OPRRm),fn(OPRH),fn(OPRRn),fn(OPRRd),NULL},796954624,3212899328},
    {aarch64_op_fmulx_advsimd_vec,"fmulx",{fn(setSIMDMode),fn(OPRsz<22,22>),fn(OPRRm),fn(OPRRn),fn(OPRRd),NULL},1579211776,4288740352},
    {aarch64_op_fmulx_advsimd_vec,"fmulx",{fn(setSIMDMode),fn(OPRQ),fn(OPRsz<22,22>),fn(OPRRm),fn(OPRRn),fn(OPRRd),NULL},237034496,3214998528},
    {aarch64_op_fneg_advsimd,"fneg",{fn(setSIMDMode),fn(OPRQ),fn(OPRsz<22,22>),fn(OPRRn),fn(OPRRd),NULL},782301184,3217030144},
    {aarch64_op_fneg_float,"fneg",{fn(setFPMode),fn(OPRtype<23,22>),fn(OPRRn),fn(OPRRd),NULL},505495552,4290771968},
    {aarch64_op_fnmadd_float,"fnmadd",{fn(setFPMode),fn(OPRtype<23,22>),fn(OPRRm),fn(OPRRa),fn(OPRRn),fn(OPRRd),NULL},522190848,4288708608},
    {aarch64_op_fnmsub_float,"fnmsub",{fn(setFPMode),fn(OPRtype<23,22>),fn(OPRRm),fn(OPRRa),fn(OPRRn),fn(OPRRd),NULL},522223616,4288708608},
    {aarch64_op_fnmul_float,"fnmul",{fn(setFPMode),fn(OPRtype<23,22>),fn(OPRRm),fn(OPRRn),fn(OPRRd),NULL},505448448,4288740352},
    {aarch64_op_frecpe_advsimd,"frecpe",{fn(setSIMDMode),fn(OPRsz<22,22>),fn(OPRRn),fn(OPRRd),NULL},1587664896,4290771968},
    {aarch64_op_frecpe_advsimd,"frecpe",{fn(setSIMDMode),fn(OPRQ),fn(OPRsz<22,22>),fn(OPRRn),fn(OPRRd),NULL},245487616,3217030144},
    {aarch64_op_frecps_advsimd,"frecps",{fn(setSIMDMode),fn(OPRsz<22,22>),fn(OPRRm),fn(OPRRn),fn(OPRRd),NULL},1579219968,4288740352},
    {aarch64_op_frecps_advsimd,"frecps",{fn(setSIMDMode),fn(OPRQ),fn(OPRsz<22,22>),fn(OPRRm),fn(OPRRn),fn(OPRRd),NULL},237042688,3214998528},
    {aarch64_op_frecpx_advsimd,"frecpx",{fn(setSIMDMode),fn(OPRsz<22,22>),fn(OPRRn),fn(OPRRd),NULL},1587673088,4290771968},
    {aarch64_op_frinta_advsimd,"frinta",{fn(setSIMDMode),fn(OPRQ),fn(OPRsz<22,22>),fn(OPRRn),fn(OPRRd),NULL},773949440,3217030144},
    {aarch64_op_frinta_float,"frinta",{fn(setFPMode),fn(OPRtype<23,22>),fn(OPRRn),fn(OPRRd),NULL},505823232,4290771968},
    {aarch64_op_frinti_advsimd,"frinti",{fn(setSIMDMode),fn(OPRQ),fn(OPRsz<22,22>),fn(OPRRn),fn(OPRRd),NULL},782342144,3217030144},
    {aarch64_op_frinti_float,"frinti",{fn(setFPMode),fn(OPRtype<23,22>),fn(OPRRn),fn(OPRRd),NULL},505921536,4290771968},
    {aarch64_op_frintm_advsimd,"frintm",{fn(setSIMDMode),fn(OPRQ),fn(OPRsz<22,22>),fn(OPRRn),fn(OPRRd),NULL},237082624,3217030144},
    {aarch64_op_frintm_float,"frintm",{fn(setFPMode),fn(OPRtype<23,22>),fn(OPRRn),fn(OPRRd),NULL},505757696,4290771968},
    {aarch64_op_frintn_advsimd,"frintn",{fn(setSIMDMode),fn(OPRQ),fn(OPRsz<22,22>),fn(OPRRn),fn(OPRRd),NULL},237078528,3217030144},
    {aarch64_op_frintn_float,"frintn",{fn(setFPMode),fn(OPRtype<23,22>),fn(OPRRn),fn(OPRRd),NULL},505692160,4290771968},
    {aarch64_op_frintp_advsimd,"frintp",{fn(setSIMDMode),fn(OPRQ),fn(OPRsz<22,22>),fn(OPRRn),fn(OPRRd),NULL},245467136,3217030144},
    {aarch64_op_frintp_float,"frintp",{fn(setFPMode),fn(OPRtype<23,22>),fn(OPRRn),fn(OPRRd),NULL},505724928,4290771968},
    {aarch64_op_frintx_advsimd,"frintx",{fn(setSIMDMode),fn(OPRQ),fn(OPRsz<22,22>),fn(OPRRn),fn(OPRRd),NULL},773953536,3217030144},
    {aarch64_op_frintx_float,"frintx",{fn(setFPMode),fn(OPRtype<23,22>),fn(OPRRn),fn(OPRRd),NULL},505888768,4290771968},
    {aarch64_op_frintz_advsimd,"frintz",{fn(setSIMDMode),fn(OPRQ),fn(OPRsz<22,22>),fn(OPRRn),fn(OPRRd),NULL},245471232,3217030144},
    {aarch64_op_frintz_float,"frintz",{fn(setFPMode),fn(OPRtype<23,22>),fn(OPRRn),fn(OPRRd),NULL},505790464,4290771968},
    {aarch64_op_frsqrte_advsimd,"frsqrte",{fn(setSIMDMode),fn(OPRsz<22,22>),fn(OPRRn),fn(OPRRd),NULL},2124535808,4290771968},
    {aarch64_op_frsqrte_advsimd,"frsqrte",{fn(setSIMDMode),fn(OPRQ),fn(OPRsz<22,22>),fn(OPRRn),fn(OPRRd),NULL},782358528,3217030144},
    {aarch64_op_frsqrts_advsimd,"frsqrts",{fn(setSIMDMode),fn(OPRsz<22,22>),fn(OPRRm),fn(OPRRn),fn(OPRRd),NULL},1587608576,4288740352},
    {aarch64_op_frsqrts_advsimd,"frsqrts",{fn(setSIMDMode),fn(OPRQ),fn(OPRsz<22,22>),fn(OPRRm),fn(OPRRn),fn(OPRRd),NULL},245431296,3214998528},
    {aarch64_op_fsqrt_advsimd,"fsqrt",{fn(setSIMDMode),fn(OPRQ),fn(OPRsz<22,22>),fn(OPRRn),fn(OPRRd),NULL},782366720,3217030144},
    {aarch64_op_fsqrt_float,"fsqrt",{fn(setFPMode),fn(OPRtype<23,22>),fn(OPRRn),fn(OPRRd),NULL},505528320,4290771968},
    {aarch64_op_fsub_advsimd,"fsub",{fn(setSIMDMode),fn(OPRQ),fn(OPRsz<22,22>),fn(OPRRm),fn(OPRRn),fn(OPRRd),NULL},245421056,3214998528},
    {aarch64_op_fsub_float,"fsub",{fn(setFPMode),fn(OPRtype<23,22>),fn(OPRRm),fn(OPRRn),fn(OPRRd),NULL},505427968,4288740352},
    {aarch64_op_hint,"hint",{fn(OPRCRm),fn(OPRop2),NULL},3573751839,4294963231},
    {aarch64_op_hlt,"hlt",{fn(OPRimm<20,5>),NULL},3560964096,4292870175},
    {aarch64_op_hvc,"hvc",{fn(OPRimm<20,5>),NULL},3556769794,4292870175},
    {aarch64_op_ic_sys,"ic",{fn(OPRop1),fn(OPRCRm),fn(OPRop2),fn(OPRRt),NULL},3574099968,4294504448},
    {aarch64_op_ins_advsimd_elt,"ins",{fn(setSIMDMode),fn(OPRimm<20,16>),fn(OPRimm<14,11>),fn(OPRRn),fn(OPRRd),NULL},1845494784,4292903936},
    {aarch64_op_ins_advsimd_gen,"ins",{fn(setSIMDMode),fn(OPRimm<20,16>),fn(OPRRn),fn(OPRRd),NULL},1308630016,4292934656},
    {aarch64_op_isb,"isb",{fn(OPRCRm),NULL},3573756127,4294963455},
    {aarch64_op_ld1_advsimd_mult,"ld1",{fn(setSIMDMode),fn(OPRQ),fn(OPRopcode),fn(OPRsize<11,10>),fn(OPRRnL),fn(OPRRtL),NULL},205529088,3221168128},
    {aarch64_op_ld1_advsimd_mult,"ld1",{fn(setSIMDMode),fn(OPRQ),fn(OPRRm),fn(OPRopcode),fn(OPRsize<11,10>),fn(OPRRnL),fn(OPRRtL),NULL},213917696,3219136512},
    {aarch64_op_ld1_advsimd_sngl,"ld1",{fn(setSIMDMode),fn(OPRQ),fn(OPRopcode),fn(OPRS<12,12>),fn(OPRsize<11,10>),fn(OPRRnL),fn(OPRRtL),NULL},222298112,3221168128},
    {aarch64_op_ld1_advsimd_sngl,"ld1",{fn(setSIMDMode),fn(OPRQ),fn(OPRRm),fn(OPRopcode),fn(OPRS<12,12>),fn(OPRsize<11,10>),fn(OPRRnL),fn(OPRRtL),NULL},230686720,3219136512},
    {aarch64_op_ld1r_advsimd,"ld1r",{fn(setSIMDMode),fn(OPRQ),fn(OPRsize<11,10>),fn(OPRRnL),fn(OPRRtL),NULL},222347264,3221221376},
    {aarch64_op_ld1r_advsimd,"ld1r",{fn(setSIMDMode),fn(OPRQ),fn(OPRRm),fn(OPRsize<11,10>),fn(OPRRnL),fn(OPRRtL),NULL},230735872,3219189760},
    {aarch64_op_ld2_advsimd_mult,"ld2",{fn(setSIMDMode),fn(OPRQ),fn(OPRsize<11,10>),fn(OPRRnL),fn(OPRRtL),NULL},205553664,3221221376},
    {aarch64_op_ld2_advsimd_mult,"ld2",{fn(setSIMDMode),fn(OPRQ),fn(OPRRm),fn(OPRsize<11,10>),fn(OPRRnL),fn(OPRRtL),NULL},213942272,3219189760},
    {aarch64_op_ld2_advsimd_sngl,"ld2",{fn(setSIMDMode),fn(OPRQ),fn(OPRopcode),fn(OPRS<12,12>),fn(OPRsize<11,10>),fn(OPRRnL),fn(OPRRtL),NULL},224395264,3221168128},
    {aarch64_op_ld2_advsimd_sngl,"ld2",{fn(setSIMDMode),fn(OPRQ),fn(OPRRm),fn(OPRopcode),fn(OPRS<12,12>),fn(OPRsize<11,10>),fn(OPRRnL),fn(OPRRtL),NULL},232783872,3219136512},
    {aarch64_op_ld2r_advsimd,"ld2r",{fn(setSIMDMode),fn(OPRQ),fn(OPRsize<11,10>),fn(OPRRnL),fn(OPRRtL),NULL},224444416,3221221376},
    {aarch64_op_ld2r_advsimd,"ld2r",{fn(setSIMDMode),fn(OPRQ),fn(OPRRm),fn(OPRsize<11,10>),fn(OPRRnL),fn(OPRRtL),NULL},232833024,3219189760},
    {aarch64_op_ld3_advsimd_mult,"ld3",{fn(setSIMDMode),fn(OPRQ),fn(OPRsize<11,10>),fn(OPRRnL),fn(OPRRtL),NULL},205537280,3221221376},
    {aarch64_op_ld3_advsimd_mult,"ld3",{fn(setSIMDMode),fn(OPRQ),fn(OPRRm),fn(OPRsize<11,10>),fn(OPRRnL),fn(OPRRtL),NULL},213925888,3219189760},
    {aarch64_op_ld3_advsimd_sngl,"ld3",{fn(setSIMDMode),fn(OPRQ),fn(OPRopcode),fn(OPRS<12,12>),fn(OPRsize<11,10>),fn(OPRRnL),fn(OPRRtL),NULL},222306304,3221168128},
    {aarch64_op_ld3_advsimd_sngl,"ld3",{fn(setSIMDMode),fn(OPRQ),fn(OPRRm),fn(OPRopcode),fn(OPRS<12,12>),fn(OPRsize<11,10>),fn(OPRRnL),fn(OPRRtL),NULL},230694912,3219136512},
    {aarch64_op_ld3r_advsimd,"ld3r",{fn(setSIMDMode),fn(OPRQ),fn(OPRsize<11,10>),fn(OPRRnL),fn(OPRRtL),NULL},222355456,3221221376},
    {aarch64_op_ld3r_advsimd,"ld3r",{fn(setSIMDMode),fn(OPRQ),fn(OPRRm),fn(OPRsize<11,10>),fn(OPRRnL),fn(OPRRtL),NULL},230744064,3219189760},
    {aarch64_op_ld4_advsimd_mult,"ld4",{fn(setSIMDMode),fn(OPRQ),fn(OPRsize<11,10>),fn(OPRRnL),fn(OPRRtL),NULL},205520896,3221221376},
    {aarch64_op_ld4_advsimd_mult,"ld4",{fn(setSIMDMode),fn(OPRQ),fn(OPRRm),fn(OPRsize<11,10>),fn(OPRRnL),fn(OPRRtL),NULL},213909504,3219189760},
    {aarch64_op_ld4_advsimd_sngl,"ld4",{fn(setSIMDMode),fn(OPRQ),fn(OPRopcode),fn(OPRS<12,12>),fn(OPRsize<11,10>),fn(OPRRnL),fn(OPRRtL),NULL},224403456,3221168128},
    {aarch64_op_ld4_advsimd_sngl,"ld4",{fn(setSIMDMode),fn(OPRQ),fn(OPRRm),fn(OPRopcode),fn(OPRS<12,12>),fn(OPRsize<11,10>),fn(OPRRnL),fn(OPRRtL),NULL},232792064,3219136512},
    {aarch64_op_ld4r_advsimd,"ld4r",{fn(setSIMDMode),fn(OPRQ),fn(OPRsize<11,10>),fn(OPRRnL),fn(OPRRtL),NULL},224452608,3221221376},
    {aarch64_op_ld4r_advsimd,"ld4r",{fn(setSIMDMode),fn(OPRQ),fn(OPRRm),fn(OPRsize<11,10>),fn(OPRRnL),fn(OPRRtL),NULL},232841216,3219189760},
    {aarch64_op_ldar,"ldar",{fn(setRegWidth),fn(OPRsize<31,30>),fn(OPRRnL),fn(OPRRtL),NULL},2296380416,3221224448},
    {aarch64_op_ldarb,"ldarb",{fn(setRegWidth),fn(OPRRnL),fn(OPRRtL),NULL},148896768,4294966272},
    {aarch64_op_ldarh,"ldarh",{fn(setRegWidth),fn(OPRRnL),fn(OPRRtL),NULL},1222638592,4294966272},
    {aarch64_op_ldaxp,"ldaxp",{fn(setRegWidth),fn(OPRsz<30,30>),fn(OPRRt2L),fn(OPRRnL),fn(OPRRtL),NULL},2290057216,3221192704},
    {aarch64_op_ldaxr,"ldaxr",{fn(setRegWidth),fn(OPRsize<31,30>),fn(OPRRnL),fn(OPRRtL),NULL},2287991808,3221224448},
    {aarch64_op_ldaxrb,"ldaxrb",{fn(setRegWidth),fn(OPRRnL),fn(OPRRtL),NULL},140508160,4294966272},
    {aarch64_op_ldaxrh,"ldaxrh",{fn(setRegWidth),fn(OPRRnL),fn(OPRRtL),NULL},1214249984,4294966272},
    {aarch64_op_ldnp_fpsimd,"ldnp",{fn(setRegWidth),fn(setSIMDMode),fn(OPRopc),fn(OPRimm<21,15>),fn(OPRRt2L),fn(OPRRnL),fn(OPRRtL),NULL},742391808,1069547520},
    {aarch64_op_ldnp_gen,"ldnp",{fn(setRegWidth),fn(OPRopc),fn(OPRimm<21,15>),fn(OPRRt2L),fn(OPRRnL),fn(OPRRtL),NULL},675282944,2143289344},
    {aarch64_op_ldp_fpsimd,"ldp",{fn(setRegWidth),fn(setSIMDMode),fn(OPRopc),fn(OPRimm<21,15>),fn(OPRRt2L),fn(OPRRnLU),fn(OPRRtL),NULL},750780416,1069547520},
    {aarch64_op_ldp_fpsimd,"ldp",{fn(setRegWidth),fn(setSIMDMode),fn(OPRopc),fn(OPRimm<21,15>),fn(OPRRt2L),fn(OPRRnLU),fn(OPRRtL),NULL},767557632,1069547520},
    {aarch64_op_ldp_fpsimd,"ldp",{fn(setRegWidth),fn(setSIMDMode),fn(OPRopc),fn(OPRimm<21,15>),fn(OPRRt2L),fn(OPRRnL),fn(OPRRtL),NULL},759169024,1069547520},
    {aarch64_op_ldp_gen,"ldp",{fn(setRegWidth),fn(OPRopc),fn(OPRimm<21,15>),fn(OPRRt2L),fn(OPRRnLU),fn(OPRRtL),NULL},683671552,2143289344},
    {aarch64_op_ldp_gen,"ldp",{fn(setRegWidth),fn(OPRopc),fn(OPRimm<21,15>),fn(OPRRt2L),fn(OPRRnLU),fn(OPRRtL),NULL},700448768,2143289344},
    {aarch64_op_ldp_gen,"ldp",{fn(setRegWidth),fn(OPRopc),fn(OPRimm<21,15>),fn(OPRRt2L),fn(OPRRnL),fn(OPRRtL),NULL},692060160,2143289344},
    {aarch64_op_ldpsw,"ldpsw",{fn(setRegWidth),fn(OPRimm<21,15>),fn(OPRRt2L),fn(OPRRnLU),fn(OPRRtL),NULL},1757413376,4290772992},
    {aarch64_op_ldpsw,"ldpsw",{fn(setRegWidth),fn(OPRimm<21,15>),fn(OPRRt2L),fn(OPRRnLU),fn(OPRRtL),NULL},1774190592,4290772992},
    {aarch64_op_ldpsw,"ldpsw",{fn(setRegWidth),fn(OPRimm<21,15>),fn(OPRRt2L),fn(OPRRnL),fn(OPRRtL),NULL},1765801984,4290772992},
    {aarch64_op_ldr_imm_fpsimd,"ldr",{fn(setRegWidth),fn(setSIMDMode),fn(OPRsize<31,30>),fn(OPRopc),fn(OPRimm<20,12>),fn(OPRRnLU),fn(OPRRtL),NULL},1010828288,1063259136},
    {aarch64_op_ldr_imm_fpsimd,"ldr",{fn(setRegWidth),fn(setSIMDMode),fn(OPRsize<31,30>),fn(OPRopc),fn(OPRimm<20,12>),fn(OPRRnLU),fn(OPRRtL),NULL},1010830336,1063259136},
    {aarch64_op_ldr_imm_fpsimd,"ldr",{fn(setRegWidth),fn(setSIMDMode),fn(OPRsize<31,30>),fn(OPRopc),fn(OPRimm<21,10>),fn(OPRRnL),fn(OPRRtL),NULL},1027604480,1061158912},
    {aarch64_op_ldr_imm_gen,"ldr",{fn(setRegWidth),fn(OPRsize<31,30>),fn(OPRimm<20,12>),fn(OPRRnLU),fn(OPRRtL),NULL},3091203072,3219131392},
    {aarch64_op_ldr_imm_gen,"ldr",{fn(setRegWidth),fn(OPRsize<31,30>),fn(OPRimm<20,12>),fn(OPRRnLU),fn(OPRRtL),NULL},3091205120,3219131392},
    {aarch64_op_ldr_imm_gen,"ldr",{fn(setRegWidth),fn(OPRsize<31,30>),fn(OPRimm<21,10>),fn(OPRRnL),fn(OPRRtL),NULL},3107979264,3217031168},
    {aarch64_op_ldr_lit_fpsimd,"ldr",{fn(setRegWidth),fn(setSIMDMode),fn(OPRopc),fn(OPRimm<23,5>),fn(OPRRtL),NULL},469762048,1056964608},
    {aarch64_op_ldr_lit_gen,"ldr",{fn(setRegWidth),fn(OPRopc),fn(OPRimm<23,5>),fn(OPRRtL),NULL},402653184,3204448256},
    {aarch64_op_ldr_reg_fpsimd,"ldr",{fn(setRegWidth),fn(setSIMDMode),fn(OPRsize<31,30>),fn(OPRopc),fn(OPRRm),fn(OPRoption<15,13>),fn(OPRS<12,12>),fn(OPRRnL),fn(OPRRtL),NULL},1012926464,1063259136},
    {aarch64_op_ldr_reg_gen,"ldr",{fn(setRegWidth),fn(OPRsize<31,30>),fn(OPRRm),fn(OPRoption<15,13>),fn(OPRS<12,12>),fn(OPRRnL),fn(OPRRtL),NULL},3093301248,3219131392},
    {aarch64_op_ldrb_imm,"ldrb",{fn(setRegWidth),fn(OPRimm<20,12>),fn(OPRRnLU),fn(OPRRtL),NULL},943719424,4292873216},
    {aarch64_op_ldrb_imm,"ldrb",{fn(setRegWidth),fn(OPRimm<20,12>),fn(OPRRnLU),fn(OPRRtL),NULL},943721472,4292873216},
    {aarch64_op_ldrb_imm,"ldrb",{fn(setRegWidth),fn(OPRimm<21,10>),fn(OPRRnL),fn(OPRRtL),NULL},960495616,4290772992},
    {aarch64_op_ldrb_reg,"ldrb",{fn(setRegWidth),fn(OPRRm),fn(OPRoption<15,13>),fn(OPRS<12,12>),fn(OPRRnL),fn(OPRRtL),NULL},945817600,4292873216},
    {aarch64_op_ldrh_imm,"ldrh",{fn(setRegWidth),fn(OPRimm<20,12>),fn(OPRRnLU),fn(OPRRtL),NULL},2017461248,4292873216},
    {aarch64_op_ldrh_imm,"ldrh",{fn(setRegWidth),fn(OPRimm<20,12>),fn(OPRRnLU),fn(OPRRtL),NULL},2017463296,4292873216},
    {aarch64_op_ldrh_imm,"ldrh",{fn(setRegWidth),fn(OPRimm<21,10>),fn(OPRRnL),fn(OPRRtL),NULL},2034237440,4290772992},
    {aarch64_op_ldrh_reg,"ldrh",{fn(setRegWidth),fn(OPRRm),fn(OPRoption<15,13>),fn(OPRS<12,12>),fn(OPRRnL),fn(OPRRtL),NULL},2019559424,4292873216},
    {aarch64_op_ldrsb_imm,"ldrsb",{fn(setRegWidth),fn(OPRopc),fn(OPRimm<20,12>),fn(OPRRnLU),fn(OPRRtL),NULL},947913728,4288678912},
    {aarch64_op_ldrsb_imm,"ldrsb",{fn(setRegWidth),fn(OPRopc),fn(OPRimm<20,12>),fn(OPRRnLU),fn(OPRRtL),NULL},947915776,4288678912},
    {aarch64_op_ldrsb_imm,"ldrsb",{fn(setRegWidth),fn(OPRopc),fn(OPRimm<21,10>),fn(OPRRnL),fn(OPRRtL),NULL},964689920,4286578688},
    {aarch64_op_ldrsb_reg,"ldrsb",{fn(setRegWidth),fn(OPRopc),fn(OPRRm),fn(OPRoption<15,13>),fn(OPRS<12,12>),fn(OPRRnL),fn(OPRRtL),NULL},950011904,4288678912},
    {aarch64_op_ldrsh_imm,"ldrsh",{fn(setRegWidth),fn(OPRopc),fn(OPRimm<20,12>),fn(OPRRnLU),fn(OPRRtL),NULL},2021655552,4288678912},
    {aarch64_op_ldrsh_imm,"ldrsh",{fn(setRegWidth),fn(OPRopc),fn(OPRimm<20,12>),fn(OPRRnLU),fn(OPRRtL),NULL},2021657600,4288678912},
    {aarch64_op_ldrsh_imm,"ldrsh",{fn(setRegWidth),fn(OPRopc),fn(OPRimm<21,10>),fn(OPRRnL),fn(OPRRtL),NULL},2038431744,4286578688},
    {aarch64_op_ldrsh_reg,"ldrsh",{fn(setRegWidth),fn(OPRopc),fn(OPRRm),fn(OPRoption<15,13>),fn(OPRS<12,12>),fn(OPRRnL),fn(OPRRtL),NULL},2023753728,4288678912},
    {aarch64_op_ldrsw_imm,"ldrsw",{fn(setRegWidth),fn(OPRimm<20,12>),fn(OPRRnLU),fn(OPRRtL),NULL},3095397376,4292873216},
    {aarch64_op_ldrsw_imm,"ldrsw",{fn(setRegWidth),fn(OPRimm<20,12>),fn(OPRRnLU),fn(OPRRtL),NULL},3095399424,4292873216},
    {aarch64_op_ldrsw_imm,"ldrsw",{fn(setRegWidth),fn(OPRimm<21,10>),fn(OPRRnL),fn(OPRRtL),NULL},3112173568,4290772992},
    {aarch64_op_ldrsw_lit,"ldrsw",{fn(setRegWidth),fn(OPRimm<23,5>),fn(OPRRtL),NULL},2550136832,4278190080},
    {aarch64_op_ldrsw_reg,"ldrsw",{fn(setRegWidth),fn(OPRRm),fn(OPRoption<15,13>),fn(OPRS<12,12>),fn(OPRRnL),fn(OPRRtL),NULL},3097495552,4292873216},
    {aarch64_op_ldtr,"ldtr",{fn(setRegWidth),fn(OPRsize<31,30>),fn(OPRimm<20,12>),fn(OPRRnL),fn(OPRRtL),NULL},3091204096,3219131392},
    {aarch64_op_ldtrb,"ldtrb",{fn(setRegWidth),fn(OPRimm<20,12>),fn(OPRRnL),fn(OPRRtL),NULL},943720448,4292873216},
    {aarch64_op_ldtrh,"ldtrh",{fn(setRegWidth),fn(OPRimm<20,12>),fn(OPRRnL),fn(OPRRtL),NULL},2017462272,4292873216},
    {aarch64_op_ldtrsb,"ldtrsb",{fn(setRegWidth),fn(OPRopc),fn(OPRimm<20,12>),fn(OPRRnL),fn(OPRRtL),NULL},947914752,4288678912},
    {aarch64_op_ldtrsh,"ldtrsh",{fn(setRegWidth),fn(OPRopc),fn(OPRimm<20,12>),fn(OPRRnL),fn(OPRRtL),NULL},2021656576,4288678912},
    {aarch64_op_ldtrsw,"ldtrsw",{fn(setRegWidth),fn(OPRimm<20,12>),fn(OPRRnL),fn(OPRRtL),NULL},3095398400,4292873216},
    {aarch64_op_ldur_fpsimd,"ldur",{fn(setRegWidth),fn(setSIMDMode),fn(OPRsize<31,30>),fn(OPRopc),fn(OPRimm<20,12>),fn(OPRRnL),fn(OPRRtL),NULL},1010827264,1063259136},
    {aarch64_op_ldur_gen,"ldur",{fn(setRegWidth),fn(OPRsize<31,30>),fn(OPRimm<20,12>),fn(OPRRnL),fn(OPRRtL),NULL},3091202048,3219131392},
    {aarch64_op_ldurb,"ldurb",{fn(setRegWidth),fn(OPRimm<20,12>),fn(OPRRnL),fn(OPRRtL),NULL},943718400,4292873216},
    {aarch64_op_ldurh,"ldurh",{fn(setRegWidth),fn(OPRimm<20,12>),fn(OPRRnL),fn(OPRRtL),NULL},2017460224,4292873216},
    {aarch64_op_ldursb,"ldursb",{fn(setRegWidth),fn(OPRopc),fn(OPRimm<20,12>),fn(OPRRnL),fn(OPRRtL),NULL},947912704,4288678912},
    {aarch64_op_ldursh,"ldursh",{fn(setRegWidth),fn(OPRopc),fn(OPRimm<20,12>),fn(OPRRnL),fn(OPRRtL),NULL},2021654528,4288678912},
    {aarch64_op_ldursw,"ldursw",{fn(setRegWidth),fn(OPRimm<20,12>),fn(OPRRnL),fn(OPRRtL),NULL},3095396352,4292873216},
    {aarch64_op_ldxp,"ldxp",{fn(setRegWidth),fn(OPRsz<30,30>),fn(OPRRt2L),fn(OPRRnL),fn(OPRRtL),NULL},2290024448,3221192704},
    {aarch64_op_ldxr,"ldxr",{fn(setRegWidth),fn(OPRsize<31,30>),fn(OPRRnL),fn(OPRRtL),NULL},2287959040,3221224448},
    {aarch64_op_ldxrb,"ldxrb",{fn(setRegWidth),fn(OPRRnL),fn(OPRRtL),NULL},140475392,4294966272},
    {aarch64_op_ldxrh,"ldxrh",{fn(setRegWidth),fn(OPRRnL),fn(OPRRtL),NULL},1214217216,4294966272},
    {aarch64_op_lsl_lslv,"lsl",{fn(OPRsf),fn(OPRRm),fn(OPRRn),fn(OPRRd),NULL},448798720,2145451008},
    {aarch64_op_lsl_ubfm,"lsl",{fn(OPRsf),fn(OPRN<22,22>),fn(OPRimm<21,16>),fn(OPRimm<15,10>),fn(OPRRn),fn(OPRRd),NULL},1392508928,2139095040},
    {aarch64_op_lslv,"lslv",{fn(OPRsf),fn(OPRRm),fn(OPRRn),fn(OPRRd),NULL},448798720,2145451008},
    {aarch64_op_lsr_lsrv,"lsr",{fn(OPRsf),fn(OPRRm),fn(OPRRn),fn(OPRRd),NULL},448799744,2145451008},
    {aarch64_op_lsr_ubfm,"lsr",{fn(OPRsf),fn(OPRN<22,22>),fn(OPRimm<21,16>),fn(OPRimm<15,10>),fn(OPRRn),fn(OPRRd),NULL},1392540672,2139126784},
    {aarch64_op_lsrv,"lsrv",{fn(OPRsf),fn(OPRRm),fn(OPRRn),fn(OPRRd),NULL},448799744,2145451008},
    {aarch64_op_madd,"madd",{fn(OPRsf),fn(OPRRm),fn(OPRRa),fn(OPRRn),fn(OPRRd),NULL},452984832,2145419264},
    {aarch64_op_mla_advsimd_elt,"mla",{fn(setSIMDMode),fn(OPRQ),fn(OPRsize<23,22>),fn(OPRL),fn(OPRM),fn(OPRRm),fn(OPRH),fn(OPRRn),fn(OPRRd),NULL},788529152,3204510720},
    {aarch64_op_mla_advsimd_vec,"mla",{fn(setSIMDMode),fn(OPRQ),fn(OPRsize<23,22>),fn(OPRRm),fn(OPRRn),fn(OPRRd),NULL},237016064,3206609920},
    {aarch64_op_mls_advsimd_elt,"mls",{fn(setSIMDMode),fn(OPRQ),fn(OPRsize<23,22>),fn(OPRL),fn(OPRM),fn(OPRRm),fn(OPRH),fn(OPRRn),fn(OPRRd),NULL},788545536,3204510720},
    {aarch64_op_mls_advsimd_vec,"mls",{fn(setSIMDMode),fn(OPRQ),fn(OPRsize<23,22>),fn(OPRRm),fn(OPRRn),fn(OPRRd),NULL},773886976,3206609920},
    {aarch64_op_mneg_msub,"mneg",{fn(OPRsf),fn(OPRRm),fn(OPRRn),fn(OPRRd),NULL},453049344,2145451008},
    {aarch64_op_mov_add_addsub_imm,"mov",{fn(OPRsf),fn(OPRRn),fn(OPRRd),NULL},285212672,2147482624},
    {aarch64_op_mov_dup_advsimd_elt,"mov",{fn(setSIMDMode),fn(OPRimm<20,16>),fn(OPRRn),fn(OPRRd),NULL},1577059328,4292934656},
    {aarch64_op_mov_ins_advsimd_elt,"mov",{fn(setSIMDMode),fn(OPRimm<20,16>),fn(OPRimm<14,11>),fn(OPRRn),fn(OPRRd),NULL},1845494784,4292903936},
    {aarch64_op_mov_ins_advsimd_gen,"mov",{fn(setSIMDMode),fn(OPRimm<20,16>),fn(OPRRn),fn(OPRRd),NULL},1308630016,4292934656},
    {aarch64_op_mov_movn,"mov",{fn(OPRsf),fn(OPRhw),fn(OPRimm<20,5>),fn(OPRRd),NULL},310378496,2139095040},
    {aarch64_op_mov_movz,"mov",{fn(OPRsf),fn(OPRhw),fn(OPRimm<20,5>),fn(OPRRd),NULL},1384120320,2139095040},
    {aarch64_op_mov_orr_advsimd_reg,"mov",{fn(setSIMDMode),fn(OPRQ),fn(OPRRm),fn(OPRRn),fn(OPRRd),NULL},245373952,3219192832},
    {aarch64_op_mov_orr_log_imm,"mov",{fn(OPRsf),fn(OPRN<22,22>),fn(OPRimm<21,16>),fn(OPRimm<15,10>),fn(OPRRd),NULL},838861792,2139096032},
    {aarch64_op_mov_orr_log_shift,"mov",{fn(OPRsf),fn(OPRRm),fn(OPRRd),NULL},704644064,2145452000},
    {aarch64_op_mov_umov_advsimd,"mov",{fn(setSIMDMode),fn(OPRQ),fn(OPRimm<20,16>),fn(OPRRn),fn(OPRRd),NULL},234896384,3219192832},
    {aarch64_op_movi_advsimd,"movi",{fn(setSIMDMode),fn(OPRQ),fn(OPRop),fn(OPRa),fn(OPRb),fn(OPRc),fn(OPRcmode),fn(OPRd),fn(OPRe),fn(OPRf),fn(OPRg),fn(OPRh),fn(OPRRd),NULL},251659264,2683833344},
    {aarch64_op_movk,"movk",{fn(OPRsf),fn(OPRhw),fn(OPRimm<20,5>),fn(OPRRd),NULL},1920991232,2139095040},
    {aarch64_op_movn,"movn",{fn(OPRsf),fn(OPRhw),fn(OPRimm<20,5>),fn(OPRRd),NULL},310378496,2139095040},
    {aarch64_op_movz,"movz",{fn(OPRsf),fn(OPRhw),fn(OPRimm<20,5>),fn(OPRRd),NULL},1384120320,2139095040},
    {aarch64_op_mrs,"mrs",{fn(OPRo0),fn(OPRop1),fn(OPRCRn),fn(OPRCRm),fn(OPRop2),fn(OPRRt),NULL},3576692736,4293918720},
    {aarch64_op_msr_imm,"msr",{fn(OPRop1),fn(OPRCRm),fn(OPRop2),NULL},3573563423,4294504479},
    {aarch64_op_msr_reg,"msr",{fn(OPRo0),fn(OPRop1),fn(OPRCRn),fn(OPRCRm),fn(OPRop2),fn(OPRRt),NULL},3574595584,4293918720},
    {aarch64_op_msub,"msub",{fn(OPRsf),fn(OPRRm),fn(OPRRa),fn(OPRRn),fn(OPRRd),NULL},453017600,2145419264},
    {aarch64_op_mul_advsimd_elt,"mul",{fn(setSIMDMode),fn(OPRQ),fn(OPRsize<23,22>),fn(OPRL),fn(OPRM),fn(OPRRm),fn(OPRH),fn(OPRRn),fn(OPRRd),NULL},251691008,3204510720},
    {aarch64_op_mul_advsimd_vec,"mul",{fn(setSIMDMode),fn(OPRQ),fn(OPRsize<23,22>),fn(OPRRm),fn(OPRRn),fn(OPRRd),NULL},237018112,3206609920},
    {aarch64_op_mul_madd,"mul",{fn(OPRsf),fn(OPRRm),fn(OPRRn),fn(OPRRd),NULL},453016576,2145451008},
    {aarch64_op_mvn_not_advsimd,"mvn",{fn(setSIMDMode),fn(OPRQ),fn(OPRRn),fn(OPRRd),NULL},773871616,3221224448},
    {aarch64_op_mvn_orn_log_shift,"mvn",{fn(OPRsf),fn(OPRshift),fn(OPRRm),fn(OPRimm<15,10>),fn(OPRRd),NULL},706741216,2132804576},
    {aarch64_op_mvni_advsimd,"mvni",{fn(setSIMDMode),fn(OPRQ),fn(OPRa),fn(OPRb),fn(OPRc),fn(OPRcmode),fn(OPRd),fn(OPRe),fn(OPRf),fn(OPRg),fn(OPRh),fn(OPRRd),NULL},788530176,3220704256},
    {aarch64_op_neg_advsimd,"neg",{fn(setSIMDMode),fn(OPRsize<23,22>),fn(OPRRn),fn(OPRRd),NULL},2116073472,4282383360},
    {aarch64_op_neg_advsimd,"neg",{fn(setSIMDMode),fn(OPRQ),fn(OPRsize<23,22>),fn(OPRRn),fn(OPRRd),NULL},773896192,3208641536},
    {aarch64_op_neg_sub_addsub_shift,"neg",{fn(OPRsf),fn(OPRshift),fn(OPRRm),fn(OPRimm<15,10>),fn(OPRRd),NULL},1258292192,2132804576},
    {aarch64_op_negs_subs_addsub_shift,"negs",{fn(setFlags),fn(OPRsf),fn(OPRshift),fn(OPRRm),fn(OPRimm<15,10>),fn(OPRRd),NULL},1795163104,2132804576},
    {aarch64_op_ngc_sbc,"ngc",{fn(OPRsf),fn(OPRRm),fn(OPRRd),NULL},1509950432,2145452000},
    {aarch64_op_ngcs_sbcs,"ngcs",{fn(setFlags),fn(OPRsf),fn(OPRRm),fn(OPRRd),NULL},2046821344,2145452000},
    {aarch64_op_nop_hint,"nop",{NULL},3573751839,4294967295},
    {aarch64_op_not_advsimd,"not",{fn(setSIMDMode),fn(OPRQ),fn(OPRRn),fn(OPRRd),NULL},773871616,3221224448},
    {aarch64_op_orn_advsimd,"orn",{fn(setSIMDMode),fn(OPRQ),fn(OPRRm),fn(OPRRn),fn(OPRRd),NULL},249568256,3219192832},
    {aarch64_op_orn_log_shift,"orn",{fn(OPRsf),fn(OPRshift),fn(OPRRm),fn(OPRimm<15,10>),fn(OPRRn),fn(OPRRd),NULL},706740224,2132803584},
    {aarch64_op_orr_advsimd_imm,"orr",{fn(setSIMDMode),fn(OPRQ),fn(OPRa),fn(OPRb),fn(OPRc),fn(OPRcmode),fn(OPRd),fn(OPRe),fn(OPRf),fn(OPRg),fn(OPRh),fn(OPRRd),NULL},251663360,3220708352},
    {aarch64_op_orr_advsimd_reg,"orr",{fn(setSIMDMode),fn(OPRQ),fn(OPRRm),fn(OPRRn),fn(OPRRd),NULL},245373952,3219192832},
    {aarch64_op_orr_log_imm,"orr",{fn(OPRsf),fn(OPRN<22,22>),fn(OPRimm<21,16>),fn(OPRimm<15,10>),fn(OPRRn),fn(OPRRd),NULL},838860800,2139095040},
    {aarch64_op_orr_log_shift,"orr",{fn(OPRsf),fn(OPRshift),fn(OPRRm),fn(OPRimm<15,10>),fn(OPRRn),fn(OPRRd),NULL},704643072,2132803584},
    {aarch64_op_pmul_advsimd,"pmul",{fn(setSIMDMode),fn(OPRQ),fn(OPRsize<23,22>),fn(OPRRm),fn(OPRRn),fn(OPRRd),NULL},773889024,3206609920},
    {aarch64_op_pmull_advsimd,"pmull",{fn(setSIMDMode),fn(OPRQ),fn(OPRsize<23,22>),fn(OPRRm),fn(OPRRn),fn(OPRRd),NULL},237035520,3206609920},
    {aarch64_op_prfm_imm,"prfm",{fn(OPRimm<21,10>),fn(OPRRnL),fn(OPRRt),NULL},4185915392,4290772992},
    {aarch64_op_prfm_lit,"prfm",{fn(OPRimm<23,5>),fn(OPRRt),NULL},3623878656,4278190080},
    {aarch64_op_prfm_reg,"prfm",{fn(OPRRm),fn(OPRoption<15,13>),fn(OPRS<12,12>),fn(OPRRnL),fn(OPRRt),NULL},4171237376,4292873216},
    {aarch64_op_prfum,"prfum",{fn(OPRimm<20,12>),fn(OPRRnL),fn(OPRRt),NULL},4169138176,4292873216},
    {aarch64_op_raddhn_advsimd,"raddhn",{fn(setSIMDMode),fn(OPRQ),fn(OPRsize<23,22>),fn(OPRRm),fn(OPRRn),fn(OPRRd),NULL},773865472,3206609920},
    {aarch64_op_rbit_advsimd,"rbit",{fn(setSIMDMode),fn(OPRQ),fn(OPRRn),fn(OPRRd),NULL},778065920,3221224448},
    {aarch64_op_rbit_int,"rbit",{fn(OPRsf),fn(OPRRn),fn(OPRRd),NULL},1522532352,2147482624},
    {aarch64_op_ret,"ret",{fn(OPRRn),NULL},3596550144,4294966303},
    {aarch64_op_rev,"rev",{fn(OPRsf),fn(OPRopc),fn(OPRRn),fn(OPRRd),NULL},1522534400,2147481600},
    {aarch64_op_rev16_advsimd,"rev16",{fn(setSIMDMode),fn(OPRQ),fn(OPRsize<23,22>),fn(OPRRn),fn(OPRRd),NULL},236984320,3208641536},
    {aarch64_op_rev16_int,"rev16",{fn(OPRsf),fn(OPRRn),fn(OPRRd),NULL},1522533376,2147482624},
    {aarch64_op_rev32_advsimd,"rev32",{fn(setSIMDMode),fn(OPRQ),fn(OPRsize<23,22>),fn(OPRRn),fn(OPRRd),NULL},773851136,3208641536},
    {aarch64_op_rev32_int,"rev32",{fn(OPRRn),fn(OPRRd),NULL},3670018048,4294966272},
    {aarch64_op_rev64_advsimd,"rev64",{fn(setSIMDMode),fn(OPRQ),fn(OPRsize<23,22>),fn(OPRRn),fn(OPRRd),NULL},236980224,3208641536},
    {aarch64_op_ror_extr,"ror",{fn(OPRsf),fn(OPRN<22,22>),fn(OPRRm),fn(OPRimm<15,10>),fn(OPRRn),fn(OPRRd),NULL},327155712,2141192192},
    {aarch64_op_ror_rorv,"ror",{fn(OPRsf),fn(OPRRm),fn(OPRRn),fn(OPRRd),NULL},448801792,2145451008},
    {aarch64_op_rorv,"rorv",{fn(OPRsf),fn(OPRRm),fn(OPRRn),fn(OPRRd),NULL},448801792,2145451008},
    {aarch64_op_rshrn_advsimd,"rshrn",{fn(setSIMDMode),fn(OPRQ),fn(OPRimm<22,19>),fn(OPRimm<18,16>),fn(OPRRn),fn(OPRRd),NULL},251694080,3212901376},
    {aarch64_op_rsubhn_advsimd,"rsubhn",{fn(setSIMDMode),fn(OPRQ),fn(OPRsize<23,22>),fn(OPRRm),fn(OPRRn),fn(OPRRd),NULL},773873664,3206609920},
    {aarch64_op_saba_advsimd,"saba",{fn(setSIMDMode),fn(OPRQ),fn(OPRsize<23,22>),fn(OPRRm),fn(OPRRn),fn(OPRRd),NULL},237009920,3206609920},
    {aarch64_op_sabal_advsimd,"sabal",{fn(setSIMDMode),fn(OPRQ),fn(OPRsize<23,22>),fn(OPRRm),fn(OPRRn),fn(OPRRd),NULL},236998656,3206609920},
    {aarch64_op_sabd_advsimd,"sabd",{fn(setSIMDMode),fn(OPRQ),fn(OPRsize<23,22>),fn(OPRRm),fn(OPRRn),fn(OPRRd),NULL},237007872,3206609920},
    {aarch64_op_sabdl_advsimd,"sabdl",{fn(setSIMDMode),fn(OPRQ),fn(OPRsize<23,22>),fn(OPRRm),fn(OPRRn),fn(OPRRd),NULL},237006848,3206609920},
    {aarch64_op_sadalp_advsimd,"sadalp",{fn(setSIMDMode),fn(OPRQ),fn(OPRsize<23,22>),fn(OPRRn),fn(OPRRd),NULL},237004800,3208641536},
    {aarch64_op_saddl_advsimd,"saddl",{fn(setSIMDMode),fn(OPRQ),fn(OPRsize<23,22>),fn(OPRRm),fn(OPRRn),fn(OPRRd),NULL},236978176,3206609920},
    {aarch64_op_saddlp_advsimd,"saddlp",{fn(setSIMDMode),fn(OPRQ),fn(OPRsize<23,22>),fn(OPRRn),fn(OPRRd),NULL},236988416,3208641536},
    {aarch64_op_saddlv_advsimd,"saddlv",{fn(setSIMDMode),fn(OPRQ),fn(OPRsize<23,22>),fn(OPRRn),fn(OPRRd),NULL},238041088,3208641536},
    {aarch64_op_saddw_advsimd,"saddw",{fn(setSIMDMode),fn(OPRQ),fn(OPRsize<23,22>),fn(OPRRm),fn(OPRRn),fn(OPRRd),NULL},236982272,3206609920},
    {aarch64_op_sbc,"sbc",{fn(OPRsf),fn(OPRRm),fn(OPRRn),fn(OPRRd),NULL},1509949440,2145451008},
    {aarch64_op_sbcs,"sbcs",{fn(setFlags),fn(OPRsf),fn(OPRRm),fn(OPRRn),fn(OPRRd),NULL},2046820352,2145451008},
    {aarch64_op_sbfiz_sbfm,"sbfiz",{fn(OPRsf),fn(OPRN<22,22>),fn(OPRimm<21,16>),fn(OPRimm<15,10>),fn(OPRRn),fn(OPRRd),NULL},318767104,2139095040},
    {aarch64_op_sbfm,"sbfm",{fn(OPRsf),fn(OPRN<22,22>),fn(OPRimm<21,16>),fn(OPRimm<15,10>),fn(OPRRn),fn(OPRRd),NULL},318767104,2139095040},
    {aarch64_op_sbfx_sbfm,"sbfx",{fn(OPRsf),fn(OPRN<22,22>),fn(OPRimm<21,16>),fn(OPRimm<15,10>),fn(OPRRn),fn(OPRRd),NULL},318767104,2139095040},
    {aarch64_op_scvtf_advsimd_fix,"scvtf",{fn(setSIMDMode),fn(OPRimm<22,19>),fn(OPRimm<18,16>),fn(OPRRn),fn(OPRRd),NULL},1593893888,4286643200},
    {aarch64_op_scvtf_advsimd_fix,"scvtf",{fn(setSIMDMode),fn(OPRQ),fn(OPRimm<22,19>),fn(OPRimm<18,16>),fn(OPRRn),fn(OPRRd),NULL},251716608,3212901376},
    {aarch64_op_scvtf_advsimd_int,"scvtf",{fn(setSIMDMode),fn(OPRsz<22,22>),fn(OPRRn),fn(OPRRd),NULL},1579276288,4290771968},
    {aarch64_op_scvtf_advsimd_int,"scvtf",{fn(setSIMDMode),fn(OPRQ),fn(OPRsz<22,22>),fn(OPRRn),fn(OPRRd),NULL},237099008,3217030144},
    {aarch64_op_scvtf_float_fix,"scvtf",{fn(setFPMode),fn(OPRsf),fn(OPRtype<23,22>),fn(OPRscale),fn(OPRRn),fn(OPRRd),NULL},503447552,2143223808},
    {aarch64_op_scvtf_float_int,"scvtf",{fn(setFPMode),fn(OPRsf),fn(OPRtype<23,22>),fn(OPRRn),fn(OPRRd),NULL},505544704,2143288320},
    {aarch64_op_sdiv,"sdiv",{fn(OPRsf),fn(OPRRm),fn(OPRRn),fn(OPRRd),NULL},448793600,2145451008},
    {aarch64_op_sev_hint,"sev",{NULL},3573751967,4294967295},
    {aarch64_op_sevl_hint,"sevl",{NULL},3573751999,4294967295},
    {aarch64_op_sha1c_advsimd,"sha1c",{fn(setSIMDMode),fn(OPRRm),fn(OPRRn),fn(OPRRd),NULL},1577058304,4292934656},
    {aarch64_op_sha1h_advsimd,"sha1h",{fn(setSIMDMode),fn(OPRRn),fn(OPRRd),NULL},1579681792,4294966272},
    {aarch64_op_sha1m_advsimd,"sha1m",{fn(setSIMDMode),fn(OPRRm),fn(OPRRn),fn(OPRRd),NULL},1577066496,4292934656},
    {aarch64_op_sha1p_advsimd,"sha1p",{fn(setSIMDMode),fn(OPRRm),fn(OPRRn),fn(OPRRd),NULL},1577062400,4292934656},
    {aarch64_op_sha1su0_advsimd,"sha1su0",{fn(setSIMDMode),fn(OPRRm),fn(OPRRn),fn(OPRRd),NULL},1577070592,4292934656},
    {aarch64_op_sha1su1_advsimd,"sha1su1",{fn(setSIMDMode),fn(OPRRn),fn(OPRRd),NULL},1579685888,4294966272},
    {aarch64_op_sha256h2_advsimd,"sha256h2",{fn(setSIMDMode),fn(OPRRm),fn(OPRRn),fn(OPRRd),NULL},1577078784,4292934656},
    {aarch64_op_sha256h_advsimd,"sha256h",{fn(setSIMDMode),fn(OPRRm),fn(OPRRn),fn(OPRRd),NULL},1577074688,4292934656},
    {aarch64_op_sha256su0_advsimd,"sha256su0",{fn(setSIMDMode),fn(OPRRn),fn(OPRRd),NULL},1579689984,4294966272},
    {aarch64_op_sha256su1_advsimd,"sha256su1",{fn(setSIMDMode),fn(OPRRm),fn(OPRRn),fn(OPRRd),NULL},1577082880,4292934656},
    {aarch64_op_shadd_advsimd,"shadd",{fn(setSIMDMode),fn(OPRQ),fn(OPRsize<23,22>),fn(OPRRm),fn(OPRRn),fn(OPRRd),NULL},236979200,3206609920},
    {aarch64_op_shl_advsimd,"shl",{fn(setSIMDMode),fn(OPRimm<22,19>),fn(OPRimm<18,16>),fn(OPRRn),fn(OPRRd),NULL},1593857024,4286643200},
    {aarch64_op_shl_advsimd,"shl",{fn(setSIMDMode),fn(OPRQ),fn(OPRimm<22,19>),fn(OPRimm<18,16>),fn(OPRRn),fn(OPRRd),NULL},251679744,3212901376},
    {aarch64_op_shll_advsimd,"shll",{fn(setSIMDMode),fn(OPRQ),fn(OPRsize<23,22>),fn(OPRRn),fn(OPRRd),NULL},773928960,3208641536},
    {aarch64_op_shrn_advsimd,"shrn",{fn(setSIMDMode),fn(OPRQ),fn(OPRimm<22,19>),fn(OPRimm<18,16>),fn(OPRRn),fn(OPRRd),NULL},251692032,3212901376},
    {aarch64_op_shsub_advsimd,"shsub",{fn(setSIMDMode),fn(OPRQ),fn(OPRsize<23,22>),fn(OPRRm),fn(OPRRn),fn(OPRRd),NULL},236987392,3206609920},
    {aarch64_op_sli_advsimd,"sli",{fn(setSIMDMode),fn(OPRimm<22,19>),fn(OPRimm<18,16>),fn(OPRRn),fn(OPRRd),NULL},2130727936,4286643200},
    {aarch64_op_sli_advsimd,"sli",{fn(setSIMDMode),fn(OPRQ),fn(OPRimm<22,19>),fn(OPRimm<18,16>),fn(OPRRn),fn(OPRRd),NULL},788550656,3212901376},
    {aarch64_op_smaddl,"smaddl",{fn(OPRRm),fn(OPRRa),fn(OPRRn),fn(OPRRd),NULL},2602565632,4292902912},
    {aarch64_op_smax_advsimd,"smax",{fn(setSIMDMode),fn(OPRQ),fn(OPRsize<23,22>),fn(OPRRm),fn(OPRRn),fn(OPRRd),NULL},237003776,3206609920},
    {aarch64_op_smaxp_advsimd,"smaxp",{fn(setSIMDMode),fn(OPRQ),fn(OPRsize<23,22>),fn(OPRRm),fn(OPRRn),fn(OPRRd),NULL},237020160,3206609920},
    {aarch64_op_smaxv_advsimd,"smaxv",{fn(setSIMDMode),fn(OPRQ),fn(OPRsize<23,22>),fn(OPRRn),fn(OPRRd),NULL},238069760,3208641536},
    {aarch64_op_smc,"smc",{fn(OPRimm<20,5>),NULL},3556769795,4292870175},
    {aarch64_op_smin_advsimd,"smin",{fn(setSIMDMode),fn(OPRQ),fn(OPRsize<23,22>),fn(OPRRm),fn(OPRRn),fn(OPRRd),NULL},237005824,3206609920},
    {aarch64_op_sminp_advsimd,"sminp",{fn(setSIMDMode),fn(OPRQ),fn(OPRsize<23,22>),fn(OPRRm),fn(OPRRn),fn(OPRRd),NULL},237022208,3206609920},
    {aarch64_op_sminv_advsimd,"sminv",{fn(setSIMDMode),fn(OPRQ),fn(OPRsize<23,22>),fn(OPRRn),fn(OPRRd),NULL},238135296,3208641536},
    {aarch64_op_smlal_advsimd_elt,"smlal",{fn(setSIMDMode),fn(OPRQ),fn(OPRsize<23,22>),fn(OPRL),fn(OPRM),fn(OPRRm),fn(OPRH),fn(OPRRn),fn(OPRRd),NULL},251666432,3204510720},
    {aarch64_op_smlal_advsimd_vec,"smlal",{fn(setSIMDMode),fn(OPRQ),fn(OPRsize<23,22>),fn(OPRRm),fn(OPRRn),fn(OPRRd),NULL},237010944,3206609920},
    {aarch64_op_smlsl_advsimd_elt,"smlsl",{fn(setSIMDMode),fn(OPRQ),fn(OPRsize<23,22>),fn(OPRL),fn(OPRM),fn(OPRRm),fn(OPRH),fn(OPRRn),fn(OPRRd),NULL},251682816,3204510720},
    {aarch64_op_smlsl_advsimd_vec,"smlsl",{fn(setSIMDMode),fn(OPRQ),fn(OPRsize<23,22>),fn(OPRRm),fn(OPRRn),fn(OPRRd),NULL},237019136,3206609920},
    {aarch64_op_smnegl_smsubl,"smnegl",{fn(OPRRm),fn(OPRRn),fn(OPRRd),NULL},2602630144,4292934656},
    {aarch64_op_smov_advsimd,"smov",{fn(setSIMDMode),fn(OPRQ),fn(OPRimm<20,16>),fn(OPRRn),fn(OPRRd),NULL},234892288,3219192832},
    {aarch64_op_smsubl,"smsubl",{fn(OPRRm),fn(OPRRa),fn(OPRRn),fn(OPRRd),NULL},2602598400,4292902912},
    {aarch64_op_smulh,"smulh",{fn(OPRRm),fn(OPRRn),fn(OPRRd),NULL},2604694528,4292934656},
    {aarch64_op_smull_advsimd_elt,"smull",{fn(setSIMDMode),fn(OPRQ),fn(OPRsize<23,22>),fn(OPRL),fn(OPRM),fn(OPRRm),fn(OPRH),fn(OPRRn),fn(OPRRd),NULL},251699200,3204510720},
    {aarch64_op_smull_advsimd_vec,"smull",{fn(setSIMDMode),fn(OPRQ),fn(OPRsize<23,22>),fn(OPRRm),fn(OPRRn),fn(OPRRd),NULL},237027328,3206609920},
    {aarch64_op_smull_smaddl,"smull",{fn(OPRRm),fn(OPRRn),fn(OPRRd),NULL},2602597376,4292934656},
    {aarch64_op_sqabs_advsimd,"sqabs",{fn(setSIMDMode),fn(OPRsize<23,22>),fn(OPRRn),fn(OPRRd),NULL},1579186176,4282383360},
    {aarch64_op_sqabs_advsimd,"sqabs",{fn(setSIMDMode),fn(OPRQ),fn(OPRsize<23,22>),fn(OPRRn),fn(OPRRd),NULL},237008896,3208641536},
    {aarch64_op_sqadd_advsimd,"sqadd",{fn(setSIMDMode),fn(OPRsize<23,22>),fn(OPRRm),fn(OPRRn),fn(OPRRd),NULL},1579158528,4280351744},
    {aarch64_op_sqadd_advsimd,"sqadd",{fn(setSIMDMode),fn(OPRQ),fn(OPRsize<23,22>),fn(OPRRm),fn(OPRRn),fn(OPRRd),NULL},236981248,3206609920},
    {aarch64_op_sqdmlal_advsimd_elt,"sqdmlal",{fn(setSIMDMode),fn(OPRsize<23,22>),fn(OPRL),fn(OPRM),fn(OPRRm),fn(OPRH),fn(OPRRn),fn(OPRRd),NULL},1593847808,4278252544},
    {aarch64_op_sqdmlal_advsimd_elt,"sqdmlal",{fn(setSIMDMode),fn(OPRQ),fn(OPRsize<23,22>),fn(OPRL),fn(OPRM),fn(OPRRm),fn(OPRH),fn(OPRRn),fn(OPRRd),NULL},251670528,3204510720},
    {aarch64_op_sqdmlal_advsimd_vec,"sqdmlal",{fn(setSIMDMode),fn(OPRsize<23,22>),fn(OPRRm),fn(OPRRn),fn(OPRRd),NULL},1579192320,4280351744},
    {aarch64_op_sqdmlal_advsimd_vec,"sqdmlal",{fn(setSIMDMode),fn(OPRQ),fn(OPRsize<23,22>),fn(OPRRm),fn(OPRRn),fn(OPRRd),NULL},237015040,3206609920},
    {aarch64_op_sqdmlsl_advsimd_elt,"sqdmlsl",{fn(setSIMDMode),fn(OPRsize<23,22>),fn(OPRL),fn(OPRM),fn(OPRRm),fn(OPRH),fn(OPRRn),fn(OPRRd),NULL},1593864192,4278252544},
    {aarch64_op_sqdmlsl_advsimd_elt,"sqdmlsl",{fn(setSIMDMode),fn(OPRQ),fn(OPRsize<23,22>),fn(OPRL),fn(OPRM),fn(OPRRm),fn(OPRH),fn(OPRRn),fn(OPRRd),NULL},251686912,3204510720},
    {aarch64_op_sqdmlsl_advsimd_vec,"sqdmlsl",{fn(setSIMDMode),fn(OPRsize<23,22>),fn(OPRRm),fn(OPRRn),fn(OPRRd),NULL},1579200512,4280351744},
    {aarch64_op_sqdmlsl_advsimd_vec,"sqdmlsl",{fn(setSIMDMode),fn(OPRQ),fn(OPRsize<23,22>),fn(OPRRm),fn(OPRRn),fn(OPRRd),NULL},237023232,3206609920},
    {aarch64_op_sqdmulh_advsimd_elt,"sqdmulh",{fn(setSIMDMode),fn(OPRsize<23,22>),fn(OPRL),fn(OPRM),fn(OPRRm),fn(OPRH),fn(OPRRn),fn(OPRRd),NULL},1593884672,4278252544},
    {aarch64_op_sqdmulh_advsimd_elt,"sqdmulh",{fn(setSIMDMode),fn(OPRQ),fn(OPRsize<23,22>),fn(OPRL),fn(OPRM),fn(OPRRm),fn(OPRH),fn(OPRRn),fn(OPRRd),NULL},251707392,3204510720},
    {aarch64_op_sqdmulh_advsimd_vec,"sqdmulh",{fn(setSIMDMode),fn(OPRsize<23,22>),fn(OPRRm),fn(OPRRn),fn(OPRRd),NULL},1579201536,4280351744},
    {aarch64_op_sqdmulh_advsimd_vec,"sqdmulh",{fn(setSIMDMode),fn(OPRQ),fn(OPRsize<23,22>),fn(OPRRm),fn(OPRRn),fn(OPRRd),NULL},237024256,3206609920},
    {aarch64_op_sqdmull_advsimd_elt,"sqdmull",{fn(setSIMDMode),fn(OPRsize<23,22>),fn(OPRL),fn(OPRM),fn(OPRRm),fn(OPRH),fn(OPRRn),fn(OPRRd),NULL},1593880576,4278252544},
    {aarch64_op_sqdmull_advsimd_elt,"sqdmull",{fn(setSIMDMode),fn(OPRQ),fn(OPRsize<23,22>),fn(OPRL),fn(OPRM),fn(OPRRm),fn(OPRH),fn(OPRRn),fn(OPRRd),NULL},251703296,3204510720},
    {aarch64_op_sqdmull_advsimd_vec,"sqdmull",{fn(setSIMDMode),fn(OPRsize<23,22>),fn(OPRRm),fn(OPRRn),fn(OPRRd),NULL},1579208704,4280351744},
    {aarch64_op_sqdmull_advsimd_vec,"sqdmull",{fn(setSIMDMode),fn(OPRQ),fn(OPRsize<23,22>),fn(OPRRm),fn(OPRRn),fn(OPRRd),NULL},237031424,3206609920},
    {aarch64_op_sqneg_advsimd,"sqneg",{fn(setSIMDMode),fn(OPRsize<23,22>),fn(OPRRn),fn(OPRRd),NULL},2116057088,4282383360},
    {aarch64_op_sqneg_advsimd,"sqneg",{fn(setSIMDMode),fn(OPRQ),fn(OPRsize<23,22>),fn(OPRRn),fn(OPRRd),NULL},773879808,3208641536},
    {aarch64_op_sqrdmulh_advsimd_elt,"sqrdmulh",{fn(setSIMDMode),fn(OPRsize<23,22>),fn(OPRL),fn(OPRM),fn(OPRRm),fn(OPRH),fn(OPRRn),fn(OPRRd),NULL},1593888768,4278252544},
    {aarch64_op_sqrdmulh_advsimd_elt,"sqrdmulh",{fn(setSIMDMode),fn(OPRQ),fn(OPRsize<23,22>),fn(OPRL),fn(OPRM),fn(OPRRm),fn(OPRH),fn(OPRRn),fn(OPRRd),NULL},251711488,3204510720},
    {aarch64_op_sqrdmulh_advsimd_vec,"sqrdmulh",{fn(setSIMDMode),fn(OPRsize<23,22>),fn(OPRRm),fn(OPRRn),fn(OPRRd),NULL},2116072448,4280351744},
    {aarch64_op_sqrdmulh_advsimd_vec,"sqrdmulh",{fn(setSIMDMode),fn(OPRQ),fn(OPRsize<23,22>),fn(OPRRm),fn(OPRRn),fn(OPRRd),NULL},773895168,3206609920},
    {aarch64_op_sqrshl_advsimd,"sqrshl",{fn(setSIMDMode),fn(OPRsize<23,22>),fn(OPRRm),fn(OPRRn),fn(OPRRd),NULL},1579179008,4280351744},
    {aarch64_op_sqrshl_advsimd,"sqrshl",{fn(setSIMDMode),fn(OPRQ),fn(OPRsize<23,22>),fn(OPRRm),fn(OPRRn),fn(OPRRd),NULL},237001728,3206609920},
    {aarch64_op_sqrshrn_advsimd,"sqrshrn",{fn(setSIMDMode),fn(OPRimm<22,19>),fn(OPRimm<18,16>),fn(OPRRn),fn(OPRRd),NULL},1593875456,4286643200},
    {aarch64_op_sqrshrn_advsimd,"sqrshrn",{fn(setSIMDMode),fn(OPRQ),fn(OPRimm<22,19>),fn(OPRimm<18,16>),fn(OPRRn),fn(OPRRd),NULL},251698176,3212901376},
    {aarch64_op_sqrshrun_advsimd,"sqrshrun",{fn(setSIMDMode),fn(OPRimm<22,19>),fn(OPRimm<18,16>),fn(OPRRn),fn(OPRRd),NULL},2130742272,4286643200},
    {aarch64_op_sqrshrun_advsimd,"sqrshrun",{fn(setSIMDMode),fn(OPRQ),fn(OPRimm<22,19>),fn(OPRimm<18,16>),fn(OPRRn),fn(OPRRd),NULL},788564992,3212901376},
    {aarch64_op_sqshl_advsimd_imm,"sqshl",{fn(setSIMDMode),fn(OPRimm<22,19>),fn(OPRimm<18,16>),fn(OPRRn),fn(OPRRd),NULL},1593865216,4286643200},
    {aarch64_op_sqshl_advsimd_imm,"sqshl",{fn(setSIMDMode),fn(OPRQ),fn(OPRimm<22,19>),fn(OPRimm<18,16>),fn(OPRRn),fn(OPRRd),NULL},251687936,3212901376},
    {aarch64_op_sqshl_advsimd_reg,"sqshl",{fn(setSIMDMode),fn(OPRsize<23,22>),fn(OPRRm),fn(OPRRn),fn(OPRRd),NULL},1579174912,4280351744},
    {aarch64_op_sqshl_advsimd_reg,"sqshl",{fn(setSIMDMode),fn(OPRQ),fn(OPRsize<23,22>),fn(OPRRm),fn(OPRRn),fn(OPRRd),NULL},236997632,3206609920},
    {aarch64_op_sqshlu_advsimd,"sqshlu",{fn(setSIMDMode),fn(OPRimm<22,19>),fn(OPRimm<18,16>),fn(OPRRn),fn(OPRRd),NULL},2130732032,4286643200},
    {aarch64_op_sqshlu_advsimd,"sqshlu",{fn(setSIMDMode),fn(OPRQ),fn(OPRimm<22,19>),fn(OPRimm<18,16>),fn(OPRRn),fn(OPRRd),NULL},788554752,3212901376},
    {aarch64_op_sqshrn_advsimd,"sqshrn",{fn(setSIMDMode),fn(OPRimm<22,19>),fn(OPRimm<18,16>),fn(OPRRn),fn(OPRRd),NULL},1593873408,4286643200},
    {aarch64_op_sqshrn_advsimd,"sqshrn",{fn(setSIMDMode),fn(OPRQ),fn(OPRimm<22,19>),fn(OPRimm<18,16>),fn(OPRRn),fn(OPRRd),NULL},251696128,3212901376},
    {aarch64_op_sqshrun_advsimd,"sqshrun",{fn(setSIMDMode),fn(OPRimm<22,19>),fn(OPRimm<18,16>),fn(OPRRn),fn(OPRRd),NULL},2130740224,4286643200},
    {aarch64_op_sqshrun_advsimd,"sqshrun",{fn(setSIMDMode),fn(OPRQ),fn(OPRimm<22,19>),fn(OPRimm<18,16>),fn(OPRRn),fn(OPRRd),NULL},788562944,3212901376},
    {aarch64_op_sqsub_advsimd,"sqsub",{fn(setSIMDMode),fn(OPRsize<23,22>),fn(OPRRm),fn(OPRRn),fn(OPRRd),NULL},1579166720,4280351744},
    {aarch64_op_sqsub_advsimd,"sqsub",{fn(setSIMDMode),fn(OPRQ),fn(OPRsize<23,22>),fn(OPRRm),fn(OPRRn),fn(OPRRd),NULL},236989440,3206609920},
    {aarch64_op_sqxtn_advsimd,"sqxtn",{fn(setSIMDMode),fn(OPRsize<23,22>),fn(OPRRn),fn(OPRRd),NULL},1579239424,4282383360},
    {aarch64_op_sqxtn_advsimd,"sqxtn",{fn(setSIMDMode),fn(OPRQ),fn(OPRsize<23,22>),fn(OPRRn),fn(OPRRd),NULL},237062144,3208641536},
    {aarch64_op_sqxtun_advsimd,"sqxtun",{fn(setSIMDMode),fn(OPRsize<23,22>),fn(OPRRn),fn(OPRRd),NULL},2116102144,4282383360},
    {aarch64_op_sqxtun_advsimd,"sqxtun",{fn(setSIMDMode),fn(OPRQ),fn(OPRsize<23,22>),fn(OPRRn),fn(OPRRd),NULL},773924864,3208641536},
    {aarch64_op_srhadd_advsimd,"srhadd",{fn(setSIMDMode),fn(OPRQ),fn(OPRsize<23,22>),fn(OPRRm),fn(OPRRn),fn(OPRRd),NULL},236983296,3206609920},
    {aarch64_op_sri_advsimd,"sri",{fn(setSIMDMode),fn(OPRimm<22,19>),fn(OPRimm<18,16>),fn(OPRRn),fn(OPRRd),NULL},2130723840,4286643200},
    {aarch64_op_sri_advsimd,"sri",{fn(setSIMDMode),fn(OPRQ),fn(OPRimm<22,19>),fn(OPRimm<18,16>),fn(OPRRn),fn(OPRRd),NULL},788546560,3212901376},
    {aarch64_op_srshl_advsimd,"srshl",{fn(setSIMDMode),fn(OPRsize<23,22>),fn(OPRRm),fn(OPRRn),fn(OPRRd),NULL},1579176960,4280351744},
    {aarch64_op_srshl_advsimd,"srshl",{fn(setSIMDMode),fn(OPRQ),fn(OPRsize<23,22>),fn(OPRRm),fn(OPRRn),fn(OPRRd),NULL},236999680,3206609920},
    {aarch64_op_srshr_advsimd,"srshr",{fn(setSIMDMode),fn(OPRimm<22,19>),fn(OPRimm<18,16>),fn(OPRRn),fn(OPRRd),NULL},1593844736,4286643200},
    {aarch64_op_srshr_advsimd,"srshr",{fn(setSIMDMode),fn(OPRQ),fn(OPRimm<22,19>),fn(OPRimm<18,16>),fn(OPRRn),fn(OPRRd),NULL},251667456,3212901376},
    {aarch64_op_srsra_advsimd,"srsra",{fn(setSIMDMode),fn(OPRimm<22,19>),fn(OPRimm<18,16>),fn(OPRRn),fn(OPRRd),NULL},1593848832,4286643200},
    {aarch64_op_srsra_advsimd,"srsra",{fn(setSIMDMode),fn(OPRQ),fn(OPRimm<22,19>),fn(OPRimm<18,16>),fn(OPRRn),fn(OPRRd),NULL},251671552,3212901376},
    {aarch64_op_sshl_advsimd,"sshl",{fn(setSIMDMode),fn(OPRsize<23,22>),fn(OPRRm),fn(OPRRn),fn(OPRRd),NULL},1579172864,4280351744},
    {aarch64_op_sshl_advsimd,"sshl",{fn(setSIMDMode),fn(OPRQ),fn(OPRsize<23,22>),fn(OPRRm),fn(OPRRn),fn(OPRRd),NULL},236995584,3206609920},
    {aarch64_op_sshll_advsimd,"sshll",{fn(setSIMDMode),fn(OPRQ),fn(OPRimm<22,19>),fn(OPRimm<18,16>),fn(OPRRn),fn(OPRRd),NULL},251700224,3212901376},
    {aarch64_op_sshr_advsimd,"sshr",{fn(setSIMDMode),fn(OPRimm<22,19>),fn(OPRimm<18,16>),fn(OPRRn),fn(OPRRd),NULL},1593836544,4286643200},
    {aarch64_op_sshr_advsimd,"sshr",{fn(setSIMDMode),fn(OPRQ),fn(OPRimm<22,19>),fn(OPRimm<18,16>),fn(OPRRn),fn(OPRRd),NULL},251659264,3212901376},
    {aarch64_op_ssra_advsimd,"ssra",{fn(setSIMDMode),fn(OPRimm<22,19>),fn(OPRimm<18,16>),fn(OPRRn),fn(OPRRd),NULL},1593840640,4286643200},
    {aarch64_op_ssra_advsimd,"ssra",{fn(setSIMDMode),fn(OPRQ),fn(OPRimm<22,19>),fn(OPRimm<18,16>),fn(OPRRn),fn(OPRRd),NULL},251663360,3212901376},
    {aarch64_op_ssubl_advsimd,"ssubl",{fn(setSIMDMode),fn(OPRQ),fn(OPRsize<23,22>),fn(OPRRm),fn(OPRRn),fn(OPRRd),NULL},236986368,3206609920},
    {aarch64_op_ssubw_advsimd,"ssubw",{fn(setSIMDMode),fn(OPRQ),fn(OPRsize<23,22>),fn(OPRRm),fn(OPRRn),fn(OPRRd),NULL},236990464,3206609920},
    {aarch64_op_st1_advsimd_mult,"st1",{fn(setSIMDMode),fn(OPRQ),fn(OPRopcode),fn(OPRsize<11,10>),fn(OPRRnS),fn(OPRRtS),NULL},201334784,3221168128},
    {aarch64_op_st1_advsimd_mult,"st1",{fn(setSIMDMode),fn(OPRQ),fn(OPRRm),fn(OPRopcode),fn(OPRsize<11,10>),fn(OPRRnS),fn(OPRRtS),NULL},209723392,3219136512},
    {aarch64_op_st1_advsimd_sngl,"st1",{fn(setSIMDMode),fn(OPRQ),fn(OPRopcode),fn(OPRS<12,12>),fn(OPRsize<11,10>),fn(OPRRnS),fn(OPRRtS),NULL},218103808,3221168128},
    {aarch64_op_st1_advsimd_sngl,"st1",{fn(setSIMDMode),fn(OPRQ),fn(OPRRm),fn(OPRopcode),fn(OPRS<12,12>),fn(OPRsize<11,10>),fn(OPRRnS),fn(OPRRtS),NULL},226492416,3219136512},
    {aarch64_op_st2_advsimd_mult,"st2",{fn(setSIMDMode),fn(OPRQ),fn(OPRsize<11,10>),fn(OPRRnS),fn(OPRRtS),NULL},201359360,3221221376},
    {aarch64_op_st2_advsimd_mult,"st2",{fn(setSIMDMode),fn(OPRQ),fn(OPRRm),fn(OPRsize<11,10>),fn(OPRRnS),fn(OPRRtS),NULL},209747968,3219189760},
    {aarch64_op_st2_advsimd_sngl,"st2",{fn(setSIMDMode),fn(OPRQ),fn(OPRopcode),fn(OPRS<12,12>),fn(OPRsize<11,10>),fn(OPRRnS),fn(OPRRtS),NULL},220200960,3221168128},
    {aarch64_op_st2_advsimd_sngl,"st2",{fn(setSIMDMode),fn(OPRQ),fn(OPRRm),fn(OPRopcode),fn(OPRS<12,12>),fn(OPRsize<11,10>),fn(OPRRnS),fn(OPRRtS),NULL},228589568,3219136512},
    {aarch64_op_st3_advsimd_mult,"st3",{fn(setSIMDMode),fn(OPRQ),fn(OPRsize<11,10>),fn(OPRRnS),fn(OPRRtS),NULL},201342976,3221221376},
    {aarch64_op_st3_advsimd_mult,"st3",{fn(setSIMDMode),fn(OPRQ),fn(OPRRm),fn(OPRsize<11,10>),fn(OPRRnS),fn(OPRRtS),NULL},209731584,3219189760},
    {aarch64_op_st3_advsimd_sngl,"st3",{fn(setSIMDMode),fn(OPRQ),fn(OPRopcode),fn(OPRS<12,12>),fn(OPRsize<11,10>),fn(OPRRnS),fn(OPRRtS),NULL},218112000,3221168128},
    {aarch64_op_st3_advsimd_sngl,"st3",{fn(setSIMDMode),fn(OPRQ),fn(OPRRm),fn(OPRopcode),fn(OPRS<12,12>),fn(OPRsize<11,10>),fn(OPRRnS),fn(OPRRtS),NULL},226500608,3219136512},
    {aarch64_op_st4_advsimd_mult,"st4",{fn(setSIMDMode),fn(OPRQ),fn(OPRsize<11,10>),fn(OPRRnS),fn(OPRRtS),NULL},201326592,3221221376},
    {aarch64_op_st4_advsimd_mult,"st4",{fn(setSIMDMode),fn(OPRQ),fn(OPRRm),fn(OPRsize<11,10>),fn(OPRRnS),fn(OPRRtS),NULL},209715200,3219189760},
    {aarch64_op_st4_advsimd_sngl,"st4",{fn(setSIMDMode),fn(OPRQ),fn(OPRopcode),fn(OPRS<12,12>),fn(OPRsize<11,10>),fn(OPRRnS),fn(OPRRtS),NULL},220209152,3221168128},
    {aarch64_op_st4_advsimd_sngl,"st4",{fn(setSIMDMode),fn(OPRQ),fn(OPRRm),fn(OPRopcode),fn(OPRS<12,12>),fn(OPRsize<11,10>),fn(OPRRnS),fn(OPRRtS),NULL},228597760,3219136512},
    {aarch64_op_stlr,"stlr",{fn(setRegWidth),fn(OPRsize<31,30>),fn(OPRRnS),fn(OPRRtS),NULL},2292186112,3221224448},
    {aarch64_op_stlrb,"stlrb",{fn(setRegWidth),fn(OPRRnS),fn(OPRRtS),NULL},144702464,4294966272},
    {aarch64_op_stlrh,"stlrh",{fn(setRegWidth),fn(OPRRnS),fn(OPRRtS),NULL},1218444288,4294966272},
    {aarch64_op_stlxp,"stlxp",{fn(setRegWidth),fn(OPRsz<30,30>),fn(OPRRs),fn(OPRRt2S),fn(OPRRnS),fn(OPRRtS),NULL},2283831296,3219161088},
    {aarch64_op_stlxr,"stlxr",{fn(setRegWidth),fn(OPRsize<31,30>),fn(OPRRs),fn(OPRRnS),fn(OPRRtS),NULL},2281765888,3219192832},
    {aarch64_op_stlxrb,"stlxrb",{fn(setRegWidth),fn(OPRRs),fn(OPRRnS),fn(OPRRtS),NULL},134282240,4292934656},
    {aarch64_op_stlxrh,"stlxrh",{fn(setRegWidth),fn(OPRRs),fn(OPRRnS),fn(OPRRtS),NULL},1208024064,4292934656},
    {aarch64_op_stnp_fpsimd,"stnp",{fn(setRegWidth),fn(setSIMDMode),fn(OPRopc),fn(OPRimm<21,15>),fn(OPRRt2S),fn(OPRRnS),fn(OPRRtS),NULL},738197504,1069547520},
    {aarch64_op_stnp_gen,"stnp",{fn(setRegWidth),fn(OPRopc),fn(OPRimm<21,15>),fn(OPRRt2S),fn(OPRRnS),fn(OPRRtS),NULL},671088640,2143289344},
    {aarch64_op_stp_fpsimd,"stp",{fn(setRegWidth),fn(setSIMDMode),fn(OPRopc),fn(OPRimm<21,15>),fn(OPRRt2S),fn(OPRRnSU),fn(OPRRtS),NULL},746586112,1069547520},
    {aarch64_op_stp_fpsimd,"stp",{fn(setRegWidth),fn(setSIMDMode),fn(OPRopc),fn(OPRimm<21,15>),fn(OPRRt2S),fn(OPRRnSU),fn(OPRRtS),NULL},763363328,1069547520},
    {aarch64_op_stp_fpsimd,"stp",{fn(setRegWidth),fn(setSIMDMode),fn(OPRopc),fn(OPRimm<21,15>),fn(OPRRt2S),fn(OPRRnS),fn(OPRRtS),NULL},754974720,1069547520},
    {aarch64_op_stp_gen,"stp",{fn(setRegWidth),fn(OPRopc),fn(OPRimm<21,15>),fn(OPRRt2S),fn(OPRRnSU),fn(OPRRtS),NULL},679477248,2143289344},
    {aarch64_op_stp_gen,"stp",{fn(setRegWidth),fn(OPRopc),fn(OPRimm<21,15>),fn(OPRRt2S),fn(OPRRnSU),fn(OPRRtS),NULL},696254464,2143289344},
    {aarch64_op_stp_gen,"stp",{fn(setRegWidth),fn(OPRopc),fn(OPRimm<21,15>),fn(OPRRt2S),fn(OPRRnS),fn(OPRRtS),NULL},687865856,2143289344},
    {aarch64_op_str_imm_fpsimd,"str",{fn(setRegWidth),fn(setSIMDMode),fn(OPRsize<31,30>),fn(OPRopc),fn(OPRimm<20,12>),fn(OPRRnSU),fn(OPRRtS),NULL},1006633984,1063259136},
    {aarch64_op_str_imm_fpsimd,"str",{fn(setRegWidth),fn(setSIMDMode),fn(OPRsize<31,30>),fn(OPRopc),fn(OPRimm<20,12>),fn(OPRRnSU),fn(OPRRtS),NULL},1006636032,1063259136},
    {aarch64_op_str_imm_fpsimd,"str",{fn(setRegWidth),fn(setSIMDMode),fn(OPRsize<31,30>),fn(OPRopc),fn(OPRimm<21,10>),fn(OPRRnS),fn(OPRRtS),NULL},1023410176,1061158912},
    {aarch64_op_str_imm_gen,"str",{fn(setRegWidth),fn(OPRsize<31,30>),fn(OPRimm<20,12>),fn(OPRRnSU),fn(OPRRtS),NULL},3087008768,3219131392},
    {aarch64_op_str_imm_gen,"str",{fn(setRegWidth),fn(OPRsize<31,30>),fn(OPRimm<20,12>),fn(OPRRnSU),fn(OPRRtS),NULL},3087010816,3219131392},
    {aarch64_op_str_imm_gen,"str",{fn(setRegWidth),fn(OPRsize<31,30>),fn(OPRimm<21,10>),fn(OPRRnS),fn(OPRRtS),NULL},3103784960,3217031168},
    {aarch64_op_str_reg_fpsimd,"str",{fn(setRegWidth),fn(setSIMDMode),fn(OPRsize<31,30>),fn(OPRopc),fn(OPRRm),fn(OPRoption<15,13>),fn(OPRS<12,12>),fn(OPRRnS),fn(OPRRtS),NULL},1008732160,1063259136},
    {aarch64_op_str_reg_gen,"str",{fn(setRegWidth),fn(OPRsize<31,30>),fn(OPRRm),fn(OPRoption<15,13>),fn(OPRS<12,12>),fn(OPRRnS),fn(OPRRtS),NULL},3089106944,3219131392},
    {aarch64_op_strb_imm,"strb",{fn(setRegWidth),fn(OPRimm<20,12>),fn(OPRRnSU),fn(OPRRtS),NULL},939525120,4292873216},
    {aarch64_op_strb_imm,"strb",{fn(setRegWidth),fn(OPRimm<20,12>),fn(OPRRnSU),fn(OPRRtS),NULL},939527168,4292873216},
    {aarch64_op_strb_imm,"strb",{fn(setRegWidth),fn(OPRimm<21,10>),fn(OPRRnS),fn(OPRRtS),NULL},956301312,4290772992},
    {aarch64_op_strb_reg,"strb",{fn(setRegWidth),fn(OPRRm),fn(OPRoption<15,13>),fn(OPRS<12,12>),fn(OPRRnS),fn(OPRRtS),NULL},941623296,4292873216},
    {aarch64_op_strh_imm,"strh",{fn(setRegWidth),fn(OPRimm<20,12>),fn(OPRRnSU),fn(OPRRtS),NULL},2013266944,4292873216},
    {aarch64_op_strh_imm,"strh",{fn(setRegWidth),fn(OPRimm<20,12>),fn(OPRRnSU),fn(OPRRtS),NULL},2013268992,4292873216},
    {aarch64_op_strh_imm,"strh",{fn(setRegWidth),fn(OPRimm<21,10>),fn(OPRRnS),fn(OPRRtS),NULL},2030043136,4290772992},
    {aarch64_op_strh_reg,"strh",{fn(setRegWidth),fn(OPRRm),fn(OPRoption<15,13>),fn(OPRS<12,12>),fn(OPRRnS),fn(OPRRtS),NULL},2015365120,4292873216},
    {aarch64_op_sttr,"sttr",{fn(setRegWidth),fn(OPRsize<31,30>),fn(OPRimm<20,12>),fn(OPRRnS),fn(OPRRtS),NULL},3087009792,3219131392},
    {aarch64_op_sttrb,"sttrb",{fn(setRegWidth),fn(OPRimm<20,12>),fn(OPRRnS),fn(OPRRtS),NULL},939526144,4292873216},
    {aarch64_op_sttrh,"sttrh",{fn(setRegWidth),fn(OPRimm<20,12>),fn(OPRRnS),fn(OPRRtS),NULL},2013267968,4292873216},
    {aarch64_op_stur_fpsimd,"stur",{fn(setRegWidth),fn(setSIMDMode),fn(OPRsize<31,30>),fn(OPRopc),fn(OPRimm<20,12>),fn(OPRRnS),fn(OPRRtS),NULL},1006632960,1063259136},
    {aarch64_op_stur_gen,"stur",{fn(setRegWidth),fn(OPRsize<31,30>),fn(OPRimm<20,12>),fn(OPRRnS),fn(OPRRtS),NULL},3087007744,3219131392},
    {aarch64_op_sturb,"sturb",{fn(setRegWidth),fn(OPRimm<20,12>),fn(OPRRnS),fn(OPRRtS),NULL},939524096,4292873216},
    {aarch64_op_sturh,"sturh",{fn(setRegWidth),fn(OPRimm<20,12>),fn(OPRRnS),fn(OPRRtS),NULL},2013265920,4292873216},
    {aarch64_op_stxp,"stxp",{fn(setRegWidth),fn(OPRsz<30,30>),fn(OPRRs),fn(OPRRt2S),fn(OPRRnS),fn(OPRRtS),NULL},2283798528,3219161088},
    {aarch64_op_stxr,"stxr",{fn(setRegWidth),fn(OPRsize<31,30>),fn(OPRRs),fn(OPRRnS),fn(OPRRtS),NULL},2281733120,3219192832},
    {aarch64_op_stxrb,"stxrb",{fn(setRegWidth),fn(OPRRs),fn(OPRRnS),fn(OPRRtS),NULL},134249472,4292934656},
    {aarch64_op_stxrh,"stxrh",{fn(setRegWidth),fn(OPRRs),fn(OPRRnS),fn(OPRRtS),NULL},1207991296,4292934656},
    {aarch64_op_sub_addsub_ext,"sub",{fn(OPRsf),fn(OPRRm),fn(OPRoption<15,13>),fn(OPRimm<12,10>),fn(OPRRn),fn(OPRRd),NULL},1260388352,2145386496},
    {aarch64_op_sub_addsub_imm,"sub",{fn(OPRsf),fn(OPRshift),fn(OPRimm<21,10>),fn(OPRRn),fn(OPRRd),NULL},1358954496,2130706432},
    {aarch64_op_sub_addsub_shift,"sub",{fn(OPRsf),fn(OPRshift),fn(OPRRm),fn(OPRimm<15,10>),fn(OPRRn),fn(OPRRd),NULL},1258291200,2132803584},
    {aarch64_op_sub_advsimd,"sub",{fn(setSIMDMode),fn(OPRsize<23,22>),fn(OPRRm),fn(OPRRn),fn(OPRRd),NULL},2116060160,4280351744},
    {aarch64_op_sub_advsimd,"sub",{fn(setSIMDMode),fn(OPRQ),fn(OPRsize<23,22>),fn(OPRRm),fn(OPRRn),fn(OPRRd),NULL},773882880,3206609920},
    {aarch64_op_subhn_advsimd,"subhn",{fn(setSIMDMode),fn(OPRQ),fn(OPRsize<23,22>),fn(OPRRm),fn(OPRRn),fn(OPRRd),NULL},237002752,3206609920},
    {aarch64_op_subs_addsub_ext,"subs",{fn(setFlags),fn(OPRsf),fn(OPRRm),fn(OPRoption<15,13>),fn(OPRimm<12,10>),fn(OPRRn),fn(OPRRd),NULL},1797259264,2145386496},
    {aarch64_op_subs_addsub_imm,"subs",{fn(setFlags),fn(OPRsf),fn(OPRshift),fn(OPRimm<21,10>),fn(OPRRn),fn(OPRRd),NULL},1895825408,2130706432},
    {aarch64_op_subs_addsub_shift,"subs",{fn(setFlags),fn(OPRsf),fn(OPRshift),fn(OPRRm),fn(OPRimm<15,10>),fn(OPRRn),fn(OPRRd),NULL},1795162112,2132803584},
    {aarch64_op_suqadd_advsimd,"suqadd",{fn(setSIMDMode),fn(OPRsize<23,22>),fn(OPRRn),fn(OPRRd),NULL},1579169792,4282383360},
    {aarch64_op_suqadd_advsimd,"suqadd",{fn(setSIMDMode),fn(OPRQ),fn(OPRsize<23,22>),fn(OPRRn),fn(OPRRd),NULL},236992512,3208641536},
    {aarch64_op_svc,"svc",{fn(OPRimm<20,5>),NULL},3556769793,4292870175},
    {aarch64_op_sxtb_sbfm,"sxtb",{fn(OPRsf),fn(OPRN<22,22>),fn(OPRRn),fn(OPRRd),NULL},318774272,2143288320},
    {aarch64_op_sxth_sbfm,"sxth",{fn(OPRsf),fn(OPRN<22,22>),fn(OPRRn),fn(OPRRd),NULL},318782464,2143288320},
    {aarch64_op_sxtl_sshll_advsimd,"sxtl",{fn(setSIMDMode),fn(OPRQ),fn(OPRimm<22,19>),fn(OPRRn),fn(OPRRd),NULL},251700224,3213360128},
    {aarch64_op_sxtw_sbfm,"sxtw",{fn(OPRRn),fn(OPRRd),NULL},2470476800,4294966272},
    {aarch64_op_sys,"sys",{fn(OPRop1),fn(OPRCRn),fn(OPRCRm),fn(OPRop2),fn(OPRRt),NULL},3574071296,4294443008},
    {aarch64_op_sysl,"sysl",{fn(OPRop1),fn(OPRCRn),fn(OPRCRm),fn(OPRop2),fn(OPRRt),NULL},3576168448,4294443008},
    {aarch64_op_tbl_advsimd,"tbl",{fn(setSIMDMode),fn(OPRQ),fn(OPRRm),fn(OPRlen),fn(OPRRn),fn(OPRRd),NULL},234881024,3219168256},
    {aarch64_op_tbnz,"tbnz",{fn(OPRb5),fn(OPRb40),fn(OPRimm<18,5>),fn(OPRRt),NULL},922746880,2130706432},
    {aarch64_op_tbx_advsimd,"tbx",{fn(setSIMDMode),fn(OPRQ),fn(OPRRm),fn(OPRlen),fn(OPRRn),fn(OPRRd),NULL},234885120,3219168256},
    {aarch64_op_tbz,"tbz",{fn(OPRb5),fn(OPRb40),fn(OPRimm<18,5>),fn(OPRRt),NULL},905969664,2130706432},
    {aarch64_op_tlbi_sys,"tlbi",{fn(OPRop1),fn(OPRCRm),fn(OPRop2),fn(OPRRt),NULL},3574104064,4294504448},
    {aarch64_op_trn1_advsimd,"trn1",{fn(setSIMDMode),fn(OPRQ),fn(OPRsize<23,22>),fn(OPRRm),fn(OPRRn),fn(OPRRd),NULL},234891264,3206609920},
    {aarch64_op_trn2_advsimd,"trn2",{fn(setSIMDMode),fn(OPRQ),fn(OPRsize<23,22>),fn(OPRRm),fn(OPRRn),fn(OPRRd),NULL},234907648,3206609920},
    {aarch64_op_tst_ands_log_imm,"tst",{fn(setFlags),fn(OPRsf),fn(OPRN<22,22>),fn(OPRimm<21,16>),fn(OPRimm<15,10>),fn(OPRRn),NULL},1912602655,2139095071},
    {aarch64_op_tst_ands_log_shift,"tst",{fn(setFlags),fn(OPRsf),fn(OPRshift),fn(OPRRm),fn(OPRimm<15,10>),fn(OPRRn),NULL},1778384927,2132803615},
    {aarch64_op_uaba_advsimd,"uaba",{fn(setSIMDMode),fn(OPRQ),fn(OPRsize<23,22>),fn(OPRRm),fn(OPRRn),fn(OPRRd),NULL},773880832,3206609920},
    {aarch64_op_uabal_advsimd,"uabal",{fn(setSIMDMode),fn(OPRQ),fn(OPRsize<23,22>),fn(OPRRm),fn(OPRRn),fn(OPRRd),NULL},773869568,3206609920},
    {aarch64_op_uabd_advsimd,"uabd",{fn(setSIMDMode),fn(OPRQ),fn(OPRsize<23,22>),fn(OPRRm),fn(OPRRn),fn(OPRRd),NULL},773878784,3206609920},
    {aarch64_op_uabdl_advsimd,"uabdl",{fn(setSIMDMode),fn(OPRQ),fn(OPRsize<23,22>),fn(OPRRm),fn(OPRRn),fn(OPRRd),NULL},773877760,3206609920},
    {aarch64_op_uadalp_advsimd,"uadalp",{fn(setSIMDMode),fn(OPRQ),fn(OPRsize<23,22>),fn(OPRRn),fn(OPRRd),NULL},773875712,3208641536},
    {aarch64_op_uaddl_advsimd,"uaddl",{fn(setSIMDMode),fn(OPRQ),fn(OPRsize<23,22>),fn(OPRRm),fn(OPRRn),fn(OPRRd),NULL},773849088,3206609920},
    {aarch64_op_uaddlp_advsimd,"uaddlp",{fn(setSIMDMode),fn(OPRQ),fn(OPRsize<23,22>),fn(OPRRn),fn(OPRRd),NULL},773859328,3208641536},
    {aarch64_op_uaddlv_advsimd,"uaddlv",{fn(setSIMDMode),fn(OPRQ),fn(OPRsize<23,22>),fn(OPRRn),fn(OPRRd),NULL},774912000,3208641536},
    {aarch64_op_uaddw_advsimd,"uaddw",{fn(setSIMDMode),fn(OPRQ),fn(OPRsize<23,22>),fn(OPRRm),fn(OPRRn),fn(OPRRd),NULL},773853184,3206609920},
    {aarch64_op_ubfiz_ubfm,"ubfiz",{fn(OPRsf),fn(OPRN<22,22>),fn(OPRimm<21,16>),fn(OPRimm<15,10>),fn(OPRRn),fn(OPRRd),NULL},1392508928,2139095040},
    {aarch64_op_ubfm,"ubfm",{fn(OPRsf),fn(OPRN<22,22>),fn(OPRimm<21,16>),fn(OPRimm<15,10>),fn(OPRRn),fn(OPRRd),NULL},1392508928,2139095040},
    {aarch64_op_ubfx_ubfm,"ubfx",{fn(OPRsf),fn(OPRN<22,22>),fn(OPRimm<21,16>),fn(OPRimm<15,10>),fn(OPRRn),fn(OPRRd),NULL},1392508928,2139095040},
    {aarch64_op_ucvtf_advsimd_fix,"ucvtf",{fn(setSIMDMode),fn(OPRimm<22,19>),fn(OPRimm<18,16>),fn(OPRRn),fn(OPRRd),NULL},2130764800,4286643200},
    {aarch64_op_ucvtf_advsimd_fix,"ucvtf",{fn(setSIMDMode),fn(OPRQ),fn(OPRimm<22,19>),fn(OPRimm<18,16>),fn(OPRRn),fn(OPRRd),NULL},788587520,3212901376},
    {aarch64_op_ucvtf_advsimd_int,"ucvtf",{fn(setSIMDMode),fn(OPRsz<22,22>),fn(OPRRn),fn(OPRRd),NULL},2116147200,4290771968},
    {aarch64_op_ucvtf_advsimd_int,"ucvtf",{fn(setSIMDMode),fn(OPRQ),fn(OPRsz<22,22>),fn(OPRRn),fn(OPRRd),NULL},773969920,3217030144},
    {aarch64_op_ucvtf_float_fix,"ucvtf",{fn(setFPMode),fn(OPRsf),fn(OPRtype<23,22>),fn(OPRscale),fn(OPRRn),fn(OPRRd),NULL},503513088,2143223808},
    {aarch64_op_ucvtf_float_int,"ucvtf",{fn(setFPMode),fn(OPRsf),fn(OPRtype<23,22>),fn(OPRRn),fn(OPRRd),NULL},505610240,2143288320},
    {aarch64_op_udiv,"udiv",{fn(OPRsf),fn(OPRRm),fn(OPRRn),fn(OPRRd),NULL},448792576,2145451008},
    {aarch64_op_uhadd_advsimd,"uhadd",{fn(setSIMDMode),fn(OPRQ),fn(OPRsize<23,22>),fn(OPRRm),fn(OPRRn),fn(OPRRd),NULL},773850112,3206609920},
    {aarch64_op_uhsub_advsimd,"uhsub",{fn(setSIMDMode),fn(OPRQ),fn(OPRsize<23,22>),fn(OPRRm),fn(OPRRn),fn(OPRRd),NULL},773858304,3206609920},
    {aarch64_op_umaddl,"umaddl",{fn(OPRRm),fn(OPRRa),fn(OPRRn),fn(OPRRd),NULL},2610954240,4292902912},
    {aarch64_op_umax_advsimd,"umax",{fn(setSIMDMode),fn(OPRQ),fn(OPRsize<23,22>),fn(OPRRm),fn(OPRRn),fn(OPRRd),NULL},773874688,3206609920},
    {aarch64_op_umaxp_advsimd,"umaxp",{fn(setSIMDMode),fn(OPRQ),fn(OPRsize<23,22>),fn(OPRRm),fn(OPRRn),fn(OPRRd),NULL},773891072,3206609920},
    {aarch64_op_umaxv_advsimd,"umaxv",{fn(setSIMDMode),fn(OPRQ),fn(OPRsize<23,22>),fn(OPRRn),fn(OPRRd),NULL},774940672,3208641536},
    {aarch64_op_umin_advsimd,"umin",{fn(setSIMDMode),fn(OPRQ),fn(OPRsize<23,22>),fn(OPRRm),fn(OPRRn),fn(OPRRd),NULL},773876736,3206609920},
    {aarch64_op_uminp_advsimd,"uminp",{fn(setSIMDMode),fn(OPRQ),fn(OPRsize<23,22>),fn(OPRRm),fn(OPRRn),fn(OPRRd),NULL},773893120,3206609920},
    {aarch64_op_uminv_advsimd,"uminv",{fn(setSIMDMode),fn(OPRQ),fn(OPRsize<23,22>),fn(OPRRn),fn(OPRRd),NULL},775006208,3208641536},
    {aarch64_op_umlal_advsimd_elt,"umlal",{fn(setSIMDMode),fn(OPRQ),fn(OPRsize<23,22>),fn(OPRL),fn(OPRM),fn(OPRRm),fn(OPRH),fn(OPRRn),fn(OPRRd),NULL},788537344,3204510720},
    {aarch64_op_umlal_advsimd_vec,"umlal",{fn(setSIMDMode),fn(OPRQ),fn(OPRsize<23,22>),fn(OPRRm),fn(OPRRn),fn(OPRRd),NULL},773881856,3206609920},
    {aarch64_op_umlsl_advsimd_elt,"umlsl",{fn(setSIMDMode),fn(OPRQ),fn(OPRsize<23,22>),fn(OPRL),fn(OPRM),fn(OPRRm),fn(OPRH),fn(OPRRn),fn(OPRRd),NULL},788553728,3204510720},
    {aarch64_op_umlsl_advsimd_vec,"umlsl",{fn(setSIMDMode),fn(OPRQ),fn(OPRsize<23,22>),fn(OPRRm),fn(OPRRn),fn(OPRRd),NULL},773890048,3206609920},
    {aarch64_op_umnegl_umsubl,"umnegl",{fn(OPRRm),fn(OPRRn),fn(OPRRd),NULL},2611018752,4292934656},
    {aarch64_op_umov_advsimd,"umov",{fn(setSIMDMode),fn(OPRQ),fn(OPRimm<20,16>),fn(OPRRn),fn(OPRRd),NULL},234896384,3219192832},
    {aarch64_op_umsubl,"umsubl",{fn(OPRRm),fn(OPRRa),fn(OPRRn),fn(OPRRd),NULL},2610987008,4292902912},
    {aarch64_op_umulh,"umulh",{fn(OPRRm),fn(OPRRn),fn(OPRRd),NULL},2613083136,4292934656},
    {aarch64_op_umull_advsimd_elt,"umull",{fn(setSIMDMode),fn(OPRQ),fn(OPRsize<23,22>),fn(OPRL),fn(OPRM),fn(OPRRm),fn(OPRH),fn(OPRRn),fn(OPRRd),NULL},788570112,3204510720},
    {aarch64_op_umull_advsimd_vec,"umull",{fn(setSIMDMode),fn(OPRQ),fn(OPRsize<23,22>),fn(OPRRm),fn(OPRRn),fn(OPRRd),NULL},773898240,3206609920},
    {aarch64_op_umull_umaddl,"umull",{fn(OPRRm),fn(OPRRn),fn(OPRRd),NULL},2610985984,4292934656},
    {aarch64_op_uqadd_advsimd,"uqadd",{fn(setSIMDMode),fn(OPRsize<23,22>),fn(OPRRm),fn(OPRRn),fn(OPRRd),NULL},2116029440,4280351744},
    {aarch64_op_uqadd_advsimd,"uqadd",{fn(setSIMDMode),fn(OPRQ),fn(OPRsize<23,22>),fn(OPRRm),fn(OPRRn),fn(OPRRd),NULL},773852160,3206609920},
    {aarch64_op_uqrshl_advsimd,"uqrshl",{fn(setSIMDMode),fn(OPRsize<23,22>),fn(OPRRm),fn(OPRRn),fn(OPRRd),NULL},2116049920,4280351744},
    {aarch64_op_uqrshl_advsimd,"uqrshl",{fn(setSIMDMode),fn(OPRQ),fn(OPRsize<23,22>),fn(OPRRm),fn(OPRRn),fn(OPRRd),NULL},773872640,3206609920},
    {aarch64_op_uqrshrn_advsimd,"uqrshrn",{fn(setSIMDMode),fn(OPRimm<22,19>),fn(OPRimm<18,16>),fn(OPRRn),fn(OPRRd),NULL},2130746368,4286643200},
    {aarch64_op_uqrshrn_advsimd,"uqrshrn",{fn(setSIMDMode),fn(OPRQ),fn(OPRimm<22,19>),fn(OPRimm<18,16>),fn(OPRRn),fn(OPRRd),NULL},788569088,3212901376},
    {aarch64_op_uqshl_advsimd_imm,"uqshl",{fn(setSIMDMode),fn(OPRimm<22,19>),fn(OPRimm<18,16>),fn(OPRRn),fn(OPRRd),NULL},2130736128,4286643200},
    {aarch64_op_uqshl_advsimd_imm,"uqshl",{fn(setSIMDMode),fn(OPRQ),fn(OPRimm<22,19>),fn(OPRimm<18,16>),fn(OPRRn),fn(OPRRd),NULL},788558848,3212901376},
    {aarch64_op_uqshl_advsimd_reg,"uqshl",{fn(setSIMDMode),fn(OPRsize<23,22>),fn(OPRRm),fn(OPRRn),fn(OPRRd),NULL},2116045824,4280351744},
    {aarch64_op_uqshl_advsimd_reg,"uqshl",{fn(setSIMDMode),fn(OPRQ),fn(OPRsize<23,22>),fn(OPRRm),fn(OPRRn),fn(OPRRd),NULL},773868544,3206609920},
    {aarch64_op_uqshrn_advsimd,"uqshrn",{fn(setSIMDMode),fn(OPRimm<22,19>),fn(OPRimm<18,16>),fn(OPRRn),fn(OPRRd),NULL},2130744320,4286643200},
    {aarch64_op_uqshrn_advsimd,"uqshrn",{fn(setSIMDMode),fn(OPRQ),fn(OPRimm<22,19>),fn(OPRimm<18,16>),fn(OPRRn),fn(OPRRd),NULL},788567040,3212901376},
    {aarch64_op_uqsub_advsimd,"uqsub",{fn(setSIMDMode),fn(OPRsize<23,22>),fn(OPRRm),fn(OPRRn),fn(OPRRd),NULL},2116037632,4280351744},
    {aarch64_op_uqsub_advsimd,"uqsub",{fn(setSIMDMode),fn(OPRQ),fn(OPRsize<23,22>),fn(OPRRm),fn(OPRRn),fn(OPRRd),NULL},773860352,3206609920},
    {aarch64_op_uqxtn_advsimd,"uqxtn",{fn(setSIMDMode),fn(OPRsize<23,22>),fn(OPRRn),fn(OPRRd),NULL},2116110336,4282383360},
    {aarch64_op_uqxtn_advsimd,"uqxtn",{fn(setSIMDMode),fn(OPRQ),fn(OPRsize<23,22>),fn(OPRRn),fn(OPRRd),NULL},773933056,3208641536},
    {aarch64_op_urecpe_advsimd,"urecpe",{fn(setSIMDMode),fn(OPRQ),fn(OPRsz<22,22>),fn(OPRRn),fn(OPRRd),NULL},245483520,3217030144},
    {aarch64_op_urhadd_advsimd,"urhadd",{fn(setSIMDMode),fn(OPRQ),fn(OPRsize<23,22>),fn(OPRRm),fn(OPRRn),fn(OPRRd),NULL},773854208,3206609920},
    {aarch64_op_urshl_advsimd,"urshl",{fn(setSIMDMode),fn(OPRsize<23,22>),fn(OPRRm),fn(OPRRn),fn(OPRRd),NULL},2116047872,4280351744},
    {aarch64_op_urshl_advsimd,"urshl",{fn(setSIMDMode),fn(OPRQ),fn(OPRsize<23,22>),fn(OPRRm),fn(OPRRn),fn(OPRRd),NULL},773870592,3206609920},
    {aarch64_op_urshr_advsimd,"urshr",{fn(setSIMDMode),fn(OPRimm<22,19>),fn(OPRimm<18,16>),fn(OPRRn),fn(OPRRd),NULL},2130715648,4286643200},
    {aarch64_op_urshr_advsimd,"urshr",{fn(setSIMDMode),fn(OPRQ),fn(OPRimm<22,19>),fn(OPRimm<18,16>),fn(OPRRn),fn(OPRRd),NULL},788538368,3212901376},
    {aarch64_op_ursqrte_advsimd,"ursqrte",{fn(setSIMDMode),fn(OPRQ),fn(OPRsz<22,22>),fn(OPRRn),fn(OPRRd),NULL},782354432,3217030144},
    {aarch64_op_ursra_advsimd,"ursra",{fn(setSIMDMode),fn(OPRimm<22,19>),fn(OPRimm<18,16>),fn(OPRRn),fn(OPRRd),NULL},2130719744,4286643200},
    {aarch64_op_ursra_advsimd,"ursra",{fn(setSIMDMode),fn(OPRQ),fn(OPRimm<22,19>),fn(OPRimm<18,16>),fn(OPRRn),fn(OPRRd),NULL},788542464,3212901376},
    {aarch64_op_ushl_advsimd,"ushl",{fn(setSIMDMode),fn(OPRsize<23,22>),fn(OPRRm),fn(OPRRn),fn(OPRRd),NULL},2116043776,4280351744},
    {aarch64_op_ushl_advsimd,"ushl",{fn(setSIMDMode),fn(OPRQ),fn(OPRsize<23,22>),fn(OPRRm),fn(OPRRn),fn(OPRRd),NULL},773866496,3206609920},
    {aarch64_op_ushll_advsimd,"ushll",{fn(setSIMDMode),fn(OPRQ),fn(OPRimm<22,19>),fn(OPRimm<18,16>),fn(OPRRn),fn(OPRRd),NULL},788571136,3212901376},
    {aarch64_op_ushr_advsimd,"ushr",{fn(setSIMDMode),fn(OPRimm<22,19>),fn(OPRimm<18,16>),fn(OPRRn),fn(OPRRd),NULL},2130707456,4286643200},
    {aarch64_op_ushr_advsimd,"ushr",{fn(setSIMDMode),fn(OPRQ),fn(OPRimm<22,19>),fn(OPRimm<18,16>),fn(OPRRn),fn(OPRRd),NULL},788530176,3212901376},
    {aarch64_op_usqadd_advsimd,"usqadd",{fn(setSIMDMode),fn(OPRsize<23,22>),fn(OPRRn),fn(OPRRd),NULL},2116040704,4282383360},
    {aarch64_op_usqadd_advsimd,"usqadd",{fn(setSIMDMode),fn(OPRQ),fn(OPRsize<23,22>),fn(OPRRn),fn(OPRRd),NULL},773863424,3208641536},
    {aarch64_op_usra_advsimd,"usra",{fn(setSIMDMode),fn(OPRimm<22,19>),fn(OPRimm<18,16>),fn(OPRRn),fn(OPRRd),NULL},2130711552,4286643200},
    {aarch64_op_usra_advsimd,"usra",{fn(setSIMDMode),fn(OPRQ),fn(OPRimm<22,19>),fn(OPRimm<18,16>),fn(OPRRn),fn(OPRRd),NULL},788534272,3212901376},
    {aarch64_op_usubl_advsimd,"usubl",{fn(setSIMDMode),fn(OPRQ),fn(OPRsize<23,22>),fn(OPRRm),fn(OPRRn),fn(OPRRd),NULL},773857280,3206609920},
    {aarch64_op_usubw_advsimd,"usubw",{fn(setSIMDMode),fn(OPRQ),fn(OPRsize<23,22>),fn(OPRRm),fn(OPRRn),fn(OPRRd),NULL},773861376,3206609920},
    {aarch64_op_uxtb_ubfm,"uxtb",{fn(OPRRn),fn(OPRRd),NULL},1392516096,4294966272},
    {aarch64_op_uxth_ubfm,"uxth",{fn(OPRRn),fn(OPRRd),NULL},1392524288,4294966272},
    {aarch64_op_uxtl_ushll_advsimd,"uxtl",{fn(setSIMDMode),fn(OPRQ),fn(OPRimm<22,19>),fn(OPRRn),fn(OPRRd),NULL},788571136,3213360128},
    {aarch64_op_uzp1_advsimd,"uzp1",{fn(setSIMDMode),fn(OPRQ),fn(OPRsize<23,22>),fn(OPRRm),fn(OPRRn),fn(OPRRd),NULL},234887168,3206609920},
    {aarch64_op_uzp2_advsimd,"uzp2",{fn(setSIMDMode),fn(OPRQ),fn(OPRsize<23,22>),fn(OPRRm),fn(OPRRn),fn(OPRRd),NULL},234903552,3206609920},
    {aarch64_op_wfe_hint,"wfe",{NULL},3573751903,4294967295},
    {aarch64_op_wfi_hint,"wfi",{NULL},3573751935,4294967295},
    {aarch64_op_xtn_advsimd,"xtn",{fn(setSIMDMode),fn(OPRQ),fn(OPRsize<23,22>),fn(OPRRn),fn(OPRRd),NULL},237053952,3208641536},
    {aarch64_op_yield_hint,"yield",{NULL},3573751871,4294967295},
    {aarch64_op_zip1_advsimd,"zip1",{fn(setSIMDMode),fn(OPRQ),fn(OPRsize<23,22>),fn(OPRRm),fn(OPRRn),fn(OPRRd),NULL},234895360,3206609920},
    {aarch64_op_zip2_advsimd,"zip2",{fn(setSIMDMode),fn(OPRQ),fn(OPRsize<23,22>),fn(OPRRm),fn(OPRRn),fn(OPRRd),NULL},234911744,3206609920},
};  // end main_insn_table

const aarch64_decoder_table aarch64_mask_entry::main_decoder_table = {
	{0x18000000, {{0,1},{1,2},{2,3},{3,4},{0,0}}, -1},
	{0x0, {{0,0}}, 0},
	{0x7000000, {{0,5},{1,6},{2,7},{3,8},{4,9},{5,10},{6,11},{7,12},{0,0}}, -1},
	{0x4000000, {{0,430},{1,431},{0,0}}, -1},
	{0x27000000, {{0,494},{2,495},{3,496},{4,497},{6,498},{7,499},{8,500},{9,501},{10,502},{12,503},{13,504},{14,505},{15,506},{0,0}}, -1},
	{0x20c00000, {{0,13},{1,14},{2,15},{3,16},{4,17},{5,18},{6,19},{7,20},{0,0}}, -1},
	{0x60c00000, {{4,51},{5,52},{6,53},{7,54},{13,55},{15,56},{0,0}}, -1},
	{0x60200000, {{0,57},{1,58},{2,59},{3,60},{4,61},{5,62},{6,63},{7,64},{0,0}}, -1},
	{0x60200000, {{0,65},{1,66},{2,67},{3,68},{4,69},{5,70},{6,71},{7,72},{0,0}}, -1},
	{0x20c00000, {{0,75},{1,76},{2,77},{3,78},{4,79},{5,80},{6,81},{7,82},{0,0}}, -1},
	{0x20c00000, {{0,103},{1,104},{2,105},{3,106},{4,107},{5,108},{6,109},{7,110},{0,0}}, -1},
	{0xa0208400, {{0,127},{1,128},{4,129},{5,130},{6,131},{7,132},{8,133},{9,134},{12,135},{13,136},{14,137},{15,138},{0,0}}, -1},
	{0x80000400, {{0,379},{1,380},{0,0}}, -1},
	{0x80208000, {{0,21},{1,22},{4,23},{5,24},{6,25},{7,26},{0,0}}, -1},
	{0x803f8000, {{62,31},{63,32},{190,33},{191,34},{254,35},{255,36},{0,0}}, -1},
	{0x803ffc00, {{2047,41},{6143,42},{0,0}}, -1},
	{0x803ffc00, {{2047,45},{6143,46},{0,0}}, -1},
	{0x0, {{0,0}}, 582},
	{0x0, {{0,0}}, 303},
	{0x0, {{0,0}}, 586},
	{0x40000000, {{0,49},{1,50},{0,0}}, -1},
	{0x40007c00, {{31,27},{63,28},{0,0}}, -1},
	{0x40007c00, {{31,29},{63,30},{0,0}}, -1},
	{0x0, {{0,0}}, 613},
	{0x0, {{0,0}}, 578},
	{0x0, {{0,0}}, 612},
	{0x0, {{0,0}}, 577},
	{0x0, {{0,0}}, 614},
	{0x0, {{0,0}}, 615},
	{0x0, {{0,0}}, 579},
	{0x0, {{0,0}}, 580},
	{0x40007c00, {{31,37},{63,38},{0,0}}, -1},
	{0x40007c00, {{31,39},{63,40},{0,0}}, -1},
	{0x0, {{0,0}}, 358},
	{0x0, {{0,0}}, 299},
	{0x0, {{0,0}}, 357},
	{0x0, {{0,0}}, 298},
	{0x0, {{0,0}}, 359},
	{0x0, {{0,0}}, 360},
	{0x0, {{0,0}}, 300},
	{0x0, {{0,0}}, 301},
	{0x40000000, {{0,43},{1,44},{0,0}}, -1},
	{0x0, {{0,0}}, 574},
	{0x0, {{0,0}}, 575},
	{0x0, {{0,0}}, 576},
	{0x40000000, {{0,47},{1,48},{0,0}}, -1},
	{0x0, {{0,0}}, 295},
	{0x0, {{0,0}}, 296},
	{0x0, {{0,0}}, 297},
	{0x0, {{0,0}}, 307},
	{0x0, {{0,0}}, 310},
	{0x0, {{0,0}}, 588},
	{0x0, {{0,0}}, 309},
	{0x0, {{0,0}}, 587},
	{0x0, {{0,0}}, 308},
	{0x0, {{0,0}}, 312},
	{0x0, {{0,0}}, 311},
	{0x0, {{0,0}}, 25},
	{0x0, {{0,0}}, 39},
	{0x0, {{0,0}}, 410},
	{0x0, {{0,0}}, 406},
	{0x0, {{0,0}}, 112},
	{0x0, {{0,0}}, 109},
	{0x0, {{0,0}}, 27},
	{0x0, {{0,0}}, 40},
	{0x0, {{0,0}}, 7},
	{0x0, {{0,0}}, 5},
	{0x0, {{0,0}}, 15},
	{0xc00000, {{0,73},{0,0}}, -1},
	{0x0, {{0,0}}, 618},
	{0x0, {{0,0}}, 616},
	{0x0, {{0,0}}, 624},
	{0xc00000, {{0,74},{0,0}}, -1},
	{0x0, {{0,0}}, 13},
	{0x0, {{0,0}}, 622},
	{0x803f2000, {{0,83},{1,84},{0,0}}, -1},
	{0x803f2000, {{0,88},{1,89},{0,0}}, -1},
	{0x80202000, {{0,93},{1,94},{0,0}}, -1},
	{0x80202000, {{0,98},{1,99},{0,0}}, -1},
	{0x0, {{0,0}}, 581},
	{0x0, {{0,0}}, 302},
	{0x0, {{0,0}}, 583},
	{0x0, {{0,0}}, 304},
	{0xd000, {{0,85},{2,86},{4,87},{0,0}}, -1},
	{0x0, {{0,0}}, 558},
	{0x0, {{0,0}}, 570},
	{0x0, {{0,0}}, 566},
	{0x0, {{0,0}}, 562},
	{0xd000, {{0,90},{2,91},{4,92},{0,0}}, -1},
	{0x0, {{0,0}}, 271},
	{0x0, {{0,0}}, 289},
	{0x0, {{0,0}}, 283},
	{0x0, {{0,0}}, 277},
	{0xd000, {{0,95},{2,96},{4,97},{0,0}}, -1},
	{0x0, {{0,0}}, 559},
	{0x0, {{0,0}}, 571},
	{0x0, {{0,0}}, 567},
	{0x0, {{0,0}}, 563},
	{0xd000, {{0,100},{2,101},{4,102},{0,0}}, -1},
	{0x0, {{0,0}}, 272},
	{0x0, {{0,0}}, 290},
	{0x0, {{0,0}}, 284},
	{0x0, {{0,0}}, 278},
	{0x803f2000, {{0,111},{1,112},{64,113},{65,114},{0,0}}, -1},
	{0x803f2000, {{0,115},{1,116},{64,117},{65,118},{0,0}}, -1},
	{0x80202000, {{0,119},{1,120},{2,121},{3,122},{0,0}}, -1},
	{0x80202000, {{0,123},{1,124},{2,125},{3,126},{0,0}}, -1},
	{0x0, {{0,0}}, 585},
	{0x0, {{0,0}}, 306},
	{0x0, {{0,0}}, 584},
	{0x0, {{0,0}}, 305},
	{0x0, {{0,0}}, 560},
	{0x0, {{0,0}}, 568},
	{0x0, {{0,0}}, 564},
	{0x0, {{0,0}}, 572},
	{0xC000, {{0,860},{1,860},{2,860},{3,861},{0,0}}, -1},
	{0xC000, {{0,868},{1,868},{2,868},{3,869},{0,0}}, -1},
	{0xC000, {{0,864},{1,864},{2,864},{3,865},{0,0}}, -1},
	{0xC000, {{0,872},{1,872},{2,872},{3,873},{0,0}}, -1},
	{0x0, {{0,0}}, 561},
	{0x0, {{0,0}}, 569},
	{0x0, {{0,0}}, 565},
	{0x0, {{0,0}}, 573},
	{0xC000, {{0,862},{1,862},{2,862},{3,863},{0,0}}, -1},
	{0xC000, {{0,870},{1,870},{2,870},{3,871},{0,0}}, -1},
	{0xC000, {{0,866},{1,866},{2,866},{3,867},{0,0}}, -1},
	{0xC000, {{0,874},{1,874},{2,874},{3,875},{0,0}}, -1},
	{0x1800, {{0,139},{1,140},{2,141},{3,142},{0,0}}, -1},
	{0xc07800, {{0,149},{1,150},{3,151},{5,152},{7,153},{0,0}}, -1},
	{0x7800, {{0,155},{1,156},{2,157},{3,158},{4,159},{5,160},{6,161},{7,162},{8,163},{9,164},{10,165},{11,166},{12,167},{13,168},{14,169},{15,170},{0,0}}, -1},
	{0x7800, {{0,186},{1,187},{2,188},{3,189},{4,190},{5,191},{6,192},{7,193},{8,194},{9,195},{10,196},{11,197},{12,198},{13,199},{14,200},{15,201},{0,0}}, -1},
	{0x7800, {{0,206},{1,207},{2,208},{3,209},{4,210},{5,211},{6,212},{7,213},{8,214},{9,215},{10,216},{11,217},{12,218},{13,219},{15,220},{0,0}}, -1},
	{0x7800, {{0,246},{1,247},{2,248},{3,249},{4,250},{5,251},{6,252},{7,253},{8,254},{9,255},{10,256},{11,257},{12,258},{14,259},{15,260},{0,0}}, -1},
	{0x0, {{0,0}}, 114},
	{0x40c00000, {{4,271},{0,0}}, -1},
	{0x7800, {{0,272},{1,273},{2,274},{4,275},{5,276},{6,277},{7,278},{8,279},{9,280},{10,281},{11,282},{12,283},{13,284},{14,285},{15,286},{0,0}}, -1},
	{0x7800, {{0,298},{1,299},{2,300},{3,301},{4,302},{5,303},{6,304},{7,305},{8,306},{9,307},{10,308},{11,309},{12,310},{13,311},{14,312},{15,313},{0,0}}, -1},
	{0x7800, {{0,318},{1,319},{3,320},{4,321},{5,322},{7,323},{8,324},{9,325},{11,326},{15,327},{0,0}}, -1},
	{0x7800, {{0,355},{1,356},{2,357},{3,358},{4,359},{5,360},{6,361},{8,362},{10,363},{11,364},{12,365},{13,366},{14,367},{15,368},{0,0}}, -1},
	{0x0, {{0,0}}, 634},
	{0x6000, {{1,143},{3,144},{0,0}}, -1},
	{0x0, {{0,0}}, 636},
	{0x6000, {{0,145},{1,146},{2,147},{3,148},{0,0}}, -1},
	{0x0, {{0,0}}, 639},
	{0x0, {{0,0}}, 640},
	{0x0, {{0,0}}, 721},
	{0x0, {{0,0}}, 727},
	{0x0, {{0,0}}, 722},
	{0x0, {{0,0}}, 728},
	{0x0, {{0,0}}, 107},
	{0x0, {{0,0}}, 108},
	{0x40000000, {{1,154},{0,0}}, -1},
	{0x0, {{0,0}}, 486},
	{0x0, {{0,0}}, 382},
	{0x0, {{0,0}}, 269},
	{0x0, {{0,0}}, 437},
	{0x0, {{0,0}}, 426},
	{0x0, {{0,0}}, 440},
	{0x0, {{0,0}}, 422},
	{0x0, {{0,0}}, 556},
	{0x1f0000, {{0,171},{1,172},{0,0}}, -1},
	{0x0, {{0,0}}, 557},
	{0x1f0000, {{0,173},{16,174},{0,0}}, -1},
	{0x0, {{0,0}}, 10},
	{0x1f0000, {{0,175},{1,176},{8,177},{0,0}}, -1},
	{0x0, {{0,0}}, 433},
	{0x1f0000, {{0,178},{8,179},{0,0}}, -1},
	{0x0, {{0,0}}, 621},
	{0x1f0000, {{0,180},{1,181},{8,182},{0,0}}, -1},
	{0x0, {{0,0}}, 435},
	{0x1f0000, {{0,183},{1,184},{8,185},{0,0}}, -1},
	{0x0, {{0,0}}, 438},
	{0x0, {{0,0}}, 725},
	{0x0, {{0,0}}, 626},
	{0x0, {{0,0}}, 439},
	{0x0, {{0,0}}, 57},
	{0x0, {{0,0}}, 537},
	{0x0, {{0,0}}, 20},
	{0x0, {{0,0}}, 90},
	{0x0, {{0,0}}, 19},
	{0x0, {{0,0}}, 436},
	{0x0, {{0,0}}, 163},
	{0x0, {{0,0}}, 22},
	{0x0, {{0,0}}, 493},
	{0x0, {{0,0}}, 156},
	{0x0, {{0,0}}, 21},
	{0x0, {{0,0}}, 465},
	{0x0, {{0,0}}, 495},
	{0x0, {{0,0}}, 540},
	{0xc00000, {{0,202},{1,203},{2,204},{3,205},{0,0}}, -1},
	{0x0, {{0,0}}, 470},
	{0x0, {{0,0}}, 535},
	{0x0, {{0,0}}, 70},
	{0x0, {{0,0}}, 66},
	{0x0, {{0,0}}, 550},
	{0x0, {{0,0}}, 527},
	{0x0, {{0,0}}, 544},
	{0x0, {{0,0}}, 519},
	{0x0, {{0,0}}, 474},
	{0x0, {{0,0}}, 478},
	{0x0, {{0,0}}, 434},
	{0x0, {{0,0}}, 432},
	{0x0, {{0,0}}, 23},
	{0x0, {{0,0}}, 38},
	{0x0, {{0,0}}, 379},
	{0x0, {{0,0}}, 405},
	{0x0, {{0,0}}, 482},
	{0x1f0000, {{0,221},{1,222},{0,0}}, -1},
	{0x0, {{0,0}}, 499},
	{0x1f0000, {{0,225},{1,226},{0,0}}, -1},
	{0x0, {{0,0}}, 484},
	{0x1f0000, {{0,229},{1,230},{16,231},{17,232},{0,0}}, -1},
	{0x0, {{0,0}}, 503},
	{0x1f0000, {{0,235},{1,236},{17,237},{0,0}}, -1},
	{0x0, {{0,0}}, 490},
	{0x9f0000, {{1,240},{32,241},{33,242},{0,0}}, -1},
	{0x0, {{0,0}}, 511},
	{0x9f0000, {{1,243},{32,244},{33,245},{0,0}}, -1},
	{0x0, {{0,0}}, 412},
	{0x0, {{0,0}}, 145},
	{0x0, {{0,0}}, 118},
	{0x0, {{0,0}}, 72},
	{0x800000, {{0,223},{1,224},{0,0}}, -1},
	{0x0, {{0,0}}, 248},
	{0x0, {{0,0}}, 250},
	{0x0, {{0,0}}, 64},
	{0x800000, {{0,227},{1,228},{0,0}}, -1},
	{0x0, {{0,0}}, 246},
	{0x0, {{0,0}}, 254},
	{0x0, {{0,0}}, 80},
	{0x800000, {{0,233},{1,234},{0,0}}, -1},
	{0x0, {{0,0}}, 476},
	{0x0, {{0,0}}, 480},
	{0x0, {{0,0}}, 165},
	{0x0, {{0,0}}, 171},
	{0x0, {{0,0}}, 2},
	{0x800000, {{0,238},{1,239},{0,0}}, -1},
	{0x0, {{0,0}}, 16},
	{0x0, {{0,0}}, 158},
	{0x0, {{0,0}}, 181},
	{0x0, {{0,0}}, 151},
	{0x0, {{0,0}}, 141},
	{0x0, {{0,0}}, 698},
	{0x0, {{0,0}}, 449},
	{0x0, {{0,0}}, 133},
	{0x0, {{0,0}}, 238},
	{0x0, {{0,0}}, 9},
	{0x0, {{0,0}}, 88},
	{0x0, {{0,0}}, 369},
	{0x0, {{0,0}}, 392},
	{0x0, {{0,0}}, 475},
	{0x0, {{0,0}}, 479},
	{0x0, {{0,0}}, 507},
	{0x0, {{0,0}}, 12},
	{0x800000, {{0,261},{1,262},{0,0}}, -1},
	{0x800000, {{0,263},{1,264},{0,0}}, -1},
	{0x800000, {{0,265},{1,266},{0,0}}, -1},
	{0x0, {{0,0}}, 231},
	{0x0, {{0,0}}, 131},
	{0x800000, {{0,267},{1,268},{0,0}}, -1},
	{0x800000, {{0,269},{1,270},{0,0}}, -1},
	{0x0, {{0,0}}, 195},
	{0x0, {{0,0}}, 205},
	{0x0, {{0,0}}, 215},
	{0x0, {{0,0}}, 218},
	{0x0, {{0,0}}, 124},
	{0x0, {{0,0}}, 262},
	{0x0, {{0,0}}, 193},
	{0x0, {{0,0}}, 203},
	{0x0, {{0,0}}, 240},
	{0x0, {{0,0}}, 259},
	{0x0, {{0,0}}, 268},
	{0x0, {{0,0}}, 648},
	{0x0, {{0,0}}, 424},
	{0x0, {{0,0}}, 651},
	{0x0, {{0,0}}, 716},
	{0x1f0000, {{0,287},{1,288},{0,0}}, -1},
	{0x0, {{0,0}}, 717},
	{0x1f0000, {{0,289},{1,290},{16,291},{0,0}}, -1},
	{0x0, {{0,0}}, 417},
	{0x1f0000, {{0,292},{1,293},{0,0}}, -1},
	{0x0, {{0,0}}, 644},
	{0xdf0000, {{0,294},{32,295},{0,0}}, -1},
	{0x0, {{0,0}}, 431},
	{0x1f0000, {{0,296},{1,297},{0,0}}, -1},
	{0x0, {{0,0}}, 646},
	{0x0, {{0,0}}, 513},
	{0x0, {{0,0}}, 649},
	{0x0, {{0,0}}, 539},
	{0x0, {{0,0}}, 713},
	{0x0, {{0,0}}, 468},
	{0x0, {{0,0}}, 650},
	{0x0, {{0,0}}, 59},
	{0x0, {{0,0}}, 697},
	{0x0, {{0,0}}, 394},
	{0x0, {{0,0}}, 418},
	{0x0, {{0,0}}, 647},
	{0x0, {{0,0}}, 177},
	{0x0, {{0,0}}, 662},
	{0x0, {{0,0}}, 683},
	{0x0, {{0,0}}, 699},
	{0xc00000, {{0,314},{1,315},{2,316},{3,317},{0,0}}, -1},
	{0x0, {{0,0}}, 663},
	{0x0, {{0,0}}, 695},
	{0x0, {{0,0}}, 74},
	{0x0, {{0,0}}, 76},
	{0x0, {{0,0}}, 708},
	{0x0, {{0,0}}, 691},
	{0x0, {{0,0}}, 701},
	{0x0, {{0,0}}, 685},
	{0x0, {{0,0}}, 665},
	{0x0, {{0,0}}, 668},
	{0x0, {{0,0}}, 645},
	{0x0, {{0,0}}, 643},
	{0x0, {{0,0}}, 110},
	{0x0, {{0,0}}, 47},
	{0x0, {{0,0}}, 42},
	{0x0, {{0,0}}, 41},
	{0x0, {{0,0}}, 672},
	{0x1f0000, {{0,328},{1,329},{0,0}}, -1},
	{0x1f0000, {{0,330},{1,331},{0,0}}, -1},
	{0x0, {{0,0}}, 674},
	{0x1f0000, {{1,334},{16,335},{17,336},{0,0}}, -1},
	{0x1f0000, {{0,339},{1,340},{0,0}}, -1},
	{0x0, {{0,0}}, 680},
	{0x9f0000, {{1,343},{16,344},{32,345},{33,346},{48,347},{0,0}}, -1},
	{0x9f0000, {{1,348},{32,349},{33,350},{0,0}}, -1},
	{0x9f0000, {{16,351},{32,352},{33,353},{48,354},{0,0}}, -1},
	{0x0, {{0,0}}, 68},
	{0x0, {{0,0}}, 242},
	{0x0, {{0,0}}, 78},
	{0x800000, {{0,332},{1,333},{0,0}}, -1},
	{0x0, {{0,0}}, 252},
	{0x0, {{0,0}}, 244},
	{0x800000, {{0,337},{1,338},{0,0}}, -1},
	{0x0, {{0,0}}, 667},
	{0x0, {{0,0}}, 670},
	{0x0, {{0,0}}, 168},
	{0x0, {{0,0}}, 174},
	{0x0, {{0,0}}, 398},
	{0x800000, {{0,341},{1,342},{0,0}}, -1},
	{0x0, {{0,0}}, 161},
	{0x0, {{0,0}}, 187},
	{0x0, {{0,0}}, 154},
	{0x0, {{0,0}}, 199},
	{0x0, {{0,0}}, 137},
	{0x0, {{0,0}}, 704},
	{0x0, {{0,0}}, 209},
	{0x0, {{0,0}}, 658},
	{0x0, {{0,0}}, 143},
	{0x0, {{0,0}}, 257},
	{0x0, {{0,0}}, 202},
	{0x0, {{0,0}}, 232},
	{0x0, {{0,0}}, 260},
	{0x0, {{0,0}}, 212},
	{0x0, {{0,0}}, 620},
	{0x0, {{0,0}}, 62},
	{0x0, {{0,0}}, 371},
	{0x0, {{0,0}}, 411},
	{0x0, {{0,0}}, 666},
	{0x0, {{0,0}}, 669},
	{0x0, {{0,0}}, 517},
	{0x800000, {{0,369},{1,370},{0,0}}, -1},
	{0x800000, {{0,371},{1,372},{0,0}}, -1},
	{0x0, {{0,0}}, 226},
	{0x800000, {{0,373},{1,374},{0,0}}, -1},
	{0x800000, {{0,375},{1,376},{0,0}}, -1},
	{0x800000, {{0,377},{1,378},{0,0}}, -1},
	{0x0, {{0,0}}, 190},
	{0x0, {{0,0}}, 198},
	{0x0, {{0,0}}, 208},
	{0x0, {{0,0}}, 127},
	{0x0, {{0,0}}, 117},
	{0x0, {{0,0}}, 135},
	{0x0, {{0,0}}, 139},
	{0x0, {{0,0}}, 121},
	{0x0, {{0,0}}, 123},
	{0x0, {{0,0}}, 201},
	{0x0, {{0,0}}, 211},
	{0x2000f000, {{1,381},{2,382},{3,383},{5,384},{6,385},{7,386},{8,387},{9,388},{10,389},{11,390},{12,391},{13,392},{16,393},{18,394},{20,395},{22,396},{25,397},{26,398},{0,0}}, -1},
	{0xf80800, {{0,399},{2,400},{4,400},{6,400},{8,400},{10,400},{12,400},{14,400},{16,400},{18,400},{20,400},{22,400},{24,400},{26,400},{28,400},{30,400},{3,401},{5,401},{7,401},{9,401},{11,401},{13,401},{15,401},{17,401},{19,401},{21,401},{23,401},{25,401},{27,401},{29,401},{31,401},{0,0}}, -1},
	{0x0, {{0,0}}, 214},
	{0x0, {{0,0}}, 481},
	{0x0, {{0,0}}, 497},
	{0x0, {{0,0}}, 217},
	{0x0, {{0,0}}, 483},
	{0x0, {{0,0}}, 501},
	{0x0, {{0,0}}, 391},
	{0x0, {{0,0}}, 225},
	{0x0, {{0,0}}, 489},
	{0x0, {{0,0}}, 509},
	{0x0, {{0,0}}, 505},
	{0x0, {{0,0}}, 515},
	{0x0, {{0,0}}, 368},
	{0x0, {{0,0}}, 671},
	{0x0, {{0,0}}, 370},
	{0x0, {{0,0}}, 673},
	{0x0, {{0,0}}, 229},
	{0x0, {{0,0}}, 679},
	{0x0000f000, {{0,853},{2,853},{4,853},{6,853},{8,853},{10,853},{12,853},{13,853},{14,855},{1,854},{3,854},{5,854},{7,854},{9,854},{11,854},{15,856},{0,0}}, -1},
	{0x2000f000, {{0,402},{1,403},{2,404},{3,405},{5,406},{7,407},{8,408},{9,409},{10,410},{14,411},{16,412},{17,413},{18,414},{19,415},{20,416},{21,417},{22,418},{23,419},{24,420},{25,421},{26,422},{30,423},{0,0}}, -1},
	{0x2000f000, {{8,424},{9,425},{15,426},{24,427},{25,428},{31,429},{0,0}}, -1},
	{0x0, {{0,0}}, 553},
	{0x0, {{0,0}}, 555},
	{0x0, {{0,0}}, 546},
	{0x0, {{0,0}}, 548},
	{0x0, {{0,0}}, 467},
	{0x0, {{0,0}}, 525},
	{0x0, {{0,0}}, 469},
	{0x0, {{0,0}}, 531},
	{0x0, {{0,0}}, 551},
	{0x0, {{0,0}}, 447},
	{0x0, {{0,0}}, 711},
	{0x0, {{0,0}}, 715},
	{0x0, {{0,0}}, 703},
	{0x0, {{0,0}}, 706},
	{0x0, {{0,0}}, 542},
	{0x0, {{0,0}}, 472},
	{0x0, {{0,0}}, 529},
	{0x0, {{0,0}}, 689},
	{0x0, {{0,0}}, 533},
	{0x0, {{0,0}}, 693},
	{0x0, {{0,0}}, 709},
	{0x0, {{0,0}}, 656},
	{0x0, {{0,0}}, 430},
	{0x0, {{0,0}}, 521},
	{0x0, {{0,0}}, 179},
	{0x0, {{0,0}}, 523},
	{0x0, {{0,0}}, 687},
	{0x0, {{0,0}}, 185},
	{0x3000000, {{0,432},{1,433},{2,434},{3,435},{0,0}}, -1},
	{0x60000000, {{0,454},{1,455},{2,456},{0,0}}, -1},
	{0x80000000, {{0,436},{1,437},{0,0}}, -1},
	{0x60000000, {{0,438},{1,439},{2,440},{3,441},{0,0}}, -1},
	{0x60800000, {{0,442},{1,443},{2,444},{4,445},{5,446},{6,447},{7,448},{0,0}}, -1},
	{0x60800000, {{0,449},{1,450},{2,451},{4,452},{0,0}}, -1},
	{0x0, {{0,0}}, 17},
	{0x0, {{0,0}}, 18},
	{0x0, {{0,0}}, 6},
	{0x0, {{0,0}}, 14},
	{0x0, {{0,0}}, 617},
	{0x0, {{0,0}}, 623},
	{0x0, {{0,0}}, 24},
	{0x0, {{0,0}}, 377},
	{0x0, {{0,0}}, 409},
	{0x0, {{0,0}}, 111},
	{0x0, {{0,0}}, 378},
	{0x0, {{0,0}}, 26},
	{0x0, {{0,0}}, 384},
	{0x0, {{0,0}}, 443},
	{0x200000, {{0,453},{0,0}}, -1},
	{0x0, {{0,0}}, 34},
	{0x0, {{0,0}}, 362},
	{0x0, {{0,0}}, 115},
	{0x80000000, {{0,457},{1,458},{0,0}}, -1},
	{0x3000000, {{0,459},{1,460},{2,461},{3,462},{0,0}}, -1},
	{0x83000000, {{0,463},{4,464},{5,465},{6,466},{0,0}}, -1},
	{0x0, {{0,0}}, 33},
	{0x0, {{0,0}}, 43},
	{0x0, {{0,0}}, 49},
	{0x0, {{0,0}}, 48},
	{0x0, {{0,0}}, 637},
	{0x0, {{0,0}}, 635},
	{0x0, {{0,0}}, 32},
	{0xe0001f, {{1,467},{2,468},{3,469},{32,470},{64,471},{161,472},{162,473},{163,474},{0,0}}, -1},
	{0xf00000, {{0,475},{1,476},{2,477},{3,478},{0,0}}, -1},
	{0xfffc1f, {{63488,489},{129024,490},{194560,491},{325632,492},{391168,493},{0,0}}, -1},
	{0x0, {{0,0}}, 627},
	{0x0, {{0,0}}, 266},
	{0x0, {{0,0}}, 477},
	{0x0, {{0,0}}, 46},
	{0x0, {{0,0}}, 265},
	{0x0, {{0,0}}, 100},
	{0x0, {{0,0}}, 101},
	{0x0, {{0,0}}, 102},
	{0x80000, {{0,479},{1,480},{0,0}}, -1},
	{0x0, {{0,0}}, 389},
	{0x0, {{0,0}}, 633},
	{0x0, {{0,0}}, 387},
	{0xf01f, {{95,481},{127,482},{159,483},{0,0}}, -1},
	{0x0, {{0,0}}, 632},
	{0x70000, {{3,484},{0,0}}, -1},
	{0x700e0, {{26,485},{28,486},{29,487},{30,488},{0,0}}, -1},
	{0x0, {{0,0}}, 388},
	{0x0, {{0,0}}, 264},
	{0x0, {{0,0}}, 56},
	{0x0, {{0,0}}, 105},
	{0x0, {{0,0}}, 103},
	{0x0, {{0,0}}, 270},
	{0x0, {{0,0}}, 45},
	{0x0, {{0,0}}, 44},
	{0x0, {{0,0}}, 420},
	{0x0, {{0,0}}, 113},
	{0x0, {{0,0}}, 104},
	{0x80000000, {{0,507},{1,508},{0,0}}, -1},
	{0x40e00000, {{0,511},{4,512},{6,513},{8,514},{12,515},{14,516},{0,0}}, -1},
	{0x40e08000, {{0,539},{1,540},{2,541},{3,542},{4,543},{10,544},{11,545},{12,546},{0,0}}, -1},
	{0x0, {{0,0}}, 319},
	{0x40200000, {{0,551},{1,552},{2,553},{3,554},{0,0}}, -1},
	{0xc0008000, {{0,668},{1,669},{2,670},{3,671},{0,0}}, -1},
	{0x80a00c00, {{0,695},{1,696},{2,697},{3,698},{6,699},{8,700},{9,701},{10,702},{11,703},{14,704},{16,705},{17,706},{18,707},{19,708},{22,709},{24,710},{25,711},{26,712},{27,713},{30,714},{0,0}}, -1},
	{0x80800000, {{0,759},{1,760},{2,761},{3,762},{0,0}}, -1},
	{0x40e00c00, {{0,773},{8,774},{10,775},{32,776},{40,777},{42,778},{0,0}}, -1},
	{0x600c00, {{0,780},{1,781},{3,782},{6,783},{8,784},{9,785},{11,786},{14,787},{0,0}}, -1},
	{0x400000, {{0,788},{1,789},{0,0}}, -1},
	{0xc020fc00, {{195,790},{202,791},{203,792},{205,793},{206,794},{207,795},{209,796},{210,797},{211,798},{213,799},{215,800},{218,801},{222,802},{225,803},{226,804},{227,805},{230,806},{234,807},{237,808},{238,809},{242,810},{245,811},{246,812},{249,813},{251,814},{254,815},{0,0}}, -1},
	{0xc080f400, {{65,836},{67,837},{69,838},{71,839},{73,840},{75,841},{77,842},{79,843},{81,844},{83,845},{93,846},{95,847},{114,848},{0,0}}, -1},
	{0x0, {{0,0}}, 320},
	{0x40000000, {{0,509},{1,510},{0,0}}, -1},
	{0x0, {{0,0}}, 342},
	{0x0, {{0,0}}, 414},
	{0x0, {{0,0}}, 3},
	{0xc00, {{0,517},{1,518},{0,0}}, -1},
	{0xf000, {{0,519},{2,520},{4,521},{5,522},{0,0}}, -1},
	{0xfc00, {{0,529},{0,0}}, -1},
	{0xc00, {{0,530},{1,531},{0,0}}, -1},
	{0x1ff800, {{0,532},{1,533},{2,534},{0,0}}, -1},
	{0x0, {{0,0}}, 93},
	{0x0, {{0,0}}, 96},
	{0xc00, {{2,523},{3,524},{0,0}}, -1},
	{0xc00, {{0,525},{1,526},{2,527},{3,528},{0,0}}, -1},
	{0x0, {{0,0}}, 91},
	{0x0, {{0,0}}, 92},
	{0x0, {{0,0}}, 661},
	{0x0, {{0,0}}, 452},
	{0x0, {{0,0}}, 361},
	{0x0, {{0,0}}, 364},
	{0x0, {{0,0}}, 28},
	{0x0, {{0,0}}, 428},
	{0x0, {{0,0}}, 441},
	{0x0, {{0,0}}, 97},
	{0x0, {{0,0}}, 98},
	{0x400, {{0,535},{1,536},{0,0}}, -1},
	{0x0, {{0,0}}, 421},
	{0x400, {{0,537},{1,538},{0,0}}, -1},
	{0x0, {{0,0}}, 419},
	{0x0, {{0,0}}, 423},
	{0x0, {{0,0}}, 60},
	{0x0, {{0,0}}, 58},
	{0x0, {{0,0}}, 367},
	{0x0, {{0,0}}, 390},
	{0x80000000, {{1,547},{0,0}}, -1},
	{0x80000000, {{1,548},{0,0}}, -1},
	{0x0, {{0,0}}, 488},
	{0x80000000, {{1,549},{0,0}}, -1},
	{0x80000000, {{1,550},{0,0}}, -1},
	{0x0, {{0,0}}, 678},
	{0x0, {{0,0}}, 473},
	{0x0, {{0,0}}, 487},
	{0x0, {{0,0}}, 664},
	{0x0, {{0,0}}, 677},
	{0x9f0000, {{2,555},{3,556},{24,557},{25,558},{0,0}}, -1},
	{0xc00, {{0,559},{1,560},{2,561},{3,562},{0,0}}, -1},
	{0x80c0fc00, {{0,613},{1,614},{4,615},{8,616},{12,617},{16,618},{20,619},{24,620},{0,0}}, -1},
	{0x8000fc00, {{2,621},{3,622},{6,623},{10,624},{11,625},{13,626},{14,627},{15,628},{17,629},{18,630},{19,631},{21,632},{23,633},{30,634},{33,635},{34,636},{35,637},{36,638},{38,639},{42,640},{44,641},{45,642},{46,643},{50,644},{52,645},{54,646},{55,647},{57,648},{58,649},{62,650},{63,651},{0,0}}, -1},
	{0x0, {{0,0}}, 450},
	{0x0, {{0,0}}, 659},
	{0x0, {{0,0}}, 182},
	{0x0, {{0,0}}, 188},
	{0x1000, {{0,563},{1,564},{0,0}}, -1},
	{0x80800010, {{0,602},{1,603},{0,0}}, -1},
	{0x8080f000, {{0,604},{1,605},{2,606},{3,607},{4,608},{5,609},{6,610},{7,611},{8,612},{0,0}}, -1},
	{0x0, {{0,0}}, 148},
	{0x6000, {{0,565},{1,566},{2,567},{0,0}}, -1},
	{0x0, {{0,0}}, 222},
	{0x168000, {{0,568},{2,569},{4,570},{6,571},{8,572},{0,0}}, -1},
	{0x80808017, {{0,585},{8,586},{0,0}}, -1},
	{0x801e0000, {{0,587},{1,588},{2,589},{3,590},{0,0}}, -1},
	{0x890000, {{0,573},{1,574},{2,575},{3,576},{0,0}}, -1},
	{0x890000, {{0,577},{1,578},{0,0}}, -1},
	{0x890000, {{0,579},{1,580},{0,0}}, -1},
	{0x0, {{0,0}}, 221},
	{0x890000, {{0,581},{1,582},{2,583},{3,584},{0,0}}, -1},
	{0x0, {{0,0}}, 166},
	{0x0, {{0,0}}, 169},
	{0x0, {{0,0}}, 172},
	{0x0, {{0,0}}, 175},
	{0x0, {{0,0}}, 451},
	{0x0, {{0,0}}, 660},
	{0x0, {{0,0}}, 152},
	{0x0, {{0,0}}, 155},
	{0x0, {{0,0}}, 159},
	{0x0, {{0,0}}, 162},
	{0x0, {{0,0}}, 183},
	{0x0, {{0,0}}, 189},
	{0x0, {{0,0}}, 146},
	{0x0, {{0,0}}, 147},
	{0x818000, {{0,591},{1,592},{2,593},{3,594},{0,0}}, -1},
	{0x0, {{0,0}}, 149},
	{0x818000, {{0,595},{1,596},{2,597},{3,598},{0,0}}, -1},
	{0x818000, {{0,599},{2,600},{3,601},{0,0}}, -1},
	{0x0, {{0,0}}, 220},
	{0x0, {{0,0}}, 119},
	{0x0, {{0,0}}, 233},
	{0x0, {{0,0}}, 261},
	{0x0, {{0,0}}, 249},
	{0x0, {{0,0}}, 251},
	{0x0, {{0,0}}, 247},
	{0x0, {{0,0}}, 255},
	{0x0, {{0,0}}, 243},
	{0x0, {{0,0}}, 253},
	{0x0, {{0,0}}, 245},
	{0x0, {{0,0}}, 128},
	{0x0, {{0,0}}, 129},
	{0x0, {{0,0}}, 227},
	{0x0, {{0,0}}, 191},
	{0x0, {{0,0}}, 125},
	{0x0, {{0,0}}, 263},
	{0x0, {{0,0}}, 194},
	{0x0, {{0,0}}, 204},
	{0x0, {{0,0}}, 196},
	{0x0, {{0,0}}, 206},
	{0x0, {{0,0}}, 236},
	{0x0, {{0,0}}, 455},
	{0x0, {{0,0}}, 106},
	{0x0, {{0,0}}, 458},
	{0x0, {{0,0}}, 457},
	{0x0, {{0,0}}, 459},
	{0x0, {{0,0}}, 462},
	{0x0, {{0,0}}, 461},
	{0x0, {{0,0}}, 464},
	{0x0, {{0,0}}, 456},
	{0x0, {{0,0}}, 494},
	{0x0, {{0,0}}, 460},
	{0x0, {{0,0}}, 463},
	{0x0, {{0,0}}, 534},
	{0x0, {{0,0}}, 69},
	{0x0, {{0,0}}, 625},
	{0x0, {{0,0}}, 65},
	{0x0, {{0,0}}, 549},
	{0x0, {{0,0}}, 536},
	{0x0, {{0,0}}, 526},
	{0x0, {{0,0}}, 543},
	{0x0, {{0,0}}, 518},
	{0x0, {{0,0}}, 492},
	{0x0, {{0,0}}, 8},
	{0x0, {{0,0}}, 71},
	{0x0, {{0,0}}, 87},
	{0x0, {{0,0}}, 498},
	{0x0, {{0,0}}, 63},
	{0x1f0000, {{0,652},{1,653},{0,0}}, -1},
	{0x0, {{0,0}}, 502},
	{0x0, {{0,0}}, 506},
	{0x1f0000, {{0,656},{1,657},{17,658},{0,0}}, -1},
	{0x9f0000, {{1,661},{32,662},{0,0}}, -1},
	{0x0, {{0,0}}, 510},
	{0x9f0000, {{1,663},{32,664},{33,665},{0,0}}, -1},
	{0x0, {{0,0}}, 230},
	{0x0, {{0,0}}, 130},
	{0x0, {{0,0}}, 144},
	{0x0, {{0,0}}, 241},
	{0x800000, {{0,666},{1,667},{0,0}}, -1},
	{0x0, {{0,0}}, 79},
	{0x800000, {{0,654},{1,655},{0,0}}, -1},
	{0x0, {{0,0}}, 164},
	{0x0, {{0,0}}, 170},
	{0x0, {{0,0}}, 1},
	{0x800000, {{0,659},{1,660},{0,0}}, -1},
	{0x0, {{0,0}}, 11},
	{0x0, {{0,0}}, 157},
	{0x0, {{0,0}}, 180},
	{0x0, {{0,0}}, 150},
	{0x0, {{0,0}}, 140},
	{0x0, {{0,0}}, 448},
	{0x0, {{0,0}}, 132},
	{0x0, {{0,0}}, 237},
	{0x0, {{0,0}}, 239},
	{0x0, {{0,0}}, 258},
	{0xa00000, {{0,672},{1,673},{0,0}}, -1},
	{0xa00000, {{0,674},{1,675},{0,0}}, -1},
	{0x7400, {{1,676},{2,677},{3,678},{5,679},{6,680},{7,681},{10,682},{11,683},{14,684},{15,685},{0,0}}, -1},
	{0x7400, {{2,686},{3,687},{6,688},{8,689},{10,690},{13,691},{15,692},{0,0}}, -1},
	{0x0, {{0,0}}, 192},
	{0x0, {{0,0}}, 234},
	{0x0, {{0,0}}, 223},
	{0x0, {{0,0}}, 235},
	{0x0, {{0,0}}, 552},
	{0x0, {{0,0}}, 213},
	{0x0, {{0,0}}, 554},
	{0x0, {{0,0}}, 545},
	{0x0, {{0,0}}, 496},
	{0x0, {{0,0}}, 547},
	{0x0, {{0,0}}, 216},
	{0x0, {{0,0}}, 466},
	{0x0, {{0,0}}, 500},
	{0x0, {{0,0}}, 524},
	{0x0, {{0,0}}, 224},
	{0x800800, {{0,693},{1,694},{0,0}}, -1},
	{0x0, {{0,0}}, 508},
	{0x0, {{0,0}}, 504},
	{0x0, {{0,0}}, 514},
	{0x0, {{0,0}}, 446},
	{0x0, {{0,0}}, 178},
	{0x0, {{0,0}}, 530},
	{0x0, {{0,0}}, 520},
	{0x40400000, {{0,715},{1,716},{2,717},{3,718},{0,0}}, -1},
	{0x40400000, {{0,719},{1,720},{2,721},{3,722},{0,0}}, -1},
	{0x40400000, {{0,723},{1,724},{2,725},{3,726},{0,0}}, -1},
	{0x40400000, {{0,727},{1,728},{2,729},{3,730},{0,0}}, -1},
	{0x40400000, {{0,731},{1,732},{2,733},{3,734},{0,0}}, -1},
	{0x40000000, {{0,735},{1,736},{0,0}}, -1},
	{0x40000000, {{0,737},{1,738},{0,0}}, -1},
	{0x40000000, {{0,739},{1,740},{0,0}}, -1},
	{0x40000000, {{0,741},{1,742},{0,0}}, -1},
	{0x40000000, {{0,743},{1,744},{0,0}}, -1},
	{0x400000, {{0,745},{1,746},{0,0}}, -1},
	{0x400000, {{0,747},{1,748},{0,0}}, -1},
	{0x400000, {{0,749},{1,750},{0,0}}, -1},
	{0x400000, {{0,751},{1,752},{0,0}}, -1},
	{0x400000, {{0,753},{1,754},{0,0}}, -1},
	{0x40400000, {{0,755},{2,756},{0,0}}, -1},
	{0x0, {{0,0}}, 339},
	{0x0, {{0,0}}, 349},
	{0x0, {{0,0}}, 340},
	{0x40400000, {{0,757},{2,758},{0,0}}, -1},
	{0x0, {{0,0}}, 610},
	{0x0, {{0,0}}, 352},
	{0x0, {{0,0}}, 611},
	{0x0, {{0,0}}, 353},
	{0x0, {{0,0}}, 597},
	{0x0, {{0,0}}, 323},
	{0x0, {{0,0}}, 601},
	{0x0, {{0,0}}, 327},
	{0x0, {{0,0}}, 606},
	{0x0, {{0,0}}, 345},
	{0x0, {{0,0}}, 607},
	{0x0, {{0,0}}, 346},
	{0x0, {{0,0}}, 598},
	{0x0, {{0,0}}, 324},
	{0x0, {{0,0}}, 602},
	{0x0, {{0,0}}, 328},
	{0x0, {{0,0}}, 600},
	{0x0, {{0,0}}, 326},
	{0x0, {{0,0}}, 604},
	{0x0, {{0,0}}, 330},
	{0x0, {{0,0}}, 354},
	{0x0, {{0,0}}, 355},
	{0x0, {{0,0}}, 331},
	{0x0, {{0,0}}, 335},
	{0x0, {{0,0}}, 347},
	{0x0, {{0,0}}, 348},
	{0x0, {{0,0}}, 332},
	{0x0, {{0,0}}, 336},
	{0x0, {{0,0}}, 334},
	{0x0, {{0,0}}, 338},
	{0x0, {{0,0}}, 609},
	{0x0, {{0,0}}, 351},
	{0x0, {{0,0}}, 592},
	{0x0, {{0,0}}, 316},
	{0x0, {{0,0}}, 605},
	{0x0, {{0,0}}, 344},
	{0x0, {{0,0}}, 593},
	{0x0, {{0,0}}, 317},
	{0x0, {{0,0}}, 596},
	{0x0, {{0,0}}, 322},
	{0x0, {{0,0}}, 356},
	{0x0, {{0,0}}, 416},
	{0x0, {{0,0}}, 343},
	{0x0, {{0,0}}, 415},
	{0x40400000, {{0,763},{1,764},{2,765},{3,766},{0,0}}, -1},
	{0x40000000, {{0,767},{1,768},{0,0}}, -1},
	{0x400000, {{0,769},{1,770},{0,0}}, -1},
	{0x40400000, {{0,771},{2,772},{0,0}}, -1},
	{0x0, {{0,0}}, 599},
	{0x0, {{0,0}}, 325},
	{0x0, {{0,0}}, 603},
	{0x0, {{0,0}}, 329},
	{0x0, {{0,0}}, 333},
	{0x0, {{0,0}}, 337},
	{0x0, {{0,0}}, 594},
	{0x0, {{0,0}}, 318},
	{0x0, {{0,0}}, 341},
	{0x0, {{0,0}}, 413},
	{0x0, {{0,0}}, 4},
	{0x0, {{0,0}}, 51},
	{0x0, {{0,0}}, 50},
	{0xf000, {{0,779},{0,0}}, -1},
	{0x0, {{0,0}}, 53},
	{0x0, {{0,0}}, 52},
	{0x0, {{0,0}}, 442},
	{0x0, {{0,0}}, 608},
	{0x0, {{0,0}}, 589},
	{0x0, {{0,0}}, 590},
	{0x0, {{0,0}}, 595},
	{0x0, {{0,0}}, 350},
	{0x0, {{0,0}}, 313},
	{0x0, {{0,0}}, 314},
	{0x0, {{0,0}}, 321},
	{0x0, {{0,0}}, 591},
	{0x0, {{0,0}}, 315},
	{0x0, {{0,0}}, 682},
	{0x0, {{0,0}}, 538},
	{0x0, {{0,0}}, 694},
	{0x0, {{0,0}}, 73},
	{0x0, {{0,0}}, 712},
	{0x0, {{0,0}}, 75},
	{0x0, {{0,0}}, 707},
	{0x0, {{0,0}}, 696},
	{0x0, {{0,0}}, 690},
	{0x0, {{0,0}}, 700},
	{0x0, {{0,0}}, 684},
	{0x0, {{0,0}}, 176},
	{0x0, {{0,0}}, 512},
	{0x0, {{0,0}}, 619},
	{0x0, {{0,0}}, 67},
	{0x0, {{0,0}}, 61},
	{0x0, {{0,0}}, 77},
	{0x9f0000, {{1,816},{33,817},{0,0}}, -1},
	{0x0, {{0,0}}, 516},
	{0x1f0000, {{0,818},{1,819},{0,0}}, -1},
	{0x9f0000, {{1,822},{16,823},{32,824},{48,825},{0,0}}, -1},
	{0x0, {{0,0}}, 116},
	{0x9f0000, {{1,826},{16,827},{32,828},{33,829},{0,0}}, -1},
	{0x800000, {{0,830},{1,831},{0,0}}, -1},
	{0x800000, {{0,832},{1,833},{0,0}}, -1},
	{0x9f0000, {{16,834},{48,835},{0,0}}, -1},
	{0x0, {{0,0}}, 167},
	{0x0, {{0,0}}, 173},
	{0x0, {{0,0}}, 397},
	{0x800000, {{0,820},{1,821},{0,0}}, -1},
	{0x0, {{0,0}}, 160},
	{0x0, {{0,0}}, 186},
	{0x0, {{0,0}}, 153},
	{0x0, {{0,0}}, 197},
	{0x0, {{0,0}}, 136},
	{0x0, {{0,0}}, 207},
	{0x0, {{0,0}}, 657},
	{0x0, {{0,0}}, 126},
	{0x0, {{0,0}}, 142},
	{0x0, {{0,0}}, 256},
	{0x0, {{0,0}}, 134},
	{0x0, {{0,0}}, 138},
	{0x0, {{0,0}}, 120},
	{0x0, {{0,0}}, 122},
	{0x0, {{0,0}}, 200},
	{0x0, {{0,0}}, 210},
	{0x0, {{0,0}}, 710},
	{0x0, {{0,0}}, 714},
	{0x0, {{0,0}}, 702},
	{0x0, {{0,0}}, 705},
	{0x0, {{0,0}}, 541},
	{0x0, {{0,0}}, 471},
	{0x0, {{0,0}}, 528},
	{0x0, {{0,0}}, 688},
	{0x800, {{0,849},{1,850},{0,0}}, -1},
	{0x800, {{0,851},{1,852},{0,0}}, -1},
	{0x0, {{0,0}}, 655},
	{0x0, {{0,0}}, 184},
	{0x0, {{0,0}}, 228},
	{0x0, {{0,0}}, 532},
	{0x0, {{0,0}}, 522},
	{0x0, {{0,0}}, 692},
	{0x0, {{0,0}}, 686},
	{0x20000000, {{0,855},{1,859},{0,0}}, -1},
	{0x20000000, {{0,857},{1,858},{0,0}}, -1},
	{0x0, {{0,0}}, 383},
	{0x0, {{0,0}}, 219},
	{0x0, {{0,0}}, 407},
	{0x0, {{0,0}}, 37},
	{0x0, {{0,0}}, 396},
	{0x0, {{0,0}}, 273},
	{0x0, {{0,0}}, 275},
	{0x0, {{0,0}}, 274},
	{0x0, {{0,0}}, 276},
	{0x0, {{0,0}}, 279},
	{0x0, {{0,0}}, 281},
	{0x0, {{0,0}}, 280},
	{0x0, {{0,0}}, 282},
	{0x0, {{0,0}}, 285},
	{0x0, {{0,0}}, 287},
	{0x0, {{0,0}}, 286},
	{0x0, {{0,0}}, 288},
	{0x0, {{0,0}}, 291},
	{0x0, {{0,0}}, 293},
	{0x0, {{0,0}}, 292},
	{0x0, {{0,0}}, 294},
};  // end main_decoder_table

MachRegister InstructionDecoder_aarch64::sysRegMap(unsigned int m) {
    switch(m) {
    case 0x741d: return aarch64::tlbi_vale3is;
    case 0xc021: return aarch64::id_aa64pfr1_el1;
    case 0xf089: return aarch64::sder32_el3;
    case 0xf664: return aarch64::icc_ctlr_el3;
    case 0xe180: return aarch64::dacr32_el2;
    case 0xe659: return aarch64::ich_vtr_el2;
    case 0xc659: return aarch64::icc_dir_el1;
    case 0x6425: return aarch64::tlbi_ipas2le1;
    case 0xda20: return aarch64::fpcr;
    case 0xf288: return aarch64::afsr0_el3;
    case 0xd807: return aarch64::dczid_el0;
    case 0xc081: return aarch64::actlr_el1;
    case 0xf080: return aarch64::sctlr_el3;
    case 0xe080: return aarch64::sctlr_el2;
    case 0x63c0: return aarch64::at_s1e2r;
    case 0x43a8: return aarch64::ic_iallu;
    case 0xf667: return aarch64::icc_igrpen1_el3;
    case 0xdce6: return aarch64::pmceid0_el0;
    case 0xde82: return aarch64::tpidr_el0;
    case 0x6405: return aarch64::tlbi_ipas2le1is;
    case 0xc681: return aarch64::contextidr_el1;
    case 0xc00e: return aarch64::id_mmfr2_el1;
    case 0xdf60: return aarch64::pmevtyper0_el0;
    case 0xdf61: return aarch64::pmevtyper1_el0;
    case 0xdf62: return aarch64::pmevtyper2_el0;
    case 0xdf63: return aarch64::pmevtyper3_el0;
    case 0xdf64: return aarch64::pmevtyper4_el0;
    case 0xdf65: return aarch64::pmevtyper5_el0;
    case 0xdf66: return aarch64::pmevtyper6_el0;
    case 0xdf67: return aarch64::pmevtyper7_el0;
    case 0xdf68: return aarch64::pmevtyper8_el0;
    case 0xdf69: return aarch64::pmevtyper9_el0;
    case 0xdf6a: return aarch64::pmevtyper10_el0;
    case 0xdf6b: return aarch64::pmevtyper11_el0;
    case 0xdf6c: return aarch64::pmevtyper12_el0;
    case 0xdf6d: return aarch64::pmevtyper13_el0;
    case 0xdf6e: return aarch64::pmevtyper14_el0;
    case 0xdf6f: return aarch64::pmevtyper15_el0;
    case 0xdf70: return aarch64::pmevtyper16_el0;
    case 0xdf71: return aarch64::pmevtyper17_el0;
    case 0xdf72: return aarch64::pmevtyper18_el0;
    case 0xdf73: return aarch64::pmevtyper19_el0;
    case 0xdf74: return aarch64::pmevtyper20_el0;
    case 0xdf75: return aarch64::pmevtyper21_el0;
    case 0xdf76: return aarch64::pmevtyper22_el0;
    case 0xdf77: return aarch64::pmevtyper23_el0;
    case 0xdf78: return aarch64::pmevtyper24_el0;
    case 0xdf79: return aarch64::pmevtyper25_el0;
    case 0xdf7a: return aarch64::pmevtyper26_el0;
    case 0xdf7b: return aarch64::pmevtyper27_el0;
    case 0xdf7c: return aarch64::pmevtyper28_el0;
    case 0xdf7d: return aarch64::pmevtyper29_el0;
    case 0xdf7e: return aarch64::pmevtyper30_el0;
    case 0xc510: return aarch64::mair_el1;
    case 0x641d: return aarch64::tlbi_vale2is;
    case 0xc800: return aarch64::ccsidr_el1;
    case 0xe218: return aarch64::spsr_irq;
    case 0xf201: return aarch64::elr_el3;
    case 0x7418: return aarch64::tlbi_alle3is;
    case 0xc208: return aarch64::sp_el0;
    case 0xc00d: return aarch64::id_mmfr1_el1;
    case 0x5ba9: return aarch64::ic_ivau;
    case 0xc641: return aarch64::icc_eoir0_el1;
    case 0xc65f: return aarch64::icc_sgi0r_el1;
    case 0xc65d: return aarch64::icc_sgi1r_el1;
    case 0x6439: return aarch64::tlbi_vae2;
    case 0xdf11: return aarch64::cntp_ctl_el0;
    case 0xc684: return aarch64::tpidr_el1;
    case 0x5bf1: return aarch64::dc_civac;
    case 0x6401: return aarch64::tlbi_ipas2e1is;
    case 0xdf1a: return aarch64::cntv_cval_el0;
    case 0xc643: return aarch64::icc_bpr0_el1;
    case 0x9808: return aarch64::mdccsr_el0;
    case 0x83c6: return aarch64::dbgclaimset_el1;
    case 0xe640: return aarch64::ich_ap0r0_el2;
    case 0xe641: return aarch64::ich_ap0r1_el2;
    case 0xe642: return aarch64::ich_ap0r2_el2;
    case 0xe643: return aarch64::ich_ap0r3_el2;
    case 0x43b1: return aarch64::dc_ivac;
    case 0xe65d: return aarch64::ich_elrsr_el2;
    case 0xe658: return aarch64::ich_hcr_el2;
    case 0xe600: return aarch64::vbar_el2;
    case 0xc65e: return aarch64::icc_asgi1r_el1;
    case 0xc02d: return aarch64::id_aa64afr1_el1;
    case 0xda10: return aarch64::nzcv;
    case 0xc100: return aarch64::ttbr0_el1;
    case 0xf081: return aarch64::actlr_el3;
    case 0x8080: return aarch64::mdrar_el1;
    case 0xf602: return aarch64::rmr_el3;
    case 0xc708: return aarch64::cntkctl_el1;
    case 0xf601: return aarch64::rvbar_el3;
    case 0xc00a: return aarch64::id_dfr0_el1;
    case 0x43c0: return aarch64::at_s1e1r;
    case 0xc648: return aarch64::icc_ap1r0_el1;
    case 0xc649: return aarch64::icc_ap1r1_el1;
    case 0xc64a: return aarch64::icc_ap1r2_el1;
    case 0xc64b: return aarch64::icc_ap1r3_el1;
    case 0xc230: return aarch64::icc_pmr_el1;
    case 0xdce7: return aarch64::pmceid1_el0;
    case 0x43d2: return aarch64::dc_csw;
    case 0xc009: return aarch64::id_pfr1_el1;
    case 0xc00b: return aarch64::id_afr0_el1;
    case 0xe518: return aarch64::amair_el2;
    case 0xdcf3: return aarch64::pmovsset_el0;
    case 0x4438: return aarch64::tlbi_vmalle1;
    case 0x643d: return aarch64::tlbi_vale2;
    case 0x63c7: return aarch64::at_s12e0w;
    case 0xc015: return aarch64::id_isar5_el1;
    case 0xc102: return aarch64::tcr_el1;
    case 0xdf40: return aarch64::pmevcntr0_el0;
    case 0xdf41: return aarch64::pmevcntr1_el0;
    case 0xdf42: return aarch64::pmevcntr2_el0;
    case 0xdf43: return aarch64::pmevcntr3_el0;
    case 0xdf44: return aarch64::pmevcntr4_el0;
    case 0xdf45: return aarch64::pmevcntr5_el0;
    case 0xdf46: return aarch64::pmevcntr6_el0;
    case 0xdf47: return aarch64::pmevcntr7_el0;
    case 0xdf48: return aarch64::pmevcntr8_el0;
    case 0xdf49: return aarch64::pmevcntr9_el0;
    case 0xdf4a: return aarch64::pmevcntr10_el0;
    case 0xdf4b: return aarch64::pmevcntr11_el0;
    case 0xdf4c: return aarch64::pmevcntr12_el0;
    case 0xdf4d: return aarch64::pmevcntr13_el0;
    case 0xdf4e: return aarch64::pmevcntr14_el0;
    case 0xdf4f: return aarch64::pmevcntr15_el0;
    case 0xdf50: return aarch64::pmevcntr16_el0;
    case 0xdf51: return aarch64::pmevcntr17_el0;
    case 0xdf52: return aarch64::pmevcntr18_el0;
    case 0xdf53: return aarch64::pmevcntr19_el0;
    case 0xdf54: return aarch64::pmevcntr20_el0;
    case 0xdf55: return aarch64::pmevcntr21_el0;
    case 0xdf56: return aarch64::pmevcntr22_el0;
    case 0xdf57: return aarch64::pmevcntr23_el0;
    case 0xdf58: return aarch64::pmevcntr24_el0;
    case 0xdf59: return aarch64::pmevcntr25_el0;
    case 0xdf5a: return aarch64::pmevcntr26_el0;
    case 0xdf5b: return aarch64::pmevcntr27_el0;
    case 0xdf5c: return aarch64::pmevcntr28_el0;
    case 0xdf5d: return aarch64::pmevcntr29_el0;
    case 0xdf5e: return aarch64::pmevcntr30_el0;
    case 0x9820: return aarch64::dbgdtr_el0;
    case 0xf518: return aarch64::amair_el3;
    case 0xc601: return aarch64::rvbar_el1;
    case 0x5bd9: return aarch64::dc_cvau;
    case 0xe288: return aarch64::afsr0_el2;
    case 0xc640: return aarch64::icc_iar0_el1;
    case 0xc663: return aarch64::icc_bpr1_el1;
    case 0x43f2: return aarch64::dc_cisw;
    case 0xe304: return aarch64::hpfar_el2;
    case 0xe219: return aarch64::spsr_abt;
    case 0xd000: return aarch64::csselr_el1;
    case 0xc4f2: return aarch64::pmintenclr_el1;
    case 0x443b: return aarch64::tlbi_vaae1;
    case 0xe300: return aarch64::far_el2;
    case 0xc212: return aarch64::currentel;
    case 0x80a4: return aarch64::dbgprcr_el1;
    case 0x6421: return aarch64::tlbi_ipas2e1;
    case 0xf510: return aarch64::mair_el3;
    case 0xc101: return aarch64::ttbr1_el1;
    case 0xdf02: return aarch64::cntvct_el0;
    case 0x441a: return aarch64::tlbi_aside1is;
    case 0xf088: return aarch64::scr_el3;
    case 0xc018: return aarch64::mvfr0_el1;
    case 0x643c: return aarch64::tlbi_alle1;
    case 0xdce3: return aarch64::pmovsclr_el0;
    case 0xc019: return aarch64::mvfr1_el1;
    case 0x443f: return aarch64::tlbi_vaale1;
    case 0xc201: return aarch64::elr_el1;
    case 0xe601: return aarch64::rvbar_el2;
    case 0xc290: return aarch64::esr_el1;
    case 0x443a: return aarch64::tlbi_aside1;
    case 0xdce1: return aarch64::pmcntenset_el0;
    case 0xda28: return aarch64::dspsr_el0;
    case 0x5ba1: return aarch64::dc_zva;
    case 0xe65a: return aarch64::ich_misr_el2;
    case 0xc600: return aarch64::vbar_el1;
    case 0xc011: return aarch64::id_isar1_el1;
    case 0xc082: return aarch64::cpacr_el1;
    case 0xc3a0: return aarch64::par_el1;
    case 0xe711: return aarch64::cnthp_ctl_el2;
    case 0xe081: return aarch64::actlr_el2;
    case 0xe21b: return aarch64::spsr_fiq;
    case 0xc020: return aarch64::id_aa64pfr0_el1;
    case 0xf682: return aarch64::tpidr_el3;
    case 0x63c1: return aarch64::at_s1e2w;
    case 0x6438: return aarch64::tlbi_alle2;
    case 0xc014: return aarch64::id_isar4_el1;
    case 0xa038: return aarch64::dbgvcr32_el2;
    case 0xe088: return aarch64::hcr_el2;
    case 0xe65b: return aarch64::ich_eisr_el2;
    case 0x8010: return aarch64::mdccint_el1;
    case 0xda29: return aarch64::dlr_el0;
    case 0xe682: return aarch64::tpidr_el2;
    case 0xdcf0: return aarch64::pmuserenr_el0;
    case 0x8032: return aarch64::oseccr_el1;
    case 0x641e: return aarch64::tlbi_vmalls12e1is;
    case 0x8012: return aarch64::mdscr_el1;
    case 0xe65f: return aarch64::ich_vmcr_el2;
    case 0xf300: return aarch64::far_el3;
    case 0xe200: return aarch64::spsr_el2;
    case 0x808c: return aarch64::oslsr_el1;
    case 0x4439: return aarch64::tlbi_vae1;
    case 0xda11: return aarch64::daif;
    case 0xc300: return aarch64::far_el1;
    case 0xe08b: return aarch64::hstr_el2;
    case 0xc801: return aarch64::clidr_el1;
    case 0xe108: return aarch64::vttbr_el2;
    case 0xc02c: return aarch64::id_aa64afr0_el1;
    case 0xc65b: return aarch64::icc_rpr_el1;
    case 0xff11: return aarch64::cntps_ctl_el1;
    case 0xc289: return aarch64::afsr1_el1;
    case 0xc210: return aarch64::spsel;
    case 0xda21: return aarch64::fpsr;
    case 0xdce0: return aarch64::pmcr_el0;
    case 0x441b: return aarch64::tlbi_vaae1is;
    case 0xc665: return aarch64::icc_sre_el1;
    case 0xdf01: return aarch64::cntpct_el0;
    case 0x6418: return aarch64::tlbi_alle2is;
    case 0xc667: return aarch64::icc_igrpen1_el1;
    case 0xe281: return aarch64::ifsr32_el2;
    case 0xc666: return aarch64::icc_igrpen0_el1;
    case 0xde83: return aarch64::tpidrro_el0;
    case 0xe08f: return aarch64::hacr_el2;
    case 0x43c3: return aarch64::at_s1e0w;
    case 0xdf7f: return aarch64::pmccfiltr_el0;
    case 0xe710: return aarch64::cnthp_tval_el2;
    case 0x63c6: return aarch64::at_s12e0r;
    case 0xe703: return aarch64::cntvoff_el2;
    case 0xe64d: return aarch64::icc_sre_el2;
    case 0xc200: return aarch64::spsr_el1;
    case 0xc608: return aarch64::isr_el1;
    case 0xe708: return aarch64::cnthctl_el2;
    case 0xe10a: return aarch64::vtcr_el2;
    case 0xe005: return aarch64::vmpidr_el2;
    case 0xdce4: return aarch64::pmswinc_el0;
    case 0xc00c: return aarch64::id_mmfr0_el1;
    case 0xdf12: return aarch64::cntp_cval_el0;
    case 0xc642: return aarch64::icc_hppir0_el1;
    case 0xf099: return aarch64::mdcr_el3;
    case 0x83f6: return aarch64::dbgauthstatus_el1;
    case 0xc661: return aarch64::icc_eoir1_el1;
    case 0xe201: return aarch64::elr_el2;
    case 0x4388: return aarch64::ic_ialluis;
    case 0xf102: return aarch64::tcr_el3;
    case 0xc807: return aarch64::aidr_el1;
    case 0xdf00: return aarch64::cntfrq_el0;
    case 0x7439: return aarch64::tlbi_vae3;
    case 0x643e: return aarch64::tlbi_vmalls12e1;
    case 0xe290: return aarch64::esr_el2;
    case 0xf600: return aarch64::vbar_el3;
    case 0xc602: return aarch64::rmr_el1;
    case 0xc000: return aarch64::midr_el1;
    case 0x43b2: return aarch64::dc_isw;
    case 0xc010: return aarch64::id_isar0_el1;
    case 0xd801: return aarch64::ctr_el0;
    case 0xe208: return aarch64::sp_el1;
    case 0xdf18: return aarch64::cntv_tval_el0;
    case 0x7419: return aarch64::tlbi_vae3is;
    case 0xf289: return aarch64::afsr1_el3;
    case 0xc016: return aarch64::id_mmfr4_el1;
    case 0xf100: return aarch64::ttbr0_el3;
    case 0xe21a: return aarch64::spsr_und;
    case 0x6419: return aarch64::tlbi_vae2is;
    case 0x73c1: return aarch64::at_s1e3w;
    case 0x809c: return aarch64::osdlr_el1;
    case 0xe660: return aarch64::ich_lr0_el2;
    case 0xe661: return aarch64::ich_lr1_el2;
    case 0xe662: return aarch64::ich_lr2_el2;
    case 0xe663: return aarch64::ich_lr3_el2;
    case 0xe664: return aarch64::ich_lr4_el2;
    case 0xe665: return aarch64::ich_lr5_el2;
    case 0xe666: return aarch64::ich_lr6_el2;
    case 0xe667: return aarch64::ich_lr7_el2;
    case 0xe668: return aarch64::ich_lr8_el2;
    case 0xe669: return aarch64::ich_lr9_el2;
    case 0xe66a: return aarch64::ich_lr10_el2;
    case 0xe66b: return aarch64::ich_lr11_el2;
    case 0xe66c: return aarch64::ich_lr12_el2;
    case 0xe66d: return aarch64::ich_lr13_el2;
    case 0xe66e: return aarch64::ich_lr14_el2;
    case 0xe66f: return aarch64::ich_lr15_el2;
    case 0x83ce: return aarch64::dbgclaimclr_el1;
    case 0x4418: return aarch64::tlbi_vmalle1is;
    case 0xc039: return aarch64::id_aa64mmfr1_el1;
    case 0xdce8: return aarch64::pmccntr_el0;
    case 0xf290: return aarch64::esr_el3;
    case 0xf08a: return aarch64::cptr_el3;
    case 0xdf10: return aarch64::cntp_tval_el0;
    case 0xe102: return aarch64::tcr_el2;
    case 0x5bd1: return aarch64::dc_cvac;
    case 0x441f: return aarch64::tlbi_vaale1is;
    case 0xc4f1: return aarch64::pmintenset_el1;
    case 0x8000: return aarch64::dbgbcr0_el1;
    case 0x8001: return aarch64::dbgbcr1_el1;
    case 0x8002: return aarch64::dbgbcr2_el1;
    case 0x8003: return aarch64::dbgbcr3_el1;
    case 0x8004: return aarch64::dbgbcr4_el1;
    case 0x8005: return aarch64::dbgbcr5_el1;
    case 0x8006: return aarch64::dbgbcr6_el1;
    case 0x8007: return aarch64::dbgbcr7_el1;
    case 0x8008: return aarch64::dbgbcr8_el1;
    case 0x8009: return aarch64::dbgbcr9_el1;
    case 0x800a: return aarch64::dbgbcr10_el1;
    case 0x800b: return aarch64::dbgbcr11_el1;
    case 0x800c: return aarch64::dbgbcr12_el1;
    case 0x800d: return aarch64::dbgbcr13_el1;
    case 0x800e: return aarch64::dbgbcr14_el1;
    case 0x800f: return aarch64::dbgbcr15_el1;
    case 0xdce5: return aarch64::pmselr_el0;
    case 0x641c: return aarch64::tlbi_alle1is;
    case 0xc662: return aarch64::icc_hppir1_el1;
    case 0x4419: return aarch64::tlbi_vae1is;
    case 0xe089: return aarch64::mdcr_el2;
    case 0x63c5: return aarch64::at_s12e1w;
    case 0xe000: return aarch64::vpidr_el2;
    case 0xc028: return aarch64::id_aa64dfr0_el1;
    case 0xc012: return aarch64::id_isar2_el1;
    case 0xe100: return aarch64::ttbr0_el2;
    case 0xc644: return aarch64::icc_ap0r0_el1;
    case 0xc645: return aarch64::icc_ap0r1_el1;
    case 0xc646: return aarch64::icc_ap0r2_el1;
    case 0xc647: return aarch64::icc_ap0r3_el1;
    case 0x9828: return aarch64::dbgdtrrx_el0;
    case 0xe298: return aarch64::fpexc32_el2;
    case 0xdce9: return aarch64::pmxevtyper_el0;
    case 0x8084: return aarch64::oslar_el1;
    case 0xc008: return aarch64::id_pfr0_el1;
    case 0xc00f: return aarch64::id_mmfr3_el1;
    case 0x801a: return aarch64::osdtrtx_el1;
    case 0xc030: return aarch64::id_aa64isar0_el1;
    case 0xc01a: return aarch64::mvfr2_el1;
    case 0xdce2: return aarch64::pmcntenclr_el0;
    case 0xe648: return aarch64::ich_ap1r0_el2;
    case 0xe649: return aarch64::ich_ap1r1_el2;
    case 0xe64a: return aarch64::ich_ap1r2_el2;
    case 0xe64b: return aarch64::ich_ap1r3_el2;
    case 0xc038: return aarch64::id_aa64mmfr0_el1;
    case 0x743d: return aarch64::tlbi_vale3;
    case 0xc006: return aarch64::revidr_el1;
    case 0xe510: return aarch64::mair_el2;
    case 0xc080: return aarch64::sctlr_el1;
    case 0x43c2: return aarch64::at_s1e0r;
    case 0xe08a: return aarch64::cptr_el2;
    case 0x63c4: return aarch64::at_s12e1r;
    case 0xdcea: return aarch64::pmxevcntr_el0;
    case 0xc288: return aarch64::afsr0_el1;
    case 0x43c1: return aarch64::at_s1e1w;
    case 0xc013: return aarch64::id_isar3_el1;
    case 0xe289: return aarch64::afsr1_el2;
    case 0x441d: return aarch64::tlbi_vale1is;
    case 0xf208: return aarch64::sp_el2;
    case 0xff12: return aarch64::cntps_cval_el1;
    case 0xff10: return aarch64::cntps_tval_el1;
    case 0xc664: return aarch64::icc_ctlr_el1;
    case 0xdf19: return aarch64::cntv_ctl_el0;
    case 0xe602: return aarch64::rmr_el2;
    case 0xc029: return aarch64::id_aa64dfr1_el1;
    case 0xe712: return aarch64::cnthp_cval_el2;
    case 0x73c0: return aarch64::at_s1e3r;
    case 0xc031: return aarch64::id_aa64isar1_el1;
    case 0xc005: return aarch64::mpidr_el1;
    case 0xc518: return aarch64::amair_el1;
    case 0x7438: return aarch64::tlbi_alle3;
    case 0xf200: return aarch64::spsr_el3;
    case 0x443d: return aarch64::tlbi_vale1;
    case 0xf665: return aarch64::icc_sre_el3;
    case 0xc660: return aarch64::icc_iar1_el1;
    default: assert(!"tried to access system register not accessible in EL0");
    };
}  // end sysRegMap
