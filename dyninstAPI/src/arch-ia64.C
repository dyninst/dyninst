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

// $Id: arch-ia64.C,v 1.8 2002/07/02 21:07:16 tlmiller Exp $
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
#define IBRANCH_BTYPE		0x00000000C0000000	/* btis 06 - 08 */

#define RIGHT_IMM5		0x00000000001F0000	/* bits 16 - 20 */
#define RIGHT_IMM9		0x000000000000FF80	/* bits 07 - 15 */
#define RIGHT_IMM7		0x000000000000007F	/* bits 00 - 06 */
#define RIGHT_IMM_I		0x8000000000000000	/* bit 63 */
#define RIGHT_IMM_IC		0x0000000000200000	/* bit 21 */
#define RIGHT_IMM41		0x7FFFFFFFFFC00000	/* bits 22 - 62 */
#define RIGHT_IMM20		0x00000000000FFFFF	/* bits 00 - 19 */
#define RIGHT_IMM39		0x07FFFFFFFFF00000	/* bits 20 - 58 */

#define ALIGN_RIGHT_SHIFT 23

/* NOTE: for the IA64_bundle constructor to work, the individual
	instruction 'halves' should left-aligned as if they were independent instructions. */
IA64_instruction_x::IA64_instruction_x( uint64_t lowHalf = 0, uint64_t highHalf = 0, uint8_t templ = 0, IA64_bundle * mybl = 0 ) {
	instruction = lowHalf;
	instruction_x = highHalf;
	templateID = templ;
	myBundle = mybl;
	} /* end IA64_Instruction_x() */

IA64_instruction::IA64_instruction( uint64_t insn, uint8_t templ, IA64_bundle * mybl ) {
	instruction = insn;
	templateID = templ;
	myBundle = mybl;
	} /* end IA64_Instruction() */

const void * IA64_instruction::ptr() const { 
	/* If I don't have a bundle, trying to write me is pointless;
	   this should also make it harder for "unaware" functions to misuse me. */
	if( myBundle == NULL ) { return NULL; }

	return myBundle->getMyBundlePtr();
	} /* end ptr() */

IA64_bundle::IA64_bundle( ia64_bundle_t rawBundle ) {
	/* FIXME: what's the Right Way to do this? */
	* this = IA64_bundle( rawBundle.low, rawBundle.high );
	} /* end IA64_bundle() */

IA64_bundle::IA64_bundle( uint8_t templateID, IA64_instruction & instruction0, IA64_instruction instruction1, IA64_instruction instruction2 ) {
	/* FIXME: what's the Right Way to do this? */
	* this = IA64_bundle( templateID, instruction0.getMachineCode(), instruction1.getMachineCode(), instruction2.getMachineCode() );
	} /* end IA64_bundle() */

/* This handles the MLX template/long instructions. */
IA64_bundle::IA64_bundle( uint8_t templateID, IA64_instruction & instruction0, IA64_instruction_x & instructionLX ) {
	if( templateID != 0x05 ) { fprintf( stderr, "Attempting to generate a bundle with a long instruction without using the MLX template, aborting.\n" ); abort(); }

	/* FIXME: what's the Right Way to do this? */
	* this = IA64_bundle( templateID, instruction0, instructionLX.getMachineCode().low, instructionLX.getMachineCode().high );
	} /* end IA64_bundle() */

IA64_bundle::IA64_bundle( uint8_t templateID, uint64_t instruction0, uint64_t instruction1, uint64_t instruction2 ) {
	this->templateID = templateID;
	this->instruction0 = instruction0;
	this->instruction1 = instruction1;
	this->instruction2 = instruction2;

	myBundle.low =	( templateID & TEMPLATE_MASK ) |
			( (instruction0 >> (ALIGN_RIGHT_SHIFT - 5)) & INSTRUCTION0_MASK ) |
			( (instruction1 << 23) & INSTRUCTION1_LOW_MASK );
	myBundle.high = ( (instruction1 >> (ALIGN_RIGHT_SHIFT + 18)) & INSTRUCTION1_HIGH_MASK ) |
			( instruction2 & INSTRUCTION2_MASK );
	} /* end IA64_bundle() */

IA64_bundle::IA64_bundle( uint64_t lowHalfBundle, uint64_t highHalfBundle ) {
	/* The template is right-aligned; the instructions are left-aligned. */
	templateID = lowHalfBundle & TEMPLATE_MASK;
	instruction0 = IA64_instruction( (lowHalfBundle & INSTRUCTION0_MASK) << 18, templateID, this );
	instruction1 = IA64_instruction( ((lowHalfBundle & INSTRUCTION1_LOW_MASK) >> 23) +
						((highHalfBundle & INSTRUCTION1_HIGH_MASK) << 41), templateID, this );
	instruction2 = IA64_instruction( highHalfBundle & INSTRUCTION2_MASK, templateID, this );

	myBundle.low = lowHalfBundle;
	myBundle.high = highHalfBundle;
	} /* end IA64_Bundle() */

