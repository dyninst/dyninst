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

#include "dyninstAPI/src/dyn_thread.h"
#include "dyninstAPI/src/dyn_lwp.h"
#include "dyninstAPI/src/rpcMgr.h"


// This will need to be filled in if we (ever) handle migrating threads
bool dyn_thread::updateLWP()
{
  return true;
}

  
// MT version lives in the <os>MT.C files, and can do things like
// get info for threads not currently scheduled on an LWP
Frame dyn_thread::getActiveFrame()
{
   updateLWP();

   if(! get_proc()->multithread_capable(true)) {
      Frame lwpFrame = lwp->getActiveFrame();  
      lwpFrame.thread_ = this;
      return lwpFrame;
   } else
      return getActiveFrameMT();
}

// stackWalk: return parameter.
bool dyn_thread::walkStack(pdvector<Frame> &stackWalk)
{
    stackWalk.clear();
    
    if (useRPCStack_) {
        stackWalk = RPCstack_;
        return true;
    }
    Frame active = getActiveFrame();

    return proc->walkStackFromFrame(active, stackWalk);
}

dyn_lwp *dyn_thread::get_lwp()
{
  if (proc->multithread_ready(true))
    updateLWP();
  return lwp;
}

bool dyn_thread::savePreRPCStack()
{
    if (useRPCStack_)
        assert(0);
    
    walkStack(RPCstack_);
    useRPCStack_ = true;
    return true;
}

void dyn_thread::clearPreRPCStack() {
    useRPCStack_ = false;
}


