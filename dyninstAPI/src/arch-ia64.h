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

// $Id: arch-ia64.h,v 1.8 2002/08/01 18:37:28 tlmiller Exp $
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
		IA64_instruction( uint64_t insn = 0, uint8_t templ = 0xFF, const IA64_bundle * mybl = 0, uint8_t slotN = 3 );

		const void * ptr() const;
		uint32_t size() const { return 16; }

		uint64_t getMachineCode() { return instruction; }

		enum insnType { RETURN, CALL, BRANCH, OTHER };
		virtual insnType getType() const;

		virtual Address getTargetAddress();

	protected:
		const IA64_bundle * myBundle;

		uint64_t instruction;
		uint8_t templateID;
		uint8_t slotNumber;
	}; /* end the 41 bit instruction */

class IA64_instruction_x : public IA64_instruction {
	friend class IA64_bundle;

	public:
		IA64_instruction_x( uint64_t lowHalf = 0, uint64_t highHalf = 0, uint8_t templ = 0xFF, IA64_bundle * mybl = 0 );

		ia64_bundle_t getMachineCode() { ia64_bundle_t r = { instruction, instruction_x }; return r; }	

		virtual insnType getType() const;

		virtual Address getTargetAddress();

	private:
		uint64_t instruction_x;

	}; /* end the 82 bit instruction */

class IA64_bundle {
	public:
		IA64_bundle( uint64_t lowHalfBundle = 0, uint64_t highHalfBundle = 0 );
		IA64_bundle( uint8_t templateID, uint64_t instruction0, uint64_t instruction1, uint64_t instruction2 );
		IA64_bundle( uint8_t templateID, IA64_instruction & instruction0, IA64_instruction instruction1, IA64_instruction instruction2 );
		IA64_bundle( ia64_bundle_t rawBundle );
		IA64_bundle( uint8_t templateID, IA64_instruction & instruction0, IA64_instruction_x & instructionLX );

		ia64_bundle_t getMachineCode() const { return myBundle; }
		const ia64_bundle_t * getMachineCodePtr() const { return & myBundle; }

		uint8_t getTemplateID() { return templateID; }
		IA64_instruction getInstruction0() { return instruction0; }
		IA64_instruction getInstruction1() { return instruction1; }
		IA64_instruction getInstruction2() { return instruction2; }
		IA64_instruction getInstruction( unsigned int slot );

		bool hasLongInstruction() { return templateID == 0x05; }
		IA64_instruction_x getLongInstruction();

	private:
		IA64_instruction instruction0;
		IA64_instruction instruction1;
		IA64_instruction instruction2;
		uint8_t templateID;

		ia64_bundle_t myBundle;
	}; /* end the 128 bit bundle */

typedef IA64_instruction instruction;

#include "inst-ia64.h"

/* Required by symtab.h, which seems to use it to check
   _instruction_ alignment.  OTOH, it doesn't seem to very
   much paid attention to.  Anyway, IA-64 instruction bundles
   are always 16-byte aligned. */
inline bool isAligned( const Address address ) {
	return (address & 0xFFFFFFFFFFFFFFF0) == address;
	} /* end isAligned() */

/* Required by linux.C to find the address bounds
   of new dynamic heap segments. */
inline Address region_lo( const Address x ) {
	#warning WAG
	return 0x00000000;
	} /* end region_lo */

/* Required by linux.C to find the address bounds
   of new dynamic heap segments. */
inline Address region_hi( const Address x ) {
	#warning WAG
	return 0xf0000000;
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
IA64_instruction generateAlteredAlloc( uint64_t allocAddr, int deltaLocal, int deltaOutput, int deltaRotate );
IA64_instruction generateShortConstantInRegister( unsigned int registerN, int imm22 );
IA64_instruction_x generateLongConstantInRegister( unsigned int registerN, long long int imm64 );
IA64_instruction_x generateLongCallTo( long long int displacement64, unsigned int branchRegister );

/* Required by linuxDL.C */
IA64_instruction generateReturnTo( unsigned int branchRegister );
IA64_instruction_x generateLongBranchTo( long long int displacement64 );

/* Convience method for inst-ia64.C */
IA64_bundle generateBundleFromLongInstruction( IA64_instruction_x longInstruction );

/* Required by inst-ia64.C */
IA64_instruction generateIPToRegisterMove( Register destination );
IA64_instruction generateBranchToRegisterMove( Register source, Register destination );
IA64_instruction generateRegisterToBranchMove( Register source, Register destination );
IA64_instruction generateShortImmediateAdd( Register destination, int immediate, Register source );
IA64_instruction generateSubtraction( Register destination, Register lhs, Register rhs );
IA64_instruction generateIndirectCallTo( Register indirect, Register rp );
IA64_instruction generatePredicatesToRegisterMove( Register destination );
IA64_instruction generateRegisterToPredicatesMove( Register source, uint64_t imm17 );
IA64_instruction generateRegisterStore( Register address, Register destination );
IA64_instruction generateRegisterLoad( Register destination, Register address );

/* For ast.C */
class instPoint;
class registerSpace;
#define NUM_LOCALS 8
#define NUM_OUTPUT 8
void defineBaseTrampRegisterSpaceFor( const instPoint * location, registerSpace * regSpace );

/* Constants for code generation. */
#define ALIGN_RIGHT_SHIFT 23

#define BRANCH_SCRATCH	6	/* or 7 */
#define BRANCH_RETURN	0
#define REGISTER_GP	1
#define REGISTER_RP	8	/* return value, if structure/union, pointer. */
#define REGISTER_SP	12	/* memory stack pointer */
#define REGISTER_TP	13	/* thread pointer */

/* (right-aligned) Template IDs. */
const uint8_t MIIstop = 0x01;
const uint8_t MLXstop = 0x05;
const uint8_t MMIstop = 0x09;
const uint8_t MstopMIstop = 0x0B;
const uint8_t MMBstop = 0x19;

/* (left-aligned) Machine code. */
const uint64_t NOP_I = 0x0004000000000000;
const uint64_t NOP_M = 0x0004000000000000;
const uint64_t TRAP_M = 0x0000800000000000;

#endif
