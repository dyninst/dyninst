#ifndef DYNINST_LOONGARCH64_REGS_H
#define DYNINST_LOONGARCH64_REGS_H

#include "Architecture.h"
#include "registers/reg_def.h"
#include <cstdint>

namespace Dyninst { namespace loongarch64 {

  // Register sizes
  const int32_t FULL   = 0x00000000;  // 64-bit
  const int32_t D_REG  = 0x00000100;  // 32-bit
  const int32_t W_REG  = 0x00000200;  // 16-bit
  const int32_t B_REG  = 0x00000300;  // 8-bit

  // Register categories
  const int32_t GPR    = 0x00000000;  // General purpose
  const int32_t FPR    = 0x10000000;  // Floating point
  const int32_t FLAG   = 0x20000000;  // Flags

  // GPR registers (r0-r31)
  DEF_REGISTER(                r0,  0 | FULL | GPR | Arch_loongarch64);
  DEF_REGISTER(                r1,  1 | FULL | GPR | Arch_loongarch64);
  DEF_REGISTER(                r2,  2 | FULL | GPR | Arch_loongarch64);
  DEF_REGISTER(                r3,  3 | FULL | GPR | Arch_loongarch64);
  DEF_REGISTER(                r4,  4 | FULL | GPR | Arch_loongarch64);
  DEF_REGISTER(                r5,  5 | FULL | GPR | Arch_loongarch64);
  DEF_REGISTER(                r6,  6 | FULL | GPR | Arch_loongarch64);
  DEF_REGISTER(                r7,  7 | FULL | GPR | Arch_loongarch64);
  DEF_REGISTER(                r8,  8 | FULL | GPR | Arch_loongarch64);
  DEF_REGISTER(                r9,  9 | FULL | GPR | Arch_loongarch64);
  DEF_REGISTER(               r10, 10 | FULL | GPR | Arch_loongarch64);
  DEF_REGISTER(               r11, 11 | FULL | GPR | Arch_loongarch64);
  DEF_REGISTER(               r12, 12 | FULL | GPR | Arch_loongarch64);
  DEF_REGISTER(               r13, 13 | FULL | GPR | Arch_loongarch64);
  DEF_REGISTER(               r14, 14 | FULL | GPR | Arch_loongarch64);
  DEF_REGISTER(               r15, 15 | FULL | GPR | Arch_loongarch64);
  DEF_REGISTER(               r16, 16 | FULL | GPR | Arch_loongarch64);
  DEF_REGISTER(               r17, 17 | FULL | GPR | Arch_loongarch64);
  DEF_REGISTER(               r18, 18 | FULL | GPR | Arch_loongarch64);
  DEF_REGISTER(               r19, 19 | FULL | GPR | Arch_loongarch64);
  DEF_REGISTER(               r20, 20 | FULL | GPR | Arch_loongarch64);
  DEF_REGISTER(               r21, 21 | FULL | GPR | Arch_loongarch64);
  DEF_REGISTER(               r22, 22 | FULL | GPR | Arch_loongarch64);
  DEF_REGISTER(               r23, 23 | FULL | GPR | Arch_loongarch64);
  DEF_REGISTER(               r24, 24 | FULL | GPR | Arch_loongarch64);
  DEF_REGISTER(               r25, 25 | FULL | GPR | Arch_loongarch64);
  DEF_REGISTER(               r26, 26 | FULL | GPR | Arch_loongarch64);
  DEF_REGISTER(               r27, 27 | FULL | GPR | Arch_loongarch64);
  DEF_REGISTER(               r28, 28 | FULL | GPR | Arch_loongarch64);
  DEF_REGISTER(               r29, 29 | FULL | GPR | Arch_loongarch64);
  DEF_REGISTER(               r30, 30 | FULL | GPR | Arch_loongarch64);
  DEF_REGISTER(               r31, 31 | FULL | GPR | Arch_loongarch64);

