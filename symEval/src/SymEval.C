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
#include "../rose/powerpcInstructionSemantics.h"
#include "dyninstAPI/src/stackanalysis.h"
#include "SymEvalVisitors.h"

using namespace Dyninst;
using namespace InstructionAPI;
using namespace SymbolicEvaluation;


template<Architecture a>
AST::Ptr SymEval<a>::expand(const Assignment::Ptr &assignment) {
  // This is a shortcut version for when we only want a
  // single assignment

    Result_t res;
  // Fill it in to mark it as existing
  res[assignment] = Placeholder;
  expand(res);
  return res[assignment];
}

template <Architecture a>
void SymEval<a>::expand(Result_t &res) {
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

    StackVisitor sv(func->symTabName(), sp, fp);
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
        Result_t::iterator i = res.begin();
        if (i == res.end()) return;

        Assignment::Ptr ptr = i->first;
        cerr << "\t\t Expanding insn " << ptr->insn()->format() << endl;

        SymEval<a>::expandInsn(ptr->insn(),
                   ptr->addr(),
                   res);
  // Let's experiment with simplification
        image_func *func = ptr->func();
        StackAnalysis sA(func);
        StackAnalysis::Height sp = sA.findSP(ptr->addr());
        StackAnalysis::Height fp = sA.findFP(ptr->addr());

        StackBindEval sbe(func->symTabName().c_str(), sp, fp);

        for (i = res.begin(); i != res.end(); ++i) {
            i->second = sbe.simplify(i->second);
        }
    }


void SymEvalArchTraits<Arch_x86>::processInstruction(SageInstruction_t* roseInsn,
                                                    SymEvalPolicy& policy)
{
    X86InstructionSemantics<SymEvalPolicy, Handle> t(policy);
    t.processInstruction(roseInsn);
}        
        
void SymEvalArchTraits<Arch_ppc32>::processInstruction(SageInstruction_t* roseInsn,
                                                      SymEvalPolicy& policy)
{
    PowerpcInstructionSemantics<SymEvalPolicy, Handle> t(policy);
    t.processInstruction(roseInsn);
}


template <Architecture a>
void SymEval<a>::expandInsn(const InstructionAPI::Instruction::Ptr insn,
			 const uint64_t addr,
			 Result_t &res) {
  SageInstruction_t* roseInsn = convert(insn, addr);
  SymEvalPolicy policy(res, addr, insn->getArch());
  SymEvalArchTraits::processInstruction(roseInsn, policy);    
  return;
}

SgAsmExpression* SymEvalArchTraits<Arch_ppc32>::convertOperand(InstructionKind_t opcode,
        unsigned int which,
        SgAsmExpression* operand)
{
  static SgAsmExpression* stash;
  if(opcode >= powerpc_lbz && opcode <= powerpc_lwzx && which > 1) return NULL;
  if(opcode >= powerpc_stb && opcode <= powerpc_stwx && which > 1) return NULL;
    return operand;
}

SgAsmExpression* SymEvalArchTraits<Arch_x86>::convertOperand(InstructionKind_t opcode,
        unsigned int which,
        SgAsmExpression* operand)
{
    if((opcode == x86_lea) && (which == 1))
    {
        cerr << "wrapping LEA operand in deref" << endl;
        // We need to wrap o1 in a memory dereference...
        SgAsmMemoryReferenceExpression *expr = new SgAsmMemoryReferenceExpression(operand);
        return expr;
    }
    else if((opcode == x86_push || opcode == x86_pop) && (which > 0))
    {
        return NULL;
    }
    else
    {
        return operand;
    }
}

template<Architecture a>
void SymEval<a>::process(AssignNode::Ptr ptr,
		      Result_t &dbase) {
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
    AbsRegionAST::Ptr use = AbsRegionAST::create(reg);

    // And substitute whatever we have in the database for that AST
    AST::Ptr definition = dbase[iter->second]; 

    if (!definition) {
      cerr << "Odd; no expansion for " << iter->second->format() << endl;
      // Can happen if we're expanding out of order, and is generally harmless.
      continue;
    }

    ast = AST::substitute(ast, use, definition);
    //cerr << "\t result is " << res->format() << endl;
  }
  dbase[ptr->assign()] = ast;
}

