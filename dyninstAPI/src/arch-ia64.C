/* -*- Mode: C; indent-tabs-mode: true; tab-width: 4 -*- */

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

// $Id: arch-ia64.C,v 1.30 2004/04/02 06:34:11 jaw Exp $
// ia64 instruction decoder

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include "dyninstAPI/src/arch-ia64.h"
#include "util.h"

#define TEMPLATE_MASK		0x000000000000001F	/* bits 00 - 04 */
#define INSTRUCTION0_MASK	0x00003FFFFFFFFFE0	/* bits 05 - 45 */
#define INSTRUCTION1_LOW_MASK	0xFFFFC00000000000	/* bits 45 - 63 */
#define INSTRUCTION1_HIGH_MASK	0x00000000007FFFFF	/* bits 00 - 20 */
#define INSTRUCTION2_MASK	0xFFFFFFFFFF800000	/* bits 21 - 63 */

#define BITS_30_35		0x07E0000000000000	/* bits 30 - 35 */
#define BITS_33_35		0x0700000000000000	/* bits 33 - 35 */
#define BITS_27_32		0x00FC000000000000	/* bits 27 - 32 */
#define BITS_06_12		0x0000000FE0000000	/* bits 06 - 12 */

#define BIT_36			0x0800000000000000	/* bit 36 */
#define BIT_27			0x0004000000000000	/* bit 27 */

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
#define IBRANCH_BTYPE	0x00000000E0000000	/* bits 06 - 08 */

#define RIGHT_IMM5C		0x00000000001F0000	/* bits 16 - 20 */
#define RIGHT_IMM9D		0x000000000000FF80	/* bits 07 - 15 */
#define RIGHT_IMM6D		0x0000000000001F80	/* bits 07 - 12 */
#define RIGHT_IMM7B		0x000000000000007F	/* bits 00 - 06 */
#define RIGHT_IMM_I		0x8000000000000000	/* bit 63 */
#define RIGHT_IMM_IC	0x0000000000200000	/* bit 21 */
#define RIGHT_IMM8C		0x0000000000007F80	/* bits 07 - 15 */
#define RIGHT_IMM7A		0x000000000000007F	/* bits 00 - 06 */

#define RIGHT_IMM41		0x7FFFFFFFFFC00000	/* bits 22 - 62 */
#define RIGHT_IMM20		0x00000000000FFFFF	/* bits 00 - 19 */
#define RIGHT_IMM39		0x07FFFFFFFFF00000	/* bits 20 - 58 */

IA64_instruction::unitType INSTRUCTION_TYPE_ARRAY[(0x20 + 1) * 3] = { 
	IA64_instruction::M, IA64_instruction::I, IA64_instruction::I,
	IA64_instruction::M, IA64_instruction::I, IA64_instruction::I,
	IA64_instruction::M, IA64_instruction::I, IA64_instruction::I,
	IA64_instruction::M, IA64_instruction::I, IA64_instruction::I,
	IA64_instruction::M, IA64_instruction::L, IA64_instruction::X,
	IA64_instruction::M, IA64_instruction::L, IA64_instruction::X,
	IA64_instruction::RESERVED, IA64_instruction::RESERVED, IA64_instruction::RESERVED,
	IA64_instruction::RESERVED, IA64_instruction::RESERVED, IA64_instruction::RESERVED,
	IA64_instruction::M, IA64_instruction::M, IA64_instruction::I,
	IA64_instruction::M, IA64_instruction::M, IA64_instruction::I,
	IA64_instruction::M, IA64_instruction::M, IA64_instruction::I,
	IA64_instruction::M, IA64_instruction::M, IA64_instruction::I,
	IA64_instruction::M, IA64_instruction::F, IA64_instruction::I,
	IA64_instruction::M, IA64_instruction::F, IA64_instruction::I,
	IA64_instruction::M, IA64_instruction::M, IA64_instruction::F,
	IA64_instruction::M, IA64_instruction::M, IA64_instruction::F,

	IA64_instruction::M, IA64_instruction::I, IA64_instruction::B,
	IA64_instruction::M, IA64_instruction::I, IA64_instruction::B,
	IA64_instruction::M, IA64_instruction::B, IA64_instruction::B,
	IA64_instruction::M, IA64_instruction::B, IA64_instruction::B,
	IA64_instruction::RESERVED, IA64_instruction::RESERVED, IA64_instruction::RESERVED,
	IA64_instruction::RESERVED, IA64_instruction::RESERVED, IA64_instruction::RESERVED,
	IA64_instruction::B, IA64_instruction::B, IA64_instruction::B,
	IA64_instruction::B, IA64_instruction::B, IA64_instruction::B,
	IA64_instruction::M, IA64_instruction::M, IA64_instruction::B,
	IA64_instruction::M, IA64_instruction::M, IA64_instruction::B,
	IA64_instruction::RESERVED, IA64_instruction::RESERVED, IA64_instruction::RESERVED,
	IA64_instruction::RESERVED, IA64_instruction::RESERVED, IA64_instruction::RESERVED,
	IA64_instruction::M, IA64_instruction::F, IA64_instruction::B,
	IA64_instruction::M, IA64_instruction::F, IA64_instruction::B,
	IA64_instruction::RESERVED, IA64_instruction::RESERVED, IA64_instruction::RESERVED,
	IA64_instruction::RESERVED, IA64_instruction::RESERVED, IA64_instruction::RESERVED,

	IA64_instruction::RESERVED, IA64_instruction::RESERVED, IA64_instruction::RESERVED,
	};


/* NOTE: for the IA64_bundle constructor to work, the individual
	instruction 'halves' should left-aligned as if they were independent instructions. */
IA64_instruction_x::IA64_instruction_x( uint64_t lowHalf, uint64_t highHalf, uint8_t templ ) {
	instruction = lowHalf;
	instruction_x = highHalf;
	templateID = templ;
	} /* end IA64_Instruction_x() */

IA64_instruction::IA64_instruction( uint64_t insn, uint8_t templ, uint8_t slotN ) {
	instruction = insn;
	templateID = templ;
	slotNumber = slotN;
	} /* end IA64_Instruction() */

const void * IA64_instruction::ptr() const { 
	return NULL;
	} /* end ptr() */

uint8_t IA64_instruction::getPredicate() const {
	return (instruction & PREDICATE_MASK) >> ALIGN_RIGHT_SHIFT;
	} /* end short instruction predication fetch */
	
uint8_t IA64_instruction_x::getPredicate() const {
	return (instruction_x & PREDICATE_MASK) >> ALIGN_RIGHT_SHIFT;
	} /* end long instruciton predication fetch */        

IA64_instruction::unitType IA64_instruction::getUnitType() const {
	return INSTRUCTION_TYPE_ARRAY[(templateID * 3) + slotNumber];
	} /* end getUnitType() */
	
