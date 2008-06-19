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

// $Id: unix.C,v 1.241 2008/06/19 22:13:43 jaw Exp $

#include <string>
#include "common/h/headers.h"
#include "common/h/Vector.h"
#include "dyninstAPI/src/debug.h"
#include "dyninstAPI/src/os.h"
#include "dyninstAPI/src/unix.h"
#include "dyninstAPI/src/util.h"
#include "dyninstAPI/src/debuggerinterface.h"
#include "dyninstAPI/src/mapped_object.h"
#include "dyninstAPI/src/callbacks.h"
#include "dyninstAPI/src/dynamiclinking.h"
#include "dyninstAPI_RT/h/dyninstAPI_RT.h"  // for DYNINST_BREAKPOINT_SIGNUM


// the following are needed for handleSigChild
#include "dyninstAPI/src/signalhandler.h"
#include "dyninstAPI/src/signalgenerator.h"
#include "dyninstAPI/src/process.h"
#include "dyninstAPI/src/dyn_lwp.h"
#include "dyninstAPI/src/dyn_thread.h"
#include "dyninstAPI/src/instP.h"
#include "common/h/stats.h"


// Forwarding a signal sets BPatch-level shtuff
#include "BPatch.h"
#include "BPatch_process.h"

// BREAK_POINT_INSN
#if defined(os_aix)
#include "dyninstAPI/src/arch-power.h"
#endif

#include <sys/poll.h>

/////////////////////////////////////////////////////////////////////////////
/// Massive amounts of signal handling code
/////////////////////////////////////////////////////////////////////////////


// Turn the return result of waitpid into something we can use
bool decodeWaitPidStatus(procWaitpidStatus_t status,
                        EventRecord &ev) 
{
    // Big if-then-else tree
    if (WIFEXITED(status)) {
        signal_printf("%s[%d]: process exited normally\n", FILE__, __LINE__);
        ev.type = evtProcessExit;
        ev.status = statusNormal;
        ev.what = (eventWhat_t) (unsigned int) WEXITSTATUS(status);
        return true;
    } 
    else if (WIFSIGNALED(status)) {
        ev.type = evtProcessExit;
        ev.status = statusSignalled;
        ev.what = (eventWhat_t) (unsigned int) WTERMSIG(status);
        signal_printf("%s[%d]: process exited via signal %d\n", FILE__, __LINE__, ev.what);
        return true;
    }
    else if (WIFSTOPPED(status)) {
        signal_printf("%s[%d]: process stopped\n", FILE__, __LINE__);
        ev.type = evtSignalled;
        ev.status = statusSignalled;
        ev.what = (eventWhat_t) (unsigned int) WSTOPSIG(status);
        return true;
    }
    else {
        fprintf(stderr, "%s[%d]:  unable to decode waitpid results\n", FILE__, __LINE__);
        ev.type = evtUndefined;
        ev.what = 0;
        return false;
    }
    return false;
}


bool SignalGenerator::decodeSigIll(EventRecord &ev) 
{
#if defined (arch_ia64) 
  if ( ev.proc->getRpcMgr()->decodeEventIfDueToIRPC(ev))
    return true;
#endif
  ev.type = evtCritical;
  return true;
}


 //////////////////////////////////////////////////////////////////
 // Syscall handling
 //////////////////////////////////////////////////////////////////
 // On AIX I've seen a long string of calls to exec, basically
 // doing a (for i in $path; do exec $i/<progname>
 // This means that the entry will be called multiple times
 // until the exec call gets the path right.
bool SignalHandler::handleExecEntry(EventRecord &ev, bool &continueHint) 
{
  bool retval = ev.proc->handleExecEntry((char *)ev.info);
  continueHint = true;
  return retval;
}

#if !defined (os_linux)
bool SignalGenerator::decodeProcStatus(procProcStatus_t status, EventRecord &ev)
{

   ev.info = GETREG_INFO(status.pr_reg);

   switch (status.pr_why) {
      case PR_SIGNALLED:
         ev.type = evtSignalled;
         ev.what = status.pr_what;
         if (!decodeSignal(ev)) {
            char buf[128];
            fprintf(stderr, "%s[%d]:  decodeSignal failed\n", 
                  FILE__, __LINE__, ev.sprint_event(buf));
            return false;
         }
         break;
      case PR_SYSENTRY:
         ev.type = evtSyscallEntry;
         ev.what = status.pr_what;
#if defined(os_aix)
         // We actually pull from the syscall argument vector
         if (status.pr_nsysarg > 0)
            ev.info = status.pr_sysarg[0];
         else
            ev.info = 0;
#endif
         decodeSyscall(ev);
         break;
      case PR_SYSEXIT:
         ev.type = evtSyscallExit;
         ev.what = status.pr_syscall;

#if defined(os_aix)
         // This from the proc header file: system returns are
         // left in pr_sysarg[0]. NOT IN MAN PAGE.
         ev.info = status.pr_sysarg[0];
#endif
         decodeSyscall(ev);
         break;
      case PR_REQUESTED:
         // Because we asked for it... for example:
         // Thread 1: poll for /proc event
         // Thread 2: pause process
         // Thread 1: ... read PR_REQUESTED stop
         ev.type = evtRequestedStop;
         break;

#if defined(PR_SUSPENDED)
      case PR_SUSPENDED:
         // I'm seeing this state at times with a forking multi-threaded
         // child process, currently handling by just continuing the process
         ev.type = evtSuspended;
         break;
#endif
      case PR_JOBCONTROL:
         //  not really sure what this means
         fprintf(stderr, "%s[%d]:  WARNING:  got PR_JOBCONTROL\n", FILE__, __LINE__);
         ev.type = evtSuspended;
         return false;
         break;
      case PR_FAULTED:
         fprintf(stderr, "%s[%d]:  WARNING:  got PR_FAULTED\n", FILE__, __LINE__);
         ev.type = evtCritical;
         return false;
         break;
      default:
         fprintf(stderr, "%s[%d]:  WARNING:  unknown process status: %d\n", FILE__, __LINE__, status.pr_why);
         ev.type = evtSuspended;
         return false;
         break;
   }

   if (ev.type == evtUndefined) {
      fprintf(stderr, "%s[%d]: WARNING:  could not decode event: \n", FILE__, __LINE__);
      fprintf(stderr, "\tpr.what = %d, pr.why = %d\n", status.pr_what, status.pr_why);
      fprintf(stderr, "Thread status flags: 0x%x (STOPPED %d, ISTOP %d, ASLEEP %d)\n",
            status.pr_flags,
            status.pr_flags & PR_STOPPED,
            status.pr_flags & PR_ISTOP,
            status.pr_flags & PR_ASLEEP);
      fprintf(stderr, "Current signal: %d, reason for stopping: %d, (REQ %d, SIG %d, ENT %d, EXIT %d), what %d\n",
            status.pr_cursig, status.pr_why,
            status.pr_why == PR_REQUESTED,
            status.pr_why == PR_SIGNALLED,
            status.pr_why == PR_SYSENTRY,
            status.pr_why == PR_SYSEXIT,
            status.pr_what);
      return false;
   }
   return true;
}
#endif

