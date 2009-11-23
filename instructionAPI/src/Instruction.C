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
#include "arch-x86.h"
using namespace std;

#include <boost/pool/pool.hpp>

namespace Dyninst
{
  namespace InstructionAPI
  {

    bool readsOperand(unsigned int opsema, unsigned int i)
    {
      switch(opsema) {
      case s1R2R:
	return (i == 0 || i == 1);
      case s1R:
      case s1RW:
	return i == 0;
      case s1W:
	return false;
      case s1W2RW:
      case s1W2R:   // second operand read, first operand written (e.g. mov)
	return i == 1;
      case s1RW2R:  // two operands read, first written (e.g. add)
      case s1RW2RW: // e.g. xchg
      case s1R2RW:
	return i == 0 || i == 1;
      case s1W2R3R: // e.g. imul
      case s1W2RW3R: // some mul
      case s1W2R3RW: // (stack) push & pop
	return i == 1 || i == 2;
      case s1W2W3R: // e.g. les
	return i == 2;
      case s1RW2R3R: // shld/shrd
      case s1RW2RW3R: // [i]div, cmpxch8b
	return i == 0 || i == 1 || i == 2;
	break;
      case sNONE:
      default:
	return false;
      }
      
    }
      
    bool writesOperand(unsigned int opsema, unsigned int i)
    {
      switch(opsema) {
      case s1R2R:
      case s1R:
	return false;
      case s1RW:
      case s1W:
      case s1W2R:   // second operand read, first operand written (e.g. mov)
      case s1RW2R:  // two operands read, first written (e.g. add)
      case s1W2R3R: // e.g. imul
      case s1RW2R3R: // shld/shrd
	return i == 0;
      case s1R2RW:
	return i == 1;
      case s1W2RW:
      case s1RW2RW: // e.g. xchg
      case s1W2RW3R: // some mul
      case s1W2W3R: // e.g. les
      case s1RW2RW3R: // [i]div, cmpxch8b
	return i == 0 || i == 1;
      case s1W2R3RW: // (stack) push & pop
	return i == 0 || i == 2;
      case sNONE:
      default:
	return false;
      }
    }    

    INSTRUCTION_EXPORT Instruction::Instruction(Operation::Ptr what, 
			     size_t size, const unsigned char* raw)
      : m_InsnOp(what), m_Valid(true)
    {
      copyRaw(size, raw);
      
    }

