#include <memory>
#include "InstructionDecoder-amdgpu-gfx908.h"

namespace Dyninst {
namespace InstructionAPI {
    void InstructionDecoder_amdgpu_gfx908::appendOPR_SIMM4(uint64_t input, bool isRead, bool isWritten, uint32_t /*_num_elements = 1*/ , bool isImplicit /*= false*/)
    {
        insn_in_progress->appendOperand(Immediate::makeImmediate(Result(s8, input)),isRead,isWritten,isImplicit);
    }

    void InstructionDecoder_amdgpu_gfx908::appendOPR_SIMM8(uint64_t input, bool isRead, bool isWritten, uint32_t /*_num_elements = 1*/ , bool isImplicit /*= false*/)
    {
        insn_in_progress->appendOperand(Immediate::makeImmediate(Result(s8, input)),isRead,isWritten,isImplicit);
    }

    void InstructionDecoder_amdgpu_gfx908::appendOPR_SIMM16(uint64_t input, bool isRead, bool isWritten, uint32_t /*_num_elements = 1*/ , bool isImplicit /*= false*/)
    {
        insn_in_progress->appendOperand(Immediate::makeImmediate(Result(s16, input)),isRead,isWritten,isImplicit);
    }

    void InstructionDecoder_amdgpu_gfx908::appendOPR_SIMM32(uint64_t input, bool isRead, bool isWritten, uint32_t /*_num_elements = 1*/ , bool isImplicit /*= false*/)
    {
        insn_in_progress->appendOperand(Immediate::makeImmediate(Result(s32, input)),isRead,isWritten,isImplicit);
    }

    void InstructionDecoder_amdgpu_gfx908::appendOPR_WAITCNT(uint64_t input, bool isRead, bool isWritten, uint32_t /*_num_elements = 1*/ , bool isImplicit /*= false*/)
    {
        insn_in_progress->appendOperand(Immediate::makeImmediate(Result(s16, input)),isRead,isWritten,isImplicit);
    }

    void InstructionDecoder_amdgpu_gfx908::appendOPR_ACCVGPR(uint64_t input, bool isRead, bool isWritten, uint32_t vec_len /*= 1*/ , bool isImplicit /*= false*/)
    {
        Expression::Ptr first = decodeOPR_ACCVGPR(input,vec_len);
        insn_in_progress->appendOperand(first,isRead,isWritten,isImplicit);
        if (boost::dynamic_pointer_cast<RegisterAST::Ptr>(first) != NULL)
        {
            for (uint32_t vec_id = 1; vec_id < vec_len; vec_id++)
            {
                insn_in_progress->appendOperand(decodeOPR_ACCVGPR(input+vec_id,0),isRead,isWritten,isImplicit);
            }
        }
    }

    void InstructionDecoder_amdgpu_gfx908::appendOPR_ATTR(uint64_t input, bool isRead, bool isWritten, uint32_t vec_len /*= 1*/ , bool isImplicit /*= false*/)
    {
        Expression::Ptr first = decodeOPR_ATTR(input,vec_len);
        insn_in_progress->appendOperand(first,isRead,isWritten,isImplicit);
        if (boost::dynamic_pointer_cast<RegisterAST::Ptr>(first) != NULL)
        {
            for (uint32_t vec_id = 1; vec_id < vec_len; vec_id++)
            {
                insn_in_progress->appendOperand(decodeOPR_ATTR(input+vec_id,0),isRead,isWritten,isImplicit);
            }
        }
    }

    void InstructionDecoder_amdgpu_gfx908::appendOPR_DSMEM(uint64_t input, bool isRead, bool isWritten, uint32_t vec_len /*= 1*/ , bool isImplicit /*= false*/)
    {
        Expression::Ptr first = decodeOPR_DSMEM(input,vec_len);
        insn_in_progress->appendOperand(first,isRead,isWritten,isImplicit);
        if (boost::dynamic_pointer_cast<RegisterAST::Ptr>(first) != NULL)
        {
            for (uint32_t vec_id = 1; vec_id < vec_len; vec_id++)
            {
                insn_in_progress->appendOperand(decodeOPR_DSMEM(input+vec_id,0),isRead,isWritten,isImplicit);
            }
        }
    }

