/*
 * Copyright (c) 1996-2002 Barton P. Miller
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
#include "dyninstAPI/src/instP.h" // initTramps
#include "dyninstAPI/src/process.h"
#include "dyninstAPI/src/rpcMgr.h"

// post RPC toDo for process
unsigned rpcMgr::postRPCtoDo(AstNode *action, bool noCost,
                             inferiorRPCcallbackFunc callbackFunc,
                             void *userData, bool lowmem,
                             dyn_thread *thr, dyn_lwp *lwp,
                             Address aixHACK) 
{
    static int sequence_num = 0;
    // posts an RPC, but does NOT make any effort to launch it.
    inferiorRPCtoDo *theStruct = new inferiorRPCtoDo;
    theStruct->action = action;
    theStruct->noCost = noCost;
    theStruct->callbackFunc = callbackFunc;
    theStruct->userData = userData;
    theStruct->lowmem = lowmem;
    theStruct->id = sequence_num++;
    theStruct->thr = thr;
    theStruct->lwp = lwp;
    theStruct->aixHACK = aixHACK;
    
    if (thr) {
       int index = thr->get_index();
       rpcThr *rpc_thr = thrs_[index];
       assert(rpc_thr != NULL);
       rpc_thr->postIRPC(theStruct);
    }
    else if (lwp) {
       int index = lwp->get_lwp_id();
       rpcLWP *rpc_lwp;
       bool foundIt = lwps_.find(index, rpc_lwp);
       assert(foundIt == true);
       rpc_lwp->postIRPC(theStruct);
    }
    else {
        postedProcessRPCs_.push_back(theStruct);

    }

    // Stick it in the global listing as well
    allPostedRPCs_.push_back(theStruct);

    return theStruct->id;
}

inferiorRPCtoDo *rpcMgr::getProcessRPC() {
    inferiorRPCtoDo *rpc = postedProcessRPCs_[0];
    
    // Sigh.. delete the first one (AGAIN)
    pdvector<inferiorRPCtoDo *> newRPCs;
    for (unsigned i = 1; i < postedProcessRPCs_.size(); i++)
        newRPCs.push_back(postedProcessRPCs_[i]);
    postedProcessRPCs_ = newRPCs;
    return rpc;
}

bool rpcMgr::existsReadyIRPC() const {
    if (postedProcessRPCs_.size() > 0)
        return true;
    for (unsigned i = 0; i < thrs_.size(); i++)
        if (thrs_[i]->isReadyForIRPC())
            return true;
    // CHECK LWPS TODO
    return false;
}

bool rpcMgr::existsRunningIRPC() const {
    if (allRunningRPCs_.size() > 0)
        return true;
    else
        return false;
}

bool rpcMgr::handleSignalIfDueToIRPC() {
    // For each IRPC we're running, check whether the thread
    // (or lwp) is at the PC equal to the trap address. 
    bool handledTrap = false;
    
   // The signal handler default is to leave the process paused.
   // If we want it running, we do so explicitly via a call to 
   // continueProc(). 
   bool runProcess = false;

   // Two main possibilities: a thread is stopped at an interesting address,
   // or a thread was waiting for a system call to complete (and it has).
   // We check the first case first.
   
   for (unsigned i = 0; i < allRunningRPCs_.size(); i++) {
       Frame activeFrame;
       inferiorRPCinProgress *currRPC = allRunningRPCs_[i];
       if (currRPC->rpcthr) 
           activeFrame = currRPC->rpcthr->get_thr()->getActiveFrame();
       else
           activeFrame = currRPC->rpclwp->get_lwp()->getActiveFrame();
       if (activeFrame.getPC() == currRPC->rpcResultAddr) {
           if (currRPC->rpcthr)
               currRPC->rpcthr->getReturnValueIRPC();
           else
               currRPC->rpclwp->getReturnValueIRPC();
           handledTrap = true;
           runProcess = true;
       }
       else if (activeFrame.getPC() == currRPC->rpcCompletionAddr) {
           if (currRPC->rpcthr)
               runProcess = currRPC->rpcthr->handleCompletedIRPC();
           else
               runProcess = currRPC->rpclwp->handleCompletedIRPC();
           handledTrap = true;
       }
       if (handledTrap) break;
   }
   if (handledTrap) {
       if (runProcess ||
           allRunningRPCs_.size() > 0)
           proc_->continueProc();
   }
   
   return handledTrap;
}



// Run da suckers
// Take all RPCs posted and run them (if possible)
// Return true if any RPCs were launched (and the process is running),
//   false if none were (and the process hasn't changed state)
// wasRunning: desired state of the process (as opposed to current
//  state).
// Note: if there are no RPCs running but wasRunning is true, launchRPCs
// will continue the process!

bool rpcMgr::launchRPCs(bool wasRunning) {
    // First, idiot check. If there aren't any RPCs to run, then
    // don't do anything. Reason: launchRPCs is called several times
    // a second in the daemon main loop

    if (recursionGuard) {
        // Error case: somehow launchRPCs was entered recursively
        cerr <<  "Error: inferior RPC mechanism was used in an unsafe way!" << endl;
        // Umm....
        return false;
    }

    recursionGuard = true;

    bool readyProcessRPC = false;
    bool readyLWPRPC = false;
    bool readyThrRPC = false;
    bool processingLWPRPC = false;
    // We check LWP RPCs first. If there are any they are run first -- even
    // if there is a thread RPC currently running. Only use LWP RPCs for very low
    // level operations. 
    
    // We have a central list of all posted or pending RPCs... if those are empty
    // then don't bother doing work
    if (allPostedRPCs_.size() == 0) {
        recursionGuard = false;
        return false;
    }
    
    dictionary_hash<unsigned, rpcLWP *>::iterator rpc_iter = lwps_.begin();
    while(rpc_iter != lwps_.end()) {
        rpcLWP *cur_rpc_lwp = (*rpc_iter);
        if (cur_rpc_lwp) {
            if(cur_rpc_lwp->isReadyForIRPC()) {
                readyLWPRPC = true;
                break;
            }
            if (cur_rpc_lwp->isProcessingIRPC()) {
                processingLWPRPC = true;
            }
        }
        rpc_iter++;
    }

#if defined(sparc_sun_solaris2_4)
    if(proc_->multithread_capable()) {
       if (!readyLWPRPC && !processingLWPRPC) {
          if (postedProcessRPCs_.size())
             readyProcessRPC = true;
       }
    }
#endif
    // Only run thread RPCs if there are no LWP RPCs either waiting or in flight.


    if (!readyLWPRPC && !processingLWPRPC && !readyProcessRPC && !processingProcessRPC) {
        for (unsigned i = 0; i < thrs_.size(); i++) {
           rpcThr *curThr = thrs_[i];
           if(curThr == NULL)
              continue;

            if (curThr->isReadyForIRPC()) {
                readyThrRPC = true;
                break;
            }
        }
    }
    
    if (!readyLWPRPC && !readyThrRPC && !readyProcessRPC) {
        if (wasRunning) {
            // the caller expects the process to be running after
            // iRPCs finish, so continue the process here
            proc_->continueProc();
        }
        recursionGuard = false;
        return false;
    }   

    // We have work to do. Pause the process.
    if (!proc_->pause()) {
        cerr << "FAILURE TO PAUSE PROCESS in launchRPCs" << endl;
        recursionGuard = false;
        return false;
    }
    
    // Okay, there is an inferior RPC to do somewhere. Now we just need
    // to launch ze sucker
    bool runProcessWhenDone = false;
    // Run LWP RPCs (if there are any)
    if (readyLWPRPC) {
        dictionary_hash<unsigned, rpcLWP *>::iterator lwp_iter = lwps_.begin();
        while(lwp_iter != lwps_.end()) {
            rpcLWP *cur_rpc_lwp = (*lwp_iter);
            if (cur_rpc_lwp) {            
                irpcLaunchState_t lwpState = cur_rpc_lwp->launchLWPIRPC(wasRunning);
                if (lwpState == irpcBreakpointSet ||
                    lwpState == irpcAgain ||
                    lwpState == irpcStarted) {
                    runProcessWhenDone = true;
                }
            }
            lwp_iter++;
        }
    }
#if defined(sparc_sun_solaris2_4)
    else if (proc_->multithread_capable() && readyProcessRPC) {
        // Loop over all threads until one can run the process RPC
        for (unsigned iter = 0; iter < thrs_.size(); iter++) {
            rpcThr *curThr = thrs_[iter];
            if (curThr == NULL) continue;
            
            irpcLaunchState_t thrState = curThr->launchProcIRPC(wasRunning);
            if (thrState == irpcStarted) {
                proc_->overrideRepresentativeLWP(curThr->get_thr()->get_lwp());
                processingProcessRPC = true;
                break;
            }
        }
    }
#endif
    else if (readyThrRPC) {
        // Loop over all threads and try to run an inferior RPC
        for (unsigned iter = 0; iter < thrs_.size(); iter++) {
           rpcThr *curThr = thrs_[iter];
           if(curThr == NULL)
              continue;

            irpcLaunchState_t thrState = curThr->launchThrIRPC(wasRunning);
            // If an IRPC was launched we've got it in the allRunningRPCs
            // vector (For bookkeeping)
            // And pick out whether the process should be run
            
            if (thrState == irpcBreakpointSet ||
                thrState == irpcAgain ||
                thrState == irpcStarted) {
                runProcessWhenDone = true;
            }
        }
    }
    else
        assert(0);

    // Return value states whether the process should be run or not.
    // If we have an inferior RPC going then always return true (since
    // the RPC needs to complete). If we have a _pending_ RPC then run
    // the process (since it needs to get ready). And if we have an RPC
    // pending with no inserted breakpoint then run the process (but
    // poll for completion)

    if (runProcessWhenDone || 
        allRunningRPCs_.size() > 0) {
        proc_->continueProc();
        recursionGuard = false;
        return true;
    }
    if (wasRunning) {
        proc_->continueProc();
    }
    
    recursionGuard = false;
    return false;
}

void generateMTpreamble(char *, Address &, process *);

Address rpcMgr::createRPCImage(AstNode *action,
                               bool noCost,
                               bool shouldStopForResult,
                               Address &breakAddr,
                               Address &stopForResultAddr,
                               Address &justAfter_stopForResultAddr,
                               Register &resultReg,
                               bool lowmem) {
   // Returns addr of temp tramp, which was allocated in the inferior heap.
   // You must free it yourself when done.
   // Note how this is, in many ways, a greatly simplified version of
   // addInstFunc().
  
   // Temp tramp structure: save; code; restore; trap; illegal
   // the illegal is just to make sure that the trap never returns
   // note that we may not need to save and restore anything, since we've
   // already done a GETREGS and we'll restore with a SETREGS, right?
   // unsigned char insnBuffer[4096];
   unsigned char insnBuffer[8192];
  
   // initializes "regSpace", but only the 1st time called
   initTramps(proc_->multithread_capable()); 
   extern registerSpace *regSpace;
   regSpace->resetSpace();
  
   Address count = 0; // number of bytes required for RPCtempTramp
  
   // The following is implemented in an arch-specific source file...
   // isFunclet added since we might save more registers based on that
   // Temporary: isFunclet disabled
   if (!emitInferiorRPCheader(insnBuffer, count)) {
      // a fancy dialog box is probably called for here...
      cerr << "createRPCtempTramp failed because emitInferiorRPCheader failed."
           << endl;
      return 0;
   }

   if (proc_->multithread_ready()) {
      // We need to put in a branch past the rest of the RPC (to the trailer,
      // actually) if the MT information given is incorrect. That's the
      // skipBRaddr part.
      generateMTpreamble((char*)insnBuffer,count, proc_);
   }

   resultReg = (Register)action->generateCode(proc_, regSpace,
                                              (char*)insnBuffer,
                                              count, noCost, true);

   if (!shouldStopForResult) {
      regSpace->freeRegister(resultReg);
   }
   else
      ; // in this case, we'll call freeRegister() the inferior rpc completes
  
   // Now, the trailer (restore, TRAP, illegal)
   // (the following is implemented in an arch-specific source file...)   
   unsigned breakOffset, stopForResultOffset, justAfter_stopForResultOffset;
   if (!emitInferiorRPCtrailer(insnBuffer, count, breakOffset,
                               shouldStopForResult, stopForResultOffset,
                               justAfter_stopForResultOffset)) {
      // last 4 args except shouldStopForResult are modified by the call
      cerr << "createRPCtempTramp failed because "
           << "emitInferiorRPCtrailer failed." << endl;
      return 0;
   }
   Address tempTrampBase;
   if (lowmem)
   {
      /* lowmemHeap should always have free space, so this will not
         require a recursive inferior RPC. */
      tempTrampBase = proc_->inferiorMalloc(count, lowmemHeap);
   }
   else
   {
      /* May cause another inferior RPC to dynamically allocate a new heap
         in the inferior. */
      tempTrampBase = proc_->inferiorMalloc(count, anyHeap);
   }
   assert(tempTrampBase);

   breakAddr                      = tempTrampBase + breakOffset;
   if (shouldStopForResult) {
      stopForResultAddr           = tempTrampBase + stopForResultOffset;
      justAfter_stopForResultAddr = tempTrampBase + 
                                    justAfter_stopForResultOffset;
   } 
   else {
      stopForResultAddr = justAfter_stopForResultAddr = 0;
   }
  
   if (pd_debug_infrpc)
      cerr << "createRPCtempTramp: temp tramp base=" << (void*)tempTrampBase
           << ", stopForResultAddr=" << (void*)stopForResultAddr
           << ", justAfter_stopForResultAddr="
           << (void*)justAfter_stopForResultAddr
           << ", breakAddr=" << (void*)breakAddr
           << ", count=" << count << " so end addr="
           << (void*)(tempTrampBase + count - 1) << endl;
  
  
   /* Now, write to the tempTramp, in the inferior addr's data space
      (all tramps are allocated in data space) */
   /*
     fprintf(stderr, "IRPC:\n");
     for (unsigned i = 0; i < count/4; i++)
     fprintf(stderr, "0x%x\n", ((int *)insnBuffer)[i]);
     fprintf(stderr, "\n\n\n\n\n");
   */
   if (!proc_->writeDataSpace((void*)tempTrampBase, count, insnBuffer)) {
      // should put up a nice error dialog window
      cerr << "createRPCtempTramp failed because writeDataSpace failed" <<endl;
      return 0;
   }
  
   extern int trampBytes; // stats.h
   trampBytes += count;
  
   return tempTrampBase;
}

