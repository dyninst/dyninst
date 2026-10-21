/*
 * Fixtures for image::findMain on aarch64.  Each is a tiny executable whose _start
 * hands main to __libc_start_main in x0 the way one of glibc's start.S variants does,
 * selected by FORM_<name>:
 *
 *   FORM_adrp            adrp x0, main; add x0, x0, :lo12:main     (non-PIC _start)
 *   FORM_got             adrp x0, :got:main; ldr x0, [x0, ...]     (PIC _start, PIE)
 *   FORM_literal         ldr  x0, =main                            (older non-PIC _start)
 *   FORM_not_glibc       _start passes a string, not main, to the first call
 *
 * FORM_got is linked as a PIE, so main's GOT slot holds no address in the file, only an
 * R_AARCH64_RELATIVE relocation whose addend is main; the others are static executables.
 * None of them is ever run; the test only opens the stripped file.
 */
	.text
	.globl	_start
	.type	_start, %function
_start:
	mov	x29, #0
	mov	x30, #0
	mov	x5, x0
	ldr	x1, [sp]
	add	x2, sp, #8
	mov	x6, sp
#if defined(FORM_adrp)
	adrp	x0, main
	add	x0, x0, :lo12:main
#elif defined(FORM_got)
	adrp	x0, :got:main
	ldr	x0, [x0, :got_lo12:main]
#elif defined(FORM_literal)
	ldr	x0, =main
#elif defined(FORM_not_glibc)
	adrp	x0, message
	add	x0, x0, :lo12:message
	bl	write_message
	mov	x0, #0
#else
#error "no FORM_ selected"
#endif
	mov	x3, #0
	mov	x4, #0
	bl	__libc_start_main
	bl	abort_here
#if defined(FORM_literal)
	.ltorg
#endif

	.globl	main
	.type	main, %function
main:
	mov	x0, #0
	ret

	.type	__libc_start_main, %function
__libc_start_main:
	ret

	.type	abort_here, %function
abort_here:
	b	abort_here

	.type	write_message, %function
write_message:
	ret

	.section .rodata
message:
	.asciz	"not main"
