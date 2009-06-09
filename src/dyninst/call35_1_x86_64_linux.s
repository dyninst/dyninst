/* Force Dyninst function relocation through simulated code sharing. See
   call35_1_x86_linux.s for details.
*/	
    .file	"call35_1_x86_64_linux.s"
    .version "01.01"
	.stabs	"call35_1_x86_64_linux.s",100,0,0,.Ltext0
	.text
.Ltext0:
	.stabs	"gcc2_compiled.",60,0,0,0
	.stabs	"int:t(0,1)=r(0,1);-2147483648;2147483647;",128,0,0,0
	.stabs	"char:t(0,2)=r(0,2);0;127;",128,0,0,0
	.stabs	"long int:t(0,3)=r(0,3);-9223372036854775808;9223372036854775807;",128,0,0,0
	.stabs	"unsigned int:t(0,4)=r(0,4);0;4294967295;",128,0,0,0
	.stabs	"long unsigned int:t(0,5)=r(0,5);0;-1;",128,0,0,0
	.stabs	"long long int:t(0,6)=r(0,6);-9223372036854775808;9223372036854775807;",128,0,0,0
	.stabs	"long long unsigned int:t(0,7)=r(0,7);0;-1;",128,0,0,0
	.stabs	"short int:t(0,8)=r(0,8);-32768;32767;",128,0,0,0
	.stabs	"short unsigned int:t(0,9)=r(0,9);0;65535;",128,0,0,0
	.stabs	"signed char:t(0,10)=r(0,10);-128;127;",128,0,0,0
	.stabs	"unsigned char:t(0,11)=r(0,11);0;255;",128,0,0,0
	.stabs	"__int128_t:t(0,12)=r(0,12);0;-1;",128,0,0,0
	.stabs	"__uint128_t:t(0,13)=r(0,13);0;-1;",128,0,0,0
	.stabs	"float:t(0,14)=r(0,1);4;0;",128,0,0,0
	.stabs	"double:t(0,15)=r(0,1);8;0;",128,0,0,0
	.stabs	"long double:t(0,16)=r(0,1);16;0;",128,0,0,0
	.stabs	"complex int:t(0,17)=s8real:(0,1),0,32;imag:(0,1),32,32;;",128,0,0,0
	.stabs	"complex float:t(0,18)=R3;8;0;",128,0,0,0
	.stabs	"complex double:t(0,19)=R4;16;0;",128,0,0,0
	.stabs	"complex long double:t(0,20)=R5;32;0;",128,0,0,0
	.stabs	"__builtin_va_list:t(0,21)=ar(0,22)=r(0,22);0;-1;;0;0;(0,23)=xs__va_list_tag:",128,0,0,0
	.stabs	"_Bool:t(0,24)=eFalse:0,True:1,;",128,0,0,0
	.stabs	"call35_1.c",130,0,0,0
	.stabs	"call35_2:F(0,1)",36,0,0,call35_2
.globl call35_2
	.type	call35_2, @function
call35_2:
	.stabn 68,0,43,.LM1-call35_2
.LM1:
.LFB3:
	pushq	%rbp
.LCFI0:
	movq	%rsp, %rbp
    /* this comparison should never be equal */
    cmpl $0,%esp
    jne .LM2
    /* this branch should never be taken, but tricks
       the parser into thinking call35_2 and call35_1 share code
    */
    jmp .ForceRelocation
.LCFI1:
	.stabn 68,0,43,.LM2-call35_2
.LM2:
	leave
	ret
.LFE3:
	.size	call35_2, .-call35_2
	.stabs	"test1_35_call1:F(0,1)",36,0,0,test1_35_call1
.globl test1_35_call1
	.type	test1_35_call1, @function
test1_35_call1:
	.stabn 68,0,53,.LM3-test1_35_call1
.LM3:
.LFB5:
	pushq	%rbp
.LCFI2:
	movq	%rsp, %rbp
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
.LCFI3:
.ForceRelocation:
	subq	$16, %rsp
.LCFI4:
	.stabn 68,0,54,.LM4-test1_35_call1
.LM4:
.LBB2:
	movl	$1, -4(%rbp)
	.stabn 68,0,55,.LM5-test1_35_call1
.LM5:
	movl	$1, -8(%rbp)
	.stabn 68,0,56,.LM6-test1_35_call1
.LM6:
	movl	$0, -12(%rbp)
	.stabn 68,0,57,.LM7-test1_35_call1
.LM7:
	movl	$2, -16(%rbp)
	.stabn 68,0,59,.LM8-test1_35_call1
.LM8:
	movl	$0, %eax
	call	call35_2
	.stabn 68,0,61,.LM9-test1_35_call1
.LM9:
	movl	-4(%rbp), %eax
	imull	-8(%rbp), %eax
	movl	%eax, -12(%rbp)
	.stabn 68,0,63,.LM10-test1_35_call1
.LM10:
	movl	$0, %eax
	call	call35_2
	.stabn 68,0,65,.LM11-test1_35_call1
.LM11:
	movl	$0, %eax
	call	call35_2
	.stabn 68,0,67,.LM12-test1_35_call1
.LM12:
	movl	-16(%rbp), %eax
    jmp .L3
.CauseProblemsIfExecuted:               
	subq	$16, %rsp
		
	.stabn 68,0,68,.LM13-test1_35_call1
.LM13:
.L3:
	leave
	ret
.LBE2:
.LFE5:
	.size	test1_35_call1, .-test1_35_call1
	.stabs	"localVariable35_1:(0,1)",128,0,0,-4
	.stabs	"localVariable35_2:(0,1)",128,0,0,-8
	.stabs	"total35_1:(0,1)",128,0,0,-12
	.stabs	"total35_2:(0,1)",128,0,0,-16
	.stabn	192,0,0,.LBB2-.LFB5
	.stabn	224,0,0,.LBE2-.LFB5
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
	.text
	.stabs "",100,0,0,.Letext
.Letext:
	.ident	"GCC: (GNU) 3.3.2"
