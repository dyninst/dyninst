/*
 * create an array to use as a heap for new instructions.
 *
 */
#include "../h/rtinst.h"

        .set noat
        .text

	.globl _DYNINSTtext
	.ent _DYNINSTtext 2
_DYNINSTtext:
	.space	SYN_INST_BUF_SIZE
	.end _DYNINSTtext
