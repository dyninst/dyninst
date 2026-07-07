	.amdgcn_target "amdgcn-amd-amdhsa--gfx908"
	.amdhsa_code_object_version 6
	.text
	.p2align	2                               ; -- Begin function gpu_hostcall
	.type	gpu_hostcall,@function
gpu_hostcall:                           ; @gpu_hostcall
; %bb.0:
	s_waitcnt vmcnt(0) expcnt(0) lgkmcnt(0)
	v_and_b32_e32 v3, 63, v31
	v_cmp_eq_u32_e32 vcc, 0, v3
	s_and_saveexec_b64 s[4:5], vcc
	s_cbranch_execz .LBB0_39
; %bb.1:
	v_cmp_ne_u64_e32 vcc, 0, v[1:2]
	s_and_saveexec_b64 s[6:7], vcc
	s_cbranch_execz .LBB0_35
; %bb.2:
	flat_load_ubyte v3, v[1:2]
	s_waitcnt vmcnt(0) lgkmcnt(0)
	v_cmp_ne_u16_e32 vcc, 0, v3
	s_and_b64 exec, exec, vcc
	s_cbranch_execz .LBB0_35
; %bb.3:
	s_getpc_b64 s[8:9]
	s_add_u32 s8, s8, mailbox@gotpcrel32@lo+4
	s_addc_u32 s9, s9, mailbox@gotpcrel32@hi+12
	s_load_dwordx2 s[8:9], s[8:9], 0x0
	v_mov_b32_e32 v4, 0
	s_waitcnt lgkmcnt(0)
	global_store_byte v4, v3, s[8:9] offset:16
	flat_load_ubyte v3, v[1:2] offset:1
	s_waitcnt vmcnt(0) lgkmcnt(0)
	v_cmp_ne_u16_e32 vcc, 0, v3
	s_and_saveexec_b64 s[10:11], vcc
	s_xor_b64 s[10:11], exec, s[10:11]
	s_cbranch_execz .LBB0_35
; %bb.4:
	global_store_byte v4, v3, s[8:9] offset:17
	flat_load_ubyte v3, v[1:2] offset:2
	s_waitcnt vmcnt(0) lgkmcnt(0)
	v_cmp_ne_u16_e32 vcc, 0, v3
	s_and_saveexec_b64 s[8:9], vcc
	s_xor_b64 s[8:9], exec, s[8:9]
	s_cbranch_execz .LBB0_35
; %bb.5:
	s_getpc_b64 s[8:9]
	s_add_u32 s8, s8, mailbox@gotpcrel32@lo+4
	s_addc_u32 s9, s9, mailbox@gotpcrel32@hi+12
	s_load_dwordx2 s[8:9], s[8:9], 0x0
	v_mov_b32_e32 v4, 0
	s_waitcnt lgkmcnt(0)
	global_store_byte v4, v3, s[8:9] offset:18
	flat_load_ubyte v3, v[1:2] offset:3
	s_waitcnt vmcnt(0) lgkmcnt(0)
	v_cmp_ne_u16_e32 vcc, 0, v3
	s_and_saveexec_b64 s[10:11], vcc
	s_xor_b64 s[10:11], exec, s[10:11]
	s_cbranch_execz .LBB0_35
; %bb.6:
	global_store_byte v4, v3, s[8:9] offset:19
	flat_load_ubyte v3, v[1:2] offset:4
	s_waitcnt vmcnt(0) lgkmcnt(0)
	v_cmp_ne_u16_e32 vcc, 0, v3
	s_and_saveexec_b64 s[8:9], vcc
	s_xor_b64 s[8:9], exec, s[8:9]
	s_cbranch_execz .LBB0_35
; %bb.7:
	s_getpc_b64 s[8:9]
	s_add_u32 s8, s8, mailbox@gotpcrel32@lo+4
	s_addc_u32 s9, s9, mailbox@gotpcrel32@hi+12
	s_load_dwordx2 s[8:9], s[8:9], 0x0
	v_mov_b32_e32 v4, 0
	s_waitcnt lgkmcnt(0)
	global_store_byte v4, v3, s[8:9] offset:20
	flat_load_ubyte v3, v[1:2] offset:5
	s_waitcnt vmcnt(0) lgkmcnt(0)
	v_cmp_ne_u16_e32 vcc, 0, v3
	s_and_saveexec_b64 s[10:11], vcc
	s_xor_b64 s[10:11], exec, s[10:11]
	s_cbranch_execz .LBB0_35
