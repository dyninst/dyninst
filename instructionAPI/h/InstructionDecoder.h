
#if !defined(INSTRUCTION_DECODER_H)
#define INSTRUCTION_DECODER_H

#include "InstructionAST.h"
#include "Expression.h"
#include "BinaryFunction.h"
#include "Operation.h"
#include "Operand.h"
#include "Instruction.h"

#include "../src/arch-x86.h"
#include <vector>

namespace Dyninst
{
  namespace Instruction
  {
    /// The %InstructionDecoder class decodes instructions, given a buffer of bytes and a length,
    /// and constructs an %Instruction.
    /// The %InstructionDecoder will, by default, be constructed to decode machine language
    /// on the platform on which it has been compiled.  The buffer
    /// will be treated as if there is an instruction stream starting at the beginning of the buffer.
    class InstructionDecoder
    {
    public:
      InstructionDecoder() : decodedInstruction(NULL), m_Operation(NULL), is32BitMode(true), sizePrefixPresent(false)
      {
      }
      
      ~InstructionDecoder() 
      {
	delete decodedInstruction;
      }
      /// Decode the buffer at \c buffer, up to \c size bytes, interpreting it as 
      /// machine language of the type understood by this %InstructionDecoder.
      /// If the buffer does not contain a valid instruction stream, an %Instruction with an invalid %Operation
      /// will be returned.  The %Instruction's \c size field will be set to the number of bytes actually decoded.
      Instruction decode(const unsigned char* buffer, size_t size);
    protected:
      void decodeOperands(std::vector<Expression::Ptr>& operands);

      void decodeOneOperand(const ia32_operand& operand,
			    std::vector<Expression::Ptr>& outputOperands);
      unsigned int decodeOpcode();
      
      Expression::Ptr makeSIBExpression(unsigned int opType);
      Expression::Ptr makeModRMExpression(unsigned int opType);
      template< typename T1, typename T2 >
      Expression::Ptr makeAddExpression(T1 lhs, T2 rhs, Result_Type resultType)
      {
	return Expression::Ptr(new BinaryFunction(lhs, rhs, resultType, boost::shared_ptr<BinaryFunction::funcT>(new BinaryFunction::addResult())));
      }
      template< typename T1, typename T2 >
      Expression::Ptr makeMultiplyExpression(T1 lhs, T2 rhs, Result_Type resultType)
      {
	return Expression::Ptr(new BinaryFunction(lhs, rhs, resultType, boost::shared_ptr<BinaryFunction::funcT>(new BinaryFunction::multResult())));
      } 
      Expression::Ptr getModRMDisplacement();
      int makeRegisterID(unsigned int intelReg, unsigned int opType);
      Expression::Ptr decodeImmediate(unsigned int opType, unsigned int position);
      Result_Type makeSizeType(unsigned int opType);
      
    private:
      const unsigned char* rawInstruction;
      ia32_locations locs;
      ia32_condition cond;
      ia32_memacc mac[3];
      ia32_instruction* decodedInstruction;
      Operation m_Operation;
      bool is32BitMode;
      bool sizePrefixPresent;
      
    };
  };
};

#endif //!defined(INSTRUCTION_DECODER_H)
