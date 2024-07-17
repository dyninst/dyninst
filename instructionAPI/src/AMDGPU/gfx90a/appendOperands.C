#include <memory>
#include "InstructionDecoder-amdgpu-gfx90a.h"

namespace Dyninst {
namespace InstructionAPI {
    void InstructionDecoder_amdgpu_gfx90a::appendOPR_ACCVGPR(uint64_t input, bool isRead, bool isWritten, uint32_t vec_len /*= 1*/ , bool isImplicit /*= false*/)
    {
        Expression::Ptr first = decodeOPR_ACCVGPR(input,vec_len);
        RegisterAST::Ptr regptr = boost::dynamic_pointer_cast<RegisterAST>(first);
        if (regptr != NULL && vec_len > 1)
        {
            insn_in_progress->appendOperand(makeMultiRegisterExpression(regptr->getID(),vec_len),isRead,isWritten,isImplicit);
        }
        else
        {
            insn_in_progress->appendOperand(first,isRead,isWritten,isImplicit);
        }
    }

    void InstructionDecoder_amdgpu_gfx90a::appendOPR_DSMEM(uint64_t input, bool isRead, bool isWritten, uint32_t vec_len /*= 1*/ , bool isImplicit /*= false*/)
    {
        Expression::Ptr first = decodeOPR_DSMEM(input,vec_len);
        RegisterAST::Ptr regptr = boost::dynamic_pointer_cast<RegisterAST>(first);
        if (regptr != NULL && vec_len > 1)
        {
            insn_in_progress->appendOperand(makeMultiRegisterExpression(regptr->getID(),vec_len),isRead,isWritten,isImplicit);
        }
        else
        {
            insn_in_progress->appendOperand(first,isRead,isWritten,isImplicit);
        }
    }

    void InstructionDecoder_amdgpu_gfx90a::appendOPR_FLAT_SCRATCH(uint64_t input, bool isRead, bool isWritten, uint32_t vec_len /*= 1*/ , bool isImplicit /*= false*/)
    {
        Expression::Ptr first = decodeOPR_FLAT_SCRATCH(input,vec_len);
        RegisterAST::Ptr regptr = boost::dynamic_pointer_cast<RegisterAST>(first);
        if (regptr != NULL && vec_len > 1)
        {
            insn_in_progress->appendOperand(makeMultiRegisterExpression(regptr->getID(),vec_len),isRead,isWritten,isImplicit);
        }
        else
        {
            insn_in_progress->appendOperand(first,isRead,isWritten,isImplicit);
        }
    }

    void InstructionDecoder_amdgpu_gfx90a::appendOPR_PC(uint64_t input, bool isRead, bool isWritten, uint32_t vec_len /*= 1*/ , bool isImplicit /*= false*/)
    {
        Expression::Ptr first = decodeOPR_PC(input,vec_len);
        RegisterAST::Ptr regptr = boost::dynamic_pointer_cast<RegisterAST>(first);
        if (regptr != NULL && vec_len > 1)
        {
            insn_in_progress->appendOperand(makeMultiRegisterExpression(regptr->getID(),vec_len),isRead,isWritten,isImplicit);
        }
        else
        {
            insn_in_progress->appendOperand(first,isRead,isWritten,isImplicit);
        }
    }

    void InstructionDecoder_amdgpu_gfx90a::appendOPR_SDST(uint64_t input, bool isRead, bool isWritten, uint32_t vec_len /*= 1*/ , bool isImplicit /*= false*/)
    {
        Expression::Ptr first = decodeOPR_SDST(input,vec_len);
        RegisterAST::Ptr regptr = boost::dynamic_pointer_cast<RegisterAST>(first);
        if (regptr != NULL && vec_len > 1)
        {
            insn_in_progress->appendOperand(makeMultiRegisterExpression(regptr->getID(),vec_len),isRead,isWritten,isImplicit);
        }
        else
        {
            insn_in_progress->appendOperand(first,isRead,isWritten,isImplicit);
        }
    }

    void InstructionDecoder_amdgpu_gfx90a::appendOPR_SDST_EXEC(uint64_t input, bool isRead, bool isWritten, uint32_t vec_len /*= 1*/ , bool isImplicit /*= false*/)
    {
        Expression::Ptr first = decodeOPR_SDST_EXEC(input,vec_len);
        RegisterAST::Ptr regptr = boost::dynamic_pointer_cast<RegisterAST>(first);
        if (regptr != NULL && vec_len > 1)
        {
            insn_in_progress->appendOperand(makeMultiRegisterExpression(regptr->getID(),vec_len),isRead,isWritten,isImplicit);
        }
        else
        {
            insn_in_progress->appendOperand(first,isRead,isWritten,isImplicit);
        }
    }

