/*
 * Copyright (c) 1996 Barton P. Miller
 * 
 * We provide the Paradyn Parallel Performance Tools (below
 * described as Paradyn") on an AS IS basis, and do not warrant its
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

// $Id: arch-ia64.C,v 1.9 2002/08/01 18:37:28 tlmiller Exp $
// ia64 instruction decoder

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include "dyninstAPI/src/arch-ia64.h"

#define TEMPLATE_MASK		0x000000000000001F	/* bits 00 - 04 */
#define INSTRUCTION0_MASK	0x00003FFFFFFFFFE0	/* bits 05 - 45 */
#define INSTRUCTION1_LOW_MASK	0xFFFFC00000000000	/* bits 45 - 63 */
#define INSTRUCTION1_HIGH_MASK	0x00000000007FFFFF	/* bits 00 - 20 */
#define INSTRUCTION2_MASK	0xFFFFFFFFFF800000	/* bits 21 - 63 */

#define MAJOR_OPCODE_MASK	0xF000000000000000	/* bits 37 - 40 */

#define MEMORY_X3_MASK		0x0700000000000000	/* bits 33 - 35 */
#define MEMORY_R1_MASK		0x0000000FE0000000	/* bits 06 - 12 */

#define ALLOC_SOR		0x003C000000000000	/* bits 27 - 30 */
#define ALLOC_SOL		0x0003F80000000000	/* bits 20 - 26 */
#define ALLOC_SOF		0x000007F000000000	/* bits 13 - 19 */
#define ALLOC_REGISTER		0x0000000FE0000000	/* bits 06 - 12 */

#define PREDICATE_MASK		0x000000001F800000	/* bits 00 - 05 */

#define ADDL_SIGN		0x0800000000000000	/* bit 36 */
#define ADDL_IMM9D		0x07FC000000000000	/* bits 27 - 35 */
#define ADDL_IMM5C		0x0003E00000000000	/* bits 22 - 26 */
#define ADDL_R3			0x0000180000000000	/* bits 20 - 21 */
#define ADDL_IMM7B		0x000007F000000000	/* bits 13 - 19 */
#define ADDL_R1			0x0000000FE0000000	/* bits 06 - 12 */

#define MOVL_I			0x0800000000000000	/* bit 36 */
#define MOVL_IMM9D		0x07FC000000000000	/* bits 27 - 35 */
#define MOVL_IMM5C		0x0003E00000000000	/* bits 22 - 26 */
#define MOVL_IC			0x0000100000000000	/* bit 21 */
#define MOVL_VC			0x0000080000000000	/* bit 20 */
#define MOVL_IMM7B		0x000007F000000000	/* bits 13 - 19 */
#define MOVL_R1			0x0000000FE0000000	/* bits 06 - 12 */

#define IBRANCH_X6		0x00FC000000000000	/* bits 27 - 32 */
#define IBRANCH_B2		0x0000007000000000	/* bits 13 - 15 */
#define IBRANCH_BTYPE		0x00000000E0000000	/* btis 06 - 08 */

#define RIGHT_IMM5C		0x00000000001F0000	/* bits 16 - 20 */
#define RIGHT_IMM9D		0x000000000000FF80	/* bits 07 - 15 */
#define RIGHT_IMM6D		0x0000000000001F80	/* bits 07 - 12 */
#define RIGHT_IMM7B		0x000000000000007F	/* bits 00 - 06 */
#define RIGHT_IMM_I		0x8000000000000000	/* bit 63 */
#define RIGHT_IMM_IC		0x0000000000200000	/* bit 21 */
#define RIGHT_IMM8C		0x0000000000007F80	/* bits 07 - 15 */
#define RIGHT_IMM7A		0x000000000000007F	/* bits 00 - 06 */

#define RIGHT_IMM41		0x7FFFFFFFFFC00000	/* bits 22 - 62 */
#define RIGHT_IMM20		0x00000000000FFFFF	/* bits 00 - 19 */
#define RIGHT_IMM39		0x07FFFFFFFFF00000	/* bits 20 - 58 */

