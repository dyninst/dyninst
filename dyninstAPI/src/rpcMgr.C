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
#include "dyninstAPI/src/instP.h" // initTramps
#include "dyninstAPI/src/baseTramp.h" // irpc code
#include "dyninstAPI/src/process.h"
#include "common/h/stats.h"
#include "dyninstAPI/src/debug.h"
#include "dyninstAPI/src/ast.h"
#include "dyninstAPI/src/rpcMgr.h"
#include "dyninstAPI/src/signalgenerator.h"
#include "dyninstAPI/src/registerSpace.h"

#if defined(arch_x86_64)
#include "dyninstAPI/src/emit-x86.h"
#endif
#ifndef CASE_RETURN_STR
#define CASE_RETURN_STR(x) case x: return #x
#endif
const char *irpcState2Str(irpcState_t s)
{
	switch (s) {
		CASE_RETURN_STR(irpcNotValid);
		CASE_RETURN_STR(irpcNotRunning);
		CASE_RETURN_STR(irpcRunning);
		CASE_RETURN_STR(irpcWaitingForSignal);
		CASE_RETURN_STR(irpcNotReadyForIRPC);
	};
	return "bad_rpc_state";
}


rpcMgr::rpcMgr(process *proc) :
    processingProcessRPC(false),
    proc_(proc),
    lwps_(rpcLwpHash),
    recursionGuard(false)
{
    // We use a base tramp skeleton to generate iRPCs.
    irpcTramp = new baseTramp(NULL, callUnset);
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
    recursionGuard(pRM->recursionGuard)
{
    // We use a base tramp skeleton to generate iRPCs.
    irpcTramp = new baseTramp(NULL, callUnset);
    irpcTramp->rpcMgr_ = this;
    irpcTramp->setRecursive(true);

    // Make all necessary thread and LWP managelets.

    for (unsigned i = 0; i < pRM->thrs_.size(); i++) {
      if (pRM->thrs_[i]) {
        dynthread_t tid = pRM->thrs_[i]->thr_->get_tid();
        dyn_thread *cthr = child->getThread(tid);
        if (!cthr) continue; // Whee, not there any more. 
        rpcThr *newT = new rpcThr(pRM->thrs_[i],
                                  this,
                                  cthr);
        thrs_.push_back(newT);
      }
      else {
          thrs_.push_back(NULL); // Can happen if indices were skipped
      }
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
        
        newRPC->action = oldRPC->action;
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
            dynthread_t tid = oldRPC->thr->get_tid();
            for (unsigned j = 0; j < thrs_.size(); j++) {
	      if (thrs_[j] == NULL) continue;
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
unsigned rpcMgr::postRPCtoDo(AstNodePtr action, bool noCost,
                             inferiorRPCcallbackFunc callbackFunc,
                             void *userData, 
                             bool runWhenFinished,
                             bool lowmem,
                             dyn_thread *thr, dyn_lwp *lwp)
{
    static int sequence_num = 0;
    process *proc = NULL;
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
    theStruct->runProcessWhenDone = runWhenFinished;
    if (thr)
       proc = thr->get_proc();
    else if (lwp)
       proc = lwp->proc();
    theStruct->saveFPState = proc ? proc->shouldSaveFPState() : true;
 
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

    inferiorrpc_printf("%s[%d]: Posting new RPC: seq %d, thr %u, lwp %d\n", FILE__, __LINE__, theStruct->id,
                       thr ? thr->get_tid() : 0,
                       lwp ? (int) lwp->get_lwp_id() : -1);

    return theStruct->id;
}

inferiorRPCtoDo *rpcMgr::getProcessRPC() {
    if (postedProcessRPCs_.size() == 0) return NULL;

    inferiorRPCtoDo *rpc = postedProcessRPCs_[0];

    VECTOR_ERASE(postedProcessRPCs_,0,0);
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

bool rpcMgr::existsActiveIRPC() const {
    for (unsigned i = 0; i < thrs_.size(); i++) {
       if (thrs_[i] == NULL) continue;
       if (thrs_[i]->isRunningIRPC()) {
          inferiorrpc_printf("%s[%d]: active IRPC on thread %d (slot %d), ret true\n",
                             FILE__, __LINE__, thrs_[i]->thr_->get_tid(), i);
          return true;
       }
    }
    dictionary_hash<unsigned, rpcLWP *>::iterator rpc_iter = lwps_.begin();
    while(rpc_iter != lwps_.end()) {
        if ((*rpc_iter)->isRunningIRPC()) {
            inferiorrpc_printf("%s[%d]: active IRPC on lwp %d, ret true\n",
                               FILE__, __LINE__, (*rpc_iter)->lwp_->get_lwp_id());
            return true;
        }
        rpc_iter++;
    }
    inferiorrpc_printf("%s[%d]: No active IRPC\n", FILE__, __LINE__);
    return false;
}

bool rpcMgr::existsPendingIRPC() const {
    for (unsigned i = 0; i < thrs_.size(); i++) {
      if (thrs_[i] == NULL) continue;
        if (thrs_[i]->isWaitingForBreakpoint()) {
            inferiorrpc_printf("%s[%d]: thread %d (slot %d) waiting for breakpoint, ret true\n",
                               FILE__, __LINE__, thrs_[i]->thr_->get_tid(), i);
            return true;
        }
    }
    dictionary_hash<unsigned, rpcLWP *>::iterator rpc_iter = lwps_.begin();
    while(rpc_iter != lwps_.end()) {
        if ((*rpc_iter)->isWaitingForBreakpoint()) {
            inferiorrpc_printf("%s[%d]: pending IRPC on lwp %d, ret true\n",
                               FILE__, __LINE__, (*rpc_iter)->lwp_->get_lwp_id());
            return true;
        }
        rpc_iter++;
    }
    inferiorrpc_printf("%s[%d]: No pending IRPC\n", FILE__, __LINE__);
    return false;
}

bool rpcMgr::existsWaitingIRPC() const {
    for (unsigned i = 0; i < thrs_.size(); i++) {
      if (thrs_[i] == NULL) continue;
        if (thrs_[i]->isReadyForIRPC()) {
            inferiorrpc_printf("%s[%d]: thread %d (slot %d) has ready RPC, ret true\n",
                               FILE__, __LINE__, thrs_[i]->thr_->get_tid(), i);
            return true;
        }
    }
    dictionary_hash<unsigned, rpcLWP *>::iterator rpc_iter = lwps_.begin();
    while(rpc_iter != lwps_.end()) {
        if ((*rpc_iter)->isReadyForIRPC()) {
            inferiorrpc_printf("%s[%d]: ready IRPC on lwp %d, ret true\n",
                               FILE__, __LINE__, (*rpc_iter)->lwp_->get_lwp_id());
            return true;
        }
        rpc_iter++;
    }
    inferiorrpc_printf("%s[%d]: No ready IRPC\n", FILE__, __LINE__);
    return false;
}

inferiorRPCinProgress *rpcMgr::findRunningRPCWithResultAddress(Address where)
{
  inferiorRPCinProgress *ret = NULL;
  inferiorrpc_printf("%s[%d]: %d running RPCs\n", FILE__, __LINE__, allRunningRPCs_.size());
  for (int i = allRunningRPCs_.size() -1; i >= 0; --i) {
      inferiorrpc_printf("%s[%d]: comparing curr addr 0x%lx to RPC result addr 0x%lx\n",
                         FILE__, __LINE__, where, allRunningRPCs_[i]->rpcResultAddr); 
      if (allRunningRPCs_[i]->rpcResultAddr == where) {
          ret = allRunningRPCs_[i];
          break;
      }
  }
  return ret;
}

inferiorRPCinProgress *rpcMgr::findRunningRPCWithCompletionAddress(Address where)
{
  inferiorRPCinProgress *ret = NULL;
  inferiorrpc_printf("%s[%d]: %d running RPCs\n", FILE__, __LINE__, allRunningRPCs_.size());
  for (int i = allRunningRPCs_.size() -1; i >= 0; --i) {
      inferiorrpc_printf("%s[%d]: comparing curr addr 0x%lx to RPC completion addr 0x%lx\n",
                         FILE__, __LINE__, where, allRunningRPCs_[i]->rpcCompletionAddr); 
      if (allRunningRPCs_[i]->rpcCompletionAddr == where) {
          ret = allRunningRPCs_[i];
          break;
      }
  }
  return ret;
}

bool rpcMgr::decodeEventIfDueToIRPC(EventRecord &ev)
{
   dyn_lwp *lwp_of_trap  = ev.lwp;

   inferiorrpc_printf("%s[%d]:  decodeEventIfDueToIRPC:  allRunningRPCs_.size = %d\n", FILE__, __LINE__, allRunningRPCs_.size());

   int curr_rpc_index = allRunningRPCs_.size();
   pdvector<inferiorRPCinProgress *>::iterator iter = allRunningRPCs_.end();
   while(iter != allRunningRPCs_.begin()) {
       inferiorRPCinProgress *currRPC = *(--iter);
       curr_rpc_index--;

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

          if(cur_dthr->get_lwp()->get_lwp_id() != lwp_of_trap->get_lwp_id()) {
              signal_printf("%s[%d]: trap LWP id %d mismatch against internal lwp ID %d\n",
                            FILE__, __LINE__, 
                            lwp_of_trap->get_lwp_id(), 
                            cur_dthr->get_lwp()->get_lwp_id());
             continue;
          }

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
          if(cur_dlwp->get_lwp_id() != lwp_of_trap->get_lwp_id()) {
             continue;
          }
          activeFrame = rpcLwp->get_lwp()->getActiveFrame();
       }

       signal_printf("%s[%d]: reported active frame PC is 0x%lx; thread %d, lwp %d\n",
                     FILE__, __LINE__, activeFrame.getPC(), 
                     activeFrame.getThread() ? (int) activeFrame.getThread()->get_tid() : -1,
                     activeFrame.getLWP() ? (int) activeFrame.getLWP()->get_lwp_id() : -1);
                    
       
       if (activeFrame.getPC() == currRPC->rpcResultAddr) {
           signal_printf("%s[%d]: PC at 0x%lx for lwp %u matches RPC result addr 0x%lx on RPC %p\n",
                         FILE__, __LINE__, activeFrame.getPC(), activeFrame.getLWP()->get_lwp_id(),
                         currRPC->rpcResultAddr, currRPC);
          ev.type = evtRPCSignal;
          ev.status = statusRPCAtReturn;
          ev.what = (eventWhat_t) curr_rpc_index;
          ev.address = activeFrame.getPC();
          return true;
       }
       else if (activeFrame.getPC() == currRPC->rpcCompletionAddr) {
           signal_printf("%s[%d]: PC at 0x%lx for lwp %u matches RPC completion addr 0x%lx on RPC %p\n",
                         FILE__, __LINE__, activeFrame.getPC(), activeFrame.getLWP()->get_lwp_id(),
                         currRPC->rpcCompletionAddr, currRPC);
          ev.type = evtRPCSignal;
          ev.status = statusRPCDone;
          ev.what = (eventWhat_t) curr_rpc_index;
          ev.address = activeFrame.getPC();
          return true;
       }
   }
          
   return false;
}

bool rpcMgr::handleRPCEvent(EventRecord &ev, bool &continueHint) 
{
  if (ev.type != evtRPCSignal) return false;

  inferiorRPCinProgress *currRPC = NULL;
  rpcThr *rpcThr = NULL;
  rpcLWP *rpcLwp = NULL;

  inferiorrpc_printf("%s[%d]: handleRPCEvent, status %d, addr 0x%lx\n", 
                     FILE__, __LINE__, ev.status, ev.address);

  if (ev.status == statusRPCAtReturn) {
    currRPC = findRunningRPCWithResultAddress(ev.address);
    assert(currRPC);
    assert(ev.address == currRPC->rpcResultAddr); 
    rpcThr = currRPC->rpcthr;
    rpcLwp = currRPC->rpclwp;

    if (rpcThr) 
       rpcThr->getReturnValueIRPC();
    else 
       rpcLwp->getReturnValueIRPC();
    
    continueHint = true;
  }
  else if (ev.status == statusRPCDone) {
    currRPC = findRunningRPCWithCompletionAddress(ev.address);

    assert(currRPC);
    assert(ev.address == currRPC->rpcCompletionAddr); 

    // currRPC goes away in handleCompleted... so slurp the result here
    if (currRPC->rpc->runProcessWhenDone) {
	  continueHint = true;
    }
    
    rpcThr = currRPC->rpcthr;
    rpcLwp = currRPC->rpclwp;
    if(rpcThr) {
        if (rpcThr->handleCompletedIRPC()) {
            continueHint = true;
        }
    }
    else if (rpcLwp) {
        if (rpcLwp->handleCompletedIRPC()) {
            continueHint = true;
        }
    }
    else {
        assert(0);
    }
  }
  else 
    assert(0);

  // Do we want this to be pending? What if someone says
  // "run iRPC" and waits for it to finish, on a thread in a
  // syscall.... we need to re-investigate aborting syscalls.

  if (process::IndependentLwpControl()) {
    // Linux can be harmlessly activated; this is _bad_ on 
    // Solaris/AIX (as we'll immediately get the latest event again)

    if (existsActiveIRPC()) {
      // Be sure that we keep consuming events on other threads,
      // even if we're paused in this one...
       signal_printf("%s[%d]: Active RPC, signaling active process\n",
                     FILE__, __LINE__);
      ev.proc->sh->signalActiveProcess();
    }
    else {
       signal_printf("%s[%d]: No active RPC, belaying active process\n",
                     FILE__, __LINE__);
      ev.proc->sh->belayActiveProcess();
    }
  }
  else 
  {
      // We stopped everything, and there's an iRPC that needs
      // running...
      // Should this be moved to whoever's waiting on the iRPC?
      // Better, go to independent control on more platforms.
    if (existsActiveIRPC()) {
       continueHint = true;
    }
    
  }
  return true;
}
  
// Run da suckers
// Take all RPCs posted and run them (if possible)
// Return true if any RPCs were launched (and the process is running),
//   false if none were (and the process hasn't changed state)
// wasRunning: desired state of the process (as opposed to current
//  state).
// Note: if there are no RPCs running but wasRunning is true, launchRPCs
// will continue the process!

bool rpcMgr::launchRPCs(bool &needsToRun, 
                        bool wasRunning) {
    // First, idiot check. If there aren't any RPCs to run, then
    // don't do anything. Reason: launchRPCs is called several times
    // a second in the daemon main loop
    //inferiorrpc_printf("Call to launchRPCs, recursionGuard %d\n", recursionGuard);
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
    //inferiorrpc_printf("%d posted RPCss...\n", allPostedRPCs_.size());
    if (allPostedRPCs_.size() == 0) {
      // Here's an interesting design question. "wasRunning" means "what do I do
      // after the RPCs are done". Now, if there weren't any RPCs, do we 
      // run the process? 
      if (wasRunning && proc_->isStopped()) {
          needsToRun = true;
      }
      recursionGuard = false;
      inferiorrpc_printf("%s[%d]: no posted RPCs, returning immediately\n", FILE__, __LINE__);
      return true;
    }

      inferiorrpc_printf("%s[%d]: launchRPCs checking for RPCs\n", FILE__, __LINE__);

    dictionary_hash<unsigned, rpcLWP *>::iterator rpc_iter = lwps_.begin();
    while(rpc_iter != lwps_.end()) {
       rpcLWP *cur_rpc_lwp = (*rpc_iter);
       if (cur_rpc_lwp) {
          if(cur_rpc_lwp->isReadyForIRPC()) {
             inferiorrpc_printf("%s[%d]: LWP %u ready for RPC...\n", 
                                FILE__, __LINE__, cur_rpc_lwp->get_lwp()->get_lwp_id());
             readyLWPRPC = true;
             break;
          }
          else
             inferiorrpc_printf("%s[%d]: LWP %u not for RPC...\n", 
                                FILE__, __LINE__, cur_rpc_lwp->get_lwp()->get_lwp_id());             
          if (cur_rpc_lwp->isProcessingIRPC()) {
             inferiorrpc_printf("%s[%d]: LWP %u currently processing RPC...\n", 
                                FILE__, __LINE__, cur_rpc_lwp->get_lwp()->get_lwp_id());
             processingLWPRPC = true;
          }
       }
       rpc_iter++;
    }

    // Only run thread RPCs if there are no LWP RPCs either waiting or in flight.

    if (!readyLWPRPC && !processingLWPRPC && !readyProcessRPC && !processingProcessRPC) {
        inferiorrpc_printf("%s[%d]: examining %d threads for RPCs...\n",
                           FILE__, __LINE__, thrs_.size());
        assert(thrs_.size());
       for (unsigned i = 0; i < thrs_.size(); i++) {
          rpcThr *curThr = thrs_[i];
          if(curThr == NULL) {
              continue;
          }
          if (curThr->isReadyForIRPC()) {
             inferiorrpc_printf("%s[%d]: Thread %u ready for RPC...\n", 
                                FILE__, __LINE__, curThr->get_thr()->get_tid());
             readyThrRPC = true;
             break;
          }
          else
             inferiorrpc_printf("%s[%d]: Thread %u not ready for RPC...\n", 
                                FILE__, __LINE__, curThr->get_thr()->get_tid());
             
          if (curThr->isRunningIRPC()) {
             inferiorrpc_printf("%s[%d]: Thread %u currently processing RPC...\n", 
                                FILE__, __LINE__, curThr->get_thr()->get_tid());
             processingThrRPC = true;
          }
          inferiorrpc_printf("%s[%d]: ---------------------------------------\n",
                             FILE__, __LINE__);
       }
    }
    inferiorrpc_printf("%s[%d]: RPC status dump: readyLWP %d, readyThr %d, readyProcess %d;\n",
		       FILE__, __LINE__, readyLWPRPC, readyThrRPC, readyProcessRPC);
    inferiorrpc_printf("%s[%d]: RPC status dump: wasRunning %d, processingLWP %d, processingThr %d\n",
		       FILE__, __LINE__, wasRunning, processingLWPRPC, processingThrRPC);
    if (!readyLWPRPC && !readyThrRPC && !readyProcessRPC) {
        if (wasRunning || processingLWPRPC || processingThrRPC) {
            // the caller expects the process to be running after
            // iRPCs finish, so continue the process here
            // ... or there is an iRPC in progress.
            needsToRun = true;
        }
        recursionGuard = false;
        inferiorrpc_printf("%s[%d]: Nothing to do, going home\n",
                           FILE__, __LINE__);
        return true;
    }

    // We have work to do. Pause the process.
    if (!proc()->IndependentLwpControl()) {
       if (!proc_->pause()) {
          recursionGuard = false;
          return false;
       }
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
                inferiorrpc_printf("%s[%d]: Result of posting RPC on LWP %d: %s\n",
				   FILE__, __LINE__, cur_rpc_lwp->get_lwp()->get_lwp_id(),
				   irpcLaunchStateAsString(lwpState));
                if (lwpState == irpcBreakpointSet ||
                    lwpState == irpcAgain ||
                    lwpState == irpcStarted) {
                    runProcessWhenDone = true;
                }
            }
            lwp_iter++;
        }
    }
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

            inferiorrpc_printf("%s[%d]: Result of posting RPC on thread %lu: %s\n",
                               FILE__, __LINE__, 
                               curThr->get_thr()->get_tid(),
                               irpcLaunchStateAsString(thrState));
            
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
        needsToRun = true;
        recursionGuard = false;

        if (!proc()->IndependentLwpControl())
            proc()->continueProc();

        return true;
    }

    // Weird... not sure how we can get here...
    if (wasRunning) {
        needsToRun = true;
    }
    
    recursionGuard = false;
    return false;
}

Address rpcMgr::createRPCImage(AstNodePtr action,
                               bool noCost,
                               bool shouldStopForResult,
                               Address &startAddr,
                               Address &breakAddr,
                               Address &stopForResultAddr,
                               Address &justAfter_stopForResultAddr,
                               Register &resultReg,
                               bool lowmem, 
                               dyn_thread *thr,
                               dyn_lwp * lwp) 
{
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
    irpcBuf.setAddrSpace(proc());
    irpcBuf.setLWP(lwp);
    irpcBuf.setThread(thr);
    
    irpcBuf.setRegisterSpace(registerSpace::irpcRegSpace(proc()));

#if defined(bug_syscall_changepc_rewind)
    // See comment in linux-power.C/linux-x86.C; search for "SGI"
    irpcBuf.fill(proc()->getAddressWidth(), codeGen::cgNOP);
#endif


    // Saves registers (first half of the base tramp) and whatever other
    // irpc-specific magic is necessary
    if (!emitInferiorRPCheader(irpcBuf)) {
        // a fancy dialog box is probably called for here...
        cerr << "createRPCtempTramp failed because emitInferiorRPCheader failed."
             << endl;
        return 0;
    }

    resultReg = REG_NULL;

    if (!action->generateCode(irpcBuf,
                              noCost,
                              resultReg)) assert(0);
    if (!shouldStopForResult) {
        irpcBuf.rs()->freeRegister(resultReg);
    }

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
            
            /* 2006-04-20: Rather than try to fix recursive iRPCs, necessary if we
               need to allocate more room in the mutatee, just use the newly-enlarged
               lowmemHeap for everything and assume that we won't be running more than
               512k worth of iRPCs at any given time.  This needs to be fixed later.
               
                 -- bernat via tlmiller */
            tempTrampBase = proc_->inferiorMalloc(irpcBuf.used(), lowmemHeap);
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

    startAddr = tempTrampBase;

#if defined(bug_syscall_changepc_rewind)
    // Some Linux kernels have the following behavior:
    // Process is in a system call;
    // We interrupt the system call;
    // We say "change PC to address N"
    // The kernel helpfully changes it to (N - address width)
    // The program crashes
    // See a more complete comment in linux-x86.C (search for "SGI"). 
    // For now, we pad the start of our code with NOOPS and change to just
    // after those; if we hit rewind behavior, then we're executing safe code.
    startAddr += proc()->getAddressWidth();
#endif

    return tempTrampBase;
}

/* ***************************************************** */

bool rpcMgr::emitInferiorRPCheader(codeGen &gen) 
{
    assert(irpcTramp);
    gen.beginTrackRegDefs();
    irpcTramp->generateSaves(gen, gen.rs(), NULL);
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
        insnCodeGen::generateTrap(gen);
        justAfter_stopForResultOffset = gen.used();
    }
    assert(irpcTramp);
    //irpcTramp->generateBT(gen);
    // Should already be built by the call to generateBT in emit... header
    irpcTramp->generateRestores(gen, gen.rs(), NULL);

    breakOffset = gen.used();
    insnCodeGen::generateTrap(gen);
    
    insnCodeGen::generateTrap(gen);

#if (defined(arch_x86) || defined(arch_x86_64))
     // X86 traps at the next insn, not the trap. So shift the
     // offsets accordingly

     if (shouldStopForResult) {
         stopForResultOffset += 1;
     }
     breakOffset += 1;
#endif
     gen.endTrackRegDefs();

    return true;
}



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
            // Oops... nothing we can do. 
            fprintf(stderr, "[%s:%d] WARNING: cancelling currently active iRPC\n",
                    __FILE__, __LINE__);
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
    inferiorrpc_printf("%s[%d]: Added running RPC to global list; %d total running\n",
                       FILE__, __LINE__, allRunningRPCs_.size());
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

const char *irpcStateAsString(irpcState_t state) {
    switch(state) {
    case irpcNotValid:
        return "IRPC Invalid";
        break;
    case irpcNotRunning:
        return "IRPC Posted, not running";
        break;
    case irpcRunning:
        return "IRPC Running";
        break;
    case irpcWaitingForSignal:
        return "IRPC Waiting for Signal";
        break;
    case irpcNotReadyForIRPC:
        return "IRPC Not Ready";
        break;
    default:
        assert(0);
        break;
    }
    return NULL;
}

const char *irpcLaunchStateAsString(irpcLaunchState_t state) {
    switch(state) {
    case irpcNoIRPC:
        return "No IRPC to run";
        break;
    case irpcStarted:
        return "IRPC Started";
        break;
    case irpcAgain:
        return "IRPC not started, try again";
        break;
    case irpcBreakpointSet:
        return "Set breakpoint for syscall exit";
        break;
    case irpcError:
        return "IRPC Error";
        break;
    default:
        assert(0);
        break;
    }
    return NULL;
}

    