    void InstructionDecoder_amdgpu_gfx90a::appendOPR_SDST_M0(uint64_t input, bool isRead, bool isWritten, uint32_t vec_len /*= 1*/ , bool isImplicit /*= false*/)
    {
        Expression::Ptr first = decodeOPR_SDST_M0(input,vec_len);
        RegisterAST::Ptr regptr = boost::dynamic_pointer_cast<RegisterAST>(first);
        if (regptr != NULL && vec_len > 1)
        {
            insn_in_progress->appendOperand(makeMultiRegisterExpression(regptr->getID(),vec_len),isRead,isWritten,isImplicit);
        }
        else
        {
            insn_in_progress->appendOperand(first,isRead,isWritten,isImplicit);
        }
    }

    void InstructionDecoder_amdgpu_gfx90a::appendOPR_SRC(uint64_t input, bool isRead, bool isWritten, uint32_t vec_len /*= 1*/ , bool isImplicit /*= false*/)
    {
        Expression::Ptr first = decodeOPR_SRC(input,vec_len);
        RegisterAST::Ptr regptr = boost::dynamic_pointer_cast<RegisterAST>(first);
        if (regptr != NULL && vec_len > 1)
        {
            insn_in_progress->appendOperand(makeMultiRegisterExpression(regptr->getID(),vec_len),isRead,isWritten,isImplicit);
        }
        else
        {
            insn_in_progress->appendOperand(first,isRead,isWritten,isImplicit);
        }
    }

    void InstructionDecoder_amdgpu_gfx90a::appendOPR_SRC_ACCVGPR(uint64_t input, bool isRead, bool isWritten, uint32_t vec_len /*= 1*/ , bool isImplicit /*= false*/)
    {
        Expression::Ptr first = decodeOPR_SRC_ACCVGPR(input,vec_len);
        RegisterAST::Ptr regptr = boost::dynamic_pointer_cast<RegisterAST>(first);
        if (regptr != NULL && vec_len > 1)
        {
            insn_in_progress->appendOperand(makeMultiRegisterExpression(regptr->getID(),vec_len),isRead,isWritten,isImplicit);
        }
        else
        {
            insn_in_progress->appendOperand(first,isRead,isWritten,isImplicit);
        }
    }

    void InstructionDecoder_amdgpu_gfx90a::appendOPR_SRC_NOLDS(uint64_t input, bool isRead, bool isWritten, uint32_t vec_len /*= 1*/ , bool isImplicit /*= false*/)
    {
        Expression::Ptr first = decodeOPR_SRC_NOLDS(input,vec_len);
        RegisterAST::Ptr regptr = boost::dynamic_pointer_cast<RegisterAST>(first);
        if (regptr != NULL && vec_len > 1)
        {
            insn_in_progress->appendOperand(makeMultiRegisterExpression(regptr->getID(),vec_len),isRead,isWritten,isImplicit);
        }
        else
        {
            insn_in_progress->appendOperand(first,isRead,isWritten,isImplicit);
        }
    }

    void InstructionDecoder_amdgpu_gfx90a::appendOPR_SRC_NOLIT(uint64_t input, bool isRead, bool isWritten, uint32_t vec_len /*= 1*/ , bool isImplicit /*= false*/)
    {
        Expression::Ptr first = decodeOPR_SRC_NOLIT(input,vec_len);
        RegisterAST::Ptr regptr = boost::dynamic_pointer_cast<RegisterAST>(first);
        if (regptr != NULL && vec_len > 1)
        {
            insn_in_progress->appendOperand(makeMultiRegisterExpression(regptr->getID(),vec_len),isRead,isWritten,isImplicit);
        }
        else
        {
            insn_in_progress->appendOperand(first,isRead,isWritten,isImplicit);
        }
    }

    void InstructionDecoder_amdgpu_gfx90a::appendOPR_SRC_SIMPLE(uint64_t input, bool isRead, bool isWritten, uint32_t vec_len /*= 1*/ , bool isImplicit /*= false*/)
    {
        Expression::Ptr first = decodeOPR_SRC_SIMPLE(input,vec_len);
        RegisterAST::Ptr regptr = boost::dynamic_pointer_cast<RegisterAST>(first);
        if (regptr != NULL && vec_len > 1)
        {
            insn_in_progress->appendOperand(makeMultiRegisterExpression(regptr->getID(),vec_len),isRead,isWritten,isImplicit);
        }
        else
        {
            insn_in_progress->appendOperand(first,isRead,isWritten,isImplicit);
        }
    }

