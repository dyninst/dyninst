#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

#include "common/h/Types.h"
#include "common/h/Vector.h"
#include "common/h/Dictionary.h"

#include "arch.h"
#include "arch-ia32.h"
#include "util.h"
#include "process.h"
#include "symtab.h"
#include "instPoint.h"
#include "InstrucIter.h"

#include "BPatch_Set.h"

//some more function used to identify the properties of the instruction
/** is the instruction used to return from the functions
  * @param i the instruction value 
  */
bool InstrucIter::isAReturnInstruction()
{
  const instruction i = getInstruction();
	return i.isReturn();
}

extern const unsigned char*
skip_headers(const unsigned char*,bool&,bool&);
extern bool insn_hasSIB(unsigned,unsigned&,unsigned&,unsigned&);

/** is the instruction an indirect jump instruction 
  * @param i the instruction value 
  */
bool InstrucIter::isAIndirectJumpInstruction()
{
  const instruction i = getInstruction();
	if((i.type() & IS_JUMP) &&
	   (i.type() & INDIR))
	{
		/* since there are two is_jump and indirect instructions
		   we are looking for the one with the indirect register
		   addressing mode one of which ModR/M contains 4 in its
		   reg/opcode field */
		bool isWordAddr,isWordOp;
		const unsigned char* ptr =
			skip_headers(i.ptr(),isWordAddr,isWordOp);
		assert(*ptr == 0xff);
		ptr++;
		if((*ptr & 0x38) == 0x20) 
			return true;
	}
	return false;
}

/** is the instruction a conditional branch instruction 
  * @param i the instruction value 
  */ 
bool InstrucIter::isACondBranchInstruction()
{
  const instruction i = getInstruction();
	if(i.type() & IS_JCC)
		return true;
	return false;
}

/** is the instruction an unconditional branch instruction 
  * @param i the instruction value 
  */
bool InstrucIter::isAJumpInstruction()
{
  const instruction i = getInstruction();
	if((i.type() & IS_JUMP) &&
	   !(i.type() & INDIR) && 
	   !(i.type() & PTR_WX))
		return true;
	return false;
}

/** is the instruction a call instruction 
  * @param i the instruction value 
  */
bool InstrucIter::isACallInstruction()
{
  const instruction i = getInstruction();
	return i.isCall();
}

bool InstrucIter::isAnneal()
{
	return true;
}

/** function which returns the offset of control transfer instructions
  * @param i the instruction value 
  */
Address InstrucIter::getBranchTargetAddress(Address pos)
{
    const instruction i = getInstruction();
	return i.getTarget(pos);
}

void initOpCodeInfo()
{
}

BPatch_memoryAccess* InstrucIter::isLoadOrStore()
{
  // TODO 16-bit registers

  int nac = 0;

  ia32_memacc mac[3];
  ia32_instruction i(mac);
  const unsigned char* addr = instructionPointers[currentAddress-baseAddress];

  BPatch_memoryAccess* bmap = BPatch_memoryAccess::none;

#if defined(i386_unknown_nt4_0) && _MSC_VER < 1300
  ia32_decode(IA32_DECODE_MEMACCESS, addr, i);
#else
  ia32_decode<IA32_DECODE_MEMACCESS>(addr, i);
#endif
  
  bool first = true;

  for(int j=0; j<3; ++j) {
    const ia32_memacc& mac = i.getMac(j);
    if(mac.is) {
      if(first) {
        bmap = new BPatch_memoryAccess(mac.read, mac.write, mac.size, mac.imm,
                                       mac.regs[0], mac.regs[1], mac.scale);
        first = false;
      }
      else
        bmap->set2nd(mac.read, mac.write, mac.size, mac.imm,
                     mac.regs[0], mac.regs[1], mac.scale);
        
      // TODO: deal with REP prefixes
      ++nac;
    }
  }
  assert(nac < 3);
  
  return bmap;
}


//Address Handle used by flowGraph which wraps the instructions
//and supply enough operation to iterate over the instrcution sequence.

void InstrucIter::init()
{
  if(!instructionPointers) {
	unsigned i=0, j;
	instructionPointers = new InstrucPos[range];
	const unsigned char* ptr = addressImage->getPtrToInstruction(baseAddress);
	while(i<range) {
		instructionPointers[i++] = ptr;
		unsigned instructionType;
		unsigned instructionSize = get_instruction(ptr,instructionType);
		for(j=1;j<instructionSize;j++){
			if(i < range)
				instructionPointers[i++] = NULL;
			else {
				range -= j;
				instructionSize = 0;
				break;
			}
		}
		ptr += instructionSize;
	}
  }
}

void InstrucIter::copy(const InstrucIter& ah)
{
	instructionPointers = new InstrucPos[range];
	for(unsigned i=0;i<range;i++)
		instructionPointers[i] = ah.instructionPointers[i];
}

void InstrucIter::kill()
{
  delete[] instructionPointers;
}

