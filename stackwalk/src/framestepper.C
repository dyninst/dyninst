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
  sw_printf("[%s:%d] - Creating FrameStepper %p with walker %p\n",
	    FILE__, __LINE__, (void*)this, (void*)walker);
  assert(walker);
}

FrameStepper::~FrameStepper()
{
  walker = NULL;
  sw_printf("[%s:%d] - Deleting FrameStepper %p\n", FILE__, __LINE__, (void*)this);
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
   else if (addr_width == 8)
      group->addStepper(this, 0, 0xffffffffffffffff);
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
      sw_printf("[%s:%d] - Stackwalking through an invalid PC at %lx\n",
                 FILE__, __LINE__, in.getRA());
      return gcf_error;
   }

   SymReader *reader = LibraryWrapper::getLibrary(lib.first);
   if (!reader) {
      sw_printf("[%s:%d] - Could not open file %s\n",
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
     sw_printf("[%s:%d] - Decided that current binary is not rewritten, "
	       "DyninstInstrStepper returning gcf_not_me at %lx\n",
	       FILE__, __LINE__, in.getRA());
     return gcf_not_me;
   }

   std::string name;
   in.getName(name);
   const char *s = name.c_str();
   if (strstr(s, "dyninst") != s)
   {
     sw_printf("[%s:%d] - Current function %s not dyninst generated\n",
		FILE__, __LINE__, s);
     return gcf_not_me;
   }

   if (strstr(s, "dyninstBT") != s)
   {
     sw_printf("[%s:%d] - Dyninst, but don't know how to read non-tramp %s\n",
		FILE__, __LINE__, s);
     return gcf_not_me;
   }

   sw_printf("[%s:%d] - Current function %s is baseTramp\n",
	      FILE__, __LINE__, s);
   Address base;
   unsigned size;
   int num_read = sscanf(s, "dyninstBT_%lx_%u_%x", &base, &size, &stack_height);
   bool has_stack_frame = (num_read == 3);
   if (!has_stack_frame) {
      sw_printf("[%s:%d] - Don't know how to walk through instrumentation without a stack frame\n",
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
  else if (addr_width == 8)
    group->addStepper(parent, 0, 0xffffffffffffffff);
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
   sw_printf("[%s:%d] - Constructing BottomOfStackStepperImpl at %p\n",
             FILE__, __LINE__, (void*)this);
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
   else if (addr_width == 8)
      group->addStepper(parent, 0, 0xffffffffffffffff);
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
  else if (addr_width == 8)
    group->addStepper(parent, 0, 0xffffffffffffffff);
  else
    assert(0 && "Unknown architecture word size");
}

DyninstDynamicStepperImpl::~DyninstDynamicStepperImpl()
{
}

DyninstInstFrameStepperImpl::DyninstInstFrameStepperImpl(Walker *w, DyninstInstFrameStepper *p) :
  FrameStepper(w),
  parent(p)
{
}

gcframe_ret_t DyninstInstFrameStepperImpl::getCallerFrame(const Frame &in, Frame &out)
{
    /**    
        This wralker is designed to "jump over" instrimentation frames when stack walker is 
        used in first party mode. The return from this walker will be the next valid frame 
        in the program. 

        Frame layout that this walker is looking for (in x86-64 ASM):

        ...
        mov 0x48(%rsp),%rax
        push %rax           // Previous Frame SP pulled from the location stored earlier.
        mov beefdead,%rax   
        push %rax           // Flag variable (can be changed in emit-x86.C)
        mov 36b6e340,%rax   
        push %rax           // Return Address used by Stackwalker Third Party for unwidning
        push %rbp           // Previous Frame FP, end of dyninst inst frame

        How this function generates a stack frame:

        1. Compares the current FP to the SP to determine if the FP holds an address that is potentially
           on the stack. The reason for this check is to prevent reading uninitialized memory. Right now
           it checks if the FP is within 500 stack positions of the SP.

        2. Check for the magic word by reading the memory loaction current FP + addr_width * 2.

        3. If the magic word is detected, rthe frame is generated by the following:

            Frame SP: reading the value at FP + addr_width * 3 then adding addr_width to this value.
            Frame FP: reading the value at FP (should be the stored previous FP)
            Frame RA: reading the value at the Previous Frames SP - addr_width.
    */


    // Magic word inserted into the stack by dyninst instrimentation frame creation.
    // If the magic word is present at in.getFP()[1], accept this frame
    uint64_t magicWord = 0xBEEFDEAD;

    Address ret;
    Address framePtr = in.getFP();
    Address stackPtr = in.getSP();

    const unsigned addr_width = getProcessState()->getAddressWidth();

    Address magicWordPos =  addr_width * 2;
    Address spPosition = addr_width * 3;

    uint64_t diff = uint64_t(framePtr) - uint64_t(stackPtr);

    // Get the address width for the architecture

    // Check if the FP is close to the SP. If it is not, the FP is likely invalid and this
    // isn't an instrimentation frame. This check is required to prevent a deref of an invalid FP. 
    if ( diff >= (addr_width * 500) ) {
      sw_printf("[%s:%d] - I am Rejecting frame because (FP - stackPtr) > 500 stack positions - FP: %lx , SP: %lx, DIFF: %lu, Check: %u\n",
          FILE__, __LINE__, framePtr, stackPtr, diff,  (addr_width * 500));          
      return gcf_not_me;
    }

    sw_printf("[%s:%d] - reading from memory at location %lx with framePtr %lx\n",
        FILE__, __LINE__, framePtr + addr_width, framePtr);

    // Read the location in the stack where the Special Value should be.
    // This value was inserted into the stack at inst frame creation. 
    if (getWord(ret, framePtr + magicWordPos)) {    
        // check the return 
        sw_printf("[%s:%d] - %lx read from memory at location %lx\n",
            FILE__, __LINE__, ret, magicWordPos);
        // Check if that value is equal to the magic word, if its not we are not in 
        // an instrimentation frame
        if ((uint64_t)ret == magicWord) {
            // Accept the frame
            Address sp, ra, fp;
            // Read SP and FP for the next frame.
            if (!getWord(fp, framePtr) || !getWord(sp, framePtr + spPosition)) {
                sw_printf("[%s:%d] - unable to read SP or FP from frame\n",
                    FILE__, __LINE__);           
                return gcf_not_me;
            }
            // Get the return address by reading the SP.
            if (!getWord(ra, sp)) {
                sw_printf("[%s:%d] - unable to read return address from %lx\n",
                    FILE__, __LINE__, sp);           
                return gcf_not_me;
            }
            // Offset sp by addr_width to account for where the frame truely ends
            // without this, debugsteppers will not function on the out frame.
            sp = sp + addr_width;
            out.setRA(ra);
            out.setFP(fp);
            out.setSP(sp);
            sw_printf("[%s:%d] - Accepted frame, output new frame with SP: %lx\n",
                    FILE__, __LINE__, sp);           
            return gcf_success;
        }
    } 
    return gcf_not_me;
}

bool DyninstInstFrameStepperImpl::getWord(Address &word_out, Address start)
{
   const unsigned addr_width = getProcessState()->getAddressWidth();
   if (start < 1024) {
      sw_printf("[%s:%d] - %lx too low to be valid memory\n",
                FILE__, __LINE__, start);
      return false;
   }
   word_out = 0x0;
   bool result = getProcessState()->readMem(&word_out, start, addr_width);
   if (!result) {
      sw_printf("[%s:%d] - DyninstInstFrameStepperImpl couldn't read from stack at 0x%lx\n",
                FILE__, __LINE__, start);
      return false;
   }

   return true;
}

unsigned DyninstInstFrameStepperImpl::getPriority() const
{
  return dyninstr_priority;
}

void DyninstInstFrameStepperImpl::registerStepperGroup(StepperGroup *group)
{
  unsigned addr_width = group->getWalker()->getProcessState()->getAddressWidth();
  if (addr_width == 4)
    group->addStepper(parent, 0, 0xffffffff);
  else if (addr_width == 8)
    group->addStepper(parent, 0, 0xffffffffffffffff);
  else
    assert(0 && "Unknown architecture word size");
}

DyninstInstFrameStepperImpl::~DyninstInstFrameStepperImpl()
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
#if defined(os_linux) || defined(os_freebsd) || defined(os_windows)
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
#if defined(os_linux) || defined(os_freebsd)
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

//DyninstInstFrameStepper defined here
#define PIMPL_IMPL_CLASS DyninstInstFrameStepperImpl
#define PIMPL_CLASS DyninstInstFrameStepper
#define PIMPL_NAME "DyninstInstFrameStepper"
#include "framestepper_pimple.h"
#undef PIMPL_CLASS
#undef PIMPL_IMPL_CLASS
#undef PIMPL_NAME
