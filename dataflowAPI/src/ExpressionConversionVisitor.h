/*
 * See the dyninst/COPYRIGHT file for copyright information.
 * 
 * We provide the Paradyn Tools (below described as "Paradyn")
 * on an AS IS basis, and do not warrant its validity or performance.
 * We reserve the right to update, modify, or discontinue this
 * software at any time.  We shall have no obligation to supply such
 * updates or modifications or any other form of support to you.
 * 
 * By your use of Paradyn, you understand and agree that we (or any
 * other person or entity with proprietary rights in Paradyn) are
 * under no obligation to provide either maintenance services,
 * update services, notices of latent defects, or correction of
 * defects for Paradyn.
 * 
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

#if !defined(_EXPRESSION_CONVERSION_VISITOR_H_)
#define _EXPRESSION_CONVERSION_VISITOR_H_


#include "dyninst_visibility.h"
#include "Architecture.h"

class SgAsmx86Instruction;
class SgAsmExpression;
class SgAsmPowerpcInstruction;
class SgAsmOperandList;
class SgAsmx86RegisterReferenceExpression;
class SgAsmPowerpcRegisterReferenceExpression;

#include "external/rose/rose-compat.h"
#include "external/rose/powerpcInstructionEnum.h"
#include "Visitor.h"

#include <stdint.h>

#include <list>

namespace Dyninst
{
  namespace InstructionAPI
  {
    class RegisterAST;
  }
  namespace DataflowAPI
  {
    class ExpressionConversionVisitor : public InstructionAPI::Visitor {
      typedef SgAsmPowerpcRegisterReferenceExpression regRef;
      typedef PowerpcRegisterClass regClass;
      typedef PowerpcConditionRegisterAccessGranularity regField;

    public:
    DYNINST_EXPORT ExpressionConversionVisitor(Architecture a, uint64_t ad, uint64_t s) :
      roseExpression(NULL), arch(a), addr(ad), size(s) {}
      
      DYNINST_EXPORT SgAsmExpression *getRoseExpression() { return roseExpression; }
      
      DYNINST_EXPORT virtual void visit(InstructionAPI::BinaryFunction *binfunc);
      DYNINST_EXPORT virtual void visit(InstructionAPI::Immediate *immed);
      DYNINST_EXPORT virtual void visit(InstructionAPI::RegisterAST *regast);
      DYNINST_EXPORT virtual void visit(InstructionAPI::Dereference *deref);
      DYNINST_EXPORT virtual void visit(InstructionAPI::MultiRegisterAST *multiregast);
      
    private:

      SgAsmExpression* archSpecificRegisterProc(InstructionAPI::RegisterAST* regast, uint64_t addr, uint64_t size);
      SgAsmExpression* makeSegRegExpr();

      SgAsmExpression *roseExpression;
      Architecture arch;
      std::list<SgAsmExpression*> m_stack;
      uint64_t addr;
      uint64_t size;
    };
  }
}

#endif
