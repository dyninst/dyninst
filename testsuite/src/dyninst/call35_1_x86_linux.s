/* This routine has been mangled to encourage Dyninst to relocate call35_1
   when instrumenting the call sites. There are two scenarios in which 
   relocation is necessary:

   1) If insufficient space exists in the basic block to be instrumented,
      the function must be relocated. The minimum space required is 5 bytes
      on x86.

   2) If a basic block is shared between two functions, instrumentation of
      that basic block will require relocation.

   Since call instructions (which are instrumented in this test) are five
   bytes long, we will not need to relocate even if the basic block
   containing the target instruction is only one instruction long. Note that
   call instructions with a word (16-bit in x86 assembly parlance) argument
   take up only four bytes, so we technically could come up with four-byte
   instructions that would force relocation. However, since this is not
   possible on x86-64, we have chosen to adopt the shared code route to
   force relocation, to maintain consistant tests across these two
   platforms.

   The appearance of shared code is created by adding a never-taken branch
   from the body of call35_2 into the body of call35_1. The parser will
   follow that branch during static analysis and mark the body of call35_1
   as being shared, but control flow will never follow that branch during
   execution.
*/
    .file	"call35_1_x86_linux.s"
	.version	"01.01"
.stabs "/p/paradyn/development/gurari/core/dyninstAPI/tests/src/",100,0,0,.Ltext0
.stabs "call35_1_x86_linux.s",100,0,0,.Ltext0
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
.globl call35_2
	.type	 call35_2,@function
call35_2:
.stabn 68,0,2,.LM1-call35_2
.LM1:
	pushl %ebp
	movl %esp,%ebp
    /* this comparison should never be equal */
    cmpl $0,%esp
    jne .L2
    /* this branch should never be taken, but tricks
       the parser into thinking call35_2 and call35_1 share code
    */
	jmp .ForceRelocation
.stabn 68,0,3,.LM2-call35_2
.LM2:
.L2:
	leave
	ret
.Lfe1:
	.size	 call35_2,.Lfe1-call35_2
.Lscope0:
.stabs "",36,0,0,.Lscope0-call35_2
	.align 4
.stabs "test1_35_call1:F(0,1)",36,0,10,test1_35_call1
.globl test1_35_call1
	.type	 test1_35_call1,@function
test1_35_call1:
.stabn 68,0,10,.LM3-test1_35_call1
.LM3:
	pushl %ebp
	movl %esp,%ebp
        nop
        nop
        nop
        nop
        nop
.ForceRelocation:
	subl $24,%esp
.stabn 68,0,11,.LM4-test1_35_call1
.LM4:
.LBB2:
	movl $1,-4(%ebp)
.stabn 68,0,12,.LM5-test1_35_call1
.LM5:
	movl $1,-8(%ebp)
.stabn 68,0,13,.LM6-test1_35_call1
.LM6:
	movl $0,-12(%ebp)
.stabn 68,0,14,.LM7-test1_35_call1
.LM7:
	movl $2,-16(%ebp)
.stabn 68,0,16,.LM8-test1_35_call1
.LM8:
	call call35_2
.stabn 68,0,18,.LM9-test1_35_call1
.LM9:
	movl -4(%ebp),%eax
	imull -8(%ebp),%eax
	movl %eax,-12(%ebp)
.stabn 68,0,20,.LM10-test1_35_call1
.LM10:
	call call35_2
.stabn 68,0,22,.LM11-test1_35_call1
.LM11:
	call call35_2
.stabn 68,0,24,.LM12-test1_35_call1
.LM12:
	movl -16(%ebp),%edx
	movl %edx,%eax
    jmp .L3
/* Never parsed or reached during execution, of unknown provenance;
   left as historical curiosity.
*/
.CauseProblemsIfExecuted:               
        subl $24,%esp
		
.stabn 68,0,25,.LM13-test1_35_call1
.LM13:
.LBE2:
.stabn 68,0,25,.LM14-test1_35_call1
.LM14:
	.align 4
.L3:
	leave
	ret
.Lfe2:
	.size	 test1_35_call1,.Lfe2-test1_35_call1
.stabs "localVariable35_1:(0,1)",128,0,11,-4
.stabs "localVariable35_2:(0,1)",128,0,12,-8
.stabs "total35_1:(0,1)",128,0,13,-12
.stabs "total35_2:(0,1)",128,0,14,-16
.stabn 192,0,0,.LBB2-test1_35_call1
.stabn 224,0,0,.LBE2-test1_35_call1
.Lscope1:
.stabs "",36,0,0,.Lscope1-test1_35_call1
	.text
	.stabs "",100,0,0,.Letext
.Letext:
	.ident	"GCC: (GNU) 2.95.2 19991024 (release)"
