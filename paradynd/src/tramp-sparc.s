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

/*
 * trampoline code to get from a code location to an inst. primative.
 *
 *    This code starts life in the controller process and moves into the
 *    appropriate inferior process via ptrace calls.
 *
 * $Log: tramp-sparc.s,v $
 * Revision 1.13  1996/09/12 15:08:24  naim
 * This commit move all saves and restores from the mini-tramps to the base
 * tramp. It also add jumps to skip instrumentation in the base-tramp when
 * it isn't required - naim
 *
 * Revision 1.12  1996/08/20 18:57:59  lzheng
 * A few slot was added to allow multiple instructions to be moved into
 * the base trampoline
 *
 * Revision 1.11  1996/08/16 21:20:13  tamches
 * updated copyright for release 1.1
 *
 * Revision 1.10  1996/06/20 21:35:07  naim
 * Adding a nop at the delay slot to avoid "illegal instruction" bug - naim
 *
 * Revision 1.9  1995/08/24  15:04:45  hollings
# AIX/SP-2 port (including option for split instruction/data heaps)
# Tracing of rexec (correctly spawns a paradynd if needed)
# Added rtinst function to read getrusage stats (can now be used in metrics)
# Critical Path
# Improved Error reporting in MDL sematic checks
# Fixed MDL Function call statement
# Fixed bugs in TK usage (strings passed where UID expected)
#
# Revision 1.8  1994/11/02  19:01:26  hollings
# Made the observed cost model use a normal variable rather than a reserved
# register.
#
# Revision 1.7  1994/11/02  11:18:32  markc
# Commented out the cost model.
#
# Revision 1.6  1994/10/13  07:25:08  krisna
# solaris porting and updates
#
# Revision 1.5  1994/07/26  19:58:03  hollings
# removed slots executed counter.
#
# Revision 1.4  1994/07/14  23:30:33  hollings
# Hybrid cost model added.
#
# Revision 1.3  1994/07/06  00:35:45  hollings
# Added code to handle SPARC ABI aggregate return type calling convention
# of using the instruction after the call's delay slot to indicate aggregate
# size.  We treat this as an extra delay slot and relocate it to the
# base tramp as needed.
#
# Revision 1.2  1994/07/05  03:26:20  hollings
# observed cost model
#
# Revision 1.1  1994/01/27  20:31:47  hollings
# Iinital version of paradynd speaking dynRPC igend protocol.
#
 * Revision 1.6  1993/10/19  15:27:54  hollings
 * AST based mini-tramp code generator.
 *
 * Revision 1.5  1993/09/10  20:33:44  hollings
 * moved tramps to data area so the dyninst code can be run under qpt.
 *
 * Revision 1.4  1993/08/11  01:36:10  hollings * fixed include files.
 *
 * Revision 1.3  1993/06/22  19:00:01  hollings
 * global inst state.
 *
 * Revision 1.2  1993/03/19  22:52:54  hollings
 * fixed comment character.
 *
 * Revision 1.1  1993/03/19  22:51:50  hollings
 * Initial revision
 *
 */
#include "as-sparc.h"

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
	.word   SKIP_PRE_INSN
	nop			/* delay slot for jump if there is no inst. */
	.word	GLOBAL_PRE_BRANCH
	save  %sp, -112, %sp	/* saving registers before jumping to */
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
	.word	LOCAL_PRE_BRANCH
	nop
	ldd  [ %fp + -8 ], %g0	/* restoring registers after coming   */
	ldd  [ %fp + -16 ], %g2	/* back from a minitramp	      */
	ldd  [ %fp + -24 ], %g4
	ldd  [ %fp + -32 ], %g6
	restore
	nop
	.word 	EMULATE_INSN
	nop			/* second instruction if it's leaf */
	nop			/* delay slot */
	nop			/* if the previous instruction is a DCTI  */
	nop			/* this slot is needed for leaf procedure */
	nop			/* extra nop for aggregate size */
	.word   SKIP_POST_INSN
	nop
	.word	GLOBAL_POST_BRANCH
	save  %sp, -112, %sp	/* saving registers before jumping to */
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
	.word	LOCAL_POST_BRANCH
	nop
	ldd  [ %fp + -8 ], %g0	/* restoring registers after coming   */
	ldd  [ %fp + -16 ], %g2	/* back from a minitramp	      */
	ldd  [ %fp + -24 ], %g4
	ldd  [ %fp + -32 ], %g6
	restore
	nop
	/* should update post insn cost of base tramp here */
	.word	RETURN_INSN
	nop			/* see if this prevents crash jkh 4/4/95 */
	.word	END_TRAMP
