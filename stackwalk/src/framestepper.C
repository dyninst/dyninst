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


FrameFuncHelper::FrameFuncHelper(ProcessState *proc_) :
   proc(proc_)
{
}

FrameFuncHelper::~FrameFuncHelper()
{
}

//FrameFuncStepper defined here
#define PIMPL_IMPL_CLASS FrameFuncStepperImpl
#define PIMPL_CLASS FrameFuncStepper
#define PIMPL_NAME "FrameFuncStepper"
#define PIMPL_ARG1 FrameFuncHelper*
#include "framestepper_pimple.h"
#undef PIMPL_CLASS
#undef PIMPL_NAME
#undef PIMPL_IMPL_CLASS
#undef PIMPL_ARG1

//DyninstInstrStepper defined here
#if defined(cap_stackwalker_use_symtab)
#include "stackwalk/src/symtab-swk.h"
#define PIMPL_IMPL_CLASS DyninstInstrStepperImpl
#endif
#define PIMPL_CLASS DyninstInstrStepper
#define PIMPL_NAME "DyninstInstrStepper"
#include "framestepper_pimple.h"
#undef PIMPL_CLASS
#undef PIMPL_IMPL_CLASS
#undef PIMPL_NAME

//BottomOfStackStepper defined here
#if defined(cap_stackwalker_use_symtab) && defined(os_linux)
#include "stackwalk/src/linux-swk.h"
#define PIMPL_IMPL_CLASS BottomOfStackStepperImpl
#endif
#define PIMPL_CLASS BottomOfStackStepper
#define PIMPL_NAME "BottomOfStackStepper"
#include "framestepper_pimple.h"
#undef PIMPL_CLASS
#undef PIMPL_IMPL_CLASS
#undef PIMPL_NAME

//DebugStepper defined here
#if defined(os_linux) && (defined(arch_x86) || defined(arch_x86_64)) && defined(cap_stackwalker_use_symtab)
#include "stackwalk/src/dbgstepper-impl.h"
#define PIMPL_IMPL_CLASS DebugStepperImpl
#endif
#define PIMPL_CLASS DebugStepper
#define PIMPL_NAME "DebugStepper"
#include "framestepper_pimple.h"
#undef PIMPL_CLASS
#undef PIMPL_IMPL_CLASS
#undef PIMPL_NAME

//StepperWanderer defined here
#if defined(arch_x86) || defined(arch_x86_64)
#include "stackwalk/src/x86-swk.h"
#define PIMPL_IMPL_CLASS StepperWandererImpl
#endif
#define PIMPL_CLASS StepperWanderer
#define PIMPL_NAME "StepperWanderer"
#define PIMPL_ARG1 WandererHelper*
#define PIMPL_ARG2 FrameFuncHelper*
#include "framestepper_pimple.h"
#undef PIMPL_CLASS
#undef PIMPL_IMPL_CLASS
#undef PIMPL_NAME
#undef PIMPL_ARG1
#undef PIMPL_ARG2

//SigHandlerStepper defined here
#if defined(os_linux)
#include "stackwalk/src/linux-swk.h"
#define PIMPL_IMPL_CLASS SigHandlerStepperImpl
#endif
#define PIMPL_CLASS SigHandlerStepper
#define PIMPL_NAME "SigHandlerStepper"
#include "framestepper_pimple.h"
#undef PIMPL_CLASS
#undef PIMPL_IMPL_CLASS
#undef PIMPL_NAME
