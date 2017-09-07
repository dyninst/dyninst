/*
 * This file was automatically generated.  Any manual edits will be lost.
 */

#ifdef  __AARCH32_ISA_GEN_SRC__
#error Generated source file included multiple times.
#endif

#define __AARCH32_ISA_GEN_SRC__

namespace Dyninst {
namespace InstructionAPI {

typedef enum Operand_t {
    OPR_empty,
    OPR_imm_const,
    OPR_imm_label,
    OPR_imm_label_2,
    OPR_imm_label_3,
    OPR_imm_label_4,
    OPR_imm_label_5,
    OPR_imm_label_6,
    OPR_imm_label_7,
    OPR_reg_Dd,
    OPR_reg_Dd_x_,
    OPR_reg_Ddm,
    OPR_reg_Dm,
    OPR_reg_Dm_x_,
    OPR_reg_Dn,
    OPR_reg_Dn_x_,
    OPR_reg_Qd,
    OPR_reg_Qd_2,
    OPR_reg_Qm,
    OPR_reg_Qn,
    OPR_reg_Ra,
    OPR_reg_Rd,
    OPR_reg_RdHi,
    OPR_reg_RdLo,
    OPR_reg_Rd_2,
    OPR_reg_Rm,
    OPR_reg_Rm_2,
    OPR_reg_Rn,
    OPR_reg_Rn_2,
    OPR_reg_Rn_3,
    OPR_reg_Rs,
    OPR_reg_Rt,
    OPR_reg_Rt2,
    OPR_reg_Rt2_2,
    OPR_reg_Rt2_3,
    OPR_reg_Rt_2,
    OPR_reg_Sd,
    OPR_reg_Sdm,
    OPR_reg_Sm,
    OPR_reg_Sm1,
    OPR_reg_Sn,
    OPR_reglist,
    OPR_reglist_2,
    OPR_reglist_3
} Operand_t;

#define MAX_OPR_COUNT 5

typedef enum Condition_t {
    COND_eq = 0x0, // Z == 1
    COND_ne = 0x1, // Z == 0
    COND_cs = 0x2, // C == 1
    COND_cc = 0x3, // C == 0
    COND_mi = 0x4, // N == 1
    COND_pl = 0x5, // N == 0
    COND_vs = 0x6, // V == 1
    COND_vc = 0x7, // V == 0
    COND_hi = 0x8, // C == 1 and Z == 0
    COND_ls = 0x9, // C == 0 or  Z == 1
    COND_ge = 0xa, // N == V
    COND_lt = 0xb, // N != V
    COND_gt = 0xc, // Z == 0 and N == V
    COND_le = 0xd, // Z == 1 or N != V
    COND_al = 0xe, // Always true
    COND_xx = 0xf, // Not a conditional instruction
} Condition_t;

typedef struct Insn_Entry {
    entryID op;
    const char* mnemonic;
    Operand_t operand[MAX_OPR_COUNT];
    uint32_t code;
    uint32_t mask;
} Insn_Entry_t;

static Insn_Entry_t insnTable[] = {
    {aarch32_op_ADC_i_A1, "adc ADC_i_A1", {OPR_reg_Rd, OPR_reg_Rn, OPR_imm_const}, 0x02a00000, 0x0ff00000},
    {aarch32_op_ADCS_i_A1, "adcs ADCS_i_A1", {OPR_reg_Rd, OPR_reg_Rn, OPR_imm_const}, 0x02b00000, 0x0ff00000},
    {aarch32_op_ADC_r_A1_RRX, "adc ADC_r_A1_RRX", {OPR_reg_Rd, OPR_reg_Rn, OPR_reg_Rm}, 0x00a00060, 0x0ff00ff0},
    {aarch32_op_ADC_r_A1, "adc ADC_r_A1", {OPR_reg_Rd, OPR_reg_Rn, OPR_reg_Rm}, 0x00a00000, 0x0ff00010},
    {aarch32_op_ADCS_r_A1_RRX, "adcs ADCS_r_A1_RRX", {OPR_reg_Rd, OPR_reg_Rn, OPR_reg_Rm}, 0x00b00060, 0x0ff00ff0},
    {aarch32_op_ADCS_r_A1, "adcs ADCS_r_A1", {OPR_reg_Rd, OPR_reg_Rn, OPR_reg_Rm}, 0x00b00000, 0x0ff00010},
    {aarch32_op_ADCS_rr_A1, "adcs ADCS_rr_A1", {OPR_reg_Rd, OPR_reg_Rn, OPR_reg_Rm, OPR_reg_Rs}, 0x00b00010, 0x0ff00090},
    {aarch32_op_ADC_rr_A1, "adc ADC_rr_A1", {OPR_reg_Rd, OPR_reg_Rn, OPR_reg_Rm, OPR_reg_Rs}, 0x00a00010, 0x0ff00090},
    {aarch32_op_ADD_ADR_A1, "add ADD_ADR_A1", {OPR_reg_Rd, OPR_imm_const}, 0x028f0000, 0x0fff0000},
    {aarch32_op_ADD_i_A1, "add ADD_i_A1", {OPR_reg_Rd, OPR_reg_Rn, OPR_imm_const}, 0x02800000, 0x0ff00000},
    {aarch32_op_ADDS_i_A1, "adds ADDS_i_A1", {OPR_reg_Rd, OPR_reg_Rn, OPR_imm_const}, 0x02900000, 0x0ff00000},
    {aarch32_op_ADD_r_A1_RRX, "add ADD_r_A1_RRX", {OPR_reg_Rd, OPR_reg_Rn, OPR_reg_Rm}, 0x00800060, 0x0ff00ff0},
    {aarch32_op_ADD_r_A1, "add ADD_r_A1", {OPR_reg_Rd, OPR_reg_Rn, OPR_reg_Rm}, 0x00800000, 0x0ff00010},
    {aarch32_op_ADDS_r_A1_RRX, "adds ADDS_r_A1_RRX", {OPR_reg_Rd, OPR_reg_Rn, OPR_reg_Rm}, 0x00900060, 0x0ff00ff0},
    {aarch32_op_ADDS_r_A1, "adds ADDS_r_A1", {OPR_reg_Rd, OPR_reg_Rn, OPR_reg_Rm}, 0x00900000, 0x0ff00010},
    {aarch32_op_ADDS_rr_A1, "adds ADDS_rr_A1", {OPR_reg_Rd, OPR_reg_Rn, OPR_reg_Rm, OPR_reg_Rs}, 0x00900010, 0x0ff00090},
    {aarch32_op_ADD_rr_A1, "add ADD_rr_A1", {OPR_reg_Rd, OPR_reg_Rn, OPR_reg_Rm, OPR_reg_Rs}, 0x00800010, 0x0ff00090},
    {aarch32_op_ADD_SP_i_A1, "add ADD_SP_i_A1", {OPR_reg_Rd, OPR_imm_const}, 0x028d0000, 0x0fff0000},
    {aarch32_op_ADDS_SP_i_A1, "adds ADDS_SP_i_A1", {OPR_reg_Rd, OPR_imm_const}, 0x029d0000, 0x0fff0000},
    {aarch32_op_ADD_SP_r_A1_RRX, "add ADD_SP_r_A1_RRX", {OPR_reg_Rd, OPR_reg_Rm}, 0x008d0060, 0x0fff0ff0},
    {aarch32_op_ADD_SP_r_A1, "add ADD_SP_r_A1", {OPR_reg_Rd, OPR_reg_Rm}, 0x008d0000, 0x0fff0010},
    {aarch32_op_ADDS_SP_r_A1_RRX, "adds ADDS_SP_r_A1_RRX", {OPR_reg_Rd, OPR_reg_Rm}, 0x009d0060, 0x0fff0ff0},
    {aarch32_op_ADDS_SP_r_A1, "adds ADDS_SP_r_A1", {OPR_reg_Rd, OPR_reg_Rm}, 0x009d0000, 0x0fff0010},
    {aarch32_op_ADR_A1, "adr ADR_A1", {OPR_reg_Rd, OPR_imm_label}, 0x028f0000, 0x0fff0000},
    {aarch32_op_ADR_A2, "adr ADR_A2", {OPR_reg_Rd, OPR_imm_label}, 0x024f0000, 0x0fff0000},
    {aarch32_op_AESD_A1, "aesd AESD_A1", {OPR_reg_Qd, OPR_reg_Qm}, 0xf3b00340, 0xffb30fd0},
    {aarch32_op_AESE_A1, "aese AESE_A1", {OPR_reg_Qd, OPR_reg_Qm}, 0xf3b00300, 0xffb30fd0},
    {aarch32_op_AESIMC_A1, "aesimc AESIMC_A1", {OPR_reg_Qd, OPR_reg_Qm}, 0xf3b003c0, 0xffb30fd0},
    {aarch32_op_AESMC_A1, "aesmc AESMC_A1", {OPR_reg_Qd, OPR_reg_Qm}, 0xf3b00380, 0xffb30fd0},
    {aarch32_op_AND_i_A1, "and AND_i_A1", {OPR_reg_Rd, OPR_reg_Rn, OPR_imm_const}, 0x02000000, 0x0ff00000},
    {aarch32_op_ANDS_i_A1, "ands ANDS_i_A1", {OPR_reg_Rd, OPR_reg_Rn, OPR_imm_const}, 0x02100000, 0x0ff00000},
    {aarch32_op_AND_r_A1_RRX, "and AND_r_A1_RRX", {OPR_reg_Rd, OPR_reg_Rn, OPR_reg_Rm}, 0x00000060, 0x0ff00ff0},
    {aarch32_op_AND_r_A1, "and AND_r_A1", {OPR_reg_Rd, OPR_reg_Rn, OPR_reg_Rm}, 0x00000000, 0x0ff00010},
    {aarch32_op_ANDS_r_A1_RRX, "ands ANDS_r_A1_RRX", {OPR_reg_Rd, OPR_reg_Rn, OPR_reg_Rm}, 0x00100060, 0x0ff00ff0},
    {aarch32_op_ANDS_r_A1, "ands ANDS_r_A1", {OPR_reg_Rd, OPR_reg_Rn, OPR_reg_Rm}, 0x00100000, 0x0ff00010},
    {aarch32_op_ANDS_rr_A1, "ands ANDS_rr_A1", {OPR_reg_Rd, OPR_reg_Rn, OPR_reg_Rm, OPR_reg_Rs}, 0x00100010, 0x0ff00090},
    {aarch32_op_AND_rr_A1, "and AND_rr_A1", {OPR_reg_Rd, OPR_reg_Rn, OPR_reg_Rm, OPR_reg_Rs}, 0x00000010, 0x0ff00090},
    {aarch32_op_ASR_MOV_r_A1, "asr ASR_MOV_r_A1", {OPR_reg_Rd, OPR_reg_Rm}, 0x01a00040, 0x0ff00070},
    {aarch32_op_ASR_MOV_rr_A1, "asr ASR_MOV_rr_A1", {OPR_reg_Rd, OPR_reg_Rm, OPR_reg_Rs}, 0x01a00050, 0x0ff000f0},
    {aarch32_op_ASRS_MOVS_r_A1, "asrs ASRS_MOVS_r_A1", {OPR_reg_Rd, OPR_reg_Rm}, 0x01b00040, 0x0ff00070},
    {aarch32_op_ASRS_MOVS_rr_A1, "asrs ASRS_MOVS_rr_A1", {OPR_reg_Rd, OPR_reg_Rm, OPR_reg_Rs}, 0x01b00050, 0x0ff000f0},
    {aarch32_op_B_A1, "b B_A1", {OPR_imm_label_2}, 0x0a000000, 0x0f000000},
    {aarch32_op_BFC_A1, "bfc BFC_A1", {OPR_reg_Rd}, 0x07c0001f, 0x0fe0007f},
    {aarch32_op_BFI_A1, "bfi BFI_A1", {OPR_reg_Rd, OPR_reg_Rn_2}, 0x07c00010, 0x0fe00070},
    {aarch32_op_BIC_i_A1, "bic BIC_i_A1", {OPR_reg_Rd, OPR_reg_Rn, OPR_imm_const}, 0x03c00000, 0x0ff00000},
    {aarch32_op_BICS_i_A1, "bics BICS_i_A1", {OPR_reg_Rd, OPR_reg_Rn, OPR_imm_const}, 0x03d00000, 0x0ff00000},
    {aarch32_op_BIC_r_A1_RRX, "bic BIC_r_A1_RRX", {OPR_reg_Rd, OPR_reg_Rn, OPR_reg_Rm}, 0x01c00060, 0x0ff00ff0},
    {aarch32_op_BIC_r_A1, "bic BIC_r_A1", {OPR_reg_Rd, OPR_reg_Rn, OPR_reg_Rm}, 0x01c00000, 0x0ff00010},
    {aarch32_op_BICS_r_A1_RRX, "bics BICS_r_A1_RRX", {OPR_reg_Rd, OPR_reg_Rn, OPR_reg_Rm}, 0x01d00060, 0x0ff00ff0},
    {aarch32_op_BICS_r_A1, "bics BICS_r_A1", {OPR_reg_Rd, OPR_reg_Rn, OPR_reg_Rm}, 0x01d00000, 0x0ff00010},
    {aarch32_op_BICS_rr_A1, "bics BICS_rr_A1", {OPR_reg_Rd, OPR_reg_Rn, OPR_reg_Rm, OPR_reg_Rs}, 0x01d00010, 0x0ff00090},
    {aarch32_op_BIC_rr_A1, "bic BIC_rr_A1", {OPR_reg_Rd, OPR_reg_Rn, OPR_reg_Rm, OPR_reg_Rs}, 0x01c00010, 0x0ff00090},
    {aarch32_op_BKPT_A1, "bkpt BKPT_A1", {}, 0x01200070, 0x0ff000f0},
    {aarch32_op_BL_i_A1, "bl BL_i_A1", {OPR_imm_label}, 0x0b000000, 0x0f000000},
    {aarch32_op_BL_i_A2, "blx BL_i_A2", {OPR_imm_label_3}, 0xfa000000, 0xfe000000},
    {aarch32_op_BLX_r_A1, "blx BLX_r_A1", {OPR_reg_Rm}, 0x01200030, 0x0ff000f0},
    {aarch32_op_BX_A1, "bx BX_A1", {OPR_reg_Rm}, 0x01200010, 0x0ff000f0},
    {aarch32_op_BXJ_A1, "bxj BXJ_A1", {OPR_reg_Rm}, 0x01200020, 0x0ff000f0},
    {aarch32_op_CLREX_A1, "clrex CLREX_A1", {}, 0xf5700010, 0xfff000f0},
    {aarch32_op_CLZ_A1, "clz CLZ_A1", {OPR_reg_Rd, OPR_reg_Rm}, 0x01600010, 0x0ff000f0},
    {aarch32_op_CMN_i_A1, "cmn CMN_i_A1", {OPR_reg_Rn, OPR_imm_const}, 0x03700000, 0x0ff00000},
    {aarch32_op_CMN_r_A1_RRX, "cmn CMN_r_A1_RRX", {OPR_reg_Rn, OPR_reg_Rm}, 0x01700060, 0x0ff00ff0},
    {aarch32_op_CMN_r_A1, "cmn CMN_r_A1", {OPR_reg_Rn, OPR_reg_Rm}, 0x01700000, 0x0ff00010},
    {aarch32_op_CMN_rr_A1, "cmn CMN_rr_A1", {OPR_reg_Rn, OPR_reg_Rm, OPR_reg_Rs}, 0x01700010, 0x0ff00090},
    {aarch32_op_CMP_i_A1, "cmp CMP_i_A1", {OPR_reg_Rn, OPR_imm_const}, 0x03500000, 0x0ff00000},
    {aarch32_op_CMP_r_A1_RRX, "cmp CMP_r_A1_RRX", {OPR_reg_Rn, OPR_reg_Rm}, 0x01500060, 0x0ff00ff0},
    {aarch32_op_CMP_r_A1, "cmp CMP_r_A1", {OPR_reg_Rn, OPR_reg_Rm}, 0x01500000, 0x0ff00010},
    {aarch32_op_CMP_rr_A1, "cmp CMP_rr_A1", {OPR_reg_Rn, OPR_reg_Rm, OPR_reg_Rs}, 0x01500010, 0x0ff00090},
    {aarch32_op_CPS_A1_AS, "cps CPS_A1_AS", {}, 0xf1020000, 0xffff0020},
    {aarch32_op_CPSID_A1_AS, "cpsid CPSID_A1_AS", {}, 0xf10c0000, 0xffff0020},
    {aarch32_op_CPSID_A1_ASM, "cpsid CPSID_A1_ASM", {}, 0xf10e0000, 0xffff0020},
    {aarch32_op_CPSIE_A1_AS, "cpsie CPSIE_A1_AS", {}, 0xf1080000, 0xffff0020},
    {aarch32_op_CPSIE_A1_ASM, "cpsie CPSIE_A1_ASM", {}, 0xf10a0000, 0xffff0020},
    {aarch32_op_CRC32B_A1, "crc32b CRC32B_A1", {OPR_reg_Rd, OPR_reg_Rn, OPR_reg_Rm}, 0x01000040, 0x0ff002f0},
    {aarch32_op_CRC32H_A1, "crc32h CRC32H_A1", {OPR_reg_Rd, OPR_reg_Rn, OPR_reg_Rm}, 0x01200040, 0x0ff002f0},
    {aarch32_op_CRC32W_A1, "crc32w CRC32W_A1", {OPR_reg_Rd, OPR_reg_Rn, OPR_reg_Rm}, 0x01400040, 0x0ff002f0},
    {aarch32_op_CRC32CB_A1, "crc32cb CRC32CB_A1", {OPR_reg_Rd, OPR_reg_Rn, OPR_reg_Rm}, 0x01000240, 0x0ff002f0},
    {aarch32_op_CRC32CH_A1, "crc32ch CRC32CH_A1", {OPR_reg_Rd, OPR_reg_Rn, OPR_reg_Rm}, 0x01200240, 0x0ff002f0},
    {aarch32_op_CRC32CW_A1, "crc32cw CRC32CW_A1", {OPR_reg_Rd, OPR_reg_Rn, OPR_reg_Rm}, 0x01400240, 0x0ff002f0},
    {aarch32_op_DBG_A1, "dbg DBG_A1", {}, 0x032000f0, 0x0fff00f0},
    {aarch32_op_DMB_A1, "dmb DMB_A1", {}, 0xf5700050, 0xfff000f0},
    {aarch32_op_DSB_A1, "dsb DSB_A1", {}, 0xf5700040, 0xfff000f0},
    {aarch32_op_EOR_i_A1, "eor EOR_i_A1", {OPR_reg_Rd, OPR_reg_Rn, OPR_imm_const}, 0x02200000, 0x0ff00000},
    {aarch32_op_EORS_i_A1, "eors EORS_i_A1", {OPR_reg_Rd, OPR_reg_Rn, OPR_imm_const}, 0x02300000, 0x0ff00000},
    {aarch32_op_EOR_r_A1_RRX, "eor EOR_r_A1_RRX", {OPR_reg_Rd, OPR_reg_Rn, OPR_reg_Rm}, 0x00200060, 0x0ff00ff0},
    {aarch32_op_EOR_r_A1, "eor EOR_r_A1", {OPR_reg_Rd, OPR_reg_Rn, OPR_reg_Rm}, 0x00200000, 0x0ff00010},
    {aarch32_op_EORS_r_A1_RRX, "eors EORS_r_A1_RRX", {OPR_reg_Rd, OPR_reg_Rn, OPR_reg_Rm}, 0x00300060, 0x0ff00ff0},
    {aarch32_op_EORS_r_A1, "eors EORS_r_A1", {OPR_reg_Rd, OPR_reg_Rn, OPR_reg_Rm}, 0x00300000, 0x0ff00010},
    {aarch32_op_EORS_rr_A1, "eors EORS_rr_A1", {OPR_reg_Rd, OPR_reg_Rn, OPR_reg_Rm, OPR_reg_Rs}, 0x00300010, 0x0ff00090},
    {aarch32_op_EOR_rr_A1, "eor EOR_rr_A1", {OPR_reg_Rd, OPR_reg_Rn, OPR_reg_Rm, OPR_reg_Rs}, 0x00200010, 0x0ff00090},
    {aarch32_op_ERET_A1, "eret ERET_A1", {}, 0x01600060, 0x0ff000f0},
    {aarch32_op_ESB_A1, "esb ESB_A1", {}, 0x03200010, 0x0fff00ff},
    {aarch32_op_FLDMDBX_A1, "fldmdbx FLDMDBX_A1", {OPR_reg_Rn}, 0x0d300b01, 0x0fb00f01},
    {aarch32_op_FLDMIAX_A1, "fldmiax FLDMIAX_A1", {OPR_reg_Rn}, 0x0c900b01, 0x0f900f01},
    {aarch32_op_FSTMDBX_A1, "fstmdbx FSTMDBX_A1", {OPR_reg_Rn}, 0x0d200b01, 0x0fb00f01},
    {aarch32_op_FSTMIAX_A1, "fstmiax FSTMIAX_A1", {OPR_reg_Rn}, 0x0c800b01, 0x0f900f01},
    {aarch32_op_HLT_A1, "hlt HLT_A1", {}, 0x01000070, 0x0ff000f0},
    {aarch32_op_HVC_A1, "hvc HVC_A1", {}, 0x01400070, 0x0ff000f0},
    {aarch32_op_ISB_A1, "isb ISB_A1", {}, 0xf5700060, 0xfff000f0},
    {aarch32_op_LDA_A1, "lda LDA_A1", {OPR_reg_Rt, OPR_reg_Rn}, 0x01900090, 0x0ff003f0},
    {aarch32_op_LDAB_A1, "ldab LDAB_A1", {OPR_reg_Rt, OPR_reg_Rn}, 0x01d00090, 0x0ff003f0},
    {aarch32_op_LDAEX_A1, "ldaex LDAEX_A1", {OPR_reg_Rt, OPR_reg_Rn}, 0x01900290, 0x0ff003f0},
    {aarch32_op_LDAEXB_A1, "ldaexb LDAEXB_A1", {OPR_reg_Rt, OPR_reg_Rn}, 0x01d00290, 0x0ff003f0},
    {aarch32_op_LDAEXD_A1, "ldaexd LDAEXD_A1", {OPR_reg_Rt, OPR_reg_Rt2, OPR_reg_Rn}, 0x01b00290, 0x0ff003f0},
    {aarch32_op_LDAEXH_A1, "ldaexh LDAEXH_A1", {OPR_reg_Rt, OPR_reg_Rn}, 0x01f00290, 0x0ff003f0},
    {aarch32_op_LDAH_A1, "ldah LDAH_A1", {OPR_reg_Rt, OPR_reg_Rn}, 0x01f00090, 0x0ff003f0},
    {aarch32_op_LDC_i_A1_off, "ldc LDC_i_A1_off", {OPR_reg_Rn}, 0x0d105e00, 0x0f70ff00},
    {aarch32_op_LDC_i_A1_post, "ldc LDC_i_A1_post", {OPR_reg_Rn_3}, 0x0c305e00, 0x0f70ff00},
    {aarch32_op_LDC_i_A1_pre, "ldc LDC_i_A1_pre", {OPR_reg_Rn}, 0x0d305e00, 0x0f70ff00},
    {aarch32_op_LDC_i_A1_unind, "ldc LDC_i_A1_unind", {OPR_reg_Rn}, 0x0c905e00, 0x0ff0ff00},
    {aarch32_op_LDC_l_A1, "ldc LDC_l_A1", {OPR_imm_label_4}, 0x0c1f5e00, 0x0e1fff00},
    {aarch32_op_LDM_A1, "ldm LDM_A1", {OPR_reg_Rn, OPR_reglist}, 0x08900000, 0x0fd00000},
    {aarch32_op_LDM_e_A1_AS, "ldm LDM_e_A1_AS", {OPR_reg_Rn, OPR_reglist_2}, 0x08508000, 0x0e508000},
    {aarch32_op_LDM_u_A1_AS, "ldm LDM_u_A1_AS", {OPR_reg_Rn, OPR_reglist}, 0x08500000, 0x0e508000},
    {aarch32_op_LDMDA_A1, "ldm LDMDA_A1", {OPR_reg_Rn, OPR_reglist}, 0x08100000, 0x0fd00000},
    {aarch32_op_LDMDB_A1, "ldm LDMDB_A1", {OPR_reg_Rn, OPR_reglist}, 0x09100000, 0x0fd00000},
    {aarch32_op_LDMIB_A1, "ldm LDMIB_A1", {OPR_reg_Rn, OPR_reglist}, 0x09900000, 0x0fd00000},
    {aarch32_op_LDR_i_A1_off, "ldr LDR_i_A1_off", {OPR_reg_Rt, OPR_reg_Rn}, 0x05100000, 0x0f700000},
    {aarch32_op_LDR_i_A1_post, "ldr LDR_i_A1_post", {OPR_reg_Rt, OPR_reg_Rn}, 0x04100000, 0x0f700000},
    {aarch32_op_LDR_i_A1_pre, "ldr LDR_i_A1_pre", {OPR_reg_Rt, OPR_reg_Rn}, 0x05300000, 0x0f700000},
    {aarch32_op_LDR_l_A1, "ldr LDR_l_A1", {OPR_reg_Rt, OPR_imm_label_5}, 0x041f0000, 0x0e5f0000},
    {aarch32_op_LDR_r_A1_off, "ldr LDR_r_A1_off", {OPR_reg_Rt, OPR_reg_Rn, OPR_reg_Rm}, 0x07100000, 0x0f700010},
    {aarch32_op_LDR_r_A1_post, "ldr LDR_r_A1_post", {OPR_reg_Rt, OPR_reg_Rn, OPR_reg_Rm}, 0x06100000, 0x0f700010},
    {aarch32_op_LDR_r_A1_pre, "ldr LDR_r_A1_pre", {OPR_reg_Rt, OPR_reg_Rn, OPR_reg_Rm}, 0x07300000, 0x0f700010},
    {aarch32_op_LDRB_i_A1_off, "ldrb LDRB_i_A1_off", {OPR_reg_Rt, OPR_reg_Rn}, 0x05500000, 0x0f700000},
    {aarch32_op_LDRB_i_A1_post, "ldrb LDRB_i_A1_post", {OPR_reg_Rt, OPR_reg_Rn}, 0x04500000, 0x0f700000},
    {aarch32_op_LDRB_i_A1_pre, "ldrb LDRB_i_A1_pre", {OPR_reg_Rt, OPR_reg_Rn}, 0x05700000, 0x0f700000},
    {aarch32_op_LDRB_l_A1, "ldrb LDRB_l_A1", {OPR_reg_Rt, OPR_imm_label}, 0x045f0000, 0x0e5f0000},
    {aarch32_op_LDRB_r_A1_off, "ldrb LDRB_r_A1_off", {OPR_reg_Rt, OPR_reg_Rn, OPR_reg_Rm}, 0x07500000, 0x0f700010},
    {aarch32_op_LDRB_r_A1_post, "ldrb LDRB_r_A1_post", {OPR_reg_Rt, OPR_reg_Rn, OPR_reg_Rm}, 0x06500000, 0x0f700010},
    {aarch32_op_LDRB_r_A1_pre, "ldrb LDRB_r_A1_pre", {OPR_reg_Rt, OPR_reg_Rn, OPR_reg_Rm}, 0x07700000, 0x0f700010},
    {aarch32_op_LDRBT_A1, "ldrbt LDRBT_A1", {OPR_reg_Rt, OPR_reg_Rn}, 0x04700000, 0x0f700000},
    {aarch32_op_LDRBT_A2, "ldrbt LDRBT_A2", {OPR_reg_Rt, OPR_reg_Rn, OPR_reg_Rm}, 0x06700000, 0x0f700010},
    {aarch32_op_LDRD_i_A1_off, "ldrd LDRD_i_A1_off", {OPR_reg_Rt, OPR_reg_Rt2, OPR_reg_Rn}, 0x014000d0, 0x0f7000f0},
    {aarch32_op_LDRD_i_A1_post, "ldrd LDRD_i_A1_post", {OPR_reg_Rt, OPR_reg_Rt2, OPR_reg_Rn}, 0x004000d0, 0x0f7000f0},
    {aarch32_op_LDRD_i_A1_pre, "ldrd LDRD_i_A1_pre", {OPR_reg_Rt, OPR_reg_Rt2, OPR_reg_Rn}, 0x016000d0, 0x0f7000f0},
    {aarch32_op_LDRD_l_A1, "ldrd LDRD_l_A1", {OPR_reg_Rt, OPR_reg_Rt2, OPR_imm_label_6}, 0x004f00d0, 0x0e5f00f0},
    {aarch32_op_LDRD_r_A1_off, "ldrd LDRD_r_A1_off", {OPR_reg_Rt, OPR_reg_Rt2, OPR_reg_Rn, OPR_reg_Rm}, 0x010000d0, 0x0f7000f0},
    {aarch32_op_LDRD_r_A1_post, "ldrd LDRD_r_A1_post", {OPR_reg_Rt, OPR_reg_Rt2, OPR_reg_Rn, OPR_reg_Rm}, 0x000000d0, 0x0f7000f0},
    {aarch32_op_LDRD_r_A1_pre, "ldrd LDRD_r_A1_pre", {OPR_reg_Rt, OPR_reg_Rt2, OPR_reg_Rn, OPR_reg_Rm}, 0x012000d0, 0x0f7000f0},
    {aarch32_op_LDREX_A1, "ldrex LDREX_A1", {OPR_reg_Rt, OPR_reg_Rn}, 0x01900390, 0x0ff003f0},
    {aarch32_op_LDREXB_A1, "ldrexb LDREXB_A1", {OPR_reg_Rt, OPR_reg_Rn}, 0x01d00390, 0x0ff003f0},
    {aarch32_op_LDREXD_A1, "ldrexd LDREXD_A1", {OPR_reg_Rt, OPR_reg_Rt2, OPR_reg_Rn}, 0x01b00390, 0x0ff003f0},
    {aarch32_op_LDREXH_A1, "ldrexh LDREXH_A1", {OPR_reg_Rt, OPR_reg_Rn}, 0x01f00390, 0x0ff003f0},
    {aarch32_op_LDRH_i_A1_off, "ldrh LDRH_i_A1_off", {OPR_reg_Rt, OPR_reg_Rn}, 0x015000b0, 0x0f7000f0},
    {aarch32_op_LDRH_i_A1_post, "ldrh LDRH_i_A1_post", {OPR_reg_Rt, OPR_reg_Rn}, 0x005000b0, 0x0f7000f0},
    {aarch32_op_LDRH_i_A1_pre, "ldrh LDRH_i_A1_pre", {OPR_reg_Rt, OPR_reg_Rn}, 0x017000b0, 0x0f7000f0},
    {aarch32_op_LDRH_l_A1, "ldrh LDRH_l_A1", {OPR_reg_Rt, OPR_imm_label}, 0x005f00b0, 0x0e5f00f0},
    {aarch32_op_LDRH_r_A1_off, "ldrh LDRH_r_A1_off", {OPR_reg_Rt, OPR_reg_Rn, OPR_reg_Rm}, 0x011000b0, 0x0f7000f0},
    {aarch32_op_LDRH_r_A1_post, "ldrh LDRH_r_A1_post", {OPR_reg_Rt, OPR_reg_Rn, OPR_reg_Rm}, 0x001000b0, 0x0f7000f0},
    {aarch32_op_LDRH_r_A1_pre, "ldrh LDRH_r_A1_pre", {OPR_reg_Rt, OPR_reg_Rn, OPR_reg_Rm}, 0x013000b0, 0x0f7000f0},
    {aarch32_op_LDRHT_A1, "ldrht LDRHT_A1", {OPR_reg_Rt, OPR_reg_Rn}, 0x007000b0, 0x0f7000f0},
    {aarch32_op_LDRHT_A2, "ldrht LDRHT_A2", {OPR_reg_Rt, OPR_reg_Rn, OPR_reg_Rm}, 0x003000b0, 0x0f7000f0},
    {aarch32_op_LDRSB_i_A1_off, "ldrsb LDRSB_i_A1_off", {OPR_reg_Rt, OPR_reg_Rn}, 0x015000d0, 0x0f7000f0},
    {aarch32_op_LDRSB_i_A1_post, "ldrsb LDRSB_i_A1_post", {OPR_reg_Rt, OPR_reg_Rn}, 0x005000d0, 0x0f7000f0},
    {aarch32_op_LDRSB_i_A1_pre, "ldrsb LDRSB_i_A1_pre", {OPR_reg_Rt, OPR_reg_Rn}, 0x017000d0, 0x0f7000f0},
    {aarch32_op_LDRSB_l_A1, "ldrsb LDRSB_l_A1", {OPR_reg_Rt, OPR_imm_label}, 0x005f00d0, 0x0e5f00f0},
    {aarch32_op_LDRSB_r_A1_off, "ldrsb LDRSB_r_A1_off", {OPR_reg_Rt, OPR_reg_Rn, OPR_reg_Rm}, 0x011000d0, 0x0f7000f0},
    {aarch32_op_LDRSB_r_A1_post, "ldrsb LDRSB_r_A1_post", {OPR_reg_Rt, OPR_reg_Rn, OPR_reg_Rm}, 0x001000d0, 0x0f7000f0},
    {aarch32_op_LDRSB_r_A1_pre, "ldrsb LDRSB_r_A1_pre", {OPR_reg_Rt, OPR_reg_Rn, OPR_reg_Rm}, 0x013000d0, 0x0f7000f0},
    {aarch32_op_LDRSBT_A1, "ldrsbt LDRSBT_A1", {OPR_reg_Rt, OPR_reg_Rn}, 0x007000d0, 0x0f7000f0},
    {aarch32_op_LDRSBT_A2, "ldrsbt LDRSBT_A2", {OPR_reg_Rt, OPR_reg_Rn, OPR_reg_Rm}, 0x003000d0, 0x0f7000f0},
    {aarch32_op_LDRSH_i_A1_off, "ldrsh LDRSH_i_A1_off", {OPR_reg_Rt, OPR_reg_Rn}, 0x015000f0, 0x0f7000f0},
    {aarch32_op_LDRSH_i_A1_post, "ldrsh LDRSH_i_A1_post", {OPR_reg_Rt, OPR_reg_Rn}, 0x005000f0, 0x0f7000f0},
    {aarch32_op_LDRSH_i_A1_pre, "ldrsh LDRSH_i_A1_pre", {OPR_reg_Rt, OPR_reg_Rn}, 0x017000f0, 0x0f7000f0},
    {aarch32_op_LDRSH_l_A1, "ldrsh LDRSH_l_A1", {OPR_reg_Rt, OPR_imm_label}, 0x005f00f0, 0x0e5f00f0},
    {aarch32_op_LDRSH_r_A1_off, "ldrsh LDRSH_r_A1_off", {OPR_reg_Rt, OPR_reg_Rn, OPR_reg_Rm}, 0x011000f0, 0x0f7000f0},
    {aarch32_op_LDRSH_r_A1_post, "ldrsh LDRSH_r_A1_post", {OPR_reg_Rt, OPR_reg_Rn, OPR_reg_Rm}, 0x001000f0, 0x0f7000f0},
    {aarch32_op_LDRSH_r_A1_pre, "ldrsh LDRSH_r_A1_pre", {OPR_reg_Rt, OPR_reg_Rn, OPR_reg_Rm}, 0x013000f0, 0x0f7000f0},
    {aarch32_op_LDRSHT_A1, "ldrsht LDRSHT_A1", {OPR_reg_Rt, OPR_reg_Rn}, 0x007000f0, 0x0f7000f0},
    {aarch32_op_LDRSHT_A2, "ldrsht LDRSHT_A2", {OPR_reg_Rt, OPR_reg_Rn, OPR_reg_Rm}, 0x003000f0, 0x0f7000f0},
    {aarch32_op_LDRT_A1, "ldrt LDRT_A1", {OPR_reg_Rt, OPR_reg_Rn}, 0x04300000, 0x0f700000},
    {aarch32_op_LDRT_A2, "ldrt LDRT_A2", {OPR_reg_Rt, OPR_reg_Rn, OPR_reg_Rm}, 0x06300000, 0x0f700010},
    {aarch32_op_LSL_MOV_r_A1, "lsl LSL_MOV_r_A1", {OPR_reg_Rd, OPR_reg_Rm}, 0x01a00000, 0x0ff00070},
    {aarch32_op_LSL_MOV_rr_A1, "lsl LSL_MOV_rr_A1", {OPR_reg_Rd, OPR_reg_Rm, OPR_reg_Rs}, 0x01a00010, 0x0ff000f0},
    {aarch32_op_LSLS_MOVS_r_A1, "lsls LSLS_MOVS_r_A1", {OPR_reg_Rd, OPR_reg_Rm}, 0x01b00000, 0x0ff00070},
    {aarch32_op_LSLS_MOVS_rr_A1, "lsls LSLS_MOVS_rr_A1", {OPR_reg_Rd, OPR_reg_Rm, OPR_reg_Rs}, 0x01b00010, 0x0ff000f0},
    {aarch32_op_LSR_MOV_r_A1, "lsr LSR_MOV_r_A1", {OPR_reg_Rd, OPR_reg_Rm}, 0x01a00020, 0x0ff00070},
    {aarch32_op_LSR_MOV_rr_A1, "lsr LSR_MOV_rr_A1", {OPR_reg_Rd, OPR_reg_Rm, OPR_reg_Rs}, 0x01a00030, 0x0ff000f0},
    {aarch32_op_LSRS_MOVS_r_A1, "lsrs LSRS_MOVS_r_A1", {OPR_reg_Rd, OPR_reg_Rm}, 0x01b00020, 0x0ff00070},
    {aarch32_op_LSRS_MOVS_rr_A1, "lsrs LSRS_MOVS_rr_A1", {OPR_reg_Rd, OPR_reg_Rm, OPR_reg_Rs}, 0x01b00030, 0x0ff000f0},
    {aarch32_op_MCR_A1, "mcr MCR_A1", {OPR_reg_Rt}, 0x0e000e10, 0x0f100e10},
    {aarch32_op_MCRR_A1, "mcrr MCRR_A1", {OPR_reg_Rt, OPR_reg_Rt2_2}, 0x0c400e00, 0x0ff00e00},
    {aarch32_op_MLAS_A1, "mlas MLAS_A1", {OPR_reg_Rd_2, OPR_reg_Rn, OPR_reg_Rm_2, OPR_reg_Ra}, 0x00300090, 0x0ff000f0},
    {aarch32_op_MLA_A1, "mla MLA_A1", {OPR_reg_Rd, OPR_reg_Rn, OPR_reg_Rm, OPR_reg_Ra}, 0x00200090, 0x0ff000f0},
    {aarch32_op_MLS_A1, "mls MLS_A1", {OPR_reg_Rd, OPR_reg_Rn, OPR_reg_Rm, OPR_reg_Ra}, 0x00600090, 0x0ff000f0},
    {aarch32_op_MOV_i_A1, "mov MOV_i_A1", {OPR_reg_Rd, OPR_imm_const}, 0x03a00000, 0x0ff00000},
    {aarch32_op_MOVS_i_A1, "movs MOVS_i_A1", {OPR_reg_Rd, OPR_imm_const}, 0x03b00000, 0x0ff00000},
    {aarch32_op_MOV_i_A2, "movw MOV_i_A2", {OPR_reg_Rd}, 0x03000000, 0x0ff00000},
    {aarch32_op_MOV_r_A1_RRX, "mov MOV_r_A1_RRX", {OPR_reg_Rd, OPR_reg_Rm}, 0x01a00060, 0x0ff00ff0},
    {aarch32_op_MOV_r_A1, "mov MOV_r_A1", {OPR_reg_Rd, OPR_reg_Rm}, 0x01a00000, 0x0ff00010},
    {aarch32_op_MOVS_r_A1_RRX, "movs MOVS_r_A1_RRX", {OPR_reg_Rd, OPR_reg_Rm}, 0x01b00060, 0x0ff00ff0},
    {aarch32_op_MOVS_r_A1, "movs MOVS_r_A1", {OPR_reg_Rd, OPR_reg_Rm}, 0x01b00000, 0x0ff00010},
    {aarch32_op_MOVS_rr_A1, "movs MOVS_rr_A1", {OPR_reg_Rd, OPR_reg_Rm, OPR_reg_Rs}, 0x01b00010, 0x0ff00090},
    {aarch32_op_MOV_rr_A1, "mov MOV_rr_A1", {OPR_reg_Rd, OPR_reg_Rm, OPR_reg_Rs}, 0x01a00010, 0x0ff00090},
    {aarch32_op_MOVT_A1, "movt MOVT_A1", {OPR_reg_Rd}, 0x03400000, 0x0ff00000},
    {aarch32_op_MRC_A1, "mrc MRC_A1", {OPR_reg_Rt}, 0x0e100e10, 0x0f100e10},
    {aarch32_op_MRRC_A1, "mrrc MRRC_A1", {OPR_reg_Rt, OPR_reg_Rt2}, 0x0c500e00, 0x0ff00e00},
    {aarch32_op_MRS_A1_AS, "mrs MRS_A1_AS", {OPR_reg_Rd}, 0x01000000, 0x0fb002f0},
    {aarch32_op_MRS_br_A1_AS, "mrs MRS_br_A1_AS", {OPR_reg_Rd}, 0x01000200, 0x0fb002f0},
    {aarch32_op_MSR_br_A1_AS, "msr MSR_br_A1_AS", {OPR_reg_Rn}, 0x01200200, 0x0fb002f0},
    {aarch32_op_MSR_i_A1_AS, "msr MSR_i_A1_AS", {}, 0x03000000, 0x0f800000},
    {aarch32_op_MSR_r_A1_AS, "msr MSR_r_A1_AS", {OPR_reg_Rn}, 0x01200000, 0x0fb002f0},
    {aarch32_op_MULS_A1, "muls MULS_A1", {OPR_reg_Rd, OPR_reg_Rn, OPR_reg_Rm}, 0x00100090, 0x0ff000f0},
    {aarch32_op_MUL_A1, "mul MUL_A1", {OPR_reg_Rd, OPR_reg_Rn, OPR_reg_Rm}, 0x00000090, 0x0ff000f0},
    {aarch32_op_MVN_i_A1, "mvn MVN_i_A1", {OPR_reg_Rd, OPR_imm_const}, 0x03e00000, 0x0ff00000},
    {aarch32_op_MVNS_i_A1, "mvns MVNS_i_A1", {OPR_reg_Rd, OPR_imm_const}, 0x03f00000, 0x0ff00000},
    {aarch32_op_MVN_r_A1_RRX, "mvn MVN_r_A1_RRX", {OPR_reg_Rd, OPR_reg_Rm}, 0x01e00060, 0x0ff00ff0},
    {aarch32_op_MVN_r_A1, "mvn MVN_r_A1", {OPR_reg_Rd, OPR_reg_Rm}, 0x01e00000, 0x0ff00010},
    {aarch32_op_MVNS_r_A1_RRX, "mvns MVNS_r_A1_RRX", {OPR_reg_Rd, OPR_reg_Rm}, 0x01f00060, 0x0ff00ff0},
    {aarch32_op_MVNS_r_A1, "mvns MVNS_r_A1", {OPR_reg_Rd, OPR_reg_Rm}, 0x01f00000, 0x0ff00010},
    {aarch32_op_MVNS_rr_A1, "mvns MVNS_rr_A1", {OPR_reg_Rd, OPR_reg_Rm, OPR_reg_Rs}, 0x01f00010, 0x0ff00090},
    {aarch32_op_MVN_rr_A1, "mvn MVN_rr_A1", {OPR_reg_Rd, OPR_reg_Rm, OPR_reg_Rs}, 0x01e00010, 0x0ff00090},
    {aarch32_op_NOP_A1, "nop NOP_A1", {}, 0x03200000, 0x0fff00ff},
    {aarch32_op_ORR_i_A1, "orr ORR_i_A1", {OPR_reg_Rd, OPR_reg_Rn, OPR_imm_const}, 0x03800000, 0x0ff00000},
    {aarch32_op_ORRS_i_A1, "orrs ORRS_i_A1", {OPR_reg_Rd, OPR_reg_Rn, OPR_imm_const}, 0x03900000, 0x0ff00000},
    {aarch32_op_ORR_r_A1_RRX, "orr ORR_r_A1_RRX", {OPR_reg_Rd, OPR_reg_Rn, OPR_reg_Rm}, 0x01800060, 0x0ff00ff0},
    {aarch32_op_ORR_r_A1, "orr ORR_r_A1", {OPR_reg_Rd, OPR_reg_Rn, OPR_reg_Rm}, 0x01800000, 0x0ff00010},
    {aarch32_op_ORRS_r_A1_RRX, "orrs ORRS_r_A1_RRX", {OPR_reg_Rd, OPR_reg_Rn, OPR_reg_Rm}, 0x01900060, 0x0ff00ff0},
    {aarch32_op_ORRS_r_A1, "orrs ORRS_r_A1", {OPR_reg_Rd, OPR_reg_Rn, OPR_reg_Rm}, 0x01900000, 0x0ff00010},
    {aarch32_op_ORRS_rr_A1, "orrs ORRS_rr_A1", {OPR_reg_Rd, OPR_reg_Rn, OPR_reg_Rm, OPR_reg_Rs}, 0x01900010, 0x0ff00090},
    {aarch32_op_ORR_rr_A1, "orr ORR_rr_A1", {OPR_reg_Rd, OPR_reg_Rn, OPR_reg_Rm, OPR_reg_Rs}, 0x01800010, 0x0ff00090},
    {aarch32_op_PKHBT_A1, "pkhbt PKHBT_A1", {OPR_reg_Rd, OPR_reg_Rn, OPR_reg_Rm}, 0x06800010, 0x0ff00070},
    {aarch32_op_PKHTB_A1, "pkhtb PKHTB_A1", {OPR_reg_Rd, OPR_reg_Rn, OPR_reg_Rm}, 0x06800050, 0x0ff00070},
    {aarch32_op_PLD_i_A1, "pld PLD_i_A1", {OPR_reg_Rn}, 0xf5500000, 0xff700000},
    {aarch32_op_PLDW_i_A1, "pldw PLDW_i_A1", {OPR_reg_Rn}, 0xf5100000, 0xff700000},
    {aarch32_op_PLD_l_A1, "pld PLD_l_A1", {OPR_imm_label}, 0xf51f0000, 0xff3f0000},
    {aarch32_op_PLD_r_A1, "pld PLD_r_A1", {OPR_reg_Rn, OPR_reg_Rm}, 0xf7500000, 0xff700010},
    {aarch32_op_PLD_r_A1_RRX, "pld PLD_r_A1_RRX", {OPR_reg_Rn, OPR_reg_Rm}, 0xf7500060, 0xff700ff0},
    {aarch32_op_PLDW_r_A1, "pldw PLDW_r_A1", {OPR_reg_Rn, OPR_reg_Rm}, 0xf7100000, 0xff700010},
    {aarch32_op_PLDW_r_A1_RRX, "pldw PLDW_r_A1_RRX", {OPR_reg_Rn, OPR_reg_Rm}, 0xf7100060, 0xff700ff0},
    {aarch32_op_PLI_i_A1, "pli PLI_i_A1", {OPR_reg_Rn}, 0xf4500000, 0xff700000},
    {aarch32_op_PLI_r_A1_RRX, "pli PLI_r_A1_RRX", {OPR_reg_Rn, OPR_reg_Rm}, 0xf6500060, 0xff700ff0},
    {aarch32_op_PLI_r_A1, "pli PLI_r_A1", {OPR_reg_Rn, OPR_reg_Rm}, 0xf6500000, 0xff700010},
    {aarch32_op_POP_LDM_A1, "pop POP_LDM_A1", {OPR_reglist}, 0x08bd0000, 0x0fff0000},
    {aarch32_op_POP_LDR_i_A1_post, "pop POP_LDR_i_A1_post", {OPR_reg_Rn}, 0x049d0004, 0x0fff0fff},
    {aarch32_op_PUSH_STMDB_A1, "push PUSH_STMDB_A1", {OPR_reglist_3}, 0x092d0000, 0x0fff0000},
    {aarch32_op_PUSH_STR_i_A1_pre, "push PUSH_STR_i_A1_pre", {OPR_reg_Rn}, 0x052d0004, 0x0fff0fff},
    {aarch32_op_QADD_A1, "qadd QADD_A1", {OPR_reg_Rd, OPR_reg_Rm, OPR_reg_Rn}, 0x01000050, 0x0ff000f0},
    {aarch32_op_QADD16_A1, "qadd16 QADD16_A1", {OPR_reg_Rd, OPR_reg_Rn, OPR_reg_Rm}, 0x06200010, 0x0ff000f0},
    {aarch32_op_QADD8_A1, "qadd8 QADD8_A1", {OPR_reg_Rd, OPR_reg_Rn, OPR_reg_Rm}, 0x06200090, 0x0ff000f0},
    {aarch32_op_QASX_A1, "qasx QASX_A1", {OPR_reg_Rd, OPR_reg_Rn, OPR_reg_Rm}, 0x06200030, 0x0ff000f0},
    {aarch32_op_QDADD_A1, "qdadd QDADD_A1", {OPR_reg_Rd, OPR_reg_Rm, OPR_reg_Rn}, 0x01400050, 0x0ff000f0},
    {aarch32_op_QDSUB_A1, "qdsub QDSUB_A1", {OPR_reg_Rd, OPR_reg_Rm, OPR_reg_Rn}, 0x01600050, 0x0ff000f0},
    {aarch32_op_QSAX_A1, "qsax QSAX_A1", {OPR_reg_Rd, OPR_reg_Rn, OPR_reg_Rm}, 0x06200050, 0x0ff000f0},
    {aarch32_op_QSUB_A1, "qsub QSUB_A1", {OPR_reg_Rd, OPR_reg_Rm, OPR_reg_Rn}, 0x01200050, 0x0ff000f0},
    {aarch32_op_QSUB16_A1, "qsub16 QSUB16_A1", {OPR_reg_Rd, OPR_reg_Rn, OPR_reg_Rm}, 0x06200070, 0x0ff000f0},
    {aarch32_op_QSUB8_A1, "qsub8 QSUB8_A1", {OPR_reg_Rd, OPR_reg_Rn, OPR_reg_Rm}, 0x062000f0, 0x0ff000f0},
    {aarch32_op_RBIT_A1, "rbit RBIT_A1", {OPR_reg_Rd, OPR_reg_Rm}, 0x06f00030, 0x0ff000f0},
    {aarch32_op_REV_A1, "rev REV_A1", {OPR_reg_Rd, OPR_reg_Rm}, 0x06b00030, 0x0ff000f0},
    {aarch32_op_REV16_A1, "rev16 REV16_A1", {OPR_reg_Rd, OPR_reg_Rm}, 0x06b000b0, 0x0ff000f0},
    {aarch32_op_REVSH_A1, "revsh REVSH_A1", {OPR_reg_Rd, OPR_reg_Rm}, 0x06f000b0, 0x0ff000f0},
    {aarch32_op_RFEDA_A1_AS, "rfeda RFEDA_A1_AS", {OPR_reg_Rn}, 0xf8100000, 0xffd00000},
    {aarch32_op_RFEDB_A1_AS, "rfedb RFEDB_A1_AS", {OPR_reg_Rn}, 0xf9100000, 0xffd00000},
    {aarch32_op_RFEIA_A1_AS, "rfe{ia} RFEIA_A1_AS", {OPR_reg_Rn}, 0xf8900000, 0xffd00000},
    {aarch32_op_RFEIB_A1_AS, "rfeib RFEIB_A1_AS", {OPR_reg_Rn}, 0xf9900000, 0xffd00000},
    {aarch32_op_ROR_MOV_r_A1, "ror ROR_MOV_r_A1", {OPR_reg_Rd, OPR_reg_Rm}, 0x01a00060, 0x0ff00070},
    {aarch32_op_ROR_MOV_rr_A1, "ror ROR_MOV_rr_A1", {OPR_reg_Rd, OPR_reg_Rm, OPR_reg_Rs}, 0x01a00070, 0x0ff000f0},
    {aarch32_op_RORS_MOVS_r_A1, "rors RORS_MOVS_r_A1", {OPR_reg_Rd, OPR_reg_Rm}, 0x01b00060, 0x0ff00070},
    {aarch32_op_RORS_MOVS_rr_A1, "rors RORS_MOVS_rr_A1", {OPR_reg_Rd, OPR_reg_Rm, OPR_reg_Rs}, 0x01b00070, 0x0ff000f0},
    {aarch32_op_RRX_MOV_r_A1_RRX, "rrx RRX_MOV_r_A1_RRX", {OPR_reg_Rd, OPR_reg_Rm}, 0x01a00060, 0x0ff00ff0},
    {aarch32_op_RRXS_MOVS_r_A1_RRX, "rrxs RRXS_MOVS_r_A1_RRX", {OPR_reg_Rd, OPR_reg_Rm}, 0x01b00060, 0x0ff00ff0},
    {aarch32_op_RSB_i_A1, "rsb RSB_i_A1", {OPR_reg_Rd, OPR_reg_Rn, OPR_imm_const}, 0x02600000, 0x0ff00000},
    {aarch32_op_RSBS_i_A1, "rsbs RSBS_i_A1", {OPR_reg_Rd, OPR_reg_Rn, OPR_imm_const}, 0x02700000, 0x0ff00000},
    {aarch32_op_RSB_r_A1_RRX, "rsb RSB_r_A1_RRX", {OPR_reg_Rd, OPR_reg_Rn, OPR_reg_Rm}, 0x00600060, 0x0ff00ff0},
    {aarch32_op_RSB_r_A1, "rsb RSB_r_A1", {OPR_reg_Rd, OPR_reg_Rn, OPR_reg_Rm}, 0x00600000, 0x0ff00010},
    {aarch32_op_RSBS_r_A1_RRX, "rsbs RSBS_r_A1_RRX", {OPR_reg_Rd, OPR_reg_Rn, OPR_reg_Rm}, 0x00700060, 0x0ff00ff0},
    {aarch32_op_RSBS_r_A1, "rsbs RSBS_r_A1", {OPR_reg_Rd, OPR_reg_Rn, OPR_reg_Rm}, 0x00700000, 0x0ff00010},
    {aarch32_op_RSBS_rr_A1, "rsbs RSBS_rr_A1", {OPR_reg_Rd, OPR_reg_Rn, OPR_reg_Rm, OPR_reg_Rs}, 0x00700010, 0x0ff00090},
    {aarch32_op_RSB_rr_A1, "rsb RSB_rr_A1", {OPR_reg_Rd, OPR_reg_Rn, OPR_reg_Rm, OPR_reg_Rs}, 0x00600010, 0x0ff00090},
    {aarch32_op_RSC_i_A1, "rsc RSC_i_A1", {OPR_reg_Rd, OPR_reg_Rn, OPR_imm_const}, 0x02e00000, 0x0ff00000},
    {aarch32_op_RSCS_i_A1, "rscs RSCS_i_A1", {OPR_reg_Rd, OPR_reg_Rn, OPR_imm_const}, 0x02f00000, 0x0ff00000},
    {aarch32_op_RSC_r_A1_RRX, "rsc RSC_r_A1_RRX", {OPR_reg_Rd, OPR_reg_Rn, OPR_reg_Rm}, 0x00e00060, 0x0ff00ff0},
    {aarch32_op_RSC_r_A1, "rsc RSC_r_A1", {OPR_reg_Rd, OPR_reg_Rn, OPR_reg_Rm}, 0x00e00000, 0x0ff00010},
    {aarch32_op_RSCS_r_A1_RRX, "rscs RSCS_r_A1_RRX", {OPR_reg_Rd, OPR_reg_Rn, OPR_reg_Rm}, 0x00f00060, 0x0ff00ff0},
    {aarch32_op_RSCS_r_A1, "rscs RSCS_r_A1", {OPR_reg_Rd, OPR_reg_Rn, OPR_reg_Rm}, 0x00f00000, 0x0ff00010},
    {aarch32_op_RSCS_rr_A1, "rscs RSCS_rr_A1", {OPR_reg_Rd, OPR_reg_Rn, OPR_reg_Rm, OPR_reg_Rs}, 0x00f00010, 0x0ff00090},
    {aarch32_op_RSC_rr_A1, "rsc RSC_rr_A1", {OPR_reg_Rd, OPR_reg_Rn, OPR_reg_Rm, OPR_reg_Rs}, 0x00e00010, 0x0ff00090},
    {aarch32_op_SADD16_A1, "sadd16 SADD16_A1", {OPR_reg_Rd, OPR_reg_Rn, OPR_reg_Rm}, 0x06100010, 0x0ff000f0},
    {aarch32_op_SADD8_A1, "sadd8 SADD8_A1", {OPR_reg_Rd, OPR_reg_Rn, OPR_reg_Rm}, 0x06100090, 0x0ff000f0},
    {aarch32_op_SASX_A1, "sasx SASX_A1", {OPR_reg_Rd, OPR_reg_Rn, OPR_reg_Rm}, 0x06100030, 0x0ff000f0},
    {aarch32_op_SBC_i_A1, "sbc SBC_i_A1", {OPR_reg_Rd, OPR_reg_Rn, OPR_imm_const}, 0x02c00000, 0x0ff00000},
    {aarch32_op_SBCS_i_A1, "sbcs SBCS_i_A1", {OPR_reg_Rd, OPR_reg_Rn, OPR_imm_const}, 0x02d00000, 0x0ff00000},
    {aarch32_op_SBC_r_A1_RRX, "sbc SBC_r_A1_RRX", {OPR_reg_Rd, OPR_reg_Rn, OPR_reg_Rm}, 0x00c00060, 0x0ff00ff0},
    {aarch32_op_SBC_r_A1, "sbc SBC_r_A1", {OPR_reg_Rd, OPR_reg_Rn, OPR_reg_Rm}, 0x00c00000, 0x0ff00010},
    {aarch32_op_SBCS_r_A1_RRX, "sbcs SBCS_r_A1_RRX", {OPR_reg_Rd, OPR_reg_Rn, OPR_reg_Rm}, 0x00d00060, 0x0ff00ff0},
    {aarch32_op_SBCS_r_A1, "sbcs SBCS_r_A1", {OPR_reg_Rd, OPR_reg_Rn, OPR_reg_Rm}, 0x00d00000, 0x0ff00010},
    {aarch32_op_SBCS_rr_A1, "sbcs SBCS_rr_A1", {OPR_reg_Rd, OPR_reg_Rn, OPR_reg_Rm, OPR_reg_Rs}, 0x00d00010, 0x0ff00090},
    {aarch32_op_SBC_rr_A1, "sbc SBC_rr_A1", {OPR_reg_Rd, OPR_reg_Rn, OPR_reg_Rm, OPR_reg_Rs}, 0x00c00010, 0x0ff00090},
    {aarch32_op_SBFX_A1, "sbfx SBFX_A1", {OPR_reg_Rd, OPR_reg_Rn}, 0x07a00050, 0x0fe00070},
    {aarch32_op_SDIV_A1, "sdiv SDIV_A1", {OPR_reg_Rd, OPR_reg_Rn, OPR_reg_Rm}, 0x07100010, 0x0ff000f0},
    {aarch32_op_SEL_A1, "sel SEL_A1", {OPR_reg_Rd, OPR_reg_Rn, OPR_reg_Rm}, 0x068000b0, 0x0ff000f0},
    {aarch32_op_SETEND_A1, "setend SETEND_A1", {}, 0xf1010000, 0xfff100f0},
    {aarch32_op_SETPAN_A1, "setpan SETPAN_A1", {}, 0xf1100000, 0xfff000f0},
    {aarch32_op_SEV_A1, "sev SEV_A1", {}, 0x03200004, 0x0fff00ff},
    {aarch32_op_SEVL_A1, "sevl SEVL_A1", {}, 0x03200005, 0x0fff00ff},
    {aarch32_op_SHA1C_A1, "sha1c SHA1C_A1", {OPR_reg_Qd, OPR_reg_Qn, OPR_reg_Qm}, 0xf2000c00, 0xffb00f10},
    {aarch32_op_SHA1H_A1, "sha1h SHA1H_A1", {OPR_reg_Qd, OPR_reg_Qm}, 0xf3b102c0, 0xffb30fd0},
    {aarch32_op_SHA1M_A1, "sha1m SHA1M_A1", {OPR_reg_Qd, OPR_reg_Qn, OPR_reg_Qm}, 0xf2200c00, 0xffb00f10},
    {aarch32_op_SHA1P_A1, "sha1p SHA1P_A1", {OPR_reg_Qd, OPR_reg_Qn, OPR_reg_Qm}, 0xf2100c00, 0xffb00f10},
    {aarch32_op_SHA1SU0_A1, "sha1su0 SHA1SU0_A1", {OPR_reg_Qd, OPR_reg_Qn, OPR_reg_Qm}, 0xf2300c00, 0xffb00f10},
    {aarch32_op_SHA1SU1_A1, "sha1su1 SHA1SU1_A1", {OPR_reg_Qd, OPR_reg_Qm}, 0xf3b20380, 0xffb30fd0},
    {aarch32_op_SHA256H_A1, "sha256h SHA256H_A1", {OPR_reg_Qd, OPR_reg_Qn, OPR_reg_Qm}, 0xf3000c00, 0xffb00f10},
    {aarch32_op_SHA256H2_A1, "sha256h2 SHA256H2_A1", {OPR_reg_Qd, OPR_reg_Qn, OPR_reg_Qm}, 0xf3100c00, 0xffb00f10},
    {aarch32_op_SHA256SU0_A1, "sha256su0 SHA256SU0_A1", {OPR_reg_Qd, OPR_reg_Qm}, 0xf3b203c0, 0xffb30fd0},
    {aarch32_op_SHA256SU1_A1, "sha256su1 SHA256SU1_A1", {OPR_reg_Qd, OPR_reg_Qn, OPR_reg_Qm}, 0xf3200c00, 0xffb00f10},
    {aarch32_op_SHADD16_A1, "shadd16 SHADD16_A1", {OPR_reg_Rd, OPR_reg_Rn, OPR_reg_Rm}, 0x06300010, 0x0ff000f0},
    {aarch32_op_SHADD8_A1, "shadd8 SHADD8_A1", {OPR_reg_Rd, OPR_reg_Rn, OPR_reg_Rm}, 0x06300090, 0x0ff000f0},
    {aarch32_op_SHASX_A1, "shasx SHASX_A1", {OPR_reg_Rd, OPR_reg_Rn, OPR_reg_Rm}, 0x06300030, 0x0ff000f0},
    {aarch32_op_SHSAX_A1, "shsax SHSAX_A1", {OPR_reg_Rd, OPR_reg_Rn, OPR_reg_Rm}, 0x06300050, 0x0ff000f0},
    {aarch32_op_SHSUB16_A1, "shsub16 SHSUB16_A1", {OPR_reg_Rd, OPR_reg_Rn, OPR_reg_Rm}, 0x06300070, 0x0ff000f0},
    {aarch32_op_SHSUB8_A1, "shsub8 SHSUB8_A1", {OPR_reg_Rd, OPR_reg_Rn, OPR_reg_Rm}, 0x063000f0, 0x0ff000f0},
    {aarch32_op_SMC_A1_AS, "smc SMC_A1_AS", {}, 0x01600070, 0x0ff000f0},
    {aarch32_op_SMLABB_A1, "smlabb SMLABB_A1", {OPR_reg_Rd, OPR_reg_Rn, OPR_reg_Rm, OPR_reg_Ra}, 0x01000080, 0x0ff000f0},
    {aarch32_op_SMLABT_A1, "smlabt SMLABT_A1", {OPR_reg_Rd, OPR_reg_Rn, OPR_reg_Rm, OPR_reg_Ra}, 0x010000c0, 0x0ff000f0},
    {aarch32_op_SMLATB_A1, "smlatb SMLATB_A1", {OPR_reg_Rd, OPR_reg_Rn, OPR_reg_Rm, OPR_reg_Ra}, 0x010000a0, 0x0ff000f0},
    {aarch32_op_SMLATT_A1, "smlatt SMLATT_A1", {OPR_reg_Rd, OPR_reg_Rn, OPR_reg_Rm, OPR_reg_Ra}, 0x010000e0, 0x0ff000f0},
    {aarch32_op_SMLAD_A1, "smlad SMLAD_A1", {OPR_reg_Rd, OPR_reg_Rn, OPR_reg_Rm, OPR_reg_Ra}, 0x07000010, 0x0ff000f0},
    {aarch32_op_SMLADX_A1, "smladx SMLADX_A1", {OPR_reg_Rd, OPR_reg_Rn, OPR_reg_Rm, OPR_reg_Ra}, 0x07000030, 0x0ff000f0},
    {aarch32_op_SMLALS_A1, "smlals SMLALS_A1", {OPR_reg_RdLo, OPR_reg_RdHi, OPR_reg_Rn, OPR_reg_Rm}, 0x00f00090, 0x0ff000f0},
    {aarch32_op_SMLAL_A1, "smlal SMLAL_A1", {OPR_reg_RdLo, OPR_reg_RdHi, OPR_reg_Rn, OPR_reg_Rm}, 0x00e00090, 0x0ff000f0},
    {aarch32_op_SMLALBB_A1, "smlalbb SMLALBB_A1", {OPR_reg_RdLo, OPR_reg_RdHi, OPR_reg_Rn, OPR_reg_Rm}, 0x01400080, 0x0ff000f0},
    {aarch32_op_SMLALBT_A1, "smlalbt SMLALBT_A1", {OPR_reg_RdLo, OPR_reg_RdHi, OPR_reg_Rn, OPR_reg_Rm}, 0x014000c0, 0x0ff000f0},
    {aarch32_op_SMLALTB_A1, "smlaltb SMLALTB_A1", {OPR_reg_RdLo, OPR_reg_RdHi, OPR_reg_Rn, OPR_reg_Rm}, 0x014000a0, 0x0ff000f0},
    {aarch32_op_SMLALTT_A1, "smlaltt SMLALTT_A1", {OPR_reg_RdLo, OPR_reg_RdHi, OPR_reg_Rn, OPR_reg_Rm}, 0x014000e0, 0x0ff000f0},
    {aarch32_op_SMLALD_A1, "smlald SMLALD_A1", {OPR_reg_RdLo, OPR_reg_RdHi, OPR_reg_Rn, OPR_reg_Rm}, 0x07400010, 0x0ff000f0},
    {aarch32_op_SMLALDX_A1, "smlaldx SMLALDX_A1", {OPR_reg_RdLo, OPR_reg_RdHi, OPR_reg_Rn, OPR_reg_Rm}, 0x07400030, 0x0ff000f0},
    {aarch32_op_SMLAWB_A1, "smlawb SMLAWB_A1", {OPR_reg_Rd, OPR_reg_Rn, OPR_reg_Rm, OPR_reg_Ra}, 0x01200080, 0x0ff000f0},
    {aarch32_op_SMLAWT_A1, "smlawt SMLAWT_A1", {OPR_reg_Rd, OPR_reg_Rn, OPR_reg_Rm, OPR_reg_Ra}, 0x012000c0, 0x0ff000f0},
    {aarch32_op_SMLSD_A1, "smlsd SMLSD_A1", {OPR_reg_Rd, OPR_reg_Rn, OPR_reg_Rm, OPR_reg_Ra}, 0x07000050, 0x0ff000f0},
    {aarch32_op_SMLSDX_A1, "smlsdx SMLSDX_A1", {OPR_reg_Rd, OPR_reg_Rn, OPR_reg_Rm, OPR_reg_Ra}, 0x07000070, 0x0ff000f0},
    {aarch32_op_SMLSLD_A1, "smlsld SMLSLD_A1", {OPR_reg_RdLo, OPR_reg_RdHi, OPR_reg_Rn, OPR_reg_Rm}, 0x07400050, 0x0ff000f0},
    {aarch32_op_SMLSLDX_A1, "smlsldx SMLSLDX_A1", {OPR_reg_RdLo, OPR_reg_RdHi, OPR_reg_Rn, OPR_reg_Rm}, 0x07400070, 0x0ff000f0},
    {aarch32_op_SMMLA_A1, "smmla SMMLA_A1", {OPR_reg_Rd, OPR_reg_Rn, OPR_reg_Rm, OPR_reg_Ra}, 0x07500010, 0x0ff000f0},
    {aarch32_op_SMMLAR_A1, "smmlar SMMLAR_A1", {OPR_reg_Rd, OPR_reg_Rn, OPR_reg_Rm, OPR_reg_Ra}, 0x07500030, 0x0ff000f0},
    {aarch32_op_SMMLS_A1, "smmls SMMLS_A1", {OPR_reg_Rd, OPR_reg_Rn, OPR_reg_Rm, OPR_reg_Ra}, 0x075000d0, 0x0ff000f0},
    {aarch32_op_SMMLSR_A1, "smmlsr SMMLSR_A1", {OPR_reg_Rd, OPR_reg_Rn, OPR_reg_Rm, OPR_reg_Ra}, 0x075000f0, 0x0ff000f0},
    {aarch32_op_SMMUL_A1, "smmul SMMUL_A1", {OPR_reg_Rd, OPR_reg_Rn, OPR_reg_Rm}, 0x0750f010, 0x0ff0f0f0},
    {aarch32_op_SMMULR_A1, "smmulr SMMULR_A1", {OPR_reg_Rd, OPR_reg_Rn, OPR_reg_Rm}, 0x0750f030, 0x0ff0f0f0},
    {aarch32_op_SMUAD_A1, "smuad SMUAD_A1", {OPR_reg_Rd, OPR_reg_Rn, OPR_reg_Rm}, 0x0700f010, 0x0ff0f0f0},
    {aarch32_op_SMUADX_A1, "smuadx SMUADX_A1", {OPR_reg_Rd, OPR_reg_Rn, OPR_reg_Rm}, 0x0700f030, 0x0ff0f0f0},
    {aarch32_op_SMULBB_A1, "smulbb SMULBB_A1", {OPR_reg_Rd, OPR_reg_Rn, OPR_reg_Rm}, 0x01600080, 0x0ff000f0},
    {aarch32_op_SMULBT_A1, "smulbt SMULBT_A1", {OPR_reg_Rd, OPR_reg_Rn, OPR_reg_Rm}, 0x016000c0, 0x0ff000f0},
    {aarch32_op_SMULTB_A1, "smultb SMULTB_A1", {OPR_reg_Rd, OPR_reg_Rn, OPR_reg_Rm}, 0x016000a0, 0x0ff000f0},
    {aarch32_op_SMULTT_A1, "smultt SMULTT_A1", {OPR_reg_Rd, OPR_reg_Rn, OPR_reg_Rm}, 0x016000e0, 0x0ff000f0},
    {aarch32_op_SMULLS_A1, "smulls SMULLS_A1", {OPR_reg_RdLo, OPR_reg_RdHi, OPR_reg_Rn, OPR_reg_Rm}, 0x00d00090, 0x0ff000f0},
    {aarch32_op_SMULL_A1, "smull SMULL_A1", {OPR_reg_RdLo, OPR_reg_RdHi, OPR_reg_Rn, OPR_reg_Rm}, 0x00c00090, 0x0ff000f0},
    {aarch32_op_SMULWB_A1, "smulwb SMULWB_A1", {OPR_reg_Rd, OPR_reg_Rn, OPR_reg_Rm}, 0x012000a0, 0x0ff000f0},
    {aarch32_op_SMULWT_A1, "smulwt SMULWT_A1", {OPR_reg_Rd, OPR_reg_Rn, OPR_reg_Rm}, 0x012000e0, 0x0ff000f0},
    {aarch32_op_SMUSD_A1, "smusd SMUSD_A1", {OPR_reg_Rd, OPR_reg_Rn, OPR_reg_Rm}, 0x0700f050, 0x0ff0f0f0},
    {aarch32_op_SMUSDX_A1, "smusdx SMUSDX_A1", {OPR_reg_Rd, OPR_reg_Rn, OPR_reg_Rm}, 0x0700f070, 0x0ff0f0f0},
    {aarch32_op_SRSDA_A1_AS, "srsda SRSDA_A1_AS", {}, 0xf8400000, 0xffd00000},
    {aarch32_op_SRSDB_A1_AS, "srsdb SRSDB_A1_AS", {}, 0xf9400000, 0xffd00000},
    {aarch32_op_SRSIA_A1_AS, "srs{ia} SRSIA_A1_AS", {}, 0xf8c00000, 0xffd00000},
    {aarch32_op_SRSIB_A1_AS, "srsib SRSIB_A1_AS", {}, 0xf9c00000, 0xffd00000},
    {aarch32_op_SSAT_A1_ASR, "ssat SSAT_A1_ASR", {OPR_reg_Rd, OPR_reg_Rn}, 0x06a00050, 0x0fe00070},
    {aarch32_op_SSAT_A1_LSL, "ssat SSAT_A1_LSL", {OPR_reg_Rd, OPR_reg_Rn}, 0x06a00010, 0x0fe00070},
    {aarch32_op_SSAT16_A1, "ssat16 SSAT16_A1", {OPR_reg_Rd, OPR_reg_Rn}, 0x06a00030, 0x0ff000f0},
    {aarch32_op_SSAX_A1, "ssax SSAX_A1", {OPR_reg_Rd, OPR_reg_Rn, OPR_reg_Rm}, 0x06100050, 0x0ff000f0},
    {aarch32_op_SSUB16_A1, "ssub16 SSUB16_A1", {OPR_reg_Rd, OPR_reg_Rn, OPR_reg_Rm}, 0x06100070, 0x0ff000f0},
    {aarch32_op_SSUB8_A1, "ssub8 SSUB8_A1", {OPR_reg_Rd, OPR_reg_Rn, OPR_reg_Rm}, 0x061000f0, 0x0ff000f0},
    {aarch32_op_STC_A1_off, "stc STC_A1_off", {OPR_reg_Rn}, 0x0d005e00, 0x0f70ff00},
    {aarch32_op_STC_A1_post, "stc STC_A1_post", {OPR_reg_Rn}, 0x0c205e00, 0x0f70ff00},
    {aarch32_op_STC_A1_pre, "stc STC_A1_pre", {OPR_reg_Rn}, 0x0d205e00, 0x0f70ff00},
    {aarch32_op_STC_A1_unind, "stc STC_A1_unind", {OPR_reg_Rn}, 0x0c805e00, 0x0ff0ff00},
    {aarch32_op_STL_A1, "stl STL_A1", {OPR_reg_Rt_2, OPR_reg_Rn}, 0x01800090, 0x0ff003f0},
    {aarch32_op_STLB_A1, "stlb STLB_A1", {OPR_reg_Rt, OPR_reg_Rn}, 0x01c00090, 0x0ff003f0},
    {aarch32_op_STLEX_A1, "stlex STLEX_A1", {OPR_reg_Rd, OPR_reg_Rt, OPR_reg_Rn}, 0x01800290, 0x0ff003f0},
    {aarch32_op_STLEXB_A1, "stlexb STLEXB_A1", {OPR_reg_Rd, OPR_reg_Rt, OPR_reg_Rn}, 0x01c00290, 0x0ff003f0},
    {aarch32_op_STLEXD_A1, "stlexd STLEXD_A1", {OPR_reg_Rd, OPR_reg_Rt, OPR_reg_Rt2_3, OPR_reg_Rn}, 0x01a00290, 0x0ff003f0},
    {aarch32_op_STLEXH_A1, "stlexh STLEXH_A1", {OPR_reg_Rd, OPR_reg_Rt, OPR_reg_Rn}, 0x01e00290, 0x0ff003f0},
    {aarch32_op_STLH_A1, "stlh STLH_A1", {OPR_reg_Rt, OPR_reg_Rn}, 0x01e00090, 0x0ff003f0},
    {aarch32_op_STM_A1, "stm STM_A1", {OPR_reg_Rn, OPR_reglist}, 0x08800000, 0x0fd00000},
    {aarch32_op_STM_u_A1_AS, "stm STM_u_A1_AS", {OPR_reg_Rn, OPR_reglist}, 0x08400000, 0x0e500000},
    {aarch32_op_STMDA_A1, "stm STMDA_A1", {OPR_reg_Rn, OPR_reglist}, 0x08000000, 0x0fd00000},
    {aarch32_op_STMDB_A1, "stm STMDB_A1", {OPR_reg_Rn, OPR_reglist}, 0x09000000, 0x0fd00000},
    {aarch32_op_STMIB_A1, "stm STMIB_A1", {OPR_reg_Rn, OPR_reglist}, 0x09800000, 0x0fd00000},
    {aarch32_op_STR_i_A1_off, "str STR_i_A1_off", {OPR_reg_Rt, OPR_reg_Rn}, 0x05000000, 0x0f700000},
    {aarch32_op_STR_i_A1_post, "str STR_i_A1_post", {OPR_reg_Rt, OPR_reg_Rn}, 0x04000000, 0x0f700000},
    {aarch32_op_STR_i_A1_pre, "str STR_i_A1_pre", {OPR_reg_Rt, OPR_reg_Rn}, 0x05200000, 0x0f700000},
    {aarch32_op_STR_r_A1_off, "str STR_r_A1_off", {OPR_reg_Rt, OPR_reg_Rn, OPR_reg_Rm}, 0x07000000, 0x0f700010},
    {aarch32_op_STR_r_A1_post, "str STR_r_A1_post", {OPR_reg_Rt, OPR_reg_Rn, OPR_reg_Rm}, 0x06000000, 0x0f700010},
    {aarch32_op_STR_r_A1_pre, "str STR_r_A1_pre", {OPR_reg_Rt, OPR_reg_Rn, OPR_reg_Rm}, 0x07200000, 0x0f700010},
    {aarch32_op_STRB_i_A1_off, "strb STRB_i_A1_off", {OPR_reg_Rt, OPR_reg_Rn}, 0x05400000, 0x0f700000},
    {aarch32_op_STRB_i_A1_post, "strb STRB_i_A1_post", {OPR_reg_Rt, OPR_reg_Rn}, 0x04400000, 0x0f700000},
    {aarch32_op_STRB_i_A1_pre, "strb STRB_i_A1_pre", {OPR_reg_Rt, OPR_reg_Rn}, 0x05600000, 0x0f700000},
    {aarch32_op_STRB_r_A1_off, "strb STRB_r_A1_off", {OPR_reg_Rt, OPR_reg_Rn, OPR_reg_Rm}, 0x07400000, 0x0f700010},
    {aarch32_op_STRB_r_A1_post, "strb STRB_r_A1_post", {OPR_reg_Rt, OPR_reg_Rn, OPR_reg_Rm}, 0x06400000, 0x0f700010},
    {aarch32_op_STRB_r_A1_pre, "strb STRB_r_A1_pre", {OPR_reg_Rt, OPR_reg_Rn, OPR_reg_Rm}, 0x07600000, 0x0f700010},
    {aarch32_op_STRBT_A1, "strbt STRBT_A1", {OPR_reg_Rt, OPR_reg_Rn}, 0x04600000, 0x0f700000},
    {aarch32_op_STRBT_A2, "strbt STRBT_A2", {OPR_reg_Rt, OPR_reg_Rn, OPR_reg_Rm}, 0x06600000, 0x0f700010},
    {aarch32_op_STRD_i_A1_off, "strd STRD_i_A1_off", {OPR_reg_Rt, OPR_reg_Rt2, OPR_reg_Rn}, 0x014000f0, 0x0f7000f0},
    {aarch32_op_STRD_i_A1_post, "strd STRD_i_A1_post", {OPR_reg_Rt, OPR_reg_Rt2, OPR_reg_Rn}, 0x004000f0, 0x0f7000f0},
    {aarch32_op_STRD_i_A1_pre, "strd STRD_i_A1_pre", {OPR_reg_Rt, OPR_reg_Rt2, OPR_reg_Rn}, 0x016000f0, 0x0f7000f0},
    {aarch32_op_STRD_r_A1_off, "strd STRD_r_A1_off", {OPR_reg_Rt, OPR_reg_Rt2, OPR_reg_Rn, OPR_reg_Rm}, 0x010000f0, 0x0f7000f0},
    {aarch32_op_STRD_r_A1_post, "strd STRD_r_A1_post", {OPR_reg_Rt, OPR_reg_Rt2, OPR_reg_Rn, OPR_reg_Rm}, 0x000000f0, 0x0f7000f0},
    {aarch32_op_STRD_r_A1_pre, "strd STRD_r_A1_pre", {OPR_reg_Rt, OPR_reg_Rt2, OPR_reg_Rn, OPR_reg_Rm}, 0x012000f0, 0x0f7000f0},
    {aarch32_op_STREX_A1, "strex STREX_A1", {OPR_reg_Rd, OPR_reg_Rt, OPR_reg_Rn}, 0x01800390, 0x0ff003f0},
    {aarch32_op_STREXB_A1, "strexb STREXB_A1", {OPR_reg_Rd, OPR_reg_Rt, OPR_reg_Rn}, 0x01c00390, 0x0ff003f0},
    {aarch32_op_STREXD_A1, "strexd STREXD_A1", {OPR_reg_Rd, OPR_reg_Rt, OPR_reg_Rt2, OPR_reg_Rn}, 0x01a00390, 0x0ff003f0},
    {aarch32_op_STREXH_A1, "strexh STREXH_A1", {OPR_reg_Rd, OPR_reg_Rt, OPR_reg_Rn}, 0x01e00390, 0x0ff003f0},
    {aarch32_op_STRH_i_A1_off, "strh STRH_i_A1_off", {OPR_reg_Rt, OPR_reg_Rn}, 0x014000b0, 0x0f7000f0},
    {aarch32_op_STRH_i_A1_post, "strh STRH_i_A1_post", {OPR_reg_Rt, OPR_reg_Rn}, 0x004000b0, 0x0f7000f0},
    {aarch32_op_STRH_i_A1_pre, "strh STRH_i_A1_pre", {OPR_reg_Rt, OPR_reg_Rn}, 0x016000b0, 0x0f7000f0},
    {aarch32_op_STRH_r_A1_off, "strh STRH_r_A1_off", {OPR_reg_Rt, OPR_reg_Rn, OPR_reg_Rm}, 0x010000b0, 0x0f7000f0},
    {aarch32_op_STRH_r_A1_post, "strh STRH_r_A1_post", {OPR_reg_Rt, OPR_reg_Rn, OPR_reg_Rm}, 0x000000b0, 0x0f7000f0},
    {aarch32_op_STRH_r_A1_pre, "strh STRH_r_A1_pre", {OPR_reg_Rt, OPR_reg_Rn, OPR_reg_Rm}, 0x012000b0, 0x0f7000f0},
    {aarch32_op_STRHT_A1, "strht STRHT_A1", {OPR_reg_Rt, OPR_reg_Rn}, 0x006000b0, 0x0f7000f0},
    {aarch32_op_STRHT_A2, "strht STRHT_A2", {OPR_reg_Rt, OPR_reg_Rn, OPR_reg_Rm}, 0x002000b0, 0x0f7000f0},
    {aarch32_op_STRT_A1, "strt STRT_A1", {OPR_reg_Rt, OPR_reg_Rn}, 0x04200000, 0x0f700000},
    {aarch32_op_STRT_A2, "strt STRT_A2", {OPR_reg_Rt, OPR_reg_Rn, OPR_reg_Rm}, 0x06200000, 0x0f700010},
    {aarch32_op_SUB_ADR_A2, "sub SUB_ADR_A2", {OPR_reg_Rd, OPR_imm_const}, 0x024f0000, 0x0fff0000},
    {aarch32_op_SUB_i_A1, "sub SUB_i_A1", {OPR_reg_Rd, OPR_reg_Rn, OPR_imm_const}, 0x02400000, 0x0ff00000},
    {aarch32_op_SUBS_i_A1, "subs SUBS_i_A1", {OPR_reg_Rd, OPR_reg_Rn, OPR_imm_const}, 0x02500000, 0x0ff00000},
    {aarch32_op_SUB_r_A1_RRX, "sub SUB_r_A1_RRX", {OPR_reg_Rd, OPR_reg_Rn, OPR_reg_Rm}, 0x00400060, 0x0ff00ff0},
    {aarch32_op_SUB_r_A1, "sub SUB_r_A1", {OPR_reg_Rd, OPR_reg_Rn, OPR_reg_Rm}, 0x00400000, 0x0ff00010},
    {aarch32_op_SUBS_r_A1_RRX, "subs SUBS_r_A1_RRX", {OPR_reg_Rd, OPR_reg_Rn, OPR_reg_Rm}, 0x00500060, 0x0ff00ff0},
    {aarch32_op_SUBS_r_A1, "subs SUBS_r_A1", {OPR_reg_Rd, OPR_reg_Rn, OPR_reg_Rm}, 0x00500000, 0x0ff00010},
    {aarch32_op_SUBS_rr_A1, "subs SUBS_rr_A1", {OPR_reg_Rd, OPR_reg_Rn, OPR_reg_Rm, OPR_reg_Rs}, 0x00500010, 0x0ff00090},
    {aarch32_op_SUB_rr_A1, "sub SUB_rr_A1", {OPR_reg_Rd, OPR_reg_Rn, OPR_reg_Rm, OPR_reg_Rs}, 0x00400010, 0x0ff00090},
    {aarch32_op_SUB_SP_i_A1, "sub SUB_SP_i_A1", {OPR_reg_Rd, OPR_imm_const}, 0x024d0000, 0x0fff0000},
    {aarch32_op_SUBS_SP_i_A1, "subs SUBS_SP_i_A1", {OPR_reg_Rd, OPR_imm_const}, 0x025d0000, 0x0fff0000},
    {aarch32_op_SUB_SP_r_A1_RRX, "sub SUB_SP_r_A1_RRX", {OPR_reg_Rd, OPR_reg_Rm}, 0x004d0060, 0x0fff0ff0},
    {aarch32_op_SUB_SP_r_A1, "sub SUB_SP_r_A1", {OPR_reg_Rd, OPR_reg_Rm}, 0x004d0000, 0x0fff0010},
    {aarch32_op_SUBS_SP_r_A1_RRX, "subs SUBS_SP_r_A1_RRX", {OPR_reg_Rd, OPR_reg_Rm}, 0x005d0060, 0x0fff0ff0},
    {aarch32_op_SUBS_SP_r_A1, "subs SUBS_SP_r_A1", {OPR_reg_Rd, OPR_reg_Rm}, 0x005d0000, 0x0fff0010},
    {aarch32_op_SVC_A1, "svc SVC_A1", {}, 0x0f000000, 0x0f000000},
    {aarch32_op_SXTAB_A1, "sxtab SXTAB_A1", {OPR_reg_Rd, OPR_reg_Rn, OPR_reg_Rm}, 0x06a00070, 0x0ff000f0},
    {aarch32_op_SXTAB16_A1, "sxtab16 SXTAB16_A1", {OPR_reg_Rd, OPR_reg_Rn, OPR_reg_Rm}, 0x06800070, 0x0ff000f0},
    {aarch32_op_SXTAH_A1, "sxtah SXTAH_A1", {OPR_reg_Rd, OPR_reg_Rn, OPR_reg_Rm}, 0x06b00070, 0x0ff000f0},
    {aarch32_op_SXTB_A1, "sxtb SXTB_A1", {OPR_reg_Rd, OPR_reg_Rm}, 0x06af0070, 0x0fff00f0},
    {aarch32_op_SXTB16_A1, "sxtb16 SXTB16_A1", {OPR_reg_Rd, OPR_reg_Rm}, 0x068f0070, 0x0fff00f0},
    {aarch32_op_SXTH_A1, "sxth SXTH_A1", {OPR_reg_Rd, OPR_reg_Rm}, 0x06bf0070, 0x0fff00f0},
    {aarch32_op_TEQ_i_A1, "teq TEQ_i_A1", {OPR_reg_Rn, OPR_imm_const}, 0x03300000, 0x0ff00000},
    {aarch32_op_TEQ_r_A1_RRX, "teq TEQ_r_A1_RRX", {OPR_reg_Rn, OPR_reg_Rm}, 0x01300060, 0x0ff00ff0},
    {aarch32_op_TEQ_r_A1, "teq TEQ_r_A1", {OPR_reg_Rn, OPR_reg_Rm}, 0x01300000, 0x0ff00010},
    {aarch32_op_TEQ_rr_A1, "teq TEQ_rr_A1", {OPR_reg_Rn, OPR_reg_Rm, OPR_reg_Rs}, 0x01300010, 0x0ff00090},
    {aarch32_op_TST_i_A1, "tst TST_i_A1", {OPR_reg_Rn, OPR_imm_const}, 0x03100000, 0x0ff00000},
    {aarch32_op_TST_r_A1_RRX, "tst TST_r_A1_RRX", {OPR_reg_Rn, OPR_reg_Rm}, 0x01100060, 0x0ff00ff0},
    {aarch32_op_TST_r_A1, "tst TST_r_A1", {OPR_reg_Rn, OPR_reg_Rm}, 0x01100000, 0x0ff00010},
    {aarch32_op_TST_rr_A1, "tst TST_rr_A1", {OPR_reg_Rn, OPR_reg_Rm, OPR_reg_Rs}, 0x01100010, 0x0ff00090},
    {aarch32_op_UADD16_A1, "uadd16 UADD16_A1", {OPR_reg_Rd, OPR_reg_Rn, OPR_reg_Rm}, 0x06500010, 0x0ff000f0},
    {aarch32_op_UADD8_A1, "uadd8 UADD8_A1", {OPR_reg_Rd, OPR_reg_Rn, OPR_reg_Rm}, 0x06500090, 0x0ff000f0},
    {aarch32_op_UASX_A1, "uasx UASX_A1", {OPR_reg_Rd, OPR_reg_Rn, OPR_reg_Rm}, 0x06500030, 0x0ff000f0},
    {aarch32_op_UBFX_A1, "ubfx UBFX_A1", {OPR_reg_Rd, OPR_reg_Rn}, 0x07e00050, 0x0fe00070},
    {aarch32_op_UDF_A1, "udf UDF_A1", {}, 0xe7f000f0, 0xfff000f0},
    {aarch32_op_UDIV_A1, "udiv UDIV_A1", {OPR_reg_Rd, OPR_reg_Rn, OPR_reg_Rm}, 0x07300010, 0x0ff000f0},
    {aarch32_op_UHADD16_A1, "uhadd16 UHADD16_A1", {OPR_reg_Rd, OPR_reg_Rn, OPR_reg_Rm}, 0x06700010, 0x0ff000f0},
    {aarch32_op_UHADD8_A1, "uhadd8 UHADD8_A1", {OPR_reg_Rd, OPR_reg_Rn, OPR_reg_Rm}, 0x06700090, 0x0ff000f0},
    {aarch32_op_UHASX_A1, "uhasx UHASX_A1", {OPR_reg_Rd, OPR_reg_Rn, OPR_reg_Rm}, 0x06700030, 0x0ff000f0},
    {aarch32_op_UHSAX_A1, "uhsax UHSAX_A1", {OPR_reg_Rd, OPR_reg_Rn, OPR_reg_Rm}, 0x06700050, 0x0ff000f0},
    {aarch32_op_UHSUB16_A1, "uhsub16 UHSUB16_A1", {OPR_reg_Rd, OPR_reg_Rn, OPR_reg_Rm}, 0x06700070, 0x0ff000f0},
    {aarch32_op_UHSUB8_A1, "uhsub8 UHSUB8_A1", {OPR_reg_Rd, OPR_reg_Rn, OPR_reg_Rm}, 0x067000f0, 0x0ff000f0},
    {aarch32_op_UMAAL_A1, "umaal UMAAL_A1", {OPR_reg_RdLo, OPR_reg_RdHi, OPR_reg_Rn, OPR_reg_Rm}, 0x00400090, 0x0ff000f0},
    {aarch32_op_UMLALS_A1, "umlals UMLALS_A1", {OPR_reg_RdLo, OPR_reg_RdHi, OPR_reg_Rn, OPR_reg_Rm}, 0x00b00090, 0x0ff000f0},
    {aarch32_op_UMLAL_A1, "umlal UMLAL_A1", {OPR_reg_RdLo, OPR_reg_RdHi, OPR_reg_Rn, OPR_reg_Rm}, 0x00a00090, 0x0ff000f0},
    {aarch32_op_UMULLS_A1, "umulls UMULLS_A1", {OPR_reg_RdLo, OPR_reg_RdHi, OPR_reg_Rn, OPR_reg_Rm}, 0x00900090, 0x0ff000f0},
    {aarch32_op_UMULL_A1, "umull UMULL_A1", {OPR_reg_RdLo, OPR_reg_RdHi, OPR_reg_Rn, OPR_reg_Rm}, 0x00800090, 0x0ff000f0},
    {aarch32_op_UQADD16_A1, "uqadd16 UQADD16_A1", {OPR_reg_Rd, OPR_reg_Rn, OPR_reg_Rm}, 0x06600010, 0x0ff000f0},
    {aarch32_op_UQADD8_A1, "uqadd8 UQADD8_A1", {OPR_reg_Rd, OPR_reg_Rn, OPR_reg_Rm}, 0x06600090, 0x0ff000f0},
    {aarch32_op_UQASX_A1, "uqasx UQASX_A1", {OPR_reg_Rd, OPR_reg_Rn, OPR_reg_Rm}, 0x06600030, 0x0ff000f0},
    {aarch32_op_UQSAX_A1, "uqsax UQSAX_A1", {OPR_reg_Rd, OPR_reg_Rn, OPR_reg_Rm}, 0x06600050, 0x0ff000f0},
    {aarch32_op_UQSUB16_A1, "uqsub16 UQSUB16_A1", {OPR_reg_Rd, OPR_reg_Rn, OPR_reg_Rm}, 0x06600070, 0x0ff000f0},
    {aarch32_op_UQSUB8_A1, "uqsub8 UQSUB8_A1", {OPR_reg_Rd, OPR_reg_Rn, OPR_reg_Rm}, 0x066000f0, 0x0ff000f0},
    {aarch32_op_USAD8_A1, "usad8 USAD8_A1", {OPR_reg_Rd, OPR_reg_Rn, OPR_reg_Rm}, 0x0780f010, 0x0ff0f0f0},
    {aarch32_op_USADA8_A1, "usada8 USADA8_A1", {OPR_reg_Rd, OPR_reg_Rn, OPR_reg_Rm, OPR_reg_Ra}, 0x07800010, 0x0ff000f0},
    {aarch32_op_USAT_A1_ASR, "usat USAT_A1_ASR", {OPR_reg_Rd, OPR_reg_Rn}, 0x06e00050, 0x0fe00070},
    {aarch32_op_USAT_A1_LSL, "usat USAT_A1_LSL", {OPR_reg_Rd, OPR_reg_Rn}, 0x06e00010, 0x0fe00070},
    {aarch32_op_USAT16_A1, "usat16 USAT16_A1", {OPR_reg_Rd, OPR_reg_Rn}, 0x06e00030, 0x0ff000f0},
    {aarch32_op_USAX_A1, "usax USAX_A1", {OPR_reg_Rd, OPR_reg_Rn, OPR_reg_Rm}, 0x06500050, 0x0ff000f0},
    {aarch32_op_USUB16_A1, "usub16 USUB16_A1", {OPR_reg_Rd, OPR_reg_Rn, OPR_reg_Rm}, 0x06500070, 0x0ff000f0},
    {aarch32_op_USUB8_A1, "usub8 USUB8_A1", {OPR_reg_Rd, OPR_reg_Rn, OPR_reg_Rm}, 0x065000f0, 0x0ff000f0},
    {aarch32_op_UXTAB_A1, "uxtab UXTAB_A1", {OPR_reg_Rd, OPR_reg_Rn, OPR_reg_Rm}, 0x06e00070, 0x0ff000f0},
    {aarch32_op_UXTAB16_A1, "uxtab16 UXTAB16_A1", {OPR_reg_Rd, OPR_reg_Rn, OPR_reg_Rm}, 0x06c00070, 0x0ff000f0},
    {aarch32_op_UXTAH_A1, "uxtah UXTAH_A1", {OPR_reg_Rd, OPR_reg_Rn, OPR_reg_Rm}, 0x06f00070, 0x0ff000f0},
    {aarch32_op_UXTB_A1, "uxtb UXTB_A1", {OPR_reg_Rd, OPR_reg_Rm}, 0x06ef0070, 0x0fff00f0},
    {aarch32_op_UXTB16_A1, "uxtb16 UXTB16_A1", {OPR_reg_Rd, OPR_reg_Rm}, 0x06cf0070, 0x0fff00f0},
    {aarch32_op_UXTH_A1, "uxth UXTH_A1", {OPR_reg_Rd, OPR_reg_Rm}, 0x06ff0070, 0x0fff00f0},
    {aarch32_op_VABA_A1_D, "vaba VABA_A1_D", {OPR_reg_Dd, OPR_reg_Dn, OPR_reg_Dm}, 0xf2000710, 0xfe800f50},
    {aarch32_op_VABA_A1_Q, "vaba VABA_A1_Q", {OPR_reg_Qd, OPR_reg_Qn, OPR_reg_Qm}, 0xf2000750, 0xfe800f50},
    {aarch32_op_VABAL_A1, "vabal VABAL_A1", {OPR_reg_Qd, OPR_reg_Dn, OPR_reg_Dm}, 0xf2800500, 0xfe800f50},
    {aarch32_op_VABD_f_A1_D, "vabd VABD_f_A1_D", {OPR_reg_Dd, OPR_reg_Dn, OPR_reg_Dm}, 0xf3200d00, 0xffa00f50},
    {aarch32_op_VABD_f_A1_Q, "vabd VABD_f_A1_Q", {OPR_reg_Qd, OPR_reg_Qn, OPR_reg_Qm}, 0xf3200d40, 0xffa00f50},
    {aarch32_op_VABD_i_A1_D, "vabd VABD_i_A1_D", {OPR_reg_Dd, OPR_reg_Dn, OPR_reg_Dm}, 0xf2000700, 0xfe800f50},
    {aarch32_op_VABD_i_A1_Q, "vabd VABD_i_A1_Q", {OPR_reg_Qd, OPR_reg_Qn, OPR_reg_Qm}, 0xf2000740, 0xfe800f50},
    {aarch32_op_VABDL_i_A1, "vabdl VABDL_i_A1", {OPR_reg_Qd, OPR_reg_Dn, OPR_reg_Dm}, 0xf2800700, 0xfe800f50},
    {aarch32_op_VABS_A1_D, "vabs VABS_A1_D", {OPR_reg_Dd, OPR_reg_Dm}, 0xf3b10300, 0xffb30bd0},
    {aarch32_op_VABS_A1_Q, "vabs VABS_A1_Q", {OPR_reg_Qd, OPR_reg_Qm}, 0xf3b10340, 0xffb30bd0},
    {aarch32_op_VABS_A2_H, "vabs VABS_A2_H", {OPR_reg_Sd, OPR_reg_Sm}, 0x0eb009c0, 0x0fbf0fd0},
    {aarch32_op_VABS_A2_S, "vabs VABS_A2_S", {OPR_reg_Sd, OPR_reg_Sm}, 0x0eb00ac0, 0x0fbf0fd0},
    {aarch32_op_VABS_A2_D, "vabs VABS_A2_D", {OPR_reg_Dd, OPR_reg_Dm}, 0x0eb00bc0, 0x0fbf0fd0},
    {aarch32_op_VACGE_A1_D, "vacge VACGE_A1_D", {OPR_reg_Dd, OPR_reg_Dn, OPR_reg_Dm}, 0xf3000e10, 0xffa00f50},
    {aarch32_op_VACGE_A1_Q, "vacge VACGE_A1_Q", {OPR_reg_Qd, OPR_reg_Qn, OPR_reg_Qm}, 0xf3000e50, 0xffa00f50},
    {aarch32_op_VACGT_A1_D, "vacgt VACGT_A1_D", {OPR_reg_Dd, OPR_reg_Dn, OPR_reg_Dm}, 0xf3200e10, 0xffa00f50},
    {aarch32_op_VACGT_A1_Q, "vacgt VACGT_A1_Q", {OPR_reg_Qd, OPR_reg_Qn, OPR_reg_Qm}, 0xf3200e50, 0xffa00f50},
    {aarch32_op_VACLE_VACGE_A1_D, "vacle VACLE_VACGE_A1_D", {OPR_reg_Dd, OPR_reg_Dn, OPR_reg_Dm}, 0xf3000e10, 0xffa00f50},
    {aarch32_op_VACLE_VACGE_A1_Q, "vacle VACLE_VACGE_A1_Q", {OPR_reg_Qd, OPR_reg_Qn, OPR_reg_Qm}, 0xf3000e50, 0xffa00f50},
    {aarch32_op_VACLT_VACGT_A1_D, "vaclt VACLT_VACGT_A1_D", {OPR_reg_Dd, OPR_reg_Dn, OPR_reg_Dm}, 0xf3200e10, 0xffa00f50},
    {aarch32_op_VACLT_VACGT_A1_Q, "vaclt VACLT_VACGT_A1_Q", {OPR_reg_Qd, OPR_reg_Qn, OPR_reg_Qm}, 0xf3200e50, 0xffa00f50},
    {aarch32_op_VADD_f_A1_D, "vadd VADD_f_A1_D", {OPR_reg_Dd, OPR_reg_Dn, OPR_reg_Dm}, 0xf2000d00, 0xffa00f50},
    {aarch32_op_VADD_f_A1_Q, "vadd VADD_f_A1_Q", {OPR_reg_Qd, OPR_reg_Qn, OPR_reg_Qm}, 0xf2000d40, 0xffa00f50},
    {aarch32_op_VADD_f_A2_H, "vadd VADD_f_A2_H", {OPR_reg_Sd, OPR_reg_Sn, OPR_reg_Sm}, 0x0e300900, 0x0fb00f50},
    {aarch32_op_VADD_f_A2_S, "vadd VADD_f_A2_S", {OPR_reg_Sd, OPR_reg_Sn, OPR_reg_Sm}, 0x0e300a00, 0x0fb00f50},
    {aarch32_op_VADD_f_A2_D, "vadd VADD_f_A2_D", {OPR_reg_Dd, OPR_reg_Dn, OPR_reg_Dm}, 0x0e300b00, 0x0fb00f50},
    {aarch32_op_VADD_i_A1_D, "vadd VADD_i_A1_D", {OPR_reg_Dd, OPR_reg_Dn, OPR_reg_Dm}, 0xf2000800, 0xff800f50},
    {aarch32_op_VADD_i_A1_Q, "vadd VADD_i_A1_Q", {OPR_reg_Qd, OPR_reg_Qn, OPR_reg_Qm}, 0xf2000840, 0xff800f50},
    {aarch32_op_VADDHN_A1, "vaddhn VADDHN_A1", {OPR_reg_Dd, OPR_reg_Qn, OPR_reg_Qm}, 0xf2800400, 0xff800f50},
    {aarch32_op_VADDL_A1, "vaddl VADDL_A1", {OPR_reg_Qd, OPR_reg_Dn, OPR_reg_Dm}, 0xf2800000, 0xfe800f50},
    {aarch32_op_VADDW_A1, "vaddw VADDW_A1", {OPR_reg_Qd, OPR_reg_Qn, OPR_reg_Dm}, 0xf2800100, 0xfe800f50},
    {aarch32_op_VAND_r_A1_D, "vand VAND_r_A1_D", {OPR_reg_Dd, OPR_reg_Dn, OPR_reg_Dm}, 0xf2000110, 0xffb00f50},
    {aarch32_op_VAND_r_A1_Q, "vand VAND_r_A1_Q", {OPR_reg_Qd, OPR_reg_Qn, OPR_reg_Qm}, 0xf2000150, 0xffb00f50},
    {aarch32_op_VAND_VBIC_i_A1_D, "vand VAND_VBIC_i_A1_D", {OPR_reg_Dd, OPR_reg_Dd}, 0xf2800130, 0xfeb809f0},
    {aarch32_op_VAND_VBIC_i_A1_Q, "vand VAND_VBIC_i_A1_Q", {OPR_reg_Qd, OPR_reg_Qd}, 0xf2800170, 0xfeb809f0},
    {aarch32_op_VAND_VBIC_i_A2_D, "vand VAND_VBIC_i_A2_D", {OPR_reg_Dd, OPR_reg_Dd}, 0xf2800930, 0xfeb80df0},
    {aarch32_op_VAND_VBIC_i_A2_Q, "vand VAND_VBIC_i_A2_Q", {OPR_reg_Qd, OPR_reg_Qd}, 0xf2800970, 0xfeb80df0},
    {aarch32_op_VBIC_i_A1_D, "vbic VBIC_i_A1_D", {OPR_reg_Dd, OPR_reg_Dd}, 0xf2800130, 0xfeb809f0},
    {aarch32_op_VBIC_i_A1_Q, "vbic VBIC_i_A1_Q", {OPR_reg_Qd, OPR_reg_Qd}, 0xf2800170, 0xfeb809f0},
    {aarch32_op_VBIC_i_A2_D, "vbic VBIC_i_A2_D", {OPR_reg_Dd, OPR_reg_Dd}, 0xf2800930, 0xfeb80df0},
    {aarch32_op_VBIC_i_A2_Q, "vbic VBIC_i_A2_Q", {OPR_reg_Qd, OPR_reg_Qd}, 0xf2800970, 0xfeb80df0},
    {aarch32_op_VBIC_r_A1_D, "vbic VBIC_r_A1_D", {OPR_reg_Dd, OPR_reg_Dn, OPR_reg_Dm}, 0xf2100110, 0xffb00f50},
    {aarch32_op_VBIC_r_A1_Q, "vbic VBIC_r_A1_Q", {OPR_reg_Qd, OPR_reg_Qn, OPR_reg_Qm}, 0xf2100150, 0xffb00f50},
    {aarch32_op_VBIF_A1_D, "vbif VBIF_A1_D", {OPR_reg_Dd, OPR_reg_Dn, OPR_reg_Dm}, 0xf3300110, 0xffb00f50},
    {aarch32_op_VBIF_A1_Q, "vbif VBIF_A1_Q", {OPR_reg_Qd, OPR_reg_Qn, OPR_reg_Qm}, 0xf3300150, 0xffb00f50},
    {aarch32_op_VBIT_A1_D, "vbit VBIT_A1_D", {OPR_reg_Dd, OPR_reg_Dn, OPR_reg_Dm}, 0xf3200110, 0xffb00f50},
    {aarch32_op_VBIT_A1_Q, "vbit VBIT_A1_Q", {OPR_reg_Qd, OPR_reg_Qn, OPR_reg_Qm}, 0xf3200150, 0xffb00f50},
    {aarch32_op_VBSL_A1_D, "vbsl VBSL_A1_D", {OPR_reg_Dd, OPR_reg_Dn, OPR_reg_Dm}, 0xf3100110, 0xffb00f50},
    {aarch32_op_VBSL_A1_Q, "vbsl VBSL_A1_Q", {OPR_reg_Qd, OPR_reg_Qn, OPR_reg_Qm}, 0xf3100150, 0xffb00f50},
    {aarch32_op_VCEQ_i_A1_D, "vceq VCEQ_i_A1_D", {OPR_reg_Dd, OPR_reg_Dm}, 0xf3b10100, 0xffb30bd0},
    {aarch32_op_VCEQ_i_A1_Q, "vceq VCEQ_i_A1_Q", {OPR_reg_Qd, OPR_reg_Qm}, 0xf3b10140, 0xffb30bd0},
    {aarch32_op_VCEQ_r_A1_D, "vceq VCEQ_r_A1_D", {OPR_reg_Dd, OPR_reg_Dn, OPR_reg_Dm}, 0xf3000810, 0xff800f50},
    {aarch32_op_VCEQ_r_A1_Q, "vceq VCEQ_r_A1_Q", {OPR_reg_Qd, OPR_reg_Qn, OPR_reg_Qm}, 0xf3000850, 0xff800f50},
    {aarch32_op_VCEQ_r_A2_D, "vceq VCEQ_r_A2_D", {OPR_reg_Dd, OPR_reg_Dn, OPR_reg_Dm}, 0xf2000e00, 0xffa00f50},
    {aarch32_op_VCEQ_r_A2_Q, "vceq VCEQ_r_A2_Q", {OPR_reg_Qd, OPR_reg_Qn, OPR_reg_Qm}, 0xf2000e40, 0xffa00f50},
    {aarch32_op_VCGE_i_A1_D, "vcge VCGE_i_A1_D", {OPR_reg_Dd, OPR_reg_Dm}, 0xf3b10080, 0xffb30bd0},
    {aarch32_op_VCGE_i_A1_Q, "vcge VCGE_i_A1_Q", {OPR_reg_Qd, OPR_reg_Qm}, 0xf3b100c0, 0xffb30bd0},
    {aarch32_op_VCGE_r_A1_D, "vcge VCGE_r_A1_D", {OPR_reg_Dd, OPR_reg_Dn, OPR_reg_Dm}, 0xf2000310, 0xfe800f50},
    {aarch32_op_VCGE_r_A1_Q, "vcge VCGE_r_A1_Q", {OPR_reg_Qd, OPR_reg_Qn, OPR_reg_Qm}, 0xf2000350, 0xfe800f50},
    {aarch32_op_VCGE_r_A2_D, "vcge VCGE_r_A2_D", {OPR_reg_Dd, OPR_reg_Dn, OPR_reg_Dm}, 0xf3000e00, 0xffa00f50},
    {aarch32_op_VCGE_r_A2_Q, "vcge VCGE_r_A2_Q", {OPR_reg_Qd, OPR_reg_Qn, OPR_reg_Qm}, 0xf3000e40, 0xffa00f50},
    {aarch32_op_VCGT_i_A1_D, "vcgt VCGT_i_A1_D", {OPR_reg_Dd, OPR_reg_Dm}, 0xf3b10000, 0xffb30bd0},
    {aarch32_op_VCGT_i_A1_Q, "vcgt VCGT_i_A1_Q", {OPR_reg_Qd, OPR_reg_Qm}, 0xf3b10040, 0xffb30bd0},
    {aarch32_op_VCGT_r_A1_D, "vcgt VCGT_r_A1_D", {OPR_reg_Dd, OPR_reg_Dn, OPR_reg_Dm}, 0xf2000300, 0xfe800f50},
    {aarch32_op_VCGT_r_A1_Q, "vcgt VCGT_r_A1_Q", {OPR_reg_Qd, OPR_reg_Qn, OPR_reg_Qm}, 0xf2000340, 0xfe800f50},
    {aarch32_op_VCGT_r_A2_D, "vcgt VCGT_r_A2_D", {OPR_reg_Dd, OPR_reg_Dn, OPR_reg_Dm}, 0xf3200e00, 0xffa00f50},
    {aarch32_op_VCGT_r_A2_Q, "vcgt VCGT_r_A2_Q", {OPR_reg_Qd, OPR_reg_Qn, OPR_reg_Qm}, 0xf3200e40, 0xffa00f50},
    {aarch32_op_VCLE_i_A1_D, "vcle VCLE_i_A1_D", {OPR_reg_Dd, OPR_reg_Dm}, 0xf3b10180, 0xffb30bd0},
    {aarch32_op_VCLE_i_A1_Q, "vcle VCLE_i_A1_Q", {OPR_reg_Qd, OPR_reg_Qm}, 0xf3b101c0, 0xffb30bd0},
    {aarch32_op_VCLE_VCGE_r_A1_D, "vcle VCLE_VCGE_r_A1_D", {OPR_reg_Dd, OPR_reg_Dn, OPR_reg_Dm}, 0xf2000310, 0xfe800f50},
    {aarch32_op_VCLE_VCGE_r_A1_Q, "vcle VCLE_VCGE_r_A1_Q", {OPR_reg_Qd, OPR_reg_Qn, OPR_reg_Qm}, 0xf2000350, 0xfe800f50},
    {aarch32_op_VCLE_VCGE_r_A2_D, "vcle VCLE_VCGE_r_A2_D", {OPR_reg_Dd, OPR_reg_Dn, OPR_reg_Dm}, 0xf3000e00, 0xffa00f50},
    {aarch32_op_VCLE_VCGE_r_A2_Q, "vcle VCLE_VCGE_r_A2_Q", {OPR_reg_Qd, OPR_reg_Qn, OPR_reg_Qm}, 0xf3000e40, 0xffa00f50},
    {aarch32_op_VCLS_A1_D, "vcls VCLS_A1_D", {OPR_reg_Dd, OPR_reg_Dm}, 0xf3b00400, 0xffb30fd0},
    {aarch32_op_VCLS_A1_Q, "vcls VCLS_A1_Q", {OPR_reg_Qd, OPR_reg_Qm}, 0xf3b00440, 0xffb30fd0},
    {aarch32_op_VCLT_i_A1_D, "vclt VCLT_i_A1_D", {OPR_reg_Dd, OPR_reg_Dm}, 0xf3b10200, 0xffb30bd0},
    {aarch32_op_VCLT_i_A1_Q, "vclt VCLT_i_A1_Q", {OPR_reg_Qd, OPR_reg_Qm}, 0xf3b10240, 0xffb30bd0},
    {aarch32_op_VCLT_VCGT_r_A1_D, "vclt VCLT_VCGT_r_A1_D", {OPR_reg_Dd, OPR_reg_Dn, OPR_reg_Dm}, 0xf2000300, 0xfe800f50},
    {aarch32_op_VCLT_VCGT_r_A1_Q, "vclt VCLT_VCGT_r_A1_Q", {OPR_reg_Qd, OPR_reg_Qn, OPR_reg_Qm}, 0xf2000340, 0xfe800f50},
    {aarch32_op_VCLT_VCGT_r_A2_D, "vclt VCLT_VCGT_r_A2_D", {OPR_reg_Dd, OPR_reg_Dn, OPR_reg_Dm}, 0xf3200e00, 0xffa00f50},
    {aarch32_op_VCLT_VCGT_r_A2_Q, "vclt VCLT_VCGT_r_A2_Q", {OPR_reg_Qd, OPR_reg_Qn, OPR_reg_Qm}, 0xf3200e40, 0xffa00f50},
    {aarch32_op_VCLZ_A1_D, "vclz VCLZ_A1_D", {OPR_reg_Dd, OPR_reg_Dm}, 0xf3b00480, 0xffb30fd0},
    {aarch32_op_VCLZ_A1_Q, "vclz VCLZ_A1_Q", {OPR_reg_Qd, OPR_reg_Qm}, 0xf3b004c0, 0xffb30fd0},
    {aarch32_op_VCMP_A1_H, "vcmp VCMP_A1_H", {OPR_reg_Sd, OPR_reg_Sm}, 0x0eb40940, 0x0fbf0fd0},
    {aarch32_op_VCMP_A1_S, "vcmp VCMP_A1_S", {OPR_reg_Sd, OPR_reg_Sm}, 0x0eb40a40, 0x0fbf0fd0},
    {aarch32_op_VCMP_A1_D, "vcmp VCMP_A1_D", {OPR_reg_Dd, OPR_reg_Dm}, 0x0eb40b40, 0x0fbf0fd0},
    {aarch32_op_VCMP_A2_H, "vcmp VCMP_A2_H", {OPR_reg_Sd}, 0x0eb50940, 0x0fbf0fd0},
    {aarch32_op_VCMP_A2_S, "vcmp VCMP_A2_S", {OPR_reg_Sd}, 0x0eb50a40, 0x0fbf0fd0},
    {aarch32_op_VCMP_A2_D, "vcmp VCMP_A2_D", {OPR_reg_Dd}, 0x0eb50b40, 0x0fbf0fd0},
    {aarch32_op_VCMPE_A1_H, "vcmpe VCMPE_A1_H", {OPR_reg_Sd, OPR_reg_Sm}, 0x0eb409c0, 0x0fbf0fd0},
    {aarch32_op_VCMPE_A1_S, "vcmpe VCMPE_A1_S", {OPR_reg_Sd, OPR_reg_Sm}, 0x0eb40ac0, 0x0fbf0fd0},
    {aarch32_op_VCMPE_A1_D, "vcmpe VCMPE_A1_D", {OPR_reg_Dd, OPR_reg_Dm}, 0x0eb40bc0, 0x0fbf0fd0},
    {aarch32_op_VCMPE_A2_H, "vcmpe VCMPE_A2_H", {OPR_reg_Sd}, 0x0eb509c0, 0x0fbf0fd0},
    {aarch32_op_VCMPE_A2_S, "vcmpe VCMPE_A2_S", {OPR_reg_Sd}, 0x0eb50ac0, 0x0fbf0fd0},
    {aarch32_op_VCMPE_A2_D, "vcmpe VCMPE_A2_D", {OPR_reg_Dd}, 0x0eb50bc0, 0x0fbf0fd0},
    {aarch32_op_VCNT_A1_D, "vcnt VCNT_A1_D", {OPR_reg_Dd, OPR_reg_Dm}, 0xf3b00500, 0xffb30fd0},
    {aarch32_op_VCNT_A1_Q, "vcnt VCNT_A1_Q", {OPR_reg_Qd, OPR_reg_Qm}, 0xf3b00540, 0xffb30fd0},
    {aarch32_op_VCVT_ds_A1, "vcvt VCVT_ds_A1", {OPR_reg_Dd, OPR_reg_Sm}, 0x0eb70ac0, 0x0fbf0fd0},
    {aarch32_op_VCVT_sd_A1, "vcvt VCVT_sd_A1", {OPR_reg_Sd, OPR_reg_Dm}, 0x0eb70bc0, 0x0fbf0fd0},
    {aarch32_op_VCVT_sh_A1, "vcvt VCVT_sh_A1", {OPR_reg_Qd, OPR_reg_Dm}, 0xf3b20700, 0xffb30fd0},
    {aarch32_op_VCVT_hs_A1, "vcvt VCVT_hs_A1", {OPR_reg_Dd, OPR_reg_Qm}, 0xf3b20600, 0xffb30fd0},
    {aarch32_op_VCVT_is_A1_D, "vcvt VCVT_is_A1_D", {OPR_reg_Dd, OPR_reg_Dm}, 0xf3b30600, 0xffb30e50},
    {aarch32_op_VCVT_is_A1_Q, "vcvt VCVT_is_A1_Q", {OPR_reg_Qd, OPR_reg_Qm}, 0xf3b30640, 0xffb30e50},
    {aarch32_op_VCVT_uiv_A1_H, "vcvt VCVT_uiv_A1_H", {OPR_reg_Sd, OPR_reg_Sm}, 0x0ebc09c0, 0x0fbf0fd0},
    {aarch32_op_VCVT_siv_A1_H, "vcvt VCVT_siv_A1_H", {OPR_reg_Sd, OPR_reg_Sm}, 0x0ebd09c0, 0x0fbf0fd0},
    {aarch32_op_VCVT_uiv_A1_S, "vcvt VCVT_uiv_A1_S", {OPR_reg_Sd, OPR_reg_Sm}, 0x0ebc0ac0, 0x0fbf0fd0},
    {aarch32_op_VCVT_siv_A1_S, "vcvt VCVT_siv_A1_S", {OPR_reg_Sd, OPR_reg_Sm}, 0x0ebd0ac0, 0x0fbf0fd0},
    {aarch32_op_VCVT_uiv_A1_D, "vcvt VCVT_uiv_A1_D", {OPR_reg_Sd, OPR_reg_Dm}, 0x0ebc0bc0, 0x0fbf0fd0},
    {aarch32_op_VCVT_siv_A1_D, "vcvt VCVT_siv_A1_D", {OPR_reg_Sd, OPR_reg_Dm}, 0x0ebd0bc0, 0x0fbf0fd0},
    {aarch32_op_VCVT_vi_A1_H, "vcvt VCVT_vi_A1_H", {OPR_reg_Sd, OPR_reg_Sm}, 0x0eb80940, 0x0fbf0f50},
    {aarch32_op_VCVT_vi_A1_S, "vcvt VCVT_vi_A1_S", {OPR_reg_Sd, OPR_reg_Sm}, 0x0eb80a40, 0x0fbf0f50},
    {aarch32_op_VCVT_vi_A1_D, "vcvt VCVT_vi_A1_D", {OPR_reg_Dd, OPR_reg_Sm}, 0x0eb80b40, 0x0fbf0f50},
    {aarch32_op_VCVT_xs_A1_D, "vcvt VCVT_xs_A1_D", {OPR_reg_Dd, OPR_reg_Dm}, 0xf2800c10, 0xfe800cd0},
    {aarch32_op_VCVT_xs_A1_Q, "vcvt VCVT_xs_A1_Q", {OPR_reg_Qd, OPR_reg_Qm}, 0xf2800c50, 0xfe800cd0},
    {aarch32_op_VCVT_toxv_A1_H, "vcvt VCVT_toxv_A1_H", {OPR_reg_Sdm, OPR_reg_Sdm}, 0x0eba0940, 0x0fbe0f50},
    {aarch32_op_VCVT_xv_A1_H, "vcvt VCVT_xv_A1_H", {OPR_reg_Sdm, OPR_reg_Sdm}, 0x0ebe0940, 0x0fbe0f50},
    {aarch32_op_VCVT_toxv_A1_S, "vcvt VCVT_toxv_A1_S", {OPR_reg_Sdm, OPR_reg_Sdm}, 0x0eba0a40, 0x0fbe0f50},
    {aarch32_op_VCVT_xv_A1_S, "vcvt VCVT_xv_A1_S", {OPR_reg_Sdm, OPR_reg_Sdm}, 0x0ebe0a40, 0x0fbe0f50},
    {aarch32_op_VCVT_toxv_A1_D, "vcvt VCVT_toxv_A1_D", {OPR_reg_Ddm, OPR_reg_Ddm}, 0x0eba0b40, 0x0fbe0f50},
    {aarch32_op_VCVT_xv_A1_D, "vcvt VCVT_xv_A1_D", {OPR_reg_Ddm, OPR_reg_Ddm}, 0x0ebe0b40, 0x0fbe0f50},
    {aarch32_op_VCVTA_asimd_A1_D, "vcvta VCVTA_asimd_A1_D", {OPR_reg_Dd, OPR_reg_Dm}, 0xf3b30000, 0xffb30f50},
    {aarch32_op_VCVTA_asimd_A1_Q, "vcvta VCVTA_asimd_A1_Q", {OPR_reg_Qd, OPR_reg_Qm}, 0xf3b30040, 0xffb30f50},
    {aarch32_op_VCVTA_vfp_A1_H, "vcvta VCVTA_vfp_A1_H", {OPR_reg_Sd, OPR_reg_Sm}, 0xfebc0940, 0xffbf0f50},
    {aarch32_op_VCVTA_vfp_A1_S, "vcvta VCVTA_vfp_A1_S", {OPR_reg_Sd, OPR_reg_Sm}, 0xfebc0a40, 0xffbf0f50},
    {aarch32_op_VCVTA_vfp_A1_D, "vcvta VCVTA_vfp_A1_D", {OPR_reg_Sd, OPR_reg_Dm}, 0xfebc0b40, 0xffbf0f50},
    {aarch32_op_VCVTB_A1_SH, "vcvtb VCVTB_A1_SH", {OPR_reg_Sd, OPR_reg_Sm}, 0x0eb20a40, 0x0fbf0fd0},
    {aarch32_op_VCVTB_A1_DH, "vcvtb VCVTB_A1_DH", {OPR_reg_Dd, OPR_reg_Sm}, 0x0eb20b40, 0x0fbf0fd0},
    {aarch32_op_VCVTB_A1_HS, "vcvtb VCVTB_A1_HS", {OPR_reg_Sd, OPR_reg_Sm}, 0x0eb30a40, 0x0fbf0fd0},
    {aarch32_op_VCVTB_A1_HD, "vcvtb VCVTB_A1_HD", {OPR_reg_Sd, OPR_reg_Dm}, 0x0eb30b40, 0x0fbf0fd0},
    {aarch32_op_VCVTM_asimd_A1_D, "vcvtm VCVTM_asimd_A1_D", {OPR_reg_Dd, OPR_reg_Dm}, 0xf3b30300, 0xffb30f50},
    {aarch32_op_VCVTM_asimd_A1_Q, "vcvtm VCVTM_asimd_A1_Q", {OPR_reg_Qd, OPR_reg_Qm}, 0xf3b30340, 0xffb30f50},
    {aarch32_op_VCVTM_vfp_A1_H, "vcvtm VCVTM_vfp_A1_H", {OPR_reg_Sd, OPR_reg_Sm}, 0xfebf0940, 0xffbf0f50},
    {aarch32_op_VCVTM_vfp_A1_S, "vcvtm VCVTM_vfp_A1_S", {OPR_reg_Sd, OPR_reg_Sm}, 0xfebf0a40, 0xffbf0f50},
    {aarch32_op_VCVTM_vfp_A1_D, "vcvtm VCVTM_vfp_A1_D", {OPR_reg_Sd, OPR_reg_Dm}, 0xfebf0b40, 0xffbf0f50},
    {aarch32_op_VCVTN_asimd_A1_D, "vcvtn VCVTN_asimd_A1_D", {OPR_reg_Dd, OPR_reg_Dm}, 0xf3b30100, 0xffb30f50},
    {aarch32_op_VCVTN_asimd_A1_Q, "vcvtn VCVTN_asimd_A1_Q", {OPR_reg_Qd, OPR_reg_Qm}, 0xf3b30140, 0xffb30f50},
    {aarch32_op_VCVTN_vfp_A1_H, "vcvtn VCVTN_vfp_A1_H", {OPR_reg_Sd, OPR_reg_Sm}, 0xfebd0940, 0xffbf0f50},
    {aarch32_op_VCVTN_vfp_A1_S, "vcvtn VCVTN_vfp_A1_S", {OPR_reg_Sd, OPR_reg_Sm}, 0xfebd0a40, 0xffbf0f50},
    {aarch32_op_VCVTN_vfp_A1_D, "vcvtn VCVTN_vfp_A1_D", {OPR_reg_Sd, OPR_reg_Dm}, 0xfebd0b40, 0xffbf0f50},
    {aarch32_op_VCVTP_asimd_A1_D, "vcvtp VCVTP_asimd_A1_D", {OPR_reg_Dd, OPR_reg_Dm}, 0xf3b30200, 0xffb30f50},
    {aarch32_op_VCVTP_asimd_A1_Q, "vcvtp VCVTP_asimd_A1_Q", {OPR_reg_Qd, OPR_reg_Qm}, 0xf3b30240, 0xffb30f50},
    {aarch32_op_VCVTP_vfp_A1_H, "vcvtp VCVTP_vfp_A1_H", {OPR_reg_Sd, OPR_reg_Sm}, 0xfebe0940, 0xffbf0f50},
    {aarch32_op_VCVTP_vfp_A1_S, "vcvtp VCVTP_vfp_A1_S", {OPR_reg_Sd, OPR_reg_Sm}, 0xfebe0a40, 0xffbf0f50},
    {aarch32_op_VCVTP_vfp_A1_D, "vcvtp VCVTP_vfp_A1_D", {OPR_reg_Sd, OPR_reg_Dm}, 0xfebe0b40, 0xffbf0f50},
    {aarch32_op_VCVTR_uiv_A1_H, "vcvtr VCVTR_uiv_A1_H", {OPR_reg_Sd, OPR_reg_Sm}, 0x0ebc0940, 0x0fbf0fd0},
    {aarch32_op_VCVTR_siv_A1_H, "vcvtr VCVTR_siv_A1_H", {OPR_reg_Sd, OPR_reg_Sm}, 0x0ebd0940, 0x0fbf0fd0},
    {aarch32_op_VCVTR_uiv_A1_S, "vcvtr VCVTR_uiv_A1_S", {OPR_reg_Sd, OPR_reg_Sm}, 0x0ebc0a40, 0x0fbf0fd0},
    {aarch32_op_VCVTR_siv_A1_S, "vcvtr VCVTR_siv_A1_S", {OPR_reg_Sd, OPR_reg_Sm}, 0x0ebd0a40, 0x0fbf0fd0},
    {aarch32_op_VCVTR_uiv_A1_D, "vcvtr VCVTR_uiv_A1_D", {OPR_reg_Sd, OPR_reg_Dm}, 0x0ebc0b40, 0x0fbf0fd0},
    {aarch32_op_VCVTR_siv_A1_D, "vcvtr VCVTR_siv_A1_D", {OPR_reg_Sd, OPR_reg_Dm}, 0x0ebd0b40, 0x0fbf0fd0},
    {aarch32_op_VCVTT_A1_SH, "vcvtt VCVTT_A1_SH", {OPR_reg_Sd, OPR_reg_Sm}, 0x0eb20ac0, 0x0fbf0fd0},
    {aarch32_op_VCVTT_A1_DH, "vcvtt VCVTT_A1_DH", {OPR_reg_Dd, OPR_reg_Sm}, 0x0eb20bc0, 0x0fbf0fd0},
    {aarch32_op_VCVTT_A1_HS, "vcvtt VCVTT_A1_HS", {OPR_reg_Sd, OPR_reg_Sm}, 0x0eb30ac0, 0x0fbf0fd0},
    {aarch32_op_VCVTT_A1_HD, "vcvtt VCVTT_A1_HD", {OPR_reg_Sd, OPR_reg_Dm}, 0x0eb30bc0, 0x0fbf0fd0},
    {aarch32_op_VDIV_A1_H, "vdiv VDIV_A1_H", {OPR_reg_Sd, OPR_reg_Sn, OPR_reg_Sm}, 0x0e800900, 0x0fb00f50},
    {aarch32_op_VDIV_A1_S, "vdiv VDIV_A1_S", {OPR_reg_Sd, OPR_reg_Sn, OPR_reg_Sm}, 0x0e800a00, 0x0fb00f50},
    {aarch32_op_VDIV_A1_D, "vdiv VDIV_A1_D", {OPR_reg_Dd, OPR_reg_Dn, OPR_reg_Dm}, 0x0e800b00, 0x0fb00f50},
    {aarch32_op_VDUP_r_A1, "vdup VDUP_r_A1", {OPR_reg_Qd_2, OPR_reg_Rt}, 0x0e800b10, 0x0f900f50},
    {aarch32_op_VDUP_s_A1_D, "vdup VDUP_s_A1_D", {OPR_reg_Dd, OPR_reg_Dm_x_}, 0xf3b00c00, 0xffb00fd0},
    {aarch32_op_VDUP_s_A1_Q, "vdup VDUP_s_A1_Q", {OPR_reg_Qd, OPR_reg_Dm_x_}, 0xf3b00c40, 0xffb00fd0},
    {aarch32_op_VEOR_A1_D, "veor VEOR_A1_D", {OPR_reg_Dd, OPR_reg_Dn, OPR_reg_Dm}, 0xf3000110, 0xffb00f50},
    {aarch32_op_VEOR_A1_Q, "veor VEOR_A1_Q", {OPR_reg_Qd, OPR_reg_Qn, OPR_reg_Qm}, 0xf3000150, 0xffb00f50},
    {aarch32_op_VEXT_A1_D, "vext VEXT_A1_D", {OPR_reg_Dd, OPR_reg_Dn, OPR_reg_Dm}, 0xf2b00000, 0xffb00050},
    {aarch32_op_VEXT_A1_Q, "vext VEXT_A1_Q", {OPR_reg_Qd, OPR_reg_Qn, OPR_reg_Qm}, 0xf2b00040, 0xffb00050},
    {aarch32_op_VEXT_VEXT_A1_D, "vext VEXT_VEXT_A1_D", {OPR_reg_Dd, OPR_reg_Dn, OPR_reg_Dm}, 0xf2b00000, 0xffb00050},
    {aarch32_op_VEXT_VEXT_A1_Q, "vext VEXT_VEXT_A1_Q", {OPR_reg_Qd, OPR_reg_Qn, OPR_reg_Qm}, 0xf2b00040, 0xffb00050},
    {aarch32_op_VFMA_A1_D, "vfma VFMA_A1_D", {OPR_reg_Dd, OPR_reg_Dn, OPR_reg_Dm}, 0xf2000c10, 0xffa00f50},
    {aarch32_op_VFMA_A1_Q, "vfma VFMA_A1_Q", {OPR_reg_Qd, OPR_reg_Qn, OPR_reg_Qm}, 0xf2000c50, 0xffa00f50},
    {aarch32_op_VFMA_A2_H, "vfma VFMA_A2_H", {OPR_reg_Sd, OPR_reg_Sn, OPR_reg_Sm}, 0x0ea00900, 0x0fb00f50},
    {aarch32_op_VFMA_A2_S, "vfma VFMA_A2_S", {OPR_reg_Sd, OPR_reg_Sn, OPR_reg_Sm}, 0x0ea00a00, 0x0fb00f50},
    {aarch32_op_VFMA_A2_D, "vfma VFMA_A2_D", {OPR_reg_Dd, OPR_reg_Dn, OPR_reg_Dm}, 0x0ea00b00, 0x0fb00f50},
    {aarch32_op_VFMS_A1_D, "vfms VFMS_A1_D", {OPR_reg_Dd, OPR_reg_Dn, OPR_reg_Dm}, 0xf2200c10, 0xffa00f50},
    {aarch32_op_VFMS_A1_Q, "vfms VFMS_A1_Q", {OPR_reg_Qd, OPR_reg_Qn, OPR_reg_Qm}, 0xf2200c50, 0xffa00f50},
    {aarch32_op_VFMS_A2_H, "vfms VFMS_A2_H", {OPR_reg_Sd, OPR_reg_Sn, OPR_reg_Sm}, 0x0ea00940, 0x0fb00f50},
    {aarch32_op_VFMS_A2_S, "vfms VFMS_A2_S", {OPR_reg_Sd, OPR_reg_Sn, OPR_reg_Sm}, 0x0ea00a40, 0x0fb00f50},
    {aarch32_op_VFMS_A2_D, "vfms VFMS_A2_D", {OPR_reg_Dd, OPR_reg_Dn, OPR_reg_Dm}, 0x0ea00b40, 0x0fb00f50},
    {aarch32_op_VFNMA_A1_H, "vfnma VFNMA_A1_H", {OPR_reg_Sd, OPR_reg_Sn, OPR_reg_Sm}, 0x0e900940, 0x0fb00f50},
    {aarch32_op_VFNMA_A1_S, "vfnma VFNMA_A1_S", {OPR_reg_Sd, OPR_reg_Sn, OPR_reg_Sm}, 0x0e900a40, 0x0fb00f50},
    {aarch32_op_VFNMA_A1_D, "vfnma VFNMA_A1_D", {OPR_reg_Dd, OPR_reg_Dn, OPR_reg_Dm}, 0x0e900b40, 0x0fb00f50},
    {aarch32_op_VFNMS_A1_H, "vfnms VFNMS_A1_H", {OPR_reg_Sd, OPR_reg_Sn, OPR_reg_Sm}, 0x0e900900, 0x0fb00f50},
    {aarch32_op_VFNMS_A1_S, "vfnms VFNMS_A1_S", {OPR_reg_Sd, OPR_reg_Sn, OPR_reg_Sm}, 0x0e900a00, 0x0fb00f50},
    {aarch32_op_VFNMS_A1_D, "vfnms VFNMS_A1_D", {OPR_reg_Dd, OPR_reg_Dn, OPR_reg_Dm}, 0x0e900b00, 0x0fb00f50},
    {aarch32_op_VHADD_A1_D, "vhadd VHADD_A1_D", {OPR_reg_Dd, OPR_reg_Dn, OPR_reg_Dm}, 0xf2000000, 0xfe800f50},
    {aarch32_op_VHADD_A1_Q, "vhadd VHADD_A1_Q", {OPR_reg_Qd, OPR_reg_Qn, OPR_reg_Qm}, 0xf2000040, 0xfe800f50},
    {aarch32_op_VHSUB_A1_D, "vhsub VHSUB_A1_D", {OPR_reg_Dd, OPR_reg_Dn, OPR_reg_Dm}, 0xf2000200, 0xfe800f50},
    {aarch32_op_VHSUB_A1_Q, "vhsub VHSUB_A1_Q", {OPR_reg_Qd, OPR_reg_Qn, OPR_reg_Qm}, 0xf2000240, 0xfe800f50},
    {aarch32_op_VINS_A1, "vins VINS_A1", {OPR_reg_Sd, OPR_reg_Sm}, 0xfeb00ac0, 0xffbf0fd0},
    {aarch32_op_VLD1_1_A1_nowb, "vld1 VLD1_1_A1_nowb", {OPR_reg_Rn}, 0xf4a0000f, 0xffb00f0f},
    {aarch32_op_VLD1_1_A1_posti, "vld1 VLD1_1_A1_posti", {OPR_reg_Rn}, 0xf4a0000d, 0xffb00f0f},
    {aarch32_op_VLD1_1_A1_postr, "vld1 VLD1_1_A1_postr", {OPR_reg_Rn, OPR_reg_Rm}, 0xf4a00000, 0xffb00f00},
    {aarch32_op_VLD1_1_A2_nowb, "vld1 VLD1_1_A2_nowb", {OPR_reg_Rn}, 0xf4a0040f, 0xffb00f0f},
    {aarch32_op_VLD1_1_A2_posti, "vld1 VLD1_1_A2_posti", {OPR_reg_Rn}, 0xf4a0040d, 0xffb00f0f},
    {aarch32_op_VLD1_1_A2_postr, "vld1 VLD1_1_A2_postr", {OPR_reg_Rn, OPR_reg_Rm}, 0xf4a00400, 0xffb00f00},
    {aarch32_op_VLD1_1_A3_nowb, "vld1 VLD1_1_A3_nowb", {OPR_reg_Rn}, 0xf4a0080f, 0xffb00f0f},
    {aarch32_op_VLD1_1_A3_posti, "vld1 VLD1_1_A3_posti", {OPR_reg_Rn}, 0xf4a0080d, 0xffb00f0f},
    {aarch32_op_VLD1_1_A3_postr, "vld1 VLD1_1_A3_postr", {OPR_reg_Rn, OPR_reg_Rm}, 0xf4a00800, 0xffb00f00},
    {aarch32_op_VLD1_a_A1_nowb, "vld1 VLD1_a_A1_nowb", {OPR_reg_Rn}, 0xf4a00c0f, 0xffb00f0f},
    {aarch32_op_VLD1_a_A1_posti, "vld1 VLD1_a_A1_posti", {OPR_reg_Rn}, 0xf4a00c0d, 0xffb00f0f},
    {aarch32_op_VLD1_a_A1_postr, "vld1 VLD1_a_A1_postr", {OPR_reg_Rn, OPR_reg_Rm}, 0xf4a00c00, 0xffb00f00},
    {aarch32_op_VLD1_m_A1_nowb, "vld1 VLD1_m_A1_nowb", {OPR_reg_Rn}, 0xf420070f, 0xffb00f0f},
    {aarch32_op_VLD1_m_A1_posti, "vld1 VLD1_m_A1_posti", {OPR_reg_Rn}, 0xf420070d, 0xffb00f0f},
    {aarch32_op_VLD1_m_A1_postr, "vld1 VLD1_m_A1_postr", {OPR_reg_Rn, OPR_reg_Rm}, 0xf4200700, 0xffb00f00},
    {aarch32_op_VLD1_m_A2_nowb, "vld1 VLD1_m_A2_nowb", {OPR_reg_Rn}, 0xf4200a0f, 0xffb00f0f},
    {aarch32_op_VLD1_m_A2_posti, "vld1 VLD1_m_A2_posti", {OPR_reg_Rn}, 0xf4200a0d, 0xffb00f0f},
    {aarch32_op_VLD1_m_A2_postr, "vld1 VLD1_m_A2_postr", {OPR_reg_Rn, OPR_reg_Rm}, 0xf4200a00, 0xffb00f00},
    {aarch32_op_VLD1_m_A3_nowb, "vld1 VLD1_m_A3_nowb", {OPR_reg_Rn}, 0xf420060f, 0xffb00f0f},
    {aarch32_op_VLD1_m_A3_posti, "vld1 VLD1_m_A3_posti", {OPR_reg_Rn}, 0xf420060d, 0xffb00f0f},
    {aarch32_op_VLD1_m_A3_postr, "vld1 VLD1_m_A3_postr", {OPR_reg_Rn, OPR_reg_Rm}, 0xf4200600, 0xffb00f00},
    {aarch32_op_VLD1_m_A4_nowb, "vld1 VLD1_m_A4_nowb", {OPR_reg_Rn}, 0xf420020f, 0xffb00f0f},
    {aarch32_op_VLD1_m_A4_posti, "vld1 VLD1_m_A4_posti", {OPR_reg_Rn}, 0xf420020d, 0xffb00f0f},
    {aarch32_op_VLD1_m_A4_postr, "vld1 VLD1_m_A4_postr", {OPR_reg_Rn, OPR_reg_Rm}, 0xf4200200, 0xffb00f00},
    {aarch32_op_VLD2_1_A1_nowb, "vld2 VLD2_1_A1_nowb", {OPR_reg_Rn}, 0xf4a0010f, 0xffb00f0f},
    {aarch32_op_VLD2_1_A1_posti, "vld2 VLD2_1_A1_posti", {OPR_reg_Rn}, 0xf4a0010d, 0xffb00f0f},
    {aarch32_op_VLD2_1_A1_postr, "vld2 VLD2_1_A1_postr", {OPR_reg_Rn, OPR_reg_Rm}, 0xf4a00100, 0xffb00f00},
    {aarch32_op_VLD2_1_A2_nowb, "vld2 VLD2_1_A2_nowb", {OPR_reg_Rn}, 0xf4a0050f, 0xffb00f0f},
    {aarch32_op_VLD2_1_A2_posti, "vld2 VLD2_1_A2_posti", {OPR_reg_Rn}, 0xf4a0050d, 0xffb00f0f},
    {aarch32_op_VLD2_1_A2_postr, "vld2 VLD2_1_A2_postr", {OPR_reg_Rn, OPR_reg_Rm}, 0xf4a00500, 0xffb00f00},
    {aarch32_op_VLD2_1_A3_nowb, "vld2 VLD2_1_A3_nowb", {OPR_reg_Rn}, 0xf4a0090f, 0xffb00f0f},
    {aarch32_op_VLD2_1_A3_posti, "vld2 VLD2_1_A3_posti", {OPR_reg_Rn}, 0xf4a0090d, 0xffb00f0f},
    {aarch32_op_VLD2_1_A3_postr, "vld2 VLD2_1_A3_postr", {OPR_reg_Rn, OPR_reg_Rm}, 0xf4a00900, 0xffb00f00},
    {aarch32_op_VLD2_a_A1_nowb, "vld2 VLD2_a_A1_nowb", {OPR_reg_Rn}, 0xf4a00d0f, 0xffb00f0f},
    {aarch32_op_VLD2_a_A1_posti, "vld2 VLD2_a_A1_posti", {OPR_reg_Rn}, 0xf4a00d0d, 0xffb00f0f},
    {aarch32_op_VLD2_a_A1_postr, "vld2 VLD2_a_A1_postr", {OPR_reg_Rn, OPR_reg_Rm}, 0xf4a00d00, 0xffb00f00},
    {aarch32_op_VLD2_m_A1_nowb, "vld2 VLD2_m_A1_nowb", {OPR_reg_Rn}, 0xf420080f, 0xffb00e0f},
    {aarch32_op_VLD2_m_A1_posti, "vld2 VLD2_m_A1_posti", {OPR_reg_Rn}, 0xf420080d, 0xffb00e0f},
    {aarch32_op_VLD2_m_A1_postr, "vld2 VLD2_m_A1_postr", {OPR_reg_Rn, OPR_reg_Rm}, 0xf4200800, 0xffb00e00},
    {aarch32_op_VLD2_m_A2_nowb, "vld2 VLD2_m_A2_nowb", {OPR_reg_Rn}, 0xf420030f, 0xffb00f0f},
    {aarch32_op_VLD2_m_A2_posti, "vld2 VLD2_m_A2_posti", {OPR_reg_Rn}, 0xf420030d, 0xffb00f0f},
    {aarch32_op_VLD2_m_A2_postr, "vld2 VLD2_m_A2_postr", {OPR_reg_Rn, OPR_reg_Rm}, 0xf4200300, 0xffb00f00},
    {aarch32_op_VLD3_1_A1_nowb, "vld3 VLD3_1_A1_nowb", {OPR_reg_Rn}, 0xf4a0020f, 0xffb00f0f},
    {aarch32_op_VLD3_1_A1_posti, "vld3 VLD3_1_A1_posti", {OPR_reg_Rn}, 0xf4a0020d, 0xffb00f0f},
    {aarch32_op_VLD3_1_A1_postr, "vld3 VLD3_1_A1_postr", {OPR_reg_Rn, OPR_reg_Rm}, 0xf4a00200, 0xffb00f00},
    {aarch32_op_VLD3_1_A2_nowb, "vld3 VLD3_1_A2_nowb", {OPR_reg_Rn}, 0xf4a0060f, 0xffb00f0f},
    {aarch32_op_VLD3_1_A2_posti, "vld3 VLD3_1_A2_posti", {OPR_reg_Rn}, 0xf4a0060d, 0xffb00f0f},
    {aarch32_op_VLD3_1_A2_postr, "vld3 VLD3_1_A2_postr", {OPR_reg_Rn, OPR_reg_Rm}, 0xf4a00600, 0xffb00f00},
    {aarch32_op_VLD3_1_A3_nowb, "vld3 VLD3_1_A3_nowb", {OPR_reg_Rn}, 0xf4a00a0f, 0xffb00f0f},
    {aarch32_op_VLD3_1_A3_posti, "vld3 VLD3_1_A3_posti", {OPR_reg_Rn}, 0xf4a00a0d, 0xffb00f0f},
    {aarch32_op_VLD3_1_A3_postr, "vld3 VLD3_1_A3_postr", {OPR_reg_Rn, OPR_reg_Rm}, 0xf4a00a00, 0xffb00f00},
    {aarch32_op_VLD3_a_A1_nowb, "vld3 VLD3_a_A1_nowb", {OPR_reg_Rn}, 0xf4a00e0f, 0xffb00f1f},
    {aarch32_op_VLD3_a_A1_posti, "vld3 VLD3_a_A1_posti", {OPR_reg_Rn}, 0xf4a00e0d, 0xffb00f1f},
    {aarch32_op_VLD3_a_A1_postr, "vld3 VLD3_a_A1_postr", {OPR_reg_Rn, OPR_reg_Rm}, 0xf4a00e00, 0xffb00f10},
    {aarch32_op_VLD3_m_A1_nowb, "vld3 VLD3_m_A1_nowb", {OPR_reg_Rn}, 0xf420040f, 0xffb00e0f},
    {aarch32_op_VLD3_m_A1_posti, "vld3 VLD3_m_A1_posti", {OPR_reg_Rn}, 0xf420040d, 0xffb00e0f},
    {aarch32_op_VLD3_m_A1_postr, "vld3 VLD3_m_A1_postr", {OPR_reg_Rn, OPR_reg_Rm}, 0xf4200400, 0xffb00e00},
    {aarch32_op_VLD4_1_A1_nowb, "vld4 VLD4_1_A1_nowb", {OPR_reg_Rn}, 0xf4a0030f, 0xffb00f0f},
    {aarch32_op_VLD4_1_A1_posti, "vld4 VLD4_1_A1_posti", {OPR_reg_Rn}, 0xf4a0030d, 0xffb00f0f},
    {aarch32_op_VLD4_1_A1_postr, "vld4 VLD4_1_A1_postr", {OPR_reg_Rn, OPR_reg_Rm}, 0xf4a00300, 0xffb00f00},
    {aarch32_op_VLD4_1_A2_nowb, "vld4 VLD4_1_A2_nowb", {OPR_reg_Rn}, 0xf4a0070f, 0xffb00f0f},
    {aarch32_op_VLD4_1_A2_posti, "vld4 VLD4_1_A2_posti", {OPR_reg_Rn}, 0xf4a0070d, 0xffb00f0f},
    {aarch32_op_VLD4_1_A2_postr, "vld4 VLD4_1_A2_postr", {OPR_reg_Rn, OPR_reg_Rm}, 0xf4a00700, 0xffb00f00},
    {aarch32_op_VLD4_1_A3_nowb, "vld4 VLD4_1_A3_nowb", {OPR_reg_Rn}, 0xf4a00b0f, 0xffb00f0f},
    {aarch32_op_VLD4_1_A3_posti, "vld4 VLD4_1_A3_posti", {OPR_reg_Rn}, 0xf4a00b0d, 0xffb00f0f},
    {aarch32_op_VLD4_1_A3_postr, "vld4 VLD4_1_A3_postr", {OPR_reg_Rn, OPR_reg_Rm}, 0xf4a00b00, 0xffb00f00},
    {aarch32_op_VLD4_a_A1_nowb, "vld4 VLD4_a_A1_nowb", {OPR_reg_Rn}, 0xf4a00f0f, 0xffb00f0f},
    {aarch32_op_VLD4_a_A1_posti, "vld4 VLD4_a_A1_posti", {OPR_reg_Rn}, 0xf4a00f0d, 0xffb00f0f},
    {aarch32_op_VLD4_a_A1_postr, "vld4 VLD4_a_A1_postr", {OPR_reg_Rn, OPR_reg_Rm}, 0xf4a00f00, 0xffb00f00},
    {aarch32_op_VLD4_m_A1_nowb, "vld4 VLD4_m_A1_nowb", {OPR_reg_Rn}, 0xf420000f, 0xffb00e0f},
    {aarch32_op_VLD4_m_A1_posti, "vld4 VLD4_m_A1_posti", {OPR_reg_Rn}, 0xf420000d, 0xffb00e0f},
    {aarch32_op_VLD4_m_A1_postr, "vld4 VLD4_m_A1_postr", {OPR_reg_Rn, OPR_reg_Rm}, 0xf4200000, 0xffb00e00},
    {aarch32_op_VLDMDB_A1, "vldmdb VLDMDB_A1", {OPR_reg_Rn}, 0x0d300b00, 0x0fb00f01},
    {aarch32_op_VLDM_A1, "vldm VLDM_A1", {OPR_reg_Rn}, 0x0c900b00, 0x0f900f01},
    {aarch32_op_VLDMDB_A2, "vldmdb VLDMDB_A2", {OPR_reg_Rn}, 0x0d300a00, 0x0fb00f00},
    {aarch32_op_VLDM_A2, "vldm VLDM_A2", {OPR_reg_Rn}, 0x0c900a00, 0x0f900f00},
    {aarch32_op_VLDR_A1_H, "vldr VLDR_A1_H", {OPR_reg_Sd, OPR_reg_Rn}, 0x0d100900, 0x0f300f00},
    {aarch32_op_VLDR_A1_S, "vldr VLDR_A1_S", {OPR_reg_Sd, OPR_reg_Rn}, 0x0d100a00, 0x0f300f00},
    {aarch32_op_VLDR_A1_D, "vldr VLDR_A1_D", {OPR_reg_Dd, OPR_reg_Rn}, 0x0d100b00, 0x0f300f00},
    {aarch32_op_VLDR_l_A1_H, "vldr VLDR_l_A1_H", {OPR_reg_Sd, OPR_imm_label_7}, 0x0d1f0900, 0x0f3f0f00},
    {aarch32_op_VLDR_l_A1_S, "vldr VLDR_l_A1_S", {OPR_reg_Sd, OPR_imm_label}, 0x0d1f0a00, 0x0f3f0f00},
    {aarch32_op_VLDR_l_A1_D, "vldr VLDR_l_A1_D", {OPR_reg_Dd, OPR_imm_label}, 0x0d1f0b00, 0x0f3f0f00},
    {aarch32_op_VMAX_f_A1_D, "vmax VMAX_f_A1_D", {OPR_reg_Dd, OPR_reg_Dn, OPR_reg_Dm}, 0xf2000f00, 0xffa00f50},
    {aarch32_op_VMAX_f_A1_Q, "vmax VMAX_f_A1_Q", {OPR_reg_Qd, OPR_reg_Qn, OPR_reg_Qm}, 0xf2000f40, 0xffa00f50},
    {aarch32_op_VMAX_i_A1_D, "vmax VMAX_i_A1_D", {OPR_reg_Dd, OPR_reg_Dn, OPR_reg_Dm}, 0xf2000600, 0xfe800f50},
    {aarch32_op_VMAX_i_A1_Q, "vmax VMAX_i_A1_Q", {OPR_reg_Qd, OPR_reg_Qn, OPR_reg_Qm}, 0xf2000640, 0xfe800f50},
    {aarch32_op_VMAXNM_A1_D, "vmaxnm VMAXNM_A1_D", {OPR_reg_Dd, OPR_reg_Dn, OPR_reg_Dm}, 0xf3000f10, 0xffa00f50},
    {aarch32_op_VMAXNM_A1_Q, "vmaxnm VMAXNM_A1_Q", {OPR_reg_Qd, OPR_reg_Qn, OPR_reg_Qm}, 0xf3000f50, 0xffa00f50},
    {aarch32_op_VMAXNM_A2_H, "vmaxnm VMAXNM_A2_H", {OPR_reg_Sd, OPR_reg_Sn, OPR_reg_Sm}, 0xfe800900, 0xffb00f50},
    {aarch32_op_VMAXNM_A2_S, "vmaxnm VMAXNM_A2_S", {OPR_reg_Sd, OPR_reg_Sn, OPR_reg_Sm}, 0xfe800a00, 0xffb00f50},
    {aarch32_op_VMAXNM_A2_D, "vmaxnm VMAXNM_A2_D", {OPR_reg_Dd, OPR_reg_Dn, OPR_reg_Dm}, 0xfe800b00, 0xffb00f50},
    {aarch32_op_VMIN_f_A1_D, "vmin VMIN_f_A1_D", {OPR_reg_Dd, OPR_reg_Dn, OPR_reg_Dm}, 0xf2200f00, 0xffa00f50},
    {aarch32_op_VMIN_f_A1_Q, "vmin VMIN_f_A1_Q", {OPR_reg_Qd, OPR_reg_Qn, OPR_reg_Qm}, 0xf2200f40, 0xffa00f50},
    {aarch32_op_VMIN_i_A1_D, "vmin VMIN_i_A1_D", {OPR_reg_Dd, OPR_reg_Dn, OPR_reg_Dm}, 0xf2000610, 0xfe800f50},
    {aarch32_op_VMIN_i_A1_Q, "vmin VMIN_i_A1_Q", {OPR_reg_Qd, OPR_reg_Qn, OPR_reg_Qm}, 0xf2000650, 0xfe800f50},
    {aarch32_op_VMINNM_A1_D, "vminnm VMINNM_A1_D", {OPR_reg_Dd, OPR_reg_Dn, OPR_reg_Dm}, 0xf3200f10, 0xffa00f50},
    {aarch32_op_VMINNM_A1_Q, "vminnm VMINNM_A1_Q", {OPR_reg_Qd, OPR_reg_Qn, OPR_reg_Qm}, 0xf3200f50, 0xffa00f50},
    {aarch32_op_VMINNM_A2_H, "vminnm VMINNM_A2_H", {OPR_reg_Sd, OPR_reg_Sn, OPR_reg_Sm}, 0xfe800940, 0xffb00f50},
    {aarch32_op_VMINNM_A2_S, "vminnm VMINNM_A2_S", {OPR_reg_Sd, OPR_reg_Sn, OPR_reg_Sm}, 0xfe800a40, 0xffb00f50},
    {aarch32_op_VMINNM_A2_D, "vminnm VMINNM_A2_D", {OPR_reg_Dd, OPR_reg_Dn, OPR_reg_Dm}, 0xfe800b40, 0xffb00f50},
    {aarch32_op_VMLA_f_A1_D, "vmla VMLA_f_A1_D", {OPR_reg_Dd, OPR_reg_Dn, OPR_reg_Dm}, 0xf2000d10, 0xffa00f50},
    {aarch32_op_VMLA_f_A1_Q, "vmla VMLA_f_A1_Q", {OPR_reg_Qd, OPR_reg_Qn, OPR_reg_Qm}, 0xf2000d50, 0xffa00f50},
    {aarch32_op_VMLA_f_A2_H, "vmla VMLA_f_A2_H", {OPR_reg_Sd, OPR_reg_Sn, OPR_reg_Sm}, 0x0e000900, 0x0fb00f50},
    {aarch32_op_VMLA_f_A2_S, "vmla VMLA_f_A2_S", {OPR_reg_Sd, OPR_reg_Sn, OPR_reg_Sm}, 0x0e000a00, 0x0fb00f50},
    {aarch32_op_VMLA_f_A2_D, "vmla VMLA_f_A2_D", {OPR_reg_Dd, OPR_reg_Dn, OPR_reg_Dm}, 0x0e000b00, 0x0fb00f50},
    {aarch32_op_VMLA_i_A1_D, "vmla VMLA_i_A1_D", {OPR_reg_Dd, OPR_reg_Dn, OPR_reg_Dm}, 0xf2000900, 0xff800f50},
    {aarch32_op_VMLA_i_A1_Q, "vmla VMLA_i_A1_Q", {OPR_reg_Qd, OPR_reg_Qn, OPR_reg_Qm}, 0xf2000940, 0xff800f50},
    {aarch32_op_VMLA_s_A1_D, "vmla VMLA_s_A1_D", {OPR_reg_Dd, OPR_reg_Dn, OPR_reg_Dm_x_}, 0xf2800040, 0xff800e50},
    {aarch32_op_VMLA_s_A1_Q, "vmla VMLA_s_A1_Q", {OPR_reg_Qd, OPR_reg_Qn, OPR_reg_Dm_x_}, 0xf3800040, 0xff800e50},
    {aarch32_op_VMLAL_i_A1, "vmlal VMLAL_i_A1", {OPR_reg_Qd, OPR_reg_Dn, OPR_reg_Dm}, 0xf2800800, 0xfe800f50},
    {aarch32_op_VMLAL_s_A1, "vmlal VMLAL_s_A1", {OPR_reg_Qd, OPR_reg_Dn, OPR_reg_Dm_x_}, 0xf2800240, 0xfe800f50},
    {aarch32_op_VMLS_f_A1_D, "vmls VMLS_f_A1_D", {OPR_reg_Dd, OPR_reg_Dn, OPR_reg_Dm}, 0xf2200d10, 0xffa00f50},
    {aarch32_op_VMLS_f_A1_Q, "vmls VMLS_f_A1_Q", {OPR_reg_Qd, OPR_reg_Qn, OPR_reg_Qm}, 0xf2200d50, 0xffa00f50},
    {aarch32_op_VMLS_f_A2_H, "vmls VMLS_f_A2_H", {OPR_reg_Sd, OPR_reg_Sn, OPR_reg_Sm}, 0x0e000940, 0x0fb00f50},
    {aarch32_op_VMLS_f_A2_S, "vmls VMLS_f_A2_S", {OPR_reg_Sd, OPR_reg_Sn, OPR_reg_Sm}, 0x0e000a40, 0x0fb00f50},
    {aarch32_op_VMLS_f_A2_D, "vmls VMLS_f_A2_D", {OPR_reg_Dd, OPR_reg_Dn, OPR_reg_Dm}, 0x0e000b40, 0x0fb00f50},
    {aarch32_op_VMLS_i_A1_D, "vmls VMLS_i_A1_D", {OPR_reg_Dd, OPR_reg_Dn, OPR_reg_Dm}, 0xf3000900, 0xff800f50},
    {aarch32_op_VMLS_i_A1_Q, "vmls VMLS_i_A1_Q", {OPR_reg_Qd, OPR_reg_Qn, OPR_reg_Qm}, 0xf3000940, 0xff800f50},
    {aarch32_op_VMLS_s_A1_D, "vmls VMLS_s_A1_D", {OPR_reg_Dd, OPR_reg_Dn, OPR_reg_Dm_x_}, 0xf2800440, 0xff800e50},
    {aarch32_op_VMLS_s_A1_Q, "vmls VMLS_s_A1_Q", {OPR_reg_Qd, OPR_reg_Qn, OPR_reg_Dm_x_}, 0xf3800440, 0xff800e50},
    {aarch32_op_VMLSL_i_A1, "vmlsl VMLSL_i_A1", {OPR_reg_Qd, OPR_reg_Dn, OPR_reg_Dm}, 0xf2800a00, 0xfe800f50},
    {aarch32_op_VMLSL_s_A1, "vmlsl VMLSL_s_A1", {OPR_reg_Qd, OPR_reg_Dn, OPR_reg_Dm_x_}, 0xf2800640, 0xfe800f50},
    {aarch32_op_VMOV_tod_A1, "vmov VMOV_tod_A1", {OPR_reg_Dm, OPR_reg_Rt, OPR_reg_Rt2}, 0x0c400b10, 0x0ff00fd0},
    {aarch32_op_VMOV_d_A1, "vmov VMOV_d_A1", {OPR_reg_Rt, OPR_reg_Rt2, OPR_reg_Dm}, 0x0c500b10, 0x0ff00fd0},
    {aarch32_op_VMOV_toh_A1, "vmov VMOV_toh_A1", {OPR_reg_Sn, OPR_reg_Rt}, 0x0e000910, 0x0ff00f10},
    {aarch32_op_VMOV_h_A1, "vmov VMOV_h_A1", {OPR_reg_Rt, OPR_reg_Sn}, 0x0e100910, 0x0ff00f10},
    {aarch32_op_VMOV_i_A1_D, "vmov VMOV_i_A1_D", {OPR_reg_Dd}, 0xf2800010, 0xfeb809f0},
    {aarch32_op_VMOV_i_A1_Q, "vmov VMOV_i_A1_Q", {OPR_reg_Qd}, 0xf2800050, 0xfeb809f0},
    {aarch32_op_VMOV_i_A2_H, "vmov VMOV_i_A2_H", {OPR_reg_Sd}, 0x0eb00900, 0x0fb00f50},
    {aarch32_op_VMOV_i_A2_S, "vmov VMOV_i_A2_S", {OPR_reg_Sd}, 0x0eb00a00, 0x0fb00f50},
    {aarch32_op_VMOV_i_A2_D, "vmov VMOV_i_A2_D", {OPR_reg_Dd}, 0x0eb00b00, 0x0fb00f50},
    {aarch32_op_VMOV_i_A3_D, "vmov VMOV_i_A3_D", {OPR_reg_Dd}, 0xf2800810, 0xfeb80df0},
    {aarch32_op_VMOV_i_A3_Q, "vmov VMOV_i_A3_Q", {OPR_reg_Qd}, 0xf2800850, 0xfeb80df0},
    {aarch32_op_VMOV_i_A4_D, "vmov VMOV_i_A4_D", {OPR_reg_Dd}, 0xf2800c10, 0xfeb80cf0},
    {aarch32_op_VMOV_i_A4_Q, "vmov VMOV_i_A4_Q", {OPR_reg_Qd}, 0xf2800c50, 0xfeb80cf0},
    {aarch32_op_VMOV_i_A5_D, "vmov VMOV_i_A5_D", {OPR_reg_Dd}, 0xf2800e30, 0xfeb80ff0},
    {aarch32_op_VMOV_i_A5_Q, "vmov VMOV_i_A5_Q", {OPR_reg_Qd}, 0xf2800e70, 0xfeb80ff0},
    {aarch32_op_VMOV_r_A2_S, "vmov VMOV_r_A2_S", {OPR_reg_Sd, OPR_reg_Sm}, 0x0eb00a40, 0x0fbf0fd0},
    {aarch32_op_VMOV_r_A2_D, "vmov VMOV_r_A2_D", {OPR_reg_Dd, OPR_reg_Dm}, 0x0eb00b40, 0x0fbf0fd0},
    {aarch32_op_VMOV_rs_A1, "vmov VMOV_rs_A1", {OPR_reg_Dd_x_, OPR_reg_Rt}, 0x0e000b10, 0x0f900f10},
    {aarch32_op_VMOV_tos_A1, "vmov VMOV_tos_A1", {OPR_reg_Sn, OPR_reg_Rt}, 0x0e000a10, 0x0ff00f10},
    {aarch32_op_VMOV_s_A1, "vmov VMOV_s_A1", {OPR_reg_Rt, OPR_reg_Sn}, 0x0e100a10, 0x0ff00f10},
    {aarch32_op_VMOV_sr_A1, "vmov VMOV_sr_A1", {OPR_reg_Rt, OPR_reg_Dn_x_}, 0x0e100b10, 0x0f100f10},
    {aarch32_op_VMOV_toss_A1, "vmov VMOV_toss_A1", {OPR_reg_Sm, OPR_reg_Sm1, OPR_reg_Rt, OPR_reg_Rt2}, 0x0c400a10, 0x0ff00fd0},
    {aarch32_op_VMOV_ss_A1, "vmov VMOV_ss_A1", {OPR_reg_Rt, OPR_reg_Rt2, OPR_reg_Sm, OPR_reg_Sm1}, 0x0c500a10, 0x0ff00fd0},
    {aarch32_op_VMOV_VORR_r_A1_D, "vmov VMOV_VORR_r_A1_D", {OPR_reg_Dd, OPR_reg_Dm}, 0xf2200110, 0xffb00f50},
    {aarch32_op_VMOV_VORR_r_A1_Q, "vmov VMOV_VORR_r_A1_Q", {OPR_reg_Qd, OPR_reg_Qm}, 0xf2200150, 0xffb00f50},
    {aarch32_op_VMOVL_A1, "vmovl VMOVL_A1", {OPR_reg_Qd, OPR_reg_Dm}, 0xf2800a10, 0xfe870fd0},
    {aarch32_op_VMOVN_A1, "vmovn VMOVN_A1", {OPR_reg_Dd, OPR_reg_Qm}, 0xf3b20200, 0xffb30fd0},
    {aarch32_op_VMOVX_A1, "vmovx VMOVX_A1", {OPR_reg_Sd, OPR_reg_Sm}, 0xfeb00a40, 0xffbf0fd0},
    {aarch32_op_VMRS_A1_AS, "vmrs VMRS_A1_AS", {OPR_reg_Rt}, 0x0ef00a10, 0x0ff00f10},
    {aarch32_op_VMSR_A1_AS, "vmsr VMSR_A1_AS", {OPR_reg_Rt}, 0x0ee00a10, 0x0ff00f10},
    {aarch32_op_VMUL_f_A1_D, "vmul VMUL_f_A1_D", {OPR_reg_Dd, OPR_reg_Dn, OPR_reg_Dm}, 0xf3000d10, 0xffa00f50},
    {aarch32_op_VMUL_f_A1_Q, "vmul VMUL_f_A1_Q", {OPR_reg_Qd, OPR_reg_Qn, OPR_reg_Qm}, 0xf3000d50, 0xffa00f50},
    {aarch32_op_VMUL_f_A2_H, "vmul VMUL_f_A2_H", {OPR_reg_Sd, OPR_reg_Sn, OPR_reg_Sm}, 0x0e200900, 0x0fb00f50},
    {aarch32_op_VMUL_f_A2_S, "vmul VMUL_f_A2_S", {OPR_reg_Sd, OPR_reg_Sn, OPR_reg_Sm}, 0x0e200a00, 0x0fb00f50},
    {aarch32_op_VMUL_f_A2_D, "vmul VMUL_f_A2_D", {OPR_reg_Dd, OPR_reg_Dn, OPR_reg_Dm}, 0x0e200b00, 0x0fb00f50},
    {aarch32_op_VMUL_i_A1_D, "vmul VMUL_i_A1_D", {OPR_reg_Dd, OPR_reg_Dn, OPR_reg_Dm}, 0xf2000910, 0xfe800f50},
    {aarch32_op_VMUL_i_A1_Q, "vmul VMUL_i_A1_Q", {OPR_reg_Qd, OPR_reg_Qn, OPR_reg_Qm}, 0xf2000950, 0xfe800f50},
    {aarch32_op_VMUL_s_A1_D, "vmul VMUL_s_A1_D", {OPR_reg_Dd, OPR_reg_Dn, OPR_reg_Dm}, 0xf2800840, 0xff800e50},
    {aarch32_op_VMUL_s_A1_Q, "vmul VMUL_s_A1_Q", {OPR_reg_Qd, OPR_reg_Qn, OPR_reg_Dm}, 0xf3800840, 0xff800e50},
    {aarch32_op_VMULL_i_A1, "vmull VMULL_i_A1", {OPR_reg_Qd, OPR_reg_Dn, OPR_reg_Dm}, 0xf2800c00, 0xfe800d50},
    {aarch32_op_VMULL_s_A1, "vmull VMULL_s_A1", {OPR_reg_Qd, OPR_reg_Dn, OPR_reg_Dm}, 0xf2800a40, 0xfe800f50},
    {aarch32_op_VMVN_i_A1_D, "vmvn VMVN_i_A1_D", {OPR_reg_Dd}, 0xf2800030, 0xfeb809f0},
    {aarch32_op_VMVN_i_A1_Q, "vmvn VMVN_i_A1_Q", {OPR_reg_Qd}, 0xf2800070, 0xfeb809f0},
    {aarch32_op_VMVN_i_A2_D, "vmvn VMVN_i_A2_D", {OPR_reg_Dd}, 0xf2800830, 0xfeb80df0},
    {aarch32_op_VMVN_i_A2_Q, "vmvn VMVN_i_A2_Q", {OPR_reg_Qd}, 0xf2800870, 0xfeb80df0},
    {aarch32_op_VMVN_i_A3_D, "vmvn VMVN_i_A3_D", {OPR_reg_Dd}, 0xf2800c30, 0xfeb80ef0},
    {aarch32_op_VMVN_i_A3_Q, "vmvn VMVN_i_A3_Q", {OPR_reg_Qd}, 0xf2800c70, 0xfeb80ef0},
    {aarch32_op_VMVN_r_A1_D, "vmvn VMVN_r_A1_D", {OPR_reg_Dd, OPR_reg_Dm}, 0xf3b00580, 0xffb30fd0},
    {aarch32_op_VMVN_r_A1_Q, "vmvn VMVN_r_A1_Q", {OPR_reg_Qd, OPR_reg_Qm}, 0xf3b005c0, 0xffb30fd0},
    {aarch32_op_VNEG_A1_D, "vneg VNEG_A1_D", {OPR_reg_Dd, OPR_reg_Dm}, 0xf3b10380, 0xffb30bd0},
    {aarch32_op_VNEG_A1_Q, "vneg VNEG_A1_Q", {OPR_reg_Qd, OPR_reg_Qm}, 0xf3b103c0, 0xffb30bd0},
    {aarch32_op_VNEG_A2_H, "vneg VNEG_A2_H", {OPR_reg_Sd, OPR_reg_Sm}, 0x0eb10940, 0x0fbf0fd0},
    {aarch32_op_VNEG_A2_S, "vneg VNEG_A2_S", {OPR_reg_Sd, OPR_reg_Sm}, 0x0eb10a40, 0x0fbf0fd0},
    {aarch32_op_VNEG_A2_D, "vneg VNEG_A2_D", {OPR_reg_Dd, OPR_reg_Dm}, 0x0eb10b40, 0x0fbf0fd0},
    {aarch32_op_VNMLA_A1_H, "vnmla VNMLA_A1_H", {OPR_reg_Sd, OPR_reg_Sn, OPR_reg_Sm}, 0x0e100940, 0x0fb00f50},
    {aarch32_op_VNMLA_A1_S, "vnmla VNMLA_A1_S", {OPR_reg_Sd, OPR_reg_Sn, OPR_reg_Sm}, 0x0e100a40, 0x0fb00f50},
    {aarch32_op_VNMLA_A1_D, "vnmla VNMLA_A1_D", {OPR_reg_Dd, OPR_reg_Dn, OPR_reg_Dm}, 0x0e100b40, 0x0fb00f50},
    {aarch32_op_VNMLS_A1_H, "vnmls VNMLS_A1_H", {OPR_reg_Sd, OPR_reg_Sn, OPR_reg_Sm}, 0x0e100900, 0x0fb00f50},
    {aarch32_op_VNMLS_A1_S, "vnmls VNMLS_A1_S", {OPR_reg_Sd, OPR_reg_Sn, OPR_reg_Sm}, 0x0e100a00, 0x0fb00f50},
    {aarch32_op_VNMLS_A1_D, "vnmls VNMLS_A1_D", {OPR_reg_Dd, OPR_reg_Dn, OPR_reg_Dm}, 0x0e100b00, 0x0fb00f50},
    {aarch32_op_VNMUL_A1_H, "vnmul VNMUL_A1_H", {OPR_reg_Sd, OPR_reg_Sn, OPR_reg_Sm}, 0x0e200940, 0x0fb00f50},
    {aarch32_op_VNMUL_A1_S, "vnmul VNMUL_A1_S", {OPR_reg_Sd, OPR_reg_Sn, OPR_reg_Sm}, 0x0e200a40, 0x0fb00f50},
    {aarch32_op_VNMUL_A1_D, "vnmul VNMUL_A1_D", {OPR_reg_Dd, OPR_reg_Dn, OPR_reg_Dm}, 0x0e200b40, 0x0fb00f50},
    {aarch32_op_VORN_r_A1_D, "vorn VORN_r_A1_D", {OPR_reg_Dd, OPR_reg_Dn, OPR_reg_Dm}, 0xf2300110, 0xffb00f50},
    {aarch32_op_VORN_r_A1_Q, "vorn VORN_r_A1_Q", {OPR_reg_Qd, OPR_reg_Qn, OPR_reg_Qm}, 0xf2300150, 0xffb00f50},
    {aarch32_op_VORN_VORR_i_A1_D, "vorn VORN_VORR_i_A1_D", {OPR_reg_Dd, OPR_reg_Dd}, 0xf2800110, 0xfeb809f0},
    {aarch32_op_VORN_VORR_i_A1_Q, "vorn VORN_VORR_i_A1_Q", {OPR_reg_Qd, OPR_reg_Qd}, 0xf2800150, 0xfeb809f0},
    {aarch32_op_VORN_VORR_i_A2_D, "vorn VORN_VORR_i_A2_D", {OPR_reg_Dd, OPR_reg_Dd}, 0xf2800910, 0xfeb80df0},
    {aarch32_op_VORN_VORR_i_A2_Q, "vorn VORN_VORR_i_A2_Q", {OPR_reg_Qd, OPR_reg_Qd}, 0xf2800950, 0xfeb80df0},
    {aarch32_op_VORR_i_A1_D, "vorr VORR_i_A1_D", {OPR_reg_Dd, OPR_reg_Dd}, 0xf2800110, 0xfeb809f0},
    {aarch32_op_VORR_i_A1_Q, "vorr VORR_i_A1_Q", {OPR_reg_Qd, OPR_reg_Qd}, 0xf2800150, 0xfeb809f0},
    {aarch32_op_VORR_i_A2_D, "vorr VORR_i_A2_D", {OPR_reg_Dd, OPR_reg_Dd}, 0xf2800910, 0xfeb80df0},
    {aarch32_op_VORR_i_A2_Q, "vorr VORR_i_A2_Q", {OPR_reg_Qd, OPR_reg_Qd}, 0xf2800950, 0xfeb80df0},
    {aarch32_op_VORR_r_A1_D, "vorr VORR_r_A1_D", {OPR_reg_Dd, OPR_reg_Dn, OPR_reg_Dm}, 0xf2200110, 0xffb00f50},
    {aarch32_op_VORR_r_A1_Q, "vorr VORR_r_A1_Q", {OPR_reg_Qd, OPR_reg_Qn, OPR_reg_Qm}, 0xf2200150, 0xffb00f50},
    {aarch32_op_VPADAL_A1_D, "vpadal VPADAL_A1_D", {OPR_reg_Dd, OPR_reg_Dm}, 0xf3b00600, 0xffb30f50},
    {aarch32_op_VPADAL_A1_Q, "vpadal VPADAL_A1_Q", {OPR_reg_Qd, OPR_reg_Qm}, 0xf3b00640, 0xffb30f50},
    {aarch32_op_VPADD_f_A1, "vpadd VPADD_f_A1", {OPR_reg_Dd, OPR_reg_Dn, OPR_reg_Dm}, 0xf3000d00, 0xffa00f10},
    {aarch32_op_VPADD_i_A1, "vpadd VPADD_i_A1", {OPR_reg_Dd, OPR_reg_Dn, OPR_reg_Dm}, 0xf2000b10, 0xff800f10},
    {aarch32_op_VPADDL_A1_D, "vpaddl VPADDL_A1_D", {OPR_reg_Dd, OPR_reg_Dm}, 0xf3b00200, 0xffb30f50},
    {aarch32_op_VPADDL_A1_Q, "vpaddl VPADDL_A1_Q", {OPR_reg_Qd, OPR_reg_Qm}, 0xf3b00240, 0xffb30f50},
    {aarch32_op_VPMAX_f_A1, "vpmax VPMAX_f_A1", {OPR_reg_Dd, OPR_reg_Dn, OPR_reg_Dm}, 0xf3000f00, 0xffa00f50},
    {aarch32_op_VPMAX_i_A1, "vpmax VPMAX_i_A1", {OPR_reg_Dd, OPR_reg_Dn, OPR_reg_Dm}, 0xf2000a00, 0xfe800f50},
    {aarch32_op_VPMIN_f_A1, "vpmin VPMIN_f_A1", {OPR_reg_Dd, OPR_reg_Dn, OPR_reg_Dm}, 0xf3200f00, 0xffa00f50},
    {aarch32_op_VPMIN_i_A1, "vpmin VPMIN_i_A1", {OPR_reg_Dd, OPR_reg_Dn, OPR_reg_Dm}, 0xf2000a10, 0xfe800f50},
    {aarch32_op_VPOP_VLDM_A1, "vpop VPOP_VLDM_A1", {}, 0x0cbd0b00, 0x0fbf0f01},
    {aarch32_op_VPOP_VLDM_A2, "vpop VPOP_VLDM_A2", {}, 0x0cbd0a00, 0x0fbf0f00},
    {aarch32_op_VPUSH_VSTMDB_A1, "vpush VPUSH_VSTMDB_A1", {}, 0x0d2d0b00, 0x0fbf0f01},
    {aarch32_op_VPUSH_VSTMDB_A2, "vpush VPUSH_VSTMDB_A2", {}, 0x0d2d0a00, 0x0fbf0f00},
    {aarch32_op_VQABS_A1_D, "vqabs VQABS_A1_D", {OPR_reg_Dd, OPR_reg_Dm}, 0xf3b00700, 0xffb30fd0},
    {aarch32_op_VQABS_A1_Q, "vqabs VQABS_A1_Q", {OPR_reg_Qd, OPR_reg_Qm}, 0xf3b00740, 0xffb30fd0},
    {aarch32_op_VQADD_A1_D, "vqadd VQADD_A1_D", {OPR_reg_Dd, OPR_reg_Dn, OPR_reg_Dm}, 0xf2000010, 0xfe800f50},
    {aarch32_op_VQADD_A1_Q, "vqadd VQADD_A1_Q", {OPR_reg_Qd, OPR_reg_Qn, OPR_reg_Qm}, 0xf2000050, 0xfe800f50},
    {aarch32_op_VQDMLAL_A1, "vqdmlal VQDMLAL_A1", {OPR_reg_Qd, OPR_reg_Dn, OPR_reg_Dm}, 0xf2800900, 0xff800f50},
    {aarch32_op_VQDMLAL_A2, "vqdmlal VQDMLAL_A2", {OPR_reg_Qd, OPR_reg_Dn, OPR_reg_Dm}, 0xf2800340, 0xff800f50},
    {aarch32_op_VQDMLSL_A1, "vqdmlsl VQDMLSL_A1", {OPR_reg_Qd, OPR_reg_Dn, OPR_reg_Dm}, 0xf2800b00, 0xff800f50},
    {aarch32_op_VQDMLSL_A2, "vqdmlsl VQDMLSL_A2", {OPR_reg_Qd, OPR_reg_Dn, OPR_reg_Dm}, 0xf2800740, 0xff800f50},
    {aarch32_op_VQDMULH_A1_D, "vqdmulh VQDMULH_A1_D", {OPR_reg_Dd, OPR_reg_Dn, OPR_reg_Dm}, 0xf2000b00, 0xff800f50},
    {aarch32_op_VQDMULH_A1_Q, "vqdmulh VQDMULH_A1_Q", {OPR_reg_Qd, OPR_reg_Qn, OPR_reg_Qm}, 0xf2000b40, 0xff800f50},
    {aarch32_op_VQDMULH_A2_D, "vqdmulh VQDMULH_A2_D", {OPR_reg_Dd, OPR_reg_Dn, OPR_reg_Dm_x_}, 0xf2800c40, 0xff800f50},
    {aarch32_op_VQDMULH_A2_Q, "vqdmulh VQDMULH_A2_Q", {OPR_reg_Qd, OPR_reg_Qn, OPR_reg_Dm_x_}, 0xf3800c40, 0xff800f50},
    {aarch32_op_VQDMULL_A1, "vqdmull VQDMULL_A1", {OPR_reg_Qd, OPR_reg_Dn, OPR_reg_Dm}, 0xf2800d00, 0xff800f50},
    {aarch32_op_VQDMULL_A2, "vqdmull VQDMULL_A2", {OPR_reg_Qd, OPR_reg_Dn, OPR_reg_Dm_x_}, 0xf2800b40, 0xff800f50},
    {aarch32_op_VQMOVN_A1, "vqmovn VQMOVN_A1", {OPR_reg_Dd, OPR_reg_Qm}, 0xf3b20280, 0xffb30f90},
    {aarch32_op_VQMOVUN_A1, "vqmovun VQMOVUN_A1", {OPR_reg_Dd, OPR_reg_Qm}, 0xf3b20240, 0xffb30fd0},
    {aarch32_op_VQNEG_A1_D, "vqneg VQNEG_A1_D", {OPR_reg_Dd, OPR_reg_Dm}, 0xf3b00780, 0xffb30fd0},
    {aarch32_op_VQNEG_A1_Q, "vqneg VQNEG_A1_Q", {OPR_reg_Qd, OPR_reg_Qm}, 0xf3b007c0, 0xffb30fd0},
    {aarch32_op_VQRDMLAH_A1_D, "vqrdmlah VQRDMLAH_A1_D", {OPR_reg_Dd, OPR_reg_Dn, OPR_reg_Dm}, 0xf3000b10, 0xff800f50},
    {aarch32_op_VQRDMLAH_A1_Q, "vqrdmlah VQRDMLAH_A1_Q", {OPR_reg_Qd, OPR_reg_Qn, OPR_reg_Qm}, 0xf3000b50, 0xff800f50},
    {aarch32_op_VQRDMLAH_A2_D, "vqrdmlah VQRDMLAH_A2_D", {OPR_reg_Dd, OPR_reg_Dn, OPR_reg_Dm_x_}, 0xf2800e40, 0xff800f50},
    {aarch32_op_VQRDMLAH_A2_Q, "vqrdmlah VQRDMLAH_A2_Q", {OPR_reg_Qd, OPR_reg_Qn, OPR_reg_Dm_x_}, 0xf3800e40, 0xff800f50},
    {aarch32_op_VQRDMLSH_A1_D, "vqrdmlsh VQRDMLSH_A1_D", {OPR_reg_Dd, OPR_reg_Dn, OPR_reg_Dm}, 0xf3000c10, 0xff800f50},
    {aarch32_op_VQRDMLSH_A1_Q, "vqrdmlsh VQRDMLSH_A1_Q", {OPR_reg_Qd, OPR_reg_Qn, OPR_reg_Qm}, 0xf3000c50, 0xff800f50},
    {aarch32_op_VQRDMLSH_A2_D, "vqrdmlsh VQRDMLSH_A2_D", {OPR_reg_Dd, OPR_reg_Dn, OPR_reg_Dm_x_}, 0xf2800f40, 0xff800f50},
    {aarch32_op_VQRDMLSH_A2_Q, "vqrdmlsh VQRDMLSH_A2_Q", {OPR_reg_Qd, OPR_reg_Qn, OPR_reg_Dm_x_}, 0xf3800f40, 0xff800f50},
    {aarch32_op_VQRDMULH_A1_D, "vqrdmulh VQRDMULH_A1_D", {OPR_reg_Dd, OPR_reg_Dn, OPR_reg_Dm}, 0xf3000b00, 0xff800f50},
    {aarch32_op_VQRDMULH_A1_Q, "vqrdmulh VQRDMULH_A1_Q", {OPR_reg_Qd, OPR_reg_Qn, OPR_reg_Qm}, 0xf3000b40, 0xff800f50},
    {aarch32_op_VQRDMULH_A2_D, "vqrdmulh VQRDMULH_A2_D", {OPR_reg_Dd, OPR_reg_Dn, OPR_reg_Dm_x_}, 0xf2800d40, 0xff800f50},
    {aarch32_op_VQRDMULH_A2_Q, "vqrdmulh VQRDMULH_A2_Q", {OPR_reg_Qd, OPR_reg_Qn, OPR_reg_Dm_x_}, 0xf3800d40, 0xff800f50},
    {aarch32_op_VQRSHL_A1_D, "vqrshl VQRSHL_A1_D", {OPR_reg_Dd, OPR_reg_Dm, OPR_reg_Dn}, 0xf2000510, 0xfe800f50},
    {aarch32_op_VQRSHL_A1_Q, "vqrshl VQRSHL_A1_Q", {OPR_reg_Qd, OPR_reg_Qm, OPR_reg_Qn}, 0xf2000550, 0xfe800f50},
    {aarch32_op_VQRSHRN_A1, "vqrshrn VQRSHRN_A1", {OPR_reg_Dd, OPR_reg_Qm}, 0xf2800950, 0xfe800fd0},
    {aarch32_op_VQRSHRUN_A1, "vqrshrun VQRSHRUN_A1", {OPR_reg_Dd, OPR_reg_Qm}, 0xf3800850, 0xff800fd0},
    {aarch32_op_VQRSHRN_VQMOVN_A1, "vqrshrn VQRSHRN_VQMOVN_A1", {OPR_reg_Dd, OPR_reg_Qm}, 0xf3b20280, 0xffb30f90},
    {aarch32_op_VQRSHRUN_VQMOVUN_A1, "vqrshrun VQRSHRUN_VQMOVUN_A1", {OPR_reg_Dd, OPR_reg_Qm}, 0xf3b20240, 0xffb30fd0},
    {aarch32_op_VQSHL_i_A1_D, "vqshl VQSHL_i_A1_D", {OPR_reg_Dd, OPR_reg_Dm}, 0xf2800710, 0xfe800f50},
    {aarch32_op_VQSHL_i_A1_Q, "vqshl VQSHL_i_A1_Q", {OPR_reg_Qd, OPR_reg_Qm}, 0xf2800750, 0xfe800f50},
    {aarch32_op_VQSHLU_i_A1_D, "vqshlu VQSHLU_i_A1_D", {OPR_reg_Dd, OPR_reg_Dm}, 0xf3800610, 0xff800f50},
    {aarch32_op_VQSHLU_i_A1_Q, "vqshlu VQSHLU_i_A1_Q", {OPR_reg_Qd, OPR_reg_Qm}, 0xf3800650, 0xff800f50},
    {aarch32_op_VQSHL_r_A1_D, "vqshl VQSHL_r_A1_D", {OPR_reg_Dd, OPR_reg_Dm, OPR_reg_Dn}, 0xf2000410, 0xfe800f50},
    {aarch32_op_VQSHL_r_A1_Q, "vqshl VQSHL_r_A1_Q", {OPR_reg_Qd, OPR_reg_Qm, OPR_reg_Qn}, 0xf2000450, 0xfe800f50},
    {aarch32_op_VQSHRN_A1, "vqshrn VQSHRN_A1", {OPR_reg_Dd, OPR_reg_Qm}, 0xf2800910, 0xfe800fd0},
    {aarch32_op_VQSHRUN_A1, "vqshrun VQSHRUN_A1", {OPR_reg_Dd, OPR_reg_Qm}, 0xf3800810, 0xff800fd0},
    {aarch32_op_VQSHRN_VQMOVN_A1, "vqshrn VQSHRN_VQMOVN_A1", {OPR_reg_Dd, OPR_reg_Qm}, 0xf3b20280, 0xffb30f90},
    {aarch32_op_VQSHRUN_VQMOVUN_A1, "vqshrun VQSHRUN_VQMOVUN_A1", {OPR_reg_Dd, OPR_reg_Qm}, 0xf3b20240, 0xffb30fd0},
    {aarch32_op_VQSUB_A1_D, "vqsub VQSUB_A1_D", {OPR_reg_Dd, OPR_reg_Dn, OPR_reg_Dm}, 0xf2000210, 0xfe800f50},
    {aarch32_op_VQSUB_A1_Q, "vqsub VQSUB_A1_Q", {OPR_reg_Qd, OPR_reg_Qn, OPR_reg_Qm}, 0xf2000250, 0xfe800f50},
    {aarch32_op_VRADDHN_A1, "vraddhn VRADDHN_A1", {OPR_reg_Dd, OPR_reg_Qn, OPR_reg_Qm}, 0xf3800400, 0xff800f50},
    {aarch32_op_VRECPE_A1_D, "vrecpe VRECPE_A1_D", {OPR_reg_Dd, OPR_reg_Dm}, 0xf3b30400, 0xffb30ed0},
    {aarch32_op_VRECPE_A1_Q, "vrecpe VRECPE_A1_Q", {OPR_reg_Qd, OPR_reg_Qm}, 0xf3b30440, 0xffb30ed0},
    {aarch32_op_VRECPS_A1_D, "vrecps VRECPS_A1_D", {OPR_reg_Dd, OPR_reg_Dn, OPR_reg_Dm}, 0xf2000f10, 0xffa00f50},
    {aarch32_op_VRECPS_A1_Q, "vrecps VRECPS_A1_Q", {OPR_reg_Qd, OPR_reg_Qn, OPR_reg_Qm}, 0xf2000f50, 0xffa00f50},
    {aarch32_op_VREV16_A1_D, "vrev16 VREV16_A1_D", {OPR_reg_Dd, OPR_reg_Dm}, 0xf3b00100, 0xffb30fd0},
    {aarch32_op_VREV16_A1_Q, "vrev16 VREV16_A1_Q", {OPR_reg_Qd, OPR_reg_Qm}, 0xf3b00140, 0xffb30fd0},
    {aarch32_op_VREV32_A1_D, "vrev32 VREV32_A1_D", {OPR_reg_Dd, OPR_reg_Dm}, 0xf3b00080, 0xffb30fd0},
    {aarch32_op_VREV32_A1_Q, "vrev32 VREV32_A1_Q", {OPR_reg_Qd, OPR_reg_Qm}, 0xf3b000c0, 0xffb30fd0},
    {aarch32_op_VREV64_A1_D, "vrev64 VREV64_A1_D", {OPR_reg_Dd, OPR_reg_Dm}, 0xf3b00000, 0xffb30fd0},
    {aarch32_op_VREV64_A1_Q, "vrev64 VREV64_A1_Q", {OPR_reg_Qd, OPR_reg_Qm}, 0xf3b00040, 0xffb30fd0},
    {aarch32_op_VRHADD_A1_D, "vrhadd VRHADD_A1_D", {OPR_reg_Dd, OPR_reg_Dn, OPR_reg_Dm}, 0xf2000100, 0xfe800f50},
    {aarch32_op_VRHADD_A1_Q, "vrhadd VRHADD_A1_Q", {OPR_reg_Qd, OPR_reg_Qn, OPR_reg_Qm}, 0xf2000140, 0xfe800f50},
    {aarch32_op_VRINTA_asimd_A1_D, "vrinta VRINTA_asimd_A1_D", {OPR_reg_Dd, OPR_reg_Dm}, 0xf3b20500, 0xffb30fd0},
    {aarch32_op_VRINTA_asimd_A1_Q, "vrinta VRINTA_asimd_A1_Q", {OPR_reg_Qd, OPR_reg_Qm}, 0xf3b20540, 0xffb30fd0},
    {aarch32_op_VRINTA_vfp_A1_H, "vrinta VRINTA_vfp_A1_H", {OPR_reg_Sd, OPR_reg_Sm}, 0xfeb80940, 0xffbf0fd0},
    {aarch32_op_VRINTA_vfp_A1_S, "vrinta VRINTA_vfp_A1_S", {OPR_reg_Sd, OPR_reg_Sm}, 0xfeb80a40, 0xffbf0fd0},
    {aarch32_op_VRINTA_vfp_A1_D, "vrinta VRINTA_vfp_A1_D", {OPR_reg_Dd, OPR_reg_Dm}, 0xfeb80b40, 0xffbf0fd0},
    {aarch32_op_VRINTM_asimd_A1_D, "vrintm VRINTM_asimd_A1_D", {OPR_reg_Dd, OPR_reg_Dm}, 0xf3b20680, 0xffb30fd0},
    {aarch32_op_VRINTM_asimd_A1_Q, "vrintm VRINTM_asimd_A1_Q", {OPR_reg_Qd, OPR_reg_Qm}, 0xf3b206c0, 0xffb30fd0},
    {aarch32_op_VRINTM_vfp_A1_H, "vrintm VRINTM_vfp_A1_H", {OPR_reg_Sd, OPR_reg_Sm}, 0xfebb0940, 0xffbf0fd0},
    {aarch32_op_VRINTM_vfp_A1_S, "vrintm VRINTM_vfp_A1_S", {OPR_reg_Sd, OPR_reg_Sm}, 0xfebb0a40, 0xffbf0fd0},
    {aarch32_op_VRINTM_vfp_A1_D, "vrintm VRINTM_vfp_A1_D", {OPR_reg_Dd, OPR_reg_Dm}, 0xfebb0b40, 0xffbf0fd0},
    {aarch32_op_VRINTN_asimd_A1_D, "vrintn VRINTN_asimd_A1_D", {OPR_reg_Dd, OPR_reg_Dm}, 0xf3b20400, 0xffb30fd0},
    {aarch32_op_VRINTN_asimd_A1_Q, "vrintn VRINTN_asimd_A1_Q", {OPR_reg_Qd, OPR_reg_Qm}, 0xf3b20440, 0xffb30fd0},
    {aarch32_op_VRINTN_vfp_A1_H, "vrintn VRINTN_vfp_A1_H", {OPR_reg_Sd, OPR_reg_Sm}, 0xfeb90940, 0xffbf0fd0},
    {aarch32_op_VRINTN_vfp_A1_S, "vrintn VRINTN_vfp_A1_S", {OPR_reg_Sd, OPR_reg_Sm}, 0xfeb90a40, 0xffbf0fd0},
    {aarch32_op_VRINTN_vfp_A1_D, "vrintn VRINTN_vfp_A1_D", {OPR_reg_Dd, OPR_reg_Dm}, 0xfeb90b40, 0xffbf0fd0},
    {aarch32_op_VRINTP_asimd_A1_D, "vrintp VRINTP_asimd_A1_D", {OPR_reg_Dd, OPR_reg_Dm}, 0xf3b20780, 0xffb30fd0},
    {aarch32_op_VRINTP_asimd_A1_Q, "vrintp VRINTP_asimd_A1_Q", {OPR_reg_Qd, OPR_reg_Qm}, 0xf3b207c0, 0xffb30fd0},
    {aarch32_op_VRINTP_vfp_A1_H, "vrintp VRINTP_vfp_A1_H", {OPR_reg_Sd, OPR_reg_Sm}, 0xfeba0940, 0xffbf0fd0},
    {aarch32_op_VRINTP_vfp_A1_S, "vrintp VRINTP_vfp_A1_S", {OPR_reg_Sd, OPR_reg_Sm}, 0xfeba0a40, 0xffbf0fd0},
    {aarch32_op_VRINTP_vfp_A1_D, "vrintp VRINTP_vfp_A1_D", {OPR_reg_Dd, OPR_reg_Dm}, 0xfeba0b40, 0xffbf0fd0},
    {aarch32_op_VRINTR_vfp_A1_H, "vrintr VRINTR_vfp_A1_H", {OPR_reg_Sd, OPR_reg_Sm}, 0x0eb60940, 0x0fbf0fd0},
    {aarch32_op_VRINTR_vfp_A1_S, "vrintr VRINTR_vfp_A1_S", {OPR_reg_Sd, OPR_reg_Sm}, 0x0eb60a40, 0x0fbf0fd0},
    {aarch32_op_VRINTR_vfp_A1_D, "vrintr VRINTR_vfp_A1_D", {OPR_reg_Dd, OPR_reg_Dm}, 0x0eb60b40, 0x0fbf0fd0},
    {aarch32_op_VRINTX_asimd_A1_D, "vrintx VRINTX_asimd_A1_D", {OPR_reg_Dd, OPR_reg_Dm}, 0xf3b20480, 0xffb30fd0},
    {aarch32_op_VRINTX_asimd_A1_Q, "vrintx VRINTX_asimd_A1_Q", {OPR_reg_Qd, OPR_reg_Qm}, 0xf3b204c0, 0xffb30fd0},
    {aarch32_op_VRINTX_vfp_A1_H, "vrintx VRINTX_vfp_A1_H", {OPR_reg_Sd, OPR_reg_Sm}, 0x0eb70940, 0x0fbf0fd0},
    {aarch32_op_VRINTX_vfp_A1_S, "vrintx VRINTX_vfp_A1_S", {OPR_reg_Sd, OPR_reg_Sm}, 0x0eb70a40, 0x0fbf0fd0},
    {aarch32_op_VRINTX_vfp_A1_D, "vrintx VRINTX_vfp_A1_D", {OPR_reg_Dd, OPR_reg_Dm}, 0x0eb70b40, 0x0fbf0fd0},
    {aarch32_op_VRINTZ_asimd_A1_D, "vrintz VRINTZ_asimd_A1_D", {OPR_reg_Dd, OPR_reg_Dm}, 0xf3b20580, 0xffb30fd0},
    {aarch32_op_VRINTZ_asimd_A1_Q, "vrintz VRINTZ_asimd_A1_Q", {OPR_reg_Qd, OPR_reg_Qm}, 0xf3b205c0, 0xffb30fd0},
    {aarch32_op_VRINTZ_vfp_A1_H, "vrintz VRINTZ_vfp_A1_H", {OPR_reg_Sd, OPR_reg_Sm}, 0x0eb609c0, 0x0fbf0fd0},
    {aarch32_op_VRINTZ_vfp_A1_S, "vrintz VRINTZ_vfp_A1_S", {OPR_reg_Sd, OPR_reg_Sm}, 0x0eb60ac0, 0x0fbf0fd0},
    {aarch32_op_VRINTZ_vfp_A1_D, "vrintz VRINTZ_vfp_A1_D", {OPR_reg_Dd, OPR_reg_Dm}, 0x0eb60bc0, 0x0fbf0fd0},
    {aarch32_op_VRSHL_A1_D, "vrshl VRSHL_A1_D", {OPR_reg_Dd, OPR_reg_Dm, OPR_reg_Dn}, 0xf2000500, 0xfe800f50},
    {aarch32_op_VRSHL_A1_Q, "vrshl VRSHL_A1_Q", {OPR_reg_Qd, OPR_reg_Qm, OPR_reg_Qn}, 0xf2000540, 0xfe800f50},
    {aarch32_op_VRSHR_A1_D, "vrshr VRSHR_A1_D", {OPR_reg_Dd, OPR_reg_Dm}, 0xf2800210, 0xfe800f50},
    {aarch32_op_VRSHR_A1_Q, "vrshr VRSHR_A1_Q", {OPR_reg_Qd, OPR_reg_Qm}, 0xf2800250, 0xfe800f50},
    {aarch32_op_VRSHR_VORR_r_A1_D, "vrshr VRSHR_VORR_r_A1_D", {OPR_reg_Dd, OPR_reg_Dm}, 0xf2200110, 0xffb00f50},
    {aarch32_op_VRSHR_VORR_r_A1_Q, "vrshr VRSHR_VORR_r_A1_Q", {OPR_reg_Qd, OPR_reg_Qm}, 0xf2200150, 0xffb00f50},
    {aarch32_op_VRSHRN_A1, "vrshrn VRSHRN_A1", {OPR_reg_Dd, OPR_reg_Qm}, 0xf2800850, 0xff800fd0},
    {aarch32_op_VRSHRN_VMOVN_A1, "vrshrn VRSHRN_VMOVN_A1", {OPR_reg_Dd, OPR_reg_Qm}, 0xf3b20200, 0xffb30fd0},
    {aarch32_op_VRSQRTE_A1_D, "vrsqrte VRSQRTE_A1_D", {OPR_reg_Dd, OPR_reg_Dm}, 0xf3b30480, 0xffb30ed0},
    {aarch32_op_VRSQRTE_A1_Q, "vrsqrte VRSQRTE_A1_Q", {OPR_reg_Qd, OPR_reg_Qm}, 0xf3b304c0, 0xffb30ed0},
    {aarch32_op_VRSQRTS_A1_D, "vrsqrts VRSQRTS_A1_D", {OPR_reg_Dd, OPR_reg_Dn, OPR_reg_Dm}, 0xf2200f10, 0xffa00f50},
    {aarch32_op_VRSQRTS_A1_Q, "vrsqrts VRSQRTS_A1_Q", {OPR_reg_Qd, OPR_reg_Qn, OPR_reg_Qm}, 0xf2200f50, 0xffa00f50},
    {aarch32_op_VRSRA_A1_Q, "vrsra VRSRA_A1_Q", {OPR_reg_Dd, OPR_reg_Dm}, 0xf2800310, 0xfe800f50},
    {aarch32_op_VRSRA_A1_D, "vrsra VRSRA_A1_D", {OPR_reg_Qd, OPR_reg_Qm}, 0xf2800350, 0xfe800f50},
    {aarch32_op_VRSUBHN_A1, "vrsubhn VRSUBHN_A1", {OPR_reg_Dd, OPR_reg_Qn, OPR_reg_Qm}, 0xf3800600, 0xff800f50},
    {aarch32_op_VSELEQ_A1_D, "vseleq VSELEQ_A1_D", {OPR_reg_Dd, OPR_reg_Dn, OPR_reg_Dm}, 0xfe000b00, 0xffb00f50},
    {aarch32_op_VSELEQ_A1_H, "vseleq VSELEQ_A1_H", {OPR_reg_Sd, OPR_reg_Sn, OPR_reg_Sm}, 0xfe000900, 0xffb00f50},
    {aarch32_op_VSELEQ_A1_S, "vseleq VSELEQ_A1_S", {OPR_reg_Sd, OPR_reg_Sn, OPR_reg_Sm}, 0xfe000a00, 0xffb00f50},
    {aarch32_op_VSELGE_A1_D, "vselge VSELGE_A1_D", {OPR_reg_Dd, OPR_reg_Dn, OPR_reg_Dm}, 0xfe200b00, 0xffb00f50},
    {aarch32_op_VSELGE_A1_H, "vselge VSELGE_A1_H", {OPR_reg_Sd, OPR_reg_Sn, OPR_reg_Sm}, 0xfe200900, 0xffb00f50},
    {aarch32_op_VSELGE_A1_S, "vselge VSELGE_A1_S", {OPR_reg_Sd, OPR_reg_Sn, OPR_reg_Sm}, 0xfe200a00, 0xffb00f50},
    {aarch32_op_VSELGT_A1_D, "vselgt VSELGT_A1_D", {OPR_reg_Dd, OPR_reg_Dn, OPR_reg_Dm}, 0xfe300b00, 0xffb00f50},
    {aarch32_op_VSELGT_A1_H, "vselgt VSELGT_A1_H", {OPR_reg_Sd, OPR_reg_Sn, OPR_reg_Sm}, 0xfe300900, 0xffb00f50},
    {aarch32_op_VSELGT_A1_S, "vselgt VSELGT_A1_S", {OPR_reg_Sd, OPR_reg_Sn, OPR_reg_Sm}, 0xfe300a00, 0xffb00f50},
    {aarch32_op_VSELVS_A1_D, "vselvs VSELVS_A1_D", {OPR_reg_Dd, OPR_reg_Dn, OPR_reg_Dm}, 0xfe100b00, 0xffb00f50},
    {aarch32_op_VSELVS_A1_H, "vselvs VSELVS_A1_H", {OPR_reg_Sd, OPR_reg_Sn, OPR_reg_Sm}, 0xfe100900, 0xffb00f50},
    {aarch32_op_VSELVS_A1_S, "vselvs VSELVS_A1_S", {OPR_reg_Sd, OPR_reg_Sn, OPR_reg_Sm}, 0xfe100a00, 0xffb00f50},
    {aarch32_op_VSHL_i_A1_D, "vshl VSHL_i_A1_D", {OPR_reg_Dd, OPR_reg_Dm}, 0xf2800510, 0xff800f50},
    {aarch32_op_VSHL_i_A1_Q, "vshl VSHL_i_A1_Q", {OPR_reg_Qd, OPR_reg_Qm}, 0xf2800550, 0xff800f50},
    {aarch32_op_VSHL_r_A1_D, "vshl VSHL_r_A1_D", {OPR_reg_Dd, OPR_reg_Dm, OPR_reg_Dn}, 0xf2000400, 0xfe800f50},
    {aarch32_op_VSHL_r_A1_Q, "vshl VSHL_r_A1_Q", {OPR_reg_Qd, OPR_reg_Qm, OPR_reg_Qn}, 0xf2000440, 0xfe800f50},
    {aarch32_op_VSHLL_A1, "vshll VSHLL_A1", {OPR_reg_Qd, OPR_reg_Dm}, 0xf2800a10, 0xfe800fd0},
    {aarch32_op_VSHLL_A2, "vshll VSHLL_A2", {OPR_reg_Qd, OPR_reg_Dm}, 0xf3b20300, 0xffb30fd0},
    {aarch32_op_VSHR_A1_D, "vshr VSHR_A1_D", {OPR_reg_Dd, OPR_reg_Dm}, 0xf2800010, 0xfe800f50},
    {aarch32_op_VSHR_A1_Q, "vshr VSHR_A1_Q", {OPR_reg_Qd, OPR_reg_Qm}, 0xf2800050, 0xfe800f50},
    {aarch32_op_VSHR_VORR_r_A1_D, "vshr VSHR_VORR_r_A1_D", {OPR_reg_Dd, OPR_reg_Dm}, 0xf2200110, 0xffb00f50},
    {aarch32_op_VSHR_VORR_r_A1_Q, "vshr VSHR_VORR_r_A1_Q", {OPR_reg_Qd, OPR_reg_Qm}, 0xf2200150, 0xffb00f50},
    {aarch32_op_VSHRN_A1, "vshrn VSHRN_A1", {OPR_reg_Dd, OPR_reg_Qm}, 0xf2800810, 0xff800fd0},
    {aarch32_op_VSHRN_VMOVN_A1, "vshrn VSHRN_VMOVN_A1", {OPR_reg_Dd, OPR_reg_Qm}, 0xf3b20200, 0xffb30fd0},
    {aarch32_op_VSLI_A1_D, "vsli VSLI_A1_D", {OPR_reg_Dd, OPR_reg_Dm}, 0xf3800510, 0xff800f50},
    {aarch32_op_VSLI_A1_Q, "vsli VSLI_A1_Q", {OPR_reg_Qd, OPR_reg_Qm}, 0xf3800550, 0xff800f50},
    {aarch32_op_VSQRT_A1_H, "vsqrt VSQRT_A1_H", {OPR_reg_Sd, OPR_reg_Sm}, 0x0eb109c0, 0x0fbf0fd0},
    {aarch32_op_VSQRT_A1_S, "vsqrt VSQRT_A1_S", {OPR_reg_Sd, OPR_reg_Sm}, 0x0eb10ac0, 0x0fbf0fd0},
    {aarch32_op_VSQRT_A1_D, "vsqrt VSQRT_A1_D", {OPR_reg_Dd, OPR_reg_Dm}, 0x0eb10bc0, 0x0fbf0fd0},
    {aarch32_op_VSRA_A1_D, "vsra VSRA_A1_D", {OPR_reg_Dd, OPR_reg_Dm}, 0xf2800110, 0xfe800f50},
    {aarch32_op_VSRA_A1_Q, "vsra VSRA_A1_Q", {OPR_reg_Qd, OPR_reg_Qm}, 0xf2800150, 0xfe800f50},
    {aarch32_op_VSRI_A1_D, "vsri VSRI_A1_D", {OPR_reg_Dd, OPR_reg_Dm}, 0xf3800410, 0xff800f50},
    {aarch32_op_VSRI_A1_Q, "vsri VSRI_A1_Q", {OPR_reg_Qd, OPR_reg_Qm}, 0xf3800450, 0xff800f50},
    {aarch32_op_VST1_1_A1_nowb, "vst1 VST1_1_A1_nowb", {OPR_reg_Rn}, 0xf480000f, 0xffb00f0f},
    {aarch32_op_VST1_1_A1_posti, "vst1 VST1_1_A1_posti", {OPR_reg_Rn}, 0xf480000d, 0xffb00f0f},
    {aarch32_op_VST1_1_A1_postr, "vst1 VST1_1_A1_postr", {OPR_reg_Rn, OPR_reg_Rm}, 0xf4800000, 0xffb00f00},
    {aarch32_op_VST1_1_A2_nowb, "vst1 VST1_1_A2_nowb", {OPR_reg_Rn}, 0xf480040f, 0xffb00f0f},
    {aarch32_op_VST1_1_A2_posti, "vst1 VST1_1_A2_posti", {OPR_reg_Rn}, 0xf480040d, 0xffb00f0f},
    {aarch32_op_VST1_1_A2_postr, "vst1 VST1_1_A2_postr", {OPR_reg_Rn, OPR_reg_Rm}, 0xf4800400, 0xffb00f00},
    {aarch32_op_VST1_1_A3_nowb, "vst1 VST1_1_A3_nowb", {OPR_reg_Rn}, 0xf480080f, 0xffb00f0f},
    {aarch32_op_VST1_1_A3_posti, "vst1 VST1_1_A3_posti", {OPR_reg_Rn}, 0xf480080d, 0xffb00f0f},
    {aarch32_op_VST1_1_A3_postr, "vst1 VST1_1_A3_postr", {OPR_reg_Rn, OPR_reg_Rm}, 0xf4800800, 0xffb00f00},
    {aarch32_op_VST1_m_A1_nowb, "vst1 VST1_m_A1_nowb", {OPR_reg_Rn}, 0xf400070f, 0xffb00f0f},
    {aarch32_op_VST1_m_A1_posti, "vst1 VST1_m_A1_posti", {OPR_reg_Rn}, 0xf400070d, 0xffb00f0f},
    {aarch32_op_VST1_m_A1_postr, "vst1 VST1_m_A1_postr", {OPR_reg_Rn, OPR_reg_Rm}, 0xf4000700, 0xffb00f00},
    {aarch32_op_VST1_m_A2_nowb, "vst1 VST1_m_A2_nowb", {OPR_reg_Rn}, 0xf4000a0f, 0xffb00f0f},
    {aarch32_op_VST1_m_A2_posti, "vst1 VST1_m_A2_posti", {OPR_reg_Rn}, 0xf4000a0d, 0xffb00f0f},
    {aarch32_op_VST1_m_A2_postr, "vst1 VST1_m_A2_postr", {OPR_reg_Rn, OPR_reg_Rm}, 0xf4000a00, 0xffb00f00},
    {aarch32_op_VST1_m_A3_nowb, "vst1 VST1_m_A3_nowb", {OPR_reg_Rn}, 0xf400060f, 0xffb00f0f},
    {aarch32_op_VST1_m_A3_posti, "vst1 VST1_m_A3_posti", {OPR_reg_Rn}, 0xf400060d, 0xffb00f0f},
    {aarch32_op_VST1_m_A3_postr, "vst1 VST1_m_A3_postr", {OPR_reg_Rn, OPR_reg_Rm}, 0xf4000600, 0xffb00f00},
    {aarch32_op_VST1_m_A4_nowb, "vst1 VST1_m_A4_nowb", {OPR_reg_Rn}, 0xf400020f, 0xffb00f0f},
    {aarch32_op_VST1_m_A4_posti, "vst1 VST1_m_A4_posti", {OPR_reg_Rn}, 0xf400020d, 0xffb00f0f},
    {aarch32_op_VST1_m_A4_postr, "vst1 VST1_m_A4_postr", {OPR_reg_Rn, OPR_reg_Rm}, 0xf4000200, 0xffb00f00},
    {aarch32_op_VST2_1_A1_nowb, "vst2 VST2_1_A1_nowb", {OPR_reg_Rn}, 0xf480010f, 0xffb00f0f},
    {aarch32_op_VST2_1_A1_posti, "vst2 VST2_1_A1_posti", {OPR_reg_Rn}, 0xf480010d, 0xffb00f0f},
    {aarch32_op_VST2_1_A1_postr, "vst2 VST2_1_A1_postr", {OPR_reg_Rn, OPR_reg_Rm}, 0xf4800100, 0xffb00f00},
    {aarch32_op_VST2_1_A2_nowb, "vst2 VST2_1_A2_nowb", {OPR_reg_Rn}, 0xf480050f, 0xffb00f0f},
    {aarch32_op_VST2_1_A2_posti, "vst2 VST2_1_A2_posti", {OPR_reg_Rn}, 0xf480050d, 0xffb00f0f},
    {aarch32_op_VST2_1_A2_postr, "vst2 VST2_1_A2_postr", {OPR_reg_Rn, OPR_reg_Rm}, 0xf4800500, 0xffb00f00},
    {aarch32_op_VST2_1_A3_nowb, "vst2 VST2_1_A3_nowb", {OPR_reg_Rn}, 0xf480090f, 0xffb00f0f},
    {aarch32_op_VST2_1_A3_posti, "vst2 VST2_1_A3_posti", {OPR_reg_Rn}, 0xf480090d, 0xffb00f0f},
    {aarch32_op_VST2_1_A3_postr, "vst2 VST2_1_A3_postr", {OPR_reg_Rn, OPR_reg_Rm}, 0xf4800900, 0xffb00f00},
    {aarch32_op_VST2_m_A1_nowb, "vst2 VST2_m_A1_nowb", {OPR_reg_Rn}, 0xf400080f, 0xffb00e0f},
    {aarch32_op_VST2_m_A1_posti, "vst2 VST2_m_A1_posti", {OPR_reg_Rn}, 0xf400080d, 0xffb00e0f},
    {aarch32_op_VST2_m_A1_postr, "vst2 VST2_m_A1_postr", {OPR_reg_Rn, OPR_reg_Rm}, 0xf4000800, 0xffb00e00},
    {aarch32_op_VST2_m_A2_nowb, "vst2 VST2_m_A2_nowb", {OPR_reg_Rn}, 0xf400030f, 0xffb00f0f},
    {aarch32_op_VST2_m_A2_posti, "vst2 VST2_m_A2_posti", {OPR_reg_Rn}, 0xf400030d, 0xffb00f0f},
    {aarch32_op_VST2_m_A2_postr, "vst2 VST2_m_A2_postr", {OPR_reg_Rn, OPR_reg_Rm}, 0xf4000300, 0xffb00f00},
    {aarch32_op_VST3_1_A1_nowb, "vst3 VST3_1_A1_nowb", {OPR_reg_Rn}, 0xf480020f, 0xffb00f0f},
    {aarch32_op_VST3_1_A1_posti, "vst3 VST3_1_A1_posti", {OPR_reg_Rn}, 0xf480020d, 0xffb00f0f},
    {aarch32_op_VST3_1_A1_postr, "vst3 VST3_1_A1_postr", {OPR_reg_Rn, OPR_reg_Rm}, 0xf4800200, 0xffb00f00},
    {aarch32_op_VST3_1_A2_nowb, "vst3 VST3_1_A2_nowb", {OPR_reg_Rn}, 0xf480060f, 0xffb00f0f},
    {aarch32_op_VST3_1_A2_posti, "vst3 VST3_1_A2_posti", {OPR_reg_Rn}, 0xf480060d, 0xffb00f0f},
    {aarch32_op_VST3_1_A2_postr, "vst3 VST3_1_A2_postr", {OPR_reg_Rn, OPR_reg_Rm}, 0xf4800600, 0xffb00f00},
    {aarch32_op_VST3_1_A3_nowb, "vst3 VST3_1_A3_nowb", {OPR_reg_Rn}, 0xf4800a0f, 0xffb00f0f},
    {aarch32_op_VST3_1_A3_posti, "vst3 VST3_1_A3_posti", {OPR_reg_Rn}, 0xf4800a0d, 0xffb00f0f},
    {aarch32_op_VST3_1_A3_postr, "vst3 VST3_1_A3_postr", {OPR_reg_Rn, OPR_reg_Rm}, 0xf4800a00, 0xffb00f00},
    {aarch32_op_VST3_m_A1_nowb, "vst3 VST3_m_A1_nowb", {OPR_reg_Rn}, 0xf400040f, 0xffb00e0f},
    {aarch32_op_VST3_m_A1_posti, "vst3 VST3_m_A1_posti", {OPR_reg_Rn}, 0xf400040d, 0xffb00e0f},
    {aarch32_op_VST3_m_A1_postr, "vst3 VST3_m_A1_postr", {OPR_reg_Rn, OPR_reg_Rm}, 0xf4000400, 0xffb00e00},
    {aarch32_op_VST4_1_A1_nowb, "vst4 VST4_1_A1_nowb", {OPR_reg_Rn}, 0xf480030f, 0xffb00f0f},
    {aarch32_op_VST4_1_A1_posti, "vst4 VST4_1_A1_posti", {OPR_reg_Rn}, 0xf480030d, 0xffb00f0f},
    {aarch32_op_VST4_1_A1_postr, "vst4 VST4_1_A1_postr", {OPR_reg_Rn, OPR_reg_Rm}, 0xf4800300, 0xffb00f00},
    {aarch32_op_VST4_1_A2_nowb, "vst4 VST4_1_A2_nowb", {OPR_reg_Rn}, 0xf480070f, 0xffb00f0f},
    {aarch32_op_VST4_1_A2_posti, "vst4 VST4_1_A2_posti", {OPR_reg_Rn}, 0xf480070d, 0xffb00f0f},
    {aarch32_op_VST4_1_A2_postr, "vst4 VST4_1_A2_postr", {OPR_reg_Rn, OPR_reg_Rm}, 0xf4800700, 0xffb00f00},
    {aarch32_op_VST4_1_A3_nowb, "vst4 VST4_1_A3_nowb", {OPR_reg_Rn}, 0xf4800b0f, 0xffb00f0f},
    {aarch32_op_VST4_1_A3_posti, "vst4 VST4_1_A3_posti", {OPR_reg_Rn}, 0xf4800b0d, 0xffb00f0f},
    {aarch32_op_VST4_1_A3_postr, "vst4 VST4_1_A3_postr", {OPR_reg_Rn, OPR_reg_Rm}, 0xf4800b00, 0xffb00f00},
    {aarch32_op_VST4_m_A1_nowb, "vst4 VST4_m_A1_nowb", {OPR_reg_Rn}, 0xf400000f, 0xffb00e0f},
    {aarch32_op_VST4_m_A1_posti, "vst4 VST4_m_A1_posti", {OPR_reg_Rn}, 0xf400000d, 0xffb00e0f},
    {aarch32_op_VST4_m_A1_postr, "vst4 VST4_m_A1_postr", {OPR_reg_Rn, OPR_reg_Rm}, 0xf4000000, 0xffb00e00},
    {aarch32_op_VSTMDB_A1, "vstmdb VSTMDB_A1", {OPR_reg_Rn}, 0x0d200b00, 0x0fb00f01},
    {aarch32_op_VSTM_A1, "vstm VSTM_A1", {OPR_reg_Rn}, 0x0c800b00, 0x0f900f01},
    {aarch32_op_VSTMDB_A2, "vstmdb VSTMDB_A2", {OPR_reg_Rn}, 0x0d200a00, 0x0fb00f00},
    {aarch32_op_VSTM_A2, "vstm VSTM_A2", {OPR_reg_Rn}, 0x0c800a00, 0x0f900f00},
    {aarch32_op_VSTR_A1_H, "vstr VSTR_A1_H", {OPR_reg_Sd, OPR_reg_Rn}, 0x0d000900, 0x0f300f00},
    {aarch32_op_VSTR_A1_S, "vstr VSTR_A1_S", {OPR_reg_Sd, OPR_reg_Rn}, 0x0d000a00, 0x0f300f00},
    {aarch32_op_VSTR_A1_D, "vstr VSTR_A1_D", {OPR_reg_Dd, OPR_reg_Rn}, 0x0d000b00, 0x0f300f00},
    {aarch32_op_VSUB_f_A1_D, "vsub VSUB_f_A1_D", {OPR_reg_Dd, OPR_reg_Dn, OPR_reg_Dm}, 0xf2200d00, 0xffa00f50},
    {aarch32_op_VSUB_f_A1_Q, "vsub VSUB_f_A1_Q", {OPR_reg_Qd, OPR_reg_Qn, OPR_reg_Qm}, 0xf2200d40, 0xffa00f50},
    {aarch32_op_VSUB_f_A2_H, "vsub VSUB_f_A2_H", {OPR_reg_Sd, OPR_reg_Sn, OPR_reg_Sm}, 0x0e300940, 0x0fb00f50},
    {aarch32_op_VSUB_f_A2_S, "vsub VSUB_f_A2_S", {OPR_reg_Sd, OPR_reg_Sn, OPR_reg_Sm}, 0x0e300a40, 0x0fb00f50},
    {aarch32_op_VSUB_f_A2_D, "vsub VSUB_f_A2_D", {OPR_reg_Dd, OPR_reg_Dn, OPR_reg_Dm}, 0x0e300b40, 0x0fb00f50},
    {aarch32_op_VSUB_i_A1_D, "vsub VSUB_i_A1_D", {OPR_reg_Dd, OPR_reg_Dn, OPR_reg_Dm}, 0xf3000800, 0xff800f50},
    {aarch32_op_VSUB_i_A1_Q, "vsub VSUB_i_A1_Q", {OPR_reg_Qd, OPR_reg_Qn, OPR_reg_Qm}, 0xf3000840, 0xff800f50},
    {aarch32_op_VSUBHN_A1, "vsubhn VSUBHN_A1", {OPR_reg_Dd, OPR_reg_Qn, OPR_reg_Qm}, 0xf2800600, 0xff800f50},
    {aarch32_op_VSUBL_A1, "vsubl VSUBL_A1", {OPR_reg_Qd, OPR_reg_Dn, OPR_reg_Dm}, 0xf2800200, 0xfe800f50},
    {aarch32_op_VSUBW_A1, "vsubw VSUBW_A1", {OPR_reg_Qd, OPR_reg_Qn, OPR_reg_Dm}, 0xf2800300, 0xfe800f50},
    {aarch32_op_VSWP_A1_D, "vswp VSWP_A1_D", {OPR_reg_Dd, OPR_reg_Dm}, 0xf3b20000, 0xffbf0fd0},
    {aarch32_op_VSWP_A1_Q, "vswp VSWP_A1_Q", {OPR_reg_Qd, OPR_reg_Qm}, 0xf3b20040, 0xffbf0fd0},
    {aarch32_op_VTBL_A1, "vtbl VTBL_A1", {OPR_reg_Dd, OPR_reg_Dm}, 0xf3b00800, 0xffb00c50},
    {aarch32_op_VTBX_A1, "vtbx VTBX_A1", {OPR_reg_Dd, OPR_reg_Dm}, 0xf3b00840, 0xffb00c50},
    {aarch32_op_VTRN_A1_D, "vtrn VTRN_A1_D", {OPR_reg_Dd, OPR_reg_Dm}, 0xf3b20080, 0xffb30fd0},
    {aarch32_op_VTRN_A1_Q, "vtrn VTRN_A1_Q", {OPR_reg_Qd, OPR_reg_Qm}, 0xf3b200c0, 0xffb30fd0},
    {aarch32_op_VTST_A1_D, "vtst VTST_A1_D", {OPR_reg_Dd, OPR_reg_Dn, OPR_reg_Dm}, 0xf2000810, 0xff800f50},
    {aarch32_op_VTST_A1_Q, "vtst VTST_A1_Q", {OPR_reg_Qd, OPR_reg_Qn, OPR_reg_Qm}, 0xf2000850, 0xff800f50},
    {aarch32_op_VUZP_A1_D, "vuzp VUZP_A1_D", {OPR_reg_Dd, OPR_reg_Dm}, 0xf3b20100, 0xffb30fd0},
    {aarch32_op_VUZP_A1_Q, "vuzp VUZP_A1_Q", {OPR_reg_Qd, OPR_reg_Qm}, 0xf3b20140, 0xffb30fd0},
    {aarch32_op_VUZP_VTRN_A1_D, "vuzp VUZP_VTRN_A1_D", {OPR_reg_Dd, OPR_reg_Dm}, 0xf3b20080, 0xffb30fd0},
    {aarch32_op_VZIP_A1_D, "vzip VZIP_A1_D", {OPR_reg_Dd, OPR_reg_Dm}, 0xf3b20180, 0xffb30fd0},
    {aarch32_op_VZIP_A1_Q, "vzip VZIP_A1_Q", {OPR_reg_Qd, OPR_reg_Qm}, 0xf3b201c0, 0xffb30fd0},
    {aarch32_op_VZIP_VTRN_A1_D, "vzip VZIP_VTRN_A1_D", {OPR_reg_Dd, OPR_reg_Dm}, 0xf3b20080, 0xffb30fd0},
    {aarch32_op_WFE_A1, "wfe WFE_A1", {}, 0x03200002, 0x0fff00ff},
    {aarch32_op_WFI_A1, "wfi WFI_A1", {}, 0x03200003, 0x0fff00ff},
    {aarch32_op_YIELD_A1, "yield YIELD_A1", {}, 0x03200001, 0x0fff00ff}
};

static Insn_Entry_t* INVALID_INSN = &insnTable[0];

typedef struct Mask_Entry {
    uint32_t mask;
    int valueIdx;
    int length;
} Mask_Entry_t;

static Mask_Entry_t maskTable[] = {
    {0x0e000000,    0,   8},
    {0x00100000,    8,   2},
    {0x00000000,  422, 134},
    {0x00000000,   19, 133},
    {0x00000000,  292, 132},
    {0x00000000,   72, 131},
    {0x00000000,   71, 130},
    {0x00000000,   70, 129},
    {0x00000000,   69, 128},
    {0x00000000,   68, 127},
    {0x00000000,  414, 126},
    {0x00000000,  283, 125},
    {0x00000000,  272, 124},
    {0x00000000,  264, 123},
    {0x00000000,  260, 122},
    {0x00000000,  216, 121},
    {0x00000000,  207, 120},
    {0x00000000,  189, 119},
    {0x00000000,   84, 118},
    {0x00000000,   46, 117},
    {0x00000000,   31, 116},
    {0x00000000,   11, 115},
    {0x00000000,    2, 114},
    {0x00000000,  400, 113},
    {0x00000000,  399, 112},
    {0x00000000,  398, 111},
    {0x00000000,  397, 110},
    {0x00000000,  371, 109},
    {0x00000000,  370, 108},
    {0x00000000,  369, 107},
    {0x00000000,  368, 106},
    {0x00000000,  367, 105},
    {0x00000000,  366, 104},
    {0x00000000,  365, 103},
    {0x00000000,  423, 102},
    {0x00000000,  136, 101},
    {0x00000000,   78, 100},
    {0x00000000,   77,  99},
    {0x00000000,   76,  98},
    {0x00000000,   75,  97},
    {0x00000000,   74,  96},
    {0x00000000,   73,  95},
    {0x00000000,   20,  94},
    {0x00000000,  457,  93},
    {0x00000000,  455,  92},
    {0x00000000,  453,  91},
    {0x00000000,  348,  90},
    {0x00000000,  347,  89},
    {0x00000000,  346,  88},
    {0x00000000,  344,  87},
    {0x00000000,  343,  86},
    {0x00000000,  342,  85},
    {0x00000000,  341,  84},
    {0x00000000,  328,  83},
    {0x00000000,  327,  82},
    {0x00000000,  324,  81},
    {0x00000000,  323,  80},
    {0x00000000,  322,  79},
    {0x00000000,  321,  78},
    {0x00000000,  320,  77},
    {0x00000000,  316,  76},
    {0x00000000,  315,  75},
    {0x00000000,  314,  74},
    {0x00000000,  313,  73},
    {0x00000000,  312,  72},
    {0x00000000,  257,  71},
    {0x00000000,  245,  70},
    {0x00000000,  243,  69},
    {0x00000000,  242,  68},
    {0x00000000,  238,  67},
    {0x00000000,  204,  66},
    {0x00000000,  202,  65},
    {0x00000000,  200,  64},
    {0x00000000,  199,  63},
    {0x00000000,  198,  62},
    {0x00000000,  185,  61},
    {0x00000000,  184,  60},
    {0x00000000,  178,  59},
    {0x00000000,  174,  58},
    {0x00000000,   97,  57},
    {0x00000000,   96,  56},
    {0x00000000,   90,  55},
    {0x00000000,   59,  54},
    {0x00000000,   57,  53},
    {0x00000000,   56,  52},
    {0x00000000,   55,  51},
    {0x00000000,   52,  50},
    {0x00000000,   38,  49},
    {0x00000000,  408,  48},
    {0x00000000,  407,  47},
    {0x00000000,  406,  46},
    {0x00000000,  405,  45},
    {0x00000000,  404,  44},
    {0x00000000,  403,  43},
    {0x00000000,  402,  42},
    {0x00000000,  401,  41},
    {0x00000000,  396,  40},
    {0x00000000,  395,  39},
    {0x00000000,  394,  38},
    {0x00000000,  393,  37},
    {0x00000000,  392,  36},
    {0x00000000,  391,  35},
    {0x00000000,  256,  34},
    {0x00000000,  177,  33},
    {0x00000000,  173,  32},
    {0x00000000,  139,  31},
    {0x00000000,  138,  30},
    {0x00000000,  137,  29},
    {0x00000000,  135,  28},
    {0x00000000,  134,  27},
    {0x00000000,  133,  26},
    {0x00000000,   37,  25},
    {0x00000000,  419,  24},
    {0x00000000,  288,  23},
    {0x00000000,  277,  22},
    {0x00000000,  269,  21},
    {0x00000000,  221,  20},
    {0x00000000,  212,  19},
    {0x00000000,  194,  18},
    {0x00000000,   89,  17},
    {0x00000000,   51,  16},
    {0x00000000,   36,  15},
    {0x00000000,   16,  14},
    {0x00000000,    7,  13},
    {0x00000000,  415,  12},
    {0x00000000,  284,  11},
    {0x00000000,  273,  10},
    {0x00000000,  265,   9},
    {0x00000000,  217,   8},
    {0x00000000,  208,   7},
    {0x00000000,  190,   6},
    {0x00000000,   85,   5},
    {0x00000000,   47,   4},
    {0x00000000,   32,   3},
    {0x00000000,   12,   2},
    {0x00000000,    3,   1},
    {0x00400010,   10,   4},
    {0x01a00000,   14,   8},
    {0x00000000,   33,   2},
    {0x00000000,   34,   1},
    {0x00000000,   86,   2},
    {0x00000000,   87,   1},
    {0x00000000,   21,   4},
    {0x00000000,   13,   3},
    {0x00000000,   22,   2},
    {0x00000000,   14,   1},
    {0x00000000,    4,   2},
    {0x00000000,    5,   1},
    {0x00000000,  438,   3},
    {0x00000000,  293,   2},
    {0x00000000,  439,   1},
    {0x00000000,  434,   2},
    {0x00000000,  435,   1},
    {0x00000000,  218,   2},
    {0x00000000,  219,   1},
    {0x00000000,  261,   7},
    {0x00000000,  191,   6},
    {0x00000000,  258,   5},
    {0x00000000,  179,   4},
    {0x00000000,  175,   3},
    {0x00000000,   39,   2},
    {0x00000000,  192,   1},
    {0x01200080,   22,   8},
    {0x00800000,   30,   2},
    {0x00000000,   35,   1},
    {0x00000000,   15,   1},
    {0x00000060,   32,   4},
    {0x00800000,   36,   2},
    {0x00000000,  203,   1},
    {0x00000000,  456,   1},
    {0x00000000,  149,   1},
    {0x00000000,  158,   1},
    {0x00000000,  167,   1},
    {0x00800000,   38,   2},
    {0x00000000,   88,   1},
    {0x00000000,    6,   1},
    {0x00000060,   40,   4},
    {0x00800000,   44,   2},
    {0x00000000,  183,   1},
    {0x00000000,  454,   1},
    {0x00000000,  152,   1},
    {0x00000000,  161,   1},
    {0x00000000,  170,   1},
    {0x00800000,   46,   2},
    {0x00000000,  440,   1},
    {0x00000000,  220,   1},
    {0x00000060,   48,   4},
    {0x00800300,   52,   3},
    {0x00000000,   99,   1},
    {0x00000000,  101,   1},
    {0x00000000,  140,   1},
    {0x00000000,  148,   1},
    {0x00000000,  157,   1},
    {0x00000000,  166,   1},
    {0x00800000,   55,   2},
    {0x00000000,  436,   1},
    {0x00000000,  259,   5},
    {0x00000000,  180,   4},
    {0x00000000,  176,   3},
    {0x00000000,   40,   2},
    {0x00000000,  193,   1},
    {0x00000060,   57,   4},
    {0x00800300,   61,   2},
    {0x00000000,  103,   1},
    {0x00000000,  142,   1},
    {0x00000000,  150,   1},
    {0x00000000,  159,   1},
    {0x00000000,  168,   1},
    {0x01a00000,   63,   8},
    {0x00000000,  424,   4},
    {0x00000000,  416,   3},
    {0x00000000,  425,   2},
    {0x00000000,  417,   1},
    {0x00000000,  266,   2},
    {0x00000000,  267,   1},
    {0x00000000,  285,   2},
    {0x00000000,  286,   1},
    {0x00000000,  274,   2},
    {0x00000000,  275,   1},
    {0x00000000,   65,   2},
    {0x00000000,   66,   1},
    {0x00000000,   61,   2},
    {0x00000000,   62,   1},
    {0x00000000,   48,   2},
    {0x00000000,   49,   1},
    {0x00000000,  209,   2},
    {0x00000000,  210,   1},
    {0x00000080,   71,   2},
    {0x01a00000,   73,   8},
    {0x00000000,  418,   1},
    {0x00000000,  268,   1},
    {0x00000000,  287,   1},
    {0x00000000,  276,   1},
    {0x00000000,   67,   1},
    {0x00000000,   63,   1},
    {0x00000000,   50,   1},
    {0x00000000,  211,   1},
    {0x00000060,   81,   4},
    {0x01a00000,   85,   4},
    {0x00000000,  345,   1},
    {0x00000000,  319,   1},
    {0x00000300,   89,   3},
    {0x00000000,  100,   1},
    {0x00000000,  102,   1},
    {0x00000000,  141,   1},
    {0x00000300,   92,   3},
    {0x00000000,  105,   1},
    {0x00000000,  104,   1},
    {0x00000000,  143,   1},
    {0x00000000,  147,   5},
    {0x00000000,  151,   4},
    {0x00000000,  146,   3},
    {0x00000000,  145,   2},
    {0x00000000,  144,   1},
    {0x00000000,  156,   5},
    {0x00000000,  160,   4},
    {0x00000000,  155,   3},
    {0x00000000,  154,   2},
    {0x00000000,  153,   1},
    {0x00000000,  165,   5},
    {0x00000000,  169,   4},
    {0x00000000,  164,   3},
    {0x00000000,  163,   2},
    {0x00000000,  162,   1},
    {0x00800000,   95,   2},
    {0x00000000, 1111, 164},
    {0x00000000, 1110, 163},
    {0x00000000, 1109, 162},
    {0x00000000,  295, 161},
    {0x00000000,  294, 160},
    {0x00000000,  213, 159},
    {0x00000000,   91, 158},
    {0x00000000, 1006, 157},
    {0x00000000, 1005, 156},
    {0x00000000,  975, 155},
    {0x00000000,  974, 154},
    {0x00000000,  861, 153},
    {0x00000000,  860, 152},
    {0x00000000,  851, 151},
    {0x00000000,  850, 150},
    {0x00000000,  811, 149},
    {0x00000000,  810, 148},
    {0x00000000,  639, 147},
    {0x00000000,  638, 146},
    {0x00000000,  526, 145},
    {0x00000000,  525, 144},
    {0x00000000,  524, 143},
    {0x00000000,  523, 142},
    {0x00000000,  522, 141},
    {0x00000000,  521, 140},
    {0x00000000,  520, 139},
    {0x00000000,  519, 138},
    {0x00000000,  510, 137},
    {0x00000000,  509, 136},
    {0x00000000, 1086, 135},
    {0x00000000, 1085, 134},
    {0x00000000,  981, 133},
    {0x00000000,  980, 132},
    {0x00000000,  928, 131},
    {0x00000000,  927, 130},
    {0x00000000,  870, 129},
    {0x00000000,  868, 128},
    {0x00000000,  818, 127},
    {0x00000000,  817, 126},
    {0x00000000,  777, 125},
    {0x00000000,  776, 124},
    {0x00000000,  766, 123},
    {0x00000000,  765, 122},
    {0x00000000,  761, 121},
    {0x00000000,  760, 120},
    {0x00000000,  757, 119},
    {0x00000000,  756, 118},
    {0x00000000,  752, 117},
    {0x00000000,  751, 116},
    {0x00000000,  748, 115},
    {0x00000000,  747, 114},
    {0x00000000,  650, 113},
    {0x00000000,  649, 112},
    {0x00000000,  645, 111},
    {0x00000000,  644, 110},
    {0x00000000,  558, 109},
    {0x00000000,  557, 108},
    {0x00000000,  550, 107},
    {0x00000000,  549, 106},
    {0x00000000,  544, 105},
    {0x00000000,  543, 104},
    {0x00000000,  538, 103},
    {0x00000000,  537, 102},
    {0x00000000,  532, 101},
    {0x00000000,  531, 100},
    {0x00000000,  500,  99},
    {0x00000000,  499,  98},
    {0x00000000,  498,  97},
    {0x00000000,  497,  96},
    {0x00000000,  496,  95},
    {0x00000000,  495,  94},
    {0x00000000,  494,  93},
    {0x00000000,  493,  92},
    {0x00000000,  492,  91},
    {0x00000000,  491,  90},
    {0x00000000,  482,  89},
    {0x00000000,  481,  88},
    {0x00000000,  305,  87},
    {0x00000000,  303,  86},
    {0x00000000,  302,  85},
    {0x00000000,  300,  84},
    {0x00000000,  299,  83},
    {0x00000000,  298,  82},
    {0x00000000,  296,  81},
    {0x00000000,   79,  80},
    {0x00000000, 1102,  79},
    {0x00000000, 1101,  78},
    {0x00000000, 1091,  77},
    {0x00000000, 1090,  76},
    {0x00000000,  903,  75},
    {0x00000000,  902,  74},
    {0x00000000,  899,  73},
    {0x00000000,  898,  72},
    {0x00000000,  895,  71},
    {0x00000000,  894,  70},
    {0x00000000,  885,  69},
    {0x00000000,  884,  68},
    {0x00000000,  864,  67},
    {0x00000000,  782,  66},
    {0x00000000,  781,  65},
    {0x00000000,  771,  64},
    {0x00000000,  770,  63},
    {0x00000000,  530,  62},
    {0x00000000,  529,  61},
    {0x00000000,  505,  60},
    {0x00000000,  504,  59},
    {0x00000000, 1000,  58},
    {0x00000000,  999,  57},
    {0x00000000,  971,  56},
    {0x00000000,  970,  55},
    {0x00000000,  936,  54},
    {0x00000000,  935,  53},
    {0x00000000,  923,  52},
    {0x00000000,  922,  51},
    {0x00000000,  917,  50},
    {0x00000000,  916,  49},
    {0x00000000,  907,  48},
    {0x00000000,  906,  47},
    {0x00000000,  879,  46},
    {0x00000000,  878,  45},
    {0x00000000,  871,  44},
    {0x00000000,  869,  43},
    {0x00000000,  865,  42},
    {0x00000000,  823,  41},
    {0x00000000,  822,  40},
    {0x00000000,  759,  39},
    {0x00000000,  758,  38},
    {0x00000000,  750,  37},
    {0x00000000,  749,  36},
    {0x00000000,  663,  35},
    {0x00000000,  662,  34},
    {0x00000000,  661,  33},
    {0x00000000,  660,  32},
    {0x00000000,  556,  31},
    {0x00000000,  555,  30},
    {0x00000000,  548,  29},
    {0x00000000,  547,  28},
    {0x00000000,  542,  27},
    {0x00000000,  541,  26},
    {0x00000000,  536,  25},
    {0x00000000,  535,  24},
    {0x00000000,  484,  23},
    {0x00000000,  483,  22},
    {0x00000000,  479,  21},
    {0x00000000,  478,  20},
    {0x00000000,  421,  19},
    {0x00000000,  420,  18},
    {0x00000000,  411,  17},
    {0x00000000,   24,  16},
    {0x00000000,  437,  15},
    {0x00000000,  433,  14},
    {0x00000000,  413,  13},
    {0x00000000,  412,  12},
    {0x00000000,  263,  11},
    {0x00000000,  262,  10},
    {0x00000000,  195,   9},
    {0x00000000,  188,   8},
    {0x00000000,   83,   7},
    {0x00000000,   82,   6},
    {0x00000000,   64,   5},
    {0x00000000,   60,   4},
    {0x00000000,   30,   3},
    {0x00000000,   29,   2},
    {0x00000000,  201,   1},
    {0x00000000, 1096, 213},
    {0x00000000, 1095, 212},
    {0x00000000, 1108, 211},
    {0x00000000, 1107, 210},
    {0x00000000, 1106, 209},
    {0x00000000, 1105, 208},
    {0x00000000, 1104, 207},
    {0x00000000, 1103, 206},
    {0x00000000, 1100, 205},
    {0x00000000, 1099, 204},
    {0x00000000, 1008, 203},
    {0x00000000, 1002, 202},
    {0x00000000,  977, 201},
    {0x00000000,  966, 200},
    {0x00000000,  965, 199},
    {0x00000000,  961, 198},
    {0x00000000,  960, 197},
    {0x00000000,  953, 196},
    {0x00000000,  952, 195},
    {0x00000000,  948, 194},
    {0x00000000,  947, 193},
    {0x00000000,  943, 192},
    {0x00000000,  942, 191},
    {0x00000000,  938, 190},
    {0x00000000,  937, 189},
    {0x00000000,  934, 188},
    {0x00000000,  933, 187},
    {0x00000000,  932, 186},
    {0x00000000,  931, 185},
    {0x00000000,  930, 184},
    {0x00000000,  929, 183},
    {0x00000000,  921, 182},
    {0x00000000,  911, 181},
    {0x00000000,  893, 180},
    {0x00000000,  892, 179},
    {0x00000000,  891, 178},
    {0x00000000,  877, 177},
    {0x00000000,  876, 176},
    {0x00000000,  835, 175},
    {0x00000000,  834, 174},
    {0x00000000,  813, 173},
    {0x00000000,  578, 172},
    {0x00000000,  577, 171},
    {0x00000000,  574, 170},
    {0x00000000,  573, 169},
    {0x00000000,  560, 168},
    {0x00000000,  559, 167},
    {0x00000000,  552, 166},
    {0x00000000,  551, 165},
    {0x00000000,  304, 164},
    {0x00000000,  301, 163},
    {0x00000000,  297, 162},
    {0x00000000,   28, 161},
    {0x00000000,   27, 160},
    {0x00000000,   26, 159},
    {0x00000000,   25, 158},
    {0x00000000,  979, 157},
    {0x00000000,  978, 156},
    {0x00000000,  926, 155},
    {0x00000000,  925, 154},
    {0x00000000,  920, 153},
    {0x00000000,  910, 152},
    {0x00000000,  890, 151},
    {0x00000000,  867, 150},
    {0x00000000,  866, 149},
    {0x00000000,  863, 148},
    {0x00000000,  862, 147},
    {0x00000000,  837, 146},
    {0x00000000,  836, 145},
    {0x00000000,  801, 144},
    {0x00000000,  800, 143},
    {0x00000000,  618, 142},
    {0x00000000,  617, 141},
    {0x00000000,  613, 140},
    {0x00000000,  612, 139},
    {0x00000000,  608, 138},
    {0x00000000,  607, 137},
    {0x00000000,  599, 136},
    {0x00000000,  598, 135},
    {0x00000000,  554, 134},
    {0x00000000,  553, 133},
    {0x00000000,  546, 132},
    {0x00000000,  545, 131},
    {0x00000000,  540, 130},
    {0x00000000,  539, 129},
    {0x00000000,  534, 128},
    {0x00000000,  533, 127},
    {0x00000000,  528, 126},
    {0x00000000,  527, 125},
    {0x00000000,  487, 124},
    {0x00000000,  486, 123},
    {0x00000000,  859, 122},
    {0x00000000,  858, 121},
    {0x00000000,  855, 120},
    {0x00000000,  854, 119},
    {0x00000000,  833, 118},
    {0x00000000,  832, 117},
    {0x00000000,  831, 116},
    {0x00000000,  830, 115},
    {0x00000000,  812, 114},
    {0x00000000,  797, 113},
    {0x00000000,  796, 112},
    {0x00000000,  637, 111},
    {0x00000000,  636, 110},
    {0x00000000,  580, 109},
    {0x00000000,  579, 108},
    {0x00000000,  518, 107},
    {0x00000000,  517, 106},
    {0x00000000,  514, 105},
    {0x00000000,  513, 104},
    {0x00000000,  857, 103},
    {0x00000000,  856, 102},
    {0x00000000,  853, 101},
    {0x00000000,  852, 100},
    {0x00000000,  829,  99},
    {0x00000000,  828,  98},
    {0x00000000,  799,  97},
    {0x00000000,  798,  96},
    {0x00000000,  792,  95},
    {0x00000000,  791,  94},
    {0x00000000,  516,  93},
    {0x00000000,  515,  92},
    {0x00000000,  512,  91},
    {0x00000000,  511,  90},
    {0x00000000, 1007,  89},
    {0x00000000,  976,  88},
    {0x00000000,  919,  87},
    {0x00000000,  909,  86},
    {0x00000000, 1098,  85},
    {0x00000000, 1097,  84},
    {0x00000000, 1092,  83},
    {0x00000000, 1017,  82},
    {0x00000000, 1016,  81},
    {0x00000000, 1010,  80},
    {0x00000000, 1009,  79},
    {0x00000000, 1001,  78},
    {0x00000000,  998,  77},
    {0x00000000,  997,  76},
    {0x00000000,  984,  75},
    {0x00000000,  924,  74},
    {0x00000000,  918,  73},
    {0x00000000,  915,  72},
    {0x00000000,  914,  71},
    {0x00000000,  908,  70},
    {0x00000000,  905,  69},
    {0x00000000,  904,  68},
    {0x00000000,  901,  67},
    {0x00000000,  900,  66},
    {0x00000000,  897,  65},
    {0x00000000,  896,  64},
    {0x00000000,  889,  63},
    {0x00000000,  888,  62},
    {0x00000000,  887,  61},
    {0x00000000,  886,  60},
    {0x00000000,  883,  59},
    {0x00000000,  882,  58},
    {0x00000000,  881,  57},
    {0x00000000,  880,  56},
    {0x00000000,  506,  55},
    {0x00000000, 1094,  54},
    {0x00000000, 1093,  53},
    {0x00000000, 1015,  52},
    {0x00000000, 1014,  51},
    {0x00000000, 1004,  50},
    {0x00000000, 1003,  49},
    {0x00000000,  983,  48},
    {0x00000000,  982,  47},
    {0x00000000,  973,  46},
    {0x00000000,  972,  45},
    {0x00000000,  913,  44},
    {0x00000000,  912,  43},
    {0x00000000,  827,  42},
    {0x00000000,  825,  41},
    {0x00000000,  824,  40},
    {0x00000000,  786,  39},
    {0x00000000,  785,  38},
    {0x00000000,  784,  37},
    {0x00000000,  783,  36},
    {0x00000000,  775,  35},
    {0x00000000,  774,  34},
    {0x00000000,  773,  33},
    {0x00000000,  772,  32},
    {0x00000000,  508,  31},
    {0x00000000,  507,  30},
    {0x00000000,  485,  29},
    {0x00000000,  480,  28},
    {0x00000000,  826,  27},
    {0x00000000,  643,  26},
    {0x00000000,  642,  25},
    {0x00000000,  641,  24},
    {0x00000000,  640,  23},
    {0x00000000,  591,  22},
    {0x00000000,  590,  21},
    {0x00000000,   23,  20},
    {0x00000000,   18,  19},
    {0x00000000,   17,  18},
    {0x00000000,    8,  17},
    {0x00000000,  282,  16},
    {0x00000000,  281,  15},
    {0x00000000,  271,  14},
    {0x00000000,  270,  13},
    {0x00000000,  215,  12},
    {0x00000000,  214,  11},
    {0x00000000,  206,  10},
    {0x00000000,  205,   9},
    {0x00000000,  187,   8},
    {0x00000000,  186,   7},
    {0x00000000,   45,   6},
    {0x00000000,   44,   5},
    {0x00000000,   10,   4},
    {0x00000000,    9,   3},
    {0x00000000,    1,   2},
    {0x00000000,    0,   1},
    {0x00100000,   97,   2},
    {0x01200000,   99,   4},
    {0x00000000, 1073,  62},
    {0x00000000, 1072,  61},
    {0x00000000, 1070,  60},
    {0x00000000, 1069,  59},
    {0x00000000, 1067,  58},
    {0x00000000, 1066,  57},
    {0x00000000, 1061,  56},
    {0x00000000, 1060,  55},
    {0x00000000, 1058,  54},
    {0x00000000, 1057,  53},
    {0x00000000, 1055,  52},
    {0x00000000, 1054,  51},
    {0x00000000, 1052,  50},
    {0x00000000, 1051,  49},
    {0x00000000, 1046,  48},
    {0x00000000, 1045,  47},
    {0x00000000, 1043,  46},
    {0x00000000, 1042,  45},
    {0x00000000, 1040,  44},
    {0x00000000, 1039,  43},
    {0x00000000, 1037,  42},
    {0x00000000, 1036,  41},
    {0x00000000, 1034,  40},
    {0x00000000, 1033,  39},
    {0x00000000, 1031,  38},
    {0x00000000, 1030,  37},
    {0x00000000, 1028,  36},
    {0x00000000, 1027,  35},
    {0x00000000, 1025,  34},
    {0x00000000, 1024,  33},
    {0x00000000, 1022,  32},
    {0x00000000, 1021,  31},
    {0x00000000, 1019,  30},
    {0x00000000, 1018,  29},
    {0x00000000, 1076,  28},
    {0x00000000, 1075,  27},
    {0x00000000, 1064,  26},
    {0x00000000, 1063,  25},
    {0x00000000, 1049,  24},
    {0x00000000, 1048,  23},
    {0x00000000, 1074,  22},
    {0x00000000, 1071,  21},
    {0x00000000, 1068,  20},
    {0x00000000, 1062,  19},
    {0x00000000, 1059,  18},
    {0x00000000, 1056,  17},
    {0x00000000, 1053,  16},
    {0x00000000, 1047,  15},
    {0x00000000, 1044,  14},
    {0x00000000, 1041,  13},
    {0x00000000, 1038,  12},
    {0x00000000, 1035,  11},
    {0x00000000, 1032,  10},
    {0x00000000, 1029,   9},
    {0x00000000, 1026,   8},
    {0x00000000, 1023,   7},
    {0x00000000, 1020,   6},
    {0x00000000, 1077,   5},
    {0x00000000, 1065,   4},
    {0x00000000, 1050,   3},
    {0x00000000,  384,   2},
    {0x00000000,  378,   1},
    {0x00000000,  717,  74},
    {0x00000000,  716,  73},
    {0x00000000,  732,  72},
    {0x00000000,  731,  71},
    {0x00000000,  729,  70},
    {0x00000000,  728,  69},
    {0x00000000,  726,  68},
    {0x00000000,  725,  67},
    {0x00000000,  723,  66},
    {0x00000000,  722,  65},
    {0x00000000,  714,  64},
    {0x00000000,  713,  63},
    {0x00000000,  711,  62},
    {0x00000000,  710,  61},
    {0x00000000,  708,  60},
    {0x00000000,  707,  59},
    {0x00000000,  705,  58},
    {0x00000000,  704,  57},
    {0x00000000,  699,  56},
    {0x00000000,  698,  55},
    {0x00000000,  696,  54},
    {0x00000000,  695,  53},
    {0x00000000,  693,  52},
    {0x00000000,  692,  51},
    {0x00000000,  690,  50},
    {0x00000000,  689,  49},
    {0x00000000,  687,  48},
    {0x00000000,  686,  47},
    {0x00000000,  684,  46},
    {0x00000000,  683,  45},
    {0x00000000,  681,  44},
    {0x00000000,  680,  43},
    {0x00000000,  678,  42},
    {0x00000000,  677,  41},
    {0x00000000,  675,  40},
    {0x00000000,  674,  39},
    {0x00000000,  672,  38},
    {0x00000000,  671,  37},
    {0x00000000,  669,  36},
    {0x00000000,  668,  35},
    {0x00000000,  666,  34},
    {0x00000000,  665,  33},
    {0x00000000,  735,  32},
    {0x00000000,  734,  31},
    {0x00000000,  720,  30},
    {0x00000000,  719,  29},
    {0x00000000,  702,  28},
    {0x00000000,  701,  27},
    {0x00000000,  718,  26},
    {0x00000000,  733,  25},
    {0x00000000,  730,  24},
    {0x00000000,  727,  23},
    {0x00000000,  724,  22},
    {0x00000000,  715,  21},
    {0x00000000,  712,  20},
    {0x00000000,  709,  19},
    {0x00000000,  706,  18},
    {0x00000000,  700,  17},
    {0x00000000,  697,  16},
    {0x00000000,  694,  15},
    {0x00000000,  691,  14},
    {0x00000000,  688,  13},
    {0x00000000,  685,  12},
    {0x00000000,  682,  11},
    {0x00000000,  679,  10},
    {0x00000000,  676,   9},
    {0x00000000,  673,   8},
    {0x00000000,  670,   7},
    {0x00000000,  667,   6},
    {0x00000000,  736,   5},
    {0x00000000,  721,   4},
    {0x00000000,  703,   3},
    {0x00000000,  409,   2},
    {0x00000000,  389,   1},
    {0x00400000,  103,   2},
    {0x00000000,  377,   1},
    {0x00000000,  383,   1},
    {0x00400000,  105,   2},
    {0x00000000,  237,   2},
    {0x00000000,  379,   1},
    {0x00000000,  385,   1},
    {0x00000000,  235,  19},
    {0x00000000,   98,  18},
    {0x00000000,   81,  17},
    {0x00000000,   80,  16},
    {0x00000000,   58,  15},
    {0x00000000,  226,  14},
    {0x00000000,  231,  13},
    {0x00000000,  225,  12},
    {0x00000000,  224,  11},
    {0x00000000,  127,  10},
    {0x00000000,  120,   9},
    {0x00000000,  171,   8},
    {0x00000000,  131,   7},
    {0x00000000,  126,   6},
    {0x00000000,  125,   5},
    {0x00000000,  124,   4},
    {0x00000000,  119,   3},
    {0x00000000,  118,   2},
    {0x00000000,  117,   1},
    {0x01600010,  107,  16},
    {0x00100000,  123,   2},
    {0x00000000,  381,   1},
    {0x00000000,  122,   1},
    {0x00900060,  125,   8},
    {0x00000080,  133,   2},
    {0x00000000,  278,   1},
    {0x00000000,  279,   1},
    {0x00000000,  280,   1},
    {0x00000000,  358,   1},
    {0x00000080,  135,   2},
    {0x00000000,  359,   1},
    {0x00000000,  360,   1},
    {0x00000000,  222,   1},
    {0x00000000,  291,   1},
    {0x00000000,  223,   1},
    {0x00000080,  137,   1},
    {0x00000000,  431,   2},
    {0x00000000,  428,   1},
    {0x00100000,  138,   2},
    {0x00000000,  410,   1},
    {0x00000000,  172,   1},
    {0x00800060,  140,   8},
    {0x00100080,  148,   4},
    {0x00000000,  239,   1},
    {0x00000000,  240,   1},
    {0x00000000,  306,   1},
    {0x00000000,  307,   1},
    {0x00100080,  152,   2},
    {0x00000000,  241,   1},
    {0x00000000,  308,   1},
    {0x00100080,  154,   2},
    {0x00000000,  244,   1},
    {0x00000000,  309,   1},
    {0x00100080,  156,   4},
    {0x00000000,  246,   1},
    {0x00000000,  247,   1},
    {0x00000000,  310,   1},
    {0x00000000,  311,   1},
    {0x00000000,  356,   1},
    {0x00100080,  160,   3},
    {0x00000000,  357,   1},
    {0x00000000,  249,   1},
    {0x00000000,  250,   1},
    {0x00000000,  355,   1},
    {0x00100080,  163,   2},
    {0x00000000,  430,   2},
    {0x00000000,  427,   1},
    {0x00000000,  432,   2},
    {0x00000000,  429,   1},
    {0x00100000,  165,   2},
    {0x00000000,  387,   1},
    {0x00000000,  232,   3},
    {0x00000000,  233,   2},
    {0x00000000,  129,   1},
    {0x009000e0,  167,   7},
    {0x00000000,  441,   1},
    {0x00000000,  443,   1},
    {0x00000000,  469,   1},
    {0x00000000,  470,   1},
    {0x00000000,  442,   1},
    {0x00000000,  471,   1},
    {0x00000000,  476,   2},
    {0x00000000,  473,   1},
    {0x00100000,  174,   2},
    {0x00000000,  390,   1},
    {0x00000000,  132,   1},
    {0x00800060,  176,   8},
    {0x00100080,  184,   4},
    {0x00000000,  458,   1},
    {0x00000000,  459,   1},
    {0x00000000,  447,   1},
    {0x00000000,  448,   1},
    {0x00100080,  188,   2},
    {0x00000000,  460,   1},
    {0x00000000,  449,   1},
    {0x00100080,  190,   2},
    {0x00000000,  461,   1},
    {0x00000000,  450,   1},
    {0x00100080,  192,   4},
    {0x00000000,  462,   1},
    {0x00000000,  463,   1},
    {0x00000000,  451,   1},
    {0x00000000,  452,   1},
    {0x00000000,  467,   1},
    {0x00100080,  196,   3},
    {0x00000000,  468,   1},
    {0x00000000,  248,   1},
    {0x00000000,  251,   1},
    {0x00000000,  466,   1},
    {0x00100080,  199,   2},
    {0x00000000,  475,   2},
    {0x00000000,  472,   1},
    {0x00000000,  477,   2},
    {0x00000000,  474,   1},
    {0x00100000,  201,   2},
    {0x00000000,  380,   1},
    {0x00000000,  230,   3},
    {0x00000000,  229,   2},
    {0x00000000,  121,   1},
    {0x009000e0,  203,   6},
    {0x00000000,  339,   2},
    {0x00000000,  317,   1},
    {0x00000000,  340,   2},
    {0x00000000,  318,   1},
    {0x00000000,  349,   2},
    {0x00000000,  329,   1},
    {0x00000000,  350,   2},
    {0x00000000,  330,   1},
    {0x00000000,  290,   1},
    {0x00000000,  464,   2},
    {0x00000000,  465,   1},
    {0x00100000,  209,   2},
    {0x00000000,  382,   1},
    {0x00000000,  123,   1},
    {0x00800060,  211,   2},
    {0x00000000,  446,   1},
    {0x00000000,  289,   1},
    {0x00100000,  213,   2},
    {0x00000000,  386,   1},
    {0x00000000,  228,   3},
    {0x00000000,  227,   2},
    {0x00000000,  128,   1},
    {0x00800060,  215,   5},
    {0x00100080,  220,   2},
    {0x00000000,  325,   1},
    {0x00000000,  337,   2},
    {0x00000000,  333,   1},
    {0x00100080,  222,   2},
    {0x00000000,  326,   1},
    {0x00000000,  338,   2},
    {0x00000000,  334,   1},
    {0x00100080,  224,   2},
    {0x00000000,  331,   1},
    {0x00000000,  335,   1},
    {0x00100080,  226,   2},
    {0x00000000,  332,   1},
    {0x00000000,  336,   1},
    {0x00000000,   42,   2},
    {0x00000000,   43,   1},
    {0x00100000,  228,   2},
    {0x00000000,  388,   1},
    {0x00000000,  130,   1},
    {0x00800060,  230,   2},
    {0x00000000,  444,   1},
    {0x00000000,  445,   1},
    {0x00500000,  232,   4},
    {0x01800000,  236,   4},
    {0x00000000,  374,   1},
    {0x00000000,  372,   1},
    {0x00000000,  236,   2},
    {0x00000000,  375,   1},
    {0x00000000,  376,   1},
    {0x01800000,  240,   4},
    {0x00000000,  252,   2},
    {0x00000000,  114,   1},
    {0x00000000,  234,   3},
    {0x00000000,  254,   2},
    {0x00000000,  111,   1},
    {0x00000000,  253,   2},
    {0x00000000,  115,   1},
    {0x00000000,  255,   2},
    {0x00000000,  116,   1},
    {0x00000000,  354,   5},
    {0x00000000,  353,   4},
    {0x00000000,  352,   3},
    {0x00000000,  351,   2},
    {0x00000000,  373,   1},
    {0x00008000,  244,   2},
    {0x00000000,  113,   1},
    {0x00000000,  112,   1},
    {0x00000000,   54,   3},
    {0x00000000,   53,   2},
    {0x00000000,   41,   1},
    {0x00100e00,  246,   6},
    {0x00000000, 1082,   1},
    {0x01000100,  252,   4},
    {0x00800000,  256,   2},
    {0x00000000,  808,   1},
    {0x00000000, 1081,   1},
    {0x00800000,  258,   2},
    {0x00000000,  787,   1},
    {0x00000001,  260,   2},
    {0x00000000, 1079,   1},
    {0x00000000,   95,   1},
    {0x00200000,  262,   2},
    {0x00000000, 1083,   1},
    {0x00800000,  264,   1},
    {0x00000000,  875,   2},
    {0x00000000, 1080,   1},
    {0x00200000,  265,   2},
    {0x00000000, 1084,   1},
    {0x00800001,  267,   2},
    {0x00000000,  874,   2},
    {0x00000000, 1078,   1},
    {0x00000000,   94,   1},
    {0x01600000,  269,   5},
    {0x00000000,  364,   1},
    {0x00000000,  362,   1},
    {0x00000000,  182,   1},
    {0x00000000,  361,   1},
    {0x00000000,  363,   1},
    {0x01200100,  274,   1},
    {0x00000000,  744,   2},
    {0x00000000,  741,   1},
    {0x01000100,  275,   4},
    {0x00800000,  279,   2},
    {0x00000000,  809,   1},
    {0x00000000,  873,   2},
    {0x00000000,  740,   1},
    {0x00800000,  281,   2},
    {0x00000000,  788,   1},
    {0x00000001,  283,   2},
    {0x00000000,  872,   2},
    {0x00000000,  738,   1},
    {0x00000000,   93,   1},
    {0x00200000,  285,   2},
    {0x00000000,  745,   2},
    {0x00000000,  742,   1},
    {0x00000000,  739,   1},
    {0x00200000,  287,   2},
    {0x00000000,  746,   2},
    {0x00000000,  743,   1},
    {0x00800001,  289,   2},
    {0x00000000,  737,   1},
    {0x00000000,   92,   1},
    {0x00000000,  110,   6},
    {0x00000000,  109,   5},
    {0x00000000,  108,   4},
    {0x00000000,  107,   3},
    {0x00000000,  106,   2},
    {0x00000000,  197,   1},
    {0x01000000,  291,   2},
    {0x00100e10,  293,  10},
    {0x00a00140,  303,   8},
    {0x00000000,  986,   2},
    {0x00000000,  767,   1},
    {0x00000000,  778,   1},
    {0x00000000,  989,   2},
    {0x00000000,  819,   1},
    {0x00000000,  847,   1},
    {0x00000000,  753,   2},
    {0x00000000,  632,   1},
    {0x00000000,  762,   1},
    {0x00000000,  646,   1},
    {0x00000000,  651,   1},
    {0x00000000,  789,   1},
    {0x00a00140,  311,  16},
    {0x00000000,  987,   2},
    {0x00000000,  768,   1},
    {0x00000000,  779,   1},
    {0x00000000,  985,   2},
    {0x00000000,  769,   1},
    {0x00000000,  780,   1},
    {0x00000000,  990,   2},
    {0x00000000,  820,   1},
    {0x00000000,  848,   1},
    {0x00000000,  988,   2},
    {0x00000000,  821,   1},
    {0x00000000,  849,   1},
    {0x00000000,  754,   2},
    {0x00000000,  633,   1},
    {0x00000000,  763,   1},
    {0x00000000,  755,   2},
    {0x00000000,  634,   1},
    {0x00000000,  764,   1},
    {0x00000000,  647,   1},
    {0x00000000,  652,   1},
    {0x00000000,  648,   1},
    {0x00000000,  653,   1},
    {0x00800100,  327,   4},
    {0x00000000,  805,   1},
    {0x00000000,  804,   1},
    {0x00000000,  816,   1},
    {0x00000000,  635,   1},
    {0x00000000,  181,   1},
    {0x00a00140,  331,   8},
    {0x00000000,  995,   2},
    {0x00000000,  844,   1},
    {0x00000000,  841,   1},
    {0x00000000,  992,   2},
    {0x00000000,  501,   1},
    {0x00000000, 1087,   1},
    {0x00000000,  657,   1},
    {0x00000000,  654,   1},
    {0x00000000,  793,   1},
    {0x000e0000,  339,   7},
    {0x00010080,  346,   3},
    {0x00000000,  488,   1},
    {0x00000000,  838,   1},
    {0x00000000, 1011,   1},
    {0x00010080,  349,   4},
    {0x00000000,  561,   1},
    {0x00000000,  567,   1},
    {0x00000000,  564,   1},
    {0x00000000,  570,   1},
    {0x00010080,  353,   3},
    {0x00000000,  957,   1},
    {0x00000000,  967,   1},
    {0x00000000,  962,   1},
    {0x00010000,  356,   2},
    {0x00000000,  939,   2},
    {0x00000000,  587,   1},
    {0x00000000,  949,   1},
    {0x00000000,  954,   3},
    {0x00000000,  944,   2},
    {0x00000000,  592,   1},
    {0x00010000,  358,   2},
    {0x00000000,  600,   3},
    {0x00000000,  622,   2},
    {0x00000000,  581,   1},
    {0x00000000,  614,   3},
    {0x00000000,  623,   2},
    {0x00000000,  582,   1},
    {0x00000000,  619,   3},
    {0x00000000,  609,   2},
    {0x00000000,  593,   1},
    {0x00000000,  790,   1},
    {0x00a00140,  360,  16},
    {0x00000000,  996,   2},
    {0x00000000,  845,   1},
    {0x00000000,  842,   1},
    {0x00000000,  994,   2},
    {0x00000000,  846,   1},
    {0x00000000,  843,   1},
    {0x00000000,  993,   2},
    {0x00000000,  502,   1},
    {0x00000000, 1088,   1},
    {0x00000000,  991,   2},
    {0x00000000,  503,   1},
    {0x00000000, 1089,   1},
    {0x00000000,  658,   1},
    {0x00000000,  655,   1},
    {0x00000000,  659,   1},
    {0x00000000,  656,   1},
    {0x00000000,  794,   1},
    {0x000e0000,  376,   8},
    {0x00010080,  384,   4},
    {0x00000000,  814,   2},
    {0x00000000,  802,   1},
    {0x00000000,  664,   2},
    {0x00000000,  489,   1},
    {0x00000000,  839,   1},
    {0x00000000, 1012,   1},
    {0x00010080,  388,   4},
    {0x00000000,  603,   1},
    {0x00000000,  628,   1},
    {0x00000000,  605,   1},
    {0x00000000,  630,   1},
    {0x00010080,  392,   4},
    {0x00000000,  562,   1},
    {0x00000000,  568,   1},
    {0x00000000,  565,   1},
    {0x00000000,  571,   1},
    {0x00010080,  396,   4},
    {0x00000000,  958,   1},
    {0x00000000,  968,   1},
    {0x00000000,  963,   1},
    {0x00000000,  575,   1},
    {0x00010000,  400,   2},
    {0x00000000,  940,   2},
    {0x00000000,  588,   1},
    {0x00000000,  950,   1},
    {0x00000000,  955,   3},
    {0x00000000,  945,   2},
    {0x00000000,  594,   1},
    {0x00010000,  402,   2},
    {0x00000000,  601,   3},
    {0x00000000,  624,   2},
    {0x00000000,  583,   1},
    {0x00000000,  615,   3},
    {0x00000000,  625,   2},
    {0x00000000,  584,   1},
    {0x00000000,  620,   3},
    {0x00000000,  610,   2},
    {0x00000000,  595,   1},
    {0x00000000,  795,   1},
    {0x000e0000,  404,   8},
    {0x00010080,  412,   4},
    {0x00000000,  803,   1},
    {0x00000000,  490,   1},
    {0x00000000,  840,   1},
    {0x00000000, 1013,   1},
    {0x00010080,  416,   4},
    {0x00000000,  604,   1},
    {0x00000000,  629,   1},
    {0x00000000,  606,   1},
    {0x00000000,  631,   1},
    {0x00010080,  420,   4},
    {0x00000000,  563,   1},
    {0x00000000,  569,   1},
    {0x00000000,  566,   1},
    {0x00000000,  572,   1},
    {0x00010080,  424,   4},
    {0x00000000,  959,   1},
    {0x00000000,  969,   1},
    {0x00000000,  964,   1},
    {0x00000000,  576,   1},
    {0x00010000,  428,   2},
    {0x00000000,  941,   2},
    {0x00000000,  589,   1},
    {0x00000000,  951,   1},
    {0x00000000,  956,   3},
    {0x00000000,  946,   2},
    {0x00000000,  596,   1},
    {0x00010000,  430,   2},
    {0x00000000,  602,   3},
    {0x00000000,  626,   2},
    {0x00000000,  585,   1},
    {0x00000000,  616,   3},
    {0x00000000,  627,   2},
    {0x00000000,  586,   1},
    {0x00000000,  621,   3},
    {0x00000000,  611,   2},
    {0x00000000,  597,   1},
    {0x00000100,  432,   2},
    {0x00e00000,  434,   2},
    {0x00000000,  806,   1},
    {0x00000000,  815,   1},
    {0x00000000,  807,   1},
    {0x00000000,  196,   1},
    {0x00000000,  426,   1},
};

typedef struct Value_Entry {
    uint32_t value;
    int maskIdx;
} Value_Entry_t;

static Value_Entry_t valueTable[] = {
    {0x00000000,    1},
    {0x02000000,  264},
    {0x04000000,  642},
    {0x06000000,  806},
    {0x08000000,  952},
    {0x0a000000,  977},
    {0x0c000000,  980},
    {0x0e000000, 1038},
    {0x00000000,    2},
    {0x00100000,  136},
    {0x00000000,  137},
    {0x00000010,  162},
    {0x00400000,  208},
    {0x00400010,  227},
    {0x00000000,  138},
    {0x00200000,  140},
    {0x00800000,  142},
    {0x00a00000,  146},
    {0x01000000,  148},
    {0x01200000,  151},
    {0x01800000,  153},
    {0x01a00000,  155},
    {0x00000000,  163},
    {0x00000080,  166},
    {0x00200000,  173},
    {0x00200080,  176},
    {0x01000000,  183},
    {0x01000080,  186},
    {0x01200000,  194},
    {0x01200080,  201},
    {0x00000000,  164},
    {0x00800000,  165},
    {0x00000000,  167},
    {0x00000020,  170},
    {0x00000040,  171},
    {0x00000060,  172},
    {0x00000000,  168},
    {0x00800000,  169},
    {0x00000000,  174},
    {0x00800000,  175},
    {0x00000000,  177},
    {0x00000020,  180},
    {0x00000040,  181},
    {0x00000060,  182},
    {0x00000000,  178},
    {0x00800000,  179},
    {0x00000000,  184},
    {0x00800000,  185},
    {0x00000000,  187},
    {0x00000020,  191},
    {0x00000040,  192},
    {0x00000060,  193},
    {0x00800000,  188},
    {0x00800200,  189},
    {0x00800300,  190},
    {0x00000000,  195},
    {0x00800000,  196},
    {0x00000000,  202},
    {0x00000020,  205},
    {0x00000040,  206},
    {0x00000060,  207},
    {0x00800200,  203},
    {0x00800300,  204},
    {0x00000000,  209},
    {0x00200000,  213},
    {0x00800000,  215},
    {0x00a00000,  217},
    {0x01000000,  219},
    {0x01200000,  221},
    {0x01800000,  223},
    {0x01a00000,  225},
    {0x00000000,  228},
    {0x00000080,  237},
    {0x00000000,  229},
    {0x00200000,  230},
    {0x00800000,  231},
    {0x00a00000,  232},
    {0x01000000,  233},
    {0x01200000,  234},
    {0x01800000,  235},
    {0x01a00000,  236},
    {0x00000000,  238},
    {0x00000020,  249},
    {0x00000040,  254},
    {0x00000060,  259},
    {0x00800000,  239},
    {0x00a00000,  240},
    {0x01800000,  241},
    {0x01a00000,  245},
    {0x00000000,  242},
    {0x00000200,  243},
    {0x00000300,  244},
    {0x00000000,  246},
    {0x00000200,  247},
    {0x00000300,  248},
    {0x00000000,  265},
    {0x00800000,  429},
    {0x00000000,  643},
    {0x00100000,  787},
    {0x00000000,  644},
    {0x00200000,  706},
    {0x01000000,  780},
    {0x01200000,  783},
    {0x00000000,  781},
    {0x00400000,  782},
    {0x00000000,  784},
    {0x00400000,  786},
    {0x00000000,  807},
    {0x00000010,  810},
    {0x00200000,  825},
    {0x00200010,  828},
    {0x00400000,  856},
    {0x00400010,  861},
    {0x00600000,  870},
    {0x00600010,  873},
    {0x01000000,  901},
    {0x01000010,  906},
    {0x01200000,  918},
    {0x01200010,  921},
    {0x01400000,  924},
    {0x01400010,  929},
    {0x01600000,  946},
    {0x01600010,  949},
    {0x00000000,  808},
    {0x00100000,  809},
    {0x00100000,  811},
    {0x00100020,  814},
    {0x00100040,  815},
    {0x00100060,  816},
    {0x00800000,  819},
    {0x00800020,  820},
    {0x00800040,  821},
    {0x00800060,  822},
    {0x00000000,  812},
    {0x00000080,  813},
    {0x00000000,  817},
    {0x00000080,  818},
    {0x00000000,  823},
    {0x00000000,  826},
    {0x00100000,  827},
    {0x00000000,  829},
    {0x00000020,  834},
    {0x00000040,  837},
    {0x00000060,  840},
    {0x00800000,  845},
    {0x00800020,  846},
    {0x00800040,  850},
    {0x00800060,  851},
    {0x00000000,  830},
    {0x00000080,  831},
    {0x00100000,  832},
    {0x00100080,  833},
    {0x00000000,  835},
    {0x00100000,  836},
    {0x00000000,  838},
    {0x00100000,  839},
    {0x00000000,  841},
    {0x00000080,  842},
    {0x00100000,  843},
    {0x00100080,  844},
    {0x00000000,  847},
    {0x00100000,  848},
    {0x00100080,  849},
    {0x00000000,  852},
    {0x00100000,  854},
    {0x00000000,  857},
    {0x00100000,  858},
    {0x00100000,  862},
    {0x00100020,  863},
    {0x00100040,  864},
    {0x00100060,  865},
    {0x00100080,  866},
    {0x001000e0,  867},
    {0x00800060,  868},
    {0x00000000,  871},
    {0x00100000,  872},
    {0x00000000,  874},
    {0x00000020,  879},
    {0x00000040,  882},
    {0x00000060,  885},
    {0x00800000,  890},
    {0x00800020,  891},
    {0x00800040,  895},
    {0x00800060,  896},
    {0x00000000,  875},
    {0x00000080,  876},
    {0x00100000,  877},
    {0x00100080,  878},
    {0x00000000,  880},
    {0x00100000,  881},
    {0x00000000,  883},
    {0x00100000,  884},
    {0x00000000,  886},
    {0x00000080,  887},
    {0x00100000,  888},
    {0x00100080,  889},
    {0x00000000,  892},
    {0x00100000,  893},
    {0x00100080,  894},
    {0x00000000,  897},
    {0x00100000,  899},
    {0x00000000,  902},
    {0x00100000,  903},
    {0x00000000,  907},
    {0x00000020,  909},
    {0x00000040,  911},
    {0x00000060,  913},
    {0x00100000,  915},
    {0x00800000,  916},
    {0x00000000,  919},
    {0x00100000,  920},
    {0x00000000,  922},
    {0x00800040,  923},
    {0x00000000,  925},
    {0x00100000,  926},
    {0x00000000,  930},
    {0x00000020,  934},
    {0x00000040,  938},
    {0x00000060,  941},
    {0x00800000,  944},
    {0x00000000,  931},
    {0x00100000,  932},
    {0x00000000,  935},
    {0x00100000,  936},
    {0x00000000,  939},
    {0x00100080,  940},
    {0x00000000,  942},
    {0x00100080,  943},
    {0x00000000,  947},
    {0x00100000,  948},
    {0x00800040,  950},
    {0x00800060,  951},
    {0x00000000,  953},
    {0x00100000,  959},
    {0x00400000,  969},
    {0x00500000,  974},
    {0x00000000,  954},
    {0x00800000,  955},
    {0x01000000,  956},
    {0x01800000,  958},
    {0x00000000,  960},
    {0x00800000,  962},
    {0x01000000,  965},
    {0x01800000,  967},
    {0x00000000,  975},
    {0x00008000,  976},
    {0x00000800,  981},
    {0x00000a00,  982},
    {0x00000e00, 1002},
    {0x00100800, 1008},
    {0x00100a00, 1011},
    {0x00100e00, 1032},
    {0x00000000,  983},
    {0x00000100,  986},
    {0x01000000,  991},
    {0x01000100,  996},
    {0x00000000,  984},
    {0x00800000,  985},
    {0x00000000,  987},
    {0x00800000,  988},
    {0x00000000,  989},
    {0x00000001,  990},
    {0x00000000,  992},
    {0x00200000,  993},
    {0x00000000,  994},
    {0x00000000,  997},
    {0x00200000,  998},
    {0x00000000,  999},
    {0x00000001, 1001},
    {0x00000000, 1003},
    {0x00200000, 1004},
    {0x00400000, 1005},
    {0x01000000, 1006},
    {0x01200000, 1007},
    {0x01000100, 1009},
    {0x00000000, 1012},
    {0x00000100, 1016},
    {0x01000000, 1022},
    {0x01000100, 1026},
    {0x00000000, 1013},
    {0x00800000, 1014},
    {0x00000000, 1017},
    {0x00800000, 1018},
    {0x00000000, 1019},
    {0x00000001, 1021},
    {0x00000000, 1023},
    {0x00200000, 1025},
    {0x00000000, 1027},
    {0x00200000, 1029},
    {0x00000000, 1030},
    {0x00000001, 1031},
    {0x00000000, 1039},
    {0x01000000, 1227},
    {0x00000800, 1040},
    {0x00000810, 1052},
    {0x00000a00, 1053},
    {0x00000a10, 1076},
    {0x00000e10, 1081},
    {0x00100800, 1082},
    {0x00100810, 1123},
    {0x00100a00, 1124},
    {0x00100a10, 1221},
    {0x00100e10, 1226},
    {0x00000100, 1041},
    {0x00000140, 1043},
    {0x00200100, 1044},
    {0x00200140, 1046},
    {0x00800100, 1047},
    {0x00800140, 1049},
    {0x00a00100, 1050},
    {0x00a00140, 1051},
    {0x00000000, 1054},
    {0x00000040, 1056},
    {0x00000100, 1057},
    {0x00000140, 1059},
    {0x00200000, 1060},
    {0x00200040, 1062},
    {0x00200100, 1063},
    {0x00200140, 1065},
    {0x00800000, 1066},
    {0x00800040, 1068},
    {0x00800100, 1069},
    {0x00800140, 1071},
    {0x00a00000, 1072},
    {0x00a00040, 1073},
    {0x00a00100, 1074},
    {0x00a00140, 1075},
    {0x00000000, 1077},
    {0x00000100, 1078},
    {0x00800000, 1079},
    {0x00800100, 1080},
    {0x00000100, 1083},
    {0x00000140, 1085},
    {0x00200100, 1086},
    {0x00200140, 1088},
    {0x00800100, 1089},
    {0x00800140, 1090},
    {0x00a00100, 1091},
    {0x00a00140, 1092},
    {0x00000000, 1093},
    {0x00040000, 1097},
    {0x00060000, 1102},
    {0x00080000, 1106},
    {0x000a0000, 1110},
    {0x000c0000, 1113},
    {0x000e0000, 1120},
    {0x00000080, 1094},
    {0x00010000, 1095},
    {0x00010080, 1096},
    {0x00000000, 1098},
    {0x00000080, 1099},
    {0x00010000, 1100},
    {0x00010080, 1101},
    {0x00000000, 1103},
    {0x00000080, 1104},
    {0x00010000, 1105},
    {0x00000000, 1107},
    {0x00010000, 1109},
    {0x00000000, 1114},
    {0x00010000, 1117},
    {0x00000000, 1125},
    {0x00000040, 1127},
    {0x00000100, 1128},
    {0x00000140, 1130},
    {0x00200000, 1131},
    {0x00200040, 1133},
    {0x00200100, 1134},
    {0x00200140, 1136},
    {0x00800000, 1137},
    {0x00800040, 1138},
    {0x00800100, 1139},
    {0x00800140, 1140},
    {0x00a00000, 1141},
    {0x00a00040, 1142},
    {0x00a00100, 1182},
    {0x00a00140, 1183},
    {0x00000000, 1143},
    {0x00020000, 1150},
    {0x00040000, 1155},
    {0x00060000, 1160},
    {0x00080000, 1165},
    {0x000a0000, 1169},
    {0x000c0000, 1172},
    {0x000e0000, 1179},
    {0x00000000, 1144},
    {0x00000080, 1146},
    {0x00010000, 1148},
    {0x00010080, 1149},
    {0x00000000, 1151},
    {0x00000080, 1152},
    {0x00010000, 1153},
    {0x00010080, 1154},
    {0x00000000, 1156},
    {0x00000080, 1157},
    {0x00010000, 1158},
    {0x00010080, 1159},
    {0x00000000, 1161},
    {0x00000080, 1162},
    {0x00010000, 1163},
    {0x00010080, 1164},
    {0x00000000, 1166},
    {0x00010000, 1168},
    {0x00000000, 1173},
    {0x00010000, 1176},
    {0x00000000, 1184},
    {0x00020000, 1189},
    {0x00040000, 1194},
    {0x00060000, 1199},
    {0x00080000, 1204},
    {0x000a0000, 1208},
    {0x000c0000, 1211},
    {0x000e0000, 1218},
    {0x00000000, 1185},
    {0x00000080, 1186},
    {0x00010000, 1187},
    {0x00010080, 1188},
    {0x00000000, 1190},
    {0x00000080, 1191},
    {0x00010000, 1192},
    {0x00010080, 1193},
    {0x00000000, 1195},
    {0x00000080, 1196},
    {0x00010000, 1197},
    {0x00010080, 1198},
    {0x00000000, 1200},
    {0x00000080, 1201},
    {0x00010000, 1202},
    {0x00010080, 1203},
    {0x00000000, 1205},
    {0x00010000, 1207},
    {0x00000000, 1212},
    {0x00010000, 1215},
    {0x00000000, 1222},
    {0x00000100, 1225},
    {0x00000000, 1223},
    {0x00e00000, 1224},
};

static const char* bit_rep[] = {
    "0000", "0001", "0010", "0011", "0100", "0101", "0110", "0111",
    "1000", "1001", "1010", "1011", "1100", "1101", "1110", "1111"
};

//
// Static functions for use in this file only.
//
#define DEBUG_AARCH32_DECODE 1

#if DEBUG_AARCH32_DECODE
static void print_bin(FILE* fp, uint32_t insn)
{
    fprintf(fp, "%s %s %s %s %s %s %s %s",
           bit_rep[(insn >> 0x1c) & 0xf], bit_rep[(insn >> 0x18) & 0xf],
           bit_rep[(insn >> 0x14) & 0xf], bit_rep[(insn >> 0x10) & 0xf],
           bit_rep[(insn >> 0x0c) & 0xf], bit_rep[(insn >> 0x08) & 0xf],
           bit_rep[(insn >> 0x04) & 0xf], bit_rep[(insn >> 0x00) & 0xf]);
}

static void identify_trace(uint32_t rawInsn)
{
    int idx = 0, len, i;

    fprintf(stderr, "===========================\n");
    fprintf(stderr, "Begin decode for %08x:\n", rawInsn);
    while (maskTable[idx].mask != 0x0) {
        uint32_t mask  = maskTable[idx].mask;
        uint32_t value = rawInsn & mask;

        print_bin(stderr, rawInsn);
        fprintf(stderr, ": raw instruction\n");
        print_bin(stderr, maskTable[idx].mask);
        fprintf(stderr, ": Mask table -- entry #%d\n", idx);

        len = maskTable[idx].length;
        idx = maskTable[idx].valueIdx;

        for (i = 0; i < len; ++i) {
            print_bin(stderr, valueTable[idx + i].value);
            if (value == valueTable[idx + i].value) {
                fprintf(stderr, ": Match at value #%d\n", i);
                idx = valueTable[idx + i].maskIdx;
                break;
            }
            fprintf(stderr, ": Value #%d does not match\n", i);
        }
        if (i >= len) {
            fprintf(stderr, "Decode failure: No value found for mask.\n");
            return;
        }
    }

    len = maskTable[idx].length;
    for (i = 0; i < len; ++i) {
        int insnIdx = maskTable[idx + i].valueIdx;
        uint32_t encoding = rawInsn & insnTable[insnIdx].mask;

        print_bin(stderr, insnTable[insnIdx].mask);
        fprintf(stderr, ": %s mask\n", insnTable[insnIdx].mnemonic);
        print_bin(stderr, insnTable[insnIdx].code);
        fprintf(stderr, ": %s encoding\n", insnTable[insnIdx].mnemonic);
        print_bin(stderr, encoding);
        if (encoding == insnTable[insnIdx].code) {
            idx = insnIdx;
            fprintf(stderr, ": Matched candidate #%d (insnTable[%d]).\n",
                    i, insnIdx);
            break;
        }
        fprintf(stderr, ": Candidate #%d (insnTable[%d]) does not match.\n",
                i, insnIdx);
    }
    if (i >= len)
        fprintf(stderr, "Decode failure: No instruction matched.\n");
}
#endif

static Insn_Entry_t* identify(uint32_t rawInsn)
{
    int idx = 0, len, i;

    while (maskTable[idx].mask != 0x0) {
        uint32_t value = rawInsn & maskTable[idx].mask;
        len = maskTable[idx].length;
        idx = maskTable[idx].valueIdx;

        for (i = 0; i < len; ++i) {
            if (value == valueTable[idx + i].value) {
                idx = valueTable[idx + i].maskIdx;
                break;
            }
        }
        if (i >= len) {
#if DEBUG_AARCH32_DECODE
            identify_trace(rawInsn);
#endif
            return INVALID_INSN;
        }
    }

    len = maskTable[idx].length;
    for (i = 0; i < len; ++i) {
        int insnIdx = maskTable[idx + i].valueIdx;
        uint32_t encoding = rawInsn & insnTable[insnIdx].mask;
        if (encoding == insnTable[insnIdx].code) {
            idx = insnIdx;
            break;
        }
    }
    if (i >= len) {
#if DEBUG_AARCH32_DECODE
        identify_trace(rawInsn);
#endif
        return INVALID_INSN;
    }

    return insnTable + idx;
}

static int SignExtend(int imm, int signBit)
{
    int mask  = ~0U >> (31 - signBit);
    int value = imm & mask;

    mask = 1 << (signBit - 1);
    if (imm & mask)
        value |= ~0U << signBit;

    return value;
}

static int A32ExpandImm(int imm)
{
    int value  = imm & 0xFF;
    int rotate = (imm & 0xF00) >> 7;

    return (value >> rotate) | (value << (32-rotate));
}

template<int highBit, int lowBit>
static int field(unsigned int raw)
{
    return (raw >> lowBit) & ~(~0UL << ((highBit - lowBit) + 1));
}

static Condition_t get_condition_field(uint32_t rawInsn)
{
    return static_cast<Condition_t>(field<31, 28>(rawInsn));
}

//
// Class InstructionDecoder_aarch32 member function implementations.
//

Expression::Ptr InstructionDecoder_aarch32::make_pc_expr()
{
    return makeRegisterExpression(MachRegister(aarch32::pc));
}

void InstructionDecoder_aarch32::handle_branch_target(Expression::Ptr targAddr,
                                                      bool isCall,
                                                      bool isIndirect)
{
    Condition_t condField = get_condition_field(rawInsn);
    bool isConditional = (condField == COND_al || condField == COND_xx);

    decodedInsn->addSuccessor(targAddr,
                              isCall,
                              isIndirect,
                              isConditional,
                              false // isFallthrough
                             );

    if (isCall || isConditional) {
        Expression::Ptr regPC = make_pc_expr();
        Expression::Ptr insnSize = Immediate::makeImmediate(Result(s32, 4));
        decodedInsn->addSuccessor(makeAddExpression(regPC, insnSize, u32),
                                  false, // isCall
                                  false, // isIndirect
                                  false, // isConditional
                                  true   // isFallthrough
                                 );
    }
}

void InstructionDecoder_aarch32::handle_operand(int oprID)
{
    switch ((Operand_t)oprID) {
      case OPR_empty          : return;
      case OPR_imm_const      : return OPRfunc_imm_const();
      case OPR_imm_label      : return OPRfunc_imm_label();
      case OPR_imm_label_2    : return OPRfunc_imm_label_2();
      case OPR_imm_label_3    : return OPRfunc_imm_label_3();
      case OPR_imm_label_4    : return OPRfunc_imm_label_4();
      case OPR_imm_label_5    : return OPRfunc_imm_label_5();
      case OPR_imm_label_6    : return OPRfunc_imm_label_6();
      case OPR_imm_label_7    : return OPRfunc_imm_label_7();
      case OPR_reg_Dd         : return OPRfunc_reg_Dd();
      case OPR_reg_Dd_x_      : return OPRfunc_reg_Dd_x_();
      case OPR_reg_Ddm        : return OPRfunc_reg_Ddm();
      case OPR_reg_Dm         : return OPRfunc_reg_Dm();
      case OPR_reg_Dm_x_      : return OPRfunc_reg_Dm_x_();
      case OPR_reg_Dn         : return OPRfunc_reg_Dn();
      case OPR_reg_Dn_x_      : return OPRfunc_reg_Dn_x_();
      case OPR_reg_Qd         : return OPRfunc_reg_Qd();
      case OPR_reg_Qd_2       : return OPRfunc_reg_Qd_2();
      case OPR_reg_Qm         : return OPRfunc_reg_Qm();
      case OPR_reg_Qn         : return OPRfunc_reg_Qn();
      case OPR_reg_Ra         : return OPRfunc_reg_Ra();
      case OPR_reg_Rd         : return OPRfunc_reg_Rd();
      case OPR_reg_RdHi       : return OPRfunc_reg_RdHi();
      case OPR_reg_RdLo       : return OPRfunc_reg_RdLo();
      case OPR_reg_Rd_2       : return OPRfunc_reg_Rd_2();
      case OPR_reg_Rm         : return OPRfunc_reg_Rm();
      case OPR_reg_Rm_2       : return OPRfunc_reg_Rm_2();
      case OPR_reg_Rn         : return OPRfunc_reg_Rn();
      case OPR_reg_Rn_2       : return OPRfunc_reg_Rn_2();
      case OPR_reg_Rn_3       : return OPRfunc_reg_Rn_3();
      case OPR_reg_Rs         : return OPRfunc_reg_Rs();
      case OPR_reg_Rt         : return OPRfunc_reg_Rt();
      case OPR_reg_Rt2        : return OPRfunc_reg_Rt2();
      case OPR_reg_Rt2_2      : return OPRfunc_reg_Rt2_2();
      case OPR_reg_Rt2_3      : return OPRfunc_reg_Rt2_3();
      case OPR_reg_Rt_2       : return OPRfunc_reg_Rt_2();
      case OPR_reg_Sd         : return OPRfunc_reg_Sd();
      case OPR_reg_Sdm        : return OPRfunc_reg_Sdm();
      case OPR_reg_Sm         : return OPRfunc_reg_Sm();
      case OPR_reg_Sm1        : return OPRfunc_reg_Sm1();
      case OPR_reg_Sn         : return OPRfunc_reg_Sn();
      case OPR_reglist        : return OPRfunc_reglist();
      case OPR_reglist_2      : return OPRfunc_reglist_2();
      case OPR_reglist_3      : return OPRfunc_reglist_3();
      default                 : assert(0);
    }
}

void InstructionDecoder_aarch32::OPRfunc_imm_const()
{
    int imm = (
      (field<11,  0>(rawInsn))
    );
    imm = A32ExpandImm(imm);
    Expression::Ptr immExpr  = Immediate::makeImmediate(Result(s32, imm));
    decodedInsn->appendOperand(immExpr, true, false);
}

void InstructionDecoder_aarch32::OPRfunc_imm_label()
{
    int imm = (
      (field<11,  0>(rawInsn))
    );
    imm = A32ExpandImm(imm);
    Expression::Ptr regPC   = make_pc_expr();
    Expression::Ptr offset  = Immediate::makeImmediate(Result(s32, imm));
    Expression::Ptr memAddr = makeAddExpression(regPC, offset, u32);
    decodedInsn->appendOperand(memAddr, true, false);
}

void InstructionDecoder_aarch32::OPRfunc_imm_label_2()
{
    int imm = (
      (field<23,  0>(rawInsn) << 2)
    );
    bool isCall = field<24, 24>(rawInsn);
    imm = SignExtend(imm, 24+2);
    Expression::Ptr regPC    = make_pc_expr();
    Expression::Ptr offset   = Immediate::makeImmediate(Result(s32, imm));
    Expression::Ptr targAddr = makeAddExpression(regPC, offset, u32);
    handle_branch_target(targAddr, isCall, false);
}

void InstructionDecoder_aarch32::OPRfunc_imm_label_3()
{
    int imm = (
        (field<24, 24>(rawInsn) << 1)
      | (field<23,  0>(rawInsn) << 2)
    );
    bool isCall = true;
    imm = SignExtend(imm, 24+2);
    Expression::Ptr regPC    = make_pc_expr();
    Expression::Ptr offset   = Immediate::makeImmediate(Result(s32, imm));
    Expression::Ptr targAddr = makeAddExpression(regPC, offset, u32);
    handle_branch_target(targAddr, isCall, false);
}

void InstructionDecoder_aarch32::OPRfunc_imm_label_4()
{
    int imm = (
      (field< 7,  0>(rawInsn) << 2)
    );
    // imm = ZeroExtend(imm, 8+2);
}

void InstructionDecoder_aarch32::OPRfunc_imm_label_5()
{
    int imm = (
      (field<11,  0>(rawInsn))
    );
    // imm = ZeroExtend(imm, 12);
    Expression::Ptr regPC   = make_pc_expr();
    Expression::Ptr offset  = Immediate::makeImmediate(Result(s32, imm));
    Expression::Ptr memAddr = makeAddExpression(regPC, offset, u32);
    decodedInsn->appendOperand(memAddr, true, false);
}

void InstructionDecoder_aarch32::OPRfunc_imm_label_6()
{
    int imm = (
        (field< 3,  0>(rawInsn))
      | (field<11,  8>(rawInsn) << 4)
    );
    // imm = ZeroExtend(imm, 8);
}

void InstructionDecoder_aarch32::OPRfunc_imm_label_7()
{
    int imm = (
      (field< 7,  0>(rawInsn) << 2)
    );
    int size = (
      (field< 9,  8>(rawInsn))
    );
    if (size == 0x1) {
        imm >>= 1;
    }
    Expression::Ptr regPC   = make_pc_expr();
    Expression::Ptr offset  = Immediate::makeImmediate(Result(s32, imm));
    Expression::Ptr memAddr = makeAddExpression(regPC, offset, u32);
    decodedInsn->appendOperand(memAddr, true, false);
}

void InstructionDecoder_aarch32::OPRfunc_reg_Dd()
{
    int base = aarch32::d0.val();
    int idx = (
        (field<15, 12>(rawInsn))
      | (field<22, 22>(rawInsn) << 4)
    );

    Expression::Ptr regExpr = makeRegisterExpression(MachRegister(base + idx));
    decodedInsn->appendOperand(regExpr, false, true);
}

void InstructionDecoder_aarch32::OPRfunc_reg_Dd_x_()
{
    int base = aarch32::d0.val();
    int idx = (
        (field<19, 16>(rawInsn))
      | (field< 7,  7>(rawInsn) << 4)
    );

    Expression::Ptr regExpr = makeRegisterExpression(MachRegister(base + idx));
    decodedInsn->appendOperand(regExpr, true, true);
}

void InstructionDecoder_aarch32::OPRfunc_reg_Ddm()
{
    int base = aarch32::d0.val();
    int idx = (
        (field<15, 12>(rawInsn))
      | (field<22, 22>(rawInsn) << 4)
    );

    Expression::Ptr regExpr = makeRegisterExpression(MachRegister(base + idx));
    decodedInsn->appendOperand(regExpr, true, true);
}

void InstructionDecoder_aarch32::OPRfunc_reg_Dm()
{
    int base = aarch32::d0.val();
    int idx = (
        (field< 3,  0>(rawInsn))
      | (field< 5,  5>(rawInsn) << 4)
    );

    Expression::Ptr regExpr = makeRegisterExpression(MachRegister(base + idx));
    decodedInsn->appendOperand(regExpr, true, false);
}

void InstructionDecoder_aarch32::OPRfunc_reg_Dm_x_()
{
    int base = aarch32::d0.val();
    int idx = (
        (field< 3,  0>(rawInsn))
      | (field< 5,  5>(rawInsn) << 4)
    );

    Expression::Ptr regExpr = makeRegisterExpression(MachRegister(base + idx));
    decodedInsn->appendOperand(regExpr, true, false);
}

void InstructionDecoder_aarch32::OPRfunc_reg_Dn()
{
    int base = aarch32::d0.val();
    int idx = (
        (field<19, 16>(rawInsn))
      | (field< 7,  7>(rawInsn) << 4)
    );

    Expression::Ptr regExpr = makeRegisterExpression(MachRegister(base + idx));
    decodedInsn->appendOperand(regExpr, true, false);
}

void InstructionDecoder_aarch32::OPRfunc_reg_Dn_x_()
{
    int base = aarch32::d0.val();
    int idx = (
        (field<19, 16>(rawInsn))
      | (field< 7,  7>(rawInsn) << 4)
    );

    Expression::Ptr regExpr = makeRegisterExpression(MachRegister(base + idx));
    decodedInsn->appendOperand(regExpr, true, false);
}

void InstructionDecoder_aarch32::OPRfunc_reg_Qd()
{
    int base = aarch32::q0.val();
    int idx = (
        (field<15, 12>(rawInsn))
      | (field<22, 22>(rawInsn) << 4)
    );

    Expression::Ptr regExpr = makeRegisterExpression(MachRegister(base + idx));
    decodedInsn->appendOperand(regExpr, false, true);
}

void InstructionDecoder_aarch32::OPRfunc_reg_Qd_2()
{
    int base = aarch32::q0.val();
    int idx = (
        (field<19, 16>(rawInsn))
      | (field< 7,  7>(rawInsn) << 4)
    );

    Expression::Ptr regExpr = makeRegisterExpression(MachRegister(base + idx));
    decodedInsn->appendOperand(regExpr, false, true);
}

void InstructionDecoder_aarch32::OPRfunc_reg_Qm()
{
    int base = aarch32::q0.val();
    int idx = (
        (field< 3,  0>(rawInsn))
      | (field< 5,  5>(rawInsn) << 4)
    );

    Expression::Ptr regExpr = makeRegisterExpression(MachRegister(base + idx));
    decodedInsn->appendOperand(regExpr, true, false);
}

void InstructionDecoder_aarch32::OPRfunc_reg_Qn()
{
    int base = aarch32::q0.val();
    int idx = (
        (field<19, 16>(rawInsn))
      | (field< 7,  7>(rawInsn) << 4)
    );

    Expression::Ptr regExpr = makeRegisterExpression(MachRegister(base + idx));
    decodedInsn->appendOperand(regExpr, true, false);
}

void InstructionDecoder_aarch32::OPRfunc_reg_Ra()
{
    int base = aarch32::r0.val();
    int idx = (
      (field<15, 12>(rawInsn))
    );

    Expression::Ptr regExpr = makeRegisterExpression(MachRegister(base + idx));
    decodedInsn->appendOperand(regExpr, true, false);
}

void InstructionDecoder_aarch32::OPRfunc_reg_Rd()
{
    int base = aarch32::r0.val();
    int idx = (
      (field<15, 12>(rawInsn))
    );

    Expression::Ptr regExpr = makeRegisterExpression(MachRegister(base + idx));
    decodedInsn->appendOperand(regExpr, false, true);
}

void InstructionDecoder_aarch32::OPRfunc_reg_RdHi()
{
    int base = aarch32::r0.val();
    int idx = (
      (field<19, 16>(rawInsn))
    );

    Expression::Ptr regExpr = makeRegisterExpression(MachRegister(base + idx));
    decodedInsn->appendOperand(regExpr, true, true);
}

void InstructionDecoder_aarch32::OPRfunc_reg_RdLo()
{
    int base = aarch32::r0.val();
    int idx = (
      (field<15, 12>(rawInsn))
    );

    Expression::Ptr regExpr = makeRegisterExpression(MachRegister(base + idx));
    decodedInsn->appendOperand(regExpr, true, true);
}

void InstructionDecoder_aarch32::OPRfunc_reg_Rd_2()
{
    int base = aarch32::r0.val();
    int idx = (
      (field<19, 16>(rawInsn))
    );

    Expression::Ptr regExpr = makeRegisterExpression(MachRegister(base + idx));
    decodedInsn->appendOperand(regExpr, false, true);
}

void InstructionDecoder_aarch32::OPRfunc_reg_Rm()
{
    int base = aarch32::r0.val();
    int idx = (
      (field< 3,  0>(rawInsn))
    );

    Expression::Ptr regExpr = makeRegisterExpression(MachRegister(base + idx));
    decodedInsn->appendOperand(regExpr, true, false);
}

void InstructionDecoder_aarch32::OPRfunc_reg_Rm_2()
{
    int base = aarch32::r0.val();
    int idx = (
      (field<11,  8>(rawInsn))
    );

    Expression::Ptr regExpr = makeRegisterExpression(MachRegister(base + idx));
    decodedInsn->appendOperand(regExpr, true, false);
}

void InstructionDecoder_aarch32::OPRfunc_reg_Rn()
{
    int base = aarch32::r0.val();
    int idx = (
      (field<19, 16>(rawInsn))
    );

    Expression::Ptr regExpr = makeRegisterExpression(MachRegister(base + idx));
    decodedInsn->appendOperand(regExpr, true, false);
}

void InstructionDecoder_aarch32::OPRfunc_reg_Rn_2()
{
    int base = aarch32::r0.val();
    int idx = (
      (field< 3,  0>(rawInsn))
    );

    Expression::Ptr regExpr = makeRegisterExpression(MachRegister(base + idx));
    decodedInsn->appendOperand(regExpr, true, false);
}

void InstructionDecoder_aarch32::OPRfunc_reg_Rn_3()
{
    int base = aarch32::r0.val();
    int idx = (
      (field<19, 16>(rawInsn))
    );

    Expression::Ptr regExpr = makeRegisterExpression(MachRegister(base + idx));
    decodedInsn->appendOperand(regExpr, true, true);
}

void InstructionDecoder_aarch32::OPRfunc_reg_Rs()
{
    int base = aarch32::r0.val();
    int idx = (
      (field<11,  8>(rawInsn))
    );

    Expression::Ptr regExpr = makeRegisterExpression(MachRegister(base + idx));
    decodedInsn->appendOperand(regExpr, true, false);
}

void InstructionDecoder_aarch32::OPRfunc_reg_Rt()
{
    int base = aarch32::r0.val();
    int idx = (
      (field<15, 12>(rawInsn))
    );

    Expression::Ptr regExpr = makeRegisterExpression(MachRegister(base + idx));
    decodedInsn->appendOperand(regExpr, false, true);
}

void InstructionDecoder_aarch32::OPRfunc_reg_Rt2()
{
    int base = aarch32::r0.val();
    int idx = (
      (field<15, 12>(rawInsn))
    );

    Expression::Ptr regExpr = makeRegisterExpression(MachRegister(base + idx));
    decodedInsn->appendOperand(regExpr, false, true);
}

void InstructionDecoder_aarch32::OPRfunc_reg_Rt2_2()
{
    int base = aarch32::r0.val();
    int idx = (
      (field<19, 16>(rawInsn))
    );

    Expression::Ptr regExpr = makeRegisterExpression(MachRegister(base + idx));
    decodedInsn->appendOperand(regExpr, false, true);
}

void InstructionDecoder_aarch32::OPRfunc_reg_Rt2_3()
{
    int base = aarch32::r0.val();
    int idx = (
      (field< 3,  0>(rawInsn))
    );

    Expression::Ptr regExpr = makeRegisterExpression(MachRegister(base + idx));
    decodedInsn->appendOperand(regExpr, false, true);
}

void InstructionDecoder_aarch32::OPRfunc_reg_Rt_2()
{
    int base = aarch32::r0.val();
    int idx = (
      (field< 3,  0>(rawInsn))
    );

    Expression::Ptr regExpr = makeRegisterExpression(MachRegister(base + idx));
    decodedInsn->appendOperand(regExpr, false, true);
}

void InstructionDecoder_aarch32::OPRfunc_reg_Sd()
{
    int base = aarch32::s0.val();
    int idx = (
        (field<22, 22>(rawInsn))
      | (field<15, 12>(rawInsn) << 1)
    );

    Expression::Ptr regExpr = makeRegisterExpression(MachRegister(base + idx));
    decodedInsn->appendOperand(regExpr, false, true);
}

void InstructionDecoder_aarch32::OPRfunc_reg_Sdm()
{
    int base = aarch32::s0.val();
    int idx = (
        (field<22, 22>(rawInsn))
      | (field<15, 12>(rawInsn) << 1)
    );

    Expression::Ptr regExpr = makeRegisterExpression(MachRegister(base + idx));
    decodedInsn->appendOperand(regExpr, true, true);
}

void InstructionDecoder_aarch32::OPRfunc_reg_Sm()
{
    int base = aarch32::s0.val();
    int idx = (
        (field< 5,  5>(rawInsn))
      | (field< 3,  0>(rawInsn) << 1)
    );

    Expression::Ptr regExpr = makeRegisterExpression(MachRegister(base + idx));
    decodedInsn->appendOperand(regExpr, true, false);
}

void InstructionDecoder_aarch32::OPRfunc_reg_Sm1()
{
    int base = aarch32::s0.val();
    int idx = (
        (field< 5,  5>(rawInsn))
      | (field< 3,  0>(rawInsn) << 1)
    );

    Expression::Ptr regExpr = makeRegisterExpression(MachRegister(base + idx + 1));
    decodedInsn->appendOperand(regExpr, true, false);
}

void InstructionDecoder_aarch32::OPRfunc_reg_Sn()
{
    int base = aarch32::s0.val();
    int idx = (
        (field< 7,  7>(rawInsn))
      | (field<19, 16>(rawInsn) << 1)
    );

    Expression::Ptr regExpr = makeRegisterExpression(MachRegister(base + idx));
    decodedInsn->appendOperand(regExpr, true, false);
}

void InstructionDecoder_aarch32::OPRfunc_reglist()
{
    int regList = (
      (field<15,  0>(rawInsn))
    );
    int idx = (
      (field<19, 16>(rawInsn))
    );

    /*
    MachRegister baseReg(aarch32::r0.val() + idx);
    int off = 0;
    for (int i = 0; i < 16; ++i) {
        int mask = 0x1 << i;
        if ( !(regList & mask))
            continue;

        Expression::Ptr regExpr   = makeRegisterExpression(baseReg);
        Expression::Ptr offExpr   = Immediate::makeImmediate(Result(s32, off));
        Expression::Ptr addrExpr  = makeAddExpression(regExpr, offExpr, u32);
        Expression::Ptr derefExpr = makeDereferenceExpression(addrExpr, s32);
        decodedInsn->appendOperand(derefExpr, !false, !true);
        off += 4;
    }
    */

    for (int i = 0; i < 16; ++i) {
        int mask = 0x1 << i;
        if ( !(regList & mask))
            continue;

        MachRegister regID(aarch32::r0.val() + i);
        Expression::Ptr listRegExpr = makeRegisterExpression(regID);
        decodedInsn->appendOperand(listRegExpr, false, true);
    }
}

void InstructionDecoder_aarch32::OPRfunc_reglist_2()
{
    int regList = (
      (field<14,  0>(rawInsn))
    );
    int idx = (
      (field<19, 16>(rawInsn))
    );

    /*
    MachRegister baseReg(aarch32::r0.val() + idx);
    int off = 0;
    for (int i = 0; i < 15; ++i) {
        int mask = 0x1 << i;
        if ( !(regList & mask))
            continue;

        Expression::Ptr regExpr   = makeRegisterExpression(baseReg);
        Expression::Ptr offExpr   = Immediate::makeImmediate(Result(s32, off));
        Expression::Ptr addrExpr  = makeAddExpression(regExpr, offExpr, u32);
        Expression::Ptr derefExpr = makeDereferenceExpression(addrExpr, s32);
        decodedInsn->appendOperand(derefExpr, !false, !true);
        off += 4;
    }
    */

    for (int i = 0; i < 15; ++i) {
        int mask = 0x1 << i;
        if ( !(regList & mask))
            continue;

        MachRegister regID(aarch32::r0.val() + i);
        Expression::Ptr listRegExpr = makeRegisterExpression(regID);
        decodedInsn->appendOperand(listRegExpr, false, true);
    }
}

void InstructionDecoder_aarch32::OPRfunc_reglist_3()
{
    int regList = (
      (field<15,  0>(rawInsn))
    );
    int idx = (
      (field<19, 16>(rawInsn))
    );

    /*
    MachRegister baseReg(aarch32::r0.val() + idx);
    int off = 0;
    for (int i = 0; i < 16; ++i) {
        int mask = 0x1 << i;
        if ( !(regList & mask))
            continue;

        Expression::Ptr regExpr   = makeRegisterExpression(baseReg);
        Expression::Ptr offExpr   = Immediate::makeImmediate(Result(s32, off));
        Expression::Ptr addrExpr  = makeAddExpression(regExpr, offExpr, u32);
        Expression::Ptr derefExpr = makeDereferenceExpression(addrExpr, s32);
        decodedInsn->appendOperand(derefExpr, !true, !false);
        off += 4;
    }
    */

    for (int i = 0; i < 16; ++i) {
        int mask = 0x1 << i;
        if ( !(regList & mask))
            continue;

        MachRegister regID(aarch32::r0.val() + i);
        Expression::Ptr listRegExpr = makeRegisterExpression(regID);
        decodedInsn->appendOperand(listRegExpr, true, false);
    }
}

}} // End namespace Dyninst::InstructionAPI
