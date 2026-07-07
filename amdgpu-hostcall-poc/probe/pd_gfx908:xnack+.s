	.amdgcn_target "amdgcn-amd-amdhsa--gfx908:xnack+"
	.amdhsa_code_object_version 6
	.text
	.protected	probe_kernel            ; -- Begin function probe_kernel
	.globl	probe_kernel
	.p2align	8
	.type	probe_kernel,@function
probe_kernel:                           ; @probe_kernel
; %bb.0:
	v_cmp_eq_u32_e32 vcc, 0, v0
	s_and_saveexec_b64 s[0:1], vcc
	s_cbranch_execz .LBB0_5
; %bb.1:
	s_getpc_b64 s[2:3]
	s_add_u32 s2, s2, mailbox@gotpcrel32@lo+4
	s_addc_u32 s3, s3, mailbox@gotpcrel32@hi+12
	s_load_dwordx2 s[2:3], s[2:3], 0x0
	v_mov_b32_e32 v3, 0
	v_mov_b32_e32 v0, 0x5550
	v_mov_b32_e32 v2, 0x47206d6f
	v_mov_b32_e32 v1, 0x7266206f
	s_waitcnt lgkmcnt(0)
	global_store_byte v3, v3, s[2:3] offset:30
	global_store_short v3, v0, s[2:3] offset:28
	v_mov_b32_e32 v0, 0x6c6c6548
	global_store_dwordx3 v3, v[0:2], s[2:3] offset:16
	s_nop 0
	v_mov_b32_e32 v0, 42
	global_store_dword v3, v0, s[2:3] offset:8
	v_mov_b32_e32 v0, s2
	v_mov_b32_e32 v2, 1
	v_mov_b32_e32 v1, s3
	s_waitcnt vmcnt(0)
	buffer_wbinvl1_vol
	flat_store_dword v[0:1], v2 offset:12
	s_waitcnt vmcnt(0)
	flat_load_dword v0, v[0:1] offset:12 glc
	s_waitcnt vmcnt(0) lgkmcnt(0)
	v_cmp_ne_u32_e32 vcc, 2, v0
	s_and_saveexec_b64 s[2:3], vcc
	s_cbranch_execz .LBB0_4
; %bb.2:
	s_mov_b64 s[4:5], 0
.LBB0_3:                                ; =>This Inner Loop Header: Depth=1
	s_getpc_b64 s[6:7]
	s_add_u32 s6, s6, mailbox@gotpcrel32@lo+4
	s_addc_u32 s7, s7, mailbox@gotpcrel32@hi+12
	s_load_dwordx2 s[6:7], s[6:7], 0x0
	s_sleep 64
	s_waitcnt lgkmcnt(0)
	v_mov_b32_e32 v0, s6
	v_mov_b32_e32 v1, s7
	flat_load_dword v0, v[0:1] offset:12 glc
	s_waitcnt vmcnt(0) lgkmcnt(0)
	v_cmp_eq_u32_e32 vcc, 2, v0
	s_or_b64 s[4:5], vcc, s[4:5]
	s_andn2_b64 exec, exec, s[4:5]
	s_cbranch_execnz .LBB0_3
.LBB0_4:
	s_or_b64 exec, exec, s[2:3]
	s_getpc_b64 s[2:3]
	s_add_u32 s2, s2, mailbox@gotpcrel32@lo+4
	s_addc_u32 s3, s3, mailbox@gotpcrel32@hi+12
	s_load_dwordx2 s[2:3], s[2:3], 0x0
	v_mov_b32_e32 v0, 0
	v_mov_b32_e32 v1, 0x63
	v_mov_b32_e32 v2, 3
	s_waitcnt lgkmcnt(0)
	global_store_dword v0, v1, s[2:3] offset:8
	v_mov_b32_e32 v0, s2
	v_mov_b32_e32 v1, s3
	s_waitcnt vmcnt(0)
	buffer_wbinvl1_vol
	flat_store_dword v[0:1], v2 offset:12
	s_waitcnt vmcnt(0)
