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

#include "stackwalk/h/swk_errors.h"
#include "stackwalk/h/symlookup.h"
#include "stackwalk/h/walker.h"
#include "stackwalk/h/steppergroup.h"
#include "stackwalk/h/procstate.h"
#include "stackwalk/h/frame.h"

#include "stackwalk/src/linux-swk.h"
#include "stackwalk/src/sw.h"
#include "stackwalk/src/symtab-swk.h"
#include "stackwalk/src/libstate.h"

#include "common/h/linuxKludges.h"
#include "common/h/parseauxv.h"
#include "common/h/Types.h"

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

#include "common/h/SymLite-elf.h"
#include "common/h/parseauxv.h"
#include "dynutil/h/dyn_regs.h"

using namespace Dyninst;
using namespace Dyninst::Stackwalker;

#ifndef SYS_tkill
#define SYS_tkill 238
#endif

//These should be defined on all modern linux's, turn these off
// if porting to some linux-like platform that doesn't support 
// them.
#include <sys/ptrace.h>
#include <linux/ptrace.h>
typedef enum __ptrace_request pt_req;
#define cap_ptrace_traceclone
#define cap_ptrace_setoptions

static int P_gettid()
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

SymbolReaderFactory *Dyninst::Stackwalker::getDefaultSymbolReader()
{
   static SymElfFactory symelffact;
   return &symelffact;
}

class Elf_X;
Elf_X *getElfHandle(std::string s)
{
   SymReader *reader = LibraryWrapper::getLibrary(s);
   if (!reader) {
      SymbolReaderFactory *fact = getDefaultSymbolReader();
      reader = fact->openSymbolReader(s);
   }
   SymElf *symelf = dynamic_cast<SymElf *>(reader);
   if (symelf)
      return symelf->getElfHandle();
   return NULL;
}

static void registerLibSpotterSelf(ProcSelf *pself);

ProcSelf::ProcSelf(std::string exe_path) :
   ProcessState(getpid(), exe_path)
{
}

void ProcSelf::initialize()
{
   setDefaultLibraryTracker();
   assert(library_tracker);
   registerLibSpotterSelf(this);
}

#if defined(cap_sw_catchfaults)

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
      fprintf(stderr, "[%s:%u] - Caught segfault that didn't come " \
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
#else
bool ProcSelf::readMem(void *dest, Address source, size_t size)
{
  memcpy(dest, (const void *) source, size);
  return true;
}
#endif

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
  THR_ID tid = P_gettid();
  if (tid == -1) {
    const char *sys_err_msg = strerror(errno);
    sw_printf("[%s:%u] - gettid syscall failed with %s\n",
	       __FILE__, __LINE__, sys_err_msg);
    std::string errmsg("gettid syscall failed with ");
    errmsg += sys_err_msg;
    setLastError(err_internal, errmsg.c_str());
    return false;
  }

  default_tid = tid;
  return true;
}

vsys_info *Dyninst::Stackwalker::getVsysInfo(ProcessState *ps)
{
#if defined(arch_x86_64)
   if (ps->getAddressWidth() == 8)
      return NULL;
#endif

   static std::map<ProcessState *, vsys_info *> vsysmap;
   vsys_info *ret = NULL;
   Address start, end;
   char *buffer = NULL;
   SymReader *reader = NULL;
   SymbolReaderFactory *fact = NULL;
   bool result;

   std::map<ProcessState *, vsys_info *>::iterator i = vsysmap.find(ps);
   if (i != vsysmap.end())
      return i->second;
   
   AuxvParser *parser = AuxvParser::createAuxvParser(ps->getProcessId(),
                                                     ps->getAddressWidth());
   if (!parser) {
      sw_printf("[%s:%u] - Unable to parse auxv for %d\n", __FILE__, __LINE__,
                ps->getProcessId());
      goto done;
   }

   start = parser->getVsyscallBase();
   end = parser->getVsyscallEnd();
   sw_printf("[%s:%u] - Found vsyscall over range %lx to %lx\n",
             __FILE__, __LINE__, start, end);   
   parser->deleteAuxvParser();
   
   if (!start || !end || end == start)
   {
      sw_printf("[%s:%u] - Error collecting vsyscall base and end\n",
                __FILE__, __LINE__);
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
      sw_printf("[%s:%u] - Error reading vsys memory\n", __FILE__, __LINE__);
      goto done;
   }
   ret->vsys_mem = buffer;

   fact = getDefaultSymbolReader();
   if (!fact) {
      sw_printf("[%s:%u] - No symbol reading capability\n",
                __FILE__, __LINE__);
      goto done;
   }   
   reader = fact->openSymbolReader(buffer, end - start);
   if (!reader) {
      sw_printf("[%s:%u] - Error reading symbol info\n");
      goto done;
   }
   ret->syms = reader;

  done:
   vsysmap[ps] = ret;
   return ret;
}

