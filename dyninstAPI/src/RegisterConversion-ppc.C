/*
 * Copyright (c) 1996-2011 Barton P. Miller
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
 * MERCHANTppc32LITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */
#if defined(arch_power)
#include "dyninstAPI/src/RegisterConversion.h"
#include "dyninstAPI/src/registerSpace.h"

#include <map>
#include <boost/assign/list_of.hpp>
#include "Register.h"
#include "dynutil/h/dyn_regs.h"

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
#endif
