#include "RTconst.h"
#include "../../dyninstAPI/src/inst-power.h"
   .machine "push"      
   .machine "ppc"       
   .globl   DYNINSTthreadIndexFAST
   .globl   .DYNINSTthreadIndexFAST
DYNINSTthreadIndexFAST:
.DYNINSTthreadIndexFAST:
   mr       3,12
   bclr     20,0
   .globl   DYNINSTthreadContext
   .globl   .DYNINSTthreadContext
DYNINSTthreadContext:
.DYNINSTthreadContext:
   addis    3,0,0
   ori      3,3,49152
   lwz      4,0(3)
   lwz      3,4(3)
   cmpi     0,4,0
   beq      DYNINSTthreadContext_NOT_SPR
   mfspr    3,275
DYNINSTthreadContext_NOT_SPR:
   bclr     20,0
   .globl   DYNINSTthreadSelf
   .globl   .DYNINSTthreadSelf
DYNINSTthreadSelf:
.DYNINSTthreadSelf:
   addis    3,0,0
   ori      3,3,49152
   lwz      4,0(3)
   lwz      3,4(3)
   cmpi     0,4,0
   beq      DYNINSTthreadSelf_NOT_SPR
   mfspr    3,275
DYNINSTthreadSelf_NOT_SPR:
   lwz      3,200(3)
   bclr     20,0
   .globl tc_lock_lock
   .globl .tc_lock_lock
tc_lock_lock:
.tc_lock_lock:
   mflr     0
   stw      0,8(1)
   mr       10,3      # &t in R10
   bl       .DYNINSTthreadSelf
   cror     31,31,31
   addi     6,0,1       # r6 == 1
tc_lock_lock_loop:
   lwarx    4,0,10    # r4 == old value of mutex
   stwcx.   6,0,10    # t->mutex = 1
   bne-     tc_lock_lock_loop
   cmpwi    4,1
   bne-     tc_lock_lock_done
tc_lock_lock_failed:  # Someone else has the lock
   lwz      5,4(10)   # r5 == old tid value
	cmpw     5,3
	bne	tc_lock_lock_loop	# Don't have lock, try again
	addi	3,0,-2		# Already have lock, return error, -2 == DYNINST_DEAD_LOCK
	b	tc_lock_lock_ret
tc_lock_lock_done:		# Need to record new tid, return
	stw	3,4(10)		# t->tid = pid
	addi	3,0,0		# Return 0
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
	addi	6,0,1		# r6 == 1
tc_lock_trylock_loop:
	lwarx	4,0,10		# r4 == old value of mutex
	stwcx.	6,0,10		# t->mutex = 1
	bne-	tc_lock_trylock_loop
	cmpwi	4,1
	bne-	tc_lock_trylock_done
tc_lock_trylock_failed:		# Someone else has the lock
	lwz	5,4(10)		# r5 == old tid value
	cmpw	5,3
	bne	tc_lock_trylock_live	# Don't have lock, try again
	addi	3,0,-2			# Already have lock, return error
	b	tc_lock_trylock_ret
tc_lock_trylock_live:
	addi	3,0,-1			# Someone else has lock, LIVE_LOCK == -1
	b	tc_lock_trylock_ret
tc_lock_trylock_done:		# Need to record new tid, return
	stw	3,4(10)		# t->tid = pid
	addi	3,0,0		# Return 0
tc_lock_trylock_ret:
   lwz 0,8(1)
   mtlr 0
   blr
   .machine "pop"
