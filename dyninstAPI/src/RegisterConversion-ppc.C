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
#include "registers/ppc32_regs.h"
#include "registers/abstract_regs.h"
#include "registerSpace.h"

using namespace Dyninst;
using namespace Dyninst::InstructionAPI;
using namespace std;
using namespace boost::assign;

multimap<Register, MachRegister> regToMachReg32 = map_list_of
  (registerSpace::r0, ppc32::r0)
  (registerSpace::r1, ppc32::r1)
  (registerSpace::r2, ppc32::r2)
  (registerSpace::r3, ppc32::r3)
  (registerSpace::r4, ppc32::r4)
  (registerSpace::r5, ppc32::r5)
  (registerSpace::r6, ppc32::r6)
  (registerSpace::r7, ppc32::r7)
  (registerSpace::r8, ppc32::r8)
  (registerSpace::r9, ppc32::r9)
  (registerSpace::r10, ppc32::r10)
  (registerSpace::r11, ppc32::r11)
  (registerSpace::r12, ppc32::r12)
  (registerSpace::r13, ppc32::r13)
  (registerSpace::r14, ppc32::r14)
  (registerSpace::r15, ppc32::r15)
  (registerSpace::r16, ppc32::r16)
  (registerSpace::r17, ppc32::r17)
  (registerSpace::r18, ppc32::r18)
  (registerSpace::r19, ppc32::r19)
  (registerSpace::r20, ppc32::r20)
  (registerSpace::r21, ppc32::r21)
  (registerSpace::r22, ppc32::r22)
  (registerSpace::r23, ppc32::r23)
  (registerSpace::r24, ppc32::r24)
  (registerSpace::r25, ppc32::r25)
  (registerSpace::r26, ppc32::r26)
  (registerSpace::r27, ppc32::r27)
  (registerSpace::r28, ppc32::r28)
  (registerSpace::r29, ppc32::r29)
  (registerSpace::r30, ppc32::r30)
  (registerSpace::r31, ppc32::r31)
  (registerSpace::fpr0, ppc32::fpr0)
  (registerSpace::fpr1, ppc32::fpr1)
  (registerSpace::fpr2, ppc32::fpr2)
  (registerSpace::fpr3, ppc32::fpr3)
  (registerSpace::fpr4, ppc32::fpr4)
  (registerSpace::fpr5, ppc32::fpr5)
  (registerSpace::fpr6, ppc32::fpr6)
  (registerSpace::fpr7, ppc32::fpr7)
  (registerSpace::fpr8, ppc32::fpr8)
  (registerSpace::fpr9, ppc32::fpr9)
  (registerSpace::fpr10, ppc32::fpr10)
  (registerSpace::fpr11, ppc32::fpr11)
  (registerSpace::fpr12, ppc32::fpr12)
  (registerSpace::fpr13, ppc32::fpr13)
  (registerSpace::fpr14, ppc32::fpr14)
  (registerSpace::fpr15, ppc32::fpr15)
  (registerSpace::fpr16, ppc32::fpr16)
  (registerSpace::fpr17, ppc32::fpr17)
  (registerSpace::fpr18, ppc32::fpr18)
  (registerSpace::fpr19, ppc32::fpr19)
  (registerSpace::fpr20, ppc32::fpr20)
  (registerSpace::fpr21, ppc32::fpr21)
  (registerSpace::fpr22, ppc32::fpr22)
  (registerSpace::fpr23, ppc32::fpr23)
  (registerSpace::fpr24, ppc32::fpr24)
  (registerSpace::fpr25, ppc32::fpr25)
  (registerSpace::fpr26, ppc32::fpr26)
  (registerSpace::fpr27, ppc32::fpr27)
  (registerSpace::fpr28, ppc32::fpr28)
  (registerSpace::fpr29, ppc32::fpr29)
  (registerSpace::fpr30, ppc32::fpr30)
  (registerSpace::fpr31, ppc32::fpr31)
  (registerSpace::xer, ppc32::xer)
  (registerSpace::lr, ppc32::lr)
  (registerSpace::ctr, ppc32::ctr)
  (registerSpace::mq, ppc32::mq)
  (registerSpace::cr, ppc32::cr);