bool rpcMgr::cancelRPC(unsigned id) {
    // We can cancel posted or pending RPCs
    for (unsigned i = 0; i < allPostedRPCs_.size(); i++) {
       inferiorRPCtoDo *rpc = allPostedRPCs_[i];
       if (rpc->id == id) {
          if (rpc->thr)
             thrs_[rpc->thr->get_index()]->deleteThrIRPC(id);
          else if (rpc->lwp)
             lwps_[rpc->lwp->get_lwp_id()]->deleteLWPIRPC(id);
          else
             deleteProcessRPC(id);
          removePostedRPC(rpc);
          return true;
       }
    }
    
    // Check pending
    for (unsigned j = 0; j < allPendingRPCs_.size(); j++) {
       inferiorRPCinProgress *inprog = allPendingRPCs_[j];
        if (inprog->rpc->id == id) {
            if (inprog->rpc->thr)
                thrs_[inprog->rpc->thr->get_index()]->deleteThrIRPC(id);
            else if (inprog->rpc->lwp)
                lwps_[inprog->rpc->lwp->get_lwp_id()]->deleteLWPIRPC(id);
            removePendingRPC(inprog);
            return true;
        }
    }
    return false;
}

void rpcMgr::addThread(dyn_thread *thr) {
    rpcThr *newThread = new rpcThr(this, thr);
    int index = newThread->get_thr()->get_index();

    // this code will fill in NULLs in any array entries that haven't yet
    // been assigned a thread
    unsigned new_size = static_cast<unsigned>(index) + 1;
    if(new_size > thrs_.size()) {
       for(unsigned i=thrs_.size(); i < new_size; i++)
          thrs_.push_back(NULL);
    }
    thrs_[index] = newThread;
}

