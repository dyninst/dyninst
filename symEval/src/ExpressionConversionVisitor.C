#include ".\expressionconversionvisitor.h"

#include "Register.h"
#include "Immediate.h"
#include "BinaryFunction.h"
#include "Dereference.h"

using namespace Dyninst;
using namespace Dyninst::InstructionAPI;

#include "../rose/x86InstructionSemantics.h"
#include "../rose/powerpcInstructionSemantics.h"

namespace Dyninst
{
	namespace SymbolicEvaluation
	{
		template <Architecture a>
			void ExpressionConversionVisitor<a>::visit(InstructionAPI::Immediate* immed) {
			// no children

			const Result &value = immed->eval();

			// TODO rose doesn't distinguish signed/unsigned within the value itself,
			// only at operations?

			// TODO rose doesn't handle large values (XMM?)

			// build different kind of rose value object based on type
			switch (value.type) {
				case s8:
				case u8:
					roseExpression = new SgAsmByteValueExpression(value.val.u8val);
					break;
				case s16:
				case u16:
					roseExpression = new SgAsmWordValueExpression(value.val.u16val);
					break;
				case s32:
				case u32:
					roseExpression = new SgAsmDoubleWordValueExpression(value.val.u32val);
					break;
				case s64:
				case u64:
					roseExpression = new SgAsmQuadWordValueExpression(value.val.u64val);
					break;
				case sp_float:
					roseExpression = new SgAsmSingleFloatValueExpression(value.val.floatval);
					break;
				case dp_float:
					roseExpression = new SgAsmDoubleFloatValueExpression(value.val.dblval);
					break;
				default:
					roseExpression = NULL;
					// error!
			}
			m_stack.push_front(roseExpression);
		}


		template <Architecture a>
		void ExpressionConversionVisitor<a>::visit(RegisterAST* regast) {
			// has no children

			m_stack.push_front(ConversionArchTraits<a>::archSpecificRegisterProc(regast));
			roseExpression = m_stack.front();
			return;
		}

		template <Architecture a>
		void ExpressionConversionVisitor<a>::visit(Dereference* deref) {
			// get child
			assert(m_stack.size());
			SgAsmExpression *toderef =
					m_stack.front();
			m_stack.pop_front();
			if(toderef == NULL) {
				roseExpression = NULL;
				return;
			}
			SgAsmType *type;

			// TODO fix some mismatched types?
			// pick correct type
			switch (deref->eval().type)
			{
				case s8:
				case u8:
					type = new SgAsmTypeByte();
					break;
				case s16:
				case u16:
					type = new SgAsmTypeWord();
					break;
				case s32:
				case u32:
					type = new SgAsmTypeDoubleWord();
					break;
				case s64:
				case u64:
					type = new SgAsmTypeQuadWord();
					break;
				case sp_float:
					type = new SgAsmTypeSingleFloat();
					break;
				case dp_float:
					type = new SgAsmTypeDoubleFloat();
					break;
				default:
					type = NULL;
					// error
			}


			SgAsmExpression *segReg = ConversionArchTraits<a>::makeSegRegExpr();
			SgAsmMemoryReferenceExpression* result = new SgAsmMemoryReferenceExpression(toderef, segReg);
			result->set_type(type);
			roseExpression = result;
		}
		SgAsmExpression* ConversionArchTraits<Arch_x86>::archSpecificRegisterProc(InstructionAPI::RegisterAST* regast)
		{
			int rreg_class;
			int rreg_num;
			int rreg_pos;

			MachRegister machReg = regast->getID();
			if(machReg.isPC()) return NULL;
			machReg.getROSERegister(rreg_class, rreg_num, rreg_pos);

			SgAsmExpression* roseRegExpr = new regRef((regClass)rreg_class,
													rreg_num,
													(regField)rreg_pos);
			return roseRegExpr;
		}

		SgAsmExpression* ConversionArchTraits<Arch_x86>::makeSegRegExpr()
		{
			return new SgAsmx86RegisterReferenceExpression(x86_regclass_segment,
					x86_segreg_none, x86_regpos_all);
		}

		SgAsmExpression* ConversionArchTraits<Arch_ppc32>::archSpecificRegisterProc(InstructionAPI::RegisterAST* regast)
		{
			int rreg_class;
			int rreg_num;
			int rreg_pos = (int)powerpc_condreggranularity_whole;

			MachRegister machReg = regast->getID();
			if(machReg.isPC()) return NULL;
			machReg.getROSERegister(rreg_class, rreg_num, rreg_pos);

			SgAsmExpression* roseRegExpr = new regRef((regClass)rreg_class,
													rreg_num,
													(regField)rreg_pos);
			return roseRegExpr;
		}

	}
}
