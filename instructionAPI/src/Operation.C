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

#include "Operation.h"
#include "arch-x86.h"
#include "entryIDs-IA32.h"
#include "../../common/h/Singleton.h"
#include "Register.h"
#include <map>
#include <boost/assign/list_of.hpp>
#include "../../common/h/singleton_object_pool.h"

using namespace boost::assign;

namespace Dyninst
{
  namespace InstructionAPI
  {
    RegisterAST::Ptr makeRegFromID(IA32Regs regID)
    {
      return make_shared(singleton_object_pool<RegisterAST>::construct(regID));
    }

    Operation::Operation(ia32_entry* e, ia32_prefixes* p, ia32_locations* l) :
      doneOtherSetup(false), doneFlagsSetup(false)
    
    {
      operationID = e->getID(l);
      if(p && p->getCount())
      {
	if (p->getPrefix(0) == PREFIX_REP || p->getPrefix(0) == PREFIX_REPNZ)
	{
	    otherRead.insert(makeRegFromID(r_DF));
	}
        int segPrefix = p->getPrefix(1);
        switch(segPrefix)
        {
            case PREFIX_SEGCS:
                otherRead.insert(makeRegFromID(r_CS));
                break;
            case PREFIX_SEGDS:
                otherRead.insert(makeRegFromID(r_DS));
                break;
            case PREFIX_SEGES:
                otherRead.insert(makeRegFromID(r_ES));
                break;
            case PREFIX_SEGFS:
                otherRead.insert(makeRegFromID(r_FS));
                break;
            case PREFIX_SEGGS:
                otherRead.insert(makeRegFromID(r_GS));
                break;
            case PREFIX_SEGSS:
                otherRead.insert(makeRegFromID(r_SS));
                break;
        }
      }
    }

    Operation::Operation(const Operation& o)
    {
      otherRead = o.otherRead;
      otherWritten = o.otherWritten;
      otherEffAddrsRead = o.otherEffAddrsRead;
      otherEffAddrsWritten = o.otherEffAddrsWritten;
      operationID = o.operationID;
      doneOtherSetup = o.doneOtherSetup;
      doneFlagsSetup = o.doneFlagsSetup;
      
    }
    const Operation& Operation::operator=(const Operation& o)
    {
      otherRead = o.otherRead;
      otherWritten = o.otherWritten;
      otherEffAddrsRead = o.otherEffAddrsRead;
      otherEffAddrsWritten = o.otherEffAddrsWritten;
      operationID = o.operationID;
      doneOtherSetup = o.doneOtherSetup;
      doneFlagsSetup = o.doneFlagsSetup;
      return *this;
    }
    Operation::Operation()
    {
      operationID = e_No_Entry;
    }
    
    const Operation::registerSet&  Operation::implicitReads() const
    {
      if(!doneOtherSetup) SetUpNonOperandData(true);
      
      return otherRead;
    }
    const Operation::registerSet&  Operation::implicitWrites() const
    {
      if(!doneOtherSetup) SetUpNonOperandData(true);

      return otherWritten;
    }
    bool Operation::isRead(Expression::Ptr candidate) const
    {
      if(!doneOtherSetup)
      {
	SetUpNonOperandData(candidate->isFlag());
      }
      for(registerSet::const_iterator r = otherRead.begin();
	  r != otherRead.end();
	  ++r)
      {
	if(*candidate == *(*r))
	{
	  return true;
	}
      }
      for(VCSet::const_iterator e = otherEffAddrsRead.begin();
	  e != otherEffAddrsRead.end();
	  ++e)
      {
	if(*candidate == *(*e))
	{
	  return true;
	}
      }
      return false;
    }
    const Operation::VCSet& Operation::getImplicitMemReads() const
    {
      if(!doneOtherSetup) SetUpNonOperandData(true);
      return otherEffAddrsRead;
    }
    const Operation::VCSet& Operation::getImplicitMemWrites() const
    {
      if(!doneOtherSetup) SetUpNonOperandData(true);
      return otherEffAddrsWritten;
    }

    bool Operation::isWritten(Expression::Ptr candidate) const
    {
      if(!doneOtherSetup)
      {
	SetUpNonOperandData(candidate->isFlag());
      }
      for(registerSet::const_iterator r = otherWritten.begin();
	  r != otherWritten.end();
	  ++r)
      {
	if(*candidate == *(*r))
	{
	  return true;
	}
      }
      for(VCSet::const_iterator e = otherEffAddrsWritten.begin();
	  e != otherEffAddrsWritten.end();
	  ++e)
      {
	if(*candidate == *(*e))
	{
	  return true;
	}
      }
      return false;
    }
	  
    std::string Operation::format() const
    {
      dyn_hash_map<entryID, std::string>::const_iterator found = entryNames_IAPI.find(operationID);
      if(found != entryNames_IAPI.end())
	return found->second;
      return "[INVALID]";
    }

    entryID Operation::getID() const
    {
      return operationID;
    }

