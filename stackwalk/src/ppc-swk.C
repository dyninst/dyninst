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
#include "stackwalk/h/procstate.h"
#include "stackwalk/h/framestepper.h"
#include "stackwalk/h/basetypes.h"
#include "stackwalk/h/frame.h"

#include "stackwalk/src/sw.h"

using namespace Dyninst;
using namespace Dyninst::Stackwalker;

bool ProcSelf::getRegValue(Dyninst::MachRegister reg, THR_ID, Dyninst::MachRegisterVal &val)
{
  register Dyninst::MachRegisterVal **sp;
  Dyninst::MachRegisterVal *fp_ra;

  __asm__("or %0, %%r1, %%r1\n"
	  : "=r"(sp));

  if (reg == Dyninst::MachRegStackBase) {
     val = (Dyninst::MachRegisterVal) sp;
  }

  fp_ra = *sp;
  if (reg == Dyninst::MachRegFrameBase) 
  {
     val = fp_ra[0];
  }

  if (reg == Dyninst::MachRegPC) 
  {
     val = fp_ra[1];
  }

  sw_printf("[%s:%u] - Returning value %lx for reg %u\n", 
            __FILE__, __LINE__, val, reg);
  return true;
}

FrameFuncStepperImpl::FrameFuncStepperImpl(Walker *w, FrameStepper *parent_,
                                           FrameFuncHelper *) :
   FrameStepper(w),
   parent(parent_),
   helper(NULL)
{
}

gcframe_ret_t FrameFuncStepperImpl::getCallerFrame(const Frame &in, Frame &out)
{
  Address in_fp, out_sp;
  bool result;
  struct {
    Address out_fp;
    Address out_ra;
  } ra_fp_pair;

  if (!in.getFP())
    return gcf_stackbottom;

  in_fp = in.getFP();

  //TODO: Mutatee word size
  out_sp = in_fp;
  out.setSP(out_sp);
  
  result = getProcessState()->readMem(&ra_fp_pair, in_fp, 
                                      sizeof(ra_fp_pair));
  if (!result) {
    sw_printf("[%s:%u] - Couldn't read from %lx\n", __FILE__, __LINE__, in_fp);
    return gcf_error;
  }
  out.setFP(ra_fp_pair.out_fp);

  result = getProcessState()->readMem(&ra_fp_pair, ra_fp_pair.out_fp, 
                                      sizeof(ra_fp_pair));
  if (!result) {
    sw_printf("[%s:%u] - Couldn't read from %lx\n", __FILE__, __LINE__,
	      ra_fp_pair.out_fp);
    return gcf_error;
  }
  if (!ra_fp_pair.out_ra) {
    return gcf_stackbottom;
  }


  out.setRA(ra_fp_pair.out_ra);


  return gcf_success;
}
 
unsigned FrameFuncStepperImpl::getPriority() const
{
  return 0x11000;
}

FrameFuncStepperImpl::~FrameFuncStepperImpl()
{
}

UninitFrameStepper::UninitFrameStepper(Walker *w, FrameFuncHelper *) :
   FrameStepper(w)
{
   impl = NULL;
}

gcframe_ret_t UninitFrameStepper::getCallerFrame(const Frame &, Frame &)
{
   sw_printf("[%s:%u] - Error,  UninitFrame used on unsupported platform\n",
             __FILE__, __LINE__);
   assert(0);
   return gcf_error;
}

unsigned UninitFrameStepper::getPriority() const
{
   sw_printf("[%s:%u] - Error,  UninitFrame used on unsupported platform\n",
             __FILE__, __LINE__);
   assert(0);
   return 0;
}

void UninitFrameStepper::registerStepperGroup(StepperGroup *)
{
   sw_printf("[%s:%u] - Error,  UninitFrame used on unsupported platform\n",
             __FILE__, __LINE__);
   assert(0);
}

UninitFrameStepper::~UninitFrameStepper()
{
   impl = NULL;
}

StepperWanderer::StepperWanderer(Walker *, WandererHelper *, FrameFuncHelper *) :
   FrameStepper(NULL)
{
   sw_printf("[%s:%u] - Error,  Wanderer used on unsupported platform\n",
             __FILE__, __LINE__);
   assert(0);
}

gcframe_ret_t StepperWanderer::getCallerFrame(const Frame &, Frame &)
{
   sw_printf("[%s:%u] - Error,  Wanderer used on unsupported platform\n",
             __FILE__, __LINE__);
   assert(0);
}

unsigned StepperWanderer::getPriority() const
{
   sw_printf("[%s:%u] - Error,  Wanderer used on unsupported platform\n",
             __FILE__, __LINE__);
   assert(0);
}

StepperWanderer::~StepperWanderer()
{
}

WandererHelper::WandererHelper(ProcessState *proc_) :
   proc(proc_)
{
}

bool WandererHelper::isPrevInstrACall(Address, Address&)
{
   sw_printf("[%s:%u] - Unimplemented on this platform!\n");
   assert(0);
   return false;
}

bool WandererHelper::isPCInFunc(Address, Address)
{
   sw_printf("[%s:%u] - Unimplemented on this platform!\n");
   assert(0);
   return false;
}

WandererHelper::~WandererHelper()
{
}

