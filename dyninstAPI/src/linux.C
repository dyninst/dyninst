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

// $Id: linux.C,v 1.279 2008/09/03 06:08:44 jaw Exp $

#include <fstream>
#include <string>

#include "dyninstAPI/src/process.h"
#include "dyninstAPI/src/dyn_thread.h"
#include "dyninstAPI/src/dyn_lwp.h"

#include <sys/ptrace.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <signal.h>
#include <dlfcn.h>
#include <sys/user.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <dirent.h>
#include <sys/resource.h>
#include <math.h> // for floor()
#include <unistd.h>
#include <string>

#include "dyninstAPI/src/binaryEdit.h"
#include "dyninstAPI/src/symtab.h"
#include "dyninstAPI/src/function.h"
#include "dyninstAPI/src/instPoint.h"
#include "dyninstAPI/src/baseTramp.h"
#include "dyninstAPI/src/signalhandler.h"
#include "dyninstAPI/src/signalgenerator.h"
#include "dyninstAPI/src/eventgate.h"
#include "dyninstAPI/src/mailbox.h"
#include "dyninstAPI/src/debuggerinterface.h"
#include "common/h/headers.h"
#include "common/h/linuxKludges.h"
#include "dyninstAPI/src/os.h"
#include "common/h/stats.h"
#include "common/h/Types.h"
#include "dyninstAPI/src/debug.h"
#include "dyninstAPI/src/util.h" // getCurrWallTime
#include "common/h/pathName.h"
#include "mapped_object.h"
#include "mapped_module.h"
#include "linux.h"
#include <libelf.h>
#include "ast.h" // instrumentation for MT

#include "dynamiclinking.h"

#if defined (cap_save_the_world)
#include "dyninstAPI/src/addLibraryLinux.h"
#include "dyninstAPI/src/writeBackElf.h"
// #include "saveSharedLibrary.h" 
#endif
#include "symtabAPI/h/Symtab.h"
using namespace Dyninst::SymtabAPI;

//TODO: Remove the writeBack functions and get rid of this include
#ifdef PAPI
#include "papi.h"
#endif

//These values can be different on different platforms, and don't seem to be
//consistently defined in the system's include files.
#if !defined(PTRACE_SETOPTIONS)
#if defined(arch_x86)
#define PTRACE_SETOPTIONS 21
#endif
#endif

#if !defined(PT_TRACE_EXIT)
#define PTRACE_O_TRACEEXIT 0x40
#endif

#if defined(PTRACEDEBUG) && !defined(PTRACEDEBUG_ALWAYS)
static bool debug_ptrace = false;
#endif

WaitpidMux SignalGenerator::waitpid_mux;

/* ********************************************************************** */

void printStackWalk( process *p ) {
  dyn_lwp *lwp_to_use = NULL;
  if(process::IndependentLwpControl() && p->getRepresentativeLWP() ==NULL)
     lwp_to_use = p->getInitialThread()->get_lwp();
  else
     lwp_to_use = p->getRepresentativeLWP();

  Frame theFrame = lwp_to_use->getActiveFrame();
  while (true) {
    // do we have a match?
    const Address framePC = theFrame.getPC();
    inferiorrpc_cerr << "stack frame pc @ " << (void*)framePC << endl;
    
    if (theFrame.isLastFrame())
      // well, we've gone as far as we can, with no match.
      break;
    
    // else, backtrace 1 more level
    theFrame = theFrame.getCallerFrame();
  }
}
 
// already setup on this FD.
// disconnect from controlling terminal 
void OS::osDisconnect(void) 
{
  int ttyfd = open ("/dev/tty", O_RDONLY);
  ioctl (ttyfd, TIOCNOTTY, NULL);
  P_close (ttyfd);
}


void OS::osTraceMe(void) 
{ 
   int result = P_ptrace(PTRACE_TRACEME, 0, 0, 0);
   if (result == -1) {
      perror("Couldn't ptrace(PTRACE_TRACEME)\n");
   }
}

bool SignalGenerator::attachToChild(int pid)
{
   //  wait for child process to stop itself, attach to it, and continue
   int status = 0, res;
   
   waitpid_mux.registerProcess(this);
   
   /**
    * Recieve the SIGSTOP event associated with a PTRACE_TRACEME/exec
    * pair here.  We'll then add it back to the signalgenerator so that it
    * can be found again latter when we're waiting for a process create event.
    **/
   do {
      res = waitpid_mux.waitpid(this, &status);
      if ((res == -1 && errno == EINTR) || (res == 0))
         continue; //waitpid returned without a value
      if (res == -1) {
         perror("Couldn't successfully waitpid on starting process");
         break;
      }
      if (WIFSIGNALED(status) || WIFEXITED(status)) {
          startup_printf("[%s:%u] - Process terminated during attach\n", 
                 FILE__, __LINE__);
         return false;
      }
      if (WIFSTOPPED(status) && WSTOPSIG(status) == SIGTRAP) {
         startup_printf("[%s:%u] - Recieved notice of successful exec after "
                        "fork.\n", FILE__, __LINE__);
         //Normal case, we got the sigtrap
         break;
      }
      if (WIFSTOPPED(status)) {
         startup_printf("[%s:%u] - Recieved unexpected signal %d, "
                        "throwing back\n", FILE__, __LINE__, WSTOPSIG(status));
         P_ptrace(PTRACE_CONT, pid, (Address) NULL, WSTOPSIG(status));
      }
   } while (!WIFSTOPPED(status) || (WSTOPSIG(status) != SIGTRAP));

   waitpid_ret_pair ev;
   ev.status = status;
   ev.pid = pid;
   waitpid_mux.enqueueWaitpidValue(ev, this);

   return true;
}

bool SignalGenerator::decodeEvents(pdvector<EventRecord> &events)
{
  //  ev.info has the status result from waitpid
    for (unsigned i = 0; i < events.size(); i++) {
        EventRecord &ev = events[i];

        if (ev.type == evtUndefined) {
            if (!decodeWaitPidStatus(ev.info, ev)) 
                fprintf(stderr, "%s[%d][%s]:  failed to decode status for event\n", 
                        FILE__, __LINE__, getThreadStr(getExecThreadID()));
        }
        
        errno = 0;
        if (ev.type == evtSignalled) {
           /* There's a process-wide waiting for stop we need to worry about;
              we could be asking the process to stop but any of the independent
              LWPs to, yet.  (If the SIGSTOP is delayed, by, say, an iRPC completion,
              then we don't know which LWP will receive the STOP.) */
           if( waiting_for_stop || (ev.lwp && ev.lwp->isWaitingForStop()) ) {
              signal_printf("%s[%d]: independentLwpStop_ %d (lwp %d %s), checking for suppression...\n",
                            FILE__, __LINE__,
                            independentLwpStop_,
                            ev.lwp ? ev.lwp->get_lwp_id() : (unsigned)-1,
                            ev.lwp ? (ev.lwp->isWaitingForStop() ? "waiting for stop" : "not waiting for stop") : "no LWP");
              
              if (suppressSignalWhenStopping(ev)) {
                 signal_printf("%s[%d]: suppressing signal... \n", FILE__, __LINE__);
                 //  we suppress this signal, just send a null event
                 ev.type = evtIgnore;
                 signal_printf("%s[%d]: suppressing signal during wait for stop\n", FILE__, __LINE__);
                 return true;
              }
           }
           signal_printf("%s[%d]: decoding signal \n", FILE__, __LINE__);
           decodeSignal(ev);
        }
        
        if (ev.type == evtUndefined) {
            //  if we still have evtSignalled, then it must not be a signal that
            //  we care about internally.  Still, send it along to the handler
            //  to be forwarded back to the process.
            char buf[512];
            fprintf(stderr, "%s[%d]:  got event %s, should have been set by now\n", FILE__, __LINE__, ev.sprint_event(buf));
        }
    }
    
    return true;
}

bool get_linux_version(int &major, int &minor, int &subvers)
{
    int subsub;
    return get_linux_version(major,minor,subvers,subsub); 
}

bool get_linux_version(int &major, int &minor, int &subvers, int &subsubvers)
{
   static int maj = 0, min = 0, sub = 0, subsub = 0;
   int result;
   FILE *f;
   if (maj)
   {
      major = maj;
      minor = min;
      subvers = sub;
      subsubvers = subsub;
      return true;
   }
   f = P_fopen("/proc/version", "r");
   if (!f) goto error;
   result = fscanf(f, "Linux version %d.%d.%d.%d", &major, &minor, &subvers,
                    &subsubvers);
   fclose(f);
   if (result != 3 && result != 4) goto error;

   maj = major;
   min = minor;
   sub = subvers;
   subsub = subsubvers;
   return true;

 error:
   //Assume 2.4, which is the earliest version we support
   major = maj = 2;
   minor = min = 4;
   subvers = sub = 0;
   subsubvers = subsub = 0;
   return false;
}

bool SignalGenerator::add_lwp_to_poll_list(dyn_lwp *lwp)
{
   char filename[64];
   int lwpid, major, minor, sub;
   struct stat buf;

   get_linux_version(major, minor, sub);   
   if ((major == 2 && minor > 4) || (major >= 3))
      return true;
   if (!lwp->proc()->multithread_capable(true))
      return true;

   /**
    * Store pids that can be found as /proc/pid as pid 
    * Store pids that can be found as /proc/.pid as -1*pid
    **/
   lwpid = lwp->get_lwp_id();
   snprintf(filename, 64, "/proc/%d", lwpid);
   if (stat(filename, &buf) == 0)
   {
      attached_lwp_ids.push_back(lwpid);
      return true;
   }

   snprintf(filename, 64, "/proc/.%d", lwpid);
   if (stat(filename, &buf) == 0)
   {
      attached_lwp_ids.push_back(-1 * lwpid);
      return true;
   }
   
   fprintf(stderr, "[%s:%u] - Internal Error.  Could not find new process %d"
           " in /proc area.  Thread deletion callbacks may not work\n", 
           __FILE__, __LINE__, lwpid);
   return false;
}

bool SignalGenerator:: remove_lwp_from_poll_list(int lwp_id)
{
   bool found = false;
   for (int i=attached_lwp_ids.size()-1; i>=0; i--)
   {
      int lcur = abs(attached_lwp_ids[i]);
      if (lcur == lwp_id)
      {
         VECTOR_ERASE(attached_lwp_ids,i, i);
         found = true;
      }
   }
   return found;
}

