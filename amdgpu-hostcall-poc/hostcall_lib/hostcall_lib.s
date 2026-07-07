	.amdgcn_target "amdgcn-amd-amdhsa--gfx908:sramecc+:xnack-"
	.amdhsa_code_object_version 6
	.text
	.p2align	2                               ; -- Begin function gpu_fopen
	.type	gpu_fopen,@function
gpu_fopen:                              ; @gpu_fopen
; %bb.0:
	s_waitcnt vmcnt(0) expcnt(0) lgkmcnt(0)
	v_mbcnt_lo_u32_b32 v4, exec_lo, 0
	v_mbcnt_hi_u32_b32 v6, exec_hi, v4
	v_mov_b32_e32 v4, -1
	v_mov_b32_e32 v5, -1
	v_cmp_eq_u32_e32 vcc, 0, v6
	s_and_saveexec_b64 s[4:5], vcc
	s_cbranch_execz .LBB0_24
; %bb.1:
	s_mov_b64 s[8:9], exec
	v_mbcnt_lo_u32_b32 v4, s8, 0
	v_mbcnt_hi_u32_b32 v4, s9, v4
	v_cmp_eq_u32_e32 vcc, 0, v4
                                        ; implicit-def: $vgpr5
	s_and_saveexec_b64 s[6:7], vcc
	s_cbranch_execz .LBB0_3
; %bb.2:
	s_getpc_b64 s[10:11]
	s_add_u32 s10, s10, mailbox@gotpcrel32@lo+4
	s_addc_u32 s11, s11, mailbox@gotpcrel32@hi+12
	s_load_dwordx2 s[10:11], s[10:11], 0x0
	s_bcnt1_i32_b64 s8, s[8:9]
	v_mov_b32_e32 v5, 0
	v_mov_b32_e32 v6, s8
	s_waitcnt lgkmcnt(0)
	global_atomic_add v5, v5, v6, s[10:11] glc
.LBB0_3:
	s_or_b64 exec, exec, s[6:7]
	s_getpc_b64 s[6:7]
	s_add_u32 s6, s6, mailbox@gotpcrel32@lo+4
	s_addc_u32 s7, s7, mailbox@gotpcrel32@hi+12
	s_load_dwordx2 s[6:7], s[6:7], 0x0
	s_waitcnt lgkmcnt(0)
	v_mov_b32_e32 v6, s6
	v_mov_b32_e32 v7, s7
	flat_load_dword v6, v[6:7] offset:4 glc
	s_waitcnt vmcnt(0)
	v_readfirstlane_b32 s6, v5
	v_add_u32_e32 v4, s6, v4
	s_waitcnt lgkmcnt(0)
	v_cmp_ne_u32_e32 vcc, v6, v4
	s_and_saveexec_b64 s[6:7], vcc
	s_cbranch_execz .LBB0_6
; %bb.4:
	s_mov_b64 s[8:9], 0
.LBB0_5:                                ; =>This Inner Loop Header: Depth=1
	s_getpc_b64 s[10:11]
	s_add_u32 s10, s10, mailbox@gotpcrel32@lo+4
	s_addc_u32 s11, s11, mailbox@gotpcrel32@hi+12
	s_load_dwordx2 s[10:11], s[10:11], 0x0
	s_sleep 2
	s_waitcnt lgkmcnt(0)
	v_mov_b32_e32 v5, s10
	v_mov_b32_e32 v6, s11
	flat_load_dword v5, v[5:6] offset:4 glc
	s_waitcnt vmcnt(0) lgkmcnt(0)
	v_cmp_eq_u32_e32 vcc, v5, v4
	s_or_b64 s[8:9], vcc, s[8:9]
	s_andn2_b64 exec, exec, s[8:9]
	s_cbranch_execnz .LBB0_5
.LBB0_6:
	s_or_b64 exec, exec, s[6:7]
	v_cmp_ne_u64_e32 vcc, 0, v[0:1]
	v_mov_b32_e32 v4, 0
	v_mov_b32_e32 v5, 0
	s_mov_b64 s[8:9], 0
	s_and_saveexec_b64 s[6:7], vcc
	s_cbranch_execz .LBB0_12
; %bb.7:
	v_mov_b32_e32 v5, 0
	s_mov_b64 s[12:13], 0
                                        ; implicit-def: $sgpr10_sgpr11
	s_branch .LBB0_9
.LBB0_8:                                ;   in Loop: Header=BB0_9 Depth=1
	s_or_b64 exec, exec, s[14:15]
	s_and_b64 s[14:15], exec, s[10:11]
	s_or_b64 s[8:9], s[14:15], s[8:9]
	s_andn2_b64 exec, exec, s[8:9]
	s_cbranch_execz .LBB0_11
.LBB0_9:                                ; =>This Inner Loop Header: Depth=1
	v_mov_b32_e32 v4, s13
	v_add_co_u32_e32 v6, vcc, s12, v0
	v_addc_co_u32_e32 v7, vcc, v1, v4, vcc
	flat_load_ubyte v6, v[6:7]
	v_mov_b32_e32 v4, s12
	s_or_b64 s[10:11], s[10:11], exec
	s_waitcnt vmcnt(0) lgkmcnt(0)
	v_cmp_ne_u16_e32 vcc, 0, v6
	s_and_saveexec_b64 s[14:15], vcc
	s_cbranch_execz .LBB0_8
; %bb.10:                               ;   in Loop: Header=BB0_9 Depth=1
	s_getpc_b64 s[16:17]
	s_add_u32 s16, s16, mailbox@gotpcrel32@lo+4
	s_addc_u32 s17, s17, mailbox@gotpcrel32@hi+12
	s_load_dwordx2 s[16:17], s[16:17], 0x0
	v_mov_b32_e32 v4, 0x7f
	s_waitcnt lgkmcnt(0)
	s_add_u32 s16, s16, s12
	s_addc_u32 s17, s17, s13
	s_add_u32 s12, s12, 1
	s_addc_u32 s13, s13, 0
	s_cmpk_eq_i32 s12, 0x7f
	global_store_byte v5, v6, s[16:17] offset:32
	s_cselect_b64 s[16:17], -1, 0
	s_andn2_b64 s[10:11], s[10:11], exec
	s_and_b64 s[16:17], s[16:17], exec
	s_or_b64 s[10:11], s[10:11], s[16:17]
	s_branch .LBB0_8
.LBB0_11:
	s_or_b64 exec, exec, s[8:9]
	v_mov_b32_e32 v5, 0
.LBB0_12:
	s_or_b64 exec, exec, s[6:7]
	s_getpc_b64 s[6:7]
	s_add_u32 s6, s6, mailbox@gotpcrel32@lo+4
	s_addc_u32 s7, s7, mailbox@gotpcrel32@hi+12
	s_load_dwordx2 s[6:7], s[6:7], 0x0
	v_mov_b32_e32 v6, 0
	s_mov_b64 s[8:9], 0
	s_waitcnt lgkmcnt(0)
	v_mov_b32_e32 v1, s7
	v_add_co_u32_e32 v0, vcc, s6, v4
	v_addc_co_u32_e32 v1, vcc, v1, v5, vcc
	global_store_byte v[0:1], v6, off offset:32
	v_cmp_ne_u64_e32 vcc, 0, v[2:3]
	v_mov_b32_e32 v0, 0
	v_mov_b32_e32 v1, 0
	s_and_saveexec_b64 s[6:7], vcc
	s_cbranch_execz .LBB0_18
; %bb.13:
	s_mov_b64 s[12:13], 0
                                        ; implicit-def: $sgpr10_sgpr11
	s_branch .LBB0_15
