
# __CLANG_OFFLOAD_BUNDLE____START__ hip-amdgcn-amd-amdhsa--gfx908
	.amdgcn_target "amdgcn-amd-amdhsa--gfx908"
	.amdhsa_code_object_version 6
	.text
	.p2align	2                               ; -- Begin function _Z1Af
	.type	_Z1Af,@function
_Z1Af:                                  ; @_Z1Af
; %bb.0:
	s_waitcnt vmcnt(0) expcnt(0) lgkmcnt(0)
	s_load_dwordx2 s[6:7], s[8:9], 0x50
	v_mbcnt_lo_u32_b32 v1, -1, 0
	v_mbcnt_hi_u32_b32 v31, -1, v1
	v_readfirstlane_b32 s4, v31
	v_mov_b32_e32 v6, 0
	v_mov_b32_e32 v7, 0
	v_cmp_eq_u32_e64 s[4:5], s4, v31
	s_and_saveexec_b64 s[8:9], s[4:5]
	s_cbranch_execz .LBB0_6
; %bb.1:
	v_mov_b32_e32 v1, 0
	s_waitcnt lgkmcnt(0)
	global_load_dwordx2 v[4:5], v1, s[6:7] offset:24 glc
	s_waitcnt vmcnt(0)
	buffer_wbinvl1_vol
	global_load_dwordx2 v[2:3], v1, s[6:7] offset:40
	global_load_dwordx2 v[6:7], v1, s[6:7]
	s_waitcnt vmcnt(1)
	v_and_b32_e32 v2, v2, v4
	v_and_b32_e32 v3, v3, v5
	v_mul_lo_u32 v3, v3, 24
	v_mul_hi_u32 v8, v2, 24
	v_mul_lo_u32 v2, v2, 24
	v_add_u32_e32 v3, v8, v3
	s_waitcnt vmcnt(0)
	v_add_co_u32_e32 v2, vcc, v6, v2
	v_addc_co_u32_e32 v3, vcc, v7, v3, vcc
	global_load_dwordx2 v[2:3], v[2:3], off glc
	s_waitcnt vmcnt(0)
	global_atomic_cmpswap_x2 v[6:7], v1, v[2:5], s[6:7] offset:24 glc
	s_waitcnt vmcnt(0)
	buffer_wbinvl1_vol
	v_cmp_ne_u64_e32 vcc, v[6:7], v[4:5]
	s_and_saveexec_b64 s[10:11], vcc
	s_cbranch_execz .LBB0_5
; %bb.2:
	s_mov_b64 s[12:13], 0
.LBB0_3:                                ; =>This Inner Loop Header: Depth=1
	s_sleep 1
	global_load_dwordx2 v[2:3], v1, s[6:7] offset:40
	global_load_dwordx2 v[8:9], v1, s[6:7]
	v_mov_b32_e32 v4, v6
	v_mov_b32_e32 v5, v7
	s_waitcnt vmcnt(1)
	v_and_b32_e32 v2, v2, v4
	s_waitcnt vmcnt(0)
	v_mad_u64_u32 v[6:7], s[14:15], v2, 24, v[8:9]
	v_and_b32_e32 v3, v3, v5
	v_mov_b32_e32 v2, v7
	v_mad_u64_u32 v[2:3], s[14:15], v3, 24, v[2:3]
	v_mov_b32_e32 v7, v2
	global_load_dwordx2 v[2:3], v[6:7], off glc
	s_waitcnt vmcnt(0)
	global_atomic_cmpswap_x2 v[6:7], v1, v[2:5], s[6:7] offset:24 glc
	s_waitcnt vmcnt(0)
	buffer_wbinvl1_vol
	v_cmp_eq_u64_e32 vcc, v[6:7], v[4:5]
	s_or_b64 s[12:13], vcc, s[12:13]
	s_andn2_b64 exec, exec, s[12:13]
	s_cbranch_execnz .LBB0_3
; %bb.4:
	s_or_b64 exec, exec, s[12:13]
.LBB0_5:
	s_or_b64 exec, exec, s[10:11]
.LBB0_6:
	s_or_b64 exec, exec, s[8:9]
	v_mov_b32_e32 v5, 0
	s_waitcnt lgkmcnt(0)
	global_load_dwordx2 v[8:9], v5, s[6:7] offset:40
	global_load_dwordx4 v[1:4], v5, s[6:7]
	v_readfirstlane_b32 s8, v6
	v_readfirstlane_b32 s9, v7
	s_mov_b64 s[10:11], exec
	s_waitcnt vmcnt(1)
	v_readfirstlane_b32 s12, v8
	v_readfirstlane_b32 s13, v9
	s_and_b64 s[12:13], s[8:9], s[12:13]
	s_mul_i32 s14, s13, 24
	s_mul_hi_u32 s15, s12, 24
	s_mul_i32 s16, s12, 24
	s_add_i32 s14, s15, s14
	v_mov_b32_e32 v6, s14
	s_waitcnt vmcnt(0)
	v_add_co_u32_e32 v8, vcc, s16, v1
	v_addc_co_u32_e32 v9, vcc, v2, v6, vcc
	s_and_saveexec_b64 s[14:15], s[4:5]
	s_cbranch_execz .LBB0_8
; %bb.7:
	v_mov_b32_e32 v10, s10
	v_mov_b32_e32 v11, s11
	v_mov_b32_e32 v12, 2
	v_mov_b32_e32 v13, 1
	global_store_dwordx4 v[8:9], v[10:13], off offset:8
.LBB0_8:
	s_or_b64 exec, exec, s[14:15]
	s_lshl_b64 s[10:11], s[12:13], 12
	v_mov_b32_e32 v6, s11
	v_add_co_u32_e32 v3, vcc, s10, v3
	v_addc_co_u32_e32 v12, vcc, v4, v6, vcc
	s_mov_b32 s12, 0
	v_lshlrev_b32_e32 v30, 6, v31
	v_mov_b32_e32 v4, 33
	v_mov_b32_e32 v6, v5
	v_mov_b32_e32 v7, v5
	v_readfirstlane_b32 s10, v3
	v_readfirstlane_b32 s11, v12
	v_add_co_u32_e32 v10, vcc, v3, v30
	s_mov_b32 s13, s12
	s_mov_b32 s14, s12
	s_mov_b32 s15, s12
	s_nop 0
	global_store_dwordx4 v30, v[4:7], s[10:11]
	v_mov_b32_e32 v3, s12
	v_addc_co_u32_e32 v11, vcc, 0, v12, vcc
	v_mov_b32_e32 v4, s13
	v_mov_b32_e32 v5, s14
	v_mov_b32_e32 v6, s15
	global_store_dwordx4 v30, v[3:6], s[10:11] offset:16
	global_store_dwordx4 v30, v[3:6], s[10:11] offset:32
	global_store_dwordx4 v30, v[3:6], s[10:11] offset:48
	s_and_saveexec_b64 s[10:11], s[4:5]
	s_cbranch_execz .LBB0_16
; %bb.9:
	v_mov_b32_e32 v7, 0
	global_load_dwordx2 v[14:15], v7, s[6:7] offset:32 glc
	global_load_dwordx2 v[3:4], v7, s[6:7] offset:40
	v_mov_b32_e32 v12, s8
	v_mov_b32_e32 v13, s9
	s_waitcnt vmcnt(0)
	v_and_b32_e32 v3, s8, v3
	v_and_b32_e32 v4, s9, v4
	v_mul_lo_u32 v4, v4, 24
	v_mul_hi_u32 v5, v3, 24
	v_mul_lo_u32 v3, v3, 24
	v_add_u32_e32 v4, v5, v4
	v_add_co_u32_e32 v5, vcc, v1, v3
	v_addc_co_u32_e32 v6, vcc, v2, v4, vcc
	global_store_dwordx2 v[5:6], v[14:15], off
	s_waitcnt vmcnt(0)
	global_atomic_cmpswap_x2 v[3:4], v7, v[12:15], s[6:7] offset:32 glc
	s_waitcnt vmcnt(0)
	v_cmp_ne_u64_e32 vcc, v[3:4], v[14:15]
	s_and_saveexec_b64 s[12:13], vcc
	s_cbranch_execz .LBB0_12
; %bb.10:
	s_mov_b64 s[14:15], 0
.LBB0_11:                               ; =>This Inner Loop Header: Depth=1
	s_sleep 1
	global_store_dwordx2 v[5:6], v[3:4], off
	v_mov_b32_e32 v1, s8
	v_mov_b32_e32 v2, s9
	s_waitcnt vmcnt(0)
	global_atomic_cmpswap_x2 v[1:2], v7, v[1:4], s[6:7] offset:32 glc
	s_waitcnt vmcnt(0)
	v_cmp_eq_u64_e32 vcc, v[1:2], v[3:4]
	v_mov_b32_e32 v4, v2
	s_or_b64 s[14:15], vcc, s[14:15]
	v_mov_b32_e32 v3, v1
	s_andn2_b64 exec, exec, s[14:15]
	s_cbranch_execnz .LBB0_11
.LBB0_12:
	s_or_b64 exec, exec, s[12:13]
	v_mov_b32_e32 v4, 0
	global_load_dwordx2 v[1:2], v4, s[6:7] offset:16
	s_mov_b64 s[12:13], exec
	v_mbcnt_lo_u32_b32 v3, s12, 0
	v_mbcnt_hi_u32_b32 v3, s13, v3
	v_cmp_eq_u32_e32 vcc, 0, v3
	s_and_saveexec_b64 s[14:15], vcc
	s_cbranch_execz .LBB0_14
; %bb.13:
	s_bcnt1_i32_b64 s12, s[12:13]
	v_mov_b32_e32 v3, s12
	s_waitcnt vmcnt(0)
	global_atomic_add_x2 v[1:2], v[3:4], off offset:8
.LBB0_14:
	s_or_b64 exec, exec, s[14:15]
	s_waitcnt vmcnt(0)
	global_load_dwordx2 v[3:4], v[1:2], off offset:16
	s_waitcnt vmcnt(0)
	v_cmp_eq_u64_e32 vcc, 0, v[3:4]
	s_cbranch_vccnz .LBB0_16
; %bb.15:
	global_load_dword v1, v[1:2], off offset:24
	v_mov_b32_e32 v2, 0
	s_waitcnt vmcnt(0)
	global_store_dwordx2 v[3:4], v[1:2], off
	v_and_b32_e32 v1, 0xffffff, v1
	v_readfirstlane_b32 s12, v1
	s_mov_b32 m0, s12
	s_nop 0
	s_sendmsg sendmsg(MSG_INTERRUPT)
.LBB0_16:
	s_or_b64 exec, exec, s[10:11]
	s_branch .LBB0_20
.LBB0_17:                               ;   in Loop: Header=BB0_20 Depth=1
	s_or_b64 exec, exec, s[10:11]
	v_readfirstlane_b32 s10, v1
	s_cmp_eq_u32 s10, 0
	s_cbranch_scc1 .LBB0_19
; %bb.18:                               ;   in Loop: Header=BB0_20 Depth=1
	s_sleep 1
	s_cbranch_execnz .LBB0_20
	s_branch .LBB0_22
.LBB0_19:
	s_branch .LBB0_22
.LBB0_20:                               ; =>This Inner Loop Header: Depth=1
	v_mov_b32_e32 v1, 1
	s_and_saveexec_b64 s[10:11], s[4:5]
	s_cbranch_execz .LBB0_17
; %bb.21:                               ;   in Loop: Header=BB0_20 Depth=1
	global_load_dword v1, v[8:9], off offset:20 glc
	s_waitcnt vmcnt(0)
	buffer_wbinvl1_vol
	v_and_b32_e32 v1, 1, v1
	s_branch .LBB0_17
.LBB0_22:
	global_load_dwordx2 v[1:2], v[10:11], off
	s_and_saveexec_b64 s[10:11], s[4:5]
	s_cbranch_execz .LBB0_25
; %bb.23:
	v_mov_b32_e32 v9, 0
	global_load_dwordx2 v[5:6], v9, s[6:7] offset:40
	global_load_dwordx2 v[10:11], v9, s[6:7] offset:24 glc
	global_load_dwordx2 v[7:8], v9, s[6:7]
	v_mov_b32_e32 v4, s9
	s_mov_b64 s[4:5], 0
	s_waitcnt vmcnt(2)
	v_add_co_u32_e32 v12, vcc, 1, v5
	v_addc_co_u32_e32 v13, vcc, 0, v6, vcc
	v_add_co_u32_e32 v3, vcc, s8, v12
	v_addc_co_u32_e32 v4, vcc, v13, v4, vcc
	v_cmp_eq_u64_e32 vcc, 0, v[3:4]
	v_cndmask_b32_e32 v4, v4, v13, vcc
	v_cndmask_b32_e32 v3, v3, v12, vcc
	v_and_b32_e32 v6, v4, v6
	v_and_b32_e32 v5, v3, v5
	v_mul_lo_u32 v6, v6, 24
	v_mul_hi_u32 v12, v5, 24
	v_mul_lo_u32 v13, v5, 24
	s_waitcnt vmcnt(1)
	v_mov_b32_e32 v5, v10
	v_add_u32_e32 v6, v12, v6
	s_waitcnt vmcnt(0)
	v_add_co_u32_e32 v7, vcc, v7, v13
	v_addc_co_u32_e32 v8, vcc, v8, v6, vcc
	global_store_dwordx2 v[7:8], v[10:11], off
	v_mov_b32_e32 v6, v11
	s_waitcnt vmcnt(0)
	global_atomic_cmpswap_x2 v[5:6], v9, v[3:6], s[6:7] offset:24 glc
	s_waitcnt vmcnt(0)
	v_cmp_ne_u64_e32 vcc, v[5:6], v[10:11]
	s_and_b64 exec, exec, vcc
	s_cbranch_execz .LBB0_25
.LBB0_24:                               ; =>This Inner Loop Header: Depth=1
	s_sleep 1
	global_store_dwordx2 v[7:8], v[5:6], off
	s_waitcnt vmcnt(0)
	global_atomic_cmpswap_x2 v[10:11], v9, v[3:6], s[6:7] offset:24 glc
	s_waitcnt vmcnt(0)
	v_cmp_eq_u64_e32 vcc, v[10:11], v[5:6]
	v_mov_b32_e32 v5, v10
	s_or_b64 s[4:5], vcc, s[4:5]
	v_mov_b32_e32 v6, v11
	s_andn2_b64 exec, exec, s[4:5]
	s_cbranch_execnz .LBB0_24
.LBB0_25:
	s_or_b64 exec, exec, s[10:11]
	s_getpc_b64 s[8:9]
	s_add_u32 s8, s8, .str@rel32@lo+4
	s_addc_u32 s9, s9, .str@rel32@hi+12
	s_cmp_lg_u64 s[8:9], 0
	s_cbranch_scc0 .LBB0_110
; %bb.26:
	s_waitcnt vmcnt(0)
	v_and_b32_e32 v32, 2, v1
	v_mov_b32_e32 v27, 0
	v_and_b32_e32 v3, -3, v1
	v_mov_b32_e32 v4, v2
	s_mov_b64 s[10:11], 14
	v_mov_b32_e32 v9, 2
	v_mov_b32_e32 v10, 1
	s_branch .LBB0_28
.LBB0_27:                               ;   in Loop: Header=BB0_28 Depth=1
	s_or_b64 exec, exec, s[16:17]
	s_sub_u32 s10, s10, s12
	s_subb_u32 s11, s11, s13
	s_add_u32 s8, s8, s12
	s_addc_u32 s9, s9, s13
	s_cmp_lg_u64 s[10:11], 0
	s_cbranch_scc0 .LBB0_109
.LBB0_28:                               ; =>This Loop Header: Depth=1
                                        ;     Child Loop BB0_31 Depth 2
                                        ;     Child Loop BB0_37 Depth 2
                                        ;     Child Loop BB0_46 Depth 2
                                        ;     Child Loop BB0_54 Depth 2
                                        ;     Child Loop BB0_62 Depth 2
                                        ;     Child Loop BB0_70 Depth 2
                                        ;     Child Loop BB0_78 Depth 2
                                        ;     Child Loop BB0_86 Depth 2
                                        ;     Child Loop BB0_94 Depth 2
                                        ;     Child Loop BB0_103 Depth 2
                                        ;     Child Loop BB0_108 Depth 2
	v_cmp_lt_u64_e64 s[4:5], s[10:11], 56
	v_cmp_gt_u64_e64 s[14:15], s[10:11], 7
	s_and_b64 s[4:5], s[4:5], exec
	s_cselect_b32 s13, s11, 0
	s_cselect_b32 s12, s10, 56
	s_and_b64 vcc, exec, s[14:15]
	s_cbranch_vccnz .LBB0_38
; %bb.29:                               ;   in Loop: Header=BB0_28 Depth=1
	v_mov_b32_e32 v5, 0
	s_cmp_eq_u64 s[10:11], 0
	v_mov_b32_e32 v6, 0
	s_mov_b64 s[4:5], 0
	s_cbranch_scc1 .LBB0_32
; %bb.30:                               ;   in Loop: Header=BB0_28 Depth=1
	v_mov_b32_e32 v5, 0
	s_lshl_b64 s[14:15], s[12:13], 3
	s_mov_b64 s[16:17], 0
	v_mov_b32_e32 v6, 0
	s_mov_b64 s[18:19], s[8:9]
.LBB0_31:                               ;   Parent Loop BB0_28 Depth=1
                                        ; =>  This Inner Loop Header: Depth=2
	global_load_ubyte v7, v27, s[18:19]
	s_waitcnt vmcnt(0)
	v_and_b32_e32 v26, 0xffff, v7
	v_lshlrev_b64 v[7:8], s16, v[26:27]
	s_add_u32 s16, s16, 8
	s_addc_u32 s17, s17, 0
	s_add_u32 s18, s18, 1
	s_addc_u32 s19, s19, 0
	v_or_b32_e32 v5, v7, v5
	s_cmp_lg_u32 s14, s16
	v_or_b32_e32 v6, v8, v6
	s_cbranch_scc1 .LBB0_31
.LBB0_32:                               ;   in Loop: Header=BB0_28 Depth=1
	s_mov_b32 s18, 0
	s_andn2_b64 vcc, exec, s[4:5]
	s_mov_b64 s[4:5], s[8:9]
	s_cbranch_vccnz .LBB0_34
.LBB0_33:                               ;   in Loop: Header=BB0_28 Depth=1
	global_load_dwordx2 v[5:6], v27, s[8:9]
	s_add_i32 s18, s12, -8
	s_add_u32 s4, s8, 8
	s_addc_u32 s5, s9, 0
.LBB0_34:                               ;   in Loop: Header=BB0_28 Depth=1
	s_cmp_gt_u32 s18, 7
	s_cbranch_scc1 .LBB0_39
; %bb.35:                               ;   in Loop: Header=BB0_28 Depth=1
	s_cmp_eq_u32 s18, 0
	s_cbranch_scc1 .LBB0_40
; %bb.36:                               ;   in Loop: Header=BB0_28 Depth=1
	v_mov_b32_e32 v11, 0
	s_mov_b64 s[14:15], 0
	v_mov_b32_e32 v12, 0
	s_mov_b64 s[16:17], 0
.LBB0_37:                               ;   Parent Loop BB0_28 Depth=1
                                        ; =>  This Inner Loop Header: Depth=2
	s_add_u32 s20, s4, s16
	s_addc_u32 s21, s5, s17
	global_load_ubyte v7, v27, s[20:21]
	s_add_u32 s16, s16, 1
	s_addc_u32 s17, s17, 0
	s_waitcnt vmcnt(0)
	v_and_b32_e32 v26, 0xffff, v7
	v_lshlrev_b64 v[7:8], s14, v[26:27]
	s_add_u32 s14, s14, 8
	s_addc_u32 s15, s15, 0
	v_or_b32_e32 v11, v7, v11
	s_cmp_lg_u32 s18, s16
	v_or_b32_e32 v12, v8, v12
	s_cbranch_scc1 .LBB0_37
	s_branch .LBB0_41
.LBB0_38:                               ;   in Loop: Header=BB0_28 Depth=1
	s_mov_b32 s18, 0
	s_mov_b64 s[4:5], s[8:9]
	s_branch .LBB0_33
.LBB0_39:                               ;   in Loop: Header=BB0_28 Depth=1
                                        ; implicit-def: $vgpr11_vgpr12
	s_mov_b32 s19, 0
	s_branch .LBB0_42
.LBB0_40:                               ;   in Loop: Header=BB0_28 Depth=1
	v_mov_b32_e32 v11, 0
	v_mov_b32_e32 v12, 0
.LBB0_41:                               ;   in Loop: Header=BB0_28 Depth=1
	s_mov_b32 s19, 0
	s_cbranch_execnz .LBB0_43
.LBB0_42:                               ;   in Loop: Header=BB0_28 Depth=1
	global_load_dwordx2 v[11:12], v27, s[4:5]
	s_add_i32 s19, s18, -8
	s_add_u32 s4, s4, 8
	s_addc_u32 s5, s5, 0
.LBB0_43:                               ;   in Loop: Header=BB0_28 Depth=1
	s_cmp_gt_u32 s19, 7
	s_cbranch_scc1 .LBB0_47
; %bb.44:                               ;   in Loop: Header=BB0_28 Depth=1
	s_cmp_eq_u32 s19, 0
	s_cbranch_scc1 .LBB0_48
; %bb.45:                               ;   in Loop: Header=BB0_28 Depth=1
	v_mov_b32_e32 v13, 0
	s_mov_b64 s[14:15], 0
	v_mov_b32_e32 v14, 0
	s_mov_b64 s[16:17], 0
.LBB0_46:                               ;   Parent Loop BB0_28 Depth=1
                                        ; =>  This Inner Loop Header: Depth=2
	s_add_u32 s20, s4, s16
	s_addc_u32 s21, s5, s17
	global_load_ubyte v7, v27, s[20:21]
	s_add_u32 s16, s16, 1
	s_addc_u32 s17, s17, 0
	s_waitcnt vmcnt(0)
	v_and_b32_e32 v26, 0xffff, v7
	v_lshlrev_b64 v[7:8], s14, v[26:27]
	s_add_u32 s14, s14, 8
	s_addc_u32 s15, s15, 0
	v_or_b32_e32 v13, v7, v13
	s_cmp_lg_u32 s19, s16
	v_or_b32_e32 v14, v8, v14
	s_cbranch_scc1 .LBB0_46
	s_branch .LBB0_49
.LBB0_47:                               ;   in Loop: Header=BB0_28 Depth=1
	s_mov_b32 s18, 0
	s_branch .LBB0_50
.LBB0_48:                               ;   in Loop: Header=BB0_28 Depth=1
	v_mov_b32_e32 v13, 0
	v_mov_b32_e32 v14, 0
.LBB0_49:                               ;   in Loop: Header=BB0_28 Depth=1
	s_mov_b32 s18, 0
	s_cbranch_execnz .LBB0_51
.LBB0_50:                               ;   in Loop: Header=BB0_28 Depth=1
	global_load_dwordx2 v[13:14], v27, s[4:5]
	s_add_i32 s18, s19, -8
	s_add_u32 s4, s4, 8
	s_addc_u32 s5, s5, 0
.LBB0_51:                               ;   in Loop: Header=BB0_28 Depth=1
	s_cmp_gt_u32 s18, 7
	s_cbranch_scc1 .LBB0_55
; %bb.52:                               ;   in Loop: Header=BB0_28 Depth=1
	s_cmp_eq_u32 s18, 0
	s_cbranch_scc1 .LBB0_56
; %bb.53:                               ;   in Loop: Header=BB0_28 Depth=1
	v_mov_b32_e32 v15, 0
	s_mov_b64 s[14:15], 0
	v_mov_b32_e32 v16, 0
	s_mov_b64 s[16:17], 0
.LBB0_54:                               ;   Parent Loop BB0_28 Depth=1
                                        ; =>  This Inner Loop Header: Depth=2
	s_add_u32 s20, s4, s16
	s_addc_u32 s21, s5, s17
	global_load_ubyte v7, v27, s[20:21]
	s_add_u32 s16, s16, 1
	s_addc_u32 s17, s17, 0
	s_waitcnt vmcnt(0)
	v_and_b32_e32 v26, 0xffff, v7
	v_lshlrev_b64 v[7:8], s14, v[26:27]
	s_add_u32 s14, s14, 8
	s_addc_u32 s15, s15, 0
	v_or_b32_e32 v15, v7, v15
	s_cmp_lg_u32 s18, s16
	v_or_b32_e32 v16, v8, v16
	s_cbranch_scc1 .LBB0_54
	s_branch .LBB0_57
.LBB0_55:                               ;   in Loop: Header=BB0_28 Depth=1
                                        ; implicit-def: $vgpr15_vgpr16
	s_mov_b32 s19, 0
	s_branch .LBB0_58
.LBB0_56:                               ;   in Loop: Header=BB0_28 Depth=1
	v_mov_b32_e32 v15, 0
	v_mov_b32_e32 v16, 0
.LBB0_57:                               ;   in Loop: Header=BB0_28 Depth=1
	s_mov_b32 s19, 0
	s_cbranch_execnz .LBB0_59
.LBB0_58:                               ;   in Loop: Header=BB0_28 Depth=1
	global_load_dwordx2 v[15:16], v27, s[4:5]
	s_add_i32 s19, s18, -8
	s_add_u32 s4, s4, 8
	s_addc_u32 s5, s5, 0
.LBB0_59:                               ;   in Loop: Header=BB0_28 Depth=1
	s_cmp_gt_u32 s19, 7
	s_cbranch_scc1 .LBB0_63
; %bb.60:                               ;   in Loop: Header=BB0_28 Depth=1
	s_cmp_eq_u32 s19, 0
	s_cbranch_scc1 .LBB0_64
; %bb.61:                               ;   in Loop: Header=BB0_28 Depth=1
	v_mov_b32_e32 v17, 0
	s_mov_b64 s[14:15], 0
	v_mov_b32_e32 v18, 0
	s_mov_b64 s[16:17], 0
.LBB0_62:                               ;   Parent Loop BB0_28 Depth=1
                                        ; =>  This Inner Loop Header: Depth=2
	s_add_u32 s20, s4, s16
	s_addc_u32 s21, s5, s17
	global_load_ubyte v7, v27, s[20:21]
	s_add_u32 s16, s16, 1
	s_addc_u32 s17, s17, 0
	s_waitcnt vmcnt(0)
	v_and_b32_e32 v26, 0xffff, v7
	v_lshlrev_b64 v[7:8], s14, v[26:27]
	s_add_u32 s14, s14, 8
	s_addc_u32 s15, s15, 0
	v_or_b32_e32 v17, v7, v17
	s_cmp_lg_u32 s19, s16
	v_or_b32_e32 v18, v8, v18
	s_cbranch_scc1 .LBB0_62
	s_branch .LBB0_65
.LBB0_63:                               ;   in Loop: Header=BB0_28 Depth=1
	s_mov_b32 s18, 0
	s_branch .LBB0_66
.LBB0_64:                               ;   in Loop: Header=BB0_28 Depth=1
	v_mov_b32_e32 v17, 0
	v_mov_b32_e32 v18, 0
.LBB0_65:                               ;   in Loop: Header=BB0_28 Depth=1
	s_mov_b32 s18, 0
	s_cbranch_execnz .LBB0_67
.LBB0_66:                               ;   in Loop: Header=BB0_28 Depth=1
	global_load_dwordx2 v[17:18], v27, s[4:5]
	s_add_i32 s18, s19, -8
	s_add_u32 s4, s4, 8
	s_addc_u32 s5, s5, 0
.LBB0_67:                               ;   in Loop: Header=BB0_28 Depth=1
	s_cmp_gt_u32 s18, 7
	s_cbranch_scc1 .LBB0_71
; %bb.68:                               ;   in Loop: Header=BB0_28 Depth=1
	s_cmp_eq_u32 s18, 0
	s_cbranch_scc1 .LBB0_72
; %bb.69:                               ;   in Loop: Header=BB0_28 Depth=1
	v_mov_b32_e32 v19, 0
	s_mov_b64 s[14:15], 0
	v_mov_b32_e32 v20, 0
	s_mov_b64 s[16:17], 0
.LBB0_70:                               ;   Parent Loop BB0_28 Depth=1
                                        ; =>  This Inner Loop Header: Depth=2
	s_add_u32 s20, s4, s16
	s_addc_u32 s21, s5, s17
	global_load_ubyte v7, v27, s[20:21]
	s_add_u32 s16, s16, 1
	s_addc_u32 s17, s17, 0
	s_waitcnt vmcnt(0)
	v_and_b32_e32 v26, 0xffff, v7
	v_lshlrev_b64 v[7:8], s14, v[26:27]
	s_add_u32 s14, s14, 8
	s_addc_u32 s15, s15, 0
	v_or_b32_e32 v19, v7, v19
	s_cmp_lg_u32 s18, s16
	v_or_b32_e32 v20, v8, v20
	s_cbranch_scc1 .LBB0_70
	s_branch .LBB0_73
.LBB0_71:                               ;   in Loop: Header=BB0_28 Depth=1
                                        ; implicit-def: $vgpr19_vgpr20
	s_mov_b32 s19, 0
	s_branch .LBB0_74
.LBB0_72:                               ;   in Loop: Header=BB0_28 Depth=1
	v_mov_b32_e32 v19, 0
	v_mov_b32_e32 v20, 0
.LBB0_73:                               ;   in Loop: Header=BB0_28 Depth=1
	s_mov_b32 s19, 0
	s_cbranch_execnz .LBB0_75
.LBB0_74:                               ;   in Loop: Header=BB0_28 Depth=1
	global_load_dwordx2 v[19:20], v27, s[4:5]
	s_add_i32 s19, s18, -8
	s_add_u32 s4, s4, 8
	s_addc_u32 s5, s5, 0
.LBB0_75:                               ;   in Loop: Header=BB0_28 Depth=1
	s_cmp_gt_u32 s19, 7
	s_cbranch_scc1 .LBB0_79
; %bb.76:                               ;   in Loop: Header=BB0_28 Depth=1
	s_cmp_eq_u32 s19, 0
	s_cbranch_scc1 .LBB0_80
; %bb.77:                               ;   in Loop: Header=BB0_28 Depth=1
	v_mov_b32_e32 v21, 0
	s_mov_b64 s[14:15], 0
	v_mov_b32_e32 v22, 0
	s_mov_b64 s[16:17], s[4:5]
.LBB0_78:                               ;   Parent Loop BB0_28 Depth=1
                                        ; =>  This Inner Loop Header: Depth=2
	global_load_ubyte v7, v27, s[16:17]
	s_add_i32 s19, s19, -1
	s_waitcnt vmcnt(0)
	v_and_b32_e32 v26, 0xffff, v7
	v_lshlrev_b64 v[7:8], s14, v[26:27]
	s_add_u32 s14, s14, 8
	s_addc_u32 s15, s15, 0
	s_add_u32 s16, s16, 1
	s_addc_u32 s17, s17, 0
	v_or_b32_e32 v21, v7, v21
	s_cmp_lg_u32 s19, 0
	v_or_b32_e32 v22, v8, v22
	s_cbranch_scc1 .LBB0_78
	s_branch .LBB0_81
.LBB0_79:                               ;   in Loop: Header=BB0_28 Depth=1
	s_branch .LBB0_82
.LBB0_80:                               ;   in Loop: Header=BB0_28 Depth=1
	v_mov_b32_e32 v21, 0
	v_mov_b32_e32 v22, 0
.LBB0_81:                               ;   in Loop: Header=BB0_28 Depth=1
	s_cbranch_execnz .LBB0_83
.LBB0_82:                               ;   in Loop: Header=BB0_28 Depth=1
	global_load_dwordx2 v[21:22], v27, s[4:5]
.LBB0_83:                               ;   in Loop: Header=BB0_28 Depth=1
	v_readfirstlane_b32 s4, v31
	v_mov_b32_e32 v7, 0
	v_mov_b32_e32 v8, 0
	v_cmp_eq_u32_e64 s[4:5], s4, v31
	s_and_saveexec_b64 s[14:15], s[4:5]
	s_cbranch_execz .LBB0_89
