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
 * excluded
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

// $Id: aix-ptrace.C,v 1.15 2003/12/18 17:15:32 schendel Exp $

#include <pthread.h>
#include "common/h/headers.h"
#include "dyninstAPI/src/os.h"
#include "dyninstAPI/src/signalhandler.h"
#include "dyninstAPI/src/process.h"
#include "dyninstAPI/src/dyn_lwp.h"
#include "dyninstAPI/src/dyn_thread.h"
#include "dyninstAPI/src/symtab.h"
#include "dyninstAPI/src/stats.h"
#include "common/h/Types.h"
#include "common/h/Dictionary.h"
#include "dyninstAPI/src/Object.h"
#include "common/h/pathName.h"
#include "dyninstAPI/src/instPoint.h"
#include "dyninstAPI/src/instP.h"
#include "dyninstAPI/src/inst-power.h" // Tramp constants

#include <procinfo.h>
#include <sys/ptrace.h>

#include <sys/ioctl.h>
#include <sys/types.h>
#include <fcntl.h>
#include <assert.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <scnhdr.h>
#include <sys/time.h>
#include <sys/reg.h>
#include <sys/user.h>
#include <sys/wait.h>
#include <procinfo.h> // struct procsinfo
#include <sys/types.h>
#include <signal.h>

// Things to use for Paradyn only
#ifndef BPATCH_LIBRARY
#include "common/h/Time.h"
#include "common/h/timing.h"
#include "paradynd/src/init.h"
#include "paradynd/src/instReqNode.h"
#endif

#if defined(BPATCH_LIBRARY)
#include "writeBackXCOFF.h"
#endif

#if !defined(BPATCH_LIBRARY)
#ifdef USES_PMAPI
#include <pmapi.h>
#endif
#endif

extern void generateBreakPoint(instruction &);

const int special_register_codenums [] = {IAR, MSR, CR, LR, CTR, XER, MQ, TID, FPSCR};

/* Getprocs() should be defined in procinfo.h, but it's not */
extern "C" {
    extern int getprocs(struct procsinfo *ProcessBuffer,
                        int ProcessSize,
                        struct fdsinfo *FileBuffer,
                        int FileSize,
                        pid_t *IndexPointer,
                        int Count);
    extern int getthrds(pid_t, struct thrdsinfo *, int, tid_t *, int);
}

dyn_lwp *process::createRepresentativeLWP() {
   // don't register the representativeLWP in real_lwps since it's not a true
   // lwp
   representativeLWP = createFictionalLWP(0);
   return representativeLWP;
}

// getActiveFrame(): populate Frame object using toplevel frame
Frame dyn_lwp::getActiveFrame()
{
   unsigned pc, fp;
   if (lwp_id_) { // We have a kernel thread to target. Nifty, eh?
      struct ptsprs spr_contents;
      bool kernel_mode = false;
      if (P_ptrace(PTT_READ_SPRS, lwp_id_, (int *)&spr_contents, 0, 0) == -1) {
         if (errno != EPERM) { // EPERM == thread in kernel mode, not to worry
            perror("----------Error getting IAR in getActiveFrame");
            fprintf(stderr, "dyn_lwp of 0x%x with lwp %d\n", (unsigned) this,
                    lwp_id_);
         }
         else
            kernel_mode = true; // This is going to be... annoying
      }
      pc = spr_contents.pt_iar;
      
      if (!kernel_mode) {
         unsigned allRegs[64];
         // Register 1 is the current stack pointer. It must contain
         // a back chain to the caller's stack pointer.
         // Note: things like the LR are stored in the "parent's" stack frame.
         // This allows leaf functions to store the LR, even without a 
         // stack frame.
         if (P_ptrace(PTT_READ_GPRS, lwp_id_, (int *)allRegs, 0, 0) == -1) {
            perror("Problem reading stack pointer in getActiveFrame");
            return Frame();
         }
         fp = allRegs[1];
      }
      else { // We're in the kernel. Any idea how to get the (old) PC and FP?
         struct thrdsinfo thrd_buf[1000]; // 1000 should be enough for anyone!
         getthrds(proc_->getPid(), thrd_buf, sizeof(struct thrdsinfo), 0,1000);
         unsigned foo = 0;
         while (thrd_buf[foo].ti_tid != lwp_id_) foo++;
         fp = thrd_buf[foo].ti_ustk;
         pc = (unsigned)-1; // ???
      }
      
   }
   else { // Old behavior, pid-based. 
      pc = P_ptrace(PT_READ_GPR, proc_->getPid(), (int *) IAR, 0, 0); // aix 4.1 likes int *
      if (pc == (unsigned) -1) return Frame();
      
      fp = P_ptrace(PT_READ_GPR, proc_->getPid(), (int *) STKP, 0, 0); // aix 4.1 likes int *
      if (fp == (unsigned) -1) return Frame();
      
   }
   return Frame(pc, fp, proc_->getPid(), NULL, this, true);
}


// See sol_proc.C for /proc version
struct dyn_saved_regs *dyn_lwp::getRegisters() {
    struct dyn_saved_regs *regs = new dyn_saved_regs();
    
    if (!lwp_id_) {
        // First, the general-purpose integer registers:
        
        // Format of PT_READ_GPR ptrace call:
        // -- pass the reg number (see <sys/reg.h>) as the 'addr' (3d param)
        // -- last 2 params (4th and 5th) ignored
        // -- returns the value, or -1 on error
        //    but this leaves the question: what if the register really did contain -1; why
        //    should that be an error?  So, we ignore what the man page says here, and
        //    instead look at 'errno'
        // Errors:
        //    EIO --> 3d arg didn't specify a valid register (must be 0-31 or 128-136)
        
        errno = 0;
        for (unsigned i=0; i < 32; i++) {
            unsigned value = P_ptrace(PT_READ_GPR, proc_->getPid(), (void *)i, 0, 0);
            if ((value == (unsigned) -1) && (errno)) {
                perror("ptrace PT_READ_GPR");
                cerr << "regnum was " << i << endl;
            }
            regs->gprs[i] = value;
        }
        
        // Next, the general purpose floating point registers.
        
        // Format of PT_READ_FPR ptrace call: (it differs greatly from PT_READ_GPR,
        // probably because the FPR registers are 64 bits instead of 32)
        // -- 3d param ('address') is the location where ptrace will store the reg's value.
        // -- 4th param ('data') specifies the fp reg (see <sys/reg.h>)
        // -- last param (5th) to ptrace ignored
        // Errors: returns -1 on error
        //    EIO --> 4th arg didn't specify a valid fp register (must be 256-287)
        // Note: don't ask me why, but apparantly a return value of -1 doesn't seem
        //       to properly indicate an error; check errno instead.
        
        for (unsigned i=0; i < 32; i++) {
            double value;
            assert(sizeof(double)==8); // make sure it's big enough!
            
            if ((P_ptrace(PT_READ_FPR, proc_->getPid(), &value,
                          FPR0 + i, // see <sys/reg.h>
                          0) == -1) && errno) {
                perror("ptrace PT_READ_FPR");
                cerr << "regnum was " << FPR0 + i << "; FPR0=" << FPR0 << endl;
            }
            regs->fprs[i] = value;
        }
        
        // Finally, the special registers.
        // (See the notes on PT_READ_GPR above: pass reg # as 3d param, 4th & 5th ignored)
        // (Only reg numbered 0-31 or 128-136 are valid)
        // see <sys/reg.h>; FPINFO and FPSCRX are out of range, so we can't use them!
        
        errno = 0;
        for (unsigned i=0; i < num_special_registers; i++) {
            unsigned value = P_ptrace(PT_READ_GPR, proc_->getPid(), (void *)special_register_codenums[i], 0, 0);
            if ((value == (unsigned) -1) && errno) {
                perror("ptrace PT_READ_GPR for a special register");
                cerr << "regnum was " << special_register_codenums[i] << endl;
            }
            regs->sprs[i] = value;
        }
    }
    