bool SignalGenerator::decodeSyscall(EventRecord &ev)
{
#if defined (os_aix)
   process *p = ev.proc;
   assert(p);
   int pid = p->getPid();
#endif
   int syscall = (int) ev.what;

   if (syscall == SYSSET_MAP(SYS_fork, pid) ||
         syscall == SYSSET_MAP(SYS_fork1, pid) ||
         syscall == SYSSET_MAP(SYS_vfork, pid)) {
      signal_printf("%s[%d]: decoded fork\n",
            FILE__, __LINE__);
      ev.what = (int) procSysFork;
      return true;
   }
   if (syscall == SYSSET_MAP(SYS_exec, pid) ||
         syscall == SYSSET_MAP(SYS_execv, pid) ||
         syscall == SYSSET_MAP(SYS_execve, pid)) {
      signal_printf("%s[%d]: decoded exec\n",
            FILE__, __LINE__);
      ev.what = (int) procSysExec;
      return true;
   }
   if (syscall == SYSSET_MAP(SYS_exit, pid)) {
      signal_printf("%s[%d]: decoded exit\n",
            FILE__, __LINE__);
      ev.what = (int) procSysExit; 
      return true;
   }
   if (syscall == SYSSET_MAP(SYS_lwp_exit, pid)) {
      signal_printf("%s[%d]: decoded lwp exit\n",
            FILE__, __LINE__);
      ev.type = evtThreadExit;
      ev.what = (int) procLwpExit;

      // Hop forward a bit...
      if (ev.proc->IndependentLwpControl()) {
         ev.proc->set_lwp_status(ev.lwp, exited);
      }

      return true;
   }
   // Don't map -- we make this up
   if (syscall == SYS_load) {
      signal_printf("%s[%d]: decoded load\n",
            FILE__, __LINE__);
      ev.what = procSysLoad;
      return true;
   }

   // Swap the syscall number into the info field
   ev.info = ev.what;
   ev.what = procSysOther;
   return false;
}

bool SignalHandler::handleProcessCreate(EventRecord &ev, bool &continueHint)
{
   process * proc = ev.proc;
   proc->setBootstrapState(begun_bs);
   if (proc->insertTrapAtEntryPointOfMain()) {
      std::string buffer = std::string("PID=") + utos(proc->getPid());
      buffer += std::string(", attached to process, stepping to main");
      statusLine(buffer.c_str());
      continueHint = true;
      return true;
   } else if (proc->getTraceSysCalls()) {
      std::string buffer = std::string("PID=") + utos(proc->getPid());
      buffer += std::string(", attached to process, looking for __libc_start_main");
      statusLine(buffer.c_str());
      continueHint = true;
      return true;
   } else {
      // We couldn't insert the trap... so detach from the process
      // and let it run. 
      fprintf(stderr, "%s[%d][%s]:  ERROR:  couldn't insert at entry of main,\n",
            FILE__, __LINE__, getThreadStr(getExecThreadID()));
      // We should actually delete any mention of this
      // process... including (for Paradyn) removing it from the
      // frontend.
      proc->triggerNormalExitCallback(0);
      continueHint = true;

     // Returning false would send unneeded error messages to the user.
     return true;

   }
   return false;
}

bool SignalHandler::handleThreadCreate(EventRecord &, bool &)
{
   assert(0);
   return false;
}

//  checkForExit returns true when an exit has been detected
bool SignalGenerator::checkForExit(EventRecord &ev, bool block)
{
   int waitpid_flags = block ? 0 : WNOHANG|WNOWAIT;
   int status;

   int retWait = waitpid(getPid(), &status, waitpid_flags);
   if (retWait == -1) {
      fprintf(stderr, "%s[%d]:  waitpid failed\n", FILE__, __LINE__);
      return false;
   }
   else if (retWait > 1) {
      //fprintf(stderr, "%s[%d]:  checkForExit is returning true: pid %d exited, status was %s\n", FILE__, __LINE__, ev.proc->getPid(), ev.proc->getStatusAsString().c_str());
      decodeWaitPidStatus(status, ev);
      ev.proc = proc;
      ev.lwp = proc->getRepresentativeLWP();
      ev.info = 0;
      return true;
   }
   fprintf(stderr, "[%s:%u] - Finished waitpid with %d\n", FILE__, __LINE__, retWait);

   return false;
}

#if !defined (os_linux)

