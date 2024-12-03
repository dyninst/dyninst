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

  //          (                name,  ID |  alias |        cat |         arch,      arch)
  DEF_REGISTER(                fpcr,   0 |  D_REG |        SPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(                nzcv,   1 |    BIT |        SPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(                  pc,   2 |   FULL |        SPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(                  sp,   3 |   FULL |        SPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(                 wsp,   4 |  D_REG |        SPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(                 wzr,   5 |  D_REG |        SPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(                 xzr,   6 |   FULL |        SPR | Arch_aarch64, "aarch64");

  DEF_REGISTER(                  w0,   0 |  D_REG |        GPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(                  w1,   1 |  D_REG |        GPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(                  w2,   2 |  D_REG |        GPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(                  w3,   3 |  D_REG |        GPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(                  w4,   4 |  D_REG |        GPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(                  w5,   5 |  D_REG |        GPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(                  w6,   6 |  D_REG |        GPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(                  w7,   7 |  D_REG |        GPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(                  w8,   8 |  D_REG |        GPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(                  w9,   9 |  D_REG |        GPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(                 w10,  10 |  D_REG |        GPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(                 w11,  11 |  D_REG |        GPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(                 w12,  12 |  D_REG |        GPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(                 w13,  13 |  D_REG |        GPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(                 w14,  14 |  D_REG |        GPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(                 w15,  15 |  D_REG |        GPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(                 w16,  16 |  D_REG |        GPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(                 w17,  17 |  D_REG |        GPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(                 w18,  18 |  D_REG |        GPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(                 w19,  19 |  D_REG |        GPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(                 w20,  20 |  D_REG |        GPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(                 w21,  21 |  D_REG |        GPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(                 w22,  22 |  D_REG |        GPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(                 w23,  23 |  D_REG |        GPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(                 w24,  24 |  D_REG |        GPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(                 w25,  25 |  D_REG |        GPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(                 w26,  26 |  D_REG |        GPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(                 w27,  27 |  D_REG |        GPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(                 w28,  28 |  D_REG |        GPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(                 w29,  29 |  D_REG |        GPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(                 w30,  30 |  D_REG |        GPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(                  x0,  31 |   FULL |        GPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(                  x1,  32 |   FULL |        GPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(                  x2,  33 |   FULL |        GPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(                  x3,  34 |   FULL |        GPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(                  x4,  35 |   FULL |        GPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(                  x5,  36 |   FULL |        GPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(                  x6,  37 |   FULL |        GPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(                  x7,  38 |   FULL |        GPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(                  x8,  39 |   FULL |        GPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(                  x9,  40 |   FULL |        GPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(                 x10,  41 |   FULL |        GPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(                 x11,  42 |   FULL |        GPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(                 x12,  43 |   FULL |        GPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(                 x13,  44 |   FULL |        GPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(                 x14,  45 |   FULL |        GPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(                 x15,  46 |   FULL |        GPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(                 x16,  47 |   FULL |        GPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(                 x17,  48 |   FULL |        GPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(                 x18,  49 |   FULL |        GPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(                 x19,  50 |   FULL |        GPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(                 x20,  51 |   FULL |        GPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(                 x21,  52 |   FULL |        GPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(                 x22,  53 |   FULL |        GPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(                 x23,  54 |   FULL |        GPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(                 x24,  55 |   FULL |        GPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(                 x25,  56 |   FULL |        GPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(                 x26,  57 |   FULL |        GPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(                 x27,  58 |   FULL |        GPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(                 x28,  59 |   FULL |        GPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(                 x29,  60 |   FULL |        GPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(                 x30,  61 |   FULL |        GPR | Arch_aarch64, "aarch64");

  DEF_REGISTER(                  b0,   0 |  B_REG |        FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(                  b1,   1 |  B_REG |        FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(                  b2,   2 |  B_REG |        FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(                  b3,   3 |  B_REG |        FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(                  b4,   4 |  B_REG |        FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(                  b5,   5 |  B_REG |        FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(                  b6,   6 |  B_REG |        FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(                  b7,   7 |  B_REG |        FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(                  b8,   8 |  B_REG |        FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(                  b9,   9 |  B_REG |        FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(                 b10,  10 |  B_REG |        FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(                 b11,  11 |  B_REG |        FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(                 b12,  12 |  B_REG |        FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(                 b13,  13 |  B_REG |        FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(                 b14,  14 |  B_REG |        FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(                 b15,  15 |  B_REG |        FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(                 b16,  16 |  B_REG |        FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(                 b17,  17 |  B_REG |        FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(                 b18,  18 |  B_REG |        FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(                 b19,  19 |  B_REG |        FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(                 b20,  20 |  B_REG |        FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(                 b21,  21 |  B_REG |        FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(                 b22,  22 |  B_REG |        FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(                 b23,  23 |  B_REG |        FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(                 b24,  24 |  B_REG |        FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(                 b25,  25 |  B_REG |        FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(                 b26,  26 |  B_REG |        FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(                 b27,  27 |  B_REG |        FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(                 b28,  28 |  B_REG |        FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(                 b29,  29 |  B_REG |        FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(                 b30,  30 |  B_REG |        FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(                 b31,  31 |  B_REG |        FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(                  d0,  32 |   FULL |        FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(                  d1,  33 |   FULL |        FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(                  d2,  34 |   FULL |        FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(                  d3,  35 |   FULL |        FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(                  d4,  36 |   FULL |        FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(                  d5,  37 |   FULL |        FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(                  d6,  38 |   FULL |        FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(                  d7,  39 |   FULL |        FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(                  d8,  40 |   FULL |        FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(                  d9,  41 |   FULL |        FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(                 d10,  42 |   FULL |        FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(                 d11,  43 |   FULL |        FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(                 d12,  44 |   FULL |        FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(                 d13,  45 |   FULL |        FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(                 d14,  46 |   FULL |        FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(                 d15,  47 |   FULL |        FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(                 d16,  48 |   FULL |        FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(                 d17,  49 |   FULL |        FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(                 d18,  50 |   FULL |        FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(                 d19,  51 |   FULL |        FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(                 d20,  52 |   FULL |        FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(                 d21,  53 |   FULL |        FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(                 d22,  54 |   FULL |        FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(                 d23,  55 |   FULL |        FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(                 d24,  56 |   FULL |        FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(                 d25,  57 |   FULL |        FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(                 d26,  58 |   FULL |        FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(                 d27,  59 |   FULL |        FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(                 d28,  60 |   FULL |        FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(                 d29,  61 |   FULL |        FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(                 d30,  62 |   FULL |        FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(                 d31,  63 |   FULL |        FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(                  h0,  64 |  W_REG |        FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(                  h1,  65 |  W_REG |        FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(                  h2,  66 |  W_REG |        FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(                  h3,  67 |  W_REG |        FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(                  h4,  68 |  W_REG |        FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(                  h5,  69 |  W_REG |        FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(                  h6,  70 |  W_REG |        FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(                  h7,  71 |  W_REG |        FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(                  h8,  72 |  W_REG |        FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(                  h9,  73 |  W_REG |        FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(                 h10,  74 |  W_REG |        FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(                 h11,  75 |  W_REG |        FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(                 h12,  76 |  W_REG |        FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(                 h13,  77 |  W_REG |        FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(                 h14,  78 |  W_REG |        FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(                 h15,  79 |  W_REG |        FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(                 h16,  80 |  W_REG |        FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(                 h17,  81 |  W_REG |        FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(                 h18,  82 |  W_REG |        FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(                 h19,  83 |  W_REG |        FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(                 h20,  84 |  W_REG |        FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(                 h21,  85 |  W_REG |        FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(                 h22,  86 |  W_REG |        FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(                 h23,  87 |  W_REG |        FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(                 h24,  88 |  W_REG |        FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(                 h25,  89 |  W_REG |        FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(                 h26,  90 |  W_REG |        FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(                 h27,  91 |  W_REG |        FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(                 h28,  92 |  W_REG |        FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(                 h29,  93 |  W_REG |        FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(                 h30,  94 |  W_REG |        FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(                 h31,  95 |  W_REG |        FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(                  q0,  96 |  Q_REG |        FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(                  q1,  97 |  Q_REG |        FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(                  q2,  98 |  Q_REG |        FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(                  q3,  99 |  Q_REG |        FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(                  q4, 100 |  Q_REG |        FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(                  q5, 101 |  Q_REG |        FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(                  q6, 102 |  Q_REG |        FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(                  q7, 103 |  Q_REG |        FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(                  q8, 104 |  Q_REG |        FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(                  q9, 105 |  Q_REG |        FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(                 q10, 106 |  Q_REG |        FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(                 q11, 107 |  Q_REG |        FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(                 q12, 108 |  Q_REG |        FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(                 q13, 109 |  Q_REG |        FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(                 q14, 110 |  Q_REG |        FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(                 q15, 111 |  Q_REG |        FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(                 q16, 112 |  Q_REG |        FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(                 q17, 113 |  Q_REG |        FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(                 q18, 114 |  Q_REG |        FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(                 q19, 115 |  Q_REG |        FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(                 q20, 116 |  Q_REG |        FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(                 q21, 117 |  Q_REG |        FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(                 q22, 118 |  Q_REG |        FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(                 q23, 119 |  Q_REG |        FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(                 q24, 120 |  Q_REG |        FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(                 q25, 121 |  Q_REG |        FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(                 q26, 122 |  Q_REG |        FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(                 q27, 123 |  Q_REG |        FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(                 q28, 124 |  Q_REG |        FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(                 q29, 125 |  Q_REG |        FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(                 q30, 126 |  Q_REG |        FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(                 q31, 127 |  Q_REG |        FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(                  s0, 128 |  D_REG |        FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(                  s1, 129 |  D_REG |        FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(                  s2, 130 |  D_REG |        FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(                  s3, 131 |  D_REG |        FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(                  s4, 132 |  D_REG |        FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(                  s5, 133 |  D_REG |        FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(                  s6, 134 |  D_REG |        FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(                  s7, 135 |  D_REG |        FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(                  s8, 136 |  D_REG |        FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(                  s9, 137 |  D_REG |        FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(                 s10, 138 |  D_REG |        FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(                 s11, 139 |  D_REG |        FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(                 s12, 140 |  D_REG |        FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(                 s13, 141 |  D_REG |        FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(                 s14, 142 |  D_REG |        FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(                 s15, 143 |  D_REG |        FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(                 s16, 144 |  D_REG |        FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(                 s17, 145 |  D_REG |        FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(                 s18, 146 |  D_REG |        FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(                 s19, 147 |  D_REG |        FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(                 s20, 148 |  D_REG |        FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(                 s21, 149 |  D_REG |        FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(                 s22, 150 |  D_REG |        FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(                 s23, 151 |  D_REG |        FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(                 s24, 152 |  D_REG |        FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(                 s25, 153 |  D_REG |        FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(                 s26, 154 |  D_REG |        FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(                 s27, 155 |  D_REG |        FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(                 s28, 156 |  D_REG |        FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(                 s29, 157 |  D_REG |        FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(                 s30, 158 |  D_REG |        FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(                 s31, 159 |  D_REG |        FPR | Arch_aarch64, "aarch64");

  DEF_REGISTER(                 ffr,   0 |  PREDS |        SVE | Arch_aarch64, "aarch64");
  DEF_REGISTER(                  p0,   1 |  PREDS |        SVE | Arch_aarch64, "aarch64");
  DEF_REGISTER(                  p1,   2 |  PREDS |        SVE | Arch_aarch64, "aarch64");
  DEF_REGISTER(                  p2,   3 |  PREDS |        SVE | Arch_aarch64, "aarch64");
  DEF_REGISTER(                  p3,   4 |  PREDS |        SVE | Arch_aarch64, "aarch64");
  DEF_REGISTER(                  p4,   5 |  PREDS |        SVE | Arch_aarch64, "aarch64");
  DEF_REGISTER(                  p5,   6 |  PREDS |        SVE | Arch_aarch64, "aarch64");
  DEF_REGISTER(                  p6,   7 |  PREDS |        SVE | Arch_aarch64, "aarch64");
  DEF_REGISTER(                  p7,   8 |  PREDS |        SVE | Arch_aarch64, "aarch64");
  DEF_REGISTER(                  p8,   9 |  PREDS |        SVE | Arch_aarch64, "aarch64");
  DEF_REGISTER(                  p9,  10 |  PREDS |        SVE | Arch_aarch64, "aarch64");
  DEF_REGISTER(                 p10,  11 |  PREDS |        SVE | Arch_aarch64, "aarch64");
  DEF_REGISTER(                 p11,  12 |  PREDS |        SVE | Arch_aarch64, "aarch64");
  DEF_REGISTER(                 p12,  13 |  PREDS |        SVE | Arch_aarch64, "aarch64");
  DEF_REGISTER(                 p13,  14 |  PREDS |        SVE | Arch_aarch64, "aarch64");
  DEF_REGISTER(                 p14,  15 |  PREDS |        SVE | Arch_aarch64, "aarch64");
  DEF_REGISTER(                 p15,  16 |  PREDS |        SVE | Arch_aarch64, "aarch64");
  DEF_REGISTER(                  vg,  17 |   FULL |        SVE | Arch_aarch64, "aarch64");
  DEF_REGISTER(                  z0,  18 |   SVES |        SVE | Arch_aarch64, "aarch64");
  DEF_REGISTER(                  z1,  19 |   SVES |        SVE | Arch_aarch64, "aarch64");
  DEF_REGISTER(                  z2,  20 |   SVES |        SVE | Arch_aarch64, "aarch64");
  DEF_REGISTER(                  z3,  21 |   SVES |        SVE | Arch_aarch64, "aarch64");
  DEF_REGISTER(                  z4,  22 |   SVES |        SVE | Arch_aarch64, "aarch64");
  DEF_REGISTER(                  z5,  23 |   SVES |        SVE | Arch_aarch64, "aarch64");
  DEF_REGISTER(                  z6,  24 |   SVES |        SVE | Arch_aarch64, "aarch64");
  DEF_REGISTER(                  z7,  25 |   SVES |        SVE | Arch_aarch64, "aarch64");
  DEF_REGISTER(                  z8,  26 |   SVES |        SVE | Arch_aarch64, "aarch64");
  DEF_REGISTER(                  z9,  27 |   SVES |        SVE | Arch_aarch64, "aarch64");
  DEF_REGISTER(                 z10,  28 |   SVES |        SVE | Arch_aarch64, "aarch64");
  DEF_REGISTER(                 z11,  29 |   SVES |        SVE | Arch_aarch64, "aarch64");
  DEF_REGISTER(                 z12,  30 |   SVES |        SVE | Arch_aarch64, "aarch64");
  DEF_REGISTER(                 z13,  31 |   SVES |        SVE | Arch_aarch64, "aarch64");
  DEF_REGISTER(                 z14,  32 |   SVES |        SVE | Arch_aarch64, "aarch64");
  DEF_REGISTER(                 z15,  33 |   SVES |        SVE | Arch_aarch64, "aarch64");
  DEF_REGISTER(                 z16,  34 |   SVES |        SVE | Arch_aarch64, "aarch64");
  DEF_REGISTER(                 z17,  35 |   SVES |        SVE | Arch_aarch64, "aarch64");
  DEF_REGISTER(                 z18,  36 |   SVES |        SVE | Arch_aarch64, "aarch64");
  DEF_REGISTER(                 z19,  37 |   SVES |        SVE | Arch_aarch64, "aarch64");
  DEF_REGISTER(                 z20,  38 |   SVES |        SVE | Arch_aarch64, "aarch64");
  DEF_REGISTER(                 z21,  39 |   SVES |        SVE | Arch_aarch64, "aarch64");
  DEF_REGISTER(                 z22,  40 |   SVES |        SVE | Arch_aarch64, "aarch64");
  DEF_REGISTER(                 z23,  41 |   SVES |        SVE | Arch_aarch64, "aarch64");
  DEF_REGISTER(                 z24,  42 |   SVES |        SVE | Arch_aarch64, "aarch64");
  DEF_REGISTER(                 z25,  43 |   SVES |        SVE | Arch_aarch64, "aarch64");
  DEF_REGISTER(                 z26,  44 |   SVES |        SVE | Arch_aarch64, "aarch64");
  DEF_REGISTER(                 z27,  45 |   SVES |        SVE | Arch_aarch64, "aarch64");
  DEF_REGISTER(                 z28,  46 |   SVES |        SVE | Arch_aarch64, "aarch64");
  DEF_REGISTER(                 z29,  47 |   SVES |        SVE | Arch_aarch64, "aarch64");
  DEF_REGISTER(                 z30,  48 |   SVES |        SVE | Arch_aarch64, "aarch64");
  DEF_REGISTER(                 z31,  49 |   SVES |        SVE | Arch_aarch64, "aarch64");
  DEF_REGISTER(                 zt0,   0 |  SVE2S |       SVE2 | Arch_aarch64, "aarch64");

  DEF_REGISTER(                  za,   0 | SMEZAS |        SME | Arch_aarch64, "aarch64");
  DEF_REGISTER(                zab0,   1 | SMEZAS |        SME | Arch_aarch64, "aarch64");
  DEF_REGISTER(                zad0,   2 | SMEZAS |        SME | Arch_aarch64, "aarch64");
  DEF_REGISTER(                zad1,   3 | SMEZAS |        SME | Arch_aarch64, "aarch64");
  DEF_REGISTER(                zad2,   4 | SMEZAS |        SME | Arch_aarch64, "aarch64");
  DEF_REGISTER(                zad3,   5 | SMEZAS |        SME | Arch_aarch64, "aarch64");
  DEF_REGISTER(                zad4,   6 | SMEZAS |        SME | Arch_aarch64, "aarch64");
  DEF_REGISTER(                zad5,   7 | SMEZAS |        SME | Arch_aarch64, "aarch64");
  DEF_REGISTER(                zad6,   8 | SMEZAS |        SME | Arch_aarch64, "aarch64");
  DEF_REGISTER(                zad7,   9 | SMEZAS |        SME | Arch_aarch64, "aarch64");
  DEF_REGISTER(                zah0,  10 | SMEZAS |        SME | Arch_aarch64, "aarch64");
  DEF_REGISTER(                zah1,  11 | SMEZAS |        SME | Arch_aarch64, "aarch64");
  DEF_REGISTER(                zaq0,  12 | SMEZAS |        SME | Arch_aarch64, "aarch64");
  DEF_REGISTER(                zaq1,  13 | SMEZAS |        SME | Arch_aarch64, "aarch64");
  DEF_REGISTER(                zaq2,  14 | SMEZAS |        SME | Arch_aarch64, "aarch64");
  DEF_REGISTER(                zaq3,  15 | SMEZAS |        SME | Arch_aarch64, "aarch64");
  DEF_REGISTER(                zaq4,  16 | SMEZAS |        SME | Arch_aarch64, "aarch64");
  DEF_REGISTER(                zaq5,  17 | SMEZAS |        SME | Arch_aarch64, "aarch64");
  DEF_REGISTER(                zaq6,  18 | SMEZAS |        SME | Arch_aarch64, "aarch64");
  DEF_REGISTER(                zaq7,  19 | SMEZAS |        SME | Arch_aarch64, "aarch64");
  DEF_REGISTER(                zaq8,  20 | SMEZAS |        SME | Arch_aarch64, "aarch64");
  DEF_REGISTER(                zaq9,  21 | SMEZAS |        SME | Arch_aarch64, "aarch64");
  DEF_REGISTER(               zaq10,  22 | SMEZAS |        SME | Arch_aarch64, "aarch64");
  DEF_REGISTER(               zaq11,  23 | SMEZAS |        SME | Arch_aarch64, "aarch64");
  DEF_REGISTER(               zaq12,  24 | SMEZAS |        SME | Arch_aarch64, "aarch64");
  DEF_REGISTER(               zaq13,  25 | SMEZAS |        SME | Arch_aarch64, "aarch64");
  DEF_REGISTER(               zaq14,  26 | SMEZAS |        SME | Arch_aarch64, "aarch64");
  DEF_REGISTER(               zaq15,  27 | SMEZAS |        SME | Arch_aarch64, "aarch64");
  DEF_REGISTER(                zas0,  28 | SMEZAS |        SME | Arch_aarch64, "aarch64");
  DEF_REGISTER(                zas1,  29 | SMEZAS |        SME | Arch_aarch64, "aarch64");
  DEF_REGISTER(                zas2,  30 | SMEZAS |        SME | Arch_aarch64, "aarch64");
  DEF_REGISTER(                zas3,  31 | SMEZAS |        SME | Arch_aarch64, "aarch64");

  DEF_REGISTER(             elr_el1,   0 |   FULL |     SYSSPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(             elr_el2,   1 |   FULL |     SYSSPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(             elr_el3,   2 |   FULL |     SYSSPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(              sp_el0,   3 |   FULL |     SYSSPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(              sp_el1,   4 |   FULL |     SYSSPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(              sp_el2,   5 |   FULL |     SYSSPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(            spsr_abt,   6 |   FULL |     SYSSPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(            spsr_el1,   7 |   FULL |     SYSSPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(            spsr_el2,   8 |   FULL |     SYSSPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(            spsr_el3,   9 |   FULL |     SYSSPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(            spsr_fiq,  10 |   FULL |     SYSSPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(            spsr_irq,  11 |   FULL |     SYSSPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(            spsr_und,  12 |   FULL |     SYSSPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(          amair2_el1,   0 |   FULL |  SYSMEMORY | Arch_aarch64, "aarch64");
  DEF_REGISTER(          amair2_el2,   1 |   FULL |  SYSMEMORY | Arch_aarch64, "aarch64");
  DEF_REGISTER(          amair2_el3,   2 |   FULL |  SYSMEMORY | Arch_aarch64, "aarch64");
  DEF_REGISTER(           amair_el1,   3 |   FULL |  SYSMEMORY | Arch_aarch64, "aarch64");
  DEF_REGISTER(           amair_el2,   4 |   FULL |  SYSMEMORY | Arch_aarch64, "aarch64");
  DEF_REGISTER(           amair_el3,   5 |   FULL |  SYSMEMORY | Arch_aarch64, "aarch64");
  DEF_REGISTER(      contextidr_el1,   6 |   FULL |  SYSMEMORY | Arch_aarch64, "aarch64");
  DEF_REGISTER(      contextidr_el2,   7 |   FULL |  SYSMEMORY | Arch_aarch64, "aarch64");
  DEF_REGISTER(          dacr32_el2,   8 |   FULL |  SYSMEMORY | Arch_aarch64, "aarch64");
  DEF_REGISTER(           gpccr_el3,   9 |   FULL |  SYSMEMORY | Arch_aarch64, "aarch64");
  DEF_REGISTER(           gptbr_el3,  10 |   FULL |  SYSMEMORY | Arch_aarch64, "aarch64");
  DEF_REGISTER(        hacdbsbr_el2,  11 |   FULL |  SYSMEMORY | Arch_aarch64, "aarch64");
  DEF_REGISTER(      hacdbscons_el2,  12 |   FULL |  SYSMEMORY | Arch_aarch64, "aarch64");
  DEF_REGISTER(         hdbssbr_el2,  13 |   FULL |  SYSMEMORY | Arch_aarch64, "aarch64");
  DEF_REGISTER(       hdbssprod_el2,  14 |   FULL |  SYSMEMORY | Arch_aarch64, "aarch64");
  DEF_REGISTER(            lorc_el1,  15 |   FULL |  SYSMEMORY | Arch_aarch64, "aarch64");
  DEF_REGISTER(           lorea_el1,  16 |   FULL |  SYSMEMORY | Arch_aarch64, "aarch64");
  DEF_REGISTER(           lorid_el1,  17 |   FULL |  SYSMEMORY | Arch_aarch64, "aarch64");
  DEF_REGISTER(            lorn_el1,  18 |   FULL |  SYSMEMORY | Arch_aarch64, "aarch64");
  DEF_REGISTER(           lorsa_el1,  19 |   FULL |  SYSMEMORY | Arch_aarch64, "aarch64");
  DEF_REGISTER(           mair2_el1,  20 |   FULL |  SYSMEMORY | Arch_aarch64, "aarch64");
  DEF_REGISTER(           mair2_el2,  21 |   FULL |  SYSMEMORY | Arch_aarch64, "aarch64");
  DEF_REGISTER(           mair2_el3,  22 |   FULL |  SYSMEMORY | Arch_aarch64, "aarch64");
  DEF_REGISTER(            mair_el1,  23 |   FULL |  SYSMEMORY | Arch_aarch64, "aarch64");
  DEF_REGISTER(            mair_el2,  24 |   FULL |  SYSMEMORY | Arch_aarch64, "aarch64");
  DEF_REGISTER(            mair_el3,  25 |   FULL |  SYSMEMORY | Arch_aarch64, "aarch64");
  DEF_REGISTER(             pir_el1,  26 |   FULL |  SYSMEMORY | Arch_aarch64, "aarch64");
  DEF_REGISTER(             pir_el2,  27 |   FULL |  SYSMEMORY | Arch_aarch64, "aarch64");
  DEF_REGISTER(             pir_el3,  28 |   FULL |  SYSMEMORY | Arch_aarch64, "aarch64");
  DEF_REGISTER(           pire0_el1,  29 |   FULL |  SYSMEMORY | Arch_aarch64, "aarch64");
  DEF_REGISTER(           pire0_el2,  30 |   FULL |  SYSMEMORY | Arch_aarch64, "aarch64");
  DEF_REGISTER(             por_el0,  31 |   FULL |  SYSMEMORY | Arch_aarch64, "aarch64");
  DEF_REGISTER(             por_el1,  32 |   FULL |  SYSMEMORY | Arch_aarch64, "aarch64");
  DEF_REGISTER(             por_el2,  33 |   FULL |  SYSMEMORY | Arch_aarch64, "aarch64");
  DEF_REGISTER(             por_el3,  34 |   FULL |  SYSMEMORY | Arch_aarch64, "aarch64");
  DEF_REGISTER(         rcwmask_el1,  35 |  Q_REG |  SYSMEMORY | Arch_aarch64, "aarch64");
  DEF_REGISTER(        rcwsmask_el1,  36 |  Q_REG |  SYSMEMORY | Arch_aarch64, "aarch64");
  DEF_REGISTER(           s2pir_el2,  37 |   FULL |  SYSMEMORY | Arch_aarch64, "aarch64");
  DEF_REGISTER(           s2por_el1,  38 |   FULL |  SYSMEMORY | Arch_aarch64, "aarch64");
  DEF_REGISTER(            tcr2_el1,  39 |   FULL |  SYSMEMORY | Arch_aarch64, "aarch64");
  DEF_REGISTER(            tcr2_el2,  40 |   FULL |  SYSMEMORY | Arch_aarch64, "aarch64");
  DEF_REGISTER(             tcr_el1,  41 |   FULL |  SYSMEMORY | Arch_aarch64, "aarch64");
  DEF_REGISTER(             tcr_el2,  42 |   FULL |  SYSMEMORY | Arch_aarch64, "aarch64");
  DEF_REGISTER(             tcr_el3,  43 |   FULL |  SYSMEMORY | Arch_aarch64, "aarch64");
  DEF_REGISTER(           ttbr0_el1,  44 |  Q_REG |  SYSMEMORY | Arch_aarch64, "aarch64");
  DEF_REGISTER(           ttbr1_el1,  45 |  Q_REG |  SYSMEMORY | Arch_aarch64, "aarch64");
  DEF_REGISTER(           ttbr0_el2,  46 |  Q_REG |  SYSMEMORY | Arch_aarch64, "aarch64");
  DEF_REGISTER(           ttbr1_el2,  47 |  Q_REG |  SYSMEMORY | Arch_aarch64, "aarch64");
  DEF_REGISTER(           ttbr0_el3,  48 |   FULL |  SYSMEMORY | Arch_aarch64, "aarch64");
  DEF_REGISTER(          vsctlr_el2,  49 |  D_REG |  SYSMEMORY | Arch_aarch64, "aarch64");
  DEF_REGISTER(            vtcr_el2,  50 |   FULL |  SYSMEMORY | Arch_aarch64, "aarch64");
  DEF_REGISTER(           vttbr_el2,  51 |  Q_REG |  SYSMEMORY | Arch_aarch64, "aarch64");
  DEF_REGISTER(         accdata_el1,   0 |   FULL |         AD | Arch_aarch64, "aarch64");
  DEF_REGISTER(           actlr_el1,   0 |   FULL |    IMPLDEF | Arch_aarch64, "aarch64");
  DEF_REGISTER(           actlr_el2,   1 |   FULL |    IMPLDEF | Arch_aarch64, "aarch64");
  DEF_REGISTER(           actlr_el3,   2 |   FULL |    IMPLDEF | Arch_aarch64, "aarch64");
  DEF_REGISTER(           afsr0_el1,   3 |   FULL |    IMPLDEF | Arch_aarch64, "aarch64");
  DEF_REGISTER(           afsr1_el1,   4 |   FULL |    IMPLDEF | Arch_aarch64, "aarch64");
  DEF_REGISTER(           afsr0_el2,   5 |   FULL |    IMPLDEF | Arch_aarch64, "aarch64");
  DEF_REGISTER(           afsr1_el2,   6 |   FULL |    IMPLDEF | Arch_aarch64, "aarch64");
  DEF_REGISTER(           afsr0_el3,   7 |   FULL |    IMPLDEF | Arch_aarch64, "aarch64");
  DEF_REGISTER(           afsr1_el3,   8 |   FULL |    IMPLDEF | Arch_aarch64, "aarch64");
  DEF_REGISTER(            aidr_el1,   9 |   FULL |    IMPLDEF | Arch_aarch64, "aarch64");
  DEF_REGISTER(              allint,   0 |   FULL |     PSTATE | Arch_aarch64, "aarch64");
  DEF_REGISTER(          amcfgr_el0,   0 |   FULL |        AMR | Arch_aarch64, "aarch64");
  DEF_REGISTER(        amcg1idr_el0,   1 |   FULL |        AMR | Arch_aarch64, "aarch64");
  DEF_REGISTER(          amcgcr_el0,   2 |   FULL |        AMR | Arch_aarch64, "aarch64");
  DEF_REGISTER(     amcntenclr0_el0,   3 |   FULL |        AMR | Arch_aarch64, "aarch64");
  DEF_REGISTER(     amcntenclr1_el0,   4 |   FULL |        AMR | Arch_aarch64, "aarch64");
  DEF_REGISTER(     amcntenset0_el0,   5 |   FULL |        AMR | Arch_aarch64, "aarch64");
  DEF_REGISTER(     amcntenset1_el0,   6 |   FULL |        AMR | Arch_aarch64, "aarch64");
  DEF_REGISTER(            amcr_el0,   7 |   FULL |        AMR | Arch_aarch64, "aarch64");
  DEF_REGISTER(      amevcntr00_el0,   8 |   FULL |        AMR | Arch_aarch64, "aarch64");
  DEF_REGISTER(      amevcntr01_el0,   9 |   FULL |        AMR | Arch_aarch64, "aarch64");
  DEF_REGISTER(      amevcntr02_el0,  10 |   FULL |        AMR | Arch_aarch64, "aarch64");
  DEF_REGISTER(      amevcntr03_el0,  11 |   FULL |        AMR | Arch_aarch64, "aarch64");
  DEF_REGISTER(      amevcntr10_el0,  12 |   FULL |        AMR | Arch_aarch64, "aarch64");
  DEF_REGISTER(      amevcntr11_el0,  13 |   FULL |        AMR | Arch_aarch64, "aarch64");
  DEF_REGISTER(      amevcntr12_el0,  14 |   FULL |        AMR | Arch_aarch64, "aarch64");
  DEF_REGISTER(      amevcntr13_el0,  15 |   FULL |        AMR | Arch_aarch64, "aarch64");
  DEF_REGISTER(      amevcntr14_el0,  16 |   FULL |        AMR | Arch_aarch64, "aarch64");
  DEF_REGISTER(      amevcntr15_el0,  17 |   FULL |        AMR | Arch_aarch64, "aarch64");
  DEF_REGISTER(      amevcntr16_el0,  18 |   FULL |        AMR | Arch_aarch64, "aarch64");
  DEF_REGISTER(      amevcntr17_el0,  19 |   FULL |        AMR | Arch_aarch64, "aarch64");
  DEF_REGISTER(      amevcntr18_el0,  20 |   FULL |        AMR | Arch_aarch64, "aarch64");
  DEF_REGISTER(      amevcntr19_el0,  21 |   FULL |        AMR | Arch_aarch64, "aarch64");
  DEF_REGISTER(     amevcntr110_el0,  22 |   FULL |        AMR | Arch_aarch64, "aarch64");
  DEF_REGISTER(     amevcntr111_el0,  23 |   FULL |        AMR | Arch_aarch64, "aarch64");
  DEF_REGISTER(     amevcntr112_el0,  24 |   FULL |        AMR | Arch_aarch64, "aarch64");
  DEF_REGISTER(     amevcntr113_el0,  25 |   FULL |        AMR | Arch_aarch64, "aarch64");
  DEF_REGISTER(     amevcntr114_el0,  26 |   FULL |        AMR | Arch_aarch64, "aarch64");
  DEF_REGISTER(     amevcntr115_el0,  27 |   FULL |        AMR | Arch_aarch64, "aarch64");
  DEF_REGISTER(   amevcntvoff00_el2,  28 |   FULL |        AMR | Arch_aarch64, "aarch64");
  DEF_REGISTER(   amevcntvoff01_el2,  29 |   FULL |        AMR | Arch_aarch64, "aarch64");
  DEF_REGISTER(   amevcntvoff02_el2,  30 |   FULL |        AMR | Arch_aarch64, "aarch64");
  DEF_REGISTER(   amevcntvoff03_el2,  31 |   FULL |        AMR | Arch_aarch64, "aarch64");
  DEF_REGISTER(   amevcntvoff04_el2,  32 |   FULL |        AMR | Arch_aarch64, "aarch64");
  DEF_REGISTER(   amevcntvoff05_el2,  33 |   FULL |        AMR | Arch_aarch64, "aarch64");
  DEF_REGISTER(   amevcntvoff06_el2,  34 |   FULL |        AMR | Arch_aarch64, "aarch64");
  DEF_REGISTER(   amevcntvoff07_el2,  35 |   FULL |        AMR | Arch_aarch64, "aarch64");
  DEF_REGISTER(   amevcntvoff08_el2,  36 |   FULL |        AMR | Arch_aarch64, "aarch64");
  DEF_REGISTER(   amevcntvoff09_el2,  37 |   FULL |        AMR | Arch_aarch64, "aarch64");
  DEF_REGISTER(  amevcntvoff010_el2,  38 |   FULL |        AMR | Arch_aarch64, "aarch64");
  DEF_REGISTER(  amevcntvoff011_el2,  39 |   FULL |        AMR | Arch_aarch64, "aarch64");
  DEF_REGISTER(  amevcntvoff012_el2,  40 |   FULL |        AMR | Arch_aarch64, "aarch64");
  DEF_REGISTER(  amevcntvoff013_el2,  41 |   FULL |        AMR | Arch_aarch64, "aarch64");
  DEF_REGISTER(  amevcntvoff014_el2,  42 |   FULL |        AMR | Arch_aarch64, "aarch64");
  DEF_REGISTER(  amevcntvoff015_el2,  43 |   FULL |        AMR | Arch_aarch64, "aarch64");
  DEF_REGISTER(   amevcntvoff10_el2,  44 |   FULL |        AMR | Arch_aarch64, "aarch64");
  DEF_REGISTER(   amevcntvoff11_el2,  45 |   FULL |        AMR | Arch_aarch64, "aarch64");
  DEF_REGISTER(   amevcntvoff12_el2,  46 |   FULL |        AMR | Arch_aarch64, "aarch64");
  DEF_REGISTER(   amevcntvoff13_el2,  47 |   FULL |        AMR | Arch_aarch64, "aarch64");
  DEF_REGISTER(   amevcntvoff14_el2,  48 |   FULL |        AMR | Arch_aarch64, "aarch64");
  DEF_REGISTER(   amevcntvoff15_el2,  49 |   FULL |        AMR | Arch_aarch64, "aarch64");
  DEF_REGISTER(   amevcntvoff16_el2,  50 |   FULL |        AMR | Arch_aarch64, "aarch64");
  DEF_REGISTER(   amevcntvoff17_el2,  51 |   FULL |        AMR | Arch_aarch64, "aarch64");
  DEF_REGISTER(   amevcntvoff18_el2,  52 |   FULL |        AMR | Arch_aarch64, "aarch64");
  DEF_REGISTER(   amevcntvoff19_el2,  53 |   FULL |        AMR | Arch_aarch64, "aarch64");
  DEF_REGISTER(  amevcntvoff110_el2,  54 |   FULL |        AMR | Arch_aarch64, "aarch64");
  DEF_REGISTER(  amevcntvoff111_el2,  55 |   FULL |        AMR | Arch_aarch64, "aarch64");
  DEF_REGISTER(  amevcntvoff112_el2,  56 |   FULL |        AMR | Arch_aarch64, "aarch64");
  DEF_REGISTER(  amevcntvoff113_el2,  57 |   FULL |        AMR | Arch_aarch64, "aarch64");
  DEF_REGISTER(  amevcntvoff114_el2,  58 |   FULL |        AMR | Arch_aarch64, "aarch64");
  DEF_REGISTER(  amevcntvoff115_el2,  59 |   FULL |        AMR | Arch_aarch64, "aarch64");
  DEF_REGISTER(     amevtyper00_el0,  60 |   FULL |        AMR | Arch_aarch64, "aarch64");
  DEF_REGISTER(     amevtyper01_el0,  61 |   FULL |        AMR | Arch_aarch64, "aarch64");
  DEF_REGISTER(     amevtyper02_el0,  62 |   FULL |        AMR | Arch_aarch64, "aarch64");
  DEF_REGISTER(     amevtyper03_el0,  63 |   FULL |        AMR | Arch_aarch64, "aarch64");
  DEF_REGISTER(     amevtyper10_el0,  64 |   FULL |        AMR | Arch_aarch64, "aarch64");
  DEF_REGISTER(     amevtyper11_el0,  65 |   FULL |        AMR | Arch_aarch64, "aarch64");
  DEF_REGISTER(     amevtyper12_el0,  66 |   FULL |        AMR | Arch_aarch64, "aarch64");
  DEF_REGISTER(     amevtyper13_el0,  67 |   FULL |        AMR | Arch_aarch64, "aarch64");
  DEF_REGISTER(     amevtyper14_el0,  68 |   FULL |        AMR | Arch_aarch64, "aarch64");
  DEF_REGISTER(     amevtyper15_el0,  69 |   FULL |        AMR | Arch_aarch64, "aarch64");
  DEF_REGISTER(     amevtyper16_el0,  70 |   FULL |        AMR | Arch_aarch64, "aarch64");
  DEF_REGISTER(     amevtyper17_el0,  71 |   FULL |        AMR | Arch_aarch64, "aarch64");
  DEF_REGISTER(     amevtyper18_el0,  72 |   FULL |        AMR | Arch_aarch64, "aarch64");
  DEF_REGISTER(     amevtyper19_el0,  73 |   FULL |        AMR | Arch_aarch64, "aarch64");
  DEF_REGISTER(    amevtyper110_el0,  74 |   FULL |        AMR | Arch_aarch64, "aarch64");
  DEF_REGISTER(    amevtyper111_el0,  75 |   FULL |        AMR | Arch_aarch64, "aarch64");
  DEF_REGISTER(    amevtyper112_el0,  76 |   FULL |        AMR | Arch_aarch64, "aarch64");
  DEF_REGISTER(    amevtyper113_el0,  77 |   FULL |        AMR | Arch_aarch64, "aarch64");
  DEF_REGISTER(    amevtyper114_el0,  78 |   FULL |        AMR | Arch_aarch64, "aarch64");
  DEF_REGISTER(    amevtyper115_el0,  79 |   FULL |        AMR | Arch_aarch64, "aarch64");
  DEF_REGISTER(       amuserenr_el0,  80 |   FULL |        AMR | Arch_aarch64, "aarch64");
  DEF_REGISTER(       apdakeyhi_el1,   0 |   FULL |     SYSPTR | Arch_aarch64, "aarch64");
  DEF_REGISTER(       apdakeylo_el1,   1 |   FULL |     SYSPTR | Arch_aarch64, "aarch64");
  DEF_REGISTER(       apdbkeyhi_el1,   2 |   FULL |     SYSPTR | Arch_aarch64, "aarch64");
  DEF_REGISTER(       apdbkeylo_el1,   3 |   FULL |     SYSPTR | Arch_aarch64, "aarch64");
  DEF_REGISTER(       apgakeyhi_el1,   4 |   FULL |     SYSPTR | Arch_aarch64, "aarch64");
  DEF_REGISTER(       apgakeylo_el1,   5 |   FULL |     SYSPTR | Arch_aarch64, "aarch64");
  DEF_REGISTER(       apiakeyhi_el1,   6 |   FULL |     SYSPTR | Arch_aarch64, "aarch64");
  DEF_REGISTER(       apiakeylo_el1,   7 |   FULL |     SYSPTR | Arch_aarch64, "aarch64");
  DEF_REGISTER(       apibkeyhi_el1,   8 |   FULL |     SYSPTR | Arch_aarch64, "aarch64");
  DEF_REGISTER(       apibkeylo_el1,   9 |   FULL |     SYSPTR | Arch_aarch64, "aarch64");
  DEF_REGISTER(           brbcr_el1,   0 |   FULL |       BRBE | Arch_aarch64, "aarch64");
  DEF_REGISTER(           brbcr_el2,   1 |   FULL |       BRBE | Arch_aarch64, "aarch64");
  DEF_REGISTER(          brbfcr_el1,   2 |   FULL |       BRBE | Arch_aarch64, "aarch64");
  DEF_REGISTER(         brbidr0_el1,   3 |   FULL |       BRBE | Arch_aarch64, "aarch64");
  DEF_REGISTER(         brbinf0_el1,   4 |   FULL |       BRBE | Arch_aarch64, "aarch64");
  DEF_REGISTER(         brbinf1_el1,   5 |   FULL |       BRBE | Arch_aarch64, "aarch64");
  DEF_REGISTER(         brbinf2_el1,   6 |   FULL |       BRBE | Arch_aarch64, "aarch64");
  DEF_REGISTER(         brbinf3_el1,   7 |   FULL |       BRBE | Arch_aarch64, "aarch64");
  DEF_REGISTER(         brbinf4_el1,   8 |   FULL |       BRBE | Arch_aarch64, "aarch64");
  DEF_REGISTER(         brbinf5_el1,   9 |   FULL |       BRBE | Arch_aarch64, "aarch64");
  DEF_REGISTER(         brbinf6_el1,  10 |   FULL |       BRBE | Arch_aarch64, "aarch64");
  DEF_REGISTER(         brbinf7_el1,  11 |   FULL |       BRBE | Arch_aarch64, "aarch64");
  DEF_REGISTER(         brbinf8_el1,  12 |   FULL |       BRBE | Arch_aarch64, "aarch64");
  DEF_REGISTER(         brbinf9_el1,  13 |   FULL |       BRBE | Arch_aarch64, "aarch64");
  DEF_REGISTER(        brbinf10_el1,  14 |   FULL |       BRBE | Arch_aarch64, "aarch64");
  DEF_REGISTER(        brbinf11_el1,  15 |   FULL |       BRBE | Arch_aarch64, "aarch64");
  DEF_REGISTER(        brbinf12_el1,  16 |   FULL |       BRBE | Arch_aarch64, "aarch64");
  DEF_REGISTER(        brbinf13_el1,  17 |   FULL |       BRBE | Arch_aarch64, "aarch64");
  DEF_REGISTER(        brbinf14_el1,  18 |   FULL |       BRBE | Arch_aarch64, "aarch64");
  DEF_REGISTER(        brbinf15_el1,  19 |   FULL |       BRBE | Arch_aarch64, "aarch64");
  DEF_REGISTER(        brbinf16_el1,  20 |   FULL |       BRBE | Arch_aarch64, "aarch64");
  DEF_REGISTER(        brbinf17_el1,  21 |   FULL |       BRBE | Arch_aarch64, "aarch64");
  DEF_REGISTER(        brbinf18_el1,  22 |   FULL |       BRBE | Arch_aarch64, "aarch64");
  DEF_REGISTER(        brbinf19_el1,  23 |   FULL |       BRBE | Arch_aarch64, "aarch64");
  DEF_REGISTER(        brbinf20_el1,  24 |   FULL |       BRBE | Arch_aarch64, "aarch64");
  DEF_REGISTER(        brbinf21_el1,  25 |   FULL |       BRBE | Arch_aarch64, "aarch64");
  DEF_REGISTER(        brbinf22_el1,  26 |   FULL |       BRBE | Arch_aarch64, "aarch64");
  DEF_REGISTER(        brbinf23_el1,  27 |   FULL |       BRBE | Arch_aarch64, "aarch64");
  DEF_REGISTER(        brbinf24_el1,  28 |   FULL |       BRBE | Arch_aarch64, "aarch64");
  DEF_REGISTER(        brbinf25_el1,  29 |   FULL |       BRBE | Arch_aarch64, "aarch64");
  DEF_REGISTER(        brbinf26_el1,  30 |   FULL |       BRBE | Arch_aarch64, "aarch64");
  DEF_REGISTER(        brbinf27_el1,  31 |   FULL |       BRBE | Arch_aarch64, "aarch64");
  DEF_REGISTER(        brbinf28_el1,  32 |   FULL |       BRBE | Arch_aarch64, "aarch64");
  DEF_REGISTER(        brbinf29_el1,  33 |   FULL |       BRBE | Arch_aarch64, "aarch64");
  DEF_REGISTER(        brbinf30_el1,  34 |   FULL |       BRBE | Arch_aarch64, "aarch64");
  DEF_REGISTER(        brbinf31_el1,  35 |   FULL |       BRBE | Arch_aarch64, "aarch64");
  DEF_REGISTER(       brbinfinj_el1,  36 |   FULL |       BRBE | Arch_aarch64, "aarch64");
  DEF_REGISTER(         brbsrc0_el1,  37 |   FULL |       BRBE | Arch_aarch64, "aarch64");
  DEF_REGISTER(         brbsrc1_el1,  38 |   FULL |       BRBE | Arch_aarch64, "aarch64");
  DEF_REGISTER(         brbsrc2_el1,  39 |   FULL |       BRBE | Arch_aarch64, "aarch64");
  DEF_REGISTER(         brbsrc3_el1,  40 |   FULL |       BRBE | Arch_aarch64, "aarch64");
  DEF_REGISTER(         brbsrc4_el1,  41 |   FULL |       BRBE | Arch_aarch64, "aarch64");
  DEF_REGISTER(         brbsrc5_el1,  42 |   FULL |       BRBE | Arch_aarch64, "aarch64");
  DEF_REGISTER(         brbsrc6_el1,  43 |   FULL |       BRBE | Arch_aarch64, "aarch64");
  DEF_REGISTER(         brbsrc7_el1,  44 |   FULL |       BRBE | Arch_aarch64, "aarch64");
  DEF_REGISTER(         brbsrc8_el1,  45 |   FULL |       BRBE | Arch_aarch64, "aarch64");
  DEF_REGISTER(         brbsrc9_el1,  46 |   FULL |       BRBE | Arch_aarch64, "aarch64");
  DEF_REGISTER(        brbsrc10_el1,  47 |   FULL |       BRBE | Arch_aarch64, "aarch64");
  DEF_REGISTER(        brbsrc11_el1,  48 |   FULL |       BRBE | Arch_aarch64, "aarch64");
  DEF_REGISTER(        brbsrc12_el1,  49 |   FULL |       BRBE | Arch_aarch64, "aarch64");
  DEF_REGISTER(        brbsrc13_el1,  50 |   FULL |       BRBE | Arch_aarch64, "aarch64");
  DEF_REGISTER(        brbsrc14_el1,  51 |   FULL |       BRBE | Arch_aarch64, "aarch64");
  DEF_REGISTER(        brbsrc15_el1,  52 |   FULL |       BRBE | Arch_aarch64, "aarch64");
  DEF_REGISTER(        brbsrc16_el1,  53 |   FULL |       BRBE | Arch_aarch64, "aarch64");
  DEF_REGISTER(        brbsrc17_el1,  54 |   FULL |       BRBE | Arch_aarch64, "aarch64");
  DEF_REGISTER(        brbsrc18_el1,  55 |   FULL |       BRBE | Arch_aarch64, "aarch64");
  DEF_REGISTER(        brbsrc19_el1,  56 |   FULL |       BRBE | Arch_aarch64, "aarch64");
  DEF_REGISTER(        brbsrc20_el1,  57 |   FULL |       BRBE | Arch_aarch64, "aarch64");
  DEF_REGISTER(        brbsrc21_el1,  58 |   FULL |       BRBE | Arch_aarch64, "aarch64");
  DEF_REGISTER(        brbsrc22_el1,  59 |   FULL |       BRBE | Arch_aarch64, "aarch64");
  DEF_REGISTER(        brbsrc23_el1,  60 |   FULL |       BRBE | Arch_aarch64, "aarch64");
  DEF_REGISTER(        brbsrc24_el1,  61 |   FULL |       BRBE | Arch_aarch64, "aarch64");
  DEF_REGISTER(        brbsrc25_el1,  62 |   FULL |       BRBE | Arch_aarch64, "aarch64");
  DEF_REGISTER(        brbsrc26_el1,  63 |   FULL |       BRBE | Arch_aarch64, "aarch64");
  DEF_REGISTER(        brbsrc27_el1,  64 |   FULL |       BRBE | Arch_aarch64, "aarch64");
  DEF_REGISTER(        brbsrc28_el1,  65 |   FULL |       BRBE | Arch_aarch64, "aarch64");
  DEF_REGISTER(        brbsrc29_el1,  66 |   FULL |       BRBE | Arch_aarch64, "aarch64");
  DEF_REGISTER(        brbsrc30_el1,  67 |   FULL |       BRBE | Arch_aarch64, "aarch64");
  DEF_REGISTER(        brbsrc31_el1,  68 |   FULL |       BRBE | Arch_aarch64, "aarch64");
  DEF_REGISTER(       brbsrcinj_el1,  69 |   FULL |       BRBE | Arch_aarch64, "aarch64");
  DEF_REGISTER(         brbtgt0_el1,  70 |   FULL |       BRBE | Arch_aarch64, "aarch64");
  DEF_REGISTER(         brbtgt1_el1,  71 |   FULL |       BRBE | Arch_aarch64, "aarch64");
  DEF_REGISTER(         brbtgt2_el1,  72 |   FULL |       BRBE | Arch_aarch64, "aarch64");
  DEF_REGISTER(         brbtgt3_el1,  73 |   FULL |       BRBE | Arch_aarch64, "aarch64");
  DEF_REGISTER(         brbtgt4_el1,  74 |   FULL |       BRBE | Arch_aarch64, "aarch64");
  DEF_REGISTER(         brbtgt5_el1,  75 |   FULL |       BRBE | Arch_aarch64, "aarch64");
  DEF_REGISTER(         brbtgt6_el1,  76 |   FULL |       BRBE | Arch_aarch64, "aarch64");
  DEF_REGISTER(         brbtgt7_el1,  77 |   FULL |       BRBE | Arch_aarch64, "aarch64");
  DEF_REGISTER(         brbtgt8_el1,  78 |   FULL |       BRBE | Arch_aarch64, "aarch64");
  DEF_REGISTER(         brbtgt9_el1,  79 |   FULL |       BRBE | Arch_aarch64, "aarch64");
  DEF_REGISTER(        brbtgt10_el1,  80 |   FULL |       BRBE | Arch_aarch64, "aarch64");
  DEF_REGISTER(        brbtgt11_el1,  81 |   FULL |       BRBE | Arch_aarch64, "aarch64");
  DEF_REGISTER(        brbtgt12_el1,  82 |   FULL |       BRBE | Arch_aarch64, "aarch64");
  DEF_REGISTER(        brbtgt13_el1,  83 |   FULL |       BRBE | Arch_aarch64, "aarch64");
  DEF_REGISTER(        brbtgt14_el1,  84 |   FULL |       BRBE | Arch_aarch64, "aarch64");
  DEF_REGISTER(        brbtgt15_el1,  85 |   FULL |       BRBE | Arch_aarch64, "aarch64");
  DEF_REGISTER(        brbtgt16_el1,  86 |   FULL |       BRBE | Arch_aarch64, "aarch64");
  DEF_REGISTER(        brbtgt17_el1,  87 |   FULL |       BRBE | Arch_aarch64, "aarch64");
  DEF_REGISTER(        brbtgt18_el1,  88 |   FULL |       BRBE | Arch_aarch64, "aarch64");
  DEF_REGISTER(        brbtgt19_el1,  89 |   FULL |       BRBE | Arch_aarch64, "aarch64");
  DEF_REGISTER(        brbtgt20_el1,  90 |   FULL |       BRBE | Arch_aarch64, "aarch64");
  DEF_REGISTER(        brbtgt21_el1,  91 |   FULL |       BRBE | Arch_aarch64, "aarch64");
  DEF_REGISTER(        brbtgt22_el1,  92 |   FULL |       BRBE | Arch_aarch64, "aarch64");
  DEF_REGISTER(        brbtgt23_el1,  93 |   FULL |       BRBE | Arch_aarch64, "aarch64");
  DEF_REGISTER(        brbtgt24_el1,  94 |   FULL |       BRBE | Arch_aarch64, "aarch64");
  DEF_REGISTER(        brbtgt25_el1,  95 |   FULL |       BRBE | Arch_aarch64, "aarch64");
  DEF_REGISTER(        brbtgt26_el1,  96 |   FULL |       BRBE | Arch_aarch64, "aarch64");
  DEF_REGISTER(        brbtgt27_el1,  97 |   FULL |       BRBE | Arch_aarch64, "aarch64");
  DEF_REGISTER(        brbtgt28_el1,  98 |   FULL |       BRBE | Arch_aarch64, "aarch64");
  DEF_REGISTER(        brbtgt29_el1,  99 |   FULL |       BRBE | Arch_aarch64, "aarch64");
  DEF_REGISTER(        brbtgt30_el1, 100 |   FULL |       BRBE | Arch_aarch64, "aarch64");
  DEF_REGISTER(        brbtgt31_el1, 101 |   FULL |       BRBE | Arch_aarch64, "aarch64");
  DEF_REGISTER(       brbtgtinj_el1, 102 |   FULL |       BRBE | Arch_aarch64, "aarch64");
  DEF_REGISTER(           brbts_el1, 103 |   FULL |       BRBE | Arch_aarch64, "aarch64");
  DEF_REGISTER(         ccsidr2_el1,   0 |   FULL |      SYSID | Arch_aarch64, "aarch64");
  DEF_REGISTER(          ccsidr_el1,   1 |   FULL |      SYSID | Arch_aarch64, "aarch64");
  DEF_REGISTER(           clidr_el1,   2 |   FULL |      SYSID | Arch_aarch64, "aarch64");
  DEF_REGISTER(          cntfrq_el0,   0 |   FULL |   SYSTIMER | Arch_aarch64, "aarch64");
  DEF_REGISTER(         cnthctl_el2,   0 |   FULL |       VIRT | Arch_aarch64, "aarch64");
  DEF_REGISTER(       cnthp_ctl_el2,   1 |   FULL |       VIRT | Arch_aarch64, "aarch64");
  DEF_REGISTER(      cnthp_cval_el2,   2 |   FULL |       VIRT | Arch_aarch64, "aarch64");
  DEF_REGISTER(      cnthp_tval_el2,   3 |   FULL |       VIRT | Arch_aarch64, "aarch64");
  DEF_REGISTER(      cnthps_ctl_el2,   4 |   FULL |       VIRT | Arch_aarch64, "aarch64");
  DEF_REGISTER(     cnthps_cval_el2,   5 |   FULL |       VIRT | Arch_aarch64, "aarch64");
  DEF_REGISTER(     cnthps_tval_el2,   6 |   FULL |       VIRT | Arch_aarch64, "aarch64");
  DEF_REGISTER(       cnthv_ctl_el2,   1 |   FULL |   SYSTIMER | Arch_aarch64, "aarch64");
  DEF_REGISTER(      cnthv_cval_el2,   2 |   FULL |   SYSTIMER | Arch_aarch64, "aarch64");
  DEF_REGISTER(      cnthv_tval_el2,   3 |   FULL |   SYSTIMER | Arch_aarch64, "aarch64");
  DEF_REGISTER(      cnthvs_ctl_el2,   4 |   FULL |   SYSTIMER | Arch_aarch64, "aarch64");
  DEF_REGISTER(     cnthvs_cval_el2,   5 |   FULL |   SYSTIMER | Arch_aarch64, "aarch64");
  DEF_REGISTER(     cnthvs_tval_el2,   6 |   FULL |   SYSTIMER | Arch_aarch64, "aarch64");
  DEF_REGISTER(         cntkctl_el1,   7 |   FULL |   SYSTIMER | Arch_aarch64, "aarch64");
  DEF_REGISTER(        cntp_ctl_el0,   8 |   FULL |   SYSTIMER | Arch_aarch64, "aarch64");
  DEF_REGISTER(       cntp_cval_el0,   9 |   FULL |   SYSTIMER | Arch_aarch64, "aarch64");
  DEF_REGISTER(       cntp_tval_el0,  10 |   FULL |   SYSTIMER | Arch_aarch64, "aarch64");
  DEF_REGISTER(          cntpct_el0,  11 |   FULL |   SYSTIMER | Arch_aarch64, "aarch64");
  DEF_REGISTER(        cntpctss_el0,  12 |   FULL |   SYSTIMER | Arch_aarch64, "aarch64");
  DEF_REGISTER(         cntpoff_el2,  13 |   FULL |   SYSTIMER | Arch_aarch64, "aarch64");
  DEF_REGISTER(       cntps_ctl_el1,  14 |   FULL |   SYSTIMER | Arch_aarch64, "aarch64");
  DEF_REGISTER(      cntps_cval_el1,  15 |   FULL |   SYSTIMER | Arch_aarch64, "aarch64");
  DEF_REGISTER(      cntps_tval_el1,  16 |   FULL |   SYSTIMER | Arch_aarch64, "aarch64");
  DEF_REGISTER(        cntv_ctl_el0,  17 |   FULL |   SYSTIMER | Arch_aarch64, "aarch64");
  DEF_REGISTER(       cntv_cval_el0,  18 |   FULL |   SYSTIMER | Arch_aarch64, "aarch64");
  DEF_REGISTER(       cntv_tval_el0,  19 |   FULL |   SYSTIMER | Arch_aarch64, "aarch64");
  DEF_REGISTER(          cntvct_el0,  20 |   FULL |   SYSTIMER | Arch_aarch64, "aarch64");
  DEF_REGISTER(        cntvctss_el0,  21 |   FULL |   SYSTIMER | Arch_aarch64, "aarch64");
  DEF_REGISTER(         cntvoff_el2,   7 |   FULL |       VIRT | Arch_aarch64, "aarch64");
  DEF_REGISTER(           cpacr_el1,   0 |   FULL |      OTHER | Arch_aarch64, "aarch64");
  DEF_REGISTER(            cptr_el2,   8 |   FULL |       VIRT | Arch_aarch64, "aarch64");
  DEF_REGISTER(            cptr_el3,   0 |   FULL |        SEC | Arch_aarch64, "aarch64");
  DEF_REGISTER(          csselr_el1,   3 |   FULL |      SYSID | Arch_aarch64, "aarch64");
  DEF_REGISTER(             ctr_el0,   4 |   FULL |      SYSID | Arch_aarch64, "aarch64");
  DEF_REGISTER(           currentel,   1 |   FULL |     PSTATE | Arch_aarch64, "aarch64");
  DEF_REGISTER(                daif,   2 |   FULL |     PSTATE | Arch_aarch64, "aarch64");
  DEF_REGISTER(   dbgauthstatus_el1,   0 |  D_REG |    DBGAUTH | Arch_aarch64, "aarch64");
  DEF_REGISTER(         dbgbcr0_el1,   0 |   FULL |     DBGBRK | Arch_aarch64, "aarch64");
  DEF_REGISTER(         dbgbcr1_el1,   1 |   FULL |     DBGBRK | Arch_aarch64, "aarch64");
  DEF_REGISTER(         dbgbcr2_el1,   2 |   FULL |     DBGBRK | Arch_aarch64, "aarch64");
  DEF_REGISTER(         dbgbcr3_el1,   3 |   FULL |     DBGBRK | Arch_aarch64, "aarch64");
  DEF_REGISTER(         dbgbcr4_el1,   4 |   FULL |     DBGBRK | Arch_aarch64, "aarch64");
  DEF_REGISTER(         dbgbcr5_el1,   5 |   FULL |     DBGBRK | Arch_aarch64, "aarch64");
  DEF_REGISTER(         dbgbcr6_el1,   6 |   FULL |     DBGBRK | Arch_aarch64, "aarch64");
  DEF_REGISTER(         dbgbcr7_el1,   7 |   FULL |     DBGBRK | Arch_aarch64, "aarch64");
  DEF_REGISTER(         dbgbcr8_el1,   8 |   FULL |     DBGBRK | Arch_aarch64, "aarch64");
  DEF_REGISTER(         dbgbcr9_el1,   9 |   FULL |     DBGBRK | Arch_aarch64, "aarch64");
  DEF_REGISTER(        dbgbcr10_el1,  10 |   FULL |     DBGBRK | Arch_aarch64, "aarch64");
  DEF_REGISTER(        dbgbcr11_el1,  11 |   FULL |     DBGBRK | Arch_aarch64, "aarch64");
  DEF_REGISTER(        dbgbcr12_el1,  12 |   FULL |     DBGBRK | Arch_aarch64, "aarch64");
  DEF_REGISTER(        dbgbcr13_el1,  13 |   FULL |     DBGBRK | Arch_aarch64, "aarch64");
  DEF_REGISTER(        dbgbcr14_el1,  14 |   FULL |     DBGBRK | Arch_aarch64, "aarch64");
  DEF_REGISTER(        dbgbcr15_el1,  15 |   FULL |     DBGBRK | Arch_aarch64, "aarch64");
  DEF_REGISTER(         dbgbvr0_el1,  16 |   FULL |     DBGBRK | Arch_aarch64, "aarch64");
  DEF_REGISTER(         dbgbvr1_el1,  17 |   FULL |     DBGBRK | Arch_aarch64, "aarch64");
  DEF_REGISTER(         dbgbvr2_el1,  18 |   FULL |     DBGBRK | Arch_aarch64, "aarch64");
  DEF_REGISTER(         dbgbvr3_el1,  19 |   FULL |     DBGBRK | Arch_aarch64, "aarch64");
  DEF_REGISTER(         dbgbvr4_el1,  20 |   FULL |     DBGBRK | Arch_aarch64, "aarch64");
  DEF_REGISTER(         dbgbvr5_el1,  21 |   FULL |     DBGBRK | Arch_aarch64, "aarch64");
  DEF_REGISTER(         dbgbvr6_el1,  22 |   FULL |     DBGBRK | Arch_aarch64, "aarch64");
  DEF_REGISTER(         dbgbvr7_el1,  23 |   FULL |     DBGBRK | Arch_aarch64, "aarch64");
  DEF_REGISTER(         dbgbvr8_el1,  24 |   FULL |     DBGBRK | Arch_aarch64, "aarch64");
  DEF_REGISTER(         dbgbvr9_el1,  25 |   FULL |     DBGBRK | Arch_aarch64, "aarch64");
  DEF_REGISTER(        dbgbvr10_el1,  26 |   FULL |     DBGBRK | Arch_aarch64, "aarch64");
  DEF_REGISTER(        dbgbvr11_el1,  27 |   FULL |     DBGBRK | Arch_aarch64, "aarch64");
  DEF_REGISTER(        dbgbvr12_el1,  28 |   FULL |     DBGBRK | Arch_aarch64, "aarch64");
  DEF_REGISTER(        dbgbvr13_el1,  29 |   FULL |     DBGBRK | Arch_aarch64, "aarch64");
  DEF_REGISTER(        dbgbvr14_el1,  30 |   FULL |     DBGBRK | Arch_aarch64, "aarch64");
  DEF_REGISTER(        dbgbvr15_el1,  31 |   FULL |     DBGBRK | Arch_aarch64, "aarch64");
  DEF_REGISTER(     dbgclaimclr_el1,   0 |  D_REG |      DBGCT | Arch_aarch64, "aarch64");
  DEF_REGISTER(     dbgclaimset_el1,   1 |  D_REG |      DBGCT | Arch_aarch64, "aarch64");
  DEF_REGISTER(          dbgdtr_el0,   0 |   FULL |     DBGDTR | Arch_aarch64, "aarch64");
  DEF_REGISTER(        dbgdtrrx_el0,   1 |   FULL |     DBGDTR | Arch_aarch64, "aarch64");
  DEF_REGISTER(        dbgdtrtx_el0,   2 |   FULL |     DBGDTR | Arch_aarch64, "aarch64");
  DEF_REGISTER(         dbgprcr_el1,   0 |   FULL |     DBGPCR | Arch_aarch64, "aarch64");
  DEF_REGISTER(        dbgvcr32_el2,   0 |   FULL |       DVCR | Arch_aarch64, "aarch64");
  DEF_REGISTER(         dbgwcr0_el1,   0 |   FULL |       DBGW | Arch_aarch64, "aarch64");
  DEF_REGISTER(         dbgwcr1_el1,   1 |   FULL |       DBGW | Arch_aarch64, "aarch64");
  DEF_REGISTER(         dbgwcr2_el1,   2 |   FULL |       DBGW | Arch_aarch64, "aarch64");
  DEF_REGISTER(         dbgwcr3_el1,   3 |   FULL |       DBGW | Arch_aarch64, "aarch64");
  DEF_REGISTER(         dbgwcr4_el1,   4 |   FULL |       DBGW | Arch_aarch64, "aarch64");
  DEF_REGISTER(         dbgwcr5_el1,   5 |   FULL |       DBGW | Arch_aarch64, "aarch64");
  DEF_REGISTER(         dbgwcr6_el1,   6 |   FULL |       DBGW | Arch_aarch64, "aarch64");
  DEF_REGISTER(         dbgwcr7_el1,   7 |   FULL |       DBGW | Arch_aarch64, "aarch64");
  DEF_REGISTER(         dbgwcr8_el1,   8 |   FULL |       DBGW | Arch_aarch64, "aarch64");
  DEF_REGISTER(         dbgwcr9_el1,   9 |   FULL |       DBGW | Arch_aarch64, "aarch64");
  DEF_REGISTER(        dbgwcr10_el1,  10 |   FULL |       DBGW | Arch_aarch64, "aarch64");
  DEF_REGISTER(        dbgwcr11_el1,  11 |   FULL |       DBGW | Arch_aarch64, "aarch64");
  DEF_REGISTER(        dbgwcr12_el1,  12 |   FULL |       DBGW | Arch_aarch64, "aarch64");
  DEF_REGISTER(        dbgwcr13_el1,  13 |   FULL |       DBGW | Arch_aarch64, "aarch64");
  DEF_REGISTER(        dbgwcr14_el1,  14 |   FULL |       DBGW | Arch_aarch64, "aarch64");
  DEF_REGISTER(        dbgwcr15_el1,  15 |   FULL |       DBGW | Arch_aarch64, "aarch64");
  DEF_REGISTER(         dbgwvr0_el1,  16 |   FULL |       DBGW | Arch_aarch64, "aarch64");
  DEF_REGISTER(         dbgwvr1_el1,  17 |   FULL |       DBGW | Arch_aarch64, "aarch64");
  DEF_REGISTER(         dbgwvr2_el1,  18 |   FULL |       DBGW | Arch_aarch64, "aarch64");
  DEF_REGISTER(         dbgwvr3_el1,  19 |   FULL |       DBGW | Arch_aarch64, "aarch64");
  DEF_REGISTER(         dbgwvr4_el1,  20 |   FULL |       DBGW | Arch_aarch64, "aarch64");
  DEF_REGISTER(         dbgwvr5_el1,  21 |   FULL |       DBGW | Arch_aarch64, "aarch64");
  DEF_REGISTER(         dbgwvr6_el1,  22 |   FULL |       DBGW | Arch_aarch64, "aarch64");
  DEF_REGISTER(         dbgwvr7_el1,  23 |   FULL |       DBGW | Arch_aarch64, "aarch64");
  DEF_REGISTER(         dbgwvr8_el1,  24 |   FULL |       DBGW | Arch_aarch64, "aarch64");
  DEF_REGISTER(         dbgwvr9_el1,  25 |   FULL |       DBGW | Arch_aarch64, "aarch64");
  DEF_REGISTER(        dbgwvr10_el1,  26 |   FULL |       DBGW | Arch_aarch64, "aarch64");
  DEF_REGISTER(        dbgwvr11_el1,  27 |   FULL |       DBGW | Arch_aarch64, "aarch64");
  DEF_REGISTER(        dbgwvr12_el1,  28 |   FULL |       DBGW | Arch_aarch64, "aarch64");
  DEF_REGISTER(        dbgwvr13_el1,  29 |   FULL |       DBGW | Arch_aarch64, "aarch64");
  DEF_REGISTER(        dbgwvr14_el1,  30 |   FULL |       DBGW | Arch_aarch64, "aarch64");
  DEF_REGISTER(        dbgwvr15_el1,  31 |   FULL |       DBGW | Arch_aarch64, "aarch64");
  DEF_REGISTER(           dczid_el0,   5 |   FULL |      SYSID | Arch_aarch64, "aarch64");
  DEF_REGISTER(            disr_el1,   0 |   FULL |        RAS | Arch_aarch64, "aarch64");
  DEF_REGISTER(                 dit,   3 |   FULL |     PSTATE | Arch_aarch64, "aarch64");
  DEF_REGISTER(             dlr_el0,   0 |   FULL |      DBGLR | Arch_aarch64, "aarch64");
  DEF_REGISTER(           dspsr_el0,   0 |   FULL |    DBGSPSR | Arch_aarch64, "aarch64");
  DEF_REGISTER(          erridr_el1,   1 |   FULL |        RAS | Arch_aarch64, "aarch64");
  DEF_REGISTER(         errselr_el1,   2 |   FULL |        RAS | Arch_aarch64, "aarch64");
  DEF_REGISTER(         erxaddr_el1,   3 |   FULL |        RAS | Arch_aarch64, "aarch64");
  DEF_REGISTER(         erxctlr_el1,   4 |   FULL |        RAS | Arch_aarch64, "aarch64");
  DEF_REGISTER(           erxfr_el1,   5 |   FULL |        RAS | Arch_aarch64, "aarch64");
  DEF_REGISTER(          erxgsr_el1,   6 |   FULL |        RAS | Arch_aarch64, "aarch64");
  DEF_REGISTER(        erxmisc0_el1,   7 |   FULL |        RAS | Arch_aarch64, "aarch64");
  DEF_REGISTER(        erxmisc1_el1,   8 |   FULL |        RAS | Arch_aarch64, "aarch64");
  DEF_REGISTER(        erxmisc2_el1,   9 |   FULL |        RAS | Arch_aarch64, "aarch64");
  DEF_REGISTER(        erxmisc3_el1,  10 |   FULL |        RAS | Arch_aarch64, "aarch64");
  DEF_REGISTER(       erxpfgcdn_el1,  11 |   FULL |        RAS | Arch_aarch64, "aarch64");
  DEF_REGISTER(       erxpfgctl_el1,  12 |   FULL |        RAS | Arch_aarch64, "aarch64");
  DEF_REGISTER(         erxpfgf_el1,  13 |   FULL |        RAS | Arch_aarch64, "aarch64");
  DEF_REGISTER(       erxstatus_el1,  14 |   FULL |        RAS | Arch_aarch64, "aarch64");
  DEF_REGISTER(             esr_el1,   0 |   FULL |       EXCP | Arch_aarch64, "aarch64");
  DEF_REGISTER(             esr_el2,   9 |   FULL |       VIRT | Arch_aarch64, "aarch64");
  DEF_REGISTER(             esr_el3,   1 |   FULL |       EXCP | Arch_aarch64, "aarch64");
  DEF_REGISTER(             far_el1,   2 |   FULL |       EXCP | Arch_aarch64, "aarch64");
  DEF_REGISTER(             far_el2,  10 |   FULL |       VIRT | Arch_aarch64, "aarch64");
  DEF_REGISTER(             far_el3,   3 |   FULL |       EXCP | Arch_aarch64, "aarch64");
  DEF_REGISTER(          fgwte3_el3,   0 |   FULL |       FWTE | Arch_aarch64, "aarch64");
  DEF_REGISTER(         fpexc32_el2,   0 |   FULL |   SYSFLOAT | Arch_aarch64, "aarch64");
  DEF_REGISTER(                fpmr,   1 |   FULL |   SYSFLOAT | Arch_aarch64, "aarch64");
  DEF_REGISTER(                fpsr,   2 |   FULL |   SYSFLOAT | Arch_aarch64, "aarch64");
  DEF_REGISTER(             gcr_el1,   0 |   FULL |     SYSCTL | Arch_aarch64, "aarch64");
  DEF_REGISTER(           gcscr_el1,   0 |   FULL |       GCSR | Arch_aarch64, "aarch64");
  DEF_REGISTER(           gcscr_el2,   1 |   FULL |       GCSR | Arch_aarch64, "aarch64");
  DEF_REGISTER(           gcscr_el3,   2 |   FULL |       GCSR | Arch_aarch64, "aarch64");
  DEF_REGISTER(         gcscre0_el1,   3 |   FULL |       GCSR | Arch_aarch64, "aarch64");
  DEF_REGISTER(           gcspr_el0,   4 |   FULL |       GCSR | Arch_aarch64, "aarch64");
  DEF_REGISTER(           gcspr_el1,   5 |   FULL |       GCSR | Arch_aarch64, "aarch64");
  DEF_REGISTER(           gcspr_el2,   6 |   FULL |       GCSR | Arch_aarch64, "aarch64");
  DEF_REGISTER(           gcspr_el3,   7 |   FULL |       GCSR | Arch_aarch64, "aarch64");
  DEF_REGISTER(            gmid_el1,   6 |   FULL |      SYSID | Arch_aarch64, "aarch64");
  DEF_REGISTER(            hacr_el2,  10 |   FULL |    IMPLDEF | Arch_aarch64, "aarch64");
  DEF_REGISTER(         hafgrtr_el2,   0 |   FULL |    HYPRDBG | Arch_aarch64, "aarch64");
  DEF_REGISTER(             hcr_el2,  11 |   FULL |       VIRT | Arch_aarch64, "aarch64");
  DEF_REGISTER(            hcrx_el2,  12 |   FULL |       VIRT | Arch_aarch64, "aarch64");
  DEF_REGISTER(        hdfgrtr2_el2,   1 |   FULL |    HYPRDBG | Arch_aarch64, "aarch64");
  DEF_REGISTER(         hdfgrtr_el2,   2 |   FULL |    HYPRDBG | Arch_aarch64, "aarch64");
  DEF_REGISTER(        hdfgwtr2_el2,   3 |   FULL |    HYPRDBG | Arch_aarch64, "aarch64");
  DEF_REGISTER(         hdfgwtr_el2,   4 |   FULL |    HYPRDBG | Arch_aarch64, "aarch64");
  DEF_REGISTER(         hfgitr2_el2,  13 |   FULL |       VIRT | Arch_aarch64, "aarch64");
  DEF_REGISTER(          hfgitr_el2,   5 |   FULL |    HYPRDBG | Arch_aarch64, "aarch64");
  DEF_REGISTER(         hfgrtr2_el2,   6 |   FULL |    HYPRDBG | Arch_aarch64, "aarch64");
  DEF_REGISTER(          hfgrtr_el2,   7 |   FULL |    HYPRDBG | Arch_aarch64, "aarch64");
  DEF_REGISTER(         hfgwtr2_el2,   8 |   FULL |    HYPRDBG | Arch_aarch64, "aarch64");
  DEF_REGISTER(          hfgwtr_el2,   9 |   FULL |    HYPRDBG | Arch_aarch64, "aarch64");
  DEF_REGISTER(           hpfar_el2,  14 |   FULL |       VIRT | Arch_aarch64, "aarch64");
  DEF_REGISTER(            hstr_el2,  15 |   FULL |       VIRT | Arch_aarch64, "aarch64");
  DEF_REGISTER(       ich_ap0r0_el2,  16 |   FULL |       VIRT | Arch_aarch64, "aarch64");
  DEF_REGISTER(       ich_ap0r1_el2,  17 |   FULL |       VIRT | Arch_aarch64, "aarch64");
  DEF_REGISTER(       ich_ap0r2_el2,  18 |   FULL |       VIRT | Arch_aarch64, "aarch64");
  DEF_REGISTER(       ich_ap0r3_el2,  19 |   FULL |       VIRT | Arch_aarch64, "aarch64");
  DEF_REGISTER(       ich_ap1r0_el2,  20 |   FULL |       VIRT | Arch_aarch64, "aarch64");
  DEF_REGISTER(       ich_ap1r1_el2,  21 |   FULL |       VIRT | Arch_aarch64, "aarch64");
  DEF_REGISTER(       ich_ap1r2_el2,  22 |   FULL |       VIRT | Arch_aarch64, "aarch64");
  DEF_REGISTER(       ich_ap1r3_el2,  23 |   FULL |       VIRT | Arch_aarch64, "aarch64");
  DEF_REGISTER(        ich_eisr_el2,  24 |   FULL |       VIRT | Arch_aarch64, "aarch64");
  DEF_REGISTER(       ich_elrsr_el2,  25 |   FULL |       VIRT | Arch_aarch64, "aarch64");
  DEF_REGISTER(         ich_hcr_el2,  26 |   FULL |       VIRT | Arch_aarch64, "aarch64");
  DEF_REGISTER(         ich_lr0_el2,  27 |   FULL |       VIRT | Arch_aarch64, "aarch64");
  DEF_REGISTER(         ich_lr1_el2,  28 |   FULL |       VIRT | Arch_aarch64, "aarch64");
  DEF_REGISTER(         ich_lr2_el2,  29 |   FULL |       VIRT | Arch_aarch64, "aarch64");
  DEF_REGISTER(         ich_lr3_el2,  30 |   FULL |       VIRT | Arch_aarch64, "aarch64");
  DEF_REGISTER(         ich_lr4_el2,  31 |   FULL |       VIRT | Arch_aarch64, "aarch64");
  DEF_REGISTER(         ich_lr5_el2,  32 |   FULL |       VIRT | Arch_aarch64, "aarch64");
  DEF_REGISTER(         ich_lr6_el2,  33 |   FULL |       VIRT | Arch_aarch64, "aarch64");
  DEF_REGISTER(         ich_lr7_el2,  34 |   FULL |       VIRT | Arch_aarch64, "aarch64");
  DEF_REGISTER(         ich_lr8_el2,  35 |   FULL |       VIRT | Arch_aarch64, "aarch64");
  DEF_REGISTER(         ich_lr9_el2,  36 |   FULL |       VIRT | Arch_aarch64, "aarch64");
  DEF_REGISTER(        ich_lr10_el2,  37 |   FULL |       VIRT | Arch_aarch64, "aarch64");
  DEF_REGISTER(        ich_lr11_el2,  38 |   FULL |       VIRT | Arch_aarch64, "aarch64");
  DEF_REGISTER(        ich_lr12_el2,  39 |   FULL |       VIRT | Arch_aarch64, "aarch64");
  DEF_REGISTER(        ich_lr13_el2,  40 |   FULL |       VIRT | Arch_aarch64, "aarch64");
  DEF_REGISTER(        ich_lr14_el2,  41 |   FULL |       VIRT | Arch_aarch64, "aarch64");
  DEF_REGISTER(        ich_lr15_el2,  42 |   FULL |       VIRT | Arch_aarch64, "aarch64");
  DEF_REGISTER(        ich_misr_el2,  43 |   FULL |       VIRT | Arch_aarch64, "aarch64");
  DEF_REGISTER(        ich_vmcr_el2,  44 |   FULL |       VIRT | Arch_aarch64, "aarch64");
  DEF_REGISTER(         ich_vtr_el2,  45 |   FULL |       VIRT | Arch_aarch64, "aarch64");
  DEF_REGISTER(     id_aa64afr0_el1,   7 |   FULL |      SYSID | Arch_aarch64, "aarch64");
  DEF_REGISTER(     id_aa64afr1_el1,   8 |   FULL |      SYSID | Arch_aarch64, "aarch64");
  DEF_REGISTER(     id_aa64dfr0_el1,   9 |   FULL |      SYSID | Arch_aarch64, "aarch64");
  DEF_REGISTER(     id_aa64dfr1_el1,  10 |   FULL |      SYSID | Arch_aarch64, "aarch64");
  DEF_REGISTER(     id_aa64dfr2_el1,  11 |   FULL |      SYSID | Arch_aarch64, "aarch64");
  DEF_REGISTER(    id_aa64fpfr0_el1,  12 |   FULL |      SYSID | Arch_aarch64, "aarch64");
  DEF_REGISTER(    id_aa64isar0_el1,  13 |   FULL |      SYSID | Arch_aarch64, "aarch64");
  DEF_REGISTER(    id_aa64isar1_el1,  14 |   FULL |      SYSID | Arch_aarch64, "aarch64");
  DEF_REGISTER(    id_aa64isar2_el1,  15 |   FULL |      SYSID | Arch_aarch64, "aarch64");
  DEF_REGISTER(    id_aa64isar3_el1,  16 |   FULL |      SYSID | Arch_aarch64, "aarch64");
  DEF_REGISTER(    id_aa64mmfr0_el1,  17 |   FULL |      SYSID | Arch_aarch64, "aarch64");
  DEF_REGISTER(    id_aa64mmfr1_el1,  18 |   FULL |      SYSID | Arch_aarch64, "aarch64");
  DEF_REGISTER(    id_aa64mmfr2_el1,  19 |   FULL |      SYSID | Arch_aarch64, "aarch64");
  DEF_REGISTER(    id_aa64mmfr3_el1,  20 |   FULL |      SYSID | Arch_aarch64, "aarch64");
  DEF_REGISTER(    id_aa64mmfr4_el1,  21 |   FULL |      SYSID | Arch_aarch64, "aarch64");
  DEF_REGISTER(     id_aa64pfr0_el1,  22 |   FULL |      SYSID | Arch_aarch64, "aarch64");
  DEF_REGISTER(     id_aa64pfr1_el1,  23 |   FULL |      SYSID | Arch_aarch64, "aarch64");
  DEF_REGISTER(     id_aa64pfr2_el1,  24 |   FULL |      SYSID | Arch_aarch64, "aarch64");
  DEF_REGISTER(    id_aa64smfr0_el1,  25 |   FULL |      SYSID | Arch_aarch64, "aarch64");
  DEF_REGISTER(     id_aa64zfr0_el1,  26 |   FULL |      SYSID | Arch_aarch64, "aarch64");
  DEF_REGISTER(         id_afr0_el1,  27 |   FULL |      SYSID | Arch_aarch64, "aarch64");
  DEF_REGISTER(         id_dfr0_el1,  28 |   FULL |      SYSID | Arch_aarch64, "aarch64");
  DEF_REGISTER(         id_dfr1_el1,  29 |   FULL |      SYSID | Arch_aarch64, "aarch64");
  DEF_REGISTER(        id_isar0_el1,  30 |   FULL |      SYSID | Arch_aarch64, "aarch64");
  DEF_REGISTER(        id_isar1_el1,  31 |   FULL |      SYSID | Arch_aarch64, "aarch64");
  DEF_REGISTER(        id_isar2_el1,  32 |   FULL |      SYSID | Arch_aarch64, "aarch64");
  DEF_REGISTER(        id_isar3_el1,  33 |   FULL |      SYSID | Arch_aarch64, "aarch64");
  DEF_REGISTER(        id_isar4_el1,  34 |   FULL |      SYSID | Arch_aarch64, "aarch64");
  DEF_REGISTER(        id_isar5_el1,  35 |   FULL |      SYSID | Arch_aarch64, "aarch64");
  DEF_REGISTER(        id_isar6_el1,  36 |   FULL |      SYSID | Arch_aarch64, "aarch64");
  DEF_REGISTER(        id_mmfr0_el1,  37 |   FULL |      SYSID | Arch_aarch64, "aarch64");
  DEF_REGISTER(        id_mmfr1_el1,  38 |   FULL |      SYSID | Arch_aarch64, "aarch64");
  DEF_REGISTER(        id_mmfr2_el1,  39 |   FULL |      SYSID | Arch_aarch64, "aarch64");
  DEF_REGISTER(        id_mmfr3_el1,  40 |   FULL |      SYSID | Arch_aarch64, "aarch64");
  DEF_REGISTER(        id_mmfr4_el1,  41 |   FULL |      SYSID | Arch_aarch64, "aarch64");
  DEF_REGISTER(        id_mmfr5_el1,  42 |   FULL |      SYSID | Arch_aarch64, "aarch64");
  DEF_REGISTER(         id_pfr0_el1,  43 |   FULL |      SYSID | Arch_aarch64, "aarch64");
  DEF_REGISTER(         id_pfr1_el1,  44 |   FULL |      SYSID | Arch_aarch64, "aarch64");
  DEF_REGISTER(         id_pfr2_el1,  45 |   FULL |      SYSID | Arch_aarch64, "aarch64");
  DEF_REGISTER(          ifsr32_el2,   4 |   FULL |       EXCP | Arch_aarch64, "aarch64");
  DEF_REGISTER(             isr_el1,   5 |   FULL |       EXCP | Arch_aarch64, "aarch64");
  DEF_REGISTER(         mdccint_el1,   0 |   FULL |     SYSMON | Arch_aarch64, "aarch64");
  DEF_REGISTER(          mdccsr_el0,   1 |   FULL |     SYSMON | Arch_aarch64, "aarch64");
  DEF_REGISTER(            mdcr_el2,  46 |   FULL |       VIRT | Arch_aarch64, "aarch64");
  DEF_REGISTER(            mdcr_el3,   1 |   FULL |        SEC | Arch_aarch64, "aarch64");
  DEF_REGISTER(           mdrar_el1,   2 |   FULL |     SYSMON | Arch_aarch64, "aarch64");
  DEF_REGISTER(           mdscr_el1,   3 |   FULL |     SYSMON | Arch_aarch64, "aarch64");
  DEF_REGISTER(          mdselr_el1,   0 |   FULL |       BAWS | Arch_aarch64, "aarch64");
  DEF_REGISTER(        mdstepop_el1,   4 |   FULL |     SYSMON | Arch_aarch64, "aarch64");
  DEF_REGISTER(        mecid_a0_el2,   1 |   FULL |     SYSCTL | Arch_aarch64, "aarch64");
  DEF_REGISTER(        mecid_a1_el2,   2 |   FULL |     SYSCTL | Arch_aarch64, "aarch64");
  DEF_REGISTER(        mecid_p0_el2,   3 |   FULL |     SYSCTL | Arch_aarch64, "aarch64");
  DEF_REGISTER(        mecid_p1_el2,   4 |   FULL |     SYSCTL | Arch_aarch64, "aarch64");
  DEF_REGISTER(      mecid_rl_a_el3,   5 |   FULL |     SYSCTL | Arch_aarch64, "aarch64");
  DEF_REGISTER(          mecidr_el2,   6 |   FULL |     SYSCTL | Arch_aarch64, "aarch64");
  DEF_REGISTER(            mfar_el3,  15 |   FULL |        RAS | Arch_aarch64, "aarch64");
  DEF_REGISTER(            midr_el1,  46 |   FULL |      SYSID | Arch_aarch64, "aarch64");
  DEF_REGISTER(           mpam0_el1,   0 |   FULL |       MPAM | Arch_aarch64, "aarch64");
  DEF_REGISTER(           mpam1_el1,   1 |   FULL |       MPAM | Arch_aarch64, "aarch64");
  DEF_REGISTER(           mpam2_el2,   2 |   FULL |       MPAM | Arch_aarch64, "aarch64");
  DEF_REGISTER(           mpam3_el3,   3 |   FULL |       MPAM | Arch_aarch64, "aarch64");
  DEF_REGISTER(         mpamhcr_el2,   4 |   FULL |       MPAM | Arch_aarch64, "aarch64");
  DEF_REGISTER(         mpamidr_el1,  47 |   FULL |      SYSID | Arch_aarch64, "aarch64");
  DEF_REGISTER(          mpamsm_el1,   5 |   FULL |       MPAM | Arch_aarch64, "aarch64");
  DEF_REGISTER(        mpamvpm0_el2,   6 |   FULL |       MPAM | Arch_aarch64, "aarch64");
  DEF_REGISTER(        mpamvpm1_el2,   7 |   FULL |       MPAM | Arch_aarch64, "aarch64");
  DEF_REGISTER(        mpamvpm2_el2,   8 |   FULL |       MPAM | Arch_aarch64, "aarch64");
  DEF_REGISTER(        mpamvpm3_el2,   9 |   FULL |       MPAM | Arch_aarch64, "aarch64");
  DEF_REGISTER(        mpamvpm4_el2,  10 |   FULL |       MPAM | Arch_aarch64, "aarch64");
  DEF_REGISTER(        mpamvpm5_el2,  11 |   FULL |       MPAM | Arch_aarch64, "aarch64");
  DEF_REGISTER(        mpamvpm6_el2,  12 |   FULL |       MPAM | Arch_aarch64, "aarch64");
  DEF_REGISTER(        mpamvpm7_el2,  13 |   FULL |       MPAM | Arch_aarch64, "aarch64");
  DEF_REGISTER(        mpamvpmv_el2,  14 |   FULL |       MPAM | Arch_aarch64, "aarch64");
  DEF_REGISTER(           mpidr_el1,  48 |   FULL |      SYSID | Arch_aarch64, "aarch64");
  DEF_REGISTER(           mvfr0_el1,   3 |   FULL |   SYSFLOAT | Arch_aarch64, "aarch64");
  DEF_REGISTER(           mvfr1_el1,   4 |   FULL |   SYSFLOAT | Arch_aarch64, "aarch64");
  DEF_REGISTER(           mvfr2_el1,   5 |   FULL |   SYSFLOAT | Arch_aarch64, "aarch64");
  DEF_REGISTER(           osdlr_el1,   0 |   FULL |      SYSOS | Arch_aarch64, "aarch64");
  DEF_REGISTER(         osdtrrx_el1,   1 |   FULL |      SYSOS | Arch_aarch64, "aarch64");
  DEF_REGISTER(         osdtrtx_el1,   2 |   FULL |      SYSOS | Arch_aarch64, "aarch64");
  DEF_REGISTER(          oseccr_el1,   3 |   FULL |      SYSOS | Arch_aarch64, "aarch64");
  DEF_REGISTER(           oslar_el1,   4 |  D_REG |      SYSOS | Arch_aarch64, "aarch64");
  DEF_REGISTER(           oslsr_el1,   5 |   FULL |      SYSOS | Arch_aarch64, "aarch64");
  DEF_REGISTER(                 pan,   4 |   FULL |     PSTATE | Arch_aarch64, "aarch64");
  DEF_REGISTER(             par_el1,   0 |  Q_REG |       ADDR | Arch_aarch64, "aarch64");
  DEF_REGISTER(            pfar_el1,   0 |   FULL |    PHYSFAR | Arch_aarch64, "aarch64");
  DEF_REGISTER(            pfar_el2,   1 |   FULL |    PHYSFAR | Arch_aarch64, "aarch64");
  DEF_REGISTER(                  pm,   5 |   FULL |     PSTATE | Arch_aarch64, "aarch64");
  DEF_REGISTER(          pmbidr_el1,   0 |   FULL |   STATPROF | Arch_aarch64, "aarch64");
  DEF_REGISTER(       pmblimitr_el1,   1 |   FULL |   STATPROF | Arch_aarch64, "aarch64");
  DEF_REGISTER(          pmbptr_el1,   2 |   FULL |   STATPROF | Arch_aarch64, "aarch64");
  DEF_REGISTER(           pmbsr_el1,   3 |   FULL |   STATPROF | Arch_aarch64, "aarch64");
  DEF_REGISTER(       pmccfiltr_el0,   0 |   FULL |        PMU | Arch_aarch64, "aarch64");
  DEF_REGISTER(         pmccntr_el0,   1 |   FULL |        PMU | Arch_aarch64, "aarch64");
  DEF_REGISTER(       pmccntsvr_el1,   2 |   FULL |        PMU | Arch_aarch64, "aarch64");
  DEF_REGISTER(         pmceid0_el0,   3 |   FULL |        PMU | Arch_aarch64, "aarch64");
  DEF_REGISTER(         pmceid1_el0,   4 |   FULL |        PMU | Arch_aarch64, "aarch64");
  DEF_REGISTER(      pmcntenclr_el0,   5 |   FULL |        PMU | Arch_aarch64, "aarch64");
  DEF_REGISTER(      pmcntenset_el0,   6 |   FULL |        PMU | Arch_aarch64, "aarch64");
  DEF_REGISTER(            pmcr_el0,   7 |   FULL |        PMU | Arch_aarch64, "aarch64");
  DEF_REGISTER(           pmecr_el1,   8 |   FULL |        PMU | Arch_aarch64, "aarch64");
  DEF_REGISTER(       pmevcntr0_el0,   9 |   FULL |        PMU | Arch_aarch64, "aarch64");
  DEF_REGISTER(       pmevcntr1_el0,  10 |   FULL |        PMU | Arch_aarch64, "aarch64");
  DEF_REGISTER(       pmevcntr2_el0,  11 |   FULL |        PMU | Arch_aarch64, "aarch64");
  DEF_REGISTER(       pmevcntr3_el0,  12 |   FULL |        PMU | Arch_aarch64, "aarch64");
  DEF_REGISTER(       pmevcntr4_el0,  13 |   FULL |        PMU | Arch_aarch64, "aarch64");
  DEF_REGISTER(       pmevcntr5_el0,  14 |   FULL |        PMU | Arch_aarch64, "aarch64");
  DEF_REGISTER(       pmevcntr6_el0,  15 |   FULL |        PMU | Arch_aarch64, "aarch64");
  DEF_REGISTER(       pmevcntr7_el0,  16 |   FULL |        PMU | Arch_aarch64, "aarch64");
  DEF_REGISTER(       pmevcntr8_el0,  17 |   FULL |        PMU | Arch_aarch64, "aarch64");
  DEF_REGISTER(       pmevcntr9_el0,  18 |   FULL |        PMU | Arch_aarch64, "aarch64");
  DEF_REGISTER(      pmevcntr10_el0,  19 |   FULL |        PMU | Arch_aarch64, "aarch64");
  DEF_REGISTER(      pmevcntr11_el0,  20 |   FULL |        PMU | Arch_aarch64, "aarch64");
  DEF_REGISTER(      pmevcntr12_el0,  21 |   FULL |        PMU | Arch_aarch64, "aarch64");
  DEF_REGISTER(      pmevcntr13_el0,  22 |   FULL |        PMU | Arch_aarch64, "aarch64");
  DEF_REGISTER(      pmevcntr14_el0,  23 |   FULL |        PMU | Arch_aarch64, "aarch64");
  DEF_REGISTER(      pmevcntr15_el0,  24 |   FULL |        PMU | Arch_aarch64, "aarch64");
  DEF_REGISTER(      pmevcntr16_el0,  25 |   FULL |        PMU | Arch_aarch64, "aarch64");
  DEF_REGISTER(      pmevcntr17_el0,  26 |   FULL |        PMU | Arch_aarch64, "aarch64");
  DEF_REGISTER(      pmevcntr18_el0,  27 |   FULL |        PMU | Arch_aarch64, "aarch64");
  DEF_REGISTER(      pmevcntr19_el0,  28 |   FULL |        PMU | Arch_aarch64, "aarch64");
  DEF_REGISTER(      pmevcntr20_el0,  29 |   FULL |        PMU | Arch_aarch64, "aarch64");
  DEF_REGISTER(      pmevcntr21_el0,  30 |   FULL |        PMU | Arch_aarch64, "aarch64");
  DEF_REGISTER(      pmevcntr22_el0,  31 |   FULL |        PMU | Arch_aarch64, "aarch64");
  DEF_REGISTER(      pmevcntr23_el0,  32 |   FULL |        PMU | Arch_aarch64, "aarch64");
  DEF_REGISTER(      pmevcntr24_el0,  33 |   FULL |        PMU | Arch_aarch64, "aarch64");
  DEF_REGISTER(      pmevcntr25_el0,  34 |   FULL |        PMU | Arch_aarch64, "aarch64");
  DEF_REGISTER(      pmevcntr26_el0,  35 |   FULL |        PMU | Arch_aarch64, "aarch64");
  DEF_REGISTER(      pmevcntr27_el0,  36 |   FULL |        PMU | Arch_aarch64, "aarch64");
  DEF_REGISTER(      pmevcntr28_el0,  37 |   FULL |        PMU | Arch_aarch64, "aarch64");
  DEF_REGISTER(      pmevcntr29_el0,  38 |   FULL |        PMU | Arch_aarch64, "aarch64");
  DEF_REGISTER(      pmevcntr30_el0,  39 |   FULL |        PMU | Arch_aarch64, "aarch64");
  DEF_REGISTER(     pmevcntsvr0_el1,  40 |   FULL |        PMU | Arch_aarch64, "aarch64");
  DEF_REGISTER(     pmevcntsvr1_el1,  41 |   FULL |        PMU | Arch_aarch64, "aarch64");
  DEF_REGISTER(     pmevcntsvr2_el1,  42 |   FULL |        PMU | Arch_aarch64, "aarch64");
  DEF_REGISTER(     pmevcntsvr3_el1,  43 |   FULL |        PMU | Arch_aarch64, "aarch64");
  DEF_REGISTER(     pmevcntsvr4_el1,  44 |   FULL |        PMU | Arch_aarch64, "aarch64");
  DEF_REGISTER(     pmevcntsvr5_el1,  45 |   FULL |        PMU | Arch_aarch64, "aarch64");
  DEF_REGISTER(     pmevcntsvr6_el1,  46 |   FULL |        PMU | Arch_aarch64, "aarch64");
  DEF_REGISTER(     pmevcntsvr7_el1,  47 |   FULL |        PMU | Arch_aarch64, "aarch64");
  DEF_REGISTER(     pmevcntsvr8_el1,  48 |   FULL |        PMU | Arch_aarch64, "aarch64");
  DEF_REGISTER(     pmevcntsvr9_el1,  49 |   FULL |        PMU | Arch_aarch64, "aarch64");
  DEF_REGISTER(    pmevcntsvr10_el1,  50 |   FULL |        PMU | Arch_aarch64, "aarch64");
  DEF_REGISTER(    pmevcntsvr11_el1,  51 |   FULL |        PMU | Arch_aarch64, "aarch64");
  DEF_REGISTER(    pmevcntsvr12_el1,  52 |   FULL |        PMU | Arch_aarch64, "aarch64");
  DEF_REGISTER(    pmevcntsvr13_el1,  53 |   FULL |        PMU | Arch_aarch64, "aarch64");
  DEF_REGISTER(    pmevcntsvr14_el1,  54 |   FULL |        PMU | Arch_aarch64, "aarch64");
  DEF_REGISTER(    pmevcntsvr15_el1,  55 |   FULL |        PMU | Arch_aarch64, "aarch64");
  DEF_REGISTER(    pmevcntsvr16_el1,  56 |   FULL |        PMU | Arch_aarch64, "aarch64");
  DEF_REGISTER(    pmevcntsvr17_el1,  57 |   FULL |        PMU | Arch_aarch64, "aarch64");
  DEF_REGISTER(    pmevcntsvr18_el1,  58 |   FULL |        PMU | Arch_aarch64, "aarch64");
  DEF_REGISTER(    pmevcntsvr19_el1,  59 |   FULL |        PMU | Arch_aarch64, "aarch64");
  DEF_REGISTER(    pmevcntsvr20_el1,  60 |   FULL |        PMU | Arch_aarch64, "aarch64");
  DEF_REGISTER(    pmevcntsvr21_el1,  61 |   FULL |        PMU | Arch_aarch64, "aarch64");
  DEF_REGISTER(    pmevcntsvr22_el1,  62 |   FULL |        PMU | Arch_aarch64, "aarch64");
  DEF_REGISTER(    pmevcntsvr23_el1,  63 |   FULL |        PMU | Arch_aarch64, "aarch64");
  DEF_REGISTER(    pmevcntsvr24_el1,  64 |   FULL |        PMU | Arch_aarch64, "aarch64");
  DEF_REGISTER(    pmevcntsvr25_el1,  65 |   FULL |        PMU | Arch_aarch64, "aarch64");
  DEF_REGISTER(    pmevcntsvr26_el1,  66 |   FULL |        PMU | Arch_aarch64, "aarch64");
  DEF_REGISTER(    pmevcntsvr27_el1,  67 |   FULL |        PMU | Arch_aarch64, "aarch64");
  DEF_REGISTER(    pmevcntsvr28_el1,  68 |   FULL |        PMU | Arch_aarch64, "aarch64");
  DEF_REGISTER(    pmevcntsvr29_el1,  69 |   FULL |        PMU | Arch_aarch64, "aarch64");
  DEF_REGISTER(    pmevcntsvr30_el1,  70 |   FULL |        PMU | Arch_aarch64, "aarch64");
  DEF_REGISTER(      pmevtyper0_el0,  71 |   FULL |        PMU | Arch_aarch64, "aarch64");
  DEF_REGISTER(      pmevtyper1_el0,  72 |   FULL |        PMU | Arch_aarch64, "aarch64");
  DEF_REGISTER(      pmevtyper2_el0,  73 |   FULL |        PMU | Arch_aarch64, "aarch64");
  DEF_REGISTER(      pmevtyper3_el0,  74 |   FULL |        PMU | Arch_aarch64, "aarch64");
  DEF_REGISTER(      pmevtyper4_el0,  75 |   FULL |        PMU | Arch_aarch64, "aarch64");
  DEF_REGISTER(      pmevtyper5_el0,  76 |   FULL |        PMU | Arch_aarch64, "aarch64");
  DEF_REGISTER(      pmevtyper6_el0,  77 |   FULL |        PMU | Arch_aarch64, "aarch64");
  DEF_REGISTER(      pmevtyper7_el0,  78 |   FULL |        PMU | Arch_aarch64, "aarch64");
  DEF_REGISTER(      pmevtyper8_el0,  79 |   FULL |        PMU | Arch_aarch64, "aarch64");
  DEF_REGISTER(      pmevtyper9_el0,  80 |   FULL |        PMU | Arch_aarch64, "aarch64");
  DEF_REGISTER(     pmevtyper10_el0,  81 |   FULL |        PMU | Arch_aarch64, "aarch64");
  DEF_REGISTER(     pmevtyper11_el0,  82 |   FULL |        PMU | Arch_aarch64, "aarch64");
  DEF_REGISTER(     pmevtyper12_el0,  83 |   FULL |        PMU | Arch_aarch64, "aarch64");
  DEF_REGISTER(     pmevtyper13_el0,  84 |   FULL |        PMU | Arch_aarch64, "aarch64");
  DEF_REGISTER(     pmevtyper14_el0,  85 |   FULL |        PMU | Arch_aarch64, "aarch64");
  DEF_REGISTER(     pmevtyper15_el0,  86 |   FULL |        PMU | Arch_aarch64, "aarch64");
  DEF_REGISTER(     pmevtyper16_el0,  87 |   FULL |        PMU | Arch_aarch64, "aarch64");
  DEF_REGISTER(     pmevtyper17_el0,  88 |   FULL |        PMU | Arch_aarch64, "aarch64");
  DEF_REGISTER(     pmevtyper18_el0,  89 |   FULL |        PMU | Arch_aarch64, "aarch64");
  DEF_REGISTER(     pmevtyper19_el0,  90 |   FULL |        PMU | Arch_aarch64, "aarch64");
  DEF_REGISTER(     pmevtyper20_el0,  91 |   FULL |        PMU | Arch_aarch64, "aarch64");
  DEF_REGISTER(     pmevtyper21_el0,  92 |   FULL |        PMU | Arch_aarch64, "aarch64");
  DEF_REGISTER(     pmevtyper22_el0,  93 |   FULL |        PMU | Arch_aarch64, "aarch64");
  DEF_REGISTER(     pmevtyper23_el0,  94 |   FULL |        PMU | Arch_aarch64, "aarch64");
  DEF_REGISTER(     pmevtyper24_el0,  95 |   FULL |        PMU | Arch_aarch64, "aarch64");
  DEF_REGISTER(     pmevtyper25_el0,  96 |   FULL |        PMU | Arch_aarch64, "aarch64");
  DEF_REGISTER(     pmevtyper26_el0,  97 |   FULL |        PMU | Arch_aarch64, "aarch64");
  DEF_REGISTER(     pmevtyper27_el0,  98 |   FULL |        PMU | Arch_aarch64, "aarch64");
  DEF_REGISTER(     pmevtyper28_el0,  99 |   FULL |        PMU | Arch_aarch64, "aarch64");
  DEF_REGISTER(     pmevtyper29_el0, 100 |   FULL |        PMU | Arch_aarch64, "aarch64");
  DEF_REGISTER(     pmevtyper30_el0, 101 |   FULL |        PMU | Arch_aarch64, "aarch64");
  DEF_REGISTER(           pmiar_el1, 102 |   FULL |        PMU | Arch_aarch64, "aarch64");
  DEF_REGISTER(       pmicfiltr_el0, 103 |   FULL |        PMU | Arch_aarch64, "aarch64");
  DEF_REGISTER(         pmicntr_el0, 104 |   FULL |        PMU | Arch_aarch64, "aarch64");
  DEF_REGISTER(       pmicntsvr_el1, 105 |   FULL |        PMU | Arch_aarch64, "aarch64");
  DEF_REGISTER(      pmintenclr_el1, 106 |   FULL |        PMU | Arch_aarch64, "aarch64");
  DEF_REGISTER(      pmintenset_el1, 107 |   FULL |        PMU | Arch_aarch64, "aarch64");
  DEF_REGISTER(           pmmir_el1, 108 |   FULL |        PMU | Arch_aarch64, "aarch64");
  DEF_REGISTER(        pmovsclr_el0, 109 |   FULL |        PMU | Arch_aarch64, "aarch64");
  DEF_REGISTER(        pmovsset_el0, 110 |   FULL |        PMU | Arch_aarch64, "aarch64");
  DEF_REGISTER(           pmscr_el1,   4 |   FULL |   STATPROF | Arch_aarch64, "aarch64");
  DEF_REGISTER(           pmscr_el2,   5 |   FULL |   STATPROF | Arch_aarch64, "aarch64");
  DEF_REGISTER(         pmsdsfr_el1,   6 |   FULL |   STATPROF | Arch_aarch64, "aarch64");
  DEF_REGISTER(          pmselr_el0, 111 |   FULL |        PMU | Arch_aarch64, "aarch64");
  DEF_REGISTER(         pmsevfr_el1,   7 |   FULL |   STATPROF | Arch_aarch64, "aarch64");
  DEF_REGISTER(          pmsfcr_el1,   8 |   FULL |   STATPROF | Arch_aarch64, "aarch64");
  DEF_REGISTER(          pmsicr_el1,   9 |   FULL |   STATPROF | Arch_aarch64, "aarch64");
  DEF_REGISTER(          pmsidr_el1,  10 |   FULL |   STATPROF | Arch_aarch64, "aarch64");
  DEF_REGISTER(          pmsirr_el1,  11 |   FULL |   STATPROF | Arch_aarch64, "aarch64");
  DEF_REGISTER(        pmslatfr_el1,  12 |   FULL |   STATPROF | Arch_aarch64, "aarch64");
  DEF_REGISTER(        pmsnevfr_el1,  13 |   FULL |   STATPROF | Arch_aarch64, "aarch64");
  DEF_REGISTER(          pmsscr_el1, 112 |   FULL |        PMU | Arch_aarch64, "aarch64");
  DEF_REGISTER(         pmswinc_el0, 113 |   FULL |        PMU | Arch_aarch64, "aarch64");
  DEF_REGISTER(          pmuacr_el1, 114 |   FULL |        PMU | Arch_aarch64, "aarch64");
  DEF_REGISTER(       pmuserenr_el0, 115 |   FULL |        PMU | Arch_aarch64, "aarch64");
  DEF_REGISTER(       pmxevcntr_el0, 116 |   FULL |        PMU | Arch_aarch64, "aarch64");
  DEF_REGISTER(      pmxevtyper_el0, 117 |   FULL |        PMU | Arch_aarch64, "aarch64");
  DEF_REGISTER(            pmzr_el0, 118 |   FULL |        PMU | Arch_aarch64, "aarch64");
  DEF_REGISTER(          revidr_el1,  49 |   FULL |      SYSID | Arch_aarch64, "aarch64");
  DEF_REGISTER(            rgsr_el1,   7 |   FULL |     SYSCTL | Arch_aarch64, "aarch64");
  DEF_REGISTER(             rmr_el1,   0 |   FULL |      RESET | Arch_aarch64, "aarch64");
  DEF_REGISTER(             rmr_el2,  47 |   FULL |       VIRT | Arch_aarch64, "aarch64");
  DEF_REGISTER(             rmr_el3,   1 |   FULL |      RESET | Arch_aarch64, "aarch64");
  DEF_REGISTER(                rndr,   8 |   FULL |     SYSCTL | Arch_aarch64, "aarch64");
  DEF_REGISTER(              rndrrs,   9 |   FULL |     SYSCTL | Arch_aarch64, "aarch64");
  DEF_REGISTER(           rvbar_el1,   2 |   FULL |      RESET | Arch_aarch64, "aarch64");
  DEF_REGISTER(           rvbar_el2,   3 |   FULL |      RESET | Arch_aarch64, "aarch64");
  DEF_REGISTER(           rvbar_el3,   4 |   FULL |      RESET | Arch_aarch64, "aarch64");
  DEF_REGISTER(             scr_el3,   2 |   FULL |        SEC | Arch_aarch64, "aarch64");
  DEF_REGISTER(          sctlr2_el1,  10 |   FULL |     SYSCTL | Arch_aarch64, "aarch64");
  DEF_REGISTER(          sctlr2_el2,  48 |   FULL |       VIRT | Arch_aarch64, "aarch64");
  DEF_REGISTER(          sctlr2_el3,  11 |   FULL |     SYSCTL | Arch_aarch64, "aarch64");
  DEF_REGISTER(           sctlr_el1,  12 |   FULL |     SYSCTL | Arch_aarch64, "aarch64");
  DEF_REGISTER(           sctlr_el2,  49 |   FULL |       VIRT | Arch_aarch64, "aarch64");
  DEF_REGISTER(           sctlr_el3,  13 |   FULL |     SYSCTL | Arch_aarch64, "aarch64");
  DEF_REGISTER(         scxtnum_el0,   0 |   FULL |       THRD | Arch_aarch64, "aarch64");
  DEF_REGISTER(         scxtnum_el1,   1 |   FULL |       THRD | Arch_aarch64, "aarch64");
  DEF_REGISTER(         scxtnum_el2,   2 |   FULL |       THRD | Arch_aarch64, "aarch64");
  DEF_REGISTER(         scxtnum_el3,   3 |   FULL |       THRD | Arch_aarch64, "aarch64");
  DEF_REGISTER(          sder32_el2,  14 |   FULL |     SYSCTL | Arch_aarch64, "aarch64");
  DEF_REGISTER(          sder32_el3,   3 |   FULL |        SEC | Arch_aarch64, "aarch64");
  DEF_REGISTER(            smcr_el1,   1 |   FULL |      OTHER | Arch_aarch64, "aarch64");
  DEF_REGISTER(            smcr_el2,   2 |   FULL |      OTHER | Arch_aarch64, "aarch64");
  DEF_REGISTER(            smcr_el3,   3 |   FULL |      OTHER | Arch_aarch64, "aarch64");
  DEF_REGISTER(           smidr_el1,  50 |   FULL |      SYSID | Arch_aarch64, "aarch64");
  DEF_REGISTER(           smpri_el1,   4 |   FULL |      OTHER | Arch_aarch64, "aarch64");
  DEF_REGISTER(        smprimap_el2,   5 |   FULL |      OTHER | Arch_aarch64, "aarch64");
  DEF_REGISTER(      spmaccessr_el1, 119 |   FULL |        PMU | Arch_aarch64, "aarch64");
  DEF_REGISTER(      spmaccessr_el2, 120 |   FULL |        PMU | Arch_aarch64, "aarch64");
  DEF_REGISTER(      spmaccessr_el3, 121 |   FULL |        PMU | Arch_aarch64, "aarch64");
  DEF_REGISTER(         spmcfgr_el1, 122 |   FULL |        PMU | Arch_aarch64, "aarch64");
  DEF_REGISTER(        spmcgcr0_el1, 123 |   FULL |        PMU | Arch_aarch64, "aarch64");
  DEF_REGISTER(        spmcgcr1_el1, 124 |   FULL |        PMU | Arch_aarch64, "aarch64");
  DEF_REGISTER(     spmcntenclr_el0, 125 |   FULL |        PMU | Arch_aarch64, "aarch64");
  DEF_REGISTER(     spmcntenset_el0, 126 |   FULL |        PMU | Arch_aarch64, "aarch64");
  DEF_REGISTER(           spmcr_el0, 127 |   FULL |        PMU | Arch_aarch64, "aarch64");
  DEF_REGISTER(       spmdevaff_el1, 128 |   FULL |        PMU | Arch_aarch64, "aarch64");
  DEF_REGISTER(      spmdevarch_el1, 129 |   FULL |        PMU | Arch_aarch64, "aarch64");
  DEF_REGISTER(      spmevcntr0_el0, 130 |   FULL |        PMU | Arch_aarch64, "aarch64");
  DEF_REGISTER(      spmevcntr1_el0, 131 |   FULL |        PMU | Arch_aarch64, "aarch64");
  DEF_REGISTER(      spmevcntr2_el0, 132 |   FULL |        PMU | Arch_aarch64, "aarch64");
  DEF_REGISTER(      spmevcntr3_el0, 133 |   FULL |        PMU | Arch_aarch64, "aarch64");
  DEF_REGISTER(      spmevcntr4_el0, 134 |   FULL |        PMU | Arch_aarch64, "aarch64");
  DEF_REGISTER(      spmevcntr5_el0, 135 |   FULL |        PMU | Arch_aarch64, "aarch64");
  DEF_REGISTER(      spmevcntr6_el0, 136 |   FULL |        PMU | Arch_aarch64, "aarch64");
  DEF_REGISTER(      spmevcntr7_el0, 137 |   FULL |        PMU | Arch_aarch64, "aarch64");
  DEF_REGISTER(      spmevcntr8_el0, 138 |   FULL |        PMU | Arch_aarch64, "aarch64");
  DEF_REGISTER(      spmevcntr9_el0, 139 |   FULL |        PMU | Arch_aarch64, "aarch64");
  DEF_REGISTER(     spmevcntr10_el0, 140 |   FULL |        PMU | Arch_aarch64, "aarch64");
  DEF_REGISTER(     spmevcntr11_el0, 141 |   FULL |        PMU | Arch_aarch64, "aarch64");
  DEF_REGISTER(     spmevcntr12_el0, 142 |   FULL |        PMU | Arch_aarch64, "aarch64");
  DEF_REGISTER(     spmevcntr13_el0, 143 |   FULL |        PMU | Arch_aarch64, "aarch64");
  DEF_REGISTER(     spmevcntr14_el0, 144 |   FULL |        PMU | Arch_aarch64, "aarch64");
  DEF_REGISTER(     spmevcntr15_el0, 145 |   FULL |        PMU | Arch_aarch64, "aarch64");
  DEF_REGISTER(    spmevfilt2r0_el0, 146 |   FULL |        PMU | Arch_aarch64, "aarch64");
  DEF_REGISTER(    spmevfilt2r1_el0, 147 |   FULL |        PMU | Arch_aarch64, "aarch64");
  DEF_REGISTER(    spmevfilt2r2_el0, 148 |   FULL |        PMU | Arch_aarch64, "aarch64");
  DEF_REGISTER(    spmevfilt2r3_el0, 149 |   FULL |        PMU | Arch_aarch64, "aarch64");
  DEF_REGISTER(    spmevfilt2r4_el0, 150 |   FULL |        PMU | Arch_aarch64, "aarch64");
  DEF_REGISTER(    spmevfilt2r5_el0, 151 |   FULL |        PMU | Arch_aarch64, "aarch64");
  DEF_REGISTER(    spmevfilt2r6_el0, 152 |   FULL |        PMU | Arch_aarch64, "aarch64");
  DEF_REGISTER(    spmevfilt2r7_el0, 153 |   FULL |        PMU | Arch_aarch64, "aarch64");
  DEF_REGISTER(    spmevfilt2r8_el0, 154 |   FULL |        PMU | Arch_aarch64, "aarch64");
  DEF_REGISTER(    spmevfilt2r9_el0, 155 |   FULL |        PMU | Arch_aarch64, "aarch64");
  DEF_REGISTER(   spmevfilt2r10_el0, 156 |   FULL |        PMU | Arch_aarch64, "aarch64");
  DEF_REGISTER(   spmevfilt2r11_el0, 157 |   FULL |        PMU | Arch_aarch64, "aarch64");
  DEF_REGISTER(   spmevfilt2r12_el0, 158 |   FULL |        PMU | Arch_aarch64, "aarch64");
  DEF_REGISTER(   spmevfilt2r13_el0, 159 |   FULL |        PMU | Arch_aarch64, "aarch64");
  DEF_REGISTER(   spmevfilt2r14_el0, 160 |   FULL |        PMU | Arch_aarch64, "aarch64");
  DEF_REGISTER(   spmevfilt2r15_el0, 161 |   FULL |        PMU | Arch_aarch64, "aarch64");
  DEF_REGISTER(     spmevfiltr0_el0, 162 |   FULL |        PMU | Arch_aarch64, "aarch64");
  DEF_REGISTER(     spmevfiltr1_el0, 163 |   FULL |        PMU | Arch_aarch64, "aarch64");
  DEF_REGISTER(     spmevfiltr2_el0, 164 |   FULL |        PMU | Arch_aarch64, "aarch64");
  DEF_REGISTER(     spmevfiltr3_el0, 165 |   FULL |        PMU | Arch_aarch64, "aarch64");
  DEF_REGISTER(     spmevfiltr4_el0, 166 |   FULL |        PMU | Arch_aarch64, "aarch64");
  DEF_REGISTER(     spmevfiltr5_el0, 167 |   FULL |        PMU | Arch_aarch64, "aarch64");
  DEF_REGISTER(     spmevfiltr6_el0, 168 |   FULL |        PMU | Arch_aarch64, "aarch64");
  DEF_REGISTER(     spmevfiltr7_el0, 169 |   FULL |        PMU | Arch_aarch64, "aarch64");
  DEF_REGISTER(     spmevfiltr8_el0, 170 |   FULL |        PMU | Arch_aarch64, "aarch64");
  DEF_REGISTER(     spmevfiltr9_el0, 171 |   FULL |        PMU | Arch_aarch64, "aarch64");
  DEF_REGISTER(    spmevfiltr10_el0, 172 |   FULL |        PMU | Arch_aarch64, "aarch64");
  DEF_REGISTER(    spmevfiltr11_el0, 173 |   FULL |        PMU | Arch_aarch64, "aarch64");
  DEF_REGISTER(    spmevfiltr12_el0, 174 |   FULL |        PMU | Arch_aarch64, "aarch64");
  DEF_REGISTER(    spmevfiltr13_el0, 175 |   FULL |        PMU | Arch_aarch64, "aarch64");
  DEF_REGISTER(    spmevfiltr14_el0, 176 |   FULL |        PMU | Arch_aarch64, "aarch64");
  DEF_REGISTER(    spmevfiltr15_el0, 177 |   FULL |        PMU | Arch_aarch64, "aarch64");
  DEF_REGISTER(     spmevtyper0_el0, 178 |   FULL |        PMU | Arch_aarch64, "aarch64");
  DEF_REGISTER(     spmevtyper1_el0, 179 |   FULL |        PMU | Arch_aarch64, "aarch64");
  DEF_REGISTER(     spmevtyper2_el0, 180 |   FULL |        PMU | Arch_aarch64, "aarch64");
  DEF_REGISTER(     spmevtyper3_el0, 181 |   FULL |        PMU | Arch_aarch64, "aarch64");
  DEF_REGISTER(     spmevtyper4_el0, 182 |   FULL |        PMU | Arch_aarch64, "aarch64");
  DEF_REGISTER(     spmevtyper5_el0, 183 |   FULL |        PMU | Arch_aarch64, "aarch64");
  DEF_REGISTER(     spmevtyper6_el0, 184 |   FULL |        PMU | Arch_aarch64, "aarch64");
  DEF_REGISTER(     spmevtyper7_el0, 185 |   FULL |        PMU | Arch_aarch64, "aarch64");
  DEF_REGISTER(     spmevtyper8_el0, 186 |   FULL |        PMU | Arch_aarch64, "aarch64");
  DEF_REGISTER(     spmevtyper9_el0, 187 |   FULL |        PMU | Arch_aarch64, "aarch64");
  DEF_REGISTER(    spmevtyper10_el0, 188 |   FULL |        PMU | Arch_aarch64, "aarch64");
  DEF_REGISTER(    spmevtyper11_el0, 189 |   FULL |        PMU | Arch_aarch64, "aarch64");
  DEF_REGISTER(    spmevtyper12_el0, 190 |   FULL |        PMU | Arch_aarch64, "aarch64");
  DEF_REGISTER(    spmevtyper13_el0, 191 |   FULL |        PMU | Arch_aarch64, "aarch64");
  DEF_REGISTER(    spmevtyper14_el0, 192 |   FULL |        PMU | Arch_aarch64, "aarch64");
  DEF_REGISTER(    spmevtyper15_el0, 193 |   FULL |        PMU | Arch_aarch64, "aarch64");
  DEF_REGISTER(         spmiidr_el1, 194 |   FULL |        PMU | Arch_aarch64, "aarch64");
  DEF_REGISTER(     spmintenclr_el1, 195 |   FULL |        PMU | Arch_aarch64, "aarch64");
  DEF_REGISTER(     spmintenset_el1, 196 |   FULL |        PMU | Arch_aarch64, "aarch64");
  DEF_REGISTER(       spmovsclr_el0, 197 |   FULL |        PMU | Arch_aarch64, "aarch64");
  DEF_REGISTER(       spmovsset_el0, 198 |   FULL |        PMU | Arch_aarch64, "aarch64");
  DEF_REGISTER(       spmrootcr_el3, 199 |   FULL |        PMU | Arch_aarch64, "aarch64");
  DEF_REGISTER(          spmscr_el1, 200 |   FULL |        PMU | Arch_aarch64, "aarch64");
  DEF_REGISTER(         spmselr_el0, 201 |   FULL |        PMU | Arch_aarch64, "aarch64");
  DEF_REGISTER(           spmzr_el0, 202 |   FULL |        PMU | Arch_aarch64, "aarch64");
  DEF_REGISTER(               spsel,   6 |   FULL |     PSTATE | Arch_aarch64, "aarch64");
  DEF_REGISTER(                ssbs,   7 |   FULL |     PSTATE | Arch_aarch64, "aarch64");
  DEF_REGISTER(                svcr,   8 |   FULL |     PSTATE | Arch_aarch64, "aarch64");
  DEF_REGISTER(                 tco,   9 |   FULL |     PSTATE | Arch_aarch64, "aarch64");
  DEF_REGISTER(         teecr32_el1,  22 |  D_REG |   SYSTIMER | Arch_aarch64, "aarch64");
  DEF_REGISTER(        teehbr32_el1,  23 |  D_REG |   SYSTIMER | Arch_aarch64, "aarch64");
  DEF_REGISTER(            tfsr_el1,  15 |   FULL |     SYSCTL | Arch_aarch64, "aarch64");
  DEF_REGISTER(            tfsr_el2,  16 |   FULL |     SYSCTL | Arch_aarch64, "aarch64");
  DEF_REGISTER(            tfsr_el3,  17 |   FULL |     SYSCTL | Arch_aarch64, "aarch64");
  DEF_REGISTER(          tfsre0_el1,  18 |   FULL |     SYSCTL | Arch_aarch64, "aarch64");
  DEF_REGISTER(          tpidr2_el0,   4 |   FULL |       THRD | Arch_aarch64, "aarch64");
  DEF_REGISTER(           tpidr_el0,   5 |   FULL |       THRD | Arch_aarch64, "aarch64");
  DEF_REGISTER(           tpidr_el1,   6 |   FULL |       THRD | Arch_aarch64, "aarch64");
  DEF_REGISTER(           tpidr_el2,  50 |   FULL |       VIRT | Arch_aarch64, "aarch64");
  DEF_REGISTER(           tpidr_el3,   7 |   FULL |       THRD | Arch_aarch64, "aarch64");
  DEF_REGISTER(         tpidrro_el0,   8 |   FULL |       THRD | Arch_aarch64, "aarch64");
  DEF_REGISTER(        trbbaser_el1,   0 |   FULL |      TRACE | Arch_aarch64, "aarch64");
  DEF_REGISTER(          trbidr_el1,   1 |   FULL |      TRACE | Arch_aarch64, "aarch64");
  DEF_REGISTER(       trblimitr_el1,   2 |   FULL |      TRACE | Arch_aarch64, "aarch64");
  DEF_REGISTER(          trbmar_el1,   3 |   FULL |      TRACE | Arch_aarch64, "aarch64");
  DEF_REGISTER(          trbptr_el1,   4 |   FULL |      TRACE | Arch_aarch64, "aarch64");
  DEF_REGISTER(           trbsr_el1,   5 |   FULL |      TRACE | Arch_aarch64, "aarch64");
  DEF_REGISTER(          trbtrg_el1,   6 |   FULL |      TRACE | Arch_aarch64, "aarch64");
  DEF_REGISTER(           trcacatr0,   7 |   FULL |      TRACE | Arch_aarch64, "aarch64");
  DEF_REGISTER(           trcacatr1,   8 |   FULL |      TRACE | Arch_aarch64, "aarch64");
  DEF_REGISTER(           trcacatr2,   9 |   FULL |      TRACE | Arch_aarch64, "aarch64");
  DEF_REGISTER(           trcacatr3,  10 |   FULL |      TRACE | Arch_aarch64, "aarch64");
  DEF_REGISTER(           trcacatr4,  11 |   FULL |      TRACE | Arch_aarch64, "aarch64");
  DEF_REGISTER(           trcacatr5,  12 |   FULL |      TRACE | Arch_aarch64, "aarch64");
  DEF_REGISTER(           trcacatr6,  13 |   FULL |      TRACE | Arch_aarch64, "aarch64");
  DEF_REGISTER(           trcacatr7,  14 |   FULL |      TRACE | Arch_aarch64, "aarch64");
  DEF_REGISTER(           trcacatr8,  15 |   FULL |      TRACE | Arch_aarch64, "aarch64");
  DEF_REGISTER(           trcacatr9,  16 |   FULL |      TRACE | Arch_aarch64, "aarch64");
  DEF_REGISTER(          trcacatr10,  17 |   FULL |      TRACE | Arch_aarch64, "aarch64");
  DEF_REGISTER(          trcacatr11,  18 |   FULL |      TRACE | Arch_aarch64, "aarch64");
  DEF_REGISTER(          trcacatr12,  19 |   FULL |      TRACE | Arch_aarch64, "aarch64");
  DEF_REGISTER(          trcacatr13,  20 |   FULL |      TRACE | Arch_aarch64, "aarch64");
  DEF_REGISTER(          trcacatr14,  21 |   FULL |      TRACE | Arch_aarch64, "aarch64");
  DEF_REGISTER(          trcacatr15,  22 |   FULL |      TRACE | Arch_aarch64, "aarch64");
  DEF_REGISTER(            trcacvr0,  23 |   FULL |      TRACE | Arch_aarch64, "aarch64");
  DEF_REGISTER(            trcacvr1,  24 |   FULL |      TRACE | Arch_aarch64, "aarch64");
  DEF_REGISTER(            trcacvr2,  25 |   FULL |      TRACE | Arch_aarch64, "aarch64");
  DEF_REGISTER(            trcacvr3,  26 |   FULL |      TRACE | Arch_aarch64, "aarch64");
  DEF_REGISTER(            trcacvr4,  27 |   FULL |      TRACE | Arch_aarch64, "aarch64");
  DEF_REGISTER(            trcacvr5,  28 |   FULL |      TRACE | Arch_aarch64, "aarch64");
  DEF_REGISTER(            trcacvr6,  29 |   FULL |      TRACE | Arch_aarch64, "aarch64");
  DEF_REGISTER(            trcacvr7,  30 |   FULL |      TRACE | Arch_aarch64, "aarch64");
  DEF_REGISTER(            trcacvr8,  31 |   FULL |      TRACE | Arch_aarch64, "aarch64");
  DEF_REGISTER(            trcacvr9,  32 |   FULL |      TRACE | Arch_aarch64, "aarch64");
  DEF_REGISTER(           trcacvr10,  33 |   FULL |      TRACE | Arch_aarch64, "aarch64");
  DEF_REGISTER(           trcacvr11,  34 |   FULL |      TRACE | Arch_aarch64, "aarch64");
  DEF_REGISTER(           trcacvr12,  35 |   FULL |      TRACE | Arch_aarch64, "aarch64");
  DEF_REGISTER(           trcacvr13,  36 |   FULL |      TRACE | Arch_aarch64, "aarch64");
  DEF_REGISTER(           trcacvr14,  37 |   FULL |      TRACE | Arch_aarch64, "aarch64");
  DEF_REGISTER(           trcacvr15,  38 |   FULL |      TRACE | Arch_aarch64, "aarch64");
  DEF_REGISTER(       trcauthstatus,  39 |  D_REG |      TRACE | Arch_aarch64, "aarch64");
  DEF_REGISTER(          trcauxctlr,  40 |  D_REG |      TRACE | Arch_aarch64, "aarch64");
  DEF_REGISTER(           trcbbctlr,  41 |  D_REG |      TRACE | Arch_aarch64, "aarch64");
  DEF_REGISTER(           trcccctlr,  42 |  D_REG |      TRACE | Arch_aarch64, "aarch64");
  DEF_REGISTER(        trccidcctlr0,  43 |  D_REG |      TRACE | Arch_aarch64, "aarch64");
  DEF_REGISTER(        trccidcctlr1,  44 |  D_REG |      TRACE | Arch_aarch64, "aarch64");
  DEF_REGISTER(          trccidcvr0,  45 |   FULL |      TRACE | Arch_aarch64, "aarch64");
  DEF_REGISTER(          trccidcvr1,  46 |   FULL |      TRACE | Arch_aarch64, "aarch64");
  DEF_REGISTER(          trccidcvr2,  47 |   FULL |      TRACE | Arch_aarch64, "aarch64");
  DEF_REGISTER(          trccidcvr3,  48 |   FULL |      TRACE | Arch_aarch64, "aarch64");
  DEF_REGISTER(          trccidcvr4,  49 |   FULL |      TRACE | Arch_aarch64, "aarch64");
  DEF_REGISTER(          trccidcvr5,  50 |   FULL |      TRACE | Arch_aarch64, "aarch64");
  DEF_REGISTER(          trccidcvr6,  51 |   FULL |      TRACE | Arch_aarch64, "aarch64");
  DEF_REGISTER(          trccidcvr7,  52 |   FULL |      TRACE | Arch_aarch64, "aarch64");
  DEF_REGISTER(            trccidr0,  53 |  D_REG |      TRACE | Arch_aarch64, "aarch64");
  DEF_REGISTER(            trccidr1,  54 |  D_REG |      TRACE | Arch_aarch64, "aarch64");
  DEF_REGISTER(            trccidr2,  55 |  D_REG |      TRACE | Arch_aarch64, "aarch64");
  DEF_REGISTER(            trccidr3,  56 |  D_REG |      TRACE | Arch_aarch64, "aarch64");
  DEF_REGISTER(         trcclaimclr,  57 |  D_REG |      TRACE | Arch_aarch64, "aarch64");
  DEF_REGISTER(         trcclaimset,  58 |  D_REG |      TRACE | Arch_aarch64, "aarch64");
  DEF_REGISTER(         trccntctlr0,  59 |  D_REG |      TRACE | Arch_aarch64, "aarch64");
  DEF_REGISTER(         trccntctlr1,  60 |  D_REG |      TRACE | Arch_aarch64, "aarch64");
  DEF_REGISTER(         trccntctlr2,  61 |  D_REG |      TRACE | Arch_aarch64, "aarch64");
  DEF_REGISTER(         trccntctlr3,  62 |  D_REG |      TRACE | Arch_aarch64, "aarch64");
  DEF_REGISTER(        trccntrldvr0,  63 |   FULL |      TRACE | Arch_aarch64, "aarch64");
  DEF_REGISTER(        trccntrldvr1,  64 |   FULL |      TRACE | Arch_aarch64, "aarch64");
  DEF_REGISTER(        trccntrldvr2,  65 |   FULL |      TRACE | Arch_aarch64, "aarch64");
  DEF_REGISTER(        trccntrldvr3,  66 |   FULL |      TRACE | Arch_aarch64, "aarch64");
  DEF_REGISTER(           trccntvr0,  67 |  D_REG |      TRACE | Arch_aarch64, "aarch64");
  DEF_REGISTER(           trccntvr1,  68 |  D_REG |      TRACE | Arch_aarch64, "aarch64");
  DEF_REGISTER(           trccntvr2,  69 |  D_REG |      TRACE | Arch_aarch64, "aarch64");
  DEF_REGISTER(           trccntvr3,  70 |  D_REG |      TRACE | Arch_aarch64, "aarch64");
  DEF_REGISTER(          trcconfigr,  71 |  D_REG |      TRACE | Arch_aarch64, "aarch64");
  DEF_REGISTER(          trcdevaff0,  72 |   FULL |      TRACE | Arch_aarch64, "aarch64");
  DEF_REGISTER(          trcdevaff1,  73 |   FULL |      TRACE | Arch_aarch64, "aarch64");
  DEF_REGISTER(          trcdevarch,  74 |  D_REG |      TRACE | Arch_aarch64, "aarch64");
  DEF_REGISTER(            trcdevid,  75 |  D_REG |      TRACE | Arch_aarch64, "aarch64");
  DEF_REGISTER(          trcdevtype,  76 |  D_REG |      TRACE | Arch_aarch64, "aarch64");
  DEF_REGISTER(           trcdvcmr0,  77 |   FULL |      TRACE | Arch_aarch64, "aarch64");
  DEF_REGISTER(           trcdvcmr1,  78 |   FULL |      TRACE | Arch_aarch64, "aarch64");
  DEF_REGISTER(           trcdvcmr2,  79 |   FULL |      TRACE | Arch_aarch64, "aarch64");
  DEF_REGISTER(           trcdvcmr3,  80 |   FULL |      TRACE | Arch_aarch64, "aarch64");
  DEF_REGISTER(           trcdvcmr4,  81 |   FULL |      TRACE | Arch_aarch64, "aarch64");
  DEF_REGISTER(           trcdvcmr5,  82 |   FULL |      TRACE | Arch_aarch64, "aarch64");
  DEF_REGISTER(           trcdvcmr6,  83 |   FULL |      TRACE | Arch_aarch64, "aarch64");
  DEF_REGISTER(           trcdvcmr7,  84 |   FULL |      TRACE | Arch_aarch64, "aarch64");
  DEF_REGISTER(           trcdvcvr0,  85 |   FULL |      TRACE | Arch_aarch64, "aarch64");
  DEF_REGISTER(           trcdvcvr1,  86 |   FULL |      TRACE | Arch_aarch64, "aarch64");
  DEF_REGISTER(           trcdvcvr2,  87 |   FULL |      TRACE | Arch_aarch64, "aarch64");
  DEF_REGISTER(           trcdvcvr3,  88 |   FULL |      TRACE | Arch_aarch64, "aarch64");
  DEF_REGISTER(           trcdvcvr4,  89 |   FULL |      TRACE | Arch_aarch64, "aarch64");
  DEF_REGISTER(           trcdvcvr5,  90 |   FULL |      TRACE | Arch_aarch64, "aarch64");
  DEF_REGISTER(           trcdvcvr6,  91 |   FULL |      TRACE | Arch_aarch64, "aarch64");
  DEF_REGISTER(           trcdvcvr7,  92 |   FULL |      TRACE | Arch_aarch64, "aarch64");
  DEF_REGISTER(       trceventctl0r,  93 |  D_REG |      TRACE | Arch_aarch64, "aarch64");
  DEF_REGISTER(       trceventctl1r,  94 |  D_REG |      TRACE | Arch_aarch64, "aarch64");
  DEF_REGISTER(        trcextinselr,  95 |   FULL |      TRACE | Arch_aarch64, "aarch64");
  DEF_REGISTER(       trcextinselr0,  96 |   FULL |      TRACE | Arch_aarch64, "aarch64");
  DEF_REGISTER(       trcextinselr1,  97 |   FULL |      TRACE | Arch_aarch64, "aarch64");
  DEF_REGISTER(       trcextinselr2,  98 |   FULL |      TRACE | Arch_aarch64, "aarch64");
  DEF_REGISTER(       trcextinselr3,  99 |   FULL |      TRACE | Arch_aarch64, "aarch64");
  DEF_REGISTER(             trcidr0, 100 |  D_REG |      TRACE | Arch_aarch64, "aarch64");
  DEF_REGISTER(             trcidr1, 101 |   FULL |      TRACE | Arch_aarch64, "aarch64");
  DEF_REGISTER(             trcidr2, 102 |   FULL |      TRACE | Arch_aarch64, "aarch64");
  DEF_REGISTER(             trcidr3, 103 |   FULL |      TRACE | Arch_aarch64, "aarch64");
  DEF_REGISTER(             trcidr4, 104 |   FULL |      TRACE | Arch_aarch64, "aarch64");
  DEF_REGISTER(             trcidr5, 105 |   FULL |      TRACE | Arch_aarch64, "aarch64");
  DEF_REGISTER(             trcidr6, 106 |  D_REG |      TRACE | Arch_aarch64, "aarch64");
  DEF_REGISTER(             trcidr7, 107 |   FULL |      TRACE | Arch_aarch64, "aarch64");
  DEF_REGISTER(             trcidr8, 108 |  D_REG |      TRACE | Arch_aarch64, "aarch64");
  DEF_REGISTER(             trcidr9, 109 |   FULL |      TRACE | Arch_aarch64, "aarch64");
  DEF_REGISTER(            trcidr10, 110 |   FULL |      TRACE | Arch_aarch64, "aarch64");
  DEF_REGISTER(            trcidr11, 111 |   FULL |      TRACE | Arch_aarch64, "aarch64");
  DEF_REGISTER(            trcidr12, 112 |  D_REG |      TRACE | Arch_aarch64, "aarch64");
  DEF_REGISTER(            trcidr13, 113 |   FULL |      TRACE | Arch_aarch64, "aarch64");
  DEF_REGISTER(          trcimspec0, 114 |   FULL |      TRACE | Arch_aarch64, "aarch64");
  DEF_REGISTER(          trcimspec1, 115 |  D_REG |      TRACE | Arch_aarch64, "aarch64");
  DEF_REGISTER(          trcimspec2, 116 |  D_REG |      TRACE | Arch_aarch64, "aarch64");
  DEF_REGISTER(          trcimspec3, 117 |  D_REG |      TRACE | Arch_aarch64, "aarch64");
  DEF_REGISTER(          trcimspec4, 118 |  D_REG |      TRACE | Arch_aarch64, "aarch64");
  DEF_REGISTER(          trcimspec5, 119 |  D_REG |      TRACE | Arch_aarch64, "aarch64");
  DEF_REGISTER(          trcimspec6, 120 |  D_REG |      TRACE | Arch_aarch64, "aarch64");
  DEF_REGISTER(          trcimspec7, 121 |  D_REG |      TRACE | Arch_aarch64, "aarch64");
  DEF_REGISTER(           trcitctrl, 122 |  D_REG |      TRACE | Arch_aarch64, "aarch64");
  DEF_REGISTER(        trcitecr_el1, 123 |   FULL |      TRACE | Arch_aarch64, "aarch64");
  DEF_REGISTER(        trcitecr_el2, 124 |   FULL |      TRACE | Arch_aarch64, "aarch64");
  DEF_REGISTER(          trciteedcr, 125 |  D_REG |      TRACE | Arch_aarch64, "aarch64");
  DEF_REGISTER(              trclar, 126 |  D_REG |      TRACE | Arch_aarch64, "aarch64");
  DEF_REGISTER(              trclsr, 127 |  D_REG |      TRACE | Arch_aarch64, "aarch64");
  DEF_REGISTER(            trcoslar, 128 |   FULL |      TRACE | Arch_aarch64, "aarch64");
  DEF_REGISTER(            trcoslsr, 129 |   FULL |      TRACE | Arch_aarch64, "aarch64");
  DEF_REGISTER(             trcpdcr, 130 |  D_REG |      TRACE | Arch_aarch64, "aarch64");
  DEF_REGISTER(             trcpdsr, 131 |  D_REG |      TRACE | Arch_aarch64, "aarch64");
  DEF_REGISTER(            trcpidr0, 132 |  D_REG |      TRACE | Arch_aarch64, "aarch64");
  DEF_REGISTER(            trcpidr1, 133 |  D_REG |      TRACE | Arch_aarch64, "aarch64");
  DEF_REGISTER(            trcpidr2, 134 |  D_REG |      TRACE | Arch_aarch64, "aarch64");
  DEF_REGISTER(            trcpidr3, 135 |  D_REG |      TRACE | Arch_aarch64, "aarch64");
  DEF_REGISTER(            trcpidr4, 136 |  D_REG |      TRACE | Arch_aarch64, "aarch64");
  DEF_REGISTER(            trcpidr5, 137 |  D_REG |      TRACE | Arch_aarch64, "aarch64");
  DEF_REGISTER(            trcpidr6, 138 |  D_REG |      TRACE | Arch_aarch64, "aarch64");
  DEF_REGISTER(            trcpidr7, 139 |  D_REG |      TRACE | Arch_aarch64, "aarch64");
  DEF_REGISTER(          trcprgctlr, 140 |   FULL |      TRACE | Arch_aarch64, "aarch64");
  DEF_REGISTER(         trcprocselr, 141 |   FULL |      TRACE | Arch_aarch64, "aarch64");
  DEF_REGISTER(            trcqctlr, 142 |  D_REG |      TRACE | Arch_aarch64, "aarch64");
  DEF_REGISTER(          trcrsctlr2, 143 |   FULL |      TRACE | Arch_aarch64, "aarch64");
  DEF_REGISTER(          trcrsctlr3, 144 |   FULL |      TRACE | Arch_aarch64, "aarch64");
  DEF_REGISTER(          trcrsctlr4, 145 |   FULL |      TRACE | Arch_aarch64, "aarch64");
  DEF_REGISTER(          trcrsctlr5, 146 |   FULL |      TRACE | Arch_aarch64, "aarch64");
  DEF_REGISTER(          trcrsctlr6, 147 |   FULL |      TRACE | Arch_aarch64, "aarch64");
  DEF_REGISTER(          trcrsctlr7, 148 |   FULL |      TRACE | Arch_aarch64, "aarch64");
  DEF_REGISTER(          trcrsctlr8, 149 |   FULL |      TRACE | Arch_aarch64, "aarch64");
  DEF_REGISTER(          trcrsctlr9, 150 |   FULL |      TRACE | Arch_aarch64, "aarch64");
  DEF_REGISTER(         trcrsctlr10, 151 |   FULL |      TRACE | Arch_aarch64, "aarch64");
  DEF_REGISTER(         trcrsctlr11, 152 |   FULL |      TRACE | Arch_aarch64, "aarch64");
  DEF_REGISTER(         trcrsctlr12, 153 |   FULL |      TRACE | Arch_aarch64, "aarch64");
  DEF_REGISTER(         trcrsctlr13, 154 |   FULL |      TRACE | Arch_aarch64, "aarch64");
  DEF_REGISTER(         trcrsctlr14, 155 |   FULL |      TRACE | Arch_aarch64, "aarch64");
  DEF_REGISTER(         trcrsctlr15, 156 |   FULL |      TRACE | Arch_aarch64, "aarch64");
  DEF_REGISTER(         trcrsctlr16, 157 |   FULL |      TRACE | Arch_aarch64, "aarch64");
  DEF_REGISTER(         trcrsctlr17, 158 |   FULL |      TRACE | Arch_aarch64, "aarch64");
  DEF_REGISTER(         trcrsctlr18, 159 |   FULL |      TRACE | Arch_aarch64, "aarch64");
  DEF_REGISTER(         trcrsctlr19, 160 |   FULL |      TRACE | Arch_aarch64, "aarch64");
  DEF_REGISTER(         trcrsctlr20, 161 |   FULL |      TRACE | Arch_aarch64, "aarch64");
  DEF_REGISTER(         trcrsctlr21, 162 |   FULL |      TRACE | Arch_aarch64, "aarch64");
  DEF_REGISTER(         trcrsctlr22, 163 |   FULL |      TRACE | Arch_aarch64, "aarch64");
  DEF_REGISTER(         trcrsctlr23, 164 |   FULL |      TRACE | Arch_aarch64, "aarch64");
  DEF_REGISTER(         trcrsctlr24, 165 |   FULL |      TRACE | Arch_aarch64, "aarch64");
  DEF_REGISTER(         trcrsctlr25, 166 |   FULL |      TRACE | Arch_aarch64, "aarch64");
  DEF_REGISTER(         trcrsctlr26, 167 |   FULL |      TRACE | Arch_aarch64, "aarch64");
  DEF_REGISTER(         trcrsctlr27, 168 |   FULL |      TRACE | Arch_aarch64, "aarch64");
  DEF_REGISTER(         trcrsctlr28, 169 |   FULL |      TRACE | Arch_aarch64, "aarch64");
  DEF_REGISTER(         trcrsctlr29, 170 |   FULL |      TRACE | Arch_aarch64, "aarch64");
  DEF_REGISTER(         trcrsctlr30, 171 |   FULL |      TRACE | Arch_aarch64, "aarch64");
  DEF_REGISTER(         trcrsctlr31, 172 |   FULL |      TRACE | Arch_aarch64, "aarch64");
  DEF_REGISTER(              trcrsr, 173 |   FULL |      TRACE | Arch_aarch64, "aarch64");
  DEF_REGISTER(          trcseqevr0, 174 |   FULL |      TRACE | Arch_aarch64, "aarch64");
  DEF_REGISTER(          trcseqevr1, 175 |   FULL |      TRACE | Arch_aarch64, "aarch64");
  DEF_REGISTER(          trcseqevr2, 176 |   FULL |      TRACE | Arch_aarch64, "aarch64");
  DEF_REGISTER(        trcseqrstevr, 177 |  D_REG |      TRACE | Arch_aarch64, "aarch64");
  DEF_REGISTER(           trcseqstr, 178 |   FULL |      TRACE | Arch_aarch64, "aarch64");
  DEF_REGISTER(           trcssccr0, 179 |  D_REG |      TRACE | Arch_aarch64, "aarch64");
  DEF_REGISTER(           trcssccr1, 180 |  D_REG |      TRACE | Arch_aarch64, "aarch64");
  DEF_REGISTER(           trcssccr2, 181 |  D_REG |      TRACE | Arch_aarch64, "aarch64");
  DEF_REGISTER(           trcssccr3, 182 |  D_REG |      TRACE | Arch_aarch64, "aarch64");
  DEF_REGISTER(           trcssccr4, 183 |  D_REG |      TRACE | Arch_aarch64, "aarch64");
  DEF_REGISTER(           trcssccr5, 184 |  D_REG |      TRACE | Arch_aarch64, "aarch64");
  DEF_REGISTER(           trcssccr6, 185 |  D_REG |      TRACE | Arch_aarch64, "aarch64");
  DEF_REGISTER(           trcssccr7, 186 |  D_REG |      TRACE | Arch_aarch64, "aarch64");
  DEF_REGISTER(           trcsscsr0, 187 |  D_REG |      TRACE | Arch_aarch64, "aarch64");
  DEF_REGISTER(           trcsscsr1, 188 |  D_REG |      TRACE | Arch_aarch64, "aarch64");
  DEF_REGISTER(           trcsscsr2, 189 |  D_REG |      TRACE | Arch_aarch64, "aarch64");
  DEF_REGISTER(           trcsscsr3, 190 |  D_REG |      TRACE | Arch_aarch64, "aarch64");
  DEF_REGISTER(           trcsscsr4, 191 |  D_REG |      TRACE | Arch_aarch64, "aarch64");
  DEF_REGISTER(           trcsscsr5, 192 |  D_REG |      TRACE | Arch_aarch64, "aarch64");
  DEF_REGISTER(           trcsscsr6, 193 |  D_REG |      TRACE | Arch_aarch64, "aarch64");
  DEF_REGISTER(           trcsscsr7, 194 |  D_REG |      TRACE | Arch_aarch64, "aarch64");
  DEF_REGISTER(         trcsspcicr0, 195 |  D_REG |      TRACE | Arch_aarch64, "aarch64");
  DEF_REGISTER(         trcsspcicr1, 196 |  D_REG |      TRACE | Arch_aarch64, "aarch64");
  DEF_REGISTER(         trcsspcicr2, 197 |  D_REG |      TRACE | Arch_aarch64, "aarch64");
  DEF_REGISTER(         trcsspcicr3, 198 |  D_REG |      TRACE | Arch_aarch64, "aarch64");
  DEF_REGISTER(         trcsspcicr4, 199 |  D_REG |      TRACE | Arch_aarch64, "aarch64");
  DEF_REGISTER(         trcsspcicr5, 200 |  D_REG |      TRACE | Arch_aarch64, "aarch64");
  DEF_REGISTER(         trcsspcicr6, 201 |  D_REG |      TRACE | Arch_aarch64, "aarch64");
  DEF_REGISTER(         trcsspcicr7, 202 |  D_REG |      TRACE | Arch_aarch64, "aarch64");
  DEF_REGISTER(        trcstallctlr, 203 |   FULL |      TRACE | Arch_aarch64, "aarch64");
  DEF_REGISTER(            trcstatr, 204 |  D_REG |      TRACE | Arch_aarch64, "aarch64");
  DEF_REGISTER(           trcsyncpr, 205 |   FULL |      TRACE | Arch_aarch64, "aarch64");
  DEF_REGISTER(         trctraceidr, 206 |  D_REG |      TRACE | Arch_aarch64, "aarch64");
  DEF_REGISTER(           trctsctlr, 207 |   FULL |      TRACE | Arch_aarch64, "aarch64");
  DEF_REGISTER(        trcvdarcctlr, 208 |   FULL |      TRACE | Arch_aarch64, "aarch64");
  DEF_REGISTER(           trcvdctlr, 209 |   FULL |      TRACE | Arch_aarch64, "aarch64");
  DEF_REGISTER(        trcvdsacctlr, 210 |   FULL |      TRACE | Arch_aarch64, "aarch64");
  DEF_REGISTER(           trcvictlr, 211 |   FULL |      TRACE | Arch_aarch64, "aarch64");
  DEF_REGISTER(         trcviiectlr, 212 |  D_REG |      TRACE | Arch_aarch64, "aarch64");
  DEF_REGISTER(       trcvipcssctlr, 213 |   FULL |      TRACE | Arch_aarch64, "aarch64");
  DEF_REGISTER(         trcvissctlr, 214 |  D_REG |      TRACE | Arch_aarch64, "aarch64");
  DEF_REGISTER(       trcvmidcctlr0, 215 |   FULL |      TRACE | Arch_aarch64, "aarch64");
  DEF_REGISTER(       trcvmidcctlr1, 216 |   FULL |      TRACE | Arch_aarch64, "aarch64");
  DEF_REGISTER(         trcvmidcvr0, 217 |   FULL |      TRACE | Arch_aarch64, "aarch64");
  DEF_REGISTER(         trcvmidcvr1, 218 |   FULL |      TRACE | Arch_aarch64, "aarch64");
  DEF_REGISTER(         trcvmidcvr2, 219 |   FULL |      TRACE | Arch_aarch64, "aarch64");
  DEF_REGISTER(         trcvmidcvr3, 220 |   FULL |      TRACE | Arch_aarch64, "aarch64");
  DEF_REGISTER(         trcvmidcvr4, 221 |   FULL |      TRACE | Arch_aarch64, "aarch64");
  DEF_REGISTER(         trcvmidcvr5, 222 |   FULL |      TRACE | Arch_aarch64, "aarch64");
  DEF_REGISTER(         trcvmidcvr6, 223 |   FULL |      TRACE | Arch_aarch64, "aarch64");
  DEF_REGISTER(         trcvmidcvr7, 224 |   FULL |      TRACE | Arch_aarch64, "aarch64");
  DEF_REGISTER(           trfcr_el1, 225 |   FULL |      TRACE | Arch_aarch64, "aarch64");
  DEF_REGISTER(           trfcr_el2, 226 |   FULL |      TRACE | Arch_aarch64, "aarch64");
  DEF_REGISTER(                 uao,  10 |   FULL |     PSTATE | Arch_aarch64, "aarch64");
  DEF_REGISTER(            vbar_el1,   6 |   FULL |       EXCP | Arch_aarch64, "aarch64");
  DEF_REGISTER(            vbar_el2,  51 |   FULL |       VIRT | Arch_aarch64, "aarch64");
  DEF_REGISTER(            vbar_el3,   7 |   FULL | EXCP | SEC | Arch_aarch64, "aarch64");
  DEF_REGISTER(           vdisr_el2,  16 |   FULL |        RAS | Arch_aarch64, "aarch64");
  DEF_REGISTER(           vdisr_el3,  17 |   FULL |        RAS | Arch_aarch64, "aarch64");
  DEF_REGISTER(        vmecid_a_el2,  19 |   FULL |     SYSCTL | Arch_aarch64, "aarch64");
  DEF_REGISTER(        vmecid_p_el2,  20 |   FULL |     SYSCTL | Arch_aarch64, "aarch64");
  DEF_REGISTER(          vmpidr_el2,  52 |   FULL |       VIRT | Arch_aarch64, "aarch64");
  DEF_REGISTER(            vncr_el2,  21 |   FULL |     SYSCTL | Arch_aarch64, "aarch64");
  DEF_REGISTER(           vpidr_el2,  53 |   FULL |       VIRT | Arch_aarch64, "aarch64");
  DEF_REGISTER(           vsesr_el2,  18 |   FULL |        RAS | Arch_aarch64, "aarch64");
  DEF_REGISTER(           vsesr_el3,  19 |   FULL |        RAS | Arch_aarch64, "aarch64");
  DEF_REGISTER(           vstcr_el2,  22 |   FULL |     SYSCTL | Arch_aarch64, "aarch64");
  DEF_REGISTER(          vsttbr_el2,  23 |   FULL |     SYSCTL | Arch_aarch64, "aarch64");
  DEF_REGISTER(             zcr_el1,   6 |   FULL |      OTHER | Arch_aarch64, "aarch64");
  DEF_REGISTER(             zcr_el2,   7 |   FULL |      OTHER | Arch_aarch64, "aarch64");
  DEF_REGISTER(             zcr_el3,   8 |   FULL |      OTHER | Arch_aarch64, "aarch64");

  /*
   *  Register aliases
   *
   *  x31 can alias either x{w}zr or x{w}sp depending on context.
   *  This makes it impossible to give it a fixed alias.
   *
   *  The intra-procedural registers are capitalized to prevent collision with the
   *  implicit names of the variables associated with the p{0,1} SVE predicate
   *  registers (see the definition of DEF_REGISTER for details).
   */
  DEF_REGISTER_ALIAS( fp, x29, "aarch64");
  DEF_REGISTER_ALIAS( lr, x30, "aarch64");
  DEF_REGISTER_ALIAS(Ip0, x16, "aarch64");  // Intra-procedure-call scratch registers
  DEF_REGISTER_ALIAS(Ip1, x17, "aarch64");


/************************************************************************************
 *
 *          Pseudo-registers
 *
 *        for internal use only
 *
 ************************************************************************************
 */

  DEF_REGISTER(IMPLEMENTATION_DEFINED_SYSREG,  255 | D_REG | SYSREG | Arch_aarch64, "aarch64");

  // special registers
  const int32_t N_FLAG = 31;
  const int32_t Z_FLAG = 30;
  const int32_t C_FLAG = 29;
  const int32_t V_FLAG = 28;

  DEF_REGISTER( pstate,       2 | D_REG |    SPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(      n,  N_FLAG |   BIT |   FLAG | Arch_aarch64, "aarch64");
  DEF_REGISTER(      z,  Z_FLAG |   BIT |   FLAG | Arch_aarch64, "aarch64");
  DEF_REGISTER(      c,  C_FLAG |   BIT |   FLAG | Arch_aarch64, "aarch64");
  DEF_REGISTER(      v,  V_FLAG |   BIT |   FLAG | Arch_aarch64, "aarch64");
  
  
  // Upper 64 bits in 128-bit reg
  const int32_t HQ_REG = 0x0000FF00;
  DEF_REGISTER(  hq0,   0 | HQ_REG |    FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(  hq1,   1 | HQ_REG |    FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(  hq2,   2 | HQ_REG |    FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(  hq3,   3 | HQ_REG |    FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(  hq4,   4 | HQ_REG |    FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(  hq5,   5 | HQ_REG |    FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(  hq6,   6 | HQ_REG |    FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(  hq7,   7 | HQ_REG |    FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(  hq8,   8 | HQ_REG |    FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(  hq9,   9 | HQ_REG |    FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER( hq10,  10 | HQ_REG |    FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER( hq11,  11 | HQ_REG |    FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER( hq12,  12 | HQ_REG |    FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER( hq13,  13 | HQ_REG |    FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER( hq14,  14 | HQ_REG |    FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER( hq15,  15 | HQ_REG |    FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER( hq16,  16 | HQ_REG |    FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER( hq17,  17 | HQ_REG |    FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER( hq18,  18 | HQ_REG |    FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER( hq19,  19 | HQ_REG |    FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER( hq20,  20 | HQ_REG |    FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER( hq21,  21 | HQ_REG |    FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER( hq22,  22 | HQ_REG |    FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER( hq23,  23 | HQ_REG |    FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER( hq24,  24 | HQ_REG |    FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER( hq25,  25 | HQ_REG |    FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER( hq26,  26 | HQ_REG |    FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER( hq27,  27 | HQ_REG |    FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER( hq28,  28 | HQ_REG |    FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER( hq29,  29 | HQ_REG |    FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER( hq30,  30 | HQ_REG |    FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER( hq31,  31 | HQ_REG |    FPR | Arch_aarch64, "aarch64");


  // Translation Lookaside Buffer maintenance instructions
  // These are not physical registers
  const int32_t TLBI   = SYSREG | 0x00EF0000;
  DEF_REGISTER(      tlbi_vale3is,   0 |   FULL | TLBI | Arch_aarch64, "aarch64");
  DEF_REGISTER(     tlbi_ipas2le1,   1 |   FULL | TLBI | Arch_aarch64, "aarch64");
  DEF_REGISTER(   tlbi_ipas2le1is,   2 |   FULL | TLBI | Arch_aarch64, "aarch64");
  DEF_REGISTER(      tlbi_vale2is,   3 |   FULL | TLBI | Arch_aarch64, "aarch64");
  DEF_REGISTER(      tlbi_alle3is,   4 |   FULL | TLBI | Arch_aarch64, "aarch64");
  DEF_REGISTER(    tlbi_ipas2e1is,   5 |   FULL | TLBI | Arch_aarch64, "aarch64");
  DEF_REGISTER(         tlbi_vae2,   6 |   FULL | TLBI | Arch_aarch64, "aarch64");
  DEF_REGISTER(      tlbi_vmalle1,   7 |   FULL | TLBI | Arch_aarch64, "aarch64");
  DEF_REGISTER(        tlbi_vale2,   8 |   FULL | TLBI | Arch_aarch64, "aarch64");
  DEF_REGISTER(        tlbi_vaae1,   9 |   FULL | TLBI | Arch_aarch64, "aarch64");
  DEF_REGISTER(      tlbi_ipas2e1,  10 |   FULL | TLBI | Arch_aarch64, "aarch64");
  DEF_REGISTER(     tlbi_aside1is,  11 |   FULL | TLBI | Arch_aarch64, "aarch64");
  DEF_REGISTER(        tlbi_alle1,  12 |   FULL | TLBI | Arch_aarch64, "aarch64");
  DEF_REGISTER(       tlbi_vaale1,  13 |   FULL | TLBI | Arch_aarch64, "aarch64");
  DEF_REGISTER(       tlbi_aside1,  14 |   FULL | TLBI | Arch_aarch64, "aarch64");
  DEF_REGISTER(        tlbi_alle2,  15 |   FULL | TLBI | Arch_aarch64, "aarch64");
  DEF_REGISTER( tlbi_vmalls12e1is,  16 |   FULL | TLBI | Arch_aarch64, "aarch64");
  DEF_REGISTER(         tlbi_vae1,  17 |   FULL | TLBI | Arch_aarch64, "aarch64");
  DEF_REGISTER(      tlbi_vaae1is,  18 |   FULL | TLBI | Arch_aarch64, "aarch64");
  DEF_REGISTER(      tlbi_alle2is,  19 |   FULL | TLBI | Arch_aarch64, "aarch64");
  DEF_REGISTER(         tlbi_vae3,  20 |   FULL | TLBI | Arch_aarch64, "aarch64");
  DEF_REGISTER(   tlbi_vmalls12e1,  21 |   FULL | TLBI | Arch_aarch64, "aarch64");
  DEF_REGISTER(       tlbi_vae3is,  22 |   FULL | TLBI | Arch_aarch64, "aarch64");
  DEF_REGISTER(       tlbi_vae2is,  23 |   FULL | TLBI | Arch_aarch64, "aarch64");
  DEF_REGISTER(    tlbi_vmalle1is,  24 |   FULL | TLBI | Arch_aarch64, "aarch64");
  DEF_REGISTER(     tlbi_vaale1is,  25 |   FULL | TLBI | Arch_aarch64, "aarch64");
  DEF_REGISTER(      tlbi_alle1is,  26 |   FULL | TLBI | Arch_aarch64, "aarch64");
  DEF_REGISTER(       tlbi_vae1is,  27 |   FULL | TLBI | Arch_aarch64, "aarch64");
  DEF_REGISTER(        tlbi_vale3,  28 |   FULL | TLBI | Arch_aarch64, "aarch64");
  DEF_REGISTER(      tlbi_vale1is,  29 |   FULL | TLBI | Arch_aarch64, "aarch64");
  DEF_REGISTER(        tlbi_alle3,  30 |   FULL | TLBI | Arch_aarch64, "aarch64");
  DEF_REGISTER(        tlbi_vale1,  31 |   FULL | TLBI | Arch_aarch64, "aarch64");


  /*  Assembler mnemonics
   *
   *  These are not physical registers; they are used by disassemblers to disambiguate
   *  opcodes into separate mnemonics.
   */

  const int32_t MNEMONICS = SYSREG | 0x00DF0000;
  DEF_REGISTER(   ic_iallu,  0 | FULL | MNEMONICS | Arch_aarch64, "aarch64");  // Instruction Cache Invalidation
  DEF_REGISTER(    ic_ivau,  1 | FULL | MNEMONICS | Arch_aarch64, "aarch64");
  DEF_REGISTER( ic_ialluis,  2 | FULL | MNEMONICS | Arch_aarch64, "aarch64");
  DEF_REGISTER(   at_s1e2r,  3 | FULL | MNEMONICS | Arch_aarch64, "aarch64");  // Address Translation
  DEF_REGISTER(   at_s1e1r,  4 | FULL | MNEMONICS | Arch_aarch64, "aarch64");
  DEF_REGISTER(  at_s12e0w,  5 | FULL | MNEMONICS | Arch_aarch64, "aarch64");
  DEF_REGISTER(   at_s1e2w,  6 | FULL | MNEMONICS | Arch_aarch64, "aarch64");
  DEF_REGISTER(   at_s1e0w,  7 | FULL | MNEMONICS | Arch_aarch64, "aarch64");
  DEF_REGISTER(  at_s12e0r,  8 | FULL | MNEMONICS | Arch_aarch64, "aarch64");
  DEF_REGISTER(   at_s1e3w,  9 | FULL | MNEMONICS | Arch_aarch64, "aarch64");
  DEF_REGISTER(  at_s12e1w, 10 | FULL | MNEMONICS | Arch_aarch64, "aarch64");
  DEF_REGISTER(   at_s1e0r, 11 | FULL | MNEMONICS | Arch_aarch64, "aarch64");
  DEF_REGISTER(  at_s12e1r, 12 | FULL | MNEMONICS | Arch_aarch64, "aarch64");
  DEF_REGISTER(   at_s1e1w, 13 | FULL | MNEMONICS | Arch_aarch64, "aarch64");
  DEF_REGISTER(   at_s1e3r, 14 | FULL | MNEMONICS | Arch_aarch64, "aarch64");
  DEF_REGISTER(   dc_civac, 15 | FULL | MNEMONICS | Arch_aarch64, "aarch64");  // Data Cache Invalidation
  DEF_REGISTER(    dc_ivac, 16 | FULL | MNEMONICS | Arch_aarch64, "aarch64");
  DEF_REGISTER(     dc_csw, 17 | FULL | MNEMONICS | Arch_aarch64, "aarch64");
  DEF_REGISTER(    dc_cvau, 18 | FULL | MNEMONICS | Arch_aarch64, "aarch64");
  DEF_REGISTER(    dc_cisw, 19 | FULL | MNEMONICS | Arch_aarch64, "aarch64");
  DEF_REGISTER(     dc_zva, 20 | FULL | MNEMONICS | Arch_aarch64, "aarch64");
  DEF_REGISTER(     dc_isw, 21 | FULL | MNEMONICS | Arch_aarch64, "aarch64");
  DEF_REGISTER(    dc_cvac, 22 | FULL | MNEMONICS | Arch_aarch64, "aarch64");


}}

#endif
