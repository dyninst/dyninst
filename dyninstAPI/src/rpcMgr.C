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
#include "dyninstAPI/src/instP.h" // initTramps
#include "dyninstAPI/src/baseTramp.h" // irpc code
#include "dyninstAPI/src/process.h"
#include "dyninstAPI/src/process.h"
#include "dyninstAPI/src/stats.h"
#include "dyninstAPI/src/showerror.h"
#include "dyninstAPI/src/ast.h"

#if defined(arch_x86_64)
#include "dyninstAPI/src/emit-x86.h"
#endif

rpcMgr::rpcMgr(process *proc) :
    processingProcessRPC(false),
    proc_(proc),
    lwps_(rpcLwpHash),
    recursionGuard(false) {
    // We use a base tramp skeleton to generate iRPCs.
    irpcTramp = new baseTramp();
    irpcTramp->rpcMgr_ = this;
    irpcTramp->setRecursive(true);
}

// Fork constructor. For each thread/LWP in the parent, check to 
// see if it still exists, and if so copy over all state. Also 
// copy all current RPC state. Oy.
rpcMgr::rpcMgr(rpcMgr *pRM, process *child) :
    processingProcessRPC(pRM->processingProcessRPC),
    proc_(child),
    lwps_(rpcLwpHash),
    recursionGuard(pRM->recursionGuard) {
    // We use a base tramp skeleton to generate iRPCs.
    irpcTramp = new baseTramp();
    irpcTramp->rpcMgr_ = this;
    irpcTramp->setRecursive(true);

    // Make all necessary thread and LWP managelets.
    
    for (unsigned i = 0; i < pRM->thrs_.size(); i++) {
        unsigned tid = pRM->thrs_[i]->thr_->get_tid();
        dyn_thread *cthr = child->getThread(tid);
        if (!cthr) continue; // Whee, not there any more. 
        rpcThr *newT = new rpcThr(pRM->thrs_[i],
                                  this,
                                  cthr);
        thrs_.push_back(newT);
    }

    // Check LWPS
    dictionary_hash_iter<unsigned, rpcLWP *> lwp_iter = pRM->lwps_.begin();
    for (; lwp_iter; lwp_iter++) {
        unsigned lwp_id = lwp_iter.currkey();

        // This doesn't handle the case if the LWP ID changed (linux PID)
        // but I'm _really_ not worried about this.

        dyn_lwp *clwp = child->lookupLWP(lwp_id);
        if (!clwp) {
            continue;
        }
        rpcLWP *newL = new rpcLWP(lwp_iter.currval(),
                                  this,
                                  clwp);
        lwps_[lwp_id] = newL;
    }

    // Okay, we have those... we need to build:
    // allPostedRPCs_;
    // postedProcessRPCs_;
    // allRunningRPCs_;
    // allPendingRPCs_;

    // allPosted is built from thr+lwp+proc, and thr/lwp already built internally;
    // but we create it so that we get ordering right. 
    // postedProcess we create
    // allRunning already done;
    // allPending already done.
        
    for (unsigned ii = 0; ii < pRM->postedProcessRPCs_.size(); ii++) {
        inferiorRPCtoDo *newRPC = new inferiorRPCtoDo;
        inferiorRPCtoDo *oldRPC = pRM->postedProcessRPCs_[ii];
        
        newRPC->action = assignAst(oldRPC->action);
        newRPC->noCost = oldRPC->noCost;
        newRPC->callbackFunc = oldRPC->callbackFunc;
        newRPC->userData = oldRPC->userData;
        newRPC->lowmem = oldRPC->lowmem;
        newRPC->id = oldRPC->id;
        assert(!oldRPC->thr); // It's a process RPC
        assert(!oldRPC->lwp);
        postedProcessRPCs_.push_back(newRPC);
    }

    // This is horridly inefficient
    for (unsigned iii = 0; iii < pRM->allPostedRPCs_.size(); iii++) {
        inferiorRPCtoDo *oldRPC = pRM->postedProcessRPCs_[iii];
        bool found = false;
        if (oldRPC->thr) {
            unsigned tid = oldRPC->thr->get_tid();
            for (unsigned j = 0; j < thrs_.size(); j++) {
                if (thrs_[j]->thr_->get_tid() == tid) {
                    rpcThr *thr = thrs_[j];
                    // Now find the matching (ID) RPC
                    for (unsigned k = 0; k < thr->postedRPCs_.size(); k++)
                        if (thr->postedRPCs_[k]->id == oldRPC->id) {
                            found = true;
                            allPostedRPCs_.push_back(thr->postedRPCs_[k]);
                            break;
                        }

                }
                if (found) break;
            }
        }
        else if (oldRPC->lwp) {
            unsigned lid = oldRPC->lwp->get_lwp_id();
            rpcLWP *lwp = lwps_[lid];
            assert(lwp);
            // Now find the matching (ID) RPC
            for (unsigned k = 0; k < lwp->postedRPCs_.size(); k++) {
                if (lwp->postedRPCs_[k]->id == oldRPC->id) {
                    found = true;
                    allPostedRPCs_.push_back(lwp->postedRPCs_[k]);
                    break;
                }
            }
        }
        else {
            for (unsigned i = 0; i < postedProcessRPCs_.size(); i++) {
                if (postedProcessRPCs_[i]->id == oldRPC->id) {
                    found = true;
                    allPostedRPCs_.push_back(postedProcessRPCs_[i]);
                    break;
                }
            }
        }
        assert(found);
    }
}
    
