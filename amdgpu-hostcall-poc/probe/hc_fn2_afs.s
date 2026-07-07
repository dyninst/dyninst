	.amdgcn_target "amdgcn-amd-amdhsa--gfx908"
	.amdhsa_code_object_version 6
	.text
	.p2align	2                               ; -- Begin function gpu_hostcall_staged
	.type	gpu_hostcall_staged,@function
gpu_hostcall_staged:                    ; @gpu_hostcall_staged
; %bb.0:
	s_waitcnt vmcnt(0) expcnt(0) lgkmcnt(0)
	v_accvgpr_write_b32 a4, v40             ;  Reload Reuse
	v_accvgpr_write_b32 a5, v41             ;  Reload Reuse
	v_accvgpr_write_b32 a6, v42             ;  Reload Reuse
	v_accvgpr_write_b32 a7, v43             ;  Reload Reuse
	v_accvgpr_write_b32 a8, v44             ;  Reload Reuse
	v_accvgpr_write_b32 a9, v45             ;  Reload Reuse
	v_accvgpr_write_b32 a10, v46            ;  Reload Reuse
	v_accvgpr_write_b32 a11, v47            ;  Reload Reuse
	v_accvgpr_write_b32 a12, v56            ;  Reload Reuse
	v_accvgpr_write_b32 a13, v57            ;  Reload Reuse
	v_accvgpr_write_b32 a14, v58            ;  Reload Reuse
	v_accvgpr_write_b32 a15, v59            ;  Reload Reuse
	v_accvgpr_write_b32 a16, v60            ;  Reload Reuse
	v_accvgpr_write_b32 a17, v61            ;  Reload Reuse
	v_accvgpr_write_b32 a18, v62            ;  Reload Reuse
	v_and_b32_e32 v3, 63, v31
	v_cmp_eq_u32_e32 vcc, 0, v3
	s_and_saveexec_b64 s[0:1], vcc
	s_cbranch_execz .LBB0_260
; %bb.1:
	v_mov_b32_e32 v3, 0
	v_cmp_ne_u64_e32 vcc, 0, v[1:2]
	s_nop 0
	v_accvgpr_write_b32 a19, v3             ;  Reload Reuse
	v_mov_b32_e32 v3, 0
	s_nop 1
	v_accvgpr_write_b32 a20, v3             ;  Reload Reuse
	s_and_saveexec_b64 s[2:3], vcc
	s_cbranch_execz .LBB0_3
; %bb.2:
	flat_load_ubyte v3, v[1:2]
	s_waitcnt vmcnt(0) lgkmcnt(0)
	v_accvgpr_write_b32 a20, v3             ;  Reload Reuse
.LBB0_3:
	s_or_b64 exec, exec, s[2:3]
	s_and_saveexec_b64 s[2:3], vcc
	s_cbranch_execz .LBB0_5
; %bb.4:
	flat_load_ubyte v3, v[1:2] offset:1
	s_waitcnt vmcnt(0) lgkmcnt(0)
	v_accvgpr_write_b32 a19, v3             ;  Reload Reuse
.LBB0_5:
	s_or_b64 exec, exec, s[2:3]
	v_mov_b32_e32 v3, 0
	s_nop 1
	v_accvgpr_write_b32 a21, v3             ;  Reload Reuse
	v_mov_b32_e32 v3, 0
	s_nop 1
	v_accvgpr_write_b32 a22, v3             ;  Reload Reuse
	s_and_saveexec_b64 s[2:3], vcc
	s_cbranch_execz .LBB0_7
; %bb.6:
	flat_load_ubyte v3, v[1:2] offset:2
	s_waitcnt vmcnt(0) lgkmcnt(0)
	v_accvgpr_write_b32 a22, v3             ;  Reload Reuse
.LBB0_7:
	s_or_b64 exec, exec, s[2:3]
	s_and_saveexec_b64 s[2:3], vcc
	s_cbranch_execz .LBB0_9
; %bb.8:
	flat_load_ubyte v3, v[1:2] offset:3
	s_waitcnt vmcnt(0) lgkmcnt(0)
	v_accvgpr_write_b32 a21, v3             ;  Reload Reuse
.LBB0_9:
	s_or_b64 exec, exec, s[2:3]
	v_mov_b32_e32 v3, 0
	v_mov_b32_e32 v13, 0
	s_nop 0
	v_accvgpr_write_b32 a23, v3             ;  Reload Reuse
	s_and_saveexec_b64 s[2:3], vcc
	s_cbranch_execz .LBB0_11
; %bb.10:
	flat_load_ubyte v13, v[1:2] offset:4
.LBB0_11:
	s_or_b64 exec, exec, s[2:3]
	s_and_saveexec_b64 s[2:3], vcc
	s_cbranch_execz .LBB0_13
; %bb.12:
	flat_load_ubyte v3, v[1:2] offset:5
	s_waitcnt vmcnt(0) lgkmcnt(0)
	v_accvgpr_write_b32 a23, v3             ;  Reload Reuse
.LBB0_13:
	s_or_b64 exec, exec, s[2:3]
	v_mov_b32_e32 v3, 0
	s_nop 1
	v_accvgpr_write_b32 a24, v3             ;  Reload Reuse
	v_mov_b32_e32 v3, 0
	s_nop 1
	v_accvgpr_write_b32 a25, v3             ;  Reload Reuse
	s_and_saveexec_b64 s[2:3], vcc
	s_cbranch_execz .LBB0_15
; %bb.14:
	flat_load_ubyte v3, v[1:2] offset:6
	s_waitcnt vmcnt(0) lgkmcnt(0)
	v_accvgpr_write_b32 a25, v3             ;  Reload Reuse
.LBB0_15:
	s_or_b64 exec, exec, s[2:3]
	s_and_saveexec_b64 s[2:3], vcc
	s_cbranch_execz .LBB0_17
; %bb.16:
	flat_load_ubyte v3, v[1:2] offset:7
	s_waitcnt vmcnt(0) lgkmcnt(0)
	v_accvgpr_write_b32 a24, v3             ;  Reload Reuse
.LBB0_17:
	s_or_b64 exec, exec, s[2:3]
	v_mov_b32_e32 v3, 0
	s_nop 1
	v_accvgpr_write_b32 a26, v3             ;  Reload Reuse
	v_mov_b32_e32 v3, 0
	s_nop 1
	v_accvgpr_write_b32 a27, v3             ;  Reload Reuse
	s_and_saveexec_b64 s[2:3], vcc
	s_cbranch_execz .LBB0_19
; %bb.18:
	flat_load_ubyte v3, v[1:2] offset:8
	s_waitcnt vmcnt(0) lgkmcnt(0)
	v_accvgpr_write_b32 a27, v3             ;  Reload Reuse
.LBB0_19:
	s_or_b64 exec, exec, s[2:3]
	s_and_saveexec_b64 s[2:3], vcc
	s_cbranch_execz .LBB0_21
; %bb.20:
	flat_load_ubyte v3, v[1:2] offset:9
	s_waitcnt vmcnt(0) lgkmcnt(0)
	v_accvgpr_write_b32 a26, v3             ;  Reload Reuse
.LBB0_21:
	s_or_b64 exec, exec, s[2:3]
	v_mov_b32_e32 v3, 0
	s_nop 1
	v_accvgpr_write_b32 a28, v3             ;  Reload Reuse
	v_mov_b32_e32 v3, 0
	s_nop 1
	v_accvgpr_write_b32 a29, v3             ;  Reload Reuse
	s_and_saveexec_b64 s[2:3], vcc
	s_cbranch_execz .LBB0_23
; %bb.22:
	flat_load_ubyte v3, v[1:2] offset:10
	s_waitcnt vmcnt(0) lgkmcnt(0)
	v_accvgpr_write_b32 a29, v3             ;  Reload Reuse
.LBB0_23:
	s_or_b64 exec, exec, s[2:3]
	s_and_saveexec_b64 s[2:3], vcc
	s_cbranch_execz .LBB0_25
; %bb.24:
	flat_load_ubyte v3, v[1:2] offset:11
	s_waitcnt vmcnt(0) lgkmcnt(0)
	v_accvgpr_write_b32 a28, v3             ;  Reload Reuse
.LBB0_25:
	s_or_b64 exec, exec, s[2:3]
	v_mov_b32_e32 v3, 0
	s_nop 1
	v_accvgpr_write_b32 a30, v3             ;  Reload Reuse
	v_mov_b32_e32 v3, 0
	s_nop 1
	v_accvgpr_write_b32 a31, v3             ;  Reload Reuse
	s_and_saveexec_b64 s[2:3], vcc
	s_cbranch_execz .LBB0_27
; %bb.26:
	flat_load_ubyte v3, v[1:2] offset:12
	s_waitcnt vmcnt(0) lgkmcnt(0)
	v_accvgpr_write_b32 a31, v3             ;  Reload Reuse
.LBB0_27:
	s_or_b64 exec, exec, s[2:3]
	s_and_saveexec_b64 s[2:3], vcc
	s_cbranch_execz .LBB0_29
; %bb.28:
	flat_load_ubyte v3, v[1:2] offset:13
	s_waitcnt vmcnt(0) lgkmcnt(0)
	v_accvgpr_write_b32 a30, v3             ;  Reload Reuse
.LBB0_29:
	s_or_b64 exec, exec, s[2:3]
	v_mov_b32_e32 v3, 0
	s_nop 1
	v_accvgpr_write_b32 a32, v3             ;  Reload Reuse
	v_mov_b32_e32 v3, 0
	s_nop 1
	v_accvgpr_write_b32 a33, v3             ;  Reload Reuse
	s_and_saveexec_b64 s[2:3], vcc
	s_cbranch_execz .LBB0_31
; %bb.30:
	flat_load_ubyte v3, v[1:2] offset:14
	s_waitcnt vmcnt(0) lgkmcnt(0)
	v_accvgpr_write_b32 a33, v3             ;  Reload Reuse
.LBB0_31:
	s_or_b64 exec, exec, s[2:3]
	s_and_saveexec_b64 s[2:3], vcc
	s_cbranch_execz .LBB0_33
; %bb.32:
	flat_load_ubyte v3, v[1:2] offset:15
	s_waitcnt vmcnt(0) lgkmcnt(0)
	v_accvgpr_write_b32 a32, v3             ;  Reload Reuse
.LBB0_33:
	s_or_b64 exec, exec, s[2:3]
	v_mov_b32_e32 v3, 0
	s_nop 1
	v_accvgpr_write_b32 a34, v3             ;  Reload Reuse
	v_mov_b32_e32 v3, 0
	s_nop 1
	v_accvgpr_write_b32 a35, v3             ;  Reload Reuse
	s_and_saveexec_b64 s[2:3], vcc
	s_cbranch_execz .LBB0_35
; %bb.34:
	flat_load_ubyte v3, v[1:2] offset:16
	s_waitcnt vmcnt(0) lgkmcnt(0)
	v_accvgpr_write_b32 a35, v3             ;  Reload Reuse
.LBB0_35:
	s_or_b64 exec, exec, s[2:3]
	s_and_saveexec_b64 s[2:3], vcc
	s_cbranch_execz .LBB0_37
