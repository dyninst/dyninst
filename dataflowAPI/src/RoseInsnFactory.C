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
#include "RoseInsnFactory.h"
//#include "../rose/x86InstructionSemantics.h"
//#include "../rose/powerpcInstructionSemantics.h"

#include "Instruction.h"
#include "Operand.h"
#include "Expression.h"
#include "Dereference.h"
#include "Immediate.h"
#include <vector>

#include "../rose/SgAsmInstruction.h"
#include "../rose/SgAsmPowerpcInstruction.h"
#include "../rose/SgAsmx86Instruction.h"
#include "../rose/SgAsmExpression.h"

#include "ExpressionConversionVisitor.h"

using namespace Dyninst;
using namespace InstructionAPI;
using namespace DataflowAPI;

SgAsmInstruction *RoseInsnFactory::convert(const InstructionAPI::Instruction::Ptr &insn, uint64_t addr) {
  SgAsmInstruction *rinsn = createInsn();
  
  rinsn->set_address(addr);
  rinsn->set_mnemonic(insn->format());
  setOpcode(rinsn, insn->getOperation().getID(), insn->getOperation().getPrefixID(), insn->getOperation().format());

  // semantics don't support 64-bit code
  setSizes(rinsn);

  //rinsn->set_operandSize(x86_insnsize_32);
  //rinsn->set_addressSize(x86_insnsize_32);
  
  std::vector<unsigned char> rawBytes;
  for (unsigned i = 0; i < insn->size(); ++i) rawBytes.push_back(insn->rawByte(i));
  rinsn->set_raw_bytes(rawBytes);
  
  // operand list
  SgAsmOperandList *roperands = new SgAsmOperandList;
  
  // std::cerr << "Converting " << insn->format(addr) << " @" << std::hex << addr << std::dec << std::endl;
  
  // std::cerr << "checking instruction: " << insn->format(addr) << " for special handling" << std::endl;
  if (handleSpecialCases(insn->getOperation().getID(), rinsn, roperands)) {
      rinsn->set_operandList(roperands);
      return rinsn;
  }

  // std::cerr << "no special handling by opcode, checking if we should mangle operands..." << std::endl;
  std::vector<InstructionAPI::Operand> operands;
  insn->getOperands(operands);
  std::cerr << "\t " << operands.size() << " operands" << std::endl;
  massageOperands(insn, operands);
  int i = 0;
  std::cerr << "converting insn " << insn->format(addr) << std::endl;
  for (std::vector<InstructionAPI::Operand>::iterator opi = operands.begin();
       opi != operands.end();
       ++opi, ++i) {
      InstructionAPI::Operand &currOperand = *opi;
      std::cerr << "Converting operand " << currOperand.format(Arch_x86, addr) << std::endl;
      roperands->append_operand(convertOperand(currOperand.getValue(), addr));
  }  
  rinsn->set_operandList(roperands);
  return rinsn;
}

SgAsmExpression *RoseInsnFactory::convertOperand(const Expression::Ptr expression, uint64_t addr) {
  if(!expression) return NULL;
  ExpressionConversionVisitor visitor(arch(), addr);
  expression->apply(&visitor);
  return visitor.getRoseExpression();
}

///////////// X86 //////////////////

SgAsmInstruction *RoseInsnX86Factory::createInsn() {
  return new SgAsmx86Instruction;
}

// Note: convertKind is defined in convertOpcodes.C

void RoseInsnX86Factory::setOpcode(SgAsmInstruction *insn, entryID opcode, prefixEntryID prefix, std::string) {
  SgAsmx86Instruction *tmp = static_cast<SgAsmx86Instruction *>(insn);
  
  tmp->set_kind(convertKind(opcode, prefix));
}

void RoseInsnX86Factory::setSizes(SgAsmInstruction *insn) {
  // FIXME when we go 64-bit...
  SgAsmx86Instruction *tmp = static_cast<SgAsmx86Instruction *>(insn);
  tmp->set_operandSize(x86_insnsize_32);
  tmp->set_addressSize(x86_insnsize_32);
}

bool RoseInsnX86Factory::handleSpecialCases(entryID, SgAsmInstruction *, SgAsmOperandList *) {
  // Does nothing?

  return false;
}

void RoseInsnX86Factory::massageOperands(const InstructionAPI::Instruction::Ptr &insn, 
					 std::vector<InstructionAPI::Operand> &operands) {
  switch (insn->getOperation().getID()) {
  case e_lea: {
    // ROSE expects there to be a "memory reference" statement wrapping the
    // address calculation. It then unwraps it. 
    Dereference::Ptr tmp = Dereference::Ptr(new Dereference(operands[1].getValue(), u32));
    operands[1] = Operand(tmp, operands[1].isRead(), operands[1].isWritten());
    operands.resize(2);
    break;  
  }
  case e_push:
  case e_pop:
    operands.resize(1);
    break;
  case e_cmpxch:
    operands.resize(2);
    break;
  case e_movsb:
  case e_movsd:
  case e_movsw:
    // No operands
    operands.clear();
    break;
  case e_cmpsb:
  case e_cmpsw:
  case e_cmpsd:
    // No operands
    operands.clear();
    break;
  case e_scasb:
  case e_scasd:
  case e_scasw:
    // Same here
    operands.clear();
    break;
  case e_stosb:
  case e_stosd:
  case e_stosw:
    // Also, no operands
    operands.clear();
    break;
  case e_jcxz_jec:
    operands.resize(1);
    break;
  case e_cbw:
  case e_cwde:
  case e_cdq:
    // Nada
    operands.clear();
    break;
  case e_popad:
  case e_pushfd:
    operands.clear();
    break;
  case e_lodsd:
  case e_lodsb:
  case e_lodsw:
      operands.clear();
      break;
  case e_pushad:
      operands.clear();
      break;
  case e_loop:
  case e_loope:
  case e_loopn:
      operands.resize(1);
      break;
  case e_ret_far:
  case e_ret_near:
	  if (operands.size() == 2) {
		  operands[0]=operands[1];
	  }
	  operands.resize(1);
	  break;
  default:
    break;
  }
}


