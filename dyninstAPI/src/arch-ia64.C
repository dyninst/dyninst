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

// $Id: arch-ia64.C,v 1.46 2005/08/25 22:45:11 bernat Exp $
// ia64 instruction decoder

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include "dyninstAPI/src/arch-ia64.h"
#include "util.h"

#define ALIGN_RIGHT_SHIFT		23
#define TEMPLATE_MASK			0x000000000000001F	/* bits 00 - 04 */
#define INSTRUCTION0_MASK		0x00003FFFFFFFFFE0	/* bits 05 - 45 */
#define INSTRUCTION1_LOW_MASK	0xFFFFC00000000000	/* bits 45 - 63 */
#define INSTRUCTION1_HIGH_MASK	0x00000000007FFFFF	/* bits 00 - 20 */
#define INSTRUCTION2_MASK		0xFFFFFFFFFF800000	/* bits 21 - 63 */
#define SYSCALL_IMM				0x100000

instruction::unitType INSTRUCTION_TYPE_ARRAY[(0x20 + 1) * 3] = { 
	instruction::M, instruction::I, instruction::I,
	instruction::M, instruction::I, instruction::I,
	instruction::M, instruction::I, instruction::I,
	instruction::M, instruction::I, instruction::I,
	instruction::M, instruction::L, instruction::X,
	instruction::M, instruction::L, instruction::X,
	instruction::RESERVED, instruction::RESERVED, instruction::RESERVED,
	instruction::RESERVED, instruction::RESERVED, instruction::RESERVED,
	instruction::M, instruction::M, instruction::I,
	instruction::M, instruction::M, instruction::I,
	instruction::M, instruction::M, instruction::I,
	instruction::M, instruction::M, instruction::I,
	instruction::M, instruction::F, instruction::I,
	instruction::M, instruction::F, instruction::I,
	instruction::M, instruction::M, instruction::F,
	instruction::M, instruction::M, instruction::F,

	instruction::M, instruction::I, instruction::B,
	instruction::M, instruction::I, instruction::B,
	instruction::M, instruction::B, instruction::B,
	instruction::M, instruction::B, instruction::B,
	instruction::RESERVED, instruction::RESERVED, instruction::RESERVED,
	instruction::RESERVED, instruction::RESERVED, instruction::RESERVED,
	instruction::B, instruction::B, instruction::B,
	instruction::B, instruction::B, instruction::B,
	instruction::M, instruction::M, instruction::B,
	instruction::M, instruction::M, instruction::B,
	instruction::RESERVED, instruction::RESERVED, instruction::RESERVED,
	instruction::RESERVED, instruction::RESERVED, instruction::RESERVED,
	instruction::M, instruction::F, instruction::B,
	instruction::M, instruction::F, instruction::B,
	instruction::RESERVED, instruction::RESERVED, instruction::RESERVED,
	instruction::RESERVED, instruction::RESERVED, instruction::RESERVED,

	instruction::RESERVED, instruction::RESERVED, instruction::RESERVED,
	};

/* NOTE: for the IA64_bundle constructor to work, the individual
	instruction 'halves' should left-aligned as if they were independent instructions. */
instruction_x::instruction_x( uint64_t lowHalf, uint64_t highHalf, uint8_t templ ) {
	insn_ = lowHalf;
	insn_x_ = highHalf;
	templateID = templ;
} /* end IA64_Instruction_x() */

instruction::instruction( uint64_t insn, uint8_t templ, uint8_t slotN ) {
	insn_ = insn;
	templateID = templ;
	slotNumber = slotN;
} /* end IA64_Instruction() */

const void * instruction::ptr() const { 
	return & insn_;
	} /* end ptr() */
	
const void * instruction_x::ptr() const { 
	return & insn_x_;
	} /* end ptr() */

uint8_t instruction::getPredicate() const {
	return GET_PREDICATE( (const insn_tmpl *)(&insn_) );
} /* end short instruction predication fetch */

uint8_t instruction_x::getPredicate() const {
	return GET_PREDICATE( (const insn_tmpl *)(&insn_x_) );
	} /* end long instruciton predication fetch */

instruction::unitType instruction::getUnitType() const {
	return INSTRUCTION_TYPE_ARRAY[(templateID * 3) + slotNumber];
	} /* end getUnitType() */

