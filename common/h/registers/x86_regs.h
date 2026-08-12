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

#ifndef DYNINST_X86_REGS_H
#define DYNINST_X86_REGS_H

//clang-format: off

#include "Architecture.h"
#include "registers/reg_def.h"
#include <cstdint>

namespace Dyninst { namespace x86 {

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
   * 
   *   The SSE registers are given the correct size of 64 bits even
   *   though they alias the lower 64 bits of the x87 FPU registers.
   * 
   *   No attempt is made to represent aliased portions of other registers.
   */
  const int32_t FULL   = 0x00000000;  // 32 bits
  const int32_t BIT    = 0x00000100;  // 1-bit EFLAGS
  const int32_t L_REG  = 0x00000200;  // 8-bit, first byte
  const int32_t H_REG  = 0x00000300;  // 8-bit, second byte
  const int32_t W_REG  = 0x00000400;  // 16-bit, first word
  const int32_t FPDBL  = 0x00000500;  // 80-bit x87 FPU
  const int32_t MMS    = 0x00000600;  // 64-bit MMX and 3DNow!
  const int32_t XMMS   = 0x00000700;  // 128-bit SSE, FC16, XOP, AVX, and FMA3/4
  const int32_t YMMS   = 0x00000800;  // 256-bit SSE, AVX2, FMA3/4
  const int32_t ZMMS   = 0x00000900;  // 512-bit AVX-512/AVX10
  const int32_t KMSKS  = 0x00000A00;  // 64-bit mask from AVX-512/AVX10

  /* Register Categories */
  const int32_t GPR    = 0x00010000;  // General-Purpose Registers
  const int32_t SEG    = 0x00020000;  // Segment Registers
  const int32_t FLAG   = 0x00030000;  // EFLAGS Register
  const int32_t MISC   = 0x00040000;  // Internal ProcControlAPI Register
  const int32_t CTL    = 0x00050000;  // Control Registers CR0-CR7
  const int32_t DBG    = 0x00060000;  // Debug Registers DR0-DR7
  const int32_t TST    = 0x00070000;  // Internal InstructionAPI Registers
  const int32_t X87    = 0x00080000;  // x87 FPU Registers
  const int32_t MMX    = 0x00090000;  // MM0-MM7 Registers
  const int32_t XMM    = 0x000A0000;  // XMM0-XMM7 Registers from SSE
  const int32_t YMM    = 0x000B0000;  // YMM0-YMM7 Registers from AVX2/FMA
  const int32_t ZMM    = 0x000C0000;  // ZMM0-ZMM7 Registers from AVX-512
  const int32_t KMASK  = 0x000D0000;  // K0-K7 opmask Registers from AVX-512
  const int32_t FPCTL  = 0x000E0000;  // control/status Registers from x87, SSE, and AVX

  /* Base IDs for aliased GPRs */
  const int32_t BASEA  = 0x0;
  const int32_t BASEC  = 0x1;
  const int32_t BASED  = 0x2;
  const int32_t BASEB  = 0x3;
  const int32_t BASESP = 0x4;
  const int32_t BASEBP = 0x5;
  const int32_t BASESI = 0x6;
  const int32_t BASEDI = 0x7;
  const int32_t FLAGS  = 0x0;

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

  /* Base IDs for each bit in EFLAGS */
  const int32_t CF    = 0x00;  // Carry Flag
  const int32_t FLAG1 = 0x01;  // Reserved
  const int32_t PF    = 0x02;  // Parity Flag
  const int32_t FLAG3 = 0x03;  // Reserved
  const int32_t AF    = 0x04;  // Auxiliary Carry Flag
  const int32_t FLAG5 = 0x05;  // Reserved
  const int32_t ZF    = 0x06;  // Zero Flag
  const int32_t SF    = 0x07;  // Sign Flag
  const int32_t TF    = 0x08;  // Trap Flag
  const int32_t IF    = 0x09;  // Interrupt Enable Flag
  const int32_t DF    = 0x0A;  // Direction Flag
  const int32_t OF    = 0x0B;  // Overflow Flag
  const int32_t FLAGC = 0x0C;  // I/O Privilege Level (bits 12 and 13)
  const int32_t FLAGD = 0x0D;  // I/O Privilege Level (bits 12 and 13)
  const int32_t NT    = 0x0E;  // Nested Task
  const int32_t FLAGF = 0x0F;  // Reserved
  const int32_t RF    = 0x10;  // Resume Flag
  const int32_t VM    = 0x11;  // Virtual-8086 Mode
  const int32_t AC    = 0x12;  // Alignment Check/Access Control
  const int32_t VIF   = 0x13;  // Virtual Interrupt Flag
  const int32_t VIP   = 0x14;  // Virtual Interrupt Pending
  const int32_t ID    = 0x15;  // ID Flag
  /* Flags 22-31 are reserved */
  

