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

#include "RegisterConversion.h"
#include "registerSpace.h"

#include <map>
#include <boost/assign/list_of.hpp>

#include "Register.h"
#include "registers/MachRegister.h"
#include "registers/abstract_regs.h"
#include "registerSpace.h"

using namespace Dyninst;
using namespace Dyninst::InstructionAPI;
using namespace std;
using namespace boost::assign;

//#warning "This file is not verified yet!"
multimap<Register, MachRegister> regToMachReg64 = map_list_of
  (registerSpace::r0,  		riscv64::x0)
  (registerSpace::r1,  		riscv64::x1)
  (registerSpace::r2,  		riscv64::x2)
  (registerSpace::r3,  		riscv64::x3)
  (registerSpace::r4,  		riscv64::x4)
  (registerSpace::r5,  		riscv64::x5)
  (registerSpace::r6,  		riscv64::x6)
  (registerSpace::r7,  		riscv64::x7)
  (registerSpace::r8,  		riscv64::x8)
  (registerSpace::r9,  		riscv64::x9)
  (registerSpace::r10, 		riscv64::x10)
  (registerSpace::r11, 		riscv64::x11)
  (registerSpace::r12, 		riscv64::x12)
  (registerSpace::r13, 		riscv64::x13)
  (registerSpace::r14, 		riscv64::x14)
  (registerSpace::r15, 		riscv64::x15)
  (registerSpace::r16, 		riscv64::x16)
  (registerSpace::r17, 		riscv64::x17)
  (registerSpace::r18, 		riscv64::x18)
  (registerSpace::r19, 		riscv64::x19)
  (registerSpace::r20, 		riscv64::x20)
  (registerSpace::r21, 		riscv64::x21)
  (registerSpace::r22, 		riscv64::x22)
  (registerSpace::r23, 		riscv64::x23)
  (registerSpace::r24, 		riscv64::x24)
  (registerSpace::r25, 		riscv64::x25)
  (registerSpace::r26, 		riscv64::x26)
  (registerSpace::r27, 		riscv64::x27)
  (registerSpace::r28, 		riscv64::x28)
  (registerSpace::r29, 		riscv64::x29)
  (registerSpace::r30, 		riscv64::x30)
  (registerSpace::r31, 		riscv64::x31)
  (registerSpace::fpr0,  	riscv64::f0)
  (registerSpace::fpr1,  	riscv64::f1)
  (registerSpace::fpr2,  	riscv64::f2)
  (registerSpace::fpr3,  	riscv64::f3)
  (registerSpace::fpr4,  	riscv64::f4)
  (registerSpace::fpr5,  	riscv64::f5)
  (registerSpace::fpr6,  	riscv64::f6)
  (registerSpace::fpr7,  	riscv64::f7)
  (registerSpace::fpr8,  	riscv64::f8)
  (registerSpace::fpr9,  	riscv64::f9)
  (registerSpace::fpr10, 	riscv64::f10)
  (registerSpace::fpr11, 	riscv64::f11)
  (registerSpace::fpr12, 	riscv64::f12)
  (registerSpace::fpr13, 	riscv64::f13)
  (registerSpace::fpr14, 	riscv64::f14)
  (registerSpace::fpr15, 	riscv64::f15)
  (registerSpace::fpr16, 	riscv64::f16)
  (registerSpace::fpr17, 	riscv64::f17)
  (registerSpace::fpr18, 	riscv64::f18)
  (registerSpace::fpr19, 	riscv64::f19)
  (registerSpace::fpr20, 	riscv64::f20)
  (registerSpace::fpr21, 	riscv64::f21)
  (registerSpace::fpr22, 	riscv64::f22)
  (registerSpace::fpr23, 	riscv64::f23)
  (registerSpace::fpr24, 	riscv64::f24)
  (registerSpace::fpr25, 	riscv64::f25)
  (registerSpace::fpr26, 	riscv64::f26)
  (registerSpace::fpr27, 	riscv64::f27)
  (registerSpace::fpr28, 	riscv64::f28)
  (registerSpace::fpr29, 	riscv64::f29)
  (registerSpace::fpr30, 	riscv64::f30)
  (registerSpace::fpr31, 	riscv64::f31)
  (registerSpace::pc, 		riscv64::pc)
  ;

