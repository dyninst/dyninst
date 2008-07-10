
#include <string>
#include "../h/Instruction.h"
#include "../h/Register.h"
#include "../h/Operation.h"

#include <iostream>
using namespace std;

namespace Dyninst
{
  namespace Instruction
  {
    Instruction::Instruction(const Operation& what, 
			     const std::vector<Expression::Ptr>& operandSource,
			     unsigned char size)
      : m_InsnOp(what), m_Size(size)
    {
      unsigned int i;
      std::vector<Expression::Ptr>::const_iterator curVC;
      
      for(i = 0, curVC = operandSource.begin();
	  curVC != operandSource.end() && i < what.read().size() && i < what.written().size();
	  ++curVC, ++i)
      {
	m_Operands.push_back(Operand(*curVC, what.read()[i], what.written()[i]));
      }
      
    }
    
    Instruction::~Instruction()
    {
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
        if(index < 0 || index >= m_Operands.size())
        {
	  // Out of range = empty operand
           return Operand(Expression::Ptr(), false, false);
        }
        return m_Operands[index];
     }
    
    unsigned char Instruction::size() const
    {
      return m_Size;
    }
    
    void Instruction::getReadSet(std::set<RegisterAST::Ptr>& regsRead) const
    {
      for(std::vector<Operand>::const_iterator curOperand = m_Operands.begin();
	  curOperand != m_Operands.end();
	  ++curOperand)
      {
	curOperand->getReadSet(regsRead);
      }
    }
    
    void Instruction::getWriteSet(std::set<RegisterAST::Ptr>& regsWritten) const
    {
      for(std::vector<Operand>::const_iterator curOperand = m_Operands.begin();
	  curOperand != m_Operands.end();
	  ++curOperand)
      {
	curOperand->getWriteSet(regsWritten);
      }
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
      std::set<RegisterAST>::const_iterator foundPC = m_InsnOp.implicitWrites().find(RegisterAST::makePC());
      if(foundPC == m_InsnOp.implicitWrites().end())
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

