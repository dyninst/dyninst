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

#if !defined(INSTRUCTION_DECODER_H)
#define INSTRUCTION_DECODER_H

#include "InstructionAST.h"
#include "Expression.h"
#include "Operation.h"
#include "Operand.h"
#include "Instruction.h"

#include <vector>

struct ia32_locations;
struct ia32_condition;
struct ia32_operand;
class ia32_instruction;
struct ia32_memacc;
    
namespace Dyninst
{
  namespace InstructionAPI
  {
    
    /// The %InstructionDecoder class decodes instructions, given a buffer of bytes and a length,
    /// and constructs an %Instruction.
    /// The %InstructionDecoder will, by default, be constructed to decode machine language
    /// on the platform on which it has been compiled.  The buffer
    /// will be treated as if there is an instruction stream starting at the beginning of the buffer.
    /// %InstructionDecoder objects are given a buffer from which to decode at construction.
    /// Calls to \c decode will proceed to decode instructions sequentially from that buffer until its
    /// end is reached.  At that point, all subsequent calls to \c decode will return an invalid
    /// %Instruction object.
    ///
    /// An %InstructionDecoder object may alternately be constructed without designating a buffer,
    /// and the buffer may be specified at the time \c decode is called.  This method of use may be
    /// more convenient for users who are decoding non-contiguous instructions.

    class InstructionDecoder
    {
      friend class Instruction;
      
    public:
      /// Construct an %InstructionDecoder object that decodes from \c buffer, up to \c size bytes.
      INSTRUCTION_EXPORT InstructionDecoder(const unsigned char* buffer, size_t size);
      
      /// Construct an %InstructionDecoder object with no buffer specified.
      INSTRUCTION_EXPORT InstructionDecoder();
      
      INSTRUCTION_EXPORT ~InstructionDecoder();
      INSTRUCTION_EXPORT InstructionDecoder(const InstructionDecoder& o);
      /// Decode the current instruction in this %InstructionDecoder object's buffer, interpreting it as 
      /// machine language of the type understood by this %InstructionDecoder.
      /// If the buffer does not contain a valid instruction stream, a null %Instruction pointer
      /// will be returned.  The %Instruction's \c size field will contain the size of the instruction decoded.
      INSTRUCTION_EXPORT Instruction::Ptr decode();
      /// Decode the instruction at \c buffer, interpreting it as machine language of the type
      /// understood by this %InstructionDecoder.  If the buffer does not contain a valid instruction stream, 
      /// a null %Instruction pointer will be returned.  The %Instruction's \c size field will contain
      /// the size of the instruction decoded.
      INSTRUCTION_EXPORT Instruction::Ptr decode(const unsigned char* buffer);
      
      INSTRUCTION_EXPORT void setMode(bool is64);
      
    protected:
      void resetBuffer(const unsigned char* buffer, unsigned int size);
      
      bool decodeOperands(std::vector<Expression::Ptr>& operands);

      bool decodeOneOperand(const ia32_operand& operand,
			    std::vector<Expression::Ptr>& outputOperands);
      unsigned int decodeOpcode();
      
      Expression::Ptr makeSIBExpression(unsigned int opType);
      Expression::Ptr makeModRMExpression(unsigned int opType);
      Expression::Ptr makeAddExpression(Expression::Ptr lhs, Expression::Ptr rhs, Result_Type resultType);
      Expression::Ptr makeMultiplyExpression(Expression::Ptr lhs, Expression::Ptr rhs, Result_Type resultType);
      Expression::Ptr getModRMDisplacement();
      int makeRegisterID(unsigned int intelReg, unsigned int opType, bool isExtendedReg = false);
      Expression::Ptr decodeImmediate(unsigned int opType, unsigned int position, bool isSigned = false);
      Result_Type makeSizeType(unsigned int opType);
      
    private:
      void doIA32Decode();
      
      ia32_locations* locs;
      ia32_condition* cond;
      ia32_memacc* mac;
      ia32_instruction* decodedInstruction;
      Operation::Ptr m_Operation;
      bool is32BitMode;
      bool sizePrefixPresent;
      const unsigned char* bufferBegin;
      size_t bufferSize;
      const unsigned char* rawInstruction;
      
      
    };
  };
};

#endif //!defined(INSTRUCTION_DECODER_H)
