#if defined(DYNINST_RT_STATIC_LIB)
.text 
.globl ___tls_get_addr
.globl __tls_get_addr

__tls_get_addr:
___tls_get_addr:
    mov (%rdi), %rax
    add %fs:0, %rax
    ret
#endif