; %bb.84:                               ;   in Loop: Header=BB0_28 Depth=1
	global_load_dwordx2 v[25:26], v27, s[6:7] offset:24 glc
	s_waitcnt vmcnt(0)
	buffer_wbinvl1_vol
	global_load_dwordx2 v[7:8], v27, s[6:7] offset:40
	global_load_dwordx2 v[23:24], v27, s[6:7]
	s_waitcnt vmcnt(1)
	v_and_b32_e32 v7, v7, v25
	v_and_b32_e32 v8, v8, v26
	v_mul_lo_u32 v8, v8, 24
	v_mul_hi_u32 v28, v7, 24
	v_mul_lo_u32 v7, v7, 24
	v_add_u32_e32 v8, v28, v8
	s_waitcnt vmcnt(0)
	v_add_co_u32_e32 v7, vcc, v23, v7
	v_addc_co_u32_e32 v8, vcc, v24, v8, vcc
	global_load_dwordx2 v[23:24], v[7:8], off glc
	s_waitcnt vmcnt(0)
	global_atomic_cmpswap_x2 v[7:8], v27, v[23:26], s[6:7] offset:24 glc
	s_waitcnt vmcnt(0)
	buffer_wbinvl1_vol
	v_cmp_ne_u64_e32 vcc, v[7:8], v[25:26]
	s_and_saveexec_b64 s[16:17], vcc
	s_cbranch_execz .LBB0_88
; %bb.85:                               ;   in Loop: Header=BB0_28 Depth=1
	s_mov_b64 s[18:19], 0
.LBB0_86:                               ;   Parent Loop BB0_28 Depth=1
                                        ; =>  This Inner Loop Header: Depth=2
	s_sleep 1
	global_load_dwordx2 v[23:24], v27, s[6:7] offset:40
	global_load_dwordx2 v[28:29], v27, s[6:7]
	v_mov_b32_e32 v26, v8
	v_mov_b32_e32 v25, v7
	s_waitcnt vmcnt(1)
	v_and_b32_e32 v7, v23, v25
	s_waitcnt vmcnt(0)
	v_mad_u64_u32 v[7:8], s[20:21], v7, 24, v[28:29]
	v_and_b32_e32 v23, v24, v26
	v_mad_u64_u32 v[23:24], s[20:21], v23, 24, v[8:9]
	v_mov_b32_e32 v8, v23
	global_load_dwordx2 v[23:24], v[7:8], off glc
	s_waitcnt vmcnt(0)
	global_atomic_cmpswap_x2 v[7:8], v27, v[23:26], s[6:7] offset:24 glc
	s_waitcnt vmcnt(0)
	buffer_wbinvl1_vol
	v_cmp_eq_u64_e32 vcc, v[7:8], v[25:26]
	s_or_b64 s[18:19], vcc, s[18:19]
	s_andn2_b64 exec, exec, s[18:19]
	s_cbranch_execnz .LBB0_86
; %bb.87:                               ;   in Loop: Header=BB0_28 Depth=1
	s_or_b64 exec, exec, s[18:19]
.LBB0_88:                               ;   in Loop: Header=BB0_28 Depth=1
	s_or_b64 exec, exec, s[16:17]
.LBB0_89:                               ;   in Loop: Header=BB0_28 Depth=1
	s_or_b64 exec, exec, s[14:15]
	global_load_dwordx2 v[28:29], v27, s[6:7] offset:40
	global_load_dwordx4 v[23:26], v27, s[6:7]
	v_readfirstlane_b32 s14, v7
	v_readfirstlane_b32 s15, v8
	s_mov_b64 s[16:17], exec
	s_waitcnt vmcnt(1)
	v_readfirstlane_b32 s18, v28
	v_readfirstlane_b32 s19, v29
	s_and_b64 s[18:19], s[14:15], s[18:19]
	s_mul_i32 s20, s19, 24
	s_mul_hi_u32 s21, s18, 24
	s_mul_i32 s22, s18, 24
	s_add_i32 s20, s21, s20
	v_mov_b32_e32 v7, s20
	s_waitcnt vmcnt(0)
	v_add_co_u32_e32 v28, vcc, s22, v23
	v_addc_co_u32_e32 v29, vcc, v24, v7, vcc
	s_and_saveexec_b64 s[20:21], s[4:5]
	s_cbranch_execz .LBB0_91
; %bb.90:                               ;   in Loop: Header=BB0_28 Depth=1
	v_mov_b32_e32 v7, s16
	v_mov_b32_e32 v8, s17
	global_store_dwordx4 v[28:29], v[7:10], off offset:8
.LBB0_91:                               ;   in Loop: Header=BB0_28 Depth=1
	s_or_b64 exec, exec, s[20:21]
	s_lshl_b64 s[16:17], s[18:19], 12
	v_mov_b32_e32 v7, s17
	v_add_co_u32_e32 v25, vcc, s16, v25
	v_addc_co_u32_e32 v33, vcc, v26, v7, vcc
	v_cmp_lt_u64_e64 vcc, s[10:11], 57
	s_lshl_b32 s16, s12, 2
	v_cndmask_b32_e32 v7, 0, v32, vcc
	s_add_i32 s16, s16, 28
	v_and_b32_e32 v3, 0xffffff1f, v3
	s_and_b32 s16, s16, 0x1e0
	v_or_b32_e32 v3, v3, v7
	v_or_b32_e32 v3, s16, v3
	v_readfirstlane_b32 s16, v25
	v_readfirstlane_b32 s17, v33
	s_nop 4
	global_store_dwordx4 v30, v[3:6], s[16:17]
	global_store_dwordx4 v30, v[11:14], s[16:17] offset:16
	global_store_dwordx4 v30, v[15:18], s[16:17] offset:32
	global_store_dwordx4 v30, v[19:22], s[16:17] offset:48
	s_and_saveexec_b64 s[16:17], s[4:5]
	s_cbranch_execz .LBB0_99
; %bb.92:                               ;   in Loop: Header=BB0_28 Depth=1
	global_load_dwordx2 v[13:14], v27, s[6:7] offset:32 glc
	global_load_dwordx2 v[3:4], v27, s[6:7] offset:40
	v_mov_b32_e32 v11, s14
	v_mov_b32_e32 v12, s15
	s_waitcnt vmcnt(0)
	v_readfirstlane_b32 s18, v3
	v_readfirstlane_b32 s19, v4
	s_and_b64 s[18:19], s[18:19], s[14:15]
	s_mul_i32 s19, s19, 24
	s_mul_hi_u32 s20, s18, 24
	s_mul_i32 s18, s18, 24
	s_add_i32 s19, s20, s19
	v_mov_b32_e32 v3, s19
	v_add_co_u32_e32 v7, vcc, s18, v23
	v_addc_co_u32_e32 v8, vcc, v24, v3, vcc
	global_store_dwordx2 v[7:8], v[13:14], off
	s_waitcnt vmcnt(0)
	global_atomic_cmpswap_x2 v[5:6], v27, v[11:14], s[6:7] offset:32 glc
	s_waitcnt vmcnt(0)
	v_cmp_ne_u64_e32 vcc, v[5:6], v[13:14]
	s_and_saveexec_b64 s[18:19], vcc
	s_cbranch_execz .LBB0_95
; %bb.93:                               ;   in Loop: Header=BB0_28 Depth=1
	s_mov_b64 s[20:21], 0
.LBB0_94:                               ;   Parent Loop BB0_28 Depth=1
                                        ; =>  This Inner Loop Header: Depth=2
	s_sleep 1
	global_store_dwordx2 v[7:8], v[5:6], off
	v_mov_b32_e32 v3, s14
	v_mov_b32_e32 v4, s15
	s_waitcnt vmcnt(0)
	global_atomic_cmpswap_x2 v[3:4], v27, v[3:6], s[6:7] offset:32 glc
	s_waitcnt vmcnt(0)
	v_cmp_eq_u64_e32 vcc, v[3:4], v[5:6]
	v_mov_b32_e32 v6, v4
	s_or_b64 s[20:21], vcc, s[20:21]
	v_mov_b32_e32 v5, v3
	s_andn2_b64 exec, exec, s[20:21]
	s_cbranch_execnz .LBB0_94
.LBB0_95:                               ;   in Loop: Header=BB0_28 Depth=1
	s_or_b64 exec, exec, s[18:19]
	global_load_dwordx2 v[3:4], v27, s[6:7] offset:16
	s_mov_b64 s[20:21], exec
	v_mbcnt_lo_u32_b32 v5, s20, 0
	v_mbcnt_hi_u32_b32 v5, s21, v5
	v_cmp_eq_u32_e32 vcc, 0, v5
	s_and_saveexec_b64 s[18:19], vcc
	s_cbranch_execz .LBB0_97
; %bb.96:                               ;   in Loop: Header=BB0_28 Depth=1
	s_bcnt1_i32_b64 s20, s[20:21]
	v_mov_b32_e32 v26, s20
	s_waitcnt vmcnt(0)
	global_atomic_add_x2 v[3:4], v[26:27], off offset:8
.LBB0_97:                               ;   in Loop: Header=BB0_28 Depth=1
	s_or_b64 exec, exec, s[18:19]
	s_waitcnt vmcnt(0)
	global_load_dwordx2 v[5:6], v[3:4], off offset:16
	s_waitcnt vmcnt(0)
	v_cmp_eq_u64_e32 vcc, 0, v[5:6]
	s_cbranch_vccnz .LBB0_99
; %bb.98:                               ;   in Loop: Header=BB0_28 Depth=1
	global_load_dword v26, v[3:4], off offset:24
	s_waitcnt vmcnt(0)
	v_and_b32_e32 v3, 0xffffff, v26
	v_readfirstlane_b32 s18, v3
	s_mov_b32 m0, s18
	global_store_dwordx2 v[5:6], v[26:27], off
	s_sendmsg sendmsg(MSG_INTERRUPT)
.LBB0_99:                               ;   in Loop: Header=BB0_28 Depth=1
	s_or_b64 exec, exec, s[16:17]
	v_add_co_u32_e32 v3, vcc, v25, v30
	v_addc_co_u32_e32 v4, vcc, 0, v33, vcc
	s_branch .LBB0_103
.LBB0_100:                              ;   in Loop: Header=BB0_103 Depth=2
	s_or_b64 exec, exec, s[16:17]
	v_readfirstlane_b32 s16, v5
	s_cmp_eq_u32 s16, 0
	s_cbranch_scc1 .LBB0_102
; %bb.101:                              ;   in Loop: Header=BB0_103 Depth=2
	s_sleep 1
	s_cbranch_execnz .LBB0_103
	s_branch .LBB0_105
.LBB0_102:                              ;   in Loop: Header=BB0_28 Depth=1
	s_branch .LBB0_105
.LBB0_103:                              ;   Parent Loop BB0_28 Depth=1
                                        ; =>  This Inner Loop Header: Depth=2
	v_mov_b32_e32 v5, 1
	s_and_saveexec_b64 s[16:17], s[4:5]
	s_cbranch_execz .LBB0_100
; %bb.104:                              ;   in Loop: Header=BB0_103 Depth=2
	global_load_dword v5, v[28:29], off offset:20 glc
	s_waitcnt vmcnt(0)
	buffer_wbinvl1_vol
	v_and_b32_e32 v5, 1, v5
	s_branch .LBB0_100
.LBB0_105:                              ;   in Loop: Header=BB0_28 Depth=1
	global_load_dwordx2 v[3:4], v[3:4], off
	s_and_saveexec_b64 s[16:17], s[4:5]
	s_cbranch_execz .LBB0_27
; %bb.106:                              ;   in Loop: Header=BB0_28 Depth=1
	global_load_dwordx2 v[7:8], v27, s[6:7] offset:40
	global_load_dwordx2 v[13:14], v27, s[6:7] offset:24 glc
	global_load_dwordx2 v[11:12], v27, s[6:7]
	v_mov_b32_e32 v6, s15
	s_waitcnt vmcnt(2)
	v_add_co_u32_e32 v15, vcc, 1, v7
	v_addc_co_u32_e32 v16, vcc, 0, v8, vcc
	v_add_co_u32_e32 v5, vcc, s14, v15
	v_addc_co_u32_e32 v6, vcc, v16, v6, vcc
	v_cmp_eq_u64_e32 vcc, 0, v[5:6]
	v_cndmask_b32_e32 v6, v6, v16, vcc
	v_cndmask_b32_e32 v5, v5, v15, vcc
	v_and_b32_e32 v8, v6, v8
	v_and_b32_e32 v7, v5, v7
	v_mul_lo_u32 v8, v8, 24
	v_mul_hi_u32 v15, v7, 24
	v_mul_lo_u32 v16, v7, 24
	s_waitcnt vmcnt(1)
	v_mov_b32_e32 v7, v13
	v_add_u32_e32 v8, v15, v8
	s_waitcnt vmcnt(0)
	v_add_co_u32_e32 v11, vcc, v11, v16
	v_addc_co_u32_e32 v12, vcc, v12, v8, vcc
	global_store_dwordx2 v[11:12], v[13:14], off
	v_mov_b32_e32 v8, v14
	s_waitcnt vmcnt(0)
	global_atomic_cmpswap_x2 v[7:8], v27, v[5:8], s[6:7] offset:24 glc
	s_waitcnt vmcnt(0)
	v_cmp_ne_u64_e32 vcc, v[7:8], v[13:14]
	s_and_b64 exec, exec, vcc
	s_cbranch_execz .LBB0_27
; %bb.107:                              ;   in Loop: Header=BB0_28 Depth=1
	s_mov_b64 s[4:5], 0
.LBB0_108:                              ;   Parent Loop BB0_28 Depth=1
                                        ; =>  This Inner Loop Header: Depth=2
	s_sleep 1
	global_store_dwordx2 v[11:12], v[7:8], off
	s_waitcnt vmcnt(0)
	global_atomic_cmpswap_x2 v[13:14], v27, v[5:8], s[6:7] offset:24 glc
	s_waitcnt vmcnt(0)
	v_cmp_eq_u64_e32 vcc, v[13:14], v[7:8]
	v_mov_b32_e32 v7, v13
	s_or_b64 s[4:5], vcc, s[4:5]
	v_mov_b32_e32 v8, v14
	s_andn2_b64 exec, exec, s[4:5]
	s_cbranch_execnz .LBB0_108
	s_branch .LBB0_27
.LBB0_109:
	s_branch .LBB0_137
.LBB0_110:
                                        ; implicit-def: $vgpr3_vgpr4
	s_cbranch_execz .LBB0_137
; %bb.111:
	v_readfirstlane_b32 s4, v31
	v_mov_b32_e32 v9, 0
	v_mov_b32_e32 v10, 0
	v_cmp_eq_u32_e64 s[4:5], s4, v31
	s_and_saveexec_b64 s[8:9], s[4:5]
	s_cbranch_execz .LBB0_117
; %bb.112:
	s_waitcnt vmcnt(0)
	v_mov_b32_e32 v3, 0
	global_load_dwordx2 v[6:7], v3, s[6:7] offset:24 glc
	s_waitcnt vmcnt(0)
	buffer_wbinvl1_vol
	global_load_dwordx2 v[4:5], v3, s[6:7] offset:40
	global_load_dwordx2 v[8:9], v3, s[6:7]
	s_waitcnt vmcnt(1)
	v_and_b32_e32 v4, v4, v6
	v_and_b32_e32 v5, v5, v7
	v_mul_lo_u32 v5, v5, 24
	v_mul_hi_u32 v10, v4, 24
	v_mul_lo_u32 v4, v4, 24
	v_add_u32_e32 v5, v10, v5
	s_waitcnt vmcnt(0)
	v_add_co_u32_e32 v4, vcc, v8, v4
	v_addc_co_u32_e32 v5, vcc, v9, v5, vcc
	global_load_dwordx2 v[4:5], v[4:5], off glc
	s_waitcnt vmcnt(0)
	global_atomic_cmpswap_x2 v[9:10], v3, v[4:7], s[6:7] offset:24 glc
	s_waitcnt vmcnt(0)
	buffer_wbinvl1_vol
	v_cmp_ne_u64_e32 vcc, v[9:10], v[6:7]
	s_and_saveexec_b64 s[10:11], vcc
	s_cbranch_execz .LBB0_116
; %bb.113:
	s_mov_b64 s[12:13], 0
.LBB0_114:                              ; =>This Inner Loop Header: Depth=1
	s_sleep 1
	global_load_dwordx2 v[4:5], v3, s[6:7] offset:40
	global_load_dwordx2 v[11:12], v3, s[6:7]
	v_mov_b32_e32 v6, v9
	v_mov_b32_e32 v7, v10
	s_waitcnt vmcnt(1)
	v_and_b32_e32 v4, v4, v6
	s_waitcnt vmcnt(0)
	v_mad_u64_u32 v[8:9], s[14:15], v4, 24, v[11:12]
	v_and_b32_e32 v5, v5, v7
	v_mov_b32_e32 v4, v9
	v_mad_u64_u32 v[4:5], s[14:15], v5, 24, v[4:5]
	v_mov_b32_e32 v9, v4
	global_load_dwordx2 v[4:5], v[8:9], off glc
	s_waitcnt vmcnt(0)
	global_atomic_cmpswap_x2 v[9:10], v3, v[4:7], s[6:7] offset:24 glc
	s_waitcnt vmcnt(0)
	buffer_wbinvl1_vol
	v_cmp_eq_u64_e32 vcc, v[9:10], v[6:7]
	s_or_b64 s[12:13], vcc, s[12:13]
	s_andn2_b64 exec, exec, s[12:13]
	s_cbranch_execnz .LBB0_114
; %bb.115:
	s_or_b64 exec, exec, s[12:13]
.LBB0_116:
	s_or_b64 exec, exec, s[10:11]
.LBB0_117:
	s_or_b64 exec, exec, s[8:9]
	s_waitcnt vmcnt(0)
	v_mov_b32_e32 v3, 0
	global_load_dwordx2 v[11:12], v3, s[6:7] offset:40
	global_load_dwordx4 v[5:8], v3, s[6:7]
	v_readfirstlane_b32 s8, v9
	v_readfirstlane_b32 s9, v10
	s_mov_b64 s[10:11], exec
	s_waitcnt vmcnt(1)
	v_readfirstlane_b32 s12, v11
	v_readfirstlane_b32 s13, v12
	s_and_b64 s[12:13], s[8:9], s[12:13]
	s_mul_i32 s14, s13, 24
	s_mul_hi_u32 s15, s12, 24
	s_mul_i32 s16, s12, 24
	s_add_i32 s14, s15, s14
	v_mov_b32_e32 v4, s14
	s_waitcnt vmcnt(0)
	v_add_co_u32_e32 v9, vcc, s16, v5
	v_addc_co_u32_e32 v10, vcc, v6, v4, vcc
	s_and_saveexec_b64 s[14:15], s[4:5]
	s_cbranch_execz .LBB0_119
; %bb.118:
	v_mov_b32_e32 v12, s11
	v_mov_b32_e32 v11, s10
	v_mov_b32_e32 v13, 2
	v_mov_b32_e32 v14, 1
	global_store_dwordx4 v[9:10], v[11:14], off offset:8
.LBB0_119:
	s_or_b64 exec, exec, s[14:15]
	s_lshl_b64 s[10:11], s[12:13], 12
	v_mov_b32_e32 v4, s11
	v_add_co_u32_e32 v11, vcc, s10, v7
	v_addc_co_u32_e32 v12, vcc, v8, v4, vcc
	s_movk_i32 s10, 0xff1f
	v_and_or_b32 v1, v1, s10, 32
	s_mov_b32 s12, 0
	v_mov_b32_e32 v4, v3
	v_readfirstlane_b32 s10, v11
	v_readfirstlane_b32 s11, v12
	v_add_co_u32_e32 v7, vcc, v11, v30
	s_mov_b32 s13, s12
	s_mov_b32 s14, s12
	s_mov_b32 s15, s12
	s_nop 0
	global_store_dwordx4 v30, v[1:4], s[10:11]
	v_addc_co_u32_e32 v8, vcc, 0, v12, vcc
	v_mov_b32_e32 v1, s12
	v_mov_b32_e32 v2, s13
	v_mov_b32_e32 v3, s14
	v_mov_b32_e32 v4, s15
	global_store_dwordx4 v30, v[1:4], s[10:11] offset:16
	global_store_dwordx4 v30, v[1:4], s[10:11] offset:32
	global_store_dwordx4 v30, v[1:4], s[10:11] offset:48
	s_and_saveexec_b64 s[10:11], s[4:5]
	s_cbranch_execz .LBB0_127
; %bb.120:
	v_mov_b32_e32 v11, 0
	global_load_dwordx2 v[14:15], v11, s[6:7] offset:32 glc
	global_load_dwordx2 v[1:2], v11, s[6:7] offset:40
	v_mov_b32_e32 v12, s8
	v_mov_b32_e32 v13, s9
	s_waitcnt vmcnt(0)
	v_readfirstlane_b32 s12, v1
	v_readfirstlane_b32 s13, v2
	s_and_b64 s[12:13], s[12:13], s[8:9]
	s_mul_i32 s13, s13, 24
	s_mul_hi_u32 s14, s12, 24
	s_mul_i32 s12, s12, 24
	s_add_i32 s13, s14, s13
	v_mov_b32_e32 v1, s13
	v_add_co_u32_e32 v5, vcc, s12, v5
	v_addc_co_u32_e32 v6, vcc, v6, v1, vcc
	global_store_dwordx2 v[5:6], v[14:15], off
	s_waitcnt vmcnt(0)
	global_atomic_cmpswap_x2 v[3:4], v11, v[12:15], s[6:7] offset:32 glc
	s_waitcnt vmcnt(0)
	v_cmp_ne_u64_e32 vcc, v[3:4], v[14:15]
	s_and_saveexec_b64 s[12:13], vcc
	s_cbranch_execz .LBB0_123
; %bb.121:
	s_mov_b64 s[14:15], 0
.LBB0_122:                              ; =>This Inner Loop Header: Depth=1
	s_sleep 1
	global_store_dwordx2 v[5:6], v[3:4], off
	v_mov_b32_e32 v1, s8
	v_mov_b32_e32 v2, s9
	s_waitcnt vmcnt(0)
	global_atomic_cmpswap_x2 v[1:2], v11, v[1:4], s[6:7] offset:32 glc
	s_waitcnt vmcnt(0)
	v_cmp_eq_u64_e32 vcc, v[1:2], v[3:4]
	v_mov_b32_e32 v4, v2
	s_or_b64 s[14:15], vcc, s[14:15]
	v_mov_b32_e32 v3, v1
	s_andn2_b64 exec, exec, s[14:15]
	s_cbranch_execnz .LBB0_122
.LBB0_123:
	s_or_b64 exec, exec, s[12:13]
	v_mov_b32_e32 v4, 0
	global_load_dwordx2 v[1:2], v4, s[6:7] offset:16
	s_mov_b64 s[12:13], exec
	v_mbcnt_lo_u32_b32 v3, s12, 0
	v_mbcnt_hi_u32_b32 v3, s13, v3
	v_cmp_eq_u32_e32 vcc, 0, v3
	s_and_saveexec_b64 s[14:15], vcc
	s_cbranch_execz .LBB0_125
; %bb.124:
	s_bcnt1_i32_b64 s12, s[12:13]
	v_mov_b32_e32 v3, s12
	s_waitcnt vmcnt(0)
	global_atomic_add_x2 v[1:2], v[3:4], off offset:8
.LBB0_125:
	s_or_b64 exec, exec, s[14:15]
	s_waitcnt vmcnt(0)
	global_load_dwordx2 v[3:4], v[1:2], off offset:16
	s_waitcnt vmcnt(0)
	v_cmp_eq_u64_e32 vcc, 0, v[3:4]
	s_cbranch_vccnz .LBB0_127
; %bb.126:
	global_load_dword v1, v[1:2], off offset:24
	v_mov_b32_e32 v2, 0
	s_waitcnt vmcnt(0)
	global_store_dwordx2 v[3:4], v[1:2], off
	v_and_b32_e32 v1, 0xffffff, v1
	v_readfirstlane_b32 s12, v1
	s_mov_b32 m0, s12
	s_nop 0
	s_sendmsg sendmsg(MSG_INTERRUPT)
.LBB0_127:
	s_or_b64 exec, exec, s[10:11]
	s_branch .LBB0_131
.LBB0_128:                              ;   in Loop: Header=BB0_131 Depth=1
	s_or_b64 exec, exec, s[10:11]
	v_readfirstlane_b32 s10, v1
	s_cmp_eq_u32 s10, 0
	s_cbranch_scc1 .LBB0_130
; %bb.129:                              ;   in Loop: Header=BB0_131 Depth=1
	s_sleep 1
	s_cbranch_execnz .LBB0_131
	s_branch .LBB0_133
.LBB0_130:
	s_branch .LBB0_133
.LBB0_131:                              ; =>This Inner Loop Header: Depth=1
	v_mov_b32_e32 v1, 1
	s_and_saveexec_b64 s[10:11], s[4:5]
	s_cbranch_execz .LBB0_128
; %bb.132:                              ;   in Loop: Header=BB0_131 Depth=1
	global_load_dword v1, v[9:10], off offset:20 glc
	s_waitcnt vmcnt(0)
	buffer_wbinvl1_vol
	v_and_b32_e32 v1, 1, v1
	s_branch .LBB0_128
.LBB0_133:
	global_load_dwordx2 v[3:4], v[7:8], off
	s_and_saveexec_b64 s[10:11], s[4:5]
	s_cbranch_execz .LBB0_136
; %bb.134:
	v_mov_b32_e32 v9, 0
	global_load_dwordx2 v[1:2], v9, s[6:7] offset:40
	global_load_dwordx2 v[10:11], v9, s[6:7] offset:24 glc
	global_load_dwordx2 v[12:13], v9, s[6:7]
	v_mov_b32_e32 v6, s9
	s_mov_b64 s[4:5], 0
	s_waitcnt vmcnt(2)
	v_add_co_u32_e32 v7, vcc, 1, v1
	v_addc_co_u32_e32 v8, vcc, 0, v2, vcc
	v_add_co_u32_e32 v5, vcc, s8, v7
	v_addc_co_u32_e32 v6, vcc, v8, v6, vcc
	v_cmp_eq_u64_e32 vcc, 0, v[5:6]
	v_cndmask_b32_e32 v6, v6, v8, vcc
	v_cndmask_b32_e32 v5, v5, v7, vcc
	v_and_b32_e32 v2, v6, v2
	v_and_b32_e32 v1, v5, v1
	v_mul_lo_u32 v2, v2, 24
	v_mul_hi_u32 v8, v1, 24
	v_mul_lo_u32 v1, v1, 24
	s_waitcnt vmcnt(1)
	v_mov_b32_e32 v7, v10
	v_add_u32_e32 v2, v8, v2
	s_waitcnt vmcnt(0)
	v_add_co_u32_e32 v1, vcc, v12, v1
	v_addc_co_u32_e32 v2, vcc, v13, v2, vcc
	global_store_dwordx2 v[1:2], v[10:11], off
	v_mov_b32_e32 v8, v11
	s_waitcnt vmcnt(0)
	global_atomic_cmpswap_x2 v[7:8], v9, v[5:8], s[6:7] offset:24 glc
	s_waitcnt vmcnt(0)
	v_cmp_ne_u64_e32 vcc, v[7:8], v[10:11]
	s_and_b64 exec, exec, vcc
	s_cbranch_execz .LBB0_136
.LBB0_135:                              ; =>This Inner Loop Header: Depth=1
	s_sleep 1
	global_store_dwordx2 v[1:2], v[7:8], off
	s_waitcnt vmcnt(0)
	global_atomic_cmpswap_x2 v[10:11], v9, v[5:8], s[6:7] offset:24 glc
	s_waitcnt vmcnt(0)
	v_cmp_eq_u64_e32 vcc, v[10:11], v[7:8]
	v_mov_b32_e32 v7, v10
	s_or_b64 s[4:5], vcc, s[4:5]
	v_mov_b32_e32 v8, v11
	s_andn2_b64 exec, exec, s[4:5]
	s_cbranch_execnz .LBB0_135
.LBB0_136:
	s_or_b64 exec, exec, s[10:11]
.LBB0_137:
	v_readfirstlane_b32 s4, v31
	s_waitcnt vmcnt(0)
	v_mov_b32_e32 v1, 0
	v_mov_b32_e32 v2, 0
	v_cmp_eq_u32_e64 s[4:5], s4, v31
	s_and_saveexec_b64 s[8:9], s[4:5]
	s_cbranch_execz .LBB0_143
; %bb.138:
	v_mov_b32_e32 v5, 0
	global_load_dwordx2 v[8:9], v5, s[6:7] offset:24 glc
	s_waitcnt vmcnt(0)
	buffer_wbinvl1_vol
	global_load_dwordx2 v[1:2], v5, s[6:7] offset:40
	global_load_dwordx2 v[6:7], v5, s[6:7]
	s_waitcnt vmcnt(1)
	v_and_b32_e32 v1, v1, v8
	v_and_b32_e32 v2, v2, v9
	v_mul_lo_u32 v2, v2, 24
	v_mul_hi_u32 v10, v1, 24
	v_mul_lo_u32 v1, v1, 24
	v_add_u32_e32 v2, v10, v2
	s_waitcnt vmcnt(0)
	v_add_co_u32_e32 v1, vcc, v6, v1
	v_addc_co_u32_e32 v2, vcc, v7, v2, vcc
	global_load_dwordx2 v[6:7], v[1:2], off glc
	s_waitcnt vmcnt(0)
	global_atomic_cmpswap_x2 v[1:2], v5, v[6:9], s[6:7] offset:24 glc
	s_waitcnt vmcnt(0)
	buffer_wbinvl1_vol
	v_cmp_ne_u64_e32 vcc, v[1:2], v[8:9]
	s_and_saveexec_b64 s[10:11], vcc
	s_cbranch_execz .LBB0_142
; %bb.139:
	s_mov_b64 s[12:13], 0
.LBB0_140:                              ; =>This Inner Loop Header: Depth=1
	s_sleep 1
	global_load_dwordx2 v[6:7], v5, s[6:7] offset:40
	global_load_dwordx2 v[10:11], v5, s[6:7]
	v_mov_b32_e32 v9, v2
	v_mov_b32_e32 v8, v1
	s_waitcnt vmcnt(1)
	v_and_b32_e32 v1, v6, v8
	s_waitcnt vmcnt(0)
	v_mad_u64_u32 v[1:2], s[14:15], v1, 24, v[10:11]
	v_and_b32_e32 v6, v7, v9
	v_mad_u64_u32 v[6:7], s[14:15], v6, 24, v[2:3]
	v_mov_b32_e32 v2, v6
	global_load_dwordx2 v[6:7], v[1:2], off glc
	s_waitcnt vmcnt(0)
	global_atomic_cmpswap_x2 v[1:2], v5, v[6:9], s[6:7] offset:24 glc
	s_waitcnt vmcnt(0)
	buffer_wbinvl1_vol
	v_cmp_eq_u64_e32 vcc, v[1:2], v[8:9]
	s_or_b64 s[12:13], vcc, s[12:13]
	s_andn2_b64 exec, exec, s[12:13]
	s_cbranch_execnz .LBB0_140
; %bb.141:
	s_or_b64 exec, exec, s[12:13]
.LBB0_142:
	s_or_b64 exec, exec, s[10:11]
