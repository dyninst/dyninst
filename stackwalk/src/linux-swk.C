/*
 * Copyright (c) 1996-2007 Barton P. Miller
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include "stackwalk/h/swk_errors.h"
#include "stackwalk/h/symlookup.h"
#include "stackwalk/h/walker.h"

#include "stackwalk/h/procstate.h"
#include "stackwalk/src/linux-swk.h"

#include "common/h/linuxKludges.h"
#include "common/h/parseauxv.h"

#include <string>

#include <string.h>
#include <sys/syscall.h>
#include <unistd.h>
#include <errno.h>
#include <assert.h>
#include <signal.h>
#include <sys/ptrace.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>

using namespace Dyninst;
using namespace Dyninst::Stackwalker;

SymbolLookup *Walker::createDefaultSymLookup(const std::string &exec)
{
   SymbolLookup *new_lookup = new SwkDynSymtab(this, exec);
   return new_lookup;
}

bool SymbolLookup::lookupLibrary(Address addr, Address &load_addr,
                                 std::string &out_library)
{
  PID pid = getProcessState()->getProcessId();
  unsigned maps_size;
  bool result;

  map_entries *maps = getLinuxMaps((int) pid, maps_size);
  if (!maps) {
    sw_printf("[%s:%u] - Unable to look up maps entries for pid %d\n",
	       __FILE__, __LINE__, pid);
    setLastError(err_procread, "Unable to read /proc/PID/maps for a process");
    result = false;
    goto done;
  }

  unsigned i;
  signed j;
  for (i=0; i<maps_size; i++) {
    if (addr >= maps[i].start && addr < maps[i].end) {
      out_library = maps[i].path;
      load_addr = maps[i].start;
      result = true;
      sw_printf("[%s:%u] - Found library %s for addr %x\n",
		 __FILE__, __LINE__, maps[i].path, addr);
      goto done;
    }
  }

  for (j = i-1; j >= 0; j--)
  {
    if (maps[i].inode == maps[j].inode)
      load_addr = maps[j].start;
  }
  sw_printf("[%s:%u] - Unable find maps entries for pid %d\n", 
	     __FILE__, __LINE__, pid);
  setLastError(err_nosymbol, "Unable to find containing library for at address\n");
  result = false;

 done:
  if (maps)
    free(maps);
  maps = NULL;
  return result;
}

#ifndef SYS_tkill
#define SYS_tkill 238
#endif

static int P_gettid()
{
  static int gettid_not_valid = 0;
  int result;

  if (gettid_not_valid)
    return getpid();

  result = syscall((long int) SYS_gettid);
  if (result == -1 && errno == ENOSYS)
  {
    gettid_not_valid = 1;
    return getpid();
  }
  return result;  
}

static bool t_kill(int pid, int sig)
{
  static bool has_tkill = true;
  int result = 0;
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

ProcSelf::ProcSelf()
{
  mypid = getpid();
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

DebugEvent ProcDebug::debug_get_event(bool block)
{
   int status = 0;
   DebugEvent ev;
   pid_t p;

   sw_printf("[%s:%u] - Calling waitpid\n",__FILE__, __LINE__);
   int flags = __WALL;
   if (!block)
     flags |= WNOHANG;
   p = waitpid(-1, &status, flags);
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
   
   if (p == 0) {
     sw_printf("[%s:%u] - No debug events available\n", __FILE__, __LINE__);
     ev.dbg = dbg_noevent;
     return ev;
   }

   if (pipe_out != -1)
   {
      /*      struct pollfd[1];
      pollfd[0].fd = pipe_out;
      pollfd[0].events = POLLIN;
      pollfd[0].revents = 0;
      poll(pollfd, 1, 0);*/
      char c;
      read(pipe_out, &c, 1);
   }
   
   std::map<PID, ProcDebug *, procdebug_ltint>::iterator i = proc_map.find(p);
   if (i == proc_map.end())
   {
      sw_printf("[%s:%u] - Error, recieved unknown pid %d from waitpid",
                __FILE__, __LINE__, p);
      setLastError(err_internal, "Error calling waitpid");
      ev.dbg = dbg_err;
      return ev;
   }
   ev.proc = i->second;

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

  switch (ev.dbg)
  {
     case dbg_stopped:
        isRunning = false;
        
        if (state == ps_neonatal && ev.data.idata == SIGSTOP) {
           sw_printf("[%s:%u] - Moving %d to state running\n", 
                     __FILE__, __LINE__, pid);
           state = ps_running;
           return true;
        }
        if (state == ps_neonatal && ev.data.idata == SIGTRAP) {
           sw_printf("[%s:%u] - Moving %d to state running\n", 
                     __FILE__, __LINE__, pid);
           state = ps_running;
           return true;
        }
        
        if (ev.data.idata != SIGSTOP) {
           result = debug_continue_with(ev.data.idata);
           if (!result) {
              sw_printf("[%s:%u] - Debug continue failed on %d with %d\n", 
                        __FILE__, __LINE__, pid, ev.data.idata);
              return false;
           }
        }
        return true;
    case dbg_crashed:
      sw_printf("[%s:%u] - Handling process crash on %d\n",
                __FILE__, __LINE__, pid);
    case dbg_exited:
      sw_printf("[%s:%u] - Handling process death on %d\n",
                __FILE__, __LINE__, pid);
      state = ps_exited;
      return true;
    case dbg_err:
    case dbg_noevent:
    default:
      sw_printf("[%s:%u] - Unexpectedly handling an error event %d on %d\n", 
                __FILE__, __LINE__, ev.dbg, pid);
      setLastError(err_internal, "Told to handle an unexpected event.");
      return false;
  }
}