rpcMgr::~rpcMgr() {
    delete irpcTramp;
}

// post RPC toDo for process
unsigned rpcMgr::postRPCtoDo(AstNode *action, bool noCost,
                             inferiorRPCcallbackFunc callbackFunc,
                             void *userData, bool lowmem,
                             dyn_thread *thr, dyn_lwp *lwp)
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

    inferiorrpc_printf("Posting new RPC: seq %d", theStruct->id);
    if (thr)
      inferiorrpc_printf(", thread %d", thr->get_tid());
    if (lwp)
      inferiorrpc_printf(", lwp %d", lwp->get_lwp_id());
    inferiorrpc_printf("\n");

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

#if defined(RPC_SHOWSTATE)  // def is ifdefed out since not normally used
void rpcMgr::showState() const {
   cerr << "   there are " << allRunningRPCs_.size() << " running rpcs\n";
   for (unsigned i = 0; i < allRunningRPCs_.size(); i++) {
      Frame activeFrame;
      inferiorRPCinProgress *currRPC = allRunningRPCs_[i];
      
      rpcThr *rpcThr = currRPC->rpcthr;
      rpcLWP *rpcLwp = currRPC->rpclwp;
      struct inferiorRPCtoDo *rpc = currRPC->rpc;

       if(rpcThr) {
          cerr << "     [" << i << "] id: " << rpc->id << ", lwp: "
               << rpcThr->get_thr()->get_lwp()->get_lwp_id()
               << ", state: ";
          switch(currRPC->state) {
            case irpcNotValid:  cerr << "irpcNotValid";  break;
            case irpcNotRunning:  cerr << "irpcNotRunning";  break;
            case irpcRunning:  cerr << "irpcRunning";  break;
            case irpcWaitingForSignal:  cerr << "irpcWaitingForSignal"; break;
            case irpcNotReadyForIRPC:  cerr << "irpcNotReadyForIRPC";  break;
          }
          cerr << endl;
       } else {
          assert(rpcLwp != NULL);
          cerr << "     [" << i << "] id: " << rpc->id << ", an lwp rpc\n";
       }
   }
}
#endif

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

