.globl dyninstTrapHandler2
.type dyninstTrapHandler2, @function
dyninstTrapHandler2:
        pushfq
        pushq %rbp
        mov %rsp, %rbp
        mov %rdx, %rdi
        leaq 0(%rip), %rdx
.Lnext_insn:
        addq $.Lret_point - .Lnext_insn, %rdx
        mov 0x8(%rbp), %rsi
        call dyninstSetupContext@PLT
        movl $0x0, 0x0
.Lret_point:
        pop %rax
        pop %r10
        pop %r11
        popfq
        retq

.section .note.GNU-stack,"",@progbits