map<MachRegister, Register> reverseRegisterMap = map_list_of
  (riscv64::x0,      registerSpace::r0)
  (riscv64::x1,      registerSpace::r1)
  (riscv64::x2,      registerSpace::r2)
  (riscv64::x3,      registerSpace::r3)
  (riscv64::x4,      registerSpace::r4)
  (riscv64::x5,      registerSpace::r5)
  (riscv64::x6,      registerSpace::r6)
  (riscv64::x7,      registerSpace::r7)
  (riscv64::x8,      registerSpace::r8)
  (riscv64::x9,      registerSpace::r9)
  (riscv64::x10,     registerSpace::r10)
  (riscv64::x11,     registerSpace::r11)
  (riscv64::x13,     registerSpace::r13)
  (riscv64::x14,     registerSpace::r14)
  (riscv64::x15,     registerSpace::r15)
  (riscv64::x16,     registerSpace::r16)
  (riscv64::x17,     registerSpace::r17)
  (riscv64::x18,     registerSpace::r18)
  (riscv64::x19,     registerSpace::r19)
  (riscv64::x20,     registerSpace::r20)
  (riscv64::x21,     registerSpace::r21)
  (riscv64::x22,     registerSpace::r22)
  (riscv64::x23,     registerSpace::r23)
  (riscv64::x24,     registerSpace::r24)
  (riscv64::x25,     registerSpace::r25)
  (riscv64::x26,     registerSpace::r26)
  (riscv64::x27,     registerSpace::r27)
  (riscv64::x28,     registerSpace::r28)
  (riscv64::x29,     registerSpace::r29)
  (riscv64::x30,     registerSpace::r30)
  (riscv64::x31,     registerSpace::r31)
  (riscv64::f0,      registerSpace::fpr0)
  (riscv64::f1,      registerSpace::fpr1)
  (riscv64::f2,      registerSpace::fpr2)
  (riscv64::f3,      registerSpace::fpr3)
  (riscv64::f4,      registerSpace::fpr4)
  (riscv64::f5,      registerSpace::fpr5)
  (riscv64::f6,      registerSpace::fpr6)
  (riscv64::f7,      registerSpace::fpr7)
  (riscv64::f8,      registerSpace::fpr8)
  (riscv64::f9,      registerSpace::fpr9)
  (riscv64::f10,     registerSpace::fpr10)
  (riscv64::f11,     registerSpace::fpr11)
  (riscv64::f12,     registerSpace::fpr12)
  (riscv64::f13,     registerSpace::fpr13)
  (riscv64::f14,     registerSpace::fpr14)
  (riscv64::f15,     registerSpace::fpr15)
  (riscv64::f16,     registerSpace::fpr16)
  (riscv64::f17,     registerSpace::fpr17)
  (riscv64::f18,     registerSpace::fpr18)
  (riscv64::f19,     registerSpace::fpr19)
  (riscv64::f20,     registerSpace::fpr20)
  (riscv64::f21,     registerSpace::fpr21)
  (riscv64::f22,     registerSpace::fpr22)
  (riscv64::f23,     registerSpace::fpr23)
  (riscv64::f24,     registerSpace::fpr24)
  (riscv64::f25,     registerSpace::fpr25)
  (riscv64::f26,     registerSpace::fpr26)
  (riscv64::f27,     registerSpace::fpr27)
  (riscv64::f28,     registerSpace::fpr28)
  (riscv64::f29,     registerSpace::fpr29)
  (riscv64::f30,     registerSpace::fpr30)
  (riscv64::f31,     registerSpace::fpr31)
  (riscv64::pc,      registerSpace::pc)
  ;

Register convertRegID(MachRegister reg, bool &wasUpcast) {
    wasUpcast = false;

    MachRegister baseReg = MachRegister((reg.getBaseRegister().val() & ~reg.getArchitecture()) | Arch_riscv64);
//    RegisterAST::Ptr debug(new RegisterAST(baseReg));
//    fprintf(stderr, "DEBUG: converting %s", toBeConverted->format().c_str());
//    fprintf(stderr, " to %s\n", debug->format().c_str());
    map<MachRegister, Register>::const_iterator found =
      reverseRegisterMap.find(baseReg);
    if(found == reverseRegisterMap.end()) {
      // Yeah, this happens when we analyze trash code. Oops...
      return registerSpace::ignored;
    }

    return found->second;
}


