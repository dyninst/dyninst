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

#include "dyninstAPI/src/rpcMgr.h"
#include "dyninstAPI/src/process.h"
#include "dyninstAPI/src/dyn_thread.h"
#include "dyninstAPI/src/dyn_lwp.h"



void rpcLwp::postIRPC(inferiorRPCtoDo todo)
{
    thrRPCsWaitingToStart += todo;
}

bool rpcLwp::readyIRPC() const
{
    // If we're running an RPC, we're not ready to start one
    if (isRunningIRPC()) {
        return false;
    }
    if (thrRPCsWaitingToStart.empty()) {
        return false;
    }
    return true;
}

// wasRunning: once the RPC is finished, leave process paused (false) or running (true)
irpcLaunchState_t rpcLwp::launchThreadIRPC(bool wasRunning)
{
    if (!readyIRPC()) {
        return irpcNoIRPC;
    }
    inferiorRPCtoDo todo = thrRPCsWaitingToStart[0];
    // There is an RPC to run. Yay.
    // We pause at a process level
    process *proc = lwp->proc();
    proc->pause();

    // Check if we're in a system call
    if (lwp->executingSystemCall()) {
        // Oh, crud. Can't run the iRPC now. Check to see if we'll
        // be able to do it later with any degree of accuracy
        if (lwp->setSyscallExitTrap()) {
            // Record what we were doing. When the trap is received
            // the process will be paused, so we want to be able
            // to restore _current_ state
            wasRunningBeforeSyscall_ = wasRunning;
            irpcState_ = irpcWaitingForTrap;
            return irpcTrapSet;
        }
        else {
            // Weren't able to set the breakpoint, so all we can
            // do is try later
            irpcState_ = irpcNotReadyForIRPC;
            return irpcAgain;
        }
    }

    // We passed the system call check, so the thread is in a state
    // where it is possible to run iRPCs.
    struct dyn_saved_regs *theSavedRegs = NULL;
    // Some platforms save daemon-side, some save process-side (on the stack)
    if (proc->getRpcMgr()->rpcSavesRegs()) {
        theSavedRegs = lwp->getRegisters();
        if (theSavedRegs == (struct dyn_saved_regs *)-1) {
            // Should only happen if we're in a syscall, which is 
            // caught above
            return irpcError;
        }
    }

    thrRPCsWaitingToStart.removeOne();
    
    // Build the in progress struct
    thrCurrRunningRPC.rpclwp = this;
    
    // Copy over state
    thrCurrRunningRPC.callbackFunc = todo.callbackFunc;
    thrCurrRunningRPC.userData = todo.userData;
    thrCurrRunningRPC.savedRegs = theSavedRegs;
    thrCurrRunningRPC.seq_num = todo.seq_num;
    
    thrCurrRunningRPC.wasRunning = wasRunning;

    Address RPCImage =
       proc->getRpcMgr()->createRPCImage(todo.action, // AST tree
                                         todo.noCost,
                                         (thrCurrRunningRPC.callbackFunc != NULL), // Callback?
                                         thrCurrRunningRPC.breakAddr, // Fills in 
                                         thrCurrRunningRPC.stopForResultAddr,
                                         thrCurrRunningRPC.justAfter_stopForResultAddr,
                                         thrCurrRunningRPC.resultRegister,
                                         todo.lowmem, // Where to allocate
                                         lwp,
                                         false); // Unused: create in function form
    if (RPCImage == 0) {
        cerr << "launchRPC failed, couldn't create image" << endl;
        return irpcError;
    }
    
    thrCurrRunningRPC.firstInstrAddr = RPCImage;

#if !defined(i386_unknown_nt4_0) && !defined(mips_unknown_ce2_11)
    // Actually, only need this if a restoreRegisters won't reset
    // the PC back to the original value
    Frame curFrame = lwp->getActiveFrame();
    thrCurrRunningRPC.origPC = curFrame.getPC();
#endif    

    // Get a copy of the current stack and save it
    if(! lwp->markRunningIRPC()) {
       cerr << "launchRPC failed: failed to obtain stack walk" << endl;
       return irpcError;
    }
    
    // Launch this sucker. Change the PC, and the caller will set running
    if (!lwp->changePC(RPCImage, NULL)) {
        cerr << "launchRPC failed: couldn't set PC" << endl;
        return irpcError;
    }
#if defined(i386_unknown_linux2_0)
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
      irpcState_ = irpcRunning;
      return irpcStarted;
}

