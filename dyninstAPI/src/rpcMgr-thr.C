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

#include "common/h/headers.h"
#include "dyninstAPI/src/dyn_lwp.h"
#include "dyninstAPI/src/dyn_thread.h"
#include "dyninstAPI/src/process.h"
#include "dyninstAPI/src/rpcMgr.h"

int rpcThr::postIRPC(inferiorRPCtoDo *todo) {
    postedRPCs_.push_back(todo);
    return todo->id;
}

bool rpcThr::isProcessingIRPC() const {
    return isRunningIRPC() || isWaitingForBreakpoint();
}

bool rpcThr::isRunningIRPC() const {
    return runningRPC_ != NULL;
}

bool rpcThr::isWaitingForBreakpoint() const {
    return pendingRPC_ != NULL;
}

bool rpcThr::isReadyForIRPC() const {
    if (isProcessingIRPC()) {
        return false;
    }
    if (postedRPCs_.size() > 0) {
        return true;
    }
    if (mgr_->postedProcessRPCs_.size() > 0) {
        return true;
    }
    return false;
}

// Launch an inferior RPC
// Two cases: 
// 1) We have a pending RPC (in pendingRPC_) that we already prepped
//    and we want to run it (and a system call breakpoint was set)
// 2) We don't have a pending IRPC but there is one on the queue.
irpcLaunchState_t rpcThr::launchThrIRPC(bool runProcWhenDone) {

    
    if (runningRPC_ || pendingRPC_) {
        return irpcError;
    }

#if defined(sparc_sun_solaris2_4)
    if(mgr_->proc()->multithread_capable()) {
       if (postedRPCs_.size() == 0)
          return irpcNoIRPC;
    } else
#endif
    {
    if(postedRPCs_.size() == 0 && mgr_->postedProcessRPCs_.size() == 0)
       return irpcNoIRPC;
    }


    // We can run the RPC if we're not currently in a system call.
    // This is defined as "any time we can't modify the state of the
    // process". In this case we try and set a breakpoint when we leave
    // the system call. If we can't set the breakpoint we poll.
    dyn_lwp *lwp = thr_->get_lwp();

    // Check if we're in a system call
    if (lwp->executingSystemCall()) {
        // We can't do any work. If there is a pending RPC try
        // to set a breakpoint at the exit of the call
        if (postedRPCs_.size() > 0) {
            if (lwp->setSyscallExitTrap(launchThrIRPCCallbackDispatch,
                                        (void *)this)) {
                // If there is an RPC queued we set it up as pending
                // and record it
                if (!pendingRPC_) {
                    pendingRPC_ = new inferiorRPCinProgress;
                    pendingRPC_->rpc = postedRPCs_[0];
                    pendingRPC_->runProcWhenDone = runProcWhenDone;
                    // Delete that iRPC (clunky)
                    pdvector<inferiorRPCtoDo *> newRPCs;
                    for (unsigned k = 1; k < postedRPCs_.size(); k++)
                        newRPCs.push_back(postedRPCs_[k]);
                    postedRPCs_ = newRPCs;
                    mgr_->addPendingRPC(pendingRPC_);
                }
                return irpcBreakpointSet;
            }
            else {
                // Weren't able to set the breakpoint, so all we can
                // do is try later
                // Don't set pending if we're polling.
                assert(!pendingRPC_);
                return irpcAgain;
            }
        }
        else {
			/* We assume that some other thread will run the process IRPC(s). */
            return irpcNoIRPC;
        }
    }

    // Get the RPC and slap it in the pendingRPC_ pointer
    if (!pendingRPC_) {
        pendingRPC_ = new inferiorRPCinProgress;
        if (postedRPCs_.size() > 0) {       
            pendingRPC_->rpc = postedRPCs_[0];
            // Delete that iRPC (clunky)
            pdvector<inferiorRPCtoDo *> newRPCs;
            for (unsigned k = 1; k < postedRPCs_.size(); k++)
                newRPCs.push_back(postedRPCs_[k]);
            postedRPCs_ = newRPCs;
            pendingRPC_->isProcessRPC = false;
        }
        else {
            // Take a process-wide RPC
            pendingRPC_->rpc = mgr_->getProcessRPC();
            // And it's running on this thread
            pendingRPC_->rpc->thr = thr_;
            pendingRPC_->isProcessRPC = true;
        }
        pendingRPC_->runProcWhenDone = runProcWhenDone;
        mgr_->addPendingRPC(pendingRPC_);
    }
    return runPendingIRPC();
}

