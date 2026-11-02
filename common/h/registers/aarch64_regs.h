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
  const int32_t HQ_REG    = 0x0000FF00;           // upper 64 bits of a 128-bit register
  const int32_t TLBI      = SYSREG | 0x00EF0000;  // TLB maintenance instructions, not physical registers
  const int32_t MNEMONICS = SYSREG | 0x00DF0000;  // disassembler mnemonics, not physical registers
  const int32_t VIRT      = SYSREG | 0x00290000;  // virtualization


  /**
   * Format of constants:
   *  [0x000000ff] Lower 8 bits are base register ID
   *  [0x0000ff00] Next 8 bits are the aliasing and subrange ID used to distinguish
   *               between whole and aliased registers like w1 and x1.
   *  [0x00ff0000] Next 8 bits are the register category, GPR, FLAG, etc.
   *  [0xff000000] Upper 8 bits are the architecture.
   **/
#include "registers/aarch64_regs.def"


}}

#endif
