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
#include "dyn_regs.h"
#include "registerSpace.h"

using namespace Dyninst;
using namespace Dyninst::InstructionAPI;
using namespace std;
using namespace boost::assign;

#warning "This file is not verified yet!"
multimap<Register, MachRegister> regToMachReg64 = map_list_of
  (registerSpace::r0,  		aarch64::x0)
  (registerSpace::r1,  		aarch64::x1)
  (registerSpace::r2,  		aarch64::x2)
  (registerSpace::r3,  		aarch64::x3)
  (registerSpace::r4,  		aarch64::x4)
  (registerSpace::r5,  		aarch64::x5)
  (registerSpace::r6,  		aarch64::x6)
  (registerSpace::r7,  		aarch64::x7)
  (registerSpace::r8,  		aarch64::x8)
  (registerSpace::r9,  		aarch64::x9)
  (registerSpace::r10, 		aarch64::x10)
  (registerSpace::r11, 		aarch64::x11)
  (registerSpace::r12, 		aarch64::x12)
  (registerSpace::r13, 		aarch64::x13)
  (registerSpace::r14, 		aarch64::x14)
  (registerSpace::r15, 		aarch64::x15)
  (registerSpace::r16, 		aarch64::x16)
  (registerSpace::r17, 		aarch64::x17)
  (registerSpace::r18, 		aarch64::x18)
  (registerSpace::r19, 		aarch64::x19)
  (registerSpace::r20, 		aarch64::x20)
  (registerSpace::r21, 		aarch64::x21)
  (registerSpace::r22, 		aarch64::x22)
  (registerSpace::r23, 		aarch64::x23)
  (registerSpace::r24, 		aarch64::x24)
  (registerSpace::r25, 		aarch64::x25)
  (registerSpace::r26, 		aarch64::x26)
  (registerSpace::r27, 		aarch64::x27)
  (registerSpace::r28, 		aarch64::x28)
  (registerSpace::r29, 		aarch64::x29)
  (registerSpace::r30, 		aarch64::x30)
  (registerSpace::fpr0,  	aarch64::fpr0)
  (registerSpace::fpr1,  	aarch64::fpr1)
  (registerSpace::fpr2,  	aarch64::fpr2)
  (registerSpace::fpr3,  	aarch64::fpr3)
  (registerSpace::fpr4,  	aarch64::fpr4)
  (registerSpace::fpr5,  	aarch64::fpr5)
  (registerSpace::fpr6,  	aarch64::fpr6)
  (registerSpace::fpr7,  	aarch64::fpr7)
  (registerSpace::fpr8,  	aarch64::fpr8)
  (registerSpace::fpr9,  	aarch64::fpr9)
  (registerSpace::fpr10, 	aarch64::fpr10)
  (registerSpace::fpr11, 	aarch64::fpr11)
  (registerSpace::fpr12, 	aarch64::fpr12)
  (registerSpace::fpr13, 	aarch64::fpr13)
  (registerSpace::fpr14, 	aarch64::fpr14)
  (registerSpace::fpr15, 	aarch64::fpr15)
  (registerSpace::fpr16, 	aarch64::fpr16)
  (registerSpace::fpr17, 	aarch64::fpr17)
  (registerSpace::fpr18, 	aarch64::fpr18)
  (registerSpace::fpr19, 	aarch64::fpr19)
  (registerSpace::fpr20, 	aarch64::fpr20)
  (registerSpace::fpr21, 	aarch64::fpr21)
  (registerSpace::fpr22, 	aarch64::fpr22)
  (registerSpace::fpr23, 	aarch64::fpr23)
  (registerSpace::fpr24, 	aarch64::fpr24)
  (registerSpace::fpr25, 	aarch64::fpr25)
  (registerSpace::fpr26, 	aarch64::fpr26)
  (registerSpace::fpr27, 	aarch64::fpr27)
  (registerSpace::fpr28, 	aarch64::fpr28)
  (registerSpace::fpr29, 	aarch64::fpr29)
  (registerSpace::fpr30, 	aarch64::fpr30)
  (registerSpace::fpr31, 	aarch64::fpr31)
  (registerSpace::xer, 		aarch64::xer)
  (registerSpace::lr, 		aarch64::lr)
  (registerSpace::ctr, 		aarch64::ctr)
  (registerSpace::mq, 		aarch64::mq)
  (registerSpace::cr, 		aarch64::cr);

