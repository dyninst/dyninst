/*
 * A load-reserve/store-conditional sequence that the thread cannot leave while the flag is 0.
 * A thread single-stepped into it stays there with ProcControl's emulated single step in
 * flight, until the mutator sets the flag.
 */

int window(volatile int *flag, int val);

/*
 * The reservation is held across a futex wait on the flag instead of a spin loop: the scan
 * puts a breakpoint at every branch target in the sequence, and the target of a spin loop's
 * back branch is inside it, so the thread would report a step on every iteration instead of
 * leaving the emulated step in flight.  The wait returns once the flag is no longer 0 (a
 * restarted wait sees the mutator's write).  The store usually fails because the stop cleared
 * the reservation; the flag is returned either way.
 */
#if defined(__aarch64__)
__asm__(".text\n"
        ".globl window\n"
        ".type window, %function\n"
        "window:\n"
        "    mov   x9, x0\n"
        "    mov   w10, w1\n"
        ".globl window_seq\n"
        "window_seq:\n"
        "    ldaxr w11, [x9]\n"
        "    mov   x1, #0\n" /* FUTEX_WAIT */
        "    mov   x2, #0\n" /* while *flag == 0 */
        "    mov   x3, #0\n" /* no timeout */
        "    mov   x8, #98\n" /* __NR_futex */
        "    svc   #0\n"
        "    stxr  w12, w10, [x9]\n"
        ".globl window_end\n"
        "window_end:\n"
        "    ldr   w0, [x9]\n"
        "    ret\n"
        ".size window, .-window\n");
#elif defined(__powerpc64__)
/* The system call may clobber r0 and r4-r12, so the flag's address and the value live in r30
 * and r31, saved in the red zone. */
__asm__(".text\n"
        ".globl window\n"
        ".type window, @function\n"
        "window:\n"
        "    std    30,-16(1)\n"
        "    std    31,-8(1)\n"
        "    mr     30,3\n"
        "    mr     31,4\n"
        ".globl window_seq\n"
        "window_seq:\n"
        "    lwarx  11,0,30\n"
        "    li     0,221\n" /* __NR_futex */
        "    mr     3,30\n"
        "    li     4,0\n" /* FUTEX_WAIT */
        "    li     5,0\n" /* while *flag == 0 */
        "    li     6,0\n" /* no timeout */
        "    sc\n"
        "    stwcx. 31,0,30\n"
        ".globl window_end\n"
        "window_end:\n"
        "    lwz    3,0(30)\n"
        "    ld     30,-16(1)\n"
        "    ld     31,-8(1)\n"
        "    blr\n"
        ".size window, .-window\n");
#else
/* No load-reserve sequence here: the test reports itself skipped before it runs the mutatee. */
int window(volatile int *flag, int val) {
  (void)val;
  return *flag;
}
#endif

volatile int flag = 0;

/* No globals, so on ppc64le the global and local entry points coincide. */
__attribute__((noinline, used)) void before_window(void) { __asm__ volatile("" ::: "memory"); }

int main(void) {
  before_window();
  /* the mutator sets the flag to 1; the sequence may store 2 over it */
  int seen = window(&flag, 2);
  return seen == 1 || seen == 2 ? 0 : 1;
}