/* NOTE: for the IA64_bundle constructor to work, the individual
	instruction 'halves' should left-aligned as if they were independent instructions. */
IA64_instruction_x::IA64_instruction_x( uint64_t lowHalf = 0, uint64_t highHalf = 0, uint8_t templ = 0xFF, IA64_bundle * mybl = 0 ) {
	instruction = lowHalf;
	instruction_x = highHalf;
	templateID = templ;
	myBundle = mybl;
	} /* end IA64_Instruction_x() */

IA64_instruction::IA64_instruction( uint64_t insn, uint8_t templ, const IA64_bundle * mybl, uint8_t slotN ) {
	instruction = insn;
	templateID = templ;
	myBundle = mybl;
	slotNumber = slotN;
	} /* end IA64_Instruction() */

const void * IA64_instruction::ptr() const { 
	/* If I don't have a bundle, trying to write me is pointless;
	   this should also make it harder for "unaware" functions to misuse me. */
	if( myBundle == NULL ) { return NULL; }

	return myBundle->getMachineCodePtr();
	} /* end ptr() */

IA64_instruction::insnType IA64_instruction::getType() const {
	/* Since we're only interested in calls and returns, for now,
	   verify we're a B-unit instruction and have the proper major opcode:
	   0x5 (ip-relative) or 0x1 (indirect) for calls, and 0x0 (indirect) for returns
	   (in which case, verify that x6 == 33 and btype == 4);
	   otherwise, return OTHER.  We'll handle the long cases in the subclass. */

	bool isBranchUnit = false;
	switch( templateID ) {
		case 0xFF:
			fprintf( stderr, "IA64_instruction constructed without templateID, aborting.\n" );
			abort();
			break;

		/* Fall-through: slot 0, slot 1, slot 2 B-units. */
		case 0x16: case 0x17:
			if( slotNumber == 0 ) { isBranchUnit = true; }

		case 0x12: case 0x13:
			if( slotNumber == 1 ) { isBranchUnit = true; }

		case 0x10: case 0x11: case 0x18: case 0x19: case 0x1C: case 0x1D:
			if( slotNumber == 2 ) { isBranchUnit = true; }
			break;

		default:
			return OTHER;
			break;
		} /* end isBranchOrReturn switch. */

	if( ! isBranchUnit ) { return OTHER; }

	/* Fetch and verify the major opcode. */
	uint8_t opcode = (instruction & MAJOR_OPCODE_MASK) >> (ALIGN_RIGHT_SHIFT + 37);

	switch( opcode ) {
		case 0x05: case 0x01:
			return CALL;

		case 0x04:
			return BRANCH;

		case 0x00:
			{
			/* Is it a return or an indirect branch or something else? */
			uint8_t x6 = (instruction & IBRANCH_X6) >> (ALIGN_RIGHT_SHIFT + 27);
			uint8_t btype = (instruction & IBRANCH_BTYPE) >> (ALIGN_RIGHT_SHIFT + 6);
			if( x6 == 33 && btype == 4 ) { return RETURN; }
			else if( x6 == 20 && (btype == 0 || btype == 1) ) { return BRANCH; }
			else { return OTHER; }
			break;
			}

		default:
			return OTHER;
		} /* end opcode switch */
	} /* end getType() */

IA64_instruction::insnType IA64_instruction_x::getType() const {
	/* We know we're a long instruction, so just check the major opcode to see which one. */
	uint8_t opcode = (instruction_x & MAJOR_OPCODE_MASK) >> (ALIGN_RIGHT_SHIFT + 37);
	if( opcode == 0x0D ) { return CALL; } else { return OTHER; }
	} /* end getType() */

IA64_bundle::IA64_bundle( ia64_bundle_t rawBundle ) {
	* this = IA64_bundle( rawBundle.low, rawBundle.high );
	} /* end IA64_bundle() */

