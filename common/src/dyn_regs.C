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

#define DYN_DEFINE_REGS
#include "dynutil/h/dyn_regs.h"

#include "external/rose/rose-compat.h"

namespace Dyninst {

std::map<signed int, const char *> *Dyninst::MachRegister::names;

MachRegister::MachRegister() :
   reg(0)
{ 
}

MachRegister::MachRegister(signed int r) :
   reg(r)
{
}
 
MachRegister::MachRegister(const MachRegister& r) :
        reg(r.reg)
{
}

MachRegister::MachRegister(signed int r, const char *n) :
   reg(r)
{
   if (!names) {
      names = new std::map<signed int, const char *>();
   }
   (*names)[r] = n;
}

MachRegister::~MachRegister() {
}

MachRegister MachRegister::getBaseRegister() const { 
   switch (getArchitecture()) {
      case Arch_x86:
      case Arch_x86_64:
          return MachRegister(reg & 0xfffff0ff);
      case Arch_ppc32:
      case Arch_ppc64:
         return *this;
      case Arch_none:
         return *this;
   }
   return InvalidReg;
}
   
Architecture MachRegister::getArchitecture() const { 
   return (Architecture) (reg & 0xff000000);
}

bool MachRegister::isValid() const {
   return (reg != InvalidReg.reg);
}

MachRegisterVal MachRegister::getSubRegValue(const MachRegister subreg, 
                                             MachRegisterVal &orig) const
{
   if (subreg.reg == reg || 
       getArchitecture() == Arch_ppc32 ||
       getArchitecture() == Arch_ppc64)
      return orig;

   assert(subreg.getBaseRegister() == getBaseRegister());
   switch ((subreg.reg & 0x00000f00) >> 8) {
      case 0x0: return orig;
      case 0x1: return (orig & 0xff);
      case 0x2: return (orig & 0xff00) >> 8;              
      case 0x3: return (orig & 0xffff);
      case 0xf: return (orig & 0xffffffff);
      default: assert(0); return orig;
   }
}

const char *MachRegister::name() const { 
   return (*names)[reg];
}

unsigned int MachRegister::size() const {
   switch (getArchitecture())
   {
      case Arch_x86:
         switch (reg & 0x0000ff00) {
             case x86::L_REG: //L_REG
            case x86::H_REG: //H_REG
               return 1;
               case x86::W_REG: //W_REG
               return 2;
               case x86::FULL: //FULL
               return 4;
             case x86::QUAD:
                return 8;
             case x86::OCT:
                 return 16;
             case x86::FPDBL:
                 return 10;
             default:
               assert(0);
         }
      case Arch_x86_64:
         switch (reg & 0x0000ff00) {
             case x86_64::L_REG: //L_REG
             case x86_64::H_REG: //H_REG
                return 1;
             case x86_64::W_REG: //W_REG
                return 2;
             case x86_64::FULL: //FULL
                return 8;
             case x86_64::D_REG:
                 return 4;
             case x86_64::OCT:
                 return 16;
             case x86_64::FPDBL:
                 return 10;
             default:
               assert(0);
         }
      case Arch_ppc32: {
         int reg_class = reg & 0x00ff0000;
         if (reg_class == ppc32::FPR || reg_class == ppc32::FSR)
            return 8;
         return 4;
      }
      case Arch_ppc64:
         return 8;
      case Arch_none:
         return 0;
   }
   return 0; //Unreachable, but disable warnings
}
   
bool MachRegister::operator<(const MachRegister &a) const { 
   return (reg < a.reg);
}
 
bool MachRegister::operator==(const MachRegister &a) const { 
   return (reg == a.reg);
}
 
MachRegister::operator signed int() const {
   return reg;
}

signed int MachRegister::val() const {
   return reg;
}


MachRegister MachRegister::getPC(Dyninst::Architecture arch)
{
   switch (arch)
   {
      case Arch_x86:
         return x86::eip;
      case Arch_x86_64:
         return x86_64::rip;
      case Arch_ppc32:
         return ppc32::pc;
      case Arch_ppc64:
         return ppc64::pc;
      case Arch_none:
         return InvalidReg;
   }
   return InvalidReg;
}

MachRegister MachRegister::getFramePointer(Dyninst::Architecture arch)
{
   switch (arch)
   {
      case Arch_x86:
         return x86::ebp;
      case Arch_x86_64:
         return x86_64::rbp;
      case Arch_ppc32:
      case Arch_ppc64:
         return FrameBase;
      case Arch_none:
         return InvalidReg;
   }
   return InvalidReg;
}

MachRegister MachRegister::getStackPointer(Dyninst::Architecture arch)
{
   switch (arch)
   {
      case Arch_x86:
         return x86::esp;
      case Arch_x86_64:
         return x86_64::rsp;
      case Arch_ppc32:
         return ppc32::r1;
      case Arch_ppc64:
         return ppc64::r1;
      case Arch_none:
         return InvalidReg;
   }
   return InvalidReg;
}

bool MachRegister::isPC() const
{
   return (*this == x86_64::rip || *this == x86::eip ||
           *this == ppc32::pc || *this == ppc64::pc);
}

bool MachRegister::isFramePointer() const
{
   return (*this == x86_64::rbp || *this == x86::ebp ||
           *this == FrameBase);
}

bool MachRegister::isStackPointer() const
{
   return (*this == x86_64::rsp || *this == x86::esp ||
           *this == ppc32::r1 || *this == ppc64::r1);
}

void MachRegister::getROSERegister(int &c, int &n, int &p)
{
   // Rose: class, number, position
   // Dyninst: category, base id, subrange

   signed int category = (reg & 0x00ff0000);
   signed int subrange = (reg & 0x0000ff00);
   signed int baseID =   (reg & 0x000000ff);

   switch (getArchitecture()) {
      case Arch_x86:
      case Arch_x86_64: // 64-bit not supported in ROSE
         switch (category) {
            case x86::GPR:
               c = x86_regclass_gpr;
               switch (baseID) {
                  case x86::BASEA:
                     n = x86_gpr_ax;
                     break;
                  case x86::BASEC:
                     n = x86_gpr_cx;
                     break;
                  case x86::BASED:
                     n = x86_gpr_dx;
                     break;
                  case x86::BASEB:
                     n = x86_gpr_bx;
                     break;
                  case x86::BASESP:
                     n = x86_gpr_sp;
                     break;
                  case x86::BASEBP:
                     n = x86_gpr_bp;
                     break;
                  case x86::BASESI:
                     n = x86_gpr_si;
                     break;
                  case x86::BASEDI:
                     n = x86_gpr_di;
                     break;
                  default:
                     n = 0;
                     break;
               }
               break;
            case x86::SEG:
               c = x86_regclass_segment;
               switch (baseID) {
                  case 0x0:
                     n = x86_segreg_ds;
                     break;
                  case 0x1:
                     n = x86_segreg_es;
                     break;
                  case 0x2:
                     n = x86_segreg_fs;
                     break;
                  case 0x3:
                     n = x86_segreg_gs;
                     break;
                  case 0x4:
                     n = x86_segreg_cs;
                     break;
                  case 0x5:
                     n = x86_segreg_ss;
                     break;
                  default:
                     n = 0;
                     break;
               }
               break;
            case x86::FLAG:
               c = x86_regclass_flags;
               n = 0;
               break;
            case x86::MISC:
               c = x86_regclass_unknown;
               break;
            case x86::XMM:
               c = x86_regclass_xmm;
               n = baseID;
               break;
            case x86::MMX:
               c = x86_regclass_mm;
               n = baseID;
               break;
            case x86::CTL:
               c = x86_regclass_cr;
               n = baseID;
               break;
            case x86::DBG:
               c = x86_regclass_dr;
               n = baseID;
               break;
            case x86::TST:
               c = x86_regclass_unknown;
               break;
            case 0:
               switch (baseID) {
                  case 0x10:
                     c = x86_regclass_ip;
                     n = 0;
                     break;
                  default:
                     c = x86_regclass_unknown;
                     break;
               }
               break;
         }
         break;
      default:
         c = x86_regclass_unknown;
         n = 0;
         break;
   }

   switch (getArchitecture()) {
      case Arch_x86:
      case Arch_x86_64:
         switch (subrange) {
            case x86::FULL:
            case x86::OCT:
            case x86::FPDBL:
               p = x86_regpos_all;
               break;
            case x86::H_REG:
               p = x86_regpos_high_byte;
               break;
            case x86::L_REG:
               p = x86_regpos_low_byte;
               break;
            case x86::W_REG:
               p = x86_regpos_word;
               break;
            case x86_64::D_REG:
               p = x86_regpos_dword;
               break;
         }
         break;
      default:
        p = x86_regpos_unknown;
   }
}

}
