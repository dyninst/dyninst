#include <stdint.h>
#include <stdio.h>

long seen_arg7 = -1;
int seen_aligned = -1;

__attribute__((noinline)) long callee(long a1, long a2, long a3, long a4, long a5, long a6,
                                      long a7) {
  /* At -O0 the frame address is the entry %rsp minus 8, which is 16-byte
   * aligned when the call was made with an aligned %rsp. */
  seen_aligned = (uintptr_t)__builtin_frame_address(0) % 16 == 0;
  seen_arg7 = a7;
  return a1 + a2 + a3 + a4 + a5 + a6 + a7;
}

/* far_value and far_fn are initialized, so they are in .data, next to the
 * code. The binary rewriter puts relocated code after the end of .bss, and pad
 * makes that more than 2 GiB past them. Nothing touches pad, so it costs no
 * memory. */
int far_value = 42;
long (*far_fn)(long, long, long, long, long, long, long) = callee;
char pad[0xa0000000];

/* far_caller calls far_fn with a RIP-relative indirect call, the seventh
 * argument on the stack and %rsp 16-byte aligned at the call. */
__asm__(".text\n"
        ".globl far_caller\n"
        ".type far_caller, @function\n"
        "far_caller:\n"
        "  push %rbp\n"
        "  mov %rsp, %rbp\n"
        "  mov $1, %edi\n"
        "  mov $2, %esi\n"
        "  mov $3, %edx\n"
        "  mov $4, %ecx\n"
        "  mov $5, %r8d\n"
        "  mov $6, %r9d\n"
        "  sub $8, %rsp\n"
        "  push $7\n"
        "  call *far_fn(%rip)\n"
        "  leave\n"
        "  ret\n"
        ".size far_caller, . - far_caller\n");
long far_caller(void);

int entered = -1;
int seen_value = -1;
int seen_canary[4] = {-1, -1, -1, -1};

#define CANARY(i) (0x11111111 * ((i) + 1))

/* leaf is written in assembly so that it is exactly this under any compiler:
 * it calls nothing and never moves %rsp, keeps CANARY(0..3) in the 16 bytes
 * of red zone just under the stack pointer, and reads far_value with one
 * RIP-relative load. */
__asm__(".text\n"
        ".globl leaf\n"
        ".type leaf, @function\n"
        "leaf:\n"
        "  push %rbp\n"
        "  mov %rsp, %rbp\n"
        "  movl $0x11111111, -0x10(%rbp)\n"
        "  movl $0x22222222, -0xc(%rbp)\n"
        "  movl $0x33333333, -0x8(%rbp)\n"
        "  movl $0x44444444, -0x4(%rbp)\n"
        "  mov far_value(%rip), %eax\n"
        "  mov %eax, seen_value(%rip)\n"
        "  mov -0x10(%rbp), %eax\n"
        "  mov %eax, seen_canary(%rip)\n"
        "  mov -0xc(%rbp), %eax\n"
        "  mov %eax, seen_canary+4(%rip)\n"
        "  mov -0x8(%rbp), %eax\n"
        "  mov %eax, seen_canary+8(%rip)\n"
        "  mov -0x4(%rbp), %eax\n"
        "  mov %eax, seen_canary+12(%rip)\n"
        "  pop %rbp\n"
        "  ret\n"
        ".size leaf, . - leaf\n");
void leaf(void);

int main(void) {
  entered = 0;
  leaf();
  long const sum = far_caller();
  int failed = 0;
  for (int i = 0; i < 4; i++) {
    if (seen_canary[i] != CANARY(i)) {
      fprintf(stderr, "leaf's canary[%d] was overwritten: %#x, not %#x\n", i, seen_canary[i],
              CANARY(i));
      failed = 1;
    }
  }
  if (seen_value != 42) {
    fprintf(stderr, "leaf read far_value as %d, not 42\n", seen_value);
    failed = 1;
  }
  if (seen_arg7 != 7 || !seen_aligned || sum != 28) {
    fprintf(stderr,
            "callee got a seventh argument of %ld, not 7, ran with %s stack and returned "
            "%ld, not 28\n",
            seen_arg7, seen_aligned ? "an aligned" : "a misaligned", sum);
    failed = 1;
  }
  if (entered != 2) {
    fprintf(stderr, "the entry snippets of leaf and far_caller ran %d times, not twice\n",
            entered);
    failed = 1;
  }
  return failed;
}