    void InstructionDecoder_amdgpu_gfx90a::appendOPR_SRC_VGPR(uint64_t input, bool isRead, bool isWritten, uint32_t vec_len /*= 1*/ , bool isImplicit /*= false*/)
    {
        Expression::Ptr first = decodeOPR_SRC_VGPR(input,vec_len);
        RegisterAST::Ptr regptr = boost::dynamic_pointer_cast<RegisterAST>(first);
        if (regptr != NULL && vec_len > 1)
        {
            insn_in_progress->appendOperand(makeMultiRegisterExpression(regptr->getID(),vec_len),isRead,isWritten,isImplicit);
        }
        else
        {
            insn_in_progress->appendOperand(first,isRead,isWritten,isImplicit);
        }
    }

    void InstructionDecoder_amdgpu_gfx90a::appendOPR_SRC_VGPR_OR_ACCVGPR(uint64_t input, bool isRead, bool isWritten, uint32_t vec_len /*= 1*/ , bool isImplicit /*= false*/)
    {
        Expression::Ptr first = decodeOPR_SRC_VGPR_OR_ACCVGPR(input,vec_len);
        RegisterAST::Ptr regptr = boost::dynamic_pointer_cast<RegisterAST>(first);
        if (regptr != NULL && vec_len > 1)
        {
            insn_in_progress->appendOperand(makeMultiRegisterExpression(regptr->getID(),vec_len),isRead,isWritten,isImplicit);
        }
        else
        {
            insn_in_progress->appendOperand(first,isRead,isWritten,isImplicit);
        }
    }

    void InstructionDecoder_amdgpu_gfx90a::appendOPR_SRC_VGPR_OR_ACCVGPR_OR_CONST(uint64_t input, bool isRead, bool isWritten, uint32_t vec_len /*= 1*/ , bool isImplicit /*= false*/)
    {
        Expression::Ptr first = decodeOPR_SRC_VGPR_OR_ACCVGPR_OR_CONST(input,vec_len);
        RegisterAST::Ptr regptr = boost::dynamic_pointer_cast<RegisterAST>(first);
        if (regptr != NULL && vec_len > 1)
        {
            insn_in_progress->appendOperand(makeMultiRegisterExpression(regptr->getID(),vec_len),isRead,isWritten,isImplicit);
        }
        else
        {
            insn_in_progress->appendOperand(first,isRead,isWritten,isImplicit);
        }
    }

    void InstructionDecoder_amdgpu_gfx90a::appendOPR_SREG(uint64_t input, bool isRead, bool isWritten, uint32_t vec_len /*= 1*/ , bool isImplicit /*= false*/)
    {
        Expression::Ptr first = decodeOPR_SREG(input,vec_len);
        RegisterAST::Ptr regptr = boost::dynamic_pointer_cast<RegisterAST>(first);
        if (regptr != NULL && vec_len > 1)
        {
            insn_in_progress->appendOperand(makeMultiRegisterExpression(regptr->getID(),vec_len),isRead,isWritten,isImplicit);
        }
        else
        {
            insn_in_progress->appendOperand(first,isRead,isWritten,isImplicit);
        }
    }

    void InstructionDecoder_amdgpu_gfx90a::appendOPR_SREG_NOVCC(uint64_t input, bool isRead, bool isWritten, uint32_t vec_len /*= 1*/ , bool isImplicit /*= false*/)
    {
        Expression::Ptr first = decodeOPR_SREG_NOVCC(input,vec_len);
        RegisterAST::Ptr regptr = boost::dynamic_pointer_cast<RegisterAST>(first);
        if (regptr != NULL && vec_len > 1)
        {
            insn_in_progress->appendOperand(makeMultiRegisterExpression(regptr->getID(),vec_len),isRead,isWritten,isImplicit);
        }
        else
        {
            insn_in_progress->appendOperand(first,isRead,isWritten,isImplicit);
        }
    }

    void InstructionDecoder_amdgpu_gfx90a::appendOPR_SSRC(uint64_t input, bool isRead, bool isWritten, uint32_t vec_len /*= 1*/ , bool isImplicit /*= false*/)
    {
        Expression::Ptr first = decodeOPR_SSRC(input,vec_len);
        RegisterAST::Ptr regptr = boost::dynamic_pointer_cast<RegisterAST>(first);
        if (regptr != NULL && vec_len > 1)
        {
            insn_in_progress->appendOperand(makeMultiRegisterExpression(regptr->getID(),vec_len),isRead,isWritten,isImplicit);
        }
        else
        {
            insn_in_progress->appendOperand(first,isRead,isWritten,isImplicit);
        }
    }