irpcLaunchState_t rpcLwp::launchPendingIRPC() {
    assert(irpcState_ == irpcWaitingForTrap);
    irpcState_ = irpcNotRunning;
    return launchThreadIRPC(wasRunningBeforeSyscall_);
}

bool rpcLwp::isRunningIRPC() const {
    return (irpcState_ == irpcRunning ||
            irpcState_ == irpcWaitingForTrap);
}

bool rpcLwp::isWaitingForTrap() const {
    return (irpcState_ == irpcWaitingForTrap);
}


Address rpcLwp::getIRPCRetValAddr()
{
    if (irpcState_ != irpcRunning) return 0;
    return thrCurrRunningRPC.stopForResultAddr;
}

// Called when an inferior RPC reaches the "grab return value" stage
bool rpcLwp::handleRetValIRPC() {
    if (!thrCurrRunningRPC.callbackFunc)
        return false;
    Address returnValue = 0;
    
    if (thrCurrRunningRPC.resultRegister != Null_Register) {
        // We have a result that we care about
        returnValue = lwp->readRegister(thrCurrRunningRPC.resultRegister);
        // Okay, this bit I don't understand. 
        // Oh, crud... we should have a register space for each thread.
        // Or not do this at all. 
        extern registerSpace *regSpace;
        regSpace->freeRegister(thrCurrRunningRPC.resultRegister);
    }
    thrCurrRunningRPC.resultValue = (void *)returnValue;
    // we continue the process...but not quite at the PC where we left off, since
    // that will just re-do the trap!  Instead, we need to continue at the location
    // of the next instruction.
    if (!lwp->changePC(thrCurrRunningRPC.justAfter_stopForResultAddr, NULL))
        assert(false);
    return true;
}

Address rpcLwp::getIRPCFinishedAddr()
{
    if (irpcState_ != irpcRunning) return 0;
    return thrCurrRunningRPC.breakAddr;
}

// False: leave process paused
// True: run the process
bool rpcLwp::handleCompletedIRPC() {
   lwp->markDoneRunningIRPC();
  
    // step 1) restore registers:
    // Assumption: LWP has not changed. 
    if (thrCurrRunningRPC.savedRegs) {
        if (!lwp->restoreRegisters(thrCurrRunningRPC.savedRegs)) {
            cerr << "handleTrapIfDueToRPC failed because restoreRegisters failed" << endl;
            assert(false);
        }
        // The above implicitly must restore the PC.
    }
    else
        if (!lwp->changePC(thrCurrRunningRPC.origPC, thrCurrRunningRPC.savedRegs)) 
            assert(0 && "Failed to reset PC");
    
    // step 2) delete temp tramp
    process *proc = lwp->proc();
    proc->inferiorFree(thrCurrRunningRPC.firstInstrAddr);
    
    // step 3) invoke user callback, if any
    
    // save enough information to call the callback function, if needed
    inferiorRPCcallbackFunc cb = thrCurrRunningRPC.callbackFunc;
    void* userData = thrCurrRunningRPC.userData;
    void* resultValue = thrCurrRunningRPC.resultValue;

    // release the RPC struct
    if (thrCurrRunningRPC.savedRegs) delete thrCurrRunningRPC.savedRegs;
    
    irpcState_ = irpcNotRunning;
    
    // call the callback function if needed
    if( cb != NULL ) {
        (*cb)(proc, userData, resultValue);
    }

    // Before we continue the process: if there is another RPC,
    // start it immediately (instead of waiting for an event loop
    // to do the job)
    if (readyIRPC()) {
        irpcLaunchState_t launchState =
           launchThreadIRPC(thrCurrRunningRPC.wasRunning);
        if (launchState == irpcStarted) {
            // Run the process
            return true;
        }
    }

    return (thrCurrRunningRPC.wasRunning);
}

void rpcMgr::newLwpFound(dyn_lwp *lwp) {
   assert(findRpcLwp(lwp) == NULL);
   rpcLwp *newRpcLwp = createRpcLwp(lwp);
}

void rpcMgr::deleteLwp(dyn_lwp *lwp) {
   rpcLwp *matching_rpcLwp = findRpcLwp(lwp);
   if(matching_rpcLwp) {
      delete matching_rpcLwp;
      rpcLwpBuf.undef(lwp);
   }
}

