/*
 * Copyright (c) 1996-2009 Barton P. Miller
 * 
 * We provide the Paradyn Parallel Performance Tools (below
 * described as "Paradyn") on an AS IS basis, and do not warrant its
 * validity or performance.  We reserve the right to update, modify,
 * or discontinue this software at any time.  We shall have no
 * obligation to supply such updates or modifications or any other
 * form of support to you.
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

#if !defined(DYN_REGS_H)
#define DYN_REGS_H
 
namespace Dyninst
{
   typedef signed int MachRegister;        //Values below
   const signed int MachRegInvalid = -1;
   const signed int MachRegReturn = -2;    //Virtual register on some systems
   const signed int MachRegFrameBase = -3; //Virtual register on some systems
   const signed int MachRegPC = -4;
   const signed int MachRegStackBase = -5; //Virtual register on some systems
   
   namespace x86
   {
      const signed int L_REG = 0x10; //8-bit, first byte
      const signed int H_REG = 0x20; //8-bit, second byte
      const signed int W_REG = (H_REG | L_REG); //16-bit, first word

      const signed int EAX = 0x0;
      const signed int ECX = 0x1;
      const signed int EDX = 0x2;
      const signed int EBX = 0x3;
      const signed int ESP = 0x4;
      const signed int EBP = 0x5;
      const signed int ESI = 0x6;
      const signed int EDI = 0x7;
      const signed int AH  = (H_REG | EAX);
      const signed int AL  = (L_REG | EAX);
      const signed int AX  = (W_REG | EAX);
      const signed int CH  = (H_REG | ECX);
      const signed int CL  = (L_REG | ECX);
      const signed int CX  = (W_REG | ECX);
      const signed int DH  = (H_REG | EDX);
      const signed int DL  = (L_REG | EDX);
      const signed int DX  = (W_REG | EDX);
      const signed int BH  = (H_REG | EBX);
      const signed int BL  = (L_REG | EBX);
      const signed int BX  = (W_REG | EBX);
   }
   namespace x86_64
   {
      const signed int L_REG = 0x10;  //8-bit, first byte
      const signed int H_REG = 0x20;  //8-bit, second byte
      const signed int W_REG = (H_REG | L_REG); //16 bit, first work
      const signed int D_REG = 0x40; //32 bit, first double word

      const signed int RAX = 0x0;
      const signed int RCX = 0x1;
      const signed int RDX = 0x2;
      const signed int RBX = 0x3;
      const signed int RSP = 0x4;
      const signed int RBP = 0x5;
      const signed int RSI = 0x6;
      const signed int RDI = 0x7;
      const signed int R8  = 0x8;
      const signed int R9  = 0x9;
      const signed int R10 = 0xa;
      const signed int R11 = 0xb;
      const signed int R12 = 0xc;
      const signed int R13 = 0xd;
      const signed int R14 = 0xe;
      const signed int R15 = 0xf;
      const signed int AH  = (H_REG | RAX);
      const signed int AL  = (L_REG | RAX);
      const signed int AX  = (W_REG | RAX);
      const signed int EAX = (D_REG | RAX);
      const signed int CH  = (H_REG | RCX);
      const signed int CL  = (L_REG | RCX);
      const signed int CX  = (W_REG | RCX);
      const signed int ECX = (D_REG | RCX);
      const signed int DH  = (H_REG | RDX);
      const signed int DL  = (L_REG | RDX);
      const signed int DX  = (W_REG | RDX);
      const signed int EDX = (D_REG | RDX);
      const signed int BH  = (H_REG | RBX);
      const signed int BL  = (L_REG | RBX);
      const signed int BX  = (W_REG | RBX);
      const signed int EBX = (D_REG | RBX);
      const signed int SPL = (L_REG | RSP);
      const signed int SP  = (W_REG | RSP);
      const signed int ESP = (D_REG | RSP);
      const signed int BPL = (L_REG | RBP);
      const signed int BP  = (W_REG | RBP);
      const signed int EBP = (D_REG | RBP);
      const signed int DIL = (L_REG | RDI);
      const signed int DI  = (W_REG | RDI);
      const signed int SIL = (L_REG | RSI);
      const signed int SI  = (W_REG | RSI);
      const signed int R8B = (L_REG | R8);
      const signed int R8W = (W_REG | R8);
      const signed int R8D = (D_REG | R8);
      const signed int R9B = (L_REG | R9);
      const signed int R9W = (W_REG | R9);
      const signed int R9D = (D_REG | R9);
      const signed int R10B = (L_REG | R10);
      const signed int R10W = (W_REG | R10);
      const signed int R10D = (D_REG | R10);
      const signed int R11B = (L_REG | R11);
      const signed int R11W = (W_REG | R11);
      const signed int R11D = (D_REG | R11);
      const signed int R12B = (L_REG | R12);
      const signed int R12W = (W_REG | R12);
      const signed int R12D = (D_REG | R12);
      const signed int R13B = (L_REG | R13);
      const signed int R13W = (W_REG | R13);
      const signed int R13D = (D_REG | R13);
      const signed int R14B = (L_REG | R14);
      const signed int R14W = (W_REG | R14);
      const signed int R14D = (D_REG | R14);
      const signed int R15B = (L_REG | R15);
      const signed int R15W = (W_REG | R15);
      const signed int R15D = (D_REG | R15);
         
   }
   
#define GPR(x) const signed int gpr##x = (x | GPR_BASE | ARCH_POWER);
#define FPR(x) const signed int fpr##x = (x | FPR_BASE | ARCH_POWER);
#define FSR(x) const signed int fsr##x = (x | FPR2_BASE | ARCH_POWER);
#define SPR(x, y) const signed int spr##y = (x | SPR_BASE | ARCH_POWER);
   
namespace power
   {
       const signed int GPR_BASE = 1 << 6;
       const signed int FPR_BASE = 1 << 7;
       const signed int FPR2_BASE = 1 << 8;
       const signed int SPR_BASE = 1 << 9;
       const signed int ARCH_POWER = 0;
       GPR(0);
       GPR(1);
       GPR(2);
       GPR(3);
       GPR(4);
       GPR(5);
       GPR(6);
       GPR(7);
       GPR(8);
       GPR(9);
       GPR(10);
       GPR(11);
       GPR(12);
       GPR(13);
       GPR(14);
       GPR(15);
       GPR(16);
       GPR(17);
       GPR(18);
       GPR(19);
       GPR(20);
       GPR(21);
       GPR(22);
       GPR(23);
       GPR(24);
       GPR(25);
       GPR(26);
       GPR(27);
       GPR(28);
       GPR(29);
       GPR(30);
       GPR(31);
       FPR(0);
       FPR(1);
       FPR(2);
       FPR(3);
       FPR(4);
       FPR(5);
       FPR(6);
       FPR(7);
       FPR(8);
       FPR(9);
       FPR(10);
       FPR(11);
       FPR(12);
       FPR(13);
       FPR(14);
       FPR(15);
       FPR(16);
       FPR(17);
       FPR(18);
       FPR(19);
       FPR(20);
       FPR(21);
       FPR(22);
       FPR(23);
       FPR(24);
       FPR(25);
       FPR(26);
       FPR(27);
       FPR(28);
       FPR(29);
       FPR(30);
       FPR(31);
       FSR(0);
       FSR(1);
       FSR(2);
       FSR(3);
       FSR(4);
       FSR(5);
       FSR(6);
       FSR(7);
       FSR(8);
       FSR(9);
       FSR(10);
       FSR(11);
       FSR(12);
       FSR(13);
       FSR(14);
       FSR(15);
       FSR(16);
       FSR(17);
       FSR(18);
       FSR(19);
       FSR(20);
       FSR(21);
       FSR(22);
       FSR(23);
       FSR(24);
       FSR(25);
       FSR(26);
       FSR(27);
       FSR(28);
       FSR(29);
       FSR(30);
       FSR(31);
       SPR(0, mq);
       SPR(1, xer);
       SPR(8, lr);
       SPR(9, ctr);
       SPR(18, dsisr);
       SPR(19, dar);
       SPR(22, dec);
       SPR(25, sdr1);
       SPR(26, srr0);
       SPR(27, srr1);
       SPR(272, sprg0);
       SPR(273, sprg1);
       SPR(274, sprg2);
       SPR(275, sprg3);
       SPR(282, ear);
       SPR(284, tbl);
       SPR(285, tbu);
       SPR(287, pvr);
       SPR(528, ibat0u);
       SPR(529, ibat0l);
       SPR(530, ibat1u);
       SPR(531, ibat1l);
       SPR(532, ibat2u);
       SPR(533, ibat2l);
       SPR(534, ibat3u);
       SPR(535, ibat3l);
       SPR(536, dbat0u);
       SPR(537, dbat0l);
       SPR(538, dbat1u);
       SPR(539, dbat1l);
       SPR(540, dbat2u);
       SPR(541, dbat2l);
       SPR(542, dbat3u);
       SPR(543, dbat3l);
       SPR(600, pc);
       SPR(601, fpscw);
       SPR(602, fpscw0);
       SPR(603, fpscw1);
       SPR(604, fpscw2);
       SPR(605, fpscw3);
       SPR(606, fpscw4);
       SPR(607, fpscw5);
       SPR(608, fpscw6);
       SPR(609, fpscw7);
       SPR(610, msr);
       SPR(611, ivpr);
       SPR(612, ivor8);
       SPR(613, seg0);
       SPR(614, seg1);
       SPR(615, seg2);
       SPR(616, seg3);
       SPR(617, seg4);
       SPR(618, seg5);
       SPR(619, seg6);
       SPR(620, seg7);
       SPR(621, cr0);
       SPR(622, cr1);
       SPR(623, cr2);
       SPR(624, cr3);
       SPR(625, cr4);
       SPR(626, cr5);
       SPR(627, cr6);
       SPR(628, cr7);
       SPR(629, cr);
       enum powerXERBits
       {
           OF_bit,
           SOF_bit,
           CF_bit,
           BYTECNT_low_bit,
           BYTECNT_high_bit,
           last_power_xer_bit_id
       };
   };

   typedef unsigned long MachRegisterVal;
#undef GPR
#undef FPR
#undef FSR
#undef SPR
}


#endif // !defined(DYN_REGS_H)