IA64_instruction::insnType IA64_instruction::getType() const {
	/* We'll try to be a little smarter, now, and just look up the unit type. */
	unitType iUnitType = getUnitType();

	uint8_t opCode = (instruction & MAJOR_OPCODE_MASK) >> (ALIGN_RIGHT_SHIFT + 37);
	switch( iUnitType ) {
		case M: {
			/* Note that we do NOT recognize advance load instructions (see also isLoadOrStore()),
			   though this can be added without too much trouble. */
			uint8_t x6 = (instruction & BITS_30_35) >> (ALIGN_RIGHT_SHIFT + 30);
			uint8_t x3 = (instruction & BITS_33_35) >> (ALIGN_RIGHT_SHIFT + 33);
			uint8_t x = (instruction & BIT_27) >> (ALIGN_RIGHT_SHIFT + 27);
		
			switch( opCode ) {
				case 0x04:
					if( x == 0x00 && ( ( x6 >= 0x30 && x6 <= 0x37 ) || x6 == 0x3B ) ) { return INTEGER_STORE; }
					else if( x == 0x00 && ( ( x6 <= 0x17 ) || x6 == 0x1B || ( x6 >= 0x20 && x6 <= 0x2B ) ) ) { return INTEGER_LOAD; }
					else { return OTHER; }
					break;
					
				case 0x05:
					if( ( x6 <= 0x17 ) || x6 == 0x1B || ( x6 >= 0x20 && x6 <= 0x2B ) ) { return INTEGER_LOAD; }
					else if( ( x6 >= 0x30 && x6 <= 0x37 ) || x6 == 0x3B ) { return INTEGER_STORE; }
					break;
					
				case 0x06:
					if ( x == 0x00 && ( ( x6 <= 0x0F ) || x6 == 0x1B || ( x6 >= 0x20 && x6 <= 0x27 ) ) ) { return FP_LOAD; }
					else if( x == 0x00 && ( ( x6 >= 0x30 && x6 <= 0x32 ) || x6 == 0x3B ) ) { return FP_STORE; }
					else if( x == 0x01 && ( ( x6 >= 0x01 && x6 <= 0x27 ) && x6 != 0x04 && x6 != 0x08 && x6 != 0x0C && x6 != 0x20 && x6 != 0x24 ) ) { 
						if( x6 && 0x3 == 0x01 ) { return INTEGER_PAIR_LOAD; } else { return FP_PAIR_LOAD; }
						}
					else if( x == 0x00 && ( x6 == 0x2C || x6 == 0x2D || x6 == 0x2E || x6 == 0x2F ) ) { return PREFETCH; }
					else { return OTHER; }
					break;
				
				case 0x07:
					if ( x6 <= 0x0F || x6 == 0x1B || ( x6 >= 0x20 && x6 <= 0x27 ) ) { return FP_LOAD; }
					else if( ( x6 >= 0x30 && x6 <= 0x32 ) || x6 == 0x3B ) { return FP_STORE; }
					else if( x6 == 0x2C || x6 == 0x2D || x6 == 0x2E || x6 == 0x2F ) { return PREFETCH; }
					else { return OTHER; }
					break;
					
				case 0x01:
					if( x3 == 0x01 || x3 == 0x03 ) { return CHECK; }
					else if( x3 == 0x06 ) { return ALLOC; }
					else { return OTHER; }
					break;
					
				case 0x00:
					if( x3 >= 0x04 && x3 <= 0x07 ) { return CHECK; }
					else { return OTHER; }
					break;
					
				default:
					return OTHER;
				} /* end memory-unit opcode switch */
			} break;

		case I: {
			uint8_t x6 = (instruction & BITS_27_32) >> (ALIGN_RIGHT_SHIFT + 27);
			uint8_t x3 = (instruction & BITS_33_35) >> (ALIGN_RIGHT_SHIFT + 27);
		
			if( opCode == 0x00 && x6 == 0x30 ) { return MOVE_FROM_IP; }
			else if( opCode == 0x00 && x3 == 0x01 ) { return CHECK; }
			else { return OTHER; }
			} break;

		case B: {
			switch( opCode ) {
				case 0x05:
					return DIRECT_CALL;
				
				case 0x01:
					return INDIRECT_CALL;
					
				case 0x04:
					return DIRECT_BRANCH;
				
				case 0x00: {
					/* Is it a return or an indirect branch or something else? */
					uint8_t x6 = (instruction & IBRANCH_X6) >> (ALIGN_RIGHT_SHIFT + 27);
					uint8_t btype = (instruction & IBRANCH_BTYPE) >> (ALIGN_RIGHT_SHIFT + 6);
					
					if( x6 == 0x21 && btype == 0x04 ) { return RETURN; }
					else if( x6 == 0x20 && btype == 0x00 ) { return INDIRECT_BRANCH; }
					else if( x6 == 0x20 && btype == 0x01 ) { return BRANCH_IA; }
					else { return OTHER; }
					} break;
				
				case 0x07:
					return BRANCH_PREDICT;
					
				case 0x02: {
					uint8_t x6 = (instruction & IBRANCH_X6) >> (ALIGN_RIGHT_SHIFT + 27);
				
					if ( x6 == 0x10 || x6 == 0x11 ) { return BRANCH_PREDICT; }
					else { return OTHER; }
					} break;
				
				default:
					return OTHER;
				} /* end branch-unit opcode switch */
			} break; 

		case F:
			return OTHER;

		case L: case X:
			break;

		case RESERVED: default:
			break;	
		} /* end i-unit type switch */
	
	return INVALID;
	} /* end getType() */

IA64_instruction_x::unitType IA64_instruction_x::getUnitType() const { 
	return IA64_instruction_x::X;
	} /* end getUnitType() */

IA64_instruction::insnType IA64_instruction_x::getType() const {
	/* We know we're a long instruction, so just check the major opcode to see which one. */
	uint8_t opcode = (instruction_x & MAJOR_OPCODE_MASK) >> (ALIGN_RIGHT_SHIFT + 37);
	switch( opcode ) {
		case 0x0D: return DIRECT_CALL;
		case 0x0C: return DIRECT_BRANCH;
		default: return OTHER;
		} /* end opcode switch */
	} /* end getType() */

IA64_bundle::IA64_bundle( ia64_bundle_t rawBundle ) {
	* this = IA64_bundle( rawBundle.low, rawBundle.high );
	} /* end IA64_bundle() */

IA64_bundle::IA64_bundle( uint8_t templateID, const IA64_instruction & instruction0, const IA64_instruction instruction1, const IA64_instruction instruction2 ) {
	* this = IA64_bundle( templateID, instruction0.getMachineCode(), instruction1.getMachineCode(), instruction2.getMachineCode() );
	} /* end IA64_bundle() */

/* This handles the MLX template/long instructions. */
IA64_bundle::IA64_bundle( uint8_t templateID, const IA64_instruction & instruction0, const IA64_instruction_x & instructionLX ) {
	if( templateID != MLXstop && templateID != MLX ) { bpfatal( "Attempting to generate a bundle with a long instruction without using the MLX template, aborting.\n" ); abort(); }

	* this = IA64_bundle( templateID, instruction0, instructionLX.getMachineCode().low, instructionLX.getMachineCode().high );
	} /* end IA64_bundle() */

