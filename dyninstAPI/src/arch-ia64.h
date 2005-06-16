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

// $Id: arch-ia64.h,v 1.31 2005/06/16 03:15:06 tlmiller Exp $
// ia64 instruction declarations

#if !defined(ia64_unknown_linux2_4)
#error "invalid architecture-os inclusion"
#endif

#ifndef _ARCH_IA64_H
#define _ARCH_IA64_H

#include "common/h/Types.h"

/* So the IA-64 has these cute ideas about ILP, one consequence of which
   is the design of its instruction set.  Instructions execute in parallel
   until an architectural stop (certain instructions, types of bundles, etc)
   is reached, which forces the processor to make it look like all the
   instructions in the group since the the last stop were executed in
   sequence.  The instructions are stored three to a bundle, which is 128
   bits long, and includes three 41-bit instructions, as well as five bits
   determining the 'template,' which includes architectural stop information,
   as well as determining which execution unit(s) get which instruction(s).

   The instructions themselves don't identify their execution unit, only
   the major opcode of the instruction for that unit.  So considering an
   instruction in isolation is an excercise in futility.

   The question, then, is if we consider only whole bundles
   'instructions' and do some magic in the instruction iterator,
   of vice-versa...
*/

class IA64_bundle;

struct ia64_bundle_t {
	uint64_t low;
	uint64_t high;
	};

class IA64_instruction {
	friend class IA64_bundle;

	public:
		IA64_instruction( uint64_t insn = 0, uint8_t templ = 0x20, uint8_t slotN = 3 );

		const void * ptr() const;
		uint32_t size() const { return 16; }

		uint64_t getMachineCode() const { return instruction; }

		enum insnType { RETURN, BRANCH_IA, DIRECT_CALL, PREDICATED_BRANCH,
						CONDITIONAL_BRANCH,
						INDIRECT_CALL, INDIRECT_BRANCH, BRANCH_PREDICT,
						ALAT_CHECK, SPEC_CHECK, MOVE_FROM_IP, OTHER,
						INVALID, ALLOC, INTEGER_STORE, INTEGER_16_STORE,
						INTEGER_LOAD, INTEGER_16_LOAD, FP_STORE, FP_LOAD,
						INTEGER_PAIR_LOAD, FP_PAIR_LOAD, PREFETCH, BREAK,
						SYSCALL };

		enum unitType { M, I, F, B, L, X, RESERVED };
		virtual insnType getType() const;
		virtual unitType getUnitType() const;

		virtual Address getTargetAddress() const;
		virtual uint8_t getPredicate() const;

		uint8_t getTemplateID() const { return templateID; }
		uint8_t getSlotNumber() const { return slotNumber; }

	protected:
		uint64_t instruction;
		uint8_t templateID;
		uint8_t slotNumber;
	}; /* end the 41 bit instruction */

class IA64_instruction_x : public IA64_instruction {
	friend class IA64_bundle;

	public:
		IA64_instruction_x( uint64_t lowHalf = 0, uint64_t highHalf = 0, uint8_t templ = 0x20 );

		ia64_bundle_t getMachineCode() const { ia64_bundle_t r = { instruction, instruction_x }; return r; }	

		virtual insnType getType() const;
		virtual unitType getUnitType() const;

		virtual Address getTargetAddress() const;
		virtual uint8_t getPredicate() const;

	private:
		uint64_t instruction_x;

	}; /* end the 82 bit instruction */

class IA64_bundle {
	public:
		IA64_bundle( uint64_t lowHalfBundle = 0, uint64_t highHalfBundle = 0 );
		IA64_bundle( uint8_t templateID, uint64_t instruction0, uint64_t instruction1, uint64_t instruction2 );
		IA64_bundle( uint8_t templateID, const IA64_instruction & instruction0, const IA64_instruction instruction1, const IA64_instruction instruction2 );
		IA64_bundle( ia64_bundle_t rawBundle );
		IA64_bundle( uint8_t templateID, const IA64_instruction & instruction0, const IA64_instruction_x & instructionLX );

		ia64_bundle_t getMachineCode() const { return myBundle; }
		const ia64_bundle_t * getMachineCodePtr() const { return & myBundle; }

		uint8_t getTemplateID() const { return templateID; }
		IA64_instruction getInstruction0() const { return instruction0; }
		IA64_instruction getInstruction1() const { return instruction1; }
		IA64_instruction getInstruction2() const { return instruction2; }
		IA64_instruction * getInstruction( unsigned int slot );

		bool setInstruction( IA64_instruction &newInst );
		bool setInstruction( IA64_instruction_x &newInst );

		bool hasLongInstruction() { return templateID == 0x05 || templateID == 0x04; }
		IA64_instruction_x getLongInstruction();

	private:
		IA64_instruction instruction0;
		IA64_instruction instruction1;
		IA64_instruction instruction2;
		IA64_instruction_x longInstruction;
		uint8_t templateID;

		ia64_bundle_t myBundle;
	}; /* end the 128 bit bundle */

typedef IA64_instruction * instruction;

#include "inst-ia64.h"

/* Required by symtab.h, which seems to use it to check
   _instruction_ alignment. */
inline bool isAligned( const Address address ) {
	Address slotNo = address % 0x10;
	Address bundle = address - slotNo;
	return	( slotNo == 0 || slotNo == 1 || slotNo == 2 ) && 
			( bundle + slotNo == address );
	} /* end isAligned() */

/* Required by linux.C to find the address bounds
   of new dynamic heap segments. */
inline Address region_lo( const Address /* x */ ) {
	/* We're guessing the shared memory region. */
	return 0x2000000000000000;
	} /* end region_lo */

/* Required by linux.C to find the address bounds
   of new dynamic heap segments. */
inline Address region_hi( const Address /* x */ ) {
	/* We're guessing the shared memory region. */
	return 0x3FFFFFFFFFFFFFFF;
	} /* end region_hi */

/* Required by func-reloc.C to calculate relative displacements. */
int get_disp( instruction *insn );

/* Required by func-reloc.C to correct relative displacements after relocation. */
int set_disp( bool setDisp, instruction * insn, int newOffset, bool outOfFunc );

/* Convenience methods for func-reloc.C */
/* The problem being that neither of these really apply, do they? */
int sizeOfMachineInsn( instruction * insn );
int addressOfMachineInsn( instruction * insn );

/* Required by linux-ia64.C */
class registerSpace;
bool extractAllocatedRegisters( uint64_t allocInsn, uint64_t * allocatedLocal, uint64_t * allocatedOutput, uint64_t * allocatedRotate );
IA64_instruction generateAllocInstructionFor( registerSpace * rs, int locals, int outputs, int rotates );
IA64_instruction generateOriginalAllocFor( registerSpace * rs );

IA64_instruction generateShortConstantInRegister( unsigned int registerN, int imm22 );
IA64_instruction_x generateLongConstantInRegister( unsigned int registerN, long long int imm64 );
IA64_instruction_x generateLongCallTo( long long int displacement64, unsigned int branchRegister, Register predicate = 0 );

/* Required by linuxDL.C */
IA64_instruction generateReturnTo( unsigned int branchRegister );
IA64_instruction_x generateLongBranchTo( long long int displacement64, Register predicate = 0 );

/* Convience method for inst-ia64.C */
IA64_bundle generateBundleFromLongInstruction( IA64_instruction_x longInstruction );
void alterLongMoveAtTo( Address target, Address imm64 );

/* Required by inst-ia64.C */
IA64_instruction generateRegisterToRegisterMove( Register source, Register destination );
IA64_instruction generateIPToRegisterMove( Register destination );
IA64_instruction generateBranchToRegisterMove( Register source, Register destination );
IA64_instruction generateRegisterToBranchMove( Register source, Register destination, int immediate = 0 );
IA64_instruction generateShortImmediateAdd( Register destination, int immediate, Register source );
IA64_instruction generateIndirectCallTo( Register indirect, Register rp );
IA64_instruction generatePredicatesToRegisterMove( Register destination );
IA64_instruction generateRegisterToPredicatesMove( Register source, int64_t imm64 );
IA64_instruction generateRegisterStore( Register address, Register source, int size = 8, Register predicate = 0 );
IA64_instruction generateRegisterStoreImmediate( Register address, Register source, int imm9, int size = 8, Register predicate = 0 );
IA64_instruction generateRegisterLoad( Register destination, Register address, int size = 8 );
IA64_instruction generateRegisterLoadImmediate( Register destination, Register address, int imm9, int size = 8 );
IA64_instruction generateRegisterToApplicationMove( Register source, Register destination );
IA64_instruction generateApplicationToRegisterMove( Register source, Register destination );

IA64_instruction generateSpillTo( Register address, Register source, int64_t imm9 = 0 );
IA64_instruction generateFillFrom( Register address, Register destination, int64_t imm9 = 0 );
IA64_instruction generateFPSpillTo( Register address, Register source, int64_t imm9 = 0 );
IA64_instruction generateFPFillFrom( Register address, Register destination, int64_t imm9 = 0 );
IA64_instruction generateRegisterToFloatMove( Register source, Register destination );
IA64_instruction generateFloatToRegisterMove( Register source, Register destination );
IA64_instruction generateFixedPointMultiply( Register destination, Register lhs, Register rhs );

IA64_instruction generateShortImmediateBranch( int64_t target25 );

#include "ast.h"
IA64_instruction generateComparison( opCode op, Register destination, Register lhs, Register rhs );
IA64_instruction generateArithmetic( opCode op, Register destination, Register lhs, Register rhs );

IA64_instruction predicateInstruction( Register predicate, IA64_instruction insn );
IA64_instruction_x predicateLongInstruction( Register predicate, IA64_instruction_x longInsn );

/* For ast.C */
class instPoint;
class registerSpace;

/* There are twenty-five unstacked integer registers, four application registers,
   and three branch registers which are or must be treated as if they were 
   scratch registers.  We also use a general register to preserve the predicates
   and the stack pointer. */
#define NUM_PRESERVED 34
#define NUM_LOCALS 8
#define NUM_OUTPUT 8
bool defineBaseTrampRegisterSpaceFor( const instPoint * location, registerSpace * regSpace, Register * deadRegisterList, process * proc );

/* Constants for code generation. */
#define BRANCH_SCRATCH		6	
#define BRANCH_SCRATCH_ALT 	7
#define BRANCH_RETURN		0
#define REGISTER_ZERO		0	/* makes it clearer when I'm using the register and not the value */
#define REGISTER_GP		1	/* the general pointer */
#define REGISTER_RV		8	/* return value, if structure/union, pointer. */
#define REGISTER_SP		12	/* memory stack pointer */
#define REGISTER_TP		13	/* thread pointer */
#define AR_PFS			64	/* application register: previous function state */
#define AR_UNAT			36	/* application register: User Not-a-Thing collection */
#define AR_CCV			32	/* application register: Compare & exchange Compare Value register */
#define AR_CSD			25	/* application register: reserved for future implicit operands */
#define AR_SSD			26	/* application register: reserved for future implicit operands */
#define AR_RSC			16 	/* application register: Register Stack engine Configuration register */
#define AR_FPSR			40	/* application register: Floating-Point Status Register */