.LBB0_14:                               ;   in Loop: Header=BB0_15 Depth=1
	s_or_b64 exec, exec, s[14:15]
	s_and_b64 s[14:15], exec, s[10:11]
	s_or_b64 s[8:9], s[14:15], s[8:9]
	s_andn2_b64 exec, exec, s[8:9]
	s_cbranch_execz .LBB0_17
.LBB0_15:                               ; =>This Inner Loop Header: Depth=1
	v_mov_b32_e32 v1, s13
	v_add_co_u32_e32 v0, vcc, s12, v2
	v_addc_co_u32_e32 v1, vcc, v3, v1, vcc
	flat_load_ubyte v1, v[0:1]
	v_mov_b32_e32 v0, s12
	s_or_b64 s[10:11], s[10:11], exec
	s_waitcnt vmcnt(0) lgkmcnt(0)
	v_cmp_ne_u16_e32 vcc, 0, v1
	s_and_saveexec_b64 s[14:15], vcc
	s_cbranch_execz .LBB0_14
; %bb.16:                               ;   in Loop: Header=BB0_15 Depth=1
	s_getpc_b64 s[16:17]
	s_add_u32 s16, s16, mailbox@gotpcrel32@lo+4
	s_addc_u32 s17, s17, mailbox@gotpcrel32@hi+12
	s_load_dwordx2 s[16:17], s[16:17], 0x0
	v_mov_b32_e32 v0, 7
	s_waitcnt lgkmcnt(0)
	s_add_u32 s16, s16, s12
	s_addc_u32 s17, s17, s13
	s_add_u32 s12, s12, 1
	s_addc_u32 s13, s13, 0
	s_cmp_eq_u32 s12, 7
	global_store_byte v6, v1, s[16:17] offset:160
	s_cselect_b64 s[16:17], -1, 0
	s_andn2_b64 s[10:11], s[10:11], exec
	s_and_b64 s[16:17], s[16:17], exec
	s_or_b64 s[10:11], s[10:11], s[16:17]
	s_branch .LBB0_14
.LBB0_17:
	s_or_b64 exec, exec, s[8:9]
	v_mov_b32_e32 v1, 0
.LBB0_18:
	s_or_b64 exec, exec, s[6:7]
	s_getpc_b64 s[6:7]
	s_add_u32 s6, s6, mailbox@gotpcrel32@lo+4
	s_addc_u32 s7, s7, mailbox@gotpcrel32@hi+12
	s_load_dwordx2 s[6:7], s[6:7], 0x0
	v_mov_b32_e32 v2, 0
	s_waitcnt lgkmcnt(0)
	v_mov_b32_e32 v3, s7
	v_add_co_u32_e32 v0, vcc, s6, v0
	v_addc_co_u32_e32 v1, vcc, v3, v1, vcc
	global_store_byte v[0:1], v2, off offset:160
	v_mov_b32_e32 v0, s6
	v_mov_b32_e32 v3, 1
	v_mov_b32_e32 v1, s7
	global_store_dword v2, v3, s[6:7] offset:8
	s_dcache_wb
	s_waitcnt vmcnt(0) lgkmcnt(0)
	buffer_wbinvl1_vol
	flat_store_dword v[0:1], v3 offset:12
	s_waitcnt vmcnt(0)
	flat_load_dword v0, v[0:1] offset:12 glc
	s_waitcnt vmcnt(0) lgkmcnt(0)
	v_cmp_ne_u32_e32 vcc, 2, v0
	s_and_saveexec_b64 s[6:7], vcc
	s_cbranch_execz .LBB0_21
; %bb.19:
	s_mov_b64 s[8:9], 0
.LBB0_20:                               ; =>This Inner Loop Header: Depth=1
	s_getpc_b64 s[10:11]
	s_add_u32 s10, s10, mailbox@gotpcrel32@lo+4
	s_addc_u32 s11, s11, mailbox@gotpcrel32@hi+12
	s_load_dwordx2 s[10:11], s[10:11], 0x0
	s_sleep 2
	s_waitcnt lgkmcnt(0)
	v_mov_b32_e32 v0, s10
	v_mov_b32_e32 v1, s11
	flat_load_dword v0, v[0:1] offset:12 glc
	s_waitcnt vmcnt(0) lgkmcnt(0)
	v_cmp_eq_u32_e32 vcc, 2, v0
	s_or_b64 s[8:9], vcc, s[8:9]
	s_andn2_b64 exec, exec, s[8:9]
	s_cbranch_execnz .LBB0_20
.LBB0_21:
	s_or_b64 exec, exec, s[6:7]
	s_getpc_b64 s[8:9]
	s_add_u32 s8, s8, mailbox@gotpcrel32@lo+4
	s_addc_u32 s9, s9, mailbox@gotpcrel32@hi+12
	s_load_dwordx2 s[8:9], s[8:9], 0x0
	v_mov_b32_e32 v0, 0
	s_dcache_inv
	s_waitcnt lgkmcnt(0)
	buffer_wbinvl1_vol
	s_mov_b64 s[6:7], exec
	global_load_dwordx2 v[4:5], v0, s[8:9] offset:16
	v_mov_b32_e32 v1, s8
	v_mov_b32_e32 v2, s9
	flat_store_dword v[1:2], v0 offset:12
	s_waitcnt vmcnt(0)
	v_mbcnt_lo_u32_b32 v1, s6, 0
	v_mbcnt_hi_u32_b32 v1, s7, v1
	v_cmp_eq_u32_e32 vcc, 0, v1
	s_and_saveexec_b64 s[10:11], vcc
	s_cbranch_execz .LBB0_23
; %bb.22:
	s_bcnt1_i32_b64 s6, s[6:7]
	v_mov_b32_e32 v1, s6
	global_atomic_add v0, v1, s[8:9] offset:4
.LBB0_23:
	s_or_b64 exec, exec, s[10:11]
.LBB0_24:
	s_or_b64 exec, exec, s[4:5]
	v_mov_b32_e32 v0, v4
	v_mov_b32_e32 v1, v5
	s_waitcnt vmcnt(0) lgkmcnt(0)
	s_setpc_b64 s[30:31]
.Lfunc_end0:
	.size	gpu_fopen, .Lfunc_end0-gpu_fopen
                                        ; -- End function
	.set .Lgpu_fopen.num_vgpr, 8
	.set .Lgpu_fopen.num_agpr, 0
	.set .Lgpu_fopen.numbered_sgpr, 32
	.set .Lgpu_fopen.private_seg_size, 0
	.set .Lgpu_fopen.uses_vcc, 1
	.set .Lgpu_fopen.uses_flat_scratch, 0
	.set .Lgpu_fopen.has_dyn_sized_stack, 0
	.set .Lgpu_fopen.has_recursion, 0
	.set .Lgpu_fopen.has_indirect_call, 0
	.section	.AMDGPU.csdata,"",@progbits
; Function info:
; codeLenInByte = 1096
; TotalNumSgprs: 34
; NumVgprs: 8
; NumAgprs: 0
; TotalNumVgprs: 8
; ScratchSize: 0
; MemoryBound: 0
	.text
	.p2align	2                               ; -- Begin function gpu_fwrite
	.type	gpu_fwrite,@function
gpu_fwrite:                             ; @gpu_fwrite
; %bb.0:
	s_waitcnt vmcnt(0) expcnt(0) lgkmcnt(0)
	v_mov_b32_e32 v6, v3
	v_mbcnt_lo_u32_b32 v3, exec_lo, 0
	v_mbcnt_hi_u32_b32 v3, exec_hi, v3
	v_mov_b32_e32 v5, v2
	v_mov_b32_e32 v2, 0
	v_cmp_eq_u32_e32 vcc, 0, v3
	s_and_saveexec_b64 s[6:7], vcc
	s_cbranch_execz .LBB1_15