.LBB0_143:
	s_or_b64 exec, exec, s[8:9]
	v_mov_b32_e32 v11, 0
	global_load_dwordx2 v[5:6], v11, s[6:7] offset:40
	global_load_dwordx4 v[7:10], v11, s[6:7]
	v_readfirstlane_b32 s8, v1
	v_readfirstlane_b32 s9, v2
	s_mov_b64 s[10:11], exec
	s_waitcnt vmcnt(1)
	v_readfirstlane_b32 s12, v5
	v_readfirstlane_b32 s13, v6
	s_and_b64 s[12:13], s[8:9], s[12:13]
	s_mul_i32 s14, s13, 24
	s_mul_hi_u32 s15, s12, 24
	s_mul_i32 s16, s12, 24
	s_add_i32 s14, s15, s14
	v_mov_b32_e32 v1, s14
	s_waitcnt vmcnt(0)
	v_add_co_u32_e32 v11, vcc, s16, v7
	v_addc_co_u32_e32 v12, vcc, v8, v1, vcc
	s_and_saveexec_b64 s[14:15], s[4:5]
	s_cbranch_execz .LBB0_145
; %bb.144:
	v_mov_b32_e32 v14, s11
	v_mov_b32_e32 v13, s10
	v_mov_b32_e32 v15, 2
	v_mov_b32_e32 v16, 1
	global_store_dwordx4 v[11:12], v[13:16], off offset:8
.LBB0_145:
	s_or_b64 exec, exec, s[14:15]
	s_lshl_b64 s[10:11], s[12:13], 12
	v_cvt_f64_f32_e32 v[5:6], v0
	v_mov_b32_e32 v1, s11
	v_add_co_u32_e32 v2, vcc, s10, v9
	v_addc_co_u32_e32 v1, vcc, v10, v1, vcc
	s_movk_i32 s10, 0xff1d
	v_and_or_b32 v3, v3, s10, 34
	v_readfirstlane_b32 s10, v2
	v_readfirstlane_b32 s11, v1
	s_mov_b32 s12, 0
	s_mov_b32 s13, s12
	s_mov_b32 s14, s12
	s_mov_b32 s15, s12
	s_nop 0
	global_store_dwordx4 v30, v[3:6], s[10:11]
	v_mov_b32_e32 v0, s12
	v_mov_b32_e32 v1, s13
	v_mov_b32_e32 v2, s14
	v_mov_b32_e32 v3, s15
	global_store_dwordx4 v30, v[0:3], s[10:11] offset:16
	global_store_dwordx4 v30, v[0:3], s[10:11] offset:32
	global_store_dwordx4 v30, v[0:3], s[10:11] offset:48
	s_and_saveexec_b64 s[10:11], s[4:5]
	s_cbranch_execz .LBB0_153
; %bb.146:
	v_mov_b32_e32 v6, 0
	global_load_dwordx2 v[15:16], v6, s[6:7] offset:32 glc
	global_load_dwordx2 v[0:1], v6, s[6:7] offset:40
	v_mov_b32_e32 v13, s8
	v_mov_b32_e32 v14, s9
	s_waitcnt vmcnt(0)
	v_readfirstlane_b32 s12, v0
	v_readfirstlane_b32 s13, v1
	s_and_b64 s[12:13], s[12:13], s[8:9]
	s_mul_i32 s13, s13, 24
	s_mul_hi_u32 s14, s12, 24
	s_mul_i32 s12, s12, 24
	s_add_i32 s13, s14, s13
	v_mov_b32_e32 v0, s13
	v_add_co_u32_e32 v4, vcc, s12, v7
	v_addc_co_u32_e32 v5, vcc, v8, v0, vcc
	global_store_dwordx2 v[4:5], v[15:16], off
	s_waitcnt vmcnt(0)
	global_atomic_cmpswap_x2 v[2:3], v6, v[13:16], s[6:7] offset:32 glc
	s_waitcnt vmcnt(0)
	v_cmp_ne_u64_e32 vcc, v[2:3], v[15:16]
	s_and_saveexec_b64 s[12:13], vcc
	s_cbranch_execz .LBB0_149
; %bb.147:
	s_mov_b64 s[14:15], 0
.LBB0_148:                              ; =>This Inner Loop Header: Depth=1
	s_sleep 1
	global_store_dwordx2 v[4:5], v[2:3], off
	v_mov_b32_e32 v0, s8
	v_mov_b32_e32 v1, s9
	s_waitcnt vmcnt(0)
	global_atomic_cmpswap_x2 v[0:1], v6, v[0:3], s[6:7] offset:32 glc
	s_waitcnt vmcnt(0)
	v_cmp_eq_u64_e32 vcc, v[0:1], v[2:3]
	v_mov_b32_e32 v3, v1
	s_or_b64 s[14:15], vcc, s[14:15]
	v_mov_b32_e32 v2, v0
	s_andn2_b64 exec, exec, s[14:15]
	s_cbranch_execnz .LBB0_148
.LBB0_149:
	s_or_b64 exec, exec, s[12:13]
	v_mov_b32_e32 v3, 0
	global_load_dwordx2 v[0:1], v3, s[6:7] offset:16
	s_mov_b64 s[12:13], exec
	v_mbcnt_lo_u32_b32 v2, s12, 0
	v_mbcnt_hi_u32_b32 v2, s13, v2
	v_cmp_eq_u32_e32 vcc, 0, v2
	s_and_saveexec_b64 s[14:15], vcc
	s_cbranch_execz .LBB0_151
; %bb.150:
	s_bcnt1_i32_b64 s12, s[12:13]
	v_mov_b32_e32 v2, s12
	s_waitcnt vmcnt(0)
	global_atomic_add_x2 v[0:1], v[2:3], off offset:8
.LBB0_151:
	s_or_b64 exec, exec, s[14:15]
	s_waitcnt vmcnt(0)
	global_load_dwordx2 v[2:3], v[0:1], off offset:16
	s_waitcnt vmcnt(0)
	v_cmp_eq_u64_e32 vcc, 0, v[2:3]
	s_cbranch_vccnz .LBB0_153
; %bb.152:
	global_load_dword v0, v[0:1], off offset:24
	v_mov_b32_e32 v1, 0
	s_waitcnt vmcnt(0)
	global_store_dwordx2 v[2:3], v[0:1], off
	v_and_b32_e32 v0, 0xffffff, v0
	v_readfirstlane_b32 s12, v0
	s_mov_b32 m0, s12
	s_nop 0
	s_sendmsg sendmsg(MSG_INTERRUPT)
.LBB0_153:
	s_or_b64 exec, exec, s[10:11]
	s_branch .LBB0_157
.LBB0_154:                              ;   in Loop: Header=BB0_157 Depth=1
	s_or_b64 exec, exec, s[10:11]
	v_readfirstlane_b32 s10, v0
	s_cmp_eq_u32 s10, 0
	s_cbranch_scc1 .LBB0_156
; %bb.155:                              ;   in Loop: Header=BB0_157 Depth=1
	s_sleep 1
	s_cbranch_execnz .LBB0_157
	s_branch .LBB0_159
.LBB0_156:
	s_branch .LBB0_159
.LBB0_157:                              ; =>This Inner Loop Header: Depth=1
	v_mov_b32_e32 v0, 1
	s_and_saveexec_b64 s[10:11], s[4:5]
	s_cbranch_execz .LBB0_154
; %bb.158:                              ;   in Loop: Header=BB0_157 Depth=1
	global_load_dword v0, v[11:12], off offset:20 glc
	s_waitcnt vmcnt(0)
	buffer_wbinvl1_vol
	v_and_b32_e32 v0, 1, v0
	s_branch .LBB0_154
.LBB0_159:
	s_and_saveexec_b64 s[10:11], s[4:5]
	s_cbranch_execz .LBB0_162
; %bb.160:
	v_mov_b32_e32 v6, 0
	global_load_dwordx2 v[2:3], v6, s[6:7] offset:40
	global_load_dwordx2 v[7:8], v6, s[6:7] offset:24 glc
	global_load_dwordx2 v[4:5], v6, s[6:7]
	v_mov_b32_e32 v1, s9
	s_mov_b64 s[4:5], 0
	s_waitcnt vmcnt(2)
	v_add_co_u32_e32 v9, vcc, 1, v2
	v_addc_co_u32_e32 v10, vcc, 0, v3, vcc
	v_add_co_u32_e32 v0, vcc, s8, v9
	v_addc_co_u32_e32 v1, vcc, v10, v1, vcc
	v_cmp_eq_u64_e32 vcc, 0, v[0:1]
	v_cndmask_b32_e32 v1, v1, v10, vcc
	v_cndmask_b32_e32 v0, v0, v9, vcc
	v_and_b32_e32 v3, v1, v3
	v_and_b32_e32 v2, v0, v2
	v_mul_lo_u32 v3, v3, 24
	v_mul_hi_u32 v9, v2, 24
	v_mul_lo_u32 v10, v2, 24
	s_waitcnt vmcnt(1)
	v_mov_b32_e32 v2, v7
	v_add_u32_e32 v3, v9, v3
	s_waitcnt vmcnt(0)
	v_add_co_u32_e32 v4, vcc, v4, v10
	v_addc_co_u32_e32 v5, vcc, v5, v3, vcc
	global_store_dwordx2 v[4:5], v[7:8], off
	v_mov_b32_e32 v3, v8
	s_waitcnt vmcnt(0)
	global_atomic_cmpswap_x2 v[2:3], v6, v[0:3], s[6:7] offset:24 glc
	s_waitcnt vmcnt(0)
	v_cmp_ne_u64_e32 vcc, v[2:3], v[7:8]
	s_and_b64 exec, exec, vcc
	s_cbranch_execz .LBB0_162
.LBB0_161:                              ; =>This Inner Loop Header: Depth=1
	s_sleep 1
	global_store_dwordx2 v[4:5], v[2:3], off
	s_waitcnt vmcnt(0)
	global_atomic_cmpswap_x2 v[7:8], v6, v[0:3], s[6:7] offset:24 glc
	s_waitcnt vmcnt(0)
	v_cmp_eq_u64_e32 vcc, v[7:8], v[2:3]
	v_mov_b32_e32 v2, v7
	s_or_b64 s[4:5], vcc, s[4:5]
	v_mov_b32_e32 v3, v8
	s_andn2_b64 exec, exec, s[4:5]
	s_cbranch_execnz .LBB0_161
.LBB0_162:
	s_or_b64 exec, exec, s[10:11]
	s_waitcnt vmcnt(0) lgkmcnt(0)
	s_setpc_b64 s[30:31]
.Lfunc_end0:
	.size	_Z1Af, .Lfunc_end0-_Z1Af
                                        ; -- End function
	.set .L_Z1Af.num_vgpr, 34
	.set .L_Z1Af.num_agpr, 0
	.set .L_Z1Af.numbered_sgpr, 32
	.set .L_Z1Af.private_seg_size, 0
	.set .L_Z1Af.uses_vcc, 1
	.set .L_Z1Af.uses_flat_scratch, 0
	.set .L_Z1Af.has_dyn_sized_stack, 0
	.set .L_Z1Af.has_recursion, 0
	.set .L_Z1Af.has_indirect_call, 0
	.section	.AMDGPU.csdata,"",@progbits
; Function info:
; codeLenInByte = 5816
; TotalNumSgprs: 36
; NumVgprs: 34
; NumAgprs: 0
; TotalNumVgprs: 34
; ScratchSize: 0
; MemoryBound: 0
	.text
	.p2align	2                               ; -- Begin function _Z1Bf
	.type	_Z1Bf,@function
_Z1Bf:                                  ; @_Z1Bf
; %bb.0:
	s_waitcnt vmcnt(0) expcnt(0) lgkmcnt(0)
	s_load_dwordx2 s[6:7], s[8:9], 0x50
	v_mbcnt_lo_u32_b32 v1, -1, 0
	v_mbcnt_hi_u32_b32 v31, -1, v1
	v_readfirstlane_b32 s4, v31
	v_mov_b32_e32 v6, 0
	v_mov_b32_e32 v7, 0
	v_cmp_eq_u32_e64 s[4:5], s4, v31
	s_and_saveexec_b64 s[8:9], s[4:5]
	s_cbranch_execz .LBB1_6
; %bb.1:
	v_mov_b32_e32 v1, 0
	s_waitcnt lgkmcnt(0)
	global_load_dwordx2 v[4:5], v1, s[6:7] offset:24 glc
	s_waitcnt vmcnt(0)
	buffer_wbinvl1_vol
	global_load_dwordx2 v[2:3], v1, s[6:7] offset:40
	global_load_dwordx2 v[6:7], v1, s[6:7]
	s_waitcnt vmcnt(1)
	v_and_b32_e32 v2, v2, v4
	v_and_b32_e32 v3, v3, v5
	v_mul_lo_u32 v3, v3, 24
	v_mul_hi_u32 v8, v2, 24
	v_mul_lo_u32 v2, v2, 24
	v_add_u32_e32 v3, v8, v3
	s_waitcnt vmcnt(0)
	v_add_co_u32_e32 v2, vcc, v6, v2
	v_addc_co_u32_e32 v3, vcc, v7, v3, vcc
	global_load_dwordx2 v[2:3], v[2:3], off glc
	s_waitcnt vmcnt(0)
	global_atomic_cmpswap_x2 v[6:7], v1, v[2:5], s[6:7] offset:24 glc
	s_waitcnt vmcnt(0)
	buffer_wbinvl1_vol
	v_cmp_ne_u64_e32 vcc, v[6:7], v[4:5]
	s_and_saveexec_b64 s[10:11], vcc
	s_cbranch_execz .LBB1_5
; %bb.2:
	s_mov_b64 s[12:13], 0
.LBB1_3:                                ; =>This Inner Loop Header: Depth=1
	s_sleep 1
	global_load_dwordx2 v[2:3], v1, s[6:7] offset:40
	global_load_dwordx2 v[8:9], v1, s[6:7]
	v_mov_b32_e32 v4, v6
	v_mov_b32_e32 v5, v7
	s_waitcnt vmcnt(1)
	v_and_b32_e32 v2, v2, v4
	s_waitcnt vmcnt(0)
	v_mad_u64_u32 v[6:7], s[14:15], v2, 24, v[8:9]
	v_and_b32_e32 v3, v3, v5
	v_mov_b32_e32 v2, v7
	v_mad_u64_u32 v[2:3], s[14:15], v3, 24, v[2:3]
	v_mov_b32_e32 v7, v2
	global_load_dwordx2 v[2:3], v[6:7], off glc
	s_waitcnt vmcnt(0)
	global_atomic_cmpswap_x2 v[6:7], v1, v[2:5], s[6:7] offset:24 glc
	s_waitcnt vmcnt(0)
	buffer_wbinvl1_vol
	v_cmp_eq_u64_e32 vcc, v[6:7], v[4:5]
	s_or_b64 s[12:13], vcc, s[12:13]
	s_andn2_b64 exec, exec, s[12:13]
	s_cbranch_execnz .LBB1_3
; %bb.4:
	s_or_b64 exec, exec, s[12:13]
.LBB1_5:
	s_or_b64 exec, exec, s[10:11]
.LBB1_6:
	s_or_b64 exec, exec, s[8:9]
	v_mov_b32_e32 v5, 0
	s_waitcnt lgkmcnt(0)
	global_load_dwordx2 v[8:9], v5, s[6:7] offset:40
	global_load_dwordx4 v[1:4], v5, s[6:7]
	v_readfirstlane_b32 s8, v6
	v_readfirstlane_b32 s9, v7
	s_mov_b64 s[10:11], exec
	s_waitcnt vmcnt(1)
	v_readfirstlane_b32 s12, v8
	v_readfirstlane_b32 s13, v9
	s_and_b64 s[12:13], s[8:9], s[12:13]
	s_mul_i32 s14, s13, 24
	s_mul_hi_u32 s15, s12, 24
	s_mul_i32 s16, s12, 24
	s_add_i32 s14, s15, s14
	v_mov_b32_e32 v6, s14
	s_waitcnt vmcnt(0)
	v_add_co_u32_e32 v8, vcc, s16, v1
	v_addc_co_u32_e32 v9, vcc, v2, v6, vcc
	s_and_saveexec_b64 s[14:15], s[4:5]
	s_cbranch_execz .LBB1_8
; %bb.7:
	v_mov_b32_e32 v10, s10
	v_mov_b32_e32 v11, s11
	v_mov_b32_e32 v12, 2
	v_mov_b32_e32 v13, 1
	global_store_dwordx4 v[8:9], v[10:13], off offset:8
.LBB1_8:
	s_or_b64 exec, exec, s[14:15]
	s_lshl_b64 s[10:11], s[12:13], 12
	v_mov_b32_e32 v6, s11
	v_add_co_u32_e32 v3, vcc, s10, v3
	v_addc_co_u32_e32 v12, vcc, v4, v6, vcc
	s_mov_b32 s12, 0
	v_lshlrev_b32_e32 v30, 6, v31
	v_mov_b32_e32 v4, 33
	v_mov_b32_e32 v6, v5
	v_mov_b32_e32 v7, v5
	v_readfirstlane_b32 s10, v3
	v_readfirstlane_b32 s11, v12
	v_add_co_u32_e32 v10, vcc, v3, v30
	s_mov_b32 s13, s12
	s_mov_b32 s14, s12
	s_mov_b32 s15, s12
	s_nop 0
	global_store_dwordx4 v30, v[4:7], s[10:11]
	v_mov_b32_e32 v3, s12
	v_addc_co_u32_e32 v11, vcc, 0, v12, vcc
	v_mov_b32_e32 v4, s13
	v_mov_b32_e32 v5, s14
	v_mov_b32_e32 v6, s15
	global_store_dwordx4 v30, v[3:6], s[10:11] offset:16
	global_store_dwordx4 v30, v[3:6], s[10:11] offset:32
	global_store_dwordx4 v30, v[3:6], s[10:11] offset:48
	s_and_saveexec_b64 s[10:11], s[4:5]
	s_cbranch_execz .LBB1_16
; %bb.9:
	v_mov_b32_e32 v7, 0
	global_load_dwordx2 v[14:15], v7, s[6:7] offset:32 glc
	global_load_dwordx2 v[3:4], v7, s[6:7] offset:40
	v_mov_b32_e32 v12, s8
	v_mov_b32_e32 v13, s9
	s_waitcnt vmcnt(0)
	v_and_b32_e32 v3, s8, v3
	v_and_b32_e32 v4, s9, v4
	v_mul_lo_u32 v4, v4, 24
	v_mul_hi_u32 v5, v3, 24
	v_mul_lo_u32 v3, v3, 24
	v_add_u32_e32 v4, v5, v4
	v_add_co_u32_e32 v5, vcc, v1, v3
	v_addc_co_u32_e32 v6, vcc, v2, v4, vcc
	global_store_dwordx2 v[5:6], v[14:15], off
	s_waitcnt vmcnt(0)
	global_atomic_cmpswap_x2 v[3:4], v7, v[12:15], s[6:7] offset:32 glc
	s_waitcnt vmcnt(0)
	v_cmp_ne_u64_e32 vcc, v[3:4], v[14:15]
	s_and_saveexec_b64 s[12:13], vcc
	s_cbranch_execz .LBB1_12
; %bb.10:
	s_mov_b64 s[14:15], 0
.LBB1_11:                               ; =>This Inner Loop Header: Depth=1
	s_sleep 1
	global_store_dwordx2 v[5:6], v[3:4], off
	v_mov_b32_e32 v1, s8
	v_mov_b32_e32 v2, s9
	s_waitcnt vmcnt(0)
	global_atomic_cmpswap_x2 v[1:2], v7, v[1:4], s[6:7] offset:32 glc
	s_waitcnt vmcnt(0)
	v_cmp_eq_u64_e32 vcc, v[1:2], v[3:4]
	v_mov_b32_e32 v4, v2
	s_or_b64 s[14:15], vcc, s[14:15]
	v_mov_b32_e32 v3, v1
	s_andn2_b64 exec, exec, s[14:15]
	s_cbranch_execnz .LBB1_11
.LBB1_12:
	s_or_b64 exec, exec, s[12:13]
	v_mov_b32_e32 v4, 0
	global_load_dwordx2 v[1:2], v4, s[6:7] offset:16
	s_mov_b64 s[12:13], exec
	v_mbcnt_lo_u32_b32 v3, s12, 0
	v_mbcnt_hi_u32_b32 v3, s13, v3
	v_cmp_eq_u32_e32 vcc, 0, v3
	s_and_saveexec_b64 s[14:15], vcc
	s_cbranch_execz .LBB1_14
; %bb.13:
	s_bcnt1_i32_b64 s12, s[12:13]
	v_mov_b32_e32 v3, s12
	s_waitcnt vmcnt(0)
	global_atomic_add_x2 v[1:2], v[3:4], off offset:8
.LBB1_14:
	s_or_b64 exec, exec, s[14:15]
	s_waitcnt vmcnt(0)
	global_load_dwordx2 v[3:4], v[1:2], off offset:16
	s_waitcnt vmcnt(0)
	v_cmp_eq_u64_e32 vcc, 0, v[3:4]
	s_cbranch_vccnz .LBB1_16
; %bb.15:
	global_load_dword v1, v[1:2], off offset:24
	v_mov_b32_e32 v2, 0
	s_waitcnt vmcnt(0)
	global_store_dwordx2 v[3:4], v[1:2], off
	v_and_b32_e32 v1, 0xffffff, v1
	v_readfirstlane_b32 s12, v1
	s_mov_b32 m0, s12
	s_nop 0
	s_sendmsg sendmsg(MSG_INTERRUPT)
.LBB1_16:
	s_or_b64 exec, exec, s[10:11]
	s_branch .LBB1_20
.LBB1_17:                               ;   in Loop: Header=BB1_20 Depth=1
	s_or_b64 exec, exec, s[10:11]
	v_readfirstlane_b32 s10, v1
	s_cmp_eq_u32 s10, 0
	s_cbranch_scc1 .LBB1_19
; %bb.18:                               ;   in Loop: Header=BB1_20 Depth=1
	s_sleep 1
	s_cbranch_execnz .LBB1_20
	s_branch .LBB1_22
.LBB1_19:
	s_branch .LBB1_22
.LBB1_20:                               ; =>This Inner Loop Header: Depth=1
	v_mov_b32_e32 v1, 1
	s_and_saveexec_b64 s[10:11], s[4:5]
	s_cbranch_execz .LBB1_17
; %bb.21:                               ;   in Loop: Header=BB1_20 Depth=1
	global_load_dword v1, v[8:9], off offset:20 glc
	s_waitcnt vmcnt(0)
	buffer_wbinvl1_vol
	v_and_b32_e32 v1, 1, v1
	s_branch .LBB1_17
.LBB1_22:
	global_load_dwordx2 v[1:2], v[10:11], off
	s_and_saveexec_b64 s[10:11], s[4:5]
	s_cbranch_execz .LBB1_25
; %bb.23:
	v_mov_b32_e32 v9, 0
	global_load_dwordx2 v[5:6], v9, s[6:7] offset:40
	global_load_dwordx2 v[10:11], v9, s[6:7] offset:24 glc
	global_load_dwordx2 v[7:8], v9, s[6:7]
	v_mov_b32_e32 v4, s9
	s_mov_b64 s[4:5], 0
	s_waitcnt vmcnt(2)
	v_add_co_u32_e32 v12, vcc, 1, v5
	v_addc_co_u32_e32 v13, vcc, 0, v6, vcc
	v_add_co_u32_e32 v3, vcc, s8, v12
	v_addc_co_u32_e32 v4, vcc, v13, v4, vcc
	v_cmp_eq_u64_e32 vcc, 0, v[3:4]
	v_cndmask_b32_e32 v4, v4, v13, vcc
	v_cndmask_b32_e32 v3, v3, v12, vcc
	v_and_b32_e32 v6, v4, v6
	v_and_b32_e32 v5, v3, v5
	v_mul_lo_u32 v6, v6, 24
	v_mul_hi_u32 v12, v5, 24
	v_mul_lo_u32 v13, v5, 24
	s_waitcnt vmcnt(1)
	v_mov_b32_e32 v5, v10
	v_add_u32_e32 v6, v12, v6
	s_waitcnt vmcnt(0)
	v_add_co_u32_e32 v7, vcc, v7, v13
	v_addc_co_u32_e32 v8, vcc, v8, v6, vcc
	global_store_dwordx2 v[7:8], v[10:11], off
	v_mov_b32_e32 v6, v11
	s_waitcnt vmcnt(0)
	global_atomic_cmpswap_x2 v[5:6], v9, v[3:6], s[6:7] offset:24 glc
	s_waitcnt vmcnt(0)
	v_cmp_ne_u64_e32 vcc, v[5:6], v[10:11]
	s_and_b64 exec, exec, vcc
	s_cbranch_execz .LBB1_25
.LBB1_24:                               ; =>This Inner Loop Header: Depth=1
	s_sleep 1
	global_store_dwordx2 v[7:8], v[5:6], off
	s_waitcnt vmcnt(0)
	global_atomic_cmpswap_x2 v[10:11], v9, v[3:6], s[6:7] offset:24 glc
	s_waitcnt vmcnt(0)
	v_cmp_eq_u64_e32 vcc, v[10:11], v[5:6]
	v_mov_b32_e32 v5, v10
	s_or_b64 s[4:5], vcc, s[4:5]
	v_mov_b32_e32 v6, v11
	s_andn2_b64 exec, exec, s[4:5]
	s_cbranch_execnz .LBB1_24
.LBB1_25:
	s_or_b64 exec, exec, s[10:11]
	s_getpc_b64 s[8:9]
	s_add_u32 s8, s8, .str.1@rel32@lo+4
	s_addc_u32 s9, s9, .str.1@rel32@hi+12
	s_cmp_lg_u64 s[8:9], 0
	s_cbranch_scc0 .LBB1_110
; %bb.26:
	s_waitcnt vmcnt(0)
	v_and_b32_e32 v32, 2, v1
	v_mov_b32_e32 v27, 0
	v_and_b32_e32 v3, -3, v1
	v_mov_b32_e32 v4, v2
	s_mov_b64 s[10:11], 14
	v_mov_b32_e32 v9, 2
	v_mov_b32_e32 v10, 1
	s_branch .LBB1_28
.LBB1_27:                               ;   in Loop: Header=BB1_28 Depth=1
	s_or_b64 exec, exec, s[16:17]
	s_sub_u32 s10, s10, s12
	s_subb_u32 s11, s11, s13
	s_add_u32 s8, s8, s12
	s_addc_u32 s9, s9, s13
	s_cmp_lg_u64 s[10:11], 0
	s_cbranch_scc0 .LBB1_109
.LBB1_28:                               ; =>This Loop Header: Depth=1
                                        ;     Child Loop BB1_31 Depth 2
                                        ;     Child Loop BB1_37 Depth 2
                                        ;     Child Loop BB1_46 Depth 2
                                        ;     Child Loop BB1_54 Depth 2
                                        ;     Child Loop BB1_62 Depth 2
                                        ;     Child Loop BB1_70 Depth 2
                                        ;     Child Loop BB1_78 Depth 2
                                        ;     Child Loop BB1_86 Depth 2
                                        ;     Child Loop BB1_94 Depth 2
                                        ;     Child Loop BB1_103 Depth 2
                                        ;     Child Loop BB1_108 Depth 2
	v_cmp_lt_u64_e64 s[4:5], s[10:11], 56
	v_cmp_gt_u64_e64 s[14:15], s[10:11], 7
	s_and_b64 s[4:5], s[4:5], exec
	s_cselect_b32 s13, s11, 0
	s_cselect_b32 s12, s10, 56
	s_and_b64 vcc, exec, s[14:15]
	s_cbranch_vccnz .LBB1_38
; %bb.29:                               ;   in Loop: Header=BB1_28 Depth=1
	v_mov_b32_e32 v5, 0
	s_cmp_eq_u64 s[10:11], 0
	v_mov_b32_e32 v6, 0
	s_mov_b64 s[4:5], 0
	s_cbranch_scc1 .LBB1_32
; %bb.30:                               ;   in Loop: Header=BB1_28 Depth=1
	v_mov_b32_e32 v5, 0
	s_lshl_b64 s[14:15], s[12:13], 3
	s_mov_b64 s[16:17], 0
	v_mov_b32_e32 v6, 0
	s_mov_b64 s[18:19], s[8:9]
.LBB1_31:                               ;   Parent Loop BB1_28 Depth=1
                                        ; =>  This Inner Loop Header: Depth=2
	global_load_ubyte v7, v27, s[18:19]
	s_waitcnt vmcnt(0)
	v_and_b32_e32 v26, 0xffff, v7
	v_lshlrev_b64 v[7:8], s16, v[26:27]
	s_add_u32 s16, s16, 8
	s_addc_u32 s17, s17, 0
	s_add_u32 s18, s18, 1
	s_addc_u32 s19, s19, 0
	v_or_b32_e32 v5, v7, v5
	s_cmp_lg_u32 s14, s16
	v_or_b32_e32 v6, v8, v6
	s_cbranch_scc1 .LBB1_31
.LBB1_32:                               ;   in Loop: Header=BB1_28 Depth=1
	s_mov_b32 s18, 0
	s_andn2_b64 vcc, exec, s[4:5]
	s_mov_b64 s[4:5], s[8:9]
	s_cbranch_vccnz .LBB1_34
.LBB1_33:                               ;   in Loop: Header=BB1_28 Depth=1
	global_load_dwordx2 v[5:6], v27, s[8:9]
	s_add_i32 s18, s12, -8
	s_add_u32 s4, s8, 8
	s_addc_u32 s5, s9, 0
.LBB1_34:                               ;   in Loop: Header=BB1_28 Depth=1
	s_cmp_gt_u32 s18, 7
	s_cbranch_scc1 .LBB1_39
; %bb.35:                               ;   in Loop: Header=BB1_28 Depth=1
	s_cmp_eq_u32 s18, 0
	s_cbranch_scc1 .LBB1_40
; %bb.36:                               ;   in Loop: Header=BB1_28 Depth=1
	v_mov_b32_e32 v11, 0
	s_mov_b64 s[14:15], 0
	v_mov_b32_e32 v12, 0
	s_mov_b64 s[16:17], 0
.LBB1_37:                               ;   Parent Loop BB1_28 Depth=1
                                        ; =>  This Inner Loop Header: Depth=2
	s_add_u32 s20, s4, s16
	s_addc_u32 s21, s5, s17
	global_load_ubyte v7, v27, s[20:21]
	s_add_u32 s16, s16, 1
	s_addc_u32 s17, s17, 0
	s_waitcnt vmcnt(0)
	v_and_b32_e32 v26, 0xffff, v7
	v_lshlrev_b64 v[7:8], s14, v[26:27]
	s_add_u32 s14, s14, 8
	s_addc_u32 s15, s15, 0
	v_or_b32_e32 v11, v7, v11
	s_cmp_lg_u32 s18, s16
	v_or_b32_e32 v12, v8, v12
	s_cbranch_scc1 .LBB1_37
	s_branch .LBB1_41
.LBB1_38:                               ;   in Loop: Header=BB1_28 Depth=1
	s_mov_b32 s18, 0
	s_mov_b64 s[4:5], s[8:9]
	s_branch .LBB1_33
.LBB1_39:                               ;   in Loop: Header=BB1_28 Depth=1
                                        ; implicit-def: $vgpr11_vgpr12
	s_mov_b32 s19, 0
	s_branch .LBB1_42
.LBB1_40:                               ;   in Loop: Header=BB1_28 Depth=1
	v_mov_b32_e32 v11, 0
	v_mov_b32_e32 v12, 0
.LBB1_41:                               ;   in Loop: Header=BB1_28 Depth=1
	s_mov_b32 s19, 0
	s_cbranch_execnz .LBB1_43
.LBB1_42:                               ;   in Loop: Header=BB1_28 Depth=1
	global_load_dwordx2 v[11:12], v27, s[4:5]
	s_add_i32 s19, s18, -8
	s_add_u32 s4, s4, 8
	s_addc_u32 s5, s5, 0
.LBB1_43:                               ;   in Loop: Header=BB1_28 Depth=1
	s_cmp_gt_u32 s19, 7
	s_cbranch_scc1 .LBB1_47
; %bb.44:                               ;   in Loop: Header=BB1_28 Depth=1
	s_cmp_eq_u32 s19, 0
	s_cbranch_scc1 .LBB1_48
