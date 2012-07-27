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

#include "stackwalk/h/swk_errors.h"
#include "stackwalk/h/symlookup.h"
#include "stackwalk/h/walker.h"
#include "stackwalk/h/procstate.h"
#include "stackwalk/h/framestepper.h"

#include <string>
#include <string.h>
#include <assert.h>

using namespace Dyninst;
using namespace Dyninst::Stackwalker;

ProcDebug *ProcDebug::newProcDebug(PID, std::string)
{
   setLastError(err_unsupported, "Third party stackwalking not supported on " \
                "this platform");
   return NULL;
}

ProcDebug *ProcDebug::newProcDebug(std::string, 
                                   const std::vector<std::string> &)
{
   setLastError(err_unsupported, "Third party stackwalking not supported on " \
                "this platform");
   return NULL;
}

DebugEvent ProcDebug::debug_get_event(bool)
{
   assert(0);
   return DebugEvent();
}

int ProcDebug::getNotificationFD()
{
   return -1;
}

ThreadState* ThreadState::createThreadState(ProcDebug *parent,
                                            Dyninst::THR_ID,
                                            bool)
{
   ThreadState *ts = new ThreadState(parent, 0);
   return ts;
}

Dyninst::Architecture ProcDebug::getArchitecture()
{
   return Dyninst::Arch_none;
}

void int_walkerSet::addToProcSet(ProcDebug *)
{
}

void int_walkerSet::eraseFromProcSet(ProcDebug *)
{
}

void int_walkerSet::clearProcSet()
{
}

void int_walkerSet::initProcSet()
{
   procset = NULL;
}

bool int_walkerSet::walkStacksProcSet(CallTree &, bool &bad_plat)
{
   bad_plat = true;
   return false;
}

