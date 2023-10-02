#ifndef ROSE_AMDGPUINSTRUCTIONENUM_H
#define ROSE_AMDGPUINSTRUCTIONENUM_H
enum AMDGPURegisterClass{
	amdgpu_regclass_hwr,
	amdgpu_regclass_pc,
	amdgpu_regclass_ttmp_sgpr,
	amdgpu_regclass_sgpr,
	amdgpu_regclass_vgpr,
};
enum AMDGPUHardwareRegister{
	amdgpu_address_mode_32,
	amdgpu_exec,
	amdgpu_expcnt,
	amdgpu_export_icount,
	amdgpu_flat_scratch,
	amdgpu_gpr_alloc,
	amdgpu_ib_sts,
	amdgpu_lds_alloc,
	amdgpu_lds_gds_constant_message_count,
	amdgpu_lgkmcnt,
	amdgpu_m0,
	amdgpu_mode,
	amdgpu_pops_exiting_wave_id,
	amdgpu_private_base,
	amdgpu_private_limit,
	amdgpu_shared_base,
	amdgpu_shared_limit,
	amdgpu_status,
	amdgpu_tba,
	amdgpu_tid,
	amdgpu_tma,
	amdgpu_trap_base_address,
	amdgpu_trap_memory_address,
	amdgpu_vcc,
	amdgpu_vectory_memory_icount,
	amdgpu_vmcnt,
	amdgpu_xnack_mask,
	amdgpu_pc
};
enum AMDGPUScalarGeneralPurposeRegister{
	amdgpu_sgpr0,
	amdgpu_sgpr1,
	amdgpu_sgpr2,
	amdgpu_sgpr3,
	amdgpu_sgpr4,
	amdgpu_sgpr5,
	amdgpu_sgpr6,
	amdgpu_sgpr7,
	amdgpu_sgpr8,
	amdgpu_sgpr9,
	amdgpu_sgpr10,
	amdgpu_sgpr11,
	amdgpu_sgpr12,
	amdgpu_sgpr13,
	amdgpu_sgpr14,
	amdgpu_sgpr15,
	amdgpu_sgpr16,
	amdgpu_sgpr17,
	amdgpu_sgpr18,
	amdgpu_sgpr19,
	amdgpu_sgpr20,
	amdgpu_sgpr21,
	amdgpu_sgpr22,
	amdgpu_sgpr23,
	amdgpu_sgpr24,
	amdgpu_sgpr25,
	amdgpu_sgpr26,
	amdgpu_sgpr27,
	amdgpu_sgpr28,
	amdgpu_sgpr29,
	amdgpu_sgpr30,
	amdgpu_sgpr31,
	amdgpu_sgpr32,
	amdgpu_sgpr33,
	amdgpu_sgpr34,
	amdgpu_sgpr35,
	amdgpu_sgpr36,
	amdgpu_sgpr37,
	amdgpu_sgpr38,
	amdgpu_sgpr39,
	amdgpu_sgpr40,
	amdgpu_sgpr41,
	amdgpu_sgpr42,
	amdgpu_sgpr43,
	amdgpu_sgpr44,
	amdgpu_sgpr45,
	amdgpu_sgpr46,
	amdgpu_sgpr47,
	amdgpu_sgpr48,
	amdgpu_sgpr49,
	amdgpu_sgpr50,
	amdgpu_sgpr51,
	amdgpu_sgpr52,
	amdgpu_sgpr53,
	amdgpu_sgpr54,
	amdgpu_sgpr55,
	amdgpu_sgpr56,
	amdgpu_sgpr57,
	amdgpu_sgpr58,
	amdgpu_sgpr59,
	amdgpu_sgpr60,
	amdgpu_sgpr61,
	amdgpu_sgpr62,
	amdgpu_sgpr63,
	amdgpu_sgpr64,
	amdgpu_sgpr65,
	amdgpu_sgpr66,
	amdgpu_sgpr67,
	amdgpu_sgpr68,
	amdgpu_sgpr69,
	amdgpu_sgpr70,
	amdgpu_sgpr71,
	amdgpu_sgpr72,
	amdgpu_sgpr73,
	amdgpu_sgpr74,
	amdgpu_sgpr75,
	amdgpu_sgpr76,
	amdgpu_sgpr77,
	amdgpu_sgpr78,
	amdgpu_sgpr79,
	amdgpu_sgpr80,
	amdgpu_sgpr81,
	amdgpu_sgpr82,
	amdgpu_sgpr83,
	amdgpu_sgpr84,
	amdgpu_sgpr85,
	amdgpu_sgpr86,
	amdgpu_sgpr87,
	amdgpu_sgpr88,
	amdgpu_sgpr89,
	amdgpu_sgpr90,
	amdgpu_sgpr91,
	amdgpu_sgpr92,
	amdgpu_sgpr93,
	amdgpu_sgpr94,
	amdgpu_sgpr95,
	amdgpu_sgpr96,
	amdgpu_sgpr97,
	amdgpu_sgpr98,
	amdgpu_sgpr99,
	amdgpu_sgpr100,
	amdgpu_sgpr101,
	amdgpu_sgpr102,
	amdgpu_sgpr103
};
enum AMDGPUVectorGeneralPurposeRegister{
	amdgpu_vgpr0,
	amdgpu_vgpr1,
	amdgpu_vgpr2,
	amdgpu_vgpr3,
	amdgpu_vgpr4,
	amdgpu_vgpr5,
	amdgpu_vgpr6,
	amdgpu_vgpr7,
	amdgpu_vgpr8,
	amdgpu_vgpr9,
	amdgpu_vgpr10,
	amdgpu_vgpr11,
	amdgpu_vgpr12,
	amdgpu_vgpr13,
	amdgpu_vgpr14,
	amdgpu_vgpr15,
	amdgpu_vgpr16,
	amdgpu_vgpr17,
	amdgpu_vgpr18,
	amdgpu_vgpr19,
	amdgpu_vgpr20,
	amdgpu_vgpr21,
	amdgpu_vgpr22,
	amdgpu_vgpr23,
	amdgpu_vgpr24,
	amdgpu_vgpr25,
	amdgpu_vgpr26,
	amdgpu_vgpr27,
	amdgpu_vgpr28,
	amdgpu_vgpr29,
	amdgpu_vgpr30,
	amdgpu_vgpr31,
	amdgpu_vgpr32,
	amdgpu_vgpr33,
	amdgpu_vgpr34,
	amdgpu_vgpr35,
	amdgpu_vgpr36,
	amdgpu_vgpr37,
	amdgpu_vgpr38,
	amdgpu_vgpr39,
	amdgpu_vgpr40,
	amdgpu_vgpr41,
	amdgpu_vgpr42,
	amdgpu_vgpr43,
	amdgpu_vgpr44,
	amdgpu_vgpr45,
	amdgpu_vgpr46,
	amdgpu_vgpr47,
	amdgpu_vgpr48,
	amdgpu_vgpr49,
	amdgpu_vgpr50,
	amdgpu_vgpr51,
	amdgpu_vgpr52,
	amdgpu_vgpr53,
	amdgpu_vgpr54,
	amdgpu_vgpr55,
	amdgpu_vgpr56,
	amdgpu_vgpr57,
	amdgpu_vgpr58,
	amdgpu_vgpr59,
	amdgpu_vgpr60,
	amdgpu_vgpr61,
	amdgpu_vgpr62,
	amdgpu_vgpr63,
	amdgpu_vgpr64,
	amdgpu_vgpr65,
	amdgpu_vgpr66,
	amdgpu_vgpr67,
	amdgpu_vgpr68,
	amdgpu_vgpr69,
	amdgpu_vgpr70,
	amdgpu_vgpr71,
	amdgpu_vgpr72,
	amdgpu_vgpr73,
	amdgpu_vgpr74,
	amdgpu_vgpr75,
	amdgpu_vgpr76,
	amdgpu_vgpr77,
	amdgpu_vgpr78,
	amdgpu_vgpr79,
	amdgpu_vgpr80,
	amdgpu_vgpr81,
	amdgpu_vgpr82,
	amdgpu_vgpr83,
	amdgpu_vgpr84,
	amdgpu_vgpr85,
	amdgpu_vgpr86,
	amdgpu_vgpr87,
	amdgpu_vgpr88,
	amdgpu_vgpr89,
	amdgpu_vgpr90,
	amdgpu_vgpr91,
	amdgpu_vgpr92,
	amdgpu_vgpr93,
	amdgpu_vgpr94,
	amdgpu_vgpr95,
	amdgpu_vgpr96,
	amdgpu_vgpr97,
	amdgpu_vgpr98,
	amdgpu_vgpr99,
	amdgpu_vgpr100,
	amdgpu_vgpr101,
	amdgpu_vgpr102,
	amdgpu_vgpr103,
	amdgpu_vgpr104,
	amdgpu_vgpr105,
	amdgpu_vgpr106,
	amdgpu_vgpr107,
	amdgpu_vgpr108,
	amdgpu_vgpr109,
	amdgpu_vgpr110,
	amdgpu_vgpr111,
	amdgpu_vgpr112,
	amdgpu_vgpr113,
	amdgpu_vgpr114,
	amdgpu_vgpr115,
	amdgpu_vgpr116,
	amdgpu_vgpr117,
	amdgpu_vgpr118,
	amdgpu_vgpr119,
	amdgpu_vgpr120,
	amdgpu_vgpr121,
	amdgpu_vgpr122,
	amdgpu_vgpr123,
	amdgpu_vgpr124,
	amdgpu_vgpr125,
	amdgpu_vgpr126,
	amdgpu_vgpr127,
	amdgpu_vgpr128,
	amdgpu_vgpr129,
	amdgpu_vgpr130,
	amdgpu_vgpr131,
	amdgpu_vgpr132,
	amdgpu_vgpr133,
	amdgpu_vgpr134,
	amdgpu_vgpr135,
	amdgpu_vgpr136,
	amdgpu_vgpr137,
	amdgpu_vgpr138,
	amdgpu_vgpr139,
	amdgpu_vgpr140,
	amdgpu_vgpr141,
	amdgpu_vgpr142,
	amdgpu_vgpr143,
	amdgpu_vgpr144,
	amdgpu_vgpr145,
	amdgpu_vgpr146,
	amdgpu_vgpr147,
	amdgpu_vgpr148,
	amdgpu_vgpr149,
	amdgpu_vgpr150,
	amdgpu_vgpr151,
	amdgpu_vgpr152,
	amdgpu_vgpr153,
	amdgpu_vgpr154,
	amdgpu_vgpr155,
	amdgpu_vgpr156,
	amdgpu_vgpr157,
	amdgpu_vgpr158,
	amdgpu_vgpr159,
	amdgpu_vgpr160,
	amdgpu_vgpr161,
	amdgpu_vgpr162,
	amdgpu_vgpr163,
	amdgpu_vgpr164,
	amdgpu_vgpr165,
	amdgpu_vgpr166,
	amdgpu_vgpr167,
	amdgpu_vgpr168,
	amdgpu_vgpr169,
	amdgpu_vgpr170,
	amdgpu_vgpr171,
	amdgpu_vgpr172,
	amdgpu_vgpr173,
	amdgpu_vgpr174,
	amdgpu_vgpr175,
	amdgpu_vgpr176,
	amdgpu_vgpr177,
	amdgpu_vgpr178,
	amdgpu_vgpr179,
	amdgpu_vgpr180,
	amdgpu_vgpr181,
	amdgpu_vgpr182,
	amdgpu_vgpr183,
	amdgpu_vgpr184,
	amdgpu_vgpr185,
	amdgpu_vgpr186,
	amdgpu_vgpr187,
	amdgpu_vgpr188,
	amdgpu_vgpr189,
	amdgpu_vgpr190,
	amdgpu_vgpr191,
	amdgpu_vgpr192,
	amdgpu_vgpr193,
	amdgpu_vgpr194,
	amdgpu_vgpr195,
	amdgpu_vgpr196,
	amdgpu_vgpr197,
	amdgpu_vgpr198,
	amdgpu_vgpr199,
	amdgpu_vgpr200,
	amdgpu_vgpr201,
	amdgpu_vgpr202,
	amdgpu_vgpr203,
	amdgpu_vgpr204,
	amdgpu_vgpr205,
	amdgpu_vgpr206,
	amdgpu_vgpr207,
	amdgpu_vgpr208,
	amdgpu_vgpr209,
	amdgpu_vgpr210,
	amdgpu_vgpr211,
	amdgpu_vgpr212,
	amdgpu_vgpr213,
	amdgpu_vgpr214,
	amdgpu_vgpr215,
	amdgpu_vgpr216,
	amdgpu_vgpr217,
	amdgpu_vgpr218,
	amdgpu_vgpr219,
	amdgpu_vgpr220,
	amdgpu_vgpr221,
	amdgpu_vgpr222,
	amdgpu_vgpr223,
	amdgpu_vgpr224,
	amdgpu_vgpr225,
	amdgpu_vgpr226,
	amdgpu_vgpr227,
	amdgpu_vgpr228,
	amdgpu_vgpr229,
	amdgpu_vgpr230,
	amdgpu_vgpr231,
	amdgpu_vgpr232,
	amdgpu_vgpr233,
	amdgpu_vgpr234,
	amdgpu_vgpr235,
	amdgpu_vgpr236,
	amdgpu_vgpr237,
	amdgpu_vgpr238,
	amdgpu_vgpr239,
	amdgpu_vgpr240,
	amdgpu_vgpr241,
	amdgpu_vgpr242,
	amdgpu_vgpr243,
	amdgpu_vgpr244,
	amdgpu_vgpr245,
	amdgpu_vgpr246,
	amdgpu_vgpr247,
	amdgpu_vgpr248,
	amdgpu_vgpr249,
	amdgpu_vgpr250,
	amdgpu_vgpr251,
	amdgpu_vgpr252,
	amdgpu_vgpr253,
	amdgpu_vgpr254,
	amdgpu_vgpr255
};
enum AMDGPUInstructionKind{
	rose_amdgpu_op_INVALID		=0,
	rose_amdgpu_op_s_cmp_ge_eq_i32,
	rose_amdgpu_op_s_cmp_ge_eq_u32,
	rose_amdgpu_op_s_cmp_lg_i32,
	rose_amdgpu_op_s_cmp_lg_u32,
	rose_amdgpu_op_s_cmp_gt_i32,
	rose_amdgpu_op_s_cmp_gt_u32,
	rose_amdgpu_op_s_cmp_ge_i32,
	rose_amdgpu_op_s_cmp_ge_u32,
	rose_amdgpu_op_s_cmp_lt_i32,
	rose_amdgpu_op_s_cmp_lt_u32,
	rose_amdgpu_op_s_cmp_le_i32,
	rose_amdgpu_op_s_cmp_le_u32,
	rose_amdgpu_op_s_bitcmp0_b32,
	rose_amdgpu_op_s_bitcmp1_b32,
	rose_amdgpu_op_s_bitcmp0_b64,
	rose_amdgpu_op_s_bitcmp1_b64,
	rose_amdgpu_op_s_setvkip,
	rose_amdgpu_op_s_set_gpr_idx_on,
	rose_amdgpu_op_s_cmp_eq_u64,
	rose_amdgpu_op_s_cmp_lg_u64,
	rose_amdgpu_op_s_nop,
	rose_amdgpu_op_s_endpgm,
	rose_amdgpu_op_s_branch,
	rose_amdgpu_op_s_wakeup,
	rose_amdgpu_op_s_cbranch_scc0,
	rose_amdgpu_op_s_cbranch_scc1,
	rose_amdgpu_op_s_cbranch_vccz,
	rose_amdgpu_op_s_cbranch_vccnz,
	rose_amdgpu_op_s_cbranch_execz,
	rose_amdgpu_op_s_cbranch_execnz,
	rose_amdgpu_op_s_barrier,
	rose_amdgpu_op_s_setkill,
	rose_amdgpu_op_s_waitcnt,
	rose_amdgpu_op_s_sethalt,
	rose_amdgpu_op_s_sleep,
	rose_amdgpu_op_s_setprio,
	rose_amdgpu_op_s_sendmsg,
	rose_amdgpu_op_s_sendmsghalt,
	rose_amdgpu_op_s_trap,
	rose_amdgpu_op_s_icache_inv,
	rose_amdgpu_op_s_incperflevel,
	rose_amdgpu_op_s_decperflevel,
	rose_amdgpu_op_s_ttracedata,
	rose_amdgpu_op_s_cbranch_cdbgsys,
	rose_amdgpu_op_s_cbranch_cdbguser,
	rose_amdgpu_op_s_cbranch_cdbgsys_and_user,
	rose_amdgpu_op_s_endpgm_saved,
	rose_amdgpu_op_s_set_gpr_idx_off,
	rose_amdgpu_op_s_set_gpr_idx_mode,
	rose_amdgpu_op_s_endpgm_ordered_ps_done,
	rose_amdgpu_op_buffer_load_format_x,
	rose_amdgpu_op_buffer_load_format_xy,
	rose_amdgpu_op_buffer_load_format_xyz,
	rose_amdgpu_op_buffer_load_format_xyzw,
	rose_amdgpu_op_buffer_store_format_x,
	rose_amdgpu_op_buffer_store_format_xy,
	rose_amdgpu_op_buffer_store_format_xyz,
	rose_amdgpu_op_buffer_store_format_xyzw,
	rose_amdgpu_op_buffer_load_dword,
	rose_amdgpu_op_buffer_load_dwordx2,
	rose_amdgpu_op_buffer_load_dwordx3,
	rose_amdgpu_op_buffer_load_dwordx4,
	rose_amdgpu_op_tbuffer_load_format_x,
	rose_amdgpu_op_tbuffer_load_format_xy,
	rose_amdgpu_op_tbuffer_load_format_xyz,
	rose_amdgpu_op_tbuffer_load_format_xyzw,
	rose_amdgpu_op_tbuffer_store_format_x,
	rose_amdgpu_op_tbuffer_store_format_xy,
	rose_amdgpu_op_tbuffer_store_format_xyz,
	rose_amdgpu_op_tbuffer_store_format_xyzw,
	rose_amdgpu_op_v_cndmask_b32,
	rose_amdgpu_op_v_add_f32,
	rose_amdgpu_op_v_sub_f32,
	rose_amdgpu_op_v_subrev_f32,
	rose_amdgpu_op_v_mul_legacy_f32,
	rose_amdgpu_op_v_mul_f32,
	rose_amdgpu_op_v_mul_i32_i24,
	rose_amdgpu_op_v_mul_hi_i32_i24,
	rose_amdgpu_op_v_mul_u32_u24,
	rose_amdgpu_op_v_mul_hi_u32_u24,
	rose_amdgpu_op_v_min_f32,
	rose_amdgpu_op_v_max_f32,
	rose_amdgpu_op_v_min_i32,
	rose_amdgpu_op_v_max_i32,
	rose_amdgpu_op_v_min_u32,
	rose_amdgpu_op_v_max_u32,
	rose_amdgpu_op_v_lshrrev_b32,
	rose_amdgpu_op_v_ashrrev_i32,
	rose_amdgpu_op_v_lshlrev_b32,
	rose_amdgpu_op_v_add_b32,
	rose_amdgpu_op_v_or_b32,
	rose_amdgpu_op_v_xor_b32,
	rose_amdgpu_op_v_mac_b32,
	rose_amdgpu_op_v_madmk_f32,
	rose_amdgpu_op_v_madak_f32,
	rose_amdgpu_op_v_add_co_u32,
	rose_amdgpu_op_v_sub_co_u32,
	rose_amdgpu_op_v_subrev_co_u32,
	rose_amdgpu_op_v_addc_co_u32,
	rose_amdgpu_op_v_subb_co_u32,
	rose_amdgpu_op_v_subbrev_co_u32,
	rose_amdgpu_op_v_add_f16,
	rose_amdgpu_op_v_sub_f16,
	rose_amdgpu_op_v_subrev_f16,
	rose_amdgpu_op_v_mul_f16,
	rose_amdgpu_op_v_mac_f16,
	rose_amdgpu_op_v_madmk_f16,
	rose_amdgpu_op_v_madak_f16,
	rose_amdgpu_op_v_add_u16,
	rose_amdgpu_op_v_sub_u16,
	rose_amdgpu_op_v_subrev_u16,
	rose_amdgpu_op_v_mul_lo_u16,
	rose_amdgpu_op_v_lshlrev_b16,
	rose_amdgpu_op_v_lshrrev_b16,
	rose_amdgpu_op_v_ashrrev_i16,
	rose_amdgpu_op_v_max_f16,
	rose_amdgpu_op_v_min_f16,
	rose_amdgpu_op_v_max_u16,
	rose_amdgpu_op_v_max_i16,
	rose_amdgpu_op_v_min_u16,
	rose_amdgpu_op_v_min_i16,
	rose_amdgpu_op_v_ldexp_f16,
	rose_amdgpu_op_v_add_u32,
	rose_amdgpu_op_v_sub_u32,
	rose_amdgpu_op_v_subrev_u32,
	rose_amdgpu_op_ds_add_u32,
	rose_amdgpu_op_v_mad_legacy_f32,
	rose_amdgpu_op_v_mad_f32,
	rose_amdgpu_op_v_lshlrev_b64,
	rose_amdgpu_op_v_pack_b32_f16,
	rose_amdgpu_op_s_add_u32,
	rose_amdgpu_op_s_sub_u32,
	rose_amdgpu_op_s_add_i32,
	rose_amdgpu_op_s_sub_i32,
	rose_amdgpu_op_s_addc_u32,
	rose_amdgpu_op_s_subb_u32,
	rose_amdgpu_op_s_min_i32,
	rose_amdgpu_op_s_min_u32,
	rose_amdgpu_op_s_max_i32,
	rose_amdgpu_op_s_max_u32,
	rose_amdgpu_op_s_cslect_b32,
	rose_amdgpu_op_s_cslect_b64,
	rose_amdgpu_op_s_and_b32,
	rose_amdgpu_op_s_and_b64,
	rose_amdgpu_op_s_or_b32,
	rose_amdgpu_op_s_or_b64,
	rose_amdgpu_op_s_xor_b32,
	rose_amdgpu_op_s_xor_b64,
	rose_amdgpu_op_s_andn2_b32,
	rose_amdgpu_op_s_andn2_b64,
	rose_amdgpu_op_s_orn2_b32,
	rose_amdgpu_op_s_orn2_b64,
	rose_amdgpu_op_s_nand_b32,
	rose_amdgpu_op_s_nand_b64,
	rose_amdgpu_op_s_nor_b32,
	rose_amdgpu_op_s_nor_b64,
	rose_amdgpu_op_s_xnor_b32,
	rose_amdgpu_op_s_xnor_b64,
	rose_amdgpu_op_s_lshl_b32,
	rose_amdgpu_op_s_lshl_b64,
	rose_amdgpu_op_s_lshr_b32,
	rose_amdgpu_op_s_lshr_b64,
	rose_amdgpu_op_s_ashr_i32,
	rose_amdgpu_op_s_ashr_i64,
	rose_amdgpu_op_s_bfm_b32,
	rose_amdgpu_op_s_bfm_b64,
	rose_amdgpu_op_s_mul_i32,
	rose_amdgpu_op_s_bfe_u32,
	rose_amdgpu_op_s_bfe_i32,
	rose_amdgpu_op_s_bfe_u64,
	rose_amdgpu_op_s_bfe_i64,
	rose_amdgpu_op_s_cbranch_g_fork,
	rose_amdgpu_op_s_absdiff_i32,
	rose_amdgpu_op_s_rfe_restore_b64,
	rose_amdgpu_op_s_mul_hi_u32,
	rose_amdgpu_op_s_mul_hi_i32,
	rose_amdgpu_op_s_lshl1_add_u32,
	rose_amdgpu_op_s_lshsl2_add_u32,
	rose_amdgpu_op_s_lshl3_add_u32,
	rose_amdgpu_op_s_lshl4_add_u32,
	rose_amdgpu_op_s_pack_ll_b32_b16,
	rose_amdgpu_op_s_pack_lh_b32_b16,
	rose_amdgpu_op_s_pack_hh_b32_B16,
	rose_amdgpu_op_v_cmp_class_f32,
	rose_amdgpu_op_v_cmp_neq_f16,
	rose_amdgpu_op_v_cmp_lt_u32,
	rose_amdgpu_op_v_cmp_eq_u32,
	rose_amdgpu_op_v_cmp_le_u32,
	rose_amdgpu_op_v_cmp_gt_u32,
	rose_amdgpu_op_v_cmp_ne_u32,
	rose_amdgpu_op_v_cmp_ge_u32,
	rose_amdgpu_op_v_cmp_lt_u64,
	rose_amdgpu_op_v_cmp_eq_u64,
	rose_amdgpu_op_v_cmp_le_u64,
	rose_amdgpu_op_v_cmp_gt_u64,
	rose_amdgpu_op_v_cmp_ne_u64,
	rose_amdgpu_op_v_cmp_ge_u64,
	rose_amdgpu_op_v_interp_p1_f32,
	rose_amdgpu_op_v_interp_p2_f32,
	rose_amdgpu_op_v_interp_mov_f32,
	rose_amdgpu_op_s_load_dword,
	rose_amdgpu_op_s_load_dwordx2,
	rose_amdgpu_op_s_load_dwordx4,
	rose_amdgpu_op_s_load_dwordx8,
	rose_amdgpu_op_s_load_dwordx16,
	rose_amdgpu_op_s_scratch_load_dword,
	rose_amdgpu_op_s_scratch_load_dwordx2,
	rose_amdgpu_op_s_scratch_load_dwordx4,
	rose_amdgpu_op_s_buffer_load_dword,
	rose_amdgpu_op_s_buffer_load_dwordx2,
	rose_amdgpu_op_s_buffer_load_dwordx4,
	rose_amdgpu_op_s_buffer_load_dwordx8,
	rose_amdgpu_op_s_buffer_load_dwordx16,
	rose_amdgpu_op_s_store_dword,
	rose_amdgpu_op_s_store_dwordx2,
	rose_amdgpu_op_s_store_dwordx4,
	rose_amdgpu_op_s_scratch_store_dword,
	rose_amdgpu_op_s_scratch_store_dwordx2,
	rose_amdgpu_op_s_scratch_store_dwordx4,
	rose_amdgpu_op_s_buffer_store_dword,
	rose_amdgpu_op_s_buffer_store_dwordx2,
	rose_amdgpu_op_s_buffer_store_dwordx4,
	rose_amdgpu_op_s_dcache_inv,
	rose_amdgpu_op_s_dcache_wb,
	rose_amdgpu_op_s_dcache_inv_vol,
	rose_amdgpu_op_s_dcache_wb_vol,
	rose_amdgpu_op_s_dcache_memtime,
	rose_amdgpu_op_s_dcache_memrealtime,
	rose_amdgpu_op_s_mov_b32,
	rose_amdgpu_op_s_mov_b64,
	rose_amdgpu_op_s_cmov_b32,
	rose_amdgpu_op_s_cmov_b64,
	rose_amdgpu_op_s_not_b32,
	rose_amdgpu_op_s_not_b64,
	rose_amdgpu_op_s_wqm_b32,
	rose_amdgpu_op_s_wqm_b64,
	rose_amdgpu_op_s_brev_b32,
	rose_amdgpu_op_s_brev_b64,
	rose_amdgpu_op_s_bcnt0_i32_b32,
	rose_amdgpu_op_s_bcnt0_i32_b64,
	rose_amdgpu_op_s_bcnt1_i32_b32,
	rose_amdgpu_op_s_bcnt1_i32_b64,
	rose_amdgpu_op_s_ff0_i32_b32,
	rose_amdgpu_op_s_ff0_i32_b64,
	rose_amdgpu_op_s_ff1_i32_b32,
	rose_amdgpu_op_s_ff1_i32_b64,
	rose_amdgpu_op_s_flbit_i32_b32,
	rose_amdgpu_op_s_flbit_i32_b64,
	rose_amdgpu_op_s_fltbit_i32,
	rose_amdgpu_op_s_fltbit_i32_i64,
	rose_amdgpu_op_s_sext_i32_i8,
	rose_amdgpu_op_s_sext_i32_i16,
	rose_amdgpu_op_s_bitset0_b32,
	rose_amdgpu_op_s_bitset0_b64,
	rose_amdgpu_op_s_bitset1_b32,
	rose_amdgpu_op_s_bitset1_b64,
	rose_amdgpu_op_s_getpc_b64,
	rose_amdgpu_op_s_setpc_b64,
	rose_amdgpu_op_s_swappc_b64,
	rose_amdgpu_op_s_rfe_b64,
	rose_amdgpu_op_s_and_saveexec_b64,
	rose_amdgpu_op_s_or_savexec_b64,
	rose_amdgpu_op_s_xor_savexec_b64,
	rose_amdgpu_op_s_andn2_savexec_b64,
	rose_amdgpu_op_s_orn2_savexec_b64,
	rose_amdgpu_op_s_nand_savexec_b64,
	rose_amdgpu_op_s_nor_savexec_b64,
	rose_amdgpu_op_s_xnor_savexec_b64,
	rose_amdgpu_op_s_quadmask_b32,
	rose_amdgpu_op_s_quadmask_b64,
	rose_amdgpu_op_s_movrels_b32,
	rose_amdgpu_op_s_movrels_b64,
	rose_amdgpu_op_s_movreld_b32,
	rose_amdgpu_op_s_movreld_b64,
	rose_amdgpu_op_s_cbranch_join,
	rose_amdgpu_op_s_invalid_1,
	rose_amdgpu_op_s_abs_i32,
	rose_amdgpu_op_s_invalid_2,
	rose_amdgpu_op_s_set_gpr_idx_idx,
	rose_amdgpu_op_s_andn1_saveexec_b64,
	rose_amdgpu_op_s_orn1_saveexec_b64,
	rose_amdgpu_op_s_andn1_wrexec_b64,
	rose_amdgpu_op_s_andn2_wrexec_b64,
	rose_amdgpu_op_s_bitreplicate_b64_b32,
	rose_amdgpu_op_flat_load_ubyte,
	rose_amdgpu_op_flat_load_dword,
	rose_amdgpu_op_flat_store_dword,
	rose_amdgpu_op_s_movk_i32,
	rose_amdgpu_op_s_cmovk_i32,
	rose_amdgpu_op_s_cmpk_eq_i32,
	rose_amdgpu_op_s_cmpk_lg_i32,
	rose_amdgpu_op_s_cmpk_gt_i32,
	rose_amdgpu_op_s_cmpk_ge_i32,
	rose_amdgpu_op_s_cmpk_lt_i32,
	rose_amdgpu_op_s_cmpk_le_i32,
	rose_amdgpu_op_s_cmpk_eq_u32,
	rose_amdgpu_op_s_cmpk_lg_u32,
	rose_amdgpu_op_s_cmpk_gt_u32,
	rose_amdgpu_op_s_cmpk_ge_u32,
	rose_amdgpu_op_s_cmpk_lt_u32,
	rose_amdgpu_op_s_cmpk_le_u32,
	rose_amdgpu_op_s_addk_i32,
	rose_amdgpu_op_s_mulk_i32,
	rose_amdgpu_op_s_cbranch_i_fork,
	rose_amdgpu_op_s_getreg_b32,
	rose_amdgpu_op_s_setreg_b32,
	rose_amdgpu_op_s_setreg_imm32_b32,
	rose_amdgpu_op_s_call_b64,
	rose_amdgpu_op_v_nop,
	rose_amdgpu_op_v_mov_b32,
	rose_amdgpu_op_v_readfirstlane_b32,
	rose_amdgpu_op_v_cvt_i32_f64,
	rose_amdgpu_op_v_cvt_f64_i32,
	rose_amdgpu_op_v_cvt_f32_i32,
	rose_amdgpu_op_v_cvt_f32_u32,
	rose_amdgpu_op_v_cvt_u32_f32,
	rose_amdgpu_op_v_cvt_i32_f32,
	rose_amdgpu_op_v_cvt_f16_f32,
	rose_amdgpu_op_v_cvt_f32_f16,
	rose_amdgpu_op_v_cvt_rpi_i32_f32,
	rose_amdgpu_op_v_cvt_flr_i32_f32,
	rose_amdgpu_op_v_cvt_off_f32_i4,
	rose_amdgpu_op_v_pk_mad_i16
};
#endif
