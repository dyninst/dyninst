.globl dyninstTrapHandler
.type dyninstTrapHandler, @function
dyninstTrapHandler:
        pushf
        pushl %ebp    
        movl %esp, %ebp
        call .Lnext_insn
.Lnext_insn:
        addl $.Lret_point-.Lnext_insn,(%esp)
        pushl 0x4(%ebp) 
        pushl 0x14(%ebp)
        call dyninstSetupContext
        movl $0x0, 0x0
.Lret_point:
        popf
        ret
