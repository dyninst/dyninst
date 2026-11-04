#include <stdint.h>
#include <stdio.h>

/* far_value and far_fn are initialized, so they are in .data, next to the
 * code. The binary rewriter puts relocated code after the end of .bss, and pad
 * makes that more than 2 GiB past them. Nothing touches pad, so it costs no
 * memory. */
int far_value = 42;
char pad[0xa0000000];

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
  if (entered != 1) {
    fprintf(stderr, "leaf's entry snippet ran %d times, not once\n", entered);
    failed = 1;
  }
  return failed;
}
