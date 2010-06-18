#pragma once

#if !defined(_ROSE_INSN_FACTORY_H_)
#define _ROSE_INSN_FACTORY_H_

#include "entryIDs.h"
#include "external/rose/rose-compat.h"
#include "external/rose/powerpcInstructionEnum.h"
#include "Visitor.h"
#include "dyn_detail/boost/shared_ptr.hpp"


#if !defined(_MSC_VER)
#include <stdint.h>
#else
#include "external/stdint-win.h"
#endif

class SgAsmInstruction;
class SgAsmx86Instruction;
class SgAsmExpression;
class SgAsmPowerpcInstruction;
class SgAsmOperandList;
class SgAsmx86RegisterReferenceExpression;
class SgAsmPowerpcRegisterReferenceExpression;

namespace Dyninst
{
namespace InstructionAPI {
  class RegisterAST;
  class Dereference;
  class Immediate;
  class BinaryFunction;
  class Expression;
  class Operand;
  class Instruction;
}

namespace SymbolicEvaluation {
  class RoseInsnFactory {
  protected:
    typedef dyn_detail::boost::shared_ptr<InstructionAPI::Expression> ExpressionPtr;
    typedef dyn_detail::boost::shared_ptr<InstructionAPI::Instruction> InstructionPtr;
  public:
    SYMEVAL_EXPORT RoseInsnFactory(void) {};
    SYMEVAL_EXPORT virtual ~RoseInsnFactory(void) {};
    SYMEVAL_EXPORT virtual SgAsmInstruction *convert(const InstructionPtr &insn, uint64_t addr);

  protected:
    virtual SgAsmInstruction *createInsn() = 0;
    virtual void setOpcode(SgAsmInstruction *insn, entryID opcode, prefixEntryID prefix, std::string mnem) = 0;
    virtual void setSizes(SgAsmInstruction *insn) = 0; 
    virtual bool handleSpecialCases(entryID opcode, SgAsmInstruction *rinsn, SgAsmOperandList *roperands) = 0;
    virtual void massageOperands(const InstructionPtr &insn, std::vector<InstructionAPI::Operand> &operands) = 0;

    virtual SgAsmExpression *convertOperand(const ExpressionPtr expression, uint64_t addr);

    friend class ExpressionConversionVisitor;

    virtual Architecture arch() { return Arch_none; };
  };

class RoseInsnX86Factory : public RoseInsnFactory {
  public:
    SYMEVAL_EXPORT RoseInsnX86Factory() {};
    SYMEVAL_EXPORT virtual ~RoseInsnX86Factory() {};
    
  private:
    virtual SgAsmInstruction *createInsn();
    virtual void setOpcode(SgAsmInstruction *insn, entryID opcode, prefixEntryID prefix, std::string mnem);
    virtual void setSizes(SgAsmInstruction *insn);
    virtual bool handleSpecialCases(entryID opcode, SgAsmInstruction *rinsn, SgAsmOperandList *roperands);
    virtual void massageOperands(const InstructionPtr &insn, std::vector<InstructionAPI::Operand> &operands);

    X86InstructionKind convertKind(entryID opcode, prefixEntryID prefix);

    virtual Architecture arch() { return Arch_x86; };
  };

  class RoseInsnPPCFactory : public RoseInsnFactory {
  public:
    SYMEVAL_EXPORT RoseInsnPPCFactory(void) {};
    SYMEVAL_EXPORT virtual ~RoseInsnPPCFactory(void) {};

  private:
    virtual SgAsmInstruction *createInsn();
    virtual void setOpcode(SgAsmInstruction *insn, entryID opcode, prefixEntryID prefix, std::string mnem);
    virtual void setSizes(SgAsmInstruction *insn);
    virtual bool handleSpecialCases(entryID opcode, SgAsmInstruction *rinsn, SgAsmOperandList *roperands);
    virtual void massageOperands(const InstructionPtr &insn, std::vector<InstructionAPI::Operand> &operands);

    PowerpcInstructionKind convertKind(entryID opcode, std::string mnem);
    PowerpcInstructionKind makeRoseBranchOpcode(entryID iapi_opcode, bool isAbsolute, bool isLink);

    virtual Architecture arch() { return Arch_ppc32; };
    PowerpcInstructionKind kind;
  };
};
};

#endif
