/*
 * Copyright (c) 1996-2009 Barton P. Miller
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

#include "../h/InstructionDecoder.h"
#include "../h/Expression.h"
#include "../src/arch-x86.h"
#include "../h/Register.h"
#include "../h/Dereference.h"
#include "../h/Immediate.h"
#include "../h/BinaryFunction.h"
#include "../../common/h/singleton_object_pool.h"

using namespace std;
namespace Dyninst
{
  namespace InstructionAPI
  {
    
    
    INSTRUCTION_EXPORT InstructionDecoder::InstructionDecoder(const unsigned char* buffer, size_t size) :
            locs(NULL),
      decodedInstruction(NULL), 
      is32BitMode(true),
      sizePrefixPresent(false),
      bufferBegin(buffer),
      bufferSize(size),
      rawInstruction(bufferBegin)
    {
    }
    INSTRUCTION_EXPORT InstructionDecoder::InstructionDecoder() :
            locs(NULL),
      decodedInstruction(NULL), 
      is32BitMode(true),
      sizePrefixPresent(false),
      bufferBegin(NULL),
      bufferSize(0),
      rawInstruction(NULL)
    {
    }
    INSTRUCTION_EXPORT InstructionDecoder::InstructionDecoder(const InstructionDecoder& o) :
            locs(NULL),
            decodedInstruction(NULL),
    is32BitMode(o.is32BitMode),
    sizePrefixPresent(o.sizePrefixPresent),
    bufferBegin(o.bufferBegin),
    bufferSize(o.bufferSize),
    rawInstruction(o.rawInstruction)
    {
    }
    INSTRUCTION_EXPORT InstructionDecoder::~InstructionDecoder()
    {
      if(decodedInstruction) decodedInstruction->~ia32_instruction();
      free(decodedInstruction);
      
      if(locs) locs->~ia32_locations();
      free(locs);
    }
    static const unsigned char modrm_use_sib = 4;
    
    INSTRUCTION_EXPORT void InstructionDecoder::setMode(bool is64)
    {
      ia32_set_mode_64(is64);
    }
    
    INSTRUCTION_EXPORT Instruction::Ptr InstructionDecoder::decode()
    {
      if(rawInstruction < bufferBegin || rawInstruction >= bufferBegin + bufferSize) return Instruction::Ptr();
      unsigned int decodedSize = decodeOpcode();
      
      rawInstruction += decodedSize;
      return make_shared(singleton_object_pool<Instruction>::construct(m_Operation, decodedSize, 
									 rawInstruction - decodedSize));
    }
    
    INSTRUCTION_EXPORT Instruction::Ptr InstructionDecoder::decode(const unsigned char* buffer)
    {
      vector<Expression::Ptr, boost::pool_allocator<Expression::Ptr> > operands;
      rawInstruction = buffer;
      unsigned int decodedSize = decodeOpcode();
      return make_shared(singleton_object_pool<Instruction>::construct(m_Operation, decodedSize, 
									 rawInstruction));
    }
    Expression::Ptr InstructionDecoder::makeAddExpression(Expression::Ptr lhs, Expression::Ptr rhs, Result_Type resultType)
    {
      static BinaryFunction::funcT::Ptr adder(new BinaryFunction::addResult());
      
      return make_shared(singleton_object_pool<BinaryFunction>::construct(lhs, rhs, resultType, adder));
    }
    Expression::Ptr InstructionDecoder::makeMultiplyExpression(Expression::Ptr lhs, Expression::Ptr rhs, Result_Type resultType)
    {
      static BinaryFunction::funcT::Ptr multiplier(new BinaryFunction::multResult());
      return make_shared(singleton_object_pool<BinaryFunction>::construct(lhs, rhs, resultType, multiplier));
    } 
    Expression::Ptr InstructionDecoder::makeSIBExpression(unsigned int opType)
    {
      unsigned scale;
      Register index;
      Register base;
      Result_Type aw = ia32_is_mode_64() ? u32 : u64;
      
      decode_SIB(locs->sib_byte, scale, index, base);
	
      Expression::Ptr scaleAST(make_shared(singleton_object_pool<Immediate>::construct(Result(u8, dword_t(scale)))));
      Expression::Ptr indexAST(make_shared(singleton_object_pool<RegisterAST>::construct(makeRegisterID(index, opType,
                                   locs->rex_x))));
      Expression::Ptr baseAST;
      if(base == 0x05)
      {
	switch(locs->modrm_mod)
	{
	case 0x00:
	  baseAST = decodeImmediate(op_d, locs->sib_position + 1);
	  break;
	case 0x01:
	  baseAST = makeAddExpression(decodeImmediate(op_b, locs->sib_position + 1),
				      make_shared(singleton_object_pool<RegisterAST>::construct(r_RBP)), aw);
	  break;
	case 0x02:
	  baseAST = makeAddExpression(decodeImmediate(op_d, locs->sib_position + 1),
				      make_shared(singleton_object_pool<RegisterAST>::construct(r_RBP)), aw);
	  break;
	case 0x03:
	default:
	  assert(0);
	  break;
	};
      }
      else
      {
	baseAST = make_shared(singleton_object_pool<RegisterAST>::construct(makeRegisterID(base, opType,
											   locs->rex_b)));
      }
      if(index == 0x04 && (!(ia32_is_mode_64()) || !(locs->rex_x)))
      {
	return baseAST;
      }
      return makeAddExpression(makeMultiplyExpression(scaleAST, indexAST, aw), baseAST, aw);
    }
     
    Expression::Ptr InstructionDecoder::makeModRMExpression(unsigned int opType)
    {
        unsigned int regType = op_d;
        if(ia32_is_mode_64())
        {
            regType = op_q;
        }
        Result_Type aw = ia32_is_mode_64() ? u32 : u64;
      Expression::Ptr e = 
      make_shared(singleton_object_pool<RegisterAST>::construct(makeRegisterID(locs->modrm_rm, regType,
									       (locs->rex_b == 1))));
      switch(locs->modrm_mod)
      {
      case 0:
	if(locs->modrm_rm == modrm_use_sib) {
	  e = makeSIBExpression(opType);
	}
	if(locs->modrm_rm == 0x5)
	{
            assert(locs->opcode_position > -1);
	  unsigned char opcode = rawInstruction[locs->opcode_position];
	  // treat FP decodes as legacy mode since it appears that's what we've got in our 
	  // old code...
	  if(ia32_is_mode_64() && (opcode < 0xD8 || opcode > 0xDF))
	  {
	    e = makeAddExpression(make_shared(singleton_object_pool<RegisterAST>::construct(r_RIP)),
				   getModRMDisplacement(), aw);
          }
          else
          {
            e = getModRMDisplacement();
	  }
	
      }
      if(opType == op_lea)
      {
          return e;
      }
	return make_shared(singleton_object_pool<Dereference>::construct(e, makeSizeType(opType)));
      case 1:
      case 2:
	{
	if(locs->modrm_rm == modrm_use_sib) {
	  e = makeSIBExpression(opType);
	}
	Expression::Ptr disp_e = makeAddExpression(e, getModRMDisplacement(), aw);
	return make_shared(singleton_object_pool<Dereference>::construct(disp_e, makeSizeType(opType)));
	}
      case 3:
	return e;
      default:
	return Expression::Ptr();
	
      };
	
    }      

    Expression::Ptr InstructionDecoder::decodeImmediate(unsigned int opType, unsigned int position, bool isSigned)
    {
        const unsigned char* bufferEnd = bufferBegin + (bufferSize ? bufferSize : 16);
        assert(position != (unsigned int)(-1));
      switch(opType)
      {
      case op_b:
          assert(rawInstruction + position < bufferEnd);
          return Immediate::makeImmediate(Result(isSigned ? s8 : u8 ,*(const byte_t*)(rawInstruction + position)));
	break;
      case op_d:
          assert(rawInstruction + position + 3 < bufferEnd);
          return Immediate::makeImmediate(Result(isSigned ? s32 : u32,*(const dword_t*)(rawInstruction + position)));
      case op_w:
          assert(rawInstruction + position + 1 < bufferEnd);
          return Immediate::makeImmediate(Result(isSigned ? s16 : u16,*(const word_t*)(rawInstruction + position)));
	break;
      case op_q:
          assert(rawInstruction + position + 7 < bufferEnd);
          return Immediate::makeImmediate(Result(isSigned ? s64 : u64,*(const int64_t*)(rawInstruction + position)));
	break;
      case op_v:
      case op_z:
	// 32 bit mode & no prefix, or 16 bit mode & prefix => 32 bit
	// 16 bit mode, no prefix or 32 bit mode, prefix => 16 bit
	if(!sizePrefixPresent)
	{
            assert(rawInstruction + position + 3 < bufferEnd);
            return Immediate::makeImmediate(Result(isSigned ? s32 : u32,*(const dword_t*)(rawInstruction + position)));
	}
	else
	{
            assert(rawInstruction + position + 1 < bufferEnd);
            return Immediate::makeImmediate(Result(isSigned ? s16 : u16,*(const word_t*)(rawInstruction + position)));
	}
	
	break;
      case op_p:
	// 32 bit mode & no prefix, or 16 bit mode & prefix => 48 bit
	// 16 bit mode, no prefix or 32 bit mode, prefix => 32 bit
	if(!sizePrefixPresent)
	{
            assert(rawInstruction + position + 5< bufferEnd);
            return Immediate::makeImmediate(Result(isSigned ? s48 : u48,*(const int64_t*)(rawInstruction + position)));
	}
	else
	{
            assert(rawInstruction + position + 3 < bufferEnd);
            return Immediate::makeImmediate(Result(isSigned ? s32 : u32,*(const dword_t*)(rawInstruction + position)));
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
	return make_shared(singleton_object_pool<Immediate>::construct(Result(s8, (*(const byte_t*)(rawInstruction + disp_pos)))));
	break;
      case 2:
	return make_shared(singleton_object_pool<Immediate>::construct(Result(s32, *((const dword_t*)(rawInstruction +
disp_pos)))));
	break;
          case 0:
              if(locs->modrm_rm == 5)
                return make_shared(singleton_object_pool<Immediate>::construct(Result(s32, *((const dword_t*)(rawInstruction +
                        disp_pos)))));
              else
                  return make_shared(singleton_object_pool<Immediate>::construct(Result(s8, 0)));
              break;
          default:
	return make_shared(singleton_object_pool<Immediate>::construct(Result(s8, 0)));
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
      },
      {
	r_R8, r_R9, r_R10, r_R11, r_R12, r_R13, r_R14, r_R15
      },
      {
       r_AL, r_CL, r_DL, r_BL, r_SPL, r_BPL, r_SIL, r_DIL
      }
      
    };
      
    int InstructionDecoder::makeRegisterID(unsigned int intelReg, unsigned int opType,
					   bool isExtendedReg)
    {
      if(isExtendedReg)
      {
	return IntelRegTable[10][intelReg];
      }
      if(locs->rex_w)
      {
	// AMD64 with 64-bit operands
	return IntelRegTable[4][intelReg];
      }
      switch(opType)
      {
      case op_b:
     	if (locs->rex_position == -1) {
		 return IntelRegTable[0][intelReg];
	} else {
		return IntelRegTable[11][intelReg];
	}	
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
	return u8;
      case op_d:
      case op_ss:
      case op_allgprs:
      case op_si:
	return u32;
      case op_w:
      case op_a:
	return u16;
      case op_q:
      case op_sd:
	return u64;
      case op_v:
      case op_lea:
      case op_z:
	if(is32BitMode ^ sizePrefixPresent)
	{
	  return u32;
	}
	else
	{
	  return u16;
	}
	break;
      case op_p:
	// book says operand size; arch-x86 says word + word * operand size
	if(is32BitMode ^ sizePrefixPresent)
	{
	  return u48;
	}
	else
	{
	  return u32;
	}
      case op_dq:
	return u64;
      case op_512:
	return m512;
      case op_pi:
      case op_ps:
      case op_pd:
	return dbl128;
      case op_s:
	return u48;
      case op_f:
	return sp_float;
      case op_dbl:
	return dp_float;
      case op_14:
	return m14;
      default:
	assert(!"Can't happen!");
	return u8;
      }
    }
    

    bool InstructionDecoder::decodeOneOperand(const ia32_operand& operand,
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
	{
	  fprintf(stderr, "ERROR: Instruction with mismatched operands.  Raw bytes: ");
	  for(unsigned int i = 0; i < decodedInstruction->getSize(); i++) {
	    fprintf(stderr, "%x ", rawInstruction[i]);
	  }
	  fprintf(stderr, "\n");
	  assert(!"Mismatched number of operands--check tables");
	  return false;
	}
      case am_A:
	{
	  // am_A only shows up as a far call/jump.  Position 1 should be universally safe.
	  Expression::Ptr addr(decodeImmediate(operand.optype, 1));
	  Expression::Ptr op(make_shared(singleton_object_pool<Dereference>::construct(addr, makeSizeType(operand.optype))));
	  outputOperands.push_back(op);
	}
	break;
      case am_C:
	{
	  Expression::Ptr op(make_shared(singleton_object_pool<RegisterAST>::construct(IntelRegTable[7][locs->modrm_reg])));
	  outputOperands.push_back(op);
	}
	break;
      case am_D:
	{
	  Expression::Ptr op(make_shared(singleton_object_pool<RegisterAST>::construct(IntelRegTable[8][locs->modrm_reg])));
	  outputOperands.push_back(op);
	}
	break;
      case am_E:
	// am_M is like am_E, except that mod of 0x03 should never occur (am_M specified memory,
	// mod of 0x03 specifies direct register access).
      case am_M:
	// am_R is the inverse of am_M; it should only have a mod of 3
      case am_R:
	outputOperands.push_back(makeModRMExpression(operand.optype));
	break;
      case am_F:
	{
	  Expression::Ptr op(make_shared(singleton_object_pool<RegisterAST>::construct(r_EFLAGS)));
	  outputOperands.push_back(op);
	}
	break;
      case am_G:
	{
	  Expression::Ptr op(make_shared(singleton_object_pool<RegisterAST>::construct(makeRegisterID(locs->modrm_reg, operand.optype, locs->rex_r))));
	  outputOperands.push_back(op);
	}
	break;
      case am_I:
	outputOperands.push_back(decodeImmediate(operand.optype, locs->imm_position));
	break;
      case am_J:
	{
	  Expression::Ptr Offset(decodeImmediate(operand.optype, locs->imm_position, true));
	  Expression::Ptr EIP(make_shared(singleton_object_pool<RegisterAST>::construct(r_EIP)));
	  Expression::Ptr InsnSize(make_shared(singleton_object_pool<Immediate>::construct(Result(u8, decodedInstruction->getSize()))));
	  Expression::Ptr postEIP(makeAddExpression(EIP, InsnSize, u32));
	  
	  Expression::Ptr op(makeAddExpression(Offset, postEIP, u32));
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
	  outputOperands.push_back(make_shared(singleton_object_pool<Dereference>::construct(decodeImmediate(pseudoOpType, 
										   offset_position), 
								   makeSizeType(operand.optype))));
	}
	break;
      case am_P:
	outputOperands.push_back(make_shared(singleton_object_pool<RegisterAST>::construct(IntelRegTable[6][locs->modrm_reg])));
	break;
      case am_Q:

	switch(locs->modrm_mod)
	{
	  // direct dereference
	case 0x00:
	case 0x01:
	case 0x02:
	  outputOperands.push_back(makeModRMExpression(operand.optype));
	  break;
	case 0x03:
	  // use of actual register
	  {
	    outputOperands.push_back(make_shared(singleton_object_pool<RegisterAST>::construct(IntelRegTable[6][locs->modrm_rm])));
	    break;
	  }
	default:
	  assert(!"2-bit value modrm_mod out of range");
	  break;
	};
	break;
      case am_S:
	// Segment register in modrm reg field.
	outputOperands.push_back(make_shared(singleton_object_pool<RegisterAST>::construct(IntelRegTable[3][locs->modrm_reg])));
	break;
      case am_T:
	// test register in modrm reg; should only be tr6/tr7, but we'll decode any of them
	// NOTE: this only appears in deprecated opcodes
	outputOperands.push_back(make_shared(singleton_object_pool<RegisterAST>::construct(IntelRegTable[9][locs->modrm_reg])));
	break;
      case am_V:
	outputOperands.push_back(make_shared(singleton_object_pool<RegisterAST>::construct(IntelRegTable[5][locs->modrm_reg])));
	break;
      case am_W:
	switch(locs->modrm_mod)
	{
	  // direct dereference
	case 0x00:
	case 0x01:
	case 0x02:
	  outputOperands.push_back(makeModRMExpression(makeSizeType(operand.optype)));
	  break;
	case 0x03:
	  // use of actual register
	  {
	    outputOperands.push_back(make_shared(singleton_object_pool<RegisterAST>::construct(IntelRegTable[5][locs->modrm_rm])));
	    break;
	  }
	default:
	  assert(!"2-bit value modrm_mod out of range");
	  break;
	};
	
	break;
      case am_X:
	{
	  Expression::Ptr ds(make_shared(singleton_object_pool<RegisterAST>::construct(r_DS)));
	  Expression::Ptr si(make_shared(singleton_object_pool<RegisterAST>::construct(r_SI)));
	  Expression::Ptr segmentOffset(make_shared(singleton_object_pool<Immediate>::construct(Result(u32, 0x10))));
	  
	  Expression::Ptr ds_segment = makeMultiplyExpression(ds, segmentOffset, u32);
	  Expression::Ptr ds_si = makeAddExpression(ds_segment, si, u32);
	  outputOperands.push_back(make_shared(singleton_object_pool<Dereference>::construct(ds_si, makeSizeType(operand.optype))));
	}
	break;
      case am_Y:
	{
	  Expression::Ptr es(make_shared(singleton_object_pool<RegisterAST>::construct(r_ES)));
	  Expression::Ptr di(make_shared(singleton_object_pool<RegisterAST>::construct(r_DI)));
	  Expression::Ptr es_segment = 
	  makeMultiplyExpression(es, make_shared(singleton_object_pool<Immediate>::construct(Result(u32, 0x10))), u32);
	  Expression::Ptr es_di = makeAddExpression(es_segment, di, u32);
	  outputOperands.push_back(make_shared(singleton_object_pool<Dereference>::construct(es_di, makeSizeType(operand.optype))));
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
	Expression::Ptr op(make_shared(singleton_object_pool<RegisterAST>::construct(registerID)));
	outputOperands.push_back(op);
	}
	
	break;
      case am_stackH:
      case am_stackP:
	// handled elsewhere
	break;
      case am_allgprs:
	{
	  Expression::Ptr op(make_shared(singleton_object_pool<RegisterAST>::construct(r_ALLGPRS)));
	  outputOperands.push_back(op);
	}
	break;
      default:
	printf("decodeOneOperand() called with unknown addressing method %d\n", operand.admet);
	break;
      };
      return true;
    }

    extern ia32_entry invalid;
    
    void InstructionDecoder::doIA32Decode()
    {
      if(decodedInstruction == NULL)
      {
	decodedInstruction = reinterpret_cast<ia32_instruction*>(malloc(sizeof(ia32_instruction)));
        assert(decodedInstruction);
      }
      if(locs == NULL)
      {
          locs = reinterpret_cast<ia32_locations*>(malloc(sizeof(ia32_locations)));
          assert(locs);
      }
      
      locs = new(locs) ia32_locations();
      assert(locs);
      decodedInstruction = new (decodedInstruction) ia32_instruction(NULL, NULL, locs);
      assert(locs);
      ia32_decode(IA32_DECODE_PREFIXES, rawInstruction, *decodedInstruction);
      sizePrefixPresent = (decodedInstruction->getPrefix()->getOperSzPrefix() == 0x66);
    }
    
    unsigned int InstructionDecoder::decodeOpcode()
    {
        static ia32_entry invalid = { e_No_Entry, 0, 0, true, { {0,0}, {0,0}, {0,0} }, 0, 0 };
        doIA32Decode();
        if(decodedInstruction->getEntry()) {
            m_Operation = make_shared(singleton_object_pool<Operation>::construct(decodedInstruction->getEntry(),
                                        decodedInstruction->getPrefix(), locs));
        }
        else
        {
            // Gap parsing can trigger this case; in particular, when it encounters prefixes in an invalid order.
            // Notably, if a REX prefix (0x40-0x48) appears followed by another prefix (0x66, 0x67, etc)
            // we'll reject the instruction as invalid and send it back with no entry.  Since this is a common
            // byte sequence to see in, for example, ASCII strings, we want to simply accept this and move on, not
            // yell at the user.
            m_Operation = make_shared(singleton_object_pool<Operation>::construct(&invalid,
                                        decodedInstruction->getPrefix(), locs));
        }
        return decodedInstruction->getSize();
    }
    
    bool InstructionDecoder::decodeOperands(std::vector<Expression::Ptr>& operands)
    {
      operands.reserve(3);
      
      if(!decodedInstruction) return false;
      assert(locs);
      
      for(unsigned i = 0; i < 3; i++)
      {
	if(decodedInstruction->getEntry()->operands[i].admet == 0 && decodedInstruction->getEntry()->operands[i].optype == 0) return true;
	if(!decodeOneOperand(decodedInstruction->getEntry()->operands[i], operands))
	{
	  return false;
	}
      }
      
      return true;
    }

    void InstructionDecoder::resetBuffer(const unsigned char* buffer, unsigned int size = 0)
    {
      rawInstruction = buffer;
      bufferBegin = buffer;
      bufferSize = size;
    }
    
  };
};