bool SignalGenerator::exists_dead_lwp()
{
   struct stat buf;
   char filename[64];
   int lwpid;

   for (unsigned i=0; i<attached_lwp_ids.size(); i++)
   {
      lwpid = attached_lwp_ids[i];
      if (lwpid >= 0)
         snprintf(filename, 64, "/proc/%d", lwpid);
      else
         snprintf(filename, 64, "/proc/.%d", -1 * lwpid);
      if (stat(filename, &buf) != 0)
      {
         return true;
      }
   }
   return false;
}

int SignalGenerator::find_dead_lwp()
{
   struct stat buf;
   char filename[64];
   int lwpid;

   for (unsigned i=0; i<attached_lwp_ids.size(); i++)
   {
      lwpid = attached_lwp_ids[i];
      if (lwpid >= 0)
         snprintf(filename, 64, "/proc/%d", lwpid);
      else
         snprintf(filename, 64, "/proc/.%d", -1 * lwpid);
      if (stat(filename, &buf) != 0)
      {
         remove_lwp_from_poll_list(lwpid);
         return lwpid;
      }
   }
   return 0;
}

pid_t SignalGenerator::waitpid_kludge(pid_t /*pid_arg*/, 
                                      int *status, 
                                      int /*options*/, 
                                      int *dead_lwp)
{
  pid_t ret = 0;

  do {
   *dead_lwp = find_dead_lwp();
   if (*dead_lwp) {
       // This is a bad hack.  On Linux 2.4 waitpid doesn't return for dead threads,
       // so we poll for any dead threads before calling waitpid, and if they exist
       // we simulate the result as if waitpid had returned the desired value.
       status = 0;
       ret = *dead_lwp;
       break;
    }

   errno = 0;
   ret = waitpid_mux.waitpid(this, status);
  } while (ret == 0 || (ret == -1 && errno == EINTR));

  if (ret == -1)
     signal_printf("%s[%d]: waitpid_kludge got -1\n", FILE__, __LINE__);
  return ret; 
}

bool SignalGenerator::waitForEventsInternal(pdvector<EventRecord> &events) 
{
  signal_printf("%s[%d]:  welcome to waitNextEventLocked\n", FILE__, __LINE__);

  assert(proc);

  int waitpid_pid = 0;
  int status = 0;
  int dead_lwp = 0;

  //  If we have a rep lwp, the process is not multithreaded, so just wait for 
  //  the pid.  If the process is MT, wait for the process group of the first
  //  mutatee thread (-1 * pgid)

  int pid_to_wait_for = (proc->getRepresentativeLWP() ? getPid() : -1*getpgid(getPid()));

  //  wait for process events, on linux __WALL signifies that both normal children
  //  and cloned children (lwps) should be listened for.
  //  Right now, we are blocking.  To make this nonblocking, or this val with WNOHANG.
  
  //  __WNOTHREAD signifies that children of other threads (other signal handlers)
  //  should not be listened for.

  int wait_options = __WALL;

  waitingForOS_ = true;
  __UNLOCK;
  waitpid_pid = waitpid_kludge( pid_to_wait_for, &status, wait_options, &dead_lwp );
  __LOCK;
  waitingForOS_ = false;
  if (WIFSTOPPED(status))
     proccontrol_printf("waitpid - %d stopped with %d\n", waitpid_pid, 
                        WSTOPSIG(status));

  /* If we were in waitpid() when we detach from/deleted the process, Funky
     Stuff may happen.  You may want to check if this is the case before
     and return false, before doing anything that uses the 'proc' variable. */

  if (waitpid_pid < 0 && errno == ECHILD) {
     /* I am told that this can spontaneously and transiently occur while running under
        Valgrind, and that the proper thing to do is just go back and try again.  I'm
        leaving the fprintf() in because while this condition is thus not always erroneous,
        we should be worried about it when it shows up otherwise. */
     signal_printf("%s[%d]:  waitpid(%d) failed with ECHILD\n", 
                   __FILE__, __LINE__, pid_to_wait_for );
     return false; /* nothing to wait for */
  } else if (waitpid_pid < 0) {
     perror("checkForEventLinux: waitpid failure");
  } else if(waitpid_pid == 0) {
     fprintf(stderr, "%s[%d]:  waitpid \n", __FILE__, __LINE__);
     return false;
  }

  //  If the UI thread wants to initiate a shutdown, it might set a flag and 
  //  then send a SIGTRAP to the mutatee process, so as to wake up the
  //  event handling system.  A shutdown event trumps all others, so we handle it
  //  first.
 
  EventRecord ev;

  if (!proc) {
     //We're exiting
     return false;
  }

  ev.proc = proc;
  ev.lwp = proc->lookupLWP(waitpid_pid);

  if (!ev.lwp || stop_request) {
      // Process was deleted? Run away in any case...
      ev.type = evtShutDown;
      return true;
  }
  
  /* For clarity during debugging, fully initialize the event. */
  ev.type = evtUndefined;
  ev.what = 0;
  ev.status = statusUnknown;
  ev.info = status;
  ev.address = 0;      
  ev.fd = -1;

  ev.proc->set_lwp_status(ev.lwp, stopped);
    
  bool process_exited = WIFEXITED(status) || dead_lwp;
  if ((process_exited) && (waitpid_pid != ev.proc->getPid())) {
     proccontrol_printf("%s[%d]: Received a thread deletion event for %d\n", 
                        FILE__, __LINE__, ev.lwp->get_lwp_id());
     signal_printf("%s[%d]: Received a thread deletion event for %d\n", 
                   FILE__, __LINE__, ev.lwp->get_lwp_id());
     // Thread exited via signal
     ev.type = evtSyscallEntry;      
     ev.what = SYS_lwp_exit;
     decodeSyscall(ev);
  }

  events.push_back(ev);
  return true;
}

void process::independentLwpControlInit() 
{
   if(multithread_capable()) {
      // On linux, if process found to be MT, there will be no
      // representativeLWP since there is no lwp which controls the entire
      // process for MT linux.
      real_lwps[representativeLWP->get_lwp_id()] = representativeLWP;
      representativeLWP = NULL;
   }
}

dyn_lwp *process::createRepresentativeLWP() 
{
   // the initial lwp has a lwp_id with the value of the pid

   // if we identify this linux process as multi-threaded, then later we will
   // adjust this lwp to be identified as a real lwp.
   dyn_lwp *initialLWP = createFictionalLWP(getPid());
   representativeLWP = initialLWP;
   // Though on linux, if process found to be MT, there will be no
   // representativeLWP since there is no lwp which controls the entire
   // process for MT linux.

   return initialLWP;
}

bool process::trapAtEntryPointOfMain(dyn_lwp *trappingLWP, Address)
{
    if (main_brk_addr == 0x0) return false;
    assert(trappingLWP);
    Frame active = trappingLWP->getActiveFrame();
    if (active.getPC() == main_brk_addr ||
        (active.getPC()-1) == main_brk_addr)
        return true;
    else {
      fprintf(stderr, "%s[%d]:  pc =  %lx\n",
            FILE__, __LINE__, active.getPC());
    }
    return false;
}

bool process::trapDueToDyninstLib(dyn_lwp *trappingLWP)
{
    // is the trap instr at dyninstlib_brk_addr?
    if (dyninstlib_brk_addr == 0) return false;
    assert(trappingLWP);
    Frame active = trappingLWP->getActiveFrame();
    if (active.getPC() == dyninstlib_brk_addr ||
        (active.getPC()-1) == dyninstlib_brk_addr)
        return true;
    return false;
}

bool process::setProcessFlags() {
    // None that I'm aware of -- bernat, 24FEB04
    return true;
}

bool process::unsetProcessFlags(){
    // As above
    return true;
}

static int lwp_kill(int pid, int sig)
{
  int result = P_tkill(pid, sig);
  if (result == -1 && errno == ENOSYS)
  {
     result = P_kill(pid, sig);
     proccontrol_printf("%s[%d]: Sent %d to %d via kill\n", FILE__, __LINE__, 
                        sig, pid);
  }
  else if ( result == -1 )
  {
  fprintf( stderr, "%s[%d]: failed to tkill( %d, %d ): ", FILE__, __LINE__, pid, sig );
  perror( "" );
  }
  else
     proccontrol_printf("%s[%d]: Sent %d to %d via tkill\n", FILE__, __LINE__, 
                        sig, pid);

  return result;
}


/**
 * Return the state of the process from /proc/pid/stat.
 * File format is:
 *   pid (executablename) state ...
 * where state is a character.  Returns '\0' on error.
 **/
static char getState(int pid)
{
  char procName[64];
  char sstat[256];
  char *status;
  int paren_level = 1;

  sprintf(procName,"/proc/%d/stat", pid);
  FILE *sfile = P_fopen(procName, "r");
  if (sfile == NULL) return '\0';
  fread( sstat, 1, 256, sfile );
  fclose( sfile );
  sstat[255] = '\0';
  status = sstat;
  
  while (*status != '\0' && *(status++) != '(') ;
  while (*status != '\0' && paren_level != 0)
  {
    if (*status == '(') paren_level++;
    if (*status == ')') paren_level--;
    status++;
  }
  while (*status == ' ') status++;
  return *status;
}
 
bool process::isRunning_() const 
{
  char result = getState(getPid());
//  assert(result != '\0');
  if (result == '\0') {
    return false;
  }
  return (result != 'T');
}

bool dyn_lwp::isRunning() const 
{
  char result = getState(get_lwp_id());
  assert(result != '\0');
  return (result != 'T');
}

bool dyn_lwp::isWaitingForStop() const
{
    signal_printf("%s[%d]: checking LWP %d waiting_for_stop: %d\n",
                  FILE__, __LINE__, get_lwp_id(), waiting_for_stop);
   return waiting_for_stop;
}

bool SignalGenerator::suppressSignalWhenStopping(EventRecord &ev)
{
 
  if ( ev.what == SIGSTOP )
     return false;

  /* If we're suppressing signals, regenerate them at the end of the
     queue to avoid race conditions. */
  if( ev.what == SIGTRAP ) {
#if defined(arch_x86) || defined(arch_x86_64)
     Address trap_pc = ev.lwp->getActiveFrame().getPC();
     ev.lwp->changePC(trap_pc - 1, NULL);
#endif 
  }

  ev.lwp->continueLWP_(0, true);
  ev.proc->set_lwp_status(ev.lwp, running);

  if ( ev.what == SIGILL  || 
       //ev.what == SIGTRAP || 
       ev.what == SIGFPE  || 
       ev.what == SIGSEGV || 
       ev.what == SIGBUS ) {
     // We don't actually throw back signals that are caused by 
     // executing an instruction.  We can just drop these and
     // let the continueLWP_ re-execute the instruction and cause
     // it to be rethrown.
     return true;
  }

  if ( ev.what != SIGTRAP ) {
     suppressed_sigs.push_back(ev.what);
     suppressed_lwps.push_back(ev.lwp);
  }

  return true;
}

