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

#include "Operation.h"
#include "arch-x86.h"
#include "entryIDs-IA32.h"

namespace Dyninst
{
  namespace InstructionAPI
  {
    RegisterAST::Ptr makeRegFromID(IA32Regs regID)
    {
      return RegisterAST::Ptr(new RegisterAST(regID));
    }

    Operation::Operation(Dyninst::InstructionAPI::ia32_entry* e, Dyninst::InstructionAPI::ia32_prefixes* p)
    {
      if(!e || !e->name())
      {
	mnemonic = "[INVALID]";
	operationID = e_No_Entry;
	return;
      }
      mnemonic = e->name();
      operationID = e->id;
      switch(e->opsema & 0xff)
      {
      case s1R2R:   // reads two operands, e.g. cmp
	readOperands.push_back(true);
	readOperands.push_back(true);
	writtenOperands.push_back(false);
	writtenOperands.push_back(false);
	break;
      case s1R:
	readOperands.push_back(true);
	writtenOperands.push_back(false);
	break;
      case s1RW:    // one operand read and written, e.g. inc
	readOperands.push_back(true);
	writtenOperands.push_back(true);
	break;
      case s1W:
	readOperands.push_back(false);
	writtenOperands.push_back(true);
	break;
      case s1W2R:   // second operand read, first operand written (e.g. mov)
	readOperands.push_back(false);
	writtenOperands.push_back(true);
	readOperands.push_back(true);
	writtenOperands.push_back(false);
	break;
      case s1RW2R:  // two operands read, first written (e.g. add)
	readOperands.push_back(true);
	writtenOperands.push_back(true);
	readOperands.push_back(true);
	writtenOperands.push_back(false);
	break;
      case s1RW2RW: // e.g. xchg
	readOperands.push_back(true);
	writtenOperands.push_back(true);
	readOperands.push_back(true);
	writtenOperands.push_back(true);
	break;
      case s1W2R3R: // e.g. imul
	readOperands.push_back(false);
	writtenOperands.push_back(true);
	readOperands.push_back(true);
	writtenOperands.push_back(false);
	readOperands.push_back(true);
	writtenOperands.push_back(false);
	break;
      case s1W2W3R: // e.g. les
	readOperands.push_back(false);
	writtenOperands.push_back(true);
	readOperands.push_back(false);
	writtenOperands.push_back(true);
	readOperands.push_back(true);
	writtenOperands.push_back(false);
	break;
      case s1W2RW3R: // some mul
	readOperands.push_back(false);
	writtenOperands.push_back(true);
	readOperands.push_back(true);
	writtenOperands.push_back(true);
	readOperands.push_back(true);
	writtenOperands.push_back(false);
	break;
      case s1W2R3RW: // (stack) push & pop
	readOperands.push_back(false);
	writtenOperands.push_back(true);
	readOperands.push_back(true);
	writtenOperands.push_back(false);
	readOperands.push_back(true);
	writtenOperands.push_back(true);
	break;
      case s1RW2R3R: // shld/shrd
	readOperands.push_back(true);
	writtenOperands.push_back(true);
	readOperands.push_back(true);
	writtenOperands.push_back(false);
	readOperands.push_back(true);
	writtenOperands.push_back(false);
	break;
      case s1RW2RW3R: // [i]div, cmpxch8b
	readOperands.push_back(true);
	writtenOperands.push_back(true);
	readOperands.push_back(true);
	writtenOperands.push_back(true);
	readOperands.push_back(true);
	writtenOperands.push_back(false);
	break;
      case s1R2RW:
	readOperands.push_back(true);
	writtenOperands.push_back(false);
	readOperands.push_back(true);
	writtenOperands.push_back(true);
	break;
      case s1W2RW:
	readOperands.push_back(false);
	writtenOperands.push_back(true);
	readOperands.push_back(true);
	writtenOperands.push_back(true);
	break;
      case sNONE:
      default:
	break;
      }
      SetUpNonOperandData();
      std::set<IA32Regs> flagsRead, flagsWritten;
      e->flagsUsed(flagsRead, flagsWritten);
      if(p && p->getCount())
      {
	for(unsigned i = 0; i < p->getCount(); i++)
	{
	  if(p->getPrefix(i) == PREFIX_REP || p->getPrefix(i) == PREFIX_REPNZ)
	  {
	    flagsRead.insert(r_DF);
	    break;
	  }
	}
      }
      
      
      std::transform(flagsRead.begin(), flagsRead.end(), 
		     inserter(otherRead, otherRead.begin()), &makeRegFromID);
      std::transform(flagsWritten.begin(), flagsWritten.end(), 
		     inserter(otherWritten, otherWritten.begin()), &makeRegFromID);
      
    }
    
