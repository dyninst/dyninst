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
#include "external/rose/armv8InstructionEnum.h"

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
		case Arch_aarch32:
		case Arch_aarch64:
				  //not verified
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
	       return 0; // Xiaozhu: do not assert, but return 0 as an indication of parsing junk.
               assert(0);
         }
      case Arch_ppc32: {
         int reg_class = reg & 0x00ff0000;
         if (reg_class == ppc32::FPR || reg_class == ppc32::FSR)
            return 8;
         return 4;
      }
      case Arch_ppc64:
        if((reg & 0x00ff0000) == aarch64::FPR)
          return 16;
        return 8;
      case Arch_aarch32:
        assert(0);
      case Arch_aarch64:
		if((reg & 0x00ff0000) == aarch64::FPR)
		{
		    switch(reg & 0x0000ff00)
		    {
			case aarch64::B_REG: return 1;
			case aarch64::W_REG: return 2;
			case aarch64::D_REG: return 4;
			case aarch64::FULL: 
			case aarch64::HQ_REG: return 8;
			case aarch64::Q_REG: return 16;
			default:
			    assert(0);
			    return 0;
		    }
		}
		else if((reg & 0x00ff0000) == aarch64::GPR || (reg & 0x00ff0000) == aarch64::SPR ||
                (reg & 0x00ff0000) == aarch64::SYSREG || (reg & 0x00ff0000) == aarch64::FLAG)
			switch(reg & 0x0000ff00)
			{
				case aarch64::FULL : return 8;
				case aarch64::D_REG: return 4;
                case aarch64::BIT:   return 0;
				default: return 0;
			}
		else
			return 4;
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
      case Arch_aarch64:  //aarch64: pc is not writable
         return aarch64::pc;
      case Arch_aarch32:
         assert(0);
      case Arch_none:
         return InvalidReg;
   }
   return InvalidReg;
}


MachRegister MachRegister::getReturnAddress(Dyninst::Architecture arch)
{
   switch (arch)
   {
      case Arch_x86:
          assert(0); //not implemented
      case Arch_x86_64:
          assert(0); //not implemented
      case Arch_ppc32:
          assert(0); //not implemented
      case Arch_ppc64:
          assert(0); //not implemented
      case Arch_aarch64:  //aarch64: x30 stores the RA for current frame
         return aarch64::x30;
      case Arch_aarch32:
         assert(0);
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
      case Arch_aarch64:
         return aarch64::x29; //aarch64: frame pointer is X29 by convention
      case Arch_none:
         return InvalidReg;
      default:
         assert(0);
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
      case Arch_aarch64:
         return aarch64::sp; //aarch64: stack pointer is an independent register
      case Arch_aarch32:
         assert(0);
      case Arch_none:
         return InvalidReg;
      default:
         assert(0);
         return InvalidReg;
   }
   return InvalidReg;
}

MachRegister MachRegister::getSyscallNumberReg(Dyninst::Architecture arch)
{
    switch (arch)
    {
        case Arch_x86:
            return x86::eax;
        case Arch_x86_64:
            return x86_64::rax;
        case Arch_ppc32:
            return ppc32::r0;
        case Arch_ppc64:
            return ppc64::r0;
        case Arch_aarch64:
            return aarch64::x8;
        case Arch_aarch32:
            assert(0);
        case Arch_none:
            return InvalidReg;
      default:
         assert(0);
         return InvalidReg;
    }
    return InvalidReg;
}

MachRegister MachRegister::getSyscallNumberOReg(Dyninst::Architecture arch)
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
        case Arch_aarch64:
            return aarch64::x8;
        case Arch_none:
            return InvalidReg;
      default:
         assert(0);
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
        case Arch_aarch64:
            return aarch64::x0; //returned value is save in x0
        case Arch_none:
            return InvalidReg;
      default:
         assert(0);
         return InvalidReg;
    }
    return InvalidReg;
}

MachRegister MachRegister::getArchRegFromAbstractReg(MachRegister abstract,
        Dyninst::Architecture arch) {
    switch(arch){
        case Arch_aarch64:
            if( abstract == ReturnAddr)
                    return aarch64::x30;
            if( abstract == FrameBase)
                    return aarch64::x29;
            if( abstract == StackTop)
                    return aarch64::sp;
            if( abstract == CFA)
                assert(0); //don't know what to do
            //not abstract, return arch reg
            return abstract;
        default:
            assert(0);
    }
    return Dyninst::InvalidReg;
}

MachRegister MachRegister::getZeroFlag(Dyninst::Architecture arch)
{
   switch (arch)
   {
      case Arch_x86:
         return x86::zf;
      case Arch_x86_64:
         return x86_64::zf;
      case Arch_aarch64: 
         return aarch64::z;
      case Arch_aarch32:
      case Arch_ppc32:
      case Arch_ppc64:
         assert(0);
      case Arch_none:
         return InvalidReg;
   }
   return InvalidReg;
}


