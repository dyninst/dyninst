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
#include "inst-aarch64.h"

//#warning "This file is not implemented yet!"
using namespace Dyninst;

bool PCProcess::createStackwalkerSteppers()
{
	assert(0);
  return true;
}

bool StackwalkInstrumentationHelper::isInstrumentation(Dyninst::Address ra,
                                                       Dyninst::Address * /*orig_ra*/,
                                                       unsigned * stack_height,
                                                       bool * /* deref */,
                                                       bool * /*entryExit*/)
{
	assert(0);
	return false;
}

using namespace Stackwalker;

FrameFuncHelper::alloc_frame_t DynFrameHelper::allocatesFrame(Address addr)
{
	assert(0);
  FrameFuncHelper::alloc_frame_t result;
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