; %bb.1:
	s_mov_b64 s[8:9], exec
	v_mbcnt_lo_u32_b32 v2, s8, 0
	v_mbcnt_hi_u32_b32 v2, s9, v2
	v_cmp_eq_u32_e32 vcc, 0, v2
                                        ; implicit-def: $vgpr3
	s_and_saveexec_b64 s[4:5], vcc
	s_cbranch_execz .LBB1_3
; %bb.2:
	s_getpc_b64 s[10:11]
	s_add_u32 s10, s10, mailbox@gotpcrel32@lo+4
	s_addc_u32 s11, s11, mailbox@gotpcrel32@hi+12
	s_load_dwordx2 s[10:11], s[10:11], 0x0
	s_bcnt1_i32_b64 s8, s[8:9]
	v_mov_b32_e32 v3, 0
	v_mov_b32_e32 v7, s8
	s_waitcnt lgkmcnt(0)
	global_atomic_add v3, v3, v7, s[10:11] glc
.LBB1_3:
	s_or_b64 exec, exec, s[4:5]
	s_getpc_b64 s[4:5]
	s_add_u32 s4, s4, mailbox@gotpcrel32@lo+4
	s_addc_u32 s5, s5, mailbox@gotpcrel32@hi+12
	s_load_dwordx2 s[4:5], s[4:5], 0x0
	s_waitcnt lgkmcnt(0)
	v_mov_b32_e32 v8, s5
	v_mov_b32_e32 v7, s4
	flat_load_dword v7, v[7:8] offset:4 glc
	s_waitcnt vmcnt(0)
	v_readfirstlane_b32 s4, v3
	v_add_u32_e32 v2, s4, v2
	s_waitcnt lgkmcnt(0)
	v_cmp_ne_u32_e32 vcc, v7, v2
	s_and_saveexec_b64 s[4:5], vcc
	s_cbranch_execz .LBB1_6
; %bb.4:
	s_mov_b64 s[8:9], 0
.LBB1_5:                                ; =>This Inner Loop Header: Depth=1
	s_getpc_b64 s[10:11]
	s_add_u32 s10, s10, mailbox@gotpcrel32@lo+4
	s_addc_u32 s11, s11, mailbox@gotpcrel32@hi+12
	s_load_dwordx2 s[10:11], s[10:11], 0x0
	s_sleep 2
	s_waitcnt lgkmcnt(0)
	v_mov_b32_e32 v7, s10
	v_mov_b32_e32 v8, s11
	flat_load_dword v3, v[7:8] offset:4 glc
	s_waitcnt vmcnt(0) lgkmcnt(0)
	v_cmp_eq_u32_e32 vcc, v3, v2
	s_or_b64 s[8:9], vcc, s[8:9]
	s_andn2_b64 exec, exec, s[8:9]
	s_cbranch_execnz .LBB1_5
.LBB1_6:
	s_or_b64 exec, exec, s[4:5]
	v_min_i32_e32 v2, 0x200, v4
	v_cmp_lt_i32_e32 vcc, 0, v4
	s_and_saveexec_b64 s[8:9], vcc
	s_cbranch_execz .LBB1_9
; %bb.7:
	s_getpc_b64 s[4:5]
	s_add_u32 s4, s4, mailbox@gotpcrel32@lo+4
	s_addc_u32 s5, s5, mailbox@gotpcrel32@hi+12
	s_load_dwordx2 s[4:5], s[4:5], 0x0
	s_mov_b64 s[10:11], 0
	v_mov_b32_e32 v3, 0
	v_mov_b32_e32 v4, v2
	s_waitcnt lgkmcnt(0)
	s_add_u32 s12, s4, 0xa8
	s_addc_u32 s13, s5, 0
.LBB1_8:                                ; =>This Inner Loop Header: Depth=1
	flat_load_ubyte v7, v[5:6]
	v_add_u32_e32 v4, -1, v4
	v_add_co_u32_e32 v5, vcc, 1, v5
	v_cmp_eq_u32_e64 s[4:5], 0, v4
	v_addc_co_u32_e32 v6, vcc, 0, v6, vcc
	s_waitcnt vmcnt(0) lgkmcnt(0)
	global_store_byte v3, v7, s[12:13]
	s_add_u32 s12, s12, 1
	s_addc_u32 s13, s13, 0
	s_or_b64 s[10:11], s[4:5], s[10:11]
	s_andn2_b64 exec, exec, s[10:11]
	s_cbranch_execnz .LBB1_8
.LBB1_9:
	s_or_b64 exec, exec, s[8:9]
	s_getpc_b64 s[4:5]
	s_add_u32 s4, s4, mailbox@gotpcrel32@lo+4
	s_addc_u32 s5, s5, mailbox@gotpcrel32@hi+12
	s_load_dwordx2 s[4:5], s[4:5], 0x0
	v_mov_b32_e32 v3, 0
	v_mov_b32_e32 v4, 3
	s_waitcnt lgkmcnt(0)
	global_store_dwordx3 v3, v[0:2], s[4:5] offset:16
	global_store_dword v3, v4, s[4:5] offset:8
	v_mov_b32_e32 v0, s4
	v_mov_b32_e32 v2, 1
	v_mov_b32_e32 v1, s5
	s_dcache_wb
	s_waitcnt vmcnt(0) lgkmcnt(0)
	buffer_wbinvl1_vol
	flat_store_dword v[0:1], v2 offset:12
	s_waitcnt vmcnt(0)
	flat_load_dword v0, v[0:1] offset:12 glc
	s_waitcnt vmcnt(0) lgkmcnt(0)
	v_cmp_ne_u32_e32 vcc, 2, v0
	s_and_saveexec_b64 s[4:5], vcc
	s_cbranch_execz .LBB1_12
; %bb.10:
	s_mov_b64 s[8:9], 0
.LBB1_11:                               ; =>This Inner Loop Header: Depth=1
	s_getpc_b64 s[10:11]
	s_add_u32 s10, s10, mailbox@gotpcrel32@lo+4
	s_addc_u32 s11, s11, mailbox@gotpcrel32@hi+12
	s_load_dwordx2 s[10:11], s[10:11], 0x0
	s_sleep 2
	s_waitcnt lgkmcnt(0)
	v_mov_b32_e32 v0, s10
	v_mov_b32_e32 v1, s11
	flat_load_dword v0, v[0:1] offset:12 glc
	s_waitcnt vmcnt(0) lgkmcnt(0)
	v_cmp_eq_u32_e32 vcc, 2, v0
	s_or_b64 s[8:9], vcc, s[8:9]
	s_andn2_b64 exec, exec, s[8:9]
	s_cbranch_execnz .LBB1_11
.LBB1_12:
	s_or_b64 exec, exec, s[4:5]
	s_getpc_b64 s[8:9]
	s_add_u32 s8, s8, mailbox@gotpcrel32@lo+4
	s_addc_u32 s9, s9, mailbox@gotpcrel32@hi+12
	s_load_dwordx2 s[8:9], s[8:9], 0x0
	v_mov_b32_e32 v0, 0
	s_dcache_inv
	s_waitcnt lgkmcnt(0)
	buffer_wbinvl1_vol
	s_mov_b64 s[4:5], exec
	global_load_dword v2, v0, s[8:9] offset:28
	v_mbcnt_lo_u32_b32 v1, s4, 0
	v_mov_b32_e32 v3, s8
	v_mbcnt_hi_u32_b32 v1, s5, v1
	v_mov_b32_e32 v4, s9
	v_cmp_eq_u32_e32 vcc, 0, v1
	flat_store_dword v[3:4], v0 offset:12
	s_waitcnt vmcnt(0)
	s_and_saveexec_b64 s[10:11], vcc
	s_cbranch_execz .LBB1_14
; %bb.13:
	s_bcnt1_i32_b64 s4, s[4:5]
	v_mov_b32_e32 v1, s4
	global_atomic_add v0, v1, s[8:9] offset:4
