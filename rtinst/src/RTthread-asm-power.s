	.file	"RTthread-asm-power.c"
.toc
.csect .text[PR]
gcc2_compiled.:
__gnu_compiled_c:
.csect _RTthreadasmpower.rw_c[RO],3
	.align 2
LC..0:
	.byte "DYNINSTthreadPos for pid %d"
	.byte 10, 0
	.align 2
LC..3:
	.byte "Bad PID: %d"
	.byte 10, 0
	.align 2
LC..6:
	.byte "DyninstThreadPos: %d"
	.byte 10, 0
.toc
LC..1:
	.tc _iob.P64[TC],_iob[RW]+64
LC..2:
	.tc LC..0[TC],LC..0
LC..4:
	.tc LC..3[TC],LC..3
LC..5:
	.tc DYNINST_ThreadTids[TC],DYNINST_ThreadTids[RW]
LC..7:
	.tc LC..6[TC],LC..6
.csect .text[PR]
	.align 2
	.globl DYNINSTthreadPos
	.globl .DYNINSTthreadPos
.csect DYNINSTthreadPos[DS]
DYNINSTthreadPos:
	.long .DYNINSTthreadPos, TOC[tc0], 0
.csect .text[PR]
.DYNINSTthreadPos:
	.extern __mulh
	.extern __mull
	.extern __divss
	.extern __divus
	.extern __quoss
	.extern __quous
	mflr 0
	stw 31,-4(1)
	stw 0,8(1)
	stwu 1,-72(1)
	mr 31,1
	bl .DYNINSTthreadPosFAST
	cror 31,31,31
	mr 0,3
	stw 0,60(31)
	bl .DYNINST_initialize_once
	cror 31,31,31
	bl .DYNINSTthreadSelf
	cror 31,31,31
	mr 0,3
	stw 0,56(31)
	lwz 3,LC..1(2)
	lwz 4,LC..2(2)
	lwz 5,56(31)
	bl .fprintf
	cror 31,31,31
	lwz 0,56(31)
	cmpwi 0,0,0
	bc 12,1,L..3
	lwz 3,LC..1(2)
	lwz 4,LC..4(2)
	lwz 5,56(31)
	bl .fprintf
	cror 31,31,31
	bl .abort
	cror 31,31,31
L..3:
	lwz 0,60(31)
	cmpwi 0,0,0
	bc 4,1,L..4
	lwz 0,60(31)
	cmpwi 0,0,511
	bc 12,1,L..4
	lwz 9,LC..5(2)
	lwz 0,60(31)
	mr 11,0
	slwi 0,11,2
	lwzx 9,9,0
	lwz 0,56(31)
	cmpw 0,9,0
	bc 4,2,L..4
	lwz 0,60(31)
	mr 3,0
	b L..2
L..4:
	lwz 3,56(31)
	lwz 4,60(31)
	bl ._threadPos
	cror 31,31,31
	mr 0,3
	stw 0,60(31)
	lwz 3,60(31)
	lwz 4,56(31)
	bl .DYNINST_ThreadCreate
	cror 31,31,31
	lwz 3,LC..1(2)
	lwz 4,LC..7(2)
	lwz 5,60(31)
	bl .fprintf
	cror 31,31,31
	lwz 0,60(31)
	mr 3,0
	b L..2
L..2:
	lwz 1,0(1)
	lwz 0,8(1)
	mtlr 0
	lwz 31,-4(1)
	blr
LT..DYNINSTthreadPos:
	.long 0
	.byte 0,0,32,97,128,1,0,1
	.long LT..DYNINSTthreadPos-.DYNINSTthreadPos
	.short 16
	.byte "DYNINSTthreadPos"
	.byte 31
.csect _RTthreadasmpower.rw_c[RO],3
	.align 2
LC..8:
	.byte "Looping"
	.byte 10, 0
.toc
LC..9:
	.tc _iob.P64[TC],_iob[RW]+64
LC..10:
	.tc LC..8[TC],LC..8
.csect .text[PR]
	.align 2
	.globl DYNINSTloop
	.globl .DYNINSTloop
.csect DYNINSTloop[DS]
DYNINSTloop:
	.long .DYNINSTloop, TOC[tc0], 0
.csect .text[PR]
.DYNINSTloop:
	mflr 0
	stw 31,-4(1)
	stw 0,8(1)
	stwu 1,-64(1)
	mr 31,1
	lwz 3,LC..9(2)
	lwz 4,LC..10(2)
	bl .fprintf
	cror 31,31,31
	li 3,0
	b L..5
L..5:
	lwz 1,0(1)
	lwz 0,8(1)
	mtlr 0
	lwz 31,-4(1)
	blr
LT..DYNINSTloop:
	.long 0
	.byte 0,0,32,97,128,1,0,1
	.long LT..DYNINSTloop-.DYNINSTloop
	.short 11
	.byte "DYNINSTloop"
	.byte 31
.csect _RTthreadasmpower.rw_c[RO],3
	.align 2
LC..11:
	.byte "DYNINST_not_deleted: ret %d"
	.byte 10, 0