void rpcMgr::deleteThread(dyn_thread *thr) {
   int index = thr->get_index();
   delete thrs_[index];
   thrs_[index] = NULL;
}

void rpcMgr::addLWP(dyn_lwp *lwp) {
    rpcLWP *newLWP = new rpcLWP(this, lwp);
    lwps_[lwp->get_lwp_id()] = newLWP;
}

void rpcMgr::deleteLWP(dyn_lwp *lwp) {
    rpcLWP *oldLWP = NULL;
    lwps_.find(lwp->get_lwp_id(), oldLWP);
    if (oldLWP) delete oldLWP;
    lwps_.undef(lwp->get_lwp_id());
}

/*
 * RPC list manipulation
 */

bool rpcMgr::removePostedRPC(inferiorRPCtoDo *rpc) {
    bool removed = false;
    pdvector<inferiorRPCtoDo *> newPostedRPCs;
    for (unsigned i = 0; i < allPostedRPCs_.size(); i++)
        if (rpc == allPostedRPCs_[i]) {
            removed = true;
        }
        else {
            newPostedRPCs.push_back(allPostedRPCs_[i]);
        }
    
    allPostedRPCs_ = newPostedRPCs;
    return removed;
}

bool rpcMgr::removePendingRPC(inferiorRPCinProgress *rpc) {
    bool removed = false;
    pdvector<inferiorRPCinProgress *> newPendingRPCs;
    for (unsigned i = 0; i < allPendingRPCs_.size(); i++)
        if (rpc == allPendingRPCs_[i]) {
            removed = true;
        }
        else {
            newPendingRPCs.push_back(allPendingRPCs_[i]);
        }
    
    allPendingRPCs_ = newPendingRPCs;
    return removed;
}