; %bb.8:
	global_store_byte v4, v3, s[8:9] offset:21
	flat_load_ubyte v3, v[1:2] offset:6
	s_waitcnt vmcnt(0) lgkmcnt(0)
	v_cmp_ne_u16_e32 vcc, 0, v3
	s_and_saveexec_b64 s[8:9], vcc
	s_xor_b64 s[8:9], exec, s[8:9]
	s_cbranch_execz .LBB0_35
; %bb.9:
	s_getpc_b64 s[8:9]
	s_add_u32 s8, s8, mailbox@gotpcrel32@lo+4
	s_addc_u32 s9, s9, mailbox@gotpcrel32@hi+12
	s_load_dwordx2 s[8:9], s[8:9], 0x0
	v_mov_b32_e32 v4, 0
	s_waitcnt lgkmcnt(0)
	global_store_byte v4, v3, s[8:9] offset:22
	flat_load_ubyte v3, v[1:2] offset:7
	s_waitcnt vmcnt(0) lgkmcnt(0)
	v_cmp_ne_u16_e32 vcc, 0, v3
	s_and_saveexec_b64 s[10:11], vcc
	s_xor_b64 s[10:11], exec, s[10:11]
	s_cbranch_execz .LBB0_35
; %bb.10:
	global_store_byte v4, v3, s[8:9] offset:23
	flat_load_ubyte v3, v[1:2] offset:8
	s_waitcnt vmcnt(0) lgkmcnt(0)
	v_cmp_ne_u16_e32 vcc, 0, v3
	s_and_saveexec_b64 s[8:9], vcc
	s_xor_b64 s[8:9], exec, s[8:9]
	s_cbranch_execz .LBB0_35
; %bb.11:
	s_getpc_b64 s[8:9]
	s_add_u32 s8, s8, mailbox@gotpcrel32@lo+4
	s_addc_u32 s9, s9, mailbox@gotpcrel32@hi+12
	s_load_dwordx2 s[8:9], s[8:9], 0x0
	v_mov_b32_e32 v4, 0
	s_waitcnt lgkmcnt(0)
	global_store_byte v4, v3, s[8:9] offset:24
	flat_load_ubyte v3, v[1:2] offset:9
	s_waitcnt vmcnt(0) lgkmcnt(0)
	v_cmp_ne_u16_e32 vcc, 0, v3
	s_and_saveexec_b64 s[10:11], vcc
	s_xor_b64 s[10:11], exec, s[10:11]
	s_cbranch_execz .LBB0_35
; %bb.12:
	global_store_byte v4, v3, s[8:9] offset:25
	flat_load_ubyte v3, v[1:2] offset:10
	s_waitcnt vmcnt(0) lgkmcnt(0)
	v_cmp_ne_u16_e32 vcc, 0, v3
	s_and_saveexec_b64 s[8:9], vcc
	s_xor_b64 s[8:9], exec, s[8:9]
	s_cbranch_execz .LBB0_35
; %bb.13:
	s_getpc_b64 s[8:9]
	s_add_u32 s8, s8, mailbox@gotpcrel32@lo+4
	s_addc_u32 s9, s9, mailbox@gotpcrel32@hi+12
	s_load_dwordx2 s[8:9], s[8:9], 0x0
	v_mov_b32_e32 v4, 0
	s_waitcnt lgkmcnt(0)
	global_store_byte v4, v3, s[8:9] offset:26
	flat_load_ubyte v3, v[1:2] offset:11
	s_waitcnt vmcnt(0) lgkmcnt(0)
	v_cmp_ne_u16_e32 vcc, 0, v3
	s_and_saveexec_b64 s[10:11], vcc
	s_xor_b64 s[10:11], exec, s[10:11]
	s_cbranch_execz .LBB0_35
; %bb.14:
	global_store_byte v4, v3, s[8:9] offset:27
	flat_load_ubyte v3, v[1:2] offset:12
	s_waitcnt vmcnt(0) lgkmcnt(0)
	v_cmp_ne_u16_e32 vcc, 0, v3
	s_and_saveexec_b64 s[8:9], vcc
	s_xor_b64 s[8:9], exec, s[8:9]
	s_cbranch_execz .LBB0_35
