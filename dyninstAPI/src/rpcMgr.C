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

void rpcMgr::postRPCtoDo(AstNode *action, bool noCost,
                         inferiorRPCcallbackFunc callbackFunc,
                         void *userData,
                         int mid, 
                         dyn_thread *thr,
                         dyn_lwp *lwp,
                         bool lowmem)
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
   theStruct.thr = thr;
   if (!lwp)
      theStruct.lwp = proc->getDefaultLWP();
   else
      theStruct.lwp = lwp;
   theStruct.seq_num = sequence_num++;
   if (theStruct.thr) {
      thr->postIRPC(theStruct);
   }
   else {
      RPCsWaitingToStart += theStruct;   
   }
   //fprintf(stderr, "Posted RPC with sequence num %d, thr %x, lwp %x\n",
   //      theStruct.seq_num, theStruct.thr, theStruct.lwp);
  
}

bool rpcMgr::existsRPCPending() const {
   // Return if the process or any thread
   // has an RPC pending
   bool retval = false;
   if (readyIRPC())
      retval = true;
   for (unsigned thr_iter = 0; thr_iter < proc->threads.size(); thr_iter++)
      if (proc->threads[thr_iter]->readyIRPC())
         retval = true;
   return retval;
}

bool rpcMgr::existsRPCinProgress() const {
   bool runningRPC = false;
   if (isRunningIRPC())
      runningRPC = true;
   for (unsigned i = 0; i < proc->threads.size(); i++)
      if (proc->threads[i]->isRunningIRPC())
         runningRPC = true;
   return runningRPC;
}

// Used to tell if someone is waiting for a system
// call to complete. The Linux SIGILL->SIGTRAP
// handler code uses this, but I think that's it.

bool rpcMgr::existsRPCWaitingForSyscall() const {
   bool isInSyscall = false;
   if (isIRPCwaitingForSyscall())
      isInSyscall = true;
   else
      for (unsigned i = 0; i < proc->threads.size(); i++) {
         if (proc->threads[i]->isIRPCwaitingForSyscall())
            isInSyscall = true;
      }
   return isInSyscall;
}


bool rpcMgr::readyIRPC() const
{
   // If we're running an RPC, we're not ready to start one
   if (isRunningIRPC())  {
      return false;
   }
    

   return !RPCsWaitingToStart.empty();
}

bool rpcMgr::isRunningIRPC() const {
   return (irpcState_ == irpcRunning ||
           irpcState_ == irpcWaitingForTrap);
}

bool rpcMgr::thr_IRPC() {
#if !defined(BPATCH_LIBRARY)
   if (proc->isParadynBootstrapped())
      return true;
#endif
   return false;
}