.LBB0_5:
	s_or_b64 exec, exec, s[0:1]
	; wave barrier
	s_endpgm
	.section	.rodata,"a",@progbits
	.p2align	6, 0x0
	.amdhsa_kernel probe_kernel
		.amdhsa_group_segment_fixed_size 0
		.amdhsa_private_segment_fixed_size 0
		.amdhsa_kernarg_size 0
		.amdhsa_user_sgpr_count 4
		.amdhsa_user_sgpr_private_segment_buffer 1
		.amdhsa_user_sgpr_dispatch_ptr 0
		.amdhsa_user_sgpr_queue_ptr 0
		.amdhsa_user_sgpr_kernarg_segment_ptr 0
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
		.amdhsa_next_free_vgpr 4
		.amdhsa_next_free_sgpr 8
		.amdhsa_reserve_vcc 1
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
.Lfunc_end0:
	.size	probe_kernel, .Lfunc_end0-probe_kernel
                                        ; -- End function
	.set probe_kernel.num_vgpr, 4
	.set probe_kernel.num_agpr, 0
	.set probe_kernel.numbered_sgpr, 8
	.set probe_kernel.private_seg_size, 0
	.set probe_kernel.uses_vcc, 1
	.set probe_kernel.uses_flat_scratch, 0
	.set probe_kernel.has_dyn_sized_stack, 0
	.set probe_kernel.has_recursion, 0
	.set probe_kernel.has_indirect_call, 0
	.section	.AMDGPU.csdata,"",@progbits
; Kernel info:
; codeLenInByte = 356
; TotalNumSgprs: 12
; NumVgprs: 4
; NumAgprs: 0
; TotalNumVgprs: 4
; ScratchSize: 0
; MemoryBound: 0
; FloatMode: 240
; IeeeMode: 1
; LDSByteSize: 0 bytes/workgroup (compile time only)
; SGPRBlocks: 1
; VGPRBlocks: 0
; NumSGPRsForWavesPerEU: 12
; NumVGPRsForWavesPerEU: 4
; Occupancy: 10
; WaveLimiterHint : 0
; COMPUTE_PGM_RSRC2:SCRATCH_EN: 0
; COMPUTE_PGM_RSRC2:USER_SGPR: 4
; COMPUTE_PGM_RSRC2:TRAP_HANDLER: 0
; COMPUTE_PGM_RSRC2:TGID_X_EN: 1
; COMPUTE_PGM_RSRC2:TGID_Y_EN: 0
; COMPUTE_PGM_RSRC2:TGID_Z_EN: 0
; COMPUTE_PGM_RSRC2:TIDIG_COMP_CNT: 0
	.section	.AMDGPU.gpr_maximums,"",@progbits
	.set amdgpu.max_num_vgpr, 0
	.set amdgpu.max_num_agpr, 0
	.set amdgpu.max_num_sgpr, 0
	.section	.AMDGPU.csdata,"",@progbits
	.type	__hip_cuid_f6ff1da173e07b19,@object ; @__hip_cuid_f6ff1da173e07b19
	.section	.bss,"aw",@nobits
	.globl	__hip_cuid_f6ff1da173e07b19
__hip_cuid_f6ff1da173e07b19:
	.byte	0                               ; 0x0
	.size	__hip_cuid_f6ff1da173e07b19, 1

	.ident	"AMD clang version 20.0.0git (https://github.com/RadeonOpenCompute/llvm-project roc-7.0.2 25385 0dda3adf56766e0aac0d03173ced3759e1ffecbc)"
	.section	".note.GNU-stack","",@progbits
	.addrsig
	.addrsig_sym mailbox
	.addrsig_sym __hip_cuid_f6ff1da173e07b19
	.amdgpu_metadata
---
amdhsa.kernels:
  - .agpr_count:     0
    .args:           []
    .group_segment_fixed_size: 0
    .kernarg_segment_align: 4
    .kernarg_segment_size: 0
    .language:       OpenCL C
    .language_version:
      - 2
      - 0
    .max_flat_workgroup_size: 1024
    .name:           probe_kernel
    .private_segment_fixed_size: 0
    .sgpr_count:     12
    .sgpr_spill_count: 0
    .symbol:         probe_kernel.kd
    .uniform_work_group_size: 1
    .uses_dynamic_stack: false
    .vgpr_count:     4
    .vgpr_spill_count: 0
    .wavefront_size: 64
amdhsa.target:   'amdgcn-amd-amdhsa--gfx908:xnack+'
amdhsa.version:
  - 1
  - 2
...

	.end_amdgpu_metadata