  // Aliases
  DEF_REGISTER(               zero,  0 | FULL | GPR | Arch_loongarch64);
  DEF_REGISTER(                ra,  1 | FULL | GPR | Arch_loongarch64);
  DEF_REGISTER(                tp,  2 | FULL | GPR | Arch_loongarch64);
  DEF_REGISTER(                sp,  3 | FULL | GPR | Arch_loongarch64);
  DEF_REGISTER(                a0,  4 | FULL | GPR | Arch_loongarch64);
  DEF_REGISTER(                a1,  5 | FULL | GPR | Arch_loongarch64);
  DEF_REGISTER(                a2,  6 | FULL | GPR | Arch_loongarch64);
  DEF_REGISTER(                a3,  7 | FULL | GPR | Arch_loongarch64);
  DEF_REGISTER(                a4,  8 | FULL | GPR | Arch_loongarch64);
  DEF_REGISTER(                a5,  9 | FULL | GPR | Arch_loongarch64);
  DEF_REGISTER(                a6, 10 | FULL | GPR | Arch_loongarch64);
  DEF_REGISTER(                a7, 11 | FULL | GPR | Arch_loongarch64);
  DEF_REGISTER(                t0, 12 | FULL | GPR | Arch_loongarch64);
  DEF_REGISTER(                t1, 13 | FULL | GPR | Arch_loongarch64);
  DEF_REGISTER(                t2, 14 | FULL | GPR | Arch_loongarch64);
  DEF_REGISTER(                t3, 15 | FULL | GPR | Arch_loongarch64);
  DEF_REGISTER(                t4, 16 | FULL | GPR | Arch_loongarch64);
  DEF_REGISTER(                t5, 17 | FULL | GPR | Arch_loongarch64);
  DEF_REGISTER(                t6, 18 | FULL | GPR | Arch_loongarch64);
  DEF_REGISTER(                t7, 19 | FULL | GPR | Arch_loongarch64);
  DEF_REGISTER(                t8, 20 | FULL | GPR | Arch_loongarch64);
  DEF_REGISTER(                fp, 22 | FULL | GPR | Arch_loongarch64);
  DEF_REGISTER(                s0, 22 | FULL | GPR | Arch_loongarch64);
  DEF_REGISTER(                s1, 23 | FULL | GPR | Arch_loongarch64);
  DEF_REGISTER(                s2, 24 | FULL | GPR | Arch_loongarch64);
  DEF_REGISTER(                s3, 25 | FULL | GPR | Arch_loongarch64);
  DEF_REGISTER(                s4, 26 | FULL | GPR | Arch_loongarch64);
  DEF_REGISTER(                s5, 27 | FULL | GPR | Arch_loongarch64);
  DEF_REGISTER(                s6, 28 | FULL | GPR | Arch_loongarch64);
  DEF_REGISTER(                s7, 29 | FULL | GPR | Arch_loongarch64);
  DEF_REGISTER(                s8, 30 | FULL | GPR | Arch_loongarch64);
  DEF_REGISTER(                s9, 31 | FULL | GPR | Arch_loongarch64);

