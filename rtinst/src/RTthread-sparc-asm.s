#include "RTconst.h"
/*
 * The Sparc implementation reserves the global register %g7 for holding
 * a pointer to the currect thread, thus allowing quck access the current
 * LWP and process.
 */

#undef ENTRY
#define ENTRY(x) \
        .seg   "text";\
        .align  4;\
        .global x; \
        .type   x, #function; \
x:

#undef SET_SIZE
#define SET_SIZE(x) \
        .size   x, (.-x)
/*  
// int DYNINSTthreadPosFAST(void); 
*/
        ENTRY(DYNINSTthreadPosFAST)
	retl
  	mov %g6, %o0
        SET_SIZE(DYNINSTthreadPosFAST)

/*  
   int DYNINSTthreadSaneLocalStorage(void);
   This is an experimental method designed to avoid segfaults when our
   instrumentation calls thr_getspecific while a thread is being deleted
   (so that its local storage has been partially destroyed). Returns non-zero
   if it is safe to call thr_getspecific, zero -- otherwise.

   In practice,	a good consistency check is:	
	(currthread->t_tls == 0 || currthread->t_tls->array != 0) that is
   either TLS has been fully initialized or not at all.
*/
        ENTRY(DYNINSTthreadSaneLocalStorage)
	ld	[ %g7 + 0x1c ], %o0
	cmp	%o0, 0
	bz,a	1f
	mov	1, %o0
	ld  [ %o0 + 4 ], %o0
1:		
	retl
	nop
        SET_SIZE(DYNINSTthreadSaneLocalStorage)

/* 
// void* DYNINST_curthread() 
*/
        ENTRY(DYNINST_curthread)
        retl
        mov %g7, %o0
        SET_SIZE(DYNINST_curthread)
	
/* 
// int tc_lock_lock(tc_lock_t *) 
*/
        ENTRY(tc_lock_lock)
        save    %sp,-96,%sp
        call    pthread_self,0     ! Result = %o0
        nop
        or      %g0,%o0,%i1

.tc_lock_lock_TEST:
	ldstub   [%i0], %o0
	cmp     %o0, 0
        be      .tc_lock_lock_DONE
        nop
        ld      [%i0+4],%o1              /* if (mp->tid == tid) */
        cmp     %o1,%i1
        bne     .tc_lock_lock_TEST /* spin */
	nop                              
        ret                              /* self dead lock */
        restore %g0,DYNINST_DEAD_LOCK,%o0
.tc_lock_lock_DONE:
        st      %i1,[%i0+4] ! volatile
        ret
        restore %g0,0,%o0
	SET_SIZE(tc_lock_lock)

/* 
// int tc_lock_trylock(tc_lock_t*)  
*/
        ENTRY(tc_lock_trylock)
        save    %sp,-96,%sp
        call    pthread_self,0     ! Result = %o0
        nop
        or      %g0,%o0,%i1
	ldstub   [%i0], %o0
	cmp     %o0, 0
        be      .tc_lock_trylock_DONE
        nop
        ld      [%i0+4],%o1              /* if (mp->tid == tid) */
	or      %g0, DYNINST_LIVE_LOCK, %i0
        cmp     %o1,%i1
        be,a    .tc_lock_trylock_LOCK 
	or       %g0, DYNINST_DEAD_LOCK, %i0
.tc_lock_trylock_LOCK:
        ret                              
        restore
.tc_lock_trylock_DONE:
        st      %i1,[%i0+4] ! volatile
        ret
        restore %g0,0,%o0
	SET_SIZE(tc_lock_trylock)
