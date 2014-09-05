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

// Needs to be the first include.
#include "common/src/Types.h"

#include <stdio.h>
#include <string>
#include "../h/InstructionCategories.h"
#include "../h/Instruction.h"
#include "../h/Register.h"
#include "../h/Operation.h"
#include "InstructionDecoder.h"
#include "Dereference.h"
#include <boost/iterator/indirect_iterator.hpp>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <set>
#include <functional>

#define INSIDE_INSTRUCTION_API
#include "common/src/arch-x86.h"

using namespace std;
using namespace NS_x86;

#include "../../common/src/singleton_object_pool.h"

namespace Dyninst
{
  namespace InstructionAPI
  {

      static const int IAPI_major_version = 8;
      static const int IAPI_minor_version = 2;
      static const int IAPI_maintenance_version = 0;

      void Instruction::version(int& major, int& minor, int& maintenance)
      {
          major = IAPI_major_version;
          minor = IAPI_minor_version;
          maintenance = IAPI_maintenance_version;
      }

      int Instruction::numInsnsAllocated = 0;
    INSTRUCTION_EXPORT Instruction::Instruction(Operation::Ptr what,
			     size_t size, const unsigned char* raw,
                             Dyninst::Architecture arch)
      : m_InsnOp(what), m_Valid(true), arch_decoded_from(arch)
    {

        copyRaw(size, raw);

#if defined(DEBUG_INSN_ALLOCATIONS)
        numInsnsAllocated++;
        if((numInsnsAllocated % 1000) == 0)
        {
            fprintf(stderr, "Instruction CTOR, %d insns allocated\n", numInsnsAllocated);
        }
#endif    
    }

    void Instruction::copyRaw(size_t size, const unsigned char* raw)
    {
      
      if(raw)
      {
	m_size = size;
	m_RawInsn.small_insn = 0;
	if(size <= sizeof(m_RawInsn.small_insn))
	{
	  memcpy(&m_RawInsn.small_insn, raw, size);
	}
	else
	{
	  m_RawInsn.large_insn = new unsigned char[size];
	  memcpy(m_RawInsn.large_insn, raw, size);
	}
      }
      else
      {
	m_size = 0;
	m_RawInsn.small_insn = 0;
      }
    }

    void Instruction::decodeOperands() const
    {
        //m_Operands.reserve(5);
        InstructionDecoder dec(ptr(), size(), arch_decoded_from);
        dec.doDelayedDecode(this);
    }
    
    INSTRUCTION_EXPORT Instruction::Instruction() :
      m_Valid(false), m_size(0), arch_decoded_from(Arch_none)
    {

#if defined(DEBUG_INSN_ALLOCATIONS)
        numInsnsAllocated++;
        if((numInsnsAllocated % 1000) == 0)
        {
            fprintf(stderr, "Instruction CTOR, %d insns allocated\n", numInsnsAllocated);
        }
#endif
    }
    
    INSTRUCTION_EXPORT Instruction::~Instruction()
    {

      if(m_size > sizeof(m_RawInsn.small_insn))
      {
	delete[] m_RawInsn.large_insn;
      }
#if defined(DEBUG_INSN_ALLOCATIONS)
      numInsnsAllocated--;
      if((numInsnsAllocated % 1000) == 0)
      {
          fprintf(stderr, "Instruction DTOR, %d insns allocated\n", numInsnsAllocated);
      }
#endif      
    }

    INSTRUCTION_EXPORT Instruction::Instruction(const Instruction& o) :
      arch_decoded_from(o.arch_decoded_from)
    {
        m_Operands = o.m_Operands;
      //m_Operands.reserve(o.m_Operands.size());
      //std::copy(o.m_Operands.begin(), o.m_Operands.end(), std::back_inserter(m_Operands));
      
      m_size = o.m_size;
      if(o.m_size > sizeof(m_RawInsn.small_insn))
      {
	m_RawInsn.large_insn = new unsigned char[o.m_size];
	memcpy(m_RawInsn.large_insn, o.m_RawInsn.large_insn, m_size);
      }
      else
      {
	m_RawInsn.small_insn = o.m_RawInsn.small_insn;
      }

      m_InsnOp = o.m_InsnOp;
      m_Valid = o.m_Valid;
#if defined(DEBUG_INSN_ALLOCATIONS)
      numInsnsAllocated++;
      if((numInsnsAllocated % 1000) == 0)
      {
          fprintf(stderr, "Instruction COPY CTOR, %d insns allocated\n", numInsnsAllocated);
      }
#endif
    }

