#pragma once

#if !defined(_ROSE_INSN_FACTORY_H_)
#define _ROSE_INSN_FACTORY_H_

#include "entryIDs.h"
#include "external/rose/rose-compat.h"
#include "external/rose/powerpcInstructionEnum.h"
#include "Visitor.h"
#include "Instruction.h"

class SgAsmx86Instruction;
class SgAsmExpression;
class SgAsmPowerpcInstruction;
class SgAsmOperandList;
class SgAsmx86RegisterReferenceExpression;
class SgAsmPowerpcRegisterReferenceExpression;

#include "RoseInsnFactoryArchTraits.h"

namespace Dyninst
{
namespace InstructionAPI {
  class RegisterAST;
  class Dereference;
  class Immediate;
  class BinaryFunction;
}

namespace SymbolicEvaluation {
  class RoseInsnFactory {
  public:
    RoseInsnFactory(void) {};
    virtual ~RoseInsnFactory(void) {};
    virtual SageInstruction_t *convert(const InstructionAPI::Instruction::Ptr &insn, uint64_t addr);

  protected:
    virtual SageInstruction_t *createInsn() = 0;
    virtual void setOpcode(SageInstruction_t *insn, entryID opcode, std::string mnem) = 0;
    virtual void setSizes(SageInstruction_t *insn) = 0; 
    virtual bool handleSpecialCases(entryID opcode, SageInstruction_t *rinsn, SgAsmOperandList *roperands) = 0;
    virtual void massageOperands(InstructionAPI::Instruction::Ptr &insn, std::vector<InstructionAPI::Operand> &operands) = 0;

    virtual SgAsmExpression *convertOperand(const Expression::Ptr expression);

    friend class ExpressionConversionVisitor;
  };

class RoseInsnX86Factory : public RoseInsnFactory {
    RoseInsnX86Factory(void) {};
    virtual ~RoseInsnX86Factory(void) {};
    
  private:
    virtual void setOpcode(SageInstruction_t *insn, entryID opcode, std::string mnem);
    virtual void setSizes(SageInstruction_t *insn);
    virtual bool handleSpecialCases(entryID opcode, SageInstruction_t *rinsn, SgAsmOperandList *roperands);
    virtual void massageOperands(InstructionAPI::Instruction::Ptr &insn, std::vector<InstructionAPI::Operand> &operands);

    X86InstructionKind convertKind(entryID opcode);
  };

  class RoseInsnPPCFactory : public RoseInsnFactory {
    RoseInsnPPCFactory(void) {};
    virtual ~RoseInsnPPCFactory(void) {};

  private:
    virtual void setOpcode(SageInstruction_t *insn, entryID opcode, std::string mnem);
    virtual void setSizes(SageInstruction_t *insn);
    virtual bool handleSpecialCases(entryID opcode, SageInstruction_t *rinsn, SgAsmOperandList *roperands);
    virtual void massageOperands(InstructionAPI::Instruction::Ptr &insn, std::vector<InstructionAPI::Operand> &operands);

    PowerpcInstructionKind convertKind(entryID opcode, std::string mnem);
    PowerpcInstructionKind makeRoseBranchOpcode(entryID iapi_opcode, bool isAbsolute, bool isLink);
  };
};
};

#endif
