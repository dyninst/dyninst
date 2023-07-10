#include <map>
#include <vector>

struct aarch64_mask_entry;
struct aarch64_insn_entry;

#define COMMA ,
#define fn(X) (X)
//#define T_ARGS( ... ) < __VA_ARGS__ >
//#define fn2(X, Y) (X##,##Y)

typedef void (*operandFactory)();
//using operandFactory = void (*)();
typedef std::vector<operandFactory> operandSpec;
typedef std::vector<aarch64_insn_entry> aarch64_insn_table;
typedef std::map<unsigned int, unsigned int> branchMap;
typedef std::map<unsigned int, aarch64_mask_entry> aarch64_decoder_table;

typedef enum {
  aarch64_op_INVALID,
  aarch64_op_unallocated,
  aarch64_op_extended,
  aarch64_op_abs_advsimd,
  aarch64_op_adc,
  aarch64_op_adcs,
  aarch64_op_add_addsub_ext,
  aarch64_op_add_addsub_imm,
  aarch64_op_add_addsub_shift,
  aarch64_op_add_advsimd,
  aarch64_op_addhn_advsimd,
  aarch64_op_addp_advsimd_pair,
  aarch64_op_addp_advsimd_vec,
  aarch64_op_adds_addsub_ext,
  aarch64_op_adds_addsub_imm,
  aarch64_op_adds_addsub_shift,
  aarch64_op_addv_advsimd,
  aarch64_op_adr,
  aarch64_op_adrp,
  aarch64_op_aesd_advsimd,
  aarch64_op_aese_advsimd,
  aarch64_op_aesimc_advsimd,
  aarch64_op_aesmc_advsimd,
  aarch64_op_and_advsimd,
  aarch64_op_and_log_imm,
  aarch64_op_and_log_shift,
  aarch64_op_ands_log_imm,
  aarch64_op_ands_log_shift,
  aarch64_op_asr_asrv,
  aarch64_op_asr_sbfm,
  aarch64_op_asrv,
  aarch64_op_at_sys,
  aarch64_op_b_cond,
  aarch64_op_b_uncond,
  aarch64_op_bfi_bfm,
  aarch64_op_bfm,
  aarch64_op_bfxil_bfm,
  aarch64_op_bic_advsimd_imm,
  aarch64_op_bic_advsimd_reg,
  aarch64_op_bic_log_shift,
  aarch64_op_bics,
  aarch64_op_bif_advsimd,
  aarch64_op_bit_advsimd,
  aarch64_op_bl,
  aarch64_op_blr,
  aarch64_op_br,
  aarch64_op_brk,
  aarch64_op_bsl_advsimd,
  aarch64_op_cbnz,
  aarch64_op_cbz,
  aarch64_op_ccmn_imm,
  aarch64_op_ccmn_reg,
  aarch64_op_ccmp_imm,
  aarch64_op_ccmp_reg,
  aarch64_op_cinc_csinc,
  aarch64_op_cinv_csinv,
  aarch64_op_clrex,
  aarch64_op_cls_advsimd,
  aarch64_op_cls_int,
  aarch64_op_clz_advsimd,
  aarch64_op_clz_int,
  aarch64_op_cmeq_advsimd_reg,
  aarch64_op_cmeq_advsimd_zero,
  aarch64_op_cmge_advsimd_reg,
  aarch64_op_cmge_advsimd_zero,
  aarch64_op_cmgt_advsimd_reg,
  aarch64_op_cmgt_advsimd_zero,
  aarch64_op_cmhi_advsimd,
  aarch64_op_cmhs_advsimd,
  aarch64_op_cmle_advsimd,
  aarch64_op_cmlt_advsimd,
  aarch64_op_cmn_adds_addsub_ext,
  aarch64_op_cmn_adds_addsub_imm,
  aarch64_op_cmn_adds_addsub_shift,
  aarch64_op_cmp_subs_addsub_ext,
  aarch64_op_cmp_subs_addsub_imm,
  aarch64_op_cmp_subs_addsub_shift,
  aarch64_op_cmtst_advsimd,
  aarch64_op_cneg_csneg,
  aarch64_op_cnt_advsimd,
  aarch64_op_crc32,
  aarch64_op_crc32c,
  aarch64_op_csel,
  aarch64_op_cset_csinc,
  aarch64_op_csetm_csinv,
  aarch64_op_csinc,
  aarch64_op_csinv,
  aarch64_op_csneg,
  aarch64_op_dc_sys,
  aarch64_op_dcps1,
  aarch64_op_dcps2,
  aarch64_op_dcps3,
  aarch64_op_dmb,
  aarch64_op_drps,
  aarch64_op_dsb,
  aarch64_op_dup_advsimd_elt,
  aarch64_op_dup_advsimd_gen,
  aarch64_op_eon,
  aarch64_op_eor_advsimd,
  aarch64_op_eor_log_imm,
  aarch64_op_eor_log_shift,
  aarch64_op_eret,
  aarch64_op_ext_advsimd,
  aarch64_op_extr,
  aarch64_op_fabd_advsimd,
  aarch64_op_fabs_advsimd,
  aarch64_op_fabs_float,
  aarch64_op_facge_advsimd,
  aarch64_op_facgt_advsimd,
  aarch64_op_fadd_advsimd,
  aarch64_op_fadd_float,
  aarch64_op_faddp_advsimd_pair,
  aarch64_op_faddp_advsimd_vec,
  aarch64_op_fccmp_float,
  aarch64_op_fccmpe_float,
  aarch64_op_fcmeq_advsimd_reg,
  aarch64_op_fcmeq_advsimd_zero,
  aarch64_op_fcmge_advsimd_reg,
  aarch64_op_fcmge_advsimd_zero,
  aarch64_op_fcmgt_advsimd_reg,
  aarch64_op_fcmgt_advsimd_zero,
  aarch64_op_fcmle_advsimd,
  aarch64_op_fcmlt_advsimd,
  aarch64_op_fcmp_float,
  aarch64_op_fcmpe_float,
  aarch64_op_fcsel_float,
  aarch64_op_fcvt_float,
  aarch64_op_fcvtas_advsimd,
  aarch64_op_fcvtas_float,
  aarch64_op_fcvtau_advsimd,
  aarch64_op_fcvtau_float,
  aarch64_op_fcvtl_advsimd,
  aarch64_op_fcvtms_advsimd,
  aarch64_op_fcvtms_float,
  aarch64_op_fcvtmu_advsimd,
  aarch64_op_fcvtmu_float,
  aarch64_op_fcvtn_advsimd,
  aarch64_op_fcvtns_advsimd,
  aarch64_op_fcvtns_float,
  aarch64_op_fcvtnu_advsimd,
  aarch64_op_fcvtnu_float,
  aarch64_op_fcvtps_advsimd,
  aarch64_op_fcvtps_float,
  aarch64_op_fcvtpu_advsimd,
  aarch64_op_fcvtpu_float,
  aarch64_op_fcvtxn_advsimd,
  aarch64_op_fcvtzs_advsimd_fix,
  aarch64_op_fcvtzs_advsimd_int,
  aarch64_op_fcvtzs_float_fix,
  aarch64_op_fcvtzs_float_int,
  aarch64_op_fcvtzu_advsimd_fix,
  aarch64_op_fcvtzu_advsimd_int,
  aarch64_op_fcvtzu_float_fix,
  aarch64_op_fcvtzu_float_int,
  aarch64_op_fdiv_advsimd,
  aarch64_op_fdiv_float,
  aarch64_op_fmadd_float,
  aarch64_op_fmax_advsimd,
  aarch64_op_fmax_float,
  aarch64_op_fmaxnm_advsimd,
  aarch64_op_fmaxnm_float,
  aarch64_op_fmaxnmp_advsimd_pair,
  aarch64_op_fmaxnmp_advsimd_vec,
  aarch64_op_fmaxnmv_advsimd,
  aarch64_op_fmaxp_advsimd_pair,
  aarch64_op_fmaxp_advsimd_vec,
  aarch64_op_fmaxv_advsimd,
  aarch64_op_fmin_advsimd,
  aarch64_op_fmin_float,
  aarch64_op_fminnm_advsimd,
  aarch64_op_fminnm_float,
  aarch64_op_fminnmp_advsimd_pair,
  aarch64_op_fminnmp_advsimd_vec,
  aarch64_op_fminnmv_advsimd,
  aarch64_op_fminp_advsimd_pair,
  aarch64_op_fminp_advsimd_vec,
  aarch64_op_fminv_advsimd,
  aarch64_op_fmla_advsimd_elt,
  aarch64_op_fmla_advsimd_vec,
  aarch64_op_fmls_advsimd_elt,
  aarch64_op_fmls_advsimd_vec,
  aarch64_op_fmov_advsimd,
  aarch64_op_fmov_float,
  aarch64_op_fmov_float_gen,
  aarch64_op_fmov_float_imm,
  aarch64_op_fmsub_float,
  aarch64_op_fmul_advsimd_elt,
  aarch64_op_fmul_advsimd_vec,
  aarch64_op_fmul_float,
  aarch64_op_fmulx_advsimd_elt,
  aarch64_op_fmulx_advsimd_vec,
  aarch64_op_fneg_advsimd,
  aarch64_op_fneg_float,
  aarch64_op_fnmadd_float,
  aarch64_op_fnmsub_float,
  aarch64_op_fnmul_float,
  aarch64_op_frecpe_advsimd,
  aarch64_op_frecps_advsimd,
  aarch64_op_frecpx_advsimd,
  aarch64_op_frinta_advsimd,
  aarch64_op_frinta_float,
  aarch64_op_frinti_advsimd,
  aarch64_op_frinti_float,
  aarch64_op_frintm_advsimd,
  aarch64_op_frintm_float,
  aarch64_op_frintn_advsimd,
  aarch64_op_frintn_float,
  aarch64_op_frintp_advsimd,
  aarch64_op_frintp_float,
  aarch64_op_frintx_advsimd,
  aarch64_op_frintx_float,
  aarch64_op_frintz_advsimd,
  aarch64_op_frintz_float,
  aarch64_op_frsqrte_advsimd,
  aarch64_op_frsqrts_advsimd,
  aarch64_op_fsqrt_advsimd,
  aarch64_op_fsqrt_float,
  aarch64_op_fsub_advsimd,
  aarch64_op_fsub_float,
  aarch64_op_hint,
  aarch64_op_hlt,
  aarch64_op_hvc,
  aarch64_op_ic_sys,
  aarch64_op_ins_advsimd_elt,
  aarch64_op_ins_advsimd_gen,
  aarch64_op_isb,
  aarch64_op_ld1_advsimd_mult,
  aarch64_op_ld1_advsimd_sngl,
  aarch64_op_ld1r_advsimd,
  aarch64_op_ld2_advsimd_mult,
  aarch64_op_ld2_advsimd_sngl,
  aarch64_op_ld2r_advsimd,
  aarch64_op_ld3_advsimd_mult,
  aarch64_op_ld3_advsimd_sngl,
  aarch64_op_ld3r_advsimd,
  aarch64_op_ld4_advsimd_mult,
  aarch64_op_ld4_advsimd_sngl,
  aarch64_op_ld4r_advsimd,
  aarch64_op_ldar,
  aarch64_op_ldarb,
  aarch64_op_ldarh,
  aarch64_op_ldaxp,
  aarch64_op_ldaxr,
  aarch64_op_ldaxrb,
  aarch64_op_ldaxrh,
  aarch64_op_ldnp_fpsimd,
  aarch64_op_ldnp_gen,
  aarch64_op_ldp_fpsimd,
  aarch64_op_ldp_gen,
  aarch64_op_ldpsw,
  aarch64_op_ldr_imm_fpsimd,
  aarch64_op_ldr_imm_gen,
  aarch64_op_ldr_lit_fpsimd,
  aarch64_op_ldr_lit_gen,
  aarch64_op_ldr_reg_fpsimd,
  aarch64_op_ldr_reg_gen,
  aarch64_op_ldrb_imm,
  aarch64_op_ldrb_reg,
  aarch64_op_ldrh_imm,
  aarch64_op_ldrh_reg,
  aarch64_op_ldrsb_imm,
  aarch64_op_ldrsb_reg,
  aarch64_op_ldrsh_imm,
  aarch64_op_ldrsh_reg,
  aarch64_op_ldrsw_imm,
  aarch64_op_ldrsw_lit,
  aarch64_op_ldrsw_reg,
  aarch64_op_ldtr,
  aarch64_op_ldtrb,
  aarch64_op_ldtrh,
  aarch64_op_ldtrsb,
  aarch64_op_ldtrsh,
  aarch64_op_ldtrsw,
  aarch64_op_ldur_fpsimd,
  aarch64_op_ldur_gen,
  aarch64_op_ldurb,
  aarch64_op_ldurh,
  aarch64_op_ldursb,
  aarch64_op_ldursh,
  aarch64_op_ldursw,
  aarch64_op_ldxp,
  aarch64_op_ldxr,
  aarch64_op_ldxrb,
  aarch64_op_ldxrh,
  aarch64_op_lsl_lslv,
  aarch64_op_lsl_ubfm,
  aarch64_op_lslv,
  aarch64_op_lsr_lsrv,
  aarch64_op_lsr_ubfm,
  aarch64_op_lsrv,
  aarch64_op_madd,
  aarch64_op_mla_advsimd_elt,
  aarch64_op_mla_advsimd_vec,
  aarch64_op_mls_advsimd_elt,
  aarch64_op_mls_advsimd_vec,
  aarch64_op_mneg_msub,
  aarch64_op_mov_add_addsub_imm,
  aarch64_op_mov_dup_advsimd_elt,
  aarch64_op_mov_ins_advsimd_elt,
  aarch64_op_mov_ins_advsimd_gen,
  aarch64_op_mov_movn,
  aarch64_op_mov_movz,
  aarch64_op_mov_orr_advsimd_reg,
  aarch64_op_mov_orr_log_imm,
  aarch64_op_mov_orr_log_shift,
  aarch64_op_mov_umov_advsimd,
  aarch64_op_movi_advsimd,
  aarch64_op_movk,
  aarch64_op_movn,
  aarch64_op_movz,
  aarch64_op_mrs,
  aarch64_op_msr_imm,
  aarch64_op_msr_reg,
  aarch64_op_msub,
  aarch64_op_mul_advsimd_elt,
  aarch64_op_mul_advsimd_vec,
  aarch64_op_mul_madd,
  aarch64_op_mvn_not_advsimd,
  aarch64_op_mvn_orn_log_shift,
  aarch64_op_mvni_advsimd,
  aarch64_op_neg_advsimd,
  aarch64_op_neg_sub_addsub_shift,
  aarch64_op_negs_subs_addsub_shift,
  aarch64_op_ngc_sbc,
  aarch64_op_ngcs_sbcs,
  aarch64_op_nop_hint,
  aarch64_op_not_advsimd,
  aarch64_op_orn_advsimd,
  aarch64_op_orn_log_shift,
  aarch64_op_orr_advsimd_imm,
  aarch64_op_orr_advsimd_reg,
  aarch64_op_orr_log_imm,
  aarch64_op_orr_log_shift,
  aarch64_op_pmul_advsimd,
  aarch64_op_pmull_advsimd,
  aarch64_op_prfm_imm,
  aarch64_op_prfm_lit,
  aarch64_op_prfm_reg,
  aarch64_op_prfum,
  aarch64_op_raddhn_advsimd,
  aarch64_op_rbit_advsimd,
  aarch64_op_rbit_int,
  aarch64_op_ret,
  aarch64_op_rev,
  aarch64_op_rev16_advsimd,
  aarch64_op_rev16_int,
  aarch64_op_rev32_advsimd,
  aarch64_op_rev32_int,
  aarch64_op_rev64_advsimd,
  aarch64_op_ror_extr,
  aarch64_op_ror_rorv,
  aarch64_op_rorv,
  aarch64_op_rshrn_advsimd,
  aarch64_op_rsubhn_advsimd,
  aarch64_op_saba_advsimd,
  aarch64_op_sabal_advsimd,
  aarch64_op_sabd_advsimd,
  aarch64_op_sabdl_advsimd,
  aarch64_op_sadalp_advsimd,
  aarch64_op_saddl_advsimd,
  aarch64_op_saddlp_advsimd,
  aarch64_op_saddlv_advsimd,
  aarch64_op_saddw_advsimd,
  aarch64_op_sbc,
  aarch64_op_sbcs,
  aarch64_op_sbfiz_sbfm,
  aarch64_op_sbfm,
  aarch64_op_sbfx_sbfm,
  aarch64_op_scvtf_advsimd_fix,
  aarch64_op_scvtf_advsimd_int,
  aarch64_op_scvtf_float_fix,
  aarch64_op_scvtf_float_int,
  aarch64_op_sdiv,
  aarch64_op_sev_hint,
  aarch64_op_sevl_hint,
  aarch64_op_sha1c_advsimd,
  aarch64_op_sha1h_advsimd,
  aarch64_op_sha1m_advsimd,
  aarch64_op_sha1p_advsimd,
  aarch64_op_sha1su0_advsimd,
  aarch64_op_sha1su1_advsimd,
  aarch64_op_sha256h2_advsimd,
  aarch64_op_sha256h_advsimd,
  aarch64_op_sha256su0_advsimd,
  aarch64_op_sha256su1_advsimd,
  aarch64_op_shadd_advsimd,
  aarch64_op_shl_advsimd,
  aarch64_op_shll_advsimd,
  aarch64_op_shrn_advsimd,
  aarch64_op_shsub_advsimd,
  aarch64_op_sli_advsimd,
  aarch64_op_smaddl,
  aarch64_op_smax_advsimd,
  aarch64_op_smaxp_advsimd,
  aarch64_op_smaxv_advsimd,
  aarch64_op_smc,
  aarch64_op_smin_advsimd,
  aarch64_op_sminp_advsimd,
  aarch64_op_sminv_advsimd,
  aarch64_op_smlal_advsimd_elt,
  aarch64_op_smlal_advsimd_vec,
  aarch64_op_smlsl_advsimd_elt,
  aarch64_op_smlsl_advsimd_vec,
  aarch64_op_smnegl_smsubl,
  aarch64_op_smov_advsimd,
  aarch64_op_smsubl,
  aarch64_op_smulh,
  aarch64_op_smull_advsimd_elt,
  aarch64_op_smull_advsimd_vec,
  aarch64_op_smull_smaddl,
  aarch64_op_sqabs_advsimd,
  aarch64_op_sqadd_advsimd,
  aarch64_op_sqdmlal_advsimd_elt,
  aarch64_op_sqdmlal_advsimd_vec,
  aarch64_op_sqdmlsl_advsimd_elt,
  aarch64_op_sqdmlsl_advsimd_vec,
  aarch64_op_sqdmulh_advsimd_elt,
  aarch64_op_sqdmulh_advsimd_vec,
  aarch64_op_sqdmull_advsimd_elt,
  aarch64_op_sqdmull_advsimd_vec,
  aarch64_op_sqneg_advsimd,
  aarch64_op_sqrdmulh_advsimd_elt,
  aarch64_op_sqrdmulh_advsimd_vec,
  aarch64_op_sqrshl_advsimd,
  aarch64_op_sqrshrn_advsimd,
  aarch64_op_sqrshrun_advsimd,
  aarch64_op_sqshl_advsimd_imm,
  aarch64_op_sqshl_advsimd_reg,
  aarch64_op_sqshlu_advsimd,
  aarch64_op_sqshrn_advsimd,
  aarch64_op_sqshrun_advsimd,
  aarch64_op_sqsub_advsimd,
  aarch64_op_sqxtn_advsimd,
  aarch64_op_sqxtun_advsimd,
  aarch64_op_srhadd_advsimd,
  aarch64_op_sri_advsimd,
  aarch64_op_srshl_advsimd,
  aarch64_op_srshr_advsimd,
  aarch64_op_srsra_advsimd,
  aarch64_op_sshl_advsimd,
  aarch64_op_sshll_advsimd,
  aarch64_op_sshr_advsimd,
  aarch64_op_ssra_advsimd,
  aarch64_op_ssubl_advsimd,
  aarch64_op_ssubw_advsimd,
  aarch64_op_st1_advsimd_mult,
  aarch64_op_st1_advsimd_sngl,
  aarch64_op_st2_advsimd_mult,
  aarch64_op_st2_advsimd_sngl,
  aarch64_op_st3_advsimd_mult,
  aarch64_op_st3_advsimd_sngl,
  aarch64_op_st4_advsimd_mult,
  aarch64_op_st4_advsimd_sngl,
  aarch64_op_stlr,
  aarch64_op_stlrb,
  aarch64_op_stlrh,
  aarch64_op_stlxp,
  aarch64_op_stlxr,
  aarch64_op_stlxrb,
  aarch64_op_stlxrh,
  aarch64_op_stnp_fpsimd,
  aarch64_op_stnp_gen,
  aarch64_op_stp_fpsimd,
  aarch64_op_stp_gen,
  aarch64_op_str_imm_fpsimd,
  aarch64_op_str_imm_gen,
  aarch64_op_str_reg_fpsimd,
  aarch64_op_str_reg_gen,
  aarch64_op_strb_imm,
  aarch64_op_strb_reg,
  aarch64_op_strh_imm,
  aarch64_op_strh_reg,
  aarch64_op_sttr,
  aarch64_op_sttrb,
  aarch64_op_sttrh,
  aarch64_op_stur_fpsimd,
  aarch64_op_stur_gen,
  aarch64_op_sturb,
  aarch64_op_sturh,
  aarch64_op_stxp,
  aarch64_op_stxr,
  aarch64_op_stxrb,
  aarch64_op_stxrh,
  aarch64_op_sub_addsub_ext,
  aarch64_op_sub_addsub_imm,
  aarch64_op_sub_addsub_shift,
  aarch64_op_sub_advsimd,
  aarch64_op_subhn_advsimd,
  aarch64_op_subs_addsub_ext,
  aarch64_op_subs_addsub_imm,
  aarch64_op_subs_addsub_shift,
  aarch64_op_suqadd_advsimd,
  aarch64_op_svc,
  aarch64_op_sxtb_sbfm,
  aarch64_op_sxth_sbfm,
  aarch64_op_sxtl_sshll_advsimd,
  aarch64_op_sxtw_sbfm,
  aarch64_op_sys,
  aarch64_op_sysl,
  aarch64_op_tbl_advsimd,
  aarch64_op_tbnz,
  aarch64_op_tbx_advsimd,
  aarch64_op_tbz,
  aarch64_op_tlbi_sys,
  aarch64_op_trn1_advsimd,
  aarch64_op_trn2_advsimd,
  aarch64_op_tst_ands_log_imm,
  aarch64_op_tst_ands_log_shift,
  aarch64_op_uaba_advsimd,
  aarch64_op_uabal_advsimd,
  aarch64_op_uabd_advsimd,
  aarch64_op_uabdl_advsimd,
  aarch64_op_uadalp_advsimd,
  aarch64_op_uaddl_advsimd,
  aarch64_op_uaddlp_advsimd,
  aarch64_op_uaddlv_advsimd,
  aarch64_op_uaddw_advsimd,
  aarch64_op_ubfiz_ubfm,
  aarch64_op_ubfm,
  aarch64_op_ubfx_ubfm,
  aarch64_op_ucvtf_advsimd_fix,
  aarch64_op_ucvtf_advsimd_int,
  aarch64_op_ucvtf_float_fix,
  aarch64_op_ucvtf_float_int,
  aarch64_op_udiv,
  aarch64_op_uhadd_advsimd,
  aarch64_op_uhsub_advsimd,
  aarch64_op_umaddl,
  aarch64_op_umax_advsimd,
  aarch64_op_umaxp_advsimd,
  aarch64_op_umaxv_advsimd,
  aarch64_op_umin_advsimd,
  aarch64_op_uminp_advsimd,
  aarch64_op_uminv_advsimd,
  aarch64_op_umlal_advsimd_elt,
  aarch64_op_umlal_advsimd_vec,
  aarch64_op_umlsl_advsimd_elt,
  aarch64_op_umlsl_advsimd_vec,
  aarch64_op_umnegl_umsubl,
  aarch64_op_umov_advsimd,
  aarch64_op_umsubl,
  aarch64_op_umulh,
  aarch64_op_umull_advsimd_elt,
  aarch64_op_umull_advsimd_vec,
  aarch64_op_umull_umaddl,
  aarch64_op_uqadd_advsimd,
  aarch64_op_uqrshl_advsimd,
  aarch64_op_uqrshrn_advsimd,
  aarch64_op_uqshl_advsimd_imm,
  aarch64_op_uqshl_advsimd_reg,
  aarch64_op_uqshrn_advsimd,
  aarch64_op_uqsub_advsimd,
  aarch64_op_uqxtn_advsimd,
  aarch64_op_urecpe_advsimd,
  aarch64_op_urhadd_advsimd,
  aarch64_op_urshl_advsimd,
  aarch64_op_urshr_advsimd,
  aarch64_op_ursqrte_advsimd,
  aarch64_op_ursra_advsimd,
  aarch64_op_ushl_advsimd,
  aarch64_op_ushll_advsimd,
  aarch64_op_ushr_advsimd,
  aarch64_op_usqadd_advsimd,
  aarch64_op_usra_advsimd,
  aarch64_op_usubl_advsimd,
  aarch64_op_usubw_advsimd,
  aarch64_op_uxtb_ubfm,
  aarch64_op_uxth_ubfm,
  aarch64_op_uxtl_ushll_advsimd,
  aarch64_op_uzp1_advsimd,
  aarch64_op_uzp2_advsimd,
  aarch64_op_wfe_hint,
  aarch64_op_wfi_hint,
  aarch64_op_xtn_advsimd,
  aarch64_op_yield_hint,
  aarch64_op_zip1_advsimd,
  aarch64_op_zip2_advsimd,
} entryID;

struct aarch64_insn_entry
{
	aarch64_insn_entry(entryID o, const char* m, operandSpec ops):
	op(o), mnemonic(m), operands(ops)
	{
	}

	aarch64_insn_entry():
	op(aarch64_op_INVALID), mnemonic("INVALID")
	{
		// TODO: why 5?
		// TODO: Is this needed here?
		operands.reserve(5);
	}

	aarch64_insn_entry(const aarch64_insn_entry& o) :
	op(o.op), mnemonic(o.mnemonic), operands(o.operands)
	{
	}

	const aarch64_insn_entry& operator=(const aarch64_insn_entry& rhs)
	{
		operands.reserve(rhs.operands.size());
		op = rhs.op;
		mnemonic = rhs.mnemonic;
		operands = rhs.operands;

		return *this;
	}

	entryID op;
	const char* mnemonic;
	operandSpec operands;

	static void buildInsnTable();
	static bool built_insn_table;

	static aarch64_insn_table main_insn_table;
};

struct aarch64_mask_entry
{
	aarch64_mask_entry(unsigned int m, branchMap bm, int tabIndex):
	mask(m), nodeBranches(bm), insnTableIndex(tabIndex)
	{
	}

	aarch64_mask_entry():
	mask(0), nodeBranches(branchMap()), insnTableIndex(-1)
	{
	}

	aarch64_mask_entry(const aarch64_mask_entry& e):
	mask(e.mask), nodeBranches(e.nodeBranches), insnTableIndex(e.insnTableIndex)
	{
	}

	const aarch64_mask_entry& operator=(const aarch64_mask_entry& rhs)
	{
		mask = rhs.mask;
		nodeBranches = rhs.nodeBranches;
		insnTableIndex = rhs.insnTableIndex;

		return *this;
	}

	unsigned int mask;
	branchMap nodeBranches;
	int insnTableIndex;

	static void buildDecoderTable();
	static bool built_decoder_table;
	static aarch64_decoder_table main_decoder_table;
};

aarch64_decoder_table aarch64_mask_entry::main_decoder_table;
aarch64_insn_table aarch64_insn_entry::main_insn_table;

void setFPMode(){ }

template<int, int>
void OPRimm(){ }
template<int, int>
void OPRoption(){ }
template<int, int>
void OPRN(){ }
template<int, int>
void OPRcond(){ }
template<int, int>
void OPRS(){ }

void OPRRt(){ }
void OPRop1(){ }
void OPRop2(){ }
void OPRcmode(){ }
void OPRRd(){ }
void OPRb5(){ }
void OPRRa(){ }
void OPRRm(){ }
void OPRRn(){ }
void OPRsize(){ }
void OPRscale(){ }
void OPRRs(){ }
void OPRRt2(){ }
void OPRtype(){ }
void OPRCRm(){ }
void OPRH(){ }
void OPRM(){ }
void OPRL(){ }
void OPRhw(){ }
void OPRQ(){ }
void OPRlen(){ }
void OPRopc(){ }
void OPRsz(){ }
void OPRa(){ }
void OPRb(){ }
void OPRc(){ }
void OPRd(){ }
void OPRe(){ }
void OPRf(){ }
void OPRg(){ }
void OPRh(){ }
void OPRshift(){ }
void OPRnzcv(){ }
void OPRCRn(){ }
void OPRo0(){ }
void OPRb40(){ }
void OPRsf(){ }
void OPRop(){ }
template<unsigned int endBit, unsigned int startBit>
void OPRsize() {}
template<unsigned int endBit, unsigned int startBit>
void OPRsz() {}
void setSIMDMode() {}
void OPRrmode() {}
template<unsigned int endBit, unsigned int startBit>
void OPRtype() {}
void setFlags() {}
void OPRRnL() {}
void OPRRtL() {}
void OPRRt2L() {}
void OPRRnLU() {}
void OPRRnS() {}
void OPRRtS() {}
void OPRRt2S() {}
void OPRRnSU() {}
void OPRopcode() {}
void setRegWidth() {}


