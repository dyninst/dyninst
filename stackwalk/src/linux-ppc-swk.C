/*
 * Copyright (c) 1996-2007 Barton P. Miller
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include "stackwalk/h/swk_errors.h"
#include "stackwalk/h/walker.h"
#include "stackwalk/h/basetypes.h"

using namespace Dyninst;
using namespace Dyninst::Stackwalker;

unsigned ProcDebugLinux::getAddressWidth()
{
   return sizeof(void *);
}

bool ProcDebugLinux::getRegValue(reg_t reg, THR_ID t, regval_t &val)
{
   struct pt_regs gprs;
   int result;

   result = ptrace(PTRACE_GETREGS, (pid_t) t, NULL, &gprs);
   if (result != 0)
   {
      int errnum = errno;
      sw_printf("[%s:%u] - Could not read gprs in %d: %s\n", 
                __FILE__, __LINE__, t, strerror(errnum));
      setLastError(err_procread, "Could not read registers from process");
      return false;
   }
   
   switch (reg)
   {
      case REG_PC:
         val = regs.nip;
         break;
      case REG_SP:
         val = 0x0;
         break;
      case REG_FP:
         val = regs.gpr[1];
         break;
      default:
         sw_printf("[%s:%u] - Request for unsupported register %d\n",
                   __FILE__, __LINE__, reg);
         setLastError(err_badparam, "Unknown register passed in reg field");
         return false;
   }
   return true;
}

bool Walker::createDefaultSteppers()
{
  FrameStepper *stepper;
  bool result;

  stepper = new FrameFuncStepper(this);
  result = addStepper(stepper);
  if (!result) {
    sw_printf("[%s:%u] - Error adding stepper %p\n", __FILE__, __LINE__,
	      stepper);
    return false;
  }

  return true;
}
