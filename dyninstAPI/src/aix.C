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

// $Id: aix.C,v 1.133 2003/03/24 01:38:27 jodom Exp $

#include <pthread.h>
#include "common/h/headers.h"
#include "dyninstAPI/src/os.h"
#include "dyninstAPI/src/signalhandler.h"
#include "dyninstAPI/src/process.h"
#include "dyninstAPI/src/dyn_lwp.h"
#include "dyninstAPI/src/symtab.h"
#include "dyninstAPI/src/stats.h"
#include "common/h/Types.h"
#include "common/h/Dictionary.h"
#include "dyninstAPI/src/Object.h"
#include "dyninstAPI/src/instP.h" // class instInstance
#include "common/h/pathName.h"
#include "dyninstAPI/src/instPoint.h"
#include "dyninstAPI/src/inst-power.h" // Tramp constants

#include <procinfo.h>

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
#include <sys/ptrace.h>
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

#ifdef USES_PMAPI
#include <pmapi.h>
#include "rtinst/h/rthwctr-aix.h"
#endif

const int special_register_codenums [] = {IAR, MSR, CR, LR, CTR, XER, MQ, TID, FPSCR};

void *(*P_native_demangle)(char *, char **, unsigned long);


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

#include "dyninstAPI/src/showerror.h"
#include "common/h/debugOstream.h"

extern "C" {
extern int ioctl(int, int, ...);
};

extern void generateBreakPoint(instruction &);

// The following vrbles were defined in process.C:
extern unsigned enable_pd_attach_detach_debug;

#if ENABLE_DEBUG_CERR == 1
#define attach_cerr if (enable_pd_attach_detach_debug) cerr
#else
#define attach_cerr if (0) cerr
#endif /* ENABLE_DEBUG_CERR == 1 */

extern unsigned enable_pd_inferior_rpc_debug;

#if ENABLE_DEBUG_CERR == 1
#define inferiorrpc_cerr if (enable_pd_inferior_rpc_debug) cerr
#else
#define inferiorrpc_cerr if (0) cerr
#endif /* ENABLE_DEBUG_CERR == 1 */

extern unsigned enable_pd_shm_sampling_debug;

#if ENABLE_DEBUG_CERR == 1
#define shmsample_cerr if (enable_pd_shm_sampling_debug) cerr
#else
#define shmsample_cerr if (0) cerr
#endif /* ENABLE_DEBUG_CERR == 1 */

extern unsigned enable_pd_fork_exec_debug;

#if ENABLE_DEBUG_CERR == 1
#define forkexec_cerr if (enable_pd_fork_exec_debug) cerr
#else
#define forkexec_cerr if (0) cerr
#endif /* ENABLE_DEBUG_CERR == 1 */

extern process* findProcess(int);

class ptraceKludge {
public:
  static bool haltProcess(process *p);
  static bool deliverPtrace(process *p, int req, void *addr,
			    int data, void *addr2);
  static void continueProcess(process *p, const bool halted);
};

