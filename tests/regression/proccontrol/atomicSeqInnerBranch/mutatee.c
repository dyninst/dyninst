/*
 * A load-reserve/store-conditional sequence with a branch back to the reserve before the
 * store-conditional, so the branch target is inside the sequence. The word is nonzero, so
 * the branch is not taken and the sequence runs straight through to the store.
 */

int inner_seq(volatile int *p, int newval);

#if defined(__aarch64__)
__asm__(".text\n"
        ".globl inner_seq\n"
        ".type inner_seq, %function\n"
        "inner_seq:\n"
        "    nop\n"
        ".globl inner_seq_reserve\n"
        "inner_seq_reserve:\n"
        "1:  ldaxr w2, [x0]\n"
        "    cbz   w2, 1b\n"
        "    stxr  w3, w1, [x0]\n"
        ".globl inner_seq_after_store\n"
        "inner_seq_after_store:\n"
        "    cbnz  w3, 1b\n"
        "    mov   w0, w2\n"
        "    ret\n"
        ".size inner_seq, .-inner_seq\n");
#elif defined(__powerpc64__)
__asm__(".text\n"
        ".globl inner_seq\n"
        ".type inner_seq, @function\n"
        "inner_seq:\n"
        "    nop\n"
        ".globl inner_seq_reserve\n"
        "inner_seq_reserve:\n"
        "1:  lwarx  5,0,3\n"
        "    cmpwi  5,0\n"
        "    beq-   1b\n"
        "    stwcx. 4,0,3\n"
        ".globl inner_seq_after_store\n"
        "inner_seq_after_store:\n"
        "    bne-   1b\n"
        "    mr     3,5\n"
        "    blr\n"
        ".size inner_seq, .-inner_seq\n");
#endif

#if defined(__aarch64__) || defined(__powerpc64__)
volatile int word = 5;

int main(void) {
  /* the old value is returned and the new one stored */
  return inner_seq(&word, 9) == 5 && word == 9 ? 0 : 1;
}
#else
/* Not run: the test skips on architectures without an emulated step through the sequence */
int main(void) { return 0; }
#endif
