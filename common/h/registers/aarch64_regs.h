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

namespace Dyninst { namespace aarch64 {

  /**
   * For interpreting constants:
   *  Lowest 16 bits (0x000000ff) is base register ID
   *  Next 16 bits (0x0000ff00) is the aliasing and subrange ID-
   *    used on x86/x86_64 to distinguish between things like EAX and AH
   *  Next 16 bits (0x00ff0000) are the register category, GPR/FPR/MMX/...
   *  Top 16 bits (0xff000000) are the architecture.
   *
   *  These values/layout are not guaranteed to remain the same as part of the
   *  public interface, and may change.
   **/

  // 0xff000000  0x00ff0000      0x0000ff00      0x000000ff
  // arch        reg cat:GPR     alias&subrange  reg ID
  const signed int GPR    = 0x00010000;
  const signed int FPR    = 0x00020000;
  const signed int FLAG   = 0x00030000;
  const signed int FSR    = 0x00040000;
  const signed int SPR    = 0x00080000;
  const signed int SYSREG = 0x00100000;

  const signed int BIT    = 0x00008000;
  const signed int B_REG  = 0x00000100;  // 8bit  byte reg
  const signed int W_REG  = 0x00000300;  // 16bit half-wor reg
  const signed int D_REG  = 0x00000f00;  // 32bit single-word reg
  const signed int FULL   = 0x00000000;  // 64bit double-word reg
  const signed int Q_REG  = 0x00000400;  // 128bit reg
  const signed int HQ_REG = 0x00000500;  // second 64bit in 128bit reg

