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

#include "dyninstAPI/src/dyn_thread.h"
#include "dyninstAPI/src/dyn_lwp.h"

/*
// Useful for debugging.  Keeps a history of last VT_MAXRECS of virtual
// timers.
#define VT_MAXRECS 20
#define VT_NUMPOS 4

typedef struct {
   rawTime64 daemon_now;
   virtualTimer timerVal;
   unsigned daemon_lwp;
} rec_t;

rec_t lastVT[VT_NUMPOS][VT_MAXRECS];
int vt_index[VT_NUMPOS] = { -1, -1, -1, -1 };

void vt_record(unsigned pos, virtualTimer *vt, rawTime64 now, unsigned dlwp) {
   assert(pos < VT_NUMPOS);
   int index, circ_index;
   index = ++vt_index[pos];
   circ_index = index % VT_MAXRECS;

   rec_t *rec = &lastVT[pos][circ_index];
   rec->timerVal = *vt;
   rec->daemon_now = now;
   rec->daemon_lwp = dlwp;
}

void vt_showTraceB(int pos) {
   int index = vt_index[pos];   
   int rctr = 1;
   fprintf(stderr,"  ----- showTrace, pos = %d  ---------------------\n", pos);
   int rnum;
   for(rnum = index % VT_MAXRECS; rnum >= 0; rnum--, rctr++) {
      rec_t *rec = &lastVT[pos][rnum];
      virtualTimer *tm = &rec->timerVal;
      fprintf(stderr, ", tot: %lld, start: %lld, ctr: %d, rt_prev: %lld, "
              "rt_lwp: %d, now: %lld, dmn_lwp: %d\n", tm->total, tm->start, 
              tm->counter, tm->rt_previous, tm->lwp, rec->daemon_now,
              rec->daemon_lwp);
   }

   if(index > VT_MAXRECS) {
      int circ_index = index % VT_MAXRECS;
      for(rnum = VT_MAXRECS-1; rnum>circ_index; rnum--, rctr++) {
         rec_t *rec = &lastVT[pos][rnum];
         virtualTimer *tm = &rec->timerVal;
         fprintf(stderr, ", tot: %lld, start: %lld, ctr: %d, rt_prev: %lld, "
                 "rt_lwp: %d, now: %lld, dmn_lwp: %d\n", tm->total, tm->start, 
                 tm->counter, tm->rt_previous, tm->lwp, rec->daemon_now,
                 rec->daemon_lwp);
      }
   }
}

void vt_showTrace(char *msg) {
   fprintf(stderr, "======================================================\n");
   fprintf(stderr, "   %s\n", msg);
   int curPos;
   for(curPos=0; curPos<VT_NUMPOS; curPos++) {
      int index = vt_index[curPos];
      if(index == -1)  continue;
      vt_showTraceB(curPos);
   }
   fprintf(stderr,"=======================================================\n");
   fflush(stderr);
}

unsigned pos_junk = 101;
*/

#if defined(MT_THREAD)
rawTime64 dyn_thread::getInferiorVtime(virtualTimer *vTimer,
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
  rawTime64 now = 0;
  if (count > 0) {
     // pos_junk = get_pos();
     now = proc->getRawCpuTime(vTimer->lwp);
     if(now == -1) {// error code
        success = false;
        return 0;
     }
     // pos_junk = 101;
     ret = total + (now - start);    
  } else {
    ret = total ;
  }
  // vt_record(get_pos(), vTimer, now, vTimer->lwp);
  return ret ;
}
#endif //MT_THREAD 

// We have an LWP handle. Make sure it's still the correct
// one by checking its ID against the one in shared memory

#if !defined(BPATCH_LIBRARY)

