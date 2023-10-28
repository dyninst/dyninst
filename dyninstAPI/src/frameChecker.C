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

#include "frameChecker.h"
#include "instructionAPI/h/InstructionDecoder.h"
#include "Architecture.h"
#include "registers/MachRegister.h"


using namespace Dyninst;
using namespace InstructionAPI;


frameChecker::frameChecker(const unsigned char* addr, size_t max_length, Dyninst::Architecture a)
  : arch(a)
{
  assert((arch == Arch_x86) ||
	 (arch == Arch_x86_64));

  // How many instructions in our stack frame idioms?
  static const unsigned max_insns = 3;
  
  InstructionDecoder d(addr, max_length, arch);
  unsigned bytesDecoded = 0;
  
  for(unsigned i = 0; i < max_insns && bytesDecoded < max_length; i++)
  {
    m_Insns.push_back(d.decode());
    addr += m_Insns.back().size();
    bytesDecoded += m_Insns.back().size();
  }
}

frameChecker::~frameChecker()
{
}


bool frameChecker::isReturn() const
{
  entryID firstOpcode = m_Insns[0].getOperation().getID();
  return (firstOpcode == e_ret_far) || (firstOpcode == e_ret_near);
}

bool frameChecker::isStackPreamble() const
{
  if(m_Insns[0].getOperation().getID() != e_push)
  {
    return false;
  }
  if(!isMovStackToBase(1) && !isMovStackToBase(2))
  {
    return false;
  }
  return true;
}

bool frameChecker::isMovStackToBase(unsigned index_to_check) const
{
   if(m_Insns.size() <= index_to_check) return false;
  if(m_Insns[index_to_check].getOperation().getID() == e_mov)
  {
      RegisterAST::Ptr stack_ptr(new RegisterAST(MachRegister::getStackPointer(Arch_x86)));
      RegisterAST::Ptr base_ptr(new RegisterAST(MachRegister::getFramePointer(Arch_x86)));
    std::string debugMe = m_Insns[index_to_check].format();
    
    if(m_Insns[index_to_check].isWritten(base_ptr) && m_Insns[index_to_check].isRead(stack_ptr))
    {
      return true;
    }
    stack_ptr = RegisterAST::Ptr(new RegisterAST(MachRegister::getStackPointer(Arch_x86_64)));
    base_ptr = RegisterAST::Ptr(new RegisterAST(MachRegister::getFramePointer(Arch_x86_64)));
    if(m_Insns[index_to_check].isWritten(base_ptr) && m_Insns[index_to_check].isRead(stack_ptr))
    {
      return true;
    }	
  }
  return false;
}


bool frameChecker::isStackFrameSetup() const
{
  return isMovStackToBase(0);
}
