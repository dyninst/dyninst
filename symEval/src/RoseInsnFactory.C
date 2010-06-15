#include "./roseInsnFactory.h"
#include "../rose/x86InstructionSemantics.h"
#include "../rose/powerpcInstructionSemantics.h"

using namespace Dyninst;
using namespace InstructionAPI;
using namespace SymbolicEvaluation;

SageInstruction_t *RoseInsnFactory::convert(const InstructionAPI::Instruction::Ptr &insn, uint64_t addr) {
  SageInstruction_t *rinsn = createInsn();
  
  rinsn->set_address(addr);
  rinsn->set_mnemonic(insn->format());
  setOpcode(rinsn, insn->getOperation().getID(), insn->getOperation().format());

  // semantics don't support 64-bit code
  setSizes(rinsn);

  //rinsn->set_operandSize(x86_insnsize_32);
  //rinsn->set_addressSize(x86_insnsize_32);
  
  std::vector<unsigned char> rawBytes;
  for (unsigned i = 0; i < insn->size(); ++i) rawBytes.push_back(insn->rawByte(i));
  rinsn->set_raw_bytes(rawBytes);
  
  // operand list
  SgAsmOperandList *roperands = new SgAsmOperandList;
  
  //cerr << "Converting " << insn->format() << " @" << hex << addr << dec << endl;
  
  //cerr << "checking instruction: " << insn->format() << " for special handling" << endl;
  if (handleSpecialCases(insn->getOperation().getID(), rinsn, roperands)) {
    rinsn->set_operandList(roperands);
    return rinsn;
  }

  if(RoseInsnFactoryArchTraits<a>::handleSpecialCases(insn->getOperation().getID(), rinsn, roperands)) {
    rinsn->set_operandList(roperands);
    return rinsn;
  }

  //cerr << "no special handling by opcode, checking if we should mangle operands..." << endl;
  std::vector<InstructionAPI::Operand> operands;
  insn->getOperands(operands);
  //cerr << "\t " << operands.size() << " operands" << endl;
  handleSpecialCases(insn, operands);
  int i = 0;
  //cerr << "converting insn " << insn->format() << endl;
  for (std::vector<InstructionAPI::Operand>::iterator opi = operands.begin();
       opi != operands.end();
       ++opi, ++i) {
    InstructionAPI::Operand &currOperand = *opi;
    //cerr << "Converting operand " << currOperand.format() << endl;
    roperands->append_operand(convertOperand(currOperand));
  }  
  rinsn->set_operandList(roperands);
  return rinsn;
}

SgAsmExpression *RoseInsnFactory::convertOperand(const Expression::Ptr expression) {
  if(!expression) return NULL;
  Visitor_t visitor;
  expression->apply(&visitor);
  return visitor.getRoseExpression();
}

///////////// X86 //////////////////

// Note: convertKind is defined in convertOpcodes.C

void RoseInsnX86Factory::setOpcode(SageInstruction_t *insn, entryID opcode, std::string) {
  SgAsmX86Instruction *tmp = static_cast<SgAsmX86Instruction *>(insn);
  tmp->set_kind(convertKind(opcode));
}

void RoseInsnX86Factory::setSizes(SageInstruction_t *insn) {
  // FIXME when we go 64-bit...
  insn->set_operandSize(x86_insnsize_32);
  insn->set_addressSize(x86_insnsize_32);
}

bool RoseInsnX86Factory::handleSpecialCases(entryID, SageInstruction_t *, SgAsmOperandList *) {
  // Does nothing?
  return false;
}

void RoseInsnX86Factory::massageOperands(InstructionAPI::Instruction::Ptr &insn, 
					 std::vector<InstructionAPI::Operand> &operands) {
  switch (insn->getOperation().getID()) {
  case e_lea: {
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
  case e_movsw_d:
    // No operands
    operands.clear();
    break;
  case e_cmpsb:
  case e_cmpsw:
  case e_cmpsd:
    // No operands
    operands.clear();
    break;
  case e_stosb:
  case e_stosw_d:
    // Also, no operands
    operands.clear();
    break;
  case e_jcxz_jec:
    operands.resize(1);
    break;
  case e_cbw_cwde:
    // Nada
    operands.clear();
    break;
  default:
    break;
  }
}


//////////// PPC ///////////////////
// Note: convertKind is defined in convertOpcodes.C

void RoseInsnPPCFactory::setOpcode(SageInstruction_t *insn, entryID opcode, std::string mnem) {
  SgAsmPowerpcInstruction *tmp = static_cast<SgAsmPowerpcInstruction *>(insn);
  tmp->set_kind(convertKind(opcode, mnem));
}


void RoseInsnPPCFactory::setSizes(SageInstruction_t *insn) {
}


bool RoseInsnPPCFactory::handleSpecialCases(entryID iapi_opcode, SageInstruction_t *insn, SgAsmOperandList *rose_operands) {
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

void RoseInsnX86Factory::massageOperands(InstructionAPI::Instruction::Ptr &insn, 
					 std::vector<InstructionAPI::Operand> &operands) {
  if(insn->writesMemory())
    std::swap(operands[0], operands[1]);
  entryID opcode = insn->getOperation().getID();
  // Anything that's writing RA, ROSE expects in RA, RS, RB/immediates form.
  // Any store, however, ROSE expects in RS, RA, RB/displacement form.  Very confusing,
  // but we handle it cleanly here.
  if(!operands[0].isWritten() && operands.size() >= 2 &&
     operands[1].isWritten() && !operands[1].writesMemory()) {
    std::cerr << "swapping RS and RA in " << insn->format() << std::endl;
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
  PowerpcInstructionKind kind = convertKind(opcode);

  if(kind >= powerpc_lbz && kind <= powerpc_lwzx) {
    operands.resize(2);
  }
  if(kind >= powerpc_stb && kind <= powerpc_stwx) {
    operands.resize(2);
  }

  return;
}

/////////////// Visitor class /////////////////

template <Architecture a>
void ExpressionConversionVisitor<a>::visit(BinaryFunction* binfunc) {
  assert(m_stack.size() >= 2);
  SgAsmExpression *rhs = m_stack.front();
  m_stack.pop_front();
  SgAsmExpression *lhs = m_stack.front();
  m_stack.pop_front();
  // If the RHS didn't convert, that means it should disappear
  // And we are just left with the LHS
  if(!rhs && !lhs) {
    roseExpression = NULL;
  }
  else if (!rhs) {
    roseExpression = lhs;
  }
  else if(!lhs) {
    roseExpression = rhs;
  }
  else {
    // now build either add or multiply
    if (binfunc->isAdd())
      roseExpression = new SgAsmBinaryAdd(lhs, rhs);
    else if (binfunc->isMultiply())
      roseExpression = new SgAsmBinaryMultiply(lhs, rhs);
    else roseExpression = NULL; // error
  }
  m_stack.push_front(roseExpression);
}