.LBB1_14:
	s_or_b64 exec, exec, s[10:11]
.LBB1_15:
	s_or_b64 exec, exec, s[6:7]
	v_mov_b32_e32 v0, v2
	s_waitcnt vmcnt(0) lgkmcnt(0)
	s_setpc_b64 s[30:31]
.Lfunc_end1:
	.size	gpu_fwrite, .Lfunc_end1-gpu_fwrite
                                        ; -- End function
	.set .Lgpu_fwrite.num_vgpr, 9
	.set .Lgpu_fwrite.num_agpr, 0
	.set .Lgpu_fwrite.numbered_sgpr, 32
	.set .Lgpu_fwrite.private_seg_size, 0
	.set .Lgpu_fwrite.uses_vcc, 1
	.set .Lgpu_fwrite.uses_flat_scratch, 0
	.set .Lgpu_fwrite.has_dyn_sized_stack, 0
	.set .Lgpu_fwrite.has_recursion, 0
	.set .Lgpu_fwrite.has_indirect_call, 0
	.section	.AMDGPU.csdata,"",@progbits
; Function info:
; codeLenInByte = 780
; TotalNumSgprs: 34
; NumVgprs: 9
; NumAgprs: 0
; TotalNumVgprs: 9
; ScratchSize: 0
; MemoryBound: 0
	.text
	.p2align	2                               ; -- Begin function gpu_fread
	.type	gpu_fread,@function
gpu_fread:                              ; @gpu_fread
; %bb.0:
	s_waitcnt vmcnt(0) expcnt(0) lgkmcnt(0)
	v_mov_b32_e32 v6, v2
	v_mbcnt_lo_u32_b32 v2, exec_lo, 0
	v_mbcnt_hi_u32_b32 v2, exec_hi, v2
	v_mov_b32_e32 v7, v3
	v_mov_b32_e32 v5, 0
	v_cmp_eq_u32_e32 vcc, 0, v2
	s_and_saveexec_b64 s[4:5], vcc
	s_cbranch_execz .LBB2_15
; %bb.1:
	s_mov_b64 s[8:9], exec
	v_mbcnt_lo_u32_b32 v2, s8, 0
	v_mbcnt_hi_u32_b32 v2, s9, v2
	v_cmp_eq_u32_e32 vcc, 0, v2
                                        ; implicit-def: $vgpr3
	s_and_saveexec_b64 s[6:7], vcc
	s_cbranch_execz .LBB2_3
; %bb.2:
	s_getpc_b64 s[10:11]
	s_add_u32 s10, s10, mailbox@gotpcrel32@lo+4
	s_addc_u32 s11, s11, mailbox@gotpcrel32@hi+12
	s_load_dwordx2 s[10:11], s[10:11], 0x0
	s_bcnt1_i32_b64 s8, s[8:9]
	v_mov_b32_e32 v3, 0
	v_mov_b32_e32 v5, s8
	s_waitcnt lgkmcnt(0)
	global_atomic_add v3, v3, v5, s[10:11] glc
.LBB2_3:
	s_or_b64 exec, exec, s[6:7]
	s_getpc_b64 s[6:7]
	s_add_u32 s6, s6, mailbox@gotpcrel32@lo+4
	s_addc_u32 s7, s7, mailbox@gotpcrel32@hi+12
	s_load_dwordx2 s[6:7], s[6:7], 0x0
	s_waitcnt lgkmcnt(0)
	v_mov_b32_e32 v9, s7
	v_mov_b32_e32 v8, s6
	flat_load_dword v5, v[8:9] offset:4 glc
	s_waitcnt vmcnt(0)
	v_readfirstlane_b32 s6, v3
	v_add_u32_e32 v2, s6, v2
	s_waitcnt lgkmcnt(0)
	v_cmp_ne_u32_e32 vcc, v5, v2
	s_and_saveexec_b64 s[6:7], vcc
	s_cbranch_execz .LBB2_6
; %bb.4:
	s_mov_b64 s[8:9], 0
.LBB2_5:                                ; =>This Inner Loop Header: Depth=1
	s_getpc_b64 s[10:11]
	s_add_u32 s10, s10, mailbox@gotpcrel32@lo+4
	s_addc_u32 s11, s11, mailbox@gotpcrel32@hi+12
	s_load_dwordx2 s[10:11], s[10:11], 0x0
	s_sleep 2
	s_waitcnt lgkmcnt(0)
	v_mov_b32_e32 v8, s10
	v_mov_b32_e32 v9, s11
	flat_load_dword v3, v[8:9] offset:4 glc
	s_waitcnt vmcnt(0) lgkmcnt(0)
	v_cmp_eq_u32_e32 vcc, v3, v2
	s_or_b64 s[8:9], vcc, s[8:9]
	s_andn2_b64 exec, exec, s[8:9]
	s_cbranch_execnz .LBB2_5
.LBB2_6:
	s_or_b64 exec, exec, s[6:7]
	s_getpc_b64 s[6:7]
	s_add_u32 s6, s6, mailbox@gotpcrel32@lo+4
	s_addc_u32 s7, s7, mailbox@gotpcrel32@hi+12
	s_load_dwordx2 s[6:7], s[6:7], 0x0
	v_min_i32_e32 v2, 0x200, v4
	v_mov_b32_e32 v3, 0
	v_mov_b32_e32 v4, 2
	s_waitcnt lgkmcnt(0)
	global_store_dwordx3 v3, v[0:2], s[6:7] offset:16
	global_store_dword v3, v4, s[6:7] offset:8
	v_mov_b32_e32 v0, s6
	v_mov_b32_e32 v3, 1
	v_mov_b32_e32 v1, s7
	s_dcache_wb
	s_waitcnt vmcnt(0) lgkmcnt(0)
	buffer_wbinvl1_vol
	flat_store_dword v[0:1], v3 offset:12
	s_waitcnt vmcnt(0)
	flat_load_dword v0, v[0:1] offset:12 glc
	s_waitcnt vmcnt(0) lgkmcnt(0)
	v_cmp_ne_u32_e32 vcc, 2, v0
	s_and_saveexec_b64 s[6:7], vcc
	s_cbranch_execz .LBB2_9
; %bb.7:
	s_mov_b64 s[8:9], 0
.LBB2_8:                                ; =>This Inner Loop Header: Depth=1
	s_getpc_b64 s[10:11]
	s_add_u32 s10, s10, mailbox@gotpcrel32@lo+4
	s_addc_u32 s11, s11, mailbox@gotpcrel32@hi+12
	s_load_dwordx2 s[10:11], s[10:11], 0x0
	s_sleep 2
	s_waitcnt lgkmcnt(0)
	v_mov_b32_e32 v0, s10
	v_mov_b32_e32 v1, s11
	flat_load_dword v0, v[0:1] offset:12 glc
	s_waitcnt vmcnt(0) lgkmcnt(0)
	v_cmp_eq_u32_e32 vcc, 2, v0
	s_or_b64 s[8:9], vcc, s[8:9]
	s_andn2_b64 exec, exec, s[8:9]
	s_cbranch_execnz .LBB2_8
.LBB2_9:
	s_or_b64 exec, exec, s[6:7]
	s_getpc_b64 s[6:7]
	s_add_u32 s6, s6, mailbox@gotpcrel32@lo+4
	s_addc_u32 s7, s7, mailbox@gotpcrel32@hi+12
	s_load_dwordx2 s[8:9], s[6:7], 0x0
	v_mov_b32_e32 v0, 0
	s_dcache_inv
	s_waitcnt lgkmcnt(0)
	buffer_wbinvl1_vol
	global_load_dword v5, v0, s[8:9] offset:28
	s_waitcnt vmcnt(0)
	v_min_i32_e32 v1, v5, v2
	v_cmp_lt_i32_e32 vcc, 0, v1
	s_and_saveexec_b64 s[6:7], vcc
	s_cbranch_execz .LBB2_12
