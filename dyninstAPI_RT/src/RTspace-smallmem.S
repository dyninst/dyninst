.file "RTspace.s"
        
.globl DYNINSTstaticHeap_512K_lowmemHeap_1
.type DYNINSTstaticHeap_512K_lowmemHeap_1, @object
.size DYNINSTstaticHeap_512K_lowmemHeap_1, 4096

.globl DYNINSTstaticHeap_16M_anyHeap_1        
.type DYNINSTstaticHeap_16M_anyHeap_1, @object
.size DYNINSTstaticHeap_16M_anyHeap_1, 16384

.section .dyninst_heap,"awx",@nobits
.align 16
DYNINSTstaticHeap_512K_lowmemHeap_1:
        .skip 1024
DYNINSTstaticHeap_16M_anyHeap_1:
        .skip 1024

