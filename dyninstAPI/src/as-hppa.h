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

#if !defined(hppa1_1_hp_hpux)
#error "invalid architecture-os inclusion"
#endif

#ifndef AS_SPARC_H
#define AS_SPARC_H

/* "pseudo" instructions that are placed in the tramp code for the inst funcs
 *   to patch up.   This must be invalid instructions
 */

/* place to put the ba,a insn to return to main code */
#define END_TRAMP	0xfffffff1

/* place to put call to inst primative */
#define CALL_PRIMITIVE	0xfffffff2

/* place to put arg to inst function */
#define PRIMITIVE_ARG	0xfffffff3

/* place to put the re-located instruction we replaced */
#define EMULATE_INSN	0xfffffff4

/* place to hold one more re-located instruction that be replaced 
   add by lzheng 1/19                             */
#define EMULATE_INSN_1  0xfffffff5

/* branch back instruction */
#define RETURN_INSN	0xfffffff7

/* Need two words to do the jump back. One of them is to modify
   the return address  */
#define RETURN_INSN_1	0xfffffff6

/* branch to first local pre insn mini-tramp */
#define LOCAL_PRE_BRANCH	0xfffffff8

/* branch to first local pre insn mini-tramp */
#define LOCAL_PRE_BRANCH_1      0xfffffff9

/* branch to first global pre insn mini-tramp */
#define GLOBAL_PRE_BRANCH	0xfffffffa

/* branch to first local post insn mini-tramp */
#define LOCAL_POST_BRANCH	0xfffffffb

/* branch to first local post insn mini-tramp */
#define LOCAL_POST_BRANCH_1     0xfffffffd

/* branch to first global post insn mini-tramp */
#define GLOBAL_POST_BRANCH	0xfffffffc

/*preambles and trailers*/
#define	PREAMBLE_0	       0xffffff0f  
#define	PREAMBLE_1             0xffffff1f
#define	PREAMBLE_2             0xffffff2f
#define PREAMBLE_3             0xffffff3f
#define PREAMBLE_4             0xffffff4f
#define	PREAMBLE_00	       0xffffff01  
#define	PREAMBLE_11            0xffffff11
#define	PREAMBLE_22            0xffffff21
#define PREAMBLE_33            0xffffff31
#define PREAMBLE_44            0xffffff41

#define	TRAILER_4              0xffffffaf
#define	TRAILER_3              0xffffffbf 
#define	TRAILER_2              0xffffffcf 
#define	TRAILER_1              0xffffffdf
#define	TRAILER_0              0xffffffef
#define	TRAILER_44             0xffffffa1 
#define	TRAILER_33             0xffffffb1 
#define	TRAILER_22             0xffffffc1 
#define	TRAILER_11             0xffffffd1 
#define	TRAILER_00             0xffffffe1	



#endif
