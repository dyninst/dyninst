#include "basereg_check.h"
#include "registers/riscv64_regs.h"

int main() {

  // GPR 64 -> 64
  BASEREG_CHECK(riscv64::x0, riscv64::x0);
  BASEREG_CHECK(riscv64::x8, riscv64::x8);
  BASEREG_CHECK(riscv64::x31, riscv64::x31);

  // FPR 64 Boxed -> 64 Boxed
  BASEREG_CHECK(riscv64::f0, riscv64::f0);
  BASEREG_CHECK(riscv64::f8, riscv64::f8);
  BASEREG_CHECK(riscv64::f31, riscv64::f31);

  // Aliases
  BASEREG_CHECK(riscv64::zero, riscv64::x0);
  BASEREG_CHECK(riscv64::ra, riscv64::x1);
  BASEREG_CHECK(riscv64::sp, riscv64::x2);
  BASEREG_CHECK(riscv64::gp, riscv64::x3);
  BASEREG_CHECK(riscv64::tp, riscv64::x4);
  BASEREG_CHECK(riscv64::t0, riscv64::x5);
  BASEREG_CHECK(riscv64::t2, riscv64::x7);
  BASEREG_CHECK(riscv64::fp, riscv64::x8);
  BASEREG_CHECK(riscv64::s0, riscv64::x8);
  BASEREG_CHECK(riscv64::s1, riscv64::x9);
  BASEREG_CHECK(riscv64::a0, riscv64::x10);
  BASEREG_CHECK(riscv64::a1, riscv64::x11);
  BASEREG_CHECK(riscv64::a7, riscv64::x17);
  BASEREG_CHECK(riscv64::s2, riscv64::x18);
  BASEREG_CHECK(riscv64::s11, riscv64::x27);
  BASEREG_CHECK(riscv64::t3, riscv64::x28);
  BASEREG_CHECK(riscv64::t6, riscv64::x31);

  BASEREG_CHECK(riscv64::f0_32, riscv64::f0);
  BASEREG_CHECK(riscv64::f8_32, riscv64::f8);
  BASEREG_CHECK(riscv64::f31_32, riscv64::f31);

  BASEREG_CHECK(riscv64::f0_64, riscv64::f0);
  BASEREG_CHECK(riscv64::f8_64, riscv64::f8);
  BASEREG_CHECK(riscv64::f31_64, riscv64::f31);

  return EXIT_SUCCESS;
}
