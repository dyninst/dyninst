/*
 * create an array to use as a heap for new instructions.
 *
 * .space	SYN_INST_BUF_SIZES 
 */
#include "../h/rtinst.h"

        .set noat
        .text

	.globl _DYNINSTtext
	.ent _DYNINSTtext 2
_DYNINSTtext:
	.space	1024 
	.end _DYNINSTtext