/* (left-aligned) instruction bit masks. */
#define MAJOR_OPCODE_MASK   0xF000000000000000  /* bits 37 - 40 */

/* (right-aligned) Template IDs. */
const uint8_t MII = 0x00;
const uint8_t MIIstop = 0x01;
const uint8_t MIstopI = 0x02;
const uint8_t MIstopIstop = 0x03;
const uint8_t MLX = 0x04;
const uint8_t MLXstop = 0x05;

const uint8_t MMI = 0x08;
const uint8_t MMIstop = 0x09;
const uint8_t MstopMI = 0x0A;
const uint8_t MstopMIstop = 0x0B;
const uint8_t MFI = 0x0C;
const uint8_t MFIstop = 0x0D;
const uint8_t MMF = 0x0E;
const uint8_t MMFstop = 0x0F;
const uint8_t MIB = 0x10;
const uint8_t MIBstop = 0x11;
const uint8_t MBB  = 0x12;
const uint8_t MBBstop = 0x13;

const uint8_t BBB  = 0x16;
const uint8_t BBBstop = 0x17;
const uint8_t MMB = 0x18;
const uint8_t MMBstop = 0x19;

const uint8_t MFB = 0x1C;
const uint8_t MFBstop = 0x1D;

/* (left-aligned) Machine code. */
const uint64_t NOP_I = 0x0004000000000000;
const uint64_t NOP_M = 0x0004000000000000;
const uint64_t NOP_B = 0x2000000000000000;
const uint64_t TRAP_M = 0x0000800000000000;
const uint64_t TRAP_I = 0x0000800000000000;

//
// Bit level templates for extracted
// (non-bundled) instructions.
//
typedef union {
	uint64_t raw;

	struct {
		uint64_t unused		: 23;
		uint64_t qp			:  6;
		uint64_t 			: 31;
		uint64_t opcode		:  4;
	} GENERAL;
#define GET_OPCODE(x)		( (x)->GENERAL.opcode )
#define GET_PREDICATE(x)	( (x)->GENERAL.qp )
#define SET_PREDICATE(x, y)	( assert( !((y) >> 6) ), \
							  (x)->GENERAL.qp = (y) )

	//
	// These templates are aid instruction classification.
	//
	struct {
		uint64_t unused		: 23;
		uint64_t 			: 27;
		uint64_t x2b		:  2;
		uint64_t x4			:  4;
		uint64_t ve			:  1;
		uint64_t x2a		:  2;
		uint64_t 			:  1;
		uint64_t opcode		:  4;
	} A_ALU;

	struct {
		uint64_t unused		: 23;
		uint64_t 			: 12;
		uint64_t c			:  1;
		uint64_t 			: 20;
		uint64_t ta			:  1;
		uint64_t x2			:  2;
		uint64_t tb			:  1;
		uint64_t opcode		:  4;
	} A_CMP;

	struct {
		uint64_t unused		: 23;
		uint64_t 			: 27;
		uint64_t x2b		:  2;
		uint64_t x4			:  4;
		uint64_t zb			:  1;
		uint64_t x2a		:  2;
		uint64_t za			:  1;
		uint64_t opcode		:  4;
	} A_MM;

	struct {
		uint64_t unused		: 23;
		uint64_t 			: 28;
		uint64_t x2b		:  2;
		uint64_t x2c		:  2;
		uint64_t ve			:  1;
		uint64_t zb			:  1;
		uint64_t x2a		:  2;
		uint64_t za			:  1;
		uint64_t opcode		:  4;
	} I_SHIFT;

	struct {
		uint64_t unused		: 23;
		uint64_t 			: 26;
		uint64_t y			:  1;
		uint64_t x6			:  6;
		uint64_t x3			:  3;
		uint64_t 			:  1;
		uint64_t opcode		:  4;
	} I_MISC;

	struct {
		uint64_t unused		: 23;
		uint64_t 			: 27;
		uint64_t x			:  1;
		uint64_t 			:  2;
		uint64_t x6			:  6;
		uint64_t m			:  1;
		uint64_t opcode		:  4;
	} M_LD_ST;

	struct {
		uint64_t unused		: 23;
		uint64_t 			: 27;
		uint64_t x			:  1;
		uint64_t 			:  2;
		uint64_t x6_low		:  2;
		uint64_t x6_high	:  4;
		uint64_t m			:  1;
		uint64_t opcode		:  4;
	} M_LD_ST_SPLIT;

	struct {
		uint64_t unused		: 23;
		uint64_t 			: 26;
		uint64_t y			:  1;
		uint64_t x4			:  4;
		uint64_t x2			:  2;
		uint64_t x3			:  3;
		uint64_t 			:  1;
		uint64_t opcode		:  4;
	} M_SYS;

	struct {
		uint64_t unused		: 23;
		uint64_t 			: 26;
		uint64_t y			:  1;
		uint64_t x6			:  6;
		uint64_t x3			:  3;
		uint64_t 			:  1;
		uint64_t opcode		:  4;
	} M_MEM;

	struct {
		uint64_t unused		: 23;
		uint64_t 			:  6;
		uint64_t btype		:  3;
		uint64_t 			: 18;
		uint64_t x6			:  6;
		uint64_t 			:  4;
		uint64_t opcode		:  4;
	} B;

	struct {
		uint64_t unused		: 23;
		uint64_t 			: 26;
		uint64_t y			:  1;
		uint64_t x6			:  6;
		uint64_t x			:  1;
		uint64_t sf			:  2;
		uint64_t q			:  1;
		uint64_t opcode		:  4;
	} F_MISC;
	
	struct {
		uint64_t unused		: 23;
		uint64_t 			: 34;
		uint64_t x2			: 2;
		uint64_t x			: 1;
		uint64_t opcode		: 4;
	} F_ARITH;

	struct {
		uint64_t unused		: 23;
		uint64_t 			: 12;
		uint64_t ta			:  1;
		uint64_t 			: 20;
		uint64_t ra			:  1;
		uint64_t 			:  2;
		uint64_t rb			:  1;
		uint64_t opcode		:  4;
	} F_CMP;

	struct {
		uint64_t unused		: 23;
		uint64_t 			:  6;
		uint64_t btype		:  3;
		uint64_t 			: 11;
		uint64_t v0			:  1;
		uint64_t 			:  5;
		uint64_t y			:  1;
		uint64_t x6			:  6;
		uint64_t x3			:  3;
		uint64_t 			:  1;
		uint64_t opcode		:  4;
	} X;

	//
	// The following templates are used to work with
	// known instructions.
	//
	struct {
		uint64_t unused		: 23;
		uint64_t qp			:  6;
		uint64_t r1			:  7;
		uint64_t r2			:  7;
		uint64_t r3			:  7;
		uint64_t x2b		:  2;
		uint64_t x4			:  4;
		uint64_t ve			:  1;
		uint64_t x2a		:  2;
		uint64_t 			:  1;
		uint64_t opcode		:  4;
	} A1;

	struct {
		uint64_t unused		: 23;
		uint64_t qp			:  6;
		uint64_t r1			:  7;
		uint64_t r2			:  7;
		uint64_t r3			:  7;
		uint64_t ct2d		:  2;
		uint64_t x4			:  4;
		uint64_t ve			:  1;
		uint64_t x2a		:  2;
		uint64_t 			:  1;
		uint64_t opcode		:  4;
	} A2;
#define GET_A2_COUNT(x)		( (x)->A2.ct2d + 1 )
#define SET_A2_COUNT(x,y)	( assert( !((y) >> 2) && (y)), \
							  (x)->A2.ct2d = (y) - 1 )

	struct {
		uint64_t unused		: 23;
		uint64_t qp			:  6;
		uint64_t r1			:  7;
		uint64_t imm7b		:  7;
		uint64_t r3			:  7;
		uint64_t x2b		:  2;
		uint64_t x4			:  4;
		uint64_t ve			:  1;
		uint64_t x2a		:  2;
		uint64_t s			:  1;
		uint64_t opcode		:  4;
	} A3;
#define GET_A3_IMM(x)		( (x)->A3.imm7b | ((x)->A3.s ? -1 : 0) << 7)
#define SET_A3_IMM(x,y)		( assert( -(1<<7) <= (y) && (y) < (1<<7) ),	\
							  ((x)->A3.s = (y) < 0),					\
							  ((x)->A3.imm7b = (y)) )

	struct {
		uint64_t unused		: 23;
		uint64_t qp			:  6;
		uint64_t r1			:  7;
		uint64_t imm7b		:  7;
		uint64_t r3			:  7;
		uint64_t imm6d		:  6;
		uint64_t ve			:  1;
		uint64_t x2a		:  2;
		uint64_t s			:  1;
		uint64_t opcode		:  4;
	} A4;
#define GET_A4_IMM(x)		( (((x)->A4.imm6d << 7) | ((x)->A4.imm7b)) | \
							  ((x)->A4.s ? -1 : 0) << 13 )
#define SET_A4_IMM(x,y)		( assert( -(1<<13) <= (y) && (y) < (1<<13) ),	\
							  ((x)->A4.s = (y) < 0),						\
							  ((x)->A4.imm6d = ((y) >> 7)),					\
							  ((x)->A4.imm7b = (y)) )

	struct {
		uint64_t unused		: 23;
		uint64_t qp			:  6;
		uint64_t r1			:  7;
		uint64_t imm7b		:  7;
		uint64_t r3			:  2;
		uint64_t imm5c		:  5;
		uint64_t imm9d		:  9;
		uint64_t s			:  1;
		uint64_t opcode		:  4;
	} A5;
#define GET_A5_IMM(x)		( (((x)->A5.imm5c << 16) | ((x)->A5.imm9d << 7) | ((x)->A5.imm7b)) | \
							  ((x)->A5.s ? -1 : 0) << 21 )
