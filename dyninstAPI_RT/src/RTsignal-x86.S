.globl dyninstTrapHandler2
.type dyninstTrapHandler2, @function
dyninstTrapHandler2:
        pushf
        pushl %ebp
        movl %esp, %ebp
        call __i686.get_pc_thunk.bx
        addl $_GLOBAL_OFFSET_TABLE_, %ebx
        call .Lnext_insn
.Lnext_insn:
        addl $.Lret_point-.Lnext_insn,(%esp)
        pushl 0x4(%ebp) 
        pushl 0x14(%ebp)
        call dyninstSetupContext@PLT
        movl $0x0, 0x0
.Lret_point:
        popf
        ret

#if defined(DYNINST_RT_STATIC_LIB)
__i686.get_pc_thunk.bx:
        mov (%esp), %ebx
        ret

__i686.get_pc_thunk.cx:
        mov (%esp), %ecx
        ret
#endif

.section .note.GNU-stack,"",@progbits
