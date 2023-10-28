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
#include "stackwalk/h/steppergroup.h"
#include "stackwalk/h/procstate.h"
#include "stackwalk/h/frame.h"

#include "stackwalk/src/linuxbsd-swk.h"
#include "stackwalk/src/sw.h"
#include "stackwalk/src/symtab-swk.h"
#include "stackwalk/src/libstate.h"

#include "common/src/parseauxv.h"

#include <string>
#include <sstream>

#include <string.h>
#include <sys/syscall.h>
#include <unistd.h>
#include <errno.h>
#include <assert.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <poll.h>

#include "common/src/parseauxv.h"

#include "symtabAPI/h/SymtabReader.h"

using namespace Dyninst;
using namespace Dyninst::Stackwalker;

#ifndef SYS_tkill
#define SYS_tkill 238
#endif

//These should be defined on all modern linux's, turn these off
// if porting to some linux-like platform that doesn't support 
// them.
#include <sys/ptrace.h>
typedef enum __ptrace_request pt_req;
#define cap_ptrace_traceclone
#define cap_ptrace_setoptions

int P_gettid()
{
  static int gettid_not_valid = 0;
  long int result;

  if (gettid_not_valid)
    return getpid();

  result = syscall(SYS_gettid);
  if (result == -1 && errno == ENOSYS)
  {
    gettid_not_valid = 1;
    return getpid();
  }
  return (int) result;
}

vsys_info *Dyninst::Stackwalker::getVsysInfo(ProcessState *ps)
{
/*
#if defined(arch_x86_64) || defined(arch_aarch64)
   if (ps->getAddressWidth() == 8)
      return NULL;
#endif
*/
   static std::map<ProcessState *, vsys_info *> vsysmap;
   vsys_info *ret = NULL;
   Address start, end;
   char *buffer = NULL;
   SymReader *reader = NULL;
   SymbolReaderFactory *fact = NULL;
   bool result;
   std::stringstream ss;
   std::map<ProcessState *, vsys_info *>::iterator i = vsysmap.find(ps);
   if (i != vsysmap.end())
      return i->second;
   
   AuxvParser *parser = AuxvParser::createAuxvParser(ps->getProcessId(),
                                                     ps->getAddressWidth());
   if (!parser) {
      sw_printf("[%s:%d] - Unable to parse auxv for %d\n", FILE__, __LINE__,
                ps->getProcessId());
      goto done;
   }

   start = parser->getVsyscallBase();
   end = parser->getVsyscallEnd();
   sw_printf("[%s:%d] - Found vsyscall over range %lx to %lx\n",
             FILE__, __LINE__, start, end);   
   parser->deleteAuxvParser();
   
   if (!start || !end || end == start)
   {
      sw_printf("[%s:%d] - Error collecting vsyscall base and end\n",
                FILE__, __LINE__);
      goto done;
   }

   ret = new vsys_info();
   assert(ret);
   ret->start = start;
   ret->end = end;

   buffer = (char *) malloc(end - start);
   assert(buffer);
   result = ps->readMem(buffer, start, end - start);
   if (!result) {
      sw_printf("[%s:%d] - Error reading vsys memory\n", FILE__, __LINE__);
      goto done;
   }
   ret->vsys_mem = buffer;

   fact = Walker::getSymbolReader();
   if (!fact) {
      sw_printf("[%s:%d] - No symbol reading capability\n",
                FILE__, __LINE__);
      goto done;
   }   
   reader = fact->openSymbolReader(buffer, end - start);
   if (!reader) {
      sw_printf("[%s:%d] - Error reading symbol info\n", FILE__, __LINE__);
      goto done;
   }
   ret->syms = reader;

   ss << "[vsyscall-" << ps->getProcessId() << "]";
   ret->name = ss.str();
   LibraryWrapper::registerLibrary(reader, ret->name);

  done:
   vsysmap[ps] = ret;
   return ret;
}

bool LibraryState::updateLibsArch(std::vector<std::pair<LibAddrPair, unsigned int> > &alibs)
{
   if (arch_libs.size()) {
      alibs = arch_libs;
      return true;
   }
   vsys_info *vsys = getVsysInfo(procstate);
   if (!vsys) {
      return false;
   }

   LibAddrPair vsyscall_page;
   vsyscall_page.first = vsys->name;
   vsyscall_page.second = vsys->start;
   std::pair<LibAddrPair, unsigned int> vsyscall_lib_pair;
   vsyscall_lib_pair.first = vsyscall_page;
   vsyscall_lib_pair.second = static_cast<unsigned int>(vsys->end - vsys->start);
   arch_libs.push_back(vsyscall_lib_pair);
   alibs = arch_libs;

   return true;
}

static const char* vsys_sigreturns[] = {
   "sigreturn",
   "_sigreturn",
   "__sigreturn",
   "__kernel_sigreturn",
   "__kernel_rt_sigreturn"
};
static const size_t NUM_VSYS_SIGRETURNS = sizeof vsys_sigreturns / sizeof vsys_sigreturns[0];