; %bb.10:
	s_add_u32 s8, s8, 0xa8
	s_addc_u32 s9, s9, 0
	s_mov_b64 s[10:11], 0
.LBB2_11:                               ; =>This Inner Loop Header: Depth=1
	global_load_ubyte v2, v0, s[8:9]
	v_add_u32_e32 v1, -1, v1
	s_add_u32 s8, s8, 1
	s_addc_u32 s9, s9, 0
	v_cmp_eq_u32_e32 vcc, 0, v1
	s_or_b64 s[10:11], vcc, s[10:11]
	s_waitcnt vmcnt(0)
	flat_store_byte v[6:7], v2
	v_add_co_u32_e32 v6, vcc, 1, v6
	v_addc_co_u32_e32 v7, vcc, 0, v7, vcc
	s_andn2_b64 exec, exec, s[10:11]
	s_cbranch_execnz .LBB2_11
.LBB2_12:
	s_or_b64 exec, exec, s[6:7]
	s_getpc_b64 s[6:7]
	s_add_u32 s6, s6, mailbox@gotpcrel32@lo+4
	s_addc_u32 s7, s7, mailbox@gotpcrel32@hi+12
	s_load_dwordx2 s[6:7], s[6:7], 0x0
	s_mov_b64 s[8:9], exec
	v_mbcnt_lo_u32_b32 v1, s8, 0
	v_mbcnt_hi_u32_b32 v3, s9, v1
	v_mov_b32_e32 v0, 0
	s_waitcnt lgkmcnt(0)
	v_mov_b32_e32 v1, s6
	v_mov_b32_e32 v2, s7
	v_cmp_eq_u32_e32 vcc, 0, v3
	flat_store_dword v[1:2], v0 offset:12
	s_waitcnt vmcnt(0)
	s_and_saveexec_b64 s[10:11], vcc
	s_cbranch_execz .LBB2_14
; %bb.13:
	s_bcnt1_i32_b64 s8, s[8:9]
	v_mov_b32_e32 v1, s8
	global_atomic_add v0, v1, s[6:7] offset:4
.LBB2_14:
	s_or_b64 exec, exec, s[10:11]
.LBB2_15:
	s_or_b64 exec, exec, s[4:5]
	v_mov_b32_e32 v0, v5
	s_waitcnt vmcnt(0) lgkmcnt(0)
	s_setpc_b64 s[30:31]
.Lfunc_end2:
	.size	gpu_fread, .Lfunc_end2-gpu_fread
                                        ; -- End function
	.set .Lgpu_fread.num_vgpr, 10
	.set .Lgpu_fread.num_agpr, 0
	.set .Lgpu_fread.numbered_sgpr, 32
	.set .Lgpu_fread.private_seg_size, 0
	.set .Lgpu_fread.uses_vcc, 1
	.set .Lgpu_fread.uses_flat_scratch, 0
	.set .Lgpu_fread.has_dyn_sized_stack, 0
	.set .Lgpu_fread.has_recursion, 0
	.set .Lgpu_fread.has_indirect_call, 0
	.section	.AMDGPU.csdata,"",@progbits
; Function info:
; codeLenInByte = 780
; TotalNumSgprs: 34
; NumVgprs: 10
; NumAgprs: 0
; TotalNumVgprs: 10
; ScratchSize: 0
; MemoryBound: 0
	.text
	.p2align	2                               ; -- Begin function hc_open
	.type	hc_open,@function
hc_open:                                ; @hc_open
; %bb.0:
	s_waitcnt vmcnt(0) expcnt(0) lgkmcnt(0)
	v_mbcnt_lo_u32_b32 v0, exec_lo, 0
	v_mbcnt_hi_u32_b32 v0, exec_hi, v0
	v_cmp_eq_u32_e32 vcc, 0, v0
	s_and_saveexec_b64 s[4:5], vcc
	s_cbranch_execz .LBB3_13
; %bb.1:
	s_mov_b64 s[8:9], exec
	v_mbcnt_lo_u32_b32 v0, s8, 0
	v_mbcnt_hi_u32_b32 v0, s9, v0
	v_cmp_eq_u32_e32 vcc, 0, v0
                                        ; implicit-def: $vgpr1
	s_and_saveexec_b64 s[6:7], vcc
	s_cbranch_execz .LBB3_3
; %bb.2:
	s_getpc_b64 s[10:11]
	s_add_u32 s10, s10, mailbox@gotpcrel32@lo+4
	s_addc_u32 s11, s11, mailbox@gotpcrel32@hi+12
	s_load_dwordx2 s[10:11], s[10:11], 0x0
	s_bcnt1_i32_b64 s8, s[8:9]
	v_mov_b32_e32 v1, 0
	v_mov_b32_e32 v2, s8
	s_waitcnt lgkmcnt(0)
	global_atomic_add v1, v1, v2, s[10:11] glc
.LBB3_3:
	s_or_b64 exec, exec, s[6:7]
	s_getpc_b64 s[6:7]
	s_add_u32 s6, s6, mailbox@gotpcrel32@lo+4
	s_addc_u32 s7, s7, mailbox@gotpcrel32@hi+12
	s_load_dwordx2 s[6:7], s[6:7], 0x0
	s_waitcnt lgkmcnt(0)
	v_mov_b32_e32 v2, s6
	v_mov_b32_e32 v3, s7
	flat_load_dword v2, v[2:3] offset:4 glc
	s_waitcnt vmcnt(0)
	v_readfirstlane_b32 s6, v1
	v_add_u32_e32 v0, s6, v0
	s_waitcnt lgkmcnt(0)
	v_cmp_ne_u32_e32 vcc, v2, v0
	s_and_saveexec_b64 s[6:7], vcc
	s_cbranch_execz .LBB3_6
; %bb.4:
	s_mov_b64 s[8:9], 0
.LBB3_5:                                ; =>This Inner Loop Header: Depth=1
	s_getpc_b64 s[10:11]
	s_add_u32 s10, s10, mailbox@gotpcrel32@lo+4
	s_addc_u32 s11, s11, mailbox@gotpcrel32@hi+12
	s_load_dwordx2 s[10:11], s[10:11], 0x0
	s_sleep 2
	s_waitcnt lgkmcnt(0)
	v_mov_b32_e32 v1, s10
	v_mov_b32_e32 v2, s11
	flat_load_dword v1, v[1:2] offset:4 glc
	s_waitcnt vmcnt(0) lgkmcnt(0)
	v_cmp_eq_u32_e32 vcc, v1, v0
	s_or_b64 s[8:9], vcc, s[8:9]
	s_andn2_b64 exec, exec, s[8:9]
	s_cbranch_execnz .LBB3_5
.LBB3_6:
	s_or_b64 exec, exec, s[6:7]
	s_mov_b64 s[6:7], 0
	v_mov_b32_e32 v0, 0
.LBB3_7:                                ; =>This Inner Loop Header: Depth=1
	s_getpc_b64 s[8:9]
	s_add_u32 s8, s8, __const.hc_open.fn@rel32@lo+4
	s_addc_u32 s9, s9, __const.hc_open.fn@rel32@hi+12
	s_add_u32 s8, s6, s8
	s_addc_u32 s9, s7, s9
	global_load_ubyte v1, v0, s[8:9]
	s_getpc_b64 s[8:9]
	s_add_u32 s8, s8, mailbox@gotpcrel32@lo+4
	s_addc_u32 s9, s9, mailbox@gotpcrel32@hi+12
	s_load_dwordx2 s[8:9], s[8:9], 0x0
	s_waitcnt lgkmcnt(0)
	s_add_u32 s10, s8, s6
	s_addc_u32 s11, s9, s7
	s_add_u32 s6, s6, 1
	s_addc_u32 s7, s7, 0
	s_cmp_eq_u32 s6, 18
	s_waitcnt vmcnt(0)
	global_store_byte v0, v1, s[10:11] offset:32
	s_cbranch_scc0 .LBB3_7
