/*
 * create an array to use as a heap for new instructions.
 *
 *    This file is required since AIX has instructions and data are in
 *       different segments that are not reachable with a PC relative
 *       branch.  On other systems this would not be a good idea since it
 *       means that the dyninst code would have to be inserted at the first 
 *       link (since the data and text segments are next to each other).
 *       However, AIX permits incremental re-linking and can insert new
 *       code into a existing text segment.
 */
#include "../h/rtinst.h"

.csect .text[PR]
	.globl DYNINSTtext
	.globl .DYNINSTtext
DYNINSTtext:
.DYNINSTtext:
	.space	SYN_INST_BUF_SIZE