bool SignalGenerator::resendSuppressedSignals()
{
  assert(suppressed_sigs.size() == suppressed_lwps.size());
  for (unsigned int i = 0; i < suppressed_sigs.size(); ++i)
  {
    //Throw back the extra signals we caught.
    lwp_kill(suppressed_lwps[i]->get_lwp_id(), suppressed_sigs[i]);
  }
  suppressed_lwps.clear();
  suppressed_sigs.clear();
  return true;
}

bool dyn_lwp::removeSigStop()
{
  //fprintf(stderr, "%s[%d][%s]:  welcome to removeSigStop, about to lwp_kill(%d, %d)\n",
   //       FILE__, __LINE__, getThreadStr(getExecThreadID()), get_lwp_id(), SIGCONT);
  return (lwp_kill(get_lwp_id(), SIGCONT) == 0);
}

bool dyn_lwp::continueLWP_(int signalToContinueWith, bool ignore_suppress) 
{
   proccontrol_printf("%s[%d]:  ContinuingLWP_ %d with %d\n", FILE__, __LINE__,
          get_lwp_id(), signalToContinueWith);

   // we don't want to operate on the process in this state
   int arg3 = 0;
   int arg4 = 0;
   if(signalToContinueWith != NoSignal) {
      arg3 = 1;
      arg4 = signalToContinueWith;
   }

   if (! ignore_suppress) {
      if (proc()->sh->waitingForStop())
      {
         return false;
      }
   }

   if (status() == exited) {
       proccontrol_printf("%s[%d]: lwp %d status is exited, not continuing...\n", FILE__, __LINE__, get_lwp_id());
      return true;
   }

   proccontrol_printf("%s[%d]: lwp %d getting PTRACE_CONT with signal %d\n",
                      FILE__, __LINE__, get_lwp_id(), arg4);
   ptraceOps++; 
   ptraceOtherOps++;

   int ptrace_errno = 0;
   int ret;
   if (proc()->getTraceSysCalls()) {
       ret = DBI_ptrace(PTRACE_SYSCALL, get_lwp_id(), arg3, arg4,
                       &ptrace_errno, proc_->getAddressWidth(),
                       FILE__, __LINE__);
   }
   else {
      ret = DBI_ptrace(PTRACE_CONT, get_lwp_id(), arg3, arg4,
                       &ptrace_errno, proc_->getAddressWidth(),
                       FILE__, __LINE__);
   }

   if (ret == 0)
      return true;

   if(ptrace_errno == ESRCH) // lwp terminated
      return true;

   /**
    * A stopped ptrace'd process can be in two states on Linux.
    * If it blocked on a signal, but never received a PTRACE_CONT,
    * then it should be continued with a PTRACE_CONT.
    *
    * If the kernel couldn't find an awake thread to deliver the signal to,
    * it'll grab anyone (usually the last) and wake them up for long enough
    * to deliver the signal.  This leaves that thread is a different state
    * than the others, and it won't actually respond to the PTRACE_CONT.  
    **/
   ret = P_kill(get_lwp_id(), SIGCONT);
   if (ret == -1)
      return false;
   return true;
}

bool dyn_lwp::waitUntilStopped()
{
   if ((status() == stopped) || (status() == exited))
   {
      return true;
   }
   
   SignalGenerator *sh = (SignalGenerator *) proc()->sh;
   waiting_for_stop = true;
   
   // Wake up the signal generator...
   signal_printf("%s[%d]: waitUntilStopped for lwp %u\n",
                 FILE__, __LINE__, get_lwp_id());
   
   // Continue suppression technique...
   sh->markProcessStop();
   while (status() != stopped) {
      if( status() == exited ) break;
      
      // and make sure the signal generator is woken up and in waitpid...
      sh->signalActiveProcess();
      
      signal_printf("%s[%d]:  before waitForEvent(evtProcessStop) for lwp %d: status is %s\n",
                    FILE__, __LINE__, get_lwp_id(), getStatusAsString().c_str());
      sh->waitForEvent(evtProcessStop, NULL, NULL, NULL_STATUS_INITIALIZER, false);
   }
 
   waiting_for_stop = false;
   
   sh->belayActiveProcess();
   sh->unmarkProcessStop();
   sh->resendSuppressedSignals();
   
   return true;
}

bool dyn_lwp::stop_() 
{
    if (waiting_for_stop) {
        return true;
    }

    proccontrol_printf("%s[%d][%s]:  welcome to stop_, about to lwp_kill(%d, %d)\n",
                       FILE__, __LINE__, getThreadStr(getExecThreadID()), get_lwp_id(), SIGSTOP);
    return (lwp_kill(get_lwp_id(), SIGSTOP) == 0);
} 

bool process::continueProc_(int sig)
{
  bool no_err = true;
  dyn_lwp *lwp;
  unsigned index = 0; 

   //Continue all real LWPs
  dictionary_hash_iter<unsigned, dyn_lwp *> lwp_iter(real_lwps);
  while (lwp_iter.next(index, lwp))
  {
     if (lwp->status() == running || lwp->status() == exited)
        continue;

     bool result = lwp->continueLWP(sig);
     if (result)
        set_lwp_status(lwp, running);
     else
        no_err = false;
  }

  //Continue any representative LWP
  if (representativeLWP && representativeLWP->status() != running)
  {
     bool result = representativeLWP->continueLWP(sig);
     if (result)
        set_lwp_status(representativeLWP, running);
     else
        no_err = false;
  }

  return no_err;
}

bool SignalGenerator::waitForStopInline()
{
    int retval = 0;
    int status = 0;
    
    retval = waitpid_mux.waitpid(this, &status);
    if (retval < 0) {
        //  should do some better checking here....
        perror("waitpid");
        return false;
    }
    
    signal_printf("%s[%d]: waitForStopInline with retval %d, sig %d/%d\n", 
                  FILE__, __LINE__, retval, WIFSTOPPED(status), WSTOPSIG(status));
    
    return true;
}

bool process::stop_(bool waitUntilStop)
{
  int result;
  bool retval = false;
  dictionary_hash_iter<unsigned, dyn_lwp *> lwp_iter(real_lwps);
  unsigned index = 0;
  dyn_lwp *lwp;
  
  
  if (status_ == stopped) {
     return true;
  }

  sh->setWaitingForStop(true);
  
  //Stop the main process
  result = P_kill(getPid(), SIGSTOP);
  if (result == -1) 
  {
    perror("Couldn't send SIGSTOP\n");
    goto done;
  }


  if (waitUntilStop) {
    if (!waitUntilLWPStops()) 
       goto done;
    if (status() == exited)
       goto done;
  }

  //Stop all other LWPs
  while(lwp_iter.next(index, lwp))
  {
     lwp->pauseLWP(waitUntilStop);
  }

  retval = true;
 done:
  sh->setWaitingForStop(false);
  ((SignalGenerator *)sh)->resendSuppressedSignals();

  return retval;
}

bool process::waitUntilStopped()
{
    signal_printf("%s[%d]: process waitUntilStopped...\n", FILE__, __LINE__);
    dictionary_hash_iter<unsigned, dyn_lwp *> lwp_iter(real_lwps);
    dyn_lwp *lwp;
    unsigned index = 0;
    bool result = true;
    
    while(lwp_iter.next(index, lwp))
        {
            
            result &= lwp->waitUntilStopped();
        }
    
    return result;
}

bool process::waitUntilLWPStops()
{
   sh->markProcessStop();

   while ( status() != stopped) {
      if (status() == exited) {
         sh->unmarkProcessStop();
         return false;
      }
      signal_printf("%s[%d][%s]:  before waitForEvent(evtProcessStop)\n", 
                    FILE__, __LINE__, getThreadStr(getExecThreadID()));
      sh->waitForEvent(evtProcessStop);
   }
   sh->unmarkProcessStop();

   return true;
}

terminateProcStatus_t process::terminateProc_()
{
  if (kill( getPid(), SIGKILL )) {
    // Kill failed... 
    if (errno == ESRCH)
      return alreadyTerminated;
    else
      return terminateFailed;
  }

  if (status() == stopped)
     continueProc(SIGCONT);

    return terminateSucceeded;
}

void dyn_lwp::realLWP_detach_() 
{
   if(! proc_->isAttached()) {
      if (! proc_->hasExited())
         cerr << "Detaching, but not attached" << endl;
      return;
   }
    
    ptraceOps++;
    ptraceOtherOps++;
    int ptrace_errno = 0;
    int ptrace_ret = DBI_ptrace(PTRACE_DETACH, get_lwp_id(),1, SIGCONT, &ptrace_errno, proc_->getAddressWidth(),  __FILE__, __LINE__); 
    if (ptrace_ret < 0) {
      fprintf(stderr, "%s[%d]:  ptrace failed: %s\n", __FILE__, __LINE__, strerror(ptrace_errno));
    }
    proc()->sh->remove_lwp_from_poll_list(get_lwp_id());
    proc()->sh->unregisterLWP(get_lwp_id());

    if (fd_ != INVALID_HANDLE_VALUE) {
        close(fd_);
        fd_ = INVALID_HANDLE_VALUE;
    }

    return;
}

void dyn_lwp::representativeLWP_detach_() {
    // If the process is already exited, then don't call ptrace
    if(! proc_->isAttached())
        return;
    
    if (fd_) close(fd_);
    
    ptraceOps++;
    ptraceOtherOps++;
   int ptrace_errno = 0;
    DBI_ptrace(PTRACE_DETACH, get_lwp_id(),1, SIGCONT, &ptrace_errno, proc_->getAddressWidth(),  __FILE__, __LINE__); 
    proc()->sh->remove_lwp_from_poll_list(get_lwp_id());
    proc()->sh->unregisterLWP(get_lwp_id());

    return;
}

bool process::dumpCore_(const std::string/* coreFile*/) { return false; }

bool dyn_lwp::writeTextWord(caddr_t inTraced, int data) {
   //  cerr << "writeTextWord @ " << (void *)inTraced << endl; cerr.flush();
   return writeDataSpace(inTraced, sizeof(int), (caddr_t) &data);
}