; %bb.36:
	flat_load_ubyte v3, v[1:2] offset:17
	s_waitcnt vmcnt(0) lgkmcnt(0)
	v_accvgpr_write_b32 a34, v3             ;  Reload Reuse
.LBB0_37:
	s_or_b64 exec, exec, s[2:3]
	v_mov_b32_e32 v3, 0
	s_nop 1
	v_accvgpr_write_b32 a36, v3             ;  Reload Reuse
	v_mov_b32_e32 v3, 0
	s_nop 1
	v_accvgpr_write_b32 a37, v3             ;  Reload Reuse
	s_and_saveexec_b64 s[2:3], vcc
	s_cbranch_execz .LBB0_39
; %bb.38:
	flat_load_ubyte v3, v[1:2] offset:18
	s_waitcnt vmcnt(0) lgkmcnt(0)
	v_accvgpr_write_b32 a37, v3             ;  Reload Reuse
.LBB0_39:
	s_or_b64 exec, exec, s[2:3]
	s_and_saveexec_b64 s[2:3], vcc
	s_cbranch_execz .LBB0_41
; %bb.40:
	flat_load_ubyte v3, v[1:2] offset:19
	s_waitcnt vmcnt(0) lgkmcnt(0)
	v_accvgpr_write_b32 a36, v3             ;  Reload Reuse
.LBB0_41:
	s_or_b64 exec, exec, s[2:3]
	v_mov_b32_e32 v3, 0
	s_nop 1
	v_accvgpr_write_b32 a38, v3             ;  Reload Reuse
	v_mov_b32_e32 v3, 0
	s_nop 1
	v_accvgpr_write_b32 a39, v3             ;  Reload Reuse
	s_and_saveexec_b64 s[2:3], vcc
	s_cbranch_execz .LBB0_43
; %bb.42:
	flat_load_ubyte v3, v[1:2] offset:20
	s_waitcnt vmcnt(0) lgkmcnt(0)
	v_accvgpr_write_b32 a39, v3             ;  Reload Reuse
.LBB0_43:
	s_or_b64 exec, exec, s[2:3]
	s_and_saveexec_b64 s[2:3], vcc
	s_cbranch_execz .LBB0_45
; %bb.44:
	flat_load_ubyte v3, v[1:2] offset:21
	s_waitcnt vmcnt(0) lgkmcnt(0)
	v_accvgpr_write_b32 a38, v3             ;  Reload Reuse
.LBB0_45:
	s_or_b64 exec, exec, s[2:3]
	v_mov_b32_e32 v3, 0
	s_nop 1
	v_accvgpr_write_b32 a40, v3             ;  Reload Reuse
	v_mov_b32_e32 v3, 0
	s_nop 1
	v_accvgpr_write_b32 a41, v3             ;  Reload Reuse
	s_and_saveexec_b64 s[2:3], vcc
	s_cbranch_execz .LBB0_47
; %bb.46:
	flat_load_ubyte v3, v[1:2] offset:22
	s_waitcnt vmcnt(0) lgkmcnt(0)
	v_accvgpr_write_b32 a41, v3             ;  Reload Reuse
.LBB0_47:
	s_or_b64 exec, exec, s[2:3]
	s_and_saveexec_b64 s[2:3], vcc
	s_cbranch_execz .LBB0_49
; %bb.48:
	flat_load_ubyte v3, v[1:2] offset:23
	s_waitcnt vmcnt(0) lgkmcnt(0)
	v_accvgpr_write_b32 a40, v3             ;  Reload Reuse
.LBB0_49:
	s_or_b64 exec, exec, s[2:3]
	v_mov_b32_e32 v3, 0
	s_nop 1
	v_accvgpr_write_b32 a42, v3             ;  Reload Reuse
	v_mov_b32_e32 v3, 0
	s_nop 1
	v_accvgpr_write_b32 a43, v3             ;  Reload Reuse
	s_and_saveexec_b64 s[2:3], vcc
	s_cbranch_execz .LBB0_51
; %bb.50:
	flat_load_ubyte v3, v[1:2] offset:24
	s_waitcnt vmcnt(0) lgkmcnt(0)
	v_accvgpr_write_b32 a43, v3             ;  Reload Reuse
.LBB0_51:
	s_or_b64 exec, exec, s[2:3]
	s_and_saveexec_b64 s[2:3], vcc
	s_cbranch_execz .LBB0_53
; %bb.52:
	flat_load_ubyte v3, v[1:2] offset:25
	s_waitcnt vmcnt(0) lgkmcnt(0)
	v_accvgpr_write_b32 a42, v3             ;  Reload Reuse
.LBB0_53:
	s_or_b64 exec, exec, s[2:3]
	v_mov_b32_e32 v3, 0
	s_nop 1
	v_accvgpr_write_b32 a44, v3             ;  Reload Reuse
	v_mov_b32_e32 v3, 0
	s_nop 1
	v_accvgpr_write_b32 a45, v3             ;  Reload Reuse
	s_and_saveexec_b64 s[2:3], vcc
	s_cbranch_execz .LBB0_55
; %bb.54:
	flat_load_ubyte v3, v[1:2] offset:26
	s_waitcnt vmcnt(0) lgkmcnt(0)
	v_accvgpr_write_b32 a45, v3             ;  Reload Reuse
.LBB0_55:
	s_or_b64 exec, exec, s[2:3]
	s_and_saveexec_b64 s[2:3], vcc
	s_cbranch_execz .LBB0_57
; %bb.56:
	flat_load_ubyte v3, v[1:2] offset:27
	s_waitcnt vmcnt(0) lgkmcnt(0)
	v_accvgpr_write_b32 a44, v3             ;  Reload Reuse
.LBB0_57:
	s_or_b64 exec, exec, s[2:3]
	v_mov_b32_e32 v3, 0
	s_nop 1
	v_accvgpr_write_b32 a46, v3             ;  Reload Reuse
	v_mov_b32_e32 v3, 0
	s_nop 1
	v_accvgpr_write_b32 a47, v3             ;  Reload Reuse
	s_and_saveexec_b64 s[2:3], vcc
	s_cbranch_execz .LBB0_59
; %bb.58:
	flat_load_ubyte v3, v[1:2] offset:28
	s_waitcnt vmcnt(0) lgkmcnt(0)
	v_accvgpr_write_b32 a47, v3             ;  Reload Reuse
.LBB0_59:
	s_or_b64 exec, exec, s[2:3]
	s_and_saveexec_b64 s[2:3], vcc
	s_cbranch_execz .LBB0_61
; %bb.60:
	flat_load_ubyte v3, v[1:2] offset:29
	s_waitcnt vmcnt(0) lgkmcnt(0)
	v_accvgpr_write_b32 a46, v3             ;  Reload Reuse
.LBB0_61:
	s_or_b64 exec, exec, s[2:3]
	v_mov_b32_e32 v3, 0
	s_nop 1
	v_accvgpr_write_b32 a48, v3             ;  Reload Reuse
	v_mov_b32_e32 v3, 0
	s_nop 1
	v_accvgpr_write_b32 a49, v3             ;  Reload Reuse
	s_and_saveexec_b64 s[2:3], vcc
	s_cbranch_execz .LBB0_63
; %bb.62:
	flat_load_ubyte v3, v[1:2] offset:30
	s_waitcnt vmcnt(0) lgkmcnt(0)
	v_accvgpr_write_b32 a49, v3             ;  Reload Reuse
.LBB0_63:
	s_or_b64 exec, exec, s[2:3]
	s_and_saveexec_b64 s[2:3], vcc
	s_cbranch_execz .LBB0_65
; %bb.64:
	flat_load_ubyte v3, v[1:2] offset:31
	s_waitcnt vmcnt(0) lgkmcnt(0)
	v_accvgpr_write_b32 a48, v3             ;  Reload Reuse
.LBB0_65:
	s_or_b64 exec, exec, s[2:3]
	v_mov_b32_e32 v3, 0
	s_nop 1
	v_accvgpr_write_b32 a50, v3             ;  Reload Reuse
	v_mov_b32_e32 v3, 0
	s_nop 1
	v_accvgpr_write_b32 a51, v3             ;  Reload Reuse
	s_and_saveexec_b64 s[2:3], vcc
	s_cbranch_execz .LBB0_67
; %bb.66:
	flat_load_ubyte v3, v[1:2] offset:32
	s_waitcnt vmcnt(0) lgkmcnt(0)
	v_accvgpr_write_b32 a51, v3             ;  Reload Reuse
.LBB0_67:
	s_or_b64 exec, exec, s[2:3]
	s_and_saveexec_b64 s[2:3], vcc
	s_cbranch_execz .LBB0_69
; %bb.68:
	flat_load_ubyte v3, v[1:2] offset:33
	s_waitcnt vmcnt(0) lgkmcnt(0)
	v_accvgpr_write_b32 a50, v3             ;  Reload Reuse
.LBB0_69:
	s_or_b64 exec, exec, s[2:3]
	v_mov_b32_e32 v3, 0
	s_nop 1
	v_accvgpr_write_b32 a52, v3             ;  Reload Reuse
	v_mov_b32_e32 v3, 0
	s_nop 1
	v_accvgpr_write_b32 a53, v3             ;  Reload Reuse
	s_and_saveexec_b64 s[2:3], vcc
	s_cbranch_execz .LBB0_71
; %bb.70:
	flat_load_ubyte v3, v[1:2] offset:34
	s_waitcnt vmcnt(0) lgkmcnt(0)
	v_accvgpr_write_b32 a53, v3             ;  Reload Reuse
.LBB0_71:
	s_or_b64 exec, exec, s[2:3]
	s_and_saveexec_b64 s[2:3], vcc
	s_cbranch_execz .LBB0_73
; %bb.72:
	flat_load_ubyte v3, v[1:2] offset:35
	s_waitcnt vmcnt(0) lgkmcnt(0)
	v_accvgpr_write_b32 a52, v3             ;  Reload Reuse
.LBB0_73:
	s_or_b64 exec, exec, s[2:3]
	v_mov_b32_e32 v3, 0
	s_nop 1
	v_accvgpr_write_b32 a54, v3             ;  Reload Reuse
	v_mov_b32_e32 v3, 0
	s_nop 1
	v_accvgpr_write_b32 a55, v3             ;  Reload Reuse
	s_and_saveexec_b64 s[2:3], vcc
	s_cbranch_execz .LBB0_75
; %bb.74:
	flat_load_ubyte v3, v[1:2] offset:36
	s_waitcnt vmcnt(0) lgkmcnt(0)
	v_accvgpr_write_b32 a55, v3             ;  Reload Reuse
.LBB0_75:
	s_or_b64 exec, exec, s[2:3]
	s_and_saveexec_b64 s[2:3], vcc
	s_cbranch_execz .LBB0_77
; %bb.76:
	flat_load_ubyte v3, v[1:2] offset:37
	s_waitcnt vmcnt(0) lgkmcnt(0)
	v_accvgpr_write_b32 a54, v3             ;  Reload Reuse
.LBB0_77:
	s_or_b64 exec, exec, s[2:3]
	v_mov_b32_e32 v3, 0
	s_nop 1
	v_accvgpr_write_b32 a56, v3             ;  Reload Reuse
	v_mov_b32_e32 v3, 0
	s_nop 1
	v_accvgpr_write_b32 a57, v3             ;  Reload Reuse
	s_and_saveexec_b64 s[2:3], vcc
	s_cbranch_execz .LBB0_79
