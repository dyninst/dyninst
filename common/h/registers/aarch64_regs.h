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

#ifndef DYNINST_AARCH64_REGS_H
#define DYNINST_AARCH64_REGS_H

//clang-format: off

#include "Architecture.h"
#include "registers/reg_def.h"
#include <cstdint>

namespace Dyninst { namespace aarch64 {

  /* Register lengths
   *
   * NOTE:
   *
   *   MachRegister::getBaseRegister clears the bit field for size, so
   *   the full register size (FULL) has to be represented as 0x0.
   *
   *  References:
   *
   *    [ARMA]
   *    Arm Architecture Reference Manual for A-profile architecture
   *    2023
   *    B1.2 Registers in AArch64 Execution state
   *
   *    [ARMv9S]
   *    Arm Architecture Reference Manual Supplement
   *    The Scalable Matrix Extension (SME) for Armv9-A
   *    7th February 2022
   *    B2.1 Architectural state summary
   *
   *  Notes:
   *
   *    ARMA - A1.4 Supported data types
   *    --------------------------------
   *    A Scalable Vector Extension (SVE) register has an IMPLEMENTATION DEFINED width that
   *    is a power of two, from a minimum of 128 bits up to a maximum of 2048 bits. All SVE
   *    scalable vector registers in an implementation are the same width. We assume they
   *    are 2048 bits.
   *
   *    An SVE predicate register has an IMPLEMENTATION DEFINED width that is a power of two,
   *    from a minimum of 16 bits up to a maximum of 256 bits (i.e., length of an SVE vector
   *    divided by 8). We assume they are 256 bits.
   *
   *    ARMv9S - B2.1 Architectural state summary
   *    -----------------------------------------
   *    The Scalable Matrix Extension (SME) Effective Streaming SVE vector length, SVL, is a
   *    power of two in the range 128 to 2048 bits inclusive. When the processor is in
   *    Streaming SVE mode, the Effective SVE vector length, VL, is equal to SVL. This might
   *    be different from the value of VL when the PE is not in Streaming SVE mode. See
   *    'C2.1.3 Vector lengths' for details. We assume SME registers are always 2048 bits.
   *
   *    ARMv9S - B2.2 SME ZA storage
   *    -----------------------------------------
   *    The ZA storage is architectural register state consisting of a two-dimensional ZA
   *    array of [SVLB × SVLB ] bytes. SLVB is the number of 8-bit elements in an SVE vector
   *    of length SVL.
   *
   **/
  const int32_t FULL   = 0x00000000;  // 64-bit double-word
  const int32_t D_REG  = 0x00000100;  // 32-bit single-word
  const int32_t W_REG  = 0x00000200;  // 16-bit half-word
  const int32_t B_REG  = 0x00000300;  // 8-bit byte
  const int32_t BIT    = 0x00000400;  // 1 bit
  const int32_t Q_REG  = 0x00000500;  // 128-bit vector
  const int32_t SVES   = 0x00000600;  // 2048-bit Scalable Vector Extension (SVE) vector length
  const int32_t PREDS  = 0x00000700;  // SVE predicate register
  const int32_t SVE2S  = 0x00000800;  // 512-bit Scalable Vector Extension
  const int32_t SVLS   = 0x00000900;  // 2048-bit SME Effective Streaming SVE vector length (SVL)
  const int32_t SMEZAS = 0x00000A00;  // Scalable Matrix Extension ZA array

  /* Base Register Categories */
  const int32_t GPR  = 0x00000000;  // General-purpose
  const int32_t FPR  = 0x00010000;  // Floating-point
  const int32_t SPR  = 0x00020000;  // Special-purpose
  const int32_t FLAG = 0x00030000;  // Control/Status flag
  const int32_t SVE  = 0x00040000;  // Scalable Vector Extension
  const int32_t SVE2 = 0x00050000;  // Scalable Vector Extension, version 2
  const int32_t SME  = 0x00060000;  // Scalable Matrix Extension

  /* System Register Categories
  *
  *  There are so many system registers, it's necessary to break them into many
  *  sub-categories. This makes it difficult to detect them because they don't have a
  *  single category value. To fix this, the uppermost bit of the category field is always
  *  set to 1 for system registers (SYSREG value).
  *
  */
  const int32_t SYSREG    =          0x00800000;  // Base mask
  const int32_t AD        = SYSREG | 0x00000000;  // accelerator data
  const int32_t ADDR      = SYSREG | 0x00010000;  // address
  const int32_t AMR       = SYSREG | 0x00020000;  // activity monitors
  const int32_t BAWS      = SYSREG | 0x00030000;  // breakpoint and watchpoint selection
  const int32_t BRBE      = SYSREG | 0x00040000;  // branch record buffer extension
  const int32_t DBG       = SYSREG | 0x00050000;  // debug
  const int32_t DBGAUTH   = SYSREG | 0x00060000;  // debug authentication
  const int32_t DBGBRK    = SYSREG | 0x00070000;  // debug breakpoint management
  const int32_t DBGCT     = SYSREG | 0x00080000;  // debug claim tag
  const int32_t DBGDTR    = SYSREG | 0x00090000;  // debug data transfer
  const int32_t DBGLR     = SYSREG | 0x000A0000;  // debug link register
  const int32_t DBGPCR    = SYSREG | 0x000B0000;  // debug power control
  const int32_t DBGSPSR   = SYSREG | 0x000C0000;  // debug saved program status
  const int32_t DBGW      = SYSREG | 0x000D0000;  // debug watchpoint
  const int32_t DVCR      = SYSREG | 0x000E0000;  // debug vector catch
  const int32_t EXCP      = SYSREG | 0x000F0000;  // exception
  const int32_t FWTE      = SYSREG | 0x00100000;  // fine-grained write traps el3
  const int32_t GCSR      = SYSREG | 0x00110000;  // guarded control stack registers
  const int32_t HYPRDBG   = SYSREG | 0x00120000;  // hypervisor debug fine-grained
  const int32_t IMPLDEF   = SYSREG | 0x00130000;  // implementation defined
  const int32_t MPAM      = SYSREG | 0x00140000;  // memory partitioning and monitoring extension
  const int32_t OTHER     = SYSREG | 0x00150000;  // other system control
  const int32_t PHYSFAR   = SYSREG | 0x00160000;  // physical fault address
  const int32_t PMU       = SYSREG | 0x00170000;  // performance monitors extension
  const int32_t PSTATE    = SYSREG | 0x00180000;  // process state
  const int32_t RAS       = SYSREG | 0x00190000;  // reliability, availability, and serviceability extension (RAS)
  const int32_t RESET     = SYSREG | 0x001B0000;  // reset management
  const int32_t SEC       = SYSREG | 0x001C0000;  // security for access to exception levels
  const int32_t STATPROF  = SYSREG | 0x001D0000;  // statistical profiling extension
  const int32_t SYSCTL    = SYSREG | 0x001E0000;  // system control
  const int32_t SYSFLOAT  = SYSREG | 0x001F0000;  // system floating-point
  const int32_t SYSID     = SYSREG | 0x00200000;  // system identification
  const int32_t SYSMEMORY = SYSREG | 0x00210000;  // system memory
  const int32_t SYSMON    = SYSREG | 0x00220000;  // system monitor
  const int32_t SYSOS     = SYSREG | 0x00230000;  // system OS lock/access/data/control
  const int32_t SYSPTR    = SYSREG | 0x00240000;  // pointer authentication
  const int32_t SYSSPR    = SYSREG | 0x00250000;  // system special-purpose
  const int32_t SYSTIMER  = SYSREG | 0x00260000;  // system timers
  const int32_t THRD      = SYSREG | 0x00270000;  // threading
  const int32_t TRACE     = SYSREG | 0x00280000;  // system trace
  const int32_t VIRT      = SYSREG | 0x00290000;  // virtualization


  /**
   * Format of constants:
   *  [0x000000ff] Lower 8 bits are base register ID
   *  [0x0000ff00] Next 8 bits are the aliasing and subrange ID used to distinguish
   *               between whole and aliased registers like w1 and x1.
   *  [0x00ff0000] Next 8 bits are the register category, GPR, FLAG, etc.
   *  [0xff000000] Upper 8 bits are the architecture.
   **/

  //          (                name,  ID |  alias |        cat |         arch)
  const int32_t HQ_REG = 0x0000FF00;
  const int32_t TLBI   = SYSREG | 0x00EF0000;
  const int32_t MNEMONICS = SYSREG | 0x00DF0000;