instruction::insnType instruction::getType() const {
	/* We'll try to be a little smarter, now, and just look up the unit type. */
	insn_tmpl tmpl = { insn_ };
	uint8_t opCode = GET_OPCODE( &tmpl );

	switch( getUnitType() ) {
		case M: {
			/* Note that we do NOT recognize advance load instructions (see also isLoadOrStore()),
			   though this can be added without too much trouble. */
			uint8_t x  = tmpl.M_LD_ST.x;
			uint8_t m  = tmpl.M_LD_ST.m;
			uint8_t x6 = tmpl.M_LD_ST.x6;
			uint8_t x4 = tmpl.M_SYS.x2;
			uint8_t x3 = tmpl.M_SYS.x3;
			uint8_t x2 = tmpl.M_SYS.x4;

			switch( opCode ) {
				case 0x0:
					if( x3 >= 0x4 && x3 <= 0x7 ) return ALAT_CHECK;
					if( x3 == 0x0 && x4 == 0x0 && x2 == 0x0 )
						if( GET_M37_IMM( &tmpl ) == SYSCALL_IMM )
							return SYSCALL;
						else
							return BREAK;

					return OTHER;
					break;

				case 0x1:
					if( x3 == 0x1 || x3 == 0x3 ) return SPEC_CHECK;
					if( x3 == 0x06 ) return ALLOC;

					return OTHER;
					break;

				case 0x4:
					if( x == 0x0 ) {
						if( ( x6 <= 0x17 ) || x6 == 0x1B ||
							( x6 >= 0x20 && x6 <= 0x2B ) ) {
							// /* DEBUG */ fprintf( stderr, "%s[%d]: INTEGER_LOAD\n", __FILE__, __LINE__ );
							return INTEGER_LOAD;
							}

						if( ( x6 >= 0x30 && x6 <= 0x37 ) || x6 == 0x3B )
							return INTEGER_STORE;
					}

					if( m == 0x0 && x == 0x1 ) {
						if( x6 == 0x28 || x6 == 0x2C ) {
							// /* DEBUG */ fprintf( stderr, "%s[%d]: INTEGER_16_LOAD\n", __FILE__, __LINE__ );
							return INTEGER_16_LOAD;
							}
						if( x6 == 0x30 || x6 == 0x34 ) return INTEGER_16_STORE;
					}

					return OTHER;
					break;

				case 0x5:
					if( ( x6 <= 0x17 ) || x6 == 0x1B ||
						( x6 >= 0x20 && x6 <= 0x2B ) ) {
						// / * DEBUG */ fprintf( stderr, "%s[%d]: INTEGER_LOAD\n", __FILE__, __LINE__ );
						return INTEGER_LOAD;
						}

					if( ( x6 >= 0x30 && x6 <= 0x37 ) || x6 == 0x3B )
						return INTEGER_STORE;

					return OTHER;
					break;

				case 0x6:
					if( x == 0x0 ) {
						if( ( x6 <= 0x0F ) || x6 == 0x1B ||
							( x6 >= 0x20 && x6 <= 0x27 ) )
							return FP_LOAD;

						if( m == 0x0 && ( ( x6 >= 0x30 && x6 <= 0x33 ) || x6 == 0x3B ) )
							return FP_STORE;

						if( x6 == 0x2C || x6 == 0x2D || x6 == 0x2E || x6 == 0x2F )
							return PREFETCH;
					}

					if( x == 0x1 ) {
						if( ( x6 >= 0x01 && x6 <= 0x0F ) || ( x6 >= 0x21 && x6 <= 0x27 ) )
								 switch ( x6 & 0x3 ) {
								 	case 0x1:
										// /* DEBUG */ fprintf( stderr, "%s[%d]: INTEGER_PAIR_LOAD\n", __FILE__, __LINE__ );
								 		return INTEGER_PAIR_LOAD;
								 	case 0x2:
								 	case 0x3: return FP_PAIR_LOAD;
								 }
					}
					return OTHER;
					break;

				case 0x7:
					if( ( x6 <= 0x0F ) || x6 == 0x1B ||
						( x6 >= 0x20 && x6 <= 0x27 ) )
						return FP_LOAD;

					if( ( x6 >= 0x30 && x6 <= 0x33 ) || x6 == 0x3B )
						return FP_STORE;

					if( x6 >= 0x2C && x6 <= 0x2F )
						return PREFETCH;

					return OTHER;
					break;

				default:
					return OTHER;
				} /* end memory-unit opcode switch */
			} break;

		case I: {
			uint8_t x6 = tmpl.I_MISC.x6;
			uint8_t x3 = tmpl.I_MISC.x3;

			if( opCode == 0x0 && x3 == 0x0 && x6 == 0x00 )
				if( GET_I19_IMM( &tmpl ) == SYSCALL_IMM )
					return SYSCALL;
				else
					return BREAK;
			if( opCode == 0x0 && x6 == 0x30 ) return MOVE_FROM_IP;
			if( opCode == 0x0 && x3 == 0x1 ) return SPEC_CHECK;

			return OTHER;
			} break;

		case B: {
			switch( opCode ) {
				case 0x0: {
					/* Is it a return or an indirect branch or something else? */
					uint8_t x6 = tmpl.B.x6;
					uint8_t btype = tmpl.B.btype;

					if( x6 == 0x00 )
						if( GET_B9_IMM( &tmpl ) == SYSCALL_IMM )
							return SYSCALL;
						else
							return BREAK;
					if( x6 == 0x21 && btype == 0x4 ) return RETURN;
					if( x6 == 0x20 && btype == 0x0 ) return INDIRECT_BRANCH;
					if( x6 == 0x20 && btype == 0x1 ) return BRANCH_IA;

					return OTHER;
					} break;

				case 0x1: return INDIRECT_CALL;
				case 0x2: {
					uint8_t x6 = tmpl.B.x6;

					if ( x6 == 0x10 || x6 == 0x11 ) return BRANCH_PREDICT;

					return OTHER;
					} break;

				case 0x4: return DIRECT_BRANCH;
				case 0x5: return DIRECT_CALL;
				case 0x7: return BRANCH_PREDICT;

				default:  return OTHER;
				} /* end branch-unit opcode switch */
			} break;

		case F:
			if( opCode == 0x0 && tmpl.F15.x == 0x0 && tmpl.F15.x6 == 0x00 )
				if( GET_F15_IMM( &tmpl ) == SYSCALL_IMM )
					return SYSCALL;
				else
					return BREAK;

			return OTHER;

		case X:
		case L:
		case RESERVED:
		default: break;
		} /* end i-unit type switch */

	return INVALID;
	} /* end getType() */

instruction_x::unitType instruction_x::getUnitType() const { 
	return instruction_x::X;
	} /* end getUnitType() */

instruction::insnType instruction_x::getType() const {
	/* We know we're a long instruction, so just check the major opcode to see which one. */
	insn_tmpl tmpl = { insn_x_ };
	insn_tmpl imm  = { insn_ };

	switch( GET_OPCODE( &tmpl )) {
		case 0x0:
			if( tmpl.X1.x3 == 0x0 && tmpl.X1.x6 == 0x00 )
				if( GET_X1_IMM( &tmpl, &imm ) == SYSCALL_IMM )
					return SYSCALL;
				else
					return BREAK;

			return OTHER;

		case 0xD: return DIRECT_CALL;
		case 0xC: return DIRECT_BRANCH;
		default: return OTHER;
		} /* end opcode switch */
	} /* end getType() */

IA64_bundle::IA64_bundle( ia64_bundle_t rawBundle ) {
	* this = IA64_bundle( rawBundle.low, rawBundle.high );
	} /* end IA64_bundle() */

IA64_bundle::IA64_bundle( uint8_t templateID, const instruction & instruction0, const instruction instruction1, const instruction instruction2 ) {
	* this = IA64_bundle( templateID, instruction0.getMachineCode(), instruction1.getMachineCode(), instruction2.getMachineCode() );
	} /* end IA64_bundle() */

/* This handles the MLX template/long instructions. */
IA64_bundle::IA64_bundle( uint8_t templateID, const instruction & instruction0, const instruction_x & instructionLX ) {
	if( templateID != MLXstop && templateID != MLX ) { bpfatal( "Attempting to generate a bundle with a long instruction without using the MLX template, aborting.\n" ); abort(); }

	* this = IA64_bundle( templateID, instruction0, instructionLX.getMachineCode().low, instructionLX.getMachineCode().high );
	} /* end IA64_bundle() */

IA64_bundle::IA64_bundle( uint8_t templateID, uint64_t instruction0, uint64_t instruction1, uint64_t instruction2 ) {
	this->templateID = templateID;
	this->instruction0 = instruction( instruction0, templateID, 0 );
	this->instruction1 = instruction( instruction1, templateID, 1 );
	this->instruction2 = instruction( instruction2, templateID, 2 ); 

	myBundle.low  =	(( templateID & TEMPLATE_MASK ) |
					 ( (instruction0 >> (ALIGN_RIGHT_SHIFT - 5)) & INSTRUCTION0_MASK ) |
					 ( (instruction1 << 23) & INSTRUCTION1_LOW_MASK ));
	myBundle.high = (( (instruction1 >> (ALIGN_RIGHT_SHIFT + 18)) & INSTRUCTION1_HIGH_MASK ) |
					 ( (instruction2 & INSTRUCTION2_MASK )));
	} /* end IA64_bundle() */

