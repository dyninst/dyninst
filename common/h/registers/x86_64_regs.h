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

#ifndef DYNINST_X86_64_REGS_H
#define DYNINST_X86_64_REGS_H

//clang-format: off

#include "Architecture.h"
#include "registers/reg_def.h"
#include "registers/x86_regs.h"
#include <cstdint>

namespace Dyninst { namespace x86_64 {

  /* Register lengths
   *
   * NOTE:
   *
   *   MachRegister::getBaseRegister clears the bit field for size, so
   *   the full register size has to be represented as 0x0.
   *
   *   The {L,H,W}_REG sizes represent the aliased portions of the GPR
   *   registers that are historically referred to by name (e.g., AL is
   *   the lower 8 bits of EAX).
   */
  const int32_t FULL   = 0x00000000; // 64 bits
  const int32_t BIT    = 0x00000100; // 1-bit EFLAGS
  const int32_t L_REG  = 0x00000200; // 8-bit, first byte
  const int32_t H_REG  = 0x00000300; // 8-bit, second byte
  const int32_t W_REG  = 0x00000400; // 16-bit, first word
  const int32_t D_REG  = 0x00000500; // 32 bit, first double word
  const int32_t FPDBL  = 0x00000600; // 80-bit x87 FPU
  const int32_t MMS    = 0x00000700; // 64-bit MMX and 3DNow!
  const int32_t XMMS   = 0x00000800; // 128-bit SSE, FC16, XOP, AVX, and FMA3/4
  const int32_t YMMS   = 0x00000900; // 256-bit SSE, AVX2, FMA3/4
  const int32_t ZMMS   = 0x00000A00; // 512-bit AVX-512/AVX10
  const int32_t KMSKS  = 0x00000B00; // 64-bit mask from AVX-512/AVX10

  /* Register Categories */
  const int32_t GPR    = 0x00010000;  // General-Purpose Registers
  const int32_t SEG    = 0x00020000;  // Segment Registers
  const int32_t FLAG   = 0x00030000;  // RFLAGS Register
  const int32_t MISC   = 0x00040000;  // Internal ProcControlAPI Register
  const int32_t CTL    = 0x00050000;  // Control Registers CR0-CR7
  const int32_t DBG    = 0x00060000;  // Debug Registers DR0-DR7
  const int32_t TST    = 0x00070000;  // Internal InstructionAPI Registers
  const int32_t X87    = 0x00080000;  // x87 FPU Registers
  const int32_t MMX    = 0x00090000;  // MM0-MM7 Registers
  const int32_t XMM    = 0x000A0000;  // XMM0-XMM15 Registers from SSE (XMM0-XMM31 for AVX-512)
  const int32_t YMM    = 0x000B0000;  // YMM0-YMM15 Registers from AVX2/FMA (YMM0-YMM31 for AVX-512)
  const int32_t ZMM    = 0x000C0000;  // ZMM0-ZMM31 Registers from AVX-512
  const int32_t KMASK  = 0x000D0000;  // K0-K7 opmask Registers from AVX-512
  const int32_t FPCTL  = 0x000E0000;  // control/status Registers from x87, SSE, and AVX

  /* Base IDs for aliased GPRs */
  const int32_t FLAGS  = 0x00;  // RFLAGS Register
  const int32_t BASEA  = 0x00;
  const int32_t BASEC  = 0x01;
  const int32_t BASED  = 0x02;
  const int32_t BASEB  = 0x03;
  const int32_t BASESP = 0x04;
  const int32_t BASEBP = 0x05;
  const int32_t BASESI = 0x06;
  const int32_t BASEDI = 0x07;
  const int32_t BASE8  = 0x08;
  const int32_t BASE9  = 0x09;
  const int32_t BASE10 = 0x0A;
  const int32_t BASE11 = 0x0B;
  const int32_t BASE12 = 0x0C;
  const int32_t BASE13 = 0x0D;
  const int32_t BASE14 = 0x0E;
  const int32_t BASE15 = 0x0F;

  /* Base IDs for memory segment registers */
  const int32_t BASEDS = 0x0; // Data Segment register
  const int32_t BASESS = 0x1; // Stack Segment register
  const int32_t BASEFS = 0x2; // F Segment register
  const int32_t BASEGS = 0x3; // G Segment register
  const int32_t BASECS = 0x4; // Code Segment register
  const int32_t BASEES = 0x5; // Extended data Segment register
  const int32_t BASEGD = 0x6; // Global Descriptor Table
  const int32_t BASELD = 0x7; // Local Descriptor Table
  const int32_t BASEID = 0X8; // Interrupt Descriptor Table
  const int32_t BASETR = 0x9; // Task Register

  /* Base IDs for each bit in RFLAGS */
  const int32_t CF    = x86::CF;    // Carry Flag
  const int32_t FLAG1 = x86::FLAG1; // Reserved
  const int32_t PF    = x86::PF;    // Parity Flag
  const int32_t FLAG3 = x86::FLAG3; // Reserved
  const int32_t AF    = x86::AF;    // Auxiliary Carry Flag
  const int32_t FLAG5 = x86::FLAG5; // Reserved
  const int32_t ZF    = x86::ZF;    // Zero Flag
  const int32_t SF    = x86::SF;    // Sign Flag
  const int32_t TF    = x86::TF;    // Trap Flag
  const int32_t IF    = x86::IF;    // Interrupt Enable Flag
  const int32_t DF    = x86::DF;    // Direction Flag
  const int32_t OF    = x86::OF;    // Overflow Flag
  const int32_t FLAGC = x86::FLAGC; // I/O Privilege Level (bits 12 and 13)
  const int32_t FLAGD = x86::FLAGD; // I/O Privilege Level (bits 12 and 13)
  const int32_t NT    = x86::NT;    // Nested Task
  const int32_t FLAGF = x86::FLAGF; // Reserved
  const int32_t RF    = x86::RF;    // Resume Flag
  const int32_t VM    = x86::VM;    // Virtual-8086 Mode
  const int32_t AC    = x86::AC;    // Alignment Check/Access Control
  const int32_t VIF   = x86::VIF;   // Virtual Interrupt Flag
  const int32_t VIP   = x86::VIP;   // Virtual Interrupt Pending
  const int32_t ID    = x86::ID;    // ID Flag
  /* Flags 22-63 are reserved */

  /**
   * Format of constants:
   *  [0x000000ff] Lower 8 bits are base register ID
   *  [0x0000ff00] Next 8 bits are the aliasing and subrange ID used to distinguish
   *               between whole and aliased registers like EAX and AH.
   *  [0x00ff0000] Next 8 bits are the register category, GPR, FLAG, etc.
   *  [0xff000000] Upper 8 bits are the architecture.
   **/
#include "registers/x86_64_regs.def"

}}

#endif