  // 31 GPRs, double word long registers
  //          (              name,  ID |  alias |    cat |         arch,      arch)
  DEF_REGISTER(                x0,   0 |   FULL |    GPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(                w0,   0 |  D_REG |    GPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(                x1,   1 |   FULL |    GPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(                w1,   1 |  D_REG |    GPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(                x2,   2 |   FULL |    GPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(                w2,   2 |  D_REG |    GPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(                x3,   3 |   FULL |    GPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(                w3,   3 |  D_REG |    GPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(                x4,   4 |   FULL |    GPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(                w4,   4 |  D_REG |    GPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(                x5,   5 |   FULL |    GPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(                w5,   5 |  D_REG |    GPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(                x6,   6 |   FULL |    GPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(                w6,   6 |  D_REG |    GPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(                x7,   7 |   FULL |    GPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(                w7,   7 |  D_REG |    GPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(                x8,   8 |   FULL |    GPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(                w8,   8 |  D_REG |    GPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(                x9,   9 |   FULL |    GPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(                w9,   9 |  D_REG |    GPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(               x10,  10 |   FULL |    GPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(               w10,  10 |  D_REG |    GPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(               x11,  11 |   FULL |    GPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(               w11,  11 |  D_REG |    GPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(               x12,  12 |   FULL |    GPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(               w12,  12 |  D_REG |    GPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(               x13,  13 |   FULL |    GPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(               w13,  13 |  D_REG |    GPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(               x14,  14 |   FULL |    GPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(               w14,  14 |  D_REG |    GPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(               x15,  15 |   FULL |    GPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(               w15,  15 |  D_REG |    GPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(               x16,  16 |   FULL |    GPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(               w16,  16 |  D_REG |    GPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(               x17,  17 |   FULL |    GPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(               w17,  17 |  D_REG |    GPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(               x18,  18 |   FULL |    GPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(               w18,  18 |  D_REG |    GPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(               x19,  19 |   FULL |    GPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(               w19,  19 |  D_REG |    GPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(               x20,  20 |   FULL |    GPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(               w20,  20 |  D_REG |    GPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(               x21,  21 |   FULL |    GPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(               w21,  21 |  D_REG |    GPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(               x22,  22 |   FULL |    GPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(               w22,  22 |  D_REG |    GPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(               x23,  23 |   FULL |    GPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(               w23,  23 |  D_REG |    GPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(               x24,  24 |   FULL |    GPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(               w24,  24 |  D_REG |    GPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(               x25,  25 |   FULL |    GPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(               w25,  25 |  D_REG |    GPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(               x26,  26 |   FULL |    GPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(               w26,  26 |  D_REG |    GPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(               x27,  27 |   FULL |    GPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(               w27,  27 |  D_REG |    GPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(               x28,  28 |   FULL |    GPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(               w28,  28 |  D_REG |    GPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(               x29,  29 |   FULL |    GPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(               w29,  29 |  D_REG |    GPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(               x30,  30 |   FULL |    GPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(               w30,  30 |  D_REG |    GPR | Arch_aarch64, "aarch64");

  // 32 FPRs-----------q-d-s-h-b
  // 128 bit
  DEF_REGISTER(                q0,   0 |  Q_REG |    FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(                q1,   1 |  Q_REG |    FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(                q2,   2 |  Q_REG |    FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(                q3,   3 |  Q_REG |    FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(                q4,   4 |  Q_REG |    FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(                q5,   5 |  Q_REG |    FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(                q6,   6 |  Q_REG |    FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(                q7,   7 |  Q_REG |    FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(                q8,   8 |  Q_REG |    FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(                q9,   9 |  Q_REG |    FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(               q10,  10 |  Q_REG |    FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(               q11,  11 |  Q_REG |    FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(               q12,  12 |  Q_REG |    FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(               q13,  13 |  Q_REG |    FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(               q14,  14 |  Q_REG |    FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(               q15,  15 |  Q_REG |    FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(               q16,  16 |  Q_REG |    FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(               q17,  17 |  Q_REG |    FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(               q18,  18 |  Q_REG |    FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(               q19,  19 |  Q_REG |    FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(               q20,  20 |  Q_REG |    FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(               q21,  21 |  Q_REG |    FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(               q22,  22 |  Q_REG |    FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(               q23,  23 |  Q_REG |    FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(               q24,  24 |  Q_REG |    FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(               q25,  25 |  Q_REG |    FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(               q26,  26 |  Q_REG |    FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(               q27,  27 |  Q_REG |    FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(               q28,  28 |  Q_REG |    FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(               q29,  29 |  Q_REG |    FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(               q30,  30 |  Q_REG |    FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(               q31,  31 |  Q_REG |    FPR | Arch_aarch64, "aarch64");

  // second 64bit
  DEF_REGISTER(               hq0,   0 | HQ_REG |    FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(               hq1,   1 | HQ_REG |    FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(               hq2,   2 | HQ_REG |    FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(               hq3,   3 | HQ_REG |    FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(               hq4,   4 | HQ_REG |    FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(               hq5,   5 | HQ_REG |    FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(               hq6,   6 | HQ_REG |    FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(               hq7,   7 | HQ_REG |    FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(               hq8,   8 | HQ_REG |    FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(               hq9,   9 | HQ_REG |    FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(              hq10,  10 | HQ_REG |    FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(              hq11,  11 | HQ_REG |    FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(              hq12,  12 | HQ_REG |    FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(              hq13,  13 | HQ_REG |    FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(              hq14,  14 | HQ_REG |    FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(              hq15,  15 | HQ_REG |    FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(              hq16,  16 | HQ_REG |    FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(              hq17,  17 | HQ_REG |    FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(              hq18,  18 | HQ_REG |    FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(              hq19,  19 | HQ_REG |    FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(              hq20,  20 | HQ_REG |    FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(              hq21,  21 | HQ_REG |    FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(              hq22,  22 | HQ_REG |    FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(              hq23,  23 | HQ_REG |    FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(              hq24,  24 | HQ_REG |    FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(              hq25,  25 | HQ_REG |    FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(              hq26,  26 | HQ_REG |    FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(              hq27,  27 | HQ_REG |    FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(              hq28,  28 | HQ_REG |    FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(              hq29,  29 | HQ_REG |    FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(              hq30,  30 | HQ_REG |    FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(              hq31,  31 | HQ_REG |    FPR | Arch_aarch64, "aarch64");

  // 64bit FP regs
  DEF_REGISTER(                d0,   0 |   FULL |    FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(                d1,   1 |   FULL |    FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(                d2,   2 |   FULL |    FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(                d3,   3 |   FULL |    FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(                d4,   4 |   FULL |    FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(                d5,   5 |   FULL |    FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(                d6,   6 |   FULL |    FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(                d7,   7 |   FULL |    FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(                d8,   8 |   FULL |    FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(                d9,   9 |   FULL |    FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(               d10,  10 |   FULL |    FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(               d11,  11 |   FULL |    FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(               d12,  12 |   FULL |    FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(               d13,  13 |   FULL |    FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(               d14,  14 |   FULL |    FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(               d15,  15 |   FULL |    FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(               d16,  16 |   FULL |    FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(               d17,  17 |   FULL |    FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(               d18,  18 |   FULL |    FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(               d19,  19 |   FULL |    FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(               d20,  20 |   FULL |    FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(               d21,  21 |   FULL |    FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(               d22,  22 |   FULL |    FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(               d23,  23 |   FULL |    FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(               d24,  24 |   FULL |    FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(               d25,  25 |   FULL |    FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(               d26,  26 |   FULL |    FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(               d27,  27 |   FULL |    FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(               d28,  28 |   FULL |    FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(               d29,  29 |   FULL |    FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(               d30,  30 |   FULL |    FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(               d31,  31 |   FULL |    FPR | Arch_aarch64, "aarch64");

  // 32 bit FP regs
  DEF_REGISTER(                s0,   0 |  D_REG |    FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(                s1,   1 |  D_REG |    FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(                s2,   2 |  D_REG |    FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(                s3,   3 |  D_REG |    FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(                s4,   4 |  D_REG |    FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(                s5,   5 |  D_REG |    FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(                s6,   6 |  D_REG |    FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(                s7,   7 |  D_REG |    FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(                s8,   8 |  D_REG |    FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(                s9,   9 |  D_REG |    FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(               s10,  10 |  D_REG |    FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(               s11,  11 |  D_REG |    FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(               s12,  12 |  D_REG |    FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(               s13,  13 |  D_REG |    FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(               s14,  14 |  D_REG |    FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(               s15,  15 |  D_REG |    FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(               s16,  16 |  D_REG |    FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(               s17,  17 |  D_REG |    FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(               s18,  18 |  D_REG |    FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(               s19,  19 |  D_REG |    FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(               s20,  20 |  D_REG |    FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(               s21,  21 |  D_REG |    FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(               s22,  22 |  D_REG |    FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(               s23,  23 |  D_REG |    FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(               s24,  24 |  D_REG |    FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(               s25,  25 |  D_REG |    FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(               s26,  26 |  D_REG |    FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(               s27,  27 |  D_REG |    FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(               s28,  28 |  D_REG |    FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(               s29,  29 |  D_REG |    FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(               s30,  30 |  D_REG |    FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(               s31,  31 |  D_REG |    FPR | Arch_aarch64, "aarch64");

  // 16 bit FP regs
  DEF_REGISTER(                h0,   0 |  W_REG |    FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(                h1,   1 |  W_REG |    FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(                h2,   2 |  W_REG |    FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(                h3,   3 |  W_REG |    FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(                h4,   4 |  W_REG |    FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(                h5,   5 |  W_REG |    FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(                h6,   6 |  W_REG |    FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(                h7,   7 |  W_REG |    FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(                h8,   8 |  W_REG |    FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(                h9,   9 |  W_REG |    FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(               h10,  10 |  W_REG |    FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(               h11,  11 |  W_REG |    FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(               h12,  12 |  W_REG |    FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(               h13,  13 |  W_REG |    FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(               h14,  14 |  W_REG |    FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(               h15,  15 |  W_REG |    FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(               h16,  16 |  W_REG |    FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(               h17,  17 |  W_REG |    FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(               h18,  18 |  W_REG |    FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(               h19,  19 |  W_REG |    FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(               h20,  20 |  W_REG |    FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(               h21,  21 |  W_REG |    FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(               h22,  22 |  W_REG |    FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(               h23,  23 |  W_REG |    FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(               h24,  24 |  W_REG |    FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(               h25,  25 |  W_REG |    FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(               h26,  26 |  W_REG |    FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(               h27,  27 |  W_REG |    FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(               h28,  28 |  W_REG |    FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(               h29,  29 |  W_REG |    FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(               h30,  30 |  W_REG |    FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(               h31,  31 |  W_REG |    FPR | Arch_aarch64, "aarch64");

  // 8 bit FP regs
  DEF_REGISTER(                b0,   0 |  B_REG |    FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(                b1,   1 |  B_REG |    FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(                b2,   2 |  B_REG |    FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(                b3,   3 |  B_REG |    FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(                b4,   4 |  B_REG |    FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(                b5,   5 |  B_REG |    FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(                b6,   6 |  B_REG |    FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(                b7,   7 |  B_REG |    FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(                b8,   8 |  B_REG |    FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(                b9,   9 |  B_REG |    FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(               b10,  10 |  B_REG |    FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(               b11,  11 |  B_REG |    FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(               b12,  12 |  B_REG |    FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(               b13,  13 |  B_REG |    FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(               b14,  14 |  B_REG |    FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(               b15,  15 |  B_REG |    FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(               b16,  16 |  B_REG |    FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(               b17,  17 |  B_REG |    FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(               b18,  18 |  B_REG |    FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(               b19,  19 |  B_REG |    FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(               b20,  20 |  B_REG |    FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(               b21,  21 |  B_REG |    FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(               b22,  22 |  B_REG |    FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(               b23,  23 |  B_REG |    FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(               b24,  24 |  B_REG |    FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(               b25,  25 |  B_REG |    FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(               b26,  26 |  B_REG |    FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(               b27,  27 |  B_REG |    FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(               b28,  28 |  B_REG |    FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(               b29,  29 |  B_REG |    FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(               b30,  30 |  B_REG |    FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(               b31,  31 |  B_REG |    FPR | Arch_aarch64, "aarch64");

  DEF_REGISTER(      tlbi_vale3is,   0 |   FULL | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(   id_aa64pfr1_el1,   1 |   FULL | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(        sder32_el3,   2 |  D_REG | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(      icc_ctlr_el3,   3 |  D_REG | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(        dacr32_el2,   4 |  D_REG | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(       ich_vtr_el2,   5 |  D_REG | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(       icc_dir_el1,   6 |  D_REG | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(     tlbi_ipas2le1,   7 |   FULL | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(         afsr0_el3,   9 |  D_REG | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(         dczid_el0,  10 |  D_REG | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(         actlr_el1,  11 |   FULL | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(         sctlr_el3,  12 |  D_REG | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(         sctlr_el2,  13 |  D_REG | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(          at_s1e2r,  14 |   FULL | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(          ic_iallu,  15 |   FULL | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(   icc_igrpen1_el3,  16 |  D_REG | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(       pmceid0_el0,  17 |  D_REG | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(         tpidr_el0,  18 |   FULL | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(   tlbi_ipas2le1is,  19 |   FULL | SYSREG | Arch_aarch64, "aarch64");
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
  DEF_REGISTER(      tlbi_vale2is,  54 |   FULL | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(        ccsidr_el1,  55 |  D_REG | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(          spsr_irq,  56 |  D_REG | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(           elr_el3,  57 |   FULL | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(      tlbi_alle3is,  58 |   FULL | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(            sp_el0,  59 |   FULL | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(      id_mmfr1_el1,  60 |  D_REG | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(           ic_ivau,  61 |   FULL | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(     icc_eoir0_el1,  62 |  D_REG | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(     icc_sgi0r_el1,  63 |   FULL | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(     icc_sgi1r_el1,  64 |   FULL | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(         tlbi_vae2,  65 |   FULL | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(      cntp_ctl_el0,  66 |  D_REG | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(         tpidr_el1,  67 |   FULL | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(          dc_civac,  68 |   FULL | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(    tlbi_ipas2e1is,  69 |   FULL | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(     cntv_cval_el0,  70 |   FULL | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(      icc_bpr0_el1,  71 |  D_REG | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(        mdccsr_el0,  72 |  D_REG | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(   dbgclaimset_el1,  73 |  D_REG | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(     ich_ap0r0_el2,  74 |  D_REG | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(     ich_ap0r1_el2,  75 |  D_REG | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(     ich_ap0r2_el2,  76 |  D_REG | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(     ich_ap0r3_el2,  77 |  D_REG | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(           dc_ivac,  78 |   FULL | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(     ich_elrsr_el2,  79 |  D_REG | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(       ich_hcr_el2,  80 |  D_REG | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(          vbar_el2,  81 |   FULL | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(    icc_asgi1r_el1,  82 |   FULL | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(   id_aa64afr1_el1,  83 |   FULL | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(              nzcv,  84 |  D_REG | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(         ttbr0_el1,  85 |   FULL | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(         actlr_el3,  86 |   FULL | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(         mdrar_el1,  87 |   FULL | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(           rmr_el3,  88 |  D_REG | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(       cntkctl_el1,  89 |  D_REG | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(         rvbar_el3,  90 |   FULL | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(       id_dfr0_el1,  91 |  D_REG | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(          at_s1e1r,  92 |   FULL | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(     icc_ap1r0_el1,  93 |  D_REG | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(     icc_ap1r1_el1,  94 |  D_REG | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(     icc_ap1r2_el1,  95 |  D_REG | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(     icc_ap1r3_el1,  96 |  D_REG | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(       icc_pmr_el1,  97 |  D_REG | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(       pmceid1_el0,  98 |  D_REG | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(            dc_csw,  99 |   FULL | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(       id_pfr1_el1, 100 |  D_REG | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(       id_afr0_el1, 101 |  D_REG | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(         amair_el2, 102 |   FULL | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(      pmovsset_el0, 103 |  D_REG | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(      tlbi_vmalle1, 104 |   FULL | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(        tlbi_vale2, 105 |   FULL | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(         at_s12e0w, 106 |   FULL | SYSREG | Arch_aarch64, "aarch64");
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
  DEF_REGISTER(           dc_cvau, 143 |   FULL | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(         afsr0_el2, 144 |  D_REG | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(      icc_iar0_el1, 145 |  D_REG | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(      icc_bpr1_el1, 146 |  D_REG | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(           dc_cisw, 147 |   FULL | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(         hpfar_el2, 148 |   FULL | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(          spsr_abt, 149 |  D_REG | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(        csselr_el1, 150 |  D_REG | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(    pmintenclr_el1, 151 |  D_REG | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(        tlbi_vaae1, 152 |   FULL | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(           far_el2, 153 |   FULL | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(         currentel, 154 |  D_REG | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(       dbgprcr_el1, 155 |  D_REG | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(      tlbi_ipas2e1, 156 |   FULL | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(          mair_el3, 157 |   FULL | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(         ttbr1_el1, 158 |   FULL | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(        cntvct_el0, 159 |   FULL | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(     tlbi_aside1is, 160 |   FULL | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(           scr_el3, 161 |  D_REG | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(         mvfr0_el1, 162 |  D_REG | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(        tlbi_alle1, 163 |   FULL | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(      pmovsclr_el0, 164 |  D_REG | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(         mvfr1_el1, 165 |  D_REG | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(       tlbi_vaale1, 166 |   FULL | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(           elr_el1, 167 |   FULL | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(         rvbar_el2, 168 |   FULL | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(           esr_el1, 169 |  D_REG | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(       tlbi_aside1, 170 |   FULL | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(       osdtrrx_el1, 171 |  D_REG | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(    pmcntenset_el0, 172 |  D_REG | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(         dspsr_el0, 173 |  D_REG | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(            dc_zva, 174 |   FULL | SYSREG | Arch_aarch64, "aarch64");
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
  DEF_REGISTER(          at_s1e2w, 185 |   FULL | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(        tlbi_alle2, 186 |   FULL | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(      id_isar4_el1, 187 |  D_REG | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(      dbgvcr32_el2, 188 |  D_REG | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(           hcr_el2, 189 |   FULL | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(      ich_eisr_el2, 190 |  D_REG | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(       mdccint_el1, 191 |  D_REG | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(           dlr_el0, 192 |   FULL | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(         tpidr_el2, 193 |   FULL | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(     pmuserenr_el0, 194 |  D_REG | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(        oseccr_el1, 195 |  D_REG | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER( tlbi_vmalls12e1is, 196 |   FULL | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(         mdscr_el1, 197 |  D_REG | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(      ich_vmcr_el2, 198 |  D_REG | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(           far_el3, 199 |   FULL | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(          spsr_el2, 200 |  D_REG | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(         oslsr_el1, 201 |  D_REG | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(         tlbi_vae1, 202 |   FULL | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(              daif, 203 |  D_REG | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(           far_el1, 204 |   FULL | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(          hstr_el2, 205 |  D_REG | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(         clidr_el1, 206 |   FULL | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(         vttbr_el2, 207 |   FULL | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(   id_aa64afr0_el1, 208 |   FULL | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(       icc_rpr_el1, 209 |  D_REG | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(     cntps_ctl_el1, 210 |  D_REG | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(         afsr1_el1, 211 |  D_REG | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(             spsel, 212 |  D_REG | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(          pmcr_el0, 214 |  D_REG | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(      tlbi_vaae1is, 215 |   FULL | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(       icc_sre_el1, 216 |  D_REG | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(        cntpct_el0, 217 |   FULL | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(      tlbi_alle2is, 218 |   FULL | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(   icc_igrpen1_el1, 219 |  D_REG | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(        ifsr32_el2, 220 |  D_REG | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(   icc_igrpen0_el1, 221 |  D_REG | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(       tpidrro_el0, 222 |   FULL | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(          hacr_el2, 223 |  D_REG | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(          at_s1e0w, 224 |   FULL | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(     pmccfiltr_el0, 225 |  D_REG | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(    cnthp_tval_el2, 226 |  D_REG | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(         at_s12e0r, 227 |   FULL | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(       cntvoff_el2, 228 |   FULL | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(       icc_sre_el2, 229 |  D_REG | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(          spsr_el1, 230 |  D_REG | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(           isr_el1, 231 |  D_REG | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(       cnthctl_el2, 232 |  D_REG | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(          vtcr_el2, 233 |  D_REG | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(        vmpidr_el2, 234 |   FULL | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(       pmswinc_el0, 235 |  D_REG | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(      id_mmfr0_el1, 236 |  D_REG | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(     cntp_cval_el0, 237 |   FULL | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(    icc_hppir0_el1, 238 |  D_REG | SYSREG | Arch_aarch64, "aarch64");
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
  DEF_REGISTER(     icc_eoir1_el1, 258 |  D_REG | SYSREG | Arch_aarch64, "aarch64");
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
  DEF_REGISTER(        ic_ialluis, 276 |   FULL | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(           tcr_el3, 277 |  D_REG | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(          aidr_el1, 278 |  D_REG | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(        cntfrq_el0, 279 |  D_REG | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(         tlbi_vae3, 280 |   FULL | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(   tlbi_vmalls12e1, 281 |   FULL | SYSREG | Arch_aarch64, "aarch64");
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
  DEF_REGISTER(            dc_isw, 302 |   FULL | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(      id_isar0_el1, 303 |  D_REG | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(           ctr_el0, 304 |  D_REG | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(            sp_el1, 305 |   FULL | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(     cntv_tval_el0, 306 |  D_REG | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(       tlbi_vae3is, 307 |   FULL | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(         afsr1_el3, 308 |  D_REG | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(      id_mmfr4_el1, 309 |  D_REG | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(         ttbr0_el3, 310 |   FULL | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(          spsr_und, 311 |  D_REG | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(       tlbi_vae2is, 312 |   FULL | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(          at_s1e3w, 313 |   FULL | SYSREG | Arch_aarch64, "aarch64");
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
  DEF_REGISTER(    tlbi_vmalle1is, 332 |   FULL | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(  id_aa64mmfr1_el1, 333 |   FULL | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(       pmccntr_el0, 334 |   FULL | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(           esr_el3, 335 |  D_REG | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(          cptr_el3, 336 |  D_REG | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(     cntp_tval_el0, 337 |  D_REG | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(           tcr_el2, 338 |  D_REG | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(           dc_cvac, 339 |   FULL | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(     tlbi_vaale1is, 340 |   FULL | SYSREG | Arch_aarch64, "aarch64");
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
  DEF_REGISTER(      tlbi_alle1is, 359 |   FULL | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(    icc_hppir1_el1, 360 |  D_REG | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(       tlbi_vae1is, 361 |   FULL | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(          mdcr_el2, 362 |  D_REG | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(         at_s12e1w, 363 |   FULL | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(         vpidr_el2, 364 |  D_REG | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(   id_aa64dfr0_el1, 365 |   FULL | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(      id_isar2_el1, 366 |  D_REG | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(         ttbr0_el2, 367 |   FULL | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(     icc_ap0r0_el1, 368 |  D_REG | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(     icc_ap0r1_el1, 369 |  D_REG | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(     icc_ap0r2_el1, 370 |  D_REG | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(     icc_ap0r3_el1, 371 |  D_REG | SYSREG | Arch_aarch64, "aarch64");
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
  DEF_REGISTER(        tlbi_vale3, 387 |   FULL | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(        revidr_el1, 388 |  D_REG | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(          mair_el2, 389 |   FULL | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(         sctlr_el1, 390 |  D_REG | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(          at_s1e0r, 391 |   FULL | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(          cptr_el2, 392 |  D_REG | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(         at_s12e1r, 393 |   FULL | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(     pmxevcntr_el0, 394 |  D_REG | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(         afsr0_el1, 395 |  D_REG | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(          at_s1e1w, 396 |   FULL | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(      id_isar3_el1, 397 |  D_REG | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(         afsr1_el2, 398 |  D_REG | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(      tlbi_vale1is, 399 |   FULL | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(            sp_el2, 400 |   FULL | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(    cntps_cval_el1, 401 |   FULL | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(    cntps_tval_el1, 402 |  D_REG | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(      icc_ctlr_el1, 403 |  D_REG | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(      cntv_ctl_el0, 404 |  D_REG | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(           rmr_el2, 405 |  D_REG | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(   id_aa64dfr1_el1, 406 |   FULL | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(    cnthp_cval_el2, 407 |   FULL | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(          at_s1e3r, 408 |   FULL | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(  id_aa64isar1_el1, 409 |   FULL | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(         mpidr_el1, 410 |   FULL | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(         amair_el1, 411 |   FULL | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(        tlbi_alle3, 412 |   FULL | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(          spsr_el3, 413 |  D_REG | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(        tlbi_vale1, 414 |   FULL | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(       icc_sre_el3, 415 |  D_REG | SYSREG | Arch_aarch64, "aarch64");
  DEF_REGISTER(      icc_iar1_el1, 416 |  D_REG | SYSREG | Arch_aarch64, "aarch64");

  DEF_REGISTER(IMPLEMENTATION_DEFINED_SYSREG,  417 | D_REG | SYSREG | Arch_aarch64, "aarch64");

  // GPRs aliases:
  // by convention
  // x29 is used as frame pointer
  // x30 is the linking register
  // x31 can be sp or zero register depending on the context

  // special registers
  // PC is not writable in aarch64
  const signed int N_FLAG = 31;
  const signed int Z_FLAG = 30;
  const signed int C_FLAG = 29;
  const signed int V_FLAG = 28;

  DEF_REGISTER(                sp,      31 |  FULL |    SPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(               wsp,       0 | D_REG |    SPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(                pc,      32 |  FULL |    SPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(            pstate,       2 | D_REG |    SPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(               xzr,       3 |  FULL |    SPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(                 n,  N_FLAG |   BIT |   FLAG | Arch_aarch64, "aarch64");
  DEF_REGISTER(                 z,  Z_FLAG |   BIT |   FLAG | Arch_aarch64, "aarch64");
  DEF_REGISTER(                 c,  C_FLAG |   BIT |   FLAG | Arch_aarch64, "aarch64");
  DEF_REGISTER(                 v,  V_FLAG |   BIT |   FLAG | Arch_aarch64, "aarch64");
  DEF_REGISTER(               wzr,       3 | D_REG |    SPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(              fpcr,       4 | D_REG |    SPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(              fpsr,       5 | D_REG |    SPR | Arch_aarch64, "aarch64");

} // end of aarch64 namespace

}

#endif
