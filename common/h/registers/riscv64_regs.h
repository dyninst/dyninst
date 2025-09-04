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

  //          (         name,  ID | cat |         arch,      arch)
  DEF_REGISTER(           x0,   0 | GPR | Arch_riscv64, "riscv64");
  DEF_REGISTER(           x1,   1 | GPR | Arch_riscv64, "riscv64");
  DEF_REGISTER(           x2,   2 | GPR | Arch_riscv64, "riscv64");
  DEF_REGISTER(           x3,   3 | GPR | Arch_riscv64, "riscv64");
  DEF_REGISTER(           x4,   4 | GPR | Arch_riscv64, "riscv64");
  DEF_REGISTER(           x5,   5 | GPR | Arch_riscv64, "riscv64");
  DEF_REGISTER(           x6,   6 | GPR | Arch_riscv64, "riscv64");
  DEF_REGISTER(           x7,   7 | GPR | Arch_riscv64, "riscv64");
  DEF_REGISTER(           x8,   8 | GPR | Arch_riscv64, "riscv64");
  DEF_REGISTER(           x9,   9 | GPR | Arch_riscv64, "riscv64");
  DEF_REGISTER(          x10,  10 | GPR | Arch_riscv64, "riscv64");
  DEF_REGISTER(          x11,  11 | GPR | Arch_riscv64, "riscv64");
  DEF_REGISTER(          x12,  12 | GPR | Arch_riscv64, "riscv64");
  DEF_REGISTER(          x13,  13 | GPR | Arch_riscv64, "riscv64");
  DEF_REGISTER(          x14,  14 | GPR | Arch_riscv64, "riscv64");
  DEF_REGISTER(          x15,  15 | GPR | Arch_riscv64, "riscv64");
  DEF_REGISTER(          x16,  16 | GPR | Arch_riscv64, "riscv64");
  DEF_REGISTER(          x17,  17 | GPR | Arch_riscv64, "riscv64");
  DEF_REGISTER(          x18,  18 | GPR | Arch_riscv64, "riscv64");
  DEF_REGISTER(          x19,  19 | GPR | Arch_riscv64, "riscv64");
  DEF_REGISTER(          x20,  20 | GPR | Arch_riscv64, "riscv64");
  DEF_REGISTER(          x21,  21 | GPR | Arch_riscv64, "riscv64");
  DEF_REGISTER(          x22,  22 | GPR | Arch_riscv64, "riscv64");
  DEF_REGISTER(          x23,  23 | GPR | Arch_riscv64, "riscv64");
  DEF_REGISTER(          x24,  24 | GPR | Arch_riscv64, "riscv64");
  DEF_REGISTER(          x25,  25 | GPR | Arch_riscv64, "riscv64");
  DEF_REGISTER(          x26,  26 | GPR | Arch_riscv64, "riscv64");
  DEF_REGISTER(          x27,  27 | GPR | Arch_riscv64, "riscv64");
  DEF_REGISTER(          x28,  28 | GPR | Arch_riscv64, "riscv64");
  DEF_REGISTER(          x29,  29 | GPR | Arch_riscv64, "riscv64");
  DEF_REGISTER(          x30,  30 | GPR | Arch_riscv64, "riscv64");
  DEF_REGISTER(          x31,  31 | GPR | Arch_riscv64, "riscv64");

  // F extension defines 32 bit floating point registers f0 ~ f31
  // D extension widens f0 ~ f31, to 64 bits

  // In riscv64, 32-bit FPRs are NaN-boxed to 64-bit FPRs.  In other words, in
  // RI64, when accessing 32-bit FPRs, you are actually accessing the entire
  // register.

  // Capstone distinguishes between 32-bit FPRs and 64-bit FPRs.  So far, other
  // libraries such as Dwarf does not distinguish between the two.  So here
  // f<N>_32 and f<N>_64 are used to handle Capstone registers On the other
  // hand, f<N> are the "actual" FPRs that include both the NaN-boxed f<N>_32
  // and f<N>_64.  f<N>_32 and f<N>_64 are both aliases of f<N>

  // Unless you are dealing with Capstone, you should always use f<N>.

  // The "actual" 64 bit FPRs, including NaN-boxed 32-bit and 64-bit FPRs
  DEF_REGISTER(           f0,   0 | FPR | Arch_riscv64, "riscv64");
  DEF_REGISTER(           f1,   1 | FPR | Arch_riscv64, "riscv64");
  DEF_REGISTER(           f2,   2 | FPR | Arch_riscv64, "riscv64");
  DEF_REGISTER(           f3,   3 | FPR | Arch_riscv64, "riscv64");
  DEF_REGISTER(           f4,   4 | FPR | Arch_riscv64, "riscv64");
  DEF_REGISTER(           f5,   5 | FPR | Arch_riscv64, "riscv64");
  DEF_REGISTER(           f6,   6 | FPR | Arch_riscv64, "riscv64");
  DEF_REGISTER(           f7,   7 | FPR | Arch_riscv64, "riscv64");
  DEF_REGISTER(           f8,   8 | FPR | Arch_riscv64, "riscv64");
  DEF_REGISTER(           f9,   9 | FPR | Arch_riscv64, "riscv64");
  DEF_REGISTER(          f10,  10 | FPR | Arch_riscv64, "riscv64");
  DEF_REGISTER(          f11,  11 | FPR | Arch_riscv64, "riscv64");
  DEF_REGISTER(          f12,  12 | FPR | Arch_riscv64, "riscv64");
  DEF_REGISTER(          f13,  13 | FPR | Arch_riscv64, "riscv64");
  DEF_REGISTER(          f14,  14 | FPR | Arch_riscv64, "riscv64");
  DEF_REGISTER(          f15,  15 | FPR | Arch_riscv64, "riscv64");
  DEF_REGISTER(          f16,  16 | FPR | Arch_riscv64, "riscv64");
  DEF_REGISTER(          f17,  17 | FPR | Arch_riscv64, "riscv64");
  DEF_REGISTER(          f18,  18 | FPR | Arch_riscv64, "riscv64");
  DEF_REGISTER(          f19,  19 | FPR | Arch_riscv64, "riscv64");
  DEF_REGISTER(          f20,  20 | FPR | Arch_riscv64, "riscv64");
  DEF_REGISTER(          f21,  21 | FPR | Arch_riscv64, "riscv64");
  DEF_REGISTER(          f22,  22 | FPR | Arch_riscv64, "riscv64");
  DEF_REGISTER(          f23,  23 | FPR | Arch_riscv64, "riscv64");
  DEF_REGISTER(          f24,  24 | FPR | Arch_riscv64, "riscv64");
  DEF_REGISTER(          f25,  25 | FPR | Arch_riscv64, "riscv64");
  DEF_REGISTER(          f26,  26 | FPR | Arch_riscv64, "riscv64");
  DEF_REGISTER(          f27,  27 | FPR | Arch_riscv64, "riscv64");
  DEF_REGISTER(          f28,  28 | FPR | Arch_riscv64, "riscv64");
  DEF_REGISTER(          f29,  29 | FPR | Arch_riscv64, "riscv64");
  DEF_REGISTER(          f30,  30 | FPR | Arch_riscv64, "riscv64");
  DEF_REGISTER(          f31,  31 | FPR | Arch_riscv64, "riscv64");

  // special purpose register
  DEF_REGISTER(           pc,   0 | SPR | Arch_riscv64, "riscv64");

  // Control Status Registers (CSRs) - the comment after each is its CSR Id
  // From The RISC-V Instruction Set Manual: Volume II Privileged Architecture,
  // Version 20250805, Section 2.2

  // Unprivileged Floating-Point CSRs
  DEF_REGISTER(       fflags,   0 | CSR | Arch_riscv64, "riscv64"); // 0x001
  DEF_REGISTER(          frm,   1 | CSR | Arch_riscv64, "riscv64"); // 0x002
  DEF_REGISTER(         fcsr,   2 | CSR | Arch_riscv64, "riscv64"); // 0x003

  // Unprivileged Vector CSRs, read/write
  DEF_REGISTER(       vstart,   3 | CSR | Arch_riscv64, "riscv64"); // 0x008
  DEF_REGISTER(        vxsat,   4 | CSR | Arch_riscv64, "riscv64"); // 0x009
  DEF_REGISTER(         vxrm,   5 | CSR | Arch_riscv64, "riscv64"); // 0x00A
  DEF_REGISTER(         vcsr,   6 | CSR | Arch_riscv64, "riscv64"); // 0x00F

  // Unprivileged Zicfiss extension CSR
  DEF_REGISTER(          ssp,   7 | CSR | Arch_riscv64, "riscv64"); // 0x011

  // Unprivileged Entropy Source Extension CSR
  DEF_REGISTER(         seed,   8 | CSR | Arch_riscv64, "riscv64"); // 0x015

  // Unprivileged Zcmt Extension CSR
  DEF_REGISTER(          jvt,   9 | CSR | Arch_riscv64, "riscv64"); // 0x017

  // Unprivileged Counter/Timers
  DEF_REGISTER(        cycle,  10 | CSR | Arch_riscv64, "riscv64"); // 0xC00
  DEF_REGISTER(         time,  11 | CSR | Arch_riscv64, "riscv64"); // 0xC01
  DEF_REGISTER(      instret,  12 | CSR | Arch_riscv64, "riscv64"); // 0xC02
  DEF_REGISTER(  hpmcounter3,  13 | CSR | Arch_riscv64, "riscv64"); // 0xC03
  DEF_REGISTER(  hpmcounter4,  14 | CSR | Arch_riscv64, "riscv64"); // 0xC04
  DEF_REGISTER(  hpmcounter5,  15 | CSR | Arch_riscv64, "riscv64"); // 0xC05
  DEF_REGISTER(  hpmcounter6,  16 | CSR | Arch_riscv64, "riscv64"); // 0xC06
  DEF_REGISTER(  hpmcounter7,  17 | CSR | Arch_riscv64, "riscv64"); // 0xC07
  DEF_REGISTER(  hpmcounter8,  18 | CSR | Arch_riscv64, "riscv64"); // 0xC08
  DEF_REGISTER(  hpmcounter9,  19 | CSR | Arch_riscv64, "riscv64"); // 0xC09
  DEF_REGISTER( hpmcounter10,  20 | CSR | Arch_riscv64, "riscv64"); // 0xC0A
  DEF_REGISTER( hpmcounter11,  21 | CSR | Arch_riscv64, "riscv64"); // 0xC0B
  DEF_REGISTER( hpmcounter12,  22 | CSR | Arch_riscv64, "riscv64"); // 0xC0C
  DEF_REGISTER( hpmcounter13,  23 | CSR | Arch_riscv64, "riscv64"); // 0xC0D
  DEF_REGISTER( hpmcounter14,  24 | CSR | Arch_riscv64, "riscv64"); // 0xC0E
  DEF_REGISTER( hpmcounter15,  25 | CSR | Arch_riscv64, "riscv64"); // 0xC0F
  DEF_REGISTER( hpmcounter16,  26 | CSR | Arch_riscv64, "riscv64"); // 0xC10
  DEF_REGISTER( hpmcounter17,  27 | CSR | Arch_riscv64, "riscv64"); // 0xC11
  DEF_REGISTER( hpmcounter18,  28 | CSR | Arch_riscv64, "riscv64"); // 0xC12
  DEF_REGISTER( hpmcounter19,  29 | CSR | Arch_riscv64, "riscv64"); // 0xC13
  DEF_REGISTER( hpmcounter20,  30 | CSR | Arch_riscv64, "riscv64"); // 0xC14
  DEF_REGISTER( hpmcounter21,  31 | CSR | Arch_riscv64, "riscv64"); // 0xC15
  DEF_REGISTER( hpmcounter22,  32 | CSR | Arch_riscv64, "riscv64"); // 0xC16
  DEF_REGISTER( hpmcounter23,  33 | CSR | Arch_riscv64, "riscv64"); // 0xC17
  DEF_REGISTER( hpmcounter24,  34 | CSR | Arch_riscv64, "riscv64"); // 0xC18
  DEF_REGISTER( hpmcounter25,  35 | CSR | Arch_riscv64, "riscv64"); // 0xC19
  DEF_REGISTER( hpmcounter26,  36 | CSR | Arch_riscv64, "riscv64"); // 0xC1A
  DEF_REGISTER( hpmcounter27,  37 | CSR | Arch_riscv64, "riscv64"); // 0xC1B
  DEF_REGISTER( hpmcounter28,  38 | CSR | Arch_riscv64, "riscv64"); // 0xC1C
  DEF_REGISTER( hpmcounter29,  39 | CSR | Arch_riscv64, "riscv64"); // 0xC1D
  DEF_REGISTER( hpmcounter30,  40 | CSR | Arch_riscv64, "riscv64"); // 0xC1E
  DEF_REGISTER( hpmcounter31,  41 | CSR | Arch_riscv64, "riscv64"); // 0xC1F

  // Unprivileged Vector CSRs, read-only
  DEF_REGISTER(           vl,  42 | CSR | Arch_riscv64, "riscv64"); // 0xC20
  DEF_REGISTER(        vtype,  43 | CSR | Arch_riscv64, "riscv64"); // 0xC21
  DEF_REGISTER(        vlenb,  44 | CSR | Arch_riscv64, "riscv64"); // 0xC22

  // Unprivileged Counter/Timers, RV32 only
  DEF_REGISTER(       cycleh,  45 | CSR | Arch_riscv64, "riscv64"); // 0xC80
  DEF_REGISTER(        timeh,  46 | CSR | Arch_riscv64, "riscv64"); // 0xC81
  DEF_REGISTER(     instreth,  47 | CSR | Arch_riscv64, "riscv64"); // 0xC82
  DEF_REGISTER( hpmcounter3h,  48 | CSR | Arch_riscv64, "riscv64"); // 0xC83
  DEF_REGISTER( hpmcounter4h,  49 | CSR | Arch_riscv64, "riscv64"); // 0xC84
  DEF_REGISTER( hpmcounter5h,  50 | CSR | Arch_riscv64, "riscv64"); // 0xC85
  DEF_REGISTER( hpmcounter6h,  51 | CSR | Arch_riscv64, "riscv64"); // 0xC86
  DEF_REGISTER( hpmcounter7h,  52 | CSR | Arch_riscv64, "riscv64"); // 0xC87
  DEF_REGISTER( hpmcounter8h,  53 | CSR | Arch_riscv64, "riscv64"); // 0xC88
  DEF_REGISTER( hpmcounter9h,  54 | CSR | Arch_riscv64, "riscv64"); // 0xC89
  DEF_REGISTER(hpmcounter10h,  55 | CSR | Arch_riscv64, "riscv64"); // 0xC8A
  DEF_REGISTER(hpmcounter11h,  56 | CSR | Arch_riscv64, "riscv64"); // 0xC8B
  DEF_REGISTER(hpmcounter12h,  57 | CSR | Arch_riscv64, "riscv64"); // 0xC8C
  DEF_REGISTER(hpmcounter13h,  58 | CSR | Arch_riscv64, "riscv64"); // 0xC8D
  DEF_REGISTER(hpmcounter14h,  59 | CSR | Arch_riscv64, "riscv64"); // 0xC8E
  DEF_REGISTER(hpmcounter15h,  60 | CSR | Arch_riscv64, "riscv64"); // 0xC8F
  DEF_REGISTER(hpmcounter16h,  61 | CSR | Arch_riscv64, "riscv64"); // 0xC90
  DEF_REGISTER(hpmcounter17h,  62 | CSR | Arch_riscv64, "riscv64"); // 0xC91
  DEF_REGISTER(hpmcounter18h,  63 | CSR | Arch_riscv64, "riscv64"); // 0xC92
  DEF_REGISTER(hpmcounter19h,  64 | CSR | Arch_riscv64, "riscv64"); // 0xC93
  DEF_REGISTER(hpmcounter20h,  65 | CSR | Arch_riscv64, "riscv64"); // 0xC94
  DEF_REGISTER(hpmcounter21h,  66 | CSR | Arch_riscv64, "riscv64"); // 0xC95
  DEF_REGISTER(hpmcounter22h,  67 | CSR | Arch_riscv64, "riscv64"); // 0xC96
  DEF_REGISTER(hpmcounter23h,  68 | CSR | Arch_riscv64, "riscv64"); // 0xC97
  DEF_REGISTER(hpmcounter24h,  69 | CSR | Arch_riscv64, "riscv64"); // 0xC98
  DEF_REGISTER(hpmcounter25h,  70 | CSR | Arch_riscv64, "riscv64"); // 0xC99
  DEF_REGISTER(hpmcounter26h,  71 | CSR | Arch_riscv64, "riscv64"); // 0xC9A
  DEF_REGISTER(hpmcounter27h,  72 | CSR | Arch_riscv64, "riscv64"); // 0xC9B
  DEF_REGISTER(hpmcounter28h,  73 | CSR | Arch_riscv64, "riscv64"); // 0xC9C
  DEF_REGISTER(hpmcounter29h,  74 | CSR | Arch_riscv64, "riscv64"); // 0xC9D
  DEF_REGISTER(hpmcounter30h,  75 | CSR | Arch_riscv64, "riscv64"); // 0xC9E
  DEF_REGISTER(hpmcounter31h,  76 | CSR | Arch_riscv64, "riscv64"); // 0xC9F

  // Register aliases
  DEF_REGISTER_ALIAS(  zero,  x0, "riscv64");
  DEF_REGISTER_ALIAS(    ra,  x1, "riscv64");
  DEF_REGISTER_ALIAS(    sp,  x2, "riscv64");
  DEF_REGISTER_ALIAS(    gp,  x3, "riscv64");
  DEF_REGISTER_ALIAS(    tp,  x4, "riscv64");
  DEF_REGISTER_ALIAS(    t0,  x5, "riscv64");
  DEF_REGISTER_ALIAS(    t1,  x6, "riscv64");
  DEF_REGISTER_ALIAS(    t2,  x7, "riscv64");
  DEF_REGISTER_ALIAS(    fp,  x8, "riscv64");
  DEF_REGISTER_ALIAS(    s0,  x8, "riscv64");
  DEF_REGISTER_ALIAS(    s1,  x9, "riscv64");
  DEF_REGISTER_ALIAS(    a0, x10, "riscv64");
  DEF_REGISTER_ALIAS(    a1, x11, "riscv64");
  DEF_REGISTER_ALIAS(    a2, x12, "riscv64");
  DEF_REGISTER_ALIAS(    a3, x13, "riscv64");
  DEF_REGISTER_ALIAS(    a4, x14, "riscv64");
  DEF_REGISTER_ALIAS(    a5, x15, "riscv64");
  DEF_REGISTER_ALIAS(    a6, x16, "riscv64");
  DEF_REGISTER_ALIAS(    a7, x17, "riscv64");
  DEF_REGISTER_ALIAS(    s2, x18, "riscv64");
  DEF_REGISTER_ALIAS(    s3, x19, "riscv64");
  DEF_REGISTER_ALIAS(    s4, x20, "riscv64");
  DEF_REGISTER_ALIAS(    s5, x21, "riscv64");
  DEF_REGISTER_ALIAS(    s6, x22, "riscv64");
  DEF_REGISTER_ALIAS(    s7, x23, "riscv64");
  DEF_REGISTER_ALIAS(    s8, x24, "riscv64");
  DEF_REGISTER_ALIAS(    s9, x25, "riscv64");
  DEF_REGISTER_ALIAS(   s10, x26, "riscv64");
  DEF_REGISTER_ALIAS(   s11, x27, "riscv64");
  DEF_REGISTER_ALIAS(    t3, x28, "riscv64");
  DEF_REGISTER_ALIAS(    t4, x29, "riscv64");
  DEF_REGISTER_ALIAS(    t5, x30, "riscv64");
  DEF_REGISTER_ALIAS(    t6, x31, "riscv64");

  // Floating point register aliases
  DEF_REGISTER_ALIAS(   ft0,  f0, "riscv64");
  DEF_REGISTER_ALIAS(   ft1,  f1, "riscv64");
  DEF_REGISTER_ALIAS(   ft2,  f2, "riscv64");
  DEF_REGISTER_ALIAS(   ft3,  f3, "riscv64");
  DEF_REGISTER_ALIAS(   ft4,  f4, "riscv64");
  DEF_REGISTER_ALIAS(   ft5,  f5, "riscv64");
  DEF_REGISTER_ALIAS(   ft6,  f6, "riscv64");
  DEF_REGISTER_ALIAS(   ft7,  f7, "riscv64");
  DEF_REGISTER_ALIAS(   fs0,  f8, "riscv64");
  DEF_REGISTER_ALIAS(   fs1,  f9, "riscv64");
  DEF_REGISTER_ALIAS(   fa0, f10, "riscv64");
  DEF_REGISTER_ALIAS(   fa1, f11, "riscv64");
  DEF_REGISTER_ALIAS(   fa2, f12, "riscv64");
  DEF_REGISTER_ALIAS(   fa3, f13, "riscv64");
  DEF_REGISTER_ALIAS(   fa4, f14, "riscv64");
  DEF_REGISTER_ALIAS(   fa5, f15, "riscv64");
  DEF_REGISTER_ALIAS(   fa6, f16, "riscv64");
  DEF_REGISTER_ALIAS(   fa7, f17, "riscv64");
  DEF_REGISTER_ALIAS(   fs2, f18, "riscv64");
  DEF_REGISTER_ALIAS(   fs3, f19, "riscv64");
  DEF_REGISTER_ALIAS(   fs4, f20, "riscv64");
  DEF_REGISTER_ALIAS(   fs5, f21, "riscv64");
  DEF_REGISTER_ALIAS(   fs6, f22, "riscv64");
  DEF_REGISTER_ALIAS(   fs7, f23, "riscv64");
  DEF_REGISTER_ALIAS(   fs8, f24, "riscv64");
  DEF_REGISTER_ALIAS(   fs9, f25, "riscv64");
  DEF_REGISTER_ALIAS(  fs10, f26, "riscv64");
  DEF_REGISTER_ALIAS(  fs11, f27, "riscv64");
  DEF_REGISTER_ALIAS(   ft8, f28, "riscv64");
  DEF_REGISTER_ALIAS(   ft9, f29, "riscv64");
  DEF_REGISTER_ALIAS(  ft10, f30, "riscv64");
  DEF_REGISTER_ALIAS(  ft11, f31, "riscv64");

  // 32 bit FPRs f<N>_32 (Mainly for Capstone compatibility)
  DEF_REGISTER_ALIAS( f0_32,  f0, "riscv64");
  DEF_REGISTER_ALIAS( f1_32,  f1, "riscv64");
  DEF_REGISTER_ALIAS( f2_32,  f2, "riscv64");
  DEF_REGISTER_ALIAS( f3_32,  f3, "riscv64");
  DEF_REGISTER_ALIAS( f4_32,  f4, "riscv64");
  DEF_REGISTER_ALIAS( f5_32,  f5, "riscv64");
  DEF_REGISTER_ALIAS( f6_32,  f6, "riscv64");
  DEF_REGISTER_ALIAS( f7_32,  f7, "riscv64");
  DEF_REGISTER_ALIAS( f8_32,  f8, "riscv64");
  DEF_REGISTER_ALIAS( f9_32,  f9, "riscv64");
  DEF_REGISTER_ALIAS(f10_32, f10, "riscv64");
  DEF_REGISTER_ALIAS(f11_32, f11, "riscv64");
  DEF_REGISTER_ALIAS(f12_32, f12, "riscv64");
  DEF_REGISTER_ALIAS(f13_32, f13, "riscv64");
  DEF_REGISTER_ALIAS(f14_32, f14, "riscv64");
  DEF_REGISTER_ALIAS(f15_32, f15, "riscv64");
  DEF_REGISTER_ALIAS(f16_32, f16, "riscv64");
  DEF_REGISTER_ALIAS(f17_32, f17, "riscv64");
  DEF_REGISTER_ALIAS(f18_32, f18, "riscv64");
  DEF_REGISTER_ALIAS(f19_32, f19, "riscv64");
  DEF_REGISTER_ALIAS(f20_32, f20, "riscv64");
  DEF_REGISTER_ALIAS(f21_32, f21, "riscv64");
  DEF_REGISTER_ALIAS(f22_32, f22, "riscv64");
  DEF_REGISTER_ALIAS(f23_32, f23, "riscv64");
  DEF_REGISTER_ALIAS(f24_32, f24, "riscv64");
  DEF_REGISTER_ALIAS(f25_32, f25, "riscv64");
  DEF_REGISTER_ALIAS(f26_32, f26, "riscv64");
  DEF_REGISTER_ALIAS(f27_32, f27, "riscv64");
  DEF_REGISTER_ALIAS(f28_32, f28, "riscv64");
  DEF_REGISTER_ALIAS(f29_32, f29, "riscv64");
  DEF_REGISTER_ALIAS(f30_32, f30, "riscv64");
  DEF_REGISTER_ALIAS(f31_32, f31, "riscv64");

  // 64 bit FPRs f<N>_64 (Mainly for Capstone compatibility)
  DEF_REGISTER_ALIAS( f0_64,  f0, "riscv64");
  DEF_REGISTER_ALIAS( f1_64,  f1, "riscv64");
  DEF_REGISTER_ALIAS( f2_64,  f2, "riscv64");
  DEF_REGISTER_ALIAS( f3_64,  f3, "riscv64");
  DEF_REGISTER_ALIAS( f4_64,  f4, "riscv64");
  DEF_REGISTER_ALIAS( f5_64,  f5, "riscv64");
  DEF_REGISTER_ALIAS( f6_64,  f6, "riscv64");
  DEF_REGISTER_ALIAS( f7_64,  f7, "riscv64");
  DEF_REGISTER_ALIAS( f8_64,  f8, "riscv64");
  DEF_REGISTER_ALIAS( f9_64,  f9, "riscv64");
  DEF_REGISTER_ALIAS(f10_64, f10, "riscv64");
  DEF_REGISTER_ALIAS(f11_64, f11, "riscv64");
  DEF_REGISTER_ALIAS(f12_64, f12, "riscv64");
  DEF_REGISTER_ALIAS(f13_64, f13, "riscv64");
  DEF_REGISTER_ALIAS(f14_64, f14, "riscv64");
  DEF_REGISTER_ALIAS(f15_64, f15, "riscv64");
  DEF_REGISTER_ALIAS(f16_64, f16, "riscv64");
  DEF_REGISTER_ALIAS(f17_64, f17, "riscv64");
  DEF_REGISTER_ALIAS(f18_64, f18, "riscv64");
  DEF_REGISTER_ALIAS(f19_64, f19, "riscv64");
  DEF_REGISTER_ALIAS(f20_64, f20, "riscv64");
  DEF_REGISTER_ALIAS(f21_64, f21, "riscv64");
  DEF_REGISTER_ALIAS(f22_64, f22, "riscv64");
  DEF_REGISTER_ALIAS(f23_64, f23, "riscv64");
  DEF_REGISTER_ALIAS(f24_64, f24, "riscv64");
  DEF_REGISTER_ALIAS(f25_64, f25, "riscv64");
  DEF_REGISTER_ALIAS(f26_64, f26, "riscv64");
  DEF_REGISTER_ALIAS(f27_64, f27, "riscv64");
  DEF_REGISTER_ALIAS(f28_64, f28, "riscv64");
  DEF_REGISTER_ALIAS(f29_64, f29, "riscv64");
  DEF_REGISTER_ALIAS(f30_64, f30, "riscv64");
  DEF_REGISTER_ALIAS(f31_64, f31, "riscv64");
} // end of riscv64 namespace
}

#endif