; %bb.8:
	v_mov_b32_e32 v0, 0
	v_mov_b32_e32 v1, 0x77
	v_mov_b32_e32 v2, 1
	global_store_short v0, v1, s[8:9] offset:160
	global_store_dword v0, v2, s[8:9] offset:8
	v_mov_b32_e32 v0, s8
	v_mov_b32_e32 v1, s9
	s_dcache_wb
	s_waitcnt vmcnt(0) lgkmcnt(0)
	buffer_wbinvl1_vol
	flat_store_dword v[0:1], v2 offset:12
	s_waitcnt vmcnt(0)
	flat_load_dword v0, v[0:1] offset:12 glc
	s_waitcnt vmcnt(0) lgkmcnt(0)
	v_cmp_ne_u32_e32 vcc, 2, v0
	s_and_saveexec_b64 s[6:7], vcc
	s_cbranch_execz .LBB3_11
; %bb.9:
	s_mov_b64 s[8:9], 0
.LBB3_10:                               ; =>This Inner Loop Header: Depth=1
	s_getpc_b64 s[10:11]
	s_add_u32 s10, s10, mailbox@gotpcrel32@lo+4
	s_addc_u32 s11, s11, mailbox@gotpcrel32@hi+12
	s_load_dwordx2 s[10:11], s[10:11], 0x0
	s_sleep 2
	s_waitcnt lgkmcnt(0)
	v_mov_b32_e32 v0, s10
	v_mov_b32_e32 v1, s11
	flat_load_dword v0, v[0:1] offset:12 glc
	s_waitcnt vmcnt(0) lgkmcnt(0)
	v_cmp_eq_u32_e32 vcc, 2, v0
	s_or_b64 s[8:9], vcc, s[8:9]
	s_andn2_b64 exec, exec, s[8:9]
	s_cbranch_execnz .LBB3_10
.LBB3_11:
	s_or_b64 exec, exec, s[6:7]
	s_getpc_b64 s[8:9]
	s_add_u32 s8, s8, mailbox@gotpcrel32@lo+4
	s_addc_u32 s9, s9, mailbox@gotpcrel32@hi+12
	s_load_dwordx2 s[8:9], s[8:9], 0x0
	s_mov_b64 s[6:7], exec
	v_mov_b32_e32 v0, 0
	s_dcache_inv
	s_waitcnt lgkmcnt(0)
	buffer_wbinvl1_vol
	v_mov_b32_e32 v1, s8
	v_mov_b32_e32 v2, s9
	flat_store_dword v[1:2], v0 offset:12
	s_waitcnt vmcnt(0)
	v_mbcnt_lo_u32_b32 v1, s6, 0
	v_mbcnt_hi_u32_b32 v1, s7, v1
	v_cmp_eq_u32_e32 vcc, 0, v1
	s_and_b64 s[10:11], exec, vcc
	s_mov_b64 exec, s[10:11]
	s_cbranch_execz .LBB3_13
; %bb.12:
	s_bcnt1_i32_b64 s6, s[6:7]
	v_mov_b32_e32 v1, s6
	global_atomic_add v0, v1, s[8:9] offset:4
.LBB3_13:
	s_or_b64 exec, exec, s[4:5]
	s_waitcnt vmcnt(0) lgkmcnt(0)
	s_setpc_b64 s[30:31]
.Lfunc_end3:
	.size	hc_open, .Lfunc_end3-hc_open
                                        ; -- End function
	.set .Lhc_open.num_vgpr, 4
	.set .Lhc_open.num_agpr, 0
	.set .Lhc_open.numbered_sgpr, 32
	.set .Lhc_open.private_seg_size, 0
	.set .Lhc_open.uses_vcc, 1
	.set .Lhc_open.uses_flat_scratch, 0
	.set .Lhc_open.has_dyn_sized_stack, 0
	.set .Lhc_open.has_recursion, 0
	.set .Lhc_open.has_indirect_call, 0
	.section	.AMDGPU.csdata,"",@progbits
; Function info:
; codeLenInByte = 700
; TotalNumSgprs: 34
; NumVgprs: 4
; NumAgprs: 0
; TotalNumVgprs: 4
; ScratchSize: 0
; MemoryBound: 0
	.text
	.p2align	2                               ; -- Begin function hc_write
	.type	hc_write,@function
hc_write:                               ; @hc_write
; %bb.0:
	s_waitcnt vmcnt(0) expcnt(0) lgkmcnt(0)
	v_mbcnt_lo_u32_b32 v0, exec_lo, 0
	v_mbcnt_hi_u32_b32 v0, exec_hi, v0
	v_cmp_eq_u32_e32 vcc, 0, v0
	s_and_saveexec_b64 s[4:5], vcc
	s_cbranch_execz .LBB4_13
; %bb.1:
	s_mov_b64 s[8:9], exec
	v_mbcnt_lo_u32_b32 v0, s8, 0
	v_mbcnt_hi_u32_b32 v0, s9, v0
	v_cmp_eq_u32_e32 vcc, 0, v0
                                        ; implicit-def: $vgpr1
	s_and_saveexec_b64 s[6:7], vcc
	s_cbranch_execz .LBB4_3
; %bb.2:
	s_getpc_b64 s[10:11]
	s_add_u32 s10, s10, mailbox@gotpcrel32@lo+4
	s_addc_u32 s11, s11, mailbox@gotpcrel32@hi+12
	s_load_dwordx2 s[10:11], s[10:11], 0x0
	s_bcnt1_i32_b64 s8, s[8:9]
	v_mov_b32_e32 v1, 0
	v_mov_b32_e32 v2, s8
	s_waitcnt lgkmcnt(0)
	global_atomic_add v1, v1, v2, s[10:11] glc
.LBB4_3:
	s_or_b64 exec, exec, s[6:7]
	s_getpc_b64 s[6:7]
	s_add_u32 s6, s6, mailbox@gotpcrel32@lo+4
	s_addc_u32 s7, s7, mailbox@gotpcrel32@hi+12
	s_load_dwordx2 s[6:7], s[6:7], 0x0
	s_waitcnt lgkmcnt(0)
	v_mov_b32_e32 v2, s6
	v_mov_b32_e32 v3, s7
	flat_load_dword v2, v[2:3] offset:4 glc
	s_waitcnt vmcnt(0)
	v_readfirstlane_b32 s6, v1
	v_add_u32_e32 v0, s6, v0
	s_waitcnt lgkmcnt(0)
	v_cmp_ne_u32_e32 vcc, v2, v0
	s_and_saveexec_b64 s[6:7], vcc
	s_cbranch_execz .LBB4_6
; %bb.4:
	s_mov_b64 s[8:9], 0
.LBB4_5:                                ; =>This Inner Loop Header: Depth=1
	s_getpc_b64 s[10:11]
	s_add_u32 s10, s10, mailbox@gotpcrel32@lo+4
	s_addc_u32 s11, s11, mailbox@gotpcrel32@hi+12
	s_load_dwordx2 s[10:11], s[10:11], 0x0
	s_sleep 2
	s_waitcnt lgkmcnt(0)
	v_mov_b32_e32 v1, s10
	v_mov_b32_e32 v2, s11
	flat_load_dword v1, v[1:2] offset:4 glc
	s_waitcnt vmcnt(0) lgkmcnt(0)
	v_cmp_eq_u32_e32 vcc, v1, v0
	s_or_b64 s[8:9], vcc, s[8:9]
	s_andn2_b64 exec, exec, s[8:9]
	s_cbranch_execnz .LBB4_5