; %bb.78:
	flat_load_ubyte v3, v[1:2] offset:38
	s_waitcnt vmcnt(0) lgkmcnt(0)
	v_accvgpr_write_b32 a57, v3             ;  Reload Reuse
.LBB0_79:
	s_or_b64 exec, exec, s[2:3]
	s_and_saveexec_b64 s[2:3], vcc
	s_cbranch_execz .LBB0_81
; %bb.80:
	flat_load_ubyte v3, v[1:2] offset:39
	s_waitcnt vmcnt(0) lgkmcnt(0)
	v_accvgpr_write_b32 a56, v3             ;  Reload Reuse
.LBB0_81:
	s_or_b64 exec, exec, s[2:3]
	v_mov_b32_e32 v3, 0
	s_nop 1
	v_accvgpr_write_b32 a58, v3             ;  Reload Reuse
	v_mov_b32_e32 v3, 0
	s_nop 1
	v_accvgpr_write_b32 a59, v3             ;  Reload Reuse
	s_and_saveexec_b64 s[2:3], vcc
	s_cbranch_execz .LBB0_83
; %bb.82:
	flat_load_ubyte v3, v[1:2] offset:40
	s_waitcnt vmcnt(0) lgkmcnt(0)
	v_accvgpr_write_b32 a59, v3             ;  Reload Reuse
.LBB0_83:
	s_or_b64 exec, exec, s[2:3]
	s_and_saveexec_b64 s[2:3], vcc
	s_cbranch_execz .LBB0_85
; %bb.84:
	flat_load_ubyte v3, v[1:2] offset:41
	s_waitcnt vmcnt(0) lgkmcnt(0)
	v_accvgpr_write_b32 a58, v3             ;  Reload Reuse
.LBB0_85:
	s_or_b64 exec, exec, s[2:3]
	v_mov_b32_e32 v3, 0
	s_nop 1
	v_accvgpr_write_b32 a60, v3             ;  Reload Reuse
	v_mov_b32_e32 v3, 0
	s_nop 1
	v_accvgpr_write_b32 a61, v3             ;  Reload Reuse
	s_and_saveexec_b64 s[2:3], vcc
	s_cbranch_execz .LBB0_87
; %bb.86:
	flat_load_ubyte v3, v[1:2] offset:42
	s_waitcnt vmcnt(0) lgkmcnt(0)
	v_accvgpr_write_b32 a61, v3             ;  Reload Reuse
.LBB0_87:
	s_or_b64 exec, exec, s[2:3]
	s_and_saveexec_b64 s[2:3], vcc
	s_cbranch_execz .LBB0_89
; %bb.88:
	flat_load_ubyte v3, v[1:2] offset:43
	s_waitcnt vmcnt(0) lgkmcnt(0)
	v_accvgpr_write_b32 a60, v3             ;  Reload Reuse
.LBB0_89:
	s_or_b64 exec, exec, s[2:3]
	v_mov_b32_e32 v3, 0
	s_nop 1
	v_accvgpr_write_b32 a62, v3             ;  Reload Reuse
	v_mov_b32_e32 v3, 0
	s_nop 1
	v_accvgpr_write_b32 a63, v3             ;  Reload Reuse
	s_and_saveexec_b64 s[2:3], vcc
	s_cbranch_execz .LBB0_91
; %bb.90:
	flat_load_ubyte v3, v[1:2] offset:44
	s_waitcnt vmcnt(0) lgkmcnt(0)
	v_accvgpr_write_b32 a63, v3             ;  Reload Reuse
.LBB0_91:
	s_or_b64 exec, exec, s[2:3]
	s_and_saveexec_b64 s[2:3], vcc
	s_cbranch_execz .LBB0_93
; %bb.92:
	flat_load_ubyte v3, v[1:2] offset:45
	s_waitcnt vmcnt(0) lgkmcnt(0)
	v_accvgpr_write_b32 a62, v3             ;  Reload Reuse
.LBB0_93:
	s_or_b64 exec, exec, s[2:3]
	v_mov_b32_e32 v3, 0
	scratch_store_dword off, v3, s32 offset:24 ; 4-byte Folded Spill
	v_mov_b32_e32 v3, 0
	scratch_store_dword off, v3, s32        ; 4-byte Folded Spill
	s_and_saveexec_b64 s[2:3], vcc
	s_cbranch_execz .LBB0_95
; %bb.94:
	flat_load_ubyte v3, v[1:2] offset:46
	s_waitcnt vmcnt(0) lgkmcnt(0)
	scratch_store_dword off, v3, s32        ; 4-byte Folded Spill
.LBB0_95:
	s_or_b64 exec, exec, s[2:3]
	s_and_saveexec_b64 s[2:3], vcc
	s_cbranch_execz .LBB0_97
; %bb.96:
	flat_load_ubyte v3, v[1:2] offset:47
	s_waitcnt vmcnt(0) lgkmcnt(0)
	scratch_store_dword off, v3, s32 offset:24 ; 4-byte Folded Spill
.LBB0_97:
	s_or_b64 exec, exec, s[2:3]
	v_mov_b32_e32 v3, 0
	scratch_store_dword off, v3, s32 offset:36 ; 4-byte Folded Spill
	v_mov_b32_e32 v3, 0
	scratch_store_dword off, v3, s32 offset:8 ; 4-byte Folded Spill
	s_and_saveexec_b64 s[2:3], vcc
	s_cbranch_execz .LBB0_99
; %bb.98:
	flat_load_ubyte v3, v[1:2] offset:48
	s_waitcnt vmcnt(0) lgkmcnt(0)
	scratch_store_dword off, v3, s32 offset:8 ; 4-byte Folded Spill
.LBB0_99:
	s_or_b64 exec, exec, s[2:3]
	s_and_saveexec_b64 s[2:3], vcc
	s_cbranch_execz .LBB0_101
; %bb.100:
	flat_load_ubyte v3, v[1:2] offset:49
	s_waitcnt vmcnt(0) lgkmcnt(0)
	scratch_store_dword off, v3, s32 offset:36 ; 4-byte Folded Spill
.LBB0_101:
	s_or_b64 exec, exec, s[2:3]
	v_mov_b32_e32 v3, 0
	scratch_store_dword off, v3, s32 offset:4 ; 4-byte Folded Spill
	v_mov_b32_e32 v3, 0
	scratch_store_dword off, v3, s32 offset:28 ; 4-byte Folded Spill
	s_and_saveexec_b64 s[2:3], vcc
	s_cbranch_execz .LBB0_103
; %bb.102:
	flat_load_ubyte v3, v[1:2] offset:50
	s_waitcnt vmcnt(0) lgkmcnt(0)
	scratch_store_dword off, v3, s32 offset:28 ; 4-byte Folded Spill
.LBB0_103:
	s_or_b64 exec, exec, s[2:3]
	s_and_saveexec_b64 s[2:3], vcc
	s_cbranch_execz .LBB0_105
; %bb.104:
	flat_load_ubyte v3, v[1:2] offset:51
	s_waitcnt vmcnt(0) lgkmcnt(0)
	scratch_store_dword off, v3, s32 offset:4 ; 4-byte Folded Spill
.LBB0_105:
	s_or_b64 exec, exec, s[2:3]
	v_mov_b32_e32 v3, 0
	scratch_store_dword off, v3, s32 offset:12 ; 4-byte Folded Spill
	v_mov_b32_e32 v3, 0
	scratch_store_dword off, v3, s32 offset:40 ; 4-byte Folded Spill
	s_and_saveexec_b64 s[2:3], vcc
	s_cbranch_execz .LBB0_107
; %bb.106:
	flat_load_ubyte v3, v[1:2] offset:52
	s_waitcnt vmcnt(0) lgkmcnt(0)
	scratch_store_dword off, v3, s32 offset:40 ; 4-byte Folded Spill
.LBB0_107:
	s_or_b64 exec, exec, s[2:3]
	s_and_saveexec_b64 s[2:3], vcc
	s_cbranch_execz .LBB0_109
; %bb.108:
	flat_load_ubyte v3, v[1:2] offset:53
	s_waitcnt vmcnt(0) lgkmcnt(0)
	scratch_store_dword off, v3, s32 offset:12 ; 4-byte Folded Spill
.LBB0_109:
	s_or_b64 exec, exec, s[2:3]
	v_mov_b32_e32 v3, 0
	scratch_store_dword off, v3, s32 offset:16 ; 4-byte Folded Spill
	v_mov_b32_e32 v3, 0
	scratch_store_dword off, v3, s32 offset:56 ; 4-byte Folded Spill
	s_and_saveexec_b64 s[2:3], vcc
	s_cbranch_execz .LBB0_111
; %bb.110:
	flat_load_ubyte v3, v[1:2] offset:54
	s_waitcnt vmcnt(0) lgkmcnt(0)
	scratch_store_dword off, v3, s32 offset:56 ; 4-byte Folded Spill
.LBB0_111:
	s_or_b64 exec, exec, s[2:3]
	s_and_saveexec_b64 s[2:3], vcc
	s_cbranch_execz .LBB0_113
; %bb.112:
	flat_load_ubyte v3, v[1:2] offset:55
	s_waitcnt vmcnt(0) lgkmcnt(0)
	scratch_store_dword off, v3, s32 offset:16 ; 4-byte Folded Spill
.LBB0_113:
	s_or_b64 exec, exec, s[2:3]
	v_mov_b32_e32 v3, 0
	v_mov_b32_e32 v11, 0
	scratch_store_dword off, v3, s32 offset:48 ; 4-byte Folded Spill
	s_and_saveexec_b64 s[2:3], vcc
	s_cbranch_execz .LBB0_115
; %bb.114:
	flat_load_ubyte v11, v[1:2] offset:56
.LBB0_115:
	s_or_b64 exec, exec, s[2:3]
	s_and_saveexec_b64 s[2:3], vcc
	s_cbranch_execz .LBB0_117
; %bb.116:
	flat_load_ubyte v3, v[1:2] offset:57
	s_waitcnt vmcnt(0) lgkmcnt(0)
	scratch_store_dword off, v3, s32 offset:48 ; 4-byte Folded Spill
.LBB0_117:
	s_or_b64 exec, exec, s[2:3]
	v_mov_b32_e32 v3, 0
	scratch_store_dword off, v3, s32 offset:64 ; 4-byte Folded Spill
	v_mov_b32_e32 v3, 0
	scratch_store_dword off, v3, s32 offset:20 ; 4-byte Folded Spill
	s_and_saveexec_b64 s[2:3], vcc
	s_cbranch_execz .LBB0_119
; %bb.118:
	flat_load_ubyte v3, v[1:2] offset:58
	s_waitcnt vmcnt(0) lgkmcnt(0)
	scratch_store_dword off, v3, s32 offset:20 ; 4-byte Folded Spill
.LBB0_119:
	s_or_b64 exec, exec, s[2:3]
	s_and_saveexec_b64 s[2:3], vcc
	s_cbranch_execz .LBB0_121
; %bb.120:
	flat_load_ubyte v3, v[1:2] offset:59
	s_waitcnt vmcnt(0) lgkmcnt(0)
	scratch_store_dword off, v3, s32 offset:64 ; 4-byte Folded Spill