bool dyn_lwp::writeTextSpace(void *inTraced, u_int amount, const void *inSelf)
{
  //    cerr << "writeTextSpace pid=" << getPid() << ", @ " << (void *)inTraced
  //     << " len=" << amount << endl; cerr.flush();
   return writeDataSpace(inTraced, amount, inSelf);
}

bool dyn_lwp::readTextSpace(const void *inTraced, u_int amount, void *inSelf) {
   return readDataSpace(inTraced, amount, inSelf);
}

bool DebuggerInterface::bulkPtraceWrite(void *inTraced, u_int nbytes, void *inSelf, int pid, int /*address_width*/)
{
   unsigned char *ap = (unsigned char*) inTraced;
   const unsigned char *dp = (const unsigned char*) inSelf;
   Address w = 0x0;               /* ptrace I/O buffer */
   int len = sizeof(Address); /* address alignment of ptrace I/O requests */
   unsigned cnt;

   //cerr << "writeDataSpace pid=" << getPid() << ", @ " << (void *)inTraced
   //    << " len=" << nbytes << endl; cerr.flush();

   ptraceOps++; ptraceBytes += nbytes;
   stats_ptrace.incrementCounter(PTRACE_WRITE_COUNTER);
   stats_ptrace.addCounter(PTRACE_WRITE_AMOUNT, nbytes);
   stats_ptrace.startTimer(PTRACE_WRITE_TIMER);

   if (0 == nbytes) {
       stats_ptrace.stopTimer(PTRACE_WRITE_TIMER);
       return true;
   }

   if ((cnt = ((Address)ap) % len)) {
      /* Start of request is not aligned. */
      unsigned char *p = (unsigned char*) &w;

      /* Read the segment containing the unaligned portion, edit
         in the data from DP, and write the segment back. */
      errno = 0;
      w = P_ptrace(PTRACE_PEEKTEXT, pid, (Address) (ap-cnt), 0);

      if (errno) {
         fprintf(stderr, "%s[%d]:  write data space failing, pid %d\n", __FILE__, __LINE__, pid);
       stats_ptrace.stopTimer(PTRACE_WRITE_TIMER);
         return false;
      }

      for (unsigned i = 0; i < len-cnt && i < nbytes; i++)
         p[cnt+i] = dp[i];

      if (0 > P_ptrace(PTRACE_POKETEXT, pid, (Address) (ap-cnt), w)) {
         fprintf(stderr, "%s[%d]:  write data space failing\n", __FILE__, __LINE__);
       stats_ptrace.stopTimer(PTRACE_WRITE_TIMER);
         return false;
      }

      if (len-cnt >= nbytes) {
       stats_ptrace.stopTimer(PTRACE_WRITE_TIMER);

         return true; /* done */
      }

      dp += len-cnt;
      ap += len-cnt;
      nbytes -= len-cnt;
   }

   /* Copy aligned portion */
   while (nbytes >= (u_int)len) {
      assert(0 == ((Address)ap) % len);
      memcpy(&w, dp, len);
      int retval =  P_ptrace(PTRACE_POKETEXT, pid, (Address) ap, w);
      if (retval < 0) {
         fprintf(stderr, "%s[%d]:  write data space failing, pid %d\n", __FILE__, __LINE__, pid);
         fprintf(stderr, "%s[%d][%s]:  tried to write %lx in address %p\n", FILE__, __LINE__, getThreadStr(getExecThreadID()),w, ap);
         perror("ptrace");
       stats_ptrace.stopTimer(PTRACE_WRITE_TIMER);
         return false;
      }

      // Check...
      /*
      Address test;
      fprintf(stderr, "Writing %x... ", w);
      test = P_ptrace(PTRACE_PEEKTEXT, get_lwp_id(), (Address) ap, 0);
      fprintf(stderr, "... got %x, lwp %d\n", test, get_lwp_id());
      */
      dp += len;
      ap += len;
      nbytes -= len;
   }

   if (nbytes > 0) {
      /* Some unaligned data remains */
      unsigned char *p = (unsigned char *) &w;

      /* Read the segment containing the unaligned portion, edit
         in the data from DP, and write it back. */
      errno = 0;
      w = P_ptrace(PTRACE_PEEKTEXT, pid, (Address) ap, 0);

      if (errno) {
         fprintf(stderr, "%s[%d]:  write data space failing\n", __FILE__, __LINE__);
       stats_ptrace.stopTimer(PTRACE_WRITE_TIMER);
         return false;
      }


      for (unsigned i = 0; i < nbytes; i++)
         p[i] = dp[i];

      if (0 > P_ptrace(PTRACE_POKETEXT, pid, (Address) ap, w)) {
         fprintf(stderr, "%s[%d]:  write data space failing\n", __FILE__, __LINE__);
       stats_ptrace.stopTimer(PTRACE_WRITE_TIMER);
         return false;
      }

   }

       stats_ptrace.stopTimer(PTRACE_WRITE_TIMER);
   return true;

}

bool dyn_lwp::writeDataSpace(void *inTraced, u_int nbytes, const void *inSelf)
{
   unsigned char *ap = (unsigned char*) inTraced;
   const unsigned char *dp = (const unsigned char*) inSelf;

   //fprintf(stderr, "%s[%d]:  welcome to dyn_lwp::writeDataSpace(%d bytes)\n", __FILE__, __LINE__, nbytes);
   //cerr << "writeDataSpace pid=" << getPid() << ", @ " << (void *)inTraced
   //    << " len=" << nbytes << endl; cerr.flush();

   ptraceOps++; ptraceBytes += nbytes;

   if (!DBI_writeDataSpace(get_lwp_id(), (Address) ap, nbytes, (Address) dp, sizeof(Address), __FILE__, __LINE__)) {
     fprintf(stderr, "%s[%d]:  bulk ptrace failed\n", __FILE__, __LINE__);
     return false;
   }
   return true;
}


bool DebuggerInterface::bulkPtraceRead(void *inTraced, u_int nelem, void *inSelf, 
                                       int pid, int /*address_width*/) 
{
   bool result;
         
   ptraceOps++; 
   ptraceBytes += nelem;
   stats_ptrace.incrementCounter(PTRACE_READ_COUNTER);
   stats_ptrace.addCounter(PTRACE_READ_AMOUNT, nelem);
   stats_ptrace.startTimer(PTRACE_READ_TIMER);
   
   result = PtraceBulkRead((Address) inTraced, nelem, inSelf, pid);
   if (!result)
      signal_printf("%s[%d]:  ptrace failed: %s\n", FILE__, __LINE__, 
                    strerror(errno));

   stats_ptrace.stopTimer(PTRACE_READ_TIMER);

   return result;
}


bool dyn_lwp::readDataSpace(const void *inTraced, u_int nbytes, void *inSelf) {
     const unsigned char *ap = (const unsigned char*) inTraced;
     unsigned char *dp = (unsigned char*) inSelf;
     int len = proc_->getAddressWidth(); /* address alignment of ptrace I/O requests */

     ptraceOps++; ptraceBytes += nbytes;

     bool ret = false;
     if (! (ret = DBI_readDataSpace(get_lwp_id(), (Address) ap, nbytes,
                                    (Address) dp, /*sizeof(Address)*/ len,
                                    FILE__, __LINE__))) {
        signal_printf("%s[%d]:  bulk ptrace read failed for lwp id %d\n",
                      FILE__, __LINE__, get_lwp_id());
        return false;
     }
     return true;
}



static std::string getNextLine(int fd)
{
   std::string line = "";
   while(true) {
      int buf = 0;
      char *byte = (char *)& buf;
      byte[0] = '\0';
      size_t retval = P_read(fd, (char *)&buf, 1);
      byte[1] = '\0';
      if ((retval > 0) && (byte[0] > 0))
         line = line + std::string(byte, 1);
      else
         break;
   }
   return line;
}

std::string process::tryToFindExecutable(const std::string& /* progpath */, int pid)
{
   char buffer[PATH_MAX];
   int fd, length;
   char pidstr[16];
   sprintf(pidstr, "%d", pid);

   //
   // Simply dereferencing the symbolic link at /proc/<pid>/exe will give the
   // full path of the executable 99% of the time on Linux. Check there first.
   // The funky business with memset() and strlen() here is to deal with early
   // Linux kernels that returned incorrect length values from readlink().
   //
   memset(buffer, 0, sizeof(buffer));
   readlink((std::string("/proc/") + std::string(pidstr) + std::string("/exe")).c_str(),
         buffer, sizeof(buffer) - 1);
   length = strlen(buffer);
   if ((length > 0) && (buffer[length-1] == '*'))
      buffer[length-1] = 0;
   if (strlen(buffer) > 0)
      return buffer;

   //
   // Currently the only known case where the above fails is on the back-end
   // nodes of a bproc (http://bproc.sourceforge.net/) system. On these systems
   // /proc/<pid>/exe is simply a zero-sized file and the /proc/<pid>/maps
   // table has an empty name for the executable's mappings. So we have to get
   // a little more creative.
   //

   // Obtain the command name from /proc/<pid>/cmdline
   std::string cmdname = "";
   fd = P_open((std::string("/proc/") + std::string(pidstr) +
            std::string("/cmdline")).c_str(), O_RDONLY, 0);
   if (fd >= 0) {
      cmdname = getNextLine(fd);
      P_close(fd);
   }

   // Obtain the path from /proc/<pid>/environ
   std::string path = "";
   fd = P_open((std::string("/proc/") + std::string(pidstr) +
            std::string("/environ")).c_str(), O_RDONLY, 0);
   if (fd >= 0) {
      while(true) {
         path = getNextLine(fd);
         //if (path.prefixed_by("PATH=")) {
         if (!strncmp(path.c_str(), "PATH=", strlen("PATH="))) {
            path = path.substr(5, path.length() - 5);
            break;
         }
         else if(path.length() == 0)
            break;
      }
      P_close(fd);
   }

   // Obtain the current working directory from /proc/<pid>/cwd
   std::string cwd = "";
   memset(buffer, 0, sizeof(buffer));
   readlink((std::string("/proc/") + std::string(pidstr) +
            std::string("/cwd")).c_str(), buffer, sizeof(buffer) - 1);
   length = strlen(buffer);
   if (length > 0)
      cwd = buffer;

   // Now try to find the executable using these three items
   std::string executable = "";
   if (executableFromArgv0AndPathAndCwd(executable, cmdname, path, cwd))
      return executable;

   // Indicate an error if none of the above methods succeeded
   return "";
}


