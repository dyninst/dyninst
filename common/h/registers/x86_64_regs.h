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

  //          (      name,     ID | alias |   cat |        arch)
  // Single source of truth for this arch's registers: (name, encoding).
  // Expanded to the constexpr constants AND all_regs[] -- no duplication.
  #define DYNINST_X86_64_REG_LIST(X) \
      X(rax, BASEA |  FULL |   GPR | Arch_x86_64) \
      X(eax, BASEA | D_REG |   GPR | Arch_x86_64) \
      X(ax, BASEA | W_REG |   GPR | Arch_x86_64) \
      X(ah, BASEA | H_REG |   GPR | Arch_x86_64) \
      X(al, BASEA | L_REG |   GPR | Arch_x86_64) \
      X(rcx, BASEC |  FULL |   GPR | Arch_x86_64) \
      X(ecx, BASEC | D_REG |   GPR | Arch_x86_64) \
      X(cx, BASEC | W_REG |   GPR | Arch_x86_64) \
      X(ch, BASEC | H_REG |   GPR | Arch_x86_64) \
      X(cl, BASEC | L_REG |   GPR | Arch_x86_64) \
      X(rdx, BASED |  FULL |   GPR | Arch_x86_64) \
      X(edx, BASED | D_REG |   GPR | Arch_x86_64) \
      X(dx, BASED | W_REG |   GPR | Arch_x86_64) \
      X(dh, BASED | H_REG |   GPR | Arch_x86_64) \
      X(dl, BASED | L_REG |   GPR | Arch_x86_64) \
      X(rbx, BASEB |  FULL |   GPR | Arch_x86_64) \
      X(ebx, BASEB | D_REG |   GPR | Arch_x86_64) \
      X(bx, BASEB | W_REG |   GPR | Arch_x86_64) \
      X(bh, BASEB | H_REG |   GPR | Arch_x86_64) \
      X(bl, BASEB | L_REG |   GPR | Arch_x86_64) \
      X(rsp, BASESP |  FULL |   GPR | Arch_x86_64) \
      X(esp, BASESP | D_REG |   GPR | Arch_x86_64) \
      X(sp, BASESP | W_REG |   GPR | Arch_x86_64) \
      X(spl, BASESP | L_REG |   GPR | Arch_x86_64) \
      X(rbp, BASEBP |  FULL |   GPR | Arch_x86_64) \
      X(ebp, BASEBP | D_REG |   GPR | Arch_x86_64) \
      X(bp, BASEBP | W_REG |   GPR | Arch_x86_64) \
      X(bpl, BASEBP | L_REG |   GPR | Arch_x86_64) \
      X(rsi, BASESI |  FULL |   GPR | Arch_x86_64) \
      X(esi, BASESI | D_REG |   GPR | Arch_x86_64) \
      X(si, BASESI | W_REG |   GPR | Arch_x86_64) \
      X(sil, BASESI | L_REG |   GPR | Arch_x86_64) \
      X(rdi, BASEDI |  FULL |   GPR | Arch_x86_64) \
      X(edi, BASEDI | D_REG |   GPR | Arch_x86_64) \
      X(di, BASEDI | W_REG |   GPR | Arch_x86_64) \
      X(dil, BASEDI | L_REG |   GPR | Arch_x86_64) \
      X(r8, BASE8 |  FULL |   GPR | Arch_x86_64) \
      X(r8b, BASE8 | L_REG |   GPR | Arch_x86_64) \
      X(r8w, BASE8 | W_REG |   GPR | Arch_x86_64) \
      X(r8d, BASE8 | D_REG |   GPR | Arch_x86_64) \
      X(r9, BASE9 |  FULL |   GPR | Arch_x86_64) \
      X(r9b, BASE9 | L_REG |   GPR | Arch_x86_64) \
      X(r9w, BASE9 | W_REG |   GPR | Arch_x86_64) \
      X(r9d, BASE9 | D_REG |   GPR | Arch_x86_64) \
      X(r10, BASE10 |  FULL |   GPR | Arch_x86_64) \
      X(r10b, BASE10 | L_REG |   GPR | Arch_x86_64) \
      X(r10w, BASE10 | W_REG |   GPR | Arch_x86_64) \
      X(r10d, BASE10 | D_REG |   GPR | Arch_x86_64) \
      X(r11, BASE11 |  FULL |   GPR | Arch_x86_64) \
      X(r11b, BASE11 | L_REG |   GPR | Arch_x86_64) \
      X(r11w, BASE11 | W_REG |   GPR | Arch_x86_64) \
      X(r11d, BASE11 | D_REG |   GPR | Arch_x86_64) \
      X(r12, BASE12 |  FULL |   GPR | Arch_x86_64) \
      X(r12b, BASE12 | L_REG |   GPR | Arch_x86_64) \
      X(r12w, BASE12 | W_REG |   GPR | Arch_x86_64) \
      X(r12d, BASE12 | D_REG |   GPR | Arch_x86_64) \
      X(r13, BASE13 |  FULL |   GPR | Arch_x86_64) \
      X(r13b, BASE13 | L_REG |   GPR | Arch_x86_64) \
      X(r13w, BASE13 | W_REG |   GPR | Arch_x86_64) \
      X(r13d, BASE13 | D_REG |   GPR | Arch_x86_64) \
      X(r14, BASE14 |  FULL |   GPR | Arch_x86_64) \
      X(r14b, BASE14 | L_REG |   GPR | Arch_x86_64) \
      X(r14w, BASE14 | W_REG |   GPR | Arch_x86_64) \
      X(r14d, BASE14 | D_REG |   GPR | Arch_x86_64) \
      X(r15, BASE15 |  FULL |   GPR | Arch_x86_64) \
      X(r15b, BASE15 | L_REG |   GPR | Arch_x86_64) \
      X(r15w, BASE15 | W_REG |   GPR | Arch_x86_64) \
      X(r15d, BASE15 | D_REG |   GPR | Arch_x86_64) \
      X(rip, 0x10 |  FULL |         Arch_x86_64) \
      X(eip, 0x10 | D_REG |         Arch_x86_64) \
      X(flags, FLAGS |  FULL |  FLAG | Arch_x86_64) \
      X(cf, CF |   BIT |  FLAG | Arch_x86_64) \
      X(flag1, FLAG1 |   BIT |  FLAG | Arch_x86_64) \
      X(pf, PF |   BIT |  FLAG | Arch_x86_64) \
      X(flag3, FLAG3 |   BIT |  FLAG | Arch_x86_64) \
      X(af, AF |   BIT |  FLAG | Arch_x86_64) \
      X(flag5, FLAG5 |   BIT |  FLAG | Arch_x86_64) \
      X(zf, ZF |   BIT |  FLAG | Arch_x86_64) \
      X(sf, SF |   BIT |  FLAG | Arch_x86_64) \
      X(tf, TF |   BIT |  FLAG | Arch_x86_64) \
      X(if_, IF |   BIT |  FLAG | Arch_x86_64) \
      X(df, DF |   BIT |  FLAG | Arch_x86_64) \
      X(of, OF |   BIT |  FLAG | Arch_x86_64) \
      X(flagc, FLAGC |   BIT |  FLAG | Arch_x86_64) \
      X(flagd, FLAGD |   BIT |  FLAG | Arch_x86_64) \
      X(nt_, NT |   BIT |  FLAG | Arch_x86_64) \
      X(flagf, FLAGF |   BIT |  FLAG | Arch_x86_64) \
      X(rf, RF |   BIT |  FLAG | Arch_x86_64) \
      X(vm, VM |   BIT |  FLAG | Arch_x86_64) \
      X(ac, AC |   BIT |  FLAG | Arch_x86_64) \
      X(vif, VIF |   BIT |  FLAG | Arch_x86_64) \
      X(vip, VIP |   BIT |  FLAG | Arch_x86_64) \
      X(id, ID |   BIT |  FLAG | Arch_x86_64) \
      X(ds, BASEDS |  FULL |   SEG | Arch_x86_64) \
      X(es, BASEES |  FULL |   SEG | Arch_x86_64) \
      X(fs, BASEFS |  FULL |   SEG | Arch_x86_64) \
      X(gs, BASEGS |  FULL |   SEG | Arch_x86_64) \
      X(cs, BASECS |  FULL |   SEG | Arch_x86_64) \
      X(ss, BASESS |  FULL |   SEG | Arch_x86_64) \
      X(gdtr, BASEGD | W_REG |   SEG | Arch_x86_64) \
      X(ldtr, BASELD | W_REG |   SEG | Arch_x86_64) \
      X(idtr, BASEID | W_REG |   SEG | Arch_x86_64) \
      X(tr, BASETR | W_REG |   SEG | Arch_x86_64) \
      X(cr0, 0x0 |  FULL |   CTL | Arch_x86_64) \
      X(cr1, 0x1 |  FULL |   CTL | Arch_x86_64) \
      X(cr2, 0x2 |  FULL |   CTL | Arch_x86_64) \
      X(cr3, 0x3 |  FULL |   CTL | Arch_x86_64) \
      X(cr4, 0x4 |  FULL |   CTL | Arch_x86_64) \
      X(cr5, 0x5 |  FULL |   CTL | Arch_x86_64) \
      X(cr6, 0x6 |  FULL |   CTL | Arch_x86_64) \
      X(cr7, 0x7 |  FULL |   CTL | Arch_x86_64) \
      X(cr8, 0x8 |  FULL |   CTL | Arch_x86_64) \
      X(cr9, 0x9 |  FULL |   CTL | Arch_x86_64) \
      X(cr10, 0xA |  FULL |   CTL | Arch_x86_64) \
      X(cr11, 0xB |  FULL |   CTL | Arch_x86_64) \
      X(cr12, 0xC |  FULL |   CTL | Arch_x86_64) \
      X(cr13, 0xD |  FULL |   CTL | Arch_x86_64) \
      X(cr14, 0xE |  FULL |   CTL | Arch_x86_64) \
      X(cr15, 0xF |  FULL |   CTL | Arch_x86_64) \
      X(dr0, 0x0 |  FULL |   DBG | Arch_x86_64) \
      X(dr1, 0x1 |  FULL |   DBG | Arch_x86_64) \
      X(dr2, 0x2 |  FULL |   DBG | Arch_x86_64) \
      X(dr3, 0x3 |  FULL |   DBG | Arch_x86_64) \
      X(dr4, 0x4 |  FULL |   DBG | Arch_x86_64) \
      X(dr5, 0x5 |  FULL |   DBG | Arch_x86_64) \
      X(dr6, 0x6 |  FULL |   DBG | Arch_x86_64) \
      X(dr7, 0x7 |  FULL |   DBG | Arch_x86_64) \
      X(dr8, 0x8 |  FULL |   DBG | Arch_x86_64) \
      X(dr9, 0x9 |  FULL |   DBG | Arch_x86_64) \
      X(dr10, 0xA |  FULL |   DBG | Arch_x86_64) \
      X(dr11, 0xB |  FULL |   DBG | Arch_x86_64) \
      X(dr12, 0xC |  FULL |   DBG | Arch_x86_64) \
      X(dr13, 0xD |  FULL |   DBG | Arch_x86_64) \
      X(dr14, 0xE |  FULL |   DBG | Arch_x86_64) \
      X(dr15, 0xF |  FULL |   DBG | Arch_x86_64) \
      X(st0, 0x0 | FPDBL |   X87 | Arch_x86_64) \
      X(st1, 0x1 | FPDBL |   X87 | Arch_x86_64) \
      X(st2, 0x2 | FPDBL |   X87 | Arch_x86_64) \
      X(st3, 0x3 | FPDBL |   X87 | Arch_x86_64) \
      X(st4, 0x4 | FPDBL |   X87 | Arch_x86_64) \
      X(st5, 0x5 | FPDBL |   X87 | Arch_x86_64) \
      X(st6, 0x6 | FPDBL |   X87 | Arch_x86_64) \
      X(st7, 0x7 | FPDBL |   X87 | Arch_x86_64) \
      X(fcw, 0x8 | W_REG | FPCTL | Arch_x86_64) \
      X(fsw, 0x9 | W_REG | FPCTL | Arch_x86_64) \
      X(mm0, 0x0 |   MMS |   MMX | Arch_x86_64) \
      X(mm1, 0x1 |   MMS |   MMX | Arch_x86_64) \
      X(mm2, 0x2 |   MMS |   MMX | Arch_x86_64) \
      X(mm3, 0x3 |   MMS |   MMX | Arch_x86_64) \
      X(mm4, 0x4 |   MMS |   MMX | Arch_x86_64) \
      X(mm5, 0x5 |   MMS |   MMX | Arch_x86_64) \
      X(mm6, 0x6 |   MMS |   MMX | Arch_x86_64) \
      X(mm7, 0x7 |   MMS |   MMX | Arch_x86_64) \
      X(xmm0, 0x00 |  XMMS |   XMM | Arch_x86_64) \
      X(xmm1, 0x01 |  XMMS |   XMM | Arch_x86_64) \
      X(xmm2, 0x02 |  XMMS |   XMM | Arch_x86_64) \
      X(xmm3, 0x03 |  XMMS |   XMM | Arch_x86_64) \
      X(xmm4, 0x04 |  XMMS |   XMM | Arch_x86_64) \
      X(xmm5, 0x05 |  XMMS |   XMM | Arch_x86_64) \
      X(xmm6, 0x06 |  XMMS |   XMM | Arch_x86_64) \
      X(xmm7, 0x07 |  XMMS |   XMM | Arch_x86_64) \
      X(xmm8, 0x08 |  XMMS |   XMM | Arch_x86_64) \
      X(xmm9, 0x09 |  XMMS |   XMM | Arch_x86_64) \
      X(xmm10, 0x0A |  XMMS |   XMM | Arch_x86_64) \
      X(xmm11, 0x0B |  XMMS |   XMM | Arch_x86_64) \
      X(xmm12, 0x0C |  XMMS |   XMM | Arch_x86_64) \
      X(xmm13, 0x0D |  XMMS |   XMM | Arch_x86_64) \
      X(xmm14, 0x0E |  XMMS |   XMM | Arch_x86_64) \
      X(xmm15, 0x0F |  XMMS |   XMM | Arch_x86_64) \
      X(xmm16, 0x10 |  XMMS |   XMM | Arch_x86_64) \
      X(xmm17, 0x11 |  XMMS |   XMM | Arch_x86_64) \
      X(xmm18, 0x12 |  XMMS |   XMM | Arch_x86_64) \
      X(xmm19, 0x13 |  XMMS |   XMM | Arch_x86_64) \
      X(xmm20, 0x14 |  XMMS |   XMM | Arch_x86_64) \
      X(xmm21, 0x15 |  XMMS |   XMM | Arch_x86_64) \
      X(xmm22, 0x16 |  XMMS |   XMM | Arch_x86_64) \
      X(xmm23, 0x17 |  XMMS |   XMM | Arch_x86_64) \
      X(xmm24, 0x18 |  XMMS |   XMM | Arch_x86_64) \
      X(xmm25, 0x19 |  XMMS |   XMM | Arch_x86_64) \
      X(xmm26, 0x1A |  XMMS |   XMM | Arch_x86_64) \
      X(xmm27, 0x1B |  XMMS |   XMM | Arch_x86_64) \
      X(xmm28, 0x1C |  XMMS |   XMM | Arch_x86_64) \
      X(xmm29, 0x1D |  XMMS |   XMM | Arch_x86_64) \
      X(xmm30, 0x1E |  XMMS |   XMM | Arch_x86_64) \
      X(xmm31, 0x1F |  XMMS |   XMM | Arch_x86_64) \
      X(mxcsr, 0x20 | D_REG | FPCTL | Arch_x86_64) \
      X(ymm0, 0x00 |  YMMS |   YMM | Arch_x86_64) \
      X(ymm1, 0x01 |  YMMS |   YMM | Arch_x86_64) \
      X(ymm2, 0x02 |  YMMS |   YMM | Arch_x86_64) \
      X(ymm3, 0x03 |  YMMS |   YMM | Arch_x86_64) \
      X(ymm4, 0x04 |  YMMS |   YMM | Arch_x86_64) \
      X(ymm5, 0x05 |  YMMS |   YMM | Arch_x86_64) \
      X(ymm6, 0x06 |  YMMS |   YMM | Arch_x86_64) \
      X(ymm7, 0x07 |  YMMS |   YMM | Arch_x86_64) \
      X(ymm8, 0x08 |  YMMS |   YMM | Arch_x86_64) \
      X(ymm9, 0x09 |  YMMS |   YMM | Arch_x86_64) \
      X(ymm10, 0x0A |  YMMS |   YMM | Arch_x86_64) \
      X(ymm11, 0x0B |  YMMS |   YMM | Arch_x86_64) \
      X(ymm12, 0x0C |  YMMS |   YMM | Arch_x86_64) \
      X(ymm13, 0x0D |  YMMS |   YMM | Arch_x86_64) \
      X(ymm14, 0x0E |  YMMS |   YMM | Arch_x86_64) \
      X(ymm15, 0x0F |  YMMS |   YMM | Arch_x86_64) \
      X(ymm16, 0x10 |  YMMS |   YMM | Arch_x86_64) \
      X(ymm17, 0x11 |  YMMS |   YMM | Arch_x86_64) \
      X(ymm18, 0x12 |  YMMS |   YMM | Arch_x86_64) \
      X(ymm19, 0x13 |  YMMS |   YMM | Arch_x86_64) \
      X(ymm20, 0x14 |  YMMS |   YMM | Arch_x86_64) \
      X(ymm21, 0x15 |  YMMS |   YMM | Arch_x86_64) \
      X(ymm22, 0x16 |  YMMS |   YMM | Arch_x86_64) \
      X(ymm23, 0x17 |  YMMS |   YMM | Arch_x86_64) \
      X(ymm24, 0x18 |  YMMS |   YMM | Arch_x86_64) \
      X(ymm25, 0x19 |  YMMS |   YMM | Arch_x86_64) \
      X(ymm26, 0x1A |  YMMS |   YMM | Arch_x86_64) \
      X(ymm27, 0x1B |  YMMS |   YMM | Arch_x86_64) \
      X(ymm28, 0x1C |  YMMS |   YMM | Arch_x86_64) \
      X(ymm29, 0x1D |  YMMS |   YMM | Arch_x86_64) \
      X(ymm30, 0x1E |  YMMS |   YMM | Arch_x86_64) \
      X(ymm31, 0x1F |  YMMS |   YMM | Arch_x86_64) \
      X(zmm0, 0x00 |  ZMMS |   ZMM | Arch_x86_64) \
      X(zmm1, 0x01 |  ZMMS |   ZMM | Arch_x86_64) \
      X(zmm2, 0x02 |  ZMMS |   ZMM | Arch_x86_64) \
      X(zmm3, 0x03 |  ZMMS |   ZMM | Arch_x86_64) \
      X(zmm4, 0x04 |  ZMMS |   ZMM | Arch_x86_64) \
      X(zmm5, 0x05 |  ZMMS |   ZMM | Arch_x86_64) \
      X(zmm6, 0x06 |  ZMMS |   ZMM | Arch_x86_64) \
      X(zmm7, 0x07 |  ZMMS |   ZMM | Arch_x86_64) \
      X(zmm8, 0x08 |  ZMMS |   ZMM | Arch_x86_64) \
      X(zmm9, 0x09 |  ZMMS |   ZMM | Arch_x86_64) \
      X(zmm10, 0x0A |  ZMMS |   ZMM | Arch_x86_64) \
      X(zmm11, 0x0B |  ZMMS |   ZMM | Arch_x86_64) \
      X(zmm12, 0x0C |  ZMMS |   ZMM | Arch_x86_64) \
      X(zmm13, 0x0D |  ZMMS |   ZMM | Arch_x86_64) \
      X(zmm14, 0x0E |  ZMMS |   ZMM | Arch_x86_64) \
      X(zmm15, 0x0F |  ZMMS |   ZMM | Arch_x86_64) \
      X(zmm16, 0x10 |  ZMMS |   ZMM | Arch_x86_64) \
      X(zmm17, 0x11 |  ZMMS |   ZMM | Arch_x86_64) \
      X(zmm18, 0x12 |  ZMMS |   ZMM | Arch_x86_64) \
      X(zmm19, 0x13 |  ZMMS |   ZMM | Arch_x86_64) \
      X(zmm20, 0x14 |  ZMMS |   ZMM | Arch_x86_64) \
      X(zmm21, 0x15 |  ZMMS |   ZMM | Arch_x86_64) \
      X(zmm22, 0x16 |  ZMMS |   ZMM | Arch_x86_64) \
      X(zmm23, 0x17 |  ZMMS |   ZMM | Arch_x86_64) \
      X(zmm24, 0x18 |  ZMMS |   ZMM | Arch_x86_64) \
      X(zmm25, 0x19 |  ZMMS |   ZMM | Arch_x86_64) \
      X(zmm26, 0x1A |  ZMMS |   ZMM | Arch_x86_64) \
      X(zmm27, 0x1B |  ZMMS |   ZMM | Arch_x86_64) \
      X(zmm28, 0x1C |  ZMMS |   ZMM | Arch_x86_64) \
      X(zmm29, 0x1D |  ZMMS |   ZMM | Arch_x86_64) \
      X(zmm30, 0x1E |  ZMMS |   ZMM | Arch_x86_64) \
      X(zmm31, 0x1F |  ZMMS |   ZMM | Arch_x86_64) \
      X(k0, 0x00 | KMSKS | KMASK | Arch_x86_64) \
      X(k1, 0x01 | KMSKS | KMASK | Arch_x86_64) \
      X(k2, 0x02 | KMSKS | KMASK | Arch_x86_64) \
      X(k3, 0x03 | KMSKS | KMASK | Arch_x86_64) \
      X(k4, 0x04 | KMSKS | KMASK | Arch_x86_64) \
      X(k5, 0x05 | KMSKS | KMASK | Arch_x86_64) \
      X(k6, 0x06 | KMSKS | KMASK | Arch_x86_64) \
      X(k7, 0x07 | KMSKS | KMASK | Arch_x86_64) \
      X(orax, 0x0 |  FULL |  MISC | Arch_x86_64) \
      X(fsbase, 0x1 |  FULL |  MISC | Arch_x86_64) \
      X(gsbase, 0x2 |  FULL |  MISC | Arch_x86_64) \
      X(tr0, 0x0 |  FULL |   TST | Arch_x86_64) \
      X(tr1, 0x1 |  FULL |   TST | Arch_x86_64) \
      X(tr2, 0x2 |  FULL |   TST | Arch_x86_64) \
      X(tr3, 0x3 |  FULL |   TST | Arch_x86_64) \
      X(tr4, 0x4 |  FULL |   TST | Arch_x86_64) \
      X(tr5, 0x5 |  FULL |   TST | Arch_x86_64) \
      X(tr6, 0x6 |  FULL |   TST | Arch_x86_64) \
      X(tr7, 0x7 |  FULL |   TST | Arch_x86_64)

  #define DEF_ONE(n, v) DEF_REGISTER(n, v);
  DYNINST_X86_64_REG_LIST(DEF_ONE)
  #undef DEF_ONE

  #define NAME_ONE(n, v) n,
  constexpr MachRegister all_regs[] = { DYNINST_X86_64_REG_LIST(NAME_ONE) };
  #undef NAME_ONE


}}

#endif
