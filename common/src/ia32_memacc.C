#include "ia32_memacc.h"

#include <cstdio>

void ia32_memacc::print() {
  fprintf(stderr, "base: %d, index: %d, scale:%d, disp: %ld (%lx), size: %d, addr_size: %d\n",
          regs[0], regs[1], scale, imm, (unsigned long)imm, size, addr_size);
}