  // Single source of truth for this arch's registers: (name, encoding).
  // Expanded to the constexpr constants AND all_regs[] -- no duplication.
  #define DYNINST_AARCH64_REG_LIST(X, A) \
      X(nzcv, 0 |   FULL |       FLAG | Arch_aarch64) \
      X(n, 1 |    BIT |       FLAG | Arch_aarch64) \
      X(z, 2 |    BIT |       FLAG | Arch_aarch64) \
      X(c, 3 |    BIT |       FLAG | Arch_aarch64) \
      X(v, 4 |    BIT |       FLAG | Arch_aarch64) \
      X(daif, 5 |   FULL |       FLAG | Arch_aarch64) \
      X(d, 6 |    BIT |       FLAG | Arch_aarch64) \
      X(a, 7 |    BIT |       FLAG | Arch_aarch64) \
      X(i, 8 |    BIT |       FLAG | Arch_aarch64) \
      X(f_, 9 |    BIT |       FLAG | Arch_aarch64) \
      X(fpcr, 10 |   FULL |       FLAG | Arch_aarch64) \
      X(pstate, 0 |   FULL |     PSTATE | Arch_aarch64) \
      X(allint, 1 |    BIT |     PSTATE | Arch_aarch64) \
      X(currentel, 2 |    BIT |     PSTATE | Arch_aarch64) \
      X(dit, 3 |    BIT |     PSTATE | Arch_aarch64) \
      X(pan, 4 |    BIT |     PSTATE | Arch_aarch64) \
      X(pm, 5 |    BIT |     PSTATE | Arch_aarch64) \
      X(spsel, 6 |    BIT |     PSTATE | Arch_aarch64) \
      X(ssbs, 7 |    BIT |     PSTATE | Arch_aarch64) \
      X(svcr, 8 |    BIT |     PSTATE | Arch_aarch64) \
      X(tco, 9 |    BIT |     PSTATE | Arch_aarch64) \
      X(uao, 10 |    BIT |     PSTATE | Arch_aarch64) \
      X(pc, 2 |   FULL |        SPR | Arch_aarch64) \
      X(sp, 3 |   FULL |        SPR | Arch_aarch64) \
      X(wsp, 4 |  D_REG |        SPR | Arch_aarch64) \
      X(wzr, 5 |  D_REG |        SPR | Arch_aarch64) \
      X(xzr, 6 |   FULL |        SPR | Arch_aarch64) \
      X(w0, 0 |  D_REG |        GPR | Arch_aarch64) \
      X(w1, 1 |  D_REG |        GPR | Arch_aarch64) \
      X(w2, 2 |  D_REG |        GPR | Arch_aarch64) \
      X(w3, 3 |  D_REG |        GPR | Arch_aarch64) \
      X(w4, 4 |  D_REG |        GPR | Arch_aarch64) \
      X(w5, 5 |  D_REG |        GPR | Arch_aarch64) \
      X(w6, 6 |  D_REG |        GPR | Arch_aarch64) \
      X(w7, 7 |  D_REG |        GPR | Arch_aarch64) \
      X(w8, 8 |  D_REG |        GPR | Arch_aarch64) \
      X(w9, 9 |  D_REG |        GPR | Arch_aarch64) \
      X(w10, 10 |  D_REG |        GPR | Arch_aarch64) \
      X(w11, 11 |  D_REG |        GPR | Arch_aarch64) \
      X(w12, 12 |  D_REG |        GPR | Arch_aarch64) \
      X(w13, 13 |  D_REG |        GPR | Arch_aarch64) \
      X(w14, 14 |  D_REG |        GPR | Arch_aarch64) \
      X(w15, 15 |  D_REG |        GPR | Arch_aarch64) \
      X(w16, 16 |  D_REG |        GPR | Arch_aarch64) \
      X(w17, 17 |  D_REG |        GPR | Arch_aarch64) \
      X(w18, 18 |  D_REG |        GPR | Arch_aarch64) \
      X(w19, 19 |  D_REG |        GPR | Arch_aarch64) \
      X(w20, 20 |  D_REG |        GPR | Arch_aarch64) \
      X(w21, 21 |  D_REG |        GPR | Arch_aarch64) \
      X(w22, 22 |  D_REG |        GPR | Arch_aarch64) \
      X(w23, 23 |  D_REG |        GPR | Arch_aarch64) \
      X(w24, 24 |  D_REG |        GPR | Arch_aarch64) \
      X(w25, 25 |  D_REG |        GPR | Arch_aarch64) \
      X(w26, 26 |  D_REG |        GPR | Arch_aarch64) \
      X(w27, 27 |  D_REG |        GPR | Arch_aarch64) \
      X(w28, 28 |  D_REG |        GPR | Arch_aarch64) \
      X(w29, 29 |  D_REG |        GPR | Arch_aarch64) \
      X(w30, 30 |  D_REG |        GPR | Arch_aarch64) \
      X(x0, 31 |   FULL |        GPR | Arch_aarch64) \
      X(x1, 32 |   FULL |        GPR | Arch_aarch64) \
      X(x2, 33 |   FULL |        GPR | Arch_aarch64) \
      X(x3, 34 |   FULL |        GPR | Arch_aarch64) \
      X(x4, 35 |   FULL |        GPR | Arch_aarch64) \
      X(x5, 36 |   FULL |        GPR | Arch_aarch64) \
      X(x6, 37 |   FULL |        GPR | Arch_aarch64) \
      X(x7, 38 |   FULL |        GPR | Arch_aarch64) \
      X(x8, 39 |   FULL |        GPR | Arch_aarch64) \
      X(x9, 40 |   FULL |        GPR | Arch_aarch64) \
      X(x10, 41 |   FULL |        GPR | Arch_aarch64) \
      X(x11, 42 |   FULL |        GPR | Arch_aarch64) \
      X(x12, 43 |   FULL |        GPR | Arch_aarch64) \
      X(x13, 44 |   FULL |        GPR | Arch_aarch64) \
      X(x14, 45 |   FULL |        GPR | Arch_aarch64) \
      X(x15, 46 |   FULL |        GPR | Arch_aarch64) \
      X(x16, 47 |   FULL |        GPR | Arch_aarch64) \
      X(x17, 48 |   FULL |        GPR | Arch_aarch64) \
      X(x18, 49 |   FULL |        GPR | Arch_aarch64) \
      X(x19, 50 |   FULL |        GPR | Arch_aarch64) \
      X(x20, 51 |   FULL |        GPR | Arch_aarch64) \
      X(x21, 52 |   FULL |        GPR | Arch_aarch64) \
      X(x22, 53 |   FULL |        GPR | Arch_aarch64) \
      X(x23, 54 |   FULL |        GPR | Arch_aarch64) \
      X(x24, 55 |   FULL |        GPR | Arch_aarch64) \
      X(x25, 56 |   FULL |        GPR | Arch_aarch64) \
      X(x26, 57 |   FULL |        GPR | Arch_aarch64) \
      X(x27, 58 |   FULL |        GPR | Arch_aarch64) \
      X(x28, 59 |   FULL |        GPR | Arch_aarch64) \
      X(x29, 60 |   FULL |        GPR | Arch_aarch64) \
      X(x30, 61 |   FULL |        GPR | Arch_aarch64) \
      X(b0, 0 |  B_REG |        FPR | Arch_aarch64) \
      X(b1, 1 |  B_REG |        FPR | Arch_aarch64) \
      X(b2, 2 |  B_REG |        FPR | Arch_aarch64) \
      X(b3, 3 |  B_REG |        FPR | Arch_aarch64) \
      X(b4, 4 |  B_REG |        FPR | Arch_aarch64) \
      X(b5, 5 |  B_REG |        FPR | Arch_aarch64) \
      X(b6, 6 |  B_REG |        FPR | Arch_aarch64) \
      X(b7, 7 |  B_REG |        FPR | Arch_aarch64) \
      X(b8, 8 |  B_REG |        FPR | Arch_aarch64) \
      X(b9, 9 |  B_REG |        FPR | Arch_aarch64) \
      X(b10, 10 |  B_REG |        FPR | Arch_aarch64) \
      X(b11, 11 |  B_REG |        FPR | Arch_aarch64) \
      X(b12, 12 |  B_REG |        FPR | Arch_aarch64) \
      X(b13, 13 |  B_REG |        FPR | Arch_aarch64) \
      X(b14, 14 |  B_REG |        FPR | Arch_aarch64) \
      X(b15, 15 |  B_REG |        FPR | Arch_aarch64) \
      X(b16, 16 |  B_REG |        FPR | Arch_aarch64) \
      X(b17, 17 |  B_REG |        FPR | Arch_aarch64) \
      X(b18, 18 |  B_REG |        FPR | Arch_aarch64) \
      X(b19, 19 |  B_REG |        FPR | Arch_aarch64) \
      X(b20, 20 |  B_REG |        FPR | Arch_aarch64) \
      X(b21, 21 |  B_REG |        FPR | Arch_aarch64) \
      X(b22, 22 |  B_REG |        FPR | Arch_aarch64) \
      X(b23, 23 |  B_REG |        FPR | Arch_aarch64) \
      X(b24, 24 |  B_REG |        FPR | Arch_aarch64) \
      X(b25, 25 |  B_REG |        FPR | Arch_aarch64) \
      X(b26, 26 |  B_REG |        FPR | Arch_aarch64) \
      X(b27, 27 |  B_REG |        FPR | Arch_aarch64) \
      X(b28, 28 |  B_REG |        FPR | Arch_aarch64) \
      X(b29, 29 |  B_REG |        FPR | Arch_aarch64) \
      X(b30, 30 |  B_REG |        FPR | Arch_aarch64) \
      X(b31, 31 |  B_REG |        FPR | Arch_aarch64) \
      X(d0, 32 |   FULL |        FPR | Arch_aarch64) \
      X(d1, 33 |   FULL |        FPR | Arch_aarch64) \
      X(d2, 34 |   FULL |        FPR | Arch_aarch64) \
      X(d3, 35 |   FULL |        FPR | Arch_aarch64) \
      X(d4, 36 |   FULL |        FPR | Arch_aarch64) \
      X(d5, 37 |   FULL |        FPR | Arch_aarch64) \
      X(d6, 38 |   FULL |        FPR | Arch_aarch64) \
      X(d7, 39 |   FULL |        FPR | Arch_aarch64) \
      X(d8, 40 |   FULL |        FPR | Arch_aarch64) \
      X(d9, 41 |   FULL |        FPR | Arch_aarch64) \
      X(d10, 42 |   FULL |        FPR | Arch_aarch64) \
      X(d11, 43 |   FULL |        FPR | Arch_aarch64) \
      X(d12, 44 |   FULL |        FPR | Arch_aarch64) \
      X(d13, 45 |   FULL |        FPR | Arch_aarch64) \
      X(d14, 46 |   FULL |        FPR | Arch_aarch64) \
      X(d15, 47 |   FULL |        FPR | Arch_aarch64) \
      X(d16, 48 |   FULL |        FPR | Arch_aarch64) \
      X(d17, 49 |   FULL |        FPR | Arch_aarch64) \
      X(d18, 50 |   FULL |        FPR | Arch_aarch64) \
      X(d19, 51 |   FULL |        FPR | Arch_aarch64) \
      X(d20, 52 |   FULL |        FPR | Arch_aarch64) \
      X(d21, 53 |   FULL |        FPR | Arch_aarch64) \
      X(d22, 54 |   FULL |        FPR | Arch_aarch64) \
      X(d23, 55 |   FULL |        FPR | Arch_aarch64) \
      X(d24, 56 |   FULL |        FPR | Arch_aarch64) \
      X(d25, 57 |   FULL |        FPR | Arch_aarch64) \
      X(d26, 58 |   FULL |        FPR | Arch_aarch64) \
      X(d27, 59 |   FULL |        FPR | Arch_aarch64) \
      X(d28, 60 |   FULL |        FPR | Arch_aarch64) \
      X(d29, 61 |   FULL |        FPR | Arch_aarch64) \
      X(d30, 62 |   FULL |        FPR | Arch_aarch64) \
      X(d31, 63 |   FULL |        FPR | Arch_aarch64) \
      X(h0, 64 |  W_REG |        FPR | Arch_aarch64) \
      X(h1, 65 |  W_REG |        FPR | Arch_aarch64) \
      X(h2, 66 |  W_REG |        FPR | Arch_aarch64) \
      X(h3, 67 |  W_REG |        FPR | Arch_aarch64) \
      X(h4, 68 |  W_REG |        FPR | Arch_aarch64) \
      X(h5, 69 |  W_REG |        FPR | Arch_aarch64) \
      X(h6, 70 |  W_REG |        FPR | Arch_aarch64) \
      X(h7, 71 |  W_REG |        FPR | Arch_aarch64) \
      X(h8, 72 |  W_REG |        FPR | Arch_aarch64) \
      X(h9, 73 |  W_REG |        FPR | Arch_aarch64) \
      X(h10, 74 |  W_REG |        FPR | Arch_aarch64) \
      X(h11, 75 |  W_REG |        FPR | Arch_aarch64) \
      X(h12, 76 |  W_REG |        FPR | Arch_aarch64) \
      X(h13, 77 |  W_REG |        FPR | Arch_aarch64) \
      X(h14, 78 |  W_REG |        FPR | Arch_aarch64) \
      X(h15, 79 |  W_REG |        FPR | Arch_aarch64) \
      X(h16, 80 |  W_REG |        FPR | Arch_aarch64) \
      X(h17, 81 |  W_REG |        FPR | Arch_aarch64) \
      X(h18, 82 |  W_REG |        FPR | Arch_aarch64) \
      X(h19, 83 |  W_REG |        FPR | Arch_aarch64) \
      X(h20, 84 |  W_REG |        FPR | Arch_aarch64) \
      X(h21, 85 |  W_REG |        FPR | Arch_aarch64) \
      X(h22, 86 |  W_REG |        FPR | Arch_aarch64) \
      X(h23, 87 |  W_REG |        FPR | Arch_aarch64) \
      X(h24, 88 |  W_REG |        FPR | Arch_aarch64) \
      X(h25, 89 |  W_REG |        FPR | Arch_aarch64) \
      X(h26, 90 |  W_REG |        FPR | Arch_aarch64) \
      X(h27, 91 |  W_REG |        FPR | Arch_aarch64) \
      X(h28, 92 |  W_REG |        FPR | Arch_aarch64) \
      X(h29, 93 |  W_REG |        FPR | Arch_aarch64) \
      X(h30, 94 |  W_REG |        FPR | Arch_aarch64) \
      X(h31, 95 |  W_REG |        FPR | Arch_aarch64) \
      X(q0, 96 |  Q_REG |        FPR | Arch_aarch64) \
      X(q1, 97 |  Q_REG |        FPR | Arch_aarch64) \
      X(q2, 98 |  Q_REG |        FPR | Arch_aarch64) \
      X(q3, 99 |  Q_REG |        FPR | Arch_aarch64) \
      X(q4, 100 |  Q_REG |        FPR | Arch_aarch64) \
      X(q5, 101 |  Q_REG |        FPR | Arch_aarch64) \
      X(q6, 102 |  Q_REG |        FPR | Arch_aarch64) \
      X(q7, 103 |  Q_REG |        FPR | Arch_aarch64) \
      X(q8, 104 |  Q_REG |        FPR | Arch_aarch64) \
      X(q9, 105 |  Q_REG |        FPR | Arch_aarch64) \
      X(q10, 106 |  Q_REG |        FPR | Arch_aarch64) \
      X(q11, 107 |  Q_REG |        FPR | Arch_aarch64) \
      X(q12, 108 |  Q_REG |        FPR | Arch_aarch64) \
      X(q13, 109 |  Q_REG |        FPR | Arch_aarch64) \
      X(q14, 110 |  Q_REG |        FPR | Arch_aarch64) \
      X(q15, 111 |  Q_REG |        FPR | Arch_aarch64) \
      X(q16, 112 |  Q_REG |        FPR | Arch_aarch64) \
      X(q17, 113 |  Q_REG |        FPR | Arch_aarch64) \
      X(q18, 114 |  Q_REG |        FPR | Arch_aarch64) \
      X(q19, 115 |  Q_REG |        FPR | Arch_aarch64) \
      X(q20, 116 |  Q_REG |        FPR | Arch_aarch64) \
      X(q21, 117 |  Q_REG |        FPR | Arch_aarch64) \
      X(q22, 118 |  Q_REG |        FPR | Arch_aarch64) \
      X(q23, 119 |  Q_REG |        FPR | Arch_aarch64) \
      X(q24, 120 |  Q_REG |        FPR | Arch_aarch64) \
      X(q25, 121 |  Q_REG |        FPR | Arch_aarch64) \
      X(q26, 122 |  Q_REG |        FPR | Arch_aarch64) \
      X(q27, 123 |  Q_REG |        FPR | Arch_aarch64) \
      X(q28, 124 |  Q_REG |        FPR | Arch_aarch64) \
      X(q29, 125 |  Q_REG |        FPR | Arch_aarch64) \
      X(q30, 126 |  Q_REG |        FPR | Arch_aarch64) \
      X(q31, 127 |  Q_REG |        FPR | Arch_aarch64) \
      X(s0, 128 |  D_REG |        FPR | Arch_aarch64) \
      X(s1, 129 |  D_REG |        FPR | Arch_aarch64) \
      X(s2, 130 |  D_REG |        FPR | Arch_aarch64) \
      X(s3, 131 |  D_REG |        FPR | Arch_aarch64) \
      X(s4, 132 |  D_REG |        FPR | Arch_aarch64) \
      X(s5, 133 |  D_REG |        FPR | Arch_aarch64) \
      X(s6, 134 |  D_REG |        FPR | Arch_aarch64) \
      X(s7, 135 |  D_REG |        FPR | Arch_aarch64) \
      X(s8, 136 |  D_REG |        FPR | Arch_aarch64) \
      X(s9, 137 |  D_REG |        FPR | Arch_aarch64) \
      X(s10, 138 |  D_REG |        FPR | Arch_aarch64) \
      X(s11, 139 |  D_REG |        FPR | Arch_aarch64) \
      X(s12, 140 |  D_REG |        FPR | Arch_aarch64) \
      X(s13, 141 |  D_REG |        FPR | Arch_aarch64) \
      X(s14, 142 |  D_REG |        FPR | Arch_aarch64) \
      X(s15, 143 |  D_REG |        FPR | Arch_aarch64) \
      X(s16, 144 |  D_REG |        FPR | Arch_aarch64) \
      X(s17, 145 |  D_REG |        FPR | Arch_aarch64) \
      X(s18, 146 |  D_REG |        FPR | Arch_aarch64) \
      X(s19, 147 |  D_REG |        FPR | Arch_aarch64) \
      X(s20, 148 |  D_REG |        FPR | Arch_aarch64) \
      X(s21, 149 |  D_REG |        FPR | Arch_aarch64) \
      X(s22, 150 |  D_REG |        FPR | Arch_aarch64) \
      X(s23, 151 |  D_REG |        FPR | Arch_aarch64) \
      X(s24, 152 |  D_REG |        FPR | Arch_aarch64) \
      X(s25, 153 |  D_REG |        FPR | Arch_aarch64) \
      X(s26, 154 |  D_REG |        FPR | Arch_aarch64) \
      X(s27, 155 |  D_REG |        FPR | Arch_aarch64) \
      X(s28, 156 |  D_REG |        FPR | Arch_aarch64) \
      X(s29, 157 |  D_REG |        FPR | Arch_aarch64) \
      X(s30, 158 |  D_REG |        FPR | Arch_aarch64) \
      X(s31, 159 |  D_REG |        FPR | Arch_aarch64) \
      X(ffr, 0 |  PREDS |        SVE | Arch_aarch64) \
      X(p0, 1 |  PREDS |        SVE | Arch_aarch64) \
      X(p1, 2 |  PREDS |        SVE | Arch_aarch64) \
      X(p2, 3 |  PREDS |        SVE | Arch_aarch64) \
      X(p3, 4 |  PREDS |        SVE | Arch_aarch64) \
      X(p4, 5 |  PREDS |        SVE | Arch_aarch64) \
      X(p5, 6 |  PREDS |        SVE | Arch_aarch64) \
      X(p6, 7 |  PREDS |        SVE | Arch_aarch64) \
      X(p7, 8 |  PREDS |        SVE | Arch_aarch64) \
      X(p8, 9 |  PREDS |        SVE | Arch_aarch64) \
      X(p9, 10 |  PREDS |        SVE | Arch_aarch64) \
      X(p10, 11 |  PREDS |        SVE | Arch_aarch64) \
      X(p11, 12 |  PREDS |        SVE | Arch_aarch64) \
      X(p12, 13 |  PREDS |        SVE | Arch_aarch64) \
      X(p13, 14 |  PREDS |        SVE | Arch_aarch64) \
      X(p14, 15 |  PREDS |        SVE | Arch_aarch64) \
      X(p15, 16 |  PREDS |        SVE | Arch_aarch64) \
      X(vg, 17 |   FULL |        SVE | Arch_aarch64) \
      X(z0, 18 |   SVES |        SVE | Arch_aarch64) \
      X(z1, 19 |   SVES |        SVE | Arch_aarch64) \
      X(z2, 20 |   SVES |        SVE | Arch_aarch64) \
      X(z3, 21 |   SVES |        SVE | Arch_aarch64) \
      X(z4, 22 |   SVES |        SVE | Arch_aarch64) \
      X(z5, 23 |   SVES |        SVE | Arch_aarch64) \
      X(z6, 24 |   SVES |        SVE | Arch_aarch64) \
      X(z7, 25 |   SVES |        SVE | Arch_aarch64) \
      X(z8, 26 |   SVES |        SVE | Arch_aarch64) \
      X(z9, 27 |   SVES |        SVE | Arch_aarch64) \
      X(z10, 28 |   SVES |        SVE | Arch_aarch64) \
      X(z11, 29 |   SVES |        SVE | Arch_aarch64) \
      X(z12, 30 |   SVES |        SVE | Arch_aarch64) \
      X(z13, 31 |   SVES |        SVE | Arch_aarch64) \
      X(z14, 32 |   SVES |        SVE | Arch_aarch64) \
      X(z15, 33 |   SVES |        SVE | Arch_aarch64) \
      X(z16, 34 |   SVES |        SVE | Arch_aarch64) \
      X(z17, 35 |   SVES |        SVE | Arch_aarch64) \
      X(z18, 36 |   SVES |        SVE | Arch_aarch64) \
      X(z19, 37 |   SVES |        SVE | Arch_aarch64) \
      X(z20, 38 |   SVES |        SVE | Arch_aarch64) \
      X(z21, 39 |   SVES |        SVE | Arch_aarch64) \
      X(z22, 40 |   SVES |        SVE | Arch_aarch64) \
      X(z23, 41 |   SVES |        SVE | Arch_aarch64) \
      X(z24, 42 |   SVES |        SVE | Arch_aarch64) \
      X(z25, 43 |   SVES |        SVE | Arch_aarch64) \
      X(z26, 44 |   SVES |        SVE | Arch_aarch64) \
      X(z27, 45 |   SVES |        SVE | Arch_aarch64) \
      X(z28, 46 |   SVES |        SVE | Arch_aarch64) \
      X(z29, 47 |   SVES |        SVE | Arch_aarch64) \
      X(z30, 48 |   SVES |        SVE | Arch_aarch64) \
      X(z31, 49 |   SVES |        SVE | Arch_aarch64) \
      X(zt0, 0 |  SVE2S |       SVE2 | Arch_aarch64) \
      X(za, 0 | SMEZAS |        SME | Arch_aarch64) \
      X(zab0, 1 | SMEZAS |        SME | Arch_aarch64) \
      X(zad0, 2 | SMEZAS |        SME | Arch_aarch64) \
      X(zad1, 3 | SMEZAS |        SME | Arch_aarch64) \
      X(zad2, 4 | SMEZAS |        SME | Arch_aarch64) \
      X(zad3, 5 | SMEZAS |        SME | Arch_aarch64) \
      X(zad4, 6 | SMEZAS |        SME | Arch_aarch64) \
      X(zad5, 7 | SMEZAS |        SME | Arch_aarch64) \
      X(zad6, 8 | SMEZAS |        SME | Arch_aarch64) \
      X(zad7, 9 | SMEZAS |        SME | Arch_aarch64) \
      X(zah0, 10 | SMEZAS |        SME | Arch_aarch64) \
      X(zah1, 11 | SMEZAS |        SME | Arch_aarch64) \
      X(zaq0, 12 | SMEZAS |        SME | Arch_aarch64) \
      X(zaq1, 13 | SMEZAS |        SME | Arch_aarch64) \
      X(zaq2, 14 | SMEZAS |        SME | Arch_aarch64) \
      X(zaq3, 15 | SMEZAS |        SME | Arch_aarch64) \
      X(zaq4, 16 | SMEZAS |        SME | Arch_aarch64) \
      X(zaq5, 17 | SMEZAS |        SME | Arch_aarch64) \
      X(zaq6, 18 | SMEZAS |        SME | Arch_aarch64) \
      X(zaq7, 19 | SMEZAS |        SME | Arch_aarch64) \
      X(zaq8, 20 | SMEZAS |        SME | Arch_aarch64) \
      X(zaq9, 21 | SMEZAS |        SME | Arch_aarch64) \
      X(zaq10, 22 | SMEZAS |        SME | Arch_aarch64) \
      X(zaq11, 23 | SMEZAS |        SME | Arch_aarch64) \
      X(zaq12, 24 | SMEZAS |        SME | Arch_aarch64) \
      X(zaq13, 25 | SMEZAS |        SME | Arch_aarch64) \
      X(zaq14, 26 | SMEZAS |        SME | Arch_aarch64) \
      X(zaq15, 27 | SMEZAS |        SME | Arch_aarch64) \
      X(zas0, 28 | SMEZAS |        SME | Arch_aarch64) \
      X(zas1, 29 | SMEZAS |        SME | Arch_aarch64) \
      X(zas2, 30 | SMEZAS |        SME | Arch_aarch64) \
      X(zas3, 31 | SMEZAS |        SME | Arch_aarch64) \
      X(elr_el1, 0 |   FULL |     SYSSPR | Arch_aarch64) \
      X(elr_el2, 1 |   FULL |     SYSSPR | Arch_aarch64) \
      X(elr_el3, 2 |   FULL |     SYSSPR | Arch_aarch64) \
      X(sp_el0, 3 |   FULL |     SYSSPR | Arch_aarch64) \
      X(sp_el1, 4 |   FULL |     SYSSPR | Arch_aarch64) \
      X(sp_el2, 5 |   FULL |     SYSSPR | Arch_aarch64) \
      X(spsr_abt, 6 |   FULL |     SYSSPR | Arch_aarch64) \
      X(spsr_el1, 7 |   FULL |     SYSSPR | Arch_aarch64) \
      X(spsr_el2, 8 |   FULL |     SYSSPR | Arch_aarch64) \
      X(spsr_el3, 9 |   FULL |     SYSSPR | Arch_aarch64) \
      X(spsr_fiq, 10 |   FULL |     SYSSPR | Arch_aarch64) \
      X(spsr_irq, 11 |   FULL |     SYSSPR | Arch_aarch64) \
      X(spsr_und, 12 |   FULL |     SYSSPR | Arch_aarch64) \
      X(amair2_el1, 0 |   FULL |  SYSMEMORY | Arch_aarch64) \
      X(amair2_el2, 1 |   FULL |  SYSMEMORY | Arch_aarch64) \
      X(amair2_el3, 2 |   FULL |  SYSMEMORY | Arch_aarch64) \
      X(amair_el1, 3 |   FULL |  SYSMEMORY | Arch_aarch64) \
      X(amair_el2, 4 |   FULL |  SYSMEMORY | Arch_aarch64) \
      X(amair_el3, 5 |   FULL |  SYSMEMORY | Arch_aarch64) \
      X(contextidr_el1, 6 |   FULL |  SYSMEMORY | Arch_aarch64) \
      X(contextidr_el2, 7 |   FULL |  SYSMEMORY | Arch_aarch64) \
      X(dacr32_el2, 8 |   FULL |  SYSMEMORY | Arch_aarch64) \
      X(gpccr_el3, 9 |   FULL |  SYSMEMORY | Arch_aarch64) \
      X(gptbr_el3, 10 |   FULL |  SYSMEMORY | Arch_aarch64) \
      X(hacdbsbr_el2, 11 |   FULL |  SYSMEMORY | Arch_aarch64) \
      X(hacdbscons_el2, 12 |   FULL |  SYSMEMORY | Arch_aarch64) \
      X(hdbssbr_el2, 13 |   FULL |  SYSMEMORY | Arch_aarch64) \
      X(hdbssprod_el2, 14 |   FULL |  SYSMEMORY | Arch_aarch64) \
      X(lorc_el1, 15 |   FULL |  SYSMEMORY | Arch_aarch64) \
      X(lorea_el1, 16 |   FULL |  SYSMEMORY | Arch_aarch64) \
      X(lorid_el1, 17 |   FULL |  SYSMEMORY | Arch_aarch64) \
      X(lorn_el1, 18 |   FULL |  SYSMEMORY | Arch_aarch64) \
      X(lorsa_el1, 19 |   FULL |  SYSMEMORY | Arch_aarch64) \
      X(mair2_el1, 20 |   FULL |  SYSMEMORY | Arch_aarch64) \
      X(mair2_el2, 21 |   FULL |  SYSMEMORY | Arch_aarch64) \
      X(mair2_el3, 22 |   FULL |  SYSMEMORY | Arch_aarch64) \
      X(mair_el1, 23 |   FULL |  SYSMEMORY | Arch_aarch64) \
      X(mair_el2, 24 |   FULL |  SYSMEMORY | Arch_aarch64) \
      X(mair_el3, 25 |   FULL |  SYSMEMORY | Arch_aarch64) \
      X(pir_el1, 26 |   FULL |  SYSMEMORY | Arch_aarch64) \
      X(pir_el2, 27 |   FULL |  SYSMEMORY | Arch_aarch64) \
      X(pir_el3, 28 |   FULL |  SYSMEMORY | Arch_aarch64) \
      X(pire0_el1, 29 |   FULL |  SYSMEMORY | Arch_aarch64) \
      X(pire0_el2, 30 |   FULL |  SYSMEMORY | Arch_aarch64) \
      X(por_el0, 31 |   FULL |  SYSMEMORY | Arch_aarch64) \
      X(por_el1, 32 |   FULL |  SYSMEMORY | Arch_aarch64) \
      X(por_el2, 33 |   FULL |  SYSMEMORY | Arch_aarch64) \
      X(por_el3, 34 |   FULL |  SYSMEMORY | Arch_aarch64) \
      X(rcwmask_el1, 35 |  Q_REG |  SYSMEMORY | Arch_aarch64) \
      X(rcwsmask_el1, 36 |  Q_REG |  SYSMEMORY | Arch_aarch64) \
      X(s2pir_el2, 37 |   FULL |  SYSMEMORY | Arch_aarch64) \
      X(s2por_el1, 38 |   FULL |  SYSMEMORY | Arch_aarch64) \
      X(tcr2_el1, 39 |   FULL |  SYSMEMORY | Arch_aarch64) \
      X(tcr2_el2, 40 |   FULL |  SYSMEMORY | Arch_aarch64) \
      X(tcr_el1, 41 |   FULL |  SYSMEMORY | Arch_aarch64) \
      X(tcr_el2, 42 |   FULL |  SYSMEMORY | Arch_aarch64) \
      X(tcr_el3, 43 |   FULL |  SYSMEMORY | Arch_aarch64) \
      X(ttbr0_el1, 44 |  Q_REG |  SYSMEMORY | Arch_aarch64) \
      X(ttbr1_el1, 45 |  Q_REG |  SYSMEMORY | Arch_aarch64) \
      X(ttbr0_el2, 46 |  Q_REG |  SYSMEMORY | Arch_aarch64) \
      X(ttbr1_el2, 47 |  Q_REG |  SYSMEMORY | Arch_aarch64) \
      X(ttbr0_el3, 48 |   FULL |  SYSMEMORY | Arch_aarch64) \
      X(vsctlr_el2, 49 |  D_REG |  SYSMEMORY | Arch_aarch64) \
      X(vtcr_el2, 50 |   FULL |  SYSMEMORY | Arch_aarch64) \
      X(vttbr_el2, 51 |  Q_REG |  SYSMEMORY | Arch_aarch64) \
      X(accdata_el1, 0 |   FULL |         AD | Arch_aarch64) \
      X(actlr_el1, 0 |   FULL |    IMPLDEF | Arch_aarch64) \
      X(actlr_el2, 1 |   FULL |    IMPLDEF | Arch_aarch64) \
      X(actlr_el3, 2 |   FULL |    IMPLDEF | Arch_aarch64) \
      X(afsr0_el1, 3 |   FULL |    IMPLDEF | Arch_aarch64) \
      X(afsr1_el1, 4 |   FULL |    IMPLDEF | Arch_aarch64) \
      X(afsr0_el2, 5 |   FULL |    IMPLDEF | Arch_aarch64) \
      X(afsr1_el2, 6 |   FULL |    IMPLDEF | Arch_aarch64) \
      X(afsr0_el3, 7 |   FULL |    IMPLDEF | Arch_aarch64) \
      X(afsr1_el3, 8 |   FULL |    IMPLDEF | Arch_aarch64) \
      X(aidr_el1, 9 |   FULL |    IMPLDEF | Arch_aarch64) \
      X(amcfgr_el0, 0 |   FULL |        AMR | Arch_aarch64) \
      X(amcg1idr_el0, 1 |   FULL |        AMR | Arch_aarch64) \
      X(amcgcr_el0, 2 |   FULL |        AMR | Arch_aarch64) \
      X(amcntenclr0_el0, 3 |   FULL |        AMR | Arch_aarch64) \
      X(amcntenclr1_el0, 4 |   FULL |        AMR | Arch_aarch64) \
      X(amcntenset0_el0, 5 |   FULL |        AMR | Arch_aarch64) \
      X(amcntenset1_el0, 6 |   FULL |        AMR | Arch_aarch64) \
      X(amcr_el0, 7 |   FULL |        AMR | Arch_aarch64) \
      X(amevcntr00_el0, 8 |   FULL |        AMR | Arch_aarch64) \
      X(amevcntr01_el0, 9 |   FULL |        AMR | Arch_aarch64) \
      X(amevcntr02_el0, 10 |   FULL |        AMR | Arch_aarch64) \
      X(amevcntr03_el0, 11 |   FULL |        AMR | Arch_aarch64) \
      X(amevcntr10_el0, 12 |   FULL |        AMR | Arch_aarch64) \
      X(amevcntr11_el0, 13 |   FULL |        AMR | Arch_aarch64) \
      X(amevcntr12_el0, 14 |   FULL |        AMR | Arch_aarch64) \
      X(amevcntr13_el0, 15 |   FULL |        AMR | Arch_aarch64) \
      X(amevcntr14_el0, 16 |   FULL |        AMR | Arch_aarch64) \
      X(amevcntr15_el0, 17 |   FULL |        AMR | Arch_aarch64) \
      X(amevcntr16_el0, 18 |   FULL |        AMR | Arch_aarch64) \
      X(amevcntr17_el0, 19 |   FULL |        AMR | Arch_aarch64) \
      X(amevcntr18_el0, 20 |   FULL |        AMR | Arch_aarch64) \
      X(amevcntr19_el0, 21 |   FULL |        AMR | Arch_aarch64) \
      X(amevcntr110_el0, 22 |   FULL |        AMR | Arch_aarch64) \
      X(amevcntr111_el0, 23 |   FULL |        AMR | Arch_aarch64) \
      X(amevcntr112_el0, 24 |   FULL |        AMR | Arch_aarch64) \
      X(amevcntr113_el0, 25 |   FULL |        AMR | Arch_aarch64) \
      X(amevcntr114_el0, 26 |   FULL |        AMR | Arch_aarch64) \
      X(amevcntr115_el0, 27 |   FULL |        AMR | Arch_aarch64) \
      X(amevcntvoff00_el2, 28 |   FULL |        AMR | Arch_aarch64) \
      X(amevcntvoff01_el2, 29 |   FULL |        AMR | Arch_aarch64) \
      X(amevcntvoff02_el2, 30 |   FULL |        AMR | Arch_aarch64) \
      X(amevcntvoff03_el2, 31 |   FULL |        AMR | Arch_aarch64) \
      X(amevcntvoff04_el2, 32 |   FULL |        AMR | Arch_aarch64) \
      X(amevcntvoff05_el2, 33 |   FULL |        AMR | Arch_aarch64) \
      X(amevcntvoff06_el2, 34 |   FULL |        AMR | Arch_aarch64) \
      X(amevcntvoff07_el2, 35 |   FULL |        AMR | Arch_aarch64) \
      X(amevcntvoff08_el2, 36 |   FULL |        AMR | Arch_aarch64) \
      X(amevcntvoff09_el2, 37 |   FULL |        AMR | Arch_aarch64) \
      X(amevcntvoff010_el2, 38 |   FULL |        AMR | Arch_aarch64) \
      X(amevcntvoff011_el2, 39 |   FULL |        AMR | Arch_aarch64) \
      X(amevcntvoff012_el2, 40 |   FULL |        AMR | Arch_aarch64) \
      X(amevcntvoff013_el2, 41 |   FULL |        AMR | Arch_aarch64) \
      X(amevcntvoff014_el2, 42 |   FULL |        AMR | Arch_aarch64) \
      X(amevcntvoff015_el2, 43 |   FULL |        AMR | Arch_aarch64) \
      X(amevcntvoff10_el2, 44 |   FULL |        AMR | Arch_aarch64) \
      X(amevcntvoff11_el2, 45 |   FULL |        AMR | Arch_aarch64) \
      X(amevcntvoff12_el2, 46 |   FULL |        AMR | Arch_aarch64) \
      X(amevcntvoff13_el2, 47 |   FULL |        AMR | Arch_aarch64) \
      X(amevcntvoff14_el2, 48 |   FULL |        AMR | Arch_aarch64) \
      X(amevcntvoff15_el2, 49 |   FULL |        AMR | Arch_aarch64) \
      X(amevcntvoff16_el2, 50 |   FULL |        AMR | Arch_aarch64) \
      X(amevcntvoff17_el2, 51 |   FULL |        AMR | Arch_aarch64) \
      X(amevcntvoff18_el2, 52 |   FULL |        AMR | Arch_aarch64) \
      X(amevcntvoff19_el2, 53 |   FULL |        AMR | Arch_aarch64) \
      X(amevcntvoff110_el2, 54 |   FULL |        AMR | Arch_aarch64) \
      X(amevcntvoff111_el2, 55 |   FULL |        AMR | Arch_aarch64) \
      X(amevcntvoff112_el2, 56 |   FULL |        AMR | Arch_aarch64) \
      X(amevcntvoff113_el2, 57 |   FULL |        AMR | Arch_aarch64) \
      X(amevcntvoff114_el2, 58 |   FULL |        AMR | Arch_aarch64) \
      X(amevcntvoff115_el2, 59 |   FULL |        AMR | Arch_aarch64) \
      X(amevtyper00_el0, 60 |   FULL |        AMR | Arch_aarch64) \
      X(amevtyper01_el0, 61 |   FULL |        AMR | Arch_aarch64) \
      X(amevtyper02_el0, 62 |   FULL |        AMR | Arch_aarch64) \
      X(amevtyper03_el0, 63 |   FULL |        AMR | Arch_aarch64) \
      X(amevtyper10_el0, 64 |   FULL |        AMR | Arch_aarch64) \
      X(amevtyper11_el0, 65 |   FULL |        AMR | Arch_aarch64) \
      X(amevtyper12_el0, 66 |   FULL |        AMR | Arch_aarch64) \
      X(amevtyper13_el0, 67 |   FULL |        AMR | Arch_aarch64) \
      X(amevtyper14_el0, 68 |   FULL |        AMR | Arch_aarch64) \
      X(amevtyper15_el0, 69 |   FULL |        AMR | Arch_aarch64) \
      X(amevtyper16_el0, 70 |   FULL |        AMR | Arch_aarch64) \
      X(amevtyper17_el0, 71 |   FULL |        AMR | Arch_aarch64) \
      X(amevtyper18_el0, 72 |   FULL |        AMR | Arch_aarch64) \
      X(amevtyper19_el0, 73 |   FULL |        AMR | Arch_aarch64) \
      X(amevtyper110_el0, 74 |   FULL |        AMR | Arch_aarch64) \
      X(amevtyper111_el0, 75 |   FULL |        AMR | Arch_aarch64) \
      X(amevtyper112_el0, 76 |   FULL |        AMR | Arch_aarch64) \
      X(amevtyper113_el0, 77 |   FULL |        AMR | Arch_aarch64) \
      X(amevtyper114_el0, 78 |   FULL |        AMR | Arch_aarch64) \
      X(amevtyper115_el0, 79 |   FULL |        AMR | Arch_aarch64) \
      X(amuserenr_el0, 80 |   FULL |        AMR | Arch_aarch64) \
      X(apdakeyhi_el1, 0 |   FULL |     SYSPTR | Arch_aarch64) \
      X(apdakeylo_el1, 1 |   FULL |     SYSPTR | Arch_aarch64) \
      X(apdbkeyhi_el1, 2 |   FULL |     SYSPTR | Arch_aarch64) \
      X(apdbkeylo_el1, 3 |   FULL |     SYSPTR | Arch_aarch64) \
      X(apgakeyhi_el1, 4 |   FULL |     SYSPTR | Arch_aarch64) \
      X(apgakeylo_el1, 5 |   FULL |     SYSPTR | Arch_aarch64) \
      X(apiakeyhi_el1, 6 |   FULL |     SYSPTR | Arch_aarch64) \
      X(apiakeylo_el1, 7 |   FULL |     SYSPTR | Arch_aarch64) \
      X(apibkeyhi_el1, 8 |   FULL |     SYSPTR | Arch_aarch64) \
      X(apibkeylo_el1, 9 |   FULL |     SYSPTR | Arch_aarch64) \
      X(brbcr_el1, 0 |   FULL |       BRBE | Arch_aarch64) \
      X(brbcr_el2, 1 |   FULL |       BRBE | Arch_aarch64) \
      X(brbfcr_el1, 2 |   FULL |       BRBE | Arch_aarch64) \
      X(brbidr0_el1, 3 |   FULL |       BRBE | Arch_aarch64) \
      X(brbinf0_el1, 4 |   FULL |       BRBE | Arch_aarch64) \
      X(brbinf1_el1, 5 |   FULL |       BRBE | Arch_aarch64) \
      X(brbinf2_el1, 6 |   FULL |       BRBE | Arch_aarch64) \
      X(brbinf3_el1, 7 |   FULL |       BRBE | Arch_aarch64) \
      X(brbinf4_el1, 8 |   FULL |       BRBE | Arch_aarch64) \
      X(brbinf5_el1, 9 |   FULL |       BRBE | Arch_aarch64) \
      X(brbinf6_el1, 10 |   FULL |       BRBE | Arch_aarch64) \
      X(brbinf7_el1, 11 |   FULL |       BRBE | Arch_aarch64) \
      X(brbinf8_el1, 12 |   FULL |       BRBE | Arch_aarch64) \
      X(brbinf9_el1, 13 |   FULL |       BRBE | Arch_aarch64) \
      X(brbinf10_el1, 14 |   FULL |       BRBE | Arch_aarch64) \
      X(brbinf11_el1, 15 |   FULL |       BRBE | Arch_aarch64) \
      X(brbinf12_el1, 16 |   FULL |       BRBE | Arch_aarch64) \
      X(brbinf13_el1, 17 |   FULL |       BRBE | Arch_aarch64) \
      X(brbinf14_el1, 18 |   FULL |       BRBE | Arch_aarch64) \
      X(brbinf15_el1, 19 |   FULL |       BRBE | Arch_aarch64) \
      X(brbinf16_el1, 20 |   FULL |       BRBE | Arch_aarch64) \
      X(brbinf17_el1, 21 |   FULL |       BRBE | Arch_aarch64) \
      X(brbinf18_el1, 22 |   FULL |       BRBE | Arch_aarch64) \
      X(brbinf19_el1, 23 |   FULL |       BRBE | Arch_aarch64) \
      X(brbinf20_el1, 24 |   FULL |       BRBE | Arch_aarch64) \
      X(brbinf21_el1, 25 |   FULL |       BRBE | Arch_aarch64) \
      X(brbinf22_el1, 26 |   FULL |       BRBE | Arch_aarch64) \
      X(brbinf23_el1, 27 |   FULL |       BRBE | Arch_aarch64) \
      X(brbinf24_el1, 28 |   FULL |       BRBE | Arch_aarch64) \
      X(brbinf25_el1, 29 |   FULL |       BRBE | Arch_aarch64) \
      X(brbinf26_el1, 30 |   FULL |       BRBE | Arch_aarch64) \
      X(brbinf27_el1, 31 |   FULL |       BRBE | Arch_aarch64) \
      X(brbinf28_el1, 32 |   FULL |       BRBE | Arch_aarch64) \
      X(brbinf29_el1, 33 |   FULL |       BRBE | Arch_aarch64) \
      X(brbinf30_el1, 34 |   FULL |       BRBE | Arch_aarch64) \
      X(brbinf31_el1, 35 |   FULL |       BRBE | Arch_aarch64) \
      X(brbinfinj_el1, 36 |   FULL |       BRBE | Arch_aarch64) \
      X(brbsrc0_el1, 37 |   FULL |       BRBE | Arch_aarch64) \
      X(brbsrc1_el1, 38 |   FULL |       BRBE | Arch_aarch64) \
      X(brbsrc2_el1, 39 |   FULL |       BRBE | Arch_aarch64) \
      X(brbsrc3_el1, 40 |   FULL |       BRBE | Arch_aarch64) \
      X(brbsrc4_el1, 41 |   FULL |       BRBE | Arch_aarch64) \
      X(brbsrc5_el1, 42 |   FULL |       BRBE | Arch_aarch64) \
      X(brbsrc6_el1, 43 |   FULL |       BRBE | Arch_aarch64) \
      X(brbsrc7_el1, 44 |   FULL |       BRBE | Arch_aarch64) \
      X(brbsrc8_el1, 45 |   FULL |       BRBE | Arch_aarch64) \
      X(brbsrc9_el1, 46 |   FULL |       BRBE | Arch_aarch64) \
      X(brbsrc10_el1, 47 |   FULL |       BRBE | Arch_aarch64) \
      X(brbsrc11_el1, 48 |   FULL |       BRBE | Arch_aarch64) \
      X(brbsrc12_el1, 49 |   FULL |       BRBE | Arch_aarch64) \
      X(brbsrc13_el1, 50 |   FULL |       BRBE | Arch_aarch64) \
      X(brbsrc14_el1, 51 |   FULL |       BRBE | Arch_aarch64) \
      X(brbsrc15_el1, 52 |   FULL |       BRBE | Arch_aarch64) \
      X(brbsrc16_el1, 53 |   FULL |       BRBE | Arch_aarch64) \
      X(brbsrc17_el1, 54 |   FULL |       BRBE | Arch_aarch64) \
      X(brbsrc18_el1, 55 |   FULL |       BRBE | Arch_aarch64) \
      X(brbsrc19_el1, 56 |   FULL |       BRBE | Arch_aarch64) \
      X(brbsrc20_el1, 57 |   FULL |       BRBE | Arch_aarch64) \
      X(brbsrc21_el1, 58 |   FULL |       BRBE | Arch_aarch64) \
      X(brbsrc22_el1, 59 |   FULL |       BRBE | Arch_aarch64) \
      X(brbsrc23_el1, 60 |   FULL |       BRBE | Arch_aarch64) \
      X(brbsrc24_el1, 61 |   FULL |       BRBE | Arch_aarch64) \
      X(brbsrc25_el1, 62 |   FULL |       BRBE | Arch_aarch64) \
      X(brbsrc26_el1, 63 |   FULL |       BRBE | Arch_aarch64) \
      X(brbsrc27_el1, 64 |   FULL |       BRBE | Arch_aarch64) \
      X(brbsrc28_el1, 65 |   FULL |       BRBE | Arch_aarch64) \
      X(brbsrc29_el1, 66 |   FULL |       BRBE | Arch_aarch64) \
      X(brbsrc30_el1, 67 |   FULL |       BRBE | Arch_aarch64) \
      X(brbsrc31_el1, 68 |   FULL |       BRBE | Arch_aarch64) \
      X(brbsrcinj_el1, 69 |   FULL |       BRBE | Arch_aarch64) \
      X(brbtgt0_el1, 70 |   FULL |       BRBE | Arch_aarch64) \
      X(brbtgt1_el1, 71 |   FULL |       BRBE | Arch_aarch64) \
      X(brbtgt2_el1, 72 |   FULL |       BRBE | Arch_aarch64) \
      X(brbtgt3_el1, 73 |   FULL |       BRBE | Arch_aarch64) \
      X(brbtgt4_el1, 74 |   FULL |       BRBE | Arch_aarch64) \
      X(brbtgt5_el1, 75 |   FULL |       BRBE | Arch_aarch64) \
      X(brbtgt6_el1, 76 |   FULL |       BRBE | Arch_aarch64) \
      X(brbtgt7_el1, 77 |   FULL |       BRBE | Arch_aarch64) \
      X(brbtgt8_el1, 78 |   FULL |       BRBE | Arch_aarch64) \
      X(brbtgt9_el1, 79 |   FULL |       BRBE | Arch_aarch64) \
      X(brbtgt10_el1, 80 |   FULL |       BRBE | Arch_aarch64) \
      X(brbtgt11_el1, 81 |   FULL |       BRBE | Arch_aarch64) \
      X(brbtgt12_el1, 82 |   FULL |       BRBE | Arch_aarch64) \
      X(brbtgt13_el1, 83 |   FULL |       BRBE | Arch_aarch64) \
      X(brbtgt14_el1, 84 |   FULL |       BRBE | Arch_aarch64) \
      X(brbtgt15_el1, 85 |   FULL |       BRBE | Arch_aarch64) \
      X(brbtgt16_el1, 86 |   FULL |       BRBE | Arch_aarch64) \
      X(brbtgt17_el1, 87 |   FULL |       BRBE | Arch_aarch64) \
      X(brbtgt18_el1, 88 |   FULL |       BRBE | Arch_aarch64) \
      X(brbtgt19_el1, 89 |   FULL |       BRBE | Arch_aarch64) \
      X(brbtgt20_el1, 90 |   FULL |       BRBE | Arch_aarch64) \
      X(brbtgt21_el1, 91 |   FULL |       BRBE | Arch_aarch64) \
      X(brbtgt22_el1, 92 |   FULL |       BRBE | Arch_aarch64) \
      X(brbtgt23_el1, 93 |   FULL |       BRBE | Arch_aarch64) \
      X(brbtgt24_el1, 94 |   FULL |       BRBE | Arch_aarch64) \
      X(brbtgt25_el1, 95 |   FULL |       BRBE | Arch_aarch64) \
      X(brbtgt26_el1, 96 |   FULL |       BRBE | Arch_aarch64) \
      X(brbtgt27_el1, 97 |   FULL |       BRBE | Arch_aarch64) \
      X(brbtgt28_el1, 98 |   FULL |       BRBE | Arch_aarch64) \
      X(brbtgt29_el1, 99 |   FULL |       BRBE | Arch_aarch64) \
      X(brbtgt30_el1, 100 |   FULL |       BRBE | Arch_aarch64) \
      X(brbtgt31_el1, 101 |   FULL |       BRBE | Arch_aarch64) \
      X(brbtgtinj_el1, 102 |   FULL |       BRBE | Arch_aarch64) \
      X(brbts_el1, 103 |   FULL |       BRBE | Arch_aarch64) \
      X(ccsidr2_el1, 0 |   FULL |      SYSID | Arch_aarch64) \
      X(ccsidr_el1, 1 |   FULL |      SYSID | Arch_aarch64) \
      X(clidr_el1, 2 |   FULL |      SYSID | Arch_aarch64) \
      X(cntfrq_el0, 0 |   FULL |   SYSTIMER | Arch_aarch64) \
      X(cnthctl_el2, 0 |   FULL |       VIRT | Arch_aarch64) \
      X(cnthp_ctl_el2, 1 |   FULL |       VIRT | Arch_aarch64) \
      X(cnthp_cval_el2, 2 |   FULL |       VIRT | Arch_aarch64) \
      X(cnthp_tval_el2, 3 |   FULL |       VIRT | Arch_aarch64) \
      X(cnthps_ctl_el2, 4 |   FULL |       VIRT | Arch_aarch64) \
      X(cnthps_cval_el2, 5 |   FULL |       VIRT | Arch_aarch64) \
      X(cnthps_tval_el2, 6 |   FULL |       VIRT | Arch_aarch64) \
      X(cnthv_ctl_el2, 1 |   FULL |   SYSTIMER | Arch_aarch64) \
      X(cnthv_cval_el2, 2 |   FULL |   SYSTIMER | Arch_aarch64) \
      X(cnthv_tval_el2, 3 |   FULL |   SYSTIMER | Arch_aarch64) \
      X(cnthvs_ctl_el2, 4 |   FULL |   SYSTIMER | Arch_aarch64) \
      X(cnthvs_cval_el2, 5 |   FULL |   SYSTIMER | Arch_aarch64) \
      X(cnthvs_tval_el2, 6 |   FULL |   SYSTIMER | Arch_aarch64) \
      X(cntkctl_el1, 7 |   FULL |   SYSTIMER | Arch_aarch64) \
      X(cntp_ctl_el0, 8 |   FULL |   SYSTIMER | Arch_aarch64) \
      X(cntp_cval_el0, 9 |   FULL |   SYSTIMER | Arch_aarch64) \
      X(cntp_tval_el0, 10 |   FULL |   SYSTIMER | Arch_aarch64) \
      X(cntpct_el0, 11 |   FULL |   SYSTIMER | Arch_aarch64) \
      X(cntpctss_el0, 12 |   FULL |   SYSTIMER | Arch_aarch64) \
      X(cntpoff_el2, 13 |   FULL |   SYSTIMER | Arch_aarch64) \
      X(cntps_ctl_el1, 14 |   FULL |   SYSTIMER | Arch_aarch64) \
      X(cntps_cval_el1, 15 |   FULL |   SYSTIMER | Arch_aarch64) \
      X(cntps_tval_el1, 16 |   FULL |   SYSTIMER | Arch_aarch64) \
      X(cntv_ctl_el0, 17 |   FULL |   SYSTIMER | Arch_aarch64) \
      X(cntv_cval_el0, 18 |   FULL |   SYSTIMER | Arch_aarch64) \
      X(cntv_tval_el0, 19 |   FULL |   SYSTIMER | Arch_aarch64) \
      X(cntvct_el0, 20 |   FULL |   SYSTIMER | Arch_aarch64) \
      X(cntvctss_el0, 21 |   FULL |   SYSTIMER | Arch_aarch64) \
      X(cntvoff_el2, 7 |   FULL |       VIRT | Arch_aarch64) \
      X(cpacr_el1, 0 |   FULL |      OTHER | Arch_aarch64) \
      X(cptr_el2, 8 |   FULL |       VIRT | Arch_aarch64) \
      X(cptr_el3, 0 |   FULL |        SEC | Arch_aarch64) \
      X(csselr_el1, 3 |   FULL |      SYSID | Arch_aarch64) \
      X(ctr_el0, 4 |   FULL |      SYSID | Arch_aarch64) \
      X(dbgauthstatus_el1, 0 |  D_REG |    DBGAUTH | Arch_aarch64) \
      X(dbgbcr0_el1, 0 |   FULL |     DBGBRK | Arch_aarch64) \
      X(dbgbcr1_el1, 1 |   FULL |     DBGBRK | Arch_aarch64) \
      X(dbgbcr2_el1, 2 |   FULL |     DBGBRK | Arch_aarch64) \
      X(dbgbcr3_el1, 3 |   FULL |     DBGBRK | Arch_aarch64) \
      X(dbgbcr4_el1, 4 |   FULL |     DBGBRK | Arch_aarch64) \
      X(dbgbcr5_el1, 5 |   FULL |     DBGBRK | Arch_aarch64) \
      X(dbgbcr6_el1, 6 |   FULL |     DBGBRK | Arch_aarch64) \
      X(dbgbcr7_el1, 7 |   FULL |     DBGBRK | Arch_aarch64) \
      X(dbgbcr8_el1, 8 |   FULL |     DBGBRK | Arch_aarch64) \
      X(dbgbcr9_el1, 9 |   FULL |     DBGBRK | Arch_aarch64) \
      X(dbgbcr10_el1, 10 |   FULL |     DBGBRK | Arch_aarch64) \
      X(dbgbcr11_el1, 11 |   FULL |     DBGBRK | Arch_aarch64) \
      X(dbgbcr12_el1, 12 |   FULL |     DBGBRK | Arch_aarch64) \
      X(dbgbcr13_el1, 13 |   FULL |     DBGBRK | Arch_aarch64) \
      X(dbgbcr14_el1, 14 |   FULL |     DBGBRK | Arch_aarch64) \
      X(dbgbcr15_el1, 15 |   FULL |     DBGBRK | Arch_aarch64) \
      X(dbgbvr0_el1, 16 |   FULL |     DBGBRK | Arch_aarch64) \
      X(dbgbvr1_el1, 17 |   FULL |     DBGBRK | Arch_aarch64) \
      X(dbgbvr2_el1, 18 |   FULL |     DBGBRK | Arch_aarch64) \
      X(dbgbvr3_el1, 19 |   FULL |     DBGBRK | Arch_aarch64) \
      X(dbgbvr4_el1, 20 |   FULL |     DBGBRK | Arch_aarch64) \
      X(dbgbvr5_el1, 21 |   FULL |     DBGBRK | Arch_aarch64) \
      X(dbgbvr6_el1, 22 |   FULL |     DBGBRK | Arch_aarch64) \
      X(dbgbvr7_el1, 23 |   FULL |     DBGBRK | Arch_aarch64) \
      X(dbgbvr8_el1, 24 |   FULL |     DBGBRK | Arch_aarch64) \
      X(dbgbvr9_el1, 25 |   FULL |     DBGBRK | Arch_aarch64) \
      X(dbgbvr10_el1, 26 |   FULL |     DBGBRK | Arch_aarch64) \
      X(dbgbvr11_el1, 27 |   FULL |     DBGBRK | Arch_aarch64) \
      X(dbgbvr12_el1, 28 |   FULL |     DBGBRK | Arch_aarch64) \
      X(dbgbvr13_el1, 29 |   FULL |     DBGBRK | Arch_aarch64) \
      X(dbgbvr14_el1, 30 |   FULL |     DBGBRK | Arch_aarch64) \
      X(dbgbvr15_el1, 31 |   FULL |     DBGBRK | Arch_aarch64) \
      X(dbgclaimclr_el1, 0 |  D_REG |      DBGCT | Arch_aarch64) \
      X(dbgclaimset_el1, 1 |  D_REG |      DBGCT | Arch_aarch64) \
      X(dbgdtr_el0, 0 |   FULL |     DBGDTR | Arch_aarch64) \
      X(dbgdtrrx_el0, 1 |   FULL |     DBGDTR | Arch_aarch64) \
      X(dbgdtrtx_el0, 2 |   FULL |     DBGDTR | Arch_aarch64) \
      X(dbgprcr_el1, 0 |   FULL |     DBGPCR | Arch_aarch64) \
      X(dbgvcr32_el2, 0 |   FULL |       DVCR | Arch_aarch64) \
      X(dbgwcr0_el1, 0 |   FULL |       DBGW | Arch_aarch64) \
      X(dbgwcr1_el1, 1 |   FULL |       DBGW | Arch_aarch64) \
      X(dbgwcr2_el1, 2 |   FULL |       DBGW | Arch_aarch64) \
      X(dbgwcr3_el1, 3 |   FULL |       DBGW | Arch_aarch64) \
      X(dbgwcr4_el1, 4 |   FULL |       DBGW | Arch_aarch64) \
      X(dbgwcr5_el1, 5 |   FULL |       DBGW | Arch_aarch64) \
      X(dbgwcr6_el1, 6 |   FULL |       DBGW | Arch_aarch64) \
      X(dbgwcr7_el1, 7 |   FULL |       DBGW | Arch_aarch64) \
      X(dbgwcr8_el1, 8 |   FULL |       DBGW | Arch_aarch64) \
      X(dbgwcr9_el1, 9 |   FULL |       DBGW | Arch_aarch64) \
      X(dbgwcr10_el1, 10 |   FULL |       DBGW | Arch_aarch64) \
      X(dbgwcr11_el1, 11 |   FULL |       DBGW | Arch_aarch64) \
      X(dbgwcr12_el1, 12 |   FULL |       DBGW | Arch_aarch64) \
      X(dbgwcr13_el1, 13 |   FULL |       DBGW | Arch_aarch64) \
      X(dbgwcr14_el1, 14 |   FULL |       DBGW | Arch_aarch64) \
      X(dbgwcr15_el1, 15 |   FULL |       DBGW | Arch_aarch64) \
      X(dbgwvr0_el1, 16 |   FULL |       DBGW | Arch_aarch64) \
      X(dbgwvr1_el1, 17 |   FULL |       DBGW | Arch_aarch64) \
      X(dbgwvr2_el1, 18 |   FULL |       DBGW | Arch_aarch64) \
      X(dbgwvr3_el1, 19 |   FULL |       DBGW | Arch_aarch64) \
      X(dbgwvr4_el1, 20 |   FULL |       DBGW | Arch_aarch64) \
      X(dbgwvr5_el1, 21 |   FULL |       DBGW | Arch_aarch64) \
      X(dbgwvr6_el1, 22 |   FULL |       DBGW | Arch_aarch64) \
      X(dbgwvr7_el1, 23 |   FULL |       DBGW | Arch_aarch64) \
      X(dbgwvr8_el1, 24 |   FULL |       DBGW | Arch_aarch64) \
      X(dbgwvr9_el1, 25 |   FULL |       DBGW | Arch_aarch64) \
      X(dbgwvr10_el1, 26 |   FULL |       DBGW | Arch_aarch64) \
      X(dbgwvr11_el1, 27 |   FULL |       DBGW | Arch_aarch64) \
      X(dbgwvr12_el1, 28 |   FULL |       DBGW | Arch_aarch64) \
      X(dbgwvr13_el1, 29 |   FULL |       DBGW | Arch_aarch64) \
      X(dbgwvr14_el1, 30 |   FULL |       DBGW | Arch_aarch64) \
      X(dbgwvr15_el1, 31 |   FULL |       DBGW | Arch_aarch64) \
      X(dczid_el0, 5 |   FULL |      SYSID | Arch_aarch64) \
      X(disr_el1, 0 |   FULL |        RAS | Arch_aarch64) \
      X(dlr_el0, 0 |   FULL |      DBGLR | Arch_aarch64) \
      X(dspsr_el0, 0 |   FULL |    DBGSPSR | Arch_aarch64) \
      X(erridr_el1, 1 |   FULL |        RAS | Arch_aarch64) \
      X(errselr_el1, 2 |   FULL |        RAS | Arch_aarch64) \
      X(erxaddr_el1, 3 |   FULL |        RAS | Arch_aarch64) \
      X(erxctlr_el1, 4 |   FULL |        RAS | Arch_aarch64) \
      X(erxfr_el1, 5 |   FULL |        RAS | Arch_aarch64) \
      X(erxgsr_el1, 6 |   FULL |        RAS | Arch_aarch64) \
      X(erxmisc0_el1, 7 |   FULL |        RAS | Arch_aarch64) \
      X(erxmisc1_el1, 8 |   FULL |        RAS | Arch_aarch64) \
      X(erxmisc2_el1, 9 |   FULL |        RAS | Arch_aarch64) \
      X(erxmisc3_el1, 10 |   FULL |        RAS | Arch_aarch64) \
      X(erxpfgcdn_el1, 11 |   FULL |        RAS | Arch_aarch64) \
      X(erxpfgctl_el1, 12 |   FULL |        RAS | Arch_aarch64) \
      X(erxpfgf_el1, 13 |   FULL |        RAS | Arch_aarch64) \
      X(erxstatus_el1, 14 |   FULL |        RAS | Arch_aarch64) \
      X(esr_el1, 0 |   FULL |       EXCP | Arch_aarch64) \
      X(esr_el2, 9 |   FULL |       VIRT | Arch_aarch64) \
      X(esr_el3, 1 |   FULL |       EXCP | Arch_aarch64) \
      X(far_el1, 2 |   FULL |       EXCP | Arch_aarch64) \
      X(far_el2, 10 |   FULL |       VIRT | Arch_aarch64) \
      X(far_el3, 3 |   FULL |       EXCP | Arch_aarch64) \
      X(fgwte3_el3, 0 |   FULL |       FWTE | Arch_aarch64) \
      X(fpexc32_el2, 0 |   FULL |   SYSFLOAT | Arch_aarch64) \
      X(fpmr, 1 |   FULL |   SYSFLOAT | Arch_aarch64) \
      X(fpsr, 2 |   FULL |   SYSFLOAT | Arch_aarch64) \
      X(gcr_el1, 0 |   FULL |     SYSCTL | Arch_aarch64) \
      X(gcscr_el1, 0 |   FULL |       GCSR | Arch_aarch64) \
      X(gcscr_el2, 1 |   FULL |       GCSR | Arch_aarch64) \
      X(gcscr_el3, 2 |   FULL |       GCSR | Arch_aarch64) \
      X(gcscre0_el1, 3 |   FULL |       GCSR | Arch_aarch64) \
      X(gcspr_el0, 4 |   FULL |       GCSR | Arch_aarch64) \
      X(gcspr_el1, 5 |   FULL |       GCSR | Arch_aarch64) \
      X(gcspr_el2, 6 |   FULL |       GCSR | Arch_aarch64) \
      X(gcspr_el3, 7 |   FULL |       GCSR | Arch_aarch64) \
      X(gmid_el1, 6 |   FULL |      SYSID | Arch_aarch64) \
      X(hacr_el2, 10 |   FULL |    IMPLDEF | Arch_aarch64) \
      X(hafgrtr_el2, 0 |   FULL |    HYPRDBG | Arch_aarch64) \
      X(hcr_el2, 11 |   FULL |       VIRT | Arch_aarch64) \
      X(hcrx_el2, 12 |   FULL |       VIRT | Arch_aarch64) \
      X(hdfgrtr2_el2, 1 |   FULL |    HYPRDBG | Arch_aarch64) \
      X(hdfgrtr_el2, 2 |   FULL |    HYPRDBG | Arch_aarch64) \
      X(hdfgwtr2_el2, 3 |   FULL |    HYPRDBG | Arch_aarch64) \
      X(hdfgwtr_el2, 4 |   FULL |    HYPRDBG | Arch_aarch64) \
      X(hfgitr2_el2, 13 |   FULL |       VIRT | Arch_aarch64) \
      X(hfgitr_el2, 5 |   FULL |    HYPRDBG | Arch_aarch64) \
      X(hfgrtr2_el2, 6 |   FULL |    HYPRDBG | Arch_aarch64) \
      X(hfgrtr_el2, 7 |   FULL |    HYPRDBG | Arch_aarch64) \
      X(hfgwtr2_el2, 8 |   FULL |    HYPRDBG | Arch_aarch64) \
      X(hfgwtr_el2, 9 |   FULL |    HYPRDBG | Arch_aarch64) \
      X(hpfar_el2, 14 |   FULL |       VIRT | Arch_aarch64) \
      X(hstr_el2, 15 |   FULL |       VIRT | Arch_aarch64) \
      X(ich_ap0r0_el2, 16 |   FULL |       VIRT | Arch_aarch64) \
      X(ich_ap0r1_el2, 17 |   FULL |       VIRT | Arch_aarch64) \
      X(ich_ap0r2_el2, 18 |   FULL |       VIRT | Arch_aarch64) \
      X(ich_ap0r3_el2, 19 |   FULL |       VIRT | Arch_aarch64) \
      X(ich_ap1r0_el2, 20 |   FULL |       VIRT | Arch_aarch64) \
      X(ich_ap1r1_el2, 21 |   FULL |       VIRT | Arch_aarch64) \
      X(ich_ap1r2_el2, 22 |   FULL |       VIRT | Arch_aarch64) \
      X(ich_ap1r3_el2, 23 |   FULL |       VIRT | Arch_aarch64) \
      X(ich_eisr_el2, 24 |   FULL |       VIRT | Arch_aarch64) \
      X(ich_elrsr_el2, 25 |   FULL |       VIRT | Arch_aarch64) \
      X(ich_hcr_el2, 26 |   FULL |       VIRT | Arch_aarch64) \
      X(ich_lr0_el2, 27 |   FULL |       VIRT | Arch_aarch64) \
      X(ich_lr1_el2, 28 |   FULL |       VIRT | Arch_aarch64) \
      X(ich_lr2_el2, 29 |   FULL |       VIRT | Arch_aarch64) \
      X(ich_lr3_el2, 30 |   FULL |       VIRT | Arch_aarch64) \
      X(ich_lr4_el2, 31 |   FULL |       VIRT | Arch_aarch64) \
      X(ich_lr5_el2, 32 |   FULL |       VIRT | Arch_aarch64) \
      X(ich_lr6_el2, 33 |   FULL |       VIRT | Arch_aarch64) \
      X(ich_lr7_el2, 34 |   FULL |       VIRT | Arch_aarch64) \
      X(ich_lr8_el2, 35 |   FULL |       VIRT | Arch_aarch64) \
      X(ich_lr9_el2, 36 |   FULL |       VIRT | Arch_aarch64) \
      X(ich_lr10_el2, 37 |   FULL |       VIRT | Arch_aarch64) \
      X(ich_lr11_el2, 38 |   FULL |       VIRT | Arch_aarch64) \
      X(ich_lr12_el2, 39 |   FULL |       VIRT | Arch_aarch64) \
      X(ich_lr13_el2, 40 |   FULL |       VIRT | Arch_aarch64) \
      X(ich_lr14_el2, 41 |   FULL |       VIRT | Arch_aarch64) \
      X(ich_lr15_el2, 42 |   FULL |       VIRT | Arch_aarch64) \
      X(ich_misr_el2, 43 |   FULL |       VIRT | Arch_aarch64) \
      X(ich_vmcr_el2, 44 |   FULL |       VIRT | Arch_aarch64) \
      X(ich_vtr_el2, 45 |   FULL |       VIRT | Arch_aarch64) \
      X(id_aa64afr0_el1, 7 |   FULL |      SYSID | Arch_aarch64) \
      X(id_aa64afr1_el1, 8 |   FULL |      SYSID | Arch_aarch64) \
      X(id_aa64dfr0_el1, 9 |   FULL |      SYSID | Arch_aarch64) \
      X(id_aa64dfr1_el1, 10 |   FULL |      SYSID | Arch_aarch64) \
      X(id_aa64dfr2_el1, 11 |   FULL |      SYSID | Arch_aarch64) \
      X(id_aa64fpfr0_el1, 12 |   FULL |      SYSID | Arch_aarch64) \
      X(id_aa64isar0_el1, 13 |   FULL |      SYSID | Arch_aarch64) \
      X(id_aa64isar1_el1, 14 |   FULL |      SYSID | Arch_aarch64) \
      X(id_aa64isar2_el1, 15 |   FULL |      SYSID | Arch_aarch64) \
      X(id_aa64isar3_el1, 16 |   FULL |      SYSID | Arch_aarch64) \
      X(id_aa64mmfr0_el1, 17 |   FULL |      SYSID | Arch_aarch64) \
      X(id_aa64mmfr1_el1, 18 |   FULL |      SYSID | Arch_aarch64) \
      X(id_aa64mmfr2_el1, 19 |   FULL |      SYSID | Arch_aarch64) \
      X(id_aa64mmfr3_el1, 20 |   FULL |      SYSID | Arch_aarch64) \
      X(id_aa64mmfr4_el1, 21 |   FULL |      SYSID | Arch_aarch64) \
      X(id_aa64pfr0_el1, 22 |   FULL |      SYSID | Arch_aarch64) \
      X(id_aa64pfr1_el1, 23 |   FULL |      SYSID | Arch_aarch64) \
      X(id_aa64pfr2_el1, 24 |   FULL |      SYSID | Arch_aarch64) \
      X(id_aa64smfr0_el1, 25 |   FULL |      SYSID | Arch_aarch64) \
      X(id_aa64zfr0_el1, 26 |   FULL |      SYSID | Arch_aarch64) \
      X(id_afr0_el1, 27 |   FULL |      SYSID | Arch_aarch64) \
      X(id_dfr0_el1, 28 |   FULL |      SYSID | Arch_aarch64) \
      X(id_dfr1_el1, 29 |   FULL |      SYSID | Arch_aarch64) \
      X(id_isar0_el1, 30 |   FULL |      SYSID | Arch_aarch64) \
      X(id_isar1_el1, 31 |   FULL |      SYSID | Arch_aarch64) \
      X(id_isar2_el1, 32 |   FULL |      SYSID | Arch_aarch64) \
      X(id_isar3_el1, 33 |   FULL |      SYSID | Arch_aarch64) \
      X(id_isar4_el1, 34 |   FULL |      SYSID | Arch_aarch64) \
      X(id_isar5_el1, 35 |   FULL |      SYSID | Arch_aarch64) \
      X(id_isar6_el1, 36 |   FULL |      SYSID | Arch_aarch64) \
      X(id_mmfr0_el1, 37 |   FULL |      SYSID | Arch_aarch64) \
      X(id_mmfr1_el1, 38 |   FULL |      SYSID | Arch_aarch64) \
      X(id_mmfr2_el1, 39 |   FULL |      SYSID | Arch_aarch64) \
      X(id_mmfr3_el1, 40 |   FULL |      SYSID | Arch_aarch64) \
      X(id_mmfr4_el1, 41 |   FULL |      SYSID | Arch_aarch64) \
      X(id_mmfr5_el1, 42 |   FULL |      SYSID | Arch_aarch64) \
      X(id_pfr0_el1, 43 |   FULL |      SYSID | Arch_aarch64) \
      X(id_pfr1_el1, 44 |   FULL |      SYSID | Arch_aarch64) \
      X(id_pfr2_el1, 45 |   FULL |      SYSID | Arch_aarch64) \
      X(ifsr32_el2, 4 |   FULL |       EXCP | Arch_aarch64) \
      X(isr_el1, 5 |   FULL |       EXCP | Arch_aarch64) \
      X(mdccint_el1, 0 |   FULL |     SYSMON | Arch_aarch64) \
      X(mdccsr_el0, 1 |   FULL |     SYSMON | Arch_aarch64) \
      X(mdcr_el2, 46 |   FULL |       VIRT | Arch_aarch64) \
      X(mdcr_el3, 1 |   FULL |        SEC | Arch_aarch64) \
      X(mdrar_el1, 2 |   FULL |     SYSMON | Arch_aarch64) \
      X(mdscr_el1, 3 |   FULL |     SYSMON | Arch_aarch64) \
      X(mdselr_el1, 0 |   FULL |       BAWS | Arch_aarch64) \
      X(mdstepop_el1, 4 |   FULL |     SYSMON | Arch_aarch64) \
      X(mecid_a0_el2, 1 |   FULL |     SYSCTL | Arch_aarch64) \
      X(mecid_a1_el2, 2 |   FULL |     SYSCTL | Arch_aarch64) \
      X(mecid_p0_el2, 3 |   FULL |     SYSCTL | Arch_aarch64) \
      X(mecid_p1_el2, 4 |   FULL |     SYSCTL | Arch_aarch64) \
      X(mecid_rl_a_el3, 5 |   FULL |     SYSCTL | Arch_aarch64) \
      X(mecidr_el2, 6 |   FULL |     SYSCTL | Arch_aarch64) \
      X(mfar_el3, 15 |   FULL |        RAS | Arch_aarch64) \
      X(midr_el1, 46 |   FULL |      SYSID | Arch_aarch64) \
      X(mpam0_el1, 0 |   FULL |       MPAM | Arch_aarch64) \
      X(mpam1_el1, 1 |   FULL |       MPAM | Arch_aarch64) \
      X(mpam2_el2, 2 |   FULL |       MPAM | Arch_aarch64) \
      X(mpam3_el3, 3 |   FULL |       MPAM | Arch_aarch64) \
      X(mpamhcr_el2, 4 |   FULL |       MPAM | Arch_aarch64) \
      X(mpamidr_el1, 47 |   FULL |      SYSID | Arch_aarch64) \
      X(mpamsm_el1, 5 |   FULL |       MPAM | Arch_aarch64) \
      X(mpamvpm0_el2, 6 |   FULL |       MPAM | Arch_aarch64) \
      X(mpamvpm1_el2, 7 |   FULL |       MPAM | Arch_aarch64) \
      X(mpamvpm2_el2, 8 |   FULL |       MPAM | Arch_aarch64) \
      X(mpamvpm3_el2, 9 |   FULL |       MPAM | Arch_aarch64) \
      X(mpamvpm4_el2, 10 |   FULL |       MPAM | Arch_aarch64) \
      X(mpamvpm5_el2, 11 |   FULL |       MPAM | Arch_aarch64) \
      X(mpamvpm6_el2, 12 |   FULL |       MPAM | Arch_aarch64) \
      X(mpamvpm7_el2, 13 |   FULL |       MPAM | Arch_aarch64) \
      X(mpamvpmv_el2, 14 |   FULL |       MPAM | Arch_aarch64) \
      X(mpidr_el1, 48 |   FULL |      SYSID | Arch_aarch64) \
      X(mvfr0_el1, 3 |   FULL |   SYSFLOAT | Arch_aarch64) \
      X(mvfr1_el1, 4 |   FULL |   SYSFLOAT | Arch_aarch64) \
      X(mvfr2_el1, 5 |   FULL |   SYSFLOAT | Arch_aarch64) \
      X(osdlr_el1, 0 |   FULL |      SYSOS | Arch_aarch64) \
      X(osdtrrx_el1, 1 |   FULL |      SYSOS | Arch_aarch64) \
      X(osdtrtx_el1, 2 |   FULL |      SYSOS | Arch_aarch64) \
      X(oseccr_el1, 3 |   FULL |      SYSOS | Arch_aarch64) \
      X(oslar_el1, 4 |  D_REG |      SYSOS | Arch_aarch64) \
      X(oslsr_el1, 5 |   FULL |      SYSOS | Arch_aarch64) \
      X(par_el1, 0 |  Q_REG |       ADDR | Arch_aarch64) \
      X(pfar_el1, 0 |   FULL |    PHYSFAR | Arch_aarch64) \
      X(pfar_el2, 1 |   FULL |    PHYSFAR | Arch_aarch64) \
      X(pmbidr_el1, 0 |   FULL |   STATPROF | Arch_aarch64) \
      X(pmblimitr_el1, 1 |   FULL |   STATPROF | Arch_aarch64) \
      X(pmbptr_el1, 2 |   FULL |   STATPROF | Arch_aarch64) \
      X(pmbsr_el1, 3 |   FULL |   STATPROF | Arch_aarch64) \
      X(pmccfiltr_el0, 0 |   FULL |        PMU | Arch_aarch64) \
      X(pmccntr_el0, 1 |   FULL |        PMU | Arch_aarch64) \
      X(pmccntsvr_el1, 2 |   FULL |        PMU | Arch_aarch64) \
      X(pmceid0_el0, 3 |   FULL |        PMU | Arch_aarch64) \
      X(pmceid1_el0, 4 |   FULL |        PMU | Arch_aarch64) \
      X(pmcntenclr_el0, 5 |   FULL |        PMU | Arch_aarch64) \
      X(pmcntenset_el0, 6 |   FULL |        PMU | Arch_aarch64) \
      X(pmcr_el0, 7 |   FULL |        PMU | Arch_aarch64) \
      X(pmecr_el1, 8 |   FULL |        PMU | Arch_aarch64) \
      X(pmevcntr0_el0, 9 |   FULL |        PMU | Arch_aarch64) \
      X(pmevcntr1_el0, 10 |   FULL |        PMU | Arch_aarch64) \
      X(pmevcntr2_el0, 11 |   FULL |        PMU | Arch_aarch64) \
      X(pmevcntr3_el0, 12 |   FULL |        PMU | Arch_aarch64) \
      X(pmevcntr4_el0, 13 |   FULL |        PMU | Arch_aarch64) \
      X(pmevcntr5_el0, 14 |   FULL |        PMU | Arch_aarch64) \
      X(pmevcntr6_el0, 15 |   FULL |        PMU | Arch_aarch64) \
      X(pmevcntr7_el0, 16 |   FULL |        PMU | Arch_aarch64) \
      X(pmevcntr8_el0, 17 |   FULL |        PMU | Arch_aarch64) \
      X(pmevcntr9_el0, 18 |   FULL |        PMU | Arch_aarch64) \
      X(pmevcntr10_el0, 19 |   FULL |        PMU | Arch_aarch64) \
      X(pmevcntr11_el0, 20 |   FULL |        PMU | Arch_aarch64) \
      X(pmevcntr12_el0, 21 |   FULL |        PMU | Arch_aarch64) \
      X(pmevcntr13_el0, 22 |   FULL |        PMU | Arch_aarch64) \
      X(pmevcntr14_el0, 23 |   FULL |        PMU | Arch_aarch64) \
      X(pmevcntr15_el0, 24 |   FULL |        PMU | Arch_aarch64) \
      X(pmevcntr16_el0, 25 |   FULL |        PMU | Arch_aarch64) \
      X(pmevcntr17_el0, 26 |   FULL |        PMU | Arch_aarch64) \
      X(pmevcntr18_el0, 27 |   FULL |        PMU | Arch_aarch64) \
      X(pmevcntr19_el0, 28 |   FULL |        PMU | Arch_aarch64) \
      X(pmevcntr20_el0, 29 |   FULL |        PMU | Arch_aarch64) \
      X(pmevcntr21_el0, 30 |   FULL |        PMU | Arch_aarch64) \
      X(pmevcntr22_el0, 31 |   FULL |        PMU | Arch_aarch64) \
      X(pmevcntr23_el0, 32 |   FULL |        PMU | Arch_aarch64) \
      X(pmevcntr24_el0, 33 |   FULL |        PMU | Arch_aarch64) \
      X(pmevcntr25_el0, 34 |   FULL |        PMU | Arch_aarch64) \
      X(pmevcntr26_el0, 35 |   FULL |        PMU | Arch_aarch64) \
      X(pmevcntr27_el0, 36 |   FULL |        PMU | Arch_aarch64) \
      X(pmevcntr28_el0, 37 |   FULL |        PMU | Arch_aarch64) \
      X(pmevcntr29_el0, 38 |   FULL |        PMU | Arch_aarch64) \
      X(pmevcntr30_el0, 39 |   FULL |        PMU | Arch_aarch64) \
      X(pmevcntsvr0_el1, 40 |   FULL |        PMU | Arch_aarch64) \
      X(pmevcntsvr1_el1, 41 |   FULL |        PMU | Arch_aarch64) \
      X(pmevcntsvr2_el1, 42 |   FULL |        PMU | Arch_aarch64) \
      X(pmevcntsvr3_el1, 43 |   FULL |        PMU | Arch_aarch64) \
      X(pmevcntsvr4_el1, 44 |   FULL |        PMU | Arch_aarch64) \
      X(pmevcntsvr5_el1, 45 |   FULL |        PMU | Arch_aarch64) \
      X(pmevcntsvr6_el1, 46 |   FULL |        PMU | Arch_aarch64) \
      X(pmevcntsvr7_el1, 47 |   FULL |        PMU | Arch_aarch64) \
      X(pmevcntsvr8_el1, 48 |   FULL |        PMU | Arch_aarch64) \
      X(pmevcntsvr9_el1, 49 |   FULL |        PMU | Arch_aarch64) \
      X(pmevcntsvr10_el1, 50 |   FULL |        PMU | Arch_aarch64) \
      X(pmevcntsvr11_el1, 51 |   FULL |        PMU | Arch_aarch64) \
      X(pmevcntsvr12_el1, 52 |   FULL |        PMU | Arch_aarch64) \
      X(pmevcntsvr13_el1, 53 |   FULL |        PMU | Arch_aarch64) \
      X(pmevcntsvr14_el1, 54 |   FULL |        PMU | Arch_aarch64) \
      X(pmevcntsvr15_el1, 55 |   FULL |        PMU | Arch_aarch64) \
      X(pmevcntsvr16_el1, 56 |   FULL |        PMU | Arch_aarch64) \
      X(pmevcntsvr17_el1, 57 |   FULL |        PMU | Arch_aarch64) \
      X(pmevcntsvr18_el1, 58 |   FULL |        PMU | Arch_aarch64) \
      X(pmevcntsvr19_el1, 59 |   FULL |        PMU | Arch_aarch64) \
      X(pmevcntsvr20_el1, 60 |   FULL |        PMU | Arch_aarch64) \
      X(pmevcntsvr21_el1, 61 |   FULL |        PMU | Arch_aarch64) \
      X(pmevcntsvr22_el1, 62 |   FULL |        PMU | Arch_aarch64) \
      X(pmevcntsvr23_el1, 63 |   FULL |        PMU | Arch_aarch64) \
      X(pmevcntsvr24_el1, 64 |   FULL |        PMU | Arch_aarch64) \
      X(pmevcntsvr25_el1, 65 |   FULL |        PMU | Arch_aarch64) \
      X(pmevcntsvr26_el1, 66 |   FULL |        PMU | Arch_aarch64) \
      X(pmevcntsvr27_el1, 67 |   FULL |        PMU | Arch_aarch64) \
      X(pmevcntsvr28_el1, 68 |   FULL |        PMU | Arch_aarch64) \
      X(pmevcntsvr29_el1, 69 |   FULL |        PMU | Arch_aarch64) \
      X(pmevcntsvr30_el1, 70 |   FULL |        PMU | Arch_aarch64) \
      X(pmevtyper0_el0, 71 |   FULL |        PMU | Arch_aarch64) \
      X(pmevtyper1_el0, 72 |   FULL |        PMU | Arch_aarch64) \
      X(pmevtyper2_el0, 73 |   FULL |        PMU | Arch_aarch64) \
      X(pmevtyper3_el0, 74 |   FULL |        PMU | Arch_aarch64) \
      X(pmevtyper4_el0, 75 |   FULL |        PMU | Arch_aarch64) \
      X(pmevtyper5_el0, 76 |   FULL |        PMU | Arch_aarch64) \
      X(pmevtyper6_el0, 77 |   FULL |        PMU | Arch_aarch64) \
      X(pmevtyper7_el0, 78 |   FULL |        PMU | Arch_aarch64) \
      X(pmevtyper8_el0, 79 |   FULL |        PMU | Arch_aarch64) \
      X(pmevtyper9_el0, 80 |   FULL |        PMU | Arch_aarch64) \
      X(pmevtyper10_el0, 81 |   FULL |        PMU | Arch_aarch64) \
      X(pmevtyper11_el0, 82 |   FULL |        PMU | Arch_aarch64) \
      X(pmevtyper12_el0, 83 |   FULL |        PMU | Arch_aarch64) \
      X(pmevtyper13_el0, 84 |   FULL |        PMU | Arch_aarch64) \
      X(pmevtyper14_el0, 85 |   FULL |        PMU | Arch_aarch64) \
      X(pmevtyper15_el0, 86 |   FULL |        PMU | Arch_aarch64) \
      X(pmevtyper16_el0, 87 |   FULL |        PMU | Arch_aarch64) \
      X(pmevtyper17_el0, 88 |   FULL |        PMU | Arch_aarch64) \
      X(pmevtyper18_el0, 89 |   FULL |        PMU | Arch_aarch64) \
      X(pmevtyper19_el0, 90 |   FULL |        PMU | Arch_aarch64) \
      X(pmevtyper20_el0, 91 |   FULL |        PMU | Arch_aarch64) \
      X(pmevtyper21_el0, 92 |   FULL |        PMU | Arch_aarch64) \
      X(pmevtyper22_el0, 93 |   FULL |        PMU | Arch_aarch64) \
      X(pmevtyper23_el0, 94 |   FULL |        PMU | Arch_aarch64) \
      X(pmevtyper24_el0, 95 |   FULL |        PMU | Arch_aarch64) \
      X(pmevtyper25_el0, 96 |   FULL |        PMU | Arch_aarch64) \
      X(pmevtyper26_el0, 97 |   FULL |        PMU | Arch_aarch64) \
      X(pmevtyper27_el0, 98 |   FULL |        PMU | Arch_aarch64) \
      X(pmevtyper28_el0, 99 |   FULL |        PMU | Arch_aarch64) \
      X(pmevtyper29_el0, 100 |   FULL |        PMU | Arch_aarch64) \
      X(pmevtyper30_el0, 101 |   FULL |        PMU | Arch_aarch64) \
      X(pmiar_el1, 102 |   FULL |        PMU | Arch_aarch64) \
      X(pmicfiltr_el0, 103 |   FULL |        PMU | Arch_aarch64) \
      X(pmicntr_el0, 104 |   FULL |        PMU | Arch_aarch64) \
      X(pmicntsvr_el1, 105 |   FULL |        PMU | Arch_aarch64) \
      X(pmintenclr_el1, 106 |   FULL |        PMU | Arch_aarch64) \
      X(pmintenset_el1, 107 |   FULL |        PMU | Arch_aarch64) \
      X(pmmir_el1, 108 |   FULL |        PMU | Arch_aarch64) \
      X(pmovsclr_el0, 109 |   FULL |        PMU | Arch_aarch64) \
      X(pmovsset_el0, 110 |   FULL |        PMU | Arch_aarch64) \
      X(pmscr_el1, 4 |   FULL |   STATPROF | Arch_aarch64) \
      X(pmscr_el2, 5 |   FULL |   STATPROF | Arch_aarch64) \
      X(pmsdsfr_el1, 6 |   FULL |   STATPROF | Arch_aarch64) \
      X(pmselr_el0, 111 |   FULL |        PMU | Arch_aarch64) \
      X(pmsevfr_el1, 7 |   FULL |   STATPROF | Arch_aarch64) \
      X(pmsfcr_el1, 8 |   FULL |   STATPROF | Arch_aarch64) \
      X(pmsicr_el1, 9 |   FULL |   STATPROF | Arch_aarch64) \
      X(pmsidr_el1, 10 |   FULL |   STATPROF | Arch_aarch64) \
      X(pmsirr_el1, 11 |   FULL |   STATPROF | Arch_aarch64) \
      X(pmslatfr_el1, 12 |   FULL |   STATPROF | Arch_aarch64) \
      X(pmsnevfr_el1, 13 |   FULL |   STATPROF | Arch_aarch64) \
      X(pmsscr_el1, 112 |   FULL |        PMU | Arch_aarch64) \
      X(pmswinc_el0, 113 |   FULL |        PMU | Arch_aarch64) \
      X(pmuacr_el1, 114 |   FULL |        PMU | Arch_aarch64) \
      X(pmuserenr_el0, 115 |   FULL |        PMU | Arch_aarch64) \
      X(pmxevcntr_el0, 116 |   FULL |        PMU | Arch_aarch64) \
      X(pmxevtyper_el0, 117 |   FULL |        PMU | Arch_aarch64) \
      X(pmzr_el0, 118 |   FULL |        PMU | Arch_aarch64) \
      X(revidr_el1, 49 |   FULL |      SYSID | Arch_aarch64) \
      X(rgsr_el1, 7 |   FULL |     SYSCTL | Arch_aarch64) \
      X(rmr_el1, 0 |   FULL |      RESET | Arch_aarch64) \
      X(rmr_el2, 47 |   FULL |       VIRT | Arch_aarch64) \
      X(rmr_el3, 1 |   FULL |      RESET | Arch_aarch64) \
      X(rndr, 8 |   FULL |     SYSCTL | Arch_aarch64) \
      X(rndrrs, 9 |   FULL |     SYSCTL | Arch_aarch64) \
      X(rvbar_el1, 2 |   FULL |      RESET | Arch_aarch64) \
      X(rvbar_el2, 3 |   FULL |      RESET | Arch_aarch64) \
      X(rvbar_el3, 4 |   FULL |      RESET | Arch_aarch64) \
      X(scr_el3, 2 |   FULL |        SEC | Arch_aarch64) \
      X(sctlr2_el1, 10 |   FULL |     SYSCTL | Arch_aarch64) \
      X(sctlr2_el2, 48 |   FULL |       VIRT | Arch_aarch64) \
      X(sctlr2_el3, 11 |   FULL |     SYSCTL | Arch_aarch64) \
      X(sctlr_el1, 12 |   FULL |     SYSCTL | Arch_aarch64) \
      X(sctlr_el2, 49 |   FULL |       VIRT | Arch_aarch64) \
      X(sctlr_el3, 13 |   FULL |     SYSCTL | Arch_aarch64) \
      X(scxtnum_el0, 0 |   FULL |       THRD | Arch_aarch64) \
      X(scxtnum_el1, 1 |   FULL |       THRD | Arch_aarch64) \
      X(scxtnum_el2, 2 |   FULL |       THRD | Arch_aarch64) \
      X(scxtnum_el3, 3 |   FULL |       THRD | Arch_aarch64) \
      X(sder32_el2, 14 |   FULL |     SYSCTL | Arch_aarch64) \
      X(sder32_el3, 3 |   FULL |        SEC | Arch_aarch64) \
      X(smcr_el1, 1 |   FULL |      OTHER | Arch_aarch64) \
      X(smcr_el2, 2 |   FULL |      OTHER | Arch_aarch64) \
      X(smcr_el3, 3 |   FULL |      OTHER | Arch_aarch64) \
      X(smidr_el1, 50 |   FULL |      SYSID | Arch_aarch64) \
      X(smpri_el1, 4 |   FULL |      OTHER | Arch_aarch64) \
      X(smprimap_el2, 5 |   FULL |      OTHER | Arch_aarch64) \
      X(spmaccessr_el1, 119 |   FULL |        PMU | Arch_aarch64) \
      X(spmaccessr_el2, 120 |   FULL |        PMU | Arch_aarch64) \
      X(spmaccessr_el3, 121 |   FULL |        PMU | Arch_aarch64) \
      X(spmcfgr_el1, 122 |   FULL |        PMU | Arch_aarch64) \
      X(spmcgcr0_el1, 123 |   FULL |        PMU | Arch_aarch64) \
      X(spmcgcr1_el1, 124 |   FULL |        PMU | Arch_aarch64) \
      X(spmcntenclr_el0, 125 |   FULL |        PMU | Arch_aarch64) \
      X(spmcntenset_el0, 126 |   FULL |        PMU | Arch_aarch64) \
      X(spmcr_el0, 127 |   FULL |        PMU | Arch_aarch64) \
      X(spmdevaff_el1, 128 |   FULL |        PMU | Arch_aarch64) \
      X(spmdevarch_el1, 129 |   FULL |        PMU | Arch_aarch64) \
      X(spmevcntr0_el0, 130 |   FULL |        PMU | Arch_aarch64) \
      X(spmevcntr1_el0, 131 |   FULL |        PMU | Arch_aarch64) \
      X(spmevcntr2_el0, 132 |   FULL |        PMU | Arch_aarch64) \
      X(spmevcntr3_el0, 133 |   FULL |        PMU | Arch_aarch64) \
      X(spmevcntr4_el0, 134 |   FULL |        PMU | Arch_aarch64) \
      X(spmevcntr5_el0, 135 |   FULL |        PMU | Arch_aarch64) \
      X(spmevcntr6_el0, 136 |   FULL |        PMU | Arch_aarch64) \
      X(spmevcntr7_el0, 137 |   FULL |        PMU | Arch_aarch64) \
      X(spmevcntr8_el0, 138 |   FULL |        PMU | Arch_aarch64) \
      X(spmevcntr9_el0, 139 |   FULL |        PMU | Arch_aarch64) \
      X(spmevcntr10_el0, 140 |   FULL |        PMU | Arch_aarch64) \
      X(spmevcntr11_el0, 141 |   FULL |        PMU | Arch_aarch64) \
      X(spmevcntr12_el0, 142 |   FULL |        PMU | Arch_aarch64) \
      X(spmevcntr13_el0, 143 |   FULL |        PMU | Arch_aarch64) \
      X(spmevcntr14_el0, 144 |   FULL |        PMU | Arch_aarch64) \
      X(spmevcntr15_el0, 145 |   FULL |        PMU | Arch_aarch64) \
      X(spmevfilt2r0_el0, 146 |   FULL |        PMU | Arch_aarch64) \
      X(spmevfilt2r1_el0, 147 |   FULL |        PMU | Arch_aarch64) \
      X(spmevfilt2r2_el0, 148 |   FULL |        PMU | Arch_aarch64) \
      X(spmevfilt2r3_el0, 149 |   FULL |        PMU | Arch_aarch64) \
      X(spmevfilt2r4_el0, 150 |   FULL |        PMU | Arch_aarch64) \
      X(spmevfilt2r5_el0, 151 |   FULL |        PMU | Arch_aarch64) \
      X(spmevfilt2r6_el0, 152 |   FULL |        PMU | Arch_aarch64) \
      X(spmevfilt2r7_el0, 153 |   FULL |        PMU | Arch_aarch64) \
      X(spmevfilt2r8_el0, 154 |   FULL |        PMU | Arch_aarch64) \
      X(spmevfilt2r9_el0, 155 |   FULL |        PMU | Arch_aarch64) \
      X(spmevfilt2r10_el0, 156 |   FULL |        PMU | Arch_aarch64) \
      X(spmevfilt2r11_el0, 157 |   FULL |        PMU | Arch_aarch64) \
      X(spmevfilt2r12_el0, 158 |   FULL |        PMU | Arch_aarch64) \
      X(spmevfilt2r13_el0, 159 |   FULL |        PMU | Arch_aarch64) \
      X(spmevfilt2r14_el0, 160 |   FULL |        PMU | Arch_aarch64) \
      X(spmevfilt2r15_el0, 161 |   FULL |        PMU | Arch_aarch64) \
      X(spmevfiltr0_el0, 162 |   FULL |        PMU | Arch_aarch64) \
      X(spmevfiltr1_el0, 163 |   FULL |        PMU | Arch_aarch64) \
      X(spmevfiltr2_el0, 164 |   FULL |        PMU | Arch_aarch64) \
      X(spmevfiltr3_el0, 165 |   FULL |        PMU | Arch_aarch64) \
      X(spmevfiltr4_el0, 166 |   FULL |        PMU | Arch_aarch64) \
      X(spmevfiltr5_el0, 167 |   FULL |        PMU | Arch_aarch64) \
      X(spmevfiltr6_el0, 168 |   FULL |        PMU | Arch_aarch64) \
      X(spmevfiltr7_el0, 169 |   FULL |        PMU | Arch_aarch64) \
      X(spmevfiltr8_el0, 170 |   FULL |        PMU | Arch_aarch64) \
      X(spmevfiltr9_el0, 171 |   FULL |        PMU | Arch_aarch64) \
      X(spmevfiltr10_el0, 172 |   FULL |        PMU | Arch_aarch64) \
      X(spmevfiltr11_el0, 173 |   FULL |        PMU | Arch_aarch64) \
      X(spmevfiltr12_el0, 174 |   FULL |        PMU | Arch_aarch64) \
      X(spmevfiltr13_el0, 175 |   FULL |        PMU | Arch_aarch64) \
      X(spmevfiltr14_el0, 176 |   FULL |        PMU | Arch_aarch64) \
      X(spmevfiltr15_el0, 177 |   FULL |        PMU | Arch_aarch64) \
      X(spmevtyper0_el0, 178 |   FULL |        PMU | Arch_aarch64) \
      X(spmevtyper1_el0, 179 |   FULL |        PMU | Arch_aarch64) \
      X(spmevtyper2_el0, 180 |   FULL |        PMU | Arch_aarch64) \
      X(spmevtyper3_el0, 181 |   FULL |        PMU | Arch_aarch64) \
      X(spmevtyper4_el0, 182 |   FULL |        PMU | Arch_aarch64) \
      X(spmevtyper5_el0, 183 |   FULL |        PMU | Arch_aarch64) \
      X(spmevtyper6_el0, 184 |   FULL |        PMU | Arch_aarch64) \
      X(spmevtyper7_el0, 185 |   FULL |        PMU | Arch_aarch64) \
      X(spmevtyper8_el0, 186 |   FULL |        PMU | Arch_aarch64) \
      X(spmevtyper9_el0, 187 |   FULL |        PMU | Arch_aarch64) \
      X(spmevtyper10_el0, 188 |   FULL |        PMU | Arch_aarch64) \
      X(spmevtyper11_el0, 189 |   FULL |        PMU | Arch_aarch64) \
      X(spmevtyper12_el0, 190 |   FULL |        PMU | Arch_aarch64) \
      X(spmevtyper13_el0, 191 |   FULL |        PMU | Arch_aarch64) \
      X(spmevtyper14_el0, 192 |   FULL |        PMU | Arch_aarch64) \
      X(spmevtyper15_el0, 193 |   FULL |        PMU | Arch_aarch64) \
      X(spmiidr_el1, 194 |   FULL |        PMU | Arch_aarch64) \
      X(spmintenclr_el1, 195 |   FULL |        PMU | Arch_aarch64) \
      X(spmintenset_el1, 196 |   FULL |        PMU | Arch_aarch64) \
      X(spmovsclr_el0, 197 |   FULL |        PMU | Arch_aarch64) \
      X(spmovsset_el0, 198 |   FULL |        PMU | Arch_aarch64) \
      X(spmrootcr_el3, 199 |   FULL |        PMU | Arch_aarch64) \
      X(spmscr_el1, 200 |   FULL |        PMU | Arch_aarch64) \
      X(spmselr_el0, 201 |   FULL |        PMU | Arch_aarch64) \
      X(spmzr_el0, 202 |   FULL |        PMU | Arch_aarch64) \
      X(teecr32_el1, 22 |  D_REG |   SYSTIMER | Arch_aarch64) \
      X(teehbr32_el1, 23 |  D_REG |   SYSTIMER | Arch_aarch64) \
      X(tfsr_el1, 15 |   FULL |     SYSCTL | Arch_aarch64) \
      X(tfsr_el2, 16 |   FULL |     SYSCTL | Arch_aarch64) \
      X(tfsr_el3, 17 |   FULL |     SYSCTL | Arch_aarch64) \
      X(tfsre0_el1, 18 |   FULL |     SYSCTL | Arch_aarch64) \
      X(tpidr2_el0, 4 |   FULL |       THRD | Arch_aarch64) \
      X(tpidr_el0, 5 |   FULL |       THRD | Arch_aarch64) \
      X(tpidr_el1, 6 |   FULL |       THRD | Arch_aarch64) \
      X(tpidr_el2, 50 |   FULL |       VIRT | Arch_aarch64) \
      X(tpidr_el3, 7 |   FULL |       THRD | Arch_aarch64) \
      X(tpidrro_el0, 8 |   FULL |       THRD | Arch_aarch64) \
      X(trbbaser_el1, 0 |   FULL |      TRACE | Arch_aarch64) \
      X(trbidr_el1, 1 |   FULL |      TRACE | Arch_aarch64) \
      X(trblimitr_el1, 2 |   FULL |      TRACE | Arch_aarch64) \
      X(trbmar_el1, 3 |   FULL |      TRACE | Arch_aarch64) \
      X(trbptr_el1, 4 |   FULL |      TRACE | Arch_aarch64) \
      X(trbsr_el1, 5 |   FULL |      TRACE | Arch_aarch64) \
      X(trbtrg_el1, 6 |   FULL |      TRACE | Arch_aarch64) \
      X(trcacatr0, 7 |   FULL |      TRACE | Arch_aarch64) \
      X(trcacatr1, 8 |   FULL |      TRACE | Arch_aarch64) \
      X(trcacatr2, 9 |   FULL |      TRACE | Arch_aarch64) \
      X(trcacatr3, 10 |   FULL |      TRACE | Arch_aarch64) \
      X(trcacatr4, 11 |   FULL |      TRACE | Arch_aarch64) \
      X(trcacatr5, 12 |   FULL |      TRACE | Arch_aarch64) \
      X(trcacatr6, 13 |   FULL |      TRACE | Arch_aarch64) \
      X(trcacatr7, 14 |   FULL |      TRACE | Arch_aarch64) \
      X(trcacatr8, 15 |   FULL |      TRACE | Arch_aarch64) \
      X(trcacatr9, 16 |   FULL |      TRACE | Arch_aarch64) \
      X(trcacatr10, 17 |   FULL |      TRACE | Arch_aarch64) \
      X(trcacatr11, 18 |   FULL |      TRACE | Arch_aarch64) \
      X(trcacatr12, 19 |   FULL |      TRACE | Arch_aarch64) \
      X(trcacatr13, 20 |   FULL |      TRACE | Arch_aarch64) \
      X(trcacatr14, 21 |   FULL |      TRACE | Arch_aarch64) \
      X(trcacatr15, 22 |   FULL |      TRACE | Arch_aarch64) \
      X(trcacvr0, 23 |   FULL |      TRACE | Arch_aarch64) \
      X(trcacvr1, 24 |   FULL |      TRACE | Arch_aarch64) \
      X(trcacvr2, 25 |   FULL |      TRACE | Arch_aarch64) \
      X(trcacvr3, 26 |   FULL |      TRACE | Arch_aarch64) \
      X(trcacvr4, 27 |   FULL |      TRACE | Arch_aarch64) \
      X(trcacvr5, 28 |   FULL |      TRACE | Arch_aarch64) \
      X(trcacvr6, 29 |   FULL |      TRACE | Arch_aarch64) \
      X(trcacvr7, 30 |   FULL |      TRACE | Arch_aarch64) \
      X(trcacvr8, 31 |   FULL |      TRACE | Arch_aarch64) \
      X(trcacvr9, 32 |   FULL |      TRACE | Arch_aarch64) \
      X(trcacvr10, 33 |   FULL |      TRACE | Arch_aarch64) \
      X(trcacvr11, 34 |   FULL |      TRACE | Arch_aarch64) \
      X(trcacvr12, 35 |   FULL |      TRACE | Arch_aarch64) \
      X(trcacvr13, 36 |   FULL |      TRACE | Arch_aarch64) \
      X(trcacvr14, 37 |   FULL |      TRACE | Arch_aarch64) \
      X(trcacvr15, 38 |   FULL |      TRACE | Arch_aarch64) \
      X(trcauthstatus, 39 |  D_REG |      TRACE | Arch_aarch64) \
      X(trcauxctlr, 40 |  D_REG |      TRACE | Arch_aarch64) \
      X(trcbbctlr, 41 |  D_REG |      TRACE | Arch_aarch64) \
      X(trcccctlr, 42 |  D_REG |      TRACE | Arch_aarch64) \
      X(trccidcctlr0, 43 |  D_REG |      TRACE | Arch_aarch64) \
      X(trccidcctlr1, 44 |  D_REG |      TRACE | Arch_aarch64) \
      X(trccidcvr0, 45 |   FULL |      TRACE | Arch_aarch64) \
      X(trccidcvr1, 46 |   FULL |      TRACE | Arch_aarch64) \
      X(trccidcvr2, 47 |   FULL |      TRACE | Arch_aarch64) \
      X(trccidcvr3, 48 |   FULL |      TRACE | Arch_aarch64) \
      X(trccidcvr4, 49 |   FULL |      TRACE | Arch_aarch64) \
      X(trccidcvr5, 50 |   FULL |      TRACE | Arch_aarch64) \
      X(trccidcvr6, 51 |   FULL |      TRACE | Arch_aarch64) \
      X(trccidcvr7, 52 |   FULL |      TRACE | Arch_aarch64) \
      X(trccidr0, 53 |  D_REG |      TRACE | Arch_aarch64) \
      X(trccidr1, 54 |  D_REG |      TRACE | Arch_aarch64) \
      X(trccidr2, 55 |  D_REG |      TRACE | Arch_aarch64) \
      X(trccidr3, 56 |  D_REG |      TRACE | Arch_aarch64) \
      X(trcclaimclr, 57 |  D_REG |      TRACE | Arch_aarch64) \
      X(trcclaimset, 58 |  D_REG |      TRACE | Arch_aarch64) \
      X(trccntctlr0, 59 |  D_REG |      TRACE | Arch_aarch64) \
      X(trccntctlr1, 60 |  D_REG |      TRACE | Arch_aarch64) \
      X(trccntctlr2, 61 |  D_REG |      TRACE | Arch_aarch64) \
      X(trccntctlr3, 62 |  D_REG |      TRACE | Arch_aarch64) \
      X(trccntrldvr0, 63 |   FULL |      TRACE | Arch_aarch64) \
      X(trccntrldvr1, 64 |   FULL |      TRACE | Arch_aarch64) \
      X(trccntrldvr2, 65 |   FULL |      TRACE | Arch_aarch64) \
      X(trccntrldvr3, 66 |   FULL |      TRACE | Arch_aarch64) \
      X(trccntvr0, 67 |  D_REG |      TRACE | Arch_aarch64) \
      X(trccntvr1, 68 |  D_REG |      TRACE | Arch_aarch64) \
      X(trccntvr2, 69 |  D_REG |      TRACE | Arch_aarch64) \
      X(trccntvr3, 70 |  D_REG |      TRACE | Arch_aarch64) \
      X(trcconfigr, 71 |  D_REG |      TRACE | Arch_aarch64) \
      X(trcdevaff0, 72 |   FULL |      TRACE | Arch_aarch64) \
      X(trcdevaff1, 73 |   FULL |      TRACE | Arch_aarch64) \
      X(trcdevarch, 74 |  D_REG |      TRACE | Arch_aarch64) \
      X(trcdevid, 75 |  D_REG |      TRACE | Arch_aarch64) \
      X(trcdevtype, 76 |  D_REG |      TRACE | Arch_aarch64) \
      X(trcdvcmr0, 77 |   FULL |      TRACE | Arch_aarch64) \
      X(trcdvcmr1, 78 |   FULL |      TRACE | Arch_aarch64) \
      X(trcdvcmr2, 79 |   FULL |      TRACE | Arch_aarch64) \
      X(trcdvcmr3, 80 |   FULL |      TRACE | Arch_aarch64) \
      X(trcdvcmr4, 81 |   FULL |      TRACE | Arch_aarch64) \
      X(trcdvcmr5, 82 |   FULL |      TRACE | Arch_aarch64) \
      X(trcdvcmr6, 83 |   FULL |      TRACE | Arch_aarch64) \
      X(trcdvcmr7, 84 |   FULL |      TRACE | Arch_aarch64) \
      X(trcdvcvr0, 85 |   FULL |      TRACE | Arch_aarch64) \
      X(trcdvcvr1, 86 |   FULL |      TRACE | Arch_aarch64) \
      X(trcdvcvr2, 87 |   FULL |      TRACE | Arch_aarch64) \
      X(trcdvcvr3, 88 |   FULL |      TRACE | Arch_aarch64) \
      X(trcdvcvr4, 89 |   FULL |      TRACE | Arch_aarch64) \
      X(trcdvcvr5, 90 |   FULL |      TRACE | Arch_aarch64) \
      X(trcdvcvr6, 91 |   FULL |      TRACE | Arch_aarch64) \
      X(trcdvcvr7, 92 |   FULL |      TRACE | Arch_aarch64) \
      X(trceventctl0r, 93 |  D_REG |      TRACE | Arch_aarch64) \
      X(trceventctl1r, 94 |  D_REG |      TRACE | Arch_aarch64) \
      X(trcextinselr, 95 |   FULL |      TRACE | Arch_aarch64) \
      X(trcextinselr0, 96 |   FULL |      TRACE | Arch_aarch64) \
      X(trcextinselr1, 97 |   FULL |      TRACE | Arch_aarch64) \
      X(trcextinselr2, 98 |   FULL |      TRACE | Arch_aarch64) \
      X(trcextinselr3, 99 |   FULL |      TRACE | Arch_aarch64) \
      X(trcidr0, 100 |  D_REG |      TRACE | Arch_aarch64) \
      X(trcidr1, 101 |   FULL |      TRACE | Arch_aarch64) \
      X(trcidr2, 102 |   FULL |      TRACE | Arch_aarch64) \
      X(trcidr3, 103 |   FULL |      TRACE | Arch_aarch64) \
      X(trcidr4, 104 |   FULL |      TRACE | Arch_aarch64) \
      X(trcidr5, 105 |   FULL |      TRACE | Arch_aarch64) \
      X(trcidr6, 106 |  D_REG |      TRACE | Arch_aarch64) \
      X(trcidr7, 107 |   FULL |      TRACE | Arch_aarch64) \
      X(trcidr8, 108 |  D_REG |      TRACE | Arch_aarch64) \
      X(trcidr9, 109 |   FULL |      TRACE | Arch_aarch64) \
      X(trcidr10, 110 |   FULL |      TRACE | Arch_aarch64) \
      X(trcidr11, 111 |   FULL |      TRACE | Arch_aarch64) \
      X(trcidr12, 112 |  D_REG |      TRACE | Arch_aarch64) \
      X(trcidr13, 113 |   FULL |      TRACE | Arch_aarch64) \
      X(trcimspec0, 114 |   FULL |      TRACE | Arch_aarch64) \
      X(trcimspec1, 115 |  D_REG |      TRACE | Arch_aarch64) \
      X(trcimspec2, 116 |  D_REG |      TRACE | Arch_aarch64) \
      X(trcimspec3, 117 |  D_REG |      TRACE | Arch_aarch64) \
      X(trcimspec4, 118 |  D_REG |      TRACE | Arch_aarch64) \
      X(trcimspec5, 119 |  D_REG |      TRACE | Arch_aarch64) \
      X(trcimspec6, 120 |  D_REG |      TRACE | Arch_aarch64) \
      X(trcimspec7, 121 |  D_REG |      TRACE | Arch_aarch64) \
      X(trcitctrl, 122 |  D_REG |      TRACE | Arch_aarch64) \
      X(trcitecr_el1, 123 |   FULL |      TRACE | Arch_aarch64) \
      X(trcitecr_el2, 124 |   FULL |      TRACE | Arch_aarch64) \
      X(trciteedcr, 125 |  D_REG |      TRACE | Arch_aarch64) \
      X(trclar, 126 |  D_REG |      TRACE | Arch_aarch64) \
      X(trclsr, 127 |  D_REG |      TRACE | Arch_aarch64) \
      X(trcoslar, 128 |   FULL |      TRACE | Arch_aarch64) \
      X(trcoslsr, 129 |   FULL |      TRACE | Arch_aarch64) \
      X(trcpdcr, 130 |  D_REG |      TRACE | Arch_aarch64) \
      X(trcpdsr, 131 |  D_REG |      TRACE | Arch_aarch64) \
      X(trcpidr0, 132 |  D_REG |      TRACE | Arch_aarch64) \
      X(trcpidr1, 133 |  D_REG |      TRACE | Arch_aarch64) \
      X(trcpidr2, 134 |  D_REG |      TRACE | Arch_aarch64) \
      X(trcpidr3, 135 |  D_REG |      TRACE | Arch_aarch64) \
      X(trcpidr4, 136 |  D_REG |      TRACE | Arch_aarch64) \
      X(trcpidr5, 137 |  D_REG |      TRACE | Arch_aarch64) \
      X(trcpidr6, 138 |  D_REG |      TRACE | Arch_aarch64) \
      X(trcpidr7, 139 |  D_REG |      TRACE | Arch_aarch64) \
      X(trcprgctlr, 140 |   FULL |      TRACE | Arch_aarch64) \
      X(trcprocselr, 141 |   FULL |      TRACE | Arch_aarch64) \
      X(trcqctlr, 142 |  D_REG |      TRACE | Arch_aarch64) \
      X(trcrsctlr2, 143 |   FULL |      TRACE | Arch_aarch64) \
      X(trcrsctlr3, 144 |   FULL |      TRACE | Arch_aarch64) \
      X(trcrsctlr4, 145 |   FULL |      TRACE | Arch_aarch64) \
      X(trcrsctlr5, 146 |   FULL |      TRACE | Arch_aarch64) \
      X(trcrsctlr6, 147 |   FULL |      TRACE | Arch_aarch64) \
      X(trcrsctlr7, 148 |   FULL |      TRACE | Arch_aarch64) \
      X(trcrsctlr8, 149 |   FULL |      TRACE | Arch_aarch64) \
      X(trcrsctlr9, 150 |   FULL |      TRACE | Arch_aarch64) \
      X(trcrsctlr10, 151 |   FULL |      TRACE | Arch_aarch64) \
      X(trcrsctlr11, 152 |   FULL |      TRACE | Arch_aarch64) \
      X(trcrsctlr12, 153 |   FULL |      TRACE | Arch_aarch64) \
      X(trcrsctlr13, 154 |   FULL |      TRACE | Arch_aarch64) \
      X(trcrsctlr14, 155 |   FULL |      TRACE | Arch_aarch64) \
      X(trcrsctlr15, 156 |   FULL |      TRACE | Arch_aarch64) \
      X(trcrsctlr16, 157 |   FULL |      TRACE | Arch_aarch64) \
      X(trcrsctlr17, 158 |   FULL |      TRACE | Arch_aarch64) \
      X(trcrsctlr18, 159 |   FULL |      TRACE | Arch_aarch64) \
      X(trcrsctlr19, 160 |   FULL |      TRACE | Arch_aarch64) \
      X(trcrsctlr20, 161 |   FULL |      TRACE | Arch_aarch64) \
      X(trcrsctlr21, 162 |   FULL |      TRACE | Arch_aarch64) \
      X(trcrsctlr22, 163 |   FULL |      TRACE | Arch_aarch64) \
      X(trcrsctlr23, 164 |   FULL |      TRACE | Arch_aarch64) \
      X(trcrsctlr24, 165 |   FULL |      TRACE | Arch_aarch64) \
      X(trcrsctlr25, 166 |   FULL |      TRACE | Arch_aarch64) \
      X(trcrsctlr26, 167 |   FULL |      TRACE | Arch_aarch64) \
      X(trcrsctlr27, 168 |   FULL |      TRACE | Arch_aarch64) \
      X(trcrsctlr28, 169 |   FULL |      TRACE | Arch_aarch64) \
      X(trcrsctlr29, 170 |   FULL |      TRACE | Arch_aarch64) \
      X(trcrsctlr30, 171 |   FULL |      TRACE | Arch_aarch64) \
      X(trcrsctlr31, 172 |   FULL |      TRACE | Arch_aarch64) \
      X(trcrsr, 173 |   FULL |      TRACE | Arch_aarch64) \
      X(trcseqevr0, 174 |   FULL |      TRACE | Arch_aarch64) \
      X(trcseqevr1, 175 |   FULL |      TRACE | Arch_aarch64) \
      X(trcseqevr2, 176 |   FULL |      TRACE | Arch_aarch64) \
      X(trcseqrstevr, 177 |  D_REG |      TRACE | Arch_aarch64) \
      X(trcseqstr, 178 |   FULL |      TRACE | Arch_aarch64) \
      X(trcssccr0, 179 |  D_REG |      TRACE | Arch_aarch64) \
      X(trcssccr1, 180 |  D_REG |      TRACE | Arch_aarch64) \
      X(trcssccr2, 181 |  D_REG |      TRACE | Arch_aarch64) \
      X(trcssccr3, 182 |  D_REG |      TRACE | Arch_aarch64) \
      X(trcssccr4, 183 |  D_REG |      TRACE | Arch_aarch64) \
      X(trcssccr5, 184 |  D_REG |      TRACE | Arch_aarch64) \
      X(trcssccr6, 185 |  D_REG |      TRACE | Arch_aarch64) \
      X(trcssccr7, 186 |  D_REG |      TRACE | Arch_aarch64) \
      X(trcsscsr0, 187 |  D_REG |      TRACE | Arch_aarch64) \
      X(trcsscsr1, 188 |  D_REG |      TRACE | Arch_aarch64) \
      X(trcsscsr2, 189 |  D_REG |      TRACE | Arch_aarch64) \
      X(trcsscsr3, 190 |  D_REG |      TRACE | Arch_aarch64) \
      X(trcsscsr4, 191 |  D_REG |      TRACE | Arch_aarch64) \
      X(trcsscsr5, 192 |  D_REG |      TRACE | Arch_aarch64) \
      X(trcsscsr6, 193 |  D_REG |      TRACE | Arch_aarch64) \
      X(trcsscsr7, 194 |  D_REG |      TRACE | Arch_aarch64) \
      X(trcsspcicr0, 195 |  D_REG |      TRACE | Arch_aarch64) \
      X(trcsspcicr1, 196 |  D_REG |      TRACE | Arch_aarch64) \
      X(trcsspcicr2, 197 |  D_REG |      TRACE | Arch_aarch64) \
      X(trcsspcicr3, 198 |  D_REG |      TRACE | Arch_aarch64) \
      X(trcsspcicr4, 199 |  D_REG |      TRACE | Arch_aarch64) \
      X(trcsspcicr5, 200 |  D_REG |      TRACE | Arch_aarch64) \
      X(trcsspcicr6, 201 |  D_REG |      TRACE | Arch_aarch64) \
      X(trcsspcicr7, 202 |  D_REG |      TRACE | Arch_aarch64) \
      X(trcstallctlr, 203 |   FULL |      TRACE | Arch_aarch64) \
      X(trcstatr, 204 |  D_REG |      TRACE | Arch_aarch64) \
      X(trcsyncpr, 205 |   FULL |      TRACE | Arch_aarch64) \
      X(trctraceidr, 206 |  D_REG |      TRACE | Arch_aarch64) \
      X(trctsctlr, 207 |   FULL |      TRACE | Arch_aarch64) \
      X(trcvdarcctlr, 208 |   FULL |      TRACE | Arch_aarch64) \
      X(trcvdctlr, 209 |   FULL |      TRACE | Arch_aarch64) \
      X(trcvdsacctlr, 210 |   FULL |      TRACE | Arch_aarch64) \
      X(trcvictlr, 211 |   FULL |      TRACE | Arch_aarch64) \
      X(trcviiectlr, 212 |  D_REG |      TRACE | Arch_aarch64) \
      X(trcvipcssctlr, 213 |   FULL |      TRACE | Arch_aarch64) \
      X(trcvissctlr, 214 |  D_REG |      TRACE | Arch_aarch64) \
      X(trcvmidcctlr0, 215 |   FULL |      TRACE | Arch_aarch64) \
      X(trcvmidcctlr1, 216 |   FULL |      TRACE | Arch_aarch64) \
      X(trcvmidcvr0, 217 |   FULL |      TRACE | Arch_aarch64) \
      X(trcvmidcvr1, 218 |   FULL |      TRACE | Arch_aarch64) \
      X(trcvmidcvr2, 219 |   FULL |      TRACE | Arch_aarch64) \
      X(trcvmidcvr3, 220 |   FULL |      TRACE | Arch_aarch64) \
      X(trcvmidcvr4, 221 |   FULL |      TRACE | Arch_aarch64) \
      X(trcvmidcvr5, 222 |   FULL |      TRACE | Arch_aarch64) \
      X(trcvmidcvr6, 223 |   FULL |      TRACE | Arch_aarch64) \
      X(trcvmidcvr7, 224 |   FULL |      TRACE | Arch_aarch64) \
      X(trfcr_el1, 225 |   FULL |      TRACE | Arch_aarch64) \
      X(trfcr_el2, 226 |   FULL |      TRACE | Arch_aarch64) \
      X(vbar_el1, 6 |   FULL |       EXCP | Arch_aarch64) \
      X(vbar_el2, 51 |   FULL |       VIRT | Arch_aarch64) \
      X(vbar_el3, 7 |   FULL | EXCP | SEC | Arch_aarch64) \
      X(vdisr_el2, 16 |   FULL |        RAS | Arch_aarch64) \
      X(vdisr_el3, 17 |   FULL |        RAS | Arch_aarch64) \
      X(vmecid_a_el2, 19 |   FULL |     SYSCTL | Arch_aarch64) \
      X(vmecid_p_el2, 20 |   FULL |     SYSCTL | Arch_aarch64) \
      X(vmpidr_el2, 52 |   FULL |       VIRT | Arch_aarch64) \
      X(vncr_el2, 21 |   FULL |     SYSCTL | Arch_aarch64) \
      X(vpidr_el2, 53 |   FULL |       VIRT | Arch_aarch64) \
      X(vsesr_el2, 18 |   FULL |        RAS | Arch_aarch64) \
      X(vsesr_el3, 19 |   FULL |        RAS | Arch_aarch64) \
      X(vstcr_el2, 22 |   FULL |     SYSCTL | Arch_aarch64) \
      X(vsttbr_el2, 23 |   FULL |     SYSCTL | Arch_aarch64) \
      X(zcr_el1, 6 |   FULL |      OTHER | Arch_aarch64) \
      X(zcr_el2, 7 |   FULL |      OTHER | Arch_aarch64) \
      X(zcr_el3, 8 |   FULL |      OTHER | Arch_aarch64) \
      A(fp, x29) \
      A(lr, x30) \
      A(Ip0, x16) \
      A(Ip1, x17) \
      X(IMPLEMENTATION_DEFINED_SYSREG, 255 | D_REG | SYSREG | Arch_aarch64) \
      X(hq0, 0 | HQ_REG |    FPR | Arch_aarch64) \
      X(hq1, 1 | HQ_REG |    FPR | Arch_aarch64) \
      X(hq2, 2 | HQ_REG |    FPR | Arch_aarch64) \
      X(hq3, 3 | HQ_REG |    FPR | Arch_aarch64) \
      X(hq4, 4 | HQ_REG |    FPR | Arch_aarch64) \
      X(hq5, 5 | HQ_REG |    FPR | Arch_aarch64) \
      X(hq6, 6 | HQ_REG |    FPR | Arch_aarch64) \
      X(hq7, 7 | HQ_REG |    FPR | Arch_aarch64) \
      X(hq8, 8 | HQ_REG |    FPR | Arch_aarch64) \
      X(hq9, 9 | HQ_REG |    FPR | Arch_aarch64) \
      X(hq10, 10 | HQ_REG |    FPR | Arch_aarch64) \
      X(hq11, 11 | HQ_REG |    FPR | Arch_aarch64) \
      X(hq12, 12 | HQ_REG |    FPR | Arch_aarch64) \
      X(hq13, 13 | HQ_REG |    FPR | Arch_aarch64) \
      X(hq14, 14 | HQ_REG |    FPR | Arch_aarch64) \
      X(hq15, 15 | HQ_REG |    FPR | Arch_aarch64) \
      X(hq16, 16 | HQ_REG |    FPR | Arch_aarch64) \
      X(hq17, 17 | HQ_REG |    FPR | Arch_aarch64) \
      X(hq18, 18 | HQ_REG |    FPR | Arch_aarch64) \
      X(hq19, 19 | HQ_REG |    FPR | Arch_aarch64) \
      X(hq20, 20 | HQ_REG |    FPR | Arch_aarch64) \
      X(hq21, 21 | HQ_REG |    FPR | Arch_aarch64) \
      X(hq22, 22 | HQ_REG |    FPR | Arch_aarch64) \
      X(hq23, 23 | HQ_REG |    FPR | Arch_aarch64) \
      X(hq24, 24 | HQ_REG |    FPR | Arch_aarch64) \
      X(hq25, 25 | HQ_REG |    FPR | Arch_aarch64) \
      X(hq26, 26 | HQ_REG |    FPR | Arch_aarch64) \
      X(hq27, 27 | HQ_REG |    FPR | Arch_aarch64) \
      X(hq28, 28 | HQ_REG |    FPR | Arch_aarch64) \
      X(hq29, 29 | HQ_REG |    FPR | Arch_aarch64) \
      X(hq30, 30 | HQ_REG |    FPR | Arch_aarch64) \
      X(hq31, 31 | HQ_REG |    FPR | Arch_aarch64) \
      X(tlbi_vale3is, 0 |   FULL | TLBI | Arch_aarch64) \
      X(tlbi_ipas2le1, 1 |   FULL | TLBI | Arch_aarch64) \
      X(tlbi_ipas2le1is, 2 |   FULL | TLBI | Arch_aarch64) \
      X(tlbi_vale2is, 3 |   FULL | TLBI | Arch_aarch64) \
      X(tlbi_alle3is, 4 |   FULL | TLBI | Arch_aarch64) \
      X(tlbi_ipas2e1is, 5 |   FULL | TLBI | Arch_aarch64) \
      X(tlbi_vae2, 6 |   FULL | TLBI | Arch_aarch64) \
      X(tlbi_vmalle1, 7 |   FULL | TLBI | Arch_aarch64) \
      X(tlbi_vale2, 8 |   FULL | TLBI | Arch_aarch64) \
      X(tlbi_vaae1, 9 |   FULL | TLBI | Arch_aarch64) \
      X(tlbi_ipas2e1, 10 |   FULL | TLBI | Arch_aarch64) \
      X(tlbi_aside1is, 11 |   FULL | TLBI | Arch_aarch64) \
      X(tlbi_alle1, 12 |   FULL | TLBI | Arch_aarch64) \
      X(tlbi_vaale1, 13 |   FULL | TLBI | Arch_aarch64) \
      X(tlbi_aside1, 14 |   FULL | TLBI | Arch_aarch64) \
      X(tlbi_alle2, 15 |   FULL | TLBI | Arch_aarch64) \
      X(tlbi_vmalls12e1is, 16 |   FULL | TLBI | Arch_aarch64) \
      X(tlbi_vae1, 17 |   FULL | TLBI | Arch_aarch64) \
      X(tlbi_vaae1is, 18 |   FULL | TLBI | Arch_aarch64) \
      X(tlbi_alle2is, 19 |   FULL | TLBI | Arch_aarch64) \
      X(tlbi_vae3, 20 |   FULL | TLBI | Arch_aarch64) \
      X(tlbi_vmalls12e1, 21 |   FULL | TLBI | Arch_aarch64) \
      X(tlbi_vae3is, 22 |   FULL | TLBI | Arch_aarch64) \
      X(tlbi_vae2is, 23 |   FULL | TLBI | Arch_aarch64) \
      X(tlbi_vmalle1is, 24 |   FULL | TLBI | Arch_aarch64) \
      X(tlbi_vaale1is, 25 |   FULL | TLBI | Arch_aarch64) \
      X(tlbi_alle1is, 26 |   FULL | TLBI | Arch_aarch64) \
      X(tlbi_vae1is, 27 |   FULL | TLBI | Arch_aarch64) \
      X(tlbi_vale3, 28 |   FULL | TLBI | Arch_aarch64) \
      X(tlbi_vale1is, 29 |   FULL | TLBI | Arch_aarch64) \
      X(tlbi_alle3, 30 |   FULL | TLBI | Arch_aarch64) \
      X(tlbi_vale1, 31 |   FULL | TLBI | Arch_aarch64) \
      X(ic_iallu, 0 | FULL | MNEMONICS | Arch_aarch64) \
      X(ic_ivau, 1 | FULL | MNEMONICS | Arch_aarch64) \
      X(ic_ialluis, 2 | FULL | MNEMONICS | Arch_aarch64) \
      X(at_s1e2r, 3 | FULL | MNEMONICS | Arch_aarch64) \
      X(at_s1e1r, 4 | FULL | MNEMONICS | Arch_aarch64) \
      X(at_s12e0w, 5 | FULL | MNEMONICS | Arch_aarch64) \
      X(at_s1e2w, 6 | FULL | MNEMONICS | Arch_aarch64) \
      X(at_s1e0w, 7 | FULL | MNEMONICS | Arch_aarch64) \
      X(at_s12e0r, 8 | FULL | MNEMONICS | Arch_aarch64) \
      X(at_s1e3w, 9 | FULL | MNEMONICS | Arch_aarch64) \
      X(at_s12e1w, 10 | FULL | MNEMONICS | Arch_aarch64) \
      X(at_s1e0r, 11 | FULL | MNEMONICS | Arch_aarch64) \
      X(at_s12e1r, 12 | FULL | MNEMONICS | Arch_aarch64) \
      X(at_s1e1w, 13 | FULL | MNEMONICS | Arch_aarch64) \
      X(at_s1e3r, 14 | FULL | MNEMONICS | Arch_aarch64) \
      X(dc_civac, 15 | FULL | MNEMONICS | Arch_aarch64) \
      X(dc_ivac, 16 | FULL | MNEMONICS | Arch_aarch64) \
      X(dc_csw, 17 | FULL | MNEMONICS | Arch_aarch64) \
      X(dc_cvau, 18 | FULL | MNEMONICS | Arch_aarch64) \
      X(dc_cisw, 19 | FULL | MNEMONICS | Arch_aarch64) \
      X(dc_zva, 20 | FULL | MNEMONICS | Arch_aarch64) \
      X(dc_isw, 21 | FULL | MNEMONICS | Arch_aarch64) \
      X(dc_cvac, 22 | FULL | MNEMONICS | Arch_aarch64)

  #define DEF_ONE(n, v) DEF_REGISTER(n, v);
  #define DEF_ALS(n, t) DEF_REGISTER_ALIAS(n, t);
  DYNINST_AARCH64_REG_LIST(DEF_ONE, DEF_ALS)
  #undef DEF_ONE
  #undef DEF_ALS

  #define NAME_ONE(n, v) n,
  constexpr MachRegister all_regs[] = { DYNINST_AARCH64_REG_LIST(NAME_ONE, NAME_ONE) };
  #undef NAME_ONE



}}

#endif
