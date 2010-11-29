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

#include "pcProcess.h"
#include "baseTramp.h"
#include "multiTramp.h"
#include "miniTramp.h"
#include "function.h"

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

  mapped_object *mobj = proc_->findObject(ra);

  if (mobj)
  {
    result = mobj->analyze();
    assert(result);
  }

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

