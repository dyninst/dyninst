
/*
 #	Do not save or restore register 26 - the return address register
 #	This will only get clobbered if a function call is added
 #	Instrumentation code will be generated in that case
 #	register 26 can't be restored - that would clobber the return address
 #	in DYNINSTrestore		
*/

#define LONG_4 .long 0 ; .long 0 ; .long 0 ; .long 0
#define LONG_16 LONG_4 ; LONG_4 ; LONG_4 ; LONG_4
#define LONG_64 LONG_16	; LONG_16 ; LONG_16 ; LONG_16
#define LONG_256 LONG_64 ; LONG_64 ; LONG_64 ; LONG_64
#define LONG_1024 LONG_256 ; LONG_256 ; LONG_256 ; LONG_256
#define LONG_4096 LONG_1024 ; LONG_1024 ; LONG_1024 ; LONG_1024
#define LONG_16k LONG_4096 ; LONG_4096 ; LONG_4096 ; LONG_4096
#define LONG_64k LONG_16k ; LONG_16k ;  LONG_16k ; LONG_16k
	
	.set noat
	.text

	.globl DYNINSTcode
	.ent DYNINSTcode 2	
DYNINSTcode:
	LONG_64k		; 
	LONG_16k		;
	.end DYNINSTcode
	
	.globl DYNINSTsave_temp
	.ent DYNINSTsave_temp 2	
DYNINSTsave_temp:
	lda $sp, -104($sp)
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
	lda $sp, 104($sp)	
	ret $31, ($26), 1
	.end DYNINSTrestore_temp

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
 * XXX These are the old versions of the save_misc functions.  They do not
 * save and restore the scratch registers (t0-t7) that we use in
 * mini-trampolines.  Ideally, we'd like it to be this way, and then we could
 * supplement it with code generated on the fly to only save and restore the
 * scratch registers that we're using, so I'll leave them here and maybe we
 * can go back to using these sometime.

	.globl DYNINSTsave_misc
	.ent DYNINSTsave_misc 2
DYNINSTsave_misc:	
	lda $sp, -80($sp)
	stq $16, 0($sp)
	stq $17, 8($sp)
	stq $18, 16($sp)
	stq $19, 24($sp)
	stq $20, 32($sp)
	stq $21, 40($sp)
	stq $27, 48($sp)
	stq $at, 56($sp)
	stq $gp, 64($sp)
	stq $0,72($sp)
	ret $31, ($26), 1
	.end DYNINSTsave_misc

	
	.globl DYNINSTrestore_misc
	.ent DYNINSTrestore_misc 2
DYNINSTrestore_misc:
	ldq $16, 0($sp)
	ldq $17, 8($sp)
	ldq $18, 16($sp)
	ldq $19, 24($sp)
	ldq $20, 32($sp)
	ldq $21, 40($sp)
	ldq $27, 48($sp)
	ldq $at, 56($sp)
	ldq $gp, 64($sp)
	ldq $0,72($sp)
	lda $sp, 80($sp)
	ret $31, ($26), 1
	.end DYNINSTrestore_misc
*/

/*
 * XXX These are the new versions of the save_misc functions.  They save all
 * scratch registers that we may potentially be using in a mini-tramp.
 */
	.globl DYNINSTsave_misc
	.ent DYNINSTsave_misc 2
DYNINSTsave_misc:	
	lda $sp, -136($sp)
	stq $1, 0($sp)
	stq $2, 8($sp)
	stq $3, 16($sp)
	stq $4, 24($sp)
	stq $5, 32($sp)
	stq $6, 40($sp)
	stq $7, 48($sp)
	stq $8, 56($sp)
	stq $16, 64($sp)
	stq $17, 72($sp)
	stq $18, 80($sp)
	stq $19, 88($sp)
	stq $20, 96($sp)
	stq $21, 104($sp)
	stq $27, 112($sp)
	stq $at, 120($sp)
	stq $gp, 128($sp)
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
	ldq $16, 64($sp)
	ldq $17, 72($sp)
	ldq $18, 80($sp)
	ldq $19, 88($sp)
	ldq $20, 96($sp)
	ldq $21, 104($sp)
	ldq $27, 112($sp)
	ldq $at, 120($sp)
	ldq $gp, 128($sp)
	lda $sp, 136($sp)
	ret $31, ($26), 1
	.end DYNINSTrestore_misc

