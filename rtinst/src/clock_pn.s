/*
 * clock_pe.s - assmebly routines for faster node clock access.
 *
 * $Log: clock_pn.s,v $
 * Revision 1.1  1993/12/13 19:46:50  hollings
 * Initial revision
 *
 *
 */
#define NI_BASE     		(0x20000000)
#define NI_TIME_A             (NI_BASE + 0x0070)

.global _DYNINSTstartWallTimer

_DYNINSTstartWallTimer:
1:
    ld  [ %o0 ], %g4				! C
    sethi  %hi(_timerBuffer), %g3		! N
    cmp  %g4, 0					! C
    bne  2f					! C
    or  %g3, %lo(_timerBuffer), %o1		! N
    mov  1, %g2					! N
    st  %g2, [ %o1 + 4 ]			! N
    ld  [ %o1 ], %o2				! N
    sethi  %hi(NI_TIME_A), %g2			! N
    ld  [ %g2 + %lo(NI_TIME_A)], %o3		! N
    ld  [ %o1 + 4 ], %g2			! N
    cmp  %g2, 1					! N
    bne  1b					! N
    std  %o2, [ %o0 + 0x10 ]			! S
2:
    inc  %g4					! C
    retl 					! ??
    st  %g4, [ %o0 ]				! C

.global _DYNINSTstopWallTimer

_DYNINSTstopWallTimer:
1:
    ld  [ %o0 ], %g2			! C
    sethi  %hi(_timerBuffer), %g3	! N
    cmp  %g2, 0				! C
    be  2f				! C
    cmp  %g2, 1				! C
    bne  3f				! C
    add  -1, %g2, %g2			! C
    or  %lo(_timerBuffer), %g3, %o3	! N
    mov  1, %g2				! N
    st  %g2, [ %o3 + 4 ]		! N
    ld  [ %g3 + 0x2c0 ], %o4		! N high word of ni time
    sethi  %hi(NI_TIME_A), %g2		! N
    ld  [ %g2 + %lo(NI_TIME_A) ], %o5   ! N low word of ni time 
    ld  [ %o3 + 4 ], %g4		! N ni time wrapped flag
    ldd  [ %o0 + 0x10 ], %o2		! m timer start
    ldd  [ %o0 + 8 ], %g2		! m total time
    cmp  %g4, 1				! N
    bne  1b				! N
    addcc  %o5, %g3, %g3		! m low word of t= (ni_time+totalTime)
    addx  %o4, %g2, %g2			! m hi  word of t= (ni_time+totalTime)
    subcc  %g3, %o3, %g3		! m low word of t= (t - startTime)
    subx  %g2, %o2, %g2			! m hi  word of t = (t - startTime)
    std  %g2, [ %o0 + 0x18 ]		! S
    mov  1, %g4				! ?
    stb  %g4, [ %o0 + 0x2c ]		! ?
    clr  [ %o0 ]			! C
    std  %g2, [ %o0 + 8 ]		! ?
    retl				! ?
    clrb  [ %o0 + 0x2c ]		! ?

3:
    st  %g2, [ %o0 ]
2:
    retl 
    nop


    .global _DYNINSTstartProcessTimer
_DYNINSTstartProcessTimer:
1:
        ld [%o0],%g4			! C
        sethi %hi(_timerBuffer),%g3 	! N
        cmp %g4,0			! C
        bne 2f				! C
        add %g4,1,%g4			! C
        or %g3,%lo(_timerBuffer),%g3 	! N
        mov 1,%g2 			! N
        st %g2,[%g3+4] 			! N
        ldd [%g3+8],%o4			! N
        ld [%g3],%o2			! N
        sethi %hi(NI_TIME_A),%g2	! N
        ld [%g2+%lo(NI_TIME_A)],%o3	! N
        ld [%g3+4],%g2			! N
        cmp %g2,1			! N
	bne 1b				! N
        subcc %o3,%o5,%g3		! N
        subx %o2,%o4,%g2		! N
        std %g2,[%o0+16]		! store
2:
        retl
        st %g4,[%o0]			! C


.global _DYNINSTstopProcessTimer
_DYNINSTstopProcessTimer:
1:
	save %sp,-152,%sp		! call
	mov %i0,%i4			! C
        ld [%i4],%g2			! C
        cmp %g2,1			! C
        bne L32				! C
        cmp %g2,0			! C
        sethi %hi(_timerBuffer),%i0	! N
        or %i0,%lo(_timerBuffer),%g3	! N
        mov 1,%g2			! N
        st %g2,[%g3+4]			! N
        ldd [%g3+8],%o2			! N
        ld [%i0+%lo(_timerBuffer)],%o0	! N
        sethi %hi(NI_TIME_A),%g2	! N
        ld [%g2+%lo(NI_TIME_A)],%o1	! N
        ld [%g3+4],%g2			! N
        cmp %g2,1			! N
        bne 1b				! N
        mov 1,%g2			! mutex
        stb %g2,[%i4+44]		! mutex
        st %g0,[%i4]			! C
        ldd [%i4+16],%i2		! math
        subcc %o1,%o3,%g3		! N
        subx %o0,%o2,%g2		! N
        ldd [%i4+8],%i0			! math
        subcc %g3,%i3,%g3		! math
        subx %g2,%i2,%g2		! math
        addcc %g3,%i1,%g3		! math
        addx %g2,%i0,%g2		! math
        ldub [%i4+45],%i0		! mutex
        std %g2,[%i4+24]		! store
        cmp %i0,0			! mutex
        bne 1b				! mutex
        stb %g0,[%i4+45]		! mutex
        ldd [%i4+24],%g2		! mutex
        stb %g0,[%i4+44]		! mutex
        std %g2,[%i4+8]			! mutex
        ret				! call
        restore				! call

L32:
        be L31
        add %g2,-1,%g2
        st %g2,[%i4]
L31:
        ret
        restore


.data
TIMER:
	.word 0
	.word 0
	.word 0
	.word 0
	.word 0
	.word 0
	.word 0
	.word 0
	.word 0

.text
	.global _timeNI
_timeNI:
	save %sp, -152, %sp
	sethi %hi(100000),%i0
	or %i0, %lo(100000), %i0

	sethi %hi(TIMER),%o0
	or %o0, %lo(TIMER), %o0
1:
	cmp %i0, 0			! 1
	be  2f				! 1
	add -1, %i0, %i0		! 1
	clr [%o0]
	save %sp, -152, %sp
	restore
	! call _DYNINSTstartWallTimer
	nop
	ba 1b				! 1
	nop				! 1
2:	ret
	restore