bool SignalGenerator::waitForEventsInternal(pdvector<EventRecord> &events)
{
   assert(events.size() == 0);
   EventRecord ev;
   ev.clear();

   bool ret = true;
   int timeout = POLL_TIMEOUT;
   signal_printf("%s[%d][%s]:  waitNextEvent\n", FILE__, __LINE__, 
         getThreadStr(getExecThreadID()));

   assert(getExecThreadID() == getThreadID());
   assert(getExecThreadID() != primary_thread_id);

   assert(proc);
   assert(proc->getRepresentativeLWP());

   struct pollfd pfds[1]; 

   pfds[0].fd = proc->getRepresentativeLWP()->POLL_FD;
   pfds[0].events = POLLPRI;
   pfds[0].revents = 0;

   waitingForOS_ = true;
   __UNLOCK;
   int num_selected_fds = poll(pfds, 1, timeout);

   signal_printf("%s[%d]: poll returned, acquiring lock...\n", FILE__, __LINE__);

   __LOCK;
   waitingForOS_ = false;

   if (num_selected_fds < 0) {
      ret = false;
#if defined(os_osf)
      //  alpha-osf apparently does not detect process exits from poll events,
      //  so we have to check for exits before calling poll().
      if (checkForExit(ev)) {
         char buf[128];
         signal_printf("%s[%d][%s]:  process exited %s\n", FILE__, __LINE__, 
               getThreadStr(getExecThreadID()), ev.sprint_event(buf));
         ret = true;
         decodeSignal(ev);
         //decodeKludge(ev);
      }
#else
      stopThreadNextIter();
      fprintf(stderr, "%s[%d]:  checkForProcessEvents: poll failed: %s\n", FILE__, __LINE__, strerror(errno));
#endif
      events.push_back(ev);
      return ret;
   } else if (num_selected_fds == 0) {
      //  poll timed out with nothing to report
      signal_printf("%s[%d]:  poll timed out\n", FILE__, __LINE__);
      ev.type = evtTimeout;
      ev.proc = proc;
      events.push_back(ev);
      return true;
   }

   if (!pfds[0].revents) {
      fprintf(stderr, "%s[%d]:  weird, no event for signalled process\n", 
            FILE__, __LINE__);
      return false;
   }

   ev.type = evtUndefined;
   ev.proc = proc;
   ev.lwp = proc->getRepresentativeLWP();
   ev.info  = pfds[0].revents;
   
   if (ev.proc->status() == running) {
      ev.proc->set_status(stopped);
   }

   events.push_back(ev);
  return true;
}

#endif

#if !defined (os_aix)
//  This function is only needed on aix (right now)
bool SignalGenerator::decodeSignal_NP(EventRecord &)
{
  return false;
}
#endif

bool SignalGenerator::decodeSignal(EventRecord &ev)
{

   //char ebuf[128];
   //fprintf(stderr, "%s[%d]:  DECODE SIGNAL: %s\n", FILE__, __LINE__, ev.sprint_event(ebuf));
   //  allow for platform specific decoding of signals, currently only used on
   //  AIX.  If decodeSignal_NP() returns true, the event is fully decoded
   //  so we're done here.

   if (decodeSignal_NP(ev)) {
      return true;
   }

   errno = 0;

   if (ev.type != evtSignalled) {
      char buf[128];
      fprintf(stderr, "%s[%d]:  decodeSignal:  event %s is not a signal event??\n", FILE__, __LINE__,
            ev.sprint_event(buf));
      return false;
   }

   signal_printf("%s[%d]: decoding signal %d\n", FILE__, __LINE__, ev.what);

   //  signal number is assumed to be in ev.what
   switch(ev.what)  {
      case SIGSTOP:
      case SIGINT:
         if (!decodeRTSignal(ev) && !decodeSigStopNInt(ev)) {
            fprintf(stderr, "%s[%d]:  weird, decodeSigStop failed for SIGSTOP\n", FILE__, __LINE__);
         }

         // We have to use SIGSTOP for RTsignals in some cases, since we
         // may not be attached at that point...
         break;
      case SIGTRAP:
         {
            signal_printf("%s[%d]:  SIGTRAP\n", FILE__, __LINE__);
            return decodeSigTrap(ev);
         }
      case SIGILL:
         {
            signal_printf("%s[%d]:  SIGILL\n", FILE__, __LINE__);
#if defined (arch_ia64)
            if (!ev.lwp) {
               fprintf(stderr, "%s[%d]:  CRITICAL SIGNAL\n", FILE__, __LINE__);
               break;
            }

            Frame frame = ev.lwp->getActiveFrame();

            Address pc = frame.getPC();

            if (pc == ev.proc->dyninstlib_brk_addr ||
                  pc == ev.proc->main_brk_addr ||
                  ev.proc->getDyn()->reachedLibHook(pc)) {
               ev.what = SIGTRAP;
               decodeSigTrap(ev);
            }
            else
               decodeSigIll(ev);

            signal_printf("%s[%d]:  SIGILL:  main brk = %p, dyn brk = %p, pc = %p\n",
                  FILE__, __LINE__, ev.proc->main_brk_addr, ev.proc->dyninstlib_brk_addr,
                  pc);
#else

            decodeSigIll(ev);
#endif
            break;
         }

      case DYNINST_BREAKPOINT_SIGNUM: /*SIGBUS2*/
         signal_printf("%s[%d]:  DYNINST BREAKPOINT\n", FILE__, __LINE__);
         ev.type = evtCritical;

         // This may override the type..
         decodeRTSignal(ev);

         break;
      case SIGSEGV:
      case SIGABRT:
         ev.type = evtCritical;
         break;
      case SIGFPE:
         ev.type = evtCritical;
         break;

      default:
         signal_printf("%s[%d]:  got signal %d\n", FILE__, __LINE__, ev.what);
         break;
   }

   return true;
}

bool SignalGeneratorCommon::decodeRTSignal_NP(EventRecord &ev, 
                                              Address rt_arg, int status)
{
   /* Okay... we use both DYNINST_BREAKPOINT_SIGNUM and sigstop,
      depending on what we're trying to stop. So we have to check the 
      flags against the signal
   */
   // This is split into two to make things easier
   if (ev.what == SIGSTOP) {
       // We only use stop on fork...
       if (status != DSE_forkExit) return false;
       // ... of the child
       if (rt_arg != 0) {
          return false;
       }
   }
   else if (ev.what == DYNINST_BREAKPOINT_SIGNUM) {
       if ((status == DSE_forkExit) &&
           (rt_arg == 0)) {
           return false;
       }
   }
   else {
       assert(0);
   }

   ev.info = (eventInfo_t)rt_arg;

   switch(status) {
     case DSE_forkEntry:
        /* Entry to fork */
        ev.type = evtSyscallEntry;
        ev.what = SYS_fork;
        break;
     case DSE_forkExit:
        ev.type = evtSyscallExit;
        ev.what = SYSSET_MAP(SYS_fork, proc->getPid());
        break;
     case DSE_execEntry:
        /* Entry to exec */
        ev.type = evtSyscallEntry;
        ev.what = SYS_exec;
        break;
     case DSE_execExit:
        ev.type = evtSyscallExit;
        ev.what = SYS_exec;
        /* Exit of exec, unused */
        break;
     case DSE_exitEntry:
        /* Entry of exit, used for the callback. We need to trap before
           the process has actually exited as the callback may want to
           read from the process */
        ev.type = evtSyscallEntry;
        ev.what = SYS_exit;
        break;
   case DSE_loadLibrary:
     /* We need to hook this into the shared library handling code... */
        ev.type = evtSyscallExit;
        ev.what = SYS_load;
        break;
   case DSE_lwpExit:
        ev.type = evtSyscallEntry;
        ev.what = SYS_lwp_exit;
        break;
   case DSE_snippetBreakpoint:
        ev.type = evtProcessStop;
        return true;
        break;
   case DSE_stopThread: 
       ev.type = evtStopThread;
       return true; 
   default:
       assert(0);
       break;
   }

   return decodeSyscall(ev);
}


