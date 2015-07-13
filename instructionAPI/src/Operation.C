/*
 * See the dyninst/COPYRIGHT file for copyright information.
 * 
 * We provide the Paradyn Tools (below described as "Paradyn")
 * on an AS IS basis, and do not warrant its validity or performance.
 * We reserve the right to update, modify, or discontinue this
 * software at any time.  We shall have no obligation to supply such
 * updates or modifications or any other form of support to you.
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

#define INSIDE_INSTRUCTION_API

#include "common/src/Types.h"

#include "Operation.h"
#include "common/src/arch-x86.h"
#include "entryIDs.h"
#include "common/src/Singleton.h"
#include "Register.h"
#include <map>
#include "common/src/singleton_object_pool.h"

using namespace NS_x86;
#include "BinaryFunction.h"
#include "Immediate.h"

namespace Dyninst
{
  namespace InstructionAPI
  {
    RegisterAST::Ptr makeRegFromID(MachRegister regID, unsigned int low, unsigned int high)
    {
      return make_shared(singleton_object_pool<RegisterAST>::construct(regID, low, high));
    }
    RegisterAST::Ptr makeRegFromID(MachRegister regID)
    {
        return make_shared(singleton_object_pool<RegisterAST>::construct(regID, 0, regID.size() * 8));
    }

    Operation::Operation(entryID id, const char* mnem, Architecture arch)
          : mnemonic(mnem), operationID(id), doneOtherSetup(true), doneFlagsSetup(true), archDecodedFrom(arch), prefixID(prefix_none)
    {
        switch(archDecodedFrom)
        {
            case Arch_x86:
            case Arch_ppc32:
                addrWidth = u32;
                break;
            default:
                addrWidth = u64;
                break;
        }
    }
    
    Operation::Operation(ia32_entry* e, ia32_prefixes* p, ia32_locations* l, Architecture arch) :
      doneOtherSetup(false), doneFlagsSetup(false), archDecodedFrom(arch), prefixID(prefix_none)
    
    {
      operationID = e->getID(l);
      // Defaults for no size prefix
      switch(archDecodedFrom)
      {
          case Arch_x86:
          case Arch_ppc32:
              addrWidth = u32;
              break;
          default:
              addrWidth = u64;
              break;
      }
      
      if(p && p->getCount())
      {
        if (p->getPrefix(0) == PREFIX_REP || p->getPrefix(0) == PREFIX_REPNZ)
	{
            otherRead.insert(makeRegFromID((archDecodedFrom == Arch_x86) ? x86::df : x86_64::df));
            otherRead.insert(makeRegFromID((archDecodedFrom == Arch_x86) ? x86::ecx : x86_64::rcx));
            otherWritten.insert(makeRegFromID((archDecodedFrom == Arch_x86) ? x86::ecx : x86_64::rcx));
            if(p->getPrefix(0) == PREFIX_REPNZ)
            {
                otherRead.insert(makeRegFromID((archDecodedFrom == Arch_x86) ? x86::zf : x86_64::zf));
                prefixID = prefix_repnz;
            }
            else
            {
                prefixID = prefix_rep;
            }
        }
        else
        {
          prefixID = prefix_none;
        }
        int segPrefix = p->getPrefix(1);
        switch(segPrefix)
        {
            case PREFIX_SEGCS:
                otherRead.insert(makeRegFromID((archDecodedFrom == Arch_x86) ? x86::cs : x86_64::cs));
                break;
            case PREFIX_SEGDS:
                otherRead.insert(makeRegFromID((archDecodedFrom == Arch_x86) ? x86::ds : x86_64::ds));
                break;
            case PREFIX_SEGES:
                otherRead.insert(makeRegFromID((archDecodedFrom == Arch_x86) ? x86::es : x86_64::es));
                break;
            case PREFIX_SEGFS:
                otherRead.insert(makeRegFromID((archDecodedFrom == Arch_x86) ? x86::fs : x86_64::fs));
                break;
            case PREFIX_SEGGS:
                otherRead.insert(makeRegFromID((archDecodedFrom == Arch_x86) ? x86::gs : x86_64::gs));
                break;
            case PREFIX_SEGSS:
                otherRead.insert(makeRegFromID((archDecodedFrom == Arch_x86) ? x86::ss : x86_64::ss));
                break;
        }
        if(p->getAddrSzPrefix())
        {
            addrWidth = u16;
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
      archDecodedFrom = o.archDecodedFrom;
      prefixID = prefix_none;
      addrWidth = o.addrWidth;
      
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
      archDecodedFrom = o.archDecodedFrom;
      prefixID = o.prefixID;
      addrWidth = o.addrWidth;
      return *this;
    }
    Operation::Operation()
    {
      operationID = e_No_Entry;
      doneOtherSetup = false;
      doneFlagsSetup = false;
      archDecodedFrom = Arch_none;
      prefixID = prefix_none;
      addrWidth = u64;
    }
    
    const Operation::registerSet&  Operation::implicitReads() const
    {
      SetUpNonOperandData(true);
      
      return otherRead;
    }
    const Operation::registerSet&  Operation::implicitWrites() const
    {
      SetUpNonOperandData(true);

      return otherWritten;
    }
    bool Operation::isRead(Expression::Ptr candidate) const
    {
     
	SetUpNonOperandData(candidate->isFlag());
     
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
      SetUpNonOperandData(true);
      return otherEffAddrsRead;
    }
    const Operation::VCSet& Operation::getImplicitMemWrites() const
    {
      SetUpNonOperandData(true);
      return otherEffAddrsWritten;
    }

    bool Operation::isWritten(Expression::Ptr candidate) const
    {
     
	SetUpNonOperandData(candidate->isFlag());
      
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
        if(mnemonic != "")
        {
            return mnemonic;
        }
      dyn_hash_map<prefixEntryID, std::string>::const_iterator foundPrefix = prefixEntryNames_IAPI.find(prefixID);
      dyn_hash_map<entryID, std::string>::const_iterator found = entryNames_IAPI.find(operationID);
      std::string result;
      if(foundPrefix != prefixEntryNames_IAPI.end())
      {
        result += (foundPrefix->second + " ");
      }
      if(found != entryNames_IAPI.end())
      {
	result += found->second;
      }
      else
      {
        result += "[INVALID]";
      }
      return result;
    }

    entryID Operation::getID() const
    {
      return operationID;
    }

    prefixEntryID Operation::getPrefixID() const
    {
      return prefixID;
    }

    struct OperationMaps
    {
    public:
      OperationMaps(Architecture arch)
      {
          thePC.insert(RegisterAST::Ptr(new RegisterAST(MachRegister::getPC(arch))));
          pcAndSP.insert(RegisterAST::Ptr(new RegisterAST(MachRegister::getPC(arch))));
          pcAndSP.insert(RegisterAST::Ptr(new RegisterAST(MachRegister::getStackPointer(arch))));
          stackPointer.insert(RegisterAST::Ptr(new RegisterAST(MachRegister::getStackPointer(arch))));
          stackPointerAsExpr.insert(RegisterAST::Ptr(new RegisterAST(MachRegister::getStackPointer(arch))));
          framePointer.insert(RegisterAST::Ptr(new RegisterAST(MachRegister::getFramePointer(arch))));
          spAndBP.insert(RegisterAST::Ptr(new RegisterAST(MachRegister::getStackPointer(arch))));
          spAndBP.insert(RegisterAST::Ptr(new RegisterAST(MachRegister::getFramePointer(arch))));
          si.insert(RegisterAST::Ptr(new RegisterAST(arch == Arch_x86_64 ? x86_64::esi : x86::esi)));
          di.insert(RegisterAST::Ptr(new RegisterAST(arch == Arch_x86_64 ? x86_64::edi : x86::edi)));
          si_and_di.insert(RegisterAST::Ptr(new RegisterAST(arch == Arch_x86_64 ? x86_64::esi : x86::esi)));
          si_and_di.insert(RegisterAST::Ptr(new RegisterAST(arch == Arch_x86_64 ? x86_64::edi : x86::edi)));
	
          nonOperandRegisterReads[e_call] = pcAndSP;
          nonOperandRegisterReads[e_ret_near] = stackPointer;
          nonOperandRegisterReads[e_ret_far] = stackPointer;
          nonOperandRegisterReads[e_leave] = framePointer;
          nonOperandRegisterReads[e_enter] = spAndBP;
	
          nonOperandRegisterWrites[e_call] = pcAndSP;
          nonOperandRegisterWrites[e_ret_near] = pcAndSP;
          nonOperandRegisterWrites[e_ret_far] = pcAndSP;
          nonOperandRegisterWrites[e_leave] = spAndBP;
          nonOperandRegisterWrites[e_enter] = spAndBP;
          nonOperandRegisterWrites[e_loop] = thePC;
          nonOperandRegisterWrites[e_loope] = thePC;
          nonOperandRegisterWrites[e_loopn] = thePC;
          nonOperandRegisterWrites[e_jb] = thePC;
          nonOperandRegisterWrites[e_jb_jnaej_j] = thePC;
        nonOperandRegisterWrites[e_jbe] = thePC;
        nonOperandRegisterWrites[e_jcxz_jec] = thePC;
        nonOperandRegisterWrites[e_jl] = thePC;
        nonOperandRegisterWrites[e_jle] = thePC;
        nonOperandRegisterWrites[e_jmp] = thePC;
        nonOperandRegisterWrites[e_jnb] = thePC;
        nonOperandRegisterWrites[e_jnb_jae_j] = thePC;
        nonOperandRegisterWrites[e_jnbe] = thePC;
        nonOperandRegisterWrites[e_jnl] = thePC;
        nonOperandRegisterWrites[e_jnle] = thePC;
        nonOperandRegisterWrites[e_jno] = thePC;
        nonOperandRegisterWrites[e_jnp] = thePC;
        nonOperandRegisterWrites[e_jns] = thePC;
        nonOperandRegisterWrites[e_jnz] = thePC;
        nonOperandRegisterWrites[e_jo] = thePC;
        nonOperandRegisterWrites[e_jp] = thePC;
        nonOperandRegisterWrites[e_js] = thePC;
        nonOperandRegisterWrites[e_jz] = thePC;
        nonOperandMemoryReads[e_pop] = stackPointerAsExpr;
        nonOperandMemoryReads[e_popa] = stackPointerAsExpr;
        nonOperandMemoryReads[e_popad] = stackPointerAsExpr;
        nonOperandMemoryWrites[e_push] = stackPointerAsExpr;
        nonOperandMemoryWrites[e_pusha] = stackPointerAsExpr;
        nonOperandMemoryWrites[e_pushad] = stackPointerAsExpr;
        nonOperandMemoryWrites[e_call] = stackPointerAsExpr;
        nonOperandMemoryReads[e_ret_near] = stackPointerAsExpr;
        nonOperandMemoryReads[e_ret_far] = stackPointerAsExpr;
	nonOperandMemoryReads[e_leave] = stackPointerAsExpr;
        nonOperandRegisterWrites[e_cmpsb] = si_and_di;
        nonOperandRegisterWrites[e_cmpsd] = si_and_di;
        nonOperandRegisterWrites[e_cmpsw] = si_and_di;
        nonOperandRegisterWrites[e_movsb] = si_and_di;
        nonOperandRegisterWrites[e_movsd] = si_and_di;
        nonOperandRegisterWrites[e_movsw] = si_and_di;
        nonOperandRegisterWrites[e_cmpsb] = si_and_di;
        nonOperandRegisterWrites[e_cmpsd] = si_and_di;
        nonOperandRegisterWrites[e_cmpsw] = si_and_di;
        nonOperandRegisterWrites[e_insb] = di;
        nonOperandRegisterWrites[e_insd] = di;
        nonOperandRegisterWrites[e_insw] = di;
        nonOperandRegisterWrites[e_stosb] = di;
        nonOperandRegisterWrites[e_stosd] = di;
        nonOperandRegisterWrites[e_stosw] = di;
        nonOperandRegisterWrites[e_scasb] = di;
        nonOperandRegisterWrites[e_scasd] = di;
        nonOperandRegisterWrites[e_scasw] = di;
        nonOperandRegisterWrites[e_lodsb] = si;
        nonOperandRegisterWrites[e_lodsd] = si;
        nonOperandRegisterWrites[e_lodsw] = si;
        nonOperandRegisterWrites[e_outsb] = si;
        nonOperandRegisterWrites[e_outsd] = si;
        nonOperandRegisterWrites[e_outsw] = si;
        
      }
      Operation::registerSet thePC;
      Operation::registerSet pcAndSP;
      Operation::registerSet stackPointer;
      Operation::VCSet stackPointerAsExpr;
      Operation::registerSet framePointer;
      Operation::registerSet spAndBP;
      Operation::registerSet si;
      Operation::registerSet di;
      Operation::registerSet si_and_di;
      dyn_hash_map<entryID, Operation::registerSet > nonOperandRegisterReads;
      dyn_hash_map<entryID, Operation::registerSet > nonOperandRegisterWrites;

      dyn_hash_map<entryID, Operation::VCSet > nonOperandMemoryReads;
      dyn_hash_map<entryID, Operation::VCSet > nonOperandMemoryWrites;
    };
    OperationMaps op_data_32(Arch_x86);
    OperationMaps op_data_64(Arch_x86_64);
    const OperationMaps& op_data(Architecture arch)
    {
        switch(arch)
        {
            case Arch_x86:
                return op_data_32;
            case Arch_x86_64:
                return op_data_64;
            default:
                return op_data_32;
        }
    }
    void Operation::SetUpNonOperandData(bool needFlags) const
    {
        if(doneOtherSetup && doneFlagsSetup) return;
#if defined(arch_x86) || defined(arch_x86_64)      
        dyn_hash_map<entryID, registerSet >::const_iterator foundRegs;
      foundRegs = op_data(archDecodedFrom).nonOperandRegisterReads.find(operationID);
      if(foundRegs != op_data(archDecodedFrom).nonOperandRegisterReads.end())
      {
          otherRead.insert(foundRegs->second.begin(), foundRegs->second.end());
      }
      foundRegs = op_data(archDecodedFrom).nonOperandRegisterWrites.find(operationID);
      if(foundRegs != op_data(archDecodedFrom).nonOperandRegisterWrites.end())
      {
          otherWritten.insert(foundRegs->second.begin(), foundRegs->second.end());
      }
      dyn_hash_map<entryID, VCSet >::const_iterator foundMem;
      foundMem = op_data(archDecodedFrom).nonOperandMemoryReads.find(operationID);
      if(foundMem != op_data(archDecodedFrom).nonOperandMemoryReads.end())
      {
          otherEffAddrsRead.insert(foundMem->second.begin(), foundMem->second.end());
      }
      if(operationID == e_push)
      {
          BinaryFunction::funcT::Ptr adder(new BinaryFunction::addResult());
                    // special case for push: we write at the new value of the SP.
          Result dummy(addrWidth, 0);
          Expression::Ptr push_addr(new BinaryFunction(
                  *(op_data(archDecodedFrom).stackPointerAsExpr.begin()),
          Immediate::makeImmediate(Result(s8, -(dummy.size()))),
          addrWidth,
          adder));
                
          otherEffAddrsWritten.insert(push_addr);
                  
      }
      else
      {
          foundMem = op_data(archDecodedFrom).nonOperandMemoryWrites.find(operationID);
          if(foundMem != op_data(archDecodedFrom).nonOperandMemoryWrites.end())
          {
              otherEffAddrsWritten.insert(foundMem->second.begin(), foundMem->second.end());
          }
      }
      if(needFlags && !doneFlagsSetup)
      {
	
	dyn_hash_map<entryID, flagInfo>::const_iterator found = ia32_instruction::getFlagTable().find(operationID);
	if(found != ia32_instruction::getFlagTable().end())
	{
	  for(unsigned i = 0; i < found->second.readFlags.size(); i++)
	  {
            switch(found->second.readFlags[i]) {
	    case x86::icf:
	      otherRead.insert(makeRegFromID((archDecodedFrom == Arch_x86) ? x86::cf : x86_64::cf));
	      break;
	    case x86::ipf:
	      otherRead.insert(makeRegFromID((archDecodedFrom == Arch_x86) ? x86::pf : x86_64::pf));
	      break;
	    case x86::iaf:
	      otherRead.insert(makeRegFromID((archDecodedFrom == Arch_x86) ? x86::af : x86_64::af));
	      break;
	    case x86::izf:
	      otherRead.insert(makeRegFromID((archDecodedFrom == Arch_x86) ? x86::zf : x86_64::zf));
	      break;
	    case x86::isf:
	      otherRead.insert(makeRegFromID((archDecodedFrom == Arch_x86) ? x86::sf : x86_64::sf));
	      break;
	    case x86::itf:
	      otherRead.insert(makeRegFromID((archDecodedFrom == Arch_x86) ? x86::tf : x86_64::tf));
	      break;
	    case x86::idf:
	      otherRead.insert(makeRegFromID((archDecodedFrom == Arch_x86) ? x86::df : x86_64::df));
	      break;
	    case x86::iof:
	      otherRead.insert(makeRegFromID((archDecodedFrom == Arch_x86) ? x86::of : x86_64::of));
	      break;
	    case x86::int_:
	      otherRead.insert(makeRegFromID((archDecodedFrom == Arch_x86) ? x86::nt_ : x86_64::nt_));
	      break;
            case x86::iif_:
              otherRead.insert(makeRegFromID((archDecodedFrom == Arch_x86) ? x86::if_ : x86_64::if_));
              break;
            default:
	      assert(0);
	    }
	  }
	  for(unsigned j = 0; j < found->second.writtenFlags.size(); j++)
	    {
            switch(found->second.writtenFlags[j]) {
	    case x86::icf:
	      otherWritten.insert(makeRegFromID((archDecodedFrom == Arch_x86) ? x86::cf : x86_64::cf));
	      break;
	    case x86::ipf:
	      otherWritten.insert(makeRegFromID((archDecodedFrom == Arch_x86) ? x86::pf : x86_64::pf));
	      break;
	    case x86::iaf:
	      otherWritten.insert(makeRegFromID((archDecodedFrom == Arch_x86) ? x86::af : x86_64::af));
	      break;
	    case x86::izf:
	      otherWritten.insert(makeRegFromID((archDecodedFrom == Arch_x86) ? x86::zf : x86_64::zf));
	      break;
	    case x86::isf:
	      otherWritten.insert(makeRegFromID((archDecodedFrom == Arch_x86) ? x86::sf : x86_64::sf));
	      break;
	    case x86::itf:
	      otherWritten.insert(makeRegFromID((archDecodedFrom == Arch_x86) ? x86::tf : x86_64::tf));
	      break;
	    case x86::idf:
	      otherWritten.insert(makeRegFromID((archDecodedFrom == Arch_x86) ? x86::df : x86_64::df));
	      break;
	    case x86::iof:
	      otherWritten.insert(makeRegFromID((archDecodedFrom == Arch_x86) ? x86::of : x86_64::of));
	      break;
	    case x86::int_:
	      otherWritten.insert(makeRegFromID((archDecodedFrom == Arch_x86) ? x86::nt_ : x86_64::nt_));
	      break;
	    case x86::iif_:
	      otherWritten.insert(makeRegFromID((archDecodedFrom == Arch_x86) ? x86::if_ : x86_64::if_));
	      break;
	    default:
               fprintf(stderr, "ERROR: unhandled entry %s\n", found->second.writtenFlags[j].name().c_str());
	       assert(0);
	    }
	  }
	}
	doneFlagsSetup = true;
      }
      doneOtherSetup = true;
#else
      (void) needFlags; //Silence warnings
#endif //defined(arch_x86) || defined(arch_x86_64)
    return;
    }
  }

};
