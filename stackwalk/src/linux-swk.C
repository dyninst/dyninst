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

ProcDebug::thread_map_t ProcDebugLinux::all_threads;
std::map<pid_t, int> ProcDebugLinux::unknown_pid_events;

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

static bool t_kill(int pid, int sig)
{
  static bool has_tkill = true;
  long int result = 0;
  sw_printf("[%s:%u] - Sending %d to %d\n", __FILE__, __LINE__, sig, pid);
  if (has_tkill) {
     result = syscall(SYS_tkill, pid, sig);
     if (result == -1 && errno == ENOSYS)
     {
        sw_printf("[%s:%d] - Using kill instead of tkill on this system\n", 
                  __FILE__, __LINE__, sig, pid);
        has_tkill = false;
     }
  }
  if (!has_tkill) {
     result = kill(pid, sig);
  }

  return (result == 0);
}

SymbolReaderFactory *Dyninst::Stackwalker::getDefaultSymbolReader()
{
   static SymElfFactory symelffact;
   return &symelffact;
}

class Elf_X;
Elf_X *getElfHandle(std::string s)
{
   SymbolReaderFactory *fact = getDefaultSymbolReader();
   SymReader *reader = fact->openSymbolReader(s);
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

bool ProcDebugLinux::isLibraryTrap(Dyninst::THR_ID thrd)
{
   LibraryState *ls = getLibraryTracker();
   if (!ls)
      return false;
   Address lib_trap_addr = ls->getLibTrapAddress();
   if (!lib_trap_addr)
      return false;

   MachRegisterVal cur_pc;
   MachRegister reg = MachRegister::getPC(getArchitecture());
   bool result = getRegValue(reg, thrd, cur_pc);
   if (!result) {
      sw_printf("[%s:%u] - Error getting PC value for thrd %d\n",
                __FILE__, __LINE__, (int) thrd);
      return false;
   }
   if (cur_pc == lib_trap_addr || cur_pc-1 == lib_trap_addr)
      return true;
   return false;
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

static bool pipeHasData(int fd)
{
   if (fd == -1)
      return false;

   struct pollfd pfd[1];
   pfd[0].fd = fd;
   pfd[0].events = POLLIN;
   pfd[0].revents = 0;
   int result = poll(pfd, 1, 0);
   return (result == 1 && pfd[0].revents & POLLIN);
}

DebugEvent ProcDebug::debug_get_event(bool block)
{
   int status = 0;
   DebugEvent ev;
   pid_t p;

   bool dataOnPipe = pipeHasData(pipe_out);

   int flags = __WALL;
   if (!block)
     flags |= WNOHANG;
   sw_printf("[%s:%u] - Calling waitpid(-1, %p, %s)\n",
             __FILE__, __LINE__, &status, block ? "WALL|WNOHANG" : "WALL");
   p = waitpid(-1, &status, flags);
   sw_printf("[%s:%u] - waitpid returned status = 0x%x, p = %d\n",
             __FILE__, __LINE__, status, p);
   if (p == -1) {
      int errnum = errno;
      sw_printf("[%s:%u] - Unable to wait for debug event: %s\n",
                __FILE__, __LINE__, strerror(errnum));
      if (errnum == EINTR)
         setLastError(err_interrupt, "System call interrupted");
      else
         setLastError(err_internal, "Error calling waitpid");
      ev.dbg = dbg_err;
      return ev;
   }

   //If we got an event, or if we didn't read a message and there
   // is a byte on the pipe, then remove one byte from the pipe.
   if (pipe_out != -1 && 
       (p != 0 || dataOnPipe)) 
   {
      char c;
      sw_printf("[%s:%u] - Removing data from pipe, %d, %s\n",
                __FILE__, __LINE__, p, dataOnPipe ? "true" : "false");
      read(pipe_out, &c, 1);
   }
   
   if (p == 0) {
     sw_printf("[%s:%u] - No debug events available\n", __FILE__, __LINE__);
     ev.dbg = dbg_noevent;
     return ev;
   }

   thread_map_t::iterator i = ProcDebugLinux::all_threads.find(p);
   if (i == ProcDebugLinux::all_threads.end())
   {
      sw_printf("[%s:%u] - Warning, recieved unknown pid %d from waitpid\n",
                __FILE__, __LINE__, p);
      ProcDebugLinux::unknown_pid_events[p] = status;
      ev.dbg = dbg_noevent;
      return ev;
   }
   
   ev.thr = i->second;
   ev.proc = ev.thr->proc();

   if (WIFEXITED(status)) 
   {
      ev.dbg = dbg_exited;
      ev.data.idata = WEXITSTATUS(status);
      sw_printf("[%s:%u] - Process %d exited with %d\n", 
                __FILE__, __LINE__, p, ev.dbg);
   }
   else if (WIFSIGNALED(status))
   {
      ev.dbg = dbg_crashed;
      ev.data.idata = WTERMSIG(status);
      sw_printf("[%s:%u] - Process %d crashed with %d\n", 
                __FILE__, __LINE__, p, ev.dbg);
   }
   else if (WIFSTOPPED(status))
   {
      ev.dbg = dbg_stopped;
      ev.data.idata = WSTOPSIG(status);
      
      ProcDebugLinux *linux_proc = dynamic_cast<ProcDebugLinux*>(ev.proc);
      assert(linux_proc);
      if (ev.data.idata == SIGTRAP) {
         if (linux_proc->state() == ps_running && 
             linux_proc->isLibraryTrap((Dyninst::THR_ID) p)) 
         {
            sw_printf("[%s:%u] - Decoded library load event\n",
                      __FILE__, __LINE__);
            ev.dbg = dbg_libraryload;
         }
         else {
            int extended_data = status >> 16;
            if (extended_data)
               ev.data.idata = (extended_data << 8) | SIGTRAP;
         }
      }
      sw_printf("[%s:%u] - Process %d stopped with %d\n",
                __FILE__, __LINE__, p, ev.data);
   }
   else
   {
      sw_printf("[%s:%u] - Process %d had strange return value from waitpid\n",
                __FILE__, __LINE__, p);
      setLastError(err_internal, "Error calling waitpid");
      ev.dbg = dbg_err;
   }
   return ev;
}

bool ProcDebugLinux::debug_handle_event(DebugEvent ev)
{
  bool result;
  ThreadState *thr = ev.thr;

  switch (ev.dbg)
  {
     case dbg_stopped:
        thr->setStopped(true);
        if (thr->state() == ps_attached_intermediate && 
            (ev.data.idata == SIGSTOP || ev.data.idata == SIGTRAP)) {
           sw_printf("[%s:%u] - Moving %d/%d to state running\n", 
                     __FILE__, __LINE__, pid, thr->getTid());
           thr->setState(ps_running);
           return true;
        }

#if defined(cap_ptrace_traceclone)
        if (ev.data.idata == (SIGTRAP | (PTRACE_EVENT_CLONE << 8))) {
           sw_printf("[%s:%u] - Discovered new thread in proc %d\n", 
                     __FILE__, __LINE__, pid);
           ThreadState *new_thread = NULL;
           unsigned long newtid_l = 0x0;
           long iresult = ptrace((pt_req) PTRACE_GETEVENTMSG, thr->getTid(), 
                                 NULL, &newtid_l);
           pid_t newtid = (pid_t) newtid_l;
           if (iresult == -1) {
              sw_printf("[%s:%u] - Unexpected error getting new tid on %d\n"
                     __FILE__, __LINE__, pid);
           }
           else 
           {
              sw_printf("[%s:%u] - New thread %ld in proc %d\n",
                        __FILE__, __LINE__, newtid, pid);
              new_thread = ThreadState::createThreadState(this, (THR_ID) newtid, true);
           }
           if (!new_thread) {
              sw_printf("[%s:%u] - Error creating thread %d in proc %d\n",
                        __FILE__, __LINE__, newtid, pid);
           }
           result = debug_continue(thr);
           if (!result) {
              sw_printf("[%s:%u] - Debug continue failed on %d/%d\n",
                        __FILE__, __LINE__, pid, thr->getTid());
              return false;
           }
           return true;
        }
#endif
        
	result = debug_handle_signal(&ev);
	if (!result) {
	  sw_printf("[%s:%u] - Debug continue failed on %d/%d with %d\n", 
		    __FILE__, __LINE__, pid, thr->getTid(), ev.data.idata);
	  return false;
	}

        return true;
    case dbg_libraryload: 
    {
       sw_printf("[%s:%u] - Handling library load event on %d/%d\n",
                 __FILE__, __LINE__, pid, thr->getTid());
       thr->setStopped(true);
       LibraryState *ls = getLibraryTracker();
       assert(ls);
       ls->notifyOfUpdate();

#if defined(arch_power)
       /**
	* Forward over the trap instruction
	**/
       Dyninst::MachRegisterVal newpc = ls->getLibTrapAddress() + 4;
       bool result = setRegValue(Dyninst::ReturnAddr, thr->getTid(), newpc);
       if (!result) {
	 sw_printf("[%s:%u] - Error! Could not set PC past trap!\n",
		   __FILE__, __LINE__);
	 setLastError(err_internal, "Could not set PC after trap\n");
	 return false;
       }
#endif
       result = debug_continue_with(thr, 0);
       if (!result) {
          sw_printf("[%s:%u] - Debug continue failed on %d/%d with %d\n", 
                    __FILE__, __LINE__, pid, thr->getTid(), ev.data.idata);
          return false;
       }
       return true;
    }
    case dbg_crashed:
      sw_printf("[%s:%u] - Handling process crash on %d/%d\n",
                __FILE__, __LINE__, pid, thr->getTid());
    case dbg_exited:
      sw_printf("[%s:%u] - Handling process death on %d/%d\n",
                __FILE__, __LINE__, pid, thr->getTid());
      thr->setState(ps_exited);
      return true;
    case dbg_err:
    case dbg_noevent:
    default:
      sw_printf("[%s:%u] - Unexpectedly handling an error event %d on %d/%d\n", 
                __FILE__, __LINE__, ev.dbg, pid, thr->getTid());
      setLastError(err_internal, "Told to handle an unexpected event.");
      return false;
  }
}

bool ProcDebugLinux::debug_handle_signal(DebugEvent *ev)
{
   assert(ev->dbg == dbg_stopped);
   int sig = ev->data.idata;
   ThreadState *thr = ev->thr;
   if (sigfunc) {
      bool user_stopped = thr->userIsStopped();
      thr->setUserStopped(true);
      sigfunc(sig, thr);
      thr->setUserStopped(user_stopped);
   }

   if (sig == SIGSTOP)
   {
     if (thr->hasPendingStop()) {
       //Leave process as is, no need to handle.
       sw_printf("Clearing pending SIGSTOP\n");
       thr->clearPendingStop();
       return true;
     }
     sw_printf("Dropping SIGSTOP and continuing process\n");
     //Continue past sigstop with-out a signal
     sig = 0;
   }

   return debug_continue_with(thr, sig);
}

#if defined(cap_ptrace_setoptions)   
void ProcDebugLinux::setOptions(Dyninst::THR_ID tid)
{

   long options = 0;
#if defined(cap_ptrace_traceclone)
   options |= PTRACE_O_TRACECLONE;
#endif

   if (options) {
      int result = ptrace((pt_req) PTRACE_SETOPTIONS, tid, NULL, 
                          (void *) options);
      if (result == -1) {
         sw_printf("[%s:%u] - Failed to set options for %d: %s\n", 
                   __FILE__, __LINE__, tid, strerror(errno));
      }
   }   
}
#else
void ProcDebugLinux::setOptions(Dyninst::THR_ID)
{
}
#endif

bool ProcDebugLinux::debug_attach(ThreadState *ts)
{
   long result;
   pid_t tid = (pid_t) ts->getTid();
   
   sw_printf("[%s:%u] - Attaching to pid %d\n", __FILE__, __LINE__, tid);
   result = ptrace((pt_req) PTRACE_ATTACH, tid, NULL, NULL);
   if (result != 0) {
      int errnum = errno;
      sw_printf("[%s:%u] - Unable to attach to process %d: %s\n",
                __FILE__, __LINE__, tid, strerror(errnum));
      if (errnum == EPERM)
         setLastError(err_prem, "Do not have correct premissions to attach " \
                      "to pid");
      else if (errnum == ESRCH)
         setLastError(err_noproc, "The specified process was not found");
      else {
         setLastError(err_internal, "DynStackwalker was unable to attach to " \
                      "the specified process");
      }
      return false;
   }
   ts->setState(ps_attached_intermediate);   

   return true;
}

bool ProcDebugLinux::debug_post_create()
{
   sw_printf("[%s:%u] - Post create on %d\n", __FILE__, __LINE__, pid);
   setOptions(pid);

   setDefaultLibraryTracker();
   assert(library_tracker);
   registerLibSpotter();
   return true;
}

bool ProcDebugLinux::debug_post_attach(ThreadState *thr)
{
   sw_printf("[%s:%u] - Post attach on %d\n", __FILE__, __LINE__, thr->getTid());
   THR_ID tid = thr->getTid();
   setOptions(tid);

   setDefaultLibraryTracker();
   assert(library_tracker);
   registerLibSpotter();

   if (tid == pid) {
      //We're attach to the initial process, also attach to all threads
      pollForNewThreads();
   }
   return true;
}

bool ProcDebugLinux::debug_pause(ThreadState *thr)
{
   bool result;

   result = t_kill(thr->getTid(), SIGSTOP);
   if (!result) {
      if (errno == ESRCH) {
         sw_printf("[%s:%u] - t_kill failed on %d, thread doesn't exist\n",
                   __FILE__, __LINE__, thr->getTid());
         setLastError(err_noproc, "Thread no longer exists");
         return false;
      }
      sw_printf("[%s:%u] - t_kill failed on %d: %s\n", __FILE__, __LINE__,
                thr->getTid(), strerror(errno));
      setLastError(err_internal, "Could not send signal to process while " \
                   "stopping");
      return false;
   }
   thr->markPendingStop();
   
   return true;
}

bool ProcDebugLinux::debug_continue(ThreadState *thr)
{
   return debug_continue_with(thr, 0);
}

bool ProcDebugLinux::debug_continue_with(ThreadState *thr, long sig)
{
   long result;
   assert(thr->isStopped());
   Dyninst::THR_ID tid = thr->getTid();
   sw_printf("[%s:%u] - Calling PTRACE_CONT with signal %d on %d\n",
             __FILE__, __LINE__, sig, tid);
   result = ptrace((pt_req) PTRACE_CONT, tid, NULL, (void *) sig);
   if (result != 0)
   {
     int errnum = errno;
      sw_printf("[%s:%u] - Error continuing %d with %d: %s\n",
                __FILE__, __LINE__, tid, sig, strerror(errnum));
      setLastError(err_internal, "Could not continue process");
      return false;
   }

   thr->setStopped(false);
   return true;
}

bool ProcDebugLinux::readMem(void *dest, Address source, size_t size)
{
   unsigned long nbytes = size;
   const unsigned char *ap = (const unsigned char*) source;
   unsigned char *dp = (unsigned char*) dest;
   Address w = 0x0;               /* ptrace I/O buffer */
   int len = sizeof(long);
   unsigned long cnt;
   
   if (!nbytes) {
      return true;
   }
   
   ThreadState *thr = active_thread;
   if (!thr) {
      //Likely doing this read in response to some event, perhaps
      // without an active thread.  The event should have stopped something,
      // so find a stopped thread.
      thread_map_t::iterator i;
      for (i = threads.begin(); i != threads.end(); i++) {
         ThreadState *t = (*i).second;
         if (t->state() == ps_exited)
            continue;
         if (t->isStopped()) {
            thr = t;
            break;
         }
      }
   }
   assert(thr); //Something should be stopped if we're here.
   pid_t tid = (pid_t) thr->getTid();

   if ((cnt = (source % len))) {
      /* Start of request is not aligned. */
      unsigned char *p = (unsigned char*) &w;
      
      /* Read the segment containing the unaligned portion, and
         copy what was requested to DP. */
      errno = 0;
      w = ptrace((pt_req) PTRACE_PEEKTEXT, tid, (Address) (ap-cnt), w);
      if (errno) {
         int errnum = errno;
         sw_printf("[%s:%u] - PTRACE_PEEKTEXT returned error on %d: %s\n",
                   __FILE__, __LINE__, pid, strerror(errnum));
         setLastError(err_procread, "Could not read from process\n");
         return false;
      }

      for (unsigned i = 0; i < len-cnt && i < nbytes; i++)
         dp[i] = p[cnt+i];
      
      if (len-cnt >= nbytes) {
         return true;
      }
      
      dp += len-cnt;
      ap += len-cnt;
      nbytes -= len-cnt;
   }
   /* Copy aligned portion */
   while (nbytes >= (u_int)len) {
      errno = 0;
      w = ptrace((pt_req) PTRACE_PEEKTEXT, tid, (Address) ap, 0);
      if (errno) {
         int errnum = errno;
         sw_printf("[%s:%u] - PTRACE_PEEKTEXT returned error on %d: %s\n",
                   __FILE__, __LINE__, pid, strerror(errnum));
         setLastError(err_procread, "Could not read from process\n");
         return false;
      }
      memcpy(dp, &w, len);
      dp += len;
      ap += len;
      nbytes -= len;
   }
   
   if (nbytes > 0) {
      /* Some unaligned data remains */
      unsigned char *p = (unsigned char *) &w;
      
      /* Read the segment containing the unaligned portion, and
         copy what was requested to DP. */
      errno = 0;
      w = ptrace((pt_req) PTRACE_PEEKTEXT, tid, (Address) ap, 0);
      if (errno) {
         int errnum = errno;
         sw_printf("[%s:%u] - PTRACE_PEEKTEXT returned error on %d: %s\n",
                   __FILE__, __LINE__, pid, strerror(errnum));
         setLastError(err_procread, "Could not read from process\n");
         return false;
      }
      for (unsigned i = 0; i < nbytes; i++)
         dp[i] = p[i];
   }

   return true;
}   

bool ProcDebugLinux::getThreadIds(std::vector<THR_ID> &thrds)
{
#if !defined(cap_ptrace_traceclone)
   pollForNewThreads();
#endif
   thread_map_t::iterator i;
   for (i = threads.begin(); i != threads.end(); i++) {
      ThreadState *ts = (*i).second;
      if (ts->state() != ps_running) {
         sw_printf("Skipping thread %d in state %d\n", 
                   ts->getTid(), ts->state());
         continue;
      }
              
      thrds.push_back(ts->getTid());
   }
   return true;
}

bool ProcDebugLinux::pollForNewThreads()
{
   std::vector<THR_ID> thrds;
   bool result = findProcLWPs(pid, thrds);
   if (!result) {
      sw_printf("[%s:%u] - getThreadIds failed in libcommon's findProcLWPs "
                "for %d", __FILE__, __LINE__, pid);                
      return false;
   }

   return add_new_threads(thrds.begin(), thrds.end());
}

bool ProcDebugLinux::getDefaultThread(THR_ID &default_tid)
{
   default_tid = (THR_ID) pid;
   return true;
}

ProcDebugLinux::ProcDebugLinux(PID pid)
   : ProcDebug(pid),
     cached_addr_width(0),
     lib_load_trap(0x0),
     trap_actual_len(0x0),
     trap_install_error(false)
{
}

ProcDebugLinux::ProcDebugLinux(std::string executable, 
                               const std::vector<std::string> &argv)
   : ProcDebug(executable, argv),
     cached_addr_width(0),
     lib_load_trap(0x0),
     trap_actual_len(0x0),
     trap_install_error(false)
{
}

ProcDebugLinux::~ProcDebugLinux()
{
}
   
ProcDebug *ProcDebug::newProcDebug(PID pid, std::string)
{
   ProcDebugLinux *pd = new ProcDebugLinux(pid);
   if (!pd)
   {
      sw_printf("[%s:%u] - Error creating new ProcDebug object\n",
                __FILE__, __LINE__);
      return pd;
   }

   bool result = pd->attach();
   if (!result || pd->state() != ps_running) {
     pd->setState(ps_errorstate);
     proc_map.erase(pid);
     sw_printf("[%s:%u] - Error attaching to process %d\n",
               __FILE__, __LINE__, pid);
     delete pd;
     return NULL;
   }

   return pd;
}

ProcDebug *ProcDebug::newProcDebug(std::string executable, 
                                   const std::vector<std::string> &argv)
{
   ProcDebugLinux *pd = new ProcDebugLinux(executable, argv);
   if (!pd)
   {
      sw_printf("[%s:%u] - Error creating new ProcDebug object\n",
                __FILE__, __LINE__);
      return NULL;
   }

   bool result = pd->create(executable, argv);
   if (!result || pd->state() != ps_running)
   {
     pd->setState(ps_errorstate);
     proc_map.erase(pd->pid);
     sw_printf("[%s:%u] - Error attaching to process %d\n",
               __FILE__, __LINE__, pd->pid);
     delete pd;
     return NULL;
   }

   return pd;   
}

Dyninst::Architecture ProcDebug::getArchitecture()
{
#if defined(arch_power) && !defined(arch_64bit)
   return Dyninst::Arch_ppc32;
#elif defined(arch_x86)
   return Dyninst::Arch_x86;
#else
   int addr_width = getAddressWidth();
   if (addr_width == 4)
   {
#if defined(arch_x86_64)
      return Dyninst::Arch_x86;
#elif defined(arch_power)
      return Dyninst::Arch_ppc32;
#endif
   }
   else if (addr_width == 8)
   {
#if defined(arch_x86_64)
      return Dyninst::Arch_x86_64;
#elif defined(arch_power)
      return Dyninst::Arch_ppc64;
#endif
   }
#endif
   assert(0);
   return Dyninst::Arch_none;
}


void chld_handler(int)
{
   write(ProcDebug::pipe_in, "s", 1);
}

int ProcDebug::getNotificationFD()
{
   static bool registered_handler = false;
   int filedes[2], result;
   if (!registered_handler) 
   {
      signal(SIGCHLD, chld_handler);
      result = pipe(filedes);
      if (result == -1)
      {
         int errnum = errno;
         sw_printf("[%s:%u] - Could not create pipe: %s\n",
                   __FILE__, __LINE__, strerror(errnum));
         setLastError(err_internal, "Could not create pipe for notificationFD");
         return -1;
      }
      pipe_out = filedes[0];
      pipe_in = filedes[1];

      result = fcntl(pipe_out, F_GETFL);
      if (result != -1)
      {
         result = fcntl(pipe_out, F_SETFL, result | O_NONBLOCK);
      }
      if (result == -1)
      {
         int errnum = errno;
         sw_printf("[%s:%u] - Could not set fcntl flags: %s\n",
                   __FILE__, __LINE__, strerror(errnum));
         setLastError(err_internal, "Could not set pipe properties");
         return -1;
      }
      registered_handler = true;
   }
   return pipe_out;
}

bool ProcDebugLinux::debug_create(std::string executable, 
                                  const std::vector<std::string> &argv)
{
   pid = fork();
   if (pid == -1)
   {
      int errnum = errno;
      sw_printf("[%s:%u] - Could not fork new process for %s: %s\n",
                __FILE__, __LINE__, executable.c_str(), strerror(errnum));
      setLastError(err_internal, "Unable to fork new process");
      return false;
   }

   if (pid)
   {
   }
   else
   {
      //Child
      long int result = ptrace((pt_req) PTRACE_TRACEME, 0, 0, 0);
      unsigned i;
      if (result == -1)
      {
         sw_printf("[%s:%u] - Failed to execute a PTRACE_TRACME.  Odd.\n",
                   __FILE__, __LINE__);
         setLastError(err_internal, "Unable to debug trace new process");
         exit(-1);
      }

      typedef const char * const_str;
      
      const_str *new_argv = (const_str *) calloc(argv.size()+3, sizeof(char *));
      new_argv[0] = executable.c_str();
      for (i=1; i<argv.size()+1; i++) {
         new_argv[i] = argv[i-1].c_str();
      }
      new_argv[i+1] = (char *) NULL;
      
      result = execv(executable.c_str(), const_cast<char * const*>(new_argv));
      int errnum = errno;         
      sw_printf("[%s:%u] - Failed to exec %s: %s\n", __FILE__, __LINE__, 
                executable.c_str(), strerror(errnum));
      if (errnum == ENOENT)
         setLastError(err_nofile, "No such file");
      if (errnum == EPERM || errnum == EACCES)
         setLastError(err_prem, "Premission denied");
      else
         setLastError(err_internal, "Unable to exec process");
      exit(-1);
   }
   return true;
}

vsys_info *Dyninst::Stackwalker::getVsysInfo(ProcessState *ps)
{
#if defined(arch_x86_64)
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
   sw_printf("[%s:%u] - Registering signal handler stepper over range %lx to %lx\n",
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

void ProcDebugLinux::registerLibSpotter()
{
   bool result;

   if (lib_load_trap)
      return;

   LibraryState *libs = getLibraryTracker();
   if (!libs) {
      sw_printf("[%s:%u] - Not using lib tracker, don't know how "
                "to get library load address\n", __FILE__, __LINE__);
      return;
   }
   
   lib_load_trap = libs->getLibTrapAddress();
   if (!lib_load_trap) {
      sw_printf("[%s:%u] - Couldn't get trap addr, couldn't set up "
                "library loading notification.\n", __FILE__, __LINE__);
      trap_install_error = true;
      return;
   }

   char trap_buffer[MAX_TRAP_LEN];
   getTrapInstruction(trap_buffer, MAX_TRAP_LEN, trap_actual_len, true);

   result = PtraceBulkRead(lib_load_trap, trap_actual_len, trap_overwrite_buffer, pid);
   if (!result) {
      sw_printf("[%s:%u] - Error reading trap bytes from %lx\n", 
                __FILE__, __LINE__, lib_load_trap);
      trap_install_error = true;
      return;
   }
   result = PtraceBulkWrite(lib_load_trap, trap_actual_len, trap_buffer, pid);
   if (!result) {
      sw_printf("[%s:%u] - Error writing trap to %lx, couldn't set up library "
                "load address\n", __FILE__, __LINE__, lib_load_trap);
      trap_install_error = true;
      return;
   }
   sw_printf("[%s:%u] - Successfully installed library trap at %lx\n",
             __FILE__, __LINE__, lib_load_trap);
}

bool ProcDebugLinux::detach_thread(int tid, bool leave_stopped)
{
   sw_printf("[%s:%u] - Detaching from tid %d\n", __FILE__, __LINE__, tid);
   long int iresult = ptrace((pt_req) PTRACE_DETACH, tid, NULL, NULL);
   if (iresult == -1) {
      int error = errno;
      sw_printf("[%s:%u] - Error.  Couldn't detach from %d: %s\n",
                __FILE__, __LINE__, tid, strerror(error));
      if (error != ESRCH) {
         setLastError(err_internal, "Could not detach from thread\n");
         return false;
      }
   }

   thread_map_t::iterator j = all_threads.find(tid);
   if (j == all_threads.end()) {
      sw_printf("[%s:%u] - Error.  Expected to find %d in all threads\n",
                __FILE__, __LINE__, tid);
      setLastError(err_internal, "Couldn't find thread in internal data structures");
      return false;
   }

   if (!leave_stopped) {
      t_kill(tid, SIGCONT);
   }

   all_threads.erase(j);
   return true;
}

bool ProcDebugLinux::detach(bool leave_stopped)
{
   bool result;
   bool error = false;
   sw_printf("[%s:%u] - Detaching from process %d\n", 
             __FILE__, __LINE__, getProcessId());
   result = pause();
   if (!result) {
      sw_printf("[%s:%u] - Error pausing process before detach\n",
                __FILE__, __LINE__);
      return false;
   }

   if (lib_load_trap && !trap_install_error)
   {
      result = PtraceBulkWrite(lib_load_trap, trap_actual_len, 
                               trap_overwrite_buffer, pid);
      if (!result) {
         sw_printf("[%s:%u] - Error.  Couldn't restore load trap bytes at %lx\n",
                   __FILE__, __LINE__, lib_load_trap);
         setLastError(err_internal, "Could not remove library trap");
         return false;
      }
      lib_load_trap = 0x0;
   }
   
   for (thread_map_t::iterator i = threads.begin(); i != threads.end(); i++)
   {
      Dyninst::THR_ID tid = (*i).first;
      ThreadState *thread_state = (*i).second;
      if (tid == getProcessId())
         continue;
      result = detach_thread(tid, leave_stopped);
      if (!result)
         error = true;

      delete thread_state;
   }
   threads.clear();

   result = detach_thread(getProcessId(), leave_stopped);
   if (!result)
      error = true;

   detach_arch_cleanup();

   return !error;
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
   map_entries *maps = getLinuxMaps(getpid(), maps_size);
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

bool TrackLibState::updateLibsArch()
{
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
   SymReader *reader = fact->openSymbolReader((char *) vsys->vsys_mem, vsys->end - vsys->start);
   if (reader)
      LibraryWrapper::registerLibrary(reader, vsyscall_page.first);

   std::pair<LibAddrPair, unsigned int> vsyscall_lib_pair;
   vsyscall_lib_pair.first = vsyscall_page;
   vsyscall_lib_pair.second = static_cast<unsigned int>(vsys->end - vsys->start);
   arch_libs.push_back(vsyscall_lib_pair);

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

ThreadState* ThreadState::createThreadState(ProcDebug *parent,
                                            Dyninst::THR_ID id,
                                            bool already_attached)
{
   assert(parent);
   Dyninst::THR_ID tid = id;
   if (id == NULL_THR_ID) {
      tid = (Dyninst::THR_ID) parent->getProcessId();
   }
   else {
      tid = id;
   }
   
   ThreadState *newts = new ThreadState(parent, tid);
   sw_printf("[%s:%u] - Creating new ThreadState %p for %d/%d\n",
             __FILE__, __LINE__, newts, parent->getProcessId(), tid);
   if (!newts || newts->state() == ps_errorstate) {
      sw_printf("[%s:%u] - Error creating new thread\n",
                __FILE__, __LINE__);
      return NULL;
   }
   if (already_attached) {
      newts->setState(ps_attached_intermediate);
   }

   std::map<pid_t, int>::iterator previous_event;
   previous_event = ProcDebugLinux::unknown_pid_events.find(tid);
   if (previous_event != ProcDebugLinux::unknown_pid_events.end()) {
      int status = (*previous_event).second;
      sw_printf("[%s:%u] - Matched new thread %d with old events with statis %lx\n",
                __FILE__, __LINE__, tid, status);
      if (WIFSTOPPED(status) && WSTOPSIG(status) == SIGSTOP) {
         newts->setState(ps_running);
      }
      ProcDebugLinux::unknown_pid_events.erase(previous_event);
   }

   ProcDebug::thread_map_t::iterator i = ProcDebugLinux::all_threads.find(tid);
   assert(i == ProcDebugLinux::all_threads.end() || 
          (*i).second->state() == ps_exited);
   ProcDebugLinux::all_threads[tid] = newts;
   parent->threads[tid] = newts;

   if (id == NULL_THR_ID) {
      //Done with work for initial thread
      return newts;
   }

   bool result;

   if (newts->state() == ps_neonatal) {
      result = parent->debug_attach(newts);
      if (!result && getLastError() == err_procexit) {
         sw_printf("[%s:%u] - Thread %d exited before attach\n", 
                   __FILE__, __LINE__, tid);
         newts->setState(ps_exited);
         return NULL;
      }
      else if (!result) {
         sw_printf("[%s:%u] - Unknown error attaching to thread %d\n",
                   __FILE__, __LINE__, tid);
         newts->setState(ps_errorstate);
         return NULL;
      }
   }
   result = parent->debug_waitfor_attach(newts);
   if (!result) {
      sw_printf("[%s:%u] - Error waiting for attach on %d\n", 
                __FILE__, __LINE__, tid);
      return NULL;
   }

   result = parent->debug_post_attach(newts);
   if (!result) {
      sw_printf("[%s:%u] - Error in post attach on %d\n",
                __FILE__, __LINE__, tid);
      return NULL;
   }

   result = parent->resume_thread(newts);
   if (!result) {
      sw_printf("[%s:%u] - Error resuming thread %d\n",
                __FILE__, __LINE__, tid);
   }
   
   return newts;
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
            ra_stack_tops.push_back(std::pair<Address, Address>(start, end));
         }
         startthread_sym = libthread->getSymbolByName("start_thread");
         if (libthread->isValidSymbol(startthread_sym)) {
            Dyninst::Address start = libthread->getSymbolOffset(startthread_sym) + 
               libthread_addr.second;
            Dyninst::Address end = libthread->getSymbolSize(startthread_sym) + start;
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

