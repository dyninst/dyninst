/*
 * Copyright (c) 1996 Barton P. Miller
 * 
 * We provide the Paradyn Parallel Performance Tools (below
 * described as Paradyn") on an AS IS basis, and do not warrant its
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

#include "dyninstAPI/src/pdThread.h"
#include "dyninstAPI/src/dyn_lwp.h"

#if defined(MT_THREAD)
rawTime64 pdThread::getInferiorVtime(virtualTimer *vTimer,
				     bool& success) {
  rawTime64 ret ;
  success = true ;

  if (!vTimer) {
    success = false ;
    return 0 ;
  }

  updateLWP();

  volatile const int protector2 = vTimer->protector2;

  const int    count = vTimer->counter;
  rawTime64 total, start;
  total = vTimer->total ;
  start = vTimer->start ;
  volatile const int protector1 = vTimer->protector1;
  
  if (protector1 != protector2) {
    success = false ;
    return 0;
  }
  if (count > 0) {
    ret = total + proc->getRawCpuTime(vTimer->lwp) - start ;    
  } else {
    ret = total ;
  }
  return ret ;
}
#endif //MT_THREAD 

// We have an LWP handle. Make sure it's still the correct
// one by checking its ID against the one in shared memory

#if !defined(BPATCH_LIBRARY)

bool pdThread::updateLWP()
{
  if (pos == -1) {
    lwp = proc->getDefaultLWP();
    return true;
  }

  if (!proc->hasInitializedMetaData) {
    lwp = proc->getDefaultLWP();
    return true;
  }
  
  int lwp_id;
  if (lwp) lwp_id = lwp->get_lwp();
  else lwp_id = 0;
  int vt_lwp = proc->shmMetaData->getVirtualTimer(pos).lwp;
  if (vt_lwp < 0) {
    lwp = NULL; // Not currently scheduled
    return false;
  }
  if (lwp_id == vt_lwp) return true;

  lwp = proc->getLWP(vt_lwp);

  if (!lwp) // Odd, not made yet?
    return false;
  return true;
}

#else

bool pdThread::updateLWP()
{
  return true;
}
#endif
  
dyn_lwp *pdThread::get_lwp()
{
  if (proc->multithread_ready())
    updateLWP();
  return lwp;
}

void pdThread::scheduleIRPC(inferiorRPCtoDo todo)
{
  thrRPCsWaitingToStart += todo;
}

bool pdThread::readyIRPC()
{
  return !thrRPCsWaitingToStart.empty();
}

inferiorRPCtoDo pdThread::peekIRPC()
{
  return thrRPCsWaitingToStart[0];
}

inferiorRPCtoDo pdThread::popIRPC()
{
  inferiorRPCtoDo first;
  first = thrRPCsWaitingToStart[0];
  thrRPCsWaitingToStart.removeOne();
  return first;
}

void pdThread::runIRPC(inferiorRPCinProgress running)
{
  thrCurrRunningRPC = running;
}

void pdThread::setRunningIRPC()
{
  in_IRPC=true;
}

inferiorRPCinProgress pdThread::getIRPC()
{
  return thrCurrRunningRPC;
}

void pdThread::clearRunningIRPC()
{
  in_IRPC = false;
}

bool pdThread::isRunningIRPC()
{
  return in_IRPC;
}

void pdThread::setInSyscall()
{
  in_syscall = true;
}

void pdThread::clearInSyscall()
{
  in_syscall = false;
}

bool pdThread::isInSyscall()
{
  return in_syscall;
}
