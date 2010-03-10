/*
 * Copyright (c) 1996-2007 Barton P. Miller
 * 
 * We provide the Paradyn Parallel Performance Tools (below
 * described as "Paradyn") on an AS IS basis, and do not warrant its
 * validity or performance.  We reserve the right to update, modify,
 * or discontinue this software at any time.  We shall have no
 * obligation to supply such updates or modifications or any other
 * form of support to you.
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include <string>

#include "SymEval.h"
#include "SymEvalPolicy.h"

#include "AST.h"

#include "../rose/x86InstructionSemantics.h"

#include "dyninstAPI/src/stackanalysis.h"
#include "SymEvalVisitors.h"

using namespace Dyninst;
using namespace Dyninst::InstructionAPI;
using namespace Dyninst::SymbolicEvaluation;

AST::Ptr SymEval::expand(const Assignment::Ptr &assignment) {
  // This is a shortcut version for when we only want a 
  // single assignment

  SymEval::Result res;
  // Fill it in to mark it as existing
  res[assignment] = AST::Ptr();
  expand(res);
  return res[assignment];
}

void SymEval::expand(Result &res) {
  // Symbolic evaluation works off an Instruction
  // so we have something to hand to ROSE. 
  // Let's assume (and, in fact, assert) that
  // all of the Assignments we've been handed are for
  // the same instruction.

  Result::iterator i = res.begin();
  if (i == res.end()) return;

  Assignment::Ptr ptr = i->first;

  expandInsn(ptr->insn(),
	     ptr->addr(),
	     res);
  // Let's experiment with simplification
  image_func *func = ptr->func();
  StackAnalysis sA(func);
  StackAnalysis::Height sp = sA.findSP(ptr->addr());
  StackAnalysis::Height fp = sA.findFP(ptr->addr());

  StackVisitor sv(func->symTabName(), sp, fp);

  for (i = res.begin(); i != res.end(); ++i) {
    i->second = i->second->accept(&sv);
  }

}


void SymEval::expandInsn(const InstructionAPI::Instruction::Ptr insn,
			 const uint64_t addr,
			 Result &res) {
  SgAsmx86Instruction roseInsn = convert(insn, addr);
  SymEvalPolicy policy(res, insn->getArch());
  X86InstructionSemantics<SymEvalPolicy, Handle> t(policy);
  t.processInstruction(&roseInsn);
  return;
}

SgAsmx86Instruction SymEval::convert(const InstructionAPI::Instruction::Ptr &insn, uint64_t addr) {
    SgAsmx86Instruction rinsn;

    rinsn.set_address(addr);
    rinsn.set_mnemonic(insn->format());
    rinsn.set_kind(convert(insn->getOperation().getID()));
    // semantics don't support 64-bit code
    rinsn.set_operandSize(x86_insnsize_32);
    rinsn.set_addressSize(x86_insnsize_32);

    std::vector<unsigned char> rawBytes;
    for (unsigned i = 0; i < insn->size(); ++i) rawBytes.push_back(insn->rawByte(i));
    rinsn.set_raw_bytes(rawBytes);

    // operand list
    SgAsmOperandList *roperands = new SgAsmOperandList;
    std::vector<InstructionAPI::Operand> operands;
    insn->getOperands(operands);

    // Override for LEA conversion
    if (rinsn.get_kind() == x86_lea) {
        assert(operands.size() == 2);
        roperands->append_operand(convert(operands[0]));
        
        SgAsmExpression *o1 = convert(operands[1]);
        // We need to wrap o1 in a memory dereference...
        SgAsmMemoryReferenceExpression *expr = new SgAsmMemoryReferenceExpression(o1);
        roperands->append_operand(expr);
        
    }
    else if (rinsn.get_kind() == x86_push) {
      assert(operands.size() == 2); 
      roperands->append_operand(convert(operands[0]));
    }
    else if (rinsn.get_kind() == x86_pop) {
      assert(operands.size() == 2);
      roperands->append_operand(convert(operands[0]));
    }
    else {
        for (std::vector<InstructionAPI::Operand>::iterator opi = operands.begin(),
                 ope = operands.end();
             opi != ope;
             ++opi) {
            InstructionAPI::Operand &currOperand = *opi;
            roperands->append_operand(convert(currOperand));
        }
    }
    rinsn.set_operandList(roperands);

    return rinsn;
}

SgAsmExpression *SymEval::convert(const InstructionAPI::Operand &operand) {
    return convert(operand.getValue());
}

SgAsmExpression *SymEval::convert(const Expression::Ptr expression) {
    ExpressionConversionVisitor visitor;
    expression->apply(&visitor);
    return visitor.getRoseExpression();
}

void ExpressionConversionVisitor::visit(BinaryFunction* binfunc) {
    // get two children
    vector<InstructionAST::Ptr> children;
    binfunc->getChildren(children);
    // convert each InstAST::Ptr to SgAsmExpression*
    // these casts will not fail
    SgAsmExpression *lhs = 
      SymEval::convert(dyn_detail::boost::dynamic_pointer_cast<Expression>(children[0]));
    SgAsmExpression *rhs =
      SymEval::convert(dyn_detail::boost::dynamic_pointer_cast<Expression>(children[1]));

    // ROSE doesn't expect us to include the implicit PC update
    //   along with explicit updates to the PC
    // If the current function involves the PC, ignore it 
    //   so the parent makes it disappear
    RegisterAST::Ptr lhsReg = dyn_detail::boost::dynamic_pointer_cast<RegisterAST>(children[0]);
    if (lhsReg)
    {
      if (lhsReg->getID().isPC())
      {
        roseExpression = NULL;
        return;
      }
    }
    // If the RHS didn't convert, that means it should disappear
    // And we are just left with the LHS
    if (!rhs)
    {
      roseExpression = lhs;
      return;
    }

    // now build either add or multiply
    if (binfunc->isAdd())
        roseExpression = new SgAsmBinaryAdd(lhs, rhs);
    else if (binfunc->isMultiply())
        roseExpression = new SgAsmBinaryMultiply(lhs, rhs);
    else roseExpression = NULL; // error
}

void ExpressionConversionVisitor::visit(Immediate* immed) {
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
}

void ExpressionConversionVisitor::visit(RegisterAST* regast) {
    // has no children

    int rreg_class;
    int rreg_num;
    int rreg_pos;

    MachRegister machReg = regast->getID();
    machReg.getROSERegister(rreg_class, rreg_num, rreg_pos);

    roseExpression = new SgAsmx86RegisterReferenceExpression((X86RegisterClass)rreg_class, 
                                                             rreg_num, 
                                                             (X86PositionInRegister)rreg_pos);
}

void ExpressionConversionVisitor::visit(Dereference* deref) {
    // get child
    vector<InstructionAST::Ptr> children;
    deref->getChildren(children);
    SgAsmExpression *toderef = 
      SymEval::convert(dyn_detail::boost::dynamic_pointer_cast<Expression>(children[0]));
    
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

    SgAsmx86RegisterReferenceExpression *segReg = new SgAsmx86RegisterReferenceExpression(x86_regclass_segment, x86_segreg_none, x86_regpos_all);

    SgAsmMemoryReferenceExpression *result = new SgAsmMemoryReferenceExpression(toderef, segReg);
    result->set_type(type);
    roseExpression = result;
}

