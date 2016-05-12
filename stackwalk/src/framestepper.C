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

#include "stackwalk/h/framestepper.h"
#include "stackwalk/h/walker.h"
#include "stackwalk/h/procstate.h"
#include "stackwalk/h/swk_errors.h"
#include "stackwalk/h/steppergroup.h"
#include "stackwalk/h/frame.h"

#include "stackwalk/src/sw.h"

#include <assert.h>

using namespace Dyninst;
using namespace Dyninst::Stackwalker;


FrameStepper::FrameStepper(Walker *w) :
  walker(w)
{
  sw_printf("[%s:%u] - Creating FrameStepper %p with walker %p\n",
	    FILE__, __LINE__, this, walker);
  assert(walker);
}

FrameStepper::~FrameStepper()
{
  walker = NULL;
  sw_printf("[%s:%u] - Deleting FrameStepper %p\n", FILE__, __LINE__, this);
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

std::map<SymReader*, bool> DyninstInstrStepperImpl::isRewritten;

DyninstInstrStepperImpl::DyninstInstrStepperImpl(Walker *w, DyninstInstrStepper *p) :
  FrameStepper(w),
  parent(p)
{
}

gcframe_ret_t DyninstInstrStepperImpl::getCallerFrame(const Frame &in, Frame &out)
{
   unsigned stack_height = 0;
   LibAddrPair lib;
   bool result;

   result = getProcessState()->getLibraryTracker()->getLibraryAtAddr(in.getRA(), lib);
   if (!result) {
      sw_printf("[%s:%u] - Stackwalking through an invalid PC at %lx\n",
                 FILE__, __LINE__, in.getRA());
      return gcf_error;
   }

   SymReader *reader = LibraryWrapper::getLibrary(lib.first);
   if (!reader) {
      sw_printf("[%s:%u] - Could not open file %s\n",
                 FILE__, __LINE__, lib.first.c_str());
      setLastError(err_nofile, "Could not open file for Debugging stackwalker\n");
      return gcf_error;
   }

   std::map<SymReader *, bool>::iterator i = isRewritten.find(reader);
   bool is_rewritten_binary;
   if (i == isRewritten.end()) {
      Section_t sec = reader->getSectionByName(".dyninstInst");
      is_rewritten_binary = reader->isValidSection(sec);
      isRewritten[reader] = is_rewritten_binary;
   }
   else {
     is_rewritten_binary = (*i).second;
   }
   if (!is_rewritten_binary) {
     sw_printf("[%s:u] - Decided that current binary is not rewritten, "
	       "DyninstInstrStepper returning gcf_not_me at %lx\n",
	       FILE__, __LINE__, in.getRA());
     return gcf_not_me;
   }

   std::string name;
   in.getName(name);
   const char *s = name.c_str();
   if (strstr(s, "dyninst") != s)
   {
     sw_printf("[%s:%u] - Current function %s not dyninst generated\n",
		FILE__, __LINE__, s);
     return gcf_not_me;
   }

   if (strstr(s, "dyninstBT") != s)
   {
     sw_printf("[%s:%u] - Dyninst, but don't know how to read non-tramp %s\n",
		FILE__, __LINE__, s);
     return gcf_not_me;
   }

   sw_printf("[%s:%u] - Current function %s is baseTramp\n",
	      FILE__, __LINE__, s);
   Address base;
   unsigned size;
   int num_read = sscanf(s, "dyninstBT_%lx_%u_%x", &base, &size, &stack_height);
   bool has_stack_frame = (num_read == 3);
   if (!has_stack_frame) {
      sw_printf("[%s:%u] - Don't know how to walk through instrumentation without a stack frame\n",
                FILE__, __LINE__);
     return gcf_not_me;
   }

   return getCallerFrameArch(in, out, base, lib.second, size, stack_height);
}

unsigned DyninstInstrStepperImpl::getPriority() const
{
  return dyninstr_priority;
}

void DyninstInstrStepperImpl::registerStepperGroup(StepperGroup *group)
{
  unsigned addr_width = group->getWalker()->getProcessState()->getAddressWidth();
  if (addr_width == 4)
    group->addStepper(parent, 0, 0xffffffff);
#if defined(arch_64bit)
  else if (addr_width == 8)
    group->addStepper(parent, 0, 0xffffffffffffffff);
#endif
  else
    assert(0 && "Unknown architecture word size");
}

DyninstInstrStepperImpl::~DyninstInstrStepperImpl()
{
}
BottomOfStackStepperImpl::BottomOfStackStepperImpl(Walker *w, BottomOfStackStepper *p) :
   FrameStepper(w),
   parent(p),
   libc_init(false),
   aout_init(false),
   libthread_init(false)
{
   sw_printf("[%s:%u] - Constructing BottomOfStackStepperImpl at %p\n",
             FILE__, __LINE__, this);
   initialize();
}

gcframe_ret_t BottomOfStackStepperImpl::getCallerFrame(const Frame &in, Frame & /*out*/)
{
   /**
    * This stepper never actually returns an 'out' frame.  It simply
    * tries to tell if we've reached the top of a stack and returns
    * either gcf_stackbottom or gcf_not_me.
    **/
   std::vector<std::pair<Address, Address> >::iterator i;
   for (i = ra_stack_tops.begin(); i != ra_stack_tops.end(); i++)
   {
      if (in.getRA() >= (*i).first && in.getRA() <= (*i).second)
         return gcf_stackbottom;
   }

   for (i = sp_stack_tops.begin(); i != sp_stack_tops.end(); i++)
   {
      if (in.getSP() >= (*i).first && in.getSP() < (*i).second)
         return gcf_stackbottom;
   }
   // So we would really like to do error checking here
   // but we can't. Specifically, the (generic) bottom of stack
   // stepper has no idea that instrumentation may have been overlaid
   // on the regular address space, because it knows nothing of instrumentation
   // at all.
   // Which, in turn, means that we delegate down to every other stepper
   // the responsibility to check for whether an RA is out of bounds
   // and error out accordingly--any one of them might be first
   // in a custom stepper group.
   // This is sub-optimal.

   return gcf_not_me;
}

unsigned BottomOfStackStepperImpl::getPriority() const
{
   //Highest priority, test for top of stack first.
   return stackbottom_priority;
}

void BottomOfStackStepperImpl::registerStepperGroup(StepperGroup *group)
{
   unsigned addr_width = group->getWalker()->getProcessState()->getAddressWidth();
   if (addr_width == 4)
      group->addStepper(parent, 0, 0xffffffff);
#if defined(arch_64bit)
   else if (addr_width == 8)
      group->addStepper(parent, 0, 0xffffffffffffffff);
#endif
   else
      assert(0 && "Unknown architecture word size");
}

BottomOfStackStepperImpl::~BottomOfStackStepperImpl()
{
}

DyninstDynamicHelper::~DyninstDynamicHelper()
{
}

DyninstDynamicStepperImpl::DyninstDynamicStepperImpl(Walker *w, DyninstDynamicStepper *p,
                                                     DyninstDynamicHelper *h) :
  FrameStepper(w),
  parent(p),
  helper(h),
  prevEntryExit(false)
{
}

gcframe_ret_t DyninstDynamicStepperImpl::getCallerFrame(const Frame &in, Frame &out)
{
   unsigned stack_height = 0;
   Address orig_ra = 0x0;
   bool entryExit = false;
   bool aligned = false;

   // Handle dynamic instrumentation
   if (helper)
   {
      bool instResult = helper->isInstrumentation(in.getRA(), &orig_ra, &stack_height, &aligned, &entryExit);
      bool pEntryExit = prevEntryExit;

      // remember that this frame was entry/exit instrumentation
      prevEntryExit = entryExit;

      if (pEntryExit || instResult) {
         out.setNonCall();
         return getCallerFrameArch(in, out, 0, 0, 0, stack_height, aligned, orig_ra, pEntryExit);
      }
   }

   return gcf_not_me;
}

unsigned DyninstDynamicStepperImpl::getPriority() const
{
  return dyninstr_priority;
}

void DyninstDynamicStepperImpl::registerStepperGroup(StepperGroup *group)
{
  unsigned addr_width = group->getWalker()->getProcessState()->getAddressWidth();
  if (addr_width == 4)
    group->addStepper(parent, 0, 0xffffffff);
#if defined(arch_64bit)
  else if (addr_width == 8)
    group->addStepper(parent, 0, 0xffffffffffffffff);
#endif
  else
    assert(0 && "Unknown architecture word size");
}

DyninstDynamicStepperImpl::~DyninstDynamicStepperImpl()
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
#define PIMPL_IMPL_CLASS DyninstInstrStepperImpl
#define PIMPL_CLASS DyninstInstrStepper
#define PIMPL_NAME "DyninstInstrStepper"
#include "framestepper_pimple.h"
#undef PIMPL_CLASS
#undef PIMPL_IMPL_CLASS
#undef PIMPL_NAME

//BottomOfStackStepper defined here
#if defined(os_linux) || defined(os_bg) || defined(os_freebsd) || defined(os_windows)
#define OVERLOAD_NEWLIBRARY
#define PIMPL_IMPL_CLASS BottomOfStackStepperImpl
#endif
#define PIMPL_CLASS BottomOfStackStepper
#define PIMPL_NAME "BottomOfStackStepper"
#include "framestepper_pimple.h"
#undef PIMPL_CLASS
#undef PIMPL_IMPL_CLASS
#undef PIMPL_NAME
#undef OVERLOAD_NEWLIBRARY

//DebugStepper defined here
//#if (defined(os_linux) || defined(os_freebsd)) && (defined(arch_x86) || defined(arch_x86_64))
#if (defined(os_linux) || defined(os_freebsd)) && (defined(arch_x86) || defined(arch_x86_64) || defined(arch_aarch64) )
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
#define OVERLOAD_NEWLIBRARY
#if defined(os_linux) || defined(os_freebsd) || defined(os_bgq)
#include "stackwalk/src/linuxbsd-swk.h"
#define PIMPL_IMPL_CLASS SigHandlerStepperImpl
#endif
#define PIMPL_CLASS SigHandlerStepper
#define PIMPL_NAME "SigHandlerStepper"
#include "framestepper_pimple.h"
#undef PIMPL_CLASS
#undef PIMPL_IMPL_CLASS
#undef PIMPL_NAME
#undef OVERLOAD_NEWLIBRARY

//AnalysisStepper defined here
#ifdef USE_PARSE_API
#include "stackwalk/src/analysis_stepper.h"
#define PIMPL_IMPL_CLASS AnalysisStepperImpl
#endif
#define PIMPL_CLASS AnalysisStepper
#define PIMPL_NAME "AnalysisStepper"
#include "framestepper_pimple.h"
#undef PIMPL_CLASS
#undef PIMPL_IMPL_CLASS
#undef PIMPL_NAME


//DyninstDynamicStepper defined here
#define PIMPL_IMPL_CLASS DyninstDynamicStepperImpl
#define PIMPL_CLASS DyninstDynamicStepper
#define PIMPL_NAME "DyninstDynamicStepper"
#define PIMPL_ARG1 DyninstDynamicHelper*
#include "framestepper_pimple.h"
#undef PIMPL_CLASS
#undef PIMPL_IMPL_CLASS
#undef PIMPL_NAME
#undef PIMPL_ARG1