; %bb.45:                               ;   in Loop: Header=BB1_28 Depth=1
	v_mov_b32_e32 v13, 0
	s_mov_b64 s[14:15], 0
	v_mov_b32_e32 v14, 0
	s_mov_b64 s[16:17], 0
.LBB1_46:                               ;   Parent Loop BB1_28 Depth=1
                                        ; =>  This Inner Loop Header: Depth=2
	s_add_u32 s20, s4, s16
	s_addc_u32 s21, s5, s17
	global_load_ubyte v7, v27, s[20:21]
	s_add_u32 s16, s16, 1
	s_addc_u32 s17, s17, 0
	s_waitcnt vmcnt(0)
	v_and_b32_e32 v26, 0xffff, v7
	v_lshlrev_b64 v[7:8], s14, v[26:27]
	s_add_u32 s14, s14, 8
	s_addc_u32 s15, s15, 0
	v_or_b32_e32 v13, v7, v13
	s_cmp_lg_u32 s19, s16
	v_or_b32_e32 v14, v8, v14
	s_cbranch_scc1 .LBB1_46
	s_branch .LBB1_49
.LBB1_47:                               ;   in Loop: Header=BB1_28 Depth=1
	s_mov_b32 s18, 0
	s_branch .LBB1_50
.LBB1_48:                               ;   in Loop: Header=BB1_28 Depth=1
	v_mov_b32_e32 v13, 0
	v_mov_b32_e32 v14, 0
.LBB1_49:                               ;   in Loop: Header=BB1_28 Depth=1
	s_mov_b32 s18, 0
	s_cbranch_execnz .LBB1_51
.LBB1_50:                               ;   in Loop: Header=BB1_28 Depth=1
	global_load_dwordx2 v[13:14], v27, s[4:5]
	s_add_i32 s18, s19, -8
	s_add_u32 s4, s4, 8
	s_addc_u32 s5, s5, 0
.LBB1_51:                               ;   in Loop: Header=BB1_28 Depth=1
	s_cmp_gt_u32 s18, 7
	s_cbranch_scc1 .LBB1_55
; %bb.52:                               ;   in Loop: Header=BB1_28 Depth=1
	s_cmp_eq_u32 s18, 0
	s_cbranch_scc1 .LBB1_56
; %bb.53:                               ;   in Loop: Header=BB1_28 Depth=1
	v_mov_b32_e32 v15, 0
	s_mov_b64 s[14:15], 0
	v_mov_b32_e32 v16, 0
	s_mov_b64 s[16:17], 0
.LBB1_54:                               ;   Parent Loop BB1_28 Depth=1
                                        ; =>  This Inner Loop Header: Depth=2
	s_add_u32 s20, s4, s16
	s_addc_u32 s21, s5, s17
	global_load_ubyte v7, v27, s[20:21]
	s_add_u32 s16, s16, 1
	s_addc_u32 s17, s17, 0
	s_waitcnt vmcnt(0)
	v_and_b32_e32 v26, 0xffff, v7
	v_lshlrev_b64 v[7:8], s14, v[26:27]
	s_add_u32 s14, s14, 8
	s_addc_u32 s15, s15, 0
	v_or_b32_e32 v15, v7, v15
	s_cmp_lg_u32 s18, s16
	v_or_b32_e32 v16, v8, v16
	s_cbranch_scc1 .LBB1_54
	s_branch .LBB1_57
.LBB1_55:                               ;   in Loop: Header=BB1_28 Depth=1
                                        ; implicit-def: $vgpr15_vgpr16
	s_mov_b32 s19, 0
	s_branch .LBB1_58
.LBB1_56:                               ;   in Loop: Header=BB1_28 Depth=1
	v_mov_b32_e32 v15, 0
	v_mov_b32_e32 v16, 0
.LBB1_57:                               ;   in Loop: Header=BB1_28 Depth=1
	s_mov_b32 s19, 0
	s_cbranch_execnz .LBB1_59
.LBB1_58:                               ;   in Loop: Header=BB1_28 Depth=1
	global_load_dwordx2 v[15:16], v27, s[4:5]
	s_add_i32 s19, s18, -8
	s_add_u32 s4, s4, 8
	s_addc_u32 s5, s5, 0
.LBB1_59:                               ;   in Loop: Header=BB1_28 Depth=1
	s_cmp_gt_u32 s19, 7
	s_cbranch_scc1 .LBB1_63
; %bb.60:                               ;   in Loop: Header=BB1_28 Depth=1
	s_cmp_eq_u32 s19, 0
	s_cbranch_scc1 .LBB1_64
; %bb.61:                               ;   in Loop: Header=BB1_28 Depth=1
	v_mov_b32_e32 v17, 0
	s_mov_b64 s[14:15], 0
	v_mov_b32_e32 v18, 0
	s_mov_b64 s[16:17], 0
.LBB1_62:                               ;   Parent Loop BB1_28 Depth=1
                                        ; =>  This Inner Loop Header: Depth=2
	s_add_u32 s20, s4, s16
	s_addc_u32 s21, s5, s17
	global_load_ubyte v7, v27, s[20:21]
	s_add_u32 s16, s16, 1
	s_addc_u32 s17, s17, 0
	s_waitcnt vmcnt(0)
	v_and_b32_e32 v26, 0xffff, v7
	v_lshlrev_b64 v[7:8], s14, v[26:27]
	s_add_u32 s14, s14, 8
	s_addc_u32 s15, s15, 0
	v_or_b32_e32 v17, v7, v17
	s_cmp_lg_u32 s19, s16
	v_or_b32_e32 v18, v8, v18
	s_cbranch_scc1 .LBB1_62
	s_branch .LBB1_65
.LBB1_63:                               ;   in Loop: Header=BB1_28 Depth=1
	s_mov_b32 s18, 0
	s_branch .LBB1_66
.LBB1_64:                               ;   in Loop: Header=BB1_28 Depth=1
	v_mov_b32_e32 v17, 0
	v_mov_b32_e32 v18, 0
.LBB1_65:                               ;   in Loop: Header=BB1_28 Depth=1
	s_mov_b32 s18, 0
	s_cbranch_execnz .LBB1_67
.LBB1_66:                               ;   in Loop: Header=BB1_28 Depth=1
	global_load_dwordx2 v[17:18], v27, s[4:5]
	s_add_i32 s18, s19, -8
	s_add_u32 s4, s4, 8
	s_addc_u32 s5, s5, 0
.LBB1_67:                               ;   in Loop: Header=BB1_28 Depth=1
	s_cmp_gt_u32 s18, 7
	s_cbranch_scc1 .LBB1_71
; %bb.68:                               ;   in Loop: Header=BB1_28 Depth=1
	s_cmp_eq_u32 s18, 0
	s_cbranch_scc1 .LBB1_72
; %bb.69:                               ;   in Loop: Header=BB1_28 Depth=1
	v_mov_b32_e32 v19, 0
	s_mov_b64 s[14:15], 0
	v_mov_b32_e32 v20, 0
	s_mov_b64 s[16:17], 0
.LBB1_70:                               ;   Parent Loop BB1_28 Depth=1
                                        ; =>  This Inner Loop Header: Depth=2
	s_add_u32 s20, s4, s16
	s_addc_u32 s21, s5, s17
	global_load_ubyte v7, v27, s[20:21]
	s_add_u32 s16, s16, 1
	s_addc_u32 s17, s17, 0
	s_waitcnt vmcnt(0)
	v_and_b32_e32 v26, 0xffff, v7
	v_lshlrev_b64 v[7:8], s14, v[26:27]
	s_add_u32 s14, s14, 8
	s_addc_u32 s15, s15, 0
	v_or_b32_e32 v19, v7, v19
	s_cmp_lg_u32 s18, s16
	v_or_b32_e32 v20, v8, v20
	s_cbranch_scc1 .LBB1_70
	s_branch .LBB1_73
.LBB1_71:                               ;   in Loop: Header=BB1_28 Depth=1
                                        ; implicit-def: $vgpr19_vgpr20
	s_mov_b32 s19, 0
	s_branch .LBB1_74
.LBB1_72:                               ;   in Loop: Header=BB1_28 Depth=1
	v_mov_b32_e32 v19, 0
	v_mov_b32_e32 v20, 0
.LBB1_73:                               ;   in Loop: Header=BB1_28 Depth=1
	s_mov_b32 s19, 0
	s_cbranch_execnz .LBB1_75
.LBB1_74:                               ;   in Loop: Header=BB1_28 Depth=1
	global_load_dwordx2 v[19:20], v27, s[4:5]
	s_add_i32 s19, s18, -8
	s_add_u32 s4, s4, 8
	s_addc_u32 s5, s5, 0
.LBB1_75:                               ;   in Loop: Header=BB1_28 Depth=1
	s_cmp_gt_u32 s19, 7
	s_cbranch_scc1 .LBB1_79
; %bb.76:                               ;   in Loop: Header=BB1_28 Depth=1
	s_cmp_eq_u32 s19, 0
	s_cbranch_scc1 .LBB1_80
; %bb.77:                               ;   in Loop: Header=BB1_28 Depth=1
	v_mov_b32_e32 v21, 0
	s_mov_b64 s[14:15], 0
	v_mov_b32_e32 v22, 0
	s_mov_b64 s[16:17], s[4:5]
.LBB1_78:                               ;   Parent Loop BB1_28 Depth=1
                                        ; =>  This Inner Loop Header: Depth=2
	global_load_ubyte v7, v27, s[16:17]
	s_add_i32 s19, s19, -1
	s_waitcnt vmcnt(0)
	v_and_b32_e32 v26, 0xffff, v7
	v_lshlrev_b64 v[7:8], s14, v[26:27]
	s_add_u32 s14, s14, 8
	s_addc_u32 s15, s15, 0
	s_add_u32 s16, s16, 1
	s_addc_u32 s17, s17, 0
	v_or_b32_e32 v21, v7, v21
	s_cmp_lg_u32 s19, 0
	v_or_b32_e32 v22, v8, v22
	s_cbranch_scc1 .LBB1_78
	s_branch .LBB1_81
.LBB1_79:                               ;   in Loop: Header=BB1_28 Depth=1
	s_branch .LBB1_82
.LBB1_80:                               ;   in Loop: Header=BB1_28 Depth=1
	v_mov_b32_e32 v21, 0
	v_mov_b32_e32 v22, 0
.LBB1_81:                               ;   in Loop: Header=BB1_28 Depth=1
	s_cbranch_execnz .LBB1_83
.LBB1_82:                               ;   in Loop: Header=BB1_28 Depth=1
	global_load_dwordx2 v[21:22], v27, s[4:5]
.LBB1_83:                               ;   in Loop: Header=BB1_28 Depth=1
	v_readfirstlane_b32 s4, v31
	v_mov_b32_e32 v7, 0
	v_mov_b32_e32 v8, 0
	v_cmp_eq_u32_e64 s[4:5], s4, v31
	s_and_saveexec_b64 s[14:15], s[4:5]
	s_cbranch_execz .LBB1_89
; %bb.84:                               ;   in Loop: Header=BB1_28 Depth=1
	global_load_dwordx2 v[25:26], v27, s[6:7] offset:24 glc
	s_waitcnt vmcnt(0)
	buffer_wbinvl1_vol
	global_load_dwordx2 v[7:8], v27, s[6:7] offset:40
	global_load_dwordx2 v[23:24], v27, s[6:7]
	s_waitcnt vmcnt(1)
	v_and_b32_e32 v7, v7, v25
	v_and_b32_e32 v8, v8, v26
	v_mul_lo_u32 v8, v8, 24
	v_mul_hi_u32 v28, v7, 24
	v_mul_lo_u32 v7, v7, 24
	v_add_u32_e32 v8, v28, v8
	s_waitcnt vmcnt(0)
	v_add_co_u32_e32 v7, vcc, v23, v7
	v_addc_co_u32_e32 v8, vcc, v24, v8, vcc
	global_load_dwordx2 v[23:24], v[7:8], off glc
	s_waitcnt vmcnt(0)
	global_atomic_cmpswap_x2 v[7:8], v27, v[23:26], s[6:7] offset:24 glc
	s_waitcnt vmcnt(0)
	buffer_wbinvl1_vol
	v_cmp_ne_u64_e32 vcc, v[7:8], v[25:26]
	s_and_saveexec_b64 s[16:17], vcc
	s_cbranch_execz .LBB1_88
; %bb.85:                               ;   in Loop: Header=BB1_28 Depth=1
	s_mov_b64 s[18:19], 0
.LBB1_86:                               ;   Parent Loop BB1_28 Depth=1
                                        ; =>  This Inner Loop Header: Depth=2
	s_sleep 1
	global_load_dwordx2 v[23:24], v27, s[6:7] offset:40
	global_load_dwordx2 v[28:29], v27, s[6:7]
	v_mov_b32_e32 v26, v8
	v_mov_b32_e32 v25, v7
	s_waitcnt vmcnt(1)
	v_and_b32_e32 v7, v23, v25
	s_waitcnt vmcnt(0)
	v_mad_u64_u32 v[7:8], s[20:21], v7, 24, v[28:29]
	v_and_b32_e32 v23, v24, v26
	v_mad_u64_u32 v[23:24], s[20:21], v23, 24, v[8:9]
	v_mov_b32_e32 v8, v23
	global_load_dwordx2 v[23:24], v[7:8], off glc
	s_waitcnt vmcnt(0)
	global_atomic_cmpswap_x2 v[7:8], v27, v[23:26], s[6:7] offset:24 glc
	s_waitcnt vmcnt(0)
	buffer_wbinvl1_vol
	v_cmp_eq_u64_e32 vcc, v[7:8], v[25:26]
	s_or_b64 s[18:19], vcc, s[18:19]
	s_andn2_b64 exec, exec, s[18:19]
	s_cbranch_execnz .LBB1_86
; %bb.87:                               ;   in Loop: Header=BB1_28 Depth=1
	s_or_b64 exec, exec, s[18:19]
.LBB1_88:                               ;   in Loop: Header=BB1_28 Depth=1
	s_or_b64 exec, exec, s[16:17]
.LBB1_89:                               ;   in Loop: Header=BB1_28 Depth=1
	s_or_b64 exec, exec, s[14:15]
	global_load_dwordx2 v[28:29], v27, s[6:7] offset:40
	global_load_dwordx4 v[23:26], v27, s[6:7]
	v_readfirstlane_b32 s14, v7
	v_readfirstlane_b32 s15, v8
	s_mov_b64 s[16:17], exec
	s_waitcnt vmcnt(1)
	v_readfirstlane_b32 s18, v28
	v_readfirstlane_b32 s19, v29
	s_and_b64 s[18:19], s[14:15], s[18:19]
	s_mul_i32 s20, s19, 24
	s_mul_hi_u32 s21, s18, 24
	s_mul_i32 s22, s18, 24
	s_add_i32 s20, s21, s20
	v_mov_b32_e32 v7, s20
	s_waitcnt vmcnt(0)
	v_add_co_u32_e32 v28, vcc, s22, v23
	v_addc_co_u32_e32 v29, vcc, v24, v7, vcc
	s_and_saveexec_b64 s[20:21], s[4:5]
	s_cbranch_execz .LBB1_91
; %bb.90:                               ;   in Loop: Header=BB1_28 Depth=1
	v_mov_b32_e32 v7, s16
	v_mov_b32_e32 v8, s17
	global_store_dwordx4 v[28:29], v[7:10], off offset:8
.LBB1_91:                               ;   in Loop: Header=BB1_28 Depth=1
	s_or_b64 exec, exec, s[20:21]
	s_lshl_b64 s[16:17], s[18:19], 12
	v_mov_b32_e32 v7, s17
	v_add_co_u32_e32 v25, vcc, s16, v25
	v_addc_co_u32_e32 v33, vcc, v26, v7, vcc
	v_cmp_lt_u64_e64 vcc, s[10:11], 57
	s_lshl_b32 s16, s12, 2
	v_cndmask_b32_e32 v7, 0, v32, vcc
	s_add_i32 s16, s16, 28
	v_and_b32_e32 v3, 0xffffff1f, v3
	s_and_b32 s16, s16, 0x1e0
	v_or_b32_e32 v3, v3, v7
	v_or_b32_e32 v3, s16, v3
	v_readfirstlane_b32 s16, v25
	v_readfirstlane_b32 s17, v33
	s_nop 4
	global_store_dwordx4 v30, v[3:6], s[16:17]
	global_store_dwordx4 v30, v[11:14], s[16:17] offset:16
	global_store_dwordx4 v30, v[15:18], s[16:17] offset:32
	global_store_dwordx4 v30, v[19:22], s[16:17] offset:48
	s_and_saveexec_b64 s[16:17], s[4:5]
	s_cbranch_execz .LBB1_99
; %bb.92:                               ;   in Loop: Header=BB1_28 Depth=1
	global_load_dwordx2 v[13:14], v27, s[6:7] offset:32 glc
	global_load_dwordx2 v[3:4], v27, s[6:7] offset:40
	v_mov_b32_e32 v11, s14
	v_mov_b32_e32 v12, s15
	s_waitcnt vmcnt(0)
	v_readfirstlane_b32 s18, v3
	v_readfirstlane_b32 s19, v4
	s_and_b64 s[18:19], s[18:19], s[14:15]
	s_mul_i32 s19, s19, 24
	s_mul_hi_u32 s20, s18, 24
	s_mul_i32 s18, s18, 24
	s_add_i32 s19, s20, s19
	v_mov_b32_e32 v3, s19
	v_add_co_u32_e32 v7, vcc, s18, v23
	v_addc_co_u32_e32 v8, vcc, v24, v3, vcc
	global_store_dwordx2 v[7:8], v[13:14], off
	s_waitcnt vmcnt(0)
	global_atomic_cmpswap_x2 v[5:6], v27, v[11:14], s[6:7] offset:32 glc
	s_waitcnt vmcnt(0)
	v_cmp_ne_u64_e32 vcc, v[5:6], v[13:14]
	s_and_saveexec_b64 s[18:19], vcc
	s_cbranch_execz .LBB1_95
; %bb.93:                               ;   in Loop: Header=BB1_28 Depth=1
	s_mov_b64 s[20:21], 0
.LBB1_94:                               ;   Parent Loop BB1_28 Depth=1
                                        ; =>  This Inner Loop Header: Depth=2
	s_sleep 1
	global_store_dwordx2 v[7:8], v[5:6], off
	v_mov_b32_e32 v3, s14
	v_mov_b32_e32 v4, s15
	s_waitcnt vmcnt(0)
	global_atomic_cmpswap_x2 v[3:4], v27, v[3:6], s[6:7] offset:32 glc
	s_waitcnt vmcnt(0)
	v_cmp_eq_u64_e32 vcc, v[3:4], v[5:6]
	v_mov_b32_e32 v6, v4
	s_or_b64 s[20:21], vcc, s[20:21]
	v_mov_b32_e32 v5, v3
	s_andn2_b64 exec, exec, s[20:21]
	s_cbranch_execnz .LBB1_94
.LBB1_95:                               ;   in Loop: Header=BB1_28 Depth=1
	s_or_b64 exec, exec, s[18:19]
	global_load_dwordx2 v[3:4], v27, s[6:7] offset:16
	s_mov_b64 s[20:21], exec
	v_mbcnt_lo_u32_b32 v5, s20, 0
	v_mbcnt_hi_u32_b32 v5, s21, v5
	v_cmp_eq_u32_e32 vcc, 0, v5
	s_and_saveexec_b64 s[18:19], vcc
	s_cbranch_execz .LBB1_97
; %bb.96:                               ;   in Loop: Header=BB1_28 Depth=1
	s_bcnt1_i32_b64 s20, s[20:21]
	v_mov_b32_e32 v26, s20
	s_waitcnt vmcnt(0)
	global_atomic_add_x2 v[3:4], v[26:27], off offset:8
.LBB1_97:                               ;   in Loop: Header=BB1_28 Depth=1
	s_or_b64 exec, exec, s[18:19]
	s_waitcnt vmcnt(0)
	global_load_dwordx2 v[5:6], v[3:4], off offset:16
	s_waitcnt vmcnt(0)
	v_cmp_eq_u64_e32 vcc, 0, v[5:6]
	s_cbranch_vccnz .LBB1_99
; %bb.98:                               ;   in Loop: Header=BB1_28 Depth=1
	global_load_dword v26, v[3:4], off offset:24
	s_waitcnt vmcnt(0)
	v_and_b32_e32 v3, 0xffffff, v26
	v_readfirstlane_b32 s18, v3
	s_mov_b32 m0, s18
	global_store_dwordx2 v[5:6], v[26:27], off
	s_sendmsg sendmsg(MSG_INTERRUPT)
.LBB1_99:                               ;   in Loop: Header=BB1_28 Depth=1
	s_or_b64 exec, exec, s[16:17]
	v_add_co_u32_e32 v3, vcc, v25, v30
	v_addc_co_u32_e32 v4, vcc, 0, v33, vcc
	s_branch .LBB1_103
.LBB1_100:                              ;   in Loop: Header=BB1_103 Depth=2
	s_or_b64 exec, exec, s[16:17]
	v_readfirstlane_b32 s16, v5
	s_cmp_eq_u32 s16, 0
	s_cbranch_scc1 .LBB1_102
; %bb.101:                              ;   in Loop: Header=BB1_103 Depth=2
	s_sleep 1
	s_cbranch_execnz .LBB1_103
	s_branch .LBB1_105
.LBB1_102:                              ;   in Loop: Header=BB1_28 Depth=1
	s_branch .LBB1_105
.LBB1_103:                              ;   Parent Loop BB1_28 Depth=1
                                        ; =>  This Inner Loop Header: Depth=2
	v_mov_b32_e32 v5, 1
	s_and_saveexec_b64 s[16:17], s[4:5]
	s_cbranch_execz .LBB1_100
; %bb.104:                              ;   in Loop: Header=BB1_103 Depth=2
	global_load_dword v5, v[28:29], off offset:20 glc
	s_waitcnt vmcnt(0)
	buffer_wbinvl1_vol
	v_and_b32_e32 v5, 1, v5
	s_branch .LBB1_100
.LBB1_105:                              ;   in Loop: Header=BB1_28 Depth=1
	global_load_dwordx2 v[3:4], v[3:4], off
	s_and_saveexec_b64 s[16:17], s[4:5]
	s_cbranch_execz .LBB1_27
; %bb.106:                              ;   in Loop: Header=BB1_28 Depth=1
	global_load_dwordx2 v[7:8], v27, s[6:7] offset:40
	global_load_dwordx2 v[13:14], v27, s[6:7] offset:24 glc
	global_load_dwordx2 v[11:12], v27, s[6:7]
	v_mov_b32_e32 v6, s15
	s_waitcnt vmcnt(2)
	v_add_co_u32_e32 v15, vcc, 1, v7
	v_addc_co_u32_e32 v16, vcc, 0, v8, vcc
	v_add_co_u32_e32 v5, vcc, s14, v15
	v_addc_co_u32_e32 v6, vcc, v16, v6, vcc
	v_cmp_eq_u64_e32 vcc, 0, v[5:6]
	v_cndmask_b32_e32 v6, v6, v16, vcc
	v_cndmask_b32_e32 v5, v5, v15, vcc
	v_and_b32_e32 v8, v6, v8
	v_and_b32_e32 v7, v5, v7
	v_mul_lo_u32 v8, v8, 24
	v_mul_hi_u32 v15, v7, 24
	v_mul_lo_u32 v16, v7, 24
	s_waitcnt vmcnt(1)
	v_mov_b32_e32 v7, v13
	v_add_u32_e32 v8, v15, v8
	s_waitcnt vmcnt(0)
	v_add_co_u32_e32 v11, vcc, v11, v16
	v_addc_co_u32_e32 v12, vcc, v12, v8, vcc
	global_store_dwordx2 v[11:12], v[13:14], off
	v_mov_b32_e32 v8, v14
	s_waitcnt vmcnt(0)
	global_atomic_cmpswap_x2 v[7:8], v27, v[5:8], s[6:7] offset:24 glc
	s_waitcnt vmcnt(0)
	v_cmp_ne_u64_e32 vcc, v[7:8], v[13:14]
	s_and_b64 exec, exec, vcc
	s_cbranch_execz .LBB1_27
; %bb.107:                              ;   in Loop: Header=BB1_28 Depth=1
	s_mov_b64 s[4:5], 0
.LBB1_108:                              ;   Parent Loop BB1_28 Depth=1
                                        ; =>  This Inner Loop Header: Depth=2
	s_sleep 1
	global_store_dwordx2 v[11:12], v[7:8], off
	s_waitcnt vmcnt(0)
	global_atomic_cmpswap_x2 v[13:14], v27, v[5:8], s[6:7] offset:24 glc
	s_waitcnt vmcnt(0)
	v_cmp_eq_u64_e32 vcc, v[13:14], v[7:8]
	v_mov_b32_e32 v7, v13
	s_or_b64 s[4:5], vcc, s[4:5]
	v_mov_b32_e32 v8, v14
	s_andn2_b64 exec, exec, s[4:5]
	s_cbranch_execnz .LBB1_108
	s_branch .LBB1_27
.LBB1_109:
	s_branch .LBB1_137
.LBB1_110:
                                        ; implicit-def: $vgpr3_vgpr4
	s_cbranch_execz .LBB1_137
; %bb.111:
	v_readfirstlane_b32 s4, v31
	v_mov_b32_e32 v9, 0
	v_mov_b32_e32 v10, 0
	v_cmp_eq_u32_e64 s[4:5], s4, v31
	s_and_saveexec_b64 s[8:9], s[4:5]
	s_cbranch_execz .LBB1_117
; %bb.112:
	s_waitcnt vmcnt(0)
	v_mov_b32_e32 v3, 0
	global_load_dwordx2 v[6:7], v3, s[6:7] offset:24 glc
	s_waitcnt vmcnt(0)
	buffer_wbinvl1_vol
	global_load_dwordx2 v[4:5], v3, s[6:7] offset:40
	global_load_dwordx2 v[8:9], v3, s[6:7]
	s_waitcnt vmcnt(1)
	v_and_b32_e32 v4, v4, v6
	v_and_b32_e32 v5, v5, v7
	v_mul_lo_u32 v5, v5, 24
	v_mul_hi_u32 v10, v4, 24
	v_mul_lo_u32 v4, v4, 24
	v_add_u32_e32 v5, v10, v5
	s_waitcnt vmcnt(0)
	v_add_co_u32_e32 v4, vcc, v8, v4
	v_addc_co_u32_e32 v5, vcc, v9, v5, vcc
	global_load_dwordx2 v[4:5], v[4:5], off glc
	s_waitcnt vmcnt(0)
	global_atomic_cmpswap_x2 v[9:10], v3, v[4:7], s[6:7] offset:24 glc
	s_waitcnt vmcnt(0)
	buffer_wbinvl1_vol
	v_cmp_ne_u64_e32 vcc, v[9:10], v[6:7]
	s_and_saveexec_b64 s[10:11], vcc
	s_cbranch_execz .LBB1_116
; %bb.113:
	s_mov_b64 s[12:13], 0
.LBB1_114:                              ; =>This Inner Loop Header: Depth=1
	s_sleep 1
	global_load_dwordx2 v[4:5], v3, s[6:7] offset:40
	global_load_dwordx2 v[11:12], v3, s[6:7]
	v_mov_b32_e32 v6, v9
	v_mov_b32_e32 v7, v10
	s_waitcnt vmcnt(1)
	v_and_b32_e32 v4, v4, v6
	s_waitcnt vmcnt(0)
	v_mad_u64_u32 v[8:9], s[14:15], v4, 24, v[11:12]
	v_and_b32_e32 v5, v5, v7
	v_mov_b32_e32 v4, v9
	v_mad_u64_u32 v[4:5], s[14:15], v5, 24, v[4:5]
	v_mov_b32_e32 v9, v4
	global_load_dwordx2 v[4:5], v[8:9], off glc
	s_waitcnt vmcnt(0)
	global_atomic_cmpswap_x2 v[9:10], v3, v[4:7], s[6:7] offset:24 glc
	s_waitcnt vmcnt(0)
	buffer_wbinvl1_vol
	v_cmp_eq_u64_e32 vcc, v[9:10], v[6:7]
	s_or_b64 s[12:13], vcc, s[12:13]
	s_andn2_b64 exec, exec, s[12:13]
	s_cbranch_execnz .LBB1_114
; %bb.115:
	s_or_b64 exec, exec, s[12:13]
.LBB1_116:
	s_or_b64 exec, exec, s[10:11]
.LBB1_117:
	s_or_b64 exec, exec, s[8:9]
	s_waitcnt vmcnt(0)
	v_mov_b32_e32 v3, 0
	global_load_dwordx2 v[11:12], v3, s[6:7] offset:40
	global_load_dwordx4 v[5:8], v3, s[6:7]
	v_readfirstlane_b32 s8, v9
	v_readfirstlane_b32 s9, v10
	s_mov_b64 s[10:11], exec
	s_waitcnt vmcnt(1)
	v_readfirstlane_b32 s12, v11
	v_readfirstlane_b32 s13, v12
	s_and_b64 s[12:13], s[8:9], s[12:13]
	s_mul_i32 s14, s13, 24
	s_mul_hi_u32 s15, s12, 24
	s_mul_i32 s16, s12, 24
	s_add_i32 s14, s15, s14
	v_mov_b32_e32 v4, s14
	s_waitcnt vmcnt(0)
	v_add_co_u32_e32 v9, vcc, s16, v5
	v_addc_co_u32_e32 v10, vcc, v6, v4, vcc
	s_and_saveexec_b64 s[14:15], s[4:5]
	s_cbranch_execz .LBB1_119
; %bb.118:
	v_mov_b32_e32 v12, s11
	v_mov_b32_e32 v11, s10
	v_mov_b32_e32 v13, 2
	v_mov_b32_e32 v14, 1
	global_store_dwordx4 v[9:10], v[11:14], off offset:8
.LBB1_119:
	s_or_b64 exec, exec, s[14:15]
	s_lshl_b64 s[10:11], s[12:13], 12
	v_mov_b32_e32 v4, s11
	v_add_co_u32_e32 v11, vcc, s10, v7
	v_addc_co_u32_e32 v12, vcc, v8, v4, vcc
	s_movk_i32 s10, 0xff1f
	v_and_or_b32 v1, v1, s10, 32
	s_mov_b32 s12, 0
	v_mov_b32_e32 v4, v3
	v_readfirstlane_b32 s10, v11
	v_readfirstlane_b32 s11, v12
	v_add_co_u32_e32 v7, vcc, v11, v30
	s_mov_b32 s13, s12
	s_mov_b32 s14, s12
	s_mov_b32 s15, s12
	s_nop 0
	global_store_dwordx4 v30, v[1:4], s[10:11]
	v_addc_co_u32_e32 v8, vcc, 0, v12, vcc
	v_mov_b32_e32 v1, s12
	v_mov_b32_e32 v2, s13
	v_mov_b32_e32 v3, s14
	v_mov_b32_e32 v4, s15
	global_store_dwordx4 v30, v[1:4], s[10:11] offset:16
	global_store_dwordx4 v30, v[1:4], s[10:11] offset:32
	global_store_dwordx4 v30, v[1:4], s[10:11] offset:48
	s_and_saveexec_b64 s[10:11], s[4:5]
	s_cbranch_execz .LBB1_127