  // FPR registers (f0-f31)
  DEF_REGISTER(                f0, 32 | FULL | FPR | Arch_loongarch64);
  DEF_REGISTER(                f1, 33 | FULL | FPR | Arch_loongarch64);
  DEF_REGISTER(                f2, 34 | FULL | FPR | Arch_loongarch64);
  DEF_REGISTER(                f3, 35 | FULL | FPR | Arch_loongarch64);
  DEF_REGISTER(                f4, 36 | FULL | FPR | Arch_loongarch64);
  DEF_REGISTER(                f5, 37 | FULL | FPR | Arch_loongarch64);
  DEF_REGISTER(                f6, 38 | FULL | FPR | Arch_loongarch64);
  DEF_REGISTER(                f7, 39 | FULL | FPR | Arch_loongarch64);
  DEF_REGISTER(                f8, 40 | FULL | FPR | Arch_loongarch64);
  DEF_REGISTER(                f9, 41 | FULL | FPR | Arch_loongarch64);
  DEF_REGISTER(               f10, 42 | FULL | FPR | Arch_loongarch64);
  DEF_REGISTER(               f11, 43 | FULL | FPR | Arch_loongarch64);
  DEF_REGISTER(               f12, 44 | FULL | FPR | Arch_loongarch64);
  DEF_REGISTER(               f13, 45 | FULL | FPR | Arch_loongarch64);
  DEF_REGISTER(               f14, 46 | FULL | FPR | Arch_loongarch64);
  DEF_REGISTER(               f15, 47 | FULL | FPR | Arch_loongarch64);
  DEF_REGISTER(               f16, 48 | FULL | FPR | Arch_loongarch64);
  DEF_REGISTER(               f17, 49 | FULL | FPR | Arch_loongarch64);
  DEF_REGISTER(               f18, 50 | FULL | FPR | Arch_loongarch64);
  DEF_REGISTER(               f19, 51 | FULL | FPR | Arch_loongarch64);
  DEF_REGISTER(               f20, 52 | FULL | FPR | Arch_loongarch64);
  DEF_REGISTER(               f21, 53 | FULL | FPR | Arch_loongarch64);
  DEF_REGISTER(               f22, 54 | FULL | FPR | Arch_loongarch64);
  DEF_REGISTER(               f23, 55 | FULL | FPR | Arch_loongarch64);
  DEF_REGISTER(               f24, 56 | FULL | FPR | Arch_loongarch64);
  DEF_REGISTER(               f25, 57 | FULL | FPR | Arch_loongarch64);
  DEF_REGISTER(               f26, 58 | FULL | FPR | Arch_loongarch64);
  DEF_REGISTER(               f27, 59 | FULL | FPR | Arch_loongarch64);
  DEF_REGISTER(               f28, 60 | FULL | FPR | Arch_loongarch64);
  DEF_REGISTER(               f29, 61 | FULL | FPR | Arch_loongarch64);
  DEF_REGISTER(               f30, 62 | FULL | FPR | Arch_loongarch64);
  DEF_REGISTER(               f31, 63 | FULL | FPR | Arch_loongarch64);

  // FPR aliases
  DEF_REGISTER(                fa0, 32 | FULL | FPR | Arch_loongarch64);
  DEF_REGISTER(                fa1, 33 | FULL | FPR | Arch_loongarch64);
  DEF_REGISTER(                fa2, 34 | FULL | FPR | Arch_loongarch64);
  DEF_REGISTER(                fa3, 35 | FULL | FPR | Arch_loongarch64);
  DEF_REGISTER(                fa4, 36 | FULL | FPR | Arch_loongarch64);
  DEF_REGISTER(                fa5, 37 | FULL | FPR | Arch_loongarch64);
  DEF_REGISTER(                fa6, 38 | FULL | FPR | Arch_loongarch64);
  DEF_REGISTER(                fa7, 39 | FULL | FPR | Arch_loongarch64);
  DEF_REGISTER(                ft0, 40 | FULL | FPR | Arch_loongarch64);
  DEF_REGISTER(                ft1, 41 | FULL | FPR | Arch_loongarch64);
  DEF_REGISTER(                ft2, 42 | FULL | FPR | Arch_loongarch64);
  DEF_REGISTER(                ft3, 43 | FULL | FPR | Arch_loongarch64);
  DEF_REGISTER(                ft4, 44 | FULL | FPR | Arch_loongarch64);
  DEF_REGISTER(                ft5, 45 | FULL | FPR | Arch_loongarch64);
  DEF_REGISTER(                ft6, 46 | FULL | FPR | Arch_loongarch64);
  DEF_REGISTER(                ft7, 47 | FULL | FPR | Arch_loongarch64);
  DEF_REGISTER(                fs0, 56 | FULL | FPR | Arch_loongarch64);
  DEF_REGISTER(                fs1, 57 | FULL | FPR | Arch_loongarch64);

} }
#endif