    struct OperationMaps
    {
    public:
      OperationMaps()
      {
        thePC.insert(RegisterAST::Ptr(new RegisterAST(RegisterAST::makePC())));
	pcAndSP.insert(RegisterAST::Ptr(new RegisterAST(RegisterAST::makePC())));
	pcAndSP.insert(RegisterAST::Ptr(new RegisterAST(r_eSP)));
	
	stackPointer.insert(RegisterAST::Ptr(new RegisterAST(r_eSP)));
	stackPointerAsExpr.insert(Expression::Ptr(new RegisterAST(r_eSP)));
	framePointer.insert(RegisterAST::Ptr(new RegisterAST(r_eBP)));
	spAndBP.insert(RegisterAST::Ptr(new RegisterAST(r_eSP)));
	spAndBP.insert(RegisterAST::Ptr(new RegisterAST(r_eBP)));
	
	nonOperandRegisterReads = 
	map_list_of
        (e_call, pcAndSP)
	(e_ret_near, stackPointer)
	(e_ret_far, stackPointer)
	(e_leave, framePointer)
	(e_enter, spAndBP);
	
	nonOperandRegisterWrites = 
	map_list_of
	(e_call, pcAndSP)
	(e_ret_near, pcAndSP)
	(e_ret_far, pcAndSP)
	(e_leave, spAndBP)
	(e_enter, spAndBP)
	(e_loop, thePC)
	(e_loope, thePC)
	(e_loopn, thePC)
	(e_jb, thePC)
	(e_jb_jnaej_j, thePC)
	(e_jbe, thePC)
	(e_jcxz_jec, thePC)
	(e_jl, thePC)
	(e_jle, thePC)
	(e_jmp, thePC)
	(e_jnb, thePC)
	(e_jnb_jae_j, thePC)
	(e_jnbe, thePC)
	(e_jnl, thePC)
	(e_jnle, thePC)
	(e_jno, thePC)
	(e_jnp, thePC)
	(e_jns, thePC)
	(e_jnz, thePC)
	(e_jo, thePC)
	(e_jp, thePC)
	(e_js, thePC)
	(e_jz, thePC);
	nonOperandMemoryReads[e_pop] = stackPointerAsExpr;
	nonOperandMemoryWrites[e_push] = stackPointerAsExpr;
        nonOperandMemoryWrites[e_call] = stackPointerAsExpr;
        nonOperandMemoryReads[e_ret_near] = stackPointerAsExpr;
        nonOperandMemoryReads[e_ret_far] = stackPointerAsExpr;
	nonOperandMemoryReads[e_leave] = stackPointerAsExpr;
      }
      Operation::registerSet thePC;
      Operation::registerSet pcAndSP;
      Operation::registerSet stackPointer;
      Operation::VCSet stackPointerAsExpr;
      Operation::registerSet framePointer;
      Operation::registerSet spAndBP;
      dyn_hash_map<entryID, Operation::registerSet > nonOperandRegisterReads;
      dyn_hash_map<entryID, Operation::registerSet > nonOperandRegisterWrites;

      dyn_hash_map<entryID, Operation::VCSet > nonOperandMemoryReads;
      dyn_hash_map<entryID, Operation::VCSet > nonOperandMemoryWrites;
    };
    OperationMaps op_data;
    void Operation::SetUpNonOperandData(bool needFlags) const
    {
      
      dyn_hash_map<entryID, registerSet >::const_iterator foundRegs;
      foundRegs = op_data.nonOperandRegisterReads.find(operationID);
      if(foundRegs != op_data.nonOperandRegisterReads.end())
      {
	otherRead = foundRegs->second;
	//std::copy(foundRegs->second.begin(), foundRegs->second.end(),
	//	  inserter(otherRead, otherRead.begin()));
      }
      foundRegs = op_data.nonOperandRegisterWrites.find(operationID);
      if(foundRegs != op_data.nonOperandRegisterWrites.end())
      {
	otherWritten = foundRegs->second;
	//std::copy(foundRegs->second.begin(), foundRegs->second.end(),
	//	  inserter(otherWritten, otherWritten.begin()));
      }
      dyn_hash_map<entryID, VCSet >::const_iterator foundMem;
      foundMem = op_data.nonOperandMemoryReads.find(operationID);
      if(foundMem != op_data.nonOperandMemoryReads.end())
      {
	otherEffAddrsRead = foundMem->second;
	//std::copy(foundMem->second.begin(), foundMem->second.end(),
	//	  inserter(otherEffAddrsRead, otherEffAddrsRead.begin()));
      }
      foundMem = op_data.nonOperandMemoryWrites.find(operationID);
      if(foundMem != op_data.nonOperandMemoryWrites.end())
      {
	otherEffAddrsWritten = foundMem->second;
	//std::copy(foundMem->second.begin(), foundMem->second.end(),
	//	  inserter(otherEffAddrsWritten, otherEffAddrsWritten.begin()));
      }
      
      if(needFlags && !doneFlagsSetup)
      {
	
	dyn_hash_map<entryID, flagInfo>::const_iterator found = ia32_instruction::getFlagTable().find(operationID);
	if(found != ia32_instruction::getFlagTable().end())
	{
	  for(unsigned i = 0; i < found->second.readFlags.size(); i++)
	  {
	    otherRead.insert(makeRegFromID(found->second.readFlags[i]));
	  }
	  for(unsigned j = 0; j < found->second.writtenFlags.size(); j++)
	  {
	    otherWritten.insert(makeRegFromID(found->second.writtenFlags[j]));
	  }
	}
	doneFlagsSetup = true;
      }
      doneOtherSetup = true;
    }
  };

};