#define SET_A5_IMM(x,y)		( assert( -(1<<21) <= (y) && (y) < (1<<21) ),	\
							  ((x)->A5.s = (y) < 0),						\
							  ((x)->A5.imm5c = ((y) >> 16)),				\
							  ((x)->A5.imm9d = ((y) >> 7)),					\
							  ((x)->A5.imm7b = (y)) )

	struct {
		uint64_t unused		: 23;
		uint64_t qp			:  6;
		uint64_t p1			:  6;
		uint64_t c			:  1;
		uint64_t r2			:  7;
		uint64_t r3			:  7;
		uint64_t p2			:  6;
		uint64_t ta			:  1;
		uint64_t x2			:  2;
		uint64_t tb			:  1;
		uint64_t opcode		:  4;
	} A6;

	struct {
		uint64_t unused		: 23;
		uint64_t qp			:  6;
		uint64_t p1			:  6;
		uint64_t c			:  1;
		uint64_t zero		:  7;
		uint64_t r3			:  7;
		uint64_t p2			:  6;
		uint64_t ta			:  1;
		uint64_t x2			:  2;
		uint64_t tb			:  1;
		uint64_t opcode		:  4;
	} A7;

	struct {
		uint64_t unused		: 23;
		uint64_t qp			:  6;
		uint64_t p1			:  6;
		uint64_t c			:  1;
		uint64_t imm7b		:  7;
		uint64_t r3			:  7;
		uint64_t p2			:  6;
		uint64_t ta			:  1;
		uint64_t x2			:  2;
		uint64_t s			:  1;
		uint64_t opcode		:  4;
	} A8;
#define GET_A8_COUNT(x)		( (x)->A8.imm7b | ((x)->A8.s ? -1 : 0) << 7)
#define SET_A8_COUNT(x,y)	( assert( -(1<<7) <= (y) && (y) < (1<<7) ),	\
							  ((x)->A8.s = (y) < 0),					\
							  ((x)->A8.imm7b = (y)) )

	struct {
		uint64_t unused		: 23;
		uint64_t qp			:  6;
		uint64_t r1			:  7;
		uint64_t r2			:  7;
		uint64_t r3			:  7;
		uint64_t x2b		:  2;
		uint64_t x4			:  4;
		uint64_t zb			:  1;
		uint64_t x2a		:  2;
		uint64_t za			:  1;
		uint64_t opcode		:  4;
	} A9;

	struct {
		uint64_t unused		: 23;
		uint64_t qp			:  6;
		uint64_t r1			:  7;
		uint64_t r2			:  7;
		uint64_t r3			:  7;
		uint64_t ct2d		:  2;
		uint64_t x4			:  4;
		uint64_t zb			:  1;
		uint64_t x2a		:  2;
		uint64_t za			:  1;
		uint64_t opcode		:  4;
	} A10;
#define GET_A10_COUNT(x)	( (x)->A10.ct2d + 1 )
#define SET_A10_COUNT(x,y)	( assert( !((y) >> 2) && (y)), \
							  (x)->A10.ct2d = (y) - 1 )

	struct {
		uint64_t unused		: 23;
		uint64_t qp			:  6;
		uint64_t r1			:  7;
		uint64_t r2			:  7;
		uint64_t r3			:  7;
		uint64_t 			:  1;
		uint64_t x2b		:  2;
		uint64_t ct2d		:  2;
		uint64_t ve			:  1;
		uint64_t zb			:  1;
		uint64_t x2a		:  2;
		uint64_t za			:  1;
		uint64_t opcode		:  4;
	} I1;
#define GET_I1_COUNT(x)		( (x)->I1.ct2d == 0 ? 0 : ((x)->I1.ct2d == 1 ? 7 : ((x)->I1.ct2d == 2 ? 15 : 16)) )
#define SET_I1_COUNT(x,y)	( assert( ((y) == 0) || ((y) == 7) || ((y) == 15) || ((y) == 16) ), \
							  (x)->I1.ct2d = ((y) == 0 ? 0 : ((y) == 7 ? 1 : ((y) == 15 ? 2 : 3))) )

	struct {
		uint64_t unused		: 23;
		uint64_t qp			:  6;
		uint64_t r1			:  7;
		uint64_t r2			:  7;
		uint64_t r3			:  7;
		uint64_t 			:  1;
		uint64_t x2b		:  2;
		uint64_t x2c		:  2;
		uint64_t ve			:  1;
		uint64_t zb			:  1;
		uint64_t x2a		:  2;
		uint64_t za			:  1;
		uint64_t opcode		:  4;
	} I2;

	struct {
		uint64_t unused		: 23;
		uint64_t qp			:  6;
		uint64_t r1			:  7;
		uint64_t r2			:  7;
		uint64_t mbt4c		:  4;
		uint64_t 			:  4;
		uint64_t x2b		:  2;
		uint64_t x2c		:  2;
		uint64_t ve			:  1;
		uint64_t zb			:  1;
		uint64_t x2a		:  2;
		uint64_t za			:  1;
		uint64_t opcode		:  4;
	} I3;

	struct {
		uint64_t unused		: 23;
		uint64_t qp			:  6;
		uint64_t r1			:  7;
		uint64_t r2			:  7;
		uint64_t mht8c		:  8;
		uint64_t x2b		:  2;
		uint64_t x2c		:  2;
		uint64_t ve			:  1;
		uint64_t zb			:  1;
		uint64_t x2a		:  2;
		uint64_t za			:  1;
		uint64_t opcode		:  4;
	} I4;
#define GET_I4_TYPE(x)		( (x)->I4.mht8c )
#define SET_I4_TYPE(x,y)	( assert( !((y) >> 8) ), \
							  (x)->I4.mht8c = (y) )

	struct {
		uint64_t unused		: 23;
		uint64_t qp			:  6;
		uint64_t r1			:  7;
		uint64_t r2			:  7;
		uint64_t r3			:  7;
		uint64_t 			:  1;
		uint64_t x2b		:  2;
		uint64_t x2c		:  2;
		uint64_t ve			:  1;
		uint64_t zb			:  1;
		uint64_t x2a		:  2;
		uint64_t za			:  1;
		uint64_t opcode		:  4;
	} I5;

	struct {
		uint64_t unused		: 23;
		uint64_t qp			:  6;
		uint64_t r1			:  7;
		uint64_t 			:  1;
		uint64_t count5b	:  5;
		uint64_t 			:  1;
		uint64_t r3			:  7;
		uint64_t 			:  1;
		uint64_t x2b		:  2;
		uint64_t x2c		:  2;
		uint64_t ve			:  1;
		uint64_t zb			:  1;
		uint64_t x2a		:  2;
		uint64_t za			:  1;
		uint64_t opcode		:  4;
	} I6;
#define GET_I6_COUNT(x)		( (x)->I6.count5b )
#define SET_I6_COUNT(x,y)	( assert( !((y) >> 5) ), \
							  (x)->I6.count5b = (y) )

	struct {
		uint64_t unused		: 23;
		uint64_t qp			:  6;
		uint64_t r1			:  7;
		uint64_t r2			:  7;
		uint64_t r3			:  7;
		uint64_t 			:  1;
		uint64_t x2b		:  2;
		uint64_t x2c		:  2;
		uint64_t ve			:  1;
		uint64_t zb			:  1;
		uint64_t x2a		:  2;
		uint64_t za			:  1;
		uint64_t opcode		:  4;
	} I7;

	struct {
		uint64_t unused		: 23;
		uint64_t qp			:  6;
		uint64_t r1			:  7;
		uint64_t r2			:  7;
		uint64_t ccount5c	:  5;
		uint64_t 			:  3;
		uint64_t x2b		:  2;
		uint64_t x2c		:  2;
		uint64_t ve			:  1;
		uint64_t zb			:  1;
		uint64_t x2a		:  2;
		uint64_t za			:  1;
		uint64_t opcode		:  4;
	} I8;
#define GET_I8_COUNT(x)		( 31 - (x)->I8.count5b )
#define SET_I8_COUNT(x,y)	( assert( !((y) >> 5) ), \
							  (x)->I8.count5b = 31 - (y) )

	struct {
		uint64_t unused		: 23;
		uint64_t qp			:  6;
		uint64_t r1			:  7;
		uint64_t zero		:  7;
		uint64_t r3			:  7;
		uint64_t 			:  1;
		uint64_t x2b		:  2;
		uint64_t x2c		:  2;
		uint64_t ve			:  1;
		uint64_t zb			:  1;
		uint64_t x2a		:  2;
		uint64_t za			:  1;
		uint64_t opcode		:  4;
	} I9;

	struct {
		uint64_t unused		: 23;
		uint64_t qp			:  6;
		uint64_t r1			:  7;
		uint64_t r2			:  7;
		uint64_t r3			:  7;
		uint64_t count6d	:  6;
		uint64_t x			:  1;
		uint64_t x2			:  2;
		uint64_t 			:  1;
		uint64_t opcode		:  4;
	} I10;
#define GET_I10_COUNT(x)	( (x)->I10.count6d )
#define SET_I10_COUNT(x,y)	( assert( !((y) >> 6)), \
							  (x)->I10.count6d = (y) )

	struct {
		uint64_t unused		: 23;
		uint64_t qp			:  6;
		uint64_t r1			:  7;
		uint64_t y			:  1;
		uint64_t pos6b		:  6;
		uint64_t r3			:  7;
		uint64_t len6d		:  6;
		uint64_t x			:  1;
		uint64_t x2			:  2;
		uint64_t 			:  1;
		uint64_t opcode		:  4;
	} I11;
#define GET_I11_LEN(x)		( (x)->I11.len6d + 1 )
#define SET_I11_LEN(x,y)	( assert( !((y) >> 6) && (y) ), \
							  (x)->I11.len6d = (y) - 1 )
#define GET_I11_POS(x)		( (x)->I11.pos6b )
#define SET_I11_POS(x,y)	( assert( !((y) >> 6)), \
							  (x)->I11.pos6b = (y) )

	struct {
		uint64_t unused		: 23;
		uint64_t qp			:  6;
		uint64_t r1			:  7;
		uint64_t r2			:  7;
		uint64_t cpos6c		:  6;
		uint64_t y			:  1;
		uint64_t len6d		:  6;
		uint64_t x			:  1;
		uint64_t x2			:  2;
		uint64_t 			:  1;
		uint64_t opcode		:  4;
	} I12;
#define GET_I12_LEN(x)		( (x)->I12.len6d + 1 )
#define SET_I12_LEN(x,y)	( assert( !((y) >> 6) && (y) ), \
							  (x)->I12.len6d = (y) - 1 )
#define GET_I12_POS(x)		( 63 - (x)->I12.cpos6c )
#define SET_I12_POS(x,y)	( assert( !((y) >> 6)), \
							  (x)->I12.cpos6c = 63 - (y) )

	struct {
		uint64_t unused		: 23;
		uint64_t qp			:  6;
		uint64_t r1			:  7;
		uint64_t imm7b		:  7;
		uint64_t cpos6c		:  6;
		uint64_t y			:  1;
		uint64_t len6d		:  6;
		uint64_t x			:  1;
		uint64_t x2			:  2;
		uint64_t s			:  1;
		uint64_t opcode		:  4;
	} I13;
#define GET_I13_LEN(x)		( (x)->I13.len6d + 1 )
#define SET_I13_LEN(x,y)	( assert( !((y) >> 6) && (y) ), \
							  (x)->I13.len6d = (y) - 1 )
