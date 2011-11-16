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

using namespace Dyninst;
using namespace Dyninst::InstructionAPI;
using namespace std;
using namespace boost::assign;

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
