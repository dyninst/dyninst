# $Id: test6LS-power.s,v 1.4 2002/03/15 16:11:45 gaburici Exp $

.file "test6LS-power.s"
.machine "any"

.toc
T.divarw:	.tc	divarwT[TC], divarw[RW]
T.dfvars:	.tc	dfvarsT[TC], dfvars[RW]
T.dfvard:	.tc	dfvardT[TC], dfvard[RW]

.globl divarw[RW]
.globl dfvars[RW]
.globl dfvard[RW]

.csect divarw[RW], 3 # .align 3
	.long 1234, 5678, 99, 11, 0xFC000000

.csect dfvars[RW], 2 # .align 2
	.float 1.25
	.float 1.75

.csect dfvard[RW], 3 # .align 3
	.double 1.125
	.double 1.725

.globl .loadsnstores[PR]
.csect .loadsnstores[PR]

#should use only volatile registers (GPR3-10, FPR0-13)

	add	12,3,4
	add	12,12,5

	# We actually need to do a
	#l	7,T.divarw(2)
	# but avoid that because the offset is unknow(?)
	# afaict, the linker may reorder/combine .tc entries...

	la	7,T.divarw(2)
	l	7,0(7)		#l1

	mr	3,7
	lbz	6,17(7)		#l2
	lbzu	6,3(7)		#l3
	mr	7,3

	li	8,1
	lbzx	5,3,8		#l4
	li	9,2
	lbzux	5,3,9		#l5
	mr	3,7

	lhz	6,0(3)		#l6
	lha	5,4(3)		#l7
	lhzu	6,2(3)		#l8
	mr	3,7
	lhau    5,0(3)		#l9
	mr	3,7

	addi	8,8,1
	lhzx	5,7,9		#l10
	lhax	5,7,8		#l11
	lhzux	5,7,9		#l12
	mr	7,3
	lhaux	5,7,8		#l13
	mr	7,3

	l	5,0(7)		#l14
	lu	6,4(7)		#l15
	mr	7,3
	li	9,4
	lx	5,3,9		#l16
	lux	6,3,9		#l17
	mr	3,7

	# Not sure if lwa, lwax, lwaux, ld, ldu, ldx, ldux work on PowerX, X<3.

        li      0,0
	lwa	5,4(3)		#l18
	lwax	6,7,0		#l19
	lwaux	5,7,8		#l20
	mr	7,3

	# Note:	 lwux does not exist...

	ld	6,0(7)		#l21
	ldu	6,0(3)		#l22
	mr	3,7
	li	9,0
	ldx	6,7,9		#l23
	ldux	5,3,9		#l24
	mr	3,7

	stb	5,3(7)		#s1 a25
	stbu	5,1(7)
	mr	7,3
	li	8,1
	stbx	6,3,8
	stbux	6,8,3

	sth	5,2(7)		#s5
	sthu	6,6(7)
	mr	7,3
	li	9,2
	sthx	5,3,9
	sthux	6,9,3

	st	5,0(7)		#s9
	stu	6,4(7)
	mr	7,3
	li	9,4
	stx	6,3,9
	stux	5,9,3

	std	5,0(7)		#s13 a37
	stdu	6,0(7)
	mr	7,3
	li	8,0
	stdx	6,7,8
	stdux	6,8,7		#s16 a40

	li	9,0
	li	8,0
	
	# VG(10/25/01):	Using the offsets below instead of the above ones
	# causes the mutatee to crash if run standalone; running it from the 
	# mutator works however... It seems that in certain instances the
	# address for reverse-order instructions need be doubleword aligned
	# on Power3. (8,8) offests are ok as well. AFAICT this behavior is not
	# documented in the PowerPC book. I don't have Power3 documentation...

#	li	9,4
#	li	8,2

	lhbrx	6,8,3		# l25 a41
	lbrx	6,9,3		#
	sthbrx	5,8,7		# s17 a43
	stbrx	5,9,7		#

	.set    ngprs,32-13
	stm     13,-4*ngprs(1)	# Save lower words of GPR13-31 on stack
	lm      13,-4*ngprs(1)	# Restore them - stack pointer not touched

	addi	4,1,-24
	lsi	5,4,24		# Fill lw GPR5-10 with crap after stack
	stsi	5,4,20		# Write some crap back

	mfxer	10
	li	9,20
	.set	where,32+25
	insrdi	10,9,7,where
	mtxer	10		# set bits 25:31 of XER to 20

	li	9,-20
	lsx	5,1,9		# save lw GPR5-10 to stack
	li	9,-20
	stsx	5,1,9		# restore them

	mr	7,3

loop:	lwarx	5,0,3		# l30 a51
	stwcx.	4,0,3		# s22
	bne-	loop

	mr	4,1		# save stack pointer value in divarw
	
loop2:	ldarx	5,0,7
	stdcx.	4,0,7		# s23
	# VG(10/23/01) For some reason, reservation is always lost here
	# on Power3 in 32-executable [we still cannot do 64-bit ones...]
	# VG(11/22/01) Now it only happens with xlc mutatees but not with
	# gcc mutatees... Very odd...
#	bne-	loop2

	la	4,T.dfvars(2)
	l	4,0(4)		# a55
	
	lfs	5,0(4)
	li	6,4
	lfsx	6,4,6
	mr	5,4
	lfsu	7,0(4)
	mr	4,5
	lfsux	8,6,4

	la	6,T.dfvard(2)
	l	6,0(6)		# a60
	
	lfd	1,0(6)
	li	9,8
	lfdx	2,6,9
	mr	7,6
	lfdu	1,8(6)
	mr	6,7
	lfdux	2,9,7

	stfs	1,4(4)		# s24 a65
	li	0,4		# this could break prologue, bet we lack it...
	stfsx	2,4,0		# s25
	stfsu	3,0(4)		# s26
	mr	4,5
	stfsux	3,0,4		# s27

	stfd	1,0(6)		# s28 a69
	li	9,8
	stfdx	2,6,9
	stfdu	3,8(6)
	mr	6,7
	stfdux	4,9,7

	# The optional FP store
	stfiwx	5,0,4		# s32

	mr	3,12

	blr       # VG(10/19/01): dyninst doesn't find function return without
	.long 0x0 # this trailing 0... The function works ok without it...

	# VG(11/27/01):	 the trailing 0 is actually the start of a traceback
	# tag. I don't bother to generate one...

.globl .gettoc[PR]
.csect .gettoc[PR]

	# addi	3,2,T.divarw - This doesn't compile even tough it the same as:
	la	3,T.divarw(2)  # T.divarw is not 0 at execution time!
	blr
	.long 0x0


.globl .getsp[PR]
.csect .getsp[PR]

	l	7,T.divarw(2)	
	ld	3,0(7)  # get the SP of loadnstores (it was saved here)
	blr
	.long 0x0