void InstrucIter::getMultipleJumpTargets(BPatch_Set<Address>& result,
					   InstrucIter& mayUpdate)
{
	Address backupAddress = currentAddress;
	(*this)--;

	/* check whether register indirect or memory indirect */
	instruction tableInsn = getInstruction();
	bool isAddressInJmp = true;
	bool isWordAddr,isWordOp;
	const unsigned char* ptr =
		skip_headers(tableInsn.ptr(),isWordAddr,isWordOp);
	assert(*ptr == 0xff);
        ptr++;
	/* if register indirect then table address will be taken from
           previous ins */
	if((*ptr & 0xc7) != 0x04){
		isAddressInJmp = false;
		(*this)--;
		while(hasMore()){
			tableInsn = getInstruction();
			ptr = skip_headers(tableInsn.ptr(),isWordAddr,isWordOp);
			if(*ptr == 0x8b)
				break;
			(*this)--;
		}
	}
	(*this)--;
	/* now get the instruction to find the maximum switch value */
	instruction maxSwitchInsn;
	while(hasMore()){
		instruction check = getInstruction();
		if(check.type() & IS_JCC){
			(*this)--;
			maxSwitchInsn = getInstruction();
			break;
		}
		(*this)--;
	}

	unsigned maxSwitch = 0;
	ptr = skip_headers(maxSwitchInsn.ptr(),isWordAddr,isWordOp);
	if(*ptr == 0x3d){
		ptr++;
		if(isWordOp){
			maxSwitch |= *(ptr+1);
			maxSwitch <<= 8;
			maxSwitch |= *ptr;
		}
		else
			maxSwitch = *(const unsigned*)ptr;
		maxSwitch++;
	}
	else if(*ptr == 0x83){
		ptr++;
		if((*ptr & 0x38) == 0x38){
			unsigned Mod,Reg,RM;
			bool hasSIB = insn_hasSIB(*ptr,Mod,Reg,RM);
			ptr++;
			if(hasSIB)
				ptr++;
			maxSwitch = *ptr;
			maxSwitch++;
		}
	}
	if(!maxSwitch){
		result += backupAddress;	
		return;
	}

	Address jumpTable = 0;
	ptr = skip_headers(tableInsn.ptr(),isWordAddr,isWordOp);
	if(isAddressInJmp || (!isAddressInJmp && (*ptr == 0x8b))){
		ptr++;
		if(((*ptr & 0xc7) == 0x04) &&
		   ((*(ptr+1) & 0xc7) == 0x85))
		{
			ptr += 2;
			if(isWordAddr){
				jumpTable |= *(ptr+1);
				jumpTable <<= 8;
				jumpTable |= *ptr;
			}
			else
				jumpTable = *(const Address*)ptr;
		}
	}

	if(!jumpTable){
		result += backupAddress;	
		return;
	}

	for(unsigned int i=0;i<maxSwitch;i++){
		Address tableEntry = jumpTable + (i * sizeof(Address));
		if(containsAddress(tableEntry)){
			for(unsigned j=0;j<sizeof(Address);j++)
				mayUpdate.instructionPointers[(tableEntry+j)-baseAddress] = NULL;
			if(mayUpdate.currentAddress == tableEntry)
				mayUpdate.currentAddress += sizeof(Address);
		}
		int jumpAddress = 0;
                addressProc->readTextSpace((const void*)tableEntry,
                                           sizeof(Address),
                                           (const void*)&jumpAddress);
                result += jumpAddress;
	}
}

bool InstrucIter::delayInstructionSupported()
{
  return false;
}

bool InstrucIter::hasMore()
{
	if((currentAddress < (baseAddress + range )) &&
	   (currentAddress >= baseAddress))
		return true;
	return false;
}

bool InstrucIter::hasPrev()
{
    if((currentAddress < (baseAddress + range )) &&
       (currentAddress > baseAddress))
	return true;
    return false;
}

Address InstrucIter::prevAddress()
{
	Address i = currentAddress-1;
	for(;i>=baseAddress;i--)
		if(instructionPointers[i-baseAddress])
			break;
	return i;
}

Address InstrucIter::nextAddress()
{
	Address i = currentAddress+1;
	for(;i<(baseAddress+range);i++)
		if(instructionPointers[i-baseAddress])
			break;
	return i;
}

void InstrucIter::setCurrentAddress(Address addr)
{
	currentAddress = addr;
}

instruction InstrucIter::getInstruction()
{
	const unsigned char* ptr = 
			instructionPointers[currentAddress-baseAddress];
	unsigned instructionType;
	unsigned instructionSize = get_instruction(ptr,instructionType);
	return instruction(ptr,instructionType,instructionSize);
}

instruction InstrucIter::getNextInstruction()
{
	Address addr = nextAddress();
	const unsigned char* ptr = instructionPointers[addr-baseAddress];
	unsigned instructionType;
	unsigned instructionSize = get_instruction(ptr,instructionType);
	return instruction(ptr,instructionType,instructionSize);
}

instruction InstrucIter::getPrevInstruction()
{
	Address addr = prevAddress();
	const unsigned char* ptr = instructionPointers[addr-baseAddress];
	unsigned instructionType;
	unsigned instructionSize = get_instruction(ptr,instructionType);
	return instruction(ptr,instructionType,instructionSize);
}

Address InstrucIter::operator++()
{
	currentAddress = nextAddress();
	return currentAddress;
}

Address InstrucIter::operator--()
{
	currentAddress = prevAddress();
	return currentAddress;
}

Address InstrucIter::operator++(int)
{
	Address ret = currentAddress;
	currentAddress = nextAddress();
	return ret;
}

Address InstrucIter::operator--(int)
{
	Address ret = currentAddress;
	currentAddress = prevAddress();
	return ret;
}

Address InstrucIter::operator*()
{
	return currentAddress;
}
