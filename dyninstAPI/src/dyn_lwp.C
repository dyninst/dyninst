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

/*
 * dyn_lwp.C -- cross-platform segments of the LWP handler class
 * $Id: dyn_lwp.C,v 1.59 2006/05/30 23:33:54 mjbrim Exp $
 */

#include "common/h/headers.h"
#include "dyn_lwp.h"
#include "process.h"
#include "debuggerinterface.h"
#include <assert.h>
#include "signalgenerator.h"

dyn_lwp::dyn_lwp() :
  changedPCvalue(0),
  status_(neonatal),
  proc_(NULL),
  lwp_id_(0),
  fd_(0),
  ctl_fd_(INVALID_HANDLE_VALUE),
  status_fd_(INVALID_HANDLE_VALUE),
  as_fd_(INVALID_HANDLE_VALUE),
  auxv_fd_(INVALID_HANDLE_VALUE),
  map_fd_(INVALID_HANDLE_VALUE),
  ps_fd_(INVALID_HANDLE_VALUE),
  procHandle_(INVALID_HANDLE_VALUE),
  singleStepping(false),
  stoppedInSyscall_(false),
  postsyscallpc_(0),
  waiting_for_stop(false),
  trappedSyscall_(NULL), trappedSyscallCallback_(NULL),
  trappedSyscallData_(NULL),
  isRunningIRPC(false), isDoingAttach_(false), is_attached_(false)  
{
};

dyn_lwp::dyn_lwp(unsigned lwp, process *proc) :
  changedPCvalue(0),
  status_(neonatal),
  proc_(proc),
  lwp_id_(lwp),
  fd_(INVALID_HANDLE_VALUE),
  ctl_fd_(INVALID_HANDLE_VALUE),
  status_fd_(INVALID_HANDLE_VALUE),
  as_fd_(INVALID_HANDLE_VALUE),
  auxv_fd_(INVALID_HANDLE_VALUE),
  map_fd_(INVALID_HANDLE_VALUE),
  ps_fd_(INVALID_HANDLE_VALUE),
  procHandle_(INVALID_HANDLE_VALUE),
  singleStepping(false),
  stoppedInSyscall_(false),
  postsyscallpc_(0),
  waiting_for_stop(false),
  trappedSyscall_(NULL), trappedSyscallCallback_(NULL),
  trappedSyscallData_(NULL),
  isRunningIRPC(false), isDoingAttach_(false), is_attached_(false)
{
}

dyn_lwp::dyn_lwp(const dyn_lwp &l) :
  changedPCvalue(0),
  status_(neonatal),
  proc_(l.proc_),
  lwp_id_(l.lwp_id_),
  fd_(INVALID_HANDLE_VALUE),
  ctl_fd_(INVALID_HANDLE_VALUE),
  status_fd_(INVALID_HANDLE_VALUE),
  as_fd_(INVALID_HANDLE_VALUE),
  auxv_fd_(INVALID_HANDLE_VALUE),
  map_fd_(INVALID_HANDLE_VALUE),
  ps_fd_(INVALID_HANDLE_VALUE),
  procHandle_(INVALID_HANDLE_VALUE),
  singleStepping(false),
  stoppedInSyscall_(false),
  postsyscallpc_(0),
  waiting_for_stop(false),
  trappedSyscall_(NULL), trappedSyscallCallback_(NULL),
  trappedSyscallData_(NULL),
  isRunningIRPC(false), isDoingAttach_(false), is_attached_(false)
{
}

dyn_lwp::~dyn_lwp()
{
  if (status_ != exited && is_attached())
    detach();
}