bool rpcMgr::distributeRPCsOverThreads()
{
   // Take process-scope RPCs and distribute them
   // over thread RPC queues. Also check if the thread
   // can run the RPC -- otherwise it's not much good. 
   // ST: passthrough.

   // Iterate over all threads. If they:
   // 1) Don't have pending RPCs
   // 2) Aren't running an RPC/waiting for syscall trap
   // 3) Aren't in a system call
   // Assign an RPC to that thread
   for (unsigned thr_iter = 0; thr_iter < proc->threads.size(); thr_iter++) {
      if (RPCsWaitingToStart.size() == 0)
         break; // No more to do here
      dyn_thread *thr = proc->threads[thr_iter];
      // Checks
      if (thr->readyIRPC()) {
         fprintf(stderr, "Skipped thread %d: pending iRPC\n", thr_iter);
         continue;
      }
        
      if (thr->isRunningIRPC() ||
          thr->isIRPCwaitingForSyscall()) {
         fprintf(stderr, "Skipped thread %d: running iRPC\n", thr_iter);
         continue;
      }
      thr->updateLWP();
      if (thr->get_lwp()->executingSystemCall()) {
         fprintf(stderr, "Skipped thread %d: in syscall\n", thr_iter);
         continue;
      }
      // Assign this RPC to the thread
      inferiorRPCtoDo rpc = RPCsWaitingToStart[0];
      rpc.thr = thr;
      rpc.lwp = thr->get_lwp();
      thr->postIRPC(rpc);
      RPCsWaitingToStart.removeOne();
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
   processState state_before_paused = proc->status();

   unsigned thr_iter; // Useful iterator
   bool RPCwasLaunched = false;
   bool readyRPC = false;
   if (readyIRPC()) {
      readyRPC = true;
   }    
   else for (thr_iter = 0; thr_iter < proc->threads.size(); thr_iter++) {
      if (proc->threads[thr_iter]->readyIRPC()) {
         readyRPC = true;
         break;
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
   distributeRPCsOverThreads();
    
   // In the MT case, we now ignore the process threads. In ST, we
   // cannot. This leads to a split in the control login, which is 
   // unfortunate but necessary.

   // MT: if any thread posts an iRPC, run the process. If no threads
   // post an RPC, revert to previous state

   if (proc->threads.size()) {
      // Loop over all threads and tell them to go run themselves
      for (thr_iter = 0; thr_iter < proc->threads.size(); thr_iter++) {
         if (proc->threads[thr_iter]->readyIRPC()) {
            irpcLaunchState_t thr_state;
            thr_state = proc->threads[thr_iter]->launchThreadIRPC(wasRunning);
            if (thr_state == irpcStarted ||
                thr_state == irpcTrapSet)
               RPCwasLaunched = true;
         }
      }
   }
   else {
      // Old style. We're stuck with doing things the hard way.
      // This code is a (near) carbon copy of the thread RPC
      // launch mechanisms. The only difference is in the thread/lwp
      // variables
      irpcLaunchState_t proc_state;
      proc_state = launchProcessIRPC(wasRunning);
      if (proc_state == irpcStarted ||
          proc_state == irpcTrapSet) 
         RPCwasLaunched = true;
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
   // to the previous state.
    
   if (state_before_paused == running) {
      proc->continueProc();
   }

   return false;
}

irpcLaunchState_t rpcMgr::launchProcessIRPC(bool wasRunning) {
    
   if (!readyIRPC()) {
      return irpcNoIRPC;
   }
   inferiorRPCtoDo todo = RPCsWaitingToStart[0];
   // Note: we don't remove the RPC until we're sure that we
   // can run it.

   if (proc->getDefaultLWP()->executingSystemCall()) {
      if (proc->getDefaultLWP()->setSyscallExitTrap()) {
         irpcState_ = irpcWaitingForTrap;
         wasRunningBeforeSyscall_ = wasRunning;
         // Effectively, we've launched an RPC... it will
         // just take a little longer to run
         return irpcTrapSet;
      } else {
         irpcState_ = irpcNotReadyForIRPC;
         return irpcAgain;
      }
   }
   struct dyn_saved_regs *theSavedRegs = NULL;
    
   if (rpcSavesRegs()) {
      theSavedRegs = proc->getDefaultLWP()->getRegisters();
      if (theSavedRegs == (struct dyn_saved_regs *)-1) {
         irpcState_ = irpcNotReadyForIRPC;
         cerr << "Failed to read registers" << endl;
         return irpcError;
      }
   }
   else {
      theSavedRegs = NULL;
   }
    
   RPCsWaitingToStart.removeOne();

   currRunningIRPC.thr = todo.thr;
   currRunningIRPC.lwp = todo.lwp;
    
   // Copy over data
   currRunningIRPC.callbackFunc = todo.callbackFunc;
   currRunningIRPC.userData = todo.userData;
   currRunningIRPC.savedRegs = theSavedRegs;
   currRunningIRPC.seq_num = todo.seq_num;
    
   currRunningIRPC.wasRunning = wasRunning;
    
   Address RPCImage = createRPCImage(todo.action,
                                     todo.noCost,
                                     (currRunningIRPC.callbackFunc != NULL),
                                     currRunningIRPC.breakAddr,
                                     currRunningIRPC.stopForResultAddr,
                                     currRunningIRPC.justAfter_stopForResultAddr,
                                     currRunningIRPC.resultRegister,
                                     todo.lowmem,
                                     todo.thr,
                                     todo.lwp,
                                     false);
   if (RPCImage == 0) {
      cerr << "launchRPCs failed because createRPCImage failed"
           << endl;
      irpcState_ = irpcNotReadyForIRPC;
      return irpcError;
   }
   currRunningIRPC.firstInstrAddr = RPCImage;
    
   //ccw 20 july 2000 : 29 mar 2001
#if !(defined i386_unknown_nt4_0) && !(defined mips_unknown_ce2_11)
   Frame frame = currRunningIRPC.lwp->getActiveFrame();
   currRunningIRPC.origPC = frame.getPC();
#endif
   if (!currRunningIRPC.lwp->changePC(RPCImage, theSavedRegs))
   {
      cerr << "launchRPCs failed because changePC() failed" << endl;
      if (wasRunning) proc->continueProc();
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
   if (!currRunningIRPC.lwp->clearOPC())
   {
      cerr << "launchRPCs failed because clearOPC() failed"
           << endl;
      if (wasRunning) proc->continueProc();
      return irpcError;
   }
#endif
   irpcState_ = irpcRunning;
   Frame active = proc->getDefaultLWP()->getActiveFrame();
    
   return irpcStarted;
}

irpcLaunchState_t rpcMgr::launchPendingIRPC() {
   return launchProcessIRPC(wasRunningBeforeSyscall_);
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
                               dyn_thread * /*thr*/,
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
      // We need to put in a branch past the rest of the RPC (to the trailer, actually)
      // if the MT information given is incorrect. That's the skipBRaddr part.
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
   if (!emitInferiorRPCtrailer(insnBuffer, count,
                               breakOffset, shouldStopForResult, stopForResultOffset,
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
      justAfter_stopForResultAddr = tempTrampBase + justAfter_stopForResultOffset;
   } 
   else {
      stopForResultAddr = justAfter_stopForResultAddr = 0;
   }
  
   if (pd_debug_infrpc)
      cerr << "createRPCtempTramp: temp tramp base=" << (void*)tempTrampBase
           << ", stopForResultAddr=" << (void*)stopForResultAddr
           << ", justAfter_stopForResultAddr=" << (void*)justAfter_stopForResultAddr
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
      cerr << "createRPCtempTramp failed because writeDataSpace failed" << endl;
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
      returnValue = proc->getDefaultLWP()->readRegister(currRunningIRPC.resultRegister);
      // Okay, this bit I don't understand. 
      // Oh, crud... we should have a register space for each thread.
      // Or not do this at all. 
      extern registerSpace *regSpace;
      regSpace->freeRegister(currRunningIRPC.resultRegister);
   }
   currRunningIRPC.resultValue = (void *)returnValue;
   // we continue the process...but not quite at the PC where we left off, since
   // that will just re-do the trap!  Instead, we need to continue at the location
   // of the next instruction.
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
         cerr << "handleTrapIfDueToRPC failed because restoreRegisters failed" << endl;
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
      irpcLaunchState_t launchState = launchProcessIRPC(currRunningIRPC.wasRunning);
      if (launchState == irpcStarted) {
         // Run the process
         return true;
      }
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
   unsigned thr_iter;
   for (thr_iter = 0; thr_iter < proc->threads.size(); thr_iter++) {
      if (proc->threads[thr_iter]->isRunningIRPC()) {
         Frame activeFrame = proc->threads[thr_iter]->getActiveFrame();
         Address retvalAddr = proc->threads[thr_iter]->getIRPCRetValAddr();                
         Address completedAddr =proc->threads[thr_iter]->getIRPCFinishedAddr();
         if (activeFrame.getPC() == retvalAddr) {
            proc->threads[thr_iter]->handleRetValIRPC();
            ateTrap = true;
            handledTrap = true;
            runProcess = true;
         }
         else if (activeFrame.getPC() == completedAddr) {
            // handleCompleted returns true if the process
            // should be run, false otherwise
            if (proc->threads[thr_iter]->handleCompletedIRPC())
               runProcess = true;
            ateTrap = true;
            handledTrap = true;
         }
      }
      if (ateTrap) break;
   }
   if (!handledTrap) {
      // Trap signal was not handled, check to see if it's due to 
      // a system call exiting
      dyn_thread *comp_thr = proc->checkSyscallExit();
      if (comp_thr) {
         // A thread hit a system call... so launch an RPC
         if (comp_thr->isWaitingForTrap()) {
            irpcLaunchState_t thr_state;
            thr_state = comp_thr->launchPendingIRPC();
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


