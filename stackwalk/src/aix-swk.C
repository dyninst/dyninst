/*
 * Copyright (c) 1996-2011 Barton P. Miller
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

#include "stackwalk/h/swk_errors.h"
#include "stackwalk/h/symlookup.h"
#include "stackwalk/h/walker.h"
#include "stackwalk/h/steppergroup.h"
#include "stackwalk/h/procstate.h"
#include "stackwalk/h/frame.h"

#include "stackwalk/src/sw.h"
#include "stackwalk/src/symtab-swk.h"

#include "common/h/headers.h"
#include "common/h/Types.h"

#include <string>
#include <sstream>

#include <string.h>
#include <errno.h>
#include <assert.h>
#include <signal.h>
#include <sys/thread.h>

using namespace Dyninst;
using namespace Dyninst::Stackwalker;

#if defined(cap_stackwalker_use_symtab)

#include "symtabAPI/h/Symtab.h"
#include "symtabAPI/h/Symbol.h"

using namespace Dyninst::SymtabAPI;
#endif

ProcSelf::ProcSelf(std::string exec_path) : 
   ProcessState(getpid(), exec_path)
{
}

void ProcSelf::initialize()
{
   setDefaultLibraryTracker();
   assert(library_tracker);
}

#include <setjmp.h>

static bool registered_handler = false;
static bool reading_memory = false;
sigjmp_buf readmem_jmp;

void handle_fault(int /*sig*/)
{
   if (!reading_memory) {
      //The instruction that caused this fault was not from
      // ProcSelf::readMem.  Restore the SIGSEGV handler, and 
      // the faulting instruction should restart after we return.
      sw_printf("[%s:%u] - Caught segfault that didn't come " \
                "from stackwalker memory read!", __FILE__, __LINE__);
      signal(SIGSEGV, SIG_DFL);
      return;
   }
   siglongjmp(readmem_jmp, 1);
}
 
bool ProcSelf::readMem(void *dest, Address source, size_t size)
{
   if (!registered_handler) {
      signal(SIGSEGV, handle_fault);
      registered_handler = true;
   }
   reading_memory = true;
   if (sigsetjmp(readmem_jmp, 1)) {
      sw_printf("[%s:%u] - Caught fault while reading from %lx to %lx\n", 
                __FILE__, __LINE__, source, source + size);
      setLastError(err_procread, "Could not read from process");
      return false;
   }
   
   memcpy(dest, (const void *) source, size);
   reading_memory = false;
   return true;
}

bool ProcSelf::getThreadIds(std::vector<THR_ID> &threads)
{
  bool result;
  THR_ID tid;

  result = getDefaultThread(tid);
  if (!result) {
    sw_printf("[%s:%u] - Could not read default thread\n", __FILE__, __LINE__);
    return false;
  }
  threads.clear();
  threads.push_back(tid);
  return true;
}

bool ProcSelf::getDefaultThread(THR_ID &default_tid)
{
   default_tid = thread_self();
   return true;
}

bool Walker::createDefaultSteppers()
{
  FrameStepper *stepper;
  bool result;

  stepper = new FrameFuncStepper(this);
  result = addStepper(stepper);
  if (!result) {
    sw_printf("[%s:%u] - Error adding stepper %p\n", __FILE__, __LINE__,
	      stepper);
    return false;
  }

  stepper = new BottomOfStackStepper(this);
  result = addStepper(stepper);
  if (!result) {
    sw_printf("[%s:%u] - Error adding stepper %p\n", __FILE__, __LINE__,
	      stepper);
    return false;
  }  

  return true;
}

#if defined(cap_stackwalker_use_symtab)

bool SymtabLibState::updateLibsArch()
{
   return true;
}

static bool libNameMatch(const char *s, const char *libname)
{
   // A poor-man's regex match for */lib<s>[0123456789-.]*.a*
   const char *filestart = strrchr(libname, '/');
   if (!filestart)
      filestart = libname;
   const char *lib_start = strstr(filestart, "lib");
   if (lib_start != filestart+1)
      return false;
   const char *libname_start = lib_start+3;
   int s_len = strlen(s);
   if (strncmp(s, libname_start, s_len) != 0) {
      return false;
   }

   const char *cur = libname_start + s_len;
   const char *valid_chars = "0123456789-.";
   while (*cur) {
      if (!strchr(valid_chars, *cur)) {
         cur--;
         if (strstr(cur, ".so") == cur) {
            return true;
         }
         if (strstr(cur, ".a") == cur) {
            return true;
         }
         return false;
      }
      cur++;
   }
   return false;
}