    else {
        errno = 0;
        if((P_ptrace(PTT_READ_GPRS, lwp_id_, (void *)regs->gprs, 0, 0) == -1) && errno) {
            perror("ptrace PTT_READ_GPRS");
            return NULL;
        }
        // Next, the general purpose floating point registers.
        // Again, we read as a block. 
        // ptrace(PTT_READ_FPRS, lwp, &buffer (at least 32*8=256), 0, 0);
        
	errno = 0;
        if((P_ptrace(PTT_READ_FPRS, lwp_id_, (void *)regs->fprs, 0, 0) == -1) && errno) {
            perror("ptrace PTT_READ_FPRS");
            return NULL;
        }
        
        // We get the SPRs in a special structure. We'll then copy to
        // our buffer for later retrieval. We could save the lot, I suppose,
        // but there's a _lot_ of extra space that is unused.
        struct ptsprs spr_contents;
        
        errno = 0;
        if((P_ptrace(PTT_READ_SPRS, lwp_id_, (void *)&spr_contents, 0, 0) == -1) && errno) {
            perror("PTT_READ_SPRS");
            return NULL;
        }
        // Now we save everything out. List: IAR, MSR, CR, LR, CTR, XER, MQ, TID, FPSCR
        regs->sprs[0] = spr_contents.pt_iar;
        regs->sprs[1] = spr_contents.pt_msr;
        regs->sprs[2] = spr_contents.pt_cr;
        regs->sprs[3] = spr_contents.pt_lr;
        regs->sprs[4] = spr_contents.pt_ctr;
        regs->sprs[5] = spr_contents.pt_xer;
        regs->sprs[6] = spr_contents.pt_mq;
        regs->sprs[7] = spr_contents.pt_reserved_0; // Match for TID, whatever that is
        regs->sprs[8] = spr_contents.pt_fpscr;
    }
    return regs;
}



bool dyn_lwp::restoreRegisters(struct dyn_saved_regs *regs) {
    if (!regs) return false;
    
  if (!lwp_id_) {
    // First, the general-purpose registers:
    // Format for PT_WRITE_GPR:
    // 3d param ('address'): specifies the register (must be 0-31 or 128-136)
    // 4th param ('data'): specifies value to store
    // 5th param ignored.
    // Returns 3d param on success else -1 on error.
    // Errors:
    //    EIO: address must be 0-31 or 128-136
      // How's this for fun: ptrace returns the value the register is set to. If this 
      // happens to be -1, we interpret it as failure (because -1 means failure). In
      // this case, check the value of errno. But errno gets set on failure, not 
      // success... so we need to manually reset it.

    errno = 0;
    for (unsigned i=0; i < 32; i++) {
        if (P_ptrace(PT_WRITE_GPR, proc_->getPid(), (int *)i, regs->gprs[i], 0) == -1) {
            if (errno) {
                perror("restoreRegisters PT_WRITE_GPR");
                return false;
            }
        }
    } 
    
    // Next, the floating-point registers:
    // Format of PT_WRITE_FPR: (it differs from PT_WRITE_GPR, probably because
    // FP registers are 8 bytes instead of 4)
    // 3d param ('address'): address of the value to store
    // 4th param ('data'): reg num (256-287)
    // 5th param ignored
    // returns -1 on error
    // Errors:
    //    EIO: reg num must be 256-287
  
    for (unsigned i=FPR0; i <= FPR31; i++) {
        
        if (P_ptrace(PT_WRITE_FPR, proc_->getPid(),
                     (void *)&(regs->fprs[i-FPR0]), i, 0) == -1) {
            if (errno) {
                perror("ptrace PT_WRITE_FPR");
                cerr << "regnum was " << i << endl;
                return false;
            }
        }
    } 
    
    // Finally, special registers:
    
    for (unsigned i=0; i < num_special_registers; i++) {
      if (P_ptrace(PT_WRITE_GPR, proc_->getPid(), 
                   (void *)(special_register_codenums[i]), 
                   regs->sprs[i], 0) == -1) {
          if (errno) {
              perror("ptrace PT_WRITE_GPR for a special register");
              cerr << "regnum was " << special_register_codenums[i] << endl;
              return false;
          }
      }
    }
  }
  else {
      if (P_ptrace(PTT_WRITE_GPRS, lwp_id_, (void *)regs->gprs, 0, 0) == -1) {
          perror("ptrace PTT_WRITE_GPRS");
          return false;
      }
      // Next, the general purpose floating point registers.
      // ptrace(PTT_WRITE_FPRS, lwp, &buffer (at least 32*8=256), 0, 0);
      if (P_ptrace(PTT_WRITE_FPRS, lwp_id_, (void *)regs->fprs, 0, 0) == -1) {
          perror("ptrace PTT_WRITE_FPRS");
          return false;
      }
      
      // We get the SPRs in a special structure. We'll then copy to
      // our buffer for later retrieval. We could save the lot, I suppose,
      // but there's a _lot_ of extra space that is unused.
      struct ptsprs saved_sprs;
      struct ptsprs current_sprs;
      // Restore. List: IAR, MSR, CR, LR, CTR, XER, MQ, TID, FPSCR
      ptrace(PTT_READ_SPRS, lwp_id_, (int *)&current_sprs, 0, 0);
      
      saved_sprs.pt_iar = regs->sprs[0];
      saved_sprs.pt_msr = regs->sprs[1];
      saved_sprs.pt_cr = regs->sprs[2];
      saved_sprs.pt_lr = regs->sprs[3];
      saved_sprs.pt_ctr = regs->sprs[4];
      saved_sprs.pt_xer = regs->sprs[5];
      saved_sprs.pt_mq = regs->sprs[6];
      saved_sprs.pt_reserved_0 = regs->sprs[7];
      saved_sprs.pt_fpscr = regs->sprs[8];
      
      if (P_ptrace(PTT_WRITE_SPRS, lwp_id_, (void *)&saved_sprs, 0, 0) == -1) {
          perror("PTT_WRITE_SPRS");
          return false;
      }
  }
  return true;
}

