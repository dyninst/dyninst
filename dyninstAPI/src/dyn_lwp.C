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
 * $Id: dyn_lwp.C,v 1.30 2005/09/20 19:07:51 bernat Exp $
 */

#include "common/h/headers.h"
#include "dyn_lwp.h"
#include "process.h"
#include <assert.h>

dyn_lwp::dyn_lwp() :
  changedPCvalue(0),
  status_(neonatal),
  proc_(NULL),
  lwp_id_(0),
  fd_(0),
  procHandle_(INVALID_HANDLE_VALUE),
  stoppedInSyscall_(false),
  postsyscallpc_(0),
  trappedSyscall_(NULL), trappedSyscallCallback_(NULL),
  trappedSyscallData_(NULL),
  cached_regs(NULL),
  isRunningIRPC(false), is_attached_(false)  
{
};

dyn_lwp::dyn_lwp(unsigned lwp, process *proc) :
  changedPCvalue(0),
  status_(neonatal),
  proc_(proc),
  lwp_id_(lwp),
  fd_(INVALID_HANDLE_VALUE),
  procHandle_(INVALID_HANDLE_VALUE),
  stoppedInSyscall_(false),
  postsyscallpc_(0),
  trappedSyscall_(NULL), trappedSyscallCallback_(NULL),
  trappedSyscallData_(NULL),
  cached_regs(NULL),
  isRunningIRPC(false), is_attached_(false)
{
}

dyn_lwp::dyn_lwp(const dyn_lwp &l) :
  changedPCvalue(0),
  status_(neonatal),
  proc_(l.proc_),
  lwp_id_(l.lwp_id_),
  fd_(INVALID_HANDLE_VALUE),
  procHandle_(INVALID_HANDLE_VALUE),
  stoppedInSyscall_(false),
  postsyscallpc_(0),
  trappedSyscall_(NULL), trappedSyscallCallback_(NULL),
  trappedSyscallData_(NULL),
  cached_regs(NULL),
  isRunningIRPC(false), is_attached_(false)
{
}

dyn_lwp::~dyn_lwp()
{
  if (status_ != exited)
    detach();
}

// TODO is this safe here ?
bool dyn_lwp::continueLWP(int signalToContinueWith) {
   if(status_ == running) {
      return true;
   }

   if (proc()->suppressEventConts())
   {
     return false;
   }

   // the saved cached register can be cleared since continuing
   if(this == proc()->getRepresentativeLWP())
      proc()->clearCachedRegister();
   else
      clearCachedRegister();

   if(cached_regs != NULL) {
      delete cached_regs;
      cached_regs = NULL;
   }

   bool ret = continueLWP_(signalToContinueWith);
   if(ret == false) {
      perror("continueLWP()");
      return false;
   }

#if !defined(mips_unknown_ce2_11) && !defined(i386_unknown_nt4_0)
   // no SIGSTOP on Windows, and also no such thing as continuing with signal
   if (signalToContinueWith != SIGSTOP)
#endif
      proc()->set_lwp_status(this, running);

   return true;
}


bool dyn_lwp::pauseLWP(bool shouldWaitUntilStopped) {
   // Not checking lwp status_ for neonatal because it breaks attach with the
   // dyninst tests.  My guess is that somewhere we set the process status to
   // running.  If we can find this, then we can set the lwp status to
   // running also.
   if(status_ == stopped) {
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

bool dyn_lwp::walkStack(pdvector<Frame> &stackWalk)
{
   // If we're in an inferior RPC, return the stack walk
   // from where the process "should" be
    stackWalk.clear();
    
    if (isRunningIRPC) {
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
   if(this == proc()->getRepresentativeLWP())
      res = representativeLWP_attach_();
   else
      res = realLWP_attach_();

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
#if defined(cap_proc_fd)
    // There is a bug somewhere in our register caching that leads to segfaults
    // and SIGBUS errors.
    return getRegisters_(regs);
#else
   if(cached_regs == NULL) {
      bool result = getRegisters_(regs);
      if(cached_regs == NULL) {
         cached_regs = new dyn_saved_regs;
         memcpy(cached_regs, regs, sizeof(struct dyn_saved_regs));
      }
      return result;
   } else {
      memcpy(regs, cached_regs, sizeof(struct dyn_saved_regs));
      return true;
   }
#endif
}

bool dyn_lwp::restoreRegisters(const struct dyn_saved_regs &regs) {
#if defined(cap_proc_fd)
    // There is a bug somewhere in our register caching that leads to segfaults
    // and SIGBUS errors.
    return restoreRegisters_(regs);
#else
   if(cached_regs != NULL) {
      delete cached_regs;
      cached_regs = NULL;
   }
   cached_regs = new dyn_saved_regs();
   memcpy(cached_regs, &regs, sizeof(struct dyn_saved_regs));

   bool res = restoreRegisters_(regs);
   if(! res) {
      delete cached_regs;
   }
   return res;
#endif
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
    
    // Make the callback
    if (trappedSyscallCallback_)
        (*trappedSyscallCallback_)(this, trappedSyscallData_);

    trappedSyscall_ = NULL;
    trappedSyscallCallback_ = NULL;
    trappedSyscallData_ = NULL;
    return true;
}

bool dyn_lwp::isWaitingForSyscall() const {
    if (trappedSyscall_) return true;
    else return false;
}

