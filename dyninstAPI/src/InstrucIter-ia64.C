/*
 * Copyright (c) 1996-2004 Barton P. Miller
 * 
 * We provide the Paradyn Parallel Performance Tools (below
 * described as "Paradyn") on an AS IS basis, and do not warrant its
 * validity or performance.  We reserve the right to update, modify,
 * or discontinue this software at any time.  We shall have no
 * obligation to supply such updates or modifications or any other
 * form of support to you.
 * 
 * This license is for research uses.  For such uses, there is no
 * charge. We define "research use" to mean you may freely use it
 * inside your organization for whatever purposes you see fit. But you
 * may not re-distribute Paradyn or parts of Paradyn, in any form
 * source or binary (including derivatives), electronic or otherwise,
 * to any other organization or entity without our permission.
 * 
 * (for other uses, please contact us at paradyn@cs.wisc.edu)
 * 
 * All warranties, including without limitation, any warranty of
 * merchantability or fitness for a particular purpose, are hereby
 * excluded.
 * 
 * By your use of Paradyn, you understand and agree that we (or any
 * other person or entity with proprietary rights in Paradyn) are
 * under no obligation to provide either maintenance services,
 * update services, notices of latent defects, or correction of
 * defects for Paradyn.
 * 
 * Even if advised of the possibility of such damages, under no
 * circumstances shall we (or any other person or entity with
 * proprietary rights in the software licensed hereunder) be liable
 * to you or any third party for direct, indirect, or consequential
 * damages of any character regardless of type of action, including,
 * without limitation, loss of profits, loss of use, loss of good
 * will, or computer failure or malfunction.  You agree to indemnify
 * us (and any other person or entity with proprietary rights in the
 * software licensed hereunder) for any and all liability it may
 * incur to third parties resulting from your use of Paradyn.
 */

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

BPatch_memoryAccess* InstrucIter::isLoadOrStore()
{
	IA64_instruction * insn = getInstruction();
	IA64_instruction::insnType type = insn->getType();

	BPatch_memoryAccess * bpma = NULL;

	insn_tmpl tmpl = { insn->getMachineCode() };
	uint8_t size = 0x1 << (tmpl.M1.x6 & 0x3);

	switch( type ) {
		case IA64_instruction::INTEGER_16_LOAD:
			size = 16;
		case IA64_instruction::INTEGER_LOAD:
		case IA64_instruction::FP_LOAD:
			bpma = new BPatch_memoryAccess( insn, sizeof( IA64_instruction ), true, false, size, 0, tmpl.M1.r3, -1 );
			assert( bpma != NULL );
			break;

		case IA64_instruction::INTEGER_16_STORE:
			size = 16;
		case IA64_instruction::INTEGER_STORE:
		case IA64_instruction::FP_STORE:
			bpma = new BPatch_memoryAccess( insn, sizeof( IA64_instruction ), false, true, size, 0, tmpl.M1.r3, -1 );
			assert( bpma != NULL );
			break;

		case IA64_instruction::FP_PAIR_LOAD:
		case IA64_instruction::INTEGER_PAIR_LOAD:
			/* The load pair instructions encode sizes a little differently. */
			size = (tmpl.M1.x6 & 0x1) ? 16 : 8;
			bpma = new BPatch_memoryAccess( insn, sizeof( IA64_instruction ), true, false, size, 0, tmpl.M1.r3, -1 );
			assert( bpma != NULL );
			break;

		case IA64_instruction::PREFETCH:
			bpma = new BPatch_memoryAccess( insn, sizeof( IA64_instruction), false, false, 0, tmpl.M1.r3, -1, 0, 0, -1, -1, 0, -1, false, 0 );
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
		insn_tmpl tmpl = { insn->getMachineCode() };
		if( GET_OPCODE( &tmpl ) != 0x9 ) { continue; }

		/* If it's an addl, is its destination register r1? */
		if( tmpl.A5.r3 != 0x1 ) { continue; }

		/* Finally, extract the constant jumpTableOffset. */
		jumpTableOffset = GET_A5_IMM( &tmpl );

		/* We've found the jumpTableOffset, stop looking. */
		// /* DEBUG */ fprintf( stderr, "ip: 0x%lx jumpTableOffset = %ld\n", currentAddress, jumpTableOffset );
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
		insn_tmpl tmpl = { insn->getMachineCode() };
		if( GET_OPCODE( &tmpl ) != 0xD ) { continue; }

		/* Extract the immediate. */
		maxTableLength = GET_A8_COUNT( &tmpl );

		/* We've found a cmp.ltu; stop looking. */
		// /* DEBUG */ fprintf( stderr, "maxTableLength = %ld\n", maxTableLength );
		break;
	} while( true );

	/* Do the indirection. */
	Address jumpTableAddress = 0;
	assert( addressProc->readTextSpace( (void *)jumpTableAddressAddress, sizeof( Address ), & jumpTableAddress ) != false );
	// /* DEBUG */ fprintf( stderr, "jumpTableAddress = 0x%lx\n", jumpTableAddress );

	/* Check for Intel compiler signature.

	   Deviating from the Intel IA64 SCRAG, icc generated jump table entries
	   contain offsets from a label inside the originating function.  Find
	   that label and store it in baseJumpAddress.
	*/
	bool isGCC = true;
	uint64_t baseJumpAddress = jumpTableAddress;
	do {
		/* Rewind one instruction. */
		if( ! hasPrev() ) { break; } (*this)--;

		/* Acquire it. */
		IA64_instruction * insn = getInstruction();

		/* Is it an integer operation? */
		IA64_instruction::unitType unitType = insn->getUnitType();
		if( unitType != IA64_instruction::I ) { continue; }

		/* If so, is it a mov from ip? */
		insn_tmpl tmpl = { insn->getMachineCode() };
		if( tmpl.I25.opcode != 0x0 ||
		    tmpl.I25.x6 != 0x30 ||
		    tmpl.I25.x3 != 0x0 ) continue;

		/* We've found the base jump address; stop looking. */
		baseJumpAddress = currentAddress & ~0xF;
		isGCC = false;
		break;
	} while( true );
	// /* DEBUG */ fprintf( stderr, "baseJumpAddress = 0x%lx\n", baseJumpAddress );

	/* Read n entries from the jump table, summing with jumpTableAddress
	   to add to the set targetAddresses. */
	uint64_t * jumpTable = (uint64_t *)malloc( sizeof( uint64_t ) * maxTableLength );
	assert( jumpTable != NULL );

	if( !addressProc->readTextSpace( (void *)jumpTableAddress, sizeof( uint64_t ) * maxTableLength, jumpTable ) ) {
	    bperr( "Could not read address of jump table (0x%lx) in mutatee.\n", jumpTableAddress );
	    assert( 0 );
	}

	for( unsigned int i = 0; i < maxTableLength; i++ ) {
	    uint64_t finalBaseAddr = baseJumpAddress;

	    /* Deviating from the Intel IA64 SCRAG, GCC generated jump
	       table entries contain offsets from the jump table entry
	       address.  Not the base of the jump table itself.
	    */
	    if (isGCC) finalBaseAddr += i * 8;
	    // /* DEBUG */ fprintf( stderr, "Adding target: 0x%lx (0x%lx + 0x%lx)\n", finalBaseAddr + jumpTable[i], finalBaseAddr, jumpTable[i] );
	    targetAddresses.insert( finalBaseAddr + jumpTable[i] );
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