bool dyn_lwp::changePC(Address loc, struct dyn_saved_regs *)
{
   if (!lwp_id_) {
      if ( !P_ptrace(PT_READ_GPR, proc_->getPid(), (void *)IAR, 0, 0)) {
         perror("changePC (PT_READ_GPR) failed");
         return false;
      }
      
      
      if (P_ptrace(PT_WRITE_GPR, proc_->getPid(), (void *)IAR, loc, 0) == -1) {
         perror("changePC (PT_WRITE_GPR) failed");
         return false;
      }
      
      // Double-check that the change was made by reading the IAR register
      if (P_ptrace(PT_READ_GPR, proc_->getPid(), (void *)IAR, 0,0) != (int)loc)
      {
         perror("changePC (verify) failed");
         return false;
      }
   }
   else {
      struct ptsprs spr_contents;
      if (P_ptrace(PTT_READ_SPRS, lwp_id_, (void *)&spr_contents, 0,0) == -1) {
         perror("changePC: PTT_READ_SPRS failed");
         return false;
      }
      spr_contents.pt_iar = loc;
      // Write the registers back in
      
      if (P_ptrace(PTT_WRITE_SPRS, lwp_id_, (void *)&spr_contents, 0, 0) == -1)
      {
         perror("changePC: PTT_WRITE_SPRS failed");
         return false;
      }
      // Double check...
      struct ptsprs spr_doublecheck;

      if (P_ptrace(PTT_READ_SPRS, lwp_id_, (void *)&spr_doublecheck, 0,0) == -1) {
         perror("changePC: PTT_READ_SPRS failed");
         return false;
      }
      if (spr_doublecheck.pt_iar != loc) {
          fprintf(stderr, "Doublecheck failed! PC at 0x%x instead of 0x%x!\n",
                  spr_doublecheck.pt_iar, loc);
      }
      
   }
   return true;
}


unsigned recognize_thread(process *proc, unsigned lwp_id) {
   dyn_lwp *lwp = proc->getLWP(lwp_id);

   pdvector<AstNode *> ast_args;
   AstNode *ast = new AstNode("DYNINSTregister_running_thread", ast_args);

   return proc->getRpcMgr()->postRPCtoDo(ast, true, NULL, (void *)lwp_id,
                                         true, NULL, lwp);
}

unsigned get_childproc_lwp(process *proc) {
   pdvector<unsigned> lwpid_buf;
   tid_t indexPtr = 0;   
   struct thrdsinfo thrd_info;
   int found_lwp = -1;
   int num_found = 0;
   do {
      num_found = getthrds(proc->getPid(), &thrd_info,
                           sizeof(struct thrdsinfo), &indexPtr, 1);
      //cerr << "called getthrds, ret: " << num_found << ", indexPtr: "
      //     << indexPtr << ", tid; " << thrd_info.ti_tid << endl;
      if(found_lwp == -1)
         found_lwp = thrd_info.ti_tid;
      else if(num_found > 0) {
         // this warning shouldn't occur because on pthreads only the thread
         // which initiated the fork should be duplicated in the child process
         cerr << "warning, multiple threads found in child process when "
              << "only one thread is expected\n";
      }
   } while(num_found > 0);

   return static_cast<unsigned>(found_lwp);
}


// run rpcs on each lwp in the child process that will start the virtual
// timer of each thread and cause the rtinst library to notify the daemon of
// a new thread.  For pthreads though, which AIX uses, there should only be
// one thread in the child process (ie. the thread which initiated the fork).

void process::recognize_threads(pdvector<unsigned> *completed_lwps) {
   unsigned found_lwp = get_childproc_lwp(this);
   //cerr << "chosen lwp = " << found_lwp << endl;

   unsigned rpc_id = recognize_thread(this, found_lwp);
   bool cancelled = false;

   do {
       getRpcMgr()->launchRPCs(false);
       if(hasExited())
           return;
       decodeAndHandleProcessEvent(false);
       
       irpcState_t rpc_state = getRpcMgr()->getRPCState(rpc_id);
       if(rpc_state == irpcWaitingForSignal) {
           //cerr << "rpc_id: " << rpc_id << " is in syscall, cancelling rpc\n";
           getRpcMgr()->cancelRPC(rpc_id);
           cancelled = true;
           break;
       }
   } while(getRpcMgr()->getRPCState(rpc_id) != irpcNotValid); // Loop rpc is done
   
   if(! cancelled) {
      (*completed_lwps).push_back(found_lwp);
   }
}



static bool executeDummyTrap(process *theProc) {
   assert(theProc->status() == stopped);
   
   // Allocate a tempTramp. Assume there is text heap space available,
   // since otherwise we're up a creek.
   unsigned tempTramp = theProc->inferiorMalloc(8, textHeap);
   assert(tempTramp);

   unsigned theInsns[2];
   theInsns[0] = BREAK_POINT_INSN;
   theInsns[1] = 0; // illegal insn, just to make sure we never exec past the trap
   if (!theProc->writeTextSpace((void *)tempTramp, sizeof(theInsns), &theInsns)) {
      cerr << "executeDummyTrap failed because writeTextSpace failed" << endl;
      return false;
   }

   
   unsigned oldpc = P_ptrace(PT_READ_GPR, theProc->getPid(), (void *)IAR, 0, 0);
   if (oldpc == (unsigned) -1) return false;
   
   if (P_ptrace(PT_WRITE_GPR, theProc->getPid(), (void *)IAR, tempTramp, 0) == -1)
     return false;

   if ((unsigned)P_ptrace(PT_READ_GPR, theProc->getPid(), (void *)IAR, 0, 0) != tempTramp) {
      cerr << "executeDummyTrap failed because PT_READ_GPR of IAR register failed" << endl;
      return false;
   }

   // We bypass continueProc() because continueProc() changes theProc->status_, which
   // we don't want to do here
   
   P_ptrace(PT_CONTINUE, theProc->getPid(), (void *)1, 0, 0);
      // what if there are any pending signals?  Don't we lose the chance to forward
      // them now?
   assert(errno == 0);

   // Restore the old PC register value
   
   P_ptrace(PT_WRITE_GPR, theProc->getPid(), (void *)IAR, oldpc, 0);
   assert(errno == 0);

   // delete the temp tramp now (not yet implemented)

#ifdef INFERIOR_RPC_DEBUG
   cerr << "leaving executeDummyTrap now" << endl;
   cerr.flush();
#endif

   return true;
}


bool dyn_lwp::executingSystemCall() 
{
  // lwp -- we may care about a particular thread.
  errno = 0;
  int retCode = 0;
  if (lwp_id_) {
    // Easiest way to check: try to read GPRs and see
    // if we get EPERM back
      struct ptsprs spr_contents;
      P_ptrace(PTT_READ_SPRS, lwp_id_, (int *)&spr_contents, 0, 0); // aix 4.1 likes int *
  }
  else {
      // aix 4.1 likes int *
      retCode = P_ptrace(PT_READ_GPR, proc_->getPid(), (int *) IAR, 0, 0); 
  }
  
  if (errno == EPERM) {
      return true;
  }
  
  return false;
}

bool attach_helper(process *proc) {
   // formerly OS::osAttach()    

   int ret = ptrace(PT_ATTACH, proc->getPid(), (int *)0, 0, 0);
   if (ret == -1)
      ret = ptrace(PT_REATT, proc->getPid(), (int *)0, 0, 0);

   return (ret != -1);
}

bool dyn_lwp::deliverPtrace(int request, void *addr, int data, void *addr2) {
   bool needToCont = false;
   bool ret;
  
   if(request != PT_DETACH && status() == running) {
      if(pauseLWP() == true)
         needToCont = true;
   }

   // aix 4.1 likes int *
   if (ptrace(request, getPid(), (int *)addr, data, (int *)addr2) == -1) 
      ret = false;
   else
      ret = true;

   if(request != PT_DETACH && needToCont==true)
      continueLWP();

   return ret;
}


bool dyn_lwp::stop_() {
   assert(this == proc_->getRepresentativeLWP());
   /* Choose either one of the following methods for stopping a process, 
    * but not both. 
    * The choice must be consistent with that in process::continueProc_ 
    * and ptraceKludge::continueProcess
    */
	return (P_kill(getPid(), SIGSTOP) != -1); 
}

