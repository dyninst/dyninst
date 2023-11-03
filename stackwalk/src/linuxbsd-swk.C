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
#include "common/src/vm_maps.h"
#include "stackwalk/src/sw.h"
#include "stackwalk/src/symtab-swk.h"
#include "stackwalk/src/libstate.h"
#include "stackwalk/src/get_trap_instruction.h"

#include <cerrno>
#include <cstring>

#include <sys/mman.h>
#include <sys/types.h>
#include <unistd.h>
#include <signal.h>

#include "Elf_X.h"

using namespace Dyninst;
using namespace Dyninst::Stackwalker;

#if defined(WITH_SYMLITE)
#include "symlite/h/SymLite-elf.h"
#elif defined(WITH_SYMTAB_API)
#include "symtabAPI/h/SymtabReader.h"
#else
#error "No defined symbol reader"
#endif

#include "linuxbsd-swk.h"


extern int P_gettid();

SymbolReaderFactory *Dyninst::Stackwalker::getDefaultSymbolReader()
{
#if defined(WITH_SYMLITE)
   static SymElfFactory symelffact;
   return &symelffact;
#elif defined(WITH_SYMTAB_API)
   return SymtabAPI::getSymtabReaderFactory();
#else
#error "No defined symbol reader"
#endif
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

void handle_fault(int /*sig*/) {
    if( !reading_memory ) {
      //The instruction that caused this fault was not from
      // ProcSelf::readMem.  Restore the SIGSEGV handler, and
      // the faulting instruction should restart after we return.
      fprintf(stderr, "[%s:%u] - Caught segfault that didn't come " \
              "from stackwalker memory read!", FILE__, __LINE__);
      signal(SIGSEGV, SIG_DFL);
      return;
   }
   siglongjmp(readmem_jmp, 1);
}

bool ProcSelf::readMem(void *dest, Address source, size_t size) {
    if( !registered_handler ) {
        signal(SIGSEGV, handle_fault);
        registered_handler = true;
    }
    reading_memory = true;
    if( sigsetjmp(readmem_jmp, 1) ) {
        sw_printf("[%s:%d] - Caught fault while reading from %lx to %lx\n",
                FILE__, __LINE__, source, source + size);
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

bool ProcSelf::getThreadIds(std::vector<THR_ID> &threads) {
    bool result;
    THR_ID tid;

    result = getDefaultThread(tid);

    if( !result ) {
        sw_printf("[%s:%d] - Could not read default thread\n",
                FILE__, __LINE__);
        return false;
    }

    threads.clear();
    threads.push_back(tid);
    return true;
}

bool ProcSelf::getDefaultThread(THR_ID &default_tid)
{
    THR_ID tid = P_gettid();
    if( tid <= 0 ) {
        const char *sys_err_msg = strerror(errno);
        sw_printf("[%s:%d] - gettid syscall failed with %s\n",
                FILE__, __LINE__, sys_err_msg);
        // Note, it's illegal to use a string temp for setLastError, so this
        // just sets a non-specific const message instead.  But P_gettid
        // should never fail anyway, as even ENOSYS is retried with getpid.
        setLastError(err_internal, "gettid syscall failed");
        return false;
    }

    default_tid = tid;
    return true;
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
      sw_printf("[%s:%d] - Not using lib tracker, don't know how "
                "to get library load address\n", FILE__, __LINE__);
      lib_trap_addr_self_err = true;
      return;
   }
   lib_trap_addr_self = libs->getLibTrapAddress();
   if (!lib_trap_addr_self) {
      sw_printf("[%s:%d] - Error getting trap address, can't install lib tracker",
                FILE__, __LINE__);
      lib_trap_addr_self_err = true;
      return;
   }

   //Use /proc/PID/maps to make sure that this address is valid and writable
   unsigned maps_size;
   map_entries *maps = getVMMaps(getpid(), maps_size);
   if (!maps) {
      sw_printf("[%s:%d] - Error reading proc/%d/maps.  Can't install lib tracker",
                FILE__, __LINE__, getpid());
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
            sw_printf("[%s:%d] - Error setting premissions for page containing %lx. "
                      "Can't install lib tracker: %s\n", FILE__, __LINE__,
                      lib_trap_addr_self, strerror(errnum));
            free(maps);
            lib_trap_addr_self_err = true;
            return;
         }
      }
   }
   free(maps);
   if (!found) {
      sw_printf("[%s:%d] - Couldn't find page containing %lx.  Can't install lib "
                "tracker.", FILE__, __LINE__, lib_trap_addr_self);
      lib_trap_addr_self_err = true;
      return;
   }

   char trap_buffer[MAX_TRAP_LEN];
   unsigned actual_len;
   getTrapInstruction(trap_buffer, MAX_TRAP_LEN, actual_len, true);

   local_lib_state = libs;
   signal(SIGTRAP, lib_trap_handler);

   memcpy((void*) lib_trap_addr_self, trap_buffer, actual_len);
   sw_printf("[%s:%d] - Successfully install lib tracker at 0x%lx\n",
            FILE__, __LINE__, lib_trap_addr_self);
}

