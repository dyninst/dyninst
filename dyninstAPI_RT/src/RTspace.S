.file "RTspace.s"
        
.globl DYNINSTstaticHeap_512K_lowmemHeap_1
.type DYNINSTstaticHeap_512K_lowmemHeap_1, @object
.size DYNINSTstaticHeap_512K_lowmemHeap_1, 524288

.globl DYNINSTstaticHeap_16M_anyHeap_1        
.type DYNINSTstaticHeap_16M_anyHeap_1, @object
.size DYNINSTstaticHeap_16M_anyHeap_1, 16777216

.section .dyninst_heap,"aw",@nobits
.align 16
DYNINSTstaticHeap_512K_lowmemHeap_1:
        .skip 524288
DYNINSTstaticHeap_16M_anyHeap_1:
        .skip 16777216

.section .note.GNU-stack,"",@progbits