    void Operation::SetUpNonOperandData()
    {
      if(nonOperandRegisterReads.find(operationID) != nonOperandRegisterReads.end())
      {
	otherRead = nonOperandRegisterReads[operationID];
      }
      if(nonOperandRegisterWrites.find(operationID) != nonOperandRegisterWrites.end())
      {
	otherWritten = nonOperandRegisterWrites[operationID];
      }
      if(nonOperandMemoryReads.find(operationID) != nonOperandMemoryReads.end())
      {
	otherEffAddrsRead = nonOperandMemoryReads[operationID];
      }
      if(nonOperandMemoryWrites.find(operationID) != nonOperandMemoryWrites.end())
      {
	otherEffAddrsWritten = nonOperandMemoryWrites[operationID];
      }      
    }
    
    Operation::Operation(const Operation& o)
    {
      readOperands = o.readOperands;
      writtenOperands = o.writtenOperands;
      otherRead = o.otherRead;
      otherWritten = o.otherWritten;
      mnemonic = o.mnemonic;
      otherEffAddrsRead = o.otherEffAddrsRead;
      otherEffAddrsWritten = o.otherEffAddrsWritten;
      operationID = o.operationID;
    }
    Operation Operation::operator=(const Operation& o)
    {
      readOperands = o.readOperands;
      writtenOperands = o.writtenOperands;
      otherRead = o.otherRead;
      otherWritten = o.otherWritten;
      mnemonic = o.mnemonic;
      otherEffAddrsRead = o.otherEffAddrsRead;
      otherEffAddrsWritten = o.otherEffAddrsWritten;
      operationID = o.operationID;
      return *this;
    }
    Operation::Operation()
    {
      mnemonic = "[INVALID]";
      operationID = e_No_Entry;
    }
    
    const Operation::bitSet& Operation::read() const
    {
      return readOperands;
    }
    const Operation::bitSet& Operation::written() const
    {
      return writtenOperands;
    }    
    const Operation::registerSet&  Operation::implicitReads() const
    {
      return otherRead;
    }
    const Operation::registerSet&  Operation::implicitWrites() const
    {

      return otherWritten;
    }
    std::string Operation::format() const
    {
      return mnemonic;
    }
    size_t Operation::numOperands() const
    {
      return readOperands.size();
      
    }

    std::set<RegisterAST::Ptr> thePC = list_of(RegisterAST::Ptr(new RegisterAST(RegisterAST::makePC())));
    std::set<RegisterAST::Ptr> pcAndSP = list_of(RegisterAST::Ptr(new RegisterAST(RegisterAST::makePC())))
      (RegisterAST::Ptr(new RegisterAST(r_eSP)));

    map<entryID, std::set<RegisterAST::Ptr> > Operation::nonOperandRegisterReads;
    map<entryID, std::set<RegisterAST::Ptr> > Operation::nonOperandRegisterWrites = map_list_of
    (e_call, pcAndSP)
      (e_ret_near, pcAndSP)
      (e_ret_far, pcAndSP)
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
      
    map<entryID, std::set<Expression::Ptr> > Operation::nonOperandMemoryReads;
    map<entryID, std::set<Expression::Ptr> > Operation::nonOperandMemoryWrites;
    
  };
};


  
