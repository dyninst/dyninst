.file "RTspace.s"
        
.globl DYNINSTstaticHeap_8K_lowmemHeap_1
.type DYNINSTstaticHeap_8K_lowmemHeap_1, @object
.size DYNINSTstaticHeap_8K_lowmemHeap_1, 8192

.globl DYNINSTstaticHeap_32K_anyHeap_1
.type DYNINSTstaticHeap_32K_anyHeap_1, @object
.size DYNINSTstaticHeap_32K_anyHeap_1, 32768

.section .dyninst_heap,"awx",@nobits
.align 16
DYNINSTstaticHeap_8K_lowmemHeap_1:
        .skip 8192
DYNINSTstaticHeap_32K_anyHeap_1:
        .skip 32768

.section .note.GNU-stack,"",@progbits