bool process::continueWithForwardSignal(int sig) {
#if defined(PTRACE_ATTACH_DETACH)
  if (sig != 0) {
      ptrace(PT_DETACH, pid, (int*)1, stat, 0);
      return (true);
  } else {
      return (ptrace(PT_CONTINUE, pid, (int*)1, 0, 0) != -1);
  }
#else
  return (ptrace(PT_CONTINUE, pid, (int*)1, sig, NULL) != -1);
#endif
}

void OS::osTraceMe(void)
{
  int ret;
  ret = ptrace(PT_TRACE_ME, 0, 0, 0, 0);
  assert(ret != -1);
}

bool process::isRunning_() const {
   // determine if a process is running by doing low-level system checks, as
   // opposed to checking the 'status_' member vrble.  May assume that attach()
   // has run, but can't assume anything else.

  // Here's the plan: use getprocs() to get process info.
  // Constant for the number of processes wanted in info
  const unsigned int numProcsWanted = 1;
  struct procsinfo procInfoBuf[numProcsWanted];
  struct fdsinfo fdsInfoBuf[numProcsWanted];
  int numProcsReturned;
  // The pid sent to getProcs() is modified, so make a copy
  pid_t wantedPid = pid; 
  // We really don't need to recalculate the size of the structures
  // every call through here. The compiler should optimize these
  // to constants.
  const int sizeProcInfo = sizeof(struct procsinfo);
  const int sizeFdsInfo = sizeof(struct fdsinfo);

  numProcsReturned = getprocs(procInfoBuf,
			      sizeProcInfo,
			      fdsInfoBuf,
			      sizeFdsInfo,
			      &wantedPid,
			      numProcsWanted);

  if (numProcsReturned == -1) // We have an error
    perror("Failure in isRunning_");

  // This is list of possible values for pi_state (from sys/proc.h)
  //#define SNONE           0               /* slot is available    */
  //#define SIDL            4               /* process is created   */
  //#define SZOMB           5               /* process is dying     */ 
  //#define SSTOP           6               /* process is stopped   */
  //#define SACTIVE         7               /* process is active    */
  //#define SSWAP           8               /* process is swapped   */
  // We use SACTIVE

  return (procInfoBuf[0].pi_state == SACTIVE);
}

// TODO is this safe here ?
bool dyn_lwp::continueLWP_() {
   // has to be representative LWP
   assert(this == proc_->getRepresentativeLWP());

   // we don't want to operate on the process in this state
   ptraceOps++; 
   ptraceOtherOps++;

   bool ret;
   // aix 4.1 likes int *
   if( ptrace(PT_CONTINUE, getPid(), (int *)1, 0, (int *)NULL) == -1)
      ret = false;
   else
      ret = true;

   return ret;
}

bool process::terminateProc_()
{
  if (!checkStatus())
    return false;

  if (P_ptrace(PT_KILL, pid, NULL, 0, NULL) != 0) {
    // For some unknown reason, the above ptrace sometimes fails with a "no
    // such process" error, even when there is such a process and the process
    // is runnable.  So, if the above fails, we try killing it another way.
    if (kill(pid, SIGKILL) != 0)
    	return false;
  }

  return true;
}

void dyn_lwp::realLWP_detach_() {
   assert(is_attached());  // dyn_lwp::detach() shouldn't call us otherwise
}

void dyn_lwp::representativeLWP_detach_() {
   assert(is_attached());  // dyn_lwp::detach() shouldn't call us otherwise
   if(! proc_->checkStatus());

   ptraceOps++; ptraceOtherOps++;
   if(! deliverPtrace(PT_DETACH, (char*)1, SIGSTOP, NULL)) {
      sprintf(errorLine, "Unable to detach %d\n", getPid());
      logLine(errorLine);
      showErrorCallback(40, (const char *) errorLine);
   }

   // always return true since we report the error condition.
   return;
}

bool process::API_detach_(const bool cont) {
  if (!checkStatus())
      return false;

  ptraceOps++; ptraceOtherOps++;
  return getRepresentativeLWP()->deliverPtrace(PT_DETACH,(char*)1,
                                               cont ? 0 : SIGSTOP, NULL);
}

// temporarily unimplemented, PT_DUMPCORE is specific to sunos4.1
bool process::dumpCore_(const pdstring coreFile) {
  if (!checkStatus()) 
    return false;
  ptraceOps++; ptraceOtherOps++;

  if (!dumpImage(coreFile)) {
     assert(false);
  }

  (void) ptrace(PT_CONTINUE, pid, (int*)1, SIGBUS, NULL);
  return true;
}

bool dyn_lwp::writeTextWord(caddr_t inTraced, int data) {
   ptraceBytes += sizeof(int); ptraceOps++;
   return deliverPtrace(PT_WRITE_I, inTraced, data, NULL);
}

bool dyn_lwp::writeTextSpace(void *inTraced, u_int amount, const void *inSelf)
{
  return writeDataSpace(inTraced, amount, inSelf);
}

bool dyn_lwp::readTextSpace(void *inTraced, u_int amount, const void *inSelf) {
  return readDataSpace(inTraced, amount, const_cast<void *>(inSelf));
}

/* Note:
 * Writes are not forced to be synchronous with instruction fetch on the
 * PowerPC architecture. So this means that if we write to memory and then
 * try to execute those instructions, our writes may not have propagated
 * through. I've seen this happen with inferiorRPCs, and occasionally with
 * base/mini tramps, though less often. So we're going to explicitly
 * flush. Ref: PowerPC architecture, "PowerPC virtual environment arch",
 * p344
 *
 * Update: there is an OS fix for this.
 */
bool dyn_lwp::writeDataSpace(void *inTraced, u_int amount, const void *inSelf)
{
   ptraceBytes += amount;
  
   while (amount > 1024) {
      ptraceOps++;
      if(! deliverPtrace(PT_WRITE_BLOCK, inTraced, 1024,
                         const_cast<void*>(inSelf)))
      {
         perror("Failed write");
         return false;
      }
      amount -= 1024;
      inTraced = (char *)inTraced + 1024;
      inSelf = (char *)const_cast<void*>(inSelf) + 1024;
   }

   ptraceOps++;

   if(! deliverPtrace(PT_WRITE_BLOCK, inTraced, amount,
                      const_cast<void*>(inSelf)))
   {
      fprintf(stderr, "Write of %d bytes from 0x%x to 0x%x\n",
              amount, inSelf, inTraced);      
      perror("Failed write2");
      while(1);
      
    return false;
   }
   return true;
}

bool dyn_lwp::readDataSpace(const void *inTraced, u_int amount, void *inSelf)
{
   ptraceBytes += amount;

   while (amount > 1024) {
      ptraceOps++;

      if(!deliverPtrace(PT_READ_BLOCK,const_cast<void*>(inTraced),
                        1024, inSelf))
         return false;

      amount -= 1024;
      inTraced = (const char *)inTraced + 1024;
      inSelf = (char *)inSelf + 1024;
   }

   ptraceOps++;
   return deliverPtrace(PT_READ_BLOCK, const_cast<void*>(inTraced),
                        amount, inSelf);
}