    void InstructionDecoder_amdgpu_gfx90a::appendOPR_SSRC_LANESEL(uint64_t input, bool isRead, bool isWritten, uint32_t vec_len /*= 1*/ , bool isImplicit /*= false*/)
    {
        Expression::Ptr first = decodeOPR_SSRC_LANESEL(input,vec_len);
        RegisterAST::Ptr regptr = boost::dynamic_pointer_cast<RegisterAST>(first);
        if (regptr != NULL && vec_len > 1)
        {
            insn_in_progress->appendOperand(makeMultiRegisterExpression(regptr->getID(),vec_len),isRead,isWritten,isImplicit);
        }
        else
        {
            insn_in_progress->appendOperand(first,isRead,isWritten,isImplicit);
        }
    }

    void InstructionDecoder_amdgpu_gfx90a::appendOPR_SSRC_NOLIT(uint64_t input, bool isRead, bool isWritten, uint32_t vec_len /*= 1*/ , bool isImplicit /*= false*/)
    {
        Expression::Ptr first = decodeOPR_SSRC_NOLIT(input,vec_len);
        RegisterAST::Ptr regptr = boost::dynamic_pointer_cast<RegisterAST>(first);
        if (regptr != NULL && vec_len > 1)
        {
            insn_in_progress->appendOperand(makeMultiRegisterExpression(regptr->getID(),vec_len),isRead,isWritten,isImplicit);
        }
        else
        {
            insn_in_progress->appendOperand(first,isRead,isWritten,isImplicit);
        }
    }

    void InstructionDecoder_amdgpu_gfx90a::appendOPR_SSRC_SPECIAL_SCC(uint64_t input, bool isRead, bool isWritten, uint32_t vec_len /*= 1*/ , bool isImplicit /*= false*/)
    {
        Expression::Ptr first = decodeOPR_SSRC_SPECIAL_SCC(input,vec_len);
        RegisterAST::Ptr regptr = boost::dynamic_pointer_cast<RegisterAST>(first);
        if (regptr != NULL && vec_len > 1)
        {
            insn_in_progress->appendOperand(makeMultiRegisterExpression(regptr->getID(),vec_len),isRead,isWritten,isImplicit);
        }
        else
        {
            insn_in_progress->appendOperand(first,isRead,isWritten,isImplicit);
        }
    }

    void InstructionDecoder_amdgpu_gfx90a::appendOPR_VCC(uint64_t input, bool isRead, bool isWritten, uint32_t vec_len /*= 1*/ , bool isImplicit /*= false*/)
    {
        Expression::Ptr first = decodeOPR_VCC(input,vec_len);
        RegisterAST::Ptr regptr = boost::dynamic_pointer_cast<RegisterAST>(first);
        if (regptr != NULL && vec_len > 1)
        {
            insn_in_progress->appendOperand(makeMultiRegisterExpression(regptr->getID(),vec_len),isRead,isWritten,isImplicit);
        }
        else
        {
            insn_in_progress->appendOperand(first,isRead,isWritten,isImplicit);
        }
    }

    void InstructionDecoder_amdgpu_gfx90a::appendOPR_VGPR(uint64_t input, bool isRead, bool isWritten, uint32_t vec_len /*= 1*/ , bool isImplicit /*= false*/)
    {
        Expression::Ptr first = decodeOPR_VGPR(input,vec_len);
        RegisterAST::Ptr regptr = boost::dynamic_pointer_cast<RegisterAST>(first);
        if (regptr != NULL && vec_len > 1)
        {
            insn_in_progress->appendOperand(makeMultiRegisterExpression(regptr->getID(),vec_len),isRead,isWritten,isImplicit);
        }
        else
        {
            insn_in_progress->appendOperand(first,isRead,isWritten,isImplicit);
        }
    }

    void InstructionDecoder_amdgpu_gfx90a::appendOPR_VGPR_OR_ACCVGPR(uint64_t input, bool isRead, bool isWritten, uint32_t vec_len /*= 1*/ , bool isImplicit /*= false*/)
    {
        Expression::Ptr first = decodeOPR_VGPR_OR_ACCVGPR(input,vec_len);
        RegisterAST::Ptr regptr = boost::dynamic_pointer_cast<RegisterAST>(first);
        if (regptr != NULL && vec_len > 1)
        {
            insn_in_progress->appendOperand(makeMultiRegisterExpression(regptr->getID(),vec_len),isRead,isWritten,isImplicit);
        }
        else
        {
            insn_in_progress->appendOperand(first,isRead,isWritten,isImplicit);
        }
    }