; %bb.15:
	s_getpc_b64 s[8:9]
	s_add_u32 s8, s8, mailbox@gotpcrel32@lo+4
	s_addc_u32 s9, s9, mailbox@gotpcrel32@hi+12
	s_load_dwordx2 s[8:9], s[8:9], 0x0
	v_mov_b32_e32 v4, 0
	s_waitcnt lgkmcnt(0)
	global_store_byte v4, v3, s[8:9] offset:28
	flat_load_ubyte v3, v[1:2] offset:13
	s_waitcnt vmcnt(0) lgkmcnt(0)
	v_cmp_ne_u16_e32 vcc, 0, v3
	s_and_saveexec_b64 s[10:11], vcc
	s_xor_b64 s[10:11], exec, s[10:11]
	s_cbranch_execz .LBB0_35
; %bb.16:
	global_store_byte v4, v3, s[8:9] offset:29
	flat_load_ubyte v3, v[1:2] offset:14
	s_waitcnt vmcnt(0) lgkmcnt(0)
	v_cmp_ne_u16_e32 vcc, 0, v3
	s_and_saveexec_b64 s[8:9], vcc
	s_xor_b64 s[8:9], exec, s[8:9]
	s_cbranch_execz .LBB0_35
; %bb.17:
	s_getpc_b64 s[8:9]
	s_add_u32 s8, s8, mailbox@gotpcrel32@lo+4
	s_addc_u32 s9, s9, mailbox@gotpcrel32@hi+12
	s_load_dwordx2 s[8:9], s[8:9], 0x0
	v_mov_b32_e32 v4, 0
	s_waitcnt lgkmcnt(0)
	global_store_byte v4, v3, s[8:9] offset:30
	flat_load_ubyte v3, v[1:2] offset:15
	s_waitcnt vmcnt(0) lgkmcnt(0)
	v_cmp_ne_u16_e32 vcc, 0, v3
	s_and_saveexec_b64 s[10:11], vcc
	s_xor_b64 s[10:11], exec, s[10:11]
	s_cbranch_execz .LBB0_35
; %bb.18:
	global_store_byte v4, v3, s[8:9] offset:31
	flat_load_ubyte v3, v[1:2] offset:16
	s_waitcnt vmcnt(0) lgkmcnt(0)
	v_cmp_ne_u16_e32 vcc, 0, v3
	s_and_saveexec_b64 s[8:9], vcc
	s_xor_b64 s[8:9], exec, s[8:9]
	s_cbranch_execz .LBB0_35
; %bb.19:
	s_getpc_b64 s[8:9]
	s_add_u32 s8, s8, mailbox@gotpcrel32@lo+4
	s_addc_u32 s9, s9, mailbox@gotpcrel32@hi+12
	s_load_dwordx2 s[8:9], s[8:9], 0x0
	v_mov_b32_e32 v4, 0
	s_waitcnt lgkmcnt(0)
	global_store_byte v4, v3, s[8:9] offset:32
	flat_load_ubyte v3, v[1:2] offset:17
	s_waitcnt vmcnt(0) lgkmcnt(0)
	v_cmp_ne_u16_e32 vcc, 0, v3
	s_and_saveexec_b64 s[10:11], vcc
	s_xor_b64 s[10:11], exec, s[10:11]
	s_cbranch_execz .LBB0_35
; %bb.20:
	global_store_byte v4, v3, s[8:9] offset:33
	flat_load_ubyte v3, v[1:2] offset:18
	s_waitcnt vmcnt(0) lgkmcnt(0)
	v_cmp_ne_u16_e32 vcc, 0, v3
	s_and_saveexec_b64 s[8:9], vcc
	s_xor_b64 s[8:9], exec, s[8:9]
	s_cbranch_execz .LBB0_35
; %bb.21:
	s_getpc_b64 s[8:9]
	s_add_u32 s8, s8, mailbox@gotpcrel32@lo+4
	s_addc_u32 s9, s9, mailbox@gotpcrel32@hi+12
	s_load_dwordx2 s[8:9], s[8:9], 0x0
	v_mov_b32_e32 v4, 0
	s_waitcnt lgkmcnt(0)
	global_store_byte v4, v3, s[8:9] offset:34
	flat_load_ubyte v3, v[1:2] offset:19
	s_waitcnt vmcnt(0) lgkmcnt(0)
	v_cmp_ne_u16_e32 vcc, 0, v3
	s_and_saveexec_b64 s[10:11], vcc
	s_xor_b64 s[10:11], exec, s[10:11]
	s_cbranch_execz .LBB0_35