multimap<Register, MachRegister> regToMachReg64 = map_list_of
  (registerSpace::r0, ppc32::r0)
  (registerSpace::r1, ppc32::r1)
  (registerSpace::r2, ppc32::r2)
  (registerSpace::r3, ppc32::r3)
  (registerSpace::r4, ppc32::r4)
  (registerSpace::r5, ppc32::r5)
  (registerSpace::r6, ppc32::r6)
  (registerSpace::r7, ppc32::r7)
  (registerSpace::r8, ppc32::r8)
  (registerSpace::r9, ppc32::r9)
  (registerSpace::r10, ppc32::r10)
  (registerSpace::r11, ppc32::r11)
  (registerSpace::r12, ppc32::r12)
  (registerSpace::r13, ppc32::r13)
  (registerSpace::r14, ppc32::r14)
  (registerSpace::r15, ppc32::r15)
  (registerSpace::r16, ppc32::r16)
  (registerSpace::r17, ppc32::r17)
  (registerSpace::r18, ppc32::r18)
  (registerSpace::r19, ppc32::r19)
  (registerSpace::r20, ppc32::r20)
  (registerSpace::r21, ppc32::r21)
  (registerSpace::r22, ppc32::r22)
  (registerSpace::r23, ppc32::r23)
  (registerSpace::r24, ppc32::r24)
  (registerSpace::r25, ppc32::r25)
  (registerSpace::r26, ppc32::r26)
  (registerSpace::r27, ppc32::r27)
  (registerSpace::r28, ppc32::r28)
  (registerSpace::r29, ppc32::r29)
  (registerSpace::r30, ppc32::r30)
  (registerSpace::r31, ppc32::r31)
  (registerSpace::fpr0, ppc32::fpr0)
  (registerSpace::fpr1, ppc32::fpr1)
  (registerSpace::fpr2, ppc32::fpr2)
  (registerSpace::fpr3, ppc32::fpr3)
  (registerSpace::fpr4, ppc32::fpr4)
  (registerSpace::fpr5, ppc32::fpr5)
  (registerSpace::fpr6, ppc32::fpr6)
  (registerSpace::fpr7, ppc32::fpr7)
  (registerSpace::fpr8, ppc32::fpr8)
  (registerSpace::fpr9, ppc32::fpr9)
  (registerSpace::fpr10, ppc32::fpr10)
  (registerSpace::fpr11, ppc32::fpr11)
  (registerSpace::fpr12, ppc32::fpr12)
  (registerSpace::fpr13, ppc32::fpr13)
  (registerSpace::fpr14, ppc32::fpr14)
  (registerSpace::fpr15, ppc32::fpr15)
  (registerSpace::fpr16, ppc32::fpr16)
  (registerSpace::fpr17, ppc32::fpr17)
  (registerSpace::fpr18, ppc32::fpr18)
  (registerSpace::fpr19, ppc32::fpr19)
  (registerSpace::fpr20, ppc32::fpr20)
  (registerSpace::fpr21, ppc32::fpr21)
  (registerSpace::fpr22, ppc32::fpr22)
  (registerSpace::fpr23, ppc32::fpr23)
  (registerSpace::fpr24, ppc32::fpr24)
  (registerSpace::fpr25, ppc32::fpr25)
  (registerSpace::fpr26, ppc32::fpr26)
  (registerSpace::fpr27, ppc32::fpr27)
  (registerSpace::fpr28, ppc32::fpr28)
  (registerSpace::fpr29, ppc32::fpr29)
  (registerSpace::fpr30, ppc32::fpr30)
  (registerSpace::fpr31, ppc32::fpr31)
  (registerSpace::xer, ppc32::xer)
  (registerSpace::lr, ppc32::lr)
  (registerSpace::ctr, ppc32::ctr)
  (registerSpace::mq, ppc32::mq)
  (registerSpace::cr, ppc32::cr);

