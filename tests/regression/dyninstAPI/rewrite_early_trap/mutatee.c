#include <stdio.h>

int mid_value(void);

// Three bytes: relocating it leaves a trap, not a jump, at this address.
__asm__(
  ".text\n"
  ".globl early_trap_target\n"
  ".type early_trap_target, @function\n"
  "early_trap_target:\n"
  "  xorl %eax, %eax\n"
  "  ret\n"
  ".size early_trap_target, . - early_trap_target\n"
);

int main(void) {
  printf("main reached, ctor_value=%d\n", mid_value());
  return mid_value() != 12;
}
