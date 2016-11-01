#ifndef INSTRUCTIONDECODER_ARMv6M_H
#define INSTRUCTIONDECODER_ARMv6M_H

#include "InstructionDecoderImpl.h"


namespace Dyninst {
namespace InstructionAPI {


class InstructionDecoder_ARMv6M : public InstructionDecoderImpl
{
public:
    InstructionDecoder_ARMv6M(Architecture arch);

    //virtual ~InstructionDecoder_ARMv6M();

    virtual void decodeOpcode(InstructionDecoder::buffer &b);

    virtual Instruction::Ptr decode(InstructionDecoder::buffer &b);

    virtual void setMode(bool) { }

    virtual bool decodeOperands(const Instruction *insn_to_complete);

    virtual void doDelayedDecode(const Instruction *insn_to_complete);

protected:
    virtual Result_Type makeSizeType(unsigned int);

private:
    typedef int32_t Word;
    typedef uint32_t UWord;

private:
    // Does the actual decoding.
    void doDecode(InstructionDecoder::buffer &buf);

    // Makes sure that the given number of bytes have been read from the input buffer.
    void ensureWeHave(int bytes, const InstructionDecoder::buffer &buf);

private:
    Instruction::Ptr m_instrInProgress;

    UWord m_instrWord = 0;
    int m_bytesRead = 0;

};

}
}

#endif
