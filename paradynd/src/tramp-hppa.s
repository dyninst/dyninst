;/*
; * trampoline code to get from a code location to an inst. primative.
; *
; *    This code starts life in the controller process and moves into the
; *    appropriate inferior process via ptrace calls.
; *
; * $Log: tramp-hppa.s,v $
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
	.word	0xfffffffa	; GLOBAL_PRE_BRANCH
	.word	0xfffffff8	; LOCAL_PRE_BRANCH
	.word 	0xfffffff4	; EMULATE_INSN
	nop			; /* delay slot */
	nop			; /* extra nop for aggregate size */
	.word	0xfffffffc	; GLOBAL_POST_BRANCH
	.word	0xfffffffb	; LOCAL_POST_BRANCH
;	/* should update post insn cost of base tramp here */
	.word	0xfffffff7	; RETURN_INSN
	nop			; /* see if this prevents crash jkh 4/4/95 */
	.word	0xfffffff1	; END_TRAMP
