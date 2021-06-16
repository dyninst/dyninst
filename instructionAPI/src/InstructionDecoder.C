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

#include "InstructionDecoder.h"
#include "InstructionDecoderImpl.h"
#include "Instruction.h"

using namespace std;
namespace Dyninst
{
  namespace InstructionAPI
  {
    INSTRUCTION_EXPORT InstructionDecoder::InstructionDecoder(const unsigned char* buffer_, size_t size, Architecture arch) :
        m_buf(buffer_, size)
    {
        m_Impl = InstructionDecoderImpl::makeDecoderImpl(arch);
        m_Impl->setMode(arch == Arch_x86_64);
    }
    INSTRUCTION_EXPORT InstructionDecoder::InstructionDecoder(const void* buffer_, size_t size, Architecture arch) :
        m_buf(reinterpret_cast<const unsigned char*>(buffer_), size)
    {
        m_Impl = InstructionDecoderImpl::makeDecoderImpl(arch);
        m_Impl->setMode(arch == Arch_x86_64);
    }
    
    INSTRUCTION_EXPORT Instruction InstructionDecoder::decode()
    {
      if(m_buf.start >= m_buf.end) return Instruction();
      return m_Impl->decode(m_buf);
    }
    
    INSTRUCTION_EXPORT Instruction InstructionDecoder::decode(const unsigned char* b)
    {
      buffer tmp(b, b+maxInstructionLength);
      
      return m_Impl->decode(tmp);
    }
    INSTRUCTION_EXPORT void InstructionDecoder::doDelayedDecode(const Instruction* i)
    {
        m_Impl->doDelayedDecode(i);
    }
    

  }
}