void rpcMgr::cleanRPCreadyToLaunch(int mid) 
{
   vectorSet<inferiorRPCtoDo> tmpRPCsWaitingToStart;

   while (RPCsWaitingToStart.size() > 0) {
      inferiorRPCtoDo currElem = RPCsWaitingToStart.removeOne();

      if (currElem.mid == mid) {
         break;
      } else {
         tmpRPCsWaitingToStart += currElem;
      }
   }
   // reconstruct RPCsWaitingToStart
   while (tmpRPCsWaitingToStart.size() > 0) {
      RPCsWaitingToStart += tmpRPCsWaitingToStart.removeOne();
   }
}

// post RPC toDo for process
void rpcMgr::postRPCtoDo(AstNode *action, bool noCost,
                         inferiorRPCcallbackFunc callbackFunc,
                         void *userData, int mid, bool lowmem) 
{
   static int sequence_num = 0;
   // posts an RPC, but does NOT make any effort to launch it.
   inferiorRPCtoDo theStruct;

   theStruct.action = action;
   theStruct.noCost = noCost;
   theStruct.callbackFunc = callbackFunc;
   theStruct.userData = userData;
   theStruct.mid = mid;
   theStruct.lowmem = lowmem;
   theStruct.rpclwp = NULL;
   theStruct.seq_num = sequence_num++;

   RPCsWaitingToStart += theStruct;   
}

void rpcMgr::postRPCtoDo(AstNode *action, bool noCost,
                         inferiorRPCcallbackFunc callbackFunc,
                         void *userData, int mid, dyn_thread *thr, bool lowmem)
{
   if(thr->get_lwp() == NULL) {
      cerr << "can't postRPCtoDo for thread without assigned LWP, tid: %d"
           << thr->get_tid() << endl;
      cerr << "RPC -------------------------\n";
      action->print();
      assert(false);
   }

   postRPCtoDo(action, noCost, callbackFunc, userData, mid, thr->get_lwp(),
               lowmem);
}

void rpcMgr::postRPCtoDo(AstNode *action, bool noCost,
                         inferiorRPCcallbackFunc callbackFunc,
                         void *userData, int mid, dyn_lwp *lwp, bool lowmem)
{
   static int sequence_num = 0;
   // posts an RPC, but does NOT make any effort to launch it.
   inferiorRPCtoDo theStruct;

   rpcLwp *rpc_lwp = NULL;
   if((rpc_lwp = findRpcLwp(lwp)) == NULL) {
      rpc_lwp = createRpcLwp(lwp);
   }

   theStruct.action = action;
   theStruct.noCost = noCost;
   theStruct.callbackFunc = callbackFunc;
   theStruct.userData = userData;
   theStruct.mid = mid;
   theStruct.lowmem = lowmem;
   theStruct.rpclwp = rpc_lwp;
   theStruct.seq_num = sequence_num++;

   rpc_lwp->postIRPC(theStruct);

   //fprintf(stderr, "Posted RPC with sequence num %d, lwp %x\n",
   //      theStruct.seq_num, theStruct.rpclwp);  
}

bool rpcMgr::existsRPCPending() const {
   // Return if the process or any thread
   // has an RPC pending
   bool retval = false;

   dictionary_hash<dyn_lwp *, rpcLwp *>::iterator rpc_iter = 
      rpcLwpBuf.begin();
   while(rpc_iter != rpcLwpBuf.end()) {
      rpcLwp *cur_rpc_lwp = (*rpc_iter);
      if(cur_rpc_lwp->readyIRPC())
         retval = true;
      
      rpc_iter++;
   }

   return retval;
}

bool rpcMgr::existsRPCinProgress() const {
   bool runningRPC = false;

   dictionary_hash<dyn_lwp *, rpcLwp *>::iterator rpc_iter =
      rpcLwpBuf.begin();
   while(rpc_iter != rpcLwpBuf.end()) {
      rpcLwp *cur_rpc_lwp = (*rpc_iter);
      if(cur_rpc_lwp->isRunningIRPC())
         runningRPC = true;
      
      rpc_iter++;
   }

   return runningRPC;
}

// Used to tell if someone is waiting for a system
// call to complete. The Linux SIGILL->SIGTRAP
// handler code uses this, but I think that's it.

bool rpcMgr::existsRPCWaitingForSyscall() const {
   bool isInSyscall = false;

   dictionary_hash<dyn_lwp *, rpcLwp *>::iterator rpc_iter = 
      rpcLwpBuf.begin();
   while(rpc_iter != rpcLwpBuf.end()) {
      rpcLwp *cur_rpc_lwp = (*rpc_iter);
      if(cur_rpc_lwp->isIRPCwaitingForSyscall())
         isInSyscall = true;
      
      rpc_iter++;
   }

   return isInSyscall;
}


