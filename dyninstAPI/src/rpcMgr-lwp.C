/*
 * Copyright (c) 1996-2011 Barton P. Miller
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
#include "dyninstAPI/src/process.h"
#include "dyninstAPI/src/rpcMgr.h"
#include "dyninstAPI/src/mailbox.h"
#include "dyninstAPI/src/callbacks.h"
#include "dyninstAPI/src/registerSpace.h"
#include "dyninstAPI/src/dyn_lwp.h"

rpcLWP::rpcLWP(rpcLWP *parL, rpcMgr *cM, dyn_lwp *cL) :
    mgr_(cM),
    lwp_(cL),
    pendingRPC_(NULL),
    runningRPC_(NULL)
{
    for (unsigned i = 0; i < parL->postedRPCs_.size(); i++) {
        inferiorRPCtoDo *newRPC = new inferiorRPCtoDo;
        inferiorRPCtoDo *oldRPC = parL->postedRPCs_[i];

        newRPC->action = oldRPC->action;
        newRPC->noCost = oldRPC->noCost;
        newRPC->callbackFunc = oldRPC->callbackFunc;
        newRPC->userData = oldRPC->userData;
        newRPC->lowmem = oldRPC->lowmem;
        newRPC->id = oldRPC->id;
        assert(!oldRPC->thr);
        assert(oldRPC->lwp);
        newRPC->lwp = cL;
        postedRPCs_.push_back(newRPC);
    }
    if (parL->pendingRPC_) {
        inferiorRPCinProgress *newProg = new inferiorRPCinProgress;
        inferiorRPCinProgress *oldProg = parL->pendingRPC_;
        
        inferiorRPCtoDo *newRPC = new inferiorRPCtoDo;
        inferiorRPCtoDo *oldRPC = oldProg->rpc;

        newRPC->action = oldRPC->action;
        newRPC->noCost = oldRPC->noCost;
        newRPC->callbackFunc = oldRPC->callbackFunc;
        newRPC->userData = oldRPC->userData;
        newRPC->lowmem = oldRPC->lowmem;
        newRPC->id = oldRPC->id;
        assert(!oldRPC->thr);
        newRPC->lwp = cL;
        assert(oldRPC->lwp);
        
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
        
        newProg->rpcthr = NULL;
        newProg->rpclwp = this;
        newProg->isProcessRPC = oldProg->isProcessRPC;
        newProg->state = oldProg->state;
        
        pendingRPC_ =  newProg;
    }
        
    if (parL->runningRPC_) {
        inferiorRPCinProgress *newProg = new inferiorRPCinProgress;
        inferiorRPCinProgress *oldProg = parL->runningRPC_;
        
        inferiorRPCtoDo *newRPC = new inferiorRPCtoDo;
        inferiorRPCtoDo *oldRPC = oldProg->rpc;

        newRPC->action = oldRPC->action;
        newRPC->noCost = oldRPC->noCost;
        newRPC->callbackFunc = oldRPC->callbackFunc;
        newRPC->userData = oldRPC->userData;
        newRPC->lowmem = oldRPC->lowmem;
        newRPC->id = oldRPC->id;
        assert(!oldRPC->thr);
        newRPC->lwp = cL;
        assert(oldRPC->lwp);
        
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
        
        newProg->rpcthr = NULL;
        newProg->rpclwp = this;
        newProg->isProcessRPC = oldProg->isProcessRPC;
        newProg->state = oldProg->state;
        
        runningRPC_ =  newProg;
    }
}
    

int rpcLWP::postIRPC(inferiorRPCtoDo *todo) {
    postedRPCs_.push_back(todo);
    return todo->id;
}

bool rpcLWP::isProcessingIRPC() const {
    return isRunningIRPC() || isWaitingForBreakpoint();
}

bool rpcLWP::isRunningIRPC() const {
    return runningRPC_ != NULL;
}

bool rpcLWP::isWaitingForBreakpoint() const {
    return pendingRPC_ != NULL;
}

// We _do not_ check process-wide RPCs here
bool rpcLWP::isReadyForIRPC() const {
    if (isProcessingIRPC()) {
       fprintf(stderr, "[%s:%u] - Already processing rpc\n", __FILE__, __LINE__);
        return false;    }
    if (postedRPCs_.size() > 0) {
        return true;
    }
    
    return false;
}

// Launch an inferior RPC
// Two cases: 
// 1) We have a pending RPC (in pendingRPC_) that we already prepped
//    and we want to run it (and a system call breakpoint was set)
// 2) We don't have a pending IRPC but there is one on the queue.
irpcLaunchState_t rpcLWP::launchLWPIRPC(bool runProcWhenDone) {
    if (runningRPC_ || pendingRPC_) return irpcError;
    
    if (postedRPCs_.size() == 0)
        // LWPs don't run proc-wide
        return irpcNoIRPC;

    // We can run the RPC if we're not currently in a system call.
    // This is defined as "any time we can't modify the state of the
    // process". In this case we try and set a breakpoint when we leave
    // the system call. If we can't set the breakpoint we poll.

    if (mgr_->proc()->IndependentLwpControl())
        lwp_->pauseLWP(true);
    
#if defined(cap_syscall_trap)
    // Check if we're in a system call
    if (lwp_->executingSystemCall()) {
        // We can't do any work. If there is a pending RPC try
        // to set a breakpoint at the exit of the call
        if (lwp_->setSyscallExitTrap(launchLWPIRPCCallbackDispatch,
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
            if (mgr_->proc()->IndependentLwpControl())
                lwp_->continueLWP(true, false);
            return irpcBreakpointSet;
        }
        else {
            // Weren't able to set the breakpoint, so all we can
            // do is try later
            // Don't set pending if we're polling.
            assert(!pendingRPC_);
            if (mgr_->proc()->IndependentLwpControl())
                lwp_->continueLWP(true, false);
            return irpcAgain;
        }
    }
#endif // cap_syscall_trap
    
    // Get the RPC and slap it in the postedRPC_ pointer
    if (!pendingRPC_) {
        pendingRPC_ = new inferiorRPCinProgress;
        if (postedRPCs_.size() > 0) {       
            pendingRPC_->rpc = postedRPCs_[0];
            // Delete that iRPC (clunky)
            pdvector<inferiorRPCtoDo *> newRPCs;
            for (unsigned k = 1; k < postedRPCs_.size(); k++)
                newRPCs.push_back(postedRPCs_[k]);
            postedRPCs_ = newRPCs;
        }
        pendingRPC_->runProcWhenDone = runProcWhenDone;
        mgr_->addPendingRPC(pendingRPC_);
    }
    
    return runPendingIRPC();
}

irpcLaunchState_t rpcLWP::runPendingIRPC() {
    // CHECK FOR SYSTEM CALL STATUS
    assert(pendingRPC_);

    // We passed the system call check, so the lwp is in a state
    // where it is possible to run iRPCs.
    struct dyn_saved_regs *theSavedRegs = NULL;
    // Some platforms save daemon-side, some save process-side (on the stack)
    // Should unify this.
    theSavedRegs = new dyn_saved_regs;

    Frame frame = lwp_->getActiveFrame();
    inferiorrpc_printf("%s[%d]: original PC at start of iRPC is 0x%lx\n", FILE__, __LINE__, frame.getPC());
    
    bool saveFP = pendingRPC_->rpc->saveFPState;
    bool status = lwp_->getRegisters(theSavedRegs, saveFP);
    if(status != true) {
        // Can happen if we're in a syscall, which is caught above
        return irpcError;
    }
    // RPC is actually going to be running
    runningRPC_ = pendingRPC_;
    pendingRPC_ = NULL;
    
    // Build the in progress struct
    runningRPC_->savedRegs = theSavedRegs;
    runningRPC_->rpcthr = NULL;
    runningRPC_->rpclwp = this;

    mgr_->addRunningRPC(runningRPC_);

    runningRPC_->rpcBaseAddr =
    mgr_->createRPCImage(runningRPC_->rpc->action, // AST tree
                         runningRPC_->rpc->noCost,
                         (runningRPC_->rpc->callbackFunc != NULL), // Callback?
                         runningRPC_->rpcStartAddr,
                         runningRPC_->rpcCompletionAddr, // Fills in 
                         runningRPC_->rpcResultAddr,
                         runningRPC_->rpcContPostResultAddr,
                         runningRPC_->resultRegister,
                         runningRPC_->rpc->lowmem, 
                         NULL, lwp_); // Where to allocate
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
    Frame curFrame = lwp_->getActiveFrame();
    runningRPC_->origPC = curFrame.getPC();
#endif    

    // Launch this sucker. Change the PC, and the caller will set running
    if (!lwp_->changePC(runningRPC_->rpcStartAddr, NULL)) {
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
      if (!lwp_->clearOPC())
      {
         cerr << "launchRPC failed because clearOPC() failed" << endl;
         return irpcError;
      }
#endif

      if (mgr_->proc()->IndependentLwpControl()) {
          signal_printf("%s[%d]: Continuing lwp %d\n", FILE__, __LINE__, lwp_->get_lwp_id());
          lwp_->continueLWP(NoSignal, false);
      }

      return irpcStarted;
}

bool rpcLWP::deleteLWPIRPC(unsigned id) {
    // Can remove a queued or pending lwp IRPC
    bool removed = false;
    
    if (pendingRPC_ && pendingRPC_->rpc->id == id) {
       // we don't want to do as we normally do when a exit trap occurs,
       // that is to run the rpc, which gets triggered by this callback
#if defined(cap_syscall_trap)
       get_lwp()->clearSyscallExitTrap();
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

bool rpcLWP::cancelLWPIRPC(unsigned id) {
  // It better be running...
  assert (runningRPC_->rpc->id == id);

  // We handle it as a completed, no-callback IRPC
  runningRPC_->rpc->callbackFunc = NULL; /* Void the callback */

  // I don't want to deal with "it finished" races...
  assert(mgr_->proc()->isStopped());

  return handleCompletedIRPC();
}


