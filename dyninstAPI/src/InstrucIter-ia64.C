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

bool InstrucIter::isAReturnInstruction()
{
	assert( 0 );
	return false;
}

bool InstrucIter::isAIndirectJumpInstruction()
{
	assert( 0 );
	return false;
}

bool InstrucIter::isACondBranchInstruction()
{
	assert( 0 );
	return false;
}

bool InstrucIter::isAJumpInstruction()
{
	assert( 0 );
	return false;
}

bool InstrucIter::isACallInstruction()
{
	assert( 0 );
	return false;
}

bool InstrucIter::isAnneal()
{
	assert( 0 );
	return false;
}

Address InstrucIter::getBranchTargetAddress(Address pos)
{
	assert( 0 );
	return false;
}

void initOpCodeInfo() {
	/* I don't need this for anything. */
	} /* end initOpCodeInfo() */

BPatch_memoryAccess* InstrucIter::isLoadOrStore()
{
	assert( 0 );
	return NULL;
}


void InstrucIter::getMultipleJumpTargets(BPatch_Set<Address>& result)
{
	assert( 0 );
}

bool InstrucIter::delayInstructionSupported()
{
	assert( 0 );
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
