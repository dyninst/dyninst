/*
 * Copyright (c) 1996 Barton P. Miller
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

/* $Id: tramp-sparc.s,v 1.28 2003/07/18 20:06:50 schendel Exp $ */

/*
 * trampoline code to get from a code location to an inst. primitive.
 *
 *    This code starts life in the controller process and moves into the
 *    appropriate inferior process via ptrace calls.
 */
#include "as-sparc.h"

/*
 * mihai Tue Feb 29 14:17:35 CST 2000
 *
 * Trampoline structure:
 *
 * baseTramp
 *
 *   _savePreInsn
 *   . SKIP_PRE_INSN
 *   .
 *   .   GLOBAL_PRE_BRANCH
 *   .   RECURSIVE_GUARD_ON_PRE_INSN
 *   .     LOCAL_PRE_BRANCH
 *   .   RECURSIVE_GUARD_OFF_PRE_INSN
 *   .
 *   . UPDATE_COST_INSN
 *   _restorePreInst
 *
 * EMULATE_INSN
 *
 *   SKIP_POST_INSN
 *
 *     GLOBAL_POST_BRANCH
 *     _savePostInsn
 *     . RECURSIVE_GUARD_ON_POST_INSN
 *     .   LOCAL_POST_BRANCH
 *     . RECURSIVE_GUARD_OFF_POST_INSN
 *     _restorePostInsn
 *
 * RETURN_INSN
 * END_TRAMP
 *
 * Notes:
 * - SKIP_PRE_INSN jumps to UPDATE_COST_INSN
 * - SKIP_POST_INSN jumps to RETURN_INSN
 * - RECURSIVE_GUARD_ON_PRE_INSN jumps to RECURSIVE_GUARD_OFF_PRE_INSN + 3
 * - RECURSIVE_GUARD_ON_POST_INSN jumps to RECURSIVE_GUARD_OFF_POST_INSN + 3
 *
 */
	
/*
 * This is the base where a tramp jumps off.
 *
 * - do global before local because global call DYNINSTinit.
 *
 */
.data
	.global baseTramp
	.global	_baseTramp
baseTramp:
_baseTramp:
	/* should update cost of base tramp here, but we don't have a
	   register to use!
	*/
	.global baseTramp_savePreInsn
	.global	_baseTramp_savePreInsn
baseTramp_savePreInsn:
_baseTramp_savePreInsn:
	save  %sp, -120, %sp	/* saving registers before jumping to */
	.word   SKIP_PRE_INSN
	nop			/* delay slot for jump if there is no inst. */
	std  %g0, [ %fp + -8 ]	/* to a minitramp		      */
	std  %g2, [ %fp + -16 ]
	std  %g4, [ %fp + -24 ]
	std  %g6, [ %fp + -32 ]
/* #if defined(MT _THREAD) */
	/* Calculate the POS of the current thread and stick it somewhere (G6?)	*/
	.word MT_POS_CALC
	nop
	nop
	nop
	nop
/* #endif */
	.word RECURSIVE_GUARD_ON_PRE_INSN /* turn on the recursive guard : 7 instrs */
	nop
	nop
	nop
	nop
	nop
	nop
/* #if defined(MT _THREAD) */
	nop
	nop
/* #endif */
	.word	LOCAL_PRE_BRANCH
	nop
	.word RECURSIVE_GUARD_OFF_PRE_INSN /* turn off the recursive guard : 3 instrs */
	nop
	nop
/* #if defined(MT _THREAD) */
	nop
	nop
/* #endif */
	ldd  [ %fp + -8 ], %g0	/* restoring registers after coming   */
	ldd  [ %fp + -16 ], %g2	/* back from a minitramp	      */
	ldd  [ %fp + -24 ], %g4
	ldd  [ %fp + -32 ], %g6
	.word  UPDATE_COST_INSN /* updating the cost for this inst point */ 
	nop
	nop
	nop
	nop
	nop
	.global baseTramp_restorePreInsn
	.global	_baseTramp_restorePreInsn
baseTramp_restorePreInsn:
_baseTramp_restorePreInsn:
	restore
	.word 	EMULATE_INSN
	nop			/* second instruction if it's leaf */
	nop			/* delay slot */
	nop			/* if the previous instruction is a DCTI  */
	nop			/* this slot is needed for leaf procedure */
	nop			/* extra nop for aggregate size */
	nop			/* extra nop for set condition code insn */
	.word   SKIP_POST_INSN
	nop
	.global baseTramp_savePostInsn
	.global	_baseTramp_savePostInsn
baseTramp_savePostInsn:
_baseTramp_savePostInsn:
	save  %sp, -120, %sp	/* saving registers before jumping to */
	std  %g0, [ %fp + -8 ]	/* to a minitramp		      */
	std  %g2, [ %fp + -16 ]
	std  %g4, [ %fp + -24 ]
	std  %g6, [ %fp + -32 ]
	nop			/* fill this in with instructions to  */
	nop			/* compute the address of the vector  */
	nop			/* of counter/timers for each thread  */
	nop
	nop
	nop
	nop
	nop
	nop