void aarch64_mask_entry::buildDecoderTable()
{
		main_decoder_table[0]=aarch64_mask_entry(0x18000000, map_list_of(0,1)(1,2)(2,3)(3,4),-1);
	main_decoder_table[1]=aarch64_mask_entry(0x0, branchMap(),0);
	main_decoder_table[2]=aarch64_mask_entry(0x7000000, map_list_of(0,5)(1,6)(2,7)(3,8)(4,9)(5,10)(6,11)(7,12),-1);
	main_decoder_table[5]=aarch64_mask_entry(0x20c00000, map_list_of(0,13)(1,14)(2,15)(3,16)(4,17)(5,18)(6,19)(7,20),-1);
	main_decoder_table[13]=aarch64_mask_entry(0x80208000, map_list_of(0,21)(1,22)(4,23)(5,24)(6,25)(7,26),-1);
	main_decoder_table[21]=aarch64_mask_entry(0x40007c00, map_list_of(31,27)(63,28),-1);
	main_decoder_table[27]=aarch64_mask_entry(0x0, branchMap(),614);
	main_decoder_table[28]=aarch64_mask_entry(0x0, branchMap(),615);
	main_decoder_table[22]=aarch64_mask_entry(0x40007c00, map_list_of(31,29)(63,30),-1);
	main_decoder_table[29]=aarch64_mask_entry(0x0, branchMap(),579);
	main_decoder_table[30]=aarch64_mask_entry(0x0, branchMap(),580);
	main_decoder_table[23]=aarch64_mask_entry(0x0, branchMap(),613);
	main_decoder_table[24]=aarch64_mask_entry(0x0, branchMap(),578);
	main_decoder_table[25]=aarch64_mask_entry(0x0, branchMap(),612);
	main_decoder_table[26]=aarch64_mask_entry(0x0, branchMap(),577);
	main_decoder_table[14]=aarch64_mask_entry(0x803f8000, map_list_of(62,31)(63,32)(190,33)(191,34)(254,35)(255,36),-1);
	main_decoder_table[31]=aarch64_mask_entry(0x40007c00, map_list_of(31,37)(63,38),-1);
	main_decoder_table[37]=aarch64_mask_entry(0x0, branchMap(),359);
	main_decoder_table[38]=aarch64_mask_entry(0x0, branchMap(),360);
	main_decoder_table[32]=aarch64_mask_entry(0x40007c00, map_list_of(31,39)(63,40),-1);
	main_decoder_table[39]=aarch64_mask_entry(0x0, branchMap(),300);
	main_decoder_table[40]=aarch64_mask_entry(0x0, branchMap(),301);
	main_decoder_table[33]=aarch64_mask_entry(0x0, branchMap(),358);
	main_decoder_table[34]=aarch64_mask_entry(0x0, branchMap(),299);
	main_decoder_table[35]=aarch64_mask_entry(0x0, branchMap(),357);
	main_decoder_table[36]=aarch64_mask_entry(0x0, branchMap(),298);
	main_decoder_table[15]=aarch64_mask_entry(0x803ffc00, map_list_of(2047,41)(6143,42),-1);
	main_decoder_table[41]=aarch64_mask_entry(0x40000000, map_list_of(0,43)(1,44),-1);
	main_decoder_table[43]=aarch64_mask_entry(0x0, branchMap(),575);
	main_decoder_table[44]=aarch64_mask_entry(0x0, branchMap(),576);
	main_decoder_table[42]=aarch64_mask_entry(0x0, branchMap(),574);
	main_decoder_table[16]=aarch64_mask_entry(0x803ffc00, map_list_of(2047,45)(6143,46),-1);
	main_decoder_table[45]=aarch64_mask_entry(0x40000000, map_list_of(0,47)(1,48),-1);
	main_decoder_table[47]=aarch64_mask_entry(0x0, branchMap(),296);
	main_decoder_table[48]=aarch64_mask_entry(0x0, branchMap(),297);
	main_decoder_table[46]=aarch64_mask_entry(0x0, branchMap(),295);
	main_decoder_table[17]=aarch64_mask_entry(0x0, branchMap(),582);
	main_decoder_table[18]=aarch64_mask_entry(0x0, branchMap(),303);
	main_decoder_table[19]=aarch64_mask_entry(0x0, branchMap(),586);
	main_decoder_table[20]=aarch64_mask_entry(0x40000000, map_list_of(0,49)(1,50),-1);
	main_decoder_table[49]=aarch64_mask_entry(0x0, branchMap(),307);
	main_decoder_table[50]=aarch64_mask_entry(0x0, branchMap(),310);
	main_decoder_table[6]=aarch64_mask_entry(0x60c00000, map_list_of(4,51)(5,52)(6,53)(7,54)(13,55)(15,56),-1);
	main_decoder_table[51]=aarch64_mask_entry(0x0, branchMap(),588);
	main_decoder_table[52]=aarch64_mask_entry(0x0, branchMap(),309);
	main_decoder_table[53]=aarch64_mask_entry(0x0, branchMap(),587);
	main_decoder_table[54]=aarch64_mask_entry(0x0, branchMap(),308);
	main_decoder_table[55]=aarch64_mask_entry(0x0, branchMap(),312);
	main_decoder_table[56]=aarch64_mask_entry(0x0, branchMap(),311);
	main_decoder_table[7]=aarch64_mask_entry(0x60200000, map_list_of(0,57)(1,58)(2,59)(3,60)(4,61)(5,62)(6,63)(7,64),-1);
	main_decoder_table[57]=aarch64_mask_entry(0x0, branchMap(),25);
	main_decoder_table[58]=aarch64_mask_entry(0x0, branchMap(),39);
	main_decoder_table[59]=aarch64_mask_entry(0x0, branchMap(),410);
	main_decoder_table[60]=aarch64_mask_entry(0x0, branchMap(),406);
	main_decoder_table[61]=aarch64_mask_entry(0x0, branchMap(),112);
	main_decoder_table[62]=aarch64_mask_entry(0x0, branchMap(),109);
	main_decoder_table[63]=aarch64_mask_entry(0x0, branchMap(),27);
	main_decoder_table[64]=aarch64_mask_entry(0x0, branchMap(),40);
	main_decoder_table[8]=aarch64_mask_entry(0x60200000, map_list_of(0,65)(1,66)(2,67)(3,68)(4,69)(5,70)(6,71)(7,72),-1);
	main_decoder_table[65]=aarch64_mask_entry(0x0, branchMap(),7);
	main_decoder_table[66]=aarch64_mask_entry(0x0, branchMap(),5);
	main_decoder_table[67]=aarch64_mask_entry(0x0, branchMap(),15);
	main_decoder_table[68]=aarch64_mask_entry(0xc00000, map_list_of(0,73),-1);
	main_decoder_table[73]=aarch64_mask_entry(0x0, branchMap(),13);
	main_decoder_table[69]=aarch64_mask_entry(0x0, branchMap(),618);
	main_decoder_table[70]=aarch64_mask_entry(0x0, branchMap(),616);
	main_decoder_table[71]=aarch64_mask_entry(0x0, branchMap(),624);
	main_decoder_table[72]=aarch64_mask_entry(0xc00000, map_list_of(0,74),-1);
	main_decoder_table[74]=aarch64_mask_entry(0x0, branchMap(),622);
	main_decoder_table[9]=aarch64_mask_entry(0x20c00000, map_list_of(0,75)(1,76)(2,77)(3,78)(4,79)(5,80)(6,81)(7,82),-1);
	main_decoder_table[75]=aarch64_mask_entry(0x803f2000, map_list_of(0,83)(1,84),-1);
	main_decoder_table[83]=aarch64_mask_entry(0xd000, map_list_of(0,85)(2,86)(4,87),-1);
	main_decoder_table[85]=aarch64_mask_entry(0x0, branchMap(),570);
	main_decoder_table[86]=aarch64_mask_entry(0x0, branchMap(),566);
	main_decoder_table[87]=aarch64_mask_entry(0x0, branchMap(),562);
	main_decoder_table[84]=aarch64_mask_entry(0x0, branchMap(),558);
	main_decoder_table[76]=aarch64_mask_entry(0x803f2000, map_list_of(0,88)(1,89),-1);
	main_decoder_table[88]=aarch64_mask_entry(0xd000, map_list_of(0,90)(2,91)(4,92),-1);
	main_decoder_table[90]=aarch64_mask_entry(0x0, branchMap(),289);
	main_decoder_table[91]=aarch64_mask_entry(0x0, branchMap(),283);
	main_decoder_table[92]=aarch64_mask_entry(0x0, branchMap(),277);
	main_decoder_table[89]=aarch64_mask_entry(0x0, branchMap(),271);
	main_decoder_table[77]=aarch64_mask_entry(0x80202000, map_list_of(0,93)(1,94),-1);
	main_decoder_table[93]=aarch64_mask_entry(0xd000, map_list_of(0,95)(2,96)(4,97),-1);
	main_decoder_table[95]=aarch64_mask_entry(0x0, branchMap(),571);
	main_decoder_table[96]=aarch64_mask_entry(0x0, branchMap(),567);
	main_decoder_table[97]=aarch64_mask_entry(0x0, branchMap(),563);
	main_decoder_table[94]=aarch64_mask_entry(0x0, branchMap(),559);
	main_decoder_table[78]=aarch64_mask_entry(0x80202000, map_list_of(0,98)(1,99),-1);
	main_decoder_table[98]=aarch64_mask_entry(0xd000, map_list_of(0,100)(2,101)(4,102),-1);
	main_decoder_table[100]=aarch64_mask_entry(0x0, branchMap(),290);
	main_decoder_table[101]=aarch64_mask_entry(0x0, branchMap(),284);
	main_decoder_table[102]=aarch64_mask_entry(0x0, branchMap(),278);
	main_decoder_table[99]=aarch64_mask_entry(0x0, branchMap(),272);
	main_decoder_table[79]=aarch64_mask_entry(0x0, branchMap(),581);
	main_decoder_table[80]=aarch64_mask_entry(0x0, branchMap(),302);
	main_decoder_table[81]=aarch64_mask_entry(0x0, branchMap(),583);
	main_decoder_table[82]=aarch64_mask_entry(0x0, branchMap(),304);
	main_decoder_table[10]=aarch64_mask_entry(0x20c00000, map_list_of(0,103)(1,104)(2,105)(3,106)(4,107)(5,108)(6,109)(7,110),-1);
	main_decoder_table[103]=aarch64_mask_entry(0x803f2000, map_list_of(0,111)(1,112)(64,113)(65,114),-1);
	main_decoder_table[111]=aarch64_mask_entry(0x0, branchMap(),560);
	main_decoder_table[112]=aarch64_mask_entry(0x0, branchMap(),568);
	main_decoder_table[113]=aarch64_mask_entry(0x0, branchMap(),564);
	main_decoder_table[114]=aarch64_mask_entry(0x0, branchMap(),572);
	main_decoder_table[104]=aarch64_mask_entry(0x803f2000, map_list_of(0,115)(1,116)(64,117)(65,118),-1);
	main_decoder_table[115]=aarch64_mask_entry(0x0, branchMap(),273);
	main_decoder_table[116]=aarch64_mask_entry(0x0, branchMap(),285);
	main_decoder_table[117]=aarch64_mask_entry(0x0, branchMap(),279);
	main_decoder_table[118]=aarch64_mask_entry(0x0, branchMap(),291);
	main_decoder_table[105]=aarch64_mask_entry(0x80202000, map_list_of(0,119)(1,120)(2,121)(3,122),-1);
	main_decoder_table[119]=aarch64_mask_entry(0x0, branchMap(),561);
	main_decoder_table[120]=aarch64_mask_entry(0x0, branchMap(),569);
	main_decoder_table[121]=aarch64_mask_entry(0x0, branchMap(),565);
	main_decoder_table[122]=aarch64_mask_entry(0x0, branchMap(),573);
	main_decoder_table[106]=aarch64_mask_entry(0x80202000, map_list_of(0,123)(1,124)(2,125)(3,126),-1);
	main_decoder_table[123]=aarch64_mask_entry(0x0, branchMap(),274);
	main_decoder_table[124]=aarch64_mask_entry(0x0, branchMap(),286);
	main_decoder_table[125]=aarch64_mask_entry(0x0, branchMap(),280);
	main_decoder_table[126]=aarch64_mask_entry(0x0, branchMap(),292);
	main_decoder_table[107]=aarch64_mask_entry(0x0, branchMap(),585);
	main_decoder_table[108]=aarch64_mask_entry(0x0, branchMap(),306);
	main_decoder_table[109]=aarch64_mask_entry(0x0, branchMap(),584);
	main_decoder_table[110]=aarch64_mask_entry(0x0, branchMap(),305);
	main_decoder_table[11]=aarch64_mask_entry(0xa0208400, map_list_of(0,127)(1,128)(4,129)(5,130)(6,131)(7,132)(8,133)(9,134)(12,135)(13,136)(14,137)(15,138),-1);
	main_decoder_table[127]=aarch64_mask_entry(0x1800, map_list_of(0,139)(1,140)(2,141)(3,142),-1);
	main_decoder_table[139]=aarch64_mask_entry(0x0, branchMap(),634);
	main_decoder_table[140]=aarch64_mask_entry(0x6000, map_list_of(1,143)(3,144),-1);
	main_decoder_table[143]=aarch64_mask_entry(0x0, branchMap(),639);
	main_decoder_table[144]=aarch64_mask_entry(0x0, branchMap(),640);
	main_decoder_table[141]=aarch64_mask_entry(0x0, branchMap(),636);
	main_decoder_table[142]=aarch64_mask_entry(0x6000, map_list_of(0,145)(1,146)(2,147)(3,148),-1);
	main_decoder_table[145]=aarch64_mask_entry(0x0, branchMap(),721);
	main_decoder_table[146]=aarch64_mask_entry(0x0, branchMap(),727);
	main_decoder_table[147]=aarch64_mask_entry(0x0, branchMap(),722);
	main_decoder_table[148]=aarch64_mask_entry(0x0, branchMap(),728);
	main_decoder_table[128]=aarch64_mask_entry(0xc07800, map_list_of(0,149)(1,150)(3,151)(5,152)(7,153),-1);
	main_decoder_table[149]=aarch64_mask_entry(0x0, branchMap(),107);
	main_decoder_table[150]=aarch64_mask_entry(0x0, branchMap(),108);
	main_decoder_table[151]=aarch64_mask_entry(0x40000000, map_list_of(1,154),-1);
	main_decoder_table[154]=aarch64_mask_entry(0x0, branchMap(),269);
	main_decoder_table[152]=aarch64_mask_entry(0x0, branchMap(),486);
	main_decoder_table[153]=aarch64_mask_entry(0x0, branchMap(),382);
	main_decoder_table[129]=aarch64_mask_entry(0x7800, map_list_of(0,155)(1,156)(2,157)(3,158)(4,159)(5,160)(6,161)(7,162)(8,163)(9,164)(10,165)(11,166)(12,167)(13,168)(14,169)(15,170),-1);
	main_decoder_table[155]=aarch64_mask_entry(0x0, branchMap(),437);
	main_decoder_table[156]=aarch64_mask_entry(0x0, branchMap(),426);
	main_decoder_table[157]=aarch64_mask_entry(0x0, branchMap(),440);
	main_decoder_table[158]=aarch64_mask_entry(0x0, branchMap(),422);
	main_decoder_table[159]=aarch64_mask_entry(0x0, branchMap(),556);
	main_decoder_table[160]=aarch64_mask_entry(0x1f0000, map_list_of(0,171)(1,172),-1);
	main_decoder_table[171]=aarch64_mask_entry(0x0, branchMap(),438);
	main_decoder_table[172]=aarch64_mask_entry(0x0, branchMap(),725);
	main_decoder_table[161]=aarch64_mask_entry(0x0, branchMap(),557);
	main_decoder_table[162]=aarch64_mask_entry(0x1f0000, map_list_of(0,173)(16,174),-1);
	main_decoder_table[173]=aarch64_mask_entry(0x0, branchMap(),626);
	main_decoder_table[174]=aarch64_mask_entry(0x0, branchMap(),439);
	main_decoder_table[163]=aarch64_mask_entry(0x0, branchMap(),10);
	main_decoder_table[164]=aarch64_mask_entry(0x1f0000, map_list_of(0,175)(1,176)(8,177),-1);
	main_decoder_table[175]=aarch64_mask_entry(0x0, branchMap(),57);
	main_decoder_table[176]=aarch64_mask_entry(0x0, branchMap(),537);
	main_decoder_table[177]=aarch64_mask_entry(0x0, branchMap(),20);
	main_decoder_table[165]=aarch64_mask_entry(0x0, branchMap(),433);
	main_decoder_table[166]=aarch64_mask_entry(0x1f0000, map_list_of(0,178)(8,179),-1);
	main_decoder_table[178]=aarch64_mask_entry(0x0, branchMap(),90);
	main_decoder_table[179]=aarch64_mask_entry(0x0, branchMap(),19);
	main_decoder_table[167]=aarch64_mask_entry(0x0, branchMap(),621);
	main_decoder_table[168]=aarch64_mask_entry(0x1f0000, map_list_of(0,180)(1,181)(8,182),-1);
	main_decoder_table[180]=aarch64_mask_entry(0x0, branchMap(),436);
	main_decoder_table[181]=aarch64_mask_entry(0x0, branchMap(),163);
	main_decoder_table[182]=aarch64_mask_entry(0x0, branchMap(),22);
	main_decoder_table[169]=aarch64_mask_entry(0x0, branchMap(),435);
	main_decoder_table[170]=aarch64_mask_entry(0x1f0000, map_list_of(0,183)(1,184)(8,185),-1);
	main_decoder_table[183]=aarch64_mask_entry(0x0, branchMap(),493);
	main_decoder_table[184]=aarch64_mask_entry(0x0, branchMap(),156);
	main_decoder_table[185]=aarch64_mask_entry(0x0, branchMap(),21);
	main_decoder_table[130]=aarch64_mask_entry(0x7800, map_list_of(0,186)(1,187)(2,188)(3,189)(4,190)(5,191)(6,192)(7,193)(8,194)(9,195)(10,196)(11,197)(12,198)(13,199)(14,200)(15,201),-1);
	main_decoder_table[186]=aarch64_mask_entry(0x0, branchMap(),465);
	main_decoder_table[187]=aarch64_mask_entry(0x0, branchMap(),495);
	main_decoder_table[188]=aarch64_mask_entry(0x0, branchMap(),540);
	main_decoder_table[189]=aarch64_mask_entry(0xc00000, map_list_of(0,202)(1,203)(2,204)(3,205),-1);
	main_decoder_table[202]=aarch64_mask_entry(0x0, branchMap(),23);
	main_decoder_table[203]=aarch64_mask_entry(0x0, branchMap(),38);
	main_decoder_table[204]=aarch64_mask_entry(0x0, branchMap(),379);
	main_decoder_table[205]=aarch64_mask_entry(0x0, branchMap(),405);
	main_decoder_table[190]=aarch64_mask_entry(0x0, branchMap(),470);
	main_decoder_table[191]=aarch64_mask_entry(0x0, branchMap(),535);
	main_decoder_table[192]=aarch64_mask_entry(0x0, branchMap(),70);
	main_decoder_table[193]=aarch64_mask_entry(0x0, branchMap(),66);
	main_decoder_table[194]=aarch64_mask_entry(0x0, branchMap(),550);
	main_decoder_table[195]=aarch64_mask_entry(0x0, branchMap(),527);
	main_decoder_table[196]=aarch64_mask_entry(0x0, branchMap(),544);
	main_decoder_table[197]=aarch64_mask_entry(0x0, branchMap(),519);
	main_decoder_table[198]=aarch64_mask_entry(0x0, branchMap(),474);
	main_decoder_table[199]=aarch64_mask_entry(0x0, branchMap(),478);
	main_decoder_table[200]=aarch64_mask_entry(0x0, branchMap(),434);
	main_decoder_table[201]=aarch64_mask_entry(0x0, branchMap(),432);
	main_decoder_table[131]=aarch64_mask_entry(0x7800, map_list_of(0,206)(1,207)(2,208)(3,209)(4,210)(5,211)(6,212)(7,213)(8,214)(9,215)(10,216)(11,217)(12,218)(13,219)(15,220),-1);
	main_decoder_table[206]=aarch64_mask_entry(0x0, branchMap(),482);
	main_decoder_table[207]=aarch64_mask_entry(0x1f0000, map_list_of(0,221)(1,222),-1);
	main_decoder_table[221]=aarch64_mask_entry(0x0, branchMap(),72);
	main_decoder_table[222]=aarch64_mask_entry(0x800000, map_list_of(0,223)(1,224),-1);
	main_decoder_table[223]=aarch64_mask_entry(0x0, branchMap(),248);
	main_decoder_table[224]=aarch64_mask_entry(0x0, branchMap(),250);
	main_decoder_table[208]=aarch64_mask_entry(0x0, branchMap(),499);
	main_decoder_table[209]=aarch64_mask_entry(0x1f0000, map_list_of(0,225)(1,226),-1);
	main_decoder_table[225]=aarch64_mask_entry(0x0, branchMap(),64);
	main_decoder_table[226]=aarch64_mask_entry(0x800000, map_list_of(0,227)(1,228),-1);
	main_decoder_table[227]=aarch64_mask_entry(0x0, branchMap(),246);
	main_decoder_table[228]=aarch64_mask_entry(0x0, branchMap(),254);
	main_decoder_table[210]=aarch64_mask_entry(0x0, branchMap(),484);
	main_decoder_table[211]=aarch64_mask_entry(0x1f0000, map_list_of(0,229)(1,230)(16,231)(17,232),-1);
	main_decoder_table[229]=aarch64_mask_entry(0x0, branchMap(),80);
	main_decoder_table[230]=aarch64_mask_entry(0x800000, map_list_of(0,233)(1,234),-1);
	main_decoder_table[233]=aarch64_mask_entry(0x0, branchMap(),165);
	main_decoder_table[234]=aarch64_mask_entry(0x0, branchMap(),171);
	main_decoder_table[231]=aarch64_mask_entry(0x0, branchMap(),476);
	main_decoder_table[232]=aarch64_mask_entry(0x0, branchMap(),480);
	main_decoder_table[212]=aarch64_mask_entry(0x0, branchMap(),503);
	main_decoder_table[213]=aarch64_mask_entry(0x1f0000, map_list_of(0,235)(1,236)(17,237),-1);
	main_decoder_table[235]=aarch64_mask_entry(0x0, branchMap(),2);
	main_decoder_table[236]=aarch64_mask_entry(0x800000, map_list_of(0,238)(1,239),-1);
	main_decoder_table[238]=aarch64_mask_entry(0x0, branchMap(),158);
	main_decoder_table[239]=aarch64_mask_entry(0x0, branchMap(),181);
	main_decoder_table[237]=aarch64_mask_entry(0x0, branchMap(),16);
	main_decoder_table[214]=aarch64_mask_entry(0x0, branchMap(),490);
	main_decoder_table[215]=aarch64_mask_entry(0x9f0000, map_list_of(1,240)(32,241)(33,242),-1);
	main_decoder_table[240]=aarch64_mask_entry(0x0, branchMap(),151);
	main_decoder_table[241]=aarch64_mask_entry(0x0, branchMap(),141);
	main_decoder_table[242]=aarch64_mask_entry(0x0, branchMap(),698);
	main_decoder_table[216]=aarch64_mask_entry(0x0, branchMap(),511);
	main_decoder_table[217]=aarch64_mask_entry(0x9f0000, map_list_of(1,243)(32,244)(33,245),-1);
	main_decoder_table[243]=aarch64_mask_entry(0x0, branchMap(),449);
	main_decoder_table[244]=aarch64_mask_entry(0x0, branchMap(),133);
	main_decoder_table[245]=aarch64_mask_entry(0x0, branchMap(),238);
	main_decoder_table[218]=aarch64_mask_entry(0x0, branchMap(),412);
	main_decoder_table[219]=aarch64_mask_entry(0x0, branchMap(),145);
	main_decoder_table[220]=aarch64_mask_entry(0x0, branchMap(),118);
	main_decoder_table[132]=aarch64_mask_entry(0x7800, map_list_of(0,246)(1,247)(2,248)(3,249)(4,250)(5,251)(6,252)(7,253)(8,254)(9,255)(10,256)(11,257)(12,258)(14,259)(15,260),-1);
	main_decoder_table[246]=aarch64_mask_entry(0x0, branchMap(),9);
	main_decoder_table[247]=aarch64_mask_entry(0x0, branchMap(),88);
	main_decoder_table[248]=aarch64_mask_entry(0x0, branchMap(),369);
	main_decoder_table[249]=aarch64_mask_entry(0x0, branchMap(),392);
	main_decoder_table[250]=aarch64_mask_entry(0x0, branchMap(),475);
	main_decoder_table[251]=aarch64_mask_entry(0x0, branchMap(),479);
	main_decoder_table[252]=aarch64_mask_entry(0x0, branchMap(),507);
	main_decoder_table[253]=aarch64_mask_entry(0x0, branchMap(),12);
	main_decoder_table[254]=aarch64_mask_entry(0x800000, map_list_of(0,261)(1,262),-1);
	main_decoder_table[261]=aarch64_mask_entry(0x0, branchMap(),195);
	main_decoder_table[262]=aarch64_mask_entry(0x0, branchMap(),205);
	main_decoder_table[255]=aarch64_mask_entry(0x800000, map_list_of(0,263)(1,264),-1);
	main_decoder_table[263]=aarch64_mask_entry(0x0, branchMap(),215);
	main_decoder_table[264]=aarch64_mask_entry(0x0, branchMap(),218);
	main_decoder_table[256]=aarch64_mask_entry(0x800000, map_list_of(0,265)(1,266),-1);
	main_decoder_table[265]=aarch64_mask_entry(0x0, branchMap(),124);
	main_decoder_table[266]=aarch64_mask_entry(0x0, branchMap(),262);
	main_decoder_table[257]=aarch64_mask_entry(0x0, branchMap(),231);
	main_decoder_table[258]=aarch64_mask_entry(0x0, branchMap(),131);
	main_decoder_table[259]=aarch64_mask_entry(0x800000, map_list_of(0,267)(1,268),-1);
	main_decoder_table[267]=aarch64_mask_entry(0x0, branchMap(),193);
	main_decoder_table[268]=aarch64_mask_entry(0x0, branchMap(),203);
	main_decoder_table[260]=aarch64_mask_entry(0x800000, map_list_of(0,269)(1,270),-1);
	main_decoder_table[269]=aarch64_mask_entry(0x0, branchMap(),240);
	main_decoder_table[270]=aarch64_mask_entry(0x0, branchMap(),259);
	main_decoder_table[133]=aarch64_mask_entry(0x0, branchMap(),114);
	main_decoder_table[134]=aarch64_mask_entry(0x40c00000, map_list_of(4,271),-1);
	main_decoder_table[271]=aarch64_mask_entry(0x0, branchMap(),268);
	main_decoder_table[135]=aarch64_mask_entry(0x7800, map_list_of(0,272)(1,273)(2,274)(4,275)(5,276)(6,277)(7,278)(8,279)(9,280)(10,281)(11,282)(12,283)(13,284)(14,285)(15,286),-1);
	main_decoder_table[272]=aarch64_mask_entry(0x0, branchMap(),648);
	main_decoder_table[273]=aarch64_mask_entry(0x0, branchMap(),424);
	main_decoder_table[274]=aarch64_mask_entry(0x0, branchMap(),651);
	main_decoder_table[275]=aarch64_mask_entry(0x0, branchMap(),716);
	main_decoder_table[276]=aarch64_mask_entry(0x1f0000, map_list_of(0,287)(1,288),-1);
	main_decoder_table[287]=aarch64_mask_entry(0x0, branchMap(),649);
	main_decoder_table[288]=aarch64_mask_entry(0x0, branchMap(),539);
	main_decoder_table[277]=aarch64_mask_entry(0x0, branchMap(),717);
	main_decoder_table[278]=aarch64_mask_entry(0x1f0000, map_list_of(0,289)(1,290)(16,291),-1);
	main_decoder_table[289]=aarch64_mask_entry(0x0, branchMap(),713);
	main_decoder_table[290]=aarch64_mask_entry(0x0, branchMap(),468);
	main_decoder_table[291]=aarch64_mask_entry(0x0, branchMap(),650);
	main_decoder_table[279]=aarch64_mask_entry(0x0, branchMap(),417);
	main_decoder_table[280]=aarch64_mask_entry(0x1f0000, map_list_of(0,292)(1,293),-1);
	main_decoder_table[292]=aarch64_mask_entry(0x0, branchMap(),59);
	main_decoder_table[293]=aarch64_mask_entry(0x0, branchMap(),697);
	main_decoder_table[281]=aarch64_mask_entry(0x0, branchMap(),644);
	main_decoder_table[282]=aarch64_mask_entry(0xdf0000, map_list_of(0,294)(32,295),-1);
	main_decoder_table[294]=aarch64_mask_entry(0x0, branchMap(),394);
	main_decoder_table[295]=aarch64_mask_entry(0x0, branchMap(),418);
	main_decoder_table[283]=aarch64_mask_entry(0x0, branchMap(),431);
	main_decoder_table[284]=aarch64_mask_entry(0x1f0000, map_list_of(0,296)(1,297),-1);
	main_decoder_table[296]=aarch64_mask_entry(0x0, branchMap(),647);
	main_decoder_table[297]=aarch64_mask_entry(0x0, branchMap(),177);
	main_decoder_table[285]=aarch64_mask_entry(0x0, branchMap(),646);
	main_decoder_table[286]=aarch64_mask_entry(0x0, branchMap(),513);
	main_decoder_table[136]=aarch64_mask_entry(0x7800, map_list_of(0,298)(1,299)(2,300)(3,301)(4,302)(5,303)(6,304)(7,305)(8,306)(9,307)(10,308)(11,309)(12,310)(13,311)(14,312)(15,313),-1);
	main_decoder_table[298]=aarch64_mask_entry(0x0, branchMap(),662);
	main_decoder_table[299]=aarch64_mask_entry(0x0, branchMap(),683);
	main_decoder_table[300]=aarch64_mask_entry(0x0, branchMap(),699);
	main_decoder_table[301]=aarch64_mask_entry(0xc00000, map_list_of(0,314)(1,315)(2,316)(3,317),-1);
	main_decoder_table[314]=aarch64_mask_entry(0x0, branchMap(),110);
	main_decoder_table[315]=aarch64_mask_entry(0x0, branchMap(),47);
	main_decoder_table[316]=aarch64_mask_entry(0x0, branchMap(),42);
	main_decoder_table[317]=aarch64_mask_entry(0x0, branchMap(),41);
	main_decoder_table[302]=aarch64_mask_entry(0x0, branchMap(),663);
	main_decoder_table[303]=aarch64_mask_entry(0x0, branchMap(),695);
	main_decoder_table[304]=aarch64_mask_entry(0x0, branchMap(),74);
	main_decoder_table[305]=aarch64_mask_entry(0x0, branchMap(),76);
	main_decoder_table[306]=aarch64_mask_entry(0x0, branchMap(),708);
	main_decoder_table[307]=aarch64_mask_entry(0x0, branchMap(),691);
	main_decoder_table[308]=aarch64_mask_entry(0x0, branchMap(),701);
	main_decoder_table[309]=aarch64_mask_entry(0x0, branchMap(),685);
	main_decoder_table[310]=aarch64_mask_entry(0x0, branchMap(),665);
	main_decoder_table[311]=aarch64_mask_entry(0x0, branchMap(),668);
	main_decoder_table[312]=aarch64_mask_entry(0x0, branchMap(),645);
	main_decoder_table[313]=aarch64_mask_entry(0x0, branchMap(),643);
	main_decoder_table[137]=aarch64_mask_entry(0x7800, map_list_of(0,318)(1,319)(3,320)(4,321)(5,322)(7,323)(8,324)(9,325)(11,326)(15,327),-1);
	main_decoder_table[318]=aarch64_mask_entry(0x0, branchMap(),672);
	main_decoder_table[319]=aarch64_mask_entry(0x1f0000, map_list_of(0,328)(1,329),-1);
	main_decoder_table[328]=aarch64_mask_entry(0x0, branchMap(),68);
	main_decoder_table[329]=aarch64_mask_entry(0x0, branchMap(),242);
	main_decoder_table[320]=aarch64_mask_entry(0x1f0000, map_list_of(0,330)(1,331),-1);
	main_decoder_table[330]=aarch64_mask_entry(0x0, branchMap(),78);
	main_decoder_table[331]=aarch64_mask_entry(0x800000, map_list_of(0,332)(1,333),-1);
	main_decoder_table[332]=aarch64_mask_entry(0x0, branchMap(),252);
	main_decoder_table[333]=aarch64_mask_entry(0x0, branchMap(),244);
	main_decoder_table[321]=aarch64_mask_entry(0x0, branchMap(),674);
	main_decoder_table[322]=aarch64_mask_entry(0x1f0000, map_list_of(1,334)(16,335)(17,336),-1);
	main_decoder_table[334]=aarch64_mask_entry(0x800000, map_list_of(0,337)(1,338),-1);
	main_decoder_table[337]=aarch64_mask_entry(0x0, branchMap(),168);
	main_decoder_table[338]=aarch64_mask_entry(0x0, branchMap(),174);
	main_decoder_table[335]=aarch64_mask_entry(0x0, branchMap(),667);
	main_decoder_table[336]=aarch64_mask_entry(0x0, branchMap(),670);
	main_decoder_table[323]=aarch64_mask_entry(0x1f0000, map_list_of(0,339)(1,340),-1);
	main_decoder_table[339]=aarch64_mask_entry(0x0, branchMap(),398);
	main_decoder_table[340]=aarch64_mask_entry(0x800000, map_list_of(0,341)(1,342),-1);
	main_decoder_table[341]=aarch64_mask_entry(0x0, branchMap(),161);
	main_decoder_table[342]=aarch64_mask_entry(0x0, branchMap(),187);
	main_decoder_table[324]=aarch64_mask_entry(0x0, branchMap(),680);
	main_decoder_table[325]=aarch64_mask_entry(0x9f0000, map_list_of(1,343)(16,344)(32,345)(33,346)(48,347),-1);
	main_decoder_table[343]=aarch64_mask_entry(0x0, branchMap(),154);
	main_decoder_table[344]=aarch64_mask_entry(0x0, branchMap(),199);
	main_decoder_table[345]=aarch64_mask_entry(0x0, branchMap(),137);
	main_decoder_table[346]=aarch64_mask_entry(0x0, branchMap(),704);
	main_decoder_table[347]=aarch64_mask_entry(0x0, branchMap(),209);
	main_decoder_table[326]=aarch64_mask_entry(0x9f0000, map_list_of(1,348)(32,349)(33,350),-1);
	main_decoder_table[348]=aarch64_mask_entry(0x0, branchMap(),658);
	main_decoder_table[349]=aarch64_mask_entry(0x0, branchMap(),143);
	main_decoder_table[350]=aarch64_mask_entry(0x0, branchMap(),257);
	main_decoder_table[327]=aarch64_mask_entry(0x9f0000, map_list_of(16,351)(32,352)(33,353)(48,354),-1);
	main_decoder_table[351]=aarch64_mask_entry(0x0, branchMap(),202);
	main_decoder_table[352]=aarch64_mask_entry(0x0, branchMap(),232);
	main_decoder_table[353]=aarch64_mask_entry(0x0, branchMap(),260);
	main_decoder_table[354]=aarch64_mask_entry(0x0, branchMap(),212);
	main_decoder_table[138]=aarch64_mask_entry(0x7800, map_list_of(0,355)(1,356)(2,357)(3,358)(4,359)(5,360)(6,361)(8,362)(10,363)(11,364)(12,365)(13,366)(14,367)(15,368),-1);
	main_decoder_table[355]=aarch64_mask_entry(0x0, branchMap(),620);
	main_decoder_table[356]=aarch64_mask_entry(0x0, branchMap(),62);
	main_decoder_table[357]=aarch64_mask_entry(0x0, branchMap(),371);
	main_decoder_table[358]=aarch64_mask_entry(0x0, branchMap(),411);
	main_decoder_table[359]=aarch64_mask_entry(0x0, branchMap(),666);
	main_decoder_table[360]=aarch64_mask_entry(0x0, branchMap(),669);
	main_decoder_table[361]=aarch64_mask_entry(0x0, branchMap(),517);
	main_decoder_table[362]=aarch64_mask_entry(0x800000, map_list_of(0,369)(1,370),-1);
	main_decoder_table[369]=aarch64_mask_entry(0x0, branchMap(),198);
	main_decoder_table[370]=aarch64_mask_entry(0x0, branchMap(),208);
	main_decoder_table[363]=aarch64_mask_entry(0x800000, map_list_of(0,371)(1,372),-1);
	main_decoder_table[371]=aarch64_mask_entry(0x0, branchMap(),127);
	main_decoder_table[372]=aarch64_mask_entry(0x0, branchMap(),117);
	main_decoder_table[364]=aarch64_mask_entry(0x0, branchMap(),226);
	main_decoder_table[365]=aarch64_mask_entry(0x800000, map_list_of(0,373)(1,374),-1);
	main_decoder_table[373]=aarch64_mask_entry(0x0, branchMap(),135);
	main_decoder_table[374]=aarch64_mask_entry(0x0, branchMap(),139);
	main_decoder_table[366]=aarch64_mask_entry(0x800000, map_list_of(0,375)(1,376),-1);
	main_decoder_table[375]=aarch64_mask_entry(0x0, branchMap(),121);
	main_decoder_table[376]=aarch64_mask_entry(0x0, branchMap(),123);
	main_decoder_table[367]=aarch64_mask_entry(0x800000, map_list_of(0,377)(1,378),-1);
	main_decoder_table[377]=aarch64_mask_entry(0x0, branchMap(),201);
	main_decoder_table[378]=aarch64_mask_entry(0x0, branchMap(),211);
	main_decoder_table[368]=aarch64_mask_entry(0x0, branchMap(),190);
	main_decoder_table[12]=aarch64_mask_entry(0x80000400, map_list_of(0,379)(1,380),-1);
	main_decoder_table[379]=aarch64_mask_entry(0x2000f000, map_list_of(1,381)(2,382)(3,383)(5,384)(6,385)(7,386)(8,387)(9,388)(10,389)(11,390)(12,391)(13,392)(16,393)(18,394)(20,395)(22,396)(25,397)(26,398),-1);
	main_decoder_table[381]=aarch64_mask_entry(0x0, branchMap(),214);
	main_decoder_table[382]=aarch64_mask_entry(0x0, branchMap(),481);
	main_decoder_table[383]=aarch64_mask_entry(0x0, branchMap(),497);
	main_decoder_table[384]=aarch64_mask_entry(0x0, branchMap(),217);
	main_decoder_table[385]=aarch64_mask_entry(0x0, branchMap(),483);
	main_decoder_table[386]=aarch64_mask_entry(0x0, branchMap(),501);
	main_decoder_table[387]=aarch64_mask_entry(0x0, branchMap(),391);
	main_decoder_table[388]=aarch64_mask_entry(0x0, branchMap(),225);
	main_decoder_table[389]=aarch64_mask_entry(0x0, branchMap(),489);
	main_decoder_table[390]=aarch64_mask_entry(0x0, branchMap(),509);
	main_decoder_table[391]=aarch64_mask_entry(0x0, branchMap(),505);
	main_decoder_table[392]=aarch64_mask_entry(0x0, branchMap(),515);
	main_decoder_table[393]=aarch64_mask_entry(0x0, branchMap(),368);
	main_decoder_table[394]=aarch64_mask_entry(0x0, branchMap(),671);
	main_decoder_table[395]=aarch64_mask_entry(0x0, branchMap(),370);
	main_decoder_table[396]=aarch64_mask_entry(0x0, branchMap(),673);
	main_decoder_table[397]=aarch64_mask_entry(0x0, branchMap(),229);
	main_decoder_table[398]=aarch64_mask_entry(0x0, branchMap(),679);
	main_decoder_table[380]=aarch64_mask_entry(0x800800, map_list_of(0,399)(1,400),-1);
	main_decoder_table[399]=aarch64_mask_entry(0x0, branchMap(),383);
	main_decoder_table[400]=aarch64_mask_entry(0x2000f000, map_list_of(8,401)(9,402)(15,403)(24,404)(25,405)(31,406),-1);
	main_decoder_table[401]=aarch64_mask_entry(0x0, branchMap(),430);
	main_decoder_table[402]=aarch64_mask_entry(0x0, branchMap(),521);
	main_decoder_table[403]=aarch64_mask_entry(0x0, branchMap(),179);
	main_decoder_table[404]=aarch64_mask_entry(0x0, branchMap(),523);
	main_decoder_table[405]=aarch64_mask_entry(0x0, branchMap(),687);
	main_decoder_table[406]=aarch64_mask_entry(0x0, branchMap(),185);
	main_decoder_table[3]=aarch64_mask_entry(0x4000000, map_list_of(0,407)(1,408),-1);
	main_decoder_table[407]=aarch64_mask_entry(0x3000000, map_list_of(0,409)(1,410)(2,411)(3,412),-1);
	main_decoder_table[409]=aarch64_mask_entry(0x80000000, map_list_of(0,413)(1,414),-1);
	main_decoder_table[413]=aarch64_mask_entry(0x0, branchMap(),17);
	main_decoder_table[414]=aarch64_mask_entry(0x0, branchMap(),18);
	main_decoder_table[410]=aarch64_mask_entry(0x60000000, map_list_of(0,415)(1,416)(2,417)(3,418),-1);
	main_decoder_table[415]=aarch64_mask_entry(0x0, branchMap(),6);
	main_decoder_table[416]=aarch64_mask_entry(0x0, branchMap(),14);
	main_decoder_table[417]=aarch64_mask_entry(0x0, branchMap(),617);
	main_decoder_table[418]=aarch64_mask_entry(0x0, branchMap(),623);
	main_decoder_table[411]=aarch64_mask_entry(0x60800000, map_list_of(0,419)(1,420)(2,421)(4,422)(5,423)(6,424)(7,425),-1);
	main_decoder_table[419]=aarch64_mask_entry(0x0, branchMap(),24);
	main_decoder_table[420]=aarch64_mask_entry(0x0, branchMap(),377);
	main_decoder_table[421]=aarch64_mask_entry(0x0, branchMap(),409);
	main_decoder_table[422]=aarch64_mask_entry(0x0, branchMap(),111);
	main_decoder_table[423]=aarch64_mask_entry(0x0, branchMap(),378);
	main_decoder_table[424]=aarch64_mask_entry(0x0, branchMap(),26);
	main_decoder_table[425]=aarch64_mask_entry(0x0, branchMap(),384);
	main_decoder_table[412]=aarch64_mask_entry(0x60800000, map_list_of(0,426)(1,427)(2,428)(4,429),-1);
	main_decoder_table[426]=aarch64_mask_entry(0x0, branchMap(),443);
	main_decoder_table[427]=aarch64_mask_entry(0x200000, map_list_of(0,430),-1);
	main_decoder_table[430]=aarch64_mask_entry(0x0, branchMap(),115);
	main_decoder_table[428]=aarch64_mask_entry(0x0, branchMap(),34);
	main_decoder_table[429]=aarch64_mask_entry(0x0, branchMap(),362);
	main_decoder_table[408]=aarch64_mask_entry(0x60000000, map_list_of(0,431)(1,432)(2,433),-1);
	main_decoder_table[431]=aarch64_mask_entry(0x80000000, map_list_of(0,434)(1,435),-1);
	main_decoder_table[434]=aarch64_mask_entry(0x0, branchMap(),33);
	main_decoder_table[435]=aarch64_mask_entry(0x0, branchMap(),43);
	main_decoder_table[432]=aarch64_mask_entry(0x3000000, map_list_of(0,436)(1,437)(2,438)(3,439),-1);
	main_decoder_table[436]=aarch64_mask_entry(0x0, branchMap(),49);
	main_decoder_table[437]=aarch64_mask_entry(0x0, branchMap(),48);
	main_decoder_table[438]=aarch64_mask_entry(0x0, branchMap(),637);
	main_decoder_table[439]=aarch64_mask_entry(0x0, branchMap(),635);
	main_decoder_table[433]=aarch64_mask_entry(0x83000000, map_list_of(0,440)(4,441)(5,442)(6,443),-1);
	main_decoder_table[440]=aarch64_mask_entry(0x0, branchMap(),32);
	main_decoder_table[441]=aarch64_mask_entry(0xe0001f, map_list_of(1,444)(2,445)(3,446)(32,447)(64,448)(161,449)(162,450)(163,451),-1);
	main_decoder_table[444]=aarch64_mask_entry(0x0, branchMap(),627);
	main_decoder_table[445]=aarch64_mask_entry(0x0, branchMap(),266);
	main_decoder_table[446]=aarch64_mask_entry(0x0, branchMap(),477);
	main_decoder_table[447]=aarch64_mask_entry(0x0, branchMap(),46);
	main_decoder_table[448]=aarch64_mask_entry(0x0, branchMap(),265);
	main_decoder_table[449]=aarch64_mask_entry(0x0, branchMap(),100);
	main_decoder_table[450]=aarch64_mask_entry(0x0, branchMap(),101);
	main_decoder_table[451]=aarch64_mask_entry(0x0, branchMap(),102);
	main_decoder_table[442]=aarch64_mask_entry(0xf00000, map_list_of(0,452)(1,453)(2,454)(3,455),-1);
	main_decoder_table[452]=aarch64_mask_entry(0x80000, map_list_of(0,456)(1,457),-1);
	main_decoder_table[456]=aarch64_mask_entry(0xf01f, map_list_of(95,458)(127,459)(159,460),-1);
	main_decoder_table[458]=aarch64_mask_entry(0x70000, map_list_of(3,461),-1);
	main_decoder_table[461]=aarch64_mask_entry(0x0, branchMap(),264);
	main_decoder_table[459]=aarch64_mask_entry(0x700e0, map_list_of(26,462)(28,463)(29,464)(30,465),-1);
	main_decoder_table[462]=aarch64_mask_entry(0x0, branchMap(),56);
	main_decoder_table[463]=aarch64_mask_entry(0x0, branchMap(),105);
	main_decoder_table[464]=aarch64_mask_entry(0x0, branchMap(),103);
	main_decoder_table[465]=aarch64_mask_entry(0x0, branchMap(),270);
	main_decoder_table[460]=aarch64_mask_entry(0x0, branchMap(),388);
	main_decoder_table[457]=aarch64_mask_entry(0x0, branchMap(),632);
	main_decoder_table[453]=aarch64_mask_entry(0x0, branchMap(),389);
	main_decoder_table[454]=aarch64_mask_entry(0x0, branchMap(),633);
	main_decoder_table[455]=aarch64_mask_entry(0x0, branchMap(),387);
	main_decoder_table[443]=aarch64_mask_entry(0xfffc1f, map_list_of(63488,466)(129024,467)(194560,468)(325632,469)(391168,470),-1);
	main_decoder_table[466]=aarch64_mask_entry(0x0, branchMap(),45);
	main_decoder_table[467]=aarch64_mask_entry(0x0, branchMap(),44);
	main_decoder_table[468]=aarch64_mask_entry(0x0, branchMap(),420);
	main_decoder_table[469]=aarch64_mask_entry(0x0, branchMap(),113);
	main_decoder_table[470]=aarch64_mask_entry(0x0, branchMap(),104);
	main_decoder_table[4]=aarch64_mask_entry(0x27000000, map_list_of(0,471)(2,472)(3,473)(4,474)(6,475)(7,476)(8,477)(9,478)(10,479)(12,480)(13,481)(14,482)(15,483),-1);
	main_decoder_table[471]=aarch64_mask_entry(0x80000000, map_list_of(0,484)(1,485),-1);
	main_decoder_table[484]=aarch64_mask_entry(0x0, branchMap(),320);
	main_decoder_table[485]=aarch64_mask_entry(0x40000000, map_list_of(0,486)(1,487),-1);
	main_decoder_table[486]=aarch64_mask_entry(0x0, branchMap(),342);
	main_decoder_table[487]=aarch64_mask_entry(0x0, branchMap(),414);
	main_decoder_table[472]=aarch64_mask_entry(0x40e00000, map_list_of(0,488)(4,489)(6,490)(8,491)(12,492)(14,493),-1);
	main_decoder_table[488]=aarch64_mask_entry(0x0, branchMap(),3);
	main_decoder_table[489]=aarch64_mask_entry(0xc00, map_list_of(0,494)(1,495),-1);
	main_decoder_table[494]=aarch64_mask_entry(0x0, branchMap(),93);
	main_decoder_table[495]=aarch64_mask_entry(0x0, branchMap(),54);
	main_decoder_table[490]=aarch64_mask_entry(0xf000, map_list_of(0,496)(2,497)(4,498)(5,499),-1);
	main_decoder_table[496]=aarch64_mask_entry(0xc00, map_list_of(2,500)(3,501),-1);
	main_decoder_table[500]=aarch64_mask_entry(0x0, branchMap(),661);
	main_decoder_table[501]=aarch64_mask_entry(0x0, branchMap(),452);
	main_decoder_table[497]=aarch64_mask_entry(0xc00, map_list_of(0,502)(1,503)(2,504)(3,505),-1);
	main_decoder_table[502]=aarch64_mask_entry(0x0, branchMap(),361);
	main_decoder_table[503]=aarch64_mask_entry(0x0, branchMap(),364);
	main_decoder_table[504]=aarch64_mask_entry(0x0, branchMap(),28);
	main_decoder_table[505]=aarch64_mask_entry(0x0, branchMap(),428);
	main_decoder_table[498]=aarch64_mask_entry(0x0, branchMap(),91);
	main_decoder_table[499]=aarch64_mask_entry(0x0, branchMap(),92);
	main_decoder_table[491]=aarch64_mask_entry(0xfc00, map_list_of(0,506),-1);
	main_decoder_table[506]=aarch64_mask_entry(0x0, branchMap(),441);
	main_decoder_table[492]=aarch64_mask_entry(0xc00, map_list_of(0,507)(1,508),-1);
	main_decoder_table[507]=aarch64_mask_entry(0x0, branchMap(),55);
	main_decoder_table[508]=aarch64_mask_entry(0x0, branchMap(),89);
	main_decoder_table[493]=aarch64_mask_entry(0x1ff800, map_list_of(0,509)(1,510)(2,511),-1);
	main_decoder_table[509]=aarch64_mask_entry(0x400, map_list_of(0,512)(1,513),-1);
	main_decoder_table[512]=aarch64_mask_entry(0x0, branchMap(),419);
	main_decoder_table[513]=aarch64_mask_entry(0x0, branchMap(),423);
	main_decoder_table[510]=aarch64_mask_entry(0x0, branchMap(),421);
	main_decoder_table[511]=aarch64_mask_entry(0x400, map_list_of(0,514)(1,515),-1);
	main_decoder_table[514]=aarch64_mask_entry(0x0, branchMap(),60);
	main_decoder_table[515]=aarch64_mask_entry(0x0, branchMap(),58);
	main_decoder_table[473]=aarch64_mask_entry(0x40e08000, map_list_of(0,516)(1,517)(2,518)(3,519)(4,520)(10,521)(11,522)(12,523),-1);
	main_decoder_table[516]=aarch64_mask_entry(0x0, branchMap(),367);
	main_decoder_table[517]=aarch64_mask_entry(0x0, branchMap(),390);
	main_decoder_table[518]=aarch64_mask_entry(0x80000000, map_list_of(1,524),-1);
	main_decoder_table[524]=aarch64_mask_entry(0x0, branchMap(),473);
	main_decoder_table[519]=aarch64_mask_entry(0x80000000, map_list_of(1,525),-1);
	main_decoder_table[525]=aarch64_mask_entry(0x0, branchMap(),487);
	main_decoder_table[520]=aarch64_mask_entry(0x0, branchMap(),488);
	main_decoder_table[521]=aarch64_mask_entry(0x80000000, map_list_of(1,526),-1);
	main_decoder_table[526]=aarch64_mask_entry(0x0, branchMap(),664);
	main_decoder_table[522]=aarch64_mask_entry(0x80000000, map_list_of(1,527),-1);
	main_decoder_table[527]=aarch64_mask_entry(0x0, branchMap(),677);
	main_decoder_table[523]=aarch64_mask_entry(0x0, branchMap(),678);
	main_decoder_table[474]=aarch64_mask_entry(0x0, branchMap(),319);
	main_decoder_table[475]=aarch64_mask_entry(0x40200000, map_list_of(0,528)(1,529)(2,530)(3,531),-1);
	main_decoder_table[528]=aarch64_mask_entry(0x9f0000, map_list_of(2,532)(3,533)(24,534)(25,535),-1);
	main_decoder_table[532]=aarch64_mask_entry(0x0, branchMap(),450);
	main_decoder_table[533]=aarch64_mask_entry(0x0, branchMap(),659);
	main_decoder_table[534]=aarch64_mask_entry(0x0, branchMap(),182);
	main_decoder_table[535]=aarch64_mask_entry(0x0, branchMap(),188);
	main_decoder_table[529]=aarch64_mask_entry(0xc00, map_list_of(0,536)(1,537)(2,538)(3,539),-1);
	main_decoder_table[536]=aarch64_mask_entry(0x1000, map_list_of(0,540)(1,541),-1);
	main_decoder_table[540]=aarch64_mask_entry(0x6000, map_list_of(0,542)(1,543)(2,544),-1);
	main_decoder_table[542]=aarch64_mask_entry(0x168000, map_list_of(0,545)(2,546)(4,547)(6,548)(8,549),-1);
	main_decoder_table[545]=aarch64_mask_entry(0x890000, map_list_of(0,550)(1,551)(2,552)(3,553),-1);
	main_decoder_table[550]=aarch64_mask_entry(0x0, branchMap(),166);
	main_decoder_table[551]=aarch64_mask_entry(0x0, branchMap(),169);
	main_decoder_table[552]=aarch64_mask_entry(0x0, branchMap(),172);
	main_decoder_table[553]=aarch64_mask_entry(0x0, branchMap(),175);
	main_decoder_table[546]=aarch64_mask_entry(0x890000, map_list_of(0,554)(1,555),-1);
	main_decoder_table[554]=aarch64_mask_entry(0x0, branchMap(),451);
	main_decoder_table[555]=aarch64_mask_entry(0x0, branchMap(),660);
	main_decoder_table[547]=aarch64_mask_entry(0x890000, map_list_of(0,556)(1,557),-1);
	main_decoder_table[556]=aarch64_mask_entry(0x0, branchMap(),152);
	main_decoder_table[557]=aarch64_mask_entry(0x0, branchMap(),155);
	main_decoder_table[548]=aarch64_mask_entry(0x0, branchMap(),221);
	main_decoder_table[549]=aarch64_mask_entry(0x890000, map_list_of(0,558)(1,559)(2,560)(3,561),-1);
	main_decoder_table[558]=aarch64_mask_entry(0x0, branchMap(),159);
	main_decoder_table[559]=aarch64_mask_entry(0x0, branchMap(),162);
	main_decoder_table[560]=aarch64_mask_entry(0x0, branchMap(),183);
	main_decoder_table[561]=aarch64_mask_entry(0x0, branchMap(),189);
	main_decoder_table[543]=aarch64_mask_entry(0x80808017, map_list_of(0,562)(8,563),-1);
	main_decoder_table[562]=aarch64_mask_entry(0x0, branchMap(),146);
	main_decoder_table[563]=aarch64_mask_entry(0x0, branchMap(),147);
	main_decoder_table[544]=aarch64_mask_entry(0x801e0000, map_list_of(0,564)(1,565)(2,566)(3,567),-1);
	main_decoder_table[564]=aarch64_mask_entry(0x818000, map_list_of(0,568)(1,569)(2,570)(3,571),-1);
	main_decoder_table[568]=aarch64_mask_entry(0x0, branchMap(),220);
	main_decoder_table[569]=aarch64_mask_entry(0x0, branchMap(),119);
	main_decoder_table[570]=aarch64_mask_entry(0x0, branchMap(),233);
	main_decoder_table[571]=aarch64_mask_entry(0x0, branchMap(),261);
	main_decoder_table[565]=aarch64_mask_entry(0x0, branchMap(),149);
	main_decoder_table[566]=aarch64_mask_entry(0x818000, map_list_of(0,572)(1,573)(2,574)(3,575),-1);
	main_decoder_table[572]=aarch64_mask_entry(0x0, branchMap(),249);
	main_decoder_table[573]=aarch64_mask_entry(0x0, branchMap(),251);
	main_decoder_table[574]=aarch64_mask_entry(0x0, branchMap(),247);
	main_decoder_table[575]=aarch64_mask_entry(0x0, branchMap(),255);
	main_decoder_table[567]=aarch64_mask_entry(0x818000, map_list_of(0,576)(2,577)(3,578),-1);
	main_decoder_table[576]=aarch64_mask_entry(0x0, branchMap(),243);
	main_decoder_table[577]=aarch64_mask_entry(0x0, branchMap(),253);
	main_decoder_table[578]=aarch64_mask_entry(0x0, branchMap(),245);
	main_decoder_table[541]=aarch64_mask_entry(0x0, branchMap(),222);
	main_decoder_table[537]=aarch64_mask_entry(0x80800010, map_list_of(0,579)(1,580),-1);
	main_decoder_table[579]=aarch64_mask_entry(0x0, branchMap(),128);
	main_decoder_table[580]=aarch64_mask_entry(0x0, branchMap(),129);
	main_decoder_table[538]=aarch64_mask_entry(0x8080f000, map_list_of(0,581)(1,582)(2,583)(3,584)(4,585)(5,586)(6,587)(7,588)(8,589),-1);
	main_decoder_table[581]=aarch64_mask_entry(0x0, branchMap(),227);
	main_decoder_table[582]=aarch64_mask_entry(0x0, branchMap(),191);
	main_decoder_table[583]=aarch64_mask_entry(0x0, branchMap(),125);
	main_decoder_table[584]=aarch64_mask_entry(0x0, branchMap(),263);
	main_decoder_table[585]=aarch64_mask_entry(0x0, branchMap(),194);
	main_decoder_table[586]=aarch64_mask_entry(0x0, branchMap(),204);
	main_decoder_table[587]=aarch64_mask_entry(0x0, branchMap(),196);
	main_decoder_table[588]=aarch64_mask_entry(0x0, branchMap(),206);
	main_decoder_table[589]=aarch64_mask_entry(0x0, branchMap(),236);
	main_decoder_table[539]=aarch64_mask_entry(0x0, branchMap(),148);
	main_decoder_table[530]=aarch64_mask_entry(0x80c0fc00, map_list_of(0,590)(1,591)(4,592)(8,593)(12,594)(16,595)(20,596)(24,597),-1);
	main_decoder_table[590]=aarch64_mask_entry(0x0, branchMap(),455);
	main_decoder_table[591]=aarch64_mask_entry(0x0, branchMap(),106);
	main_decoder_table[592]=aarch64_mask_entry(0x0, branchMap(),458);
	main_decoder_table[593]=aarch64_mask_entry(0x0, branchMap(),457);
	main_decoder_table[594]=aarch64_mask_entry(0x0, branchMap(),459);
	main_decoder_table[595]=aarch64_mask_entry(0x0, branchMap(),462);
	main_decoder_table[596]=aarch64_mask_entry(0x0, branchMap(),461);
	main_decoder_table[597]=aarch64_mask_entry(0x0, branchMap(),464);
	main_decoder_table[531]=aarch64_mask_entry(0x8000fc00, map_list_of(2,598)(3,599)(6,600)(10,601)(11,602)(13,603)(14,604)(15,605)(17,606)(18,607)(19,608)(21,609)(23,610)(30,611)(33,612)(34,613)(35,614)(36,615)(38,616)(42,617)(44,618)(45,619)(46,620)(50,621)(52,622)(54,623)(55,624)(57,625)(58,626)(62,627)(63,628),-1);
	main_decoder_table[598]=aarch64_mask_entry(0x0, branchMap(),456);
	main_decoder_table[599]=aarch64_mask_entry(0x0, branchMap(),494);
	main_decoder_table[600]=aarch64_mask_entry(0x0, branchMap(),460);
	main_decoder_table[601]=aarch64_mask_entry(0x0, branchMap(),463);
	main_decoder_table[602]=aarch64_mask_entry(0x0, branchMap(),534);
	main_decoder_table[603]=aarch64_mask_entry(0x0, branchMap(),69);
	main_decoder_table[604]=aarch64_mask_entry(0x0, branchMap(),625);
	main_decoder_table[605]=aarch64_mask_entry(0x0, branchMap(),65);
	main_decoder_table[606]=aarch64_mask_entry(0x0, branchMap(),549);
	main_decoder_table[607]=aarch64_mask_entry(0x0, branchMap(),536);
	main_decoder_table[608]=aarch64_mask_entry(0x0, branchMap(),526);
	main_decoder_table[609]=aarch64_mask_entry(0x0, branchMap(),543);
	main_decoder_table[610]=aarch64_mask_entry(0x0, branchMap(),518);
	main_decoder_table[611]=aarch64_mask_entry(0x0, branchMap(),492);
	main_decoder_table[612]=aarch64_mask_entry(0x0, branchMap(),8);
	main_decoder_table[613]=aarch64_mask_entry(0x0, branchMap(),71);
	main_decoder_table[614]=aarch64_mask_entry(0x0, branchMap(),87);
	main_decoder_table[615]=aarch64_mask_entry(0x0, branchMap(),498);
	main_decoder_table[616]=aarch64_mask_entry(0x0, branchMap(),63);
	main_decoder_table[617]=aarch64_mask_entry(0x1f0000, map_list_of(0,629)(1,630),-1);
	main_decoder_table[629]=aarch64_mask_entry(0x0, branchMap(),79);
	main_decoder_table[630]=aarch64_mask_entry(0x800000, map_list_of(0,631)(1,632),-1);
	main_decoder_table[631]=aarch64_mask_entry(0x0, branchMap(),164);
	main_decoder_table[632]=aarch64_mask_entry(0x0, branchMap(),170);
	main_decoder_table[618]=aarch64_mask_entry(0x0, branchMap(),502);
	main_decoder_table[619]=aarch64_mask_entry(0x0, branchMap(),506);
	main_decoder_table[620]=aarch64_mask_entry(0x1f0000, map_list_of(0,633)(1,634)(17,635),-1);
	main_decoder_table[633]=aarch64_mask_entry(0x0, branchMap(),1);
	main_decoder_table[634]=aarch64_mask_entry(0x800000, map_list_of(0,636)(1,637),-1);
	main_decoder_table[636]=aarch64_mask_entry(0x0, branchMap(),157);
	main_decoder_table[637]=aarch64_mask_entry(0x0, branchMap(),180);
	main_decoder_table[635]=aarch64_mask_entry(0x0, branchMap(),11);
	main_decoder_table[621]=aarch64_mask_entry(0x9f0000, map_list_of(1,638)(32,639),-1);
	main_decoder_table[638]=aarch64_mask_entry(0x0, branchMap(),150);
	main_decoder_table[639]=aarch64_mask_entry(0x0, branchMap(),140);
	main_decoder_table[622]=aarch64_mask_entry(0x0, branchMap(),510);
	main_decoder_table[623]=aarch64_mask_entry(0x9f0000, map_list_of(1,640)(32,641)(33,642),-1);
	main_decoder_table[640]=aarch64_mask_entry(0x0, branchMap(),448);
	main_decoder_table[641]=aarch64_mask_entry(0x0, branchMap(),132);
	main_decoder_table[642]=aarch64_mask_entry(0x0, branchMap(),237);
	main_decoder_table[624]=aarch64_mask_entry(0x0, branchMap(),230);
	main_decoder_table[625]=aarch64_mask_entry(0x0, branchMap(),130);
	main_decoder_table[626]=aarch64_mask_entry(0x0, branchMap(),144);
	main_decoder_table[627]=aarch64_mask_entry(0x0, branchMap(),241);
	main_decoder_table[628]=aarch64_mask_entry(0x800000, map_list_of(0,643)(1,644),-1);
	main_decoder_table[643]=aarch64_mask_entry(0x0, branchMap(),239);
	main_decoder_table[644]=aarch64_mask_entry(0x0, branchMap(),258);
	main_decoder_table[476]=aarch64_mask_entry(0xc0008000, map_list_of(0,645)(1,646)(2,647)(3,648),-1);
	main_decoder_table[645]=aarch64_mask_entry(0xa00000, map_list_of(0,649)(1,650),-1);
	main_decoder_table[649]=aarch64_mask_entry(0x0, branchMap(),192);
	main_decoder_table[650]=aarch64_mask_entry(0x0, branchMap(),234);
	main_decoder_table[646]=aarch64_mask_entry(0xa00000, map_list_of(0,651)(1,652),-1);
	main_decoder_table[651]=aarch64_mask_entry(0x0, branchMap(),223);
	main_decoder_table[652]=aarch64_mask_entry(0x0, branchMap(),235);
	main_decoder_table[647]=aarch64_mask_entry(0x7400, map_list_of(1,653)(2,654)(3,655)(5,656)(6,657)(7,658)(10,659)(11,660)(14,661)(15,662),-1);
	main_decoder_table[653]=aarch64_mask_entry(0x0, branchMap(),552);
	main_decoder_table[654]=aarch64_mask_entry(0x0, branchMap(),213);
	main_decoder_table[655]=aarch64_mask_entry(0x0, branchMap(),554);
	main_decoder_table[656]=aarch64_mask_entry(0x0, branchMap(),545);
	main_decoder_table[657]=aarch64_mask_entry(0x0, branchMap(),496);
	main_decoder_table[658]=aarch64_mask_entry(0x0, branchMap(),547);
	main_decoder_table[659]=aarch64_mask_entry(0x0, branchMap(),216);
	main_decoder_table[660]=aarch64_mask_entry(0x0, branchMap(),466);
	main_decoder_table[661]=aarch64_mask_entry(0x0, branchMap(),500);
	main_decoder_table[662]=aarch64_mask_entry(0x0, branchMap(),524);
	main_decoder_table[648]=aarch64_mask_entry(0x7400, map_list_of(2,663)(3,664)(6,665)(8,666)(10,667)(13,668)(15,669),-1);
	main_decoder_table[663]=aarch64_mask_entry(0x0, branchMap(),224);
	main_decoder_table[664]=aarch64_mask_entry(0x800800, map_list_of(0,670)(1,671),-1);
	main_decoder_table[670]=aarch64_mask_entry(0x0, branchMap(),530);
	main_decoder_table[671]=aarch64_mask_entry(0x0, branchMap(),520);
	main_decoder_table[665]=aarch64_mask_entry(0x0, branchMap(),508);
	main_decoder_table[666]=aarch64_mask_entry(0x0, branchMap(),504);
	main_decoder_table[667]=aarch64_mask_entry(0x0, branchMap(),514);
	main_decoder_table[668]=aarch64_mask_entry(0x0, branchMap(),446);
	main_decoder_table[669]=aarch64_mask_entry(0x0, branchMap(),178);
	main_decoder_table[477]=aarch64_mask_entry(0x80a00c00, map_list_of(0,672)(1,673)(2,674)(3,675)(6,676)(8,677)(9,678)(10,679)(11,680)(14,681)(16,682)(17,683)(18,684)(19,685)(22,686)(24,687)(25,688)(26,689)(27,690)(30,691),-1);
	main_decoder_table[672]=aarch64_mask_entry(0x40400000, map_list_of(0,692)(1,693)(2,694)(3,695),-1);
	main_decoder_table[692]=aarch64_mask_entry(0x0, branchMap(),610);
	main_decoder_table[693]=aarch64_mask_entry(0x0, branchMap(),352);
	main_decoder_table[694]=aarch64_mask_entry(0x0, branchMap(),611);
	main_decoder_table[695]=aarch64_mask_entry(0x0, branchMap(),353);
	main_decoder_table[673]=aarch64_mask_entry(0x40400000, map_list_of(0,696)(1,697)(2,698)(3,699),-1);
	main_decoder_table[696]=aarch64_mask_entry(0x0, branchMap(),597);
	main_decoder_table[697]=aarch64_mask_entry(0x0, branchMap(),323);
	main_decoder_table[698]=aarch64_mask_entry(0x0, branchMap(),601);
	main_decoder_table[699]=aarch64_mask_entry(0x0, branchMap(),327);
	main_decoder_table[674]=aarch64_mask_entry(0x40400000, map_list_of(0,700)(1,701)(2,702)(3,703),-1);
	main_decoder_table[700]=aarch64_mask_entry(0x0, branchMap(),606);
	main_decoder_table[701]=aarch64_mask_entry(0x0, branchMap(),345);
	main_decoder_table[702]=aarch64_mask_entry(0x0, branchMap(),607);
	main_decoder_table[703]=aarch64_mask_entry(0x0, branchMap(),346);
	main_decoder_table[675]=aarch64_mask_entry(0x40400000, map_list_of(0,704)(1,705)(2,706)(3,707),-1);
	main_decoder_table[704]=aarch64_mask_entry(0x0, branchMap(),598);
	main_decoder_table[705]=aarch64_mask_entry(0x0, branchMap(),324);
	main_decoder_table[706]=aarch64_mask_entry(0x0, branchMap(),602);
	main_decoder_table[707]=aarch64_mask_entry(0x0, branchMap(),328);
	main_decoder_table[676]=aarch64_mask_entry(0x40400000, map_list_of(0,708)(1,709)(2,710)(3,711),-1);
	main_decoder_table[708]=aarch64_mask_entry(0x0, branchMap(),600);
	main_decoder_table[709]=aarch64_mask_entry(0x0, branchMap(),326);
	main_decoder_table[710]=aarch64_mask_entry(0x0, branchMap(),604);
	main_decoder_table[711]=aarch64_mask_entry(0x0, branchMap(),330);
	main_decoder_table[677]=aarch64_mask_entry(0x40000000, map_list_of(0,712)(1,713),-1);
	main_decoder_table[712]=aarch64_mask_entry(0x0, branchMap(),354);
	main_decoder_table[713]=aarch64_mask_entry(0x0, branchMap(),355);
	main_decoder_table[678]=aarch64_mask_entry(0x40000000, map_list_of(0,714)(1,715),-1);
	main_decoder_table[714]=aarch64_mask_entry(0x0, branchMap(),331);
	main_decoder_table[715]=aarch64_mask_entry(0x0, branchMap(),335);
	main_decoder_table[679]=aarch64_mask_entry(0x40000000, map_list_of(0,716)(1,717),-1);
	main_decoder_table[716]=aarch64_mask_entry(0x0, branchMap(),347);
	main_decoder_table[717]=aarch64_mask_entry(0x0, branchMap(),348);
	main_decoder_table[680]=aarch64_mask_entry(0x40000000, map_list_of(0,718)(1,719),-1);
	main_decoder_table[718]=aarch64_mask_entry(0x0, branchMap(),332);
	main_decoder_table[719]=aarch64_mask_entry(0x0, branchMap(),336);
	main_decoder_table[681]=aarch64_mask_entry(0x40000000, map_list_of(0,720)(1,721),-1);
	main_decoder_table[720]=aarch64_mask_entry(0x0, branchMap(),334);
	main_decoder_table[721]=aarch64_mask_entry(0x0, branchMap(),338);
	main_decoder_table[682]=aarch64_mask_entry(0x400000, map_list_of(0,722)(1,723),-1);
	main_decoder_table[722]=aarch64_mask_entry(0x0, branchMap(),609);
	main_decoder_table[723]=aarch64_mask_entry(0x0, branchMap(),351);
	main_decoder_table[683]=aarch64_mask_entry(0x400000, map_list_of(0,724)(1,725),-1);
	main_decoder_table[724]=aarch64_mask_entry(0x0, branchMap(),592);
	main_decoder_table[725]=aarch64_mask_entry(0x0, branchMap(),316);
	main_decoder_table[684]=aarch64_mask_entry(0x400000, map_list_of(0,726)(1,727),-1);
	main_decoder_table[726]=aarch64_mask_entry(0x0, branchMap(),605);
	main_decoder_table[727]=aarch64_mask_entry(0x0, branchMap(),344);
	main_decoder_table[685]=aarch64_mask_entry(0x400000, map_list_of(0,728)(1,729),-1);
	main_decoder_table[728]=aarch64_mask_entry(0x0, branchMap(),593);
	main_decoder_table[729]=aarch64_mask_entry(0x0, branchMap(),317);
	main_decoder_table[686]=aarch64_mask_entry(0x400000, map_list_of(0,730)(1,731),-1);
	main_decoder_table[730]=aarch64_mask_entry(0x0, branchMap(),596);
	main_decoder_table[731]=aarch64_mask_entry(0x0, branchMap(),322);
	main_decoder_table[687]=aarch64_mask_entry(0x40400000, map_list_of(0,732)(2,733),-1);
	main_decoder_table[732]=aarch64_mask_entry(0x0, branchMap(),356);
	main_decoder_table[733]=aarch64_mask_entry(0x0, branchMap(),416);
	main_decoder_table[688]=aarch64_mask_entry(0x0, branchMap(),339);
	main_decoder_table[689]=aarch64_mask_entry(0x0, branchMap(),349);
	main_decoder_table[690]=aarch64_mask_entry(0x0, branchMap(),340);
	main_decoder_table[691]=aarch64_mask_entry(0x40400000, map_list_of(0,734)(2,735),-1);
	main_decoder_table[734]=aarch64_mask_entry(0x0, branchMap(),343);
	main_decoder_table[735]=aarch64_mask_entry(0x0, branchMap(),415);
	main_decoder_table[478]=aarch64_mask_entry(0x80800000, map_list_of(0,736)(1,737)(2,738)(3,739),-1);
	main_decoder_table[736]=aarch64_mask_entry(0x40400000, map_list_of(0,740)(1,741)(2,742)(3,743),-1);
	main_decoder_table[740]=aarch64_mask_entry(0x0, branchMap(),599);
	main_decoder_table[741]=aarch64_mask_entry(0x0, branchMap(),325);
	main_decoder_table[742]=aarch64_mask_entry(0x0, branchMap(),603);
	main_decoder_table[743]=aarch64_mask_entry(0x0, branchMap(),329);
	main_decoder_table[737]=aarch64_mask_entry(0x40000000, map_list_of(0,744)(1,745),-1);
	main_decoder_table[744]=aarch64_mask_entry(0x0, branchMap(),333);
	main_decoder_table[745]=aarch64_mask_entry(0x0, branchMap(),337);
	main_decoder_table[738]=aarch64_mask_entry(0x400000, map_list_of(0,746)(1,747),-1);
	main_decoder_table[746]=aarch64_mask_entry(0x0, branchMap(),594);
	main_decoder_table[747]=aarch64_mask_entry(0x0, branchMap(),318);
	main_decoder_table[739]=aarch64_mask_entry(0x40400000, map_list_of(0,748)(2,749),-1);
	main_decoder_table[748]=aarch64_mask_entry(0x0, branchMap(),341);
	main_decoder_table[749]=aarch64_mask_entry(0x0, branchMap(),413);
	main_decoder_table[479]=aarch64_mask_entry(0x40e00c00, map_list_of(0,750)(8,751)(10,752)(32,753)(40,754)(42,755),-1);
	main_decoder_table[750]=aarch64_mask_entry(0x0, branchMap(),4);
	main_decoder_table[751]=aarch64_mask_entry(0x0, branchMap(),51);
	main_decoder_table[752]=aarch64_mask_entry(0x0, branchMap(),50);
	main_decoder_table[753]=aarch64_mask_entry(0xf000, map_list_of(0,756),-1);
	main_decoder_table[756]=aarch64_mask_entry(0x0, branchMap(),442);
	main_decoder_table[754]=aarch64_mask_entry(0x0, branchMap(),53);
	main_decoder_table[755]=aarch64_mask_entry(0x0, branchMap(),52);
	main_decoder_table[480]=aarch64_mask_entry(0x600c00, map_list_of(0,757)(1,758)(3,759)(6,760)(8,761)(9,762)(11,763)(14,764),-1);
	main_decoder_table[757]=aarch64_mask_entry(0x0, branchMap(),608);
	main_decoder_table[758]=aarch64_mask_entry(0x0, branchMap(),589);
	main_decoder_table[759]=aarch64_mask_entry(0x0, branchMap(),590);
	main_decoder_table[760]=aarch64_mask_entry(0x0, branchMap(),595);
	main_decoder_table[761]=aarch64_mask_entry(0x0, branchMap(),350);
	main_decoder_table[762]=aarch64_mask_entry(0x0, branchMap(),313);
	main_decoder_table[763]=aarch64_mask_entry(0x0, branchMap(),314);
	main_decoder_table[764]=aarch64_mask_entry(0x0, branchMap(),321);
	main_decoder_table[481]=aarch64_mask_entry(0x400000, map_list_of(0,765)(1,766),-1);
	main_decoder_table[765]=aarch64_mask_entry(0x0, branchMap(),591);
	main_decoder_table[766]=aarch64_mask_entry(0x0, branchMap(),315);
	main_decoder_table[482]=aarch64_mask_entry(0xc020fc00, map_list_of(195,767)(202,768)(203,769)(205,770)(206,771)(207,772)(209,773)(210,774)(211,775)(213,776)(215,777)(218,778)(222,779)(225,780)(226,781)(227,782)(230,783)(234,784)(237,785)(238,786)(242,787)(245,788)(246,789)(249,790)(251,791)(254,792),-1);
	main_decoder_table[767]=aarch64_mask_entry(0x0, branchMap(),682);
	main_decoder_table[768]=aarch64_mask_entry(0x0, branchMap(),538);
	main_decoder_table[769]=aarch64_mask_entry(0x0, branchMap(),694);
	main_decoder_table[770]=aarch64_mask_entry(0x0, branchMap(),73);
	main_decoder_table[771]=aarch64_mask_entry(0x0, branchMap(),712);
	main_decoder_table[772]=aarch64_mask_entry(0x0, branchMap(),75);
	main_decoder_table[773]=aarch64_mask_entry(0x0, branchMap(),707);
	main_decoder_table[774]=aarch64_mask_entry(0x0, branchMap(),696);
	main_decoder_table[775]=aarch64_mask_entry(0x0, branchMap(),690);
	main_decoder_table[776]=aarch64_mask_entry(0x0, branchMap(),700);
	main_decoder_table[777]=aarch64_mask_entry(0x0, branchMap(),684);
	main_decoder_table[778]=aarch64_mask_entry(0x0, branchMap(),176);
	main_decoder_table[779]=aarch64_mask_entry(0x0, branchMap(),512);
	main_decoder_table[780]=aarch64_mask_entry(0x0, branchMap(),619);
	main_decoder_table[781]=aarch64_mask_entry(0x0, branchMap(),67);
	main_decoder_table[782]=aarch64_mask_entry(0x0, branchMap(),61);
	main_decoder_table[783]=aarch64_mask_entry(0x0, branchMap(),77);
	main_decoder_table[784]=aarch64_mask_entry(0x9f0000, map_list_of(1,793)(33,794),-1);
	main_decoder_table[793]=aarch64_mask_entry(0x0, branchMap(),167);
	main_decoder_table[794]=aarch64_mask_entry(0x0, branchMap(),173);
	main_decoder_table[785]=aarch64_mask_entry(0x0, branchMap(),516);
	main_decoder_table[786]=aarch64_mask_entry(0x1f0000, map_list_of(0,795)(1,796),-1);
	main_decoder_table[795]=aarch64_mask_entry(0x0, branchMap(),397);
	main_decoder_table[796]=aarch64_mask_entry(0x800000, map_list_of(0,797)(1,798),-1);
	main_decoder_table[797]=aarch64_mask_entry(0x0, branchMap(),160);
	main_decoder_table[798]=aarch64_mask_entry(0x0, branchMap(),186);
	main_decoder_table[787]=aarch64_mask_entry(0x9f0000, map_list_of(1,799)(16,800)(32,801)(48,802),-1);
	main_decoder_table[799]=aarch64_mask_entry(0x0, branchMap(),153);
	main_decoder_table[800]=aarch64_mask_entry(0x0, branchMap(),197);
	main_decoder_table[801]=aarch64_mask_entry(0x0, branchMap(),136);
	main_decoder_table[802]=aarch64_mask_entry(0x0, branchMap(),207);
	main_decoder_table[788]=aarch64_mask_entry(0x0, branchMap(),116);
	main_decoder_table[789]=aarch64_mask_entry(0x9f0000, map_list_of(1,803)(16,804)(32,805)(33,806),-1);
	main_decoder_table[803]=aarch64_mask_entry(0x0, branchMap(),657);
	main_decoder_table[804]=aarch64_mask_entry(0x0, branchMap(),126);
	main_decoder_table[805]=aarch64_mask_entry(0x0, branchMap(),142);
	main_decoder_table[806]=aarch64_mask_entry(0x0, branchMap(),256);
	main_decoder_table[790]=aarch64_mask_entry(0x800000, map_list_of(0,807)(1,808),-1);
	main_decoder_table[807]=aarch64_mask_entry(0x0, branchMap(),134);
	main_decoder_table[808]=aarch64_mask_entry(0x0, branchMap(),138);
	main_decoder_table[791]=aarch64_mask_entry(0x800000, map_list_of(0,809)(1,810),-1);
	main_decoder_table[809]=aarch64_mask_entry(0x0, branchMap(),120);
	main_decoder_table[810]=aarch64_mask_entry(0x0, branchMap(),122);
	main_decoder_table[792]=aarch64_mask_entry(0x9f0000, map_list_of(16,811)(48,812),-1);
	main_decoder_table[811]=aarch64_mask_entry(0x0, branchMap(),200);
	main_decoder_table[812]=aarch64_mask_entry(0x0, branchMap(),210);
	main_decoder_table[483]=aarch64_mask_entry(0xc080f400, map_list_of(65,813)(67,814)(69,815)(71,816)(73,817)(75,818)(77,819)(79,820)(81,821)(83,822)(93,823)(95,824)(114,825),-1);
	main_decoder_table[813]=aarch64_mask_entry(0x0, branchMap(),710);
	main_decoder_table[814]=aarch64_mask_entry(0x0, branchMap(),714);
	main_decoder_table[815]=aarch64_mask_entry(0x0, branchMap(),702);
	main_decoder_table[816]=aarch64_mask_entry(0x0, branchMap(),705);
	main_decoder_table[817]=aarch64_mask_entry(0x0, branchMap(),541);
	main_decoder_table[818]=aarch64_mask_entry(0x0, branchMap(),471);
	main_decoder_table[819]=aarch64_mask_entry(0x0, branchMap(),528);
	main_decoder_table[820]=aarch64_mask_entry(0x0, branchMap(),688);
	main_decoder_table[821]=aarch64_mask_entry(0x800, map_list_of(0,826)(1,827),-1);
	main_decoder_table[826]=aarch64_mask_entry(0x0, branchMap(),532);
	main_decoder_table[827]=aarch64_mask_entry(0x0, branchMap(),522);
	main_decoder_table[822]=aarch64_mask_entry(0x800, map_list_of(0,828)(1,829),-1);
	main_decoder_table[828]=aarch64_mask_entry(0x0, branchMap(),692);
	main_decoder_table[829]=aarch64_mask_entry(0x0, branchMap(),686);
	main_decoder_table[823]=aarch64_mask_entry(0x0, branchMap(),655);
	main_decoder_table[824]=aarch64_mask_entry(0x0, branchMap(),184);
	main_decoder_table[825]=aarch64_mask_entry(0x0, branchMap(),228);
}

