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
