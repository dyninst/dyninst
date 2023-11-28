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
  const signed int FULL   = 0x00000000;  // 32 bits
  const signed int BIT    = 0x00000100;  // 1-bit EFLAGS
  const signed int L_REG  = 0x00000200;  // 8-bit, first byte
  const signed int H_REG  = 0x00000300;  // 8-bit, second byte
  const signed int W_REG  = 0x00000400;  // 16-bit, first word
  const signed int FPDBL  = 0x00000500;  // 80-bit x87 FPU
  const signed int MMS    = 0x00000600;  // 64-bit MMX and 3DNow!
  const signed int XMMS   = 0x00000700;  // 128-bit SSE, FC16, XOP, AVX, and FMA3/4
  const signed int YMMS   = 0x00000800;  // 256-bit SSE, AVX2, FMA3/4
  const signed int ZMMS   = 0x00000900;  // 512-bit AVX-512/AVX10
  const signed int KMSKS  = 0x00000A00;  // 64-bit mask from AVX-512/AVX10

  /* Register Categories */
  const signed int GPR    = 0x00010000;  // General-Purpose Registers
  const signed int SEG    = 0x00020000;  // Segment Registers
  const signed int FLAG   = 0x00030000;  // EFLAGS Register
  const signed int MISC   = 0x00040000;  // Internal ProcControlAPI Register
  const signed int CTL    = 0x00050000;  // Control Registers CR0-CR7
  const signed int DBG    = 0x00060000;  // Debug Registers DR0-DR7
  const signed int TST    = 0x00070000;  // Internal InstructionAPI Registers
  const signed int X87    = 0x00080000;  // x87 FPU Registers
  const signed int MMX    = 0x00090000;  // MM0-MM7 Registers
  const signed int XMM    = 0x000A0000;  // XMM0-XMM7 Registers from SSE
  const signed int YMM    = 0x000B0000;  // YMM0-YMM7 Registers from AVX2/FMA
  const signed int ZMM    = 0x000C0000;  // ZMM0-ZMM7 Registers from AVX-512
  const signed int KMASK  = 0x000D0000;  // K0-K7 opmask Registers from AVX-512
  const signed int FPCTL  = 0x000E0000;  // control/status Registers from x87, SSE, and AVX

  /* Base IDs for aliased GPRs */
  const signed int BASEA  = 0x0;
  const signed int BASEC  = 0x1;
  const signed int BASED  = 0x2;
  const signed int BASEB  = 0x3;
  const signed int BASESP = 0x4;
  const signed int BASEBP = 0x5;
  const signed int BASESI = 0x6;
  const signed int BASEDI = 0x7;
  const signed int FLAGS  = 0x0;

  /* Base IDs for memory segment registers */
  const signed int BASEDS = 0x0; // Data Segment register
  const signed int BASESS = 0x1; // Stack Segment register
  const signed int BASEFS = 0x2; // F Segment register
  const signed int BASEGS = 0x3; // G Segment register
  const signed int BASECS = 0x4; // Code Segment register
  const signed int BASEES = 0x5; // Extended data Segment register
  const signed int BASEGD = 0x6; // Global Descriptor Table
  const signed int BASELD = 0x7; // Local Descriptor Table
  const signed int BASEID = 0X8; // Interrupt Descriptor Table
  const signed int BASETR = 0x9; // Task Register

  /* Base IDs for each bit in EFLAGS */
  const signed int CF    = 0x00;  // Carry Flag
  const signed int FLAG1 = 0x01;  // Reserved
  const signed int PF    = 0x02;  // Parity Flag
  const signed int FLAG3 = 0x03;  // Reserved
  const signed int AF    = 0x04;  // Auxiliary Carry Flag
  const signed int FLAG5 = 0x05;  // Reserved
  const signed int ZF    = 0x06;  // Zero Flag
  const signed int SF    = 0x07;  // Sign Flag
  const signed int TF    = 0x08;  // Trap Flag
  const signed int IF    = 0x09;  // Interrupt Enable Flag
  const signed int DF    = 0x0A;  // Direction Flag
  const signed int OF    = 0x0B;  // Overflow Flag
  const signed int FLAGC = 0x0C;  // I/O Privilege Level (bits 12 and 13)
  const signed int FLAGD = 0x0D;  // I/O Privilege Level (bits 12 and 13)
  const signed int NT    = 0x0E;  // Nested Task
  const signed int FLAGF = 0x0F;  // Reserved
  const signed int RF    = 0x10;  // Resume Flag
  const signed int VM    = 0x11;  // Virtual-8086 Mode
  const signed int AC    = 0x12;  // Alignment Check/Access Control
  const signed int VIF   = 0x13;  // Virtual Interrupt Flag
  const signed int VIP   = 0x14;  // Virtual Interrupt Pending
  const signed int ID    = 0x15;  // ID Flag
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
  //          (      name,     ID | alias |   cat |     arch,  arch)
  DEF_REGISTER(       eax,  BASEA |  FULL |   GPR | Arch_x86, "x86");
  DEF_REGISTER(        ax,  BASEA | W_REG |   GPR | Arch_x86, "x86");
  DEF_REGISTER(        ah,  BASEA | H_REG |   GPR | Arch_x86, "x86");
  DEF_REGISTER(        al,  BASEA | L_REG |   GPR | Arch_x86, "x86");
  DEF_REGISTER(       ecx,  BASEC |  FULL |   GPR | Arch_x86, "x86");
  DEF_REGISTER(        cx,  BASEC | W_REG |   GPR | Arch_x86, "x86");
  DEF_REGISTER(        ch,  BASEC | H_REG |   GPR | Arch_x86, "x86");
  DEF_REGISTER(        cl,  BASEC | L_REG |   GPR | Arch_x86, "x86");
  DEF_REGISTER(       edx,  BASED |  FULL |   GPR | Arch_x86, "x86");
  DEF_REGISTER(        dx,  BASED | W_REG |   GPR | Arch_x86, "x86");
  DEF_REGISTER(        dh,  BASED | H_REG |   GPR | Arch_x86, "x86");
  DEF_REGISTER(        dl,  BASED | L_REG |   GPR | Arch_x86, "x86");
  DEF_REGISTER(       ebx,  BASEB |  FULL |   GPR | Arch_x86, "x86");
  DEF_REGISTER(        bx,  BASEB | W_REG |   GPR | Arch_x86, "x86");
  DEF_REGISTER(        bh,  BASEB | H_REG |   GPR | Arch_x86, "x86");
  DEF_REGISTER(        bl,  BASEB | L_REG |   GPR | Arch_x86, "x86");
  DEF_REGISTER(       esp, BASESP |  FULL |   GPR | Arch_x86, "x86");
  DEF_REGISTER(        sp, BASESP | W_REG |   GPR | Arch_x86, "x86");
  DEF_REGISTER(       ebp, BASEBP |  FULL |   GPR | Arch_x86, "x86");
  DEF_REGISTER(        bp, BASEBP | W_REG |   GPR | Arch_x86, "x86");
  DEF_REGISTER(       esi, BASESI |  FULL |   GPR | Arch_x86, "x86");
  DEF_REGISTER(        si, BASESI | W_REG |   GPR | Arch_x86, "x86");
  DEF_REGISTER(       edi, BASEDI |  FULL |   GPR | Arch_x86, "x86");
  DEF_REGISTER(        di, BASEDI | W_REG |   GPR | Arch_x86, "x86");
  DEF_REGISTER(       eip,   0x10 |  FULL |         Arch_x86, "x86");

  DEF_REGISTER(        cs, BASECS | W_REG |   SEG | Arch_x86, "x86");
  DEF_REGISTER(        ds, BASEDS | W_REG |   SEG | Arch_x86, "x86");
  DEF_REGISTER(        es, BASEES | W_REG |   SEG | Arch_x86, "x86");
  DEF_REGISTER(        ss, BASESS | W_REG |   SEG | Arch_x86, "x86");
  DEF_REGISTER(        fs, BASEFS | W_REG |   SEG | Arch_x86, "x86");
  DEF_REGISTER(        gs, BASEGS | W_REG |   SEG | Arch_x86, "x86");
  DEF_REGISTER(      gdtr, BASEGD | W_REG |   SEG | Arch_x86, "x86");
  DEF_REGISTER(      ldtr, BASELD | W_REG |   SEG | Arch_x86, "x86");
  DEF_REGISTER(      idtr, BASEID | W_REG |   SEG | Arch_x86, "x86");
  DEF_REGISTER(        tr, BASETR | W_REG |   SEG | Arch_x86, "x86");

  DEF_REGISTER(     flags,  FLAGS |  FULL |  FLAG | Arch_x86, "x86");
  DEF_REGISTER(        cf,     CF |   BIT |  FLAG | Arch_x86, "x86");
  DEF_REGISTER(     flag1,  FLAG1 |   BIT |  FLAG | Arch_x86, "x86");
  DEF_REGISTER(        pf,     PF |   BIT |  FLAG | Arch_x86, "x86");
  DEF_REGISTER(     flag3,  FLAG3 |   BIT |  FLAG | Arch_x86, "x86");
  DEF_REGISTER(        af,     AF |   BIT |  FLAG | Arch_x86, "x86");
  DEF_REGISTER(     flag5,  FLAG5 |   BIT |  FLAG | Arch_x86, "x86");
  DEF_REGISTER(        zf,     ZF |   BIT |  FLAG | Arch_x86, "x86");
  DEF_REGISTER(        sf,     SF |   BIT |  FLAG | Arch_x86, "x86");
  DEF_REGISTER(        tf,     TF |   BIT |  FLAG | Arch_x86, "x86");
  DEF_REGISTER(       if_,     IF |   BIT |  FLAG | Arch_x86, "x86");
  DEF_REGISTER(        df,     DF |   BIT |  FLAG | Arch_x86, "x86");
  DEF_REGISTER(        of,     OF |   BIT |  FLAG | Arch_x86, "x86");
  DEF_REGISTER(     flagc,  FLAGC |   BIT |  FLAG | Arch_x86, "x86");
  DEF_REGISTER(     flagd,  FLAGD |   BIT |  FLAG | Arch_x86, "x86");
  DEF_REGISTER(       nt_,     NT |   BIT |  FLAG | Arch_x86, "x86");
  DEF_REGISTER(     flagf,  FLAGF |   BIT |  FLAG | Arch_x86, "x86");
  DEF_REGISTER(        rf,     RF |   BIT |  FLAG | Arch_x86, "x86");
  DEF_REGISTER(        vm,     VM |   BIT |  FLAG | Arch_x86, "x86");
  DEF_REGISTER(        ac,     AC |   BIT |  FLAG | Arch_x86, "x86");
  DEF_REGISTER(       vif,    VIF |   BIT |  FLAG | Arch_x86, "x86");
  DEF_REGISTER(       vip,    VIP |   BIT |  FLAG | Arch_x86, "x86");
  DEF_REGISTER(        id,     ID |   BIT |  FLAG | Arch_x86, "x86");

  DEF_REGISTER(       cr0,    0x0 |  FULL |   CTL | Arch_x86, "x86");
  DEF_REGISTER(       cr1,    0x1 |  FULL |   CTL | Arch_x86, "x86");
  DEF_REGISTER(       cr2,    0x2 |  FULL |   CTL | Arch_x86, "x86");
  DEF_REGISTER(       cr3,    0x3 |  FULL |   CTL | Arch_x86, "x86");
  DEF_REGISTER(       cr4,    0x4 |  FULL |   CTL | Arch_x86, "x86");
  DEF_REGISTER(       cr5,    0x5 |  FULL |   CTL | Arch_x86, "x86");
  DEF_REGISTER(       cr6,    0x6 |  FULL |   CTL | Arch_x86, "x86");
  DEF_REGISTER(       cr7,    0x7 |  FULL |   CTL | Arch_x86, "x86");

  DEF_REGISTER(       dr0,    0x0 |  FULL |   DBG | Arch_x86, "x86");
  DEF_REGISTER(       dr1,    0x1 |  FULL |   DBG | Arch_x86, "x86");
  DEF_REGISTER(       dr2,    0x2 |  FULL |   DBG | Arch_x86, "x86");
  DEF_REGISTER(       dr3,    0x3 |  FULL |   DBG | Arch_x86, "x86");
  DEF_REGISTER(       dr4,    0x4 |  FULL |   DBG | Arch_x86, "x86");
  DEF_REGISTER(       dr5,    0x5 |  FULL |   DBG | Arch_x86, "x86");
  DEF_REGISTER(       dr6,    0x6 |  FULL |   DBG | Arch_x86, "x86");
  DEF_REGISTER(       dr7,    0x7 |  FULL |   DBG | Arch_x86, "x86");

  DEF_REGISTER(       st0,    0x0 | FPDBL |   X87 | Arch_x86, "x86");
  DEF_REGISTER(       st1,    0x1 | FPDBL |   X87 | Arch_x86, "x86");
  DEF_REGISTER(       st2,    0x2 | FPDBL |   X87 | Arch_x86, "x86");
  DEF_REGISTER(       st3,    0x3 | FPDBL |   X87 | Arch_x86, "x86");
  DEF_REGISTER(       st4,    0x4 | FPDBL |   X87 | Arch_x86, "x86");
  DEF_REGISTER(       st5,    0x5 | FPDBL |   X87 | Arch_x86, "x86");
  DEF_REGISTER(       st6,    0x6 | FPDBL |   X87 | Arch_x86, "x86");
  DEF_REGISTER(       st7,    0x7 | FPDBL |   X87 | Arch_x86, "x86");
  DEF_REGISTER(       fcw,    0x8 | W_REG | FPCTL | Arch_x86, "x86");
  DEF_REGISTER(       fsw,    0x9 | W_REG | FPCTL | Arch_x86, "x86");

  DEF_REGISTER(       mm0,    0x0 |   MMS |   MMX | Arch_x86, "x86");
  DEF_REGISTER(       mm1,    0x1 |   MMS |   MMX | Arch_x86, "x86");
  DEF_REGISTER(       mm2,    0x2 |   MMS |   MMX | Arch_x86, "x86");
  DEF_REGISTER(       mm3,    0x3 |   MMS |   MMX | Arch_x86, "x86");
  DEF_REGISTER(       mm4,    0x4 |   MMS |   MMX | Arch_x86, "x86");
  DEF_REGISTER(       mm5,    0x5 |   MMS |   MMX | Arch_x86, "x86");
  DEF_REGISTER(       mm6,    0x6 |   MMS |   MMX | Arch_x86, "x86");
  DEF_REGISTER(       mm7,    0x7 |   MMS |   MMX | Arch_x86, "x86");

  DEF_REGISTER(      xmm0,   0x00 |  XMMS |   XMM | Arch_x86, "x86");
  DEF_REGISTER(      xmm1,   0x01 |  XMMS |   XMM | Arch_x86, "x86");
  DEF_REGISTER(      xmm2,   0x02 |  XMMS |   XMM | Arch_x86, "x86");
  DEF_REGISTER(      xmm3,   0x03 |  XMMS |   XMM | Arch_x86, "x86");
  DEF_REGISTER(      xmm4,   0x04 |  XMMS |   XMM | Arch_x86, "x86");
  DEF_REGISTER(      xmm5,   0x05 |  XMMS |   XMM | Arch_x86, "x86");
  DEF_REGISTER(      xmm6,   0x06 |  XMMS |   XMM | Arch_x86, "x86");
  DEF_REGISTER(      xmm7,   0x07 |  XMMS |   XMM | Arch_x86, "x86");
  DEF_REGISTER(     mxcsr,   0x08 |  FULL | FPCTL | Arch_x86, "x86");

  DEF_REGISTER(      ymm0,   0x00 |  YMMS |   YMM | Arch_x86, "x86");
  DEF_REGISTER(      ymm1,   0x01 |  YMMS |   YMM | Arch_x86, "x86");
  DEF_REGISTER(      ymm2,   0x02 |  YMMS |   YMM | Arch_x86, "x86");
  DEF_REGISTER(      ymm3,   0x03 |  YMMS |   YMM | Arch_x86, "x86");
  DEF_REGISTER(      ymm4,   0x04 |  YMMS |   YMM | Arch_x86, "x86");
  DEF_REGISTER(      ymm5,   0x05 |  YMMS |   YMM | Arch_x86, "x86");
  DEF_REGISTER(      ymm6,   0x06 |  YMMS |   YMM | Arch_x86, "x86");
  DEF_REGISTER(      ymm7,   0x07 |  YMMS |   YMM | Arch_x86, "x86");

  DEF_REGISTER(      zmm0,   0x00 |  ZMMS |   ZMM | Arch_x86, "x86");
  DEF_REGISTER(      zmm1,   0x01 |  ZMMS |   ZMM | Arch_x86, "x86");
  DEF_REGISTER(      zmm2,   0x02 |  ZMMS |   ZMM | Arch_x86, "x86");
  DEF_REGISTER(      zmm3,   0x03 |  ZMMS |   ZMM | Arch_x86, "x86");
  DEF_REGISTER(      zmm4,   0x04 |  ZMMS |   ZMM | Arch_x86, "x86");
  DEF_REGISTER(      zmm5,   0x05 |  ZMMS |   ZMM | Arch_x86, "x86");
  DEF_REGISTER(      zmm6,   0x06 |  ZMMS |   ZMM | Arch_x86, "x86");
  DEF_REGISTER(      zmm7,   0x07 |  ZMMS |   ZMM | Arch_x86, "x86");

  DEF_REGISTER(        k0,   0x00 | KMSKS | KMASK | Arch_x86, "x86");
  DEF_REGISTER(        k1,   0x01 | KMSKS | KMASK | Arch_x86, "x86");
  DEF_REGISTER(        k2,   0x02 | KMSKS | KMASK | Arch_x86, "x86");
  DEF_REGISTER(        k3,   0x03 | KMSKS | KMASK | Arch_x86, "x86");
  DEF_REGISTER(        k4,   0x04 | KMSKS | KMASK | Arch_x86, "x86");
  DEF_REGISTER(        k5,   0x05 | KMSKS | KMASK | Arch_x86, "x86");
  DEF_REGISTER(        k6,   0x06 | KMSKS | KMASK | Arch_x86, "x86");
  DEF_REGISTER(        k7,   0x07 | KMSKS | KMASK | Arch_x86, "x86");

  /* Pseudo-registers for internal use only */
  DEF_REGISTER(      oeax,    0x0 |  FULL |  MISC | Arch_x86, "x86");
  DEF_REGISTER(    fsbase,    0x1 |  FULL |  MISC | Arch_x86, "x86");
  DEF_REGISTER(    gsbase,    0x2 |  FULL |  MISC | Arch_x86, "x86");
  DEF_REGISTER(       tr0,    0x0 |  FULL |   TST | Arch_x86, "x86");
  DEF_REGISTER(       tr1,    0x1 |  FULL |   TST | Arch_x86, "x86");
  DEF_REGISTER(       tr2,    0x2 |  FULL |   TST | Arch_x86, "x86");
  DEF_REGISTER(       tr3,    0x3 |  FULL |   TST | Arch_x86, "x86");
  DEF_REGISTER(       tr4,    0x4 |  FULL |   TST | Arch_x86, "x86");
  DEF_REGISTER(       tr5,    0x5 |  FULL |   TST | Arch_x86, "x86");
  DEF_REGISTER(       tr6,    0x6 |  FULL |   TST | Arch_x86, "x86");
  DEF_REGISTER(       tr7,    0x7 |  FULL |   TST | Arch_x86, "x86");
}

inline bool isSegmentRegister(int regClass) {
  return 0 != (regClass & x86::SEG);
}

}

#endif
