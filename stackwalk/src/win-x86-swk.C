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
#include "common/src/ntHeaders.h"

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
            FILE__, __LINE__, stepper);
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
	       FILE__, __LINE__);
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
    setDefaultLibraryTracker();
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

static const char* START_FUNC_NAME = "_start";
static const char* CLONE_FUNC_NAME = "clone";
//static const char* START_THREAD_FUNC_NAME = 

void BottomOfStackStepperImpl::initialize()
{
	// For now, we stop when we get a return address of 0
	ra_stack_tops.push_back(std::pair<Address, Address>(0, 0));
	// By examination, this is an _start equivalent
	ra_stack_tops.push_back(std::pair<Address, Address>(0x77959ECB, 0x77959F01));
	ra_stack_tops.push_back(std::pair<Address, Address>(0x77959EAA, 0x77959EC6));
/*   ProcessState *proc = walker->getProcessState();
   assert(proc);

   sw_printf("[%s:%u] - Initializing BottomOfStackStepper\n", FILE__, __LINE__);
   
   LibraryState *libs = proc->getLibraryTracker();
   if (!libs) {
      sw_printf("[%s:%u] - Error initing StackBottom.  No library state for process.\n",
                FILE__, __LINE__);
      return;
   }
   SymbolReaderFactory *fact = Walker::getSymbolReader();
   if (!fact) {
      sw_printf("[%s:%u] - Failed to get symbol reader\n");
      return;
   }

   if (!aout_init)
   {
      LibAddrPair aout_addr;
      SymReader *aout = NULL;
      Symbol_t start_sym;
      bool result = libs->getAOut(aout_addr);
      if (result) {
         aout = fact->openSymbolReader(aout_addr.first);
         aout_init = true;
      }
      if (aout) {
         start_sym = aout->getSymbolByName(START_FUNC_NAME);
         if (aout->isValidSymbol(start_sym)) {
            Dyninst::Address start = aout->getSymbolOffset(start_sym)+aout_addr.second;
            Dyninst::Address end = aout->getSymbolSize(start_sym) + start;
            if (start == end) {
               sw_printf("[%s:%u] - %s symbol has 0 length, using length of %lu\n",
                       FILE__, __LINE__, START_FUNC_NAME, START_HEURISTIC_LENGTH);
               end = start + START_HEURISTIC_LENGTH;
            }
            sw_printf("[%s:%u] - Bottom stepper taking %lx to %lx for start\n", 
                      FILE__, __LINE__, start, end);
            ra_stack_tops.push_back(std::pair<Address, Address>(start, end));
         }
      }
   }

   if (!libthread_init)
   {
      LibAddrPair libthread_addr;
      SymReader *libthread = NULL;
      Symbol_t clone_sym, startthread_sym;
      bool result = libs->getLibthread(libthread_addr);
      if (result) {
         libthread = fact->openSymbolReader(libthread_addr.first);
         libthread_init = true;
      }
      if (libthread) {
         clone_sym = libthread->getSymbolByName(CLONE_FUNC_NAME);
         if (libthread->isValidSymbol(clone_sym)) {
            Dyninst::Address start = libthread->getSymbolOffset(clone_sym) + 
               libthread_addr.second;
            Dyninst::Address end = libthread->getSymbolSize(clone_sym) + start;
            sw_printf("[%s:%u] - Bottom stepper taking %lx to %lx for clone\n", 
                      FILE__, __LINE__, start, end);
            ra_stack_tops.push_back(std::pair<Address, Address>(start, end));
         }
         startthread_sym = libthread->getSymbolByName(START_THREAD_FUNC_NAME);
         if (libthread->isValidSymbol(startthread_sym)) {
            Dyninst::Address start = libthread->getSymbolOffset(startthread_sym) + 
               libthread_addr.second;
            Dyninst::Address end = libthread->getSymbolSize(startthread_sym) + start;
            sw_printf("[%s:%u] - Bottom stepper taking %lx to %lx for start_thread\n", 
                      FILE__, __LINE__, start, end);
            ra_stack_tops.push_back(std::pair<Address, Address>(start, end));
         }
      }
   }
*/
}
void BottomOfStackStepperImpl::newLibraryNotification(LibAddrPair *, lib_change_t)
{

}