/* #if defined(MT _THREAD) */
	.word MT_POS_CALC
	nop
	nop
	nop
	nop
/* #endif */
	.word RECURSIVE_GUARD_ON_POST_INSN /* turn on the recursive guard : 7 instrs */
	nop
	nop
	nop
	nop
	nop
	nop
/* #if defined(MT _THREAD) */
	nop
	nop
/* #endif */
	.word	LOCAL_POST_BRANCH
	nop
	.word RECURSIVE_GUARD_OFF_POST_INSN /* turn off the recursive guard : 3 instrs */
	nop
	nop
/* #if defined(MT _THREAD) */
	nop
	nop
/* #endif */
	ldd  [ %fp + -8 ], %g0	/* restoring registers after coming   */
	ldd  [ %fp + -16 ], %g2	/* back from a minitramp	      */
	ldd  [ %fp + -24 ], %g4
	ldd  [ %fp + -32 ], %g6
	.global baseTramp_restorePostInsn
	.global	_baseTramp_restorePostInsn
baseTramp_restorePostInsn:
_baseTramp_restorePostInsn:
	restore
	.word	RETURN_INSN
	nop			/* see if this prevents crash jkh 4/4/95 */
	nop
	.word	END_TRAMP



/* conservative base trampoline which also saves the condition codes 
   It is used for random instrumentation points in sun-sparc-solaris
*/

.data
	.global conservativeBaseTramp
	.global	_conservativeBaseTramp
conservativeBaseTramp:
_conservativeBaseTramp:
	/* should update cost of base tramp here, but we don't have a
	   register to use!
	*/
	.global conservativeBaseTramp_savePreInsn
	.global	_conservativeBaseTramp_savePreInsn
conservativeBaseTramp_savePreInsn:
_conservativeBaseTramp_savePreInsn:
	save  %sp, -256, %sp	/* saving registers before jumping to */
	.word   SKIP_PRE_INSN
	nop			/* delay slot for jump if there is no inst. */
	std  %g0, [ %fp + -8 ]	/* to a minitramp		      */
	std  %g2, [ %fp + -16 ]
	std  %g4, [ %fp + -24 ]
	std  %g6, [ %fp + -32 ] 
	std  %f0, [ %fp + -40 ] /* floating point registers are saved */
	std  %f2, [ %fp + -48 ]
	std  %f4, [ %fp + -56 ]
	std  %f6, [ %fp + -64 ]
	std  %f8, [ %fp + -72 ]
	std  %f10, [ %fp + -80 ]
	std  %f12, [ %fp + -88 ]
	std  %f14, [ %fp + -96 ]
	std  %f16, [ %fp + -104 ]
	std  %f18, [ %fp + -112 ]
	std  %f20, [ %fp + -120 ]
	std  %f22, [ %fp + -128 ]
	std  %f24, [ %fp + -136 ]
	std  %f26, [ %fp + -144 ]
	std  %f28, [ %fp + -152 ]
	std  %f30, [ %fp + -160 ]
	.word CONSERVATIVE_TRAMP_READ_CONDITION /* saving the condition codes */
	st   %g1, [ %fp + -164 ]
/* #if defined(MT _THREAD) */
	/* Calculate the POS of the current thread and stick it somewhere (G6?)	*/
	.word MT_POS_CALC
	nop
	nop
	nop
	nop
/* #endif */
	.word RECURSIVE_GUARD_ON_PRE_INSN /* turn on the recursive guard : 7 instrs */
	nop
	nop
	nop
	nop
	nop
	nop
/* #if defined(MT _THREAD) */
	nop
	nop
/* #endif */
	.word	LOCAL_PRE_BRANCH
	nop
	.word RECURSIVE_GUARD_OFF_PRE_INSN /* turn off the recursive guard : 3 instrs */
	nop
	nop
/* #if defined(MT _THREAD) */
	nop
	nop
/* #endif */
	ld   [ %fp + -164 ], %g1 /* restoring the value of condition codes */
	.word CONSERVATIVE_TRAMP_WRITE_CONDITION
	ldd   [ %fp + -160 ], %f30 /* restoring the floating point registers */
	ldd   [ %fp + -152 ], %f28
	ldd   [ %fp + -144 ], %f26
	ldd   [ %fp + -136 ], %f24
	ldd   [ %fp + -128 ], %f22
	ldd   [ %fp + -120 ], %f20
	ldd   [ %fp + -112 ], %f18
	ldd   [ %fp + -104 ], %f16
	ldd   [ %fp + -96 ], %f14
	ldd   [ %fp + -88 ], %f12
	ldd   [ %fp + -80 ], %f10
	ldd   [ %fp + -72 ], %f8
	ldd   [ %fp + -64 ], %f6
	ldd   [ %fp + -56 ], %f4
	ldd   [ %fp + -48 ], %f2
	ldd   [ %fp + -40 ], %f0
	ldd  [ %fp + -8 ], %g0	/* restoring registers after coming   */
	ldd  [ %fp + -16 ], %g2	/* back from a minitramp	      */
	ldd  [ %fp + -24 ], %g4
	ldd  [ %fp + -32 ], %g6
	.word  UPDATE_COST_INSN /* updating the cost for this inst point */ 
	nop
	nop
	nop
	nop
	nop
	.global conservativeBaseTramp_restorePreInsn
	.global	_conservativeBaseTramp_restorePreInsn
