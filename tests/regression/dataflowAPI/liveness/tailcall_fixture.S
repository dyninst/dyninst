/* Never executed: x86_64 functions that leave through a branch into another function. */
.text
.globl callee, jmp_tail, jne_tail, table_tail
.hidden callee, jmp_tail, jne_tail, table_tail

.type callee, @function
callee:
    ret
.size callee, .-callee

/* An unconditional tail call. */
.type jmp_tail, @function
jmp_tail:
    mov (%rdi), %rdi
    jmp callee
.size jmp_tail, .-jmp_tail

/* A conditional tail call: the taken branch enters callee, the fall-through returns. */
.type jne_tail, @function
jne_tail:
    mov (%rdi), %rdi
    test %rdi, %rdi
    jne callee
    ret
.size jne_tail, .-jne_tail

/* A jump table with one slot inside the function and one at the entry of callee. */
.type table_tail, @function
table_tail:
    cmp $1, %rsi
    ja .Ldefault
    lea .Ltable(%rip), %rax
    movslq (%rax,%rsi,4), %rcx
    add %rax, %rcx
    jmp *%rcx
.Lcase0:
    xor %eax, %eax
    ret
.Ldefault:
    mov $1, %eax
    ret
.size table_tail, .-table_tail

.section .rodata
.balign 4
.Ltable:
    .long .Lcase0 - .Ltable
    .long callee - .Ltable

.section .note.GNU-stack, "", @progbits
