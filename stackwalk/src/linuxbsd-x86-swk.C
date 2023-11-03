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
#include "stackwalk/h/procstate.h"
#include "stackwalk/h/framestepper.h"
#include "stackwalk/h/basetypes.h"
#include "stackwalk/h/frame.h"

#include "stackwalk/src/symtab-swk.h"
#include "stackwalk/src/linuxbsd-swk.h"
#include "stackwalk/src/dbgstepper-impl.h"
#include "stackwalk/src/x86-swk.h"

#include "registers/x86_regs.h"
#include "registers/x86_64_regs.h"

#include <sys/user.h>
#include <sys/ptrace.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>

using namespace Dyninst;
using namespace Dyninst::Stackwalker;

bool Walker::createDefaultSteppers()
{
  FrameStepper *stepper;
  WandererHelper *whelper_x86;
  LookupFuncStart *frameFuncHelper_x86;
  bool result = true;

  stepper = new DebugStepper(this);
  result = addStepper(stepper);
  if (!result)
     goto error;
  sw_printf("[%s:%d] - Stepper %p is DebugStepper\n",
            FILE__, __LINE__, (void*)stepper);

  frameFuncHelper_x86 = LookupFuncStart::getLookupFuncStart(getProcessState());
  stepper = new FrameFuncStepper(this, frameFuncHelper_x86);
  result = addStepper(stepper);
  if (!result)
     goto error;
  sw_printf("[%s:%d] - Stepper %p is FrameFuncStepper\n",
            FILE__, __LINE__, (void*)stepper);

  //Call getLookupFuncStart twice to get reference counts correct.
  frameFuncHelper_x86 = LookupFuncStart::getLookupFuncStart(getProcessState());
  whelper_x86 = new WandererHelper(getProcessState());
  stepper = new StepperWanderer(this, whelper_x86, frameFuncHelper_x86);
  result = addStepper(stepper);
  if (!result)
     goto error;
  sw_printf("[%s:%d] - Stepper %p is StepperWanderer\n",
            FILE__, __LINE__, (void*)stepper);

  stepper = new SigHandlerStepper(this);
  result = addStepper(stepper);
  if (!result)
     goto error;
  sw_printf("[%s:%d] - Stepper %p is SigHandlerStepper\n",
            FILE__, __LINE__, (void*)stepper);

  stepper = new BottomOfStackStepper(this);
  result = addStepper(stepper);
  if (!result)
     goto error;
  sw_printf("[%s:%d] - Stepper %p is BottomOfStackStepper\n",
            FILE__, __LINE__, (void*)stepper);

#ifdef USE_PARSE_API
  stepper = new AnalysisStepper(this);
  result = addStepper(stepper);
  if (!result)
     goto error;
  sw_printf("[%s:%d] - Stepper %p is AnalysisStepper\n",
            FILE__, __LINE__, (void*)stepper);
#endif

  stepper = new DyninstInstFrameStepper(this);
  result = addStepper(stepper);
  if (!result)
     goto error;

  return true;
 error:
  sw_printf("[%s:%d] - Error adding stepper %p\n",
            FILE__, __LINE__, (void*)stepper);
    return false;
}

bool DebugStepperImpl::isFrameRegister(MachRegister reg)
{
   if (getProcessState()->getAddressWidth() == 4)
      return (reg == x86::ebp);
   else
      return (reg == x86_64::rbp);
}

bool DebugStepperImpl::isStackRegister(MachRegister reg)
{
   if (getProcessState()->getAddressWidth() == 4)
      return (reg == x86::esp);
   else
      return (reg == x86_64::rsp);
}
