/*
 * The X86 implementation reserves the virtual register #32 for holding
 * a pointer to the currect thread, thus allowing quck access the current
 * LWP and process.
 */

        .file   "RTthread-x86-asm.s"
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


/**
 * tc_lock_lock(tc_lock_t *tc)
 *
 * %ecx = tc
 *    (%ecx) = tc->mutex
 *   4(%ecx) = tc->tid
 * %eax = result of pthread_self()/Return value
 * %edx = tested value of mutex
**/
        .text
.globl tc_lock_lock
        .type tc_lock_lock, @function
tc_lock_lock:
        pushl %ebp
        movl  %esp,%ebp
/* eax = pthread_self() */
        call  pthread_self
        movl  8(%ebp),%ecx
        movl  $1,%edx
/*if (pthread_self() == tc->tid) return DYNINST_DEAD_LOCK;*/
        cmpl  %eax,4(%ecx)
        jne   .Lattemptlock
        movl  $-2,%eax
        leave
        ret
.Lattemptlock:
/*while (tc->mutex != 0);*/
        cmpl  $0,(%ecx)	
        jne   .Lattemptlock
/*<ATOMIC>if (tc->mutex == 0) tc->mutex = 1</ATOMIC> else goto attemptlock*/
        lock
        xchg  %edx,(%ecx)
        cmpl  $0,%edx
        jne   .Lattemptlock
/*tc->tid = pthread_self()*/
        movl  %eax,4(%ecx)
/*return 0*/
        movl  $0,%eax
        leave
        ret
.size tc_lock_lock, .-tc_lock_lock