.LBB0_121:
	s_or_b64 exec, exec, s[2:3]
	v_mov_b32_e32 v3, 0
	scratch_store_dword off, v3, s32 offset:80 ; 4-byte Folded Spill
	v_mov_b32_e32 v3, 0
	scratch_store_dword off, v3, s32 offset:32 ; 4-byte Folded Spill
	s_and_saveexec_b64 s[2:3], vcc
	s_cbranch_execz .LBB0_123
; %bb.122:
	flat_load_ubyte v3, v[1:2] offset:60
	s_waitcnt vmcnt(0) lgkmcnt(0)
	scratch_store_dword off, v3, s32 offset:32 ; 4-byte Folded Spill
.LBB0_123:
	s_or_b64 exec, exec, s[2:3]
	s_and_saveexec_b64 s[2:3], vcc
	s_cbranch_execz .LBB0_125
; %bb.124:
	flat_load_ubyte v3, v[1:2] offset:61
	s_waitcnt vmcnt(0) lgkmcnt(0)
	scratch_store_dword off, v3, s32 offset:80 ; 4-byte Folded Spill
.LBB0_125:
	s_or_b64 exec, exec, s[2:3]
	v_mov_b32_e32 v10, 0
	v_mov_b32_e32 v3, 0
	scratch_store_dword off, v3, s32 offset:52 ; 4-byte Folded Spill
	s_and_saveexec_b64 s[2:3], vcc
	s_cbranch_execz .LBB0_127
; %bb.126:
	flat_load_ubyte v3, v[1:2] offset:62
	s_waitcnt vmcnt(0) lgkmcnt(0)
	scratch_store_dword off, v3, s32 offset:52 ; 4-byte Folded Spill
.LBB0_127:
	s_or_b64 exec, exec, s[2:3]
	s_and_saveexec_b64 s[2:3], vcc
	s_cbranch_execz .LBB0_129
; %bb.128:
	flat_load_ubyte v10, v[1:2] offset:63
.LBB0_129:
	s_or_b64 exec, exec, s[2:3]
	v_mov_b32_e32 v3, 0
	scratch_store_dword off, v3, s32 offset:44 ; 4-byte Folded Spill
	v_mov_b32_e32 v3, 0
	scratch_store_dword off, v3, s32 offset:68 ; 4-byte Folded Spill
	s_and_saveexec_b64 s[2:3], vcc
	s_cbranch_execz .LBB0_131
; %bb.130:
	flat_load_ubyte v3, v[1:2] offset:64
	s_waitcnt vmcnt(0) lgkmcnt(0)
	scratch_store_dword off, v3, s32 offset:68 ; 4-byte Folded Spill
.LBB0_131:
	s_or_b64 exec, exec, s[2:3]
	s_and_saveexec_b64 s[2:3], vcc
	s_cbranch_execz .LBB0_133
; %bb.132:
	flat_load_ubyte v3, v[1:2] offset:65
	s_waitcnt vmcnt(0) lgkmcnt(0)
	scratch_store_dword off, v3, s32 offset:44 ; 4-byte Folded Spill
.LBB0_133:
	s_or_b64 exec, exec, s[2:3]
	v_mov_b32_e32 v3, 0
	v_mov_b32_e32 v14, 0
	scratch_store_dword off, v3, s32 offset:60 ; 4-byte Folded Spill
	s_and_saveexec_b64 s[2:3], vcc
	s_cbranch_execz .LBB0_135
; %bb.134:
	flat_load_ubyte v14, v[1:2] offset:66
.LBB0_135:
	s_or_b64 exec, exec, s[2:3]
	s_and_saveexec_b64 s[2:3], vcc
	s_cbranch_execz .LBB0_137
; %bb.136:
	flat_load_ubyte v3, v[1:2] offset:67
	s_waitcnt vmcnt(0) lgkmcnt(0)
	scratch_store_dword off, v3, s32 offset:60 ; 4-byte Folded Spill
.LBB0_137:
	s_or_b64 exec, exec, s[2:3]
	v_mov_b32_e32 v3, 0
	v_mov_b32_e32 v17, 0
	scratch_store_dword off, v3, s32 offset:72 ; 4-byte Folded Spill
	s_and_saveexec_b64 s[2:3], vcc
	s_cbranch_execz .LBB0_139
; %bb.138:
	flat_load_ubyte v17, v[1:2] offset:68
.LBB0_139:
	s_or_b64 exec, exec, s[2:3]
	s_and_saveexec_b64 s[2:3], vcc
	s_cbranch_execz .LBB0_141
; %bb.140:
	flat_load_ubyte v3, v[1:2] offset:69
	s_waitcnt vmcnt(0) lgkmcnt(0)
	scratch_store_dword off, v3, s32 offset:72 ; 4-byte Folded Spill
.LBB0_141:
	s_or_b64 exec, exec, s[2:3]
	v_mov_b32_e32 v3, 0
	v_mov_b32_e32 v12, 0
	scratch_store_dword off, v3, s32 offset:84 ; 4-byte Folded Spill
	s_and_saveexec_b64 s[2:3], vcc
	s_cbranch_execz .LBB0_143
; %bb.142:
	flat_load_ubyte v12, v[1:2] offset:70
.LBB0_143:
	s_or_b64 exec, exec, s[2:3]
	s_and_saveexec_b64 s[2:3], vcc
	s_cbranch_execz .LBB0_145
; %bb.144:
	flat_load_ubyte v3, v[1:2] offset:71
	s_waitcnt vmcnt(0) lgkmcnt(0)
	scratch_store_dword off, v3, s32 offset:84 ; 4-byte Folded Spill
.LBB0_145:
	s_or_b64 exec, exec, s[2:3]
	v_mov_b32_e32 v19, 0
	v_mov_b32_e32 v3, 0
	scratch_store_dword off, v3, s32 offset:76 ; 4-byte Folded Spill
	s_and_saveexec_b64 s[2:3], vcc
	s_cbranch_execz .LBB0_147
; %bb.146:
	flat_load_ubyte v3, v[1:2] offset:72
	s_waitcnt vmcnt(0) lgkmcnt(0)
	scratch_store_dword off, v3, s32 offset:76 ; 4-byte Folded Spill
.LBB0_147:
	s_or_b64 exec, exec, s[2:3]
	s_and_saveexec_b64 s[2:3], vcc
	s_cbranch_execz .LBB0_149
; %bb.148:
	flat_load_ubyte v19, v[1:2] offset:73
.LBB0_149:
	s_or_b64 exec, exec, s[2:3]
	v_mov_b32_e32 v23, 0
	v_mov_b32_e32 v3, 0
	scratch_store_dword off, v3, s32 offset:88 ; 4-byte Folded Spill
	s_and_saveexec_b64 s[2:3], vcc
	s_cbranch_execz .LBB0_151
; %bb.150:
	flat_load_ubyte v3, v[1:2] offset:74
	s_waitcnt vmcnt(0) lgkmcnt(0)
	scratch_store_dword off, v3, s32 offset:88 ; 4-byte Folded Spill
.LBB0_151:
	s_or_b64 exec, exec, s[2:3]
	s_and_saveexec_b64 s[2:3], vcc
	s_cbranch_execz .LBB0_153
; %bb.152:
	flat_load_ubyte v23, v[1:2] offset:75
.LBB0_153:
	s_or_b64 exec, exec, s[2:3]
	v_mov_b32_e32 v27, 0
	v_mov_b32_e32 v16, 0
	s_and_saveexec_b64 s[2:3], vcc
	s_cbranch_execz .LBB0_155
; %bb.154:
	flat_load_ubyte v16, v[1:2] offset:76
.LBB0_155:
	s_or_b64 exec, exec, s[2:3]
	s_and_saveexec_b64 s[2:3], vcc
	s_cbranch_execz .LBB0_157
; %bb.156:
	flat_load_ubyte v27, v[1:2] offset:77
.LBB0_157:
	s_or_b64 exec, exec, s[2:3]
	v_mov_b32_e32 v15, 0
	v_mov_b32_e32 v20, 0
	s_and_saveexec_b64 s[2:3], vcc
	s_cbranch_execz .LBB0_159
; %bb.158:
	flat_load_ubyte v20, v[1:2] offset:78
.LBB0_159:
	s_or_b64 exec, exec, s[2:3]
	s_and_saveexec_b64 s[2:3], vcc
	s_cbranch_execz .LBB0_161
; %bb.160:
	flat_load_ubyte v15, v[1:2] offset:79
.LBB0_161:
	s_or_b64 exec, exec, s[2:3]
	v_mov_b32_e32 v18, 0
	v_mov_b32_e32 v24, 0
	s_and_saveexec_b64 s[2:3], vcc
	s_cbranch_execz .LBB0_163
; %bb.162:
	flat_load_ubyte v24, v[1:2] offset:80
.LBB0_163:
	s_or_b64 exec, exec, s[2:3]
	s_and_saveexec_b64 s[2:3], vcc
	s_cbranch_execz .LBB0_165
; %bb.164:
	flat_load_ubyte v18, v[1:2] offset:81
.LBB0_165:
	s_or_b64 exec, exec, s[2:3]
	v_mov_b32_e32 v21, 0
	v_mov_b32_e32 v31, 0
	s_and_saveexec_b64 s[2:3], vcc
	s_cbranch_execz .LBB0_167
; %bb.166:
	flat_load_ubyte v31, v[1:2] offset:82
.LBB0_167:
	s_or_b64 exec, exec, s[2:3]
	s_and_saveexec_b64 s[2:3], vcc
	s_cbranch_execz .LBB0_169
; %bb.168:
	flat_load_ubyte v21, v[1:2] offset:83
.LBB0_169:
	s_or_b64 exec, exec, s[2:3]
	v_mov_b32_e32 v25, 0
	v_mov_b32_e32 v34, 0
	s_and_saveexec_b64 s[2:3], vcc
	s_cbranch_execz .LBB0_171
; %bb.170:
	flat_load_ubyte v34, v[1:2] offset:84
.LBB0_171:
	s_or_b64 exec, exec, s[2:3]
	s_and_saveexec_b64 s[2:3], vcc
	s_cbranch_execz .LBB0_173
; %bb.172:
	flat_load_ubyte v25, v[1:2] offset:85
.LBB0_173:
	s_or_b64 exec, exec, s[2:3]
	v_mov_b32_e32 v29, 0
	v_mov_b32_e32 v22, 0
	s_and_saveexec_b64 s[2:3], vcc
	s_cbranch_execz .LBB0_175
; %bb.174:
	flat_load_ubyte v22, v[1:2] offset:86
.LBB0_175:
	s_or_b64 exec, exec, s[2:3]
	s_and_saveexec_b64 s[2:3], vcc
	s_cbranch_execz .LBB0_177
; %bb.176:
	flat_load_ubyte v29, v[1:2] offset:87
.LBB0_177:
	s_or_b64 exec, exec, s[2:3]
	v_mov_b32_e32 v37, 0
	v_mov_b32_e32 v26, 0
	s_and_saveexec_b64 s[2:3], vcc
	s_cbranch_execz .LBB0_179
; %bb.178:
	flat_load_ubyte v26, v[1:2] offset:88
.LBB0_179:
	s_or_b64 exec, exec, s[2:3]
	s_and_saveexec_b64 s[2:3], vcc
	s_cbranch_execz .LBB0_181