// Can this be unified with linux' version?
// Maybe, but it can be unified with decodeProcessEvent
// if we add a "pid" parameter to decodeProcessEvent. 
bool dyn_lwp::waitUntilStopped() {
  /* make sure the process is stopped in the eyes of ptrace */
   bool isStopped = false;
   int waitStatus;
   int loops = 0;
   while (!isStopped) {
      if(proc_->hasExited()) return false;
      if (loops == 2000) {
         // Resend sigstop...
         if(proc_->multithread_capable()) {
            // We see the process stopped, but we think it is running
            // Check to see if the process is stopped, and if so set status
            struct procsinfo procInfoBuf;
            const int procsinfoSize = sizeof(struct procsinfo);
            struct fdsinfo fdsInfoBuf;
            const int fdsinfoSize = sizeof(struct fdsinfo);
            int pidVar = getPid();
            int numProcs = getprocs(&procInfoBuf,
                                    procsinfoSize,
                                    &fdsInfoBuf,
                                    fdsinfoSize,
                                    &pidVar,
                                    1);
            if (numProcs == 1) {
               if (procInfoBuf.pi_state == SSTOP) {
                  proc_->set_status(stopped);
                  return true;
               }
            }
         }
         stop_();
         loops = 0;
      }
      
      procSignalWhy_t why;
      procSignalWhat_t what;
      procSignalInfo_t info;
      dyn_lwp *selectedLWP;
      process *proc = decodeProcessEvent(&selectedLWP, getPid(), why, what,
                                         info, false);
      assert(proc == NULL || proc == proc_);
      
      if (proc == NULL) {
         loops++;
         usleep(10);
         continue;
      }
      
      if (didProcReceiveSignal(why) && what == SIGSTOP) {
         isStopped = true;
      }
      else {
         handleProcessEvent(proc_, selectedLWP, why, what, info);
         // if handleProcessEvent left the proc stopped, continue
         // it so we that we will get the SIGSTOP signal we sent the proc
         if(proc_->status() == stopped) {
            proc_->continueProc();
         }
         loops = 0;
      }
   }
   return true;
}


// Can this be unified with linux' version?
// Maybe, but it can be unified with decodeProcessEvent
// if we add a "pid" parameter to decodeProcessEvent. 
#ifdef notdef
bool process::loopUntilStopped() {
  /* make sure the process is stopped in the eyes of ptrace */
   if (hasExited()) {
      return false;
   }
   
   getRepresentativeLWP()->stop_();     //Send the process a SIGSTOP

   bool isStopped = false;
   int waitStatus;
   int loops = 0;
   while (!isStopped) {
        if(hasExited()) return false;
        if (loops == 2000) {
            // Resend sigstop...
           if(multithread_capable()) {
              // We see the process stopped, but we think it is running
              // Check to see if the process is stopped, and if so set status
              struct procsinfo procInfoBuf;
              const int procsinfoSize = sizeof(struct procsinfo);
              struct fdsinfo fdsInfoBuf;
              const int fdsinfoSize = sizeof(struct fdsinfo);
              int pidVar = getPid();
              int numProcs = getprocs(&procInfoBuf,
                                      procsinfoSize,
                                      &fdsInfoBuf,
                                      fdsinfoSize,
                                      &pidVar,
                                      1);
              if (numProcs == 1) {
                 if (procInfoBuf.pi_state == SSTOP) {
                    set_status(stopped);
                    return true;
                 }
              }
           }
           getRepresentativeLWP()->stop_();
           loops = 0;
        }

        procSignalWhy_t why;
        procSignalWhat_t what;
        procSignalInfo_t info;
        dyn_lwp *selectedLWP;
        process *proc = decodeProcessEvent(&selectedLWP, pid, why, what, info,
                                           false);
        assert(proc == NULL ||
               proc == this);

        if (proc == NULL) {
            loops++;
            usleep(10);
            continue;
        }
        
        if (didProcReceiveSignal(why) &&
            what == SIGSTOP) {
            isStopped = true;
        }
        else {
            handleProcessEvent(this, selectedLWP, why, what, info);
            // if handleProcessEvent left the proc stopped, continue
            // it so we that we will get the SIGSTOP signal we sent the proc
            if(status() == stopped) {
                continueProc();
            }
            loops = 0;
        }
    }
    return true;
}
#endif

//
// Write out the current contents of the text segment to disk.  This is useful
//    for debugging dyninst.
//
bool process::dumpImage(pdstring outFile) {
    // formerly OS::osDumpImage()
    const pdstring &imageFileName = symbols->file();
    // const Address codeOff = symbols->codeOffset();
    int i;
    int rd;
    int ifd;
    int ofd;
    int cnt;
    int ret;
    int total;
    int length;
    Address baseAddr;
#if !defined(_THREAD_SAFE) || !defined(_THREAD_SAFE_ERRNO)
    // extern int errno;
#endif
    char buffer[4096];
    struct filehdr hdr;
    struct stat statBuf;
    struct aouthdr aout;
    struct scnhdr *sectHdr;
    bool needsCont = false;
    struct ld_info info[64];
#ifdef notdef
    // Added 19JAN01, various debug items
    // Get the kernel thread ID for the currently running
    // process
    struct thrdsinfo thrd_buf[10]; // max 10 threads
    int num_thrds = getthrds(pid, thrd_buf, sizeof(struct thrdsinfo),
			     0, 1);
    if (num_thrds == -1) {
        fprintf(stderr, "%d ", errno);
      perror("getting TID");
    }
    else
      fprintf(stderr, "%d threads currently running\n", num_thrds);
    int kernel_thread = thrd_buf[0].ti_tid;
    int gpr_contents[32];
    ptrace(PTT_READ_GPRS, kernel_thread,
	   gpr_contents, 0, 0);
    fprintf(stderr, "Register contents:\n");
    for (i = 0; i < 32; i++)
      fprintf(stderr, "%d: %x\n", i, gpr_contents[i]);
    
    struct ptsprs spr_contents;
    ptrace(PTT_READ_SPRS, kernel_thread,
	   (int *)&spr_contents, 0, 0);
    
    fprintf(stderr, "IAR: %x\n", spr_contents.pt_iar);
    fprintf(stderr, "LR: %x\n", spr_contents.pt_lr);
    fprintf(stderr, "CR: %x\n", spr_contents.pt_cr);
    fprintf(stderr, "CTR: %x\n", spr_contents.pt_ctr);
    fprintf(stderr, "MSR: %x\n", spr_contents.pt_msr);

    // And get and print the chunk of memory around the error
    int memory[32];
    ptrace(PT_READ_BLOCK, pid, (int *)spr_contents.pt_iar-64,
	   4*32, memory);
    for (i = 0; i < 32; i++) {
      fprintf(stderr, "%x ", (spr_contents.pt_iar-64) + i*4);
      decodeInstr(memory[i]);
    }

#endif

    ifd = open(imageFileName.c_str(), O_RDONLY, 0);
    if (ifd < 0) {
      sprintf(errorLine, "Unable to open %s\n", imageFileName.c_str());
      logLine(errorLine);
      showErrorCallback(41, (const char *) errorLine);
      perror("open");
      return true;
    }

    rd = fstat(ifd, &statBuf);
    if (rd != 0) {
      perror("fstat");
      sprintf(errorLine, "Unable to stat %s\n", imageFileName.c_str());
      logLine(errorLine);
      showErrorCallback(72, (const char *) errorLine);
      return true;
    }
    length = statBuf.st_size;
    ofd = open(outFile.c_str(), O_WRONLY|O_CREAT, 0777);
    if (ofd < 0) {
      perror("open");
      exit(-1);
    }

    /* read header and section headers */
    cnt = read(ifd, &hdr, sizeof(struct filehdr));
    if (cnt != sizeof(struct filehdr)) {
	sprintf(errorLine, "Error reading header\n");
	logLine(errorLine);
	showErrorCallback(44, (const char *) errorLine);
	return false;
    }

    cnt = read(ifd, &aout, sizeof(struct aouthdr));

    sectHdr = (struct scnhdr *) calloc(sizeof(struct scnhdr), hdr.f_nscns);
    cnt = read(ifd, sectHdr, sizeof(struct scnhdr) * hdr.f_nscns);
    if ((unsigned) cnt != sizeof(struct scnhdr)* hdr.f_nscns) {
	sprintf(errorLine, "Section headers\n");
	logLine(errorLine);
	return false;
    }

    /* now copy the entire file */
    lseek(ofd, 0, SEEK_SET);
    lseek(ifd, 0, SEEK_SET);
    for (i=0; i < length; i += 4096) {
        rd = read(ifd, buffer, 4096);
        write(ofd, buffer, rd);
        total += rd;
    }

    if (!stopped) {
        // make sure it is stopped.
        process::findProcess(pid)->getRepresentativeLWP()->stop_();

        waitpid(pid, NULL, WUNTRACED);
        needsCont = true;
    }

    ret = ptrace(PT_LDINFO, pid, (int *) &info, sizeof(info), (int *) &info);
    if (ret != 0) {
	statusLine("Unable to get loader info about process");
	showErrorCallback(43, "Unable to get loader info about process");
	return false;
    }

    baseAddr = (unsigned)info[0].ldinfo_textorg + (unsigned)aout.text_start;
    sprintf(errorLine, "seeking to %ld as the offset of the text segment \n",
	aout.text_start);
    logLine(errorLine);
    sprintf(errorLine, "Code offset = 0x%lx\n", baseAddr);
    logLine(errorLine);

    /* seek to the text segment */
    lseek(ofd, aout.text_start, SEEK_SET);
    for (i=0; i < aout.tsize; i+= 1024) {
        length = ((i + 1024) < aout.tsize) ? 1024 : aout.tsize -i;
        if (ptrace(PT_READ_BLOCK, pid, (int*) (baseAddr + i), length, (int *)buffer) == -1) {
	  perror("ptrace");
	  assert(0);
        }
	write(ofd, buffer, length);
    }

    if (needsCont) {
       ptrace(PT_CONTINUE, pid, (int*) 1, SIGCONT, 0);
    }

    close(ofd);
    close(ifd);

    return true;
}

