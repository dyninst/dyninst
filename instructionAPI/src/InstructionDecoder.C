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
#include "../h/InstructionDecoder-power.h"
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
    
    
    INSTRUCTION_EXPORT InstructionDecoder::InstructionDecoder(const unsigned char* buffer, size_t size, Architecture arch) :
      bufferBegin(buffer),
      bufferSize(size),
      rawInstruction(bufferBegin),
      m_Arch(arch)
    {
        m_Impl = InstructionDecoderImpl::makeDecoderImpl(m_Arch);
    }
    INSTRUCTION_EXPORT InstructionDecoder::InstructionDecoder(const void* buffer, size_t size, Architecture arch) :
      bufferBegin(reinterpret_cast<const unsigned char*>(buffer)),
      bufferSize(size),
      rawInstruction(bufferBegin),
      m_Arch(arch)
    {
        m_Impl = InstructionDecoderImpl::makeDecoderImpl(m_Arch);
    }
    INSTRUCTION_EXPORT InstructionDecoder::InstructionDecoder() :
      bufferBegin(NULL),
      bufferSize(0),
      rawInstruction(NULL),
      m_Arch(Arch_none)
    {
        assert(!"no architecture specified, decoder won't decode!");
    }
    INSTRUCTION_EXPORT InstructionDecoder::InstructionDecoder(const InstructionDecoder& o) :
    bufferBegin(o.bufferBegin),
    bufferSize(o.bufferSize),
    rawInstruction(o.rawInstruction),
    m_Arch(o.m_Arch),
    m_Impl(o.m_Impl)
    {
    }
    INSTRUCTION_EXPORT InstructionDecoder::~InstructionDecoder()
    {
    }
    Instruction* InstructionDecoderImpl::makeInstruction(entryID opcode, const char* mnem,
            unsigned int decodedSize, const unsigned char* raw)
    {
        Operation::Ptr tmp(make_shared(singleton_object_pool<Operation>::construct(opcode, mnem, m_Arch)));
        return singleton_object_pool<Instruction>::construct(tmp, decodedSize, raw, m_Arch);
    }
    
    INSTRUCTION_EXPORT Instruction::Ptr InstructionDecoder::decode()
    {
      if(rawInstruction < bufferBegin || rawInstruction >= bufferBegin + bufferSize) return Instruction::Ptr();
      buffer b(rawInstruction, bufferBegin + bufferSize);
      
      Instruction::Ptr ret = m_Impl->decode(b);
      
      rawInstruction = b.start;
      return ret;
    }
    
    INSTRUCTION_EXPORT Instruction::Ptr InstructionDecoder::decode(const unsigned char* b)
    {
      buffer tmp(b, b+maxInstructionLength);
      
      return m_Impl->decode(tmp);
    }
    INSTRUCTION_EXPORT void InstructionDecoder::doDelayedDecode(const Instruction* i)
    {
        m_Impl->doDelayedDecode(i);
    }
    
    INSTRUCTION_EXPORT Instruction::Ptr InstructionDecoderImpl::decode(InstructionDecoder::buffer& b)
    {
        const unsigned char* start = b.start;
        decodeOpcode(b);
	unsigned int decodedSize = b.start - start;
	
        return make_shared(singleton_object_pool<Instruction>::construct(m_Operation, decodedSize,
                           start, m_Arch));
        
    }

    std::map<Architecture, InstructionDecoderImpl::Ptr> InstructionDecoderImpl::impls;
    INSTRUCTION_EXPORT InstructionDecoderImpl::Ptr InstructionDecoderImpl::makeDecoderImpl(Architecture a)
    {
        if(impls.empty())
        {
            impls[Arch_x86] = Ptr(new InstructionDecoder_x86(Arch_x86));
            impls[Arch_x86_64] = Ptr(new InstructionDecoder_x86(Arch_x86_64));
            impls[Arch_ppc32] = Ptr(new InstructionDecoder_power(Arch_ppc32));
            impls[Arch_ppc64] = Ptr(new InstructionDecoder_power(Arch_ppc64));
        }
        std::map<Architecture, Ptr>::const_iterator foundImpl = impls.find(a);
        if(foundImpl == impls.end())
        {
            return Ptr();
        }
        return foundImpl->second;
    }
    Expression::Ptr InstructionDecoderImpl::makeAddExpression(Expression::Ptr lhs, Expression::Ptr rhs, Result_Type resultType)
    {
      static BinaryFunction::funcT::Ptr adder(new BinaryFunction::addResult());
      
      return make_shared(singleton_object_pool<BinaryFunction>::construct(lhs, rhs, resultType, adder));
    }
    Expression::Ptr InstructionDecoderImpl::makeMultiplyExpression(Expression::Ptr lhs, Expression::Ptr rhs,
            Result_Type resultType)
    {
      static BinaryFunction::funcT::Ptr multiplier(new BinaryFunction::multResult());
      return make_shared(singleton_object_pool<BinaryFunction>::construct(lhs, rhs, resultType, multiplier));
    }
    Expression::Ptr InstructionDecoderImpl::makeDereferenceExpression(Expression::Ptr addrToDereference, Result_Type resultType)
    {
        return make_shared(singleton_object_pool<Dereference>::construct(addrToDereference, resultType));
    }
    Expression::Ptr InstructionDecoderImpl::makeRegisterExpression(MachRegister registerID)
    {
        int newID = registerID.val();
        int minusArch = newID & ~(registerID.getArchitecture());
        int convertedID = minusArch | m_Arch;
        MachRegister converted(convertedID);
        return make_shared(singleton_object_pool<RegisterAST>::construct(converted, 0, registerID.size() * 8));
    }
    

    void InstructionDecoder::setBuffer(const unsigned char* buffer, unsigned int size)
    {
        oldBuffer = rawInstruction;
        oldBufferBegin = bufferBegin;
        oldBufferSize = bufferSize;
      rawInstruction = buffer;
      bufferBegin = buffer;
      bufferSize = size;
    }

    void InstructionDecoder::resetBuffer()
    {
        rawInstruction = oldBuffer;
        bufferBegin = oldBufferBegin;
        bufferSize = oldBufferSize;
    }
    
  };
};

