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
   *  Next 16 bits (0x0000ff00) are used to distinguish between 32/64 bit floating point registers
   *  Next 16 bits (0x00ff0000) are the register category, GPR/FPR/MMX/...
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

  //          (      name,  ID | cat |         arch,      arch)
  DEF_REGISTER(        x0,   0 | GPR | Arch_riscv64, "riscv64");
  DEF_REGISTER(        x1,   1 | GPR | Arch_riscv64, "riscv64");
  DEF_REGISTER(        x2,   2 | GPR | Arch_riscv64, "riscv64");
  DEF_REGISTER(        x3,   3 | GPR | Arch_riscv64, "riscv64");
  DEF_REGISTER(        x4,   4 | GPR | Arch_riscv64, "riscv64");
  DEF_REGISTER(        x5,   5 | GPR | Arch_riscv64, "riscv64");
  DEF_REGISTER(        x6,   6 | GPR | Arch_riscv64, "riscv64");
  DEF_REGISTER(        x7,   7 | GPR | Arch_riscv64, "riscv64");
  DEF_REGISTER(        x8,   8 | GPR | Arch_riscv64, "riscv64");
  DEF_REGISTER(        x9,   9 | GPR | Arch_riscv64, "riscv64");
  DEF_REGISTER(       x10,  10 | GPR | Arch_riscv64, "riscv64");
  DEF_REGISTER(       x11,  11 | GPR | Arch_riscv64, "riscv64");
  DEF_REGISTER(       x12,  12 | GPR | Arch_riscv64, "riscv64");
  DEF_REGISTER(       x13,  13 | GPR | Arch_riscv64, "riscv64");
  DEF_REGISTER(       x14,  14 | GPR | Arch_riscv64, "riscv64");
  DEF_REGISTER(       x15,  15 | GPR | Arch_riscv64, "riscv64");
  DEF_REGISTER(       x16,  16 | GPR | Arch_riscv64, "riscv64");
  DEF_REGISTER(       x17,  17 | GPR | Arch_riscv64, "riscv64");
  DEF_REGISTER(       x18,  18 | GPR | Arch_riscv64, "riscv64");
  DEF_REGISTER(       x19,  19 | GPR | Arch_riscv64, "riscv64");
  DEF_REGISTER(       x20,  20 | GPR | Arch_riscv64, "riscv64");
  DEF_REGISTER(       x21,  21 | GPR | Arch_riscv64, "riscv64");
  DEF_REGISTER(       x22,  22 | GPR | Arch_riscv64, "riscv64");
  DEF_REGISTER(       x23,  23 | GPR | Arch_riscv64, "riscv64");
  DEF_REGISTER(       x24,  24 | GPR | Arch_riscv64, "riscv64");
  DEF_REGISTER(       x25,  25 | GPR | Arch_riscv64, "riscv64");
  DEF_REGISTER(       x26,  26 | GPR | Arch_riscv64, "riscv64");
  DEF_REGISTER(       x27,  27 | GPR | Arch_riscv64, "riscv64");
  DEF_REGISTER(       x28,  28 | GPR | Arch_riscv64, "riscv64");
  DEF_REGISTER(       x29,  29 | GPR | Arch_riscv64, "riscv64");
  DEF_REGISTER(       x30,  30 | GPR | Arch_riscv64, "riscv64");
  DEF_REGISTER(       x31,  31 | GPR | Arch_riscv64, "riscv64");

  // F extension defines 32 bit floating point registers f0 ~ f31
  // D extension widens f0 ~ f31, to 64 bits

  // In riscv64, 32-bit FPRs are NaN-boxed to 64-bit FPRs.
  // In other words, in RI64, when accessing 32-bit FPRs, you are actually accessing the entire register.

  // Capstone distinguishes between 32-bit FPRs and 64-bit FPRs.
  // So far, other libraries such as Dwarf does not distinguish between the two.
  // So here f<N>_32 and f<N>_64 are used to handle Capstone registers
  // On the other hand, f<N> are the "actual" FPRs that include both the NaN-boxed f<N>_32 and f<N>_64.
  // f<N>_32 and f<N>_64 are both aliases of f<N>

  // Unless you are dealing with Capstone, you should always use f<N>.

  // The "actual" 64 bit FPRs, including NaN-boxed 32-bit and 64-bit FPRs
  DEF_REGISTER(        f0,   0 | FPR | Arch_riscv64, "riscv64");
  DEF_REGISTER(        f1,   1 | FPR | Arch_riscv64, "riscv64");
  DEF_REGISTER(        f2,   2 | FPR | Arch_riscv64, "riscv64");
  DEF_REGISTER(        f3,   3 | FPR | Arch_riscv64, "riscv64");
  DEF_REGISTER(        f4,   4 | FPR | Arch_riscv64, "riscv64");
  DEF_REGISTER(        f5,   5 | FPR | Arch_riscv64, "riscv64");
  DEF_REGISTER(        f6,   6 | FPR | Arch_riscv64, "riscv64");
  DEF_REGISTER(        f7,   7 | FPR | Arch_riscv64, "riscv64");
  DEF_REGISTER(        f8,   8 | FPR | Arch_riscv64, "riscv64");
  DEF_REGISTER(        f9,   9 | FPR | Arch_riscv64, "riscv64");
  DEF_REGISTER(       f10,  10 | FPR | Arch_riscv64, "riscv64");
  DEF_REGISTER(       f11,  11 | FPR | Arch_riscv64, "riscv64");
  DEF_REGISTER(       f12,  12 | FPR | Arch_riscv64, "riscv64");
  DEF_REGISTER(       f13,  13 | FPR | Arch_riscv64, "riscv64");
  DEF_REGISTER(       f14,  14 | FPR | Arch_riscv64, "riscv64");
  DEF_REGISTER(       f15,  15 | FPR | Arch_riscv64, "riscv64");
  DEF_REGISTER(       f16,  16 | FPR | Arch_riscv64, "riscv64");
  DEF_REGISTER(       f17,  17 | FPR | Arch_riscv64, "riscv64");
  DEF_REGISTER(       f18,  18 | FPR | Arch_riscv64, "riscv64");
  DEF_REGISTER(       f19,  19 | FPR | Arch_riscv64, "riscv64");
  DEF_REGISTER(       f20,  20 | FPR | Arch_riscv64, "riscv64");
  DEF_REGISTER(       f21,  21 | FPR | Arch_riscv64, "riscv64");
  DEF_REGISTER(       f22,  22 | FPR | Arch_riscv64, "riscv64");
  DEF_REGISTER(       f23,  23 | FPR | Arch_riscv64, "riscv64");
  DEF_REGISTER(       f24,  24 | FPR | Arch_riscv64, "riscv64");
  DEF_REGISTER(       f25,  25 | FPR | Arch_riscv64, "riscv64");
  DEF_REGISTER(       f26,  26 | FPR | Arch_riscv64, "riscv64");
  DEF_REGISTER(       f27,  27 | FPR | Arch_riscv64, "riscv64");
  DEF_REGISTER(       f28,  28 | FPR | Arch_riscv64, "riscv64");
  DEF_REGISTER(       f29,  29 | FPR | Arch_riscv64, "riscv64");
  DEF_REGISTER(       f30,  30 | FPR | Arch_riscv64, "riscv64");
  DEF_REGISTER(       f31,  31 | FPR | Arch_riscv64, "riscv64");

  // Aliases (ABI Name)
  DEF_REGISTER(      zero,   0 | GPR | Arch_riscv64, "riscv64");
  DEF_REGISTER(        ra,   1 | GPR | Arch_riscv64, "riscv64");
  DEF_REGISTER(        sp,   2 | GPR | Arch_riscv64, "riscv64");
  DEF_REGISTER(        gp,   3 | GPR | Arch_riscv64, "riscv64");
  DEF_REGISTER(        tp,   4 | GPR | Arch_riscv64, "riscv64");
  DEF_REGISTER(        t0,   5 | GPR | Arch_riscv64, "riscv64");
  DEF_REGISTER(        t1,   6 | GPR | Arch_riscv64, "riscv64");
  DEF_REGISTER(        t2,   7 | GPR | Arch_riscv64, "riscv64");
  DEF_REGISTER(        fp,   8 | GPR | Arch_riscv64, "riscv64");
  DEF_REGISTER(        s0,   8 | GPR | Arch_riscv64, "riscv64");
  DEF_REGISTER(        s1,   9 | GPR | Arch_riscv64, "riscv64");
  DEF_REGISTER(        a0,  10 | GPR | Arch_riscv64, "riscv64");
  DEF_REGISTER(        a1,  11 | GPR | Arch_riscv64, "riscv64");
  DEF_REGISTER(        a2,  12 | GPR | Arch_riscv64, "riscv64");
  DEF_REGISTER(        a3,  13 | GPR | Arch_riscv64, "riscv64");
  DEF_REGISTER(        a4,  14 | GPR | Arch_riscv64, "riscv64");
  DEF_REGISTER(        a5,  15 | GPR | Arch_riscv64, "riscv64");
  DEF_REGISTER(        a6,  16 | GPR | Arch_riscv64, "riscv64");
  DEF_REGISTER(        a7,  17 | GPR | Arch_riscv64, "riscv64");
  DEF_REGISTER(        s2,  18 | GPR | Arch_riscv64, "riscv64");
  DEF_REGISTER(        s3,  19 | GPR | Arch_riscv64, "riscv64");
  DEF_REGISTER(        s4,  20 | GPR | Arch_riscv64, "riscv64");
  DEF_REGISTER(        s5,  21 | GPR | Arch_riscv64, "riscv64");
  DEF_REGISTER(        s6,  22 | GPR | Arch_riscv64, "riscv64");
  DEF_REGISTER(        s7,  23 | GPR | Arch_riscv64, "riscv64");
  DEF_REGISTER(        s8,  24 | GPR | Arch_riscv64, "riscv64");
  DEF_REGISTER(        s9,  25 | GPR | Arch_riscv64, "riscv64");
  DEF_REGISTER(       s10,  26 | GPR | Arch_riscv64, "riscv64");
  DEF_REGISTER(       s11,  27 | GPR | Arch_riscv64, "riscv64");
  DEF_REGISTER(        t3,  28 | GPR | Arch_riscv64, "riscv64");
  DEF_REGISTER(        t4,  29 | GPR | Arch_riscv64, "riscv64");
  DEF_REGISTER(        t5,  31 | GPR | Arch_riscv64, "riscv64");
  DEF_REGISTER(        t6,  30 | GPR | Arch_riscv64, "riscv64");

  // 32 bit FPRs f<N>_32 (Mainly for Capstone compatibility)
  DEF_REGISTER(     f0_32,   0 | FPR | Arch_riscv64, "riscv64");
  DEF_REGISTER(     f1_32,   1 | FPR | Arch_riscv64, "riscv64");
  DEF_REGISTER(     f2_32,   2 | FPR | Arch_riscv64, "riscv64");
  DEF_REGISTER(     f3_32,   3 | FPR | Arch_riscv64, "riscv64");
  DEF_REGISTER(     f4_32,   4 | FPR | Arch_riscv64, "riscv64");
  DEF_REGISTER(     f5_32,   5 | FPR | Arch_riscv64, "riscv64");
  DEF_REGISTER(     f6_32,   6 | FPR | Arch_riscv64, "riscv64");
  DEF_REGISTER(     f7_32,   7 | FPR | Arch_riscv64, "riscv64");
  DEF_REGISTER(     f8_32,   8 | FPR | Arch_riscv64, "riscv64");
  DEF_REGISTER(     f9_32,   9 | FPR | Arch_riscv64, "riscv64");
  DEF_REGISTER(    f10_32,  10 | FPR | Arch_riscv64, "riscv64");
  DEF_REGISTER(    f11_32,  11 | FPR | Arch_riscv64, "riscv64");
  DEF_REGISTER(    f12_32,  12 | FPR | Arch_riscv64, "riscv64");
  DEF_REGISTER(    f13_32,  13 | FPR | Arch_riscv64, "riscv64");
  DEF_REGISTER(    f14_32,  14 | FPR | Arch_riscv64, "riscv64");
  DEF_REGISTER(    f15_32,  15 | FPR | Arch_riscv64, "riscv64");
  DEF_REGISTER(    f16_32,  16 | FPR | Arch_riscv64, "riscv64");
  DEF_REGISTER(    f17_32,  17 | FPR | Arch_riscv64, "riscv64");
  DEF_REGISTER(    f18_32,  18 | FPR | Arch_riscv64, "riscv64");
  DEF_REGISTER(    f19_32,  19 | FPR | Arch_riscv64, "riscv64");
  DEF_REGISTER(    f20_32,  20 | FPR | Arch_riscv64, "riscv64");
  DEF_REGISTER(    f21_32,  21 | FPR | Arch_riscv64, "riscv64");
  DEF_REGISTER(    f22_32,  22 | FPR | Arch_riscv64, "riscv64");
  DEF_REGISTER(    f23_32,  23 | FPR | Arch_riscv64, "riscv64");
  DEF_REGISTER(    f24_32,  24 | FPR | Arch_riscv64, "riscv64");
  DEF_REGISTER(    f25_32,  25 | FPR | Arch_riscv64, "riscv64");
  DEF_REGISTER(    f26_32,  26 | FPR | Arch_riscv64, "riscv64");
  DEF_REGISTER(    f27_32,  27 | FPR | Arch_riscv64, "riscv64");
  DEF_REGISTER(    f28_32,  28 | FPR | Arch_riscv64, "riscv64");
  DEF_REGISTER(    f29_32,  29 | FPR | Arch_riscv64, "riscv64");
  DEF_REGISTER(    f30_32,  30 | FPR | Arch_riscv64, "riscv64");
  DEF_REGISTER(    f31_32,  31 | FPR | Arch_riscv64, "riscv64");

  // 64 bit FPRs f<N>_64 (Mainly stone compatibility)
  DEF_REGISTER(     f0_64,   0 | FPR | Arch_riscv64, "riscv64");
  DEF_REGISTER(     f1_64,   1 | FPR | Arch_riscv64, "riscv64");
  DEF_REGISTER(     f2_64,   2 | FPR | Arch_riscv64, "riscv64");
  DEF_REGISTER(     f3_64,   3 | FPR | Arch_riscv64, "riscv64");
  DEF_REGISTER(     f4_64,   4 | FPR | Arch_riscv64, "riscv64");
  DEF_REGISTER(     f5_64,   5 | FPR | Arch_riscv64, "riscv64");
  DEF_REGISTER(     f6_64,   6 | FPR | Arch_riscv64, "riscv64");
  DEF_REGISTER(     f7_64,   7 | FPR | Arch_riscv64, "riscv64");
  DEF_REGISTER(     f8_64,   8 | FPR | Arch_riscv64, "riscv64");
  DEF_REGISTER(     f9_64,   9 | FPR | Arch_riscv64, "riscv64");
  DEF_REGISTER(    f10_64,  10 | FPR | Arch_riscv64, "riscv64");
  DEF_REGISTER(    f11_64,  11 | FPR | Arch_riscv64, "riscv64");
  DEF_REGISTER(    f12_64,  12 | FPR | Arch_riscv64, "riscv64");
  DEF_REGISTER(    f13_64,  13 | FPR | Arch_riscv64, "riscv64");
  DEF_REGISTER(    f14_64,  14 | FPR | Arch_riscv64, "riscv64");
  DEF_REGISTER(    f15_64,  15 | FPR | Arch_riscv64, "riscv64");
  DEF_REGISTER(    f16_64,  16 | FPR | Arch_riscv64, "riscv64");
  DEF_REGISTER(    f17_64,  17 | FPR | Arch_riscv64, "riscv64");
  DEF_REGISTER(    f18_64,  18 | FPR | Arch_riscv64, "riscv64");
  DEF_REGISTER(    f19_64,  19 | FPR | Arch_riscv64, "riscv64");
  DEF_REGISTER(    f20_64,  20 | FPR | Arch_riscv64, "riscv64");
  DEF_REGISTER(    f21_64,  21 | FPR | Arch_riscv64, "riscv64");
  DEF_REGISTER(    f22_64,  22 | FPR | Arch_riscv64, "riscv64");
  DEF_REGISTER(    f23_64,  23 | FPR | Arch_riscv64, "riscv64");
  DEF_REGISTER(    f24_64,  24 | FPR | Arch_riscv64, "riscv64");
  DEF_REGISTER(    f25_64,  25 | FPR | Arch_riscv64, "riscv64");
  DEF_REGISTER(    f26_64,  26 | FPR | Arch_riscv64, "riscv64");
  DEF_REGISTER(    f27_64,  27 | FPR | Arch_riscv64, "riscv64");
  DEF_REGISTER(    f28_64,  28 | FPR | Arch_riscv64, "riscv64");
  DEF_REGISTER(    f29_64,  29 | FPR | Arch_riscv64, "riscv64");
  DEF_REGISTER(    f30_64,  30 | FPR | Arch_riscv64, "riscv64");
  DEF_REGISTER(    f31_64,  31 | FPR | Arch_riscv64, "riscv64");

  // special purpose register
  DEF_REGISTER(        pc,   0 | SPR | Arch_riscv64, "riscv64");
  DEF_REGISTER(    fflags,   1 | SPR | Arch_riscv64, "riscv64");
  DEF_REGISTER(       frm,   2 | SPR | Arch_riscv64, "riscv64");
  DEF_REGISTER(      fcsr,   3 | SPR | Arch_riscv64, "riscv64");
  DEF_REGISTER(    vstart,   4 | SPR | Arch_riscv64, "riscv64");
  DEF_REGISTER(     vxsat,   5 | SPR | Arch_riscv64, "riscv64");
  DEF_REGISTER(      vxrm,   6 | SPR | Arch_riscv64, "riscv64");
  DEF_REGISTER(      vcsr,   7 | SPR | Arch_riscv64, "riscv64");
  DEF_REGISTER(      seed,   8 | SPR | Arch_riscv64, "riscv64");
  DEF_REGISTER(     cycle,   9 | SPR | Arch_riscv64, "riscv64");
  DEF_REGISTER(      time,  10 | SPR | Arch_riscv64, "riscv64");
  DEF_REGISTER(   instret,  11 | SPR | Arch_riscv64, "riscv64");
  DEF_REGISTER(        vl,  12 | SPR | Arch_riscv64, "riscv64");
  DEF_REGISTER(     vtype,  13 | SPR | Arch_riscv64, "riscv64");
  DEF_REGISTER(     vlenb,  14 | SPR | Arch_riscv64, "riscv64");
  DEF_REGISTER(      priv,  15 | SPR | Arch_riscv64, "riscv64");
} // end of riscv64 namespace
}

#endif