; %bb.22:
	global_store_byte v4, v3, s[8:9] offset:35
	flat_load_ubyte v3, v[1:2] offset:20
	s_waitcnt vmcnt(0) lgkmcnt(0)
	v_cmp_ne_u16_e32 vcc, 0, v3
	s_and_saveexec_b64 s[8:9], vcc
	s_xor_b64 s[8:9], exec, s[8:9]
	s_cbranch_execz .LBB0_35
; %bb.23:
	s_getpc_b64 s[8:9]
	s_add_u32 s8, s8, mailbox@gotpcrel32@lo+4
	s_addc_u32 s9, s9, mailbox@gotpcrel32@hi+12
	s_load_dwordx2 s[8:9], s[8:9], 0x0
	v_mov_b32_e32 v4, 0
	s_waitcnt lgkmcnt(0)
	global_store_byte v4, v3, s[8:9] offset:36
	flat_load_ubyte v3, v[1:2] offset:21
	s_waitcnt vmcnt(0) lgkmcnt(0)
	v_cmp_ne_u16_e32 vcc, 0, v3
	s_and_saveexec_b64 s[10:11], vcc
	s_xor_b64 s[10:11], exec, s[10:11]
	s_cbranch_execz .LBB0_35
; %bb.24:
	global_store_byte v4, v3, s[8:9] offset:37
	flat_load_ubyte v3, v[1:2] offset:22
	s_waitcnt vmcnt(0) lgkmcnt(0)
	v_cmp_ne_u16_e32 vcc, 0, v3
	s_and_saveexec_b64 s[8:9], vcc
	s_xor_b64 s[8:9], exec, s[8:9]
	s_cbranch_execz .LBB0_35
; %bb.25:
	s_getpc_b64 s[8:9]
	s_add_u32 s8, s8, mailbox@gotpcrel32@lo+4
	s_addc_u32 s9, s9, mailbox@gotpcrel32@hi+12
	s_load_dwordx2 s[8:9], s[8:9], 0x0
	v_mov_b32_e32 v4, 0
	s_waitcnt lgkmcnt(0)
	global_store_byte v4, v3, s[8:9] offset:38
	flat_load_ubyte v3, v[1:2] offset:23
	s_waitcnt vmcnt(0) lgkmcnt(0)
	v_cmp_ne_u16_e32 vcc, 0, v3
	s_and_saveexec_b64 s[10:11], vcc
	s_xor_b64 s[10:11], exec, s[10:11]
	s_cbranch_execz .LBB0_35
; %bb.26:
	global_store_byte v4, v3, s[8:9] offset:39
	flat_load_ubyte v3, v[1:2] offset:24
	s_waitcnt vmcnt(0) lgkmcnt(0)
	v_cmp_ne_u16_e32 vcc, 0, v3
	s_and_saveexec_b64 s[8:9], vcc
	s_xor_b64 s[8:9], exec, s[8:9]
	s_cbranch_execz .LBB0_35
; %bb.27:
	s_getpc_b64 s[8:9]
	s_add_u32 s8, s8, mailbox@gotpcrel32@lo+4
	s_addc_u32 s9, s9, mailbox@gotpcrel32@hi+12
	s_load_dwordx2 s[8:9], s[8:9], 0x0
	v_mov_b32_e32 v4, 0
	s_waitcnt lgkmcnt(0)
	global_store_byte v4, v3, s[8:9] offset:40
	flat_load_ubyte v3, v[1:2] offset:25
	s_waitcnt vmcnt(0) lgkmcnt(0)
	v_cmp_ne_u16_e32 vcc, 0, v3
	s_and_saveexec_b64 s[10:11], vcc
	s_xor_b64 s[10:11], exec, s[10:11]
	s_cbranch_execz .LBB0_35
; %bb.28:
	global_store_byte v4, v3, s[8:9] offset:41
	flat_load_ubyte v3, v[1:2] offset:26
	s_waitcnt vmcnt(0) lgkmcnt(0)
	v_cmp_ne_u16_e32 vcc, 0, v3
	s_and_saveexec_b64 s[8:9], vcc
	s_xor_b64 s[8:9], exec, s[8:9]
	s_cbranch_execz .LBB0_35
