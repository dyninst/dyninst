#pragma once

#if !defined(_EXPRESSION_CONVERSION_VISITOR_H_)
#define _EXPRESSION_CONVERSION_VISITOR_H_


#include "dyn_regs.h"

class SgAsmx86Instruction;
class SgAsmExpression;
class SgAsmPowerpcInstruction;
class SgAsmOperandList;
class SgAsmx86RegisterReferenceExpression;
class SgAsmPowerpcRegisterReferenceExpression;

#include "external/rose/rose-compat.h"
#include "external/rose/powerpcInstructionEnum.h"
#include "Visitor.h"

#if !defined(_MSC_VER)
#include <stdint.h>
#else
#include "external/stdint-win.h"
#endif

#include <list>

namespace Dyninst
{
  namespace InstructionAPI
  {
    class RegisterAST;
  }
  namespace SymbolicEvaluation
  {
    class ExpressionConversionVisitor : public InstructionAPI::Visitor {
      typedef SgAsmPowerpcRegisterReferenceExpression regRef;
      typedef PowerpcRegisterClass regClass;
      typedef PowerpcConditionRegisterAccessGranularity regField;

    public:
    SYMEVAL_EXPORT ExpressionConversionVisitor(Architecture a, uint64_t ad) :
      roseExpression(NULL), arch(a), addr(ad) {};
      
      SYMEVAL_EXPORT SgAsmExpression *getRoseExpression() { return roseExpression; }
      
      SYMEVAL_EXPORT virtual void visit(InstructionAPI::BinaryFunction *binfunc);
      SYMEVAL_EXPORT virtual void visit(InstructionAPI::Immediate *immed);
      SYMEVAL_EXPORT virtual void visit(InstructionAPI::RegisterAST *regast);
      SYMEVAL_EXPORT virtual void visit(InstructionAPI::Dereference *deref);
      
    private:

      SgAsmExpression* archSpecificRegisterProc(InstructionAPI::RegisterAST* regast, uint64_t addr);
      SgAsmExpression* makeSegRegExpr();

      SgAsmExpression *roseExpression;
      Architecture arch;
      std::list<SgAsmExpression*> m_stack;
      uint64_t addr;
    };
  }
}

#endif