IA64_bundle::IA64_bundle( uint8_t templateID, IA64_instruction & instruction0, IA64_instruction instruction1, IA64_instruction instruction2 ) {
	* this = IA64_bundle( templateID, instruction0.getMachineCode(), instruction1.getMachineCode(), instruction2.getMachineCode() );
	} /* end IA64_bundle() */

/* This handles the MLX template/long instructions. */
IA64_bundle::IA64_bundle( uint8_t templateID, IA64_instruction & instruction0, IA64_instruction_x & instructionLX ) {
	if( templateID != 0x05 ) { fprintf( stderr, "Attempting to generate a bundle with a long instruction without using the MLX template, aborting.\n" ); abort(); }

	* this = IA64_bundle( templateID, instruction0, instructionLX.getMachineCode().low, instructionLX.getMachineCode().high );
	} /* end IA64_bundle() */

IA64_bundle::IA64_bundle( uint8_t templateID, uint64_t instruction0, uint64_t instruction1, uint64_t instruction2 ) {
	this->templateID = templateID;
	this->instruction0 = IA64_instruction( instruction0, templateID, this, 0 );
	this->instruction1 = IA64_instruction( instruction1, templateID, this, 1 );
	this->instruction2 = IA64_instruction( instruction2, templateID, this, 2 ); 

	myBundle.low =	( templateID & TEMPLATE_MASK ) |
			( (instruction0 >> (ALIGN_RIGHT_SHIFT - 5)) & INSTRUCTION0_MASK ) |
			( (instruction1 << 23) & INSTRUCTION1_LOW_MASK );
	myBundle.high = ( (instruction1 >> (ALIGN_RIGHT_SHIFT + 18)) & INSTRUCTION1_HIGH_MASK ) |
			( instruction2 & INSTRUCTION2_MASK );
	} /* end IA64_bundle() */

IA64_bundle::IA64_bundle( uint64_t lowHalfBundle, uint64_t highHalfBundle ) {
	/* The template is right-aligned; the instructions are left-aligned. */
	templateID = lowHalfBundle & TEMPLATE_MASK;
	instruction0 = IA64_instruction( (lowHalfBundle & INSTRUCTION0_MASK) << 18, templateID, this, 0 );
	instruction1 = IA64_instruction( ((lowHalfBundle & INSTRUCTION1_LOW_MASK) >> 23) +
						((highHalfBundle & INSTRUCTION1_HIGH_MASK) << 41), templateID, this, 1 );
	instruction2 = IA64_instruction( highHalfBundle & INSTRUCTION2_MASK, templateID, this, 2 );

	myBundle.low = lowHalfBundle;
	myBundle.high = highHalfBundle;
	} /* end IA64_Bundle() */

IA64_instruction_x IA64_bundle::getLongInstruction() {
	return IA64_instruction_x( instruction1.getMachineCode(), instruction2.getMachineCode(), 0x05, this );
	} /* end getLongInstruction() */

IA64_instruction IA64_bundle::getInstruction( unsigned int slot ) {
	switch( slot ) {
		case 0: return instruction0;
		case 1: return instruction1;
		case 2: return instruction2;
		default: fprintf( stderr, "Request of invalid instruction (%d), aborting.\n", slot ); abort();
		}
	} /* end getInstruction() */

/* private (refactoring) function, used by generateAlteredAlloc() and defineBaseTrampRegisterSpaceFor() */
bool extractAllocatedRegisters( uint64_t allocInsn, uint64_t * allocatedLocal, uint64_t * allocatedOutput, uint64_t * allocatedRotate ) {
	/* Verify that the given instruction is actually, so far as we can tell
	   (we don't have the template and the offset), an alloc. */
	if( allocInsn & MAJOR_OPCODE_MASK != ((uint64_t)1) << 37 + ALIGN_RIGHT_SHIFT ||
	    allocInsn & MEMORY_X3_MASK != ((uint64_t)6) << 33 + ALIGN_RIGHT_SHIFT ) {
		* allocatedLocal = * allocatedOutput = * allocatedRotate = 0;
		return false;
		} /* end if not an alloc instruction */

	/* Extract the local, output, and rotate sizes. */
	* allocatedLocal = (allocInsn & ALLOC_SOL) >> (20 + ALIGN_RIGHT_SHIFT);
	* allocatedOutput = ( (allocInsn & ALLOC_SOF) >> (13 + ALIGN_RIGHT_SHIFT) ) - * allocatedLocal;
	* allocatedRotate = (allocInsn & ALLOC_SOR) >> (27 + ALIGN_RIGHT_SHIFT + 3); // SOR = r >> 3

	/* Completed successfully. */
	return true;
        } /* end extractAllocatedRegisters() */

