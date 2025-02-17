#include "basereg_check.h"
#include "registers/riscv64_regs.h"

int main() {

  // GPR 64 -> 64
  BASEREG_CHECK(riscv64::x0, riscv64::x0);
  BASEREG_CHECK(riscv64::x1, riscv64::x1);
  BASEREG_CHECK(riscv64::x2, riscv64::x2);
  BASEREG_CHECK(riscv64::x3, riscv64::x3);
  BASEREG_CHECK(riscv64::x4, riscv64::x4);
  BASEREG_CHECK(riscv64::x5, riscv64::x5);
  BASEREG_CHECK(riscv64::x6, riscv64::x6);
  BASEREG_CHECK(riscv64::x7, riscv64::x7);
  BASEREG_CHECK(riscv64::x8, riscv64::x8);
  BASEREG_CHECK(riscv64::x9, riscv64::x9);
  BASEREG_CHECK(riscv64::x10, riscv64::x10);
  BASEREG_CHECK(riscv64::x11, riscv64::x11);
  BASEREG_CHECK(riscv64::x12, riscv64::x12);
  BASEREG_CHECK(riscv64::x13, riscv64::x13);
  BASEREG_CHECK(riscv64::x14, riscv64::x14);
  BASEREG_CHECK(riscv64::x15, riscv64::x15);
  BASEREG_CHECK(riscv64::x16, riscv64::x16);
  BASEREG_CHECK(riscv64::x17, riscv64::x17);
  BASEREG_CHECK(riscv64::x18, riscv64::x18);
  BASEREG_CHECK(riscv64::x19, riscv64::x19);
  BASEREG_CHECK(riscv64::x20, riscv64::x20);
  BASEREG_CHECK(riscv64::x21, riscv64::x21);
  BASEREG_CHECK(riscv64::x22, riscv64::x22);
  BASEREG_CHECK(riscv64::x23, riscv64::x23);
  BASEREG_CHECK(riscv64::x24, riscv64::x24);
  BASEREG_CHECK(riscv64::x25, riscv64::x25);
  BASEREG_CHECK(riscv64::x26, riscv64::x26);
  BASEREG_CHECK(riscv64::x27, riscv64::x27);
  BASEREG_CHECK(riscv64::x28, riscv64::x28);
  BASEREG_CHECK(riscv64::x29, riscv64::x29);
  BASEREG_CHECK(riscv64::x30, riscv64::x30);
  BASEREG_CHECK(riscv64::x31, riscv64::x31);

  // FPR * -> *
  BASEREG_CHECK(riscv64::f0, riscv64::f0);
  BASEREG_CHECK(riscv64::f1, riscv64::f1);
  BASEREG_CHECK(riscv64::f2, riscv64::f2);
  BASEREG_CHECK(riscv64::f3, riscv64::f3);
  BASEREG_CHECK(riscv64::f4, riscv64::f4);
  BASEREG_CHECK(riscv64::f5, riscv64::f5);
  BASEREG_CHECK(riscv64::f6, riscv64::f6);
  BASEREG_CHECK(riscv64::f7, riscv64::f7);
  BASEREG_CHECK(riscv64::f8, riscv64::f8);
  BASEREG_CHECK(riscv64::f9, riscv64::f9);
  BASEREG_CHECK(riscv64::f10, riscv64::f10);
  BASEREG_CHECK(riscv64::f11, riscv64::f11);
  BASEREG_CHECK(riscv64::f12, riscv64::f12);
  BASEREG_CHECK(riscv64::f13, riscv64::f13);
  BASEREG_CHECK(riscv64::f14, riscv64::f14);
  BASEREG_CHECK(riscv64::f15, riscv64::f15);
  BASEREG_CHECK(riscv64::f16, riscv64::f16);
  BASEREG_CHECK(riscv64::f17, riscv64::f17);
  BASEREG_CHECK(riscv64::f18, riscv64::f18);
  BASEREG_CHECK(riscv64::f19, riscv64::f19);
  BASEREG_CHECK(riscv64::f20, riscv64::f20);
  BASEREG_CHECK(riscv64::f21, riscv64::f21);
  BASEREG_CHECK(riscv64::f22, riscv64::f22);
  BASEREG_CHECK(riscv64::f23, riscv64::f23);
  BASEREG_CHECK(riscv64::f24, riscv64::f24);
  BASEREG_CHECK(riscv64::f25, riscv64::f25);
  BASEREG_CHECK(riscv64::f26, riscv64::f26);
  BASEREG_CHECK(riscv64::f27, riscv64::f27);
  BASEREG_CHECK(riscv64::f28, riscv64::f28);
  BASEREG_CHECK(riscv64::f29, riscv64::f29);
  BASEREG_CHECK(riscv64::f30, riscv64::f30);
  BASEREG_CHECK(riscv64::f31, riscv64::f31);

  return EXIT_SUCCESS;
}