/* regToMachReg64 should be this, but for consistency in liveness
   and instructionAPI, we only deal with ppc32 registers

multimap<Register, MachRegister> regToMachReg64 = map_list_of
  (registerSpace::r0, ppc64::r0)
  (registerSpace::r1, ppc64::r1)
  (registerSpace::r2, ppc64::r2)
  (registerSpace::r3, ppc64::r3)
  (registerSpace::r4, ppc64::r4)
  (registerSpace::r5, ppc64::r5)
  (registerSpace::r6, ppc64::r6)
  (registerSpace::r7, ppc64::r7)
  (registerSpace::r8, ppc64::r8)
  (registerSpace::r9, ppc64::r9)
  (registerSpace::r10, ppc64::r10)
  (registerSpace::r11, ppc64::r11)
  (registerSpace::r12, ppc64::r12)
  (registerSpace::r13, ppc64::r13)
  (registerSpace::r14, ppc64::r14)
  (registerSpace::r15, ppc64::r15)
  (registerSpace::r16, ppc64::r16)
  (registerSpace::r17, ppc64::r17)
  (registerSpace::r18, ppc64::r18)
  (registerSpace::r19, ppc64::r19)
  (registerSpace::r20, ppc64::r20)
  (registerSpace::r21, ppc64::r21)
  (registerSpace::r22, ppc64::r22)
  (registerSpace::r23, ppc64::r23)
  (registerSpace::r24, ppc64::r24)
  (registerSpace::r25, ppc64::r25)
  (registerSpace::r26, ppc64::r26)
  (registerSpace::r27, ppc64::r27)
  (registerSpace::r28, ppc64::r28)
  (registerSpace::r29, ppc64::r29)
  (registerSpace::r30, ppc64::r30)
  (registerSpace::r31, ppc64::r31)
  (registerSpace::fpr0, ppc64::fpr0)
  (registerSpace::fpr1, ppc64::fpr1)
  (registerSpace::fpr2, ppc64::fpr2)
  (registerSpace::fpr3, ppc64::fpr3)
  (registerSpace::fpr4, ppc64::fpr4)
  (registerSpace::fpr5, ppc64::fpr5)
  (registerSpace::fpr6, ppc64::fpr6)
  (registerSpace::fpr7, ppc64::fpr7)
  (registerSpace::fpr8, ppc64::fpr8)
  (registerSpace::fpr9, ppc64::fpr9)
  (registerSpace::fpr10, ppc64::fpr10)
  (registerSpace::fpr11, ppc64::fpr11)
  (registerSpace::fpr12, ppc64::fpr12)
  (registerSpace::fpr13, ppc64::fpr13)
  (registerSpace::fpr14, ppc64::fpr14)
  (registerSpace::fpr15, ppc64::fpr15)
  (registerSpace::fpr16, ppc64::fpr16)
  (registerSpace::fpr17, ppc64::fpr17)
  (registerSpace::fpr18, ppc64::fpr18)
  (registerSpace::fpr19, ppc64::fpr19)
  (registerSpace::fpr20, ppc64::fpr20)
  (registerSpace::fpr21, ppc64::fpr21)
  (registerSpace::fpr22, ppc64::fpr22)
  (registerSpace::fpr23, ppc64::fpr23)
  (registerSpace::fpr24, ppc64::fpr24)
  (registerSpace::fpr25, ppc64::fpr25)
  (registerSpace::fpr26, ppc64::fpr26)
  (registerSpace::fpr27, ppc64::fpr27)
  (registerSpace::fpr28, ppc64::fpr28)
  (registerSpace::fpr29, ppc64::fpr29)
  (registerSpace::fpr30, ppc64::fpr30)
  (registerSpace::fpr31, ppc64::fpr31)
  (registerSpace::xer, ppc64::xer)
  (registerSpace::lr, ppc64::lr)
  (registerSpace::ctr, ppc64::ctr)
  (registerSpace::mq, ppc64::mq)
  (registerSpace::cr, ppc64::cr);
*/

