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

// $Id: arch-ia64.C,v 1.2 2002/06/03 18:17:13 tlmiller Exp $
// ia64 instruction decoder

#include <assert.h>
#include "dyninstAPI/src/arch-ia64.h"

#define TEMPLATE_MASK		0x000000000000001F	/* bits 00 - 04 */
#define INSTRUCTION0_MASK	0x00003FFFFFFFFFE0	/* bits 05 - 45 */
#define INSTRUCTION1_LOW_MASK	0xFFFFC00000000000	/* bits 45 - 63 */
#define INSTRUCTION1_HIGH_MASK	0x00000000007FFFFF	/* bits 00 - 20 */
#define INSTRUCTION2_MASK	0xFFFFFFFFFF800000	/* bits 21 - 63 */

#define MAJOR_OPCODE_MASK	0xF800000000000000	/* bits 37 - 40 */
#define PREDICATE_MASK		0x000000000000003F	/* bits 00 - 05 */

#define ALIGN_RIGHT_SHIFT	23

IA64_instruction::IA64_instruction( uint64_t insn, uint8_t templ, IA64_bundle * mybl ) {
	instruction = insn;
	templateID = templ;
	myBundle = mybl;
	} /* end IA64_Instruction() */

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

/* debugging code. */
IA64_bundle check( myBundle.low, myBundle.high );
assert( check.templateID == templateID );
assert( check.instruction0.instruction == instruction0 );
assert( check.instruction1.instruction == instruction1 );
assert( check.instruction2.instruction == instruction2 );
	} /* end IA65_bundle() */

IA64_bundle::IA64_bundle( uint64_t lowHalfBundle, uint64_t highHalfBundle ) {
	/* The template is right-aligned; the instructions are left-aligned. */
	templateID = lowHalfBundle & TEMPLATE_MASK;
	instruction0 = IA64_instruction( (lowHalfBundle & INSTRUCTION0_MASK) << 18, templateID );
	instruction1 = IA64_instruction( ((lowHalfBundle & INSTRUCTION1_LOW_MASK) >> 23) +
						((highHalfBundle & INSTRUCTION1_HIGH_MASK) << 41), templateID );
	instruction2 = IA64_instruction( highHalfBundle & INSTRUCTION2_MASK, templateID );

	myBundle.low = lowHalfBundle;
	myBundle.high = highHalfBundle;
	} /* end IA64_Bundle() */



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

