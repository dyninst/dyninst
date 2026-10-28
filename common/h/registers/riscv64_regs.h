/*
 * See the dyninst/COPYRIGHT file for copyright information.
 *
 * We provide the Paradyn Tools (below described as "Paradyn")
 * on an AS IS basis, and do not warrant its validity or performance.
 * We reserve the right to update, modify, or discontinue this
 * software at any time.  We shall have no obligation to supply such
 * updates or modifications or any other form of support to you.
 *
 * By your use of Paradyn, you understand and agree that we (or any
 * other person or entity with proprietary rights in Paradyn) are
 * under no obligation to provide either maintenance services,
 * update services, notices of latent defects, or correction of
 * defects for Paradyn.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

#ifndef DYNINST_RISCV64_REGS_H
#define DYNINST_RISCV64_REGS_H

//clang-format: off

#include "Architecture.h"
#include "registers/reg_def.h"

namespace Dyninst { namespace riscv64 {

  /**
   * For interpreting constants:
   *  Lowest 16 bits (0x000000ff) is base register ID
   *  Next 16 bits (0x0000ff00) are used to distinguish between 32/64 bit
   *                            floating point registers
   *  Next 16 bits (0x00ff0000) are the register category, GPR/FPR/SPR/CSR
   *  Top 16 bits (0xff000000) are the architecture.
   *
   *  These values/layout are not guaranteed to remain the same as part of the
   *  public interface, and may change.
   **/

  // 0xff000000  0x00ff0000      0x0000ff00      0x000000ff
  // arch        reg cat:GPR     alias&subrange  reg ID
  const int32_t GPR = 0x00010000; // general purpose registers
  const int32_t FPR = 0x00020000; // floating point registers
  //const int32_t VTR = 0x00040000; // vector registers
  const int32_t SPR = 0x00080000; // special purpose registers
  const int32_t CSR = 0x00100000; // control and status registers

  //          (         name,  ID | cat |         arch)
  // Single source of truth for this arch's registers: (name, encoding).
  // Expanded to the constexpr constants AND all_regs[] -- no duplication.
  #define DYNINST_RISCV64_REG_LIST(X, A) \
      X(x0, 0 | GPR | Arch_riscv64) \
      X(x1, 1 | GPR | Arch_riscv64) \
      X(x2, 2 | GPR | Arch_riscv64) \
      X(x3, 3 | GPR | Arch_riscv64) \
      X(x4, 4 | GPR | Arch_riscv64) \
      X(x5, 5 | GPR | Arch_riscv64) \
      X(x6, 6 | GPR | Arch_riscv64) \
      X(x7, 7 | GPR | Arch_riscv64) \
      X(x8, 8 | GPR | Arch_riscv64) \
      X(x9, 9 | GPR | Arch_riscv64) \
      X(x10, 10 | GPR | Arch_riscv64) \
      X(x11, 11 | GPR | Arch_riscv64) \
      X(x12, 12 | GPR | Arch_riscv64) \
      X(x13, 13 | GPR | Arch_riscv64) \
      X(x14, 14 | GPR | Arch_riscv64) \
      X(x15, 15 | GPR | Arch_riscv64) \
      X(x16, 16 | GPR | Arch_riscv64) \
      X(x17, 17 | GPR | Arch_riscv64) \
      X(x18, 18 | GPR | Arch_riscv64) \
      X(x19, 19 | GPR | Arch_riscv64) \
      X(x20, 20 | GPR | Arch_riscv64) \
      X(x21, 21 | GPR | Arch_riscv64) \
      X(x22, 22 | GPR | Arch_riscv64) \
      X(x23, 23 | GPR | Arch_riscv64) \
      X(x24, 24 | GPR | Arch_riscv64) \
      X(x25, 25 | GPR | Arch_riscv64) \
      X(x26, 26 | GPR | Arch_riscv64) \
      X(x27, 27 | GPR | Arch_riscv64) \
      X(x28, 28 | GPR | Arch_riscv64) \
      X(x29, 29 | GPR | Arch_riscv64) \
      X(x30, 30 | GPR | Arch_riscv64) \
      X(x31, 31 | GPR | Arch_riscv64) \
      X(f0, 0 | FPR | Arch_riscv64) \
      X(f1, 1 | FPR | Arch_riscv64) \
      X(f2, 2 | FPR | Arch_riscv64) \
      X(f3, 3 | FPR | Arch_riscv64) \
      X(f4, 4 | FPR | Arch_riscv64) \
      X(f5, 5 | FPR | Arch_riscv64) \
      X(f6, 6 | FPR | Arch_riscv64) \
      X(f7, 7 | FPR | Arch_riscv64) \
      X(f8, 8 | FPR | Arch_riscv64) \
      X(f9, 9 | FPR | Arch_riscv64) \
      X(f10, 10 | FPR | Arch_riscv64) \
      X(f11, 11 | FPR | Arch_riscv64) \
      X(f12, 12 | FPR | Arch_riscv64) \
      X(f13, 13 | FPR | Arch_riscv64) \
      X(f14, 14 | FPR | Arch_riscv64) \
      X(f15, 15 | FPR | Arch_riscv64) \
      X(f16, 16 | FPR | Arch_riscv64) \
      X(f17, 17 | FPR | Arch_riscv64) \
      X(f18, 18 | FPR | Arch_riscv64) \
      X(f19, 19 | FPR | Arch_riscv64) \
      X(f20, 20 | FPR | Arch_riscv64) \
      X(f21, 21 | FPR | Arch_riscv64) \
      X(f22, 22 | FPR | Arch_riscv64) \
      X(f23, 23 | FPR | Arch_riscv64) \
      X(f24, 24 | FPR | Arch_riscv64) \
      X(f25, 25 | FPR | Arch_riscv64) \
      X(f26, 26 | FPR | Arch_riscv64) \
      X(f27, 27 | FPR | Arch_riscv64) \
      X(f28, 28 | FPR | Arch_riscv64) \
      X(f29, 29 | FPR | Arch_riscv64) \
      X(f30, 30 | FPR | Arch_riscv64) \
      X(f31, 31 | FPR | Arch_riscv64) \
      X(pc, 0 | SPR | Arch_riscv64) \
      X(fflags, 0 | CSR | Arch_riscv64) \
      X(frm, 1 | CSR | Arch_riscv64) \
      X(fcsr, 2 | CSR | Arch_riscv64) \
      X(vstart, 3 | CSR | Arch_riscv64) \
      X(vxsat, 4 | CSR | Arch_riscv64) \
      X(vxrm, 5 | CSR | Arch_riscv64) \
      X(vcsr, 6 | CSR | Arch_riscv64) \
      X(ssp, 7 | CSR | Arch_riscv64) \
      X(seed, 8 | CSR | Arch_riscv64) \
      X(jvt, 9 | CSR | Arch_riscv64) \
      X(cycle, 10 | CSR | Arch_riscv64) \
      X(time, 11 | CSR | Arch_riscv64) \
      X(instret, 12 | CSR | Arch_riscv64) \
      X(hpmcounter3, 13 | CSR | Arch_riscv64) \
      X(hpmcounter4, 14 | CSR | Arch_riscv64) \
      X(hpmcounter5, 15 | CSR | Arch_riscv64) \
      X(hpmcounter6, 16 | CSR | Arch_riscv64) \
      X(hpmcounter7, 17 | CSR | Arch_riscv64) \
      X(hpmcounter8, 18 | CSR | Arch_riscv64) \
      X(hpmcounter9, 19 | CSR | Arch_riscv64) \
      X(hpmcounter10, 20 | CSR | Arch_riscv64) \
      X(hpmcounter11, 21 | CSR | Arch_riscv64) \
      X(hpmcounter12, 22 | CSR | Arch_riscv64) \
      X(hpmcounter13, 23 | CSR | Arch_riscv64) \
      X(hpmcounter14, 24 | CSR | Arch_riscv64) \
      X(hpmcounter15, 25 | CSR | Arch_riscv64) \
      X(hpmcounter16, 26 | CSR | Arch_riscv64) \
      X(hpmcounter17, 27 | CSR | Arch_riscv64) \
      X(hpmcounter18, 28 | CSR | Arch_riscv64) \
      X(hpmcounter19, 29 | CSR | Arch_riscv64) \
      X(hpmcounter20, 30 | CSR | Arch_riscv64) \
      X(hpmcounter21, 31 | CSR | Arch_riscv64) \
      X(hpmcounter22, 32 | CSR | Arch_riscv64) \
      X(hpmcounter23, 33 | CSR | Arch_riscv64) \
      X(hpmcounter24, 34 | CSR | Arch_riscv64) \
      X(hpmcounter25, 35 | CSR | Arch_riscv64) \
      X(hpmcounter26, 36 | CSR | Arch_riscv64) \
      X(hpmcounter27, 37 | CSR | Arch_riscv64) \
      X(hpmcounter28, 38 | CSR | Arch_riscv64) \
      X(hpmcounter29, 39 | CSR | Arch_riscv64) \
      X(hpmcounter30, 40 | CSR | Arch_riscv64) \
      X(hpmcounter31, 41 | CSR | Arch_riscv64) \
      X(vl, 42 | CSR | Arch_riscv64) \
      X(vtype, 43 | CSR | Arch_riscv64) \
      X(vlenb, 44 | CSR | Arch_riscv64) \
      X(cycleh, 45 | CSR | Arch_riscv64) \
      X(timeh, 46 | CSR | Arch_riscv64) \
      X(instreth, 47 | CSR | Arch_riscv64) \
      X(hpmcounter3h, 48 | CSR | Arch_riscv64) \
      X(hpmcounter4h, 49 | CSR | Arch_riscv64) \
      X(hpmcounter5h, 50 | CSR | Arch_riscv64) \
      X(hpmcounter6h, 51 | CSR | Arch_riscv64) \
      X(hpmcounter7h, 52 | CSR | Arch_riscv64) \
      X(hpmcounter8h, 53 | CSR | Arch_riscv64) \
      X(hpmcounter9h, 54 | CSR | Arch_riscv64) \
      X(hpmcounter10h, 55 | CSR | Arch_riscv64) \
      X(hpmcounter11h, 56 | CSR | Arch_riscv64) \
      X(hpmcounter12h, 57 | CSR | Arch_riscv64) \
      X(hpmcounter13h, 58 | CSR | Arch_riscv64) \
      X(hpmcounter14h, 59 | CSR | Arch_riscv64) \
      X(hpmcounter15h, 60 | CSR | Arch_riscv64) \
      X(hpmcounter16h, 61 | CSR | Arch_riscv64) \
      X(hpmcounter17h, 62 | CSR | Arch_riscv64) \
      X(hpmcounter18h, 63 | CSR | Arch_riscv64) \
      X(hpmcounter19h, 64 | CSR | Arch_riscv64) \
      X(hpmcounter20h, 65 | CSR | Arch_riscv64) \
      X(hpmcounter21h, 66 | CSR | Arch_riscv64) \
      X(hpmcounter22h, 67 | CSR | Arch_riscv64) \
      X(hpmcounter23h, 68 | CSR | Arch_riscv64) \
      X(hpmcounter24h, 69 | CSR | Arch_riscv64) \
      X(hpmcounter25h, 70 | CSR | Arch_riscv64) \
      X(hpmcounter26h, 71 | CSR | Arch_riscv64) \
      X(hpmcounter27h, 72 | CSR | Arch_riscv64) \
      X(hpmcounter28h, 73 | CSR | Arch_riscv64) \
      X(hpmcounter29h, 74 | CSR | Arch_riscv64) \
      X(hpmcounter30h, 75 | CSR | Arch_riscv64) \
      X(hpmcounter31h, 76 | CSR | Arch_riscv64) \
      A(zero, x0) \
      A(ra, x1) \
      A(sp, x2) \
      A(gp, x3) \
      A(tp, x4) \
      A(t0, x5) \
      A(t1, x6) \
      A(t2, x7) \
      A(fp, x8) \
      A(s0, x8) \
      A(s1, x9) \
      A(a0, x10) \
      A(a1, x11) \
      A(a2, x12) \
      A(a3, x13) \
      A(a4, x14) \
      A(a5, x15) \
      A(a6, x16) \
      A(a7, x17) \
      A(s2, x18) \
      A(s3, x19) \
      A(s4, x20) \
      A(s5, x21) \
      A(s6, x22) \
      A(s7, x23) \
      A(s8, x24) \
      A(s9, x25) \
      A(s10, x26) \
      A(s11, x27) \
      A(t3, x28) \
      A(t4, x29) \
      A(t5, x30) \
      A(t6, x31) \
      A(ft0, f0) \
      A(ft1, f1) \
      A(ft2, f2) \
      A(ft3, f3) \
      A(ft4, f4) \
      A(ft5, f5) \
      A(ft6, f6) \
      A(ft7, f7) \
      A(fs0, f8) \
      A(fs1, f9) \
      A(fa0, f10) \
      A(fa1, f11) \
      A(fa2, f12) \
      A(fa3, f13) \
      A(fa4, f14) \
      A(fa5, f15) \
      A(fa6, f16) \
      A(fa7, f17) \
      A(fs2, f18) \
      A(fs3, f19) \
      A(fs4, f20) \
      A(fs5, f21) \
      A(fs6, f22) \
      A(fs7, f23) \
      A(fs8, f24) \
      A(fs9, f25) \
      A(fs10, f26) \
      A(fs11, f27) \
      A(ft8, f28) \
      A(ft9, f29) \
      A(ft10, f30) \
      A(ft11, f31) \
      A(f0_32, f0) \
      A(f1_32, f1) \
      A(f2_32, f2) \
      A(f3_32, f3) \
      A(f4_32, f4) \
      A(f5_32, f5) \
      A(f6_32, f6) \
      A(f7_32, f7) \
      A(f8_32, f8) \
      A(f9_32, f9) \
      A(f10_32, f10) \
      A(f11_32, f11) \
      A(f12_32, f12) \
      A(f13_32, f13) \
      A(f14_32, f14) \
      A(f15_32, f15) \
      A(f16_32, f16) \
      A(f17_32, f17) \
      A(f18_32, f18) \
      A(f19_32, f19) \
      A(f20_32, f20) \
      A(f21_32, f21) \
      A(f22_32, f22) \
      A(f23_32, f23) \
      A(f24_32, f24) \
      A(f25_32, f25) \
      A(f26_32, f26) \
      A(f27_32, f27) \
      A(f28_32, f28) \
      A(f29_32, f29) \
      A(f30_32, f30) \
      A(f31_32, f31) \
      A(f0_64, f0) \
      A(f1_64, f1) \
      A(f2_64, f2) \
      A(f3_64, f3) \
      A(f4_64, f4) \
      A(f5_64, f5) \
      A(f6_64, f6) \
      A(f7_64, f7) \
      A(f8_64, f8) \
      A(f9_64, f9) \
      A(f10_64, f10) \
      A(f11_64, f11) \
      A(f12_64, f12) \
      A(f13_64, f13) \
      A(f14_64, f14) \
      A(f15_64, f15) \
      A(f16_64, f16) \
      A(f17_64, f17) \
      A(f18_64, f18) \
      A(f19_64, f19) \
      A(f20_64, f20) \
      A(f21_64, f21) \
      A(f22_64, f22) \
      A(f23_64, f23) \
      A(f24_64, f24) \
      A(f25_64, f25) \
      A(f26_64, f26) \
      A(f27_64, f27) \
      A(f28_64, f28) \
      A(f29_64, f29) \
      A(f30_64, f30) \
      A(f31_64, f31)

  #define DEF_ONE(n, v) DEF_REGISTER(n, v);
  #define DEF_ALS(n, t) DEF_REGISTER_ALIAS(n, t);
  DYNINST_RISCV64_REG_LIST(DEF_ONE, DEF_ALS)
  #undef DEF_ONE
  #undef DEF_ALS

  #define NAME_ONE(n, v) n,
  constexpr MachRegister all_regs[] = { DYNINST_RISCV64_REG_LIST(NAME_ONE, NAME_ONE) };
  #undef NAME_ONE


} // end of riscv64 namespace
}

#endif
