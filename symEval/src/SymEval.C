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
}


void SymEval::expandInsn(const InstructionAPI::Instruction::Ptr insn,
			 const uint64_t addr,
			 Result &res) {
  SgAsmx86Instruction roseInsn = convert(insn, addr);
  SymEvalPolicy policy(res);
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

    X86RegisterClass rreg_class;
    int rreg_num;
    X86PositionInRegister rreg_pos;

    unsigned int ireg_id = regast->getID();

    // this will likely need to change when rose supports 64-bit

    // TODO resolve naming inconsistencies between register naming
          // are there 16 cr, dr, xmm regs?
          // don't care about tr in rose?
          // what are spl, bpl, sil, dil

    // set register class and number
    switch (ireg_id) {
        case r_AH:
        case r_AL:
        case r_AX:
        case r_eAX:
        case r_EAX:
        case r_rAX:
        case r_RAX:
            rreg_class = x86_regclass_gpr;
            rreg_num = x86_gpr_ax;
            break;
        case r_BH:
        case r_BL:
        case r_BX:
        case r_eBX:
        case r_EBX:
        case r_rBX:
        case r_RBX:
            rreg_class = x86_regclass_gpr;
            rreg_num = x86_gpr_bx;
            break;
        case r_CH:
        case r_CL:
        case r_CX:
        case r_eCX:
        case r_ECX:
        case r_rCX:
        case r_RCX:
            rreg_class = x86_regclass_gpr;
            rreg_num = x86_gpr_cx;
            break;
        case r_DH:
        case r_DL:
        case r_DX:
        case r_eDX:
        case r_EDX:
        case r_rDX:
        case r_RDX:
            rreg_class = x86_regclass_gpr;
            rreg_num = x86_gpr_dx;
            break;
        case r_SI:
        case r_eSI:
        case r_ESI:
        case r_rSI:
        case r_RSI:
            rreg_class = x86_regclass_gpr;
            rreg_num = x86_gpr_si;
            break;
        case r_DI:
        case r_eDI:
        case r_EDI:
        case r_rDI:
        case r_RDI:
            rreg_class = x86_regclass_gpr;
            rreg_num = x86_gpr_di;
            break;
        case r_eSP:
        case r_ESP:
        case r_rSP:
        case r_RSP:
            rreg_class = x86_regclass_gpr;
            rreg_num = x86_gpr_sp;
            break;
        case r_eBP:
        case r_EBP:
        case r_rBP:
        case r_RBP:
            rreg_class = x86_regclass_gpr;
            rreg_num = x86_gpr_bp;
            break;
        case r_EFLAGS:
            rreg_class = x86_regclass_flags;
            rreg_num = 0;
            break;
        case r_CS:
            rreg_class = x86_regclass_segment;
            rreg_num = x86_segreg_cs;
            break;
        case r_DS:
            rreg_class = x86_regclass_segment;
            rreg_num = x86_segreg_ds;
            break;
        case r_ES:
            rreg_class = x86_regclass_segment;
            rreg_num = x86_segreg_es;
            break;
        case r_FS:
            rreg_class = x86_regclass_segment;
            rreg_num = x86_segreg_fs;
            break;
        case r_GS:
            rreg_class = x86_regclass_segment;
            rreg_num = x86_segreg_gs;
            break;
        case r_SS:
            rreg_class = x86_regclass_segment;
            rreg_num = x86_segreg_ss;
            break;
        case r_EIP:
        case r_RIP:
            rreg_class = x86_regclass_ip;
            rreg_num = 0;
            break;
        case r_XMM0:
            rreg_class = x86_regclass_xmm;
            rreg_num = 0;
            break;
        case r_XMM1:
            rreg_class = x86_regclass_xmm;
            rreg_num = 1;
            break;
        case r_XMM2:
            rreg_class = x86_regclass_xmm;
            rreg_num = 2;
            break;
        case r_XMM3:
            rreg_class = x86_regclass_xmm;
            rreg_num = 3;
            break;
        case r_XMM4:
            rreg_class = x86_regclass_xmm;
            rreg_num = 4;
            break;
        case r_XMM5:
            rreg_class = x86_regclass_xmm;
            rreg_num = 5;
            break;
        case r_XMM6:
            rreg_class = x86_regclass_xmm;
            rreg_num = 6;
            break;
        case r_XMM7:
            rreg_class = x86_regclass_xmm;
            rreg_num = 7;
            break;
        case r_MM0:
            rreg_class = x86_regclass_mm;
            rreg_num = 0;
            break;
        case r_MM1:
            rreg_class = x86_regclass_mm;
            rreg_num = 1;
            break;
        case r_MM2:
            rreg_class = x86_regclass_mm;
            rreg_num = 2;
            break;
        case r_MM3:
            rreg_class = x86_regclass_mm;
            rreg_num = 3;
            break;
        case r_MM4:
            rreg_class = x86_regclass_mm;
            rreg_num = 4;
            break;
        case r_MM5:
            rreg_class = x86_regclass_mm;
            rreg_num = 5;
            break;
        case r_MM6:
            rreg_class = x86_regclass_mm;
            rreg_num = 6;
            break;
        case r_MM7:
            rreg_class = x86_regclass_mm;
            rreg_num = 7;
            break;
        case r_ST0:
            rreg_class = x86_regclass_st;
            rreg_num = 0;
            break;
        case r_ST1:
            rreg_class = x86_regclass_st;
            rreg_num = 1;
            break;
        case r_ST2:
            rreg_class = x86_regclass_st;
            rreg_num = 2;
            break;
        case r_ST3:
            rreg_class = x86_regclass_st;
            rreg_num = 3;
            break;
        case r_ST4:
            rreg_class = x86_regclass_st;
            rreg_num = 4;
            break;
        case r_ST5:
            rreg_class = x86_regclass_st;
            rreg_num = 5;
            break;
        case r_ST6:
            rreg_class = x86_regclass_st;
            rreg_num = 6;
            break;
        case r_ST7:
            rreg_class = x86_regclass_st;
            rreg_num = 7;
            break;
        case r_DR0:
            rreg_class = x86_regclass_dr;
            rreg_num = 0;
            break;
        case r_DR1:
            rreg_class = x86_regclass_dr;
            rreg_num = 1;
            break;
        case r_DR2:
            rreg_class = x86_regclass_dr;
            rreg_num = 2;
            break;
        case r_DR3:
            rreg_class = x86_regclass_dr;
            rreg_num = 3;
            break;
        case r_DR4:
            rreg_class = x86_regclass_dr;
            rreg_num = 4;
            break;
        case r_DR5:
            rreg_class = x86_regclass_dr;
            rreg_num = 5;
            break;
        case r_DR6:
            rreg_class = x86_regclass_dr;
            rreg_num = 6;
            break;
        case r_DR7:
            rreg_class = x86_regclass_dr;
            rreg_num = 7;
            break;
        case r_CR0:
            rreg_class = x86_regclass_cr;
            rreg_num = 0;
            break;
        case r_CR1:
            rreg_class = x86_regclass_cr;
            rreg_num = 1;
            break;
        case r_CR2:
            rreg_class = x86_regclass_cr;
            rreg_num = 2;
            break;
        case r_CR3:
            rreg_class = x86_regclass_cr;
            rreg_num = 3;
            break;
        case r_CR4:
            rreg_class = x86_regclass_cr;
            rreg_num = 4;
            break;
        case r_CR5:
            rreg_class = x86_regclass_cr;
            rreg_num = 5;
            break;
        case r_CR6:
            rreg_class = x86_regclass_cr;
            rreg_num = 6;
            break;
        case r_CR7:
            rreg_class = x86_regclass_cr;
            rreg_num = 7;
            break; 
        default:
            rreg_class = x86_regclass_unknown;
            rreg_num = 0;
            rreg_pos = x86_regpos_unknown;
    }

    // set register position
    // TODO will be ever be in 16-bit mode? If so, then fix e implied regs.
    switch (ireg_id) {
        case r_AH:
        case r_BH:
        case r_CH:
        case r_DH:
            rreg_pos = x86_regpos_high_byte;
            break;
        case r_AL:
        case r_BL:
        case r_CL:
        case r_DL:
            rreg_pos = x86_regpos_low_byte;
            break;
        case r_AX:
        case r_BX:
        case r_CX:
        case r_DX:
        case r_SI:
        case r_DI:
            rreg_pos = x86_regpos_word;
            break;
        case r_eAX:
        case r_eBX:
        case r_eCX:
        case r_eDX:
        case r_eSI:
        case r_eDI:
        case r_eSP:
        case r_eBP:
        case r_EAX:
        case r_EBX:
        case r_ECX:
        case r_EDX:
        case r_ESI:
        case r_EDI:
        case r_ESP:
        case r_EBP:
            rreg_pos = x86_regpos_dword;
            break;
        case r_rAX:
        case r_rBX:
        case r_rCX:
        case r_rDX:
        case r_rSI:
        case r_rDI:
        case r_rSP:
        case r_rBP:
	  std::cerr << "FIXME forcing 32-bit conversion!" << std::endl;
            rreg_pos = x86_regpos_dword;
	    break;
        case r_RAX:
        case r_RBX:
        case r_RCX:
        case r_RDX:
        case r_RSI:
        case r_RDI:
        case r_RSP:
        case r_RBP:
        case r_R8:
        case r_R9:
        case r_R10:
        case r_R11:
        case r_R12:
        case r_R13:
        case r_R14:
        case r_R15:
            rreg_pos = x86_regpos_qword;
            break;
        default:
	  fprintf(stderr, "Odd: got register %d\n", ireg_id);
            rreg_pos = x86_regpos_all;
    }

    roseExpression = new SgAsmx86RegisterReferenceExpression(rreg_class, rreg_num, rreg_pos);
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