.LBB4_6:
	s_or_b64 exec, exec, s[6:7]
	s_mov_b64 s[6:7], 0
	v_mov_b32_e32 v0, 0
.LBB4_7:                                ; =>This Inner Loop Header: Depth=1
	s_getpc_b64 s[8:9]
	s_add_u32 s8, s8, __const.hc_write.msg@rel32@lo+4
	s_addc_u32 s9, s9, __const.hc_write.msg@rel32@hi+12
	s_add_u32 s8, s6, s8
	s_addc_u32 s9, s7, s9
	global_load_ubyte v1, v0, s[8:9]
	s_getpc_b64 s[8:9]
	s_add_u32 s8, s8, mailbox@gotpcrel32@lo+4
	s_addc_u32 s9, s9, mailbox@gotpcrel32@hi+12
	s_load_dwordx2 s[8:9], s[8:9], 0x0
	s_waitcnt lgkmcnt(0)
	s_add_u32 s10, s8, s6
	s_addc_u32 s11, s9, s7
	s_add_u32 s6, s6, 1
	s_addc_u32 s7, s7, 0
	s_cmp_lg_u32 s6, 23
	s_waitcnt vmcnt(0)
	global_store_byte v0, v1, s[10:11] offset:168
	s_cbranch_scc1 .LBB4_7
; %bb.8:
	v_mov_b32_e32 v0, 0
	v_mov_b32_e32 v1, 23
	global_store_dword v0, v1, s[8:9] offset:24
	v_mov_b32_e32 v1, 3
	global_store_dword v0, v1, s[8:9] offset:8
	v_mov_b32_e32 v0, s8
	v_mov_b32_e32 v2, 1
	v_mov_b32_e32 v1, s9
	s_dcache_wb
	s_waitcnt vmcnt(0) lgkmcnt(0)
	buffer_wbinvl1_vol
	flat_store_dword v[0:1], v2 offset:12
	s_waitcnt vmcnt(0)
	flat_load_dword v0, v[0:1] offset:12 glc
	s_waitcnt vmcnt(0) lgkmcnt(0)
	v_cmp_ne_u32_e32 vcc, 2, v0
	s_and_saveexec_b64 s[6:7], vcc
	s_cbranch_execz .LBB4_11
; %bb.9:
	s_mov_b64 s[8:9], 0
.LBB4_10:                               ; =>This Inner Loop Header: Depth=1
	s_getpc_b64 s[10:11]
	s_add_u32 s10, s10, mailbox@gotpcrel32@lo+4
	s_addc_u32 s11, s11, mailbox@gotpcrel32@hi+12
	s_load_dwordx2 s[10:11], s[10:11], 0x0
	s_sleep 2
	s_waitcnt lgkmcnt(0)
	v_mov_b32_e32 v0, s10
	v_mov_b32_e32 v1, s11
	flat_load_dword v0, v[0:1] offset:12 glc
	s_waitcnt vmcnt(0) lgkmcnt(0)
	v_cmp_eq_u32_e32 vcc, 2, v0
	s_or_b64 s[8:9], vcc, s[8:9]
	s_andn2_b64 exec, exec, s[8:9]
	s_cbranch_execnz .LBB4_10
.LBB4_11:
	s_or_b64 exec, exec, s[6:7]
	s_getpc_b64 s[8:9]
	s_add_u32 s8, s8, mailbox@gotpcrel32@lo+4
	s_addc_u32 s9, s9, mailbox@gotpcrel32@hi+12
	s_load_dwordx2 s[8:9], s[8:9], 0x0
	s_mov_b64 s[6:7], exec
	v_mov_b32_e32 v0, 0
	s_dcache_inv
	s_waitcnt lgkmcnt(0)
	buffer_wbinvl1_vol
	v_mov_b32_e32 v1, s8
	v_mov_b32_e32 v2, s9
	flat_store_dword v[1:2], v0 offset:12
	s_waitcnt vmcnt(0)
	v_mbcnt_lo_u32_b32 v1, s6, 0
	v_mbcnt_hi_u32_b32 v1, s7, v1
	v_cmp_eq_u32_e32 vcc, 0, v1
	s_and_b64 s[10:11], exec, vcc
	s_mov_b64 exec, s[10:11]
	s_cbranch_execz .LBB4_13
; %bb.12:
	s_bcnt1_i32_b64 s6, s[6:7]
	v_mov_b32_e32 v1, s6
	global_atomic_add v0, v1, s[8:9] offset:4
.LBB4_13:
	s_or_b64 exec, exec, s[4:5]
	s_waitcnt vmcnt(0) lgkmcnt(0)
	s_setpc_b64 s[30:31]
.Lfunc_end4:
	.size	hc_write, .Lfunc_end4-hc_write
                                        ; -- End function
	.set .Lhc_write.num_vgpr, 4
	.set .Lhc_write.num_agpr, 0
	.set .Lhc_write.numbered_sgpr, 32
	.set .Lhc_write.private_seg_size, 0
	.set .Lhc_write.uses_vcc, 1
	.set .Lhc_write.uses_flat_scratch, 0
	.set .Lhc_write.has_dyn_sized_stack, 0
	.set .Lhc_write.has_recursion, 0
	.set .Lhc_write.has_indirect_call, 0
	.section	.AMDGPU.csdata,"",@progbits
; Function info:
; codeLenInByte = 700
; TotalNumSgprs: 34
; NumVgprs: 4
; NumAgprs: 0
; TotalNumVgprs: 4
; ScratchSize: 0
; MemoryBound: 0
	.text
	.p2align	2                               ; -- Begin function hc_close
	.type	hc_close,@function
hc_close:                               ; @hc_close
; %bb.0:
	s_waitcnt vmcnt(0) expcnt(0) lgkmcnt(0)
	v_mbcnt_lo_u32_b32 v0, exec_lo, 0
	v_mbcnt_hi_u32_b32 v0, exec_hi, v0
	v_cmp_eq_u32_e32 vcc, 0, v0
	s_and_saveexec_b64 s[4:5], vcc
	s_cbranch_execz .LBB5_11
; %bb.1:
	s_mov_b64 s[8:9], exec
	v_mbcnt_lo_u32_b32 v0, s8, 0
	v_mbcnt_hi_u32_b32 v0, s9, v0
	v_cmp_eq_u32_e32 vcc, 0, v0
                                        ; implicit-def: $vgpr1
	s_and_saveexec_b64 s[6:7], vcc
	s_cbranch_execz .LBB5_3
; %bb.2:
	s_getpc_b64 s[10:11]
	s_add_u32 s10, s10, mailbox@gotpcrel32@lo+4
	s_addc_u32 s11, s11, mailbox@gotpcrel32@hi+12
	s_load_dwordx2 s[10:11], s[10:11], 0x0
	s_bcnt1_i32_b64 s8, s[8:9]
	v_mov_b32_e32 v1, 0
	v_mov_b32_e32 v2, s8
	s_waitcnt lgkmcnt(0)
	global_atomic_add v1, v1, v2, s[10:11] glc
.LBB5_3:
	s_or_b64 exec, exec, s[6:7]
	s_getpc_b64 s[6:7]
	s_add_u32 s6, s6, mailbox@gotpcrel32@lo+4
	s_addc_u32 s7, s7, mailbox@gotpcrel32@hi+12
	s_load_dwordx2 s[6:7], s[6:7], 0x0
	s_waitcnt lgkmcnt(0)
	v_mov_b32_e32 v2, s6
	v_mov_b32_e32 v3, s7
	flat_load_dword v2, v[2:3] offset:4 glc
	s_waitcnt vmcnt(0)
	v_readfirstlane_b32 s6, v1
	v_add_u32_e32 v0, s6, v0
	s_waitcnt lgkmcnt(0)
	v_cmp_ne_u32_e32 vcc, v2, v0
	s_and_saveexec_b64 s[6:7], vcc
	s_cbranch_execz .LBB5_6
