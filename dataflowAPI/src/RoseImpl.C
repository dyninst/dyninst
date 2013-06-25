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

#include <string>
#include <map>
#include <iostream>

#include "../rose/SgAsmx86Instruction.h"
#include "../rose/SgAsmPowerpcInstruction.h"
#include "external/rose/rose-compat.h"
#include "../rose/x86InstructionSemantics.h"
#include "../rose/powerpcInstructionSemantics.h"

// SgAsmType.h

SgAsmType::SgAsmType()
{

}

std::string SgAsmType::class_name() const
{
  return "SgAsmType";
}

VariantT SgAsmType::variantT() const
{
  return static_variant;
}

SgAsmType::~SgAsmType()
{

}

SgAsmTypeByte::SgAsmTypeByte()
{

}

std::string SgAsmTypeByte::class_name() const
{
  return "SgAsmTypeByte";
}

VariantT SgAsmTypeByte::variantT() const
{
  return static_variant;
}

SgAsmTypeByte::~SgAsmTypeByte()
{

}

SgAsmTypeWord::SgAsmTypeWord()
{

}

std::string SgAsmTypeWord::class_name() const
{
  return "SgAsmTypeWord";
}

VariantT SgAsmTypeWord::variantT() const
{
  return static_variant;
}

SgAsmTypeWord::~SgAsmTypeWord()
{

}

SgAsmTypeDoubleWord::SgAsmTypeDoubleWord()
{

}

std::string SgAsmTypeDoubleWord::class_name() const
{
  return "SgAsmTypeDoubleWord";
}

VariantT SgAsmTypeDoubleWord::variantT() const
{
  return static_variant;
}

SgAsmTypeDoubleWord::~SgAsmTypeDoubleWord()
{

}

SgAsmTypeQuadWord::SgAsmTypeQuadWord()
{

}

std::string SgAsmTypeQuadWord::class_name() const
{
  return "SgAsmTypeQuadWord";
}

VariantT SgAsmTypeQuadWord::variantT() const
{
  return static_variant;
}

SgAsmTypeQuadWord::~SgAsmTypeQuadWord()
{

}

SgAsmTypeSingleFloat::SgAsmTypeSingleFloat()
{

}

std::string SgAsmTypeSingleFloat::class_name() const
{
  return "SgAsmTypeSingleFloat";
}

VariantT SgAsmTypeSingleFloat::variantT() const
{
  return static_variant;
}

SgAsmTypeSingleFloat::~SgAsmTypeSingleFloat()
{

}

SgAsmTypeDoubleFloat::SgAsmTypeDoubleFloat()
{

}

std::string SgAsmTypeDoubleFloat::class_name() const
{
  return "SgAsmTypeDoubleFloat";
}

VariantT SgAsmTypeDoubleFloat::variantT() const
{
  return static_variant;
}

SgAsmTypeDoubleFloat::~SgAsmTypeDoubleFloat()
{

}

// SgAsmExpression.h

SgAsmExpression::SgAsmExpression()
{

}

SgAsmType* SgAsmExpression::get_type()
{
  return new SgAsmType();
}

std::string SgAsmExpression::class_name() const
{
  return "SgAsmExpression";
}

VariantT SgAsmExpression::variantT() const
{
  return static_variant;
}

SgAsmExpression::~SgAsmExpression()
{
}

SgAsmValueExpression::SgAsmValueExpression()
{
  p_unfolded_expression_tree = NULL;
  p_bit_offset = 0;
  p_bit_size = 0;
}

SgAsmType* SgAsmValueExpression::get_type()
{
  return new SgAsmType();
}

std::string SgAsmValueExpression::class_name() const
{
  return "SgAsmValueExpression";
}

VariantT SgAsmValueExpression::variantT() const
{
  return static_variant;
}

SgAsmValueExpression::~SgAsmValueExpression()
{

}

SgAsmByteValueExpression::SgAsmByteValueExpression(unsigned char value)
{
  p_value = value;
}

SgAsmType* SgAsmByteValueExpression::get_type()
{
  return new SgAsmTypeByte();
}

std::string SgAsmByteValueExpression::class_name() const
{
  return "SgAsmByteValueExpression";
}

VariantT SgAsmByteValueExpression::variantT() const
{
  return static_variant;
}

uint8_t SgAsmByteValueExpression::get_value() const
{
  return p_value;
}

SgAsmByteValueExpression::~SgAsmByteValueExpression()
{

}

