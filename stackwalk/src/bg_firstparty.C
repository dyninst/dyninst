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

#include "stackwalk/h/procstate.h"
#include "stackwalk/h/walker.h"
#include "stackwalk/h/framestepper.h"
#include "stackwalk/h/swk_errors.h"

using namespace Dyninst;
using namespace Stackwalker;

#include "symlite/h/SymLite-elf.h"
#include "elf/h/Elf_X.h"

#include "stackwalk/src/sw.h"

#include <assert.h>

ProcSelf::ProcSelf(std::string exec_path) :
   ProcessState(getpid(), exec_path)
{
}

void ProcSelf::initialize()
{
   setDefaultLibraryTracker();
   assert(library_tracker);
}

bool Walker::createDefaultSteppers() {
   FrameStepper *stepper = new FrameFuncStepper(this);
   bool result = addStepper(stepper);
   if (!result) {
      sw_printf("[%s:%u] - Error adding stepper %p\n", FILE__, __LINE__, stepper);
   }
   return result;
}

bool ProcSelf::readMem(void *dest, Address source, size_t size)
{
   memcpy(dest, (const void *) source, size);
   return true;
}

bool ProcSelf::getThreadIds(std::vector<THR_ID> &threads)
{
  bool result;
  THR_ID tid;

  result = getDefaultThread(tid);
  if (!result) {
    sw_printf("[%s:%u] - Could not read default thread\n", FILE__, __LINE__);
    return false;
  }
  threads.clear();
  threads.push_back(tid);
  return true;
}

bool ProcSelf::getDefaultThread(THR_ID &default_tid)
{
   default_tid = 0;
   return true;
}

void BottomOfStackStepperImpl::initialize()
{
}

void BottomOfStackStepperImpl::newLibraryNotification(LibAddrPair *, lib_change_t)
{
}

SymbolReaderFactory *Dyninst::Stackwalker::getDefaultSymbolReader()
{
   static SymElfFactory symelffact;
   if (!Walker::symrfact)
      Walker::symrfact = (SymbolReaderFactory *) &symelffact;
   return Walker::symrfact;
}

bool TrackLibState::updateLibsArch() {
   return true;
}

