#include "registers/AMDGPU/amdgpu_gfx90a_regs.h"
#include "InstructionDecoder-amdgpu-gfx90a.h"

namespace Dyninst {
namespace InstructionAPI {
    void InstructionDecoder_amdgpu_gfx90a::finalizeENC_DSOperands()
    {
        layout_ENC_DS & layout = insn_layout.ENC_DS;
        switch (layout.OP)  {
            case 0:  case 1:  case 2:  case 3:  case 4:   // DS_ADD_U32,DS_SUB_U32,DS_RSUB_U32,DS_INC_U32,DS_DEC_U32,
            case 5:  case 6:  case 7:  case 8:  case 9:   // DS_MIN_I32,DS_MAX_I32,DS_MIN_U32,DS_MAX_U32,DS_AND_B32,
            case 10:  case 11:  case 13:  case 18:  case 19:   // DS_OR_B32,DS_XOR_B32,DS_WRITE_B32,DS_MIN_F32,DS_MAX_F32,
            case 21:  case 30:  case 31:  case 84:   // DS_ADD_F32,DS_WRITE_B8,DS_WRITE_B16,DS_WRITE_B8_D16_HI,
            case 85:   // DS_WRITE_B16_D16_HI,
                appendOPR_VGPR(layout.ADDR,true,false);
                appendOPR_VGPR_OR_ACCVGPR(layout.DATA0,true,false);
                appendOPR_DSMEM(0,true,false,1,true);
                if (layout.OFFSET0 || layout.OFFSET1)  {
                    insn_in_progress->appendOperand(NamedImmediate::makeNamedImmediate(std::string("offset"),Result(u16,(layout.OFFSET1 << 8) + layout.OFFSET0)),true,false,false);
                }
                break;
            case 12:  case 16:  case 17:   // DS_MSKOR_B32,DS_CMPST_B32,DS_CMPST_F32,
                appendOPR_VGPR(layout.ADDR,true,false);
                appendOPR_VGPR_OR_ACCVGPR(layout.DATA0,true,false);
                appendOPR_VGPR_OR_ACCVGPR(layout.DATA1,true,false);
                appendOPR_DSMEM(0,true,false,1,true);
                if (layout.OFFSET0 || layout.OFFSET1)  {
                    insn_in_progress->appendOperand(NamedImmediate::makeNamedImmediate(std::string("offset"),Result(u16,(layout.OFFSET1 << 8) + layout.OFFSET0)),true,false,false);
                }
                break;
            case 14:  case 15:   // DS_WRITE2_B32,DS_WRITE2ST64_B32,
                appendOPR_VGPR(layout.ADDR,true,false);
                appendOPR_VGPR_OR_ACCVGPR(layout.DATA0,true,false);
                appendOPR_VGPR_OR_ACCVGPR(layout.DATA1,true,false);
                appendOPR_DSMEM(0,true,false,1,true);
                if (layout.OFFSET0)  {
                    insn_in_progress->appendOperand(NamedImmediate::makeNamedImmediate(std::string("offset0"),Result(u8,layout.OFFSET0)),true,false,false);
                }
                if (layout.OFFSET1)  {
                    insn_in_progress->appendOperand(NamedImmediate::makeNamedImmediate(std::string("offset1"),Result(u8,layout.OFFSET1)),true,false,false);
                }
                break;
            case 20:   // DS_NOP,
                if (layout.OFFSET0 || layout.OFFSET1)  {
                    insn_in_progress->appendOperand(NamedImmediate::makeNamedImmediate(std::string("offset"),Result(u16,(layout.OFFSET1 << 8) + layout.OFFSET0)),true,false,false);
                }
                break;
            case 29:   // DS_WRITE_ADDTID_B32,
                appendOPR_VGPR_OR_ACCVGPR(layout.DATA0,true,false);
                appendOPR_DSMEM(0,true,false,1,true);
                appendOPR_SDST_M0(124,true,false,1,true);
                if (layout.OFFSET0 || layout.OFFSET1)  {
                    insn_in_progress->appendOperand(NamedImmediate::makeNamedImmediate(std::string("offset"),Result(u16,(layout.OFFSET1 << 8) + layout.OFFSET0)),true,false,false);
                }
                break;
            case 32:  case 33:  case 34:  case 35:   // DS_ADD_RTN_U32,DS_SUB_RTN_U32,DS_RSUB_RTN_U32,DS_INC_RTN_U32,
            case 36:  case 37:  case 38:  case 39:   // DS_DEC_RTN_U32,DS_MIN_RTN_I32,DS_MAX_RTN_I32,DS_MIN_RTN_U32,
            case 40:  case 41:  case 42:  case 43:   // DS_MAX_RTN_U32,DS_AND_RTN_B32,DS_OR_RTN_B32,DS_XOR_RTN_B32,
            case 45:  case 50:  case 51:  case 53:   // DS_WRXCHG_RTN_B32,DS_MIN_RTN_F32,DS_MAX_RTN_F32,DS_ADD_RTN_F32,
                appendOPR_VGPR_OR_ACCVGPR(layout.VDST,false,true);
                appendOPR_VGPR(layout.ADDR,true,false);
                appendOPR_VGPR_OR_ACCVGPR(layout.DATA0,true,false);
                appendOPR_DSMEM(0,true,false,1,true);
                if (layout.OFFSET0 || layout.OFFSET1)  {
                    insn_in_progress->appendOperand(NamedImmediate::makeNamedImmediate(std::string("offset"),Result(u16,(layout.OFFSET1 << 8) + layout.OFFSET0)),true,false,false);
                }
                break;
            case 44:  case 48:  case 49:  case 52:   // DS_MSKOR_RTN_B32,DS_CMPST_RTN_B32,DS_CMPST_RTN_F32,DS_WRAP_RTN_B32,
                appendOPR_VGPR_OR_ACCVGPR(layout.VDST,false,true);
                appendOPR_VGPR(layout.ADDR,true,false);
                appendOPR_VGPR_OR_ACCVGPR(layout.DATA0,true,false);
                appendOPR_VGPR_OR_ACCVGPR(layout.DATA1,true,false);
                appendOPR_DSMEM(0,true,false,1,true);
                if (layout.OFFSET0 || layout.OFFSET1)  {
                    insn_in_progress->appendOperand(NamedImmediate::makeNamedImmediate(std::string("offset"),Result(u16,(layout.OFFSET1 << 8) + layout.OFFSET0)),true,false,false);
                }
                break;
            case 46:  case 47:   // DS_WRXCHG2_RTN_B32,DS_WRXCHG2ST64_RTN_B32,
                appendOPR_VGPR_OR_ACCVGPR(layout.VDST,false,true,2);
                appendOPR_VGPR(layout.ADDR,true,false);
                appendOPR_VGPR_OR_ACCVGPR(layout.DATA0,true,false);
                appendOPR_VGPR_OR_ACCVGPR(layout.DATA1,true,false);
                appendOPR_DSMEM(0,true,false,1,true);
                if (layout.OFFSET0 || layout.OFFSET1)  {
                    insn_in_progress->appendOperand(NamedImmediate::makeNamedImmediate(std::string("offset"),Result(u16,(layout.OFFSET1 << 8) + layout.OFFSET0)),true,false,false);
                }
                break;
            case 54:  case 57:  case 58:  case 59:  case 60:   // DS_READ_B32,DS_READ_I8,DS_READ_U8,DS_READ_I16,DS_READ_U16,
            case 86:  case 87:  case 88:  case 89:   // DS_READ_U8_D16,DS_READ_U8_D16_HI,DS_READ_I8_D16,DS_READ_I8_D16_HI,
            case 90:  case 91:   // DS_READ_U16_D16,DS_READ_U16_D16_HI,
                appendOPR_VGPR_OR_ACCVGPR(layout.VDST,false,true);
                appendOPR_VGPR(layout.ADDR,true,false);
                appendOPR_DSMEM(0,true,false,1,true);
                if (layout.OFFSET0 || layout.OFFSET1)  {
                    insn_in_progress->appendOperand(NamedImmediate::makeNamedImmediate(std::string("offset"),Result(u16,(layout.OFFSET1 << 8) + layout.OFFSET0)),true,false,false);
                }
                break;
            case 55:  case 56:   // DS_READ2_B32,DS_READ2ST64_B32,
                appendOPR_VGPR_OR_ACCVGPR(layout.VDST,false,true,2);
                appendOPR_VGPR(layout.ADDR,true,false);
                appendOPR_DSMEM(0,true,false,1,true);
                if (layout.OFFSET0)  {
                    insn_in_progress->appendOperand(NamedImmediate::makeNamedImmediate(std::string("offset0"),Result(u8,layout.OFFSET0)),true,false,false);
                }
                if (layout.OFFSET1)  {
                    insn_in_progress->appendOperand(NamedImmediate::makeNamedImmediate(std::string("offset1"),Result(u8,layout.OFFSET1)),true,false,false);
                }
                break;
            case 61:   // DS_SWIZZLE_B32,
                appendOPR_VGPR_OR_ACCVGPR(layout.VDST,false,true);
                appendOPR_VGPR(layout.ADDR,true,false);
                if (layout.OFFSET0 || layout.OFFSET1)  {
                    insn_in_progress->appendOperand(NamedImmediate::makeNamedImmediate(std::string("offset"),Result(u16,(layout.OFFSET1 << 8) + layout.OFFSET0)),true,false,false);
                }
                break;
            case 62:  case 63:   // DS_PERMUTE_B32,DS_BPERMUTE_B32,
                appendOPR_VGPR_OR_ACCVGPR(layout.VDST,false,true);
                appendOPR_VGPR(layout.ADDR,true,false);
                appendOPR_VGPR_OR_ACCVGPR(layout.DATA0,true,false);
                if (layout.OFFSET0 || layout.OFFSET1)  {
                    insn_in_progress->appendOperand(NamedImmediate::makeNamedImmediate(std::string("offset"),Result(u16,(layout.OFFSET1 << 8) + layout.OFFSET0)),true,false,false);
                }
                break;
            case 64:  case 65:  case 66:  case 67:  case 68:   // DS_ADD_U64,DS_SUB_U64,DS_RSUB_U64,DS_INC_U64,DS_DEC_U64,
            case 69:  case 70:  case 71:  case 72:  case 73:   // DS_MIN_I64,DS_MAX_I64,DS_MIN_U64,DS_MAX_U64,DS_AND_B64,
            case 74:  case 75:  case 77:  case 82:  case 83:   // DS_OR_B64,DS_XOR_B64,DS_WRITE_B64,DS_MIN_F64,DS_MAX_F64,
                appendOPR_VGPR(layout.ADDR,true,false);
                appendOPR_VGPR_OR_ACCVGPR(layout.DATA0,true,false,2);
                appendOPR_DSMEM(0,true,false,1,true);
                if (layout.OFFSET0 || layout.OFFSET1)  {
                    insn_in_progress->appendOperand(NamedImmediate::makeNamedImmediate(std::string("offset"),Result(u16,(layout.OFFSET1 << 8) + layout.OFFSET0)),true,false,false);
                }
                break;
            case 76:  case 80:  case 81:   // DS_MSKOR_B64,DS_CMPST_B64,DS_CMPST_F64,
                appendOPR_VGPR(layout.ADDR,true,false);
                appendOPR_VGPR_OR_ACCVGPR(layout.DATA0,true,false,2);
                appendOPR_VGPR_OR_ACCVGPR(layout.DATA1,true,false,2);
                appendOPR_DSMEM(0,true,false,1,true);
                if (layout.OFFSET0 || layout.OFFSET1)  {
                    insn_in_progress->appendOperand(NamedImmediate::makeNamedImmediate(std::string("offset"),Result(u16,(layout.OFFSET1 << 8) + layout.OFFSET0)),true,false,false);
                }
                break;
            case 78:  case 79:   // DS_WRITE2_B64,DS_WRITE2ST64_B64,
                appendOPR_VGPR(layout.ADDR,true,false);
                appendOPR_VGPR_OR_ACCVGPR(layout.DATA0,true,false,2);
                appendOPR_VGPR_OR_ACCVGPR(layout.DATA1,true,false,2);
                appendOPR_DSMEM(0,true,false,1,true);
                if (layout.OFFSET0)  {
                    insn_in_progress->appendOperand(NamedImmediate::makeNamedImmediate(std::string("offset0"),Result(u8,layout.OFFSET0)),true,false,false);
                }
                if (layout.OFFSET1)  {
                    insn_in_progress->appendOperand(NamedImmediate::makeNamedImmediate(std::string("offset1"),Result(u8,layout.OFFSET1)),true,false,false);
                }
                break;
            case 92:   // DS_ADD_F64,
                appendOPR_VGPR(layout.ADDR,true,false);
                appendOPR_VGPR_OR_ACCVGPR(layout.DATA0,true,false,2);
                if (layout.OFFSET0 || layout.OFFSET1)  {
                    insn_in_progress->appendOperand(NamedImmediate::makeNamedImmediate(std::string("offset"),Result(u16,(layout.OFFSET1 << 8) + layout.OFFSET0)),true,false,false);
                }
                break;
            case 96:  case 97:  case 98:  case 99:   // DS_ADD_RTN_U64,DS_SUB_RTN_U64,DS_RSUB_RTN_U64,DS_INC_RTN_U64,
            case 100:  case 101:  case 102:  case 103:   // DS_DEC_RTN_U64,DS_MIN_RTN_I64,DS_MAX_RTN_I64,DS_MIN_RTN_U64,
            case 104:  case 105:  case 106:  case 107:   // DS_MAX_RTN_U64,DS_AND_RTN_B64,DS_OR_RTN_B64,DS_XOR_RTN_B64,
            case 109:  case 114:  case 115:  case 126:   // DS_WRXCHG_RTN_B64,DS_MIN_RTN_F64,DS_MAX_RTN_F64,DS_CONDXCHG32_RTN_B64,
                appendOPR_VGPR_OR_ACCVGPR(layout.VDST,false,true,2);
                appendOPR_VGPR(layout.ADDR,true,false);
                appendOPR_VGPR_OR_ACCVGPR(layout.DATA0,true,false,2);
                appendOPR_DSMEM(0,true,false,1,true);
                if (layout.OFFSET0 || layout.OFFSET1)  {
                    insn_in_progress->appendOperand(NamedImmediate::makeNamedImmediate(std::string("offset"),Result(u16,(layout.OFFSET1 << 8) + layout.OFFSET0)),true,false,false);
                }
                break;
            case 108:  case 112:  case 113:   // DS_MSKOR_RTN_B64,DS_CMPST_RTN_B64,DS_CMPST_RTN_F64,
                appendOPR_VGPR_OR_ACCVGPR(layout.VDST,false,true,2);
                appendOPR_VGPR(layout.ADDR,true,false);
                appendOPR_VGPR_OR_ACCVGPR(layout.DATA0,true,false,2);
                appendOPR_VGPR_OR_ACCVGPR(layout.DATA1,true,false,2);
                appendOPR_DSMEM(0,true,false,1,true);
                if (layout.OFFSET0 || layout.OFFSET1)  {
                    insn_in_progress->appendOperand(NamedImmediate::makeNamedImmediate(std::string("offset"),Result(u16,(layout.OFFSET1 << 8) + layout.OFFSET0)),true,false,false);
                }
                break;
            case 110:  case 111:   // DS_WRXCHG2_RTN_B64,DS_WRXCHG2ST64_RTN_B64,
                appendOPR_VGPR_OR_ACCVGPR(layout.VDST,false,true,4);
                appendOPR_VGPR(layout.ADDR,true,false);
                appendOPR_VGPR_OR_ACCVGPR(layout.DATA0,true,false,2);
                appendOPR_VGPR_OR_ACCVGPR(layout.DATA1,true,false,2);
                appendOPR_DSMEM(0,true,false,1,true);
                if (layout.OFFSET0 || layout.OFFSET1)  {
                    insn_in_progress->appendOperand(NamedImmediate::makeNamedImmediate(std::string("offset"),Result(u16,(layout.OFFSET1 << 8) + layout.OFFSET0)),true,false,false);
                }
                break;
            case 118:   // DS_READ_B64,
                appendOPR_VGPR_OR_ACCVGPR(layout.VDST,false,true,2);
                appendOPR_VGPR(layout.ADDR,true,false);
                appendOPR_DSMEM(0,true,false,1,true);
                if (layout.OFFSET0 || layout.OFFSET1)  {
                    insn_in_progress->appendOperand(NamedImmediate::makeNamedImmediate(std::string("offset"),Result(u16,(layout.OFFSET1 << 8) + layout.OFFSET0)),true,false,false);
                }
                break;
            case 119:  case 120:   // DS_READ2_B64,DS_READ2ST64_B64,
                appendOPR_VGPR_OR_ACCVGPR(layout.VDST,false,true,4);
                appendOPR_VGPR(layout.ADDR,true,false);
                appendOPR_DSMEM(0,true,false,1,true);
                if (layout.OFFSET0)  {
                    insn_in_progress->appendOperand(NamedImmediate::makeNamedImmediate(std::string("offset0"),Result(u8,layout.OFFSET0)),true,false,false);
                }
                if (layout.OFFSET1)  {
                    insn_in_progress->appendOperand(NamedImmediate::makeNamedImmediate(std::string("offset1"),Result(u8,layout.OFFSET1)),true,false,false);
                }
                break;
            case 124:   // DS_ADD_RTN_F64,
                appendOPR_VGPR_OR_ACCVGPR(layout.VDST,false,true,2);
                appendOPR_VGPR(layout.ADDR,true,false);
                appendOPR_VGPR_OR_ACCVGPR(layout.DATA0,true,false,2);
                if (layout.OFFSET0 || layout.OFFSET1)  {
                    insn_in_progress->appendOperand(NamedImmediate::makeNamedImmediate(std::string("offset"),Result(u16,(layout.OFFSET1 << 8) + layout.OFFSET0)),true,false,false);
                }
                break;
            case 152:  case 154:  case 156:   // DS_GWS_SEMA_RELEASE_ALL,DS_GWS_SEMA_V,DS_GWS_SEMA_P,
                appendOPR_SDST_M0(124,true,false,1,true);
                if (layout.OFFSET0 || layout.OFFSET1)  {
                    insn_in_progress->appendOperand(NamedImmediate::makeNamedImmediate(std::string("offset"),Result(u16,(layout.OFFSET1 << 8) + layout.OFFSET0)),true,false,false);
                }
                break;
            case 153:  case 155:  case 157:   // DS_GWS_INIT,DS_GWS_SEMA_BR,DS_GWS_BARRIER,
                appendOPR_VGPR_OR_ACCVGPR(layout.DATA0,true,false);
                appendOPR_SDST_M0(124,true,false,1,true);
                if (layout.OFFSET0 || layout.OFFSET1)  {
                    insn_in_progress->appendOperand(NamedImmediate::makeNamedImmediate(std::string("offset"),Result(u16,(layout.OFFSET1 << 8) + layout.OFFSET0)),true,false,false);
                }
                break;
            case 182:  case 189:  case 190:   // DS_READ_ADDTID_B32,DS_CONSUME,DS_APPEND,
                appendOPR_VGPR_OR_ACCVGPR(layout.VDST,false,true);
                appendOPR_DSMEM(0,true,false,1,true);
                appendOPR_SDST_M0(124,true,false,1,true);
                if (layout.OFFSET0 || layout.OFFSET1)  {
                    insn_in_progress->appendOperand(NamedImmediate::makeNamedImmediate(std::string("offset"),Result(u16,(layout.OFFSET1 << 8) + layout.OFFSET0)),true,false,false);
                }
                break;
            case 222:   // DS_WRITE_B96,
                appendOPR_VGPR(layout.ADDR,true,false);
                appendOPR_VGPR_OR_ACCVGPR(layout.DATA0,true,false,3);
                appendOPR_DSMEM(0,true,false,1,true);
                if (layout.OFFSET0 || layout.OFFSET1)  {
                    insn_in_progress->appendOperand(NamedImmediate::makeNamedImmediate(std::string("offset"),Result(u16,(layout.OFFSET1 << 8) + layout.OFFSET0)),true,false,false);
                }
                break;
            case 223:   // DS_WRITE_B128,
                appendOPR_VGPR(layout.ADDR,true,false);
                appendOPR_VGPR_OR_ACCVGPR(layout.DATA0,true,false,4);
                appendOPR_DSMEM(0,true,false,1,true);
                if (layout.OFFSET0 || layout.OFFSET1)  {
                    insn_in_progress->appendOperand(NamedImmediate::makeNamedImmediate(std::string("offset"),Result(u16,(layout.OFFSET1 << 8) + layout.OFFSET0)),true,false,false);
                }
                break;
            case 254:   // DS_READ_B96,
                appendOPR_VGPR_OR_ACCVGPR(layout.VDST,false,true,3);
                appendOPR_VGPR(layout.ADDR,true,false);
                appendOPR_DSMEM(0,true,false,1,true);
                if (layout.OFFSET0 || layout.OFFSET1)  {
                    insn_in_progress->appendOperand(NamedImmediate::makeNamedImmediate(std::string("offset"),Result(u16,(layout.OFFSET1 << 8) + layout.OFFSET0)),true,false,false);
                }
                break;
            case 255:   // DS_READ_B128,
                appendOPR_VGPR_OR_ACCVGPR(layout.VDST,false,true,4);
                appendOPR_VGPR(layout.ADDR,true,false);
                appendOPR_DSMEM(0,true,false,1,true);
                if (layout.OFFSET0 || layout.OFFSET1)  {
                    insn_in_progress->appendOperand(NamedImmediate::makeNamedImmediate(std::string("offset"),Result(u16,(layout.OFFSET1 << 8) + layout.OFFSET0)),true,false,false);
                }
                break;
        }
    }