/* For linux-ia64.C */
IA64_instruction generateAlteredAlloc( uint64_t allocInsn, int deltaLocal, int deltaOutput, int deltaRotate ) {
	/* Extract the allocated sizes. */
	uint64_t allocatedLocal, allocatedOutput, allocatedRotate;
	if( ! extractAllocatedRegisters( allocInsn, & allocatedLocal, & allocatedOutput, & allocatedRotate ) ) {
		fprintf( stderr, "Alleged alloc instruction is not, aborting.\n" );
		abort();
		} /* end if not an alloc instruction */

	/* Calculate the new sizes. */
	uint64_t generatedLocal = allocatedLocal + deltaLocal;
	uint64_t generatedOutput = allocatedOutput + deltaOutput;
	uint64_t generatedRotate = allocatedRotate + deltaRotate;

	/* Alter allocInsn. */
	allocInsn = 0x0000000000000000 |
		    ( ((uint64_t)1) << (37 + ALIGN_RIGHT_SHIFT) ) |
		    ( ((uint64_t)6) << (33 + ALIGN_RIGHT_SHIFT) ) |
		    ( generatedLocal << (20 + ALIGN_RIGHT_SHIFT) ) | 
		    ( ( generatedRotate >> 3) << (27 + ALIGN_RIGHT_SHIFT) ) |
		    ( ( generatedLocal + generatedOutput ) << (13 + ALIGN_RIGHT_SHIFT) ) | 
		    ( allocInsn & ALLOC_REGISTER );
	
	/* We're done. */
	return allocInsn;
	} /* end generateAlteredAlloc() */

/* imm22 is assumed to be right-aligned, e.g., an actual value. :) */
IA64_instruction generateShortConstantInRegister( unsigned int registerN, int imm22 ) {
	uint64_t sImm22 = (uint64_t)imm22;
	uint64_t sRegisterN = (uint64_t)registerN;
	
	/* addl */
	uint64_t rawInsn = 0x0000000000000000 | 
			   ( ((uint64_t)9) << (37 + ALIGN_RIGHT_SHIFT)) |
			   ( ((uint64_t)(imm22 < 0)) << (36 + ALIGN_RIGHT_SHIFT)) |
			   ( (sImm22 & RIGHT_IMM7B) << (13 + ALIGN_RIGHT_SHIFT)) |
			   ( (sImm22 & RIGHT_IMM5C) << (-16 + 22 + ALIGN_RIGHT_SHIFT)) |
			   ( (sImm22 & RIGHT_IMM9D) << (-7 + 27 + ALIGN_RIGHT_SHIFT)) |
			   ( (sRegisterN & RIGHT_IMM7B) << (6 + ALIGN_RIGHT_SHIFT) );

	return IA64_instruction( rawInsn );
	} /* end generateConstantInRegister( imm22 ) */