IA64_instruction IA64_bundle::getInstruction( unsigned int slot ) {
	switch( slot ) {
		case 0: return instruction0;
		case 1: return instruction1;
		case 2: return instruction2;
		default: fprintf( stderr, "Request of invalid instruction (%d), aborting.\n", slot ); abort();
		}
	} /* end getInstruction() */

/* For linux-ia64.C */
IA64_instruction generateAlteredAlloc( InsnAddr & allocAddr, int deltaLocal, int deltaOutput, int deltaRotate ) {
	uint64_t allocInsn = * allocAddr;

	/* Verify that the given instruction is actually an alloc. */
	// FIXME: check the template type and offset, too.
	if( allocInsn & MAJOR_OPCODE_MASK != ((uint64_t)1) << 37 + ALIGN_RIGHT_SHIFT ||
	    allocInsn & MEMORY_X3_MASK != ((uint64_t)6) << 33 + ALIGN_RIGHT_SHIFT ) {
		fprintf( stderr, "Alleged alloc instruction is not, aborting.\n" );
		abort();
		}

	/* Extract the local, output, and rotate sizes. */
	uint64_t allocatedLocal = (allocInsn & ALLOC_SOL) >> (20 + ALIGN_RIGHT_SHIFT);
	uint64_t allocatedOutput = ( (allocInsn & ALLOC_SOF) >> (13 + ALIGN_RIGHT_SHIFT) ) - allocatedLocal;
	uint64_t allocatedRotate = (allocInsn & ALLOC_SOR) >> (27 + ALIGN_RIGHT_SHIFT + 3);		// SOR = r >> 3
	
	/* Calculate the new sizes. */
	uint64_t generatedLocal = allocatedLocal + deltaLocal;
	uint64_t generatedOutput = allocatedOutput + deltaOutput;
	uint64_t generatedRotate = allocatedRotate + deltaRotate;

	/* Finally, alter allocInsn. */
	allocInsn = ( generatedLocal << (20 + ALIGN_RIGHT_SHIFT) ) | 
		    ( ( generatedRotate >> 3) << (27 + ALIGN_RIGHT_SHIFT) ) |
		    ( ( generatedLocal + generatedOutput ) << (13 + ALIGN_RIGHT_SHIFT) ) |
		    allocInsn & (~ALLOC_SOF) & (~ALLOC_SOR) & (~ALLOC_SOL);
	
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
			   ( (sImm22 & RIGHT_IMM7) << (13 + ALIGN_RIGHT_SHIFT)) |
			   ( (sImm22 & RIGHT_IMM5) << (-16 + 22 + ALIGN_RIGHT_SHIFT)) |
			   ( (sImm22 & RIGHT_IMM9) << (-7 + 27 + ALIGN_RIGHT_SHIFT)) |
			   ( (sRegisterN & RIGHT_IMM7) << (6 + ALIGN_RIGHT_SHIFT) );

	return IA64_instruction( rawInsn );
	} /* end generateConstantInRegister( imm22 ) */

IA64_instruction_x generateLongConstantInRegister( unsigned int registerN, long long int imm64 ) {
	uint64_t sRegisterN = (uint64_t)registerN;

	/* movl */
	uint64_t rawInsnHigh = 0x0000000000000000 | 
			   ( ((uint64_t)6) << (37 + ALIGN_RIGHT_SHIFT)) |
			   ( (imm64 & RIGHT_IMM_I) << (-63 + 36 + ALIGN_RIGHT_SHIFT)) |
			   ( (imm64 & RIGHT_IMM9) << (-7 + 27 + ALIGN_RIGHT_SHIFT)) |
			   ( (imm64 & RIGHT_IMM5) << (-16 + 22 + ALIGN_RIGHT_SHIFT)) |
			   ( (imm64 & RIGHT_IMM_IC) << (-21 + 21 + ALIGN_RIGHT_SHIFT)) |
			   ( (imm64 & RIGHT_IMM7) << (13 + ALIGN_RIGHT_SHIFT)) |
			   ( (sRegisterN & RIGHT_IMM7) << (6 + ALIGN_RIGHT_SHIFT) );
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
			   ( ((uint64_t)33) << (27 + ALIGN_RIGHT_SHIFT ) ) |  /* 21 ? */
			   ( ((uint64_t)4) << (6 + ALIGN_RIGHT_SHIFT ) ) |
			   ( ((uint64_t)1) << (12 + ALIGN_RIGHT_SHIFT ) ) |
			   ( (sBranchRegister & 0x7) << (13 + ALIGN_RIGHT_SHIFT) );

	return IA64_instruction( rawInsn );	
	} /* end generateReturnTo */

/* --- FIXME LINE --- */

/* Required by func-reloc.C to calculate relative displacements. */
int get_disp( instruction *insn ) {
	return 0;
	} /* end get_disp() */
                
/* Required by func-reloc.C to correct relative displacements after relocation. */
int set_disp( bool setDisp, instruction * insn, int newOffset, bool outOfFunc ) {
	return 0;
	} /* end set_disp() */

/* Convience methods for func-reloc.C */
int sizeOfMachineInsn( instruction * insn ) {
	return 0;
	} /* end sizeOfMachineInsn() */

int addressOfMachineInsn( instruction * insn ) {
	return 0;
	} /* end addressOfMachineInsn */
