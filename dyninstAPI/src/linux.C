/*
 * Copyright (c) 1996-2004 Barton P. Miller
 * 
 * We provide the Paradyn Parallel Performance Tools (below
 * described as "Paradyn") on an AS IS basis, and do not warrant its
 * validity or performance.  We reserve the right to update, modify,
 * or discontinue this software at any time.  We shall have no
 * obligation to supply such updates or modifications or any other
 * form of support to you.
 * 
 * This license is for research uses.  For such uses, there is no
 * charge. We define "research use" to mean you may freely use it
 * inside your organization for whatever purposes you see fit. But you
 * may not re-distribute Paradyn or parts of Paradyn, in any form
 * source or binary (including derivatives), electronic or otherwise,
 * to any other organization or entity without our permission.
 * 
 * (for other uses, please contact us at paradyn@cs.wisc.edu)
 * 
 * All warranties, including without limitation, any warranty of
 * merchantability or fitness for a particular purpose, are hereby
 * excluded.
 * 
 * By your use of Paradyn, you understand and agree that we (or any
 * other person or entity with proprietary rights in Paradyn) are
 * under no obligation to provide either maintenance services,
 * update services, notices of latent defects, or correction of
 * defects for Paradyn.
 * 
 * Even if advised of the possibility of such damages, under no
 * circumstances shall we (or any other person or entity with
 * proprietary rights in the software licensed hereunder) be liable
 * to you or any third party for direct, indirect, or consequential
 * damages of any character regardless of type of action, including,
 * without limitation, loss of profits, loss of use, loss of good
 * will, or computer failure or malfunction.  You agree to indemnify
 * us (and any other person or entity with proprietary rights in the
 * software licensed hereunder) for any and all liability it may
 * incur to third parties resulting from your use of Paradyn.
 */

// $Id: linux.C,v 1.199 2006/03/08 19:57:50 mjbrim Exp $

#include <fstream>

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

#include "dyninstAPI/src/symtab.h"
#include "dyninstAPI/src/function.h"
#include "dyninstAPI/src/instPoint.h"
#include "dyninstAPI/src/baseTramp.h"
#include "dyninstAPI/src/signalhandler.h"
#include "dyninstAPI/src/mailbox.h"
#include "dyninstAPI/src/debuggerinterface.h"
#include "common/h/headers.h"
#include "dyninstAPI/src/os.h"
#include "dyninstAPI/src/stats.h"
#include "common/h/Types.h"
#include "dyninstAPI/src/showerror.h"
#include "dyninstAPI/src/util.h" // getCurrWallTime
#include "common/h/pathName.h"
#include "mapped_object.h"
#include "mapped_module.h"

#include "dynamiclinking.h"

#ifndef BPATCH_LIBRARY
#include "common/h/Time.h"
#include "common/h/timing.h"
#include "paradynd/src/init.h"
#endif

#include "Elf_X.h"
#include "dyninstAPI/src/addLibraryLinux.h"
#include "dyninstAPI/src/writeBackElf.h"
// #include "saveSharedLibrary.h" 

#ifdef PAPI
#include "papi.h"
#endif

//These values can be different on different platforms, and don't seem to be
//consistently defined in the system's include files.
#if !defined(PTRACE_SETOPTIONS)
#if defined(arch_x86)
#define PTRACE_SETOPTIONS 21
#elif defined(arch_ia64)
#define PTRACE_SETOPTIONS 0x4200
#endif
#endif

#if !defined(PT_TRACE_EXIT)
#define PTRACE_O_TRACEEXIT 0x40
#endif

#if defined(PTRACEDEBUG) && !defined(PTRACEDEBUG_ALWAYS)
static bool debug_ptrace = false;
#endif



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
  int result = P_kill(getpid(), SIGSTOP);
  if (0 != result) {
    fprintf(stderr, "%s[%d]:  failed to stop child\n", FILE__, __LINE__);
    abort();
  }
}

bool attachToChild(int pid)
{
   //  wait for child process to stop itself, attach to it, and continue
     int wait_options = __WALL | WUNTRACED;
     int status = 0;
     int res = waitpid(pid, &status, wait_options);
     if (res <= 0) {
        fprintf(stderr, "%s[%d]:  waitpid failed\n", FILE__, __LINE__);
        abort();
     }

     if (!WIFSTOPPED(status)) {
       fprintf(stderr, "%s[%d]:  status is not stopped\n", FILE__, __LINE__);
     }
     if ( SIGSTOP != WSTOPSIG(status)) {
       fprintf(stderr, "%s[%d]:  signal is not SIGILL: stopsig = %d\n", FILE__, __LINE__, WSTOPSIG(status));
     }

     if (0 != P_ptrace(PTRACE_ATTACH, pid, 0, 0, 4 )) {
       fprintf(stderr, "%s[%d]:  ptrace (ATTACH) failed\n", FILE__, __LINE__);
       abort();
    }

    if (0 != P_ptrace(PTRACE_CONT, pid, 0, SIGCONT, 4 ) )  {
       perror("ptrace(CONT)");
       fprintf(stderr, "%s[%d]:  ptrace (CONT) failed\n", FILE__, __LINE__);
       abort();
    }

    if (0 > waitpid(pid, NULL, 0)) {
       perror("process::attach - waitpid");
       exit(1);
    }

    if (0 != P_ptrace(PTRACE_CONT, pid, 0, SIGCONT, 4 ) )  {
       perror("ptrace(CONT)");
       fprintf(stderr, "%s[%d]:  ptrace (CONT) failed\n", FILE__, __LINE__);
       abort();
    }

    return true;
}

bool SignalGenerator::decodeEvent(EventRecord &ev)
{
  //  ev.info has the status result from waitpid

   if (ev.type == evtUndefined) {
      if (!decodeWaitPidStatus(ev.info, ev)) 
          fprintf(stderr, "%s[%d][%s]:  failed to decode status for event\n", 
          FILE__, __LINE__, getThreadStr(getExecThreadID()));
   }

   errno = 0;
   if (ev.type == evtSignalled) {
      if (waiting_for_stop) {
         if (suppressSignalWhenStopping(ev)) {
            //  we suppress this signal, just send a null event
            if (ev.what != SIGTRAP) {
               // it's common to suppress traps while waiting for a stop,
               // only report other signals
               char buf[128];
               fprintf(stderr, "%s[%d]:  suppressing signal %s\n", FILE__, __LINE__, ev.sprint_event(buf));
            }
            ev.type = evtNullEvent;
            return true;
         }
      }
      decodeSignal(ev);
   }

   if (ev.type == evtUndefined) {
     //  if we still have evtSignalled, then it must not be a signal that
     //  we care about internally.  Still, send it along to the handler
     //  to be forwarded back to the process.
     char buf[512];
     fprintf(stderr, "%s[%d]:  got event %s, should have been set by now\n", FILE__, __LINE__, ev.sprint_event(buf));
     return true;
   }


   return true;
}