IA64_instruction_x generateLongConstantInRegister( unsigned int registerN, long long int imm64 ) {
	uint64_t sRegisterN = (uint64_t)registerN;

	/* movl */
	uint64_t rawInsnHigh = 0x0000000000000000 | 
			   ( ((uint64_t)6) << (37 + ALIGN_RIGHT_SHIFT)) |
			   ( (imm64 & RIGHT_IMM_I) << (-63 + 36 + ALIGN_RIGHT_SHIFT)) |
			   ( (imm64 & RIGHT_IMM9D) << (-7 + 27 + ALIGN_RIGHT_SHIFT)) |
			   ( (imm64 & RIGHT_IMM5C) << (-16 + 22 + ALIGN_RIGHT_SHIFT)) |
			   ( (imm64 & RIGHT_IMM_IC) << (-21 + 21 + ALIGN_RIGHT_SHIFT)) |
			   ( (imm64 & RIGHT_IMM7B) << (13 + ALIGN_RIGHT_SHIFT)) |
			   ( (sRegisterN & RIGHT_IMM7B) << (6 + ALIGN_RIGHT_SHIFT) );
	uint64_t rawInsnLow = 0x0000000000000000 |
			   ( (imm64 & RIGHT_IMM41) << (-22 + ALIGN_RIGHT_SHIFT) );

	return IA64_instruction_x( rawInsnLow, rawInsnHigh );
	} /* end generateConstantInRegister( imm64 ) */

IA64_instruction_x generateLongCallTo( long long int displacement64, unsigned int branchRegister ) {
	int64_t displacement60 = displacement64 >> 4;
	uint64_t sBranchRegister = (uint64_t)branchRegister;

	uint64_t rawInsnHigh = 0x0000000000000000 | 
			   ( ((uint64_t)0xD) << (37 + ALIGN_RIGHT_SHIFT)) |
			   ( ((uint64_t)(displacement64 < 0)) << (36 + ALIGN_RIGHT_SHIFT)) |
			   ( (displacement60 & RIGHT_IMM20) << (13 + ALIGN_RIGHT_SHIFT)) |
			   ( (sBranchRegister & 0x7 ) << (6 + ALIGN_RIGHT_SHIFT));
	uint64_t rawInsnLow = 0x0000000000000000 |
			   ( (displacement60 & RIGHT_IMM39) << (-20 + 2 + ALIGN_RIGHT_SHIFT) );

	return IA64_instruction_x( rawInsnLow, rawInsnHigh );
	} /* end generateLongCallTo( displacement64 ) */

IA64_instruction_x generateLongBranchTo( long long int displacement64 ) {
	int64_t displacement60 = displacement64 >> 4;

	uint64_t rawInsnHigh = 0x0000000000000000 | 
			   ( ((uint64_t)0xC) << (37 + ALIGN_RIGHT_SHIFT)) |
			   ( ((uint64_t)(displacement64 < 0)) << (36 + ALIGN_RIGHT_SHIFT)) |
			   ( (displacement60 & RIGHT_IMM20) << (13 + ALIGN_RIGHT_SHIFT));
	uint64_t rawInsnLow = 0x0000000000000000 |
			   ( (displacement60 & RIGHT_IMM39) << (-20 + 2 + ALIGN_RIGHT_SHIFT) );

	return IA64_instruction_x( rawInsnLow, rawInsnHigh );
	} /* end generateLongBranchTo( displacement64 ) */

IA64_instruction generateReturnTo( unsigned int branchRegister ) {
	uint64_t sBranchRegister = (uint64_t)branchRegister;

	uint64_t rawInsn = 0x0000000000000000 |
			   ( ((uint64_t)33) << (27 + ALIGN_RIGHT_SHIFT ) ) | 
			   ( ((uint64_t)4) << (6 + ALIGN_RIGHT_SHIFT ) ) |
			   ( ((uint64_t)1) << (12 + ALIGN_RIGHT_SHIFT ) ) |
			   ( (sBranchRegister & 0x7) << (13 + ALIGN_RIGHT_SHIFT) );

	return IA64_instruction( rawInsn );	
	} /* end generateReturnTo */

/* Required by func-reloc.C to calculate relative displacements. */
int get_disp( instruction *insn ) {
	assert( 0 );
	return 0;
	} /* end get_disp() */
                