IA64_bundle::IA64_bundle( uint64_t lowHalfBundle, uint64_t highHalfBundle ) {
	/* The template is right-aligned; the instructions are left-aligned. */
	templateID = lowHalfBundle & TEMPLATE_MASK;
	instruction0 = instruction( (lowHalfBundle & INSTRUCTION0_MASK) << 18, templateID, 0 );
	instruction1 = instruction( ((lowHalfBundle & INSTRUCTION1_LOW_MASK) >> 23) +
									 ((highHalfBundle & INSTRUCTION1_HIGH_MASK) << 41), templateID, 1 );
	instruction2 = instruction( highHalfBundle & INSTRUCTION2_MASK, templateID, 2 );

	myBundle.low = lowHalfBundle;
	myBundle.high = highHalfBundle;

} /* end IA64_Bundle() */

instruction_x IA64_bundle::getLongInstruction() {
	longInstruction = instruction_x( instruction1.getMachineCode(), instruction2.getMachineCode(), templateID );
	return longInstruction;
	} /* end getLongInstruction() */

instruction * IA64_bundle::getInstruction( unsigned int slot ) {
	if( (slot == 1 || slot == 2) && hasLongInstruction() ) {
		return new instruction_x( instruction1.getMachineCode(), instruction2.getMachineCode(), templateID );
		}
	switch( slot ) {
		case 0: return new instruction( instruction0 );
		case 1: return new instruction( instruction1 );
		case 2: return new instruction( instruction2 );
		default: bpfatal("Request of invalid instruction (%d), aborting.\n", slot ); abort();
		}
	} /* end getInstruction() */

// Aids bundle modification.  Used by set_breakpoint_for_syscall_completion().
bool IA64_bundle::setInstruction(instruction &newInst)
{
    if ( (templateID == 0x04 || templateID == 0x05) && newInst.slotNumber != 0)
		return false;

    switch (newInst.slotNumber) {
		case 0:
			instruction0 = instruction(newInst.insn_, templateID, newInst.slotNumber);
			myBundle.low &= ~INSTRUCTION0_MASK | (newInst.insn_ << (ALIGN_RIGHT_SHIFT - 5));
			break;
		case 1:
			instruction1 = instruction(newInst.insn_, templateID, newInst.slotNumber);
			myBundle.low &= ~INSTRUCTION1_LOW_MASK | (newInst.insn_ << 23);
			myBundle.high &= ~INSTRUCTION1_HIGH_MASK | (newInst.insn_ >> (ALIGN_RIGHT_SHIFT + 18));
			break;
		case 2:
			instruction2 = instruction(newInst.insn_, templateID, newInst.slotNumber);
			myBundle.high &= ~INSTRUCTION2_MASK | newInst.insn_;
			break;
		default:
			return false;
			break;
    }
    return true;
} /* end setInstruction() */

// Aids bundle modification.  Added for completion.
bool IA64_bundle::setInstruction(instruction_x &newInst)
{
    if (templateID != 0x04 && templateID != 0x05)
	return false;

    instruction1 = instruction(newInst.insn_, templateID, 1);
    instruction2 = instruction(newInst.insn_x_, templateID, 2);

    myBundle.low &= ~INSTRUCTION1_LOW_MASK | (newInst.insn_ << 23);
    myBundle.high = ( ((newInst.insn_ >> (ALIGN_RIGHT_SHIFT + 18)) & INSTRUCTION1_HIGH_MASK ) |
					  ( newInst.insn_x_ & INSTRUCTION2_MASK ) );
    return true;
} /* end setInstruction(x) */

/* private refactoring function */
bool extractAllocatedRegisters( uint64_t allocInsn, uint64_t * allocatedLocal, uint64_t * allocatedOutput, uint64_t * allocatedRotate ) {
	/* Verify that the given instruction is actually, so far as we can tell
	   (we don't have the template and the offset), an alloc. */

	insn_tmpl alloc = { allocInsn };
	if (alloc.M34.opcode != 0x1 || alloc.M34.x3 != 0x6) {
		*allocatedLocal = *allocatedOutput = *allocatedRotate = 0;
		return false;
	} /* end if not an alloc instruction */

	/* Extract the local, output, and rotate sizes. */
	*allocatedLocal = GET_M34_LOCAL(&alloc);
	*allocatedOutput = GET_M34_OUTPUT(&alloc);
	*allocatedRotate = GET_M34_ROTATE(&alloc);

	/* Completed successfully. */
	return true;
	} /* end extractAllocatedRegisters() */

instruction generateAllocInstructionFor( registerSpace * rs, int locals, int outputs, int rotates ) {
	insn_tmpl alloc = { 0x0 };
	uint64_t sizeOfLocals = rs->getRegSlot( 0 )->number - 32 + locals;

	if( sizeOfLocals + outputs > 96 ) {
		// Never allocate a frame larger than 96 registers.
		sizeOfLocals = 96 - outputs;
	}

	alloc.M34.opcode	= 0x1;
	alloc.M34.x3		= 0x6;
	alloc.M34.r1		= rs->originalLocals + rs->originalOutputs + 32;
	SET_M34_FIELDS(&alloc, sizeOfLocals, outputs, rotates);

	return instruction( alloc.raw );
	} /* end generateAllocInstructionFor() */

instruction generateOriginalAllocFor( registerSpace * rs ) {
	insn_tmpl alloc = { 0x0 };

	alloc.M34.opcode	= 0x1;
	alloc.M34.x3		= 0x6;
	alloc.M34.r1		= rs->originalLocals + rs->originalOutputs + 32;

	/* Allocate a spurious output register so the ar.pfs doesn't overwrite
	   one of the registers we're trying to save. */
	SET_M34_FIELDS(&alloc, rs->originalLocals, rs->originalOutputs + 1, rs->originalRotates);

	return instruction( alloc.raw );
	} /* end generateOriginalAllocFor() */

/* imm22 is assumed to be right-aligned, e.g., an actual value. :) */
instruction generateShortConstantInRegister( unsigned int registerN, int imm22 ) {
	insn_tmpl addl = { 0x0 };

	addl.A5.opcode	= 0x9;
	addl.A5.r1		= registerN;
	SET_A5_IMM(&addl, imm22);

	return instruction( addl.raw );
	} /* end generateConstantInRegister( imm22 ) */

instruction_x generateLongConstantInRegister( unsigned int registerN, long long int immediate ) {
	insn_tmpl movl = { 0x0 }, imm = { 0x0 };

	movl.X2.opcode	= 0x6;
	movl.X2.r1		= registerN;
	SET_X2_IMM(&movl, &imm, immediate);

	return instruction_x( imm.raw, movl.raw );
	} /* end generateConstantInRegister( imm64 ) */