map<MachRegister, Register> reverseRegisterMap = map_list_of
  (ppc32::r0, registerSpace::r0)
  (ppc32::r1, registerSpace::r1)
  (ppc32::r2, registerSpace::r2)
  (ppc32::r3, registerSpace::r3)
  (ppc32::r4, registerSpace::r4)
  (ppc32::r5, registerSpace::r5)
  (ppc32::r6, registerSpace::r6)
  (ppc32::r7, registerSpace::r7)
  (ppc32::r8, registerSpace::r8)
  (ppc32::r9, registerSpace::r9)
  (ppc32::r10, registerSpace::r10)
  (ppc32::r11, registerSpace::r11)
  (ppc32::r12, registerSpace::r12)
  (ppc32::r13, registerSpace::r13)
  (ppc32::r14, registerSpace::r14)
  (ppc32::r15, registerSpace::r15)
  (ppc32::r16, registerSpace::r16)
  (ppc32::r17, registerSpace::r17)
  (ppc32::r18, registerSpace::r18)
  (ppc32::r19, registerSpace::r19)
  (ppc32::r20, registerSpace::r20)
  (ppc32::r21, registerSpace::r21)
  (ppc32::r22, registerSpace::r22)
  (ppc32::r23, registerSpace::r23)
  (ppc32::r24, registerSpace::r24)
  (ppc32::r25, registerSpace::r25)
  (ppc32::r26, registerSpace::r26)
  (ppc32::r27, registerSpace::r27)
  (ppc32::r28, registerSpace::r28)
  (ppc32::r29, registerSpace::r29)
  (ppc32::r30, registerSpace::r30)
  (ppc32::r31, registerSpace::r31)
  (ppc32::fpr0, registerSpace::fpr0)
  (ppc32::fpr1, registerSpace::fpr1)
  (ppc32::fpr2, registerSpace::fpr2)
  (ppc32::fpr3, registerSpace::fpr3)
  (ppc32::fpr4, registerSpace::fpr4)
  (ppc32::fpr5, registerSpace::fpr5)
  (ppc32::fpr6, registerSpace::fpr6)
  (ppc32::fpr7, registerSpace::fpr7)
  (ppc32::fpr8, registerSpace::fpr8)
  (ppc32::fpr9, registerSpace::fpr9)
  (ppc32::fpr10, registerSpace::fpr10)
  (ppc32::fpr11, registerSpace::fpr11)
  (ppc32::fpr12, registerSpace::fpr12)
  (ppc32::fpr13, registerSpace::fpr13)
  (ppc32::fpr14, registerSpace::fpr14)
  (ppc32::fpr15, registerSpace::fpr15)
  (ppc32::fpr16, registerSpace::fpr16)
  (ppc32::fpr17, registerSpace::fpr17)
  (ppc32::fpr18, registerSpace::fpr18)
  (ppc32::fpr19, registerSpace::fpr19)
  (ppc32::fpr20, registerSpace::fpr20)
  (ppc32::fpr21, registerSpace::fpr21)
  (ppc32::fpr22, registerSpace::fpr22)
  (ppc32::fpr23, registerSpace::fpr23)
  (ppc32::fpr24, registerSpace::fpr24)
  (ppc32::fpr25, registerSpace::fpr25)
  (ppc32::fpr26, registerSpace::fpr26)
  (ppc32::fpr27, registerSpace::fpr27)
  (ppc32::fpr28, registerSpace::fpr28)
  (ppc32::fpr29, registerSpace::fpr29)
  (ppc32::fpr30, registerSpace::fpr30)
  (ppc32::fpr31, registerSpace::fpr31)
  (ppc32::xer, registerSpace::xer)
  (ppc32::lr, registerSpace::lr)
  (ppc32::ctr, registerSpace::ctr)
  (ppc32::mq, registerSpace::mq)
  (ppc32::cr, registerSpace::cr);