; %bb.180:
	flat_load_ubyte v37, v[1:2] offset:89
.LBB0_181:
	s_or_b64 exec, exec, s[2:3]
	v_mov_b32_e32 v49, 0
	v_mov_b32_e32 v30, 0
	s_and_saveexec_b64 s[2:3], vcc
	s_cbranch_execz .LBB0_183
; %bb.182:
	flat_load_ubyte v30, v[1:2] offset:90
.LBB0_183:
	s_or_b64 exec, exec, s[2:3]
	s_and_saveexec_b64 s[2:3], vcc
	s_cbranch_execz .LBB0_185
; %bb.184:
	flat_load_ubyte v49, v[1:2] offset:91
.LBB0_185:
	s_or_b64 exec, exec, s[2:3]
	v_mov_b32_e32 v28, 0
	v_mov_b32_e32 v33, 0
	s_and_saveexec_b64 s[2:3], vcc
	s_cbranch_execz .LBB0_187
; %bb.186:
	flat_load_ubyte v33, v[1:2] offset:92
.LBB0_187:
	s_or_b64 exec, exec, s[2:3]
	s_and_saveexec_b64 s[2:3], vcc
	s_cbranch_execz .LBB0_189
; %bb.188:
	flat_load_ubyte v28, v[1:2] offset:93
.LBB0_189:
	s_or_b64 exec, exec, s[2:3]
	v_mov_b32_e32 v32, 0
	v_mov_b32_e32 v38, 0
	s_and_saveexec_b64 s[2:3], vcc
	s_cbranch_execz .LBB0_191
; %bb.190:
	flat_load_ubyte v38, v[1:2] offset:94
.LBB0_191:
	s_or_b64 exec, exec, s[2:3]
	s_and_saveexec_b64 s[2:3], vcc
	s_cbranch_execz .LBB0_193
; %bb.192:
	flat_load_ubyte v32, v[1:2] offset:95
.LBB0_193:
	s_or_b64 exec, exec, s[2:3]
	v_mov_b32_e32 v35, 0
	v_mov_b32_e32 v50, 0
	s_and_saveexec_b64 s[2:3], vcc
	s_cbranch_execz .LBB0_195
; %bb.194:
	flat_load_ubyte v50, v[1:2] offset:96
.LBB0_195:
	s_or_b64 exec, exec, s[2:3]
	s_and_saveexec_b64 s[2:3], vcc
	s_cbranch_execz .LBB0_197
; %bb.196:
	flat_load_ubyte v35, v[1:2] offset:97
.LBB0_197:
	s_or_b64 exec, exec, s[2:3]
	v_mov_b32_e32 v39, 0
	v_mov_b32_e32 v41, 0
	s_and_saveexec_b64 s[2:3], vcc
	s_cbranch_execz .LBB0_199
; %bb.198:
	flat_load_ubyte v41, v[1:2] offset:98
.LBB0_199:
	s_or_b64 exec, exec, s[2:3]
	s_and_saveexec_b64 s[2:3], vcc
	s_cbranch_execz .LBB0_201
; %bb.200:
	flat_load_ubyte v39, v[1:2] offset:99
.LBB0_201:
	s_or_b64 exec, exec, s[2:3]
	v_mov_b32_e32 v52, 0
	v_mov_b32_e32 v36, 0
	s_and_saveexec_b64 s[2:3], vcc
	s_cbranch_execz .LBB0_203
; %bb.202:
	flat_load_ubyte v36, v[1:2] offset:100
.LBB0_203:
	s_or_b64 exec, exec, s[2:3]
	s_and_saveexec_b64 s[2:3], vcc
	s_cbranch_execz .LBB0_205
; %bb.204:
	flat_load_ubyte v52, v[1:2] offset:101
.LBB0_205:
	s_or_b64 exec, exec, s[2:3]
	v_mov_b32_e32 v55, 0
	v_mov_b32_e32 v48, 0
	s_and_saveexec_b64 s[2:3], vcc
	s_cbranch_execz .LBB0_207
; %bb.206:
	flat_load_ubyte v48, v[1:2] offset:102
.LBB0_207:
	s_or_b64 exec, exec, s[2:3]
	s_and_saveexec_b64 s[2:3], vcc
	s_cbranch_execz .LBB0_209
; %bb.208:
	flat_load_ubyte v55, v[1:2] offset:103
.LBB0_209:
	s_or_b64 exec, exec, s[2:3]
	v_mov_b32_e32 v47, 0
	v_mov_b32_e32 v53, 0
	s_and_saveexec_b64 s[2:3], vcc
	s_cbranch_execz .LBB0_211
; %bb.210:
	flat_load_ubyte v53, v[1:2] offset:104
.LBB0_211:
	s_or_b64 exec, exec, s[2:3]
	s_and_saveexec_b64 s[2:3], vcc
	s_cbranch_execz .LBB0_213
; %bb.212:
	flat_load_ubyte v47, v[1:2] offset:105
.LBB0_213:
	s_or_b64 exec, exec, s[2:3]
	v_mov_b32_e32 v51, 0
	v_mov_b32_e32 v40, 0
	s_and_saveexec_b64 s[2:3], vcc
	s_cbranch_execz .LBB0_215
; %bb.214:
	flat_load_ubyte v40, v[1:2] offset:106
.LBB0_215:
	s_or_b64 exec, exec, s[2:3]
	s_and_saveexec_b64 s[2:3], vcc
	s_cbranch_execz .LBB0_217
; %bb.216:
	flat_load_ubyte v51, v[1:2] offset:107
.LBB0_217:
	s_or_b64 exec, exec, s[2:3]
	v_mov_b32_e32 v54, 0
	v_mov_b32_e32 v44, 0
	s_and_saveexec_b64 s[2:3], vcc
	s_cbranch_execz .LBB0_219
; %bb.218:
	flat_load_ubyte v44, v[1:2] offset:108
.LBB0_219:
	s_or_b64 exec, exec, s[2:3]
	s_and_saveexec_b64 s[2:3], vcc
	s_cbranch_execz .LBB0_221
; %bb.220:
	flat_load_ubyte v54, v[1:2] offset:109
.LBB0_221:
	s_or_b64 exec, exec, s[2:3]
	v_mov_b32_e32 v42, 0
	v_mov_b32_e32 v56, 0
	s_and_saveexec_b64 s[2:3], vcc
	s_cbranch_execz .LBB0_223
; %bb.222:
	flat_load_ubyte v56, v[1:2] offset:110
.LBB0_223:
	s_or_b64 exec, exec, s[2:3]
	s_and_saveexec_b64 s[2:3], vcc
	s_cbranch_execz .LBB0_225
; %bb.224:
	flat_load_ubyte v42, v[1:2] offset:111
.LBB0_225:
	s_or_b64 exec, exec, s[2:3]
	v_mov_b32_e32 v45, 0
	v_mov_b32_e32 v60, 0
	s_and_saveexec_b64 s[2:3], vcc
	s_cbranch_execz .LBB0_227
; %bb.226:
	flat_load_ubyte v60, v[1:2] offset:112
.LBB0_227:
	s_or_b64 exec, exec, s[2:3]
	s_and_saveexec_b64 s[2:3], vcc
	s_cbranch_execz .LBB0_229
; %bb.228:
	flat_load_ubyte v45, v[1:2] offset:113
.LBB0_229:
	s_or_b64 exec, exec, s[2:3]
	v_mov_b32_e32 v58, 0
	v_mov_b32_e32 v43, 0
	s_and_saveexec_b64 s[2:3], vcc
	s_cbranch_execz .LBB0_231
; %bb.230:
	flat_load_ubyte v43, v[1:2] offset:114
.LBB0_231:
	s_or_b64 exec, exec, s[2:3]
	s_and_saveexec_b64 s[2:3], vcc
	s_cbranch_execz .LBB0_233
; %bb.232:
	flat_load_ubyte v58, v[1:2] offset:115
.LBB0_233:
	s_or_b64 exec, exec, s[2:3]
	v_mov_b32_e32 v62, 0
	v_mov_b32_e32 v46, 0
	s_and_saveexec_b64 s[2:3], vcc
	s_cbranch_execz .LBB0_235
; %bb.234:
	flat_load_ubyte v46, v[1:2] offset:116
.LBB0_235:
	s_or_b64 exec, exec, s[2:3]
	s_and_saveexec_b64 s[2:3], vcc
	s_cbranch_execz .LBB0_237
; %bb.236:
	flat_load_ubyte v62, v[1:2] offset:117
.LBB0_237:
	s_or_b64 exec, exec, s[2:3]
	v_mov_b32_e32 v5, 0
	v_mov_b32_e32 v59, 0
	s_and_saveexec_b64 s[2:3], vcc
	s_cbranch_execz .LBB0_239
; %bb.238:
	flat_load_ubyte v59, v[1:2] offset:118
.LBB0_239:
	s_or_b64 exec, exec, s[2:3]
	s_and_saveexec_b64 s[2:3], vcc
	s_cbranch_execz .LBB0_241
; %bb.240:
	flat_load_ubyte v5, v[1:2] offset:119
.LBB0_241:
	s_or_b64 exec, exec, s[2:3]
	v_mov_b32_e32 v57, 0
	v_mov_b32_e32 v3, 0
	s_and_saveexec_b64 s[2:3], vcc
	s_cbranch_execz .LBB0_243
; %bb.242:
	flat_load_ubyte v3, v[1:2] offset:120
.LBB0_243:
	s_or_b64 exec, exec, s[2:3]
	s_and_saveexec_b64 s[2:3], vcc
	s_cbranch_execz .LBB0_245
; %bb.244:
	flat_load_ubyte v57, v[1:2] offset:121
.LBB0_245:
	s_or_b64 exec, exec, s[2:3]
	v_mov_b32_e32 v61, 0
	v_mov_b32_e32 v6, 0
	s_and_saveexec_b64 s[2:3], vcc
	s_cbranch_execz .LBB0_247
; %bb.246:
	flat_load_ubyte v6, v[1:2] offset:122
.LBB0_247:
	s_or_b64 exec, exec, s[2:3]
	s_and_saveexec_b64 s[2:3], vcc
	s_cbranch_execz .LBB0_249
; %bb.248:
	flat_load_ubyte v61, v[1:2] offset:123
.LBB0_249:
	s_or_b64 exec, exec, s[2:3]
	v_mov_b32_e32 v4, 0
	v_mov_b32_e32 v8, 0
	s_and_saveexec_b64 s[2:3], vcc
	s_cbranch_execz .LBB0_251
; %bb.250:
	flat_load_ubyte v8, v[1:2] offset:124
.LBB0_251:
	s_or_b64 exec, exec, s[2:3]
	s_and_saveexec_b64 s[2:3], vcc
	s_cbranch_execz .LBB0_253
; %bb.252:
	flat_load_ubyte v4, v[1:2] offset:125
.LBB0_253:
	s_or_b64 exec, exec, s[2:3]
	v_mov_b32_e32 v7, 0
	v_mov_b32_e32 v9, 0
	s_and_saveexec_b64 s[2:3], vcc
	s_cbranch_execz .LBB0_255
; %bb.254:
	flat_load_ubyte v9, v[1:2] offset:126
