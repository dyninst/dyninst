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

#include "stackwalk/h/framestepper.h"
#include "stackwalk/h/walker.h"
#include "stackwalk/h/procstate.h"
#include "stackwalk/h/swk_errors.h"
#include "stackwalk/h/steppergroup.h"

#include "stackwalk/src/sw.h"
#include <assert.h>

#if defined(os_linux)
#include "stackwalk/src/linux-swk.h"
#endif 
using namespace Dyninst;
using namespace Dyninst::Stackwalker;

FrameStepper::FrameStepper(Walker *w) :
  walker(w)
{
  sw_printf("[%s:%u] - Creating FrameStepper %p with walker %p\n", 
	    __FILE__, __LINE__, this, walker);
  assert(walker);
}

FrameStepper::~FrameStepper() 
{
  walker = NULL;
  sw_printf("[%s:%u] - Deleting FrameStepper %p\n", __FILE__, __LINE__, this);
}

Walker *FrameStepper::getWalker()
{
  assert(walker);
  return walker;
}

ProcessState *FrameStepper::getProcessState() 
{
  return getWalker()->getProcessState();
}

void FrameStepper::newLibraryNotification(LibAddrPair *, lib_change_t)
{
}

void FrameStepper::registerStepperGroup(StepperGroup *group)
{
   unsigned addr_width = group->getWalker()->getProcessState()->getAddressWidth();
   if (addr_width == 4)
      group->addStepper(this, 0, 0xffffffff);
#if defined(arch_64bit)
   else if (addr_width == 8)
      group->addStepper(this, 0, 0xffffffffffffffff);
#endif
   else
      assert(0 && "Unknown architecture word size");
}

FrameFuncStepper::FrameFuncStepper(Walker *w, FrameFuncHelper *helper) :
   FrameStepper(w)
{
   impl = new FrameFuncStepperImpl(w, this, helper);
}

FrameFuncStepper::~FrameFuncStepper()
{
   if (impl)
      delete impl;
}

gcframe_ret_t FrameFuncStepper::getCallerFrame(const Frame &in, Frame &out)
{
   if (impl)
      return impl->getCallerFrame(in, out);
   sw_printf("[%s:%u] - Platform does not have basic stepper\n", 
             __FILE__, __LINE__);
   setLastError(err_unsupported, "Function frames not supported on this platform.");
   return gcf_error;
}

unsigned FrameFuncStepper::getPriority() const
{
   if (impl)
      return impl->getPriority();
   sw_printf("[%s:%u] - Platform does not have basic stepper\n", 
             __FILE__, __LINE__);
   setLastError(err_unsupported, "Function frames not supported on this platform.");
   return 0;
}

void FrameFuncStepper::registerStepperGroup(StepperGroup *group)
{
   FrameStepper::registerStepperGroup(group);
}

FrameFuncHelper::FrameFuncHelper(ProcessState *proc_) :
   proc(proc_)
{
}

FrameFuncHelper::~FrameFuncHelper()
{
}

BottomOfStackStepper::BottomOfStackStepper(Walker *w) :
   FrameStepper(w)
{
   sw_printf("[%s:%u] - Constructing BottomOfStackStepper at %p\n",
             __FILE__, __LINE__, this);
#if defined(os_linux) && (defined(arch_x86) || defined(arch_x86_64))
   impl = new BottomOfStackStepperImpl(w, this);
#else
   impl = NULL;
#endif
}

gcframe_ret_t BottomOfStackStepper::getCallerFrame(const Frame &in, Frame &out)
{
   if (impl) {
      return impl->getCallerFrame(in, out);
   }
   sw_printf("[%s:%u] - Error, top of stack not implemented on this platform\n",
             __FILE__, __LINE__);
   setLastError(err_unsupported, "Top of stack recognition not supported on this platform");
   return gcf_error;
}

unsigned BottomOfStackStepper::getPriority() const
{
   if (impl) {
      return impl->getPriority();
   }
   sw_printf("[%s:%u] - Error, top of stack not implemented on this platform\n",
             __FILE__, __LINE__);
   setLastError(err_unsupported, "Top of stack recognition not supported on this platform");
   return 0;
}

void BottomOfStackStepper::registerStepperGroup(StepperGroup *group)
{
   if (impl) {
      return impl->registerStepperGroup(group);
   }
   sw_printf("[%s:%u] - Error, top of stack not implemented on this platform\n",
             __FILE__, __LINE__);
   setLastError(err_unsupported, "Top of stack recognition not supported on this platform");
}

BottomOfStackStepper::~BottomOfStackStepper()
{
   sw_printf("[%s:%u] - Deleting BottomOfStackStepper %p (impl %p)\n", 
             __FILE__, __LINE__, this, impl);
   if (impl)
      delete impl;
}
