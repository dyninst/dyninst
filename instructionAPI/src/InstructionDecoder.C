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
#include "../h/InstructionDecoder-x86.h"
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
      bufferBegin(buffer),
      bufferSize(size),
      rawInstruction(bufferBegin)
    {
    }
    INSTRUCTION_EXPORT InstructionDecoder::InstructionDecoder() :
      bufferBegin(NULL),
      bufferSize(0),
      rawInstruction(NULL)
    {
    }
    INSTRUCTION_EXPORT InstructionDecoder::InstructionDecoder(const InstructionDecoder& o) :
    bufferBegin(o.bufferBegin),
    bufferSize(o.bufferSize),
    rawInstruction(o.rawInstruction)
    {
    }
    INSTRUCTION_EXPORT InstructionDecoder::~InstructionDecoder()
    {
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
    Result_Type InstructionDecoder::makeSizeType(unsigned int opType)
    {
        return u32;
    }
    
    bool InstructionDecoder::decodeOperands(std::vector<Expression::Ptr>& operands)
    {
        return false;
    }

    void InstructionDecoder::resetBuffer(const unsigned char* buffer, unsigned int size = 0)
    {
      rawInstruction = buffer;
      bufferBegin = buffer;
      bufferSize = size;
    }
    dyn_detail::boost::shared_ptr<InstructionDecoder> makeDecoder(archID arch, const unsigned char* buffer, unsigned len)
    {
        switch(arch)
        {
            case x86:
                return dyn_detail::boost::shared_ptr<InstructionDecoder>(new InstructionDecoder_x86(buffer, len));
                break;
            default:
                assert(!"not implemented");
                return dyn_detail::boost::shared_ptr<InstructionDecoder>();
                break;
        }
        assert(!"can't happen");
        return dyn_detail::boost::shared_ptr<InstructionDecoder>();
    }
    
  };
};