bool dyn_thread::updateLWP()
{
  // ST case
  if ((!proc->multithread_ready()) || 
      (pos == (unsigned) -1)) {
    lwp = proc->getDefaultLWP();
    return true;
  }
  
  int lwp_id;
  if (lwp) lwp_id = lwp->get_lwp();
  else lwp_id = 0;
  int vt_lwp = proc->shmMetaData->getVirtualTimer(pos)->lwp;

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

// No shared data, so we can't use the above since the reference
// won't link
bool dyn_thread::updateLWP()
{
  return true;
}
#endif
  

#if !defined(MT_THREAD)
// MT version lives in the <os>MT.C files, and can do things like
// get info for threads not currently scheduled on an LWP
Frame dyn_thread::getActiveFrame()
{
  updateLWP();
  Frame lwpFrame = lwp->getActiveFrame();  
  return Frame(lwpFrame.getPC(), lwpFrame.getFP(),
               lwpFrame.getSP(), lwpFrame.getPID(),
               this, lwpFrame.getLWP(),
               true);
  
  return lwp->getActiveFrame();
}
#endif

// stackWalk: return parameter.
bool dyn_thread::walkStack(pdvector<Frame> &stackWalk)
{
    // If we're in an inferior RPC, return the stack walk
    // from where the process "should" be
    if (isRunningIRPC()) {
        stackWalk = cachedStackWalk;
        return true;
    }
    
    // We cheat (a bit): this method is here for transparency, 
    // but the process class does the work in the walkStackFromFrame
    // method. We get the active frame and hand off.
    Frame active = getActiveFrame();
    
    return proc->walkStackFromFrame(active, stackWalk);
}

dyn_lwp *dyn_thread::get_lwp()
{
  if (proc->multithread_ready())
    updateLWP();
  return lwp;
}

void dyn_thread::postIRPC(inferiorRPCtoDo todo)
{
    thrRPCsWaitingToStart += todo;
}

bool dyn_thread::readyIRPC() const
{
    // If we're running an RPC, we're not ready to start one
    if (isRunningIRPC()) return false;

    return !thrRPCsWaitingToStart.empty();
}

// wasRunning: once the RPC is finished, leave process paused (false) or running (true)
irpcLaunchState_t dyn_thread::launchThreadIRPC(bool wasRunning)
{
    cerr << "Launching thr-specific RPC" << endl;
    if (!readyIRPC()) {
        cerr << "No thread IRPC to run" << endl;
        return irpcNoIRPC;
    }
    
    inferiorRPCtoDo todo = thrRPCsWaitingToStart[0];
    
    // There is an RPC to run. Yay.
    // We pause at a process level
    assert (proc->status() == stopped);

    // Check if we're in a system call
    updateLWP();
    if (lwp->executingSystemCall()) {
        // Oh, crud. Can't run the iRPC now. Check to see if we'll
        // be able to do it later with any degree of accuracy
        if (proc->set_breakpoint_for_syscall_completion()) {
            // Breakpoints are set at a process level still
            irpcState_ = irpcWaitingForTrap;
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
    if (proc->rpcSavesRegs()) {
        theSavedRegs = lwp->getRegisters();
        if (theSavedRegs == (struct dyn_saved_regs *)-1) {
            // Should only happen if we're in a syscall, which is 
            // caught above
            return irpcError;
        }
    }

    thrRPCsWaitingToStart.removeOne();
    
    // Build the in progress struct
    thrCurrRunningRPC.thr = this;
    thrCurrRunningRPC.lwp = lwp;
    
    // Copy over state
    thrCurrRunningRPC.callbackFunc = todo.callbackFunc;
    thrCurrRunningRPC.userData = todo.userData;
    thrCurrRunningRPC.savedRegs = theSavedRegs;
    thrCurrRunningRPC.seq_num = todo.seq_num;
    
    thrCurrRunningRPC.wasRunning = wasRunning;
    
    Address RPCImage = proc->createRPCImage(todo.action, // AST tree
                                            todo.noCost,
                                            (thrCurrRunningRPC.callbackFunc != NULL), // Callback?
                                            thrCurrRunningRPC.breakAddr, // Fills in 
                                            thrCurrRunningRPC.stopForResultAddr,
                                            thrCurrRunningRPC.justAfter_stopForResultAddr,
                                            thrCurrRunningRPC.resultRegister,
                                            todo.lowmem, // Where to allocate
                                            this, // Thread
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
    if (!walkStack(cachedStackWalk)) {
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

irpcLaunchState_t dyn_thread::launchPendingIRPC() {
    return launchThreadIRPC(wasRunningBeforeSyscall_);
}

bool dyn_thread::isRunningIRPC() const {
    return (irpcState_ == irpcRunning ||
            irpcState_ == irpcWaitingForTrap);
}

Address dyn_thread::getIRPCRetValAddr()
{
    if (irpcState_ != irpcRunning) return 0;
    return thrCurrRunningRPC.stopForResultAddr;
}

// Called when an inferior RPC reaches the "grab return value" stage
bool dyn_thread::handleRetValIRPC() {
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

Address dyn_thread::getIRPCFinishedAddr()
{
    if (irpcState_ != irpcRunning) return 0;
    return thrCurrRunningRPC.breakAddr;
}

// False: leave process paused
// True: run the process
bool dyn_thread::handleCompletedIRPC() {
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
        irpcLaunchState_t launchState = launchThreadIRPC(thrCurrRunningRPC.wasRunning);
        if (launchState == irpcStarted) {
            // Run the process
            return true;
        }
    }

    return (thrCurrRunningRPC.wasRunning);
}

            


  
