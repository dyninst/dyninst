.file "test6LS-power.s"
.machine "any"

.toc
T.divarw:	.tc	divarw[TC], divarw[RW]
T.dfvars:	.tc	dfvars[TC], dfvars[RW]
T.dfvard:	.tc	dfvard[TC], dfvard[RW]

.lglobl divarw
.lglobl dfvars
.lglobl dfvard

.csect divarw[RW]
	.align 3
	.long 1234, 5678, 99, 11

.csect dfvars[RW]
	.align 2
	.float 1.25
	.float 1.75

.csect dfvard[RW]
	.align 3
	.double 1.125
	.double 1.725

.globl .loadsnstores[PR]
.csect .loadsnstores[PR]

#should use only volatile registres (GPR3-10, FPR0-13)

	add	12,3,4
	add	12,12,5

	la	7,T.divarw(2)
	mr	3,7
	lbz	6,0(7)
	lbzu	6,3(7)
	mr	7,3

	li	8,1
	lbzx	5,3,8
	li	9,2
	lbzux	5,3,9
	mr	3,7

	lhz	6,0(3)
	lha	5,4(3)
	lhzu	6,2(3)
	mr	3,7
	lhau    5,0(3)
	mr	3,7

	addi	8,8,1
	lhzx	5,7,9
	lhax	5,7,8
	lhzux	5,7,9
	mr	7,3
	lhaux	5,7,8
	mr	7,3

	l	5,0(7)
	lu	6,4(7)
	mr	7,3
	li	9,4
	lx	5,3,9
	lux	6,3,9
	mr	3,7

	# Not sure if lwa, lwax, lwaux, ld, ldu, ldx, ldux work on PowerX, X<3.

	lwa	5,4(3)
	lwax	6,7,0
	lwaux	5,7,8
	mr	7,3

	# Note:	 lwux does not exist...

	ld	6,0(7)
	ldu	6,0(3)
	mr	3,7
	li	9,0
	ldx	6,7,9
	ldux	5,3,9
	mr	3,7

	stb	5,3(7)
	stbu	5,1(7)
	mr	7,3
	li	8,1
	stbx	6,3,8
	stbux	6,8,3

	sth	5,2(7)
	sthu	6,6(7)
	mr	7,3
	li	9,2
	sthx	5,3,9
	sthux	6,9,3

	st	5,0(7)
	stu	6,4(7)
	mr	7,3
	li	9,4
	stx	6,3,9
	stux	5,9,3

	std	5,0(7)
	stdu	6,0(7)
	mr	7,3
	li	8,0
	stdx	6,7,8
	stdux	6,8,7

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

	lhbrx	6,8,3
	lbrx	6,9,3
	sthbrx	5,8,7
	stbrx	5,9,7

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

loop:	lwarx	5,0,3
	stwcx.	4,0,3
	bne-	loop
 
loop2:	ldarx	5,0,7
	stdcx.	4,0,7
	# VG(10/23/01) For some reason, reservation is always lost here
	# on Power3 in 32-executable [we still cannot do 64-bit ones...]
#	bne-	loop2

	la	4,T.dfvars(2)
	lfs	5,0(4)
	li	6,4
	lfsx	6,4,6
	mr	5,4
	lfsu	7,0(4)
	mr	4,5
	lfsux	8,6,4

	la	6,T.dfvard(2)
	lfd	1,0(6)
	li	9,8
	lfdx	2,6,9
	mr	7,6
	lfdu	1,8(6)
	mr	6,7
	lfdux	2,9,7

	stfs	1,4(4)
	li	0,4		# this could break prologue, bet we lack it...
	stfsx	2,4,0		# need to test RB=0 (not same as RA=0 on Power)
	stfsu	3,0(4)
	mr	4,5
	stfsux	3,0,4

	stfd	1,0(6)
	li	9,8
	stfdx	2,6,9
	stfdu	3,8(6)
	mr	6,7
	stfdux	4,9,7

	# The optional FP store
	stfiwx	5,0,4

	mr	3,12

	blr       # VG(10/19/01): dyninst doesn't find function return without
	.long 0x0 # this trailing 0... The function works ok without it...