bool SignalGenerator::decodeSigTrap(EventRecord &ev)
{
   char buf[128];
   process *proc = ev.proc;

  //fprintf(stderr, "%s[%d]:  SIGTRAP: %s\n", FILE__, __LINE__, ev.sprint_event(buf));
  if (decodeIfDueToProcessStartup(ev)) {
     signal_printf("%s[%d]:  decodeSigTrap for %s, state: %s\n",
                FILE__, __LINE__, ev.sprint_event(buf), 
                proc->getBootstrapStateAsString().c_str());
     return true;
  }

  Frame af = ev.lwp->getActiveFrame();
  signal_printf("[%s:%u] - Starting decodeSigTrap from trap at 0x%lx\n",
                FILE__, __LINE__, af.getPC());

  // (1)  Is this trap due to an instPoint ??
  if (proc->trapMapping.definesTrapMapping(af.getPC())) {
     ev.type = evtInstPointTrap;
     ev.address = af.getPC();
     return true;
  }

  // (2) Is this trap due to a RPC ??
  if (proc->getRpcMgr()->decodeEventIfDueToIRPC(ev)) {
     signal_printf("%s[%d]:  SIGTRAP due to RPC\n", FILE__, __LINE__);
     return true;
  }

  // (3) Is this trap due to a library being loaded/unloaded ??
  if (proc->isDynamicallyLinked()) {
     if (proc->decodeIfDueToSharedObjectMapping(ev)){
        signal_printf("%s[%d]:  SIGTRAP due to dlopen/dlclose\n", FILE__, __LINE__);
        return true;
     }
  }

#if defined(cap_syscall_trap)
  // (4) Is this an instrumentation-based method of grabbing the exit
  // of a system call?
  if (ev.lwp->decodeSyscallTrap(ev)) {
      // That sets all information....
      return true;
  }
#endif

  // (5) Is this trap due to a single step debugger operation ??
  if (ev.lwp->isSingleStepping()) {
     ev.type = evtDebugStep;
     ev.address = af.getPC();
     signal_printf("Single step trap at %lx\n", ev.address);
     return true;
  }

#if defined (os_linux)
    // (6)  Is this trap due to an exec ??
    // On Linux we see a trap when the process execs. However,
    // there is no way to distinguish this trap from any other,
    // and so it is special-cased here.
    if (proc->nextTrapIsExec) {
        signal_printf("%s[%d]: decoding trap as exec exit\n", FILE__, __LINE__);
        ev.type = evtSyscallExit;
        ev.what = SYS_exec;
        decodeSyscall(ev);
        return true;
    }
#endif

    signal_printf("%s[%d]: decodeSigTrap failing, PC at 0x%lx\n", FILE__, __LINE__, af.getPC());
  return false;
}


// This function is a hollow shell of its former self. It used to check for other things,
// such as iRPC completion. However, iRPCs don't stop for SIGSTOP or SIGINT, and
// were creating false positives when compared with stopping the process before
// the iRPC event was consumed.

bool SignalGenerator::decodeSigStopNInt(EventRecord &ev)
{
   process *proc = ev.proc;

   signal_printf("%s[%d]:  welcome to decodeSigStopNInt for %d, state is %s\n",
         FILE__, __LINE__, ev.proc->getPid(), 
         proc->getBootstrapStateAsString().c_str());

   if (ev.lwp && !ev.lwp->is_attached() && ev.what == SIGSTOP) {
      //The result of a PTRACE_ATTACH
      ev.type = evtLwpAttach;
      return true;
   }

   ev.type = evtProcessStop;

   return true;
}

///////////////////////////////////////////
/////      DebuggerInterface
///////////////////////////////////////////

unsigned long dbi_thread_id = -1UL;
DebuggerInterface *global_dbi = NULL;
DebuggerInterface *getDBI()
{
   if (global_dbi) return global_dbi;
   global_dbi = new DebuggerInterface();
   return global_dbi; 
}

void DBICallbackBase::dbi_signalCompletion(CallbackBase *cbb) 
{
   DBICallbackBase *cb = (DBICallbackBase *) cbb;
   cb->lock->_Lock(FILE__, __LINE__); 
   cb->completion_signalled = true;
   dbi_printf("%s[%d]:  DBICallback, signalling completion\n", FILE__, __LINE__);
   cb->lock->_Broadcast(FILE__, __LINE__); 
   cb->lock->_Unlock(FILE__, __LINE__); 
}

bool DBICallbackBase::waitForCompletion()
{
   assert(lock->depth() == 1);

   getDBI()->evt = type;

   while (!completion_signalled) {
      dbi_printf("%s[%d]:  waiting for completion of callback\n", FILE__, __LINE__);
      if (0 != lock->_Broadcast(FILE__, __LINE__)) assert(0);
      if (0 != lock->_WaitForSignal(FILE__, __LINE__)) assert(0);
   }
   dbi_printf("%s[%d]:  callback completion signalled\n", FILE__, __LINE__);
   return true;
}

bool DebuggerInterface::waitNextEvent(DBIEvent &ev)
{
   isReady = true;
   dbi_printf("%s[%d]:  welcome to waitNextEvent for DebugInterface\n", FILE__, __LINE__);
   dbilock._Lock(FILE__, __LINE__);
   if (evt == dbiUndefined) {
      dbi_printf("%s[%d]:  DebugInterface waiting for something to do\n", FILE__, __LINE__);
      dbilock._WaitForSignal(FILE__, __LINE__);
   }
   //  got something
   ev.type = evt;
   dbi_printf("%s[%d]:  DebuggerInterface got event %s\n", FILE__, __LINE__, 
         dbiEventType2str(evt));
   //       eventType2str(ev.type));
   dbilock._Unlock(FILE__, __LINE__);
   return true;
}