SgAsmWordValueExpression::SgAsmWordValueExpression(unsigned short value)
{
  p_value = value;
}

SgAsmType* SgAsmWordValueExpression::get_type()
{
  return new SgAsmTypeWord();
}

std::string SgAsmWordValueExpression::class_name() const
{
  return "SgAsmWordValueExpression";
}

VariantT SgAsmWordValueExpression::variantT() const
{
  return static_variant;
}

uint16_t SgAsmWordValueExpression::get_value() const
{
  return p_value;
}

SgAsmWordValueExpression::~SgAsmWordValueExpression()
{

}

SgAsmDoubleWordValueExpression::SgAsmDoubleWordValueExpression(unsigned int value)
{
  p_value = value;
}

SgAsmType* SgAsmDoubleWordValueExpression::get_type()
{
  return new SgAsmTypeDoubleWord();
}

std::string SgAsmDoubleWordValueExpression::class_name() const
{
  return "SgAsmDoubleWordValueExpression";
}

VariantT SgAsmDoubleWordValueExpression::variantT() const
{
  return static_variant;
}

uint32_t SgAsmDoubleWordValueExpression::get_value() const
{
  return p_value;
}

SgAsmDoubleWordValueExpression::~SgAsmDoubleWordValueExpression()
{

}

SgAsmQuadWordValueExpression::SgAsmQuadWordValueExpression(uint64_t value)
{
  p_value = value;
}

SgAsmType* SgAsmQuadWordValueExpression::get_type()
{
  return new SgAsmTypeQuadWord();
}

std::string SgAsmQuadWordValueExpression::class_name() const
{
  return "SgAsmQuadWordValueExpression";
}

VariantT SgAsmQuadWordValueExpression::variantT() const
{
  return static_variant;
}

uint64_t SgAsmQuadWordValueExpression::get_value() const
{
  return p_value;
}

SgAsmQuadWordValueExpression::~SgAsmQuadWordValueExpression()
{

}

SgAsmSingleFloatValueExpression::SgAsmSingleFloatValueExpression(float value)
{
  p_value = value;
}

SgAsmType* SgAsmSingleFloatValueExpression::get_type()
{
  return new SgAsmTypeSingleFloat();
}

std::string SgAsmSingleFloatValueExpression::class_name() const
{
  return "SgAsmSingleFloatValueExpression";
}

VariantT SgAsmSingleFloatValueExpression::variantT() const
{
  return static_variant;
}

SgAsmSingleFloatValueExpression::~SgAsmSingleFloatValueExpression()
{

}

SgAsmDoubleFloatValueExpression::SgAsmDoubleFloatValueExpression(double value)
{
  p_value = value;
}

SgAsmType* SgAsmDoubleFloatValueExpression::get_type()
{
  return new SgAsmTypeDoubleFloat();
}

std::string SgAsmDoubleFloatValueExpression::class_name() const
{
  return "SgAsmDoubleFloatValueExpression";
}

VariantT SgAsmDoubleFloatValueExpression::variantT() const
{
  return static_variant;
}

SgAsmDoubleFloatValueExpression::~SgAsmDoubleFloatValueExpression()
{

}

SgAsmBinaryExpression::SgAsmBinaryExpression(SgAsmExpression* lhs, SgAsmExpression* rhs)
{
  p_lhs = lhs;
  p_rhs = rhs;
}

SgAsmType* SgAsmBinaryExpression::get_type()
{
  // TODO this might not be safe
  return p_lhs->get_type();
}

std::string SgAsmBinaryExpression::class_name() const
{
  return "SgAsmBinaryExpression";
}

VariantT SgAsmBinaryExpression::variantT() const
{
  return static_variant;
}

SgAsmExpression* SgAsmBinaryExpression::get_lhs() const
{
  return p_lhs;
}

SgAsmExpression* SgAsmBinaryExpression::get_rhs() const
{
  return p_rhs;
}

SgAsmBinaryExpression::~SgAsmBinaryExpression()
{

}

SgAsmBinaryAdd::SgAsmBinaryAdd(SgAsmExpression* lhs, SgAsmExpression* rhs)
 : SgAsmBinaryExpression(lhs, rhs)
{

}

SgAsmType* SgAsmBinaryAdd::get_type()
{
  return ((SgAsmBinaryExpression*)this)->get_type();
}

