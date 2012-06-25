.file	"test6LS.S"
		
.text
.align 16

	.global loadsnstores
	.type loadsnstores,@function

   .global amd_features
   .type amd_features,@function
       
	.global rip_relative_load_address
	.type rip_relative_load_address,@object
	.size rip_relative_load_address,8
        .global ia32features
.type ia32features,@function
        .global amd_features
                .type amd_featuresm,@function                            

                
rip_relative_load_address:
	.long rip_relative_load
	.long 0
			
loadsnstores:

	/* stack frame */
	push %rbp         /* S1 A1 */
	mov %rsp, %rbp

	/* save preserved registers (each is a store) */
	/* also %rbp */
	push %rbp
	push %rbx
	push %r12
	push %r13
	push %r14
	push %r15         /* S7 A7 */

	/* copy divarw's address into GPRs (except %rsp) */
	mov $divarw, %rax
	mov %rax, %rcx
	mov %rax, %rdx	
	mov %rax, %rbx
	mov %rax, %rbp
	mov %rax, %rsi
	mov %rax, %rdi
	mov %rax, %r8
	mov %rax, %r9
	mov %rax, %r10
	mov %rax, %r11
	mov %rax, %r12
	mov %rax, %r13
	mov %rax, %r14
	mov %rax, %r15
	
	/* ModRM decoding tests - 64 bit */
	/* each access here is a load */

	/* mod = 0 */
	mov (%rax), %eax
	mov (%rcx), %rax		
	mov (%rdx), %eax	
	mov (%rbx), %rax
	/* mov (%rsp), %eax is SIB - tested later */	
	mov 3(%rip), %eax	/* mov (%rbp), %eax requires mod = 01 */	
	mov (%rsi), %rax
rip_relative_load:	
	mov (%rdi), %eax
	mov (%r8), %rax
	mov (%r9), %eax
	mov (%r10), %rax
	mov (%r11), %eax
	/* mov (%r12), %eax is SIB */
	/* mov (%r13), %eax requires mod = 01 */
	mov (%r14), %rax
	mov (%r15), %eax    /* L13 A20 */

	/* mod = 1 */
	mov $divarw, %rax
	mov 4(%rax), %eax
	mov 8(%rcx), %rax		
	mov -4(%rdx), %eax	
	mov -8(%rbx), %rax
	/* SIB tested later */
	mov 4(%rbp), %eax
	mov 8(%rsi), %rax
	mov -4(%rdi), %eax
	mov -8(%r8), %rax
	mov 4(%r9), %eax
	mov 8(%r10), %rax
	mov -4(%r11), %eax
	/* SIB tested later */
	mov -8(%r13), %rax
	mov 127(%r14), %eax
	mov -128(%r15), %rax    /* L27 A34 */

	/* SIB tests */
	mov $divarw, %rax
	mov %rax, %rbx
	mov %rax, %rbp
	xor %rdx, %rdx
	mov $4, %rcx
	mov $12, %rsi

	mov (%rbx,%rsi), %eax
	mov (%rsp), %rax
	mov (%rbx,%rcx,2), %eax
	mov divarw(,%rcx,2), %rax
	mov 4(%rbx,%rcx), %eax
	mov divarw(%rdx,%rdx,8), %rax
	mov $1, %rcx
	mov 2(%rbp,%rcx,2), %eax
	mov 4(%rbx,%rcx,4), %rax
	mov $0, %rbp
	mov divarw(%rbp,%rcx,4), %eax /* L36 A43 */

	/* semantic test cases */
	incl divarw+4		/* L37 S8 A44 - s1RW */  
	cmp divarw+4, %eax	/* L38 A45 - s1R2R [one operand register] */
	mov $divarw, %edi
	mov $divarw+4, %esi
	cmpsb			/* L39 A46 - s1R2R [both memory], TWO MINITRAMPS */
	mov %edi, divarw+4	/* S9 A47 - s1W2R */
	add %eax, divarw	/* L40 S10 A48 - s1RW2R */
	xchg %edi, divarw+4	/* L41 S11 A49 - s1RW2RW */
	imul $3, divarw+8, %eax /* L42 A50 - s1W2R3R */
	mull divarw		/* L43 A51 - s1W2RW3R */
	shld $3, %eax, divarw+4	/* L44 S12 A52 - s1RW2R3R */
	mov $4, %eax
	mov $5, %edx
	movl $100, divarw	/* S13 A53 */
	idivl divarw		/* L45 A54 - s1RW2RW3R */

	/* MMX */
        call ia32features
        test $0x800000, %eax    /* 1<<23 */
        jz   .bail          /* assuming it cannot do sse/sse2*/
        rex64 movd divarw, %mm0	/* L46 A55, size = 8 */
	pmaddwd divarw+8, %mm0  /* L47 A56, size = 8 */
	psraw $2, %mm0
	movntq %mm0, divarw	/* LS14 A57, size = 8 */
	emms

	/* SSE */
	movaps dfvart, %xmm1	/* L48 A58, size = 16 */
	cmpeqss dfvart, %xmm0	/* L49 A59, size = 4 */
	prefetcht0 divarw	/* P1 A60 */

	/* SSE2 */
        call ia32features
        test $0x4000000, %eax    /* 1<<26 */
        jz   .bail
        movapd dfvart, %xmm1	/* L50 A61, size = 16 */
	cmpeqsd dfvart, %xmm0	/* L51 A62, size = 8 */
	psrldq $2, %xmm0

