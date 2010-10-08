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

/*
 * dyn_lwp.C -- cross-platform segments of the LWP handler class
 * $Id: dyn_lwp.C,v 1.68 2008/04/15 16:43:13 roundy Exp $
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
  threadInfoBlockAddr_(0),
  trappedSyscall_(NULL), trappedSyscallCallback_(NULL),
  trappedSyscallData_(NULL),
  isRunningIRPC(false), isDoingAttach_(false), is_attached_(false),
  is_as_lwp_(false),
  is_dead_(false),
  is_debugger_lwp(false)
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
  threadInfoBlockAddr_(0),
  trappedSyscall_(NULL), trappedSyscallCallback_(NULL),
  trappedSyscallData_(NULL),
  isRunningIRPC(false), isDoingAttach_(false), is_attached_(false),
  is_as_lwp_(false),
  is_dead_(false),
  is_debugger_lwp(false)
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
  threadInfoBlockAddr_(0),
  trappedSyscall_(NULL), trappedSyscallCallback_(NULL),
  trappedSyscallData_(NULL),
  isRunningIRPC(false), isDoingAttach_(false), is_attached_(false),
  is_as_lwp_(false),
  is_dead_(false),
  is_debugger_lwp(false)
{
}

dyn_lwp::~dyn_lwp()
{
  if (status_ != exited && is_attached())
    detach();
}

// TODO is this safe here ?
bool dyn_lwp::continueLWP(int signalToContinueWith, bool clear_stackwalk) 
{
   if (clear_stackwalk) {
      clearStackwalk();
      //proc()->invalidateActiveMultis();
   }
    if (!proc()->IndependentLwpControl() &&
        (this != proc()->getRepresentativeLWP())) {
        // This'll hit stop_, which calls pauseLWP on the representative LWP.
        // Hence the comparison.
        return proc()->continueProc(); 
    }

    assert(proc()->IndependentLwpControl() ||
           (this == proc()->getRepresentativeLWP()));

   if(status_ == running || isDoingAttach_) {
      return true;
   }
/*
   if (status_ == exited) {
       return true;
   }
*/
#if !defined(os_vxworks)
   if (proc()->sh->waitingForStop())
   {
     return false;
   }
#endif

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


std::string dyn_lwp::getStatusAsString() const 
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

    
    if (isDoingAttach_) {
       // We'll get to it later...
       return true;
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
    if(status_ == exited) {
      return true;
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
   return proc_->walkStackFromFrame(active, cached_stackwalk.getStackwalk());
}

bool dyn_lwp::walkStack(pdvector<Frame> &stackWalk, bool /* ignoreRPC = false */)
{
   stackWalk.clear();
   if (cached_stackwalk.isValid()) {
      stackWalk = cached_stackwalk.getStackwalk();
      return true;
   }
    
   // We cheat (a bit): this method is here for transparency, 
   // but the process class does the work in the walkStackFromFrame
   // method. We get the active frame and hand off.
   Frame active = getActiveFrame();
   bool result = proc_->walkStackFromFrame(active, stackWalk);
   if (result)
      cached_stackwalk.setStackwalk(stackWalk);
   return result;
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

bool dyn_lwp::getRegisters(struct dyn_saved_regs *regs, bool includeFP) {
	/* Don't cache registers.  It's broken on most platforms. */
    return getRegisters_(regs, includeFP);
	}

bool dyn_lwp::restoreRegisters(const struct dyn_saved_regs &regs, bool includeFP) {
	/* Don't cache registers.  It's broken on most platforms. */
    return restoreRegisters_(regs, includeFP);
	}

// Find out some info about the system call we're waiting on,
// and ask the process class to set a breakpoint there. 

#if defined(cap_syscall_trap)
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
#endif // cap_syscall_trap

#if !defined(os_linux) && !defined(os_windows)
Address dyn_lwp::step_next_insn() {
   fprintf(stderr, "Single stepping not implemented on this platform\n");
   return 0x0;
}
#endif

void dyn_lwp::internal_lwp_set_status___(processState st) {
	status_ = st;
}

bool dyn_lwp::is_asLWP() {
    return is_as_lwp_;
}

void dyn_lwp::clearStackwalk() {
   cached_stackwalk.clear();
}

