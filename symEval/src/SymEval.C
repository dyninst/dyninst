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

#include "../h/SymEval.h"
#include "SymEvalPolicy.h"

#include "AST.h"

#include "../rose/x86InstructionSemantics.h"

#include "dyninstAPI/src/stackanalysis.h"
#include "SymEvalVisitors.h"

using namespace Dyninst;
using namespace Dyninst::InstructionAPI;
using namespace Dyninst::SymbolicEvaluation;

const AST::Ptr SymEval::Placeholder;

AST::Ptr SymEval::expand(const Assignment::Ptr &assignment) {
  // This is a shortcut version for when we only want a 
  // single assignment

  SymEval::Result res;
  // Fill it in to mark it as existing
  res[assignment] = Placeholder;
  expand(res);
  return res[assignment];
}

void SymEval::expand(Result &res) {
  // Symbolic evaluation works off an Instruction
  // so we have something to hand to ROSE. 
  for (Result::iterator i = res.begin(); i != res.end(); ++i) {
    if (i->second != Placeholder) {
      // Must've already filled it in from a previous instruction crack
      continue;
    }
    Assignment::Ptr ptr = i->first;

    expandInsn(ptr->insn(),
	       ptr->addr(),
	       res);

  }

  // Must apply the visitor to each filled in element
  for (Result::iterator i = res.begin(); i != res.end(); ++i) {
    if (i->second == Placeholder) {
      // Must not have been filled in above
      continue;
    }
    Assignment::Ptr ptr = i->first;

    // Let's experiment with simplification
    image_func *func = ptr->func();
    StackAnalysis sA(func);
    StackAnalysis::Height sp = sA.findSP(ptr->addr());
    StackAnalysis::Height fp = sA.findFP(ptr->addr());
    
    StackVisitor sv(ptr->addr(), func->symTabName(), sp, fp);
    if (i->second)
      i->second = i->second->accept(&sv);
  }
}

// Do the previous, but use a Graph as a guide for
// performing forward substitution on the AST results
void SymEval::expand(Graph::Ptr slice, Result &res) {
  // Other than the substitution this is pretty similar to the first example.
  NodeIterator gbegin, gend;
  slice->entryNodes(gbegin, gend);
  
  std::queue<Node::Ptr> worklist;
  for (; gbegin != gend; ++gbegin) {
    worklist.push(*gbegin);
  }
  std::set<Node::Ptr> visited;
  
  while (!worklist.empty()) {
    Node::Ptr ptr = worklist.front(); worklist.pop();
    AssignNode::Ptr aNode = dyn_detail::boost::dynamic_pointer_cast<AssignNode>(ptr);
    if (!aNode) continue; // They need to be AssignNodes

    if (!aNode->assign()) continue; // Could be a widen point

    //cerr << "Visiting node " << ass->assign()->format() << endl;
    if (visited.find(ptr) != visited.end()) continue;
    visited.insert(ptr);

    process(aNode, res);
    
    NodeIterator nbegin, nend;
    aNode->outs(nbegin, nend);
    for (; nbegin != nend; ++nbegin) {
      AssignNode::Ptr target = dyn_detail::boost::dynamic_pointer_cast<AssignNode>(*nbegin);
      if (!target) continue;
      if (!target->assign()) continue;
      //cerr << "\t Pushing successors " << ass2->assign()->format() << endl;
      worklist.push(*nbegin);
    }
  }
}


void SymEval::expandInsn(const InstructionAPI::Instruction::Ptr insn,
			 const uint64_t addr,
			 Result &res) {
  SgAsmx86Instruction roseInsn = convert(insn, addr);
  SymEvalPolicy policy(res, addr, insn->getArch());
  X86InstructionSemantics<SymEvalPolicy, Handle> t(policy);
  t.processInstruction(&roseInsn);
  return;
}

