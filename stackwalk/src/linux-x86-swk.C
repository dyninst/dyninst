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
#include "stackwalk/h/procstate.h"
#include "stackwalk/h/framestepper.h"
#include "stackwalk/h/basetypes.h"
#include "stackwalk/h/frame.h"

#include "stackwalk/src/linux-swk.h"

#include "symtabAPI/h/Symtab.h"

#include <sys/user.h>
#include <sys/ptrace.h>
#include <string.h>
#include <errno.h>

using namespace Dyninst;
using namespace Dyninst::Stackwalker;
using namespace Dyninst::SymtabAPI;

#if defined(arch_x86_64)

#define PTRACE_REG_IP rip
#define PTRACE_REG_FP rbp
#define PTRACE_REG_SP rsp

#include "symtabAPI/h/Symtab.h"

unsigned ProcDebugLinux::getAddressWidth()
{
   if (cached_addr_width)
      return cached_addr_width;

   sw_printf("[%s:%u] - Computing address size of proc %d\n",
             __FILE__, __LINE__, pid);
   char procname[64];
   snprintf(procname, 64, "/proc/%d/exe", pid);
   std::string str(procname);
   
   Symtab *obj;
   bool result = Symtab::openFile(str, obj);
   if (!result) {
      sw_printf("[%s:%u] - Error. Failed to open /proc/pid/exe\n");
      cached_addr_width = sizeof(void*); //Assume 64-bit process
      return cached_addr_width;
   }

   sw_printf("[%s:%u] - AMD64 process has address width %d\n",
             __FILE__, __LINE__, cached_addr_width);
   cached_addr_width = obj->getAddressWidth();
   obj->closeFile();
   
   return cached_addr_width;
   return sizeof(void *);
}

#else

unsigned ProcDebugLinux::getAddressWidth() 
{
   return sizeof(void*);
}

#define PTRACE_REG_IP eip
#define PTRACE_REG_FP ebp
#define PTRACE_REG_SP esp

#endif


bool ProcDebugLinux::getRegValue(reg_t reg, THR_ID t, regval_t &val)
{
   user_regs_struct gprs;
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
         val = gprs.PTRACE_REG_IP;
         break;
      case REG_SP:
         val = gprs.PTRACE_REG_SP;
         break;
      case REG_FP:
         val = gprs.PTRACE_REG_FP;
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
