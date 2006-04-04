	.text
.globl cpuidCall
	.type	cpuidCall, @function
cpuidCall:
	pushl   %ebp
	movl    %esp, %ebp
	subl    $8, %esp
	movl    %ebx, 4(%esp)
	movl    $1, %eax
	cpuid
	movl	%edx, %eax
	movl    4(%esp), %ebx
	movl    %ebp, %esp
	popl    %ebp	
	ret
	.size	cpuidCall, .-cpuidCall
	.section	.note.GNU-stack,"",@progbits
	.ident	"GCC: (GNU) 3.3.3"
