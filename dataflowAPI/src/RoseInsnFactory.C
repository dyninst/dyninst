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

#include "Instruction.h"
#include "Dereference.h"
#include "Immediate.h"
#include "instructionAPI/src/InstructionDecoderImpl.h"
#include "common/src/singleton_object_pool.h"

#include "../rose/SgAsmInstruction.h"
#include "../rose/SgAsmPowerpcInstruction.h"
#include "../rose/SgAsmAmdgpuVegaInstruction.h"

#include "../rose/SgAsmArmv8Instruction.h"
#include "../rose/SgAsmx86Instruction.h"
#include "../rose/SgAsmExpression.h"

#include "ExpressionConversionVisitor.h"

// Assume Windows/MSVC is little-endian

#if defined(_MSC_VER)
#define htobe _byteswap_ulong
#endif

using namespace Dyninst;
using namespace InstructionAPI;
using namespace DataflowAPI;

SgAsmInstruction *RoseInsnFactory::convert(const Instruction &insn, uint64_t addr) {
  SgAsmInstruction *rinsn = createInsn();
  _addr = addr;  
  rinsn->set_address(addr);
  rinsn->set_mnemonic(insn.format());
  setOpcode(rinsn, insn.getOperation().getID(), insn.getOperation().getPrefixID(), insn.getOperation().format());

  // semantics don't support 64-bit code
  setSizes(rinsn);

  //rinsn->set_operandSize(x86_insnsize_32);
  //rinsn->set_addressSize(x86_insnsize_32);
  
  std::vector<unsigned char> rawBytes;
  for (unsigned i = 0; i < insn.size(); ++i) rawBytes.push_back(insn.rawByte(i));
  rinsn->set_raw_bytes(rawBytes);
  
  // operand list
  SgAsmOperandList *roperands = new SgAsmOperandList;
  
   //std::cerr << "Converting " << insn.format(addr) << " @" << std::hex << addr << std::dec << std::endl;
  
   //std::cerr << "checking instruction: " << insn.format(addr) << " for special handling" << std::endl;
  if (handleSpecialCases(insn.getOperation().getID(), rinsn, roperands)) {
      rinsn->set_operandList(roperands);
      return rinsn;
  }

   //std::cerr << "no special handling by opcode, checking if we should mangle operands..." << std::endl;
  std::vector<InstructionAPI::Operand> operands;
  insn.getOperands(operands);
  massageOperands(insn, operands);
  int i = 0;
  //std::cerr << "converting insn " << insn.format(addr) << std::endl;
  //std::cerr << "\t " << operands.size() << " operands" << std::endl;
  for (std::vector<InstructionAPI::Operand>::iterator opi = operands.begin();
       opi != operands.end();
       ++opi, ++i) {
      InstructionAPI::Operand &currOperand = *opi;
    //std::cerr << "Converting operand " << currOperand.format(arch(), addr) << std::endl;
      SgAsmExpression *converted = convertOperand(currOperand.getValue(), addr, insn.size());
      if (converted == NULL) return NULL;
      roperands->append_operand(converted);
  }  
  rinsn->set_operandList(roperands);
  return rinsn;
}

SgAsmExpression *RoseInsnFactory::convertOperand(const Expression::Ptr expression, int64_t addr, size_t insnSize) {
  if(!expression) return NULL;
  ExpressionConversionVisitor visitor(arch(), addr, insnSize);
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
  SgAsmx86Instruction *tmp = static_cast<SgAsmx86Instruction *>(insn);
  if (a == Arch_x86_64) {
      tmp->set_operandSize(x86_insnsize_64);
      tmp->set_addressSize(x86_insnsize_64);
  } else {
      tmp->set_operandSize(x86_insnsize_32);
      tmp->set_addressSize(x86_insnsize_32);
  }
}

bool RoseInsnX86Factory::handleSpecialCases(entryID, SgAsmInstruction *, SgAsmOperandList *) {
  // Does nothing?

  return false;
}

void RoseInsnX86Factory::massageOperands(const Instruction &insn,
                                         std::vector<InstructionAPI::Operand> &operands) {
  switch (insn.getOperation().getID()) {
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
  case e_aaa:
  case e_aas: 
	  // ROSE does not expect implicit operand rax/eax to be treated as an operand
	  operands.clear();
	  break;
  case e_aad:
  case e_aam: {
	  // ROSE does not expect implicit operand rax/eax to be treated as an operand
	  std::set<RegisterAST::Ptr> regs;
	  operands[0].getReadSet(regs);
	  operands[0].getWriteSet(regs);	 	  
	  if (!regs.empty()) {	      
		      operands[0] = operands[1];
	  }
	  operands.resize(1);
	  break;
  }
  case e_div:
  case e_idiv:
  case e_imul:
  case e_mul:
    // remove implicit operands.
    if (operands.size() == 3) {
      operands[0] = operands[2];
      operands.resize(1);
    }
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
#ifdef os_windows
    // Visual Studio doensn't define htobe32, so we assume that Windows is always little endian.
    raw = _byteswap_ulong(raw);
#else
    raw = htobe32(raw);
#endif
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
      // bi field specifies which condition register bit to test.
      // bi has a 5-bit value. The top 3 bit specifies which condition register to use
      // The bottom 2 bit specifies which bit within the given condition register
      bi = ((raw >> 16) & 0x0000001F);
      rose_operands->append_operand(new SgAsmIntegerValueExpression(bo, new SgAsmIntegerType(ByteOrder::ORDER_LSB, 8, false)));
      
      SgAsmDirectRegisterExpression *dre = new SgAsmDirectRegisterExpression(RegisterDescriptor(powerpc_regclass_cr, 0, bi , 1));
      dre->set_type(new SgAsmIntegerType(ByteOrder::ORDER_LSB, 1, false));
      rose_operands->append_operand(dre);
    }

    // It looks like the ROSE semantics code will infer the target from 
    // the bo field. So, what is passed in as the third operands does not matter
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
    rose_operands->append_operand(new SgAsmIntegerValueExpression(lev, new SgAsmIntegerType(ByteOrder::ORDER_LSB, 8, false)));
    //cerr << "LEV = " << lev << endl;
    return true;
  }
  default:
    return false;
  }
  
}  