map<MachRegister, Register> reverseRegisterMap = map_list_of
  (aarch64::x0,  registerSpace::r0)
  (aarch64::x1,  registerSpace::r1)
  (aarch64::x2,  registerSpace::r2)
  (aarch64::x3,  registerSpace::r3)
  (aarch64::x4,  registerSpace::r4)
  (aarch64::x5,  registerSpace::r5)
  (aarch64::x6,  registerSpace::r6)
  (aarch64::x7,  registerSpace::r7)
  (aarch64::x8,  registerSpace::r8)
  (aarch64::x9,  registerSpace::r9)
  (aarch64::x10, registerSpace::r10)
  (aarch64::x11, registerSpace::r11)
  (aarch64::x12, registerSpace::r12)
  (aarch64::x13, registerSpace::r13)
  (aarch64::x14, registerSpace::r14)
  (aarch64::x15, registerSpace::r15)
  (aarch64::x16, registerSpace::r16)
  (aarch64::x17, registerSpace::r17)
  (aarch64::x18, registerSpace::r18)
  (aarch64::x19, registerSpace::r19)
  (aarch64::x20, registerSpace::r20)
  (aarch64::x21, registerSpace::r21)
  (aarch64::x22, registerSpace::r22)
  (aarch64::x23, registerSpace::r23)
  (aarch64::x24, registerSpace::r24)
  (aarch64::x25, registerSpace::r25)
  (aarch64::x26, registerSpace::r26)
  (aarch64::x27, registerSpace::r27)
  (aarch64::x28, registerSpace::r28)
  (aarch64::x29, registerSpace::r29)
  (aarch64::x30, registerSpace::r30)
  (aarch64::fpr0, registerSpace::fpr0)
  (aarch64::fpr1, registerSpace::fpr1)
  (aarch64::fpr2, registerSpace::fpr2)
  (aarch64::fpr3, registerSpace::fpr3)
  (aarch64::fpr4, registerSpace::fpr4)
  (aarch64::fpr5, registerSpace::fpr5)
  (aarch64::fpr6, registerSpace::fpr6)
  (aarch64::fpr7, registerSpace::fpr7)
  (aarch64::fpr8, registerSpace::fpr8)
  (aarch64::fpr9, registerSpace::fpr9)
  (aarch64::fpr10, registerSpace::fpr10)
  (aarch64::fpr11, registerSpace::fpr11)
  (aarch64::fpr12, registerSpace::fpr12)
  (aarch64::fpr13, registerSpace::fpr13)
  (aarch64::fpr14, registerSpace::fpr14)
  (aarch64::fpr15, registerSpace::fpr15)
  (aarch64::fpr16, registerSpace::fpr16)
  (aarch64::fpr17, registerSpace::fpr17)
  (aarch64::fpr18, registerSpace::fpr18)
  (aarch64::fpr19, registerSpace::fpr19)
  (aarch64::fpr20, registerSpace::fpr20)
  (aarch64::fpr21, registerSpace::fpr21)
  (aarch64::fpr22, registerSpace::fpr22)
  (aarch64::fpr23, registerSpace::fpr23)
  (aarch64::fpr24, registerSpace::fpr24)
  (aarch64::fpr25, registerSpace::fpr25)
  (aarch64::fpr26, registerSpace::fpr26)
  (aarch64::fpr27, registerSpace::fpr27)
  (aarch64::fpr28, registerSpace::fpr28)
  (aarch64::fpr29, registerSpace::fpr29)
  (aarch64::fpr30, registerSpace::fpr30)
  (aarch64::fpr31, registerSpace::fpr31)
  (aarch64::xer, 		registerSpace::xer)
  (aarch64::lr, 		registerSpace::lr)
  (aarch64::ctr, 		registerSpace::ctr)
  (aarch64::mq, 		registerSpace::mq)
  (aarch64::cr, 		registerSpace::cr);

