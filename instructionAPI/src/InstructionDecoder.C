/*
 * Copyright (c) 2007-2008 Barton P. Miller
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

#include "../h/InstructionDecoder.h"
#include "../h/Expression.h"
#include "../src/arch-x86.h"
#include "../h/Register.h"
#include "../h/Dereference.h"
#include "../h/Immediate.h"

using namespace std;
namespace Dyninst
{
  namespace InstructionAPI
  {
    
    INSTRUCTION_EXPORT InstructionDecoder::~InstructionDecoder()
    {
      delete decodedInstruction;
      delete cond;
      delete locs;
      delete[] mac;
      
    }
    static const unsigned char modrm_use_sib = 4;
    
    INSTRUCTION_EXPORT Instruction InstructionDecoder::decode()
    {
      if(rawInstruction < bufferBegin || rawInstruction >= bufferBegin + bufferSize) return Instruction();
      vector<Expression::Ptr> operands;
      unsigned int decodedSize = decodeOpcode();
      decodeOperands(operands);
      rawInstruction += decodedSize;
      return Instruction(m_Operation, operands, decodedSize, rawInstruction - decodedSize);      
    }
    
    INSTRUCTION_EXPORT Instruction InstructionDecoder::decode(const unsigned char* buffer, size_t /*len*/)
    {
      vector<Expression::Ptr> operands;
      rawInstruction = buffer;
      unsigned int decodedSize = decodeOpcode();
      decodeOperands(operands);
      return Instruction(m_Operation, operands, decodedSize, rawInstruction);
    }

    Expression::Ptr InstructionDecoder::makeSIBExpression(unsigned int opType)
    {
      unsigned scale;
      Register index;
      Register base;
      decode_SIB(locs->sib_byte, scale, index, base);
      // 0x04 is both a "use SIB" and a "don't scale, just use the base" code
      // rename later
      if(index == modrm_use_sib)
      {
	return Expression::Ptr(new RegisterAST(makeRegisterID(base, opType)));
      }
      else
      {
	Expression::Ptr scaleAST(new Immediate(Result(s32, dword_t(scale))));
	Expression::Ptr indexAST(new RegisterAST(makeRegisterID(index, opType)));
	Expression::Ptr baseAST(new RegisterAST(makeRegisterID(base, opType)));
	return makeAddExpression(makeMultiplyExpression(scaleAST, indexAST, u32), baseAST, u32);
      }
    }
     
    Expression::Ptr InstructionDecoder::makeModRMExpression(unsigned int opType)
    {
	if(ia32_is_mode_64())
    {
		if((locs->modrm_mod == 0x0) && (locs->modrm_rm == 0x5))
		{
			return Expression::Ptr(new RegisterAST(r_RIP));
		}
	}

      
      // This handles the rm and reg fields; the mod field affects how this expression is wrapped
      if(locs->modrm_rm != modrm_use_sib || locs->modrm_mod == 0x03)
      {
		return Expression::Ptr(new RegisterAST(makeRegisterID(locs->modrm_rm, opType)));
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
	return Expression::Ptr(new Immediate(Result(s8,*(const byte_t*)(rawInstruction + position))));
	break;
      case op_d:
	return Expression::Ptr(new Immediate(Result(s32, *(const dword_t*)(rawInstruction + position))));
      case op_w:
	return Expression::Ptr(new Immediate(Result(s16, *(const word_t*)(rawInstruction + position))));
	break;
      case op_q:
	return Expression::Ptr(new Immediate(Result(s64, *(const int64_t*)(rawInstruction + position))));
	break;
      case op_v:
      case op_z:
	// 32 bit mode & no prefix, or 16 bit mode & prefix => 32 bit
	// 16 bit mode, no prefix or 32 bit mode, prefix => 16 bit
	if(is32BitMode ^ sizePrefixPresent)
	{
	  return Expression::Ptr(new Immediate(Result(s32, *(const dword_t*)(rawInstruction + position))));
	}
	else
	{
	  return Expression::Ptr(new Immediate(Result(s16, *(const word_t*)(rawInstruction + position))));
	}
	
	break;
      case op_p:
	// 32 bit mode & no prefix, or 16 bit mode & prefix => 48 bit
	// 16 bit mode, no prefix or 32 bit mode, prefix => 32 bit
	if(is32BitMode ^ sizePrefixPresent)
	{
	  return Expression::Ptr(new Immediate(Result(s48, *(const int64_t*)(rawInstruction + position))));
	}
	else
	{
	  return Expression::Ptr(new Immediate(Result(s32, *(const dword_t*)(rawInstruction + position))));
	}
	
	break;
      case op_a:
      case op_dq:
      case op_pd:
      case op_ps:
      case op_s:
      case op_si:
      case op_lea:
      case op_allgprs:
      case op_512:
      case op_c:
	assert(!"Can't happen: opType unexpected for valid ways to decode an immediate");
	return Expression::Ptr();
      default:
	assert(!"Can't happen: opType out of range");
	return Expression::Ptr();
      }
    }
    
    Expression::Ptr InstructionDecoder::getModRMDisplacement()
    {
      int disp_pos;
      
      if(locs->sib_position != -1)
      {
	disp_pos = locs->sib_position + 1;
      }
      else
      {
	disp_pos = locs->modrm_position + 1;
      }
      switch(locs->modrm_mod)
      {
      case 1:
	return Expression::Ptr(new Immediate(Result(s8, (*(const byte_t*)(rawInstruction + disp_pos)))));
	break;
      case 2:
	return Expression::Ptr(new Immediate(Result(s32, *((const dword_t*)(rawInstruction + disp_pos)))));
	break;
      default:
	return Expression::Ptr(new Immediate(Result(s8, 0)));
	break;
      }
    }

    static IA32Regs IntelRegTable[][8] = {
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
      },
      {
	r_RAX, r_RCX, r_RDX, r_RBX, r_RSP, r_RBP, r_RSI, r_RDI
      },
      {
	r_XMM0, r_XMM1, r_XMM2, r_XMM3, r_XMM4, r_XMM5, r_XMM6, r_XMM7
      },
      {
	r_MM0, r_MM1, r_MM2, r_MM3, r_MM4, r_MM5, r_MM6, r_MM7
      },
      {
	r_CR0, r_CR1, r_CR2, r_CR3, r_CR4, r_CR5, r_CR6, r_CR7
      },      
      {
	r_DR0, r_DR1, r_DR2, r_DR3, r_DR4, r_DR5, r_DR6, r_DR7
      },      
      {
	r_TR0, r_TR1, r_TR2, r_TR3, r_TR4, r_TR5, r_TR6, r_TR7
      }      
    };
      
    int InstructionDecoder::makeRegisterID(unsigned int intelReg, unsigned int opType)
    {
      if(locs->rex_w)
      {
	// AMD64 with 64-bit operands
	return IntelRegTable[4][intelReg];
      }
      
      switch(opType)
      {
      case op_b:
	return IntelRegTable[0][intelReg];
	  case op_q:
	return IntelRegTable[4][intelReg];
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
      case op_si:
	return s32;
      case op_w:
      case op_a:
	return s16;
      case op_q:
      case op_sd:
	return s64;
      case op_v:
      case op_lea:
      case op_z:
	if(is32BitMode ^ sizePrefixPresent)
	{
	  return s32;
	}
	else
	{
	  return s16;
	}
	break;
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
      case op_dq:
	return s64;
      case op_512:
	return m512;
      case op_pi:
      case op_ps:
      case op_pd:
	return dbl128;
      case op_s:
	return u48;
      default:
	assert(!"Can't happen!");
	return u8;
      }
    }
    

    void InstructionDecoder::decodeOneOperand(const ia32_operand& operand,
					      std::vector<Expression::Ptr>& outputOperands)
    {
		unsigned int regType = op_d;
		if(ia32_is_mode_64())
		{
			regType = op_q;
		}
      switch(operand.admet)
      {
      case 0:
	// No operand
	assert(!"Mismatched number of operands--check tables");
	return;
      case am_A:
	{
	  // am_A only shows up as a far call/jump.  Position 1 should be universally safe.
	  Expression::Ptr addr(decodeImmediate(operand.optype, 1));
	  Expression::Ptr op(new Dereference(addr, makeSizeType(operand.optype)));
	  outputOperands.push_back(op);
	}
	break;
      case am_C:
	{
	  Expression::Ptr op(new RegisterAST(IntelRegTable[7][locs->modrm_reg]));
	  outputOperands.push_back(op);
	}
	break;
      case am_D:
	{
	  Expression::Ptr op(new RegisterAST(IntelRegTable[8][locs->modrm_reg]));
	  outputOperands.push_back(op);
	}
	break;
      case am_E:
	// am_M is like am_E, except that mod of 0x03 should never occur (am_M specified memory,
	// mod of 0x03 specifies direct register access).
      case am_M:
	// am_R is the inverse of am_M; it should only have a mod of 3
      case am_R:
	switch(locs->modrm_mod)
	{
	  // direct dereference
	case 0x00:
	  assert(operand.admet != am_R);
	  {
	    Expression::Ptr op(new Dereference(makeModRMExpression(regType), makeSizeType(operand.optype)));
	    outputOperands.push_back(op);
	  }
	  break;
	  // dereference with 8-bit offset following mod/rm byte
	case 0x01:
	case 0x02:
	  assert(operand.admet != am_R);
	  {
	    Expression::Ptr RMPlusDisplacement(makeAddExpression(makeModRMExpression(regType), 
								 getModRMDisplacement(), 
								 makeSizeType(regType)));
	    Expression::Ptr op(new Dereference(RMPlusDisplacement, makeSizeType(operand.optype)));
	    outputOperands.push_back(op);
	    break;
	  }
	case 0x03:
	  assert(operand.admet != am_M);
	  // use of actual register
	  {
	    outputOperands.push_back(makeModRMExpression(operand.optype));
	    break;
	  }
	default:
	  assert(!"2-bit value modrm_mod out of range");
	  break;
	};
	
	break;
      case am_F:
	{
	  Expression::Ptr op(new RegisterAST(r_EFLAGS));
	  outputOperands.push_back(op);
	}
	break;
      case am_G:
	{
	  Expression::Ptr op(new RegisterAST(makeRegisterID(locs->modrm_reg, operand.optype)));
	  outputOperands.push_back(op);
	}
	break;
      case am_I:
	outputOperands.push_back(decodeImmediate(operand.optype, locs->imm_position));
	break;
      case am_J:
	{
	  Expression::Ptr Offset(decodeImmediate(operand.optype, locs->imm_position));
	  Expression::Ptr EIP(new RegisterAST(r_EIP));
	  Expression::Ptr op(makeAddExpression(Offset, EIP, u32));
	  outputOperands.push_back(op);
	}
	break;
      case am_O:
	{
	  // Address/offset width, which is *not* what's encoded by the optype...
	  // The deref's width is what's actually encoded here.  Need to address this issue somehow.
	  int pseudoOpType;
	  switch(locs->address_size)
	  {
	  case 1:
	    pseudoOpType = op_b;
	    break;
	  case 2:
	    pseudoOpType = op_w;
	    break;
	  case 4:
	    pseudoOpType = op_d;
	    break;
	  case 0:
	    // closest I can get to "will be address size by default"
	    pseudoOpType = op_v;
	    break;
	  default:
	    assert(!"Bad address size, should be 0, 1, 2, or 4!");
	    pseudoOpType = op_b;
	    break;
	  }
	  

	  int offset_position = locs->opcode_position;
	  if(locs->modrm_position > offset_position && locs->modrm_operand < (int)(outputOperands.size()))
	  {
	    offset_position = locs->modrm_position;
	  }
	  if(locs->sib_position > offset_position)
	  {
	    offset_position = locs->sib_position;
	  }
	  offset_position++;
	  outputOperands.push_back(Expression::Ptr(new Dereference(decodeImmediate(pseudoOpType, 
										   offset_position), 
								   makeSizeType(operand.optype))));
	}
	break;
      case am_P:
	outputOperands.push_back(Expression::Ptr(new RegisterAST(IntelRegTable[6][locs->modrm_reg])));
	break;
      case am_Q:
	switch(locs->modrm_mod)
	{
	  // direct dereference
	case 0x00:
	  {
	    outputOperands.push_back(Expression::Ptr(new Dereference(makeModRMExpression(regType), 
								     makeSizeType(operand.optype)))); 
	  }
	  break;
	  // dereference with 8-bit offset following mod/rm byte
	case 0x01:
	case 0x02:
	  {
	    Expression::Ptr RMPlusDisplacement(makeAddExpression(makeModRMExpression(regType), 
								 getModRMDisplacement(), 
								 makeSizeType(regType)));
	    outputOperands.push_back(Expression::Ptr(new Dereference(RMPlusDisplacement, 
								     makeSizeType(operand.optype))));
	    break;
	  }
	case 0x03:
	  // use of actual register
	  {
	    outputOperands.push_back(Expression::Ptr(new RegisterAST(IntelRegTable[6][locs->modrm_rm])));
	    break;
	  }
	default:
	  assert(!"2-bit value modrm_mod out of range");
	  break;
	};
	
	break;
      case am_S:
	// Segment register in modrm reg field.
	outputOperands.push_back(Expression::Ptr(new RegisterAST(IntelRegTable[3][locs->modrm_reg])));
	break;
      case am_T:
	// test register in modrm reg; should only be tr6/tr7, but we'll decode any of them
	// NOTE: this only appears in deprecated opcodes
	outputOperands.push_back(Expression::Ptr(new RegisterAST(IntelRegTable[9][locs->modrm_reg])));
	break;
      case am_V:
	outputOperands.push_back(Expression::Ptr(new RegisterAST(IntelRegTable[5][locs->modrm_reg])));
	break;
      case am_W:
	switch(locs->modrm_mod)
	{
	  // direct dereference
	case 0x00:
	  {
	    outputOperands.push_back(Expression::Ptr(new Dereference(makeModRMExpression(regType), 
								     makeSizeType(operand.optype)))); 
	  }
	  break;
	  // dereference with 8-bit offset following mod/rm byte
	case 0x01:
	case 0x02:
	  {
	    Expression::Ptr RMPlusDisplacement(makeAddExpression(makeModRMExpression(regType), 
								 getModRMDisplacement(), 
								 makeSizeType(regType)));
	    outputOperands.push_back(Expression::Ptr(new Dereference(RMPlusDisplacement, 
								     makeSizeType(operand.optype))));
	    break;
	  }
	case 0x03:
	  // use of actual register
	  {
	    outputOperands.push_back(Expression::Ptr(new RegisterAST(IntelRegTable[5][locs->modrm_rm])));
	    break;
	  }
	default:
	  assert(!"2-bit value modrm_mod out of range");
	  break;
	};
	
	break;
      case am_X:
	{
	  Expression::Ptr ds(new RegisterAST(r_DS));
	  Expression::Ptr si(new RegisterAST(r_SI));
	  Expression::Ptr segmentOffset(new Immediate(Result(u32, 0x10)));
	  
	  Expression::Ptr ds_segment = makeMultiplyExpression(ds, segmentOffset, u32);
	  Expression::Ptr ds_si = makeAddExpression(ds_segment, si, u32);
	  outputOperands.push_back(Expression::Ptr(new Dereference(ds_si, makeSizeType(operand.optype))));
	}
	break;
      case am_Y:
	{
	  Expression::Ptr es(new RegisterAST(r_ES));
	  Expression::Ptr di(new RegisterAST(r_DI));
	  Expression::Ptr es_segment = 
	  makeMultiplyExpression(es, Expression::Ptr(new Immediate(Result(u32, 0x10))), u32);
	  Expression::Ptr es_di = makeAddExpression(es_segment, di, u32);
	  outputOperands.push_back(Expression::Ptr(new Dereference(es_di, makeSizeType(operand.optype))));
	}
	break;
      case am_reg:
	{
	  int registerID = operand.optype;
	  
#if defined(arch_x86_64)
	if(locs->rex_b)
	{
	  // We need to flip this guy...
	  switch(registerID)
	  {
	  case r_AL:
	  case r_rAX:
	  case r_eAX:
	  case r_EAX:
	  case r_AX:
	    registerID = r_R8;
	    break;
	  case r_CL:
	  case r_rCX:
	  case r_eCX:
	  case r_ECX:
	    registerID = r_R9;
	    break;
	  case r_DL:
	  case r_rDX:
	  case r_eDX:
	  case r_EDX:
	  case r_DX:
	    registerID = r_R10;
	    break;
	  case r_BL:
	  case r_rBX:
	  case r_eBX:
	  case r_EBX:
	    registerID = r_R11;	    
	    break;
	  case r_AH:
	  case r_rSP:
	  case r_eSP:
	  case r_ESP:
	    registerID = r_R12;
	    break;
	  case r_CH:
	  case r_rBP:
	  case r_eBP:
	  case r_EBP:
	    registerID = r_R13;	  
	    break;
	  case r_DH:
	  case r_rSI:
	  case r_eSI:
	  case r_ESI:
	  case r_SI:
	    registerID = r_R14;
	    break;
	  case r_BH:
	  case r_rDI:
	  case r_eDI:
	  case r_EDI:
	  case r_DI:
	    registerID = r_R15;
	    break;
	  default:
	    break;
	  };
	  
	}
	
#endif
	Expression::Ptr op(new RegisterAST(registerID));
	outputOperands.push_back(op);
	}
	
	break;
      case am_stackH:
      case am_stackP:
	// handled elsewhere
	break;
      case am_allgprs:
	{
	  Expression::Ptr op(new RegisterAST(r_ALLGPRS));
	  outputOperands.push_back(op);
	}
	break;
      default:
	printf("decodeOneOperand() called with unknown addressing method %d\n", operand.admet);
	break;
      };
      
    }

    unsigned int InstructionDecoder::decodeOpcode()
    {
      delete decodedInstruction;
      delete cond;
      delete locs;
      delete[] mac;
      
      cond = new ia32_condition;
      locs = new ia32_locations;
      mac = new ia32_memacc[3];
      decodedInstruction = new ia32_instruction(mac, cond, locs);
      ia32_decode(IA32_DECODE_MEMACCESS | IA32_DECODE_CONDITION, 
					   rawInstruction, *decodedInstruction);
      m_Operation = Operation(decodedInstruction->getEntry(), decodedInstruction->getPrefix(), locs);
      sizePrefixPresent = (decodedInstruction->getPrefix()->getOperSzPrefix() == 0x66);
      return decodedInstruction->getSize();
    }
    
    void InstructionDecoder::decodeOperands(vector<Expression::Ptr>& operands)
    {
      for(unsigned i = 0; i < m_Operation.numOperands(); i++)
      {
	decodeOneOperand(decodedInstruction->getEntry()->operands[i], operands);
      }
    }
    
  };
};