// TODO is this safe here ?
bool dyn_lwp::continueLWP(int signalToContinueWith) 
{
    if (!proc()->IndependentLwpControl() &&
        (this != proc()->getRepresentativeLWP())) {
        // This'll hit stop_, which calls pauseLWP on the representative LWP.
        // Hence the comparison.
        return proc()->continueProc(); 
    }

    assert(proc()->IndependentLwpControl() ||
           (this == proc()->getRepresentativeLWP()));

   if(status_ == running) {
      return true;
   }
/*
   if (status_ == exited) {
       return true;
   }
*/
   if (proc()->sh->waitingForStop())
   {
     return false;
   }


   bool ret = continueLWP_(signalToContinueWith);
   if(ret == false) {
      perror(NULL);
      return false;
   }

#if defined (os_windows)
   if (status_ != exited)
        proc()->set_lwp_status(this, running);
   if (getExecThreadID() != proc()->sh->getThreadID()) {
     signal_printf("%s[%d][%s]:  signalling active process\n", 
                   FILE__, __LINE__, getThreadStr(getExecThreadID()));
     proc()->sh->signalActiveProcess();
   }
   // no SIGSTOP on Windows, and also no such thing as continuing with signal
#else
   if (signalToContinueWith != SIGSTOP)  {
      proc()->set_lwp_status(this, running);
      if (getExecThreadID() != proc()->sh->getThreadID()) {
          signal_printf("%s[%d][%s]:  signalling active process from continueLWP\n", 
                        FILE__, __LINE__, getThreadStr(getExecThreadID()));
          proc()->sh->signalActiveProcess();
      }
   }
#endif
   
   // When we fork we may cross the streams (a sighandler from the parent controlling
   // the child). Argh.
   proc()->sh->markProcessContinue();

   return true;
}


pdstring dyn_lwp::getStatusAsString() const 
{
   // useful for debugging
   if (status_ == neonatal)
      return "neonatal";
   if (status_ == stopped)
      return "stopped";
   if (status_ == running)
      return "running";
   if (status_ == exited)
      return "exited";
   if (status_ == detached)
       return "detached";
   assert(false);
   return "???";
}

bool dyn_lwp::pauseLWP(bool shouldWaitUntilStopped) {
    if (!proc()->IndependentLwpControl() &&
        (this != proc()->getRepresentativeLWP())) {
        // This'll hit stop_, which calls pauseLWP on the representative LWP.
        // Hence the comparison.
        return proc()->pause(); 
    }

    assert(proc()->IndependentLwpControl() ||
           (this == proc()->getRepresentativeLWP()));

    
    eventType evt;
    while (isDoingAttach_) {
       evt = proc()->sh->waitForEvent(evtAnyEvent, proc_, this);
       if (evt == evtProcessExit)
          return false;
    }

   // Not checking lwp status_ for neonatal because it breaks attach with the
   // dyninst tests.  My guess is that somewhere we set the process status to
   // running.  If we can find this, then we can set the lwp status to
   // running also.

    if(status_ == stopped || status_ == exited) {
        return true;
    }
    
    bool res = stop_();
    if(res == false)
        return false;
    
    if(shouldWaitUntilStopped) {
        res = waitUntilStopped();
    }
    
    proc()->set_lwp_status(this, stopped);
    return res;
}


// Not sure this is a good idea... when would we be walking the stack
// (conceptually) of an LWP rather than the thread running on it?  For now:
// non-MT will walk getRepresentativeLWP() since it doesn't understand
// multithreaded programs

void dyn_lwp::markDoneRunningIRPC() {
   isRunningIRPC = false;
}

// stackWalk: return parameter.
bool dyn_lwp::markRunningIRPC() {
   // Cache the current stack frame

   Frame active = getActiveFrame();
   isRunningIRPC = true;
   return proc_->walkStackFromFrame(active, cachedStackWalk);
}

bool dyn_lwp::walkStack(pdvector<Frame> &stackWalk, bool ignoreRPC /* = false */)
{
   // If we're in an inferior RPC, return the stack walk
   // from where the process "should" be
    stackWalk.clear();
    
    if (isRunningIRPC && !ignoreRPC) {
        stackWalk = cachedStackWalk;
        return true;
    }
    
    // We cheat (a bit): this method is here for transparency, 
    // but the process class does the work in the walkStackFromFrame
    // method. We get the active frame and hand off.
    Frame active = getActiveFrame();
    
    return proc_->walkStackFromFrame(active, stackWalk);
}