    void InstructionDecoder_amdgpu_gfx908::appendOPR_FLAT_SCRATCH(uint64_t input, bool isRead, bool isWritten, uint32_t vec_len /*= 1*/ , bool isImplicit /*= false*/)
    {
        Expression::Ptr first = decodeOPR_FLAT_SCRATCH(input,vec_len);
        insn_in_progress->appendOperand(first,isRead,isWritten,isImplicit);
        if (boost::dynamic_pointer_cast<RegisterAST::Ptr>(first) != NULL)
        {
            for (uint32_t vec_id = 1; vec_id < vec_len; vec_id++)
            {
                insn_in_progress->appendOperand(decodeOPR_FLAT_SCRATCH(input+vec_id,0),isRead,isWritten,isImplicit);
            }
        }
    }

    void InstructionDecoder_amdgpu_gfx908::appendOPR_PARAM(uint64_t input, bool isRead, bool isWritten, uint32_t vec_len /*= 1*/ , bool isImplicit /*= false*/)
    {
        Expression::Ptr first = decodeOPR_PARAM(input,vec_len);
        insn_in_progress->appendOperand(first,isRead,isWritten,isImplicit);
        if (boost::dynamic_pointer_cast<RegisterAST::Ptr>(first) != NULL)
        {
            for (uint32_t vec_id = 1; vec_id < vec_len; vec_id++)
            {
                insn_in_progress->appendOperand(decodeOPR_PARAM(input+vec_id,0),isRead,isWritten,isImplicit);
            }
        }
    }

    void InstructionDecoder_amdgpu_gfx908::appendOPR_PC(uint64_t input, bool isRead, bool isWritten, uint32_t vec_len /*= 1*/ , bool isImplicit /*= false*/)
    {
        Expression::Ptr first = decodeOPR_PC(input,vec_len);
        insn_in_progress->appendOperand(first,isRead,isWritten,isImplicit);
        if (boost::dynamic_pointer_cast<RegisterAST::Ptr>(first) != NULL)
        {
            for (uint32_t vec_id = 1; vec_id < vec_len; vec_id++)
            {
                insn_in_progress->appendOperand(decodeOPR_PC(input+vec_id,0),isRead,isWritten,isImplicit);
            }
        }
    }

    void InstructionDecoder_amdgpu_gfx908::appendOPR_SDST(uint64_t input, bool isRead, bool isWritten, uint32_t vec_len /*= 1*/ , bool isImplicit /*= false*/)
    {
        Expression::Ptr first = decodeOPR_SDST(input,vec_len);
        insn_in_progress->appendOperand(first,isRead,isWritten,isImplicit);
        if (boost::dynamic_pointer_cast<RegisterAST::Ptr>(first) != NULL)
        {
            for (uint32_t vec_id = 1; vec_id < vec_len; vec_id++)
            {
                insn_in_progress->appendOperand(decodeOPR_SDST(input+vec_id,0),isRead,isWritten,isImplicit);
            }
        }
    }

    void InstructionDecoder_amdgpu_gfx908::appendOPR_SDST_EXEC(uint64_t input, bool isRead, bool isWritten, uint32_t vec_len /*= 1*/ , bool isImplicit /*= false*/)
    {
        Expression::Ptr first = decodeOPR_SDST_EXEC(input,vec_len);
        insn_in_progress->appendOperand(first,isRead,isWritten,isImplicit);
        if (boost::dynamic_pointer_cast<RegisterAST::Ptr>(first) != NULL)
        {
            for (uint32_t vec_id = 1; vec_id < vec_len; vec_id++)
            {
                insn_in_progress->appendOperand(decodeOPR_SDST_EXEC(input+vec_id,0),isRead,isWritten,isImplicit);
            }
        }
    }

    void InstructionDecoder_amdgpu_gfx908::appendOPR_SDST_M0(uint64_t input, bool isRead, bool isWritten, uint32_t vec_len /*= 1*/ , bool isImplicit /*= false*/)
    {
        Expression::Ptr first = decodeOPR_SDST_M0(input,vec_len);
        insn_in_progress->appendOperand(first,isRead,isWritten,isImplicit);
        if (boost::dynamic_pointer_cast<RegisterAST::Ptr>(first) != NULL)
        {
            for (uint32_t vec_id = 1; vec_id < vec_len; vec_id++)
            {
                insn_in_progress->appendOperand(decodeOPR_SDST_M0(input+vec_id,0),isRead,isWritten,isImplicit);
            }
        }
    }