#define GET_I13_POS(x)		( 63 - (x)->I13.cpos6c )
#define SET_I13_POS(x,y)	( assert( !((y) >> 6)), \
							  (x)->I13.cpos6c = 63 - (y) )
#define GET_I13_IMM(x)		( (x)->I13.imm7b | ((x)->I13.s ? -1 : 0) << 7)
#define SET_I13_IMM(x,y)	( assert( -(1<<7) <= (y) && (y) < (1<<7) ),	\
							  ((x)->I13.s = ((y) < 0)),					\
							  ((x)->I13.imm7b = (y)) )

	struct {
		uint64_t unused		: 23;
		uint64_t qp			:  6;
		uint64_t r1			:  7;
		uint64_t 			:  1;
		uint64_t cpos6b		:  6;
		uint64_t r3			:  7;
		uint64_t len6d		:  6;
		uint64_t x			:  1;
		uint64_t x2			:  2;
		uint64_t s			:  1;
		uint64_t opcode		:  4;
	} I14;
#define GET_I14_LEN(x)		( (x)->I14.len6d + 1 )
#define SET_I14_LEN(x,y)	( assert( !((y) >> 6) && (y) ), \
							  (x)->I14.len6d = (y) - 1 )
#define GET_I14_POS(x)		( 63 - (x)->I14.cpos6b )
#define SET_I14_POS(x,y)	( assert( !((y) >> 6)), \
							  (x)->I14.cpos6b = 63 - (y) )
#define GET_I14_IMM(x)		( -((x)->I14.s) )
#define SET_I14_IMM(x,y)	( assert( (y) == -1 || (y) == 0 ), \
							  ((x)->I14.s = (y)) )

	struct {
		uint64_t unused		: 23;
		uint64_t qp			:  6;
		uint64_t r1			:  7;
		uint64_t r2			:  7;
		uint64_t r3			:  7;
		uint64_t len4d		:  4;
		uint64_t cpos6d		:  6;
		uint64_t opcode		:  4;
	} I15;
#define GET_I15_LEN(x)		( (x)->I15.len4d + 1 )
#define SET_I15_LEN(x,y)	( assert( ((y) == 16 || !((y) >> 4)) && (y) ), \
							  (x)->I15.len4d = (y) - 1 )
#define GET_I15_POS(x)		( 63 - (x)->I15.cpos6b )
#define SET_I15_POS(x,y)	( assert( !((y) >> 6)), \
							  (x)->I15.cpos6b = 63 - (y) )

	struct {
		uint64_t unused		: 23;
		uint64_t qp			:  6;
		uint64_t p1			:  6;
		uint64_t c			:  1;
		uint64_t y			:  1;
		uint64_t pos6b		:  6;
		uint64_t r3			:  7;
		uint64_t p2			:  6;
		uint64_t ta			:  1;
		uint64_t x2			:  2;
		uint64_t tb			:  1;
		uint64_t opcode		:  4;
	} I16;
#define GET_I16_POS(x)		( (x)->I16.pos6b )
#define SET_I16_POS(x,y)	( assert( !((y) >> 6) ), \
							  (x)->I16.pos6b = (y) )

	struct {
		uint64_t unused		: 23;
		uint64_t qp			:  6;
		uint64_t p1			:  6;
		uint64_t c			:  1;
		uint64_t y			:  1;
		uint64_t 			:  6;
		uint64_t r3			:  7;
		uint64_t p2			:  6;
		uint64_t ta			:  1;
		uint64_t x2			:  2;
		uint64_t tb			:  1;
		uint64_t opcode		:  4;
	} I17;

	struct {
		uint64_t unused		: 23;
		uint64_t qp			:  6;
		uint64_t imm20a		: 20;
		uint64_t y			:  1;
		uint64_t x6			:  6;
		uint64_t x3			:  3;
		uint64_t i			:  1;
		uint64_t opcode		:  4;
	} I18;
#define GET_I18_IMM(x)		( ((x)->I18.i << 20) | ((x)->I18.imm20a) )
#define SET_I18_IMM(x,y)	( assert( !((y) >> 21) ),	\
							  ((x)->I18.i = (y) >> 20),	\
							  ((x)->I18.imm20a = (y)) )

	struct {
		uint64_t unused		: 23;
		uint64_t qp			:  6;
		uint64_t imm20a		: 20;
		uint64_t 			:  1;
		uint64_t x6			:  6;
		uint64_t x3			:  3;
		uint64_t i			:  1;
		uint64_t opcode		:  4;
	} I19;
#define GET_I19_IMM(x)		( ((x)->I19.i << 20) | ((x)->I19.imm20a) )
#define SET_I19_IMM(x,y)	( assert( !((y) >> 21) ),	\
							  ((x)->I19.i = (y) >> 20),	\
							  ((x)->I19.imm20a = (y)) )

	struct {
		uint64_t unused		: 23;
		uint64_t qp			:  6;
		uint64_t imm7a		:  7;
		uint64_t r2			:  7;
		uint64_t imm13c		: 13;
		uint64_t x3			:  3;
		uint64_t s			:  1;
		uint64_t opcode		:  4;
	} I20;
#define GET_I20_TARGET(x)	( (((x)->I20.imm13c << 11) | ((x)->I20.imm7a << 4)) |			\
							  (((x)->I20.s ? -1 : 0) << 24) )
#define SET_I20_TARGET(x,y)	( assert( -(1<<24) <= (y) && (y) < (1<<24) && !((y) & 0xF) ),	\
							  ((x)->I20.s = ((y) < 0)),										\
							  ((x)->I20.imm13c = (y) >> 11),								\
							  ((x)->I20.imm7a = (y) >> 4) )

	struct {
		uint64_t unused		: 23;
		uint64_t qp			:  6;
		uint64_t b1			:  3;
		uint64_t 			:  4;
		uint64_t r2			:  7;
		uint64_t wh			:  2;
		uint64_t x			:  1;
		uint64_t ih			:  1;
		uint64_t timm9c		:  9;
		uint64_t x3			:  3;
		uint64_t 			:  1;
		uint64_t opcode		:  4;
	} I21;
#define GET_I21_TAG(x)		( ((x)->I21.timm9c << 4) |										\
							  ((x)->I21.timm9c >> 8 ? -1 : 0) << 12 )
#define SET_I21_TAG(x,y)	( assert( -(1<<12) <= (y) && (y) < (1<<12) && !((y) & 0xF) ),	\
							  (x)->I21.timm9c = (y) >> 4 )

	struct {
		uint64_t unused		: 23;
		uint64_t qp			:  6;
		uint64_t r1			:  7;
		uint64_t b2			:  3;
		uint64_t 			: 11;
		uint64_t x6			:  6;
		uint64_t x3			:  3;
		uint64_t 			:  1;
		uint64_t opcode		:  4;
	} I22;

	struct {
		uint64_t unused		: 23;
		uint64_t qp			:  6;
		uint64_t mask7a		:  7;
		uint64_t r2			:  7;
		uint64_t 			:  4;
		uint64_t mask8c		:  8;
		uint64_t 			:  1;
		uint64_t x3			:  3;
		uint64_t s			:  1;
		uint64_t opcode		:  4;
	} I23;
#define GET_I23_MASK(x)		( (((x)->I23.mask8c << 8) | ((x)->I23.mask7a << 1)) | \
							  (((x)->I23.s ? -1 : 0) << 16) )
#define SET_I23_MASK(x,y)	( assert( -(1<<16) <= (y) && (y) < (1<<16) && !((y) & 0x1)),	\
							  ((x)->I23.s = ((y) < 0)),										\
							  ((x)->I23.mask8c = (y) >> 8),									\
							  ((x)->I23.mask7a = (y) >> 1) )

	struct {
		uint64_t unused		: 23;
		uint64_t qp			:  6;
		uint64_t imm27a		: 27;
		uint64_t x3			:  3;
		uint64_t s			:  1;
		uint64_t opcode		:  4;
	} I24;
#define GET_I24_IMM(x)		( ((x)->I24.imm27a << 16) | ((x)->I24.s ? -1 : 0) << 43 )
#define SET_I24_IMM(x,y)	( assert( -(1<<43) <= (y) && (y) < (1<<43) && !((y) & 0xFFFF) ),	\
							  ((x)->I24.s = ((y) < 0)),											\
							  ((x)->I24.imm27a = (y) >> 16) )
#define GET_I24_MASK(x)		GET_I24_IMM(x)
#define SET_I24_MASK(x,y)	SET_I24_IMM(x,y)

	struct {
		uint64_t unused		: 23;
		uint64_t qp			:  6;
		uint64_t r1			:  7;
		uint64_t 			: 14;
		uint64_t x6			:  6;
		uint64_t x3			:  3;
		uint64_t 			:  1;
		uint64_t opcode		:  4;
	} I25;

	struct {
		uint64_t unused		: 23;
		uint64_t qp			:  6;
		uint64_t 			:  7;
		uint64_t r2			:  7;
		uint64_t ar3		:  7;
		uint64_t x6			:  6;
		uint64_t x3			:  3;
		uint64_t 			:  1;
		uint64_t opcode		:  4;
	} I26;

	struct {
		uint64_t unused		: 23;
		uint64_t qp			:  6;
		uint64_t 			:  7;
		uint64_t imm7b		:  7;
		uint64_t ar3		:  7;
		uint64_t x6			:  6;
		uint64_t x3			:  3;
		uint64_t s			:  1;
		uint64_t opcode		:  4;
	} I27;
#define GET_I27_IMM(x)		( (x)->I27.imm7b | ((x)->I27.s ? -1 : 0) << 7)
#define SET_I27_IMM(x,y)	( assert( -(1<<7) <= (y) && (y) < (1<<7) ),	\
							  ((x)->I27.s = (y) < 0), 					\
							  ((x)->I27.imm7b = (y)) )

	struct {
		uint64_t unused		: 23;
		uint64_t qp			:  6;
		uint64_t r1			:  7;
		uint64_t 			:  7;
		uint64_t ar3		:  7;
		uint64_t x6			:  6;
		uint64_t x3			:  3;
		uint64_t 			:  1;
		uint64_t opcode		:  4;
	} I28;

	struct {
		uint64_t unused		: 23;
		uint64_t qp			:  6;
		uint64_t r1			:  7;
		uint64_t 			:  7;
		uint64_t r3			:  7;
		uint64_t x6			:  6;
		uint64_t x3			:  3;
		uint64_t 			:  1;
		uint64_t opcode		:  4;
	} I29;

	struct {
		uint64_t unused		: 23;
		uint64_t qp			:  6;
		uint64_t r1			:  7;
		uint64_t 			:  7;
		uint64_t r3			:  7;
		uint64_t x			:  1;
		uint64_t hint		:  2;
		uint64_t x6			:  6;
		uint64_t m			:  1;
		uint64_t opcode		:  4;
	} M1;

	struct {
		uint64_t unused		: 23;
		uint64_t qp			:  6;
		uint64_t r1			:  7;
		uint64_t r2			:  7;
		uint64_t r3			:  7;
		uint64_t x			:  1;
		uint64_t hint		:  2;
		uint64_t x6			:  6;
		uint64_t m			:  1;
		uint64_t opcode		:  4;
	} M2;

	struct {
		uint64_t unused		: 23;
		uint64_t qp			:  6;
		uint64_t r1			:  7;
		uint64_t imm7b		:  7;
		uint64_t r3			:  7;
		uint64_t i			:  1;
		uint64_t hint		:  2;
		uint64_t x6			:  6;
		uint64_t s			:  1;
		uint64_t opcode		:  4;
	} M3;
