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

/* $Id: as-sparc.h,v 1.10 2002/09/17 20:08:01 bernat Exp $ */

#ifndef AS_SPARC_H
#define AS_SPARC_H


/* "pseudo" instructions that are placed in the tramp code for the inst funcs
 *   to patch up.   This must be invalid instructions (any instruction with
 *   its top 10 bits as 0 is invalid (technically UNIMP).
 */

/* place to put the ba,a insn to return to main code */
#define END_TRAMP	0x1

/* place to put call to inst primative */
#define CALL_PRIMITIVE	0x2

/* place to put arg to inst function */
#define PRIMITIVE_ARG	0x3

/* place to put the re-located instruction we replaced */
#define EMULATE_INSN	0x4

/* branch back instruction */
#define RETURN_INSN	0x7

/* branch to first local pre insn mini-tramp */
#define LOCAL_PRE_BRANCH	0x8

/* branch to first global pre insn mini-tramp */
#define GLOBAL_PRE_BRANCH	0xa

/* branch to first local post insn mini-tramp */
#define LOCAL_POST_BRANCH	0xb

/* branch to first global post insn mini-tramp */
#define GLOBAL_POST_BRANCH	0xc

/* branch back to the application if there is no instrumentation at 
   this point */
#define SKIP_PRE_INSN           0xd
#define SKIP_POST_INSN          0xe

#define UPDATE_COST_INSN        0xf

#define RECURSIVE_GUARD_ON_PRE_INSN   0x10
#define RECURSIVE_GUARD_OFF_PRE_INSN  0x11
#define RECURSIVE_GUARD_ON_POST_INSN  0x12
#define RECURSIVE_GUARD_OFF_POST_INSN 0x13

#define CONSERVATIVE_TRAMP_READ_CONDITION 0x14
#define CONSERVATIVE_TRAMP_WRITE_CONDITION 0x15

#define MT_POS_CALC 0x20

#endif