; %bb.120:
	v_mov_b32_e32 v11, 0
	global_load_dwordx2 v[14:15], v11, s[6:7] offset:32 glc
	global_load_dwordx2 v[1:2], v11, s[6:7] offset:40
	v_mov_b32_e32 v12, s8
	v_mov_b32_e32 v13, s9
	s_waitcnt vmcnt(0)
	v_readfirstlane_b32 s12, v1
	v_readfirstlane_b32 s13, v2
	s_and_b64 s[12:13], s[12:13], s[8:9]
	s_mul_i32 s13, s13, 24
	s_mul_hi_u32 s14, s12, 24
	s_mul_i32 s12, s12, 24
	s_add_i32 s13, s14, s13
	v_mov_b32_e32 v1, s13
	v_add_co_u32_e32 v5, vcc, s12, v5
	v_addc_co_u32_e32 v6, vcc, v6, v1, vcc
	global_store_dwordx2 v[5:6], v[14:15], off
	s_waitcnt vmcnt(0)
	global_atomic_cmpswap_x2 v[3:4], v11, v[12:15], s[6:7] offset:32 glc
	s_waitcnt vmcnt(0)
	v_cmp_ne_u64_e32 vcc, v[3:4], v[14:15]
	s_and_saveexec_b64 s[12:13], vcc
	s_cbranch_execz .LBB1_123
; %bb.121:
	s_mov_b64 s[14:15], 0
.LBB1_122:                              ; =>This Inner Loop Header: Depth=1
	s_sleep 1
	global_store_dwordx2 v[5:6], v[3:4], off
	v_mov_b32_e32 v1, s8
	v_mov_b32_e32 v2, s9
	s_waitcnt vmcnt(0)
	global_atomic_cmpswap_x2 v[1:2], v11, v[1:4], s[6:7] offset:32 glc
	s_waitcnt vmcnt(0)
	v_cmp_eq_u64_e32 vcc, v[1:2], v[3:4]
	v_mov_b32_e32 v4, v2
	s_or_b64 s[14:15], vcc, s[14:15]
	v_mov_b32_e32 v3, v1
	s_andn2_b64 exec, exec, s[14:15]
	s_cbranch_execnz .LBB1_122
.LBB1_123:
	s_or_b64 exec, exec, s[12:13]
	v_mov_b32_e32 v4, 0
	global_load_dwordx2 v[1:2], v4, s[6:7] offset:16
	s_mov_b64 s[12:13], exec
	v_mbcnt_lo_u32_b32 v3, s12, 0
	v_mbcnt_hi_u32_b32 v3, s13, v3
	v_cmp_eq_u32_e32 vcc, 0, v3
	s_and_saveexec_b64 s[14:15], vcc
	s_cbranch_execz .LBB1_125
; %bb.124:
	s_bcnt1_i32_b64 s12, s[12:13]
	v_mov_b32_e32 v3, s12
	s_waitcnt vmcnt(0)
	global_atomic_add_x2 v[1:2], v[3:4], off offset:8
.LBB1_125:
	s_or_b64 exec, exec, s[14:15]
	s_waitcnt vmcnt(0)
	global_load_dwordx2 v[3:4], v[1:2], off offset:16
	s_waitcnt vmcnt(0)
	v_cmp_eq_u64_e32 vcc, 0, v[3:4]
	s_cbranch_vccnz .LBB1_127
; %bb.126:
	global_load_dword v1, v[1:2], off offset:24
	v_mov_b32_e32 v2, 0
	s_waitcnt vmcnt(0)
	global_store_dwordx2 v[3:4], v[1:2], off
	v_and_b32_e32 v1, 0xffffff, v1
	v_readfirstlane_b32 s12, v1
	s_mov_b32 m0, s12
	s_nop 0
	s_sendmsg sendmsg(MSG_INTERRUPT)
.LBB1_127:
	s_or_b64 exec, exec, s[10:11]
	s_branch .LBB1_131
.LBB1_128:                              ;   in Loop: Header=BB1_131 Depth=1
	s_or_b64 exec, exec, s[10:11]
	v_readfirstlane_b32 s10, v1
	s_cmp_eq_u32 s10, 0
	s_cbranch_scc1 .LBB1_130
; %bb.129:                              ;   in Loop: Header=BB1_131 Depth=1
	s_sleep 1
	s_cbranch_execnz .LBB1_131
	s_branch .LBB1_133
.LBB1_130:
	s_branch .LBB1_133
.LBB1_131:                              ; =>This Inner Loop Header: Depth=1
	v_mov_b32_e32 v1, 1
	s_and_saveexec_b64 s[10:11], s[4:5]
	s_cbranch_execz .LBB1_128
; %bb.132:                              ;   in Loop: Header=BB1_131 Depth=1
	global_load_dword v1, v[9:10], off offset:20 glc
	s_waitcnt vmcnt(0)
	buffer_wbinvl1_vol
	v_and_b32_e32 v1, 1, v1
	s_branch .LBB1_128
.LBB1_133:
	global_load_dwordx2 v[3:4], v[7:8], off
	s_and_saveexec_b64 s[10:11], s[4:5]
	s_cbranch_execz .LBB1_136
; %bb.134:
	v_mov_b32_e32 v9, 0
	global_load_dwordx2 v[1:2], v9, s[6:7] offset:40
	global_load_dwordx2 v[10:11], v9, s[6:7] offset:24 glc
	global_load_dwordx2 v[12:13], v9, s[6:7]
	v_mov_b32_e32 v6, s9
	s_mov_b64 s[4:5], 0
	s_waitcnt vmcnt(2)
	v_add_co_u32_e32 v7, vcc, 1, v1
	v_addc_co_u32_e32 v8, vcc, 0, v2, vcc
	v_add_co_u32_e32 v5, vcc, s8, v7
	v_addc_co_u32_e32 v6, vcc, v8, v6, vcc
	v_cmp_eq_u64_e32 vcc, 0, v[5:6]
	v_cndmask_b32_e32 v6, v6, v8, vcc
	v_cndmask_b32_e32 v5, v5, v7, vcc
	v_and_b32_e32 v2, v6, v2
	v_and_b32_e32 v1, v5, v1
	v_mul_lo_u32 v2, v2, 24
	v_mul_hi_u32 v8, v1, 24
	v_mul_lo_u32 v1, v1, 24
	s_waitcnt vmcnt(1)
	v_mov_b32_e32 v7, v10
	v_add_u32_e32 v2, v8, v2
	s_waitcnt vmcnt(0)
	v_add_co_u32_e32 v1, vcc, v12, v1
	v_addc_co_u32_e32 v2, vcc, v13, v2, vcc
	global_store_dwordx2 v[1:2], v[10:11], off
	v_mov_b32_e32 v8, v11
	s_waitcnt vmcnt(0)
	global_atomic_cmpswap_x2 v[7:8], v9, v[5:8], s[6:7] offset:24 glc
	s_waitcnt vmcnt(0)
	v_cmp_ne_u64_e32 vcc, v[7:8], v[10:11]
	s_and_b64 exec, exec, vcc
	s_cbranch_execz .LBB1_136
.LBB1_135:                              ; =>This Inner Loop Header: Depth=1
	s_sleep 1
	global_store_dwordx2 v[1:2], v[7:8], off
	s_waitcnt vmcnt(0)
	global_atomic_cmpswap_x2 v[10:11], v9, v[5:8], s[6:7] offset:24 glc
	s_waitcnt vmcnt(0)
	v_cmp_eq_u64_e32 vcc, v[10:11], v[7:8]
	v_mov_b32_e32 v7, v10
	s_or_b64 s[4:5], vcc, s[4:5]
	v_mov_b32_e32 v8, v11
	s_andn2_b64 exec, exec, s[4:5]
	s_cbranch_execnz .LBB1_135
.LBB1_136:
	s_or_b64 exec, exec, s[10:11]
.LBB1_137:
	v_readfirstlane_b32 s4, v31
	s_waitcnt vmcnt(0)
	v_mov_b32_e32 v1, 0
	v_mov_b32_e32 v2, 0
	v_cmp_eq_u32_e64 s[4:5], s4, v31
	s_and_saveexec_b64 s[8:9], s[4:5]
	s_cbranch_execz .LBB1_143
; %bb.138:
	v_mov_b32_e32 v5, 0
	global_load_dwordx2 v[8:9], v5, s[6:7] offset:24 glc
	s_waitcnt vmcnt(0)
	buffer_wbinvl1_vol
	global_load_dwordx2 v[1:2], v5, s[6:7] offset:40
	global_load_dwordx2 v[6:7], v5, s[6:7]
	s_waitcnt vmcnt(1)
	v_and_b32_e32 v1, v1, v8
	v_and_b32_e32 v2, v2, v9
	v_mul_lo_u32 v2, v2, 24
	v_mul_hi_u32 v10, v1, 24
	v_mul_lo_u32 v1, v1, 24
	v_add_u32_e32 v2, v10, v2
	s_waitcnt vmcnt(0)
	v_add_co_u32_e32 v1, vcc, v6, v1
	v_addc_co_u32_e32 v2, vcc, v7, v2, vcc
	global_load_dwordx2 v[6:7], v[1:2], off glc
	s_waitcnt vmcnt(0)
	global_atomic_cmpswap_x2 v[1:2], v5, v[6:9], s[6:7] offset:24 glc
	s_waitcnt vmcnt(0)
	buffer_wbinvl1_vol
	v_cmp_ne_u64_e32 vcc, v[1:2], v[8:9]
	s_and_saveexec_b64 s[10:11], vcc
	s_cbranch_execz .LBB1_142
; %bb.139:
	s_mov_b64 s[12:13], 0
.LBB1_140:                              ; =>This Inner Loop Header: Depth=1
	s_sleep 1
	global_load_dwordx2 v[6:7], v5, s[6:7] offset:40
	global_load_dwordx2 v[10:11], v5, s[6:7]
	v_mov_b32_e32 v9, v2
	v_mov_b32_e32 v8, v1
	s_waitcnt vmcnt(1)
	v_and_b32_e32 v1, v6, v8
	s_waitcnt vmcnt(0)
	v_mad_u64_u32 v[1:2], s[14:15], v1, 24, v[10:11]
	v_and_b32_e32 v6, v7, v9
	v_mad_u64_u32 v[6:7], s[14:15], v6, 24, v[2:3]
	v_mov_b32_e32 v2, v6
	global_load_dwordx2 v[6:7], v[1:2], off glc
	s_waitcnt vmcnt(0)
	global_atomic_cmpswap_x2 v[1:2], v5, v[6:9], s[6:7] offset:24 glc
	s_waitcnt vmcnt(0)
	buffer_wbinvl1_vol
	v_cmp_eq_u64_e32 vcc, v[1:2], v[8:9]
	s_or_b64 s[12:13], vcc, s[12:13]
	s_andn2_b64 exec, exec, s[12:13]
	s_cbranch_execnz .LBB1_140
; %bb.141:
	s_or_b64 exec, exec, s[12:13]
.LBB1_142:
	s_or_b64 exec, exec, s[10:11]
.LBB1_143:
	s_or_b64 exec, exec, s[8:9]
	v_mov_b32_e32 v11, 0
	global_load_dwordx2 v[5:6], v11, s[6:7] offset:40
	global_load_dwordx4 v[7:10], v11, s[6:7]
	v_readfirstlane_b32 s8, v1
	v_readfirstlane_b32 s9, v2
	s_mov_b64 s[10:11], exec
	s_waitcnt vmcnt(1)
	v_readfirstlane_b32 s12, v5
	v_readfirstlane_b32 s13, v6
	s_and_b64 s[12:13], s[8:9], s[12:13]
	s_mul_i32 s14, s13, 24
	s_mul_hi_u32 s15, s12, 24
	s_mul_i32 s16, s12, 24
	s_add_i32 s14, s15, s14
	v_mov_b32_e32 v1, s14
	s_waitcnt vmcnt(0)
	v_add_co_u32_e32 v11, vcc, s16, v7
	v_addc_co_u32_e32 v12, vcc, v8, v1, vcc
	s_and_saveexec_b64 s[14:15], s[4:5]
	s_cbranch_execz .LBB1_145
; %bb.144:
	v_mov_b32_e32 v14, s11
	v_mov_b32_e32 v13, s10
	v_mov_b32_e32 v15, 2
	v_mov_b32_e32 v16, 1
	global_store_dwordx4 v[11:12], v[13:16], off offset:8
.LBB1_145:
	s_or_b64 exec, exec, s[14:15]
	s_lshl_b64 s[10:11], s[12:13], 12
	v_cvt_f64_f32_e32 v[5:6], v0
	v_mov_b32_e32 v1, s11
	v_add_co_u32_e32 v2, vcc, s10, v9
	v_addc_co_u32_e32 v1, vcc, v10, v1, vcc
	s_movk_i32 s10, 0xff1d
	v_and_or_b32 v3, v3, s10, 34
	v_readfirstlane_b32 s10, v2
	v_readfirstlane_b32 s11, v1
	s_mov_b32 s12, 0
	s_mov_b32 s13, s12
	s_mov_b32 s14, s12
	s_mov_b32 s15, s12
	s_nop 0
	global_store_dwordx4 v30, v[3:6], s[10:11]
	v_mov_b32_e32 v0, s12
	v_mov_b32_e32 v1, s13
	v_mov_b32_e32 v2, s14
	v_mov_b32_e32 v3, s15
	global_store_dwordx4 v30, v[0:3], s[10:11] offset:16
	global_store_dwordx4 v30, v[0:3], s[10:11] offset:32
	global_store_dwordx4 v30, v[0:3], s[10:11] offset:48
	s_and_saveexec_b64 s[10:11], s[4:5]
	s_cbranch_execz .LBB1_153
; %bb.146:
	v_mov_b32_e32 v6, 0
	global_load_dwordx2 v[15:16], v6, s[6:7] offset:32 glc
	global_load_dwordx2 v[0:1], v6, s[6:7] offset:40
	v_mov_b32_e32 v13, s8
	v_mov_b32_e32 v14, s9
	s_waitcnt vmcnt(0)
	v_readfirstlane_b32 s12, v0
	v_readfirstlane_b32 s13, v1
	s_and_b64 s[12:13], s[12:13], s[8:9]
	s_mul_i32 s13, s13, 24
	s_mul_hi_u32 s14, s12, 24
	s_mul_i32 s12, s12, 24
	s_add_i32 s13, s14, s13
	v_mov_b32_e32 v0, s13
	v_add_co_u32_e32 v4, vcc, s12, v7
	v_addc_co_u32_e32 v5, vcc, v8, v0, vcc
	global_store_dwordx2 v[4:5], v[15:16], off
	s_waitcnt vmcnt(0)
	global_atomic_cmpswap_x2 v[2:3], v6, v[13:16], s[6:7] offset:32 glc
	s_waitcnt vmcnt(0)
	v_cmp_ne_u64_e32 vcc, v[2:3], v[15:16]
	s_and_saveexec_b64 s[12:13], vcc
	s_cbranch_execz .LBB1_149
; %bb.147:
	s_mov_b64 s[14:15], 0
.LBB1_148:                              ; =>This Inner Loop Header: Depth=1
	s_sleep 1
	global_store_dwordx2 v[4:5], v[2:3], off
	v_mov_b32_e32 v0, s8
	v_mov_b32_e32 v1, s9
	s_waitcnt vmcnt(0)
	global_atomic_cmpswap_x2 v[0:1], v6, v[0:3], s[6:7] offset:32 glc
	s_waitcnt vmcnt(0)
	v_cmp_eq_u64_e32 vcc, v[0:1], v[2:3]
	v_mov_b32_e32 v3, v1
	s_or_b64 s[14:15], vcc, s[14:15]
	v_mov_b32_e32 v2, v0
	s_andn2_b64 exec, exec, s[14:15]
	s_cbranch_execnz .LBB1_148
.LBB1_149:
	s_or_b64 exec, exec, s[12:13]
	v_mov_b32_e32 v3, 0
	global_load_dwordx2 v[0:1], v3, s[6:7] offset:16
	s_mov_b64 s[12:13], exec
	v_mbcnt_lo_u32_b32 v2, s12, 0
	v_mbcnt_hi_u32_b32 v2, s13, v2
	v_cmp_eq_u32_e32 vcc, 0, v2
	s_and_saveexec_b64 s[14:15], vcc
	s_cbranch_execz .LBB1_151
; %bb.150:
	s_bcnt1_i32_b64 s12, s[12:13]
	v_mov_b32_e32 v2, s12
	s_waitcnt vmcnt(0)
	global_atomic_add_x2 v[0:1], v[2:3], off offset:8
.LBB1_151:
	s_or_b64 exec, exec, s[14:15]
	s_waitcnt vmcnt(0)
	global_load_dwordx2 v[2:3], v[0:1], off offset:16
	s_waitcnt vmcnt(0)
	v_cmp_eq_u64_e32 vcc, 0, v[2:3]
	s_cbranch_vccnz .LBB1_153
; %bb.152:
	global_load_dword v0, v[0:1], off offset:24
	v_mov_b32_e32 v1, 0
	s_waitcnt vmcnt(0)
	global_store_dwordx2 v[2:3], v[0:1], off
	v_and_b32_e32 v0, 0xffffff, v0
	v_readfirstlane_b32 s12, v0
	s_mov_b32 m0, s12
	s_nop 0
	s_sendmsg sendmsg(MSG_INTERRUPT)
.LBB1_153:
	s_or_b64 exec, exec, s[10:11]
	s_branch .LBB1_157
.LBB1_154:                              ;   in Loop: Header=BB1_157 Depth=1
	s_or_b64 exec, exec, s[10:11]
	v_readfirstlane_b32 s10, v0
	s_cmp_eq_u32 s10, 0
	s_cbranch_scc1 .LBB1_156
; %bb.155:                              ;   in Loop: Header=BB1_157 Depth=1
	s_sleep 1
	s_cbranch_execnz .LBB1_157
	s_branch .LBB1_159
.LBB1_156:
	s_branch .LBB1_159
.LBB1_157:                              ; =>This Inner Loop Header: Depth=1
	v_mov_b32_e32 v0, 1
	s_and_saveexec_b64 s[10:11], s[4:5]
	s_cbranch_execz .LBB1_154
; %bb.158:                              ;   in Loop: Header=BB1_157 Depth=1
	global_load_dword v0, v[11:12], off offset:20 glc
	s_waitcnt vmcnt(0)
	buffer_wbinvl1_vol
	v_and_b32_e32 v0, 1, v0
	s_branch .LBB1_154
.LBB1_159:
	s_and_saveexec_b64 s[10:11], s[4:5]
	s_cbranch_execz .LBB1_162
; %bb.160:
	v_mov_b32_e32 v6, 0
	global_load_dwordx2 v[2:3], v6, s[6:7] offset:40
	global_load_dwordx2 v[7:8], v6, s[6:7] offset:24 glc
	global_load_dwordx2 v[4:5], v6, s[6:7]
	v_mov_b32_e32 v1, s9
	s_mov_b64 s[4:5], 0
	s_waitcnt vmcnt(2)
	v_add_co_u32_e32 v9, vcc, 1, v2
	v_addc_co_u32_e32 v10, vcc, 0, v3, vcc
	v_add_co_u32_e32 v0, vcc, s8, v9
	v_addc_co_u32_e32 v1, vcc, v10, v1, vcc
	v_cmp_eq_u64_e32 vcc, 0, v[0:1]
	v_cndmask_b32_e32 v1, v1, v10, vcc
	v_cndmask_b32_e32 v0, v0, v9, vcc
	v_and_b32_e32 v3, v1, v3
	v_and_b32_e32 v2, v0, v2
	v_mul_lo_u32 v3, v3, 24
	v_mul_hi_u32 v9, v2, 24
	v_mul_lo_u32 v10, v2, 24
	s_waitcnt vmcnt(1)
	v_mov_b32_e32 v2, v7
	v_add_u32_e32 v3, v9, v3
	s_waitcnt vmcnt(0)
	v_add_co_u32_e32 v4, vcc, v4, v10
	v_addc_co_u32_e32 v5, vcc, v5, v3, vcc
	global_store_dwordx2 v[4:5], v[7:8], off
	v_mov_b32_e32 v3, v8
	s_waitcnt vmcnt(0)
	global_atomic_cmpswap_x2 v[2:3], v6, v[0:3], s[6:7] offset:24 glc
	s_waitcnt vmcnt(0)
	v_cmp_ne_u64_e32 vcc, v[2:3], v[7:8]
	s_and_b64 exec, exec, vcc
	s_cbranch_execz .LBB1_162
.LBB1_161:                              ; =>This Inner Loop Header: Depth=1
	s_sleep 1
	global_store_dwordx2 v[4:5], v[2:3], off
	s_waitcnt vmcnt(0)
	global_atomic_cmpswap_x2 v[7:8], v6, v[0:3], s[6:7] offset:24 glc
	s_waitcnt vmcnt(0)
	v_cmp_eq_u64_e32 vcc, v[7:8], v[2:3]
	v_mov_b32_e32 v2, v7
	s_or_b64 s[4:5], vcc, s[4:5]
	v_mov_b32_e32 v3, v8
	s_andn2_b64 exec, exec, s[4:5]
	s_cbranch_execnz .LBB1_161
.LBB1_162:
	s_or_b64 exec, exec, s[10:11]
	s_waitcnt vmcnt(0) lgkmcnt(0)
	s_setpc_b64 s[30:31]
.Lfunc_end1:
	.size	_Z1Bf, .Lfunc_end1-_Z1Bf
                                        ; -- End function
	.set .L_Z1Bf.num_vgpr, 34
	.set .L_Z1Bf.num_agpr, 0
	.set .L_Z1Bf.numbered_sgpr, 32
	.set .L_Z1Bf.private_seg_size, 0
	.set .L_Z1Bf.uses_vcc, 1
	.set .L_Z1Bf.uses_flat_scratch, 0
	.set .L_Z1Bf.has_dyn_sized_stack, 0
	.set .L_Z1Bf.has_recursion, 0
	.set .L_Z1Bf.has_indirect_call, 0
	.section	.AMDGPU.csdata,"",@progbits
; Function info:
; codeLenInByte = 5816
; TotalNumSgprs: 36
; NumVgprs: 34
; NumAgprs: 0
; TotalNumVgprs: 34
; ScratchSize: 0
; MemoryBound: 0
	.text
	.p2align	2                               ; -- Begin function _Z1Cf
	.type	_Z1Cf,@function
_Z1Cf:                                  ; @_Z1Cf
; %bb.0:
	s_waitcnt vmcnt(0) expcnt(0) lgkmcnt(0)
	s_load_dwordx2 s[6:7], s[8:9], 0x50
	v_mbcnt_lo_u32_b32 v1, -1, 0
	v_mbcnt_hi_u32_b32 v31, -1, v1
	v_readfirstlane_b32 s4, v31
	v_mov_b32_e32 v6, 0
	v_mov_b32_e32 v7, 0
	v_cmp_eq_u32_e64 s[4:5], s4, v31
	s_and_saveexec_b64 s[8:9], s[4:5]
	s_cbranch_execz .LBB2_6
; %bb.1:
	v_mov_b32_e32 v1, 0
	s_waitcnt lgkmcnt(0)
	global_load_dwordx2 v[4:5], v1, s[6:7] offset:24 glc
	s_waitcnt vmcnt(0)
	buffer_wbinvl1_vol
	global_load_dwordx2 v[2:3], v1, s[6:7] offset:40
	global_load_dwordx2 v[6:7], v1, s[6:7]
	s_waitcnt vmcnt(1)
	v_and_b32_e32 v2, v2, v4
	v_and_b32_e32 v3, v3, v5
	v_mul_lo_u32 v3, v3, 24
	v_mul_hi_u32 v8, v2, 24
	v_mul_lo_u32 v2, v2, 24
	v_add_u32_e32 v3, v8, v3
	s_waitcnt vmcnt(0)
	v_add_co_u32_e32 v2, vcc, v6, v2
	v_addc_co_u32_e32 v3, vcc, v7, v3, vcc
	global_load_dwordx2 v[2:3], v[2:3], off glc
	s_waitcnt vmcnt(0)
	global_atomic_cmpswap_x2 v[6:7], v1, v[2:5], s[6:7] offset:24 glc
	s_waitcnt vmcnt(0)
	buffer_wbinvl1_vol
	v_cmp_ne_u64_e32 vcc, v[6:7], v[4:5]
	s_and_saveexec_b64 s[10:11], vcc
	s_cbranch_execz .LBB2_5
; %bb.2:
	s_mov_b64 s[12:13], 0
.LBB2_3:                                ; =>This Inner Loop Header: Depth=1
	s_sleep 1
	global_load_dwordx2 v[2:3], v1, s[6:7] offset:40
	global_load_dwordx2 v[8:9], v1, s[6:7]
	v_mov_b32_e32 v4, v6
	v_mov_b32_e32 v5, v7
	s_waitcnt vmcnt(1)
	v_and_b32_e32 v2, v2, v4
	s_waitcnt vmcnt(0)
	v_mad_u64_u32 v[6:7], s[14:15], v2, 24, v[8:9]
	v_and_b32_e32 v3, v3, v5
	v_mov_b32_e32 v2, v7
	v_mad_u64_u32 v[2:3], s[14:15], v3, 24, v[2:3]
	v_mov_b32_e32 v7, v2
	global_load_dwordx2 v[2:3], v[6:7], off glc
	s_waitcnt vmcnt(0)
	global_atomic_cmpswap_x2 v[6:7], v1, v[2:5], s[6:7] offset:24 glc
	s_waitcnt vmcnt(0)
	buffer_wbinvl1_vol
	v_cmp_eq_u64_e32 vcc, v[6:7], v[4:5]
	s_or_b64 s[12:13], vcc, s[12:13]
	s_andn2_b64 exec, exec, s[12:13]
	s_cbranch_execnz .LBB2_3
; %bb.4:
	s_or_b64 exec, exec, s[12:13]
.LBB2_5:
	s_or_b64 exec, exec, s[10:11]
.LBB2_6:
	s_or_b64 exec, exec, s[8:9]
	v_mov_b32_e32 v5, 0
	s_waitcnt lgkmcnt(0)
	global_load_dwordx2 v[8:9], v5, s[6:7] offset:40
	global_load_dwordx4 v[1:4], v5, s[6:7]
	v_readfirstlane_b32 s8, v6
	v_readfirstlane_b32 s9, v7
	s_mov_b64 s[10:11], exec
	s_waitcnt vmcnt(1)
	v_readfirstlane_b32 s12, v8
	v_readfirstlane_b32 s13, v9
	s_and_b64 s[12:13], s[8:9], s[12:13]
	s_mul_i32 s14, s13, 24
	s_mul_hi_u32 s15, s12, 24
	s_mul_i32 s16, s12, 24
	s_add_i32 s14, s15, s14
	v_mov_b32_e32 v6, s14
	s_waitcnt vmcnt(0)
	v_add_co_u32_e32 v8, vcc, s16, v1
	v_addc_co_u32_e32 v9, vcc, v2, v6, vcc
	s_and_saveexec_b64 s[14:15], s[4:5]
	s_cbranch_execz .LBB2_8
; %bb.7:
	v_mov_b32_e32 v10, s10
	v_mov_b32_e32 v11, s11
	v_mov_b32_e32 v12, 2
	v_mov_b32_e32 v13, 1
	global_store_dwordx4 v[8:9], v[10:13], off offset:8
.LBB2_8:
	s_or_b64 exec, exec, s[14:15]
	s_lshl_b64 s[10:11], s[12:13], 12
	v_mov_b32_e32 v6, s11
	v_add_co_u32_e32 v3, vcc, s10, v3
	v_addc_co_u32_e32 v12, vcc, v4, v6, vcc
	s_mov_b32 s12, 0
	v_lshlrev_b32_e32 v30, 6, v31
	v_mov_b32_e32 v4, 33
	v_mov_b32_e32 v6, v5
	v_mov_b32_e32 v7, v5
	v_readfirstlane_b32 s10, v3
	v_readfirstlane_b32 s11, v12
	v_add_co_u32_e32 v10, vcc, v3, v30
	s_mov_b32 s13, s12
	s_mov_b32 s14, s12
	s_mov_b32 s15, s12
	s_nop 0
	global_store_dwordx4 v30, v[4:7], s[10:11]
	v_mov_b32_e32 v3, s12
	v_addc_co_u32_e32 v11, vcc, 0, v12, vcc
	v_mov_b32_e32 v4, s13
	v_mov_b32_e32 v5, s14
	v_mov_b32_e32 v6, s15
	global_store_dwordx4 v30, v[3:6], s[10:11] offset:16
	global_store_dwordx4 v30, v[3:6], s[10:11] offset:32
	global_store_dwordx4 v30, v[3:6], s[10:11] offset:48
	s_and_saveexec_b64 s[10:11], s[4:5]
	s_cbranch_execz .LBB2_16
; %bb.9:
	v_mov_b32_e32 v7, 0
	global_load_dwordx2 v[14:15], v7, s[6:7] offset:32 glc
	global_load_dwordx2 v[3:4], v7, s[6:7] offset:40
	v_mov_b32_e32 v12, s8
	v_mov_b32_e32 v13, s9
	s_waitcnt vmcnt(0)
	v_and_b32_e32 v3, s8, v3
	v_and_b32_e32 v4, s9, v4
	v_mul_lo_u32 v4, v4, 24
	v_mul_hi_u32 v5, v3, 24
	v_mul_lo_u32 v3, v3, 24
	v_add_u32_e32 v4, v5, v4
	v_add_co_u32_e32 v5, vcc, v1, v3
	v_addc_co_u32_e32 v6, vcc, v2, v4, vcc
	global_store_dwordx2 v[5:6], v[14:15], off
	s_waitcnt vmcnt(0)
	global_atomic_cmpswap_x2 v[3:4], v7, v[12:15], s[6:7] offset:32 glc
	s_waitcnt vmcnt(0)
	v_cmp_ne_u64_e32 vcc, v[3:4], v[14:15]
	s_and_saveexec_b64 s[12:13], vcc
	s_cbranch_execz .LBB2_12
; %bb.10:
	s_mov_b64 s[14:15], 0
.LBB2_11:                               ; =>This Inner Loop Header: Depth=1
	s_sleep 1
	global_store_dwordx2 v[5:6], v[3:4], off
	v_mov_b32_e32 v1, s8
	v_mov_b32_e32 v2, s9
	s_waitcnt vmcnt(0)
	global_atomic_cmpswap_x2 v[1:2], v7, v[1:4], s[6:7] offset:32 glc
	s_waitcnt vmcnt(0)
	v_cmp_eq_u64_e32 vcc, v[1:2], v[3:4]
	v_mov_b32_e32 v4, v2
	s_or_b64 s[14:15], vcc, s[14:15]
	v_mov_b32_e32 v3, v1
	s_andn2_b64 exec, exec, s[14:15]
	s_cbranch_execnz .LBB2_11
.LBB2_12:
	s_or_b64 exec, exec, s[12:13]
	v_mov_b32_e32 v4, 0
	global_load_dwordx2 v[1:2], v4, s[6:7] offset:16
	s_mov_b64 s[12:13], exec
	v_mbcnt_lo_u32_b32 v3, s12, 0
	v_mbcnt_hi_u32_b32 v3, s13, v3
	v_cmp_eq_u32_e32 vcc, 0, v3
	s_and_saveexec_b64 s[14:15], vcc
	s_cbranch_execz .LBB2_14
; %bb.13:
	s_bcnt1_i32_b64 s12, s[12:13]
	v_mov_b32_e32 v3, s12
	s_waitcnt vmcnt(0)
	global_atomic_add_x2 v[1:2], v[3:4], off offset:8
.LBB2_14:
	s_or_b64 exec, exec, s[14:15]
	s_waitcnt vmcnt(0)
	global_load_dwordx2 v[3:4], v[1:2], off offset:16
	s_waitcnt vmcnt(0)
	v_cmp_eq_u64_e32 vcc, 0, v[3:4]
	s_cbranch_vccnz .LBB2_16
; %bb.15:
	global_load_dword v1, v[1:2], off offset:24
	v_mov_b32_e32 v2, 0
	s_waitcnt vmcnt(0)
	global_store_dwordx2 v[3:4], v[1:2], off
	v_and_b32_e32 v1, 0xffffff, v1
	v_readfirstlane_b32 s12, v1
	s_mov_b32 m0, s12
	s_nop 0
	s_sendmsg sendmsg(MSG_INTERRUPT)
.LBB2_16:
	s_or_b64 exec, exec, s[10:11]
	s_branch .LBB2_20
