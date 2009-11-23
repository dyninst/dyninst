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
   typedef unsigned long MachRegisterVal;

}