bool rpcMgr::readyIRPC() const
{
   return !RPCsWaitingToStart.empty();
}

bool rpcMgr::isRunningIRPC() const {
   bool res = false;

   dictionary_hash<dyn_lwp *, rpcLwp *>::iterator rpc_iter = 
      rpcLwpBuf.begin();
   while(rpc_iter != rpcLwpBuf.end()) {
      rpcLwp *cur_rpc_lwp = (*rpc_iter);
      if(cur_rpc_lwp->isRunningIRPC())
         res = true;

      rpc_iter++;   
   }

   return res;
}


bool rpcMgr::thr_IRPC() {
#if !defined(BPATCH_LIBRARY)
   if (proc->isParadynBootstrapped())
      return true;
#endif
   return false;
}



bool rpcMgr::distributeRPCsOverLwps()
{
   // Take process-scope RPCs and distribute them
   // over thread RPC queues. Also check if the thread
   // can run the RPC -- otherwise it's not much good. 

   // Iterate over all threads. If they:
   // 1) Don't have pending RPCs
   // 2) Aren't running an RPC/waiting for syscall trap
   // 3) Aren't in a system call
   // Assign an RPC to that thread
   dictionary_hash<dyn_lwp *, rpcLwp *>::iterator rpc_iter = rpcLwpBuf.begin();
   while(rpc_iter != rpcLwpBuf.end()) {
      rpcLwp *cur_rpc_lwp = (*rpc_iter);
      if (RPCsWaitingToStart.size() == 0)
         break; // No more to do here

      // Checks
      if(cur_rpc_lwp->readyIRPC()) {
         fprintf(stderr, "Skipped lwp %p: pending iRPC\n", cur_rpc_lwp);
         continue;
      }

      if(cur_rpc_lwp->isRunningIRPC() ||
         cur_rpc_lwp->isIRPCwaitingForSyscall()) {
         fprintf(stderr, "Skipped lwp %p: running iRPC\n", cur_rpc_lwp);
         continue;
      }

      if(cur_rpc_lwp->get_lwp()->executingSystemCall()) {
         fprintf(stderr, "Skipped lwp %p: in syscall\n", cur_rpc_lwp);
         continue;
      }

      // Assign this RPC to the thread
      inferiorRPCtoDo rpc = RPCsWaitingToStart[0];
      rpc.rpclwp = cur_rpc_lwp;
      cur_rpc_lwp->postIRPC(rpc);

      RPCsWaitingToStart.removeOne();
      rpc_iter++;
   }
   return true;
}


