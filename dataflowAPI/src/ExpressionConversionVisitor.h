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

#if defined(_MSC_VER)
#include "external/stdint-win.h"
#else
#include <stdint.h>
#endif

#include <list>

namespace Dyninst {
namespace InstructionAPI {
class RegisterAST;
}
namespace DataflowAPI {
class ExpressionConversionVisitor : public InstructionAPI::Visitor {
  typedef SgAsmPowerpcRegisterReferenceExpression regRef;
  typedef PowerpcRegisterClass regClass;
  typedef PowerpcConditionRegisterAccessGranularity regField;

 public:
  DATAFLOW_EXPORT ExpressionConversionVisitor(Architecture a, uint64_t ad,
                                              uint64_t s)
      : roseExpression(NULL), arch(a), addr(ad), size(s){};

  DATAFLOW_EXPORT SgAsmExpression *getRoseExpression() {
    return roseExpression;
  }

  DATAFLOW_EXPORT virtual void visit(InstructionAPI::BinaryFunction *binfunc);
  DATAFLOW_EXPORT virtual void visit(InstructionAPI::Immediate *immed);
  DATAFLOW_EXPORT virtual void visit(InstructionAPI::RegisterAST *regast);
  DATAFLOW_EXPORT virtual void visit(InstructionAPI::Dereference *deref);

 private:
  SgAsmExpression *archSpecificRegisterProc(InstructionAPI::RegisterAST *regast,
                                            uint64_t addr, uint64_t size);
  SgAsmExpression *makeSegRegExpr();

  SgAsmExpression *roseExpression;
  Architecture arch;
  std::list<SgAsmExpression *> m_stack;
  uint64_t addr;
  uint64_t size;
};
}
}

#endif
