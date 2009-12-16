#if defined(DYNINST_RT_STATIC_LIB)
.text 
.globl ___tls_get_addr

___tls_get_addr:
    mov (%eax), %eax
    add %gs:0, %eax
    ret
#endif
