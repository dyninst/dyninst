//
// Created by angus on 07/25/24.
//

#ifndef ROSE_RISCV64INSTRUCTIONENUM_H
#define ROSE_RISCV64INSTRUCTIONENUM_H

/** RISC-V major register numbers */
enum Riscv64RegisterClass {
    riscv64_regclass_gpr,
    riscv64_regclass_fpr32,
    riscv64_regclass_fpr64,
    riscv64_regclass_pc,
};

/** RISC-V general purpose registers */
enum Riscv64GeneralPurposeRegister {
    riscv64_gpr_x0,
    riscv64_gpr_x1,
    riscv64_gpr_x2,
    riscv64_gpr_x3,
    riscv64_gpr_x4,
    riscv64_gpr_x5,
    riscv64_gpr_x6,
    riscv64_gpr_x7,
    riscv64_gpr_x8,
    riscv64_gpr_x9,
    riscv64_gpr_x10,
    riscv64_gpr_x11,
    riscv64_gpr_x12,
    riscv64_gpr_x13,
    riscv64_gpr_x14,
    riscv64_gpr_x15,
    riscv64_gpr_x16,
    riscv64_gpr_x17,
    riscv64_gpr_x18,
    riscv64_gpr_x19,
    riscv64_gpr_x20,
    riscv64_gpr_x21,
    riscv64_gpr_x22,
    riscv64_gpr_x23,
    riscv64_gpr_x24,
    riscv64_gpr_x25,
    riscv64_gpr_x26,
    riscv64_gpr_x27,
    riscv64_gpr_x28,
    riscv64_gpr_x29,
    riscv64_gpr_x30,
    riscv64_gpr_x31,
};

/** RISC-V FP 32 bit registers */
enum Riscv64FloatingPoint32Register {
    riscv64_fpr_f0_32,
    riscv64_fpr_f1_32,
    riscv64_fpr_f2_32,
    riscv64_fpr_f3_32,
    riscv64_fpr_f4_32,
    riscv64_fpr_f5_32,
    riscv64_fpr_f6_32,
    riscv64_fpr_f7_32,
    riscv64_fpr_f8_32,
    riscv64_fpr_f9_32,
    riscv64_fpr_f10_32,
    riscv64_fpr_f11_32,
    riscv64_fpr_f12_32,
    riscv64_fpr_f13_32,
    riscv64_fpr_f14_32,
    riscv64_fpr_f15_32,
    riscv64_fpr_f16_32,
    riscv64_fpr_f17_32,
    riscv64_fpr_f18_32,
    riscv64_fpr_f19_32,
    riscv64_fpr_f20_32,
    riscv64_fpr_f21_32,
    riscv64_fpr_f22_32,
    riscv64_fpr_f23_32,
    riscv64_fpr_f24_32,
    riscv64_fpr_f25_32,
    riscv64_fpr_f26_32,
    riscv64_fpr_f27_32,
    riscv64_fpr_f28_32,
    riscv64_fpr_f29_32,
    riscv64_fpr_f30_32,
    riscv64_fpr_f31_32,
};

/** RISC-V FP 64 bit registers */
enum Riscv64FloatingPoint64Register {
    riscv64_fpr_f0_64,
    riscv64_fpr_f1_64,
    riscv64_fpr_f2_64,
    riscv64_fpr_f3_64,
    riscv64_fpr_f4_64,
    riscv64_fpr_f5_64,
    riscv64_fpr_f6_64,
    riscv64_fpr_f7_64,
    riscv64_fpr_f8_64,
    riscv64_fpr_f9_64,
    riscv64_fpr_f10_64,
    riscv64_fpr_f11_64,
    riscv64_fpr_f12_64,
    riscv64_fpr_f13_64,
    riscv64_fpr_f14_64,
    riscv64_fpr_f15_64,
    riscv64_fpr_f16_64,
    riscv64_fpr_f17_64,
    riscv64_fpr_f18_64,
    riscv64_fpr_f19_64,
    riscv64_fpr_f20_64,
    riscv64_fpr_f21_64,
    riscv64_fpr_f22_64,
    riscv64_fpr_f23_64,
    riscv64_fpr_f24_64,
    riscv64_fpr_f25_64,
    riscv64_fpr_f26_64,
    riscv64_fpr_f27_64,
    riscv64_fpr_f28_64,
    riscv64_fpr_f29_64,
    riscv64_fpr_f30_64,
    riscv64_fpr_f31_64,
};

