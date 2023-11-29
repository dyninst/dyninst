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
  const signed int FULL   = 0x00000000; // 64 bits
  const signed int BIT    = 0x00000100; // 1-bit EFLAGS
  const signed int L_REG  = 0x00000200; // 8-bit, first byte
  const signed int H_REG  = 0x00000300; // 8-bit, second byte
  const signed int W_REG  = 0x00000400; // 16-bit, first word
  const signed int D_REG  = 0x00000500; // 32 bit, first double word
  const signed int FPDBL  = 0x00000600; // 80-bit x87 FPU
  const signed int MMS    = 0x00000700; // 64-bit MMX and 3DNow!
  const signed int XMMS   = 0x00000800; // 128-bit SSE, FC16, XOP, AVX, and FMA3/4
  const signed int YMMS   = 0x00000900; // 256-bit SSE, AVX2, FMA3/4
  const signed int ZMMS   = 0x00000A00; // 512-bit AVX-512/AVX10
  const signed int KMSKS  = 0x00000B00; // 64-bit mask from AVX-512/AVX10

  /* Register Categories */
  const signed int GPR    = 0x00010000;  // General-Purpose Registers
  const signed int SEG    = 0x00020000;  // Segment Registers
  const signed int FLAG   = 0x00030000;  // RFLAGS Register
  const signed int MISC   = 0x00040000;  // Internal ProcControlAPI Register
  const signed int CTL    = 0x00050000;  // Control Registers CR0-CR7
  const signed int DBG    = 0x00060000;  // Debug Registers DR0-DR7
  const signed int TST    = 0x00070000;  // Internal InstructionAPI Registers
  const signed int X87    = 0x00080000;  // x87 FPU Registers
  const signed int MMX    = 0x00090000;  // MM0-MM7 Registers
  const signed int XMM    = 0x000A0000;  // XMM0-XMM15 Registers from SSE (XMM0-XMM31 for AVX-512)
  const signed int YMM    = 0x000B0000;  // YMM0-YMM15 Registers from AVX2/FMA (YMM0-YMM31 for AVX-512)
  const signed int ZMM    = 0x000C0000;  // ZMM0-ZMM31 Registers from AVX-512
  const signed int KMASK  = 0x000D0000;  // K0-K7 opmask Registers from AVX-512
  const signed int FPCTL  = 0x000E0000;  // control/status Registers from x87, SSE, and AVX

  /* Base IDs for aliased GPRs */
  const signed int FLAGS  = 0x00;  // RFLAGS Register
  const signed int BASEA  = 0x00;
  const signed int BASEC  = 0x01;
  const signed int BASED  = 0x02;
  const signed int BASEB  = 0x03;
  const signed int BASESP = 0x04;
  const signed int BASEBP = 0x05;
  const signed int BASESI = 0x06;
  const signed int BASEDI = 0x07;
  const signed int BASE8  = 0x08;
  const signed int BASE9  = 0x09;
  const signed int BASE10 = 0x0A;
  const signed int BASE11 = 0x0B;
  const signed int BASE12 = 0x0C;
  const signed int BASE13 = 0x0D;
  const signed int BASE14 = 0x0E;
  const signed int BASE15 = 0x0F;

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

  /* Base IDs for each bit in RFLAGS */
  const signed int CF    = x86::CF;    // Carry Flag
  const signed int FLAG1 = x86::FLAG1; // Reserved
  const signed int PF    = x86::PF;    // Parity Flag
  const signed int FLAG3 = x86::FLAG3; // Reserved
  const signed int AF    = x86::AF;    // Auxiliary Carry Flag
  const signed int FLAG5 = x86::FLAG5; // Reserved
  const signed int ZF    = x86::ZF;    // Zero Flag
  const signed int SF    = x86::SF;    // Sign Flag
  const signed int TF    = x86::TF;    // Trap Flag
  const signed int IF    = x86::IF;    // Interrupt Enable Flag
  const signed int DF    = x86::DF;    // Direction Flag
  const signed int OF    = x86::OF;    // Overflow Flag
  const signed int FLAGC = x86::FLAGC; // I/O Privilege Level (bits 12 and 13)
  const signed int FLAGD = x86::FLAGD; // I/O Privilege Level (bits 12 and 13)
  const signed int NT    = x86::NT;    // Nested Task
  const signed int FLAGF = x86::FLAGF; // Reserved
  const signed int RF    = x86::RF;    // Resume Flag
  const signed int VM    = x86::VM;    // Virtual-8086 Mode
  const signed int AC    = x86::AC;    // Alignment Check/Access Control
  const signed int VIF   = x86::VIF;   // Virtual Interrupt Flag
  const signed int VIP   = x86::VIP;   // Virtual Interrupt Pending
  const signed int ID    = x86::ID;    // ID Flag
  /* Flags 22-63 are reserved */

  /* Format of constants:
   *  [0x000000ff] Lower 16 bits are base register ID
   *  [0x0000ff00] Next 16 bits are the aliasing and subrange ID used to distinguish
   *               between whole and aliased registers like EAX and AH.
   *  [0x00ff0000] Next 16 bits are the register category, GPR, FLAG, etc.
   *  [0xff000000] Upper 16 bits are the architecture.
   */
  //          (      name,     ID | alias |   cat |        arch,     arch)
  DEF_REGISTER(       rax,  BASEA |  FULL |   GPR | Arch_x86_64, "x86_64");
  DEF_REGISTER(       eax,  BASEA | D_REG |   GPR | Arch_x86_64, "x86_64");
  DEF_REGISTER(        ax,  BASEA | W_REG |   GPR | Arch_x86_64, "x86_64");
  DEF_REGISTER(        ah,  BASEA | H_REG |   GPR | Arch_x86_64, "x86_64");
  DEF_REGISTER(        al,  BASEA | L_REG |   GPR | Arch_x86_64, "x86_64");
  DEF_REGISTER(       rcx,  BASEC |  FULL |   GPR | Arch_x86_64, "x86_64");
  DEF_REGISTER(       ecx,  BASEC | D_REG |   GPR | Arch_x86_64, "x86_64");
  DEF_REGISTER(        cx,  BASEC | W_REG |   GPR | Arch_x86_64, "x86_64");
  DEF_REGISTER(        ch,  BASEC | H_REG |   GPR | Arch_x86_64, "x86_64");
  DEF_REGISTER(        cl,  BASEC | L_REG |   GPR | Arch_x86_64, "x86_64");
  DEF_REGISTER(       rdx,  BASED |  FULL |   GPR | Arch_x86_64, "x86_64");
  DEF_REGISTER(       edx,  BASED | D_REG |   GPR | Arch_x86_64, "x86_64");
  DEF_REGISTER(        dx,  BASED | W_REG |   GPR | Arch_x86_64, "x86_64");
  DEF_REGISTER(        dh,  BASED | H_REG |   GPR | Arch_x86_64, "x86_64");
  DEF_REGISTER(        dl,  BASED | L_REG |   GPR | Arch_x86_64, "x86_64");
  DEF_REGISTER(       rbx,  BASEB |  FULL |   GPR | Arch_x86_64, "x86_64");
  DEF_REGISTER(       ebx,  BASEB | D_REG |   GPR | Arch_x86_64, "x86_64");
  DEF_REGISTER(        bx,  BASEB | W_REG |   GPR | Arch_x86_64, "x86_64");
  DEF_REGISTER(        bh,  BASEB | H_REG |   GPR | Arch_x86_64, "x86_64");
  DEF_REGISTER(        bl,  BASEB | L_REG |   GPR | Arch_x86_64, "x86_64");
  DEF_REGISTER(       rsp, BASESP |  FULL |   GPR | Arch_x86_64, "x86_64");
  DEF_REGISTER(       esp, BASESP | D_REG |   GPR | Arch_x86_64, "x86_64");
  DEF_REGISTER(        sp, BASESP | W_REG |   GPR | Arch_x86_64, "x86_64");
  DEF_REGISTER(       spl, BASESP | L_REG |   GPR | Arch_x86_64, "x86_64");
  DEF_REGISTER(       rbp, BASEBP |  FULL |   GPR | Arch_x86_64, "x86_64");
  DEF_REGISTER(       ebp, BASEBP | D_REG |   GPR | Arch_x86_64, "x86_64");
  DEF_REGISTER(        bp, BASEBP | W_REG |   GPR | Arch_x86_64, "x86_64");
  DEF_REGISTER(       bpl, BASEBP | L_REG |   GPR | Arch_x86_64, "x86_64");
  DEF_REGISTER(       rsi, BASESI |  FULL |   GPR | Arch_x86_64, "x86_64");
  DEF_REGISTER(       esi, BASESI | D_REG |   GPR | Arch_x86_64, "x86_64");
  DEF_REGISTER(        si, BASESI | W_REG |   GPR | Arch_x86_64, "x86_64");
  DEF_REGISTER(       sil, BASESI | L_REG |   GPR | Arch_x86_64, "x86_64");
  DEF_REGISTER(       rdi, BASEDI |  FULL |   GPR | Arch_x86_64, "x86_64");
  DEF_REGISTER(       edi, BASEDI | D_REG |   GPR | Arch_x86_64, "x86_64");
  DEF_REGISTER(        di, BASEDI | W_REG |   GPR | Arch_x86_64, "x86_64");
  DEF_REGISTER(       dil, BASEDI | L_REG |   GPR | Arch_x86_64, "x86_64");
  DEF_REGISTER(        r8,  BASE8 |  FULL |   GPR | Arch_x86_64, "x86_64");
  DEF_REGISTER(       r8b,  BASE8 | L_REG |   GPR | Arch_x86_64, "x86_64");
  DEF_REGISTER(       r8w,  BASE8 | W_REG |   GPR | Arch_x86_64, "x86_64");
  DEF_REGISTER(       r8d,  BASE8 | D_REG |   GPR | Arch_x86_64, "x86_64");
  DEF_REGISTER(        r9,  BASE9 |  FULL |   GPR | Arch_x86_64, "x86_64");
  DEF_REGISTER(       r9b,  BASE9 | L_REG |   GPR | Arch_x86_64, "x86_64");
  DEF_REGISTER(       r9w,  BASE9 | W_REG |   GPR | Arch_x86_64, "x86_64");
  DEF_REGISTER(       r9d,  BASE9 | D_REG |   GPR | Arch_x86_64, "x86_64");
  DEF_REGISTER(       r10, BASE10 |  FULL |   GPR | Arch_x86_64, "x86_64");
  DEF_REGISTER(      r10b, BASE10 | L_REG |   GPR | Arch_x86_64, "x86_64");
  DEF_REGISTER(      r10w, BASE10 | W_REG |   GPR | Arch_x86_64, "x86_64");
  DEF_REGISTER(      r10d, BASE10 | D_REG |   GPR | Arch_x86_64, "x86_64");
  DEF_REGISTER(       r11, BASE11 |  FULL |   GPR | Arch_x86_64, "x86_64");
  DEF_REGISTER(      r11b, BASE11 | L_REG |   GPR | Arch_x86_64, "x86_64");
  DEF_REGISTER(      r11w, BASE11 | W_REG |   GPR | Arch_x86_64, "x86_64");
  DEF_REGISTER(      r11d, BASE11 | D_REG |   GPR | Arch_x86_64, "x86_64");
  DEF_REGISTER(       r12, BASE12 |  FULL |   GPR | Arch_x86_64, "x86_64");
  DEF_REGISTER(      r12b, BASE12 | L_REG |   GPR | Arch_x86_64, "x86_64");
  DEF_REGISTER(      r12w, BASE12 | W_REG |   GPR | Arch_x86_64, "x86_64");
  DEF_REGISTER(      r12d, BASE12 | D_REG |   GPR | Arch_x86_64, "x86_64");
  DEF_REGISTER(       r13, BASE13 |  FULL |   GPR | Arch_x86_64, "x86_64");
  DEF_REGISTER(      r13b, BASE13 | L_REG |   GPR | Arch_x86_64, "x86_64");
  DEF_REGISTER(      r13w, BASE13 | W_REG |   GPR | Arch_x86_64, "x86_64");
  DEF_REGISTER(      r13d, BASE13 | D_REG |   GPR | Arch_x86_64, "x86_64");
  DEF_REGISTER(       r14, BASE14 |  FULL |   GPR | Arch_x86_64, "x86_64");
  DEF_REGISTER(      r14b, BASE14 | L_REG |   GPR | Arch_x86_64, "x86_64");
  DEF_REGISTER(      r14w, BASE14 | W_REG |   GPR | Arch_x86_64, "x86_64");
  DEF_REGISTER(      r14d, BASE14 | D_REG |   GPR | Arch_x86_64, "x86_64");
  DEF_REGISTER(       r15, BASE15 |  FULL |   GPR | Arch_x86_64, "x86_64");
  DEF_REGISTER(      r15b, BASE15 | L_REG |   GPR | Arch_x86_64, "x86_64");
  DEF_REGISTER(      r15w, BASE15 | W_REG |   GPR | Arch_x86_64, "x86_64");
  DEF_REGISTER(      r15d, BASE15 | D_REG |   GPR | Arch_x86_64, "x86_64");
  DEF_REGISTER(       rip,   0x10 |  FULL |         Arch_x86_64, "x86_64");
  DEF_REGISTER(       eip,   0x10 | D_REG |         Arch_x86_64, "x86_64");

  DEF_REGISTER(     flags,  FLAGS |  FULL |  FLAG | Arch_x86_64, "x86_64");
  DEF_REGISTER(        cf,     CF |   BIT |  FLAG | Arch_x86_64, "x86_64");
  DEF_REGISTER(     flag1,  FLAG1 |   BIT |  FLAG | Arch_x86_64, "x86_64");
  DEF_REGISTER(        pf,     PF |   BIT |  FLAG | Arch_x86_64, "x86_64");
  DEF_REGISTER(     flag3,  FLAG3 |   BIT |  FLAG | Arch_x86_64, "x86_64");
  DEF_REGISTER(        af,     AF |   BIT |  FLAG | Arch_x86_64, "x86_64");
  DEF_REGISTER(     flag5,  FLAG5 |   BIT |  FLAG | Arch_x86_64, "x86_64");
  DEF_REGISTER(        zf,     ZF |   BIT |  FLAG | Arch_x86_64, "x86_64");
  DEF_REGISTER(        sf,     SF |   BIT |  FLAG | Arch_x86_64, "x86_64");
  DEF_REGISTER(        tf,     TF |   BIT |  FLAG | Arch_x86_64, "x86_64");
  DEF_REGISTER(       if_,     IF |   BIT |  FLAG | Arch_x86_64, "x86_64");
  DEF_REGISTER(        df,     DF |   BIT |  FLAG | Arch_x86_64, "x86_64");
  DEF_REGISTER(        of,     OF |   BIT |  FLAG | Arch_x86_64, "x86_64");
  DEF_REGISTER(     flagc,  FLAGC |   BIT |  FLAG | Arch_x86_64, "x86_64");
  DEF_REGISTER(     flagd,  FLAGD |   BIT |  FLAG | Arch_x86_64, "x86_64");
  DEF_REGISTER(       nt_,     NT |   BIT |  FLAG | Arch_x86_64, "x86_64");
  DEF_REGISTER(     flagf,  FLAGF |   BIT |  FLAG | Arch_x86_64, "x86_64");
  DEF_REGISTER(        rf,     RF |   BIT |  FLAG | Arch_x86_64, "x86_64");
  DEF_REGISTER(        vm,     VM |   BIT |  FLAG | Arch_x86_64, "x86_64");
  DEF_REGISTER(        ac,     AC |   BIT |  FLAG | Arch_x86_64, "x86_64");
  DEF_REGISTER(       vif,    VIF |   BIT |  FLAG | Arch_x86_64, "x86_64");
  DEF_REGISTER(       vip,    VIP |   BIT |  FLAG | Arch_x86_64, "x86_64");
  DEF_REGISTER(       id,      ID |   BIT |  FLAG | Arch_x86_64, "x86_64");

  DEF_REGISTER(        ds, BASEDS |  FULL |   SEG | Arch_x86_64, "x86_64");
  DEF_REGISTER(        es, BASEES |  FULL |   SEG | Arch_x86_64, "x86_64");
  DEF_REGISTER(        fs, BASEFS |  FULL |   SEG | Arch_x86_64, "x86_64");
  DEF_REGISTER(        gs, BASEGS |  FULL |   SEG | Arch_x86_64, "x86_64");
  DEF_REGISTER(        cs, BASECS |  FULL |   SEG | Arch_x86_64, "x86_64");
  DEF_REGISTER(        ss, BASESS |  FULL |   SEG | Arch_x86_64, "x86_64");
  DEF_REGISTER(      gdtr, BASEGD | W_REG |   SEG | Arch_x86_64, "x86_64");
  DEF_REGISTER(      ldtr, BASELD | W_REG |   SEG | Arch_x86_64, "x86_64");
  DEF_REGISTER(      idtr, BASEID | W_REG |   SEG | Arch_x86_64, "x86_64");
  DEF_REGISTER(        tr, BASETR | W_REG |   SEG | Arch_x86_64, "x86_64");

  DEF_REGISTER(        cr0,   0x0 |  FULL |   CTL | Arch_x86_64, "x86_64");
  DEF_REGISTER(        cr1,   0x1 |  FULL |   CTL | Arch_x86_64, "x86_64");
  DEF_REGISTER(        cr2,   0x2 |  FULL |   CTL | Arch_x86_64, "x86_64");
  DEF_REGISTER(        cr3,   0x3 |  FULL |   CTL | Arch_x86_64, "x86_64");
  DEF_REGISTER(        cr4,   0x4 |  FULL |   CTL | Arch_x86_64, "x86_64");
  DEF_REGISTER(        cr5,   0x5 |  FULL |   CTL | Arch_x86_64, "x86_64");
  DEF_REGISTER(        cr6,   0x6 |  FULL |   CTL | Arch_x86_64, "x86_64");
  DEF_REGISTER(        cr7,   0x7 |  FULL |   CTL | Arch_x86_64, "x86_64");
  DEF_REGISTER(        cr8,   0x8 |  FULL |   CTL | Arch_x86_64, "x86_64");
  DEF_REGISTER(        cr9,   0x9 |  FULL |   CTL | Arch_x86_64, "x86_64");
  DEF_REGISTER(       cr10,   0xA |  FULL |   CTL | Arch_x86_64, "x86_64");
  DEF_REGISTER(       cr11,   0xB |  FULL |   CTL | Arch_x86_64, "x86_64");
  DEF_REGISTER(       cr12,   0xC |  FULL |   CTL | Arch_x86_64, "x86_64");
  DEF_REGISTER(       cr13,   0xD |  FULL |   CTL | Arch_x86_64, "x86_64");
  DEF_REGISTER(       cr14,   0xE |  FULL |   CTL | Arch_x86_64, "x86_64");
  DEF_REGISTER(       cr15,   0xF |  FULL |   CTL | Arch_x86_64, "x86_64");

  DEF_REGISTER(        dr0,    0x0 |  FULL |   DBG | Arch_x86_64, "x86_64");
  DEF_REGISTER(        dr1,    0x1 |  FULL |   DBG | Arch_x86_64, "x86_64");
  DEF_REGISTER(        dr2,    0x2 |  FULL |   DBG | Arch_x86_64, "x86_64");
  DEF_REGISTER(        dr3,    0x3 |  FULL |   DBG | Arch_x86_64, "x86_64");
  DEF_REGISTER(        dr4,    0x4 |  FULL |   DBG | Arch_x86_64, "x86_64");
  DEF_REGISTER(        dr5,    0x5 |  FULL |   DBG | Arch_x86_64, "x86_64");
  DEF_REGISTER(        dr6,    0x6 |  FULL |   DBG | Arch_x86_64, "x86_64");
  DEF_REGISTER(        dr7,    0x7 |  FULL |   DBG | Arch_x86_64, "x86_64");
  DEF_REGISTER(        dr8,    0x8 |  FULL |   DBG | Arch_x86_64, "x86_64");
  DEF_REGISTER(        dr9,    0x9 |  FULL |   DBG | Arch_x86_64, "x86_64");
  DEF_REGISTER(       dr10,    0xA |  FULL |   DBG | Arch_x86_64, "x86_64");
  DEF_REGISTER(       dr11,    0xB |  FULL |   DBG | Arch_x86_64, "x86_64");
  DEF_REGISTER(       dr12,    0xC |  FULL |   DBG | Arch_x86_64, "x86_64");
  DEF_REGISTER(       dr13,    0xD |  FULL |   DBG | Arch_x86_64, "x86_64");
  DEF_REGISTER(       dr14,    0xE |  FULL |   DBG | Arch_x86_64, "x86_64");
  DEF_REGISTER(       dr15,    0xF |  FULL |   DBG | Arch_x86_64, "x86_64");

  DEF_REGISTER(       st0,    0x0 | FPDBL |   X87 | Arch_x86_64, "x86_64");
  DEF_REGISTER(       st1,    0x1 | FPDBL |   X87 | Arch_x86_64, "x86_64");
  DEF_REGISTER(       st2,    0x2 | FPDBL |   X87 | Arch_x86_64, "x86_64");
  DEF_REGISTER(       st3,    0x3 | FPDBL |   X87 | Arch_x86_64, "x86_64");
  DEF_REGISTER(       st4,    0x4 | FPDBL |   X87 | Arch_x86_64, "x86_64");
  DEF_REGISTER(       st5,    0x5 | FPDBL |   X87 | Arch_x86_64, "x86_64");
  DEF_REGISTER(       st6,    0x6 | FPDBL |   X87 | Arch_x86_64, "x86_64");
  DEF_REGISTER(       st7,    0x7 | FPDBL |   X87 | Arch_x86_64, "x86_64");
  DEF_REGISTER(       fcw,    0x8 | W_REG | FPCTL | Arch_x86_64, "x86_64");
  DEF_REGISTER(       fsw,    0x9 | W_REG | FPCTL | Arch_x86_64, "x86_64");

  DEF_REGISTER(       mm0,    0x0 |   MMS |   MMX | Arch_x86_64, "x86_64");
  DEF_REGISTER(       mm1,    0x1 |   MMS |   MMX | Arch_x86_64, "x86_64");
  DEF_REGISTER(       mm2,    0x2 |   MMS |   MMX | Arch_x86_64, "x86_64");
  DEF_REGISTER(       mm3,    0x3 |   MMS |   MMX | Arch_x86_64, "x86_64");
  DEF_REGISTER(       mm4,    0x4 |   MMS |   MMX | Arch_x86_64, "x86_64");
  DEF_REGISTER(       mm5,    0x5 |   MMS |   MMX | Arch_x86_64, "x86_64");
  DEF_REGISTER(       mm6,    0x6 |   MMS |   MMX | Arch_x86_64, "x86_64");
  DEF_REGISTER(       mm7,    0x7 |   MMS |   MMX | Arch_x86_64, "x86_64");

  DEF_REGISTER(      xmm0,   0x00 |  XMMS |   XMM | Arch_x86_64, "x86_64");
  DEF_REGISTER(      xmm1,   0x01 |  XMMS |   XMM | Arch_x86_64, "x86_64");
  DEF_REGISTER(      xmm2,   0x02 |  XMMS |   XMM | Arch_x86_64, "x86_64");
  DEF_REGISTER(      xmm3,   0x03 |  XMMS |   XMM | Arch_x86_64, "x86_64");
  DEF_REGISTER(      xmm4,   0x04 |  XMMS |   XMM | Arch_x86_64, "x86_64");
  DEF_REGISTER(      xmm5,   0x05 |  XMMS |   XMM | Arch_x86_64, "x86_64");
  DEF_REGISTER(      xmm6,   0x06 |  XMMS |   XMM | Arch_x86_64, "x86_64");
  DEF_REGISTER(      xmm7,   0x07 |  XMMS |   XMM | Arch_x86_64, "x86_64");
  DEF_REGISTER(      xmm8,   0x08 |  XMMS |   XMM | Arch_x86_64, "x86_64");
  DEF_REGISTER(      xmm9,   0x09 |  XMMS |   XMM | Arch_x86_64, "x86_64");
  DEF_REGISTER(     xmm10,   0x0A |  XMMS |   XMM | Arch_x86_64, "x86_64");
  DEF_REGISTER(     xmm11,   0x0B |  XMMS |   XMM | Arch_x86_64, "x86_64");
  DEF_REGISTER(     xmm12,   0x0C |  XMMS |   XMM | Arch_x86_64, "x86_64");
  DEF_REGISTER(     xmm13,   0x0D |  XMMS |   XMM | Arch_x86_64, "x86_64");
  DEF_REGISTER(     xmm14,   0x0E |  XMMS |   XMM | Arch_x86_64, "x86_64");
  DEF_REGISTER(     xmm15,   0x0F |  XMMS |   XMM | Arch_x86_64, "x86_64");
  DEF_REGISTER(     xmm16,   0x10 |  XMMS |   XMM | Arch_x86_64, "x86_64");
  DEF_REGISTER(     xmm17,   0x11 |  XMMS |   XMM | Arch_x86_64, "x86_64");
  DEF_REGISTER(     xmm18,   0x12 |  XMMS |   XMM | Arch_x86_64, "x86_64");
  DEF_REGISTER(     xmm19,   0x13 |  XMMS |   XMM | Arch_x86_64, "x86_64");
  DEF_REGISTER(     xmm20,   0x14 |  XMMS |   XMM | Arch_x86_64, "x86_64");
  DEF_REGISTER(     xmm21,   0x15 |  XMMS |   XMM | Arch_x86_64, "x86_64");
  DEF_REGISTER(     xmm22,   0x16 |  XMMS |   XMM | Arch_x86_64, "x86_64");
  DEF_REGISTER(     xmm23,   0x17 |  XMMS |   XMM | Arch_x86_64, "x86_64");
  DEF_REGISTER(     xmm24,   0x18 |  XMMS |   XMM | Arch_x86_64, "x86_64");
  DEF_REGISTER(     xmm25,   0x19 |  XMMS |   XMM | Arch_x86_64, "x86_64");
  DEF_REGISTER(     xmm26,   0x1A |  XMMS |   XMM | Arch_x86_64, "x86_64");
  DEF_REGISTER(     xmm27,   0x1B |  XMMS |   XMM | Arch_x86_64, "x86_64");
  DEF_REGISTER(     xmm28,   0x1C |  XMMS |   XMM | Arch_x86_64, "x86_64");
  DEF_REGISTER(     xmm29,   0x1D |  XMMS |   XMM | Arch_x86_64, "x86_64");
  DEF_REGISTER(     xmm30,   0x1E |  XMMS |   XMM | Arch_x86_64, "x86_64");
  DEF_REGISTER(     xmm31,   0x1F |  XMMS |   XMM | Arch_x86_64, "x86_64");
  DEF_REGISTER(     mxcsr,   0x20 | D_REG | FPCTL | Arch_x86_64, "x86_64");

  DEF_REGISTER(      ymm0,   0x00 |  YMMS |   YMM | Arch_x86_64, "x86_64");
  DEF_REGISTER(      ymm1,   0x01 |  YMMS |   YMM | Arch_x86_64, "x86_64");
  DEF_REGISTER(      ymm2,   0x02 |  YMMS |   YMM | Arch_x86_64, "x86_64");
  DEF_REGISTER(      ymm3,   0x03 |  YMMS |   YMM | Arch_x86_64, "x86_64");
  DEF_REGISTER(      ymm4,   0x04 |  YMMS |   YMM | Arch_x86_64, "x86_64");
  DEF_REGISTER(      ymm5,   0x05 |  YMMS |   YMM | Arch_x86_64, "x86_64");
  DEF_REGISTER(      ymm6,   0x06 |  YMMS |   YMM | Arch_x86_64, "x86_64");
  DEF_REGISTER(      ymm7,   0x07 |  YMMS |   YMM | Arch_x86_64, "x86_64");
  DEF_REGISTER(      ymm8,   0x08 |  YMMS |   YMM | Arch_x86_64, "x86_64");
  DEF_REGISTER(      ymm9,   0x09 |  YMMS |   YMM | Arch_x86_64, "x86_64");
  DEF_REGISTER(     ymm10,   0x0A |  YMMS |   YMM | Arch_x86_64, "x86_64");
  DEF_REGISTER(     ymm11,   0x0B |  YMMS |   YMM | Arch_x86_64, "x86_64");
  DEF_REGISTER(     ymm12,   0x0C |  YMMS |   YMM | Arch_x86_64, "x86_64");
  DEF_REGISTER(     ymm13,   0x0D |  YMMS |   YMM | Arch_x86_64, "x86_64");
  DEF_REGISTER(     ymm14,   0x0E |  YMMS |   YMM | Arch_x86_64, "x86_64");
  DEF_REGISTER(     ymm15,   0x0F |  YMMS |   YMM | Arch_x86_64, "x86_64");
  DEF_REGISTER(     ymm16,   0x10 |  YMMS |   YMM | Arch_x86_64, "x86_64");
  DEF_REGISTER(     ymm17,   0x11 |  YMMS |   YMM | Arch_x86_64, "x86_64");
  DEF_REGISTER(     ymm18,   0x12 |  YMMS |   YMM | Arch_x86_64, "x86_64");
  DEF_REGISTER(     ymm19,   0x13 |  YMMS |   YMM | Arch_x86_64, "x86_64");
  DEF_REGISTER(     ymm20,   0x14 |  YMMS |   YMM | Arch_x86_64, "x86_64");
  DEF_REGISTER(     ymm21,   0x15 |  YMMS |   YMM | Arch_x86_64, "x86_64");
  DEF_REGISTER(     ymm22,   0x16 |  YMMS |   YMM | Arch_x86_64, "x86_64");
  DEF_REGISTER(     ymm23,   0x17 |  YMMS |   YMM | Arch_x86_64, "x86_64");
  DEF_REGISTER(     ymm24,   0x18 |  YMMS |   YMM | Arch_x86_64, "x86_64");
  DEF_REGISTER(     ymm25,   0x19 |  YMMS |   YMM | Arch_x86_64, "x86_64");
  DEF_REGISTER(     ymm26,   0x1A |  YMMS |   YMM | Arch_x86_64, "x86_64");
  DEF_REGISTER(     ymm27,   0x1B |  YMMS |   YMM | Arch_x86_64, "x86_64");
  DEF_REGISTER(     ymm28,   0x1C |  YMMS |   YMM | Arch_x86_64, "x86_64");
  DEF_REGISTER(     ymm29,   0x1D |  YMMS |   YMM | Arch_x86_64, "x86_64");
  DEF_REGISTER(     ymm30,   0x1E |  YMMS |   YMM | Arch_x86_64, "x86_64");
  DEF_REGISTER(     ymm31,   0x1F |  YMMS |   YMM | Arch_x86_64, "x86_64");

  DEF_REGISTER(      zmm0,   0x00 |  ZMMS |   ZMM | Arch_x86_64, "x86_64");
  DEF_REGISTER(      zmm1,   0x01 |  ZMMS |   ZMM | Arch_x86_64, "x86_64");
  DEF_REGISTER(      zmm2,   0x02 |  ZMMS |   ZMM | Arch_x86_64, "x86_64");
  DEF_REGISTER(      zmm3,   0x03 |  ZMMS |   ZMM | Arch_x86_64, "x86_64");
  DEF_REGISTER(      zmm4,   0x04 |  ZMMS |   ZMM | Arch_x86_64, "x86_64");
  DEF_REGISTER(      zmm5,   0x05 |  ZMMS |   ZMM | Arch_x86_64, "x86_64");
  DEF_REGISTER(      zmm6,   0x06 |  ZMMS |   ZMM | Arch_x86_64, "x86_64");
  DEF_REGISTER(      zmm7,   0x07 |  ZMMS |   ZMM | Arch_x86_64, "x86_64");
  DEF_REGISTER(      zmm8,   0x08 |  ZMMS |   ZMM | Arch_x86_64, "x86_64");
  DEF_REGISTER(      zmm9,   0x09 |  ZMMS |   ZMM | Arch_x86_64, "x86_64");
  DEF_REGISTER(     zmm10,   0x0A |  ZMMS |   ZMM | Arch_x86_64, "x86_64");
  DEF_REGISTER(     zmm11,   0x0B |  ZMMS |   ZMM | Arch_x86_64, "x86_64");
  DEF_REGISTER(     zmm12,   0x0C |  ZMMS |   ZMM | Arch_x86_64, "x86_64");
  DEF_REGISTER(     zmm13,   0x0D |  ZMMS |   ZMM | Arch_x86_64, "x86_64");
  DEF_REGISTER(     zmm14,   0x0E |  ZMMS |   ZMM | Arch_x86_64, "x86_64");
  DEF_REGISTER(     zmm15,   0x0F |  ZMMS |   ZMM | Arch_x86_64, "x86_64");
  DEF_REGISTER(     zmm16,   0x10 |  ZMMS |   ZMM | Arch_x86_64, "x86_64");
  DEF_REGISTER(     zmm17,   0x11 |  ZMMS |   ZMM | Arch_x86_64, "x86_64");
  DEF_REGISTER(     zmm18,   0x12 |  ZMMS |   ZMM | Arch_x86_64, "x86_64");
  DEF_REGISTER(     zmm19,   0x13 |  ZMMS |   ZMM | Arch_x86_64, "x86_64");
  DEF_REGISTER(     zmm20,   0x14 |  ZMMS |   ZMM | Arch_x86_64, "x86_64");
  DEF_REGISTER(     zmm21,   0x15 |  ZMMS |   ZMM | Arch_x86_64, "x86_64");
  DEF_REGISTER(     zmm22,   0x16 |  ZMMS |   ZMM | Arch_x86_64, "x86_64");
  DEF_REGISTER(     zmm23,   0x17 |  ZMMS |   ZMM | Arch_x86_64, "x86_64");
  DEF_REGISTER(     zmm24,   0x18 |  ZMMS |   ZMM | Arch_x86_64, "x86_64");
  DEF_REGISTER(     zmm25,   0x19 |  ZMMS |   ZMM | Arch_x86_64, "x86_64");
  DEF_REGISTER(     zmm26,   0x1A |  ZMMS |   ZMM | Arch_x86_64, "x86_64");
  DEF_REGISTER(     zmm27,   0x1B |  ZMMS |   ZMM | Arch_x86_64, "x86_64");
  DEF_REGISTER(     zmm28,   0x1C |  ZMMS |   ZMM | Arch_x86_64, "x86_64");
  DEF_REGISTER(     zmm29,   0x1D |  ZMMS |   ZMM | Arch_x86_64, "x86_64");
  DEF_REGISTER(     zmm30,   0x1E |  ZMMS |   ZMM | Arch_x86_64, "x86_64");
  DEF_REGISTER(     zmm31,   0x1F |  ZMMS |   ZMM | Arch_x86_64, "x86_64");

  DEF_REGISTER(        k0,   0x00 | KMSKS | KMASK | Arch_x86_64, "x86_64");
  DEF_REGISTER(        k1,   0x01 | KMSKS | KMASK | Arch_x86_64, "x86_64");
  DEF_REGISTER(        k2,   0x02 | KMSKS | KMASK | Arch_x86_64, "x86_64");
  DEF_REGISTER(        k3,   0x03 | KMSKS | KMASK | Arch_x86_64, "x86_64");
  DEF_REGISTER(        k4,   0x04 | KMSKS | KMASK | Arch_x86_64, "x86_64");
  DEF_REGISTER(        k5,   0x05 | KMSKS | KMASK | Arch_x86_64, "x86_64");
  DEF_REGISTER(        k6,   0x06 | KMSKS | KMASK | Arch_x86_64, "x86_64");
  DEF_REGISTER(        k7,   0x07 | KMSKS | KMASK | Arch_x86_64, "x86_64");


  /* Pseudo-registers for internal use only */
  DEF_REGISTER(      orax,    0x0 |  FULL |  MISC | Arch_x86_64, "x86_64");
  DEF_REGISTER(    fsbase,    0x1 |  FULL |  MISC | Arch_x86_64, "x86_64");
  DEF_REGISTER(    gsbase,    0x2 |  FULL |  MISC | Arch_x86_64, "x86_64");
  DEF_REGISTER(       tr0,    0x0 |  FULL |   TST | Arch_x86_64, "x86_64");
  DEF_REGISTER(       tr1,    0x1 |  FULL |   TST | Arch_x86_64, "x86_64");
  DEF_REGISTER(       tr2,    0x2 |  FULL |   TST | Arch_x86_64, "x86_64");
  DEF_REGISTER(       tr3,    0x3 |  FULL |   TST | Arch_x86_64, "x86_64");
  DEF_REGISTER(       tr4,    0x4 |  FULL |   TST | Arch_x86_64, "x86_64");
  DEF_REGISTER(       tr5,    0x5 |  FULL |   TST | Arch_x86_64, "x86_64");
  DEF_REGISTER(       tr6,    0x6 |  FULL |   TST | Arch_x86_64, "x86_64");
  DEF_REGISTER(       tr7,    0x7 |  FULL |   TST | Arch_x86_64, "x86_64");

}}

#endif
