#include "RTconst.h"
/*
 * The Sparc implementation reserves the global register %g7 for holding
 * a pointer to the currect thread, thus allowing quck access the current
 * LWP and process.
 */

/* #define PROFILE_BASETRAMP */

.seg "text"
#if !defined(ONE_THREAD)
	.align 4
  .global DYNINSTthreadPos
  .type DYNINSTthreadPos, #function
DYNINSTthreadPos:
	  save	%sp,-96,%sp
.L_DYNINSTthreadPos:
          call    .+8
          sethi   %hi((_GLOBAL_OFFSET_TABLE_-(.L_DYNINSTthreadPos-.))),%l1
          add     %l1,%lo((_GLOBAL_OFFSET_TABLE_-(.L_DYNINSTthreadPos-.))),%l1
          add     %l1,%o7,%i1
/*
	  call    DYNINSTthreadCheckRPC
	  nop
*/
          call    DYNINST_initialize_once     /* some initialization */     
          nop                                     
	  call    DYNINSTthreadSelf
	  nop
	  mov     %o0, %l3                     /* %l3 = DYNINSTthreadSelf */
	  cmp     %l3, 0                       /* if %l3 <= 0? */
          bg      .LDYNINSTthreadPos_TEST1     /* DYNINSTthreadSelf>=0 */
	  ld      [%i1+DYNINST_ThreadTids],%l2 /* l2 = DYNINST_ThreadTids */
          ret                                      
          restore %g0,-2,%o0                       

  .LDYNINSTthreadPos_TEST1:                                     
          cmp     %g6,0                            
          bl,a    .LDYNINSTthreadPos_TEST2                      
          nop                                
          cmp     %g6,MAX_NUMBER_OF_THREADS                     
          bge     .LDYNINSTthreadPos_SLOW                   
          nop                                   
          sll     %g6,2,%o0                      
          ld      [%o0+%l2],%o0 
          cmp     %l3,%o0                 /* tid <> DYNINST_ThreadTids[pos]*/  
          be,a    .LDYNINSTthreadPos_DONE                    
          mov     %g6, %i0                /* verified, return */
          ba      .LDYNINSTthreadPos_SLOW                    
          nop                                   

  .LDYNINSTthreadPos_TEST2:                                  
          cmp     %g6,-2                        
          bne     .LDYNINSTthreadPos_SLOW                    
          nop                                   
          call    lookup_removeList             
          or      %g0,%l3,%o0                   /* tid */
          cmp     %o0,0                         
          bne,a   .LDYNINSTthreadPos_DONE                     
          mov     -2, %i0                                   

  .LDYNINSTthreadPos_SLOW:                                   
	  mov     %g6, %o1
          call    _threadPos           
          or      %g0,%l3,%o0           /* tid */
          mov     %o0, %g6             /* %g6 = _threadPos(tid, pos) */
	  call    DYNINST_ThreadCreate /* %o0 is pos, %o1 is tid                 */
				       /* DYNINST_ThreadCreate defined in RTsolaris.c */
	  mov     %l3, %o1             
          sll     %g6,2,%l0                        
          st      %l3,[%l2+%l0] ! volatile         
	  std     %g6, [%sp +184]  ! make sure baseTramp will not mess it up
	                           ! 120+96-32
          mov     %g6,%i0 ! volatile              
  .LDYNINSTthreadPos_DONE:                                      
#ifdef PROFILE_BASETRAMP
          call btramp_total
	  nop
#endif PROFILE_BASETRAMP
	  ret                      
	  restore
#endif /* ONE_THREAD*/


/*  ---
 * void DYNINSTthreadDeletePos(void); 
 * MUST be called in the same frame as the baseTramp
 */
	.align  4
        .global DYNINSTthreadDeletePos
        .type  DYNINSTthreadDeletePos,#function 
DYNINSTthreadDeletePos:
  	mov -2, %g6 
	retl
	std %g6, [%fp - 32] ! make sure that we do not overwrites it when
			    ! baseTramp restores


/*  ---
 * void DYNINSTloop(void); 
 */
	.align  4
        .global DYNINSTloop
        .type  DYNINSTloop,#function 
DYNINSTloop:
	retl
  	mov 0, %o0

#if !defined(ONE_THREAD)
/*  
 * int DYNINSTthreadPosFAST(void); 
 */
	.align  4
        .global DYNINSTthreadPosFAST
        .type  DYNINSTthreadPosFAST,#function 
DYNINSTthreadPosFAST:
	retl
  	mov %g6, %o0
#endif

/* ---
 * int DYNINST_not_deleted(void) 
 */
#if !defined(ONE_THREAD)
	.align  4
        .global DYNINST_not_deleted
        .type  DYNINST_not_deleted,#function 
DYNINST_not_deleted:
	 mov 1, %l0 
	 cmp %g6, -2
	 be,a .L_DYNINST_not_deleted
	 mov 0, %l0
.L_DYNINST_not_deleted :
	 retl
	 mov %l0, %o0
#endif /*ONE_THREAD */


/* ---
 * int DYNINSTthreadSelf(void)
 */
        .align  4
        .global DYNINSTthreadSelf
        .type  DYNINSTthreadSelf,#function
DYNINSTthreadSelf:
	 save %sp, -96, %sp
	 call thr_self
	 nop
	 mov %o0, %i0
         ret
	 restore
	 /* ld  [%g7 + 0x30], %i0 ! call thr_self */


/* int tc_lock_init(tc_lock_t*) ; */
	.align 4
	.global tc_lock_init
	.type  tc_lock_init,#function
tc_lock_init:
	st      %g0,[%o0]  !  mp->mutex = 0  
        or      %g0,-1,%g1 !  mp->tid = -1 ;
	st      %g1,[%o0+4] ! volatile
	retl
	or      %g0,0,%o0

/* int tc_lock_lock(tc_lock_t *) */
	.global tc_lock_lock
	.type tc_lock_lock,#function
tc_lock_lock:
        save    %sp,-96,%sp
        call    DYNINSTthreadSelf,0     ! Result = %o0
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

/* int tc_lock_trylock(tc_lock_t*)  */
	.global tc_lock_trylock
	.type tc_lock_trylock,#function
tc_lock_trylock:
        save    %sp,-96,%sp
        call    DYNINSTthreadSelf,0     ! Result = %o0
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

/* int tc_lock_unlock(&tc_lock_t *) */
        .global tc_lock_unlock
        .type tc_lock_unlock,#function
tc_lock_unlock:
        or      %g0,-1,%g1
	st      %g1, [%o0+4]
	st      %g0, [%o0]
        retl
        mov     0,   %o0

/* int tc_lock_destroy(tc_lock_t *) */
        .global tc_lock_destroy
        .type tc_lock_destroy,#function
tc_lock_destroy:
        or      %g0,-1,%g1
        st      %g1,[%o0+4] ! volatile
        st      %g0,[%o0] ! volatile
        retl
        or      %g0,0,%o0


/* sparc_thread_t* DYNINST_curthread() */
        .global DYNINST_curthread
	.type DYNINST_curthread,#function
DYNINST_curthread:
	retl
	mov %g7, %o0

