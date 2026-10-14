#include <memory>
#include "InstructionDecoder-amdgpu-gfx1250.h"

namespace Dyninst {
namespace InstructionAPI {
    void InstructionDecoder_amdgpu_gfx1250::appendOPR_DSMEM(uint64_t input, bool isRead, bool isWritten, uint32_t vec_len /*= 1*/ , bool isImplicit /*= false*/)
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

    void InstructionDecoder_amdgpu_gfx1250::appendOPR_EXEC(uint64_t input, bool isRead, bool isWritten, uint32_t vec_len /*= 1*/ , bool isImplicit /*= false*/)
    {
        Expression::Ptr first = decodeOPR_EXEC(input,vec_len);
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

    void InstructionDecoder_amdgpu_gfx1250::appendOPR_GPUMEM(uint64_t input, bool isRead, bool isWritten, uint32_t vec_len /*= 1*/ , bool isImplicit /*= false*/)
    {
        Expression::Ptr first = decodeOPR_GPUMEM(input,vec_len);
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

    void InstructionDecoder_amdgpu_gfx1250::appendOPR_PC(uint64_t input, bool isRead, bool isWritten, uint32_t vec_len /*= 1*/ , bool isImplicit /*= false*/)
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

    void InstructionDecoder_amdgpu_gfx1250::appendOPR_SDST(uint64_t input, bool isRead, bool isWritten, uint32_t vec_len /*= 1*/ , bool isImplicit /*= false*/)
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

    void InstructionDecoder_amdgpu_gfx1250::appendOPR_SDST_EXEC(uint64_t input, bool isRead, bool isWritten, uint32_t vec_len /*= 1*/ , bool isImplicit /*= false*/)
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

    void InstructionDecoder_amdgpu_gfx1250::appendOPR_SDST_M0(uint64_t input, bool isRead, bool isWritten, uint32_t vec_len /*= 1*/ , bool isImplicit /*= false*/)
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

    void InstructionDecoder_amdgpu_gfx1250::appendOPR_SGPR(uint64_t input, bool isRead, bool isWritten, uint32_t vec_len /*= 1*/ , bool isImplicit /*= false*/)
    {
        Expression::Ptr first = decodeOPR_SGPR(input,vec_len);
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

    void InstructionDecoder_amdgpu_gfx1250::appendOPR_SMEM_OFFSET_NOK(uint64_t input, bool isRead, bool isWritten, uint32_t vec_len /*= 1*/ , bool isImplicit /*= false*/)
    {
        Expression::Ptr first = decodeOPR_SMEM_OFFSET_NOK(input,vec_len);
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

    void InstructionDecoder_amdgpu_gfx1250::appendOPR_SRC(uint64_t input, bool isRead, bool isWritten, uint32_t vec_len /*= 1*/ , bool isImplicit /*= false*/)
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

    void InstructionDecoder_amdgpu_gfx1250::appendOPR_SRC_NOINLINE(uint64_t input, bool isRead, bool isWritten, uint32_t vec_len /*= 1*/ , bool isImplicit /*= false*/)
    {
        Expression::Ptr first = decodeOPR_SRC_NOINLINE(input,vec_len);
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

    void InstructionDecoder_amdgpu_gfx1250::appendOPR_SRC_SIMPLE(uint64_t input, bool isRead, bool isWritten, uint32_t vec_len /*= 1*/ , bool isImplicit /*= false*/)
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

    void InstructionDecoder_amdgpu_gfx1250::appendOPR_SRC_VGPR(uint64_t input, bool isRead, bool isWritten, uint32_t vec_len /*= 1*/ , bool isImplicit /*= false*/)
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

    void InstructionDecoder_amdgpu_gfx1250::appendOPR_SRC_VGPR_OR_INLINE(uint64_t input, bool isRead, bool isWritten, uint32_t vec_len /*= 1*/ , bool isImplicit /*= false*/)
    {
        Expression::Ptr first = decodeOPR_SRC_VGPR_OR_INLINE(input,vec_len);
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

    void InstructionDecoder_amdgpu_gfx1250::appendOPR_SREG(uint64_t input, bool isRead, bool isWritten, uint32_t vec_len /*= 1*/ , bool isImplicit /*= false*/)
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

    void InstructionDecoder_amdgpu_gfx1250::appendOPR_SREG_M0(uint64_t input, bool isRead, bool isWritten, uint32_t vec_len /*= 1*/ , bool isImplicit /*= false*/)
    {
        Expression::Ptr first = decodeOPR_SREG_M0(input,vec_len);
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

    void InstructionDecoder_amdgpu_gfx1250::appendOPR_SSRC(uint64_t input, bool isRead, bool isWritten, uint32_t vec_len /*= 1*/ , bool isImplicit /*= false*/)
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

    void InstructionDecoder_amdgpu_gfx1250::appendOPR_SSRC_BARRIER_ID(uint64_t input, bool isRead, bool isWritten, uint32_t vec_len /*= 1*/ , bool isImplicit /*= false*/)
    {
        Expression::Ptr first = decodeOPR_SSRC_BARRIER_ID(input,vec_len);
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

    void InstructionDecoder_amdgpu_gfx1250::appendOPR_SSRC_LANESEL(uint64_t input, bool isRead, bool isWritten, uint32_t vec_len /*= 1*/ , bool isImplicit /*= false*/)
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

    void InstructionDecoder_amdgpu_gfx1250::appendOPR_SSRC_SPECIAL_SCC(uint64_t input, bool isRead, bool isWritten, uint32_t vec_len /*= 1*/ , bool isImplicit /*= false*/)
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

    void InstructionDecoder_amdgpu_gfx1250::appendOPR_VCC(uint64_t input, bool isRead, bool isWritten, uint32_t vec_len /*= 1*/ , bool isImplicit /*= false*/)
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

    void InstructionDecoder_amdgpu_gfx1250::appendOPR_VGPR(uint64_t input, bool isRead, bool isWritten, uint32_t vec_len /*= 1*/ , bool isImplicit /*= false*/)
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

    void InstructionDecoder_amdgpu_gfx1250::appendOPR_CLAUSE(uint64_t input, bool isRead, bool isWritten, uint32_t , bool isImplicit /*= false*/)
    {
        insn_in_progress->appendOperand(NamedImmediate::makeNamedImmediate(std::string("BREAK_SPAN"),Result(u16,longfield<8,11>(input))),isRead,isWritten,isImplicit);
        insn_in_progress->appendOperand(NamedImmediate::makeNamedImmediate(std::string("LENGTH"),Result(u16,longfield<0,5>(input))),isRead,isWritten,isImplicit);
    }

    void InstructionDecoder_amdgpu_gfx1250::appendOPR_DELAY(uint64_t input, bool isRead, bool isWritten, uint32_t vec_len /*= 1*/ , bool isImplicit /*= false*/)
    {
        insn_in_progress->appendOperand(decodeOPR_DELAY_INSTID0(input, vec_len),isRead,isWritten,isImplicit);
        insn_in_progress->appendOperand(decodeOPR_DELAY_INSTID1(input, vec_len),isRead,isWritten,isImplicit);
        insn_in_progress->appendOperand(decodeOPR_DELAY_INSTSKIP(input, vec_len),isRead,isWritten,isImplicit);
    }

    void InstructionDecoder_amdgpu_gfx1250::appendOPR_HWREG(uint64_t input, bool isRead, bool isWritten, uint32_t , bool isImplicit /*= false*/)
    {
        insn_in_progress->appendOperand(decodeOPR_HWREG_ID(field<0,5>(input),field<6,10>(input),field<6,10>(input) + field<11,15>(input) + 1),isRead,isWritten,isImplicit);
    }

    void InstructionDecoder_amdgpu_gfx1250::appendOPR_LABEL(uint64_t input, bool isRead, bool isWritten, uint32_t , bool isImplicit /*= false*/)
    {
        insn_in_progress->appendOperand(NamedImmediate::makeNamedImmediate(std::string("TARGET"),Result(u16,longfield<0,15>(input))),isRead,isWritten,isImplicit);
    }

    void InstructionDecoder_amdgpu_gfx1250::appendOPR_SENDMSG(uint64_t input, bool isRead, bool isWritten, uint32_t vec_len /*= 1*/ , bool isImplicit /*= false*/)
    {
        insn_in_progress->appendOperand(NamedImmediate::makeNamedImmediate(std::string("HAS_RTN"),Result(u16,longfield<7,7>(input))),isRead,isWritten,isImplicit);
        insn_in_progress->appendOperand(decodeOPR_SENDMSG_RTN_MSG(input, vec_len),isRead,isWritten,isImplicit);
        insn_in_progress->appendOperand(NamedImmediate::makeNamedImmediate(std::string("SYSTEM"),Result(u16,longfield<8,8>(input))),isRead,isWritten,isImplicit);
    }

    void InstructionDecoder_amdgpu_gfx1250::appendOPR_SENDMSG_RTN(uint64_t input, bool isRead, bool isWritten, uint32_t vec_len /*= 1*/ , bool isImplicit /*= false*/)
    {
        insn_in_progress->appendOperand(NamedImmediate::makeNamedImmediate(std::string("HAS_RTN"),Result(u16,longfield<7,7>(input))),isRead,isWritten,isImplicit);
        insn_in_progress->appendOperand(decodeOPR_SENDMSG_RTN_MSG(input, vec_len),isRead,isWritten,isImplicit);
        insn_in_progress->appendOperand(NamedImmediate::makeNamedImmediate(std::string("SYSTEM"),Result(u16,longfield<8,8>(input))),isRead,isWritten,isImplicit);
    }

    void InstructionDecoder_amdgpu_gfx1250::appendOPR_SET_VGPR_MSB(uint64_t input, bool isRead, bool isWritten, uint32_t , bool isImplicit /*= false*/)
    {
        insn_in_progress->appendOperand(NamedImmediate::makeNamedImmediate(std::string("DST_MSB"),Result(u16,longfield<6,7>(input))),isRead,isWritten,isImplicit);
        insn_in_progress->appendOperand(NamedImmediate::makeNamedImmediate(std::string("SRC0_MSB"),Result(u16,longfield<0,1>(input))),isRead,isWritten,isImplicit);
        insn_in_progress->appendOperand(NamedImmediate::makeNamedImmediate(std::string("SRC1_MSB"),Result(u16,longfield<2,3>(input))),isRead,isWritten,isImplicit);
        insn_in_progress->appendOperand(NamedImmediate::makeNamedImmediate(std::string("SRC2_MSB"),Result(u16,longfield<4,5>(input))),isRead,isWritten,isImplicit);
    }

    void InstructionDecoder_amdgpu_gfx1250::appendOPR_SIMM16(uint64_t input, bool isRead, bool isWritten, uint32_t , bool isImplicit /*= false*/)
    {
        insn_in_progress->appendOperand(Immediate::makeImmediate(Result(s16,longfield<0,15>(input))),isRead,isWritten,isImplicit);
    }

    void InstructionDecoder_amdgpu_gfx1250::appendOPR_SIMM24(uint64_t input, bool isRead, bool isWritten, uint32_t , bool isImplicit /*= false*/)
    {
        insn_in_progress->appendOperand(Immediate::makeImmediate(Result(s32,longfield<0,23>(input))),isRead,isWritten,isImplicit);
    }

    void InstructionDecoder_amdgpu_gfx1250::appendOPR_SIMM32(uint64_t input, bool isRead, bool isWritten, uint32_t , bool isImplicit /*= false*/)
    {
        insn_in_progress->appendOperand(Immediate::makeImmediate(Result(s32,longfield<0,31>(input))),isRead,isWritten,isImplicit);
    }

    void InstructionDecoder_amdgpu_gfx1250::appendOPR_SIMM5(uint64_t input, bool isRead, bool isWritten, uint32_t , bool isImplicit /*= false*/)
    {
        insn_in_progress->appendOperand(Immediate::makeImmediate(Result(s8,longfield<0,4>(input))),isRead,isWritten,isImplicit);
    }

    void InstructionDecoder_amdgpu_gfx1250::appendOPR_SIMM64(uint64_t input, bool isRead, bool isWritten, uint32_t , bool isImplicit /*= false*/)
    {
        insn_in_progress->appendOperand(Immediate::makeImmediate(Result(s64,longfield<0,63>(input))),isRead,isWritten,isImplicit);
    }

    void InstructionDecoder_amdgpu_gfx1250::appendOPR_SIMM8(uint64_t input, bool isRead, bool isWritten, uint32_t , bool isImplicit /*= false*/)
    {
        insn_in_progress->appendOperand(Immediate::makeImmediate(Result(s8,longfield<0,7>(input))),isRead,isWritten,isImplicit);
    }

    void InstructionDecoder_amdgpu_gfx1250::appendOPR_SLEEP(uint64_t input, bool isRead, bool isWritten, uint32_t , bool isImplicit /*= false*/)
    {
        insn_in_progress->appendOperand(NamedImmediate::makeNamedImmediate(std::string("DURATION"),Result(u16,longfield<0,6>(input))),isRead,isWritten,isImplicit);
        insn_in_progress->appendOperand(NamedImmediate::makeNamedImmediate(std::string("SLEEP_FOREVER"),Result(u16,longfield<15,15>(input))),isRead,isWritten,isImplicit);
    }

    void InstructionDecoder_amdgpu_gfx1250::appendOPR_VERSION(uint64_t input, bool isRead, bool isWritten, uint32_t vec_len /*= 1*/ , bool isImplicit /*= false*/)
    {
        insn_in_progress->appendOperand(NamedImmediate::makeNamedImmediate(std::string("MDP"),Result(u16,longfield<15,15>(input))),isRead,isWritten,isImplicit);
        insn_in_progress->appendOperand(decodeOPR_VERSION_VERSION(input, vec_len),isRead,isWritten,isImplicit);
        insn_in_progress->appendOperand(NamedImmediate::makeNamedImmediate(std::string("W32"),Result(u16,longfield<14,14>(input))),isRead,isWritten,isImplicit);
        insn_in_progress->appendOperand(NamedImmediate::makeNamedImmediate(std::string("W64"),Result(u16,longfield<13,13>(input))),isRead,isWritten,isImplicit);
    }

    void InstructionDecoder_amdgpu_gfx1250::appendOPR_WAIT_ALU(uint64_t input, bool isRead, bool isWritten, uint32_t , bool isImplicit /*= false*/)
    {
        insn_in_progress->appendOperand(NamedImmediate::makeNamedImmediate(std::string("HOLD_CNT"),Result(u16,longfield<7,7>(input))),isRead,isWritten,isImplicit);
        insn_in_progress->appendOperand(NamedImmediate::makeNamedImmediate(std::string("SA_SDST"),Result(u16,longfield<0,0>(input))),isRead,isWritten,isImplicit);
        insn_in_progress->appendOperand(NamedImmediate::makeNamedImmediate(std::string("VA_SDST"),Result(u16,longfield<9,11>(input))),isRead,isWritten,isImplicit);
        insn_in_progress->appendOperand(NamedImmediate::makeNamedImmediate(std::string("VA_SSRC"),Result(u16,longfield<8,8>(input))),isRead,isWritten,isImplicit);
        insn_in_progress->appendOperand(NamedImmediate::makeNamedImmediate(std::string("VA_VCC"),Result(u16,longfield<1,1>(input))),isRead,isWritten,isImplicit);
        insn_in_progress->appendOperand(NamedImmediate::makeNamedImmediate(std::string("VA_VDST"),Result(u16,longfield<12,15>(input))),isRead,isWritten,isImplicit);
        insn_in_progress->appendOperand(NamedImmediate::makeNamedImmediate(std::string("VM_VSRC"),Result(u16,longfield<2,4>(input))),isRead,isWritten,isImplicit);
    }

    void InstructionDecoder_amdgpu_gfx1250::appendOPR_WAIT_MEM_DS(uint64_t input, bool isRead, bool isWritten, uint32_t , bool isImplicit /*= false*/)
    {
        insn_in_progress->appendOperand(NamedImmediate::makeNamedImmediate(std::string("DS"),Result(u16,longfield<0,5>(input))),isRead,isWritten,isImplicit);
        insn_in_progress->appendOperand(NamedImmediate::makeNamedImmediate(std::string("MEM"),Result(u16,longfield<8,13>(input))),isRead,isWritten,isImplicit);
    }

}
}
