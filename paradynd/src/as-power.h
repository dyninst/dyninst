
#ifndef AS_POWER_H
#define AS_POWER_H

/*
 * $Log: as-power.h,v $
 * Revision 1.1  1995/08/24 15:03:41  hollings
 * AIX/SP-2 port (including option for split instruction/data heaps)
 * Tracing of rexec (correctly spawns a paradynd if needed)
 * Added rtinst function to read getrusage stats (can now be used in metrics)
 * Critical Path
 * Improved Error reporting in MDL sematic checks
 * Fixed MDL Function call statement
 * Fixed bugs in TK usage (strings passed where UID expected)
 *
 *
 */

/*
 *  Copyright 1995 Jeff Hollingsworth.  All rights reserved.
 *
 */

/* "pseudo" instructions that are placed in the tramp code for the inst funcs
 *   to patch up.   This must be invalid instructions (any instruction with
 *   its top 6 bits as 0 is invalid (technically UNIMP?).
 *
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

#endif