SigHandlerStepperImpl::SigHandlerStepperImpl(Walker *w, SigHandlerStepper *parent) :
   FrameStepper(w),
   parent_stepper(parent),
   init_libc(false),
   init_libthread(false)
{
}

unsigned SigHandlerStepperImpl::getPriority() const
{
   return sighandler_priority;
}

SigHandlerStepperImpl::~SigHandlerStepperImpl()
{
}

void SigHandlerStepperImpl::newLibraryNotification(LibAddrPair *, lib_change_t change)
{
   if (change == library_unload)
      return;
   StepperGroup *group = getWalker()->getStepperGroup();
   registerStepperGroup(group);
}


static LibraryState *local_lib_state = NULL;
extern "C" {
   static void lib_trap_handler(int sig);
}
static void lib_trap_handler(int /*sig*/)
{
   local_lib_state->notifyOfUpdate();
}

static Address lib_trap_addr_self = 0x0;
static bool lib_trap_addr_self_err = false;
static void registerLibSpotterSelf(ProcSelf *pself)
{
   if (lib_trap_addr_self)
      return;
   if (lib_trap_addr_self_err)
      return;

   //Get the address to install a trap to
   LibraryState *libs = pself->getLibraryTracker();
   if (!libs) {
      sw_printf("[%s:%u] - Not using lib tracker, don't know how "
                "to get library load address\n", __FILE__, __LINE__);
      lib_trap_addr_self_err = true;
      return;
   }   
   lib_trap_addr_self = libs->getLibTrapAddress();
   if (!lib_trap_addr_self) {
      sw_printf("[%s:%u] - Error getting trap address, can't install lib tracker",
                __FILE__, __LINE__);
      lib_trap_addr_self_err = true;
      return;
   }

   //Use /proc/PID/maps to make sure that this address is valid and writable
   unsigned maps_size;
   map_entries *maps = getVMMaps(getpid(), maps_size);
   if (!maps) {
      sw_printf("[%s:%u] - Error reading proc/%d/maps.  Can't install lib tracker",
                __FILE__, __LINE__, getpid());
      lib_trap_addr_self_err = true;
      return;
   }

   bool found = false;
   for (unsigned i=0; i<maps_size; i++) {
      if (maps[i].start <= lib_trap_addr_self && 
          maps[i].end > lib_trap_addr_self)
      {
         found = true;
         if (maps[i].prems & PREMS_WRITE) {
            break;
         }
         int pgsize = getpagesize();
         Address first_page = (lib_trap_addr_self / pgsize) * pgsize;
         unsigned size = pgsize;
         if (first_page + size < lib_trap_addr_self+MAX_TRAP_LEN)
            size += pgsize;
         int result = mprotect((void*) first_page,
                               size, 
                               PROT_READ|PROT_WRITE|PROT_EXEC);
         if (result == -1) {
            int errnum = errno;
            sw_printf("[%s:%u] - Error setting premissions for page containing %lx. "
                      "Can't install lib tracker: %s\n", __FILE__, __LINE__, 
                      lib_trap_addr_self, strerror(errnum));
            free(maps);
            lib_trap_addr_self_err = true;
            return;
         }
      }
   }
   free(maps);
   if (!found) {
      sw_printf("[%s:%u] - Couldn't find page containing %lx.  Can't install lib "
                "tracker.", __FILE__, __LINE__, lib_trap_addr_self);
      lib_trap_addr_self_err = true;
      return;
   }

   char trap_buffer[MAX_TRAP_LEN];
   unsigned actual_len;
   getTrapInstruction(trap_buffer, MAX_TRAP_LEN, actual_len, true);

   local_lib_state = libs;
   signal(SIGTRAP, lib_trap_handler);

   memcpy((void*) lib_trap_addr_self, trap_buffer, actual_len);   
   sw_printf("[%s:%u] - Successfully install lib tracker at 0x%lx\n",
            __FILE__, __LINE__, lib_trap_addr_self);
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
   std::stringstream ss;
   ss << "[vsyscall-" << procstate->getProcessId() << "]";
   LibAddrPair vsyscall_page;
   vsyscall_page.first = ss.str();
   vsyscall_page.second = vsys->start;
   
   SymbolReaderFactory *fact = getDefaultSymbolReader();
   SymReader *reader = fact->openSymbolReader((char *) vsys->vsys_mem,
                                              vsys->end - vsys->start);
   if (reader)
      LibraryWrapper::registerLibrary(reader, vsyscall_page.first);

   std::pair<LibAddrPair, unsigned int> vsyscall_lib_pair;
   vsyscall_lib_pair.first = vsyscall_page;
   vsyscall_lib_pair.second = static_cast<unsigned int>(vsys->end - vsys->start);
   arch_libs.push_back(vsyscall_lib_pair);
   alibs = arch_libs;

   return true;
}

