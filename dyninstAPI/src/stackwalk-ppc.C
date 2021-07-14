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

#include "debug.h"
#include "dynProcess.h"
#include "baseTramp.h"
#include "function.h"
#include "frameChecker.h"
#include "inst-power.h"

using namespace Dyninst;

bool PCProcess::createStackwalkerSteppers()
{
  using namespace Stackwalker;

  FrameStepper *stepper = NULL;
  StackwalkInstrumentationHelper *swInstrHelper = NULL;
  DynFrameHelper *dynFrameHelper = NULL;

  // Create steppers, adding to walker

  swInstrHelper = new StackwalkInstrumentationHelper(this);
  stepper = new DyninstDynamicStepper(stackwalker_, swInstrHelper);
  if (!stackwalker_->addStepper(stepper))
  {
    startup_printf("Error adding Stackwalker stepper %p\n", (void*)stepper);
    return false;
  }
  startup_printf("Stackwalker stepper %p is a DyninstDynamicStepper\n", (void*)stepper);

  // FrameFuncHelper not used on PPC
  dynFrameHelper = new DynFrameHelper(this);
  stepper = new FrameFuncStepper(stackwalker_, dynFrameHelper);
  if (!stackwalker_->addStepper(stepper))
  {
    startup_printf("Error adding Stackwalker stepper %p\n", (void*)stepper);
    return false;
  }
  startup_printf("Stackwalker stepper %p is a FrameFuncStepper\n", (void*)stepper);

  return true;
}

bool StackwalkInstrumentationHelper::isInstrumentation(Dyninst::Address ra,
                                                       Dyninst::Address * /*orig_ra*/,
                                                       unsigned * stack_height,
                                                       bool * /* deref */,
                                                       bool * /*entryExit*/)
{
  AddressSpace::RelocInfo ri;
  baseTramp *base = NULL;

  if (!proc_->getRelocInfo(ra, ri))
  {
    return false;
  }

  base = ri.bt;

  if (base)
  {
    // set offset from instrumentation frame pointer to saved return address
    *stack_height = TRAMP_SPR_OFFSET(proc_->getAddressWidth()) + STK_LR;

    return true;
  }
  else
  {
    return false;
  }
}

using namespace Stackwalker;

FrameFuncHelper::alloc_frame_t DynFrameHelper::allocatesFrame(Address addr)
{
  FrameFuncHelper::alloc_frame_t result;
  func_instance *func = proc_->findOneFuncByAddr(addr);

  result.first = FrameFuncHelper::unknown_t; // frame type
  result.second = FrameFuncHelper::unknown_s; // frame state

  // This helper will only be used on the topmost frame

  if (func)
  {
    if (!func->savesReturnAddr())
    {
      // Leaf function
      result.second = FrameFuncHelper::unset_frame;
    }
    else
    {
      result.second = FrameFuncHelper::set_frame;
    }

    if (func->hasNoStackFrame())
    {
      result.first = FrameFuncHelper::no_frame;
    }
    else
    {
      result.first = FrameFuncHelper::standard_frame;
    }
  }

  return result;
}

bool DynWandererHelper::isPrevInstrACall(Address /*addr*/, Address &/*target*/)
{
  // NOT IMPLEMENTED
  assert(0);
  return false;
}

WandererHelper::pc_state DynWandererHelper::isPCInFunc(Address /*func_entry*/, Address /*pc*/)
{
  // NOT IMPLEMENTED
  assert(0);
  return WandererHelper::unknown_s;
}

bool DynWandererHelper::requireExactMatch()
{
  // NOT IMPLEMENTED
  assert(0);
  return false;
}