irpcLaunchState_t rpcThr::runPendingIRPC() {
    if (!pendingRPC_) {
        return irpcNoIRPC;
    }

    
    dyn_lwp *lwp = thr_->get_lwp();


    // We passed the system call check, so the thread is in a state
    // where it is possible to run iRPCs.
    struct dyn_saved_regs *theSavedRegs = new dyn_saved_regs;
    // Some platforms save daemon-side, some save process-side (on the stack)
    // Should unify this.
    bool status = lwp->getRegisters(theSavedRegs);
    if (status == false) {
        // Can happen if we're in a syscall, which is caught above
        return irpcError;
    }
    assert(theSavedRegs);
    
    // RPC is actually going to be running
    runningRPC_ = pendingRPC_;
    pendingRPC_ = NULL;
    mgr_->addRunningRPC(runningRPC_);
    
    // Build the in progress struct
    runningRPC_->savedRegs = theSavedRegs;
    runningRPC_->rpcthr = this;
    runningRPC_->rpclwp = NULL;
    
    runningRPC_->rpcStartAddr =
    mgr_->createRPCImage(runningRPC_->rpc->action, // AST tree
                         runningRPC_->rpc->noCost,
                         (runningRPC_->rpc->callbackFunc != NULL), // Callback?
                         runningRPC_->rpcCompletionAddr, // Fills in 
                         runningRPC_->rpcResultAddr,
                         runningRPC_->rpcContPostResultAddr,
                         runningRPC_->resultRegister,
                         runningRPC_->rpc->lowmem); // Where to allocate

    if (!runningRPC_->rpcStartAddr) {
        cerr << "launchRPC failed, couldn't create image" << endl;
        return irpcError;
    }

#if !defined(i386_unknown_nt4_0) \
 && !defined(mips_unknown_ce2_11)
    // Actually, only need this if a restoreRegisters won't reset
    // the PC back to the original value
    Frame curFrame = lwp->getActiveFrame();
    runningRPC_->origPC = curFrame.getPC();
#endif    

    // Save the current stack (to fake stack walks while we're in
    // the RPC)
    thr_->savePreRPCStack();
    // Launch this sucker. Change the PC, and the caller will set running
#if !defined(ia64_unknown_linux2_4)
    if (!lwp->changePC(runningRPC_->rpcStartAddr, NULL)) {
#else
	/* Syscalls can actually rewind the PC by 0x10, so we need
	   a bundle _before_ the new PC to check for this. */
    if (!lwp->changePC(runningRPC_->rpcStartAddr + 0x10, NULL)) {
#endif
        cerr << "launchRPC failed: couldn't set PC" << endl;
        return irpcError;
    }
    
#if defined(i386_unknown_linux2_0) \
 || defined(x86_64_unknown_linux2_4) /* Blind duplication - Ray */
      // handle system call interruption:
      // if we interupted a system call, this clears the state that
      // would cause the OS to restart the call when we run the rpc.
      // if we did not, this is a no-op.
      // after the rpc, an interrupted system call will
      // be restarted when we restore the original
      // registers (restore undoes clearOPC)
      // note that executingSystemCall is always false on this platform.
      if (!lwp->clearOPC())
      {
         cerr << "launchRPC failed because clearOPC() failed"
	      << endl;
         return irpcError;
      }
#endif

      return irpcStarted;

}

bool rpcThr::deleteThrIRPC(unsigned id) {
    // Can remove a queued or pending thr IRPC
    bool removed = false;

    if (pendingRPC_ && pendingRPC_->rpc->id == id) {
       // we don't want to do as we normally do when a exit trap occurs,
       // that is to run the rpc, which gets triggered by this callback
       get_thr()->get_lwp()->clearSyscallExitTrapCallback();
       get_thr()->get_lwp()->clearSyscallExitTrap();
       
        delete pendingRPC_->rpc;
        delete pendingRPC_;
        
        pendingRPC_ = NULL;

        // Should probably also remove a system call trap
        // if one exists

        return true;
    }
    
    pdvector<inferiorRPCtoDo *> newPostedRPCs;
    for (unsigned i = 0; i < postedRPCs_.size(); i++) {
        if (postedRPCs_[i]->id == id) {
            removed = true;
        }
        else {
            newPostedRPCs.push_back(postedRPCs_[i]);
        }
    }
    postedRPCs_ = newPostedRPCs;
    return removed;
}

bool rpcThr::handleCompletedIRPC() {
    // The LWP can be a different one than the lwp the RPC was originally
    // started on if the thread was migrated.
    dyn_lwp *lwp = thr_->get_lwp();
    assert(lwp);

#if defined(sparc_sun_solaris2_4)    
    if(mgr_->proc()->multithread_capable() && runningRPC_->isProcessRPC) {
        mgr_->processingProcessRPC = false;
    }
#endif

    // step 1) restore registers:
    if (runningRPC_->savedRegs) {
        if (!lwp->restoreRegisters(*runningRPC_->savedRegs)) {
            cerr << "handleCompletedIRPC failed because restoreRegisters failed" << endl;
            assert(false);
        }
        delete runningRPC_->savedRegs;
        runningRPC_->savedRegs = NULL;
        // The above implicitly must restore the PC.
    }
    else
        if (!lwp->changePC(runningRPC_->origPC, NULL)) 
            assert(0 && "Failed to reset PC");
    
    // step 2) delete temp tramp
    process *proc = lwp->proc();
    proc->inferiorFree(runningRPC_->rpcStartAddr);

    // save enough information to call the callback function, if needed
    inferiorRPCcallbackFunc cb = runningRPC_->rpc->callbackFunc;
    void* userData = runningRPC_->rpc->userData;
    void* resultValue = runningRPC_->resultValue;
    unsigned id = runningRPC_->rpc->id;

    // Save whether we were running or not
    bool runProcess = runningRPC_->runProcWhenDone;
    
    // And blow away the data structures
    mgr_->removeRunningRPC(runningRPC_);
    delete runningRPC_->rpc;
    delete runningRPC_;
    runningRPC_ = NULL;
    thr_->clearPreRPCStack();
    

    // step 3) invoke user callback, if any
    // call the callback function if needed
    if( cb != NULL ) {
        (*cb)(proc, id, userData, resultValue);
    }

    // Before we continue the process: if there is another RPC,
    // start it immediately (instead of waiting for an event loop
    // to do the job)
    if (isReadyForIRPC()) {
        irpcLaunchState_t launchState =
        launchThrIRPC(runProcess);
        if (launchState == irpcStarted) {
            // Run the process
            return true;
        }
    }
       
    return runProcess;
}

// Get the return value (preperatory for a callback)
bool rpcThr::getReturnValueIRPC() {
    if (!runningRPC_ || !runningRPC_->rpc->callbackFunc)
        return false;
    Address returnValue = 0;
    
    if (runningRPC_->resultRegister != Null_Register) {
        // We have a result that we care about
        returnValue = thr_->get_lwp()->readRegister(runningRPC_->resultRegister);
        // Okay, this bit I don't understand. 
        // Oh, crud... we should have a register space for each thread.
        // Or not do this at all. 
        extern registerSpace *regSpace;
        regSpace->freeRegister(runningRPC_->resultRegister);
    }
    runningRPC_->resultValue = (void *)returnValue;
    // we continue the process...but not quite at the PC where we left off,
    // since that will just re-do the trap!  Instead, we need to continue at
    // the location of the next instruction.
    if (!thr_->get_lwp()->changePC(runningRPC_->rpcContPostResultAddr, NULL))
        assert(false);
    return true;
}

void rpcThr::launchThrIRPCCallbackDispatch(dyn_lwp * /*lwp*/,
                                           void *data)
{
    rpcThr *thr = (rpcThr *)data;
    // the runProcess code here is ignored if a pending RPC
    // is already set (which must be true for this callback to
    // happen)
    thr->runPendingIRPC();
}


// Launch an inferior RPC
// Two cases: 
// 1) We have a pending RPC (in pendingRPC_) that we already prepped
//    and we want to run it (and a system call breakpoint was set)
// 2) We don't have a pending IRPC but there is one on the queue.
irpcLaunchState_t rpcThr::launchProcIRPC(bool runProcWhenDone) {

    // Most important thing: find a LWP that's not blocked in a system call

    if (runningRPC_ || pendingRPC_) {
        return irpcError;
    }
    
    if (mgr_->postedProcessRPCs_.size() == 0)
        return irpcNoIRPC;

    // We can run the RPC if we're not currently in a system call.
    // This is defined as "any time we can't modify the state of the
    // process". In this case we try and set a breakpoint when we leave
    // the system call. If we can't set the breakpoint we poll.
    dyn_lwp *lwp = thr_->get_lwp();

    // Check if we're in a system call
    if (lwp->executingSystemCall()) {
        // No RPCs anyway
        return irpcError;
    }

    // Get the RPC and slap it in the pendingRPC_ pointer
    pendingRPC_ = new inferiorRPCinProgress;
    // Take a process-wide RPC
    pendingRPC_->rpc = mgr_->getProcessRPC();
    // And it's running on this thread
    pendingRPC_->rpc->thr = thr_;
    pendingRPC_->isProcessRPC = true;
    
    pendingRPC_->runProcWhenDone = runProcWhenDone;
    mgr_->addPendingRPC(pendingRPC_);

    return runPendingIRPC();
}

