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
#include "stackwalk/h/walker.h"
#include "stackwalk/h/basetypes.h"
#include "stackwalk/h/procstate.h"
#include "stackwalk/h/framestepper.h"

#include "stackwalk/src/linuxbsd-swk.h"
#include "stackwalk/src/dbgstepper-impl.h"

#include "common/h/dyn_regs.h"

#include <sys/user.h>
#include <sys/ptrace.h>
#include <assert.h>
#include <errno.h>
#include <string.h>

#define TEST_DEBUGINFO_ALONE 1

using namespace Dyninst;
using namespace Dyninst::Stackwalker;

bool Walker::createDefaultSteppers()
{
  FrameStepper *stepper;
  bool result;

#if !TEST_DEBUGINFO_ALONE
  stepper = new FrameFuncStepper(this);
  result = addStepper(stepper);
  if (!result) {
    sw_printf("[%s:%u] - Error adding stepper %p\n", FILE__, __LINE__,
	      stepper);
    return false;
  }

  // ARM: this works on ARM.
  // Need to adjust a variable that stores the length of _start
  stepper = new BottomOfStackStepper(this);
  result = addStepper(stepper);
  if (!result){
    sw_printf("[%s:%u] - Error adding stepper %p\n", FILE__, __LINE__,
	      stepper);
    return false;
  }else{
    sw_printf("[%s:%u] - Stepper %p is BottomOfStackStepper\n",
            FILE__, __LINE__, stepper);
  }

#else
  stepper = new DebugStepper(this);
  result = addStepper(stepper);
  if (!result){
    sw_printf("[%s:%u] - Error adding stepper %p\n", FILE__, __LINE__,
	      stepper);
    return false;
  }else{
    sw_printf("[%s:%u] - Stepper %p is DebugStepper\n",
            FILE__, __LINE__, stepper);
  }
#endif

  return true;
}

bool DebugStepperImpl::isFrameRegister(MachRegister reg)
{
   if (getProcessState()->getAddressWidth() == 4){
       assert(0);
      return (reg == aarch64::x29);
   }
   else
      return (reg == aarch64::x29);
}

bool DebugStepperImpl::isStackRegister(MachRegister reg)
{
   if (getProcessState()->getAddressWidth() == 4){
       assert(0);
      return (reg == aarch64::sp);
   }
   else
      return (reg == aarch64::sp);
}

gcframe_ret_t SigHandlerStepperImpl::getCallerFrame(const Frame &/*in*/,
                                                    Frame &/*out*/)
{
   /**
    * TODO: Implement me on non-x86 platforms.
    **/
   return gcf_not_me;
}