bool SymtabLibState::getLibc(LibAddrPair &addr_pair)
{
   std::vector<LibAddrPair> libs;
   getLibraries(libs);
   if (libs.size() == 1) {
      //Static binary.
      addr_pair = libs[0];
      return true;
   }
   for (std::vector<LibAddrPair>::iterator i = libs.begin(); i != libs.end(); i++)
   {
      if (libNameMatch("c", i->first.c_str())) {
         addr_pair = *i;
         return true;
      }
   }
   return false;
}

bool SymtabLibState::getLibpthread(LibAddrPair &addr_pair)
{
   std::vector<LibAddrPair> libs;
   getLibraries(libs);
   if (libs.size() == 1) {
      //Static binary.
      addr_pair = libs[0];
      return true;
   }
   for (std::vector<LibAddrPair>::iterator i = libs.begin(); i != libs.end(); i++)
   {
      if (libNameMatch("pthread", i->first.c_str())) {
         addr_pair = *i;
         return true;
      }
   }
   return false;
}

#endif

void BottomOfStackStepperImpl::initialize()
{
#if defined(cap_stackwalker_use_symtab)
   std::vector<LibAddrPair> libs;
   ProcessState *proc = walker->getProcessState();
   assert(proc);

   sw_printf("[%s:%u] - Initializing BottomOfStackStepper\n", __FILE__, __LINE__);
   initialized = true;
   
   LibraryState *ls = proc->getLibraryTracker();
   if (!ls) {
      sw_printf("[%s:%u] - Error initing StackBottom.  No library state for process.\n",
                __FILE__, __LINE__);
      return;
   }
   SymtabLibState *symtab_ls = dynamic_cast<SymtabLibState *>(ls);
   if (!symtab_ls) {
      sw_printf("[%s:%u] - Error initing StackBottom. Unknown library state.\n",
                __FILE__, __LINE__);
   }

   bool result = false;
   std::vector<Function *> funcs;
   std::vector<Function *>::iterator i;

   //Find main in a.out
   LibAddrPair aout_libaddr = symtab_ls->getAOut();
   Symtab *aout = SymtabWrapper::getSymtab(aout_libaddr.first);
   if (aout) {
      result = aout->findFunctionsByName(funcs, "main");
      if (!result || !funcs.size()) {
         sw_printf("[%s:%u] - Error. Could not locate main\n", __FILE__, __LINE__);
      }
      for (i = funcs.begin(); i != funcs.end(); i++) {
         Address start = (*i)->getOffset() + aout_libaddr.second;
         Address end = start + (*i)->getSize();
         sw_printf("[%s:%u] - Adding _start stack bottom [0x%lx, 0x%lx]\n",
                   __FILE__, __LINE__, start, end);
         ra_stack_tops.push_back(std::pair<Address, Address>(start, end));
      }
   }
   else {
      sw_printf("[%s:%u] - Error. Could not locate a.out\n", __FILE__, __LINE__);
   }

   //Find pthread_body in 
   LibAddrPair pthread_libaddr;
   if (symtab_ls->getLibpthread(pthread_libaddr))
   {
      Symtab *libpthread = SymtabWrapper::getSymtab(aout_libaddr.first);
      if (libpthread) {
         result = libpthread->findFunctionsByName(funcs, "_pthread_body");
         if (!result || !funcs.size()) {
            sw_printf("[%s:%u] - Error. Could not locate main\n", __FILE__, __LINE__);
         }
         for (i = funcs.begin(); i != funcs.end(); i++) {
            Address start = (*i)->getOffset() + aout_libaddr.second;
            Address end = start + (*i)->getSize();
            sw_printf("[%s:%u] - Adding _start stack bottom [0x%lx, 0x%lx]\n",
                      __FILE__, __LINE__, start, end);
            ra_stack_tops.push_back(std::pair<Address, Address>(start, end));
         }
      }
   }
#endif
}
