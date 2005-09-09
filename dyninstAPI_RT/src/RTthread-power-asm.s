#include "RTconst.h"
#include "../../dyninstAPI/src/inst-power.h"
   .machine "push"      
   .machine "ppc"       
   .globl   DYNINSTthreadIndexFAST
   .globl   .DYNINSTthreadIndexFAST
DYNINSTthreadIndexFAST:
.DYNINSTthreadIndexFAST:
   mr       3,12
   br
   .globl   DYNINSTthreadContext
   .globl   .DYNINSTthreadContext
DYNINSTthreadContext:
.DYNINSTthreadContext:
   liu      3,0
   oril     3,3,49152
   l        4,0(3)
   l        3,4(3)
   cmpi     0,4,0
   beq      DYNINSTthreadContext_NOT_SPR
   mfspr    3,275
DYNINSTthreadContext_NOT_SPR:
   br
   .globl   DYNINSTthreadSelf
   .globl   .DYNINSTthreadSelf
DYNINSTthreadSelf:
.DYNINSTthreadSelf:
   liu      3,0
   oril     3,3,49152
   l        4,0(3)
   l        3,4(3)
   cmpi     0,4,0
   beq      DYNINSTthreadSelf_NOT_SPR
   mfspr    3,275
DYNINSTthreadSelf_NOT_SPR:
   l        3,200(3)
   br
   .globl tc_lock_lock
   .globl .tc_lock_lock
tc_lock_lock:
.tc_lock_lock:
   mflr     0
   stw      0,8(1)
   mr       10,3      # &t in R10
   bl       .DYNINSTthreadSelf
   cror     31,31,31
   lil      6,1       # r6 == 1
tc_lock_lock_loop:
   lwarx    4,0,10    # r4 == old value of mutex
   stwcx.   6,0,10    # t->mutex = 1
   bne-     tc_lock_lock_loop
   cmpwi    4,1
   bne-     tc_lock_lock_done
tc_lock_lock_failed:  # Someone else has the lock
   l        5,4(10)   # r5 == old tid value
	cmpw     5,3
	bne	tc_lock_lock_loop	# Don't have lock, try again
	lil	3,-2		# Already have lock, return error, -2 == DYNINST_DEAD_LOCK
	b	tc_lock_lock_ret
tc_lock_lock_done:		# Need to record new tid, return
	st	3,4(10)		# t->tid = pid
	lil	3,0		# Return 0
tc_lock_lock_ret:
   lwz 0,8(1)
   mtlr 0
   blr

   .globl tc_lock_trylock
   .globl .tc_lock_trylock
tc_lock_trylock:
.tc_lock_trylock:
   mflr	0
   stw	0,8(1)
   mr	10,3		# &t in R10
   bl	.DYNINSTthreadSelf
   cror	31,31,31
	lil	6,1		# r6 == 1
tc_lock_trylock_loop:
	lwarx	4,0,10		# r4 == old value of mutex
	stwcx.	6,0,10		# t->mutex = 1
	bne-	tc_lock_trylock_loop
	cmpwi	4,1
	bne-	tc_lock_trylock_done
tc_lock_trylock_failed:		# Someone else has the lock
	l	5,4(10)		# r5 == old tid value
	cmpw	5,3
	bne	tc_lock_trylock_live	# Don't have lock, try again
	lil	3,-2			# Already have lock, return error
	b	tc_lock_trylock_ret
tc_lock_trylock_live:
	lil	3,-1			# Someone else has lock, LIVE_LOCK == -1
	b	tc_lock_trylock_ret
tc_lock_trylock_done:		# Need to record new tid, return
	st	3,4(10)		# t->tid = pid
	lil	3,0		# Return 0
tc_lock_trylock_ret:
   lwz 0,8(1)
   mtlr 0
   blr
   .machine "pop"
