#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

#include "common/h/Types.h"
#include "common/h/Vector.h"
#include "common/h/Dictionary.h"

#include "arch.h"
#include "util.h"
#include "process.h"
#include "symtab.h"
#include "instPoint.h"
#include "InstrucIter.h"

#include "BPatch_Set.h"

/* A few utility functions. */
int extractInstructionSlot( Address addr ) {
	return (addr % 16);
	} /* end extractInstructionSlot */

bool addressIsValidInsnAddr( Address addr ) {
	switch( extractInstructionSlot( addr ) ) {
		case 0: case 1: case 2:
			return true;
		default:
			return false;
		}
	} /* end addressIsValidInsnAddr() */

// some more functions used to identify the properties of the instruction
/** is the instruction used to return from the functions
  * @param i the instruction value 
  */
bool InstrucIter::isAReturnInstruction()
{
	return false;
}

extern const unsigned char*
skip_headers(const unsigned char*,bool&,bool&);
extern bool insn_hasSIB(unsigned,unsigned&,unsigned&,unsigned&);

/** is the instruction an indirect jump instruction 
  * @param i the instruction value 
  */
bool InstrucIter::isAIndirectJumpInstruction()
{
	return false;
}

/** is the instruction a conditional branch instruction 
  * @param i the instruction value 
  */ 
bool InstrucIter::isACondBranchInstruction()
{
	return false;
}

/** is the instruction an unconditional branch instruction 
  * @param i the instruction value 
  */
bool InstrucIter::isAJumpInstruction()
{
	return false;
}

/** is the instruction a call instruction 
  * @param i the instruction value 
  */
bool InstrucIter::isACallInstruction()
{
	return false;
}

bool InstrucIter::isAnneal()
{
	return false;
}

/** function which returns the offset of control transfer instructions
  * @param i the instruction value 
  */
Address InstrucIter::getBranchTargetAddress(Address pos)
{
	return false;
}

void initOpCodeInfo()
{
}

/* NOT yet implemented. */
BPatch_memoryAccess* InstrucIter::isLoadOrStore()
{
  return BPatch_memoryAccess::none;
}


void InstrucIter::getMultipleJumpTargets(BPatch_Set<Address>& result)
{
	;
}

bool InstrucIter::delayInstructionSupported()
{
	return false;
}

bool InstrucIter::hasMore()
{
	if( (currentAddress >= baseAddress) &&
		addressIsValidInsnAddr( currentAddress ) &&
		currentAddress < (baseAddress + range) ) {
		return true;
	} else {
		return false;
	}
}

bool InstrucIter::hasPrev()
{
	if( (currentAddress > baseAddress) &&
		addressIsValidInsnAddr( currentAddress ) &&
		currentAddress < (baseAddress + range) ) {
		return true;
	} else {
		return false;
	}
}

Address InstrucIter::prevAddress()
{
	int instructionSlot = extractInstructionSlot( currentAddress );
	switch( instructionSlot ) {
		case 0: return currentAddress - 16 + 2;
		case 1: return currentAddress - 1;
		case 2: return currentAddress - 1;
		default: return 0;
		}
}

Address InstrucIter::nextAddress()
{
	int instructionSlot = extractInstructionSlot( currentAddress );
	switch( instructionSlot ) {
		case 0: return currentAddress + 1;
		case 1: return currentAddress + 1;
		case 2: return currentAddress - 2 + 16;
		default: return 0;
		}
}

void InstrucIter::setCurrentAddress(Address addr)
{
	currentAddress = addr;
}

instruction InstrucIter::getInstruction()
{	
	assert( 0 );
	return instruction();
}

instruction InstrucIter::getNextInstruction()
{	
	assert( 0 );
	return instruction();
}

instruction InstrucIter::getPrevInstruction()
{	
	assert( 0 );
	return instruction();
}

Address InstrucIter::operator++()
{
	return currentAddress = nextAddress();
}

Address InstrucIter::operator--()
{
	return currentAddress = prevAddress();
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
	assert( 0 );
	return 0;
}