PowerpcInstructionKind makeRoseBranchOpcode(entryID iapi_opcode, bool isAbsolute, bool isLink)
{
    switch(iapi_opcode)
    {
        case power_op_b:
            if(isAbsolute && isLink) return powerpc_bla;
            if(isAbsolute) return powerpc_ba;
            if(isLink) return powerpc_bl;
            return powerpc_b;
        case power_op_bc:
            if(isAbsolute && isLink) return powerpc_bcla;
            if(isAbsolute) return powerpc_bca;
            if(isLink) return powerpc_bcl;
            return powerpc_bc;
        case power_op_bcctr:
            assert(!isAbsolute);
            if(isLink) return powerpc_bcctrl;
            return powerpc_bcctr;
        case power_op_bclr:
            assert(!isAbsolute);
            if(isLink) return powerpc_bclrl;
            return powerpc_bclr;
        default:
            assert(!"makeRoseBranchOpcode called with unknown branch opcode!");
            return powerpc_unknown_instruction;
    }
}

bool SymEvalArchTraits<Arch_ppc32>::handleSpecialCases(entryID iapi_opcode,
        SageInstruction_t& rose_insn,
        SgAsmOperandList* rose_operands)
{
    switch(iapi_opcode)
    {
        case power_op_b:
        case power_op_bc:
        case power_op_bcctr:
        case power_op_bclr:
        {
            unsigned int raw = 0;
            unsigned int branch_target = 0;
            unsigned int bo = 0, bi = 0;
            std::vector<unsigned char> bytes = rose_insn.get_raw_bytes();
            for(unsigned i = 0; i < bytes.size(); i++)
            {
                raw = raw << 8;
                raw |= bytes[i];
            }
            bool isAbsolute = (bool)(raw & 0x00000002);
            bool isLink = (bool)(raw & 0x00000001);
            rose_insn.set_kind(makeRoseBranchOpcode(iapi_opcode, isAbsolute, isLink));
            if(power_op_b == iapi_opcode) {
                branch_target = ((raw >> 2) & 0x00FFFFFF) << 2;
            } else {
                if(power_op_bc == iapi_opcode) {
                    branch_target = ((raw >> 2) & 0x0000CFFF) << 2;
                }
                bo = ((raw >> 21) & 0x0000001F);
                bi = ((raw >> 16) & 0x0000001F);
            }
            if(power_op_b != iapi_opcode) {
                roperands->append_operand(new SgAsmByteValueExpression(bo));
		cerr << "appending BO operand: " << bo << endl;
		rose_operands->append_operand(new SgAsmByteValueExpression(bo));
		cerr << "appending BI operand: CR bit " << bi << endl;
		rose_operands->append_operand(new SgAsmPowerpcRegisterReferenceExpression(powerpc_regclass_cr, bi,
										    powerpc_condreggranularity_bit));
            }
            if(branch_target) {
	      cerr << "appending branch target operand: " << branch_target << endl;
                rose_operands->append_operand(new SgAsmDoubleWordValueExpression(branch_target));
            } else if(power_op_bcctr == iapi_opcode) {
	      cerr << "appending branch target operand: count register" << endl;
                rose_operands->append_operand(new SgAsmPowerpcRegisterReferenceExpression(powerpc_regclass_spr, powerpc_spr_ctr));
            } else {
                assert(power_op_bclr == iapi_opcode);
		cerr << "appending branch target operand: link register" << endl;
                rose_operands->append_operand(new SgAsmPowerpcRegisterReferenceExpression(powerpc_regclass_spr, powerpc_spr_lr));
            }
            return true;
        }
            break;
        default:
            return false;
    }
    
}

void SymEvalArchTraits<Arch_ppc32>::handleSpecialCases(InstructionAPI::Instruction::Ptr insn,
        std::vector<InstructionAPI::Operand>& operands)
{
    if(insn->writesMemory())
        std::swap(operands[0], operands[1]);
}


SgAsmExpression* SymEvalArchTraits<Arch_ppc32>::convertOperand(InstructionKind_t ,
        unsigned int ,
        SgAsmExpression* operand)
{
    // add filtering as needed...
    return operand;
}