conservativeBaseTramp_restorePreInsn:
_conservativeBaseTramp_restorePreInsn:
	restore
	.word 	EMULATE_INSN
	nop			/* second instruction if it's leaf */
	nop			/* delay slot */
	nop			/* if the previous instruction is a DCTI  */
	nop			/* this slot is needed for leaf procedure */
	nop			/* extra nop for aggregate size */
	nop			/* extra nop for set condition code insn */
	.word   SKIP_POST_INSN
	nop
	.global conservativeBaseTramp_savePostInsn
	.global	_conservativeBaseTramp_savePostInsn
conservativeBaseTramp_savePostInsn:
_conservativeBaseTramp_savePostInsn:
	save  %sp, -256, %sp	/* saving registers before jumping to */
	std  %g0, [ %fp + -8 ]	/* to a minitramp		      */
	std  %g2, [ %fp + -16 ]
	std  %g4, [ %fp + -24 ]
	std  %g6, [ %fp + -32 ]
	std  %f0, [ %fp + -40 ] /* floating point registers are saved */
	std  %f2, [ %fp + -48 ]
	std  %f4, [ %fp + -56 ]
	std  %f6, [ %fp + -64 ]
	std  %f8, [ %fp + -72 ]
	std  %f10, [ %fp + -80 ]
	std  %f12, [ %fp + -88 ]
	std  %f14, [ %fp + -96 ]
	std  %f16, [ %fp + -104 ]
	std  %f18, [ %fp + -112 ]
	std  %f20, [ %fp + -120 ]
	std  %f22, [ %fp + -128 ]
	std  %f24, [ %fp + -136 ]
	std  %f26, [ %fp + -144 ]
	std  %f28, [ %fp + -152 ]
	std  %f30, [ %fp + -160 ]
	.word CONSERVATIVE_TRAMP_READ_CONDITION /* saving the condition codes */
	st   %g1, [ %fp + -164 ]
/* #if defined(MT _THREAD) */
	.word MT_POS_CALC
	nop
	nop
	nop
	nop
/* #endif */
	.word RECURSIVE_GUARD_ON_POST_INSN /* turn on the recursive guard : 7 instrs */
	nop
	nop
	nop
	nop
	nop
	nop
/* #if defined(MT _THREAD) */
	nop
	nop
/* #endif */
	.word	LOCAL_POST_BRANCH
	nop
	.word RECURSIVE_GUARD_OFF_POST_INSN /* turn off the recursive guard : 3 instrs */
	nop
	nop
/* #if defined(MT _THREAD) */
	nop
	nop
/* #endif */
	ld   [ %fp + -164 ], %g1 /* restoring the value of condition codes */
	.word CONSERVATIVE_TRAMP_WRITE_CONDITION
	ldd   [ %fp + -160 ], %f30 /* restoring the floating point registers */
	ldd   [ %fp + -152 ], %f28
	ldd   [ %fp + -144 ], %f26
	ldd   [ %fp + -136 ], %f24
	ldd   [ %fp + -128 ], %f22
	ldd   [ %fp + -120 ], %f20
	ldd   [ %fp + -112 ], %f18
	ldd   [ %fp + -104 ], %f16
	ldd   [ %fp + -96 ], %f14
	ldd   [ %fp + -88 ], %f12
	ldd   [ %fp + -80 ], %f10
	ldd   [ %fp + -72 ], %f8
	ldd   [ %fp + -64 ], %f6
	ldd   [ %fp + -56 ], %f4
	ldd   [ %fp + -48 ], %f2
	ldd   [ %fp + -40 ], %f0
	ldd  [ %fp + -8  ], %g0	/* restoring registers after coming   */ 
	ldd  [ %fp + -16 ], %g2	/* back from a minitramp	      */
	ldd  [ %fp + -24 ], %g4
	ldd  [ %fp + -32 ], %g6
	.global conservativeBaseTramp_restorePostInsn
	.global	_conservativeBaseTramp_restorePostInsn
conservativeBaseTramp_restorePostInsn:
_conservativeBaseTramp_restorePostInsn:
	restore
	.word	RETURN_INSN
	nop
	nop
	.word	END_TRAMP
