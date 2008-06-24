
#include "../h/InstructionDecoder.h"
#include "../h/Expression.h"
#include "arch-x86.h"
#include "../h/Register.h"
#include "../h/Dereference.h"
#include "../h/Immediate.h"

using namespace std;
namespace Dyninst
{
  namespace Instruction
  {
    
    static const unsigned char modrm_use_sib = 4;
    
  
    Instruction InstructionDecoder::decode(const unsigned char* buffer, size_t /*len*/)
    {
      vector<Expression::Ptr> operands;
      rawInstruction = buffer;
      unsigned int decodedSize = decodeOpcode();
      decodeOperands(operands);
      return Instruction(m_Operation, operands, decodedSize);
    }

    Expression::Ptr InstructionDecoder::makeSIBExpression(unsigned int opType)
    {
      unsigned scale;
      Register index;
      Register base;
      decode_SIB(locs.sib_byte, scale, index, base);
      // 0x04 is both a "use SIB" and a "don't scale, just use the base" code
      // rename later
      if(index == modrm_use_sib)
      {
	return Expression::Ptr(new RegisterAST(makeRegisterID(base, opType)));
      }
      else
      {
	Expression* scaleAST(new Immediate(Result(s32, dword_t(scale))));
	Expression* indexAST(new RegisterAST(makeRegisterID(index, opType)));
	Expression* baseAST(new RegisterAST(makeRegisterID(base, opType)));
	return makeAddExpression(makeMultiplyExpression(scaleAST, indexAST, u32), baseAST, u32);
      }
    }

    Expression::Ptr InstructionDecoder::makeModRMExpression(unsigned int opType)
    {
      // This handles the rm and reg fields; the mod field affects how this expression is wrapped
      if(locs.modrm_rm != modrm_use_sib)
      {
	return Expression::Ptr(new RegisterAST(makeRegisterID(locs.modrm_rm, opType)));
      }
      else
      {
	return makeSIBExpression(opType);
      }      
    }

    Expression::Ptr InstructionDecoder::decodeImmediate(unsigned int opType, unsigned int position)
    {
      switch(opType)
      {
      case op_b:
	return Expression::Ptr(new Immediate(Result(s8,*(byte_t*)(rawInstruction + position))));
	break;
      case op_d:
	return Expression::Ptr(new Immediate(Result(s32, *(dword_t*)(rawInstruction + position))));
      case op_si:
      case op_w:
	return Expression::Ptr(new Immediate(Result(s16, *(word_t*)(rawInstruction + position))));
	break;
      case op_q:
	return Expression::Ptr(new Immediate(Result(s64, *(int64_t*)(rawInstruction + position))));
	break;
      case op_v:
      case op_z:
	// 32 bit mode & no prefix, or 16 bit mode & prefix => 32 bit
	// 16 bit mode, no prefix or 32 bit mode, prefix => 16 bit
	if(is32BitMode ^ sizePrefixPresent)
	{
	  return Expression::Ptr(new Immediate(Result(s32, *(dword_t*)(rawInstruction + position))));
	}
	else
	{
	  return Expression::Ptr(new Immediate(Result(s16, *(word_t*)(rawInstruction + position))));
	}
	
	break;
      default:
	assert(!"FIXME: Unimplemented opType");
      }
    }
    
    Expression::Ptr InstructionDecoder::getModRMDisplacement()
    {
      int disp_pos;
      
      if(locs.sib_position != -1)
      {
	disp_pos = locs.sib_position + 1;
      }
      else
      {
	disp_pos = locs.modrm_position + 1;
      }
      switch(locs.modrm_mod)
      {
      case 1:
	return Expression::Ptr(new Immediate(Result(s8, (*(byte_t*)(rawInstruction + disp_pos)))));
	break;
      case 2:
	return Expression::Ptr(new Immediate(Result(s32, *((dword_t*)(rawInstruction + disp_pos)))));
	break;
      default:
	return Expression::Ptr();
	break;
      }
    }

    static IA32Regs IntelRegTable[4][8] = {
      {
	r_AL, r_CL, r_DL, r_BL, r_AH, r_CH, r_DH, r_BH
      },
      {
	r_AX, r_eCX, r_DX, r_eBX, r_eSP, r_eBP, r_eSI, r_eDI
      },
      {
	r_EAX, r_ECX, r_EDX, r_EBX, r_ESP, r_EBP, r_ESI, r_EDI
      },
      {
	r_ES, r_CS, r_SS, r_DS, r_FS, r_GS, r_Reserved, r_Reserved
      }
    };
      
    int InstructionDecoder::makeRegisterID(unsigned int intelReg, unsigned int opType)
    {
      switch(opType)
      {
      case op_b:
	return IntelRegTable[0][intelReg];
      case op_d:
      case op_si:
      case op_w:
      default:
	return IntelRegTable[2][intelReg];
	break;
      }
      
    }
    
    Result_Type InstructionDecoder::makeSizeType(unsigned int opType)
    {
      switch(opType)
      {
      case op_b:
      case op_c:
	return s8;
      case op_d:
      case op_ss:
      case op_allgprs:
	return s32;
      case op_w:
      case op_a:
      case op_si:
      case op_v:
	return s16;
      case op_q:
      case op_sd:
	return s64;
      case op_z:
	if(is32BitMode ^ sizePrefixPresent)
	{
	  return s32;
	}
	else
	{
	  return s16;
	}
      case op_p:
	// book says operand size; arch-x86 says word + word * operand size
	if(is32BitMode ^ sizePrefixPresent)
	{
	  return s48;
	}
	else
	{
	  return s32;
	}
	
	
      case op_s:
      case op_dq:
      case op_pi:
      case op_ps:
      case op_lea:
      case op_512:
	assert(!"Not implemented!");
	return u8;
      default:
	assert(!"Can't happen!");
	return u8;
      }
    }
    