.LBB2_17:                               ;   in Loop: Header=BB2_20 Depth=1
	s_or_b64 exec, exec, s[10:11]
	v_readfirstlane_b32 s10, v1
	s_cmp_eq_u32 s10, 0
	s_cbranch_scc1 .LBB2_19
; %bb.18:                               ;   in Loop: Header=BB2_20 Depth=1
	s_sleep 1
	s_cbranch_execnz .LBB2_20
	s_branch .LBB2_22
.LBB2_19:
	s_branch .LBB2_22
.LBB2_20:                               ; =>This Inner Loop Header: Depth=1
	v_mov_b32_e32 v1, 1
	s_and_saveexec_b64 s[10:11], s[4:5]
	s_cbranch_execz .LBB2_17
; %bb.21:                               ;   in Loop: Header=BB2_20 Depth=1
	global_load_dword v1, v[8:9], off offset:20 glc
	s_waitcnt vmcnt(0)
	buffer_wbinvl1_vol
	v_and_b32_e32 v1, 1, v1
	s_branch .LBB2_17
.LBB2_22:
	global_load_dwordx2 v[1:2], v[10:11], off
	s_and_saveexec_b64 s[10:11], s[4:5]
	s_cbranch_execz .LBB2_25
; %bb.23:
	v_mov_b32_e32 v9, 0
	global_load_dwordx2 v[5:6], v9, s[6:7] offset:40
	global_load_dwordx2 v[10:11], v9, s[6:7] offset:24 glc
	global_load_dwordx2 v[7:8], v9, s[6:7]
	v_mov_b32_e32 v4, s9
	s_mov_b64 s[4:5], 0
	s_waitcnt vmcnt(2)
	v_add_co_u32_e32 v12, vcc, 1, v5
	v_addc_co_u32_e32 v13, vcc, 0, v6, vcc
	v_add_co_u32_e32 v3, vcc, s8, v12
	v_addc_co_u32_e32 v4, vcc, v13, v4, vcc
	v_cmp_eq_u64_e32 vcc, 0, v[3:4]
	v_cndmask_b32_e32 v4, v4, v13, vcc
	v_cndmask_b32_e32 v3, v3, v12, vcc
	v_and_b32_e32 v6, v4, v6
	v_and_b32_e32 v5, v3, v5
	v_mul_lo_u32 v6, v6, 24
	v_mul_hi_u32 v12, v5, 24
	v_mul_lo_u32 v13, v5, 24
	s_waitcnt vmcnt(1)
	v_mov_b32_e32 v5, v10
	v_add_u32_e32 v6, v12, v6
	s_waitcnt vmcnt(0)
	v_add_co_u32_e32 v7, vcc, v7, v13
	v_addc_co_u32_e32 v8, vcc, v8, v6, vcc
	global_store_dwordx2 v[7:8], v[10:11], off
	v_mov_b32_e32 v6, v11
	s_waitcnt vmcnt(0)
	global_atomic_cmpswap_x2 v[5:6], v9, v[3:6], s[6:7] offset:24 glc
	s_waitcnt vmcnt(0)
	v_cmp_ne_u64_e32 vcc, v[5:6], v[10:11]
	s_and_b64 exec, exec, vcc
	s_cbranch_execz .LBB2_25
.LBB2_24:                               ; =>This Inner Loop Header: Depth=1
	s_sleep 1
	global_store_dwordx2 v[7:8], v[5:6], off
	s_waitcnt vmcnt(0)
	global_atomic_cmpswap_x2 v[10:11], v9, v[3:6], s[6:7] offset:24 glc
	s_waitcnt vmcnt(0)
	v_cmp_eq_u64_e32 vcc, v[10:11], v[5:6]
	v_mov_b32_e32 v5, v10
	s_or_b64 s[4:5], vcc, s[4:5]
	v_mov_b32_e32 v6, v11
	s_andn2_b64 exec, exec, s[4:5]
	s_cbranch_execnz .LBB2_24
.LBB2_25:
	s_or_b64 exec, exec, s[10:11]
	s_getpc_b64 s[8:9]
	s_add_u32 s8, s8, .str.2@rel32@lo+4
	s_addc_u32 s9, s9, .str.2@rel32@hi+12
	s_cmp_lg_u64 s[8:9], 0
	s_cbranch_scc0 .LBB2_110
; %bb.26:
	s_waitcnt vmcnt(0)
	v_and_b32_e32 v32, 2, v1
	v_mov_b32_e32 v27, 0
	v_and_b32_e32 v3, -3, v1
	v_mov_b32_e32 v4, v2
	s_mov_b64 s[10:11], 14
	v_mov_b32_e32 v9, 2
	v_mov_b32_e32 v10, 1
	s_branch .LBB2_28
.LBB2_27:                               ;   in Loop: Header=BB2_28 Depth=1
	s_or_b64 exec, exec, s[16:17]
	s_sub_u32 s10, s10, s12
	s_subb_u32 s11, s11, s13
	s_add_u32 s8, s8, s12
	s_addc_u32 s9, s9, s13
	s_cmp_lg_u64 s[10:11], 0
	s_cbranch_scc0 .LBB2_109
.LBB2_28:                               ; =>This Loop Header: Depth=1
                                        ;     Child Loop BB2_31 Depth 2
                                        ;     Child Loop BB2_37 Depth 2
                                        ;     Child Loop BB2_46 Depth 2
                                        ;     Child Loop BB2_54 Depth 2
                                        ;     Child Loop BB2_62 Depth 2
                                        ;     Child Loop BB2_70 Depth 2
                                        ;     Child Loop BB2_78 Depth 2
                                        ;     Child Loop BB2_86 Depth 2
                                        ;     Child Loop BB2_94 Depth 2
                                        ;     Child Loop BB2_103 Depth 2
                                        ;     Child Loop BB2_108 Depth 2
	v_cmp_lt_u64_e64 s[4:5], s[10:11], 56
	v_cmp_gt_u64_e64 s[14:15], s[10:11], 7
	s_and_b64 s[4:5], s[4:5], exec
	s_cselect_b32 s13, s11, 0
	s_cselect_b32 s12, s10, 56
	s_and_b64 vcc, exec, s[14:15]
	s_cbranch_vccnz .LBB2_38
; %bb.29:                               ;   in Loop: Header=BB2_28 Depth=1
	v_mov_b32_e32 v5, 0
	s_cmp_eq_u64 s[10:11], 0
	v_mov_b32_e32 v6, 0
	s_mov_b64 s[4:5], 0
	s_cbranch_scc1 .LBB2_32
; %bb.30:                               ;   in Loop: Header=BB2_28 Depth=1
	v_mov_b32_e32 v5, 0
	s_lshl_b64 s[14:15], s[12:13], 3
	s_mov_b64 s[16:17], 0
	v_mov_b32_e32 v6, 0
	s_mov_b64 s[18:19], s[8:9]
.LBB2_31:                               ;   Parent Loop BB2_28 Depth=1
                                        ; =>  This Inner Loop Header: Depth=2
	global_load_ubyte v7, v27, s[18:19]
	s_waitcnt vmcnt(0)
	v_and_b32_e32 v26, 0xffff, v7
	v_lshlrev_b64 v[7:8], s16, v[26:27]
	s_add_u32 s16, s16, 8
	s_addc_u32 s17, s17, 0
	s_add_u32 s18, s18, 1
	s_addc_u32 s19, s19, 0
	v_or_b32_e32 v5, v7, v5
	s_cmp_lg_u32 s14, s16
	v_or_b32_e32 v6, v8, v6
	s_cbranch_scc1 .LBB2_31
.LBB2_32:                               ;   in Loop: Header=BB2_28 Depth=1
	s_mov_b32 s18, 0
	s_andn2_b64 vcc, exec, s[4:5]
	s_mov_b64 s[4:5], s[8:9]
	s_cbranch_vccnz .LBB2_34
.LBB2_33:                               ;   in Loop: Header=BB2_28 Depth=1
	global_load_dwordx2 v[5:6], v27, s[8:9]
	s_add_i32 s18, s12, -8
	s_add_u32 s4, s8, 8
	s_addc_u32 s5, s9, 0
.LBB2_34:                               ;   in Loop: Header=BB2_28 Depth=1
	s_cmp_gt_u32 s18, 7
	s_cbranch_scc1 .LBB2_39
; %bb.35:                               ;   in Loop: Header=BB2_28 Depth=1
	s_cmp_eq_u32 s18, 0
	s_cbranch_scc1 .LBB2_40
; %bb.36:                               ;   in Loop: Header=BB2_28 Depth=1
	v_mov_b32_e32 v11, 0
	s_mov_b64 s[14:15], 0
	v_mov_b32_e32 v12, 0
	s_mov_b64 s[16:17], 0
.LBB2_37:                               ;   Parent Loop BB2_28 Depth=1
                                        ; =>  This Inner Loop Header: Depth=2
	s_add_u32 s20, s4, s16
	s_addc_u32 s21, s5, s17
	global_load_ubyte v7, v27, s[20:21]
	s_add_u32 s16, s16, 1
	s_addc_u32 s17, s17, 0
	s_waitcnt vmcnt(0)
	v_and_b32_e32 v26, 0xffff, v7
	v_lshlrev_b64 v[7:8], s14, v[26:27]
	s_add_u32 s14, s14, 8
	s_addc_u32 s15, s15, 0
	v_or_b32_e32 v11, v7, v11
	s_cmp_lg_u32 s18, s16
	v_or_b32_e32 v12, v8, v12
	s_cbranch_scc1 .LBB2_37
	s_branch .LBB2_41
.LBB2_38:                               ;   in Loop: Header=BB2_28 Depth=1
	s_mov_b32 s18, 0
	s_mov_b64 s[4:5], s[8:9]
	s_branch .LBB2_33
.LBB2_39:                               ;   in Loop: Header=BB2_28 Depth=1
                                        ; implicit-def: $vgpr11_vgpr12
	s_mov_b32 s19, 0
	s_branch .LBB2_42
.LBB2_40:                               ;   in Loop: Header=BB2_28 Depth=1
	v_mov_b32_e32 v11, 0
	v_mov_b32_e32 v12, 0
.LBB2_41:                               ;   in Loop: Header=BB2_28 Depth=1
	s_mov_b32 s19, 0
	s_cbranch_execnz .LBB2_43
.LBB2_42:                               ;   in Loop: Header=BB2_28 Depth=1
	global_load_dwordx2 v[11:12], v27, s[4:5]
	s_add_i32 s19, s18, -8
	s_add_u32 s4, s4, 8
	s_addc_u32 s5, s5, 0
.LBB2_43:                               ;   in Loop: Header=BB2_28 Depth=1
	s_cmp_gt_u32 s19, 7
	s_cbranch_scc1 .LBB2_47
; %bb.44:                               ;   in Loop: Header=BB2_28 Depth=1
	s_cmp_eq_u32 s19, 0
	s_cbranch_scc1 .LBB2_48
; %bb.45:                               ;   in Loop: Header=BB2_28 Depth=1
	v_mov_b32_e32 v13, 0
	s_mov_b64 s[14:15], 0
	v_mov_b32_e32 v14, 0
	s_mov_b64 s[16:17], 0
.LBB2_46:                               ;   Parent Loop BB2_28 Depth=1
                                        ; =>  This Inner Loop Header: Depth=2
	s_add_u32 s20, s4, s16
	s_addc_u32 s21, s5, s17
	global_load_ubyte v7, v27, s[20:21]
	s_add_u32 s16, s16, 1
	s_addc_u32 s17, s17, 0
	s_waitcnt vmcnt(0)
	v_and_b32_e32 v26, 0xffff, v7
	v_lshlrev_b64 v[7:8], s14, v[26:27]
	s_add_u32 s14, s14, 8
	s_addc_u32 s15, s15, 0
	v_or_b32_e32 v13, v7, v13
	s_cmp_lg_u32 s19, s16
	v_or_b32_e32 v14, v8, v14
	s_cbranch_scc1 .LBB2_46
	s_branch .LBB2_49
.LBB2_47:                               ;   in Loop: Header=BB2_28 Depth=1
	s_mov_b32 s18, 0
	s_branch .LBB2_50
.LBB2_48:                               ;   in Loop: Header=BB2_28 Depth=1
	v_mov_b32_e32 v13, 0
	v_mov_b32_e32 v14, 0
.LBB2_49:                               ;   in Loop: Header=BB2_28 Depth=1
	s_mov_b32 s18, 0
	s_cbranch_execnz .LBB2_51
.LBB2_50:                               ;   in Loop: Header=BB2_28 Depth=1
	global_load_dwordx2 v[13:14], v27, s[4:5]
	s_add_i32 s18, s19, -8
	s_add_u32 s4, s4, 8
	s_addc_u32 s5, s5, 0
.LBB2_51:                               ;   in Loop: Header=BB2_28 Depth=1
	s_cmp_gt_u32 s18, 7
	s_cbranch_scc1 .LBB2_55
; %bb.52:                               ;   in Loop: Header=BB2_28 Depth=1
	s_cmp_eq_u32 s18, 0
	s_cbranch_scc1 .LBB2_56
; %bb.53:                               ;   in Loop: Header=BB2_28 Depth=1
	v_mov_b32_e32 v15, 0
	s_mov_b64 s[14:15], 0
	v_mov_b32_e32 v16, 0
	s_mov_b64 s[16:17], 0
.LBB2_54:                               ;   Parent Loop BB2_28 Depth=1
                                        ; =>  This Inner Loop Header: Depth=2
	s_add_u32 s20, s4, s16
	s_addc_u32 s21, s5, s17
	global_load_ubyte v7, v27, s[20:21]
	s_add_u32 s16, s16, 1
	s_addc_u32 s17, s17, 0
	s_waitcnt vmcnt(0)
	v_and_b32_e32 v26, 0xffff, v7
	v_lshlrev_b64 v[7:8], s14, v[26:27]
	s_add_u32 s14, s14, 8
	s_addc_u32 s15, s15, 0
	v_or_b32_e32 v15, v7, v15
	s_cmp_lg_u32 s18, s16
	v_or_b32_e32 v16, v8, v16
	s_cbranch_scc1 .LBB2_54
	s_branch .LBB2_57
.LBB2_55:                               ;   in Loop: Header=BB2_28 Depth=1
                                        ; implicit-def: $vgpr15_vgpr16
	s_mov_b32 s19, 0
	s_branch .LBB2_58
.LBB2_56:                               ;   in Loop: Header=BB2_28 Depth=1
	v_mov_b32_e32 v15, 0
	v_mov_b32_e32 v16, 0
.LBB2_57:                               ;   in Loop: Header=BB2_28 Depth=1
	s_mov_b32 s19, 0
	s_cbranch_execnz .LBB2_59
.LBB2_58:                               ;   in Loop: Header=BB2_28 Depth=1
	global_load_dwordx2 v[15:16], v27, s[4:5]
	s_add_i32 s19, s18, -8
	s_add_u32 s4, s4, 8
	s_addc_u32 s5, s5, 0
.LBB2_59:                               ;   in Loop: Header=BB2_28 Depth=1
	s_cmp_gt_u32 s19, 7
	s_cbranch_scc1 .LBB2_63
; %bb.60:                               ;   in Loop: Header=BB2_28 Depth=1
	s_cmp_eq_u32 s19, 0
	s_cbranch_scc1 .LBB2_64
; %bb.61:                               ;   in Loop: Header=BB2_28 Depth=1
	v_mov_b32_e32 v17, 0
	s_mov_b64 s[14:15], 0
	v_mov_b32_e32 v18, 0
	s_mov_b64 s[16:17], 0
.LBB2_62:                               ;   Parent Loop BB2_28 Depth=1
                                        ; =>  This Inner Loop Header: Depth=2
	s_add_u32 s20, s4, s16
	s_addc_u32 s21, s5, s17
	global_load_ubyte v7, v27, s[20:21]
	s_add_u32 s16, s16, 1
	s_addc_u32 s17, s17, 0
	s_waitcnt vmcnt(0)
	v_and_b32_e32 v26, 0xffff, v7
	v_lshlrev_b64 v[7:8], s14, v[26:27]
	s_add_u32 s14, s14, 8
	s_addc_u32 s15, s15, 0
	v_or_b32_e32 v17, v7, v17
	s_cmp_lg_u32 s19, s16
	v_or_b32_e32 v18, v8, v18
	s_cbranch_scc1 .LBB2_62
	s_branch .LBB2_65
.LBB2_63:                               ;   in Loop: Header=BB2_28 Depth=1
	s_mov_b32 s18, 0
	s_branch .LBB2_66
.LBB2_64:                               ;   in Loop: Header=BB2_28 Depth=1
	v_mov_b32_e32 v17, 0
	v_mov_b32_e32 v18, 0
.LBB2_65:                               ;   in Loop: Header=BB2_28 Depth=1
	s_mov_b32 s18, 0
	s_cbranch_execnz .LBB2_67
.LBB2_66:                               ;   in Loop: Header=BB2_28 Depth=1
	global_load_dwordx2 v[17:18], v27, s[4:5]
	s_add_i32 s18, s19, -8
	s_add_u32 s4, s4, 8
	s_addc_u32 s5, s5, 0
.LBB2_67:                               ;   in Loop: Header=BB2_28 Depth=1
	s_cmp_gt_u32 s18, 7
	s_cbranch_scc1 .LBB2_71
; %bb.68:                               ;   in Loop: Header=BB2_28 Depth=1
	s_cmp_eq_u32 s18, 0
	s_cbranch_scc1 .LBB2_72
; %bb.69:                               ;   in Loop: Header=BB2_28 Depth=1
	v_mov_b32_e32 v19, 0
	s_mov_b64 s[14:15], 0
	v_mov_b32_e32 v20, 0
	s_mov_b64 s[16:17], 0
.LBB2_70:                               ;   Parent Loop BB2_28 Depth=1
                                        ; =>  This Inner Loop Header: Depth=2
	s_add_u32 s20, s4, s16
	s_addc_u32 s21, s5, s17
	global_load_ubyte v7, v27, s[20:21]
	s_add_u32 s16, s16, 1
	s_addc_u32 s17, s17, 0
	s_waitcnt vmcnt(0)
	v_and_b32_e32 v26, 0xffff, v7
	v_lshlrev_b64 v[7:8], s14, v[26:27]
	s_add_u32 s14, s14, 8
	s_addc_u32 s15, s15, 0
	v_or_b32_e32 v19, v7, v19
	s_cmp_lg_u32 s18, s16
	v_or_b32_e32 v20, v8, v20
	s_cbranch_scc1 .LBB2_70
	s_branch .LBB2_73
.LBB2_71:                               ;   in Loop: Header=BB2_28 Depth=1
                                        ; implicit-def: $vgpr19_vgpr20
	s_mov_b32 s19, 0
	s_branch .LBB2_74
.LBB2_72:                               ;   in Loop: Header=BB2_28 Depth=1
	v_mov_b32_e32 v19, 0
	v_mov_b32_e32 v20, 0
.LBB2_73:                               ;   in Loop: Header=BB2_28 Depth=1
	s_mov_b32 s19, 0
	s_cbranch_execnz .LBB2_75
.LBB2_74:                               ;   in Loop: Header=BB2_28 Depth=1
	global_load_dwordx2 v[19:20], v27, s[4:5]
	s_add_i32 s19, s18, -8
	s_add_u32 s4, s4, 8
	s_addc_u32 s5, s5, 0
.LBB2_75:                               ;   in Loop: Header=BB2_28 Depth=1
	s_cmp_gt_u32 s19, 7
	s_cbranch_scc1 .LBB2_79
; %bb.76:                               ;   in Loop: Header=BB2_28 Depth=1
	s_cmp_eq_u32 s19, 0
	s_cbranch_scc1 .LBB2_80
; %bb.77:                               ;   in Loop: Header=BB2_28 Depth=1
	v_mov_b32_e32 v21, 0
	s_mov_b64 s[14:15], 0
	v_mov_b32_e32 v22, 0
	s_mov_b64 s[16:17], s[4:5]
.LBB2_78:                               ;   Parent Loop BB2_28 Depth=1
                                        ; =>  This Inner Loop Header: Depth=2
	global_load_ubyte v7, v27, s[16:17]
	s_add_i32 s19, s19, -1
	s_waitcnt vmcnt(0)
	v_and_b32_e32 v26, 0xffff, v7
	v_lshlrev_b64 v[7:8], s14, v[26:27]
	s_add_u32 s14, s14, 8
	s_addc_u32 s15, s15, 0
	s_add_u32 s16, s16, 1
	s_addc_u32 s17, s17, 0
	v_or_b32_e32 v21, v7, v21
	s_cmp_lg_u32 s19, 0
	v_or_b32_e32 v22, v8, v22
	s_cbranch_scc1 .LBB2_78
	s_branch .LBB2_81
.LBB2_79:                               ;   in Loop: Header=BB2_28 Depth=1
	s_branch .LBB2_82
.LBB2_80:                               ;   in Loop: Header=BB2_28 Depth=1
	v_mov_b32_e32 v21, 0
	v_mov_b32_e32 v22, 0
.LBB2_81:                               ;   in Loop: Header=BB2_28 Depth=1
	s_cbranch_execnz .LBB2_83
.LBB2_82:                               ;   in Loop: Header=BB2_28 Depth=1
	global_load_dwordx2 v[21:22], v27, s[4:5]
.LBB2_83:                               ;   in Loop: Header=BB2_28 Depth=1
	v_readfirstlane_b32 s4, v31
	v_mov_b32_e32 v7, 0
	v_mov_b32_e32 v8, 0
	v_cmp_eq_u32_e64 s[4:5], s4, v31
	s_and_saveexec_b64 s[14:15], s[4:5]
	s_cbranch_execz .LBB2_89
; %bb.84:                               ;   in Loop: Header=BB2_28 Depth=1
	global_load_dwordx2 v[25:26], v27, s[6:7] offset:24 glc
	s_waitcnt vmcnt(0)
	buffer_wbinvl1_vol
	global_load_dwordx2 v[7:8], v27, s[6:7] offset:40
	global_load_dwordx2 v[23:24], v27, s[6:7]
	s_waitcnt vmcnt(1)
	v_and_b32_e32 v7, v7, v25
	v_and_b32_e32 v8, v8, v26
	v_mul_lo_u32 v8, v8, 24
	v_mul_hi_u32 v28, v7, 24
	v_mul_lo_u32 v7, v7, 24
	v_add_u32_e32 v8, v28, v8
	s_waitcnt vmcnt(0)
	v_add_co_u32_e32 v7, vcc, v23, v7
	v_addc_co_u32_e32 v8, vcc, v24, v8, vcc
	global_load_dwordx2 v[23:24], v[7:8], off glc
	s_waitcnt vmcnt(0)
	global_atomic_cmpswap_x2 v[7:8], v27, v[23:26], s[6:7] offset:24 glc
	s_waitcnt vmcnt(0)
	buffer_wbinvl1_vol
	v_cmp_ne_u64_e32 vcc, v[7:8], v[25:26]
	s_and_saveexec_b64 s[16:17], vcc
	s_cbranch_execz .LBB2_88
; %bb.85:                               ;   in Loop: Header=BB2_28 Depth=1
	s_mov_b64 s[18:19], 0
.LBB2_86:                               ;   Parent Loop BB2_28 Depth=1
                                        ; =>  This Inner Loop Header: Depth=2
	s_sleep 1
	global_load_dwordx2 v[23:24], v27, s[6:7] offset:40
	global_load_dwordx2 v[28:29], v27, s[6:7]
	v_mov_b32_e32 v26, v8
	v_mov_b32_e32 v25, v7
	s_waitcnt vmcnt(1)
	v_and_b32_e32 v7, v23, v25
	s_waitcnt vmcnt(0)
	v_mad_u64_u32 v[7:8], s[20:21], v7, 24, v[28:29]
	v_and_b32_e32 v23, v24, v26
	v_mad_u64_u32 v[23:24], s[20:21], v23, 24, v[8:9]
	v_mov_b32_e32 v8, v23
	global_load_dwordx2 v[23:24], v[7:8], off glc
	s_waitcnt vmcnt(0)
	global_atomic_cmpswap_x2 v[7:8], v27, v[23:26], s[6:7] offset:24 glc
	s_waitcnt vmcnt(0)
	buffer_wbinvl1_vol
	v_cmp_eq_u64_e32 vcc, v[7:8], v[25:26]
	s_or_b64 s[18:19], vcc, s[18:19]
	s_andn2_b64 exec, exec, s[18:19]
	s_cbranch_execnz .LBB2_86
; %bb.87:                               ;   in Loop: Header=BB2_28 Depth=1
	s_or_b64 exec, exec, s[18:19]
.LBB2_88:                               ;   in Loop: Header=BB2_28 Depth=1
	s_or_b64 exec, exec, s[16:17]
.LBB2_89:                               ;   in Loop: Header=BB2_28 Depth=1
	s_or_b64 exec, exec, s[14:15]
	global_load_dwordx2 v[28:29], v27, s[6:7] offset:40
	global_load_dwordx4 v[23:26], v27, s[6:7]
	v_readfirstlane_b32 s14, v7
	v_readfirstlane_b32 s15, v8
	s_mov_b64 s[16:17], exec
	s_waitcnt vmcnt(1)
	v_readfirstlane_b32 s18, v28
	v_readfirstlane_b32 s19, v29
	s_and_b64 s[18:19], s[14:15], s[18:19]
	s_mul_i32 s20, s19, 24
	s_mul_hi_u32 s21, s18, 24
	s_mul_i32 s22, s18, 24
	s_add_i32 s20, s21, s20
	v_mov_b32_e32 v7, s20
	s_waitcnt vmcnt(0)
	v_add_co_u32_e32 v28, vcc, s22, v23
	v_addc_co_u32_e32 v29, vcc, v24, v7, vcc
	s_and_saveexec_b64 s[20:21], s[4:5]
	s_cbranch_execz .LBB2_91
; %bb.90:                               ;   in Loop: Header=BB2_28 Depth=1
	v_mov_b32_e32 v7, s16
	v_mov_b32_e32 v8, s17
	global_store_dwordx4 v[28:29], v[7:10], off offset:8
.LBB2_91:                               ;   in Loop: Header=BB2_28 Depth=1
	s_or_b64 exec, exec, s[20:21]
	s_lshl_b64 s[16:17], s[18:19], 12
	v_mov_b32_e32 v7, s17
	v_add_co_u32_e32 v25, vcc, s16, v25
	v_addc_co_u32_e32 v33, vcc, v26, v7, vcc
	v_cmp_lt_u64_e64 vcc, s[10:11], 57
	s_lshl_b32 s16, s12, 2
	v_cndmask_b32_e32 v7, 0, v32, vcc
	s_add_i32 s16, s16, 28
	v_and_b32_e32 v3, 0xffffff1f, v3
	s_and_b32 s16, s16, 0x1e0
	v_or_b32_e32 v3, v3, v7
	v_or_b32_e32 v3, s16, v3
	v_readfirstlane_b32 s16, v25
	v_readfirstlane_b32 s17, v33
	s_nop 4
	global_store_dwordx4 v30, v[3:6], s[16:17]
	global_store_dwordx4 v30, v[11:14], s[16:17] offset:16
	global_store_dwordx4 v30, v[15:18], s[16:17] offset:32
	global_store_dwordx4 v30, v[19:22], s[16:17] offset:48
	s_and_saveexec_b64 s[16:17], s[4:5]
	s_cbranch_execz .LBB2_99
; %bb.92:                               ;   in Loop: Header=BB2_28 Depth=1
	global_load_dwordx2 v[13:14], v27, s[6:7] offset:32 glc
	global_load_dwordx2 v[3:4], v27, s[6:7] offset:40
	v_mov_b32_e32 v11, s14
	v_mov_b32_e32 v12, s15
	s_waitcnt vmcnt(0)
	v_readfirstlane_b32 s18, v3
	v_readfirstlane_b32 s19, v4
	s_and_b64 s[18:19], s[18:19], s[14:15]
	s_mul_i32 s19, s19, 24
	s_mul_hi_u32 s20, s18, 24
	s_mul_i32 s18, s18, 24
	s_add_i32 s19, s20, s19
	v_mov_b32_e32 v3, s19
	v_add_co_u32_e32 v7, vcc, s18, v23
	v_addc_co_u32_e32 v8, vcc, v24, v3, vcc
	global_store_dwordx2 v[7:8], v[13:14], off
	s_waitcnt vmcnt(0)
	global_atomic_cmpswap_x2 v[5:6], v27, v[11:14], s[6:7] offset:32 glc
	s_waitcnt vmcnt(0)
	v_cmp_ne_u64_e32 vcc, v[5:6], v[13:14]
	s_and_saveexec_b64 s[18:19], vcc
	s_cbranch_execz .LBB2_95
; %bb.93:                               ;   in Loop: Header=BB2_28 Depth=1
	s_mov_b64 s[20:21], 0
.LBB2_94:                               ;   Parent Loop BB2_28 Depth=1
                                        ; =>  This Inner Loop Header: Depth=2
	s_sleep 1
	global_store_dwordx2 v[7:8], v[5:6], off
	v_mov_b32_e32 v3, s14
	v_mov_b32_e32 v4, s15
	s_waitcnt vmcnt(0)
	global_atomic_cmpswap_x2 v[3:4], v27, v[3:6], s[6:7] offset:32 glc
	s_waitcnt vmcnt(0)
	v_cmp_eq_u64_e32 vcc, v[3:4], v[5:6]
	v_mov_b32_e32 v6, v4
	s_or_b64 s[20:21], vcc, s[20:21]
	v_mov_b32_e32 v5, v3
	s_andn2_b64 exec, exec, s[20:21]
	s_cbranch_execnz .LBB2_94
.LBB2_95:                               ;   in Loop: Header=BB2_28 Depth=1
	s_or_b64 exec, exec, s[18:19]
	global_load_dwordx2 v[3:4], v27, s[6:7] offset:16
	s_mov_b64 s[20:21], exec
	v_mbcnt_lo_u32_b32 v5, s20, 0
	v_mbcnt_hi_u32_b32 v5, s21, v5
	v_cmp_eq_u32_e32 vcc, 0, v5
	s_and_saveexec_b64 s[18:19], vcc
	s_cbranch_execz .LBB2_97
; %bb.96:                               ;   in Loop: Header=BB2_28 Depth=1
	s_bcnt1_i32_b64 s20, s[20:21]
	v_mov_b32_e32 v26, s20
	s_waitcnt vmcnt(0)
	global_atomic_add_x2 v[3:4], v[26:27], off offset:8
.LBB2_97:                               ;   in Loop: Header=BB2_28 Depth=1
	s_or_b64 exec, exec, s[18:19]
	s_waitcnt vmcnt(0)
	global_load_dwordx2 v[5:6], v[3:4], off offset:16
	s_waitcnt vmcnt(0)
	v_cmp_eq_u64_e32 vcc, 0, v[5:6]
	s_cbranch_vccnz .LBB2_99
; %bb.98:                               ;   in Loop: Header=BB2_28 Depth=1
	global_load_dword v26, v[3:4], off offset:24
	s_waitcnt vmcnt(0)
	v_and_b32_e32 v3, 0xffffff, v26
	v_readfirstlane_b32 s18, v3
	s_mov_b32 m0, s18
	global_store_dwordx2 v[5:6], v[26:27], off
	s_sendmsg sendmsg(MSG_INTERRUPT)
.LBB2_99:                               ;   in Loop: Header=BB2_28 Depth=1
	s_or_b64 exec, exec, s[16:17]
	v_add_co_u32_e32 v3, vcc, v25, v30
	v_addc_co_u32_e32 v4, vcc, 0, v33, vcc
	s_branch .LBB2_103
.LBB2_100:                              ;   in Loop: Header=BB2_103 Depth=2
	s_or_b64 exec, exec, s[16:17]
	v_readfirstlane_b32 s16, v5
	s_cmp_eq_u32 s16, 0
	s_cbranch_scc1 .LBB2_102