// Returns all information necessary to get and parse the executable
// file.
// We also do some other one-time setup here. It should probably go somewhere
// else.
// Returns something of type fileDescriptor
// Take a file name, since we might not be using member file
fileDescriptor *getExecFileDescriptor(pdstring filename,
				      int &status, 
				      bool waitForTrap)
{
    int ret;
    // Sneak the pid out of the status word, since it's only overwritten
    // anyway
    int pid = status; 
    struct ld_info *ptr;
    // Credit where credit is due: it works in GDB 5.0, so it should
    // work here. Right? Of course it will. NOT!
    // It's impossible to a priori know how many ld_info records there
    // will be for an executable, so we guess n < 1024
    ptr = (struct ld_info *) malloc (1024*sizeof(struct ld_info));

    // wait for the TRAP point.
    if (waitForTrap)
      waitpid(pid, &status, WUNTRACED);

    status = WSTOPSIG(status);

    /* It seems that AIX has some timing problems and
     when the user stack grows, the kernel doesn't update the stack info in time
     and ptrace calls step on user stack. This is the reason why call sleep 
     here, giving the kernel some time to update its internals. */
    // Is this still around? How do you tell?
    usleep (36000);

    ret = ptrace(PT_LDINFO, pid, 
		 (int *) ptr, 1024 * sizeof(struct ld_info), (int *)ptr);

    if (ret != 0) {
      // Two possible problems here -- one, the process didn't exist. We
      // need to handle this. Second, we haven't attached yet. Work around
      if (0) { // Proc doesn't exist
	return NULL;
      }
      else {
	ptr->ldinfo_textorg = (void *) -1; // illegal flag value
	ptr->ldinfo_dataorg = (void *) -1; // illegal flag value
	// attempt to continue and patch up later
      }	
      /*
	perror("failed to get loader information about process");
	statusLine("Unable to get loader info about process, application aborted");
	showErrorCallback(43, "Unable to get loader info about process, application aborted");
	return NULL;
      */
    }

    // Set up and return the file descriptor. In this case we actually
    // return a fileDescriptor_AIX type (text/data org value, pid)
    pdstring member = "";
    Address text_org = (Address) ptr->ldinfo_textorg;
    Address data_org = (Address) ptr->ldinfo_dataorg;

    fileDescriptor *desc = 
      (fileDescriptor *) new fileDescriptor_AIX(filename, member,
						text_org, data_org,
						pid, true);

    return desc;
}

Address dyn_lwp::readRegister(Register returnValReg) 
{
  if (lwp_id_) {
      unsigned allRegs[64];
      P_ptrace(PTT_READ_GPRS, lwp_id_, (int *)allRegs, 0, 0); // aix 4.1 likes int *
      return allRegs[returnValReg];
  }
  else
    return P_ptrace(PT_READ_GPR, proc_->getPid(), (void *)returnValReg, 0, 0);
}

syscallTrap *process::trapSyscallExitInternal(Address syscall) {
    syscallTrap *trappedSyscall = NULL;

    // HACK for catchup only
    if (syscall == 0) return NULL;
    
    for (unsigned iter = 0; iter < syscallTraps_.size(); iter++) {
        if (syscallTraps_[iter]->syscall_id == syscall) {
            trappedSyscall = syscallTraps_[iter];
            break;
        }
    }
    if (trappedSyscall) {
        trappedSyscall->refcount++;
        return trappedSyscall;
    }
    else {
        // Add a trap at this address, please
        trappedSyscall = new syscallTrap;
        trappedSyscall->refcount = 1;
        trappedSyscall->syscall_id = syscall;
        readDataSpace( (void*)syscall, sizeof(instruction), trappedSyscall->saved_insn, true);

        instruction insnTrap;
        generateBreakPoint(insnTrap);
        writeDataSpace((void *)syscall, sizeof(instruction), &(insnTrap.raw));
        syscallTraps_.push_back(trappedSyscall);
        return trappedSyscall;
    }
    // Should never be reached
    return NULL;
}

bool process::clearSyscallTrapInternal(syscallTrap *trappedSyscall) {
    // Decrement the reference count, and if it's 0 remove the trapped
    // system call
    assert(trappedSyscall->refcount > 0);
    
    trappedSyscall->refcount--;
    if (trappedSyscall->refcount > 0)
        return true;
    
    // Erk... we hit 0. Undo the trap
    if (!writeDataSpace((void *)trappedSyscall->syscall_id, sizeof(instruction),
                        trappedSyscall->saved_insn))
        return false;
    // Now that we've reset the original behavior, remove this
    // entry from the vector

    pdvector<syscallTrap *> newSyscallTraps;
    for (unsigned iter = 0; iter < syscallTraps_.size(); iter++) {
        if (trappedSyscall != syscallTraps_[iter])
            newSyscallTraps.push_back(syscallTraps_[iter]);
    }
    syscallTraps_ = newSyscallTraps;

    delete trappedSyscall;
    return true;
}