bool rpcMgr::rpcSavesRegs()
{
#if defined(rs6000_ibm_aix4_1)
   return false;
#else
   return true;
#endif
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

   bool RPCwasLaunched = false;
   bool readyRPC = false;
   if (readyIRPC()) {
      readyRPC = true;
   }    
   else {
      dictionary_hash<dyn_lwp *, rpcLwp *>::iterator rpc_iter = 
         rpcLwpBuf.begin();
      while(rpc_iter != rpcLwpBuf.end()) {
         rpcLwp *cur_rpc_lwp = (*rpc_iter);
         if(cur_rpc_lwp->readyIRPC()) {
            readyRPC = true;
            break;
         }
         rpc_iter++;
      }
   }

   if (!readyRPC) {
      if (wasRunning) {
         // the caller expects the process to be running after
         // iRPCs finish, so continue the process here
         proc->continueProc();
      }
        
      return false;
   }   

   //cerr << "readyRPC = " << (int)readyRPC << ", #rpcsQeuedToStart: "
   //     << RPCsWaitingToStart.size() << endl;

   // We have work to do. Pause the process.
   if (!proc->pause()) {
      cerr << "FAILURE TO PAUSE PROCESS in launchRPCs" << endl;
      return false;
   }
   // Okay, there is an inferior RPC to do somewhere. Now we just need
   // to launch ze sucker
   // MT: take all RPCs in the process queue and find a thread to run
   // them in. Requirements: thread must not have any local RPCs ready,
   // and must not be in a system call. Reason: with so many threads to 
   // choose from, why wait longer than necessary?
   distributeRPCsOverLwps();

   // MT: if any thread posts an iRPC, run the process. If no threads
   // post an RPC, revert to previous state

   // Loop over all rpcLwps and tell them to go run themselves
   dictionary_hash<dyn_lwp *, rpcLwp *>::iterator rpc_iter = 
      rpcLwpBuf.begin();
   while(rpc_iter != rpcLwpBuf.end()) {
      rpcLwp *cur_rpc_lwp = (*rpc_iter);
      if(cur_rpc_lwp->readyIRPC()) {
         irpcLaunchState_t thr_state;
         thr_state = cur_rpc_lwp->launchThreadIRPC(wasRunning);
         if (thr_state == irpcStarted ||
                thr_state == irpcTrapSet)
            RPCwasLaunched = true;
      }
      rpc_iter++;
   }

   if (RPCwasLaunched) {
      if (!proc->continueProc()) {
         return false;
      }
      return true;
   }

   // Hrm... we thought there was an RPC ready, but there wasn't.
   // Most likely cause: we're in a system call, and so while there
   // is an RPC pending it isn't launchable. Revert the process
   // to the previous state. We'll try and catch the RPC later.

   if (wasRunning) {
      proc->continueProc();
   }

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
                               bool lowmem,
                               dyn_lwp * /*lwp*/,
                               bool isFunclet)
{
   // Returns addr of temp tramp, which was allocated in the inferior heap.
   // You must free it yourself when done.
   // Note how this is, in many ways, a greatly simplified version of
   // addInstFunc().
  
   // Temp tramp structure: save; code; restore; trap; illegal
   // the illegal is just to make sure that the trap never returns
   // note that we may not need to save and restore anything, since we've
   // already done a GETREGS and we'll restore with a SETREGS, right?
   unsigned char insnBuffer[4096];
  
   initTramps(); // initializes "regSpace", but only the 1st time called
   extern registerSpace *regSpace;
   regSpace->resetSpace();
  
   Address count = 0; // number of bytes required for RPCtempTramp
  
   // The following is implemented in an arch-specific source file...
   // isFunclet added since we might save more registers based on that
   if (!emitInferiorRPCheader(insnBuffer, count, isFunclet)) {
      // a fancy dialog box is probably called for here...
      cerr << "createRPCtempTramp failed because emitInferiorRPCheader failed."
           << endl;
      return 0;
   }

   if (proc->multithread_ready()) {
      // We need to put in a branch past the rest of the RPC (to the trailer,
      // actually) if the MT information given is incorrect. That's the
      // skipBRaddr part.
      generateMTpreamble((char*)insnBuffer,count, proc);
   }

   resultReg = (Register)action->generateCode(proc, regSpace,
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
                               justAfter_stopForResultOffset, isFunclet)) {
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
      tempTrampBase = proc->inferiorMalloc(count, lowmemHeap);
   }
   else
   {
      /* May cause another inferior RPC to dynamically allocate a new heap
         in the inferior. */
      tempTrampBase = proc->inferiorMalloc(count, anyHeap);
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
   if (!proc->writeDataSpace((void*)tempTrampBase, count, insnBuffer)) {
      // should put up a nice error dialog window
      cerr << "createRPCtempTramp failed because writeDataSpace failed" <<endl;
      return 0;
   }
  
   extern int trampBytes; // stats.h
   trampBytes += count;
  
   return tempTrampBase;
}

Address rpcMgr::getIRPCRetValAddr()
{
   if (irpcState_ != irpcRunning) return 0;
   return currRunningIRPC.stopForResultAddr;
}

// Called when an inferior RPC reaches the "grab return value" stage
bool rpcMgr::handleRetValIRPC() {
   if (!currRunningIRPC.callbackFunc)
      return false;
   Address returnValue = 0;
    
   if (currRunningIRPC.resultRegister != Null_Register) {
      // We have a result that we care about
      returnValue =
         proc->getDefaultLWP()->readRegister(currRunningIRPC.resultRegister);
      // Okay, this bit I don't understand. 
      // Oh, crud... we should have a register space for each thread.
      // Or not do this at all. 
      extern registerSpace *regSpace;
      regSpace->freeRegister(currRunningIRPC.resultRegister);
   }
   currRunningIRPC.resultValue = (void *)returnValue;
   // we continue the process...but not quite at the PC where we left off,
   // since that will just re-do the trap!  Instead, we need to continue at
   // the location of the next instruction.
   if (!proc->getDefaultLWP()->changePC(currRunningIRPC.justAfter_stopForResultAddr, NULL))
      assert(false);
   return true;
}

Address rpcMgr::getIRPCFinishedAddr()
{
   if (irpcState_ != irpcRunning) return 0;
   return currRunningIRPC.breakAddr;
}

