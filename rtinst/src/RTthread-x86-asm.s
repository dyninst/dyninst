#include "RTconst.h"
/*
 * The X86 implementation reserves the virtual register #32 for holding
 * a pointer to the currect thread, thus allowing quck access the current
 * LWP and process.
 */

        .file   "doit.c"
        .text
.globl DYNINSTthreadIndexFAST
        .type   DYNINSTthreadIndexFAST, @function
DYNINSTthreadIndexFAST:
        pushl   %ebp
        movl    %esp, %ebp
        movl    124(%ebp), %eax
        popl    %ebp
        ret
        .size   DYNINSTthreadIndexFAST, .-DYNINSTthreadIndexFAST
        .section        .note.GNU-stack,"",@progbits
        .ident  "GCC: (GNU) 3.3 20030718 (Gentoo Linux 3.3-r1, propolice)"

