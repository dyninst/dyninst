/* -*- Mode: C; indent-tabs-mode: true; tab-width: 4 -*- */

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
	IA64_instruction * insn = getInstruction();
	switch( insn->getType() ) {
		case IA64_instruction::RETURN:
			return true;
			break;
	
		default:
			break;
		} /* end instruction-type switch */
	return false;
}

bool InstrucIter::isAIndirectJumpInstruction()
{
	IA64_instruction * insn = getInstruction();
	switch( insn->getType() ) {
		case IA64_instruction::INDIRECT_BRANCH:
			if( insn->getPredicate() == 0 ) { return true; }
			break;

		default:
			break;
		} /* end instruction-type switch */
	return false;
}

bool InstrucIter::isACondBranchInstruction()
{
	IA64_instruction * insn = getInstruction();
	switch( insn->getType() ) {
		case IA64_instruction::DIRECT_BRANCH:
		/* Not sure if this second case is intended. */
		case IA64_instruction::INDIRECT_BRANCH: {
			if( insn->getPredicate() != 0 ) { return true; }
			break; } 
		
		default:
			break;
		} /* end instruction-type switch */
	return false;
}

/* We take this to mean a dirct conditional branch which always executes. */
bool InstrucIter::isAJumpInstruction()
{
	IA64_instruction * insn = getInstruction();
	switch( insn->getType() ) {
		case IA64_instruction::DIRECT_BRANCH:
			if( insn->getPredicate() == 0 ) { return true; }
			break;

		default:
			break;
		} /* end instruction-type switch */
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

Address InstrucIter::getBranchTargetAddress( Address pos )
{
	IA64_instruction * insn = getInstruction();
	Address rawTargetAddress = insn->getTargetAddress() + pos;
	Address targetAddress = rawTargetAddress - (rawTargetAddress % 0x10);
	// /* DEBUG */ fprintf( stderr, "Instruction at 0x%lx targets 0x%lx\n", currentAddress, targetAddress );
	return targetAddress;
}

void initOpCodeInfo() {
	/* I don't need this for anything. */
	} /* end initOpCodeInfo() */


#define MEMORY_X6_MASK      0x07E0000000000000  /* bits 30 - 35 */
#define MEMORY_ADDR_MASK	0x0003F80000000000	/* bits 20 - 26 */
BPatch_memoryAccess* InstrucIter::isLoadOrStore()
{
	IA64_instruction * insn = getInstruction();
	IA64_instruction::insnType type = insn->getType();

	BPatch_memoryAccess * bpma = NULL;
	
	uint64_t instruction = insn->getMachineCode();	
	unsigned addr = (instruction & MEMORY_ADDR_MASK) >> (ALIGN_RIGHT_SHIFT + 20);

	uint8_t x6 = (instruction & MEMORY_X6_MASK) >> (ALIGN_RIGHT_SHIFT + 30);
	uint8_t size = x6 & 0x03;
	switch( size ) { case 0: size = 1; break; case 1: size = 2; break; case 2: size = 4; break; case 3: size = 8; break; }
	
	switch( type ) {
		case IA64_instruction::INTEGER_LOAD: {
			bpma = new BPatch_memoryAccess( insn, sizeof( IA64_instruction ), true, false, size, 0, addr, -1 );
			assert( bpma != NULL );
			} break;
		case IA64_instruction::INTEGER_STORE:
			bpma = new BPatch_memoryAccess( insn, sizeof( IA64_instruction ), false, true, size, 0, addr, -1 );
			assert( bpma != NULL );
			break;
		case IA64_instruction::FP_LOAD:
			bpma = new BPatch_memoryAccess( insn, sizeof( IA64_instruction ), true, false, size, 0, addr, -1 );
			assert( bpma != NULL );
			break;
		case IA64_instruction::FP_STORE:
			bpma = new BPatch_memoryAccess( insn, sizeof( IA64_instruction ), false, true, size, 0, addr, -1 );
			assert( bpma != NULL );
			break;											
		
		case IA64_instruction::FP_PAIR_LOAD:
		case IA64_instruction::INTEGER_PAIR_LOAD:
			/* The load pair instructions encode sizes a little differently. */
			switch( x6 & 0x03 ) { case 0x02: size = 8; break; case 0x03: size = 16; break; case 0x01: size = 16; break; }

			bpma = new BPatch_memoryAccess( insn, sizeof( IA64_instruction ), true, false, size, 0, addr, -1 );
			assert( bpma != NULL );
			break;
			
		case IA64_instruction::PREFETCH:
			bpma = new BPatch_memoryAccess( insn, sizeof( IA64_instruction), false, false, 0, addr, -1, 0, 0, -1, -1, 0, -1, false, 0 );
			assert( bpma != NULL );
			break;
		
		default:
			return BPatch_memoryAccess::none;
		}

	return bpma;
}

BPatch_instruction *InstrucIter::getBPInstruction() {

  BPatch_memoryAccess *ma = isLoadOrStore();
  BPatch_instruction *in;

  if (ma != BPatch_memoryAccess::none)
    return ma;

  IA64_instruction * i = getInstruction();
  /* Work around compiler idiocy.  FIXME: ignoring long instructions. */
  uint64_t raw = i->getMachineCode();
  in = new BPatch_instruction( & raw, sizeof( raw ) );

  return in;
}

#define INDIRECT_BRANCH_REGISTER_MASK	0x0000007000000000	/* bits 13 - 15 */
#define MOVE_TO_BR_DESTINATION_MASK		0x00000000E0000000	/* bits 06 - 08 */

#define A_TYPE_SIGN_BIT					0x0800000000000000	/* bit 36 */
#define A_TYPE_IMM9D					0x07FC000000000000	/* bits 27 - 35 */
#define A_TYPE_IMM5C					0x0003E00000000000  /* bits 22 - 26 */
#define A_TYPE_SOURCE_MASK				0x0000180000000000  /* bits 21 - 20 */
#define A_TYPE_IMM7B					0x000007F000000000  /* bits 19 - 13 */

void InstrucIter::getMultipleJumpTargets( BPatch_Set<Address> & targetAddresses ) {
	/* The IA-64 SCRAG defines a pattern similar to the power's.  At some constant offset
	   from the GP, there's a jump table whose 64-bit entries are offsets from the base
	   address of the table to the target.  We assume that the nearest previous
	   addition involving r1 is calculating the jump table's address; if this proves to be
	   ill-founded, we'll have to trace registers backwards, starting with the branch
	   register used in the indirect jump. */

	Address gpAddress = (addressImage->getObject()).getTOCoffset();
	Address originalAddress = currentAddress;

	/* We assume that gcc will always generate an addl-form.  (Otherwise,
	   our checks will have to be somewhat more general.) */
	Address jumpTableOffset = 0;
	do {
		/* Rewind one instruction. */
		if( ! hasPrev() ) { return; } (*this)--;
		
		/* Acquire it. */
		IA64_instruction * insn = getInstruction();
		
		/* Is it an integer or memory operation? */
		IA64_instruction::unitType unitType = insn->getUnitType();
		if( unitType != IA64_instruction::I && unitType != IA64_instruction::M ) { continue; }

		/* If so, is it an addl? */
		uint64_t rawInstruction = insn->getMachineCode();
		uint64_t majorOpCode = (rawInstruction & MAJOR_OPCODE_MASK ) >> ( 37 + ALIGN_RIGHT_SHIFT );
		if( majorOpCode != 0x09 ) { continue; }
		
		/* If it's an addl, is its destination register r1? */
		uint64_t destinationRegister = ( rawInstruction & A_TYPE_SOURCE_MASK ) >> ( 20 + ALIGN_RIGHT_SHIFT );
		if( destinationRegister != 0x01 ) { continue; }
		
		/* Finally, extract the constant jumpTableOffset. */
		uint64_t signBit = ( rawInstruction & A_TYPE_SIGN_BIT ) >> ( 36 + ALIGN_RIGHT_SHIFT );
		uint64_t immediate = 	( rawInstruction & A_TYPE_IMM5C ) >> ( 22 + ALIGN_RIGHT_SHIFT - 16 ) |
								( rawInstruction & A_TYPE_IMM9D ) >> ( 27 + ALIGN_RIGHT_SHIFT - 7 ) |
								( rawInstruction & A_TYPE_IMM7B ) >> ( 13 + ALIGN_RIGHT_SHIFT );
		jumpTableOffset = signExtend( signBit, immediate );
		
		/* We've found the jumpTableOffset, stop looking. */
		// /* DEBUG */ fprintf( stderr, "jumpTableOffset = %ld\n", jumpTableOffset );
		break;
	} while( true );
	
	/* Calculate the jump table's address. */
	Address jumpTableAddressAddress = gpAddress + jumpTableOffset;
	// /* DEBUG */ fprintf( stderr, "jumpTableAddressAddress = 0x%lx\n", jumpTableAddressAddress );
	
	/* Assume that the nearest previous immediate-register compare
	   is range-checking the jump-table offset.  Extract that
	   constant, n.  (Otherwise, we're kind of screwed: how big 
	   is the jump table?) */
	uint64_t maxTableLength = 0;
	do {
		/* Rewind one instruction. */
		if( ! hasPrev() ) { return; } (*this)--;
		
		/* Acquire it. */
		IA64_instruction * insn = getInstruction();

		/* Is it an integer or memory operation? */
		IA64_instruction::unitType unitType = insn->getUnitType();
		if( unitType != IA64_instruction::I && unitType != IA64_instruction::M ) { continue; }

		/* If so, is it a cmp.ltu? */
		uint64_t rawInstruction = insn->getMachineCode();
		uint64_t majorOpCode = (rawInstruction & MAJOR_OPCODE_MASK ) >> ( 37 + ALIGN_RIGHT_SHIFT );
		if( majorOpCode != 0x0D ) { continue; }
		
		/* Extract the immediate. */
		uint64_t signBit = ( rawInstruction & A_TYPE_SIGN_BIT ) >> ( 36 + ALIGN_RIGHT_SHIFT );
		uint64_t immediate = ( rawInstruction & A_TYPE_IMM7B ) >> ( 13 + ALIGN_RIGHT_SHIFT );
		maxTableLength = signExtend( signBit, immediate );

		/* We've found a cmp.ltu; stop looking. */
		// /* DEBUG */ fprintf( stderr, "maxTableLength = %ld\n", maxTableLength );
		break;		
	} while( true );

	/* Do the indirection. */
	Address jumpTableAddress = 0;
	assert( addressProc->readTextSpace( (void *)jumpTableAddressAddress, sizeof( Address ), & jumpTableAddress ) != false );
	// /* DEBUG */ fprintf( stderr, "jumpTableAddress = 0x%lx\n", jumpTableAddress );

	/* Read n entries from the jump table, summing with jumpTableAddress
	   to add to the set targetAddresses. */
	uint64_t * jumpTable = (uint64_t *)malloc( sizeof( uint64_t ) * maxTableLength );
	assert( jumpTable != NULL );
	assert( addressProc->readTextSpace( (void *)jumpTableAddress, sizeof( uint64_t ) * maxTableLength, jumpTable ) != false );
	
	for( unsigned int i = 0; i < maxTableLength; i++ ) {
		// /* DEBUG */ fprintf( stderr, "Adding target: 0x%lx (0x%lx + 0x%lx + (%d * 8))\n", jumpTableAddress + jumpTable[i] + (i * 8), jumpTableAddress, jumpTable[i], i );
		targetAddresses.insert( jumpTable[i] + jumpTableAddress + (i * 8) );
		} /* end jump table iteration */

	/* Clean up. */
	free( jumpTable );
	setCurrentAddress( originalAddress );
	} /* end getMultipleJumpTargets() */

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
	assert( (currentAddress % 0x10) < 3 );
	currentAddress = addr;
}

instruction InstrucIter::getInstruction()
{	
	/* In addition to being more efficient, an instruction
	   iterator _must_ iterate over the local copy of the
	   mutatee's address space to pass test6.  (It is apparently
	   defined such that it iterates only over the original code.) */
	uint8_t slotNo = (currentAddress % 0x10 );
	Address alignedBundleAddress = currentAddress - slotNo;
	const ia64_bundle_t * bundlePtr = (const ia64_bundle_t *)addressImage->getPtrToInstruction( alignedBundleAddress );
	IA64_bundle theBundle( * bundlePtr );
	return theBundle.getInstruction( slotNo );
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
	return currentAddress;
}
