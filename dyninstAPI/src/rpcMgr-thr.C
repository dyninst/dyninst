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

#include "common/h/headers.h"
#include "dyninstAPI/src/dyn_lwp.h"
#include "dyninstAPI/src/dyn_thread.h"
#include "dyninstAPI/src/process.h"
#include "dyninstAPI/src/rpcMgr.h"
#include "dyninstAPI/src/mailbox.h"
#include "dyninstAPI/src/callbacks.h"

#if defined(os_vxworks)
#include "vxworks.h"
#endif

rpcThr::rpcThr(rpcThr *parT, rpcMgr *cM, dyn_thread *cT) :
    mgr_(cM),
    thr_(cT),
    pendingRPC_(NULL),
    runningRPC_(NULL)
{
    for (unsigned i = 0; i < parT->postedRPCs_.size(); i++) {
        inferiorRPCtoDo *newRPC = new inferiorRPCtoDo;
        inferiorRPCtoDo *oldRPC = parT->postedRPCs_[i];

        newRPC->action = oldRPC->action;
        newRPC->noCost = oldRPC->noCost;
        newRPC->callbackFunc = oldRPC->callbackFunc;
        newRPC->userData = oldRPC->userData;
        newRPC->lowmem = oldRPC->lowmem;
        newRPC->id = oldRPC->id;
        assert(oldRPC->thr);
        newRPC->thr = cT;
        assert(!oldRPC->lwp);
        postedRPCs_.push_back(newRPC);
    }
    
    if (parT->pendingRPC_) {
        inferiorRPCinProgress *newProg = new inferiorRPCinProgress;
        inferiorRPCinProgress *oldProg = parT->pendingRPC_;
        
        inferiorRPCtoDo *newRPC = new inferiorRPCtoDo;
        inferiorRPCtoDo *oldRPC = oldProg->rpc;

        newRPC->action = oldRPC->action;
        newRPC->noCost = oldRPC->noCost;
        newRPC->callbackFunc = oldRPC->callbackFunc;
        newRPC->userData = oldRPC->userData;
        newRPC->lowmem = oldRPC->lowmem;
        newRPC->id = oldRPC->id;
        assert(oldRPC->thr);
        newRPC->thr = cT;
        assert(!oldRPC->lwp);
        
        newProg->rpc = newRPC;
        if (oldProg->savedRegs) {
            newProg->savedRegs = new dyn_saved_regs;
            memcpy(newProg->savedRegs, oldProg->savedRegs, sizeof(dyn_saved_regs));
        }
        else newProg->savedRegs = NULL;
        newProg->origPC = oldProg->origPC;
        newProg->runProcWhenDone = oldProg->runProcWhenDone;
	newProg->rpcBaseAddr = oldProg->rpcBaseAddr;
        newProg->rpcStartAddr = oldProg->rpcStartAddr;
        newProg->rpcResultAddr = oldProg->rpcResultAddr;
        newProg->rpcContPostResultAddr = oldProg->rpcContPostResultAddr;
        newProg->rpcCompletionAddr = oldProg->rpcCompletionAddr;
        newProg->resultRegister = oldProg->resultRegister;
        newProg->resultValue = oldProg->resultValue;
        
        newProg->rpcthr = this;
        newProg->rpclwp = NULL;
        newProg->isProcessRPC = oldProg->isProcessRPC;
        newProg->state = oldProg->state;
        
        pendingRPC_ =  newProg;
    }
        

    if (parT->runningRPC_) {
        inferiorRPCinProgress *newProg = new inferiorRPCinProgress;
        inferiorRPCinProgress *oldProg = parT->runningRPC_;
        
        inferiorRPCtoDo *newRPC = new inferiorRPCtoDo;
        inferiorRPCtoDo *oldRPC = oldProg->rpc;

        newRPC->action = oldRPC->action;
        newRPC->noCost = oldRPC->noCost;
        newRPC->callbackFunc = oldRPC->callbackFunc;
        newRPC->userData = oldRPC->userData;
        newRPC->lowmem = oldRPC->lowmem;
        newRPC->id = oldRPC->id;
        assert(oldRPC->thr);
        newRPC->thr = cT;
        assert(!oldRPC->lwp);
        
        newProg->rpc = newRPC;
        if (oldProg->savedRegs) {
            newProg->savedRegs = new dyn_saved_regs;
            memcpy(newProg->savedRegs, oldProg->savedRegs, sizeof(dyn_saved_regs));
        }
        else newProg->savedRegs = NULL;
        newProg->origPC = oldProg->origPC;
        newProg->runProcWhenDone = oldProg->runProcWhenDone;
	newProg->rpcBaseAddr = oldProg->rpcBaseAddr;
        newProg->rpcStartAddr = oldProg->rpcStartAddr;
        newProg->rpcResultAddr = oldProg->rpcResultAddr;
        newProg->rpcContPostResultAddr = oldProg->rpcContPostResultAddr;
        newProg->rpcCompletionAddr = oldProg->rpcCompletionAddr;
        newProg->resultRegister = oldProg->resultRegister;
        newProg->resultValue = oldProg->resultValue;
        
        newProg->rpcthr = this;
        newProg->rpclwp = NULL;
        newProg->isProcessRPC = oldProg->isProcessRPC;
        newProg->state = oldProg->state;
        
        runningRPC_ =  newProg;
    }
}