#define NUM_VSYS_SIGRETURNS 3
static const char* vsys_sigreturns[] = {
   "_sigreturn",
   "__kernel_sigreturn",
   "__kernel_rt_sigreturn"
};

void SigHandlerStepperImpl::registerStepperGroup(StepperGroup *group)
{
   ProcessState *ps = getProcessState();
   assert(ps);

   LibraryState *libs = getProcessState()->getLibraryTracker();
   if (!libs) {
      sw_printf("[%s:%u] - Custom library tracker.  Don't know how to"
                " to get libc\n", __FILE__, __LINE__);
      return;
   }
   SymbolReaderFactory *fact = getDefaultSymbolReader();
   if (!fact) {
      sw_printf("[%s:%u] - Failed to get symbol reader\n", __FILE__, __LINE__);
      return;
   }

   if (!init_libc) {
      /**
       * Get __restore_rt out of libc
       **/
      LibAddrPair libc_addr;
      Dyninst::SymReader *libc = NULL;
      Symbol_t libc_restore;
      bool result = libs->getLibc(libc_addr);
      if (!result) {
         sw_printf("[%s:%u] - Unable to find libc, not registering restore_rt"
                   "tracker.\n", __FILE__, __LINE__);
      }
      if (result) {
         init_libc = true;
         libc = fact->openSymbolReader(libc_addr.first);
         if (!libc) {
            sw_printf("[%s:%u] - Unable to open libc, not registering restore_rt\n",
                      __FILE__, __LINE__);
         }   
      }
      if (libc) {
         libc_restore = libc->getSymbolByName("__restore_rt");
         if (!libc->isValidSymbol(libc_restore)) {
            sw_printf("[%s:%u] - Unable to find restore_rt in libc\n",
                      __FILE__, __LINE__);
         }
         else {
            Dyninst::Address start = libc->getSymbolOffset(libc_restore);
            Dyninst::Address end = libc->getSymbolSize(libc_restore) + start;
            if (start == end)
               end = start + 16; //Estimate--annoying
            sw_printf("[%s:%u] - Registering libc restore_rt as at %lx to %lx\n",
                      __FILE__, __LINE__, start, end);
            group->addStepper(parent_stepper, start, end);
         }
      }
   }

   if (!init_libthread) {
      /**
       * Get __restore_rt out of libpthread
       **/
      LibAddrPair libpthread_addr;
      Dyninst::SymReader *libpthread = NULL;
      Symbol_t libpthread_restore;
      bool result  = libs->getLibthread(libpthread_addr);
      if (!result) {
         sw_printf("[%s:%u] - Unable to find libpthread, not registering restore_rt"
                   "pthread tracker.\n", __FILE__, __LINE__);
      }
      if (result) {
         libpthread = fact->openSymbolReader(libpthread_addr.first);
         if (!libpthread) {
            sw_printf("[%s:%u] - Unable to open libc, not registering restore_rt\n",
                      __FILE__, __LINE__);
         }
         init_libthread = true;
      }
      if (libpthread) {
         libpthread_restore = libpthread->getSymbolByName("__restore_rt");
         if (!result) {
            sw_printf("[%s:%u] - Unable to find restore_rt in libc\n",
                      __FILE__, __LINE__);
         }
         else {
            Dyninst::Address start = libpthread->getSymbolOffset(libpthread_restore);
            Dyninst::Address end = libpthread->getSymbolSize(libpthread_restore) + start;
            if (start == end)
               end = start + 16; //Estimate--annoying
            sw_printf("[%s:%u] - Registering libpthread restore_rt as at %lx to %lx\n",
                      __FILE__, __LINE__, start, end);
            group->addStepper(parent_stepper, start, end);
         }
      }   
   }

   /**
    * Get symbols out of vsyscall page
    **/
   vsys_info *vsyscall = getVsysInfo(ps);
   if (!vsyscall)
   {
#if !defined(arch_x86_64)
      sw_printf("[%s:%u] - Odd.  Couldn't find vsyscall page. Signal handler"
                " stepping may not work\n", __FILE__, __LINE__);
#endif
   }
   else
   {
      SymReader *vsys_syms = vsyscall->syms;
      if (!vsys_syms) {
         sw_printf("[%s:%u] - Vsyscall page wasn't parsed\n", __FILE__, __LINE__);
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

void BottomOfStackStepperImpl::initialize()
{
   ProcessState *proc = walker->getProcessState();
   assert(proc);

   sw_printf("[%s:%u] - Initializing BottomOfStackStepper\n", __FILE__, __LINE__);
   
   LibraryState *libs = proc->getLibraryTracker();
   if (!libs) {
      sw_printf("[%s:%u] - Error initing StackBottom.  No library state for process.\n",
                __FILE__, __LINE__);
      return;
   }
   SymbolReaderFactory *fact = getDefaultSymbolReader();
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
         start_sym = aout->getSymbolByName("_start");
         if (aout->isValidSymbol(start_sym)) {
            Dyninst::Address start = aout->getSymbolOffset(start_sym)+aout_addr.second;
            Dyninst::Address end = aout->getSymbolSize(start_sym) + start;
            if (start == end)
               end = start + 43;
            sw_printf("[%s:%u] - Bottom stepper taking %lx to %lx for start\n", 
                      __FILE__, __LINE__, start, end);
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
         clone_sym = libthread->getSymbolByName("__clone");
         if (libthread->isValidSymbol(clone_sym)) {
            Dyninst::Address start = libthread->getSymbolOffset(clone_sym) + 
               libthread_addr.second;
            Dyninst::Address end = libthread->getSymbolSize(clone_sym) + start;
            sw_printf("[%s:%u] - Bottom stepper taking %lx to %lx for clone\n", 
                      __FILE__, __LINE__, start, end);
            ra_stack_tops.push_back(std::pair<Address, Address>(start, end));
         }
         startthread_sym = libthread->getSymbolByName("start_thread");
         if (libthread->isValidSymbol(startthread_sym)) {
            Dyninst::Address start = libthread->getSymbolOffset(startthread_sym) + 
               libthread_addr.second;
            Dyninst::Address end = libthread->getSymbolSize(startthread_sym) + start;
            sw_printf("[%s:%u] - Bottom stepper taking %lx to %lx for start_thread\n", 
                      __FILE__, __LINE__, start, end);
            ra_stack_tops.push_back(std::pair<Address, Address>(start, end));
         }
      }
   }
}

void BottomOfStackStepperImpl::newLibraryNotification(LibAddrPair *, lib_change_t change)
{
   if (change == library_unload)
      return;
   if (!libthread_init || !aout_init) {
      initialize();
   }
}