IA64_bundle::IA64_bundle( uint8_t templateID, uint64_t instruction0, uint64_t instruction1, uint64_t instruction2 ) {
	this->templateID = templateID;
	this->instruction0 = IA64_instruction( instruction0, templateID, 0 );
	this->instruction1 = IA64_instruction( instruction1, templateID, 1 );
	this->instruction2 = IA64_instruction( instruction2, templateID, 2 ); 

	myBundle.low =	( templateID & TEMPLATE_MASK ) |
			( (instruction0 >> (ALIGN_RIGHT_SHIFT - 5)) & INSTRUCTION0_MASK ) |
			( (instruction1 << 23) & INSTRUCTION1_LOW_MASK );
	myBundle.high = ( (instruction1 >> (ALIGN_RIGHT_SHIFT + 18)) & INSTRUCTION1_HIGH_MASK ) |
			( instruction2 & INSTRUCTION2_MASK );
	} /* end IA64_bundle() */

IA64_bundle::IA64_bundle( uint64_t lowHalfBundle, uint64_t highHalfBundle ) {
	/* The template is right-aligned; the instructions are left-aligned. */
	templateID = lowHalfBundle & TEMPLATE_MASK;
	instruction0 = IA64_instruction( (lowHalfBundle & INSTRUCTION0_MASK) << 18, templateID, 0 );
	instruction1 = IA64_instruction( ((lowHalfBundle & INSTRUCTION1_LOW_MASK) >> 23) +
						((highHalfBundle & INSTRUCTION1_HIGH_MASK) << 41), templateID, 1 );
	instruction2 = IA64_instruction( highHalfBundle & INSTRUCTION2_MASK, templateID, 2 );

	myBundle.low = lowHalfBundle;
	myBundle.high = highHalfBundle;
	} /* end IA64_Bundle() */

IA64_instruction_x IA64_bundle::getLongInstruction() {
	longInstruction = IA64_instruction_x( instruction1.getMachineCode(), instruction2.getMachineCode(), templateID );
	return longInstruction;
	} /* end getLongInstruction() */

IA64_instruction * IA64_bundle::getInstruction( unsigned int slot ) {
	if( (slot == 1 || slot == 2) && hasLongInstruction() ) {
		return new IA64_instruction_x( instruction1.getMachineCode(), instruction2.getMachineCode(), templateID );
		}
	switch( slot ) {
		case 0: return new IA64_instruction( instruction0 );
		case 1: return new IA64_instruction( instruction1 );
		case 2: return new IA64_instruction( instruction2 );
		default: bpfatal("Request of invalid instruction (%d), aborting.\n", slot ); abort();
		}
	} /* end getInstruction() */

// Aids bundle modification.  Used by set_breakpoint_for_syscall_completion().
bool IA64_bundle::setInstruction(IA64_instruction &newInst)
{
    if ( (templateID == 0x04 || templateID == 0x05) && newInst.slotNumber != 0)
	return false;

    switch (newInst.slotNumber) {
    case 0:
	instruction0 = IA64_instruction(newInst.instruction, templateID, newInst.slotNumber);
	myBundle.low &= ~INSTRUCTION0_MASK | (newInst.instruction << (ALIGN_RIGHT_SHIFT - 5));
	break;
    case 1:
	instruction1 = IA64_instruction(newInst.instruction, templateID, newInst.slotNumber);
	myBundle.low &= ~INSTRUCTION1_LOW_MASK | (newInst.instruction << 23);
	myBundle.high &= ~INSTRUCTION1_HIGH_MASK | (newInst.instruction >> (ALIGN_RIGHT_SHIFT + 18));
	break;
    case 2:
	instruction2 = IA64_instruction(newInst.instruction, templateID, newInst.slotNumber);
	myBundle.high &= ~INSTRUCTION2_MASK | newInst.instruction;
	break;
    default:
	return false;
	break;
    }
    return true;
} /* end setInstruction() */

// Aids bundle modification.  Added for completion.
bool IA64_bundle::setInstruction(IA64_instruction_x &newInst)
{
    if (templateID != 0x04 && templateID != 0x05)
	return false;

    instruction1 = IA64_instruction(newInst.instruction, templateID, 1);
    instruction2 = IA64_instruction(newInst.instruction_x, templateID, 2);

    myBundle.low &= ~INSTRUCTION1_LOW_MASK | (newInst.instruction << 23);
    myBundle.high = ( ( (newInst.instruction >> (ALIGN_RIGHT_SHIFT + 18)) & INSTRUCTION1_HIGH_MASK ) |
		      ( newInst.instruction_x & INSTRUCTION2_MASK ) );
    return true;
} /* end setInstruction(x) */