std::string SgAsmBinaryAdd::class_name() const
{
  return "SgAsmBinaryAdd";
}

VariantT SgAsmBinaryAdd::variantT() const
{
  return static_variant;
}

SgAsmBinaryAdd::~SgAsmBinaryAdd()
{

}

SgAsmBinaryMultiply::SgAsmBinaryMultiply(SgAsmExpression* lhs, SgAsmExpression* rhs)
 : SgAsmBinaryExpression(lhs, rhs)
{

}

SgAsmType* SgAsmBinaryMultiply::get_type()
{
  return ((SgAsmBinaryExpression*)this)->get_type();
}

std::string SgAsmBinaryMultiply::class_name() const
{
  return "SgAsmBinaryMultiply";
}

VariantT SgAsmBinaryMultiply::variantT() const
{
  return static_variant;
}

SgAsmBinaryMultiply::~SgAsmBinaryMultiply()
{

}

SgAsmRegisterReferenceExpression::SgAsmRegisterReferenceExpression()
{
    p_type = NULL;
}

SgAsmType* SgAsmRegisterReferenceExpression::get_type()
{
  return p_type;
}
void SgAsmRegisterReferenceExpression::set_type(SgAsmType* type)
{
    p_type = type;
}

std::string SgAsmRegisterReferenceExpression::class_name() const
{
  return "SgAsmRegisterReferenceExpression";
}

VariantT SgAsmRegisterReferenceExpression::variantT() const
{
  return static_variant;
}

SgAsmRegisterReferenceExpression::~SgAsmRegisterReferenceExpression()
{

}



SgAsmx86RegisterReferenceExpression::SgAsmx86RegisterReferenceExpression(X86RegisterClass register_class, int register_number, X86PositionInRegister position_in_register)
{
  p_register_class = register_class;
  p_register_number = register_number;
  p_position_in_register = position_in_register;

  // Need to fix for 64-bit
  // Right now semantics only accepts byte, word, doubleword

  switch (position_in_register)
  {
    case x86_regpos_low_byte:
    case x86_regpos_high_byte:
      p_type = new SgAsmTypeByte();
      break;
    case x86_regpos_word:
      p_type = new SgAsmTypeWord();
      break;
    case x86_regpos_all:
    case x86_regpos_dword:
      p_type = new SgAsmTypeDoubleWord();
      break;
    case x86_regpos_qword:
    default:
      p_type = 0;
  }
}


std::string SgAsmx86RegisterReferenceExpression::class_name() const
{
  return "SgAsmx86RegisterReferenceExpression";
}

VariantT SgAsmx86RegisterReferenceExpression::variantT() const
{
  return static_variant;
}

int SgAsmx86RegisterReferenceExpression::get_register_number() const
{
  return p_register_number;
}

X86RegisterClass SgAsmx86RegisterReferenceExpression::get_register_class() const
{
  return p_register_class;
}

X86PositionInRegister SgAsmx86RegisterReferenceExpression::get_position_in_register() const
{
  return p_position_in_register;
}

SgAsmx86RegisterReferenceExpression::~SgAsmx86RegisterReferenceExpression()
{

}

SgAsmMemoryReferenceExpression::SgAsmMemoryReferenceExpression(SgAsmExpression* address, SgAsmExpression* segment)
{
  p_address = address;
  p_segment = segment;
  p_type = NULL;
}

SgAsmType* SgAsmMemoryReferenceExpression::get_type()
{
  return p_type;
}

std::string SgAsmMemoryReferenceExpression::class_name() const
{
  return "SgAsmMemoryReferenceExpression";
}

VariantT SgAsmMemoryReferenceExpression::variantT() const
{
  return static_variant;
}

SgAsmExpression * SgAsmMemoryReferenceExpression::get_address() const
{
  return p_address;
}

SgAsmExpression* SgAsmMemoryReferenceExpression::get_segment() const
{
  return p_segment;
}

void SgAsmMemoryReferenceExpression::set_type(SgAsmType* type)
{
  p_type = type;
}

SgAsmMemoryReferenceExpression::~SgAsmMemoryReferenceExpression()
{

}