bool DBICallbackBase::execute() {
   return execute_real();
}

bool DebuggerInterface::handleEventLocked(DBIEvent &ev)
{
   assert(dbilock.depth());

   evt = ev.type;

   getMailbox()->executeCallbacks(FILE__, __LINE__);

   //  event is handled, so set evt back to undefined. (our waiting criterion)
   evt = dbiUndefined;
   dbilock._Broadcast(FILE__, __LINE__);
   return true;
}

bool PtraceCallback::operator()(int req, pid_t pid, Address addr,
      Address data, int word_len)
{
   //  No need to copy buffers since dbi callbacks will only be used in
   //  the immediate context of the call;
   lock->_Lock(FILE__, __LINE__);
   req_ = req;
   pid_ = pid;
   addr_ = addr;
   data_ = data;
   word_len_  = word_len;
   ret = (PTRACE_RETURN)0;
   getMailbox()->executeOrRegisterCallback(this);
   if (synchronous) {
      dbi_printf("%s[%d]:  waiting for completion of callback\n", FILE__, __LINE__);
      waitForCompletion();
   }

   lock->_Unlock(FILE__, __LINE__);
   return true;
}

bool PtraceCallback::execute_real()
{
   errno = 0;

   ret = P_ptrace(req_, pid_, addr_, data_, word_len_);
   ptrace_errno = errno;

#if defined(os_linux)
   if (ptrace_errno == ESRCH && req_ == PTRACE_ATTACH) {
      //Handled higher up
      return false;
   }
#endif
   switch (ptrace_errno) {
      case 0:
         break;
      case ESRCH:
         //fprintf(stderr, "... got esrch, returning true anyway\n");
         // Don't report an error... and LWP could have gone away and we don't know
         // about it yet.
         return false;
         break;
      default:
         perror("ptrace error");
         return false;
         break;
   }

   return true;
}

PTRACE_RETURN DebuggerInterface::ptrace(int req, pid_t pid, Address addr, 
      Address data, int word_len, int *ptrace_errno)
{
   dbi_printf("%s[%d][%s]:  welcome to DebuggerInterface::ptrace()\n",
         FILE__, __LINE__, getThreadStr(getExecThreadID()));
   getBusy();

   PTRACE_RETURN ret;
   PtraceCallback *ptp = new PtraceCallback(&dbilock);
   PtraceCallback &pt = *ptp;

   pt.enableDelete(false);
   pt(req, pid, addr, data, word_len);
   ret = pt.getReturnValue();
   if (ptrace_errno) 
      *ptrace_errno = pt.getErrno();
   pt.enableDelete();

   releaseBusy();
   return ret;
}



PTRACE_RETURN DBI_ptrace(int req, pid_t pid, Address addr, Address data, int *ptrace_errno, int word_len,  const char * /* file */, unsigned int /* line */) 
{
   PTRACE_RETURN ret;
   ret = getDBI()->ptrace(req, pid, addr, data, word_len, ptrace_errno);
   return ret;
}

bool WaitPidNoBlockCallback::operator()(int *status)
{
   lock->_Lock(FILE__, __LINE__);
   status_ = status;
   getMailbox()->executeOrRegisterCallback(this);
   //DBIEvent ev(dbiWaitPid);
   if (synchronous) {
      dbi_printf("%s[%d]:  waiting for completion of callback\n", FILE__, __LINE__);
      waitForCompletion();
   }
   lock->_Unlock(FILE__, __LINE__);
   return true;
}

bool WaitPidNoBlockCallback::execute_real() 
{
#if defined (os_linux)
   int wait_options = __WALL | WNOHANG;
   ret = waitpid(-1, status_, wait_options);
#else
   assert(0);
#endif
   return true;
}

int DebuggerInterface::waitpidNoBlock(int *status)
{
   dbi_printf("%s[%d][%s]:  welcome to DebuggerInterface::waitPidNoBlock()\n",
         FILE__, __LINE__, getThreadStr(getExecThreadID()));
   getBusy();

   bool ret;
   WaitPidNoBlockCallback *cbp = new WaitPidNoBlockCallback(&dbilock);
   WaitPidNoBlockCallback &cb = *cbp;

   cb.enableDelete(false);
   cb(status);
   ret = cb.getReturnValue();
   cb.enableDelete();

   releaseBusy();
   return ret;
}

void reportPreloadError(const std::string &msg)
{
   showErrorCallback(101, msg);
   cerr << msg << endl;
}

// Setup the environment for preloading our runtime library
// Modify pnum_entries and envs if not null, putenv otherwise
bool setEnvPreload(unsigned max_entries, char **envs, unsigned *pnum_entries)
{
   unsigned num_entries = *pnum_entries;
   char *rt_lib_name = getenv("DYNINSTAPI_RT_LIB");
   if (rt_lib_name == 0) {
      reportPreloadError(std::string("setEnvPreload: DYNINSTAPI_RT_LIB is "
               "undefined"));
      return false;
   }
   // Check to see if the library given exists.
   if (access(rt_lib_name, R_OK)) {
      std::string msg = std::string("Runtime library ") + std::string(rt_lib_name) +
         std::string(" does not exist or cannot be accessed!");
      reportPreloadError(msg);
      return false;
   }

   const char *var_name = "LD_PRELOAD";
   if (envs != 0) {
      // Check if some LD_PRELOAD is already part of the environment.
      unsigned ivar;
      for (ivar=0; ivar < num_entries &&
            strncmp(envs[ivar], var_name, strlen(var_name)) != 0;
            ivar++) ;
      if (ivar == num_entries) {
         // Not found, append an entry to envs
         std::string ld_preload = std::string(var_name) + std::string("=") +
            std::string(rt_lib_name);
         if (num_entries >= max_entries) {
            reportPreloadError(std::string("setEnvPreload: out of space"));
            return false;
         }
         if ((envs[num_entries++] = P_strdup(ld_preload.c_str())) == 0) {
            reportPreloadError(std::string("setEnvPreload: out of memory"));
            return false;
         }
         envs[num_entries] = NULL;
         *pnum_entries = num_entries;
      }
      else {
         // Found, modify envs in-place
         std::string ld_preload = std::string(envs[ivar]) + std::string(":") + 
            std::string(rt_lib_name);
         if ((envs[ivar] = P_strdup(ld_preload.c_str())) == 0) {
            reportPreloadError(std::string("setEnvPreload: out of memory"));
            return false;
         }
      }
   }
   else {
      // Environment inherited from this process, do putenv
      char *ld_preload_orig = getenv("LD_PRELOAD");
      std::string ld_preload;

      if (ld_preload_orig != 0) {
         // Append to existing var
         ld_preload = std::string(var_name) + std::string("=") +
            std::string(ld_preload_orig) + std::string(":") +
            std::string(rt_lib_name);
      }
      else {
         // Define a new var
         ld_preload = std::string(var_name) + std::string("=") +
            std::string(rt_lib_name);
      }
      char *ld_preload_cstr = P_strdup(ld_preload.c_str());
      if (ld_preload_cstr == 0 ||
          P_putenv(ld_preload_cstr) < 0) {
         reportPreloadError(std::string("setEnvPreload: out of memory"));
         return false;
      }
   }
   return true;
}

