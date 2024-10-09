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
  const int32_t SYSREG = 0x00100000;


  /**
   * Format of constants:
   *  [0x000000ff] Lower 8 bits are base register ID
   *  [0x0000ff00] Next 8 bits are the aliasing and subrange ID used to distinguish
   *               between whole and aliased registers like w1 and x1.
   *  [0x00ff0000] Next 8 bits are the register category, GPR, FLAG, etc.
   *  [0xff000000] Upper 8 bits are the architecture.
   **/

  //          (                name,  ID |  alias |        cat |         arch,      arch)
  DEF_REGISTER(                  fp,   0 |   FULL |        SPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(                fpcr,   1 |  D_REG |        SPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(                  lr,   2 |   FULL |        SPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(                nzcv,   0 |    BIT |        SPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(                  pc,   3 |   FULL |        SPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(                  sp,   4 |   FULL |        SPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(                 wsp,   5 |  D_REG |        SPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(                 wzr,   6 |  D_REG |        SPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(                 xzr,   7 |   FULL |        SPR | Arch_aarch64, "aarch64");

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

  DEF_REGISTER(   id_aa64pfr1_el1,   1 |   FULL | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(        sder32_el3,   2 |  D_REG | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(        dacr32_el2,   4 |  D_REG | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(       ich_vtr_el2,   5 |  D_REG | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(         afsr0_el3,   9 |  D_REG | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(         dczid_el0,  10 |  D_REG | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(         actlr_el1,  11 |   FULL | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(         sctlr_el3,  12 |  D_REG | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(         sctlr_el2,  13 |  D_REG | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(       pmceid0_el0,  17 |  D_REG | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(         tpidr_el0,  18 |   FULL | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(    contextidr_el1,  20 |  D_REG | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(      id_mmfr2_el1,  21 |  D_REG | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(    pmevtyper0_el0,  22 |  D_REG | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(    pmevtyper1_el0,  23 |  D_REG | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(    pmevtyper2_el0,  24 |  D_REG | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(    pmevtyper3_el0,  25 |  D_REG | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(    pmevtyper4_el0,  26 |  D_REG | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(    pmevtyper5_el0,  27 |  D_REG | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(    pmevtyper6_el0,  28 |  D_REG | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(    pmevtyper7_el0,  29 |  D_REG | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(    pmevtyper8_el0,  30 |  D_REG | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(    pmevtyper9_el0,  31 |  D_REG | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(   pmevtyper10_el0,  32 |  D_REG | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(   pmevtyper11_el0,  33 |  D_REG | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(   pmevtyper12_el0,  34 |  D_REG | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(   pmevtyper13_el0,  35 |  D_REG | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(   pmevtyper14_el0,  36 |  D_REG | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(   pmevtyper15_el0,  37 |  D_REG | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(   pmevtyper16_el0,  38 |  D_REG | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(   pmevtyper17_el0,  39 |  D_REG | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(   pmevtyper18_el0,  40 |  D_REG | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(   pmevtyper19_el0,  41 |  D_REG | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(   pmevtyper20_el0,  42 |  D_REG | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(   pmevtyper21_el0,  43 |  D_REG | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(   pmevtyper22_el0,  44 |  D_REG | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(   pmevtyper23_el0,  45 |  D_REG | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(   pmevtyper24_el0,  46 |  D_REG | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(   pmevtyper25_el0,  47 |  D_REG | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(   pmevtyper26_el0,  48 |  D_REG | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(   pmevtyper27_el0,  49 |  D_REG | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(   pmevtyper28_el0,  50 |  D_REG | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(   pmevtyper29_el0,  51 |  D_REG | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(   pmevtyper30_el0,  52 |  D_REG | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(          mair_el1,  53 |   FULL | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(        ccsidr_el1,  55 |  D_REG | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(          spsr_irq,  56 |  D_REG | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(           elr_el3,  57 |   FULL | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(            sp_el0,  59 |   FULL | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(      id_mmfr1_el1,  60 |  D_REG | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(      cntp_ctl_el0,  66 |  D_REG | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(         tpidr_el1,  67 |   FULL | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(     cntv_cval_el0,  70 |   FULL | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(        mdccsr_el0,  72 |  D_REG | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(   dbgclaimset_el1,  73 |  D_REG | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(     ich_ap0r0_el2,  74 |  D_REG | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(     ich_ap0r1_el2,  75 |  D_REG | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(     ich_ap0r2_el2,  76 |  D_REG | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(     ich_ap0r3_el2,  77 |  D_REG | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(     ich_elrsr_el2,  79 |  D_REG | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(       ich_hcr_el2,  80 |  D_REG | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(          vbar_el2,  81 |   FULL | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(   id_aa64afr1_el1,  83 |   FULL | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(              nzcv,  84 |  D_REG | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(         ttbr0_el1,  85 |   FULL | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(         actlr_el3,  86 |   FULL | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(         mdrar_el1,  87 |   FULL | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(           rmr_el3,  88 |  D_REG | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(       cntkctl_el1,  89 |  D_REG | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(         rvbar_el3,  90 |   FULL | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(       id_dfr0_el1,  91 |  D_REG | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(       pmceid1_el0,  98 |  D_REG | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(       id_pfr1_el1, 100 |  D_REG | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(       id_afr0_el1, 101 |  D_REG | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(         amair_el2, 102 |   FULL | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(      pmovsset_el0, 103 |  D_REG | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(      id_isar5_el1, 107 |  D_REG | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(           tcr_el1, 108 |   FULL | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(     pmevcntr0_el0, 109 |  D_REG | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(     pmevcntr1_el0, 110 |  D_REG | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(     pmevcntr2_el0, 111 |  D_REG | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(     pmevcntr3_el0, 112 |  D_REG | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(     pmevcntr4_el0, 113 |  D_REG | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(     pmevcntr5_el0, 114 |  D_REG | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(     pmevcntr6_el0, 115 |  D_REG | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(     pmevcntr7_el0, 116 |  D_REG | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(     pmevcntr8_el0, 117 |  D_REG | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(     pmevcntr9_el0, 118 |  D_REG | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(    pmevcntr10_el0, 119 |  D_REG | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(    pmevcntr11_el0, 120 |  D_REG | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(    pmevcntr12_el0, 121 |  D_REG | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(    pmevcntr13_el0, 122 |  D_REG | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(    pmevcntr14_el0, 123 |  D_REG | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(    pmevcntr15_el0, 124 |  D_REG | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(    pmevcntr16_el0, 125 |  D_REG | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(    pmevcntr17_el0, 126 |  D_REG | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(    pmevcntr18_el0, 127 |  D_REG | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(    pmevcntr19_el0, 128 |  D_REG | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(    pmevcntr20_el0, 129 |  D_REG | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(    pmevcntr21_el0, 130 |  D_REG | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(    pmevcntr22_el0, 131 |  D_REG | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(    pmevcntr23_el0, 132 |  D_REG | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(    pmevcntr24_el0, 133 |  D_REG | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(    pmevcntr25_el0, 134 |  D_REG | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(    pmevcntr26_el0, 135 |  D_REG | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(    pmevcntr27_el0, 136 |  D_REG | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(    pmevcntr28_el0, 137 |  D_REG | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(    pmevcntr29_el0, 138 |  D_REG | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(    pmevcntr30_el0, 139 |  D_REG | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(        dbgdtr_el0, 140 |   FULL | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(         amair_el3, 141 |   FULL | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(         rvbar_el1, 142 |   FULL | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(         afsr0_el2, 144 |  D_REG | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(         hpfar_el2, 148 |   FULL | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(          spsr_abt, 149 |  D_REG | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(        csselr_el1, 150 |  D_REG | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(    pmintenclr_el1, 151 |  D_REG | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(           far_el2, 153 |   FULL | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(         currentel, 154 |  D_REG | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(       dbgprcr_el1, 155 |  D_REG | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(          mair_el3, 157 |   FULL | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(         ttbr1_el1, 158 |   FULL | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(        cntvct_el0, 159 |   FULL | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(           scr_el3, 161 |  D_REG | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(         mvfr0_el1, 162 |  D_REG | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(      pmovsclr_el0, 164 |  D_REG | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(         mvfr1_el1, 165 |  D_REG | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(           elr_el1, 167 |   FULL | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(         rvbar_el2, 168 |   FULL | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(           esr_el1, 169 |  D_REG | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(       osdtrrx_el1, 171 |  D_REG | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(    pmcntenset_el0, 172 |  D_REG | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(         dspsr_el0, 173 |  D_REG | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(      ich_misr_el2, 175 |  D_REG | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(          vbar_el1, 176 |   FULL | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(      id_isar1_el1, 177 |  D_REG | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(         cpacr_el1, 178 |  D_REG | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(           par_el1, 179 |   FULL | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(     cnthp_ctl_el2, 180 |  D_REG | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(         actlr_el2, 181 |   FULL | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(          spsr_fiq, 182 |  D_REG | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(   id_aa64pfr0_el1, 183 |   FULL | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(         tpidr_el3, 184 |   FULL | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(      id_isar4_el1, 187 |  D_REG | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(      dbgvcr32_el2, 188 |  D_REG | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(           hcr_el2, 189 |   FULL | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(      ich_eisr_el2, 190 |  D_REG | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(       mdccint_el1, 191 |  D_REG | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(           dlr_el0, 192 |   FULL | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(         tpidr_el2, 193 |   FULL | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(     pmuserenr_el0, 194 |  D_REG | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(        oseccr_el1, 195 |  D_REG | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(         mdscr_el1, 197 |  D_REG | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(      ich_vmcr_el2, 198 |  D_REG | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(           far_el3, 199 |   FULL | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(          spsr_el2, 200 |  D_REG | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(         oslsr_el1, 201 |  D_REG | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(              daif, 203 |  D_REG | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(           far_el1, 204 |   FULL | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(          hstr_el2, 205 |  D_REG | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(         clidr_el1, 206 |   FULL | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(         vttbr_el2, 207 |   FULL | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(   id_aa64afr0_el1, 208 |   FULL | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(     cntps_ctl_el1, 210 |  D_REG | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(         afsr1_el1, 211 |  D_REG | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(             spsel, 212 |  D_REG | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(          pmcr_el0, 214 |  D_REG | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(        cntpct_el0, 217 |   FULL | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(        ifsr32_el2, 220 |  D_REG | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(       tpidrro_el0, 222 |   FULL | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(          hacr_el2, 223 |  D_REG | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(     pmccfiltr_el0, 225 |  D_REG | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(    cnthp_tval_el2, 226 |  D_REG | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(       cntvoff_el2, 228 |   FULL | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(          spsr_el1, 230 |  D_REG | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(           isr_el1, 231 |  D_REG | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(       cnthctl_el2, 232 |  D_REG | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(          vtcr_el2, 233 |  D_REG | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(        vmpidr_el2, 234 |   FULL | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(       pmswinc_el0, 235 |  D_REG | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(      id_mmfr0_el1, 236 |  D_REG | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(     cntp_cval_el0, 237 |   FULL | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(          mdcr_el3, 239 |  D_REG | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(       dbgwvr0_el1, 240 |   FULL | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(       dbgwvr1_el1, 241 |   FULL | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(       dbgwvr2_el1, 242 |   FULL | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(       dbgwvr3_el1, 243 |   FULL | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(       dbgwvr4_el1, 244 |   FULL | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(       dbgwvr5_el1, 245 |   FULL | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(       dbgwvr6_el1, 246 |   FULL | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(       dbgwvr7_el1, 247 |   FULL | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(       dbgwvr8_el1, 248 |   FULL | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(       dbgwvr9_el1, 249 |   FULL | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(      dbgwvr10_el1, 250 |   FULL | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(      dbgwvr11_el1, 251 |   FULL | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(      dbgwvr12_el1, 252 |   FULL | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(      dbgwvr13_el1, 253 |   FULL | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(      dbgwvr14_el1, 254 |   FULL | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(      dbgwvr15_el1, 255 |   FULL | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER( dbgauthstatus_el1, 256 |  D_REG | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(      dbgdtrtx_el0, 257 |  D_REG | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(           elr_el2, 259 |   FULL | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(       dbgbvr0_el1, 260 |   FULL | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(       dbgbvr1_el1, 261 |   FULL | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(       dbgbvr2_el1, 262 |   FULL | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(       dbgbvr3_el1, 263 |   FULL | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(       dbgbvr4_el1, 264 |   FULL | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(       dbgbvr5_el1, 265 |   FULL | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(       dbgbvr6_el1, 266 |   FULL | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(       dbgbvr7_el1, 267 |   FULL | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(       dbgbvr8_el1, 268 |   FULL | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(       dbgbvr9_el1, 269 |   FULL | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(      dbgbvr10_el1, 270 |   FULL | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(      dbgbvr11_el1, 271 |   FULL | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(      dbgbvr12_el1, 272 |   FULL | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(      dbgbvr13_el1, 273 |   FULL | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(      dbgbvr14_el1, 274 |   FULL | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(      dbgbvr15_el1, 275 |   FULL | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(           tcr_el3, 277 |  D_REG | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(          aidr_el1, 278 |  D_REG | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(        cntfrq_el0, 279 |  D_REG | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(           esr_el2, 282 |  D_REG | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(          vbar_el3, 283 |   FULL | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(       dbgwcr0_el1, 284 |  D_REG | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(       dbgwcr1_el1, 285 |  D_REG | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(       dbgwcr2_el1, 286 |  D_REG | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(       dbgwcr3_el1, 287 |  D_REG | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(       dbgwcr4_el1, 288 |  D_REG | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(       dbgwcr5_el1, 289 |  D_REG | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(       dbgwcr6_el1, 290 |  D_REG | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(       dbgwcr7_el1, 291 |  D_REG | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(       dbgwcr8_el1, 292 |  D_REG | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(       dbgwcr9_el1, 293 |  D_REG | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(      dbgwcr10_el1, 294 |  D_REG | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(      dbgwcr11_el1, 295 |  D_REG | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(      dbgwcr12_el1, 296 |  D_REG | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(      dbgwcr13_el1, 297 |  D_REG | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(      dbgwcr14_el1, 298 |  D_REG | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(      dbgwcr15_el1, 299 |  D_REG | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(           rmr_el1, 300 |  D_REG | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(          midr_el1, 301 |  D_REG | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(      id_isar0_el1, 303 |  D_REG | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(           ctr_el0, 304 |  D_REG | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(            sp_el1, 305 |   FULL | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(     cntv_tval_el0, 306 |  D_REG | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(         afsr1_el3, 308 |  D_REG | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(      id_mmfr4_el1, 309 |  D_REG | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(         ttbr0_el3, 310 |   FULL | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(          spsr_und, 311 |  D_REG | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(         osdlr_el1, 314 |  D_REG | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(       ich_lr0_el2, 315 |   FULL | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(       ich_lr1_el2, 316 |   FULL | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(       ich_lr2_el2, 317 |   FULL | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(       ich_lr3_el2, 318 |   FULL | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(       ich_lr4_el2, 319 |   FULL | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(       ich_lr5_el2, 320 |   FULL | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(       ich_lr6_el2, 321 |   FULL | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(       ich_lr7_el2, 322 |   FULL | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(       ich_lr8_el2, 323 |   FULL | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(       ich_lr9_el2, 324 |   FULL | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(      ich_lr10_el2, 325 |   FULL | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(      ich_lr11_el2, 326 |   FULL | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(      ich_lr12_el2, 327 |   FULL | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(      ich_lr13_el2, 328 |   FULL | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(      ich_lr14_el2, 329 |   FULL | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(      ich_lr15_el2, 330 |   FULL | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(   dbgclaimclr_el1, 331 |  D_REG | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(  id_aa64mmfr1_el1, 333 |   FULL | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(       pmccntr_el0, 334 |   FULL | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(           esr_el3, 335 |  D_REG | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(          cptr_el3, 336 |  D_REG | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(     cntp_tval_el0, 337 |  D_REG | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(           tcr_el2, 338 |  D_REG | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(    pmintenset_el1, 341 |  D_REG | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(       dbgbcr0_el1, 342 |  D_REG | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(       dbgbcr1_el1, 343 |  D_REG | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(       dbgbcr2_el1, 344 |  D_REG | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(       dbgbcr3_el1, 345 |  D_REG | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(       dbgbcr4_el1, 346 |  D_REG | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(       dbgbcr5_el1, 347 |  D_REG | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(       dbgbcr6_el1, 348 |  D_REG | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(       dbgbcr7_el1, 349 |  D_REG | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(       dbgbcr8_el1, 350 |  D_REG | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(       dbgbcr9_el1, 351 |  D_REG | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(      dbgbcr10_el1, 352 |  D_REG | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(      dbgbcr11_el1, 353 |  D_REG | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(      dbgbcr12_el1, 354 |  D_REG | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(      dbgbcr13_el1, 355 |  D_REG | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(      dbgbcr14_el1, 356 |  D_REG | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(      dbgbcr15_el1, 357 |  D_REG | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(        pmselr_el0, 358 |  D_REG | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(          mdcr_el2, 362 |  D_REG | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(         vpidr_el2, 364 |  D_REG | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(   id_aa64dfr0_el1, 365 |   FULL | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(      id_isar2_el1, 366 |  D_REG | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(         ttbr0_el2, 367 |   FULL | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(      dbgdtrrx_el0, 372 |  D_REG | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(       fpexc32_el2, 373 |  D_REG | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(    pmxevtyper_el0, 374 |  D_REG | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(         oslar_el1, 375 |  D_REG | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(       id_pfr0_el1, 376 |  D_REG | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(      id_mmfr3_el1, 377 |  D_REG | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(       osdtrtx_el1, 378 |  D_REG | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(  id_aa64isar0_el1, 379 |   FULL | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(         mvfr2_el1, 380 |  D_REG | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(    pmcntenclr_el0, 381 |  D_REG | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(     ich_ap1r0_el2, 382 |  D_REG | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(     ich_ap1r1_el2, 383 |  D_REG | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(     ich_ap1r2_el2, 384 |  D_REG | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(     ich_ap1r3_el2, 385 |  D_REG | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(  id_aa64mmfr0_el1, 386 |   FULL | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(        revidr_el1, 388 |  D_REG | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(          mair_el2, 389 |   FULL | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(         sctlr_el1, 390 |  D_REG | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(          cptr_el2, 392 |  D_REG | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(     pmxevcntr_el0, 394 |  D_REG | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(         afsr0_el1, 395 |  D_REG | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(      id_isar3_el1, 397 |  D_REG | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(         afsr1_el2, 398 |  D_REG | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(            sp_el2, 400 |   FULL | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(    cntps_cval_el1, 401 |   FULL | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(    cntps_tval_el1, 402 |  D_REG | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(      cntv_ctl_el0, 404 |  D_REG | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(           rmr_el2, 405 |  D_REG | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(   id_aa64dfr1_el1, 406 |   FULL | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(    cnthp_cval_el2, 407 |   FULL | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(  id_aa64isar1_el1, 409 |   FULL | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(         mpidr_el1, 410 |   FULL | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(         amair_el1, 411 |   FULL | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(          spsr_el3, 413 |  D_REG | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(              fpsr, 414 |  D_REG | SYSREG | Arch_aarch64, "aarch64");

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
  DEF_REGISTER_ALIAS(x29, fp, "aarch64");
  DEF_REGISTER_ALIAS(x30, lr, "aarch64");
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