#define GET_M3_IMM(x)		( (((x)->M3.i << 7) | ((x)->M3.imm7b)) |	\
							  (((x)->M3.s ? -1 : 0) << 8) )
#define SET_M3_IMM(x,y)		( assert( -(1<<8) <= (y) && (y) < (1<<8) ),	\
							  ((x)->M3.s = ((y) < 0)),					\
							  ((x)->M3.i = ((y) >> 7)),					\
							  ((x)->M3.imm7b = (y)) )

	struct {
		uint64_t unused		: 23;
		uint64_t qp			:  6;
		uint64_t 			:  7;
		uint64_t r2			:  7;
		uint64_t r3			:  7;
		uint64_t x			:  1;
		uint64_t hint		:  2;
		uint64_t x6			:  6;
		uint64_t m			:  1;
		uint64_t opcode		:  4;
	} M4;

	struct {
		uint64_t unused		: 23;
		uint64_t qp			:  6;
		uint64_t imm7a		:  7;
		uint64_t r2			:  7;
		uint64_t r3			:  7;
		uint64_t i			:  1;
		uint64_t hint		:  2;
		uint64_t x6			:  6;
		uint64_t s			:  1;
		uint64_t opcode		:  4;
	} M5;
#define GET_M5_IMM(x)		( (((x)->M5.i << 7) | ((x)->M5.imm7a)) |	\
							  (((x)->M5.s ? -1 : 0) << 8) )
#define SET_M5_IMM(x,y)		( assert( -(1<<8) <= (y) && (y) < (1<<8) ),	\
							  ((x)->M5.s = ((y) < 0)),					\
							  ((x)->M5.i = ((y) >> 7)),					\
							  ((x)->M5.imm7a = (y)) )

	struct {
		uint64_t unused		: 23;
		uint64_t qp			:  6;
		uint64_t f1			:  7;
		uint64_t 			:  7;
		uint64_t r3			:  7;
		uint64_t x			:  1;
		uint64_t hint		:  2;
		uint64_t x6			:  6;
		uint64_t m			:  1;
		uint64_t opcode		:  4;
	} M6;

	struct {
		uint64_t unused		: 23;
		uint64_t qp			:  6;
		uint64_t f1			:  7;
		uint64_t r2			:  7;
		uint64_t r3			:  7;
		uint64_t x			:  1;
		uint64_t hint		:  2;
		uint64_t x6			:  6;
		uint64_t m			:  1;
		uint64_t opcode		:  4;
	} M7;

	struct {
		uint64_t unused		: 23;
		uint64_t qp			:  6;
		uint64_t f1			:  7;
		uint64_t imm7b		:  7;
		uint64_t r3			:  7;
		uint64_t i			:  1;
		uint64_t hint		:  2;
		uint64_t x6			:  6;
		uint64_t s			:  1;
		uint64_t opcode		:  4;
	} M8;
#define GET_M8_IMM(x)		( (((x)->M8.i << 7) | ((x)->M8.imm7b)) |	\
							  (((x)->M8.s ? -1 : 0) << 8) )
#define SET_M8_IMM(x,y)		( assert( -(1<<8) <= (y) && (y) < (1<<8) ),	\
							  ((x)->M8.s = ((y) < 0)),					\
							  ((x)->M8.i = (y) >> 7),					\
							  ((x)->M8.imm7b = (y)) )

	struct {
		uint64_t unused		: 23;
		uint64_t qp			:  6;
		uint64_t 			:  7;
		uint64_t f2			:  7;
		uint64_t r3			:  7;
		uint64_t x			:  1;
		uint64_t hint		:  2;
		uint64_t x6			:  6;
		uint64_t m			:  1;
		uint64_t opcode		:  4;
	} M9;

	struct {
		uint64_t unused		: 23;
		uint64_t qp			:  6;
		uint64_t imm7a		:  7;
		uint64_t f2			:  7;
		uint64_t r3			:  7;
		uint64_t i			:  1;
		uint64_t hint		:  2;
		uint64_t x6			:  6;
		uint64_t s			:  1;
		uint64_t opcode		:  4;
	} M10;
#define GET_M10_IMM(x)		( (((x)->M10.i << 7) | ((x)->M10.imm7a)) |	\
							  (((x)->M10.s ? -1 : 0) << 8) )
#define SET_M10_IMM(x,y)	( assert( -(1<<8) <= (y) && (y) < (1<<8) ),	\
							  ((x)->M10.s = ((y) < 0)),					\
							  ((x)->M10.i = (y) >> 7),					\
							  ((x)->M10.imm7a = (y)) )

	struct {
		uint64_t unused		: 23;
		uint64_t qp			:  6;
		uint64_t f1			:  7;
		uint64_t f2			:  7;
		uint64_t r3			:  7;
		uint64_t x			:  1;
		uint64_t hint		:  2;
		uint64_t x6			:  6;
		uint64_t m			:  1;
		uint64_t opcode		:  4;
	} M11;

	struct {
		uint64_t unused		: 23;
		uint64_t qp			:  6;
		uint64_t f1			:  7;
		uint64_t f2			:  7;
		uint64_t r3			:  7;
		uint64_t x			:  1;
		uint64_t hint		:  2;
		uint64_t x6			:  6;
		uint64_t m			:  1;
		uint64_t opcode		:  4;
	} M12;

	struct {
		uint64_t unused		: 23;
		uint64_t qp			:  6;
		uint64_t 			: 14;
		uint64_t r3			:  7;
		uint64_t x			:  1;
		uint64_t hint		:  2;
		uint64_t x6			:  6;
		uint64_t m			:  1;
		uint64_t opcode		:  4;
	} M13;

	struct {
		uint64_t unused		: 23;
		uint64_t qp			:  6;
		uint64_t 			:  7;
		uint64_t r2			:  7;
		uint64_t r3			:  7;
		uint64_t x			:  1;
		uint64_t hint		:  2;
		uint64_t x6			:  6;
		uint64_t m			:  1;
		uint64_t opcode		:  4;
	} M14;

	struct {
		uint64_t unused		: 23;
		uint64_t qp			:  6;
		uint64_t 			:  7;
		uint64_t imm7b		:  7;
		uint64_t r3			:  7;
		uint64_t i			:  1;
		uint64_t hint		:  2;
		uint64_t x6			:  6;
		uint64_t s			:  1;
		uint64_t opcode		:  4;
	} M15;
#define GET_M15_IMM(x)		( (((x)->M15.i << 7) | ((x)->M15.imm7b)) |	\
							  (((x)->M15.s ? -1 : 0) << 8) )
#define SET_M15_IMM(x,y)	( assert( -(1<<8) <= (y) && (y) < (1<<8) ),	\
							  ((x)->M15.s = ((y) < 0)),					\
							  ((x)->M15.i = (y) >> 7),					\
							  ((x)->M15.imm7b = (y)) )

	struct {
		uint64_t unused		: 23;
		uint64_t qp			:  6;
		uint64_t r1			:  7;
		uint64_t r2			:  7;
		uint64_t r3			:  7;
		uint64_t x			:  1;
		uint64_t hint		:  2;
		uint64_t x6			:  6;
		uint64_t m			:  1;
		uint64_t opcode		:  4;
	} M16;

	struct {
		uint64_t unused		: 23;
		uint64_t qp			:  6;
		uint64_t r1			:  7;
		uint64_t i2b		:  2;
		uint64_t s			:  1;
		uint64_t 			:  4;
		uint64_t r3			:  7;
		uint64_t x			:  1;
		uint64_t hint		:  2;
		uint64_t x6			:  6;
		uint64_t m			:  1;
		uint64_t opcode		:  4;
	} M17;
#define GET_M17_INC(x)		( ((x)->M17.s?-1:1) * ((x)->M17.i2b == 3 ? 1 : (1 << (4 - (x)->M17.i2b))) )
#define SET_M17_INC(x,y)	( assert( (y) == -1 || (y) == -4 || (y) == -8 || (y) == -16 ||	\
									  (y) ==  1 || (y) ==  4 || (y) ==  8 || (y) ==  16 ),	\
							  ((x)->M17.s = ((y) < 0)),										\
							  ((x)->M17.i2b = ((y) == 1 || (y) == -1 ? 0x3 :				\
										  ((y) == 4 || (y) == -4 ? 0x2 :					\
										  ((y) == 8 || (y) == -8 ? 0x1 : 0x0)))) )

	struct {
		uint64_t unused		: 23;
		uint64_t qp			:  6;
		uint64_t f1			:  7;
		uint64_t r2			:  7;
		uint64_t 			:  7;
		uint64_t x			:  1;
		uint64_t 			:  2;
		uint64_t x6			:  6;
		uint64_t m			:  1;
		uint64_t opcode		:  4;
	} M18;

	struct {
		uint64_t unused		: 23;
		uint64_t qp			:  6;
		uint64_t r1			:  7;
		uint64_t f2			:  7;
		uint64_t 			:  7;
		uint64_t x			:  1;
		uint64_t 			:  2;
		uint64_t x6			:  6;
		uint64_t m			:  1;
		uint64_t opcode		:  4;
	} M19;

	struct {
		uint64_t unused		: 23;
		uint64_t qp			:  6;
		uint64_t imm7a		:  7;
		uint64_t r2			:  7;
		uint64_t imm13c		: 13;
		uint64_t x3			:  3;
		uint64_t s			:  1;
		uint64_t opcode		:  4;
	} M20;
#define GET_M20_TARGET(x)	( (((x)->M20.imm13c << 11) | ((x)->M20.imm7a << 4)) |			\
							  (((x)->M20.s ? -1 : 0) << 24) )
#define SET_M20_TARGET(x,y)	( assert( -(1<<24) <= (y) && (y) < (1<<24) && !((y) & 0xF) ),	\
							  ((x)->M20.s = ((y) < 0)),										\
							  ((x)->M20.imm13c = (y) >> 11),								\
							  ((x)->M20.imm7a = (y) >> 4) )

	struct {
		uint64_t unused		: 23;
		uint64_t qp			:  6;
		uint64_t imm7a		:  7;
		uint64_t f2			:  7;
		uint64_t imm13c		: 13;
		uint64_t x3			:  3;
		uint64_t s			:  1;
		uint64_t opcode		:  4;
	} M21;