bool process::determineLWPs(pdvector<unsigned> &lwp_ids)
{
   char name[128];
   struct dirent *direntry;

   /**
    * Linux 2.6:
    **/
   sprintf(name, "/proc/%d/task", getPid());
   DIR *dirhandle = opendir(name);
   if (dirhandle)
   {
      //Only works on Linux 2.6
      while((direntry = readdir(dirhandle)) != NULL) {
         unsigned lwp_id = atoi(direntry->d_name);
         if (lwp_id) 
            lwp_ids.push_back(lwp_id);
      }
      closedir(dirhandle);
      return true;
   }
   /**
    * Linux 2.4:
    *
    * PIDs that are created by pthreads have a '.' prepending their name
    * in /proc.  We'll check all of those for the ones that have this lwp
    * as a parent pid.
    **/
   dirhandle = opendir("/proc");
   if (!dirhandle)
   {
      //No /proc directory.  I give up.  No threads for you.
      return false;
   } 
   while ((direntry = readdir(dirhandle)) != NULL)
   {
      if (direntry->d_name[0] != '.') {
         //fprintf(stderr, "%s[%d]: Skipping entry %s\n", FILE__, __LINE__, direntry->d_name);
         continue;
      }
      unsigned lwp_id = atoi(direntry->d_name+1);
      int lwp_ppid;
      if (!lwp_id) 
         continue;
      sprintf(name, "/proc/%d/status", lwp_id);
      FILE *fd = P_fopen(name, "r");
      if (!fd) {
         continue;
     }
     char buffer[1024];
     while (fgets(buffer, 1024, fd)) {
         if (strncmp(buffer, "Tgid", 4) == 0) {
             sscanf(buffer, "%*s %d", &lwp_ppid);
             break;
         }
     }

     fclose(fd);

     if (lwp_ppid != getPid()) {
         continue;
     }
     lwp_ids.push_back(lwp_id);
  }
  closedir(dirhandle);
  lwp_ids.push_back(getPid());
  
  return true;
}


bool process::dumpImage( std::string ) 
{
   return false;
}

//Returns true if the function is part of the PLT table
bool isPLT(int_function *f)
{
    Symtab *obj = f->mod()->obj()->parse_img()->getObject();
    Region *sec;
    if(obj->findRegion(sec, ".plt"))
	return sec->isOffsetInRegion(f->ifunc()->getOffset());
    return false;	
}

bool SignalGeneratorCommon::getExecFileDescriptor(std::string filename,
                                                  int /*pid*/,
                                                  bool /*whocares*/,
                                                  int &,
                                                  fileDescriptor &desc)
{
    desc = fileDescriptor(filename.c_str(), 
                          0, // code
                          0, // data
                          false); // a.out
    return true;
}

static const Address lowest_addr = 0x0;
void process::inferiorMallocConstraints(Address near, Address &lo, Address &hi,
			       inferiorHeapType /* type */ )
{
  if (near)
    {
#if !defined(arch_x86_64) && !defined(arch_power)
      lo = region_lo(near);
      hi = region_hi(near);  
#else
      if (getAddressWidth() == 8) {
	  lo = region_lo_64(near);
	  hi = region_hi_64(near);
      }
      else {
	  lo = region_lo(near);
	  hi = region_hi(near);  
      }
#endif
    }
}

bool dyn_lwp::realLWP_attach_() {
   char procName[128];
   sprintf(procName, "/proc/%d/mem", get_lwp_id());
   fd_ = P_open(procName, O_RDWR, 0);

   if (fd_ < 0)
     fd_ = INVALID_HANDLE_VALUE;

   if (!proc()->sh->registerLWP(get_lwp_id())) {
      fprintf(stderr, "%s[%d]:  failed to register lwp %d here\n", FILE__, __LINE__, get_lwp_id());
   }

   isDoingAttach_ = true;

   startup_printf("%s[%d]:  realLWP_attach doing PTRACE_ATTACH to %lu\n", 
                  FILE__, __LINE__, get_lwp_id());
   int ptrace_errno = 0;
   if( 0 != DBI_ptrace(PTRACE_ATTACH, get_lwp_id(), 0, 0, &ptrace_errno, 
                       proc_->getAddressWidth(),  __FILE__, __LINE__) )
   {
      //Any thread could have exited before we attached to it.
      isDoingAttach_ = false;
      return false;
   }
   
   proc()->sh->add_lwp_to_poll_list(this);

   eventType evt;

   pdvector<eventType> evts;   
   evts.push_back(evtLwpAttach);
   evts.push_back(evtThreadExit);
   
   proc()->sh->signalActiveProcess();
   do {

     evt = proc()->sh->waitForOneOf(evts, this);
     //evt = proc()->sh->waitForEvent(evtLwpAttach, proc_, this);
     if (evt == evtProcessExit) {
       isDoingAttach_ = false;
       return false;
     }
     if (evt == evtThreadExit && status_ == exited) {
       isDoingAttach_ = false;
       proc()->sh->remove_lwp_from_poll_list(get_lwp_id());       
       return false;
     }
   } while (!is_attached());
   
   isDoingAttach_ = false;

   // Make sure nobody has mucked with our process state.
   status_ = stopped;

   if (proc_->sh->syncRunWhenFinished_ == runRequest) {
       continueLWP();
   }

   return true;
}

static bool is_control_stopped(int lwp)
{
 FILE *f = NULL;
 char line[128];
 char filename[32];
 bool result = false;
 char *sresult = NULL;
 
 snprintf(filename, 32, "/proc/%d/status", lwp);
 filename[31] = '\0';
 
 f = fopen(filename, "r");
 if (!f)
 {
    if (dyn_debug_startup) {
       startup_printf("Error opening %s", filename);
       perror("ERROR: Couldn't open file");
    }
    goto done;
 }

 while (!feof(f)) {
    sresult = fgets(line, 128, f);
    if (!sresult) {
       if (dyn_debug_startup) {
          startup_printf("Error reading from %s", filename);
          perror("ERROR: Couldn't read from file");
       }
       goto done;
    }
    if (strncmp(line, "State:", 6) == 0) {
       if (strstr(line, "T (stopped)"))
          result = true;
       goto done;
    }
 }


 done:
 if (f)
    fclose(f);
 return result;
}

bool dyn_lwp::representativeLWP_attach_() 
{
   bool running = false;
   int result;

   if( proc_->wasCreatedViaAttach() )
      running = proc_->isRunning_();
   
   startup_printf("%s[%d]: in representative lwp attach, isRunning %d\n",
                  FILE__, __LINE__, running);


   // QUESTION: does this attach operation lead to a SIGTRAP being forwarded
   // to paradynd in all cases?  How about when we are attaching to an
   // already-running process?  (Seems that in the latter case, no SIGTRAP
   // is automatically generated)
   
   // Only if we are really attaching rather than spawning the inferior
   // process ourselves do we need to call PTRACE_ATTACH
   if(proc_->wasCreatedViaAttach() || 
      proc_->wasCreatedViaFork() || 
      proc_->wasCreatedViaAttachToCreated())
   {
      int ptrace_errno = 0;
      int address_width = sizeof(Address);
      assert(address_width);
      startup_printf("%s[%d]: process attach doing PT_ATTACH to %d\n",
                     FILE__, __LINE__, get_lwp_id());
      if( 0 != DBI_ptrace(PTRACE_ATTACH, getPid(), 0, 0, &ptrace_errno, 
                          address_width, __FILE__, __LINE__) )
      {
         startup_printf("%s[%d]:  ptrace attach to pid %d failing: %s\n", 
                        FILE__, __LINE__, getPid(), strerror(ptrace_errno));
         return false;
      }
      startup_printf("%s[%d]: attached via DBI\n", FILE__, __LINE__);
      proc_->sh->add_lwp_to_poll_list(this);

      /**
       * The following code is trying to deal with a bug on recent utrace 
       * kernels (first appeared in Fedora Core 8, 9, and RHEL 5.2).  When 
       * attaching to a process that is already control stopped, wait() will
       * stop blocking and keep returning notice that 'signal 0' has been 
       * delivered. 
       *
       * As per RedHat's suggestion, we're detecting when a process is control 
       * stopped, then moving it into a tracing stop state by sending it a SIGSTOP
       * and then moving it out of the control stop state with a PTRACE_CONT.
       *
       * The PTRACE_CONT will destroy the normal SIGSTOP you recieve after attaching
       * to a process, but we've just put another SIGSTOP into the signal queue to 
       * replace it.  
       **/
      if (is_control_stopped(get_lwp_id()))
      {
         startup_printf("[%s:%u] - Attached to already STOP'd process, %d.  Handling.\n",
                        FILE__, __LINE__, get_lwp_id());
         proc_->started_stopped = true;
         
         result = lwp_kill(get_lwp_id(), SIGSTOP);
         if (result == -1) {
            startup_printf("[%s:%u] - ERROR.  Could not lwp_kill %d: %s\n", 
                           FILE__, __LINE__, get_lwp_id(), strerror(errno));
         }
         else {
            result = DBI_ptrace(PTRACE_CONT, get_lwp_id(), 0, 0, &ptrace_errno,
                                address_width, FILE__, __LINE__);
            if (result == -1) {
               startup_printf("[%s:%u] - ERROR. Could not PTRACE_CONT: %s\n", 
                              FILE__, __LINE__, strerror(ptrace_errno));
            }
         }
      }

      int status = 0;
      int retval = 0;
      int dead_lwp = 0;
      retval = proc_->sh->waitpid_kludge(getPid(), &status, 0, &dead_lwp);
      if (retval < 0) {
          fprintf(stderr, "%s[%d]:  waitpid failed\n", FILE__, __LINE__);
          perror("process::attach - waitpid");
          exit(1);
      }
      startup_printf("%s[%d]: waitpid return from %d of %d/%d\n",
                     FILE__, __LINE__, retval, WIFSTOPPED(status), WSTOPSIG(status));
      proc_->set_status(stopped);
   }

   return true;
}

void loadNativeDemangler() {}

const unsigned int N_DYNINST_LOAD_HIJACK_FUNCTIONS = 4;
const char DYNINST_LOAD_HIJACK_FUNCTIONS[][20] = {
  "__libc_start_main",
  "_init",
  "_start",
  "main"
};

/**
 * Returns an address that we can use to write the code that executes
 * dlopen on the runtime library.
 *
 * Inserting the code into libc is a good thing, since _dl_open
 * will sometimes check it's caller and return with a 'invalid caller'
 * error if it's called from the application.
 **/
