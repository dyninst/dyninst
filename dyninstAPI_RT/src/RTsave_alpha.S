
/*
 #	Do not save or restore register 26 - the return address register
 #	This will only get clobbered if a function call is added
 #	Instrumentation code will be generated in that case
 #	register 26 can't be restored - that would clobber the return address
 #	in DYNINSTrestore		
 #
 #	Added save/restore of $fp.  This is required for inferrior RPC to work.
 #		- jkh 11/5/98
*/

#define LONG_4 .long 0 ; .long 0 ; .long 0 ; .long 0
#define LONG_16 LONG_4 ; LONG_4 ; LONG_4 ; LONG_4
#define LONG_64 LONG_16	; LONG_16 ; LONG_16 ; LONG_16
#define LONG_256 LONG_64 ; LONG_64 ; LONG_64 ; LONG_64
#define LONG_1024 LONG_256 ; LONG_256 ; LONG_256 ; LONG_256
#define LONG_4096 LONG_1024 ; LONG_1024 ; LONG_1024 ; LONG_1024
#define LONG_16k LONG_4096 ; LONG_4096 ; LONG_4096 ; LONG_4096
#define LONG_64k LONG_16k ; LONG_16k ;  LONG_16k ; LONG_16k
	
#define STACK_AREA		(120)
#define CONSERVATIVE_STACK_AREA	(256+31*8)

	.set noat
	.text

	.globl DYNINSTcode
	.ent DYNINSTcode 2	
DYNINSTcode:
	LONG_1024		;
	.end DYNINSTcode
	
	.globl DYNINSTsave_temp
	.ent DYNINSTsave_temp 2	
DYNINSTsave_temp:
	stq $30, 112-STACK_AREA($sp) /* save frame pointer */
	lda $sp, -STACK_AREA($sp)
	stq $1, 0($sp)
	stq $2, 8($sp)
	stq $3, 16($sp)
	stq $4, 24($sp)
	stq $5, 32($sp)
	stq $6, 40($sp)
	stq $7, 48($sp)
	stq $8, 56($sp)
	stq $22, 64($sp)
	stq $23, 72($sp)
	stq $24, 80($sp)
	stq $25, 88($sp)
	stq $0, 96($sp) /* Save v0 - see note after DYNINSTrestore_temp */
	/* if you change this, check loadFrameRelativeOp too */
	stq $15, 104($sp) /* save frame pointer */
	ret $31, ($26), 1
	.end DYNINSTsave_temp
	
	.globl DYNINSTrestore_temp
	.ent DYNINSTrestore_temp 2
DYNINSTrestore_temp:	
	ldq $1, 0($sp)
	ldq $2, 8($sp)
	ldq $3, 16($sp)
	ldq $4, 24($sp)
	ldq $5, 32($sp)
	ldq $6, 40($sp)
	ldq $7, 48($sp)
	ldq $8, 56($sp)
	ldq $22, 64($sp)
	ldq $23, 72($sp)
	ldq $24, 80($sp)
	ldq $25, 88($sp)
	ldq $0, 96($sp) /* Restore v0 - see note after this function */
	ldq $15, 104($sp) /* restore frame pointer */
	lda $sp, STACK_AREA($sp)	
	ret $31, ($26), 1
	.end DYNINSTrestore_temp

/*
 * This version is conservative and saves everything for arbitrary inst points.
 *
 */
	.globl DYNINSTsave_conservative
	.ent DYNINSTsave_conservative 2	
DYNINSTsave_conservative:
	lda $sp, -CONSERVATIVE_STACK_AREA($sp)
	stq $1, 0($sp)
	stq $2, 8($sp)
	stq $3, 16($sp)
	stq $4, 24($sp)
	stq $5, 32($sp)
	stq $6, 40($sp)
	stq $7, 48($sp)
	stq $8, 56($sp)
	stq $9, 64($sp)
	stq $10, 72($sp)
	stq $11, 80($sp)
	stq $12, 88($sp)
	stq $13, 96($sp)
	stq $14, 104($sp)

	/* 15 - 21 saved by saveMisc if used */

	stq $22, 168($sp)
	stq $23, 176($sp)
	stq $24, 184($sp)
	stq $25, 192($sp)

	/* skip $26 - return address */

        /* save misc saves 27-28 */

	stq $0, 240($sp) /* Save v0 - see note after DYNINSTrestore_temp */

	/* now the FP registers */
	stt $f0, 248($sp)
	stt $f1, 256($sp)
	stt $f2, 264($sp)
	stt $f3, 272($sp)
	stt $f4, 280($sp)
	stt $f5, 288($sp)
	stt $f6, 296($sp)
	stt $f7, 304($sp)
	stt $f8, 312($sp)
	stt $f9, 320($sp)
	stt $f10, 328($sp)
	stt $f11, 336($sp)
	stt $f12, 344($sp)
	stt $f13, 352($sp)
	stt $f14, 360($sp)
	stt $f15, 368($sp)
	stt $f16, 376($sp)
	stt $f17, 384($sp)
	stt $f18, 392($sp)
	stt $f19, 400($sp)
	stt $f20, 408($sp)
	stt $f21, 416($sp)
	stt $f22, 424($sp)
	stt $f23, 432($sp)
	stt $f24, 440($sp)
	stt $f25, 448($sp)
	stt $f26, 456($sp)
	stt $f27, 464($sp)
	stt $f28, 472($sp)
	stt $f29, 480($sp)
	stt $f30, 488($sp)

	/* leave extra gap */
	lda $sp, -CONSERVATIVE_STACK_AREA($sp)
	ret $31,($26), 1
	.end DYNINSTsave_conservative
	
	.globl DYNINSTrestore_conservative
	.ent DYNINSTrestore_conservative 2
