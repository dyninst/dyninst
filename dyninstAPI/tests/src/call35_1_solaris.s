	.file	"call35_1.c"
	.version	"01.01"
.stabs "/p/paradyn/development/gurari/core/dyninstAPI/tests/src/",100,0,0,.Ltext0
.stabs "call35_1.c",100,0,0,.Ltext0
.text
.Ltext0:
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
.stabs "long double:t(0,14)=r(0,1);12;0;",128,0,0,0
.stabs "complex int:t(0,15)=s8real:(0,1),0,32;imag:(0,1),32,32;;",128,0,0,0
.stabs "complex float:t(0,16)=r(0,16);4;0;",128,0,0,0
.stabs "complex double:t(0,17)=r(0,17);8;0;",128,0,0,0
.stabs "complex long double:t(0,18)=r(0,18);12;0;",128,0,0,0
.stabs "void:t(0,19)=(0,19)",128,0,0,0
	.align 4
.stabs "call35_2:F(0,1)",36,0,2,call35_2
.stabs "x:p(0,1)",160,0,2,8
.stabs "y:p(0,1)",160,0,2,12
.globl call35_2
	.type	 call35_2,@function
call35_2:
.stabn 68,0,2,.LM1-call35_2
.LM1:
	pushl %ebp
	movl %esp,%ebp
.stabn 68,0,3,.LM2-call35_2
.LM2:
	movl 8(%ebp),%edx
	imull 12(%ebp),%edx
	movl %edx,%eax
	jmp .L2
.stabn 68,0,4,.LM3-call35_2
.LM3:
	.align 4
.L2:
	leave
	ret
.Lfe1:
	.size	 call35_2,.Lfe1-call35_2
.Lscope0:
.stabs "",36,0,0,.Lscope0-call35_2
	.align 4
.stabs "call35_1:F(0,1)",36,0,6,call35_1
.globl call35_1
	.type	 call35_1,@function
call35_1:
.stabn 68,0,6,.LM4-call35_1
.LM4:
	pushl %ebp
	movl %esp,%ebp
.ForceRelocation:
	subl $24,%esp
.stabn 68,0,7,.LM5-call35_1
.LM5:
.LBB2:
	movl $1,-4(%ebp)
.stabn 68,0,8,.LM6-call35_1
.LM6:
	movl $1,-8(%ebp)
.stabn 68,0,9,.LM7-call35_1
.LM7:
	movl $0,-12(%ebp)
.stabn 68,0,10,.LM8-call35_1
.LM8:
	movl $2,-16(%ebp)
.stabn 68,0,12,.LM9-call35_1
.LM9:
	addl $-8,%esp
	movl -8(%ebp),%eax
	pushl %eax
	movl -4(%ebp),%eax
	pushl %eax
	call call35_2
	addl $16,%esp
.stabn 68,0,14,.LM10-call35_1
.LM10:
	movl -4(%ebp),%eax
	imull -8(%ebp),%eax
	movl %eax,-12(%ebp)
.stabn 68,0,16,.LM11-call35_1
.LM11:
	addl $-8,%esp
	movl -8(%ebp),%eax
	pushl %eax
	movl -4(%ebp),%eax
	pushl %eax
	call call35_2
	addl $16,%esp
.stabn 68,0,18,.LM12-call35_1
.LM12:
	movl -16(%ebp),%edx
	movl %edx,%eax
	jmp .L3
	
	jmp .ForceRelocation
	jmp .InsertNops1
.InsertNops2:
        ret
        jmp .InsertNops2
.InsertNops1:
        jmp .InsertNops3
.InsertNops4:
        ret
        jmp .InsertNops4        
.InsertNops3:   
.SplitPointsApart:
        ret
        ret
.FillTenBytes1: 
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
.FillTenBytes2: 
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
.FillTenBytes3: 
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
.FillTenBytes4:
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
.FillTenBytes5:
        nop
        nop
        nop
        nop
        nop
        nop
        nop
        nop
        nop
.FillTenBytes6:
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
.FillTenBytes7: 
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
.FillTenBytes8:
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
.FillTenBytes9: 
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
.FillTenBytes10:
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
.Fill10Bytes11:
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
.CauseProblemsIfExecuted:               
        subl $24,%esp
	
.stabn 68,0,19,.LM13-call35_1
.LM13:
.LBE2:
.stabn 68,0,19,.LM14-call35_1
.LM14:
	.align 4
.L3:
	leave
	ret
.Lfe2:
	.size	 call35_1,.Lfe2-call35_1
.stabs "localVariable35_1:(0,1)",128,0,7,-4
.stabs "localVariable35_2:(0,1)",128,0,8,-8
.stabs "total35_1:(0,1)",128,0,9,-12
.stabs "total35_2:(0,1)",128,0,10,-16
.stabn 192,0,0,.LBB2-call35_1
.stabn 224,0,0,.LBE2-call35_1
.Lscope1:
.stabs "",36,0,0,.Lscope1-call35_1
	.text
	.stabs "",100,0,0,.Letext
.Letext:
	.ident	"GCC: (GNU) 2.95.2 19991024 (release)"
