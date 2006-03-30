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

// $Id: unix.C,v 1.175 2006/03/30 01:00:10 bernat Exp $

#include "common/h/headers.h"
#include "common/h/String.h"
#include "common/h/Vector.h"
#include "dyninstAPI/src/showerror.h"
#include "dyninstAPI/src/os.h"
#include "dyninstAPI/src/util.h"
#include "dyninstAPI/src/debuggerinterface.h"
#include "dyninstAPI/src/mapped_object.h"
#include "dyninstAPI/src/callbacks.h"
#include "dyninstAPI/src/dynamiclinking.h"
#include "dyninstAPI_RT/h/dyninstAPI_RT.h"  // for DYNINST_BREAKPOINT_SIGNUM


#ifndef BPATCH_LIBRARY
#include "paradynd/src/main.h"  // for "tp" ?
#include "paradynd/src/pd_process.h" // for class pd_process
#endif

// the following are needed for handleSigChild
#include "dyninstAPI/src/signalhandler.h"
#include "dyninstAPI/src/signalgenerator.h"
#include "dyninstAPI/src/process.h"
#include "dyninstAPI/src/dyn_lwp.h"
#include "dyninstAPI/src/dyn_thread.h"
#include "dyninstAPI/src/instP.h"
#include "dyninstAPI/src/stats.h"

// BREAK_POINT_INSN
#if defined(AIX_PROC)
#include "dyninstAPI/src/arch-power.h"
#endif
#include "dyninstAPI/src/sol_proc.h"

#include <sys/poll.h>

/////////////////////////////////////////////////////////////////////////////
/// Massive amounts of signal handling code
/////////////////////////////////////////////////////////////////////////////


// Turn the return result of waitpid into something we can use
bool decodeWaitPidStatus(procWaitpidStatus_t status,
                        EventRecord &ev) {
    // Big if-then-else tree
    if (WIFEXITED(status)) {
        ev.type = evtProcessExit;
        ev.status = statusNormal;
        ev.what = (eventWhat_t) (unsigned int) WEXITSTATUS(status);
        return true;
    } 
    else if (WIFSIGNALED(status)) {
        ev.type = evtProcessExit;
        ev.status = statusSignalled;
        ev.what = (eventWhat_t) (unsigned int) WTERMSIG(status);
        return true;
    }
    else if (WIFSTOPPED(status)) {
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

bool SignalGenerator::decodeRTSignal(EventRecord &ev)
{
   // We've received a signal we believe was sent
   // from the runtime library. Check the RT lib's
   // status variable and return it.
   // These should be made constants
   process *proc = ev.proc;
   if (!proc) return false;

   pdstring status_str = pdstring("DYNINST_synch_event_id");
   pdstring arg_str = pdstring("DYNINST_synch_event_arg1");

   int status;
   Address arg;

   pdvector<int_variable *> vars;
   if (!proc->findVarsByAll(status_str, vars)) {
     return false;
   }

   if (vars.size() != 1) {
     fprintf(stderr, "%s[%d]:  ERROR:  %d vars matching %s, not 1\n", 
             FILE__, __LINE__, vars.size(), status_str.c_str());
     return false;
   }

   Address status_addr = vars[0]->getAddress();

   if (!proc->readDataSpace((void *)status_addr, sizeof(int),
                            &status, true)) {
      fprintf(stderr, "%s[%d]:  readDataSpace failed\n", FILE__, __LINE__);
      return false;
   }

   if (status == DSE_undefined) {
      return false; // Nothing to see here
   }

   vars.clear();
   if (!proc->findVarsByAll(arg_str, vars)) {
     return false;
   }

   if (vars.size() != 1) {
     fprintf(stderr, "%s[%d]:  ERROR:  %d vars matching %s, not 1\n", 
             FILE__, __LINE__, vars.size(), arg_str.c_str());
     return false;
   }

   Address arg_addr = vars[0]->getAddress();

   if (!proc->readDataSpace((void *)arg_addr, sizeof(Address),
                            &arg, true)) {
      fprintf(stderr, "%s[%d]:  readDataSpace failed\n", FILE__, __LINE__);
      return false;
   }

   ev.info = (eventInfo_t)arg;
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
   default:
        assert(0);
        break;
   }
  return decodeSyscall(ev);
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
  // continuation is handled at the BPatch layer via callbacks
  // continueHint = true;
  return retval;
}

bool SignalHandler::handleLoadLibrary(EventRecord &ev, bool &continueHint)
{
   process *proc = ev.proc;
   if (!proc->handleChangeInSharedObjectMapping(ev)) {
      fprintf(stderr, "%s[%d]:  setting event to NULL because handleChangeIn.. failed\n",
              FILE__, __LINE__);
     ev.type = evtNullEvent;
     return false;
   }

   continueHint = true;
   return true;
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
          fprintf(stderr, "%s[%d]:  decodeSignal failed\n", FILE__, __LINE__, ev.sprint_event(buf));
          return false;
        }
        break;
     case PR_SYSENTRY:
        ev.type = evtSyscallEntry;
        ev.what = status.pr_what;
#if defined(AIX_PROC)
        // We actually pull from the syscall argument vector
        if (status.pr_nsysarg > 0)
           ev.info = status.pr_sysarg[0];
        else
           ev.info = 0;
#endif
#if defined (os_osf)
       ev.info = status.pr_reg.regs[A0_REGNUM];
#endif
        decodeSyscall(ev);
        break;
     case PR_SYSEXIT:
        ev.type = evtSyscallExit;
        ev.what = status.pr_what;

#if defined(AIX_PROC)
        // This from the proc header file: system returns are
        // left in pr_sysarg[0]. NOT IN MAN PAGE.
        ev.info = status.pr_sysarg[0];
#endif
#if defined (os_osf)
       ev.info = status.pr_reg.regs[V0_REGNUM];
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
     case PR_FAULTED:
     default:
        assert(0);
        break;
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
        ev.what = (int) procSysFork;
        return true;
    }
    if (syscall == SYSSET_MAP(SYS_exec, pid) ||
        syscall == SYSSET_MAP(SYS_execv, pid) ||
        syscall == SYSSET_MAP(SYS_execve, pid)) {
        ev.what = (int) procSysExec;
        return true;
    }
    if (syscall == SYSSET_MAP(SYS_exit, pid)) {
        ev.what = (int) procSysExit; 
        return true;
    }
    if (syscall == SYSSET_MAP(SYS_lwp_exit, pid)) {
       ev.type = evtThreadExit;
       ev.what = (int) procLwpExit;
       return true;
    }
    // Don't map -- we make this up
    if (syscall == SYS_load) {
      ev.what = procSysLoad;
      return true;
    }

    ev.what = procSysOther;
    return false;
}