.toc
LC..12:
	.tc _iob.P64[TC],_iob[RW]+64
LC..13:
	.tc LC..11[TC],LC..11
.csect .text[PR]
	.align 2
	.globl DYNINST_not_deleted
	.globl .DYNINST_not_deleted
.csect DYNINST_not_deleted[DS]
DYNINST_not_deleted:
	.long .DYNINST_not_deleted, TOC[tc0], 0
.csect .text[PR]
.DYNINST_not_deleted:
	mflr 0
	stw 31,-4(1)
	stw 0,8(1)
	stwu 1,-72(1)
	mr 31,1
	xoris 9,11,0xffff
	xori 0,9,65534
	srawi 11,0,31
	xor 9,11,0
	subfc 9,11,9
	neg 0,9
	srwi 9,0,31
	stw 9,56(31)
	lwz 3,LC..12(2)
	lwz 4,LC..13(2)
	lwz 5,56(31)
	bl .fprintf
	cror 31,31,31
	lwz 0,56(31)
	mr 3,0
	b L..6
L..6:
	lwz 1,0(1)
	lwz 0,8(1)
	mtlr 0
	lwz 31,-4(1)
	blr
LT..DYNINST_not_deleted:
	.long 0
	.byte 0,0,32,97,128,1,0,1
	.long LT..DYNINST_not_deleted-.DYNINST_not_deleted
	.short 19
	.byte "DYNINST_not_deleted"
	.byte 31
	.align 2
	.globl DYNINSTthreadDeletePos
	.globl .DYNINSTthreadDeletePos
.csect DYNINSTthreadDeletePos[DS]
DYNINSTthreadDeletePos:
	.long .DYNINSTthreadDeletePos, TOC[tc0], 0
.csect .text[PR]
.DYNINSTthreadDeletePos:
	stw 31,-4(1)
	stwu 1,-32(1)
	mr 31,1
	li 11,-2
	mr 3,11
	b L..7
L..7:
	lwz 1,0(1)
	lwz 31,-4(1)
	blr
LT..DYNINSTthreadDeletePos:
	.long 0
	.byte 0,0,32,96,128,1,0,1
	.long LT..DYNINSTthreadDeletePos-.DYNINSTthreadDeletePos
	.short 22
	.byte "DYNINSTthreadDeletePos"
	.byte 31
	.align 2
	.globl tc_lock_init
	.globl .tc_lock_init
.csect tc_lock_init[DS]
tc_lock_init:
	.long .tc_lock_init, TOC[tc0], 0
.csect .text[PR]
.tc_lock_init:
	stw 31,-4(1)
	stwu 1,-32(1)
	mr 31,1
	stw 3,56(31)
	lwz 9,56(31)
	li 0,0
	stw 0,0(9)
	lwz 9,56(31)
	li 0,-1
	stw 0,4(9)
	li 3,0
	b L..8
L..8:
	lwz 1,0(1)
	lwz 31,-4(1)
	blr
LT..tc_lock_init:
	.long 0
	.byte 0,0,32,96,128,1,1,1
	.long 0
	.long LT..tc_lock_init-.tc_lock_init
	.short 12
	.byte "tc_lock_init"
	.byte 31
	.align 2
	.globl tc_lock_unlock
	.globl .tc_lock_unlock
.csect tc_lock_unlock[DS]
tc_lock_unlock:
	.long .tc_lock_unlock, TOC[tc0], 0
.csect .text[PR]
.tc_lock_unlock:
	stw 31,-4(1)
	stwu 1,-32(1)
	mr 31,1
	stw 3,56(31)
	lwz 9,56(31)
	li 0,-1
	stw 0,4(9)
	lwz 9,56(31)
	li 0,0
	stw 0,0(9)
	li 3,0
	b L..9
L..9:
	lwz 1,0(1)
	lwz 31,-4(1)
	blr
LT..tc_lock_unlock:
	.long 0
	.byte 0,0,32,96,128,1,1,1
	.long 0
	.long LT..tc_lock_unlock-.tc_lock_unlock
	.short 14
	.byte "tc_lock_unlock"
	.byte 31
	.align 2
	.globl tc_lock_destroy
	.globl .tc_lock_destroy
.csect tc_lock_destroy[DS]
tc_lock_destroy:
	.long .tc_lock_destroy, TOC[tc0], 0
.csect .text[PR]
.tc_lock_destroy:
	stw 31,-4(1)
	stwu 1,-32(1)
	mr 31,1
	stw 3,56(31)
	lwz 9,56(31)
	li 0,-1
	stw 0,4(9)
	lwz 9,56(31)
	li 0,0
	stw 0,0(9)
	li 3,0
	b L..10
L..10:
	lwz 1,0(1)
	lwz 31,-4(1)
	blr
LT..tc_lock_destroy:
	.long 0
	.byte 0,0,32,96,128,1,1,1
	.long 0
	.long LT..tc_lock_destroy-.tc_lock_destroy
	.short 15
	.byte "tc_lock_destroy"
	.byte 31
_section_.text:
.csect .data[RW],3
	.long _section_.text