instruction_x generateLongCallTo( long long int displacement64, unsigned int branchRegister, Register predicate ) {
	insn_tmpl call = { 0x0 }, imm = { 0x0 };

	call.X4.opcode	= 0xD;
	call.X4.b1		= branchRegister;
	call.X4.qp		= predicate;
	SET_X4_TARGET(&call, &imm, displacement64);

	return instruction_x( imm.raw, call.raw );
	} /* end generateLongCallTo( displacement64 ) */

instruction_x generateLongBranchTo( long long int displacement64, Register predicate ) {
	insn_tmpl brl = { 0x0 }, imm = { 0x0 };
	brl.X3.opcode	= 0xC;
	brl.X3.qp		= predicate;
	SET_X3_TARGET(&brl, &imm, displacement64);

	return instruction_x( imm.raw, brl.raw );
	} /* end generateLongBranchTo( displacement64 ) */

instruction generateReturnTo( unsigned int branchRegister ) {
	insn_tmpl ret = { 0x0 };

	/* Ret Opcode	= 0x0 */
	ret.B4.x6		= 0x21;
	ret.B4.btype	= 0x4;
	ret.B4.p		= 0x1;
	ret.B4.b2		= branchRegister;

	return instruction( ret.raw );
	} /* end generateReturnTo */

/* Required by func-reloc.C to calculate relative displacements. */
int get_disp( instruction * /* insn */ ) {
	assert( 0 );
	return 0;
	} /* end get_disp() */

/* Required by func-reloc.C to correct relative displacements after relocation. */
int set_disp( bool /* setDisp */, instruction * /* insn */, int /* newOffset */, bool /* outOfFunc */ ) {
	assert( 0 );
	return 0;
	} /* end set_disp() */

/* Convience methods for func-reloc.C */
int sizeOfMachineInsn( instruction * /* insn */ ) {
	assert( 0 );
	return 0;
	} /* end sizeOfMachineInsn() */

int addressOfMachineInsn( instruction * /* insn */ ) {
	assert( 0 );
	return 0;
	} /* end addressOfMachineInsn */

/* Convience method for inst-ia64.C */
IA64_bundle generateBundleFromLongInstruction( instruction_x longInstruction ) {
	instruction memoryNOP( NOP_M );
	return IA64_bundle( MLXstop, memoryNOP, longInstruction );
	} /* end generateBundleFromLongInstruction() */

/* Required by inst-ia64.C */
Address instruction::getTargetAddress() const {
	insnType myType = getType();
	insn_tmpl tmpl = { insn_ };

	if( myType == DIRECT_CALL || myType == DIRECT_BRANCH ) { /* Kind of pointless to guess at the target of indirect jumps. */
		switch( GET_OPCODE(&tmpl) ) {
			case 0x00: /* Indirect call and branch, respectively. */
			case 0x01: assert( 0 );
			case 0x04: return GET_B1_TARGET(&tmpl);
			case 0x05: return GET_B3_TARGET(&tmpl);
			default:
				bpfatal( "getTargetAddress(): unrecognized major opcode, aborting.\n" );
				abort();
				break;
			} /* end opcode switch */
		} else {
		// /* DEBUG */ bperr( "getTargetAddress() returning 0 for indirect branch or call.\n" );
		}
	return 0;
	} /* end getTargetAddress() */

Address instruction_x::getTargetAddress() const {
	insnType myType = getType();
	insn_tmpl tmpl = { insn_x_ }, imm = { insn_ };

	if (myType == DIRECT_CALL || myType == DIRECT_BRANCH ) {
		switch (GET_OPCODE(&tmpl)) {
			case 0xC: return GET_X3_TARGET(&tmpl, &imm);
			case 0xD: return GET_X4_TARGET(&tmpl, &imm);
		}
	}
	return 0;
	} /* end getTargetAddress() */

#include "process.h"
#include "function.h"
#include <list>

/* private refactoring function, for dBTRSF() */
int_basicBlock * findBasicBlockInCFG( Address addr, 
									  const pdvector<int_basicBlock *> &blocks) {
	for (unsigned i = 0; i < blocks.size(); i++) {
		if ((blocks[i]->origInstance()->firstInsnAddr() <= addr) &&
			(addr < blocks[i]->origInstance()->endAddr()))
			return blocks[i];
	}
	return NULL;
} /* end findBasicBlockInCFG() */

void initBaseTrampStorageMap( registerSpace *regSpace, int sizeOfFrame, bool *usedFPregs )
{
	// Clear the data structures.
	regSpace->sizeOfStack = 0;
	memset( regSpace->storageMap, 0, sizeof( regSpace->storageMap ) );

	// Unstacked register save locations
	int stackIndex = 32 + sizeOfFrame;
	if( stackIndex > 128 - ( NUM_PRESERVED + NUM_LOCALS + NUM_OUTPUT ) )
		stackIndex = 128 - ( NUM_PRESERVED + NUM_LOCALS + NUM_OUTPUT );

	regSpace->storageMap[ BP_AR_PFS   ] = stackIndex++;

	regSpace->storageMap[ BP_GR0 +  1 ] = stackIndex++;
	regSpace->storageMap[ BP_GR0 +  2 ] = stackIndex++;
	regSpace->storageMap[ BP_GR0 +  3 ] = stackIndex++;

	regSpace->storageMap[ BP_GR0 +  8 ] = stackIndex++;
	regSpace->storageMap[ BP_GR0 +  9 ] = stackIndex++;
	regSpace->storageMap[ BP_GR0 + 10 ] = stackIndex++;
	regSpace->storageMap[ BP_GR0 + 11 ] = stackIndex++;
	regSpace->storageMap[ BP_GR0 + 12 ] = stackIndex++;

	regSpace->storageMap[ BP_GR0 + 14 ] = stackIndex++;
	regSpace->storageMap[ BP_GR0 + 15 ] = stackIndex++;
	regSpace->storageMap[ BP_GR0 + 16 ] = stackIndex++;
	regSpace->storageMap[ BP_GR0 + 17 ] = stackIndex++;
	regSpace->storageMap[ BP_GR0 + 18 ] = stackIndex++;
	regSpace->storageMap[ BP_GR0 + 19 ] = stackIndex++;
	regSpace->storageMap[ BP_GR0 + 20 ] = stackIndex++;
	regSpace->storageMap[ BP_GR0 + 21 ] = stackIndex++;
	regSpace->storageMap[ BP_GR0 + 22 ] = stackIndex++;
	regSpace->storageMap[ BP_GR0 + 23 ] = stackIndex++;
	regSpace->storageMap[ BP_GR0 + 24 ] = stackIndex++;
	regSpace->storageMap[ BP_GR0 + 25 ] = stackIndex++;
	regSpace->storageMap[ BP_GR0 + 26 ] = stackIndex++;
	regSpace->storageMap[ BP_GR0 + 27 ] = stackIndex++;
	regSpace->storageMap[ BP_GR0 + 28 ] = stackIndex++;
	regSpace->storageMap[ BP_GR0 + 29 ] = stackIndex++;
	regSpace->storageMap[ BP_GR0 + 30 ] = stackIndex++;
	regSpace->storageMap[ BP_GR0 + 31 ] = stackIndex++;

	regSpace->storageMap[ BP_AR_CCV   ] = stackIndex++;
	regSpace->storageMap[ BP_AR_CSD   ] = stackIndex++;
	regSpace->storageMap[ BP_AR_SSD   ] = stackIndex++;
	regSpace->storageMap[ BP_BR0 +  0 ] = stackIndex++;
	regSpace->storageMap[ BP_BR0 +  6 ] = stackIndex++;
	regSpace->storageMap[ BP_BR0 +  7 ] = stackIndex++;
	regSpace->storageMap[ BP_PR       ] = stackIndex++;

	// Stacked register save locations, if needed.
	// Stacked registers are always saved on the memory stack.
	stackIndex = 0;
	for( int i = 128 - (NUM_PRESERVED + NUM_LOCALS + NUM_OUTPUT); i < (32 + sizeOfFrame); ++i )
		regSpace->storageMap[ BP_GR0 + i ] = --stackIndex;

	int stackCount = 0;
	if( stackIndex < 0 ) {
		stackCount = -stackIndex * 8;
		stackCount += stackCount % 16; // Align stack to 0x10 boundry.
	}

	if( usedFPregs ) {
		for( int i = 0; i < 128; ++i )
			if( usedFPregs[ i ] ) stackCount += 16;

	} else {
		stackCount += 16 * 106;
	}

	/* The runtime conventions require a 16-byte scratch area
	   above the SP for function calls.  Since we're assuming
	   function calls will be made from instrumentation, and
	   will thus always have a subtraction here, go ahead and
	   subtract an additional 16 bytes so we can spill two
	   floating-point registers to do multiplication. */
	regSpace->sizeOfStack = stackCount + 32;
}