/*
 * RISC-V Instructions
 */
enum Riscv64InstructionKind {
    rose_riscv64_op_INVALID = 0,
    rose_riscv64_op_add,
    rose_riscv64_op_addi,
    rose_riscv64_op_addiw,
    rose_riscv64_op_addw,
    rose_riscv64_op_amoadd_d,
    rose_riscv64_op_amoadd_d_aq,
    rose_riscv64_op_amoadd_d_aq_rl,
    rose_riscv64_op_amoadd_d_rl,
    rose_riscv64_op_amoadd_w,
    rose_riscv64_op_amoadd_w_aq,
    rose_riscv64_op_amoadd_w_aq_rl,
    rose_riscv64_op_amoadd_w_rl,
    rose_riscv64_op_amoand_d,
    rose_riscv64_op_amoand_d_aq,
    rose_riscv64_op_amoand_d_aq_rl,
    rose_riscv64_op_amoand_d_rl,
    rose_riscv64_op_amoand_w,
    rose_riscv64_op_amoand_w_aq,
    rose_riscv64_op_amoand_w_aq_rl,
    rose_riscv64_op_amoand_w_rl,
    rose_riscv64_op_amomax_d,
    rose_riscv64_op_amomax_d_aq,
    rose_riscv64_op_amomax_d_aq_rl,
    rose_riscv64_op_amomax_d_rl,
    rose_riscv64_op_amomax_w,
    rose_riscv64_op_amomax_w_aq,
    rose_riscv64_op_amomax_w_aq_rl,
    rose_riscv64_op_amomax_w_rl,
    rose_riscv64_op_amomaxu_d,
    rose_riscv64_op_amomaxu_d_aq,
    rose_riscv64_op_amomaxu_d_aq_rl,
    rose_riscv64_op_amomaxu_d_rl,
    rose_riscv64_op_amomaxu_w,
    rose_riscv64_op_amomaxu_w_aq,
    rose_riscv64_op_amomaxu_w_aq_rl,
    rose_riscv64_op_amomaxu_w_rl,
    rose_riscv64_op_amomin_d,
    rose_riscv64_op_amomin_d_aq,
    rose_riscv64_op_amomin_d_aq_rl,
    rose_riscv64_op_amomin_d_rl,
    rose_riscv64_op_amomin_w,
    rose_riscv64_op_amomin_w_aq,
    rose_riscv64_op_amomin_w_aq_rl,
    rose_riscv64_op_amomin_w_rl,
    rose_riscv64_op_amominu_d,
    rose_riscv64_op_amominu_d_aq,
    rose_riscv64_op_amominu_d_aq_rl,
    rose_riscv64_op_amominu_d_rl,
    rose_riscv64_op_amominu_w,
    rose_riscv64_op_amominu_w_aq,
    rose_riscv64_op_amominu_w_aq_rl,
    rose_riscv64_op_amominu_w_rl,
    rose_riscv64_op_amoor_d,
    rose_riscv64_op_amoor_d_aq,
    rose_riscv64_op_amoor_d_aq_rl,
    rose_riscv64_op_amoor_d_rl,
    rose_riscv64_op_amoor_w,
    rose_riscv64_op_amoor_w_aq,
    rose_riscv64_op_amoor_w_aq_rl,
    rose_riscv64_op_amoor_w_rl,
    rose_riscv64_op_amoswap_d,
    rose_riscv64_op_amoswap_d_aq,
    rose_riscv64_op_amoswap_d_aq_rl,
    rose_riscv64_op_amoswap_d_rl,
    rose_riscv64_op_amoswap_w,
    rose_riscv64_op_amoswap_w_aq,
    rose_riscv64_op_amoswap_w_aq_rl,
    rose_riscv64_op_amoswap_w_rl,
    rose_riscv64_op_amoxor_d,
    rose_riscv64_op_amoxor_d_aq,
    rose_riscv64_op_amoxor_d_aq_rl,
    rose_riscv64_op_amoxor_d_rl,
    rose_riscv64_op_amoxor_w,
    rose_riscv64_op_amoxor_w_aq,
    rose_riscv64_op_amoxor_w_aq_rl,
    rose_riscv64_op_amoxor_w_rl,
    rose_riscv64_op_and,
    rose_riscv64_op_andi,
    rose_riscv64_op_auipc,
    rose_riscv64_op_beq,
    rose_riscv64_op_bge,
    rose_riscv64_op_bgeu,
    rose_riscv64_op_blt,
    rose_riscv64_op_bltu,
    rose_riscv64_op_bne,
    rose_riscv64_op_c_add,
    rose_riscv64_op_c_addi,
    rose_riscv64_op_c_addi16sp,
    rose_riscv64_op_c_addi4spn,
    rose_riscv64_op_c_addiw,
    rose_riscv64_op_c_addw,
    rose_riscv64_op_c_and,
    rose_riscv64_op_c_andi,
    rose_riscv64_op_c_beqz,
    rose_riscv64_op_c_bnez,
    rose_riscv64_op_c_ebreak,
    rose_riscv64_op_c_fld,
    rose_riscv64_op_c_fldsp,
    rose_riscv64_op_c_flw,
    rose_riscv64_op_c_flwsp,
    rose_riscv64_op_c_fsd,
    rose_riscv64_op_c_fsdsp,
    rose_riscv64_op_c_fsw,
    rose_riscv64_op_c_fswsp,
    rose_riscv64_op_c_j,
    rose_riscv64_op_c_jal,
    rose_riscv64_op_c_jalr,
    rose_riscv64_op_c_jr,
    rose_riscv64_op_c_ld,
    rose_riscv64_op_c_ldsp,
    rose_riscv64_op_c_li,
    rose_riscv64_op_c_lui,
    rose_riscv64_op_c_lw,
    rose_riscv64_op_c_lwsp,
    rose_riscv64_op_c_mv,
    rose_riscv64_op_c_nop,
    rose_riscv64_op_c_or,
    rose_riscv64_op_c_sd,
    rose_riscv64_op_c_sdsp,
    rose_riscv64_op_c_slli,
    rose_riscv64_op_c_srai,
    rose_riscv64_op_c_srli,
    rose_riscv64_op_c_sub,
    rose_riscv64_op_c_subw,
    rose_riscv64_op_c_sw,
    rose_riscv64_op_c_swsp,
    rose_riscv64_op_c_unimp,
    rose_riscv64_op_c_xor,
    rose_riscv64_op_csrrc,
    rose_riscv64_op_csrrci,
    rose_riscv64_op_csrrs,
    rose_riscv64_op_csrrsi,
    rose_riscv64_op_csrrw,
    rose_riscv64_op_csrrwi,
    rose_riscv64_op_div,
    rose_riscv64_op_divu,
    rose_riscv64_op_divuw,
    rose_riscv64_op_divw,
    rose_riscv64_op_ebreak,
    rose_riscv64_op_ecall,
    rose_riscv64_op_fadd_d,
    rose_riscv64_op_fadd_s,
    rose_riscv64_op_fclass_d,
    rose_riscv64_op_fclass_s,
    rose_riscv64_op_fcvt_d_l,
    rose_riscv64_op_fcvt_d_lu,
    rose_riscv64_op_fcvt_d_s,
    rose_riscv64_op_fcvt_d_w,
    rose_riscv64_op_fcvt_d_wu,
    rose_riscv64_op_fcvt_l_d,
    rose_riscv64_op_fcvt_l_s,
    rose_riscv64_op_fcvt_lu_d,
    rose_riscv64_op_fcvt_lu_s,
    rose_riscv64_op_fcvt_s_d,
    rose_riscv64_op_fcvt_s_l,
    rose_riscv64_op_fcvt_s_lu,
    rose_riscv64_op_fcvt_s_w,
    rose_riscv64_op_fcvt_s_wu,
    rose_riscv64_op_fcvt_w_d,
    rose_riscv64_op_fcvt_w_s,
    rose_riscv64_op_fcvt_wu_d,
    rose_riscv64_op_fcvt_wu_s,
    rose_riscv64_op_fdiv_d,
    rose_riscv64_op_fdiv_s,
    rose_riscv64_op_fence,
    rose_riscv64_op_fence_i,
    rose_riscv64_op_fence_tso,
    rose_riscv64_op_feq_d,
    rose_riscv64_op_feq_s,
    rose_riscv64_op_fld,
    rose_riscv64_op_fle_d,
    rose_riscv64_op_fle_s,
    rose_riscv64_op_flt_d,
    rose_riscv64_op_flt_s,
    rose_riscv64_op_flw,
    rose_riscv64_op_fmadd_d,
    rose_riscv64_op_fmadd_s,
    rose_riscv64_op_fmax_d,
    rose_riscv64_op_fmax_s,
    rose_riscv64_op_fmin_d,
    rose_riscv64_op_fmin_s,
    rose_riscv64_op_fmsub_d,
    rose_riscv64_op_fmsub_s,
    rose_riscv64_op_fmul_d,
    rose_riscv64_op_fmul_s,
    rose_riscv64_op_fmv_d_x,
    rose_riscv64_op_fmv_w_x,
    rose_riscv64_op_fmv_x_d,
    rose_riscv64_op_fmv_x_w,
    rose_riscv64_op_fnmadd_d,
    rose_riscv64_op_fnmadd_s,
    rose_riscv64_op_fnmsub_d,
    rose_riscv64_op_fnmsub_s,
    rose_riscv64_op_fsd,
    rose_riscv64_op_fsgnj_d,
    rose_riscv64_op_fsgnj_s,
    rose_riscv64_op_fsgnjn_d,
    rose_riscv64_op_fsgnjn_s,
    rose_riscv64_op_fsgnjx_d,
    rose_riscv64_op_fsgnjx_s,
    rose_riscv64_op_fsqrt_d,
    rose_riscv64_op_fsqrt_s,
    rose_riscv64_op_fsub_d,
    rose_riscv64_op_fsub_s,
    rose_riscv64_op_fsw,
    rose_riscv64_op_jal,
    rose_riscv64_op_jalr,
    rose_riscv64_op_lb,
    rose_riscv64_op_lbu,
    rose_riscv64_op_ld,
    rose_riscv64_op_lh,
    rose_riscv64_op_lhu,
    rose_riscv64_op_lr_d,
    rose_riscv64_op_lr_d_aq,
    rose_riscv64_op_lr_d_aq_rl,
    rose_riscv64_op_lr_d_rl,
    rose_riscv64_op_lr_w,
    rose_riscv64_op_lr_w_aq,
    rose_riscv64_op_lr_w_aq_rl,
    rose_riscv64_op_lr_w_rl,
    rose_riscv64_op_lui,
    rose_riscv64_op_lw,
    rose_riscv64_op_lwu,
    rose_riscv64_op_mret,
    rose_riscv64_op_mul,
    rose_riscv64_op_mulh,
    rose_riscv64_op_mulhsu,
    rose_riscv64_op_mulhu,
    rose_riscv64_op_mulw,
    rose_riscv64_op_or,
    rose_riscv64_op_ori,
    rose_riscv64_op_rem,
    rose_riscv64_op_remu,
    rose_riscv64_op_remuw,
    rose_riscv64_op_remw,
    rose_riscv64_op_sb,
    rose_riscv64_op_sc_d,
    rose_riscv64_op_sc_d_aq,
    rose_riscv64_op_sc_d_aq_rl,
    rose_riscv64_op_sc_d_rl,
    rose_riscv64_op_sc_w,
    rose_riscv64_op_sc_w_aq,
    rose_riscv64_op_sc_w_aq_rl,
    rose_riscv64_op_sc_w_rl,
    rose_riscv64_op_sd,
    rose_riscv64_op_sfence_vma,
    rose_riscv64_op_sh,
    rose_riscv64_op_sll,
    rose_riscv64_op_slli,
    rose_riscv64_op_slliw,
    rose_riscv64_op_sllw,
    rose_riscv64_op_slt,
    rose_riscv64_op_slti,
    rose_riscv64_op_sltiu,
    rose_riscv64_op_sltu,
    rose_riscv64_op_sra,
    rose_riscv64_op_srai,
    rose_riscv64_op_sraiw,
    rose_riscv64_op_sraw,
    rose_riscv64_op_sret,
    rose_riscv64_op_srl,
    rose_riscv64_op_srli,
    rose_riscv64_op_srliw,
    rose_riscv64_op_srlw,
    rose_riscv64_op_sub,
    rose_riscv64_op_subw,
    rose_riscv64_op_sw,
    rose_riscv64_op_unimp,
    rose_riscv64_op_uret,
    rose_riscv64_op_wfi,
    rose_riscv64_op_xor,
    rose_riscv64_op_xori,
};

#endif //ROSE_RISCV64INSTRUCTIONENUM_H