SgAsmExpression* SymEvalArchTraits<Arch_x86>::convertOperand(InstructionKind_t opcode,
        unsigned int which,
        SgAsmExpression* operand)
{
    if((opcode == x86_lea) && (which == 1))
    {
        // We need to wrap o1 in a memory dereference...
        SgAsmMemoryReferenceExpression *expr = new SgAsmMemoryReferenceExpression(operand);
        return expr;
    }
    else if((opcode == x86_push || opcode == x86_pop) && (which > 0))
    {
        fprintf(stderr, "push/pop detected, skipping operand %d\n", which);
        return NULL;
    }
    else
    {
        return operand;
    }
}

template<Architecture a>
typename SymEval<a>::SageInstruction_t
SymEval<a>::convert(const InstructionAPI::Instruction::Ptr &insn, uint64_t addr) {
    SageInstruction_t rinsn;

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

    if(SymEvalArchTraits<a>::handleSpecialCases(insn->getOperation().getID(), rinsn, roperands))
    {
        rinsn.set_operandList(roperands);
        return rinsn;
    }
    std::vector<InstructionAPI::Operand> operands;
    insn->getOperands(operands);
    SymEvalArchTraits<a>::handleSpecialCases(insn->getOperation().getID(), operands)
    int i = 0;
    fprintf(stderr, "converting insn %s\n", insn->format().c_str());
    for (std::vector<InstructionAPI::Operand>::iterator opi = operands.begin();
             opi != operands.end();
             ++opi, ++i) {
            InstructionAPI::Operand &currOperand = *opi;
            SgAsmExpression* converted = convert(currOperand);
            fprintf(stderr, "converted operand %s, result was %s\n", currOperand.format().c_str(),
                    converted ? "not NULL" : "NULL");
            SgAsmExpression* final = SymEvalArchTraits<a>::convertOperand(rinsn.get_kind(), i, converted);
            if(final != NULL) {
                fprintf(stderr, "appending operand %d (%s)\n", i, currOperand.format().c_str());
                roperands->append_operand(final);
            }
    }
    rinsn.set_operandList(roperands);

    return rinsn;
}

template <Architecture a>
SgAsmExpression *SymEval<a>::convert(const InstructionAPI::Operand &operand) {
    return convert(operand.getValue());
}

template <Architecture a>
        SgAsmExpression *SymEval<a>::convert(const Expression::Ptr expression) {
    Visitor_t visitor;
    expression->apply(&visitor);
    return visitor.getRoseExpression();
}


template <Architecture a>
void ExpressionConversionVisitor<a>::visit(BinaryFunction* binfunc) {
    assert(m_stack.size() >= 2);
    SgAsmExpression *lhs =
            m_stack.front();
    m_stack.pop_front();
    SgAsmExpression *rhs =
            m_stack.front();
    m_stack.pop_front();
    // If the RHS didn't convert, that means it should disappear
    // And we are just left with the LHS
    if(!rhs && !lhs) {
        roseExpression = NULL;
        return;
    }
    if (!rhs)
    {
      roseExpression = lhs;
      return;
    }
    if(!lhs)
    {
        roseExpression = rhs;
        return;
    }
    
    // now build either add or multiply
    if (binfunc->isAdd())
        roseExpression = new SgAsmBinaryAdd(lhs, rhs);
    else if (binfunc->isMultiply())
        roseExpression = new SgAsmBinaryMultiply(lhs, rhs);
    else roseExpression = NULL; // error
}

template <Architecture a>
void ExpressionConversionVisitor<a>::visit(Immediate* immed) {
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

virtual SgAsmExpression* ConversionArchTraits<Arch_ppc32>::archSpecificRegisterProc(InstructionAPI::RegisterAST* regast)
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
virtual SgAsmExpression* ConversionArchTraits<Arch_ppc32>::makeSegRegExpr()
{
    return NULL;
}



template class ExpressionConversionVisitor<Arch_x86>;
template class ExpressionConversionVisitor<Arch_ppc32>;
template class ConversionArchTraits<Arch_x86>;
template class ConversionArchTraits<Arch_ppc32>;
template class SymEval<Arch_x86>;
template class SymEval<Arch_ppc32>;