#include "instPoint.h"
#include "process.h"

extern bool *doFloatingPointStaticAnalysis( const instPoint * );

bool defineBaseTrampRegisterSpaceFor( const instPoint * location, 
									  registerSpace *regSpace, 
									  Register * deadRegisterList) {
	/* If no alloc's definition reaches the instPoint _location_, create a base tramp
	   register space compatible with any possible leaf function.

	   If exactly one alloc's definition reaches the instPoint _location_, create a
	   base tramp by extending the frame created by that alloc.

	   If more than alloc's definition reaches the instPoint _location_, return false,
	   because we can't statically determine the register frame that will be active
	   when the instrumentation at _location_ executes. */

	int_function * pdf = location->func();
	assert( pdf != NULL );
	
	/* Determine used FP regs, if needed */
	if( !pdf->usedFPregs )
		pdf->usedFPregs = doFloatingPointStaticAnalysis( location );
	
	const pdvector<int_basicBlock *> &blocks = pdf->blocks();

	/* Initialize the dataflow sets and construct the initial worklist. */
	std::list< int_basicBlock * > workList;
	for (unsigned bIter = 0; bIter < blocks.size(); bIter++) {
		int_basicBlock * basicBlock = blocks[bIter];
		basicBlock->setDataFlowIn( new BPatch_Set< int_basicBlock * > );
		basicBlock->setDataFlowOut( new BPatch_Set< int_basicBlock * > );
		basicBlock->setDataFlowGen( NULL );
		basicBlock->setDataFlowKill( NULL );

		workList.push_back( basicBlock );
	} /* end initialization iteration over all basic blocks */

	/* Initialize the alloc blocks. */
	for( unsigned int i = 0; i < pdf->allocs.size(); i++ ) {
		Address absoluteAddress = pdf->allocs[i];
		// /* DEBUG */ fprintf( stderr, "%s[%d]: absolute address of alloc: 0x%lx (in function starting at 0x%lx)\n", __FILE__, __LINE__, absoluteAddress, pdf->getAddress() );
		int_basicBlock * currentAlloc = 
			findBasicBlockInCFG( absoluteAddress, blocks);
		
		/* The old parser uses the frequently-incorrect symbol table size information,
		   so we can get allocs in unreachable basic blocks.  Since they're unreachable, 
		   the CFG doesn't create them and we can't find them.  */
		if( currentAlloc == NULL ) { continue; }
		/* Switch back to me when the new parser arrives. */
		// assert( currentAlloc != NULL );
		
		/* Generically, these should be functors from sets to sets. */
		currentAlloc->setDataFlowGen( currentAlloc );
		currentAlloc->setDataFlowKill( currentAlloc );
	} /* end initialization iteration over all allocs. */

	/* Start running the worklist. */
	while( ! workList.empty() ) {
		int_basicBlock * workBlock = workList.front();
		workList.pop_front();
		// /* DEBUG */ fprintf( stderr, "Working on basicBlock %p\n", workBlock );

		/* Construct workBlock's new output set from workBlock's immediate predecessors.  If
		   it's different from workBlock's old output set, add all of workBlock's successors
		   to the workList. */
		BPatch_Set< int_basicBlock * > newOutputSet;
		pdvector< int_basicBlock * > predecessors;
		workBlock->getSources( predecessors );
		for( unsigned int i = 0; i < predecessors.size(); i++ ) {
			int_basicBlock * predecessor = predecessors[i];
			newOutputSet |= * predecessor->getDataFlowOut();
		} /* end iteration over predecessors */
		
		// /* DEBUG */ fprintf( stderr, "From %d predecessors, %d allocs in input set.\n", predecessors.size(), newOutputSet.size() );
		
		if( workBlock->getDataFlowKill() != NULL ) {
			/* Special case for allocs: any non-NULL kill set kills everything.  Otherwise, you'd
			   have to use an associative set for kill and gen. */
			newOutputSet = BPatch_Set<int_basicBlock *>();
		}

		if( workBlock->getDataFlowGen() != NULL ) {	
			newOutputSet.insert(workBlock->getDataFlowGen() ); 
		}

		// /* DEBUG */ fprintf( stderr, "After gen/kill sets, %d in (new) output set.\n", newOutputSet.size() );

		if( newOutputSet != *workBlock->getDataFlowOut() ) {
			// /* DEBUG */ fprintf( stderr, "New output set different, adding successors:" );
			* workBlock->getDataFlowOut() = newOutputSet;

			pdvector< int_basicBlock * > successors;
			workBlock->getTargets( successors );
			
			for( unsigned int i = 0; i < successors.size(); i++ ) {
				// /* DEBUG */ fprintf( stderr, " %p", successors[i] );
				workList.push_back( successors[i] );
			} /* end iteration over successors */
			// /* DEBUG */ fprintf( stderr, "\n" );
		} /* end if the output set changed. */
	} /* end iteration over worklist. */

	// /* DEBUG */ fprintf( stderr, "%s[%d]: absolute address of location: 0x%lx\n", __FILE__, __LINE__, location->addr());
	int numAllocs = 0;
	bool success = true;
	BPatch_Set< int_basicBlock * > * reachingAllocs = NULL;
	int_basicBlock * locationBlock = findBasicBlockInCFG( location->addr(), blocks );
	if( locationBlock ) {
		reachingAllocs = locationBlock->getDataFlowOut();
		numAllocs = reachingAllocs->size();
	}
	// /* DEBUG */ fprintf( stderr, "%s[%d]: %d reaching allocs located.\n", __FILE__, __LINE__, numAllocs );

	switch( numAllocs ) {
		case 0: {
			// /* DEBUG */ fprintf( stderr, "%s[%d]: no reaching allocs located.\n", __FILE__, __LINE__ );
			
			/* The largest possible unallocated frame (by the ABI, for leaf
			   functions) is 8 input registers. */
			for( int i = 0; i < NUM_LOCALS + NUM_OUTPUT; i++ ) {
				deadRegisterList[i] = 32 + 8 + i + NUM_PRESERVED;
				}

			/* Construct the registerSpace reflecting the desired frame. */
			* regSpace = registerSpace( NUM_LOCALS + NUM_OUTPUT, deadRegisterList, 0, NULL );
			initBaseTrampStorageMap( regSpace, 8, pdf->usedFPregs );

			/* If we did not have a frame originally, create one such that wrapper functions
			   will work correctly. */
			regSpace->originalLocals = 0;
			regSpace->originalOutputs = 8;
			regSpace->originalRotates = 0;

			/* Our static analysis succeeded. */
			} break;
			
		case 1: {			
			/* Where is our alloc instruction?  We need to have a look at it... */
			int_basicBlock * allocBlock = * reachingAllocs->begin();
			// /* DEBUG */ fprintf( stderr, "%s[%d]: reaching alloc at 0x%lx\n", __FILE__, __LINE__, allocBlock->firstInsnAddr() );
			
			Address encodedAddress = allocBlock->origInstance()->firstInsnAddr();
			unsigned short slotNumber = encodedAddress % 16;
			Address alignedOffset = encodedAddress - pdf->getAddress() - slotNumber;
			
			Address fnEntryOffset = pdf->getAddress();
			Address fnEntryAddress = (Address)location->proc()->getPtrToInstruction(fnEntryOffset);
			assert( fnEntryAddress % 16 == 0 );
			const ia64_bundle_t * rawBundlePointer = (const ia64_bundle_t *) fnEntryAddress;
			IA64_bundle allocBundle = rawBundlePointer[ alignedOffset / 16 ];

			/* ... so we find out what the frame it generates looks like... */
			uint64_t allocatedLocals, allocatedOutputs, allocatedRotates;
			extractAllocatedRegisters( allocBundle.getInstruction( slotNumber )->getMachineCode(),
				& allocatedLocals, & allocatedOutputs, & allocatedRotates );
			uint64_t sizeOfFrame = allocatedLocals + allocatedOutputs;

			/* ... and construct a deadRegisterList and regSpace above the
			   registers the application's using. */

			// Insure that deadRegisterList fits within the 128 general register pool.
			int baseReg = 32 + sizeOfFrame + NUM_PRESERVED;
			if( baseReg > 128 - (NUM_LOCALS + NUM_OUTPUT) )
				baseReg = 128 - (NUM_LOCALS + NUM_OUTPUT);

			for( int i = 0; i < NUM_LOCALS + NUM_OUTPUT; i++ ) {
				deadRegisterList[i] = baseReg + i;
				}

			* regSpace = registerSpace( NUM_LOCALS + NUM_OUTPUT, deadRegisterList, 0, NULL );
			initBaseTrampStorageMap( regSpace, sizeOfFrame, pdf->usedFPregs );

			/* Note that we assume that having extra registers can't be harmful;
			   that is, that 'restoring' the alloc instruction's frame before
			   it executes does not change the semantics of the program.  AFAIK,
			   this will be true for all correct programs. */
			regSpace->originalLocals = allocatedLocals;
			regSpace->originalOutputs = allocatedOutputs;
			regSpace->originalRotates = allocatedRotates;

			/* Our static analysis succeeded. */
			} break;
			
		default:
			// /* DEBUG */ fprintf( stderr, "%s[%d]: more than one (%d) allocs reached.\n", __FILE__, __LINE__, numAllocs );
			success = false;
			break;
		} /* end #-of-dominating-allocs switch */

	/* Regardless, clean up. */
	for(unsigned bIter = 0; bIter < blocks.size(); bIter++) {
		int_basicBlock *block = blocks[bIter];
		delete (block->getDataFlowOut());
		delete (block->getDataFlowIn());
		block->setDataFlowIn(NULL);
		block->setDataFlowOut(NULL);
		block->setDataFlowGen(NULL);
		block->setDataFlowKill(NULL);

	} /* end iteration over all blocks. */	

	return success;
} /* end defineBaseTrampRegisterSpace() */