; %bb.101:                              ;   in Loop: Header=BB2_103 Depth=2
	s_sleep 1
	s_cbranch_execnz .LBB2_103
	s_branch .LBB2_105
.LBB2_102:                              ;   in Loop: Header=BB2_28 Depth=1
	s_branch .LBB2_105
.LBB2_103:                              ;   Parent Loop BB2_28 Depth=1
                                        ; =>  This Inner Loop Header: Depth=2
	v_mov_b32_e32 v5, 1
	s_and_saveexec_b64 s[16:17], s[4:5]
	s_cbranch_execz .LBB2_100
; %bb.104:                              ;   in Loop: Header=BB2_103 Depth=2
	global_load_dword v5, v[28:29], off offset:20 glc
	s_waitcnt vmcnt(0)
	buffer_wbinvl1_vol
	v_and_b32_e32 v5, 1, v5
	s_branch .LBB2_100
.LBB2_105:                              ;   in Loop: Header=BB2_28 Depth=1
	global_load_dwordx2 v[3:4], v[3:4], off
	s_and_saveexec_b64 s[16:17], s[4:5]
	s_cbranch_execz .LBB2_27
; %bb.106:                              ;   in Loop: Header=BB2_28 Depth=1
	global_load_dwordx2 v[7:8], v27, s[6:7] offset:40
	global_load_dwordx2 v[13:14], v27, s[6:7] offset:24 glc
	global_load_dwordx2 v[11:12], v27, s[6:7]
	v_mov_b32_e32 v6, s15
	s_waitcnt vmcnt(2)
	v_add_co_u32_e32 v15, vcc, 1, v7
	v_addc_co_u32_e32 v16, vcc, 0, v8, vcc
	v_add_co_u32_e32 v5, vcc, s14, v15
	v_addc_co_u32_e32 v6, vcc, v16, v6, vcc
	v_cmp_eq_u64_e32 vcc, 0, v[5:6]
	v_cndmask_b32_e32 v6, v6, v16, vcc
	v_cndmask_b32_e32 v5, v5, v15, vcc
	v_and_b32_e32 v8, v6, v8
	v_and_b32_e32 v7, v5, v7
	v_mul_lo_u32 v8, v8, 24
	v_mul_hi_u32 v15, v7, 24
	v_mul_lo_u32 v16, v7, 24
	s_waitcnt vmcnt(1)
	v_mov_b32_e32 v7, v13
	v_add_u32_e32 v8, v15, v8
	s_waitcnt vmcnt(0)
	v_add_co_u32_e32 v11, vcc, v11, v16
	v_addc_co_u32_e32 v12, vcc, v12, v8, vcc
	global_store_dwordx2 v[11:12], v[13:14], off
	v_mov_b32_e32 v8, v14
	s_waitcnt vmcnt(0)
	global_atomic_cmpswap_x2 v[7:8], v27, v[5:8], s[6:7] offset:24 glc
	s_waitcnt vmcnt(0)
	v_cmp_ne_u64_e32 vcc, v[7:8], v[13:14]
	s_and_b64 exec, exec, vcc
	s_cbranch_execz .LBB2_27
; %bb.107:                              ;   in Loop: Header=BB2_28 Depth=1
	s_mov_b64 s[4:5], 0
.LBB2_108:                              ;   Parent Loop BB2_28 Depth=1
                                        ; =>  This Inner Loop Header: Depth=2
	s_sleep 1
	global_store_dwordx2 v[11:12], v[7:8], off
	s_waitcnt vmcnt(0)
	global_atomic_cmpswap_x2 v[13:14], v27, v[5:8], s[6:7] offset:24 glc
	s_waitcnt vmcnt(0)
	v_cmp_eq_u64_e32 vcc, v[13:14], v[7:8]
	v_mov_b32_e32 v7, v13
	s_or_b64 s[4:5], vcc, s[4:5]
	v_mov_b32_e32 v8, v14
	s_andn2_b64 exec, exec, s[4:5]
	s_cbranch_execnz .LBB2_108
	s_branch .LBB2_27
.LBB2_109:
	s_branch .LBB2_137
.LBB2_110:
                                        ; implicit-def: $vgpr3_vgpr4
	s_cbranch_execz .LBB2_137
; %bb.111:
	v_readfirstlane_b32 s4, v31
	v_mov_b32_e32 v9, 0
	v_mov_b32_e32 v10, 0
	v_cmp_eq_u32_e64 s[4:5], s4, v31
	s_and_saveexec_b64 s[8:9], s[4:5]
	s_cbranch_execz .LBB2_117
; %bb.112:
	s_waitcnt vmcnt(0)
	v_mov_b32_e32 v3, 0
	global_load_dwordx2 v[6:7], v3, s[6:7] offset:24 glc
	s_waitcnt vmcnt(0)
	buffer_wbinvl1_vol
	global_load_dwordx2 v[4:5], v3, s[6:7] offset:40
	global_load_dwordx2 v[8:9], v3, s[6:7]
	s_waitcnt vmcnt(1)
	v_and_b32_e32 v4, v4, v6
	v_and_b32_e32 v5, v5, v7
	v_mul_lo_u32 v5, v5, 24
	v_mul_hi_u32 v10, v4, 24
	v_mul_lo_u32 v4, v4, 24
	v_add_u32_e32 v5, v10, v5
	s_waitcnt vmcnt(0)
	v_add_co_u32_e32 v4, vcc, v8, v4
	v_addc_co_u32_e32 v5, vcc, v9, v5, vcc
	global_load_dwordx2 v[4:5], v[4:5], off glc
	s_waitcnt vmcnt(0)
	global_atomic_cmpswap_x2 v[9:10], v3, v[4:7], s[6:7] offset:24 glc
	s_waitcnt vmcnt(0)
	buffer_wbinvl1_vol
	v_cmp_ne_u64_e32 vcc, v[9:10], v[6:7]
	s_and_saveexec_b64 s[10:11], vcc
	s_cbranch_execz .LBB2_116
; %bb.113:
	s_mov_b64 s[12:13], 0
.LBB2_114:                              ; =>This Inner Loop Header: Depth=1
	s_sleep 1
	global_load_dwordx2 v[4:5], v3, s[6:7] offset:40
	global_load_dwordx2 v[11:12], v3, s[6:7]
	v_mov_b32_e32 v6, v9
	v_mov_b32_e32 v7, v10
	s_waitcnt vmcnt(1)
	v_and_b32_e32 v4, v4, v6
	s_waitcnt vmcnt(0)
	v_mad_u64_u32 v[8:9], s[14:15], v4, 24, v[11:12]
	v_and_b32_e32 v5, v5, v7
	v_mov_b32_e32 v4, v9
	v_mad_u64_u32 v[4:5], s[14:15], v5, 24, v[4:5]
	v_mov_b32_e32 v9, v4
	global_load_dwordx2 v[4:5], v[8:9], off glc
	s_waitcnt vmcnt(0)
	global_atomic_cmpswap_x2 v[9:10], v3, v[4:7], s[6:7] offset:24 glc
	s_waitcnt vmcnt(0)
	buffer_wbinvl1_vol
	v_cmp_eq_u64_e32 vcc, v[9:10], v[6:7]
	s_or_b64 s[12:13], vcc, s[12:13]
	s_andn2_b64 exec, exec, s[12:13]
	s_cbranch_execnz .LBB2_114
; %bb.115:
	s_or_b64 exec, exec, s[12:13]
.LBB2_116:
	s_or_b64 exec, exec, s[10:11]
.LBB2_117:
	s_or_b64 exec, exec, s[8:9]
	s_waitcnt vmcnt(0)
	v_mov_b32_e32 v3, 0
	global_load_dwordx2 v[11:12], v3, s[6:7] offset:40
	global_load_dwordx4 v[5:8], v3, s[6:7]
	v_readfirstlane_b32 s8, v9
	v_readfirstlane_b32 s9, v10
	s_mov_b64 s[10:11], exec
	s_waitcnt vmcnt(1)
	v_readfirstlane_b32 s12, v11
	v_readfirstlane_b32 s13, v12
	s_and_b64 s[12:13], s[8:9], s[12:13]
	s_mul_i32 s14, s13, 24
	s_mul_hi_u32 s15, s12, 24
	s_mul_i32 s16, s12, 24
	s_add_i32 s14, s15, s14
	v_mov_b32_e32 v4, s14
	s_waitcnt vmcnt(0)
	v_add_co_u32_e32 v9, vcc, s16, v5
	v_addc_co_u32_e32 v10, vcc, v6, v4, vcc
	s_and_saveexec_b64 s[14:15], s[4:5]
	s_cbranch_execz .LBB2_119
; %bb.118:
	v_mov_b32_e32 v12, s11
	v_mov_b32_e32 v11, s10
	v_mov_b32_e32 v13, 2
	v_mov_b32_e32 v14, 1
	global_store_dwordx4 v[9:10], v[11:14], off offset:8
.LBB2_119:
	s_or_b64 exec, exec, s[14:15]
	s_lshl_b64 s[10:11], s[12:13], 12
	v_mov_b32_e32 v4, s11
	v_add_co_u32_e32 v11, vcc, s10, v7
	v_addc_co_u32_e32 v12, vcc, v8, v4, vcc
	s_movk_i32 s10, 0xff1f
	v_and_or_b32 v1, v1, s10, 32
	s_mov_b32 s12, 0
	v_mov_b32_e32 v4, v3
	v_readfirstlane_b32 s10, v11
	v_readfirstlane_b32 s11, v12
	v_add_co_u32_e32 v7, vcc, v11, v30
	s_mov_b32 s13, s12
	s_mov_b32 s14, s12
	s_mov_b32 s15, s12
	s_nop 0
	global_store_dwordx4 v30, v[1:4], s[10:11]
	v_addc_co_u32_e32 v8, vcc, 0, v12, vcc
	v_mov_b32_e32 v1, s12
	v_mov_b32_e32 v2, s13
	v_mov_b32_e32 v3, s14
	v_mov_b32_e32 v4, s15
	global_store_dwordx4 v30, v[1:4], s[10:11] offset:16
	global_store_dwordx4 v30, v[1:4], s[10:11] offset:32
	global_store_dwordx4 v30, v[1:4], s[10:11] offset:48
	s_and_saveexec_b64 s[10:11], s[4:5]
	s_cbranch_execz .LBB2_127
; %bb.120:
	v_mov_b32_e32 v11, 0
	global_load_dwordx2 v[14:15], v11, s[6:7] offset:32 glc
	global_load_dwordx2 v[1:2], v11, s[6:7] offset:40
	v_mov_b32_e32 v12, s8
	v_mov_b32_e32 v13, s9
	s_waitcnt vmcnt(0)
	v_readfirstlane_b32 s12, v1
	v_readfirstlane_b32 s13, v2
	s_and_b64 s[12:13], s[12:13], s[8:9]
	s_mul_i32 s13, s13, 24
	s_mul_hi_u32 s14, s12, 24
	s_mul_i32 s12, s12, 24
	s_add_i32 s13, s14, s13
	v_mov_b32_e32 v1, s13
	v_add_co_u32_e32 v5, vcc, s12, v5
	v_addc_co_u32_e32 v6, vcc, v6, v1, vcc
	global_store_dwordx2 v[5:6], v[14:15], off
	s_waitcnt vmcnt(0)
	global_atomic_cmpswap_x2 v[3:4], v11, v[12:15], s[6:7] offset:32 glc
	s_waitcnt vmcnt(0)
	v_cmp_ne_u64_e32 vcc, v[3:4], v[14:15]
	s_and_saveexec_b64 s[12:13], vcc
	s_cbranch_execz .LBB2_123
; %bb.121:
	s_mov_b64 s[14:15], 0
.LBB2_122:                              ; =>This Inner Loop Header: Depth=1
	s_sleep 1
	global_store_dwordx2 v[5:6], v[3:4], off
	v_mov_b32_e32 v1, s8
	v_mov_b32_e32 v2, s9
	s_waitcnt vmcnt(0)
	global_atomic_cmpswap_x2 v[1:2], v11, v[1:4], s[6:7] offset:32 glc
	s_waitcnt vmcnt(0)
	v_cmp_eq_u64_e32 vcc, v[1:2], v[3:4]
	v_mov_b32_e32 v4, v2
	s_or_b64 s[14:15], vcc, s[14:15]
	v_mov_b32_e32 v3, v1
	s_andn2_b64 exec, exec, s[14:15]
	s_cbranch_execnz .LBB2_122
.LBB2_123:
	s_or_b64 exec, exec, s[12:13]
	v_mov_b32_e32 v4, 0
	global_load_dwordx2 v[1:2], v4, s[6:7] offset:16
	s_mov_b64 s[12:13], exec
	v_mbcnt_lo_u32_b32 v3, s12, 0
	v_mbcnt_hi_u32_b32 v3, s13, v3
	v_cmp_eq_u32_e32 vcc, 0, v3
	s_and_saveexec_b64 s[14:15], vcc
	s_cbranch_execz .LBB2_125
; %bb.124:
	s_bcnt1_i32_b64 s12, s[12:13]
	v_mov_b32_e32 v3, s12
	s_waitcnt vmcnt(0)
	global_atomic_add_x2 v[1:2], v[3:4], off offset:8
.LBB2_125:
	s_or_b64 exec, exec, s[14:15]
	s_waitcnt vmcnt(0)
	global_load_dwordx2 v[3:4], v[1:2], off offset:16
	s_waitcnt vmcnt(0)
	v_cmp_eq_u64_e32 vcc, 0, v[3:4]
	s_cbranch_vccnz .LBB2_127
; %bb.126:
	global_load_dword v1, v[1:2], off offset:24
	v_mov_b32_e32 v2, 0
	s_waitcnt vmcnt(0)
	global_store_dwordx2 v[3:4], v[1:2], off
	v_and_b32_e32 v1, 0xffffff, v1
	v_readfirstlane_b32 s12, v1
	s_mov_b32 m0, s12
	s_nop 0
	s_sendmsg sendmsg(MSG_INTERRUPT)
.LBB2_127:
	s_or_b64 exec, exec, s[10:11]
	s_branch .LBB2_131
.LBB2_128:                              ;   in Loop: Header=BB2_131 Depth=1
	s_or_b64 exec, exec, s[10:11]
	v_readfirstlane_b32 s10, v1
	s_cmp_eq_u32 s10, 0
	s_cbranch_scc1 .LBB2_130
; %bb.129:                              ;   in Loop: Header=BB2_131 Depth=1
	s_sleep 1
	s_cbranch_execnz .LBB2_131
	s_branch .LBB2_133
.LBB2_130:
	s_branch .LBB2_133
.LBB2_131:                              ; =>This Inner Loop Header: Depth=1
	v_mov_b32_e32 v1, 1
	s_and_saveexec_b64 s[10:11], s[4:5]
	s_cbranch_execz .LBB2_128
; %bb.132:                              ;   in Loop: Header=BB2_131 Depth=1
	global_load_dword v1, v[9:10], off offset:20 glc
	s_waitcnt vmcnt(0)
	buffer_wbinvl1_vol
	v_and_b32_e32 v1, 1, v1
	s_branch .LBB2_128
.LBB2_133:
	global_load_dwordx2 v[3:4], v[7:8], off
	s_and_saveexec_b64 s[10:11], s[4:5]
	s_cbranch_execz .LBB2_136
; %bb.134:
	v_mov_b32_e32 v9, 0
	global_load_dwordx2 v[1:2], v9, s[6:7] offset:40
	global_load_dwordx2 v[10:11], v9, s[6:7] offset:24 glc
	global_load_dwordx2 v[12:13], v9, s[6:7]
	v_mov_b32_e32 v6, s9
	s_mov_b64 s[4:5], 0
	s_waitcnt vmcnt(2)
	v_add_co_u32_e32 v7, vcc, 1, v1
	v_addc_co_u32_e32 v8, vcc, 0, v2, vcc
	v_add_co_u32_e32 v5, vcc, s8, v7
	v_addc_co_u32_e32 v6, vcc, v8, v6, vcc
	v_cmp_eq_u64_e32 vcc, 0, v[5:6]
	v_cndmask_b32_e32 v6, v6, v8, vcc
	v_cndmask_b32_e32 v5, v5, v7, vcc
	v_and_b32_e32 v2, v6, v2
	v_and_b32_e32 v1, v5, v1
	v_mul_lo_u32 v2, v2, 24
	v_mul_hi_u32 v8, v1, 24
	v_mul_lo_u32 v1, v1, 24
	s_waitcnt vmcnt(1)
	v_mov_b32_e32 v7, v10
	v_add_u32_e32 v2, v8, v2
	s_waitcnt vmcnt(0)
	v_add_co_u32_e32 v1, vcc, v12, v1
	v_addc_co_u32_e32 v2, vcc, v13, v2, vcc
	global_store_dwordx2 v[1:2], v[10:11], off
	v_mov_b32_e32 v8, v11
	s_waitcnt vmcnt(0)
	global_atomic_cmpswap_x2 v[7:8], v9, v[5:8], s[6:7] offset:24 glc
	s_waitcnt vmcnt(0)
	v_cmp_ne_u64_e32 vcc, v[7:8], v[10:11]
	s_and_b64 exec, exec, vcc
	s_cbranch_execz .LBB2_136
.LBB2_135:                              ; =>This Inner Loop Header: Depth=1
	s_sleep 1
	global_store_dwordx2 v[1:2], v[7:8], off
	s_waitcnt vmcnt(0)
	global_atomic_cmpswap_x2 v[10:11], v9, v[5:8], s[6:7] offset:24 glc
	s_waitcnt vmcnt(0)
	v_cmp_eq_u64_e32 vcc, v[10:11], v[7:8]
	v_mov_b32_e32 v7, v10
	s_or_b64 s[4:5], vcc, s[4:5]
	v_mov_b32_e32 v8, v11
	s_andn2_b64 exec, exec, s[4:5]
	s_cbranch_execnz .LBB2_135
.LBB2_136:
	s_or_b64 exec, exec, s[10:11]
.LBB2_137:
	v_readfirstlane_b32 s4, v31
	s_waitcnt vmcnt(0)
	v_mov_b32_e32 v1, 0
	v_mov_b32_e32 v2, 0
	v_cmp_eq_u32_e64 s[4:5], s4, v31
	s_and_saveexec_b64 s[8:9], s[4:5]
	s_cbranch_execz .LBB2_143
; %bb.138:
	v_mov_b32_e32 v5, 0
	global_load_dwordx2 v[8:9], v5, s[6:7] offset:24 glc
	s_waitcnt vmcnt(0)
	buffer_wbinvl1_vol
	global_load_dwordx2 v[1:2], v5, s[6:7] offset:40
	global_load_dwordx2 v[6:7], v5, s[6:7]
	s_waitcnt vmcnt(1)
	v_and_b32_e32 v1, v1, v8
	v_and_b32_e32 v2, v2, v9
	v_mul_lo_u32 v2, v2, 24
	v_mul_hi_u32 v10, v1, 24
	v_mul_lo_u32 v1, v1, 24
	v_add_u32_e32 v2, v10, v2
	s_waitcnt vmcnt(0)
	v_add_co_u32_e32 v1, vcc, v6, v1
	v_addc_co_u32_e32 v2, vcc, v7, v2, vcc
	global_load_dwordx2 v[6:7], v[1:2], off glc
	s_waitcnt vmcnt(0)
	global_atomic_cmpswap_x2 v[1:2], v5, v[6:9], s[6:7] offset:24 glc
	s_waitcnt vmcnt(0)
	buffer_wbinvl1_vol
	v_cmp_ne_u64_e32 vcc, v[1:2], v[8:9]
	s_and_saveexec_b64 s[10:11], vcc
	s_cbranch_execz .LBB2_142
; %bb.139:
	s_mov_b64 s[12:13], 0
.LBB2_140:                              ; =>This Inner Loop Header: Depth=1
	s_sleep 1
	global_load_dwordx2 v[6:7], v5, s[6:7] offset:40
	global_load_dwordx2 v[10:11], v5, s[6:7]
	v_mov_b32_e32 v9, v2
	v_mov_b32_e32 v8, v1
	s_waitcnt vmcnt(1)
	v_and_b32_e32 v1, v6, v8
	s_waitcnt vmcnt(0)
	v_mad_u64_u32 v[1:2], s[14:15], v1, 24, v[10:11]
	v_and_b32_e32 v6, v7, v9
	v_mad_u64_u32 v[6:7], s[14:15], v6, 24, v[2:3]
	v_mov_b32_e32 v2, v6
	global_load_dwordx2 v[6:7], v[1:2], off glc
	s_waitcnt vmcnt(0)
	global_atomic_cmpswap_x2 v[1:2], v5, v[6:9], s[6:7] offset:24 glc
	s_waitcnt vmcnt(0)
	buffer_wbinvl1_vol
	v_cmp_eq_u64_e32 vcc, v[1:2], v[8:9]
	s_or_b64 s[12:13], vcc, s[12:13]
	s_andn2_b64 exec, exec, s[12:13]
	s_cbranch_execnz .LBB2_140
; %bb.141:
	s_or_b64 exec, exec, s[12:13]
.LBB2_142:
	s_or_b64 exec, exec, s[10:11]
.LBB2_143:
	s_or_b64 exec, exec, s[8:9]
	v_mov_b32_e32 v11, 0
	global_load_dwordx2 v[5:6], v11, s[6:7] offset:40
	global_load_dwordx4 v[7:10], v11, s[6:7]
	v_readfirstlane_b32 s8, v1
	v_readfirstlane_b32 s9, v2
	s_mov_b64 s[10:11], exec
	s_waitcnt vmcnt(1)
	v_readfirstlane_b32 s12, v5
	v_readfirstlane_b32 s13, v6
	s_and_b64 s[12:13], s[8:9], s[12:13]
	s_mul_i32 s14, s13, 24
	s_mul_hi_u32 s15, s12, 24
	s_mul_i32 s16, s12, 24
	s_add_i32 s14, s15, s14
	v_mov_b32_e32 v1, s14
	s_waitcnt vmcnt(0)
	v_add_co_u32_e32 v11, vcc, s16, v7
	v_addc_co_u32_e32 v12, vcc, v8, v1, vcc
	s_and_saveexec_b64 s[14:15], s[4:5]
	s_cbranch_execz .LBB2_145
; %bb.144:
	v_mov_b32_e32 v14, s11
	v_mov_b32_e32 v13, s10
	v_mov_b32_e32 v15, 2
	v_mov_b32_e32 v16, 1
	global_store_dwordx4 v[11:12], v[13:16], off offset:8
.LBB2_145:
	s_or_b64 exec, exec, s[14:15]
	s_lshl_b64 s[10:11], s[12:13], 12
	v_cvt_f64_f32_e32 v[5:6], v0
	v_mov_b32_e32 v1, s11
	v_add_co_u32_e32 v2, vcc, s10, v9
	v_addc_co_u32_e32 v1, vcc, v10, v1, vcc
	s_movk_i32 s10, 0xff1d
	v_and_or_b32 v3, v3, s10, 34
	v_readfirstlane_b32 s10, v2
	v_readfirstlane_b32 s11, v1
	s_mov_b32 s12, 0
	s_mov_b32 s13, s12
	s_mov_b32 s14, s12
	s_mov_b32 s15, s12
	s_nop 0
	global_store_dwordx4 v30, v[3:6], s[10:11]
	v_mov_b32_e32 v0, s12
	v_mov_b32_e32 v1, s13
	v_mov_b32_e32 v2, s14
	v_mov_b32_e32 v3, s15
	global_store_dwordx4 v30, v[0:3], s[10:11] offset:16
	global_store_dwordx4 v30, v[0:3], s[10:11] offset:32
	global_store_dwordx4 v30, v[0:3], s[10:11] offset:48
	s_and_saveexec_b64 s[10:11], s[4:5]
	s_cbranch_execz .LBB2_153
; %bb.146:
	v_mov_b32_e32 v6, 0
	global_load_dwordx2 v[15:16], v6, s[6:7] offset:32 glc
	global_load_dwordx2 v[0:1], v6, s[6:7] offset:40
	v_mov_b32_e32 v13, s8
	v_mov_b32_e32 v14, s9
	s_waitcnt vmcnt(0)
	v_readfirstlane_b32 s12, v0
	v_readfirstlane_b32 s13, v1
	s_and_b64 s[12:13], s[12:13], s[8:9]
	s_mul_i32 s13, s13, 24
	s_mul_hi_u32 s14, s12, 24
	s_mul_i32 s12, s12, 24
	s_add_i32 s13, s14, s13
	v_mov_b32_e32 v0, s13
	v_add_co_u32_e32 v4, vcc, s12, v7
	v_addc_co_u32_e32 v5, vcc, v8, v0, vcc
	global_store_dwordx2 v[4:5], v[15:16], off
	s_waitcnt vmcnt(0)
	global_atomic_cmpswap_x2 v[2:3], v6, v[13:16], s[6:7] offset:32 glc
	s_waitcnt vmcnt(0)
	v_cmp_ne_u64_e32 vcc, v[2:3], v[15:16]
	s_and_saveexec_b64 s[12:13], vcc
	s_cbranch_execz .LBB2_149
; %bb.147:
	s_mov_b64 s[14:15], 0
.LBB2_148:                              ; =>This Inner Loop Header: Depth=1
	s_sleep 1
	global_store_dwordx2 v[4:5], v[2:3], off
	v_mov_b32_e32 v0, s8
	v_mov_b32_e32 v1, s9
	s_waitcnt vmcnt(0)
	global_atomic_cmpswap_x2 v[0:1], v6, v[0:3], s[6:7] offset:32 glc
	s_waitcnt vmcnt(0)
	v_cmp_eq_u64_e32 vcc, v[0:1], v[2:3]
	v_mov_b32_e32 v3, v1
	s_or_b64 s[14:15], vcc, s[14:15]
	v_mov_b32_e32 v2, v0
	s_andn2_b64 exec, exec, s[14:15]
	s_cbranch_execnz .LBB2_148
.LBB2_149:
	s_or_b64 exec, exec, s[12:13]
	v_mov_b32_e32 v3, 0
	global_load_dwordx2 v[0:1], v3, s[6:7] offset:16
	s_mov_b64 s[12:13], exec
	v_mbcnt_lo_u32_b32 v2, s12, 0
	v_mbcnt_hi_u32_b32 v2, s13, v2
	v_cmp_eq_u32_e32 vcc, 0, v2
	s_and_saveexec_b64 s[14:15], vcc
	s_cbranch_execz .LBB2_151
; %bb.150:
	s_bcnt1_i32_b64 s12, s[12:13]
	v_mov_b32_e32 v2, s12
	s_waitcnt vmcnt(0)
	global_atomic_add_x2 v[0:1], v[2:3], off offset:8
.LBB2_151:
	s_or_b64 exec, exec, s[14:15]
	s_waitcnt vmcnt(0)
	global_load_dwordx2 v[2:3], v[0:1], off offset:16
	s_waitcnt vmcnt(0)
	v_cmp_eq_u64_e32 vcc, 0, v[2:3]
	s_cbranch_vccnz .LBB2_153
; %bb.152:
	global_load_dword v0, v[0:1], off offset:24
	v_mov_b32_e32 v1, 0
	s_waitcnt vmcnt(0)
	global_store_dwordx2 v[2:3], v[0:1], off
	v_and_b32_e32 v0, 0xffffff, v0
	v_readfirstlane_b32 s12, v0
	s_mov_b32 m0, s12
	s_nop 0
	s_sendmsg sendmsg(MSG_INTERRUPT)
.LBB2_153:
	s_or_b64 exec, exec, s[10:11]
	s_branch .LBB2_157
.LBB2_154:                              ;   in Loop: Header=BB2_157 Depth=1
	s_or_b64 exec, exec, s[10:11]
	v_readfirstlane_b32 s10, v0
	s_cmp_eq_u32 s10, 0
	s_cbranch_scc1 .LBB2_156
; %bb.155:                              ;   in Loop: Header=BB2_157 Depth=1
	s_sleep 1
	s_cbranch_execnz .LBB2_157
	s_branch .LBB2_159
.LBB2_156:
	s_branch .LBB2_159
.LBB2_157:                              ; =>This Inner Loop Header: Depth=1
	v_mov_b32_e32 v0, 1
	s_and_saveexec_b64 s[10:11], s[4:5]
	s_cbranch_execz .LBB2_154
; %bb.158:                              ;   in Loop: Header=BB2_157 Depth=1
	global_load_dword v0, v[11:12], off offset:20 glc
	s_waitcnt vmcnt(0)
	buffer_wbinvl1_vol
	v_and_b32_e32 v0, 1, v0
	s_branch .LBB2_154
.LBB2_159:
	s_and_saveexec_b64 s[10:11], s[4:5]
	s_cbranch_execz .LBB2_162
; %bb.160:
	v_mov_b32_e32 v6, 0
	global_load_dwordx2 v[2:3], v6, s[6:7] offset:40
	global_load_dwordx2 v[7:8], v6, s[6:7] offset:24 glc
	global_load_dwordx2 v[4:5], v6, s[6:7]
	v_mov_b32_e32 v1, s9
	s_mov_b64 s[4:5], 0
	s_waitcnt vmcnt(2)
	v_add_co_u32_e32 v9, vcc, 1, v2
	v_addc_co_u32_e32 v10, vcc, 0, v3, vcc
	v_add_co_u32_e32 v0, vcc, s8, v9
	v_addc_co_u32_e32 v1, vcc, v10, v1, vcc
	v_cmp_eq_u64_e32 vcc, 0, v[0:1]
	v_cndmask_b32_e32 v1, v1, v10, vcc
	v_cndmask_b32_e32 v0, v0, v9, vcc
	v_and_b32_e32 v3, v1, v3
	v_and_b32_e32 v2, v0, v2
	v_mul_lo_u32 v3, v3, 24
	v_mul_hi_u32 v9, v2, 24
	v_mul_lo_u32 v10, v2, 24
	s_waitcnt vmcnt(1)
	v_mov_b32_e32 v2, v7
	v_add_u32_e32 v3, v9, v3
	s_waitcnt vmcnt(0)
	v_add_co_u32_e32 v4, vcc, v4, v10
	v_addc_co_u32_e32 v5, vcc, v5, v3, vcc
	global_store_dwordx2 v[4:5], v[7:8], off
	v_mov_b32_e32 v3, v8
	s_waitcnt vmcnt(0)
	global_atomic_cmpswap_x2 v[2:3], v6, v[0:3], s[6:7] offset:24 glc
	s_waitcnt vmcnt(0)
	v_cmp_ne_u64_e32 vcc, v[2:3], v[7:8]
	s_and_b64 exec, exec, vcc
	s_cbranch_execz .LBB2_162
.LBB2_161:                              ; =>This Inner Loop Header: Depth=1
	s_sleep 1
	global_store_dwordx2 v[4:5], v[2:3], off
	s_waitcnt vmcnt(0)
	global_atomic_cmpswap_x2 v[7:8], v6, v[0:3], s[6:7] offset:24 glc
	s_waitcnt vmcnt(0)
	v_cmp_eq_u64_e32 vcc, v[7:8], v[2:3]
	v_mov_b32_e32 v2, v7
	s_or_b64 s[4:5], vcc, s[4:5]
	v_mov_b32_e32 v3, v8
	s_andn2_b64 exec, exec, s[4:5]
	s_cbranch_execnz .LBB2_161
.LBB2_162:
	s_or_b64 exec, exec, s[10:11]
	s_waitcnt vmcnt(0) lgkmcnt(0)
	s_setpc_b64 s[30:31]
.Lfunc_end2:
	.size	_Z1Cf, .Lfunc_end2-_Z1Cf
                                        ; -- End function
	.set .L_Z1Cf.num_vgpr, 34
	.set .L_Z1Cf.num_agpr, 0
	.set .L_Z1Cf.numbered_sgpr, 32
	.set .L_Z1Cf.private_seg_size, 0
	.set .L_Z1Cf.uses_vcc, 1
	.set .L_Z1Cf.uses_flat_scratch, 0
	.set .L_Z1Cf.has_dyn_sized_stack, 0
	.set .L_Z1Cf.has_recursion, 0
	.set .L_Z1Cf.has_indirect_call, 0
	.section	.AMDGPU.csdata,"",@progbits