#define GET_M21_TARGET(x)	( (((x)->M21.imm13c << 11) | ((x)->M21.imm7a << 4)) |			\
							  (((x)->M21.s ? -1 : 0) << 24) )
#define SET_M21_TARGET(x,y)	( assert( -(1<<24) <= (y) && (y) < (1<<24) && !((y) & 0xF) ),	\
							  ((x)->M21.s = ((y) < 0)),										\
							  ((x)->M21.imm13c = (y) >> 11),								\
							  ((x)->M21.imm7a = (y) >> 4) )

	struct {
		uint64_t unused		: 23;
		uint64_t qp			:  6;
		uint64_t r1			:  7;
		uint64_t imm20b		: 20;
		uint64_t x3			:  3;
		uint64_t s			:  1;
		uint64_t opcode		:  4;
	} M22;
#define GET_M22_TARGET(x)	( ((x)->M22.imm20b << 4) | ((x)->M22.s ? -1 : 0) << 24 )
#define SET_M22_TARGET(x,y)	( assert( -(1<<24) <= (y) && (y) < (1<<24) && !((y) & 0xF) ),	\
							  ((x)->M22.s = ((y) < 0)),										\
							  ((x)->M22.imm20b = (y) >> 4) )

	struct {
		uint64_t unused		: 23;
		uint64_t qp			:  6;
		uint64_t f1			:  7;
		uint64_t imm20b		: 20;
		uint64_t x3			:  3;
		uint64_t s			:  1;
		uint64_t opcode		:  4;
	} M23;
#define GET_M23_TARGET(x)	( ((x)->M23.imm20b << 4) | ((x)->M23.s ? -1 : 0) << 24 )
#define SET_M23_TARGET(x,y)	( assert( -(1<<24) <= (y) && (y) < (1<<24) && !((y) & 0xF) ),	\
							  ((x)->M23.s = ((y) < 0)),										\
							  ((x)->M23.imm20b = (y) >> 4) )

	struct {
		uint64_t unused		: 23;
		uint64_t qp			:  6;
		uint64_t 			: 21;
		uint64_t x4			:  4;
		uint64_t x2			:  2;
		uint64_t x3			:  3;
		uint64_t 			:  1;
		uint64_t opcode		:  4;
	} M24;

	struct {
		uint64_t unused		: 23;
		uint64_t zero		:  6;
		uint64_t 			: 21;
		uint64_t x4			:  4;
		uint64_t x2			:  2;
		uint64_t x3			:  3;
		uint64_t 			:  1;
		uint64_t opcode		:  4;
	} M25;

	struct {
		uint64_t unused		: 23;
		uint64_t qp			:  6;
		uint64_t r1			:  7;
		uint64_t 			: 14;
		uint64_t x4			:  4;
		uint64_t x2			:  2;
		uint64_t x3			:  3;
		uint64_t 			:  1;
		uint64_t opcode		:  4;
	} M26;

	struct {
		uint64_t unused		: 23;
		uint64_t qp			:  6;
		uint64_t f1			:  7;
		uint64_t 			: 14;
		uint64_t x4			:  4;
		uint64_t x2			:  2;
		uint64_t x3			:  3;
		uint64_t 			:  1;
		uint64_t opcode		:  4;
	} M27;

	struct {
		uint64_t unused		: 23;
		uint64_t qp			:  6;
		uint64_t 			: 14;
		uint64_t r3			:  7;
		uint64_t x6			:  6;
		uint64_t x3			:  3;
		uint64_t x			:  1;
		uint64_t opcode		:  4;
	} M28;

	struct {
		uint64_t unused		: 23;
		uint64_t qp			:  6;
		uint64_t 			:  7;
		uint64_t r2			:  7;
		uint64_t ar3		:  7;
		uint64_t x6			:  6;
		uint64_t x3			:  3;
		uint64_t 			:  1;
		uint64_t opcode		:  4;
	} M29;

	struct {
		uint64_t unused		: 23;
		uint64_t qp			:  6;
		uint64_t 			:  7;
		uint64_t imm7b		:  7;
		uint64_t ar3		:  7;
		uint64_t x4			:  4;
		uint64_t x2			:  2;
		uint64_t x3			:  3;
		uint64_t s			:  1;
		uint64_t opcode		:  4;
	} M30;
#define GET_M30_IMM(x)		( (x)->M30.imm7b | ((x)->M30.s ? -1 : 0) << 7)
#define SET_M30_IMM(x,y)	( ((x)->M30.s = (y) < 0), \
							  ((x)->M30.imm7b = (y)) )

	struct {
		uint64_t unused		: 23;
		uint64_t qp			:  6;
		uint64_t r1			:  7;
		uint64_t 			:  7;
		uint64_t ar3		:  7;
		uint64_t x6			:  6;
		uint64_t x3			:  3;
		uint64_t 			:  1;
		uint64_t opcode		:  4;
	} M31;

	struct {
		uint64_t unused		: 23;
		uint64_t qp			:  6;
		uint64_t 			:  7;
		uint64_t r2			:  7;
		uint64_t cr3		:  7;
		uint64_t x6			:  6;
		uint64_t x3			:  3;
		uint64_t 			:  1;
		uint64_t opcode		:  4;
	} M32;

	struct {
		uint64_t unused		: 23;
		uint64_t qp			:  6;
		uint64_t r1			:  7;
		uint64_t 			:  7;
		uint64_t cr3		:  7;
		uint64_t x6			:  6;
		uint64_t x3			:  3;
		uint64_t 			:  1;
		uint64_t opcode		:  4;
	} M33;

	struct {
		uint64_t unused		: 23;
		uint64_t qp			:  6;
		uint64_t r1			:  7;
		uint64_t sof		:  7;
		uint64_t sol		:  7;
		uint64_t sor		:  4;
		uint64_t 			:  2;
		uint64_t x3			:  3;
		uint64_t 			:  1;
		uint64_t opcode		:  4;
	} M34;
#define GET_M34_LOCAL(x)	( (x)->M34.sol )
#define GET_M34_OUTPUT(x)	( (x)->M34.sof - (x)->M34.sol )
#define GET_M34_ROTATE(x)	( (x)->M34.sor << 3 )
#define SET_M34_FIELDS(x, l, o, r)	( assert( !(((l) + (o)) >> 7) && !((r) >> 7) && !((r) & 0x7) ),	\
									  (x)->M34.sol = (l),											\
									  (x)->M34.sof = (l) + (o),										\
									  (x)->M34.sor = (r) >> 3 )

	struct {
		uint64_t unused		: 23;
		uint64_t qp			:  6;
		uint64_t 			:  7;
		uint64_t r2			:  7;
		uint64_t 			:  7;
		uint64_t x6			:  6;
		uint64_t x3			:  3;
		uint64_t 			:  1;
		uint64_t opcode		:  4;
	} M35;

	struct {
		uint64_t unused		: 23;
		uint64_t qp			:  6;
		uint64_t r1			:  7;
		uint64_t 			: 14;
		uint64_t x6			:  6;
		uint64_t x3			:  3;
		uint64_t 			:  1;
		uint64_t opcode		:  4;
	} M36;

	struct {
		uint64_t unused		: 23;
		uint64_t qp			:  6;
		uint64_t imm20a		: 20;
		uint64_t 			:  1;
		uint64_t x4			:  4;
		uint64_t x2			:  2;
		uint64_t x3			:  3;
		uint64_t i			:  1;
		uint64_t opcode		:  4;
	} M37;
#define GET_M37_IMM(x)		( ((x)->M37.i << 20) | ((x)->M37.imm20a) )
#define SET_M37_IMM(x,y)	( assert( !((y) >> 21) ),	\
							  ((x)->M37.i = (y) >> 20),	\
							  ((x)->M37.imm20a = (y)) )

	struct {
		uint64_t unused		: 23;
		uint64_t qp			:  6;
		uint64_t r1			:  7;
		uint64_t r2			:  7;
		uint64_t r3			:  7;
		uint64_t x6			:  6;
		uint64_t x3			:  3;
		uint64_t 			:  1;
		uint64_t opcode		:  4;
	} M38;

	struct {
		uint64_t unused		: 23;
		uint64_t qp			:  6;
		uint64_t r1			:  7;
		uint64_t i2b		:  2;
		uint64_t 			:  5;
		uint64_t r3			:  7;
		uint64_t x6			:  6;
		uint64_t x3			:  3;
		uint64_t 			:  1;
		uint64_t opcode		:  4;
	} M39;
#define GET_M39_IMM(x)		( (x)->M39.i2b )
#define SET_M39_IMM(x,y)	( assert( !((y) >> 2) ), \
							  (x)->M39.i2b )

	struct {
		uint64_t unused		: 23;
		uint64_t qp			:  6;
		uint64_t 			:  7;
		uint64_t i2b		:  2;
		uint64_t 			:  5;
		uint64_t r3			:  7;
		uint64_t x6			:  6;
		uint64_t x3			:  3;
		uint64_t 			:  1;
		uint64_t opcode		:  4;
	} M40;
#define GET_M40_IMM(x)		( (x)->M40.i2b )
#define SET_M40_IMM(x,y)	( assert( !((y) >> 2) ), \
							  (x)->M40.i2b )

	struct {
		uint64_t unused		: 23;
		uint64_t qp			:  6;
		uint64_t 			:  7;
		uint64_t r2			:  7;
		uint64_t 			:  7;
		uint64_t x6			:  6;
		uint64_t x3			:  3;
		uint64_t 			:  1;
		uint64_t opcode		:  4;
	} M41;

	struct {
		uint64_t unused		: 23;
		uint64_t qp			:  6;
		uint64_t 			:  7;
		uint64_t r2			:  7;
		uint64_t r3			:  7;
		uint64_t x6			:  6;
		uint64_t x3			:  3;
		uint64_t 			:  1;
		uint64_t opcode		:  4;
	} M42;

	struct {
		uint64_t unused		: 23;
		uint64_t qp			:  6;
		uint64_t r1			:  7;
		uint64_t 			:  7;
		uint64_t r3			:  7;
		uint64_t x6			:  6;
		uint64_t x3			:  3;
		uint64_t 			:  1;
		uint64_t opcode		:  4;
	} M43;

	struct {
		uint64_t unused		: 23;
		uint64_t qp			:  6;
		uint64_t imm21a		: 21;
		uint64_t x4			:  4;
		uint64_t i2d		:  2;
		uint64_t x3			:  3;
		uint64_t i			:  1;
		uint64_t opcode		:  4;
	} M44;