    void InstructionDecoder_amdgpu_gfx90a::finalizeENC_FLATOperands()
    {
        layout_ENC_FLAT & layout = insn_layout.ENC_FLAT;
        switch (layout.OP)  {
            case 16:  case 17:  case 18:  case 19:   // FLAT_LOAD_UBYTE,FLAT_LOAD_SBYTE,FLAT_LOAD_USHORT,FLAT_LOAD_SSHORT,
            case 20:  case 32:  case 33:   // FLAT_LOAD_DWORD,FLAT_LOAD_UBYTE_D16,FLAT_LOAD_UBYTE_D16_HI,
            case 34:  case 35:  case 36:   // FLAT_LOAD_SBYTE_D16,FLAT_LOAD_SBYTE_D16_HI,FLAT_LOAD_SHORT_D16,
            case 37:   // FLAT_LOAD_SHORT_D16_HI,
                appendOPR_VGPR_OR_ACCVGPR(layout.VDST,false,true);
                appendOPR_VGPR(layout.ADDR,true,false,2);
                appendOPR_FLAT_SCRATCH(0,true,false,1,true);
                appendOPR_SDST_M0(124,true,false,1,true);
                insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx90a::vmcnt,0,36),true,true,true);
                insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx90a::lgkmcnt,0,16),true,true,true);
                break;
            case 21:   // FLAT_LOAD_DWORDX2,
                appendOPR_VGPR_OR_ACCVGPR(layout.VDST,false,true,2);
                appendOPR_VGPR(layout.ADDR,true,false,2);
                appendOPR_FLAT_SCRATCH(0,true,false,1,true);
                appendOPR_SDST_M0(124,true,false,1,true);
                insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx90a::vmcnt,0,36),true,true,true);
                insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx90a::lgkmcnt,0,16),true,true,true);
                break;
            case 22:   // FLAT_LOAD_DWORDX3,
                appendOPR_VGPR_OR_ACCVGPR(layout.VDST,false,true,3);
                appendOPR_VGPR(layout.ADDR,true,false,2);
                appendOPR_FLAT_SCRATCH(0,true,false,1,true);
                appendOPR_SDST_M0(124,true,false,1,true);
                insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx90a::vmcnt,0,36),true,true,true);
                insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx90a::lgkmcnt,0,16),true,true,true);
                break;
            case 23:   // FLAT_LOAD_DWORDX4,
                appendOPR_VGPR_OR_ACCVGPR(layout.VDST,false,true,4);
                appendOPR_VGPR(layout.ADDR,true,false,2);
                appendOPR_FLAT_SCRATCH(0,true,false,1,true);
                appendOPR_SDST_M0(124,true,false,1,true);
                insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx90a::vmcnt,0,36),true,true,true);
                insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx90a::lgkmcnt,0,16),true,true,true);
                break;
            case 24:  case 25:  case 26:   // FLAT_STORE_BYTE,FLAT_STORE_BYTE_D16_HI,FLAT_STORE_SHORT,
            case 27:  case 28:   // FLAT_STORE_SHORT_D16_HI,FLAT_STORE_DWORD,
                appendOPR_VGPR(layout.ADDR,true,false,2);
                appendOPR_VGPR_OR_ACCVGPR(layout.DATA,true,false);
                appendOPR_FLAT_SCRATCH(0,true,false,1,true);
                appendOPR_SDST_M0(124,true,false,1,true);
                insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx90a::vmcnt,0,36),true,true,true);
                insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx90a::lgkmcnt,0,16),true,true,true);
                break;
            case 29:   // FLAT_STORE_DWORDX2,
                appendOPR_VGPR(layout.ADDR,true,false,2);
                appendOPR_VGPR_OR_ACCVGPR(layout.DATA,true,false,2);
                appendOPR_FLAT_SCRATCH(0,true,false,1,true);
                appendOPR_SDST_M0(124,true,false,1,true);
                insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx90a::vmcnt,0,36),true,true,true);
                insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx90a::lgkmcnt,0,16),true,true,true);
                break;
            case 30:   // FLAT_STORE_DWORDX3,
                appendOPR_VGPR(layout.ADDR,true,false,2);
                appendOPR_VGPR_OR_ACCVGPR(layout.DATA,true,false,3);
                appendOPR_FLAT_SCRATCH(0,true,false,1,true);
                appendOPR_SDST_M0(124,true,false,1,true);
                insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx90a::vmcnt,0,36),true,true,true);
                insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx90a::lgkmcnt,0,16),true,true,true);
                break;
            case 31:   // FLAT_STORE_DWORDX4,
                appendOPR_VGPR(layout.ADDR,true,false,2);
                appendOPR_VGPR_OR_ACCVGPR(layout.DATA,true,false,4);
                appendOPR_FLAT_SCRATCH(0,true,false,1,true);
                appendOPR_SDST_M0(124,true,false,1,true);
                insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx90a::vmcnt,0,36),true,true,true);
                insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx90a::lgkmcnt,0,16),true,true,true);
                break;
            case 64:  case 66:  case 67:  case 68:   // FLAT_ATOMIC_SWAP,FLAT_ATOMIC_ADD,FLAT_ATOMIC_SUB,FLAT_ATOMIC_SMIN,
            case 69:  case 70:  case 71:  case 72:   // FLAT_ATOMIC_UMIN,FLAT_ATOMIC_SMAX,FLAT_ATOMIC_UMAX,FLAT_ATOMIC_AND,
            case 73:  case 74:  case 75:  case 76:   // FLAT_ATOMIC_OR,FLAT_ATOMIC_XOR,FLAT_ATOMIC_INC,FLAT_ATOMIC_DEC,
                appendOPR_VGPR_OR_ACCVGPR(layout.VDST,false,true);
                appendOPR_VGPR(layout.ADDR,true,false,2);
                appendOPR_VGPR_OR_ACCVGPR(layout.DATA,true,false);
                appendOPR_FLAT_SCRATCH(0,true,false,1,true);
                appendOPR_SDST_M0(124,true,false,1,true);
                insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx90a::vmcnt,0,36),true,true,true);
                insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx90a::lgkmcnt,0,16),true,true,true);
                break;
            case 65:  case 79:  case 80:   // FLAT_ATOMIC_CMPSWAP,FLAT_ATOMIC_ADD_F64,FLAT_ATOMIC_MIN_F64,
            case 81:  case 96:  case 98:  case 99:   // FLAT_ATOMIC_MAX_F64,FLAT_ATOMIC_SWAP_X2,FLAT_ATOMIC_ADD_X2,FLAT_ATOMIC_SUB_X2,
            case 100:  case 101:  case 102:   // FLAT_ATOMIC_SMIN_X2,FLAT_ATOMIC_UMIN_X2,FLAT_ATOMIC_SMAX_X2,
            case 103:  case 104:  case 105:   // FLAT_ATOMIC_UMAX_X2,FLAT_ATOMIC_AND_X2,FLAT_ATOMIC_OR_X2,
            case 106:  case 107:  case 108:   // FLAT_ATOMIC_XOR_X2,FLAT_ATOMIC_INC_X2,FLAT_ATOMIC_DEC_X2,
                appendOPR_VGPR_OR_ACCVGPR(layout.VDST,false,true,2);
                appendOPR_VGPR(layout.ADDR,true,false,2);
                appendOPR_VGPR_OR_ACCVGPR(layout.DATA,true,false,2);
                appendOPR_FLAT_SCRATCH(0,true,false,1,true);
                appendOPR_SDST_M0(124,true,false,1,true);
                insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx90a::vmcnt,0,36),true,true,true);
                insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx90a::lgkmcnt,0,16),true,true,true);
                break;
            case 97:   // FLAT_ATOMIC_CMPSWAP_X2,
                appendOPR_VGPR_OR_ACCVGPR(layout.VDST,false,true,4);
                appendOPR_VGPR(layout.ADDR,true,false,2);
                appendOPR_VGPR_OR_ACCVGPR(layout.DATA,true,false,4);
                appendOPR_FLAT_SCRATCH(0,true,false,1,true);
                appendOPR_SDST_M0(124,true,false,1,true);
                insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx90a::vmcnt,0,36),true,true,true);
                insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx90a::lgkmcnt,0,16),true,true,true);
                break;
        }
    }

    void InstructionDecoder_amdgpu_gfx90a::finalizeENC_FLAT_GLBLOperands()
    {
        layout_ENC_FLAT_GLBL & layout = insn_layout.ENC_FLAT_GLBL;
        switch (layout.OP)  {
            case 16:  case 17:  case 18:  case 19:   // GLOBAL_LOAD_UBYTE,GLOBAL_LOAD_SBYTE,GLOBAL_LOAD_USHORT,GLOBAL_LOAD_SSHORT,
            case 20:  case 32:  case 33:   // GLOBAL_LOAD_DWORD,GLOBAL_LOAD_UBYTE_D16,GLOBAL_LOAD_UBYTE_D16_HI,
            case 34:  case 35:  case 36:   // GLOBAL_LOAD_SBYTE_D16,GLOBAL_LOAD_SBYTE_D16_HI,GLOBAL_LOAD_SHORT_D16,
            case 37:   // GLOBAL_LOAD_SHORT_D16_HI,
                appendOPR_VGPR_OR_ACCVGPR(layout.VDST,false,true);
                if (layout.SADDR == 0x7f)  {
                    appendOPR_VGPR(layout.ADDR,true,false,2);
                } else {
                    appendOPR_VGPR(layout.ADDR,true,false);
                }
                if (layout.SADDR != 0x7f)  {
                    appendOPR_SREG(layout.SADDR,true,false,2);
                }
                appendOPR_SDST_M0(124,true,false,1,true);
                insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx90a::vmcnt,0,36),true,true,true);
                insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx90a::lgkmcnt,0,16),true,true,true);
                break;
            case 21:   // GLOBAL_LOAD_DWORDX2,
                appendOPR_VGPR_OR_ACCVGPR(layout.VDST,false,true,2);
                if (layout.SADDR == 0x7f)  {
                    appendOPR_VGPR(layout.ADDR,true,false,2);
                } else {
                    appendOPR_VGPR(layout.ADDR,true,false);
                }
                if (layout.SADDR != 0x7f)  {
                    appendOPR_SREG(layout.SADDR,true,false,2);
                }
                appendOPR_SDST_M0(124,true,false,1,true);
                insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx90a::vmcnt,0,36),true,true,true);
                insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx90a::lgkmcnt,0,16),true,true,true);
                break;
            case 22:   // GLOBAL_LOAD_DWORDX3,
                appendOPR_VGPR_OR_ACCVGPR(layout.VDST,false,true,3);
                if (layout.SADDR == 0x7f)  {
                    appendOPR_VGPR(layout.ADDR,true,false,2);
                } else {
                    appendOPR_VGPR(layout.ADDR,true,false);
                }
                if (layout.SADDR != 0x7f)  {
                    appendOPR_SREG(layout.SADDR,true,false,2);
                }
                appendOPR_SDST_M0(124,true,false,1,true);
                insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx90a::vmcnt,0,36),true,true,true);
                insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx90a::lgkmcnt,0,16),true,true,true);
                break;
            case 23:   // GLOBAL_LOAD_DWORDX4,
                appendOPR_VGPR_OR_ACCVGPR(layout.VDST,false,true,4);
                if (layout.SADDR == 0x7f)  {
                    appendOPR_VGPR(layout.ADDR,true,false,2);
                } else {
                    appendOPR_VGPR(layout.ADDR,true,false);
                }
                if (layout.SADDR != 0x7f)  {
                    appendOPR_SREG(layout.SADDR,true,false,2);
                }
                appendOPR_SDST_M0(124,true,false,1,true);
                insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx90a::vmcnt,0,36),true,true,true);
                insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx90a::lgkmcnt,0,16),true,true,true);
                break;
            case 24:  case 25:  case 26:   // GLOBAL_STORE_BYTE,GLOBAL_STORE_BYTE_D16_HI,GLOBAL_STORE_SHORT,
            case 27:  case 28:   // GLOBAL_STORE_SHORT_D16_HI,GLOBAL_STORE_DWORD,
                if (layout.SADDR == 0x7f)  {
                    appendOPR_VGPR(layout.ADDR,true,false,2);
                } else {
                    appendOPR_VGPR(layout.ADDR,true,false);
                }
                appendOPR_VGPR_OR_ACCVGPR(layout.DATA,true,false);
                if (layout.SADDR != 0x7f)  {
                    appendOPR_SREG(layout.SADDR,true,false,2);
                }
                appendOPR_SDST_M0(124,true,false,1,true);
                insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx90a::vmcnt,0,36),true,true,true);
                insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx90a::lgkmcnt,0,16),true,true,true);
                break;
            case 29:   // GLOBAL_STORE_DWORDX2,
                if (layout.SADDR == 0x7f)  {
                    appendOPR_VGPR(layout.ADDR,true,false,2);
                } else {
                    appendOPR_VGPR(layout.ADDR,true,false);
                }
                appendOPR_VGPR_OR_ACCVGPR(layout.DATA,true,false,2);
                if (layout.SADDR != 0x7f)  {
                    appendOPR_SREG(layout.SADDR,true,false,2);
                }
                appendOPR_SDST_M0(124,true,false,1,true);
                insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx90a::vmcnt,0,36),true,true,true);
                insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx90a::lgkmcnt,0,16),true,true,true);
                break;
            case 30:   // GLOBAL_STORE_DWORDX3,
                if (layout.SADDR == 0x7f)  {
                    appendOPR_VGPR(layout.ADDR,true,false,2);
                } else {
                    appendOPR_VGPR(layout.ADDR,true,false);
                }
                appendOPR_VGPR_OR_ACCVGPR(layout.DATA,true,false,3);
                if (layout.SADDR != 0x7f)  {
                    appendOPR_SREG(layout.SADDR,true,false,2);
                }
                appendOPR_SDST_M0(124,true,false,1,true);
                insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx90a::vmcnt,0,36),true,true,true);
                insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx90a::lgkmcnt,0,16),true,true,true);
                break;
            case 31:   // GLOBAL_STORE_DWORDX4,
                if (layout.SADDR == 0x7f)  {
                    appendOPR_VGPR(layout.ADDR,true,false,2);
                } else {
                    appendOPR_VGPR(layout.ADDR,true,false);
                }
                appendOPR_VGPR_OR_ACCVGPR(layout.DATA,true,false,4);
                if (layout.SADDR != 0x7f)  {
                    appendOPR_SREG(layout.SADDR,true,false,2);
                }
                appendOPR_SDST_M0(124,true,false,1,true);
                insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx90a::vmcnt,0,36),true,true,true);
                insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx90a::lgkmcnt,0,16),true,true,true);
                break;
            case 64:  case 66:  case 67:  case 68:   // GLOBAL_ATOMIC_SWAP,GLOBAL_ATOMIC_ADD,GLOBAL_ATOMIC_SUB,GLOBAL_ATOMIC_SMIN,
            case 69:  case 70:  case 71:  case 72:   // GLOBAL_ATOMIC_UMIN,GLOBAL_ATOMIC_SMAX,GLOBAL_ATOMIC_UMAX,GLOBAL_ATOMIC_AND,
            case 73:  case 74:  case 75:  case 76:   // GLOBAL_ATOMIC_OR,GLOBAL_ATOMIC_XOR,GLOBAL_ATOMIC_INC,GLOBAL_ATOMIC_DEC,
            case 77:  case 78:   // GLOBAL_ATOMIC_ADD_F32,GLOBAL_ATOMIC_PK_ADD_F16,
                appendOPR_VGPR_OR_ACCVGPR(layout.VDST,false,true);
                if (layout.SADDR == 0x7f)  {
                    appendOPR_VGPR(layout.ADDR,true,false,2);
                } else {
                    appendOPR_VGPR(layout.ADDR,true,false);
                }
                appendOPR_VGPR_OR_ACCVGPR(layout.DATA,true,false);
                if (layout.SADDR != 0x7f)  {
                    appendOPR_SREG(layout.SADDR,true,false,2);
                }
                appendOPR_SDST_M0(124,true,false,1,true);
                insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx90a::vmcnt,0,36),true,true,true);
                insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx90a::lgkmcnt,0,16),true,true,true);
                break;
            case 65:  case 79:  case 80:   // GLOBAL_ATOMIC_CMPSWAP,GLOBAL_ATOMIC_ADD_F64,GLOBAL_ATOMIC_MIN_F64,
            case 81:  case 96:  case 98:   // GLOBAL_ATOMIC_MAX_F64,GLOBAL_ATOMIC_SWAP_X2,GLOBAL_ATOMIC_ADD_X2,
            case 99:  case 100:  case 101:   // GLOBAL_ATOMIC_SUB_X2,GLOBAL_ATOMIC_SMIN_X2,GLOBAL_ATOMIC_UMIN_X2,
            case 102:  case 103:  case 104:   // GLOBAL_ATOMIC_SMAX_X2,GLOBAL_ATOMIC_UMAX_X2,GLOBAL_ATOMIC_AND_X2,
            case 105:  case 106:  case 107:   // GLOBAL_ATOMIC_OR_X2,GLOBAL_ATOMIC_XOR_X2,GLOBAL_ATOMIC_INC_X2,
            case 108:   // GLOBAL_ATOMIC_DEC_X2,
                appendOPR_VGPR_OR_ACCVGPR(layout.VDST,false,true,2);
                if (layout.SADDR == 0x7f)  {
                    appendOPR_VGPR(layout.ADDR,true,false,2);
                } else {
                    appendOPR_VGPR(layout.ADDR,true,false);
                }
                appendOPR_VGPR_OR_ACCVGPR(layout.DATA,true,false,2);
                if (layout.SADDR != 0x7f)  {
                    appendOPR_SREG(layout.SADDR,true,false,2);
                }
                appendOPR_SDST_M0(124,true,false,1,true);
                insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx90a::vmcnt,0,36),true,true,true);
                insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx90a::lgkmcnt,0,16),true,true,true);
                break;
            case 97:   // GLOBAL_ATOMIC_CMPSWAP_X2,
                appendOPR_VGPR_OR_ACCVGPR(layout.VDST,false,true,4);
                if (layout.SADDR == 0x7f)  {
                    appendOPR_VGPR(layout.ADDR,true,false,2);
                } else {
                    appendOPR_VGPR(layout.ADDR,true,false);
                }
                appendOPR_VGPR_OR_ACCVGPR(layout.DATA,true,false,4);
                if (layout.SADDR != 0x7f)  {
                    appendOPR_SREG(layout.SADDR,true,false,2);
                }
                appendOPR_SDST_M0(124,true,false,1,true);
                insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx90a::vmcnt,0,36),true,true,true);
                insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx90a::lgkmcnt,0,16),true,true,true);
                break;
        }
    }

    void InstructionDecoder_amdgpu_gfx90a::finalizeENC_FLAT_SCRATCHOperands()
    {
        layout_ENC_FLAT_SCRATCH & layout = insn_layout.ENC_FLAT_SCRATCH;
        switch (layout.OP)  {
            case 16:  case 17:  case 18:  case 19:   // SCRATCH_LOAD_UBYTE,SCRATCH_LOAD_SBYTE,SCRATCH_LOAD_USHORT,SCRATCH_LOAD_SSHORT,
            case 20:  case 32:  case 33:   // SCRATCH_LOAD_DWORD,SCRATCH_LOAD_UBYTE_D16,SCRATCH_LOAD_UBYTE_D16_HI,
            case 34:  case 35:  case 36:   // SCRATCH_LOAD_SBYTE_D16,SCRATCH_LOAD_SBYTE_D16_HI,SCRATCH_LOAD_SHORT_D16,
            case 37:   // SCRATCH_LOAD_SHORT_D16_HI,
                appendOPR_VGPR_OR_ACCVGPR(layout.VDST,false,true);
                if (layout.SADDR == 0x7f)  {
                    appendOPR_VGPR(layout.ADDR,true,false,2);
                } else {
                    appendOPR_VGPR(layout.ADDR,true,false);
                }
                if (layout.SADDR != 0x7f)  {
                    appendOPR_SREG(layout.SADDR,true,false,2);
                }
                appendOPR_FLAT_SCRATCH(0,true,false,1,true);
                appendOPR_SDST_M0(124,true,false,1,true);
                insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx90a::vmcnt,0,36),true,true,true);
                insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx90a::lgkmcnt,0,16),true,true,true);
                break;
            case 21:   // SCRATCH_LOAD_DWORDX2,
                appendOPR_VGPR_OR_ACCVGPR(layout.VDST,false,true,2);
                if (layout.SADDR == 0x7f)  {
                    appendOPR_VGPR(layout.ADDR,true,false,2);
                } else {
                    appendOPR_VGPR(layout.ADDR,true,false);
                }
                if (layout.SADDR != 0x7f)  {
                    appendOPR_SREG(layout.SADDR,true,false,2);
                }
                appendOPR_FLAT_SCRATCH(0,true,false,1,true);
                appendOPR_SDST_M0(124,true,false,1,true);
                insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx90a::vmcnt,0,36),true,true,true);
                insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx90a::lgkmcnt,0,16),true,true,true);
                break;
            case 22:   // SCRATCH_LOAD_DWORDX3,
                appendOPR_VGPR_OR_ACCVGPR(layout.VDST,false,true,3);
                if (layout.SADDR == 0x7f)  {
                    appendOPR_VGPR(layout.ADDR,true,false,2);
                } else {
                    appendOPR_VGPR(layout.ADDR,true,false);
                }
                if (layout.SADDR != 0x7f)  {
                    appendOPR_SREG(layout.SADDR,true,false,2);
                }
                appendOPR_FLAT_SCRATCH(0,true,false,1,true);
                appendOPR_SDST_M0(124,true,false,1,true);
                insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx90a::vmcnt,0,36),true,true,true);
                insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx90a::lgkmcnt,0,16),true,true,true);
                break;
            case 23:   // SCRATCH_LOAD_DWORDX4,
                appendOPR_VGPR_OR_ACCVGPR(layout.VDST,false,true,4);
                if (layout.SADDR == 0x7f)  {
                    appendOPR_VGPR(layout.ADDR,true,false,2);
                } else {
                    appendOPR_VGPR(layout.ADDR,true,false);
                }
                if (layout.SADDR != 0x7f)  {
                    appendOPR_SREG(layout.SADDR,true,false,2);
                }
                appendOPR_FLAT_SCRATCH(0,true,false,1,true);
                appendOPR_SDST_M0(124,true,false,1,true);
                insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx90a::vmcnt,0,36),true,true,true);
                insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx90a::lgkmcnt,0,16),true,true,true);
                break;
            case 24:  case 25:  case 26:   // SCRATCH_STORE_BYTE,SCRATCH_STORE_BYTE_D16_HI,SCRATCH_STORE_SHORT,
            case 27:  case 28:   // SCRATCH_STORE_SHORT_D16_HI,SCRATCH_STORE_DWORD,
                if (layout.SADDR == 0x7f)  {
                    appendOPR_VGPR(layout.ADDR,true,false,2);
                } else {
                    appendOPR_VGPR(layout.ADDR,true,false);
                }
                appendOPR_VGPR_OR_ACCVGPR(layout.DATA,true,false);
                if (layout.SADDR != 0x7f)  {
                    appendOPR_SREG(layout.SADDR,true,false,2);
                }
                appendOPR_FLAT_SCRATCH(0,true,false,1,true);
                appendOPR_SDST_M0(124,true,false,1,true);
                insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx90a::vmcnt,0,36),true,true,true);
                insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx90a::lgkmcnt,0,16),true,true,true);
                break;
            case 29:   // SCRATCH_STORE_DWORDX2,
                if (layout.SADDR == 0x7f)  {
                    appendOPR_VGPR(layout.ADDR,true,false,2);
                } else {
                    appendOPR_VGPR(layout.ADDR,true,false);
                }
                appendOPR_VGPR_OR_ACCVGPR(layout.DATA,true,false,2);
                if (layout.SADDR != 0x7f)  {
                    appendOPR_SREG(layout.SADDR,true,false,2);
                }
                appendOPR_FLAT_SCRATCH(0,true,false,1,true);
                appendOPR_SDST_M0(124,true,false,1,true);
                insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx90a::vmcnt,0,36),true,true,true);
                insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx90a::lgkmcnt,0,16),true,true,true);
                break;
            case 30:   // SCRATCH_STORE_DWORDX3,
                if (layout.SADDR == 0x7f)  {
                    appendOPR_VGPR(layout.ADDR,true,false,2);
                } else {
                    appendOPR_VGPR(layout.ADDR,true,false);
                }
                appendOPR_VGPR_OR_ACCVGPR(layout.DATA,true,false,3);
                if (layout.SADDR != 0x7f)  {
                    appendOPR_SREG(layout.SADDR,true,false,2);
                }
                appendOPR_FLAT_SCRATCH(0,true,false,1,true);
                appendOPR_SDST_M0(124,true,false,1,true);
                insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx90a::vmcnt,0,36),true,true,true);
                insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx90a::lgkmcnt,0,16),true,true,true);
                break;
            case 31:   // SCRATCH_STORE_DWORDX4,
                if (layout.SADDR == 0x7f)  {
                    appendOPR_VGPR(layout.ADDR,true,false,2);
                } else {
                    appendOPR_VGPR(layout.ADDR,true,false);
                }
                appendOPR_VGPR_OR_ACCVGPR(layout.DATA,true,false,4);
                if (layout.SADDR != 0x7f)  {
                    appendOPR_SREG(layout.SADDR,true,false,2);
                }
                appendOPR_FLAT_SCRATCH(0,true,false,1,true);
                appendOPR_SDST_M0(124,true,false,1,true);
                insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx90a::vmcnt,0,36),true,true,true);
                insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx90a::lgkmcnt,0,16),true,true,true);
                break;
        }
    }

    void InstructionDecoder_amdgpu_gfx90a::finalizeENC_MIMGOperands()
    {
        layout_ENC_MIMG & layout = insn_layout.ENC_MIMG;
        switch (layout.OP)  {
            case 0:  case 1:  case 2:  case 3:   // IMAGE_LOAD,IMAGE_LOAD_MIP,IMAGE_LOAD_PCK,IMAGE_LOAD_PCK_SGN,
            case 4:  case 5:   // IMAGE_LOAD_MIP_PCK,IMAGE_LOAD_MIP_PCK_SGN,
                appendOPR_VGPR_OR_ACCVGPR(layout.VDATA,false,true,4);
                appendOPR_VGPR(layout.VADDR,true,false,4);
                appendOPR_SREG(layout.SRSRC,true,false,8);
                insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx90a::vmcnt,0,36),true,true,true);
                break;
            case 8:  case 9:  case 10:  case 11:   // IMAGE_STORE,IMAGE_STORE_MIP,IMAGE_STORE_PCK,IMAGE_STORE_MIP_PCK,
                appendOPR_VGPR_OR_ACCVGPR(layout.VDATA,true,false,4);
                appendOPR_VGPR(layout.VADDR,true,false,4);
                appendOPR_SREG(layout.SRSRC,true,false,8);
                insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx90a::vmcnt,0,36),true,true,true);
                break;
            case 14:   // IMAGE_GET_RESINFO,
                appendOPR_VGPR_OR_ACCVGPR(layout.VDATA,false,true,4);
                appendOPR_VGPR(layout.VADDR,true,false);
                appendOPR_SREG(layout.SRSRC,true,false,8);
                insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx90a::vmcnt,0,36),true,true,true);
                break;
            case 16:  case 17:  case 18:  case 19:   // IMAGE_ATOMIC_SWAP,IMAGE_ATOMIC_CMPSWAP,IMAGE_ATOMIC_ADD,IMAGE_ATOMIC_SUB,
            case 20:  case 21:  case 22:  case 23:   // IMAGE_ATOMIC_SMIN,IMAGE_ATOMIC_UMIN,IMAGE_ATOMIC_SMAX,IMAGE_ATOMIC_UMAX,
            case 24:  case 25:  case 26:  case 27:   // IMAGE_ATOMIC_AND,IMAGE_ATOMIC_OR,IMAGE_ATOMIC_XOR,IMAGE_ATOMIC_INC,
            case 28:   // IMAGE_ATOMIC_DEC,
                appendOPR_VGPR_OR_ACCVGPR(layout.VDATA,true,true,4);
                appendOPR_VGPR(layout.VADDR,true,false,4);
                appendOPR_SREG(layout.SRSRC,true,false,8);
                insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx90a::vmcnt,0,36),true,true,true);
                break;
            case 32:   // IMAGE_SAMPLE,
                appendOPR_VGPR_OR_ACCVGPR(layout.VDATA,false,true,4);
                appendOPR_VGPR(layout.VADDR,true,false,3);
                appendOPR_SREG(layout.SRSRC,true,false,8);
                appendOPR_SREG(layout.SSAMP,true,false,4);
                insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx90a::vmcnt,0,36),true,true,true);
                break;
        }
    }

    void InstructionDecoder_amdgpu_gfx90a::finalizeENC_MTBUFOperands()
    {
        layout_ENC_MTBUF & layout = insn_layout.ENC_MTBUF;
        switch (layout.OP)  {
            case 0:  case 8:  case 9:   // TBUFFER_LOAD_FORMAT_X,TBUFFER_LOAD_FORMAT_D16_X,TBUFFER_LOAD_FORMAT_D16_XY,
                appendOPR_VGPR_OR_ACCVGPR(layout.VDATA,false,true);
                appendOPR_VGPR(layout.VADDR,true,false,2);
                appendOPR_SREG(layout.SRSRC,true,false,4);
                appendOPR_SSRC_NOLIT(layout.SOFFSET,true,false);
                insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx90a::vmcnt,0,36),true,true,true);
                break;
            case 1:  case 10:  case 11:   // TBUFFER_LOAD_FORMAT_XY,TBUFFER_LOAD_FORMAT_D16_XYZ,TBUFFER_LOAD_FORMAT_D16_XYZW,
                appendOPR_VGPR_OR_ACCVGPR(layout.VDATA,false,true,2);
                appendOPR_VGPR(layout.VADDR,true,false,2);
                appendOPR_SREG(layout.SRSRC,true,false,4);
                appendOPR_SSRC_NOLIT(layout.SOFFSET,true,false);
                insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx90a::vmcnt,0,36),true,true,true);
                break;
            case 2:   // TBUFFER_LOAD_FORMAT_XYZ,
                appendOPR_VGPR_OR_ACCVGPR(layout.VDATA,false,true,3);
                appendOPR_VGPR(layout.VADDR,true,false,2);
                appendOPR_SREG(layout.SRSRC,true,false,4);
                appendOPR_SSRC_NOLIT(layout.SOFFSET,true,false);
                insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx90a::vmcnt,0,36),true,true,true);
                break;
            case 3:   // TBUFFER_LOAD_FORMAT_XYZW,
                appendOPR_VGPR_OR_ACCVGPR(layout.VDATA,false,true,4);
                appendOPR_VGPR(layout.VADDR,true,false,2);
                appendOPR_SREG(layout.SRSRC,true,false,4);
                appendOPR_SSRC_NOLIT(layout.SOFFSET,true,false);
                insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx90a::vmcnt,0,36),true,true,true);
                break;
            case 4:  case 12:  case 13:   // TBUFFER_STORE_FORMAT_X,TBUFFER_STORE_FORMAT_D16_X,TBUFFER_STORE_FORMAT_D16_XY,
                appendOPR_VGPR_OR_ACCVGPR(layout.VDATA,true,false);
                appendOPR_VGPR(layout.VADDR,true,false,2);
                appendOPR_SREG(layout.SRSRC,true,false,4);
                appendOPR_SSRC_NOLIT(layout.SOFFSET,true,false);
                insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx90a::vmcnt,0,36),true,true,true);
                break;
            case 5:  case 14:  case 15:   // TBUFFER_STORE_FORMAT_XY,TBUFFER_STORE_FORMAT_D16_XYZ,TBUFFER_STORE_FORMAT_D16_XYZW,
                appendOPR_VGPR_OR_ACCVGPR(layout.VDATA,true,false,2);
                appendOPR_VGPR(layout.VADDR,true,false,2);
                appendOPR_SREG(layout.SRSRC,true,false,4);
                appendOPR_SSRC_NOLIT(layout.SOFFSET,true,false);
                insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx90a::vmcnt,0,36),true,true,true);
                break;
            case 6:   // TBUFFER_STORE_FORMAT_XYZ,
                appendOPR_VGPR_OR_ACCVGPR(layout.VDATA,true,false,3);
                appendOPR_VGPR(layout.VADDR,true,false,2);
                appendOPR_SREG(layout.SRSRC,true,false,4);
                appendOPR_SSRC_NOLIT(layout.SOFFSET,true,false);
                insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx90a::vmcnt,0,36),true,true,true);
                break;
            case 7:   // TBUFFER_STORE_FORMAT_XYZW,
                appendOPR_VGPR_OR_ACCVGPR(layout.VDATA,true,false,4);
                appendOPR_VGPR(layout.VADDR,true,false,2);
                appendOPR_SREG(layout.SRSRC,true,false,4);
                appendOPR_SSRC_NOLIT(layout.SOFFSET,true,false);
                insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx90a::vmcnt,0,36),true,true,true);
                break;
        }
    }

    void InstructionDecoder_amdgpu_gfx90a::finalizeENC_MUBUFOperands()
    {
        layout_ENC_MUBUF & layout = insn_layout.ENC_MUBUF;
        switch (layout.OP)  {
            case 0:  case 8:  case 9:   // BUFFER_LOAD_FORMAT_X,BUFFER_LOAD_FORMAT_D16_X,BUFFER_LOAD_FORMAT_D16_XY,
            case 16:  case 17:  case 18:  case 19:   // BUFFER_LOAD_UBYTE,BUFFER_LOAD_SBYTE,BUFFER_LOAD_USHORT,BUFFER_LOAD_SSHORT,
            case 20:  case 32:  case 33:   // BUFFER_LOAD_DWORD,BUFFER_LOAD_UBYTE_D16,BUFFER_LOAD_UBYTE_D16_HI,
            case 34:  case 35:  case 36:   // BUFFER_LOAD_SBYTE_D16,BUFFER_LOAD_SBYTE_D16_HI,BUFFER_LOAD_SHORT_D16,
            case 37:  case 38:   // BUFFER_LOAD_SHORT_D16_HI,BUFFER_LOAD_FORMAT_D16_HI_X,
                appendOPR_VGPR_OR_ACCVGPR(layout.VDATA,false,true);
                if (layout.IDXEN || layout.OFFEN)  {
                    appendOPR_VGPR(layout.VADDR,true,false);
                } else if (layout.OFFSET)  {
                    insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx90a::off,1),false,false,false);
                }
                if (layout.IDXEN && layout.OFFEN)  {
                    appendOPR_VGPR(layout.VADDR+1,true,false);
                }
                appendOPR_SREG(layout.SRSRC,true,false,4);
                appendOPR_SSRC_NOLIT(layout.SOFFSET,true,false);
                insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx90a::vmcnt,0,36),true,true,true);
                if (layout.IDXEN)  {
                    insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx90a::idxen,1),false,false,false);
                }
                if (layout.OFFEN)  {
                    insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx90a::offen,1),false,false,false);
                }
                if (layout.OFFSET)  {
                    insn_in_progress->appendOperand(NamedImmediate::makeNamedImmediate(std::string("offset"),Result(u16,layout.OFFSET)),true,false,false);
                }
                break;
            case 1:  case 10:  case 11:   // BUFFER_LOAD_FORMAT_XY,BUFFER_LOAD_FORMAT_D16_XYZ,BUFFER_LOAD_FORMAT_D16_XYZW,
            case 21:   // BUFFER_LOAD_DWORDX2,
                appendOPR_VGPR_OR_ACCVGPR(layout.VDATA,false,true,2);
                if (layout.IDXEN || layout.OFFEN)  {
                    appendOPR_VGPR(layout.VADDR,true,false);
                } else if (layout.OFFSET)  {
                    insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx90a::off,1),false,false,false);
                }
                if (layout.IDXEN && layout.OFFEN)  {
                    appendOPR_VGPR(layout.VADDR+1,true,false);
                }
                appendOPR_SREG(layout.SRSRC,true,false,4);
                appendOPR_SSRC_NOLIT(layout.SOFFSET,true,false);
                insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx90a::vmcnt,0,36),true,true,true);
                if (layout.IDXEN)  {
                    insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx90a::idxen,1),false,false,false);
                }
                if (layout.OFFEN)  {
                    insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx90a::offen,1),false,false,false);
                }
                if (layout.OFFSET)  {
                    insn_in_progress->appendOperand(NamedImmediate::makeNamedImmediate(std::string("offset"),Result(u16,layout.OFFSET)),true,false,false);
                }
                break;
            case 2:  case 22:   // BUFFER_LOAD_FORMAT_XYZ,BUFFER_LOAD_DWORDX3,
                appendOPR_VGPR_OR_ACCVGPR(layout.VDATA,false,true,3);
                if (layout.IDXEN || layout.OFFEN)  {
                    appendOPR_VGPR(layout.VADDR,true,false);
                } else if (layout.OFFSET)  {
                    insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx90a::off,1),false,false,false);
                }
                if (layout.IDXEN && layout.OFFEN)  {
                    appendOPR_VGPR(layout.VADDR+1,true,false);
                }
                appendOPR_SREG(layout.SRSRC,true,false,4);
                appendOPR_SSRC_NOLIT(layout.SOFFSET,true,false);
                insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx90a::vmcnt,0,36),true,true,true);
                if (layout.IDXEN)  {
                    insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx90a::idxen,1),false,false,false);
                }
                if (layout.OFFEN)  {
                    insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx90a::offen,1),false,false,false);
                }
                if (layout.OFFSET)  {
                    insn_in_progress->appendOperand(NamedImmediate::makeNamedImmediate(std::string("offset"),Result(u16,layout.OFFSET)),true,false,false);
                }
                break;
            case 3:  case 23:   // BUFFER_LOAD_FORMAT_XYZW,BUFFER_LOAD_DWORDX4,
                appendOPR_VGPR_OR_ACCVGPR(layout.VDATA,false,true,4);
                if (layout.IDXEN || layout.OFFEN)  {
                    appendOPR_VGPR(layout.VADDR,true,false);
                } else if (layout.OFFSET)  {
                    insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx90a::off,1),false,false,false);
                }
                if (layout.IDXEN && layout.OFFEN)  {
                    appendOPR_VGPR(layout.VADDR+1,true,false);
                }
                appendOPR_SREG(layout.SRSRC,true,false,4);
                appendOPR_SSRC_NOLIT(layout.SOFFSET,true,false);
                insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx90a::vmcnt,0,36),true,true,true);
                if (layout.IDXEN)  {
                    insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx90a::idxen,1),false,false,false);
                }
                if (layout.OFFEN)  {
                    insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx90a::offen,1),false,false,false);
                }
                if (layout.OFFSET)  {
                    insn_in_progress->appendOperand(NamedImmediate::makeNamedImmediate(std::string("offset"),Result(u16,layout.OFFSET)),true,false,false);
                }
                break;
            case 4:  case 12:  case 13:   // BUFFER_STORE_FORMAT_X,BUFFER_STORE_FORMAT_D16_X,BUFFER_STORE_FORMAT_D16_XY,
            case 24:  case 25:  case 26:   // BUFFER_STORE_BYTE,BUFFER_STORE_BYTE_D16_HI,BUFFER_STORE_SHORT,
            case 27:  case 28:  case 39:   // BUFFER_STORE_SHORT_D16_HI,BUFFER_STORE_DWORD,BUFFER_STORE_FORMAT_D16_HI_X,
                appendOPR_VGPR_OR_ACCVGPR(layout.VDATA,true,false);
                if (layout.IDXEN || layout.OFFEN)  {
                    appendOPR_VGPR(layout.VADDR,true,false);
                } else if (layout.OFFSET)  {
                    insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx90a::off,1),false,false,false);
                }
                if (layout.IDXEN && layout.OFFEN)  {
                    appendOPR_VGPR(layout.VADDR+1,true,false);
                }
                appendOPR_SREG(layout.SRSRC,true,false,4);
                appendOPR_SSRC_NOLIT(layout.SOFFSET,true,false);
                insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx90a::vmcnt,0,36),true,true,true);
                if (layout.IDXEN)  {
                    insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx90a::idxen,1),false,false,false);
                }
                if (layout.OFFEN)  {
                    insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx90a::offen,1),false,false,false);
                }
                if (layout.OFFSET)  {
                    insn_in_progress->appendOperand(NamedImmediate::makeNamedImmediate(std::string("offset"),Result(u16,layout.OFFSET)),true,false,false);
                }
                break;
            case 5:  case 14:  case 15:   // BUFFER_STORE_FORMAT_XY,BUFFER_STORE_FORMAT_D16_XYZ,BUFFER_STORE_FORMAT_D16_XYZW,
            case 29:   // BUFFER_STORE_DWORDX2,
                appendOPR_VGPR_OR_ACCVGPR(layout.VDATA,true,false,2);
                if (layout.IDXEN || layout.OFFEN)  {
                    appendOPR_VGPR(layout.VADDR,true,false);
                } else if (layout.OFFSET)  {
                    insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx90a::off,1),false,false,false);
                }
                if (layout.IDXEN && layout.OFFEN)  {
                    appendOPR_VGPR(layout.VADDR+1,true,false);
                }
                appendOPR_SREG(layout.SRSRC,true,false,4);
                appendOPR_SSRC_NOLIT(layout.SOFFSET,true,false);
                insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx90a::vmcnt,0,36),true,true,true);
                if (layout.IDXEN)  {
                    insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx90a::idxen,1),false,false,false);
                }
                if (layout.OFFEN)  {
                    insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx90a::offen,1),false,false,false);
                }
                if (layout.OFFSET)  {
                    insn_in_progress->appendOperand(NamedImmediate::makeNamedImmediate(std::string("offset"),Result(u16,layout.OFFSET)),true,false,false);
                }
                break;
            case 6:  case 30:   // BUFFER_STORE_FORMAT_XYZ,BUFFER_STORE_DWORDX3,
                appendOPR_VGPR_OR_ACCVGPR(layout.VDATA,true,false,3);
                if (layout.IDXEN || layout.OFFEN)  {
                    appendOPR_VGPR(layout.VADDR,true,false);
                } else if (layout.OFFSET)  {
                    insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx90a::off,1),false,false,false);
                }
                if (layout.IDXEN && layout.OFFEN)  {
                    appendOPR_VGPR(layout.VADDR+1,true,false);
                }
                appendOPR_SREG(layout.SRSRC,true,false,4);
                appendOPR_SSRC_NOLIT(layout.SOFFSET,true,false);
                insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx90a::vmcnt,0,36),true,true,true);
                if (layout.IDXEN)  {
                    insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx90a::idxen,1),false,false,false);
                }
                if (layout.OFFEN)  {
                    insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx90a::offen,1),false,false,false);
                }
                if (layout.OFFSET)  {
                    insn_in_progress->appendOperand(NamedImmediate::makeNamedImmediate(std::string("offset"),Result(u16,layout.OFFSET)),true,false,false);
                }
                break;
            case 7:  case 31:   // BUFFER_STORE_FORMAT_XYZW,BUFFER_STORE_DWORDX4,
                appendOPR_VGPR_OR_ACCVGPR(layout.VDATA,true,false,4);
                if (layout.IDXEN || layout.OFFEN)  {
                    appendOPR_VGPR(layout.VADDR,true,false);
                } else if (layout.OFFSET)  {
                    insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx90a::off,1),false,false,false);
                }
                if (layout.IDXEN && layout.OFFEN)  {
                    appendOPR_VGPR(layout.VADDR+1,true,false);
                }
                appendOPR_SREG(layout.SRSRC,true,false,4);
                appendOPR_SSRC_NOLIT(layout.SOFFSET,true,false);
                insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx90a::vmcnt,0,36),true,true,true);
                if (layout.IDXEN)  {
                    insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx90a::idxen,1),false,false,false);
                }
                if (layout.OFFEN)  {
                    insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx90a::offen,1),false,false,false);
                }
                if (layout.OFFSET)  {
                    insn_in_progress->appendOperand(NamedImmediate::makeNamedImmediate(std::string("offset"),Result(u16,layout.OFFSET)),true,false,false);
                }
                break;
            case 40:  case 41:  case 62:  case 63:   // BUFFER_WBL2,BUFFER_INVL2,BUFFER_WBINVL1,BUFFER_WBINVL1_VOL,
                insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx90a::vmcnt,0,36),true,true,true);
                if (layout.IDXEN)  {
                    insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx90a::idxen,1),false,false,false);
                }
                if (layout.OFFEN)  {
                    insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx90a::offen,1),false,false,false);
                }
                if (layout.OFFSET)  {
                    insn_in_progress->appendOperand(NamedImmediate::makeNamedImmediate(std::string("offset"),Result(u16,layout.OFFSET)),true,false,false);
                }
                break;
            case 61:   // BUFFER_STORE_LDS_DWORD,
                appendOPR_SREG(layout.SRSRC,true,false,4);
                appendOPR_SSRC_NOLIT(layout.SOFFSET,true,false);
                insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx90a::vmcnt,0,36),true,true,true);
                if (layout.IDXEN)  {
                    insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx90a::idxen,1),false,false,false);
                }
                if (layout.OFFEN)  {
                    insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx90a::offen,1),false,false,false);
                }
                if (layout.OFFSET)  {
                    insn_in_progress->appendOperand(NamedImmediate::makeNamedImmediate(std::string("offset"),Result(u16,layout.OFFSET)),true,false,false);
                }
                break;
            case 64:  case 66:  case 67:  case 68:   // BUFFER_ATOMIC_SWAP,BUFFER_ATOMIC_ADD,BUFFER_ATOMIC_SUB,BUFFER_ATOMIC_SMIN,
            case 69:  case 70:  case 71:  case 72:   // BUFFER_ATOMIC_UMIN,BUFFER_ATOMIC_SMAX,BUFFER_ATOMIC_UMAX,BUFFER_ATOMIC_AND,
            case 73:  case 74:  case 75:  case 76:   // BUFFER_ATOMIC_OR,BUFFER_ATOMIC_XOR,BUFFER_ATOMIC_INC,BUFFER_ATOMIC_DEC,
            case 77:  case 78:   // BUFFER_ATOMIC_ADD_F32,BUFFER_ATOMIC_PK_ADD_F16,
                appendOPR_VGPR_OR_ACCVGPR(layout.VDATA,true,true);
                if (layout.IDXEN || layout.OFFEN)  {
                    appendOPR_VGPR(layout.VADDR,true,false);
                } else if (layout.OFFSET)  {
                    insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx90a::off,1),false,false,false);
                }
                if (layout.IDXEN && layout.OFFEN)  {
                    appendOPR_VGPR(layout.VADDR+1,true,false);
                }
                appendOPR_SREG(layout.SRSRC,true,false,4);
                appendOPR_SSRC_NOLIT(layout.SOFFSET,true,false);
                insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx90a::vmcnt,0,36),true,true,true);
                if (layout.IDXEN)  {
                    insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx90a::idxen,1),false,false,false);
                }
                if (layout.OFFEN)  {
                    insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx90a::offen,1),false,false,false);
                }
                if (layout.OFFSET)  {
                    insn_in_progress->appendOperand(NamedImmediate::makeNamedImmediate(std::string("offset"),Result(u16,layout.OFFSET)),true,false,false);
                }
                break;
            case 65:  case 79:  case 80:   // BUFFER_ATOMIC_CMPSWAP,BUFFER_ATOMIC_ADD_F64,BUFFER_ATOMIC_MIN_F64,
            case 81:  case 96:  case 98:   // BUFFER_ATOMIC_MAX_F64,BUFFER_ATOMIC_SWAP_X2,BUFFER_ATOMIC_ADD_X2,
            case 99:  case 100:  case 101:   // BUFFER_ATOMIC_SUB_X2,BUFFER_ATOMIC_SMIN_X2,BUFFER_ATOMIC_UMIN_X2,
            case 102:  case 103:  case 104:   // BUFFER_ATOMIC_SMAX_X2,BUFFER_ATOMIC_UMAX_X2,BUFFER_ATOMIC_AND_X2,
            case 105:  case 106:  case 107:   // BUFFER_ATOMIC_OR_X2,BUFFER_ATOMIC_XOR_X2,BUFFER_ATOMIC_INC_X2,
            case 108:   // BUFFER_ATOMIC_DEC_X2,
                appendOPR_VGPR_OR_ACCVGPR(layout.VDATA,true,true,2);
                if (layout.IDXEN || layout.OFFEN)  {
                    appendOPR_VGPR(layout.VADDR,true,false);
                } else if (layout.OFFSET)  {
                    insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx90a::off,1),false,false,false);
                }
                if (layout.IDXEN && layout.OFFEN)  {
                    appendOPR_VGPR(layout.VADDR+1,true,false);
                }
                appendOPR_SREG(layout.SRSRC,true,false,4);
                appendOPR_SSRC_NOLIT(layout.SOFFSET,true,false);
                insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx90a::vmcnt,0,36),true,true,true);
                if (layout.IDXEN)  {
                    insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx90a::idxen,1),false,false,false);
                }
                if (layout.OFFEN)  {
                    insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx90a::offen,1),false,false,false);
                }
                if (layout.OFFSET)  {
                    insn_in_progress->appendOperand(NamedImmediate::makeNamedImmediate(std::string("offset"),Result(u16,layout.OFFSET)),true,false,false);
                }
                break;
            case 97:   // BUFFER_ATOMIC_CMPSWAP_X2,
                appendOPR_VGPR_OR_ACCVGPR(layout.VDATA,true,true,4);
                if (layout.IDXEN || layout.OFFEN)  {
                    appendOPR_VGPR(layout.VADDR,true,false);
                } else if (layout.OFFSET)  {
                    insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx90a::off,1),false,false,false);
                }
                if (layout.IDXEN && layout.OFFEN)  {
                    appendOPR_VGPR(layout.VADDR+1,true,false);
                }
                appendOPR_SREG(layout.SRSRC,true,false,4);
                appendOPR_SSRC_NOLIT(layout.SOFFSET,true,false);
                insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx90a::vmcnt,0,36),true,true,true);
                if (layout.IDXEN)  {
                    insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx90a::idxen,1),false,false,false);
                }
                if (layout.OFFEN)  {
                    insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx90a::offen,1),false,false,false);
                }
                if (layout.OFFSET)  {
                    insn_in_progress->appendOperand(NamedImmediate::makeNamedImmediate(std::string("offset"),Result(u16,layout.OFFSET)),true,false,false);
                }
                break;
        }
    }

    void InstructionDecoder_amdgpu_gfx90a::finalizeENC_SMEMOperands()
    {
        layout_ENC_SMEM & layout = insn_layout.ENC_SMEM;
        switch (layout.OP)  {
            case 0:  case 5:   // S_LOAD_DWORD,S_SCRATCH_LOAD_DWORD,
                appendOPR_SREG(layout.SDATA,false,true);
                appendOPR_SREG(layout.SBASE,true,false,2);
                processOPR_SMEM_OFFSET(layout);
                insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx90a::lgkmcnt,0,16),true,true,true);
                break;
            case 1:  case 6:   // S_LOAD_DWORDX2,S_SCRATCH_LOAD_DWORDX2,
                appendOPR_SREG(layout.SDATA,false,true,2);
                appendOPR_SREG(layout.SBASE,true,false,2);
                processOPR_SMEM_OFFSET(layout);
                insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx90a::lgkmcnt,0,16),true,true,true);
                break;
            case 2:  case 7:   // S_LOAD_DWORDX4,S_SCRATCH_LOAD_DWORDX4,
                appendOPR_SREG(layout.SDATA,false,true,4);
                appendOPR_SREG(layout.SBASE,true,false,2);
                processOPR_SMEM_OFFSET(layout);
                insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx90a::lgkmcnt,0,16),true,true,true);
                break;
            case 3:   // S_LOAD_DWORDX8,
                appendOPR_SREG(layout.SDATA,false,true,8);
                appendOPR_SREG(layout.SBASE,true,false,2);
                processOPR_SMEM_OFFSET(layout);
                insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx90a::lgkmcnt,0,16),true,true,true);
                break;
            case 4:   // S_LOAD_DWORDX16,
                appendOPR_SREG(layout.SDATA,false,true,16);
                appendOPR_SREG(layout.SBASE,true,false,2);
                processOPR_SMEM_OFFSET(layout);
                insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx90a::lgkmcnt,0,16),true,true,true);
                break;
            case 8:   // S_BUFFER_LOAD_DWORD,
                appendOPR_SREG(layout.SDATA,false,true);
                appendOPR_SREG(layout.SBASE,true,false,4);
                processOPR_SMEM_OFFSET(layout);
                insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx90a::lgkmcnt,0,16),true,true,true);
                break;
            case 9:   // S_BUFFER_LOAD_DWORDX2,
                appendOPR_SREG(layout.SDATA,false,true,2);
                appendOPR_SREG(layout.SBASE,true,false,4);
                processOPR_SMEM_OFFSET(layout);
                insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx90a::lgkmcnt,0,16),true,true,true);
                break;
            case 10:   // S_BUFFER_LOAD_DWORDX4,
                appendOPR_SREG(layout.SDATA,false,true,4);
                appendOPR_SREG(layout.SBASE,true,false,4);
                processOPR_SMEM_OFFSET(layout);
                insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx90a::lgkmcnt,0,16),true,true,true);
                break;
            case 11:   // S_BUFFER_LOAD_DWORDX8,
                appendOPR_SREG(layout.SDATA,false,true,8);
                appendOPR_SREG(layout.SBASE,true,false,4);
                processOPR_SMEM_OFFSET(layout);
                insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx90a::lgkmcnt,0,16),true,true,true);
                break;
            case 12:   // S_BUFFER_LOAD_DWORDX16,
                appendOPR_SREG(layout.SDATA,false,true,16);
                appendOPR_SREG(layout.SBASE,true,false,4);
                processOPR_SMEM_OFFSET(layout);
                insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx90a::lgkmcnt,0,16),true,true,true);
                break;
            case 16:  case 21:   // S_STORE_DWORD,S_SCRATCH_STORE_DWORD,
                appendOPR_SREG(layout.SDATA,true,false);
                appendOPR_SREG(layout.SBASE,true,false,2);
                processOPR_SMEM_OFFSET(layout);
                insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx90a::lgkmcnt,0,16),true,true,true);
                break;
            case 17:  case 22:   // S_STORE_DWORDX2,S_SCRATCH_STORE_DWORDX2,
                appendOPR_SREG(layout.SDATA,true,false,2);
                appendOPR_SREG(layout.SBASE,true,false,2);
                processOPR_SMEM_OFFSET(layout);
                insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx90a::lgkmcnt,0,16),true,true,true);
                break;
            case 18:  case 23:   // S_STORE_DWORDX4,S_SCRATCH_STORE_DWORDX4,
                appendOPR_SREG(layout.SDATA,true,false,4);
                appendOPR_SREG(layout.SBASE,true,false,2);
                processOPR_SMEM_OFFSET(layout);
                insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx90a::lgkmcnt,0,16),true,true,true);
                break;
            case 24:   // S_BUFFER_STORE_DWORD,
                appendOPR_SREG(layout.SDATA,true,false);
                appendOPR_SREG(layout.SBASE,true,false,4);
                processOPR_SMEM_OFFSET(layout);
                insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx90a::lgkmcnt,0,16),true,true,true);
                break;
            case 25:   // S_BUFFER_STORE_DWORDX2,
                appendOPR_SREG(layout.SDATA,true,false,2);
                appendOPR_SREG(layout.SBASE,true,false,4);
                processOPR_SMEM_OFFSET(layout);
                insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx90a::lgkmcnt,0,16),true,true,true);
                break;
            case 26:   // S_BUFFER_STORE_DWORDX4,
                appendOPR_SREG(layout.SDATA,true,false,4);
                appendOPR_SREG(layout.SBASE,true,false,4);
                processOPR_SMEM_OFFSET(layout);
                insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx90a::lgkmcnt,0,16),true,true,true);
                break;
            case 32:  case 33:  case 34:  case 35:   // S_DCACHE_INV,S_DCACHE_WB,S_DCACHE_INV_VOL,S_DCACHE_WB_VOL,
                insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx90a::lgkmcnt,0,16),true,true,true);
                break;
            case 36:  case 37:   // S_MEMTIME,S_MEMREALTIME,
                appendOPR_SREG(layout.SDATA,false,true,2);
                insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx90a::lgkmcnt,0,16),true,true,true);
                break;
            case 38:   // S_ATC_PROBE,
                appendOPR_SIMM8(layout.SDATA,true,false);
                appendOPR_SREG(layout.SBASE,true,false,2);
                processOPR_SMEM_OFFSET(layout);
                insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx90a::lgkmcnt,0,16),true,true,true);
                break;
            case 39:   // S_ATC_PROBE_BUFFER,
                appendOPR_SIMM8(layout.SDATA,true,false);
                appendOPR_SREG(layout.SBASE,true,false,4);
                processOPR_SMEM_OFFSET(layout);
                insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx90a::lgkmcnt,0,16),true,true,true);
                break;
            case 40:  case 41:   // S_DCACHE_DISCARD,S_DCACHE_DISCARD_X2,
                appendOPR_SREG(layout.SBASE,true,false,2);
                processOPR_SMEM_OFFSET(layout);
                insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx90a::lgkmcnt,0,16),true,true,true);
                break;
            case 64:  case 66:  case 67:   // S_BUFFER_ATOMIC_SWAP,S_BUFFER_ATOMIC_ADD,S_BUFFER_ATOMIC_SUB,
            case 68:  case 69:  case 70:   // S_BUFFER_ATOMIC_SMIN,S_BUFFER_ATOMIC_UMIN,S_BUFFER_ATOMIC_SMAX,
            case 71:  case 72:  case 73:   // S_BUFFER_ATOMIC_UMAX,S_BUFFER_ATOMIC_AND,S_BUFFER_ATOMIC_OR,
            case 74:  case 75:  case 76:   // S_BUFFER_ATOMIC_XOR,S_BUFFER_ATOMIC_INC,S_BUFFER_ATOMIC_DEC,
                appendOPR_SREG(layout.SDATA,true,true);
                appendOPR_SREG(layout.SBASE,true,false,4);
                processOPR_SMEM_OFFSET(layout);
                insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx90a::lgkmcnt,0,16),true,true,true);
                break;
            case 65:  case 96:  case 98:   // S_BUFFER_ATOMIC_CMPSWAP,S_BUFFER_ATOMIC_SWAP_X2,S_BUFFER_ATOMIC_ADD_X2,
            case 99:  case 100:  case 101:   // S_BUFFER_ATOMIC_SUB_X2,S_BUFFER_ATOMIC_SMIN_X2,S_BUFFER_ATOMIC_UMIN_X2,
            case 102:  case 103:  case 104:   // S_BUFFER_ATOMIC_SMAX_X2,S_BUFFER_ATOMIC_UMAX_X2,S_BUFFER_ATOMIC_AND_X2,
            case 105:  case 106:  case 107:   // S_BUFFER_ATOMIC_OR_X2,S_BUFFER_ATOMIC_XOR_X2,S_BUFFER_ATOMIC_INC_X2,
            case 108:   // S_BUFFER_ATOMIC_DEC_X2,
                appendOPR_SREG(layout.SDATA,true,true,2);
                appendOPR_SREG(layout.SBASE,true,false,4);
                processOPR_SMEM_OFFSET(layout);
                insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx90a::lgkmcnt,0,16),true,true,true);
                break;
            case 97:   // S_BUFFER_ATOMIC_CMPSWAP_X2,
                appendOPR_SREG(layout.SDATA,true,true,4);
                appendOPR_SREG(layout.SBASE,true,false,4);
                processOPR_SMEM_OFFSET(layout);
                insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx90a::lgkmcnt,0,16),true,true,true);
                break;
            case 128:  case 130:  case 131:  case 132:   // S_ATOMIC_SWAP,S_ATOMIC_ADD,S_ATOMIC_SUB,S_ATOMIC_SMIN,
            case 133:  case 134:  case 135:  case 136:   // S_ATOMIC_UMIN,S_ATOMIC_SMAX,S_ATOMIC_UMAX,S_ATOMIC_AND,
            case 137:  case 138:  case 139:  case 140:   // S_ATOMIC_OR,S_ATOMIC_XOR,S_ATOMIC_INC,S_ATOMIC_DEC,
                appendOPR_SREG(layout.SDATA,true,true);
                appendOPR_SREG(layout.SBASE,true,false,2);
                processOPR_SMEM_OFFSET(layout);
                insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx90a::lgkmcnt,0,16),true,true,true);
                break;
            case 129:  case 160:  case 162:  case 163:   // S_ATOMIC_CMPSWAP,S_ATOMIC_SWAP_X2,S_ATOMIC_ADD_X2,S_ATOMIC_SUB_X2,
            case 164:  case 165:  case 166:  case 167:   // S_ATOMIC_SMIN_X2,S_ATOMIC_UMIN_X2,S_ATOMIC_SMAX_X2,S_ATOMIC_UMAX_X2,
            case 168:  case 169:  case 170:  case 171:   // S_ATOMIC_AND_X2,S_ATOMIC_OR_X2,S_ATOMIC_XOR_X2,S_ATOMIC_INC_X2,
            case 172:   // S_ATOMIC_DEC_X2,
                appendOPR_SREG(layout.SDATA,true,true,2);
                appendOPR_SREG(layout.SBASE,true,false,2);
                processOPR_SMEM_OFFSET(layout);
                insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx90a::lgkmcnt,0,16),true,true,true);
                break;
            case 161:   // S_ATOMIC_CMPSWAP_X2,
                appendOPR_SREG(layout.SDATA,true,true,4);
                appendOPR_SREG(layout.SBASE,true,false,2);
                processOPR_SMEM_OFFSET(layout);
                insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx90a::lgkmcnt,0,16),true,true,true);
                break;
        }
    }

    void InstructionDecoder_amdgpu_gfx90a::finalizeENC_SOP1Operands()
    {
        layout_ENC_SOP1 & layout = insn_layout.ENC_SOP1;
        switch (layout.OP)  {
            case 0:  case 8:  case 14:  case 16:  case 18:   // S_MOV_B32,S_BREV_B32,S_FF0_I32_B32,S_FF1_I32_B32,S_FLBIT_I32_B32,
            case 20:  case 22:  case 23:   // S_FLBIT_I32,S_SEXT_I32_I8,S_SEXT_I32_I16,
                appendOPR_SDST(layout.SDST,false,true);
                appendOPR_SSRC(layout.SSRC0,true,false);
                break;
            case 1:  case 9:   // S_MOV_B64,S_BREV_B64,
                appendOPR_SDST(layout.SDST,false,true,2);
                appendOPR_SSRC(layout.SSRC0,true,false,2);
                break;
            case 2:   // S_CMOV_B32,
                appendOPR_SDST(layout.SDST,true,true);
                appendOPR_SSRC(layout.SSRC0,true,false);
                appendOPR_SSRC_SPECIAL_SCC(253,true,false,1,true);
                break;
            case 3:   // S_CMOV_B64,
                appendOPR_SDST(layout.SDST,true,true,2);
                appendOPR_SSRC(layout.SSRC0,true,false,2);
                appendOPR_SSRC_SPECIAL_SCC(253,true,false,1,true);
                break;
            case 4:  case 6:  case 10:  case 12:  case 40:   // S_NOT_B32,S_WQM_B32,S_BCNT0_I32_B32,S_BCNT1_I32_B32,S_QUADMASK_B32,
            case 48:   // S_ABS_I32,
                appendOPR_SDST(layout.SDST,false,true);
                appendOPR_SSRC(layout.SSRC0,true,false);
                appendOPR_SSRC_SPECIAL_SCC(253,false,true,1,true);
                break;
            case 5:  case 7:  case 41:   // S_NOT_B64,S_WQM_B64,S_QUADMASK_B64,
                appendOPR_SDST(layout.SDST,false,true,2);
                appendOPR_SSRC(layout.SSRC0,true,false,2);
                appendOPR_SSRC_SPECIAL_SCC(253,false,true,1,true);
                break;
            case 11:  case 13:   // S_BCNT0_I32_B64,S_BCNT1_I32_B64,
                appendOPR_SDST(layout.SDST,false,true);
                appendOPR_SSRC(layout.SSRC0,true,false,2);
                appendOPR_SSRC_SPECIAL_SCC(253,false,true,1,true);
                break;
            case 15:  case 17:  case 19:  case 21:   // S_FF0_I32_B64,S_FF1_I32_B64,S_FLBIT_I32_B64,S_FLBIT_I32_I64,
                appendOPR_SDST(layout.SDST,false,true);
                appendOPR_SSRC(layout.SSRC0,true,false,2);
                break;
            case 24:  case 26:   // S_BITSET0_B32,S_BITSET1_B32,
                appendOPR_SDST(layout.SDST,true,true);
                appendOPR_SSRC(layout.SSRC0,true,false);
                break;
            case 25:  case 27:   // S_BITSET0_B64,S_BITSET1_B64,
                appendOPR_SDST(layout.SDST,true,true,2);
                appendOPR_SSRC(layout.SSRC0,true,false);
                break;
            case 28:   // S_GETPC_B64,
                appendOPR_SDST(layout.SDST,false,true,2);
                appendOPR_PC(0,true,false,1,true);
                break;
            case 29:   // S_SETPC_B64,
                setBranch();
                setModifyPC();
                appendOPR_SREG(layout.SSRC0,true,false,2);
                appendOPR_PC(0,false,true,1,true);
                break;
            case 30:   // S_SWAPPC_B64,
                setBranch();
                setModifyPC();
                appendOPR_SDST(layout.SDST,false,true,2);
                appendOPR_SREG(layout.SSRC0,true,false,2);
                appendOPR_PC(0,false,true,1,true);
                appendOPR_PC(0,true,false,1,true);
                break;
            case 31:   // S_RFE_B64,
                appendOPR_SREG(layout.SSRC0,true,false,2);
                appendOPR_PC(0,false,true,1,true);
                break;
            case 32:  case 33:  case 34:  case 35:   // S_AND_SAVEEXEC_B64,S_OR_SAVEEXEC_B64,S_XOR_SAVEEXEC_B64,S_ANDN2_SAVEEXEC_B64,
            case 36:  case 37:  case 38:   // S_ORN2_SAVEEXEC_B64,S_NAND_SAVEEXEC_B64,S_NOR_SAVEEXEC_B64,
            case 39:  case 51:  case 52:   // S_XNOR_SAVEEXEC_B64,S_ANDN1_SAVEEXEC_B64,S_ORN1_SAVEEXEC_B64,
            case 53:  case 54:   // S_ANDN1_WREXEC_B64,S_ANDN2_WREXEC_B64,
                appendOPR_SREG(layout.SDST,false,true,2);
                appendOPR_SSRC(layout.SSRC0,true,false,2);
                appendOPR_SDST_EXEC(126,false,true,1,true);
                appendOPR_SSRC_SPECIAL_SCC(253,false,true,1,true);
                appendOPR_SDST_EXEC(126,true,false,1,true);
                break;
            case 42:   // S_MOVRELS_B32,
                appendOPR_SDST(layout.SDST,false,true);
                appendOPR_SREG(layout.SSRC0,true,false);
                appendOPR_SDST_M0(124,true,false,1,true);
                break;
            case 43:   // S_MOVRELS_B64,
                appendOPR_SDST(layout.SDST,false,true,2);
                appendOPR_SREG(layout.SSRC0,true,false,2);
                appendOPR_SDST_M0(124,true,false,1,true);
                break;
            case 44:   // S_MOVRELD_B32,
                appendOPR_SREG(layout.SDST,false,true);
                appendOPR_SSRC(layout.SSRC0,true,false);
                appendOPR_SDST_M0(124,true,false,1,true);
                break;
            case 45:   // S_MOVRELD_B64,
                appendOPR_SREG(layout.SDST,false,true,2);
                appendOPR_SSRC(layout.SSRC0,true,false,2);
                appendOPR_SDST_M0(124,true,false,1,true);
                break;
            case 46:   // S_CBRANCH_JOIN,
                appendOPR_SREG(layout.SSRC0,true,false);
                appendOPR_SDST_EXEC(126,false,true,1,true);
                appendOPR_PC(0,false,true,1,true);
                break;
            case 50:   // S_SET_GPR_IDX_IDX,
                appendOPR_SSRC(layout.SSRC0,true,false);
                appendOPR_SDST_M0(124,false,true,1,true);
                appendOPR_SDST_M0(124,true,false,1,true);
                break;
            case 55:   // S_BITREPLICATE_B64_B32,
                appendOPR_SDST(layout.SDST,false,true,2);
                appendOPR_SSRC(layout.SSRC0,true,false);
                break;
        }
    }

    void InstructionDecoder_amdgpu_gfx90a::finalizeENC_SOP2Operands()
    {
        layout_ENC_SOP2 & layout = insn_layout.ENC_SOP2;
        switch (layout.OP)  {
            case 0:  case 1:  case 2:  case 3:  case 6:  case 7:   // S_ADD_U32,S_SUB_U32,S_ADD_I32,S_SUB_I32,S_MIN_I32,S_MIN_U32,
            case 8:  case 9:  case 12:  case 14:  case 16:   // S_MAX_I32,S_MAX_U32,S_AND_B32,S_OR_B32,S_XOR_B32,
            case 18:  case 20:  case 22:  case 24:  case 26:   // S_ANDN2_B32,S_ORN2_B32,S_NAND_B32,S_NOR_B32,S_XNOR_B32,
            case 28:  case 30:  case 32:  case 37:  case 38:   // S_LSHL_B32,S_LSHR_B32,S_ASHR_I32,S_BFE_U32,S_BFE_I32,
            case 42:  case 46:  case 47:  case 48:   // S_ABSDIFF_I32,S_LSHL1_ADD_U32,S_LSHL2_ADD_U32,S_LSHL3_ADD_U32,
            case 49:   // S_LSHL4_ADD_U32,
                appendOPR_SDST(layout.SDST,false,true);
                appendOPR_SSRC(layout.SSRC0,true,false);
                appendOPR_SSRC(layout.SSRC1,true,false);
                appendOPR_SSRC_SPECIAL_SCC(253,false,true,1,true);
                break;
            case 4:  case 5:   // S_ADDC_U32,S_SUBB_U32,
                appendOPR_SDST(layout.SDST,false,true);
                appendOPR_SSRC(layout.SSRC0,true,false);
                appendOPR_SSRC(layout.SSRC1,true,false);
                appendOPR_SSRC_SPECIAL_SCC(253,false,true,1,true);
                appendOPR_SSRC_SPECIAL_SCC(253,true,false,1,true);
                break;
            case 10:   // S_CSELECT_B32,
                appendOPR_SDST(layout.SDST,false,true);
                appendOPR_SSRC(layout.SSRC0,true,false);
                appendOPR_SSRC(layout.SSRC1,true,false);
                appendOPR_SSRC_SPECIAL_SCC(253,true,false,1,true);
                break;
            case 11:   // S_CSELECT_B64,
                appendOPR_SDST(layout.SDST,false,true,2);
                appendOPR_SSRC(layout.SSRC0,true,false,2);
                appendOPR_SSRC(layout.SSRC1,true,false,2);
                appendOPR_SSRC_SPECIAL_SCC(253,true,false,1,true);
                break;
            case 13:  case 15:  case 17:  case 19:  case 21:   // S_AND_B64,S_OR_B64,S_XOR_B64,S_ANDN2_B64,S_ORN2_B64,
            case 23:  case 25:  case 27:   // S_NAND_B64,S_NOR_B64,S_XNOR_B64,
                appendOPR_SDST(layout.SDST,false,true,2);
                appendOPR_SSRC(layout.SSRC0,true,false,2);
                appendOPR_SSRC(layout.SSRC1,true,false,2);
                appendOPR_SSRC_SPECIAL_SCC(253,false,true,1,true);
                break;
            case 29:  case 31:  case 33:  case 39:  case 40:   // S_LSHL_B64,S_LSHR_B64,S_ASHR_I64,S_BFE_U64,S_BFE_I64,
                appendOPR_SDST(layout.SDST,false,true,2);
                appendOPR_SSRC(layout.SSRC0,true,false,2);
                appendOPR_SSRC(layout.SSRC1,true,false);
                appendOPR_SSRC_SPECIAL_SCC(253,false,true,1,true);
                break;
            case 34:  case 36:  case 44:  case 45:  case 50:   // S_BFM_B32,S_MUL_I32,S_MUL_HI_U32,S_MUL_HI_I32,S_PACK_LL_B32_B16,
            case 51:  case 52:   // S_PACK_LH_B32_B16,S_PACK_HH_B32_B16,
                appendOPR_SDST(layout.SDST,false,true);
                appendOPR_SSRC(layout.SSRC0,true,false);
                appendOPR_SSRC(layout.SSRC1,true,false);
                break;
            case 35:   // S_BFM_B64,
                appendOPR_SDST(layout.SDST,false,true,2);
                appendOPR_SSRC(layout.SSRC0,true,false);
                appendOPR_SSRC(layout.SSRC1,true,false);
                break;
            case 41:   // S_CBRANCH_G_FORK,
                appendOPR_SSRC_NOLIT(layout.SSRC0,true,false,2);
                appendOPR_SSRC_NOLIT(layout.SSRC1,true,false,2);
                appendOPR_PC(0,false,true,1,true);
                break;
            case 43:   // S_RFE_RESTORE_B64,
                appendOPR_SSRC(layout.SSRC0,true,false,2);
                appendOPR_SSRC(layout.SSRC1,true,false);
                appendOPR_PC(0,false,true,1,true);
                break;
        }
    }

    void InstructionDecoder_amdgpu_gfx90a::finalizeENC_SOPCOperands()
    {
        layout_ENC_SOPC & layout = insn_layout.ENC_SOPC;
        switch (layout.OP)  {
            case 0:  case 1:  case 2:  case 3:  case 4:   // S_CMP_EQ_I32,S_CMP_LG_I32,S_CMP_GT_I32,S_CMP_GE_I32,S_CMP_LT_I32,
            case 5:  case 6:  case 7:  case 8:  case 9:   // S_CMP_LE_I32,S_CMP_EQ_U32,S_CMP_LG_U32,S_CMP_GT_U32,S_CMP_GE_U32,
            case 10:  case 11:  case 12:  case 13:   // S_CMP_LT_U32,S_CMP_LE_U32,S_BITCMP0_B32,S_BITCMP1_B32,
                appendOPR_SSRC(layout.SSRC0,true,false);
                appendOPR_SSRC(layout.SSRC1,true,false);
                appendOPR_SSRC_SPECIAL_SCC(253,false,true,1,true);
                break;
            case 14:  case 15:   // S_BITCMP0_B64,S_BITCMP1_B64,
                appendOPR_SSRC(layout.SSRC0,true,false,2);
                appendOPR_SSRC(layout.SSRC1,true,false);
                appendOPR_SSRC_SPECIAL_SCC(253,false,true,1,true);
                break;
            case 16:   // S_SETVSKIP,
                appendOPR_SSRC(layout.SSRC0,true,false);
                appendOPR_SSRC(layout.SSRC1,true,false);
                break;
            case 17:   // S_SET_GPR_IDX_ON,
                appendOPR_SSRC(layout.SSRC0,true,false);
                appendOPR_SIMM4(layout.SSRC1,true,false);
                appendOPR_SDST_M0(124,false,true,1,true);
                appendOPR_SDST_M0(124,true,false,1,true);
                break;
            case 18:  case 19:   // S_CMP_EQ_U64,S_CMP_LG_U64,
                appendOPR_SSRC(layout.SSRC0,true,false,2);
                appendOPR_SSRC(layout.SSRC1,true,false,2);
                appendOPR_SSRC_SPECIAL_SCC(253,false,true,1,true);
                break;
        }
    }

    void InstructionDecoder_amdgpu_gfx90a::finalizeENC_SOPKOperands()
    {
        layout_ENC_SOPK & layout = insn_layout.ENC_SOPK;
        switch (layout.OP)  {
            case 0:  case 17:   // S_MOVK_I32,S_GETREG_B32,
                appendOPR_SDST(layout.SDST,false,true);
                appendOPR_SIMM16(layout.SIMM16,true,false);
                break;
            case 1:   // S_CMOVK_I32,
                appendOPR_SDST(layout.SDST,true,true);
                appendOPR_SIMM16(layout.SIMM16,true,false);
                appendOPR_SSRC_SPECIAL_SCC(253,true,false,1,true);
                break;
            case 2:  case 3:  case 4:  case 5:  case 6:   // S_CMPK_EQ_I32,S_CMPK_LG_I32,S_CMPK_GT_I32,S_CMPK_GE_I32,S_CMPK_LT_I32,
            case 7:  case 8:  case 9:  case 10:  case 11:   // S_CMPK_LE_I32,S_CMPK_EQ_U32,S_CMPK_LG_U32,S_CMPK_GT_U32,S_CMPK_GE_U32,
            case 12:  case 13:   // S_CMPK_LT_U32,S_CMPK_LE_U32,
                appendOPR_SDST(layout.SDST,true,false);
                appendOPR_SIMM16(layout.SIMM16,true,false);
                appendOPR_SSRC_SPECIAL_SCC(253,false,true,1,true);
                break;
            case 14:   // S_ADDK_I32,
                appendOPR_SDST(layout.SDST,true,true);
                appendOPR_SIMM16(layout.SIMM16,true,false);
                appendOPR_SSRC_SPECIAL_SCC(253,false,true,1,true);
                break;
            case 15:   // S_MULK_I32,
                appendOPR_SDST(layout.SDST,true,true);
                appendOPR_SIMM16(layout.SIMM16,true,false);
                break;
            case 16:   // S_CBRANCH_I_FORK,
                appendOPR_SDST(layout.SDST,true,false,2);
                insn_in_progress->appendOperand(decodeOPR_LABEL(layout.SIMM16),true,false);
                break;
            case 18:   // S_SETREG_B32,
                appendOPR_SIMM16(layout.SIMM16,false,true);
                appendOPR_SDST(layout.SDST,true,false);
                break;
            case 21:   // S_CALL_B64,
                appendOPR_SDST(layout.SDST,false,true,2);
                insn_in_progress->appendOperand(decodeOPR_LABEL(layout.SIMM16),true,false);
                appendOPR_PC(0,false,true,1,true);
                appendOPR_PC(0,true,false,1,true);
                break;
        }
    }

    void InstructionDecoder_amdgpu_gfx90a::finalizeSOPK_INST_LITERAL_Operands()
    {
        layout_SOPK_INST_LITERAL_ & layout = insn_layout.SOPK_INST_LITERAL_;
        switch (layout.OP)  {
            case 20:   // S_SETREG_IMM32_B32,
                appendOPR_SIMM16(layout.SIMM16,false,true);
                appendOPR_SIMM32(layout.SIMM32,true,false);
                break;
        }
    }

    void InstructionDecoder_amdgpu_gfx90a::finalizeENC_SOPPOperands()
    {
        layout_ENC_SOPP & layout = insn_layout.ENC_SOPP;
        switch (layout.OP)  {
            case 0:  case 11:  case 13:  case 14:  case 15:  case 18:   // S_NOP,S_SETKILL,S_SETHALT,S_SLEEP,S_SETPRIO,S_TRAP,
            case 20:  case 21:   // S_INCPERFLEVEL,S_DECPERFLEVEL,
                appendOPR_SIMM16(layout.SIMM16,true,false);
                break;
            case 1:  case 3:  case 10:  case 19:  case 27:   // S_ENDPGM,S_WAKEUP,S_BARRIER,S_ICACHE_INV,S_ENDPGM_SAVED,
            case 28:  case 30:   // S_SET_GPR_IDX_OFF,S_ENDPGM_ORDERED_PS_DONE,
                break;
            case 2:   // S_BRANCH,
                setBranch();
                makeBranchTarget(isCall,isConditional,layout.SIMM16);
                break;
            case 4:  case 5:   // S_CBRANCH_SCC0,S_CBRANCH_SCC1,
                setBranch();
                setConditionalBranch();
                makeBranchTarget(isCall,isConditional,layout.SIMM16);
                appendOPR_SSRC_SPECIAL_SCC(253,true,false,1,true);
                break;
            case 6:  case 7:   // S_CBRANCH_VCCZ,S_CBRANCH_VCCNZ,
                setBranch();
                setConditionalBranch();
                makeBranchTarget(isCall,isConditional,layout.SIMM16);
                appendOPR_VCC(0,true,false,1,true);
                break;
            case 8:  case 9:   // S_CBRANCH_EXECZ,S_CBRANCH_EXECNZ,
                setBranch();
                setConditionalBranch();
                makeBranchTarget(isCall,isConditional,layout.SIMM16);
                appendOPR_SDST_EXEC(126,true,false,1,true);
                break;
            case 12:   // S_WAITCNT,
                {
                    uint32_t vmcnt = ((0x3& (layout.SIMM16 >>14))<<4) | (layout.SIMM16 & 0xf);
                    uint32_t expcnt = ((layout.SIMM16>>4) & 0x7);
                    uint32_t lgkmcnt = ((layout.SIMM16>>8) & 0xf);
                    if (vmcnt != 0x3f)
                    {
                        insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx90a::vmcnt,0,32),false,true);
                        insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u32,vmcnt)),false,false);
                    }
                    if (expcnt != 0x7)
                    {
                        insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx90a::expcnt,0,32),false,true);
                        insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u32,expcnt)),false,false);
                    }
                    if (lgkmcnt != 0xf)
                    {
                        insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx90a::lgkmcnt,0,32),false,true);
                        insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u32,lgkmcnt)),false,false);
                    }
                }
                break;
            case 16:  case 17:   // S_SENDMSG,S_SENDMSGHALT,
                appendOPR_SIMM16(layout.SIMM16,true,false);
                appendOPR_SDST_M0(124,true,false,1,true);
                break;
            case 22:   // S_TTRACEDATA,
                appendOPR_SDST_M0(124,true,false,1,true);
                break;
            case 23:  case 24:  case 25:   // S_CBRANCH_CDBGSYS,S_CBRANCH_CDBGUSER,S_CBRANCH_CDBGSYS_OR_USER,
            case 26:   // S_CBRANCH_CDBGSYS_AND_USER,
                insn_in_progress->appendOperand(decodeOPR_LABEL(layout.SIMM16),true,false);
                break;
            case 29:   // S_SET_GPR_IDX_MODE,
                appendOPR_SIMM16(layout.SIMM16,true,false);
                appendOPR_SDST_M0(124,false,true,1,true);
                appendOPR_SDST_M0(124,true,false,1,true);
                break;
        }
    }

    void InstructionDecoder_amdgpu_gfx90a::finalizeENC_VOP1Operands()
    {
        layout_ENC_VOP1 & layout = insn_layout.ENC_VOP1;
        switch (layout.OP)  {
            case 0:  case 53:   // V_NOP,V_CLREXCP,
                break;
            case 1:  case 5:  case 6:  case 7:  case 8:   // V_MOV_B32,V_CVT_F32_I32,V_CVT_F32_U32,V_CVT_U32_F32,V_CVT_I32_F32,
            case 10:  case 11:  case 12:  case 13:   // V_CVT_F16_F32,V_CVT_F32_F16,V_CVT_RPI_I32_F32,V_CVT_FLR_I32_F32,
            case 14:  case 17:  case 18:  case 19:   // V_CVT_OFF_F32_I4,V_CVT_F32_UBYTE0,V_CVT_F32_UBYTE1,V_CVT_F32_UBYTE2,
            case 20:  case 27:  case 28:  case 29:  case 30:   // V_CVT_F32_UBYTE3,V_FRACT_F32,V_TRUNC_F32,V_CEIL_F32,V_RNDNE_F32,
            case 31:  case 32:  case 33:  case 34:  case 35:   // V_FLOOR_F32,V_EXP_F32,V_LOG_F32,V_RCP_F32,V_RCP_IFLAG_F32,
            case 36:  case 39:  case 41:  case 42:  case 43:   // V_RSQ_F32,V_SQRT_F32,V_SIN_F32,V_COS_F32,V_NOT_B32,
            case 44:  case 45:  case 46:  case 47:  case 51:   // V_BFREV_B32,V_FFBH_U32,V_FFBL_B32,V_FFBH_I32,V_FREXP_EXP_I32_F32,
            case 52:  case 55:  case 57:  case 58:   // V_FREXP_MANT_F32,V_SCREEN_PARTITION_4SE_B32,V_CVT_F16_U16,V_CVT_F16_I16,
            case 59:  case 60:  case 61:  case 62:  case 63:   // V_CVT_U16_F16,V_CVT_I16_F16,V_RCP_F16,V_SQRT_F16,V_RSQ_F16,
            case 64:  case 65:  case 66:  case 67:   // V_LOG_F16,V_EXP_F16,V_FREXP_MANT_F16,V_FREXP_EXP_I16_F16,
            case 68:  case 69:  case 70:  case 71:  case 72:   // V_FLOOR_F16,V_CEIL_F16,V_TRUNC_F16,V_RNDNE_F16,V_FRACT_F16,
            case 73:  case 74:  case 75:  case 76:   // V_SIN_F16,V_COS_F16,V_EXP_LEGACY_F32,V_LOG_LEGACY_F32,
            case 77:  case 78:  case 79:   // V_CVT_NORM_I16_F16,V_CVT_NORM_U16_F16,V_SAT_PK_U8_I16,
                appendOPR_VGPR(layout.VDST,false,true);
                appendOPR_SRC(layout.SRC0,true,false);
                break;
            case 2:   // V_READFIRSTLANE_B32,
                appendOPR_SREG_NOVCC(layout.VDST,false,true);
                appendOPR_VGPR_OR_LDS(layout.SRC0,true,false);
                break;
            case 3:  case 15:  case 21:  case 48:   // V_CVT_I32_F64,V_CVT_F32_F64,V_CVT_U32_F64,V_FREXP_EXP_I32_F64,
                appendOPR_VGPR(layout.VDST,false,true);
                appendOPR_SRC(layout.SRC0,true,false,2);
                break;
            case 4:  case 16:  case 22:   // V_CVT_F64_I32,V_CVT_F64_F32,V_CVT_F64_U32,
                appendOPR_VGPR(layout.VDST,false,true,2);
                appendOPR_SRC(layout.SRC0,true,false);
                break;
            case 23:  case 24:  case 25:  case 26:  case 37:   // V_TRUNC_F64,V_CEIL_F64,V_RNDNE_F64,V_FLOOR_F64,V_RCP_F64,
            case 38:  case 40:  case 49:  case 50:   // V_RSQ_F64,V_SQRT_F64,V_FREXP_MANT_F64,V_FRACT_F64,
                appendOPR_VGPR(layout.VDST,false,true,2);
                appendOPR_SRC(layout.SRC0,true,false,2);
                break;
            case 81:   // V_SWAP_B32,
                appendOPR_VGPR(layout.VDST,true,true);
                appendOPR_SRC_VGPR(layout.SRC0,true,true);
                break;
            case 82:   // V_ACCVGPR_MOV_B32,
                appendOPR_ACCVGPR(layout.VDST,false,true);
                appendOPR_SRC_ACCVGPR(layout.SRC0,true,false);
                break;
        }
    }

    void InstructionDecoder_amdgpu_gfx90a::finalizeENC_VOP3Operands()
    {
        layout_ENC_VOP3 & layout = insn_layout.ENC_VOP3;
        switch (layout.OP)  {
            case 320:  case 373:   // V_NOP,V_CLREXCP,
                break;
            case 321:  case 325:  case 326:  case 327:   // V_MOV_B32,V_CVT_F32_I32,V_CVT_F32_U32,V_CVT_U32_F32,
            case 328:  case 330:  case 331:  case 332:   // V_CVT_I32_F32,V_CVT_F16_F32,V_CVT_F32_F16,V_CVT_RPI_I32_F32,
            case 333:  case 334:  case 337:  case 338:   // V_CVT_FLR_I32_F32,V_CVT_OFF_F32_I4,V_CVT_F32_UBYTE0,V_CVT_F32_UBYTE1,
            case 339:  case 340:  case 347:  case 348:   // V_CVT_F32_UBYTE2,V_CVT_F32_UBYTE3,V_FRACT_F32,V_TRUNC_F32,
            case 349:  case 350:  case 351:  case 352:  case 353:   // V_CEIL_F32,V_RNDNE_F32,V_FLOOR_F32,V_EXP_F32,V_LOG_F32,
            case 354:  case 355:  case 356:  case 359:  case 361:   // V_RCP_F32,V_RCP_IFLAG_F32,V_RSQ_F32,V_SQRT_F32,V_SIN_F32,
            case 362:  case 363:  case 364:  case 365:  case 366:   // V_COS_F32,V_NOT_B32,V_BFREV_B32,V_FFBH_U32,V_FFBL_B32,
            case 367:  case 371:  case 372:   // V_FFBH_I32,V_FREXP_EXP_I32_F32,V_FREXP_MANT_F32,
            case 375:  case 377:  case 378:  case 379:   // V_SCREEN_PARTITION_4SE_B32,V_CVT_F16_U16,V_CVT_F16_I16,V_CVT_U16_F16,
            case 380:  case 381:  case 382:  case 383:  case 384:   // V_CVT_I16_F16,V_RCP_F16,V_SQRT_F16,V_RSQ_F16,V_LOG_F16,
            case 385:  case 386:  case 387:  case 388:   // V_EXP_F16,V_FREXP_MANT_F16,V_FREXP_EXP_I16_F16,V_FLOOR_F16,
            case 389:  case 390:  case 391:  case 392:  case 393:   // V_CEIL_F16,V_TRUNC_F16,V_RNDNE_F16,V_FRACT_F16,V_SIN_F16,
            case 394:  case 395:  case 396:  case 397:   // V_COS_F16,V_EXP_LEGACY_F32,V_LOG_LEGACY_F32,V_CVT_NORM_I16_F16,
            case 398:  case 399:   // V_CVT_NORM_U16_F16,V_SAT_PK_U8_I16,
                appendOPR_VGPR(layout.VDST,false,true);
                appendOPR_SRC_NOLIT(layout.SRC0,true,false);
                break;
            case 322:   // V_READFIRSTLANE_B32,
                appendOPR_SREG_NOVCC(layout.VDST,false,true);
                appendOPR_VGPR_OR_LDS(layout.SRC0,true,false);
                break;
            case 323:  case 335:  case 341:  case 368:   // V_CVT_I32_F64,V_CVT_F32_F64,V_CVT_U32_F64,V_FREXP_EXP_I32_F64,
                appendOPR_VGPR(layout.VDST,false,true);
                appendOPR_SRC_NOLIT(layout.SRC0,true,false,2);
                break;
            case 324:  case 336:  case 342:   // V_CVT_F64_I32,V_CVT_F64_F32,V_CVT_F64_U32,
                appendOPR_VGPR(layout.VDST,false,true,2);
                appendOPR_SRC_NOLIT(layout.SRC0,true,false);
                break;
            case 343:  case 344:  case 345:  case 346:  case 357:   // V_TRUNC_F64,V_CEIL_F64,V_RNDNE_F64,V_FLOOR_F64,V_RCP_F64,
            case 358:  case 360:  case 369:  case 370:   // V_RSQ_F64,V_SQRT_F64,V_FREXP_MANT_F64,V_FRACT_F64,
                appendOPR_VGPR(layout.VDST,false,true,2);
                appendOPR_SRC_NOLIT(layout.SRC0,true,false,2);
                break;
            case 401:   // V_SWAP_B32,
                appendOPR_VGPR(layout.VDST,true,true);
                appendOPR_SRC_VGPR(layout.SRC0,true,true);
                break;
            case 402:   // V_ACCVGPR_MOV_B32,
                appendOPR_ACCVGPR(layout.VDST,false,true);
                appendOPR_SRC_ACCVGPR(layout.SRC0,true,false);
                break;
            case 256:   // V_CNDMASK_B32,
                appendOPR_VGPR(layout.VDST,false,true);
                appendOPR_SRC_NOLIT(layout.SRC0,true,false);
                appendOPR_SRC_SIMPLE(layout.SRC1,true,false);
                appendOPR_SREG(layout.SRC2,true,false,2);
                break;
            case 257:  case 258:  case 259:  case 261:  case 262:   // V_ADD_F32,V_SUB_F32,V_SUBREV_F32,V_MUL_F32,V_MUL_I32_I24,
            case 263:  case 264:  case 265:  case 266:   // V_MUL_HI_I32_I24,V_MUL_U32_U24,V_MUL_HI_U32_U24,V_MIN_F32,
            case 267:  case 268:  case 269:  case 270:  case 271:   // V_MAX_F32,V_MIN_I32,V_MAX_I32,V_MIN_U32,V_MAX_U32,
            case 275:  case 276:  case 277:  case 287:  case 288:   // V_AND_B32,V_OR_B32,V_XOR_B32,V_ADD_F16,V_SUB_F16,
            case 290:  case 294:  case 295:  case 297:  case 301:   // V_MUL_F16,V_ADD_U16,V_SUB_U16,V_MUL_LO_U16,V_MAX_F16,
            case 302:  case 303:  case 304:  case 305:  case 306:   // V_MIN_F16,V_MAX_U16,V_MAX_I16,V_MIN_U16,V_MIN_I16,
            case 307:  case 308:  case 309:  case 317:  case 645:   // V_LDEXP_F16,V_ADD_U32,V_SUB_U32,V_XNOR_B32,V_MUL_LO_U32,
            case 646:  case 647:  case 648:  case 651:   // V_MUL_HI_U32,V_MUL_HI_I32,V_LDEXP_F32,V_BCNT_U32_B32,
            case 652:  case 653:  case 659:  case 660:   // V_MBCNT_LO_U32_B32,V_MBCNT_HI_U32_B32,V_BFM_B32,V_CVT_PKNORM_I16_F32,
            case 661:  case 662:  case 663:   // V_CVT_PKNORM_U16_F32,V_CVT_PKRTZ_F16_F32,V_CVT_PK_U16_U32,
            case 664:  case 665:  case 666:  case 668:   // V_CVT_PK_I16_I32,V_CVT_PKNORM_I16_F16,V_CVT_PKNORM_U16_F16,V_ADD_I32,
            case 669:  case 670:  case 671:  case 672:  case 673:   // V_SUB_I32,V_ADD_I16,V_SUB_I16,V_PACK_B32_F16,V_MUL_LEGACY_F32,
                appendOPR_VGPR(layout.VDST,false,true);
                appendOPR_SRC_NOLIT(layout.SRC0,true,false);
                appendOPR_SRC_SIMPLE(layout.SRC1,true,false);
                break;
            case 260:   // V_FMAC_F64,
                appendOPR_VGPR(layout.VDST,true,true,2);
                appendOPR_SRC_NOLIT(layout.SRC0,true,false,2);
                appendOPR_SRC_SIMPLE(layout.SRC1,true,false,2);
                break;
            case 272:  case 273:  case 274:  case 289:   // V_LSHRREV_B32,V_ASHRREV_I32,V_LSHLREV_B32,V_SUBREV_F16,
            case 296:  case 298:  case 299:  case 300:   // V_SUBREV_U16,V_LSHLREV_B16,V_LSHRREV_B16,V_ASHRREV_I16,
            case 310:   // V_SUBREV_U32,
                appendOPR_VGPR(layout.VDST,false,true);
                appendOPR_SRC_SIMPLE(layout.SRC0,true,false);
                appendOPR_SRC_SIMPLE(layout.SRC1,true,false);
                break;
            case 278:  case 291:  case 311:  case 312:   // V_MAC_F32,V_MAC_F16,V_DOT2C_F32_F16,V_DOT2C_I32_I16,
            case 313:  case 314:  case 315:  case 316:   // V_DOT4C_I32_I8,V_DOT8C_I32_I4,V_FMAC_F32,V_PK_FMAC_F16,
            case 496:   // V_CVT_PKACCUM_U8_F32,
                appendOPR_VGPR(layout.VDST,true,true);
                appendOPR_SRC_NOLIT(layout.SRC0,true,false);
                appendOPR_SRC_SIMPLE(layout.SRC1,true,false);
                break;
            case 448:  case 449:  case 450:  case 451:   // V_MAD_LEGACY_F32,V_MAD_F32,V_MAD_I32_I24,V_MAD_U32_U24,
            case 452:  case 453:  case 454:  case 455:  case 456:   // V_CUBEID_F32,V_CUBESC_F32,V_CUBETC_F32,V_CUBEMA_F32,V_BFE_U32,
            case 457:  case 458:  case 459:  case 461:  case 462:   // V_BFE_I32,V_BFI_B32,V_FMA_F32,V_LERP_U8,V_ALIGNBIT_B32,
            case 463:  case 464:  case 465:  case 466:  case 467:   // V_ALIGNBYTE_B32,V_MIN3_F32,V_MIN3_I32,V_MIN3_U32,V_MAX3_F32,
            case 468:  case 469:  case 470:  case 471:  case 472:   // V_MAX3_I32,V_MAX3_U32,V_MED3_F32,V_MED3_I32,V_MED3_U32,
            case 473:  case 474:  case 475:  case 476:  case 477:   // V_SAD_U8,V_SAD_HI_U8,V_SAD_U16,V_SAD_U32,V_CVT_PK_U8_F32,
            case 478:  case 484:  case 490:  case 491:   // V_DIV_FIXUP_F32,V_MSAD_U8,V_MAD_LEGACY_F16,V_MAD_LEGACY_U16,
            case 492:  case 493:  case 494:  case 495:   // V_MAD_LEGACY_I16,V_PERM_B32,V_FMA_LEGACY_F16,V_DIV_FIXUP_LEGACY_F16,
            case 497:  case 498:  case 499:  case 500:  case 501:   // V_MAD_U32_U16,V_MAD_I32_I16,V_XAD_U32,V_MIN3_F16,V_MIN3_I16,
            case 502:  case 503:  case 504:  case 505:  case 506:   // V_MIN3_U16,V_MAX3_F16,V_MAX3_I16,V_MAX3_U16,V_MED3_F16,
            case 507:  case 508:  case 509:  case 510:  case 511:   // V_MED3_I16,V_MED3_U16,V_LSHL_ADD_U32,V_ADD_LSHL_U32,V_ADD3_U32,
            case 512:  case 513:  case 514:  case 515:  case 516:   // V_LSHL_OR_B32,V_AND_OR_B32,V_OR3_B32,V_MAD_F16,V_MAD_U16,
            case 517:  case 518:  case 519:   // V_MAD_I16,V_FMA_F16,V_DIV_FIXUP_F16,
                appendOPR_VGPR(layout.VDST,false,true);
                appendOPR_SRC_NOLIT(layout.SRC0,true,false);
                appendOPR_SRC_SIMPLE(layout.SRC1,true,false);
                appendOPR_SRC_SIMPLE(layout.SRC2,true,false);
                break;
            case 460:  case 479:   // V_FMA_F64,V_DIV_FIXUP_F64,
                appendOPR_VGPR(layout.VDST,false,true,2);
                appendOPR_SRC_NOLIT(layout.SRC0,true,false,2);
                appendOPR_SRC_SIMPLE(layout.SRC1,true,false,2);
                appendOPR_SRC_SIMPLE(layout.SRC2,true,false,2);
                break;
            case 482:   // V_DIV_FMAS_F32,
                appendOPR_VGPR(layout.VDST,false,true);
                appendOPR_SRC_NOLIT(layout.SRC0,true,false);
                appendOPR_SRC_SIMPLE(layout.SRC1,true,false);
                appendOPR_SRC_SIMPLE(layout.SRC2,true,false);
                appendOPR_VCC(0,true,false,1,true);
                break;
            case 483:   // V_DIV_FMAS_F64,
                appendOPR_VGPR(layout.VDST,false,true,2);
                appendOPR_SRC_NOLIT(layout.SRC0,true,false,2);
                appendOPR_SRC_SIMPLE(layout.SRC1,true,false,2);
                appendOPR_SRC_SIMPLE(layout.SRC2,true,false,2);
                appendOPR_VCC(0,true,false,1,true);
                break;
            case 485:  case 486:   // V_QSAD_PK_U16_U8,V_MQSAD_PK_U16_U8,
                appendOPR_VGPR(layout.VDST,false,true,2);
                appendOPR_SRC_NOLIT(layout.SRC0,true,false,2);
                appendOPR_SRC_SIMPLE(layout.SRC1,true,false);
                appendOPR_SRC_SIMPLE(layout.SRC2,true,false,2);
                break;
            case 487:   // V_MQSAD_U32_U8,
                appendOPR_VGPR(layout.VDST,false,true,4);
                appendOPR_SRC_NOLIT(layout.SRC0,true,false,2);
                appendOPR_SRC_SIMPLE(layout.SRC1,true,false);
                appendOPR_SRC_VGPR(layout.SRC2,true,false,4);
                break;
            case 640:  case 641:  case 642:  case 643:   // V_ADD_F64,V_MUL_F64,V_MIN_F64,V_MAX_F64,
                appendOPR_VGPR(layout.VDST,false,true,2);
                appendOPR_SRC_NOLIT(layout.SRC0,true,false,2);
                appendOPR_SRC_SIMPLE(layout.SRC1,true,false,2);
                break;
            case 644:  case 658:   // V_LDEXP_F64,V_TRIG_PREOP_F64,
                appendOPR_VGPR(layout.VDST,false,true,2);
                appendOPR_SRC_NOLIT(layout.SRC0,true,false,2);
                appendOPR_SRC_SIMPLE(layout.SRC1,true,false);
                break;
            case 649:   // V_READLANE_B32,
                appendOPR_SREG_NOVCC(layout.VDST,false,true);
                appendOPR_VGPR_OR_LDS(layout.SRC0,true,false);
                appendOPR_SSRC_LANESEL(layout.SRC1,true,false);
                break;
            case 650:   // V_WRITELANE_B32,
                appendOPR_VGPR(layout.VDST,true,true);
                appendOPR_SSRC_NOLIT(layout.SRC0,true,false);
                appendOPR_SSRC_LANESEL(layout.SRC1,true,false);
                break;
            case 655:  case 656:  case 657:   // V_LSHLREV_B64,V_LSHRREV_B64,V_ASHRREV_I64,
                appendOPR_VGPR(layout.VDST,false,true,2);
                appendOPR_SRC_SIMPLE(layout.SRC0,true,false);
                appendOPR_SRC_SIMPLE(layout.SRC1,true,false,2);
                break;
            case 16:  case 20:  case 32:  case 33:   // V_CMP_CLASS_F32,V_CMP_CLASS_F16,V_CMP_F_F16,V_CMP_LT_F16,
            case 34:  case 35:  case 36:  case 37:  case 38:   // V_CMP_EQ_F16,V_CMP_LE_F16,V_CMP_GT_F16,V_CMP_LG_F16,V_CMP_GE_F16,
            case 39:  case 40:  case 41:  case 42:  case 43:   // V_CMP_O_F16,V_CMP_U_F16,V_CMP_NGE_F16,V_CMP_NLG_F16,V_CMP_NGT_F16,
            case 44:  case 45:  case 46:  case 47:  case 64:   // V_CMP_NLE_F16,V_CMP_NEQ_F16,V_CMP_NLT_F16,V_CMP_TRU_F16,V_CMP_F_F32,
            case 65:  case 66:  case 67:  case 68:  case 69:   // V_CMP_LT_F32,V_CMP_EQ_F32,V_CMP_LE_F32,V_CMP_GT_F32,V_CMP_LG_F32,
            case 70:  case 71:  case 72:  case 73:  case 74:   // V_CMP_GE_F32,V_CMP_O_F32,V_CMP_U_F32,V_CMP_NGE_F32,V_CMP_NLG_F32,
            case 75:  case 76:  case 77:  case 78:   // V_CMP_NGT_F32,V_CMP_NLE_F32,V_CMP_NEQ_F32,V_CMP_NLT_F32,
            case 79:  case 160:  case 161:  case 162:   // V_CMP_TRU_F32,V_CMP_F_I16,V_CMP_LT_I16,V_CMP_EQ_I16,
            case 163:  case 164:  case 165:  case 166:   // V_CMP_LE_I16,V_CMP_GT_I16,V_CMP_NE_I16,V_CMP_GE_I16,
            case 167:  case 168:  case 169:  case 170:  case 171:   // V_CMP_T_I16,V_CMP_F_U16,V_CMP_LT_U16,V_CMP_EQ_U16,V_CMP_LE_U16,
            case 172:  case 173:  case 174:  case 175:  case 192:   // V_CMP_GT_U16,V_CMP_NE_U16,V_CMP_GE_U16,V_CMP_T_U16,V_CMP_F_I32,
            case 193:  case 194:  case 195:  case 196:   // V_CMP_LT_I32,V_CMP_EQ_I32,V_CMP_LE_I32,V_CMP_GT_I32,
            case 197:  case 198:  case 199:  case 200:  case 201:   // V_CMP_NE_I32,V_CMP_GE_I32,V_CMP_T_I32,V_CMP_F_U32,V_CMP_LT_U32,
            case 202:  case 203:  case 204:  case 205:   // V_CMP_EQ_U32,V_CMP_LE_U32,V_CMP_GT_U32,V_CMP_NE_U32,
            case 206:  case 207:   // V_CMP_GE_U32,V_CMP_T_U32,
                appendOPR_SREG(layout.VDST,false,true,2);
                appendOPR_SRC_NOLIT(layout.SRC0,true,false);
                appendOPR_SRC_SIMPLE(layout.SRC1,true,false);
                break;
            case 17:  case 21:  case 48:  case 49:   // V_CMPX_CLASS_F32,V_CMPX_CLASS_F16,V_CMPX_F_F16,V_CMPX_LT_F16,
            case 50:  case 51:  case 52:  case 53:   // V_CMPX_EQ_F16,V_CMPX_LE_F16,V_CMPX_GT_F16,V_CMPX_LG_F16,
            case 54:  case 55:  case 56:  case 57:   // V_CMPX_GE_F16,V_CMPX_O_F16,V_CMPX_U_F16,V_CMPX_NGE_F16,
            case 58:  case 59:  case 60:  case 61:   // V_CMPX_NLG_F16,V_CMPX_NGT_F16,V_CMPX_NLE_F16,V_CMPX_NEQ_F16,
            case 62:  case 63:  case 80:  case 81:   // V_CMPX_NLT_F16,V_CMPX_TRU_F16,V_CMPX_F_F32,V_CMPX_LT_F32,
            case 82:  case 83:  case 84:  case 85:   // V_CMPX_EQ_F32,V_CMPX_LE_F32,V_CMPX_GT_F32,V_CMPX_LG_F32,
            case 86:  case 87:  case 88:  case 89:   // V_CMPX_GE_F32,V_CMPX_O_F32,V_CMPX_U_F32,V_CMPX_NGE_F32,
            case 90:  case 91:  case 92:  case 93:   // V_CMPX_NLG_F32,V_CMPX_NGT_F32,V_CMPX_NLE_F32,V_CMPX_NEQ_F32,
            case 94:  case 95:  case 176:  case 177:   // V_CMPX_NLT_F32,V_CMPX_TRU_F32,V_CMPX_F_I16,V_CMPX_LT_I16,
            case 178:  case 179:  case 180:  case 181:   // V_CMPX_EQ_I16,V_CMPX_LE_I16,V_CMPX_GT_I16,V_CMPX_NE_I16,
            case 182:  case 183:  case 184:  case 185:   // V_CMPX_GE_I16,V_CMPX_T_I16,V_CMPX_F_U16,V_CMPX_LT_U16,
            case 186:  case 187:  case 188:  case 189:   // V_CMPX_EQ_U16,V_CMPX_LE_U16,V_CMPX_GT_U16,V_CMPX_NE_U16,
            case 190:  case 191:  case 208:  case 209:   // V_CMPX_GE_U16,V_CMPX_T_U16,V_CMPX_F_I32,V_CMPX_LT_I32,
            case 210:  case 211:  case 212:  case 213:   // V_CMPX_EQ_I32,V_CMPX_LE_I32,V_CMPX_GT_I32,V_CMPX_NE_I32,
            case 214:  case 215:  case 216:  case 217:   // V_CMPX_GE_I32,V_CMPX_T_I32,V_CMPX_F_U32,V_CMPX_LT_U32,
            case 218:  case 219:  case 220:  case 221:   // V_CMPX_EQ_U32,V_CMPX_LE_U32,V_CMPX_GT_U32,V_CMPX_NE_U32,
            case 222:  case 223:   // V_CMPX_GE_U32,V_CMPX_T_U32,
                appendOPR_SDST(layout.VDST,false,true,2);
                appendOPR_SRC_NOLIT(layout.SRC0,true,false);
                appendOPR_SRC_SIMPLE(layout.SRC1,true,false);
                appendOPR_SDST_EXEC(126,false,true,1,true);
                break;
            case 18:   // V_CMP_CLASS_F64,
                appendOPR_SREG(layout.VDST,false,true,2);
                appendOPR_SRC_NOLIT(layout.SRC0,true,false,2);
                appendOPR_SRC_SIMPLE(layout.SRC1,true,false);
                break;
            case 19:   // V_CMPX_CLASS_F64,
                appendOPR_SDST(layout.VDST,false,true,2);
                appendOPR_SRC_NOLIT(layout.SRC0,true,false,2);
                appendOPR_SRC_SIMPLE(layout.SRC1,true,false);
                appendOPR_SDST_EXEC(126,false,true,1,true);
                break;
            case 96:  case 97:  case 98:  case 99:  case 100:   // V_CMP_F_F64,V_CMP_LT_F64,V_CMP_EQ_F64,V_CMP_LE_F64,V_CMP_GT_F64,
            case 101:  case 102:  case 103:  case 104:   // V_CMP_LG_F64,V_CMP_GE_F64,V_CMP_O_F64,V_CMP_U_F64,
            case 105:  case 106:  case 107:  case 108:   // V_CMP_NGE_F64,V_CMP_NLG_F64,V_CMP_NGT_F64,V_CMP_NLE_F64,
            case 109:  case 110:  case 111:  case 224:   // V_CMP_NEQ_F64,V_CMP_NLT_F64,V_CMP_TRU_F64,V_CMP_F_I64,
            case 225:  case 226:  case 227:  case 228:   // V_CMP_LT_I64,V_CMP_EQ_I64,V_CMP_LE_I64,V_CMP_GT_I64,
            case 229:  case 230:  case 231:  case 232:  case 233:   // V_CMP_NE_I64,V_CMP_GE_I64,V_CMP_T_I64,V_CMP_F_U64,V_CMP_LT_U64,
            case 234:  case 235:  case 236:  case 237:   // V_CMP_EQ_U64,V_CMP_LE_U64,V_CMP_GT_U64,V_CMP_NE_U64,
            case 238:  case 239:   // V_CMP_GE_U64,V_CMP_T_U64,
                appendOPR_SREG(layout.VDST,false,true,2);
                appendOPR_SRC_NOLIT(layout.SRC0,true,false,2);
                appendOPR_SRC_SIMPLE(layout.SRC1,true,false,2);
                break;
            case 112:  case 113:  case 114:  case 115:   // V_CMPX_F_F64,V_CMPX_LT_F64,V_CMPX_EQ_F64,V_CMPX_LE_F64,
            case 116:  case 117:  case 118:  case 119:   // V_CMPX_GT_F64,V_CMPX_LG_F64,V_CMPX_GE_F64,V_CMPX_O_F64,
            case 120:  case 121:  case 122:  case 123:   // V_CMPX_U_F64,V_CMPX_NGE_F64,V_CMPX_NLG_F64,V_CMPX_NGT_F64,
            case 124:  case 125:  case 126:  case 127:   // V_CMPX_NLE_F64,V_CMPX_NEQ_F64,V_CMPX_NLT_F64,V_CMPX_TRU_F64,
            case 240:  case 241:  case 242:  case 243:   // V_CMPX_F_I64,V_CMPX_LT_I64,V_CMPX_EQ_I64,V_CMPX_LE_I64,
            case 244:  case 245:  case 246:  case 247:   // V_CMPX_GT_I64,V_CMPX_NE_I64,V_CMPX_GE_I64,V_CMPX_T_I64,
            case 248:  case 249:  case 250:  case 251:   // V_CMPX_F_U64,V_CMPX_LT_U64,V_CMPX_EQ_U64,V_CMPX_LE_U64,
            case 252:  case 253:  case 254:  case 255:   // V_CMPX_GT_U64,V_CMPX_NE_U64,V_CMPX_GE_U64,V_CMPX_T_U64,
                appendOPR_SDST(layout.VDST,false,true,2);
                appendOPR_SRC_NOLIT(layout.SRC0,true,false,2);
                appendOPR_SRC_SIMPLE(layout.SRC1,true,false,2);
                appendOPR_SDST_EXEC(126,false,true,1,true);
                break;
        }
    }

    void InstructionDecoder_amdgpu_gfx90a::finalizeENC_VOP2Operands()
    {
        layout_ENC_VOP2 & layout = insn_layout.ENC_VOP2;
        switch (layout.OP)  {
            case 0:   // V_CNDMASK_B32,
                appendOPR_VGPR(layout.VDST,false,true);
                appendOPR_SRC(layout.SRC0,true,false);
                appendOPR_VGPR(layout.VSRC1,true,false);
                appendOPR_VCC(0,true,false,2);
                break;
            case 1:  case 2:  case 3:  case 5:  case 6:   // V_ADD_F32,V_SUB_F32,V_SUBREV_F32,V_MUL_F32,V_MUL_I32_I24,
            case 7:  case 8:  case 9:  case 10:  case 11:   // V_MUL_HI_I32_I24,V_MUL_U32_U24,V_MUL_HI_U32_U24,V_MIN_F32,V_MAX_F32,
            case 12:  case 13:  case 14:  case 15:  case 19:   // V_MIN_I32,V_MAX_I32,V_MIN_U32,V_MAX_U32,V_AND_B32,
            case 20:  case 21:  case 31:  case 32:  case 34:   // V_OR_B32,V_XOR_B32,V_ADD_F16,V_SUB_F16,V_MUL_F16,
            case 38:  case 39:  case 41:  case 45:  case 46:   // V_ADD_U16,V_SUB_U16,V_MUL_LO_U16,V_MAX_F16,V_MIN_F16,
            case 47:  case 48:  case 49:  case 50:  case 51:   // V_MAX_U16,V_MAX_I16,V_MIN_U16,V_MIN_I16,V_LDEXP_F16,
            case 52:  case 53:  case 61:   // V_ADD_U32,V_SUB_U32,V_XNOR_B32,
                appendOPR_VGPR(layout.VDST,false,true);
                appendOPR_SRC(layout.SRC0,true,false);
                appendOPR_VGPR(layout.VSRC1,true,false);
                break;
            case 4:   // V_FMAC_F64,
                appendOPR_VGPR(layout.VDST,true,true,2);
                appendOPR_SRC(layout.SRC0,true,false,2);
                appendOPR_VGPR(layout.VSRC1,true,false,2);
                break;
            case 16:  case 17:  case 18:  case 33:  case 40:   // V_LSHRREV_B32,V_ASHRREV_I32,V_LSHLREV_B32,V_SUBREV_F16,V_SUBREV_U16,
            case 42:  case 43:  case 44:  case 54:   // V_LSHLREV_B16,V_LSHRREV_B16,V_ASHRREV_I16,V_SUBREV_U32,
                appendOPR_VGPR(layout.VDST,false,true);
                appendOPR_SRC_NOLDS(layout.SRC0,true,false);
                appendOPR_VGPR(layout.VSRC1,true,false);
                break;
            case 22:  case 35:  case 55:  case 56:  case 57:   // V_MAC_F32,V_MAC_F16,V_DOT2C_F32_F16,V_DOT2C_I32_I16,V_DOT4C_I32_I8,
            case 58:  case 59:  case 60:   // V_DOT8C_I32_I4,V_FMAC_F32,V_PK_FMAC_F16,
                appendOPR_VGPR(layout.VDST,true,true);
                appendOPR_SRC(layout.SRC0,true,false);
                appendOPR_VGPR(layout.VSRC1,true,false);
                break;
            case 25:  case 26:   // V_ADD_CO_U32,V_SUB_CO_U32,
                appendOPR_VGPR(layout.VDST,false,true);
                appendOPR_VCC(0,false,true,2);
                appendOPR_SRC(layout.SRC0,true,false);
                appendOPR_VGPR(layout.VSRC1,true,false);
                break;
            case 27:   // V_SUBREV_CO_U32,
                appendOPR_VGPR(layout.VDST,false,true);
                appendOPR_VCC(0,false,true,2);
                appendOPR_SRC_NOLDS(layout.SRC0,true,false);
                appendOPR_VGPR(layout.VSRC1,true,false);
                break;
            case 28:  case 29:   // V_ADDC_CO_U32,V_SUBB_CO_U32,
                appendOPR_VGPR(layout.VDST,false,true);
                appendOPR_VCC(0,false,true,2);
                appendOPR_SRC(layout.SRC0,true,false);
                appendOPR_VGPR(layout.VSRC1,true,false);
                appendOPR_VCC(0,true,false,2);
                break;
            case 30:   // V_SUBBREV_CO_U32,
                appendOPR_VGPR(layout.VDST,false,true);
                appendOPR_VCC(0,false,true,2);
                appendOPR_SRC_NOLDS(layout.SRC0,true,false);
                appendOPR_VGPR(layout.VSRC1,true,false);
                appendOPR_VCC(0,true,false,2);
                break;
        }
    }

    void InstructionDecoder_amdgpu_gfx90a::finalizeENC_VOP2_LITERALOperands()
    {
        layout_ENC_VOP2_LITERAL & layout = insn_layout.ENC_VOP2_LITERAL;
        switch (layout.OP)  {
            case 23:  case 36:   // V_MADMK_F32,V_MADMK_F16,
                appendOPR_VGPR(layout.VDST,false,true);
                appendOPR_SRC(layout.SRC0,true,false);
                appendOPR_SIMM32(layout.SIMM32,true,false);
                appendOPR_VGPR(layout.VSRC1,true,false);
                break;
            case 24:  case 37:   // V_MADAK_F32,V_MADAK_F16,
                appendOPR_VGPR(layout.VDST,false,true);
                appendOPR_SRC(layout.SRC0,true,false);
                appendOPR_VGPR(layout.VSRC1,true,false);
                appendOPR_SIMM32(layout.SIMM32,true,false);
                break;
        }
    }

    void InstructionDecoder_amdgpu_gfx90a::finalizeENC_VOP3BOperands()
    {
        layout_ENC_VOP3B & layout = insn_layout.ENC_VOP3B;
        switch (layout.OP)  {
            case 281:  case 282:   // V_ADD_CO_U32,V_SUB_CO_U32,
                appendOPR_VGPR(layout.VDST,false,true);
                appendOPR_SREG(layout.SDST,false,true,2);
                appendOPR_SRC_NOLIT(layout.SRC0,true,false);
                appendOPR_SRC_SIMPLE(layout.SRC1,true,false);
                break;
            case 283:   // V_SUBREV_CO_U32,
                appendOPR_VGPR(layout.VDST,false,true);
                appendOPR_SREG(layout.SDST,false,true,2);
                appendOPR_SRC_SIMPLE(layout.SRC0,true,false);
                appendOPR_SRC_SIMPLE(layout.SRC1,true,false);
                break;
            case 284:  case 285:   // V_ADDC_CO_U32,V_SUBB_CO_U32,
                appendOPR_VGPR(layout.VDST,false,true);
                appendOPR_SREG(layout.SDST,false,true,2);
                appendOPR_SRC_NOLIT(layout.SRC0,true,false);
                appendOPR_SRC_SIMPLE(layout.SRC1,true,false);
                appendOPR_SREG(layout.SRC2,true,false,2);
                break;
            case 286:   // V_SUBBREV_CO_U32,
                appendOPR_VGPR(layout.VDST,false,true);
                appendOPR_SREG(layout.SDST,false,true,2);
                appendOPR_SRC_SIMPLE(layout.SRC0,true,false);
                appendOPR_SRC_SIMPLE(layout.SRC1,true,false);
                appendOPR_SREG(layout.SRC2,true,false,2);
                break;
            case 480:   // V_DIV_SCALE_F32,
                appendOPR_VGPR(layout.VDST,false,true);
                appendOPR_VCC(layout.SDST,false,true,2);
                appendOPR_SRC_NOLIT(layout.SRC0,true,false);
                appendOPR_SRC_SIMPLE(layout.SRC1,true,false);
                appendOPR_SRC_SIMPLE(layout.SRC2,true,false);
                break;
            case 481:   // V_DIV_SCALE_F64,
                appendOPR_VGPR(layout.VDST,false,true,2);
                appendOPR_VCC(layout.SDST,false,true,2);
                appendOPR_SRC_NOLIT(layout.SRC0,true,false,2);
                appendOPR_SRC_SIMPLE(layout.SRC1,true,false,2);
                appendOPR_SRC_SIMPLE(layout.SRC2,true,false,2);
                break;
            case 488:  case 489:   // V_MAD_U64_U32,V_MAD_I64_I32,
                appendOPR_VGPR(layout.VDST,false,true,2);
                appendOPR_SREG(layout.SDST,false,true,2);
                appendOPR_SRC_NOLIT(layout.SRC0,true,false);
                appendOPR_SRC_SIMPLE(layout.SRC1,true,false);
                appendOPR_SRC_SIMPLE(layout.SRC2,true,false,2);
                break;
        }
    }

    void InstructionDecoder_amdgpu_gfx90a::finalizeENC_VOP3POperands()
    {
        layout_ENC_VOP3P & layout = insn_layout.ENC_VOP3P;
        switch (layout.OP)  {
            case 0:  case 9:  case 14:  case 32:  case 33:   // V_PK_MAD_I16,V_PK_MAD_U16,V_PK_FMA_F16,V_MAD_MIX_F32,V_MAD_MIXLO_F16,
            case 34:  case 35:  case 38:  case 39:   // V_MAD_MIXHI_F16,V_DOT2_F32_F16,V_DOT2_I32_I16,V_DOT2_U32_U16,
            case 40:  case 41:  case 42:  case 43:   // V_DOT4_I32_I8,V_DOT4_U32_U8,V_DOT8_I32_I4,V_DOT8_U32_U4,
            case 48:   // V_PK_FMA_F32,
                appendOPR_VGPR(layout.VDST,false,true);
                appendOPR_SRC_NOLIT(layout.SRC0,true,false);
                appendOPR_SRC_SIMPLE(layout.SRC1,true,false);
                appendOPR_SRC_SIMPLE(layout.SRC2,true,false);
                break;
            case 1:  case 2:  case 3:  case 7:  case 8:   // V_PK_MUL_LO_U16,V_PK_ADD_I16,V_PK_SUB_I16,V_PK_MAX_I16,V_PK_MIN_I16,
            case 10:  case 11:  case 12:  case 13:  case 15:   // V_PK_ADD_U16,V_PK_SUB_U16,V_PK_MAX_U16,V_PK_MIN_U16,V_PK_ADD_F16,
            case 16:  case 17:  case 18:  case 49:  case 50:   // V_PK_MUL_F16,V_PK_MIN_F16,V_PK_MAX_F16,V_PK_MUL_F32,V_PK_ADD_F32,
                appendOPR_VGPR(layout.VDST,false,true);
                appendOPR_SRC_NOLIT(layout.SRC0,true,false);
                appendOPR_SRC_SIMPLE(layout.SRC1,true,false);
                break;
            case 4:  case 5:  case 6:   // V_PK_LSHLREV_B16,V_PK_LSHRREV_B16,V_PK_ASHRREV_I16,
                appendOPR_VGPR(layout.VDST,false,true);
                appendOPR_SRC_SIMPLE(layout.SRC0,true,false);
                appendOPR_SRC_SIMPLE(layout.SRC1,true,false);
                break;
            case 51:   // V_PK_MOV_B32,
                appendOPR_VGPR(layout.VDST,false,true,2);
                appendOPR_SRC_NOLIT(layout.SRC0,true,false,2);
                appendOPR_SRC_SIMPLE(layout.SRC1,true,false,2);
                break;
            case 88:   // V_ACCVGPR_READ,
                appendOPR_VGPR(layout.VDST,false,true);
                appendOPR_SRC_ACCVGPR(layout.SRC0,true,false);
                break;
            case 89:   // V_ACCVGPR_WRITE,
                appendOPR_ACCVGPR(layout.VDST,false,true);
                appendOPR_SRC_NOLIT(layout.SRC0,true,false);
                break;
        }
    }

    void InstructionDecoder_amdgpu_gfx90a::finalizeENC_VOP3P_MFMAOperands()
    {
        layout_ENC_VOP3P_MFMA & layout = insn_layout.ENC_VOP3P_MFMA;
        switch (layout.OP)  {
            case 64:  case 80:  case 104:   // V_MFMA_F32_32X32X1F32,V_MFMA_I32_32X32X4I8,V_MFMA_F32_32X32X2BF16,
                appendOPR_VGPR_OR_ACCVGPR(layout.VDST,false,true,32);
                appendOPR_SRC_VGPR_OR_ACCVGPR(layout.SRC0,true,false);
                appendOPR_SRC_VGPR_OR_ACCVGPR(layout.SRC1,true,false);
                appendOPR_SRC_VGPR_OR_ACCVGPR_OR_CONST(layout.SRC2,true,false,32);
                break;
            case 65:  case 68:  case 81:   // V_MFMA_F32_16X16X1F32,V_MFMA_F32_32X32X2F32,V_MFMA_I32_16X16X4I8,
            case 84:  case 105:  case 108:   // V_MFMA_I32_32X32X8I8,V_MFMA_F32_16X16X2BF16,V_MFMA_F32_32X32X4BF16,
                appendOPR_VGPR_OR_ACCVGPR(layout.VDST,false,true,16);
                appendOPR_SRC_VGPR_OR_ACCVGPR(layout.SRC0,true,false);
                appendOPR_SRC_VGPR_OR_ACCVGPR(layout.SRC1,true,false);
                appendOPR_SRC_VGPR_OR_ACCVGPR_OR_CONST(layout.SRC2,true,false,16);
                break;
            case 66:  case 69:  case 82:   // V_MFMA_F32_4X4X1F32,V_MFMA_F32_16X16X4F32,V_MFMA_I32_4X4X4I8,
            case 85:  case 107:  case 109:   // V_MFMA_I32_16X16X16I8,V_MFMA_F32_4X4X2BF16,V_MFMA_F32_16X16X8BF16,
                appendOPR_VGPR_OR_ACCVGPR(layout.VDST,false,true,4);
                appendOPR_SRC_VGPR_OR_ACCVGPR(layout.SRC0,true,false);
                appendOPR_SRC_VGPR_OR_ACCVGPR(layout.SRC1,true,false);
                appendOPR_SRC_VGPR_OR_ACCVGPR_OR_CONST(layout.SRC2,true,false,4);
                break;
            case 72:  case 99:   // V_MFMA_F32_32X32X4F16,V_MFMA_F32_32X32X4BF16_1K,
                appendOPR_VGPR_OR_ACCVGPR(layout.VDST,false,true,32);
                appendOPR_SRC_VGPR_OR_ACCVGPR(layout.SRC0,true,false,2);
                appendOPR_SRC_VGPR_OR_ACCVGPR(layout.SRC1,true,false,2);
                appendOPR_SRC_VGPR_OR_ACCVGPR_OR_CONST(layout.SRC2,true,false,32);
                break;
            case 73:  case 76:  case 100:   // V_MFMA_F32_16X16X4F16,V_MFMA_F32_32X32X8F16,V_MFMA_F32_16X16X4BF16_1K,
            case 102:   // V_MFMA_F32_32X32X8BF16_1K,
                appendOPR_VGPR_OR_ACCVGPR(layout.VDST,false,true,16);
                appendOPR_SRC_VGPR_OR_ACCVGPR(layout.SRC0,true,false,2);
                appendOPR_SRC_VGPR_OR_ACCVGPR(layout.SRC1,true,false,2);
                appendOPR_SRC_VGPR_OR_ACCVGPR_OR_CONST(layout.SRC2,true,false,16);
                break;
            case 74:  case 77:  case 101:   // V_MFMA_F32_4X4X4F16,V_MFMA_F32_16X16X16F16,V_MFMA_F32_4X4X4BF16_1K,
            case 103:   // V_MFMA_F32_16X16X16BF16_1K,
                appendOPR_VGPR_OR_ACCVGPR(layout.VDST,false,true,4);
                appendOPR_SRC_VGPR_OR_ACCVGPR(layout.SRC0,true,false,2);
                appendOPR_SRC_VGPR_OR_ACCVGPR(layout.SRC1,true,false,2);
                appendOPR_SRC_VGPR_OR_ACCVGPR_OR_CONST(layout.SRC2,true,false,4);
                break;
            case 110:   // V_MFMA_F64_16X16X4F64,
                appendOPR_VGPR_OR_ACCVGPR(layout.VDST,false,true,8);
                appendOPR_SRC_VGPR_OR_ACCVGPR(layout.SRC0,true,false,2);
                appendOPR_SRC_VGPR_OR_ACCVGPR(layout.SRC1,true,false,2);
                appendOPR_SRC_VGPR_OR_ACCVGPR_OR_CONST(layout.SRC2,true,false,8);
                break;
            case 111:   // V_MFMA_F64_4X4X4F64,
                appendOPR_VGPR_OR_ACCVGPR(layout.VDST,false,true,2);
                appendOPR_SRC_VGPR_OR_ACCVGPR(layout.SRC0,true,false,2);
                appendOPR_SRC_VGPR_OR_ACCVGPR(layout.SRC1,true,false,2);
                appendOPR_SRC_VGPR_OR_ACCVGPR_OR_CONST(layout.SRC2,true,false,2);
                break;
        }
    }

    void InstructionDecoder_amdgpu_gfx90a::finalizeENC_VOPCOperands()
    {
        layout_ENC_VOPC & layout = insn_layout.ENC_VOPC;
        switch (layout.OP)  {
            case 16:  case 20:  case 32:  case 33:   // V_CMP_CLASS_F32,V_CMP_CLASS_F16,V_CMP_F_F16,V_CMP_LT_F16,
            case 34:  case 35:  case 36:  case 37:  case 38:   // V_CMP_EQ_F16,V_CMP_LE_F16,V_CMP_GT_F16,V_CMP_LG_F16,V_CMP_GE_F16,
            case 39:  case 40:  case 41:  case 42:  case 43:   // V_CMP_O_F16,V_CMP_U_F16,V_CMP_NGE_F16,V_CMP_NLG_F16,V_CMP_NGT_F16,
            case 44:  case 45:  case 46:  case 47:  case 64:   // V_CMP_NLE_F16,V_CMP_NEQ_F16,V_CMP_NLT_F16,V_CMP_TRU_F16,V_CMP_F_F32,
            case 65:  case 66:  case 67:  case 68:  case 69:   // V_CMP_LT_F32,V_CMP_EQ_F32,V_CMP_LE_F32,V_CMP_GT_F32,V_CMP_LG_F32,
            case 70:  case 71:  case 72:  case 73:  case 74:   // V_CMP_GE_F32,V_CMP_O_F32,V_CMP_U_F32,V_CMP_NGE_F32,V_CMP_NLG_F32,
            case 75:  case 76:  case 77:  case 78:   // V_CMP_NGT_F32,V_CMP_NLE_F32,V_CMP_NEQ_F32,V_CMP_NLT_F32,
            case 79:  case 160:  case 161:  case 162:   // V_CMP_TRU_F32,V_CMP_F_I16,V_CMP_LT_I16,V_CMP_EQ_I16,
            case 163:  case 164:  case 165:  case 166:   // V_CMP_LE_I16,V_CMP_GT_I16,V_CMP_NE_I16,V_CMP_GE_I16,
            case 167:  case 168:  case 169:  case 170:  case 171:   // V_CMP_T_I16,V_CMP_F_U16,V_CMP_LT_U16,V_CMP_EQ_U16,V_CMP_LE_U16,
            case 172:  case 173:  case 174:  case 175:  case 192:   // V_CMP_GT_U16,V_CMP_NE_U16,V_CMP_GE_U16,V_CMP_T_U16,V_CMP_F_I32,
            case 193:  case 194:  case 195:  case 196:   // V_CMP_LT_I32,V_CMP_EQ_I32,V_CMP_LE_I32,V_CMP_GT_I32,
            case 197:  case 198:  case 199:  case 200:  case 201:   // V_CMP_NE_I32,V_CMP_GE_I32,V_CMP_T_I32,V_CMP_F_U32,V_CMP_LT_U32,
            case 202:  case 203:  case 204:  case 205:   // V_CMP_EQ_U32,V_CMP_LE_U32,V_CMP_GT_U32,V_CMP_NE_U32,
            case 206:  case 207:   // V_CMP_GE_U32,V_CMP_T_U32,
                appendOPR_VCC(0,false,true,2);
                appendOPR_SRC(layout.SRC0,true,false);
                appendOPR_VGPR(layout.VSRC1,true,false);
                break;
            case 17:  case 21:  case 48:  case 49:   // V_CMPX_CLASS_F32,V_CMPX_CLASS_F16,V_CMPX_F_F16,V_CMPX_LT_F16,
            case 50:  case 51:  case 52:  case 53:   // V_CMPX_EQ_F16,V_CMPX_LE_F16,V_CMPX_GT_F16,V_CMPX_LG_F16,
            case 54:  case 55:  case 56:  case 57:   // V_CMPX_GE_F16,V_CMPX_O_F16,V_CMPX_U_F16,V_CMPX_NGE_F16,
            case 58:  case 59:  case 60:  case 61:   // V_CMPX_NLG_F16,V_CMPX_NGT_F16,V_CMPX_NLE_F16,V_CMPX_NEQ_F16,
            case 62:  case 63:  case 80:  case 81:   // V_CMPX_NLT_F16,V_CMPX_TRU_F16,V_CMPX_F_F32,V_CMPX_LT_F32,
            case 82:  case 83:  case 84:  case 85:   // V_CMPX_EQ_F32,V_CMPX_LE_F32,V_CMPX_GT_F32,V_CMPX_LG_F32,
            case 86:  case 87:  case 88:  case 89:   // V_CMPX_GE_F32,V_CMPX_O_F32,V_CMPX_U_F32,V_CMPX_NGE_F32,
            case 90:  case 91:  case 92:  case 93:   // V_CMPX_NLG_F32,V_CMPX_NGT_F32,V_CMPX_NLE_F32,V_CMPX_NEQ_F32,
            case 94:  case 95:  case 176:  case 177:   // V_CMPX_NLT_F32,V_CMPX_TRU_F32,V_CMPX_F_I16,V_CMPX_LT_I16,
            case 178:  case 179:  case 180:  case 181:   // V_CMPX_EQ_I16,V_CMPX_LE_I16,V_CMPX_GT_I16,V_CMPX_NE_I16,
            case 182:  case 183:  case 184:  case 185:   // V_CMPX_GE_I16,V_CMPX_T_I16,V_CMPX_F_U16,V_CMPX_LT_U16,
            case 186:  case 187:  case 188:  case 189:   // V_CMPX_EQ_U16,V_CMPX_LE_U16,V_CMPX_GT_U16,V_CMPX_NE_U16,
            case 190:  case 191:  case 208:  case 209:   // V_CMPX_GE_U16,V_CMPX_T_U16,V_CMPX_F_I32,V_CMPX_LT_I32,
            case 210:  case 211:  case 212:  case 213:   // V_CMPX_EQ_I32,V_CMPX_LE_I32,V_CMPX_GT_I32,V_CMPX_NE_I32,
            case 214:  case 215:  case 216:  case 217:   // V_CMPX_GE_I32,V_CMPX_T_I32,V_CMPX_F_U32,V_CMPX_LT_U32,
            case 218:  case 219:  case 220:  case 221:   // V_CMPX_EQ_U32,V_CMPX_LE_U32,V_CMPX_GT_U32,V_CMPX_NE_U32,
            case 222:  case 223:   // V_CMPX_GE_U32,V_CMPX_T_U32,
                appendOPR_VCC(0,false,true,2);
                appendOPR_SRC(layout.SRC0,true,false);
                appendOPR_VGPR(layout.VSRC1,true,false);
                appendOPR_SDST_EXEC(126,false,true,1,true);
                break;
            case 18:   // V_CMP_CLASS_F64,
                appendOPR_VCC(0,false,true,2);
                appendOPR_SRC(layout.SRC0,true,false,2);
                appendOPR_VGPR(layout.VSRC1,true,false);
                break;
            case 19:   // V_CMPX_CLASS_F64,
                appendOPR_VCC(0,false,true,2);
                appendOPR_SRC(layout.SRC0,true,false,2);
                appendOPR_VGPR(layout.VSRC1,true,false);
                appendOPR_SDST_EXEC(126,false,true,1,true);
                break;
            case 96:  case 97:  case 98:  case 99:  case 100:   // V_CMP_F_F64,V_CMP_LT_F64,V_CMP_EQ_F64,V_CMP_LE_F64,V_CMP_GT_F64,
            case 101:  case 102:  case 103:  case 104:   // V_CMP_LG_F64,V_CMP_GE_F64,V_CMP_O_F64,V_CMP_U_F64,
            case 105:  case 106:  case 107:  case 108:   // V_CMP_NGE_F64,V_CMP_NLG_F64,V_CMP_NGT_F64,V_CMP_NLE_F64,
            case 109:  case 110:  case 111:  case 224:   // V_CMP_NEQ_F64,V_CMP_NLT_F64,V_CMP_TRU_F64,V_CMP_F_I64,
            case 225:  case 226:  case 227:  case 228:   // V_CMP_LT_I64,V_CMP_EQ_I64,V_CMP_LE_I64,V_CMP_GT_I64,
            case 229:  case 230:  case 231:  case 232:  case 233:   // V_CMP_NE_I64,V_CMP_GE_I64,V_CMP_T_I64,V_CMP_F_U64,V_CMP_LT_U64,
            case 234:  case 235:  case 236:  case 237:   // V_CMP_EQ_U64,V_CMP_LE_U64,V_CMP_GT_U64,V_CMP_NE_U64,
            case 238:  case 239:   // V_CMP_GE_U64,V_CMP_T_U64,
                appendOPR_VCC(0,false,true,2);
                appendOPR_SRC(layout.SRC0,true,false,2);
                appendOPR_VGPR(layout.VSRC1,true,false,2);
                break;
            case 112:  case 113:  case 114:  case 115:   // V_CMPX_F_F64,V_CMPX_LT_F64,V_CMPX_EQ_F64,V_CMPX_LE_F64,
            case 116:  case 117:  case 118:  case 119:   // V_CMPX_GT_F64,V_CMPX_LG_F64,V_CMPX_GE_F64,V_CMPX_O_F64,
            case 120:  case 121:  case 122:  case 123:   // V_CMPX_U_F64,V_CMPX_NGE_F64,V_CMPX_NLG_F64,V_CMPX_NGT_F64,
            case 124:  case 125:  case 126:  case 127:   // V_CMPX_NLE_F64,V_CMPX_NEQ_F64,V_CMPX_NLT_F64,V_CMPX_TRU_F64,
            case 240:  case 241:  case 242:  case 243:   // V_CMPX_F_I64,V_CMPX_LT_I64,V_CMPX_EQ_I64,V_CMPX_LE_I64,
            case 244:  case 245:  case 246:  case 247:   // V_CMPX_GT_I64,V_CMPX_NE_I64,V_CMPX_GE_I64,V_CMPX_T_I64,
            case 248:  case 249:  case 250:  case 251:   // V_CMPX_F_U64,V_CMPX_LT_U64,V_CMPX_EQ_U64,V_CMPX_LE_U64,
            case 252:  case 253:  case 254:  case 255:   // V_CMPX_GT_U64,V_CMPX_NE_U64,V_CMPX_GE_U64,V_CMPX_T_U64,
                appendOPR_VCC(0,false,true,2);
                appendOPR_SRC(layout.SRC0,true,false,2);
                appendOPR_VGPR(layout.VSRC1,true,false,2);
                appendOPR_SDST_EXEC(126,false,true,1,true);
                break;
        }
    }

void InstructionDecoder_amdgpu_gfx90a::finalizeENC_VINTRPOperands()  {
}

}
}