bool rpcMgr::removeRunningRPC(inferiorRPCinProgress *rpc) {
    bool removed = false;
    pdvector<inferiorRPCinProgress *> newRunningRPCs;
    for (unsigned i = 0; i < allRunningRPCs_.size(); i++)
        if (rpc == allRunningRPCs_[i]) {
            removed = true;
        }
        else {
            newRunningRPCs.push_back(allRunningRPCs_[i]);
        }
    
    allRunningRPCs_ = newRunningRPCs;
    return removed;
}

bool rpcMgr::addPendingRPC(inferiorRPCinProgress *pending) {
    allPendingRPCs_.push_back(pending);
    return removePostedRPC(pending->rpc);
}

bool rpcMgr::addRunningRPC(inferiorRPCinProgress *running) {
    allRunningRPCs_.push_back(running);
    return removePendingRPC(running);
}

bool rpcMgr::deleteProcessRPC(unsigned id) {
    bool removed = false;
    pdvector<inferiorRPCtoDo *> newRPCs;
    for (unsigned i = 0; i < postedProcessRPCs_.size(); i++) {
        if (postedProcessRPCs_[i]->id == id)
            removed = true;
        else
            newRPCs.push_back(postedProcessRPCs_[i]);
    }
    
    return removed;
    
}

irpcState_t rpcMgr::getRPCState(unsigned id) {
    // Check to see if it's posted
    for (unsigned i = 0; i < allPostedRPCs_.size(); i++)
        if (allPostedRPCs_[i]->id == id)
            return irpcNotRunning;
    
    // Check pending
    for (unsigned j = 0; j < allPendingRPCs_.size(); j++) 
        if (allPendingRPCs_[j]->rpc->id == id)
            return irpcWaitingForSignal;
    
    // Check running
    for (unsigned k = 0; k < allRunningRPCs_.size(); k++)
        if (allRunningRPCs_[k]->rpc->id == id)
            return irpcRunning;
    
    return irpcNotValid;
}

    