void SigHandlerStepperImpl::registerStepperGroup(StepperGroup *group)
{
  sw_printf("[%s:%d] - Begin SigHandlerStepperImpl::registerStepperGroup\n", FILE__, __LINE__);
   ProcessState *ps = getProcessState();
   assert(ps);

   LibraryState *libs = getProcessState()->getLibraryTracker();
   if (!libs) {
      sw_printf("[%s:%d] - Custom library tracker.  Don't know how to"
                " to get libc\n", FILE__, __LINE__);
      return;
   }
   SymbolReaderFactory *fact = Walker::getSymbolReader();
   if (!fact) {
      sw_printf("[%s:%d] - Failed to get symbol reader\n", FILE__, __LINE__);
      return;
   }
  sw_printf("[%s:%d] - Got lib tracker and sym reader OK, checking for __restore_rt...\n", FILE__, __LINE__);

   if (!init_libc) {
      /**
       * Get __restore_rt out of libc
       **/
      sw_printf("[%s:%d] - Getting __restore_rt from libc\n", FILE__, __LINE__);
      LibAddrPair libc_addr;
      Dyninst::SymReader *libc = NULL;
      Symbol_t libc_restore;
      bool result = libs->getLibc(libc_addr);
      if (!result) {
         sw_printf("[%s:%d] - Unable to find libc, not registering restore_rt"
                   "tracker.\n", FILE__, __LINE__);
      }
      if (!init_libc) {
         if (result) {
            init_libc = true;
            libc = fact->openSymbolReader(libc_addr.first);
            if (!libc) {
               sw_printf("[%s:%d] - Unable to open libc, not registering restore_rt\n",
                         FILE__, __LINE__);
            }   
         }
         if (result && libc) {
            init_libc = true;
            libc_restore = libc->getSymbolByName("__restore_rt");
            if (!libc->isValidSymbol(libc_restore)) {
               sw_printf("[%s:%d] - Unable to find restore_rt in libc\n",
                         FILE__, __LINE__);
            }
            else {
               Dyninst::Address start = libc->getSymbolOffset(libc_restore) + libc_addr.second;
               Dyninst::Address end = libc->getSymbolSize(libc_restore) + start;
               if (start == end)
                  end = start + 16; //Estimate--annoying
               sw_printf("[%s:%d] - Registering libc restore_rt as at %lx to %lx\n",
                         FILE__, __LINE__, start, end);
               group->addStepper(parent_stepper, start, end);
            }
         }
      }
   }

   if (!init_libthread) {
      /**
       * Get __restore_rt out of libpthread
       **/
      sw_printf("[%s:%d] - Getting __restore_rt out of libpthread\n", FILE__, __LINE__);
      LibAddrPair libpthread_addr;
      Dyninst::SymReader *libpthread = NULL;
      Symbol_t libpthread_restore;
      bool result  = libs->getLibthread(libpthread_addr);
      if (!result) {
         sw_printf("[%s:%d] - Unable to find libpthread, not registering restore_rt"
                   "pthread tracker.\n", FILE__, __LINE__);
      }
      if (result) {
         libpthread = fact->openSymbolReader(libpthread_addr.first);
         if (!libpthread) {
            sw_printf("[%s:%d] - Unable to open libc, not registering restore_rt\n",
                      FILE__, __LINE__);
         }
         init_libthread = true;
      }
      if (libpthread) {
         libpthread_restore = libpthread->getSymbolByName("__restore_rt");
         if (!libpthread->isValidSymbol(libpthread_restore)) {
            sw_printf("[%s:%d] - Unable to find restore_rt in libpthread\n",
                      FILE__, __LINE__);
         }
         else {
            Dyninst::Address start = libpthread->getSymbolOffset(libpthread_restore) + libpthread_addr.second;
            Dyninst::Address end = libpthread->getSymbolSize(libpthread_restore) + start;
            if (start == end)
               end = start + 16; //Estimate--annoying
            sw_printf("[%s:%d] - Registering libpthread restore_rt as at %lx to %lx\n",
                      FILE__, __LINE__, start, end);
            group->addStepper(parent_stepper, start, end);
         }
      }   
   }

   /**
    * Get symbols out of vsyscall page
    **/
   sw_printf("[%s:%d] - Getting vsyscall page symbols\n", FILE__, __LINE__);
   vsys_info *vsyscall = getVsysInfo(ps);
   if (!vsyscall)
   {
#if !defined(arch_x86_64) && !defined(arch_aarch64)
      sw_printf("[%s:%d] - Odd.  Couldn't find vsyscall page. Signal handler"
                " stepping may not work\n", FILE__, __LINE__);
#endif
   }
   else
   {
      SymReader *vsys_syms = vsyscall->syms;
      if (!vsys_syms) {
         sw_printf("[%s:%d] - Vsyscall page wasn't parsed\n", FILE__, __LINE__);
      }
      else {
         for (unsigned i=0; i<NUM_VSYS_SIGRETURNS; i++)
         {
            Symbol_t sym;
            sym = vsys_syms->getSymbolByName(vsys_sigreturns[i]);
            if (!vsys_syms->isValidSymbol(sym))
               continue;
            
            Dyninst::Offset offset = vsys_syms->getSymbolOffset(sym);
            Dyninst::Address addr;
            if (offset < vsyscall->start)
               addr = offset + vsyscall->start;
            else
               addr = offset;
            unsigned long size = vsys_syms->getSymbolSize(sym);
            if (!size) 
               size = ps->getAddressWidth();
            
            group->addStepper(parent_stepper, addr, addr + size);
         }
      }
   }
}
