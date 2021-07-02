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

using namespace Dyninst;

bool PCProcess::createStackwalkerSteppers()
{
  using namespace Stackwalker;

  FrameStepper *stepper = NULL;
  StackwalkInstrumentationHelper *swInstrHelper = NULL;
  DynFrameHelper *dynFrameHelper = NULL;
  DynWandererHelper *dynWandererHelper = NULL;

  // Create steppers, adding to walker

  swInstrHelper = new StackwalkInstrumentationHelper(this);
  stepper = new DyninstDynamicStepper(stackwalker_, swInstrHelper);
  if (!stackwalker_->addStepper(stepper))
  {
    startup_printf("Error adding Stackwalker stepper %p\n", (void*)stepper);
    return false;
  }
  startup_printf("Stackwalker stepper %p is a DyninstDynamicStepper\n", (void*)stepper);

  stepper = new DebugStepper(stackwalker_);
  if (!stackwalker_->addStepper(stepper))
  {
    startup_printf("Error adding Stackwalker stepper %p\n", (void*)stepper);
    return false;
  }
  startup_printf("Stackwalker stepper %p is a DebugStepper\n", (void*)stepper);

  dynFrameHelper = new DynFrameHelper(this);
  stepper = new FrameFuncStepper(stackwalker_, dynFrameHelper);
  if (!stackwalker_->addStepper(stepper))
  {
    startup_printf("Error adding Stackwalker stepper %p\n", (void*)stepper);
    return false;
  }
  startup_printf("Stackwalker stepper %p is a FrameFuncStepper\n", (void*)stepper);

    stepper = new AnalysisStepper(stackwalker_);
  if (!stackwalker_->addStepper(stepper))
  {
    startup_printf("Error adding Stackwalker stepper %p\n", (void*)stepper);
    return false;
  }
  startup_printf("Stackwalker stepper %p is an AnalysisStepper\n", (void*)stepper);


  stepper = new SigHandlerStepper(stackwalker_);
  if (!stackwalker_->addStepper(stepper))
  {
    startup_printf("Error adding Stackwalker stepper %p\n", (void*)stepper);
    return false;
  }
  startup_printf("Stackwalker stepper %p is a SigHandlerStepper\n", (void*)stepper);

  stepper = new BottomOfStackStepper(stackwalker_);
  if (!stackwalker_->addStepper(stepper))
  {
    startup_printf("Error adding Stackwalker stepper %p\n", (void*)stepper);
    return false;
  }
  startup_printf("Stackwalker stepper %p is a BottomOfStackStepper\n", (void*)stepper);

  // create a separate helper to avoid double deletion
  dynFrameHelper = new DynFrameHelper(this);
  dynWandererHelper = new DynWandererHelper(this);
  stepper = new StepperWanderer(stackwalker_, dynWandererHelper, dynFrameHelper);
  if (!stackwalker_->addStepper(stepper))
  {
    startup_printf("Error adding Stackwalker stepper %p\n", (void*)stepper);
    return false;
  }
  startup_printf("Stackwalker stepper %p is a WandererStepper\n", (void*)stepper);

  return true;
}

// IN:
//   ra - Address to query for instrumentation status
// OUT:
//   *orig_ra - If in frameless instrumentation, the original address of the instruction
//              otherwise, 0x0 
//   *stack_height -
//   bool *entryExit - if this frame is entry/exit instrumentation 
bool StackwalkInstrumentationHelper::isInstrumentation(Dyninst::Address ra,
                                                       Dyninst::Address *orig_ra,
                                                       unsigned *stack_height,
                                                       bool *aligned,
                                                       bool *entryExit)
{
  bool result;
  AddressSpace::RelocInfo ri;
  baseTramp *base = NULL;
  instPoint *point = NULL;
  Address orig = 0;

  *orig_ra = 0x0;
  *stack_height = 0;
  *entryExit = 0;
  result = false;

  // don't handle signal handlers
  if (proc_->isInSignalHandler(ra))
  {
    return false;
  }

  if (!proc_->getRelocInfo(ra, ri))
  {
    return false;
  }

  base = ri.bt;
  orig = ri.orig;

  // set entryExit if we're in entry or exit instrumentation
  // base tramp type is entry or exit

  if (base)
  {
      result = true;

      point = base->point();

      *stack_height = base->stackHeight;
      *aligned = base->alignedStack;

      if (!base->madeFrame())
      {
        *orig_ra = orig;
      }

      if (point)
      {
        if ((point->type() == instPoint::FuncEntry) ||
            (point->type() == instPoint::FuncExit))
        {
          *entryExit = 1;
        }
      }
  }

  return result;
}