bool ptraceKludge::haltProcess(process *p) {
  bool wasStopped = (p->status() == stopped);
  if (p->status() != neonatal && !wasStopped) {
    if (!p->loopUntilStopped()) {
      cerr << "error in loopUntilStopped\n";
      assert(0);
    }
  }
  return wasStopped;
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


// The frame threesome: normal (singlethreaded), thread (given a pthread ID),
// and LWP (given an LWP/kernel thread).
// The behavior is identical unless we're in a leaf node where
// the LR is in a register, then it's different.

Frame Frame::getCallerFrame(process *p) const
{
  typedef struct {
    unsigned oldFp;
    unsigned savedCR;
    unsigned savedLR;
    unsigned compilerInfo;
    unsigned binderInfo;
    unsigned savedTOC;
  } linkArea_t;

  linkArea_t thisStackFrame;
  linkArea_t lastStackFrame;
  linkArea_t stackFrame;

  Frame ret; // zero frame

  // Are we in a leaf function?
  pd_Function *func = p->findFuncByAddr(pc_);
  bool isLeaf = false;
  bool noFrame = false;
  ret.pid_ = pid_;
  ret.thread_ = thread_;
  ret.lwp_ = lwp_;
  if (func && uppermost_) {
    isLeaf = func->makesNoCalls();
    noFrame = func->hasNoStackFrame();
  }

  // Are we in a minitramp?
  bool inTramp = false;
  if (!func) {
    trampTemplate *tramp = NULL;
    instInstance *mini = NULL;
    instPoint *instP = p->findInstPointFromAddress(pc_, &tramp, &mini);
    if (instP) { /* We found something */
      inTramp = true;
    }
  }
  
  // Get current stack frame link area
  if (!p->readDataSpace((caddr_t)fp_, sizeof(linkArea_t),
			(caddr_t)&thisStackFrame, false))
    return Frame();
  p->readDataSpace((caddr_t) thisStackFrame.oldFp, sizeof(linkArea_t),
		   (caddr_t) &lastStackFrame, false);

  if (noFrame)
    stackFrame = thisStackFrame;
  else
    stackFrame = lastStackFrame;
  
  if (isLeaf) {
    // So:
    // Normal: call ptrace(pid)
    // Thread: (not implemented) if bound to a kernel thread, go to LWP
    //    if not, get it from the pthread debug library
    // LWP: call ptrace(lwp)
    struct ptsprs spr_contents;
    if (lwp_) {
      if (P_ptrace(PTT_READ_SPRS, lwp_->get_lwp_id(), (int *)&spr_contents,
		   0, 0) == -1) {
	perror("Failed to read SPR data in getCallerFrameLWP");
	fprintf(stderr, "errno = %d\n", errno);
	return Frame();
      }
      ret.pc_ = spr_contents.pt_lr;
    }
    else if (thread_) {
	cerr << "NOT IMPLEMENTED YET" << endl;
      }
    else { // normal
      ret.pc_ = P_ptrace(PT_READ_GPR, pid_, (void *)LR, 0, 0);
    }
  }
  else if (inTramp) {
    p->readDataSpace((caddr_t)thisStackFrame.oldFp-TRAMP_FRAME_SIZE+TRAMP_SPR_OFFSET+STK_LR, sizeof(int),
		     (caddr_t)&ret.pc_, false);
  }
  else { // Not a leaf function, grab the LR from the stack
    // Oh lordy... but NOT if we're at the entry of the function. See, we haven't
    // saved the LR on the stack frame yet!
    ret.pc_ = stackFrame.savedLR;
  }

  // If we're in instrumented functions, then the actual LR is stored
  // in the stack in the compilerInfo word. But how can we tell this?
  // Well... if there's an exit tramp at the LR addr, then it's the wrong one.
  if (func) {
    instPoint *exitInst = func->funcExits(p)[0];
    if (p->baseMap.defines(exitInst)) { // should always be true
      if (p->baseMap[exitInst]->baseAddr == ret.pc_) {
	// Again, we might be down one too far stack frames
	ret.pc_ = stackFrame.compilerInfo;
      }
    }
  }

  

  if (noFrame) { // We never shifted the stack down, so recycle
    ret.fp_ = fp_;
  }
  else {
    ret.fp_ = thisStackFrame.oldFp;
  }
#ifdef DEBUG_STACKWALK
  fprintf(stderr, "PC %x, FP %x\n", ret.pc_, ret.fp_);
#endif

  return ret;
}


void decodeInstr(unsigned instr_raw) {
  // Decode an instruction. Fun, eh?
  union instructUnion instr;
  instr.raw = instr_raw;

  switch(instr.generic.op) {
  case Bop:
    fprintf(stderr, "Branch (abs=%d, link=%d) to 0x%x\n",
	    instr.iform.aa, instr.iform.lk, instr.iform.li);
    break;
  case CMPIop:
    fprintf(stderr, "CMPI reg(%d), 0x%x\n",
	    instr.dform.ra, instr.dform.d_or_si);
    break;
  case SIop:
    fprintf(stderr, "SI src(%d), tgt(%d), 0x%x\n",
	    instr.dform.ra, instr.dform.rt, instr.dform.d_or_si);
    break;
  case CALop:
    fprintf(stderr, "CAL src(%d), tgt(%d), 0x%x\n",
	    instr.dform.ra, instr.dform.rt, instr.dform.d_or_si);
    break;
  case CAUop:
    fprintf(stderr, "CAU src(%d), tgt(%d), 0x%x\n",
	    instr.dform.ra, instr.dform.rt, instr.dform.d_or_si);
    break;
  case ORILop:
    fprintf(stderr, "ORIL src(%d), tgt(%d), 0x%x\n",
	    instr.dform.rt, instr.dform.ra, instr.dform.d_or_si);
    break;
  case ANDILop:
    fprintf(stderr, "CAU src(%d), tgt(%d), 0x%x\n",
	    instr.dform.rt, instr.dform.ra, instr.dform.d_or_si);
    break;
  case Lop:
    fprintf(stderr, "L src(%d)+0x%x, tgt(%d)\n",
	    instr.dform.ra, instr.dform.d_or_si, instr.dform.rt);
    break;
  case STop:
    fprintf(stderr, "L src(%d), tgt(%d)+0x%x\n",
	    instr.dform.rt, instr.dform.ra, instr.dform.d_or_si);
    break;
  case BCop:
    fprintf(stderr, "BC op(0x%x), CR bit(0x%x), abs(%d), link(%d), tgt(0x%x)\n",
	    instr.bform.bo, instr.bform.bi, instr.bform.aa, instr.bform.lk, instr.bform.bd);
    break;
  case BCLRop:
    switch (instr.xform.xo) {
    case BCLRxop:
      fprintf(stderr, "BCLR op(0x%x), bit(0x%x), link(%d)\n",
	      instr.xform.rt, instr.xform.ra, instr.xform.rc);
      break;
    default:
      fprintf(stderr, "%x\n", instr.raw);
      break;
    }
    break;
  case 0:
    fprintf(stderr, "NULL INSTRUCTION\n");
    break;
  default:
    fprintf(stderr, "Unknown instr with opcode %d\n",
	    instr.generic.op);

    break;
  }
  return;
}      

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
        P_ptrace(PTT_READ_GPRS, lwp_id_, (void *)regs->gprs, 0, 0);
        if (errno != 0) {
            perror("ptrace PTT_READ_GPRS");
            return NULL;
        }
        // Next, the general purpose floating point registers.
        // Again, we read as a block. 
        // ptrace(PTT_READ_FPRS, lwp, &buffer (at least 32*8=256), 0, 0);
        
        P_ptrace(PTT_READ_FPRS, lwp_id_, (void *)regs->fprs, 0, 0);
        if (errno != 0) {
            perror("ptrace PTT_READ_FPRS");
            return NULL;
        }
        
        // We get the SPRs in a special structure. We'll then copy to
        // our buffer for later retrieval. We could save the lot, I suppose,
        // but there's a _lot_ of extra space that is unused.
        struct ptsprs spr_contents;
        
        P_ptrace(PTT_READ_SPRS, lwp_id_, (void *)&spr_contents, 0, 0);
        if (errno) {
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

static bool executeDummyTrap(process *theProc) {
   assert(theProc->status_ == stopped);
   
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
   }
   return true;
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
    
    for (unsigned i=0; i < 32; i++) {
        if (P_ptrace(PT_WRITE_GPR, proc_->getPid(), (void *)i, regs->gprs[i], 0) == -1) {
            //perror("restoreRegisters PT_WRITE_GPR");
            //return false;
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
            perror("ptrace PT_WRITE_FPR");
            cerr << "regnum was " << i << endl;
            return false;
        }
    } 
    
    // Finally, special registers:
    
    for (unsigned i=0; i < num_special_registers; i++) {
      if (P_ptrace(PT_WRITE_GPR, proc_->getPid(), 
                   (void *)(special_register_codenums[i]), 
                   regs->sprs[i], 0) == -1) {
          perror("ptrace PT_WRITE_GPR for a special register");
          cerr << "regnum was " << special_register_codenums[i] << endl;
          return false;
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

bool ptraceKludge::deliverPtrace(process *p, int req, void *addr,
				 int data, void *addr2) {
  bool halted=false;
  bool ret;

  if (req != PT_DETACH) halted = haltProcess(p);

  if (ptrace(req, p->getPid(), (int *)addr, data, (int *)addr2) == -1) // aix 4.1 likes int *
    ret = false;
  else
    ret = true;
  if (req != PT_DETACH) continueProcess(p, halted);
  return ret;
}

void ptraceKludge::continueProcess(process *p, const bool wasStopped) {
  if ((p->status() != neonatal) && (!wasStopped)) {

/* Choose either one of the following methods to continue a process.
 * The choice must be consistent with that in process::continueProc_ and stop_
 */

#ifndef PTRACE_ATTACH_DETACH
    if (ptrace(PT_CONTINUE, p->pid, (int *) 1, SIGCONT, NULL) == -1) {
#else
    if (ptrace(PT_DETACH, p->pid, (int *) 1, SIGCONT, NULL) == -1) { 
    // aix 4.1 likes int *
#endif
      logLine("Error in continueProcess\n");
      assert(0);
    }
  }
}

// already setup on this FD.
// disconnect from controlling terminal 
void OS::osDisconnect(void) {
  int ttyfd = open ("/dev/tty", O_RDONLY);
  ioctl (ttyfd, TIOCNOTTY, NULL); 
  close (ttyfd);
}

bool attach_helper(process *proc) {
   // formerly OS::osAttach()
   int ret = ptrace(PT_ATTACH, proc->getPid(), (int *)0, 0, 0);
   if (ret == -1)
      ret = ptrace(PT_REATT, proc->getPid(), (int *)0, 0, 0);
   return (ret != -1);
}

bool process::stop_() {
/* Choose either one of the following methods for stopping a process, 
 * but not both. 
 * The choice must be consistent with that in process::continueProc_ 
 * and ptraceKludge::continueProcess
 */
#ifndef PTRACE_ATTACH_DETACH
	return (P_kill(pid, SIGSTOP) != -1); 
#else
	// attach generates a SIG TRAP which we catch
	if (!attach_helper(this)) {
	  if (kill(pid, SIGSTOP) == -1)
	     return false;
        }
        return(true);
#endif
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
    string status_str = string("DYNINST_instSyscallState");
    string arg_str = string("DYNINST_instSyscallArg1");

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


process *decodeProcessEvent(int pid,
                            procSignalWhy_t &why,
                            procSignalWhat_t &what,
                            procSignalInfo_t &info,
                            bool block) {
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
    result = waitpid( pid, &status, options );
    
    // Translate the signal into a why/what combo.
    // We can fake results here as well: translate a stop in fork
    // to a (SYSEXIT,fork) pair. Don't do that yet.
    if (result > 0) {
        proc = findProcess(result);

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
            proc->status_ = stopped;
            
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
                switch(status & 0x7f) {
              case W_SLWTED:
                  // Load
                  why = procSyscallExit;
                  what = SYS_load;
                  break;
              case W_SFWTED:
                  // Fork
                  why = procSyscallExit;
                  what = SYS_fork;
                  
                  if (proc) {
                      // We're the parent, since the child doesn't have a
                      // process object
                      int childPid = proc-> childHasSignalled();
                      if (childPid) {
                          info = childPid;
                      }
                      else {
                          // Haven't seen the child yet, so 
                          // discard this signal. We'll use the child
                          // trap to begin fork handling
                          proc->setParentHasSignalled(result);
                          return NULL;
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
                          return false;
                      }
                      
                      assert((pid_t)psinfo.pi_pid == result);
                      
                      int parentPid = psinfo.pi_ppid;
                      process *parent = findProcess(parentPid);
                      if (parent->parentHasSignalled()) {
                          // Parent trap was already hit, so do the work here
                          proc = parent;
                          info = result;
                      }
                      else {
                          // Parent hasn't been seen yet, set variables and wait
                          parent->setChildHasSignalled(result);
                          proc = NULL;
                      }
                  }
                  break;
              case W_SEWTED:
                  // Exec
                  why = procSyscallExit;
                  what = SYS_exec;
                  break;
              default:                  
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
        perror("waitpid");
    }
    
    return proc;
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
    tracingRequests += new instMapping("fork", "DYNINST_instForkEntry",
                                       FUNC_ENTRY);
    // Post-fork: handled for us by the system

    // Pre-exec: get the exec'ed file name
    AstNode *arg1 = new AstNode(AstNode::Param, (void *)0);
    tracingRequests += new instMapping("execve", "DYNINST_instExecEntry",
                                       FUNC_ENTRY|FUNC_ARG,
                                       arg1);
    // Post-exec: handled for us by the system

    // Pre-exit: get the return code
    tracingRequests += new instMapping("exit", "DYNINST_instExitEntry",
                                       FUNC_ENTRY|FUNC_ARG,
                                       arg1);

    // Post-exit: handled for us by the system
    removeAst(arg1);

    return true;
}


// attach to an inferior process.
bool process::attach_() {
  if(getDefaultLWP() == NULL)
     return false;

  // we only need to attach to a process that is not our direct children.

  //ccw 30 apr 2002 : SPLIT5
  string buffer ="attach!";
  statusLine(buffer.c_str());
  
  if (parent != 0 || createdViaAttach) {
      return attach_helper(this);
  }
  else
      return true;
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
bool process::continueProc_() {
  int ret;

  if (!checkStatus()) {
      cerr << "Status check failed" << endl;
      return false; 
  }
  
  ptraceOps++; ptraceOtherOps++;

/* Choose either one of the following methods to continue a process.
 * The choice must be consistent with that in process::continueProc_ and stop_
 */

  if (!ptraceKludge::deliverPtrace(this, PT_CONTINUE, (char*)1, 0, NULL)) {
      perror("ptrace continue");
      ret = -1;
  }
  
  else
    ret = 0;
  // switch these to not detach after every call.
  //ret = ptrace(PT_CONTINUE, pid, (int *)1, 0, NULL);

  return (ret != -1);
}

#ifdef BPATCH_LIBRARY
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
#endif

// TODO ??
bool process::pause_() {
  if (!checkStatus()) 
    return false;
  ptraceOps++; ptraceOtherOps++;
  bool wasStopped = (status() == stopped);
  if (status() != neonatal && !wasStopped) {
     bool res = loopUntilStopped();
     return res;
  }
  else
    return true;
}

bool process::detach_() {
   if (checkStatus()) {
      ptraceOps++; ptraceOtherOps++;
      if (!ptraceKludge::deliverPtrace(this,PT_DETACH,(char*)1,SIGSTOP, NULL))
      {
         sprintf(errorLine, "Unable to detach %d\n", getPid());
         logLine(errorLine);
         showErrorCallback(40, (const char *) errorLine);
      }
   }
   // always return true since we report the error condition.
   return (true);
}

bool process::API_detach_(const bool cont) {
  if (!checkStatus())
      return false;
  ptraceOps++; ptraceOtherOps++;
  return (ptraceKludge::deliverPtrace(this,PT_DETACH,(char*)1, cont ? 0 : SIGSTOP,NULL));
}

// temporarily unimplemented, PT_DUMPCORE is specific to sunos4.1
bool process::dumpCore_(const string coreFile) {
  if (!checkStatus()) 
    return false;
  ptraceOps++; ptraceOtherOps++;

  if (!dumpImage(coreFile)) {
     assert(false);
  }

  (void) ptrace(PT_CONTINUE, pid, (int*)1, SIGBUS, NULL);
  return true;
}

bool process::writeTextWord_(caddr_t inTraced, int data) {
  if (!checkStatus()) 
    return false;
  ptraceBytes += sizeof(int); ptraceOps++;
  return (ptraceKludge::deliverPtrace(this, PT_WRITE_I, inTraced, data, NULL));
}

bool process::writeTextSpace_(void *inTraced, u_int amount, const void *inSelf) {
  return writeDataSpace_(inTraced, amount, inSelf);
}

bool process::readTextSpace_(void *inTraced, u_int amount, const void *inSelf) {
  return readDataSpace_(inTraced, amount, const_cast<void *>(inSelf));
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
bool process::writeDataSpace_(void *inTraced, u_int amount, const void *inSelf) {
  if (!checkStatus()) 
    return false;

  ptraceBytes += amount;

  while (amount > 1024) {
    ptraceOps++;
    if (!ptraceKludge::deliverPtrace(this, PT_WRITE_BLOCK, inTraced,
				     1024, const_cast<void*>(inSelf))) {
      perror("Failed write");
      return false;
    }
    amount -= 1024;
    inTraced = (char *)inTraced + 1024;
    inSelf = (char *)const_cast<void*>(inSelf) + 1024;
  }

  ptraceOps++;
  if (!ptraceKludge::deliverPtrace(this, PT_WRITE_BLOCK, inTraced,
				   amount, const_cast<void*>(inSelf))) {
      fprintf(stderr, "Write of %d bytes from 0x%x to 0x%x\n",
              amount, inSelf, inTraced);      
    perror("Failed write2");
    while(1);
    
    return false;
  }
  return true;
}

bool process::readDataSpace_(const void *inTraced, u_int amount, void *inSelf) {
  if (!checkStatus())
    return false;

  ptraceBytes += amount;

  while (amount > 1024) {
    ptraceOps++;
    if (!ptraceKludge::deliverPtrace(this, PT_READ_BLOCK, 
                                     const_cast<void*>(inTraced),
				     1024, inSelf)) return false;
    amount -= 1024;
    inTraced = (const char *)inTraced + 1024;
    inSelf = (char *)inSelf + 1024;
  }

  ptraceOps++;
  return (ptraceKludge::deliverPtrace(this, PT_READ_BLOCK, 
                const_cast<void*>(inTraced), amount, inSelf));
}

// Can this be unified with linux' version?
// Maybe, but it can be unified with decodeProcessEvent
// if we add a "pid" parameter to decodeProcessEvent. 
bool process::loopUntilStopped() {
  /* make sure the process is stopped in the eyes of ptrace */
    stop_();     //Send the process a SIGSTOP
    
    bool isStopped = false;
    int waitStatus;
    while (!isStopped) {
        procSignalWhy_t why;
        procSignalWhat_t what;
        procSignalInfo_t info;
        if(hasExited()) false;
        process *proc = decodeProcessEvent(pid, why, what, info, true);
        assert(proc == NULL ||
               proc == this);
        if (didProcReceiveSignal(why) &&
            what == SIGSTOP) {
            isStopped = true;
        }
        else {
            handleProcessEvent(this, why, what, info);
        }
    }
    return true;
}


//
// Write out the current contents of the text segment to disk.  This is useful
//    for debugging dyninst.
//
bool process::dumpImage(string outFile) {
    // formerly OS::osDumpImage()
    const string &imageFileName = symbols->file();
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

    // Print the stack from register 1 to 0x30000000
    int instr_temp;
    for (i = gpr_contents[1]; i < 0x30000000; i += 4) {
      if (ptrace(PT_READ_BLOCK, pid, (int *)i, 4, &instr_temp) == -1)
	perror("Failed to read stack!");
      fprintf(stderr, "0x%x: 0x%x", i, instr_temp);
      if ((instr_temp > 0x10000000) && (instr_temp < 0x20000000)) {
	pd_Function *func = symbols->findFuncByAddr((Address) instr_temp, this);
	fprintf(stderr, ":  %s", (func->prettyName()).c_str());
      }
      if ((instr_temp > 0xd0000000) && (instr_temp < 0xe0000000)) {
	pd_Function *func;
	if (shared_objects)
	  for(u_int j=0; j < shared_objects->size(); j++) {
	    const image *img = ((*shared_objects)[j])->getImage();
	    func = img->findFuncByAddr((Address) instr_temp, this);
	    if (func)
	      fprintf(stderr, ":   %s", (func->prettyName()).c_str());
	  }
      }
      fprintf(stderr, "\n");
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
        findProcess(pid)->stop_();

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
fileDescriptor *getExecFileDescriptor(string filename,
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
    string member = "";
    Address text_org = (Address) ptr->ldinfo_textorg;
    Address data_org = (Address) ptr->ldinfo_dataorg;

    fileDescriptor *desc = 
      (fileDescriptor *) new fileDescriptor_AIX(filename, member,
						text_org, data_org,
						pid, true);

    return desc;
}

//
// dummy versions of OS statistics.
//
float OS::compute_rusage_cpu() { return(0.0); }
float OS::compute_rusage_sys() { return(0.0); }
float OS::compute_rusage_min() { return(0.0); }
float OS::compute_rusage_maj() { return(0.0); }
float OS::compute_rusage_swap() { return(0.0); }
float OS::compute_rusage_io_in() { return(0.0); }
float OS::compute_rusage_io_out() { return(0.0); }
float OS::compute_rusage_msg_send() { return(0.0); }
float OS::compute_rusage_sigs() { return(0.0); }
float OS::compute_rusage_vol_cs() { return(0.0); }
float OS::compute_rusage_inv_cs() { return(0.0); }
float OS::compute_rusage_msg_recv() { return(0.0); }

int getNumberOfCPUs()
{
  return(1);
}

// #include "paradynd/src/costmetrics.h"

class instInstance;

string process::tryToFindExecutable(const string &progpath, int /*pid*/) {
   // returns empty string on failure

  if (!progpath.length())
    cerr << "Warning: Attach on AIX requires program path to be specified" << endl;
   if (progpath.length() == 0)
      return "";
   if (exists_executable(progpath)) // util lib
      return progpath;

   return ""; // failure
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
    // Don't support trapping syscalls here yet, sorry
    return NULL;
}

bool process::clearSyscallTrapInternal(syscallTrap *trappedSyscall) {
    assert(0 && "Unimplemented");
    return true;
}

Address dyn_lwp::getCurrentSyscall() {
    return 0;
}

bool dyn_lwp::stepPastSyscallTrap() {
    return false;
}

int dyn_lwp::hasReachedSyscallTrap() {
    return 0;
}

#if defined(duplicated_in_process_c_because_linux_ia64_needs_it)
Address process::getTOCoffsetInfo(Address dest)
{
  // We have an address, and want to find the module the addr is
  // contained in. Given the probabilities, we (probably) want
  // the module dyninst_rt is contained in. 
  // I think this is the right func to use

  if (symbols->findFuncByAddr(dest, this))
    return (Address) (symbols->getObject()).getTOCoffset();

  if (shared_objects)
    for(u_int j=0; j < shared_objects->size(); j++)
      if (((*shared_objects)[j])->getImage()->findFuncByAddr(dest, this))
	return (Address) (((*shared_objects)[j])->getImage()->getObject()).getTOCoffset();
  // Serious error! Assert?
  return 0;
}
#endif

#if !defined(BPATCH_LIBRARY)

/*
// Useful for debugging.  Keeps a history of last CT_MAXRECS of daemon
// side time queries (along with the previous_time).
#define CT_MAXRECS 20
#define CT_NUMPOS 4

typedef struct {
   rawTime64 cpu_time;
   rawTime64 prev_time;   
   unsigned lwp;
   unsigned thr;
} rec_t;

rec_t lastVT[CT_NUMPOS][CT_MAXRECS];
int ct_index[CT_NUMPOS] = { -1, -1, -1, -1 };

void ct_record(unsigned pos, rawTime64 cput, rawTime64 prevt, unsigned lwp_,
               unsigned thr_) {
   assert(pos != 0);
   assert(pos < CT_NUMPOS);
   int index, circ_index;
   index = ++ct_index[pos];
   circ_index = index % CT_MAXRECS;

   rec_t *rec = &lastVT[pos][circ_index];
   rec->cpu_time = cput;
   rec->prev_time = prevt;
   rec->lwp = lwp_;
   rec->thr = thr_;
}

void ct_showTraceB(int pos) {
   int index = ct_index[pos];   
   int rctr = 1;
   fprintf(stderr,"  ----- showTrace, pos = %d  ---------------------\n", pos);
   int rnum;
   for(rnum = index % CT_MAXRECS; rnum >= 0; rnum--, rctr++) {
      rec_t *rec = &lastVT[pos][rnum];
      fprintf(stderr, "%d, cput: %lld, prevt: %lld, lwp: %u, thr: %u", rctr,
              rec->cpu_time, rec->prev_time, rec->lwp, rec->thr);
      if(rec->cpu_time < rec->prev_time) fprintf(stderr, " (RB)\n");
      else fprintf(stderr, "\n");
   }

   if(index > CT_MAXRECS) {
      int circ_index = index % CT_MAXRECS;
      for(rnum = CT_MAXRECS-1; rnum>circ_index; rnum--, rctr++) {
         rec_t *rec = &lastVT[pos][rnum];
         fprintf(stderr, "%d, cput: %lld, prevt: %lld, lwp: %d, thr: %d\n", 
                 rctr, rec->cpu_time, rec->prev_time, rec->lwp, rec->thr);
         if(rec->cpu_time < rec->prev_time) fprintf(stderr, " (RB)\n");
         else fprintf(stderr, "\n");
      }
   }
}

void ct_showTrace(char *msg) {
   fprintf(stderr, "======================================================\n");
   fprintf(stderr, "   %s\n", msg);
   int curPos;
   for(curPos=0; curPos<CT_NUMPOS; curPos++) {
      int index = ct_index[curPos];
      if(index == -1)  { fprintf(stderr, "pos skipped\n"); continue; }
      ct_showTraceB(curPos);
   }
   fprintf(stderr,"=======================================================\n");
   fflush(stderr);
}

extern unsigned pos_junk;
*/

rawTime64 dyn_lwp::getRawCpuTime_hw() 
{
#ifdef USES_PMAPI
   // Hardware method, using the PMAPI
   int ret;
   
   static bool need_init = true;
   if(need_init) {
      pm_info_t pinfo;
#ifdef PMAPI_GROUPS
      pm_groups_info_t pginfo;
      ret = pm_init(PM_VERIFIED | PM_CAVEAT | PM_GET_GROUPS, &pinfo, &pginfo);
#else
      ret = pm_init(PM_VERIFIED | PM_CAVEAT, &pinfo);
#endif
      // We ignore the return, but pm_init must be called to initialize the
      // library
      if (ret) pm_error("PARADYNos_init: pm_init", ret);
      need_init = false;
   }
   int lwp_to_use;
   tid_t indexPtr = 0;   
   struct thrdsinfo thrd_buf;

   if (lwp_id_ > 0) 
      lwp_to_use = lwp_id_;
   else {
      /* If we need to get the data for the entire process (ie. lwp_id_ == 0)
         then we need to the pm_get_data_group function requires any lwp in
         the group, so we'll grab the lwp of any active thread in the
         process */
      if(getthrds(proc_->getPid(), &thrd_buf, sizeof(struct thrdsinfo), 
                  &indexPtr, 1) == 0) {
         // perhaps the process ended
         return -1;
      }
      lwp_to_use = thrd_buf.ti_tid;
   }

   // PM counters are only valid when the process is paused. 
   bool needToCont = (proc_->status() == running);
   if(proc_->status() == running) {
      if(! proc_->pause()) {
         return -1;  // pause failed, so returning failure
      }
   }

   pm_data_t data;
   if(lwp_id_ > 0) 
      ret = pm_get_data_thread(proc_->getPid(), lwp_to_use, &data);
   else {  // lwp == 0, means get data for the entire process (ie. all lwps)
      ret = pm_get_data_group(proc_->getPid(), lwp_to_use, &data);
      while(ret) {
         // if failed, could have been because the lwp (retrieved via
         // getthrds) was in process of being deleted.
         //cerr << "  prev lwp_to_use " << lwp_to_use << " failed\n";
         if(getthrds(proc_->getPid(), &thrd_buf, sizeof(struct thrdsinfo), 
                     &indexPtr, 1) == 0) {
            // couldn't get a valid lwp, go to standard error handling
            ret = 1;
            break;
         }
         lwp_to_use = thrd_buf.ti_tid;
         //cerr << "  next lwp_to_use is " << lwp_to_use << "\n";
         ret = pm_get_data_group(proc_->getPid(), lwp_to_use, &data);
      }
   }

   if (ret) {
       pm_error("dyn_lwp::getRawCpuTime_hw: pm_get_data_thread", ret);
       fprintf(stderr, "Attempted pm_get_data(%d, %d, %d)\n",
               proc_->getPid(), lwp_id_, lwp_to_use);
       return -1;
   }
   rawTime64 result = data.accu[get_hwctr_binding(PM_CYC_EVENT)];

   // Continue the process
   if(needToCont) {
      proc_->continueProc();
   }

   //if(pos_junk != 101)
   //  ct_record(pos_junk, result, hw_previous_, lwp_id_, lwp_to_use);

   if(result < hw_previous_) {
      cerr << "rollback in dyn_lwp::getRawCpuTime_hw, lwp_to_use: " 
           << lwp_to_use << ", lwp: " << lwp_id_ << ", result: " << result 
           << ", previous result: " << hw_previous_ << "\n";
      result = hw_previous_;
   }
   else 
      hw_previous_ = result;
   
   return result;
#else
   return 0;
#endif
}

rawTime64 dyn_lwp::getRawCpuTime_sw()
{
  // returns user+sys time from the user area of the inferior process.
  // Since AIX 4.1 doesn't have a /proc file system, this is slightly
  // more complicated than solaris or the others. 

  // It must not stop the inferior process or assume it is stopped.
  // It must be "in sync" with rtinst's DYNINSTgetCPUtime()

  // Idea number one: use getprocs() (which needs to be included anyway
  // because of a use above) to grab the process table info.
  // We probably want pi_ru.ru_utime and pi_ru.ru_stime.

  // int lwp_id: thread ID of desired time. Ignored for now.
  // int pid: process ID that we want the time for. 
  
  // int getprocs (struct procsinfo *ProcessBuffer, // Array of procsinfos
  //               int ProcessSize,                 // sizeof(procsinfo)
  //               struct fdsinfo *FileBuffer,      // Array of fdsinfos
  //               int FileSize,                    // sizeof(...)
  //               pid_t *IndexPointer,             // Next PID after call
  //               int Count);                      // How many to retrieve
  
  // Constant for the number of processes wanted in info
  const unsigned int numProcsWanted = 1;
  struct procsinfo procInfoBuf[numProcsWanted];
  struct fdsinfo fdsInfoBuf[numProcsWanted];
  int numProcsReturned;
  // The pid sent to getProcs() is modified, so make a copy
  pid_t wantedPid = proc_->getPid();
  // We really don't need to recalculate the size of the structures
  // every call through here. The compiler should optimize these
  // to constants.
  const int sizeProcInfo = sizeof(struct procsinfo);
  const int sizeFdsInfo = sizeof(struct fdsinfo);
  
  if (lwp_id_ > 0) {
    // Whoops, we _really_ don't want to do this. 
    cerr << "Error: calling software timer routine with a valid kernel thread ID" << endl;
  }
  
  numProcsReturned = getprocs(procInfoBuf,
			      sizeProcInfo,
			      fdsInfoBuf,
			      sizeFdsInfo,
			      &wantedPid,
			      numProcsWanted);
  
  if (numProcsReturned == -1) // We have an error
    perror("Failure in getInferiorCPUtime");

  // Now we have the process table information. Since there is no description
  // other than the header file, I've included descriptions of used fields.
  /* 
     struct  procsinfo
     {
        // valid when the process is a zombie only 
        unsigned long   pi_utime;       / this process user time 
        unsigned long   pi_stime;       / this process system time 
        // accounting and profiling data 
        unsigned long   pi_start;       // time at which process began 
        struct rusage   pi_ru;          // this process' rusage info 
        struct rusage   pi_cru;         // children's rusage info 

     };
  */
  // Other things are included, but we don't need 'em here.
  // In addition, the fdsinfo returned is ignored, since we don't need
  // open file descriptor data.

  // This isn't great, since the returned time is in seconds run. It works
  // (horribly) for now, though. Multiply it by a million and we'll call 
  // it a day. Back to the drawing board.

  // Get the time (user+system?) in seconds
  rawTime64 result = 
    (rawTime64) procInfoBuf[0].pi_ru.ru_utime.tv_sec + // User time
    (rawTime64) procInfoBuf[0].pi_ru.ru_stime.tv_sec;  // System time
  
  result *= I64_C(1000000);
  // It looks like the tv_usec fields are actually nanoseconds in this
  // case. If so, it's undocumented -- but I'm getting numbers like
  // "980000000" which is either 980 _million_ microseconds (i.e. 980sec)
  // or .98 seconds if the units are nanoseconds.

  // IF STRANGE RESULTS HAPPEN IN THE TIMERS, make sure that usec is 
  // actually nanos, not micros.

  rawTime64 nanoseconds = 
    (rawTime64) procInfoBuf[0].pi_ru.ru_utime.tv_usec + // User time
    (rawTime64) procInfoBuf[0].pi_ru.ru_stime.tv_usec; //System time
  result += (nanoseconds / 1000);

  if (result < sw_previous_) // Time ran backwards?
    {
      // When the process exits we often get a final time call.
      // If the result is 0(.0), don't print an error.
      if (result) {
	char errLine[150];
	sprintf(errLine,"process::getRawCpuTime_sw - time going backwards in "
		"daemon - cur: %lld, prev: %lld\n", result, sw_previous_);
	cerr << errLine;
	logLine(errLine);
      }
      result = sw_previous_;
    }
  else sw_previous_=result;
  return result;

}

#endif

#if defined(USES_DYNAMIC_INF_HEAP)
static const Address branch_range = 0x01fffffc;
static const Address lowest_addr = 0x10000000;
static const Address highest_addr = 0xe0000000;
Address data_low_addr;
static const Address data_hi_addr = 0xcfffff00;
// Segment 0 is kernel space, and off-limits
// Segment 1 is text space, and OK
// Segment 2-12 (c) is data space
// Segment 13 (d) is shared library text, and scavenged
// Segment 14 (e) is kernel space, and off-limits
// Segment 15 (f) is shared library data, and we don't care about it.
// However, we can scavenge some space in with the shared libraries.

void process::inferiorMallocConstraints(Address near, Address &lo, 
					Address &hi, inferiorHeapType type)
{
  lo = lowest_addr;
  hi = highest_addr;
  if (near)
    {
      if (near < (lowest_addr + branch_range))
	lo = lowest_addr;
      else
	lo = near - branch_range;
      if (near > (highest_addr - branch_range))
	hi = highest_addr;
      else
	hi = near + branch_range;
    }
  switch (type)
    {
    case dataHeap:
      // mmap, preexisting dataheap constraints
      // so shift down lo and hi accordingly
      if (lo < data_low_addr) {
	lo = data_low_addr;
	// Keep within branch range so that we know we can
	// reach anywhere inside.
	if (hi < (lo + branch_range))
	  hi = lo + branch_range;
      }
      if (hi > data_hi_addr) {
	hi = data_hi_addr;
	if (lo > (hi - branch_range))
	  lo = hi - branch_range;
      }
      break;
    default:
      // no change
      break;
    }
}

void process::inferiorMallocAlign(unsigned &size)
{
     /* 32 byte alignment.  Should it be 64? */
  size = (size + 0x1f) & ~0x1f;
}
#endif

#ifndef BPATCH_LIBRARY
bool process::isPmapiAvail() {
#ifdef USES_PMAPI
  return true;
#else
  return false;
#endif
}

void process::initCpuTimeMgrPlt() {
  cpuTimeMgr->installLevel(cpuTimeMgr_t::LEVEL_ONE, &process::isPmapiAvail, 
			   getCyclesPerSecond(), timeBase::bNone(), 
			   &process::getRawCpuTime_hw, "hwCpuTimeFPtrInfo");
  cpuTimeMgr->installLevel(cpuTimeMgr_t::LEVEL_TWO, &process::yesAvail, 
			   timeUnit::us(), timeBase::bNone(), 
			   &process::getRawCpuTime_sw, "swCpuTimeFPtrInfo");
}

class instReqNode;

bool process::catchupSideEffect(Frame &frame, instReqNode *inst)
{
  // Okay, here's what we need to do: when a base tramp is
  // entered, we change the link register to point to the
  // exit tramp. We need to repeat that behavior. It
  // consists of:
  //   Get the address of the exit tramp for the current
  //     function.
  //   Write the old LR to the save slot.
  //   Write this address to where the LR is stored (or the 
  //     register if we have a leaf func)
  //   Layout: (starting at 0 and proceeding upwards)
  //        0: stack backchain
  //        4: saved TC
  //        8: saved LR
  //       12: our save slot for the LR

  Address exitTrampAddr;
  pd_Function *instFunc = (pd_Function *)(inst->Point()->iPgetFunction());
  if (!instFunc) return false;
  // Check: see if the PC is within the instFunc. We might be within
  // an entry or exit tramp, at which point we don't need to fix anything.
  // 27FEB03: if we're before the function _or at the first instruction_ there
  // is no need to fix anything, as we will jump into the base tramp normally
  if ((frame.getPC() <= instFunc->addr()) || (frame.getPC() > instFunc->addr() + instFunc->size()))
    return true;

  const pdvector <instPoint *>exitPoints = instFunc->funcExits(this);
  exitTrampAddr = baseMap[exitPoints[0]]->baseAddr;

  // If the function is a leaf function, we need to overwrite the LR directly.
  bool isLeaf = false;
  if (frame.isUppermost())
    isLeaf = instFunc->makesNoCalls();
  
  if (frame.isLastFrame())
    return false;
  Frame parentFrame = frame.getCallerFrame(this);
  Address oldReturnAddr;

  if (isLeaf) {
    // Stomp the LR
    if (frame.getLWP()) {
      // LWP method
      struct ptsprs spr_contents;
      Address oldLR;
      if (P_ptrace(PTT_READ_SPRS, frame.getLWP()->get_lwp_id(),
                   (int *)&spr_contents, 0, 0) == -1) {
	perror("Failed to read SPRS in catchupSideEffect");
	return false;
      }
      oldReturnAddr = spr_contents.pt_lr;
      spr_contents.pt_lr = (unsigned) exitTrampAddr;
      if (P_ptrace(PTT_WRITE_SPRS, frame.getLWP()->get_lwp_id(),
                   (int *)&spr_contents, 0, 0) == -1) {
	perror("Failed to write SPRS in catchupSideEffect");
	return false;
      }
    }
    else {
      // Old method
      oldReturnAddr = P_ptrace(PT_READ_GPR, pid, (void *)LR, 0, 0);
      if (oldReturnAddr == -1)
	{
	  perror("Failed to read LR in catchupSideEffect");
	}
      if (P_ptrace(PT_WRITE_GPR, pid, (void *)LR, exitTrampAddr, 0) == -1) {
	perror("Failed to write LR in catchupSideEffect");
	return false;
      }
    }
  }
  else {    
    // Here's the fun bit. We actually store the LR in the parent's frame. So 
    // we need to backtrace a bit.
    readDataSpace((void *)(parentFrame.getFP()+8), sizeof(Address), &oldReturnAddr, false);
    if (oldReturnAddr == exitTrampAddr) {
      // We must've already overwritten the link register, so we're fine 
      return true;
    }
    else { 
      // Write it into the save slot
      writeDataSpace((void*)(parentFrame.getFP()+8), sizeof(Address), &exitTrampAddr);
    }
  }
  writeDataSpace((void*)(parentFrame.getFP()+12), sizeof(Address), &oldReturnAddr);
  return true;

}
#endif

bool dyn_lwp::openFD_()
{
  // Umm... no file descriptors on AIX
  return true;
}

void dyn_lwp::closeFD_()
{
  return;
}


//////////////////////////////////////////////////////////
// This code is for when IBM gets the PMAPI interface working
// with AIX 5.1 and /proc
/////////////////////////////////////////////////////////

#if 0
#define GETREG_GPR(regs,reg)   (regs.__gpr[reg])
// AIX system calls can vary in name and number. We need a way
// to decode this mess. So what we do is #define the syscalls we 
// want to numbers and use those to index into a mapping array.
// The first time we try and map a syscall we fill the array in.

int SYSSET_MAP(int syscall, int pid)
{
    static int syscall_mapping[NUM_SYSCALLS];
    static bool mapping_valid = false;
    
    if (mapping_valid)
        return syscall_mapping[syscall];
    
    for (int i = 0; i < NUM_SYSCALLS; i++)
        syscall_mapping[i] = -1;
    
    // Open and read the sysent file to find exit, fork, and exec.
    prsysent_t sysent;
    prsyscall_t *syscalls;
    int fd;
    char filename[256];
    char syscallname[256];
    sprintf(filename, "/proc/%d/sysent", pid);
    fd = open(filename, O_RDONLY, 0);
    if (read(fd, &sysent,
             sizeof(sysent) - sizeof(prsyscall_t))
        != sizeof(sysent) - sizeof(prsyscall_t))
        perror("AIX syscall_map: read");
    syscalls = (prsyscall_t *)malloc(sizeof(prsyscall_t)*sysent.pr_nsyscalls);
    if (read(fd, syscalls,
             sizeof(prsyscall_t)*sysent.pr_nsyscalls) !=
        sizeof(prsyscall_t)*sysent.pr_nsyscalls)
        perror("AIX syscall_map: read2");
    for (int j = 0; j < sysent.pr_nsyscalls; j++) {
        lseek(fd, syscalls[j].pr_nameoff, SEEK_SET);
        read(fd, syscallname, 256);
        
        // Now comes the interesting part. We're interested in a list of
        // system calls. Compare the freshly read name to the list, and if
        // there is a match then set the syscall mapping.
        if (!strcmp(syscallname, "_exit")) {
            syscall_mapping[SYS_exit] = syscalls[j].pr_number;
        }
        else if (!strcmp(syscallname, "_kfork")) {
            syscall_mapping[SYS_fork] = syscalls[j].pr_number;
        }
        else if (!strcmp(syscallname, "execve")) {    
            syscall_mapping[SYS_exec] = syscalls[j].pr_number;
        }
        
    }
    close(fd);
    free(syscalls);
    mapping_valid = true;
    return syscall_mapping[syscall];
}

// Bleah...
unsigned SYSSET_SIZE(sysset_t *x)
{
    // (pr_size - 1) because sysset_t is one uint64_t too large
    return sizeof(sysset_t) + (sizeof (uint64_t) * (x->pr_size-1));
}

sysset_t *SYSSET_ALLOC(int pid)
{
    static bool init = false;
    static int num_calls = 0;
    if (!init) {
        prsysent_t sysent;
        int fd;
        char filename[256];
        sprintf(filename, "/proc/%d/sysent", pid);
        fd = open(filename, O_RDONLY, 0);
        if (read(fd, &sysent,
                 sizeof(sysent) - sizeof(prsyscall_t))
            != sizeof(sysent) - sizeof(prsyscall_t))
            perror("AIX syscall_alloc: read");
        num_calls = sysent.pr_nsyscalls;
        init = true;
        close(fd);
    }
    int size = 0; // Number of 64-bit ints we use for the bitmap
    // array size (*8 because we're bitmapping)
    size = ((num_calls / (8*sizeof(uint64_t))) + 1);
    sysset_t *ret = (sysset_t *)malloc(sizeof(sysset_t) 
                                       - sizeof(uint64_t) 
                                       + size*sizeof(uint64_t));

    ret->pr_size = size;
    
    return ret;
}

bool process::get_entry_syscalls(pstatus_t *status,
                                 sysset_t *entry)
{
    // If the offset is 0, no syscalls are being traced
    if (status->pr_sysentry_offset == 0) {
        premptysysset(entry);
    }
    else {
        // The entry member of the status vrble is a pointer
        // to the sysset_t array.
        if (pread(status_fd(), entry, 
                  SYSSET_SIZE(entry), status->pr_sysentry_offset)
            != SYSSET_SIZE(entry)) {
            perror("get_entry_syscalls: read");
            return false;
        }
    }
    return true;
}

bool process::get_exit_syscalls(pstatus_t *status,
                                sysset_t *exit)
{
    // If the offset is 0, no syscalls are being traced
    if (status->pr_sysexit_offset == 0) {
        premptysysset(exit);
    }
    else {
        if (pread(status_fd(), exit, 
                  SYSSET_SIZE(exit), status->pr_sysexit_offset)
            != SYSSET_SIZE(exit)) {
            perror("get_exit_syscalls: read");
            return false;
        }
    }
    
    return true;
}

#endif

#if defined(BPATCH_LIBRARY)
#define DEBUG_MSG 0 
#define _DEBUG_MSG 0
void compactLoadableSections(pdvector <imageUpdate*> imagePatches, pdvector<imageUpdate*> &newPatches){
	int startPage, stopPage;
	imageUpdate *patch;
	//this function now returns only ONE section that is loadable.
	int pageSize = getpagesize();

	imageUpdate *curr, *next;
	bool foundDup=true;
	unsigned int j;

	VECTOR_SORT(imagePatches, imageUpdateSort);

	while(foundDup){
		foundDup = false;
		j =0;
	        while(imagePatches[j]->address==0 && j < imagePatches.size()){
       	        	j++;
        	}
		curr = imagePatches[j];

		for(j++;j<imagePatches.size();j++){
			next = imagePatches[j];		
			if(curr->address == next->address){
				//duplicate
				//find which is bigger and save that one.
				if(curr->size > next->size){
					next->address=0;
				}else{
					curr->address=0;
					curr=next;
				}
				foundDup =true;
			}else{
				curr=next;
			}

		}
		VECTOR_SORT(imagePatches, imageUpdateSort);
	}


	for(unsigned int i=0;i<imagePatches.size();i++){
		if(imagePatches[i]->address!=0){
			imagePatches[i]->startPage = imagePatches[i]->address- imagePatches[i]->address%pageSize;
			imagePatches[i]->stopPage = imagePatches[i]->address + imagePatches[i]->size- 
					(imagePatches[i]->address + imagePatches[i]->size )%pageSize;

		}
	}

	foundDup = true;

	while(foundDup){
		foundDup = false;
                j =0;
                while(imagePatches[j]->address==0 && j < imagePatches.size()){
                        j++;
                }
		imagePatches.erase(0,j-1);
		j=0;
		for(;j<imagePatches.size()-1;j++){
			if(imagePatches[j]->stopPage > imagePatches[j+1]->startPage){
				foundDup = true;
				if(imagePatches[j]->stopPage > imagePatches[j+1]->stopPage){
					imagePatches[j+1]->address = 0;	
				}else{

					imagePatches[j]->size = (imagePatches[j+1]->address + imagePatches[j+1]->size) -
						imagePatches[j]->address;
					imagePatches[j+1]->address = 0; 
					imagePatches[j]->stopPage = imagePatches[j]->address + imagePatches[j]->size-
                                        	(imagePatches[j]->address + imagePatches[j]->size )%pageSize;		
				}
			}  
		}
		VECTOR_SORT(imagePatches, imageUpdateSort);
	}

	unsigned int k=0;

	while(imagePatches[k]->address==0 && k < imagePatches.size()){
	        k++;
        }

	startPage = imagePatches[k]->startPage;
	stopPage = imagePatches[imagePatches.size()-1]->stopPage;
	int startIndex=k, stopIndex=imagePatches.size()-1;
	/*if(DEBUG_MSG){
		printf("COMPACTING....\n");	
		printf("COMPACTING %x %x %x\n", imagePatches[0]->startPage, stopPage, imagePatches[0]->address);
	}
	patch = new imageUpdate;
        patch->address = imagePatches[startIndex]->address;
        patch->size = imagePatches[stopIndex]->address - imagePatches[startIndex]->address +
                                   imagePatches[stopIndex]->size;
        newPatches.push_back(patch);
	if(DEBUG_MSG){
		printf(" COMPACTED: %x --> %x \n", patch->address, patch->size);
	}*/
	bool finished = false;
	if(_DEBUG_MSG){
		printf("COMPACTING....\n");	
		printf("COMPACTING %x %x %x\n", imagePatches[0]->startPage, stopPage, imagePatches[0]->address);
	}
	for(;k<imagePatches.size();k++){
		if(imagePatches[k]->address!=0){
			if(_DEBUG_MSG){
				printf("COMPACTING k[start] %x k[stop] %x stop %x addr %x size %x\n", imagePatches[k]->startPage, 
					imagePatches[k]->stopPage,stopPage, imagePatches[k]->address, imagePatches[k]->size);
			}
			if(imagePatches[k]->startPage <= stopPage){
				stopIndex = k;
				stopPage = imagePatches[k]->stopPage;
			}else{

				patch = new imageUpdate;
				patch->address = imagePatches[startIndex]->address;
				patch->size = imagePatches[stopIndex]->address - imagePatches[startIndex]->address + 
						imagePatches[stopIndex]->size;
				newPatches.push_back(patch);
				if(_DEBUG_MSG){
					printf(" COMPACTED: address %x --> %x    start %x  stop %x\n", 
						patch->address, patch->size, startPage,  stopPage);
				}
				finished = true;
				//was k+1	
				if(k < imagePatches.size()){
					while(imagePatches[k]->address==0 && k < imagePatches.size()){
						k++;
					}
					startIndex = k;
					stopIndex = k;
					startPage = imagePatches[k]->startPage;
					stopPage  = imagePatches[k]->stopPage;
					finished = false;
					if(k == imagePatches.size()){
						finished = true;
					}
				} 
			}
		}

	}

	if(!finished){
		patch = new imageUpdate;
                patch->address = imagePatches[startIndex]->address;
                patch->size = imagePatches[stopIndex]->address - imagePatches[startIndex]->address +
                                   imagePatches[stopIndex]->size;
                newPatches.push_back(patch);
		if(_DEBUG_MSG){
			printf(" COMPACTED: %x --> %x \n", patch->address, patch->size);
			fflush(stdout);
		}
	}	

	
}

void compactSections(pdvector <imageUpdate*> imagePatches, pdvector<imageUpdate*> &newPatches){

	unsigned startPage, stopPage;
	imageUpdate *patch;

	int pageSize = getpagesize();

	imageUpdate *curr, *next;
	bool foundDup=true;
	unsigned int j;

	VECTOR_SORT(imagePatches, imageUpdateSort);

	while(foundDup){
		foundDup = false;
		j =0;
	        while(imagePatches[j]->address==0 && j < imagePatches.size()){
       	        	j++;
        	}
		curr = imagePatches[j];

		for(j++;j<imagePatches.size();j++){
			next = imagePatches[j];		
			if(curr->address == next->address){
				//duplicate
				//find which is bigger and save that one.
				if(curr->size > next->size){
					next->address=0;
				}else{
					curr->address=0;
					curr=next;
				}
				foundDup =true;
			}else{
				curr=next;
			}

		}
		VECTOR_SORT(imagePatches, imageUpdateSort);
	}
	if(DEBUG_MSG){
		printf(" SORT 1 %d \n", imagePatches.size());
	
		for(unsigned int kk=0;kk<imagePatches.size();kk++){
			printf("%d address 0x%x  size 0x%x \n",kk, imagePatches[kk]->address, imagePatches[kk]->size);
		}
		fflush(stdout);
	}

	unsigned int endAddr;
	for(unsigned int i=0;i<imagePatches.size();i++){
		if(imagePatches[i]->address!=0){
			imagePatches[i]->startPage = imagePatches[i]->address- (imagePatches[i]->address%pageSize);
				
			endAddr = imagePatches[i]->address + imagePatches[i]->size;
			imagePatches[i]->stopPage =  endAddr - (endAddr % pageSize);

			if(DEBUG_MSG){
				printf("%d address %x end addr %x : start page %x stop page %x \n",
					i,imagePatches[i]->address ,imagePatches[i]->address + imagePatches[i]->size,
					imagePatches[i]->startPage, imagePatches[i]->stopPage);
			}
		}
	
	}
	foundDup = true;

	while(foundDup){
		foundDup = false;
                j =0;
                while(imagePatches[j]->address==0 && j < imagePatches.size()){
                        j++;
                }
		//imagePatches.erase(0,j-1); //is it correct to erase here? 
		//j = 0;
		for(;j<imagePatches.size()-1;j++){ 
			if(imagePatches[j]->address!=0 && imagePatches[j]->stopPage >= imagePatches[j+1]->startPage){
				foundDup = true;
				if(imagePatches[j]->stopPage > imagePatches[j+1]->stopPage){
					imagePatches[j+1]->address = 0;	
				}else{
					imagePatches[j]->size = (imagePatches[j+1]->address + imagePatches[j+1]->size) -
						imagePatches[j]->address;
					imagePatches[j+1]->address = 0; 
					endAddr = imagePatches[j]->address + imagePatches[j]->size;
					imagePatches[j]->stopPage =  endAddr - (endAddr % pageSize);
				}
			}  
		}
		VECTOR_SORT(imagePatches, imageUpdateSort);
	}

	unsigned int k=0;

	if(DEBUG_MSG){
		printf(" SORT 3 %d \n", imagePatches.size());

		for(unsigned int kk=0;kk<imagePatches.size();kk++){
			printf("%d address 0x%x  size 0x%x \n",kk, imagePatches[kk]->address, imagePatches[kk]->size);
		}
		fflush(stdout);
	}
	while(imagePatches[k]->address==0 && k < imagePatches.size()){
	        k++;
        }

	startPage = imagePatches[k]->startPage;
	stopPage = imagePatches[k]->stopPage;
	int startIndex=k, stopIndex=k;
	bool finished = false;
	if(DEBUG_MSG){
		printf("COMPACTING....\n");	
		printf("COMPACTING %x %x %x\n", imagePatches[0]->startPage, stopPage, imagePatches[0]->address);
	}
	for(;k<imagePatches.size();k++){
		if(imagePatches[k]->address!=0){
			if(DEBUG_MSG){
				printf("COMPACTING k[start] %x k[stop] %x stop %x addr %x size %x\n", imagePatches[k]->startPage, 
					imagePatches[k]->stopPage,stopPage, imagePatches[k]->address, imagePatches[k]->size);
			}
			if(imagePatches[k]->startPage <= stopPage){
				stopIndex = k;
				stopPage = imagePatches[k]->stopPage;
			}else{

				patch = new imageUpdate;
				patch->address = imagePatches[startIndex]->address;
				patch->size = imagePatches[stopIndex]->address - imagePatches[startIndex]->address + 
						imagePatches[stopIndex]->size;
				newPatches.push_back(patch);
				if(DEBUG_MSG){
					printf(" COMPACTED: address %x --> %x    start %x  stop %x\n", 
						patch->address, patch->size, startPage,  stopPage);
				}
				finished = true;
				//was k+1	
				if(k < imagePatches.size()){
					while(imagePatches[k]->address==0 && k < imagePatches.size()){
						k++;
					}
					startIndex = k;
					stopIndex = k;
					startPage = imagePatches[k]->startPage;
					stopPage  = imagePatches[k]->stopPage;
					finished = false;
					if(k == imagePatches.size()){
						finished = true;
					}
				} 
			}
		}

	}

	if(!finished){
		patch = new imageUpdate;
                patch->address = imagePatches[startIndex]->address;
                patch->size = imagePatches[stopIndex]->address - imagePatches[startIndex]->address +
                                   imagePatches[stopIndex]->size;
                newPatches.push_back(patch);
		if(DEBUG_MSG){
			printf(" COMPACTED: %x --> %x \n", patch->address, patch->size);
			fflush(stdout);
		}
	}	
	
}


void process::addLib(char* lname){

	BPatch_thread *appThread = thread;
	BPatch_image *appImage = thread->getImage();

    BPatch_Vector<BPatch_point *> *mainFunc;

	bool isTrampRecursive = BPatch::bpatch->isTrampRecursive();
    BPatch::bpatch->setTrampRecursive( true ); //ccw 31 jan 2003

	BPatch_function *mainFuncPtr = 	appImage->findFunction("main");
	if( mainFuncPtr == NULL){
		fprintf(stderr,"Unable to find function \"main\". Save the world will fail.\n");
		return;
	}

	mainFunc = mainFuncPtr->findPoint(BPatch_entry);

    if (!mainFunc || ((*mainFunc).size() == 0)) {
	fprintf(stderr, "    Unable to find entry point to \"main.\"\n");
	exit(1);
    }

    BPatch_function *dlopen_func;

    dlopen_func = appImage->findFunction("dlopen");

    if (dlopen_func == NULL) {
	fprintf(stderr, "Unable to find function \"dlopen\"\n");
	exit(1);
    }

    BPatch_Vector<BPatch_snippet *> dlopen_args;
    BPatch_constExpr nameArg(lname);
    BPatch_constExpr rtldArg(4);

    dlopen_args.push_back(&nameArg);
    dlopen_args.push_back(&rtldArg);

    BPatch_funcCallExpr dlopenExpr(*dlopen_func, dlopen_args);

	//printf(" inserting DLOPEN(%s)\n",lname);
	requestTextMiniTramp = 1;
		
    appThread->insertSnippet(dlopenExpr, *mainFunc, BPatch_callBefore, BPatch_firstSnippet);
	requestTextMiniTramp = 0;

	BPatch::bpatch->setTrampRecursive( isTrampRecursive ); //ccw 31 jan 2003
}


//save world
char* process::dumpPatchedImage(string imageFileName){ //ccw 28 oct 2001

	writeBackXCOFF *newXCOFF;
	//addLibrary *addLibraryXCOFF;
	//char name[50];	
	pdvector<imageUpdate*> compactedUpdates;
	pdvector<imageUpdate*> compactedHighmemUpdates;
	void *data;//, *paddedData;
	//Address guardFlagAddr;
	char *directoryName = 0;

	if(!collectSaveWorldData){
		BPatch_reportError(BPatchSerious,122,"dumpPatchedImage: BPatch_thread::startSaveWorld() not called.  No mutated binary saved\n");
		return NULL;
	}

	directoryName = saveWorldFindDirectory();
	if(!directoryName){
		return NULL;
	}
	strcat(directoryName, "/");


	//at this point build an ast to call dlopen("libdyninstAPI_RT.so.1",);
	//and insert it at the entry point of main.

	addLib("libdyninstAPI_RT.so.1");

	
#ifndef USE_STL_VECTOR
	imageUpdates.sort(imageUpdateSort);// imageUpdate::mysort ); 
#else
	sort(imageUpdates.begin(), imageUpdates.end(), imageUpdateOrderingRelation());
#endif

	compactLoadableSections(imageUpdates,compactedUpdates);

#ifndef USE_STL_VECTOR
	highmemUpdates.sort( imageUpdateSort);
#else
	sort(highmemUpdates.begin(), highmemUpdates.end(), imageUpdateOrderingRelation());
#endif
	compactSections(highmemUpdates, compactedHighmemUpdates);

	imageFileName = "dyninst_mutatedBinary";
	char* fullName = new char[strlen(directoryName) + strlen (imageFileName.c_str())+1];
    strcpy(fullName, directoryName);
    strcat(fullName, imageFileName.c_str());

	newXCOFF = new writeBackXCOFF( (char *)getImage()->file().c_str(), fullName /*"/tmp/dyninstMutatee"*/ );

	newXCOFF->registerProcess(this);
	//int sectionsAdded = 0;
	//unsigned int newSize, nextPage, paddedDiff;
	//unsigned int pageSize = getpagesize();


	//This adds the LOADABLE HEAP TRAMP sections
	//AIX/XCOFF NOTES:
	//On AIX we allocate the heap tramps in two locations:
	//on the heap (0x20000000) and around the text section (0x10000000)
	//The os loader will ONLY load ONE text section, ONE data section and
	//ONE bss section. We cannot (from within the mutated binary)
	//muck with addresses in the range 0x10000000 - 0x1fffffff so to reload
	//these tramps we MUST expand the text section and tack these on the
	//end.  THIS WILL INCREASE THE FILE SIZE BY A HUGE AMOUNT.  The file
	//size will increase by (sizeof(text section) + sizeof(tramps) + (gap between text section and tramps))
	//the gap may be quite large 

	//SO we do NOT do what we do on the other platforms, ie work around the
	//heap with the compactedUpdates. we just expand the text section and 
	//tack 'em on the end.

	assert(compactedUpdates.size() < 2);
	(char*) data = new char[compactedUpdates[0]->size];
	readDataSpace((void*) compactedUpdates[0]->address, compactedUpdates[0]->size, data, true);	

	newXCOFF->attachToText(compactedUpdates[0]->address,compactedUpdates[0]->size, (char*)data);

	saveWorldCreateHighMemSections(compactedHighmemUpdates, highmemUpdates, (void*) newXCOFF);

        saveWorldCreateDataSections((void*)newXCOFF);

	newXCOFF->createXCOFF();
	newXCOFF->outputXCOFF();
/*
	char* fullName = new char[strlen(directoryName) + strlen ( (char*)imageFileName.c_str())+1];
        strcpy(fullName, directoryName);
        strcat(fullName, (char*)imageFileName.c_str());

        addLibraryXCOFF= new addLibrary(fullName, "/tmp/dyninstMutatee", "libdyninstAPI_RT.so.1");

	addLibraryXCOFF->outputXCOFF();
*/
        delete [] fullName;

	return directoryName;
}

#endif

void loadNativeDemangler() {
  
  P_native_demangle = NULL;
  // The following is untested - JMO 03/21/03
#if 0
  void *hDemangler = dlopen("libdemangle.a", 0);
  if (hDemangler != NULL)
    P_native_demangle = ((void *) (*) (char *, char **, unsigned long)) 
      dlsym(hDemangler, "demangle");
#endif
}
