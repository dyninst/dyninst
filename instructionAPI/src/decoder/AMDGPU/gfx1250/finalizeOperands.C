#include "registers/AMDGPU/amdgpu_gfx1250_regs.h"
#include "InstructionDecoder-amdgpu-gfx1250.h"

namespace Dyninst {
namespace InstructionAPI {
    void InstructionDecoder_amdgpu_gfx1250::finalizeENC_SMEMOperands()
    {
        layout_ENC_SMEM & layout = insn_layout.ENC_SMEM;
        switch (layout.OP)  {
            case 0:  case 8:  case 9:  case 10:  case 11:   // S_LOAD_B32,S_LOAD_I8,S_LOAD_U8,S_LOAD_I16,S_LOAD_U16,
                appendOPR_SREG(layout.SDATA,false,true);
                appendOPR_SREG(layout.SBASE,true,false,2);
                processOPR_SMEM_OFFSET(layout);
                appendOPR_GPUMEM(0,true,false,1,true);
                break;
            case 1:   // S_LOAD_B64,
                appendOPR_SREG(layout.SDATA,false,true,2);
                appendOPR_SREG(layout.SBASE,true,false,2);
                processOPR_SMEM_OFFSET(layout);
                appendOPR_GPUMEM(0,true,false,2,true);
                break;
            case 2:   // S_LOAD_B128,
                appendOPR_SREG(layout.SDATA,false,true,4);
                appendOPR_SREG(layout.SBASE,true,false,2);
                processOPR_SMEM_OFFSET(layout);
                appendOPR_GPUMEM(0,true,false,4,true);
                break;
            case 3:   // S_LOAD_B256,
                appendOPR_SREG(layout.SDATA,false,true,8);
                appendOPR_SREG(layout.SBASE,true,false,2);
                processOPR_SMEM_OFFSET(layout);
                appendOPR_GPUMEM(0,true,false,8,true);
                break;
            case 4:   // S_LOAD_B512,
                appendOPR_SREG(layout.SDATA,false,true,16);
                appendOPR_SREG(layout.SBASE,true,false,2);
                processOPR_SMEM_OFFSET(layout);
                appendOPR_GPUMEM(0,true,false,16,true);
                break;
            case 5:   // S_LOAD_B96,
                appendOPR_SREG(layout.SDATA,false,true,3);
                appendOPR_SREG(layout.SBASE,true,false,2);
                processOPR_SMEM_OFFSET(layout);
                appendOPR_GPUMEM(0,true,false,3,true);
                break;
            case 16:  case 24:  case 25:  case 26:   // S_BUFFER_LOAD_B32,S_BUFFER_LOAD_I8,S_BUFFER_LOAD_U8,S_BUFFER_LOAD_I16,
            case 27:   // S_BUFFER_LOAD_U16,
                appendOPR_SREG(layout.SDATA,false,true);
                appendOPR_SREG(layout.SBASE,true,false,4);
                processOPR_SMEM_OFFSET(layout);
                appendOPR_GPUMEM(0,true,false,1,true);
                break;
            case 17:   // S_BUFFER_LOAD_B64,
                appendOPR_SREG(layout.SDATA,false,true,2);
                appendOPR_SREG(layout.SBASE,true,false,4);
                processOPR_SMEM_OFFSET(layout);
                appendOPR_GPUMEM(0,true,false,2,true);
                break;
            case 18:   // S_BUFFER_LOAD_B128,
                appendOPR_SREG(layout.SDATA,false,true,4);
                appendOPR_SREG(layout.SBASE,true,false,4);
                processOPR_SMEM_OFFSET(layout);
                appendOPR_GPUMEM(0,true,false,4,true);
                break;
            case 19:   // S_BUFFER_LOAD_B256,
                appendOPR_SREG(layout.SDATA,false,true,8);
                appendOPR_SREG(layout.SBASE,true,false,4);
                processOPR_SMEM_OFFSET(layout);
                appendOPR_GPUMEM(0,true,false,8,true);
                break;
            case 20:   // S_BUFFER_LOAD_B512,
                appendOPR_SREG(layout.SDATA,false,true,16);
                appendOPR_SREG(layout.SBASE,true,false,4);
                processOPR_SMEM_OFFSET(layout);
                appendOPR_GPUMEM(0,true,false,16,true);
                break;
            case 21:   // S_BUFFER_LOAD_B96,
                appendOPR_SREG(layout.SDATA,false,true,3);
                appendOPR_SREG(layout.SBASE,true,false,4);
                processOPR_SMEM_OFFSET(layout);
                appendOPR_GPUMEM(0,true,false,3,true);
                break;
            case 33:   // S_DCACHE_INV,
                break;
            case 34:   // S_ATC_PROBE,
                appendOPR_SIMM8(layout.SDATA,true,false);
                appendOPR_SREG(layout.SBASE,true,false,2);
                processOPR_SMEM_OFFSET(layout);
                break;
            case 35:   // S_ATC_PROBE_BUFFER,
                appendOPR_SIMM8(layout.SDATA,true,false);
                appendOPR_SREG(layout.SBASE,true,false,4);
                processOPR_SMEM_OFFSET(layout);
                break;
            case 36:  case 38:   // S_PREFETCH_INST,S_PREFETCH_DATA,
                appendOPR_SREG(layout.SBASE,true,false,2);
                appendOPR_SIMM24(layout.IOFFSET,true,false);
                appendOPR_SMEM_OFFSET_NOK(layout.SOFFSET,true,false);
                appendOPR_SIMM5(layout.SDATA,true,false);
                break;
            case 37:  case 40:   // S_PREFETCH_INST_PC_REL,S_PREFETCH_DATA_PC_REL,
                appendOPR_SIMM24(layout.IOFFSET,true,false);
                appendOPR_SMEM_OFFSET_NOK(layout.SOFFSET,true,false);
                appendOPR_SIMM5(layout.SDATA,true,false);
                break;
            case 39:   // S_BUFFER_PREFETCH_DATA,
                appendOPR_SREG(layout.SBASE,true,false,4);
                appendOPR_SIMM24(layout.IOFFSET,true,false);
                appendOPR_SMEM_OFFSET_NOK(layout.SOFFSET,true,false);
                appendOPR_SIMM5(layout.SDATA,true,false);
                break;
        }
    }

    void InstructionDecoder_amdgpu_gfx1250::finalizeENC_SOP1Operands()
    {
        layout_ENC_SOP1 & layout = insn_layout.ENC_SOP1;
        switch (layout.OP)  {
            case 0:  case 4:  case 8:  case 10:  case 12:   // S_MOV_B32,S_BREV_B32,S_CTZ_I32_B32,S_CLZ_I32_U32,S_CLS_I32,
            case 14:  case 15:  case 16:  case 18:  case 96:   // S_SEXT_I32_I8,S_SEXT_I32_I16,S_BITSET0_B32,S_BITSET1_B32,S_CEIL_F32,
            case 97:  case 98:  case 99:  case 100:  case 101:   // S_FLOOR_F32,S_TRUNC_F32,S_RNDNE_F32,S_CVT_F32_I32,S_CVT_F32_U32,
            case 102:  case 103:  case 104:  case 105:   // S_CVT_I32_F32,S_CVT_U32_F32,S_CVT_F16_F32,S_CVT_F32_F16,
            case 106:  case 107:  case 108:  case 109:   // S_CVT_HI_F32_F16,S_CEIL_F16,S_FLOOR_F16,S_TRUNC_F16,
            case 110:   // S_RNDNE_F16,
                appendOPR_SDST(layout.SDST,false,true);
                appendOPR_SSRC(layout.SSRC0,true,false);
                break;
            case 1:  case 5:   // S_MOV_B64,S_BREV_B64,
                appendOPR_SDST(layout.SDST,false,true,2);
                appendOPR_SSRC(layout.SSRC0,true,false,2);
                break;
            case 2:   // S_CMOV_B32,
                appendOPR_SDST(layout.SDST,false,true);
                appendOPR_SSRC(layout.SSRC0,true,false);
                appendOPR_SSRC_SPECIAL_SCC(253,true,false,1,true);
                break;
            case 3:   // S_CMOV_B64,
                appendOPR_SDST(layout.SDST,false,true,2);
                appendOPR_SSRC(layout.SSRC0,true,false,2);
                appendOPR_SSRC_SPECIAL_SCC(253,true,false,1,true);
                break;
            case 6:   // S_GET_SHADER_CYCLES_U64,
                appendOPR_SDST(layout.SDST,false,true,2);
                break;
            case 9:  case 11:  case 13:   // S_CTZ_I32_B64,S_CLZ_I32_U64,S_CLS_I32_I64,
                appendOPR_SDST(layout.SDST,false,true);
                appendOPR_SSRC(layout.SSRC0,true,false,2);
                break;
            case 17:  case 19:  case 20:   // S_BITSET0_B64,S_BITSET1_B64,S_BITREPLICATE_B64_B32,
                appendOPR_SDST(layout.SDST,false,true,2);
                appendOPR_SSRC(layout.SSRC0,true,false);
                break;
            case 21:  case 22:  case 24:  case 26:  case 28:   // S_ABS_I32,S_BCNT0_I32_B32,S_BCNT1_I32_B32,S_QUADMASK_B32,S_WQM_B32,
            case 30:   // S_NOT_B32,
                appendOPR_SDST(layout.SDST,false,true);
                appendOPR_SSRC(layout.SSRC0,true,false);
                appendOPR_SSRC_SPECIAL_SCC(253,false,true,1,true);
                break;
            case 23:  case 25:   // S_BCNT0_I32_B64,S_BCNT1_I32_B64,
                appendOPR_SDST(layout.SDST,false,true);
                appendOPR_SSRC(layout.SSRC0,true,false,2);
                appendOPR_SSRC_SPECIAL_SCC(253,false,true,1,true);
                break;
            case 27:  case 29:  case 31:   // S_QUADMASK_B64,S_WQM_B64,S_NOT_B64,
                appendOPR_SDST(layout.SDST,false,true,2);
                appendOPR_SSRC(layout.SSRC0,true,false,2);
                appendOPR_SSRC_SPECIAL_SCC(253,false,true,1,true);
                break;
            case 32:  case 34:  case 36:  case 38:   // S_AND_SAVEEXEC_B32,S_OR_SAVEEXEC_B32,S_XOR_SAVEEXEC_B32,S_NAND_SAVEEXEC_B32,
            case 40:  case 42:  case 44:   // S_NOR_SAVEEXEC_B32,S_XNOR_SAVEEXEC_B32,S_AND_NOT0_SAVEEXEC_B32,
            case 46:  case 48:  case 50:   // S_OR_NOT0_SAVEEXEC_B32,S_AND_NOT1_SAVEEXEC_B32,S_OR_NOT1_SAVEEXEC_B32,
            case 52:  case 54:   // S_AND_NOT0_WREXEC_B32,S_AND_NOT1_WREXEC_B32,
                appendOPR_SREG(layout.SDST,false,true);
                appendOPR_SSRC(layout.SSRC0,true,false);
                appendOPR_SDST_EXEC(126,false,true,1,true);
                appendOPR_SSRC_SPECIAL_SCC(253,false,true,1,true);
                appendOPR_SDST_EXEC(126,true,false,1,true);
                break;
            case 33:  case 35:  case 37:  case 39:   // S_AND_SAVEEXEC_B64,S_OR_SAVEEXEC_B64,S_XOR_SAVEEXEC_B64,S_NAND_SAVEEXEC_B64,
            case 41:  case 43:  case 45:   // S_NOR_SAVEEXEC_B64,S_XNOR_SAVEEXEC_B64,S_AND_NOT0_SAVEEXEC_B64,
            case 47:  case 49:  case 51:   // S_OR_NOT0_SAVEEXEC_B64,S_AND_NOT1_SAVEEXEC_B64,S_OR_NOT1_SAVEEXEC_B64,
            case 53:  case 55:   // S_AND_NOT0_WREXEC_B64,S_AND_NOT1_WREXEC_B64,
                appendOPR_SREG(layout.SDST,false,true,2);
                appendOPR_SSRC(layout.SSRC0,true,false,2);
                appendOPR_SDST_EXEC(126,false,true,1,true);
                appendOPR_SSRC_SPECIAL_SCC(253,false,true,1,true);
                appendOPR_SDST_EXEC(126,true,false,1,true);
                break;
            case 64:   // S_MOVRELS_B32,
                appendOPR_SDST(layout.SDST,false,true);
                appendOPR_SREG(layout.SSRC0,true,false);
                appendOPR_SDST_M0(125,true,false,1,true);
                break;
            case 65:   // S_MOVRELS_B64,
                appendOPR_SDST(layout.SDST,false,true,2);
                appendOPR_SREG(layout.SSRC0,true,false,2);
                appendOPR_SDST_M0(125,true,false,1,true);
                break;
            case 66:   // S_MOVRELD_B32,
                appendOPR_SREG(layout.SDST,false,true);
                appendOPR_SSRC(layout.SSRC0,true,false);
                appendOPR_SDST_M0(125,true,false,1,true);
                break;
            case 67:   // S_MOVRELD_B64,
                appendOPR_SREG(layout.SDST,false,true,2);
                appendOPR_SSRC(layout.SSRC0,true,false,2);
                appendOPR_SDST_M0(125,true,false,1,true);
                break;
            case 68:   // S_MOVRELSD_2_B32,
                appendOPR_SREG(layout.SDST,false,true);
                appendOPR_SREG(layout.SSRC0,true,false);
                appendOPR_SDST_M0(125,true,false,1,true);
                break;
            case 71:   // S_GET_PC_I64,
                appendOPR_SDST(layout.SDST,false,true,2);
                appendOPR_PC(0,true,false,2,true);
                break;
            case 72:   // S_SET_PC_I64,
                setBranch();
                setModifyPC();
                appendOPR_SREG(layout.SSRC0,true,false,2);
                appendOPR_PC(0,false,true,2,true);
                break;
            case 73:   // S_SWAP_PC_I64,
                setBranch();
                setModifyPC();
                appendOPR_SDST(layout.SDST,false,true,2);
                appendOPR_SREG(layout.SSRC0,true,false,2);
                appendOPR_PC(0,false,true,2,true);
                appendOPR_PC(0,true,false,2,true);
                break;
            case 74:   // S_RFE_I64,
                appendOPR_SREG(layout.SSRC0,true,false,2);
                appendOPR_PC(0,false,true,2,true);
                break;
            case 75:   // S_ADD_PC_I64,
                setBranch();
                setModifyPC();
                appendOPR_SSRC(layout.SSRC0,true,false,2);
                appendOPR_PC(0,false,true,2,true);
                break;
            case 76:   // S_SENDMSG_RTN_B32,
                appendOPR_SDST(layout.SDST,false,true);
                appendOPR_SENDMSG_RTN(layout.SSRC0,true,false);
                break;
            case 77:   // S_SENDMSG_RTN_B64,
                appendOPR_SDST(layout.SDST,false,true,2);
                appendOPR_SENDMSG_RTN(layout.SSRC0,true,false);
                break;
            case 78:  case 82:  case 87:   // S_BARRIER_SIGNAL,S_BARRIER_JOIN,S_WAKEUP_BARRIER,
                appendOPR_SSRC_BARRIER_ID(layout.SSRC0,true,false);
                break;
            case 79:   // S_BARRIER_SIGNAL_ISFIRST,
                appendOPR_SSRC_BARRIER_ID(layout.SSRC0,true,false);
                appendOPR_SSRC_SPECIAL_SCC(253,false,true,1,true);
                break;
            case 80:   // S_GET_BARRIER_STATE,
                appendOPR_SREG(layout.SDST,false,true);
                appendOPR_SSRC_BARRIER_ID(layout.SSRC0,true,false);
                break;
            case 81:   // S_BARRIER_INIT,
                appendOPR_SSRC_BARRIER_ID(layout.SSRC0,true,false);
                appendOPR_SDST_M0(125,true,false,1,true);
                break;
            case 83:   // S_ALLOC_VGPR,
                appendOPR_SSRC(layout.SSRC0,true,false);
                appendOPR_SSRC_SPECIAL_SCC(253,false,true,1,true);
                break;
            case 88:   // S_SLEEP_VAR,
                appendOPR_SSRC(layout.SSRC0,true,false);
                break;
        }
    }

    void InstructionDecoder_amdgpu_gfx1250::finalizeSOP1_INST_LITERALOperands()
    {
        layout_SOP1_INST_LITERAL & layout = insn_layout.SOP1_INST_LITERAL;
        switch (layout.OP)  {
            case 0:  case 4:  case 8:  case 9:  case 10:   // S_MOV_B32,S_BREV_B32,S_CTZ_I32_B32,S_CTZ_I32_B64,S_CLZ_I32_U32,
            case 11:  case 12:  case 13:  case 14:  case 15:   // S_CLZ_I32_U64,S_CLS_I32,S_CLS_I32_I64,S_SEXT_I32_I8,S_SEXT_I32_I16,
            case 16:  case 18:  case 96:  case 97:  case 98:   // S_BITSET0_B32,S_BITSET1_B32,S_CEIL_F32,S_FLOOR_F32,S_TRUNC_F32,
            case 99:  case 100:  case 101:  case 102:   // S_RNDNE_F32,S_CVT_F32_I32,S_CVT_F32_U32,S_CVT_I32_F32,
            case 103:  case 104:  case 105:  case 106:   // S_CVT_U32_F32,S_CVT_F16_F32,S_CVT_F32_F16,S_CVT_HI_F32_F16,
            case 107:  case 108:  case 109:  case 110:   // S_CEIL_F16,S_FLOOR_F16,S_TRUNC_F16,S_RNDNE_F16,
                appendOPR_SDST(layout.SDST,false,true);
                insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u32,layout.LITERAL)),true,false);
                break;
            case 1:  case 5:  case 17:  case 19:   // S_MOV_B64,S_BREV_B64,S_BITSET0_B64,S_BITSET1_B64,
            case 20:   // S_BITREPLICATE_B64_B32,
                appendOPR_SDST(layout.SDST,false,true,2);
                insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u32,layout.LITERAL)),true,false);
                break;
            case 2:   // S_CMOV_B32,
                appendOPR_SDST(layout.SDST,false,true);
                insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u32,layout.LITERAL)),true,false);
                appendOPR_SSRC_SPECIAL_SCC(253,true,false,1,true);
                break;
            case 3:   // S_CMOV_B64,
                appendOPR_SDST(layout.SDST,false,true,2);
                insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u32,layout.LITERAL)),true,false);
                appendOPR_SSRC_SPECIAL_SCC(253,true,false,1,true);
                break;
            case 21:  case 22:  case 23:  case 24:   // S_ABS_I32,S_BCNT0_I32_B32,S_BCNT0_I32_B64,S_BCNT1_I32_B32,
            case 25:  case 26:  case 28:  case 30:   // S_BCNT1_I32_B64,S_QUADMASK_B32,S_WQM_B32,S_NOT_B32,
                appendOPR_SDST(layout.SDST,false,true);
                insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u32,layout.LITERAL)),true,false);
                appendOPR_SSRC_SPECIAL_SCC(253,false,true,1,true);
                break;
            case 27:  case 29:  case 31:   // S_QUADMASK_B64,S_WQM_B64,S_NOT_B64,
                appendOPR_SDST(layout.SDST,false,true,2);
                insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u32,layout.LITERAL)),true,false);
                appendOPR_SSRC_SPECIAL_SCC(253,false,true,1,true);
                break;
            case 32:  case 34:  case 36:  case 38:   // S_AND_SAVEEXEC_B32,S_OR_SAVEEXEC_B32,S_XOR_SAVEEXEC_B32,S_NAND_SAVEEXEC_B32,
            case 40:  case 42:  case 44:   // S_NOR_SAVEEXEC_B32,S_XNOR_SAVEEXEC_B32,S_AND_NOT0_SAVEEXEC_B32,
            case 46:  case 48:  case 50:   // S_OR_NOT0_SAVEEXEC_B32,S_AND_NOT1_SAVEEXEC_B32,S_OR_NOT1_SAVEEXEC_B32,
            case 52:  case 54:   // S_AND_NOT0_WREXEC_B32,S_AND_NOT1_WREXEC_B32,
                appendOPR_SREG(layout.SDST,false,true);
                insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u32,layout.LITERAL)),true,false);
                appendOPR_SDST_EXEC(126,false,true,1,true);
                appendOPR_SSRC_SPECIAL_SCC(253,false,true,1,true);
                appendOPR_SDST_EXEC(126,true,false,1,true);
                break;
            case 33:  case 35:  case 37:  case 39:   // S_AND_SAVEEXEC_B64,S_OR_SAVEEXEC_B64,S_XOR_SAVEEXEC_B64,S_NAND_SAVEEXEC_B64,
            case 41:  case 43:  case 45:   // S_NOR_SAVEEXEC_B64,S_XNOR_SAVEEXEC_B64,S_AND_NOT0_SAVEEXEC_B64,
            case 47:  case 49:  case 51:   // S_OR_NOT0_SAVEEXEC_B64,S_AND_NOT1_SAVEEXEC_B64,S_OR_NOT1_SAVEEXEC_B64,
            case 53:  case 55:   // S_AND_NOT0_WREXEC_B64,S_AND_NOT1_WREXEC_B64,
                appendOPR_SREG(layout.SDST,false,true,2);
                insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u32,layout.LITERAL)),true,false);
                appendOPR_SDST_EXEC(126,false,true,1,true);
                appendOPR_SSRC_SPECIAL_SCC(253,false,true,1,true);
                appendOPR_SDST_EXEC(126,true,false,1,true);
                break;
            case 66:   // S_MOVRELD_B32,
                appendOPR_SREG(layout.SDST,false,true);
                insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u32,layout.LITERAL)),true,false);
                appendOPR_SDST_M0(125,true,false,1,true);
                break;
            case 67:   // S_MOVRELD_B64,
                appendOPR_SREG(layout.SDST,false,true,2);
                insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u32,layout.LITERAL)),true,false);
                appendOPR_SDST_M0(125,true,false,1,true);
                break;
            case 75:   // S_ADD_PC_I64,
                setBranch();
                setModifyPC();
                insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u32,layout.LITERAL)),true,false);
                appendOPR_PC(0,false,true,2,true);
                break;
            case 83:   // S_ALLOC_VGPR,
                insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u32,layout.LITERAL)),true,false);
                appendOPR_SSRC_SPECIAL_SCC(253,false,true,1,true);
                break;
            case 88:   // S_SLEEP_VAR,
                insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u32,layout.LITERAL)),true,false);
                break;
        }
    }

    void InstructionDecoder_amdgpu_gfx1250::finalizeSOP1_INST_LITERAL64Operands()
    {
        layout_SOP1_INST_LITERAL64 & layout = insn_layout.SOP1_INST_LITERAL64;
        switch (layout.OP)  {
            case 0:  case 4:  case 8:  case 9:  case 10:   // S_MOV_B32,S_BREV_B32,S_CTZ_I32_B32,S_CTZ_I32_B64,S_CLZ_I32_U32,
            case 11:  case 12:  case 13:  case 14:  case 15:   // S_CLZ_I32_U64,S_CLS_I32,S_CLS_I32_I64,S_SEXT_I32_I8,S_SEXT_I32_I16,
            case 16:  case 18:  case 96:  case 97:  case 98:   // S_BITSET0_B32,S_BITSET1_B32,S_CEIL_F32,S_FLOOR_F32,S_TRUNC_F32,
            case 99:  case 100:  case 101:  case 102:   // S_RNDNE_F32,S_CVT_F32_I32,S_CVT_F32_U32,S_CVT_I32_F32,
            case 103:  case 104:  case 105:  case 106:   // S_CVT_U32_F32,S_CVT_F16_F32,S_CVT_F32_F16,S_CVT_HI_F32_F16,
            case 107:  case 108:  case 109:  case 110:   // S_CEIL_F16,S_FLOOR_F16,S_TRUNC_F16,S_RNDNE_F16,
                appendOPR_SDST(layout.SDST,false,true);
                insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u64,layout.LITERAL64)),true,false);
                break;
            case 1:  case 5:  case 17:  case 19:   // S_MOV_B64,S_BREV_B64,S_BITSET0_B64,S_BITSET1_B64,
            case 20:   // S_BITREPLICATE_B64_B32,
                appendOPR_SDST(layout.SDST,false,true,2);
                insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u64,layout.LITERAL64)),true,false);
                break;
            case 2:   // S_CMOV_B32,
                appendOPR_SDST(layout.SDST,false,true);
                insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u64,layout.LITERAL64)),true,false);
                appendOPR_SSRC_SPECIAL_SCC(253,true,false,1,true);
                break;
            case 3:   // S_CMOV_B64,
                appendOPR_SDST(layout.SDST,false,true,2);
                insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u64,layout.LITERAL64)),true,false);
                appendOPR_SSRC_SPECIAL_SCC(253,true,false,1,true);
                break;
            case 21:  case 22:  case 23:  case 24:   // S_ABS_I32,S_BCNT0_I32_B32,S_BCNT0_I32_B64,S_BCNT1_I32_B32,
            case 25:  case 26:  case 28:  case 30:   // S_BCNT1_I32_B64,S_QUADMASK_B32,S_WQM_B32,S_NOT_B32,
                appendOPR_SDST(layout.SDST,false,true);
                insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u64,layout.LITERAL64)),true,false);
                appendOPR_SSRC_SPECIAL_SCC(253,false,true,1,true);
                break;
            case 27:  case 29:  case 31:   // S_QUADMASK_B64,S_WQM_B64,S_NOT_B64,
                appendOPR_SDST(layout.SDST,false,true,2);
                insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u64,layout.LITERAL64)),true,false);
                appendOPR_SSRC_SPECIAL_SCC(253,false,true,1,true);
                break;
            case 32:  case 34:  case 36:  case 38:   // S_AND_SAVEEXEC_B32,S_OR_SAVEEXEC_B32,S_XOR_SAVEEXEC_B32,S_NAND_SAVEEXEC_B32,
            case 40:  case 42:  case 44:   // S_NOR_SAVEEXEC_B32,S_XNOR_SAVEEXEC_B32,S_AND_NOT0_SAVEEXEC_B32,
            case 46:  case 48:  case 50:   // S_OR_NOT0_SAVEEXEC_B32,S_AND_NOT1_SAVEEXEC_B32,S_OR_NOT1_SAVEEXEC_B32,
            case 52:  case 54:   // S_AND_NOT0_WREXEC_B32,S_AND_NOT1_WREXEC_B32,
                appendOPR_SREG(layout.SDST,false,true);
                insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u64,layout.LITERAL64)),true,false);
                appendOPR_SDST_EXEC(126,false,true,1,true);
                appendOPR_SSRC_SPECIAL_SCC(253,false,true,1,true);
                appendOPR_SDST_EXEC(126,true,false,1,true);
                break;
            case 33:  case 35:  case 37:  case 39:   // S_AND_SAVEEXEC_B64,S_OR_SAVEEXEC_B64,S_XOR_SAVEEXEC_B64,S_NAND_SAVEEXEC_B64,
            case 41:  case 43:  case 45:   // S_NOR_SAVEEXEC_B64,S_XNOR_SAVEEXEC_B64,S_AND_NOT0_SAVEEXEC_B64,
            case 47:  case 49:  case 51:   // S_OR_NOT0_SAVEEXEC_B64,S_AND_NOT1_SAVEEXEC_B64,S_OR_NOT1_SAVEEXEC_B64,
            case 53:  case 55:   // S_AND_NOT0_WREXEC_B64,S_AND_NOT1_WREXEC_B64,
                appendOPR_SREG(layout.SDST,false,true,2);
                insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u64,layout.LITERAL64)),true,false);
                appendOPR_SDST_EXEC(126,false,true,1,true);
                appendOPR_SSRC_SPECIAL_SCC(253,false,true,1,true);
                appendOPR_SDST_EXEC(126,true,false,1,true);
                break;
            case 66:   // S_MOVRELD_B32,
                appendOPR_SREG(layout.SDST,false,true);
                insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u64,layout.LITERAL64)),true,false);
                appendOPR_SDST_M0(125,true,false,1,true);
                break;
            case 67:   // S_MOVRELD_B64,
                appendOPR_SREG(layout.SDST,false,true,2);
                insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u64,layout.LITERAL64)),true,false);
                appendOPR_SDST_M0(125,true,false,1,true);
                break;
            case 75:   // S_ADD_PC_I64,
                setBranch();
                setModifyPC();
                insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u64,layout.LITERAL64)),true,false);
                appendOPR_PC(0,false,true,2,true);
                break;
            case 83:   // S_ALLOC_VGPR,
                insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u64,layout.LITERAL64)),true,false);
                appendOPR_SSRC_SPECIAL_SCC(253,false,true,1,true);
                break;
            case 88:   // S_SLEEP_VAR,
                insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u64,layout.LITERAL64)),true,false);
                break;
        }
    }

    void InstructionDecoder_amdgpu_gfx1250::finalizeENC_SOP2Operands()
    {
        layout_ENC_SOP2 & layout = insn_layout.ENC_SOP2;
        switch (layout.OP)  {
            case 0:  case 1:  case 2:  case 3:  case 6:   // S_ADD_CO_U32,S_SUB_CO_U32,S_ADD_CO_I32,S_SUB_CO_I32,S_ABSDIFF_I32,
            case 8:  case 10:  case 12:  case 14:  case 15:   // S_LSHL_B32,S_LSHR_B32,S_ASHR_I32,S_LSHL1_ADD_U32,S_LSHL2_ADD_U32,
            case 16:  case 17:  case 18:  case 19:  case 20:   // S_LSHL3_ADD_U32,S_LSHL4_ADD_U32,S_MIN_I32,S_MIN_U32,S_MAX_I32,
            case 21:  case 22:  case 24:  case 26:  case 28:   // S_MAX_U32,S_AND_B32,S_OR_B32,S_XOR_B32,S_NAND_B32,
            case 30:  case 32:  case 34:  case 36:  case 38:   // S_NOR_B32,S_XNOR_B32,S_AND_NOT1_B32,S_OR_NOT1_B32,S_BFE_U32,
            case 39:   // S_BFE_I32,
                appendOPR_SDST(layout.SDST,false,true);
                appendOPR_SSRC(layout.SSRC0,true,false);
                appendOPR_SSRC(layout.SSRC1,true,false);
                appendOPR_SSRC_SPECIAL_SCC(253,false,true,1,true);
                break;
            case 4:  case 5:   // S_ADD_CO_CI_U32,S_SUB_CO_CI_U32,
                appendOPR_SDST(layout.SDST,false,true);
                appendOPR_SSRC(layout.SSRC0,true,false);
                appendOPR_SSRC(layout.SSRC1,true,false);
                appendOPR_SSRC_SPECIAL_SCC(253,false,true,1,true);
                appendOPR_SSRC_SPECIAL_SCC(253,true,false,1,true);
                break;
            case 9:  case 11:  case 13:  case 40:  case 41:   // S_LSHL_B64,S_LSHR_B64,S_ASHR_I64,S_BFE_U64,S_BFE_I64,
                appendOPR_SDST(layout.SDST,false,true,2);
                appendOPR_SSRC(layout.SSRC0,true,false,2);
                appendOPR_SSRC(layout.SSRC1,true,false);
                appendOPR_SSRC_SPECIAL_SCC(253,false,true,1,true);
                break;
            case 23:  case 25:  case 27:  case 29:  case 31:   // S_AND_B64,S_OR_B64,S_XOR_B64,S_NAND_B64,S_NOR_B64,
            case 33:  case 35:  case 37:   // S_XNOR_B64,S_AND_NOT1_B64,S_OR_NOT1_B64,
                appendOPR_SDST(layout.SDST,false,true,2);
                appendOPR_SSRC(layout.SSRC0,true,false,2);
                appendOPR_SSRC(layout.SSRC1,true,false,2);
                appendOPR_SSRC_SPECIAL_SCC(253,false,true,1,true);
                break;
            case 42:  case 44:  case 45:  case 46:  case 50:   // S_BFM_B32,S_MUL_I32,S_MUL_HI_U32,S_MUL_HI_I32,S_PACK_LL_B32_B16,
            case 51:  case 52:  case 53:  case 64:   // S_PACK_LH_B32_B16,S_PACK_HH_B32_B16,S_PACK_HL_B32_B16,S_ADD_F32,
            case 65:  case 66:  case 67:  case 68:  case 71:   // S_SUB_F32,S_MIN_NUM_F32,S_MAX_NUM_F32,S_MUL_F32,S_FMAC_F32,
            case 72:  case 73:  case 74:  case 75:   // S_CVT_PK_RTZ_F16_F32,S_ADD_F16,S_SUB_F16,S_MIN_NUM_F16,
            case 76:  case 77:  case 78:  case 79:  case 80:   // S_MAX_NUM_F16,S_MUL_F16,S_FMAC_F16,S_MINIMUM_F32,S_MAXIMUM_F32,
            case 81:  case 82:   // S_MINIMUM_F16,S_MAXIMUM_F16,
                appendOPR_SDST(layout.SDST,false,true);
                appendOPR_SSRC(layout.SSRC0,true,false);
                appendOPR_SSRC(layout.SSRC1,true,false);
                break;
            case 43:   // S_BFM_B64,
                appendOPR_SDST(layout.SDST,false,true,2);
                appendOPR_SSRC(layout.SSRC0,true,false);
                appendOPR_SSRC(layout.SSRC1,true,false);
                break;
            case 48:   // S_CSELECT_B32,
                appendOPR_SDST(layout.SDST,false,true);
                appendOPR_SSRC(layout.SSRC0,true,false);
                appendOPR_SSRC(layout.SSRC1,true,false);
                appendOPR_SSRC_SPECIAL_SCC(253,true,false,1,true);
                break;
            case 49:   // S_CSELECT_B64,
                appendOPR_SDST(layout.SDST,false,true,2);
                appendOPR_SSRC(layout.SSRC0,true,false,2);
                appendOPR_SSRC(layout.SSRC1,true,false,2);
                appendOPR_SSRC_SPECIAL_SCC(253,true,false,1,true);
                break;
            case 83:  case 84:  case 85:   // S_ADD_NC_U64,S_SUB_NC_U64,S_MUL_U64,
                appendOPR_SDST(layout.SDST,false,true,2);
                appendOPR_SSRC(layout.SSRC0,true,false,2);
                appendOPR_SSRC(layout.SSRC1,true,false,2);
                break;
        }
    }

    void InstructionDecoder_amdgpu_gfx1250::finalizeSOP2_INST_LITERALOperands()
    {
        layout_SOP2_INST_LITERAL & layout = insn_layout.SOP2_INST_LITERAL;
        switch (layout.OP)  {
            case 0:  case 1:  case 2:  case 3:  case 6:   // S_ADD_CO_U32,S_SUB_CO_U32,S_ADD_CO_I32,S_SUB_CO_I32,S_ABSDIFF_I32,
            case 8:  case 10:  case 12:  case 14:  case 15:   // S_LSHL_B32,S_LSHR_B32,S_ASHR_I32,S_LSHL1_ADD_U32,S_LSHL2_ADD_U32,
            case 16:  case 17:  case 18:  case 19:  case 20:   // S_LSHL3_ADD_U32,S_LSHL4_ADD_U32,S_MIN_I32,S_MIN_U32,S_MAX_I32,
            case 21:  case 22:  case 24:  case 26:  case 28:   // S_MAX_U32,S_AND_B32,S_OR_B32,S_XOR_B32,S_NAND_B32,
            case 30:  case 32:  case 34:  case 36:  case 38:   // S_NOR_B32,S_XNOR_B32,S_AND_NOT1_B32,S_OR_NOT1_B32,S_BFE_U32,
            case 39:   // S_BFE_I32,
                if (COND_SOP2_INST_LITERAL_has_lit_0(insn_long))  {
                    appendOPR_SDST(layout.SDST,false,true);
                    insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u32,layout.LITERAL)),true,false);
                    appendOPR_SSRC(layout.SSRC1,true,false);
                    appendOPR_SSRC_SPECIAL_SCC(253,false,true,1,true);
                }
                else if (COND_SOP2_INST_LITERAL_has_lit_1(insn_long))  {
                    appendOPR_SDST(layout.SDST,false,true);
                    appendOPR_SSRC(layout.SSRC0,true,false);
                    insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u32,layout.LITERAL)),true,false);
                    appendOPR_SSRC_SPECIAL_SCC(253,false,true,1,true);
                }
                else  {
                    appendOPR_SDST(layout.SDST,false,true);
                    insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u32,layout.LITERAL)),true,false);
                    insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u32,layout.LITERAL)),true,false);
                    appendOPR_SSRC_SPECIAL_SCC(253,false,true,1,true);
                }
                break;
            case 4:  case 5:   // S_ADD_CO_CI_U32,S_SUB_CO_CI_U32,
                if (COND_SOP2_INST_LITERAL_has_lit_0(insn_long))  {
                    appendOPR_SDST(layout.SDST,false,true);
                    insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u32,layout.LITERAL)),true,false);
                    appendOPR_SSRC(layout.SSRC1,true,false);
                    appendOPR_SSRC_SPECIAL_SCC(253,false,true,1,true);
                    appendOPR_SSRC_SPECIAL_SCC(253,true,false,1,true);
                }
                else if (COND_SOP2_INST_LITERAL_has_lit_1(insn_long))  {
                    appendOPR_SDST(layout.SDST,false,true);
                    appendOPR_SSRC(layout.SSRC0,true,false);
                    insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u32,layout.LITERAL)),true,false);
                    appendOPR_SSRC_SPECIAL_SCC(253,false,true,1,true);
                    appendOPR_SSRC_SPECIAL_SCC(253,true,false,1,true);
                }
                else  {
                    appendOPR_SDST(layout.SDST,false,true);
                    insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u32,layout.LITERAL)),true,false);
                    insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u32,layout.LITERAL)),true,false);
                    appendOPR_SSRC_SPECIAL_SCC(253,false,true,1,true);
                    appendOPR_SSRC_SPECIAL_SCC(253,true,false,1,true);
                }
                break;
            case 9:  case 11:  case 13:  case 40:  case 41:   // S_LSHL_B64,S_LSHR_B64,S_ASHR_I64,S_BFE_U64,S_BFE_I64,
                if (COND_SOP2_INST_LITERAL_has_lit_0(insn_long))  {
                    appendOPR_SDST(layout.SDST,false,true,2);
                    insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u32,layout.LITERAL)),true,false);
                    appendOPR_SSRC(layout.SSRC1,true,false);
                    appendOPR_SSRC_SPECIAL_SCC(253,false,true,1,true);
                }
                else if (COND_SOP2_INST_LITERAL_has_lit_1(insn_long))  {
                    appendOPR_SDST(layout.SDST,false,true,2);
                    appendOPR_SSRC(layout.SSRC0,true,false,2);
                    insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u32,layout.LITERAL)),true,false);
                    appendOPR_SSRC_SPECIAL_SCC(253,false,true,1,true);
                }
                else  {
                    appendOPR_SDST(layout.SDST,false,true,2);
                    insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u32,layout.LITERAL)),true,false);
                    insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u32,layout.LITERAL)),true,false);
                    appendOPR_SSRC_SPECIAL_SCC(253,false,true,1,true);
                }
                break;
            case 23:  case 25:  case 27:  case 29:  case 31:   // S_AND_B64,S_OR_B64,S_XOR_B64,S_NAND_B64,S_NOR_B64,
            case 33:  case 35:  case 37:   // S_XNOR_B64,S_AND_NOT1_B64,S_OR_NOT1_B64,
                if (COND_SOP2_INST_LITERAL_has_lit_0(insn_long))  {
                    appendOPR_SDST(layout.SDST,false,true,2);
                    insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u32,layout.LITERAL)),true,false);
                    appendOPR_SSRC(layout.SSRC1,true,false,2);
                    appendOPR_SSRC_SPECIAL_SCC(253,false,true,1,true);
                }
                else if (COND_SOP2_INST_LITERAL_has_lit_1(insn_long))  {
                    appendOPR_SDST(layout.SDST,false,true,2);
                    appendOPR_SSRC(layout.SSRC0,true,false,2);
                    insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u32,layout.LITERAL)),true,false);
                    appendOPR_SSRC_SPECIAL_SCC(253,false,true,1,true);
                }
                else  {
                    appendOPR_SDST(layout.SDST,false,true,2);
                    insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u32,layout.LITERAL)),true,false);
                    insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u32,layout.LITERAL)),true,false);
                    appendOPR_SSRC_SPECIAL_SCC(253,false,true,1,true);
                }
                break;
            case 42:  case 44:  case 45:  case 46:  case 50:   // S_BFM_B32,S_MUL_I32,S_MUL_HI_U32,S_MUL_HI_I32,S_PACK_LL_B32_B16,
            case 51:  case 52:  case 53:  case 64:   // S_PACK_LH_B32_B16,S_PACK_HH_B32_B16,S_PACK_HL_B32_B16,S_ADD_F32,
            case 65:  case 66:  case 67:  case 68:  case 71:   // S_SUB_F32,S_MIN_NUM_F32,S_MAX_NUM_F32,S_MUL_F32,S_FMAC_F32,
            case 72:  case 73:  case 74:  case 75:   // S_CVT_PK_RTZ_F16_F32,S_ADD_F16,S_SUB_F16,S_MIN_NUM_F16,
            case 76:  case 77:  case 78:  case 79:  case 80:   // S_MAX_NUM_F16,S_MUL_F16,S_FMAC_F16,S_MINIMUM_F32,S_MAXIMUM_F32,
            case 81:  case 82:   // S_MINIMUM_F16,S_MAXIMUM_F16,
                if (COND_SOP2_INST_LITERAL_has_lit_0(insn_long))  {
                    appendOPR_SDST(layout.SDST,false,true);
                    insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u32,layout.LITERAL)),true,false);
                    appendOPR_SSRC(layout.SSRC1,true,false);
                }
                else if (COND_SOP2_INST_LITERAL_has_lit_1(insn_long))  {
                    appendOPR_SDST(layout.SDST,false,true);
                    appendOPR_SSRC(layout.SSRC0,true,false);
                    insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u32,layout.LITERAL)),true,false);
                }
                else  {
                    appendOPR_SDST(layout.SDST,false,true);
                    insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u32,layout.LITERAL)),true,false);
                    insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u32,layout.LITERAL)),true,false);
                }
                break;
            case 43:   // S_BFM_B64,
                if (COND_SOP2_INST_LITERAL_has_lit_0(insn_long))  {
                    appendOPR_SDST(layout.SDST,false,true,2);
                    insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u32,layout.LITERAL)),true,false);
                    appendOPR_SSRC(layout.SSRC1,true,false);
                }
                else if (COND_SOP2_INST_LITERAL_has_lit_1(insn_long))  {
                    appendOPR_SDST(layout.SDST,false,true,2);
                    appendOPR_SSRC(layout.SSRC0,true,false);
                    insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u32,layout.LITERAL)),true,false);
                }
                else  {
                    appendOPR_SDST(layout.SDST,false,true,2);
                    insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u32,layout.LITERAL)),true,false);
                    insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u32,layout.LITERAL)),true,false);
                }
                break;
            case 48:   // S_CSELECT_B32,
                if (COND_SOP2_INST_LITERAL_has_lit_0(insn_long))  {
                    appendOPR_SDST(layout.SDST,false,true);
                    insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u32,layout.LITERAL)),true,false);
                    appendOPR_SSRC(layout.SSRC1,true,false);
                    appendOPR_SSRC_SPECIAL_SCC(253,true,false,1,true);
                }
                else if (COND_SOP2_INST_LITERAL_has_lit_1(insn_long))  {
                    appendOPR_SDST(layout.SDST,false,true);
                    appendOPR_SSRC(layout.SSRC0,true,false);
                    insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u32,layout.LITERAL)),true,false);
                    appendOPR_SSRC_SPECIAL_SCC(253,true,false,1,true);
                }
                else  {
                    appendOPR_SDST(layout.SDST,false,true);
                    insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u32,layout.LITERAL)),true,false);
                    insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u32,layout.LITERAL)),true,false);
                    appendOPR_SSRC_SPECIAL_SCC(253,true,false,1,true);
                }
                break;
            case 49:   // S_CSELECT_B64,
                if (COND_SOP2_INST_LITERAL_has_lit_0(insn_long))  {
                    appendOPR_SDST(layout.SDST,false,true,2);
                    insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u32,layout.LITERAL)),true,false);
                    appendOPR_SSRC(layout.SSRC1,true,false,2);
                    appendOPR_SSRC_SPECIAL_SCC(253,true,false,1,true);
                }
                else if (COND_SOP2_INST_LITERAL_has_lit_1(insn_long))  {
                    appendOPR_SDST(layout.SDST,false,true,2);
                    appendOPR_SSRC(layout.SSRC0,true,false,2);
                    insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u32,layout.LITERAL)),true,false);
                    appendOPR_SSRC_SPECIAL_SCC(253,true,false,1,true);
                }
                else  {
                    appendOPR_SDST(layout.SDST,false,true,2);
                    insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u32,layout.LITERAL)),true,false);
                    insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u32,layout.LITERAL)),true,false);
                    appendOPR_SSRC_SPECIAL_SCC(253,true,false,1,true);
                }
                break;
            case 69:   // S_FMAAK_F32,
                if (COND_SOP2_INST_LITERAL_default(insn_long))  {
                    appendOPR_SDST(layout.SDST,false,true);
                    appendOPR_SSRC(layout.SSRC0,true,false);
                    appendOPR_SSRC(layout.SSRC1,true,false);
                    insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u32,layout.LITERAL)),true,false);
                }
                else if (COND_SOP2_INST_LITERAL_has_lit_0(insn_long))  {
                    appendOPR_SDST(layout.SDST,false,true);
                    insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u32,layout.LITERAL)),true,false);
                    appendOPR_SSRC(layout.SSRC1,true,false);
                    insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u32,layout.LITERAL)),true,false);
                }
                else if (COND_SOP2_INST_LITERAL_has_lit_1(insn_long))  {
                    appendOPR_SDST(layout.SDST,false,true);
                    appendOPR_SSRC(layout.SSRC0,true,false);
                    insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u32,layout.LITERAL)),true,false);
                    insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u32,layout.LITERAL)),true,false);
                }
                else  {
                    appendOPR_SDST(layout.SDST,false,true);
                    insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u32,layout.LITERAL)),true,false);
                    insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u32,layout.LITERAL)),true,false);
                    insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u32,layout.LITERAL)),true,false);
                }
                break;
            case 70:   // S_FMAMK_F32,
                if (COND_SOP2_INST_LITERAL_default(insn_long))  {
                    appendOPR_SDST(layout.SDST,false,true);
                    appendOPR_SSRC(layout.SSRC0,true,false);
                    insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u32,layout.LITERAL)),true,false);
                    appendOPR_SSRC(layout.SSRC1,true,false);
                }
                else if (COND_SOP2_INST_LITERAL_has_lit_0(insn_long))  {
                    appendOPR_SDST(layout.SDST,false,true);
                    insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u32,layout.LITERAL)),true,false);
                    insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u32,layout.LITERAL)),true,false);
                    appendOPR_SSRC(layout.SSRC1,true,false);
                }
                else if (COND_SOP2_INST_LITERAL_has_lit_1(insn_long))  {
                    appendOPR_SDST(layout.SDST,false,true);
                    appendOPR_SSRC(layout.SSRC0,true,false);
                    insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u32,layout.LITERAL)),true,false);
                    insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u32,layout.LITERAL)),true,false);
                }
                else  {
                    appendOPR_SDST(layout.SDST,false,true);
                    insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u32,layout.LITERAL)),true,false);
                    insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u32,layout.LITERAL)),true,false);
                    insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u32,layout.LITERAL)),true,false);
                }
                break;
            case 83:  case 84:  case 85:   // S_ADD_NC_U64,S_SUB_NC_U64,S_MUL_U64,
                if (COND_SOP2_INST_LITERAL_has_lit_0(insn_long))  {
                    appendOPR_SDST(layout.SDST,false,true,2);
                    insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u32,layout.LITERAL)),true,false);
                    appendOPR_SSRC(layout.SSRC1,true,false,2);
                }
                else if (COND_SOP2_INST_LITERAL_has_lit_1(insn_long))  {
                    appendOPR_SDST(layout.SDST,false,true,2);
                    appendOPR_SSRC(layout.SSRC0,true,false,2);
                    insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u32,layout.LITERAL)),true,false);
                }
                else  {
                    appendOPR_SDST(layout.SDST,false,true,2);
                    insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u32,layout.LITERAL)),true,false);
                    insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u32,layout.LITERAL)),true,false);
                }
                break;
        }
    }

    void InstructionDecoder_amdgpu_gfx1250::finalizeSOP2_INST_LITERAL64Operands()
    {
        layout_SOP2_INST_LITERAL64 & layout = insn_layout.SOP2_INST_LITERAL64;
        switch (layout.OP)  {
            case 0:  case 1:  case 2:  case 3:  case 6:   // S_ADD_CO_U32,S_SUB_CO_U32,S_ADD_CO_I32,S_SUB_CO_I32,S_ABSDIFF_I32,
            case 8:  case 10:  case 12:  case 14:  case 15:   // S_LSHL_B32,S_LSHR_B32,S_ASHR_I32,S_LSHL1_ADD_U32,S_LSHL2_ADD_U32,
            case 16:  case 17:  case 18:  case 19:  case 20:   // S_LSHL3_ADD_U32,S_LSHL4_ADD_U32,S_MIN_I32,S_MIN_U32,S_MAX_I32,
            case 21:  case 22:  case 24:  case 26:  case 28:   // S_MAX_U32,S_AND_B32,S_OR_B32,S_XOR_B32,S_NAND_B32,
            case 30:  case 32:  case 34:  case 36:  case 38:   // S_NOR_B32,S_XNOR_B32,S_AND_NOT1_B32,S_OR_NOT1_B32,S_BFE_U32,
            case 39:   // S_BFE_I32,
                if (COND_SOP2_INST_LITERAL64_has_lit64_0(insn_long))  {
                    appendOPR_SDST(layout.SDST,false,true);
                    insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u64,layout.LITERAL64)),true,false);
                    appendOPR_SSRC(layout.SSRC1,true,false);
                    appendOPR_SSRC_SPECIAL_SCC(253,false,true,1,true);
                }
                else if (COND_SOP2_INST_LITERAL64_has_lit64_1(insn_long))  {
                    appendOPR_SDST(layout.SDST,false,true);
                    appendOPR_SSRC(layout.SSRC0,true,false);
                    insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u64,layout.LITERAL64)),true,false);
                    appendOPR_SSRC_SPECIAL_SCC(253,false,true,1,true);
                }
                else  {
                    appendOPR_SDST(layout.SDST,false,true);
                    insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u64,layout.LITERAL64)),true,false);
                    insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u64,layout.LITERAL64)),true,false);
                    appendOPR_SSRC_SPECIAL_SCC(253,false,true,1,true);
                }
                break;
            case 4:  case 5:   // S_ADD_CO_CI_U32,S_SUB_CO_CI_U32,
                if (COND_SOP2_INST_LITERAL64_has_lit64_0(insn_long))  {
                    appendOPR_SDST(layout.SDST,false,true);
                    insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u64,layout.LITERAL64)),true,false);
                    appendOPR_SSRC(layout.SSRC1,true,false);
                    appendOPR_SSRC_SPECIAL_SCC(253,false,true,1,true);
                    appendOPR_SSRC_SPECIAL_SCC(253,true,false,1,true);
                }
                else if (COND_SOP2_INST_LITERAL64_has_lit64_1(insn_long))  {
                    appendOPR_SDST(layout.SDST,false,true);
                    appendOPR_SSRC(layout.SSRC0,true,false);
                    insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u64,layout.LITERAL64)),true,false);
                    appendOPR_SSRC_SPECIAL_SCC(253,false,true,1,true);
                    appendOPR_SSRC_SPECIAL_SCC(253,true,false,1,true);
                }
                else  {
                    appendOPR_SDST(layout.SDST,false,true);
                    insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u64,layout.LITERAL64)),true,false);
                    insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u64,layout.LITERAL64)),true,false);
                    appendOPR_SSRC_SPECIAL_SCC(253,false,true,1,true);
                    appendOPR_SSRC_SPECIAL_SCC(253,true,false,1,true);
                }
                break;
            case 9:  case 11:  case 13:  case 40:  case 41:   // S_LSHL_B64,S_LSHR_B64,S_ASHR_I64,S_BFE_U64,S_BFE_I64,
                if (COND_SOP2_INST_LITERAL64_has_lit64_0(insn_long))  {
                    appendOPR_SDST(layout.SDST,false,true,2);
                    insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u64,layout.LITERAL64)),true,false);
                    appendOPR_SSRC(layout.SSRC1,true,false);
                    appendOPR_SSRC_SPECIAL_SCC(253,false,true,1,true);
                }
                else if (COND_SOP2_INST_LITERAL64_has_lit64_1(insn_long))  {
                    appendOPR_SDST(layout.SDST,false,true,2);
                    appendOPR_SSRC(layout.SSRC0,true,false,2);
                    insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u64,layout.LITERAL64)),true,false);
                    appendOPR_SSRC_SPECIAL_SCC(253,false,true,1,true);
                }
                else  {
                    appendOPR_SDST(layout.SDST,false,true,2);
                    insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u64,layout.LITERAL64)),true,false);
                    insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u64,layout.LITERAL64)),true,false);
                    appendOPR_SSRC_SPECIAL_SCC(253,false,true,1,true);
                }
                break;
            case 23:  case 25:  case 27:  case 29:  case 31:   // S_AND_B64,S_OR_B64,S_XOR_B64,S_NAND_B64,S_NOR_B64,
            case 33:  case 35:  case 37:   // S_XNOR_B64,S_AND_NOT1_B64,S_OR_NOT1_B64,
                if (COND_SOP2_INST_LITERAL64_has_lit64_0(insn_long))  {
                    appendOPR_SDST(layout.SDST,false,true,2);
                    insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u64,layout.LITERAL64)),true,false);
                    appendOPR_SSRC(layout.SSRC1,true,false,2);
                    appendOPR_SSRC_SPECIAL_SCC(253,false,true,1,true);
                }
                else if (COND_SOP2_INST_LITERAL64_has_lit64_1(insn_long))  {
                    appendOPR_SDST(layout.SDST,false,true,2);
                    appendOPR_SSRC(layout.SSRC0,true,false,2);
                    insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u64,layout.LITERAL64)),true,false);
                    appendOPR_SSRC_SPECIAL_SCC(253,false,true,1,true);
                }
                else  {
                    appendOPR_SDST(layout.SDST,false,true,2);
                    insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u64,layout.LITERAL64)),true,false);
                    insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u64,layout.LITERAL64)),true,false);
                    appendOPR_SSRC_SPECIAL_SCC(253,false,true,1,true);
                }
                break;
            case 42:  case 44:  case 45:  case 46:  case 50:   // S_BFM_B32,S_MUL_I32,S_MUL_HI_U32,S_MUL_HI_I32,S_PACK_LL_B32_B16,
            case 51:  case 52:  case 53:  case 64:   // S_PACK_LH_B32_B16,S_PACK_HH_B32_B16,S_PACK_HL_B32_B16,S_ADD_F32,
            case 65:  case 66:  case 67:  case 68:  case 71:   // S_SUB_F32,S_MIN_NUM_F32,S_MAX_NUM_F32,S_MUL_F32,S_FMAC_F32,
            case 72:  case 73:  case 74:  case 75:   // S_CVT_PK_RTZ_F16_F32,S_ADD_F16,S_SUB_F16,S_MIN_NUM_F16,
            case 76:  case 77:  case 78:  case 79:  case 80:   // S_MAX_NUM_F16,S_MUL_F16,S_FMAC_F16,S_MINIMUM_F32,S_MAXIMUM_F32,
            case 81:  case 82:   // S_MINIMUM_F16,S_MAXIMUM_F16,
                if (COND_SOP2_INST_LITERAL64_has_lit64_0(insn_long))  {
                    appendOPR_SDST(layout.SDST,false,true);
                    insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u64,layout.LITERAL64)),true,false);
                    appendOPR_SSRC(layout.SSRC1,true,false);
                }
                else if (COND_SOP2_INST_LITERAL64_has_lit64_1(insn_long))  {
                    appendOPR_SDST(layout.SDST,false,true);
                    appendOPR_SSRC(layout.SSRC0,true,false);
                    insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u64,layout.LITERAL64)),true,false);
                }
                else  {
                    appendOPR_SDST(layout.SDST,false,true);
                    insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u64,layout.LITERAL64)),true,false);
                    insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u64,layout.LITERAL64)),true,false);
                }
                break;
            case 43:   // S_BFM_B64,
                if (COND_SOP2_INST_LITERAL64_has_lit64_0(insn_long))  {
                    appendOPR_SDST(layout.SDST,false,true,2);
                    insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u64,layout.LITERAL64)),true,false);
                    appendOPR_SSRC(layout.SSRC1,true,false);
                }
                else if (COND_SOP2_INST_LITERAL64_has_lit64_1(insn_long))  {
                    appendOPR_SDST(layout.SDST,false,true,2);
                    appendOPR_SSRC(layout.SSRC0,true,false);
                    insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u64,layout.LITERAL64)),true,false);
                }
                else  {
                    appendOPR_SDST(layout.SDST,false,true,2);
                    insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u64,layout.LITERAL64)),true,false);
                    insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u64,layout.LITERAL64)),true,false);
                }
                break;
            case 48:   // S_CSELECT_B32,
                if (COND_SOP2_INST_LITERAL64_has_lit64_0(insn_long))  {
                    appendOPR_SDST(layout.SDST,false,true);
                    insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u64,layout.LITERAL64)),true,false);
                    appendOPR_SSRC(layout.SSRC1,true,false);
                    appendOPR_SSRC_SPECIAL_SCC(253,true,false,1,true);
                }
                else if (COND_SOP2_INST_LITERAL64_has_lit64_1(insn_long))  {
                    appendOPR_SDST(layout.SDST,false,true);
                    appendOPR_SSRC(layout.SSRC0,true,false);
                    insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u64,layout.LITERAL64)),true,false);
                    appendOPR_SSRC_SPECIAL_SCC(253,true,false,1,true);
                }
                else  {
                    appendOPR_SDST(layout.SDST,false,true);
                    insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u64,layout.LITERAL64)),true,false);
                    insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u64,layout.LITERAL64)),true,false);
                    appendOPR_SSRC_SPECIAL_SCC(253,true,false,1,true);
                }
                break;
            case 49:   // S_CSELECT_B64,
                if (COND_SOP2_INST_LITERAL64_has_lit64_0(insn_long))  {
                    appendOPR_SDST(layout.SDST,false,true,2);
                    insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u64,layout.LITERAL64)),true,false);
                    appendOPR_SSRC(layout.SSRC1,true,false,2);
                    appendOPR_SSRC_SPECIAL_SCC(253,true,false,1,true);
                }
                else if (COND_SOP2_INST_LITERAL64_has_lit64_1(insn_long))  {
                    appendOPR_SDST(layout.SDST,false,true,2);
                    appendOPR_SSRC(layout.SSRC0,true,false,2);
                    insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u64,layout.LITERAL64)),true,false);
                    appendOPR_SSRC_SPECIAL_SCC(253,true,false,1,true);
                }
                else  {
                    appendOPR_SDST(layout.SDST,false,true,2);
                    insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u64,layout.LITERAL64)),true,false);
                    insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u64,layout.LITERAL64)),true,false);
                    appendOPR_SSRC_SPECIAL_SCC(253,true,false,1,true);
                }
                break;
            case 83:  case 84:  case 85:   // S_ADD_NC_U64,S_SUB_NC_U64,S_MUL_U64,
                if (COND_SOP2_INST_LITERAL64_has_lit64_0(insn_long))  {
                    appendOPR_SDST(layout.SDST,false,true,2);
                    insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u64,layout.LITERAL64)),true,false);
                    appendOPR_SSRC(layout.SSRC1,true,false,2);
                }
                else if (COND_SOP2_INST_LITERAL64_has_lit64_1(insn_long))  {
                    appendOPR_SDST(layout.SDST,false,true,2);
                    appendOPR_SSRC(layout.SSRC0,true,false,2);
                    insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u64,layout.LITERAL64)),true,false);
                }
                else  {
                    appendOPR_SDST(layout.SDST,false,true,2);
                    insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u64,layout.LITERAL64)),true,false);
                    insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u64,layout.LITERAL64)),true,false);
                }
                break;
        }
    }

    void InstructionDecoder_amdgpu_gfx1250::finalizeENC_SOPCOperands()
    {
        layout_ENC_SOPC & layout = insn_layout.ENC_SOPC;
        switch (layout.OP)  {
            case 0:  case 1:  case 2:  case 3:  case 4:   // S_CMP_EQ_I32,S_CMP_LG_I32,S_CMP_GT_I32,S_CMP_GE_I32,S_CMP_LT_I32,
            case 5:  case 6:  case 7:  case 8:  case 9:   // S_CMP_LE_I32,S_CMP_EQ_U32,S_CMP_LG_U32,S_CMP_GT_U32,S_CMP_GE_U32,
            case 10:  case 11:  case 12:  case 13:  case 65:   // S_CMP_LT_U32,S_CMP_LE_U32,S_BITCMP0_B32,S_BITCMP1_B32,S_CMP_LT_F32,
            case 81:  case 66:  case 82:  case 67:  case 83:   // S_CMP_LT_F16,S_CMP_EQ_F32,S_CMP_EQ_F16,S_CMP_LE_F32,S_CMP_LE_F16,
            case 68:  case 84:  case 69:  case 85:  case 70:   // S_CMP_GT_F32,S_CMP_GT_F16,S_CMP_LG_F32,S_CMP_LG_F16,S_CMP_GE_F32,
            case 86:  case 71:  case 87:  case 72:  case 88:   // S_CMP_GE_F16,S_CMP_O_F32,S_CMP_O_F16,S_CMP_U_F32,S_CMP_U_F16,
            case 73:  case 89:  case 74:  case 90:   // S_CMP_NGE_F32,S_CMP_NGE_F16,S_CMP_NLG_F32,S_CMP_NLG_F16,
            case 75:  case 91:  case 76:  case 92:   // S_CMP_NGT_F32,S_CMP_NGT_F16,S_CMP_NLE_F32,S_CMP_NLE_F16,
            case 77:  case 93:  case 78:  case 94:   // S_CMP_NEQ_F32,S_CMP_NEQ_F16,S_CMP_NLT_F32,S_CMP_NLT_F16,
                appendOPR_SSRC(layout.SSRC0,true,false);
                appendOPR_SSRC(layout.SSRC1,true,false);
                appendOPR_SSRC_SPECIAL_SCC(253,false,true,1,true);
                break;
            case 14:  case 15:   // S_BITCMP0_B64,S_BITCMP1_B64,
                appendOPR_SSRC(layout.SSRC0,true,false,2);
                appendOPR_SSRC(layout.SSRC1,true,false);
                appendOPR_SSRC_SPECIAL_SCC(253,false,true,1,true);
                break;
            case 16:  case 17:   // S_CMP_EQ_U64,S_CMP_LG_U64,
                appendOPR_SSRC(layout.SSRC0,true,false,2);
                appendOPR_SSRC(layout.SSRC1,true,false,2);
                appendOPR_SSRC_SPECIAL_SCC(253,false,true,1,true);
                break;
        }
    }

    void InstructionDecoder_amdgpu_gfx1250::finalizeSOPC_INST_LITERALOperands()
    {
        layout_SOPC_INST_LITERAL & layout = insn_layout.SOPC_INST_LITERAL;
        switch (layout.OP)  {
            case 0:  case 1:  case 2:  case 3:  case 4:   // S_CMP_EQ_I32,S_CMP_LG_I32,S_CMP_GT_I32,S_CMP_GE_I32,S_CMP_LT_I32,
            case 5:  case 6:  case 7:  case 8:  case 9:   // S_CMP_LE_I32,S_CMP_EQ_U32,S_CMP_LG_U32,S_CMP_GT_U32,S_CMP_GE_U32,
            case 10:  case 11:  case 12:  case 13:  case 65:   // S_CMP_LT_U32,S_CMP_LE_U32,S_BITCMP0_B32,S_BITCMP1_B32,S_CMP_LT_F32,
            case 81:  case 66:  case 82:  case 67:  case 83:   // S_CMP_LT_F16,S_CMP_EQ_F32,S_CMP_EQ_F16,S_CMP_LE_F32,S_CMP_LE_F16,
            case 68:  case 84:  case 69:  case 85:  case 70:   // S_CMP_GT_F32,S_CMP_GT_F16,S_CMP_LG_F32,S_CMP_LG_F16,S_CMP_GE_F32,
            case 86:  case 71:  case 87:  case 72:  case 88:   // S_CMP_GE_F16,S_CMP_O_F32,S_CMP_O_F16,S_CMP_U_F32,S_CMP_U_F16,
            case 73:  case 89:  case 74:  case 90:   // S_CMP_NGE_F32,S_CMP_NGE_F16,S_CMP_NLG_F32,S_CMP_NLG_F16,
            case 75:  case 91:  case 76:  case 92:   // S_CMP_NGT_F32,S_CMP_NGT_F16,S_CMP_NLE_F32,S_CMP_NLE_F16,
            case 77:  case 93:  case 78:  case 94:   // S_CMP_NEQ_F32,S_CMP_NEQ_F16,S_CMP_NLT_F32,S_CMP_NLT_F16,
                if (COND_SOPC_INST_LITERAL_has_lit_0(insn_long))  {
                    insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u32,layout.LITERAL)),true,false);
                    appendOPR_SSRC(layout.SSRC1,true,false);
                    appendOPR_SSRC_SPECIAL_SCC(253,false,true,1,true);
                }
                else if (COND_SOPC_INST_LITERAL_has_lit_1(insn_long))  {
                    appendOPR_SSRC(layout.SSRC0,true,false);
                    insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u32,layout.LITERAL)),true,false);
                    appendOPR_SSRC_SPECIAL_SCC(253,false,true,1,true);
                }
                else  {
                    insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u32,layout.LITERAL)),true,false);
                    insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u32,layout.LITERAL)),true,false);
                    appendOPR_SSRC_SPECIAL_SCC(253,false,true,1,true);
                }
                break;
            case 14:  case 15:   // S_BITCMP0_B64,S_BITCMP1_B64,
                if (COND_SOPC_INST_LITERAL_has_lit_0(insn_long))  {
                    insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u32,layout.LITERAL)),true,false);
                    appendOPR_SSRC(layout.SSRC1,true,false);
                    appendOPR_SSRC_SPECIAL_SCC(253,false,true,1,true);
                }
                else if (COND_SOPC_INST_LITERAL_has_lit_1(insn_long))  {
                    appendOPR_SSRC(layout.SSRC0,true,false,2);
                    insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u32,layout.LITERAL)),true,false);
                    appendOPR_SSRC_SPECIAL_SCC(253,false,true,1,true);
                }
                else  {
                    insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u32,layout.LITERAL)),true,false);
                    insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u32,layout.LITERAL)),true,false);
                    appendOPR_SSRC_SPECIAL_SCC(253,false,true,1,true);
                }
                break;
            case 16:  case 17:   // S_CMP_EQ_U64,S_CMP_LG_U64,
                if (COND_SOPC_INST_LITERAL_has_lit_0(insn_long))  {
                    insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u32,layout.LITERAL)),true,false);
                    appendOPR_SSRC(layout.SSRC1,true,false,2);
                    appendOPR_SSRC_SPECIAL_SCC(253,false,true,1,true);
                }
                else if (COND_SOPC_INST_LITERAL_has_lit_1(insn_long))  {
                    appendOPR_SSRC(layout.SSRC0,true,false,2);
                    insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u32,layout.LITERAL)),true,false);
                    appendOPR_SSRC_SPECIAL_SCC(253,false,true,1,true);
                }
                else  {
                    insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u32,layout.LITERAL)),true,false);
                    insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u32,layout.LITERAL)),true,false);
                    appendOPR_SSRC_SPECIAL_SCC(253,false,true,1,true);
                }
                break;
        }
    }

    void InstructionDecoder_amdgpu_gfx1250::finalizeSOPC_INST_LITERAL64Operands()
    {
        layout_SOPC_INST_LITERAL64 & layout = insn_layout.SOPC_INST_LITERAL64;
        switch (layout.OP)  {
            case 0:  case 1:  case 2:  case 3:  case 4:   // S_CMP_EQ_I32,S_CMP_LG_I32,S_CMP_GT_I32,S_CMP_GE_I32,S_CMP_LT_I32,
            case 5:  case 6:  case 7:  case 8:  case 9:   // S_CMP_LE_I32,S_CMP_EQ_U32,S_CMP_LG_U32,S_CMP_GT_U32,S_CMP_GE_U32,
            case 10:  case 11:  case 12:  case 13:  case 65:   // S_CMP_LT_U32,S_CMP_LE_U32,S_BITCMP0_B32,S_BITCMP1_B32,S_CMP_LT_F32,
            case 81:  case 66:  case 82:  case 67:  case 83:   // S_CMP_LT_F16,S_CMP_EQ_F32,S_CMP_EQ_F16,S_CMP_LE_F32,S_CMP_LE_F16,
            case 68:  case 84:  case 69:  case 85:  case 70:   // S_CMP_GT_F32,S_CMP_GT_F16,S_CMP_LG_F32,S_CMP_LG_F16,S_CMP_GE_F32,
            case 86:  case 71:  case 87:  case 72:  case 88:   // S_CMP_GE_F16,S_CMP_O_F32,S_CMP_O_F16,S_CMP_U_F32,S_CMP_U_F16,
            case 73:  case 89:  case 74:  case 90:   // S_CMP_NGE_F32,S_CMP_NGE_F16,S_CMP_NLG_F32,S_CMP_NLG_F16,
            case 75:  case 91:  case 76:  case 92:   // S_CMP_NGT_F32,S_CMP_NGT_F16,S_CMP_NLE_F32,S_CMP_NLE_F16,
            case 77:  case 93:  case 78:  case 94:   // S_CMP_NEQ_F32,S_CMP_NEQ_F16,S_CMP_NLT_F32,S_CMP_NLT_F16,
                if (COND_SOPC_INST_LITERAL64_has_lit64_0(insn_long))  {
                    insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u64,layout.LITERAL64)),true,false);
                    appendOPR_SSRC(layout.SSRC1,true,false);
                    appendOPR_SSRC_SPECIAL_SCC(253,false,true,1,true);
                }
                else if (COND_SOPC_INST_LITERAL64_has_lit64_1(insn_long))  {
                    appendOPR_SSRC(layout.SSRC0,true,false);
                    insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u64,layout.LITERAL64)),true,false);
                    appendOPR_SSRC_SPECIAL_SCC(253,false,true,1,true);
                }
                else  {
                    insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u64,layout.LITERAL64)),true,false);
                    insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u64,layout.LITERAL64)),true,false);
                    appendOPR_SSRC_SPECIAL_SCC(253,false,true,1,true);
                }
                break;
            case 14:  case 15:   // S_BITCMP0_B64,S_BITCMP1_B64,
                if (COND_SOPC_INST_LITERAL64_has_lit64_0(insn_long))  {
                    insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u64,layout.LITERAL64)),true,false);
                    appendOPR_SSRC(layout.SSRC1,true,false);
                    appendOPR_SSRC_SPECIAL_SCC(253,false,true,1,true);
                }
                else if (COND_SOPC_INST_LITERAL64_has_lit64_1(insn_long))  {
                    appendOPR_SSRC(layout.SSRC0,true,false,2);
                    insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u64,layout.LITERAL64)),true,false);
                    appendOPR_SSRC_SPECIAL_SCC(253,false,true,1,true);
                }
                else  {
                    insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u64,layout.LITERAL64)),true,false);
                    insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u64,layout.LITERAL64)),true,false);
                    appendOPR_SSRC_SPECIAL_SCC(253,false,true,1,true);
                }
                break;
            case 16:  case 17:   // S_CMP_EQ_U64,S_CMP_LG_U64,
                if (COND_SOPC_INST_LITERAL64_has_lit64_0(insn_long))  {
                    insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u64,layout.LITERAL64)),true,false);
                    appendOPR_SSRC(layout.SSRC1,true,false,2);
                    appendOPR_SSRC_SPECIAL_SCC(253,false,true,1,true);
                }
                else if (COND_SOPC_INST_LITERAL64_has_lit64_1(insn_long))  {
                    appendOPR_SSRC(layout.SSRC0,true,false,2);
                    insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u64,layout.LITERAL64)),true,false);
                    appendOPR_SSRC_SPECIAL_SCC(253,false,true,1,true);
                }
                else  {
                    insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u64,layout.LITERAL64)),true,false);
                    insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u64,layout.LITERAL64)),true,false);
                    appendOPR_SSRC_SPECIAL_SCC(253,false,true,1,true);
                }
                break;
        }
    }

    void InstructionDecoder_amdgpu_gfx1250::finalizeENC_SOPKOperands()
    {
        layout_ENC_SOPK & layout = insn_layout.ENC_SOPK;
        switch (layout.OP)  {
            case 0:  case 16:   // S_MOVK_I32,S_MULK_I32,
                appendOPR_SDST(layout.SDST,false,true);
                appendOPR_SIMM16(layout.SIMM16,true,false);
                break;
            case 1:   // S_VERSION,
                appendOPR_VERSION(layout.SIMM16,true,false);
                break;
            case 2:   // S_CMOVK_I32,
                appendOPR_SDST(layout.SDST,false,true);
                appendOPR_SIMM16(layout.SIMM16,true,false);
                appendOPR_SSRC_SPECIAL_SCC(253,true,false,1,true);
                break;
            case 15:   // S_ADDK_CO_I32,
                appendOPR_SDST(layout.SDST,false,true);
                appendOPR_SIMM16(layout.SIMM16,true,false);
                appendOPR_SSRC_SPECIAL_SCC(253,false,true,1,true);
                break;
            case 17:   // S_GETREG_B32,
                appendOPR_SDST(layout.SDST,false,true);
                appendOPR_HWREG(layout.SIMM16,true,false);
                break;
            case 18:   // S_SETREG_B32,
                appendOPR_HWREG(layout.SIMM16,false,true);
                appendOPR_SDST(layout.SDST,true,false);
                break;
            case 20:   // S_CALL_I64,
                setCall();
                appendOPR_SDST(layout.SDST,false,true,2);
                makeBranchTarget(isCall,isConditional,layout.SIMM16);
                appendOPR_PC(0,false,true,2,true);
                appendOPR_PC(0,true,false,2,true);
                break;
        }
    }

    void InstructionDecoder_amdgpu_gfx1250::finalizeSOPK_INST_LITERALOperands()
    {
        layout_SOPK_INST_LITERAL & layout = insn_layout.SOPK_INST_LITERAL;
        switch (layout.OP)  {
            case 19:   // S_SETREG_IMM32_B32,
                appendOPR_HWREG(layout.SIMM16,false,true);
                insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u32,layout.LITERAL)),true,false);
                break;
        }
    }

    void InstructionDecoder_amdgpu_gfx1250::finalizeENC_SOPPOperands()
    {
        layout_ENC_SOPP & layout = insn_layout.ENC_SOPP;
        switch (layout.OP)  {
            case 0:  case 2:  case 4:  case 16:  case 17:   // S_NOP,S_SETHALT,S_MONITOR_SLEEP,S_TRAP,S_ROUND_MODE,
            case 18:  case 20:  case 53:  case 56:   // S_DENORM_MODE,S_BARRIER_WAIT,S_SETPRIO,S_INCPERFLEVEL,
            case 57:  case 59:  case 62:  case 64:   // S_DECPERFLEVEL,S_TTRACEDATA_IMM,S_SETPRIO_INC_WG,S_WAIT_LOADCNT,
            case 65:  case 69:  case 70:  case 71:   // S_WAIT_STORECNT,S_WAIT_XCNT,S_WAIT_DSCNT,S_WAIT_KMCNT,
            case 74:  case 75:   // S_WAIT_ASYNCCNT,S_WAIT_TENSORCNT,
                appendOPR_SIMM16(layout.SIMM16,true,false);
                break;
            case 3:   // S_SLEEP,
                appendOPR_SLEEP(layout.SIMM16,true,false);
                break;
            case 5:   // S_CLAUSE,
                appendOPR_CLAUSE(layout.SIMM16,true,false);
                break;
            case 6:   // S_SET_VGPR_MSB,
                appendOPR_SET_VGPR_MSB(layout.SIMM16,true,false);
                break;
            case 7:   // S_DELAY_ALU,
                appendOPR_DELAY(layout.SIMM16,true,false);
                break;
            case 8:   // S_WAIT_ALU,
                appendOPR_WAIT_ALU(layout.SIMM16,true,false);
                break;
            case 10:  case 31:  case 48:  case 49:  case 52:   // S_WAIT_IDLE,S_CODE_END,S_ENDPGM,S_ENDPGM_SAVED,S_WAKEUP,
            case 60:   // S_ICACHE_INV,
                break;
            case 21:   // S_BARRIER_LEAVE,
                appendOPR_SSRC_SPECIAL_SCC(253,false,true,1,true);
                break;
            case 32:   // S_BRANCH,
                setBranch();
                makeBranchTarget(isCall,isConditional,layout.SIMM16);
                break;
            case 33:  case 34:   // S_CBRANCH_SCC0,S_CBRANCH_SCC1,
                setBranch();
                setConditionalBranch();
                makeBranchTarget(isCall,isConditional,layout.SIMM16);
                appendOPR_SSRC_SPECIAL_SCC(253,true,false,1,true);
                break;
            case 35:  case 36:   // S_CBRANCH_VCCZ,S_CBRANCH_VCCNZ,
                setBranch();
                setConditionalBranch();
                makeBranchTarget(isCall,isConditional,layout.SIMM16);
                appendOPR_VCC(106,true,false,1,true);
                break;
            case 37:  case 38:   // S_CBRANCH_EXECZ,S_CBRANCH_EXECNZ,
                setBranch();
                setConditionalBranch();
                makeBranchTarget(isCall,isConditional,layout.SIMM16);
                appendOPR_SDST_EXEC(126,true,false,1,true);
                break;
            case 54:  case 55:   // S_SENDMSG,S_SENDMSGHALT,
                appendOPR_SENDMSG(layout.SIMM16,true,false);
                appendOPR_SDST_M0(125,true,false,1,true);
                break;
            case 58:   // S_TTRACEDATA,
                appendOPR_SDST_M0(125,true,false,1,true);
                break;
            case 72:  case 73:   // S_WAIT_LOADCNT_DSCNT,S_WAIT_STORECNT_DSCNT,
                appendOPR_WAIT_MEM_DS(layout.SIMM16,true,false);
                break;
        }
    }

    void InstructionDecoder_amdgpu_gfx1250::finalizeENC_VBUFFEROperands()
    {
        layout_ENC_VBUFFER & layout = insn_layout.ENC_VBUFFER;
        switch (layout.OP)  {
            case 16:  case 17:  case 18:  case 19:   // BUFFER_LOAD_U8,BUFFER_LOAD_I8,BUFFER_LOAD_U16,BUFFER_LOAD_I16,
            case 20:  case 30:  case 31:  case 32:   // BUFFER_LOAD_B32,BUFFER_LOAD_D16_U8,BUFFER_LOAD_D16_I8,BUFFER_LOAD_D16_B16,
            case 33:  case 34:  case 35:   // BUFFER_LOAD_D16_HI_U8,BUFFER_LOAD_D16_HI_I8,BUFFER_LOAD_D16_HI_B16,
                appendOPR_VGPR(layout.VDATA,false,true);
                if (layout.IDXEN && layout.OFFEN)  {
                    appendOPR_VGPR(layout.VADDR,true,false,2);
                } else if (layout.IDXEN || layout.OFFEN)  {
                    appendOPR_VGPR(layout.VADDR,true,false);
                }
                appendOPR_SREG(layout.RSRC,true,false,4);
                if (layout.SOFFSET != 124)  {
                    appendOPR_SREG_M0(layout.SOFFSET,true,false);
                }
                appendOPR_GPUMEM(0,true,false,1,true);
                break;
            case 21:   // BUFFER_LOAD_B64,
                appendOPR_VGPR(layout.VDATA,false,true,2);
                if (layout.IDXEN && layout.OFFEN)  {
                    appendOPR_VGPR(layout.VADDR,true,false,2);
                } else if (layout.IDXEN || layout.OFFEN)  {
                    appendOPR_VGPR(layout.VADDR,true,false);
                }
                appendOPR_SREG(layout.RSRC,true,false,4);
                if (layout.SOFFSET != 124)  {
                    appendOPR_SREG_M0(layout.SOFFSET,true,false);
                }
                appendOPR_GPUMEM(0,true,false,2,true);
                break;
            case 22:   // BUFFER_LOAD_B96,
                appendOPR_VGPR(layout.VDATA,false,true,3);
                if (layout.IDXEN && layout.OFFEN)  {
                    appendOPR_VGPR(layout.VADDR,true,false,2);
                } else if (layout.IDXEN || layout.OFFEN)  {
                    appendOPR_VGPR(layout.VADDR,true,false);
                }
                appendOPR_SREG(layout.RSRC,true,false,4);
                if (layout.SOFFSET != 124)  {
                    appendOPR_SREG_M0(layout.SOFFSET,true,false);
                }
                appendOPR_GPUMEM(0,true,false,3,true);
                break;
            case 23:   // BUFFER_LOAD_B128,
                appendOPR_VGPR(layout.VDATA,false,true,4);
                if (layout.IDXEN && layout.OFFEN)  {
                    appendOPR_VGPR(layout.VADDR,true,false,2);
                } else if (layout.IDXEN || layout.OFFEN)  {
                    appendOPR_VGPR(layout.VADDR,true,false);
                }
                appendOPR_SREG(layout.RSRC,true,false,4);
                if (layout.SOFFSET != 124)  {
                    appendOPR_SREG_M0(layout.SOFFSET,true,false);
                }
                appendOPR_GPUMEM(0,true,false,4,true);
                break;
            case 24:  case 25:  case 26:  case 36:   // BUFFER_STORE_B8,BUFFER_STORE_B16,BUFFER_STORE_B32,BUFFER_STORE_D16_HI_B8,
            case 37:   // BUFFER_STORE_D16_HI_B16,
                appendOPR_VGPR(layout.VDATA,true,false);
                if (layout.IDXEN && layout.OFFEN)  {
                    appendOPR_VGPR(layout.VADDR,true,false,2);
                } else if (layout.IDXEN || layout.OFFEN)  {
                    appendOPR_VGPR(layout.VADDR,true,false);
                }
                appendOPR_SREG(layout.RSRC,true,false,4);
                if (layout.SOFFSET != 124)  {
                    appendOPR_SREG_M0(layout.SOFFSET,true,false);
                }
                appendOPR_GPUMEM(0,false,true,1,true);
                break;
            case 27:   // BUFFER_STORE_B64,
                appendOPR_VGPR(layout.VDATA,true,false,2);
                if (layout.IDXEN && layout.OFFEN)  {
                    appendOPR_VGPR(layout.VADDR,true,false,2);
                } else if (layout.IDXEN || layout.OFFEN)  {
                    appendOPR_VGPR(layout.VADDR,true,false);
                }
                appendOPR_SREG(layout.RSRC,true,false,4);
                if (layout.SOFFSET != 124)  {
                    appendOPR_SREG_M0(layout.SOFFSET,true,false);
                }
                appendOPR_GPUMEM(0,false,true,2,true);
                break;
            case 28:   // BUFFER_STORE_B96,
                appendOPR_VGPR(layout.VDATA,true,false,3);
                if (layout.IDXEN && layout.OFFEN)  {
                    appendOPR_VGPR(layout.VADDR,true,false,2);
                } else if (layout.IDXEN || layout.OFFEN)  {
                    appendOPR_VGPR(layout.VADDR,true,false);
                }
                appendOPR_SREG(layout.RSRC,true,false,4);
                if (layout.SOFFSET != 124)  {
                    appendOPR_SREG_M0(layout.SOFFSET,true,false);
                }
                appendOPR_GPUMEM(0,false,true,3,true);
                break;
            case 29:   // BUFFER_STORE_B128,
                appendOPR_VGPR(layout.VDATA,true,false,4);
                if (layout.IDXEN && layout.OFFEN)  {
                    appendOPR_VGPR(layout.VADDR,true,false,2);
                } else if (layout.IDXEN || layout.OFFEN)  {
                    appendOPR_VGPR(layout.VADDR,true,false);
                }
                appendOPR_SREG(layout.RSRC,true,false,4);
                if (layout.SOFFSET != 124)  {
                    appendOPR_SREG_M0(layout.SOFFSET,true,false);
                }
                appendOPR_GPUMEM(0,false,true,4,true);
                break;
            case 51:  case 53:  case 54:   // BUFFER_ATOMIC_SWAP_B32,BUFFER_ATOMIC_ADD_U32,BUFFER_ATOMIC_SUB_U32,
            case 55:  case 56:  case 57:   // BUFFER_ATOMIC_SUB_CLAMP_U32,BUFFER_ATOMIC_MIN_I32,BUFFER_ATOMIC_MIN_U32,
            case 58:  case 59:  case 60:   // BUFFER_ATOMIC_MAX_I32,BUFFER_ATOMIC_MAX_U32,BUFFER_ATOMIC_AND_B32,
            case 61:  case 62:  case 63:   // BUFFER_ATOMIC_OR_B32,BUFFER_ATOMIC_XOR_B32,BUFFER_ATOMIC_INC_U32,
            case 64:  case 80:  case 81:   // BUFFER_ATOMIC_DEC_U32,BUFFER_ATOMIC_COND_SUB_U32,BUFFER_ATOMIC_MIN_NUM_F32,
            case 82:  case 86:  case 89:   // BUFFER_ATOMIC_MAX_NUM_F32,BUFFER_ATOMIC_ADD_F32,BUFFER_ATOMIC_PK_ADD_F16,
            case 90:   // BUFFER_ATOMIC_PK_ADD_BF16,
                appendOPR_VGPR(layout.VDATA,false,true);
                if (layout.IDXEN && layout.OFFEN)  {
                    appendOPR_VGPR(layout.VADDR,true,false,2);
                } else if (layout.IDXEN || layout.OFFEN)  {
                    appendOPR_VGPR(layout.VADDR,true,false);
                }
                appendOPR_SREG(layout.RSRC,true,false,4);
                if (layout.SOFFSET != 124)  {
                    appendOPR_SREG_M0(layout.SOFFSET,true,false);
                }
                appendOPR_GPUMEM(0,false,true,1,true);
                appendOPR_GPUMEM(0,true,false,1,true);
                break;
            case 52:   // BUFFER_ATOMIC_CMPSWAP_B32,
                appendOPR_VGPR(layout.VDATA,false,true,2);
                if (layout.IDXEN && layout.OFFEN)  {
                    appendOPR_VGPR(layout.VADDR,true,false,2);
                } else if (layout.IDXEN || layout.OFFEN)  {
                    appendOPR_VGPR(layout.VADDR,true,false);
                }
                appendOPR_SREG(layout.RSRC,true,false,4);
                if (layout.SOFFSET != 124)  {
                    appendOPR_SREG_M0(layout.SOFFSET,true,false);
                }
                appendOPR_GPUMEM(0,false,true,1,true);
                appendOPR_GPUMEM(0,true,false,1,true);
                break;
            case 65:  case 67:  case 68:   // BUFFER_ATOMIC_SWAP_B64,BUFFER_ATOMIC_ADD_U64,BUFFER_ATOMIC_SUB_U64,
            case 69:  case 70:  case 71:   // BUFFER_ATOMIC_MIN_I64,BUFFER_ATOMIC_MIN_U64,BUFFER_ATOMIC_MAX_I64,
            case 72:  case 73:  case 74:   // BUFFER_ATOMIC_MAX_U64,BUFFER_ATOMIC_AND_B64,BUFFER_ATOMIC_OR_B64,
            case 75:  case 76:  case 77:   // BUFFER_ATOMIC_XOR_B64,BUFFER_ATOMIC_INC_U64,BUFFER_ATOMIC_DEC_U64,
            case 85:  case 91:  case 92:   // BUFFER_ATOMIC_ADD_F64,BUFFER_ATOMIC_MIN_NUM_F64,BUFFER_ATOMIC_MAX_NUM_F64,
                appendOPR_VGPR(layout.VDATA,false,true,2);
                if (layout.IDXEN && layout.OFFEN)  {
                    appendOPR_VGPR(layout.VADDR,true,false,2);
                } else if (layout.IDXEN || layout.OFFEN)  {
                    appendOPR_VGPR(layout.VADDR,true,false);
                }
                appendOPR_SREG(layout.RSRC,true,false,4);
                if (layout.SOFFSET != 124)  {
                    appendOPR_SREG_M0(layout.SOFFSET,true,false);
                }
                appendOPR_GPUMEM(0,false,true,2,true);
                appendOPR_GPUMEM(0,true,false,2,true);
                break;
            case 66:   // BUFFER_ATOMIC_CMPSWAP_B64,
                appendOPR_VGPR(layout.VDATA,false,true,4);
                if (layout.IDXEN && layout.OFFEN)  {
                    appendOPR_VGPR(layout.VADDR,true,false,2);
                } else if (layout.IDXEN || layout.OFFEN)  {
                    appendOPR_VGPR(layout.VADDR,true,false);
                }
                appendOPR_SREG(layout.RSRC,true,false,4);
                if (layout.SOFFSET != 124)  {
                    appendOPR_SREG_M0(layout.SOFFSET,true,false);
                }
                appendOPR_GPUMEM(0,false,true,2,true);
                appendOPR_GPUMEM(0,true,false,2,true);
                break;
        }
    }

    void InstructionDecoder_amdgpu_gfx1250::finalizeENC_VDSOperands()
    {
        layout_ENC_VDS & layout = insn_layout.ENC_VDS;
        switch (layout.OP)  {
            case 0:  case 1:  case 2:  case 3:  case 4:   // DS_ADD_U32,DS_SUB_U32,DS_RSUB_U32,DS_INC_U32,DS_DEC_U32,
            case 5:  case 6:  case 7:  case 8:  case 9:   // DS_MIN_I32,DS_MAX_I32,DS_MIN_U32,DS_MAX_U32,DS_AND_B32,
            case 10:  case 11:  case 18:  case 19:  case 21:   // DS_OR_B32,DS_XOR_B32,DS_MIN_NUM_F32,DS_MAX_NUM_F32,DS_ADD_F32,
            case 152:  case 153:  case 154:  case 155:   // DS_COND_SUB_U32,DS_SUB_CLAMP_U32,DS_PK_ADD_F16,DS_PK_ADD_BF16,
                appendOPR_VGPR(layout.ADDR,true,false);
                appendOPR_VGPR(layout.DATA0,true,false);
                appendOPR_DSMEM(0,false,true,1,true);
                appendOPR_DSMEM(0,true,false,1,true);
                break;
            case 12:  case 16:   // DS_MSKOR_B32,DS_CMPSTORE_B32,
                appendOPR_VGPR(layout.ADDR,true,false);
                appendOPR_VGPR(layout.DATA0,true,false);
                appendOPR_VGPR(layout.DATA1,true,false);
                appendOPR_DSMEM(0,false,true,1,true);
                appendOPR_DSMEM(0,true,false,1,true);
                break;
            case 13:  case 30:  case 31:  case 160:   // DS_STORE_B32,DS_STORE_B8,DS_STORE_B16,DS_STORE_B8_D16_HI,
            case 161:   // DS_STORE_B16_D16_HI,
                appendOPR_VGPR(layout.ADDR,true,false);
                appendOPR_VGPR(layout.DATA0,true,false);
                appendOPR_DSMEM(0,false,true,1,true);
                break;
            case 14:  case 15:   // DS_STORE_2ADDR_B32,DS_STORE_2ADDR_STRIDE64_B32,
                appendOPR_VGPR(layout.ADDR,true,false);
                appendOPR_VGPR(layout.DATA0,true,false);
                appendOPR_VGPR(layout.DATA1,true,false);
                appendOPR_DSMEM(0,false,true,1,true);
                break;
            case 20:   // DS_NOP,
                break;
            case 32:  case 33:  case 34:  case 35:   // DS_ADD_RTN_U32,DS_SUB_RTN_U32,DS_RSUB_RTN_U32,DS_INC_RTN_U32,
            case 36:  case 37:  case 38:  case 39:   // DS_DEC_RTN_U32,DS_MIN_RTN_I32,DS_MAX_RTN_I32,DS_MIN_RTN_U32,
            case 40:  case 41:  case 42:  case 43:   // DS_MAX_RTN_U32,DS_AND_RTN_B32,DS_OR_RTN_B32,DS_XOR_RTN_B32,
            case 45:  case 50:  case 51:  case 121:   // DS_STOREXCHG_RTN_B32,DS_MIN_NUM_RTN_F32,DS_MAX_NUM_RTN_F32,DS_ADD_RTN_F32,
            case 168:  case 169:  case 170:   // DS_COND_SUB_RTN_U32,DS_SUB_CLAMP_RTN_U32,DS_PK_ADD_RTN_F16,
            case 171:   // DS_PK_ADD_RTN_BF16,
                appendOPR_VGPR(layout.VDST,false,true);
                appendOPR_VGPR(layout.ADDR,true,false);
                appendOPR_VGPR(layout.DATA0,true,false);
                appendOPR_DSMEM(0,false,true,1,true);
                appendOPR_DSMEM(0,true,false,1,true);
                break;
            case 44:  case 48:   // DS_MSKOR_RTN_B32,DS_CMPSTORE_RTN_B32,
                appendOPR_VGPR(layout.VDST,false,true);
                appendOPR_VGPR(layout.ADDR,true,false);
                appendOPR_VGPR(layout.DATA0,true,false);
                appendOPR_VGPR(layout.DATA1,true,false);
                appendOPR_DSMEM(0,false,true,1,true);
                appendOPR_DSMEM(0,true,false,1,true);
                break;
            case 46:  case 47:   // DS_STOREXCHG_2ADDR_RTN_B32,DS_STOREXCHG_2ADDR_STRIDE64_RTN_B32,
                appendOPR_VGPR(layout.VDST,false,true,2);
                appendOPR_VGPR(layout.ADDR,true,false);
                appendOPR_VGPR(layout.DATA0,true,false);
                appendOPR_VGPR(layout.DATA1,true,false);
                appendOPR_DSMEM(0,false,true,1,true);
                appendOPR_DSMEM(0,true,false,1,true);
                break;
            case 53:   // DS_SWIZZLE_B32,
                appendOPR_VGPR(layout.VDST,false,true);
                appendOPR_VGPR(layout.ADDR,true,false);
                break;
            case 54:  case 57:  case 58:  case 59:  case 60:   // DS_LOAD_B32,DS_LOAD_I8,DS_LOAD_U8,DS_LOAD_I16,DS_LOAD_U16,
            case 162:  case 163:  case 164:  case 165:   // DS_LOAD_U8_D16,DS_LOAD_U8_D16_HI,DS_LOAD_I8_D16,DS_LOAD_I8_D16_HI,
            case 166:  case 167:   // DS_LOAD_U16_D16,DS_LOAD_U16_D16_HI,
                appendOPR_VGPR(layout.VDST,false,true);
                appendOPR_VGPR(layout.ADDR,true,false);
                appendOPR_DSMEM(0,true,false,1,true);
                break;
            case 55:  case 56:   // DS_LOAD_2ADDR_B32,DS_LOAD_2ADDR_STRIDE64_B32,
                appendOPR_VGPR(layout.VDST,false,true,2);
                appendOPR_VGPR(layout.ADDR,true,false);
                appendOPR_DSMEM(0,true,false,1,true);
                break;
            case 61:  case 62:   // DS_CONSUME,DS_APPEND,
                appendOPR_VGPR(layout.VDST,false,true);
                appendOPR_DSMEM(0,false,true,1,true);
                appendOPR_DSMEM(0,true,false,1,true);
                break;
            case 64:  case 65:  case 66:  case 67:  case 68:   // DS_ADD_U64,DS_SUB_U64,DS_RSUB_U64,DS_INC_U64,DS_DEC_U64,
            case 69:  case 70:  case 71:  case 72:  case 73:   // DS_MIN_I64,DS_MAX_I64,DS_MIN_U64,DS_MAX_U64,DS_AND_B64,
            case 74:  case 75:  case 82:  case 83:  case 84:   // DS_OR_B64,DS_XOR_B64,DS_MIN_NUM_F64,DS_MAX_NUM_F64,DS_ADD_F64,
                appendOPR_VGPR(layout.ADDR,true,false);
                appendOPR_VGPR(layout.DATA0,true,false,2);
                appendOPR_DSMEM(0,false,true,2,true);
                appendOPR_DSMEM(0,true,false,2,true);
                break;
            case 76:  case 80:   // DS_MSKOR_B64,DS_CMPSTORE_B64,
                appendOPR_VGPR(layout.ADDR,true,false);
                appendOPR_VGPR(layout.DATA0,true,false,2);
                appendOPR_VGPR(layout.DATA1,true,false,2);
                appendOPR_DSMEM(0,false,true,2,true);
                appendOPR_DSMEM(0,true,false,2,true);
                break;
            case 77:   // DS_STORE_B64,
                appendOPR_VGPR(layout.ADDR,true,false);
                appendOPR_VGPR(layout.DATA0,true,false,2);
                appendOPR_DSMEM(0,false,true,2,true);
                break;
            case 78:  case 79:   // DS_STORE_2ADDR_B64,DS_STORE_2ADDR_STRIDE64_B64,
                appendOPR_VGPR(layout.ADDR,true,false);
                appendOPR_VGPR(layout.DATA0,true,false,2);
                appendOPR_VGPR(layout.DATA1,true,false,2);
                appendOPR_DSMEM(0,false,true,2,true);
                break;
            case 86:   // DS_ATOMIC_ASYNC_BARRIER_ARRIVE_B64,
                appendOPR_VGPR(layout.ADDR,true,false);
                appendOPR_DSMEM(0,false,true,2,true);
                appendOPR_DSMEM(0,true,false,2,true);
                break;
            case 96:  case 97:  case 98:  case 99:   // DS_ADD_RTN_U64,DS_SUB_RTN_U64,DS_RSUB_RTN_U64,DS_INC_RTN_U64,
            case 100:  case 101:  case 102:  case 103:   // DS_DEC_RTN_U64,DS_MIN_RTN_I64,DS_MAX_RTN_I64,DS_MIN_RTN_U64,
            case 104:  case 105:  case 106:  case 107:   // DS_MAX_RTN_U64,DS_AND_RTN_B64,DS_OR_RTN_B64,DS_XOR_RTN_B64,
            case 109:  case 114:  case 115:  case 116:   // DS_STOREXCHG_RTN_B64,DS_MIN_NUM_RTN_F64,DS_MAX_NUM_RTN_F64,DS_ADD_RTN_F64,
            case 117:  case 126:   // DS_ATOMIC_BARRIER_ARRIVE_RTN_B64,DS_CONDXCHG32_RTN_B64,
                appendOPR_VGPR(layout.VDST,false,true,2);
                appendOPR_VGPR(layout.ADDR,true,false);
                appendOPR_VGPR(layout.DATA0,true,false,2);
                appendOPR_DSMEM(0,false,true,2,true);
                appendOPR_DSMEM(0,true,false,2,true);
                break;
            case 108:  case 112:   // DS_MSKOR_RTN_B64,DS_CMPSTORE_RTN_B64,
                appendOPR_VGPR(layout.VDST,false,true,2);
                appendOPR_VGPR(layout.ADDR,true,false);
                appendOPR_VGPR(layout.DATA0,true,false,2);
                appendOPR_VGPR(layout.DATA1,true,false,2);
                appendOPR_DSMEM(0,false,true,2,true);
                appendOPR_DSMEM(0,true,false,2,true);
                break;
            case 110:  case 111:   // DS_STOREXCHG_2ADDR_RTN_B64,DS_STOREXCHG_2ADDR_STRIDE64_RTN_B64,
                appendOPR_VGPR(layout.VDST,false,true,4);
                appendOPR_VGPR(layout.ADDR,true,false);
                appendOPR_VGPR(layout.DATA0,true,false,2);
                appendOPR_VGPR(layout.DATA1,true,false,2);
                appendOPR_DSMEM(0,false,true,2,true);
                appendOPR_DSMEM(0,true,false,2,true);
                break;
            case 118:  case 250:  case 253:   // DS_LOAD_B64,DS_LOAD_TR4_B64,DS_LOAD_TR8_B64,
                appendOPR_VGPR(layout.VDST,false,true,2);
                appendOPR_VGPR(layout.ADDR,true,false);
                appendOPR_DSMEM(0,true,false,2,true);
                break;
            case 119:  case 120:   // DS_LOAD_2ADDR_B64,DS_LOAD_2ADDR_STRIDE64_B64,
                appendOPR_VGPR(layout.VDST,false,true,4);
                appendOPR_VGPR(layout.ADDR,true,false);
                appendOPR_DSMEM(0,true,false,2,true);
                break;
            case 176:   // DS_STORE_ADDTID_B32,
                appendOPR_VGPR(layout.DATA0,true,false);
                appendOPR_DSMEM(0,false,true,1,true);
                appendOPR_SDST_M0(125,true,false,1,true);
                break;
            case 177:   // DS_LOAD_ADDTID_B32,
                appendOPR_VGPR(layout.VDST,false,true);
                appendOPR_DSMEM(0,true,false,1,true);
                appendOPR_SDST_M0(125,true,false,1,true);
                break;
            case 178:  case 179:  case 205:   // DS_PERMUTE_B32,DS_BPERMUTE_B32,DS_BPERMUTE_FI_B32,
                appendOPR_VGPR(layout.VDST,false,true);
                appendOPR_VGPR(layout.ADDR,true,false);
                appendOPR_VGPR(layout.DATA0,true,false);
                break;
            case 222:   // DS_STORE_B96,
                appendOPR_VGPR(layout.ADDR,true,false);
                appendOPR_VGPR(layout.DATA0,true,false,3);
                appendOPR_DSMEM(0,false,true,3,true);
                break;
            case 223:   // DS_STORE_B128,
                appendOPR_VGPR(layout.ADDR,true,false);
                appendOPR_VGPR(layout.DATA0,true,false,4);
                appendOPR_DSMEM(0,false,true,4,true);
                break;
            case 251:  case 254:   // DS_LOAD_TR6_B96,DS_LOAD_B96,
                appendOPR_VGPR(layout.VDST,false,true,3);
                appendOPR_VGPR(layout.ADDR,true,false);
                appendOPR_DSMEM(0,true,false,3,true);
                break;
            case 252:  case 255:   // DS_LOAD_TR16_B128,DS_LOAD_B128,
                appendOPR_VGPR(layout.VDST,false,true,4);
                appendOPR_VGPR(layout.ADDR,true,false);
                appendOPR_DSMEM(0,true,false,4,true);
                break;
        }
    }

    void InstructionDecoder_amdgpu_gfx1250::finalizeENC_VFLATOperands()
    {
        layout_ENC_VFLAT & layout = insn_layout.ENC_VFLAT;
        switch (layout.OP)  {
            case 16:  case 17:  case 18:  case 19:  case 20:   // FLAT_LOAD_U8,FLAT_LOAD_I8,FLAT_LOAD_U16,FLAT_LOAD_I16,FLAT_LOAD_B32,
            case 30:  case 31:  case 32:  case 33:   // FLAT_LOAD_D16_U8,FLAT_LOAD_D16_I8,FLAT_LOAD_D16_B16,FLAT_LOAD_D16_HI_U8,
            case 34:  case 35:  case 112:   // FLAT_LOAD_D16_HI_I8,FLAT_LOAD_D16_HI_B16,FLAT_LOAD_MONITOR_B32,
                appendOPR_VGPR(layout.VDST,false,true);
                if (layout.SADDR == 124)  {
                    appendOPR_VGPR(layout.VADDR,true,false,2);
                } else  {
                    appendOPR_VGPR(layout.VADDR,true,false);
                }
                if (layout.SADDR != 124)  {
                    appendOPR_SREG(layout.SADDR,true,false,2);
                }
                appendOPR_GPUMEM(0,true,false,1,true);
                break;
            case 21:  case 113:   // FLAT_LOAD_B64,FLAT_LOAD_MONITOR_B64,
                appendOPR_VGPR(layout.VDST,false,true,2);
                if (layout.SADDR == 124)  {
                    appendOPR_VGPR(layout.VADDR,true,false,2);
                } else  {
                    appendOPR_VGPR(layout.VADDR,true,false);
                }
                if (layout.SADDR != 124)  {
                    appendOPR_SREG(layout.SADDR,true,false,2);
                }
                appendOPR_GPUMEM(0,true,false,2,true);
                break;
            case 22:   // FLAT_LOAD_B96,
                appendOPR_VGPR(layout.VDST,false,true,3);
                if (layout.SADDR == 124)  {
                    appendOPR_VGPR(layout.VADDR,true,false,2);
                } else  {
                    appendOPR_VGPR(layout.VADDR,true,false);
                }
                if (layout.SADDR != 124)  {
                    appendOPR_SREG(layout.SADDR,true,false,2);
                }
                appendOPR_GPUMEM(0,true,false,3,true);
                break;
            case 23:  case 114:   // FLAT_LOAD_B128,FLAT_LOAD_MONITOR_B128,
                appendOPR_VGPR(layout.VDST,false,true,4);
                if (layout.SADDR == 124)  {
                    appendOPR_VGPR(layout.VADDR,true,false,2);
                } else  {
                    appendOPR_VGPR(layout.VADDR,true,false);
                }
                if (layout.SADDR != 124)  {
                    appendOPR_SREG(layout.SADDR,true,false,2);
                }
                appendOPR_GPUMEM(0,true,false,4,true);
                break;
            case 24:  case 25:  case 26:  case 36:   // FLAT_STORE_B8,FLAT_STORE_B16,FLAT_STORE_B32,FLAT_STORE_D16_HI_B8,
            case 37:   // FLAT_STORE_D16_HI_B16,
                if (layout.SADDR == 124)  {
                    appendOPR_VGPR(layout.VADDR,true,false,2);
                } else  {
                    appendOPR_VGPR(layout.VADDR,true,false);
                }
                appendOPR_VGPR(layout.VSRC,true,false);
                if (layout.SADDR != 124)  {
                    appendOPR_SREG(layout.SADDR,true,false,2);
                }
                appendOPR_GPUMEM(0,false,true,1,true);
                break;
            case 27:   // FLAT_STORE_B64,
                if (layout.SADDR == 124)  {
                    appendOPR_VGPR(layout.VADDR,true,false,2);
                } else  {
                    appendOPR_VGPR(layout.VADDR,true,false);
                }
                appendOPR_VGPR(layout.VSRC,true,false,2);
                if (layout.SADDR != 124)  {
                    appendOPR_SREG(layout.SADDR,true,false,2);
                }
                appendOPR_GPUMEM(0,false,true,2,true);
                break;
            case 28:   // FLAT_STORE_B96,
                if (layout.SADDR == 124)  {
                    appendOPR_VGPR(layout.VADDR,true,false,2);
                } else  {
                    appendOPR_VGPR(layout.VADDR,true,false);
                }
                appendOPR_VGPR(layout.VSRC,true,false,3);
                if (layout.SADDR != 124)  {
                    appendOPR_SREG(layout.SADDR,true,false,2);
                }
                appendOPR_GPUMEM(0,false,true,3,true);
                break;
            case 29:   // FLAT_STORE_B128,
                if (layout.SADDR == 124)  {
                    appendOPR_VGPR(layout.VADDR,true,false,2);
                } else  {
                    appendOPR_VGPR(layout.VADDR,true,false);
                }
                appendOPR_VGPR(layout.VSRC,true,false,4);
                if (layout.SADDR != 124)  {
                    appendOPR_SREG(layout.SADDR,true,false,2);
                }
                appendOPR_GPUMEM(0,false,true,4,true);
                break;
            case 51:  case 53:  case 54:   // FLAT_ATOMIC_SWAP_B32,FLAT_ATOMIC_ADD_U32,FLAT_ATOMIC_SUB_U32,
            case 55:  case 56:  case 57:   // FLAT_ATOMIC_SUB_CLAMP_U32,FLAT_ATOMIC_MIN_I32,FLAT_ATOMIC_MIN_U32,
            case 58:  case 59:  case 60:   // FLAT_ATOMIC_MAX_I32,FLAT_ATOMIC_MAX_U32,FLAT_ATOMIC_AND_B32,
            case 61:  case 62:  case 63:   // FLAT_ATOMIC_OR_B32,FLAT_ATOMIC_XOR_B32,FLAT_ATOMIC_INC_U32,
            case 64:  case 80:  case 81:   // FLAT_ATOMIC_DEC_U32,FLAT_ATOMIC_COND_SUB_U32,FLAT_ATOMIC_MIN_NUM_F32,
            case 82:  case 86:  case 89:   // FLAT_ATOMIC_MAX_NUM_F32,FLAT_ATOMIC_ADD_F32,FLAT_ATOMIC_PK_ADD_F16,
            case 90:   // FLAT_ATOMIC_PK_ADD_BF16,
                appendOPR_VGPR(layout.VDST,false,true);
                if (layout.SADDR == 124)  {
                    appendOPR_VGPR(layout.VADDR,true,false,2);
                } else  {
                    appendOPR_VGPR(layout.VADDR,true,false);
                }
                appendOPR_VGPR(layout.VSRC,true,false);
                if (layout.SADDR != 124)  {
                    appendOPR_SREG(layout.SADDR,true,false,2);
                }
                appendOPR_GPUMEM(0,false,true,1,true);
                appendOPR_GPUMEM(0,true,false,1,true);
                break;
            case 52:   // FLAT_ATOMIC_CMPSWAP_B32,
                appendOPR_VGPR(layout.VDST,false,true);
                if (layout.SADDR == 124)  {
                    appendOPR_VGPR(layout.VADDR,true,false,2);
                } else  {
                    appendOPR_VGPR(layout.VADDR,true,false);
                }
                appendOPR_VGPR(layout.VSRC,true,false,2);
                if (layout.SADDR != 124)  {
                    appendOPR_SREG(layout.SADDR,true,false,2);
                }
                appendOPR_GPUMEM(0,false,true,1,true);
                appendOPR_GPUMEM(0,true,false,1,true);
                break;
            case 65:  case 67:  case 68:   // FLAT_ATOMIC_SWAP_B64,FLAT_ATOMIC_ADD_U64,FLAT_ATOMIC_SUB_U64,
            case 69:  case 70:  case 71:   // FLAT_ATOMIC_MIN_I64,FLAT_ATOMIC_MIN_U64,FLAT_ATOMIC_MAX_I64,
            case 72:  case 73:  case 74:   // FLAT_ATOMIC_MAX_U64,FLAT_ATOMIC_AND_B64,FLAT_ATOMIC_OR_B64,
            case 75:  case 76:  case 77:   // FLAT_ATOMIC_XOR_B64,FLAT_ATOMIC_INC_U64,FLAT_ATOMIC_DEC_U64,
            case 85:  case 91:  case 92:   // FLAT_ATOMIC_ADD_F64,FLAT_ATOMIC_MIN_NUM_F64,FLAT_ATOMIC_MAX_NUM_F64,
                appendOPR_VGPR(layout.VDST,false,true,2);
                if (layout.SADDR == 124)  {
                    appendOPR_VGPR(layout.VADDR,true,false,2);
                } else  {
                    appendOPR_VGPR(layout.VADDR,true,false);
                }
                appendOPR_VGPR(layout.VSRC,true,false,2);
                if (layout.SADDR != 124)  {
                    appendOPR_SREG(layout.SADDR,true,false,2);
                }
                appendOPR_GPUMEM(0,false,true,2,true);
                appendOPR_GPUMEM(0,true,false,2,true);
                break;
            case 66:   // FLAT_ATOMIC_CMPSWAP_B64,
                appendOPR_VGPR(layout.VDST,false,true,2);
                if (layout.SADDR == 124)  {
                    appendOPR_VGPR(layout.VADDR,true,false,2);
                } else  {
                    appendOPR_VGPR(layout.VADDR,true,false);
                }
                appendOPR_VGPR(layout.VSRC,true,false,4);
                if (layout.SADDR != 124)  {
                    appendOPR_SREG(layout.SADDR,true,false,2);
                }
                appendOPR_GPUMEM(0,false,true,2,true);
                appendOPR_GPUMEM(0,true,false,2,true);
                break;
            case 93:   // FLAT_PREFETCH_B8,
                if (layout.SADDR == 124)  {
                    appendOPR_VGPR(layout.VADDR,true,false,2);
                } else  {
                    appendOPR_VGPR(layout.VADDR,true,false);
                }
                if (layout.SADDR != 124)  {
                    appendOPR_SREG(layout.SADDR,true,false,2);
                }
                appendOPR_GPUMEM(0,true,false,1,true);
                break;
        }
    }

    void InstructionDecoder_amdgpu_gfx1250::finalizeENC_VGLOBALOperands()
    {
        layout_ENC_VGLOBAL & layout = insn_layout.ENC_VGLOBAL;
        switch (layout.OP)  {
            case 16:  case 17:  case 18:  case 19:   // GLOBAL_LOAD_U8,GLOBAL_LOAD_I8,GLOBAL_LOAD_U16,GLOBAL_LOAD_I16,
            case 20:  case 30:  case 31:  case 32:   // GLOBAL_LOAD_B32,GLOBAL_LOAD_D16_U8,GLOBAL_LOAD_D16_I8,GLOBAL_LOAD_D16_B16,
            case 33:  case 34:  case 35:   // GLOBAL_LOAD_D16_HI_U8,GLOBAL_LOAD_D16_HI_I8,GLOBAL_LOAD_D16_HI_B16,
            case 112:   // GLOBAL_LOAD_MONITOR_B32,
                appendOPR_VGPR(layout.VDST,false,true);
                if (layout.SADDR == 124)  {
                    appendOPR_VGPR(layout.VADDR,true,false,2);
                } else  {
                    appendOPR_VGPR(layout.VADDR,true,false);
                }
                if (layout.SADDR != 124)  {
                    appendOPR_SREG(layout.SADDR,true,false,2);
                }
                appendOPR_GPUMEM(0,true,false,1,true);
                break;
            case 21:  case 88:  case 113:   // GLOBAL_LOAD_B64,GLOBAL_LOAD_TR8_B64,GLOBAL_LOAD_MONITOR_B64,
            case 115:   // GLOBAL_LOAD_TR4_B64,
                appendOPR_VGPR(layout.VDST,false,true,2);
                if (layout.SADDR == 124)  {
                    appendOPR_VGPR(layout.VADDR,true,false,2);
                } else  {
                    appendOPR_VGPR(layout.VADDR,true,false);
                }
                if (layout.SADDR != 124)  {
                    appendOPR_SREG(layout.SADDR,true,false,2);
                }
                appendOPR_GPUMEM(0,true,false,2,true);
                break;
            case 22:  case 116:   // GLOBAL_LOAD_B96,GLOBAL_LOAD_TR6_B96,
                appendOPR_VGPR(layout.VDST,false,true,3);
                if (layout.SADDR == 124)  {
                    appendOPR_VGPR(layout.VADDR,true,false,2);
                } else  {
                    appendOPR_VGPR(layout.VADDR,true,false);
                }
                if (layout.SADDR != 124)  {
                    appendOPR_SREG(layout.SADDR,true,false,2);
                }
                appendOPR_GPUMEM(0,true,false,3,true);
                break;
            case 23:  case 87:  case 114:   // GLOBAL_LOAD_B128,GLOBAL_LOAD_TR16_B128,GLOBAL_LOAD_MONITOR_B128,
                appendOPR_VGPR(layout.VDST,false,true,4);
                if (layout.SADDR == 124)  {
                    appendOPR_VGPR(layout.VADDR,true,false,2);
                } else  {
                    appendOPR_VGPR(layout.VADDR,true,false);
                }
                if (layout.SADDR != 124)  {
                    appendOPR_SREG(layout.SADDR,true,false,2);
                }
                appendOPR_GPUMEM(0,true,false,4,true);
                break;
            case 24:  case 25:  case 26:  case 36:   // GLOBAL_STORE_B8,GLOBAL_STORE_B16,GLOBAL_STORE_B32,GLOBAL_STORE_D16_HI_B8,
            case 37:   // GLOBAL_STORE_D16_HI_B16,
                if (layout.SADDR == 124)  {
                    appendOPR_VGPR(layout.VADDR,true,false,2);
                } else  {
                    appendOPR_VGPR(layout.VADDR,true,false);
                }
                appendOPR_VGPR(layout.VSRC,true,false);
                if (layout.SADDR != 124)  {
                    appendOPR_SREG(layout.SADDR,true,false,2);
                }
                appendOPR_GPUMEM(0,false,true,1,true);
                break;
            case 27:   // GLOBAL_STORE_B64,
                if (layout.SADDR == 124)  {
                    appendOPR_VGPR(layout.VADDR,true,false,2);
                } else  {
                    appendOPR_VGPR(layout.VADDR,true,false);
                }
                appendOPR_VGPR(layout.VSRC,true,false,2);
                if (layout.SADDR != 124)  {
                    appendOPR_SREG(layout.SADDR,true,false,2);
                }
                appendOPR_GPUMEM(0,false,true,2,true);
                break;
            case 28:   // GLOBAL_STORE_B96,
                if (layout.SADDR == 124)  {
                    appendOPR_VGPR(layout.VADDR,true,false,2);
                } else  {
                    appendOPR_VGPR(layout.VADDR,true,false);
                }
                appendOPR_VGPR(layout.VSRC,true,false,3);
                if (layout.SADDR != 124)  {
                    appendOPR_SREG(layout.SADDR,true,false,2);
                }
                appendOPR_GPUMEM(0,false,true,3,true);
                break;
            case 29:   // GLOBAL_STORE_B128,
                if (layout.SADDR == 124)  {
                    appendOPR_VGPR(layout.VADDR,true,false,2);
                } else  {
                    appendOPR_VGPR(layout.VADDR,true,false);
                }
                appendOPR_VGPR(layout.VSRC,true,false,4);
                if (layout.SADDR != 124)  {
                    appendOPR_SREG(layout.SADDR,true,false,2);
                }
                appendOPR_GPUMEM(0,false,true,4,true);
                break;
            case 40:   // GLOBAL_LOAD_ADDTID_B32,
                appendOPR_VGPR(layout.VDST,false,true);
                if (layout.SADDR != 124)  {
                    appendOPR_SREG(layout.SADDR,true,false,2);
                }
                appendOPR_GPUMEM(0,true,false,1,true);
                break;
            case 41:   // GLOBAL_STORE_ADDTID_B32,
                appendOPR_VGPR(layout.VSRC,true,false);
                if (layout.SADDR != 124)  {
                    appendOPR_SREG(layout.SADDR,true,false,2);
                }
                appendOPR_GPUMEM(0,false,true,1,true);
                break;
            case 43:  case 44:  case 79:   // GLOBAL_INV,GLOBAL_WB,GLOBAL_WBINV,
                break;
            case 51:  case 53:  case 54:   // GLOBAL_ATOMIC_SWAP_B32,GLOBAL_ATOMIC_ADD_U32,GLOBAL_ATOMIC_SUB_U32,
            case 55:  case 56:  case 57:   // GLOBAL_ATOMIC_SUB_CLAMP_U32,GLOBAL_ATOMIC_MIN_I32,GLOBAL_ATOMIC_MIN_U32,
            case 58:  case 59:  case 60:   // GLOBAL_ATOMIC_MAX_I32,GLOBAL_ATOMIC_MAX_U32,GLOBAL_ATOMIC_AND_B32,
            case 61:  case 62:  case 63:   // GLOBAL_ATOMIC_OR_B32,GLOBAL_ATOMIC_XOR_B32,GLOBAL_ATOMIC_INC_U32,
            case 64:  case 80:  case 81:   // GLOBAL_ATOMIC_DEC_U32,GLOBAL_ATOMIC_COND_SUB_U32,GLOBAL_ATOMIC_MIN_NUM_F32,
            case 82:  case 86:  case 89:   // GLOBAL_ATOMIC_MAX_NUM_F32,GLOBAL_ATOMIC_ADD_F32,GLOBAL_ATOMIC_PK_ADD_F16,
            case 90:   // GLOBAL_ATOMIC_PK_ADD_BF16,
                appendOPR_VGPR(layout.VDST,false,true);
                if (layout.SADDR == 124)  {
                    appendOPR_VGPR(layout.VADDR,true,false,2);
                } else  {
                    appendOPR_VGPR(layout.VADDR,true,false);
                }
                appendOPR_VGPR(layout.VSRC,true,false);
                if (layout.SADDR != 124)  {
                    appendOPR_SREG(layout.SADDR,true,false,2);
                }
                appendOPR_GPUMEM(0,false,true,1,true);
                appendOPR_GPUMEM(0,true,false,1,true);
                break;
            case 52:   // GLOBAL_ATOMIC_CMPSWAP_B32,
                appendOPR_VGPR(layout.VDST,false,true);
                if (layout.SADDR == 124)  {
                    appendOPR_VGPR(layout.VADDR,true,false,2);
                } else  {
                    appendOPR_VGPR(layout.VADDR,true,false);
                }
                appendOPR_VGPR(layout.VSRC,true,false,2);
                if (layout.SADDR != 124)  {
                    appendOPR_SREG(layout.SADDR,true,false,2);
                }
                appendOPR_GPUMEM(0,false,true,1,true);
                appendOPR_GPUMEM(0,true,false,1,true);
                break;
            case 65:  case 67:  case 68:   // GLOBAL_ATOMIC_SWAP_B64,GLOBAL_ATOMIC_ADD_U64,GLOBAL_ATOMIC_SUB_U64,
            case 69:  case 70:  case 71:   // GLOBAL_ATOMIC_MIN_I64,GLOBAL_ATOMIC_MIN_U64,GLOBAL_ATOMIC_MAX_I64,
            case 72:  case 73:  case 74:   // GLOBAL_ATOMIC_MAX_U64,GLOBAL_ATOMIC_AND_B64,GLOBAL_ATOMIC_OR_B64,
            case 75:  case 76:  case 77:   // GLOBAL_ATOMIC_XOR_B64,GLOBAL_ATOMIC_INC_U64,GLOBAL_ATOMIC_DEC_U64,
            case 85:  case 91:  case 92:   // GLOBAL_ATOMIC_ADD_F64,GLOBAL_ATOMIC_MIN_NUM_F64,GLOBAL_ATOMIC_MAX_NUM_F64,
                appendOPR_VGPR(layout.VDST,false,true,2);
                if (layout.SADDR == 124)  {
                    appendOPR_VGPR(layout.VADDR,true,false,2);
                } else  {
                    appendOPR_VGPR(layout.VADDR,true,false);
                }
                appendOPR_VGPR(layout.VSRC,true,false,2);
                if (layout.SADDR != 124)  {
                    appendOPR_SREG(layout.SADDR,true,false,2);
                }
                appendOPR_GPUMEM(0,false,true,2,true);
                appendOPR_GPUMEM(0,true,false,2,true);
                break;
            case 66:   // GLOBAL_ATOMIC_CMPSWAP_B64,
                appendOPR_VGPR(layout.VDST,false,true,2);
                if (layout.SADDR == 124)  {
                    appendOPR_VGPR(layout.VADDR,true,false,2);
                } else  {
                    appendOPR_VGPR(layout.VADDR,true,false);
                }
                appendOPR_VGPR(layout.VSRC,true,false,4);
                if (layout.SADDR != 124)  {
                    appendOPR_SREG(layout.SADDR,true,false,2);
                }
                appendOPR_GPUMEM(0,false,true,2,true);
                appendOPR_GPUMEM(0,true,false,2,true);
                break;
            case 83:   // GLOBAL_LOAD_BLOCK,
                appendOPR_VGPR(layout.VDST,false,true,32);
                if (layout.SADDR == 124)  {
                    appendOPR_VGPR(layout.VADDR,true,false,2);
                } else  {
                    appendOPR_VGPR(layout.VADDR,true,false);
                }
                if (layout.SADDR != 124)  {
                    appendOPR_SREG(layout.SADDR,true,false,2);
                }
                appendOPR_GPUMEM(0,true,false,32,true);
                appendOPR_SDST_M0(125,true,false,1,true);
                break;
            case 84:   // GLOBAL_STORE_BLOCK,
                if (layout.SADDR == 124)  {
                    appendOPR_VGPR(layout.VADDR,true,false,2);
                } else  {
                    appendOPR_VGPR(layout.VADDR,true,false);
                }
                appendOPR_VGPR(layout.VSRC,true,false,32);
                if (layout.SADDR != 124)  {
                    appendOPR_SREG(layout.SADDR,true,false,2);
                }
                appendOPR_GPUMEM(0,false,true,32,true);
                appendOPR_SDST_M0(125,true,false,1,true);
                break;
            case 93:   // GLOBAL_PREFETCH_B8,
                if (layout.SADDR == 124)  {
                    appendOPR_VGPR(layout.VADDR,true,false,2);
                } else  {
                    appendOPR_VGPR(layout.VADDR,true,false);
                }
                if (layout.SADDR != 124)  {
                    appendOPR_SREG(layout.SADDR,true,false,2);
                }
                appendOPR_GPUMEM(0,true,false,1,true);
                break;
            case 95:  case 96:   // GLOBAL_LOAD_ASYNC_TO_LDS_B8,GLOBAL_LOAD_ASYNC_TO_LDS_B32,
                appendOPR_VGPR(layout.VDST,true,false);
                if (layout.SADDR == 124)  {
                    appendOPR_VGPR(layout.VADDR,true,false,2);
                } else  {
                    appendOPR_VGPR(layout.VADDR,true,false);
                }
                if (layout.SADDR != 124)  {
                    appendOPR_SREG(layout.SADDR,true,false,2);
                }
                appendOPR_DSMEM(0,false,true,1,true);
                appendOPR_GPUMEM(0,true,false,1,true);
                break;
            case 97:   // GLOBAL_LOAD_ASYNC_TO_LDS_B64,
                appendOPR_VGPR(layout.VDST,true,false);
                if (layout.SADDR == 124)  {
                    appendOPR_VGPR(layout.VADDR,true,false,2);
                } else  {
                    appendOPR_VGPR(layout.VADDR,true,false);
                }
                if (layout.SADDR != 124)  {
                    appendOPR_SREG(layout.SADDR,true,false,2);
                }
                appendOPR_DSMEM(0,false,true,2,true);
                appendOPR_GPUMEM(0,true,false,2,true);
                break;
            case 98:   // GLOBAL_LOAD_ASYNC_TO_LDS_B128,
                appendOPR_VGPR(layout.VDST,true,false);
                if (layout.SADDR == 124)  {
                    appendOPR_VGPR(layout.VADDR,true,false,2);
                } else  {
                    appendOPR_VGPR(layout.VADDR,true,false);
                }
                if (layout.SADDR != 124)  {
                    appendOPR_SREG(layout.SADDR,true,false,2);
                }
                appendOPR_DSMEM(0,false,true,4,true);
                appendOPR_GPUMEM(0,true,false,4,true);
                break;
            case 99:  case 100:   // GLOBAL_STORE_ASYNC_FROM_LDS_B8,GLOBAL_STORE_ASYNC_FROM_LDS_B32,
                if (layout.SADDR == 124)  {
                    appendOPR_VGPR(layout.VADDR,true,false,2);
                } else  {
                    appendOPR_VGPR(layout.VADDR,true,false);
                }
                appendOPR_VGPR(layout.VSRC,true,false);
                if (layout.SADDR != 124)  {
                    appendOPR_SREG(layout.SADDR,true,false,2);
                }
                appendOPR_GPUMEM(0,false,true,1,true);
                appendOPR_DSMEM(0,true,false,1,true);
                break;
            case 101:   // GLOBAL_STORE_ASYNC_FROM_LDS_B64,
                if (layout.SADDR == 124)  {
                    appendOPR_VGPR(layout.VADDR,true,false,2);
                } else  {
                    appendOPR_VGPR(layout.VADDR,true,false);
                }
                appendOPR_VGPR(layout.VSRC,true,false);
                if (layout.SADDR != 124)  {
                    appendOPR_SREG(layout.SADDR,true,false,2);
                }
                appendOPR_GPUMEM(0,false,true,2,true);
                appendOPR_DSMEM(0,true,false,2,true);
                break;
            case 102:   // GLOBAL_STORE_ASYNC_FROM_LDS_B128,
                if (layout.SADDR == 124)  {
                    appendOPR_VGPR(layout.VADDR,true,false,2);
                } else  {
                    appendOPR_VGPR(layout.VADDR,true,false);
                }
                appendOPR_VGPR(layout.VSRC,true,false);
                if (layout.SADDR != 124)  {
                    appendOPR_SREG(layout.SADDR,true,false,2);
                }
                appendOPR_GPUMEM(0,false,true,4,true);
                appendOPR_DSMEM(0,true,false,4,true);
                break;
            case 103:   // CLUSTER_LOAD_B32,
                appendOPR_VGPR(layout.VDST,false,true);
                if (layout.SADDR == 124)  {
                    appendOPR_VGPR(layout.VADDR,true,false,2);
                } else  {
                    appendOPR_VGPR(layout.VADDR,true,false);
                }
                if (layout.SADDR != 124)  {
                    appendOPR_SREG(layout.SADDR,true,false,2);
                }
                appendOPR_GPUMEM(0,true,false,1,true);
                appendOPR_SDST_M0(125,true,false,1,true);
                break;
            case 104:   // CLUSTER_LOAD_B64,
                appendOPR_VGPR(layout.VDST,false,true,2);
                if (layout.SADDR == 124)  {
                    appendOPR_VGPR(layout.VADDR,true,false,2);
                } else  {
                    appendOPR_VGPR(layout.VADDR,true,false);
                }
                if (layout.SADDR != 124)  {
                    appendOPR_SREG(layout.SADDR,true,false,2);
                }
                appendOPR_GPUMEM(0,true,false,2,true);
                appendOPR_SDST_M0(125,true,false,1,true);
                break;
            case 105:   // CLUSTER_LOAD_B128,
                appendOPR_VGPR(layout.VDST,false,true,4);
                if (layout.SADDR == 124)  {
                    appendOPR_VGPR(layout.VADDR,true,false,2);
                } else  {
                    appendOPR_VGPR(layout.VADDR,true,false);
                }
                if (layout.SADDR != 124)  {
                    appendOPR_SREG(layout.SADDR,true,false,2);
                }
                appendOPR_GPUMEM(0,true,false,4,true);
                appendOPR_SDST_M0(125,true,false,1,true);
                break;
            case 106:  case 107:   // CLUSTER_LOAD_ASYNC_TO_LDS_B8,CLUSTER_LOAD_ASYNC_TO_LDS_B32,
                appendOPR_VGPR(layout.VDST,true,false);
                if (layout.SADDR == 124)  {
                    appendOPR_VGPR(layout.VADDR,true,false,2);
                } else  {
                    appendOPR_VGPR(layout.VADDR,true,false);
                }
                if (layout.SADDR != 124)  {
                    appendOPR_SREG(layout.SADDR,true,false,2);
                }
                appendOPR_DSMEM(0,false,true,1,true);
                appendOPR_GPUMEM(0,true,false,1,true);
                appendOPR_SDST_M0(125,true,false,1,true);
                break;
            case 108:   // CLUSTER_LOAD_ASYNC_TO_LDS_B64,
                appendOPR_VGPR(layout.VDST,true,false);
                if (layout.SADDR == 124)  {
                    appendOPR_VGPR(layout.VADDR,true,false,2);
                } else  {
                    appendOPR_VGPR(layout.VADDR,true,false);
                }
                if (layout.SADDR != 124)  {
                    appendOPR_SREG(layout.SADDR,true,false,2);
                }
                appendOPR_DSMEM(0,false,true,2,true);
                appendOPR_GPUMEM(0,true,false,2,true);
                appendOPR_SDST_M0(125,true,false,1,true);
                break;
            case 109:   // CLUSTER_LOAD_ASYNC_TO_LDS_B128,
                appendOPR_VGPR(layout.VDST,true,false);
                if (layout.SADDR == 124)  {
                    appendOPR_VGPR(layout.VADDR,true,false,2);
                } else  {
                    appendOPR_VGPR(layout.VADDR,true,false);
                }
                if (layout.SADDR != 124)  {
                    appendOPR_SREG(layout.SADDR,true,false,2);
                }
                appendOPR_DSMEM(0,false,true,4,true);
                appendOPR_GPUMEM(0,true,false,4,true);
                appendOPR_SDST_M0(125,true,false,1,true);
                break;
        }
    }

    void InstructionDecoder_amdgpu_gfx1250::finalizeENC_VIMAGEOperands()
    {
        layout_ENC_VIMAGE & layout = insn_layout.ENC_VIMAGE;
        switch (layout.OP)  {
            case 196:  case 197:   // TENSOR_LOAD_TO_LDS,TENSOR_STORE_FROM_LDS,
                appendOPR_SGPR(layout.VADDR0,true,false,4);
                appendOPR_SGPR(layout.VADDR1,true,false,8);
                appendOPR_SREG(layout.VADDR2,true,false,4);
                appendOPR_SREG(layout.VADDR3,true,false,4);
                break;
        }
    }

    void InstructionDecoder_amdgpu_gfx1250::finalizeENC_VOP1Operands()
    {
        layout_ENC_VOP1 & layout = insn_layout.ENC_VOP1;
        switch (layout.OP)  {
            case 0:  case 27:   // V_NOP,V_PIPEFLUSH,
                break;
            case 1:  case 5:  case 6:  case 7:  case 8:   // V_MOV_B32,V_CVT_F32_I32,V_CVT_F32_U32,V_CVT_U32_F32,V_CVT_I32_F32,
            case 10:  case 11:  case 12:  case 13:   // V_CVT_F16_F32,V_CVT_F32_F16,V_CVT_NEAREST_I32_F32,V_CVT_FLOOR_I32_F32,
            case 14:  case 17:  case 18:  case 19:   // V_CVT_OFF_F32_I4,V_CVT_F32_UBYTE0,V_CVT_F32_UBYTE1,V_CVT_F32_UBYTE2,
            case 20:  case 28:  case 30:  case 31:  case 32:   // V_CVT_F32_UBYTE3,V_MOV_B16,V_TANH_F32,V_TANH_F16,V_FRACT_F32,
            case 33:  case 34:  case 35:  case 36:  case 37:   // V_TRUNC_F32,V_CEIL_F32,V_RNDNE_F32,V_FLOOR_F32,V_EXP_F32,
            case 39:  case 42:  case 43:  case 46:  case 51:   // V_LOG_F32,V_RCP_F32,V_RCP_IFLAG_F32,V_RSQ_F32,V_SQRT_F32,
            case 53:  case 54:  case 55:  case 56:  case 57:   // V_SIN_F32,V_COS_F32,V_NOT_B32,V_BFREV_B32,V_CLZ_I32_U32,
            case 58:  case 59:  case 63:  case 64:   // V_CTZ_I32_B32,V_CLS_I32,V_FREXP_EXP_I32_F32,V_FREXP_MANT_F32,
            case 74:  case 75:  case 80:  case 81:  case 82:   // V_TANH_BF16,V_PRNG_B32,V_CVT_F16_U16,V_CVT_F16_I16,V_CVT_U16_F16,
            case 83:  case 84:  case 85:  case 86:  case 87:   // V_CVT_I16_F16,V_RCP_F16,V_SQRT_F16,V_RSQ_F16,V_LOG_F16,
            case 88:  case 89:  case 90:  case 91:   // V_EXP_F16,V_FREXP_MANT_F16,V_FREXP_EXP_I16_F16,V_FLOOR_F16,
            case 92:  case 93:  case 94:  case 95:  case 96:   // V_CEIL_F16,V_TRUNC_F16,V_RNDNE_F16,V_FRACT_F16,V_SIN_F16,
            case 97:  case 98:  case 99:  case 100:   // V_COS_F16,V_SAT_PK_U8_I16,V_CVT_NORM_I16_F16,V_CVT_NORM_U16_F16,
            case 105:  case 106:  case 107:  case 108:   // V_NOT_B16,V_CVT_I32_I16,V_CVT_U32_U16,V_CVT_F32_FP8,
            case 109:  case 114:  case 115:  case 116:   // V_CVT_F32_BF8,V_CVT_F32_BF16,V_SAT_PK4_I4_I8,V_SAT_PK4_U4_U8,
            case 117:  case 118:  case 119:  case 120:   // V_CVT_PK_F16_FP8,V_CVT_PK_F16_BF8,V_CVT_F16_FP8,V_CVT_F16_BF8,
            case 121:  case 122:  case 123:  case 124:  case 125:   // V_RCP_BF16,V_SQRT_BF16,V_RSQ_BF16,V_LOG_BF16,V_EXP_BF16,
            case 126:  case 127:   // V_SIN_BF16,V_COS_BF16,
                appendOPR_VGPR(layout.VDST,false,true);
                appendOPR_SRC(layout.SRC0,true,false);
                break;
            case 2:   // V_READFIRSTLANE_B32,
                appendOPR_SREG(layout.VDST,false,true);
                appendOPR_SRC_VGPR(layout.SRC0,true,false);
                break;
            case 3:  case 15:  case 21:  case 60:   // V_CVT_I32_F64,V_CVT_F32_F64,V_CVT_U32_F64,V_FREXP_EXP_I32_F64,
                appendOPR_VGPR(layout.VDST,false,true);
                appendOPR_SRC(layout.SRC0,true,false,2);
                break;
            case 4:  case 16:  case 22:  case 110:   // V_CVT_F64_I32,V_CVT_F64_F32,V_CVT_F64_U32,V_CVT_PK_F32_FP8,
            case 111:   // V_CVT_PK_F32_BF8,
                appendOPR_VGPR(layout.VDST,false,true,2);
                appendOPR_SRC(layout.SRC0,true,false);
                break;
            case 23:  case 24:  case 25:  case 26:  case 29:   // V_TRUNC_F64,V_CEIL_F64,V_RNDNE_F64,V_FLOOR_F64,V_MOV_B64,
            case 47:  case 49:  case 52:  case 61:  case 62:   // V_RCP_F64,V_RSQ_F64,V_SQRT_F64,V_FREXP_MANT_F64,V_FRACT_F64,
                appendOPR_VGPR(layout.VDST,false,true,2);
                appendOPR_SRC(layout.SRC0,true,false,2);
                break;
            case 66:   // V_MOVRELD_B32,
                appendOPR_VGPR(layout.VDST,false,true);
                appendOPR_SRC(layout.SRC0,true,false);
                appendOPR_SDST_M0(125,true,false,1,true);
                break;
            case 67:  case 68:  case 72:   // V_MOVRELS_B32,V_MOVRELSD_B32,V_MOVRELSD_2_B32,
                appendOPR_VGPR(layout.VDST,false,true);
                appendOPR_SRC_VGPR(layout.SRC0,true,false);
                appendOPR_SDST_M0(125,true,false,1,true);
                break;
            case 73:  case 101:  case 102:   // V_PERMLANE16_SWAP_B32,V_SWAP_B32,V_SWAP_B16,
                appendOPR_VGPR(layout.VDST,false,true);
                appendOPR_SRC_VGPR(layout.SRC0,false,true);
                break;
            case 104:   // V_SWAPREL_B32,
                appendOPR_VGPR(layout.VDST,false,true);
                appendOPR_SRC_VGPR(layout.SRC0,false,true);
                appendOPR_SDST_M0(125,true,false,1,true);
                break;
        }
    }

    void InstructionDecoder_amdgpu_gfx1250::finalizeENC_VOP3Operands()
    {
        layout_ENC_VOP3 & layout = insn_layout.ENC_VOP3;
        switch (layout.OP)  {
            case 384:  case 411:   // V_NOP,V_PIPEFLUSH,
                break;
            case 385:  case 389:  case 390:  case 391:   // V_MOV_B32,V_CVT_F32_I32,V_CVT_F32_U32,V_CVT_U32_F32,
            case 392:  case 394:  case 395:  case 396:   // V_CVT_I32_F32,V_CVT_F16_F32,V_CVT_F32_F16,V_CVT_NEAREST_I32_F32,
            case 397:  case 398:  case 401:  case 402:   // V_CVT_FLOOR_I32_F32,V_CVT_OFF_F32_I4,V_CVT_F32_UBYTE0,V_CVT_F32_UBYTE1,
            case 403:  case 404:  case 412:  case 414:   // V_CVT_F32_UBYTE2,V_CVT_F32_UBYTE3,V_MOV_B16,V_TANH_F32,
            case 415:  case 416:  case 417:  case 418:  case 419:   // V_TANH_F16,V_FRACT_F32,V_TRUNC_F32,V_CEIL_F32,V_RNDNE_F32,
            case 420:  case 421:  case 423:  case 426:  case 427:   // V_FLOOR_F32,V_EXP_F32,V_LOG_F32,V_RCP_F32,V_RCP_IFLAG_F32,
            case 430:  case 435:  case 437:  case 438:  case 439:   // V_RSQ_F32,V_SQRT_F32,V_SIN_F32,V_COS_F32,V_NOT_B32,
            case 440:  case 441:  case 442:  case 443:   // V_BFREV_B32,V_CLZ_I32_U32,V_CTZ_I32_B32,V_CLS_I32,
            case 447:  case 448:  case 458:  case 459:   // V_FREXP_EXP_I32_F32,V_FREXP_MANT_F32,V_TANH_BF16,V_PRNG_B32,
            case 464:  case 465:  case 466:  case 467:   // V_CVT_F16_U16,V_CVT_F16_I16,V_CVT_U16_F16,V_CVT_I16_F16,
            case 468:  case 469:  case 470:  case 471:  case 472:   // V_RCP_F16,V_SQRT_F16,V_RSQ_F16,V_LOG_F16,V_EXP_F16,
            case 473:  case 474:  case 475:  case 476:   // V_FREXP_MANT_F16,V_FREXP_EXP_I16_F16,V_FLOOR_F16,V_CEIL_F16,
            case 477:  case 478:  case 479:  case 480:  case 481:   // V_TRUNC_F16,V_RNDNE_F16,V_FRACT_F16,V_SIN_F16,V_COS_F16,
            case 482:  case 483:  case 484:  case 489:   // V_SAT_PK_U8_I16,V_CVT_NORM_I16_F16,V_CVT_NORM_U16_F16,V_NOT_B16,
            case 490:  case 491:  case 492:  case 493:   // V_CVT_I32_I16,V_CVT_U32_U16,V_CVT_F32_FP8,V_CVT_F32_BF8,
            case 498:  case 499:  case 500:  case 501:   // V_CVT_F32_BF16,V_SAT_PK4_I4_I8,V_SAT_PK4_U4_U8,V_CVT_PK_F16_FP8,
            case 502:  case 503:  case 504:  case 505:   // V_CVT_PK_F16_BF8,V_CVT_F16_FP8,V_CVT_F16_BF8,V_RCP_BF16,
            case 506:  case 507:  case 508:  case 509:  case 510:   // V_SQRT_BF16,V_RSQ_BF16,V_LOG_BF16,V_EXP_BF16,V_SIN_BF16,
            case 511:   // V_COS_BF16,
                appendOPR_VGPR(layout.VDST,false,true);
                appendOPR_SRC(layout.SRC0,true,false);
                break;
            case 386:   // V_READFIRSTLANE_B32,
                appendOPR_SREG(layout.VDST,false,true);
                appendOPR_SRC_VGPR(layout.SRC0,true,false);
                break;
            case 387:  case 399:  case 405:  case 444:   // V_CVT_I32_F64,V_CVT_F32_F64,V_CVT_U32_F64,V_FREXP_EXP_I32_F64,
                appendOPR_VGPR(layout.VDST,false,true);
                appendOPR_SRC(layout.SRC0,true,false,2);
                break;
            case 388:  case 400:  case 406:  case 494:   // V_CVT_F64_I32,V_CVT_F64_F32,V_CVT_F64_U32,V_CVT_PK_F32_FP8,
            case 495:   // V_CVT_PK_F32_BF8,
                appendOPR_VGPR(layout.VDST,false,true,2);
                appendOPR_SRC(layout.SRC0,true,false);
                break;
            case 407:  case 408:  case 409:  case 410:  case 413:   // V_TRUNC_F64,V_CEIL_F64,V_RNDNE_F64,V_FLOOR_F64,V_MOV_B64,
            case 431:  case 433:  case 436:  case 445:  case 446:   // V_RCP_F64,V_RSQ_F64,V_SQRT_F64,V_FREXP_MANT_F64,V_FRACT_F64,
                appendOPR_VGPR(layout.VDST,false,true,2);
                appendOPR_SRC(layout.SRC0,true,false,2);
                break;
            case 450:   // V_MOVRELD_B32,
                appendOPR_VGPR(layout.VDST,false,true);
                appendOPR_SRC(layout.SRC0,true,false);
                appendOPR_SDST_M0(125,true,false,1,true);
                break;
            case 451:  case 452:  case 456:   // V_MOVRELS_B32,V_MOVRELSD_B32,V_MOVRELSD_2_B32,
                appendOPR_VGPR(layout.VDST,false,true);
                appendOPR_SRC_VGPR(layout.SRC0,true,false);
                appendOPR_SDST_M0(125,true,false,1,true);
                break;
            case 457:   // V_PERMLANE16_SWAP_B32,
                appendOPR_VGPR(layout.VDST,false,true);
                appendOPR_SRC_VGPR(layout.SRC0,false,true);
                break;
            case 257:  case 605:   // V_CNDMASK_B32,V_CNDMASK_B16,
                appendOPR_VGPR(layout.VDST,false,true);
                appendOPR_SRC(layout.SRC0,true,false);
                appendOPR_SRC(layout.SRC1,true,false);
                appendOPR_SREG(layout.SRC2,true,false);
                break;
            case 258:  case 262:  case 269:  case 270:  case 279:   // V_ADD_F64,V_MUL_F64,V_MIN_NUM_F64,V_MAX_NUM_F64,V_FMAC_F64,
            case 296:  case 297:  case 298:  case 792:  case 793:   // V_ADD_NC_U64,V_SUB_NC_U64,V_MUL_U64,V_MIN_U64,V_MAX_U64,
            case 794:  case 795:  case 833:  case 834:   // V_MIN_I64,V_MAX_I64,V_MINIMUM_F64,V_MAXIMUM_F64,
                appendOPR_VGPR(layout.VDST,false,true,2);
                appendOPR_SRC(layout.SRC0,true,false,2);
                appendOPR_SRC(layout.SRC1,true,false,2);
                break;
            case 259:  case 260:  case 261:  case 263:  case 264:   // V_ADD_F32,V_SUB_F32,V_SUBREV_F32,V_MUL_DX9_ZERO_F32,V_MUL_F32,
            case 265:  case 266:  case 267:  case 268:   // V_MUL_I32_I24,V_MUL_HI_I32_I24,V_MUL_U32_U24,V_MUL_HI_U32_U24,
            case 273:  case 274:  case 275:  case 276:  case 277:   // V_MIN_I32,V_MAX_I32,V_MIN_U32,V_MAX_U32,V_MIN_NUM_F32,
            case 278:  case 280:  case 281:  case 282:   // V_MAX_NUM_F32,V_LSHLREV_B32,V_LSHRREV_B32,V_ASHRREV_I32,
            case 283:  case 284:  case 285:  case 286:  case 293:   // V_AND_B32,V_OR_B32,V_XOR_B32,V_XNOR_B32,V_ADD_NC_U32,
            case 294:  case 295:  case 299:  case 303:   // V_SUB_NC_U32,V_SUBREV_NC_U32,V_FMAC_F32,V_CVT_PK_RTZ_F16_F32,
            case 304:  case 305:  case 306:  case 307:  case 308:   // V_MIN_NUM_F16,V_MAX_NUM_F16,V_ADD_F16,V_SUB_F16,V_SUBREV_F16,
            case 309:  case 310:  case 315:  case 771:  case 772:   // V_MUL_F16,V_FMAC_F16,V_LDEXP_F16,V_ADD_NC_U16,V_SUB_NC_U16,
            case 773:  case 774:  case 775:  case 777:   // V_MUL_LO_U16,V_CVT_PK_I16_F32,V_CVT_PK_U16_F32,V_MAX_U16,
            case 778:  case 779:  case 780:  case 781:  case 782:   // V_MAX_I16,V_MIN_U16,V_MIN_I16,V_ADD_NC_I16,V_SUB_NC_I16,
            case 785:  case 786:  case 787:  case 796:   // V_PACK_B32_F16,V_CVT_PK_NORM_I16_F16,V_CVT_PK_NORM_U16_F16,V_LDEXP_F32,
            case 797:  case 798:  case 799:  case 800:   // V_BFM_B32,V_BCNT_U32_B32,V_MBCNT_LO_U32_B32,V_MBCNT_HI_U32_B32,
            case 801:  case 802:  case 803:   // V_CVT_PK_NORM_I16_F32,V_CVT_PK_NORM_U16_F32,V_CVT_PK_U16_U32,
            case 804:  case 805:  case 806:  case 812:   // V_CVT_PK_I16_I32,V_SUB_NC_I32,V_ADD_NC_I32,V_MUL_LO_U32,
            case 813:  case 814:  case 824:  case 825:   // V_MUL_HI_U32,V_MUL_HI_I32,V_LSHLREV_B16,V_LSHRREV_B16,
            case 826:  case 866:  case 867:  case 868:  case 869:   // V_ASHRREV_I16,V_AND_B16,V_OR_B16,V_XOR_B16,V_MINIMUM_F32,
            case 870:  case 871:  case 872:  case 873:   // V_MAXIMUM_F32,V_MINIMUM_F16,V_MAXIMUM_F16,V_CVT_PK_FP8_F32,
            case 874:  case 875:  case 876:  case 877:   // V_CVT_PK_BF8_F32,V_CVT_SR_FP8_F32,V_CVT_SR_BF8_F32,V_CVT_PK_BF16_F32,
            case 879:  case 884:  case 885:   // V_CVT_PK_F16_F32,V_CVT_SR_FP8_F16,V_CVT_SR_BF8_F16,
                appendOPR_VGPR(layout.VDST,false,true);
                appendOPR_SRC(layout.SRC0,true,false);
                appendOPR_SRC(layout.SRC1,true,false);
                break;
            case 287:  case 829:  case 830:   // V_LSHLREV_B64,V_LSHRREV_B64,V_ASHRREV_I64,
                appendOPR_VGPR(layout.VDST,false,true,2);
                appendOPR_SRC(layout.SRC0,true,false);
                appendOPR_SRC(layout.SRC1,true,false,2);
                break;
            case 521:  case 522:  case 523:  case 524:   // V_FMA_DX9_ZERO_F32,V_MAD_I32_I24,V_MAD_U32_U24,V_CUBEID_F32,
            case 525:  case 526:  case 527:  case 528:  case 529:   // V_CUBESC_F32,V_CUBETC_F32,V_CUBEMA_F32,V_BFE_U32,V_BFE_I32,
            case 530:  case 531:  case 533:  case 534:  case 535:   // V_BFI_B32,V_FMA_F32,V_LERP_U8,V_ALIGNBIT_B32,V_ALIGNBYTE_B32,
            case 536:  case 538:  case 539:  case 541:  case 542:   // V_MULLIT_F32,V_MIN3_I32,V_MIN3_U32,V_MAX3_I32,V_MAX3_U32,
            case 544:  case 545:  case 546:  case 547:  case 548:   // V_MED3_I32,V_MED3_U32,V_SAD_U8,V_SAD_HI_U8,V_SAD_U16,
            case 549:  case 550:  case 551:  case 553:   // V_SAD_U32,V_CVT_PK_U8_F32,V_DIV_FIXUP_F32,V_MIN3_NUM_F32,
            case 554:  case 555:  case 556:  case 557:   // V_MAX3_NUM_F32,V_MIN3_NUM_F16,V_MAX3_NUM_F16,V_MINIMUM3_F32,
            case 558:  case 559:  case 560:  case 561:   // V_MAXIMUM3_F32,V_MINIMUM3_F16,V_MAXIMUM3_F16,V_MED3_NUM_F32,
            case 562:  case 563:  case 564:  case 565:  case 569:   // V_MED3_NUM_F16,V_BITOP3_B16,V_BITOP3_B32,V_MAD_U32,V_MSAD_U8,
            case 576:  case 577:  case 580:  case 581:  case 582:   // V_XOR3_B32,V_MAD_U16,V_PERM_B32,V_XAD_U32,V_LSHL_ADD_U32,
            case 583:  case 584:  case 586:  case 587:  case 589:   // V_ADD_LSHL_U32,V_FMA_F16,V_MIN3_I16,V_MIN3_U16,V_MAX3_I16,
            case 590:  case 592:  case 593:  case 595:  case 596:   // V_MAX3_U16,V_MED3_I16,V_MED3_U16,V_MAD_I16,V_DIV_FIXUP_F16,
            case 597:  case 598:  case 599:  case 600:  case 601:   // V_ADD3_U32,V_LSHL_OR_B32,V_AND_OR_B32,V_OR3_B32,V_MAD_U32_U16,
            case 602:  case 606:  case 607:  case 608:   // V_MAD_I32_I16,V_ADD_MAX_I32,V_ADD_MAX_U32,V_ADD_MIN_I32,
            case 609:  case 610:  case 611:  case 612:   // V_ADD_MIN_U32,V_MAXMIN_U32,V_MINMAX_U32,V_MAXMIN_I32,
            case 613:  case 616:  case 617:  case 618:   // V_MINMAX_I32,V_MINMAX_NUM_F32,V_MAXMIN_NUM_F32,V_MINMAX_NUM_F16,
            case 619:  case 620:  case 621:   // V_MAXMIN_NUM_F16,V_MINIMUMMAXIMUM_F32,V_MAXIMUMMINIMUM_F32,
            case 622:  case 623:  case 656:   // V_MINIMUMMAXIMUM_F16,V_MAXIMUMMINIMUM_F16,V_ASHR_PK_I8_I32,
            case 657:  case 878:  case 880:   // V_ASHR_PK_U8_I32,V_CVT_SR_PK_BF16_F32,V_CVT_SR_PK_F16_F32,
                appendOPR_VGPR(layout.VDST,false,true);
                appendOPR_SRC(layout.SRC0,true,false);
                appendOPR_SRC(layout.SRC1,true,false);
                appendOPR_SRC(layout.SRC2,true,false);
                break;
            case 532:  case 552:   // V_FMA_F64,V_DIV_FIXUP_F64,
                appendOPR_VGPR(layout.VDST,false,true,2);
                appendOPR_SRC(layout.SRC0,true,false,2);
                appendOPR_SRC(layout.SRC1,true,false,2);
                appendOPR_SRC(layout.SRC2,true,false,2);
                break;
            case 567:   // V_DIV_FMAS_F32,
                appendOPR_VGPR(layout.VDST,false,true);
                appendOPR_SRC(layout.SRC0,true,false);
                appendOPR_SRC(layout.SRC1,true,false);
                appendOPR_SRC(layout.SRC2,true,false);
                appendOPR_VCC(106,true,false,1,true);
                break;
            case 568:   // V_DIV_FMAS_F64,
                appendOPR_VGPR(layout.VDST,false,true,2);
                appendOPR_SRC(layout.SRC0,true,false,2);
                appendOPR_SRC(layout.SRC1,true,false,2);
                appendOPR_SRC(layout.SRC2,true,false,2);
                appendOPR_VCC(106,true,false,1,true);
                break;
            case 570:  case 571:  case 594:   // V_QSAD_PK_U16_U8,V_MQSAD_PK_U16_U8,V_LSHL_ADD_U64,
                appendOPR_VGPR(layout.VDST,false,true,2);
                appendOPR_SRC(layout.SRC0,true,false,2);
                appendOPR_SRC(layout.SRC1,true,false);
                appendOPR_SRC(layout.SRC2,true,false,2);
                break;
            case 573:   // V_MQSAD_U32_U8,
                appendOPR_VGPR(layout.VDST,false,true,4);
                appendOPR_SRC(layout.SRC0,true,false,2);
                appendOPR_SRC(layout.SRC1,true,false);
                appendOPR_SRC_VGPR(layout.SRC2,true,false,4);
                break;
            case 575:  case 762:  case 763:   // V_PERM_PK16_B4_U4,V_MAD_NC_U64_U32,V_MAD_NC_I64_I32,
                appendOPR_VGPR(layout.VDST,false,true,2);
                appendOPR_SRC(layout.SRC0,true,false);
                appendOPR_SRC(layout.SRC1,true,false);
                appendOPR_SRC(layout.SRC2,true,false,2);
                break;
            case 578:   // V_PERM_PK16_B6_U4,
                appendOPR_VGPR(layout.VDST,false,true,3);
                appendOPR_SRC(layout.SRC0,true,false);
                appendOPR_SRC(layout.SRC1,true,false,2);
                appendOPR_SRC(layout.SRC2,true,false,2);
                break;
            case 579:   // V_PERM_PK16_B8_U4,
                appendOPR_VGPR(layout.VDST,false,true,4);
                appendOPR_SRC(layout.SRC0,true,false,2);
                appendOPR_SRC(layout.SRC1,true,false,2);
                appendOPR_SRC(layout.SRC2,true,false,2);
                break;
            case 603:  case 604:  case 624:  case 625:   // V_PERMLANE16_B32,V_PERMLANEX16_B32,V_PERMLANE_BCAST_B32,V_PERMLANE_UP_B32,
            case 626:  case 627:   // V_PERMLANE_DOWN_B32,V_PERMLANE_XOR_B32,
                appendOPR_VGPR(layout.VDST,false,true);
                appendOPR_SRC_VGPR(layout.SRC0,true,false);
                appendOPR_SSRC(layout.SRC1,true,false);
                appendOPR_SSRC(layout.SRC2,true,false);
                break;
            case 640:  case 641:  case 642:  case 643:  case 644:   // V_S_EXP_F32,V_S_EXP_F16,V_S_LOG_F32,V_S_LOG_F16,V_S_RCP_F32,
            case 645:  case 646:  case 647:  case 648:  case 649:   // V_S_RCP_F16,V_S_RSQ_F32,V_S_RSQ_F16,V_S_SQRT_F32,V_S_SQRT_F16,
                appendOPR_SREG(layout.VDST,false,true);
                appendOPR_SSRC(layout.SRC0,true,false);
                break;
            case 663:   // V_CVT_SCALEF32_SR_PK8_FP4_F32,
                appendOPR_VGPR(layout.VDST,false,true);
                appendOPR_SRC_VGPR(layout.SRC0,true,false,8);
                appendOPR_SRC(layout.SRC1,true,false);
                appendOPR_SRC(layout.SRC2,true,false);
                break;
            case 664:  case 665:   // V_CVT_SCALEF32_SR_PK8_FP8_F32,V_CVT_SCALEF32_SR_PK8_BF8_F32,
                appendOPR_VGPR(layout.VDST,false,true,2);
                appendOPR_SRC_VGPR(layout.SRC0,true,false,8);
                appendOPR_SRC(layout.SRC1,true,false);
                appendOPR_SRC(layout.SRC2,true,false);
                break;
            case 671:  case 672:   // V_CVT_SCALE_PK8_F16_FP4,V_CVT_SCALE_PK8_BF16_FP4,
                appendOPR_VGPR(layout.VDST,false,true,4);
                appendOPR_SRC_VGPR(layout.SRC0,true,false);
                appendOPR_SRC(layout.SRC1,true,false);
                break;
            case 673:   // V_CVT_SCALE_PK8_F32_FP4,
                appendOPR_VGPR(layout.VDST,false,true,8);
                appendOPR_SRC_VGPR(layout.SRC0,true,false);
                appendOPR_SRC(layout.SRC1,true,false);
                break;
            case 680:  case 681:  case 683:   // V_CVT_SCALE_PK8_F16_FP8,V_CVT_SCALE_PK8_BF16_FP8,V_CVT_SCALE_PK8_F16_BF8,
            case 684:   // V_CVT_SCALE_PK8_BF16_BF8,
                appendOPR_VGPR(layout.VDST,false,true,4);
                appendOPR_SRC_VGPR(layout.SRC0,true,false,2);
                appendOPR_SRC(layout.SRC1,true,false);
                break;
            case 682:  case 685:   // V_CVT_SCALE_PK8_F32_FP8,V_CVT_SCALE_PK8_F32_BF8,
                appendOPR_VGPR(layout.VDST,false,true,8);
                appendOPR_SRC_VGPR(layout.SRC0,true,false,2);
                appendOPR_SRC(layout.SRC1,true,false);
                break;
            case 688:   // V_CVT_SCALEF32_PK8_FP4_F32,
                appendOPR_VGPR(layout.VDST,false,true);
                appendOPR_SRC_VGPR(layout.SRC0,true,false,8);
                appendOPR_SRC(layout.SRC1,true,false);
                break;
            case 691:  case 696:   // V_CVT_SCALEF32_PK8_FP4_F16,V_CVT_SCALEF32_PK8_FP4_BF16,
                appendOPR_VGPR(layout.VDST,false,true);
                appendOPR_SRC_VGPR(layout.SRC0,true,false,4);
                appendOPR_SRC(layout.SRC1,true,false);
                break;
            case 692:  case 693:  case 708:   // V_CVT_SCALEF32_PK8_FP8_BF16,V_CVT_SCALEF32_PK8_BF8_BF16,V_CVT_SCALEF32_PK8_FP8_F16,
            case 710:   // V_CVT_SCALEF32_PK8_BF8_F16,
                appendOPR_VGPR(layout.VDST,false,true,2);
                appendOPR_SRC_VGPR(layout.SRC0,true,false,4);
                appendOPR_SRC(layout.SRC1,true,false);
                break;
            case 697:  case 700:   // V_CVT_SCALEF32_SR_PK8_FP4_F16,V_CVT_SCALEF32_SR_PK8_FP4_BF16,
                appendOPR_VGPR(layout.VDST,false,true);
                appendOPR_SRC_VGPR(layout.SRC0,true,false,4);
                appendOPR_SRC(layout.SRC1,true,false);
                appendOPR_SRC(layout.SRC2,true,false);
                break;
            case 703:  case 704:   // V_CVT_SCALEF32_SR_PK8_FP8_F16,V_CVT_SCALEF32_SR_PK8_FP8_BF16,
            case 705:  case 706:   // V_CVT_SCALEF32_SR_PK8_BF8_F16,V_CVT_SCALEF32_SR_PK8_BF8_BF16,
                appendOPR_VGPR(layout.VDST,false,true,2);
                appendOPR_SRC_VGPR(layout.SRC0,true,false,4);
                appendOPR_SRC(layout.SRC1,true,false);
                appendOPR_SRC(layout.SRC2,true,false);
                break;
            case 707:  case 709:   // V_CVT_SCALEF32_PK8_FP8_F32,V_CVT_SCALEF32_PK8_BF8_F32,
                appendOPR_VGPR(layout.VDST,false,true,2);
                appendOPR_SRC_VGPR(layout.SRC0,true,false,8);
                appendOPR_SRC(layout.SRC1,true,false);
                break;
            case 711:  case 712:  case 714:   // V_CVT_SCALE_PK16_F16_FP6,V_CVT_SCALE_PK16_BF16_FP6,V_CVT_SCALE_PK16_F16_BF6,
            case 715:   // V_CVT_SCALE_PK16_BF16_BF6,
                appendOPR_VGPR(layout.VDST,false,true,8);
                appendOPR_SRC_VGPR(layout.SRC0,true,false,3);
                appendOPR_SRC(layout.SRC1,true,false);
                break;
            case 713:  case 716:   // V_CVT_SCALE_PK16_F32_FP6,V_CVT_SCALE_PK16_F32_BF6,
                appendOPR_VGPR(layout.VDST,false,true,16);
                appendOPR_SRC_VGPR(layout.SRC0,true,false,3);
                appendOPR_SRC(layout.SRC1,true,false);
                break;
            case 717:  case 718:   // V_CVT_SCALEF32_PK16_FP6_F32,V_CVT_SCALEF32_PK16_BF6_F32,
                appendOPR_VGPR(layout.VDST,false,true,3);
                appendOPR_SRC_VGPR(layout.SRC0,true,false,16);
                appendOPR_SRC(layout.SRC1,true,false);
                break;
            case 719:  case 720:  case 721:   // V_CVT_SCALEF32_PK16_FP6_F16,V_CVT_SCALEF32_PK16_BF6_F16,V_CVT_SCALEF32_PK16_FP6_BF16,
            case 722:   // V_CVT_SCALEF32_PK16_BF6_BF16,
                appendOPR_VGPR(layout.VDST,false,true,3);
                appendOPR_SRC_VGPR(layout.SRC0,true,false,8);
                appendOPR_SRC(layout.SRC1,true,false);
                break;
            case 723:  case 724:   // V_CVT_SCALEF32_SR_PK16_FP6_F32,V_CVT_SCALEF32_SR_PK16_BF6_F32,
                appendOPR_VGPR(layout.VDST,false,true,3);
                appendOPR_SRC_VGPR(layout.SRC0,true,false,16);
                appendOPR_SRC(layout.SRC1,true,false);
                appendOPR_SRC(layout.SRC2,true,false);
                break;
            case 725:  case 726:   // V_CVT_SCALEF32_SR_PK16_FP6_F16,V_CVT_SCALEF32_SR_PK16_BF6_F16,
            case 727:  case 728:   // V_CVT_SCALEF32_SR_PK16_FP6_BF16,V_CVT_SCALEF32_SR_PK16_BF6_BF16,
                appendOPR_VGPR(layout.VDST,false,true,3);
                appendOPR_SRC_VGPR(layout.SRC0,true,false,8);
                appendOPR_SRC(layout.SRC1,true,false);
                appendOPR_SRC(layout.SRC2,true,false);
                break;
            case 783:  case 784:   // V_PERMLANE16_VAR_B32,V_PERMLANEX16_VAR_B32,
                appendOPR_VGPR(layout.VDST,false,true);
                appendOPR_SRC_VGPR(layout.SRC0,true,false);
                appendOPR_SRC_VGPR(layout.SRC1,true,false);
                break;
            case 788:   // V_PERMLANE_IDX_GEN_B32,
                appendOPR_VGPR(layout.VDST,false,true);
                appendOPR_SRC_VGPR(layout.SRC0,true,false);
                appendOPR_SSRC(layout.SRC1,true,false);
                break;
            case 811:  case 815:   // V_LDEXP_F64,V_TRIG_PREOP_F64,
                appendOPR_VGPR(layout.VDST,false,true,2);
                appendOPR_SRC(layout.SRC0,true,false,2);
                appendOPR_SRC(layout.SRC1,true,false);
                break;
            case 864:   // V_READLANE_B32,
                appendOPR_SREG(layout.VDST,false,true);
                appendOPR_SRC_VGPR(layout.SRC0,true,false);
                appendOPR_SSRC_LANESEL(layout.SRC1,true,false);
                break;
            case 865:   // V_WRITELANE_B32,
                appendOPR_VGPR(layout.VDST,false,true);
                appendOPR_SSRC(layout.SRC0,true,false);
                appendOPR_SSRC_LANESEL(layout.SRC1,true,false);
                break;
            case 882:  case 883:   // V_CVT_PK_FP8_F16,V_CVT_PK_BF8_F16,
                appendOPR_VGPR(layout.VDST,false,true);
                appendOPR_SRC_NOINLINE(layout.SRC0,true,false);
                break;
            case 1:  case 2:  case 3:  case 4:  case 5:   // V_CMP_LT_F16,V_CMP_EQ_F16,V_CMP_LE_F16,V_CMP_GT_F16,V_CMP_LG_F16,
            case 6:  case 7:  case 8:  case 9:  case 10:   // V_CMP_GE_F16,V_CMP_O_F16,V_CMP_U_F16,V_CMP_NGE_F16,V_CMP_NLG_F16,
            case 11:  case 12:  case 13:  case 14:   // V_CMP_NGT_F16,V_CMP_NLE_F16,V_CMP_NEQ_F16,V_CMP_NLT_F16,
            case 17:  case 18:  case 19:  case 20:  case 21:   // V_CMP_LT_F32,V_CMP_EQ_F32,V_CMP_LE_F32,V_CMP_GT_F32,V_CMP_LG_F32,
            case 22:  case 23:  case 24:  case 25:  case 26:   // V_CMP_GE_F32,V_CMP_O_F32,V_CMP_U_F32,V_CMP_NGE_F32,V_CMP_NLG_F32,
            case 27:  case 28:  case 29:  case 30:   // V_CMP_NGT_F32,V_CMP_NLE_F32,V_CMP_NEQ_F32,V_CMP_NLT_F32,
            case 49:  case 50:  case 51:  case 52:  case 53:   // V_CMP_LT_I16,V_CMP_EQ_I16,V_CMP_LE_I16,V_CMP_GT_I16,V_CMP_NE_I16,
            case 54:  case 57:  case 58:  case 59:  case 60:   // V_CMP_GE_I16,V_CMP_LT_U16,V_CMP_EQ_U16,V_CMP_LE_U16,V_CMP_GT_U16,
            case 61:  case 62:  case 65:  case 66:  case 67:   // V_CMP_NE_U16,V_CMP_GE_U16,V_CMP_LT_I32,V_CMP_EQ_I32,V_CMP_LE_I32,
            case 68:  case 69:  case 70:  case 73:  case 74:   // V_CMP_GT_I32,V_CMP_NE_I32,V_CMP_GE_I32,V_CMP_LT_U32,V_CMP_EQ_U32,
            case 75:  case 76:  case 77:  case 78:   // V_CMP_LE_U32,V_CMP_GT_U32,V_CMP_NE_U32,V_CMP_GE_U32,
            case 125:  case 126:   // V_CMP_CLASS_F16,V_CMP_CLASS_F32,
                appendOPR_SREG(layout.VDST,false,true);
                appendOPR_SRC(layout.SRC0,true,false);
                appendOPR_SRC(layout.SRC1,true,false);
                break;
            case 33:  case 34:  case 35:  case 36:  case 37:   // V_CMP_LT_F64,V_CMP_EQ_F64,V_CMP_LE_F64,V_CMP_GT_F64,V_CMP_LG_F64,
            case 38:  case 39:  case 40:  case 41:  case 42:   // V_CMP_GE_F64,V_CMP_O_F64,V_CMP_U_F64,V_CMP_NGE_F64,V_CMP_NLG_F64,
            case 43:  case 44:  case 45:  case 46:   // V_CMP_NGT_F64,V_CMP_NLE_F64,V_CMP_NEQ_F64,V_CMP_NLT_F64,
            case 81:  case 82:  case 83:  case 84:  case 85:   // V_CMP_LT_I64,V_CMP_EQ_I64,V_CMP_LE_I64,V_CMP_GT_I64,V_CMP_NE_I64,
            case 86:  case 89:  case 90:  case 91:  case 92:   // V_CMP_GE_I64,V_CMP_LT_U64,V_CMP_EQ_U64,V_CMP_LE_U64,V_CMP_GT_U64,
            case 93:  case 94:   // V_CMP_NE_U64,V_CMP_GE_U64,
                appendOPR_SREG(layout.VDST,false,true);
                appendOPR_SRC(layout.SRC0,true,false,2);
                appendOPR_SRC(layout.SRC1,true,false,2);
                break;
            case 127:   // V_CMP_CLASS_F64,
                appendOPR_SREG(layout.VDST,false,true);
                appendOPR_SRC(layout.SRC0,true,false,2);
                appendOPR_SRC(layout.SRC1,true,false);
                break;
            case 129:  case 130:  case 131:  case 132:   // V_CMPX_LT_F16,V_CMPX_EQ_F16,V_CMPX_LE_F16,V_CMPX_GT_F16,
            case 133:  case 134:  case 135:  case 136:   // V_CMPX_LG_F16,V_CMPX_GE_F16,V_CMPX_O_F16,V_CMPX_U_F16,
            case 137:  case 138:  case 139:  case 140:   // V_CMPX_NGE_F16,V_CMPX_NLG_F16,V_CMPX_NGT_F16,V_CMPX_NLE_F16,
            case 141:  case 142:  case 145:  case 146:   // V_CMPX_NEQ_F16,V_CMPX_NLT_F16,V_CMPX_LT_F32,V_CMPX_EQ_F32,
            case 147:  case 148:  case 149:  case 150:   // V_CMPX_LE_F32,V_CMPX_GT_F32,V_CMPX_LG_F32,V_CMPX_GE_F32,
            case 151:  case 152:  case 153:  case 154:   // V_CMPX_O_F32,V_CMPX_U_F32,V_CMPX_NGE_F32,V_CMPX_NLG_F32,
            case 155:  case 156:  case 157:  case 158:   // V_CMPX_NGT_F32,V_CMPX_NLE_F32,V_CMPX_NEQ_F32,V_CMPX_NLT_F32,
            case 177:  case 178:  case 179:  case 180:   // V_CMPX_LT_I16,V_CMPX_EQ_I16,V_CMPX_LE_I16,V_CMPX_GT_I16,
            case 181:  case 182:  case 185:  case 186:   // V_CMPX_NE_I16,V_CMPX_GE_I16,V_CMPX_LT_U16,V_CMPX_EQ_U16,
            case 187:  case 188:  case 189:  case 190:   // V_CMPX_LE_U16,V_CMPX_GT_U16,V_CMPX_NE_U16,V_CMPX_GE_U16,
            case 193:  case 194:  case 195:  case 196:   // V_CMPX_LT_I32,V_CMPX_EQ_I32,V_CMPX_LE_I32,V_CMPX_GT_I32,
            case 197:  case 198:  case 201:  case 202:   // V_CMPX_NE_I32,V_CMPX_GE_I32,V_CMPX_LT_U32,V_CMPX_EQ_U32,
            case 203:  case 204:  case 205:  case 206:   // V_CMPX_LE_U32,V_CMPX_GT_U32,V_CMPX_NE_U32,V_CMPX_GE_U32,
            case 253:  case 254:   // V_CMPX_CLASS_F16,V_CMPX_CLASS_F32,
                appendOPR_EXEC(layout.VDST,false,true);
                appendOPR_SRC(layout.SRC0,true,false);
                appendOPR_SRC(layout.SRC1,true,false);
                appendOPR_SDST_EXEC(126,false,true,1,true);
                break;
            case 161:  case 162:  case 163:  case 164:   // V_CMPX_LT_F64,V_CMPX_EQ_F64,V_CMPX_LE_F64,V_CMPX_GT_F64,
            case 165:  case 166:  case 167:  case 168:   // V_CMPX_LG_F64,V_CMPX_GE_F64,V_CMPX_O_F64,V_CMPX_U_F64,
            case 169:  case 170:  case 171:  case 172:   // V_CMPX_NGE_F64,V_CMPX_NLG_F64,V_CMPX_NGT_F64,V_CMPX_NLE_F64,
            case 173:  case 174:  case 209:  case 210:   // V_CMPX_NEQ_F64,V_CMPX_NLT_F64,V_CMPX_LT_I64,V_CMPX_EQ_I64,
            case 211:  case 212:  case 213:  case 214:   // V_CMPX_LE_I64,V_CMPX_GT_I64,V_CMPX_NE_I64,V_CMPX_GE_I64,
            case 217:  case 218:  case 219:  case 220:   // V_CMPX_LT_U64,V_CMPX_EQ_U64,V_CMPX_LE_U64,V_CMPX_GT_U64,
            case 221:  case 222:   // V_CMPX_NE_U64,V_CMPX_GE_U64,
                appendOPR_EXEC(layout.VDST,false,true);
                appendOPR_SRC(layout.SRC0,true,false,2);
                appendOPR_SRC(layout.SRC1,true,false,2);
                appendOPR_SDST_EXEC(126,false,true,1,true);
                break;
            case 255:   // V_CMPX_CLASS_F64,
                appendOPR_EXEC(layout.VDST,false,true);
                appendOPR_SRC(layout.SRC0,true,false,2);
                appendOPR_SRC(layout.SRC1,true,false);
                appendOPR_SDST_EXEC(126,false,true,1,true);
                break;
        }
    }

    void InstructionDecoder_amdgpu_gfx1250::finalizeVOP1_INST_LITERALOperands()
    {
        layout_VOP1_INST_LITERAL & layout = insn_layout.VOP1_INST_LITERAL;
        switch (layout.OP)  {
            case 1:  case 3:  case 5:  case 6:  case 7:   // V_MOV_B32,V_CVT_I32_F64,V_CVT_F32_I32,V_CVT_F32_U32,V_CVT_U32_F32,
            case 8:  case 10:  case 11:  case 12:   // V_CVT_I32_F32,V_CVT_F16_F32,V_CVT_F32_F16,V_CVT_NEAREST_I32_F32,
            case 13:  case 14:  case 15:  case 17:   // V_CVT_FLOOR_I32_F32,V_CVT_OFF_F32_I4,V_CVT_F32_F64,V_CVT_F32_UBYTE0,
            case 18:  case 19:  case 20:  case 21:   // V_CVT_F32_UBYTE1,V_CVT_F32_UBYTE2,V_CVT_F32_UBYTE3,V_CVT_U32_F64,
            case 28:  case 30:  case 31:  case 32:  case 33:   // V_MOV_B16,V_TANH_F32,V_TANH_F16,V_FRACT_F32,V_TRUNC_F32,
            case 34:  case 35:  case 36:  case 37:  case 39:   // V_CEIL_F32,V_RNDNE_F32,V_FLOOR_F32,V_EXP_F32,V_LOG_F32,
            case 42:  case 43:  case 46:  case 51:  case 53:   // V_RCP_F32,V_RCP_IFLAG_F32,V_RSQ_F32,V_SQRT_F32,V_SIN_F32,
            case 54:  case 55:  case 56:  case 57:  case 58:   // V_COS_F32,V_NOT_B32,V_BFREV_B32,V_CLZ_I32_U32,V_CTZ_I32_B32,
            case 59:  case 60:  case 63:  case 64:   // V_CLS_I32,V_FREXP_EXP_I32_F64,V_FREXP_EXP_I32_F32,V_FREXP_MANT_F32,
            case 74:  case 75:  case 80:  case 81:  case 82:   // V_TANH_BF16,V_PRNG_B32,V_CVT_F16_U16,V_CVT_F16_I16,V_CVT_U16_F16,
            case 83:  case 84:  case 85:  case 86:  case 87:   // V_CVT_I16_F16,V_RCP_F16,V_SQRT_F16,V_RSQ_F16,V_LOG_F16,
            case 88:  case 89:  case 90:  case 91:   // V_EXP_F16,V_FREXP_MANT_F16,V_FREXP_EXP_I16_F16,V_FLOOR_F16,
            case 92:  case 93:  case 94:  case 95:  case 96:   // V_CEIL_F16,V_TRUNC_F16,V_RNDNE_F16,V_FRACT_F16,V_SIN_F16,
            case 97:  case 98:  case 99:  case 100:   // V_COS_F16,V_SAT_PK_U8_I16,V_CVT_NORM_I16_F16,V_CVT_NORM_U16_F16,
            case 105:  case 106:  case 107:  case 108:   // V_NOT_B16,V_CVT_I32_I16,V_CVT_U32_U16,V_CVT_F32_FP8,
            case 109:  case 114:  case 115:  case 116:   // V_CVT_F32_BF8,V_CVT_F32_BF16,V_SAT_PK4_I4_I8,V_SAT_PK4_U4_U8,
            case 117:  case 118:  case 119:  case 120:   // V_CVT_PK_F16_FP8,V_CVT_PK_F16_BF8,V_CVT_F16_FP8,V_CVT_F16_BF8,
            case 121:  case 122:  case 123:  case 124:  case 125:   // V_RCP_BF16,V_SQRT_BF16,V_RSQ_BF16,V_LOG_BF16,V_EXP_BF16,
            case 126:  case 127:   // V_SIN_BF16,V_COS_BF16,
                appendOPR_VGPR(layout.VDST,false,true);
                insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u32,layout.LITERAL)),true,false);
                break;
            case 4:  case 16:  case 22:  case 23:  case 24:   // V_CVT_F64_I32,V_CVT_F64_F32,V_CVT_F64_U32,V_TRUNC_F64,V_CEIL_F64,
            case 25:  case 26:  case 29:  case 47:  case 49:   // V_RNDNE_F64,V_FLOOR_F64,V_MOV_B64,V_RCP_F64,V_RSQ_F64,
            case 52:  case 61:  case 62:  case 110:   // V_SQRT_F64,V_FREXP_MANT_F64,V_FRACT_F64,V_CVT_PK_F32_FP8,
            case 111:   // V_CVT_PK_F32_BF8,
                appendOPR_VGPR(layout.VDST,false,true,2);
                insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u32,layout.LITERAL)),true,false);
                break;
            case 66:   // V_MOVRELD_B32,
                appendOPR_VGPR(layout.VDST,false,true);
                insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u32,layout.LITERAL)),true,false);
                appendOPR_SDST_M0(125,true,false,1,true);
                break;
        }
    }

    void InstructionDecoder_amdgpu_gfx1250::finalizeVOP1_INST_LITERAL64Operands()
    {
        layout_VOP1_INST_LITERAL64 & layout = insn_layout.VOP1_INST_LITERAL64;
        switch (layout.OP)  {
            case 1:  case 3:  case 5:  case 6:  case 7:   // V_MOV_B32,V_CVT_I32_F64,V_CVT_F32_I32,V_CVT_F32_U32,V_CVT_U32_F32,
            case 8:  case 10:  case 11:  case 12:   // V_CVT_I32_F32,V_CVT_F16_F32,V_CVT_F32_F16,V_CVT_NEAREST_I32_F32,
            case 13:  case 14:  case 15:  case 17:   // V_CVT_FLOOR_I32_F32,V_CVT_OFF_F32_I4,V_CVT_F32_F64,V_CVT_F32_UBYTE0,
            case 18:  case 19:  case 20:  case 21:   // V_CVT_F32_UBYTE1,V_CVT_F32_UBYTE2,V_CVT_F32_UBYTE3,V_CVT_U32_F64,
            case 28:  case 30:  case 31:  case 32:  case 33:   // V_MOV_B16,V_TANH_F32,V_TANH_F16,V_FRACT_F32,V_TRUNC_F32,
            case 34:  case 35:  case 36:  case 37:  case 39:   // V_CEIL_F32,V_RNDNE_F32,V_FLOOR_F32,V_EXP_F32,V_LOG_F32,
            case 42:  case 43:  case 46:  case 51:  case 53:   // V_RCP_F32,V_RCP_IFLAG_F32,V_RSQ_F32,V_SQRT_F32,V_SIN_F32,
            case 54:  case 55:  case 56:  case 57:  case 58:   // V_COS_F32,V_NOT_B32,V_BFREV_B32,V_CLZ_I32_U32,V_CTZ_I32_B32,
            case 59:  case 60:  case 63:  case 64:   // V_CLS_I32,V_FREXP_EXP_I32_F64,V_FREXP_EXP_I32_F32,V_FREXP_MANT_F32,
            case 74:  case 75:  case 80:  case 81:  case 82:   // V_TANH_BF16,V_PRNG_B32,V_CVT_F16_U16,V_CVT_F16_I16,V_CVT_U16_F16,
            case 83:  case 84:  case 85:  case 86:  case 87:   // V_CVT_I16_F16,V_RCP_F16,V_SQRT_F16,V_RSQ_F16,V_LOG_F16,
            case 88:  case 89:  case 90:  case 91:   // V_EXP_F16,V_FREXP_MANT_F16,V_FREXP_EXP_I16_F16,V_FLOOR_F16,
            case 92:  case 93:  case 94:  case 95:  case 96:   // V_CEIL_F16,V_TRUNC_F16,V_RNDNE_F16,V_FRACT_F16,V_SIN_F16,
            case 97:  case 98:  case 99:  case 100:   // V_COS_F16,V_SAT_PK_U8_I16,V_CVT_NORM_I16_F16,V_CVT_NORM_U16_F16,
            case 105:  case 106:  case 107:  case 108:   // V_NOT_B16,V_CVT_I32_I16,V_CVT_U32_U16,V_CVT_F32_FP8,
            case 109:  case 114:  case 115:  case 116:   // V_CVT_F32_BF8,V_CVT_F32_BF16,V_SAT_PK4_I4_I8,V_SAT_PK4_U4_U8,
            case 117:  case 118:  case 119:  case 120:   // V_CVT_PK_F16_FP8,V_CVT_PK_F16_BF8,V_CVT_F16_FP8,V_CVT_F16_BF8,
            case 121:  case 122:  case 123:  case 124:  case 125:   // V_RCP_BF16,V_SQRT_BF16,V_RSQ_BF16,V_LOG_BF16,V_EXP_BF16,
            case 126:  case 127:   // V_SIN_BF16,V_COS_BF16,
                appendOPR_VGPR(layout.VDST,false,true);
                insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u64,layout.LITERAL64)),true,false);
                break;
            case 4:  case 16:  case 22:  case 23:  case 24:   // V_CVT_F64_I32,V_CVT_F64_F32,V_CVT_F64_U32,V_TRUNC_F64,V_CEIL_F64,
            case 25:  case 26:  case 29:  case 47:  case 49:   // V_RNDNE_F64,V_FLOOR_F64,V_MOV_B64,V_RCP_F64,V_RSQ_F64,
            case 52:  case 61:  case 62:  case 110:   // V_SQRT_F64,V_FREXP_MANT_F64,V_FRACT_F64,V_CVT_PK_F32_FP8,
            case 111:   // V_CVT_PK_F32_BF8,
                appendOPR_VGPR(layout.VDST,false,true,2);
                insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u64,layout.LITERAL64)),true,false);
                break;
            case 66:   // V_MOVRELD_B32,
                appendOPR_VGPR(layout.VDST,false,true);
                insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u64,layout.LITERAL64)),true,false);
                appendOPR_SDST_M0(125,true,false,1,true);
                break;
        }
    }

    void InstructionDecoder_amdgpu_gfx1250::finalizeVOP1_VOP_DPP8Operands()
    {
        layout_VOP1_VOP_DPP8 & layout = insn_layout.VOP1_VOP_DPP8;
        switch (layout.OP)  {
            case 1:  case 5:  case 6:  case 7:  case 8:   // V_MOV_B32,V_CVT_F32_I32,V_CVT_F32_U32,V_CVT_U32_F32,V_CVT_I32_F32,
            case 10:  case 11:  case 12:  case 13:   // V_CVT_F16_F32,V_CVT_F32_F16,V_CVT_NEAREST_I32_F32,V_CVT_FLOOR_I32_F32,
            case 14:  case 17:  case 18:  case 19:   // V_CVT_OFF_F32_I4,V_CVT_F32_UBYTE0,V_CVT_F32_UBYTE1,V_CVT_F32_UBYTE2,
            case 20:  case 28:  case 30:  case 31:  case 32:   // V_CVT_F32_UBYTE3,V_MOV_B16,V_TANH_F32,V_TANH_F16,V_FRACT_F32,
            case 33:  case 34:  case 35:  case 36:  case 37:   // V_TRUNC_F32,V_CEIL_F32,V_RNDNE_F32,V_FLOOR_F32,V_EXP_F32,
            case 39:  case 42:  case 43:  case 46:  case 51:   // V_LOG_F32,V_RCP_F32,V_RCP_IFLAG_F32,V_RSQ_F32,V_SQRT_F32,
            case 53:  case 54:  case 55:  case 56:  case 57:   // V_SIN_F32,V_COS_F32,V_NOT_B32,V_BFREV_B32,V_CLZ_I32_U32,
            case 58:  case 59:  case 63:  case 64:   // V_CTZ_I32_B32,V_CLS_I32,V_FREXP_EXP_I32_F32,V_FREXP_MANT_F32,
            case 74:  case 75:  case 80:  case 81:  case 82:   // V_TANH_BF16,V_PRNG_B32,V_CVT_F16_U16,V_CVT_F16_I16,V_CVT_U16_F16,
            case 83:  case 84:  case 85:  case 86:  case 87:   // V_CVT_I16_F16,V_RCP_F16,V_SQRT_F16,V_RSQ_F16,V_LOG_F16,
            case 88:  case 89:  case 90:  case 91:   // V_EXP_F16,V_FREXP_MANT_F16,V_FREXP_EXP_I16_F16,V_FLOOR_F16,
            case 92:  case 93:  case 94:  case 95:  case 96:   // V_CEIL_F16,V_TRUNC_F16,V_RNDNE_F16,V_FRACT_F16,V_SIN_F16,
            case 97:  case 98:  case 99:  case 100:   // V_COS_F16,V_SAT_PK_U8_I16,V_CVT_NORM_I16_F16,V_CVT_NORM_U16_F16,
            case 105:  case 106:  case 107:  case 108:   // V_NOT_B16,V_CVT_I32_I16,V_CVT_U32_U16,V_CVT_F32_FP8,
            case 109:  case 114:  case 115:  case 116:   // V_CVT_F32_BF8,V_CVT_F32_BF16,V_SAT_PK4_I4_I8,V_SAT_PK4_U4_U8,
            case 117:  case 118:  case 119:  case 120:   // V_CVT_PK_F16_FP8,V_CVT_PK_F16_BF8,V_CVT_F16_FP8,V_CVT_F16_BF8,
            case 121:  case 122:  case 123:  case 124:  case 125:   // V_RCP_BF16,V_SQRT_BF16,V_RSQ_BF16,V_LOG_BF16,V_EXP_BF16,
            case 126:  case 127:   // V_SIN_BF16,V_COS_BF16,
                appendOPR_VGPR(layout.VDST,false,true);
                appendOPR_VGPR(layout.VSRC0,true,false);
                break;
            case 66:  case 67:  case 68:  case 72:   // V_MOVRELD_B32,V_MOVRELS_B32,V_MOVRELSD_B32,V_MOVRELSD_2_B32,
                appendOPR_VGPR(layout.VDST,false,true);
                appendOPR_VGPR(layout.VSRC0,true,false);
                appendOPR_SDST_M0(125,true,false,1,true);
                break;
        }
    }

    void InstructionDecoder_amdgpu_gfx1250::finalizeVOP1_VOP_DPP16Operands()
    {
        layout_VOP1_VOP_DPP16 & layout = insn_layout.VOP1_VOP_DPP16;
        switch (layout.OP)  {
            case 1:  case 5:  case 6:  case 7:  case 8:   // V_MOV_B32,V_CVT_F32_I32,V_CVT_F32_U32,V_CVT_U32_F32,V_CVT_I32_F32,
            case 10:  case 11:  case 12:  case 13:   // V_CVT_F16_F32,V_CVT_F32_F16,V_CVT_NEAREST_I32_F32,V_CVT_FLOOR_I32_F32,
            case 14:  case 17:  case 18:  case 19:   // V_CVT_OFF_F32_I4,V_CVT_F32_UBYTE0,V_CVT_F32_UBYTE1,V_CVT_F32_UBYTE2,
            case 20:  case 28:  case 30:  case 31:  case 32:   // V_CVT_F32_UBYTE3,V_MOV_B16,V_TANH_F32,V_TANH_F16,V_FRACT_F32,
            case 33:  case 34:  case 35:  case 36:  case 37:   // V_TRUNC_F32,V_CEIL_F32,V_RNDNE_F32,V_FLOOR_F32,V_EXP_F32,
            case 39:  case 42:  case 43:  case 46:  case 51:   // V_LOG_F32,V_RCP_F32,V_RCP_IFLAG_F32,V_RSQ_F32,V_SQRT_F32,
            case 53:  case 54:  case 55:  case 56:  case 57:   // V_SIN_F32,V_COS_F32,V_NOT_B32,V_BFREV_B32,V_CLZ_I32_U32,
            case 58:  case 59:  case 63:  case 64:   // V_CTZ_I32_B32,V_CLS_I32,V_FREXP_EXP_I32_F32,V_FREXP_MANT_F32,
            case 74:  case 75:  case 80:  case 81:  case 82:   // V_TANH_BF16,V_PRNG_B32,V_CVT_F16_U16,V_CVT_F16_I16,V_CVT_U16_F16,
            case 83:  case 84:  case 85:  case 86:  case 87:   // V_CVT_I16_F16,V_RCP_F16,V_SQRT_F16,V_RSQ_F16,V_LOG_F16,
            case 88:  case 89:  case 90:  case 91:   // V_EXP_F16,V_FREXP_MANT_F16,V_FREXP_EXP_I16_F16,V_FLOOR_F16,
            case 92:  case 93:  case 94:  case 95:  case 96:   // V_CEIL_F16,V_TRUNC_F16,V_RNDNE_F16,V_FRACT_F16,V_SIN_F16,
            case 97:  case 98:  case 99:  case 100:   // V_COS_F16,V_SAT_PK_U8_I16,V_CVT_NORM_I16_F16,V_CVT_NORM_U16_F16,
            case 105:  case 106:  case 107:  case 108:   // V_NOT_B16,V_CVT_I32_I16,V_CVT_U32_U16,V_CVT_F32_FP8,
            case 109:  case 114:  case 115:  case 116:   // V_CVT_F32_BF8,V_CVT_F32_BF16,V_SAT_PK4_I4_I8,V_SAT_PK4_U4_U8,
            case 117:  case 118:  case 119:  case 120:   // V_CVT_PK_F16_FP8,V_CVT_PK_F16_BF8,V_CVT_F16_FP8,V_CVT_F16_BF8,
            case 121:  case 122:  case 123:  case 124:  case 125:   // V_RCP_BF16,V_SQRT_BF16,V_RSQ_BF16,V_LOG_BF16,V_EXP_BF16,
            case 126:  case 127:   // V_SIN_BF16,V_COS_BF16,
                appendOPR_VGPR(layout.VDST,false,true);
                appendOPR_VGPR(layout.VSRC0,true,false);
                break;
            case 3:  case 15:  case 21:  case 60:   // V_CVT_I32_F64,V_CVT_F32_F64,V_CVT_U32_F64,V_FREXP_EXP_I32_F64,
                appendOPR_VGPR(layout.VDST,false,true);
                appendOPR_VGPR(layout.VSRC0,true,false,2);
                break;
            case 4:  case 16:  case 22:   // V_CVT_F64_I32,V_CVT_F64_F32,V_CVT_F64_U32,
                appendOPR_VGPR(layout.VDST,false,true,2);
                appendOPR_VGPR(layout.VSRC0,true,false);
                break;
            case 23:  case 24:  case 25:  case 26:  case 29:   // V_TRUNC_F64,V_CEIL_F64,V_RNDNE_F64,V_FLOOR_F64,V_MOV_B64,
            case 61:  case 62:   // V_FREXP_MANT_F64,V_FRACT_F64,
                appendOPR_VGPR(layout.VDST,false,true,2);
                appendOPR_VGPR(layout.VSRC0,true,false,2);
                break;
            case 66:  case 67:  case 68:  case 72:   // V_MOVRELD_B32,V_MOVRELS_B32,V_MOVRELSD_B32,V_MOVRELSD_2_B32,
                appendOPR_VGPR(layout.VDST,false,true);
                appendOPR_VGPR(layout.VSRC0,true,false);
                appendOPR_SDST_M0(125,true,false,1,true);
                break;
        }
    }

    void InstructionDecoder_amdgpu_gfx1250::finalizeVOP3_INST_LITERALOperands()
    {
        layout_VOP3_INST_LITERAL & layout = insn_layout.VOP3_INST_LITERAL;
        switch (layout.OP)  {
            case 385:  case 387:  case 389:  case 390:   // V_MOV_B32,V_CVT_I32_F64,V_CVT_F32_I32,V_CVT_F32_U32,
            case 391:  case 392:  case 394:  case 395:   // V_CVT_U32_F32,V_CVT_I32_F32,V_CVT_F16_F32,V_CVT_F32_F16,
            case 396:  case 397:  case 398:  case 399:   // V_CVT_NEAREST_I32_F32,V_CVT_FLOOR_I32_F32,V_CVT_OFF_F32_I4,V_CVT_F32_F64,
            case 401:  case 402:  case 403:  case 404:   // V_CVT_F32_UBYTE0,V_CVT_F32_UBYTE1,V_CVT_F32_UBYTE2,V_CVT_F32_UBYTE3,
            case 405:  case 412:  case 414:  case 415:  case 416:   // V_CVT_U32_F64,V_MOV_B16,V_TANH_F32,V_TANH_F16,V_FRACT_F32,
            case 417:  case 418:  case 419:  case 420:  case 421:   // V_TRUNC_F32,V_CEIL_F32,V_RNDNE_F32,V_FLOOR_F32,V_EXP_F32,
            case 423:  case 426:  case 427:  case 430:  case 435:   // V_LOG_F32,V_RCP_F32,V_RCP_IFLAG_F32,V_RSQ_F32,V_SQRT_F32,
            case 437:  case 438:  case 439:  case 440:  case 441:   // V_SIN_F32,V_COS_F32,V_NOT_B32,V_BFREV_B32,V_CLZ_I32_U32,
            case 442:  case 443:  case 444:  case 447:   // V_CTZ_I32_B32,V_CLS_I32,V_FREXP_EXP_I32_F64,V_FREXP_EXP_I32_F32,
            case 448:  case 458:  case 459:  case 464:   // V_FREXP_MANT_F32,V_TANH_BF16,V_PRNG_B32,V_CVT_F16_U16,
            case 465:  case 466:  case 467:  case 468:  case 469:   // V_CVT_F16_I16,V_CVT_U16_F16,V_CVT_I16_F16,V_RCP_F16,V_SQRT_F16,
            case 470:  case 471:  case 472:  case 473:   // V_RSQ_F16,V_LOG_F16,V_EXP_F16,V_FREXP_MANT_F16,
            case 474:  case 475:  case 476:  case 477:   // V_FREXP_EXP_I16_F16,V_FLOOR_F16,V_CEIL_F16,V_TRUNC_F16,
            case 478:  case 479:  case 480:  case 481:  case 482:   // V_RNDNE_F16,V_FRACT_F16,V_SIN_F16,V_COS_F16,V_SAT_PK_U8_I16,
            case 483:  case 484:  case 489:  case 490:   // V_CVT_NORM_I16_F16,V_CVT_NORM_U16_F16,V_NOT_B16,V_CVT_I32_I16,
            case 491:  case 492:  case 493:  case 498:   // V_CVT_U32_U16,V_CVT_F32_FP8,V_CVT_F32_BF8,V_CVT_F32_BF16,
            case 499:  case 500:  case 501:  case 502:   // V_SAT_PK4_I4_I8,V_SAT_PK4_U4_U8,V_CVT_PK_F16_FP8,V_CVT_PK_F16_BF8,
            case 503:  case 504:  case 505:  case 506:  case 507:   // V_CVT_F16_FP8,V_CVT_F16_BF8,V_RCP_BF16,V_SQRT_BF16,V_RSQ_BF16,
            case 508:  case 509:  case 510:  case 511:  case 882:   // V_LOG_BF16,V_EXP_BF16,V_SIN_BF16,V_COS_BF16,V_CVT_PK_FP8_F16,
            case 883:   // V_CVT_PK_BF8_F16,
                appendOPR_VGPR(layout.VDST,false,true);
                insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u32,layout.LITERAL)),true,false);
                break;
            case 388:  case 400:  case 406:  case 407:   // V_CVT_F64_I32,V_CVT_F64_F32,V_CVT_F64_U32,V_TRUNC_F64,
            case 408:  case 409:  case 410:  case 413:  case 431:   // V_CEIL_F64,V_RNDNE_F64,V_FLOOR_F64,V_MOV_B64,V_RCP_F64,
            case 433:  case 436:  case 445:  case 446:   // V_RSQ_F64,V_SQRT_F64,V_FREXP_MANT_F64,V_FRACT_F64,
            case 494:  case 495:   // V_CVT_PK_F32_FP8,V_CVT_PK_F32_BF8,
                appendOPR_VGPR(layout.VDST,false,true,2);
                insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u32,layout.LITERAL)),true,false);
                break;
            case 450:   // V_MOVRELD_B32,
                appendOPR_VGPR(layout.VDST,false,true);
                insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u32,layout.LITERAL)),true,false);
                appendOPR_SDST_M0(125,true,false,1,true);
                break;
            case 257:  case 605:   // V_CNDMASK_B32,V_CNDMASK_B16,
                if (COND_VOP3_INST_LITERAL_has_lit_0(insn_long))  {
                    appendOPR_VGPR(layout.VDST,false,true);
                    insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u32,layout.LITERAL)),true,false);
                    appendOPR_SRC(layout.SRC1,true,false);
                    appendOPR_SREG(layout.SRC2,true,false);
                }
                else if (COND_VOP3_INST_LITERAL_has_lit_1(insn_long))  {
                    appendOPR_VGPR(layout.VDST,false,true);
                    appendOPR_SRC(layout.SRC0,true,false);
                    insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u32,layout.LITERAL)),true,false);
                    appendOPR_SREG(layout.SRC2,true,false);
                }
                else  {
                    appendOPR_VGPR(layout.VDST,false,true);
                    insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u32,layout.LITERAL)),true,false);
                    insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u32,layout.LITERAL)),true,false);
                    appendOPR_SREG(layout.SRC2,true,false);
                }
                break;
            case 258:  case 262:  case 269:  case 270:  case 279:   // V_ADD_F64,V_MUL_F64,V_MIN_NUM_F64,V_MAX_NUM_F64,V_FMAC_F64,
            case 296:  case 297:  case 298:  case 792:  case 793:   // V_ADD_NC_U64,V_SUB_NC_U64,V_MUL_U64,V_MIN_U64,V_MAX_U64,
            case 794:  case 795:  case 833:  case 834:   // V_MIN_I64,V_MAX_I64,V_MINIMUM_F64,V_MAXIMUM_F64,
                if (COND_VOP3_INST_LITERAL_has_lit_0(insn_long))  {
                    appendOPR_VGPR(layout.VDST,false,true,2);
                    insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u32,layout.LITERAL)),true,false);
                    appendOPR_SRC(layout.SRC1,true,false,2);
                }
                else if (COND_VOP3_INST_LITERAL_has_lit_1(insn_long))  {
                    appendOPR_VGPR(layout.VDST,false,true,2);
                    appendOPR_SRC(layout.SRC0,true,false,2);
                    insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u32,layout.LITERAL)),true,false);
                }
                else  {
                    appendOPR_VGPR(layout.VDST,false,true,2);
                    insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u32,layout.LITERAL)),true,false);
                    insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u32,layout.LITERAL)),true,false);
                }
                break;
            case 259:  case 260:  case 261:  case 263:  case 264:   // V_ADD_F32,V_SUB_F32,V_SUBREV_F32,V_MUL_DX9_ZERO_F32,V_MUL_F32,
            case 265:  case 266:  case 267:  case 268:   // V_MUL_I32_I24,V_MUL_HI_I32_I24,V_MUL_U32_U24,V_MUL_HI_U32_U24,
            case 273:  case 274:  case 275:  case 276:  case 277:   // V_MIN_I32,V_MAX_I32,V_MIN_U32,V_MAX_U32,V_MIN_NUM_F32,
            case 278:  case 280:  case 281:  case 282:   // V_MAX_NUM_F32,V_LSHLREV_B32,V_LSHRREV_B32,V_ASHRREV_I32,
            case 283:  case 284:  case 285:  case 286:  case 293:   // V_AND_B32,V_OR_B32,V_XOR_B32,V_XNOR_B32,V_ADD_NC_U32,
            case 294:  case 295:  case 299:  case 303:   // V_SUB_NC_U32,V_SUBREV_NC_U32,V_FMAC_F32,V_CVT_PK_RTZ_F16_F32,
            case 304:  case 305:  case 306:  case 307:  case 308:   // V_MIN_NUM_F16,V_MAX_NUM_F16,V_ADD_F16,V_SUB_F16,V_SUBREV_F16,
            case 309:  case 310:  case 315:  case 771:  case 772:   // V_MUL_F16,V_FMAC_F16,V_LDEXP_F16,V_ADD_NC_U16,V_SUB_NC_U16,
            case 773:  case 774:  case 775:  case 777:   // V_MUL_LO_U16,V_CVT_PK_I16_F32,V_CVT_PK_U16_F32,V_MAX_U16,
            case 778:  case 779:  case 780:  case 781:  case 782:   // V_MAX_I16,V_MIN_U16,V_MIN_I16,V_ADD_NC_I16,V_SUB_NC_I16,
            case 785:  case 786:  case 787:  case 796:   // V_PACK_B32_F16,V_CVT_PK_NORM_I16_F16,V_CVT_PK_NORM_U16_F16,V_LDEXP_F32,
            case 797:  case 798:  case 799:  case 800:   // V_BFM_B32,V_BCNT_U32_B32,V_MBCNT_LO_U32_B32,V_MBCNT_HI_U32_B32,
            case 801:  case 802:  case 803:   // V_CVT_PK_NORM_I16_F32,V_CVT_PK_NORM_U16_F32,V_CVT_PK_U16_U32,
            case 804:  case 805:  case 806:  case 812:   // V_CVT_PK_I16_I32,V_SUB_NC_I32,V_ADD_NC_I32,V_MUL_LO_U32,
            case 813:  case 814:  case 824:  case 825:   // V_MUL_HI_U32,V_MUL_HI_I32,V_LSHLREV_B16,V_LSHRREV_B16,
            case 826:  case 866:  case 867:  case 868:  case 869:   // V_ASHRREV_I16,V_AND_B16,V_OR_B16,V_XOR_B16,V_MINIMUM_F32,
            case 870:  case 871:  case 872:  case 873:   // V_MAXIMUM_F32,V_MINIMUM_F16,V_MAXIMUM_F16,V_CVT_PK_FP8_F32,
            case 874:  case 875:  case 876:  case 877:   // V_CVT_PK_BF8_F32,V_CVT_SR_FP8_F32,V_CVT_SR_BF8_F32,V_CVT_PK_BF16_F32,
            case 879:  case 884:  case 885:   // V_CVT_PK_F16_F32,V_CVT_SR_FP8_F16,V_CVT_SR_BF8_F16,
                if (COND_VOP3_INST_LITERAL_has_lit_0(insn_long))  {
                    appendOPR_VGPR(layout.VDST,false,true);
                    insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u32,layout.LITERAL)),true,false);
                    appendOPR_SRC(layout.SRC1,true,false);
                }
                else if (COND_VOP3_INST_LITERAL_has_lit_1(insn_long))  {
                    appendOPR_VGPR(layout.VDST,false,true);
                    appendOPR_SRC(layout.SRC0,true,false);
                    insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u32,layout.LITERAL)),true,false);
                }
                else  {
                    appendOPR_VGPR(layout.VDST,false,true);
                    insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u32,layout.LITERAL)),true,false);
                    insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u32,layout.LITERAL)),true,false);
                }
                break;
            case 287:  case 829:  case 830:   // V_LSHLREV_B64,V_LSHRREV_B64,V_ASHRREV_I64,
                if (COND_VOP3_INST_LITERAL_has_lit_0(insn_long))  {
                    appendOPR_VGPR(layout.VDST,false,true,2);
                    insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u32,layout.LITERAL)),true,false);
                    appendOPR_SRC(layout.SRC1,true,false,2);
                }
                else  {
                    appendOPR_VGPR(layout.VDST,false,true,2);
                    appendOPR_SRC(layout.SRC0,true,false);
                    insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u32,layout.LITERAL)),true,false);
                }
                break;
            case 521:  case 522:  case 523:  case 524:   // V_FMA_DX9_ZERO_F32,V_MAD_I32_I24,V_MAD_U32_U24,V_CUBEID_F32,
            case 525:  case 526:  case 527:  case 528:  case 529:   // V_CUBESC_F32,V_CUBETC_F32,V_CUBEMA_F32,V_BFE_U32,V_BFE_I32,
            case 530:  case 531:  case 533:  case 534:  case 535:   // V_BFI_B32,V_FMA_F32,V_LERP_U8,V_ALIGNBIT_B32,V_ALIGNBYTE_B32,
            case 536:  case 538:  case 539:  case 541:  case 542:   // V_MULLIT_F32,V_MIN3_I32,V_MIN3_U32,V_MAX3_I32,V_MAX3_U32,
            case 544:  case 545:  case 546:  case 547:  case 548:   // V_MED3_I32,V_MED3_U32,V_SAD_U8,V_SAD_HI_U8,V_SAD_U16,
            case 549:  case 550:  case 551:  case 553:   // V_SAD_U32,V_CVT_PK_U8_F32,V_DIV_FIXUP_F32,V_MIN3_NUM_F32,
            case 554:  case 555:  case 556:  case 557:   // V_MAX3_NUM_F32,V_MIN3_NUM_F16,V_MAX3_NUM_F16,V_MINIMUM3_F32,
            case 558:  case 559:  case 560:  case 561:   // V_MAXIMUM3_F32,V_MINIMUM3_F16,V_MAXIMUM3_F16,V_MED3_NUM_F32,
            case 562:  case 563:  case 564:  case 565:  case 569:   // V_MED3_NUM_F16,V_BITOP3_B16,V_BITOP3_B32,V_MAD_U32,V_MSAD_U8,
            case 576:  case 577:  case 580:  case 581:  case 582:   // V_XOR3_B32,V_MAD_U16,V_PERM_B32,V_XAD_U32,V_LSHL_ADD_U32,
            case 583:  case 584:  case 586:  case 587:  case 589:   // V_ADD_LSHL_U32,V_FMA_F16,V_MIN3_I16,V_MIN3_U16,V_MAX3_I16,
            case 590:  case 592:  case 593:  case 595:  case 596:   // V_MAX3_U16,V_MED3_I16,V_MED3_U16,V_MAD_I16,V_DIV_FIXUP_F16,
            case 597:  case 598:  case 599:  case 600:  case 601:   // V_ADD3_U32,V_LSHL_OR_B32,V_AND_OR_B32,V_OR3_B32,V_MAD_U32_U16,
            case 602:  case 606:  case 607:  case 608:   // V_MAD_I32_I16,V_ADD_MAX_I32,V_ADD_MAX_U32,V_ADD_MIN_I32,
            case 609:  case 610:  case 611:  case 612:   // V_ADD_MIN_U32,V_MAXMIN_U32,V_MINMAX_U32,V_MAXMIN_I32,
            case 613:  case 616:  case 617:  case 618:   // V_MINMAX_I32,V_MINMAX_NUM_F32,V_MAXMIN_NUM_F32,V_MINMAX_NUM_F16,
            case 619:  case 620:  case 621:   // V_MAXMIN_NUM_F16,V_MINIMUMMAXIMUM_F32,V_MAXIMUMMINIMUM_F32,
            case 622:  case 623:  case 656:   // V_MINIMUMMAXIMUM_F16,V_MAXIMUMMINIMUM_F16,V_ASHR_PK_I8_I32,
            case 657:  case 878:  case 880:   // V_ASHR_PK_U8_I32,V_CVT_SR_PK_BF16_F32,V_CVT_SR_PK_F16_F32,
                if (COND_VOP3_INST_LITERAL_has_lit_0(insn_long))  {
                    appendOPR_VGPR(layout.VDST,false,true);
                    insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u32,layout.LITERAL)),true,false);
                    appendOPR_SRC(layout.SRC1,true,false);
                    appendOPR_SRC(layout.SRC2,true,false);
                }
                else if (COND_VOP3_INST_LITERAL_has_lit_1(insn_long))  {
                    appendOPR_VGPR(layout.VDST,false,true);
                    appendOPR_SRC(layout.SRC0,true,false);
                    insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u32,layout.LITERAL)),true,false);
                    appendOPR_SRC(layout.SRC2,true,false);
                }
                else if (COND_VOP3_INST_LITERAL_has_lit_0_and_has_lit_1(insn_long))  {
                    appendOPR_VGPR(layout.VDST,false,true);
                    insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u32,layout.LITERAL)),true,false);
                    insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u32,layout.LITERAL)),true,false);
                    appendOPR_SRC(layout.SRC2,true,false);
                }
                else if (COND_VOP3_INST_LITERAL_has_lit_2(insn_long))  {
                    appendOPR_VGPR(layout.VDST,false,true);
                    appendOPR_SRC(layout.SRC0,true,false);
                    appendOPR_SRC(layout.SRC1,true,false);
                    insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u32,layout.LITERAL)),true,false);
                }
                else if (COND_VOP3_INST_LITERAL_has_lit_0_and_has_lit_2(insn_long))  {
                    appendOPR_VGPR(layout.VDST,false,true);
                    insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u32,layout.LITERAL)),true,false);
                    appendOPR_SRC(layout.SRC1,true,false);
                    insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u32,layout.LITERAL)),true,false);
                }
                else if (COND_VOP3_INST_LITERAL_has_lit_1_and_has_lit_2(insn_long))  {
                    appendOPR_VGPR(layout.VDST,false,true);
                    appendOPR_SRC(layout.SRC0,true,false);
                    insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u32,layout.LITERAL)),true,false);
                    insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u32,layout.LITERAL)),true,false);
                }
                else  {
                    appendOPR_VGPR(layout.VDST,false,true);
                    insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u32,layout.LITERAL)),true,false);
                    insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u32,layout.LITERAL)),true,false);
                    insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u32,layout.LITERAL)),true,false);
                }
                break;
            case 532:  case 552:   // V_FMA_F64,V_DIV_FIXUP_F64,
                if (COND_VOP3_INST_LITERAL_has_lit_0(insn_long))  {
                    appendOPR_VGPR(layout.VDST,false,true,2);
                    insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u32,layout.LITERAL)),true,false);
                    appendOPR_SRC(layout.SRC1,true,false,2);
                    appendOPR_SRC(layout.SRC2,true,false,2);
                }
                else if (COND_VOP3_INST_LITERAL_has_lit_1(insn_long))  {
                    appendOPR_VGPR(layout.VDST,false,true,2);
                    appendOPR_SRC(layout.SRC0,true,false,2);
                    insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u32,layout.LITERAL)),true,false);
                    appendOPR_SRC(layout.SRC2,true,false,2);
                }
                else if (COND_VOP3_INST_LITERAL_has_lit_0_and_has_lit_1(insn_long))  {
                    appendOPR_VGPR(layout.VDST,false,true,2);
                    insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u32,layout.LITERAL)),true,false);
                    insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u32,layout.LITERAL)),true,false);
                    appendOPR_SRC(layout.SRC2,true,false,2);
                }
                else if (COND_VOP3_INST_LITERAL_has_lit_2(insn_long))  {
                    appendOPR_VGPR(layout.VDST,false,true,2);
                    appendOPR_SRC(layout.SRC0,true,false,2);
                    appendOPR_SRC(layout.SRC1,true,false,2);
                    insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u32,layout.LITERAL)),true,false);
                }
                else if (COND_VOP3_INST_LITERAL_has_lit_0_and_has_lit_2(insn_long))  {
                    appendOPR_VGPR(layout.VDST,false,true,2);
                    insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u32,layout.LITERAL)),true,false);
                    appendOPR_SRC(layout.SRC1,true,false,2);
                    insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u32,layout.LITERAL)),true,false);
                }
                else if (COND_VOP3_INST_LITERAL_has_lit_1_and_has_lit_2(insn_long))  {
                    appendOPR_VGPR(layout.VDST,false,true,2);
                    appendOPR_SRC(layout.SRC0,true,false,2);
                    insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u32,layout.LITERAL)),true,false);
                    insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u32,layout.LITERAL)),true,false);
                }
                else  {
                    appendOPR_VGPR(layout.VDST,false,true,2);
                    insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u32,layout.LITERAL)),true,false);
                    insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u32,layout.LITERAL)),true,false);
                    insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u32,layout.LITERAL)),true,false);
                }
                break;
            case 567:   // V_DIV_FMAS_F32,
                if (COND_VOP3_INST_LITERAL_has_lit_0(insn_long))  {
                    appendOPR_VGPR(layout.VDST,false,true);
                    insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u32,layout.LITERAL)),true,false);
                    appendOPR_SRC(layout.SRC1,true,false);
                    appendOPR_SRC(layout.SRC2,true,false);
                    appendOPR_VCC(106,true,false,1,true);
                }
                else if (COND_VOP3_INST_LITERAL_has_lit_1(insn_long))  {
                    appendOPR_VGPR(layout.VDST,false,true);
                    appendOPR_SRC(layout.SRC0,true,false);
                    insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u32,layout.LITERAL)),true,false);
                    appendOPR_SRC(layout.SRC2,true,false);
                    appendOPR_VCC(106,true,false,1,true);
                }
                else if (COND_VOP3_INST_LITERAL_has_lit_0_and_has_lit_1(insn_long))  {
                    appendOPR_VGPR(layout.VDST,false,true);
                    insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u32,layout.LITERAL)),true,false);
                    insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u32,layout.LITERAL)),true,false);
                    appendOPR_SRC(layout.SRC2,true,false);
                    appendOPR_VCC(106,true,false,1,true);
                }
                else if (COND_VOP3_INST_LITERAL_has_lit_2(insn_long))  {
                    appendOPR_VGPR(layout.VDST,false,true);
                    appendOPR_SRC(layout.SRC0,true,false);
                    appendOPR_SRC(layout.SRC1,true,false);
                    insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u32,layout.LITERAL)),true,false);
                    appendOPR_VCC(106,true,false,1,true);
                }
                else if (COND_VOP3_INST_LITERAL_has_lit_0_and_has_lit_2(insn_long))  {
                    appendOPR_VGPR(layout.VDST,false,true);
                    insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u32,layout.LITERAL)),true,false);
                    appendOPR_SRC(layout.SRC1,true,false);
                    insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u32,layout.LITERAL)),true,false);
                    appendOPR_VCC(106,true,false,1,true);
                }
                else if (COND_VOP3_INST_LITERAL_has_lit_1_and_has_lit_2(insn_long))  {
                    appendOPR_VGPR(layout.VDST,false,true);
                    appendOPR_SRC(layout.SRC0,true,false);
                    insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u32,layout.LITERAL)),true,false);
                    insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u32,layout.LITERAL)),true,false);
                    appendOPR_VCC(106,true,false,1,true);
                }
                else  {
                    appendOPR_VGPR(layout.VDST,false,true);
                    insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u32,layout.LITERAL)),true,false);
                    insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u32,layout.LITERAL)),true,false);
                    insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u32,layout.LITERAL)),true,false);
                    appendOPR_VCC(106,true,false,1,true);
                }
                break;
            case 568:   // V_DIV_FMAS_F64,
                if (COND_VOP3_INST_LITERAL_has_lit_0(insn_long))  {
                    appendOPR_VGPR(layout.VDST,false,true,2);
                    insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u32,layout.LITERAL)),true,false);
                    appendOPR_SRC(layout.SRC1,true,false,2);
                    appendOPR_SRC(layout.SRC2,true,false,2);
                    appendOPR_VCC(106,true,false,1,true);
                }
                else if (COND_VOP3_INST_LITERAL_has_lit_1(insn_long))  {
                    appendOPR_VGPR(layout.VDST,false,true,2);
                    appendOPR_SRC(layout.SRC0,true,false,2);
                    insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u32,layout.LITERAL)),true,false);
                    appendOPR_SRC(layout.SRC2,true,false,2);
                    appendOPR_VCC(106,true,false,1,true);
                }
                else if (COND_VOP3_INST_LITERAL_has_lit_0_and_has_lit_1(insn_long))  {
                    appendOPR_VGPR(layout.VDST,false,true,2);
                    insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u32,layout.LITERAL)),true,false);
                    insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u32,layout.LITERAL)),true,false);
                    appendOPR_SRC(layout.SRC2,true,false,2);
                    appendOPR_VCC(106,true,false,1,true);
                }
                else if (COND_VOP3_INST_LITERAL_has_lit_2(insn_long))  {
                    appendOPR_VGPR(layout.VDST,false,true,2);
                    appendOPR_SRC(layout.SRC0,true,false,2);
                    appendOPR_SRC(layout.SRC1,true,false,2);
                    insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u32,layout.LITERAL)),true,false);
                    appendOPR_VCC(106,true,false,1,true);
                }
                else if (COND_VOP3_INST_LITERAL_has_lit_0_and_has_lit_2(insn_long))  {
                    appendOPR_VGPR(layout.VDST,false,true,2);
                    insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u32,layout.LITERAL)),true,false);
                    appendOPR_SRC(layout.SRC1,true,false,2);
                    insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u32,layout.LITERAL)),true,false);
                    appendOPR_VCC(106,true,false,1,true);
                }
                else if (COND_VOP3_INST_LITERAL_has_lit_1_and_has_lit_2(insn_long))  {
                    appendOPR_VGPR(layout.VDST,false,true,2);
                    appendOPR_SRC(layout.SRC0,true,false,2);
                    insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u32,layout.LITERAL)),true,false);
                    insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u32,layout.LITERAL)),true,false);
                    appendOPR_VCC(106,true,false,1,true);
                }
                else  {
                    appendOPR_VGPR(layout.VDST,false,true,2);
                    insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u32,layout.LITERAL)),true,false);
                    insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u32,layout.LITERAL)),true,false);
                    insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u32,layout.LITERAL)),true,false);
                    appendOPR_VCC(106,true,false,1,true);
                }
                break;
            case 570:  case 571:  case 594:   // V_QSAD_PK_U16_U8,V_MQSAD_PK_U16_U8,V_LSHL_ADD_U64,
                if (COND_VOP3_INST_LITERAL_has_lit_0(insn_long))  {
                    appendOPR_VGPR(layout.VDST,false,true,2);
                    insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u32,layout.LITERAL)),true,false);
                    appendOPR_SRC(layout.SRC1,true,false);
                    appendOPR_SRC(layout.SRC2,true,false,2);
                }
                else if (COND_VOP3_INST_LITERAL_has_lit_1(insn_long))  {
                    appendOPR_VGPR(layout.VDST,false,true,2);
                    appendOPR_SRC(layout.SRC0,true,false,2);
                    insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u32,layout.LITERAL)),true,false);
                    appendOPR_SRC(layout.SRC2,true,false,2);
                }
                else if (COND_VOP3_INST_LITERAL_has_lit_0_and_has_lit_1(insn_long))  {
                    appendOPR_VGPR(layout.VDST,false,true,2);
                    insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u32,layout.LITERAL)),true,false);
                    insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u32,layout.LITERAL)),true,false);
                    appendOPR_SRC(layout.SRC2,true,false,2);
                }
                else if (COND_VOP3_INST_LITERAL_has_lit_2(insn_long))  {
                    appendOPR_VGPR(layout.VDST,false,true,2);
                    appendOPR_SRC(layout.SRC0,true,false,2);
                    appendOPR_SRC(layout.SRC1,true,false);
                    insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u32,layout.LITERAL)),true,false);
                }
                else if (COND_VOP3_INST_LITERAL_has_lit_0_and_has_lit_2(insn_long))  {
                    appendOPR_VGPR(layout.VDST,false,true,2);
                    insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u32,layout.LITERAL)),true,false);
                    appendOPR_SRC(layout.SRC1,true,false);
                    insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u32,layout.LITERAL)),true,false);
                }
                else if (COND_VOP3_INST_LITERAL_has_lit_1_and_has_lit_2(insn_long))  {
                    appendOPR_VGPR(layout.VDST,false,true,2);
                    appendOPR_SRC(layout.SRC0,true,false,2);
                    insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u32,layout.LITERAL)),true,false);
                    insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u32,layout.LITERAL)),true,false);
                }
                else  {
                    appendOPR_VGPR(layout.VDST,false,true,2);
                    insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u32,layout.LITERAL)),true,false);
                    insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u32,layout.LITERAL)),true,false);
                    insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u32,layout.LITERAL)),true,false);
                }
                break;
            case 573:   // V_MQSAD_U32_U8,
                if (COND_VOP3_INST_LITERAL_has_lit_0(insn_long))  {
                    appendOPR_VGPR(layout.VDST,false,true,4);
                    insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u32,layout.LITERAL)),true,false);
                    appendOPR_SRC(layout.SRC1,true,false);
                    appendOPR_SRC_VGPR(layout.SRC2,true,false,4);
                }
                else if (COND_VOP3_INST_LITERAL_has_lit_1(insn_long))  {
                    appendOPR_VGPR(layout.VDST,false,true,4);
                    appendOPR_SRC(layout.SRC0,true,false,2);
                    insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u32,layout.LITERAL)),true,false);
                    appendOPR_SRC_VGPR(layout.SRC2,true,false,4);
                }
                else  {
                    appendOPR_VGPR(layout.VDST,false,true,4);
                    insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u32,layout.LITERAL)),true,false);
                    insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u32,layout.LITERAL)),true,false);
                    appendOPR_SRC_VGPR(layout.SRC2,true,false,4);
                }
                break;
            case 575:  case 762:  case 763:   // V_PERM_PK16_B4_U4,V_MAD_NC_U64_U32,V_MAD_NC_I64_I32,
                if (COND_VOP3_INST_LITERAL_has_lit_0(insn_long))  {
                    appendOPR_VGPR(layout.VDST,false,true,2);
                    insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u32,layout.LITERAL)),true,false);
                    appendOPR_SRC(layout.SRC1,true,false);
                    appendOPR_SRC(layout.SRC2,true,false,2);
                }
                else if (COND_VOP3_INST_LITERAL_has_lit_1(insn_long))  {
                    appendOPR_VGPR(layout.VDST,false,true,2);
                    appendOPR_SRC(layout.SRC0,true,false);
                    insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u32,layout.LITERAL)),true,false);
                    appendOPR_SRC(layout.SRC2,true,false,2);
                }
                else if (COND_VOP3_INST_LITERAL_has_lit_0_and_has_lit_1(insn_long))  {
                    appendOPR_VGPR(layout.VDST,false,true,2);
                    insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u32,layout.LITERAL)),true,false);
                    insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u32,layout.LITERAL)),true,false);
                    appendOPR_SRC(layout.SRC2,true,false,2);
                }
                else if (COND_VOP3_INST_LITERAL_has_lit_2(insn_long))  {
                    appendOPR_VGPR(layout.VDST,false,true,2);
                    appendOPR_SRC(layout.SRC0,true,false);
                    appendOPR_SRC(layout.SRC1,true,false);
                    insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u32,layout.LITERAL)),true,false);
                }
                else if (COND_VOP3_INST_LITERAL_has_lit_0_and_has_lit_2(insn_long))  {
                    appendOPR_VGPR(layout.VDST,false,true,2);
                    insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u32,layout.LITERAL)),true,false);
                    appendOPR_SRC(layout.SRC1,true,false);
                    insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u32,layout.LITERAL)),true,false);
                }
                else if (COND_VOP3_INST_LITERAL_has_lit_1_and_has_lit_2(insn_long))  {
                    appendOPR_VGPR(layout.VDST,false,true,2);
                    appendOPR_SRC(layout.SRC0,true,false);
                    insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u32,layout.LITERAL)),true,false);
                    insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u32,layout.LITERAL)),true,false);
                }
                else  {
                    appendOPR_VGPR(layout.VDST,false,true,2);
                    insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u32,layout.LITERAL)),true,false);
                    insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u32,layout.LITERAL)),true,false);
                    insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u32,layout.LITERAL)),true,false);
                }
                break;
            case 578:   // V_PERM_PK16_B6_U4,
                if (COND_VOP3_INST_LITERAL_has_lit_0(insn_long))  {
                    appendOPR_VGPR(layout.VDST,false,true,3);
                    insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u32,layout.LITERAL)),true,false);
                    appendOPR_SRC(layout.SRC1,true,false,2);
                    appendOPR_SRC(layout.SRC2,true,false,2);
                }
                else if (COND_VOP3_INST_LITERAL_has_lit_1(insn_long))  {
                    appendOPR_VGPR(layout.VDST,false,true,3);
                    appendOPR_SRC(layout.SRC0,true,false);
                    insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u32,layout.LITERAL)),true,false);
                    appendOPR_SRC(layout.SRC2,true,false,2);
                }
                else if (COND_VOP3_INST_LITERAL_has_lit_0_and_has_lit_1(insn_long))  {
                    appendOPR_VGPR(layout.VDST,false,true,3);
                    insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u32,layout.LITERAL)),true,false);
                    insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u32,layout.LITERAL)),true,false);
                    appendOPR_SRC(layout.SRC2,true,false,2);
                }
                else if (COND_VOP3_INST_LITERAL_has_lit_2(insn_long))  {
                    appendOPR_VGPR(layout.VDST,false,true,3);
                    appendOPR_SRC(layout.SRC0,true,false);
                    appendOPR_SRC(layout.SRC1,true,false,2);
                    insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u32,layout.LITERAL)),true,false);
                }
                else if (COND_VOP3_INST_LITERAL_has_lit_0_and_has_lit_2(insn_long))  {
                    appendOPR_VGPR(layout.VDST,false,true,3);
                    insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u32,layout.LITERAL)),true,false);
                    appendOPR_SRC(layout.SRC1,true,false,2);
                    insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u32,layout.LITERAL)),true,false);
                }
                else if (COND_VOP3_INST_LITERAL_has_lit_1_and_has_lit_2(insn_long))  {
                    appendOPR_VGPR(layout.VDST,false,true,3);
                    appendOPR_SRC(layout.SRC0,true,false);
                    insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u32,layout.LITERAL)),true,false);
                    insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u32,layout.LITERAL)),true,false);
                }
                else  {
                    appendOPR_VGPR(layout.VDST,false,true,3);
                    insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u32,layout.LITERAL)),true,false);
                    insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u32,layout.LITERAL)),true,false);
                    insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u32,layout.LITERAL)),true,false);
                }
                break;
            case 579:   // V_PERM_PK16_B8_U4,
                if (COND_VOP3_INST_LITERAL_has_lit_0(insn_long))  {
                    appendOPR_VGPR(layout.VDST,false,true,4);
                    insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u32,layout.LITERAL)),true,false);
                    appendOPR_SRC(layout.SRC1,true,false,2);
                    appendOPR_SRC(layout.SRC2,true,false,2);
                }
                else if (COND_VOP3_INST_LITERAL_has_lit_1(insn_long))  {
                    appendOPR_VGPR(layout.VDST,false,true,4);
                    appendOPR_SRC(layout.SRC0,true,false,2);
                    insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u32,layout.LITERAL)),true,false);
                    appendOPR_SRC(layout.SRC2,true,false,2);
                }
                else if (COND_VOP3_INST_LITERAL_has_lit_0_and_has_lit_1(insn_long))  {
                    appendOPR_VGPR(layout.VDST,false,true,4);
                    insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u32,layout.LITERAL)),true,false);
                    insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u32,layout.LITERAL)),true,false);
                    appendOPR_SRC(layout.SRC2,true,false,2);
                }
                else if (COND_VOP3_INST_LITERAL_has_lit_2(insn_long))  {
                    appendOPR_VGPR(layout.VDST,false,true,4);
                    appendOPR_SRC(layout.SRC0,true,false,2);
                    appendOPR_SRC(layout.SRC1,true,false,2);
                    insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u32,layout.LITERAL)),true,false);
                }
                else if (COND_VOP3_INST_LITERAL_has_lit_0_and_has_lit_2(insn_long))  {
                    appendOPR_VGPR(layout.VDST,false,true,4);
                    insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u32,layout.LITERAL)),true,false);
                    appendOPR_SRC(layout.SRC1,true,false,2);
                    insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u32,layout.LITERAL)),true,false);
                }
                else if (COND_VOP3_INST_LITERAL_has_lit_1_and_has_lit_2(insn_long))  {
                    appendOPR_VGPR(layout.VDST,false,true,4);
                    appendOPR_SRC(layout.SRC0,true,false,2);
                    insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u32,layout.LITERAL)),true,false);
                    insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u32,layout.LITERAL)),true,false);
                }
                else  {
                    appendOPR_VGPR(layout.VDST,false,true,4);
                    insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u32,layout.LITERAL)),true,false);
                    insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u32,layout.LITERAL)),true,false);
                    insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u32,layout.LITERAL)),true,false);
                }
                break;
            case 603:  case 604:  case 624:  case 625:   // V_PERMLANE16_B32,V_PERMLANEX16_B32,V_PERMLANE_BCAST_B32,V_PERMLANE_UP_B32,
            case 626:  case 627:   // V_PERMLANE_DOWN_B32,V_PERMLANE_XOR_B32,
                if (COND_VOP3_INST_LITERAL_has_lit_1(insn_long))  {
                    appendOPR_VGPR(layout.VDST,false,true);
                    appendOPR_SRC_VGPR(layout.SRC0,true,false);
                    insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u32,layout.LITERAL)),true,false);
                    appendOPR_SSRC(layout.SRC2,true,false);
                }
                else if (COND_VOP3_INST_LITERAL_has_lit_2(insn_long))  {
                    appendOPR_VGPR(layout.VDST,false,true);
                    appendOPR_SRC_VGPR(layout.SRC0,true,false);
                    appendOPR_SSRC(layout.SRC1,true,false);
                    insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u32,layout.LITERAL)),true,false);
                }
                else  {
                    appendOPR_VGPR(layout.VDST,false,true);
                    appendOPR_SRC_VGPR(layout.SRC0,true,false);
                    insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u32,layout.LITERAL)),true,false);
                    insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u32,layout.LITERAL)),true,false);
                }
                break;
            case 640:  case 641:  case 642:  case 643:  case 644:   // V_S_EXP_F32,V_S_EXP_F16,V_S_LOG_F32,V_S_LOG_F16,V_S_RCP_F32,
            case 645:  case 646:  case 647:  case 648:  case 649:   // V_S_RCP_F16,V_S_RSQ_F32,V_S_RSQ_F16,V_S_SQRT_F32,V_S_SQRT_F16,
                appendOPR_SREG(layout.VDST,false,true);
                insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u32,layout.LITERAL)),true,false);
                break;
            case 663:   // V_CVT_SCALEF32_SR_PK8_FP4_F32,
                if (COND_VOP3_INST_LITERAL_has_lit_1(insn_long))  {
                    appendOPR_VGPR(layout.VDST,false,true);
                    appendOPR_SRC_VGPR(layout.SRC0,true,false,8);
                    insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u32,layout.LITERAL)),true,false);
                    appendOPR_SRC(layout.SRC2,true,false);
                }
                else if (COND_VOP3_INST_LITERAL_has_lit_2(insn_long))  {
                    appendOPR_VGPR(layout.VDST,false,true);
                    appendOPR_SRC_VGPR(layout.SRC0,true,false,8);
                    appendOPR_SRC(layout.SRC1,true,false);
                    insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u32,layout.LITERAL)),true,false);
                }
                else  {
                    appendOPR_VGPR(layout.VDST,false,true);
                    appendOPR_SRC_VGPR(layout.SRC0,true,false,8);
                    insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u32,layout.LITERAL)),true,false);
                    insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u32,layout.LITERAL)),true,false);
                }
                break;
            case 664:  case 665:   // V_CVT_SCALEF32_SR_PK8_FP8_F32,V_CVT_SCALEF32_SR_PK8_BF8_F32,
                if (COND_VOP3_INST_LITERAL_has_lit_1(insn_long))  {
                    appendOPR_VGPR(layout.VDST,false,true,2);
                    appendOPR_SRC_VGPR(layout.SRC0,true,false,8);
                    insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u32,layout.LITERAL)),true,false);
                    appendOPR_SRC(layout.SRC2,true,false);
                }
                else if (COND_VOP3_INST_LITERAL_has_lit_2(insn_long))  {
                    appendOPR_VGPR(layout.VDST,false,true,2);
                    appendOPR_SRC_VGPR(layout.SRC0,true,false,8);
                    appendOPR_SRC(layout.SRC1,true,false);
                    insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u32,layout.LITERAL)),true,false);
                }
                else  {
                    appendOPR_VGPR(layout.VDST,false,true,2);
                    appendOPR_SRC_VGPR(layout.SRC0,true,false,8);
                    insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u32,layout.LITERAL)),true,false);
                    insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u32,layout.LITERAL)),true,false);
                }
                break;
            case 671:  case 672:   // V_CVT_SCALE_PK8_F16_FP4,V_CVT_SCALE_PK8_BF16_FP4,
                appendOPR_VGPR(layout.VDST,false,true,4);
                appendOPR_SRC_VGPR(layout.SRC0,true,false);
                insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u32,layout.LITERAL)),true,false);
                break;
            case 673:   // V_CVT_SCALE_PK8_F32_FP4,
                appendOPR_VGPR(layout.VDST,false,true,8);
                appendOPR_SRC_VGPR(layout.SRC0,true,false);
                insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u32,layout.LITERAL)),true,false);
                break;
            case 680:  case 681:  case 683:   // V_CVT_SCALE_PK8_F16_FP8,V_CVT_SCALE_PK8_BF16_FP8,V_CVT_SCALE_PK8_F16_BF8,
            case 684:   // V_CVT_SCALE_PK8_BF16_BF8,
                appendOPR_VGPR(layout.VDST,false,true,4);
                appendOPR_SRC_VGPR(layout.SRC0,true,false,2);
                insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u32,layout.LITERAL)),true,false);
                break;
            case 682:  case 685:   // V_CVT_SCALE_PK8_F32_FP8,V_CVT_SCALE_PK8_F32_BF8,
                appendOPR_VGPR(layout.VDST,false,true,8);
                appendOPR_SRC_VGPR(layout.SRC0,true,false,2);
                insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u32,layout.LITERAL)),true,false);
                break;
            case 688:   // V_CVT_SCALEF32_PK8_FP4_F32,
                appendOPR_VGPR(layout.VDST,false,true);
                appendOPR_SRC_VGPR(layout.SRC0,true,false,8);
                insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u32,layout.LITERAL)),true,false);
                break;
            case 691:  case 696:   // V_CVT_SCALEF32_PK8_FP4_F16,V_CVT_SCALEF32_PK8_FP4_BF16,
                appendOPR_VGPR(layout.VDST,false,true);
                appendOPR_SRC_VGPR(layout.SRC0,true,false,4);
                insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u32,layout.LITERAL)),true,false);
                break;
            case 692:  case 693:  case 708:   // V_CVT_SCALEF32_PK8_FP8_BF16,V_CVT_SCALEF32_PK8_BF8_BF16,V_CVT_SCALEF32_PK8_FP8_F16,
            case 710:   // V_CVT_SCALEF32_PK8_BF8_F16,
                appendOPR_VGPR(layout.VDST,false,true,2);
                appendOPR_SRC_VGPR(layout.SRC0,true,false,4);
                insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u32,layout.LITERAL)),true,false);
                break;
            case 697:  case 700:   // V_CVT_SCALEF32_SR_PK8_FP4_F16,V_CVT_SCALEF32_SR_PK8_FP4_BF16,
                if (COND_VOP3_INST_LITERAL_has_lit_1(insn_long))  {
                    appendOPR_VGPR(layout.VDST,false,true);
                    appendOPR_SRC_VGPR(layout.SRC0,true,false,4);
                    insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u32,layout.LITERAL)),true,false);
                    appendOPR_SRC(layout.SRC2,true,false);
                }
                else if (COND_VOP3_INST_LITERAL_has_lit_2(insn_long))  {
                    appendOPR_VGPR(layout.VDST,false,true);
                    appendOPR_SRC_VGPR(layout.SRC0,true,false,4);
                    appendOPR_SRC(layout.SRC1,true,false);
                    insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u32,layout.LITERAL)),true,false);
                }
                else  {
                    appendOPR_VGPR(layout.VDST,false,true);
                    appendOPR_SRC_VGPR(layout.SRC0,true,false,4);
                    insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u32,layout.LITERAL)),true,false);
                    insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u32,layout.LITERAL)),true,false);
                }
                break;
            case 703:  case 704:   // V_CVT_SCALEF32_SR_PK8_FP8_F16,V_CVT_SCALEF32_SR_PK8_FP8_BF16,
            case 705:  case 706:   // V_CVT_SCALEF32_SR_PK8_BF8_F16,V_CVT_SCALEF32_SR_PK8_BF8_BF16,
                if (COND_VOP3_INST_LITERAL_has_lit_1(insn_long))  {
                    appendOPR_VGPR(layout.VDST,false,true,2);
                    appendOPR_SRC_VGPR(layout.SRC0,true,false,4);
                    insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u32,layout.LITERAL)),true,false);
                    appendOPR_SRC(layout.SRC2,true,false);
                }
                else if (COND_VOP3_INST_LITERAL_has_lit_2(insn_long))  {
                    appendOPR_VGPR(layout.VDST,false,true,2);
                    appendOPR_SRC_VGPR(layout.SRC0,true,false,4);
                    appendOPR_SRC(layout.SRC1,true,false);
                    insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u32,layout.LITERAL)),true,false);
                }
                else  {
                    appendOPR_VGPR(layout.VDST,false,true,2);
                    appendOPR_SRC_VGPR(layout.SRC0,true,false,4);
                    insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u32,layout.LITERAL)),true,false);
                    insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u32,layout.LITERAL)),true,false);
                }
                break;
            case 707:  case 709:   // V_CVT_SCALEF32_PK8_FP8_F32,V_CVT_SCALEF32_PK8_BF8_F32,
                appendOPR_VGPR(layout.VDST,false,true,2);
                appendOPR_SRC_VGPR(layout.SRC0,true,false,8);
                insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u32,layout.LITERAL)),true,false);
                break;
            case 711:  case 712:  case 714:   // V_CVT_SCALE_PK16_F16_FP6,V_CVT_SCALE_PK16_BF16_FP6,V_CVT_SCALE_PK16_F16_BF6,
            case 715:   // V_CVT_SCALE_PK16_BF16_BF6,
                appendOPR_VGPR(layout.VDST,false,true,8);
                appendOPR_SRC_VGPR(layout.SRC0,true,false,3);
                insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u32,layout.LITERAL)),true,false);
                break;
            case 713:  case 716:   // V_CVT_SCALE_PK16_F32_FP6,V_CVT_SCALE_PK16_F32_BF6,
                appendOPR_VGPR(layout.VDST,false,true,16);
                appendOPR_SRC_VGPR(layout.SRC0,true,false,3);
                insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u32,layout.LITERAL)),true,false);
                break;
            case 717:  case 718:   // V_CVT_SCALEF32_PK16_FP6_F32,V_CVT_SCALEF32_PK16_BF6_F32,
                appendOPR_VGPR(layout.VDST,false,true,3);
                appendOPR_SRC_VGPR(layout.SRC0,true,false,16);
                insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u32,layout.LITERAL)),true,false);
                break;
            case 719:  case 720:  case 721:   // V_CVT_SCALEF32_PK16_FP6_F16,V_CVT_SCALEF32_PK16_BF6_F16,V_CVT_SCALEF32_PK16_FP6_BF16,
            case 722:   // V_CVT_SCALEF32_PK16_BF6_BF16,
                appendOPR_VGPR(layout.VDST,false,true,3);
                appendOPR_SRC_VGPR(layout.SRC0,true,false,8);
                insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u32,layout.LITERAL)),true,false);
                break;
            case 723:  case 724:   // V_CVT_SCALEF32_SR_PK16_FP6_F32,V_CVT_SCALEF32_SR_PK16_BF6_F32,
                if (COND_VOP3_INST_LITERAL_has_lit_1(insn_long))  {
                    appendOPR_VGPR(layout.VDST,false,true,3);
                    appendOPR_SRC_VGPR(layout.SRC0,true,false,16);
                    insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u32,layout.LITERAL)),true,false);
                    appendOPR_SRC(layout.SRC2,true,false);
                }
                else if (COND_VOP3_INST_LITERAL_has_lit_2(insn_long))  {
                    appendOPR_VGPR(layout.VDST,false,true,3);
                    appendOPR_SRC_VGPR(layout.SRC0,true,false,16);
                    appendOPR_SRC(layout.SRC1,true,false);
                    insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u32,layout.LITERAL)),true,false);
                }
                else  {
                    appendOPR_VGPR(layout.VDST,false,true,3);
                    appendOPR_SRC_VGPR(layout.SRC0,true,false,16);
                    insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u32,layout.LITERAL)),true,false);
                    insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u32,layout.LITERAL)),true,false);
                }
                break;
            case 725:  case 726:   // V_CVT_SCALEF32_SR_PK16_FP6_F16,V_CVT_SCALEF32_SR_PK16_BF6_F16,
            case 727:  case 728:   // V_CVT_SCALEF32_SR_PK16_FP6_BF16,V_CVT_SCALEF32_SR_PK16_BF6_BF16,
                if (COND_VOP3_INST_LITERAL_has_lit_1(insn_long))  {
                    appendOPR_VGPR(layout.VDST,false,true,3);
                    appendOPR_SRC_VGPR(layout.SRC0,true,false,8);
                    insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u32,layout.LITERAL)),true,false);
                    appendOPR_SRC(layout.SRC2,true,false);
                }
                else if (COND_VOP3_INST_LITERAL_has_lit_2(insn_long))  {
                    appendOPR_VGPR(layout.VDST,false,true,3);
                    appendOPR_SRC_VGPR(layout.SRC0,true,false,8);
                    appendOPR_SRC(layout.SRC1,true,false);
                    insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u32,layout.LITERAL)),true,false);
                }
                else  {
                    appendOPR_VGPR(layout.VDST,false,true,3);
                    appendOPR_SRC_VGPR(layout.SRC0,true,false,8);
                    insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u32,layout.LITERAL)),true,false);
                    insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u32,layout.LITERAL)),true,false);
                }
                break;
            case 788:   // V_PERMLANE_IDX_GEN_B32,
                appendOPR_VGPR(layout.VDST,false,true);
                appendOPR_SRC_VGPR(layout.SRC0,true,false);
                insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u32,layout.LITERAL)),true,false);
                break;
            case 811:  case 815:   // V_LDEXP_F64,V_TRIG_PREOP_F64,
                if (COND_VOP3_INST_LITERAL_has_lit_0(insn_long))  {
                    appendOPR_VGPR(layout.VDST,false,true,2);
                    insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u32,layout.LITERAL)),true,false);
                    appendOPR_SRC(layout.SRC1,true,false);
                }
                else if (COND_VOP3_INST_LITERAL_has_lit_1(insn_long))  {
                    appendOPR_VGPR(layout.VDST,false,true,2);
                    appendOPR_SRC(layout.SRC0,true,false,2);
                    insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u32,layout.LITERAL)),true,false);
                }
                else  {
                    appendOPR_VGPR(layout.VDST,false,true,2);
                    insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u32,layout.LITERAL)),true,false);
                    insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u32,layout.LITERAL)),true,false);
                }
                break;
            case 865:   // V_WRITELANE_B32,
                appendOPR_VGPR(layout.VDST,false,true);
                insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u32,layout.LITERAL)),true,false);
                appendOPR_SSRC_LANESEL(layout.SRC1,true,false);
                break;
            case 1:  case 2:  case 3:  case 4:  case 5:   // V_CMP_LT_F16,V_CMP_EQ_F16,V_CMP_LE_F16,V_CMP_GT_F16,V_CMP_LG_F16,
            case 6:  case 7:  case 8:  case 9:  case 10:   // V_CMP_GE_F16,V_CMP_O_F16,V_CMP_U_F16,V_CMP_NGE_F16,V_CMP_NLG_F16,
            case 11:  case 12:  case 13:  case 14:   // V_CMP_NGT_F16,V_CMP_NLE_F16,V_CMP_NEQ_F16,V_CMP_NLT_F16,
            case 17:  case 18:  case 19:  case 20:  case 21:   // V_CMP_LT_F32,V_CMP_EQ_F32,V_CMP_LE_F32,V_CMP_GT_F32,V_CMP_LG_F32,
            case 22:  case 23:  case 24:  case 25:  case 26:   // V_CMP_GE_F32,V_CMP_O_F32,V_CMP_U_F32,V_CMP_NGE_F32,V_CMP_NLG_F32,
            case 27:  case 28:  case 29:  case 30:   // V_CMP_NGT_F32,V_CMP_NLE_F32,V_CMP_NEQ_F32,V_CMP_NLT_F32,
            case 49:  case 50:  case 51:  case 52:  case 53:   // V_CMP_LT_I16,V_CMP_EQ_I16,V_CMP_LE_I16,V_CMP_GT_I16,V_CMP_NE_I16,
            case 54:  case 57:  case 58:  case 59:  case 60:   // V_CMP_GE_I16,V_CMP_LT_U16,V_CMP_EQ_U16,V_CMP_LE_U16,V_CMP_GT_U16,
            case 61:  case 62:  case 65:  case 66:  case 67:   // V_CMP_NE_U16,V_CMP_GE_U16,V_CMP_LT_I32,V_CMP_EQ_I32,V_CMP_LE_I32,
            case 68:  case 69:  case 70:  case 73:  case 74:   // V_CMP_GT_I32,V_CMP_NE_I32,V_CMP_GE_I32,V_CMP_LT_U32,V_CMP_EQ_U32,
            case 75:  case 76:  case 77:  case 78:   // V_CMP_LE_U32,V_CMP_GT_U32,V_CMP_NE_U32,V_CMP_GE_U32,
            case 125:  case 126:   // V_CMP_CLASS_F16,V_CMP_CLASS_F32,
                if (COND_VOP3_INST_LITERAL_has_lit_0(insn_long))  {
                    appendOPR_SREG(layout.VDST,false,true);
                    insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u32,layout.LITERAL)),true,false);
                    appendOPR_SRC(layout.SRC1,true,false);
                }
                else if (COND_VOP3_INST_LITERAL_has_lit_1(insn_long))  {
                    appendOPR_SREG(layout.VDST,false,true);
                    appendOPR_SRC(layout.SRC0,true,false);
                    insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u32,layout.LITERAL)),true,false);
                }
                else  {
                    appendOPR_SREG(layout.VDST,false,true);
                    insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u32,layout.LITERAL)),true,false);
                    insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u32,layout.LITERAL)),true,false);
                }
                break;
            case 33:  case 34:  case 35:  case 36:  case 37:   // V_CMP_LT_F64,V_CMP_EQ_F64,V_CMP_LE_F64,V_CMP_GT_F64,V_CMP_LG_F64,
            case 38:  case 39:  case 40:  case 41:  case 42:   // V_CMP_GE_F64,V_CMP_O_F64,V_CMP_U_F64,V_CMP_NGE_F64,V_CMP_NLG_F64,
            case 43:  case 44:  case 45:  case 46:   // V_CMP_NGT_F64,V_CMP_NLE_F64,V_CMP_NEQ_F64,V_CMP_NLT_F64,
            case 81:  case 82:  case 83:  case 84:  case 85:   // V_CMP_LT_I64,V_CMP_EQ_I64,V_CMP_LE_I64,V_CMP_GT_I64,V_CMP_NE_I64,
            case 86:  case 89:  case 90:  case 91:  case 92:   // V_CMP_GE_I64,V_CMP_LT_U64,V_CMP_EQ_U64,V_CMP_LE_U64,V_CMP_GT_U64,
            case 93:  case 94:   // V_CMP_NE_U64,V_CMP_GE_U64,
                if (COND_VOP3_INST_LITERAL_has_lit_0(insn_long))  {
                    appendOPR_SREG(layout.VDST,false,true);
                    insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u32,layout.LITERAL)),true,false);
                    appendOPR_SRC(layout.SRC1,true,false,2);
                }
                else if (COND_VOP3_INST_LITERAL_has_lit_1(insn_long))  {
                    appendOPR_SREG(layout.VDST,false,true);
                    appendOPR_SRC(layout.SRC0,true,false,2);
                    insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u32,layout.LITERAL)),true,false);
                }
                else  {
                    appendOPR_SREG(layout.VDST,false,true);
                    insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u32,layout.LITERAL)),true,false);
                    insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u32,layout.LITERAL)),true,false);
                }
                break;
            case 127:   // V_CMP_CLASS_F64,
                if (COND_VOP3_INST_LITERAL_has_lit_0(insn_long))  {
                    appendOPR_SREG(layout.VDST,false,true);
                    insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u32,layout.LITERAL)),true,false);
                    appendOPR_SRC(layout.SRC1,true,false);
                }
                else if (COND_VOP3_INST_LITERAL_has_lit_1(insn_long))  {
                    appendOPR_SREG(layout.VDST,false,true);
                    appendOPR_SRC(layout.SRC0,true,false,2);
                    insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u32,layout.LITERAL)),true,false);
                }
                else  {
                    appendOPR_SREG(layout.VDST,false,true);
                    insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u32,layout.LITERAL)),true,false);
                    insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u32,layout.LITERAL)),true,false);
                }
                break;
            case 129:  case 130:  case 131:  case 132:   // V_CMPX_LT_F16,V_CMPX_EQ_F16,V_CMPX_LE_F16,V_CMPX_GT_F16,
            case 133:  case 134:  case 135:  case 136:   // V_CMPX_LG_F16,V_CMPX_GE_F16,V_CMPX_O_F16,V_CMPX_U_F16,
            case 137:  case 138:  case 139:  case 140:   // V_CMPX_NGE_F16,V_CMPX_NLG_F16,V_CMPX_NGT_F16,V_CMPX_NLE_F16,
            case 141:  case 142:  case 145:  case 146:   // V_CMPX_NEQ_F16,V_CMPX_NLT_F16,V_CMPX_LT_F32,V_CMPX_EQ_F32,
            case 147:  case 148:  case 149:  case 150:   // V_CMPX_LE_F32,V_CMPX_GT_F32,V_CMPX_LG_F32,V_CMPX_GE_F32,
            case 151:  case 152:  case 153:  case 154:   // V_CMPX_O_F32,V_CMPX_U_F32,V_CMPX_NGE_F32,V_CMPX_NLG_F32,
            case 155:  case 156:  case 157:  case 158:   // V_CMPX_NGT_F32,V_CMPX_NLE_F32,V_CMPX_NEQ_F32,V_CMPX_NLT_F32,
            case 177:  case 178:  case 179:  case 180:   // V_CMPX_LT_I16,V_CMPX_EQ_I16,V_CMPX_LE_I16,V_CMPX_GT_I16,
            case 181:  case 182:  case 185:  case 186:   // V_CMPX_NE_I16,V_CMPX_GE_I16,V_CMPX_LT_U16,V_CMPX_EQ_U16,
            case 187:  case 188:  case 189:  case 190:   // V_CMPX_LE_U16,V_CMPX_GT_U16,V_CMPX_NE_U16,V_CMPX_GE_U16,
            case 193:  case 194:  case 195:  case 196:   // V_CMPX_LT_I32,V_CMPX_EQ_I32,V_CMPX_LE_I32,V_CMPX_GT_I32,
            case 197:  case 198:  case 201:  case 202:   // V_CMPX_NE_I32,V_CMPX_GE_I32,V_CMPX_LT_U32,V_CMPX_EQ_U32,
            case 203:  case 204:  case 205:  case 206:   // V_CMPX_LE_U32,V_CMPX_GT_U32,V_CMPX_NE_U32,V_CMPX_GE_U32,
            case 253:  case 254:   // V_CMPX_CLASS_F16,V_CMPX_CLASS_F32,
                if (COND_VOP3_INST_LITERAL_has_lit_0(insn_long))  {
                    appendOPR_EXEC(layout.VDST,false,true);
                    insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u32,layout.LITERAL)),true,false);
                    appendOPR_SRC(layout.SRC1,true,false);
                    appendOPR_SDST_EXEC(126,false,true,1,true);
                }
                else if (COND_VOP3_INST_LITERAL_has_lit_1(insn_long))  {
                    appendOPR_EXEC(layout.VDST,false,true);
                    appendOPR_SRC(layout.SRC0,true,false);
                    insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u32,layout.LITERAL)),true,false);
                    appendOPR_SDST_EXEC(126,false,true,1,true);
                }
                else  {
                    appendOPR_EXEC(layout.VDST,false,true);
                    insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u32,layout.LITERAL)),true,false);
                    insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u32,layout.LITERAL)),true,false);
                    appendOPR_SDST_EXEC(126,false,true,1,true);
                }
                break;
            case 161:  case 162:  case 163:  case 164:   // V_CMPX_LT_F64,V_CMPX_EQ_F64,V_CMPX_LE_F64,V_CMPX_GT_F64,
            case 165:  case 166:  case 167:  case 168:   // V_CMPX_LG_F64,V_CMPX_GE_F64,V_CMPX_O_F64,V_CMPX_U_F64,
            case 169:  case 170:  case 171:  case 172:   // V_CMPX_NGE_F64,V_CMPX_NLG_F64,V_CMPX_NGT_F64,V_CMPX_NLE_F64,
            case 173:  case 174:  case 209:  case 210:   // V_CMPX_NEQ_F64,V_CMPX_NLT_F64,V_CMPX_LT_I64,V_CMPX_EQ_I64,
            case 211:  case 212:  case 213:  case 214:   // V_CMPX_LE_I64,V_CMPX_GT_I64,V_CMPX_NE_I64,V_CMPX_GE_I64,
            case 217:  case 218:  case 219:  case 220:   // V_CMPX_LT_U64,V_CMPX_EQ_U64,V_CMPX_LE_U64,V_CMPX_GT_U64,
            case 221:  case 222:   // V_CMPX_NE_U64,V_CMPX_GE_U64,
                if (COND_VOP3_INST_LITERAL_has_lit_0(insn_long))  {
                    appendOPR_EXEC(layout.VDST,false,true);
                    insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u32,layout.LITERAL)),true,false);
                    appendOPR_SRC(layout.SRC1,true,false,2);
                    appendOPR_SDST_EXEC(126,false,true,1,true);
                }
                else if (COND_VOP3_INST_LITERAL_has_lit_1(insn_long))  {
                    appendOPR_EXEC(layout.VDST,false,true);
                    appendOPR_SRC(layout.SRC0,true,false,2);
                    insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u32,layout.LITERAL)),true,false);
                    appendOPR_SDST_EXEC(126,false,true,1,true);
                }
                else  {
                    appendOPR_EXEC(layout.VDST,false,true);
                    insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u32,layout.LITERAL)),true,false);
                    insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u32,layout.LITERAL)),true,false);
                    appendOPR_SDST_EXEC(126,false,true,1,true);
                }
                break;
            case 255:   // V_CMPX_CLASS_F64,
                if (COND_VOP3_INST_LITERAL_has_lit_0(insn_long))  {
                    appendOPR_EXEC(layout.VDST,false,true);
                    insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u32,layout.LITERAL)),true,false);
                    appendOPR_SRC(layout.SRC1,true,false);
                    appendOPR_SDST_EXEC(126,false,true,1,true);
                }
                else if (COND_VOP3_INST_LITERAL_has_lit_1(insn_long))  {
                    appendOPR_EXEC(layout.VDST,false,true);
                    appendOPR_SRC(layout.SRC0,true,false,2);
                    insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u32,layout.LITERAL)),true,false);
                    appendOPR_SDST_EXEC(126,false,true,1,true);
                }
                else  {
                    appendOPR_EXEC(layout.VDST,false,true);
                    insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u32,layout.LITERAL)),true,false);
                    insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u32,layout.LITERAL)),true,false);
                    appendOPR_SDST_EXEC(126,false,true,1,true);
                }
                break;
        }
    }

    void InstructionDecoder_amdgpu_gfx1250::finalizeVOP3_VOP_DPP8Operands()
    {
        layout_VOP3_VOP_DPP8 & layout = insn_layout.VOP3_VOP_DPP8;
        switch (layout.OP)  {
            case 385:  case 389:  case 390:  case 391:   // V_MOV_B32,V_CVT_F32_I32,V_CVT_F32_U32,V_CVT_U32_F32,
            case 392:  case 394:  case 395:  case 396:   // V_CVT_I32_F32,V_CVT_F16_F32,V_CVT_F32_F16,V_CVT_NEAREST_I32_F32,
            case 397:  case 398:  case 401:  case 402:   // V_CVT_FLOOR_I32_F32,V_CVT_OFF_F32_I4,V_CVT_F32_UBYTE0,V_CVT_F32_UBYTE1,
            case 403:  case 404:  case 412:  case 414:   // V_CVT_F32_UBYTE2,V_CVT_F32_UBYTE3,V_MOV_B16,V_TANH_F32,
            case 415:  case 416:  case 417:  case 418:  case 419:   // V_TANH_F16,V_FRACT_F32,V_TRUNC_F32,V_CEIL_F32,V_RNDNE_F32,
            case 420:  case 421:  case 423:  case 426:  case 427:   // V_FLOOR_F32,V_EXP_F32,V_LOG_F32,V_RCP_F32,V_RCP_IFLAG_F32,
            case 430:  case 435:  case 437:  case 438:  case 439:   // V_RSQ_F32,V_SQRT_F32,V_SIN_F32,V_COS_F32,V_NOT_B32,
            case 440:  case 441:  case 442:  case 443:   // V_BFREV_B32,V_CLZ_I32_U32,V_CTZ_I32_B32,V_CLS_I32,
            case 447:  case 448:  case 458:  case 459:   // V_FREXP_EXP_I32_F32,V_FREXP_MANT_F32,V_TANH_BF16,V_PRNG_B32,
            case 464:  case 465:  case 466:  case 467:   // V_CVT_F16_U16,V_CVT_F16_I16,V_CVT_U16_F16,V_CVT_I16_F16,
            case 468:  case 469:  case 470:  case 471:  case 472:   // V_RCP_F16,V_SQRT_F16,V_RSQ_F16,V_LOG_F16,V_EXP_F16,
            case 473:  case 474:  case 475:  case 476:   // V_FREXP_MANT_F16,V_FREXP_EXP_I16_F16,V_FLOOR_F16,V_CEIL_F16,
            case 477:  case 478:  case 479:  case 480:  case 481:   // V_TRUNC_F16,V_RNDNE_F16,V_FRACT_F16,V_SIN_F16,V_COS_F16,
            case 482:  case 483:  case 484:  case 489:   // V_SAT_PK_U8_I16,V_CVT_NORM_I16_F16,V_CVT_NORM_U16_F16,V_NOT_B16,
            case 490:  case 491:  case 492:  case 493:   // V_CVT_I32_I16,V_CVT_U32_U16,V_CVT_F32_FP8,V_CVT_F32_BF8,
            case 498:  case 499:  case 500:  case 501:   // V_CVT_F32_BF16,V_SAT_PK4_I4_I8,V_SAT_PK4_U4_U8,V_CVT_PK_F16_FP8,
            case 502:  case 503:  case 504:  case 505:   // V_CVT_PK_F16_BF8,V_CVT_F16_FP8,V_CVT_F16_BF8,V_RCP_BF16,
            case 506:  case 507:  case 508:  case 509:  case 510:   // V_SQRT_BF16,V_RSQ_BF16,V_LOG_BF16,V_EXP_BF16,V_SIN_BF16,
            case 511:  case 882:  case 883:   // V_COS_BF16,V_CVT_PK_FP8_F16,V_CVT_PK_BF8_F16,
                appendOPR_VGPR(layout.VDST,false,true);
                appendOPR_VGPR(layout.VSRC0,true,false);
                break;
            case 450:  case 451:  case 452:  case 456:   // V_MOVRELD_B32,V_MOVRELS_B32,V_MOVRELSD_B32,V_MOVRELSD_2_B32,
                appendOPR_VGPR(layout.VDST,false,true);
                appendOPR_VGPR(layout.VSRC0,true,false);
                appendOPR_SDST_M0(125,true,false,1,true);
                break;
            case 257:  case 605:   // V_CNDMASK_B32,V_CNDMASK_B16,
                appendOPR_VGPR(layout.VDST,false,true);
                appendOPR_VGPR(layout.VSRC0,true,false);
                appendOPR_SRC(layout.SRC1,true,false);
                appendOPR_SREG(layout.SRC2,true,false);
                break;
            case 259:  case 260:  case 263:  case 264:  case 265:   // V_ADD_F32,V_SUB_F32,V_MUL_DX9_ZERO_F32,V_MUL_F32,V_MUL_I32_I24,
            case 266:  case 267:  case 268:  case 273:   // V_MUL_HI_I32_I24,V_MUL_U32_U24,V_MUL_HI_U32_U24,V_MIN_I32,
            case 274:  case 275:  case 276:  case 277:  case 278:   // V_MAX_I32,V_MIN_U32,V_MAX_U32,V_MIN_NUM_F32,V_MAX_NUM_F32,
            case 280:  case 281:  case 282:  case 283:  case 284:   // V_LSHLREV_B32,V_LSHRREV_B32,V_ASHRREV_I32,V_AND_B32,V_OR_B32,
            case 285:  case 286:  case 293:  case 294:  case 299:   // V_XOR_B32,V_XNOR_B32,V_ADD_NC_U32,V_SUB_NC_U32,V_FMAC_F32,
            case 303:  case 304:  case 305:  case 306:   // V_CVT_PK_RTZ_F16_F32,V_MIN_NUM_F16,V_MAX_NUM_F16,V_ADD_F16,
            case 307:  case 309:  case 310:  case 315:  case 771:   // V_SUB_F16,V_MUL_F16,V_FMAC_F16,V_LDEXP_F16,V_ADD_NC_U16,
            case 772:  case 773:  case 774:  case 775:   // V_SUB_NC_U16,V_MUL_LO_U16,V_CVT_PK_I16_F32,V_CVT_PK_U16_F32,
            case 777:  case 778:  case 779:  case 780:  case 781:   // V_MAX_U16,V_MAX_I16,V_MIN_U16,V_MIN_I16,V_ADD_NC_I16,
            case 782:  case 785:  case 786:  case 787:   // V_SUB_NC_I16,V_PACK_B32_F16,V_CVT_PK_NORM_I16_F16,V_CVT_PK_NORM_U16_F16,
            case 796:  case 797:  case 798:  case 799:   // V_LDEXP_F32,V_BFM_B32,V_BCNT_U32_B32,V_MBCNT_LO_U32_B32,
            case 800:  case 801:  case 802:   // V_MBCNT_HI_U32_B32,V_CVT_PK_NORM_I16_F32,V_CVT_PK_NORM_U16_F32,
            case 803:  case 804:  case 805:  case 806:   // V_CVT_PK_U16_U32,V_CVT_PK_I16_I32,V_SUB_NC_I32,V_ADD_NC_I32,
            case 824:  case 825:  case 826:  case 866:  case 867:   // V_LSHLREV_B16,V_LSHRREV_B16,V_ASHRREV_I16,V_AND_B16,V_OR_B16,
            case 868:  case 869:  case 870:  case 871:   // V_XOR_B16,V_MINIMUM_F32,V_MAXIMUM_F32,V_MINIMUM_F16,
            case 872:  case 873:  case 874:  case 875:   // V_MAXIMUM_F16,V_CVT_PK_FP8_F32,V_CVT_PK_BF8_F32,V_CVT_SR_FP8_F32,
            case 876:  case 877:  case 879:  case 884:   // V_CVT_SR_BF8_F32,V_CVT_PK_BF16_F32,V_CVT_PK_F16_F32,V_CVT_SR_FP8_F16,
            case 885:   // V_CVT_SR_BF8_F16,
                appendOPR_VGPR(layout.VDST,false,true);
                appendOPR_VGPR(layout.VSRC0,true,false);
                appendOPR_SRC(layout.SRC1,true,false);
                break;
            case 261:  case 295:  case 308:   // V_SUBREV_F32,V_SUBREV_NC_U32,V_SUBREV_F16,
                appendOPR_VGPR(layout.VDST,false,true);
                appendOPR_VGPR(layout.VSRC0,true,false);
                appendOPR_SRC_VGPR(layout.SRC1,true,false);
                break;
            case 521:  case 522:  case 523:  case 524:   // V_FMA_DX9_ZERO_F32,V_MAD_I32_I24,V_MAD_U32_U24,V_CUBEID_F32,
            case 525:  case 526:  case 527:  case 528:  case 529:   // V_CUBESC_F32,V_CUBETC_F32,V_CUBEMA_F32,V_BFE_U32,V_BFE_I32,
            case 530:  case 531:  case 533:  case 534:  case 535:   // V_BFI_B32,V_FMA_F32,V_LERP_U8,V_ALIGNBIT_B32,V_ALIGNBYTE_B32,
            case 536:  case 538:  case 539:  case 541:  case 542:   // V_MULLIT_F32,V_MIN3_I32,V_MIN3_U32,V_MAX3_I32,V_MAX3_U32,
            case 544:  case 545:  case 546:  case 547:  case 548:   // V_MED3_I32,V_MED3_U32,V_SAD_U8,V_SAD_HI_U8,V_SAD_U16,
            case 549:  case 550:  case 553:  case 554:   // V_SAD_U32,V_CVT_PK_U8_F32,V_MIN3_NUM_F32,V_MAX3_NUM_F32,
            case 555:  case 556:  case 557:  case 558:   // V_MIN3_NUM_F16,V_MAX3_NUM_F16,V_MINIMUM3_F32,V_MAXIMUM3_F32,
            case 559:  case 560:  case 561:  case 562:   // V_MINIMUM3_F16,V_MAXIMUM3_F16,V_MED3_NUM_F32,V_MED3_NUM_F16,
            case 563:  case 564:  case 569:  case 576:  case 577:   // V_BITOP3_B16,V_BITOP3_B32,V_MSAD_U8,V_XOR3_B32,V_MAD_U16,
            case 580:  case 581:  case 582:  case 583:  case 584:   // V_PERM_B32,V_XAD_U32,V_LSHL_ADD_U32,V_ADD_LSHL_U32,V_FMA_F16,
            case 586:  case 587:  case 589:  case 590:  case 592:   // V_MIN3_I16,V_MIN3_U16,V_MAX3_I16,V_MAX3_U16,V_MED3_I16,
            case 593:  case 595:  case 596:  case 597:  case 598:   // V_MED3_U16,V_MAD_I16,V_DIV_FIXUP_F16,V_ADD3_U32,V_LSHL_OR_B32,
            case 599:  case 600:  case 601:  case 602:   // V_AND_OR_B32,V_OR3_B32,V_MAD_U32_U16,V_MAD_I32_I16,
            case 606:  case 607:  case 608:  case 609:   // V_ADD_MAX_I32,V_ADD_MAX_U32,V_ADD_MIN_I32,V_ADD_MIN_U32,
            case 610:  case 611:  case 612:  case 613:   // V_MAXMIN_U32,V_MINMAX_U32,V_MAXMIN_I32,V_MINMAX_I32,
            case 616:  case 617:  case 618:  case 619:   // V_MINMAX_NUM_F32,V_MAXMIN_NUM_F32,V_MINMAX_NUM_F16,V_MAXMIN_NUM_F16,
            case 620:  case 621:  case 622:   // V_MINIMUMMAXIMUM_F32,V_MAXIMUMMINIMUM_F32,V_MINIMUMMAXIMUM_F16,
            case 623:  case 656:  case 657:   // V_MAXIMUMMINIMUM_F16,V_ASHR_PK_I8_I32,V_ASHR_PK_U8_I32,
            case 878:  case 880:   // V_CVT_SR_PK_BF16_F32,V_CVT_SR_PK_F16_F32,
                appendOPR_VGPR(layout.VDST,false,true);
                appendOPR_VGPR(layout.VSRC0,true,false);
                appendOPR_SRC(layout.SRC1,true,false);
                appendOPR_SRC(layout.SRC2,true,false);
                break;
            case 1:  case 2:  case 3:  case 4:  case 5:   // V_CMP_LT_F16,V_CMP_EQ_F16,V_CMP_LE_F16,V_CMP_GT_F16,V_CMP_LG_F16,
            case 6:  case 7:  case 8:  case 9:  case 10:   // V_CMP_GE_F16,V_CMP_O_F16,V_CMP_U_F16,V_CMP_NGE_F16,V_CMP_NLG_F16,
            case 11:  case 12:  case 13:  case 14:   // V_CMP_NGT_F16,V_CMP_NLE_F16,V_CMP_NEQ_F16,V_CMP_NLT_F16,
            case 17:  case 18:  case 19:  case 20:  case 21:   // V_CMP_LT_F32,V_CMP_EQ_F32,V_CMP_LE_F32,V_CMP_GT_F32,V_CMP_LG_F32,
            case 22:  case 23:  case 24:  case 25:  case 26:   // V_CMP_GE_F32,V_CMP_O_F32,V_CMP_U_F32,V_CMP_NGE_F32,V_CMP_NLG_F32,
            case 27:  case 28:  case 29:  case 30:   // V_CMP_NGT_F32,V_CMP_NLE_F32,V_CMP_NEQ_F32,V_CMP_NLT_F32,
            case 49:  case 50:  case 51:  case 52:  case 53:   // V_CMP_LT_I16,V_CMP_EQ_I16,V_CMP_LE_I16,V_CMP_GT_I16,V_CMP_NE_I16,
            case 54:  case 57:  case 58:  case 59:  case 60:   // V_CMP_GE_I16,V_CMP_LT_U16,V_CMP_EQ_U16,V_CMP_LE_U16,V_CMP_GT_U16,
            case 61:  case 62:  case 65:  case 66:  case 67:   // V_CMP_NE_U16,V_CMP_GE_U16,V_CMP_LT_I32,V_CMP_EQ_I32,V_CMP_LE_I32,
            case 68:  case 69:  case 70:  case 73:  case 74:   // V_CMP_GT_I32,V_CMP_NE_I32,V_CMP_GE_I32,V_CMP_LT_U32,V_CMP_EQ_U32,
            case 75:  case 76:  case 77:  case 78:   // V_CMP_LE_U32,V_CMP_GT_U32,V_CMP_NE_U32,V_CMP_GE_U32,
            case 125:  case 126:   // V_CMP_CLASS_F16,V_CMP_CLASS_F32,
                appendOPR_SREG(layout.VDST,false,true);
                appendOPR_VGPR(layout.VSRC0,true,false);
                appendOPR_SRC(layout.SRC1,true,false);
                break;
            case 129:  case 130:  case 131:  case 132:   // V_CMPX_LT_F16,V_CMPX_EQ_F16,V_CMPX_LE_F16,V_CMPX_GT_F16,
            case 133:  case 134:  case 135:  case 136:   // V_CMPX_LG_F16,V_CMPX_GE_F16,V_CMPX_O_F16,V_CMPX_U_F16,
            case 137:  case 138:  case 139:  case 140:   // V_CMPX_NGE_F16,V_CMPX_NLG_F16,V_CMPX_NGT_F16,V_CMPX_NLE_F16,
            case 141:  case 142:  case 145:  case 146:   // V_CMPX_NEQ_F16,V_CMPX_NLT_F16,V_CMPX_LT_F32,V_CMPX_EQ_F32,
            case 147:  case 148:  case 149:  case 150:   // V_CMPX_LE_F32,V_CMPX_GT_F32,V_CMPX_LG_F32,V_CMPX_GE_F32,
            case 151:  case 152:  case 153:  case 154:   // V_CMPX_O_F32,V_CMPX_U_F32,V_CMPX_NGE_F32,V_CMPX_NLG_F32,
            case 155:  case 156:  case 157:  case 158:   // V_CMPX_NGT_F32,V_CMPX_NLE_F32,V_CMPX_NEQ_F32,V_CMPX_NLT_F32,
            case 177:  case 178:  case 179:  case 180:   // V_CMPX_LT_I16,V_CMPX_EQ_I16,V_CMPX_LE_I16,V_CMPX_GT_I16,
            case 181:  case 182:  case 185:  case 186:   // V_CMPX_NE_I16,V_CMPX_GE_I16,V_CMPX_LT_U16,V_CMPX_EQ_U16,
            case 187:  case 188:  case 189:  case 190:   // V_CMPX_LE_U16,V_CMPX_GT_U16,V_CMPX_NE_U16,V_CMPX_GE_U16,
            case 193:  case 194:  case 195:  case 196:   // V_CMPX_LT_I32,V_CMPX_EQ_I32,V_CMPX_LE_I32,V_CMPX_GT_I32,
            case 197:  case 198:  case 201:  case 202:   // V_CMPX_NE_I32,V_CMPX_GE_I32,V_CMPX_LT_U32,V_CMPX_EQ_U32,
            case 203:  case 204:  case 205:  case 206:   // V_CMPX_LE_U32,V_CMPX_GT_U32,V_CMPX_NE_U32,V_CMPX_GE_U32,
            case 253:  case 254:   // V_CMPX_CLASS_F16,V_CMPX_CLASS_F32,
                appendOPR_EXEC(layout.VDST,false,true);
                appendOPR_VGPR(layout.VSRC0,true,false);
                appendOPR_SRC(layout.SRC1,true,false);
                appendOPR_SDST_EXEC(126,false,true,1,true);
                break;
        }
    }

    void InstructionDecoder_amdgpu_gfx1250::finalizeVOP3_VOP_DPP16Operands()
    {
        layout_VOP3_VOP_DPP16 & layout = insn_layout.VOP3_VOP_DPP16;
        switch (layout.OP)  {
            case 385:  case 389:  case 390:  case 391:   // V_MOV_B32,V_CVT_F32_I32,V_CVT_F32_U32,V_CVT_U32_F32,
            case 392:  case 394:  case 395:  case 396:   // V_CVT_I32_F32,V_CVT_F16_F32,V_CVT_F32_F16,V_CVT_NEAREST_I32_F32,
            case 397:  case 398:  case 401:  case 402:   // V_CVT_FLOOR_I32_F32,V_CVT_OFF_F32_I4,V_CVT_F32_UBYTE0,V_CVT_F32_UBYTE1,
            case 403:  case 404:  case 412:  case 414:   // V_CVT_F32_UBYTE2,V_CVT_F32_UBYTE3,V_MOV_B16,V_TANH_F32,
            case 415:  case 416:  case 417:  case 418:  case 419:   // V_TANH_F16,V_FRACT_F32,V_TRUNC_F32,V_CEIL_F32,V_RNDNE_F32,
            case 420:  case 421:  case 423:  case 426:  case 427:   // V_FLOOR_F32,V_EXP_F32,V_LOG_F32,V_RCP_F32,V_RCP_IFLAG_F32,
            case 430:  case 435:  case 437:  case 438:  case 439:   // V_RSQ_F32,V_SQRT_F32,V_SIN_F32,V_COS_F32,V_NOT_B32,
            case 440:  case 441:  case 442:  case 443:   // V_BFREV_B32,V_CLZ_I32_U32,V_CTZ_I32_B32,V_CLS_I32,
            case 447:  case 448:  case 458:  case 459:   // V_FREXP_EXP_I32_F32,V_FREXP_MANT_F32,V_TANH_BF16,V_PRNG_B32,
            case 464:  case 465:  case 466:  case 467:   // V_CVT_F16_U16,V_CVT_F16_I16,V_CVT_U16_F16,V_CVT_I16_F16,
            case 468:  case 469:  case 470:  case 471:  case 472:   // V_RCP_F16,V_SQRT_F16,V_RSQ_F16,V_LOG_F16,V_EXP_F16,
            case 473:  case 474:  case 475:  case 476:   // V_FREXP_MANT_F16,V_FREXP_EXP_I16_F16,V_FLOOR_F16,V_CEIL_F16,
            case 477:  case 478:  case 479:  case 480:  case 481:   // V_TRUNC_F16,V_RNDNE_F16,V_FRACT_F16,V_SIN_F16,V_COS_F16,
            case 482:  case 483:  case 484:  case 489:   // V_SAT_PK_U8_I16,V_CVT_NORM_I16_F16,V_CVT_NORM_U16_F16,V_NOT_B16,
            case 490:  case 491:  case 492:  case 493:   // V_CVT_I32_I16,V_CVT_U32_U16,V_CVT_F32_FP8,V_CVT_F32_BF8,
            case 498:  case 499:  case 500:  case 501:   // V_CVT_F32_BF16,V_SAT_PK4_I4_I8,V_SAT_PK4_U4_U8,V_CVT_PK_F16_FP8,
            case 502:  case 503:  case 504:  case 505:   // V_CVT_PK_F16_BF8,V_CVT_F16_FP8,V_CVT_F16_BF8,V_RCP_BF16,
            case 506:  case 507:  case 508:  case 509:  case 510:   // V_SQRT_BF16,V_RSQ_BF16,V_LOG_BF16,V_EXP_BF16,V_SIN_BF16,
            case 511:  case 882:  case 883:   // V_COS_BF16,V_CVT_PK_FP8_F16,V_CVT_PK_BF8_F16,
                appendOPR_VGPR(layout.VDST,false,true);
                appendOPR_VGPR(layout.VSRC0,true,false);
                break;
            case 387:  case 399:  case 405:  case 444:   // V_CVT_I32_F64,V_CVT_F32_F64,V_CVT_U32_F64,V_FREXP_EXP_I32_F64,
                appendOPR_VGPR(layout.VDST,false,true);
                appendOPR_VGPR(layout.VSRC0,true,false,2);
                break;
            case 388:  case 400:  case 406:   // V_CVT_F64_I32,V_CVT_F64_F32,V_CVT_F64_U32,
                appendOPR_VGPR(layout.VDST,false,true,2);
                appendOPR_VGPR(layout.VSRC0,true,false);
                break;
            case 407:  case 408:  case 409:  case 410:  case 413:   // V_TRUNC_F64,V_CEIL_F64,V_RNDNE_F64,V_FLOOR_F64,V_MOV_B64,
            case 445:  case 446:   // V_FREXP_MANT_F64,V_FRACT_F64,
                appendOPR_VGPR(layout.VDST,false,true,2);
                appendOPR_VGPR(layout.VSRC0,true,false,2);
                break;
            case 450:  case 451:  case 452:  case 456:   // V_MOVRELD_B32,V_MOVRELS_B32,V_MOVRELSD_B32,V_MOVRELSD_2_B32,
                appendOPR_VGPR(layout.VDST,false,true);
                appendOPR_VGPR(layout.VSRC0,true,false);
                appendOPR_SDST_M0(125,true,false,1,true);
                break;
            case 257:  case 605:   // V_CNDMASK_B32,V_CNDMASK_B16,
                appendOPR_VGPR(layout.VDST,false,true);
                appendOPR_VGPR(layout.VSRC0,true,false);
                appendOPR_SRC(layout.SRC1,true,false);
                appendOPR_SREG(layout.SRC2,true,false);
                break;
            case 258:  case 262:  case 269:  case 270:  case 279:   // V_ADD_F64,V_MUL_F64,V_MIN_NUM_F64,V_MAX_NUM_F64,V_FMAC_F64,
            case 296:  case 297:  case 792:  case 793:  case 794:   // V_ADD_NC_U64,V_SUB_NC_U64,V_MIN_U64,V_MAX_U64,V_MIN_I64,
            case 795:  case 833:  case 834:   // V_MAX_I64,V_MINIMUM_F64,V_MAXIMUM_F64,
                appendOPR_VGPR(layout.VDST,false,true,2);
                appendOPR_VGPR(layout.VSRC0,true,false,2);
                appendOPR_SRC(layout.SRC1,true,false,2);
                break;
            case 259:  case 260:  case 263:  case 264:  case 265:   // V_ADD_F32,V_SUB_F32,V_MUL_DX9_ZERO_F32,V_MUL_F32,V_MUL_I32_I24,
            case 266:  case 267:  case 268:  case 273:   // V_MUL_HI_I32_I24,V_MUL_U32_U24,V_MUL_HI_U32_U24,V_MIN_I32,
            case 274:  case 275:  case 276:  case 277:  case 278:   // V_MAX_I32,V_MIN_U32,V_MAX_U32,V_MIN_NUM_F32,V_MAX_NUM_F32,
            case 280:  case 281:  case 282:  case 283:  case 284:   // V_LSHLREV_B32,V_LSHRREV_B32,V_ASHRREV_I32,V_AND_B32,V_OR_B32,
            case 285:  case 286:  case 293:  case 294:  case 299:   // V_XOR_B32,V_XNOR_B32,V_ADD_NC_U32,V_SUB_NC_U32,V_FMAC_F32,
            case 303:  case 304:  case 305:  case 306:   // V_CVT_PK_RTZ_F16_F32,V_MIN_NUM_F16,V_MAX_NUM_F16,V_ADD_F16,
            case 307:  case 309:  case 310:  case 315:  case 771:   // V_SUB_F16,V_MUL_F16,V_FMAC_F16,V_LDEXP_F16,V_ADD_NC_U16,
            case 772:  case 773:  case 774:  case 775:   // V_SUB_NC_U16,V_MUL_LO_U16,V_CVT_PK_I16_F32,V_CVT_PK_U16_F32,
            case 777:  case 778:  case 779:  case 780:  case 781:   // V_MAX_U16,V_MAX_I16,V_MIN_U16,V_MIN_I16,V_ADD_NC_I16,
            case 782:  case 785:  case 786:  case 787:   // V_SUB_NC_I16,V_PACK_B32_F16,V_CVT_PK_NORM_I16_F16,V_CVT_PK_NORM_U16_F16,
            case 796:  case 797:  case 798:  case 799:   // V_LDEXP_F32,V_BFM_B32,V_BCNT_U32_B32,V_MBCNT_LO_U32_B32,
            case 800:  case 801:  case 802:   // V_MBCNT_HI_U32_B32,V_CVT_PK_NORM_I16_F32,V_CVT_PK_NORM_U16_F32,
            case 803:  case 804:  case 805:  case 806:   // V_CVT_PK_U16_U32,V_CVT_PK_I16_I32,V_SUB_NC_I32,V_ADD_NC_I32,
            case 812:  case 813:  case 814:  case 824:   // V_MUL_LO_U32,V_MUL_HI_U32,V_MUL_HI_I32,V_LSHLREV_B16,
            case 825:  case 826:  case 866:  case 867:  case 868:   // V_LSHRREV_B16,V_ASHRREV_I16,V_AND_B16,V_OR_B16,V_XOR_B16,
            case 869:  case 870:  case 871:  case 872:   // V_MINIMUM_F32,V_MAXIMUM_F32,V_MINIMUM_F16,V_MAXIMUM_F16,
            case 873:  case 874:  case 875:  case 876:   // V_CVT_PK_FP8_F32,V_CVT_PK_BF8_F32,V_CVT_SR_FP8_F32,V_CVT_SR_BF8_F32,
            case 877:  case 879:  case 884:  case 885:   // V_CVT_PK_BF16_F32,V_CVT_PK_F16_F32,V_CVT_SR_FP8_F16,V_CVT_SR_BF8_F16,
                appendOPR_VGPR(layout.VDST,false,true);
                appendOPR_VGPR(layout.VSRC0,true,false);
                appendOPR_SRC(layout.SRC1,true,false);
                break;
            case 261:  case 295:  case 308:   // V_SUBREV_F32,V_SUBREV_NC_U32,V_SUBREV_F16,
                appendOPR_VGPR(layout.VDST,false,true);
                appendOPR_VGPR(layout.VSRC0,true,false);
                appendOPR_SRC_VGPR(layout.SRC1,true,false);
                break;
            case 287:  case 829:  case 830:   // V_LSHLREV_B64,V_LSHRREV_B64,V_ASHRREV_I64,
                appendOPR_VGPR(layout.VDST,false,true,2);
                appendOPR_VGPR(layout.VSRC0,true,false);
                appendOPR_SRC(layout.SRC1,true,false,2);
                break;
            case 521:  case 522:  case 523:  case 524:   // V_FMA_DX9_ZERO_F32,V_MAD_I32_I24,V_MAD_U32_U24,V_CUBEID_F32,
            case 525:  case 526:  case 527:  case 528:  case 529:   // V_CUBESC_F32,V_CUBETC_F32,V_CUBEMA_F32,V_BFE_U32,V_BFE_I32,
            case 530:  case 531:  case 533:  case 534:  case 535:   // V_BFI_B32,V_FMA_F32,V_LERP_U8,V_ALIGNBIT_B32,V_ALIGNBYTE_B32,
            case 536:  case 538:  case 539:  case 541:  case 542:   // V_MULLIT_F32,V_MIN3_I32,V_MIN3_U32,V_MAX3_I32,V_MAX3_U32,
            case 544:  case 545:  case 546:  case 547:  case 548:   // V_MED3_I32,V_MED3_U32,V_SAD_U8,V_SAD_HI_U8,V_SAD_U16,
            case 549:  case 550:  case 553:  case 554:   // V_SAD_U32,V_CVT_PK_U8_F32,V_MIN3_NUM_F32,V_MAX3_NUM_F32,
            case 555:  case 556:  case 557:  case 558:   // V_MIN3_NUM_F16,V_MAX3_NUM_F16,V_MINIMUM3_F32,V_MAXIMUM3_F32,
            case 559:  case 560:  case 561:  case 562:   // V_MINIMUM3_F16,V_MAXIMUM3_F16,V_MED3_NUM_F32,V_MED3_NUM_F16,
            case 563:  case 564:  case 565:  case 569:  case 576:   // V_BITOP3_B16,V_BITOP3_B32,V_MAD_U32,V_MSAD_U8,V_XOR3_B32,
            case 577:  case 580:  case 581:  case 582:  case 583:   // V_MAD_U16,V_PERM_B32,V_XAD_U32,V_LSHL_ADD_U32,V_ADD_LSHL_U32,
            case 584:  case 586:  case 587:  case 589:  case 590:   // V_FMA_F16,V_MIN3_I16,V_MIN3_U16,V_MAX3_I16,V_MAX3_U16,
            case 592:  case 593:  case 595:  case 596:  case 597:   // V_MED3_I16,V_MED3_U16,V_MAD_I16,V_DIV_FIXUP_F16,V_ADD3_U32,
            case 598:  case 599:  case 600:  case 601:   // V_LSHL_OR_B32,V_AND_OR_B32,V_OR3_B32,V_MAD_U32_U16,
            case 602:  case 606:  case 607:  case 608:   // V_MAD_I32_I16,V_ADD_MAX_I32,V_ADD_MAX_U32,V_ADD_MIN_I32,
            case 609:  case 610:  case 611:  case 612:   // V_ADD_MIN_U32,V_MAXMIN_U32,V_MINMAX_U32,V_MAXMIN_I32,
            case 613:  case 616:  case 617:  case 618:   // V_MINMAX_I32,V_MINMAX_NUM_F32,V_MAXMIN_NUM_F32,V_MINMAX_NUM_F16,
            case 619:  case 620:  case 621:   // V_MAXMIN_NUM_F16,V_MINIMUMMAXIMUM_F32,V_MAXIMUMMINIMUM_F32,
            case 622:  case 623:  case 656:   // V_MINIMUMMAXIMUM_F16,V_MAXIMUMMINIMUM_F16,V_ASHR_PK_I8_I32,
            case 657:  case 878:  case 880:   // V_ASHR_PK_U8_I32,V_CVT_SR_PK_BF16_F32,V_CVT_SR_PK_F16_F32,
                appendOPR_VGPR(layout.VDST,false,true);
                appendOPR_VGPR(layout.VSRC0,true,false);
                appendOPR_SRC(layout.SRC1,true,false);
                appendOPR_SRC(layout.SRC2,true,false);
                break;
            case 532:  case 552:   // V_FMA_F64,V_DIV_FIXUP_F64,
                appendOPR_VGPR(layout.VDST,false,true,2);
                appendOPR_VGPR(layout.VSRC0,true,false,2);
                appendOPR_SRC(layout.SRC1,true,false,2);
                appendOPR_SRC(layout.SRC2,true,false,2);
                break;
            case 568:   // V_DIV_FMAS_F64,
                appendOPR_VGPR(layout.VDST,false,true,2);
                appendOPR_VGPR(layout.VSRC0,true,false,2);
                appendOPR_SRC(layout.SRC1,true,false,2);
                appendOPR_SRC(layout.SRC2,true,false,2);
                appendOPR_VCC(106,true,false,1,true);
                break;
            case 594:   // V_LSHL_ADD_U64,
                appendOPR_VGPR(layout.VDST,false,true,2);
                appendOPR_VGPR(layout.VSRC0,true,false,2);
                appendOPR_SRC(layout.SRC1,true,false);
                appendOPR_SRC(layout.SRC2,true,false,2);
                break;
            case 762:  case 763:   // V_MAD_NC_U64_U32,V_MAD_NC_I64_I32,
                appendOPR_VGPR(layout.VDST,false,true,2);
                appendOPR_VGPR(layout.VSRC0,true,false);
                appendOPR_SRC(layout.SRC1,true,false);
                appendOPR_SRC(layout.SRC2,true,false,2);
                break;
            case 811:   // V_LDEXP_F64,
                appendOPR_VGPR(layout.VDST,false,true,2);
                appendOPR_VGPR(layout.VSRC0,true,false,2);
                appendOPR_SRC(layout.SRC1,true,false);
                break;
            case 1:  case 2:  case 3:  case 4:  case 5:   // V_CMP_LT_F16,V_CMP_EQ_F16,V_CMP_LE_F16,V_CMP_GT_F16,V_CMP_LG_F16,
            case 6:  case 7:  case 8:  case 9:  case 10:   // V_CMP_GE_F16,V_CMP_O_F16,V_CMP_U_F16,V_CMP_NGE_F16,V_CMP_NLG_F16,
            case 11:  case 12:  case 13:  case 14:   // V_CMP_NGT_F16,V_CMP_NLE_F16,V_CMP_NEQ_F16,V_CMP_NLT_F16,
            case 17:  case 18:  case 19:  case 20:  case 21:   // V_CMP_LT_F32,V_CMP_EQ_F32,V_CMP_LE_F32,V_CMP_GT_F32,V_CMP_LG_F32,
            case 22:  case 23:  case 24:  case 25:  case 26:   // V_CMP_GE_F32,V_CMP_O_F32,V_CMP_U_F32,V_CMP_NGE_F32,V_CMP_NLG_F32,
            case 27:  case 28:  case 29:  case 30:   // V_CMP_NGT_F32,V_CMP_NLE_F32,V_CMP_NEQ_F32,V_CMP_NLT_F32,
            case 49:  case 50:  case 51:  case 52:  case 53:   // V_CMP_LT_I16,V_CMP_EQ_I16,V_CMP_LE_I16,V_CMP_GT_I16,V_CMP_NE_I16,
            case 54:  case 57:  case 58:  case 59:  case 60:   // V_CMP_GE_I16,V_CMP_LT_U16,V_CMP_EQ_U16,V_CMP_LE_U16,V_CMP_GT_U16,
            case 61:  case 62:  case 65:  case 66:  case 67:   // V_CMP_NE_U16,V_CMP_GE_U16,V_CMP_LT_I32,V_CMP_EQ_I32,V_CMP_LE_I32,
            case 68:  case 69:  case 70:  case 73:  case 74:   // V_CMP_GT_I32,V_CMP_NE_I32,V_CMP_GE_I32,V_CMP_LT_U32,V_CMP_EQ_U32,
            case 75:  case 76:  case 77:  case 78:   // V_CMP_LE_U32,V_CMP_GT_U32,V_CMP_NE_U32,V_CMP_GE_U32,
            case 125:  case 126:   // V_CMP_CLASS_F16,V_CMP_CLASS_F32,
                appendOPR_SREG(layout.VDST,false,true);
                appendOPR_VGPR(layout.VSRC0,true,false);
                appendOPR_SRC(layout.SRC1,true,false);
                break;
            case 129:  case 130:  case 131:  case 132:   // V_CMPX_LT_F16,V_CMPX_EQ_F16,V_CMPX_LE_F16,V_CMPX_GT_F16,
            case 133:  case 134:  case 135:  case 136:   // V_CMPX_LG_F16,V_CMPX_GE_F16,V_CMPX_O_F16,V_CMPX_U_F16,
            case 137:  case 138:  case 139:  case 140:   // V_CMPX_NGE_F16,V_CMPX_NLG_F16,V_CMPX_NGT_F16,V_CMPX_NLE_F16,
            case 141:  case 142:  case 145:  case 146:   // V_CMPX_NEQ_F16,V_CMPX_NLT_F16,V_CMPX_LT_F32,V_CMPX_EQ_F32,
            case 147:  case 148:  case 149:  case 150:   // V_CMPX_LE_F32,V_CMPX_GT_F32,V_CMPX_LG_F32,V_CMPX_GE_F32,
            case 151:  case 152:  case 153:  case 154:   // V_CMPX_O_F32,V_CMPX_U_F32,V_CMPX_NGE_F32,V_CMPX_NLG_F32,
            case 155:  case 156:  case 157:  case 158:   // V_CMPX_NGT_F32,V_CMPX_NLE_F32,V_CMPX_NEQ_F32,V_CMPX_NLT_F32,
            case 177:  case 178:  case 179:  case 180:   // V_CMPX_LT_I16,V_CMPX_EQ_I16,V_CMPX_LE_I16,V_CMPX_GT_I16,
            case 181:  case 182:  case 185:  case 186:   // V_CMPX_NE_I16,V_CMPX_GE_I16,V_CMPX_LT_U16,V_CMPX_EQ_U16,
            case 187:  case 188:  case 189:  case 190:   // V_CMPX_LE_U16,V_CMPX_GT_U16,V_CMPX_NE_U16,V_CMPX_GE_U16,
            case 193:  case 194:  case 195:  case 196:   // V_CMPX_LT_I32,V_CMPX_EQ_I32,V_CMPX_LE_I32,V_CMPX_GT_I32,
            case 197:  case 198:  case 201:  case 202:   // V_CMPX_NE_I32,V_CMPX_GE_I32,V_CMPX_LT_U32,V_CMPX_EQ_U32,
            case 203:  case 204:  case 205:  case 206:   // V_CMPX_LE_U32,V_CMPX_GT_U32,V_CMPX_NE_U32,V_CMPX_GE_U32,
            case 253:  case 254:   // V_CMPX_CLASS_F16,V_CMPX_CLASS_F32,
                appendOPR_EXEC(layout.VDST,false,true);
                appendOPR_VGPR(layout.VSRC0,true,false);
                appendOPR_SRC(layout.SRC1,true,false);
                appendOPR_SDST_EXEC(126,false,true,1,true);
                break;
        }
    }

    void InstructionDecoder_amdgpu_gfx1250::finalizeENC_VOP2Operands()
    {
        layout_ENC_VOP2 & layout = insn_layout.ENC_VOP2;
        switch (layout.OP)  {
            case 1:   // V_CNDMASK_B32,
                appendOPR_VGPR(layout.VDST,false,true);
                appendOPR_SRC(layout.SRC0,true,false);
                appendOPR_VGPR(layout.VSRC1,true,false);
                appendOPR_VCC(106,true,false);
                break;
            case 2:  case 6:  case 13:  case 14:  case 23:   // V_ADD_F64,V_MUL_F64,V_MIN_NUM_F64,V_MAX_NUM_F64,V_FMAC_F64,
            case 40:  case 41:  case 42:   // V_ADD_NC_U64,V_SUB_NC_U64,V_MUL_U64,
                appendOPR_VGPR(layout.VDST,false,true,2);
                appendOPR_SRC(layout.SRC0,true,false,2);
                appendOPR_VGPR(layout.VSRC1,true,false,2);
                break;
            case 3:  case 4:  case 5:  case 7:  case 8:   // V_ADD_F32,V_SUB_F32,V_SUBREV_F32,V_MUL_DX9_ZERO_F32,V_MUL_F32,
            case 9:  case 10:  case 11:  case 12:   // V_MUL_I32_I24,V_MUL_HI_I32_I24,V_MUL_U32_U24,V_MUL_HI_U32_U24,
            case 17:  case 18:  case 19:  case 20:  case 21:   // V_MIN_I32,V_MAX_I32,V_MIN_U32,V_MAX_U32,V_MIN_NUM_F32,
            case 22:  case 24:  case 25:  case 26:  case 27:   // V_MAX_NUM_F32,V_LSHLREV_B32,V_LSHRREV_B32,V_ASHRREV_I32,V_AND_B32,
            case 28:  case 29:  case 30:  case 37:  case 38:   // V_OR_B32,V_XOR_B32,V_XNOR_B32,V_ADD_NC_U32,V_SUB_NC_U32,
            case 39:  case 43:  case 47:  case 48:   // V_SUBREV_NC_U32,V_FMAC_F32,V_CVT_PK_RTZ_F16_F32,V_MIN_NUM_F16,
            case 49:  case 50:  case 51:  case 52:  case 53:   // V_MAX_NUM_F16,V_ADD_F16,V_SUB_F16,V_SUBREV_F16,V_MUL_F16,
            case 54:  case 59:  case 60:   // V_FMAC_F16,V_LDEXP_F16,V_PK_FMAC_F16,
                appendOPR_VGPR(layout.VDST,false,true);
                appendOPR_SRC(layout.SRC0,true,false);
                appendOPR_VGPR(layout.VSRC1,true,false);
                break;
            case 31:   // V_LSHLREV_B64,
                appendOPR_VGPR(layout.VDST,false,true,2);
                appendOPR_SRC(layout.SRC0,true,false);
                appendOPR_VGPR(layout.VSRC1,true,false,2);
                break;
            case 32:  case 33:  case 34:   // V_ADD_CO_CI_U32,V_SUB_CO_CI_U32,V_SUBREV_CO_CI_U32,
                appendOPR_VGPR(layout.VDST,false,true);
                appendOPR_VCC(106,false,true);
                appendOPR_SRC(layout.SRC0,true,false);
                appendOPR_VGPR(layout.VSRC1,true,false);
                appendOPR_VCC(106,true,false);
                break;
        }
    }

    void InstructionDecoder_amdgpu_gfx1250::finalizeVOP2_INST_LITERALOperands()
    {
        layout_VOP2_INST_LITERAL & layout = insn_layout.VOP2_INST_LITERAL;
        switch (layout.OP)  {
            case 1:   // V_CNDMASK_B32,
                appendOPR_VGPR(layout.VDST,false,true);
                insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u32,layout.LITERAL)),true,false);
                appendOPR_VGPR(layout.VSRC1,true,false);
                appendOPR_VCC(106,true,false);
                break;
            case 2:  case 6:  case 13:  case 14:  case 23:   // V_ADD_F64,V_MUL_F64,V_MIN_NUM_F64,V_MAX_NUM_F64,V_FMAC_F64,
            case 31:  case 40:  case 41:  case 42:   // V_LSHLREV_B64,V_ADD_NC_U64,V_SUB_NC_U64,V_MUL_U64,
                appendOPR_VGPR(layout.VDST,false,true,2);
                insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u32,layout.LITERAL)),true,false);
                appendOPR_VGPR(layout.VSRC1,true,false,2);
                break;
            case 3:  case 4:  case 5:  case 7:  case 8:   // V_ADD_F32,V_SUB_F32,V_SUBREV_F32,V_MUL_DX9_ZERO_F32,V_MUL_F32,
            case 9:  case 10:  case 11:  case 12:   // V_MUL_I32_I24,V_MUL_HI_I32_I24,V_MUL_U32_U24,V_MUL_HI_U32_U24,
            case 17:  case 18:  case 19:  case 20:  case 21:   // V_MIN_I32,V_MAX_I32,V_MIN_U32,V_MAX_U32,V_MIN_NUM_F32,
            case 22:  case 24:  case 25:  case 26:  case 27:   // V_MAX_NUM_F32,V_LSHLREV_B32,V_LSHRREV_B32,V_ASHRREV_I32,V_AND_B32,
            case 28:  case 29:  case 30:  case 37:  case 38:   // V_OR_B32,V_XOR_B32,V_XNOR_B32,V_ADD_NC_U32,V_SUB_NC_U32,
            case 39:  case 43:  case 47:  case 48:   // V_SUBREV_NC_U32,V_FMAC_F32,V_CVT_PK_RTZ_F16_F32,V_MIN_NUM_F16,
            case 49:  case 50:  case 51:  case 52:  case 53:   // V_MAX_NUM_F16,V_ADD_F16,V_SUB_F16,V_SUBREV_F16,V_MUL_F16,
            case 54:  case 59:  case 60:   // V_FMAC_F16,V_LDEXP_F16,V_PK_FMAC_F16,
                appendOPR_VGPR(layout.VDST,false,true);
                insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u32,layout.LITERAL)),true,false);
                appendOPR_VGPR(layout.VSRC1,true,false);
                break;
            case 32:  case 33:  case 34:   // V_ADD_CO_CI_U32,V_SUB_CO_CI_U32,V_SUBREV_CO_CI_U32,
                appendOPR_VGPR(layout.VDST,false,true);
                appendOPR_VCC(106,false,true);
                insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u32,layout.LITERAL)),true,false);
                appendOPR_VGPR(layout.VSRC1,true,false);
                appendOPR_VCC(106,true,false);
                break;
            case 44:  case 55:   // V_FMAMK_F32,V_FMAMK_F16,
                if (COND_VOP2_INST_LITERAL_default(insn_long))  {
                    appendOPR_VGPR(layout.VDST,false,true);
                    appendOPR_SRC(layout.SRC0,true,false);
                    insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u32,layout.LITERAL)),true,false);
                    appendOPR_VGPR(layout.VSRC1,true,false);
                }
                else  {
                    appendOPR_VGPR(layout.VDST,false,true);
                    insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u32,layout.LITERAL)),true,false);
                    insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u32,layout.LITERAL)),true,false);
                    appendOPR_VGPR(layout.VSRC1,true,false);
                }
                break;
            case 45:  case 56:   // V_FMAAK_F32,V_FMAAK_F16,
                if (COND_VOP2_INST_LITERAL_default(insn_long))  {
                    appendOPR_VGPR(layout.VDST,false,true);
                    appendOPR_SRC(layout.SRC0,true,false);
                    appendOPR_VGPR(layout.VSRC1,true,false);
                    insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u32,layout.LITERAL)),true,false);
                }
                else  {
                    appendOPR_VGPR(layout.VDST,false,true);
                    insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u32,layout.LITERAL)),true,false);
                    appendOPR_VGPR(layout.VSRC1,true,false);
                    insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u32,layout.LITERAL)),true,false);
                }
                break;
        }
    }

    void InstructionDecoder_amdgpu_gfx1250::finalizeVOP2_INST_LITERAL64Operands()
    {
        layout_VOP2_INST_LITERAL64 & layout = insn_layout.VOP2_INST_LITERAL64;
        switch (layout.OP)  {
            case 1:   // V_CNDMASK_B32,
                appendOPR_VGPR(layout.VDST,false,true);
                insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u64,layout.LITERAL64)),true,false);
                appendOPR_VGPR(layout.VSRC1,true,false);
                appendOPR_VCC(106,true,false);
                break;
            case 2:  case 6:  case 13:  case 14:  case 23:   // V_ADD_F64,V_MUL_F64,V_MIN_NUM_F64,V_MAX_NUM_F64,V_FMAC_F64,
            case 31:  case 40:  case 41:  case 42:   // V_LSHLREV_B64,V_ADD_NC_U64,V_SUB_NC_U64,V_MUL_U64,
                appendOPR_VGPR(layout.VDST,false,true,2);
                insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u64,layout.LITERAL64)),true,false);
                appendOPR_VGPR(layout.VSRC1,true,false,2);
                break;
            case 3:  case 4:  case 5:  case 7:  case 8:   // V_ADD_F32,V_SUB_F32,V_SUBREV_F32,V_MUL_DX9_ZERO_F32,V_MUL_F32,
            case 9:  case 10:  case 11:  case 12:   // V_MUL_I32_I24,V_MUL_HI_I32_I24,V_MUL_U32_U24,V_MUL_HI_U32_U24,
            case 17:  case 18:  case 19:  case 20:  case 21:   // V_MIN_I32,V_MAX_I32,V_MIN_U32,V_MAX_U32,V_MIN_NUM_F32,
            case 22:  case 24:  case 25:  case 26:  case 27:   // V_MAX_NUM_F32,V_LSHLREV_B32,V_LSHRREV_B32,V_ASHRREV_I32,V_AND_B32,
            case 28:  case 29:  case 30:  case 37:  case 38:   // V_OR_B32,V_XOR_B32,V_XNOR_B32,V_ADD_NC_U32,V_SUB_NC_U32,
            case 39:  case 43:  case 47:  case 48:   // V_SUBREV_NC_U32,V_FMAC_F32,V_CVT_PK_RTZ_F16_F32,V_MIN_NUM_F16,
            case 49:  case 50:  case 51:  case 52:  case 53:   // V_MAX_NUM_F16,V_ADD_F16,V_SUB_F16,V_SUBREV_F16,V_MUL_F16,
            case 54:  case 59:  case 60:   // V_FMAC_F16,V_LDEXP_F16,V_PK_FMAC_F16,
                appendOPR_VGPR(layout.VDST,false,true);
                insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u64,layout.LITERAL64)),true,false);
                appendOPR_VGPR(layout.VSRC1,true,false);
                break;
            case 32:  case 33:  case 34:   // V_ADD_CO_CI_U32,V_SUB_CO_CI_U32,V_SUBREV_CO_CI_U32,
                appendOPR_VGPR(layout.VDST,false,true);
                appendOPR_VCC(106,false,true);
                insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u64,layout.LITERAL64)),true,false);
                appendOPR_VGPR(layout.VSRC1,true,false);
                appendOPR_VCC(106,true,false);
                break;
            case 35:   // V_FMAMK_F64,
                if (COND_VOP2_INST_LITERAL64_default(insn_long))  {
                    appendOPR_VGPR(layout.VDST,false,true,2);
                    appendOPR_SRC(layout.SRC0,true,false,2);
                    insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u64,layout.LITERAL64)),true,false);
                    appendOPR_VGPR(layout.VSRC1,true,false,2);
                }
                else  {
                    appendOPR_VGPR(layout.VDST,false,true,2);
                    insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u64,layout.LITERAL64)),true,false);
                    insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u64,layout.LITERAL64)),true,false);
                    appendOPR_VGPR(layout.VSRC1,true,false,2);
                }
                break;
            case 36:   // V_FMAAK_F64,
                if (COND_VOP2_INST_LITERAL64_default(insn_long))  {
                    appendOPR_VGPR(layout.VDST,false,true,2);
                    appendOPR_SRC(layout.SRC0,true,false,2);
                    appendOPR_VGPR(layout.VSRC1,true,false,2);
                    insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u64,layout.LITERAL64)),true,false);
                }
                else  {
                    appendOPR_VGPR(layout.VDST,false,true,2);
                    insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u64,layout.LITERAL64)),true,false);
                    appendOPR_VGPR(layout.VSRC1,true,false,2);
                    insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u64,layout.LITERAL64)),true,false);
                }
                break;
        }
    }

    void InstructionDecoder_amdgpu_gfx1250::finalizeVOP2_VOP_DPP8Operands()
    {
        layout_VOP2_VOP_DPP8 & layout = insn_layout.VOP2_VOP_DPP8;
        switch (layout.OP)  {
            case 1:   // V_CNDMASK_B32,
                appendOPR_VGPR(layout.VDST,false,true);
                appendOPR_VGPR(layout.VSRC0,true,false);
                appendOPR_VGPR(layout.VSRC1,true,false);
                appendOPR_VCC(106,true,false);
                break;
            case 3:  case 4:  case 5:  case 7:  case 8:   // V_ADD_F32,V_SUB_F32,V_SUBREV_F32,V_MUL_DX9_ZERO_F32,V_MUL_F32,
            case 9:  case 10:  case 11:  case 12:   // V_MUL_I32_I24,V_MUL_HI_I32_I24,V_MUL_U32_U24,V_MUL_HI_U32_U24,
            case 17:  case 18:  case 19:  case 20:  case 21:   // V_MIN_I32,V_MAX_I32,V_MIN_U32,V_MAX_U32,V_MIN_NUM_F32,
            case 22:  case 24:  case 25:  case 26:  case 27:   // V_MAX_NUM_F32,V_LSHLREV_B32,V_LSHRREV_B32,V_ASHRREV_I32,V_AND_B32,
            case 28:  case 29:  case 30:  case 37:  case 38:   // V_OR_B32,V_XOR_B32,V_XNOR_B32,V_ADD_NC_U32,V_SUB_NC_U32,
            case 39:  case 43:  case 47:  case 48:   // V_SUBREV_NC_U32,V_FMAC_F32,V_CVT_PK_RTZ_F16_F32,V_MIN_NUM_F16,
            case 49:  case 50:  case 51:  case 52:  case 53:   // V_MAX_NUM_F16,V_ADD_F16,V_SUB_F16,V_SUBREV_F16,V_MUL_F16,
            case 54:  case 59:   // V_FMAC_F16,V_LDEXP_F16,
                appendOPR_VGPR(layout.VDST,false,true);
                appendOPR_VGPR(layout.VSRC0,true,false);
                appendOPR_VGPR(layout.VSRC1,true,false);
                break;
            case 32:  case 33:  case 34:   // V_ADD_CO_CI_U32,V_SUB_CO_CI_U32,V_SUBREV_CO_CI_U32,
                appendOPR_VGPR(layout.VDST,false,true);
                appendOPR_VCC(106,false,true);
                appendOPR_VGPR(layout.VSRC0,true,false);
                appendOPR_VGPR(layout.VSRC1,true,false);
                appendOPR_VCC(106,true,false);
                break;
        }
    }

    void InstructionDecoder_amdgpu_gfx1250::finalizeVOP2_VOP_DPP16Operands()
    {
        layout_VOP2_VOP_DPP16 & layout = insn_layout.VOP2_VOP_DPP16;
        switch (layout.OP)  {
            case 1:   // V_CNDMASK_B32,
                appendOPR_VGPR(layout.VDST,false,true);
                appendOPR_VGPR(layout.VSRC0,true,false);
                appendOPR_VGPR(layout.VSRC1,true,false);
                appendOPR_VCC(106,true,false);
                break;
            case 2:  case 6:  case 13:  case 14:  case 23:   // V_ADD_F64,V_MUL_F64,V_MIN_NUM_F64,V_MAX_NUM_F64,V_FMAC_F64,
            case 40:  case 41:   // V_ADD_NC_U64,V_SUB_NC_U64,
                appendOPR_VGPR(layout.VDST,false,true,2);
                appendOPR_VGPR(layout.VSRC0,true,false,2);
                appendOPR_VGPR(layout.VSRC1,true,false,2);
                break;
            case 3:  case 4:  case 5:  case 7:  case 8:   // V_ADD_F32,V_SUB_F32,V_SUBREV_F32,V_MUL_DX9_ZERO_F32,V_MUL_F32,
            case 9:  case 10:  case 11:  case 12:   // V_MUL_I32_I24,V_MUL_HI_I32_I24,V_MUL_U32_U24,V_MUL_HI_U32_U24,
            case 17:  case 18:  case 19:  case 20:  case 21:   // V_MIN_I32,V_MAX_I32,V_MIN_U32,V_MAX_U32,V_MIN_NUM_F32,
            case 22:  case 24:  case 25:  case 26:  case 27:   // V_MAX_NUM_F32,V_LSHLREV_B32,V_LSHRREV_B32,V_ASHRREV_I32,V_AND_B32,
            case 28:  case 29:  case 30:  case 37:  case 38:   // V_OR_B32,V_XOR_B32,V_XNOR_B32,V_ADD_NC_U32,V_SUB_NC_U32,
            case 39:  case 43:  case 47:  case 48:   // V_SUBREV_NC_U32,V_FMAC_F32,V_CVT_PK_RTZ_F16_F32,V_MIN_NUM_F16,
            case 49:  case 50:  case 51:  case 52:  case 53:   // V_MAX_NUM_F16,V_ADD_F16,V_SUB_F16,V_SUBREV_F16,V_MUL_F16,
            case 54:  case 59:   // V_FMAC_F16,V_LDEXP_F16,
                appendOPR_VGPR(layout.VDST,false,true);
                appendOPR_VGPR(layout.VSRC0,true,false);
                appendOPR_VGPR(layout.VSRC1,true,false);
                break;
            case 31:   // V_LSHLREV_B64,
                appendOPR_VGPR(layout.VDST,false,true,2);
                appendOPR_VGPR(layout.VSRC0,true,false);
                appendOPR_VGPR(layout.VSRC1,true,false,2);
                break;
            case 32:  case 33:  case 34:   // V_ADD_CO_CI_U32,V_SUB_CO_CI_U32,V_SUBREV_CO_CI_U32,
                appendOPR_VGPR(layout.VDST,false,true);
                appendOPR_VCC(106,false,true);
                appendOPR_VGPR(layout.VSRC0,true,false);
                appendOPR_VGPR(layout.VSRC1,true,false);
                appendOPR_VCC(106,true,false);
                break;
        }
    }

    void InstructionDecoder_amdgpu_gfx1250::finalizeVOP3_SDST_ENCOperands()
    {
        layout_VOP3_SDST_ENC & layout = insn_layout.VOP3_SDST_ENC;
        switch (layout.OP)  {
            case 288:  case 289:  case 290:   // V_ADD_CO_CI_U32,V_SUB_CO_CI_U32,V_SUBREV_CO_CI_U32,
                appendOPR_VGPR(layout.VDST,false,true);
                appendOPR_SREG(layout.SDST,false,true);
                appendOPR_SRC(layout.SRC0,true,false);
                appendOPR_SRC(layout.SRC1,true,false);
                appendOPR_SREG(layout.SRC2,true,false);
                break;
            case 764:   // V_DIV_SCALE_F32,
                appendOPR_VGPR(layout.VDST,false,true);
                appendOPR_SREG(layout.SDST,false,true);
                appendOPR_SRC(layout.SRC0,true,false);
                appendOPR_SRC(layout.SRC1,true,false);
                appendOPR_SRC(layout.SRC2,true,false);
                break;
            case 765:   // V_DIV_SCALE_F64,
                appendOPR_VGPR(layout.VDST,false,true,2);
                appendOPR_SREG(layout.SDST,false,true);
                appendOPR_SRC(layout.SRC0,true,false,2);
                appendOPR_SRC(layout.SRC1,true,false,2);
                appendOPR_SRC(layout.SRC2,true,false,2);
                break;
            case 766:  case 767:   // V_MAD_CO_U64_U32,V_MAD_CO_I64_I32,
                appendOPR_VGPR(layout.VDST,false,true,2);
                appendOPR_SREG(layout.SDST,false,true);
                appendOPR_SRC(layout.SRC0,true,false);
                appendOPR_SRC(layout.SRC1,true,false);
                appendOPR_SRC(layout.SRC2,true,false,2);
                break;
            case 768:  case 769:  case 770:   // V_ADD_CO_U32,V_SUB_CO_U32,V_SUBREV_CO_U32,
                appendOPR_VGPR(layout.VDST,false,true);
                appendOPR_SREG(layout.SDST,false,true);
                appendOPR_SRC(layout.SRC0,true,false);
                appendOPR_SRC(layout.SRC1,true,false);
                break;
        }
    }

    void InstructionDecoder_amdgpu_gfx1250::finalizeVOP3_SDST_ENC_INST_LITERALOperands()
    {
        layout_VOP3_SDST_ENC_INST_LITERAL & layout = insn_layout.VOP3_SDST_ENC_INST_LITERAL;
        switch (layout.OP)  {
            case 288:  case 289:  case 290:   // V_ADD_CO_CI_U32,V_SUB_CO_CI_U32,V_SUBREV_CO_CI_U32,
                if (COND_VOP3_SDST_ENC_INST_LITERAL_has_lit_0(insn_long))  {
                    appendOPR_VGPR(layout.VDST,false,true);
                    appendOPR_SREG(layout.SDST,false,true);
                    insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u32,layout.LITERAL)),true,false);
                    appendOPR_SRC(layout.SRC1,true,false);
                    appendOPR_SREG(layout.SRC2,true,false);
                }
                else if (COND_VOP3_SDST_ENC_INST_LITERAL_has_lit_1(insn_long))  {
                    appendOPR_VGPR(layout.VDST,false,true);
                    appendOPR_SREG(layout.SDST,false,true);
                    appendOPR_SRC(layout.SRC0,true,false);
                    insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u32,layout.LITERAL)),true,false);
                    appendOPR_SREG(layout.SRC2,true,false);
                }
                else  {
                    appendOPR_VGPR(layout.VDST,false,true);
                    appendOPR_SREG(layout.SDST,false,true);
                    insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u32,layout.LITERAL)),true,false);
                    insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u32,layout.LITERAL)),true,false);
                    appendOPR_SREG(layout.SRC2,true,false);
                }
                break;
            case 764:   // V_DIV_SCALE_F32,
                if (COND_VOP3_SDST_ENC_INST_LITERAL_has_lit_0(insn_long))  {
                    appendOPR_VGPR(layout.VDST,false,true);
                    appendOPR_SREG(layout.SDST,false,true);
                    insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u32,layout.LITERAL)),true,false);
                    appendOPR_SRC(layout.SRC1,true,false);
                    appendOPR_SRC(layout.SRC2,true,false);
                }
                else if (COND_VOP3_SDST_ENC_INST_LITERAL_has_lit_1(insn_long))  {
                    appendOPR_VGPR(layout.VDST,false,true);
                    appendOPR_SREG(layout.SDST,false,true);
                    appendOPR_SRC(layout.SRC0,true,false);
                    insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u32,layout.LITERAL)),true,false);
                    appendOPR_SRC(layout.SRC2,true,false);
                }
                else if (COND_VOP3_SDST_ENC_INST_LITERAL_has_lit_0_and_has_lit_1(insn_long))  {
                    appendOPR_VGPR(layout.VDST,false,true);
                    appendOPR_SREG(layout.SDST,false,true);
                    insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u32,layout.LITERAL)),true,false);
                    insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u32,layout.LITERAL)),true,false);
                    appendOPR_SRC(layout.SRC2,true,false);
                }
                else if (COND_VOP3_SDST_ENC_INST_LITERAL_has_lit_2(insn_long))  {
                    appendOPR_VGPR(layout.VDST,false,true);
                    appendOPR_SREG(layout.SDST,false,true);
                    appendOPR_SRC(layout.SRC0,true,false);
                    appendOPR_SRC(layout.SRC1,true,false);
                    insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u32,layout.LITERAL)),true,false);
                }
                else if (COND_VOP3_SDST_ENC_INST_LITERAL_has_lit_0_and_has_lit_2(insn_long))  {
                    appendOPR_VGPR(layout.VDST,false,true);
                    appendOPR_SREG(layout.SDST,false,true);
                    insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u32,layout.LITERAL)),true,false);
                    appendOPR_SRC(layout.SRC1,true,false);
                    insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u32,layout.LITERAL)),true,false);
                }
                else if (COND_VOP3_SDST_ENC_INST_LITERAL_has_lit_1_and_has_lit_2(insn_long))  {
                    appendOPR_VGPR(layout.VDST,false,true);
                    appendOPR_SREG(layout.SDST,false,true);
                    appendOPR_SRC(layout.SRC0,true,false);
                    insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u32,layout.LITERAL)),true,false);
                    insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u32,layout.LITERAL)),true,false);
                }
                else  {
                    appendOPR_VGPR(layout.VDST,false,true);
                    appendOPR_SREG(layout.SDST,false,true);
                    insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u32,layout.LITERAL)),true,false);
                    insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u32,layout.LITERAL)),true,false);
                    insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u32,layout.LITERAL)),true,false);
                }
                break;
            case 765:   // V_DIV_SCALE_F64,
                if (COND_VOP3_SDST_ENC_INST_LITERAL_has_lit_0(insn_long))  {
                    appendOPR_VGPR(layout.VDST,false,true,2);
                    appendOPR_SREG(layout.SDST,false,true);
                    insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u32,layout.LITERAL)),true,false);
                    appendOPR_SRC(layout.SRC1,true,false,2);
                    appendOPR_SRC(layout.SRC2,true,false,2);
                }
                else if (COND_VOP3_SDST_ENC_INST_LITERAL_has_lit_1(insn_long))  {
                    appendOPR_VGPR(layout.VDST,false,true,2);
                    appendOPR_SREG(layout.SDST,false,true);
                    appendOPR_SRC(layout.SRC0,true,false,2);
                    insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u32,layout.LITERAL)),true,false);
                    appendOPR_SRC(layout.SRC2,true,false,2);
                }
                else if (COND_VOP3_SDST_ENC_INST_LITERAL_has_lit_0_and_has_lit_1(insn_long))  {
                    appendOPR_VGPR(layout.VDST,false,true,2);
                    appendOPR_SREG(layout.SDST,false,true);
                    insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u32,layout.LITERAL)),true,false);
                    insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u32,layout.LITERAL)),true,false);
                    appendOPR_SRC(layout.SRC2,true,false,2);
                }
                else if (COND_VOP3_SDST_ENC_INST_LITERAL_has_lit_2(insn_long))  {
                    appendOPR_VGPR(layout.VDST,false,true,2);
                    appendOPR_SREG(layout.SDST,false,true);
                    appendOPR_SRC(layout.SRC0,true,false,2);
                    appendOPR_SRC(layout.SRC1,true,false,2);
                    insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u32,layout.LITERAL)),true,false);
                }
                else if (COND_VOP3_SDST_ENC_INST_LITERAL_has_lit_0_and_has_lit_2(insn_long))  {
                    appendOPR_VGPR(layout.VDST,false,true,2);
                    appendOPR_SREG(layout.SDST,false,true);
                    insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u32,layout.LITERAL)),true,false);
                    appendOPR_SRC(layout.SRC1,true,false,2);
                    insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u32,layout.LITERAL)),true,false);
                }
                else if (COND_VOP3_SDST_ENC_INST_LITERAL_has_lit_1_and_has_lit_2(insn_long))  {
                    appendOPR_VGPR(layout.VDST,false,true,2);
                    appendOPR_SREG(layout.SDST,false,true);
                    appendOPR_SRC(layout.SRC0,true,false,2);
                    insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u32,layout.LITERAL)),true,false);
                    insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u32,layout.LITERAL)),true,false);
                }
                else  {
                    appendOPR_VGPR(layout.VDST,false,true,2);
                    appendOPR_SREG(layout.SDST,false,true);
                    insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u32,layout.LITERAL)),true,false);
                    insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u32,layout.LITERAL)),true,false);
                    insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u32,layout.LITERAL)),true,false);
                }
                break;
            case 766:  case 767:   // V_MAD_CO_U64_U32,V_MAD_CO_I64_I32,
                if (COND_VOP3_SDST_ENC_INST_LITERAL_has_lit_0(insn_long))  {
                    appendOPR_VGPR(layout.VDST,false,true,2);
                    appendOPR_SREG(layout.SDST,false,true);
                    insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u32,layout.LITERAL)),true,false);
                    appendOPR_SRC(layout.SRC1,true,false);
                    appendOPR_SRC(layout.SRC2,true,false,2);
                }
                else if (COND_VOP3_SDST_ENC_INST_LITERAL_has_lit_1(insn_long))  {
                    appendOPR_VGPR(layout.VDST,false,true,2);
                    appendOPR_SREG(layout.SDST,false,true);
                    appendOPR_SRC(layout.SRC0,true,false);
                    insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u32,layout.LITERAL)),true,false);
                    appendOPR_SRC(layout.SRC2,true,false,2);
                }
                else if (COND_VOP3_SDST_ENC_INST_LITERAL_has_lit_0_and_has_lit_1(insn_long))  {
                    appendOPR_VGPR(layout.VDST,false,true,2);
                    appendOPR_SREG(layout.SDST,false,true);
                    insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u32,layout.LITERAL)),true,false);
                    insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u32,layout.LITERAL)),true,false);
                    appendOPR_SRC(layout.SRC2,true,false,2);
                }
                else if (COND_VOP3_SDST_ENC_INST_LITERAL_has_lit_2(insn_long))  {
                    appendOPR_VGPR(layout.VDST,false,true,2);
                    appendOPR_SREG(layout.SDST,false,true);
                    appendOPR_SRC(layout.SRC0,true,false);
                    appendOPR_SRC(layout.SRC1,true,false);
                    insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u32,layout.LITERAL)),true,false);
                }
                else if (COND_VOP3_SDST_ENC_INST_LITERAL_has_lit_0_and_has_lit_2(insn_long))  {
                    appendOPR_VGPR(layout.VDST,false,true,2);
                    appendOPR_SREG(layout.SDST,false,true);
                    insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u32,layout.LITERAL)),true,false);
                    appendOPR_SRC(layout.SRC1,true,false);
                    insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u32,layout.LITERAL)),true,false);
                }
                else if (COND_VOP3_SDST_ENC_INST_LITERAL_has_lit_1_and_has_lit_2(insn_long))  {
                    appendOPR_VGPR(layout.VDST,false,true,2);
                    appendOPR_SREG(layout.SDST,false,true);
                    appendOPR_SRC(layout.SRC0,true,false);
                    insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u32,layout.LITERAL)),true,false);
                    insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u32,layout.LITERAL)),true,false);
                }
                else  {
                    appendOPR_VGPR(layout.VDST,false,true,2);
                    appendOPR_SREG(layout.SDST,false,true);
                    insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u32,layout.LITERAL)),true,false);
                    insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u32,layout.LITERAL)),true,false);
                    insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u32,layout.LITERAL)),true,false);
                }
                break;
            case 768:  case 769:  case 770:   // V_ADD_CO_U32,V_SUB_CO_U32,V_SUBREV_CO_U32,
                if (COND_VOP3_SDST_ENC_INST_LITERAL_has_lit_0(insn_long))  {
                    appendOPR_VGPR(layout.VDST,false,true);
                    appendOPR_SREG(layout.SDST,false,true);
                    insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u32,layout.LITERAL)),true,false);
                    appendOPR_SRC(layout.SRC1,true,false);
                }
                else if (COND_VOP3_SDST_ENC_INST_LITERAL_has_lit_1(insn_long))  {
                    appendOPR_VGPR(layout.VDST,false,true);
                    appendOPR_SREG(layout.SDST,false,true);
                    appendOPR_SRC(layout.SRC0,true,false);
                    insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u32,layout.LITERAL)),true,false);
                }
                else  {
                    appendOPR_VGPR(layout.VDST,false,true);
                    appendOPR_SREG(layout.SDST,false,true);
                    insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u32,layout.LITERAL)),true,false);
                    insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u32,layout.LITERAL)),true,false);
                }
                break;
        }
    }

    void InstructionDecoder_amdgpu_gfx1250::finalizeVOP3_SDST_ENC_VOP_DPP8Operands()
    {
        layout_VOP3_SDST_ENC_VOP_DPP8 & layout = insn_layout.VOP3_SDST_ENC_VOP_DPP8;
        switch (layout.OP)  {
            case 288:  case 289:   // V_ADD_CO_CI_U32,V_SUB_CO_CI_U32,
                appendOPR_VGPR(layout.VDST,false,true);
                appendOPR_SREG(layout.SDST,false,true);
                appendOPR_VGPR(layout.VSRC0,true,false);
                appendOPR_SRC(layout.SRC1,true,false);
                appendOPR_SREG(layout.SRC2,true,false);
                break;
            case 290:   // V_SUBREV_CO_CI_U32,
                appendOPR_VGPR(layout.VDST,false,true);
                appendOPR_SREG(layout.SDST,false,true);
                appendOPR_VGPR(layout.VSRC0,true,false);
                appendOPR_SRC_VGPR(layout.SRC1,true,false);
                appendOPR_SREG(layout.SRC2,true,false);
                break;
            case 768:  case 769:   // V_ADD_CO_U32,V_SUB_CO_U32,
                appendOPR_VGPR(layout.VDST,false,true);
                appendOPR_SREG(layout.SDST,false,true);
                appendOPR_VGPR(layout.VSRC0,true,false);
                appendOPR_SRC(layout.SRC1,true,false);
                break;
            case 770:   // V_SUBREV_CO_U32,
                appendOPR_VGPR(layout.VDST,false,true);
                appendOPR_SREG(layout.SDST,false,true);
                appendOPR_VGPR(layout.VSRC0,true,false);
                appendOPR_SRC_VGPR(layout.SRC1,true,false);
                break;
        }
    }

    void InstructionDecoder_amdgpu_gfx1250::finalizeVOP3_SDST_ENC_VOP_DPP16Operands()
    {
        layout_VOP3_SDST_ENC_VOP_DPP16 & layout = insn_layout.VOP3_SDST_ENC_VOP_DPP16;
        switch (layout.OP)  {
            case 288:  case 289:   // V_ADD_CO_CI_U32,V_SUB_CO_CI_U32,
                appendOPR_VGPR(layout.VDST,false,true);
                appendOPR_SREG(layout.SDST,false,true);
                appendOPR_VGPR(layout.VSRC0,true,false);
                appendOPR_SRC(layout.SRC1,true,false);
                appendOPR_SREG(layout.SRC2,true,false);
                break;
            case 290:   // V_SUBREV_CO_CI_U32,
                appendOPR_VGPR(layout.VDST,false,true);
                appendOPR_SREG(layout.SDST,false,true);
                appendOPR_VGPR(layout.VSRC0,true,false);
                appendOPR_SRC_VGPR(layout.SRC1,true,false);
                appendOPR_SREG(layout.SRC2,true,false);
                break;
            case 765:   // V_DIV_SCALE_F64,
                appendOPR_VGPR(layout.VDST,false,true,2);
                appendOPR_SREG(layout.SDST,false,true);
                appendOPR_VGPR(layout.VSRC0,true,false,2);
                appendOPR_SRC(layout.SRC1,true,false,2);
                appendOPR_SRC(layout.SRC2,true,false,2);
                break;
            case 766:  case 767:   // V_MAD_CO_U64_U32,V_MAD_CO_I64_I32,
                appendOPR_VGPR(layout.VDST,false,true,2);
                appendOPR_SREG(layout.SDST,false,true);
                appendOPR_VGPR(layout.VSRC0,true,false);
                appendOPR_SRC(layout.SRC1,true,false);
                appendOPR_SRC(layout.SRC2,true,false,2);
                break;
            case 768:  case 769:   // V_ADD_CO_U32,V_SUB_CO_U32,
                appendOPR_VGPR(layout.VDST,false,true);
                appendOPR_SREG(layout.SDST,false,true);
                appendOPR_VGPR(layout.VSRC0,true,false);
                appendOPR_SRC(layout.SRC1,true,false);
                break;
            case 770:   // V_SUBREV_CO_U32,
                appendOPR_VGPR(layout.VDST,false,true);
                appendOPR_SREG(layout.SDST,false,true);
                appendOPR_VGPR(layout.VSRC0,true,false);
                appendOPR_SRC_VGPR(layout.SRC1,true,false);
                break;
        }
    }

    void InstructionDecoder_amdgpu_gfx1250::finalizeENC_VOP3POperands()
    {
        layout_ENC_VOP3P & layout = insn_layout.ENC_VOP3P;
        switch (layout.OP)  {
            case 0:  case 9:  case 14:  case 17:  case 20:   // V_PK_MAD_I16,V_PK_MAD_U16,V_PK_FMA_F16,V_PK_FMA_BF16,V_PK_ADD_MAX_I16,
            case 21:  case 22:  case 23:  case 24:   // V_PK_ADD_MAX_U16,V_DOT4_I32_IU8,V_DOT4_U32_U8,V_DOT8_I32_IU4,
            case 25:  case 32:  case 33:  case 34:   // V_DOT8_U32_U4,V_FMA_MIX_F32,V_FMA_MIXLO_F16,V_FMA_MIXHI_F16,
            case 45:  case 46:  case 47:  case 48:   // V_PK_ADD_MIN_I16,V_PK_ADD_MIN_U16,V_PK_MAX3_I16,V_PK_MAX3_U16,
            case 49:  case 50:  case 54:  case 55:   // V_PK_MIN3_I16,V_PK_MIN3_U16,V_PK_MINIMUM3_F16,V_PK_MAXIMUM3_F16,
            case 56:  case 57:  case 61:  case 62:   // V_PK_MIN3_NUM_F16,V_PK_MAX3_NUM_F16,V_FMA_MIX_F32_BF16,V_FMA_MIXLO_BF16,
            case 63:   // V_FMA_MIXHI_BF16,
                appendOPR_VGPR(layout.VDST,false,true);
                appendOPR_SRC(layout.SRC0,true,false);
                appendOPR_SRC(layout.SRC1,true,false);
                appendOPR_SRC(layout.SRC2,true,false);
                break;
            case 1:  case 2:  case 3:  case 4:   // V_PK_MUL_LO_U16,V_PK_ADD_I16,V_PK_SUB_I16,V_PK_LSHLREV_B16,
            case 5:  case 6:  case 7:  case 8:   // V_PK_LSHRREV_B16,V_PK_ASHRREV_I16,V_PK_MAX_I16,V_PK_MIN_I16,
            case 10:  case 11:  case 12:  case 13:  case 15:   // V_PK_ADD_U16,V_PK_SUB_U16,V_PK_MAX_U16,V_PK_MIN_U16,V_PK_ADD_F16,
            case 16:  case 27:  case 28:  case 29:   // V_PK_MUL_F16,V_PK_MIN_NUM_F16,V_PK_MAX_NUM_F16,V_PK_MINIMUM_F16,
            case 30:  case 35:  case 42:  case 43:   // V_PK_MAXIMUM_F16,V_PK_ADD_BF16,V_PK_MUL_BF16,V_PK_MIN_NUM_BF16,
            case 44:   // V_PK_MAX_NUM_BF16,
                appendOPR_VGPR(layout.VDST,false,true);
                appendOPR_SRC(layout.SRC0,true,false);
                appendOPR_SRC(layout.SRC1,true,false);
                break;
            case 31:   // V_PK_FMA_F32,
                appendOPR_VGPR(layout.VDST,false,true,2);
                appendOPR_SRC(layout.SRC0,true,false,2);
                appendOPR_SRC(layout.SRC1,true,false,2);
                appendOPR_SRC(layout.SRC2,true,false,2);
                break;
            case 40:  case 41:   // V_PK_MUL_F32,V_PK_ADD_F32,
                appendOPR_VGPR(layout.VDST,false,true,2);
                appendOPR_SRC(layout.SRC0,true,false,2);
                appendOPR_SRC(layout.SRC1,true,false,2);
                break;
            case 51:  case 128:  case 129:   // V_WMMA_F32_16X16X128_F8F6F4,V_WMMA_F32_16X16X128_FP8_FP8,V_WMMA_F32_16X16X128_FP8_BF8,
            case 130:  case 131:   // V_WMMA_F32_16X16X128_BF8_FP8,V_WMMA_F32_16X16X128_BF8_BF8,
                appendOPR_VGPR(layout.VDST,false,true,8);
                appendOPR_SRC_VGPR(layout.SRC0,true,false,16);
                appendOPR_SRC_VGPR(layout.SRC1,true,false,16);
                appendOPR_SRC_VGPR_OR_INLINE(layout.SRC2,true,false,8);
                break;
            case 93:   // V_WMMA_F32_16X16X4_F32,
                appendOPR_VGPR(layout.VDST,false,true,8);
                appendOPR_SRC_VGPR(layout.SRC0,true,false,2);
                appendOPR_SRC_VGPR(layout.SRC1,true,false,2);
                appendOPR_SRC_VGPR_OR_INLINE(layout.SRC2,true,false,8);
                break;
            case 96:  case 98:  case 106:   // V_WMMA_F32_16X16X32_F16,V_WMMA_F32_16X16X32_BF16,V_WMMA_F32_16X16X64_FP8_FP8,
            case 107:  case 108:  case 109:   // V_WMMA_F32_16X16X64_FP8_BF8,V_WMMA_F32_16X16X64_BF8_FP8,V_WMMA_F32_16X16X64_BF8_BF8,
            case 114:   // V_WMMA_I32_16X16X64_IU8,
                appendOPR_VGPR(layout.VDST,false,true,8);
                appendOPR_SRC_VGPR(layout.SRC0,true,false,8);
                appendOPR_SRC_VGPR(layout.SRC1,true,false,8);
                appendOPR_SRC_VGPR_OR_INLINE(layout.SRC2,true,false,8);
                break;
            case 97:  case 99:  case 110:   // V_WMMA_F16_16X16X32_F16,V_WMMA_BF16_16X16X32_BF16,V_WMMA_F16_16X16X64_FP8_FP8,
            case 111:  case 112:  case 113:   // V_WMMA_F16_16X16X64_FP8_BF8,V_WMMA_F16_16X16X64_BF8_FP8,V_WMMA_F16_16X16X64_BF8_BF8,
                appendOPR_VGPR(layout.VDST,false,true,4);
                appendOPR_SRC_VGPR(layout.SRC0,true,false,8);
                appendOPR_SRC_VGPR(layout.SRC1,true,false,8);
                appendOPR_SRC_VGPR_OR_INLINE(layout.SRC2,true,false,4);
                break;
            case 100:   // V_WMMA_BF16F32_16X16X32_BF16,
                appendOPR_VGPR(layout.VDST,false,true,4);
                appendOPR_SRC_VGPR(layout.SRC0,true,false,8);
                appendOPR_SRC_VGPR(layout.SRC1,true,false,8);
                appendOPR_SRC_VGPR_OR_INLINE(layout.SRC2,true,false,8);
                break;
            case 101:  case 102:  case 105:   // V_SWMMAC_F32_16X16X64_F16,V_SWMMAC_F32_16X16X64_BF16,V_SWMMAC_BF16F32_16X16X64_BF16,
                appendOPR_VGPR(layout.VDST,false,true,8);
                appendOPR_SRC_VGPR(layout.SRC0,true,false,8);
                appendOPR_SRC_VGPR(layout.SRC1,true,false,16);
                appendOPR_SRC_VGPR(layout.SRC2,true,false);
                break;
            case 103:  case 104:   // V_SWMMAC_F16_16X16X64_F16,V_SWMMAC_BF16_16X16X64_BF16,
                appendOPR_VGPR(layout.VDST,false,true,4);
                appendOPR_SRC_VGPR(layout.SRC0,true,false,8);
                appendOPR_SRC_VGPR(layout.SRC1,true,false,16);
                appendOPR_SRC_VGPR(layout.SRC2,true,false);
                break;
            case 115:  case 116:   // V_SWMMAC_F32_16X16X128_FP8_FP8,V_SWMMAC_F32_16X16X128_FP8_BF8,
            case 117:  case 118:   // V_SWMMAC_F32_16X16X128_BF8_FP8,V_SWMMAC_F32_16X16X128_BF8_BF8,
            case 123:   // V_SWMMAC_I32_16X16X128_IU8,
                appendOPR_VGPR(layout.VDST,false,true,8);
                appendOPR_SRC_VGPR(layout.SRC0,true,false,8);
                appendOPR_SRC_VGPR(layout.SRC1,true,false,16);
                appendOPR_SRC_VGPR(layout.SRC2,true,false,2);
                break;
            case 119:  case 120:   // V_SWMMAC_F16_16X16X128_FP8_FP8,V_SWMMAC_F16_16X16X128_FP8_BF8,
            case 121:  case 122:   // V_SWMMAC_F16_16X16X128_BF8_FP8,V_SWMMAC_F16_16X16X128_BF8_BF8,
                appendOPR_VGPR(layout.VDST,false,true,4);
                appendOPR_SRC_VGPR(layout.SRC0,true,false,8);
                appendOPR_SRC_VGPR(layout.SRC1,true,false,16);
                appendOPR_SRC_VGPR(layout.SRC2,true,false,2);
                break;
            case 132:  case 133:   // V_WMMA_F16_16X16X128_FP8_FP8,V_WMMA_F16_16X16X128_FP8_BF8,
            case 134:  case 135:   // V_WMMA_F16_16X16X128_BF8_FP8,V_WMMA_F16_16X16X128_BF8_BF8,
                appendOPR_VGPR(layout.VDST,false,true,4);
                appendOPR_SRC_VGPR(layout.SRC0,true,false,16);
                appendOPR_SRC_VGPR(layout.SRC1,true,false,16);
                appendOPR_SRC_VGPR_OR_INLINE(layout.SRC2,true,false,4);
                break;
            case 136:   // V_WMMA_F32_32X16X128_F4,
                appendOPR_VGPR(layout.VDST,false,true,16);
                appendOPR_SRC_VGPR(layout.SRC0,true,false,16);
                appendOPR_SRC_VGPR(layout.SRC1,true,false,8);
                appendOPR_SRC_VGPR_OR_INLINE(layout.SRC2,true,false,16);
                break;
        }
    }

    void InstructionDecoder_amdgpu_gfx1250::finalizeVOP3P_INST_LITERALOperands()
    {
        layout_VOP3P_INST_LITERAL & layout = insn_layout.VOP3P_INST_LITERAL;
        switch (layout.OP)  {
            case 0:  case 9:  case 14:  case 17:  case 20:   // V_PK_MAD_I16,V_PK_MAD_U16,V_PK_FMA_F16,V_PK_FMA_BF16,V_PK_ADD_MAX_I16,
            case 21:  case 22:  case 23:  case 24:   // V_PK_ADD_MAX_U16,V_DOT4_I32_IU8,V_DOT4_U32_U8,V_DOT8_I32_IU4,
            case 25:  case 32:  case 33:  case 34:   // V_DOT8_U32_U4,V_FMA_MIX_F32,V_FMA_MIXLO_F16,V_FMA_MIXHI_F16,
            case 45:  case 46:  case 47:  case 48:   // V_PK_ADD_MIN_I16,V_PK_ADD_MIN_U16,V_PK_MAX3_I16,V_PK_MAX3_U16,
            case 49:  case 50:  case 54:  case 55:   // V_PK_MIN3_I16,V_PK_MIN3_U16,V_PK_MINIMUM3_F16,V_PK_MAXIMUM3_F16,
            case 56:  case 57:  case 61:  case 62:   // V_PK_MIN3_NUM_F16,V_PK_MAX3_NUM_F16,V_FMA_MIX_F32_BF16,V_FMA_MIXLO_BF16,
            case 63:   // V_FMA_MIXHI_BF16,
                if (COND_VOP3P_INST_LITERAL_has_lit_0(insn_long))  {
                    appendOPR_VGPR(layout.VDST,false,true);
                    insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u32,layout.LITERAL)),true,false);
                    appendOPR_SRC(layout.SRC1,true,false);
                    appendOPR_SRC(layout.SRC2,true,false);
                }
                else if (COND_VOP3P_INST_LITERAL_has_lit_1(insn_long))  {
                    appendOPR_VGPR(layout.VDST,false,true);
                    appendOPR_SRC(layout.SRC0,true,false);
                    insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u32,layout.LITERAL)),true,false);
                    appendOPR_SRC(layout.SRC2,true,false);
                }
                else if (COND_VOP3P_INST_LITERAL_has_lit_0_and_has_lit_1(insn_long))  {
                    appendOPR_VGPR(layout.VDST,false,true);
                    insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u32,layout.LITERAL)),true,false);
                    insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u32,layout.LITERAL)),true,false);
                    appendOPR_SRC(layout.SRC2,true,false);
                }
                else if (COND_VOP3P_INST_LITERAL_has_lit_2(insn_long))  {
                    appendOPR_VGPR(layout.VDST,false,true);
                    appendOPR_SRC(layout.SRC0,true,false);
                    appendOPR_SRC(layout.SRC1,true,false);
                    insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u32,layout.LITERAL)),true,false);
                }
                else if (COND_VOP3P_INST_LITERAL_has_lit_0_and_has_lit_2(insn_long))  {
                    appendOPR_VGPR(layout.VDST,false,true);
                    insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u32,layout.LITERAL)),true,false);
                    appendOPR_SRC(layout.SRC1,true,false);
                    insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u32,layout.LITERAL)),true,false);
                }
                else if (COND_VOP3P_INST_LITERAL_has_lit_1_and_has_lit_2(insn_long))  {
                    appendOPR_VGPR(layout.VDST,false,true);
                    appendOPR_SRC(layout.SRC0,true,false);
                    insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u32,layout.LITERAL)),true,false);
                    insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u32,layout.LITERAL)),true,false);
                }
                else  {
                    appendOPR_VGPR(layout.VDST,false,true);
                    insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u32,layout.LITERAL)),true,false);
                    insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u32,layout.LITERAL)),true,false);
                    insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u32,layout.LITERAL)),true,false);
                }
                break;
            case 1:  case 2:  case 3:  case 4:   // V_PK_MUL_LO_U16,V_PK_ADD_I16,V_PK_SUB_I16,V_PK_LSHLREV_B16,
            case 5:  case 6:  case 7:  case 8:   // V_PK_LSHRREV_B16,V_PK_ASHRREV_I16,V_PK_MAX_I16,V_PK_MIN_I16,
            case 10:  case 11:  case 12:  case 13:  case 15:   // V_PK_ADD_U16,V_PK_SUB_U16,V_PK_MAX_U16,V_PK_MIN_U16,V_PK_ADD_F16,
            case 16:  case 27:  case 28:  case 29:   // V_PK_MUL_F16,V_PK_MIN_NUM_F16,V_PK_MAX_NUM_F16,V_PK_MINIMUM_F16,
            case 30:  case 35:  case 42:  case 43:   // V_PK_MAXIMUM_F16,V_PK_ADD_BF16,V_PK_MUL_BF16,V_PK_MIN_NUM_BF16,
            case 44:   // V_PK_MAX_NUM_BF16,
                if (COND_VOP3P_INST_LITERAL_has_lit_0(insn_long))  {
                    appendOPR_VGPR(layout.VDST,false,true);
                    insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u32,layout.LITERAL)),true,false);
                    appendOPR_SRC(layout.SRC1,true,false);
                }
                else if (COND_VOP3P_INST_LITERAL_has_lit_1(insn_long))  {
                    appendOPR_VGPR(layout.VDST,false,true);
                    appendOPR_SRC(layout.SRC0,true,false);
                    insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u32,layout.LITERAL)),true,false);
                }
                else  {
                    appendOPR_VGPR(layout.VDST,false,true);
                    insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u32,layout.LITERAL)),true,false);
                    insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u32,layout.LITERAL)),true,false);
                }
                break;
            case 31:   // V_PK_FMA_F32,
                if (COND_VOP3P_INST_LITERAL_has_lit_0(insn_long))  {
                    appendOPR_VGPR(layout.VDST,false,true,2);
                    insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u32,layout.LITERAL)),true,false);
                    appendOPR_SRC(layout.SRC1,true,false,2);
                    appendOPR_SRC(layout.SRC2,true,false,2);
                }
                else if (COND_VOP3P_INST_LITERAL_has_lit_1(insn_long))  {
                    appendOPR_VGPR(layout.VDST,false,true,2);
                    appendOPR_SRC(layout.SRC0,true,false,2);
                    insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u32,layout.LITERAL)),true,false);
                    appendOPR_SRC(layout.SRC2,true,false,2);
                }
                else if (COND_VOP3P_INST_LITERAL_has_lit_0_and_has_lit_1(insn_long))  {
                    appendOPR_VGPR(layout.VDST,false,true,2);
                    insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u32,layout.LITERAL)),true,false);
                    insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u32,layout.LITERAL)),true,false);
                    appendOPR_SRC(layout.SRC2,true,false,2);
                }
                else if (COND_VOP3P_INST_LITERAL_has_lit_2(insn_long))  {
                    appendOPR_VGPR(layout.VDST,false,true,2);
                    appendOPR_SRC(layout.SRC0,true,false,2);
                    appendOPR_SRC(layout.SRC1,true,false,2);
                    insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u32,layout.LITERAL)),true,false);
                }
                else if (COND_VOP3P_INST_LITERAL_has_lit_0_and_has_lit_2(insn_long))  {
                    appendOPR_VGPR(layout.VDST,false,true,2);
                    insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u32,layout.LITERAL)),true,false);
                    appendOPR_SRC(layout.SRC1,true,false,2);
                    insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u32,layout.LITERAL)),true,false);
                }
                else if (COND_VOP3P_INST_LITERAL_has_lit_1_and_has_lit_2(insn_long))  {
                    appendOPR_VGPR(layout.VDST,false,true,2);
                    appendOPR_SRC(layout.SRC0,true,false,2);
                    insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u32,layout.LITERAL)),true,false);
                    insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u32,layout.LITERAL)),true,false);
                }
                else  {
                    appendOPR_VGPR(layout.VDST,false,true,2);
                    insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u32,layout.LITERAL)),true,false);
                    insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u32,layout.LITERAL)),true,false);
                    insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u32,layout.LITERAL)),true,false);
                }
                break;
            case 40:  case 41:   // V_PK_MUL_F32,V_PK_ADD_F32,
                if (COND_VOP3P_INST_LITERAL_has_lit_0(insn_long))  {
                    appendOPR_VGPR(layout.VDST,false,true,2);
                    insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u32,layout.LITERAL)),true,false);
                    appendOPR_SRC(layout.SRC1,true,false,2);
                }
                else if (COND_VOP3P_INST_LITERAL_has_lit_1(insn_long))  {
                    appendOPR_VGPR(layout.VDST,false,true,2);
                    appendOPR_SRC(layout.SRC0,true,false,2);
                    insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u32,layout.LITERAL)),true,false);
                }
                else  {
                    appendOPR_VGPR(layout.VDST,false,true,2);
                    insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u32,layout.LITERAL)),true,false);
                    insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u32,layout.LITERAL)),true,false);
                }
                break;
        }
    }

    void InstructionDecoder_amdgpu_gfx1250::finalizeVOP3P_VOP_DPP8Operands()
    {
        layout_VOP3P_VOP_DPP8 & layout = insn_layout.VOP3P_VOP_DPP8;
        switch (layout.OP)  {
            case 32:  case 33:  case 34:  case 61:   // V_FMA_MIX_F32,V_FMA_MIXLO_F16,V_FMA_MIXHI_F16,V_FMA_MIX_F32_BF16,
            case 62:  case 63:   // V_FMA_MIXLO_BF16,V_FMA_MIXHI_BF16,
                appendOPR_VGPR(layout.VDST,false,true);
                appendOPR_VGPR(layout.VSRC0,true,false);
                appendOPR_SRC(layout.SRC1,true,false);
                appendOPR_SRC(layout.SRC2,true,false);
                break;
        }
    }

    void InstructionDecoder_amdgpu_gfx1250::finalizeVOP3P_VOP_DPP16Operands()
    {
        layout_VOP3P_VOP_DPP16 & layout = insn_layout.VOP3P_VOP_DPP16;
        switch (layout.OP)  {
            case 32:  case 33:  case 34:  case 61:   // V_FMA_MIX_F32,V_FMA_MIXLO_F16,V_FMA_MIXHI_F16,V_FMA_MIX_F32_BF16,
            case 62:  case 63:   // V_FMA_MIXLO_BF16,V_FMA_MIXHI_BF16,
                appendOPR_VGPR(layout.VDST,false,true);
                appendOPR_VGPR(layout.VSRC0,true,false);
                appendOPR_SRC(layout.SRC1,true,false);
                appendOPR_SRC(layout.SRC2,true,false);
                break;
        }
    }

    void InstructionDecoder_amdgpu_gfx1250::finalizeENC_VOP3PX2Operands()
    {
        layout_ENC_VOP3PX2 & layout = insn_layout.ENC_VOP3PX2;
        switch (layout.OP)  {
            case 51:   // V_WMMA_SCALE_F32_16X16X128_F8F6F4,
                appendOPR_VGPR(layout.VDST,false,true,8);
                appendOPR_SRC_VGPR(layout.SRC0,true,false,16);
                appendOPR_SRC_VGPR(layout.SRC1,true,false,16);
                appendOPR_SRC_VGPR_OR_INLINE(layout.SRC2,true,false,8);
                appendOPR_SRC_SIMPLE(layout.SCALE_SRC0,true,false);
                appendOPR_SRC_SIMPLE(layout.SCALE_SRC1,true,false);
                break;
            case 136:   // V_WMMA_SCALE_F32_32X16X128_F4,
                appendOPR_VGPR(layout.VDST,false,true,16);
                appendOPR_SRC_VGPR(layout.SRC0,true,false,16);
                appendOPR_SRC_VGPR(layout.SRC1,true,false,8);
                appendOPR_SRC_VGPR_OR_INLINE(layout.SRC2,true,false,16);
                appendOPR_SRC_SIMPLE(layout.SCALE_SRC0,true,false);
                appendOPR_SRC_SIMPLE(layout.SCALE_SRC1,true,false);
                break;
        }
    }

    void InstructionDecoder_amdgpu_gfx1250::finalizeENC_VOP3PX3Operands()
    {
        layout_ENC_VOP3PX3 & layout = insn_layout.ENC_VOP3PX3;
        switch (layout.OP)  {
            case 51:   // V_WMMA_SCALE16_F32_16X16X128_F8F6F4,
                appendOPR_VGPR(layout.VDST,false,true,8);
                appendOPR_SRC_VGPR(layout.SRC0,true,false,16);
                appendOPR_SRC_VGPR(layout.SRC1,true,false,16);
                appendOPR_SRC_VGPR_OR_INLINE(layout.SRC2,true,false,8);
                appendOPR_SRC_SIMPLE(layout.SCALE_SRC0,true,false,2);
                appendOPR_SRC_SIMPLE(layout.SCALE_SRC1,true,false,2);
                break;
            case 136:   // V_WMMA_SCALE16_F32_32X16X128_F4,
                appendOPR_VGPR(layout.VDST,false,true,16);
                appendOPR_SRC_VGPR(layout.SRC0,true,false,16);
                appendOPR_SRC_VGPR(layout.SRC1,true,false,8);
                appendOPR_SRC_VGPR_OR_INLINE(layout.SRC2,true,false,16);
                appendOPR_SRC_SIMPLE(layout.SCALE_SRC0,true,false,2);
                appendOPR_SRC_SIMPLE(layout.SCALE_SRC1,true,false,2);
                break;
        }
    }

    void InstructionDecoder_amdgpu_gfx1250::finalizeENC_VOPCOperands()
    {
        layout_ENC_VOPC & layout = insn_layout.ENC_VOPC;
        switch (layout.OP)  {
            case 1:  case 2:  case 3:  case 4:  case 5:   // V_CMP_LT_F16,V_CMP_EQ_F16,V_CMP_LE_F16,V_CMP_GT_F16,V_CMP_LG_F16,
            case 6:  case 7:  case 8:  case 9:  case 10:   // V_CMP_GE_F16,V_CMP_O_F16,V_CMP_U_F16,V_CMP_NGE_F16,V_CMP_NLG_F16,
            case 11:  case 12:  case 13:  case 14:   // V_CMP_NGT_F16,V_CMP_NLE_F16,V_CMP_NEQ_F16,V_CMP_NLT_F16,
            case 17:  case 18:  case 19:  case 20:  case 21:   // V_CMP_LT_F32,V_CMP_EQ_F32,V_CMP_LE_F32,V_CMP_GT_F32,V_CMP_LG_F32,
            case 22:  case 23:  case 24:  case 25:  case 26:   // V_CMP_GE_F32,V_CMP_O_F32,V_CMP_U_F32,V_CMP_NGE_F32,V_CMP_NLG_F32,
            case 27:  case 28:  case 29:  case 30:   // V_CMP_NGT_F32,V_CMP_NLE_F32,V_CMP_NEQ_F32,V_CMP_NLT_F32,
            case 49:  case 50:  case 51:  case 52:  case 53:   // V_CMP_LT_I16,V_CMP_EQ_I16,V_CMP_LE_I16,V_CMP_GT_I16,V_CMP_NE_I16,
            case 54:  case 57:  case 58:  case 59:  case 60:   // V_CMP_GE_I16,V_CMP_LT_U16,V_CMP_EQ_U16,V_CMP_LE_U16,V_CMP_GT_U16,
            case 61:  case 62:  case 65:  case 66:  case 67:   // V_CMP_NE_U16,V_CMP_GE_U16,V_CMP_LT_I32,V_CMP_EQ_I32,V_CMP_LE_I32,
            case 68:  case 69:  case 70:  case 73:  case 74:   // V_CMP_GT_I32,V_CMP_NE_I32,V_CMP_GE_I32,V_CMP_LT_U32,V_CMP_EQ_U32,
            case 75:  case 76:  case 77:  case 78:   // V_CMP_LE_U32,V_CMP_GT_U32,V_CMP_NE_U32,V_CMP_GE_U32,
            case 125:  case 126:   // V_CMP_CLASS_F16,V_CMP_CLASS_F32,
                appendOPR_VCC(106,false,true);
                appendOPR_SRC(layout.SRC0,true,false);
                appendOPR_VGPR(layout.VSRC1,true,false);
                break;
            case 33:  case 34:  case 35:  case 36:  case 37:   // V_CMP_LT_F64,V_CMP_EQ_F64,V_CMP_LE_F64,V_CMP_GT_F64,V_CMP_LG_F64,
            case 38:  case 39:  case 40:  case 41:  case 42:   // V_CMP_GE_F64,V_CMP_O_F64,V_CMP_U_F64,V_CMP_NGE_F64,V_CMP_NLG_F64,
            case 43:  case 44:  case 45:  case 46:   // V_CMP_NGT_F64,V_CMP_NLE_F64,V_CMP_NEQ_F64,V_CMP_NLT_F64,
            case 81:  case 82:  case 83:  case 84:  case 85:   // V_CMP_LT_I64,V_CMP_EQ_I64,V_CMP_LE_I64,V_CMP_GT_I64,V_CMP_NE_I64,
            case 86:  case 89:  case 90:  case 91:  case 92:   // V_CMP_GE_I64,V_CMP_LT_U64,V_CMP_EQ_U64,V_CMP_LE_U64,V_CMP_GT_U64,
            case 93:  case 94:   // V_CMP_NE_U64,V_CMP_GE_U64,
                appendOPR_VCC(106,false,true);
                appendOPR_SRC(layout.SRC0,true,false,2);
                appendOPR_VGPR(layout.VSRC1,true,false,2);
                break;
            case 127:   // V_CMP_CLASS_F64,
                appendOPR_VCC(106,false,true);
                appendOPR_SRC(layout.SRC0,true,false,2);
                appendOPR_VGPR(layout.VSRC1,true,false);
                break;
            case 129:  case 130:  case 131:  case 132:   // V_CMPX_LT_F16,V_CMPX_EQ_F16,V_CMPX_LE_F16,V_CMPX_GT_F16,
            case 133:  case 134:  case 135:  case 136:   // V_CMPX_LG_F16,V_CMPX_GE_F16,V_CMPX_O_F16,V_CMPX_U_F16,
            case 137:  case 138:  case 139:  case 140:   // V_CMPX_NGE_F16,V_CMPX_NLG_F16,V_CMPX_NGT_F16,V_CMPX_NLE_F16,
            case 141:  case 142:  case 145:  case 146:   // V_CMPX_NEQ_F16,V_CMPX_NLT_F16,V_CMPX_LT_F32,V_CMPX_EQ_F32,
            case 147:  case 148:  case 149:  case 150:   // V_CMPX_LE_F32,V_CMPX_GT_F32,V_CMPX_LG_F32,V_CMPX_GE_F32,
            case 151:  case 152:  case 153:  case 154:   // V_CMPX_O_F32,V_CMPX_U_F32,V_CMPX_NGE_F32,V_CMPX_NLG_F32,
            case 155:  case 156:  case 157:  case 158:   // V_CMPX_NGT_F32,V_CMPX_NLE_F32,V_CMPX_NEQ_F32,V_CMPX_NLT_F32,
            case 177:  case 178:  case 179:  case 180:   // V_CMPX_LT_I16,V_CMPX_EQ_I16,V_CMPX_LE_I16,V_CMPX_GT_I16,
            case 181:  case 182:  case 185:  case 186:   // V_CMPX_NE_I16,V_CMPX_GE_I16,V_CMPX_LT_U16,V_CMPX_EQ_U16,
            case 187:  case 188:  case 189:  case 190:   // V_CMPX_LE_U16,V_CMPX_GT_U16,V_CMPX_NE_U16,V_CMPX_GE_U16,
            case 193:  case 194:  case 195:  case 196:   // V_CMPX_LT_I32,V_CMPX_EQ_I32,V_CMPX_LE_I32,V_CMPX_GT_I32,
            case 197:  case 198:  case 201:  case 202:   // V_CMPX_NE_I32,V_CMPX_GE_I32,V_CMPX_LT_U32,V_CMPX_EQ_U32,
            case 203:  case 204:  case 205:  case 206:   // V_CMPX_LE_U32,V_CMPX_GT_U32,V_CMPX_NE_U32,V_CMPX_GE_U32,
            case 253:  case 254:   // V_CMPX_CLASS_F16,V_CMPX_CLASS_F32,
                appendOPR_EXEC(126,false,true);
                appendOPR_SRC(layout.SRC0,true,false);
                appendOPR_VGPR(layout.VSRC1,true,false);
                appendOPR_SDST_EXEC(126,false,true,1,true);
                break;
            case 161:  case 162:  case 163:  case 164:   // V_CMPX_LT_F64,V_CMPX_EQ_F64,V_CMPX_LE_F64,V_CMPX_GT_F64,
            case 165:  case 166:  case 167:  case 168:   // V_CMPX_LG_F64,V_CMPX_GE_F64,V_CMPX_O_F64,V_CMPX_U_F64,
            case 169:  case 170:  case 171:  case 172:   // V_CMPX_NGE_F64,V_CMPX_NLG_F64,V_CMPX_NGT_F64,V_CMPX_NLE_F64,
            case 173:  case 174:  case 209:  case 210:   // V_CMPX_NEQ_F64,V_CMPX_NLT_F64,V_CMPX_LT_I64,V_CMPX_EQ_I64,
            case 211:  case 212:  case 213:  case 214:   // V_CMPX_LE_I64,V_CMPX_GT_I64,V_CMPX_NE_I64,V_CMPX_GE_I64,
            case 217:  case 218:  case 219:  case 220:   // V_CMPX_LT_U64,V_CMPX_EQ_U64,V_CMPX_LE_U64,V_CMPX_GT_U64,
            case 221:  case 222:   // V_CMPX_NE_U64,V_CMPX_GE_U64,
                appendOPR_EXEC(126,false,true);
                appendOPR_SRC(layout.SRC0,true,false,2);
                appendOPR_VGPR(layout.VSRC1,true,false,2);
                appendOPR_SDST_EXEC(126,false,true,1,true);
                break;
            case 255:   // V_CMPX_CLASS_F64,
                appendOPR_EXEC(126,false,true);
                appendOPR_SRC(layout.SRC0,true,false,2);
                appendOPR_VGPR(layout.VSRC1,true,false);
                appendOPR_SDST_EXEC(126,false,true,1,true);
                break;
        }
    }

    void InstructionDecoder_amdgpu_gfx1250::finalizeVOPC_INST_LITERALOperands()
    {
        layout_VOPC_INST_LITERAL & layout = insn_layout.VOPC_INST_LITERAL;
        switch (layout.OP)  {
            case 1:  case 2:  case 3:  case 4:  case 5:   // V_CMP_LT_F16,V_CMP_EQ_F16,V_CMP_LE_F16,V_CMP_GT_F16,V_CMP_LG_F16,
            case 6:  case 7:  case 8:  case 9:  case 10:   // V_CMP_GE_F16,V_CMP_O_F16,V_CMP_U_F16,V_CMP_NGE_F16,V_CMP_NLG_F16,
            case 11:  case 12:  case 13:  case 14:   // V_CMP_NGT_F16,V_CMP_NLE_F16,V_CMP_NEQ_F16,V_CMP_NLT_F16,
            case 17:  case 18:  case 19:  case 20:  case 21:   // V_CMP_LT_F32,V_CMP_EQ_F32,V_CMP_LE_F32,V_CMP_GT_F32,V_CMP_LG_F32,
            case 22:  case 23:  case 24:  case 25:  case 26:   // V_CMP_GE_F32,V_CMP_O_F32,V_CMP_U_F32,V_CMP_NGE_F32,V_CMP_NLG_F32,
            case 27:  case 28:  case 29:  case 30:   // V_CMP_NGT_F32,V_CMP_NLE_F32,V_CMP_NEQ_F32,V_CMP_NLT_F32,
            case 49:  case 50:  case 51:  case 52:  case 53:   // V_CMP_LT_I16,V_CMP_EQ_I16,V_CMP_LE_I16,V_CMP_GT_I16,V_CMP_NE_I16,
            case 54:  case 57:  case 58:  case 59:  case 60:   // V_CMP_GE_I16,V_CMP_LT_U16,V_CMP_EQ_U16,V_CMP_LE_U16,V_CMP_GT_U16,
            case 61:  case 62:  case 65:  case 66:  case 67:   // V_CMP_NE_U16,V_CMP_GE_U16,V_CMP_LT_I32,V_CMP_EQ_I32,V_CMP_LE_I32,
            case 68:  case 69:  case 70:  case 73:  case 74:   // V_CMP_GT_I32,V_CMP_NE_I32,V_CMP_GE_I32,V_CMP_LT_U32,V_CMP_EQ_U32,
            case 75:  case 76:  case 77:  case 78:   // V_CMP_LE_U32,V_CMP_GT_U32,V_CMP_NE_U32,V_CMP_GE_U32,
            case 125:  case 126:  case 127:   // V_CMP_CLASS_F16,V_CMP_CLASS_F32,V_CMP_CLASS_F64,
                appendOPR_VCC(106,false,true);
                insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u32,layout.LITERAL)),true,false);
                appendOPR_VGPR(layout.VSRC1,true,false);
                break;
            case 33:  case 34:  case 35:  case 36:  case 37:   // V_CMP_LT_F64,V_CMP_EQ_F64,V_CMP_LE_F64,V_CMP_GT_F64,V_CMP_LG_F64,
            case 38:  case 39:  case 40:  case 41:  case 42:   // V_CMP_GE_F64,V_CMP_O_F64,V_CMP_U_F64,V_CMP_NGE_F64,V_CMP_NLG_F64,
            case 43:  case 44:  case 45:  case 46:   // V_CMP_NGT_F64,V_CMP_NLE_F64,V_CMP_NEQ_F64,V_CMP_NLT_F64,
            case 81:  case 82:  case 83:  case 84:  case 85:   // V_CMP_LT_I64,V_CMP_EQ_I64,V_CMP_LE_I64,V_CMP_GT_I64,V_CMP_NE_I64,
            case 86:  case 89:  case 90:  case 91:  case 92:   // V_CMP_GE_I64,V_CMP_LT_U64,V_CMP_EQ_U64,V_CMP_LE_U64,V_CMP_GT_U64,
            case 93:  case 94:   // V_CMP_NE_U64,V_CMP_GE_U64,
                appendOPR_VCC(106,false,true);
                insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u32,layout.LITERAL)),true,false);
                appendOPR_VGPR(layout.VSRC1,true,false,2);
                break;
            case 129:  case 130:  case 131:  case 132:   // V_CMPX_LT_F16,V_CMPX_EQ_F16,V_CMPX_LE_F16,V_CMPX_GT_F16,
            case 133:  case 134:  case 135:  case 136:   // V_CMPX_LG_F16,V_CMPX_GE_F16,V_CMPX_O_F16,V_CMPX_U_F16,
            case 137:  case 138:  case 139:  case 140:   // V_CMPX_NGE_F16,V_CMPX_NLG_F16,V_CMPX_NGT_F16,V_CMPX_NLE_F16,
            case 141:  case 142:  case 145:  case 146:   // V_CMPX_NEQ_F16,V_CMPX_NLT_F16,V_CMPX_LT_F32,V_CMPX_EQ_F32,
            case 147:  case 148:  case 149:  case 150:   // V_CMPX_LE_F32,V_CMPX_GT_F32,V_CMPX_LG_F32,V_CMPX_GE_F32,
            case 151:  case 152:  case 153:  case 154:   // V_CMPX_O_F32,V_CMPX_U_F32,V_CMPX_NGE_F32,V_CMPX_NLG_F32,
            case 155:  case 156:  case 157:  case 158:   // V_CMPX_NGT_F32,V_CMPX_NLE_F32,V_CMPX_NEQ_F32,V_CMPX_NLT_F32,
            case 177:  case 178:  case 179:  case 180:   // V_CMPX_LT_I16,V_CMPX_EQ_I16,V_CMPX_LE_I16,V_CMPX_GT_I16,
            case 181:  case 182:  case 185:  case 186:   // V_CMPX_NE_I16,V_CMPX_GE_I16,V_CMPX_LT_U16,V_CMPX_EQ_U16,
            case 187:  case 188:  case 189:  case 190:   // V_CMPX_LE_U16,V_CMPX_GT_U16,V_CMPX_NE_U16,V_CMPX_GE_U16,
            case 193:  case 194:  case 195:  case 196:   // V_CMPX_LT_I32,V_CMPX_EQ_I32,V_CMPX_LE_I32,V_CMPX_GT_I32,
            case 197:  case 198:  case 201:  case 202:   // V_CMPX_NE_I32,V_CMPX_GE_I32,V_CMPX_LT_U32,V_CMPX_EQ_U32,
            case 203:  case 204:  case 205:  case 206:   // V_CMPX_LE_U32,V_CMPX_GT_U32,V_CMPX_NE_U32,V_CMPX_GE_U32,
            case 253:  case 254:  case 255:   // V_CMPX_CLASS_F16,V_CMPX_CLASS_F32,V_CMPX_CLASS_F64,
                appendOPR_EXEC(126,false,true);
                insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u32,layout.LITERAL)),true,false);
                appendOPR_VGPR(layout.VSRC1,true,false);
                appendOPR_SDST_EXEC(126,false,true,1,true);
                break;
            case 161:  case 162:  case 163:  case 164:   // V_CMPX_LT_F64,V_CMPX_EQ_F64,V_CMPX_LE_F64,V_CMPX_GT_F64,
            case 165:  case 166:  case 167:  case 168:   // V_CMPX_LG_F64,V_CMPX_GE_F64,V_CMPX_O_F64,V_CMPX_U_F64,
            case 169:  case 170:  case 171:  case 172:   // V_CMPX_NGE_F64,V_CMPX_NLG_F64,V_CMPX_NGT_F64,V_CMPX_NLE_F64,
            case 173:  case 174:  case 209:  case 210:   // V_CMPX_NEQ_F64,V_CMPX_NLT_F64,V_CMPX_LT_I64,V_CMPX_EQ_I64,
            case 211:  case 212:  case 213:  case 214:   // V_CMPX_LE_I64,V_CMPX_GT_I64,V_CMPX_NE_I64,V_CMPX_GE_I64,
            case 217:  case 218:  case 219:  case 220:   // V_CMPX_LT_U64,V_CMPX_EQ_U64,V_CMPX_LE_U64,V_CMPX_GT_U64,
            case 221:  case 222:   // V_CMPX_NE_U64,V_CMPX_GE_U64,
                appendOPR_EXEC(126,false,true);
                insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u32,layout.LITERAL)),true,false);
                appendOPR_VGPR(layout.VSRC1,true,false,2);
                appendOPR_SDST_EXEC(126,false,true,1,true);
                break;
        }
    }

    void InstructionDecoder_amdgpu_gfx1250::finalizeVOPC_INST_LITERAL64Operands()
    {
        layout_VOPC_INST_LITERAL64 & layout = insn_layout.VOPC_INST_LITERAL64;
        switch (layout.OP)  {
            case 1:  case 2:  case 3:  case 4:  case 5:   // V_CMP_LT_F16,V_CMP_EQ_F16,V_CMP_LE_F16,V_CMP_GT_F16,V_CMP_LG_F16,
            case 6:  case 7:  case 8:  case 9:  case 10:   // V_CMP_GE_F16,V_CMP_O_F16,V_CMP_U_F16,V_CMP_NGE_F16,V_CMP_NLG_F16,
            case 11:  case 12:  case 13:  case 14:   // V_CMP_NGT_F16,V_CMP_NLE_F16,V_CMP_NEQ_F16,V_CMP_NLT_F16,
            case 17:  case 18:  case 19:  case 20:  case 21:   // V_CMP_LT_F32,V_CMP_EQ_F32,V_CMP_LE_F32,V_CMP_GT_F32,V_CMP_LG_F32,
            case 22:  case 23:  case 24:  case 25:  case 26:   // V_CMP_GE_F32,V_CMP_O_F32,V_CMP_U_F32,V_CMP_NGE_F32,V_CMP_NLG_F32,
            case 27:  case 28:  case 29:  case 30:   // V_CMP_NGT_F32,V_CMP_NLE_F32,V_CMP_NEQ_F32,V_CMP_NLT_F32,
            case 49:  case 50:  case 51:  case 52:  case 53:   // V_CMP_LT_I16,V_CMP_EQ_I16,V_CMP_LE_I16,V_CMP_GT_I16,V_CMP_NE_I16,
            case 54:  case 57:  case 58:  case 59:  case 60:   // V_CMP_GE_I16,V_CMP_LT_U16,V_CMP_EQ_U16,V_CMP_LE_U16,V_CMP_GT_U16,
            case 61:  case 62:  case 65:  case 66:  case 67:   // V_CMP_NE_U16,V_CMP_GE_U16,V_CMP_LT_I32,V_CMP_EQ_I32,V_CMP_LE_I32,
            case 68:  case 69:  case 70:  case 73:  case 74:   // V_CMP_GT_I32,V_CMP_NE_I32,V_CMP_GE_I32,V_CMP_LT_U32,V_CMP_EQ_U32,
            case 75:  case 76:  case 77:  case 78:   // V_CMP_LE_U32,V_CMP_GT_U32,V_CMP_NE_U32,V_CMP_GE_U32,
            case 125:  case 126:  case 127:   // V_CMP_CLASS_F16,V_CMP_CLASS_F32,V_CMP_CLASS_F64,
                appendOPR_VCC(106,false,true);
                insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u64,layout.LITERAL64)),true,false);
                appendOPR_VGPR(layout.VSRC1,true,false);
                break;
            case 33:  case 34:  case 35:  case 36:  case 37:   // V_CMP_LT_F64,V_CMP_EQ_F64,V_CMP_LE_F64,V_CMP_GT_F64,V_CMP_LG_F64,
            case 38:  case 39:  case 40:  case 41:  case 42:   // V_CMP_GE_F64,V_CMP_O_F64,V_CMP_U_F64,V_CMP_NGE_F64,V_CMP_NLG_F64,
            case 43:  case 44:  case 45:  case 46:   // V_CMP_NGT_F64,V_CMP_NLE_F64,V_CMP_NEQ_F64,V_CMP_NLT_F64,
            case 81:  case 82:  case 83:  case 84:  case 85:   // V_CMP_LT_I64,V_CMP_EQ_I64,V_CMP_LE_I64,V_CMP_GT_I64,V_CMP_NE_I64,
            case 86:  case 89:  case 90:  case 91:  case 92:   // V_CMP_GE_I64,V_CMP_LT_U64,V_CMP_EQ_U64,V_CMP_LE_U64,V_CMP_GT_U64,
            case 93:  case 94:   // V_CMP_NE_U64,V_CMP_GE_U64,
                appendOPR_VCC(106,false,true);
                insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u64,layout.LITERAL64)),true,false);
                appendOPR_VGPR(layout.VSRC1,true,false,2);
                break;
            case 129:  case 130:  case 131:  case 132:   // V_CMPX_LT_F16,V_CMPX_EQ_F16,V_CMPX_LE_F16,V_CMPX_GT_F16,
            case 133:  case 134:  case 135:  case 136:   // V_CMPX_LG_F16,V_CMPX_GE_F16,V_CMPX_O_F16,V_CMPX_U_F16,
            case 137:  case 138:  case 139:  case 140:   // V_CMPX_NGE_F16,V_CMPX_NLG_F16,V_CMPX_NGT_F16,V_CMPX_NLE_F16,
            case 141:  case 142:  case 145:  case 146:   // V_CMPX_NEQ_F16,V_CMPX_NLT_F16,V_CMPX_LT_F32,V_CMPX_EQ_F32,
            case 147:  case 148:  case 149:  case 150:   // V_CMPX_LE_F32,V_CMPX_GT_F32,V_CMPX_LG_F32,V_CMPX_GE_F32,
            case 151:  case 152:  case 153:  case 154:   // V_CMPX_O_F32,V_CMPX_U_F32,V_CMPX_NGE_F32,V_CMPX_NLG_F32,
            case 155:  case 156:  case 157:  case 158:   // V_CMPX_NGT_F32,V_CMPX_NLE_F32,V_CMPX_NEQ_F32,V_CMPX_NLT_F32,
            case 177:  case 178:  case 179:  case 180:   // V_CMPX_LT_I16,V_CMPX_EQ_I16,V_CMPX_LE_I16,V_CMPX_GT_I16,
            case 181:  case 182:  case 185:  case 186:   // V_CMPX_NE_I16,V_CMPX_GE_I16,V_CMPX_LT_U16,V_CMPX_EQ_U16,
            case 187:  case 188:  case 189:  case 190:   // V_CMPX_LE_U16,V_CMPX_GT_U16,V_CMPX_NE_U16,V_CMPX_GE_U16,
            case 193:  case 194:  case 195:  case 196:   // V_CMPX_LT_I32,V_CMPX_EQ_I32,V_CMPX_LE_I32,V_CMPX_GT_I32,
            case 197:  case 198:  case 201:  case 202:   // V_CMPX_NE_I32,V_CMPX_GE_I32,V_CMPX_LT_U32,V_CMPX_EQ_U32,
            case 203:  case 204:  case 205:  case 206:   // V_CMPX_LE_U32,V_CMPX_GT_U32,V_CMPX_NE_U32,V_CMPX_GE_U32,
            case 253:  case 254:  case 255:   // V_CMPX_CLASS_F16,V_CMPX_CLASS_F32,V_CMPX_CLASS_F64,
                appendOPR_EXEC(126,false,true);
                insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u64,layout.LITERAL64)),true,false);
                appendOPR_VGPR(layout.VSRC1,true,false);
                appendOPR_SDST_EXEC(126,false,true,1,true);
                break;
            case 161:  case 162:  case 163:  case 164:   // V_CMPX_LT_F64,V_CMPX_EQ_F64,V_CMPX_LE_F64,V_CMPX_GT_F64,
            case 165:  case 166:  case 167:  case 168:   // V_CMPX_LG_F64,V_CMPX_GE_F64,V_CMPX_O_F64,V_CMPX_U_F64,
            case 169:  case 170:  case 171:  case 172:   // V_CMPX_NGE_F64,V_CMPX_NLG_F64,V_CMPX_NGT_F64,V_CMPX_NLE_F64,
            case 173:  case 174:  case 209:  case 210:   // V_CMPX_NEQ_F64,V_CMPX_NLT_F64,V_CMPX_LT_I64,V_CMPX_EQ_I64,
            case 211:  case 212:  case 213:  case 214:   // V_CMPX_LE_I64,V_CMPX_GT_I64,V_CMPX_NE_I64,V_CMPX_GE_I64,
            case 217:  case 218:  case 219:  case 220:   // V_CMPX_LT_U64,V_CMPX_EQ_U64,V_CMPX_LE_U64,V_CMPX_GT_U64,
            case 221:  case 222:   // V_CMPX_NE_U64,V_CMPX_GE_U64,
                appendOPR_EXEC(126,false,true);
                insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u64,layout.LITERAL64)),true,false);
                appendOPR_VGPR(layout.VSRC1,true,false,2);
                appendOPR_SDST_EXEC(126,false,true,1,true);
                break;
        }
    }

    void InstructionDecoder_amdgpu_gfx1250::finalizeVOPC_VOP_DPP8Operands()
    {
        layout_VOPC_VOP_DPP8 & layout = insn_layout.VOPC_VOP_DPP8;
        switch (layout.OP)  {
            case 1:  case 2:  case 3:  case 4:  case 5:   // V_CMP_LT_F16,V_CMP_EQ_F16,V_CMP_LE_F16,V_CMP_GT_F16,V_CMP_LG_F16,
            case 6:  case 7:  case 8:  case 9:  case 10:   // V_CMP_GE_F16,V_CMP_O_F16,V_CMP_U_F16,V_CMP_NGE_F16,V_CMP_NLG_F16,
            case 11:  case 12:  case 13:  case 14:   // V_CMP_NGT_F16,V_CMP_NLE_F16,V_CMP_NEQ_F16,V_CMP_NLT_F16,
            case 17:  case 18:  case 19:  case 20:  case 21:   // V_CMP_LT_F32,V_CMP_EQ_F32,V_CMP_LE_F32,V_CMP_GT_F32,V_CMP_LG_F32,
            case 22:  case 23:  case 24:  case 25:  case 26:   // V_CMP_GE_F32,V_CMP_O_F32,V_CMP_U_F32,V_CMP_NGE_F32,V_CMP_NLG_F32,
            case 27:  case 28:  case 29:  case 30:   // V_CMP_NGT_F32,V_CMP_NLE_F32,V_CMP_NEQ_F32,V_CMP_NLT_F32,
            case 49:  case 50:  case 51:  case 52:  case 53:   // V_CMP_LT_I16,V_CMP_EQ_I16,V_CMP_LE_I16,V_CMP_GT_I16,V_CMP_NE_I16,
            case 54:  case 57:  case 58:  case 59:  case 60:   // V_CMP_GE_I16,V_CMP_LT_U16,V_CMP_EQ_U16,V_CMP_LE_U16,V_CMP_GT_U16,
            case 61:  case 62:  case 65:  case 66:  case 67:   // V_CMP_NE_U16,V_CMP_GE_U16,V_CMP_LT_I32,V_CMP_EQ_I32,V_CMP_LE_I32,
            case 68:  case 69:  case 70:  case 73:  case 74:   // V_CMP_GT_I32,V_CMP_NE_I32,V_CMP_GE_I32,V_CMP_LT_U32,V_CMP_EQ_U32,
            case 75:  case 76:  case 77:  case 78:   // V_CMP_LE_U32,V_CMP_GT_U32,V_CMP_NE_U32,V_CMP_GE_U32,
            case 125:  case 126:   // V_CMP_CLASS_F16,V_CMP_CLASS_F32,
                appendOPR_VCC(106,false,true);
                appendOPR_VGPR(layout.VSRC0,true,false);
                appendOPR_VGPR(layout.VSRC1,true,false);
                break;
            case 129:  case 130:  case 131:  case 132:   // V_CMPX_LT_F16,V_CMPX_EQ_F16,V_CMPX_LE_F16,V_CMPX_GT_F16,
            case 133:  case 134:  case 135:  case 136:   // V_CMPX_LG_F16,V_CMPX_GE_F16,V_CMPX_O_F16,V_CMPX_U_F16,
            case 137:  case 138:  case 139:  case 140:   // V_CMPX_NGE_F16,V_CMPX_NLG_F16,V_CMPX_NGT_F16,V_CMPX_NLE_F16,
            case 141:  case 142:  case 145:  case 146:   // V_CMPX_NEQ_F16,V_CMPX_NLT_F16,V_CMPX_LT_F32,V_CMPX_EQ_F32,
            case 147:  case 148:  case 149:  case 150:   // V_CMPX_LE_F32,V_CMPX_GT_F32,V_CMPX_LG_F32,V_CMPX_GE_F32,
            case 151:  case 152:  case 153:  case 154:   // V_CMPX_O_F32,V_CMPX_U_F32,V_CMPX_NGE_F32,V_CMPX_NLG_F32,
            case 155:  case 156:  case 157:  case 158:   // V_CMPX_NGT_F32,V_CMPX_NLE_F32,V_CMPX_NEQ_F32,V_CMPX_NLT_F32,
            case 177:  case 178:  case 179:  case 180:   // V_CMPX_LT_I16,V_CMPX_EQ_I16,V_CMPX_LE_I16,V_CMPX_GT_I16,
            case 181:  case 182:  case 185:  case 186:   // V_CMPX_NE_I16,V_CMPX_GE_I16,V_CMPX_LT_U16,V_CMPX_EQ_U16,
            case 187:  case 188:  case 189:  case 190:   // V_CMPX_LE_U16,V_CMPX_GT_U16,V_CMPX_NE_U16,V_CMPX_GE_U16,
            case 193:  case 194:  case 195:  case 196:   // V_CMPX_LT_I32,V_CMPX_EQ_I32,V_CMPX_LE_I32,V_CMPX_GT_I32,
            case 197:  case 198:  case 201:  case 202:   // V_CMPX_NE_I32,V_CMPX_GE_I32,V_CMPX_LT_U32,V_CMPX_EQ_U32,
            case 203:  case 204:  case 205:  case 206:   // V_CMPX_LE_U32,V_CMPX_GT_U32,V_CMPX_NE_U32,V_CMPX_GE_U32,
            case 253:  case 254:   // V_CMPX_CLASS_F16,V_CMPX_CLASS_F32,
                appendOPR_EXEC(126,false,true);
                appendOPR_VGPR(layout.VSRC0,true,false);
                appendOPR_VGPR(layout.VSRC1,true,false);
                appendOPR_SDST_EXEC(126,false,true,1,true);
                break;
        }
    }

    void InstructionDecoder_amdgpu_gfx1250::finalizeVOPC_VOP_DPP16Operands()
    {
        layout_VOPC_VOP_DPP16 & layout = insn_layout.VOPC_VOP_DPP16;
        switch (layout.OP)  {
            case 1:  case 2:  case 3:  case 4:  case 5:   // V_CMP_LT_F16,V_CMP_EQ_F16,V_CMP_LE_F16,V_CMP_GT_F16,V_CMP_LG_F16,
            case 6:  case 7:  case 8:  case 9:  case 10:   // V_CMP_GE_F16,V_CMP_O_F16,V_CMP_U_F16,V_CMP_NGE_F16,V_CMP_NLG_F16,
            case 11:  case 12:  case 13:  case 14:   // V_CMP_NGT_F16,V_CMP_NLE_F16,V_CMP_NEQ_F16,V_CMP_NLT_F16,
            case 17:  case 18:  case 19:  case 20:  case 21:   // V_CMP_LT_F32,V_CMP_EQ_F32,V_CMP_LE_F32,V_CMP_GT_F32,V_CMP_LG_F32,
            case 22:  case 23:  case 24:  case 25:  case 26:   // V_CMP_GE_F32,V_CMP_O_F32,V_CMP_U_F32,V_CMP_NGE_F32,V_CMP_NLG_F32,
            case 27:  case 28:  case 29:  case 30:   // V_CMP_NGT_F32,V_CMP_NLE_F32,V_CMP_NEQ_F32,V_CMP_NLT_F32,
            case 49:  case 50:  case 51:  case 52:  case 53:   // V_CMP_LT_I16,V_CMP_EQ_I16,V_CMP_LE_I16,V_CMP_GT_I16,V_CMP_NE_I16,
            case 54:  case 57:  case 58:  case 59:  case 60:   // V_CMP_GE_I16,V_CMP_LT_U16,V_CMP_EQ_U16,V_CMP_LE_U16,V_CMP_GT_U16,
            case 61:  case 62:  case 65:  case 66:  case 67:   // V_CMP_NE_U16,V_CMP_GE_U16,V_CMP_LT_I32,V_CMP_EQ_I32,V_CMP_LE_I32,
            case 68:  case 69:  case 70:  case 73:  case 74:   // V_CMP_GT_I32,V_CMP_NE_I32,V_CMP_GE_I32,V_CMP_LT_U32,V_CMP_EQ_U32,
            case 75:  case 76:  case 77:  case 78:   // V_CMP_LE_U32,V_CMP_GT_U32,V_CMP_NE_U32,V_CMP_GE_U32,
            case 125:  case 126:   // V_CMP_CLASS_F16,V_CMP_CLASS_F32,
                appendOPR_VCC(106,false,true);
                appendOPR_VGPR(layout.VSRC0,true,false);
                appendOPR_VGPR(layout.VSRC1,true,false);
                break;
            case 129:  case 130:  case 131:  case 132:   // V_CMPX_LT_F16,V_CMPX_EQ_F16,V_CMPX_LE_F16,V_CMPX_GT_F16,
            case 133:  case 134:  case 135:  case 136:   // V_CMPX_LG_F16,V_CMPX_GE_F16,V_CMPX_O_F16,V_CMPX_U_F16,
            case 137:  case 138:  case 139:  case 140:   // V_CMPX_NGE_F16,V_CMPX_NLG_F16,V_CMPX_NGT_F16,V_CMPX_NLE_F16,
            case 141:  case 142:  case 145:  case 146:   // V_CMPX_NEQ_F16,V_CMPX_NLT_F16,V_CMPX_LT_F32,V_CMPX_EQ_F32,
            case 147:  case 148:  case 149:  case 150:   // V_CMPX_LE_F32,V_CMPX_GT_F32,V_CMPX_LG_F32,V_CMPX_GE_F32,
            case 151:  case 152:  case 153:  case 154:   // V_CMPX_O_F32,V_CMPX_U_F32,V_CMPX_NGE_F32,V_CMPX_NLG_F32,
            case 155:  case 156:  case 157:  case 158:   // V_CMPX_NGT_F32,V_CMPX_NLE_F32,V_CMPX_NEQ_F32,V_CMPX_NLT_F32,
            case 177:  case 178:  case 179:  case 180:   // V_CMPX_LT_I16,V_CMPX_EQ_I16,V_CMPX_LE_I16,V_CMPX_GT_I16,
            case 181:  case 182:  case 185:  case 186:   // V_CMPX_NE_I16,V_CMPX_GE_I16,V_CMPX_LT_U16,V_CMPX_EQ_U16,
            case 187:  case 188:  case 189:  case 190:   // V_CMPX_LE_U16,V_CMPX_GT_U16,V_CMPX_NE_U16,V_CMPX_GE_U16,
            case 193:  case 194:  case 195:  case 196:   // V_CMPX_LT_I32,V_CMPX_EQ_I32,V_CMPX_LE_I32,V_CMPX_GT_I32,
            case 197:  case 198:  case 201:  case 202:   // V_CMPX_NE_I32,V_CMPX_GE_I32,V_CMPX_LT_U32,V_CMPX_EQ_U32,
            case 203:  case 204:  case 205:  case 206:   // V_CMPX_LE_U32,V_CMPX_GT_U32,V_CMPX_NE_U32,V_CMPX_GE_U32,
            case 253:  case 254:   // V_CMPX_CLASS_F16,V_CMPX_CLASS_F32,
                appendOPR_EXEC(126,false,true);
                appendOPR_VGPR(layout.VSRC0,true,false);
                appendOPR_VGPR(layout.VSRC1,true,false);
                appendOPR_SDST_EXEC(126,false,true,1,true);
                break;
        }
    }

    void InstructionDecoder_amdgpu_gfx1250::finalizeVOPD3XY_XOperands()
    {
        layout_VOPD3XY_X & layout = insn_layout.VOPD3XY_X;
        switch (layout.OPX)  {
            case 32:   // V_DUAL_FMA_F64,
                appendOPR_VGPR(layout.VDSTX,false,true,2);
                appendOPR_SRC_SIMPLE(layout.SRCX0,true,false,2);
                appendOPR_VGPR(layout.VSRCX1,true,false,2);
                appendOPR_VGPR(layout.VSRCX2,true,false,2);
                break;
            case 33:  case 34:  case 35:  case 36:   // V_DUAL_ADD_F64,V_DUAL_MUL_F64,V_DUAL_MAX_NUM_F64,V_DUAL_MIN_NUM_F64,
                appendOPR_VGPR(layout.VDSTX,false,true,2);
                appendOPR_SRC_SIMPLE(layout.SRCX0,true,false,2);
                appendOPR_VGPR(layout.VSRCX1,true,false,2);
                break;
            case 0:  case 3:  case 4:  case 5:   // V_DUAL_FMAC_F32,V_DUAL_MUL_F32,V_DUAL_ADD_F32,V_DUAL_SUB_F32,
            case 6:  case 7:  case 10:  case 11:   // V_DUAL_SUBREV_F32,V_DUAL_MUL_DX9_ZERO_F32,V_DUAL_MAX_NUM_F32,V_DUAL_MIN_NUM_F32,
            case 16:  case 17:  case 20:  case 21:   // V_DUAL_ADD_NC_U32,V_DUAL_LSHLREV_B32,V_DUAL_SUB_NC_U32,V_DUAL_LSHRREV_B32,
            case 22:   // V_DUAL_ASHRREV_I32,
                appendOPR_VGPR(layout.VDSTX,false,true);
                appendOPR_SRC_SIMPLE(layout.SRCX0,true,false);
                appendOPR_VGPR(layout.VSRCX1,true,false);
                break;
            case 8:   // V_DUAL_MOV_B32,
                appendOPR_VGPR(layout.VDSTX,false,true);
                appendOPR_SRC_SIMPLE(layout.SRCX0,true,false);
                break;
            case 9:   // V_DUAL_CNDMASK_B32,
                appendOPR_VGPR(layout.VDSTX,false,true);
                appendOPR_SRC_SIMPLE(layout.SRCX0,true,false);
                appendOPR_VGPR(layout.VSRCX1,true,false);
                appendOPR_SREG(layout.VSRCX2,true,false);
                break;
            case 19:   // V_DUAL_FMA_F32,
                appendOPR_VGPR(layout.VDSTX,false,true);
                appendOPR_SRC_SIMPLE(layout.SRCX0,true,false);
                appendOPR_VGPR(layout.VSRCX1,true,false);
                appendOPR_VGPR(layout.VSRCX2,true,false);
                break;
        }
    }

    void InstructionDecoder_amdgpu_gfx1250::finalizeVOPD3XY_YOperands()
    {
        layout_VOPD3XY_Y & layout = insn_layout.VOPD3XY_Y;
        switch (layout.OPY)  {
            case 0:  case 3:  case 4:  case 5:   // V_DUAL_FMAC_F32,V_DUAL_MUL_F32,V_DUAL_ADD_F32,V_DUAL_SUB_F32,
            case 6:  case 7:  case 10:  case 11:   // V_DUAL_SUBREV_F32,V_DUAL_MUL_DX9_ZERO_F32,V_DUAL_MAX_NUM_F32,V_DUAL_MIN_NUM_F32,
            case 16:  case 17:  case 18:  case 20:   // V_DUAL_ADD_NC_U32,V_DUAL_LSHLREV_B32,V_DUAL_BITOP2_B32,V_DUAL_SUB_NC_U32,
            case 21:  case 22:  case 23:  case 24:   // V_DUAL_LSHRREV_B32,V_DUAL_ASHRREV_I32,V_DUAL_MAX_I32,V_DUAL_MIN_I32,
                appendOPR_VGPR(layout.VDSTY,false,true);
                appendOPR_SRC_SIMPLE(layout.SRCY0,true,false);
                appendOPR_VGPR(layout.VSRCY1,true,false);
                break;
            case 8:   // V_DUAL_MOV_B32,
                appendOPR_VGPR(layout.VDSTY,false,true);
                appendOPR_SRC_SIMPLE(layout.SRCY0,true,false);
                break;
            case 9:   // V_DUAL_CNDMASK_B32,
                appendOPR_VGPR(layout.VDSTY,false,true);
                appendOPR_SRC_SIMPLE(layout.SRCY0,true,false);
                appendOPR_VGPR(layout.VSRCY1,true,false);
                appendOPR_SREG(layout.VSRCY2,true,false);
                break;
            case 19:   // V_DUAL_FMA_F32,
                appendOPR_VGPR(layout.VDSTY,false,true);
                appendOPR_SRC_SIMPLE(layout.SRCY0,true,false);
                appendOPR_VGPR(layout.VSRCY1,true,false);
                appendOPR_VGPR(layout.VSRCY2,true,false);
                break;
        }
    }

    void InstructionDecoder_amdgpu_gfx1250::finalizeVOPDXY_XOperands()
    {
        layout_VOPDXY_X & layout = insn_layout.VOPDXY_X;
        switch (layout.OPX)  {
            case 0:  case 3:  case 4:  case 5:   // V_DUAL_FMAC_F32,V_DUAL_MUL_F32,V_DUAL_ADD_F32,V_DUAL_SUB_F32,
            case 6:  case 7:  case 10:  case 11:   // V_DUAL_SUBREV_F32,V_DUAL_MUL_DX9_ZERO_F32,V_DUAL_MAX_NUM_F32,V_DUAL_MIN_NUM_F32,
                appendOPR_VGPR(layout.VDSTX,false,true);
                appendOPR_SRC(layout.SRCX0,true,false);
                appendOPR_VGPR(layout.VSRCX1,true,false);
                break;
            case 8:   // V_DUAL_MOV_B32,
                appendOPR_VGPR(layout.VDSTX,false,true);
                appendOPR_SRC(layout.SRCX0,true,false);
                break;
            case 9:   // V_DUAL_CNDMASK_B32,
                appendOPR_VGPR(layout.VDSTX,false,true);
                appendOPR_SRC(layout.SRCX0,true,false);
                appendOPR_VGPR(layout.VSRCX1,true,false);
                appendOPR_VCC(106,true,false);
                break;
        }
    }

    void InstructionDecoder_amdgpu_gfx1250::finalizeVOPDXY_INST_LITERAL_XOperands()
    {
        layout_VOPDXY_INST_LITERAL_X & layout = insn_layout.VOPDXY_INST_LITERAL_X;
        switch (layout.OPX)  {
            case 0:  case 3:  case 4:  case 5:   // V_DUAL_FMAC_F32,V_DUAL_MUL_F32,V_DUAL_ADD_F32,V_DUAL_SUB_F32,
            case 6:  case 7:  case 10:  case 11:   // V_DUAL_SUBREV_F32,V_DUAL_MUL_DX9_ZERO_F32,V_DUAL_MAX_NUM_F32,V_DUAL_MIN_NUM_F32,
                appendOPR_VGPR(layout.VDSTX,false,true);
                insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u32,layout.LITERAL)),true,false);
                appendOPR_VGPR(layout.VSRCX1,true,false);
                break;
            case 1:   // V_DUAL_FMAAK_F32,
                if (COND_VOPDXY_INST_LITERAL_X_default(insn_long))  {
                    appendOPR_VGPR(layout.VDSTX,false,true);
                    appendOPR_SRC(layout.SRCX0,true,false);
                    appendOPR_VGPR(layout.VSRCX1,true,false);
                    insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u32,layout.LITERAL)),true,false);
                }
                else  {
                    appendOPR_VGPR(layout.VDSTX,false,true);
                    insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u32,layout.LITERAL)),true,false);
                    appendOPR_VGPR(layout.VSRCX1,true,false);
                    insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u32,layout.LITERAL)),true,false);
                }
                break;
            case 2:   // V_DUAL_FMAMK_F32,
                if (COND_VOPDXY_INST_LITERAL_X_default(insn_long))  {
                    appendOPR_VGPR(layout.VDSTX,false,true);
                    appendOPR_SRC(layout.SRCX0,true,false);
                    insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u32,layout.LITERAL)),true,false);
                    appendOPR_VGPR(layout.VSRCX1,true,false);
                }
                else  {
                    appendOPR_VGPR(layout.VDSTX,false,true);
                    insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u32,layout.LITERAL)),true,false);
                    insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u32,layout.LITERAL)),true,false);
                    appendOPR_VGPR(layout.VSRCX1,true,false);
                }
                break;
            case 8:   // V_DUAL_MOV_B32,
                appendOPR_VGPR(layout.VDSTX,false,true);
                insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u32,layout.LITERAL)),true,false);
                break;
            case 9:   // V_DUAL_CNDMASK_B32,
                appendOPR_VGPR(layout.VDSTX,false,true);
                insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u32,layout.LITERAL)),true,false);
                appendOPR_VGPR(layout.VSRCX1,true,false);
                appendOPR_VCC(106,true,false);
                break;
        }
    }

    void InstructionDecoder_amdgpu_gfx1250::finalizeVOPDXY_YOperands()
    {
        layout_VOPDXY_Y & layout = insn_layout.VOPDXY_Y;
        switch (layout.OPY)  {
            case 0:  case 3:  case 4:  case 5:   // V_DUAL_FMAC_F32,V_DUAL_MUL_F32,V_DUAL_ADD_F32,V_DUAL_SUB_F32,
            case 6:  case 7:  case 10:  case 11:   // V_DUAL_SUBREV_F32,V_DUAL_MUL_DX9_ZERO_F32,V_DUAL_MAX_NUM_F32,V_DUAL_MIN_NUM_F32,
            case 16:  case 17:  case 20:  case 21:   // V_DUAL_ADD_NC_U32,V_DUAL_LSHLREV_B32,V_DUAL_SUB_NC_U32,V_DUAL_LSHRREV_B32,
            case 22:  case 23:  case 24:   // V_DUAL_ASHRREV_I32,V_DUAL_MAX_I32,V_DUAL_MIN_I32,
                appendOPR_VGPR(layout.VDSTY | ((layout.VDSTX & 1) ^ 1),false,true);
                appendOPR_SRC(layout.SRCY0,true,false);
                appendOPR_VGPR(layout.VSRCY1,true,false);
                break;
            case 8:   // V_DUAL_MOV_B32,
                appendOPR_VGPR(layout.VDSTY | ((layout.VDSTX & 1) ^ 1),false,true);
                appendOPR_SRC(layout.SRCY0,true,false);
                break;
            case 9:   // V_DUAL_CNDMASK_B32,
                appendOPR_VGPR(layout.VDSTY | ((layout.VDSTX & 1) ^ 1),false,true);
                appendOPR_SRC(layout.SRCY0,true,false);
                appendOPR_VGPR(layout.VSRCY1,true,false);
                appendOPR_VCC(106,true,false);
                break;
        }
    }

    void InstructionDecoder_amdgpu_gfx1250::finalizeVOPDXY_INST_LITERAL_YOperands()
    {
        layout_VOPDXY_INST_LITERAL_Y & layout = insn_layout.VOPDXY_INST_LITERAL_Y;
        switch (layout.OPY)  {
            case 0:  case 3:  case 4:  case 5:   // V_DUAL_FMAC_F32,V_DUAL_MUL_F32,V_DUAL_ADD_F32,V_DUAL_SUB_F32,
            case 6:  case 7:  case 10:  case 11:   // V_DUAL_SUBREV_F32,V_DUAL_MUL_DX9_ZERO_F32,V_DUAL_MAX_NUM_F32,V_DUAL_MIN_NUM_F32,
            case 16:  case 17:  case 20:  case 21:   // V_DUAL_ADD_NC_U32,V_DUAL_LSHLREV_B32,V_DUAL_SUB_NC_U32,V_DUAL_LSHRREV_B32,
            case 22:  case 23:  case 24:   // V_DUAL_ASHRREV_I32,V_DUAL_MAX_I32,V_DUAL_MIN_I32,
                appendOPR_VGPR(layout.VDSTY | ((layout.VDSTX & 1) ^ 1),false,true);
                insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u32,layout.LITERAL)),true,false);
                appendOPR_VGPR(layout.VSRCY1,true,false);
                break;
            case 1:   // V_DUAL_FMAAK_F32,
                if (COND_VOPDXY_INST_LITERAL_Y_default(insn_long))  {
                    appendOPR_VGPR(layout.VDSTY | ((layout.VDSTX & 1) ^ 1),false,true);
                    appendOPR_SRC(layout.SRCY0,true,false);
                    appendOPR_VGPR(layout.VSRCY1,true,false);
                    insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u32,layout.LITERAL)),true,false);
                }
                else  {
                    appendOPR_VGPR(layout.VDSTY | ((layout.VDSTX & 1) ^ 1),false,true);
                    insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u32,layout.LITERAL)),true,false);
                    appendOPR_VGPR(layout.VSRCY1,true,false);
                    insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u32,layout.LITERAL)),true,false);
                }
                break;
            case 2:   // V_DUAL_FMAMK_F32,
                if (COND_VOPDXY_INST_LITERAL_Y_default(insn_long))  {
                    appendOPR_VGPR(layout.VDSTY | ((layout.VDSTX & 1) ^ 1),false,true);
                    appendOPR_SRC(layout.SRCY0,true,false);
                    insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u32,layout.LITERAL)),true,false);
                    appendOPR_VGPR(layout.VSRCY1,true,false);
                }
                else  {
                    appendOPR_VGPR(layout.VDSTY | ((layout.VDSTX & 1) ^ 1),false,true);
                    insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u32,layout.LITERAL)),true,false);
                    insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u32,layout.LITERAL)),true,false);
                    appendOPR_VGPR(layout.VSRCY1,true,false);
                }
                break;
            case 8:   // V_DUAL_MOV_B32,
                appendOPR_VGPR(layout.VDSTY | ((layout.VDSTX & 1) ^ 1),false,true);
                insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u32,layout.LITERAL)),true,false);
                break;
            case 9:   // V_DUAL_CNDMASK_B32,
                appendOPR_VGPR(layout.VDSTY | ((layout.VDSTX & 1) ^ 1),false,true);
                insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u32,layout.LITERAL)),true,false);
                appendOPR_VGPR(layout.VSRCY1,true,false);
                appendOPR_VCC(106,true,false);
                break;
        }
    }

    void InstructionDecoder_amdgpu_gfx1250::finalizeENC_VSCRATCHOperands()
    {
        layout_ENC_VSCRATCH & layout = insn_layout.ENC_VSCRATCH;
        switch (layout.OP)  {
            case 16:  case 17:  case 18:  case 19:   // SCRATCH_LOAD_U8,SCRATCH_LOAD_I8,SCRATCH_LOAD_U16,SCRATCH_LOAD_I16,
            case 20:  case 30:  case 31:  case 32:   // SCRATCH_LOAD_B32,SCRATCH_LOAD_D16_U8,SCRATCH_LOAD_D16_I8,SCRATCH_LOAD_D16_B16,
            case 33:  case 34:  case 35:   // SCRATCH_LOAD_D16_HI_U8,SCRATCH_LOAD_D16_HI_I8,SCRATCH_LOAD_D16_HI_B16,
                appendOPR_VGPR(layout.VDST,false,true);
                if (layout.SVE)  {
                    appendOPR_VGPR(layout.VADDR,true,false);
                }
                if (layout.SADDR != 124)  {
                    appendOPR_SREG(layout.SADDR,true,false);
                }
                appendOPR_GPUMEM(0,true,false,1,true);
                break;
            case 21:   // SCRATCH_LOAD_B64,
                appendOPR_VGPR(layout.VDST,false,true,2);
                if (layout.SVE)  {
                    appendOPR_VGPR(layout.VADDR,true,false);
                }
                if (layout.SADDR != 124)  {
                    appendOPR_SREG(layout.SADDR,true,false);
                }
                appendOPR_GPUMEM(0,true,false,2,true);
                break;
            case 22:   // SCRATCH_LOAD_B96,
                appendOPR_VGPR(layout.VDST,false,true,3);
                if (layout.SVE)  {
                    appendOPR_VGPR(layout.VADDR,true,false);
                }
                if (layout.SADDR != 124)  {
                    appendOPR_SREG(layout.SADDR,true,false);
                }
                appendOPR_GPUMEM(0,true,false,3,true);
                break;
            case 23:   // SCRATCH_LOAD_B128,
                appendOPR_VGPR(layout.VDST,false,true,4);
                if (layout.SVE)  {
                    appendOPR_VGPR(layout.VADDR,true,false);
                }
                if (layout.SADDR != 124)  {
                    appendOPR_SREG(layout.SADDR,true,false);
                }
                appendOPR_GPUMEM(0,true,false,4,true);
                break;
            case 24:  case 25:  case 26:  case 36:   // SCRATCH_STORE_B8,SCRATCH_STORE_B16,SCRATCH_STORE_B32,SCRATCH_STORE_D16_HI_B8,
            case 37:   // SCRATCH_STORE_D16_HI_B16,
                if (layout.SVE)  {
                    appendOPR_VGPR(layout.VADDR,true,false);
                }
                appendOPR_VGPR(layout.VSRC,true,false);
                if (layout.SADDR != 124)  {
                    appendOPR_SREG(layout.SADDR,true,false);
                }
                appendOPR_GPUMEM(0,false,true,1,true);
                break;
            case 27:   // SCRATCH_STORE_B64,
                if (layout.SVE)  {
                    appendOPR_VGPR(layout.VADDR,true,false);
                }
                appendOPR_VGPR(layout.VSRC,true,false,2);
                if (layout.SADDR != 124)  {
                    appendOPR_SREG(layout.SADDR,true,false);
                }
                appendOPR_GPUMEM(0,false,true,2,true);
                break;
            case 28:   // SCRATCH_STORE_B96,
                if (layout.SVE)  {
                    appendOPR_VGPR(layout.VADDR,true,false);
                }
                appendOPR_VGPR(layout.VSRC,true,false,3);
                if (layout.SADDR != 124)  {
                    appendOPR_SREG(layout.SADDR,true,false);
                }
                appendOPR_GPUMEM(0,false,true,3,true);
                break;
            case 29:   // SCRATCH_STORE_B128,
                if (layout.SVE)  {
                    appendOPR_VGPR(layout.VADDR,true,false);
                }
                appendOPR_VGPR(layout.VSRC,true,false,4);
                if (layout.SADDR != 124)  {
                    appendOPR_SREG(layout.SADDR,true,false);
                }
                appendOPR_GPUMEM(0,false,true,4,true);
                break;
            case 83:   // SCRATCH_LOAD_BLOCK,
                appendOPR_VGPR(layout.VDST,false,true,32);
                if (layout.SVE)  {
                    appendOPR_VGPR(layout.VADDR,true,false);
                }
                if (layout.SADDR != 124)  {
                    appendOPR_SREG(layout.SADDR,true,false);
                }
                appendOPR_GPUMEM(0,true,false,32,true);
                appendOPR_SDST_M0(125,true,false,1,true);
                break;
            case 84:   // SCRATCH_STORE_BLOCK,
                if (layout.SVE)  {
                    appendOPR_VGPR(layout.VADDR,true,false);
                }
                appendOPR_VGPR(layout.VSRC,true,false,32);
                if (layout.SADDR != 124)  {
                    appendOPR_SREG(layout.SADDR,true,false);
                }
                appendOPR_GPUMEM(0,false,true,32,true);
                appendOPR_SDST_M0(125,true,false,1,true);
                break;
        }
    }

}
}