/*****************************************************************************
 * forkNewProcess: starts a new process, setting up trace and io links between
 *                the new process and the daemon
 * Returns true if succesfull.
 * 
 * Arguments:
 *   file: file to execute
 *   dir: working directory for the new process
 *   argv: arguments to new process
 *   argp: environment variables for new process
 *   inputFile: where to redirect standard input
 *   outputFile: where to redirect standard output
 *   traceLink: handle or file descriptor of trace link (read only)
 *   pid: process id of new process
 // now an internal unix function, no longer trying to be consistent with
 // Windows 
 // *   tid: thread id for main thread (needed by WindowsNT)
 // *   procHandle: handle for new process (needed by WindowsNT)
 // *   thrHandle: handle for main thread (needed by WindowsNT)
 ****************************************************************************/

bool forkNewProcess_real(std::string file,
                            std::string dir, pdvector<std::string> *argv,
                            pdvector<std::string> *envp,
                            std::string /* inputFile */, std::string /* outputFile */,
                            int &/* traceLink */,
                            pid_t &pid, int stdin_fd, int stdout_fd, int stderr_fd)
{
   forkexec_printf("%s[%d][%s]:  welcome to forkNewProcess(%s)\n",
                   FILE__, __LINE__, getThreadStr(getExecThreadID()), file.c_str());

   errno = 0;

   pid = fork();

   if (pid != 0) {
      // *** parent
      startup_printf("%s[%d][%s]:  ForkNewProcessCallback::execute(%s): " \
                     "FORK PARENT\n", FILE__, __LINE__, 
                     getThreadStr(getExecThreadID()), file.c_str());

#if !defined(alpha_dec_osf4_0)
      /*
       * On Irix, errno sometimes seems to have a non-zero value after
       * the fork even though it succeeded.  For now, if we're using fork
       * and not vfork, we will check the return code of fork to determine
       * if there was error, instead of relying on errno (so make sure the
       * condition for this section of code is the same as the condition for
       * using fork instead of vfork above). - brb
       */
      if (pid == -1)
#else
         if (errno)
#endif
         {
            fprintf(stderr, "%s[%d][%s]:  ForkNewProcessCallback::execute(%s):" \
                    "FORK ERROR\n", FILE__, __LINE__, 
                    getThreadStr(getExecThreadID()), file.c_str());
            sprintf(errorLine, "Unable to start %s: %s\n", file.c_str(), 
                    strerror(errno));
            logLine(errorLine);
            showErrorCallback(68, (const char *) errorLine);
            return false; 
         }

      return true;

   } else if (pid == 0) {
      // *** child

      if (dir.length() > 0)
          P_chdir(dir.c_str());

      if (stdin_fd != 0) dup2(stdin_fd, 0);
      if (stdout_fd != 1) dup2(stdout_fd, 1);
      if (stderr_fd != 2) dup2(stderr_fd, 2);

#ifndef rs6000_ibm_aix4_1
      // define our own session id so we don't get the mutators signals
      setsid();
#endif
      /* indicate our desire to be traced */
      errno = 0;
      OS::osTraceMe();
      if (errno != 0) {
         fprintf(stderr, 
                 "Could perform set PTRACE_TRACEME on forked process\n");
         fprintf(stderr, 
                 " Perhaps your executable doesn't have the exec bit set?\n");
         P__exit(-1);   // double underscores are correct
      }

      char **envs = NULL;
      unsigned num_envs_entries = 0; // not including terminating NULL
      unsigned max_envs_entries = 0;
      if (envp) {
         max_envs_entries = envp->size() + 3;
         // +3: Allocate room for PARADYN_MASTER_INFO, LD_PRELOAD, and NULL
         envs = new char*[max_envs_entries];
         for(unsigned ei = 0; ei < envp->size(); ++ei)
            envs[ei] = P_strdup((*envp)[ei].c_str());
         num_envs_entries = envp->size();
         envs[num_envs_entries] = NULL;
      }
#if (defined(os_linux) && !defined(arch_x86_64)) || defined(os_solaris)
      // Platforms that use LD_PRELOAD. We exclude x86_64 since we do
      // not yet know which kind of the RT lib to load (we determine
      // whether the mutatee is 32 or 64-bit only after starting it).
      if (!setEnvPreload(max_envs_entries, envs, &num_envs_entries)) {
         P__exit(-1);
      }
#endif

      char **args;
      args = new char*[argv->size()+1];
      for (unsigned ai=0; ai<argv->size(); ai++)
         args[ai] = P_strdup((*argv)[ai].c_str());
      args[argv->size()] = NULL;

      startup_printf("%s[%d]:  before exec\n", FILE__, __LINE__);
      startup_printf("%s[%d]:  EXEC: %s\n", FILE__, __LINE__, 
                     file.c_str());
      if (dyn_debug_startup) {
         for (unsigned int ji=0; ji < argv->size(); ji++) {
            fprintf(stderr, "%s ", ((*argv)[ji]).c_str());
         }
      }
      startup_printf("\n");

      if (envp) {
         P_execve(file.c_str(), args, envs);
      }else
         P_execvp(file.c_str(), args);

      fprintf(stderr, "%s[%d]:  exec %s failed, aborting child process\n", __FILE__, __LINE__, file.c_str());
      //P_abort();
      P__exit(-1);
      // not reached
    
      return false;
   }
   return false;
}