    void InstructionDecoder_amdgpu_gfx90a::appendOPR_VGPR_OR_LDS(uint64_t input, bool isRead, bool isWritten, uint32_t vec_len /*= 1*/ , bool isImplicit /*= false*/)
    {
        Expression::Ptr first = decodeOPR_VGPR_OR_LDS(input,vec_len);
        RegisterAST::Ptr regptr = boost::dynamic_pointer_cast<RegisterAST>(first);
        if (regptr != NULL && vec_len > 1)
        {
            insn_in_progress->appendOperand(makeMultiRegisterExpression(regptr->getID(),vec_len),isRead,isWritten,isImplicit);
        }
        else
        {
            insn_in_progress->appendOperand(first,isRead,isWritten,isImplicit);
        }
    }

    void InstructionDecoder_amdgpu_gfx90a::appendOPR_SIMM16(uint64_t input, bool isRead, bool isWritten, uint32_t , bool isImplicit /*= false*/)
    {
        insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u16,field<0,15>(input))),isRead,isWritten,isImplicit);
    }

    void InstructionDecoder_amdgpu_gfx90a::appendOPR_SIMM4(uint64_t input, bool isRead, bool isWritten, uint32_t , bool isImplicit /*= false*/)
    {
        insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u8,field<0,3>(input))),isRead,isWritten,isImplicit);
    }

    void InstructionDecoder_amdgpu_gfx90a::appendOPR_HWREG(uint64_t input, bool isRead, bool isWritten, uint32_t , bool isImplicit /*= false*/)
    {
        insn_in_progress->appendOperand(decodeOPR_HWREG_ID(input,field<6,11>(input),field<11,16>(input)),isRead,isWritten,isImplicit);
    }

    void InstructionDecoder_amdgpu_gfx90a::appendOPR_WAITCNT(uint64_t input, bool isRead, bool isWritten, uint32_t , bool isImplicit /*= false*/)
    {
        insn_in_progress->appendOperand(NamedImmediate::makeNamedImmediate(std::string("EXP"),Result(u16,field<4,6>(input))),isRead,isWritten,isImplicit);
        insn_in_progress->appendOperand(NamedImmediate::makeNamedImmediate(std::string("LGKM"),Result(u16,field<8,11>(input))),isRead,isWritten,isImplicit);
        insn_in_progress->appendOperand(NamedImmediate::makeNamedImmediate(std::string("VM"),Result(u16,field<0,3>(input))),isRead,isWritten,isImplicit);
        insn_in_progress->appendOperand(NamedImmediate::makeNamedImmediate(std::string("VM_HI"),Result(u16,field<14,15>(input))),isRead,isWritten,isImplicit);
    }

    void InstructionDecoder_amdgpu_gfx90a::appendOPR_LABEL(uint64_t input, bool isRead, bool isWritten, uint32_t , bool isImplicit /*= false*/)
    {
        insn_in_progress->appendOperand(NamedImmediate::makeNamedImmediate(std::string("TARGET"),Result(u16,field<0,15>(input))),isRead,isWritten,isImplicit);
    }

    void InstructionDecoder_amdgpu_gfx90a::appendOPR_SENDMSG(uint64_t input, bool isRead, bool isWritten, uint32_t vec_len /*= 1*/ , bool isImplicit /*= false*/)
    {
        insn_in_progress->appendOperand(decodeOPR_SENDMSG_GSOP(input, vec_len),isRead,isWritten,isImplicit);
        insn_in_progress->appendOperand(decodeOPR_SENDMSG_MSG(input, vec_len),isRead,isWritten,isImplicit);
        insn_in_progress->appendOperand(NamedImmediate::makeNamedImmediate(std::string("STREAMID"),Result(u16,field<8,9>(input))),isRead,isWritten,isImplicit);
    }

    void InstructionDecoder_amdgpu_gfx90a::appendOPR_SIMM8(uint64_t input, bool isRead, bool isWritten, uint32_t , bool isImplicit /*= false*/)
    {
        insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u8,field<0,7>(input))),isRead,isWritten,isImplicit);
    }

    void InstructionDecoder_amdgpu_gfx90a::appendOPR_SIMM32(uint64_t input, bool isRead, bool isWritten, uint32_t , bool isImplicit /*= false*/)
    {
        insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u32,field<0,31>(input))),isRead,isWritten,isImplicit);
    }

}
}
