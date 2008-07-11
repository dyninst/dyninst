
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

