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
 * tramp-sparc.s,v
 *
 */
#include "as-power.h"

/*
 * This is the base where a tramp jumps off.
 *
 * - do global before local because global call DYNINSTinit.
 *
 */
.csect .data[RW]
	.globl baseTramp
	.globl _baseTramp
baseTramp:
_baseTramp:
	/* should update cost of base tramp here, but we don't have a
	   register to use!
	*/

	.long   SKIP_PRE_INSN		/* if no instrumentation, then skip */
	.long	GLOBAL_PRE_BRANCH
        nop				/* fill this in with instructions to */
	nop				/* compute the address of the vector */
	nop				/* of counter/timers for each thread */
	nop
	nop
	nop
	nop
	nop
	nop
	.long	LOCAL_PRE_BRANCH
	.long   UPDATE_COST_INSN
	nop
	nop
	nop
	.long 	EMULATE_INSN
	nop
        .long	SKIP_POST_INSN		/* if no instrumentation, then skip */
	.long	GLOBAL_POST_BRANCH
        nop				/* fill this in with instructions to */
	nop				/* compute the address of the vector */
	nop				/* of counter/timers for each thread */
	nop
	nop
	nop
	nop
	nop
	nop
	.long	LOCAL_POST_BRANCH
	.long	RETURN_INSN
	nop
	.long	END_TRAMP