bool get_linux_version(int &major, int &minor, int &subvers)
{
   static int maj = 0, min = 0, sub = 0;
   int result;
   FILE *f;
   if (maj)
   {
      major = maj;
      minor = min;
      subvers = sub;
      return true;
   }
   f = fopen("/proc/version", "r");
   if (!f) goto error;
   result = fscanf(f, "Linux version %d.%d.%d", &major, &minor, &subvers);
   fclose(f);
   if (result != 3) goto error;

   maj = major;
   min = minor;
   sub = subvers;
   return true;

 error:
   //Assume 2.4, which is the earliest version we support
   major = maj = 2;
   minor = min = 4;
   subvers = sub = 0;
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
         attached_lwp_ids.erase(i, i);
         found = true;
      }
   }
   return found;
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

pid_t SignalGenerator::waitpid_kludge(pid_t pid_arg, int *status, int options, int *dead_lwp)
{
  pid_t ret = 0;
  int wait_options = options | WNOHANG;

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

    ret = waitpid(pid_arg, status, wait_options); 
  } while (ret == 0);

  return ret; 
}

bool SignalGenerator::waitNextEventLocked(EventRecord &ev) 
{
  signal_printf("%s[%d]:  welcome to waitNextEventLocked\n", FILE__, __LINE__);

  assert(proc);

  int waitpid_pid = 0;
  int status = 0;
  int dead_lwp = 0;

  //  If we have a rep lwp, the process is not multithreaded, so just wait for 
  //  the pid.  If the process is MT, wait for (-pid), which waits for all process
  //  with thread group <pid>

  int pid_to_wait_for = proc->getRepresentativeLWP() ? pid : -1 * pid;

   //  wait for process events, on linux __WALL signifies that both normal children
   //  and cloned children (lwps) should be listened for.
   //  Right now, we are blocking.  To make this nonblocking, or this val with WNOHANG.

   //  __WNOTHREAD signifies that children of other threads (other signal handlers)
   //  should not be listened for.

  int wait_options = __WALL;

  waiting_for_event = true;
  __UNLOCK;
  waitpid_pid = waitpid_kludge( pid_to_wait_for /* -1 for any child*/, 
                                &status, wait_options, &dead_lwp );
  __LOCK;
  waiting_for_event = false;
  if (WIFSTOPPED(status))
     proccontrol_printf("waitpid - %d stopped with %d\n", waitpid_pid, 
                        WSTOPSIG(status));

  if (waitpid_pid < 0 && errno == ECHILD) {
     fprintf(stderr, "%s[%d]:  waitpid failed with ECHILD\n", __FILE__, __LINE__);
     //assert(0);
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
  
  if (stop_request) {
     ev.type = evtShutDown;
     ev.proc = proc;
     return true;
  }

  ev.proc = proc;
  ev.lwp = proc->lookupLWP(waitpid_pid);
  ev.type = evtUndefined;

  assert(ev.lwp);
  assert(ev.lwp->proc() == proc);
  
  if (!ev.lwp->is_attached()) {
     //  Hijack thread detection events here (very platform specific)
     ev.type = evtThreadDetect;
     return true;
  }

  assert(ev.lwp);
  ev.proc->set_lwp_status(ev.lwp, stopped);
  ev.type = evtUndefined;
  ev.info = status;
   
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

  if (!decodeEvent(ev)) {
     char buf[128];
     fprintf(stderr, "%s[%d]:  FIXME: %s\n", __FILE__, __LINE__, ev.sprint_event(buf));
     return false;
  }

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


void emitCallRel32(unsigned disp32, unsigned char *&insn);

static int lwp_kill(int pid, int sig)
{
  int result = P_tkill(pid, sig);
  if (result == -1 && errno == ENOSYS)
  {
     result = P_kill(pid, sig);
     proccontrol_printf("%s[%d]: Sent %d to %d via kill\n", FILE__, __LINE__, 
                        sig, pid);
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
  
  while (*status != '\0' && *(status++) != '(');
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

bool SignalGenerator::suppressSignalWhenStopping(EventRecord &ev)
{
 
  bool suppressed_something = false;

  if ( ev.what == SIGSTOP )
     return suppressed_something;

  /* If we're suppressing signals, regenerate them at the end of the
     queue to avoid race conditions. */
  if( ev.what == SIGTRAP ) {
#if defined(arch_x86)
     Address trap_pc = ev.lwp->getActiveFrame().getPC();
     ev.lwp->changePC(trap_pc - 1, NULL);
#elif defined(arch_ia64)
     /* DEBUG */ fprintf( stderr, "%s[%d]: IA-64 regenerating suppressed SIGTRAP.\n", __FILE__, __LINE__ );
     Address trapAddr = ev.lwp->getActiveFrame().getPC();
     Address preTrapAddr = trapAddr;
     uint64_t slotNo = trapAddr % 16;
     assert( slotNo < 3 );
     if( slotNo == 0 ) {
     	fprintf( stderr, "%s[%d]: FIXME: test for long instruction in previous bundle.\n", __FILE__, __LINE__ );
        preTrapAddr -= 16;
        slotNo = 2;
        }
    else { --slotNo; }
    preTrapAddr = (preTrapAddr % 16) + slotNo;
 
    /* DEBUG */ fprintf( stderr, "%s[%d]: 0x%lx -> 0x%lx\n", __FILE__, __LINE__, trapAddr, trapAddr );
    ev.lwp->changePC( preTrapAddr, NULL );
#endif /* defined( arch_ia64 ) */
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
     return suppressed_something;
  }

  if ( ev.what != SIGTRAP ) {
     suppressed_sigs.push_back(ev.what);
     suppressed_lwps.push_back(ev.lwp);
  }
  suppressed_something = true;

  return suppressed_something;
}

bool SignalGenerator::resendSuppressedSignals()
{
  assert(suppressed_sigs.size() == suppressed_lwps.size());
  for (unsigned int i = 0; i < suppressed_sigs.size(); ++i)
  {
    fprintf(stderr, "%s[%d]:  resending %d to %d via lwp_kill\n", FILE__, __LINE__,
            suppressed_sigs[i], suppressed_lwps[i]->get_lwp_id());
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
   if(signalToContinueWith != dyn_lwp::NoSignal) {
      arg3 = 1;
      arg4 = signalToContinueWith;
   }

   if (! ignore_suppress) {
      if (proc()->sh->waitingForStop())
      {
         fprintf(stderr, "%s[%d]:  suppressing continue\n", FILE__, __LINE__);
         return false;
      }
   }

   if (status() == exited)
      return true;

   ptraceOps++; 
   ptraceOtherOps++;

   int ptrace_errno = 0;
   int ret = DBI_ptrace(PTRACE_CONT, get_lwp_id(), arg3, arg4, &ptrace_errno, proc_->getAddressWidth(),  FILE__, __LINE__);
   if (ret == 0)
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
  if (status() == stopped)
  {
    return true;
  }

  //return (doWaitUntilStopped(proc(), get_lwp_id(), true) != NULL);
  SignalGenerator *sh = (SignalGenerator *) proc()->sh;
  sh->setWaitingForStop(true);
  while ( status() != stopped) {
    signal_printf("%s[%d]:  before waitForEvent(evtProcessStop): status is %s\n",
            FILE__, __LINE__, getStatusAsString().c_str());
    sh->waitForEvent(evtProcessStop);
  }
  sh->setWaitingForStop(false);
  sh->resendSuppressedSignals();

  return true;
}

bool dyn_lwp::stop_() 
{
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

   while (proc->isRunning_()) {
       if (0 > waitpid(getPid(), NULL, 0)) {
         //  should do some better checking here....
         perror("waitpid");
         return false;
       }
   }
   
   return true;
}

bool process::stop_(bool waitUntilStop)
{
  int result;
  
  //Stop the main process
  result = P_kill(getPid(), SIGSTOP);
  if (result == -1) 
  {
    perror("Couldn't send SIGSTOP\n");
    return false;
  }

  if (waitUntilStop) {
    if (!waitUntilLWPStops()) 
       return false;
    if (status() == exited)
       return false;
  }

  //Stop all other LWPs
  dictionary_hash_iter<unsigned, dyn_lwp *> lwp_iter(real_lwps);
  unsigned index = 0;
  dyn_lwp *lwp;
  
  while(lwp_iter.next(index, lwp))
  {
    lwp->pauseLWP(waitUntilStop);
  }

  return true;
}

bool process::waitUntilStopped()
{
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

  ((SignalGenerator *)sh)->setWaitingForStop(true);
  while ( status() != stopped) {
    if (status() == exited) {
      return false;
    }
    signal_printf("%s[%d][%s]:  before waitForEvent(evtProcessStop)\n", 
            FILE__, __LINE__, getThreadStr(getExecThreadID()));
    sh->waitForEvent(evtProcessStop);
  }

  ((SignalGenerator *)sh)->setWaitingForStop(false);
  ((SignalGenerator *)sh)->resendSuppressedSignals();

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
  else
    return terminateSucceeded;
}

void dyn_lwp::realLWP_detach_() 
{
   if(! proc_->isAttached()) {
      if (! proc_->hasExited())
         cerr << "Detaching, but not attached" << endl;
      return;
   }
    
    cerr <<"Detaching..." << endl;
    ptraceOps++;
    ptraceOtherOps++;
    int ptrace_errno = 0;
    int ptrace_ret = DBI_ptrace(PTRACE_DETACH, get_lwp_id(),1, SIGCONT, &ptrace_errno, proc_->getAddressWidth(),  __FILE__, __LINE__); 
    if (ptrace_ret < 0) {
      fprintf(stderr, "%s[%d]:  ptrace failed: %s\n", __FILE__, __LINE__, strerror(ptrace_errno));
    }
    proc()->sh->remove_lwp_from_poll_list(get_lwp_id());
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
    return;
}

bool process::dumpCore_(const pdstring/* coreFile*/) { return false; }

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

bool dyn_lwp::readTextSpace(void *inTraced, u_int amount, const void *inSelf) {
   return readDataSpace(inTraced, amount, const_cast<void*>( inSelf ));
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

   if (0 == nbytes)
      return true;

   if ((cnt = ((Address)ap) % len)) {
      /* Start of request is not aligned. */
      unsigned char *p = (unsigned char*) &w;

      /* Read the segment containing the unaligned portion, edit
         in the data from DP, and write the segment back. */
      errno = 0;
      w = P_ptrace(PTRACE_PEEKTEXT, pid, (Address) (ap-cnt), 0);

      if (errno) {
         fprintf(stderr, "%s[%d]:  write data space failing, pid %d\n", __FILE__, __LINE__, pid);
         return false;
      }

      for (unsigned i = 0; i < len-cnt && i < nbytes; i++)
         p[cnt+i] = dp[i];

      if (0 > P_ptrace(PTRACE_POKETEXT, pid, (Address) (ap-cnt), w)) {
         fprintf(stderr, "%s[%d]:  write data space failing\n", __FILE__, __LINE__);
         return false;
      }

      if (len-cnt >= nbytes)
         return true; /* done */

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
         return false;
      }


      for (unsigned i = 0; i < nbytes; i++)
         p[i] = dp[i];

      if (0 > P_ptrace(PTRACE_POKETEXT, pid, (Address) ap, w)) {
         fprintf(stderr, "%s[%d]:  write data space failing\n", __FILE__, __LINE__);
         return false;
      }

   }

   return true;

}

bool dyn_lwp::writeDataSpace(void *inTraced, u_int nbytes, const void *inSelf)
{
   unsigned char *ap = (unsigned char*) inTraced;
   const unsigned char *dp = (const unsigned char*) inSelf;

   //fprintf(stderr, "%s[%d]:  welcome to dyn_lwp::writeDataSpace(%d bytes)\n", __FILE__, __LINE__, nbytes);
   //cerr << "writeDataSpace pid=" << getPid() << ", @ " << (void *)inTraced
   //    << " len=" << nbytes << endl; cerr.flush();

#if defined(BPATCH_LIBRARY)
#if defined(i386_unknown_linux2_0) \
 || defined(x86_64_unknown_linux2_4) /* Blind duplication - Ray */
   if (proc_->collectSaveWorldData) {
       codeRange *range = NULL;
	proc_->codeRangesByAddr_.precessor((Address)inTraced, range); //findCodeRangeByAddress((Address)inTraced);
	if(range){
	        mapped_object *mappedobj_ptr = range->is_mapped_object();
	       if (mappedobj_ptr) {
        	   // If we're writing into a shared object, mark it as dirty.
	           // _Unless_ we're writing "__libc_sigaction"
        	   int_function *func = range->is_function();
	           if ((! func) || (func->prettyName() != "__libc_sigaction")){
        	      mappedobj_ptr->setDirty();
		   }
	       }
	}
   }
#endif
#endif

   ptraceOps++; ptraceBytes += nbytes;

   if (!DBI_writeDataSpace(get_lwp_id(), (Address) ap, nbytes, (Address) dp, sizeof(Address), __FILE__, __LINE__)) {
     fprintf(stderr, "%s[%d]:  bulk ptrace failed\n", __FILE__, __LINE__);
     return false;
   }
   return true;
}


bool DebuggerInterface::bulkPtraceRead(void *inTraced, u_int nelem, void *inSelf, int pid, int address_width) 
{

     u_int nbytes = nelem;
     const unsigned char *ap = (const unsigned char*) inTraced; 
     unsigned char *dp = (unsigned char*) inSelf;
     Address w = 0x0;               /* ptrace I/O buffer */
     int len = address_width; /* address alignment of ptrace I/O requests */
     unsigned cnt;
         
     ptraceOps++; ptraceBytes += nbytes;
      
     if (0 == nbytes)
          return true;
      
     if ((cnt = ((Address)ap) % len)) {
          /* Start of request is not aligned. */
          unsigned char *p = (unsigned char*) &w;
   
          /* Read the segment containing the unaligned portion, and
             copy what was requested to DP. */
          errno = 0;
          w = P_ptrace(PTRACE_PEEKTEXT, pid, (Address) (ap-cnt), w, len);
          if (errno) {
               signal_printf("%s[%d]:  ptrace failed: %s\n", FILE__, __LINE__, strerror(errno));
               return false;
          }
          for (unsigned i = 0; i < len-cnt && i < nbytes; i++)
               dp[i] = p[cnt+i];

          if (len-cnt >= nbytes)
               return true; /* done */

          dp += len-cnt;
          ap += len-cnt;
          nbytes -= len-cnt;
     }
   /* Copy aligned portion */
     while (nbytes >= (u_int)len) {
          errno = 0;
          w = P_ptrace(PTRACE_PEEKTEXT, pid, (Address) ap, 0, len);
          if (errno) {
              signal_printf("%s[%d]:  ptrace failed: %s\n", FILE__, __LINE__, strerror(errno));
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
          w = P_ptrace(PTRACE_PEEKTEXT, pid, (Address) ap, 0, len);
          if (errno) {
               signal_printf("%s[%d]:  ptrace failed: %s\n", FILE__, __LINE__, strerror(errno));
               return false;
          }
          for (unsigned i = 0; i < nbytes; i++)
               dp[i] = p[i];
     }
     return true;



}

bool dyn_lwp::readDataSpace(const void *inTraced, u_int nbytes, void *inSelf) {
     const unsigned char *ap = (const unsigned char*) inTraced;
     unsigned char *dp = (unsigned char*) inSelf;
     int len = proc_->getAddressWidth(); /* address alignment of ptrace I/O requests */

     ptraceOps++; ptraceBytes += nbytes;

     bool ret = false;
     if (! (ret = DBI_readDataSpace(get_lwp_id(), (Address) ap, nbytes,(Address) dp, /*sizeof(Address)*/ len,__FILE__, __LINE__))) {
        signal_printf("%s[%d]:  bulk ptrace read failed for lwp id %d\n",
                      __FILE__, __LINE__, get_lwp_id());
       return false;
     }
     return true;
}

// You know, /proc/*/exe is a perfectly good link (directly to the inode) to
// the executable file, who cares where the executable really is, we can open
// this link. - nash
pdstring process::tryToFindExecutable(const pdstring & /* iprogpath */, int pid) {
  // We need to dereference the /proc link.
  // Case 1: multiple copies of the same file opened with multiple
  // pids will not match (and should)
  // Case 2: an exec'ed program will have the same /proc path,
  // but different program paths
  pdstring procpath = pdstring("/proc/") + pdstring(pid) + pdstring("/exe");
  char buf[1024];
  int chars_read = readlink(procpath.c_str(), buf, 1024);
  if (chars_read == -1) {
    // Note: the name could be too long. Not handling yet.
    return procpath;
  }
  buf[chars_read] = 0;
  return pdstring(buf);
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
     if (direntry->d_name[0] != '.')
        continue;
     unsigned lwp_id = atoi(direntry->d_name+1);
     int lwp_ppid;
     if (!lwp_id) 
        continue;
     sprintf(name, "/proc/%d/stat", lwp_id);
     FILE *fd = fopen(name, "r");
     if (!fd)
        continue;
     fscanf(fd, "%*d %*s %*c %d", &lwp_ppid);
     fclose(fd);
     if (lwp_ppid != getPid())
        continue;
     lwp_ids.push_back(lwp_id);
  }
  closedir(dirhandle);
  lwp_ids.push_back(getPid());
  
  return true;
}

#if !defined(BPATCH_LIBRARY)
#ifdef PAPI
papiMgr* dyn_lwp::papi() {

  return proc()->getPapiMgr();

}
#endif
#endif


#ifdef NOTDEF // PDSEP
#if !defined(BPATCH_LIBRARY)

rawTime64 dyn_lwp::getRawCpuTime_hw()
{
  rawTime64 result = 0;
  
#ifdef PAPI
  result = papi()->getCurrentVirtCycles();
#endif
  
  if (result < hw_previous_) {
    logLine("********* time going backwards in paradynd **********\n");
    result = hw_previous_;
  }
  else 
    hw_previous_ = result;
  
  return result;
}

rawTime64 dyn_lwp::getRawCpuTime_sw()
{
  rawTime64 result = 0;
  int bufsize = 150;
  unsigned long utime, stime;
  char procfn[bufsize], *buf;

  sprintf( procfn, "/proc/%d/stat", get_lwp_id());

  int fd;

  // The reason for this complicated method of reading and sseekf-ing is
  // to ensure that we read enough of the buffer 'atomically' to make sure
  // the data is consistent.  Is this necessary?  I *think* so. - nash
  do {
    fd = P_open(procfn, O_RDONLY, 0);
    if (fd < 0) {
      perror("getInferiorProcessCPUtime (open)");
      return false;
    }

    buf = new char[ bufsize ];

    if ((int)P_read( fd, buf, bufsize-1 ) < 0) {
      perror("getInferiorProcessCPUtime");
      return false;
    }

	/* While I'd bet that any of the numbers preceding utime and stime could overflow 
	   a signed int on IA-64, the compiler whines if you add length specifiers to
	   elements whose conversion has been surpressed. */
    if(2==sscanf(buf,"%*d %*s %*c %*d %*d %*d %*d %*d %*u %*u %*u %*u %*u %lu %lu "
		 , &utime, &stime ) ) {
      // These numbers are in 'jiffies' or timeslices.
      // Oh, and I'm also assuming that process time includes system time
      result = static_cast<rawTime64>(utime) + static_cast<rawTime64>(stime);
      break;
    }

    delete [] buf;
    bufsize = bufsize * 2;

    P_close( fd );
  } while ( true );

  delete [] buf;
  P_close(fd);

  if (result < sw_previous_) {
    logLine("********* time going backwards in paradynd **********\n");
    result = sw_previous_;
  }
  else 
    sw_previous_ = result;

  return result;
}
#endif

#endif // NOTDEF // PDSEP

bool process::dumpImage( pdstring imageFileName ) {
	/* What we do is duplicate the original file,
	   and replace the copy's .text section with
	   the (presumably instrumented) in-memory
	   executable image.  Note that we don't seem
	   to be concerned with making sure that we
	   actually grab the instrumentation code itself... */
	
	/* Acquire the filename. */
   if (!mapped_objects.size()) {
      return false;
   }

   pdstring originalFileName = mapped_objects[0]->fullName();
	
	/* Use system() to execute the copy. */
	pdstring copyCommand = "cp " + originalFileName + " " + imageFileName;
   system( copyCommand.c_str() );

   /* Open the copy so we can use libelf to find the .text section. */
   int copyFD = open( imageFileName.c_str(), O_RDWR, 0 );
   if( copyFD < 0 ) { return false; }
   
   /* Start up the elven widgetry. */
   Elf_X elf( copyFD, ELF_C_READ );
   if (!elf.isValid()) return false;
   
   /* Acquire the shared names pointer. */
   Elf_X_Shdr elfSection = elf.get_shdr( elf.e_shstrndx() );
   Elf_X_Data elfData = elfSection.get_data();
   const char *sharedNames = elfData.get_string();

   /* Iterate over the sections to find the text section's
      offset, length, and base address. */
   Address offset = 0;
   Address length = 0;
   Address baseAddr = 0;
   
   for( int i = 0; i < elf.e_shnum(); ++i ) {
      elfSection = elf.get_shdr( i );
      const char * name = (const char *) &sharedNames[ elfSection.sh_name() ];
      
      if( P_strcmp( name, ".text" ) == 0 ) {
         offset = elfSection.sh_offset();
         length = elfSection.sh_size();
         baseAddr = elfSection.sh_addr();
         break;
      } /* end if we've found the text section */
   } /* end iteration over sections */

   /* Copy the code out of the mutatee. */
   char * codeBuffer = (char *)malloc( length );
   assert( codeBuffer != NULL );
   
   if( ! readTextSpace( (void *) baseAddr, length, codeBuffer ) ) {
      free( codeBuffer );
      elf.end();
      P_close( copyFD );
      return false;
   }

   /* Write that code to the image file. */
   lseek( copyFD, offset, SEEK_SET );
   write( copyFD, codeBuffer, length );

   /* Clean up. */
   free( codeBuffer );
   elf.end();
   P_close( copyFD );
   return true;
}

int getNumberOfCPUs()
{
  return sysconf(_SC_NPROCESSORS_ONLN);
}

//Returns true if the function is part of the PLT table
bool isPLT(int_function *f)
{
    const Object &obj = f->mod()->obj()->parse_img()->getObject();
    return obj.is_offset_in_plt(f->ifunc()->getOffset());
}

// findCallee: finds the function called by the instruction corresponding
// to the instPoint "instr". If the function call has been bound to an
// address, then the callee function is returned in "target" and the 
// instPoint "callee" data member is set to pt to callee's int_function.  
// If the function has not yet been bound, then "target" is set to the 
// int_function associated with the name of the target function (this is 
// obtained by the PLT and relocation entries in the image), and the instPoint
// callee is not set.  If the callee function cannot be found, (ex. function
// pointers, or other indirect calls), it returns false.
// Returns false on error (ex. process doesn't contain this instPoint).

int_function *instPoint::findCallee() {

   // Already been bound
   if (callee_) {
      return callee_;
   }  
   if (ipType_ != callSite) {
      // Assert?
      return NULL; 
   }
   if (isDynamicCall()) {
      return NULL;
   }

   assert(img_p_);
   image_func *icallee = img_p_->getCallee(); 
   if (icallee) {
      // Now we have to look up our specialized version
      // Can't do module lookup because of DEFAULT_MODULE...
      const pdvector<int_function *> *possibles = func()->obj()->findFuncVectorByMangled(icallee->symTabName());
      if (!possibles) {
         return NULL;
      }
      for (unsigned i = 0; i < possibles->size(); i++) {
         if ((*possibles)[i]->ifunc() == icallee) {
            callee_ = (*possibles)[i];
            return callee_;
         }
      }
      // No match... very odd
      assert(0);
      return NULL;
   }
   // Do this the hard way - an inter-module jump
   // get the target address of this function
   Address target_addr = img_p_->callTarget();
   if(!target_addr) {
      // this is either not a call instruction or an indirect call instr
      // that we can't get the target address
      return NULL;
   }

   // get the relocation information for this image
   const Object &obj = func()->obj()->parse_img()->getObject();
   const pdvector<relocationEntry> *fbt;
   if(!obj.get_func_binding_table_ptr(fbt)) {
      return NULL; // target cannot be found...it is an indirect call.
   }
  
   Address base_addr = func()->obj()->codeBase();
   // find the target address in the list of relocationEntries
   for(u_int i=0; i < fbt->size(); i++) {
      if((*fbt)[i].target_addr() == target_addr) {
         // check to see if this function has been bound yet...if the
         // PLT entry for this function has been modified by the runtime
         // linker
         int_function *target_pdf = 0;
         if(proc()->hasBeenBound((*fbt)[i], target_pdf, base_addr)) {
            callee_ = target_pdf;
            return callee_;  // target has been bound
         } 
         else {
            pdvector<int_function *> pdfv;
            bool found = proc()->findFuncsByMangled((*fbt)[i].name(), pdfv);
            if(found) {
               assert(pdfv.size());
               callee_ = pdfv[0];
               return callee_;
            }
         }
         break;
      }
   }
   return NULL;
}

bool SignalGeneratorCommon::getExecFileDescriptor(pdstring filename,
                                    int /*pid*/,
                                    bool /*whocares*/,
                                    int &,
                                    fileDescriptor &desc)
{
    desc = fileDescriptor(filename, 
                          0, // code
                          0, // data
                          false); // a.out
    return true;
}

#if defined(USES_DYNAMIC_INF_HEAP)
static const Address lowest_addr = 0x0;
void process::inferiorMallocConstraints(Address near, Address &lo, Address &hi,
			       inferiorHeapType /* type */ )
{
  if (near)
    {
#if !defined(arch_x86_64)
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

void process::inferiorMallocAlign(unsigned &size)
{
     /* 32 byte alignment.  Should it be 64? */
  size = (size + 0x1f) & ~0x1f;
}
#endif

bool dyn_lwp::realLWP_attach_() {
   char procName[128];
   sprintf(procName, "/proc/%d/mem", get_lwp_id());
   fd_ = P_open(procName, O_RDWR, 0);
   if (fd_ < 0)
     fd_ = INVALID_HANDLE_VALUE;

   startup_printf("%s[%d]:  realLWP_attach doing PTRACE_ATTACH to %lu\n", 
                  FILE__, __LINE__, get_lwp_id());
   int ptrace_errno = 0;
   if( 0 != DBI_ptrace(PTRACE_ATTACH, get_lwp_id(), 0, 0, &ptrace_errno, 
                       proc_->getAddressWidth(),  __FILE__, __LINE__) )
   {
      //Any thread could have exited before we attached to it.
      return false;
   }
   
   proc()->sh->add_lwp_to_poll_list(this);

   eventType evt;
   evt = proc()->sh->waitForEvent(evtThreadDetect, proc_, this);
   if (evt != evtThreadDetect) {
      fprintf(stderr, "%s[%d]:  received unexpected event %s\n", 
              __FILE__, __LINE__, eventType2str(evt));
      abort();
   }
   if (proc_->status() == running)
      continueLWP();

   return true;
}

bool dyn_lwp::representativeLWP_attach_() {

   // step 1) /proc open: attach to the inferior process memory file
   char procName[128];
   sprintf(procName, "/proc/%d/mem", (int) proc_->getPid());
   fd_ = P_open(procName, O_RDWR, 0);
   if (fd_ < 0) 
     fd_ = INVALID_HANDLE_VALUE;
   
   bool running = false;
   if( proc_->wasCreatedViaAttach() )
      running = proc_->isRunning_();
   
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
      startup_cerr << "process::attach() doing PTRACE_ATTACH to " <<get_lwp_id() << endl;
      int ptrace_errno = 0;
      int address_width = sizeof(Address);
      assert(address_width);

      if( 0 != DBI_ptrace(PTRACE_ATTACH, getPid(), 0, 0, &ptrace_errno, address_width, __FILE__, __LINE__) )
      {
         startup_printf("%s[%d]:  ptrace attach to pid %d failing\n", FILE__, __LINE__, getPid());
         perror( "process::attach - PTRACE_ATTACH" );
         return false;
      }
      proc_->sh->add_lwp_to_poll_list(this);

#if defined (arch_ia64)
      if (0 > getDBI()->waitpid(getPid(), NULL, 0)) {
         perror("process::attach - waitpid");
         exit(1);
      }
#else
      if (0 > waitpid(getPid(), NULL, 0)) {
         perror("process::attach - waitpid");
         exit(1);
      }
#endif
   }

   if(proc_->wasCreatedViaAttach() )
   {
      // If the process was running, it will need to be restarted, as
      // PTRACE_ATTACH kills it
      // Actually, the attach process contructor assumes that the process is
      // running.  While this is foolish, let's play along for now.
      if( proc_->status() != running || !proc_->isRunning_() ) {
         int ptrace_errno = 0;
         int address_width = sizeof(Address); //proc_->getAddressWidth();
         if( 0 != DBI_ptrace(PTRACE_CONT, getPid(), 0, 0, &ptrace_errno, address_width,  __FILE__, __LINE__) )
         {
            perror( "process::attach - continue 1" );
         }
      }
   }

   if(proc_->wasCreatedViaAttachToCreated())
   {
      // This case is a special situation. The process is stopped
      // in the exec() system call but we have not received the first 
      // TRAP because it has been caught by another process.
      
      /* lose race */
      sleep(1);
      int ptrace_errno = 0; 
      /* continue, clearing pending stop */
      if (0 > DBI_ptrace(PTRACE_CONT, getPid(), 0, SIGCONT, &ptrace_errno, proc_->getAddressWidth(),  __FILE__, __LINE__)) {
         perror("process::attach: PTRACE_CONT 1");
         return false;
      }
     
      if (0 > waitpid(getPid(), NULL, 0)) {
         perror("process::attach: WAITPID");
         return false;
      }
      /* continue, resending the TRAP to emulate the normal situation*/
      if ( 0 > kill(getPid(), SIGTRAP)){
         perror("process::attach: KILL");
         return false;
      }
      
      if (0 > DBI_ptrace(PTRACE_CONT, getPid(), 0, SIGCONT, &ptrace_errno, proc_->getAddressWidth(),  __FILE__, __LINE__)) {
         perror("process::attach: PTRACE_CONT 2");
         return false;
      }

      proc_->set_status(neonatal);
   } // end - if createdViaAttachToCreated

   return true;
}

#define PREMS_PRIVATE (1 << 4)
#define PREMS_SHARED  (1 << 3)
#define PREMS_READ    (1 << 2)
#define PREMS_WRITE   (1 << 1)
#define PREMS_EXEC    (1 << 0)

typedef struct maps_entries {
   Address start;
   Address end;
   unsigned prems;
   Address offset;
   int dev_major;
   int dev_minor;
   int inode;
   char path[512];
} map_entries;

#define LINE_LEN 1024
struct maps_entries *getLinuxMaps(int pid, unsigned &maps_size) {
   char line[LINE_LEN], prems[16], *s;
   int result;
   FILE *f;
   map_entries *maps;
   unsigned i, no_lines = 0;
   
  
   sprintf(line, "/proc/%d/maps", pid);
   f = fopen(line, "r");
   if (!f)
      return NULL;
   
   //Calc num of entries needed and allocate the buffer.  Assume the 
   //process is stopped.
   while (!feof(f)) {
      fgets(line, LINE_LEN, f);
      no_lines++;
   }
   maps = (map_entries *) malloc(sizeof(map_entries) * (no_lines+1));
   if (!maps)
      return NULL;
   result = fseek(f, 0, SEEK_SET);
   if (result == -1)
      return NULL;

   //Read all of the maps entries
   for (i = 0; i < no_lines; i++) {
      if (!fgets(line, LINE_LEN, f))
         break;
      line[LINE_LEN - 1] = '\0';
      sscanf(line, "%lx-%lx %16s %lx %x:%x %u %512s\n", 
             (Address *) &maps[i].start, (Address *) &maps[i].end, prems, 
             (Address *) &maps[i].offset, &maps[i].dev_major,
             &maps[i].dev_minor, &maps[i].inode, maps[i].path);
      maps[i].prems = 0;
      for (s = prems; *s != '\0'; s++) {
         switch (*s) {
            case 'r':
               maps[i].prems |= PREMS_READ;
               break;
            case 'w':
               maps[i].prems |= PREMS_WRITE;
               break;
            case 'x':
               maps[i].prems |= PREMS_EXEC;
               break;
            case 'p':
               maps[i].prems |= PREMS_PRIVATE;
               break;
            case 's':
               maps[i].prems |= PREMS_EXEC;
               break;
         }
      }
   }
   //Zero out the last entry
   memset(&(maps[i]), 0, sizeof(maps_entries));
   maps_size = i;
   
   return maps;
}

// These constants are not defined in all versions of elf.h
#ifndef AT_NULL
#define AT_NULL 0
#endif
#ifndef AT_SYSINFO
#define AT_SYSINFO 32
#endif
#ifndef AT_SYSINFO_EHDR
#define AT_SYSINFO_EHDR 33
#endif
#ifndef PTRACE_GET_THREAD_AREA
#define PTRACE_GET_THREAD_AREA 25
#endif 

static bool couldBeVsyscallPage(map_entries *entry, bool strict, Address pagesize) {
   if (strict) {
       if (entry->prems != PREMS_PRIVATE)
         return false;
      if (entry->path[0] != '\0')
         return false;
      if (((entry->end - entry->start) / pagesize) > 0xf)
         return false;
   }
   if (entry->offset != 0)
      return false;
   if (entry->dev_major != 0 || entry->dev_minor != 0)
      return false;
   if (entry->inode != 0)
      return false;

   return true;
}


bool process::readAuxvInfo()
{
  /**
   * The location of the vsyscall is stored in /proc/PID/auxv in Linux 2.6.
   * auxv consists of a list of name/value pairs, ending with the AT_NULL
   * name.  There isn't a direct way to get the vsyscall info on Linux 2.4
   **/
  char buffer[32];
  int fd;
  Address dso_start = 0x0, text_start = 0x0;
  unsigned page_size = 0x0;
  struct {
    int type;
    Address value;
  } auxv_entry;

  if (vsys_status_ != vsys_unknown) {
     // If we've already found the vsyscall page, just return.
     // True if it's used, false if it's not.
     return !(vsys_status_ == vsys_unused);
  }
  
  /**
   * Try to read from /proc/%d/auxv.  On Linux 2.4 systems auxv
   * doesn't exist, which is okay because vsyscall isn't used.
   * On latter 2.6 kernels the AT_SYSINFO field isn't present,
   * so we have to resort to more "extreme" measures.
   **/
  sprintf(buffer, "/proc/%d/auxv", getPid());
  fd = open(buffer, O_RDONLY);
  if (fd != -1) {
     //Try to read the location out of /proc/pid/auxv
     do {
        read(fd, &auxv_entry, sizeof(auxv_entry));
        if (auxv_entry.type == AT_SYSINFO)
           text_start = auxv_entry.value;
        else if (auxv_entry.type == AT_SYSINFO_EHDR)
           dso_start = auxv_entry.value;
        else if (auxv_entry.type == AT_PAGESZ)
           page_size = auxv_entry.value;
     } while (auxv_entry.type != AT_NULL);
     P_close(fd);
  }

 /**
   * Even if we found dso_start in /proc/pid/auxv, the vsyscall 'page'
   * can be larger than a single page.  Thus we look through /proc/pid/maps
   * for known, default, or guessed start address(es).
   **/
  pdvector<Address> guessed_addrs;
  
  /* The first thing to check is the auxvinfo, if we have any. */
  if( dso_start != 0x0 ) { guessed_addrs.push_back( dso_start ); }
    
 /**
   * We'll make several educated attempts at guessing an address
   * for the vsyscall page.  After deciding on a guess, we'll try to
   * verify that using /proc/pid/maps.
   **/

#if defined(arch_x86)
  // On x86 we can try to read the vsyscall's entry point out of
  // %gs:0x10.  We can use that address to map into /proc/pid/maps
  // and find the vsyscall dso.  This seems to work at the moment,
  // but I wouldn't be surprised if it breaks some day.
  if (!reachedBootstrapState(loadedRT_bs) && dso_start == 0x0) {
     //We haven't initialized yet, leave the status set to unknown, 
     // so we'll try again latter.
     vsys_status_ = vsys_unknown;
     return false;
  }
  pdvector<int_variable *> vars;
  Address g_addr;
  if (findVarsByAll("DYNINST_sysEntry", vars) && vars.size() > 0) {
     readDataSpace((void *) vars[0]->getAddress(), sizeof(Address), 
                   &g_addr, false);
     if (g_addr)
        guessed_addrs.push_back(g_addr);
  }
#endif

  /**
   * Guess some constants that we've seen before.
   **/
#if defined(arch_x86) 
  guessed_addrs.push_back(0xffffe000); //Many early 2.6 systems
  guessed_addrs.push_back(0xffffd000); //RHEL4
#elif defined(arch_ia64)
  guessed_addrs.push_back(0xa000000000000000); 
  guessed_addrs.push_back(0xa000000000010000); 
  guessed_addrs.push_back(0xa000000000020000); //Juniper & Hogan
#endif

  /**
   * Look through every entry in /proc/maps, and compare it to every 
   * entry in guessed_addrs.  If a guessed_addr looks like the right
   * thing, then we'll go ahead and call it the vsyscall page.
   **/
  unsigned num_maps;
  maps_entries *secondary_match = NULL;
  maps_entries *maps = getLinuxMaps(getPid(), num_maps);
  for (unsigned i=0; i<guessed_addrs.size(); i++) {
     Address addr = guessed_addrs[i];
     for (unsigned j=0; j<num_maps; j++) {
        map_entries *entry = &(maps[j]);
        if (addr < entry->start || addr >= entry->end)
           continue;

        if (couldBeVsyscallPage(entry, true, page_size)) {
           //We found a possible page using a strict check. 
           // This is really likely to be it.
           vsyscall_start_ = entry->start;
           vsyscall_end_ = entry->end;
           vsyscall_text_ = 0x0;
           vsys_status_ = vsys_found;
           free(maps);
           return true;
        }

        if (!couldBeVsyscallPage(entry, false, page_size)) {
           //We found an entry that loosely looks like the
           // vsyscall page.  Let's hang onto this and return 
           // it if we find nothing else.
           secondary_match = entry;
        }
     }  
  }

  /**
   * There were no hits using our guessed_addrs scheme.  Let's
   * try to look at every entry in the maps table (not just the 
   * guessed addresses), and see if any of those look like a vsyscall page.
   **/
  for (unsigned i=0; i<num_maps; i++) {
     if (couldBeVsyscallPage(&(maps[i]), true, page_size)) {
        vsyscall_start_ = maps[i].start;
        vsyscall_end_ = maps[i].end;
        vsyscall_text_ = 0x0;
        vsys_status_ = vsys_found;
        free(maps);
        return true;
     }
  }

  /**
   * Return any secondary possiblitiy pages we found in our earlier search.
   **/
  if (secondary_match) {
     vsyscall_start_ = secondary_match->start;
     vsyscall_end_ = secondary_match->end;
     vsyscall_text_ = 0x0;
     vsys_status_ = vsys_found;
     free(maps);
     return true;
  }

  /**
   * Time to give up.  Sigh.
   **/
  vsys_status_ = vsys_notfound;
  free(maps);
  return false;
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
   Address codeBase;
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
static void findThreadFuncs(process *p, pdstring func, 
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
      pdvector<AstNode *> args;
      AstNode call_dummy_create(dummy_create, args);
      AstNode *ast = &call_dummy_create;
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
#if 0
      /*
   //Find functions that are run on pthread exit
   pdvector<int_function *> thread_dest_funcs;
   findThreadFuncs(this, "__pthread_do_exit", &thread_dest_funcs);
   findThreadFuncs(this, "pthread_exit", &thread_dest_funcs);
   findThreadFuncs(this, "deallocate_tsd", &thread_dest_funcs);
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
   //Instrument
   for (i=0; i<thread_dest_funcs.size(); i++)
   {
      pdvector<AstNode *> args;
      AstNode call_thread_destroy(threadDestroy, args);
      AstNode *ast = &call_thread_destroy;
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
      */
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
      fprintf(stderr, "[%s:%d] - Found %d pthread_self functions, expected 1\n",
              __FILE__, __LINE__, pthread_self_funcs.size());
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

   do {
      if(proc()->hasExited()) 
         return (Address) -1;
      proc()->sh->waitForEvent(evtDebugStep);
   } while (singleStepping);

   return getActiveFrame().getPC();
}

// ****** Support linux-specific forkNewProcess DBI callbacks ***** //

bool ForkNewProcessCallback::operator()(pdstring file, 
                    pdstring dir, pdvector<pdstring> *argv,
                    pdvector<pdstring> *envp,
                    pdstring inputFile, pdstring outputFile, int &traceLink,
                    pid_t &pid, int stdin_fd, int stdout_fd, int stderr_fd)
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
    stdin_fd_ = stdin_fd;
    stdout_fd_ = stdout_fd;
    stderr_fd_ = stderr_fd;
    
  startup_printf("%s[%d]:  ForkNewProcessCallback, target thread is %lu(%s)\n", 
__FILE__, __LINE__, targetThread(), getThreadStr(targetThread()));
  //DBIEvent ev(dbiForkNewProcess);
  getMailbox()->executeOrRegisterCallback(this);
  if (synchronous) {
    dbi_printf("%s[%d]:  waiting for completion of callback\n", FILE__, __LINE__
);
    waitForCompletion();
  }
  lock->_Unlock(FILE__, __LINE__);
  return true;
}

bool ForkNewProcessCallback::execute_real()
{
    // call the actual forking routine in unix.C
    ret = forkNewProcess_real(*file_,*dir_,argv_,envp_,*inputFile_,
            *outputFile_,*traceLink_,*pid_,stdin_fd_,stdout_fd_,
            stderr_fd_); 
   
    // attach
    if(ret && !attachToChild(*pid_))
        assert(0 && "failed to ptrace attach to child process");

    return ret;
}

bool DebuggerInterface::forkNewProcess(pdstring file, 
                    pdstring dir, pdvector<pdstring> *argv,
                    pdvector<pdstring> *envp,
                    pdstring inputFile, pdstring outputFile, int &traceLink,
                    pid_t &pid, int stdin_fd, int stdout_fd, int stderr_fd)
{
    dbi_printf("%s[%d][%s]:  welcome to DebuggerInterface::forkNewProcess()\n",
          FILE__, __LINE__, getThreadStr(getExecThreadID()));
    getBusy();

    bool ret;
    ForkNewProcessCallback *fnpp = new ForkNewProcessCallback(&dbilock);
    ForkNewProcessCallback &fnp = *fnpp;

    fnp.enableDelete(false);
    fnp(file, dir, argv, envp, inputFile, outputFile, traceLink, pid,
        stdin_fd, stdout_fd, stderr_fd);

    ret = fnp.getReturnValue();
    fnp.enableDelete();
    releaseBusy();

    return ret;  
}