#define GET_M44_IMM(x)		( ((x)->M44.i << 23) | ((x)->M44.i2d << 21) | (x)->M44.imm21a )
#define SET_M44_IMM(x,y)	( assert( !((y) >> 24) ),		\
							  ((x)->M44.i = (y) >> 23),		\
							  ((x)->M44.i2d = (y) >> 21),	\
							  ((x)->M44.imm21a = (y)) )

	struct {
		uint64_t unused		: 23;
		uint64_t qp			:  6;
		uint64_t 			:  7;
		uint64_t r2			:  7;
		uint64_t r3			:  7;
		uint64_t x6			:  6;
		uint64_t x3			:  3;
		uint64_t 			:  1;
		uint64_t opcode		:  4;
	} M45;

	struct {
		uint64_t unused		: 23;
		uint64_t qp			:  6;
		uint64_t r1			:  7;
		uint64_t 			:  7;
		uint64_t r3			:  7;
		uint64_t x6			:  6;
		uint64_t x3			:  3;
		uint64_t 			:  1;
		uint64_t opcode		:  4;
	} M46;

	struct {
		uint64_t unused		: 23;
		uint64_t qp			:  6;
		uint64_t 			: 14;
		uint64_t r3			:  7;
		uint64_t x6			:  6;
		uint64_t x3			:  3;
		uint64_t 			:  1;
		uint64_t opcode		:  4;
	} M47;

	struct {
		uint64_t unused		: 23;
		uint64_t qp			:  6;
		uint64_t imm20a		: 20;
		uint64_t y			:  1;
		uint64_t x4			:  4;
		uint64_t x2			:  2;
		uint64_t x3			:  3;
		uint64_t i			:  1;
		uint64_t opcode		:  4;
	} M48;
#define GET_M48_IMM(x)		( ((x)->M48.i << 20) | ((x)->M48.imm20a) )
#define SET_M48_IMM(x,y)	( assert( !((y) >> 21) ),	\
							  ((x)->M48.i = (y) >> 20),	\
							  ((x)->M48.imm20a = (y)) )

	struct {
		uint64_t unused		: 23;
		uint64_t qp			:  6;
		uint64_t btype		:  3;
		uint64_t 			:  3;
		uint64_t p			:  1;
		uint64_t imm20b		: 20;
		uint64_t wh			:  2;
		uint64_t d			:  1;
		uint64_t s			:  1;
		uint64_t opcode		:  4;
	} B1;
#define GET_B1_TARGET(x)	( ((x)->B1.imm20b << 4) | ((x)->B1.s ? -1 : 0) << 24 )
#define SET_B1_TARGET(x,y)	( assert( -(1<<24) <= (y) && (y) < (1<<24) && !((y) & 0xF) ),	\
							  ((x)->B1.s = ((y) < 0)),										\
							  ((x)->B1.imm20b = (y) >> 4) )

	struct {
		uint64_t unused		: 23;
		uint64_t zero		:  6;
		uint64_t btype		:  3;
		uint64_t 			:  3;
		uint64_t p			:  1;
		uint64_t imm20b		: 20;
		uint64_t wh			:  2;
		uint64_t d			:  1;
		uint64_t s			:  1;
		uint64_t opcode		:  4;
	} B2;
#define GET_B2_TARGET(x)	( ((x)->B2.imm20b << 4) | ((x)->B2.s ? -1 : 0) << 24 )
#define SET_B2_TARGET(x,y)	( assert( -(1<<24) <= (y) && (y) < (1<<24) && !((y) & 0xF) ),	\
							  ((x)->B2.s = ((y) < 0)),										\
							  ((x)->B2.imm20b = (y) >> 4) )

	struct {
		uint64_t unused		: 23;
		uint64_t qp			:  6;
		uint64_t b1			:  3;
		uint64_t 			:  3;
		uint64_t p			:  1;
		uint64_t imm20b		: 20;
		uint64_t wh			:  2;
		uint64_t d			:  1;
		uint64_t s			:  1;
		uint64_t opcode		:  4;
	} B3;
#define GET_B3_TARGET(x)	( ((x)->B3.imm20b << 4) | ((x)->B3.s ? -1 : 0) << 24 )
#define SET_B3_TARGET(x,y)	( assert( -(1<<24) <= (y) && (y) < (1<<24) && !((y) & 0xF) ),	\
							  ((x)->B3.s = ((y) < 0)),										\
							  ((x)->B3.imm20b = (y) >> 4) )

	struct {
		uint64_t unused		: 23;
		uint64_t qp			:  6;
		uint64_t btype		:  3;
		uint64_t 			:  3;
		uint64_t p			:  1;
		uint64_t b2			:  3;
		uint64_t 			: 11;
		uint64_t x6			:  6;
		uint64_t wh			:  2;
		uint64_t d			:  1;
		uint64_t 			:  1;
		uint64_t opcode		:  4;
	} B4;

	struct {
		uint64_t unused		: 23;
		uint64_t qp			:  6;
		uint64_t b1			:  3;
		uint64_t 			:  3;
		uint64_t p			:  1;
		uint64_t b2			:  3;
		uint64_t 			: 16;
		uint64_t wh			:  3;
		uint64_t d			:  1;
		uint64_t 			:  1;
		uint64_t opcode		:  4;
	} B5;

	struct {
		uint64_t unused		: 23;
		uint64_t 			:  3;
		uint64_t wh			:  2;
		uint64_t 			:  1;
		uint64_t timm7a		:  7;
		uint64_t imm20b		: 20;
		uint64_t t2e		:  2;
		uint64_t ih			:  1;
		uint64_t s			:  1;
		uint64_t opcode		:  4;
	} B6;
#define GET_B6_TARGET(x)	( ((x)->B6.imm20b << 4) | ((x)->B6.s ? -1 : 0) << 24 )
#define SET_B6_TARGET(x,y)	( assert( -(1<<24) <= (y) && (y) < (1<<24) && !((y) & 0xF) ),	\
							  ((x)->B6.s = ((y) < 0)),										\
							  ((x)->B6.imm20b = (y) >> 4) )
#define GET_B6_TAG(x)		( (((x)->B6.t2e << 11) | ((x)->B6.timm7a << 4)) |				\
							  (((x)->B6.t2e >> 1 ? -1 : 0) << 12) )
#define SET_B6_TAG(x,y)		( assert( -(1<<12) <= (y) && (y) < (1<<12) && !((y) & 0xF) ),	\
							  ((x)->B6.t2e = (y) >> 11),									\
							  ((x)->B6.timm7a = (y) >> 4) )

	struct {
		uint64_t unused		: 23;
		uint64_t 			:  3;
		uint64_t wh			:  2;
		uint64_t 			:  1;
		uint64_t timm7a		:  7;
		uint64_t b2			:  3;
		uint64_t 			: 11;
		uint64_t x6			:  6;
		uint64_t t2e		:  2;
		uint64_t ih			:  1;
		uint64_t 			:  1;
		uint64_t opcode		:  4;
	} B7;
#define GET_B7_TAG(x)		( (((x)->B7.t2e << 11) | ((x)->B7.timm7a << 4)) |				\
							  (((x)->B7.t2e >> 1 ? -1 : 0) << 12) )
#define SET_B7_TAG(x,y)		( assert( -(1<<12) <= (y) && (y) < (1<<12) && !((y) & 0xF) ),	\
							  ((x)->B7.t2e = (y) >> 11),									\
							  ((x)->B7.timm7a = (y) >> 4) )

	struct {
		uint64_t unused		: 23;
		uint64_t zero		:  6;
		uint64_t 			: 21;
		uint64_t x6			:  6;
		uint64_t 			:  4;
		uint64_t opcode		:  4;
	} B8;

	struct {
		uint64_t unused		: 23;
		uint64_t qp			:  6;
		uint64_t imm20a		: 20;
		uint64_t 			:  1;
		uint64_t x6			:  6;
		uint64_t 			:  3;
		uint64_t i			:  1;
		uint64_t opcode		:  4;
	} B9;
#define GET_B9_IMM(x)		( ((x)->B9.i << 20) | (x)->B9.imm20a )
#define SET_B9_IMM(x,y)		( assert( !((y) >> 21) ),	\
							  ((x)->B9.i = (y) >> 20),	\
							  ((x)->B9.imm20a = (y)) )

	struct {
		uint64_t unused		: 23;
		uint64_t qp			:  6;
		uint64_t f1			:  7;
		uint64_t f2			:  7;
		uint64_t f3			:  7;
		uint64_t f4			:  7;
		uint64_t sf			:  2;
		uint64_t x			:  1;
		uint64_t opcode		:  4;
	} F1;

	struct {
		uint64_t unused		: 23;
		uint64_t qp			:  6;
		uint64_t f1			:  7;
		uint64_t f2			:  7;
		uint64_t f3			:  7;
		uint64_t f4			:  7;
		uint64_t x2			:  2;
		uint64_t x			:  1;
		uint64_t opcode		:  4;
	} F2;

	struct {
		uint64_t unused		: 23;
		uint64_t qp			:  6;
		uint64_t f1			:  7;
		uint64_t f2			:  7;
		uint64_t f3			:  7;
		uint64_t f4			:  7;
		uint64_t 			:  2;
		uint64_t x			:  1;
		uint64_t opcode		:  4;
	} F3;

	struct {
		uint64_t unused		: 23;
		uint64_t qp			:  6;
		uint64_t p1			:  6;
		uint64_t ta			:  1;
		uint64_t f2			:  7;
		uint64_t f3			:  7;
		uint64_t p2			:  6;
		uint64_t ra			:  1;
		uint64_t sf			:  2;
		uint64_t rb			:  1;
		uint64_t opcode		:  4;
	} F4;

	struct {
		uint64_t unused		: 23;
		uint64_t qp			:  6;
		uint64_t p1			:  6;
		uint64_t ta			:  1;
		uint64_t f2			:  7;
		uint64_t fclass7c	:  7;
		uint64_t p2			:  6;
		uint64_t sf			:  2;
		uint64_t 			:  2;
		uint64_t opcode		:  4;
	} F5;
