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
  const signed int GPR = 0x00010000;
  const signed int FPR = 0x00020000;
  const signed int FLAG = 0x00030000;
  const signed int FSR = 0x00040000;
  const signed int SPR = 0x00080000;
  const signed int SYSREG = 0x00100000;

  const signed int BIT = 0x00008000;
  const signed int B_REG = 0x00000100;  // 8bit  byte reg
  const signed int W_REG = 0x00000300;  // 16bit half-wor reg
  const signed int D_REG = 0x00000f00;  // 32bit single-word reg
  const signed int FULL = 0x00000000;   // 64bit double-word reg
  const signed int Q_REG = 0x00000400;  // 128bit reg
  const signed int HQ_REG = 0x00000500; // second 64bit in 128bit reg

  // 31 GPRs, double word long registers
  //           (name   regID| alias | cat | arch           arch    )
  DEF_REGISTER(x0, 0 | FULL | GPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(w0, 0 | D_REG | GPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(x1, 1 | FULL | GPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(w1, 1 | D_REG | GPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(x2, 2 | FULL | GPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(w2, 2 | D_REG | GPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(x3, 3 | FULL | GPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(w3, 3 | D_REG | GPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(x4, 4 | FULL | GPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(w4, 4 | D_REG | GPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(x5, 5 | FULL | GPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(w5, 5 | D_REG | GPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(x6, 6 | FULL | GPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(w6, 6 | D_REG | GPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(x7, 7 | FULL | GPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(w7, 7 | D_REG | GPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(x8, 8 | FULL | GPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(w8, 8 | D_REG | GPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(x9, 9 | FULL | GPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(w9, 9 | D_REG | GPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(x10, 10 | FULL | GPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(w10, 10 | D_REG | GPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(x11, 11 | FULL | GPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(w11, 11 | D_REG | GPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(x12, 12 | FULL | GPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(w12, 12 | D_REG | GPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(x13, 13 | FULL | GPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(w13, 13 | D_REG | GPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(x14, 14 | FULL | GPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(w14, 14 | D_REG | GPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(x15, 15 | FULL | GPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(w15, 15 | D_REG | GPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(x16, 16 | FULL | GPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(w16, 16 | D_REG | GPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(x17, 17 | FULL | GPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(w17, 17 | D_REG | GPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(x18, 18 | FULL | GPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(w18, 18 | D_REG | GPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(x19, 19 | FULL | GPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(w19, 19 | D_REG | GPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(x20, 20 | FULL | GPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(w20, 20 | D_REG | GPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(x21, 21 | FULL | GPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(w21, 21 | D_REG | GPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(x22, 22 | FULL | GPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(w22, 22 | D_REG | GPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(x23, 23 | FULL | GPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(w23, 23 | D_REG | GPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(x24, 24 | FULL | GPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(w24, 24 | D_REG | GPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(x25, 25 | FULL | GPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(w25, 25 | D_REG | GPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(x26, 26 | FULL | GPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(w26, 26 | D_REG | GPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(x27, 27 | FULL | GPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(w27, 27 | D_REG | GPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(x28, 28 | FULL | GPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(w28, 28 | D_REG | GPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(x29, 29 | FULL | GPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(w29, 29 | D_REG | GPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(x30, 30 | FULL | GPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(w30, 30 | D_REG | GPR | Arch_aarch64, "aarch64");

  // 32 FPRs-----------q-d-s-h-b
  // 128 bit
  DEF_REGISTER(q0, 0 | Q_REG | FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(q1, 1 | Q_REG | FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(q2, 2 | Q_REG | FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(q3, 3 | Q_REG | FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(q4, 4 | Q_REG | FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(q5, 5 | Q_REG | FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(q6, 6 | Q_REG | FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(q7, 7 | Q_REG | FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(q8, 8 | Q_REG | FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(q9, 9 | Q_REG | FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(q10, 10 | Q_REG | FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(q11, 11 | Q_REG | FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(q12, 12 | Q_REG | FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(q13, 13 | Q_REG | FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(q14, 14 | Q_REG | FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(q15, 15 | Q_REG | FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(q16, 16 | Q_REG | FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(q17, 17 | Q_REG | FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(q18, 18 | Q_REG | FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(q19, 19 | Q_REG | FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(q20, 20 | Q_REG | FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(q21, 21 | Q_REG | FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(q22, 22 | Q_REG | FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(q23, 23 | Q_REG | FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(q24, 24 | Q_REG | FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(q25, 25 | Q_REG | FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(q26, 26 | Q_REG | FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(q27, 27 | Q_REG | FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(q28, 28 | Q_REG | FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(q29, 29 | Q_REG | FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(q30, 30 | Q_REG | FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(q31, 31 | Q_REG | FPR | Arch_aarch64, "aarch64");

  // second 64bit
  DEF_REGISTER(hq0, 0 | HQ_REG | FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(hq1, 1 | HQ_REG | FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(hq2, 2 | HQ_REG | FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(hq3, 3 | HQ_REG | FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(hq4, 4 | HQ_REG | FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(hq5, 5 | HQ_REG | FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(hq6, 6 | HQ_REG | FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(hq7, 7 | HQ_REG | FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(hq8, 8 | HQ_REG | FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(hq9, 9 | HQ_REG | FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(hq10, 10 | HQ_REG | FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(hq11, 11 | HQ_REG | FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(hq12, 12 | HQ_REG | FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(hq13, 13 | HQ_REG | FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(hq14, 14 | HQ_REG | FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(hq15, 15 | HQ_REG | FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(hq16, 16 | HQ_REG | FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(hq17, 17 | HQ_REG | FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(hq18, 18 | HQ_REG | FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(hq19, 19 | HQ_REG | FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(hq20, 20 | HQ_REG | FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(hq21, 21 | HQ_REG | FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(hq22, 22 | HQ_REG | FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(hq23, 23 | HQ_REG | FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(hq24, 24 | HQ_REG | FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(hq25, 25 | HQ_REG | FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(hq26, 26 | HQ_REG | FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(hq27, 27 | HQ_REG | FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(hq28, 28 | HQ_REG | FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(hq29, 29 | HQ_REG | FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(hq30, 30 | HQ_REG | FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(hq31, 31 | HQ_REG | FPR | Arch_aarch64, "aarch64");

  // 64bit FP regs
  DEF_REGISTER(d0, 0 | FULL | FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(d1, 1 | FULL | FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(d2, 2 | FULL | FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(d3, 3 | FULL | FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(d4, 4 | FULL | FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(d5, 5 | FULL | FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(d6, 6 | FULL | FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(d7, 7 | FULL | FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(d8, 8 | FULL | FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(d9, 9 | FULL | FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(d10, 10 | FULL | FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(d11, 11 | FULL | FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(d12, 12 | FULL | FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(d13, 13 | FULL | FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(d14, 14 | FULL | FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(d15, 15 | FULL | FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(d16, 16 | FULL | FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(d17, 17 | FULL | FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(d18, 18 | FULL | FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(d19, 19 | FULL | FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(d20, 20 | FULL | FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(d21, 21 | FULL | FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(d22, 22 | FULL | FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(d23, 23 | FULL | FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(d24, 24 | FULL | FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(d25, 25 | FULL | FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(d26, 26 | FULL | FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(d27, 27 | FULL | FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(d28, 28 | FULL | FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(d29, 29 | FULL | FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(d30, 30 | FULL | FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(d31, 31 | FULL | FPR | Arch_aarch64, "aarch64");

  // 32 bit FP regs
  DEF_REGISTER(s0, 0 | D_REG | FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(s1, 1 | D_REG | FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(s2, 2 | D_REG | FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(s3, 3 | D_REG | FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(s4, 4 | D_REG | FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(s5, 5 | D_REG | FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(s6, 6 | D_REG | FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(s7, 7 | D_REG | FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(s8, 8 | D_REG | FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(s9, 9 | D_REG | FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(s10, 10 | D_REG | FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(s11, 11 | D_REG | FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(s12, 12 | D_REG | FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(s13, 13 | D_REG | FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(s14, 14 | D_REG | FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(s15, 15 | D_REG | FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(s16, 16 | D_REG | FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(s17, 17 | D_REG | FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(s18, 18 | D_REG | FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(s19, 19 | D_REG | FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(s20, 20 | D_REG | FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(s21, 21 | D_REG | FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(s22, 22 | D_REG | FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(s23, 23 | D_REG | FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(s24, 24 | D_REG | FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(s25, 25 | D_REG | FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(s26, 26 | D_REG | FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(s27, 27 | D_REG | FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(s28, 28 | D_REG | FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(s29, 29 | D_REG | FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(s30, 30 | D_REG | FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(s31, 31 | D_REG | FPR | Arch_aarch64, "aarch64");

  // 16 bit FP regs
  DEF_REGISTER(h0, 0 | W_REG | FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(h1, 1 | W_REG | FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(h2, 2 | W_REG | FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(h3, 3 | W_REG | FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(h4, 4 | W_REG | FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(h5, 5 | W_REG | FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(h6, 6 | W_REG | FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(h7, 7 | W_REG | FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(h8, 8 | W_REG | FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(h9, 9 | W_REG | FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(h10, 10 | W_REG | FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(h11, 11 | W_REG | FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(h12, 12 | W_REG | FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(h13, 13 | W_REG | FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(h14, 14 | W_REG | FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(h15, 15 | W_REG | FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(h16, 16 | W_REG | FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(h17, 17 | W_REG | FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(h18, 18 | W_REG | FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(h19, 19 | W_REG | FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(h20, 20 | W_REG | FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(h21, 21 | W_REG | FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(h22, 22 | W_REG | FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(h23, 23 | W_REG | FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(h24, 24 | W_REG | FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(h25, 25 | W_REG | FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(h26, 26 | W_REG | FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(h27, 27 | W_REG | FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(h28, 28 | W_REG | FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(h29, 29 | W_REG | FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(h30, 30 | W_REG | FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(h31, 31 | W_REG | FPR | Arch_aarch64, "aarch64");

  // 8 bit FP regs
  DEF_REGISTER(b0, 0 | B_REG | FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(b1, 1 | B_REG | FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(b2, 2 | B_REG | FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(b3, 3 | B_REG | FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(b4, 4 | B_REG | FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(b5, 5 | B_REG | FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(b6, 6 | B_REG | FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(b7, 7 | B_REG | FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(b8, 8 | B_REG | FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(b9, 9 | B_REG | FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(b10, 10 | B_REG | FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(b11, 11 | B_REG | FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(b12, 12 | B_REG | FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(b13, 13 | B_REG | FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(b14, 14 | B_REG | FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(b15, 15 | B_REG | FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(b16, 16 | B_REG | FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(b17, 17 | B_REG | FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(b18, 18 | B_REG | FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(b19, 19 | B_REG | FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(b20, 20 | B_REG | FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(b21, 21 | B_REG | FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(b22, 22 | B_REG | FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(b23, 23 | B_REG | FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(b24, 24 | B_REG | FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(b25, 25 | B_REG | FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(b26, 26 | B_REG | FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(b27, 27 | B_REG | FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(b28, 28 | B_REG | FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(b29, 29 | B_REG | FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(b30, 30 | B_REG | FPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(b31, 31 | B_REG | FPR | Arch_aarch64, "aarch64");

#include "aarch64_sys_regs.h"

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

  DEF_REGISTER(sp, 31 | FULL | SPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(wsp, 0 | D_REG | SPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(pc, 32 | FULL | SPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(pstate, 2 | D_REG | SPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(xzr, 3 | FULL | SPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(n, N_FLAG | BIT | FLAG | Arch_aarch64, "aarch64");
  DEF_REGISTER(z, Z_FLAG | BIT | FLAG | Arch_aarch64, "aarch64");
  DEF_REGISTER(c, C_FLAG | BIT | FLAG | Arch_aarch64, "aarch64");
  DEF_REGISTER(v, V_FLAG | BIT | FLAG | Arch_aarch64, "aarch64");
  DEF_REGISTER(wzr, 3 | D_REG | SPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(fpcr, 4 | D_REG | SPR | Arch_aarch64, "aarch64");
  DEF_REGISTER(fpsr, 5 | D_REG | SPR | Arch_aarch64, "aarch64");

} // end of aarch64 namespace

}

#endif
