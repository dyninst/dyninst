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

#include <string>
#include "../h/Instruction.h"
#include "../h/Register.h"
#include "../h/Operation.h"
#include <boost/iterator/indirect_iterator.hpp>
#include <iostream>
using namespace std;

namespace Dyninst
{
  namespace InstructionAPI
  {
    Instruction::Instruction(const Operation& what, 
			     const std::vector<Expression::Ptr>& operandSource,
			     unsigned char size, const unsigned char* raw)
      : m_InsnOp(what), m_Valid(true)
    {
      unsigned int i;
      std::vector<Expression::Ptr>::const_iterator curVC;
      
      for(i = 0, curVC = operandSource.begin();
	  curVC != operandSource.end() && i < what.read().size() && i < what.written().size();
	  ++curVC, ++i)
      {
	Operand tmp(*curVC, what.read()[i], what.written()[i]);
	m_Operands.push_back(tmp);
      }
      if(raw)
      {
      	for(int i = 0; i < size; i++)
	{
	  m_RawInsn.push_back(raw[i]);
	}
      }
    }
    Instruction::Instruction() :
      m_Valid(false)
    {
    }
    
    Instruction::~Instruction()
    {
    }

    Instruction::Instruction(const Instruction& o)
    {
      m_Operands.clear();
      m_RawInsn.clear();
      std::copy(o.m_Operands.begin(), o.m_Operands.end(), std::back_inserter(m_Operands));
      std::copy(o.m_RawInsn.begin(), o.m_RawInsn.end(), std::back_inserter(m_RawInsn));
      m_InsnOp = o.m_InsnOp;
      m_Valid = o.m_Valid;
    }

    const Instruction& Instruction::operator=(const Instruction& rhs)
    {
      m_Operands.clear();
      m_RawInsn.clear();
      std::copy(rhs.m_Operands.begin(), rhs.m_Operands.end(), std::back_inserter(m_Operands));
      std::copy(rhs.m_RawInsn.begin(), rhs.m_RawInsn.end(), std::back_inserter(m_RawInsn));
      m_InsnOp = rhs.m_InsnOp;
      m_Valid = rhs.m_Valid;
      return *this;
    }    
    
    bool Instruction::isValid() const
    {
      return m_Valid;
    }
    
    const Operation& Instruction::getOperation() const
    {
      return m_InsnOp;
    }
    
    void Instruction::getOperands(std::vector<Operand>& operands) const
    {
      std::copy(m_Operands.begin(), m_Operands.end(), std::back_inserter(operands));
    }
    
     Operand Instruction::getOperand(int index) const
     {
        if(index < 0 || index >= (int)(m_Operands.size()))
        {
	  // Out of range = empty operand
           return Operand(Expression::Ptr(), false, false);
        }
        return m_Operands[index];
     }
    
    unsigned char Instruction::size() const
    {
      return m_RawInsn.size();
    }
    
    void Instruction::getReadSet(std::set<RegisterAST::Ptr>& regsRead) const
    {
      for(std::vector<Operand>::const_iterator curOperand = m_Operands.begin();
	  curOperand != m_Operands.end();
	  ++curOperand)
      {
	curOperand->getReadSet(regsRead);
      }
      std::set<RegisterAST::Ptr> implicitReads = m_InsnOp.implicitReads();
      std::copy(implicitReads.begin(), implicitReads.end(), std::inserter(regsRead, regsRead.begin()));
      
    }
    
    void Instruction::getWriteSet(std::set<RegisterAST::Ptr>& regsWritten) const
    {
      for(std::vector<Operand>::const_iterator curOperand = m_Operands.begin();
	  curOperand != m_Operands.end();
	  ++curOperand)
      {
	curOperand->getWriteSet(regsWritten);
      }
      std::set<RegisterAST::Ptr> implicitWrites = m_InsnOp.implicitWrites();
      std::copy(implicitWrites.begin(), implicitWrites.end(), std::inserter(regsWritten, regsWritten.begin()));
      
    }
    
    bool Instruction::isRead(Expression::Ptr candidate) const
    {
      for(std::vector<Operand>::const_iterator curOperand = m_Operands.begin();
	  curOperand != m_Operands.end();
	  ++curOperand)
      {
	if(curOperand->isRead(candidate))
	{
	  return true;
	}
      }
      return false;
    }

    bool Instruction::isWritten(Expression::Ptr candidate) const
    {
      for(std::vector<Operand>::const_iterator curOperand = m_Operands.begin();
	  curOperand != m_Operands.end();
	  ++curOperand)
      {
	if(curOperand->isWritten(candidate))
	{
	  return true;
	}
      }
      return false;
    }
    
    bool Instruction::readsMemory() const
    {
      for(std::vector<Operand>::const_iterator curOperand = m_Operands.begin();
	  curOperand != m_Operands.end();
	  ++curOperand)
      {
	if(curOperand->readsMemory());
	{
	  return true;
	}
      }
      return false;
    }
    
    bool Instruction::writesMemory() const
    {
      for(std::vector<Operand>::const_iterator curOperand = m_Operands.begin();
	  curOperand != m_Operands.end();
	  ++curOperand)
      {
	if(curOperand->writesMemory());
	{
	  return true;
	}
      }
      return false;
    }
    
    void Instruction::getMemoryReadOperands(std::set<Expression::Ptr>& memAccessors) const
    {
      for(std::vector<Operand>::const_iterator curOperand = m_Operands.begin();
	  curOperand != m_Operands.end();
	  ++curOperand)
      {
	curOperand->addEffectiveReadAddresses(memAccessors);
      }  
    }
    
    void Instruction::getMemoryWriteOperands(std::set<Expression::Ptr>& memAccessors) const
    {
      for(std::vector<Operand>::const_iterator curOperand = m_Operands.begin();
	  curOperand != m_Operands.end();
	  ++curOperand)
      {
	curOperand->addEffectiveWriteAddresses(memAccessors);
      }  
    }
    
    Expression::Ptr Instruction::getControlFlowTarget() const
    {
      // We assume control flow transfer instructions have the PC as
      // an implicit write, and that we have decoded the control flow
      // target's full location as the first and only operand.
      // If this is not the case, we'll squawk for the time being...
      
      if(std::find(boost::make_indirect_iterator(m_InsnOp.implicitWrites().begin()),
		   boost::make_indirect_iterator(m_InsnOp.implicitWrites().end()),
		   RegisterAST::makePC())
	 == boost::make_indirect_iterator(m_InsnOp.implicitWrites().end()))
      {
	return Expression::Ptr();
      }
      assert(m_Operands.size() == 1);
      return m_Operands[0].getValue();
    }
    
    std::string Instruction::format() const
    {
      std::string retVal = m_InsnOp.format();
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
  };
};