; %bb.29:
	s_getpc_b64 s[8:9]
	s_add_u32 s8, s8, mailbox@gotpcrel32@lo+4
	s_addc_u32 s9, s9, mailbox@gotpcrel32@hi+12
	s_load_dwordx2 s[8:9], s[8:9], 0x0
	v_mov_b32_e32 v4, 0
	s_waitcnt lgkmcnt(0)
	global_store_byte v4, v3, s[8:9] offset:42
	flat_load_ubyte v3, v[1:2] offset:27
	s_waitcnt vmcnt(0) lgkmcnt(0)
	v_cmp_ne_u16_e32 vcc, 0, v3
	s_and_saveexec_b64 s[10:11], vcc
	s_xor_b64 s[10:11], exec, s[10:11]
	s_cbranch_execz .LBB0_35
; %bb.30:
	global_store_byte v4, v3, s[8:9] offset:43
	flat_load_ubyte v3, v[1:2] offset:28
	s_waitcnt vmcnt(0) lgkmcnt(0)
	v_cmp_ne_u16_e32 vcc, 0, v3
	s_and_saveexec_b64 s[8:9], vcc
	s_xor_b64 s[8:9], exec, s[8:9]
	s_cbranch_execz .LBB0_35
; %bb.31:
	s_getpc_b64 s[8:9]
	s_add_u32 s8, s8, mailbox@gotpcrel32@lo+4
	s_addc_u32 s9, s9, mailbox@gotpcrel32@hi+12
	s_load_dwordx2 s[8:9], s[8:9], 0x0
	v_mov_b32_e32 v4, 0
	s_waitcnt lgkmcnt(0)
	global_store_byte v4, v3, s[8:9] offset:44
	flat_load_ubyte v3, v[1:2] offset:29
	s_waitcnt vmcnt(0) lgkmcnt(0)
	v_cmp_ne_u16_e32 vcc, 0, v3
	s_and_saveexec_b64 s[10:11], vcc
	s_xor_b64 s[10:11], exec, s[10:11]
	s_cbranch_execz .LBB0_35
; %bb.32:
	global_store_byte v4, v3, s[8:9] offset:45
	flat_load_ubyte v3, v[1:2] offset:30
	s_waitcnt vmcnt(0) lgkmcnt(0)
	v_cmp_ne_u16_e32 vcc, 0, v3
	s_and_saveexec_b64 s[8:9], vcc
	s_xor_b64 s[8:9], exec, s[8:9]
	s_cbranch_execz .LBB0_35
; %bb.33:
	s_getpc_b64 s[8:9]
	s_add_u32 s8, s8, mailbox@gotpcrel32@lo+4
	s_addc_u32 s9, s9, mailbox@gotpcrel32@hi+12
	s_load_dwordx2 s[8:9], s[8:9], 0x0
	v_mov_b32_e32 v4, 0
	s_waitcnt lgkmcnt(0)
	global_store_byte v4, v3, s[8:9] offset:46
	flat_load_ubyte v1, v[1:2] offset:31
	s_waitcnt vmcnt(0) lgkmcnt(0)
	v_cmp_ne_u16_e32 vcc, 0, v1
	s_and_saveexec_b64 s[10:11], vcc
	s_xor_b64 s[10:11], exec, s[10:11]
	s_cbranch_execz .LBB0_35
; %bb.34:
	global_store_byte v4, v1, s[8:9] offset:47
.LBB0_35:
	s_or_b64 exec, exec, s[6:7]
	s_getpc_b64 s[6:7]
	s_add_u32 s6, s6, mailbox@gotpcrel32@lo+4
	s_addc_u32 s7, s7, mailbox@gotpcrel32@hi+12
	s_load_dwordx2 s[6:7], s[6:7], 0x0
	v_mov_b32_e32 v1, 0
	v_mov_b32_e32 v2, 1
	s_waitcnt lgkmcnt(0)
	global_store_dword v1, v0, s[6:7] offset:8
	v_mov_b32_e32 v0, s6
	v_mov_b32_e32 v1, s7
	s_waitcnt vmcnt(0)
	buffer_wbinvl1_vol
	flat_store_dword v[0:1], v2 offset:12
	s_waitcnt vmcnt(0)
	flat_load_dword v0, v[0:1] offset:12 glc
	s_waitcnt vmcnt(0) lgkmcnt(0)
	v_cmp_ne_u32_e32 vcc, 2, v0
	s_and_saveexec_b64 s[6:7], vcc
	s_cbranch_execz .LBB0_38
