
#include "../h/Operation.h"
#include "arch-x86.h"

namespace Dyninst
{
  namespace Instruction
  {
    Operation::Operation(ia32_entry* e)
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
      case sNONE:
      default:
	break;
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
    
  };
};


  
