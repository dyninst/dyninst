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

#include "Register.h"
#include "../../common/h/Singleton.h"
#include <vector>
#include <set>
#include <sstream>

using namespace std;
using namespace boost;


namespace Dyninst
{
  namespace InstructionAPI
  {
	RegisterAST::RegisterAST(int id) : 
		Expression(Singleton<IA32RegTable>::getInstance().IA32_register_names[IA32Regs(id)].regSize), registerID(id) 
    {
    }
    RegisterAST::~RegisterAST()
    {
    }
    void RegisterAST::getChildren(vector<InstructionAST::Ptr>& /*children*/) const
    {
      return;
    }
    void RegisterAST::getUses(set<InstructionAST::Ptr>& uses) const
    {
		if(registerID == r_ALLGPRS)
		{
			for(std::set<InstructionAST::Ptr>::iterator cur = uses.begin();
				cur != uses.end();
				++cur)
			{
				if(**cur == *this)
				{
					uses.erase(cur);
					break;
				}
			}
			uses.insert(InstructionAST::Ptr(new RegisterAST(r_EAX)));
			uses.insert(InstructionAST::Ptr(new RegisterAST(r_ECX)));
			uses.insert(InstructionAST::Ptr(new RegisterAST(r_EDX)));
			uses.insert(InstructionAST::Ptr(new RegisterAST(r_EBX)));
			uses.insert(InstructionAST::Ptr(new RegisterAST(r_ESP)));
			uses.insert(InstructionAST::Ptr(new RegisterAST(r_EBP)));
			uses.insert(InstructionAST::Ptr(new RegisterAST(r_ESI)));
			uses.insert(InstructionAST::Ptr(new RegisterAST(r_EDI)));
		}
      return;
    }
    bool RegisterAST::isUsed(InstructionAST::Ptr findMe) const
    {
      if(*findMe == *this)
	  {
		  return true;
	  }
	  if(registerID == r_ALLGPRS)
	  {
		  RegisterAST::Ptr asReg = boost::dynamic_pointer_cast<RegisterAST>(findMe);
		  if(!asReg) return false;
		  if(asReg->registerID == r_EAX ||
			asReg->registerID == r_EDX ||
			asReg->registerID == r_ECX ||
			asReg->registerID == r_EBX ||
			asReg->registerID == r_ESP ||
			asReg->registerID == r_EBP ||
			asReg->registerID == r_ESI ||
			asReg->registerID == r_EDI)
		  {
			  return true;
		  }
	  }
	  return false;
    }
    unsigned int RegisterAST::getID() const
    {
      return registerID;
    }
    
    std::string RegisterAST::format() const
    {
      std::stringstream retVal;
	  RegTable::iterator foundName = Singleton<IA32RegTable>::getInstance().IA32_register_names.find(IA32Regs(registerID));
      if(foundName != Singleton<IA32RegTable>::getInstance().IA32_register_names.end())
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
