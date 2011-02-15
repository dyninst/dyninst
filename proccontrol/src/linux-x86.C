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

#include "linux.h"

/* ptrace code specific to Linux x86 */

// This header is why this code is in a separate file
#include <asm/ldt.h>

bool linux_thread::getSegmentBase(Dyninst::MachRegister reg, Dyninst::MachRegisterVal &val)
{
   switch (llproc()->getTargetArch())
   {
      case Arch_x86_64:
         // TODO
         // use ptrace_arch_prctl     
         pthrd_printf("Segment bases on x86_64 not implemented\n");
         return false;
      case Arch_x86: {
         MachRegister segmentSelectorReg;
         MachRegisterVal segmentSelectorVal;
         unsigned long entryNumber;
         struct user_desc entryDesc;

         switch (reg.val())
         {
            case x86::ifsbase: segmentSelectorReg = x86::fs; break;
            case x86::igsbase: segmentSelectorReg = x86::gs; break;
            default: {
               pthrd_printf("Failed to get unrecognized segment base\n");
               return false;
            }
         }

         if (!plat_getRegister(segmentSelectorReg, segmentSelectorVal))
         {
           pthrd_printf("Failed to get segment base with selector %s\n", segmentSelectorReg.name());
           return false;
         }
         entryNumber = segmentSelectorVal / 8;

         pthrd_printf("Get segment base doing PTRACE with entry %lu\n", entryNumber);
         long result = do_ptrace((pt_req) PTRACE_GET_THREAD_AREA, 
                                 lwp, (void *) entryNumber, (void *) &entryDesc);
         if (result == -1 && errno != 0) {
            pthrd_printf("PTRACE to get segment base failed: %s\n", strerror(errno));
            return false;
         }

         val = entryDesc.base_addr;
         pthrd_printf("Got segment base: 0x%lx\n", val);
         return true;
      }
      default:
         assert(0);
   }
}