; %bb.4:
	s_mov_b64 s[8:9], 0
.LBB5_5:                                ; =>This Inner Loop Header: Depth=1
	s_getpc_b64 s[10:11]
	s_add_u32 s10, s10, mailbox@gotpcrel32@lo+4
	s_addc_u32 s11, s11, mailbox@gotpcrel32@hi+12
	s_load_dwordx2 s[10:11], s[10:11], 0x0
	s_sleep 2
	s_waitcnt lgkmcnt(0)
	v_mov_b32_e32 v1, s10
	v_mov_b32_e32 v2, s11
	flat_load_dword v1, v[1:2] offset:4 glc
	s_waitcnt vmcnt(0) lgkmcnt(0)
	v_cmp_eq_u32_e32 vcc, v1, v0
	s_or_b64 s[8:9], vcc, s[8:9]
	s_andn2_b64 exec, exec, s[8:9]
	s_cbranch_execnz .LBB5_5
.LBB5_6:
	s_or_b64 exec, exec, s[6:7]
	s_getpc_b64 s[6:7]
	s_add_u32 s6, s6, mailbox@gotpcrel32@lo+4
	s_addc_u32 s7, s7, mailbox@gotpcrel32@hi+12
	s_load_dwordx2 s[6:7], s[6:7], 0x0
	v_mov_b32_e32 v0, 0
	v_mov_b32_e32 v1, 4
	v_mov_b32_e32 v2, 1
	s_waitcnt lgkmcnt(0)
	global_store_dword v0, v1, s[6:7] offset:8
	v_mov_b32_e32 v0, s6
	v_mov_b32_e32 v1, s7
	s_dcache_wb
	s_waitcnt vmcnt(0) lgkmcnt(0)
	buffer_wbinvl1_vol
	flat_store_dword v[0:1], v2 offset:12
	s_waitcnt vmcnt(0)
	flat_load_dword v0, v[0:1] offset:12 glc
	s_waitcnt vmcnt(0) lgkmcnt(0)
	v_cmp_ne_u32_e32 vcc, 2, v0
	s_and_saveexec_b64 s[6:7], vcc
	s_cbranch_execz .LBB5_9
; %bb.7:
	s_mov_b64 s[8:9], 0
.LBB5_8:                                ; =>This Inner Loop Header: Depth=1
	s_getpc_b64 s[10:11]
	s_add_u32 s10, s10, mailbox@gotpcrel32@lo+4
	s_addc_u32 s11, s11, mailbox@gotpcrel32@hi+12
	s_load_dwordx2 s[10:11], s[10:11], 0x0
	s_sleep 2
	s_waitcnt lgkmcnt(0)
	v_mov_b32_e32 v0, s10
	v_mov_b32_e32 v1, s11
	flat_load_dword v0, v[0:1] offset:12 glc
	s_waitcnt vmcnt(0) lgkmcnt(0)
	v_cmp_eq_u32_e32 vcc, 2, v0
	s_or_b64 s[8:9], vcc, s[8:9]
	s_andn2_b64 exec, exec, s[8:9]
	s_cbranch_execnz .LBB5_8
.LBB5_9:
	s_or_b64 exec, exec, s[6:7]
	s_getpc_b64 s[8:9]
	s_add_u32 s8, s8, mailbox@gotpcrel32@lo+4
	s_addc_u32 s9, s9, mailbox@gotpcrel32@hi+12
	s_load_dwordx2 s[8:9], s[8:9], 0x0
	s_mov_b64 s[6:7], exec
	v_mov_b32_e32 v0, 0
	s_dcache_inv
	s_waitcnt lgkmcnt(0)
	buffer_wbinvl1_vol
	v_mov_b32_e32 v1, s8
	v_mov_b32_e32 v2, s9
	flat_store_dword v[1:2], v0 offset:12
	s_waitcnt vmcnt(0)
	v_mbcnt_lo_u32_b32 v1, s6, 0
	v_mbcnt_hi_u32_b32 v1, s7, v1
	v_cmp_eq_u32_e32 vcc, 0, v1
	s_and_b64 s[10:11], exec, vcc
	s_mov_b64 exec, s[10:11]
	s_cbranch_execz .LBB5_11
; %bb.10:
	s_bcnt1_i32_b64 s6, s[6:7]
	v_mov_b32_e32 v1, s6
	global_atomic_add v0, v1, s[8:9] offset:4
.LBB5_11:
	s_or_b64 exec, exec, s[4:5]
	s_waitcnt vmcnt(0) lgkmcnt(0)
	s_setpc_b64 s[30:31]
.Lfunc_end5:
	.size	hc_close, .Lfunc_end5-hc_close
                                        ; -- End function
	.set .Lhc_close.num_vgpr, 4
	.set .Lhc_close.num_agpr, 0
	.set .Lhc_close.numbered_sgpr, 32
	.set .Lhc_close.private_seg_size, 0
	.set .Lhc_close.uses_vcc, 1
	.set .Lhc_close.uses_flat_scratch, 0
	.set .Lhc_close.has_dyn_sized_stack, 0
	.set .Lhc_close.has_recursion, 0
	.set .Lhc_close.has_indirect_call, 0
	.section	.AMDGPU.csdata,"",@progbits
; Function info:
; codeLenInByte = 608
; TotalNumSgprs: 34
; NumVgprs: 4
; NumAgprs: 0
; TotalNumVgprs: 4
; ScratchSize: 0
; MemoryBound: 0
	.section	.AMDGPU.gpr_maximums,"",@progbits
	.set amdgpu.max_num_vgpr, 10
	.set amdgpu.max_num_agpr, 0
	.set amdgpu.max_num_sgpr, 32
	.section	.AMDGPU.csdata,"",@progbits
	.type	__const.hc_open.fn,@object      ; @__const.hc_open.fn
	.section	.rodata.str1.16,"aMS",@progbits,1
	.p2align	4, 0x0
__const.hc_open.fn:
	.asciz	"dyninst_trace.txt"
	.size	__const.hc_open.fn, 18

	.type	__const.hc_write.msg,@object    ; @__const.hc_write.msg
	.p2align	4, 0x0
__const.hc_write.msg:
	.asciz	"[gpu] function entered\n"
	.size	__const.hc_write.msg, 24

	.type	__hip_cuid_5c46e04ef7c3c37a,@object ; @__hip_cuid_5c46e04ef7c3c37a
	.section	.bss,"aw",@nobits
	.globl	__hip_cuid_5c46e04ef7c3c37a
__hip_cuid_5c46e04ef7c3c37a:
	.byte	0                               ; 0x0
	.size	__hip_cuid_5c46e04ef7c3c37a, 1

	.ident	"AMD clang version 20.0.0git (https://github.com/RadeonOpenCompute/llvm-project roc-7.0.2 25385 0dda3adf56766e0aac0d03173ced3759e1ffecbc)"
	.section	".note.GNU-stack","",@progbits
	.addrsig
	.addrsig_sym gpu_fopen
	.addrsig_sym gpu_fwrite
	.addrsig_sym gpu_fread
	.addrsig_sym hc_open
	.addrsig_sym hc_write
	.addrsig_sym hc_close
	.addrsig_sym mailbox
	.addrsig_sym __hip_cuid_5c46e04ef7c3c37a
	.amdgpu_metadata
---
amdhsa.kernels:  []
amdhsa.target:   'amdgcn-amd-amdhsa--gfx908:sramecc+:xnack-'
amdhsa.version:
  - 1
  - 2
...

	.end_amdgpu_metadata