bool SignalHandler::handleProcessCreate(EventRecord &ev, bool &continueHint)
{
  process * proc = ev.proc;
  proc->setBootstrapState(begun_bs);
  if (proc->insertTrapAtEntryPointOfMain()) {
     pdstring buffer = pdstring("PID=") + pdstring(proc->getPid());
     buffer += pdstring(", attached to process, stepping to main");
     statusLine(buffer.c_str());
     continueHint = true;
     return true;
  } else {
     // We couldn't insert the trap... so detach from the process
     // and let it run. 
     fprintf(stderr, "%s[%d][%s]:  ERROR:  couldn't insert at entry of main,\n",
             FILE__, __LINE__, getThreadStr(getExecThreadID()));
     fprintf(stderr, "\tinstrumenting process impossible\n");
     // We should actually delete any mention of this
     // process... including (for Paradyn) removing it from the
     // frontend.
     proc->triggerNormalExitCallback(0);
     proc->handleProcessExit();
     continueHint = true;
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
       fprintf(stderr, "%s[%d]:  waitpid failed\n", __FILE__, __LINE__);
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
     fprintf(stderr, "[%s:%u] - Finished waitpid with %d\n", __FILE__, __LINE__, retWait);

  return false;
}

#if !defined (os_linux)

bool SignalGenerator::waitForEventInternal(EventRecord &ev)
{
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

  waiting_for_event = true;
  __UNLOCK;
  int num_selected_fds = poll(pfds, 1, timeout);
  
  __LOCK;
  
  waiting_for_event = false;
  int handled_fds = 0;

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
     return ret;
  } else if (num_selected_fds == 0) {
    //  poll timed out with nothing to report
    signal_printf("%s[%d]:  poll timed out\n", FILE__, __LINE__);
    ev.type = evtTimeout;
    ev.proc = proc;
    return true;
  }

  if (!pfds[0].revents) {
     fprintf(stderr, "%s[%d]:  weird, no event for signalled process\n", FILE__, __LINE__);
     return false;
  }

  ev.type = evtUndefined;
  ev.proc = proc;
  ev.lwp = proc->getRepresentativeLWP();
  ev.info  = pfds[0].revents;

  if (ev.proc->status() == running) {
      ev.proc->set_status(stopped);
  }

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

  //  signal number is assumed to be in ev.what
  switch(ev.what)  {
  case SIGSTOP:
  case SIGINT:
      if (!decodeRTSignal(ev) && !decodeSigStopNInt(ev)) {
          fprintf(stderr, "%s[%d]:  weird, decodeSigStop failed for SIGSTOP\n", FILE__, __LINE__);
      }

      // We have to use SIGSTOP for RTsignals, since we may not be attached
      // at that point...
      break;
  case DYNINST_BREAKPOINT_SIGNUM: /*SIGUSR2*/
    signal_printf("%s[%d]:  DYNINST BREAKPOINT\n", FILE__, __LINE__);
    if (!decodeRTSignal(ev)) {
        ev.type = evtProcessStop; // happens when we get a DYNINSTbreakPoint
      }
     break;
  case SIGIOT:
  {
     ev.type = evtProcessExit;
     ev.status = statusSignalled;
  }
  case SIGTRAP:
  {
    signal_printf("%s[%d]:  SIGTRAP\n", FILE__, __LINE__);
    return decodeSigTrap(ev);
  }
  case SIGILL:
  case SIGSEGV:
  {
     signal_printf("%s[%d]:  SIGILL\n", FILE__, __LINE__);
#if defined (arch_ia64)
     assert(ev.lwp);
     
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
  case SIGCHLD:
#if defined (os_linux)
     // Linux fork() sends a SIGCHLD once the fork has been created
     ev.type = evtPreFork;
#endif
     break;
  default:
     signal_printf("%s[%d]:  got signal %d\n", FILE__, __LINE__, ev.what);
     break;
  }

  return true;
}

bool SignalGenerator::decodeSigTrap(EventRecord &ev)
{
  char buf[128];
  process *proc = ev.proc;

  if (decodeIfDueToProcessStartup(ev)) {
     signal_printf("%s[%d]:  decodeSigTrap for %s, state: %s\n",
                FILE__, __LINE__, ev.sprint_event(buf), 
                proc->getBootstrapStateAsString().c_str());
     return true;
  }

  Frame af = ev.lwp->getActiveFrame();

  // (1)  Is this trap due to an instPoint ??
  if (proc->trampTrapMapping.defines(af.getPC())) {
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

  // (4) Is this an instrumentation-based method of grabbing the exit
  // of a system call?
  if (ev.lwp->decodeSyscallTrap(ev)) {
      // That sets all information....
      return true;
  }

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
      // Don't report an error... and LWP could have gone away and we don't know
      // about it yet.
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

//bool SignalGenerator::forkNewProcess()
bool forkNewProcess_real(pdstring file,
                    pdstring dir, pdvector<pdstring> *argv,
                    pdvector<pdstring> *envp,
                    pdstring inputFile, pdstring outputFile, int &traceLink,
                    pid_t &pid, int stdin_fd, int stdout_fd, int stderr_fd)
{
  forkexec_printf("%s[%d][%s]:  welcome to forkNewProcess(%s)\n",
          __FILE__, __LINE__, getThreadStr(getExecThreadID()), file.c_str());
#ifndef BPATCH_LIBRARY
   int tracePipe[2];
   int r = P_pipe(tracePipe);
   if (r) {
      // P_perror("socketpair");
      pdstring msg = pdstring("Unable to create trace pipe for program '") + file +
         pdstring("': ") + pdstring(strerror(errno));
      showErrorCallback(68, msg);
      return false;
   }
#endif
   errno = 0;

   pid = fork();

   if (pid != 0) {
      // *** parent
      startup_printf("%s[%d][%s]:  ForkNewProcessCallback::execute(%s): FORK PARENT\n",
               __FILE__, __LINE__, getThreadStr(getExecThreadID()), file.c_str());


//#if defined(os_linux)
//      if (!attachToChild(pid)) 
//        assert (0 && "failed to ptrace attach to child process");
//#endif

#if (defined(BPATCH_LIBRARY) && !defined(alpha_dec_osf4_0))
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
      fprintf(stderr, "%s[%d][%s]:  ForkNewProcessCallback::execute(%s): FORK ERROR\n",
               __FILE__, __LINE__, getThreadStr(getExecThreadID()), file.c_str());
            sprintf(errorLine, "Unable to start %s: %s\n", file.c_str(), 
                    strerror(errno));
            logLine(errorLine);
            showErrorCallback(68, (const char *) errorLine);
            return false; 
         }

#ifndef BPATCH_LIBRARY
      // parent never writes trace records; it only receives them.
      P_close(tracePipe[1]);
      traceLink = tracePipe[0];
#endif
      return true;

   } else if (pid == 0) {
      // *** child

#ifndef BPATCH_LIBRARY
      //setup output redirection to termWin
      dup2(stdout_fd,1);
      dup2(stdout_fd,2);

      // Now that stdout is going to a pipe, it'll (unfortunately) be block
      // buffered instead of the usual line buffered (when it goes to a tty).
      // In effect the stdio library is being a little too clever for our
      // purposes.  We don't want the "bufferedness" to change.  So we set it
      // back to line-buffered.  The command to do this is setlinebuf(stdout)
      // [stdio.h call] But we don't do it here, since the upcoming execve()
      // would undo our work [execve keeps fd's but resets higher-level stdio
      // information, which is recreated before execution of main()] So when
      // do we do it?  In rtinst's DYNINSTinit (RTposix.c et al.)

      // setup stderr for rest of exec try.
      FILE *childError = P_fdopen(2, "w");

      P_close(tracePipe[0]);

      if (P_dup2(tracePipe[1], 3) != 3) {
         fprintf(childError, "dup2 failed\n");
         fflush(childError);
         P__exit(-1);
      }

      /* close if higher */
      if (tracePipe[1] > 3) P_close(tracePipe[1]);


      if ((dir.length() > 0) && (P_chdir(dir.c_str()) < 0)) {
         bpfatal("cannot chdir to '%s': %s\n", dir.c_str(), 
                 strerror(errno));
         P__exit(-1);
      }
#endif
#if !defined(BPATCH_LIBRARY)
      /* see if I/O needs to be redirected */
      if (inputFile.length()) {
         int fd = P_open(inputFile.c_str(), O_RDONLY, 0);
         if (fd < 0) {
            fprintf(childError, "stdin open of %s failed\n", inputFile.c_str());
            fflush(childError);
            P__exit(-1);
         } else {
            dup2(fd, 0);
            P_close(fd);
         }
      }

      if (outputFile.length()) {
         int fd = P_open(outputFile.c_str(), O_WRONLY|O_CREAT, 0444);
         if (fd < 0) {
            fprintf(childError, "stdout open of %s failed\n", outputFile.c_str());
            fflush(childError);
            P__exit(-1);
         } else {
            dup2(fd, 1); // redirect fd 1 (stdout) to a copy of descriptor "fd"
            P_close(fd); // not using descriptor fd any more; close it.
         }
      }
#endif

#if defined (BPATCH_LIBRARY)
      // Should unify with (fancier) Paradyn handling
      /* see if we should use alternate file decriptors */
      if (stdin_fd != 0) dup2(stdin_fd, 0);
      if (stdout_fd != 1) dup2(stdout_fd, 1);
      if (stderr_fd != 2) dup2(stderr_fd, 2);
#endif

      // define our own session id so we don't get the mutators signals
#ifndef rs6000_ibm_aix4_1
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
      if (envp) {
	  envs = new char*[envp->size() + 2]; // Also room for PARADYN_MASTER_INFO
	  for(unsigned ei = 0; ei < envp->size(); ++ei)
	      envs[ei] = P_strdup((*envp)[ei].c_str());
	  envs[envp->size()] = NULL;
      }
      
#ifndef BPATCH_LIBRARY
      // hand off info about how to start a paradynd to the application.
      //   used to catch rexec calls, and poe events.
      //
      char* paradynInfo = new char[1024];
      sprintf(paradynInfo, "PARADYN_MASTER_INFO= ");
      for (unsigned i=0; i < pd_process::arg_list.size(); i++) {
         const char *str;

         str = P_strdup(pd_process::arg_list[i].c_str());
         if (!strcmp(str, "-l1")) {
            strcat(paradynInfo, "-l0");
         } else {
            strcat(paradynInfo, str);
         }
         strcat(paradynInfo, " ");
      }

      if (envp) {
	  envs[envp->size()] = P_strdup(paradynInfo);
	  envs[envp->size() + 1] = NULL;
      } else {
	  P_putenv(paradynInfo);
      }
#endif

      char **args;
      args = new char*[argv->size()+1];
      for (unsigned ai=0; ai<argv->size(); ai++)
         args[ai] = P_strdup((*argv)[ai].c_str());
      args[argv->size()] = NULL;

     startup_printf("%s[%d]:  before exec\n", __FILE__, __LINE__);
     char argstr[2048];
     argstr[0] = '\0';
     for (unsigned int ji=0; ji < argv->size(); ji++) {
       pdstring &s = (*argv)[ji];
       sprintf(argstr, "%s %s", argstr, s.c_str());
     }
     startup_printf("%s[%d]:  EXEC: %s %s\n", __FILE__, __LINE__, file.c_str(), argstr);
      if (envp) {
	  P_execve(file.c_str(), args, envs);
      }else
	  P_execvp(file.c_str(), args);

      sprintf(errorLine, "paradynd: execv failed, errno=%d\n", errno);
      logLine(errorLine);
    
      logLine(strerror(errno));
      {
         int i=0;
         while (args[i]) {
            sprintf(errorLine, "argv %d = %s\n", i, args[i]);
            logLine(errorLine);
            i++;
         }
      }
      {
	  for(unsigned i = 0; envs[i] != NULL; ++i) {
	      sprintf(errorLine, "envp %d = %s\n", i, envs[i]);
	      logLine(errorLine);
	  }
      }	      
      P__exit(-1);
      // not reached
    
      return false;
   } 
   return false;
}

bool SignalGenerator::forkNewProcess()
{
#if !defined (os_linux)
    return forkNewProcess_real(file_, dir_, 
                               argv_, envp_, 
                               inputFile_, outputFile_,
                               traceLink_, pid_, 
                               stdin_fd_, stdout_fd_, stderr_fd_);
#else
    // Linux platforms MUST execute fork() calls on the debugger interface
    // (ptrace) thread; see comment above DebuggerInterface::forkNewProcess
    // in debuginterface.h for details. -- nater 22.feb.06
    return getDBI()->forkNewProcess(file_, dir_, 
                                    argv_, envp_, 
                                    inputFile_, outputFile_, 
                                    traceLink_, pid_,
                                    stdin_fd_, stdout_fd_, stderr_fd_);
#endif
}

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

       int num_selected_fds = poll(pfds, 1, timeout);
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

bool DBI_writeDataSpace(pid_t pid, Address addr, int nelem, Address data, int word_len, const char *file, unsigned int line)
{
  dbi_printf("%s[%d]: DBI_writeDataSpace(%d, %p, %d, %p, %d) called from %s[%d]\n", 
            FILE__, __LINE__, pid, (void *) addr, nelem, (void *) data, word_len,file,line);
  return getDBI()->writeDataSpace(pid, addr, nelem, data, word_len, file, line);
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

bool DBI_readDataSpace(pid_t pid, Address addr, int nelem, Address data, int word_len, const char *file, unsigned int line)
{
  bool ret =  getDBI()->readDataSpace(pid, addr, nelem, data, word_len, file, line);
  if (!ret)
    signal_printf("%s[%d]:  readDataSpace at %s[%d] failing\n", 
                  __FILE__, __LINE__, file, line);
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

SignalGenerator::SignalGenerator(char *idstr, pdstring file, pdstring dir,
                                 pdvector<pdstring> *argv,
                                 pdvector<pdstring> *envp,
                                 pdstring inputFile,
                                 pdstring outputFile,
                                 int stdin_fd, int stdout_fd,
                                 int stderr_fd) :
    SignalGeneratorCommon(idstr),
    waiting_for_stop(false) {
    setupCreated(file, dir, 
                 argv, envp, 
                 inputFile, outputFile,
                 stdin_fd, stdout_fd, stderr_fd);
}

/// Experiment: wait for the process we're attaching to to be created
// before we return. 

SignalGenerator::SignalGenerator(char *idstr, pdstring file, int pid)
    : SignalGeneratorCommon(idstr),
      waiting_for_stop(false) {
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