#define GET_F5_FCLASS(x)	( ((x)->F5.fclass7c << 2) | (x)->F5.fc2 )
#define SET_F5_FCLASS(x,y)	( assert( !((y) >> 9) ),			\
							  ((x)->F5.fclass7c = (y) >> 2),	\
							  ((x)->F5.fc2 = (y)) )

	struct {
		uint64_t unused		: 23;
		uint64_t qp			:  6;
		uint64_t f1			:  7;
		uint64_t f2			:  7;
		uint64_t f3			:  7;
		uint64_t p2			:  6;
		uint64_t x			:  1;
		uint64_t sf			:  2;
		uint64_t q			:  1;
		uint64_t opcode		:  4;
	} F6;

	struct {
		uint64_t unused		: 23;
		uint64_t qp			:  6;
		uint64_t f1			:  7;
		uint64_t 			:  7;
		uint64_t f3			:  7;
		uint64_t p2			:  6;
		uint64_t x			:  1;
		uint64_t sf			:  2;
		uint64_t q			:  1;
		uint64_t opcode		:  4;
	} F7;

	struct {
		uint64_t unused		: 23;
		uint64_t qp			:  6;
		uint64_t f1			:  7;
		uint64_t f2			:  7;
		uint64_t f3			:  7;
		uint64_t x6			:  6;
		uint64_t x			:  1;
		uint64_t sf			:  2;
		uint64_t 			:  1;
		uint64_t opcode		:  4;
	} F8;

	struct {
		uint64_t unused		: 23;
		uint64_t qp			:  6;
		uint64_t f1			:  7;
		uint64_t f2			:  7;
		uint64_t f3			:  7;
		uint64_t x6			:  6;
		uint64_t x			:  1;
		uint64_t 			:  3;
		uint64_t opcode		:  4;
	} F9;

	struct {
		uint64_t unused		: 23;
		uint64_t qp			:  6;
		uint64_t f1			:  7;
		uint64_t f2			:  7;
		uint64_t 			:  7;
		uint64_t x6			:  6;
		uint64_t x			:  1;
		uint64_t sf			:  2;
		uint64_t 			:  1;
		uint64_t opcode		:  4;
	} F10;

	struct {
		uint64_t unused		: 23;
		uint64_t qp			:  6;
		uint64_t f1			:  7;
		uint64_t f2			:  7;
		uint64_t 			:  7;
		uint64_t x6			:  6;
		uint64_t x			:  1;
		uint64_t 			:  3;
		uint64_t opcode		:  4;
	} F11;

	struct {
		uint64_t unused		: 23;
		uint64_t qp			:  6;
		uint64_t 			:  7;
		uint64_t amask7b	:  7;
		uint64_t omask7c	:  7;
		uint64_t x6			:  6;
		uint64_t x			:  1;
		uint64_t sf			:  2;
		uint64_t 			:  1;
		uint64_t opcode		:  4;
	} F12;
#define SET_F12_AMASK(x)	( (x)->F12.amask7b)
#define GET_F12_AMASK(x,y)	( assert( !((y) >> 7) ), \
							  ((x)->F12.amask7b = (y)) )
#define GET_F12_OMASK(x)	( (x)->F12.omask7c)
#define SET_F12_OMASK(x,y)	( assert( !((y) >> 7) ), \
							  ((x)->F12.omask7c = (y)) )

	struct {
		uint64_t unused		: 23;
		uint64_t qp			:  6;
		uint64_t 			: 21;
		uint64_t x6			:  6;
		uint64_t x			:  1;
		uint64_t sf			:  2;
		uint64_t 			:  1;
		uint64_t opcode		:  4;
	} F13;

	struct {
		uint64_t unused		: 23;
		uint64_t qp			:  6;
		uint64_t imm20a		: 20;
		uint64_t 			:  1;
		uint64_t x6			:  6;
		uint64_t x			:  1;
		uint64_t sf			:  2;
		uint64_t s			:  1;
		uint64_t opcode		:  4;
	} F14;
#define GET_F14_TARGET(x)	( ((x)->F14.imm20a << 4) | ((x)->F14.s ? -1 : 0) << 24 )
#define SET_F14_TARGET(x,y)	( assert( -(1<<24) <= (y) && (y) < (1<<24) && !((y) & 0xF) ),	\
							  ((x)->F14.s = ((y) < 0)),										\
							  ((x)->F14.imm20a = ((y) >> 4)) )

	struct {
		uint64_t unused		: 23;
		uint64_t qp			:  6;
		uint64_t imm20a		: 20;
		uint64_t 			:  1;
		uint64_t x6			:  6;
		uint64_t x			:  1;
		uint64_t 			:  2;
		uint64_t i			:  1;
		uint64_t opcode		:  4;
	} F15;
#define GET_F15_IMM(x)		( ((x)->F15.i << 20) | (x)->F15.imm20a )
#define SET_F15_IMM(x,y)	( assert( !((y) >> 21) ),	\
							  ((x)->F15.i = (y) >> 20),	\
							  ((x)->F15.imm20a = (y)) )

	struct {
		uint64_t unused		: 23;
		uint64_t qp			:  6;
		uint64_t imm20a		: 20;
		uint64_t y			:  1;
		uint64_t x6			:  6;
		uint64_t x			:  1;
		uint64_t 			:  2;
		uint64_t i			:  1;
		uint64_t opcode		:  4;
	} F16;
#define GET_F16_IMM(x)		( ((x)->F16.i << 20) | (x)->F16.imm20a )
#define SET_F16_IMM(x,y)	( assert( !((y) >> 21) ),	\
							  ((x)->F16.i = (y) >> 20),	\
							  ((x)->F16.imm20a = (y)) )

	struct {
		uint64_t unused		: 23;
		uint64_t qp			:  6;
		uint64_t imm20a		: 20;
		uint64_t 			:  1;
		uint64_t x6			:  6;
		uint64_t x3			:  3;
		uint64_t i			:  1;
		uint64_t opcode		:  4;
	} X1;
#define GET_X1_IMM(x,y)		( ((y)->L2.imm41 << 21) | ((x)->X1.i << 20) | ((x)->X1.imm20a) )
#define SET_X1_IMM(x,y,z)	( assert( !((z) >> 62) ),	\
							  ((x)->X1.i = (z) >> 20),	\
							  ((x)->X1.imm20a = (z)),	\
							  ((y)->L2.imm41 = (z) >> 21) )

	struct {
		uint64_t unused		: 23;
		uint64_t qp			:  6;
   		uint64_t r1			:  7;
		uint64_t imm7b		:  7;
		uint64_t vc			:  1;
		uint64_t ic			:  1;
		uint64_t imm5c		:  5;
		uint64_t imm9d		:  9;
		uint64_t i			:  1;
		uint64_t opcode		:  4;
	} X2;
#define GET_X2_IMM(x,y)		( ((x)->X2.i << 63) | ((y)->L2.imm41 << 22) | ((x)->X2.ic << 21) | \
							  ((x)->X2.imm5c << 16 ) | ((x)->X2.imm9d << 7) | ((x)->X2.imm7b) )
#define SET_X2_IMM(x,y,z)	( ((x)->X2.i = (z) >> 63),		\
							  ((x)->X2.ic = (z) >> 21),		\
							  ((x)->X2.imm5c = (z) >> 16),	\
							  ((x)->X2.imm9d = (z) >> 7),	\
							  ((x)->X2.imm7b = (z)),		\
							  ((y)->L2.imm41 = (z) >> 22) )

	struct {
		uint64_t unused		: 23;
		uint64_t qp			:  6;
		uint64_t btype		:  3;
		uint64_t 			:  3;
		uint64_t p			:  1;
		uint64_t imm20b		: 20;
		uint64_t wh			:  2;
		uint64_t d			:  1;
		uint64_t i			:  1;
		uint64_t opcode		:  4;
	} X3;
#define GET_X3_TARGET(x,y)		( ((x)->X3.i << 63) | ((y)->L1.imm39 << 24) | ((x)->X3.imm20b << 4) )
#define SET_X3_TARGET(x,y,z)	( assert( !((z) & 0xF) ),		\
								  ((x)->X3.i = (z) >> 63),		\
								  ((x)->X3.imm20b = (z) >> 4),	\
								  ((y)->L1.imm39 = (z) >> 24) )

	struct {
		uint64_t unused		: 23;
		uint64_t qp			:  6;
		uint64_t b1			:  3;
		uint64_t 			:  3;
		uint64_t p			:  1;
		uint64_t imm20b		: 20;
		uint64_t wh			:  2;
		uint64_t d			:  1;
		uint64_t i			:  1;
		uint64_t opcode		:  4;
	} X4;
#define GET_X4_TARGET(x,y)		( ((x)->X4.i << 63) | ((y)->L1.imm39 << 24) | ((x)->X4.imm20b << 4) )
#define SET_X4_TARGET(x,y,z)	( assert( !((z) & 0xF) ),		\
								  ((x)->X4.i = (z) >> 63),		\
								  ((x)->X4.imm20b = (z) >> 4),	\
								  ((y)->L1.imm39 = (z) >> 24) )

	struct {
		uint64_t unused		: 23;
		uint64_t qp			:  6;
		uint64_t imm20a		: 20;
		uint64_t y			:  1;
		uint64_t x6			:  6;
		uint64_t x3			:  3;
		uint64_t i			:  1;
		uint64_t opcode		:  4;
	} X5;
#define GET_X5_IMM(x,y)		( ((y)->L2.imm41 << 21) | ((x)->X5.i << 20) | ((x)->X5.imm20a) )
#define SET_X5_IMM(x,y,z)	( assert( !((z) >> 62) ),	\
							  ((x)->X5.i = (z) >> 20),	\
							  ((x)->X5.imm20a = (z)),	\
							  ((y)->L2.imm41 = (z) >> 21) )

	struct {
		uint64_t unused		: 23;
		uint64_t 			:  2;
		uint64_t imm39		: 39;
	} L1;

	struct {
		uint64_t unused		: 23;
		uint64_t imm41		: 41;
	} L2;

} insn_tmpl;

typedef union {
	uint64_t raw;

	struct {
		uint64_t 			:  1;
		uint64_t be			:  1;
		uint64_t up			:  1;
		uint64_t ac			:  1;
		uint64_t mfl		:  1;
		uint64_t mfh		:  1;
		uint64_t 			:  7;
		uint64_t ic			:  1;
		uint64_t i			:  1;
		uint64_t pk			:  1;
		uint64_t 			:  1;
		uint64_t dt			:  1;
		uint64_t dfl		:  1;
		uint64_t dfh		:  1;
		uint64_t sp			:  1;
		uint64_t pp			:  1;
		uint64_t di			:  1;
		uint64_t si			:  1;
		uint64_t db			:  1;
		uint64_t lp			:  1;
		uint64_t tb			:  1;
		uint64_t rt			:  1;
		uint64_t 			:  4;

		uint64_t cpl		:  2;
		uint64_t is			:  1;
		uint64_t mc			:  1;
		uint64_t it			:  1;
		uint64_t id			:  1;
		uint64_t da			:  1;
		uint64_t dd			:  1;
		uint64_t ss			:  1;
		uint64_t ri			:  2;
		uint64_t ed			:  1;
		uint64_t bn			:  1;
		uint64_t ia			:  1;
		uint64_t 			: 18;
	} PSR;

	struct {
		uint64_t sof		:  7;
		uint64_t sol		:  7;
		uint64_t sor		:  4;
		uint64_t rrb_gr		:  7;
		uint64_t rrb_fr		:  7;
		uint64_t rrb_pr		:  7;
		uint64_t			: 27;
	} CFM;

} reg_tmpl;

#endif