void aarch64_insn_entry::buildInsnTable()
{
		main_insn_table.push_back(aarch64_insn_entry(aarch64_op_INVALID, 	"INVALID",	operandSpec() ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_abs_advsimd, 	"abs",	list_of( fn(setSIMDMode) )( fn(OPRsize<23 COMMA 22>) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_abs_advsimd, 	"abs",	list_of( fn(setSIMDMode) )( fn(OPRQ) )( fn(OPRsize<23 COMMA 22>) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_adc, 	"adc",	list_of( fn(OPRsf) )( fn(OPRRm) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_adcs, 	"adcs",	list_of( fn(setFlags) )( fn(OPRsf) )( fn(OPRRm) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_add_addsub_ext, 	"add",	list_of( fn(OPRsf) )( fn(OPRRm) )( fn(OPRoption<15 COMMA 13>) )( fn(OPRimm<12 COMMA 10>) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_add_addsub_imm, 	"add",	list_of( fn(OPRsf) )( fn(OPRshift) )( fn(OPRimm<21 COMMA 10>) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_add_addsub_shift, 	"add",	list_of( fn(OPRsf) )( fn(OPRshift) )( fn(OPRRm) )( fn(OPRimm<15 COMMA 10>) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_add_advsimd, 	"add",	list_of( fn(setSIMDMode) )( fn(OPRsize<23 COMMA 22>) )( fn(OPRRm) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_add_advsimd, 	"add",	list_of( fn(setSIMDMode) )( fn(OPRQ) )( fn(OPRsize<23 COMMA 22>) )( fn(OPRRm) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_addhn_advsimd, 	"addhn",	list_of( fn(setSIMDMode) )( fn(OPRQ) )( fn(OPRsize<23 COMMA 22>) )( fn(OPRRm) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_addp_advsimd_pair, 	"addp",	list_of( fn(setSIMDMode) )( fn(OPRsize<23 COMMA 22>) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_addp_advsimd_vec, 	"addp",	list_of( fn(setSIMDMode) )( fn(OPRQ) )( fn(OPRsize<23 COMMA 22>) )( fn(OPRRm) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_adds_addsub_ext, 	"adds",	list_of( fn(setFlags) )( fn(OPRsf) )( fn(OPRRm) )( fn(OPRoption<15 COMMA 13>) )( fn(OPRimm<12 COMMA 10>) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_adds_addsub_imm, 	"adds",	list_of( fn(setFlags) )( fn(OPRsf) )( fn(OPRshift) )( fn(OPRimm<21 COMMA 10>) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_adds_addsub_shift, 	"adds",	list_of( fn(setFlags) )( fn(OPRsf) )( fn(OPRshift) )( fn(OPRRm) )( fn(OPRimm<15 COMMA 10>) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_addv_advsimd, 	"addv",	list_of( fn(setSIMDMode) )( fn(OPRQ) )( fn(OPRsize<23 COMMA 22>) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_adr, 	"adr",	list_of( (operandFactory) fn(OPRimm<30 COMMA 29>) )( fn(OPRimm<23 COMMA 5>) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_adrp, 	"adrp",	list_of( (operandFactory) fn(OPRimm<30 COMMA 29>) )( fn(OPRimm<23 COMMA 5>) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_aesd_advsimd, 	"aesd",	list_of( fn(setSIMDMode) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_aese_advsimd, 	"aese",	list_of( fn(setSIMDMode) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_aesimc_advsimd, 	"aesimc",	list_of( fn(setSIMDMode) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_aesmc_advsimd, 	"aesmc",	list_of( fn(setSIMDMode) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_and_advsimd, 	"and",	list_of( fn(setSIMDMode) )( fn(OPRQ) )( fn(OPRRm) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_and_log_imm, 	"and",	list_of( fn(OPRsf) )( fn(OPRN<22 COMMA 22>) )( fn(OPRimm<21 COMMA 16>) )( fn(OPRimm<15 COMMA 10>) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_and_log_shift, 	"and",	list_of( fn(OPRsf) )( fn(OPRshift) )( fn(OPRRm) )( fn(OPRimm<15 COMMA 10>) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_ands_log_imm, 	"ands",	list_of( fn(setFlags) )( fn(OPRsf) )( fn(OPRN<22 COMMA 22>) )( fn(OPRimm<21 COMMA 16>) )( fn(OPRimm<15 COMMA 10>) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_ands_log_shift, 	"ands",	list_of( fn(setFlags) )( fn(OPRsf) )( fn(OPRshift) )( fn(OPRRm) )( fn(OPRimm<15 COMMA 10>) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_asr_asrv, 	"asr",	list_of( fn(OPRsf) )( fn(OPRRm) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_asr_sbfm, 	"asr",	list_of( fn(OPRsf) )( fn(OPRN<22 COMMA 22>) )( fn(OPRimm<21 COMMA 16>) )( fn(OPRimm<15 COMMA 10>) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_asrv, 	"asrv",	list_of( fn(OPRsf) )( fn(OPRRm) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_at_sys, 	"at",	list_of( fn(OPRop1) )( fn(OPRop2) )( fn(OPRRt) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_b_cond, 	"b",	list_of( (operandFactory) fn(OPRimm<23 COMMA 5>) )( fn(OPRcond<3 COMMA 0>) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_b_uncond, 	"b",	list_of( (operandFactory) fn(OPRimm<25 COMMA 0>) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_bfi_bfm, 	"bfi",	list_of( fn(OPRsf) )( fn(OPRN<22 COMMA 22>) )( fn(OPRimm<21 COMMA 16>) )( fn(OPRimm<15 COMMA 10>) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_bfm, 	"bfm",	list_of( fn(OPRsf) )( fn(OPRN<22 COMMA 22>) )( fn(OPRimm<21 COMMA 16>) )( fn(OPRimm<15 COMMA 10>) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_bfxil_bfm, 	"bfxil",	list_of( fn(OPRsf) )( fn(OPRN<22 COMMA 22>) )( fn(OPRimm<21 COMMA 16>) )( fn(OPRimm<15 COMMA 10>) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_bic_advsimd_imm, 	"bic",	list_of( fn(setSIMDMode) )( fn(OPRQ) )( fn(OPRa) )( fn(OPRb) )( fn(OPRc) )( fn(OPRcmode) )( fn(OPRcmode) )( fn(OPRcmode) )( fn(OPRd) )( fn(OPRe) )( fn(OPRf) )( fn(OPRg) )( fn(OPRh) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_bic_advsimd_reg, 	"bic",	list_of( fn(setSIMDMode) )( fn(OPRQ) )( fn(OPRRm) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_bic_log_shift, 	"bic",	list_of( fn(OPRsf) )( fn(OPRshift) )( fn(OPRRm) )( fn(OPRimm<15 COMMA 10>) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_bics, 	"bics",	list_of( fn(setFlags) )( fn(OPRsf) )( fn(OPRshift) )( fn(OPRRm) )( fn(OPRimm<15 COMMA 10>) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_bif_advsimd, 	"bif",	list_of( fn(setSIMDMode) )( fn(OPRQ) )( fn(OPRRm) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_bit_advsimd, 	"bit",	list_of( fn(setSIMDMode) )( fn(OPRQ) )( fn(OPRRm) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_bl, 	"bl",	list_of( (operandFactory) fn(OPRimm<25 COMMA 0>) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_blr, 	"blr",	list_of( fn(OPRRn) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_br, 	"br",	list_of( fn(OPRRn) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_brk, 	"brk",	list_of( (operandFactory) fn(OPRimm<20 COMMA 5>) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_bsl_advsimd, 	"bsl",	list_of( fn(setSIMDMode) )( fn(OPRQ) )( fn(OPRRm) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_cbnz, 	"cbnz",	list_of( fn(OPRsf) )( fn(OPRimm<23 COMMA 5>) )( fn(OPRRt) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_cbz, 	"cbz",	list_of( fn(OPRsf) )( fn(OPRimm<23 COMMA 5>) )( fn(OPRRt) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_ccmn_imm, 	"ccmn",	list_of( fn(OPRsf) )( fn(OPRimm<20 COMMA 16>) )( fn(OPRcond<15 COMMA 12>) )( fn(OPRRn) )( fn(OPRnzcv) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_ccmn_reg, 	"ccmn",	list_of( fn(OPRsf) )( fn(OPRRm) )( fn(OPRcond<15 COMMA 12>) )( fn(OPRRn) )( fn(OPRnzcv) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_ccmp_imm, 	"ccmp",	list_of( fn(OPRsf) )( fn(OPRimm<20 COMMA 16>) )( fn(OPRcond<15 COMMA 12>) )( fn(OPRRn) )( fn(OPRnzcv) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_ccmp_reg, 	"ccmp",	list_of( fn(OPRsf) )( fn(OPRRm) )( fn(OPRcond<15 COMMA 12>) )( fn(OPRRn) )( fn(OPRnzcv) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_cinc_csinc, 	"cinc",	list_of( fn(OPRsf) )( fn(OPRRm) )( fn(OPRcond<15 COMMA 12>) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_cinv_csinv, 	"cinv",	list_of( fn(OPRsf) )( fn(OPRRm) )( fn(OPRcond<15 COMMA 12>) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_clrex, 	"clrex",	list_of( fn(OPRCRm) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_cls_advsimd, 	"cls",	list_of( fn(setSIMDMode) )( fn(OPRQ) )( fn(OPRsize<23 COMMA 22>) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_cls_int, 	"cls",	list_of( fn(OPRsf) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_clz_advsimd, 	"clz",	list_of( fn(setSIMDMode) )( fn(OPRQ) )( fn(OPRsize<23 COMMA 22>) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_clz_int, 	"clz",	list_of( fn(OPRsf) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_cmeq_advsimd_reg, 	"cmeq",	list_of( fn(setSIMDMode) )( fn(OPRsize<23 COMMA 22>) )( fn(OPRRm) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_cmeq_advsimd_reg, 	"cmeq",	list_of( fn(setSIMDMode) )( fn(OPRQ) )( fn(OPRsize<23 COMMA 22>) )( fn(OPRRm) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_cmeq_advsimd_zero, 	"cmeq",	list_of( fn(setSIMDMode) )( fn(OPRsize<23 COMMA 22>) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_cmeq_advsimd_zero, 	"cmeq",	list_of( fn(setSIMDMode) )( fn(OPRQ) )( fn(OPRsize<23 COMMA 22>) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_cmge_advsimd_reg, 	"cmge",	list_of( fn(setSIMDMode) )( fn(OPRsize<23 COMMA 22>) )( fn(OPRRm) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_cmge_advsimd_reg, 	"cmge",	list_of( fn(setSIMDMode) )( fn(OPRQ) )( fn(OPRsize<23 COMMA 22>) )( fn(OPRRm) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_cmge_advsimd_zero, 	"cmge",	list_of( fn(setSIMDMode) )( fn(OPRsize<23 COMMA 22>) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_cmge_advsimd_zero, 	"cmge",	list_of( fn(setSIMDMode) )( fn(OPRQ) )( fn(OPRsize<23 COMMA 22>) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_cmgt_advsimd_reg, 	"cmgt",	list_of( fn(setSIMDMode) )( fn(OPRsize<23 COMMA 22>) )( fn(OPRRm) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_cmgt_advsimd_reg, 	"cmgt",	list_of( fn(setSIMDMode) )( fn(OPRQ) )( fn(OPRsize<23 COMMA 22>) )( fn(OPRRm) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_cmgt_advsimd_zero, 	"cmgt",	list_of( fn(setSIMDMode) )( fn(OPRsize<23 COMMA 22>) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_cmgt_advsimd_zero, 	"cmgt",	list_of( fn(setSIMDMode) )( fn(OPRQ) )( fn(OPRsize<23 COMMA 22>) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_cmhi_advsimd, 	"cmhi",	list_of( fn(setSIMDMode) )( fn(OPRsize<23 COMMA 22>) )( fn(OPRRm) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_cmhi_advsimd, 	"cmhi",	list_of( fn(setSIMDMode) )( fn(OPRQ) )( fn(OPRsize<23 COMMA 22>) )( fn(OPRRm) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_cmhs_advsimd, 	"cmhs",	list_of( fn(setSIMDMode) )( fn(OPRsize<23 COMMA 22>) )( fn(OPRRm) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_cmhs_advsimd, 	"cmhs",	list_of( fn(setSIMDMode) )( fn(OPRQ) )( fn(OPRsize<23 COMMA 22>) )( fn(OPRRm) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_cmle_advsimd, 	"cmle",	list_of( fn(setSIMDMode) )( fn(OPRsize<23 COMMA 22>) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_cmle_advsimd, 	"cmle",	list_of( fn(setSIMDMode) )( fn(OPRQ) )( fn(OPRsize<23 COMMA 22>) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_cmlt_advsimd, 	"cmlt",	list_of( fn(setSIMDMode) )( fn(OPRsize<23 COMMA 22>) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_cmlt_advsimd, 	"cmlt",	list_of( fn(setSIMDMode) )( fn(OPRQ) )( fn(OPRsize<23 COMMA 22>) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_cmn_adds_addsub_ext, 	"cmn",	list_of( fn(setFlags) )( fn(OPRsf) )( fn(OPRRm) )( fn(OPRoption<15 COMMA 13>) )( fn(OPRimm<12 COMMA 10>) )( fn(OPRRn) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_cmn_adds_addsub_imm, 	"cmn",	list_of( fn(setFlags) )( fn(OPRsf) )( fn(OPRshift) )( fn(OPRimm<21 COMMA 10>) )( fn(OPRRn) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_cmn_adds_addsub_shift, 	"cmn",	list_of( fn(setFlags) )( fn(OPRsf) )( fn(OPRshift) )( fn(OPRRm) )( fn(OPRimm<15 COMMA 10>) )( fn(OPRRn) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_cmp_subs_addsub_ext, 	"cmp",	list_of( fn(setFlags) )( fn(OPRsf) )( fn(OPRRm) )( fn(OPRoption<15 COMMA 13>) )( fn(OPRimm<12 COMMA 10>) )( fn(OPRRn) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_cmp_subs_addsub_imm, 	"cmp",	list_of( fn(setFlags) )( fn(OPRsf) )( fn(OPRshift) )( fn(OPRimm<21 COMMA 10>) )( fn(OPRRn) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_cmp_subs_addsub_shift, 	"cmp",	list_of( fn(setFlags) )( fn(OPRsf) )( fn(OPRshift) )( fn(OPRRm) )( fn(OPRimm<15 COMMA 10>) )( fn(OPRRn) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_cmtst_advsimd, 	"cmtst",	list_of( fn(setSIMDMode) )( fn(OPRsize<23 COMMA 22>) )( fn(OPRRm) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_cmtst_advsimd, 	"cmtst",	list_of( fn(setSIMDMode) )( fn(OPRQ) )( fn(OPRsize<23 COMMA 22>) )( fn(OPRRm) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_cneg_csneg, 	"cneg",	list_of( fn(OPRsf) )( fn(OPRRm) )( fn(OPRcond<15 COMMA 12>) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_cnt_advsimd, 	"cnt",	list_of( fn(setSIMDMode) )( fn(OPRQ) )( fn(OPRsize<23 COMMA 22>) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_crc32, 	"crc32",	list_of( fn(OPRsf) )( fn(OPRRm) )( fn(OPRsz<11 COMMA 10>) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_crc32c, 	"crc32c",	list_of( fn(OPRsf) )( fn(OPRRm) )( fn(OPRsz<11 COMMA 10>) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_csel, 	"csel",	list_of( fn(OPRsf) )( fn(OPRRm) )( fn(OPRcond<15 COMMA 12>) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_cset_csinc, 	"cset",	list_of( fn(OPRsf) )( fn(OPRcond<15 COMMA 12>) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_csetm_csinv, 	"csetm",	list_of( fn(OPRsf) )( fn(OPRcond<15 COMMA 12>) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_csinc, 	"csinc",	list_of( fn(OPRsf) )( fn(OPRRm) )( fn(OPRcond<15 COMMA 12>) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_csinv, 	"csinv",	list_of( fn(OPRsf) )( fn(OPRRm) )( fn(OPRcond<15 COMMA 12>) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_csneg, 	"csneg",	list_of( fn(OPRsf) )( fn(OPRRm) )( fn(OPRcond<15 COMMA 12>) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_dc_sys, 	"dc",	list_of( fn(OPRop1) )( fn(OPRCRm) )( fn(OPRop2) )( fn(OPRRt) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_dcps1, 	"dcps1",	list_of( (operandFactory) fn(OPRimm<20 COMMA 5>) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_dcps2, 	"dcps2",	list_of( (operandFactory) fn(OPRimm<20 COMMA 5>) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_dcps3, 	"dcps3",	list_of( (operandFactory) fn(OPRimm<20 COMMA 5>) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_dmb, 	"dmb",	list_of( fn(OPRCRm) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_drps, 	"drps",	operandSpec() ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_dsb, 	"dsb",	list_of( fn(OPRCRm) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_dup_advsimd_elt, 	"dup",	list_of( fn(setSIMDMode) )( fn(OPRimm<20 COMMA 16>) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_dup_advsimd_elt, 	"dup",	list_of( fn(setSIMDMode) )( fn(OPRQ) )( fn(OPRimm<20 COMMA 16>) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_dup_advsimd_gen, 	"dup",	list_of( fn(setSIMDMode) )( fn(OPRQ) )( fn(OPRimm<20 COMMA 16>) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_eon, 	"eon",	list_of( fn(OPRsf) )( fn(OPRshift) )( fn(OPRRm) )( fn(OPRimm<15 COMMA 10>) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_eor_advsimd, 	"eor",	list_of( fn(setSIMDMode) )( fn(OPRQ) )( fn(OPRRm) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_eor_log_imm, 	"eor",	list_of( fn(OPRsf) )( fn(OPRN<22 COMMA 22>) )( fn(OPRimm<21 COMMA 16>) )( fn(OPRimm<15 COMMA 10>) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_eor_log_shift, 	"eor",	list_of( fn(OPRsf) )( fn(OPRshift) )( fn(OPRRm) )( fn(OPRimm<15 COMMA 10>) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_eret, 	"eret",	operandSpec() ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_ext_advsimd, 	"ext",	list_of( fn(setSIMDMode) )( fn(OPRQ) )( fn(OPRRm) )( fn(OPRimm<14 COMMA 11>) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_extr, 	"extr",	list_of( fn(OPRsf) )( fn(OPRN<22 COMMA 22>) )( fn(OPRRm) )( fn(OPRimm<15 COMMA 10>) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_fabd_advsimd, 	"fabd",	list_of( fn(setSIMDMode) )( fn(OPRsz<22 COMMA 22>) )( fn(OPRRm) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_fabd_advsimd, 	"fabd",	list_of( fn(setSIMDMode) )( fn(OPRQ) )( fn(OPRsz<22 COMMA 22>) )( fn(OPRRm) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_fabs_advsimd, 	"fabs",	list_of( fn(setSIMDMode) )( fn(OPRQ) )( fn(OPRsz<22 COMMA 22>) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_fabs_float, 	"fabs",	list_of( fn(setFPMode) )( fn(OPRtype<23 COMMA 22>) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_facge_advsimd, 	"facge",	list_of( fn(setSIMDMode) )( fn(OPRsz<22 COMMA 22>) )( fn(OPRRm) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_facge_advsimd, 	"facge",	list_of( fn(setSIMDMode) )( fn(OPRQ) )( fn(OPRsz<22 COMMA 22>) )( fn(OPRRm) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_facgt_advsimd, 	"facgt",	list_of( fn(setSIMDMode) )( fn(OPRsz<22 COMMA 22>) )( fn(OPRRm) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_facgt_advsimd, 	"facgt",	list_of( fn(setSIMDMode) )( fn(OPRQ) )( fn(OPRsz<22 COMMA 22>) )( fn(OPRRm) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_fadd_advsimd, 	"fadd",	list_of( fn(setSIMDMode) )( fn(OPRQ) )( fn(OPRsz<22 COMMA 22>) )( fn(OPRRm) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_fadd_float, 	"fadd",	list_of( fn(setFPMode) )( fn(OPRtype<23 COMMA 22>) )( fn(OPRRm) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_faddp_advsimd_pair, 	"faddp",	list_of( fn(setSIMDMode) )( fn(OPRsz<22 COMMA 22>) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_faddp_advsimd_vec, 	"faddp",	list_of( fn(setSIMDMode) )( fn(OPRQ) )( fn(OPRsz<22 COMMA 22>) )( fn(OPRRm) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_fccmp_float, 	"fccmp",	list_of( fn(setFPMode) )( fn(setFlags) )( fn(OPRtype<23 COMMA 22>) )( fn(OPRRm) )( fn(OPRcond<15 COMMA 12>) )( fn(OPRRn) )( fn(OPRnzcv) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_fccmpe_float, 	"fccmpe",	list_of( fn(setFPMode) )( fn(setFlags) )( fn(OPRtype<23 COMMA 22>) )( fn(OPRRm) )( fn(OPRcond<15 COMMA 12>) )( fn(OPRRn) )( fn(OPRnzcv) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_fcmeq_advsimd_reg, 	"fcmeq",	list_of( fn(setSIMDMode) )( fn(OPRsz<22 COMMA 22>) )( fn(OPRRm) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_fcmeq_advsimd_reg, 	"fcmeq",	list_of( fn(setSIMDMode) )( fn(OPRQ) )( fn(OPRsz<22 COMMA 22>) )( fn(OPRRm) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_fcmeq_advsimd_zero, 	"fcmeq",	list_of( fn(setSIMDMode) )( fn(OPRsz<22 COMMA 22>) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_fcmeq_advsimd_zero, 	"fcmeq",	list_of( fn(setSIMDMode) )( fn(OPRQ) )( fn(OPRsz<22 COMMA 22>) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_fcmge_advsimd_reg, 	"fcmge",	list_of( fn(setSIMDMode) )( fn(OPRsz<22 COMMA 22>) )( fn(OPRRm) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_fcmge_advsimd_reg, 	"fcmge",	list_of( fn(setSIMDMode) )( fn(OPRQ) )( fn(OPRsz<22 COMMA 22>) )( fn(OPRRm) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_fcmge_advsimd_zero, 	"fcmge",	list_of( fn(setSIMDMode) )( fn(OPRsz<22 COMMA 22>) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_fcmge_advsimd_zero, 	"fcmge",	list_of( fn(setSIMDMode) )( fn(OPRQ) )( fn(OPRsz<22 COMMA 22>) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_fcmgt_advsimd_reg, 	"fcmgt",	list_of( fn(setSIMDMode) )( fn(OPRsz<22 COMMA 22>) )( fn(OPRRm) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_fcmgt_advsimd_reg, 	"fcmgt",	list_of( fn(setSIMDMode) )( fn(OPRQ) )( fn(OPRsz<22 COMMA 22>) )( fn(OPRRm) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_fcmgt_advsimd_zero, 	"fcmgt",	list_of( fn(setSIMDMode) )( fn(OPRsz<22 COMMA 22>) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_fcmgt_advsimd_zero, 	"fcmgt",	list_of( fn(setSIMDMode) )( fn(OPRQ) )( fn(OPRsz<22 COMMA 22>) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_fcmle_advsimd, 	"fcmle",	list_of( fn(setSIMDMode) )( fn(OPRsz<22 COMMA 22>) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_fcmle_advsimd, 	"fcmle",	list_of( fn(setSIMDMode) )( fn(OPRQ) )( fn(OPRsz<22 COMMA 22>) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_fcmlt_advsimd, 	"fcmlt",	list_of( fn(setSIMDMode) )( fn(OPRsz<22 COMMA 22>) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_fcmlt_advsimd, 	"fcmlt",	list_of( fn(setSIMDMode) )( fn(OPRQ) )( fn(OPRsz<22 COMMA 22>) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_fcmp_float, 	"fcmp",	list_of( fn(setFPMode) )( fn(setFlags) )( fn(OPRtype<23 COMMA 22>) )( fn(OPRRm) )( fn(OPRRn) )( fn(OPRopc) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_fcmpe_float, 	"fcmpe",	list_of( fn(setFPMode) )( fn(setFlags) )( fn(OPRtype<23 COMMA 22>) )( fn(OPRRm) )( fn(OPRRn) )( fn(OPRopc) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_fcsel_float, 	"fcsel",	list_of( fn(setFPMode) )( fn(OPRtype<23 COMMA 22>) )( fn(OPRRm) )( fn(OPRcond<15 COMMA 12>) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_fcvt_float, 	"fcvt",	list_of( fn(setFPMode) )( fn(OPRtype<23 COMMA 22>) )( fn(OPRopc) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_fcvtas_advsimd, 	"fcvtas",	list_of( fn(setSIMDMode) )( fn(OPRsz<22 COMMA 22>) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_fcvtas_advsimd, 	"fcvtas",	list_of( fn(setSIMDMode) )( fn(OPRQ) )( fn(OPRsz<22 COMMA 22>) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_fcvtas_float, 	"fcvtas",	list_of( fn(setFPMode) )( fn(OPRsf) )( fn(OPRtype<23 COMMA 22>) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_fcvtau_advsimd, 	"fcvtau",	list_of( fn(setSIMDMode) )( fn(OPRsz<22 COMMA 22>) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_fcvtau_advsimd, 	"fcvtau",	list_of( fn(setSIMDMode) )( fn(OPRQ) )( fn(OPRsz<22 COMMA 22>) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_fcvtau_float, 	"fcvtau",	list_of( fn(setFPMode) )( fn(OPRsf) )( fn(OPRtype<23 COMMA 22>) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_fcvtl_advsimd, 	"fcvtl",	list_of( fn(setSIMDMode) )( fn(OPRQ) )( fn(OPRsz<22 COMMA 22>) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_fcvtms_advsimd, 	"fcvtms",	list_of( fn(setSIMDMode) )( fn(OPRsz<22 COMMA 22>) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_fcvtms_advsimd, 	"fcvtms",	list_of( fn(setSIMDMode) )( fn(OPRQ) )( fn(OPRsz<22 COMMA 22>) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_fcvtms_float, 	"fcvtms",	list_of( fn(setFPMode) )( fn(OPRsf) )( fn(OPRtype<23 COMMA 22>) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_fcvtmu_advsimd, 	"fcvtmu",	list_of( fn(setSIMDMode) )( fn(OPRsz<22 COMMA 22>) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_fcvtmu_advsimd, 	"fcvtmu",	list_of( fn(setSIMDMode) )( fn(OPRQ) )( fn(OPRsz<22 COMMA 22>) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_fcvtmu_float, 	"fcvtmu",	list_of( fn(setFPMode) )( fn(OPRsf) )( fn(OPRtype<23 COMMA 22>) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_fcvtn_advsimd, 	"fcvtn",	list_of( fn(setSIMDMode) )( fn(OPRQ) )( fn(OPRsz<22 COMMA 22>) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_fcvtns_advsimd, 	"fcvtns",	list_of( fn(setSIMDMode) )( fn(OPRsz<22 COMMA 22>) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_fcvtns_advsimd, 	"fcvtns",	list_of( fn(setSIMDMode) )( fn(OPRQ) )( fn(OPRsz<22 COMMA 22>) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_fcvtns_float, 	"fcvtns",	list_of( fn(setFPMode) )( fn(OPRsf) )( fn(OPRtype<23 COMMA 22>) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_fcvtnu_advsimd, 	"fcvtnu",	list_of( fn(setSIMDMode) )( fn(OPRsz<22 COMMA 22>) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_fcvtnu_advsimd, 	"fcvtnu",	list_of( fn(setSIMDMode) )( fn(OPRQ) )( fn(OPRsz<22 COMMA 22>) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_fcvtnu_float, 	"fcvtnu",	list_of( fn(setFPMode) )( fn(OPRsf) )( fn(OPRtype<23 COMMA 22>) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_fcvtps_advsimd, 	"fcvtps",	list_of( fn(setSIMDMode) )( fn(OPRsz<22 COMMA 22>) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_fcvtps_advsimd, 	"fcvtps",	list_of( fn(setSIMDMode) )( fn(OPRQ) )( fn(OPRsz<22 COMMA 22>) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_fcvtps_float, 	"fcvtps",	list_of( fn(setFPMode) )( fn(OPRsf) )( fn(OPRtype<23 COMMA 22>) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_fcvtpu_advsimd, 	"fcvtpu",	list_of( fn(setSIMDMode) )( fn(OPRsz<22 COMMA 22>) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_fcvtpu_advsimd, 	"fcvtpu",	list_of( fn(setSIMDMode) )( fn(OPRQ) )( fn(OPRsz<22 COMMA 22>) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_fcvtpu_float, 	"fcvtpu",	list_of( fn(setFPMode) )( fn(OPRsf) )( fn(OPRtype<23 COMMA 22>) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_fcvtxn_advsimd, 	"fcvtxn",	list_of( fn(setSIMDMode) )( fn(OPRsz<22 COMMA 22>) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_fcvtxn_advsimd, 	"fcvtxn",	list_of( fn(setSIMDMode) )( fn(OPRQ) )( fn(OPRsz<22 COMMA 22>) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_fcvtzs_advsimd_fix, 	"fcvtzs",	list_of( fn(setSIMDMode) )( fn(OPRimm<22 COMMA 19>) )( fn(OPRimm<18 COMMA 16>) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_fcvtzs_advsimd_fix, 	"fcvtzs",	list_of( fn(setSIMDMode) )( fn(OPRQ) )( fn(OPRimm<22 COMMA 19>) )( fn(OPRimm<18 COMMA 16>) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_fcvtzs_advsimd_int, 	"fcvtzs",	list_of( fn(setSIMDMode) )( fn(OPRsz<22 COMMA 22>) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_fcvtzs_advsimd_int, 	"fcvtzs",	list_of( fn(setSIMDMode) )( fn(OPRQ) )( fn(OPRsz<22 COMMA 22>) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_fcvtzs_float_fix, 	"fcvtzs",	list_of( fn(setFPMode) )( fn(OPRsf) )( fn(OPRtype<23 COMMA 22>) )( fn(OPRscale) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_fcvtzs_float_int, 	"fcvtzs",	list_of( fn(setFPMode) )( fn(OPRsf) )( fn(OPRtype<23 COMMA 22>) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_fcvtzu_advsimd_fix, 	"fcvtzu",	list_of( fn(setSIMDMode) )( fn(OPRimm<22 COMMA 19>) )( fn(OPRimm<18 COMMA 16>) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_fcvtzu_advsimd_fix, 	"fcvtzu",	list_of( fn(setSIMDMode) )( fn(OPRQ) )( fn(OPRimm<22 COMMA 19>) )( fn(OPRimm<18 COMMA 16>) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_fcvtzu_advsimd_int, 	"fcvtzu",	list_of( fn(setSIMDMode) )( fn(OPRsz<22 COMMA 22>) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_fcvtzu_advsimd_int, 	"fcvtzu",	list_of( fn(setSIMDMode) )( fn(OPRQ) )( fn(OPRsz<22 COMMA 22>) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_fcvtzu_float_fix, 	"fcvtzu",	list_of( fn(setFPMode) )( fn(OPRsf) )( fn(OPRtype<23 COMMA 22>) )( fn(OPRscale) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_fcvtzu_float_int, 	"fcvtzu",	list_of( fn(setFPMode) )( fn(OPRsf) )( fn(OPRtype<23 COMMA 22>) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_fdiv_advsimd, 	"fdiv",	list_of( fn(setSIMDMode) )( fn(OPRQ) )( fn(OPRsz<22 COMMA 22>) )( fn(OPRRm) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_fdiv_float, 	"fdiv",	list_of( fn(setFPMode) )( fn(OPRtype<23 COMMA 22>) )( fn(OPRRm) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_fmadd_float, 	"fmadd",	list_of( fn(setFPMode) )( fn(OPRtype<23 COMMA 22>) )( fn(OPRRm) )( fn(OPRRa) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_fmax_advsimd, 	"fmax",	list_of( fn(setSIMDMode) )( fn(OPRQ) )( fn(OPRsz<22 COMMA 22>) )( fn(OPRRm) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_fmax_float, 	"fmax",	list_of( fn(setFPMode) )( fn(OPRtype<23 COMMA 22>) )( fn(OPRRm) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_fmaxnm_advsimd, 	"fmaxnm",	list_of( fn(setSIMDMode) )( fn(OPRQ) )( fn(OPRsz<22 COMMA 22>) )( fn(OPRRm) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_fmaxnm_float, 	"fmaxnm",	list_of( fn(setFPMode) )( fn(OPRtype<23 COMMA 22>) )( fn(OPRRm) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_fmaxnmp_advsimd_pair, 	"fmaxnmp",	list_of( fn(setSIMDMode) )( fn(OPRsz<22 COMMA 22>) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_fmaxnmp_advsimd_vec, 	"fmaxnmp",	list_of( fn(setSIMDMode) )( fn(OPRQ) )( fn(OPRsz<22 COMMA 22>) )( fn(OPRRm) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_fmaxnmv_advsimd, 	"fmaxnmv",	list_of( fn(setSIMDMode) )( fn(OPRQ) )( fn(OPRsz<22 COMMA 22>) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_fmaxp_advsimd_pair, 	"fmaxp",	list_of( fn(setSIMDMode) )( fn(OPRsz<22 COMMA 22>) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_fmaxp_advsimd_vec, 	"fmaxp",	list_of( fn(setSIMDMode) )( fn(OPRQ) )( fn(OPRsz<22 COMMA 22>) )( fn(OPRRm) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_fmaxv_advsimd, 	"fmaxv",	list_of( fn(setSIMDMode) )( fn(OPRQ) )( fn(OPRsz<22 COMMA 22>) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_fmin_advsimd, 	"fmin",	list_of( fn(setSIMDMode) )( fn(OPRQ) )( fn(OPRsz<22 COMMA 22>) )( fn(OPRRm) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_fmin_float, 	"fmin",	list_of( fn(setFPMode) )( fn(OPRtype<23 COMMA 22>) )( fn(OPRRm) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_fminnm_advsimd, 	"fminnm",	list_of( fn(setSIMDMode) )( fn(OPRQ) )( fn(OPRsz<22 COMMA 22>) )( fn(OPRRm) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_fminnm_float, 	"fminnm",	list_of( fn(setFPMode) )( fn(OPRtype<23 COMMA 22>) )( fn(OPRRm) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_fminnmp_advsimd_pair, 	"fminnmp",	list_of( fn(setSIMDMode) )( fn(OPRsz<22 COMMA 22>) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_fminnmp_advsimd_vec, 	"fminnmp",	list_of( fn(setSIMDMode) )( fn(OPRQ) )( fn(OPRsz<22 COMMA 22>) )( fn(OPRRm) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_fminnmv_advsimd, 	"fminnmv",	list_of( fn(setSIMDMode) )( fn(OPRQ) )( fn(OPRsz<22 COMMA 22>) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_fminp_advsimd_pair, 	"fminp",	list_of( fn(setSIMDMode) )( fn(OPRsz<22 COMMA 22>) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_fminp_advsimd_vec, 	"fminp",	list_of( fn(setSIMDMode) )( fn(OPRQ) )( fn(OPRsz<22 COMMA 22>) )( fn(OPRRm) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_fminv_advsimd, 	"fminv",	list_of( fn(setSIMDMode) )( fn(OPRQ) )( fn(OPRsz<22 COMMA 22>) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_fmla_advsimd_elt, 	"fmla",	list_of( fn(setSIMDMode) )( fn(OPRsz<22 COMMA 22>) )( fn(OPRL) )( fn(OPRM) )( fn(OPRRm) )( fn(OPRH) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_fmla_advsimd_elt, 	"fmla",	list_of( fn(setSIMDMode) )( fn(OPRQ) )( fn(OPRsz<22 COMMA 22>) )( fn(OPRL) )( fn(OPRM) )( fn(OPRRm) )( fn(OPRH) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_fmla_advsimd_vec, 	"fmla",	list_of( fn(setSIMDMode) )( fn(OPRQ) )( fn(OPRsz<22 COMMA 22>) )( fn(OPRRm) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_fmls_advsimd_elt, 	"fmls",	list_of( fn(setSIMDMode) )( fn(OPRsz<22 COMMA 22>) )( fn(OPRL) )( fn(OPRM) )( fn(OPRRm) )( fn(OPRH) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_fmls_advsimd_elt, 	"fmls",	list_of( fn(setSIMDMode) )( fn(OPRQ) )( fn(OPRsz<22 COMMA 22>) )( fn(OPRL) )( fn(OPRM) )( fn(OPRRm) )( fn(OPRH) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_fmls_advsimd_vec, 	"fmls",	list_of( fn(setSIMDMode) )( fn(OPRQ) )( fn(OPRsz<22 COMMA 22>) )( fn(OPRRm) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_fmov_advsimd, 	"fmov",	list_of( fn(setSIMDMode) )( fn(OPRQ) )( fn(OPRop) )( fn(OPRa) )( fn(OPRb) )( fn(OPRc) )( fn(OPRd) )( fn(OPRe) )( fn(OPRf) )( fn(OPRg) )( fn(OPRh) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_fmov_float, 	"fmov",	list_of( fn(setFPMode) )( fn(OPRtype<23 COMMA 22>) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_fmov_float_gen, 	"fmov",	list_of( fn(setFPMode) )( fn(OPRsf) )( fn(OPRtype<23 COMMA 22>) )( fn(OPRrmode) )( fn(OPRopcode) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_fmov_float_imm, 	"fmov",	list_of( fn(setFPMode) )( fn(OPRtype<23 COMMA 22>) )( fn(OPRimm<20 COMMA 13>) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_fmsub_float, 	"fmsub",	list_of( fn(setFPMode) )( fn(OPRtype<23 COMMA 22>) )( fn(OPRRm) )( fn(OPRRa) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_fmul_advsimd_elt, 	"fmul",	list_of( fn(setSIMDMode) )( fn(OPRsz<22 COMMA 22>) )( fn(OPRL) )( fn(OPRM) )( fn(OPRRm) )( fn(OPRH) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_fmul_advsimd_elt, 	"fmul",	list_of( fn(setSIMDMode) )( fn(OPRQ) )( fn(OPRsz<22 COMMA 22>) )( fn(OPRL) )( fn(OPRM) )( fn(OPRRm) )( fn(OPRH) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_fmul_advsimd_vec, 	"fmul",	list_of( fn(setSIMDMode) )( fn(OPRQ) )( fn(OPRsz<22 COMMA 22>) )( fn(OPRRm) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_fmul_float, 	"fmul",	list_of( fn(setFPMode) )( fn(OPRtype<23 COMMA 22>) )( fn(OPRRm) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_fmulx_advsimd_elt, 	"fmulx",	list_of( fn(setSIMDMode) )( fn(OPRsz<22 COMMA 22>) )( fn(OPRL) )( fn(OPRM) )( fn(OPRRm) )( fn(OPRH) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_fmulx_advsimd_elt, 	"fmulx",	list_of( fn(setSIMDMode) )( fn(OPRQ) )( fn(OPRsz<22 COMMA 22>) )( fn(OPRL) )( fn(OPRM) )( fn(OPRRm) )( fn(OPRH) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_fmulx_advsimd_vec, 	"fmulx",	list_of( fn(setSIMDMode) )( fn(OPRsz<22 COMMA 22>) )( fn(OPRRm) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_fmulx_advsimd_vec, 	"fmulx",	list_of( fn(setSIMDMode) )( fn(OPRQ) )( fn(OPRsz<22 COMMA 22>) )( fn(OPRRm) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_fneg_advsimd, 	"fneg",	list_of( fn(setSIMDMode) )( fn(OPRQ) )( fn(OPRsz<22 COMMA 22>) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_fneg_float, 	"fneg",	list_of( fn(setFPMode) )( fn(OPRtype<23 COMMA 22>) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_fnmadd_float, 	"fnmadd",	list_of( fn(setFPMode) )( fn(OPRtype<23 COMMA 22>) )( fn(OPRRm) )( fn(OPRRa) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_fnmsub_float, 	"fnmsub",	list_of( fn(setFPMode) )( fn(OPRtype<23 COMMA 22>) )( fn(OPRRm) )( fn(OPRRa) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_fnmul_float, 	"fnmul",	list_of( fn(setFPMode) )( fn(OPRtype<23 COMMA 22>) )( fn(OPRRm) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_frecpe_advsimd, 	"frecpe",	list_of( fn(setSIMDMode) )( fn(OPRsz<22 COMMA 22>) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_frecpe_advsimd, 	"frecpe",	list_of( fn(setSIMDMode) )( fn(OPRQ) )( fn(OPRsz<22 COMMA 22>) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_frecps_advsimd, 	"frecps",	list_of( fn(setSIMDMode) )( fn(OPRsz<22 COMMA 22>) )( fn(OPRRm) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_frecps_advsimd, 	"frecps",	list_of( fn(setSIMDMode) )( fn(OPRQ) )( fn(OPRsz<22 COMMA 22>) )( fn(OPRRm) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_frecpx_advsimd, 	"frecpx",	list_of( fn(setSIMDMode) )( fn(OPRsz<22 COMMA 22>) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_frinta_advsimd, 	"frinta",	list_of( fn(setSIMDMode) )( fn(OPRQ) )( fn(OPRsz<22 COMMA 22>) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_frinta_float, 	"frinta",	list_of( fn(setFPMode) )( fn(OPRtype<23 COMMA 22>) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_frinti_advsimd, 	"frinti",	list_of( fn(setSIMDMode) )( fn(OPRQ) )( fn(OPRsz<22 COMMA 22>) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_frinti_float, 	"frinti",	list_of( fn(setFPMode) )( fn(OPRtype<23 COMMA 22>) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_frintm_advsimd, 	"frintm",	list_of( fn(setSIMDMode) )( fn(OPRQ) )( fn(OPRsz<22 COMMA 22>) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_frintm_float, 	"frintm",	list_of( fn(setFPMode) )( fn(OPRtype<23 COMMA 22>) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_frintn_advsimd, 	"frintn",	list_of( fn(setSIMDMode) )( fn(OPRQ) )( fn(OPRsz<22 COMMA 22>) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_frintn_float, 	"frintn",	list_of( fn(setFPMode) )( fn(OPRtype<23 COMMA 22>) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_frintp_advsimd, 	"frintp",	list_of( fn(setSIMDMode) )( fn(OPRQ) )( fn(OPRsz<22 COMMA 22>) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_frintp_float, 	"frintp",	list_of( fn(setFPMode) )( fn(OPRtype<23 COMMA 22>) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_frintx_advsimd, 	"frintx",	list_of( fn(setSIMDMode) )( fn(OPRQ) )( fn(OPRsz<22 COMMA 22>) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_frintx_float, 	"frintx",	list_of( fn(setFPMode) )( fn(OPRtype<23 COMMA 22>) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_frintz_advsimd, 	"frintz",	list_of( fn(setSIMDMode) )( fn(OPRQ) )( fn(OPRsz<22 COMMA 22>) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_frintz_float, 	"frintz",	list_of( fn(setFPMode) )( fn(OPRtype<23 COMMA 22>) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_frsqrte_advsimd, 	"frsqrte",	list_of( fn(setSIMDMode) )( fn(OPRsz<22 COMMA 22>) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_frsqrte_advsimd, 	"frsqrte",	list_of( fn(setSIMDMode) )( fn(OPRQ) )( fn(OPRsz<22 COMMA 22>) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_frsqrts_advsimd, 	"frsqrts",	list_of( fn(setSIMDMode) )( fn(OPRsz<22 COMMA 22>) )( fn(OPRRm) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_frsqrts_advsimd, 	"frsqrts",	list_of( fn(setSIMDMode) )( fn(OPRQ) )( fn(OPRsz<22 COMMA 22>) )( fn(OPRRm) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_fsqrt_advsimd, 	"fsqrt",	list_of( fn(setSIMDMode) )( fn(OPRQ) )( fn(OPRsz<22 COMMA 22>) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_fsqrt_float, 	"fsqrt",	list_of( fn(setFPMode) )( fn(OPRtype<23 COMMA 22>) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_fsub_advsimd, 	"fsub",	list_of( fn(setSIMDMode) )( fn(OPRQ) )( fn(OPRsz<22 COMMA 22>) )( fn(OPRRm) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_fsub_float, 	"fsub",	list_of( fn(setFPMode) )( fn(OPRtype<23 COMMA 22>) )( fn(OPRRm) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_hint, 	"hint",	list_of( fn(OPRCRm) )( fn(OPRop2) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_hlt, 	"hlt",	list_of( (operandFactory) fn(OPRimm<20 COMMA 5>) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_hvc, 	"hvc",	list_of( (operandFactory) fn(OPRimm<20 COMMA 5>) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_ic_sys, 	"ic",	list_of( fn(OPRop1) )( fn(OPRCRm) )( fn(OPRop2) )( fn(OPRRt) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_ins_advsimd_elt, 	"ins",	list_of( fn(setSIMDMode) )( fn(OPRimm<20 COMMA 16>) )( fn(OPRimm<14 COMMA 11>) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_ins_advsimd_gen, 	"ins",	list_of( fn(setSIMDMode) )( fn(OPRimm<20 COMMA 16>) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_isb, 	"isb",	list_of( fn(OPRCRm) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_ld1_advsimd_mult, 	"ld1",	list_of( fn(setSIMDMode) )( fn(OPRQ) )( fn(OPRopcode) )( fn(OPRopcode) )( fn(OPRopcode) )( fn(OPRsize<11 COMMA 10>) )( fn(OPRRnL) )( fn(OPRRtL) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_ld1_advsimd_mult, 	"ld1",	list_of( fn(setSIMDMode) )( fn(OPRQ) )( fn(OPRRm) )( fn(OPRopcode) )( fn(OPRopcode) )( fn(OPRopcode) )( fn(OPRsize<11 COMMA 10>) )( fn(OPRRnL) )( fn(OPRRtL) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_ld1_advsimd_sngl, 	"ld1",	list_of( fn(setSIMDMode) )( fn(OPRQ) )( fn(OPRopcode) )( fn(OPRopcode) )( fn(OPRS<12 COMMA 12>) )( fn(OPRsize<11 COMMA 10>) )( fn(OPRRnL) )( fn(OPRRtL) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_ld1_advsimd_sngl, 	"ld1",	list_of( fn(setSIMDMode) )( fn(OPRQ) )( fn(OPRRm) )( fn(OPRopcode) )( fn(OPRopcode) )( fn(OPRS<12 COMMA 12>) )( fn(OPRsize<11 COMMA 10>) )( fn(OPRRnL) )( fn(OPRRtL) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_ld1r_advsimd, 	"ld1r",	list_of( fn(setRegWidth) )( fn(setSIMDMode) )( fn(OPRQ) )( fn(OPRsize<11 COMMA 10>) )( fn(OPRRnL) )( fn(OPRRtL) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_ld1r_advsimd, 	"ld1r",	list_of( fn(setRegWidth) )( fn(setSIMDMode) )( fn(OPRQ) )( fn(OPRRm) )( fn(OPRsize<11 COMMA 10>) )( fn(OPRRnL) )( fn(OPRRtL) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_ld2_advsimd_mult, 	"ld2",	list_of( fn(setSIMDMode) )( fn(OPRQ) )( fn(OPRsize<11 COMMA 10>) )( fn(OPRRnL) )( fn(OPRRtL) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_ld2_advsimd_mult, 	"ld2",	list_of( fn(setSIMDMode) )( fn(OPRQ) )( fn(OPRRm) )( fn(OPRsize<11 COMMA 10>) )( fn(OPRRnL) )( fn(OPRRtL) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_ld2_advsimd_sngl, 	"ld2",	list_of( fn(setSIMDMode) )( fn(OPRQ) )( fn(OPRopcode) )( fn(OPRopcode) )( fn(OPRS<12 COMMA 12>) )( fn(OPRsize<11 COMMA 10>) )( fn(OPRRnL) )( fn(OPRRtL) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_ld2_advsimd_sngl, 	"ld2",	list_of( fn(setSIMDMode) )( fn(OPRQ) )( fn(OPRRm) )( fn(OPRopcode) )( fn(OPRopcode) )( fn(OPRS<12 COMMA 12>) )( fn(OPRsize<11 COMMA 10>) )( fn(OPRRnL) )( fn(OPRRtL) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_ld2r_advsimd, 	"ld2r",	list_of( fn(setRegWidth) )( fn(setSIMDMode) )( fn(OPRQ) )( fn(OPRsize<11 COMMA 10>) )( fn(OPRRnL) )( fn(OPRRtL) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_ld2r_advsimd, 	"ld2r",	list_of( fn(setRegWidth) )( fn(setSIMDMode) )( fn(OPRQ) )( fn(OPRRm) )( fn(OPRsize<11 COMMA 10>) )( fn(OPRRnL) )( fn(OPRRtL) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_ld3_advsimd_mult, 	"ld3",	list_of( fn(setSIMDMode) )( fn(OPRQ) )( fn(OPRsize<11 COMMA 10>) )( fn(OPRRnL) )( fn(OPRRtL) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_ld3_advsimd_mult, 	"ld3",	list_of( fn(setSIMDMode) )( fn(OPRQ) )( fn(OPRRm) )( fn(OPRsize<11 COMMA 10>) )( fn(OPRRnL) )( fn(OPRRtL) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_ld3_advsimd_sngl, 	"ld3",	list_of( fn(setSIMDMode) )( fn(OPRQ) )( fn(OPRopcode) )( fn(OPRopcode) )( fn(OPRS<12 COMMA 12>) )( fn(OPRsize<11 COMMA 10>) )( fn(OPRRnL) )( fn(OPRRtL) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_ld3_advsimd_sngl, 	"ld3",	list_of( fn(setSIMDMode) )( fn(OPRQ) )( fn(OPRRm) )( fn(OPRopcode) )( fn(OPRopcode) )( fn(OPRS<12 COMMA 12>) )( fn(OPRsize<11 COMMA 10>) )( fn(OPRRnL) )( fn(OPRRtL) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_ld3r_advsimd, 	"ld3r",	list_of( fn(setRegWidth) )( fn(setSIMDMode) )( fn(OPRQ) )( fn(OPRsize<11 COMMA 10>) )( fn(OPRRnL) )( fn(OPRRtL) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_ld3r_advsimd, 	"ld3r",	list_of( fn(setRegWidth) )( fn(setSIMDMode) )( fn(OPRQ) )( fn(OPRRm) )( fn(OPRsize<11 COMMA 10>) )( fn(OPRRnL) )( fn(OPRRtL) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_ld4_advsimd_mult, 	"ld4",	list_of( fn(setSIMDMode) )( fn(OPRQ) )( fn(OPRsize<11 COMMA 10>) )( fn(OPRRnL) )( fn(OPRRtL) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_ld4_advsimd_mult, 	"ld4",	list_of( fn(setSIMDMode) )( fn(OPRQ) )( fn(OPRRm) )( fn(OPRsize<11 COMMA 10>) )( fn(OPRRnL) )( fn(OPRRtL) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_ld4_advsimd_sngl, 	"ld4",	list_of( fn(setSIMDMode) )( fn(OPRQ) )( fn(OPRopcode) )( fn(OPRopcode) )( fn(OPRS<12 COMMA 12>) )( fn(OPRsize<11 COMMA 10>) )( fn(OPRRnL) )( fn(OPRRtL) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_ld4_advsimd_sngl, 	"ld4",	list_of( fn(setSIMDMode) )( fn(OPRQ) )( fn(OPRRm) )( fn(OPRopcode) )( fn(OPRopcode) )( fn(OPRS<12 COMMA 12>) )( fn(OPRsize<11 COMMA 10>) )( fn(OPRRnL) )( fn(OPRRtL) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_ld4r_advsimd, 	"ld4r",	list_of( fn(setRegWidth) )( fn(setSIMDMode) )( fn(OPRQ) )( fn(OPRsize<11 COMMA 10>) )( fn(OPRRnL) )( fn(OPRRtL) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_ld4r_advsimd, 	"ld4r",	list_of( fn(setRegWidth) )( fn(setSIMDMode) )( fn(OPRQ) )( fn(OPRRm) )( fn(OPRsize<11 COMMA 10>) )( fn(OPRRnL) )( fn(OPRRtL) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_ldar, 	"ldar",	list_of( fn(setRegWidth) )( fn(OPRsize<31 COMMA 30>) )( fn(OPRRnL) )( fn(OPRRtL) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_ldarb, 	"ldarb",	list_of( fn(setRegWidth) )( fn(OPRRnL) )( fn(OPRRtL) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_ldarh, 	"ldarh",	list_of( fn(setRegWidth) )( fn(OPRRnL) )( fn(OPRRtL) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_ldaxp, 	"ldaxp",	list_of( fn(setRegWidth) )( fn(OPRsz<30 COMMA 30>) )( fn(OPRRt2L) )( fn(OPRRnL) )( fn(OPRRtL) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_ldaxr, 	"ldaxr",	list_of( fn(setRegWidth) )( fn(OPRsize<31 COMMA 30>) )( fn(OPRRnL) )( fn(OPRRtL) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_ldaxrb, 	"ldaxrb",	list_of( fn(setRegWidth) )( fn(OPRRnL) )( fn(OPRRtL) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_ldaxrh, 	"ldaxrh",	list_of( fn(setRegWidth) )( fn(OPRRnL) )( fn(OPRRtL) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_ldnp_fpsimd, 	"ldnp",	list_of( fn(setRegWidth) )( fn(setSIMDMode) )( fn(OPRopc) )( fn(OPRimm<21 COMMA 15>) )( fn(OPRRt2L) )( fn(OPRRnL) )( fn(OPRRtL) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_ldnp_gen, 	"ldnp",	list_of( fn(setRegWidth) )( fn(OPRopc) )( fn(OPRimm<21 COMMA 15>) )( fn(OPRRt2L) )( fn(OPRRnL) )( fn(OPRRtL) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_ldp_fpsimd, 	"ldp",	list_of( fn(setRegWidth) )( fn(setSIMDMode) )( fn(OPRopc) )( fn(OPRimm<21 COMMA 15>) )( fn(OPRRt2L) )( fn(OPRRnLU) )( fn(OPRRtL) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_ldp_fpsimd, 	"ldp",	list_of( fn(setRegWidth) )( fn(setSIMDMode) )( fn(OPRopc) )( fn(OPRimm<21 COMMA 15>) )( fn(OPRRt2L) )( fn(OPRRnLU) )( fn(OPRRtL) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_ldp_fpsimd, 	"ldp",	list_of( fn(setRegWidth) )( fn(setSIMDMode) )( fn(OPRopc) )( fn(OPRimm<21 COMMA 15>) )( fn(OPRRt2L) )( fn(OPRRnL) )( fn(OPRRtL) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_ldp_gen, 	"ldp",	list_of( fn(setRegWidth) )( fn(OPRopc) )( fn(OPRimm<21 COMMA 15>) )( fn(OPRRt2L) )( fn(OPRRnLU) )( fn(OPRRtL) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_ldp_gen, 	"ldp",	list_of( fn(setRegWidth) )( fn(OPRopc) )( fn(OPRimm<21 COMMA 15>) )( fn(OPRRt2L) )( fn(OPRRnLU) )( fn(OPRRtL) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_ldp_gen, 	"ldp",	list_of( fn(setRegWidth) )( fn(OPRopc) )( fn(OPRimm<21 COMMA 15>) )( fn(OPRRt2L) )( fn(OPRRnL) )( fn(OPRRtL) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_ldpsw, 	"ldpsw",	list_of( fn(setRegWidth) )( fn(OPRimm<21 COMMA 15>) )( fn(OPRRt2L) )( fn(OPRRnLU) )( fn(OPRRtL) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_ldpsw, 	"ldpsw",	list_of( fn(setRegWidth) )( fn(OPRimm<21 COMMA 15>) )( fn(OPRRt2L) )( fn(OPRRnLU) )( fn(OPRRtL) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_ldpsw, 	"ldpsw",	list_of( fn(setRegWidth) )( fn(OPRimm<21 COMMA 15>) )( fn(OPRRt2L) )( fn(OPRRnL) )( fn(OPRRtL) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_ldr_imm_fpsimd, 	"ldr",	list_of( fn(setRegWidth) )( fn(setSIMDMode) )( fn(OPRsize<31 COMMA 30>) )( fn(OPRopc) )( fn(OPRimm<20 COMMA 12>) )( fn(OPRRnLU) )( fn(OPRRtL) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_ldr_imm_fpsimd, 	"ldr",	list_of( fn(setRegWidth) )( fn(setSIMDMode) )( fn(OPRsize<31 COMMA 30>) )( fn(OPRopc) )( fn(OPRimm<20 COMMA 12>) )( fn(OPRRnLU) )( fn(OPRRtL) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_ldr_imm_fpsimd, 	"ldr",	list_of( fn(setRegWidth) )( fn(setSIMDMode) )( fn(OPRsize<31 COMMA 30>) )( fn(OPRopc) )( fn(OPRimm<21 COMMA 10>) )( fn(OPRRnL) )( fn(OPRRtL) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_ldr_imm_gen, 	"ldr",	list_of( fn(setRegWidth) )( fn(OPRsize<31 COMMA 30>) )( fn(OPRimm<20 COMMA 12>) )( fn(OPRRnLU) )( fn(OPRRtL) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_ldr_imm_gen, 	"ldr",	list_of( fn(setRegWidth) )( fn(OPRsize<31 COMMA 30>) )( fn(OPRimm<20 COMMA 12>) )( fn(OPRRnLU) )( fn(OPRRtL) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_ldr_imm_gen, 	"ldr",	list_of( fn(setRegWidth) )( fn(OPRsize<31 COMMA 30>) )( fn(OPRimm<21 COMMA 10>) )( fn(OPRRnL) )( fn(OPRRtL) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_ldr_lit_fpsimd, 	"ldr",	list_of( fn(setRegWidth) )( fn(setSIMDMode) )( fn(OPRopc) )( fn(OPRimm<23 COMMA 5>) )( fn(OPRRtL) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_ldr_lit_gen, 	"ldr",	list_of( fn(setRegWidth) )( fn(OPRopc) )( fn(OPRimm<23 COMMA 5>) )( fn(OPRRtL) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_ldr_reg_fpsimd, 	"ldr",	list_of( fn(setRegWidth) )( fn(setSIMDMode) )( fn(OPRsize<31 COMMA 30>) )( fn(OPRopc) )( fn(OPRRm) )( fn(OPRoption<15 COMMA 13>) )( fn(OPRS<12 COMMA 12>) )( fn(OPRRnL) )( fn(OPRRtL) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_ldr_reg_gen, 	"ldr",	list_of( fn(setRegWidth) )( fn(OPRsize<31 COMMA 30>) )( fn(OPRRm) )( fn(OPRoption<15 COMMA 13>) )( fn(OPRS<12 COMMA 12>) )( fn(OPRRnL) )( fn(OPRRtL) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_ldrb_imm, 	"ldrb",	list_of( fn(setRegWidth) )( fn(OPRimm<20 COMMA 12>) )( fn(OPRRnLU) )( fn(OPRRtL) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_ldrb_imm, 	"ldrb",	list_of( fn(setRegWidth) )( fn(OPRimm<20 COMMA 12>) )( fn(OPRRnLU) )( fn(OPRRtL) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_ldrb_imm, 	"ldrb",	list_of( fn(setRegWidth) )( fn(OPRimm<21 COMMA 10>) )( fn(OPRRnL) )( fn(OPRRtL) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_ldrb_reg, 	"ldrb",	list_of( fn(setRegWidth) )( fn(OPRRm) )( fn(OPRoption<15 COMMA 13>) )( fn(OPRS<12 COMMA 12>) )( fn(OPRRnL) )( fn(OPRRtL) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_ldrh_imm, 	"ldrh",	list_of( fn(setRegWidth) )( fn(OPRimm<20 COMMA 12>) )( fn(OPRRnLU) )( fn(OPRRtL) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_ldrh_imm, 	"ldrh",	list_of( fn(setRegWidth) )( fn(OPRimm<20 COMMA 12>) )( fn(OPRRnLU) )( fn(OPRRtL) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_ldrh_imm, 	"ldrh",	list_of( fn(setRegWidth) )( fn(OPRimm<21 COMMA 10>) )( fn(OPRRnL) )( fn(OPRRtL) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_ldrh_reg, 	"ldrh",	list_of( fn(setRegWidth) )( fn(OPRRm) )( fn(OPRoption<15 COMMA 13>) )( fn(OPRS<12 COMMA 12>) )( fn(OPRRnL) )( fn(OPRRtL) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_ldrsb_imm, 	"ldrsb",	list_of( fn(setRegWidth) )( fn(OPRopc) )( fn(OPRimm<20 COMMA 12>) )( fn(OPRRnLU) )( fn(OPRRtL) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_ldrsb_imm, 	"ldrsb",	list_of( fn(setRegWidth) )( fn(OPRopc) )( fn(OPRimm<20 COMMA 12>) )( fn(OPRRnLU) )( fn(OPRRtL) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_ldrsb_imm, 	"ldrsb",	list_of( fn(setRegWidth) )( fn(OPRopc) )( fn(OPRimm<21 COMMA 10>) )( fn(OPRRnL) )( fn(OPRRtL) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_ldrsb_reg, 	"ldrsb",	list_of( fn(setRegWidth) )( fn(OPRopc) )( fn(OPRRm) )( fn(OPRoption<15 COMMA 13>) )( fn(OPRS<12 COMMA 12>) )( fn(OPRRnL) )( fn(OPRRtL) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_ldrsh_imm, 	"ldrsh",	list_of( fn(setRegWidth) )( fn(OPRopc) )( fn(OPRimm<20 COMMA 12>) )( fn(OPRRnLU) )( fn(OPRRtL) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_ldrsh_imm, 	"ldrsh",	list_of( fn(setRegWidth) )( fn(OPRopc) )( fn(OPRimm<20 COMMA 12>) )( fn(OPRRnLU) )( fn(OPRRtL) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_ldrsh_imm, 	"ldrsh",	list_of( fn(setRegWidth) )( fn(OPRopc) )( fn(OPRimm<21 COMMA 10>) )( fn(OPRRnL) )( fn(OPRRtL) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_ldrsh_reg, 	"ldrsh",	list_of( fn(setRegWidth) )( fn(OPRopc) )( fn(OPRRm) )( fn(OPRoption<15 COMMA 13>) )( fn(OPRS<12 COMMA 12>) )( fn(OPRRnL) )( fn(OPRRtL) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_ldrsw_imm, 	"ldrsw",	list_of( fn(setRegWidth) )( fn(OPRimm<20 COMMA 12>) )( fn(OPRRnLU) )( fn(OPRRtL) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_ldrsw_imm, 	"ldrsw",	list_of( fn(setRegWidth) )( fn(OPRimm<20 COMMA 12>) )( fn(OPRRnLU) )( fn(OPRRtL) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_ldrsw_imm, 	"ldrsw",	list_of( fn(setRegWidth) )( fn(OPRimm<21 COMMA 10>) )( fn(OPRRnL) )( fn(OPRRtL) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_ldrsw_lit, 	"ldrsw",	list_of( fn(setRegWidth) )( fn(OPRimm<23 COMMA 5>) )( fn(OPRRtL) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_ldrsw_reg, 	"ldrsw",	list_of( fn(setRegWidth) )( fn(OPRRm) )( fn(OPRoption<15 COMMA 13>) )( fn(OPRS<12 COMMA 12>) )( fn(OPRRnL) )( fn(OPRRtL) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_ldtr, 	"ldtr",	list_of( fn(setRegWidth) )( fn(OPRsize<31 COMMA 30>) )( fn(OPRimm<20 COMMA 12>) )( fn(OPRRnL) )( fn(OPRRtL) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_ldtrb, 	"ldtrb",	list_of( fn(setRegWidth) )( fn(OPRimm<20 COMMA 12>) )( fn(OPRRnL) )( fn(OPRRtL) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_ldtrh, 	"ldtrh",	list_of( fn(setRegWidth) )( fn(OPRimm<20 COMMA 12>) )( fn(OPRRnL) )( fn(OPRRtL) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_ldtrsb, 	"ldtrsb",	list_of( fn(setRegWidth) )( fn(OPRopc) )( fn(OPRimm<20 COMMA 12>) )( fn(OPRRnL) )( fn(OPRRtL) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_ldtrsh, 	"ldtrsh",	list_of( fn(setRegWidth) )( fn(OPRopc) )( fn(OPRimm<20 COMMA 12>) )( fn(OPRRnL) )( fn(OPRRtL) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_ldtrsw, 	"ldtrsw",	list_of( fn(setRegWidth) )( fn(OPRimm<20 COMMA 12>) )( fn(OPRRnL) )( fn(OPRRtL) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_ldur_fpsimd, 	"ldur",	list_of( fn(setRegWidth) )( fn(setSIMDMode) )( fn(OPRsize<31 COMMA 30>) )( fn(OPRopc) )( fn(OPRimm<20 COMMA 12>) )( fn(OPRRnL) )( fn(OPRRtL) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_ldur_gen, 	"ldur",	list_of( fn(setRegWidth) )( fn(OPRsize<31 COMMA 30>) )( fn(OPRimm<20 COMMA 12>) )( fn(OPRRnL) )( fn(OPRRtL) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_ldurb, 	"ldurb",	list_of( fn(setRegWidth) )( fn(OPRimm<20 COMMA 12>) )( fn(OPRRnL) )( fn(OPRRtL) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_ldurh, 	"ldurh",	list_of( fn(setRegWidth) )( fn(OPRimm<20 COMMA 12>) )( fn(OPRRnL) )( fn(OPRRtL) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_ldursb, 	"ldursb",	list_of( fn(setRegWidth) )( fn(OPRopc) )( fn(OPRimm<20 COMMA 12>) )( fn(OPRRnL) )( fn(OPRRtL) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_ldursh, 	"ldursh",	list_of( fn(setRegWidth) )( fn(OPRopc) )( fn(OPRimm<20 COMMA 12>) )( fn(OPRRnL) )( fn(OPRRtL) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_ldursw, 	"ldursw",	list_of( fn(setRegWidth) )( fn(OPRimm<20 COMMA 12>) )( fn(OPRRnL) )( fn(OPRRtL) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_ldxp, 	"ldxp",	list_of( fn(setRegWidth) )( fn(OPRsz<30 COMMA 30>) )( fn(OPRRt2L) )( fn(OPRRnL) )( fn(OPRRtL) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_ldxr, 	"ldxr",	list_of( fn(setRegWidth) )( fn(OPRsize<31 COMMA 30>) )( fn(OPRRnL) )( fn(OPRRtL) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_ldxrb, 	"ldxrb",	list_of( fn(setRegWidth) )( fn(OPRRnL) )( fn(OPRRtL) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_ldxrh, 	"ldxrh",	list_of( fn(setRegWidth) )( fn(OPRRnL) )( fn(OPRRtL) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_lsl_lslv, 	"lsl",	list_of( fn(OPRsf) )( fn(OPRRm) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_lsl_ubfm, 	"lsl",	list_of( fn(OPRsf) )( fn(OPRN<22 COMMA 22>) )( fn(OPRimm<21 COMMA 16>) )( fn(OPRimm<15 COMMA 10>) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_lslv, 	"lslv",	list_of( fn(OPRsf) )( fn(OPRRm) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_lsr_lsrv, 	"lsr",	list_of( fn(OPRsf) )( fn(OPRRm) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_lsr_ubfm, 	"lsr",	list_of( fn(OPRsf) )( fn(OPRN<22 COMMA 22>) )( fn(OPRimm<21 COMMA 16>) )( fn(OPRimm<15 COMMA 10>) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_lsrv, 	"lsrv",	list_of( fn(OPRsf) )( fn(OPRRm) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_madd, 	"madd",	list_of( fn(OPRsf) )( fn(OPRRm) )( fn(OPRRa) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_mla_advsimd_elt, 	"mla",	list_of( fn(setSIMDMode) )( fn(OPRQ) )( fn(OPRsize<23 COMMA 22>) )( fn(OPRL) )( fn(OPRM) )( fn(OPRRm) )( fn(OPRH) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_mla_advsimd_vec, 	"mla",	list_of( fn(setSIMDMode) )( fn(OPRQ) )( fn(OPRsize<23 COMMA 22>) )( fn(OPRRm) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_mls_advsimd_elt, 	"mls",	list_of( fn(setSIMDMode) )( fn(OPRQ) )( fn(OPRsize<23 COMMA 22>) )( fn(OPRL) )( fn(OPRM) )( fn(OPRRm) )( fn(OPRH) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_mls_advsimd_vec, 	"mls",	list_of( fn(setSIMDMode) )( fn(OPRQ) )( fn(OPRsize<23 COMMA 22>) )( fn(OPRRm) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_mneg_msub, 	"mneg",	list_of( fn(OPRsf) )( fn(OPRRm) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_mov_add_addsub_imm, 	"mov",	list_of( fn(OPRsf) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_mov_dup_advsimd_elt, 	"mov",	list_of( fn(setSIMDMode) )( fn(OPRimm<20 COMMA 16>) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_mov_ins_advsimd_elt, 	"mov",	list_of( fn(setSIMDMode) )( fn(OPRimm<20 COMMA 16>) )( fn(OPRimm<14 COMMA 11>) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_mov_ins_advsimd_gen, 	"mov",	list_of( fn(setSIMDMode) )( fn(OPRimm<20 COMMA 16>) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_mov_movn, 	"mov",	list_of( fn(OPRsf) )( fn(OPRhw) )( fn(OPRimm<20 COMMA 5>) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_mov_movz, 	"mov",	list_of( fn(OPRsf) )( fn(OPRhw) )( fn(OPRimm<20 COMMA 5>) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_mov_orr_advsimd_reg, 	"mov",	list_of( fn(setSIMDMode) )( fn(OPRQ) )( fn(OPRRm) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_mov_orr_log_imm, 	"mov",	list_of( fn(OPRsf) )( fn(OPRN<22 COMMA 22>) )( fn(OPRimm<21 COMMA 16>) )( fn(OPRimm<15 COMMA 10>) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_mov_orr_log_shift, 	"mov",	list_of( fn(OPRsf) )( fn(OPRRm) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_mov_umov_advsimd, 	"mov",	list_of( fn(setSIMDMode) )( fn(OPRQ) )( fn(OPRimm<20 COMMA 16>) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_movi_advsimd, 	"movi",	list_of( fn(setSIMDMode) )( fn(OPRQ) )( fn(OPRop) )( fn(OPRa) )( fn(OPRb) )( fn(OPRc) )( fn(OPRcmode) )( fn(OPRd) )( fn(OPRe) )( fn(OPRf) )( fn(OPRg) )( fn(OPRh) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_movk, 	"movk",	list_of( fn(OPRsf) )( fn(OPRhw) )( fn(OPRimm<20 COMMA 5>) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_movn, 	"movn",	list_of( fn(OPRsf) )( fn(OPRhw) )( fn(OPRimm<20 COMMA 5>) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_movz, 	"movz",	list_of( fn(OPRsf) )( fn(OPRhw) )( fn(OPRimm<20 COMMA 5>) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_mrs, 	"mrs",	list_of( fn(OPRo0) )( fn(OPRop1) )( fn(OPRCRn) )( fn(OPRCRm) )( fn(OPRop2) )( fn(OPRRt) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_msr_imm, 	"msr",	list_of( fn(OPRop1) )( fn(OPRCRm) )( fn(OPRop2) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_msr_reg, 	"msr",	list_of( fn(OPRo0) )( fn(OPRop1) )( fn(OPRCRn) )( fn(OPRCRm) )( fn(OPRop2) )( fn(OPRRt) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_msub, 	"msub",	list_of( fn(OPRsf) )( fn(OPRRm) )( fn(OPRRa) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_mul_advsimd_elt, 	"mul",	list_of( fn(setSIMDMode) )( fn(OPRQ) )( fn(OPRsize<23 COMMA 22>) )( fn(OPRL) )( fn(OPRM) )( fn(OPRRm) )( fn(OPRH) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_mul_advsimd_vec, 	"mul",	list_of( fn(setSIMDMode) )( fn(OPRQ) )( fn(OPRsize<23 COMMA 22>) )( fn(OPRRm) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_mul_madd, 	"mul",	list_of( fn(OPRsf) )( fn(OPRRm) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_mvn_not_advsimd, 	"mvn",	list_of( fn(setSIMDMode) )( fn(OPRQ) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_mvn_orn_log_shift, 	"mvn",	list_of( fn(OPRsf) )( fn(OPRshift) )( fn(OPRRm) )( fn(OPRimm<15 COMMA 10>) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_mvni_advsimd, 	"mvni",	list_of( fn(setSIMDMode) )( fn(OPRQ) )( fn(OPRa) )( fn(OPRb) )( fn(OPRc) )( fn(OPRcmode) )( fn(OPRd) )( fn(OPRe) )( fn(OPRf) )( fn(OPRg) )( fn(OPRh) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_neg_advsimd, 	"neg",	list_of( fn(setSIMDMode) )( fn(OPRsize<23 COMMA 22>) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_neg_advsimd, 	"neg",	list_of( fn(setSIMDMode) )( fn(OPRQ) )( fn(OPRsize<23 COMMA 22>) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_neg_sub_addsub_shift, 	"neg",	list_of( fn(OPRsf) )( fn(OPRshift) )( fn(OPRRm) )( fn(OPRimm<15 COMMA 10>) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_negs_subs_addsub_shift, 	"negs",	list_of( fn(setFlags) )( fn(OPRsf) )( fn(OPRshift) )( fn(OPRRm) )( fn(OPRimm<15 COMMA 10>) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_ngc_sbc, 	"ngc",	list_of( fn(OPRsf) )( fn(OPRRm) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_ngcs_sbcs, 	"ngcs",	list_of( fn(setFlags) )( fn(OPRsf) )( fn(OPRRm) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_nop_hint, 	"nop",	operandSpec() ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_not_advsimd, 	"not",	list_of( fn(setSIMDMode) )( fn(OPRQ) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_orn_advsimd, 	"orn",	list_of( fn(setSIMDMode) )( fn(OPRQ) )( fn(OPRRm) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_orn_log_shift, 	"orn",	list_of( fn(OPRsf) )( fn(OPRshift) )( fn(OPRRm) )( fn(OPRimm<15 COMMA 10>) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_orr_advsimd_imm, 	"orr",	list_of( fn(setSIMDMode) )( fn(OPRQ) )( fn(OPRa) )( fn(OPRb) )( fn(OPRc) )( fn(OPRcmode) )( fn(OPRcmode) )( fn(OPRcmode) )( fn(OPRd) )( fn(OPRe) )( fn(OPRf) )( fn(OPRg) )( fn(OPRh) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_orr_advsimd_reg, 	"orr",	list_of( fn(setSIMDMode) )( fn(OPRQ) )( fn(OPRRm) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_orr_log_imm, 	"orr",	list_of( fn(OPRsf) )( fn(OPRN<22 COMMA 22>) )( fn(OPRimm<21 COMMA 16>) )( fn(OPRimm<15 COMMA 10>) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_orr_log_shift, 	"orr",	list_of( fn(OPRsf) )( fn(OPRshift) )( fn(OPRRm) )( fn(OPRimm<15 COMMA 10>) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_pmul_advsimd, 	"pmul",	list_of( fn(setSIMDMode) )( fn(OPRQ) )( fn(OPRsize<23 COMMA 22>) )( fn(OPRRm) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_pmull_advsimd, 	"pmull",	list_of( fn(setSIMDMode) )( fn(OPRQ) )( fn(OPRsize<23 COMMA 22>) )( fn(OPRRm) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_prfm_imm, 	"prfm",	list_of( (operandFactory) fn(OPRimm<21 COMMA 10>) )( fn(OPRRn) )( fn(OPRRt) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_prfm_lit, 	"prfm",	list_of( (operandFactory) fn(OPRimm<23 COMMA 5>) )( fn(OPRRt) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_prfm_reg, 	"prfm",	list_of( fn(OPRRm) )( fn(OPRoption<15 COMMA 13>) )( fn(OPRS<12 COMMA 12>) )( fn(OPRRn) )( fn(OPRRt) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_prfum, 	"prfum",	list_of( (operandFactory) fn(OPRimm<20 COMMA 12>) )( fn(OPRRn) )( fn(OPRRt) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_raddhn_advsimd, 	"raddhn",	list_of( fn(setSIMDMode) )( fn(OPRQ) )( fn(OPRsize<23 COMMA 22>) )( fn(OPRRm) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_rbit_advsimd, 	"rbit",	list_of( fn(setSIMDMode) )( fn(OPRQ) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_rbit_int, 	"rbit",	list_of( fn(OPRsf) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_ret, 	"ret",	list_of( fn(OPRRn) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_rev, 	"rev",	list_of( fn(OPRsf) )( fn(OPRopc) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_rev16_advsimd, 	"rev16",	list_of( fn(setSIMDMode) )( fn(OPRQ) )( fn(OPRsize<23 COMMA 22>) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_rev16_int, 	"rev16",	list_of( fn(OPRsf) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_rev32_advsimd, 	"rev32",	list_of( fn(setSIMDMode) )( fn(OPRQ) )( fn(OPRsize<23 COMMA 22>) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_rev32_int, 	"rev32",	list_of( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_rev64_advsimd, 	"rev64",	list_of( fn(setSIMDMode) )( fn(OPRQ) )( fn(OPRsize<23 COMMA 22>) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_ror_extr, 	"ror",	list_of( fn(OPRsf) )( fn(OPRN<22 COMMA 22>) )( fn(OPRRm) )( fn(OPRimm<15 COMMA 10>) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_ror_rorv, 	"ror",	list_of( fn(OPRsf) )( fn(OPRRm) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_rorv, 	"rorv",	list_of( fn(OPRsf) )( fn(OPRRm) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_rshrn_advsimd, 	"rshrn",	list_of( fn(setSIMDMode) )( fn(OPRQ) )( fn(OPRimm<22 COMMA 19>) )( fn(OPRimm<18 COMMA 16>) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_rsubhn_advsimd, 	"rsubhn",	list_of( fn(setSIMDMode) )( fn(OPRQ) )( fn(OPRsize<23 COMMA 22>) )( fn(OPRRm) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_saba_advsimd, 	"saba",	list_of( fn(setSIMDMode) )( fn(OPRQ) )( fn(OPRsize<23 COMMA 22>) )( fn(OPRRm) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_sabal_advsimd, 	"sabal",	list_of( fn(setSIMDMode) )( fn(OPRQ) )( fn(OPRsize<23 COMMA 22>) )( fn(OPRRm) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_sabd_advsimd, 	"sabd",	list_of( fn(setSIMDMode) )( fn(OPRQ) )( fn(OPRsize<23 COMMA 22>) )( fn(OPRRm) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_sabdl_advsimd, 	"sabdl",	list_of( fn(setSIMDMode) )( fn(OPRQ) )( fn(OPRsize<23 COMMA 22>) )( fn(OPRRm) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_sadalp_advsimd, 	"sadalp",	list_of( fn(setSIMDMode) )( fn(OPRQ) )( fn(OPRsize<23 COMMA 22>) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_saddl_advsimd, 	"saddl",	list_of( fn(setSIMDMode) )( fn(OPRQ) )( fn(OPRsize<23 COMMA 22>) )( fn(OPRRm) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_saddlp_advsimd, 	"saddlp",	list_of( fn(setSIMDMode) )( fn(OPRQ) )( fn(OPRsize<23 COMMA 22>) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_saddlv_advsimd, 	"saddlv",	list_of( fn(setSIMDMode) )( fn(OPRQ) )( fn(OPRsize<23 COMMA 22>) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_saddw_advsimd, 	"saddw",	list_of( fn(setSIMDMode) )( fn(OPRQ) )( fn(OPRsize<23 COMMA 22>) )( fn(OPRRm) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_sbc, 	"sbc",	list_of( fn(OPRsf) )( fn(OPRRm) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_sbcs, 	"sbcs",	list_of( fn(setFlags) )( fn(OPRsf) )( fn(OPRRm) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_sbfiz_sbfm, 	"sbfiz",	list_of( fn(OPRsf) )( fn(OPRN<22 COMMA 22>) )( fn(OPRimm<21 COMMA 16>) )( fn(OPRimm<15 COMMA 10>) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_sbfm, 	"sbfm",	list_of( fn(OPRsf) )( fn(OPRN<22 COMMA 22>) )( fn(OPRimm<21 COMMA 16>) )( fn(OPRimm<15 COMMA 10>) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_sbfx_sbfm, 	"sbfx",	list_of( fn(OPRsf) )( fn(OPRN<22 COMMA 22>) )( fn(OPRimm<21 COMMA 16>) )( fn(OPRimm<15 COMMA 10>) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_scvtf_advsimd_fix, 	"scvtf",	list_of( fn(setSIMDMode) )( fn(OPRimm<22 COMMA 19>) )( fn(OPRimm<18 COMMA 16>) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_scvtf_advsimd_fix, 	"scvtf",	list_of( fn(setSIMDMode) )( fn(OPRQ) )( fn(OPRimm<22 COMMA 19>) )( fn(OPRimm<18 COMMA 16>) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_scvtf_advsimd_int, 	"scvtf",	list_of( fn(setSIMDMode) )( fn(OPRsz<22 COMMA 22>) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_scvtf_advsimd_int, 	"scvtf",	list_of( fn(setSIMDMode) )( fn(OPRQ) )( fn(OPRsz<22 COMMA 22>) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_scvtf_float_fix, 	"scvtf",	list_of( fn(setFPMode) )( fn(OPRsf) )( fn(OPRtype<23 COMMA 22>) )( fn(OPRscale) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_scvtf_float_int, 	"scvtf",	list_of( fn(setFPMode) )( fn(OPRsf) )( fn(OPRtype<23 COMMA 22>) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_sdiv, 	"sdiv",	list_of( fn(OPRsf) )( fn(OPRRm) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_sev_hint, 	"sev",	operandSpec() ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_sevl_hint, 	"sevl",	operandSpec() ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_sha1c_advsimd, 	"sha1c",	list_of( fn(setSIMDMode) )( fn(OPRRm) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_sha1h_advsimd, 	"sha1h",	list_of( fn(setSIMDMode) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_sha1m_advsimd, 	"sha1m",	list_of( fn(setSIMDMode) )( fn(OPRRm) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_sha1p_advsimd, 	"sha1p",	list_of( fn(setSIMDMode) )( fn(OPRRm) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_sha1su0_advsimd, 	"sha1su0",	list_of( fn(setSIMDMode) )( fn(OPRRm) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_sha1su1_advsimd, 	"sha1su1",	list_of( fn(setSIMDMode) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_sha256h2_advsimd, 	"sha256h2",	list_of( fn(setSIMDMode) )( fn(OPRRm) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_sha256h_advsimd, 	"sha256h",	list_of( fn(setSIMDMode) )( fn(OPRRm) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_sha256su0_advsimd, 	"sha256su0",	list_of( fn(setSIMDMode) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_sha256su1_advsimd, 	"sha256su1",	list_of( fn(setSIMDMode) )( fn(OPRRm) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_shadd_advsimd, 	"shadd",	list_of( fn(setSIMDMode) )( fn(OPRQ) )( fn(OPRsize<23 COMMA 22>) )( fn(OPRRm) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_shl_advsimd, 	"shl",	list_of( fn(setSIMDMode) )( fn(OPRimm<22 COMMA 19>) )( fn(OPRimm<18 COMMA 16>) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_shl_advsimd, 	"shl",	list_of( fn(setSIMDMode) )( fn(OPRQ) )( fn(OPRimm<22 COMMA 19>) )( fn(OPRimm<18 COMMA 16>) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_shll_advsimd, 	"shll",	list_of( fn(setSIMDMode) )( fn(OPRQ) )( fn(OPRsize<23 COMMA 22>) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_shrn_advsimd, 	"shrn",	list_of( fn(setSIMDMode) )( fn(OPRQ) )( fn(OPRimm<22 COMMA 19>) )( fn(OPRimm<18 COMMA 16>) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_shsub_advsimd, 	"shsub",	list_of( fn(setSIMDMode) )( fn(OPRQ) )( fn(OPRsize<23 COMMA 22>) )( fn(OPRRm) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_sli_advsimd, 	"sli",	list_of( fn(setSIMDMode) )( fn(OPRimm<22 COMMA 19>) )( fn(OPRimm<18 COMMA 16>) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_sli_advsimd, 	"sli",	list_of( fn(setSIMDMode) )( fn(OPRQ) )( fn(OPRimm<22 COMMA 19>) )( fn(OPRimm<18 COMMA 16>) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_smaddl, 	"smaddl",	list_of( fn(OPRRm) )( fn(OPRRa) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_smax_advsimd, 	"smax",	list_of( fn(setSIMDMode) )( fn(OPRQ) )( fn(OPRsize<23 COMMA 22>) )( fn(OPRRm) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_smaxp_advsimd, 	"smaxp",	list_of( fn(setSIMDMode) )( fn(OPRQ) )( fn(OPRsize<23 COMMA 22>) )( fn(OPRRm) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_smaxv_advsimd, 	"smaxv",	list_of( fn(setSIMDMode) )( fn(OPRQ) )( fn(OPRsize<23 COMMA 22>) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_smc, 	"smc",	list_of( (operandFactory) fn(OPRimm<20 COMMA 5>) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_smin_advsimd, 	"smin",	list_of( fn(setSIMDMode) )( fn(OPRQ) )( fn(OPRsize<23 COMMA 22>) )( fn(OPRRm) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_sminp_advsimd, 	"sminp",	list_of( fn(setSIMDMode) )( fn(OPRQ) )( fn(OPRsize<23 COMMA 22>) )( fn(OPRRm) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_sminv_advsimd, 	"sminv",	list_of( fn(setSIMDMode) )( fn(OPRQ) )( fn(OPRsize<23 COMMA 22>) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_smlal_advsimd_elt, 	"smlal",	list_of( fn(setSIMDMode) )( fn(OPRQ) )( fn(OPRsize<23 COMMA 22>) )( fn(OPRL) )( fn(OPRM) )( fn(OPRRm) )( fn(OPRH) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_smlal_advsimd_vec, 	"smlal",	list_of( fn(setSIMDMode) )( fn(OPRQ) )( fn(OPRsize<23 COMMA 22>) )( fn(OPRRm) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_smlsl_advsimd_elt, 	"smlsl",	list_of( fn(setSIMDMode) )( fn(OPRQ) )( fn(OPRsize<23 COMMA 22>) )( fn(OPRL) )( fn(OPRM) )( fn(OPRRm) )( fn(OPRH) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_smlsl_advsimd_vec, 	"smlsl",	list_of( fn(setSIMDMode) )( fn(OPRQ) )( fn(OPRsize<23 COMMA 22>) )( fn(OPRRm) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_smnegl_smsubl, 	"smnegl",	list_of( fn(OPRRm) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_smov_advsimd, 	"smov",	list_of( fn(setSIMDMode) )( fn(OPRQ) )( fn(OPRimm<20 COMMA 16>) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_smsubl, 	"smsubl",	list_of( fn(OPRRm) )( fn(OPRRa) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_smulh, 	"smulh",	list_of( fn(OPRRm) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_smull_advsimd_elt, 	"smull",	list_of( fn(setSIMDMode) )( fn(OPRQ) )( fn(OPRsize<23 COMMA 22>) )( fn(OPRL) )( fn(OPRM) )( fn(OPRRm) )( fn(OPRH) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_smull_advsimd_vec, 	"smull",	list_of( fn(setSIMDMode) )( fn(OPRQ) )( fn(OPRsize<23 COMMA 22>) )( fn(OPRRm) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_smull_smaddl, 	"smull",	list_of( fn(OPRRm) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_sqabs_advsimd, 	"sqabs",	list_of( fn(setSIMDMode) )( fn(OPRsize<23 COMMA 22>) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_sqabs_advsimd, 	"sqabs",	list_of( fn(setSIMDMode) )( fn(OPRQ) )( fn(OPRsize<23 COMMA 22>) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_sqadd_advsimd, 	"sqadd",	list_of( fn(setSIMDMode) )( fn(OPRsize<23 COMMA 22>) )( fn(OPRRm) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_sqadd_advsimd, 	"sqadd",	list_of( fn(setSIMDMode) )( fn(OPRQ) )( fn(OPRsize<23 COMMA 22>) )( fn(OPRRm) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_sqdmlal_advsimd_elt, 	"sqdmlal",	list_of( fn(setSIMDMode) )( fn(OPRsize<23 COMMA 22>) )( fn(OPRL) )( fn(OPRM) )( fn(OPRRm) )( fn(OPRH) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_sqdmlal_advsimd_elt, 	"sqdmlal",	list_of( fn(setSIMDMode) )( fn(OPRQ) )( fn(OPRsize<23 COMMA 22>) )( fn(OPRL) )( fn(OPRM) )( fn(OPRRm) )( fn(OPRH) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_sqdmlal_advsimd_vec, 	"sqdmlal",	list_of( fn(setSIMDMode) )( fn(OPRsize<23 COMMA 22>) )( fn(OPRRm) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_sqdmlal_advsimd_vec, 	"sqdmlal",	list_of( fn(setSIMDMode) )( fn(OPRQ) )( fn(OPRsize<23 COMMA 22>) )( fn(OPRRm) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_sqdmlsl_advsimd_elt, 	"sqdmlsl",	list_of( fn(setSIMDMode) )( fn(OPRsize<23 COMMA 22>) )( fn(OPRL) )( fn(OPRM) )( fn(OPRRm) )( fn(OPRH) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_sqdmlsl_advsimd_elt, 	"sqdmlsl",	list_of( fn(setSIMDMode) )( fn(OPRQ) )( fn(OPRsize<23 COMMA 22>) )( fn(OPRL) )( fn(OPRM) )( fn(OPRRm) )( fn(OPRH) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_sqdmlsl_advsimd_vec, 	"sqdmlsl",	list_of( fn(setSIMDMode) )( fn(OPRsize<23 COMMA 22>) )( fn(OPRRm) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_sqdmlsl_advsimd_vec, 	"sqdmlsl",	list_of( fn(setSIMDMode) )( fn(OPRQ) )( fn(OPRsize<23 COMMA 22>) )( fn(OPRRm) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_sqdmulh_advsimd_elt, 	"sqdmulh",	list_of( fn(setSIMDMode) )( fn(OPRsize<23 COMMA 22>) )( fn(OPRL) )( fn(OPRM) )( fn(OPRRm) )( fn(OPRH) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_sqdmulh_advsimd_elt, 	"sqdmulh",	list_of( fn(setSIMDMode) )( fn(OPRQ) )( fn(OPRsize<23 COMMA 22>) )( fn(OPRL) )( fn(OPRM) )( fn(OPRRm) )( fn(OPRH) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_sqdmulh_advsimd_vec, 	"sqdmulh",	list_of( fn(setSIMDMode) )( fn(OPRsize<23 COMMA 22>) )( fn(OPRRm) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_sqdmulh_advsimd_vec, 	"sqdmulh",	list_of( fn(setSIMDMode) )( fn(OPRQ) )( fn(OPRsize<23 COMMA 22>) )( fn(OPRRm) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_sqdmull_advsimd_elt, 	"sqdmull",	list_of( fn(setSIMDMode) )( fn(OPRsize<23 COMMA 22>) )( fn(OPRL) )( fn(OPRM) )( fn(OPRRm) )( fn(OPRH) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_sqdmull_advsimd_elt, 	"sqdmull",	list_of( fn(setSIMDMode) )( fn(OPRQ) )( fn(OPRsize<23 COMMA 22>) )( fn(OPRL) )( fn(OPRM) )( fn(OPRRm) )( fn(OPRH) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_sqdmull_advsimd_vec, 	"sqdmull",	list_of( fn(setSIMDMode) )( fn(OPRsize<23 COMMA 22>) )( fn(OPRRm) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_sqdmull_advsimd_vec, 	"sqdmull",	list_of( fn(setSIMDMode) )( fn(OPRQ) )( fn(OPRsize<23 COMMA 22>) )( fn(OPRRm) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_sqneg_advsimd, 	"sqneg",	list_of( fn(setSIMDMode) )( fn(OPRsize<23 COMMA 22>) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_sqneg_advsimd, 	"sqneg",	list_of( fn(setSIMDMode) )( fn(OPRQ) )( fn(OPRsize<23 COMMA 22>) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_sqrdmulh_advsimd_elt, 	"sqrdmulh",	list_of( fn(setSIMDMode) )( fn(OPRsize<23 COMMA 22>) )( fn(OPRL) )( fn(OPRM) )( fn(OPRRm) )( fn(OPRH) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_sqrdmulh_advsimd_elt, 	"sqrdmulh",	list_of( fn(setSIMDMode) )( fn(OPRQ) )( fn(OPRsize<23 COMMA 22>) )( fn(OPRL) )( fn(OPRM) )( fn(OPRRm) )( fn(OPRH) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_sqrdmulh_advsimd_vec, 	"sqrdmulh",	list_of( fn(setSIMDMode) )( fn(OPRsize<23 COMMA 22>) )( fn(OPRRm) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_sqrdmulh_advsimd_vec, 	"sqrdmulh",	list_of( fn(setSIMDMode) )( fn(OPRQ) )( fn(OPRsize<23 COMMA 22>) )( fn(OPRRm) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_sqrshl_advsimd, 	"sqrshl",	list_of( fn(setSIMDMode) )( fn(OPRsize<23 COMMA 22>) )( fn(OPRRm) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_sqrshl_advsimd, 	"sqrshl",	list_of( fn(setSIMDMode) )( fn(OPRQ) )( fn(OPRsize<23 COMMA 22>) )( fn(OPRRm) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_sqrshrn_advsimd, 	"sqrshrn",	list_of( fn(setSIMDMode) )( fn(OPRimm<22 COMMA 19>) )( fn(OPRimm<18 COMMA 16>) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_sqrshrn_advsimd, 	"sqrshrn",	list_of( fn(setSIMDMode) )( fn(OPRQ) )( fn(OPRimm<22 COMMA 19>) )( fn(OPRimm<18 COMMA 16>) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_sqrshrun_advsimd, 	"sqrshrun",	list_of( fn(setSIMDMode) )( fn(OPRimm<22 COMMA 19>) )( fn(OPRimm<18 COMMA 16>) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_sqrshrun_advsimd, 	"sqrshrun",	list_of( fn(setSIMDMode) )( fn(OPRQ) )( fn(OPRimm<22 COMMA 19>) )( fn(OPRimm<18 COMMA 16>) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_sqshl_advsimd_imm, 	"sqshl",	list_of( fn(setSIMDMode) )( fn(OPRimm<22 COMMA 19>) )( fn(OPRimm<18 COMMA 16>) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_sqshl_advsimd_imm, 	"sqshl",	list_of( fn(setSIMDMode) )( fn(OPRQ) )( fn(OPRimm<22 COMMA 19>) )( fn(OPRimm<18 COMMA 16>) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_sqshl_advsimd_reg, 	"sqshl",	list_of( fn(setSIMDMode) )( fn(OPRsize<23 COMMA 22>) )( fn(OPRRm) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_sqshl_advsimd_reg, 	"sqshl",	list_of( fn(setSIMDMode) )( fn(OPRQ) )( fn(OPRsize<23 COMMA 22>) )( fn(OPRRm) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_sqshlu_advsimd, 	"sqshlu",	list_of( fn(setSIMDMode) )( fn(OPRimm<22 COMMA 19>) )( fn(OPRimm<18 COMMA 16>) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_sqshlu_advsimd, 	"sqshlu",	list_of( fn(setSIMDMode) )( fn(OPRQ) )( fn(OPRimm<22 COMMA 19>) )( fn(OPRimm<18 COMMA 16>) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_sqshrn_advsimd, 	"sqshrn",	list_of( fn(setSIMDMode) )( fn(OPRimm<22 COMMA 19>) )( fn(OPRimm<18 COMMA 16>) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_sqshrn_advsimd, 	"sqshrn",	list_of( fn(setSIMDMode) )( fn(OPRQ) )( fn(OPRimm<22 COMMA 19>) )( fn(OPRimm<18 COMMA 16>) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_sqshrun_advsimd, 	"sqshrun",	list_of( fn(setSIMDMode) )( fn(OPRimm<22 COMMA 19>) )( fn(OPRimm<18 COMMA 16>) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_sqshrun_advsimd, 	"sqshrun",	list_of( fn(setSIMDMode) )( fn(OPRQ) )( fn(OPRimm<22 COMMA 19>) )( fn(OPRimm<18 COMMA 16>) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_sqsub_advsimd, 	"sqsub",	list_of( fn(setSIMDMode) )( fn(OPRsize<23 COMMA 22>) )( fn(OPRRm) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_sqsub_advsimd, 	"sqsub",	list_of( fn(setSIMDMode) )( fn(OPRQ) )( fn(OPRsize<23 COMMA 22>) )( fn(OPRRm) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_sqxtn_advsimd, 	"sqxtn",	list_of( fn(setSIMDMode) )( fn(OPRsize<23 COMMA 22>) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_sqxtn_advsimd, 	"sqxtn",	list_of( fn(setSIMDMode) )( fn(OPRQ) )( fn(OPRsize<23 COMMA 22>) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_sqxtun_advsimd, 	"sqxtun",	list_of( fn(setSIMDMode) )( fn(OPRsize<23 COMMA 22>) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_sqxtun_advsimd, 	"sqxtun",	list_of( fn(setSIMDMode) )( fn(OPRQ) )( fn(OPRsize<23 COMMA 22>) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_srhadd_advsimd, 	"srhadd",	list_of( fn(setSIMDMode) )( fn(OPRQ) )( fn(OPRsize<23 COMMA 22>) )( fn(OPRRm) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_sri_advsimd, 	"sri",	list_of( fn(setSIMDMode) )( fn(OPRimm<22 COMMA 19>) )( fn(OPRimm<18 COMMA 16>) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_sri_advsimd, 	"sri",	list_of( fn(setSIMDMode) )( fn(OPRQ) )( fn(OPRimm<22 COMMA 19>) )( fn(OPRimm<18 COMMA 16>) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_srshl_advsimd, 	"srshl",	list_of( fn(setSIMDMode) )( fn(OPRsize<23 COMMA 22>) )( fn(OPRRm) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_srshl_advsimd, 	"srshl",	list_of( fn(setSIMDMode) )( fn(OPRQ) )( fn(OPRsize<23 COMMA 22>) )( fn(OPRRm) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_srshr_advsimd, 	"srshr",	list_of( fn(setSIMDMode) )( fn(OPRimm<22 COMMA 19>) )( fn(OPRimm<18 COMMA 16>) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_srshr_advsimd, 	"srshr",	list_of( fn(setSIMDMode) )( fn(OPRQ) )( fn(OPRimm<22 COMMA 19>) )( fn(OPRimm<18 COMMA 16>) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_srsra_advsimd, 	"srsra",	list_of( fn(setSIMDMode) )( fn(OPRimm<22 COMMA 19>) )( fn(OPRimm<18 COMMA 16>) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_srsra_advsimd, 	"srsra",	list_of( fn(setSIMDMode) )( fn(OPRQ) )( fn(OPRimm<22 COMMA 19>) )( fn(OPRimm<18 COMMA 16>) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_sshl_advsimd, 	"sshl",	list_of( fn(setSIMDMode) )( fn(OPRsize<23 COMMA 22>) )( fn(OPRRm) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_sshl_advsimd, 	"sshl",	list_of( fn(setSIMDMode) )( fn(OPRQ) )( fn(OPRsize<23 COMMA 22>) )( fn(OPRRm) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_sshll_advsimd, 	"sshll",	list_of( fn(setSIMDMode) )( fn(OPRQ) )( fn(OPRimm<22 COMMA 19>) )( fn(OPRimm<18 COMMA 16>) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_sshr_advsimd, 	"sshr",	list_of( fn(setSIMDMode) )( fn(OPRimm<22 COMMA 19>) )( fn(OPRimm<18 COMMA 16>) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_sshr_advsimd, 	"sshr",	list_of( fn(setSIMDMode) )( fn(OPRQ) )( fn(OPRimm<22 COMMA 19>) )( fn(OPRimm<18 COMMA 16>) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_ssra_advsimd, 	"ssra",	list_of( fn(setSIMDMode) )( fn(OPRimm<22 COMMA 19>) )( fn(OPRimm<18 COMMA 16>) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_ssra_advsimd, 	"ssra",	list_of( fn(setSIMDMode) )( fn(OPRQ) )( fn(OPRimm<22 COMMA 19>) )( fn(OPRimm<18 COMMA 16>) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_ssubl_advsimd, 	"ssubl",	list_of( fn(setSIMDMode) )( fn(OPRQ) )( fn(OPRsize<23 COMMA 22>) )( fn(OPRRm) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_ssubw_advsimd, 	"ssubw",	list_of( fn(setSIMDMode) )( fn(OPRQ) )( fn(OPRsize<23 COMMA 22>) )( fn(OPRRm) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_st1_advsimd_mult, 	"st1",	list_of( fn(setSIMDMode) )( fn(OPRQ) )( fn(OPRopcode) )( fn(OPRopcode) )( fn(OPRopcode) )( fn(OPRsize<11 COMMA 10>) )( fn(OPRRnS) )( fn(OPRRtS) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_st1_advsimd_mult, 	"st1",	list_of( fn(setSIMDMode) )( fn(OPRQ) )( fn(OPRRm) )( fn(OPRopcode) )( fn(OPRopcode) )( fn(OPRopcode) )( fn(OPRsize<11 COMMA 10>) )( fn(OPRRnS) )( fn(OPRRtS) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_st1_advsimd_sngl, 	"st1",	list_of( fn(setSIMDMode) )( fn(OPRQ) )( fn(OPRopcode) )( fn(OPRopcode) )( fn(OPRS<12 COMMA 12>) )( fn(OPRsize<11 COMMA 10>) )( fn(OPRRnS) )( fn(OPRRtS) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_st1_advsimd_sngl, 	"st1",	list_of( fn(setSIMDMode) )( fn(OPRQ) )( fn(OPRRm) )( fn(OPRopcode) )( fn(OPRopcode) )( fn(OPRS<12 COMMA 12>) )( fn(OPRsize<11 COMMA 10>) )( fn(OPRRnS) )( fn(OPRRtS) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_st2_advsimd_mult, 	"st2",	list_of( fn(setSIMDMode) )( fn(OPRQ) )( fn(OPRsize<11 COMMA 10>) )( fn(OPRRnS) )( fn(OPRRtS) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_st2_advsimd_mult, 	"st2",	list_of( fn(setSIMDMode) )( fn(OPRQ) )( fn(OPRRm) )( fn(OPRsize<11 COMMA 10>) )( fn(OPRRnS) )( fn(OPRRtS) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_st2_advsimd_sngl, 	"st2",	list_of( fn(setSIMDMode) )( fn(OPRQ) )( fn(OPRopcode) )( fn(OPRopcode) )( fn(OPRS<12 COMMA 12>) )( fn(OPRsize<11 COMMA 10>) )( fn(OPRRnS) )( fn(OPRRtS) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_st2_advsimd_sngl, 	"st2",	list_of( fn(setSIMDMode) )( fn(OPRQ) )( fn(OPRRm) )( fn(OPRopcode) )( fn(OPRopcode) )( fn(OPRS<12 COMMA 12>) )( fn(OPRsize<11 COMMA 10>) )( fn(OPRRnS) )( fn(OPRRtS) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_st3_advsimd_mult, 	"st3",	list_of( fn(setSIMDMode) )( fn(OPRQ) )( fn(OPRsize<11 COMMA 10>) )( fn(OPRRnS) )( fn(OPRRtS) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_st3_advsimd_mult, 	"st3",	list_of( fn(setSIMDMode) )( fn(OPRQ) )( fn(OPRRm) )( fn(OPRsize<11 COMMA 10>) )( fn(OPRRnS) )( fn(OPRRtS) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_st3_advsimd_sngl, 	"st3",	list_of( fn(setSIMDMode) )( fn(OPRQ) )( fn(OPRopcode) )( fn(OPRopcode) )( fn(OPRS<12 COMMA 12>) )( fn(OPRsize<11 COMMA 10>) )( fn(OPRRnS) )( fn(OPRRtS) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_st3_advsimd_sngl, 	"st3",	list_of( fn(setSIMDMode) )( fn(OPRQ) )( fn(OPRRm) )( fn(OPRopcode) )( fn(OPRopcode) )( fn(OPRS<12 COMMA 12>) )( fn(OPRsize<11 COMMA 10>) )( fn(OPRRnS) )( fn(OPRRtS) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_st4_advsimd_mult, 	"st4",	list_of( fn(setSIMDMode) )( fn(OPRQ) )( fn(OPRsize<11 COMMA 10>) )( fn(OPRRnS) )( fn(OPRRtS) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_st4_advsimd_mult, 	"st4",	list_of( fn(setSIMDMode) )( fn(OPRQ) )( fn(OPRRm) )( fn(OPRsize<11 COMMA 10>) )( fn(OPRRnS) )( fn(OPRRtS) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_st4_advsimd_sngl, 	"st4",	list_of( fn(setSIMDMode) )( fn(OPRQ) )( fn(OPRopcode) )( fn(OPRopcode) )( fn(OPRS<12 COMMA 12>) )( fn(OPRsize<11 COMMA 10>) )( fn(OPRRnS) )( fn(OPRRtS) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_st4_advsimd_sngl, 	"st4",	list_of( fn(setSIMDMode) )( fn(OPRQ) )( fn(OPRRm) )( fn(OPRopcode) )( fn(OPRopcode) )( fn(OPRS<12 COMMA 12>) )( fn(OPRsize<11 COMMA 10>) )( fn(OPRRnS) )( fn(OPRRtS) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_stlr, 	"stlr",	list_of( fn(setRegWidth) )( fn(OPRsize<31 COMMA 30>) )( fn(OPRRnS) )( fn(OPRRtS) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_stlrb, 	"stlrb",	list_of( fn(setRegWidth) )( fn(OPRRnS) )( fn(OPRRtS) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_stlrh, 	"stlrh",	list_of( fn(setRegWidth) )( fn(OPRRnS) )( fn(OPRRtS) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_stlxp, 	"stlxp",	list_of( fn(setRegWidth) )( fn(OPRsz<30 COMMA 30>) )( fn(OPRRs) )( fn(OPRRt2S) )( fn(OPRRnS) )( fn(OPRRtS) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_stlxr, 	"stlxr",	list_of( fn(setRegWidth) )( fn(OPRsize<31 COMMA 30>) )( fn(OPRRs) )( fn(OPRRnS) )( fn(OPRRtS) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_stlxrb, 	"stlxrb",	list_of( fn(setRegWidth) )( fn(OPRRs) )( fn(OPRRnS) )( fn(OPRRtS) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_stlxrh, 	"stlxrh",	list_of( fn(setRegWidth) )( fn(OPRRs) )( fn(OPRRnS) )( fn(OPRRtS) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_stnp_fpsimd, 	"stnp",	list_of( fn(setRegWidth) )( fn(setSIMDMode) )( fn(OPRopc) )( fn(OPRimm<21 COMMA 15>) )( fn(OPRRt2S) )( fn(OPRRnS) )( fn(OPRRtS) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_stnp_gen, 	"stnp",	list_of( fn(setRegWidth) )( fn(OPRopc) )( fn(OPRimm<21 COMMA 15>) )( fn(OPRRt2S) )( fn(OPRRnS) )( fn(OPRRtS) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_stp_fpsimd, 	"stp",	list_of( fn(setRegWidth) )( fn(setSIMDMode) )( fn(OPRopc) )( fn(OPRimm<21 COMMA 15>) )( fn(OPRRt2S) )( fn(OPRRnSU) )( fn(OPRRtS) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_stp_fpsimd, 	"stp",	list_of( fn(setRegWidth) )( fn(setSIMDMode) )( fn(OPRopc) )( fn(OPRimm<21 COMMA 15>) )( fn(OPRRt2S) )( fn(OPRRnSU) )( fn(OPRRtS) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_stp_fpsimd, 	"stp",	list_of( fn(setRegWidth) )( fn(setSIMDMode) )( fn(OPRopc) )( fn(OPRimm<21 COMMA 15>) )( fn(OPRRt2S) )( fn(OPRRnS) )( fn(OPRRtS) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_stp_gen, 	"stp",	list_of( fn(setRegWidth) )( fn(OPRopc) )( fn(OPRimm<21 COMMA 15>) )( fn(OPRRt2S) )( fn(OPRRnSU) )( fn(OPRRtS) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_stp_gen, 	"stp",	list_of( fn(setRegWidth) )( fn(OPRopc) )( fn(OPRimm<21 COMMA 15>) )( fn(OPRRt2S) )( fn(OPRRnSU) )( fn(OPRRtS) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_stp_gen, 	"stp",	list_of( fn(setRegWidth) )( fn(OPRopc) )( fn(OPRimm<21 COMMA 15>) )( fn(OPRRt2S) )( fn(OPRRnS) )( fn(OPRRtS) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_str_imm_fpsimd, 	"str",	list_of( fn(setRegWidth) )( fn(setSIMDMode) )( fn(OPRsize<31 COMMA 30>) )( fn(OPRopc) )( fn(OPRimm<20 COMMA 12>) )( fn(OPRRnSU) )( fn(OPRRtS) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_str_imm_fpsimd, 	"str",	list_of( fn(setRegWidth) )( fn(setSIMDMode) )( fn(OPRsize<31 COMMA 30>) )( fn(OPRopc) )( fn(OPRimm<20 COMMA 12>) )( fn(OPRRnSU) )( fn(OPRRtS) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_str_imm_fpsimd, 	"str",	list_of( fn(setRegWidth) )( fn(setSIMDMode) )( fn(OPRsize<31 COMMA 30>) )( fn(OPRopc) )( fn(OPRimm<21 COMMA 10>) )( fn(OPRRnS) )( fn(OPRRtS) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_str_imm_gen, 	"str",	list_of( fn(setRegWidth) )( fn(OPRsize<31 COMMA 30>) )( fn(OPRimm<20 COMMA 12>) )( fn(OPRRnSU) )( fn(OPRRtS) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_str_imm_gen, 	"str",	list_of( fn(setRegWidth) )( fn(OPRsize<31 COMMA 30>) )( fn(OPRimm<20 COMMA 12>) )( fn(OPRRnSU) )( fn(OPRRtS) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_str_imm_gen, 	"str",	list_of( fn(setRegWidth) )( fn(OPRsize<31 COMMA 30>) )( fn(OPRimm<21 COMMA 10>) )( fn(OPRRnS) )( fn(OPRRtS) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_str_reg_fpsimd, 	"str",	list_of( fn(setRegWidth) )( fn(setSIMDMode) )( fn(OPRsize<31 COMMA 30>) )( fn(OPRopc) )( fn(OPRRm) )( fn(OPRoption<15 COMMA 13>) )( fn(OPRS<12 COMMA 12>) )( fn(OPRRnS) )( fn(OPRRtS) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_str_reg_gen, 	"str",	list_of( fn(setRegWidth) )( fn(OPRsize<31 COMMA 30>) )( fn(OPRRm) )( fn(OPRoption<15 COMMA 13>) )( fn(OPRS<12 COMMA 12>) )( fn(OPRRnS) )( fn(OPRRtS) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_strb_imm, 	"strb",	list_of( fn(setRegWidth) )( fn(OPRimm<20 COMMA 12>) )( fn(OPRRnSU) )( fn(OPRRtS) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_strb_imm, 	"strb",	list_of( fn(setRegWidth) )( fn(OPRimm<20 COMMA 12>) )( fn(OPRRnSU) )( fn(OPRRtS) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_strb_imm, 	"strb",	list_of( fn(setRegWidth) )( fn(OPRimm<21 COMMA 10>) )( fn(OPRRnS) )( fn(OPRRtS) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_strb_reg, 	"strb",	list_of( fn(setRegWidth) )( fn(OPRRm) )( fn(OPRoption<15 COMMA 13>) )( fn(OPRS<12 COMMA 12>) )( fn(OPRRnS) )( fn(OPRRtS) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_strh_imm, 	"strh",	list_of( fn(setRegWidth) )( fn(OPRimm<20 COMMA 12>) )( fn(OPRRnSU) )( fn(OPRRtS) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_strh_imm, 	"strh",	list_of( fn(setRegWidth) )( fn(OPRimm<20 COMMA 12>) )( fn(OPRRnSU) )( fn(OPRRtS) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_strh_imm, 	"strh",	list_of( fn(setRegWidth) )( fn(OPRimm<21 COMMA 10>) )( fn(OPRRnS) )( fn(OPRRtS) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_strh_reg, 	"strh",	list_of( fn(setRegWidth) )( fn(OPRRm) )( fn(OPRoption<15 COMMA 13>) )( fn(OPRS<12 COMMA 12>) )( fn(OPRRnS) )( fn(OPRRtS) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_sttr, 	"sttr",	list_of( fn(setRegWidth) )( fn(OPRsize<31 COMMA 30>) )( fn(OPRimm<20 COMMA 12>) )( fn(OPRRnS) )( fn(OPRRtS) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_sttrb, 	"sttrb",	list_of( fn(setRegWidth) )( fn(OPRimm<20 COMMA 12>) )( fn(OPRRnS) )( fn(OPRRtS) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_sttrh, 	"sttrh",	list_of( fn(setRegWidth) )( fn(OPRimm<20 COMMA 12>) )( fn(OPRRnS) )( fn(OPRRtS) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_stur_fpsimd, 	"stur",	list_of( fn(setRegWidth) )( fn(setSIMDMode) )( fn(OPRsize<31 COMMA 30>) )( fn(OPRopc) )( fn(OPRimm<20 COMMA 12>) )( fn(OPRRnS) )( fn(OPRRtS) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_stur_gen, 	"stur",	list_of( fn(setRegWidth) )( fn(OPRsize<31 COMMA 30>) )( fn(OPRimm<20 COMMA 12>) )( fn(OPRRnS) )( fn(OPRRtS) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_sturb, 	"sturb",	list_of( fn(setRegWidth) )( fn(OPRimm<20 COMMA 12>) )( fn(OPRRnS) )( fn(OPRRtS) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_sturh, 	"sturh",	list_of( fn(setRegWidth) )( fn(OPRimm<20 COMMA 12>) )( fn(OPRRnS) )( fn(OPRRtS) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_stxp, 	"stxp",	list_of( fn(setRegWidth) )( fn(OPRsz<30 COMMA 30>) )( fn(OPRRs) )( fn(OPRRt2S) )( fn(OPRRnS) )( fn(OPRRtS) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_stxr, 	"stxr",	list_of( fn(setRegWidth) )( fn(OPRsize<31 COMMA 30>) )( fn(OPRRs) )( fn(OPRRnS) )( fn(OPRRtS) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_stxrb, 	"stxrb",	list_of( fn(setRegWidth) )( fn(OPRRs) )( fn(OPRRnS) )( fn(OPRRtS) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_stxrh, 	"stxrh",	list_of( fn(setRegWidth) )( fn(OPRRs) )( fn(OPRRnS) )( fn(OPRRtS) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_sub_addsub_ext, 	"sub",	list_of( fn(OPRsf) )( fn(OPRRm) )( fn(OPRoption<15 COMMA 13>) )( fn(OPRimm<12 COMMA 10>) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_sub_addsub_imm, 	"sub",	list_of( fn(OPRsf) )( fn(OPRshift) )( fn(OPRimm<21 COMMA 10>) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_sub_addsub_shift, 	"sub",	list_of( fn(OPRsf) )( fn(OPRshift) )( fn(OPRRm) )( fn(OPRimm<15 COMMA 10>) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_sub_advsimd, 	"sub",	list_of( fn(setSIMDMode) )( fn(OPRsize<23 COMMA 22>) )( fn(OPRRm) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_sub_advsimd, 	"sub",	list_of( fn(setSIMDMode) )( fn(OPRQ) )( fn(OPRsize<23 COMMA 22>) )( fn(OPRRm) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_subhn_advsimd, 	"subhn",	list_of( fn(setSIMDMode) )( fn(OPRQ) )( fn(OPRsize<23 COMMA 22>) )( fn(OPRRm) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_subs_addsub_ext, 	"subs",	list_of( fn(setFlags) )( fn(OPRsf) )( fn(OPRRm) )( fn(OPRoption<15 COMMA 13>) )( fn(OPRimm<12 COMMA 10>) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_subs_addsub_imm, 	"subs",	list_of( fn(setFlags) )( fn(OPRsf) )( fn(OPRshift) )( fn(OPRimm<21 COMMA 10>) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_subs_addsub_shift, 	"subs",	list_of( fn(setFlags) )( fn(OPRsf) )( fn(OPRshift) )( fn(OPRRm) )( fn(OPRimm<15 COMMA 10>) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_suqadd_advsimd, 	"suqadd",	list_of( fn(setSIMDMode) )( fn(OPRsize<23 COMMA 22>) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_suqadd_advsimd, 	"suqadd",	list_of( fn(setSIMDMode) )( fn(OPRQ) )( fn(OPRsize<23 COMMA 22>) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_svc, 	"svc",	list_of( (operandFactory) fn(OPRimm<20 COMMA 5>) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_sxtb_sbfm, 	"sxtb",	list_of( fn(OPRsf) )( fn(OPRN<22 COMMA 22>) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_sxth_sbfm, 	"sxth",	list_of( fn(OPRsf) )( fn(OPRN<22 COMMA 22>) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_sxtl_sshll_advsimd, 	"sxtl",	list_of( fn(setSIMDMode) )( fn(OPRQ) )( fn(OPRimm<22 COMMA 19>) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_sxtw_sbfm, 	"sxtw",	list_of( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_sys, 	"sys",	list_of( fn(OPRop1) )( fn(OPRCRn) )( fn(OPRCRm) )( fn(OPRop2) )( fn(OPRRt) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_sysl, 	"sysl",	list_of( fn(OPRop1) )( fn(OPRCRn) )( fn(OPRCRm) )( fn(OPRop2) )( fn(OPRRt) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_tbl_advsimd, 	"tbl",	list_of( fn(setSIMDMode) )( fn(OPRQ) )( fn(OPRRm) )( fn(OPRlen) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_tbnz, 	"tbnz",	list_of( fn(OPRb5) )( fn(OPRb40) )( fn(OPRimm<18 COMMA 5>) )( fn(OPRRt) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_tbx_advsimd, 	"tbx",	list_of( fn(setSIMDMode) )( fn(OPRQ) )( fn(OPRRm) )( fn(OPRlen) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_tbz, 	"tbz",	list_of( fn(OPRb5) )( fn(OPRb40) )( fn(OPRimm<18 COMMA 5>) )( fn(OPRRt) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_tlbi_sys, 	"tlbi",	list_of( fn(OPRop1) )( fn(OPRCRm) )( fn(OPRop2) )( fn(OPRRt) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_trn1_advsimd, 	"trn1",	list_of( fn(setSIMDMode) )( fn(OPRQ) )( fn(OPRsize<23 COMMA 22>) )( fn(OPRRm) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_trn2_advsimd, 	"trn2",	list_of( fn(setSIMDMode) )( fn(OPRQ) )( fn(OPRsize<23 COMMA 22>) )( fn(OPRRm) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_tst_ands_log_imm, 	"tst",	list_of( fn(setFlags) )( fn(OPRsf) )( fn(OPRN<22 COMMA 22>) )( fn(OPRimm<21 COMMA 16>) )( fn(OPRimm<15 COMMA 10>) )( fn(OPRRn) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_tst_ands_log_shift, 	"tst",	list_of( fn(setFlags) )( fn(OPRsf) )( fn(OPRshift) )( fn(OPRRm) )( fn(OPRimm<15 COMMA 10>) )( fn(OPRRn) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_uaba_advsimd, 	"uaba",	list_of( fn(setSIMDMode) )( fn(OPRQ) )( fn(OPRsize<23 COMMA 22>) )( fn(OPRRm) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_uabal_advsimd, 	"uabal",	list_of( fn(setSIMDMode) )( fn(OPRQ) )( fn(OPRsize<23 COMMA 22>) )( fn(OPRRm) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_uabd_advsimd, 	"uabd",	list_of( fn(setSIMDMode) )( fn(OPRQ) )( fn(OPRsize<23 COMMA 22>) )( fn(OPRRm) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_uabdl_advsimd, 	"uabdl",	list_of( fn(setSIMDMode) )( fn(OPRQ) )( fn(OPRsize<23 COMMA 22>) )( fn(OPRRm) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_uadalp_advsimd, 	"uadalp",	list_of( fn(setSIMDMode) )( fn(OPRQ) )( fn(OPRsize<23 COMMA 22>) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_uaddl_advsimd, 	"uaddl",	list_of( fn(setSIMDMode) )( fn(OPRQ) )( fn(OPRsize<23 COMMA 22>) )( fn(OPRRm) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_uaddlp_advsimd, 	"uaddlp",	list_of( fn(setSIMDMode) )( fn(OPRQ) )( fn(OPRsize<23 COMMA 22>) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_uaddlv_advsimd, 	"uaddlv",	list_of( fn(setSIMDMode) )( fn(OPRQ) )( fn(OPRsize<23 COMMA 22>) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_uaddw_advsimd, 	"uaddw",	list_of( fn(setSIMDMode) )( fn(OPRQ) )( fn(OPRsize<23 COMMA 22>) )( fn(OPRRm) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_ubfiz_ubfm, 	"ubfiz",	list_of( fn(OPRsf) )( fn(OPRN<22 COMMA 22>) )( fn(OPRimm<21 COMMA 16>) )( fn(OPRimm<15 COMMA 10>) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_ubfm, 	"ubfm",	list_of( fn(OPRsf) )( fn(OPRN<22 COMMA 22>) )( fn(OPRimm<21 COMMA 16>) )( fn(OPRimm<15 COMMA 10>) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_ubfx_ubfm, 	"ubfx",	list_of( fn(OPRsf) )( fn(OPRN<22 COMMA 22>) )( fn(OPRimm<21 COMMA 16>) )( fn(OPRimm<15 COMMA 10>) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_ucvtf_advsimd_fix, 	"ucvtf",	list_of( fn(setSIMDMode) )( fn(OPRimm<22 COMMA 19>) )( fn(OPRimm<18 COMMA 16>) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_ucvtf_advsimd_fix, 	"ucvtf",	list_of( fn(setSIMDMode) )( fn(OPRQ) )( fn(OPRimm<22 COMMA 19>) )( fn(OPRimm<18 COMMA 16>) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_ucvtf_advsimd_int, 	"ucvtf",	list_of( fn(setSIMDMode) )( fn(OPRsz<22 COMMA 22>) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_ucvtf_advsimd_int, 	"ucvtf",	list_of( fn(setSIMDMode) )( fn(OPRQ) )( fn(OPRsz<22 COMMA 22>) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_ucvtf_float_fix, 	"ucvtf",	list_of( fn(setFPMode) )( fn(OPRsf) )( fn(OPRtype<23 COMMA 22>) )( fn(OPRscale) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_ucvtf_float_int, 	"ucvtf",	list_of( fn(setFPMode) )( fn(OPRsf) )( fn(OPRtype<23 COMMA 22>) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_udiv, 	"udiv",	list_of( fn(OPRsf) )( fn(OPRRm) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_uhadd_advsimd, 	"uhadd",	list_of( fn(setSIMDMode) )( fn(OPRQ) )( fn(OPRsize<23 COMMA 22>) )( fn(OPRRm) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_uhsub_advsimd, 	"uhsub",	list_of( fn(setSIMDMode) )( fn(OPRQ) )( fn(OPRsize<23 COMMA 22>) )( fn(OPRRm) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_umaddl, 	"umaddl",	list_of( fn(OPRRm) )( fn(OPRRa) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_umax_advsimd, 	"umax",	list_of( fn(setSIMDMode) )( fn(OPRQ) )( fn(OPRsize<23 COMMA 22>) )( fn(OPRRm) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_umaxp_advsimd, 	"umaxp",	list_of( fn(setSIMDMode) )( fn(OPRQ) )( fn(OPRsize<23 COMMA 22>) )( fn(OPRRm) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_umaxv_advsimd, 	"umaxv",	list_of( fn(setSIMDMode) )( fn(OPRQ) )( fn(OPRsize<23 COMMA 22>) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_umin_advsimd, 	"umin",	list_of( fn(setSIMDMode) )( fn(OPRQ) )( fn(OPRsize<23 COMMA 22>) )( fn(OPRRm) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_uminp_advsimd, 	"uminp",	list_of( fn(setSIMDMode) )( fn(OPRQ) )( fn(OPRsize<23 COMMA 22>) )( fn(OPRRm) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_uminv_advsimd, 	"uminv",	list_of( fn(setSIMDMode) )( fn(OPRQ) )( fn(OPRsize<23 COMMA 22>) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_umlal_advsimd_elt, 	"umlal",	list_of( fn(setSIMDMode) )( fn(OPRQ) )( fn(OPRsize<23 COMMA 22>) )( fn(OPRL) )( fn(OPRM) )( fn(OPRRm) )( fn(OPRH) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_umlal_advsimd_vec, 	"umlal",	list_of( fn(setSIMDMode) )( fn(OPRQ) )( fn(OPRsize<23 COMMA 22>) )( fn(OPRRm) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_umlsl_advsimd_elt, 	"umlsl",	list_of( fn(setSIMDMode) )( fn(OPRQ) )( fn(OPRsize<23 COMMA 22>) )( fn(OPRL) )( fn(OPRM) )( fn(OPRRm) )( fn(OPRH) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_umlsl_advsimd_vec, 	"umlsl",	list_of( fn(setSIMDMode) )( fn(OPRQ) )( fn(OPRsize<23 COMMA 22>) )( fn(OPRRm) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_umnegl_umsubl, 	"umnegl",	list_of( fn(OPRRm) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_umov_advsimd, 	"umov",	list_of( fn(setSIMDMode) )( fn(OPRQ) )( fn(OPRimm<20 COMMA 16>) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_umsubl, 	"umsubl",	list_of( fn(OPRRm) )( fn(OPRRa) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_umulh, 	"umulh",	list_of( fn(OPRRm) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_umull_advsimd_elt, 	"umull",	list_of( fn(setSIMDMode) )( fn(OPRQ) )( fn(OPRsize<23 COMMA 22>) )( fn(OPRL) )( fn(OPRM) )( fn(OPRRm) )( fn(OPRH) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_umull_advsimd_vec, 	"umull",	list_of( fn(setSIMDMode) )( fn(OPRQ) )( fn(OPRsize<23 COMMA 22>) )( fn(OPRRm) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_umull_umaddl, 	"umull",	list_of( fn(OPRRm) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_uqadd_advsimd, 	"uqadd",	list_of( fn(setSIMDMode) )( fn(OPRsize<23 COMMA 22>) )( fn(OPRRm) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_uqadd_advsimd, 	"uqadd",	list_of( fn(setSIMDMode) )( fn(OPRQ) )( fn(OPRsize<23 COMMA 22>) )( fn(OPRRm) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_uqrshl_advsimd, 	"uqrshl",	list_of( fn(setSIMDMode) )( fn(OPRsize<23 COMMA 22>) )( fn(OPRRm) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_uqrshl_advsimd, 	"uqrshl",	list_of( fn(setSIMDMode) )( fn(OPRQ) )( fn(OPRsize<23 COMMA 22>) )( fn(OPRRm) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_uqrshrn_advsimd, 	"uqrshrn",	list_of( fn(setSIMDMode) )( fn(OPRimm<22 COMMA 19>) )( fn(OPRimm<18 COMMA 16>) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_uqrshrn_advsimd, 	"uqrshrn",	list_of( fn(setSIMDMode) )( fn(OPRQ) )( fn(OPRimm<22 COMMA 19>) )( fn(OPRimm<18 COMMA 16>) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_uqshl_advsimd_imm, 	"uqshl",	list_of( fn(setSIMDMode) )( fn(OPRimm<22 COMMA 19>) )( fn(OPRimm<18 COMMA 16>) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_uqshl_advsimd_imm, 	"uqshl",	list_of( fn(setSIMDMode) )( fn(OPRQ) )( fn(OPRimm<22 COMMA 19>) )( fn(OPRimm<18 COMMA 16>) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_uqshl_advsimd_reg, 	"uqshl",	list_of( fn(setSIMDMode) )( fn(OPRsize<23 COMMA 22>) )( fn(OPRRm) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_uqshl_advsimd_reg, 	"uqshl",	list_of( fn(setSIMDMode) )( fn(OPRQ) )( fn(OPRsize<23 COMMA 22>) )( fn(OPRRm) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_uqshrn_advsimd, 	"uqshrn",	list_of( fn(setSIMDMode) )( fn(OPRimm<22 COMMA 19>) )( fn(OPRimm<18 COMMA 16>) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_uqshrn_advsimd, 	"uqshrn",	list_of( fn(setSIMDMode) )( fn(OPRQ) )( fn(OPRimm<22 COMMA 19>) )( fn(OPRimm<18 COMMA 16>) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_uqsub_advsimd, 	"uqsub",	list_of( fn(setSIMDMode) )( fn(OPRsize<23 COMMA 22>) )( fn(OPRRm) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_uqsub_advsimd, 	"uqsub",	list_of( fn(setSIMDMode) )( fn(OPRQ) )( fn(OPRsize<23 COMMA 22>) )( fn(OPRRm) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_uqxtn_advsimd, 	"uqxtn",	list_of( fn(setSIMDMode) )( fn(OPRsize<23 COMMA 22>) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_uqxtn_advsimd, 	"uqxtn",	list_of( fn(setSIMDMode) )( fn(OPRQ) )( fn(OPRsize<23 COMMA 22>) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_urecpe_advsimd, 	"urecpe",	list_of( fn(setSIMDMode) )( fn(OPRQ) )( fn(OPRsz<22 COMMA 22>) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_urhadd_advsimd, 	"urhadd",	list_of( fn(setSIMDMode) )( fn(OPRQ) )( fn(OPRsize<23 COMMA 22>) )( fn(OPRRm) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_urshl_advsimd, 	"urshl",	list_of( fn(setSIMDMode) )( fn(OPRsize<23 COMMA 22>) )( fn(OPRRm) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_urshl_advsimd, 	"urshl",	list_of( fn(setSIMDMode) )( fn(OPRQ) )( fn(OPRsize<23 COMMA 22>) )( fn(OPRRm) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_urshr_advsimd, 	"urshr",	list_of( fn(setSIMDMode) )( fn(OPRimm<22 COMMA 19>) )( fn(OPRimm<18 COMMA 16>) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_urshr_advsimd, 	"urshr",	list_of( fn(setSIMDMode) )( fn(OPRQ) )( fn(OPRimm<22 COMMA 19>) )( fn(OPRimm<18 COMMA 16>) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_ursqrte_advsimd, 	"ursqrte",	list_of( fn(setSIMDMode) )( fn(OPRQ) )( fn(OPRsz<22 COMMA 22>) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_ursra_advsimd, 	"ursra",	list_of( fn(setSIMDMode) )( fn(OPRimm<22 COMMA 19>) )( fn(OPRimm<18 COMMA 16>) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_ursra_advsimd, 	"ursra",	list_of( fn(setSIMDMode) )( fn(OPRQ) )( fn(OPRimm<22 COMMA 19>) )( fn(OPRimm<18 COMMA 16>) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_ushl_advsimd, 	"ushl",	list_of( fn(setSIMDMode) )( fn(OPRsize<23 COMMA 22>) )( fn(OPRRm) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_ushl_advsimd, 	"ushl",	list_of( fn(setSIMDMode) )( fn(OPRQ) )( fn(OPRsize<23 COMMA 22>) )( fn(OPRRm) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_ushll_advsimd, 	"ushll",	list_of( fn(setSIMDMode) )( fn(OPRQ) )( fn(OPRimm<22 COMMA 19>) )( fn(OPRimm<18 COMMA 16>) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_ushr_advsimd, 	"ushr",	list_of( fn(setSIMDMode) )( fn(OPRimm<22 COMMA 19>) )( fn(OPRimm<18 COMMA 16>) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_ushr_advsimd, 	"ushr",	list_of( fn(setSIMDMode) )( fn(OPRQ) )( fn(OPRimm<22 COMMA 19>) )( fn(OPRimm<18 COMMA 16>) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_usqadd_advsimd, 	"usqadd",	list_of( fn(setSIMDMode) )( fn(OPRsize<23 COMMA 22>) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_usqadd_advsimd, 	"usqadd",	list_of( fn(setSIMDMode) )( fn(OPRQ) )( fn(OPRsize<23 COMMA 22>) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_usra_advsimd, 	"usra",	list_of( fn(setSIMDMode) )( fn(OPRimm<22 COMMA 19>) )( fn(OPRimm<18 COMMA 16>) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_usra_advsimd, 	"usra",	list_of( fn(setSIMDMode) )( fn(OPRQ) )( fn(OPRimm<22 COMMA 19>) )( fn(OPRimm<18 COMMA 16>) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_usubl_advsimd, 	"usubl",	list_of( fn(setSIMDMode) )( fn(OPRQ) )( fn(OPRsize<23 COMMA 22>) )( fn(OPRRm) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_usubw_advsimd, 	"usubw",	list_of( fn(setSIMDMode) )( fn(OPRQ) )( fn(OPRsize<23 COMMA 22>) )( fn(OPRRm) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_uxtb_ubfm, 	"uxtb",	list_of( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_uxth_ubfm, 	"uxth",	list_of( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_uxtl_ushll_advsimd, 	"uxtl",	list_of( fn(setSIMDMode) )( fn(OPRQ) )( fn(OPRimm<22 COMMA 19>) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_uzp1_advsimd, 	"uzp1",	list_of( fn(setSIMDMode) )( fn(OPRQ) )( fn(OPRsize<23 COMMA 22>) )( fn(OPRRm) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_uzp2_advsimd, 	"uzp2",	list_of( fn(setSIMDMode) )( fn(OPRQ) )( fn(OPRsize<23 COMMA 22>) )( fn(OPRRm) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_wfe_hint, 	"wfe",	operandSpec() ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_wfi_hint, 	"wfi",	operandSpec() ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_xtn_advsimd, 	"xtn",	list_of( fn(setSIMDMode) )( fn(OPRQ) )( fn(OPRsize<23 COMMA 22>) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_yield_hint, 	"yield",	operandSpec() ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_zip1_advsimd, 	"zip1",	list_of( fn(setSIMDMode) )( fn(OPRQ) )( fn(OPRsize<23 COMMA 22>) )( fn(OPRRm) )( fn(OPRRn) )( fn(OPRRd) ) ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_zip2_advsimd, 	"zip2",	list_of( fn(setSIMDMode) )( fn(OPRQ) )( fn(OPRsize<23 COMMA 22>) )( fn(OPRRm) )( fn(OPRRn) )( fn(OPRRd) ) ));
}
