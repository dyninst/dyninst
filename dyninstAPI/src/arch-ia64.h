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

// $Id: arch-ia64.h,v 1.2 2002/06/03 18:17:13 tlmiller Exp $
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

class IA64_instruction {
	friend class IA64_bundle;

	public:
		IA64_instruction( uint64_t insn = 0, uint8_t templ = 0, IA64_bundle * mybl = 0 );

		/* Required of instructions; deprecate ASAP, since
		   they don't have sensible semantics on the IA-64. */
		const void * ptr() const { return (void *)0; }
		uint32_t size() const { return 16; }

	private:
		IA64_bundle * myBundle;

		uint64_t instruction;
		uint8_t templateID;
	}; /* end the 41 bit instruction */

struct ia64_bundle_t {
	uint64_t low;
	uint64_t high;
	};

class IA64_bundle {
	public:
		IA64_bundle( uint64_t lowHalfBundle = 0, uint64_t highHalfBundle = 0 );
		IA64_bundle( uint8_t templateID, uint64_t instruction0, uint64_t instruction1, uint64_t instruction2 );

		ia64_bundle_t getMyBundle() const { return myBundle; }

	private:
		IA64_instruction instruction0;
		IA64_instruction instruction1;
		IA64_instruction instruction2;
		uint8_t templateID;

		ia64_bundle_t myBundle;

	}; /* end the 128 bit bundle */

typedef IA64_instruction instruction;

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


#endif