Address findFunctionToHijack(process *p) 
{
   Address codeBase = 0;
   unsigned i;
   for(i = 0; i < N_DYNINST_LOAD_HIJACK_FUNCTIONS; i++ ) {
      const char *func_name = DYNINST_LOAD_HIJACK_FUNCTIONS[i];

      pdvector<int_function *> hijacks;
      if (!p->findFuncsByAll(func_name, hijacks))
          return 0;
      codeBase = hijacks[0]->getAddress();

      if (codeBase)
          break;
   }
   
  return codeBase;
} /* end findFunctionToHijack() */

/**
 * Searches for function in order, with preference given first 
 * to libpthread, then to libc, then to the process.
 **/
static void findThreadFuncs(process *p, std::string func, 
                            pdvector<int_function *> &result)
{
   bool found = false;
   mapped_module *lpthread = p->findModule("libpthread*", true);
   if (lpthread)
      found = lpthread->findFuncVectorByPretty(func, result);
   if (found)
      return;

   mapped_module *lc = p->findModule("libc.so*", true);
   if (lc)
      found = lc->findFuncVectorByPretty(func, result);
   if (found)
      return;
   
   p->findFuncsByPretty(func, result);
}

bool process::initMT()
{
   unsigned i;
   bool res;

#if !defined(cap_threads)
   return true;
#endif

   /**
    * Instrument thread_create with calls to DYNINST_dummy_create
    **/
   //Find create_thread
   pdvector<int_function *> thread_init_funcs;
   findThreadFuncs(this, "create_thread", thread_init_funcs);
   findThreadFuncs(this, "start_thread", thread_init_funcs);
   if (thread_init_funcs.size() < 1)
   {
      fprintf(stderr, "[%s:%d] - Found no copies of create_thread, expected 1\n",
              __FILE__, __LINE__);
      return false;
   }
   //Find DYNINST_dummy_create
   int_function *dummy_create = findOnlyOneFunction("DYNINST_dummy_create");
   if (!dummy_create)
   {
     fprintf(stderr, "[%s:%d] - Couldn't find DYNINST_dummy_create",
             __FILE__, __LINE__);
      return false;
   }
   //Instrument
   for (i=0; i<thread_init_funcs.size(); i++)
   {
      pdvector<AstNodePtr> args;
      AstNodePtr ast = AstNode::funcCallNode(dummy_create, args);
      const pdvector<instPoint *> &ips = thread_init_funcs[i]->funcEntries();
      for (unsigned j=0; j<ips.size(); j++)
      {
         miniTramp *mt;
         mt = ips[j]->instrument(ast, callPreInsn, orderFirstAtPoint, false, 
                                 false);
         if (!mt)
         {
            fprintf(stderr, "[%s:%d] - Couldn't instrument thread_create\n",
                    __FILE__, __LINE__);
         }
         //TODO: Save the mt objects for detach
      }
   }
      
   //Find functions that are run on pthread exit
   pdvector<int_function *> thread_dest_funcs;
   findThreadFuncs(this, "__pthread_do_exit", thread_dest_funcs);
   findThreadFuncs(this, "pthread_exit", thread_dest_funcs);
   findThreadFuncs(this, "deallocate_tsd", thread_dest_funcs);
   if (!thread_dest_funcs.size())
   {
      fprintf(stderr,"[%s:%d] - Found 0 copies of pthread_exit, expected 1\n",
              __FILE__, __LINE__);
      return false;
   }
   //Find DYNINSTthreadDestroy
   int_function *threadDestroy = findOnlyOneFunction("DYNINSTthreadDestroy");
   if (!threadDestroy)
   {
      fprintf(stderr, "[%s:%d] - Couldn't find DYNINSTthreadDestroy",
              __FILE__, __LINE__);
      return false;
   }

   for (i=0; i<thread_dest_funcs.size(); i++)
   {
      pdvector<AstNodePtr> args;
      AstNodePtr ast = AstNode::funcCallNode(threadDestroy, args);
      const pdvector<instPoint *> &ips = thread_dest_funcs[i]->funcExits();
      for (unsigned j=0; j<ips.size(); j++)
      {
         miniTramp *mt;
         mt = ips[j]->instrument(ast, callPreInsn, orderFirstAtPoint, false, 
                                 false);
         if (!mt)
         {
            fprintf(stderr, "[%s:%d] - Couldn't instrument thread_exit\n",
                    __FILE__, __LINE__);
         }
         //TODO: Save the mt objects for detach
      }
   }

#if 0
   //Instrument
   for (i=0; i<thread_dest_funcs.size(); i++)
   {
      pdvector<AstNodePtr > args;
      AstNode call_thread_destroy(threadDestroy, args);
      AstNodePtr ast = &call_thread_destroy;
      miniTrampHandle *mthandle;
      instPoint *ip = thread_dest_funcs[i]->funcEntry(this);

      result = addInstFunc(this, mthandle, ip, ast, callPostInsn, 
                           orderFirstAtPoint, true, true, true);
      if (result != success_res)
      {
         fprintf(stderr, "[%s:%d] - Couldn't instrument thread_destroy\n",
                 __FILE__, __LINE__);
      }
   }
#endif
     
   /**
    * Have dyn_pthread_self call the actual pthread_self
    **/
   //Find dyn_pthread_self
   pdvector<int_variable *> ptself_syms;
   res = findVarsByAll("DYNINST_pthread_self", ptself_syms);
   if (!res)
   {
      fprintf(stderr, "[%s:%d] - Couldn't find any dyn_pthread_self, expected 1\n",
              __FILE__, __LINE__);
   }
   assert(ptself_syms.size() == 1);
   Address dyn_pthread_self = ptself_syms[0]->getAddress();
   //Find pthread_self
   pdvector<int_function *> pthread_self_funcs;
   findThreadFuncs(this, "pthread_self", pthread_self_funcs);   
   if (pthread_self_funcs.size() != 1)
   {
      fprintf(stderr, "[%s:%d] - Found %ld pthread_self functions, expected 1\n",
              __FILE__, __LINE__, (long) pthread_self_funcs.size());
      for (unsigned j=0; j<pthread_self_funcs.size(); j++)
      {
         int_function *ps = pthread_self_funcs[j];
         fprintf(stderr, "[%s:%u] - %s in module %s at %lx\n", __FILE__, __LINE__,
                 ps->prettyName().c_str(), ps->mod()->fullName().c_str(), 
                 ps->getAddress());
      }
      return false;
   }   
   //Replace
   res = writeFunctionPtr(this, dyn_pthread_self, pthread_self_funcs[0]);
   if (!res)
   {
      fprintf(stderr, "[%s:%d] - Couldn't update dyn_pthread_self\n",
              __FILE__, __LINE__);
      return false;
   }

   return true;
}

void dyninst_yield()
{
   pthread_yield();
}

Address dyn_lwp::step_next_insn() {
   int result;
   if (status() != stopped) {
      fprintf(stderr, "[%s:%u] - Internal Error.  lwp wasn't stopped\n",
              __FILE__, __LINE__);
      return (Address) -1;
   }
   
   singleStepping = true;
   proc()->set_lwp_status(this, running);
   result = DBI_ptrace(PTRACE_SINGLESTEP, get_lwp_id(), 0x0, 0x0);
   if (result == -1) {
      perror("Couldn't PTRACE_SINGLESTEP");
      return (Address) -1;
   }

   proc()->getSG()->signalActiveProcess();
   while (singleStepping) {
      if(proc()->hasExited()) 
         return (Address) -1;
      proc()->sh->waitForEvent(evtDebugStep);
   }

   return getActiveFrame().getPC();
}

// ****** Support linux-specific forkNewProcess DBI callbacks ***** //

bool ForkNewProcessCallback::operator()(std::string file, 
                    std::string dir, pdvector<std::string> *argv,
                    pdvector<std::string> *envp,
                    std::string inputFile, std::string outputFile, int &traceLink,
                    pid_t &pid, int stdin_fd, int stdout_fd, int stderr_fd,
                    SignalGenerator *sg)
{
  lock->_Lock(FILE__, __LINE__);
   
  file_ = &file;
  dir_ = &dir;
  argv_ = argv;
  envp_ = envp;
  inputFile_ = &inputFile;
  outputFile_ = &outputFile;
  traceLink_ = &traceLink;
  pid_ = &pid;
  sg_ = sg;
  stdin_fd_ = stdin_fd;
  stdout_fd_ = stdout_fd;
  stderr_fd_ = stderr_fd;
  bool result = true;
  
  startup_printf("%s[%d]:  ForkNewProcessCallback, target thread is %lu(%s)\n", 
                  __FILE__, __LINE__, targetThread(), 
                 getThreadStr(targetThread()));
  getMailbox()->executeOrRegisterCallback(this);

  if (synchronous) 
  {
     dbi_printf("%s[%d]:  waiting for completion of callback\n", FILE__, __LINE__);
     waitForCompletion();
  }

  lock->_Unlock(FILE__, __LINE__);
  return result;
}

bool ForkNewProcessCallback::execute_real()
{
    // call the actual forking routine in unix.C
    ret = forkNewProcess_real(*file_,*dir_,argv_,envp_,*inputFile_,
            *outputFile_,*traceLink_,*pid_,stdin_fd_,stdout_fd_,
            stderr_fd_); 
    if (ret)
       ret = sg_->attachToChild(*pid_);
    return ret;
}

bool DebuggerInterface::forkNewProcess(std::string file, 
                    std::string dir, pdvector<std::string> *argv,
                    pdvector<std::string> *envp,
                    std::string inputFile, std::string outputFile, int &traceLink,
                    pid_t &pid, int stdin_fd, int stdout_fd, int stderr_fd,
                    SignalGenerator *sg)
{
    dbi_printf("%s[%d][%s]:  welcome to DebuggerInterface::forkNewProcess()\n",
          FILE__, __LINE__, getThreadStr(getExecThreadID()));
    getBusy();

    bool ret;
    ForkNewProcessCallback *fnpp = new ForkNewProcessCallback(&dbilock);
    ForkNewProcessCallback &fnp = *fnpp;

    fnp.enableDelete(false);
    fnp(file, dir, argv, envp, inputFile, outputFile, traceLink, pid,
        stdin_fd, stdout_fd, stderr_fd, sg);

    ret = fnp.getReturnValue();
    fnp.enableDelete();
    releaseBusy();

    return ret;  
}

bool SignalHandler::handleProcessExitPlat(EventRecord & /*ev*/, 
                                          bool & /*continueHint */) 
{
   sg->waitpid_mux.unregisterProcess(sg);
   return true;
}

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

void chld_handler(int) {
}

void chld_handler2(int, siginfo_t *, void *) {
}

