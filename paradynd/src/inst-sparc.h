/*
 *  Copyright 1993 Jeff Hollingsworth.  All rights reserved.
 *
 */

/*
 * inst-sparc.h - Common definitions to the SPARC specific instrumentation code.
 *
 * $Log: inst-sparc.h,v $
 * Revision 1.1  1994/01/27 20:31:23  hollings
 * Iinital version of paradynd speaking dynRPC igend protocol.
 *
 * Revision 1.2  1993/06/22  19:00:01  hollings
 * global inst state.
 *
 * Revision 1.1  1993/03/19  22:51:05  hollings
 * Initial revision
 *
 *
 */


/* "pseudo" instructions that are placed in the tramp code for the inst funcs
 *   to patch up.   This must be invalid instructions (any instruction with
 *   its top 10 bits as 0 is invalid (technically UNIMP).
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