Address dyn_lwp::getCurrentSyscall(Address aixHACK) {
    // We've been given the address of a return point in the function.
    // Get and return the return address
    if (!aixHACK || aixHACK == -1) return 0;

    pd_Function *func = proc_->findFuncByAddr(aixHACK);
    if (!func) {
        return 0;
    }

    pdvector<instPoint *> funcExits = func->funcExits(proc());
    // Only one exit on AIX
    if (funcExits.size() != 1) {
        return 0;
    }
    
    trampTemplate *baseTramp = NULL;
    // [] operator defines if it can't find the data -- that's BAD
    proc()->baseMap.find(funcExits[0],baseTramp);
    
    if (!baseTramp) {
        // Okay... we need a base tramp to insert the breakpoint at,
        // but there's no base tramp there. So insert one (man, this
        // is getting complicated)
        returnInstance *retInstance = NULL;
        bool defer;
        baseTramp = findOrInstallBaseTramp(proc_,
                                            funcExits[0],
                                            retInstance,
                                            false,
                                            false,
                                            defer);
        if (!retInstance) return 0;
        if (!baseTramp) return 0;
        retInstance->installReturnInstance(proc_);

        // But it gets worse :) since we need to fix up the return address
        // to go to our new tramp as well... copy code from catchup side effect
        // fixes
        pdvector<Frame> stackwalk;
        walkStack(stackwalk);
        int i = 0;
        while (stackwalk[i].getPC() != aixHACK) i++;
        
        // Frame won't be uppermost...
        Frame parentFrame = stackwalk[i].getCallerFrame(proc());
        Address oldReturnAddr;
                
        // Here's the fun bit. We actually store the LR in the parent's frame. So 
        // we need to backtrace a bit.
        proc()->readDataSpace((void *)(parentFrame.getFP()+8), sizeof(Address), &oldReturnAddr, false);
        
        if (oldReturnAddr != baseTramp->baseAddr) {
            // Write it into the save slot
            proc()->writeDataSpace((void*)(parentFrame.getFP()+8), sizeof(Address), &(baseTramp->baseAddr));
            proc()->writeDataSpace((void*)(parentFrame.getFP()+12), sizeof(Address), &oldReturnAddr);
        }
    }
    return baseTramp->baseAddr;
}
// Assumes we've stopped at a noop
bool dyn_lwp::stepPastSyscallTrap() {
    Frame frame = getActiveFrame();
    
    return changePC(frame.getPC() + sizeof(instruction), NULL);
}

int dyn_lwp::hasReachedSyscallTrap() {
    
    Frame frame = getActiveFrame();
    if (frame.getPC() == -1)
        return 0;
    if (trappedSyscall_ && frame.getPC() == trappedSyscall_->syscall_id) {
        return 2;
    }
    if (proc()->checkTrappedSyscallsInternal(frame.getPC())) {
        return 1;
    }
    
    return 0;
}

bool dyn_lwp::realLWP_attach_() {
   // Umm... no file descriptors on AIX
   return true;
}

bool dyn_lwp::representativeLWP_attach_() {
   // Umm... no file descriptors on AIX

   // we only need to attach to a process that is not our direct children.

   //ccw 30 apr 2002 : SPLIT5
   pdstring buffer ="attach!";
   statusLine(buffer.c_str());
   
   if (proc_->parent != 0 || proc_->createdViaAttach) {
      return attach_helper(proc_);
   }
   else
      return true;
}

pdstring process::tryToFindExecutable(const pdstring &progpath, int /*pid*/) {
   // returns empty string on failure

    if (!progpath.length())
        cerr << "Warning: Attach on AIX requires program path to be specified" << endl;
    if (progpath.length() == 0)
        return "";
    if (exists_executable(progpath)) // util lib
        return progpath;
    
    return ""; // failure
}

bool process::installSyscallTracing() {
    // turn on 'multiprocess debugging', which allows ptracing of both the
    // parent and child after a fork.  In particular, both parent & child will
    // TRAP after a fork.  Also, a process will TRAP after an exec (after the
    // new image has loaded but before it has started to execute at all).
    // Note that turning on multiprocess debugging enables two new values to be
    // returned by wait(): W_SEWTED and W_SFWTED, which indicate stops during
    // execution of exec and fork, respectively.
    // Should do this in loadSharedObjects
    // Note: this can also get called when we incrementally find a shared object.
    // So? :)
    ptrace(PT_MULTI, getPid(), 0, 1, 0);

    // We mimic system call tracing via instrumentation
    
    // Pre-fork (is this strictly necessary?
    tracingRequests += new instMapping("libc.a/fork", "DYNINST_instForkEntry",
                                       FUNC_ENTRY);
    // Post-fork: handled for us by the system

    // Pre-exec: get the exec'ed file name
    AstNode *arg1 = new AstNode(AstNode::Param, (void *)0);
    tracingRequests += new instMapping("libc.a/execve", "DYNINST_instExecEntry",
                                       FUNC_ENTRY|FUNC_ARG,
                                       arg1);
    // Post-exec: handled for us by the system

    // Pre-exit: get the return code
    
    tracingRequests += new instMapping("libc.a/exit", "DYNINST_instExitEntry",
                                       FUNC_ENTRY|FUNC_ARG,
                                       arg1);
    
    // Post-exit: handled for us by the system
    removeAst(arg1);

    return true;
}



procSyscall_t decodeSyscall(process *p, procSignalWhat_t what)
{
    switch (what) {
  case SYS_fork:
      return procSysFork;
      break;
  case SYS_exec:
      return procSysExec;
      break;
  case SYS_load:
      return procSysLoad;
      break;
  case SYS_exit:
      return procSysExit;
      break;
  default:
      return procSysOther;
      break;
    }
    assert(0);
    return procSysOther;
}

int decodeRTSignal(process *proc,
                   procSignalWhy_t &why,
                   procSignalWhat_t &what,
                   procSignalInfo_t &info)
{
    // We've received a signal we believe was sent
    // from the runtime library. Check the RT lib's
    // status variable and return it.
    // These should be made constants
    pdstring status_str = pdstring("DYNINST_instSyscallState");
    pdstring arg_str = pdstring("DYNINST_instSyscallArg1");

    int status;
    Address arg;

    bool err = false;
    Address status_addr = proc->findInternalAddress(status_str, false, err);
    if (err) {
        // Couldn't find symbol
        return 0;
    }
    if (!proc->readDataSpace((void *)status_addr, sizeof(int), 
                             &status, true)) {
        return 0;
    }

    if (status == 0) {
        return 0; // Nothing to see here
    }
    
    Address arg_addr = proc->findInternalAddress(arg_str, false, err);
    if (err) {
    }
    
    if (!proc->readDataSpace((void *)arg_addr, sizeof(Address),
                             &arg, true))
        assert(0);
    info = (procSignalInfo_t)arg;

    // We need to clear out the DYNINST_instSyscallState variable
    // immediately.  I've run into a problem where we handle a fork (ie.  a
    // stop in the application initiated from our instrumenting fork exit)
    // and then we issue SIGSTOP in loopUntilStopped but we're still stopped
    // here after the fork.  We'll think this second stop at the fork exit is
    // for handling the fork, when really it was just an ordinary stop
    // initiated through loopUntilStopped.
    int init_status = 0;
    proc->writeDataSpace((void*)status_addr, sizeof(int), &init_status);

    switch(status) {
  case 1:
      /* Entry to fork */
      why = procSyscallEntry;
      what = SYS_fork;
      break;
  case 2:
      /* Exit of fork, unused */
      break;
  case 3:
      /* Entry to exec */
      why = procSyscallEntry;
      what = SYS_exec;
      break;
  case 4:
      /* Exit of exec, unused */
      break;
  case 5:
      /* Entry of exit, used for the callback. We need to trap before
         the process has actually exited as the callback may want to
         read from the process */
      why = procSyscallEntry;
      what = SYS_exit;
      break;
  default:
      assert(0);
      break;
    }
    return 1;

}