using namespace Stackwalker;

FrameFuncHelper::alloc_frame_t DynFrameHelper::allocatesFrame(Address addr)
{
  FrameFuncHelper::alloc_frame_t result;
  func_instance *func = proc_->findOneFuncByAddr(addr);
  std::set<block_instance*> blocks;
  block_instance *aBlock = NULL;

  result.first = FrameFuncHelper::unknown_t; // frame type
  result.second = FrameFuncHelper::unknown_s; // frame state

  if (func)
  {
    // Determine frame type
    if (!func->hasNoStackFrame())
    {
      result.first = FrameFuncHelper::standard_frame;
    }
    else if (func->savesFramePointer())
    {
      result.first = FrameFuncHelper::savefp_only_frame;
    }
    else
    {
      result.first = FrameFuncHelper::no_frame;
    }

    // Determine frame state
    if (FrameFuncHelper::standard_frame == result.first)
    {
      result.second = FrameFuncHelper::set_frame;

      proc_->findBlocksByAddr(addr, blocks);

      if (!blocks.empty())
      {
        aBlock = *(blocks.begin());
        frameChecker fc((const unsigned char*)(proc_->getPtrToInstruction(addr)),
                        aBlock->size() - (addr - aBlock->start()),
                        proc_->getArch());
        if (fc.isReturn() || fc.isStackPreamble())
        {
          result.second = FrameFuncHelper::unset_frame;
        }
        if (fc.isStackFrameSetup())
        {
          result.second = FrameFuncHelper::halfset_frame;
        }
      }
    }
  }
  
  return result;
}

bool DynWandererHelper::isPrevInstrACall(Address addr, Address &target)
{
    // is the instruction just before the instruction at addr a call?
    // if so, set target to be the function it calls
     
    std::set<func_instance *> funcs;
    func_instance *calleeFunc = NULL;
    proc_->findFuncsByAddr(addr, funcs, true);
    for (std::set<func_instance *>::iterator iter = funcs.begin();
         iter != funcs.end(); ++iter) {
       for (PatchAPI::PatchFunction::Blockset::const_iterator c_iter = (*iter)->callBlocks().begin();
            c_iter != (*iter)->callBlocks().end(); ++c_iter) {
          if ((*c_iter)->end() == addr) {
             calleeFunc = SCAST_BI((*c_iter))->callee();
             if (calleeFunc) {
                target = calleeFunc->addr();
             }
             else {
                target = 0;
             }
             return true;
            }
       }
    }
    return false;
}

WandererHelper::pc_state DynWandererHelper::isPCInFunc(Address func_entry, Address pc)
{
  func_instance *callee_func = proc_->findOneFuncByAddr(func_entry),
                *cur_func    = proc_->findOneFuncByAddr(pc);

  stackwalk_printf("[%s:%d] - DynWandererHelper called for func entry: %lx, pc: %lx - "
                     "found callee func: %p, cur func: %p\n", __FILE__, __LINE__, 
                     func_entry, pc, (void*)callee_func, (void*)cur_func);

  if (!callee_func || !cur_func)
  {
    return WandererHelper::unknown_s;
  }
  else if (callee_func == cur_func)
  {
    return WandererHelper::in_func;
  }
  else
  {
    return WandererHelper::outside_func;
  }
}

bool DynWandererHelper::requireExactMatch()
{
  // Allow callsites that aren't verified to call to the current function
  return false;
}
