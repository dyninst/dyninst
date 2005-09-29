	.file	"call35_1.c"
	.text
.globl call35_2
	.type	call35_2, @function
call35_2:
.LFB3:
	pushq	%rbp
.LCFI0:
	movq	%rsp, %rbp
.LCFI1:
	leave
	ret
.LFE3:
	.size	call35_2, .-call35_2
.globl call35_1
	.type	call35_1, @function
call35_1:
.LFB5:
	pushq	%rbp
.LCFI2:
	movq	%rsp, %rbp
.LCFI3:
	subq	$16, %rsp
.LCFI4:
	movl	$1, -4(%rbp)
	movl	$1, -8(%rbp)
	movl	$0, -12(%rbp)
	movl	$2, -16(%rbp)
	movl	$0, %eax
	call	call35_2
	movl	-4(%rbp), %eax
	imull	-8(%rbp), %eax
	movl	%eax, -12(%rbp)
	movl	$0, %eax
	call	call35_2
	movl	$0, %eax
	call	call35_2
	movl	-16(%rbp), %eax
	leave
	ret
.LFE5:
	.size	call35_1, .-call35_1
	.section	.eh_frame,"a",@progbits
.Lframe1:
	.long	.LECIE1-.LSCIE1
.LSCIE1:
	.long	0x0
	.byte	0x1
	.string	""
	.uleb128 0x1
	.sleb128 -8
	.byte	0x10
	.byte	0xc
	.uleb128 0x7
	.uleb128 0x8
	.byte	0x90
	.uleb128 0x1
	.align 8
.LECIE1:
.LSFDE1:
	.long	.LEFDE1-.LASFDE1
.LASFDE1:
	.long	.LASFDE1-.Lframe1
	.quad	.LFB3
	.quad	.LFE3-.LFB3
	.byte	0x4
	.long	.LCFI0-.LFB3
	.byte	0xe
	.uleb128 0x10
	.byte	0x86
	.uleb128 0x2
	.byte	0x4
	.long	.LCFI1-.LCFI0
	.byte	0xd
	.uleb128 0x6
	.align 8
.LEFDE1:
.LSFDE3:
	.long	.LEFDE3-.LASFDE3
.LASFDE3:
	.long	.LASFDE3-.Lframe1
	.quad	.LFB5
	.quad	.LFE5-.LFB5
	.byte	0x4
	.long	.LCFI2-.LFB5
	.byte	0xe
	.uleb128 0x10
	.byte	0x86
	.uleb128 0x2
	.byte	0x4
	.long	.LCFI3-.LCFI2
	.byte	0xd
	.uleb128 0x6
	.align 8
.LEFDE3:
	.section	.note.GNU-stack,"",@progbits
	.ident	"GCC: (GNU) 3.3.3"
