	.text
.globl cpuidCall
	.type	cpuidCall, @function
cpuidCall:
	pushl	%ebp
	movl	%esp, %ebp
	movl    $1, %eax
	cpuid
	movl	%edx, %eax	
	popl	%ebp
	ret
	.size	cpuidCall, .-cpuidCall
	.section	.note.GNU-stack,"",@progbits
	.ident	"GCC: (GNU) 3.3.3"
