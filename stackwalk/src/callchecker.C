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

#include "stackwalk/h/swk_errors.h"
#include "stackwalk/src/sw.h"

using namespace Dyninst;
using namespace Stackwalker;
using namespace std;

CallChecker::CallChecker(ProcessState * proc_) : proc(proc_) {}
CallChecker::~CallChecker() {}

bool CallChecker::isPrevInstrACall(Address addr, Address & target)
{
    const unsigned max_call_length = 5;
   bool result;
   unsigned char buffer[max_call_length];

   sw_printf("[%s:%d] - isPrevInstrACall on %lx\n", FILE__, __LINE__, addr);
   Address start = addr - max_call_length;
   result = proc->readMem(buffer, start, max_call_length);
   if (!result)
   {
      sw_printf("[%s:%d] - Address 0x%lx is not a call--unreadable\n",
                FILE__, __LINE__, addr);
      return false;
   }

   if (buffer[max_call_length - 5] == 0xe8) {
      int32_t disp = *((int32_t *) (buffer+1));
      target = addr + disp;
      sw_printf("[%s:%d] - Found call encoded by %x to %lx (addr = %lx, disp = %dx)\n",
                FILE__, __LINE__, (int) buffer[0], target, addr, disp);
                
      return true;
   }

   target = 0x0;
   for (unsigned i=0; i<max_call_length-1; i++)
   {
      if (buffer[i] != 0xff) 
         continue;
      int modrm_reg = buffer[i+1] >> 3 & 7;
      if (modrm_reg != 2)
         continue;

      /**
       * Compute the size of the x86 instruction.
       **/
      int modrm_mod = buffer[i+1] >> 6;
      int modrm_rm = buffer[i+1] & 7;
      unsigned size = 2; //Opcode + MOD/RM
      switch (modrm_mod)
      {
         case 0:
            if (modrm_rm == 5)
               size += 4; //disp32
            if (modrm_rm == 4)
               size += 1; //SIB
            break;
         case 1:
            size += 1; //disp8
            if (modrm_rm == 4)
               size += 1; //SIB
            break;
         case 2:
            size += 4; //disp32
            if (modrm_rm == 4)
               size += 1; //SIB
            break;
         case 3:
            break;
      }

      if (i + size == max_call_length)
      {
         sw_printf("[%s:%d] - Found call of size %u encoded by: ",
                   FILE__, __LINE__, size);
         for (unsigned j=i; j<i+size; j++) {
            sw_printf("%x ", buffer[j]);
         }
         sw_printf("\n");

         return true;
      }
   }

   return false;
}
