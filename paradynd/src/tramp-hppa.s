;/*
; * trampoline code to get from a code location to an inst. primative.
; *
; *    This code starts life in the controller process and moves into the
; *    appropriate inferior process via ptrace calls.
; *
; * $Log: tramp-hppa.s,v $
; * Revision 1.3  1996/04/26 19:54:40  lzheng
; * Removed the instruction words used to save the function argument registers.
; * They are saved in miniTrampoline now
; *
; * Revision 1.2  1996/04/08 21:14:12  lzheng
; * The working version of paradynd/HP
; *
; * Revision 1.1  1995/11/29 18:47:48  krisna
; * hpux/hppa instrumentation code
; *
; */
;
;#include "as-hppa.h"
;
;/*
; * This is the base where a tramp jumps off.
; *
; * - do global before local because global call DYNINSTinit.
; *
; */
;
; assembler on HPUX does not do macro processing
;

	.SPACE $PRIVATE$
	.SUBSPA $DATA$,QUAD=1,ALIGN=8,ACCESS=31
	.SUBSPA $BSS$,QUAD=1,ALIGN=8,ACCESS=31,ZERO,SORT=82
	.SPACE $TEXT$
	.SUBSPA $LIT$,QUAD=0,ALIGN=8,ACCESS=44
	.SUBSPA $CODE$,QUAD=0,ALIGN=8,ACCESS=44,CODE_ONLY
	.IMPORT $global$,DATA
	.IMPORT $$dyncall,MILLICODE

	.SPACE $TEXT$
	.SUBSPA $CODE$

	.align 4
	.EXPORT baseTramp,ENTRY,PRIV_LEV=3
baseTramp
_baseTramp
;	/* should update cost of base tramp here, but we don't have a
;	   register to use!
;	*/
	.word	0xffffff0f	; PREAMBLE_0    stack pointer
	.word	0xffffff1f	; PREAMBLE_1    save 31
	.word	0xffffff2f	; PREAMBLE_2    save 2
	.word   0xffffff3f	; PREAMBLE_3    save 3
	.word	0xffffff4f	; PREAMBLE_4    save 28
	.word	0xfffffffa	; GLOBAL_PRE_BRANCH
	.word	0xfffffff8	; LOCAL_PRE_BRANCH
	.word	0xfffffff9	; LOCAL_PRE_BRANCH_1
	.word   0xffffffaf      ; TRAILER_4     restore 28
	.word   0xffffffbf      ; TRAILER_3     restore 3
	.word	0xffffffcf	; TRAILER_2     restore 2
	.word	0xffffffdf	; TRAILER_1     restore 31
	.word	0xffffffef	; TRAILER_0 
	.word 	0xfffffff4	; EMULATE_INSN
	.word   0xfffffff5	; EMULATE_INSN_1 
	nop			; /* delay slot */
	nop			; /* extra nop for aggregate size */
	nop
	nop
	nop
	nop
	.word	0xffffff01	; PREAMBLE_00    stack pointer
	.word	0xffffff11	; PREAMBLE_11    save 31
	.word	0xffffff21	; PREAMBLE_22    save 2
	.word   0xffffff31	; PREAMBLE_33    save 3
	.word	0xffffff41	; PREAMBLE_44    save 28
	.word	0xfffffffc	; GLOBAL_POST_BRANCH
	.word	0xfffffffb	; LOCAL_POST_BRANCH
	.word	0xfffffffd	; LOCAL_POST_BRANCH_1
;	/* should update post insn cost of base tramp here */
	.word	0xffffffa1	; TRAILER_44	 restore 28
	.word   0xffffffb1      ; TRAILER_33     restore 3
	.word	0xffffffc1	; TRAILER_22     restore 2
	.word	0xffffffd1	; TRAILER_11     restore 31
	.word	0xffffffe1	; TRAILER_00     stack pointer
	.word	0xfffffff7	; RETURN_INSN
	.word   0xfffffff6	; RETURN_INSN_1
	nop			; /* see if this prevents crash jkh 4/4/95 */
	.word	0xfffffff1	; END_TRAMP