.LBB0_255:
	s_or_b64 exec, exec, s[2:3]
	s_waitcnt vmcnt(0) lgkmcnt(0)
	v_accvgpr_write_b32 a3, v13
	v_accvgpr_read_b32 v13, a19             ;  Reload Reuse
	v_accvgpr_write_b32 a0, v0
	s_nop 0
	v_accvgpr_write_b32 a2, v13
	v_accvgpr_read_b32 v13, a22             ;  Reload Reuse
	s_nop 1
	v_accvgpr_write_b32 a1, v13
	s_and_saveexec_b64 s[2:3], vcc
	s_cbranch_execz .LBB0_257
; %bb.256:
	flat_load_ubyte v7, v[1:2] offset:127
.LBB0_257:
	s_or_b64 exec, exec, s[2:3]
	v_lshlrev_b16_e32 v10, 8, v10
	v_or_b32_sdwa v10, v11, v10 dst_sel:DWORD dst_unused:UNUSED_PAD src0_sel:BYTE_0 src1_sel:DWORD
	v_lshlrev_b16_e32 v11, 8, v27
	v_lshlrev_b16_e32 v1, 8, v49
	v_lshlrev_b16_e32 v2, 8, v47
	v_or_b32_sdwa v0, v12, v11 dst_sel:WORD_1 dst_unused:UNUSED_PAD src0_sel:BYTE_0 src1_sel:DWORD
	v_or_b32_sdwa v1, v34, v1 dst_sel:DWORD dst_unused:UNUSED_PAD src0_sel:BYTE_0 src1_sel:DWORD
	v_or_b32_sdwa v2, v41, v2 dst_sel:WORD_1 dst_unused:UNUSED_PAD src0_sel:BYTE_0 src1_sel:DWORD
	v_or_b32_sdwa v12, v10, v0 dst_sel:DWORD dst_unused:UNUSED_PAD src0_sel:WORD_0 src1_sel:DWORD
	v_accvgpr_read_b32 v0, a52              ;  Reload Reuse
	v_or_b32_sdwa v13, v1, v2 dst_sel:DWORD dst_unused:UNUSED_PAD src0_sel:WORD_0 src1_sel:DWORD
	v_lshlrev_b16_e32 v0, 8, v0
	v_accvgpr_read_b32 v1, a47              ;  Reload Reuse
	v_or_b32_sdwa v0, v1, v0 dst_sel:DWORD dst_unused:UNUSED_PAD src0_sel:BYTE_0 src1_sel:DWORD
	scratch_load_dword v1, off, s32 offset:36 ; 4-byte Folded Reload
	v_accvgpr_read_b32 v2, a61              ;  Reload Reuse
	s_getpc_b64 s[2:3]
	s_add_u32 s2, s2, mailbox@gotpcrel32@lo+4
	s_addc_u32 s3, s3, mailbox@gotpcrel32@hi+12
	s_load_dwordx2 s[2:3], s[2:3], 0x0
	s_waitcnt vmcnt(0)
	v_lshlrev_b16_e32 v1, 8, v1
	v_or_b32_sdwa v1, v2, v1 dst_sel:WORD_1 dst_unused:UNUSED_PAD src0_sel:BYTE_0 src1_sel:DWORD
	v_or_b32_sdwa v11, v0, v1 dst_sel:DWORD dst_unused:UNUSED_PAD src0_sel:WORD_0 src1_sel:DWORD
	v_accvgpr_read_b32 v0, a24              ;  Reload Reuse
	v_lshlrev_b16_e32 v0, 8, v0
	v_accvgpr_read_b32 v1, a20              ;  Reload Reuse
	v_or_b32_sdwa v0, v1, v0 dst_sel:DWORD dst_unused:UNUSED_PAD src0_sel:BYTE_0 src1_sel:DWORD
	v_accvgpr_read_b32 v1, a38              ;  Reload Reuse
	v_lshlrev_b16_e32 v1, 8, v1
	v_accvgpr_read_b32 v2, a33              ;  Reload Reuse
	v_or_b32_sdwa v1, v2, v1 dst_sel:WORD_1 dst_unused:UNUSED_PAD src0_sel:BYTE_0 src1_sel:DWORD
	v_or_b32_sdwa v10, v0, v1 dst_sel:DWORD dst_unused:UNUSED_PAD src0_sel:WORD_0 src1_sel:DWORD
	v_mov_b32_e32 v0, 0
	s_waitcnt lgkmcnt(0)
	global_store_dwordx4 v0, v[10:13], s[2:3] offset:16
	scratch_load_dword v10, off, s32 offset:24 ; 4-byte Folded Reload
	v_accvgpr_read_b32 v11, a59             ;  Reload Reuse
	scratch_load_dword v12, off, s32 offset:56 ; 4-byte Folded Reload
	v_lshlrev_b16_e32 v1, 8, v23
	v_lshlrev_b16_e32 v2, 8, v37
	v_or_b32_sdwa v1, v17, v1 dst_sel:DWORD dst_unused:UNUSED_PAD src0_sel:BYTE_0 src1_sel:DWORD
	v_or_b32_sdwa v2, v31, v2 dst_sel:WORD_1 dst_unused:UNUSED_PAD src0_sel:BYTE_0 src1_sel:DWORD
	s_waitcnt vmcnt(1)
	v_lshlrev_b16_e32 v10, 8, v10
	v_or_b32_sdwa v10, v11, v10 dst_sel:DWORD dst_unused:UNUSED_PAD src0_sel:BYTE_0 src1_sel:DWORD
	scratch_load_dword v11, off, s32 offset:80 ; 4-byte Folded Reload
	s_waitcnt vmcnt(0)
	v_lshlrev_b16_e32 v11, 8, v11
	v_or_b32_sdwa v11, v12, v11 dst_sel:WORD_1 dst_unused:UNUSED_PAD src0_sel:BYTE_0 src1_sel:DWORD
	v_or_b32_sdwa v12, v1, v2 dst_sel:DWORD dst_unused:UNUSED_PAD src0_sel:WORD_0 src1_sel:DWORD
	v_accvgpr_read_b32 v1, a36              ;  Reload Reuse
	v_lshlrev_b16_e32 v1, 8, v1
	v_accvgpr_read_b32 v2, a31              ;  Reload Reuse
	v_or_b32_sdwa v1, v2, v1 dst_sel:DWORD dst_unused:UNUSED_PAD src0_sel:BYTE_0 src1_sel:DWORD
	v_accvgpr_read_b32 v2, a50              ;  Reload Reuse
	v_or_b32_sdwa v11, v10, v11 dst_sel:DWORD dst_unused:UNUSED_PAD src0_sel:WORD_0 src1_sel:DWORD
	v_lshlrev_b16_e32 v2, 8, v2
	v_accvgpr_read_b32 v10, a45             ;  Reload Reuse
	v_or_b32_sdwa v2, v10, v2 dst_sel:WORD_1 dst_unused:UNUSED_PAD src0_sel:BYTE_0 src1_sel:DWORD
	v_or_b32_sdwa v10, v1, v2 dst_sel:DWORD dst_unused:UNUSED_PAD src0_sel:WORD_0 src1_sel:DWORD
	v_accvgpr_read_b32 v2, a23              ;  Reload Reuse
	v_lshlrev_b16_e32 v1, 8, v5
	v_lshlrev_b16_e32 v2, 8, v2
	v_or_b32_sdwa v1, v60, v1 dst_sel:DWORD dst_unused:UNUSED_PAD src0_sel:BYTE_0 src1_sel:DWORD
	v_or_b32_sdwa v2, v9, v2 dst_sel:WORD_1 dst_unused:UNUSED_PAD src0_sel:BYTE_0 src1_sel:DWORD
	v_or_b32_sdwa v9, v1, v2 dst_sel:DWORD dst_unused:UNUSED_PAD src0_sel:WORD_0 src1_sel:DWORD
	scratch_load_dword v1, off, s32 offset:64 ; 4-byte Folded Reload
	scratch_load_dword v2, off, s32 offset:40 ; 4-byte Folded Reload
	v_accvgpr_read_b32 v5, a48              ;  Reload Reuse
	global_store_dwordx4 v0, v[9:12], s[2:3] offset:32
	v_lshlrev_b16_e32 v5, 8, v5
	v_accvgpr_read_b32 v9, a43              ;  Reload Reuse
	v_or_b32_sdwa v5, v9, v5 dst_sel:DWORD dst_unused:UNUSED_PAD src0_sel:BYTE_0 src1_sel:DWORD
	v_accvgpr_read_b32 v9, a62              ;  Reload Reuse
	v_lshlrev_b16_e32 v9, 8, v9
	v_accvgpr_read_b32 v10, a57             ;  Reload Reuse
	v_or_b32_sdwa v9, v10, v9 dst_sel:WORD_1 dst_unused:UNUSED_PAD src0_sel:BYTE_0 src1_sel:DWORD
	v_or_b32_sdwa v10, v5, v9 dst_sel:DWORD dst_unused:UNUSED_PAD src0_sel:WORD_0 src1_sel:DWORD
	v_accvgpr_read_b32 v5, a29              ;  Reload Reuse
	s_waitcnt vmcnt(2)
	v_lshlrev_b16_e32 v1, 8, v1
	s_waitcnt vmcnt(1)
	v_or_b32_sdwa v1, v2, v1 dst_sel:DWORD dst_unused:UNUSED_PAD src0_sel:BYTE_0 src1_sel:DWORD
	v_lshlrev_b16_e32 v2, 8, v19
	v_or_b32_sdwa v2, v14, v2 dst_sel:WORD_1 dst_unused:UNUSED_PAD src0_sel:BYTE_0 src1_sel:DWORD
	v_or_b32_sdwa v11, v1, v2 dst_sel:DWORD dst_unused:UNUSED_PAD src0_sel:WORD_0 src1_sel:DWORD
	v_accvgpr_read_b32 v1, a21              ;  Reload Reuse
	v_accvgpr_read_b32 v2, a34              ;  Reload Reuse
	v_lshlrev_b16_e32 v1, 8, v1
	v_lshlrev_b16_e32 v2, 8, v2
	v_or_b32_sdwa v1, v8, v1 dst_sel:DWORD dst_unused:UNUSED_PAD src0_sel:BYTE_0 src1_sel:DWORD
	v_or_b32_sdwa v2, v5, v2 dst_sel:WORD_1 dst_unused:UNUSED_PAD src0_sel:BYTE_0 src1_sel:DWORD
	v_or_b32_sdwa v9, v1, v2 dst_sel:DWORD dst_unused:UNUSED_PAD src0_sel:WORD_0 src1_sel:DWORD
	v_lshlrev_b16_e32 v1, 8, v55
	v_lshlrev_b16_e32 v2, 8, v62
	v_or_b32_sdwa v1, v50, v1 dst_sel:DWORD dst_unused:UNUSED_PAD src0_sel:BYTE_0 src1_sel:DWORD
	v_or_b32_sdwa v2, v56, v2 dst_sel:WORD_1 dst_unused:UNUSED_PAD src0_sel:BYTE_0 src1_sel:DWORD
	v_or_b32_sdwa v8, v1, v2 dst_sel:DWORD dst_unused:UNUSED_PAD src0_sel:WORD_0 src1_sel:DWORD
	v_accvgpr_read_b32 v1, a60              ;  Reload Reuse
	v_lshlrev_b16_e32 v1, 8, v1
	v_accvgpr_read_b32 v2, a55              ;  Reload Reuse
	v_or_b32_sdwa v1, v2, v1 dst_sel:DWORD dst_unused:UNUSED_PAD src0_sel:BYTE_0 src1_sel:DWORD
	scratch_load_dword v2, off, s32 offset:48 ; 4-byte Folded Reload
	scratch_load_dword v5, off, s32 offset:28 ; 4-byte Folded Reload
	s_waitcnt vmcnt(1)
	v_lshlrev_b16_e32 v2, 8, v2
	s_waitcnt vmcnt(0)
	v_or_b32_sdwa v2, v5, v2 dst_sel:WORD_1 dst_unused:UNUSED_PAD src0_sel:BYTE_0 src1_sel:DWORD
	v_accvgpr_read_b32 v5, a32              ;  Reload Reuse
	global_store_dwordx4 v0, v[8:11], s[2:3] offset:48
	v_lshlrev_b16_e32 v5, 8, v5
	v_accvgpr_read_b32 v8, a27              ;  Reload Reuse
	v_or_b32_sdwa v11, v1, v2 dst_sel:DWORD dst_unused:UNUSED_PAD src0_sel:WORD_0 src1_sel:DWORD
	v_accvgpr_read_b32 v2, a2
	v_or_b32_sdwa v5, v8, v5 dst_sel:DWORD dst_unused:UNUSED_PAD src0_sel:BYTE_0 src1_sel:DWORD
	v_accvgpr_read_b32 v8, a46              ;  Reload Reuse
	v_lshlrev_b16_e32 v1, 8, v58
	v_lshlrev_b16_e32 v2, 8, v2
	v_lshlrev_b16_e32 v8, 8, v8
	v_accvgpr_read_b32 v9, a41              ;  Reload Reuse
	v_or_b32_sdwa v1, v44, v1 dst_sel:DWORD dst_unused:UNUSED_PAD src0_sel:BYTE_0 src1_sel:DWORD
	v_or_b32_sdwa v2, v6, v2 dst_sel:WORD_1 dst_unused:UNUSED_PAD src0_sel:BYTE_0 src1_sel:DWORD
	v_or_b32_sdwa v8, v9, v8 dst_sel:WORD_1 dst_unused:UNUSED_PAD src0_sel:BYTE_0 src1_sel:DWORD
	v_or_b32_sdwa v9, v1, v2 dst_sel:DWORD dst_unused:UNUSED_PAD src0_sel:WORD_0 src1_sel:DWORD
	v_lshlrev_b16_e32 v1, 8, v29
	v_lshlrev_b16_e32 v2, 8, v52
	v_or_b32_sdwa v1, v24, v1 dst_sel:DWORD dst_unused:UNUSED_PAD src0_sel:BYTE_0 src1_sel:DWORD
	v_or_b32_sdwa v2, v38, v2 dst_sel:WORD_1 dst_unused:UNUSED_PAD src0_sel:BYTE_0 src1_sel:DWORD
	v_or_b32_sdwa v10, v5, v8 dst_sel:DWORD dst_unused:UNUSED_PAD src0_sel:WORD_0 src1_sel:DWORD
	v_or_b32_sdwa v8, v1, v2 dst_sel:DWORD dst_unused:UNUSED_PAD src0_sel:WORD_0 src1_sel:DWORD
	v_accvgpr_read_b32 v1, a44              ;  Reload Reuse
	v_lshlrev_b16_e32 v1, 8, v1
	v_accvgpr_read_b32 v2, a39              ;  Reload Reuse
	v_or_b32_sdwa v1, v2, v1 dst_sel:DWORD dst_unused:UNUSED_PAD src0_sel:BYTE_0 src1_sel:DWORD
	v_accvgpr_read_b32 v2, a58              ;  Reload Reuse
	v_lshlrev_b16_e32 v2, 8, v2
	v_accvgpr_read_b32 v5, a53              ;  Reload Reuse
	v_or_b32_sdwa v2, v5, v2 dst_sel:WORD_1 dst_unused:UNUSED_PAD src0_sel:BYTE_0 src1_sel:DWORD
	v_lshlrev_b16_e32 v5, 8, v7
	global_store_dwordx4 v0, v[8:11], s[2:3] offset:64
	v_or_b32_sdwa v3, v3, v5 dst_sel:DWORD dst_unused:UNUSED_PAD src0_sel:BYTE_0 src1_sel:DWORD
	v_accvgpr_read_b32 v5, a30              ;  Reload Reuse
	v_or_b32_sdwa v8, v1, v2 dst_sel:DWORD dst_unused:UNUSED_PAD src0_sel:WORD_0 src1_sel:DWORD
	v_lshlrev_b16_e32 v1, 8, v39
	v_lshlrev_b16_e32 v2, 8, v45
	v_lshlrev_b16_e32 v5, 8, v5
	v_accvgpr_read_b32 v6, a25              ;  Reload Reuse
	v_or_b32_sdwa v1, v33, v1 dst_sel:DWORD dst_unused:UNUSED_PAD src0_sel:BYTE_0 src1_sel:DWORD
	v_or_b32_sdwa v2, v40, v2 dst_sel:WORD_1 dst_unused:UNUSED_PAD src0_sel:BYTE_0 src1_sel:DWORD
	v_or_b32_sdwa v5, v6, v5 dst_sel:WORD_1 dst_unused:UNUSED_PAD src0_sel:BYTE_0 src1_sel:DWORD
	v_or_b32_sdwa v6, v1, v2 dst_sel:DWORD dst_unused:UNUSED_PAD src0_sel:WORD_0 src1_sel:DWORD
	scratch_load_dword v1, off, s32 offset:84 ; 4-byte Folded Reload
	scratch_load_dword v2, off, s32 offset:68 ; 4-byte Folded Reload
	v_or_b32_sdwa v7, v3, v5 dst_sel:DWORD dst_unused:UNUSED_PAD src0_sel:WORD_0 src1_sel:DWORD
	v_accvgpr_read_b32 v3, a37              ;  Reload Reuse
	s_waitcnt vmcnt(1)
	v_lshlrev_b16_e32 v1, 8, v1
	s_waitcnt vmcnt(0)
	v_or_b32_sdwa v1, v2, v1 dst_sel:DWORD dst_unused:UNUSED_PAD src0_sel:BYTE_0 src1_sel:DWORD
	v_lshlrev_b16_e32 v2, 8, v25
	v_or_b32_sdwa v2, v20, v2 dst_sel:WORD_1 dst_unused:UNUSED_PAD src0_sel:BYTE_0 src1_sel:DWORD
	v_or_b32_sdwa v5, v1, v2 dst_sel:DWORD dst_unused:UNUSED_PAD src0_sel:WORD_0 src1_sel:DWORD
	v_accvgpr_read_b32 v1, a28              ;  Reload Reuse
	v_lshlrev_b16_e32 v1, 8, v1
	v_accvgpr_read_b32 v2, a3
	v_or_b32_sdwa v1, v2, v1 dst_sel:DWORD dst_unused:UNUSED_PAD src0_sel:BYTE_0 src1_sel:DWORD
	v_accvgpr_read_b32 v2, a42              ;  Reload Reuse
	v_lshlrev_b16_e32 v2, 8, v2
	v_or_b32_sdwa v2, v3, v2 dst_sel:WORD_1 dst_unused:UNUSED_PAD src0_sel:BYTE_0 src1_sel:DWORD
	global_store_dwordx4 v0, v[5:8], s[2:3] offset:80
	scratch_load_dword v6, off, s32 offset:52 ; 4-byte Folded Reload
	v_or_b32_sdwa v5, v1, v2 dst_sel:DWORD dst_unused:UNUSED_PAD src0_sel:WORD_0 src1_sel:DWORD
	v_lshlrev_b16_e32 v1, 8, v42
	v_lshlrev_b16_e32 v2, 8, v4
	v_or_b32_sdwa v1, v53, v1 dst_sel:DWORD dst_unused:UNUSED_PAD src0_sel:BYTE_0 src1_sel:DWORD
	v_or_b32_sdwa v2, v59, v2 dst_sel:WORD_1 dst_unused:UNUSED_PAD src0_sel:BYTE_0 src1_sel:DWORD
	v_or_b32_sdwa v4, v1, v2 dst_sel:DWORD dst_unused:UNUSED_PAD src0_sel:WORD_0 src1_sel:DWORD
	v_lshlrev_b16_e32 v1, 8, v21
	v_lshlrev_b16_e32 v2, 8, v35
	v_or_b32_sdwa v1, v16, v1 dst_sel:DWORD dst_unused:UNUSED_PAD src0_sel:BYTE_0 src1_sel:DWORD
	v_or_b32_sdwa v2, v30, v2 dst_sel:WORD_1 dst_unused:UNUSED_PAD src0_sel:BYTE_0 src1_sel:DWORD
	v_or_b32_sdwa v3, v1, v2 dst_sel:DWORD dst_unused:UNUSED_PAD src0_sel:WORD_0 src1_sel:DWORD
	scratch_load_dword v1, off, s32 offset:16 ; 4-byte Folded Reload
	scratch_load_dword v2, off, s32 offset:8 ; 4-byte Folded Reload
	s_waitcnt vmcnt(1)
	v_lshlrev_b16_e32 v1, 8, v1
	s_waitcnt vmcnt(0)
	v_or_b32_sdwa v1, v2, v1 dst_sel:DWORD dst_unused:UNUSED_PAD src0_sel:BYTE_0 src1_sel:DWORD
	scratch_load_dword v2, off, s32 offset:72 ; 4-byte Folded Reload
	s_waitcnt vmcnt(0)
	v_lshlrev_b16_e32 v2, 8, v2
	v_or_b32_sdwa v2, v6, v2 dst_sel:WORD_1 dst_unused:UNUSED_PAD src0_sel:BYTE_0 src1_sel:DWORD
	v_or_b32_sdwa v2, v1, v2 dst_sel:DWORD dst_unused:UNUSED_PAD src0_sel:WORD_0 src1_sel:DWORD
	global_store_dwordx4 v0, v[2:5], s[2:3] offset:96
	v_lshlrev_b16_e32 v1, 8, v61
	v_accvgpr_read_b32 v2, a26              ;  Reload Reuse
	v_lshlrev_b16_e32 v2, 8, v2
	v_accvgpr_read_b32 v3, a1
	v_or_b32_sdwa v1, v46, v1 dst_sel:DWORD dst_unused:UNUSED_PAD src0_sel:BYTE_0 src1_sel:DWORD
	v_or_b32_sdwa v2, v3, v2 dst_sel:WORD_1 dst_unused:UNUSED_PAD src0_sel:BYTE_0 src1_sel:DWORD
	v_or_b32_sdwa v4, v1, v2 dst_sel:DWORD dst_unused:UNUSED_PAD src0_sel:WORD_0 src1_sel:DWORD
	v_lshlrev_b16_e32 v1, 8, v32
	v_lshlrev_b16_e32 v2, 8, v54
	v_or_b32_sdwa v1, v26, v1 dst_sel:DWORD dst_unused:UNUSED_PAD src0_sel:BYTE_0 src1_sel:DWORD
	v_or_b32_sdwa v2, v48, v2 dst_sel:WORD_1 dst_unused:UNUSED_PAD src0_sel:BYTE_0 src1_sel:DWORD
	v_or_b32_sdwa v3, v1, v2 dst_sel:DWORD dst_unused:UNUSED_PAD src0_sel:WORD_0 src1_sel:DWORD
	scratch_load_dword v1, off, s32 offset:60 ; 4-byte Folded Reload
	scratch_load_dword v5, off, s32 offset:88 ; 4-byte Folded Reload
	scratch_load_dword v2, off, s32 offset:32 ; 4-byte Folded Reload
	scratch_load_dword v6, off, s32         ; 4-byte Folded Reload
	s_waitcnt vmcnt(3)
	v_lshlrev_b16_e32 v1, 8, v1
	s_waitcnt vmcnt(1)
	v_or_b32_sdwa v1, v2, v1 dst_sel:DWORD dst_unused:UNUSED_PAD src0_sel:BYTE_0 src1_sel:DWORD
	v_lshlrev_b16_e32 v2, 8, v18
	v_or_b32_sdwa v2, v5, v2 dst_sel:WORD_1 dst_unused:UNUSED_PAD src0_sel:BYTE_0 src1_sel:DWORD
	v_or_b32_sdwa v2, v1, v2 dst_sel:DWORD dst_unused:UNUSED_PAD src0_sel:WORD_0 src1_sel:DWORD
	v_accvgpr_read_b32 v1, a56              ;  Reload Reuse
	v_lshlrev_b16_e32 v1, 8, v1
	v_accvgpr_read_b32 v5, a51              ;  Reload Reuse
	v_or_b32_sdwa v1, v5, v1 dst_sel:DWORD dst_unused:UNUSED_PAD src0_sel:BYTE_0 src1_sel:DWORD
	scratch_load_dword v5, off, s32 offset:12 ; 4-byte Folded Reload
	s_waitcnt vmcnt(0)
	v_lshlrev_b16_e32 v5, 8, v5
	v_or_b32_sdwa v5, v6, v5 dst_sel:WORD_1 dst_unused:UNUSED_PAD src0_sel:BYTE_0 src1_sel:DWORD
	v_or_b32_sdwa v1, v1, v5 dst_sel:DWORD dst_unused:UNUSED_PAD src0_sel:WORD_0 src1_sel:DWORD
	global_store_dwordx4 v0, v[1:4], s[2:3] offset:112
	scratch_load_dword v5, off, s32 offset:20 ; 4-byte Folded Reload
	v_lshlrev_b16_e32 v1, 8, v51
	v_lshlrev_b16_e32 v2, 8, v57
	v_or_b32_sdwa v1, v36, v1 dst_sel:DWORD dst_unused:UNUSED_PAD src0_sel:BYTE_0 src1_sel:DWORD
	v_or_b32_sdwa v2, v43, v2 dst_sel:WORD_1 dst_unused:UNUSED_PAD src0_sel:BYTE_0 src1_sel:DWORD
	v_or_b32_sdwa v4, v1, v2 dst_sel:DWORD dst_unused:UNUSED_PAD src0_sel:WORD_0 src1_sel:DWORD
	scratch_load_dword v2, off, s32 offset:76 ; 4-byte Folded Reload
	v_lshlrev_b16_e32 v1, 8, v15
	v_accvgpr_read_b32 v6, a49              ;  Reload Reuse
	s_waitcnt vmcnt(0)
	v_or_b32_sdwa v1, v2, v1 dst_sel:DWORD dst_unused:UNUSED_PAD src0_sel:BYTE_0 src1_sel:DWORD
	v_lshlrev_b16_e32 v2, 8, v28
	v_or_b32_sdwa v2, v22, v2 dst_sel:WORD_1 dst_unused:UNUSED_PAD src0_sel:BYTE_0 src1_sel:DWORD
	v_or_b32_sdwa v3, v1, v2 dst_sel:DWORD dst_unused:UNUSED_PAD src0_sel:WORD_0 src1_sel:DWORD
	scratch_load_dword v1, off, s32 offset:4 ; 4-byte Folded Reload
	v_accvgpr_read_b32 v2, a63              ;  Reload Reuse
	s_waitcnt vmcnt(0)
	v_lshlrev_b16_e32 v1, 8, v1
	v_or_b32_sdwa v1, v2, v1 dst_sel:DWORD dst_unused:UNUSED_PAD src0_sel:BYTE_0 src1_sel:DWORD
	scratch_load_dword v2, off, s32 offset:44 ; 4-byte Folded Reload
	s_waitcnt vmcnt(0)
	v_lshlrev_b16_e32 v2, 8, v2
	v_or_b32_sdwa v2, v5, v2 dst_sel:WORD_1 dst_unused:UNUSED_PAD src0_sel:BYTE_0 src1_sel:DWORD
	v_or_b32_sdwa v2, v1, v2 dst_sel:DWORD dst_unused:UNUSED_PAD src0_sel:WORD_0 src1_sel:DWORD
	v_accvgpr_read_b32 v1, a40              ;  Reload Reuse
	v_lshlrev_b16_e32 v1, 8, v1
	v_accvgpr_read_b32 v5, a35              ;  Reload Reuse
	v_or_b32_sdwa v1, v5, v1 dst_sel:DWORD dst_unused:UNUSED_PAD src0_sel:BYTE_0 src1_sel:DWORD
	v_accvgpr_read_b32 v5, a54              ;  Reload Reuse
	v_lshlrev_b16_e32 v5, 8, v5
	v_or_b32_sdwa v5, v6, v5 dst_sel:WORD_1 dst_unused:UNUSED_PAD src0_sel:BYTE_0 src1_sel:DWORD
	v_or_b32_sdwa v1, v1, v5 dst_sel:DWORD dst_unused:UNUSED_PAD src0_sel:WORD_0 src1_sel:DWORD
	global_store_dwordx4 v0, v[1:4], s[2:3] offset:128
	s_nop 0
	v_accvgpr_read_b32 v1, a0
	v_mov_b32_e32 v2, 1
	s_nop 0
	global_store_dword v0, v1, s[2:3] offset:8
	v_mov_b32_e32 v0, s2
	v_mov_b32_e32 v1, s3
	s_waitcnt vmcnt(0)
	buffer_wbinvl1_vol
	flat_store_dword v[0:1], v2 offset:12
	s_waitcnt vmcnt(0)
	flat_load_dword v0, v[0:1] offset:12 glc
	s_waitcnt vmcnt(0) lgkmcnt(0)
	v_cmp_ne_u32_e32 vcc, 2, v0
	s_and_b64 exec, exec, vcc
	s_cbranch_execz .LBB0_260
