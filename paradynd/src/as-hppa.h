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

/* branch back instruction */
#define RETURN_INSN	0xfffffff7

/* branch to first local pre insn mini-tramp */
#define LOCAL_PRE_BRANCH	0xfffffff8

/* branch to first global pre insn mini-tramp */
#define GLOBAL_PRE_BRANCH	0xfffffffa

/* branch to first local post insn mini-tramp */
#define LOCAL_POST_BRANCH	0xfffffffb

/* branch to first global post insn mini-tramp */
#define GLOBAL_POST_BRANCH	0xfffffffc

#endif