#if !defined (os_linux)
bool SignalGenerator::forkNewProcess()
{
    return forkNewProcess_real(file_, dir_, 
                               argv_, envp_, 
                               inputFile_, outputFile_,
                               traceLink_, pid_, 
                               stdin_fd_, stdout_fd_, stderr_fd_);
}
#else
bool SignalGenerator::forkNewProcess()
{
    // Linux platforms MUST execute fork() calls on the debugger interface
   // (ptrace) thread; see comment above DebuggerInterface::forkNewProcess
   // in debuginterface.h for details. -- nater 22.feb.06
   bool result = getDBI()->forkNewProcess(file_, dir_, 
                                          argv_, envp_, 
                                          inputFile_, outputFile_, 
                                          traceLink_, pid_,
                                          stdin_fd_, stdout_fd_, stderr_fd_,
                                          this);
   return result;
}
#endif

#if !defined (os_linux)
//  linux version of this function needs to use waitpid();
bool SignalGenerator::waitForStopInline()
{
   int timeout = 10 /*ms*/;
   assert(proc);
   assert(proc->getRepresentativeLWP());

   while (proc->isRunning_()) {
       struct pollfd pfds[1]; 

       pfds[0].fd = proc->getRepresentativeLWP()->status_fd();
       pfds[0].events = POLLPRI;
       pfds[0].revents = 0;

       poll(pfds, 1, timeout);
   }

   return true;

}
#endif

bool DebuggerInterface::writeDataSpace(pid_t pid, Address addr, int nbytes, Address data, int word_len, const char * /*file*/, unsigned int /*line*/) 
{
  dbi_printf("%s[%d][%s]:  welcome to DebuggerInterface::writeDataSpace()\n",
          FILE__, __LINE__, getThreadStr(getExecThreadID()));
  getBusy();

  bool ret;
  WriteDataSpaceCallback *cbp = new WriteDataSpaceCallback(&dbilock);
  WriteDataSpaceCallback &cb = *cbp;

  cb.enableDelete(false);
  cb(pid, addr, nbytes, data, word_len);
  ret = cb.getReturnValue();
  cb.enableDelete();

  releaseBusy();
  return ret;
}

bool WriteDataSpaceCallback::operator()(pid_t pid, Address addr, int nelem, Address data, int word_len)
{
  lock->_Lock(FILE__, __LINE__);
  pid_ = pid;
  addr_ = addr;
  nelem_ = nelem;
  data_ = data;
  word_len_ = word_len;
  getMailbox()->executeOrRegisterCallback(this);
  //DBIEvent ev(dbiWriteDataSpace);
  if (synchronous) {
    dbi_printf("%s[%d]:  waiting for completion of callback\n", FILE__, __LINE__);
    waitForCompletion();
  }
  lock->_Unlock(FILE__, __LINE__);
  return true;
}

bool WriteDataSpaceCallback::execute_real()
{
#if defined (os_linux)
  ret =  getDBI()->bulkPtraceWrite((void *)addr_, nelem_, (void *)data_, pid_, word_len_);
#else
  assert(0);
#endif
  return true;
}

#if defined (os_linux)
bool DBI_writeDataSpace(pid_t pid, Address addr, int nelem, Address data, int word_len, const char *file, unsigned int line)
#else
bool DBI_writeDataSpace(pid_t pid, Address addr, int nelem, Address data, int /* word_len */, const char * /* file */, unsigned int /* line */)
#endif
{
#if defined (os_linux)
  dbi_printf("%s[%d]: DBI_writeDataSpace(%d, %p, %d, %p, %d) called from %s[%d]\n", 
            FILE__, __LINE__, pid, (void *) addr, nelem, (void *) data, word_len,file,line);
  return getDBI()->writeDataSpace(pid, addr, nelem, data, word_len, file, line);
#else
  process *p = NULL;
  for (unsigned int i = 0; i < processVec.size(); ++i) {
     if (processVec[i]->getPid() == pid) {
       p = processVec[i];
       break;
     }
  }
  if (!p) {
     fprintf(stderr, "%s[%d]:  no process corresp to pid %d\n", FILE__, __LINE__);
     return false;
  }

  return p->readDataSpace((void *)addr, nelem, (void *)data, true /*display error?*/);
#endif
}

bool ReadDataSpaceCallback::operator()(pid_t pid, Address addr, int nelem, Address data, int word_len)
{
  lock->_Lock(FILE__, __LINE__);
  pid_ = pid;
  addr_ = addr;
  nelem_ = nelem;
  data_ = data;
  word_len_ = word_len;
  getMailbox()->executeOrRegisterCallback(this);
  if (synchronous) {
    dbi_printf("%s[%d]:  waiting for completion of callback\n", FILE__, __LINE__);
    waitForCompletion();
  }
  //DBIEvent ev(dbiReadDataSpace);
  lock->_Unlock(FILE__, __LINE__);
  return true;
}

bool ReadDataSpaceCallback::execute_real()
{
#if defined (os_linux)
  ret =  getDBI()->bulkPtraceRead((void *)addr_, nelem_, (void *)data_, pid_, word_len_);
#else
  assert(0);
#endif
  return true;
}

bool DebuggerInterface::readDataSpace(pid_t pid, Address addr, int nbytes, Address data, int word_len, const char * /*file*/, unsigned int /*line*/) 
{
  dbi_printf("%s[%d][%s]:  welcome to DebuggerInterface::readDataSpace()\n",
          FILE__, __LINE__, getThreadStr(getExecThreadID()));
  getBusy();
  dbi_printf("%s[%d]:  got busy\n", FILE__, __LINE__);

  bool ret;
  ReadDataSpaceCallback *cbp = new ReadDataSpaceCallback(&dbilock);
  ReadDataSpaceCallback &cb = *cbp;

  cb.enableDelete(false);
  cb(pid, addr, nbytes, data, word_len);
  ret = cb.getReturnValue();
  cb.enableDelete();

  releaseBusy();
  return ret;
}

