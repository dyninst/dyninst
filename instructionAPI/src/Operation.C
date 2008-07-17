
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

    Operation::Operation(Dyninst::InstructionAPI::ia32_entry* e)
    {
      if(!e)
      {
	return;
      }
      if(e->name())
      {
	mnemonic = e->name();
      }
      else
      {
	mnemonic = "[INVALID]";
      }
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
    int Operation::numOperands() const
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


  
