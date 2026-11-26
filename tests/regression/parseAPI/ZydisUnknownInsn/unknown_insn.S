/*
 * A function whose only path to `ret` crosses two instructions InstructionAPI cannot decode
 * (found in TensorFlow's libtensorflow_framework.so.2), followed by a function that starts right
 * after it.
 */
        .text
        .globl  past_unknown
        .type   past_unknown, @function
        .p2align 4
past_unknown:
        vmovaps %ymm0, 0xe0(%rsp)       /* c5 fc 29 84 24 e0 00 00 00 */
        vmovq   %xmm0, 0x20(%rbx)       /* c5 f9 d6 43 20 */
        ret
        .size   past_unknown, .-past_unknown

        .globl  after
        .type   after, @function
        .p2align 4
after:
        ret
        .size   after, .-after

        .section .note.GNU-stack,"",@progbits
