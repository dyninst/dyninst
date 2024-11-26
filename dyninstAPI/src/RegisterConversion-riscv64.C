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
  (registerSpace::fpr32_0,  	riscv64::f0_32)
  (registerSpace::fpr32_1,  	riscv64::f1_32)
  (registerSpace::fpr32_2,  	riscv64::f2_32)
  (registerSpace::fpr32_3,  	riscv64::f3_32)
  (registerSpace::fpr32_4,  	riscv64::f4_32)
  (registerSpace::fpr32_5,  	riscv64::f5_32)
  (registerSpace::fpr32_6,  	riscv64::f6_32)
  (registerSpace::fpr32_7,  	riscv64::f7_32)
  (registerSpace::fpr32_8,  	riscv64::f8_32)
  (registerSpace::fpr32_9,  	riscv64::f9_32)
  (registerSpace::fpr32_10, 	riscv64::f10_32)
  (registerSpace::fpr32_11, 	riscv64::f11_32)
  (registerSpace::fpr32_12, 	riscv64::f12_32)
  (registerSpace::fpr32_13, 	riscv64::f13_32)
  (registerSpace::fpr32_14, 	riscv64::f14_32)
  (registerSpace::fpr32_15, 	riscv64::f15_32)
  (registerSpace::fpr32_16, 	riscv64::f16_32)
  (registerSpace::fpr32_17, 	riscv64::f17_32)
  (registerSpace::fpr32_18, 	riscv64::f18_32)
  (registerSpace::fpr32_19, 	riscv64::f19_32)
  (registerSpace::fpr32_20, 	riscv64::f20_32)
  (registerSpace::fpr32_21, 	riscv64::f21_32)
  (registerSpace::fpr32_22, 	riscv64::f22_32)
  (registerSpace::fpr32_23, 	riscv64::f23_32)
  (registerSpace::fpr32_24, 	riscv64::f24_32)
  (registerSpace::fpr32_25, 	riscv64::f25_32)
  (registerSpace::fpr32_26, 	riscv64::f26_32)
  (registerSpace::fpr32_27, 	riscv64::f27_32)
  (registerSpace::fpr32_28, 	riscv64::f28_32)
  (registerSpace::fpr32_29, 	riscv64::f29_32)
  (registerSpace::fpr32_30, 	riscv64::f30_32)
  (registerSpace::fpr32_31, 	riscv64::f31_32)
  (registerSpace::fpr64_0,  	riscv64::f0_64)
  (registerSpace::fpr64_1,  	riscv64::f1_64)
  (registerSpace::fpr64_2,  	riscv64::f2_64)
  (registerSpace::fpr64_3,  	riscv64::f3_64)
  (registerSpace::fpr64_4,  	riscv64::f4_64)
  (registerSpace::fpr64_5,  	riscv64::f5_64)
  (registerSpace::fpr64_6,  	riscv64::f6_64)
  (registerSpace::fpr64_7,  	riscv64::f7_64)
  (registerSpace::fpr64_8,  	riscv64::f8_64)
  (registerSpace::fpr64_9,  	riscv64::f9_64)
  (registerSpace::fpr64_10, 	riscv64::f10_64)
  (registerSpace::fpr64_11, 	riscv64::f11_64)
  (registerSpace::fpr64_12, 	riscv64::f12_64)
  (registerSpace::fpr64_13, 	riscv64::f13_64)
  (registerSpace::fpr64_14, 	riscv64::f14_64)
  (registerSpace::fpr64_15, 	riscv64::f15_64)
  (registerSpace::fpr64_16, 	riscv64::f16_64)
  (registerSpace::fpr64_17, 	riscv64::f17_64)
  (registerSpace::fpr64_18, 	riscv64::f18_64)
  (registerSpace::fpr64_19, 	riscv64::f19_64)
  (registerSpace::fpr64_20, 	riscv64::f20_64)
  (registerSpace::fpr64_21, 	riscv64::f21_64)
  (registerSpace::fpr64_22, 	riscv64::f22_64)
  (registerSpace::fpr64_23, 	riscv64::f23_64)
  (registerSpace::fpr64_24, 	riscv64::f24_64)
  (registerSpace::fpr64_25, 	riscv64::f25_64)
  (registerSpace::fpr64_26, 	riscv64::f26_64)
  (registerSpace::fpr64_27, 	riscv64::f27_64)
  (registerSpace::fpr64_28, 	riscv64::f28_64)
  (registerSpace::fpr64_29, 	riscv64::f29_64)
  (registerSpace::fpr64_30, 	riscv64::f30_64)
  (registerSpace::fpr64_31, 	riscv64::f31_64)
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
  (riscv64::f0_32,   registerSpace::fpr32_0)
  (riscv64::f1_32,   registerSpace::fpr32_1)
  (riscv64::f2_32,   registerSpace::fpr32_2)
  (riscv64::f3_32,   registerSpace::fpr32_3)
  (riscv64::f4_32,   registerSpace::fpr32_4)
  (riscv64::f5_32,   registerSpace::fpr32_5)
  (riscv64::f6_32,   registerSpace::fpr32_6)
  (riscv64::f7_32,   registerSpace::fpr32_7)
  (riscv64::f8_32,   registerSpace::fpr32_8)
  (riscv64::f9_32,   registerSpace::fpr32_9)
  (riscv64::f10_32,  registerSpace::fpr32_10)
  (riscv64::f11_32,  registerSpace::fpr32_11)
  (riscv64::f12_32,  registerSpace::fpr32_12)
  (riscv64::f13_32,  registerSpace::fpr32_13)
  (riscv64::f14_32,  registerSpace::fpr32_14)
  (riscv64::f15_32,  registerSpace::fpr32_15)
  (riscv64::f16_32,  registerSpace::fpr32_16)
  (riscv64::f17_32,  registerSpace::fpr32_17)
  (riscv64::f18_32,  registerSpace::fpr32_18)
  (riscv64::f19_32,  registerSpace::fpr32_19)
  (riscv64::f20_32,  registerSpace::fpr32_20)
  (riscv64::f21_32,  registerSpace::fpr32_21)
  (riscv64::f22_32,  registerSpace::fpr32_22)
  (riscv64::f23_32,  registerSpace::fpr32_23)
  (riscv64::f24_32,  registerSpace::fpr32_24)
  (riscv64::f25_32,  registerSpace::fpr32_25)
  (riscv64::f26_32,  registerSpace::fpr32_26)
  (riscv64::f27_32,  registerSpace::fpr32_27)
  (riscv64::f28_32,  registerSpace::fpr32_28)
  (riscv64::f29_32,  registerSpace::fpr32_29)
  (riscv64::f30_32,  registerSpace::fpr32_30)
  (riscv64::f31_32,  registerSpace::fpr32_31)
  (riscv64::f0_64,   registerSpace::fpr64_0)
  (riscv64::f1_64,   registerSpace::fpr64_1)
  (riscv64::f2_64,   registerSpace::fpr64_2)
  (riscv64::f3_64,   registerSpace::fpr64_3)
  (riscv64::f4_64,   registerSpace::fpr64_4)
  (riscv64::f5_64,   registerSpace::fpr64_5)
  (riscv64::f6_64,   registerSpace::fpr64_6)
  (riscv64::f7_64,   registerSpace::fpr64_7)
  (riscv64::f8_64,   registerSpace::fpr64_8)
  (riscv64::f9_64,   registerSpace::fpr64_9)
  (riscv64::f10_64,  registerSpace::fpr64_10)
  (riscv64::f11_64,  registerSpace::fpr64_11)
  (riscv64::f12_64,  registerSpace::fpr64_12)
  (riscv64::f13_64,  registerSpace::fpr64_13)
  (riscv64::f14_64,  registerSpace::fpr64_14)
  (riscv64::f15_64,  registerSpace::fpr64_15)
  (riscv64::f16_64,  registerSpace::fpr64_16)
  (riscv64::f17_64,  registerSpace::fpr64_17)
  (riscv64::f18_64,  registerSpace::fpr64_18)
  (riscv64::f19_64,  registerSpace::fpr64_19)
  (riscv64::f20_64,  registerSpace::fpr64_20)
  (riscv64::f21_64,  registerSpace::fpr64_21)
  (riscv64::f22_64,  registerSpace::fpr64_22)
  (riscv64::f23_64,  registerSpace::fpr64_23)
  (riscv64::f24_64,  registerSpace::fpr64_24)
  (riscv64::f25_64,  registerSpace::fpr64_25)
  (riscv64::f26_64,  registerSpace::fpr64_26)
  (riscv64::f27_64,  registerSpace::fpr64_27)
  (riscv64::f28_64,  registerSpace::fpr64_28)
  (riscv64::f29_64,  registerSpace::fpr64_29)
  (riscv64::f30_64,  registerSpace::fpr64_30)
  (riscv64::f31_64,  registerSpace::fpr64_31)
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
            case registerSpace::r0: 	return riscv64::x0;
            case registerSpace::r1: 	return riscv64::x1;
            case registerSpace::r2: 	return riscv64::x2;
            case registerSpace::r3: 	return riscv64::x3;
            case registerSpace::r4: 	return riscv64::x4;
            case registerSpace::r5: 	return riscv64::x5;
            case registerSpace::r6: 	return riscv64::x6;
            case registerSpace::r7: 	return riscv64::x7;
            case registerSpace::r8: 	return riscv64::x8;
            case registerSpace::r9: 	return riscv64::x9;
            case registerSpace::r10: 	return riscv64::x10;
            case registerSpace::r11: 	return riscv64::x11;
            case registerSpace::r12: 	return riscv64::x12;
            case registerSpace::r13: 	return riscv64::x13;
            case registerSpace::r14: 	return riscv64::x14;
            case registerSpace::r15: 	return riscv64::x15;
            case registerSpace::r16: 	return riscv64::x16;
            case registerSpace::r17: 	return riscv64::x17;
            case registerSpace::r18: 	return riscv64::x18;
            case registerSpace::r19: 	return riscv64::x19;
            case registerSpace::r20: 	return riscv64::x20;
            case registerSpace::r21: 	return riscv64::x21;
            case registerSpace::r22: 	return riscv64::x22;
            case registerSpace::r23: 	return riscv64::x23;
            case registerSpace::r24: 	return riscv64::x24;
            case registerSpace::r25: 	return riscv64::x25;
            case registerSpace::r26: 	return riscv64::x26;
            case registerSpace::r27: 	return riscv64::x27;
            case registerSpace::r28: 	return riscv64::x28;
            case registerSpace::r29: 	return riscv64::x29;
            case registerSpace::r30: 	return riscv64::x30;
            case registerSpace::r31: 	return riscv64::x31;
            case registerSpace::fpr32_0:  return riscv64::f0_32;
            case registerSpace::fpr32_1:  return riscv64::f1_32;
            case registerSpace::fpr32_2:  return riscv64::f2_32;
            case registerSpace::fpr32_3:  return riscv64::f3_32;
            case registerSpace::fpr32_4:  return riscv64::f4_32;
            case registerSpace::fpr32_5:  return riscv64::f5_32;
            case registerSpace::fpr32_6:  return riscv64::f6_32;
            case registerSpace::fpr32_7:  return riscv64::f7_32;
            case registerSpace::fpr32_8:  return riscv64::f8_32;
            case registerSpace::fpr32_9:  return riscv64::f9_32;
            case registerSpace::fpr32_10: return riscv64::f10_32;
            case registerSpace::fpr32_11: return riscv64::f11_32;
            case registerSpace::fpr32_12: return riscv64::f12_32;
            case registerSpace::fpr32_13: return riscv64::f13_32;
            case registerSpace::fpr32_14: return riscv64::f14_32;
            case registerSpace::fpr32_15: return riscv64::f15_32;
            case registerSpace::fpr32_16: return riscv64::f16_32;
            case registerSpace::fpr32_17: return riscv64::f17_32;
            case registerSpace::fpr32_18: return riscv64::f18_32;
            case registerSpace::fpr32_19: return riscv64::f19_32;
            case registerSpace::fpr32_20: return riscv64::f20_32;
            case registerSpace::fpr32_21: return riscv64::f21_32;
            case registerSpace::fpr32_22: return riscv64::f22_32;
            case registerSpace::fpr32_23: return riscv64::f23_32;
            case registerSpace::fpr32_24: return riscv64::f24_32;
            case registerSpace::fpr32_25: return riscv64::f25_32;
            case registerSpace::fpr32_26: return riscv64::f26_32;
            case registerSpace::fpr32_27: return riscv64::f27_32;
            case registerSpace::fpr32_28: return riscv64::f28_32;
            case registerSpace::fpr32_29: return riscv64::f29_32;
            case registerSpace::fpr32_30: return riscv64::f30_32;
            case registerSpace::fpr32_31: return riscv64::f31_32;
            case registerSpace::fpr64_0:  return riscv64::f0_64;
            case registerSpace::fpr64_1:  return riscv64::f1_64;
            case registerSpace::fpr64_2:  return riscv64::f2_64;
            case registerSpace::fpr64_3:  return riscv64::f3_64;
            case registerSpace::fpr64_4:  return riscv64::f4_64;
            case registerSpace::fpr64_5:  return riscv64::f5_64;
            case registerSpace::fpr64_6:  return riscv64::f6_64;
            case registerSpace::fpr64_7:  return riscv64::f7_64;
            case registerSpace::fpr64_8:  return riscv64::f8_64;
            case registerSpace::fpr64_9:  return riscv64::f9_64;
            case registerSpace::fpr64_10: return riscv64::f10_64;
            case registerSpace::fpr64_11: return riscv64::f11_64;
            case registerSpace::fpr64_12: return riscv64::f12_64;
            case registerSpace::fpr64_13: return riscv64::f13_64;
            case registerSpace::fpr64_14: return riscv64::f14_64;
            case registerSpace::fpr64_15: return riscv64::f15_64;
            case registerSpace::fpr64_16: return riscv64::f16_64;
            case registerSpace::fpr64_17: return riscv64::f17_64;
            case registerSpace::fpr64_18: return riscv64::f18_64;
            case registerSpace::fpr64_19: return riscv64::f19_64;
            case registerSpace::fpr64_20: return riscv64::f20_64;
            case registerSpace::fpr64_21: return riscv64::f21_64;
            case registerSpace::fpr64_22: return riscv64::f22_64;
            case registerSpace::fpr64_23: return riscv64::f23_64;
            case registerSpace::fpr64_24: return riscv64::f24_64;
            case registerSpace::fpr64_25: return riscv64::f25_64;
            case registerSpace::fpr64_26: return riscv64::f26_64;
            case registerSpace::fpr64_27: return riscv64::f27_64;
            case registerSpace::fpr64_28: return riscv64::f28_64;
            case registerSpace::fpr64_29: return riscv64::f29_64;
            case registerSpace::fpr64_30: return riscv64::f30_64;
            case registerSpace::fpr64_31: return riscv64::f31_64;
            case registerSpace::pc: 	return riscv64::pc;
            default:
                break;
        }
    }else{
        assert(!"Invalid architecture");
    }

    assert(!"Register not handled");
    return InvalidReg;
}