DYNINSTrestore_conservative:	
	/* remove extra gap */
	lda $sp, CONSERVATIVE_STACK_AREA($sp)	

	ldq $1, 0($sp)
	ldq $2, 8($sp)
	ldq $3, 16($sp)
	ldq $4, 24($sp)
	ldq $5, 32($sp)
	ldq $6, 40($sp)
	ldq $7, 48($sp)
	ldq $8, 56($sp)
	ldq $9, 64($sp)
	ldq $10, 72($sp)
	ldq $11, 80($sp)
	ldq $12, 88($sp)
	ldq $13, 96($sp)
	ldq $14, 104($sp)

	/* 15 - 21 restored by saveMisc */

	ldq $22, 168($sp)
	ldq $23, 176($sp)
	ldq $24, 184($sp)
	ldq $25, 192($sp)

	/* 26-29 restored by restoreMisc */
		
	/* skip r30 it's the stack pointer */

	ldq $0, 240($sp) /* Restore v0 - see note after this function */

	/* now the FP registers */
	ldt $f0, 248($sp)
	ldt $f1, 256($sp)
	ldt $f2, 264($sp)
	ldt $f3, 272($sp)
	ldt $f4, 280($sp)
	ldt $f5, 288($sp)
	ldt $f6, 296($sp)
	ldt $f7, 304($sp)
	ldt $f8, 312($sp)
	ldt $f9, 320($sp)
	ldt $f10, 328($sp)
	ldt $f11, 336($sp)
	ldt $f12, 344($sp)
	ldt $f13, 352($sp)
	ldt $f14, 360($sp)
	ldt $f15, 368($sp)
	ldt $f16, 376($sp)
	ldt $f17, 384($sp)
	ldt $f18, 392($sp)
	ldt $f19, 400($sp)
	ldt $f20, 408($sp)
	ldt $f21, 416($sp)
	ldt $f22, 424($sp)
	ldt $f23, 432($sp)
	ldt $f24, 440($sp)
	ldt $f25, 448($sp)
	ldt $f26, 456($sp)
	ldt $f27, 464($sp)
	ldt $f28, 472($sp)
	ldt $f29, 480($sp)
	ldt $f30, 488($sp)

	lda $sp, CONSERVATIVE_STACK_AREA($sp)	
	ret $31, ($26), 1
	.end DYNINSTrestore_conservative

/*
 * XXX Note about saving and restoring v0 above: We don't necessarily
 * need to save and restore v0 in the base tramp.  We're only doing it
 * there now because it makes it easier for us to get the return value
 * of a function without having to generate code on the fly to selectively
 * save and restore registers.  Basically, we save and restore all scratch
 * registers before and after calling a function from a mini-tramp.  We don't
 * want to have to save and restore v0 there, because we'd like to call
 * DYNINSTrestore_misc and then, *after* the restore, we want to read the
 * return value out of v0.  If we work out some alternative way to store the
 * return value *before* the restore so that it doesn't get wiped out by the
 * restore, then we won't need to do anything with v0 in the base tramp.
 */


/*
 * XXX These are the new versions of the save_misc functions.  They save all
 * scratch registers that we may potentially be using in a mini-tramp.
 */
	.globl DYNINSTsave_misc
	.ent DYNINSTsave_misc 2
DYNINSTsave_misc:	
	lda $sp, -144($sp)
	stq $1, 0($sp)
	stq $2, 8($sp)
	stq $3, 16($sp)
	stq $4, 24($sp)
	stq $5, 32($sp)
	stq $6, 40($sp)
	stq $7, 48($sp)
	stq $8, 56($sp)

	stq $15, 136($sp)	/* 15 == $fp */
	stq $16, 64($sp)
	stq $17, 72($sp)
	stq $18, 80($sp)
	stq $19, 88($sp)
	stq $20, 96($sp)
	stq $21, 104($sp)

	stq $27, 112($sp)	
	stq $28, 120($sp)	/* 28 == $at */
	stq $29, 128($sp)	/* 29 == $gp */
	stq $30, 144($sp)	/* $30 == $sp */
	ret $31, ($26), 1
	.end DYNINSTsave_misc

	.globl DYNINSTrestore_misc
	.ent DYNINSTrestore_misc 2
DYNINSTrestore_misc:
	ldq $1, 0($sp)
	ldq $2, 8($sp)
	ldq $3, 16($sp)
	ldq $4, 24($sp)
	ldq $5, 32($sp)
	ldq $6, 40($sp)
	ldq $7, 48($sp)
	ldq $8, 56($sp)

	ldq $15, 136($sp)	/* $15 == $fp */
	ldq $16, 64($sp)
	ldq $17, 72($sp)
	ldq $18, 80($sp)
	ldq $19, 88($sp)
	ldq $20, 96($sp)
	ldq $21, 104($sp)

	/* $26 == $ra, skip this one */

	ldq $27, 112($sp)
	ldq $28, 120($sp)
	ldq $29, 128($sp)
	lda $30, 144($sp)	/* $30 == $sp */
	ret $31, ($26), 1
	.end DYNINSTrestore_misc