//////////// PPC ///////////////////
// Note: convertKind is defined in convertOpcodes.C

SgAsmInstruction *RoseInsnPPCFactory::createInsn() {
  return new SgAsmPowerpcInstruction;
}

void RoseInsnPPCFactory::setOpcode(SgAsmInstruction *insn, entryID opcode, prefixEntryID /*prefix*/, std::string mnem) {
  SgAsmPowerpcInstruction *tmp = static_cast<SgAsmPowerpcInstruction *>(insn);
  kind = convertKind(opcode, mnem);
  tmp->set_kind(kind);
}


void RoseInsnPPCFactory::setSizes(SgAsmInstruction *) {
}


bool RoseInsnPPCFactory::handleSpecialCases(entryID iapi_opcode, 
					    SgAsmInstruction *insn, 
					    SgAsmOperandList *rose_operands) {
  SgAsmPowerpcInstruction *rose_insn = static_cast<SgAsmPowerpcInstruction *>(insn);

  switch(iapi_opcode) {
  case power_op_b:
  case power_op_bc:
  case power_op_bcctr:
  case power_op_bclr: {
    unsigned int raw = 0;
    int branch_target = 0;
    unsigned int bo = 0, bi = 0;
    std::vector<unsigned char> bytes = rose_insn->get_raw_bytes();
    for(unsigned i = 0; i < bytes.size(); i++) {
      raw = raw << 8;
      raw |= bytes[i];
    }
    bool isAbsolute = (bool)(raw & 0x00000002);
    bool isLink = (bool)(raw & 0x00000001);
    rose_insn->set_kind(makeRoseBranchOpcode(iapi_opcode, isAbsolute, isLink));
    if(power_op_b == iapi_opcode) {
      branch_target = ((raw >> 2) & 0x00FFFFFF) << 2;
      branch_target = (branch_target << 8) >> 8;
    } else {
      if(power_op_bc == iapi_opcode) {
	branch_target = ((raw >> 2) & 0x00003FFF) << 2;
	branch_target = (branch_target << 18) >> 18;
	//cerr << "14-bit branch target: " << branch_target << endl;
      }
      bo = ((raw >> 21) & 0x0000001F);
      bi = ((raw >> 16) & 0x0000001F);
      rose_operands->append_operand(new SgAsmByteValueExpression(bo));
      rose_operands->append_operand(new SgAsmPowerpcRegisterReferenceExpression(powerpc_regclass_cr, bi,
										powerpc_condreggranularity_bit));
    }
    if(branch_target) {
      rose_operands->append_operand(new SgAsmDoubleWordValueExpression(branch_target));
    } else if(power_op_bcctr == iapi_opcode) {
      rose_operands->append_operand(new SgAsmPowerpcRegisterReferenceExpression(powerpc_regclass_spr, powerpc_spr_ctr));
    } else {
      assert(power_op_bclr == iapi_opcode);
      rose_operands->append_operand(new SgAsmPowerpcRegisterReferenceExpression(powerpc_regclass_spr, powerpc_spr_lr));
    }
    return true;
  }
    break;
  case power_op_sc:
  case power_op_svcs: {
    //cerr << "special-casing syscall insn" << endl;
    unsigned int raw = 0;
    std::vector<unsigned char> bytes = rose_insn->get_raw_bytes();
    for(unsigned i = 0; i < bytes.size(); i++) {
      raw = raw << 8;
      raw |= bytes[i];
    }
    unsigned int lev = (raw >> 5) & 0x7F;
    rose_operands->append_operand(new SgAsmByteValueExpression(lev));
    //cerr << "LEV = " << lev << endl;
    return true;
  }
  default:
    return false;
  }
  
}  

void RoseInsnPPCFactory::massageOperands(const InstructionAPI::Instruction::Ptr &insn, 
					 std::vector<InstructionAPI::Operand> &operands) {
  /*
  if(insn->writesMemory())
    std::swap(operands[0], operands[1]);
  */
  entryID opcode = insn->getOperation().getID();
  // Anything that's writing RA, ROSE expects in RA, RS, RB/immediates form.
  // Any store, however, ROSE expects in RS, RA, RB/displacement form.  Very confusing,
  // but we handle it cleanly here.
  if(!operands[0].isWritten() && operands.size() >= 2 &&
     operands[1].isWritten() && !operands[1].writesMemory()) {
    //std::cerr << "swapping RS and RA in " << insn->format() << std::endl;
    std::swap(operands[0], operands[1]);
  }
  if(opcode == power_op_cmp ||
     opcode == power_op_cmpl ||
     opcode == power_op_cmpi ||
     opcode == power_op_cmpli) {
    operands.push_back(Operand(Immediate::makeImmediate(Result(u8, 1)), false, false));
    std::swap(operands[2], operands[3]);
    std::swap(operands[1], operands[2]);
  }
  if(insn->getOperation().format().find(".") != std::string::npos &&
     insn->getOperation().getID() != power_op_stwcx_rc) {
    operands.pop_back();
  }

  // Convert to ROSE so we can use numeric greater than/less than

  if(kind >= powerpc_lbz && kind <= powerpc_lwzx) {
    operands.resize(2);
  }
  if(kind >= powerpc_stb && kind <= powerpc_stwx) {
    operands.resize(2);
  }

  return;
}

