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

#ifndef DYNINST_COMMON_ARCH_REGS_X86_H
#define DYNINST_COMMON_ARCH_REGS_X86_H

// The general machine registers.
// These values are taken from the Pentium manual and CANNOT be changed.

// 32-bit
#define REGNUM_EAX 0
#define REGNUM_ECX 1
#define REGNUM_EDX 2
#define REGNUM_EBX 3
#define REGNUM_ESP 4
#define REGNUM_EBP 5
#define REGNUM_ESI 6
#define REGNUM_EDI 7

// 64-bit
enum AMD64_REG_NUMBERS {
  REGNUM_RAX = 0,
  REGNUM_RCX,
  REGNUM_RDX,
  REGNUM_RBX,
  REGNUM_RSP,
  REGNUM_RBP,
  REGNUM_RSI,
  REGNUM_RDI,
  REGNUM_R8,
  REGNUM_R9,
  REGNUM_R10,
  REGNUM_R11,
  REGNUM_R12,
  REGNUM_R13,
  REGNUM_R14,
  REGNUM_R15,
  REGNUM_DUMMYFPR,
  REGNUM_OF,
  REGNUM_SF,
  REGNUM_ZF,
  REGNUM_AF,
  REGNUM_PF,
  REGNUM_CF,
  REGNUM_TF,
  REGNUM_IF,
  REGNUM_DF,
  REGNUM_NT,
  REGNUM_RF,
  REGNUM_MM0,
  REGNUM_MM1,
  REGNUM_MM2,
  REGNUM_MM3,
  REGNUM_MM4,
  REGNUM_MM5,
  REGNUM_MM6,
  REGNUM_MM7,
  REGNUM_K0,
  REGNUM_K1,
  REGNUM_K2,
  REGNUM_K3,
  REGNUM_K4,
  REGNUM_K5,
  REGNUM_K6,
  REGNUM_K7,
  REGNUM_XMM0,
  REGNUM_XMM1,
  REGNUM_XMM2,
  REGNUM_XMM3,
  REGNUM_XMM4,
  REGNUM_XMM5,
  REGNUM_XMM6,
  REGNUM_XMM7,
  REGNUM_XMM8,
  REGNUM_XMM9,
  REGNUM_XMM10,
  REGNUM_XMM11,
  REGNUM_XMM12,
  REGNUM_XMM13,
  REGNUM_XMM14,
  REGNUM_XMM15,
  REGNUM_XMM16,
  REGNUM_XMM17,
  REGNUM_XMM18,
  REGNUM_XMM19,
  REGNUM_XMM20,
  REGNUM_XMM21,
  REGNUM_XMM22,
  REGNUM_XMM23,
  REGNUM_XMM24,
  REGNUM_XMM25,
  REGNUM_XMM26,
  REGNUM_XMM27,
  REGNUM_XMM28,
  REGNUM_XMM29,
  REGNUM_XMM30,
  REGNUM_XMM31,
  REGNUM_YMM0,
  REGNUM_YMM1,
  REGNUM_YMM2,
  REGNUM_YMM3,
  REGNUM_YMM4,
  REGNUM_YMM5,
  REGNUM_YMM6,
  REGNUM_YMM7,
  REGNUM_YMM8,
  REGNUM_YMM9,
  REGNUM_YMM10,
  REGNUM_YMM11,
  REGNUM_YMM12,
  REGNUM_YMM13,
  REGNUM_YMM14,
  REGNUM_YMM15,
  REGNUM_YMM16,
  REGNUM_YMM17,
  REGNUM_YMM18,
  REGNUM_YMM19,
  REGNUM_YMM20,
  REGNUM_YMM21,
  REGNUM_YMM22,
  REGNUM_YMM23,
  REGNUM_YMM24,
  REGNUM_YMM25,
  REGNUM_YMM26,
  REGNUM_YMM27,
  REGNUM_YMM28,
  REGNUM_YMM29,
  REGNUM_YMM30,
  REGNUM_YMM31,
  REGNUM_ZMM0,
  REGNUM_ZMM1,
  REGNUM_ZMM2,
  REGNUM_ZMM3,
  REGNUM_ZMM4,
  REGNUM_ZMM5,
  REGNUM_ZMM6,
  REGNUM_ZMM7,
  REGNUM_ZMM8,
  REGNUM_ZMM9,
  REGNUM_ZMM10,
  REGNUM_ZMM11,
  REGNUM_ZMM12,
  REGNUM_ZMM13,
  REGNUM_ZMM14,
  REGNUM_ZMM15,
  REGNUM_ZMM16,
  REGNUM_ZMM17,
  REGNUM_ZMM18,
  REGNUM_ZMM19,
  REGNUM_ZMM20,
  REGNUM_ZMM21,
  REGNUM_ZMM22,
  REGNUM_ZMM23,
  REGNUM_ZMM24,
  REGNUM_ZMM25,
  REGNUM_ZMM26,
  REGNUM_ZMM27,
  REGNUM_ZMM28,
  REGNUM_ZMM29,
  REGNUM_ZMM30,
  REGNUM_ZMM31,
  REGNUM_EFLAGS,
  REGNUM_FS,
  REGNUM_GS,
  REGNUM_IGNORED
};

#endif