// the pertinantLWP and wait_options are ignored on Solaris, AIX

process *decodeProcessEvent(dyn_lwp **pertinantLWP, int wait_arg, 
                            procSignalWhy_t &why, procSignalWhat_t &what,
                            procSignalInfo_t &info, bool block) 
{
    int options;
    if (block) options = 0;
    else options = WNOHANG;
    process *proc = NULL;
    why = procUndefined;
    what = 0;
    info = 0;
    int result = 0;
    int sig = 0;
    bool ignore;
    int status;

    result = waitpid( wait_arg, &status, options );
    
    // Translate the signal into a why/what combo.
    // We can fake results here as well: translate a stop in fork
    // to a (SYSEXIT,fork) pair. Don't do that yet.
    if (result > 0) {
        proc = process::findProcess(result);

        if (!proc) {
            // This happens if we fork -- we get a trap from both the
            // parent and the child, but the child came first (and so
            // we have no clue about it). This is handled specially in the
            // fork case below.
        }

        if (proc) {
            // Processes' state is saved in preSignalStatus()
            proc->savePreSignalStatus();
            // Got a signal, process is stopped.
            proc->set_status(stopped);
            
        }
        
        if (WIFEXITED(status)) {
            // Process exited via signal
            why = procExitedNormally;
            what = WEXITSTATUS(status);
            }
        else if (WIFSIGNALED(status)) {
            why = procExitedViaSignal;
            what = WTERMSIG(status);
        }
        else if (WIFSTOPPED(status)) {
            // More interesting (and common) case
            // This is where return value faking would occur
            // as well. procSignalled is a generic return.
            // For example, we translate SIGILL to SIGTRAP
            // in a few cases    
            why = procSignalled;
            what = WSTOPSIG(status);
            // AIX returns status information in the guise
            // of a trap. In this case, fake the info to 
            // the form we want.
            if (what == SIGSTOP) {
                // Not due to a ptrace-derived signal, see if it is
                // from the RT lib
                // Only overwrites why,what,info if the signal is from the RT
                decodeRTSignal(proc, why, what, info);
            }
            else if (what == SIGTRAP) {
                if(proc->childHasSignalled() && status==0x57f) {
                    // This admittedly is a hack.  Occasionally, we'll get the
                    // trap from the child indicating fork exit, but we'll
                    // never get the trap from the parent indicating fork exit.
                    // In this case though, we do appear to reliably get a trap
                    // from the parent with an unexpected status value (0x57f).
                    // We're using this trap as the fork event from the parent.
                    status = W_SFWTED;
                }
                
                switch(status & 0x7f) {
              case W_SLWTED:
                  // Load
                  why = procSyscallExit;
                  what = SYS_load;
                  static bool in_trap_loop = false;
                  static int recurse_level = 0;
                  // Debug info
                  dyn_saved_regs *regs;
                  regs = proc->getRepresentativeLWP()->getRegisters();
                  if (proc->previousSignalAddr() == regs->gprs[3]) {
                      if (!in_trap_loop) {
                          fprintf(stderr, "Spinning to handle multiple traps caused by null library loads...\n");
                          in_trap_loop = true;
                      }
                      
                      // Nothing to see here, move along... you get the idea
                      proc->continueProc();
                      
                      // Even if we perform no processing, we still get a big
                      // slowdown because we don't check for signals often enough.
                      // So we wait until we get a valid event.
                      
                      // We don't want to spin completely, since that makes the
                      // daemon appear to have hung from the outside. So eat the
                      // first, say, 50 signals... then return control to the
                      // main event loop for a second.
                      if (recurse_level < 50) { // Made up constant
                          recurse_level++;
                          // Allow the process a bit of time
                          usleep(500);
                          dyn_lwp *selectedLWP;
                          return decodeProcessEvent(&selectedLWP, wait_arg,
                                                    why, what, info, block);
                      }
                      else {
                          recurse_level = 0;
                          return NULL;
                      }
                  }
                  else if (in_trap_loop) {
                      fprintf(stderr, "Finished spinning, returning to normal processing.\n");
                      in_trap_loop = false;
                  }
                  proc->setPreviousSignalAddr(regs->gprs[3]); 
                  break;
              case W_SFWTED:
                  // Fork
                  why = procSyscallExit;
                  what = SYS_fork;
                  
                  if (proc) {
                      // We're the parent, since the child doesn't have a
                      // process object
                      int childPid = proc->childHasSignalled();
                      if (childPid) {
                          info = childPid;
                          // successfully noticed a fork (both parent and
                          // child), reset state variables this allows
                          // successive forks to work
                          proc->setChildHasSignalled(0);
                          proc->setParentHasSignalled(0);
                      }
                      else {
                          // Haven't seen the child yet, so 
                          // discard this signal. We'll use the child
                          // trap to begin fork handling
                          proc->setParentHasSignalled(result);
                      }
                  }
                  else {
                      // Child-side. See if the parent has trapped yet.
                      // Uh... who is our parent?
                      struct procsinfo psinfo;
                      pid_t temp_child_pid = result;
                      if (getprocs(&psinfo, sizeof(psinfo), NULL, 0, &temp_child_pid, 1) 
                          == -1) {
                          assert(false);
                          return NULL;
                      }
                      
                      assert((pid_t)psinfo.pi_pid == result);
                      
                      int parentPid = psinfo.pi_ppid;
                      process *parent = process::findProcess(parentPid);
                      if (parent->parentHasSignalled()) {
                          // Parent trap was already hit, so do the work here
                          proc = parent;
                          info = result;
                          // successfully noticed a fork, reset state variables
                          proc->setChildHasSignalled(0);
                          proc->setParentHasSignalled(0);
                       }
                       else {
                          //Parent hasn't been seen yet, set variables and wait
                          parent->setChildHasSignalled(result);
                       }
                    }
                    break;
                 case W_SEWTED:
                    // Exec
                    why = procSyscallExit;
                    what = SYS_exec;
                    break;
                 default:                  
                     // Check to see if we've hit a "system call" trap
                     
                    break;
               }
            }
        }
        else {
            fprintf(stderr, "Unknown status 0x%x for process %d\n",
                    status, result);
        }
        
    }
    else if (result < 0) {
        // Possible the process exited but we weren't aware of
        // it yet.
        proc = NULL;
        // If the errno is ECHILD we don't have any children. This
        // is acceptable -- so don't print tons of error messages
        if (errno != ECHILD) 
            perror("waitpid");
    }

    return proc;
}


