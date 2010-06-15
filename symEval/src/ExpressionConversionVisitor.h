#pragma once

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

namespace Dyninst
{
	namespace InstructionAPI
	{
		class RegisterAST;
	}
	namespace SymbolicEvaluation
	{
		struct ConversionArchTraitsBase
		{
			virtual SgAsmExpression* archSpecificRegisterProc(InstructionAPI::RegisterAST* regast) = 0;
			virtual SgAsmExpression* makeSegRegExpr() = 0;
			virtual ~ConversionArchTraitsBase() {}
		};

		template <Architecture a>
				struct ConversionArchTraits : public ConversionArchTraitsBase
		{
			virtual SgAsmExpression* archSpecificRegisterProc(InstructionAPI::RegisterAST* regast)
			{
				return NULL;
			}
			virtual SgAsmExpression* makeSegRegExpr()
			{
				return NULL;
			}
			virtual ~ConversionArchTraits() {}
			typedef SgAsmPowerpcRegisterReferenceExpression regRef;
			typedef PowerpcRegisterClass regClass;
			typedef PowerpcConditionRegisterAccessGranularity regField;
		};
            
		template <>
				struct ConversionArchTraits<Arch_x86> : public ConversionArchTraitsBase
		{
			virtual SgAsmExpression* archSpecificRegisterProc(InstructionAPI::RegisterAST* regast);
			virtual SgAsmExpression* makeSegRegExpr();
			typedef SgAsmx86RegisterReferenceExpression regRef;
			typedef X86RegisterClass regClass;
			typedef X86PositionInRegister regField;
			virtual ~ConversionArchTraits<Arch_x86>() {}
		};
            
		template <>
				struct ConversionArchTraits<Arch_ppc32> : public ConversionArchTraitsBase
		{
			virtual SgAsmExpression* archSpecificRegisterProc(InstructionAPI::RegisterAST* regast);
			virtual SgAsmExpression* makeSegRegExpr()
			{
				return NULL;
			}
			virtual ~ConversionArchTraits<Arch_ppc32>() {}
			typedef SgAsmPowerpcRegisterReferenceExpression regRef;
			typedef PowerpcRegisterClass regClass;
			typedef PowerpcConditionRegisterAccessGranularity regField;

		};

		template <Architecture a = Arch_x86>
		class ExpressionConversionVisitor : public InstructionAPI::Visitor, public ConversionArchTraits<a>
		{
			public:
				typedef typename ConversionArchTraits<a>::regRef regRef;
				typedef typename ConversionArchTraits<a>::regClass regClass;
				typedef typename ConversionArchTraits<a>::regClass regField;
				ExpressionConversionVisitor() { roseExpression = NULL; }

				SgAsmExpression *getRoseExpression() { return roseExpression; }

				virtual void visit(InstructionAPI::BinaryFunction *binfunc);
				virtual void visit(InstructionAPI::Immediate *immed);
				virtual void visit(InstructionAPI::RegisterAST *regast);
				virtual void visit(InstructionAPI::Dereference *deref);

			private:
				SgAsmExpression *roseExpression;
				std::list<SgAsmExpression*> m_stack;
		};
	}
}