Register convertRegID(MachRegister reg, bool &wasUpcast) {
    wasUpcast = false;

    MachRegister baseReg = MachRegister((reg.getBaseRegister().val() & ~reg.getArchitecture()) | Arch_ppc32);
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
    if( arch == Arch_ppc32 ) {
        switch(r) {
            case registerSpace::r0: return ppc32::r0;
            case registerSpace::r1: return ppc32::r1;
            case registerSpace::r2: return ppc32::r2;
            case registerSpace::r3: return ppc32::r3;
            case registerSpace::r4: return ppc32::r4;
            case registerSpace::r5: return ppc32::r5;
            case registerSpace::r6: return ppc32::r6;
            case registerSpace::r7: return ppc32::r7;
            case registerSpace::r8: return ppc32::r8;
            case registerSpace::r9: return ppc32::r9;
            case registerSpace::r10: return ppc32::r10;
            case registerSpace::r11: return ppc32::r11;
            case registerSpace::r12: return ppc32::r12;
            case registerSpace::r13: return ppc32::r13;
            case registerSpace::r14: return ppc32::r14;
            case registerSpace::r15: return ppc32::r15;
            case registerSpace::r16: return ppc32::r16;
            case registerSpace::r17: return ppc32::r17;
            case registerSpace::r18: return ppc32::r18;
            case registerSpace::r19: return ppc32::r19;
            case registerSpace::r20: return ppc32::r20;
            case registerSpace::r21: return ppc32::r21;
            case registerSpace::r22: return ppc32::r22;
            case registerSpace::r23: return ppc32::r23;
            case registerSpace::r24: return ppc32::r24;
            case registerSpace::r25: return ppc32::r25;
            case registerSpace::r26: return ppc32::r26;
            case registerSpace::r27: return ppc32::r27;
            case registerSpace::r28: return ppc32::r28;
            case registerSpace::r29: return ppc32::r29;
            case registerSpace::r30: return ppc32::r30;
            case registerSpace::r31: return ppc32::r31;
            case registerSpace::fpr0: return ppc32::fpr0;
            case registerSpace::fpr1: return ppc32::fpr1;
            case registerSpace::fpr2: return ppc32::fpr2;
            case registerSpace::fpr3: return ppc32::fpr3;
            case registerSpace::fpr4: return ppc32::fpr4;
            case registerSpace::fpr5: return ppc32::fpr5;
            case registerSpace::fpr6: return ppc32::fpr6;
            case registerSpace::fpr7: return ppc32::fpr7;
            case registerSpace::fpr8: return ppc32::fpr8;
            case registerSpace::fpr9: return ppc32::fpr9;
            case registerSpace::fpr10: return ppc32::fpr10;
            case registerSpace::fpr11: return ppc32::fpr11;
            case registerSpace::fpr12: return ppc32::fpr12;
            case registerSpace::fpr13: return ppc32::fpr13;
            case registerSpace::fpr14: return ppc32::fpr14;
            case registerSpace::fpr15: return ppc32::fpr15;
            case registerSpace::fpr16: return ppc32::fpr16;
            case registerSpace::fpr17: return ppc32::fpr17;
            case registerSpace::fpr18: return ppc32::fpr18;
            case registerSpace::fpr19: return ppc32::fpr19;
            case registerSpace::fpr20: return ppc32::fpr20;
            case registerSpace::fpr21: return ppc32::fpr21;
            case registerSpace::fpr22: return ppc32::fpr22;
            case registerSpace::fpr23: return ppc32::fpr23;
            case registerSpace::fpr24: return ppc32::fpr24;
            case registerSpace::fpr25: return ppc32::fpr25;
            case registerSpace::fpr26: return ppc32::fpr26;
            case registerSpace::fpr27: return ppc32::fpr27;
            case registerSpace::fpr28: return ppc32::fpr28;
            case registerSpace::fpr29: return ppc32::fpr29;
            case registerSpace::fpr30: return ppc32::fpr30;
            case registerSpace::fpr31: return ppc32::fpr31;
            case registerSpace::xer: return ppc32::xer;
            case registerSpace::lr: return ppc32::lr;
            case registerSpace::ctr: return ppc32::ctr;
            case registerSpace::mq: return ppc32::mq;
            case registerSpace::cr: return ppc32::cr0;
            default:
                break;
        }
    }else if( arch == Arch_ppc64 ) {
        switch(r) {
            case registerSpace::r0: return ppc64::r0;
            case registerSpace::r1: return ppc64::r1;
            case registerSpace::r2: return ppc64::r2;
            case registerSpace::r3: return ppc64::r3;
            case registerSpace::r4: return ppc64::r4;
            case registerSpace::r5: return ppc64::r5;
            case registerSpace::r6: return ppc64::r6;
            case registerSpace::r7: return ppc64::r7;
            case registerSpace::r8: return ppc64::r8;
            case registerSpace::r9: return ppc64::r9;
            case registerSpace::r10: return ppc64::r10;
            case registerSpace::r11: return ppc64::r11;
            case registerSpace::r12: return ppc64::r12;
            case registerSpace::r13: return ppc64::r13;
            case registerSpace::r14: return ppc64::r14;
            case registerSpace::r15: return ppc64::r15;
            case registerSpace::r16: return ppc64::r16;
            case registerSpace::r17: return ppc64::r17;
            case registerSpace::r18: return ppc64::r18;
            case registerSpace::r19: return ppc64::r19;
            case registerSpace::r20: return ppc64::r20;
            case registerSpace::r21: return ppc64::r21;
            case registerSpace::r22: return ppc64::r22;
            case registerSpace::r23: return ppc64::r23;
            case registerSpace::r24: return ppc64::r24;
            case registerSpace::r25: return ppc64::r25;
            case registerSpace::r26: return ppc64::r26;
            case registerSpace::r27: return ppc64::r27;
            case registerSpace::r28: return ppc64::r28;
            case registerSpace::r29: return ppc64::r29;
            case registerSpace::r30: return ppc64::r30;
            case registerSpace::r31: return ppc64::r31;
            case registerSpace::fpr0: return ppc64::fpr0;
            case registerSpace::fpr1: return ppc64::fpr1;
            case registerSpace::fpr2: return ppc64::fpr2;
            case registerSpace::fpr3: return ppc64::fpr3;
            case registerSpace::fpr4: return ppc64::fpr4;
            case registerSpace::fpr5: return ppc64::fpr5;
            case registerSpace::fpr6: return ppc64::fpr6;
            case registerSpace::fpr7: return ppc64::fpr7;
            case registerSpace::fpr8: return ppc64::fpr8;
            case registerSpace::fpr9: return ppc64::fpr9;
            case registerSpace::fpr10: return ppc64::fpr10;
            case registerSpace::fpr11: return ppc64::fpr11;
            case registerSpace::fpr12: return ppc64::fpr12;
            case registerSpace::fpr13: return ppc64::fpr13;
            case registerSpace::fpr14: return ppc64::fpr14;
            case registerSpace::fpr15: return ppc64::fpr15;
            case registerSpace::fpr16: return ppc64::fpr16;
            case registerSpace::fpr17: return ppc64::fpr17;
            case registerSpace::fpr18: return ppc64::fpr18;
            case registerSpace::fpr19: return ppc64::fpr19;
            case registerSpace::fpr20: return ppc64::fpr20;
            case registerSpace::fpr21: return ppc64::fpr21;
            case registerSpace::fpr22: return ppc64::fpr22;
            case registerSpace::fpr23: return ppc64::fpr23;
            case registerSpace::fpr24: return ppc64::fpr24;
            case registerSpace::fpr25: return ppc64::fpr25;
            case registerSpace::fpr26: return ppc64::fpr26;
            case registerSpace::fpr27: return ppc64::fpr27;
            case registerSpace::fpr28: return ppc64::fpr28;
            case registerSpace::fpr29: return ppc64::fpr29;
            case registerSpace::fpr30: return ppc64::fpr30;
            case registerSpace::fpr31: return ppc64::fpr31;
            case registerSpace::xer: return ppc64::xer;
            case registerSpace::lr: return ppc64::lr;
            case registerSpace::ctr: return ppc64::ctr;
            case registerSpace::mq: return ppc64::mq;
            case registerSpace::cr: return ppc64::cr0;
            default:
                break;
        }
    }else{
        assert(!"Invalid architecture");
    }

    assert(!"Register not handled");
    return InvalidReg;
}