/* Required by func-reloc.C to correct relative displacements after relocation. */
int set_disp( bool setDisp, instruction * insn, int newOffset, bool outOfFunc ) {
	assert( 0 );
	return 0;
	} /* end set_disp() */

/* Convience methods for func-reloc.C */
int sizeOfMachineInsn( instruction * insn ) {
	assert( 0 );
	return 0;
	} /* end sizeOfMachineInsn() */

int addressOfMachineInsn( instruction * insn ) {
	assert( 0 );
	return 0;
	} /* end addressOfMachineInsn */

/* Convience method for inst-ia64.C */
IA64_bundle generateBundleFromLongInstruction( IA64_instruction_x longInstruction ) {	
	IA64_instruction memoryNOP( NOP_M );
	return IA64_bundle( MLXstop, memoryNOP, longInstruction );
	} /* end generateBundleFromLongInstruction() */

/* Required by inst-ia64.C */
Address IA64_instruction::getTargetAddress() {
	if( getType() == CALL || getType() == BRANCH ) {
		uint8_t opcode = (instruction & MAJOR_OPCODE_MASK) >> (ALIGN_RIGHT_SHIFT + 37);
		switch( opcode ) {
			case 0x04: case 0x05:
				/* ip-relative branch and call, respectively. */
				return ( 0x0000000000000000 |
					(instruction >> (13 + ALIGN_RIGHT_SHIFT)) & RIGHT_IMM20 |
					((instruction >> (36 + ALIGN_RIGHT_SHIFT)) << 20 ) ) << 4;
				break;
			case 0x01: case 0x00:
				/* indirect branch and call, respectively. */
				return 0;
				break;
			default:
				fprintf( stderr, "getTargetAddress(): unrecognized major opcode, aborting.\n" );
				abort();
				break;
			} /* end opcode switch */	
		} else {
		return 0;
		}
	} /* end getTargetAddress() */

Address IA64_instruction_x::getTargetAddress() {
	if( getType() == CALL || getType() == BRANCH ) { // FIXME
		assert( 0 );
		} else {
		return 0;
		}
	} /* end getTargetAddress() */

/* We don't know which registers we'll be freeing (killing) until we can look
   at the relevant alloc instruction. */
#include "instPoint-ia64.h"
void defineBaseTrampRegisterSpaceFor( const instPoint * location, registerSpace * regSpace ) {
	/* FIXME: Scan the function for alloc instructions*.  If there's more than one,
	   define the register space to be the first eight locals, and the first eight output
	   registers; but mark them live so that they'll be saved before being used (and
	   restored afterwards).  Option: go ahead and save all sixteen of these guys
	   at the beginning of the base tramp and mark them dead, and then restore them
	   at the end.

	   * Actually, we should flag the alloc instructions during parse & decode. */

	Address fnEntryOffset = location->iPgetFunction()->addr();
	const ia64_bundle_t * rawBundles = (const ia64_bundle_t *) location->iPgetOwner()->getPtrToInstruction( fnEntryOffset );
	IA64_bundle initialBundle( rawBundles[0] );

	/* Extract the allocated sizes. */
	uint64_t allocatedLocal, allocatedOutput, allocatedRotate;
	if( ! extractAllocatedRegisters( initialBundle.getInstruction(0).getMachineCode(), & allocatedLocal, & allocatedOutput, & allocatedRotate ) ) {
		assert( 0 );
		return;
		} /* end if no alloc was found. */

	/* Adjust regSpace to reflect the size of the function's frame. */
	Register deadRegisterList[NUM_LOCALS + NUM_OUTPUT];
	for( int i = 0; i < NUM_LOCALS + NUM_OUTPUT; i++ ) {
		deadRegisterList[i] = 32 + allocatedLocal + allocatedOutput + i;
		} /* end deadRegisterList calculation */
	registerSpace rs( NUM_LOCALS + NUM_OUTPUT, deadRegisterList, 0, NULL );
	* regSpace = rs;
	} /* end dBTRSF() */