uint64_t getAsmSignedConstant(SgAsmValueExpression* valexp)
{
  switch(valexp->variantT())
  {
    case V_SgAsmByteValueExpression:
      return (uint64_t) ((int8_t)  static_cast<SgAsmByteValueExpression*>(valexp)->get_value());
      break;
    case V_SgAsmWordValueExpression:
      return (uint64_t) ((int16_t) static_cast<SgAsmWordValueExpression*>(valexp)->get_value());
      break;
    case V_SgAsmDoubleWordValueExpression:
      return (uint64_t) ((int32_t) static_cast<SgAsmDoubleWordValueExpression*>(valexp)->get_value());
      break;
    case V_SgAsmQuadWordValueExpression:
      return (uint64_t) ((int64_t) static_cast<SgAsmQuadWordValueExpression*>(valexp)->get_value());
      break;
    default:
      return 0; // error
  }
}

// conversions.h

uint64_t SageInterface::getAsmSignedConstant(SgAsmValueExpression *valexp)
{
  return ::getAsmSignedConstant(valexp);
}

SgAsmValueExpression* isSgAsmValueExpression(SgNode* s)
{
  return dynamic_cast<SgAsmValueExpression*>(s);
}

SgAsmBinaryAdd* isSgAsmBinaryAdd(SgNode* s)
{
  return dynamic_cast<SgAsmBinaryAdd*>(s);
}

SgAsmBinaryMultiply* isSgAsmBinaryMultiply(SgNode* s)
{
  return dynamic_cast<SgAsmBinaryMultiply*>(s);
}

SgAsmx86RegisterReferenceExpression* isSgAsmx86RegisterReferenceExpression( SgNode *n)
{
  return dynamic_cast<SgAsmx86RegisterReferenceExpression*>(n);
}

SgAsmByteValueExpression* isSgAsmByteValueExpression(SgNode* s)
{
  return dynamic_cast<SgAsmByteValueExpression*>(s);
}
SgAsmWordValueExpression* isSgAsmWordValueExpression(SgNode* s)
{
    return dynamic_cast<SgAsmWordValueExpression*>(s);
}
SgAsmDoubleWordValueExpression* isSgAsmDoubleWordValueExpression(SgNode* s)
{
    return dynamic_cast<SgAsmDoubleWordValueExpression*>(s);
}
SgAsmQuadWordValueExpression* isSgAsmQuadWordValueExpression(SgNode* s)
{
    return dynamic_cast<SgAsmQuadWordValueExpression*>(s);
}

SgAsmMemoryReferenceExpression* isSgAsmMemoryReferenceExpression(SgNode* s)
{
  return dynamic_cast<SgAsmMemoryReferenceExpression*>(s);
}

SgAsmPowerpcRegisterReferenceExpression* isSgAsmPowerpcRegisterReferenceExpression(SgNode* s)
{
    return dynamic_cast<SgAsmPowerpcRegisterReferenceExpression*>(s);
}

const char * regclassToString(X86RegisterClass)
{
  return "NOT IMPLEMENTED";
}
const char * regclassToString(PowerpcRegisterClass c)
{
    switch(c)
    {
        case powerpc_regclass_gpr:
            return "GPR";
        case powerpc_regclass_spr:
            return "SPR";
        case powerpc_regclass_fpr:
            return "FPR";
        case powerpc_regclass_sr:
            return "SR";
        default:
            return "unexpected register class--not gpr, spr, segment, or fpr";
    }
}

// SgAsmx86Instruction.h
SgAsmPowerpcInstruction::SgAsmPowerpcInstruction(rose_addr_t address, std::string mnemonic, PowerpcInstructionKind kind)
{
    p_address = address;
    p_mnemonic = mnemonic;
    p_kind = kind;
    p_operandList = NULL;
}

SgAsmx86Instruction::SgAsmx86Instruction(rose_addr_t address,
                        std::string mnemonic,
                        X86InstructionKind kind,
                        X86InstructionSize baseSize,
                        X86InstructionSize operandSize,
                        X86InstructionSize addressSize)
{
  p_address = address;
  p_mnemonic = mnemonic;
  p_kind = kind;
  p_baseSize = baseSize;
  p_operandSize = operandSize;
  p_addressSize = addressSize;

  // these are as-yet unknown, but should still be initialized
  p_lockPrefix = false;
  p_repeatPrefix = x86_repeat_none;
  p_branchPrediction = x86_branch_prediction_none;
  p_segmentOverride = x86_segreg_none;
}

std::string SgAsmx86Instruction::class_name() const
{
  return "SgAsmx86Instruction";
}

std::string SgAsmPowerpcInstruction::class_name() const
{
    return "SgAsmPowerpcInstruction";
}