    void InstructionDecoder_amdgpu_gfx908::appendOPR_SRC(uint64_t input, bool isRead, bool isWritten, uint32_t vec_len /*= 1*/ , bool isImplicit /*= false*/)
    {
        Expression::Ptr first = decodeOPR_SRC(input,vec_len);
        insn_in_progress->appendOperand(first,isRead,isWritten,isImplicit);
        if (boost::dynamic_pointer_cast<RegisterAST::Ptr>(first) != NULL)
        {
            for (uint32_t vec_id = 1; vec_id < vec_len; vec_id++)
            {
                insn_in_progress->appendOperand(decodeOPR_SRC(input+vec_id,0),isRead,isWritten,isImplicit);
            }
        }
    }

    void InstructionDecoder_amdgpu_gfx908::appendOPR_SRC_ACCVGPR(uint64_t input, bool isRead, bool isWritten, uint32_t vec_len /*= 1*/ , bool isImplicit /*= false*/)
    {
        Expression::Ptr first = decodeOPR_SRC_ACCVGPR(input,vec_len);
        insn_in_progress->appendOperand(first,isRead,isWritten,isImplicit);
        if (boost::dynamic_pointer_cast<RegisterAST::Ptr>(first) != NULL)
        {
            for (uint32_t vec_id = 1; vec_id < vec_len; vec_id++)
            {
                insn_in_progress->appendOperand(decodeOPR_SRC_ACCVGPR(input+vec_id,0),isRead,isWritten,isImplicit);
            }
        }
    }

    void InstructionDecoder_amdgpu_gfx908::appendOPR_SRC_ACCVGPR_OR_CONST(uint64_t input, bool isRead, bool isWritten, uint32_t vec_len /*= 1*/ , bool isImplicit /*= false*/)
    {
        Expression::Ptr first = decodeOPR_SRC_ACCVGPR_OR_CONST(input,vec_len);
        insn_in_progress->appendOperand(first,isRead,isWritten,isImplicit);
        if (boost::dynamic_pointer_cast<RegisterAST::Ptr>(first) != NULL)
        {
            for (uint32_t vec_id = 1; vec_id < vec_len; vec_id++)
            {
                insn_in_progress->appendOperand(decodeOPR_SRC_ACCVGPR_OR_CONST(input+vec_id,0),isRead,isWritten,isImplicit);
            }
        }
    }

    void InstructionDecoder_amdgpu_gfx908::appendOPR_SRC_NOLDS(uint64_t input, bool isRead, bool isWritten, uint32_t vec_len /*= 1*/ , bool isImplicit /*= false*/)
    {
        Expression::Ptr first = decodeOPR_SRC_NOLDS(input,vec_len);
        insn_in_progress->appendOperand(first,isRead,isWritten,isImplicit);
        if (boost::dynamic_pointer_cast<RegisterAST::Ptr>(first) != NULL)
        {
            for (uint32_t vec_id = 1; vec_id < vec_len; vec_id++)
            {
                insn_in_progress->appendOperand(decodeOPR_SRC_NOLDS(input+vec_id,0),isRead,isWritten,isImplicit);
            }
        }
    }

    void InstructionDecoder_amdgpu_gfx908::appendOPR_SRC_NOLIT(uint64_t input, bool isRead, bool isWritten, uint32_t vec_len /*= 1*/ , bool isImplicit /*= false*/)
    {
        Expression::Ptr first = decodeOPR_SRC_NOLIT(input,vec_len);
        insn_in_progress->appendOperand(first,isRead,isWritten,isImplicit);
        if (boost::dynamic_pointer_cast<RegisterAST::Ptr>(first) != NULL)
        {
            for (uint32_t vec_id = 1; vec_id < vec_len; vec_id++)
            {
                insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(input+vec_id,0),isRead,isWritten,isImplicit);
            }
        }
    }