    INSTRUCTION_EXPORT const Instruction& Instruction::operator=(const Instruction& rhs)
    {
      m_Operands = rhs.m_Operands;
      //m_Operands.reserve(rhs.m_Operands.size());
      //std::copy(rhs.m_Operands.begin(), rhs.m_Operands.end(), std::back_inserter(m_Operands));
      if(m_size > sizeof(m_RawInsn.small_insn))
      {
	delete[] m_RawInsn.large_insn;
      }
      
      m_size = rhs.m_size;
      if(rhs.m_size > sizeof(m_RawInsn.small_insn))
      {
	m_RawInsn.large_insn = new unsigned char[rhs.m_size];
	memcpy(m_RawInsn.large_insn, rhs.m_RawInsn.large_insn, m_size);
      }
      else
      {
	m_RawInsn.small_insn = rhs.m_RawInsn.small_insn;
      }


      m_InsnOp = rhs.m_InsnOp;
      m_Valid = rhs.m_Valid;
      arch_decoded_from = rhs.arch_decoded_from;
      return *this;
    }    
    
    INSTRUCTION_EXPORT bool Instruction::isValid() const
    {
      return m_Valid;
    }
    
    INSTRUCTION_EXPORT const Operation& Instruction::getOperation() const
    {
      return *m_InsnOp;
    }
    
    INSTRUCTION_EXPORT void Instruction::getOperands(std::vector<Operand>& operands) const
    {
      if(m_Operands.empty())
      {
	decodeOperands();
      }
      
      std::copy(m_Operands.begin(), m_Operands.end(), std::back_inserter(operands));
    }
    
    INSTRUCTION_EXPORT Operand Instruction::getOperand(int index) const
     {
      if(m_Operands.empty())
      {
	decodeOperands();
      }

        if(index < 0 || index >= (int)(m_Operands.size()))
        {
	  // Out of range = empty operand
            return Operand(Expression::Ptr(), false, false);
        }
        std::list<Operand>::const_iterator found = m_Operands.begin();
        std::advance(found, index);
        return *found;
     }

     INSTRUCTION_EXPORT const void* Instruction::ptr() const
     {
         if(m_size > sizeof(m_RawInsn.small_insn))
         {
             return m_RawInsn.large_insn;
         }
         else
         {
             return reinterpret_cast<const void*>(&m_RawInsn.small_insn);
         }
     }
    INSTRUCTION_EXPORT unsigned char Instruction::rawByte(unsigned int index) const
    {
      if(index >= m_size) return 0;
      if(m_size > sizeof(m_RawInsn.small_insn))
      {
	return m_RawInsn.large_insn[index];
      }
      else
      {
	return reinterpret_cast<const unsigned char*>(&m_RawInsn.small_insn)[index];
      }
    }
    
    INSTRUCTION_EXPORT size_t Instruction::size() const
    {
      return m_size;
      
    }
    
    INSTRUCTION_EXPORT void Instruction::getReadSet(std::set<RegisterAST::Ptr>& regsRead) const
    {
      if(m_Operands.empty())
      {
	decodeOperands();
      }
      for(std::list<Operand>::const_iterator curOperand = m_Operands.begin();
	  curOperand != m_Operands.end();
	  ++curOperand)
      {
          curOperand->getReadSet(regsRead);
      }
      std::copy(m_InsnOp->implicitReads().begin(), m_InsnOp->implicitReads().end(), std::inserter(regsRead, regsRead.begin()));
      
    }
    
    INSTRUCTION_EXPORT void Instruction::getWriteSet(std::set<RegisterAST::Ptr>& regsWritten) const
    { 
      if(m_Operands.empty())
      {
	decodeOperands();
      }
      for(std::list<Operand>::const_iterator curOperand = m_Operands.begin();
	  curOperand != m_Operands.end();
	  ++curOperand)
      {
          curOperand->getWriteSet(regsWritten);
      }
      std::copy(m_InsnOp->implicitWrites().begin(), m_InsnOp->implicitWrites().end(), std::inserter(regsWritten,
regsWritten.begin()));
      
    }
    
