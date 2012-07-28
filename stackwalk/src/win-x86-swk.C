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

#include "SymtabReader.h"
#include "stackwalk/h/walker.h"
#include "stackwalk/h/framestepper.h"
#include "stackwalk/h/steppergroup.h"
#include "stackwalk/h/swk_errors.h"
#include "common/h/ntHeaders.h"

#include <windows.h>


#include "stackwalk/h/procstate.h"
#include "stackwalk/h/frame.h"

#include "stackwalk/src/sw.h"

#include <assert.h>


using namespace Dyninst;
using namespace Dyninst::Stackwalker;

bool Walker::createDefaultSteppers()
{
  FrameStepper *stepper;
  bool result = true;

  stepper = new FrameFuncStepper(this);
  result = addStepper(stepper);
  if (!result)
     goto error;
  sw_printf("[%s:%u] - Stepper %p is FrameFuncStepper\n",
            __FILE__, __LINE__, stepper);
#if defined(USE_PARSE_API)
  stepper = new AnalysisStepper(this);
  result = addStepper(stepper);
  if (!result)
		goto error;
#endif

  return true;
 error:
  sw_printf("[%s:%u] - Error adding stepper %p\n", stepper);
  return false;
}

bool ProcSelf::getThreadIds(std::vector<THR_ID> &threads)
{
  bool result;
  THR_ID tid;

  result = getDefaultThread(tid);
  if (!result) {
    sw_printf("[%s:%u] - Could not read default thread\n",
	       __FILE__, __LINE__);
    return false;
  }
  threads.clear();
  threads.push_back(tid);
  return true;
}

bool ProcSelf::getDefaultThread(THR_ID &default_tid)
{
  default_tid = GetCurrentThread();
  return true;
}

bool ProcSelf::readMem(void *dest, Address source, size_t size)
{
   memcpy(dest, (const void *) source, size);
   return true;
}

ProcSelf::ProcSelf(std::string exe_path) :
   ProcessState(P_getpid(), exe_path)
{
}

void ProcSelf::initialize()
{
}

bool LibraryState::updateLibsArch(std::vector<std::pair<LibAddrPair, unsigned int> > &alibs)
{
	return true;
}

SymbolReaderFactory* Stackwalker::getDefaultSymbolReader()
{
	if (NULL == Walker::getSymbolReader()) {
		static SymtabAPI::SymtabReaderFactory fact;
		Walker::setSymbolReader(&fact);
	}
	return Walker::getSymbolReader();
}

void BottomOfStackStepperImpl::initialize()
{
	// For now, we stop when we get a return address of 0
	ra_stack_tops.push_back(std::pair<Address, Address>(0, 0));
}
void BottomOfStackStepperImpl::newLibraryNotification(LibAddrPair *, lib_change_t)
{

}