int rpcThr::postIRPC(inferiorRPCtoDo *todo) {
    postedRPCs_.push_back(todo);
    return todo->id;
}

bool rpcThr::isProcessingIRPC() const {
    inferiorrpc_printf("%s[%d]: call to isProcessing: isRunning %d, isWaiting %d\n",
                       FILE__, __LINE__, isRunningIRPC(), isWaitingForBreakpoint());
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
      inferiorrpc_printf("... thr %lu currently processing, sorry\n",
			 get_thr()->get_tid());
        return false;
    }
    if (postedRPCs_.size() > 0) {
      inferiorrpc_printf("... thr %lu with a thread RPC\n",
			 get_thr()->get_tid());
        return true;
    }
    if (mgr_->postedProcessRPCs_.size() > 0) {
      inferiorrpc_printf("... thr %lu picking up a process RPC\n",
			 get_thr()->get_tid());
        return true;
    }
    inferiorrpc_printf("... thr %lu with nothing to do\n",
		       get_thr()->get_tid());
    return false;
}

// Launch an inferior RPC
// Two cases: 
// 1) We have a pending RPC (in pendingRPC_) that we already prepped
//    and we want to run it (and a system call breakpoint was set)
// 2) We don't have a pending IRPC but there is one on the queue.
irpcLaunchState_t rpcThr::launchThrIRPC(bool runProcWhenDone) 
{

    if (runningRPC_ || pendingRPC_) {
        return irpcError;
    }

    if(postedRPCs_.size() == 0 && mgr_->postedProcessRPCs_.size() == 0)
        return irpcNoIRPC;

    // We can run the RPC if we're not currently in a system call.
    // This is defined as "any time we can't modify the state of the
    // process". In this case we try and set a breakpoint when we leave
    // the system call. If we can't set the breakpoint we poll.
    dyn_lwp *lwp = thr_->get_lwp();
    
    if (mgr_->proc()->IndependentLwpControl()) {
        if (lwp->status() == running) runProcWhenDone = true;
        lwp->pauseLWP();
    }

    // That call can handle process signals (I'm not kidding) at which point
    // someone else picked up our iRPC... seen with Paradyn

    if(postedRPCs_.size() == 0 && mgr_->postedProcessRPCs_.size() == 0) {
        if (mgr_->proc()->IndependentLwpControl() && runProcWhenDone)
            lwp->continueLWP(NoSignal, false);
        return irpcNoIRPC;
    }

#if defined(cap_syscall_trap)
    // Check if we're in a system call
    if (lwp->executingSystemCall()) {
        // We can't do any work. If there is a pending RPC try
        // to set a breakpoint at the exit of the call

        // See if we have a thread-specific iRPC. If not, we're talking process...
        // and that means that we just went into a system call. 

        if (postedRPCs_.size() == 0) 
            return irpcNoIRPC;
        
        if (lwp->setSyscallExitTrap(launchThrIRPCCallbackDispatch,
                                    (void *)this)) {
            // If there is an RPC queued we set it up as pending
            // and record it
            if (!pendingRPC_) {
                pendingRPC_ = new inferiorRPCinProgress;
                assert(postedRPCs_.size());
                pendingRPC_->rpc = postedRPCs_[0];
                pendingRPC_->runProcWhenDone = runProcWhenDone;
                // Delete that iRPC (clunky)
                pdvector<inferiorRPCtoDo *> newRPCs;
                VECTOR_ERASE(postedRPCs_,0,0);
                mgr_->addPendingRPC(pendingRPC_);
            }
            if (mgr_->proc()->IndependentLwpControl() && runProcWhenDone)
                lwp->continueLWP();
            return irpcBreakpointSet;
        }
        else {
            // Weren't able to set the breakpoint, so all we can
            // do is try later
            // Don't set pending if we're polling.
            assert(!pendingRPC_);
            if (mgr_->proc()->IndependentLwpControl() && runProcWhenDone)
                lwp->continueLWP();
            return irpcAgain;
        }
    }
#endif // cap_syscall_trap

    // Get the RPC and slap it in the pendingRPC_ pointer
    if (!pendingRPC_) {
        inferiorrpc_printf("%s[%d]: ready to run, creating pending RPC structure\n",
                           FILE__, __LINE__);
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
    assert(pendingRPC_);
    return runPendingIRPC();
}

irpcLaunchState_t rpcThr::runPendingIRPC() 
{
    if (!pendingRPC_) {
        return irpcNoIRPC;
    }

    inferiorrpc_printf("%s[%d]: running a pending IRPC on thread %lu\n",
                       FILE__, __LINE__, thr_->get_tid());
    
    dyn_lwp *lwp = thr_->get_lwp();

    // We passed the system call check, so the thread is in a state
    // where it is possible to run iRPCs.
    struct dyn_saved_regs *theSavedRegs = new dyn_saved_regs;
    // Some platforms save daemon-side, some save process-side (on the stack)
    // Should unify this.
    bool saveFP = pendingRPC_->rpc->saveFPState;
    bool status = lwp->getRegisters(theSavedRegs, saveFP);
    inferiorrpc_printf("RPC saved registers: %d\n", status);

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
    
    runningRPC_->rpcBaseAddr =
    mgr_->createRPCImage(runningRPC_->rpc->action, // AST tree
                         runningRPC_->rpc->noCost,
                         (runningRPC_->rpc->callbackFunc != NULL), // Callback?
                         runningRPC_->rpcStartAddr, // Fills in
                         runningRPC_->rpcCompletionAddr, 
                         runningRPC_->rpcResultAddr,
                         runningRPC_->rpcContPostResultAddr,
                         runningRPC_->resultRegister,
                         runningRPC_->rpc->lowmem, 
                         thr_,
                         lwp ); // Where to allocate

    if (!runningRPC_->rpcBaseAddr) {
        cerr << "launchRPC failed, couldn't create image" << endl;
        return irpcError;
    }
    
    /* Why we don't just pass runningRPC_ into createRPCImage()... */
    mgr_->proc()->addOrigRange( runningRPC_ );

#if !defined(i386_unknown_nt4_0) \
 && !defined(mips_unknown_ce2_11)
    // Actually, only need this if a restoreRegisters won't reset
    // the PC back to the original value
    Frame curFrame = lwp->getActiveFrame();
    runningRPC_->origPC = curFrame.getPC();
    inferiorrpc_printf("%s[%d]: thread %lu at PC 0x%lx, saving and setting to 0x%lx\n",
                       FILE__, __LINE__, thr_->get_tid(), runningRPC_->origPC, runningRPC_->rpcStartAddr);
#endif    

    // Launch this sucker. Change the PC, and the caller will set running
    if (!lwp->changePC(runningRPC_->rpcStartAddr, NULL)) {
        cerr << "launchRPC failed: couldn't set PC" << endl;
        return irpcError;
    }
    inferiorrpc_printf("Changed PC to addr 0x%lx\n", runningRPC_->rpcStartAddr);
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

#if defined(os_vxworks)
      // No signals on VxWorks
      if (runningRPC_->rpcResultAddr)
          addBreakpoint(runningRPC_->rpcResultAddr);
      addBreakpoint(runningRPC_->rpcCompletionAddr);
#endif

      if (mgr_->proc()->IndependentLwpControl())
          lwp->continueLWP(NoSignal, false);

      return irpcStarted;

}

bool rpcThr::deleteThrIRPC(unsigned id) {
    // Can remove a queued or pending thr IRPC
    bool removed = false;

    if (pendingRPC_ && pendingRPC_->rpc->id == id) {
       // we don't want to do as we normally do when a exit trap occurs,
       // that is to run the rpc, which gets triggered by this callback
#if defined(cap_syscall_trap)
       get_thr()->get_lwp()->clearSyscallExitTrap();
#endif
       
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

bool rpcThr::cancelThrIRPC(unsigned id) {
  // It better be running...
  assert (runningRPC_->rpc->id == id);

  // We handle it as a completed, no-callback IRPC
  runningRPC_->rpc->callbackFunc = NULL; /* Void the callback */

  // I don't want to deal with "it finished" races...
  assert(mgr_->proc()->isStopped());

  return handleCompletedIRPC();
}

bool rpcThr::handleCompletedIRPC() 
{
    // The LWP can be a different one than the lwp the RPC was originally
    // started on if the thread was migrated.
    dyn_lwp *lwp = thr_->get_lwp();
    assert(lwp);

#if 0
#if defined(sparc_sun_solaris2_4)    
    if(mgr_->proc()->multithread_capable() && runningRPC_->isProcessRPC) {
        mgr_->processingProcessRPC = false;
    }
#endif
#endif

    inferiorrpc_printf("Completed thread RPC %d on thread %lu\n", runningRPC_->rpc->id, thr_->get_tid());

    // step 1) restore registers:
    if (runningRPC_->savedRegs) {
        bool savedFP = runningRPC_->rpc->saveFPState;
        if (!lwp->restoreRegisters(*runningRPC_->savedRegs, savedFP)) {

            cerr << "handleCompletedIRPC failed because restoreRegisters failed"
                 << endl;
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
    proc->removeOrigRange(runningRPC_);
    proc->inferiorFree(runningRPC_->rpcBaseAddr);

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

    int retstate = 0;

    // step 3) invoke user callback, if any
    // call the callback function if needed
    if( cb != NULL ) {
        inferiorrpc_printf("%s[%d][%s]:  about to exec/register rpc done callback\n", 
                           FILE__, __LINE__, getThreadStr(getExecThreadID()));
        retstate = cb(proc, id, userData, resultValue);
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
    
    if (retstate == RPC_RUN_WHEN_DONE)
        return true;
    
    return runProcess;
}

// Get the return value (preperatory for a callback)
bool rpcThr::getReturnValueIRPC() 
{
    if (!runningRPC_ || !runningRPC_->rpc->callbackFunc)
        return false;

    Address returnValue = 0;

    dyn_lwp *thr_lwp = thr_->get_lwp();

    // make sure lwp is stopped
    if(thr_lwp->status() != stopped) {
       return false;
    }
    
    inferiorrpc_printf("%s[%d]: Getting return value for irpc %d, thr %lu\n",
                       FILE__, __LINE__, runningRPC_->rpc->id, thr_->get_tid());

    if (runningRPC_->resultRegister != Null_Register) {
        // We have a result that we care about
        returnValue = thr_lwp->readRegister(runningRPC_->resultRegister);
    }

    inferiorrpc_printf("%s[%d]: return value is %p\n", FILE__, __LINE__, returnValue);

    runningRPC_->resultValue = (void *)returnValue;
    // we continue the process...but not quite at the PC where we left off,
    // since that will just re-do the trap!  Instead, we need to continue at
    // the location of the next instruction.
    inferiorrpc_printf("%s[%d]: changing RPC PC to continue address 0x%lx\n",
                       FILE__, __LINE__, runningRPC_->rpcContPostResultAddr);
    if (! thr_lwp->changePC(runningRPC_->rpcContPostResultAddr, NULL)) {
        // What if we're exited?
        inferiorrpc_printf("%s[%d]: FAILED TO SET continue PC\n",
                           FILE__, __LINE__);
        return false;
    }

    return true;
}

bool rpcThr::launchThrIRPCCallbackDispatch(dyn_lwp * /*lwp*/,
                                           void *data)
{
    rpcThr *thr = (rpcThr *)data;
    // the runProcess code here is ignored if a pending RPC
    // is already set (which must be true for this callback to
    // happen)
    irpcLaunchState_t launchState = thr->runPendingIRPC();
    if (launchState == irpcStarted)
      return true;
    return false;
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
    
    inferiorrpc_printf("Thread %lu, lwp %u, checking status...\n", thr_->get_tid(), lwp->get_lwp_id());

#if defined(cap_syscall_trap)    
    // Check if we're in a system call
    if (lwp->executingSystemCall()) {
        // No RPCs anyway
        return irpcError;
    }
#endif

    inferiorrpc_printf("Status is go, grabbing process RPC and running\n");
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