#if defined (os_linux)
bool DBI_readDataSpace(pid_t pid, Address addr, int nelem, Address data, int word_len, const char *file, unsigned int line)
#else
bool DBI_readDataSpace(pid_t pid, Address addr, int nelem, Address data, int /* word_len */, const char *file, unsigned int line)
#endif
{
  bool ret = false;
#if defined (os_linux)
  ret =  getDBI()->readDataSpace(pid, addr, nelem, data, word_len, file, line);
#else
  process *p = NULL;
  for (unsigned int i = 0; i < processVec.size(); ++i) {
     if (processVec[i]->getPid() == pid) {
       p = processVec[i];
       break;
     }
  }
  if (!p) {
     fprintf(stderr, "%s[%d]:  no process corresp to pid %d\n", FILE__, __LINE__);
     return false;
  }

  ret = p->readDataSpace((void *)addr, nelem, (void *)data, true /*display error?*/);
#endif
  if (!ret) {
      signal_printf("%s[%d]:  readDataSpace at %s[%d] failing\n", 
                    FILE__, __LINE__, file, line);
  }

  return ret;
}

bool  OS::osKill(int pid) 
{
  return (P_kill(pid,9)==0);
}

void OS::unlink(char *file) 
{
   if (file)
      unlink(file);
}
void OS::make_tempfile(char *s)
{
   int result;
   result = mkstemp(s);
   if (result != -1)
      close(result);
}

bool OS::execute_file(char *path) {
   int result;
   result = system(path);
   if (result == -1) {
      fprintf(stderr, "%s ",path);
      perror("couldn't be executed");
   }
   return (result != -1);
}

#ifndef CASE_RETURN_STR
#define CASE_RETURN_STR(x) case x: return #x
#endif
const char *dbiEventType2str(DBIEventType t)
{
  switch(t) {
  CASE_RETURN_STR(dbiUndefined);
  CASE_RETURN_STR(dbiForkNewProcess);
  CASE_RETURN_STR(dbiPtrace);
  CASE_RETURN_STR(dbiWriteDataSpace);
  CASE_RETURN_STR(dbiReadDataSpace);
  CASE_RETURN_STR(dbiWaitPid);
  CASE_RETURN_STR(dbiGetFrameRegister);
  CASE_RETURN_STR(dbiSetFrameRegister);
  CASE_RETURN_STR(dbiCreateUnwindAddressSpace);
  CASE_RETURN_STR(dbiDestroyUnwindAddressSpace);
  CASE_RETURN_STR(dbiUPTCreate);
  CASE_RETURN_STR(dbiUPTDestroy);
  CASE_RETURN_STR(dbiInitFrame);
  CASE_RETURN_STR(dbiStepFrame);
  CASE_RETURN_STR(dbiIsSignalFrame);
  CASE_RETURN_STR(dbiWaitpid);
  CASE_RETURN_STR(dbiLastEvent);/* placeholder for the end of the list */
  };
  return "invalid";
}

#include <sys/types.h>
#include <dirent.h>


/// Experiment: wait for the process we're attaching to to be created
// before we return. 

SignalGenerator::SignalGenerator(char *idstr, std::string file, int pid)
    : SignalGeneratorCommon(idstr),
      waiting_for_stop(false)
{

    char buffer[128];
    sprintf(buffer, "/proc/%d", pid);

    setupAttached(file, pid);

    // We wait until the entry has shown up in /proc. I believe this
    // is sufficient; unsure though.

    // Not there after three seconds, give up.
    int timeout = 3;
    int counter = 0;

    DIR *dirName = NULL;
    while ((dirName == NULL) && (counter < timeout)) {
        dirName = opendir(buffer);
        if (!dirName) {
            sleep(1);
        }
        counter++;
    }

    if (dirName)
      closedir(dirName);
} 

void EventRecord::clear() {
    proc = NULL;
    lwp = NULL;
    type = evtUndefined;
    what = 0;
    status = statusUnknown;
    info = 0;
    address = 0;
    fd = 0;
}

// TODO: we need to centralize this code as well. Maybe make continueHint
// a struct to hold signals, or set the signal separately.

bool SignalHandler::forwardSigToProcess(EventRecord &ev, bool &continueHint) 
{
    signal_printf("%s[%d]: forwardSigToProcess\n", FILE__, __LINE__);
    
    // We continue the process here to ensure that the signal gets there

    bool exists = false;
    BPatch_process *bproc = BPatch::bpatch->getProcessByPid(ev.proc->getPid(), &exists);
    if (bproc) {
        setBPatchProcessSignal(bproc, ev.what);
        if (!bproc->isVisiblyStopped)
           sg->overrideSyncContinueState(runRequest);
    }

    bool res = false;

    if(process::IndependentLwpControl()) {
        res = ev.lwp->continueLWP(ev.what);
    } else {
        res = ev.proc->continueProc(ev.what);
    }
    if (res == false) {
        fprintf(stderr, "%s[%d]:  Couldn't forward signal %d to process %d\n",
                FILE__, __LINE__, ev.what, ev.proc->getPid());
        logLine("error  in forwarding  signal\n");
        showErrorCallback(38, "Error  in forwarding  signal");
        return false;
    } 

    // And so don't continue later.
    continueHint = false;
    return true;
}

bool SignalHandler::handleSignalHandlerCallback(EventRecord &ev)
{
    pdvector<CallbackBase *> cbs;
    if (!getCBManager()->dispenseCallbacksMatching(evtSignalled, cbs)) {
        return false;
    }
    printf("Handling signal number 0x%X\n",ev.what);

    //KEVINTODO: need one time code here to call sigaction so we can
    //retrieve the registered signal handler address and trigger a
    //callback, if there is one
    assert(false); // for now
    return false;
}

bool SignalGeneratorCommon::postSignalHandler() 
{
    return true;
}

bool OS::executableExists(const std::string &file) 
{
   struct stat file_stat;
   int stat_result;

   const char *fn = file.c_str();
   stat_result = stat(fn, &file_stat);
   return (stat_result != -1);
}

int_function *dyn_thread::map_initial_func(int_function *ifunc) 
{
    return ifunc;
}

SignalGenerator::~SignalGenerator() 
{
#if defined(os_linux)
   waitpid_mux.unregisterProcess(this);
#endif
}

bool SignalHandler::handleProcessAttach(EventRecord &ev, bool &continueHint) {
    ev.proc->setBootstrapState(initialized_bs);
    if (ev.proc->main_brk_addr) {
       ev.proc->handleTrapAtEntryPointOfMain(ev.lwp);
    }
    continueHint = false;
    return true;
}