bool rpcLWP::handleCompletedIRPC() 
{
  inferiorrpc_cerr << "Completed lwp RPC " << runningRPC_->rpc->id << " on lwp " << lwp_->get_lwp_id() << endl;

    // step 1) restore registers:
  if (runningRPC_->savedRegs) {
      bool savedFP = runningRPC_->rpc->saveFPState;
      if (!lwp_->restoreRegisters(*runningRPC_->savedRegs, savedFP)) {
          cerr << "handleCompletedIRPC failed because restoreRegisters failed" << endl;
          assert(false);
      }
      delete runningRPC_->savedRegs;
      // The above implicitly must restore the PC.
  }
  else {
      inferiorrpc_printf("%s[%d]: odd case with no saved registers, changing PC to 0x%lx\n",
                         FILE__, __LINE__, runningRPC_->origPC);
      if (!lwp_->changePC(runningRPC_->origPC, NULL)) 
          assert(0 && "Failed to reset PC");
  }
  
  // step 2) delete temp tramp
  process *proc = lwp_->proc();
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
        retstate = cb(proc, id, userData, resultValue);
        inferiorrpc_printf("%s[%d][%s]:  registered/exec'd callback %p\n", 
                           FILE__, __LINE__, getThreadStr(getExecThreadID()), 
                           (void *) (*cb));
    }

    // Before we continue the process: if there is another RPC,
    // start it immediately (instead of waiting for an event loop
    // to do the job)
    if (isReadyForIRPC()) {
        irpcLaunchState_t launchState =
        launchLWPIRPC(runProcess);
        if (launchState == irpcStarted) {
            // Run the process
            return true;
        }
    }
    if (retstate == RPC_RUN_WHEN_DONE)  {
        return true;
    }
    return runProcess;
}

// Get the return value (preperatory for a callback)
bool rpcLWP::getReturnValueIRPC() 
{
    if (!runningRPC_ || !runningRPC_->rpc->callbackFunc)
        return false;
    Address returnValue = 0;
    
    if (runningRPC_->resultRegister != Null_Register) {
        // We have a result that we care about
        returnValue = lwp_->readRegister(runningRPC_->resultRegister);
    }

    runningRPC_->resultValue = (void *)returnValue;
    // we continue the process...but not quite at the PC where we left off,
    // since that will just re-do the trap!  Instead, we need to continue at
    // the location of the next instruction.
    if (!lwp_->changePC(runningRPC_->rpcContPostResultAddr, NULL))
        assert(false);
    return true;
}

bool rpcLWP::launchLWPIRPCCallbackDispatch(dyn_lwp * /*lwp*/,
                                           void *data)
{
    rpcLWP *lwp = (rpcLWP *)data;
    
    // the runProcess code here is ignored if a pending RPC
    // is already set (which must be true for this callback to
    // happen)
    irpcLaunchState_t launchState = lwp->runPendingIRPC();
    if (launchState == irpcStarted)
      return true; // continue
    return false;
}