bool dyn_lwp::attach() {
   assert(!is_attached());
   bool res;
   if(this == proc()->getRepresentativeLWP()) {
      res = representativeLWP_attach_();
   }else {
      res = realLWP_attach_();
   }

   if(res == true)
      is_attached_ = true;

   return res;
}

void dyn_lwp::detach() {
   if(! is_attached())
      return;

   if(this == proc()->getRepresentativeLWP())
      representativeLWP_detach_();
   else
      realLWP_detach_();

   is_attached_ = false;
}

int dyn_lwp::getPid() const {
   return proc_->getPid();
}

bool dyn_lwp::getRegisters(struct dyn_saved_regs *regs) {
	/* Don't cache registers.  It's broken on most platforms. */
    return getRegisters_(regs);
	}

bool dyn_lwp::restoreRegisters(const struct dyn_saved_regs &regs) {
	/* Don't cache registers.  It's broken on most platforms. */
    return restoreRegisters_(regs);
	}

// Find out some info about the system call we're waiting on,
// and ask the process class to set a breakpoint there. 

bool dyn_lwp::setSyscallExitTrap(syscallTrapCallbackLWP_t callback,
                                 void *data)
{
    assert(executingSystemCall());
    if (trappedSyscall_) {
        // Can't set trap twice
        bperr( "Error: syscall already trapped on LWP %d\n",
                get_lwp_id());
        return false;
    }

    Address syscallInfo = getCurrentSyscall();

    if(syscallInfo == 0) return false;
    signal_printf("%s[%d]: LWP %d placing syscall trap for %d (0x%lx)...\n",
                  FILE__, __LINE__, get_lwp_id(),
		  syscallInfo, syscallInfo);

    trappedSyscall_ = proc()->trapSyscallExitInternal(syscallInfo);

    assert(trappedSyscallCallback_ == NULL);
    assert(trappedSyscallData_ == NULL);
    trappedSyscallCallback_ = callback;
    trappedSyscallData_ = data;

    return (trappedSyscall_ != NULL);
}


// Clear the trap set

bool dyn_lwp::clearSyscallExitTrap()
{
    assert(trappedSyscall_);
    
    if (!proc()->clearSyscallTrapInternal(trappedSyscall_))
        return false;

    trappedSyscall_ = NULL;
    trappedSyscallCallback_ = NULL;
    trappedSyscallData_ = NULL;
    return true;
}

bool dyn_lwp::handleSyscallTrap(EventRecord &ev, bool &continueHint) 
{
    // See if this is the right one...
    if (ev.type != evtSyscallExit) return false;
    if (!trappedSyscall_) return false;
    if (ev.what != procSysOther) return false;

    // Our event handling is a little skewed. We have a two-level
    // structure: known syscalls (mapped to a local enumerated type)
    // and unknown (with the syscall # in the info field).
    // Let's assume for now that we only trap unknown syscalls...

#if !defined(os_windows)
    if (ev.info != trappedSyscall_->syscall_id) return false;
#endif

    // Step past the trap (if necessary)
    stepPastSyscallTrap();

    // Make a copy of the callback... we clear before we call,
    // but clear is called from other locations as well.

    syscallTrapCallbackLWP_t callback = trappedSyscallCallback_;
    void *data = trappedSyscallData_;
    
    // And clear the callback
    clearSyscallExitTrap();

    // Make the callback
    if (callback)
      continueHint = (*callback)(this, data);

    return true;
}

bool dyn_lwp::isWaitingForSyscall() const {
    if (trappedSyscall_) return true;
    else return false;
}

#if !defined(os_linux) && !defined(os_windows)
Address dyn_lwp::step_next_insn() {
   fprintf(stderr, "Single stepping not implemented on this platform\n");
   return 0x0;
}
#endif

void dyn_lwp::internal_lwp_set_status___(processState st) {
	status_ = st;
}