; %bb.258:
	s_mov_b64 s[2:3], 0
.LBB0_259:                              ; =>This Inner Loop Header: Depth=1
	s_getpc_b64 s[4:5]
	s_add_u32 s4, s4, mailbox@gotpcrel32@lo+4
	s_addc_u32 s5, s5, mailbox@gotpcrel32@hi+12
	s_load_dwordx2 s[4:5], s[4:5], 0x0
	s_sleep 64
	s_waitcnt lgkmcnt(0)
	v_mov_b32_e32 v0, s4
	v_mov_b32_e32 v1, s5
	flat_load_dword v0, v[0:1] offset:12 glc
	s_waitcnt vmcnt(0) lgkmcnt(0)
	v_cmp_eq_u32_e32 vcc, 2, v0
	s_or_b64 s[2:3], vcc, s[2:3]
	s_andn2_b64 exec, exec, s[2:3]
	s_cbranch_execnz .LBB0_259
.LBB0_260:
	s_or_b64 exec, exec, s[0:1]
	; wave barrier
	v_accvgpr_read_b32 v62, a18             ;  Reload Reuse
	v_accvgpr_read_b32 v61, a17             ;  Reload Reuse
	v_accvgpr_read_b32 v60, a16             ;  Reload Reuse
	v_accvgpr_read_b32 v59, a15             ;  Reload Reuse
	v_accvgpr_read_b32 v58, a14             ;  Reload Reuse
	v_accvgpr_read_b32 v57, a13             ;  Reload Reuse
	v_accvgpr_read_b32 v56, a12             ;  Reload Reuse
	v_accvgpr_read_b32 v47, a11             ;  Reload Reuse
	v_accvgpr_read_b32 v46, a10             ;  Reload Reuse
	v_accvgpr_read_b32 v45, a9              ;  Reload Reuse
	v_accvgpr_read_b32 v44, a8              ;  Reload Reuse
	v_accvgpr_read_b32 v43, a7              ;  Reload Reuse
	v_accvgpr_read_b32 v42, a6              ;  Reload Reuse
	v_accvgpr_read_b32 v41, a5              ;  Reload Reuse
	v_accvgpr_read_b32 v40, a4              ;  Reload Reuse
	s_setpc_b64 s[30:31]
