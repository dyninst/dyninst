
#include "frameChecker.h"
#include "instructionAPI/h/InstructionDecoder.h"

using namespace Dyninst::InstructionAPI;


frameChecker::frameChecker(const unsigned char* addr, size_t max_length)
{
  // How many instructions in our stack frame idioms?
  static const unsigned max_insns = 3;
  
  InstructionDecoder d;
  unsigned bytesDecoded = 0;
  
  for(unsigned i = 0; i < max_insns && bytesDecoded < max_length; i++)
  {
    m_Insns.push_back(d.decode(addr, max_length - bytesDecoded));
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
   if(m_Insns.size() < index_to_check) return false;
  if(m_Insns[index_to_check].getOperation().getID() == e_mov)
  {
    RegisterAST::Ptr stack_ptr(new RegisterAST(r_ESP));
    RegisterAST::Ptr base_ptr(new RegisterAST(r_EBP));
    std::string debugMe = m_Insns[index_to_check].format();
    
    if(m_Insns[index_to_check].isWritten(base_ptr) && m_Insns[index_to_check].isRead(stack_ptr))
    {
      return true;
    }
    stack_ptr = RegisterAST::Ptr(new RegisterAST(r_RSP));
    base_ptr = RegisterAST::Ptr(new RegisterAST(r_RBP));
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