bool ProcDebugLinux::debug_handle_signal(DebugEvent *ev)
{
   assert(ev->dbg == dbg_stopped);
   return debug_continue_with(ev->data.idata);
}

bool ProcDebugLinux::debug_attach()
{
   long result;
   
   sw_printf("[%s:%u] - Attaching to pid\n", __FILE__, __LINE__, pid);
   result = ptrace(PTRACE_ATTACH, pid, NULL, NULL);
   if (result != 0) {
      int errnum = errno;
      sw_printf("[%s:%u] - Unable to attach to process %d: %s\n",
                __FILE__, __LINE__, pid, strerror(errnum));
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
   return true;
}

bool ProcDebugLinux::debug_pause() 
{
   bool result;

   result = t_kill(pid, SIGSTOP);
   if (result != 0) {
      sw_printf("[%s:%u] - t_kill failed on %d: %s\n", __FILE__, __LINE__,
                pid, strerror(errno));
      setLastError(err_internal, "Could not send signal to process while " \
                   "stopping");
      return false;
   }
   
   return true;
}

bool ProcDebugLinux::debug_continue()
{
   return debug_continue_with(0);
}

bool ProcDebugLinux::debug_continue_with(long sig)
{
   long result;
   assert(!isRunning);

   result = ptrace(PTRACE_CONT, pid, NULL, (void *) sig);
   if (result != 0)
   {
     int errnum = errno;
      sw_printf("[%s:%u] - Error continuing %d with %d: %s\n",
                __FILE__, __LINE__, pid, sig, strerror(errnum));
      setLastError(err_internal, "Could not continue process");
      return false;
   }

   isRunning = true;
   return true;
}

bool ProcDebugLinux::readMem(void *dest, Address source, size_t size)
{
   unsigned int nbytes = size;
   const unsigned char *ap = (const unsigned char*) source;
   unsigned char *dp = (unsigned char*) dest;
   Address w = 0x0;               /* ptrace I/O buffer */
   int len = sizeof(long);
   unsigned cnt;
   
   if (!nbytes) {
      return true;
   }
   
   if ((cnt = ((Address)ap) % len)) {
      /* Start of request is not aligned. */
      unsigned char *p = (unsigned char*) &w;
      
      /* Read the segment containing the unaligned portion, and
         copy what was requested to DP. */
      errno = 0;
      w = ptrace(PTRACE_PEEKTEXT, pid, (Address) (ap-cnt), w);
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
      w = ptrace(PTRACE_PEEKTEXT, pid, (Address) ap, 0);
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
      w = ptrace(PTRACE_PEEKTEXT, pid, (Address) ap, 0);
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

bool ProcDebugLinux::getThreadIds(std::vector<THR_ID> &threads)
{
   threads.clear();
   threads.push_back((THR_ID) pid);
   return true;
}

bool ProcDebugLinux::getDefaultThread(THR_ID &default_tid)
{
   default_tid = (THR_ID) pid;
   return true;
}

ProcDebugLinux::ProcDebugLinux(PID pid)
   : ProcDebug(pid),
     cached_addr_width(0)
{
}

ProcDebugLinux::ProcDebugLinux(const std::string &executable, 
                               const std::vector<std::string> &argv)
   : ProcDebug(executable, argv),
     cached_addr_width(0)
{
}

ProcDebugLinux::~ProcDebugLinux()
{
}
   
ProcDebug *ProcDebug::newProcDebug(PID pid)
{
   ProcDebugLinux *pd = new ProcDebugLinux(pid);
   if (!pd)
   {
      sw_printf("[%s:%u] - Error creating new ProcDebug object\n",
                __FILE__, __LINE__);
      return pd;
   }

   bool result = pd->attach();
   if (!result || pd->state != ps_running) {
     pd->state = ps_errorstate;
     proc_map.erase(pid);
     sw_printf("[%s:%u] - Error attaching to process %d\n",
               __FILE__, __LINE__, pid);
     delete pd;
     return NULL;
   }

   return pd;
}

ProcDebug *ProcDebug::newProcDebug(const std::string &executable, 
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
   if (!result || pd->state != ps_running)
   {
     pd->state = ps_errorstate;
     proc_map.erase(pd->pid);
     sw_printf("[%s:%u] - Error attaching to process %d\n",
               __FILE__, __LINE__, pd->pid);
     delete pd;
     return NULL;
   }

   return pd;   
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

bool ProcDebugLinux::debug_create(const std::string &executable, 
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
      //Parent
   }
   else
   {
      //Child
      int result = ptrace(PTRACE_TRACEME, 0, 0, 0);
      unsigned i;
      if (result == -1)
      {
         sw_printf("[%s:%u] - Failed to execute a PTRACE_TRACME.  Odd.\n",
                   __FILE__, __LINE__);
         setLastError(err_internal, "Unable to debug trace new process");
         exit(-1);
      }

      typedef const char * const_str;
      const_str *new_argv = (const_str *) malloc((argv.size()+2) * sizeof(char *));
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