bool rpcMgr::handleSignalIfDueToIRPC(dyn_lwp *lwp_of_trap) {
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
   
   dyn_lwp *lwp_to_cont = NULL;

   pdvector<inferiorRPCinProgress *>::iterator iter = allRunningRPCs_.end();
   while(iter != allRunningRPCs_.begin()) {
       inferiorRPCinProgress *currRPC = *(--iter);
       Frame activeFrame;

       rpcThr *rpcThr = currRPC->rpcthr;
       rpcLWP *rpcLwp = currRPC->rpclwp;

       if(rpcThr) {
          dyn_thread *cur_dthr = rpcThr->get_thr();

          // skip comparing against any outstanding rpcs from threads/lwps
          // that aren't stopped; couldn't be the one if not stopped
	  // Only for independently-controlled LWPs. Otherwise they're in
	  // the same state as the process.	 
	  if(process::IndependentLwpControl()) {
	    if(cur_dthr->get_lwp()->status() != stopped) {
	      continue;
	    }
	  }

#if !defined(rs6000_ibm_aix4_1)  || defined(AIX_PROC)  // non AIX-PTRACE
          if(cur_dthr->get_lwp()->get_lwp_id() != lwp_of_trap->get_lwp_id()) {
             continue;
          }
#endif
          activeFrame = cur_dthr->getActiveFrame();
       } else {
          assert(rpcLwp != NULL);

          dyn_lwp *cur_dlwp = rpcLwp->get_lwp();

	  // See thread comment
	  if(process::IndependentLwpControl()) {
	    if(cur_dlwp->status() != stopped) {
	      continue;
	    }
	  }
#if !defined(rs6000_ibm_aix4_1)  || defined(AIX_PROC)  // non AIX-PTRACE
          if(cur_dlwp->get_lwp_id() != lwp_of_trap->get_lwp_id()) {
             continue;
          }
#endif
          activeFrame = rpcLwp->get_lwp()->getActiveFrame();
       }
       if (activeFrame.getPC() == currRPC->rpcResultAddr) {

          if(rpcThr)
             rpcThr->getReturnValueIRPC();
          else {
             rpcLwp->getReturnValueIRPC();
	  }
          handledTrap = true;
          runProcess = true;
       }
       else if (activeFrame.getPC() == currRPC->rpcCompletionAddr) {
          if(rpcThr)
             runProcess = rpcThr->handleCompletedIRPC();
          else
	    {
             runProcess = rpcLwp->handleCompletedIRPC();
	    }
          handledTrap = true;
       }

       if(process::IndependentLwpControl()) {
          if(rpcThr) {
             dyn_thread *dthr = rpcThr->get_thr();
             lwp_to_cont = dthr->get_lwp();
          }  else {
             lwp_to_cont = rpcLwp->get_lwp();
          }
       }

       if (handledTrap) break;
   }
   if (handledTrap) {
     inferiorrpc_printf("Completed RPC: pending %d, requestedRun %d\n",
			allRunningRPCs_.size(), runProcess);
      if (runProcess || allRunningRPCs_.size() > 0) {
         if(process::IndependentLwpControl()) {
            lwp_to_cont->continueLWP();
         } else {
            proc_->continueProc();
         }
      }
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
    inferiorrpc_printf("Call to launchRPCs, recursionGuard %d\n", recursionGuard);
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
    bool processingThrRPC = false;
    // We check LWP RPCs first. If there are any they are run first -- even
    // if there is a thread RPC currently running. Only use LWP RPCs for very low
    // level operations. 
    
    // We have a central list of all posted or pending RPCs... if those are empty
    // then don't bother doing work
    inferiorrpc_printf("%d posted RPCss...\n", allPostedRPCs_.size());
    if (allPostedRPCs_.size() == 0) {
      // Here's an interesting design question. "wasRunning" means "what do I do
      // after the RPCs are done". Now, if there weren't any RPCs, do we 
      // run the process? 
      if (wasRunning && proc_->isStopped()) {
	proc_->continueProc();
      }
      recursionGuard = false;
      return false;
    }

    inferiorrpc_cerr << "RPCs posted: " << allPostedRPCs_.size() << endl;
    
    dictionary_hash<unsigned, rpcLWP *>::iterator rpc_iter = lwps_.begin();
    while(rpc_iter != lwps_.end()) {
      rpcLWP *cur_rpc_lwp = (*rpc_iter);
      if (cur_rpc_lwp) {
	if(cur_rpc_lwp->isReadyForIRPC()) {
	  inferiorrpc_printf("LWP %d ready for RPC...\n", cur_rpc_lwp->get_lwp()->get_lwp_id());
	  readyLWPRPC = true;
	  break;
	}
	if (cur_rpc_lwp->isProcessingIRPC()) {
	  inferiorrpc_printf("LWP %d currently processing RPC...\n", cur_rpc_lwp->get_lwp()->get_lwp_id());
	  processingLWPRPC = true;
	}
      }
      rpc_iter++;
    }

#if defined(sparc_sun_solaris2_4)
    if(proc_->multithread_capable()) {
       if (!readyLWPRPC && !processingLWPRPC) {
	 if (postedProcessRPCs_.size()) {
	   inferiorrpc_printf("SOLARIS: waiting process RPC\n");
	   readyProcessRPC = true;
	 }
       }
    }
#endif
    // Only run thread RPCs if there are no LWP RPCs either waiting or in flight.

    if (!readyLWPRPC && !processingLWPRPC && !readyProcessRPC && !processingProcessRPC) {
        for (unsigned i = 0; i < thrs_.size(); i++) {
           rpcThr *curThr = thrs_[i];
           if(curThr == NULL) {
	     fprintf(stderr, "Odd case: RPC thread object is NULL for slot %d\n",
		     i);
	     continue;
	   }
	   if (curThr->isReadyForIRPC()) {
	     inferiorrpc_printf("Thread %d ready for RPC...\n", curThr->get_thr()->get_tid());
	     readyThrRPC = true;
	     break;
	   }
	   if (curThr->isRunningIRPC()) {
	     inferiorrpc_printf("Thread %d currently processing RPC...\n", curThr->get_thr()->get_tid());
	     processingThrRPC = true;
	   }
        }
    }
    inferiorrpc_printf("RPC status dump: readyLWP %d, readyThr %d, readyProcess %d;\n",
		       readyLWPRPC, readyThrRPC, readyProcessRPC);
    inferiorrpc_printf("RPC status dump: wasRunning %d, processingLWP %d, processingThr %d\n",
		       wasRunning, processingLWPRPC, processingThrRPC);
    if (!readyLWPRPC && !readyThrRPC && !readyProcessRPC) {
        if (wasRunning || processingLWPRPC || processingThrRPC) {
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
		inferiorrpc_printf("Result of posting RPC on LWP %d: %d\n",
				   cur_rpc_lwp->get_lwp()->get_lwp_id(),
				   lwpState);
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
	    inferiorrpc_printf("Result of posting process-wide RPC on thread %d: %d\n",
			       curThr->get_thr()->get_tid(),
			       thrState);
            if (thrState == irpcStarted) {
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

	    inferiorrpc_printf("Result of posting RPC on thread %d: %d\n",
			       curThr->get_thr()->get_tid(),
			       thrState);
            
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

    // Rather than worrying about saving mutator-side, we run this as a
    // conservative base tramp. They're really the same thing, 
    // with some extra bits at the end.

   // Temp tramp structure: save; code; restore; trap; illegal
   // the illegal is just to make sure that the trap never returns
   // note that we may not need to save and restore anything, since we've
   // already done a GETREGS and we'll restore with a SETREGS, right?
   // unsigned char insnBuffer[4096];
    codeGen irpcBuf(MAX_IRPC_SIZE);
    
    // initializes "regSpace", but only the 1st time called
    initTramps(proc_->multithread_capable()); 

#if defined(arch_x86_64)
    if (proc()->getAddressWidth() == 8)
	emit64();
    else
	emit32();
#endif

    extern registerSpace *regSpace;
    regSpace->resetSpace();
    
    // Saves registers (first half of the base tramp) and whatever other
    // irpc-specific magic is necessary
    if (!emitInferiorRPCheader(irpcBuf)) {
        // a fancy dialog box is probably called for here...
        cerr << "createRPCtempTramp failed because emitInferiorRPCheader failed."
             << endl;
        return 0;
    }

    resultReg = (Register)action->generateCode(proc_, regSpace,
                                               irpcBuf,
                                               noCost, true);
    
    if (!shouldStopForResult) {
        regSpace->freeRegister(resultReg);
    }
    else
        ; // in this case, we'll call freeRegister() the inferior rpc completes
    
    // Now, the trailer (restore, TRAP, illegal)
    // (the following is implemented in an arch-specific source file...)   
    // breakOffset: where the irpc ends
    // stopForResultOffset: we expect a trap here if we're getting a result back
    // justAfter_stopForResultOffset: where to continue the process at (next insn)

    unsigned breakOffset, stopForResultOffset, justAfter_stopForResultOffset;
    if (!emitInferiorRPCtrailer(irpcBuf, breakOffset,
                                shouldStopForResult, stopForResultOffset,
                                justAfter_stopForResultOffset)) {
        // last 4 args except shouldStopForResult are modified by the call
        cerr << "createRPCtempTramp failed because "
             << "emitInferiorRPCtrailer failed." << endl;
        return 0;
    }
    Address tempTrampBase;
    inferiorrpc_printf("Allocating RPC image... lowmem %d, count %d\n",
                       lowmem, irpcBuf.used());
    if (lowmem)
        {
            /* lowmemHeap should always have free space, so this will not
               require a recursive inferior RPC. */
            tempTrampBase = proc_->inferiorMalloc(irpcBuf.used(), lowmemHeap);
        }
    else
        {
            /* May cause another inferior RPC to dynamically allocate a new heap
               in the inferior. */
            // This is currently broken; noticed when I wasn't adding
            // the heaps correctly. Problem is, I'm not sure how to fix
            // it up, so leaving for now -- bernat, 12MAY05
            // The recursive allocation, that is. We don't like starting another
            // RPC at this point.
            
            tempTrampBase = proc_->inferiorMalloc(irpcBuf.used(), anyHeap);
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
    
    inferiorrpc_cerr << "createRPCtempTramp: temp tramp base=" << (void*)tempTrampBase
                     << ", stopForResultAddr=" << (void*)stopForResultAddr
                     << ", justAfter_stopForResultAddr="
                     << (void*)justAfter_stopForResultAddr
                     << ", breakAddr=" << (void*)breakAddr
                     << ", count=" << irpcBuf.used() << " so end addr="
                     << (void*)(tempTrampBase + irpcBuf.used()) << endl;
    
    
    /* Now, write to the tempTramp, in the inferior addr's data space
       (all tramps are allocated in data space) */
    /*
      bperr( "IRPC:\n");
      for (unsigned i = 0; i < count/4; i++)
      bperr( "0x%x\n", ((int *)insnBuffer)[i]);
      bperr("\n\n\n\n\n");
    */
    
    if (!proc_->writeDataSpace((void*)tempTrampBase, irpcBuf.used(), irpcBuf.start_ptr())) {
        // should put up a nice error dialog window
        cerr << "createRPCtempTramp failed because writeDataSpace failed" <<endl;
        return 0;
    }
        
    return tempTrampBase;
}

/* ***************************************************** */


#if !defined(arch_ia64)

// IA64 has to figure out what to save before it can do things correctly. This is very
// nasty (see inst-ia64.C), and so we're leaving it alone. Everyone else
// is relatively normal. If the saves are ever wrapped into a function we could
// probably get rid of the ifdef

bool rpcMgr::emitInferiorRPCheader(codeGen &gen) 
{
    assert(irpcTramp);
    irpcTramp->generateBT();
    gen.copy(irpcTramp->preTrampCode_);
    return true;
}

bool rpcMgr::emitInferiorRPCtrailer(codeGen &gen,
                                    unsigned &breakOffset,
                                    bool shouldStopForResult,
                                    unsigned &stopForResultOffset,
                                    unsigned &justAfter_stopForResultOffset) {
    if (shouldStopForResult) {
        // Trappity!
        stopForResultOffset = gen.used();
        instruction::generateTrap(gen);
        justAfter_stopForResultOffset = gen.used();
    }
    assert(irpcTramp);
    irpcTramp->generateBT();
    gen.copy(irpcTramp->postTrampCode_);
    // We can't do a SIGTRAP since SIGTRAP is reserved in x86.
    // So we do a SIGILL instead.
    breakOffset = gen.used();
    instruction::generateTrap(gen);
    
    instruction::generateIllegal(gen);

#if defined(arch_x86) || defined(arch_x86_64)
     // X86 traps at the next insn, not the trap. So shift the
     // offsets accordingly
     if (shouldStopForResult) {
         stopForResultOffset += 1;
     }
     breakOffset += 1;
#endif
         

    return true;
}

#endif


bool rpcMgr::cancelRPC(unsigned id) {
  inferiorrpc_printf("Cancelling RPC %d...\n", id);
    // We can cancel posted or pending RPCs
    for (unsigned i = 0; i < allPostedRPCs_.size(); i++) {
       inferiorRPCtoDo *rpc = allPostedRPCs_[i];
       inferiorrpc_printf("Checking RPC %d against %d\n", rpc->id, id);
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
       inferiorrpc_printf("Checking pending RPC %d against %d\n", inprog->rpc->id, id);

        if (inprog->rpc->id == id) {
            if (inprog->rpc->thr)
                thrs_[inprog->rpc->thr->get_index()]->deleteThrIRPC(id);
            else if (inprog->rpc->lwp)
                lwps_[inprog->rpc->lwp->get_lwp_id()]->deleteLWPIRPC(id);
            removePendingRPC(inprog);
            return true;
        }
    }

    // And running...
    for (unsigned l = 0; l < allRunningRPCs_.size(); l++) {
       inferiorRPCinProgress *running = allRunningRPCs_[l];
       inferiorrpc_printf("Checking running RPC %d against %d\n", running->rpc->id, id);

        if (running->rpc->id == id) {
	  assert(0);
	  return false;
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

    

