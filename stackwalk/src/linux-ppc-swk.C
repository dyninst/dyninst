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

#include "stackwalk/h/swk_errors.h"
#include "stackwalk/h/walker.h"
#include "stackwalk/h/basetypes.h"
#include "stackwalk/h/procstate.h"
#include "stackwalk/h/framestepper.h"
#include "stackwalk/src/linux-swk.h"
#include <sys/user.h>
#include <sys/ptrace.h>
#include <assert.h>
#include <errno.h>
#include <string.h>

using namespace Dyninst;
using namespace Dyninst::Stackwalker;

unsigned ProcDebugLinux::getAddressWidth()
{
   return sizeof(void *);
}

#define USER_OFFSET_OF(register) ((signed int) &(((struct user *) NULL)->regs.register))

static int getRegOffset(Dyninst::MachRegister reg, int addr_width)
{
  if (addr_width == 4)
  {
    switch (reg.val()) {
      case Dyninst::iReturnAddr:
      case Dyninst::ppc32::ipc:
	return USER_OFFSET_OF(nip);
      case Dyninst::iStackTop:
	return -2;
      case Dyninst::iFrameBase:
      case Dyninst::ppc32::ir1:
	return USER_OFFSET_OF(gpr[1]);
    }
  }
  return -1;
}

bool ProcDebugLinux::getRegValue(Dyninst::MachRegister reg, 
                                 Dyninst::THR_ID t, 
                                 Dyninst::MachRegisterVal &val)
{
   int result;
   signed int offset = getRegOffset(reg, getAddressWidth());
   if (offset == -2) {
     val = 0x0;
     return true;
   }
   if (offset == -1) {
     sw_printf("[%s:%u] - Request for unsupported register %s\n",
	       __FILE__, __LINE__, reg.name());
     setLastError(err_badparam, "Unknown register passed in reg field");
     return false;
   }
   
   errno = 0;
   result = ptrace(PTRACE_PEEKUSER, (pid_t) t, (void*) offset, NULL);
   if (errno)
   {
      int errnum = errno;
      sw_printf("[%s:%u] - Could not read gprs in %d: %s\n", 
                __FILE__, __LINE__, t, strerror(errnum));
      setLastError(err_procread, "Could not read registers from process");
      return false;
   }

   val = result;

   return true;
}

bool ProcDebugLinux::setRegValue(Dyninst::MachRegister reg, 
                                 Dyninst::THR_ID t, 
                                 Dyninst::MachRegisterVal val)
{
   int result;
   signed int offset = getRegOffset(reg, getAddressWidth());
   if (offset < 0) {
     sw_printf("[%s:%u] - Request for unsupported register %s\n",
	       __FILE__, __LINE__, reg.name());
     setLastError(err_badparam, "Unknown register passed in reg field");
     return false;
   }
   
   result = ptrace(PTRACE_POKEUSER, (pid_t) t, (void*) offset, (void *) val);
   if (result == -1)
   {
      int errnum = errno;
      sw_printf("[%s:%u] - Could not write to gprs in %d: %s\n", 
                __FILE__, __LINE__, t, strerror(errnum));
      setLastError(err_procread, "Could not write registers to process");
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

Dyninst::Architecture ProcDebugLinux::getArchitecture()
{
   return Dyninst::Arch_ppc32;
}

gcframe_ret_t SigHandlerStepperImpl::getCallerFrame(const Frame &/*in*/, 
                                                    Frame &/*out*/)
{
   /**
    * TODO: Implement me on non-x86 platforms.
    **/
   return gcf_not_me;
}

void ProcDebugLinux::detach_arch_cleanup()
{
}