    INSTRUCTION_EXPORT bool Instruction::isRead(Expression::Ptr candidate) const
    {
      if(m_Operands.empty())
      {
	decodeOperands();
      }
      for(std::list<Operand >::const_iterator curOperand = m_Operands.begin();
	  curOperand != m_Operands.end();
	  ++curOperand)
      {
          if(curOperand->isRead(candidate))
	{
	  return true;
	}
      }
      return m_InsnOp->isRead(candidate);
    }

    INSTRUCTION_EXPORT bool Instruction::isWritten(Expression::Ptr candidate) const
    {
      if(m_Operands.empty())
      {
	decodeOperands();
      }
      for(std::list<Operand>::const_iterator curOperand = m_Operands.begin();
	  curOperand != m_Operands.end();
	  ++curOperand)
      {
          if(curOperand->isWritten(candidate))
	{
	  return true;
	}
      }
      return m_InsnOp->isWritten(candidate);
    }
    
    INSTRUCTION_EXPORT bool Instruction::readsMemory() const
    {
      if(m_Operands.empty())
      {
	decodeOperands();
      }
      if(getCategory() == c_PrefetchInsn)
      {
          return false;
      }
      for(std::list<Operand>::const_iterator curOperand = m_Operands.begin();
	  curOperand != m_Operands.end();
	  ++curOperand)
      {
          if(curOperand->readsMemory())
	{
	  return true;
	}
      }
      return !m_InsnOp->getImplicitMemReads().empty();
    }
    
    INSTRUCTION_EXPORT bool Instruction::writesMemory() const
    {
      if(m_Operands.empty())
      {
	decodeOperands();
      }
      for(std::list<Operand>::const_iterator curOperand = m_Operands.begin();
          curOperand != m_Operands.end();
	  ++curOperand)
      {
          if(curOperand->writesMemory())
	{
	  return true;
	}
      }
      return !m_InsnOp->getImplicitMemWrites().empty();
    }
    
    INSTRUCTION_EXPORT void Instruction::getMemoryReadOperands(std::set<Expression::Ptr>& memAccessors) const
    {
      if(m_Operands.empty())
      {
	decodeOperands();
      }
      for(std::list<Operand>::const_iterator curOperand = m_Operands.begin();
	  curOperand != m_Operands.end();
	  ++curOperand)
      {
          curOperand->addEffectiveReadAddresses(memAccessors);
      }  
      std::copy(m_InsnOp->getImplicitMemReads().begin(), m_InsnOp->getImplicitMemReads().end(), std::inserter(memAccessors,
memAccessors.begin()));
    }
    
    INSTRUCTION_EXPORT void Instruction::getMemoryWriteOperands(std::set<Expression::Ptr>& memAccessors) const
    {
      if(m_Operands.empty())
      {
	decodeOperands();
      }
      for(std::list<Operand>::const_iterator curOperand = m_Operands.begin();
          curOperand != m_Operands.end();
	  ++curOperand)
      {
          curOperand->addEffectiveWriteAddresses(memAccessors);
      }  
      std::copy(m_InsnOp->getImplicitMemWrites().begin(), m_InsnOp->getImplicitMemWrites().end(), std::inserter(memAccessors,
memAccessors.begin()));
    }
    
    INSTRUCTION_EXPORT Expression::Ptr Instruction::getControlFlowTarget() const
    {
        // We assume control flow transfer instructions have the PC as
        // an implicit write, and that we have decoded the control flow
        // target's full location as the first and only operand.
        // If this is not the case, we'll squawk for the time being...
        if(getCategory() == c_NoCategory)
        {
            return Expression::Ptr();
        }
        if(getCategory() == c_ReturnInsn)
        {
            return makeReturnExpression();
        }
        if(m_Operands.empty())
        {
            decodeOperands();
        }
        if(m_Successors.empty())
        {
            return Expression::Ptr();
        }
        return m_Successors.front().target;
    }
    