bool MachRegister::isPC() const
{
   return (*this == x86_64::rip || *this == x86::eip ||
           *this == ppc32::pc || *this == ppc64::pc ||
           *this == aarch64::pc );
}

bool MachRegister::isFramePointer() const
{
   return (*this == x86_64::rbp || *this == x86::ebp ||
           *this == FrameBase ||
           *this == aarch64::x29);
}

bool MachRegister::isStackPointer() const
{
   return (*this == x86_64::rsp || *this == x86::esp ||
           *this == ppc32::r1   || *this == ppc64::r1 ||
           *this == aarch64::sp);
}

bool MachRegister::isSyscallNumberReg() const
{
   return ( *this == x86_64::orax || *this == x86::oeax ||
            *this == ppc32::r1    || *this == ppc64::r1 ||
            *this == aarch64::x8
            );
}

bool MachRegister::isSyscallReturnValueReg() const
{
   if(getArchitecture() == Arch_aarch64)
      assert(0);
    return (*this == x86_64::rax || *this == x86::eax ||
            *this == ppc32::r1   || *this == ppc64::r1 ||
            *this == aarch64::x0
            );
}

bool MachRegister::isFlag() const
{
    int regC = regClass();
    switch (getArchitecture())
    {
      case Arch_x86:
         return regC == x86::FLAG;
      case Arch_x86_64:
         return regC == x86_64::FLAG;
      case Arch_aarch64:
         return regC == aarch64::FLAG;
      default:
         assert(!"Not implemented!");
   }
   return false;
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
    case Arch_x86_64:
         switch (category) {
            case x86_64::GPR:
               c = x86_regclass_gpr;
               switch (baseID) {
                  case x86_64::BASEA:
                     n = x86_gpr_ax;
                     break;
                  case x86_64::BASEC:
                     n = x86_gpr_cx;
                     break;
                  case x86_64::BASED:
                     n = x86_gpr_dx;
                     break;
                  case x86_64::BASEB:
                     n = x86_gpr_bx;
                     break;
                  case x86_64::BASESP:
                     n = x86_gpr_sp;
                     break;
                  case x86_64::BASEBP:
                     n = x86_gpr_bp;
                     break;
                  case x86_64::BASESI:
                     n = x86_gpr_si;
                     break;
                  case x86_64::BASEDI:
                     n = x86_gpr_di;
                     break;
		  case x86_64::BASE8:
		     n = x86_gpr_r8;
		     break;
		  case x86_64::BASE9:
		     n = x86_gpr_r9;
		     break;
		  case x86_64::BASE10:
		     n = x86_gpr_r10;
		     break;
		  case x86_64::BASE11:
		     n = x86_gpr_r11;
		     break;
		  case x86_64::BASE12:
		     n = x86_gpr_r12;
		     break;
		  case x86_64::BASE13:
		     n = x86_gpr_r13;
		     break;
		  case x86_64::BASE14:
		     n = x86_gpr_r14;
		     break;
		  case x86_64::BASE15:
		     n = x86_gpr_r15;
		     break;
                  default:
                     n = 0;
                     break;
               }
               break;
            case x86_64::SEG:
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
            case x86_64::FLAG:
               c = x86_regclass_flags;
	       switch(baseID) {
	       case x86_64::CF:
		 n = x86_flag_cf;
		 break;
	       case x86_64::PF:
		 n = x86_flag_pf;
		 break;
	       case x86_64::AF:
		 n = x86_flag_af;
		 break;
	       case x86_64::ZF:
		 n = x86_flag_zf;
		 break;
	       case x86_64::SF:
		 n = x86_flag_sf;
		 break;
	       case x86_64::TF:
		 n = x86_flag_tf;
		 break;
	       case x86_64::IF:
		 n = x86_flag_if;
		 break;
	       case x86_64::DF:
		 n = x86_flag_df;
		 break;
	       case x86_64::OF:
		 n = x86_flag_of;
		 break;
	       default:
		 assert(0);
		 break;
      }
               break;
            case x86_64::MISC:
               c = x86_regclass_unknown;
               break;
            case x86_64::KMASK:
               c = x86_regclass_kmask;
               n = baseID;
               break;
            case x86_64::ZMM:
               c = x86_regclass_zmm;
               n = baseID;
               break;
            case x86_64::YMM:
               c = x86_regclass_ymm;
               n = baseID;
               break;
            case x86_64::XMM:
               c = x86_regclass_xmm;
               n = baseID;
               break;
            case x86_64::MMX:
               c = x86_regclass_mm;
               n = baseID;
               break;
            case x86_64::CTL:
               c = x86_regclass_cr;
               n = baseID;
               break;
            case x86_64::DBG:
               c = x86_regclass_dr;
               n = baseID;
               break;
            case x86_64::TST:
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
       break;
       case Arch_aarch64: {
           p = 0;
           switch (category) {
               case aarch64::GPR: {
                   c = armv8_regclass_gpr;
                   int regnum = baseID - (aarch64::x0 & 0xFF);
                   n = armv8_gpr_r0 + regnum;
               }
                   break;
               case aarch64::SPR: {
                   n = 0;
                   if (baseID == (aarch64::pstate & 0xFF)) {
                       c = armv8_regclass_pstate;
                   } else if(baseID == (aarch64::zr & 0xFF) || baseID == (aarch64::wzr & 0xFF)) {
                       c = armv8_regclass_gpr;
                       n = armv8_gpr_zr;
                   } else if (baseID == (aarch64::pc & 0xFF)) {
                       c = armv8_regclass_pc;
                   } else if (baseID == (aarch64::sp & 0xFF) || baseID == (aarch64::wsp & 0xFF)) {
                       c = armv8_regclass_sp;
                   }
               }
                   break;
               case aarch64::FPR: {
                   c = armv8_regclass_simd_fpr;

                   int firstRegId;
                   switch(reg & 0xFF00) {
                       case aarch64::Q_REG: firstRegId = (aarch64::q0 & 0xFF);
                           break;
                       case aarch64::HQ_REG: firstRegId = (aarch64::hq0 & 0xFF);
                           p = 64;
                           break;
                       case aarch64::FULL: firstRegId = (aarch64::d0 & 0xFF);
                           break;
                       case aarch64::D_REG: firstRegId = (aarch64::s0 & 0xFF);
                           break;
                       case aarch64::W_REG: firstRegId = (aarch64::h0 & 0xFF);
                           break;
                       case aarch64::B_REG: firstRegId = (aarch64::b0 & 0xFF);
                           break;
                       default:assert(!"invalid register subcategory for ARM64!");
                           break;
                   }
                   n = armv8_simdfpr_v0 + (baseID - firstRegId);
               }
                   break;
               case aarch64::FLAG: {
                   c = armv8_regclass_pstate;
                   n = 0;
                   switch (baseID) {
                       case aarch64::N_FLAG:
                           p = armv8_pstatefield_n;
                           break;
                       case aarch64::Z_FLAG:
                           p = armv8_pstatefield_z;
                           break;
                       case aarch64::V_FLAG:
                           p = armv8_pstatefield_v;
                           break;
                       case aarch64::C_FLAG:
                           p = armv8_pstatefield_c;
                           break;
                       default:
                           assert(!"unknown flag type!");
                           break;
                   }
               }
                   break;
               default:
                   assert(!"unknown register type!");
                   break;
           }
           return;
       }

      break;
      default:
         c = x86_regclass_unknown;
         n = 0;
         break;
   }

   switch (getArchitecture()) {
      case Arch_x86:
         switch (subrange) {
            case x86::OCT:
            case x86::FPDBL:
               p = x86_regpos_qword;
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
            case x86::FULL:
            case x86_64::D_REG:
               p = x86_regpos_dword;
               break;
	    case x86::BIT:
     	       p = x86_regpos_all;
	       break;
         }
         break;

      case Arch_x86_64:
         switch (subrange) {
            case x86::FULL:
            case x86::OCT:
            case x86::FPDBL:
               p = x86_regpos_qword;
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
      case Arch_aarch64:
      {
          assert(0);
        }
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
            case 17: return Dyninst::x86_64::k0;
            case 18: return Dyninst::x86_64::k1;
            case 19: return Dyninst::x86_64::k2;
            case 20: return Dyninst::x86_64::k3;
            case 21: return Dyninst::x86_64::k4;
            case 22: return Dyninst::x86_64::k5;
            case 23: return Dyninst::x86_64::k6;
            case 24: return Dyninst::x86_64::k7;
            case 25: return Dyninst::x86_64::zmm0;
            case 26: return Dyninst::x86_64::zmm1;
            case 27: return Dyninst::x86_64::zmm2;
            case 28: return Dyninst::x86_64::zmm3;
            case 29: return Dyninst::x86_64::zmm4;
            case 30: return Dyninst::x86_64::zmm5;
            case 31: return Dyninst::x86_64::zmm6;
            case 32: return Dyninst::x86_64::zmm7;
            case 33: return Dyninst::x86_64::zmm8;
            case 34: return Dyninst::x86_64::zmm9;
            case 35: return Dyninst::x86_64::zmm10;
            case 36: return Dyninst::x86_64::zmm11;
            case 37: return Dyninst::x86_64::zmm12;
            case 38: return Dyninst::x86_64::zmm13;
            case 39: return Dyninst::x86_64::zmm14;
            case 40: return Dyninst::x86_64::zmm15;
            case 41: return Dyninst::x86_64::zmm16;
            case 42: return Dyninst::x86_64::zmm17;
            case 43: return Dyninst::x86_64::zmm18;
            case 44: return Dyninst::x86_64::zmm19;
            case 45: return Dyninst::x86_64::zmm20;
            case 46: return Dyninst::x86_64::zmm21;
            case 47: return Dyninst::x86_64::zmm22;
            case 48: return Dyninst::x86_64::zmm23;
            case 49: return Dyninst::x86_64::zmm24;
            case 50: return Dyninst::x86_64::zmm25;
            case 51: return Dyninst::x86_64::zmm26;
            case 52: return Dyninst::x86_64::zmm27;
            case 53: return Dyninst::x86_64::zmm28;
            case 54: return Dyninst::x86_64::zmm29;
            case 55: return Dyninst::x86_64::zmm30;
            case 56: return Dyninst::x86_64::zmm31;
            case 57: return Dyninst::x86_64::ymm0;
            case 58: return Dyninst::x86_64::ymm1;
            case 59: return Dyninst::x86_64::ymm2;
            case 60: return Dyninst::x86_64::ymm3;
            case 61: return Dyninst::x86_64::ymm4;
            case 62: return Dyninst::x86_64::ymm5;
            case 63: return Dyninst::x86_64::ymm6;
            case 64: return Dyninst::x86_64::ymm7;
            case 65: return Dyninst::x86_64::ymm8;
            case 66: return Dyninst::x86_64::ymm9;
            case 67: return Dyninst::x86_64::ymm10;
            case 68: return Dyninst::x86_64::ymm11;
            case 69: return Dyninst::x86_64::ymm12;
            case 70: return Dyninst::x86_64::ymm13;
            case 71: return Dyninst::x86_64::ymm14;
            case 72: return Dyninst::x86_64::ymm15;
            case 73: return Dyninst::x86_64::ymm16;
            case 74: return Dyninst::x86_64::ymm17;
            case 75: return Dyninst::x86_64::ymm18;
            case 76: return Dyninst::x86_64::ymm19;
            case 77: return Dyninst::x86_64::ymm20;
            case 78: return Dyninst::x86_64::ymm21;
            case 79: return Dyninst::x86_64::ymm22;
            case 80: return Dyninst::x86_64::ymm23;
            case 81: return Dyninst::x86_64::ymm24;
            case 82: return Dyninst::x86_64::ymm25;
            case 83: return Dyninst::x86_64::ymm26;
            case 84: return Dyninst::x86_64::ymm27;
            case 85: return Dyninst::x86_64::ymm28;
            case 86: return Dyninst::x86_64::ymm29;
            case 87: return Dyninst::x86_64::ymm30;
            case 88: return Dyninst::x86_64::ymm31;
            case 89: return Dyninst::x86_64::xmm0;
            case 90: return Dyninst::x86_64::xmm1;
            case 91: return Dyninst::x86_64::xmm2;
            case 92: return Dyninst::x86_64::xmm3;
            case 93: return Dyninst::x86_64::xmm4;
            case 94: return Dyninst::x86_64::xmm5;
            case 95: return Dyninst::x86_64::xmm6;
            case 96: return Dyninst::x86_64::xmm7;
            case 97: return Dyninst::x86_64::xmm8;
            case 98: return Dyninst::x86_64::xmm9;
            case 99: return Dyninst::x86_64::xmm10;
            case 100: return Dyninst::x86_64::xmm11;
            case 101: return Dyninst::x86_64::xmm12;
            case 102: return Dyninst::x86_64::xmm13;
            case 103: return Dyninst::x86_64::xmm14;
            case 104: return Dyninst::x86_64::xmm15;
            case 105: return Dyninst::x86_64::xmm16;
            case 106: return Dyninst::x86_64::xmm17;
            case 107: return Dyninst::x86_64::xmm18;
            case 108: return Dyninst::x86_64::xmm19;
            case 109: return Dyninst::x86_64::xmm20;
            case 110: return Dyninst::x86_64::xmm21;
            case 111: return Dyninst::x86_64::xmm22;
            case 112: return Dyninst::x86_64::xmm23;
            case 113: return Dyninst::x86_64::xmm24;
            case 114: return Dyninst::x86_64::xmm25;
            case 115: return Dyninst::x86_64::xmm26;
            case 116: return Dyninst::x86_64::xmm27;
            case 117: return Dyninst::x86_64::xmm28;
            case 118: return Dyninst::x86_64::xmm29;
            case 119: return Dyninst::x86_64::xmm30;
            case 120: return Dyninst::x86_64::xmm31;
            case 121: return Dyninst::x86_64::st0;
            case 122: return Dyninst::x86_64::st1;
            case 123: return Dyninst::x86_64::st2;
            case 124: return Dyninst::x86_64::st3;
            case 125: return Dyninst::x86_64::st4;
            case 126: return Dyninst::x86_64::st5;
            case 127: return Dyninst::x86_64::st6;
            case 128: return Dyninst::x86_64::st7;
            case 129: return Dyninst::x86_64::mm0;
            case 130: return Dyninst::x86_64::mm1;
            case 131: return Dyninst::x86_64::mm2;
            case 132: return Dyninst::x86_64::mm3;
            case 133: return Dyninst::x86_64::mm4;
            case 134: return Dyninst::x86_64::mm5;
            case 135: return Dyninst::x86_64::mm6;
            case 136: return Dyninst::x86_64::mm7;
            case 137: return Dyninst::x86_64::flags;
            case 138: return Dyninst::x86_64::es;
            case 139: return Dyninst::x86_64::cs;
            case 140: return Dyninst::x86_64::ss;
            case 141: return Dyninst::x86_64::ds;
            case 142: return Dyninst::x86_64::fs;
            case 143: return Dyninst::x86_64::gs;
            case 144: return Dyninst::InvalidReg;
            case 145: return Dyninst::InvalidReg;
            case 146: return Dyninst::x86_64::fsbase;
            case 147: return Dyninst::x86_64::gsbase;
            case 148: return Dyninst::InvalidReg;
            case 149: return Dyninst::InvalidReg;
            case 150: return Dyninst::InvalidReg; //tr
            case 151: return Dyninst::InvalidReg; //ldtr
            case 152: return Dyninst::InvalidReg; //mxcsr
            case 153: return Dyninst::InvalidReg; //fcw
            case 154: return Dyninst::InvalidReg; //fsw
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
      case Arch_aarch64:
         {
         // this info can be found in
         // DWARF for the ARM ® 64-bit Architecture (AArch64)
         switch(encoding){
            case 0:  return Dyninst::aarch64::x0;
            case 1:  return Dyninst::aarch64::x1;
            case 2:  return Dyninst::aarch64::x2;
            case 3:  return Dyninst::aarch64::x3;
            case 4:  return Dyninst::aarch64::x4;
            case 5:  return Dyninst::aarch64::x5;
            case 6:  return Dyninst::aarch64::x6;
            case 7:  return Dyninst::aarch64::x7;
            case 8:  return Dyninst::aarch64::x8;
            case 9:  return Dyninst::aarch64::x9;
            case 10: return Dyninst::aarch64::x10;
            case 11: return Dyninst::aarch64::x11;
            case 12: return Dyninst::aarch64::x12;
            case 13: return Dyninst::aarch64::x13;
            case 14: return Dyninst::aarch64::x14;
            case 15: return Dyninst::aarch64::x15;
            case 16: return Dyninst::aarch64::x16;
            case 17: return Dyninst::aarch64::x17;
            case 18: return Dyninst::aarch64::x18;
            case 19: return Dyninst::aarch64::x19;
            case 20: return Dyninst::aarch64::x20;
            case 21: return Dyninst::aarch64::x21;
            case 22: return Dyninst::aarch64::x22;
            case 23: return Dyninst::aarch64::x23;
            case 24: return Dyninst::aarch64::x24;
            case 25: return Dyninst::aarch64::x25;
            case 26: return Dyninst::aarch64::x26;
            case 27: return Dyninst::aarch64::x27;
            case 28: return Dyninst::aarch64::x28;
            case 29: return Dyninst::aarch64::x29;
            case 30: return Dyninst::aarch64::x30;
            case 31: return Dyninst::aarch64::sp;
            case 32: return Dyninst::InvalidReg;
         }
         switch(encoding){
            case 64: return Dyninst::aarch64::q0;
            case 65: return Dyninst::aarch64::q1;
            case 66: return Dyninst::aarch64::q2;
            case 67: return Dyninst::aarch64::q3;
            case 68: return Dyninst::aarch64::q4;
            case 69: return Dyninst::aarch64::q5;
            case 70: return Dyninst::aarch64::q6;
            case 71: return Dyninst::aarch64::q7;
            case 72: return Dyninst::aarch64::q8;
            case 73: return Dyninst::aarch64::q9;
            case 74: return Dyninst::aarch64::q10;
            case 75: return Dyninst::aarch64::q11;
            case 76: return Dyninst::aarch64::q12;
            case 77: return Dyninst::aarch64::q13;
            case 78: return Dyninst::aarch64::q14;
            case 79: return Dyninst::aarch64::q15;
            case 80: return Dyninst::aarch64::q16;
            case 81: return Dyninst::aarch64::q17;
            case 82: return Dyninst::aarch64::q18;
            case 83: return Dyninst::aarch64::q19;
            case 84: return Dyninst::aarch64::q20;
            case 85: return Dyninst::aarch64::q21;
            case 86: return Dyninst::aarch64::q22;
            case 87: return Dyninst::aarch64::q23;
            case 88: return Dyninst::aarch64::q24;
            case 89: return Dyninst::aarch64::q25;
            case 90: return Dyninst::aarch64::q26;
            case 91: return Dyninst::aarch64::q27;
            case 92: return Dyninst::aarch64::q28;
            case 93: return Dyninst::aarch64::q29;
            case 94: return Dyninst::aarch64::q30;
            case 95: return Dyninst::aarch64::q31;

            default: return Dyninst::InvalidReg;
            break;
         }
         return Dyninst::InvalidReg;
         }
      case Arch_none:
         return Dyninst::InvalidReg;
         break;
      default:
         assert(0);
         return InvalidReg;
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
            case Dyninst::x86_64::ik0: return 17;
            case Dyninst::x86_64::ik1: return 18;
            case Dyninst::x86_64::ik2: return 19;
            case Dyninst::x86_64::ik3: return 20;
            case Dyninst::x86_64::ik4: return 21;
            case Dyninst::x86_64::ik5: return 22;
            case Dyninst::x86_64::ik6: return 23;
            case Dyninst::x86_64::ik7: return 24;
            case Dyninst::x86_64::izmm0: return 25;
            case Dyninst::x86_64::izmm1: return 26;
            case Dyninst::x86_64::izmm2: return 27;
            case Dyninst::x86_64::izmm3: return 28;
            case Dyninst::x86_64::izmm4: return 29;
            case Dyninst::x86_64::izmm5: return 30;
            case Dyninst::x86_64::izmm6: return 31;
            case Dyninst::x86_64::izmm7: return 32;
            case Dyninst::x86_64::izmm8: return 33;
            case Dyninst::x86_64::izmm9: return 34;
            case Dyninst::x86_64::izmm10: return 35;
            case Dyninst::x86_64::izmm11: return 36;
            case Dyninst::x86_64::izmm12: return 37;
            case Dyninst::x86_64::izmm13: return 38;
            case Dyninst::x86_64::izmm14: return 39;
            case Dyninst::x86_64::izmm15: return 40;
            case Dyninst::x86_64::izmm16: return 41;
            case Dyninst::x86_64::izmm17: return 42;
            case Dyninst::x86_64::izmm18: return 43;
            case Dyninst::x86_64::izmm19: return 44;
            case Dyninst::x86_64::izmm20: return 45;
            case Dyninst::x86_64::izmm21: return 46;
            case Dyninst::x86_64::izmm22: return 47;
            case Dyninst::x86_64::izmm23: return 48;
            case Dyninst::x86_64::izmm24: return 49;
            case Dyninst::x86_64::izmm25: return 50;
            case Dyninst::x86_64::izmm26: return 51;
            case Dyninst::x86_64::izmm27: return 52;
            case Dyninst::x86_64::izmm28: return 53;
            case Dyninst::x86_64::izmm29: return 54;
            case Dyninst::x86_64::izmm30: return 55;
            case Dyninst::x86_64::izmm31: return 56;
            case Dyninst::x86_64::iymm0: return 57;
            case Dyninst::x86_64::iymm1: return 58;
            case Dyninst::x86_64::iymm2: return 59;
            case Dyninst::x86_64::iymm3: return 60;
            case Dyninst::x86_64::iymm4: return 61;
            case Dyninst::x86_64::iymm5: return 62;
            case Dyninst::x86_64::iymm6: return 63;
            case Dyninst::x86_64::iymm7: return 64;
            case Dyninst::x86_64::iymm8: return 65;
            case Dyninst::x86_64::iymm9: return 66;
            case Dyninst::x86_64::iymm10: return 67;
            case Dyninst::x86_64::iymm11: return 68;
            case Dyninst::x86_64::iymm12: return 69;
            case Dyninst::x86_64::iymm13: return 70;
            case Dyninst::x86_64::iymm14: return 71;
            case Dyninst::x86_64::iymm15: return 72;
            case Dyninst::x86_64::iymm16: return 73;
            case Dyninst::x86_64::iymm17: return 74;
            case Dyninst::x86_64::iymm18: return 75;
            case Dyninst::x86_64::iymm19: return 76;
            case Dyninst::x86_64::iymm20: return 77;
            case Dyninst::x86_64::iymm21: return 78;
            case Dyninst::x86_64::iymm22: return 79;
            case Dyninst::x86_64::iymm23: return 80;
            case Dyninst::x86_64::iymm24: return 81;
            case Dyninst::x86_64::iymm25: return 82;
            case Dyninst::x86_64::iymm26: return 83;
            case Dyninst::x86_64::iymm27: return 84;
            case Dyninst::x86_64::iymm28: return 85;
            case Dyninst::x86_64::iymm29: return 86;
            case Dyninst::x86_64::iymm30: return 87;
            case Dyninst::x86_64::iymm31: return 88;
            case Dyninst::x86_64::ixmm0: return 89;
            case Dyninst::x86_64::ixmm1: return 90;
            case Dyninst::x86_64::ixmm2: return 91;
            case Dyninst::x86_64::ixmm3: return 92;
            case Dyninst::x86_64::ixmm4: return 93;
            case Dyninst::x86_64::ixmm5: return 94;
            case Dyninst::x86_64::ixmm6: return 95;
            case Dyninst::x86_64::ixmm7: return 96;
            case Dyninst::x86_64::ixmm8: return 97;
            case Dyninst::x86_64::ixmm9: return 98;
            case Dyninst::x86_64::ixmm10: return 99;
            case Dyninst::x86_64::ixmm11: return 100;
            case Dyninst::x86_64::ixmm12: return 101;
            case Dyninst::x86_64::ixmm13: return 102;
            case Dyninst::x86_64::ixmm14: return 103;
            case Dyninst::x86_64::ixmm15: return 104;
            case Dyninst::x86_64::ixmm16: return 105;
            case Dyninst::x86_64::ixmm17: return 106;
            case Dyninst::x86_64::ixmm18: return 107;
            case Dyninst::x86_64::ixmm19: return 108;
            case Dyninst::x86_64::ixmm20: return 109;
            case Dyninst::x86_64::ixmm21: return 110;
            case Dyninst::x86_64::ixmm22: return 111;
            case Dyninst::x86_64::ixmm23: return 112;
            case Dyninst::x86_64::ixmm24: return 113;
            case Dyninst::x86_64::ixmm25: return 114;
            case Dyninst::x86_64::ixmm26: return 115;
            case Dyninst::x86_64::ixmm27: return 116;
            case Dyninst::x86_64::ixmm28: return 117;
            case Dyninst::x86_64::ixmm29: return 118;
            case Dyninst::x86_64::ixmm30: return 119;
            case Dyninst::x86_64::ixmm31: return 120;
            //case Dyninst::x86_64::ist0: return 121;
            //case Dyninst::x86_64::ist1: return 122;
            //case Dyninst::x86_64::ist2: return 123;
            //case Dyninst::x86_64::ist3: return 124;
            //case Dyninst::x86_64::ist4: return 125;
            //case Dyninst::x86_64::ist5: return 126;
            //case Dyninst::x86_64::ist6: return 127;
            //case Dyninst::x86_64::ist7: return 128;
            case Dyninst::x86_64::imm0: return 129;
            case Dyninst::x86_64::imm1: return 130;
            case Dyninst::x86_64::imm2: return 131;
            case Dyninst::x86_64::imm3: return 132;
            case Dyninst::x86_64::imm4: return 133;
            case Dyninst::x86_64::imm5: return 134;
            case Dyninst::x86_64::imm6: return 135;
            case Dyninst::x86_64::imm7: return 136;
            case Dyninst::x86_64::iflags: return 137;
            case Dyninst::x86_64::ies: return 138;
            case Dyninst::x86_64::ics: return 139;
            case Dyninst::x86_64::iss: return 140;
            case Dyninst::x86_64::ids: return 141;
            case Dyninst::x86_64::ifs: return 142;
            case Dyninst::x86_64::igs: return 143;
            // INVALID REG 144
            // INVALID REG 145
            case Dyninst::x86_64::ifsbase: return 146;
            case Dyninst::x86_64::igsbase: return 147;
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
      case Arch_aarch64:
         switch (val()) {
            case Dyninst::aarch64::ix0: 	    return 0;
            case Dyninst::aarch64::ix1: 	    return 1;
            case Dyninst::aarch64::ix2: 	    return 2;
            case Dyninst::aarch64::ix3: 	    return 3;
            case Dyninst::aarch64::ix4: 	    return 4;
            case Dyninst::aarch64::ix5: 	    return 5;
            case Dyninst::aarch64::ix6: 	    return 6;
            case Dyninst::aarch64::ix7: 	    return 7;
            case Dyninst::aarch64::ix8: 	    return 8;
            case Dyninst::aarch64::ix9: 	    return 9;
            case Dyninst::aarch64::ix10: 	return 10;
            case Dyninst::aarch64::ix11: 	return 11;
            case Dyninst::aarch64::ix12: 	return 12;
            case Dyninst::aarch64::ix13: 	return 13;
            case Dyninst::aarch64::ix14: 	return 14;
            case Dyninst::aarch64::ix15: 	return 15;
            case Dyninst::aarch64::ix16: 	return 16;
            case Dyninst::aarch64::ix17: 	return 17;
            case Dyninst::aarch64::ix18: 	return 18;
            case Dyninst::aarch64::ix19: 	return 19;
            case Dyninst::aarch64::ix20: 	return 20;
            case Dyninst::aarch64::ix21: 	return 21;
            case Dyninst::aarch64::ix22: 	return 22;
            case Dyninst::aarch64::ix23: 	return 23;
            case Dyninst::aarch64::ix24: 	return 24;
            case Dyninst::aarch64::ix25: 	return 25;
            case Dyninst::aarch64::ix26: 	return 26;
            case Dyninst::aarch64::ix27: 	return 27;
            case Dyninst::aarch64::ix28: 	return 28;
            case Dyninst::aarch64::ix29: 	return 29;
            case Dyninst::aarch64::ix30: 	return 30;
            case Dyninst::aarch64::isp:      return 31;

            case Dyninst::aarch64::iq0:      return 64;
            case Dyninst::aarch64::iq1:      return 65;
            case Dyninst::aarch64::iq2:      return 66;
            case Dyninst::aarch64::iq3:      return 67;
            case Dyninst::aarch64::iq4:      return 68;
            case Dyninst::aarch64::iq5:      return 69;
            case Dyninst::aarch64::iq6:      return 70;
            case Dyninst::aarch64::iq7:      return 71;
            case Dyninst::aarch64::iq8:      return 72;
            case Dyninst::aarch64::iq9:      return 73;
            case Dyninst::aarch64::iq10:     return 74;
            case Dyninst::aarch64::iq11:     return 75;
            case Dyninst::aarch64::iq12:     return 76;
            case Dyninst::aarch64::iq13:     return 77;
            case Dyninst::aarch64::iq14:     return 78;
            case Dyninst::aarch64::iq15:     return 79;
            case Dyninst::aarch64::iq16:     return 80;
            case Dyninst::aarch64::iq17:     return 81;
            case Dyninst::aarch64::iq18:     return 82;
            case Dyninst::aarch64::iq19:     return 83;
            case Dyninst::aarch64::iq20:     return 84;
            case Dyninst::aarch64::iq21:     return 85;
            case Dyninst::aarch64::iq22:     return 86;
            case Dyninst::aarch64::iq23:     return 87;
            case Dyninst::aarch64::iq24:     return 88;
            case Dyninst::aarch64::iq25:     return 89;
            case Dyninst::aarch64::iq26:     return 90;
            case Dyninst::aarch64::iq27:     return 91;
            case Dyninst::aarch64::iq28:     return 92;
            case Dyninst::aarch64::iq29:     return 93;
            case Dyninst::aarch64::iq30:     return 94;
            case Dyninst::aarch64::iq31:     return 95;

            default: return -1;
         }
         break;
      case Arch_none:
         assert(0);
         return -1;
      default:
        assert(0);
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
      case Arch_aarch64:
         return 8;
      default:
         assert(0);
         return InvalidReg;
   }
   return 0;
}

MachRegister MachRegister::getArchReg(unsigned int regNum, Dyninst::Architecture arch){
    switch(arch){
      case Arch_aarch64:
         switch(regNum){
            case 0:  return Dyninst::aarch64::x0;
            case 1:  return Dyninst::aarch64::x1;
            case 2:  return Dyninst::aarch64::x2;
            case 3:  return Dyninst::aarch64::x3;
            case 4:  return Dyninst::aarch64::x4;
            case 5:  return Dyninst::aarch64::x5;
            case 6:  return Dyninst::aarch64::x6;
            case 7:  return Dyninst::aarch64::x7;
            case 8:  return Dyninst::aarch64::x8;
            case 9:  return Dyninst::aarch64::x9;
            case 10: return Dyninst::aarch64::x10;
            case 11: return Dyninst::aarch64::x11;
            case 12: return Dyninst::aarch64::x12;
            case 13: return Dyninst::aarch64::x13;
            case 14: return Dyninst::aarch64::x14;
            case 15: return Dyninst::aarch64::x15;
            case 16: return Dyninst::aarch64::x16;
            case 17: return Dyninst::aarch64::x17;
            case 18: return Dyninst::aarch64::x18;
            case 19: return Dyninst::aarch64::x19;
            case 20: return Dyninst::aarch64::x20;
            case 21: return Dyninst::aarch64::x21;
            case 22: return Dyninst::aarch64::x22;
            case 23: return Dyninst::aarch64::x23;
            case 24: return Dyninst::aarch64::x24;
            case 25: return Dyninst::aarch64::x25;
            case 26: return Dyninst::aarch64::x26;
            case 27: return Dyninst::aarch64::x27;
            case 28: return Dyninst::aarch64::x28;
            case 29: return Dyninst::aarch64::x29;
            case 30: return Dyninst::aarch64::x30;

            case 100: return Dyninst::aarch64::sp;
            case 101: return Dyninst::aarch64::pc;
            case 102: return Dyninst::aarch64::pstate;
            case 103: return Dyninst::aarch64::zr;
         }
      default:
         return InvalidReg;
    }
    return InvalidReg;
}