void RoseInsnPPCFactory::massageOperands(const Instruction &insn,
                                         std::vector<InstructionAPI::Operand> &operands) {
  /*
  if(insn.writesMemory())
    std::swap(operands[0], operands[1]);
  */
  entryID opcode = insn.getOperation().getID();
  // Anything that's writing RA, ROSE sometimes expects in RA, RS, RB/immediates form.
  // Any store, however, Dyninst expects in RS, RA, RB/displacement form.  Very confusing,
  // but we handle it cleanly here.
  if( opcode != power_op_rldicr &&
      opcode != power_op_rldic && 
      !operands[0].isWritten() && operands.size() >= 2 &&
     operands[1].isWritten() && !operands[1].writesMemory()) {
    //std::cerr << "swapping RS and RA in " << insn.format() << std::endl;
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
  if(insn.getOperation().format().find(".") != std::string::npos &&
     insn.getOperation().getID() != power_op_stwcx_rc) {
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

void RoseInsnArmv8Factory::setSizes(SgAsmInstruction */*insn*/) {

}

SgAsmInstruction *RoseInsnArmv8Factory::createInsn() {
  return new SgAsmArmv8Instruction;
}

void RoseInsnArmv8Factory::setOpcode(SgAsmInstruction *insn, entryID opcode, prefixEntryID, std::string) {
  SgAsmArmv8Instruction *tmp = static_cast<SgAsmArmv8Instruction *>(insn);
  tmp->set_kind(convertKind(opcode));
}

bool RoseInsnArmv8Factory::handleSpecialCases(entryID, SgAsmInstruction *, SgAsmOperandList *) {
  return false;
}

void RoseInsnArmv8Factory::massageOperands(const Instruction &insn,
        std::vector<InstructionAPI::Operand> &operands) {

}
 

void RoseInsnAmdgpuVegaFactory::setSizes(SgAsmInstruction */*insn*/) {

}

SgAsmInstruction *RoseInsnAmdgpuVegaFactory::createInsn() {
  return new SgAsmAmdgpuVegaInstruction;
}

void RoseInsnAmdgpuVegaFactory::setOpcode(SgAsmInstruction *insn, entryID opcode, prefixEntryID, std::string) {
  SgAsmAmdgpuVegaInstruction *tmp = static_cast<SgAsmAmdgpuVegaInstruction *>(insn);
  tmp->set_kind(convertKind(opcode));
}
bool RoseInsnAmdgpuVegaFactory::handleSpecialCases(entryID, SgAsmInstruction *, SgAsmOperandList *) {
  return false;
}

// This helper function expand a single sgpr pair operand into two constructing components
static std::pair<InstructionAPI::Operand,InstructionAPI::Operand> expandSgprPair(InstructionAPI::Operand orig){
    RegisterAST::Ptr sgpr_pair = boost::dynamic_pointer_cast<RegisterAST>(orig.getValue());
    unsigned int offset = sgpr_pair->getID() - amdgpu_vega::sgpr_vec2_0 ;

    MachRegister m_low = MachRegister(amdgpu_vega::sgpr0+offset) ;
    MachRegister m_high = MachRegister(amdgpu_vega::sgpr0+offset+1) ;
    Expression::Ptr  e_oper0 = make_shared(singleton_object_pool<RegisterAST>::construct(m_low,0,m_low.size()*8));
    Expression::Ptr  e_oper1 = make_shared(singleton_object_pool<RegisterAST>::construct(m_high,0,m_high.size()*8));

    auto oper0 = Operand(e_oper0,orig.isRead(),orig.isWritten());
    auto oper1 = Operand(e_oper1,orig.isRead(),orig.isWritten());

    return std::make_pair(oper0,oper1);
}

// TODO: Turn sgpr_pair operands into individual registers
void RoseInsnAmdgpuVegaFactory::massageOperands(const Instruction &insn,
        std::vector<InstructionAPI::Operand> &operands) {
    switch (insn.getOperation().getID()) {
        case amdgpu_op_s_swappc_b64:
            {
                // swap pc has two operands
                // first one is the source of new pc
                // second one is dst for storing the pc, so we turn it into 4 registers
                // TODO : Make this a function call 
                assert(operands.size() == 2);
                auto [oper0,oper1] = expandSgprPair(operands[0]);
                auto [oper2,oper3] = expandSgprPair(operands[1]);
                operands.resize(4);
                operands[0] = oper0;
                operands[1] = oper1;
                operands[2] = oper2;
                operands[3] = oper3;
                break;


            }
 
        case amdgpu_op_s_setpc_b64:
            {
                assert(operands.size() == 1);
                auto [oper0,oper1] = expandSgprPair(operands[0]);
                operands.resize(2);
                operands[0] = oper0;
                operands[1] = oper1;

                break;

            }
        case amdgpu_op_s_getpc_b64:
            {
                assert(operands.size() == 1);
                auto [oper0,oper1] = expandSgprPair(operands[0]);
                operands.resize(3);
                operands[0] = oper0;
                operands[1] = oper1;
                operands[2] = Operand(InstructionAPI::Immediate::makeImmediate(Result(u64,_addr+4)),false,false); 

                break;
            }
        default:
                    break;
    }
}