    void InstructionDecoder::decodeOneOperand(const ia32_operand& operand,
					      std::vector<Expression::Ptr>& outputOperands)
    {
      switch(operand.admet)
      {
      case 0:
	// No operand
	outputOperands.push_back(Expression::Ptr());
	return;
      case am_A:
	{
	  Expression::Ptr addr(decodeImmediate(operand.optype, locs.disp_position));
	  outputOperands.push_back(Expression::Ptr(new Dereference(addr, makeSizeType(operand.optype))));
	}
	break;
      case am_C:
	assert(!"Not implemented yet!  Operand is control register.");
	break;
      case am_D:
	assert(!"Not implemented yet!  Operand is debug register.");
	break;
      case am_E:
	// am_M is like am_E, except that mod of 0x03 should never occur (am_M specified memory,
	// mod of 0x03 specifies direct register access).
      case am_M:
	// am_R is the inverse of am_M; it should only have a mod of 3
      case am_R:
	switch(locs.modrm_mod)
	{
	  // direct dereference
	case 0x00:
	  assert(operand.admet != am_R);
	  {
	    outputOperands.push_back(Expression::Ptr(new Dereference(makeModRMExpression(operand.optype), makeSizeType(operand.optype))));	    
	  }
	  break;
	  // dereference with 8-bit offset following mod/rm byte
	case 0x01:
	case 0x02:
	  assert(operand.admet != am_R);
	  {
	    Expression::Ptr RMPlusDisplacement(makeAddExpression(makeModRMExpression(operand.optype), 
								   getModRMDisplacement(), makeSizeType(operand.optype)));
	    outputOperands.push_back(Expression::Ptr(new Dereference(RMPlusDisplacement, makeSizeType(operand.optype))));
	    break;
	  }
	case 0x03:
	  assert(operand.admet != am_M);
	  // use of actual register
	  {
	    outputOperands.push_back(Expression::Ptr(makeModRMExpression(operand.optype)));
	    break;
	  }
	default:
	  assert(!"2-bit value modrm_mod out of range");
	  break;
	};
	
	break;
      case am_F:
	assert(!"Not implemented, flags register");
	break;
      case am_G:
	outputOperands.push_back(Expression::Ptr(new RegisterAST(makeRegisterID
								     (locs.modrm_reg, operand.optype))));
	break;
      case am_I:
	outputOperands.push_back(Expression::Ptr(decodeImmediate(operand.optype, locs.imm_position)));
	break;
      case am_J:
	{
	  Expression::Ptr Offset(decodeImmediate(operand.optype, locs.imm_position));
	  Expression::Ptr EIP(new RegisterAST(r_EIP));
	  outputOperands.push_back(Expression::Ptr(makeAddExpression(Offset, EIP, u32)));
	}
	break;
      case am_O:
	{
	  // Address/offset width, which is *not* what's encoded by the optype...
	  // The deref's width is what's actually encoded here.  Need to address this issue somehow.
	  int pseudoOpType = op_d;
	  outputOperands.push_back(Expression::Ptr(new Dereference(decodeImmediate(pseudoOpType, locs.imm_position), makeSizeType(operand.optype))));
	}
	break;
      case am_P:
	assert(!"Not implemented, mod r/m reg = MMX");
	break;
      case am_Q:
	assert(!"Not implemented, mod r/m = MMX or memory");
	break;
      case am_S:
	assert(!"Not implemented, mod r/m reg = segment");
	break;
      case am_T:
	assert(!"Not implemented, mod r/m reg = test");
	break;
      case am_V:
	assert(!"Not implemented, mod r/m reg = XMM");
	break;
      case am_W:
	assert(!"Not implemented, mod r/m r/m = XMM or memory");
	break;
      case am_X:
	assert(!"Not implemented, operand at *(DS:SI)");
	break;
      case am_Y:
	assert(!"Not implemented, operand memory at *(ES:DI)");
	break;
      case am_reg:
	outputOperands.push_back(Expression::Ptr(new RegisterAST(operand.optype)));
	break;
      case am_stackH:
      case am_stackP:
	// handled elsewhere
	break;
      case am_allgprs:
	assert(!"Not implemented, pseudo-operand for all GPRs");
	break;
      default:
	printf("decodeOneOperand() called with unknown addressing method %d\n", operand.admet);
	break;
      };
      
    }

    unsigned int InstructionDecoder::decodeOpcode()
    {
      delete decodedInstruction;
      decodedInstruction = new ia32_instruction(mac, &cond, &locs);
      ia32_decode(IA32_DECODE_MEMACCESS | IA32_DECODE_CONDITION, rawInstruction, *decodedInstruction);
      m_Operation = Operation(decodedInstruction->getEntry());
      sizePrefixPresent = (decodedInstruction->getPrefix()->getOperSzPrefix() == 0x66);
      return decodedInstruction->getSize();
    }
    
    void InstructionDecoder::decodeOperands(vector<Expression::Ptr>& operands)
    {
      for(int i = 0; i < m_Operation.numOperands(); i++)
      {
	decodeOneOperand(decodedInstruction->getEntry()->operands[i], operands);
      }
    }
    
  };
};