/* For inst-ia64.h */
instruction generateRegisterToRegisterMove( Register source, Register destination ) {
	return generateShortImmediateAdd( destination, 0, source ); }

instruction generateIPToRegisterMove( Register destination ) {
	insn_tmpl mov = { 0x0 };

	/* Mov Opcode	= 0x0 */
	mov.I25.x6		= 0x30;
	mov.I25.r1		= destination;

	return instruction( mov.raw );
	} /* end generateIPToRegisterMove() */

instruction generateBranchToRegisterMove( Register source, Register destination ) {
	insn_tmpl mov = { 0x0 };

	mov.I22.x6 = 0x31;
	mov.I22.b2 = source;
	mov.I22.r1 = destination;

	return instruction( mov.raw );
	} /* end generateBranchToRegisterMove() */

instruction generateRegisterToBranchMove( Register source, Register destination, int immediate ) {
	insn_tmpl mov = { 0x0 };

	mov.I21.x3		= 0x7;
	mov.I21.r2		= source;
	mov.I21.b1		= destination;
	mov.I21.timm9c	= immediate;

	return instruction( mov.raw );	
	} /* end generateRegisterToBranchMove() */

instruction generateShortImmediateAdd( Register destination, int immediate, Register source ) {
	insn_tmpl add = { 0x0 };

	add.A4.opcode	= 0x8;
	add.A4.x2a		= 0x2;
	add.A4.r3		= source;
	add.A4.r1		= destination;
	SET_A4_IMM(&add, immediate);

	return instruction( add.raw );
	} /* end generateShortImmediateAdd() */

