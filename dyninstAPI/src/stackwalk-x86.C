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

#include "debug.h"
#include "pcProcess.h"
#include "baseTramp.h"
#include "multiTramp.h"
#include "miniTramp.h"
#include "function.h"
#include "frameChecker.h"

bool StackwalkSymLookup::lookupAtAddr(Dyninst::Address addr, std::string &out_name, void* &out_value)
{
  mapped_object *mobj = proc_->findObject(addr);
  codeRange *range = NULL;
  int_function *func = NULL;
  bool result = false;

  // set out_name to the name of the function at this addr
  // set out_value to be the codeRange* for this addr

  if (mobj)
  {
    result = mobj->analyze();
    assert(result);
  }

  range = proc_->findOrigByAddr(addr);

  out_value = (void*) range;

  func = range->is_function();

  if (func)
  {
    out_name = func->prettyName();
  }
  else
  {
    out_name = string("[UNKNOWN]");
  }
  
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
                                                       bool *entryExit)
{
  bool result;
  codeRange *range = NULL;
  multiTramp *multi = NULL;
  miniTrampInstance *mini = NULL;
  baseTrampInstance *base = NULL;
  instPoint *instP = NULL;

  int_function *func = NULL;





  *orig_ra = 0x0;
  *stack_height = 0;
  *entryExit = 0;
  result = false;

  // don't handle signal handlers
  if (proc_->isInSignalHandler(ra))
  {
    return false;
  }

  range = proc_->findOrigByAddr(ra);
  multi = range->is_multitramp();
  mini = range->is_minitramp();

  func = range->is_function();

  if (multi)
  {
    base = multi->getBaseTrampInstanceByAddr(ra);
  }

  if (base)
  {
    if (base->isInInstru(ra))
    {
      *stack_height = base->trampStackHeight();
      if (!base->baseT->createFrame())
      {
        // Frameless instrumentation
        *orig_ra = base->baseT->origInstAddr();
      }

      result = true;
    }

    result = true;
  }
  else if (multi)
  {

  }
  else if (mini)
  {
    result = true;
  }

  // Determine if this is entry/exit instrumentation
  if ((base == NULL) && mini)
  {
    base = mini->baseTI;
  }

  if (base)
  {
    instP = base->baseT->instP();

    if (instP && (instP->getPointType() == functionEntry ||
                  instP->getPointType() == functionExit))
    {
      *entryExit = true;
    }
  }

  // NOTE: Do not handle other, non-instrumentation functions with a stack frame

  return result;
}

using namespace Stackwalker;

FrameFuncHelper::alloc_frame_t DynFrameHelper::allocatesFrame(Address addr)
{
  FrameFuncHelper::alloc_frame_t result;
  codeRange *range = proc_->findOrigByAddr(addr);
  int_function *func = range->is_function();

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

      if (range && range->is_basicBlockInstance())
      {
        frameChecker fc((const unsigned char*)(proc_->getPtrToInstruction(addr)),
                        range->get_size() - (addr - range->get_address()),
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
    int_function *callee = NULL;
    target = 0;

    if (BPatch_defensiveMode != proc_->getHybridMode()) {
        codeRange *range = proc_->findOrigByAddr(addr);
        pdvector<instPoint *> callsites;

        if (range == NULL)
            return false;
   
        int_function *func_ptr = range->is_function();

        if (func_ptr != NULL)
            callsites = func_ptr->funcCalls();
        else
            return false;
      
        for (unsigned i = 0; i < callsites.size(); i++)
            {
                instPoint *site = callsites[i];

                // Argh. We need to check for each call site in each
                // instantiation of the function.
                if (site->match(addr - site->insn()->size())) {
                    callee = site->findCallee();

                    if (callee)
                    {
                      target = callee->getAddress();
                    }

                    return true;
                }
            }   
   
        return false; 
    }
    else { // in defensive mode 

        /*In defensive mode calls may be deemed to be non-returning at
          parse time and there may be two additional complications:
          1st, the addr may be in a multiTramp. 2nd, the block may
          have been split, meaning that block->containsCall()
          is not a reliable test.

          General approach: 
          find block at addr-1, then translate back to original
          address and see if the block's last instruction is a call
        */

        codeRange *callRange = proc_->findOrigByAddr(addr-1);
        bblInstance *callBBI = callRange->is_basicBlockInstance();
        Address callAddr = 0; //address of call in original block instance

        if (callRange->is_mapped_object()) {
            callRange->is_mapped_object()->analyze();
            callRange = proc_->findOrigByAddr(addr-1);
            callBBI = callRange->is_basicBlockInstance();
        }
        if (!callRange) {
            return false;
        }

        // if the range is a multi, set the callBBI and its post-call address
        multiTramp *callMulti = callRange->is_multitramp();
        if (callMulti) {
            using namespace InstructionAPI;
            addr = callMulti->instToUninstAddr(addr-1);//sets to addr of prev insn
            callBBI = proc_->findOrigByAddr(addr)->is_basicBlockInstance();
            if (!callBBI) {
                return false; //stackwalking has gone bad
            }
            // get back to orig instruction addr
            mapped_object *obj = callBBI->func()->obj();
            InstructionDecoder dec(obj->getPtrToInstruction(addr),
                                   InstructionDecoder::maxInstructionLength,
                                   proc_->getArch());
            Instruction::Ptr insn = dec.decode();
            assert(insn);
            addr += insn->size(); 
        }

        // if the range is a bbi, and it contains a call, and the call instruction
        // matches the address, set callee and return true
        if (callBBI && addr == callBBI->endAddr()) {

            // if the block has no call, make sure it's not due to block splitting
            if ( !callBBI->block()->containsCall() ) {
                Address origAddr = callBBI->equivAddr(0,callBBI->lastInsnAddr());
                callBBI = proc_->findOrigByAddr(origAddr)->is_basicBlockInstance();
                if (callBBI && callBBI->block()->containsCall()) {
                    callAddr = origAddr;//addr of orig call instruction
                }
            }
            // if the block does contain a call, set callAddr if we haven't yet
            if ( !callAddr && callBBI->block()->containsCall() ) {
                callAddr = callBBI->equivAddr(0, callBBI->lastInsnAddr());
            }
        }

        if (callAddr) {
            instPoint *callPoint = callBBI->func()->findInstPByAddr( callAddr );
            if (!callPoint) { // this is necessary, at least the first time
                callBBI->func()->funcCalls();
                callPoint = callBBI->func()->findInstPByAddr( callAddr );
            }
            if (!callPoint) {
                assert(callBBI->func()->obj()->parse_img()->codeObject()->
                       defensiveMode());
                mal_printf("Warning, call at %lx, found while "
                           "stackwalking, has no callpoint attached, does "
                           "the target tamper with the call stack? %s[%d]\n", 
                           callAddr, FILE__,__LINE__);
                callee = callBBI->func();
            } else {
                callee = callPoint->findCallee();
            }
            if (NULL == callee && 
                string::npos == callBBI->func()->get_name().find("DYNINSTbreakPoint"))
                {
                    mal_printf("WARNING: didn't find a callee for callPoint at "
                               "%lx when stackwalking %s[%d]\n", callAddr, 
                               FILE__,__LINE__);
                }
            if (callee)
            {
              target = callee->getAddress();
            }
            return true;
        }
        return false;
    }

  return false;
}

WandererHelper::pc_state DynWandererHelper::isPCInFunc(Address func_entry, Address pc)
{
  int_function *callee_func = proc_->findFuncByAddr(func_entry),
               *cur_func    = proc_->findFuncByAddr(pc);

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