; %bb.36:
	s_mov_b64 s[8:9], 0
.LBB0_37:                               ; =>This Inner Loop Header: Depth=1
	s_getpc_b64 s[10:11]
	s_add_u32 s10, s10, mailbox@gotpcrel32@lo+4
	s_addc_u32 s11, s11, mailbox@gotpcrel32@hi+12
	s_load_dwordx2 s[10:11], s[10:11], 0x0
	s_sleep 64
	s_waitcnt lgkmcnt(0)
	v_mov_b32_e32 v0, s10
	v_mov_b32_e32 v1, s11
	flat_load_dword v0, v[0:1] offset:12 glc
	s_waitcnt vmcnt(0) lgkmcnt(0)
	v_cmp_eq_u32_e32 vcc, 2, v0
	s_or_b64 s[8:9], vcc, s[8:9]
	s_andn2_b64 exec, exec, s[8:9]
	s_cbranch_execnz .LBB0_37
.LBB0_38:
	s_or_b64 exec, exec, s[6:7]
	s_getpc_b64 s[6:7]
	s_add_u32 s6, s6, mailbox@gotpcrel32@lo+4
	s_addc_u32 s7, s7, mailbox@gotpcrel32@hi+12
	s_load_dwordx2 s[6:7], s[6:7], 0x0
	v_mov_b32_e32 v2, 0
	s_waitcnt lgkmcnt(0)
	v_mov_b32_e32 v0, s6
	v_mov_b32_e32 v1, s7
	flat_store_dword v[0:1], v2 offset:12
	s_waitcnt vmcnt(0)
.LBB0_39:
	s_or_b64 exec, exec, s[4:5]
	; wave barrier
	s_waitcnt lgkmcnt(0)
	s_setpc_b64 s[30:31]
.Lfunc_end0:
	.size	gpu_hostcall, .Lfunc_end0-gpu_hostcall
                                        ; -- End function
	.set .Lgpu_hostcall.num_vgpr, 32
	.set .Lgpu_hostcall.num_agpr, 0
	.set .Lgpu_hostcall.numbered_sgpr, 32
	.set .Lgpu_hostcall.private_seg_size, 0
	.set .Lgpu_hostcall.uses_vcc, 1
	.set .Lgpu_hostcall.uses_flat_scratch, 0
	.set .Lgpu_hostcall.has_dyn_sized_stack, 0
	.set .Lgpu_hostcall.has_recursion, 0
	.set .Lgpu_hostcall.has_indirect_call, 0
	.section	.AMDGPU.csdata,"",@progbits
; Function info:
; codeLenInByte = 2012
; TotalNumSgprs: 36
; NumVgprs: 32
; NumAgprs: 0
; TotalNumVgprs: 32
; ScratchSize: 0
; MemoryBound: 0
	.section	.AMDGPU.gpr_maximums,"",@progbits
	.set amdgpu.max_num_vgpr, 32
	.set amdgpu.max_num_agpr, 0
	.set amdgpu.max_num_sgpr, 32
	.section	.AMDGPU.csdata,"",@progbits
	.type	__hip_cuid_f76af1a619d8d91c,@object ; @__hip_cuid_f76af1a619d8d91c
	.section	.bss,"aw",@nobits
	.globl	__hip_cuid_f76af1a619d8d91c
__hip_cuid_f76af1a619d8d91c:
	.byte	0                               ; 0x0
	.size	__hip_cuid_f76af1a619d8d91c, 1

	.ident	"AMD clang version 20.0.0git (https://github.com/RadeonOpenCompute/llvm-project roc-7.0.2 25385 0dda3adf56766e0aac0d03173ced3759e1ffecbc)"
	.section	".note.GNU-stack","",@progbits
	.addrsig
	.addrsig_sym gpu_hostcall
	.addrsig_sym mailbox
	.addrsig_sym __hip_cuid_f76af1a619d8d91c
	.amdgpu_metadata
---
amdhsa.kernels:  []
amdhsa.target:   amdgcn-amd-amdhsa--gfx908
amdhsa.version:
  - 1
  - 2
...

	.end_amdgpu_metadata