.Lfunc_end0:
	.size	gpu_hostcall_staged, .Lfunc_end0-gpu_hostcall_staged
                                        ; -- End function
	.set .Lgpu_hostcall_staged.num_vgpr, 63
	.set .Lgpu_hostcall_staged.num_agpr, 64
	.set .Lgpu_hostcall_staged.numbered_sgpr, 33
	.set .Lgpu_hostcall_staged.private_seg_size, 96
	.set .Lgpu_hostcall_staged.uses_vcc, 1
	.set .Lgpu_hostcall_staged.uses_flat_scratch, 0
	.set .Lgpu_hostcall_staged.has_dyn_sized_stack, 0
	.set .Lgpu_hostcall_staged.has_recursion, 0
	.set .Lgpu_hostcall_staged.has_indirect_call, 0
	.section	.AMDGPU.csdata,"",@progbits
; Function info:
; codeLenInByte = 6852
; TotalNumSgprs: 39
; NumVgprs: 63
; NumAgprs: 64
; TotalNumVgprs: 64
; ScratchSize: 96
; MemoryBound: 0
	.section	.AMDGPU.gpr_maximums,"",@progbits
	.set amdgpu.max_num_vgpr, 63
	.set amdgpu.max_num_agpr, 64
	.set amdgpu.max_num_sgpr, 33
	.section	.AMDGPU.csdata,"",@progbits
	.type	__hip_cuid_38979112a0c84ce7,@object ; @__hip_cuid_38979112a0c84ce7
	.section	.bss,"aw",@nobits
	.globl	__hip_cuid_38979112a0c84ce7
__hip_cuid_38979112a0c84ce7:
	.byte	0                               ; 0x0
	.size	__hip_cuid_38979112a0c84ce7, 1

	.ident	"AMD clang version 20.0.0git (https://github.com/RadeonOpenCompute/llvm-project roc-7.0.2 25385 0dda3adf56766e0aac0d03173ced3759e1ffecbc)"
	.section	".note.GNU-stack","",@progbits
	.addrsig
	.addrsig_sym gpu_hostcall_staged
	.addrsig_sym mailbox
	.addrsig_sym __hip_cuid_38979112a0c84ce7
	.amdgpu_metadata
---
amdhsa.kernels:  []
amdhsa.target:   amdgcn-amd-amdhsa--gfx908
amdhsa.version:
  - 1
  - 2
...

	.end_amdgpu_metadata