VariantT SgAsmx86Instruction::variantT() const
{
  return static_variant;
}
VariantT SgAsmPowerpcInstruction::variantT() const
{
    return static_variant;
}

SgAsmx86Instruction::~SgAsmx86Instruction()
{

}

rose_addr_t SgAsmInstruction::get_address() const
{
  return p_address;
}


X86InstructionKind SgAsmx86Instruction::get_kind() const
{
  return p_kind;
}

PowerpcInstructionKind SgAsmPowerpcInstruction::get_kind() const
{
    return p_kind;
}

X86InstructionSize SgAsmx86Instruction::get_addressSize() const
{
  return p_addressSize;
}

X86InstructionSize SgAsmx86Instruction::get_operandSize() const
{
  return p_operandSize;
}

X86SegmentRegister SgAsmx86Instruction::get_segmentOverride() const
{
  return p_segmentOverride;
}

SgAsmOperandList* SgAsmInstruction::get_operandList() const
{
  return p_operandList;
}

std::string SgAsmInstruction::get_mnemonic() const
{
  return p_mnemonic;
}

SgUnsignedCharList SgAsmInstruction::get_raw_bytes() const
{
  return p_raw_bytes;
}

void SgAsmInstruction::set_address(rose_addr_t address)
{
  p_address = address;
}

void SgAsmx86Instruction::set_kind(X86InstructionKind kind)
{
  p_kind = kind;
}
void SgAsmPowerpcInstruction::set_kind(PowerpcInstructionKind kind)
{
    p_kind = kind;
}

void SgAsmx86Instruction::set_addressSize(X86InstructionSize size)
{
  p_addressSize = size;
}

void SgAsmx86Instruction::set_operandSize(X86InstructionSize size)
{
  p_operandSize = size;
}

void SgAsmInstruction::set_operandList(SgAsmOperandList* operandList)
{
    p_operandList = operandList;
}

void SgAsmInstruction::set_mnemonic(std::string mnemonic)
{
  p_mnemonic = mnemonic;
}

void SgAsmInstruction::set_raw_bytes(std::vector<unsigned char> raw_bytes)
{
  p_raw_bytes = raw_bytes;
}

// SgAsmOperandList.h

SgAsmOperandList::SgAsmOperandList()
{

}

std::string SgAsmOperandList::class_name() const
{
  return "SgAsmOperandList";
}

VariantT SgAsmOperandList::variantT() const
{
  return static_variant;
}

SgAsmExpressionPtrList& SgAsmOperandList::get_operands()
{
  return p_operands;
}

void SgAsmOperandList::append_operand(SgAsmExpression* operand)
{
  p_operands.push_back(operand);
}

SgAsmOperandList::~SgAsmOperandList()
{

}

//        PowerpcRegisterClass p_register_class;
//        int p_register_number;
//        PowerpcConditionRegisterAccessGranularity p_conditionRegisterGranularity;

PowerpcRegisterClass SgAsmPowerpcRegisterReferenceExpression::get_register_class() const
{
    return p_register_class;
}
void SgAsmPowerpcRegisterReferenceExpression::set_register_class(PowerpcRegisterClass register_class)
{
    p_register_class = register_class;
}
int SgAsmPowerpcRegisterReferenceExpression::get_register_number() const
{
    return p_register_number;
}
void SgAsmPowerpcRegisterReferenceExpression::set_register_number(int register_number)
{
    p_register_number = register_number;
}
PowerpcConditionRegisterAccessGranularity
SgAsmPowerpcRegisterReferenceExpression::get_conditionRegisterGranularity() const
{
    return p_conditionRegisterGranularity;
}
void SgAsmPowerpcRegisterReferenceExpression::set_conditionRegisterGranularity(
        PowerpcConditionRegisterAccessGranularity conditionRegisterGranularity)
{
    p_conditionRegisterGranularity = conditionRegisterGranularity;
}

SgAsmPowerpcRegisterReferenceExpression::~SgAsmPowerpcRegisterReferenceExpression()
{
}

SgAsmPowerpcRegisterReferenceExpression::SgAsmPowerpcRegisterReferenceExpression(
        PowerpcRegisterClass register_class,
        int register_number,
        PowerpcConditionRegisterAccessGranularity conditionRegisterGranularity)
    : p_register_class(register_class), p_register_number(register_number),
    p_conditionRegisterGranularity(conditionRegisterGranularity)
{
}