Register convertRegID(MachRegister reg, bool &wasUpcast) {
    wasUpcast = false;

    MachRegister baseReg = MachRegister((reg.getBaseRegister().val() & ~reg.getArchitecture()) | Arch_aarch64);
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
    if( arch == Arch_aarch64 ) {
        switch(r) {
            case registerSpace::r0: 	return aarch64::x0;
            case registerSpace::r1: 	return aarch64::x1;
            case registerSpace::r2: 	return aarch64::x2;
            case registerSpace::r3: 	return aarch64::x3;
            case registerSpace::r4: 	return aarch64::x4;
            case registerSpace::r5: 	return aarch64::x5;
            case registerSpace::r6: 	return aarch64::x6;
            case registerSpace::r7: 	return aarch64::x7;
            case registerSpace::r8: 	return aarch64::x8;
            case registerSpace::r9: 	return aarch64::x9;
            case registerSpace::r10: 	return aarch64::x10;
            case registerSpace::r11: 	return aarch64::x11;
            case registerSpace::r12: 	return aarch64::x12;
            case registerSpace::r13: 	return aarch64::x13;
            case registerSpace::r14: 	return aarch64::x14;
            case registerSpace::r15: 	return aarch64::x15;
            case registerSpace::r16: 	return aarch64::x16;
            case registerSpace::r17: 	return aarch64::x17;
            case registerSpace::r18: 	return aarch64::x18;
            case registerSpace::r19: 	return aarch64::x19;
            case registerSpace::r20: 	return aarch64::x20;
            case registerSpace::r21: 	return aarch64::x21;
            case registerSpace::r22: 	return aarch64::x22;
            case registerSpace::r23: 	return aarch64::x23;
            case registerSpace::r24: 	return aarch64::x24;
            case registerSpace::r25: 	return aarch64::x25;
            case registerSpace::r26: 	return aarch64::x26;
            case registerSpace::r27: 	return aarch64::x27;
            case registerSpace::r28: 	return aarch64::x28;
            case registerSpace::r29: 	return aarch64::x29;
            case registerSpace::r30: 	return aarch64::x30;
            case registerSpace::fpr0: return aarch64::fpr0;
            case registerSpace::fpr1: return aarch64::fpr1;
            case registerSpace::fpr2: return aarch64::fpr2;
            case registerSpace::fpr3: return aarch64::fpr3;
            case registerSpace::fpr4: return aarch64::fpr4;
            case registerSpace::fpr5: return aarch64::fpr5;
            case registerSpace::fpr6: return aarch64::fpr6;
            case registerSpace::fpr7: return aarch64::fpr7;
            case registerSpace::fpr8: return aarch64::fpr8;
            case registerSpace::fpr9: return aarch64::fpr9;
            case registerSpace::fpr10: return aarch64::fpr10;
            case registerSpace::fpr11: return aarch64::fpr11;
            case registerSpace::fpr12: return aarch64::fpr12;
            case registerSpace::fpr13: return aarch64::fpr13;
            case registerSpace::fpr14: return aarch64::fpr14;
            case registerSpace::fpr15: return aarch64::fpr15;
            case registerSpace::fpr16: return aarch64::fpr16;
            case registerSpace::fpr17: return aarch64::fpr17;
            case registerSpace::fpr18: return aarch64::fpr18;
            case registerSpace::fpr19: return aarch64::fpr19;
            case registerSpace::fpr20: return aarch64::fpr20;
            case registerSpace::fpr21: return aarch64::fpr21;
            case registerSpace::fpr22: return aarch64::fpr22;
            case registerSpace::fpr23: return aarch64::fpr23;
            case registerSpace::fpr24: return aarch64::fpr24;
            case registerSpace::fpr25: return aarch64::fpr25;
            case registerSpace::fpr26: return aarch64::fpr26;
            case registerSpace::fpr27: return aarch64::fpr27;
            case registerSpace::fpr28: return aarch64::fpr28;
            case registerSpace::fpr29: return aarch64::fpr29;
            case registerSpace::fpr30: return aarch64::fpr30;
            case registerSpace::fpr31: return aarch64::fpr31;
            case registerSpace::xer: 		return aarch64::xer;
            case registerSpace::lr: 		return aarch64::lr;
            case registerSpace::ctr: 		return aarch64::ctr;
            case registerSpace::mq: 		return aarch64::mq;
            case registerSpace::cr: 		return aarch64::cr;
            default:
                break;
        }
    }else{
        assert(!"Invalid architecture");
    }

    assert(!"Register not handled");
    return InvalidReg;
}