void BottomOfStackStepperImpl::newLibraryNotification(LibAddrPair *, lib_change_t change)
{
   if (change == library_unload)
      return;
   if (!libthread_init || !aout_init) {
      initialize();
   }
}

#if defined(arch_aarch64)
static const int heuristic_length = 2;
static const Dyninst::Address heuristic_length_array[] = {0x38, 0x4C};
static const uint8_t expectedValue = 0x97;
#else
static const int heuristic_length = 1;
static const Dyninst::Address heuristic_length_array[] = {43};
static const uint8_t expectedValue = 0;
#endif

void BottomOfStackStepperImpl::initialize()
{
   ProcessState *proc = walker->getProcessState();
   assert(proc);

   sw_printf("[%s:%d] - Initializing BottomOfStackStepper\n", FILE__, __LINE__);

   LibraryState *libs = proc->getLibraryTracker();
   if (!libs) {
      sw_printf("[%s:%d] - Error initing StackBottom.  No library state for process.\n",
                FILE__, __LINE__);
      return;
   }
   SymbolReaderFactory *fact = Walker::getSymbolReader();
   if (!fact) {
      sw_printf("[%s:%d] - Failed to get symbol reader\n", FILE__, __LINE__);
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
               sw_printf("[%s:%d] - %s symbol has 0 length, using length heuristics\n",
                       FILE__, __LINE__, START_FUNC_NAME);
               if (heuristic_length == 1) {
                   end = start + heuristic_length_array[0];
               } else {
                   for (int i = 0; i < heuristic_length; ++i) {
                       end = start + heuristic_length_array[i];
                       uint8_t byteValue;
                       result = proc->readMem(&byteValue, end - 1, 1);
                       if (result && byteValue != expectedValue) continue; 
                       break;
                   }
               }
            }
            sw_printf("[%s:%d] - Bottom stepper taking %lx to %lx for start\n",
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
            sw_printf("[%s:%d] - Bottom stepper taking %lx to %lx for clone\n",
                      FILE__, __LINE__, start, end);
            ra_stack_tops.push_back(std::pair<Address, Address>(start, end));
         }
         startthread_sym = libthread->getSymbolByName(START_THREAD_FUNC_NAME);
         if (libthread->isValidSymbol(startthread_sym)) {
            Dyninst::Address start = libthread->getSymbolOffset(startthread_sym) +
               libthread_addr.second;
            Dyninst::Address end = libthread->getSymbolSize(startthread_sym) + start;
            sw_printf("[%s:%d] - Bottom stepper taking %lx to %lx for start_thread\n",
                      FILE__, __LINE__, start, end);
            ra_stack_tops.push_back(std::pair<Address, Address>(start, end));
         }
      }
   }
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