    INSTRUCTION_EXPORT std::string Instruction::format(Address addr) const
    {
      if(m_Operands.empty())
      {
	decodeOperands();
      }

      std::string retVal = m_InsnOp->format();
      retVal += " ";
      std::list<Operand>::const_iterator curOperand;
      for(curOperand = m_Operands.begin();
	  curOperand != m_Operands.end();
	  ++curOperand)
      {
          retVal += curOperand->format(getArch(), addr);
	retVal += ", ";
      }
      if(!m_Operands.empty())
      {
	// trim trailing ", "
	retVal.erase(retVal.size() - 2, retVal.size());
      }
#if defined(DEBUG_READ_WRITE)      
      std::set<RegisterAST::Ptr> tmp;
      getReadSet(tmp);
      cout << "Read set:" << endl;
      for(std::set<RegisterAST::Ptr>::iterator i = tmp.begin();
          i != tmp.end();
         ++i)
      {
          cout << (*i)->format() << " ";
      }
      cout << endl;
      tmp.clear();
      getWriteSet(tmp);
      cout << "Write set:" << endl;
      for(std::set<RegisterAST::Ptr>::iterator i = tmp.begin();
          i != tmp.end();
          ++i)
      {
          cout << (*i)->format() << " ";
      }
      cout << endl;
      std::set<Expression::Ptr> mem;
      getMemoryReadOperands(mem);
      cout << "Read mem:" << endl;
      for(std::set<Expression::Ptr>::iterator i = mem.begin();
          i != mem.end();
          ++i)
      {
          cout << (*i)->format() << " ";
      }
      cout << endl;
      mem.clear();
      getMemoryWriteOperands(mem);
      cout << "Write mem:" << endl;
      for(std::set<Expression::Ptr>::iterator i = mem.begin();
          i != mem.end();
          ++i)
      {
          cout << (*i)->format() << " ";
      }
      cout << endl;
#endif // defined(DEBUG_READ_WRITE)
      return retVal;
    }
    INSTRUCTION_EXPORT bool Instruction::allowsFallThrough() const
    {
      switch(m_InsnOp->getID())
      {
      case e_ret_far:
      case e_ret_near:
      case e_iret:
      case e_jmp:
      case e_hlt:
      case e_sysret:
      case e_sysexit:
      case e_call:
      case e_syscall:
	return false;
      default:
      {
	decodeOperands();
          for(cftConstIter targ = m_Successors.begin();
              targ != m_Successors.end();
              ++targ)
          {
	    if(targ->isFallthrough) return true;
          }
          return m_Successors.empty();
      }
      }
      
    }
    INSTRUCTION_EXPORT bool Instruction::isLegalInsn() const
    {
      return (m_InsnOp->getID() != e_No_Entry);
    }

    INSTRUCTION_EXPORT Architecture Instruction::getArch() const {
      return arch_decoded_from;
    }

    Expression::Ptr Instruction::makeReturnExpression() const
    {
        Expression::Ptr stackPtr = Expression::Ptr(new RegisterAST(MachRegister::getStackPointer(arch_decoded_from),
                0, MachRegister::getStackPointer(arch_decoded_from).size()));
        Expression::Ptr retLoc = Expression::Ptr(new Dereference(stackPtr, u32));
        return retLoc;
    }
    INSTRUCTION_EXPORT InsnCategory Instruction::getCategory() const
    {
       InsnCategory c = entryToCategory(m_InsnOp->getID());
       if(c == c_BranchInsn && (arch_decoded_from == Arch_ppc32 || arch_decoded_from == Arch_ppc64))
       {
          if(m_Operands.empty()) decodeOperands();
          for(cftConstIter cft = cft_begin();
              cft != cft_end();
              ++cft)
          {
             if(cft->isCall)
             {/*
                static RegisterAST* thePC = new RegisterAST(MachRegister::getPC(arch_decoded_from));
		long offset;
		cft->target->bind(thePC, Result(u32, 0));
		offset = cft->target->eval().convert<long>();
                if(offset != (int)(size()))*/
                return c_CallInsn;
             }
          }
          if(m_InsnOp->getID() == power_op_bclr)
          {
             return c_ReturnInsn;
          }
       }
      return c;
    }
    void Instruction::addSuccessor(Expression::Ptr e, 
				   bool isCall, 
				   bool isIndirect, 
				   bool isConditional, 
				   bool isFallthrough) const
    {
        CFT c(e, isCall, isIndirect, isConditional, isFallthrough);
        m_Successors.push_back(c);
        if (!isFallthrough) appendOperand(e, true, false);
    }
    void Instruction::appendOperand(Expression::Ptr e, bool isRead, bool isWritten) const
    {
        m_Operands.push_back(Operand(e, isRead, isWritten));
    }
  

  };
};