void SymEval::process(AssignNode::Ptr ptr,
		      SymEval::Result &dbase) {
  std::map<unsigned, Assignment::Ptr> inputMap;

  //cerr << "Calling process on " << ptr->format() << endl;

  // Don't try an expansion of a widen node...
  if (!ptr->assign()) return;

  NodeIterator begin, end;
  ptr->ins(begin, end);
  
  for (; begin != end; ++begin) {
    AssignNode::Ptr in = dyn_detail::boost::dynamic_pointer_cast<AssignNode>(*begin);
    if (!in) continue;

    Assignment::Ptr assign = in->assign();

    if (!assign) continue;

    // Find which input this assignNode maps to
    unsigned index = ptr->getAssignmentIndex(in);
    if (inputMap.find(index) == inputMap.end()) {
      inputMap[index] = assign;
    }
    else {
      // Need join operator!
      inputMap[index] = Assignment::Ptr(); // Null equivalent
    }
  }

  //cerr << "\t Input map has size " << inputMap.size() << endl;

  // All of the expanded inputs are in the parameter dbase
  // If not (like this one), add it

  AST::Ptr ast = SymEval::expand(ptr->assign());
  //cerr << "\t ... resulting in " << res->format() << endl;

  // We have an AST. Now substitute in all of its predecessors.
  for (std::map<unsigned, Assignment::Ptr>::iterator iter = inputMap.begin();
       iter != inputMap.end(); ++iter) {
    if (!iter->second) {
      // Colliding definitions; skip.
      continue;
    }

    // The region used by the current assignment...
    const AbsRegion &reg = ptr->assign()->inputs()[iter->first];

    // Create an AST around this one
    VariableAST::Ptr use = VariableAST::create(Variable(reg, ptr->addr()));

    // And substitute whatever we have in the database for that AST
    AST::Ptr definition = dbase[iter->second]; 

    if (!definition) {
      //cerr << "Odd; no expansion for " << iter->second->format() << endl;
      // Can happen if we're expanding out of order, and is generally harmless.
      continue;
    }

    ast = AST::substitute(ast, use, definition);
    //cerr << "\t result is " << res->format() << endl;
  }
  dbase[ptr->assign()] = ast;
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

    switch (rinsn.get_kind()) {
    case x86_lea: {
      assert(operands.size() == 2);
      roperands->append_operand(convert(operands[0]));
      
      SgAsmExpression *o1 = convert(operands[1]);
      // We need to wrap o1 in a memory dereference...
      SgAsmMemoryReferenceExpression *expr = new SgAsmMemoryReferenceExpression(o1);
      roperands->append_operand(expr);
      break;
    }
    case x86_push: {
      assert(operands.size() == 2); 
      roperands->append_operand(convert(operands[0]));
      break;
    }
    case x86_pop: {
      assert(operands.size() == 2);
      roperands->append_operand(convert(operands[0]));
      break;
    }
    case x86_cmpxchg: {
      assert(operands.size() == 3);
      roperands->append_operand(convert(operands[0]));
      roperands->append_operand(convert(operands[1]));
      break;
    }
    case x86_movsb:
    case x86_movsw:
    case x86_movsd: {
      // No operands
      break;
    }
    case x86_repne_cmpsb:
    case x86_repne_cmpsw:
    case x86_repne_cmpsd:
    case x86_repe_cmpsb:
    case x86_repe_cmpsw:
    case x86_repe_cmpsd:
    case x86_cmpsb:
    case x86_cmpsw:
    case x86_cmpsd: {
      // No operands
      break;
    }
    case x86_stosb:
    case x86_stosw:
    case x86_stosd: {
      // Also, no operands
      break;
    }
    case x86_jcxz:
    case x86_jecxz: {
      assert(operands.size() == 2); 
      roperands->append_operand(convert(operands[0]));
      break;
    }
    case x86_cbw:
    case x86_cwde:
    case x86_cwd:
    case x86_cdq: {
      // Nada
      break;
    }
    default: {
      for (std::vector<InstructionAPI::Operand>::iterator opi = operands.begin(),
	     ope = operands.end();
	   opi != ope;
	   ++opi) {
	InstructionAPI::Operand &currOperand = *opi;
	roperands->append_operand(convert(currOperand));
      }
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