// False: leave process paused
// True: run the process
bool rpcMgr::handleCompletedIRPC() {
   // step 1) restore registers:
   // Assumption: LWP has not changed. 
   if (currRunningIRPC.savedRegs) {
      if (!proc->getDefaultLWP()->restoreRegisters(currRunningIRPC.savedRegs))
      {
         cerr << "handleTrapIfDueToRPC failed because restoreRegisters failed" 
              << endl;
         assert(false);
      }
      // The above implicitly must restore the PC.
   }
   else
      if (!proc->getDefaultLWP()->changePC(currRunningIRPC.origPC, 
                                           currRunningIRPC.savedRegs)) 
         assert(0 && "Failed to reset PC");
    
   // step 2) delete temp tramp
   proc->inferiorFree(currRunningIRPC.firstInstrAddr);
    
   // step 3) invoke user callback, if any
    
   // save enough information to call the callback function, if needed
   inferiorRPCcallbackFunc cb = currRunningIRPC.callbackFunc;
   void* userData = currRunningIRPC.userData;
   void* resultValue = currRunningIRPC.resultValue;

   // release the RPC struct
   if (currRunningIRPC.savedRegs) delete currRunningIRPC.savedRegs;
    
   irpcState_ = irpcNotRunning;
    
   // call the callback function if needed
   if( cb != NULL ) {
      (*cb)(proc, userData, resultValue);
   }

   // Before we continue the process: if there is another RPC,
   // start it immediately (instead of waiting for an event loop
   // to do the job)
   if (readyIRPC()) {
      launchRPCs(currRunningIRPC.wasRunning);
   }

   return (currRunningIRPC.wasRunning);
}

// This function handles any traps due to RPCs
// This includes:
// Trap when an IRPC completes
// Trap when an IRPC stops to read a result
// Trap when a system call completes if an IRPC is pending
bool rpcMgr::handleTrapIfDueToRPC() {
   // We will return this parameter to say whether the 
   // trap was something we needed or should be passed along

   bool handledTrap = false;
   // If true: we did whatever work was associated with the trap,
   // so quit immediately (don't fall through). We'll get further
   // traps for more work.
   bool ateTrap = false;

   // The signal handler default is to leave the process paused.
   // If we want it running, we do so explicitly via a call to 
   // continueProc(). 
   bool runProcess = false;

   // Two main possibilities: a thread is stopped at an interesting address,
   // or a thread was waiting for a system call to complete (and it has).
   // We check the first case first.
   dictionary_hash<dyn_lwp *, rpcLwp *>::iterator rpc_iter = rpcLwpBuf.begin();
   while(rpc_iter != rpcLwpBuf.end()) {
      rpcLwp *cur_rpc_lwp = (*rpc_iter);

      if(cur_rpc_lwp->isRunningIRPC()) {
         Frame activeFrame = cur_rpc_lwp->get_lwp()->getActiveFrame();
         Address retvalAddr = cur_rpc_lwp->getIRPCRetValAddr();
         Address completedAddr = cur_rpc_lwp->getIRPCFinishedAddr();

         if (activeFrame.getPC() == retvalAddr) {
            cur_rpc_lwp->handleRetValIRPC();
            ateTrap = true;
            handledTrap = true;
            runProcess = true;
         }
         else if (activeFrame.getPC() == completedAddr) {
            // handleCompleted returns true if the process
            // should be run, false otherwise
            if(cur_rpc_lwp->handleCompletedIRPC())
               runProcess = true;
            ateTrap = true;
            handledTrap = true;
         }
      }
      if (ateTrap) break;
      rpc_iter++;
   }

   if (!handledTrap) {
      // Trap signal was not handled, check to see if it's due to 
      // a system call exiting
      dyn_lwp *comp_lwp = proc->checkSyscallExit();
      rpcLwp *rpclwp = findRpcLwp(comp_lwp);

      if(rpclwp) {
         // A thread hit a system call... so launch an RPC
         if(rpclwp->isWaitingForTrap()) {
            irpcLaunchState_t thr_state;
            thr_state = rpclwp->launchPendingIRPC();
            ateTrap = true;
            handledTrap = true;
            runProcess = true;
         }
         else {
            // A thread hit a system call exit, but was not expecting
            // it. Treat this as a handled trap, but don't try and
            // launch an iRPC
            ateTrap = true;
            handledTrap = true;
            runProcess = true;
         }
      }
   }
    
   if (runProcess)
      proc->continueProc();

   return handledTrap;
}