/* For inst-ia64.h */
IA64_instruction generateIPToRegisterMove( Register destination ) {
	uint64_t generalRegister = ((uint64_t)destination) & 0x7F;
	
	uint64_t rawInsn = 0x0000000000000000 |
			   ( ((uint64_t)48) << (27 + ALIGN_RIGHT_SHIFT) ) |
			   ( generalRegister << (6 + ALIGN_RIGHT_SHIFT) );

	return IA64_instruction( rawInsn );
	} /* end generateIPToRegisterMove() */

IA64_instruction generateBranchToRegisterMove( Register source, Register destination ) {
	uint64_t branchRegister = ((uint64_t)source) & 0x07;
	uint64_t generalRegister = ((uint64_t)destination) & 0x7F;

	uint64_t rawInsn = 0x0000000000000000 |
			   ( ((uint64_t)49) << (27 + ALIGN_RIGHT_SHIFT)) |
			   ( branchRegister << (13 + ALIGN_RIGHT_SHIFT)) |
			   ( generalRegister << (6 + ALIGN_RIGHT_SHIFT));

	return IA64_instruction( rawInsn );
	} /* end generateBranchToRegisterMove() */

IA64_instruction generateRegisterToBranchMove( Register source, Register destination ) {
	uint64_t generalRegister = ((uint64_t)source) & 0x7F;
	uint64_t branchRegister = ((uint64_t)destination) & 0x07;

	/* FIXME: Since I'm ignoring the prediction for now anyway, don't set
	   timm9, the address of the predicted branch insn. */
	uint64_t rawInsn = 0x0000000000000000 |
			   ( ((uint64_t)7) << (33 + ALIGN_RIGHT_SHIFT)) |
			   ( generalRegister << (13 + ALIGN_RIGHT_SHIFT)) |
			   ( branchRegister << (6 + ALIGN_RIGHT_SHIFT));

	return IA64_instruction( rawInsn );	
	} /* end generateRegisterToBranchMove() */

IA64_instruction generateShortImmediateAdd( Register destination, int immediate, Register source ) {
	uint64_t destinationRegister = ((uint64_t)destination) & 0x7F;
	uint64_t sourceRegister = ((uint64_t)source) & 0x7F;
	uint64_t immediate14 = (uint64_t)immediate & 0x3FFF;
	uint64_t signBit = immediate < 0 ? 1 : 0;

	uint64_t rawInsn = 0x0000000000000000 |
			   ( ((uint64_t)8) << (37 + ALIGN_RIGHT_SHIFT) ) |
			   ( ((uint64_t)2) << (34 + ALIGN_RIGHT_SHIFT) ) |
			   ( sourceRegister << (20 + ALIGN_RIGHT_SHIFT) ) |
			   ( destinationRegister << (6 + ALIGN_RIGHT_SHIFT) ) |
			   ( (immediate14 & RIGHT_IMM7B) << (13 + ALIGN_RIGHT_SHIFT) ) |
			   ( (immediate14 & RIGHT_IMM6D) << (-7 + 27 + ALIGN_RIGHT_SHIFT) ) |
			   ( signBit << (36 + ALIGN_RIGHT_SHIFT ) );

	return IA64_instruction( rawInsn );
	} /* end generateShortImmediateAdd() */

IA64_instruction generateSubtraction( Register destination, Register lhs, Register rhs ) {
	uint64_t destinationRegister = ((uint64_t)destination) & 0x7F;
	uint64_t lhsRegister = ((uint64_t)lhs) & 0x7F;
	uint64_t rhsRegister = ((uint64_t)rhs) & 0x7F;

	uint64_t rawInsn = 0x0000000000000000 |
			   ( ((uint64_t)8) << (37 + ALIGN_RIGHT_SHIFT) ) |
			   ( ((uint64_t)1) << (29 + ALIGN_RIGHT_SHIFT) ) |
			   ( ((uint64_t)1) << (27 + ALIGN_RIGHT_SHIFT) ) |
			   ( destinationRegister << (6 + ALIGN_RIGHT_SHIFT) ) |
			   ( lhsRegister << (13 + ALIGN_RIGHT_SHIFT) ) |
			   ( rhsRegister << (20 + ALIGN_RIGHT_SHIFT) );
			   
	return IA64_instruction( rawInsn );
	} /* end generateSubtraction() */