static void kickWaitpider(int pid) {
   struct sigaction handler;

   //   handler.sa_handler = chld_handler;
   sigfillset(& handler.sa_mask);
   handler.sa_sigaction = chld_handler2;
   handler.sa_flags = 0; 
   sigaction(SIGCHLD, &handler, NULL);

   proccontrol_printf("[%s:%u] - kicking %d for wakeup\n",
                      FILE__, __LINE__, pid);
   lwp_kill(pid, SIGCHLD);
}

//Force the SignalGenerator to return -1, EINTR from waitpid
bool WaitpidMux::registerProcess(SignalGenerator *me) 
{

  pthread_mutex_lock(&waiter_mutex);
  
  for (unsigned i=0; i<unassigned_events.size(); i++)
     if (unassigned_events[i].pid == me->getPid()) {
        proccontrol_printf("[%s:%u] - Found early event for %d, restoring\n",
                           FILE__, __LINE__, me->getPid());
        me->event_queue.push_back(unassigned_events[i]);
        VECTOR_ERASE(unassigned_events,i, i);
        i--;
     }
  addPidGen(me->getPid(), me);

  pthread_mutex_unlock(&waiter_mutex);

  return true;
}

bool WaitpidMux::registerLWP(unsigned lwpid, SignalGenerator *me) 
{
   pthread_mutex_lock(&waiter_mutex);

   for (unsigned i=0; i<unassigned_events.size(); i++)
      if (unassigned_events[i].pid == me->getPid()) {
         proccontrol_printf("[%s:%u] - Found early event for %d, restoring\n",
                            FILE__, __LINE__, me->getPid());
         me->event_queue.push_back(unassigned_events[i]);
         VECTOR_ERASE(unassigned_events,i, i);
         i--;
      }
   
   addPidGen(lwpid, me);
   
   pthread_mutex_unlock(&waiter_mutex);
   return true;
}

bool WaitpidMux::unregisterLWP(unsigned lwpid, SignalGenerator *me) 
{
  pthread_mutex_lock(&waiter_mutex);
  removePidGen(lwpid, me);
  pthread_mutex_unlock(&waiter_mutex);
  return true;
}

bool WaitpidMux::unregisterProcess(SignalGenerator *me) 
{
  pthread_mutex_lock(&waiter_mutex);
  removePidGen(me);

  if (waitpid_thread_id == me->getThreadID()) {
    forceWaitpidReturn();
  }

  pthread_mutex_unlock(&waiter_mutex);
  return true;
}

void WaitpidMux::forceWaitpidReturn() 
{
  pthread_mutex_lock(&waiter_mutex);
  if (isInWaitpid) {
     kickWaitpider(waiter_exists);
  }
  else if (isInWaitLock) {
     forcedExit = true;
     pthread_cond_broadcast(&waiter_condvar);
  }
  pthread_mutex_unlock(&waiter_mutex);
}

bool WaitpidMux::suppressWaitpidActivity()
{     
   pthread_mutex_lock(&waiter_mutex);
   pause_flag = true;
   if (waiter_exists) {
      kickWaitpider(waiter_exists);
   }
   pthread_mutex_unlock(&waiter_mutex);
   return true;           
}

bool WaitpidMux::resumeWaitpidActivity()
{  
   pthread_mutex_lock(&waiter_mutex);
   pause_flag = false;
   pthread_cond_broadcast(&waiter_condvar);
   pthread_mutex_unlock(&waiter_mutex);
   return true;
}        

int WaitpidMux::waitpid(SignalGenerator *me, int *status)
{   
   int result;
   int options = __WALL;
   SignalGenerator *event_owner;

   pthread_mutex_lock(&waiter_mutex);
   
   proccontrol_printf("[%s:%u] waitpid_demultiplex called for %d\n", 
                      FILE__, __LINE__, me->getPid());
   for (;;) {
      //  Find the queue for this signalgenerator
      if (me->event_queue.size()) {
         //Someone put an event into our queue (thank you), we'll
         //go ahead and dequeue and return it.
         waitpid_ret_pair ret = me->event_queue[0];
         VECTOR_ERASE(me->event_queue,0, 0);
         *status = ret.status;
         proccontrol_printf("[%s:%u] %d found an event %d in my queue\n",
                            FILE__, __LINE__, me->getPid(), *status);
         pthread_mutex_unlock(&waiter_mutex);
         return ret.pid;
      }

      if (forcedExit) {
         //Someone wants to force us out of waitpid.  Go along with it.
         // In most cases, we'll just return to the waitpid_kludge function
         // and then re-enter this one.
         proccontrol_printf("[%s:%u] %d forced out of waitpid\n",
                            FILE__, __LINE__, me->getPid());
         forcedExit = false;
         pthread_mutex_unlock(&waiter_mutex);         
         return 0;
      }

      if (!waiter_exists) {
         // There's no events for us and no one is doing the waitpid.
         // Let's break out of this foolish loop and do
         // a waitpid.  We've still got the mutex, so we'll be the
         // only ones who can get out and set waiter_exists to true.
         proccontrol_printf("[%s:%u] %d becoming new waitpider\n",
                            FILE__, __LINE__, me->getPid());
         break;
      }

      if (hasFirstTimer(me)) {
         //Darn Linux annoyances.  If a thread is blocked in waitpid, and
         // someone else PTRACE_ATTACH's to a new thread, then we won't get
         // any events until waitpid is re-entered.  We'll send a SIGILL
         // to a mutatee to force waitPid to return, then drop the signal.
         kickWaitpider(waiter_exists);
      }
      // Someone else is currently doing a waitpid for us.  Block until
      // they either put an event into our event queue, or we find that they've
      // left and we need to become the new waitpider.
      isInWaitLock = true;
      pthread_cond_wait(&waiter_condvar, &waiter_mutex);
      isInWaitLock = false;
   }

   assert(!waiter_exists);
   waiter_exists = P_gettid();
   for (;;) {
      if (me->stopRequested()) {
         fprintf(stderr, "%s[%d]:  got request to stop doing waitpid()\n", FILE__, __LINE__);
         return -1;
      }
      //If we're in this loop, then we are the process doing a waitpid.
      // It's up to us to recieve events for all SignalGenerators, and put 
      // them on the event queue until we get one that belongs to us.
      // Once we get an event that belongs to us, we'll set a waiter_exists
      // to false (which will let someone else become the new waitpid'er)
      // and return the waitpid result.

      //A first-timer is someone who was created while we were are 
      // waitpid.  Since no one is in waitpid right now (we're 
      // about to enter) there are no first-timers.
      first_timers.clear();

      isInWaitpid = true;
      waitpid_thread_id = me->getThreadID();

      // Now everyone thinks we're doing waitpid, that's good because it means they
      // won't try...  but before we really do waitpid(), need to make sure that 
      // some other part of the system doesn't want us to:
      while (pause_flag) {
         pthread_cond_wait(&waiter_condvar, &waiter_mutex);        
      }

      pthread_mutex_unlock(&waiter_mutex);
      result = ::waitpid(-1, status, options);
      pthread_mutex_lock(&waiter_mutex);

      isInWaitpid = false;
      waitpid_thread_id = 0;

      proccontrol_printf("[%s:%u] waitpid by %d returned status %d for %d\n",
                         FILE__, __LINE__, me->getPid(), *status, result);

      //We got an event.  Map that back to the signal generator that it
      // belongs to.
      event_owner = NULL;
      for (unsigned i=0; i<pidgens.size(); i++) {
         if (pidgens[i].pid == result) {
            event_owner = pidgens[i].sg;
            break;
         }
      }

      if (WIFSTOPPED(*status) && WSTOPSIG(*status) == 0 && !event_owner) {
        /**
         * Kernel bug.  See the comment in dyn_lwp::representativeLWP_attach_()
         * The code that handles this in attach will notice that the process
         * is in a SIGSTOP'd state and try to clear that state, but we may
         * be attached for a brief amount of time before we manage to clear
         * that state.  In the meantime, we may recieve 'signal 0' notices.
         * These aren't real signals, so we'll just drop them on the ground.
         **/
         startup_printf("[%s:%u] - Got a signal 0 from waitpid on %d\n",
                        FILE__, __LINE__, result);
         continue;
      }


      if (me == event_owner || result == -1) {
         //We got an event for ourselves, Yea!  Let's go ahead and return it.
         //We broadcast here to let other people know that someone else needs
         //to take over as a the guy on the waitpid.
         proccontrol_printf("[%s:%u] Got event for ourself (%d), result = %d\n",
                            FILE__, __LINE__, me->getPid(), result);
         waiter_exists = 0;
         pthread_cond_broadcast(&waiter_condvar);
         pthread_mutex_unlock(&waiter_mutex);
         return result;
      }

      waitpid_ret_pair ev;
      ev.status = *status;
      ev.pid = result;
      
      if (event_owner) {
         //This event belongs to someone else.  Let's put it into
         // their queue so that they can get it next time they call
         // this function.
         int res = enqueueWaitpidValue(ev, event_owner);
         if (res == -1) {
            pthread_cond_broadcast(&waiter_condvar);
            pthread_mutex_unlock(&waiter_mutex);
            return -1;
         }
         pthread_cond_broadcast(&waiter_condvar);
      }
      else {
         //Race condition happened here.  We can start getting events
         // for a process the moment we fork it off, and before we
         // ever know it's PID.  In this case we'll just cache the event
         // and add it to the event's queue when it calls addPidGen.
         proccontrol_printf("[%s:%u] - Caching event for %d\n",
                            FILE__, __LINE__, result);
         unassigned_events.push_back(ev);
      }
   }
   assert(0);
   return -1;
}

bool WaitpidMux::hasFirstTimer(SignalGenerator *me) 
{
   for (unsigned i=0; i<first_timers.size(); i++) 
      if (first_timers[i] == me)
         return true;
   return false;
}

void WaitpidMux::addPidGen(int pid, SignalGenerator *sg) 
{
   proccontrol_printf("[%s:%u] Adding pidgen %d for sg %d\n",
                      FILE__, __LINE__, pid, sg->getPid());

   first_timers.push_back(sg);
   for (unsigned i=0; i<pidgens.size(); i++) 
      assert (pidgens[i].pid != pid);
   

   pid_generator_pair_t new_pidgen;
   new_pidgen.pid = pid;
   new_pidgen.sg = sg;
   pidgens.push_back(new_pidgen);
}