; Function info:
; codeLenInByte = 5816
; TotalNumSgprs: 36
; NumVgprs: 34
; NumAgprs: 0
; TotalNumVgprs: 34
; ScratchSize: 0
; MemoryBound: 0
	.text
	.protected	_Z11load_fn_ptrPf       ; -- Begin function _Z11load_fn_ptrPf
	.globl	_Z11load_fn_ptrPf
	.p2align	8
	.type	_Z11load_fn_ptrPf,@function
_Z11load_fn_ptrPf:                      ; @_Z11load_fn_ptrPf
; %bb.0:
	s_load_dwordx2 s[0:1], s[4:5], 0x0
	v_mov_b32_e32 v0, 0
	v_mov_b32_e32 v1, 1.0
	v_mov_b32_e32 v2, 2.0
	s_getpc_b64 s[2:3]
	s_add_u32 s2, s2, _Z1Cf@rel32@lo+4
	s_addc_u32 s3, s3, _Z1Cf@rel32@hi+12
	s_waitcnt lgkmcnt(0)
	global_store_dwordx3 v0, v[0:2], s[0:1]
	s_getpc_b64 s[0:1]
	s_add_u32 s0, s0, fn_array@rel32@lo+20
	s_addc_u32 s1, s1, fn_array@rel32@hi+28
	v_mov_b32_e32 v1, s2
	v_mov_b32_e32 v2, s3
	global_store_dwordx2 v0, v[1:2], s[0:1]
	s_endpgm
	.section	.rodata,"a",@progbits
	.p2align	6, 0x0
	.amdhsa_kernel _Z11load_fn_ptrPf
		.amdhsa_group_segment_fixed_size 0
		.amdhsa_private_segment_fixed_size 0
		.amdhsa_kernarg_size 8
		.amdhsa_user_sgpr_count 6
		.amdhsa_user_sgpr_private_segment_buffer 1
		.amdhsa_user_sgpr_dispatch_ptr 0
		.amdhsa_user_sgpr_queue_ptr 0
		.amdhsa_user_sgpr_kernarg_segment_ptr 1
		.amdhsa_user_sgpr_dispatch_id 0
		.amdhsa_user_sgpr_flat_scratch_init 0
		.amdhsa_user_sgpr_private_segment_size 0
		.amdhsa_uses_dynamic_stack 0
		.amdhsa_system_sgpr_private_segment_wavefront_offset 0
		.amdhsa_system_sgpr_workgroup_id_x 1
		.amdhsa_system_sgpr_workgroup_id_y 0
		.amdhsa_system_sgpr_workgroup_id_z 0
		.amdhsa_system_sgpr_workgroup_info 0
		.amdhsa_system_vgpr_workitem_id 0
		.amdhsa_next_free_vgpr 3
		.amdhsa_next_free_sgpr 6
		.amdhsa_reserve_vcc 0
		.amdhsa_reserve_flat_scratch 0
		.amdhsa_float_round_mode_32 0
		.amdhsa_float_round_mode_16_64 0
		.amdhsa_float_denorm_mode_32 3
		.amdhsa_float_denorm_mode_16_64 3
		.amdhsa_dx10_clamp 1
		.amdhsa_ieee_mode 1
		.amdhsa_fp16_overflow 0
		.amdhsa_exception_fp_ieee_invalid_op 0
		.amdhsa_exception_fp_denorm_src 0
		.amdhsa_exception_fp_ieee_div_zero 0
		.amdhsa_exception_fp_ieee_overflow 0
		.amdhsa_exception_fp_ieee_underflow 0
		.amdhsa_exception_fp_ieee_inexact 0
		.amdhsa_exception_int_div_zero 0
	.end_amdhsa_kernel
	.text
.Lfunc_end3:
	.size	_Z11load_fn_ptrPf, .Lfunc_end3-_Z11load_fn_ptrPf
                                        ; -- End function
	.set _Z11load_fn_ptrPf.num_vgpr, 3
	.set _Z11load_fn_ptrPf.num_agpr, 0
	.set _Z11load_fn_ptrPf.numbered_sgpr, 6
	.set _Z11load_fn_ptrPf.private_seg_size, 0
	.set _Z11load_fn_ptrPf.uses_vcc, 0
	.set _Z11load_fn_ptrPf.uses_flat_scratch, 0
	.set _Z11load_fn_ptrPf.has_dyn_sized_stack, 0
	.set _Z11load_fn_ptrPf.has_recursion, 0
	.set _Z11load_fn_ptrPf.has_indirect_call, 0
	.section	.AMDGPU.csdata,"",@progbits
; Kernel info:
; codeLenInByte = 92
; TotalNumSgprs: 10
; NumVgprs: 3
; NumAgprs: 0
; TotalNumVgprs: 3
; ScratchSize: 0
; MemoryBound: 0
; FloatMode: 240
; IeeeMode: 1
; LDSByteSize: 0 bytes/workgroup (compile time only)
; SGPRBlocks: 1
; VGPRBlocks: 0
; NumSGPRsForWavesPerEU: 10
; NumVGPRsForWavesPerEU: 3
; Occupancy: 10
; WaveLimiterHint : 0
; COMPUTE_PGM_RSRC2:SCRATCH_EN: 0
; COMPUTE_PGM_RSRC2:USER_SGPR: 6
; COMPUTE_PGM_RSRC2:TRAP_HANDLER: 0
; COMPUTE_PGM_RSRC2:TGID_X_EN: 1
; COMPUTE_PGM_RSRC2:TGID_Y_EN: 0
; COMPUTE_PGM_RSRC2:TGID_Z_EN: 0
; COMPUTE_PGM_RSRC2:TIDIG_COMP_CNT: 0
	.text
	.protected	_Z7call_fnPKfi          ; -- Begin function _Z7call_fnPKfi
	.globl	_Z7call_fnPKfi
	.p2align	8
	.type	_Z7call_fnPKfi,@function
_Z7call_fnPKfi:                         ; @_Z7call_fnPKfi
; %bb.0:
	s_add_u32 flat_scratch_lo, s12, s17
	s_mov_b32 s12, s14
	s_load_dword s14, s[8:9], 0x8
	s_load_dwordx2 s[18:19], s[8:9], 0x0
	s_addc_u32 flat_scratch_hi, s13, 0
	s_add_u32 s0, s0, s17
	s_addc_u32 s1, s1, 0
	s_mov_b32 s13, s15
	s_waitcnt lgkmcnt(0)
	s_ashr_i32 s15, s14, 31
	s_lshl_b64 s[20:21], s[14:15], 3
	s_getpc_b64 s[22:23]
	s_add_u32 s22, s22, fn_array@rel32@lo+4
	s_addc_u32 s23, s23, fn_array@rel32@hi+12
	s_add_u32 s20, s20, s22
	s_addc_u32 s21, s21, s23
	s_lshl_b64 s[14:15], s[14:15], 2
	s_add_u32 s14, s18, s14
	s_addc_u32 s15, s19, s15
	s_load_dword s15, s[14:15], 0x0
	s_add_u32 s8, s8, 16
	s_load_dwordx2 s[20:21], s[20:21], 0x0
	v_lshlrev_b32_e32 v2, 20, v2
	v_lshlrev_b32_e32 v1, 10, v1
	s_addc_u32 s9, s9, 0
	v_or3_b32 v31, v0, v1, v2
	s_mov_b32 s14, s16
	s_waitcnt lgkmcnt(0)
	v_mov_b32_e32 v0, s15
	s_mov_b32 s32, 0
	s_swappc_b64 s[30:31], s[20:21]
	s_endpgm
	.section	.rodata,"a",@progbits
	.p2align	6, 0x0
	.amdhsa_kernel _Z7call_fnPKfi
		.amdhsa_group_segment_fixed_size 0
		.amdhsa_private_segment_fixed_size 0
		.amdhsa_kernarg_size 272
		.amdhsa_user_sgpr_count 14
		.amdhsa_user_sgpr_private_segment_buffer 1
		.amdhsa_user_sgpr_dispatch_ptr 1
		.amdhsa_user_sgpr_queue_ptr 1
		.amdhsa_user_sgpr_kernarg_segment_ptr 1
		.amdhsa_user_sgpr_dispatch_id 1
		.amdhsa_user_sgpr_flat_scratch_init 1
		.amdhsa_user_sgpr_private_segment_size 0
		.amdhsa_uses_dynamic_stack 1
		.amdhsa_system_sgpr_private_segment_wavefront_offset 1
		.amdhsa_system_sgpr_workgroup_id_x 1
		.amdhsa_system_sgpr_workgroup_id_y 1
		.amdhsa_system_sgpr_workgroup_id_z 1
		.amdhsa_system_sgpr_workgroup_info 0
		.amdhsa_system_vgpr_workitem_id 2
		.amdhsa_next_free_vgpr max(totalnumvgprs(_Z7call_fnPKfi.num_agpr, _Z7call_fnPKfi.num_vgpr), 1, 0)
		.amdhsa_next_free_sgpr (max(_Z7call_fnPKfi.numbered_sgpr+6, 1, 0))-6
		.amdhsa_reserve_vcc 1
		.amdhsa_reserve_flat_scratch 1
		.amdhsa_float_round_mode_32 0
		.amdhsa_float_round_mode_16_64 0
		.amdhsa_float_denorm_mode_32 3
		.amdhsa_float_denorm_mode_16_64 3
		.amdhsa_dx10_clamp 1
		.amdhsa_ieee_mode 1
		.amdhsa_fp16_overflow 0
		.amdhsa_exception_fp_ieee_invalid_op 0
		.amdhsa_exception_fp_denorm_src 0
		.amdhsa_exception_fp_ieee_div_zero 0
		.amdhsa_exception_fp_ieee_overflow 0
		.amdhsa_exception_fp_ieee_underflow 0
		.amdhsa_exception_fp_ieee_inexact 0
		.amdhsa_exception_int_div_zero 0
	.end_amdhsa_kernel
	.text
.Lfunc_end4:
	.size	_Z7call_fnPKfi, .Lfunc_end4-_Z7call_fnPKfi
                                        ; -- End function
	.set _Z7call_fnPKfi.num_vgpr, max(32, amdgpu.max_num_vgpr)
	.set _Z7call_fnPKfi.num_agpr, max(0, amdgpu.max_num_agpr)
	.set _Z7call_fnPKfi.numbered_sgpr, max(33, amdgpu.max_num_sgpr)
	.set _Z7call_fnPKfi.private_seg_size, 0
	.set _Z7call_fnPKfi.uses_vcc, 1
	.set _Z7call_fnPKfi.uses_flat_scratch, 1
	.set _Z7call_fnPKfi.has_dyn_sized_stack, 1
	.set _Z7call_fnPKfi.has_recursion, 1
	.set _Z7call_fnPKfi.has_indirect_call, 1
	.section	.AMDGPU.csdata,"",@progbits
; Kernel info:
; codeLenInByte = 156
; TotalNumSgprs: _Z7call_fnPKfi.numbered_sgpr+6
; NumVgprs: _Z7call_fnPKfi.num_vgpr
; NumAgprs: _Z7call_fnPKfi.num_agpr
; TotalNumVgprs: totalnumvgprs(_Z7call_fnPKfi.num_agpr, _Z7call_fnPKfi.num_vgpr)
; ScratchSize: 0
; MemoryBound: 0
; FloatMode: 240
; IeeeMode: 1
; LDSByteSize: 0 bytes/workgroup (compile time only)
; SGPRBlocks: ((alignto(max(max(_Z7call_fnPKfi.numbered_sgpr+(extrasgprs(_Z7call_fnPKfi.uses_vcc, _Z7call_fnPKfi.uses_flat_scratch, 1)), 1, 0), 1), 8))/8)-1
; VGPRBlocks: ((alignto(max(max(totalnumvgprs(_Z7call_fnPKfi.num_agpr, _Z7call_fnPKfi.num_vgpr), 1, 0), 1), 4))/4)-1
; NumSGPRsForWavesPerEU: max(_Z7call_fnPKfi.numbered_sgpr+6, 1, 0)
; NumVGPRsForWavesPerEU: max(totalnumvgprs(_Z7call_fnPKfi.num_agpr, _Z7call_fnPKfi.num_vgpr), 1, 0)
; Occupancy: occupancy(10, 4, 256, 8, 10, max(_Z7call_fnPKfi.numbered_sgpr+(extrasgprs(_Z7call_fnPKfi.uses_vcc, _Z7call_fnPKfi.uses_flat_scratch, 1)), 1, 0), max(totalnumvgprs(_Z7call_fnPKfi.num_agpr, _Z7call_fnPKfi.num_vgpr), 1, 0))
; WaveLimiterHint : 0
; COMPUTE_PGM_RSRC2:SCRATCH_EN: 1
; COMPUTE_PGM_RSRC2:USER_SGPR: 14
; COMPUTE_PGM_RSRC2:TRAP_HANDLER: 0
; COMPUTE_PGM_RSRC2:TGID_X_EN: 1
; COMPUTE_PGM_RSRC2:TGID_Y_EN: 1
; COMPUTE_PGM_RSRC2:TGID_Z_EN: 1
; COMPUTE_PGM_RSRC2:TIDIG_COMP_CNT: 2
	.section	.AMDGPU.gpr_maximums,"",@progbits
	.set amdgpu.max_num_vgpr, 34
	.set amdgpu.max_num_agpr, 0
	.set amdgpu.max_num_sgpr, 32
	.section	.AMDGPU.csdata,"",@progbits
	.type	.str,@object                    ; @.str
	.section	.rodata.str1.1,"aMS",@progbits,1
.str:
	.asciz	"A called: %f\n"
	.size	.str, 14

	.type	.str.1,@object                  ; @.str.1
.str.1:
	.asciz	"B called: %f\n"
	.size	.str.1, 14

	.type	.str.2,@object                  ; @.str.2
.str.2:
	.asciz	"C called: %f\n"
	.size	.str.2, 14

	.protected	fn_array                ; @fn_array
	.type	fn_array,@object
	.data
	.globl	fn_array
	.p2align	4, 0x0
fn_array:
	.quad	_Z1Af
	.quad	_Z1Bf
	.quad	_Z1Bf
	.size	fn_array, 24

	.type	__hip_cuid_79ab1a9f7bf93313,@object ; @__hip_cuid_79ab1a9f7bf93313
	.section	.bss,"aw",@nobits
	.globl	__hip_cuid_79ab1a9f7bf93313
__hip_cuid_79ab1a9f7bf93313:
	.byte	0                               ; 0x0
	.size	__hip_cuid_79ab1a9f7bf93313, 1

	.ident	"AMD clang version 20.0.0git (https://github.com/RadeonOpenCompute/llvm-project roc-7.0.2 25385 0dda3adf56766e0aac0d03173ced3759e1ffecbc)"
	.section	".note.GNU-stack","",@progbits
	.addrsig
	.addrsig_sym _Z1Af
	.addrsig_sym _Z1Bf
	.addrsig_sym _Z1Cf
	.addrsig_sym fn_array
	.addrsig_sym __hip_cuid_79ab1a9f7bf93313
	.amdgpu_metadata
---
amdhsa.kernels:
  - .agpr_count:     0
    .args:
      - .address_space:  global
        .offset:         0
        .size:           8
        .value_kind:     global_buffer
    .group_segment_fixed_size: 0
    .kernarg_segment_align: 8
    .kernarg_segment_size: 8
    .language:       OpenCL C
    .language_version:
      - 2
      - 0
    .max_flat_workgroup_size: 1024
    .name:           _Z11load_fn_ptrPf
    .private_segment_fixed_size: 0
    .sgpr_count:     10
    .sgpr_spill_count: 0
    .symbol:         _Z11load_fn_ptrPf.kd
    .uniform_work_group_size: 1
    .uses_dynamic_stack: false
    .vgpr_count:     3
    .vgpr_spill_count: 0
    .wavefront_size: 64
  - .agpr_count:     0
    .args:
      - .address_space:  global
        .offset:         0
        .size:           8
        .value_kind:     global_buffer
      - .offset:         8
        .size:           4
        .value_kind:     by_value
      - .offset:         16
        .size:           4
        .value_kind:     hidden_block_count_x
      - .offset:         20
        .size:           4
        .value_kind:     hidden_block_count_y
      - .offset:         24
        .size:           4
        .value_kind:     hidden_block_count_z
      - .offset:         28
        .size:           2
        .value_kind:     hidden_group_size_x
      - .offset:         30
        .size:           2
        .value_kind:     hidden_group_size_y
      - .offset:         32
        .size:           2
        .value_kind:     hidden_group_size_z
      - .offset:         34
        .size:           2
        .value_kind:     hidden_remainder_x
      - .offset:         36
        .size:           2
        .value_kind:     hidden_remainder_y
      - .offset:         38
        .size:           2
        .value_kind:     hidden_remainder_z
      - .offset:         56
        .size:           8
        .value_kind:     hidden_global_offset_x
      - .offset:         64
        .size:           8
        .value_kind:     hidden_global_offset_y
      - .offset:         72
        .size:           8
        .value_kind:     hidden_global_offset_z
      - .offset:         80
        .size:           2
        .value_kind:     hidden_grid_dims
      - .offset:         96
        .size:           8
        .value_kind:     hidden_hostcall_buffer
      - .offset:         104
        .size:           8
        .value_kind:     hidden_multigrid_sync_arg
      - .offset:         112
        .size:           8
        .value_kind:     hidden_heap_v1
      - .offset:         120
        .size:           8
        .value_kind:     hidden_default_queue
      - .offset:         128
        .size:           8
        .value_kind:     hidden_completion_action
      - .offset:         216
        .size:           8
        .value_kind:     hidden_queue_ptr
    .group_segment_fixed_size: 0
    .kernarg_segment_align: 8
    .kernarg_segment_size: 272
    .language:       OpenCL C
    .language_version:
      - 2
      - 0
    .max_flat_workgroup_size: 1024
    .name:           _Z7call_fnPKfi
    .private_segment_fixed_size: 0
    .sgpr_count:     39
    .sgpr_spill_count: 0
    .symbol:         _Z7call_fnPKfi.kd
    .uniform_work_group_size: 1
    .uses_dynamic_stack: true
    .vgpr_count:     34
    .vgpr_spill_count: 0
    .wavefront_size: 64
amdhsa.target:   amdgcn-amd-amdhsa--gfx908
amdhsa.version:
  - 1
  - 2
...

	.end_amdgpu_metadata

# __CLANG_OFFLOAD_BUNDLE____END__ hip-amdgcn-amd-amdhsa--gfx908

# __CLANG_OFFLOAD_BUNDLE____START__ host-x86_64-unknown-linux-gnu-
	.file	"test.cpp"
	.text
	.globl	_Z26__device_stub__load_fn_ptrPf # -- Begin function _Z26__device_stub__load_fn_ptrPf
	.p2align	4
	.type	_Z26__device_stub__load_fn_ptrPf,@function
_Z26__device_stub__load_fn_ptrPf:       # @_Z26__device_stub__load_fn_ptrPf
	.cfi_startproc
# %bb.0:
	subq	$72, %rsp
	.cfi_def_cfa_offset 80
	movq	%rdi, 64(%rsp)
	leaq	64(%rsp), %rax
	movq	%rax, (%rsp)
	leaq	48(%rsp), %rdi
	leaq	32(%rsp), %rsi
	leaq	24(%rsp), %rdx
	leaq	16(%rsp), %rcx
	callq	__hipPopCallConfiguration
	movq	48(%rsp), %rsi
	movl	56(%rsp), %edx
	movq	32(%rsp), %rcx
	movl	40(%rsp), %r8d
	movq	%rsp, %r9
	movl	$_Z11load_fn_ptrPf, %edi
	pushq	16(%rsp)
	.cfi_adjust_cfa_offset 8
	pushq	32(%rsp)
	.cfi_adjust_cfa_offset 8
	callq	hipLaunchKernel
	addq	$88, %rsp
	.cfi_adjust_cfa_offset -88
	retq
.Lfunc_end0:
	.size	_Z26__device_stub__load_fn_ptrPf, .Lfunc_end0-_Z26__device_stub__load_fn_ptrPf
	.cfi_endproc
                                        # -- End function
	.globl	_Z22__device_stub__call_fnPKfi  # -- Begin function _Z22__device_stub__call_fnPKfi
	.p2align	4
	.type	_Z22__device_stub__call_fnPKfi,@function
_Z22__device_stub__call_fnPKfi:         # @_Z22__device_stub__call_fnPKfi
	.cfi_startproc
# %bb.0:
	subq	$88, %rsp
	.cfi_def_cfa_offset 96
	movq	%rdi, 56(%rsp)
	movl	%esi, 4(%rsp)
	leaq	56(%rsp), %rax
	movq	%rax, 64(%rsp)
	leaq	4(%rsp), %rax
	movq	%rax, 72(%rsp)
	leaq	40(%rsp), %rdi
	leaq	24(%rsp), %rsi
	leaq	16(%rsp), %rdx
	leaq	8(%rsp), %rcx
	callq	__hipPopCallConfiguration
	movq	40(%rsp), %rsi
	movl	48(%rsp), %edx
	movq	24(%rsp), %rcx
	movl	32(%rsp), %r8d
	leaq	64(%rsp), %r9
	movl	$_Z7call_fnPKfi, %edi
	pushq	8(%rsp)
	.cfi_adjust_cfa_offset 8
	pushq	24(%rsp)
	.cfi_adjust_cfa_offset 8
	callq	hipLaunchKernel
	addq	$104, %rsp
	.cfi_adjust_cfa_offset -104
	retq
.Lfunc_end1:
	.size	_Z22__device_stub__call_fnPKfi, .Lfunc_end1-_Z22__device_stub__call_fnPKfi
	.cfi_endproc
                                        # -- End function
	.globl	main                            # -- Begin function main
	.p2align	4
	.type	main,@function
main:                                   # @main
	.cfi_startproc
# %bb.0:
	pushq	%rbx
	.cfi_def_cfa_offset 16
	subq	$96, %rsp
	.cfi_def_cfa_offset 112
	.cfi_offset %rbx, -16
	movabsq	$4294967297, %rbx               # imm = 0x100000001
	movq	$0, 16(%rsp)
	leaq	16(%rsp), %rdi
	movl	$12, %esi
	callq	hipMalloc
	movq	%rbx, %rdi
	movl	$1, %esi
	movq	%rbx, %rdx
	movl	$1, %ecx
	xorl	%r8d, %r8d
	xorl	%r9d, %r9d
	callq	__hipPushCallConfiguration
	testl	%eax, %eax
	jne	.LBB2_2
# %bb.1:
	movq	16(%rsp), %rax
	movq	%rax, 64(%rsp)
	leaq	64(%rsp), %rax
	movq	%rax, (%rsp)
	leaq	80(%rsp), %rdi
	leaq	32(%rsp), %rsi
	leaq	48(%rsp), %rdx
	leaq	24(%rsp), %rcx
	callq	__hipPopCallConfiguration
	movq	80(%rsp), %rsi
	movl	88(%rsp), %edx
	movq	32(%rsp), %rcx
	movl	40(%rsp), %r8d
	movq	%rsp, %r9
	movl	$_Z11load_fn_ptrPf, %edi
	pushq	24(%rsp)
	.cfi_adjust_cfa_offset 8
	pushq	56(%rsp)
	.cfi_adjust_cfa_offset 8
	callq	hipLaunchKernel
	addq	$16, %rsp
	.cfi_adjust_cfa_offset -16
.LBB2_2:
	movq	%rbx, %rdi
	movl	$1, %esi
	movq	%rbx, %rdx
	movl	$1, %ecx
	xorl	%r8d, %r8d
	xorl	%r9d, %r9d
	callq	__hipPushCallConfiguration
	testl	%eax, %eax
	jne	.LBB2_4
# %bb.3:
	movq	16(%rsp), %rax
	movq	%rax, 48(%rsp)
	movl	$2, 60(%rsp)
	leaq	48(%rsp), %rax
	movq	%rax, 80(%rsp)
	leaq	60(%rsp), %rax
	movq	%rax, 88(%rsp)
	leaq	32(%rsp), %rdi
	leaq	64(%rsp), %rsi
	leaq	24(%rsp), %rdx
	movq	%rsp, %rcx
	callq	__hipPopCallConfiguration
	movq	32(%rsp), %rsi
	movl	40(%rsp), %edx
	movq	64(%rsp), %rcx
	movl	72(%rsp), %r8d
	leaq	80(%rsp), %r9
	movl	$_Z7call_fnPKfi, %edi
	pushq	(%rsp)
	.cfi_adjust_cfa_offset 8
	pushq	32(%rsp)
	.cfi_adjust_cfa_offset 8
	callq	hipLaunchKernel
	addq	$16, %rsp
	.cfi_adjust_cfa_offset -16
.LBB2_4:
	callq	hipDeviceSynchronize
	xorl	%eax, %eax
	addq	$96, %rsp
	.cfi_def_cfa_offset 16
	popq	%rbx
	.cfi_def_cfa_offset 8
	retq
.Lfunc_end2:
	.size	main, .Lfunc_end2-main
	.cfi_endproc
                                        # -- End function
	.p2align	4                               # -- Begin function __hip_module_ctor
	.type	__hip_module_ctor,@function
__hip_module_ctor:                      # @__hip_module_ctor
	.cfi_startproc
# %bb.0:
	pushq	%rbx
	.cfi_def_cfa_offset 16
	subq	$32, %rsp
	.cfi_def_cfa_offset 48
	.cfi_offset %rbx, -16
	movq	__hip_gpubin_handle_79ab1a9f7bf93313(%rip), %rbx
	testq	%rbx, %rbx
	jne	.LBB3_2
# %bb.1:
	movl	$__hip_fatbin_wrapper, %edi
	callq	__hipRegisterFatBinary
	movq	%rax, %rbx
	movq	%rax, __hip_gpubin_handle_79ab1a9f7bf93313(%rip)
.LBB3_2:
	xorps	%xmm0, %xmm0
	movups	%xmm0, 16(%rsp)
	movups	%xmm0, (%rsp)
	movl	$_Z11load_fn_ptrPf, %esi
	movl	$.L__unnamed_1, %edx
	movl	$.L__unnamed_1, %ecx
	movq	%rbx, %rdi
	movl	$-1, %r8d
	xorl	%r9d, %r9d
	callq	__hipRegisterFunction
	xorps	%xmm0, %xmm0
	movups	%xmm0, 16(%rsp)
	movups	%xmm0, (%rsp)
	movl	$_Z7call_fnPKfi, %esi
	movl	$.L__unnamed_2, %edx
	movl	$.L__unnamed_2, %ecx
	movq	%rbx, %rdi
	movl	$-1, %r8d
	xorl	%r9d, %r9d
	callq	__hipRegisterFunction
	movl	$0, 8(%rsp)
	movl	$0, (%rsp)
	movl	$fn_array, %esi
	movl	$.L__unnamed_3, %edx
	movl	$.L__unnamed_3, %ecx
	movl	$24, %r9d
	movq	%rbx, %rdi
	xorl	%r8d, %r8d
	callq	__hipRegisterVar
	movl	$__hip_module_dtor, %edi
	addq	$32, %rsp
	.cfi_def_cfa_offset 16
	popq	%rbx
	.cfi_def_cfa_offset 8
	jmp	atexit                          # TAILCALL
.Lfunc_end3:
	.size	__hip_module_ctor, .Lfunc_end3-__hip_module_ctor
	.cfi_endproc
                                        # -- End function
	.p2align	4                               # -- Begin function __hip_module_dtor
	.type	__hip_module_dtor,@function
__hip_module_dtor:                      # @__hip_module_dtor
	.cfi_startproc
# %bb.0:
	movq	__hip_gpubin_handle_79ab1a9f7bf93313(%rip), %rdi
	testq	%rdi, %rdi
	je	.LBB4_2
# %bb.1:
	pushq	%rax
	.cfi_def_cfa_offset 16
	callq	__hipUnregisterFatBinary
	movq	$0, __hip_gpubin_handle_79ab1a9f7bf93313(%rip)
	addq	$8, %rsp
	.cfi_def_cfa_offset 8
.LBB4_2:
	retq
.Lfunc_end4:
	.size	__hip_module_dtor, .Lfunc_end4-__hip_module_dtor
	.cfi_endproc
                                        # -- End function
	.type	fn_array,@object                # @fn_array
	.local	fn_array
	.comm	fn_array,24,16
	.type	_Z11load_fn_ptrPf,@object       # @_Z11load_fn_ptrPf
	.section	.rodata,"a",@progbits
	.globl	_Z11load_fn_ptrPf
	.p2align	3, 0x0
_Z11load_fn_ptrPf:
	.quad	_Z26__device_stub__load_fn_ptrPf
	.size	_Z11load_fn_ptrPf, 8

	.type	_Z7call_fnPKfi,@object          # @_Z7call_fnPKfi
	.globl	_Z7call_fnPKfi
	.p2align	3, 0x0
_Z7call_fnPKfi:
	.quad	_Z22__device_stub__call_fnPKfi
	.size	_Z7call_fnPKfi, 8

	.type	.L__unnamed_1,@object           # @0
	.section	.rodata.str1.1,"aMS",@progbits,1
.L__unnamed_1:
	.asciz	"_Z11load_fn_ptrPf"
	.size	.L__unnamed_1, 18

	.type	.L__unnamed_2,@object           # @1
.L__unnamed_2:
	.asciz	"_Z7call_fnPKfi"
	.size	.L__unnamed_2, 15

	.type	.L__unnamed_3,@object           # @2
.L__unnamed_3:
	.asciz	"fn_array"
	.size	.L__unnamed_3, 9

	.type	__hip_fatbin_wrapper,@object    # @__hip_fatbin_wrapper
	.section	.hipFatBinSegment,"a",@progbits
	.p2align	3, 0x0
__hip_fatbin_wrapper:
	.long	1212764230                      # 0x48495046
	.long	1                               # 0x1
	.quad	__hip_fatbin_79ab1a9f7bf93313
	.quad	0
	.size	__hip_fatbin_wrapper, 24

	.type	__hip_gpubin_handle_79ab1a9f7bf93313,@object # @__hip_gpubin_handle_79ab1a9f7bf93313
	.local	__hip_gpubin_handle_79ab1a9f7bf93313
	.comm	__hip_gpubin_handle_79ab1a9f7bf93313,8,8
	.section	.init_array,"aw",@init_array
	.p2align	3, 0x0
	.quad	__hip_module_ctor
	.type	__hip_cuid_79ab1a9f7bf93313,@object # @__hip_cuid_79ab1a9f7bf93313
	.bss
	.globl	__hip_cuid_79ab1a9f7bf93313
__hip_cuid_79ab1a9f7bf93313:
	.byte	0                               # 0x0
	.size	__hip_cuid_79ab1a9f7bf93313, 1

	.ident	"AMD clang version 20.0.0git (https://github.com/RadeonOpenCompute/llvm-project roc-7.0.2 25385 0dda3adf56766e0aac0d03173ced3759e1ffecbc)"
	.section	".note.GNU-stack","",@progbits
	.addrsig
	.addrsig_sym _Z26__device_stub__load_fn_ptrPf
	.addrsig_sym _Z22__device_stub__call_fnPKfi
	.addrsig_sym __hip_module_ctor
	.addrsig_sym __hip_module_dtor
	.addrsig_sym fn_array
	.addrsig_sym _Z11load_fn_ptrPf
	.addrsig_sym _Z7call_fnPKfi
	.addrsig_sym __hip_fatbin_79ab1a9f7bf93313
	.addrsig_sym __hip_fatbin_wrapper
	.addrsig_sym __hip_cuid_79ab1a9f7bf93313

# __CLANG_OFFLOAD_BUNDLE____END__ host-x86_64-unknown-linux-gnu-