instruction generateArithmetic( opCode op, Register destination, Register lhs, Register rhs ) {
	insn_tmpl alu = { 0x0 };

	alu.A1.opcode	= 0x8;
	alu.A1.r1		= destination;
	alu.A1.r2		= lhs;
	alu.A1.r3		= rhs;
	switch( op ) {
		case plusOp:	alu.A1.x4 = 0; alu.A1.x2b = 0; break;
		case minusOp:	alu.A1.x4 = 1; alu.A1.x2b = 1; break;
		case andOp:		alu.A1.x4 = 3; alu.A1.x2b = 0; break;
		case orOp:		alu.A1.x4 = 3; alu.A1.x2b = 2; break;
		default:
			bpfatal( "generateArithmetic() did not recognize opcode %d, aborting.\n", op );
			abort();
			break;
		} /* end op switch */

	return instruction( alu.raw );
	} /* end generateArithmetic() */

instruction generateIndirectCallTo( Register indirect, Register rp ) {
	insn_tmpl call = { 0x0 };

	call.B5.opcode	= 0x1;
	call.B5.wh		= 0x1;
	call.B5.b2		= indirect;
	call.B5.b1		= rp;

	return instruction( call.raw );
	} /* end generateIndirectCallTo() */

instruction generatePredicatesToRegisterMove( Register destination ) {
	insn_tmpl mov = { 0x0 };

	/* Mov Opcode	= 0x0 */
	mov.I25.x6		= 0x33;
	mov.I25.r1		= destination;

	return instruction( mov.raw );
	} /* end generatePredicatesToRegisterMove() */

instruction generateRegisterToPredicatesMove( Register source, int64_t mask64 ) {
	insn_tmpl mov = { 0x0 };

	/* Mov Opcode	= 0x0 */
	mov.I23.x3		= 0x3;
	mov.I23.r2		= source;
	SET_I23_MASK(&mov, mask64);

	return instruction( mov.raw );
	} /* end generateRegisterToPredicatesMove() */

instruction generateSpillTo( Register address, Register source, int64_t imm9 ) {
	insn_tmpl spill = { 0x0 };

	spill.M5.opcode	= 0x5;
	spill.M5.x6		= 0x3B;
	spill.M5.r2		= source;
	spill.M5.r3		= address;
	SET_M5_IMM(&spill, imm9);

	return instruction( spill.raw );
	} /* end generateSpillTo() */

instruction generateFillFrom( Register address, Register destination, int64_t imm9 ) {
	insn_tmpl fill = { 0x0 };

	if( imm9 == 0x0 ) {
		// Use no update form.
		fill.M1.opcode	= 0x4;
		fill.M1.x6		= 0x1B;
		fill.M1.r3		= address;
		fill.M1.r1		= destination;

	} else {
		// Use base update form.
		fill.M3.opcode	= 0x5;
		fill.M3.x6		= 0x1B;
		fill.M3.r3		= address;
		fill.M3.r1		= destination;
		SET_M3_IMM(&fill, imm9);
	}

	return instruction( fill.raw );
	} /* end generateFillFrom() */

instruction generateRegisterStore( Register address, Register source, int size, Register predicate ) {
	return generateRegisterStoreImmediate( address, source, 0, size, predicate );
	} /* generateRegisterStore() */

instruction generateRegisterStoreImmediate( Register address, Register source, int imm9, int size, Register predicate ) {
	insn_tmpl store = { 0x0 };

	store.M5.opcode	= 0x5;
	store.M5.r2		= source;
	store.M5.r3		= address;
	store.M5.qp		= predicate;
	switch (size) {
		case 1: store.M5.x6 = 0x30; break;
		case 2: store.M5.x6 = 0x31; break;
		case 4: store.M5.x6 = 0x32; break;
		case 8: store.M5.x6 = 0x33; break;
		default:
			bpfatal( "Illegal size %d, aborting.\n", size );
			assert( 0 );
			break;
		} /* end sizeSpec determiner */
	SET_M5_IMM(&store, imm9);

	return instruction( store.raw );
	} /* end generateRegisterStore() */

/* This is the no-update form, which lets the code generator do dumb
   stuff like load from and into the same register. */
instruction generateRegisterLoad( Register destination, Register address, int size ) {
	insn_tmpl load = { 0x0 };

	load.M1.opcode	= 0x4;
	load.M1.r3		= address;
	load.M1.r1		= destination;
	switch( size ) {
		case 1: load.M1.x6 = 0x00; break;
		case 2: load.M1.x6 = 0x01; break;
		case 4: load.M1.x6 = 0x02; break;
		case 8: load.M1.x6 = 0x03; break;
		default:
			bpfatal( "Illegal size %d, aborting.\n", size );
			assert( 0 );
			break;
		} /* end sizeSpec determiner */

	return instruction( load.raw );	
	} /* end generateRegisterLoad() */

instruction generateRegisterLoadImmediate( Register destination, Register address, int imm9, int size ) { 
	insn_tmpl load = { 0x0 };

	load.M3.opcode	= 0x5;
	load.M3.r3		= address;
	load.M3.r1		= destination;
	switch( size ) {
		case 1: load.M3.x6 = 0x00; break;
		case 2: load.M3.x6 = 0x01; break;
		case 4: load.M3.x6 = 0x02; break;
		case 8: load.M3.x6 = 0x03; break;
		default:
			bpfatal( "Illegal size %d, aborting.\n", size );
			assert( 0 );
			break;
		} /* end sizeSpec determiner */
	SET_M3_IMM(&load, imm9);

	return instruction( load.raw );
	} /* end generateRegisterLoad() */

instruction generateRegisterToApplicationMove( Register source, Register destination ) {
	/* The lower 48 application registers are only accessible via the M unit.  For simplicity,
	   divide responsibility at the sixty-fourth application register, with an I unit handling
	   the upper 64. */
	insn_tmpl mov = { 0x0 };

	if (destination <= 63) {
		mov.M29.opcode	= 0x1;
		mov.M29.x6		= 0x2A;
		mov.M29.r2		= source;
		mov.M29.ar3		= destination;

	} else {
		/* Mov Opcode	= 0x0 */
		mov.I26.x6		= 0x2A;
		mov.I26.r2 		= source;
		mov.I26.ar3		= destination;
	}

	return instruction( mov.raw );
	} /* end generateRegisterToApplicationMove() */

instruction generateApplicationToRegisterMove( Register source, Register destination ) {
	/* The lower 48 application registers are only accessible via the M unit.  For simplicity,
	   divide responsibility at the sixty-fourth application register, with an I unit handling
	   the upper 64. */
	insn_tmpl mov = { 0x0 };

	if (source <= 63) {
		mov.M31.opcode	= 0x1;
		mov.M31.x6		= 0x22;
		mov.M31.ar3		= source;
		mov.M31.r1		= destination;

	} else {
		/* Mov Opcode	= 0x0 */
		mov.I28.x6		= 0x32;
		mov.I28.ar3		= source;
		mov.I28.r1		= destination;
	}

	return instruction( mov.raw );
	} /* end generateRegisterToApplicationMove() */