void WaitpidMux::removePidGen(int pid, SignalGenerator *sg) 
{
   bool found = false;

   proccontrol_printf("[%s:%u] Removing pidgen %d for sg %d\n",
                      FILE__, __LINE__, pid, sg->getPid());

   for (unsigned i=0; i<pidgens.size(); i++) {
      if (pidgens[i].pid == pid) {
         assert(pidgens[i].sg == sg);
         assert(!found);
         VECTOR_ERASE(pidgens,i, i);
         found = true;
      }
   }
}

void WaitpidMux::removePidGen(SignalGenerator *sg) 
{
   proccontrol_printf("[%s:%u] Removing all pidgens for sg %d\n",
                      FILE__, __LINE__, sg->getPid());

   for (unsigned i=0; i<pidgens.size(); i++) {
      if (pidgens[i].sg == sg) {
         proccontrol_printf("\t[%s:%u] Removing pidgen %d for sg %d\n",
                            FILE__, __LINE__, pidgens[i].pid, sg->getPid());
         VECTOR_ERASE(pidgens,i, i);
         i--;
      }
   }
}

int WaitpidMux::enqueueWaitpidValue(waitpid_ret_pair ev, 
                                    SignalGenerator *event_owner) 
{
   proccontrol_printf("[%s:%u] Giving event to %d\n",
                      FILE__, __LINE__, event_owner->getPid());
   event_owner->event_queue.push_back(ev);
   return 0;
}

/**
 * Strategy:  The program entry point is in /lib/ld-2.x.x at the 
 * _start function.  Get the current PC, parse /lib/ld-2.x.x, and 
 * compare the two points.
 **/
bool process::hasPassedMain() 
{
   if (reachedBootstrapState(initialized_bs))
      return true;

   // We only need to parse /lib/ld-2.x.x once for any process,
   // so just do it once for any process.  We'll cache the result
   // in lib_to_addr.
   static dictionary_hash<std::string, Address> lib_to_addr(::Dyninst::stringhash);
   Symtab *ld_file = NULL;
   bool result, tresult;
   std::string name;
   Address entry_addr, ldso_start_addr;

   //Get current PC
   Frame active_frame = getRepresentativeLWP()->getActiveFrame();
   Address current_pc = active_frame.getPC();

   //Get the current version of /lib/ld-2.x.x
   const char *path = getInterpreterName();
   ldso_start_addr = getInterpreterBase();

   if (!path) {
      //Strange... This shouldn't happen on a normal linux system
      startup_printf("[%s:%u] - Couldn't find /lib/ld-x.x.x in hasPassedMain\n",
                     FILE__, __LINE__);
      result = true;
      goto cleanup;
   }

   if (lib_to_addr.defines(path)) {
      //We've already parsed this library.  Use those results.
      Address start_in_ld = lib_to_addr[path];
      result = (start_in_ld != current_pc);
      goto cleanup;
   }

   //Open /lib/ld-x.x.x and find the entry point
   name = path;
   tresult = Symtab::openFile(ld_file, name);
   if (!tresult) {
      startup_printf("[%s:%u] - Unable to open %s in hasPassedMain\n", 
                     FILE__, __LINE__, path);
      result = true;
      goto cleanup;
   }
   

   entry_addr = ld_file->getEntryOffset();
   if (!entry_addr) {
      startup_printf("[%s:%u] - No entry addr for %s\n", 
                     FILE__, __LINE__, path);
      result = true;
      goto cleanup;
   }
   entry_addr += ldso_start_addr;
   
   lib_to_addr[path] = entry_addr;
   result = (entry_addr != current_pc);
   startup_printf("[%s:%u] - hasPassedMain returning %d (%lx %lx)\n",
                  FILE__, __LINE__, (int) result, entry_addr, current_pc);

   cleanup:

   return result;
}

Address process::setAOutLoadAddress(fileDescriptor &desc) {
   //The load address of the a.out isn't correct.  We can't read a
   // correct one out of ld-x.x.x.so because it may not be initialized yet,
   // and it won't be initialized until we reach main.  But we need the load
   // address to find main.  Darn.
   //
   //Instead we'll read the entry out of /proc/pid/maps, and try to make a good
   // effort to correctly match the fileDescriptor to an entry.  Unfortunately,
   // symlinks can complicate this, so we'll stat the files and compare inodes

   struct stat aout, maps_entry;
   map_entries *maps = NULL;
   unsigned maps_size = 0, i;
   char proc_path[128];
   int result;

   //Get the inode for the a.out
   startup_printf("[%s:%u] - a.out is a shared library, computing load addr\n",
                  FILE__, __LINE__);
   memset(&aout, 0, sizeof(aout));
   proc_path[127] = '\0';
   snprintf(proc_path, 127, "/proc/%d/exe", getPid());
   result = stat(proc_path, &aout);
   if (result == -1) {
      startup_printf("[%s:%u] - setAOutLoadAddress couldn't stat %s: %s\n",
                     FILE__, __LINE__, proc_path, strerror(errno));
      goto done;
   }
                    
   //Get the maps
   maps = getLinuxMaps(getPid(), maps_size);
   if (!maps) {
      startup_printf("[%s:%u] - setAOutLoadAddress, getLinuxMaps return NULL\n",
                     FILE__, __LINE__);
      goto done;
   }
   
   //Compare the inode of each map entry to the a.out's
   for (i=0; i<maps_size; i++) {
      memset(&maps_entry, 0, sizeof(maps_entry));
      result = stat(maps[i].path, &maps_entry);
      if (result == -1) {
         startup_printf("[%s:%u] - setAOutLoadAddress couldn't stat %s: %s\n",
                        FILE__, __LINE__, maps[i].path, strerror(errno));
         continue;
      }
      if (maps_entry.st_dev == aout.st_dev && maps_entry.st_ino == aout.st_ino)
      {
         //We have a match
         desc.setLoadAddr(maps[i].start);
         goto done;
      }
   }
        
 done:
   if (maps)
      free(maps);

   return desc.loadAddr();
}

bool process::readAuxvInfo()
{
   if (auxv_parser)
      return false;

   auxv_parser = AuxvParser::createAuxvParser(getPid(), getAddressWidth());
   if (!auxv_parser) {
      startup_printf("%s[%u]: - ERROR, failed to parse Auxv\n", FILE__, __LINE__);
      vsys_status_ = vsys_notfound;
      return false;
   }
   
   vsyscall_start_ = auxv_parser->getVsyscallBase();
   vsyscall_end_ = auxv_parser->getVsyscallEnd();
   vsyscall_text_ = auxv_parser->getVsyscallText();
   vsys_status_ = vsys_found;
   
   setInterpreterBase(auxv_parser->getInterpreterBase());
   
   return true;
   
   
}

bool BinaryEdit::getResolvedLibraryPath(const std::string &filename, std::vector<std::string> &paths) {
    char *libPathStr, *libPath;
    std::vector<std::string> libPaths;
    struct stat dummy;
    FILE *ldconfig;
    char buffer[512];
    char *pos, *key, *val;

    // prefer qualified file paths
    if (stat(filename.c_str(), &dummy) == 0) {
        paths.push_back(filename);
    }

    // search paths from environment variables
    libPathStr = strdup(getenv("LD_LIBRARY_PATH"));
    libPath = strtok(libPathStr, ":");
    while (libPath != NULL) {
        libPaths.push_back(std::string(libPath));
        libPath = strtok(NULL, ":");
    }
    free(libPathStr);

    //libPaths.push_back(std::string(getenv("PWD")));
    for (unsigned int i = 0; i < libPaths.size(); i++) {
        std::string str = libPaths[i] + "/" + filename;
        if (stat(str.c_str(), &dummy) == 0) {
            paths.push_back(str);
        }
    }

    // search ld.so.cache
    ldconfig = popen("/sbin/ldconfig -p", "r");
    if (ldconfig) {
        fgets(buffer, 512, ldconfig);	// ignore first line
        while (fgets(buffer, 512, ldconfig) != NULL) {
            pos = buffer;
            while (*pos == ' ' || *pos == '\t') pos++;
            key = pos;
            while (*pos != ' ') pos++;
            *pos = '\0';
            while (*pos != '=' && *(pos + 1) != '>') pos++;
            pos += 2;
            while (*pos == ' ' || *pos == '\t') pos++;
            val = pos;
            while (*pos != '\n' && *pos != '\0') pos++;
            *pos = '\0';
            if (strcmp(key, filename.c_str()) == 0) {
                paths.push_back(val);
            }
        }
        pclose(ldconfig);
    }

    // search hard-coded system paths
    libPaths.clear();
    libPaths.push_back("/usr/local/lib");
    libPaths.push_back("/usr/share/lib");
    libPaths.push_back("/usr/lib");
    libPaths.push_back("/usr/lib64");
    libPaths.push_back("/lib");
    libPaths.push_back("/lib64");
    for (unsigned int i = 0; i < libPaths.size(); i++) {
        std::string str = libPaths[i] + "/" + filename;
        if (stat(str.c_str(), &dummy) == 0) {
            paths.push_back(str);
        }
    }

    return ( 0 < paths.size() );
}

bool process::detachForDebugger(const EventRecord &/*crash_event*/) {
   long ret;
   int ptrace_errno;
   if (strcmp(dyn_debug_crash_debugger, "core") == 0)
      return true;
   kill(getPid(), SIGSTOP);
   ret = DBI_ptrace(PTRACE_DETACH, getPid(), 0x0, 0x0, &ptrace_errno,
                    getAddressWidth(), __FILE__, __LINE__);
   if (ret == -1) {
      fprintf(stderr, "PTrace detach failed: %s\n", strerror(ptrace_errno));
   }
   return (ret != -1);
}

bool BinaryEdit::archSpecificMultithreadCapable() {
    /*
     * The heuristic for this check on Linux is that some symbols provided by
     * pthreads are only defined when the binary contains pthreads. Therefore,
     * check for these symbols, and if they are defined in the binary, then
     * conclude that the binary is multithread capable.
     */
    const int NUM_PTHREAD_SYMS = 4;
    const char *pthreadSyms[NUM_PTHREAD_SYMS] = 
        { "pthread_cancel", "pthread_once", 
          "pthread_mutex_unlock", "pthread_mutex_lock" 
        };
    if( mobj->isStaticExec() ) {
        int numSymsFound = 0;
        for(int i = 0; i < NUM_PTHREAD_SYMS; ++i) {
            const pdvector<int_function *> *tmpFuncs = 
                mobj->findFuncVectorByPretty(pthreadSyms[i]);
            if( tmpFuncs != NULL && tmpFuncs->size() ) numSymsFound++;
        }

        if( numSymsFound == NUM_PTHREAD_SYMS ) return true;
    }

    return false;
}
