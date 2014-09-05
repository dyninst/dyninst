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

#define DYN_DEFINE_REGS
#include "common/h/dyn_regs.h"

#include "external/rose/rose-compat.h"
#include "external/rose/powerpcInstructionEnum.h"

#include <iostream>

using namespace Dyninst;

boost::shared_ptr<MachRegister::NameMap> MachRegister::names()
{
    static boost::shared_ptr<MachRegister::NameMap> store = 
       boost::shared_ptr<MachRegister::NameMap>(new MachRegister::NameMap);
    return store;
}

MachRegister::MachRegister() :
   reg(0)
{ 
}

MachRegister::MachRegister(signed int r) :
   reg(r)
{
}
 
MachRegister::MachRegister(signed int r, const char *n) :
   reg(r)
{
	(*names())[r] = std::string(n);
}

MachRegister::MachRegister(signed int r, std::string n) :
reg(r)
{
	(*names())[r] = n;
}

unsigned int MachRegister::regClass() const
{
    return reg & 0x00ff0000;
}

MachRegister MachRegister::getBaseRegister() const { 
   switch (getArchitecture()) {
      case Arch_x86:
         if (reg & x86::GPR) return MachRegister(reg & 0xfffff0ff);
         else return *this;
      case Arch_x86_64:
         if (reg & x86_64::GPR) return MachRegister(reg & 0xfffff0ff);
         else return *this;
      case Arch_ppc32:
      case Arch_ppc64:
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

MachRegisterVal MachRegister::getSubRegValue(const MachRegister& subreg, 
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

std::string MachRegister::name() const { 
	assert(names() != NULL);
	NameMap::const_iterator iter = names()->find(reg);
	if (iter != names()->end()) {
		return iter->second;
	}
	return std::string("<INVALID_REG>");
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
            case x86::BIT:
               return 0;
            default:
               return 0;//KEVINTODO: removed sanity-check assert because of asprotect fuzz testing, could use this as a sign that the parse has gone into junk
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
            case x86_64::BIT:
               return 0;
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
         return ppc32::r1;
      case Arch_ppc64:
         return ppc64::r1;
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

MachRegister MachRegister::getSyscallNumberReg(Dyninst::Architecture arch)
{
    switch (arch)
    {
        case Arch_x86:
            return x86::oeax;
        case Arch_x86_64:
            return x86_64::orax;
        case Arch_ppc32: 
            return ppc32::r0;
        case Arch_ppc64:
            return ppc64::r0;
        case Arch_none:
            return InvalidReg;
    }
    return InvalidReg;
}

MachRegister MachRegister::getSyscallReturnValueReg(Dyninst::Architecture arch)
{
    switch (arch)
    {
        case Arch_x86:
            return x86::eax;
        case Arch_x86_64:
            return x86_64::rax;
        case Arch_ppc32: 
            return ppc32::r3;
        case Arch_ppc64:
            return ppc64::r3;
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

bool MachRegister::isSyscallNumberReg() const
{
    return (*this == x86_64::orax || *this == x86::oeax ||
            *this == ppc32::r1 || *this == ppc64::r1);
}

bool MachRegister::isSyscallReturnValueReg() const
{
    return (*this == x86_64::rax || *this == x86::eax ||
            *this == ppc32::r1 || *this == ppc64::r1);
}

COMMON_EXPORT bool Dyninst::isSegmentRegister(int regClass)
{
   return 0 != (regClass & x86::SEG);
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
	       switch(baseID) {
	       case x86::CF:
		 n = x86_flag_cf;
		 break;
	       case x86::PF:
		 n = x86_flag_pf;
		 break;
	       case x86::AF:
		 n = x86_flag_af;
		 break;
	       case x86::ZF:
		 n = x86_flag_zf;
		 break;
	       case x86::SF:
		 n = x86_flag_sf;
		 break;
	       case x86::TF:
		 n = x86_flag_tf;
		 break;
	       case x86::IF:
		 n = x86_flag_if;
		 break;
	       case x86::DF:
		 n = x86_flag_df;
		 break;
	       case x86::OF:
		 n = x86_flag_of;
		 break;
	       default:
		 assert(0);
		 break;
	       }
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
       case Arch_ppc32:
       case Arch_ppc64: // 64-bit not supported in ROSE
       {
	 baseID = reg & 0x0000FFFF;
           n = baseID;
           switch(category)
           {
               case ppc32::GPR:
                   c = powerpc_regclass_gpr;
                   break;
               case ppc32::FPR:
               case ppc32::FSR:
                   c = powerpc_regclass_fpr;
                   break;
               case ppc32::SPR:
               {
                   if(baseID < 613) {
                       c = powerpc_regclass_spr;
                   } else if(baseID < 621 ) {
                       c = powerpc_regclass_sr; 
                   } else {
                       c = powerpc_regclass_cr;
                       n = baseID - 621;
		       if(n > 7) {
			 n = 0;
			 p = powerpc_condreggranularity_whole;
		       } else {
			 p = powerpc_condreggranularity_field;
		       }

                   }
               }
               break;
               default:
                   assert(!"unknown register type!");
                   break;
           }
           return;
       }
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
	    case x86::BIT:
     	       p = x86_regpos_all;
	       break;
         }
         break;
      default:
        p = x86_regpos_unknown;
   }
}

MachRegister MachRegister::DwarfEncToReg(int encoding, Dyninst::Architecture arch)
{
   switch (arch)
   {
      case Arch_x86:
         switch (encoding) {
            case 0: return Dyninst::x86::eax;
            case 1: return Dyninst::x86::ecx;
            case 2: return Dyninst::x86::edx;
            case 3: return Dyninst::x86::ebx;
            case 4: return Dyninst::x86::esp;
            case 5: return Dyninst::x86::ebp;
            case 6: return Dyninst::x86::esi;
            case 7: return Dyninst::x86::edi;
            case 8: return Dyninst::x86::eip;
            case 9: return Dyninst::x86::flags;
            case 10: return Dyninst::InvalidReg;
            case 11: return Dyninst::x86::st0;
            case 12: return Dyninst::x86::st1;
            case 13: return Dyninst::x86::st2;
            case 14: return Dyninst::x86::st3;
            case 15: return Dyninst::x86::st4;
            case 16: return Dyninst::x86::st5;
            case 17: return Dyninst::x86::st6;
            case 18: return Dyninst::x86::st7;
            case 19: return Dyninst::InvalidReg;
            case 20: return Dyninst::InvalidReg;
            case 21: return Dyninst::x86::xmm0;
            case 22: return Dyninst::x86::xmm1;
            case 23: return Dyninst::x86::xmm2;
            case 24: return Dyninst::x86::xmm3;
            case 25: return Dyninst::x86::xmm4;
            case 26: return Dyninst::x86::xmm5;
            case 27: return Dyninst::x86::xmm6;
            case 28: return Dyninst::x86::xmm7;
            case 29: return Dyninst::x86::mm0;
            case 30: return Dyninst::x86::mm1;
            case 31: return Dyninst::x86::mm2;
            case 32: return Dyninst::x86::mm3;
            case 33: return Dyninst::x86::mm4;
            case 34: return Dyninst::x86::mm5;
            case 35: return Dyninst::x86::mm6;
            case 36: return Dyninst::x86::mm7;
            case 37: return Dyninst::InvalidReg; //fcw
            case 38: return Dyninst::InvalidReg; //fsw
            case 39: return Dyninst::InvalidReg; //mxcsr
            case 40: return Dyninst::x86::es;
            case 41: return Dyninst::x86::cs;
            case 42: return Dyninst::x86::ss;
            case 43: return Dyninst::x86::ds;
            case 44: return Dyninst::x86::fs;
            case 45: return Dyninst::x86::gs;
            case 46: return Dyninst::InvalidReg;
            case 47: return Dyninst::InvalidReg;
            case 48: return Dyninst::InvalidReg; //tr
            case 49: return Dyninst::InvalidReg; //ldtr
            default: return Dyninst::InvalidReg;
         }
         break;
      case Arch_x86_64:
         switch (encoding) {
            case 0: return Dyninst::x86_64::rax;
            case 1: return Dyninst::x86_64::rdx;
            case 2: return Dyninst::x86_64::rcx;
            case 3: return Dyninst::x86_64::rbx;
            case 4: return Dyninst::x86_64::rsi;
            case 5: return Dyninst::x86_64::rdi;
            case 6: return Dyninst::x86_64::rbp;
            case 7: return Dyninst::x86_64::rsp;
            case 8: return Dyninst::x86_64::r8;
            case 9: return Dyninst::x86_64::r9;
            case 10: return Dyninst::x86_64::r10;
            case 11: return Dyninst::x86_64::r11;
            case 12: return Dyninst::x86_64::r12;
            case 13: return Dyninst::x86_64::r13;
            case 14: return Dyninst::x86_64::r14;
            case 15: return Dyninst::x86_64::r15;
            case 16: return Dyninst::x86_64::rip;
            case 17: return Dyninst::x86_64::xmm0;
            case 18: return Dyninst::x86_64::xmm1;
            case 19: return Dyninst::x86_64::xmm2;
            case 20: return Dyninst::x86_64::xmm3;
            case 21: return Dyninst::x86_64::xmm4;
            case 22: return Dyninst::x86_64::xmm5;
            case 23: return Dyninst::x86_64::xmm6;
            case 24: return Dyninst::x86_64::xmm7;
            case 25: return Dyninst::x86_64::xmm8;
            case 26: return Dyninst::x86_64::xmm9;
            case 27: return Dyninst::x86_64::xmm10;
            case 28: return Dyninst::x86_64::xmm11;
            case 29: return Dyninst::x86_64::xmm12;
            case 30: return Dyninst::x86_64::xmm13;
            case 31: return Dyninst::x86_64::xmm14;
            case 32: return Dyninst::x86_64::xmm15;
            case 33: return Dyninst::x86_64::st0;
            case 34: return Dyninst::x86_64::st1;
            case 35: return Dyninst::x86_64::st2;
            case 36: return Dyninst::x86_64::st3;
            case 37: return Dyninst::x86_64::st4;
            case 38: return Dyninst::x86_64::st5;
            case 39: return Dyninst::x86_64::st6;
            case 40: return Dyninst::x86_64::st7;
            case 41: return Dyninst::x86_64::mm0;
            case 42: return Dyninst::x86_64::mm1;
            case 43: return Dyninst::x86_64::mm2;
            case 44: return Dyninst::x86_64::mm3;
            case 45: return Dyninst::x86_64::mm4;
            case 46: return Dyninst::x86_64::mm5;
            case 47: return Dyninst::x86_64::mm6;
            case 48: return Dyninst::x86_64::mm7;
            case 49: return Dyninst::x86_64::flags;
            case 50: return Dyninst::x86_64::es;
            case 51: return Dyninst::x86_64::cs;
            case 52: return Dyninst::x86_64::ss;
            case 53: return Dyninst::x86_64::ds;
            case 54: return Dyninst::x86_64::fs;
            case 55: return Dyninst::x86_64::gs;
            case 56: return Dyninst::InvalidReg;
            case 57: return Dyninst::InvalidReg;
            case 58: return Dyninst::x86_64::fsbase;
            case 59: return Dyninst::x86_64::gsbase;
            case 60: return Dyninst::InvalidReg; 
            case 61: return Dyninst::InvalidReg; 
            case 62: return Dyninst::InvalidReg; //tr
            case 63: return Dyninst::InvalidReg; //ldtr
            case 64: return Dyninst::InvalidReg; //mxcsr
            case 65: return Dyninst::InvalidReg; //fcw
            case 66: return Dyninst::InvalidReg; //fsw
         }
         break;
      case Arch_ppc32:
         switch (encoding) {
            case 0: return Dyninst::ppc32::r0;
            case 1: return Dyninst::ppc32::r1;
            case 2: return Dyninst::ppc32::r2;
            case 3: return Dyninst::ppc32::r3;
            case 4: return Dyninst::ppc32::r4;
            case 5: return Dyninst::ppc32::r5;
            case 6: return Dyninst::ppc32::r6;
            case 7: return Dyninst::ppc32::r7;
            case 8: return Dyninst::ppc32::r8;
            case 9: return Dyninst::ppc32::r9;
            case 10: return Dyninst::ppc32::r10;
            case 11: return Dyninst::ppc32::r11;
            case 12: return Dyninst::ppc32::r12;
            case 13: return Dyninst::ppc32::r13;
            case 14: return Dyninst::ppc32::r14;
            case 15: return Dyninst::ppc32::r15;
            case 16: return Dyninst::ppc32::r16;
            case 17: return Dyninst::ppc32::r17;
            case 18: return Dyninst::ppc32::r18;
            case 19: return Dyninst::ppc32::r19;
            case 20: return Dyninst::ppc32::r20;
            case 21: return Dyninst::ppc32::r21;
            case 22: return Dyninst::ppc32::r22;
            case 23: return Dyninst::ppc32::r23;
            case 24: return Dyninst::ppc32::r24;
            case 25: return Dyninst::ppc32::r25;
            case 26: return Dyninst::ppc32::r26;
            case 27: return Dyninst::ppc32::r27;
            case 28: return Dyninst::ppc32::r28;
            case 29: return Dyninst::ppc32::r29;
            case 30: return Dyninst::ppc32::r30;
            case 31: return Dyninst::ppc32::r31;
            case 32: return Dyninst::ppc32::fpr0;
            case 33: return Dyninst::ppc32::fpr1;
            case 34: return Dyninst::ppc32::fpr2;
            case 35: return Dyninst::ppc32::fpr3;
            case 36: return Dyninst::ppc32::fpr4;
            case 37: return Dyninst::ppc32::fpr5;
            case 38: return Dyninst::ppc32::fpr6;
            case 39: return Dyninst::ppc32::fpr7;
            case 40: return Dyninst::ppc32::fpr8;
            case 41: return Dyninst::ppc32::fpr9;
            case 42: return Dyninst::ppc32::fpr10;
            case 43: return Dyninst::ppc32::fpr11;
            case 44: return Dyninst::ppc32::fpr12;
            case 45: return Dyninst::ppc32::fpr13;
            case 46: return Dyninst::ppc32::fpr14;
            case 47: return Dyninst::ppc32::fpr15;
            case 48: return Dyninst::ppc32::fpr16;
            case 49: return Dyninst::ppc32::fpr17;
            case 50: return Dyninst::ppc32::fpr18;
            case 51: return Dyninst::ppc32::fpr19;
            case 52: return Dyninst::ppc32::fpr20;
            case 53: return Dyninst::ppc32::fpr21;
            case 54: return Dyninst::ppc32::fpr22;
            case 55: return Dyninst::ppc32::fpr23;
            case 56: return Dyninst::ppc32::fpr24;
            case 57: return Dyninst::ppc32::fpr25;
            case 58: return Dyninst::ppc32::fpr26;
            case 59: return Dyninst::ppc32::fpr27;
            case 60: return Dyninst::ppc32::fpr28;
            case 61: return Dyninst::ppc32::fpr29;
            case 62: return Dyninst::ppc32::fpr30;
            case 63: return Dyninst::ppc32::fpr31;
            case 64: return Dyninst::ppc32::cr;
            case 65: return Dyninst::InvalidReg; //FPSCR
         }
         //Seperate switch statements to give compilers an easier time of 
         // optimizing
         switch (encoding) {
            case 100: return Dyninst::ppc32::mq;
            case 101: return Dyninst::ppc32::xer;
            case 102: return Dyninst::InvalidReg;
            case 103: return Dyninst::InvalidReg;
            case 104: return Dyninst::InvalidReg; //RTCU
            case 105: return Dyninst::InvalidReg; //RTCL
            case 106: return Dyninst::InvalidReg;
            case 107: return Dyninst::InvalidReg;
            case 108: return Dyninst::ppc32::lr;
            case 109: return Dyninst::ppc32::ctr;
            default: return Dyninst::InvalidReg;
         }
         break;
      case Arch_ppc64:
         switch (encoding) {
            case 0: return Dyninst::ppc64::r0;
            case 1: return Dyninst::ppc64::r1;
            case 2: return Dyninst::ppc64::r2;
            case 3: return Dyninst::ppc64::r3;
            case 4: return Dyninst::ppc64::r4;
            case 5: return Dyninst::ppc64::r5;
            case 6: return Dyninst::ppc64::r6;
            case 7: return Dyninst::ppc64::r7;
            case 8: return Dyninst::ppc64::r8;
            case 9: return Dyninst::ppc64::r9;
            case 10: return Dyninst::ppc64::r10;
            case 11: return Dyninst::ppc64::r11;
            case 12: return Dyninst::ppc64::r12;
            case 13: return Dyninst::ppc64::r13;
            case 14: return Dyninst::ppc64::r14;
            case 15: return Dyninst::ppc64::r15;
            case 16: return Dyninst::ppc64::r16;
            case 17: return Dyninst::ppc64::r17;
            case 18: return Dyninst::ppc64::r18;
            case 19: return Dyninst::ppc64::r19;
            case 20: return Dyninst::ppc64::r20;
            case 21: return Dyninst::ppc64::r21;
            case 22: return Dyninst::ppc64::r22;
            case 23: return Dyninst::ppc64::r23;
            case 24: return Dyninst::ppc64::r24;
            case 25: return Dyninst::ppc64::r25;
            case 26: return Dyninst::ppc64::r26;
            case 27: return Dyninst::ppc64::r27;
            case 28: return Dyninst::ppc64::r28;
            case 29: return Dyninst::ppc64::r29;
            case 30: return Dyninst::ppc64::r30;
            case 31: return Dyninst::ppc64::r31;
            case 32: return Dyninst::ppc64::fpr0;
            case 33: return Dyninst::ppc64::fpr1;
            case 34: return Dyninst::ppc64::fpr2;
            case 35: return Dyninst::ppc64::fpr3;
            case 36: return Dyninst::ppc64::fpr4;
            case 37: return Dyninst::ppc64::fpr5;
            case 38: return Dyninst::ppc64::fpr6;
            case 39: return Dyninst::ppc64::fpr7;
            case 40: return Dyninst::ppc64::fpr8;
            case 41: return Dyninst::ppc64::fpr9;
            case 42: return Dyninst::ppc64::fpr10;
            case 43: return Dyninst::ppc64::fpr11;
            case 44: return Dyninst::ppc64::fpr12;
            case 45: return Dyninst::ppc64::fpr13;
            case 46: return Dyninst::ppc64::fpr14;
            case 47: return Dyninst::ppc64::fpr15;
            case 48: return Dyninst::ppc64::fpr16;
            case 49: return Dyninst::ppc64::fpr17;
            case 50: return Dyninst::ppc64::fpr18;
            case 51: return Dyninst::ppc64::fpr19;
            case 52: return Dyninst::ppc64::fpr20;
            case 53: return Dyninst::ppc64::fpr21;
            case 54: return Dyninst::ppc64::fpr22;
            case 55: return Dyninst::ppc64::fpr23;
            case 56: return Dyninst::ppc64::fpr24;
            case 57: return Dyninst::ppc64::fpr25;
            case 58: return Dyninst::ppc64::fpr26;
            case 59: return Dyninst::ppc64::fpr27;
            case 60: return Dyninst::ppc64::fpr28;
            case 61: return Dyninst::ppc64::fpr29;
            case 62: return Dyninst::ppc64::fpr30;
            case 63: return Dyninst::ppc64::fpr31;
            case 64: return Dyninst::ppc64::cr;
            case 65: return Dyninst::InvalidReg; //FPSCR
         }
         //Seperate switch statements to give compilers an easier time of 
         // optimizing
         switch (encoding) {
            case 100: return Dyninst::ppc64::mq;
            case 101: return Dyninst::ppc64::xer;
            case 102: return Dyninst::InvalidReg;
            case 103: return Dyninst::InvalidReg;
            case 104: return Dyninst::InvalidReg; //RTCU
            case 105: return Dyninst::InvalidReg; //RTCL
            case 106: return Dyninst::InvalidReg;
            case 107: return Dyninst::InvalidReg;
            case 108: return Dyninst::ppc64::lr;
            case 109: return Dyninst::ppc64::ctr;
            default: return Dyninst::InvalidReg;
         }
         break;
      case Arch_none:
         return Dyninst::InvalidReg;
         break;
   }
   //Invalid Architecture passed
   return Dyninst::InvalidReg;

}

int MachRegister::getDwarfEnc() const
{
   switch (getArchitecture())
   {
      case Arch_x86:
         switch (val()) {
            case Dyninst::x86::ieax: return 0;
            case Dyninst::x86::iecx: return 1;
            case Dyninst::x86::iedx: return 2;
            case Dyninst::x86::iebx: return 3;
            case Dyninst::x86::iesp: return 4;
            case Dyninst::x86::iebp: return 5;
            case Dyninst::x86::iesi: return 6;
            case Dyninst::x86::iedi: return 7;
            case Dyninst::x86::ieip: return 8;
            case Dyninst::x86::iflags: return 9;
            case Dyninst::x86::ixmm0: return 21;
            case Dyninst::x86::ixmm1: return 22;
            case Dyninst::x86::ixmm2: return 23;
            case Dyninst::x86::ixmm3: return 24;
            case Dyninst::x86::ixmm4: return 25;
            case Dyninst::x86::ixmm5: return 26;
            case Dyninst::x86::ixmm6: return 27;
            case Dyninst::x86::ixmm7: return 28;
            case Dyninst::x86::imm0: return 29;
            case Dyninst::x86::imm1: return 30;
            case Dyninst::x86::imm2: return 31;
            case Dyninst::x86::imm3: return 32;
            case Dyninst::x86::imm4: return 33;
            case Dyninst::x86::imm5: return 34;
            case Dyninst::x86::imm6: return 35;
            case Dyninst::x86::imm7: return 36;
            case Dyninst::x86::ies: return 40;
            case Dyninst::x86::ics: return 41;
            case Dyninst::x86::iss: return 42;
            case Dyninst::x86::ids: return 43;
            case Dyninst::x86::ifs: return 44;
            case Dyninst::x86::igs: return 45;
            default: return -1;
         }
         break;
      case Arch_x86_64:
         switch (val()) {
            case Dyninst::x86_64::irax: return 0;
            case Dyninst::x86_64::irdx: return 1;
            case Dyninst::x86_64::ircx: return 2;
            case Dyninst::x86_64::irbx: return 3;
            case Dyninst::x86_64::irsi: return 4;
            case Dyninst::x86_64::irdi: return 5;
            case Dyninst::x86_64::irbp: return 6;
            case Dyninst::x86_64::irsp: return 7;
            case Dyninst::x86_64::ir8: return 8;
            case Dyninst::x86_64::ir9: return 9;
            case Dyninst::x86_64::ir10: return 10;
            case Dyninst::x86_64::ir11: return 11;
            case Dyninst::x86_64::ir12: return 12;
            case Dyninst::x86_64::ir13: return 13;
            case Dyninst::x86_64::ir14: return 14;
            case Dyninst::x86_64::ir15: return 15;
            case Dyninst::x86_64::irip: return 16;
            case Dyninst::x86_64::ixmm0: return 17;
            case Dyninst::x86_64::ixmm1: return 18;
            case Dyninst::x86_64::ixmm2: return 19;
            case Dyninst::x86_64::ixmm3: return 20;
            case Dyninst::x86_64::ixmm4: return 21;
            case Dyninst::x86_64::ixmm5: return 22;
            case Dyninst::x86_64::ixmm6: return 23;
            case Dyninst::x86_64::ixmm7: return 24;
            case Dyninst::x86_64::ixmm8: return 25;
            case Dyninst::x86_64::ixmm9: return 26;
            case Dyninst::x86_64::ixmm10: return 27;
            case Dyninst::x86_64::ixmm11: return 28;
            case Dyninst::x86_64::ixmm12: return 29;
            case Dyninst::x86_64::ixmm13: return 30;
            case Dyninst::x86_64::ixmm14: return 31;
            case Dyninst::x86_64::ixmm15: return 32;
            case Dyninst::x86_64::imm0: return 41;
            case Dyninst::x86_64::imm1: return 42;
            case Dyninst::x86_64::imm2: return 43;
            case Dyninst::x86_64::imm3: return 44;
            case Dyninst::x86_64::imm4: return 45;
            case Dyninst::x86_64::imm5: return 46;
            case Dyninst::x86_64::imm6: return 47;
            case Dyninst::x86_64::imm7: return 48;
            case Dyninst::x86_64::iflags: return 49;
            case Dyninst::x86_64::ies: return 50;
            case Dyninst::x86_64::ics: return 51;
            case Dyninst::x86_64::iss: return 52;
            case Dyninst::x86_64::ids: return 53;
            case Dyninst::x86_64::ifs: return 54;
            case Dyninst::x86_64::igs: return 55;
            case Dyninst::x86_64::ifsbase: return 58;
            case Dyninst::x86_64::igsbase: return 59;
            default: return -1;
         }
         break;
      case Arch_ppc32:
         switch (val()) {
            case Dyninst::ppc32::ir0: return 0;
            case Dyninst::ppc32::ir1: return 1;
            case Dyninst::ppc32::ir2: return 2;
            case Dyninst::ppc32::ir3: return 3;
            case Dyninst::ppc32::ir4: return 4;
            case Dyninst::ppc32::ir5: return 5;
            case Dyninst::ppc32::ir6: return 6;
            case Dyninst::ppc32::ir7: return 7;
            case Dyninst::ppc32::ir8: return 8;
            case Dyninst::ppc32::ir9: return 9;
            case Dyninst::ppc32::ir10: return 10;
            case Dyninst::ppc32::ir11: return 11;
            case Dyninst::ppc32::ir12: return 12;
            case Dyninst::ppc32::ir13: return 13;
            case Dyninst::ppc32::ir14: return 14;
            case Dyninst::ppc32::ir15: return 15;
            case Dyninst::ppc32::ir16: return 16;
            case Dyninst::ppc32::ir17: return 17;
            case Dyninst::ppc32::ir18: return 18;
            case Dyninst::ppc32::ir19: return 19;
            case Dyninst::ppc32::ir20: return 20;
            case Dyninst::ppc32::ir21: return 21;
            case Dyninst::ppc32::ir22: return 22;
            case Dyninst::ppc32::ir23: return 23;
            case Dyninst::ppc32::ir24: return 24;
            case Dyninst::ppc32::ir25: return 25;
            case Dyninst::ppc32::ir26: return 26;
            case Dyninst::ppc32::ir27: return 27;
            case Dyninst::ppc32::ir28: return 28;
            case Dyninst::ppc32::ir29: return 29;
            case Dyninst::ppc32::ir30: return 30;
            case Dyninst::ppc32::ir31: return 31;
            case Dyninst::ppc32::ifpr0: return 32;
            case Dyninst::ppc32::ifpr1: return 33;
            case Dyninst::ppc32::ifpr2: return 34;
            case Dyninst::ppc32::ifpr3: return 35;
            case Dyninst::ppc32::ifpr4: return 36;
            case Dyninst::ppc32::ifpr5: return 37;
            case Dyninst::ppc32::ifpr6: return 38;
            case Dyninst::ppc32::ifpr7: return 39;
            case Dyninst::ppc32::ifpr8: return 40;
            case Dyninst::ppc32::ifpr9: return 41;
            case Dyninst::ppc32::ifpr10: return 42;
            case Dyninst::ppc32::ifpr11: return 43;
            case Dyninst::ppc32::ifpr12: return 44;
            case Dyninst::ppc32::ifpr13: return 45;
            case Dyninst::ppc32::ifpr14: return 46;
            case Dyninst::ppc32::ifpr15: return 47;
            case Dyninst::ppc32::ifpr16: return 48;
            case Dyninst::ppc32::ifpr17: return 49;
            case Dyninst::ppc32::ifpr18: return 50;
            case Dyninst::ppc32::ifpr19: return 51;
            case Dyninst::ppc32::ifpr20: return 52;
            case Dyninst::ppc32::ifpr21: return 53;
            case Dyninst::ppc32::ifpr22: return 54;
            case Dyninst::ppc32::ifpr23: return 55;
            case Dyninst::ppc32::ifpr24: return 56;
            case Dyninst::ppc32::ifpr25: return 57;
            case Dyninst::ppc32::ifpr26: return 58;
            case Dyninst::ppc32::ifpr27: return 59;
            case Dyninst::ppc32::ifpr28: return 60;
            case Dyninst::ppc32::ifpr29: return 61;
            case Dyninst::ppc32::ifpr30: return 62;
            case Dyninst::ppc32::ifpr31: return 63;
            case Dyninst::ppc32::icr: return 64;
            case Dyninst::ppc32::imq: return 100;
            case Dyninst::ppc32::ixer: return 101;
            case Dyninst::ppc32::ilr: return 108;
            case Dyninst::ppc32::ictr: return 109;
            default: return -1;
         }
      case Arch_ppc64:
         switch (val()) {
            case Dyninst::ppc64::ir0: return 0;
            case Dyninst::ppc64::ir1: return 1;
            case Dyninst::ppc64::ir2: return 2;
            case Dyninst::ppc64::ir3: return 3;
            case Dyninst::ppc64::ir4: return 4;
            case Dyninst::ppc64::ir5: return 5;
            case Dyninst::ppc64::ir6: return 6;
            case Dyninst::ppc64::ir7: return 7;
            case Dyninst::ppc64::ir8: return 8;
            case Dyninst::ppc64::ir9: return 9;
            case Dyninst::ppc64::ir10: return 10;
            case Dyninst::ppc64::ir11: return 11;
            case Dyninst::ppc64::ir12: return 12;
            case Dyninst::ppc64::ir13: return 13;
            case Dyninst::ppc64::ir14: return 14;
            case Dyninst::ppc64::ir15: return 15;
            case Dyninst::ppc64::ir16: return 16;
            case Dyninst::ppc64::ir17: return 17;
            case Dyninst::ppc64::ir18: return 18;
            case Dyninst::ppc64::ir19: return 19;
            case Dyninst::ppc64::ir20: return 20;
            case Dyninst::ppc64::ir21: return 21;
            case Dyninst::ppc64::ir22: return 22;
            case Dyninst::ppc64::ir23: return 23;
            case Dyninst::ppc64::ir24: return 24;
            case Dyninst::ppc64::ir25: return 25;
            case Dyninst::ppc64::ir26: return 26;
            case Dyninst::ppc64::ir27: return 27;
            case Dyninst::ppc64::ir28: return 28;
            case Dyninst::ppc64::ir29: return 29;
            case Dyninst::ppc64::ir30: return 30;
            case Dyninst::ppc64::ir31: return 31;
            case Dyninst::ppc64::ifpr0: return 32;
            case Dyninst::ppc64::ifpr1: return 33;
            case Dyninst::ppc64::ifpr2: return 34;
            case Dyninst::ppc64::ifpr3: return 35;
            case Dyninst::ppc64::ifpr4: return 36;
            case Dyninst::ppc64::ifpr5: return 37;
            case Dyninst::ppc64::ifpr6: return 38;
            case Dyninst::ppc64::ifpr7: return 39;
            case Dyninst::ppc64::ifpr8: return 40;
            case Dyninst::ppc64::ifpr9: return 41;
            case Dyninst::ppc64::ifpr10: return 42;
            case Dyninst::ppc64::ifpr11: return 43;
            case Dyninst::ppc64::ifpr12: return 44;
            case Dyninst::ppc64::ifpr13: return 45;
            case Dyninst::ppc64::ifpr14: return 46;
            case Dyninst::ppc64::ifpr15: return 47;
            case Dyninst::ppc64::ifpr16: return 48;
            case Dyninst::ppc64::ifpr17: return 49;
            case Dyninst::ppc64::ifpr18: return 50;
            case Dyninst::ppc64::ifpr19: return 51;
            case Dyninst::ppc64::ifpr20: return 52;
            case Dyninst::ppc64::ifpr21: return 53;
            case Dyninst::ppc64::ifpr22: return 54;
            case Dyninst::ppc64::ifpr23: return 55;
            case Dyninst::ppc64::ifpr24: return 56;
            case Dyninst::ppc64::ifpr25: return 57;
            case Dyninst::ppc64::ifpr26: return 58;
            case Dyninst::ppc64::ifpr27: return 59;
            case Dyninst::ppc64::ifpr28: return 60;
            case Dyninst::ppc64::ifpr29: return 61;
            case Dyninst::ppc64::ifpr30: return 62;
            case Dyninst::ppc64::ifpr31: return 63;
            case Dyninst::ppc64::icr: return 64;
            case Dyninst::ppc64::imq: return 100;
            case Dyninst::ppc64::ixer: return 101;
            case Dyninst::ppc64::ilr: return 108;
            case Dyninst::ppc64::ictr: return 109;
            default: return -1;
         }
         break;
      case Arch_none:
         return -1;
   }
   //Invalid register passed
   return -1;
}

unsigned Dyninst::getArchAddressWidth(Dyninst::Architecture arch)
{
   switch (arch) {
      case Arch_none: 
         return 0;
      case Arch_x86:
      case Arch_ppc32:
         return 4;
      case Arch_x86_64:
      case Arch_ppc64:
         return 8;
   }
   return 0;
}