.bail:
        /* 3DNow! */
        call amd_features
        test $0x80000000, %eax
        jz   .bail2
	movq dfvard, %mm0	/* L52 A63, size = 8 */
	pfmin dfvard+8, %mm0	/* L53 A64, size = 8 */
	prefetch divarw		/* P2 A65 */
	femms
	
	/* REP prefixes */
.bail2:

	/* store 3 dword-sized "10"s to divarw */
	mov $3, %ecx
	mov $10, %eax
	mov $divarw, %rdi
	rep stosl		/* S15 A66, size = 12 */

	/* store 4 dword-sized "10"s to divarw (in backwards order) */
	std			/* we don't handle D=1 for repz and repnz */
	mov $4, %ecx
	mov $divarw+12, %rdi
	rep stosl			/* S16 A67, size = 16 */

	/* mov 4 dwords from ddvars to divarw */
	cld
	mov $4, %ecx
	mov $dfvars, %rsi
	mov $divarw, %rdi
	rep movsl		/* L54 S17 A68, size = 16, TWO MINITRAMPS */

	/* scan memory at dlarge for a zero byte */
	mov $0, %al
	mov $256, %ecx
	mov $dlarge, %rdi
	repne scasb		/* L55 A69, size = ? */

	/* string memory compare */
	mov $256, %ecx
	mov $dlarge+25, %rdi
	mov $dlarge+44, %rsi
	repe cmpsb		/* L56, A70, size = ?, TWO MINITRAMPS */
		
	/* x87 */

	/* loads */
	fld dfvars
	fldl dfvard
	fldt dfvart
	fild divarw
	fildl divarw+4
	fildq divarw+8		/* L62 A76 */

	/* stores */
	fst dfvars
	fstpl dfvard
	fstpt dfvart
	fist divarw+2
	fistl divarw+4
	fistpq divarw+8		/* S68 A82 */

	fnstcw divarw		/* S69 A83 */
	fldcw divarw		/* L63 A84 */

	fnstenv dlarge		/* S70 A85 */
	fldenv dlarge		/* L64 A86 */
	
	/* conditional moves */
	mov $3, %eax
	mov $2, %edx
	mov $15, %ecx
	mov $15, %ebx
	cmp %edx, %eax
	cmova divarw, %ecx	/* executed (3 > 2) - L65 A87 */
	cmove divarw+4, %ebx	/* not executed (3 != 2) - L66 A88 */
	mov divarw+8, %ebx	/* L67 A89 */
	
	/* restore saved registers (each is a load) */
	pop %r15
	pop %r14
	pop %r13
	pop %r12
	pop %rbx
	pop %rbp /* L59 A75 */
	
	/* tear down frame and return */
	/* we still have a decoding bug that does't count these instruction as loads */
	leave
	ret

ia32features:
        mov $1, %eax
        push %rbx                    /* cpuid changes ebx as well, but ebx "belongs" to the caller */
        cpuid
        mov %edx,%eax
        pop %rbx
        ret


amd_features:
        mov $0x80000000, %eax
        push %rbx                    /* cpuid changes ebx as well, but ebx "belongs" to the caller */
        cpuid
        cmp $0x80000000, %eax
        jbe .noext
        mov $0x80000001, %eax
        cpuid
        mov %edx,%eax
        jmp .done
.noext:
        mov $0, %eax
.done:
        pop %rbx
        ret
