
#include "../h/Register.h"
#include <vector>
#include <set>
#include <sstream>

using namespace std;
using namespace boost;


namespace Dyninst
{
  namespace InstructionAPI
  {
    RegisterAST::RegisterAST(int id) : Expression(IA32_register_names[IA32Regs(id)].regSize), registerID(id) 
    {
    }
    RegisterAST::~RegisterAST()
    {
    }
    void RegisterAST::getChildren(vector<InstructionAST::Ptr>& /*children*/) const
    {
      return;
    }
    void RegisterAST::getUses(set<InstructionAST::Ptr>& /*uses*/) const
    {
      //uses.insert(InstructionAST::Ptr(const_cast<RegisterAST*>(this)));
      return;
    }
    bool RegisterAST::isUsed(InstructionAST::Ptr findMe) const
    {
      return (*findMe == *this);
    }
    unsigned int RegisterAST::getID() const
    {
      return registerID;
    }
    
    std::string RegisterAST::format() const
    {
      std::stringstream retVal;
      RegTable::iterator foundName = IA32_register_names.find(IA32Regs(registerID));
      if(foundName != IA32_register_names.end())
      {
	retVal << (*foundName).second.regName;
      }
      else
      {
	retVal << "R" << registerID;
      }
      return retVal.str();
    }
    RegisterAST RegisterAST::makePC()
    {
      // Make this platform independent
      return RegisterAST(r_EIP);
    }
    
    bool RegisterAST::operator<(const RegisterAST& rhs) const
    {
      return registerID < rhs.registerID;
    }
    bool RegisterAST::isSameType(const InstructionAST& rhs) const
    {
      return dynamic_cast<const RegisterAST*>(&rhs) != NULL;
    }
    bool RegisterAST::isStrictEqual(const InstructionAST& rhs) const
    {
      const RegisterAST& otherRegisterAST(dynamic_cast<const RegisterAST&>(rhs));
      return otherRegisterAST.registerID == registerID;
    }
  };
};
