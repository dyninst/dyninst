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




#ifndef __RPCMGR__
#define __RPCMGR__


#include "common/h/vectorSet.h"
#include "dyninstAPI/src/inferiorRPC.h"
#include "common/h/Dictionary.h"

class process;
class dyn_lwp;

class rpcLwp {
   dyn_lwp *lwp;
   vectorSet<inferiorRPCtoDo> thrRPCsWaitingToStart;
   inferiorRPCinProgress thrCurrRunningRPC;
   irpcState_t irpcState_;
   bool wasRunningBeforeSyscall_;

 public:

   rpcLwp(dyn_lwp *lwp_) : lwp(lwp_), irpcState_(irpcNotValid)  { }
   
   dyn_lwp *get_lwp() { return lwp; }

  // Add an iRPC to the list of work to do
  void postIRPC(inferiorRPCtoDo todo);
  // Returns true iff
  // 1) There is an inferior RPC to run
  // 2) We're not currently running an inferior RPC
  // 3) We're not waiting for a syscall trap
  bool readyIRPC() const;
  // Returns true iff
  // 1) An RPC is running, or
  // 2) We're waiting for a trap to be reached
  bool isRunningIRPC() const;
  // Returns true if we're waiting for a trap
  bool isWaitingForTrap() const;
  // Launch an iRPC.
  irpcLaunchState_t launchThreadIRPC(bool wasRunning);
  // After a syscall completes, launch an RPC. Special case
  // of launchThreadIRPC
  irpcLaunchState_t launchPendingIRPC();

  // Clear/query whether we're waiting for a trap (for signal handling)
  bool isIRPCwaitingForSyscall() { return irpcState_ == irpcWaitingForTrap; }

  // Handle completing IRPCs
  Address getIRPCRetValAddr();
  bool handleRetValIRPC();
  Address getIRPCFinishedAddr();
  bool handleCompletedIRPC();

  irpcState_t getLastIRPCState() { return irpcState_; }
};

static inline unsigned rpcLwpHash(dyn_lwp * const &lwp_addr)
{
  // assume all addresses are 4-byte aligned
  unsigned result = (unsigned)(Address)lwp_addr;
  result >>= 2;
  return result;
  // how about %'ing by a huge prime number?  Nah, x % y == x when x < y 
  // so we don't want the number to be huge.
}

class rpcMgr {
   process *proc;

  // This structure keeps track of an inferiorRPC that we will start sometime
  // in the (presumably near) future.  There's a different structure for RPCs
  // which have been launched and which we're waiting to finish.
  // Don't confuse the two!

  vectorSet<inferiorRPCtoDo> RPCsWaitingToStart;
  irpcState_t irpcState_;

  inferiorRPCinProgress currRunningIRPC;
  bool wasRunningBeforeSyscall_;   

  dictionary_hash<dyn_lwp *, rpcLwp *> rpcLwpBuf;

 private:
   bool distributeRPCsOverLwps();

   // Placeholder function to handle the split between
   // Paradyn and Dyninst IRPC-wise
   bool thr_IRPC();

 public:
   rpcMgr(process *proc_) : proc(proc_), irpcState_(irpcNotValid),
      rpcLwpBuf(rpcLwpHash) { };

   void newLwpFound(dyn_lwp *lwp);
   void deleteLwp(dyn_lwp *lwp);
   
   rpcLwp *createRpcLwp(dyn_lwp *lwp) {
      rpcLwp *rpc_lwp = new rpcLwp(lwp);
      rpcLwpBuf.set(lwp, rpc_lwp);
      return rpc_lwp;
   }

   rpcLwp *findRpcLwp(dyn_lwp *lwp) {
      rpcLwp *rpc_lwp;
      if(rpcLwpBuf.find(lwp, rpc_lwp))
         return rpc_lwp;
      else
         return NULL;
   }

   bool launchRPCs(bool wasRunning);
   bool handleCompletedIRPC();
   Address getIRPCRetValAddr();
   bool handleRetValIRPC();
   Address getIRPCFinishedAddr();
   bool handleTrapIfDueToRPC();
   bool rpcSavesRegs();

   bool isIRPCwaitingForSyscall() const {
      return irpcState_ == irpcWaitingForTrap;
   }

   bool isRunningIRPC() const;
   bool readyIRPC() const;
   bool existsRPCWaitingForSyscall() const;
   bool existsRPCinProgress() const;
   bool existsRPCPending() const;

   void cleanRPCreadyToLaunch(int mid);

   // posting RPC on a process
   void postRPCtoDo(AstNode *action, bool noCost,
                    inferiorRPCcallbackFunc callbackFunc,
                    void *userData, int mid, bool lowmem);

   // posting RPC on a thread
   void postRPCtoDo(AstNode *action, bool noCost,
                    inferiorRPCcallbackFunc callbackFunc,
                    void *userData, int mid, dyn_thread *thr, bool lowmem);

   // posting RPC on a lwp
   void postRPCtoDo(AstNode *action, bool noCost,
                    inferiorRPCcallbackFunc callbackFunc, void *userData,
                    int mid,  dyn_lwp *lwp, bool lowmem);


   Address createRPCImage(AstNode *action, bool noCost,
                          bool shouldStopForResult, Address &breakAddr,
                          Address &stopForResultAddr,
                          Address &justAfter_stopForResultAddr,
                          Register &resultReg, bool lowmem, 
                          dyn_lwp * /*lwp*/, bool isFunclet);

   bool emitInferiorRPCheader(void *, Address &baseBytes, bool isFunclet);

   bool emitInferiorRPCtrailer(void *, Address &baseBytes,
                               unsigned &breakOffset,
                               bool stopForResult,
                               unsigned &stopForResultOffset,
                               unsigned &justAfter_stopForResultOffset,
                               bool isFunclet);

};




#endif


