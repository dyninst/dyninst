	.file	"call35_1_sparc_solaris.s"
.stabs "/p/paradyn/development/gurari/core/dyninstAPI/tests/src/",100,0,0,.LLtext0
.stabs "call35_1_sparc_solaris.s",100,0,0,.LLtext0
.section	".text"
.LLtext0:
	.stabs	"gcc2_compiled.", 0x3c, 0, 0, 0
.stabs "int:t(0,1)=r(0,1);0020000000000;0017777777777;",128,0,0,0
.stabs "char:t(0,2)=r(0,2);0;127;",128,0,0,0
.stabs "long int:t(0,3)=r(0,1);0020000000000;0017777777777;",128,0,0,0
.stabs "unsigned int:t(0,4)=r(0,1);0000000000000;0037777777777;",128,0,0,0
.stabs "long unsigned int:t(0,5)=r(0,1);0000000000000;0037777777777;",128,0,0,0
.stabs "long long int:t(0,6)=r(0,1);01000000000000000000000;0777777777777777777777;",128,0,0,0
.stabs "long long unsigned int:t(0,7)=r(0,1);0000000000000;01777777777777777777777;",128,0,0,0
.stabs "short int:t(0,8)=r(0,8);-32768;32767;",128,0,0,0
.stabs "short unsigned int:t(0,9)=r(0,9);0;65535;",128,0,0,0
.stabs "signed char:t(0,10)=r(0,10);-128;127;",128,0,0,0
.stabs "unsigned char:t(0,11)=r(0,11);0;255;",128,0,0,0
.stabs "float:t(0,12)=r(0,1);4;0;",128,0,0,0
.stabs "double:t(0,13)=r(0,1);8;0;",128,0,0,0
.stabs "long double:t(0,14)=r(0,1);16;0;",128,0,0,0
.stabs "complex int:t(0,15)=s8real:(0,1),0,32;imag:(0,1),32,32;;",128,0,0,0
.stabs "complex float:t(0,16)=r(0,16);4;0;",128,0,0,0
.stabs "complex double:t(0,17)=r(0,17);8;0;",128,0,0,0
.stabs "complex long double:t(0,18)=r(0,18);16;0;",128,0,0,0
.stabs "void:t(0,19)=(0,19)",128,0,0,0
	.align 4
.stabs "call35_2:F(0,1)",36,0,2,call35_2
	.global call35_2
	.type	 call35_2,#function
	.proc	04
call35_2:
.stabn 68,0,2,.LLM1-call35_2
.LLM1:
	!#PROLOGUE# 0
	save	%sp, -112, %sp
	!#PROLOGUE# 1
.stabn 68,0,3,.LLM2-call35_2
.LLM2:
.LL2:
	ret
	restore
.LLfe1:
	.size	 call35_2,.LLfe1-call35_2
.LLscope0:
.stabs "",36,0,0,.LLscope0-call35_2
	.global .umul
	.align 4
.stabs "test1_35_call1:F(0,1)",36,0,5,test1_35_call1
	.global test1_35_call1
	.type	 test1_35_call1,#function
	.proc	04
test1_35_call1:
.stabn 68,0,5,.LLM3-test1_35_call1
.LLM3:
	!#PROLOGUE# 0
	save	%sp, -128, %sp
	!#PROLOGUE# 1
.stabn 68,0,6,.LLM4-test1_35_call1
.LLM4:
.LLBB2:
	mov	1, %o0
	st	%o0, [%fp-20]
.stabn 68,0,7,.LLM5-test1_35_call1
.LLM5:
	mov	1, %o0
	st	%o0, [%fp-24]
.stabn 68,0,8,.LLM6-test1_35_call1
.LLM6:
	st	%g0, [%fp-28]
.stabn 68,0,9,.LLM7-test1_35_call1
.LLM7:
	mov	2, %o0
	st	%o0, [%fp-32]
.stabn 68,0,11,.LLM8-test1_35_call1
.LLM8:
	call	call35_2, 0
	 nop
.stabn 68,0,13,.LLM9-test1_35_call1
.LLM9:
	ld	[%fp-20], %o0
	ld	[%fp-24], %o1
	call	.umul, 0
	 nop
	st	%o0, [%fp-28]
.stabn 68,0,15,.LLM10-test1_35_call1
.LLM10:
	call	call35_2, 0
	 nop
.stabn 68,0,17,.LLM11-test1_35_call1
.LLM11:
	call	call35_2, 0
	 nop
.stabn 68,0,19,.LLM12-test1_35_call1
.LLM12:
	ld	[%fp-32], %o0
	mov	%o0, %i0
	b	.LL3
	 nop
.stabn 68,0,20,.LLM13-test1_35_call1
.LLM13:
.LLBE2:
.stabn 68,0,20,.LLM14-test1_35_call1
.LLM14:
.LL3:
	
.ForceRelocation:
	ba,a    .skipTailCallOptimization
	call    call35_2, 0
	restore
.skipTailCallOptimization:				

	ret
	restore
.LLfe2:
	.size	 test1_35_call1,.LLfe2-test1_35_call1
.stabs "localVariable35_1:(0,1)",128,0,6,-20
.stabs "localVariable35_2:(0,1)",128,0,7,-24
.stabs "total35_1:(0,1)",128,0,8,-28
.stabs "total35_2:(0,1)",128,0,9,-32
.stabn 192,0,0,.LLBB2-test1_35_call1
.stabn 224,0,0,.LLBE2-test1_35_call1
.LLscope1:
.stabs "",36,0,0,.LLscope1-test1_35_call1
	.text
	.stabs "",100,0,0,Letext
Letext:
	.ident	"GCC: (GNU) 2.95.2 19991024 (release)"