  /**
   * Format of constants:
   *  [0x000000ff] Lower 8 bits are base register ID
   *  [0x0000ff00] Next 8 bits are the aliasing and subrange ID used to distinguish
   *               between whole and aliased registers like EAX and AH.
   *  [0x00ff0000] Next 8 bits are the register category, GPR, FLAG, etc.
   *  [0xff000000] Upper 8 bits are the architecture.
   **/
 
   /* General-purpose Registers */
  //          (      name,     ID | alias |   cat |     arch)
  // Single source of truth for this arch's registers: (name, encoding).
  // Expanded to the constexpr constants AND all_regs[] -- no duplication.
  #define DYNINST_X86_REG_LIST(X) \
      X(eax, BASEA |  FULL |   GPR | Arch_x86) \
      X(ax, BASEA | W_REG |   GPR | Arch_x86) \
      X(ah, BASEA | H_REG |   GPR | Arch_x86) \
      X(al, BASEA | L_REG |   GPR | Arch_x86) \
      X(ecx, BASEC |  FULL |   GPR | Arch_x86) \
      X(cx, BASEC | W_REG |   GPR | Arch_x86) \
      X(ch, BASEC | H_REG |   GPR | Arch_x86) \
      X(cl, BASEC | L_REG |   GPR | Arch_x86) \
      X(edx, BASED |  FULL |   GPR | Arch_x86) \
      X(dx, BASED | W_REG |   GPR | Arch_x86) \
      X(dh, BASED | H_REG |   GPR | Arch_x86) \
      X(dl, BASED | L_REG |   GPR | Arch_x86) \
      X(ebx, BASEB |  FULL |   GPR | Arch_x86) \
      X(bx, BASEB | W_REG |   GPR | Arch_x86) \
      X(bh, BASEB | H_REG |   GPR | Arch_x86) \
      X(bl, BASEB | L_REG |   GPR | Arch_x86) \
      X(esp, BASESP |  FULL |   GPR | Arch_x86) \
      X(sp, BASESP | W_REG |   GPR | Arch_x86) \
      X(ebp, BASEBP |  FULL |   GPR | Arch_x86) \
      X(bp, BASEBP | W_REG |   GPR | Arch_x86) \
      X(esi, BASESI |  FULL |   GPR | Arch_x86) \
      X(si, BASESI | W_REG |   GPR | Arch_x86) \
      X(edi, BASEDI |  FULL |   GPR | Arch_x86) \
      X(di, BASEDI | W_REG |   GPR | Arch_x86) \
      X(eip, 0x10 |  FULL |         Arch_x86) \
      X(cs, BASECS | W_REG |   SEG | Arch_x86) \
      X(ds, BASEDS | W_REG |   SEG | Arch_x86) \
      X(es, BASEES | W_REG |   SEG | Arch_x86) \
      X(ss, BASESS | W_REG |   SEG | Arch_x86) \
      X(fs, BASEFS | W_REG |   SEG | Arch_x86) \
      X(gs, BASEGS | W_REG |   SEG | Arch_x86) \
      X(gdtr, BASEGD | W_REG |   SEG | Arch_x86) \
      X(ldtr, BASELD | W_REG |   SEG | Arch_x86) \
      X(idtr, BASEID | W_REG |   SEG | Arch_x86) \
      X(tr, BASETR | W_REG |   SEG | Arch_x86) \
      X(flags, FLAGS |  FULL |  FLAG | Arch_x86) \
      X(cf, CF |   BIT |  FLAG | Arch_x86) \
      X(flag1, FLAG1 |   BIT |  FLAG | Arch_x86) \
      X(pf, PF |   BIT |  FLAG | Arch_x86) \
      X(flag3, FLAG3 |   BIT |  FLAG | Arch_x86) \
      X(af, AF |   BIT |  FLAG | Arch_x86) \
      X(flag5, FLAG5 |   BIT |  FLAG | Arch_x86) \
      X(zf, ZF |   BIT |  FLAG | Arch_x86) \
      X(sf, SF |   BIT |  FLAG | Arch_x86) \
      X(tf, TF |   BIT |  FLAG | Arch_x86) \
      X(if_, IF |   BIT |  FLAG | Arch_x86) \
      X(df, DF |   BIT |  FLAG | Arch_x86) \
      X(of, OF |   BIT |  FLAG | Arch_x86) \
      X(flagc, FLAGC |   BIT |  FLAG | Arch_x86) \
      X(flagd, FLAGD |   BIT |  FLAG | Arch_x86) \
      X(nt_, NT |   BIT |  FLAG | Arch_x86) \
      X(flagf, FLAGF |   BIT |  FLAG | Arch_x86) \
      X(rf, RF |   BIT |  FLAG | Arch_x86) \
      X(vm, VM |   BIT |  FLAG | Arch_x86) \
      X(ac, AC |   BIT |  FLAG | Arch_x86) \
      X(vif, VIF |   BIT |  FLAG | Arch_x86) \
      X(vip, VIP |   BIT |  FLAG | Arch_x86) \
      X(id, ID |   BIT |  FLAG | Arch_x86) \
      X(cr0, 0x0 |  FULL |   CTL | Arch_x86) \
      X(cr1, 0x1 |  FULL |   CTL | Arch_x86) \
      X(cr2, 0x2 |  FULL |   CTL | Arch_x86) \
      X(cr3, 0x3 |  FULL |   CTL | Arch_x86) \
      X(cr4, 0x4 |  FULL |   CTL | Arch_x86) \
      X(cr5, 0x5 |  FULL |   CTL | Arch_x86) \
      X(cr6, 0x6 |  FULL |   CTL | Arch_x86) \
      X(cr7, 0x7 |  FULL |   CTL | Arch_x86) \
      X(dr0, 0x0 |  FULL |   DBG | Arch_x86) \
      X(dr1, 0x1 |  FULL |   DBG | Arch_x86) \
      X(dr2, 0x2 |  FULL |   DBG | Arch_x86) \
      X(dr3, 0x3 |  FULL |   DBG | Arch_x86) \
      X(dr4, 0x4 |  FULL |   DBG | Arch_x86) \
      X(dr5, 0x5 |  FULL |   DBG | Arch_x86) \
      X(dr6, 0x6 |  FULL |   DBG | Arch_x86) \
      X(dr7, 0x7 |  FULL |   DBG | Arch_x86) \
      X(st0, 0x0 | FPDBL |   X87 | Arch_x86) \
      X(st1, 0x1 | FPDBL |   X87 | Arch_x86) \
      X(st2, 0x2 | FPDBL |   X87 | Arch_x86) \
      X(st3, 0x3 | FPDBL |   X87 | Arch_x86) \
      X(st4, 0x4 | FPDBL |   X87 | Arch_x86) \
      X(st5, 0x5 | FPDBL |   X87 | Arch_x86) \
      X(st6, 0x6 | FPDBL |   X87 | Arch_x86) \
      X(st7, 0x7 | FPDBL |   X87 | Arch_x86) \
      X(fcw, 0x8 | W_REG | FPCTL | Arch_x86) \
      X(fsw, 0x9 | W_REG | FPCTL | Arch_x86) \
      X(mm0, 0x0 |   MMS |   MMX | Arch_x86) \
      X(mm1, 0x1 |   MMS |   MMX | Arch_x86) \
      X(mm2, 0x2 |   MMS |   MMX | Arch_x86) \
      X(mm3, 0x3 |   MMS |   MMX | Arch_x86) \
      X(mm4, 0x4 |   MMS |   MMX | Arch_x86) \
      X(mm5, 0x5 |   MMS |   MMX | Arch_x86) \
      X(mm6, 0x6 |   MMS |   MMX | Arch_x86) \
      X(mm7, 0x7 |   MMS |   MMX | Arch_x86) \
      X(xmm0, 0x00 |  XMMS |   XMM | Arch_x86) \
      X(xmm1, 0x01 |  XMMS |   XMM | Arch_x86) \
      X(xmm2, 0x02 |  XMMS |   XMM | Arch_x86) \
      X(xmm3, 0x03 |  XMMS |   XMM | Arch_x86) \
      X(xmm4, 0x04 |  XMMS |   XMM | Arch_x86) \
      X(xmm5, 0x05 |  XMMS |   XMM | Arch_x86) \
      X(xmm6, 0x06 |  XMMS |   XMM | Arch_x86) \
      X(xmm7, 0x07 |  XMMS |   XMM | Arch_x86) \
      X(mxcsr, 0x08 |  FULL | FPCTL | Arch_x86) \
      X(ymm0, 0x00 |  YMMS |   YMM | Arch_x86) \
      X(ymm1, 0x01 |  YMMS |   YMM | Arch_x86) \
      X(ymm2, 0x02 |  YMMS |   YMM | Arch_x86) \
      X(ymm3, 0x03 |  YMMS |   YMM | Arch_x86) \
      X(ymm4, 0x04 |  YMMS |   YMM | Arch_x86) \
      X(ymm5, 0x05 |  YMMS |   YMM | Arch_x86) \
      X(ymm6, 0x06 |  YMMS |   YMM | Arch_x86) \
      X(ymm7, 0x07 |  YMMS |   YMM | Arch_x86) \
      X(zmm0, 0x00 |  ZMMS |   ZMM | Arch_x86) \
      X(zmm1, 0x01 |  ZMMS |   ZMM | Arch_x86) \
      X(zmm2, 0x02 |  ZMMS |   ZMM | Arch_x86) \
      X(zmm3, 0x03 |  ZMMS |   ZMM | Arch_x86) \
      X(zmm4, 0x04 |  ZMMS |   ZMM | Arch_x86) \
      X(zmm5, 0x05 |  ZMMS |   ZMM | Arch_x86) \
      X(zmm6, 0x06 |  ZMMS |   ZMM | Arch_x86) \
      X(zmm7, 0x07 |  ZMMS |   ZMM | Arch_x86) \
      X(k0, 0x00 | KMSKS | KMASK | Arch_x86) \
      X(k1, 0x01 | KMSKS | KMASK | Arch_x86) \
      X(k2, 0x02 | KMSKS | KMASK | Arch_x86) \
      X(k3, 0x03 | KMSKS | KMASK | Arch_x86) \
      X(k4, 0x04 | KMSKS | KMASK | Arch_x86) \
      X(k5, 0x05 | KMSKS | KMASK | Arch_x86) \
      X(k6, 0x06 | KMSKS | KMASK | Arch_x86) \
      X(k7, 0x07 | KMSKS | KMASK | Arch_x86) \
      X(oeax, 0x0 |  FULL |  MISC | Arch_x86) \
      X(fsbase, 0x1 |  FULL |  MISC | Arch_x86) \
      X(gsbase, 0x2 |  FULL |  MISC | Arch_x86) \
      X(tr0, 0x0 |  FULL |   TST | Arch_x86) \
      X(tr1, 0x1 |  FULL |   TST | Arch_x86) \
      X(tr2, 0x2 |  FULL |   TST | Arch_x86) \
      X(tr3, 0x3 |  FULL |   TST | Arch_x86) \
      X(tr4, 0x4 |  FULL |   TST | Arch_x86) \
      X(tr5, 0x5 |  FULL |   TST | Arch_x86) \
      X(tr6, 0x6 |  FULL |   TST | Arch_x86) \
      X(tr7, 0x7 |  FULL |   TST | Arch_x86)

  #define DEF_ONE(n, v) DEF_REGISTER(n, v);
  DYNINST_X86_REG_LIST(DEF_ONE)
  #undef DEF_ONE

  #define NAME_ONE(n, v) n,
  constexpr MachRegister all_regs[] = { DYNINST_X86_REG_LIST(NAME_ONE) };
  #undef NAME_ONE


}

inline bool isSegmentRegister(int regClass) {
  return 0 != (regClass & x86::SEG);
}

}

#endif
