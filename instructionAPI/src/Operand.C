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

#include "../h/Operand.h"
#include "../h/Dereference.h"
#include "../h/Register.h"
#include "../h/Expression.h"
#include <boost/shared_ptr.hpp>
#include <iostream>

using namespace std;


namespace Dyninst
{
  namespace InstructionAPI
  {
    void Operand::getReadSet(std::set<RegisterAST::Ptr>& regsRead) const
    {
      RegisterAST::Ptr op_as_reg = boost::dynamic_pointer_cast<RegisterAST>(op_value);
      if(m_isRead && (op_as_reg == NULL))
      {
	std::set<InstructionAST::Ptr> useSet;
	op_value->getUses(useSet);
	std::set<InstructionAST::Ptr>::const_iterator curUse;
	for(curUse = useSet.begin(); curUse != useSet.end(); ++curUse)
	{
	  boost::shared_ptr<RegisterAST> tmp = boost::dynamic_pointer_cast<RegisterAST>(*curUse);
	  if(tmp) 
	  {
	    regsRead.insert(tmp);
	  }
	}
      }
      else if(m_isRead)
      {
	regsRead.insert(op_as_reg);
      }
      
    }
    void Operand::getWriteSet(std::set<RegisterAST::Ptr>& regsWritten) const
    {
      RegisterAST::Ptr op_as_reg = boost::dynamic_pointer_cast<RegisterAST>(op_value);
      if(m_isWritten && op_as_reg)
      {
	regsWritten.insert(op_as_reg);
      }
    }

    bool Operand::isRead(Expression::Ptr candidate) const
    {
      // The whole expression of a read, any subexpression of a write
      return op_value->isUsed(candidate) && (m_isRead || !(*candidate == *op_value));
    }
    bool Operand::isWritten(Expression::Ptr candidate) const
    {
      // Whole expression of a write
      return m_isWritten && (*op_value == *candidate);
    }    
    bool Operand::readsMemory() const
    {
      return (boost::dynamic_pointer_cast<Dereference::Ptr>(op_value) && m_isRead);
    }
    bool Operand::writesMemory() const
    {
      return (boost::dynamic_pointer_cast<Dereference::Ptr>(op_value) && m_isWritten);
    }
    void Operand::addEffectiveReadAddresses(std::set<Expression::Ptr>& memAccessors) const
    {
      if(m_isRead && boost::dynamic_pointer_cast<Dereference>(op_value))
      {
	std::vector<InstructionAST::Ptr> tmp;
	op_value->getChildren(tmp);
	for(std::vector<InstructionAST::Ptr>::const_iterator curKid = tmp.begin();
	    curKid != tmp.end();
	    ++curKid)
	{
	  memAccessors.insert(boost::dynamic_pointer_cast<Expression>(*curKid));
	}
      }
    }
    void Operand::addEffectiveWriteAddresses(std::set<Expression::Ptr>& memAccessors) const
    {
      if(m_isWritten && boost::dynamic_pointer_cast<Dereference>(op_value))
      {
	std::vector<InstructionAST::Ptr> tmp;
	op_value->getChildren(tmp);
	for(std::vector<InstructionAST::Ptr>::const_iterator curKid = tmp.begin();
	    curKid != tmp.end();
	    ++curKid)
	{
	  memAccessors.insert(boost::dynamic_pointer_cast<Expression>(*curKid));
	}
      }
    }
    std::string Operand::format() const
    {
      if(!op_value) return "ERROR: format() called on empty operand!";
      return op_value->format();
    }
    Expression::Ptr Operand::getValue() const
    {
      return op_value;
    }
  };
};