/* private refactoring function */
bool extractAllocatedRegisters( uint64_t allocInsn, uint64_t * allocatedLocal, uint64_t * allocatedOutput, uint64_t * allocatedRotate ) {
	/* Verify that the given instruction is actually, so far as we can tell
	   (we don't have the template and the offset), an alloc. */

	// GCC didn't seem to want to compile this the right way inline...
	uint64_t majorOpCode = allocInsn & MAJOR_OPCODE_MASK;
	uint64_t x3 = allocInsn & BITS_33_35;
	uint64_t opcodeOne = ((uint64_t)0x01) << (37 + ALIGN_RIGHT_SHIFT);
	uint64_t x3six = ((uint64_t)0x06) << (33 + ALIGN_RIGHT_SHIFT); 

	if( majorOpCode != opcodeOne || x3 != x3six ) {
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

IA64_instruction generateAllocInstructionFor( registerSpace * rs, int locals, int outputs, int rotates ) {
	uint64_t sizeOfLocals = rs->getRegSlot( 0 )->number - 32 + locals;
	uint64_t sizeOfFrame = sizeOfLocals + outputs;
	uint64_t sizeOfRotates = rotates >> 3;
	uint64_t ar_pfs = rs->getRegSlot( 0 )->number - NUM_PRESERVED;

	uint64_t rawInsn = 0x0000000000000000 |
		    ( ((uint64_t)0x01) << (37 + ALIGN_RIGHT_SHIFT) ) |
		    ( ((uint64_t)0x06) << (33 + ALIGN_RIGHT_SHIFT) ) |
		    ( sizeOfRotates << (27 + ALIGN_RIGHT_SHIFT) ) |
		    ( sizeOfLocals << (20 + ALIGN_RIGHT_SHIFT) ) | 
		    ( sizeOfFrame << (13 + ALIGN_RIGHT_SHIFT) ) | 
		    ( ar_pfs << (6 + ALIGN_RIGHT_SHIFT) );

	return IA64_instruction( rawInsn );
	} /* end generateAllocInstructionFor() */

IA64_instruction generateOriginalAllocFor( registerSpace * rs ) {
	/* Allocate a spurious output register so the ar.pfs doesn't overwrite
	   one of the registers we're trying to save. */
	uint64_t sizeOfLocals = rs->originalLocals;
	uint64_t sizeOfOutputs = rs->originalOutputs + 1;
	uint64_t sizeOfRotates = rs->originalRotates >> 3;
	uint64_t sizeOfFrame = sizeOfLocals + sizeOfOutputs;
	uint64_t ar_pfs = 32 + sizeOfFrame - 1;

	uint64_t rawInsn = 0x0000000000000000 |
		    ( ((uint64_t)0x01) << (37 + ALIGN_RIGHT_SHIFT) ) |
		    ( ((uint64_t)0x06) << (33 + ALIGN_RIGHT_SHIFT) ) |
		    ( sizeOfRotates << (27 + ALIGN_RIGHT_SHIFT) ) |
		    ( sizeOfLocals << (20 + ALIGN_RIGHT_SHIFT) ) | 
		    ( sizeOfFrame << (13 + ALIGN_RIGHT_SHIFT) ) | 
		    ( ar_pfs << (6 + ALIGN_RIGHT_SHIFT) );

	return IA64_instruction( rawInsn );
	} /* end generateOriginalAllocFor() */

/* imm22 is assumed to be right-aligned, e.g., an actual value. :) */
IA64_instruction generateShortConstantInRegister( unsigned int registerN, int imm22 ) {
	uint64_t sImm22 = (uint64_t)imm22;
	uint64_t sRegisterN = (uint64_t)registerN;
	
	/* addl */
	uint64_t rawInsn = 0x0000000000000000 | 
			   ( ((uint64_t)0x09) << (37 + ALIGN_RIGHT_SHIFT)) |
			   ( ((uint64_t)(imm22 < 0)) << (36 + ALIGN_RIGHT_SHIFT)) |
			   ( (sImm22 & RIGHT_IMM7B) << (13 + ALIGN_RIGHT_SHIFT)) |
			   ( (sImm22 & RIGHT_IMM5C) << (-16 + 22 + ALIGN_RIGHT_SHIFT)) |
			   ( (sImm22 & RIGHT_IMM9D) << (-7 + 27 + ALIGN_RIGHT_SHIFT)) |
			   ( (sRegisterN & RIGHT_IMM7B) << (6 + ALIGN_RIGHT_SHIFT) );

	return IA64_instruction( rawInsn );
	} /* end generateConstantInRegister( imm22 ) */

IA64_instruction_x generateLongConstantInRegister( unsigned int registerN, long long int immediate ) {
	uint64_t imm64 = (uint64_t)immediate;
	uint64_t sRegisterN = (uint64_t)registerN;
	uint64_t signBit = immediate < 0 ? 1 : 0;

	/* movl */
	uint64_t rawInsnHigh = 0x0000000000000000 | 
			   ( ((uint64_t)0x06) << (37 + ALIGN_RIGHT_SHIFT)) |
			   ( (signBit) << (36 + ALIGN_RIGHT_SHIFT)) |
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
			   ( ((uint64_t)0x0D) << (37 + ALIGN_RIGHT_SHIFT)) |
			   ( ((uint64_t)(displacement64 < 0)) << (36 + ALIGN_RIGHT_SHIFT)) |
			   ( (displacement60 & RIGHT_IMM20) << (13 + ALIGN_RIGHT_SHIFT)) |
			   ( (sBranchRegister & 0x07 ) << (6 + ALIGN_RIGHT_SHIFT));
	uint64_t rawInsnLow = 0x0000000000000000 |
			   ( (displacement60 & RIGHT_IMM39) << (-20 + 2 + ALIGN_RIGHT_SHIFT) );

	return IA64_instruction_x( rawInsnLow, rawInsnHigh );
	} /* end generateLongCallTo( displacement64 ) */

IA64_instruction_x generateLongBranchTo( long long int displacement64, Register predicate ) {
	int64_t displacement60 = displacement64 >> 4;

	uint64_t rawInsnHigh = 0x0000000000000000 | 
			   ( ((uint64_t)0x0C) << (37 + ALIGN_RIGHT_SHIFT)) |
			   ( ((uint64_t)(predicate & 0x3F)) << (0 + ALIGN_RIGHT_SHIFT)) |
			   ( ((uint64_t)(displacement64 < 0)) << (36 + ALIGN_RIGHT_SHIFT)) |
			   ( (displacement60 & RIGHT_IMM20) << (13 + ALIGN_RIGHT_SHIFT));
	uint64_t rawInsnLow = 0x0000000000000000 |
			   ( (displacement60 & RIGHT_IMM39) << (-20 + 2 + ALIGN_RIGHT_SHIFT) );

	return IA64_instruction_x( rawInsnLow, rawInsnHigh );
	} /* end generateLongBranchTo( displacement64 ) */

IA64_instruction generateReturnTo( unsigned int branchRegister ) {
	uint64_t sBranchRegister = (uint64_t)branchRegister;

	uint64_t rawInsn = 0x0000000000000000 |
			   ( ((uint64_t)0x21) << (27 + ALIGN_RIGHT_SHIFT ) ) | 
			   ( ((uint64_t)0x04) << (6 + ALIGN_RIGHT_SHIFT ) ) |
			   ( ((uint64_t)0x01) << (12 + ALIGN_RIGHT_SHIFT ) ) |
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
Address IA64_instruction::getTargetAddress() const {
	insnType myType = getType();
	if( myType == DIRECT_CALL || myType == DIRECT_BRANCH ) { /* Kind of pointless to guess at the target of indirect jumps. */
		uint8_t opcode = (instruction & MAJOR_OPCODE_MASK) >> (ALIGN_RIGHT_SHIFT + 37);
		switch( opcode ) {
			case 0x04: case 0x05: {
				/* ip-relative branch and call, respectively. */
				uint64_t imm20b = (instruction >> (13 + ALIGN_RIGHT_SHIFT) ) & RIGHT_IMM20;
				uint64_t signBit = (instruction >> (26 + ALIGN_RIGHT_SHIFT) ) & 0x1;
				return signExtend( signBit, imm20b ) << 4;
				} break;
			case 0x01: case 0x00:
				/* indirect branch and call, respectively. */
				assert( 0 );
				break;
			default:
				bpfatal( "getTargetAddress(): unrecognized major opcode, aborting.\n" );
				abort();
				break;
			} /* end opcode switch */	
		} else {
		// /* DEBUG */ bperr( "getTargetAddress() returning 0 for indirect branch or call.\n" );
		return 0;
		}
	} /* end getTargetAddress() */

Address IA64_instruction_x::getTargetAddress() const {
	if( getType() == DIRECT_CALL || getType() == DIRECT_BRANCH ) {
		assert( 0 );  // FIXME
		} else {
		return 0;
		}
	} /* end getTargetAddress() */

#include "instPoint-ia64.h"
bool defineBaseTrampRegisterSpaceFor( const instPoint * location, registerSpace * regSpace, Register * deadRegisterList ) {
	/* Handy for the second two cases. */
	Address fnEntryOffset = location->pointFunc()->get_address();
	Address fnEntryAddress = (Address)location->getOwner()->getPtrToInstruction( fnEntryOffset );
	assert( fnEntryAddress % 16 == 0 );
	const ia64_bundle_t * rawBundlePointer = (const ia64_bundle_t *) fnEntryAddress;

	/* Zero and one alloc intstruction are easily handled.  Special
	   case some more complicated common forms, like system calls,
	   and leave the rest to the dynamic basetramp. */
	pdvector< Address > allocs = location->pointFunc()->allocs;
	switch( allocs.size() ) {
		case 0: {
			/* Since we cannot know the current frame without static analysis,
			   create ours after the largest frame possible (8 local, 0 output). */
			for( int i = 0; i < NUM_LOCALS + NUM_OUTPUT; i++ ) {
				deadRegisterList[i] = 32 + 8 + i + NUM_PRESERVED;
				}

			/* Construct the registerSpace reflecting the desired frame. */
			* regSpace = registerSpace( NUM_LOCALS + NUM_OUTPUT, deadRegisterList, 0, NULL );

			/* Tell generateOriginalAllocFor() to generate the largest frame possible. */
			regSpace->originalLocals = 8;
			regSpace->originalOutputs = 0;
			regSpace->originalRotates = 0;

			/* Our static analysis succeeded. */
			} return true;

		case 1: {
			/* Where is our alloc instruction?  We need to have a look at it... */
			Address encodedAddress = (location->pointFunc()->allocs)[0];
			unsigned short slotNumber = encodedAddress % 16;
			Address alignedOffset = encodedAddress - slotNumber;
			IA64_bundle allocBundle = rawBundlePointer[ alignedOffset / 16 ];

			/* ... so we find out what the frame it generates looks like... */
			uint64_t allocatedLocals, allocatedOutputs, allocatedRotates;
			extractAllocatedRegisters( allocBundle.getInstruction( slotNumber )->getMachineCode(),
				& allocatedLocals, & allocatedOutputs, & allocatedRotates );
			uint64_t sizeOfFrame = allocatedLocals + allocatedOutputs;

			/* ... and construct a deadRegisterList and regSpace above the
			   registers the application's using. */
			for( int i = 0; i < NUM_LOCALS + NUM_OUTPUT; i++ ) {
				deadRegisterList[i] = 32 + sizeOfFrame + i + NUM_PRESERVED;
				}

			* regSpace = registerSpace( NUM_LOCALS + NUM_OUTPUT, deadRegisterList, 0, NULL );

			/* Note that we assume that having extra registers can't be harmful;
			   that is, that 'restoring' the alloc instruction's frame before
			   it executes does not change the semantics of the program.  AFAIK,
			   this will be true for all correct programs. */
			regSpace->originalLocals = allocatedLocals;
			regSpace->originalOutputs = allocatedOutputs;
			regSpace->originalRotates = allocatedRotates;

			/* Our static analysis succeeded. */
			} return true;

		default:
			return false;
		} /* end #-of-allocs switch. */
	} /* end dBTRSF() */

/* For inst-ia64.h */
IA64_instruction generateIPToRegisterMove( Register destination ) {
	uint64_t generalRegister = ((uint64_t)destination) & 0x7F;
	
	uint64_t rawInsn = 0x0000000000000000 |
			   ( ((uint64_t)0x30) << (27 + ALIGN_RIGHT_SHIFT) ) |
			   ( generalRegister << (6 + ALIGN_RIGHT_SHIFT) );

	return IA64_instruction( rawInsn );
	} /* end generateIPToRegisterMove() */

IA64_instruction generateBranchToRegisterMove( Register source, Register destination ) {
	uint64_t branchRegister = ((uint64_t)source) & 0x07;
	uint64_t generalRegister = ((uint64_t)destination) & 0x7F;

	uint64_t rawInsn = 0x0000000000000000 |
			   ( ((uint64_t)0x31) << (27 + ALIGN_RIGHT_SHIFT)) |
			   ( branchRegister << (13 + ALIGN_RIGHT_SHIFT)) |
			   ( generalRegister << (6 + ALIGN_RIGHT_SHIFT));

	return IA64_instruction( rawInsn );
	} /* end generateBranchToRegisterMove() */

IA64_instruction generateRegisterToBranchMove( Register source, Register destination ) {
	uint64_t generalRegister = ((uint64_t)source) & 0x7F;
	uint64_t branchRegister = ((uint64_t)destination) & 0x07;

	uint64_t rawInsn = 0x0000000000000000 |
			   ( ((uint64_t)0x07) << (33 + ALIGN_RIGHT_SHIFT)) |
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
			   ( ((uint64_t)0x08) << (37 + ALIGN_RIGHT_SHIFT) ) |
			   ( ((uint64_t)0x02) << (34 + ALIGN_RIGHT_SHIFT) ) |
			   ( sourceRegister << (20 + ALIGN_RIGHT_SHIFT) ) |
			   ( destinationRegister << (6 + ALIGN_RIGHT_SHIFT) ) |
			   ( (immediate14 & RIGHT_IMM7B) << (13 + ALIGN_RIGHT_SHIFT) ) |
			   ( (immediate14 & RIGHT_IMM6D) << (-7 + 27 + ALIGN_RIGHT_SHIFT) ) |
			   ( signBit << (36 + ALIGN_RIGHT_SHIFT ) );

	return IA64_instruction( rawInsn );
	} /* end generateShortImmediateAdd() */

IA64_instruction generateArithmetic( opCode op, Register destination, Register lhs, Register rhs ) {
	uint64_t destinationRegister = ((uint64_t)destination) & 0x7F;
	uint64_t lhsRegister = ((uint64_t)lhs) & 0x7F;
	uint64_t rhsRegister = ((uint64_t)rhs) & 0x7F;

	uint64_t x4, x2b;
	switch( op ) {
		case plusOp: x4 = 0; x2b = 0; break;
		case minusOp: x4 = 1; x2b = 1; break;
		case andOp: x4 = 3; x2b = 0; break;
		case orOp: x4 = 3; x2b = 2; break;
		default:
			bpfatal( "generateArithmetic() did not recognize opcode %d, aborting.\n", op );
			abort();
			break;
		} /* end op switch */

	uint64_t rawInsn = 0x0000000000000000 |
			   ( ((uint64_t)0x08) << (37 + ALIGN_RIGHT_SHIFT) ) |
			   ( x4 << (29 + ALIGN_RIGHT_SHIFT) ) |
			   ( x2b << (27 + ALIGN_RIGHT_SHIFT) ) |
			   ( destinationRegister << (6 + ALIGN_RIGHT_SHIFT) ) |
			   ( lhsRegister << (13 + ALIGN_RIGHT_SHIFT) ) |
			   ( rhsRegister << (20 + ALIGN_RIGHT_SHIFT) );
			   
	return IA64_instruction( rawInsn );
	} /* end generateArithmetic() */

IA64_instruction generateIndirectCallTo( Register indirect, Register rp ) {
	uint64_t indirectRegister = ((uint64_t)indirect) & 0x7;
	uint64_t returnRegister = ((uint64_t)rp) & 0x7;

	uint64_t rawInsn = 0x0000000000000000 |
			   ( ((uint64_t)0x01) << (37 + ALIGN_RIGHT_SHIFT) ) |
			   ( ((uint64_t)0x01) << (32 + ALIGN_RIGHT_SHIFT) ) |
			   ( indirectRegister << (13 + ALIGN_RIGHT_SHIFT) ) |
			   ( returnRegister << (6 + ALIGN_RIGHT_SHIFT) );

	return IA64_instruction( rawInsn );
	} /* end generateIndirectCallTo() */

IA64_instruction generatePredicatesToRegisterMove( Register destination ) {
	uint64_t generalRegister = ((uint64_t)destination) & 0x7F;
	
	uint64_t rawInsn = 0x0000000000000000 |
			   ( ((uint64_t)0x33) << (27 + ALIGN_RIGHT_SHIFT) ) |
			   ( generalRegister << (6 + ALIGN_RIGHT_SHIFT) );

	return IA64_instruction( rawInsn );
	} /* end generatePredicatesToRegisterMove() */

IA64_instruction generateRegisterToPredicatesMove( Register source, uint64_t mask17 ) {
	uint64_t generalRegister = ((uint64_t)source) & 0x7F;
	
	uint64_t rawInsn = 0x0000000000000000 |
			   ( (mask17 & 0x10000) << ( -16 + 36 + ALIGN_RIGHT_SHIFT ) ) |
			   ( ((uint64_t)0x03) << ( 33 + ALIGN_RIGHT_SHIFT ) ) |
			   ( ((mask17 >> 0x01) & RIGHT_IMM8C) << ( -7 + 24 + ALIGN_RIGHT_SHIFT ) ) |
			   ( ((mask17 >> 0x01) & RIGHT_IMM7A) << ( 6 + ALIGN_RIGHT_SHIFT ) ) |
			   ( generalRegister << ( 13 + ALIGN_RIGHT_SHIFT ) );
	
	return IA64_instruction( rawInsn );
	} /* end generateRegisterToPredicatesMove() */

#define BIT_30_35 0xFC0000000
#define NOT_BIT_30_35 (~ BIT_30_35)
IA64_instruction generateSpillTo( Register address, Register source, int64_t imm9 ) {
	IA64_instruction temp = generateRegisterStoreImmediate( address, source, imm9 );
	uint64_t rawInsn = temp.getMachineCode();
	rawInsn = rawInsn & (NOT_BIT_30_35 << ALIGN_RIGHT_SHIFT);
	rawInsn = rawInsn | ( ((uint64_t)0x3B) << (30 + ALIGN_RIGHT_SHIFT) );
	return IA64_instruction( rawInsn );
	} /* end generateSpillTo() */

IA64_instruction generateFillFrom( Register address, Register destination, int64_t imm9 ) {
	IA64_instruction temp = generateRegisterLoadImmediate( address, destination, imm9 );
	uint64_t rawInsn = temp.getMachineCode();
	rawInsn = rawInsn & (NOT_BIT_30_35 << ALIGN_RIGHT_SHIFT);
	rawInsn = rawInsn | ( ((uint64_t)0x1B) << (30 + ALIGN_RIGHT_SHIFT) );
	return IA64_instruction( rawInsn );
	} /* end generateFillFrom() */

IA64_instruction generateRegisterStore( Register address, Register source, int size, Register predicate ) {
	return generateRegisterStoreImmediate( address, source, 0, size, predicate );
	} /* generateRegisterStore() */

IA64_instruction generateRegisterStoreImmediate( Register address, Register source, int imm9, int size, Register predicate ) {
	uint64_t addressRegister = ((uint64_t)address) & 0x7F;
	uint64_t sourceRegister = ((uint64_t)source) & 0x7F;
	uint64_t immediate = ((uint64_t)imm9) & 0x7F;
	uint64_t signBit = imm9 < 0 ? 1 : 0;
	uint64_t iBit = (((uint64_t)imm9) & 0x080) >> 7;

	uint64_t sizeSpec = 0;
	switch( size ) {
		case 1: sizeSpec = 0x30; break;
		case 2: sizeSpec = 0x31; break;
		case 4: sizeSpec = 0x32; break;
		case 8: sizeSpec = 0x33; break;
		default:
			bpfatal( "Illegal size %d, aborting.\n", size );
			assert( 0 );
			break;
		} /* end sizeSpec determiner */

	uint64_t rawInsn = 0x0000000000000000 |
			   ( ((uint64_t)0x05) << (37 + ALIGN_RIGHT_SHIFT) ) |
			   ( signBit << (36 + ALIGN_RIGHT_SHIFT) ) |
			   ( ((uint64_t)sizeSpec) << (30 + ALIGN_RIGHT_SHIFT) ) |
			   ( iBit << (27 + ALIGN_RIGHT_SHIFT) ) |
			   ( addressRegister << (20 + ALIGN_RIGHT_SHIFT) ) |
			   ( sourceRegister << (13 + ALIGN_RIGHT_SHIFT) ) |
			   ( immediate << (6 + ALIGN_RIGHT_SHIFT) ) |
			   ( predicate << (0 + ALIGN_RIGHT_SHIFT) );

	return IA64_instruction( rawInsn );
	} /* end generateRegisterStore() */

/* This is the no-update form, which lets the code generator do dumb
   stuff like load from and into the same register. */
IA64_instruction generateRegisterLoad( Register destination, Register address, int size ) {
	uint64_t addressRegister = ((uint64_t)address) & 0x7F;
	uint64_t destinationRegister = ((uint64_t)destination) & 0x7F;

	uint64_t sizeSpec = 0;
	switch( size ) {
		case 1: sizeSpec = 0x00; break;
		case 2: sizeSpec = 0x01; break;
		case 4: sizeSpec = 0x02; break;
		case 8: sizeSpec = 0x03; break;
		default:
			bpfatal( "Illegal size %d, aborting.\n", size );
			assert( 0 );
			break;
		} /* end sizeSpec determiner */

	uint64_t rawInsn = 0x0000000000000000 |
			   ( ((uint64_t)0x04) << (37 + ALIGN_RIGHT_SHIFT) ) |
			   ( ((uint64_t)sizeSpec) << (30 + ALIGN_RIGHT_SHIFT) ) |
			   ( addressRegister << (20 + ALIGN_RIGHT_SHIFT) ) |
			   ( destinationRegister << (6 + ALIGN_RIGHT_SHIFT) );

	return IA64_instruction( rawInsn );	
	} /* end generateRegisterLoad() */

IA64_instruction generateRegisterLoadImmediate( Register destination, Register address, int imm9, int size ) { 
	uint64_t addressRegister = ((uint64_t)address) & 0x7F;
	uint64_t destinationRegister = ((uint64_t)destination) & 0x7F;
	uint64_t immediate = ((uint64_t)imm9) & 0x7F;
	uint64_t signBit = imm9 < 0 ? 1 : 0;
	uint64_t iBit = (((uint64_t)imm9) & 0x080) >> 7;

	uint64_t sizeSpec = 0;
	switch( size ) {
		case 1: sizeSpec = 0x00; break;
		case 2: sizeSpec = 0x01; break;
		case 4: sizeSpec = 0x02; break;
		case 8: sizeSpec = 0x03; break;
		default:
			bpfatal( "Illegal size %d, aborting.\n", size );
			assert( 0 );
			break;
		} /* end sizeSpec determiner */

	uint64_t rawInsn = 0x0000000000000000 |
			   ( ((uint64_t)0x05) << (37 + ALIGN_RIGHT_SHIFT) ) |
			   ( signBit << (36 + ALIGN_RIGHT_SHIFT) ) |
			   ( ((uint64_t)sizeSpec) << (30 + ALIGN_RIGHT_SHIFT) ) |
			   ( iBit << (27 + ALIGN_RIGHT_SHIFT) ) |
			   ( addressRegister << (20 + ALIGN_RIGHT_SHIFT) ) |
			   ( immediate << (13 + ALIGN_RIGHT_SHIFT) ) |
			   ( destinationRegister << (6 + ALIGN_RIGHT_SHIFT) );

	return IA64_instruction( rawInsn );
	} /* end generateRegisterLoad() */

IA64_instruction generateRegisterToApplicationMove( Register source, Register destination ) {
	uint64_t sourceRegister = ((uint64_t)source) & 0x7F;
	uint64_t destinationRegister = ((uint64_t)destination) & 0x7F;
	uint64_t rawInsn;

	/* The lower 48 application registers are only accessible via the M unit.  For simplicity,
	   divide responsibility at the sixty-fourth application register, with an I unit handling
	   the upper 64. */
	if( destination <= 63 ) {
		rawInsn = 0x0000000000000000 |
			   ( ((uint64_t)0x01) << (37 + ALIGN_RIGHT_SHIFT) ) |
			   ( ((uint64_t)0x2A) << (27 + ALIGN_RIGHT_SHIFT) ) |
			   ( sourceRegister << (13 + ALIGN_RIGHT_SHIFT) ) |
			   ( destinationRegister << (20 + ALIGN_RIGHT_SHIFT) );		
	} else {
		rawInsn = 0x0000000000000000 |
			   ( ((uint64_t)0x2A) << (27 + ALIGN_RIGHT_SHIFT) ) |
			   ( sourceRegister << (13 + ALIGN_RIGHT_SHIFT) ) |
			   ( destinationRegister << (20 + ALIGN_RIGHT_SHIFT) );
		} /* end if using I unit */

	return IA64_instruction( rawInsn );
	} /* end generateRegisterToApplicationMove() */

IA64_instruction generateApplicationToRegisterMove( Register source, Register destination ) {
	uint64_t sourceRegister = ((uint64_t)source) & 0x7F;
	uint64_t destinationRegister = ((uint64_t)destination) & 0x7F;
	uint64_t rawInsn;

	/* The lower 48 application registers are only accessible via the M unit.  For simplicity,
	   divide responsibility at the sixty-fourth application register, with an I unit handling
	   the upper 64. */
	if( source <= 63 ) {
		rawInsn = 0x0000000000000000 |
			   ( ((uint64_t)0x01) << (37 + ALIGN_RIGHT_SHIFT) ) |
			   ( ((uint64_t)0x22) << (27 + ALIGN_RIGHT_SHIFT) ) |
			   ( sourceRegister << (20 + ALIGN_RIGHT_SHIFT) ) |
			   ( destinationRegister << (6 + ALIGN_RIGHT_SHIFT) );
	} else {
		rawInsn = 0x0000000000000000 |
			   ( ((uint64_t)0x32) << (27 + ALIGN_RIGHT_SHIFT) ) |
			   ( sourceRegister << (20 + ALIGN_RIGHT_SHIFT) ) |
			   ( destinationRegister << (6 + ALIGN_RIGHT_SHIFT) );
		} /* end if using I unit */

	return IA64_instruction( rawInsn );
	} /* end generateRegisterToApplicationMove() */

IA64_instruction generateRegisterToRegisterMove( Register source, Register destination ) {
	return generateShortImmediateAdd( destination, 0, source ); }

IA64_instruction predicateInstruction( Register predicate, IA64_instruction insn ) {
	uint64_t predicateRegister = ((uint64_t)predicate) & 0x3F;

	uint64_t predicatesMaskedInsn = insn.getMachineCode() & (~(0x3f << ALIGN_RIGHT_SHIFT));
	uint64_t rawInsn = predicatesMaskedInsn |
			   predicateRegister << ALIGN_RIGHT_SHIFT;

	return IA64_instruction( rawInsn, insn.getTemplateID(), insn.getSlotNumber() );
	} /* end predicateInstruction() */

/* This was probably implemented somewhere else already.  Too bad. */
void swap( uint64_t & lhs, uint64_t & rhs ) {
	uint64_t temporary = lhs;
	lhs = rhs;
	rhs = temporary;
	} /* end swap() */

IA64_instruction generateComparison( opCode op, Register destination, Register lhs, Register rhs ) {
	uint64_t truePredicate = ((uint64_t)destination) & 0x3F; 
	uint64_t falsePredicate = truePredicate + 1;
	uint64_t lhsRegister = ((uint64_t)lhs) & 0x7F;
	uint64_t rhsRegister = ((uint64_t)rhs) & 0x7F;

	uint64_t machineCodeOp;
	
	/* We'll assume that all of our comparisons are signed. */
	switch( op ) {
		/* This gets cute.  The IA-64 hardware only implements the eq and lt ops,
		   so we get to do some argument and target rewriting to make things work.
	 	   The idea is to fall through the operations until we get to one that
		   can be implemented in hardware. */
	
		case greaterOp:
			/* Swap source registers; since we fall through geOp's
			   predicate register swap, swap them, too. */
			swap( truePredicate, falsePredicate );
		case leOp:
			/* Swap both source and predicate registers. */
			swap( lhsRegister, rhsRegister );
		case geOp:
			/* Swap predicate registers. */
			swap( truePredicate, falsePredicate );
		case lessOp:
			/* Generate a cmp.lt instruction. */
			machineCodeOp = 0x0C;
			break;

		case neOp:
			swap( truePredicate, falsePredicate );
		case eqOp:
			/* Generate a cmp.eq instruction. */
			machineCodeOp = 0x0E;
			break;

		default:
			bpfatal( "Unrecognized op %d in generateComparison(), aborting.\n", op );
			abort();
		} /* end op switch */

	uint64_t rawInsn = 0x0000000000000000 |
				( machineCodeOp << (37 + ALIGN_RIGHT_SHIFT) ) |
				( falsePredicate << (27 + ALIGN_RIGHT_SHIFT) ) |
				( rhsRegister << (20 + ALIGN_RIGHT_SHIFT) ) |
				( lhsRegister << (13 + ALIGN_RIGHT_SHIFT) ) |
				( truePredicate << (6 + ALIGN_RIGHT_SHIFT) );

	return IA64_instruction( rawInsn );
	} /* end generateComparison() */

IA64_instruction_x predicateLongInstruction( Register predicate, IA64_instruction_x insn ) {
	uint64_t predicateRegister = ((uint64_t)predicate) & 0x3F;

	uint64_t predicatesMaskedInsn = insn.getMachineCode().high & (~(0x3f << ALIGN_RIGHT_SHIFT));
	uint64_t highInsn = predicatesMaskedInsn |
			   predicateRegister << ALIGN_RIGHT_SHIFT;

	return IA64_instruction_x( insn.getMachineCode().low, highInsn, insn.getTemplateID() );
	} /* end predicateLongInstruction() */

IA64_instruction generateFPSpillTo( Register address, Register source, int64_t imm9 ) {
	uint64_t addressRegister = ((uint64_t)address) & 0x7F;
	uint64_t sourceRegister = ((uint64_t)source) & 0x7F;
	uint64_t immediate = ((uint64_t)imm9) & 0x7F;
	uint64_t signBit = imm9 < 0 ? 1 : 0;
	uint64_t iBit = (((uint64_t)imm9) & 0x080) >> 7;

	uint64_t rawInsn = 0x0000000000000000 |
			   ( ((uint64_t)0x07) << (37 + ALIGN_RIGHT_SHIFT) ) |
			   ( (signBit) << (36 + ALIGN_RIGHT_SHIFT) ) |
			   ( ((uint64_t)0x3B) << (30 + ALIGN_RIGHT_SHIFT) ) |
			   ( (iBit) << (27 + ALIGN_RIGHT_SHIFT) ) |
			   ( (addressRegister) << (20 + ALIGN_RIGHT_SHIFT) ) |
			   ( (sourceRegister) << (13 + ALIGN_RIGHT_SHIFT) ) |
			   ( (immediate) << (6 + ALIGN_RIGHT_SHIFT) );

	return IA64_instruction( rawInsn );
	} /* end generateFPSpillTo() */

IA64_instruction generateFPFillFrom( Register address, Register destination, int64_t imm9 ) {
	uint64_t addressRegister = ((uint64_t)address) & 0x7F;
	uint64_t destinationRegister = ((uint64_t)destination) & 0x7F;
	uint64_t immediate = (uint64_t)imm9 & 0x7F;
	uint64_t signBit = imm9 < 0 ? 1 : 0;
	uint64_t iBit = ((uint64_t)imm9 & 0x080) >> 7;

	uint64_t rawInsn = 0x0000000000000000 |
			   ( ((uint64_t)0x07) << (37 + ALIGN_RIGHT_SHIFT) ) |
			   ( (signBit) << (36 + ALIGN_RIGHT_SHIFT) ) |
			   ( ((uint64_t)0x1B) << (30 + ALIGN_RIGHT_SHIFT) ) |
			   ( (iBit) << (27 + ALIGN_RIGHT_SHIFT) ) |
			   ( (addressRegister) << (20 + ALIGN_RIGHT_SHIFT) ) |
			   ( (immediate) << (13 + ALIGN_RIGHT_SHIFT) ) |
			   ( (destinationRegister) << (6 + ALIGN_RIGHT_SHIFT) );

	return IA64_instruction( rawInsn );
	} /* end generateFPFillFrom() */

IA64_instruction generateRegisterToFloatMove( Register source, Register destination ) {
	uint64_t sourceRegister = ((uint64_t)source) & 0x7F;
	uint64_t destinationRegister = ((uint64_t)destination) & 0x7F;
	
	uint64_t rawInsn = 0x0000000000000000 |
			   ( ((uint64_t)0x06) << (37 + ALIGN_RIGHT_SHIFT) ) |
			   ( ((uint64_t)0x1C) << (30 + ALIGN_RIGHT_SHIFT) ) |
			   ( ((uint64_t)0x01) << (27 + ALIGN_RIGHT_SHIFT) ) |
			   ( sourceRegister << (13 + ALIGN_RIGHT_SHIFT) ) |
			   ( destinationRegister << (6 + ALIGN_RIGHT_SHIFT) );

	return IA64_instruction( rawInsn );
	} /* end generateRegisterToFloatMove() */

IA64_instruction generateFloatToRegisterMove( Register source, Register destination ) {
	uint64_t sourceRegister = ((uint64_t)source) & 0x7F;
	uint64_t destinationRegister = ((uint64_t)destination) & 0x7F;
	
	uint64_t rawInsn = 0x0000000000000000 |
			   ( ((uint64_t)0x04) << (37 + ALIGN_RIGHT_SHIFT) ) |
			   ( ((uint64_t)0x1C) << (30 + ALIGN_RIGHT_SHIFT) ) |
			   ( ((uint64_t)0x01) << (27 + ALIGN_RIGHT_SHIFT) ) |
			   ( sourceRegister << (13 + ALIGN_RIGHT_SHIFT) ) |
			   ( destinationRegister << (6 + ALIGN_RIGHT_SHIFT) );

	return IA64_instruction( rawInsn );
	} /* end generateFloatToRegisterMove() */

IA64_instruction generateFixedPointMultiply( Register destination, Register lhs, Register rhs ) {
	uint64_t destinationRegister = ((uint64_t)destination) & 0x7F;
	uint64_t lhsRegister = ((uint64_t)lhs) & 0x7F;
	uint64_t rhsRegister = ((uint64_t)rhs) & 0x7F;

	/* FIXME: We're assuming unsigned, and that the lower 64 bits are more interesting,
		  but this may well not be the case. */
	uint64_t rawInsn = 0x0000000000000000 |
			   ( ((uint64_t)0x0E) << (37 + ALIGN_RIGHT_SHIFT) ) |
			   ( ((uint64_t)0x01) << (36 + ALIGN_RIGHT_SHIFT) ) |
			   ( destinationRegister << (6 + ALIGN_RIGHT_SHIFT) ) |
			   ( lhsRegister << (20 + ALIGN_RIGHT_SHIFT) ) |
			   ( rhsRegister << (27 + ALIGN_RIGHT_SHIFT) );

	return IA64_instruction( rawInsn );
	} /* end generateFixedPointMultiply() */

void alterLongMoveAtTo( Address target, Address imm64 ) {
	ia64_bundle_t rawBundle = *( ia64_bundle_t *)target;

	uint64_t daRegister = rawBundle.high & 
				( 0x7F << (6 + ALIGN_RIGHT_SHIFT) );
	rawBundle.high = daRegister | 
			( ((uint64_t)0x06) << (37 + ALIGN_RIGHT_SHIFT)) |
			( (imm64 & RIGHT_IMM_I) << (-63 + 36 + ALIGN_RIGHT_SHIFT)) |
			( (imm64 & RIGHT_IMM9D) << (-7 + 27 + ALIGN_RIGHT_SHIFT)) |
			( (imm64 & RIGHT_IMM5C) << (-16 + 22 + ALIGN_RIGHT_SHIFT)) |
			( (imm64 & RIGHT_IMM_IC) << (-21 + 21 + ALIGN_RIGHT_SHIFT)) |
			( (imm64 & RIGHT_IMM7B) << (13 + ALIGN_RIGHT_SHIFT));
			
	uint64_t daTemplate = rawBundle.low & 0x3F;
	rawBundle.low = daTemplate |
			( (imm64 & RIGHT_IMM41) << (-22 + ALIGN_RIGHT_SHIFT) );

	*(ia64_bundle_t *)target = rawBundle;
	} /* end alterLongMoveAtTo() */

IA64_instruction generateShortImmediateBranch( int64_t target25 ) {
	uint64_t signBit = target25 < 0 ? 1 : 0;
	uint64_t immediate = ( ((uint64_t)target25) >> 4 ) & 0xFFFFF;

	uint64_t rawInsn = 0x0000000000000000 |
			( ((uint64_t)0x04) << (37 + ALIGN_RIGHT_SHIFT)) |
			( signBit << (36 + ALIGN_RIGHT_SHIFT) ) |
			( immediate << (13 + ALIGN_RIGHT_SHIFT) ) |
			( ((uint64_t)0x0) << (6 + ALIGN_RIGHT_SHIFT));

	return IA64_instruction( rawInsn );
	} /* end generateShortImmediateBranch() */