    void InstructionDecoder_amdgpu_gfx908::appendOPR_SRC_SIMPLE(uint64_t input, bool isRead, bool isWritten, uint32_t vec_len /*= 1*/ , bool isImplicit /*= false*/)
    {
        Expression::Ptr first = decodeOPR_SRC_SIMPLE(input,vec_len);
        insn_in_progress->appendOperand(first,isRead,isWritten,isImplicit);
        if (boost::dynamic_pointer_cast<RegisterAST::Ptr>(first) != NULL)
        {
            for (uint32_t vec_id = 1; vec_id < vec_len; vec_id++)
            {
                insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(input+vec_id,0),isRead,isWritten,isImplicit);
            }
        }
    }

    void InstructionDecoder_amdgpu_gfx908::appendOPR_SRC_VGPR(uint64_t input, bool isRead, bool isWritten, uint32_t vec_len /*= 1*/ , bool isImplicit /*= false*/)
    {
        Expression::Ptr first = decodeOPR_SRC_VGPR(input,vec_len);
        insn_in_progress->appendOperand(first,isRead,isWritten,isImplicit);
        if (boost::dynamic_pointer_cast<RegisterAST::Ptr>(first) != NULL)
        {
            for (uint32_t vec_id = 1; vec_id < vec_len; vec_id++)
            {
                insn_in_progress->appendOperand(decodeOPR_SRC_VGPR(input+vec_id,0),isRead,isWritten,isImplicit);
            }
        }
    }

    void InstructionDecoder_amdgpu_gfx908::appendOPR_SRC_VGPR_OR_ACCVGPR(uint64_t input, bool isRead, bool isWritten, uint32_t vec_len /*= 1*/ , bool isImplicit /*= false*/)
    {
        Expression::Ptr first = decodeOPR_SRC_VGPR_OR_ACCVGPR(input,vec_len);
        insn_in_progress->appendOperand(first,isRead,isWritten,isImplicit);
        if (boost::dynamic_pointer_cast<RegisterAST::Ptr>(first) != NULL)
        {
            for (uint32_t vec_id = 1; vec_id < vec_len; vec_id++)
            {
                insn_in_progress->appendOperand(decodeOPR_SRC_VGPR_OR_ACCVGPR(input+vec_id,0),isRead,isWritten,isImplicit);
            }
        }
    }

    void InstructionDecoder_amdgpu_gfx908::appendOPR_SREG(uint64_t input, bool isRead, bool isWritten, uint32_t vec_len /*= 1*/ , bool isImplicit /*= false*/)
    {
        Expression::Ptr first = decodeOPR_SREG(input,vec_len);
        insn_in_progress->appendOperand(first,isRead,isWritten,isImplicit);
        if (boost::dynamic_pointer_cast<RegisterAST::Ptr>(first) != NULL)
        {
            for (uint32_t vec_id = 1; vec_id < vec_len; vec_id++)
            {
                insn_in_progress->appendOperand(decodeOPR_SREG(input+vec_id,0),isRead,isWritten,isImplicit);
            }
        }
    }

    void InstructionDecoder_amdgpu_gfx908::appendOPR_SREG_NOVCC(uint64_t input, bool isRead, bool isWritten, uint32_t vec_len /*= 1*/ , bool isImplicit /*= false*/)
    {
        Expression::Ptr first = decodeOPR_SREG_NOVCC(input,vec_len);
        insn_in_progress->appendOperand(first,isRead,isWritten,isImplicit);
        if (boost::dynamic_pointer_cast<RegisterAST::Ptr>(first) != NULL)
        {
            for (uint32_t vec_id = 1; vec_id < vec_len; vec_id++)
            {
                insn_in_progress->appendOperand(decodeOPR_SREG_NOVCC(input+vec_id,0),isRead,isWritten,isImplicit);
            }
        }
    }

    void InstructionDecoder_amdgpu_gfx908::appendOPR_SSRC(uint64_t input, bool isRead, bool isWritten, uint32_t vec_len /*= 1*/ , bool isImplicit /*= false*/)
    {
        Expression::Ptr first = decodeOPR_SSRC(input,vec_len);
        insn_in_progress->appendOperand(first,isRead,isWritten,isImplicit);
        if (boost::dynamic_pointer_cast<RegisterAST::Ptr>(first) != NULL)
        {
            for (uint32_t vec_id = 1; vec_id < vec_len; vec_id++)
            {
                insn_in_progress->appendOperand(decodeOPR_SSRC(input+vec_id,0),isRead,isWritten,isImplicit);
            }
        }
    }

    void InstructionDecoder_amdgpu_gfx908::appendOPR_SSRC_LANESEL(uint64_t input, bool isRead, bool isWritten, uint32_t vec_len /*= 1*/ , bool isImplicit /*= false*/)
    {
        Expression::Ptr first = decodeOPR_SSRC_LANESEL(input,vec_len);
        insn_in_progress->appendOperand(first,isRead,isWritten,isImplicit);
        if (boost::dynamic_pointer_cast<RegisterAST::Ptr>(first) != NULL)
        {
            for (uint32_t vec_id = 1; vec_id < vec_len; vec_id++)
            {
                insn_in_progress->appendOperand(decodeOPR_SSRC_LANESEL(input+vec_id,0),isRead,isWritten,isImplicit);
            }
        }
    }

    void InstructionDecoder_amdgpu_gfx908::appendOPR_SSRC_NOLIT(uint64_t input, bool isRead, bool isWritten, uint32_t vec_len /*= 1*/ , bool isImplicit /*= false*/)
    {
        Expression::Ptr first = decodeOPR_SSRC_NOLIT(input,vec_len);
        insn_in_progress->appendOperand(first,isRead,isWritten,isImplicit);
        if (boost::dynamic_pointer_cast<RegisterAST::Ptr>(first) != NULL)
        {
            for (uint32_t vec_id = 1; vec_id < vec_len; vec_id++)
            {
                insn_in_progress->appendOperand(decodeOPR_SSRC_NOLIT(input+vec_id,0),isRead,isWritten,isImplicit);
            }
        }
    }

    void InstructionDecoder_amdgpu_gfx908::appendOPR_SSRC_SPECIAL_SCC(uint64_t input, bool isRead, bool isWritten, uint32_t vec_len /*= 1*/ , bool isImplicit /*= false*/)
    {
        Expression::Ptr first = decodeOPR_SSRC_SPECIAL_SCC(input,vec_len);
        insn_in_progress->appendOperand(first,isRead,isWritten,isImplicit);
        if (boost::dynamic_pointer_cast<RegisterAST::Ptr>(first) != NULL)
        {
            for (uint32_t vec_id = 1; vec_id < vec_len; vec_id++)
            {
                insn_in_progress->appendOperand(decodeOPR_SSRC_SPECIAL_SCC(input+vec_id,0),isRead,isWritten,isImplicit);
            }
        }
    }

    void InstructionDecoder_amdgpu_gfx908::appendOPR_TGT(uint64_t input, bool isRead, bool isWritten, uint32_t vec_len /*= 1*/ , bool isImplicit /*= false*/)
    {
        Expression::Ptr first = decodeOPR_TGT(input,vec_len);
        insn_in_progress->appendOperand(first,isRead,isWritten,isImplicit);
        if (boost::dynamic_pointer_cast<RegisterAST::Ptr>(first) != NULL)
        {
            for (uint32_t vec_id = 1; vec_id < vec_len; vec_id++)
            {
                insn_in_progress->appendOperand(decodeOPR_TGT(input+vec_id,0),isRead,isWritten,isImplicit);
            }
        }
    }

    void InstructionDecoder_amdgpu_gfx908::appendOPR_VCC(uint64_t input, bool isRead, bool isWritten, uint32_t vec_len /*= 1*/ , bool isImplicit /*= false*/)
    {
        Expression::Ptr first = decodeOPR_VCC(input,vec_len);
        insn_in_progress->appendOperand(first,isRead,isWritten,isImplicit);
        if (boost::dynamic_pointer_cast<RegisterAST::Ptr>(first) != NULL)
        {
            for (uint32_t vec_id = 1; vec_id < vec_len; vec_id++)
            {
                insn_in_progress->appendOperand(decodeOPR_VCC(input+vec_id,0),isRead,isWritten,isImplicit);
            }
        }
    }

    void InstructionDecoder_amdgpu_gfx908::appendOPR_VGPR(uint64_t input, bool isRead, bool isWritten, uint32_t vec_len /*= 1*/ , bool isImplicit /*= false*/)
    {
        Expression::Ptr first = decodeOPR_VGPR(input,vec_len);
        insn_in_progress->appendOperand(first,isRead,isWritten,isImplicit);
        if (boost::dynamic_pointer_cast<RegisterAST::Ptr>(first) != NULL)
        {
            for (uint32_t vec_id = 1; vec_id < vec_len; vec_id++)
            {
                insn_in_progress->appendOperand(decodeOPR_VGPR(input+vec_id,0),isRead,isWritten,isImplicit);
            }
        }
    }

    void InstructionDecoder_amdgpu_gfx908::appendOPR_VGPR_OR_LDS(uint64_t input, bool isRead, bool isWritten, uint32_t vec_len /*= 1*/ , bool isImplicit /*= false*/)
    {
        Expression::Ptr first = decodeOPR_VGPR_OR_LDS(input,vec_len);
        insn_in_progress->appendOperand(first,isRead,isWritten,isImplicit);
        if (boost::dynamic_pointer_cast<RegisterAST::Ptr>(first) != NULL)
        {
            for (uint32_t vec_id = 1; vec_id < vec_len; vec_id++)
            {
                insn_in_progress->appendOperand(decodeOPR_VGPR_OR_LDS(input+vec_id,0),isRead,isWritten,isImplicit);
            }
        }
    }

}
}