IA64_instruction generateIndirectCallTo( Register indirect, Register rp ) {
	uint64_t indirectRegister = ((uint64_t)indirect) & 0x7;
	uint64_t returnRegister = ((uint64_t)rp) & 0x7;

	uint64_t rawInsn = 0x0000000000000000 |
			   ( ((uint64_t)1) << (37 + ALIGN_RIGHT_SHIFT) ) |
			   ( indirectRegister << (13 + ALIGN_RIGHT_SHIFT) ) |
			   ( returnRegister << (6 + ALIGN_RIGHT_SHIFT) );

	return IA64_instruction( rawInsn );
	} /* end generateIndirectBranchTo() */

IA64_instruction generatePredicatesToRegisterMove( Register destination ) {
	uint64_t generalRegister = ((uint64_t)destination) & 0x7F;
	
	uint64_t rawInsn = 0x0000000000000000 |
			   ( ((uint64_t)51) << (27 + ALIGN_RIGHT_SHIFT) ) | // FIXME: 33?
			   ( generalRegister << (6 + ALIGN_RIGHT_SHIFT) );

	return IA64_instruction( rawInsn );
	} /* end generatePredicatesToRegisterMove() */

IA64_instruction generateRegisterToPredicatesMove( Register source, uint64_t mask17 ) {
	uint64_t generalRegister = ((uint64_t)source) & 0x7F;
	
	uint64_t rawInsn = 0x0000000000000000 |
			   ( (mask17 & 0x10000) << ( -16 + 36 + ALIGN_RIGHT_SHIFT ) ) |
			   ( ((uint64_t)3) << ( 33 + ALIGN_RIGHT_SHIFT ) ) |
			   ( ((mask17 >> 1) & RIGHT_IMM8C) << ( -7 + 24 + ALIGN_RIGHT_SHIFT ) ) |
			   ( ((mask17 >> 1) & RIGHT_IMM7A) << ( 6 + ALIGN_RIGHT_SHIFT ) ) |
			   ( generalRegister << ( 13 + ALIGN_RIGHT_SHIFT ) );
	
	return IA64_instruction( rawInsn );
	} /* end generateRegisterToPredicatesMove() */

IA64_instruction generateRegisterStore( Register address, Register source ) {
	uint64_t addressRegister = ((uint64_t)address) & 0x7F;
	uint64_t sourceRegister = ((uint64_t)source) & 0x7F;

	uint64_t rawInsn = 0x0000000000000000 |
			   ( ((uint64_t)4) << (37 + ALIGN_RIGHT_SHIFT) ) |
			   ( ((uint64_t)51) << (30 + ALIGN_RIGHT_SHIFT) ) |
			   ( addressRegister << (20 + ALIGN_RIGHT_SHIFT) ) |
			   ( sourceRegister << (13 + ALIGN_RIGHT_SHIFT) );

	return IA64_instruction( rawInsn );
	} /* end generateRegisterStore() */

IA64_instruction generateRegisterLoad ( Register destination, Register address ) { 
	uint64_t addressRegister = ((uint64_t)address) & 0x7F;
	uint64_t destinationRegister = ((uint64_t)destination) & 0x7F;

	uint64_t rawInsn = 0x0000000000000000 |
			   ( ((uint64_t)4) << (37 + ALIGN_RIGHT_SHIFT) ) |
			   ( ((uint64_t)3) << (30 + ALIGN_RIGHT_SHIFT) ) |
			   ( addressRegister << (20 + ALIGN_RIGHT_SHIFT) ) |
			   ( destinationRegister << (6 + ALIGN_RIGHT_SHIFT) );

	return IA64_instruction( rawInsn );
	} /* end generateRegisterLoad() */