instruction predicateInstruction( Register predicate, instruction insn ) {
	insn_tmpl tmpl = { insn.getMachineCode() };

	SET_PREDICATE(&tmpl, predicate);
	return instruction( tmpl.raw, insn.getTemplateID(), insn.getSlotNumber() );
	} /* end predicateInstruction() */

instruction_x predicateLongInstruction( Register predicate, instruction_x insn ) {
	insn_tmpl tmpl = { insn.getMachineCode().high };

	SET_PREDICATE(&tmpl, predicate);
	return instruction_x( insn.getMachineCode().low, tmpl.raw, insn.getTemplateID() );
	} /* end predicateLongInstruction() */

#define SWAP(a, b)	((a) ^= (b), (b) ^= (a), (a) ^= (b))
instruction generateComparison( opCode op, Register destination, Register lhs, Register rhs ) {
	insn_tmpl cmp = { 0x0 };
	Register anti_destination = destination + 1;

	/* We'll assume that all of our comparisons are signed. */
	switch( op ) {
		/* This gets cute.  The IA-64 hardware only implements the eq and lt ops,
		   so we get to do some argument and target rewriting to make things work.
	 	   The idea is to fall through the operations until we get to one that
		   can be implemented in hardware. */
	
		case greaterOp:	SWAP( destination, anti_destination ); /* Extra SWAP to undo geOp's */
		case leOp:		SWAP( lhs, rhs );
		case geOp:		SWAP( destination, anti_destination );
		case lessOp:	cmp.A6.opcode	= 0xC;
			break;

		case neOp:		SWAP( destination, anti_destination );
		case eqOp:		cmp.A6.opcode	= 0xE;
			break;

		default:
			bpfatal( "Unrecognized op %d in generateComparison(), aborting.\n", op );
			abort();
		} /* end op switch */

	cmp.A6.r2	= lhs;
	cmp.A6.r3	= rhs;
	cmp.A6.p1	= destination;
	cmp.A6.p2	= anti_destination;

	return instruction( cmp.raw );
	} /* end generateComparison() */

instruction generateFPSpillTo( Register address, Register source, int64_t imm9 ) {
	insn_tmpl spill_f = { 0x0 };

	spill_f.M10.opcode	= 0x7;
	spill_f.M10.x6		= 0x3B;
	spill_f.M10.f2		= source;
	spill_f.M10.r3		= address;
	SET_M10_IMM(&spill_f, imm9);

	return instruction( spill_f.raw );
	} /* end generateFPSpillTo() */

instruction generateFPFillFrom( Register address, Register destination, int64_t imm9 ) {
	insn_tmpl fill_f = { 0x0 };

	fill_f.M8.opcode	= 0x7;
	fill_f.M8.x6		= 0x1B;
	fill_f.M8.r3		= address;
	fill_f.M8.f1		= destination;
	SET_M8_IMM(&fill_f, imm9);

	return instruction( fill_f.raw );
	} /* end generateFPFillFrom() */

instruction generateRegisterToFloatMove( Register source, Register destination ) {
	insn_tmpl mov_f = { 0x0 };

	mov_f.M18.opcode	= 0x6;
	mov_f.M18.x6		= 0x1C;
	mov_f.M18.x			= 0x1;
	mov_f.M18.r2		= source;
	mov_f.M18.f1		= destination;

	return instruction( mov_f.raw );
	} /* end generateRegisterToFloatMove() */

instruction generateFloatToRegisterMove( Register source, Register destination ) {
	insn_tmpl mov_f = { 0x0 };

	mov_f.M19.opcode	= 0x4;
	mov_f.M19.x6		= 0x1C;
	mov_f.M19.x			= 0x1;
	mov_f.M19.f2		= source;
	mov_f.M19.r1		= destination;

	return instruction( mov_f.raw );
	} /* end generateFloatToRegisterMove() */

instruction generateFixedPointMultiply( Register destination, Register lhs, Register rhs ) {
	insn_tmpl xma_l = { 0x0 };

	/* FIXME: We're assuming unsigned, and that the lower 64 bits are more interesting,
		  but this may well not be the case. */
	xma_l.F2.opcode	= 0xE;
	xma_l.F2.x		= 0x1;
	xma_l.F2.f3		= lhs;
	xma_l.F2.f4		= rhs;
	xma_l.F2.f1		= destination;

	return instruction( xma_l.raw );
	} /* end generateFixedPointMultiply() */

void alterLongMoveAtTo( Address target, Address imm64 ) {
	ia64_bundle_t *rawBundle = (ia64_bundle_t *)target;
	insn_tmpl movl = { rawBundle->high }, imm = { rawBundle->low };

	SET_X2_IMM( &movl, &imm, imm64 );

	rawBundle->high	= movl.raw;
	rawBundle->low	= imm.raw;
	} /* end alterLongMoveAtTo() */

instruction generateShortImmediateBranch( int64_t target25 ) {
	insn_tmpl br_cond = { 0x0 };

	br_cond.B1.opcode = 0x4;
	SET_B1_TARGET(&br_cond, target25);

	return instruction( br_cond.raw );
	} /* end generateShortImmediateBranch() */



/* Require by insertTrapAtEntryPointOfMain() */
IA64_bundle generateTrapBundle() {
	/* Note: we're using 0x80000 as our break.m immediate,
	   which is defined to be a debugger breakpoint.  If this
	   gets flaky, anything up to 0x0FFFFF will generate a SIGTRAP. */

	/* Actually, what we're going to do is generate
	   a SIGILL, (0x40000) because SIGTRAP does silly things. */

	return IA64_bundle( MIIstop, TRAP_M, NOP_I, NOP_I );
	} /* end generateTrapBundle() */


void IA64_bundle::generate(codeGen &gen) {
	GET_PTR(insn, gen);
	*insn = myBundle;
	insn++;
	SET_PTR(insn, gen);
}

void instruction::generateIllegal(codeGen &gen) {
	generateTrapBundle().generate(gen);
}

void instruction::generateTrap(codeGen &gen) {
	// The trap is actually an illegal, apparently
	generateIllegal(gen);
}

void instruction::generateNOOP(codeGen &gen, unsigned size) {
	assert((size % 16) == 0);
	IA64_bundle nopBundle( MIIstop, NOP_M, NOP_I, NOP_I );
	while (size) {
		nopBundle.generate(gen);
		size -= 16;
	}
}

void instruction::generateBranch(codeGen &gen, Address from, Address to) {
	instruction_x lbtOriginal = generateLongBranchTo( to - from );
	IA64_bundle jumpBackBundle( MLXstop, instruction(NOP_M), lbtOriginal );
	jumpBackBundle.generate(gen);
}

Address instruction::getTarget(Address origAddr) const {
	return origAddr + getTargetAddress();
}

instruction *instruction::copy() const {
	return new instruction(insn_, templateID, slotNumber);
}

instruction_x *instruction_x::copy() const {
	return new instruction_x(insn_, insn_x_, templateID);
}
