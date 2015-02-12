.toc
.csect .text[PR]
	.globl DYNINSTstaticHeap_1048576_textHeap_libSpace
	.globl .DYNINSTstaticHeap_1048576_textHeap_libSpace
.csect DYNINSTstaticHeap_1048576_textHeap_libSpace[DS]
DYNINSTstaticHeap_1048576_textHeap_libSpace:
	.long .DYNINSTstaticHeap_1048576_textHeap_libSpace, TOC[tc0], 0
.csect .text[PR]
.DYNINSTstaticHeap_1048576_textHeap_libSpace:
	blr
	.space 1048576
LT..DYNINSTstaticHeap_1048576_textHeap_libSpace:
	.long 0
	.byte 0,0,32,96,128,1,0,1
	.long LT..DYNINSTstaticHeap_1048576_textHeap_libSpace-.DYNINSTstaticHeap_1048576_textHeap_libSpace
	.short 43
	.byte "DYNINSTstaticHeap_1048576_textHeap_libSpace"
	.byte 31
_section_.text:
.csect .data[RW],3
	.long _section_.text

.section .note.GNU-stack,"",@progbits