    void Instruction::copyRaw(size_t size, const unsigned char* raw)
    {
      
      if(raw)
      {
	m_size = size;
	m_RawInsn.small_insn = 0;
	if(size <= sizeof(unsigned int))
	{
	  memcpy(&m_RawInsn, raw, size);
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
      static InstructionDecoder d;
      const unsigned char* buffer = reinterpret_cast<const unsigned char*>(&(m_RawInsn.small_insn));
      if(m_size > sizeof(unsigned int)) {
	buffer = m_RawInsn.large_insn;
      }
      std::vector<Expression::Ptr> opSrc;
      d.resetBuffer(buffer, size());
      d.doIA32Decode();
      d.decodeOperands(opSrc);
      m_Operands.reserve(opSrc.size());
      unsigned int opsema = d.decodedInstruction->getEntry()->opsema & 0xFF;
      
      for(unsigned int i = 0;
	  i < opSrc.size();
	  ++i)
      {
	m_Operands.push_back(Operand(opSrc[i], readsOperand(opsema, i), writesOperand(opsema, i)));
      }
    }
    
    INSTRUCTION_EXPORT Instruction::Instruction(Operation::Ptr what, 
			     const std::vector<Expression::Ptr>& operandSource,
			     size_t size, const unsigned char* raw, unsigned int opsema)
      : m_InsnOp(what), m_Valid(true)
    {
      std::vector<Expression::Ptr>::const_iterator curVC;
      m_Operands.reserve(operandSource.size());
      
      for(unsigned int i = 0;
	  i < operandSource.size();
	  ++i)
      {
	m_Operands.push_back(Operand(operandSource[i], readsOperand(opsema, i), writesOperand(opsema, i)));
      }
      copyRaw(size, raw);
      
    }
    INSTRUCTION_EXPORT Instruction::Instruction() :
      m_Valid(false), m_size(0)
    {
    }
    
    INSTRUCTION_EXPORT Instruction::~Instruction()
    {
      if(m_size > sizeof(unsigned int))
      {
	delete[] m_RawInsn.large_insn;
      }
      
    }

    INSTRUCTION_EXPORT Instruction::Instruction(const Instruction& o)
    {
      m_Operands.clear();
      //m_Operands.reserve(o.m_Operands.size());
      //std::copy(o.m_Operands.begin(), o.m_Operands.end(), std::back_inserter(m_Operands));
      if(m_size > sizeof(unsigned int)) 
      {
	delete[] m_RawInsn.large_insn;
      }
      
      m_size = o.m_size;
      if(o.m_size > sizeof(unsigned int))
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
    }

    INSTRUCTION_EXPORT const Instruction& Instruction::operator=(const Instruction& rhs)
    {
      m_Operands.clear();
      //m_Operands.reserve(rhs.m_Operands.size());
      //std::copy(rhs.m_Operands.begin(), rhs.m_Operands.end(), std::back_inserter(m_Operands));
      if(m_size > sizeof(unsigned int)) 
      {
	delete[] m_RawInsn.large_insn;
      }
      
      m_size = rhs.m_size;
      if(rhs.m_size > sizeof(unsigned int))
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
        return m_Operands[index];
     }

     INSTRUCTION_EXPORT const void* Instruction::ptr() const
     {
         if(m_size > sizeof(unsigned int))
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
      if(index > m_size) return 0;
      if(m_size > sizeof(unsigned int)) 
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
      for(std::vector<Operand>::const_iterator curOperand = m_Operands.begin();
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
      for(std::vector<Operand>::const_iterator curOperand = m_Operands.begin();
	  curOperand != m_Operands.end();
	  ++curOperand)
      {
	curOperand->getWriteSet(regsWritten);
      }
      std::copy(m_InsnOp->implicitWrites().begin(), m_InsnOp->implicitWrites().end(), std::inserter(regsWritten, regsWritten.begin()));
      
    }
    
    INSTRUCTION_EXPORT bool Instruction::isRead(Expression::Ptr candidate) const
    {
      if(m_Operands.empty())
      {
	decodeOperands();
      }
      for(std::vector<Operand>::const_iterator curOperand = m_Operands.begin();
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
      for(std::vector<Operand>::const_iterator curOperand = m_Operands.begin();
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
      for(std::vector<Operand>::const_iterator curOperand = m_Operands.begin();
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
      for(std::vector<Operand>::const_iterator curOperand = m_Operands.begin();
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
      for(std::vector<Operand>::const_iterator curOperand = m_Operands.begin();
	  curOperand != m_Operands.end();
	  ++curOperand)
      {
	curOperand->addEffectiveReadAddresses(memAccessors);
      }  
      std::copy(m_InsnOp->getImplicitMemReads().begin(), m_InsnOp->getImplicitMemReads().end(), std::inserter(memAccessors, memAccessors.begin()));
    }
    
    INSTRUCTION_EXPORT void Instruction::getMemoryWriteOperands(std::set<Expression::Ptr>& memAccessors) const
    {
      if(m_Operands.empty())
      {
	decodeOperands();
      }
      for(std::vector<Operand>::const_iterator curOperand = m_Operands.begin();
	  curOperand != m_Operands.end();
	  ++curOperand)
      {
	curOperand->addEffectiveWriteAddresses(memAccessors);
      }  
      std::copy(m_InsnOp->getImplicitMemWrites().begin(), m_InsnOp->getImplicitMemWrites().end(), std::inserter(memAccessors, memAccessors.begin()));
    }
    
    INSTRUCTION_EXPORT Expression::Ptr Instruction::getControlFlowTarget() const
    {
        // We assume control flow transfer instructions have the PC as
        // an implicit write, and that we have decoded the control flow
        // target's full location as the first and only operand.
        // If this is not the case, we'll squawk for the time being...
        static Expression::Ptr thePC(new RegisterAST(RegisterAST::makePC()));
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
    #if defined(DEBUG_LOG)      
        if(!(m_Operands[0].isRead(thePC) || m_Operands.size() == 1))
        {
            fprintf(stderr, "WARNING: control flow target for instruction %s may be incorrect\n", format().c_str());
        }
    #endif      
        if(getCategory() == c_BranchInsn ||
        getCategory() == c_CallInsn)
        {
#if defined(NO_OPT_FLAG)            
            assert(m_InsnOp->isWritten(thePC));
#endif //defined(NO_OPT_FLAG)
            return m_Operands[0].getValue();
        }
        return Expression::Ptr();
    }
    
    INSTRUCTION_EXPORT std::string Instruction::format() const
    {
      if(m_Operands.empty())
      {
	decodeOperands();
      }
      std::string retVal = m_InsnOp->format();
      retVal += " ";
      std::vector<Operand>::const_iterator curOperand;
      for(curOperand = m_Operands.begin();
	  curOperand != m_Operands.end();
	  ++curOperand)
      {
	retVal += curOperand->format();
	retVal += ", ";
      }
      if(!m_Operands.empty())
      {
	// trim trailing ", "
	retVal.erase(retVal.size() - 2, retVal.size());
      }
      
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
	return false;
      default:
	return true;
      }
      
    }
    INSTRUCTION_EXPORT bool Instruction::isLegalInsn() const
    {
      return (m_InsnOp->getID() != e_No_Entry);
    }
    Expression::Ptr Instruction::makeReturnExpression() const
    {
      Expression::Ptr stackPtr = Expression::Ptr(new RegisterAST(r_rSP));
      Expression::Ptr retLoc = Expression::Ptr(new Dereference(stackPtr, u32));
      return retLoc;
    }
    INSTRUCTION_EXPORT InsnCategory Instruction::getCategory() const
    {
      return entryToCategory(m_InsnOp->getID());
    }
  };
};