Register convertRegID(RegisterAST::Ptr toBeConverted, bool& wasUpcast)
{
    return convertRegID(toBeConverted.get(), wasUpcast);
}

Register convertRegID(RegisterAST* toBeConverted, bool& wasUpcast)
{
    if(!toBeConverted) {
        //assert(0);
      return registerSpace::ignored;
    }
    return convertRegID(toBeConverted->getID(), wasUpcast);
}

MachRegister convertRegID(Register r, Dyninst::Architecture arch) {
    if( arch == Arch_riscv64 ) {
        switch(r) {
            case registerSpace::r0:    return riscv64::x0;
            case registerSpace::r1:    return riscv64::x1;
            case registerSpace::r2:    return riscv64::x2;
            case registerSpace::r3:    return riscv64::x3;
            case registerSpace::r4:    return riscv64::x4;
            case registerSpace::r5:    return riscv64::x5;
            case registerSpace::r6:    return riscv64::x6;
            case registerSpace::r7:    return riscv64::x7;
            case registerSpace::r8:    return riscv64::x8;
            case registerSpace::r9:    return riscv64::x9;
            case registerSpace::r10:   return riscv64::x10;
            case registerSpace::r11:   return riscv64::x11;
            case registerSpace::r12:   return riscv64::x12;
            case registerSpace::r13:   return riscv64::x13;
            case registerSpace::r14:   return riscv64::x14;
            case registerSpace::r15:   return riscv64::x15;
            case registerSpace::r16:   return riscv64::x16;
            case registerSpace::r17:   return riscv64::x17;
            case registerSpace::r18:   return riscv64::x18;
            case registerSpace::r19:   return riscv64::x19;
            case registerSpace::r20:   return riscv64::x20;
            case registerSpace::r21:   return riscv64::x21;
            case registerSpace::r22:   return riscv64::x22;
            case registerSpace::r23:   return riscv64::x23;
            case registerSpace::r24:   return riscv64::x24;
            case registerSpace::r25:   return riscv64::x25;
            case registerSpace::r26:   return riscv64::x26;
            case registerSpace::r27:   return riscv64::x27;
            case registerSpace::r28:   return riscv64::x28;
            case registerSpace::r29:   return riscv64::x29;
            case registerSpace::r30:   return riscv64::x30;
            case registerSpace::r31:   return riscv64::x31;
            case registerSpace::fpr0:  return riscv64::f0;
            case registerSpace::fpr1:  return riscv64::f1;
            case registerSpace::fpr2:  return riscv64::f2;
            case registerSpace::fpr3:  return riscv64::f3;
            case registerSpace::fpr4:  return riscv64::f4;
            case registerSpace::fpr5:  return riscv64::f5;
            case registerSpace::fpr6:  return riscv64::f6;
            case registerSpace::fpr7:  return riscv64::f7;
            case registerSpace::fpr8:  return riscv64::f8;
            case registerSpace::fpr9:  return riscv64::f9;
            case registerSpace::fpr10: return riscv64::f10;
            case registerSpace::fpr11: return riscv64::f11;
            case registerSpace::fpr12: return riscv64::f12;
            case registerSpace::fpr13: return riscv64::f13;
            case registerSpace::fpr14: return riscv64::f14;
            case registerSpace::fpr15: return riscv64::f15;
            case registerSpace::fpr16: return riscv64::f16;
            case registerSpace::fpr17: return riscv64::f17;
            case registerSpace::fpr18: return riscv64::f18;
            case registerSpace::fpr19: return riscv64::f19;
            case registerSpace::fpr20: return riscv64::f20;
            case registerSpace::fpr21: return riscv64::f21;
            case registerSpace::fpr22: return riscv64::f22;
            case registerSpace::fpr23: return riscv64::f23;
            case registerSpace::fpr24: return riscv64::f24;
            case registerSpace::fpr25: return riscv64::f25;
            case registerSpace::fpr26: return riscv64::f26;
            case registerSpace::fpr27: return riscv64::f27;
            case registerSpace::fpr28: return riscv64::f28;
            case registerSpace::fpr29: return riscv64::f29;
            case registerSpace::fpr30: return riscv64::f30;
            case registerSpace::fpr31: return riscv64::f31;
            case registerSpace::pc:    return riscv64::pc;
            default:
                break;
        }
    }else{
        assert(!"Invalid architecture");
    }

    assert(!"Register not handled");
    return InvalidReg;
}
