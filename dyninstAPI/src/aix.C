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

// $Id: aix.C,v 1.115 2002/11/21 23:41:51 bernat Exp $

#include <pthread.h>
#include "common/h/headers.h"
#include "dyninstAPI/src/os.h"
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

#ifdef USES_PMAPI
#include <pmapi.h>
#include "rtinst/h/rthwctr-aix.h"
#endif

const int special_register_codenums [] = {IAR, MSR, CR, LR, CTR, XER, MQ, TID, FPSCR};


/* Getprocs() should be defined in procinfo.h, but it's not */
extern "C" {
extern int getprocs(struct procsinfo *ProcessBuffer,
		    int ProcessSize,
		    struct fdsinfo *FileBuffer,
		    int FileSize,
		    pid_t *IndexPointer,
		    int Count);
extern int getthrds(pid_t, struct thrdsinfo *, int, tid_t, int);
}

#include "dyninstAPI/src/showerror.h"
#include "common/h/debugOstream.h"

extern "C" {
extern int ioctl(int, int, ...);
};

extern void generateBreakPoint(instruction &);

// The following vrbles were defined in process.C:
extern debug_ostream attach_cerr;
extern debug_ostream inferiorrpc_cerr;
extern debug_ostream shmsample_cerr;
extern debug_ostream forkexec_cerr;

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

  if (lwp_) { // We have a kernel thread to target. Nifty, eh?
    struct ptsprs spr_contents;
    bool kernel_mode = false;
    if (P_ptrace(PTT_READ_SPRS, lwp_, (int *)&spr_contents, 0, 0) == -1) {
      if (errno != EPERM) { // EPERM == thread in kernel mode, not to worry
	perror("----------Error getting IAR in getActiveFrame");
	fprintf(stderr, "dyn_lwp of 0x%x with lwp %d\n", (unsigned) this, lwp_);
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
      if (P_ptrace(PTT_READ_GPRS, lwp_, (int *)allRegs, 0, 0) == -1) {
	perror("Problem reading stack pointer in getActiveFrame");
	return Frame();
      }
      fp = allRegs[1];
    }
    else { // We're in the kernel. Any idea how to get the (old) PC and FP?
      struct thrdsinfo thrd_buf[1000]; // 1000 should be enough for anyone!
      getthrds(proc_->getPid(), thrd_buf, sizeof(struct thrdsinfo), 0, 1000);
      unsigned foo = 0;
      while (thrd_buf[foo].ti_tid != lwp_) foo++;
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

// We don't have leaf frames yet (should we?)
bool process::needToAddALeafFrame(Frame, Address &){
  return false;
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
      if (P_ptrace(PTT_READ_SPRS, lwp_->get_lwp(), (int *)&spr_contents,
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
    
    if (!lwp_) {
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
                return NULL;
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
            
            if (P_ptrace(PT_READ_FPR, proc_->getPid(), &value,
                         FPR0 + i, // see <sys/reg.h>
                         0) == -1) {
                perror("ptrace PT_READ_FPR");
                cerr << "regnum was " << FPR0 + i << "; FPR0=" << FPR0 << endl;
                return NULL;
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
                return NULL;
            }
            regs->sprs[i] = value;
        }
    }
    
    else {
        cerr << "Trying based on lwp_, which is " << lwp_ << endl;
        P_ptrace(PTT_READ_GPRS, lwp_, (void *)regs->gprs, 0, 0);
        if (errno != 0) {
            perror("ptrace PTT_READ_GPRS");
            return NULL;
        }
        // Next, the general purpose floating point registers.
        // Again, we read as a block. 
        // ptrace(PTT_READ_FPRS, lwp, &buffer (at least 32*8=256), 0, 0);
        
        P_ptrace(PTT_READ_FPRS, lwp_, (void *)regs->fprs, 0, 0);
        if (errno != 0) {
            perror("ptrace PTT_READ_FPRS");
            return NULL;
        }
        
        // We get the SPRs in a special structure. We'll then copy to
        // our buffer for later retrieval. We could save the lot, I suppose,
        // but there's a _lot_ of extra space that is unused.
        struct ptsprs spr_contents;
        
        P_ptrace(PTT_READ_SPRS, lwp_, (void *)&spr_contents, 0, 0);
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
  if (lwp_) {
    // Easiest way to check: try to read GPRs and see
    // if we get EPERM back
    struct ptsprs spr_contents;
    P_ptrace(PTT_READ_SPRS, lwp_, (int *)&spr_contents, 0, 0); // aix 4.1 likes int *
  }
  else {
     // aix 4.1 likes int *
     retCode = P_ptrace(PT_READ_GPR, proc_->getPid(), (int *) IAR, 0, 0); 
  }
  
  if(retCode == -1) {
     if (errno == EPERM) {
        return true;
     } else {
        if(lwp_)  perror("dyn_lwp::executingSystemCall() failed on call to "
                         "P_ptrace(PTT_READ_SPRS");
        else      perror("dyn_lwp::executingSystemCall() failed on call to "
                         "PT_READ_GPR");
        assert(false);
     }
  }
  return false;
}

bool dyn_lwp::changePC(Address loc, struct dyn_saved_regs *)
{
  if (!lwp_) {
    if ( !P_ptrace(PT_READ_GPR, proc_->getPid(), (void *)IAR, 0, 0)) {
      cerr << "changePC failed because couldn't re-read IAR register" << endl;
      return false;
    }
    
    
    if (P_ptrace(PT_WRITE_GPR, proc_->getPid(), (void *)IAR, loc, 0) == -1) {
      perror("changePC (PT_WRITE_GPR) failed");
      return false;
    }
    
    // Double-check that the change was made by reading the IAR register
    if (P_ptrace(PT_READ_GPR, proc_->getPid(), (void *)IAR, 0, 0) != (int)loc) {
      cerr << "changePC failed because couldn't re-read IAR register" << endl;
      return false;
    }
  }
  else {
    struct ptsprs spr_contents;
    if (P_ptrace(PTT_READ_SPRS, lwp_, (void *)&spr_contents, 0, 0) == -1) {
      perror("changePC: PTT_READ_SPRS failed");
      return false;
    }
    spr_contents.pt_iar = loc;
    // Write the registers back in
    
    if (P_ptrace(PTT_WRITE_SPRS, lwp_, (void *)&spr_contents, 0, 0) == -1) {
      perror("changePC: PTT_WRITE_SPRS failed");
      return false;
    }
  }
  return true;
}

bool dyn_lwp::restoreRegisters(struct dyn_saved_regs *regs) {
    if (!regs) return false;
    
  if (!lwp_) {
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
      if (P_ptrace(PTT_WRITE_GPRS, lwp_, (void *)regs->gprs, 0, 0) == -1) {
          perror("ptrace PTT_WRITE_GPRS");
          return false;
      }
      // Next, the general purpose floating point registers.
      // ptrace(PTT_WRITE_FPRS, lwp, &buffer (at least 32*8=256), 0, 0);
      if (P_ptrace(PTT_WRITE_FPRS, lwp_, (void *)regs->fprs, 0, 0) == -1) {
          perror("ptrace PTT_WRITE_FPRS");
          return false;
      }
      
      // We get the SPRs in a special structure. We'll then copy to
      // our buffer for later retrieval. We could save the lot, I suppose,
      // but there's a _lot_ of extra space that is unused.
      struct ptsprs saved_sprs;
      struct ptsprs current_sprs;
      // Restore. List: IAR, MSR, CR, LR, CTR, XER, MQ, TID, FPSCR
      ptrace(PTT_READ_SPRS, lwp_, (int *)&current_sprs, 0, 0);
      
      saved_sprs.pt_iar = regs->sprs[0];
      saved_sprs.pt_msr = regs->sprs[1];
      saved_sprs.pt_cr = regs->sprs[2];
      saved_sprs.pt_lr = regs->sprs[3];
      saved_sprs.pt_ctr = regs->sprs[4];
      saved_sprs.pt_xer = regs->sprs[5];
      saved_sprs.pt_mq = regs->sprs[6];
      saved_sprs.pt_reserved_0 = regs->sprs[7];
      saved_sprs.pt_fpscr = regs->sprs[8];
      
      if (P_ptrace(PTT_WRITE_SPRS, lwp_, (void *)&saved_sprs, 0, 0) == -1) {
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
	if (!attach_()) {
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

// wait for a process to terminate or stop
#ifdef BPATCH_LIBRARY
int process::waitProcs(int *status, bool block) {
  int options;
  if (block) options = 0;
  else options = WNOHANG;
#else
int process::waitProcs(int *status) {
  int options = WNOHANG;
#endif
  int result = 0;
  int sig = 0;
  bool ignore;

  do {
      ignore = false;
      result = waitpid( -1, status, options);
      // if the signal's not SIGSTOP or SIGTRAP,
      // send the signal back and wait for another.
      if( result > 0 && WIFSTOPPED(*status) ) {
	process *p = findProcess( result );
	sig = WSTOPSIG(*status);
	if(sig == SIGTRAP && ( !p->reachedVeryFirstTrap || p->inExec ))
	  ;
	else if( sig != SIGSTOP && sig != SIGTRAP ) {
	  ignore = true;
	  if( P_ptrace(PT_CONTINUE, result, (void *)1, sig, 0) == -1 ) {
	    if( errno == ESRCH ) {
	      cerr << "WARNING -- process does not exist, constructing exit code" << endl;
	      //*status = W_EXITCODE(0,sig);
	      ignore = false;
	    } else
	      cerr << "ERROR -- process::waitProcs forwarding signal " << sig << " -- " << sys_errlist[errno] << endl;
	  } // P_ptrace(PT_CONTINUE)
	} // else if (sig != SIGSTOP && sig != SIGTRAP)
      } // result > 0, WIFSTOPPED(status)
  } while ( ignore );
  
  // This is really a hack. Shouldn't whoever is calling
  // waitProcs handle the dyninst trap/new shared object?
  if( result > 0 ) {
    if( WIFSTOPPED(*status) ) {
      process *curr = findProcess( result );
      if(curr == 0) { 
	return result;
      }  // was it a forked child process?
      if (!curr->dyninstLibAlreadyLoaded() && curr->wasCreatedViaAttach())
	{
	  /* FIXME: Is any of this code ever executed? */
	  // make sure we are stopped in the eyes of paradynd - naim
	  bool wasRunning = (curr->status() == running);
	  if (curr->status() != stopped)
	    curr->Stopped();   
	  if(curr->isDynamicallyLinked()) {
	    curr->handleIfDueToSharedObjectMapping();
	  }
	  if (curr->trapDueToDyninstLib()) {
	    // we need to load libdyninstRT.so.1 - naim
	    curr->handleIfDueToDyninstLib();
	  }
	  if (wasRunning) 
	    if (!curr->continueProc()) assert(0);
	}

		  //ccw 30 apr 2002 : SPLIT4  
#if !defined(BPATCH_LIBRARY)
		else if (!curr->paradynLibAlreadyLoaded() && curr->wasCreatedViaAttach()){
			  /* FIXME: Is any of this code ever executed? */
			  // make sure we are stopped in the eyes of paradynd - naim
			  bool wasRunning = (curr->status() == running);
			  if (curr->status() != stopped)
				  curr->Stopped();   
			  if(curr->isDynamicallyLinked()) {
				  curr->handleIfDueToSharedObjectMapping();
			  }
			  if (curr->trapDueToParadynLib()) {
				  // we need to load libdyninstRT.so.1 - naim
				  curr->handleIfDueToDyninstLib();
			  }
			  if (wasRunning) 
				  if (!curr->continueProc()) assert(0);

		}
#endif

    }
  }// else if( errno )
  //perror( "process::waitProcs - waitpid" );
  return result;
}


// attach to an inferior process.
bool process::attach() {
  // Create the default LWP
  dyn_lwp *lwp = new dyn_lwp(0, this);
  if (!lwp->openFD()) {
    delete lwp;
    return false;
  }
  
  lwps[0] = lwp;
  // we only need to attach to a process that is not our direct children.
#ifdef BPATCH_LIBRARY
  if (parent != 0 || createdViaAttach) {
    if (!attach_())
      return false;
    // Get the initial trap
    bool gotTrap = false;
    while (!gotTrap) {
      int waitStatus;
      int ret = waitpid(pid, &waitStatus, WUNTRACED);
      if ((ret == -1) && (errno == EINTR)) continue;
      if ((ret == -1) && (errno == ECHILD)) return false;
      if(WIFEXITED(waitStatus)) {
        // the child is gone.
        //status_ = exited;
        handleProcessExit(this, WEXITSTATUS(waitStatus));
        return false;
      }
      if (!WIFSTOPPED(waitStatus) && !WIFSIGNALED(waitStatus))
        return false;
      int sig = WSTOPSIG(waitStatus);
      if (sig != SIGTRAP) {
        extern int handleSigChild(int, int);
        if (handleSigChild(pid, waitStatus) < 0) 
	  cerr << "handleSigChild failed for pid " << pid << endl; 
      } else {              //Process stopped by our attach
	gotTrap = TRUE;
      }
    }
    return true;
  } else
    return true;
#else

	//ccw 30 apr 2002 : SPLIT5
 string buffer ="attach!";
   statusLine(buffer.c_str());

  if (parent != 0 || createdViaAttach) {
    return attach_();
  }
  else
    return true;
#endif
}

bool process::attach_() {
   // formerly OS::osAttach()
   int ret = ptrace(PT_ATTACH, getPid(), (int *)0, 0, 0);
   if (ret == -1)
      ret = ptrace(PT_REATT, getPid(), (int *)0, 0, 0);
   return (ret != -1);
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

  if (!checkStatus()) 
    return false;
  ptraceOps++; ptraceOtherOps++;

/* Choose either one of the following methods to continue a process.
 * The choice must be consistent with that in process::continueProc_ and stop_
 */

#ifndef PTRACE_ATTACH_DETACH
  if (!ptraceKludge::deliverPtrace(this, PT_CONTINUE, (char*)1, 0, NULL))
    ret = -1;
  else
    ret = 0;
  // switch these to not detach after every call.
  //ret = ptrace(PT_CONTINUE, pid, (int *)1, 0, NULL);
#else
  ret = ptrace(PT_DETACH, pid, (int *)1, 0, NULL);
#endif

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
  if (status() != neonatal && !wasStopped)
    return (loopUntilStopped());
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

#ifdef BPATCH_LIBRARY
bool process::API_detach_(const bool cont) {
  if (!checkStatus())
      return false;
  ptraceOps++; ptraceOtherOps++;
  return (ptraceKludge::deliverPtrace(this,PT_DETACH,(char*)1, cont ? 0 : SIGSTOP,NULL));
}
#endif

// temporarily unimplemented, PT_DUMPCORE is specific to sunos4.1
bool process::dumpCore_(const string coreFile) {
  if (!checkStatus()) 
    return false;
  ptraceOps++; ptraceOtherOps++;

#ifdef BPATCH_LIBRARY
  if (!dumpImage(coreFile)) {
#else
  if (!dumpImage()) {
//  if (!OS::osDumpImage(symbols->file(), pid, symbols->codeOffset()))
#endif
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

#ifdef BPATCH_SET_MUTATIONS_ACTIVE
bool process::readTextSpace_(void *inTraced, u_int amount, const void *inSelf) {
  return readDataSpace_(inTraced, amount, const_cast<void *>(inSelf));
}
#endif

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
    perror("Failed write2");
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

bool process::loopUntilStopped() {
  /* make sure the process is stopped in the eyes of ptrace */
  stop_();     //Send the process a SIGSTOP
  bool isStopped = false;
  int waitStatus;
  while (!isStopped) {
    int ret = waitpid(pid, &waitStatus, WUNTRACED);
    if ((ret == -1) && (errno == EINTR)) continue;
    // these two ifs (ret==-1&&errno==ECHILD)||(WIF..) used to be together
    // but had to seperate them, we were receiving ECHILD in a different
    // situation, for some reason..
    // if ((ret == -1 && errno == ECHILD) || (WIFEXITED(waitStatus))) {
    if ((ret == -1) && (errno == ECHILD)) return true;
    if(WIFEXITED(waitStatus)) {
      // the child is gone.
      status_ = exited;
      handleProcessExit(this, WEXITSTATUS(waitStatus));
      return(false);
    }
    if (!WIFSTOPPED(waitStatus) && !WIFSIGNALED(waitStatus)) {
      printf("problem stopping process\n");
      return false;
    }
    int sig = WSTOPSIG(waitStatus);
    if ((sig == SIGTRAP) || (sig == SIGSTOP) || (sig == SIGINT)) {
      if (sig != SIGSTOP) { //Process already stopped, but not by our SIGSTOP
        extern int handleSigChild(int, int);
        if (handleSigChild(pid, waitStatus) < 0) 
	  cerr << "handleSigChild failed for pid " << pid << endl; 
	// The process might be stopped after the handleSigChild is
	// done (e.g. by an inferior RPC which completed in a paused state).
	// In this case, we want to return immediately
	// Idea: restart the process and let it get the pause signal? Hrm.
	//if (status_ == stopped) isStopped = true;
	if (status_ == stopped) continueProc();
      } else {              //Process stopped by our SIGSTOP
	isStopped = true;
      }
    } else {
      if (ptrace(PT_CONTINUE, pid, (int*)1, sig, 0) == -1) {
	perror("Ptrace error in PT_CONTINUE");
	logLine("Ptrace error in PT_CONTINUE, loopUntilStopped\n");
        return false;
      }
    }
  }

  return true;
}


//
// Write out the current contents of the text segment to disk.  This is useful
//    for debugging dyninst.
//
#ifdef BPATCH_LIBRARY
bool process::dumpImage(string outFile) {
#else
bool process::dumpImage() {
#endif
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
#ifndef BPATCH_LIBRARY
    char outFile[256];
#endif
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
#ifdef BPATCH_LIBRARY
    ofd = open(outFile.c_str(), O_WRONLY|O_CREAT, 0777);
#else
    sprintf(outFile, "%s.real", imageFileName.c_str());
    sprintf(errorLine, "Saving program to %s\n", outFile);
    logLine(errorLine);

    ofd = open(outFile, O_WRONLY|O_CREAT, 0777);
#endif
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

extern bool completeTheFork(process *, int);
   // in inst-power.C since class instPoint is too

void process_whenBothForkTrapsReceived(process *parent, int childPid) {
   forkexec_cerr << "welcome to: process_whenBothForkTrapsReceived" << endl;
   forkexec_cerr << "calling completeTheFork" << endl;

   if (!completeTheFork(parent, childPid))
      assert(false);

   parent->resetForkTrapData();

   // let's continue both processes
   if (!parent->continueProc())
      assert(false);

   // Note that we use PT_CONTINUE on the child instead of kill(pid, SIGSTOP).
   // Is this right?  (I think so)

//   int ret = ptrace(PT_CONTINUE, pidForChild, (int*)1, 0, 0);
   int ret = ptrace(PT_CONTINUE, childPid, (int*)1, SIGCONT, 0);
   if (ret == -1)
      assert(false);

   forkexec_cerr << "leaving process_whenBothForkTrapsReceived (parent & "
                 << "child running and should execute DYNINSTfork soon)"
                 << endl;
}

     //////////////////////////////////////////////////////////////////////////
     //Restore the base trampolines after they have been cleared by an AIX load
     //
void resurrectBaseTramps(process *p)
{
  if (! p) return;

  vector<const instPoint*> allInstPoints = p->baseMap.keys();

  extern void findAndReinstallBaseTramps(process *, vector<const instPoint*> &);
  findAndReinstallBaseTramps(p, allInstPoints);

  vector<process::mtListInfo> allMTlistsInfo;
  p->getMiniTrampLists(&allMTlistsInfo);

  extern void reattachMiniTramps(process *, 
			   const vector<process::mtListInfo> &allMTlistsInfo);
  reattachMiniTramps(p, allMTlistsInfo);
}


bool handleAIXsigTraps(int pid, int status) {
    process *curr = findProcess(pid); // NULL for child of a fork
    // see man page for "waitpid" et al for descriptions of constants such
    // as W_SLWTED, W_SFWTED, etc.
    if (WIFSTOPPED(status) && (WSTOPSIG(status)==SIGTRAP)
	&& ((status & 0x7f) == W_SLWTED)) {
      //Process is stopped on a load.  AIX has reloaded the process image and
      //   the text segment heap has been cleared.
      if (curr) {
	curr->status_ = stopped;
        //fprintf(stderr, "Got load SIGTRAP from pid %d, PC=%x\n", pid,
	//curr->currentPC());
        resurrectBaseTramps(curr);            //Restore base trampolines

	// We've loaded a library. Handle it as a dlopen(), basically
	// Actually, handle it exactly like a dlopen. 
	curr->handleIfDueToSharedObjectMapping(); 
	curr->continueProc();
      }
      return true;
    } // W_SLWTED (stopped-on-load)

    // On AIX the processes will get a SIGTRAP when they execute a fork.
    // (Both the parent and the child will get this TRAP).
    // we must check for the SIGTRAP here, and handle the fork.
    // On aix the instrumentation on the parent will not be duplicated on 
    // the child, so we need to insert instrumentation again.

    if (WIFSTOPPED(status) && WSTOPSIG(status)==SIGTRAP 
	&& ((status & 0x7f) == W_SFWTED)) {
      if (curr) {
	// parent process.  Stay stopped until the child process has completed
	// calling "completeTheFork()".
	forkexec_cerr << "AIX: got fork SIGTRAP from parent process " 
		      << pid << "\n";
	curr->status_ = stopped;
	curr->receivedForkTrapForParent();
	
	int childPid;
	if(curr->readyToCopyInstrToChild(&childPid))
	  process_whenBothForkTrapsReceived(curr, childPid);
	return true;
      } else {
	// child process
	forkexec_cerr << "AIX: got SIGTRAP from forked (child) process " 
		      << pid << "\n";

	// get process info
	struct procsinfo psinfo;
	pid_t temp_child_pid = pid;
	pid_t child_pid = pid;
	if (getprocs(&psinfo, sizeof(psinfo), NULL, 0, &temp_child_pid, 1) 
	    == -1) {
	  assert(false);
	  return false;
	}

	assert((pid_t)psinfo.pi_pid == pid);

	int parentPid = psinfo.pi_ppid;
	process *parent = findProcess(parentPid); // NULL for child of a fork

	//string str = string("Parent of process ") + string(pid) + " is " +
	//               string(psinfo.pi_ppid) + "\n";
	//logLine(str.c_str());

	parent->receivedForkTrapForChild((int)child_pid);

	int childPid;
	if(parent->readyToCopyInstrToChild(&childPid)) {
	  assert(childPid == child_pid);
	  if(parent->status() == running) {
	    if(! parent->pause())   assert(false);
	  }
	  process_whenBothForkTrapsReceived(parent, childPid);
	}
        return true;
      } // child process
    } //  W_SFWTED (stopped-on-fork)
    return false;
}

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
  if (lwp_) {
      unsigned allRegs[64];
      P_ptrace(PTT_READ_GPRS, lwp_, (int *)allRegs, 0, 0); // aix 4.1 likes int *
      return allRegs[returnValReg];
  }
  else
    return P_ptrace(PT_READ_GPR, proc_->getPid(), (void *)returnValReg, 0, 0);
}

bool process::set_breakpoint_for_syscall_completion() {
   // We don't know how to do this on AIX
   return false;
}

void process::clear_breakpoint_for_syscall_completion() { return; }

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
   if (lwp_ > 0) 
      lwp_to_use = lwp_;
   else {
      /* If we need to get the data for the entire process (ie. lwp_ == 0)
         then we need to the pm_get_data_group function requires any
         lwp in the group, so we'll grab the lwp of any active thread in the
         process */
      struct thrdsinfo thrd_buf[10]; // max 10 threads
      getthrds(proc_->getPid(), thrd_buf, sizeof(struct thrdsinfo), 0, 1);
      lwp_to_use = thrd_buf[0].ti_tid;
   }
  
   // PM counters are only valid when the process is paused. 
   bool needToCont = (proc_->status() == running);
   if(proc_->status() == running) {
      proc_->pause();
   }

   pm_data_t data;
   if(lwp_ > 0) 
      ret = pm_get_data_thread(proc_->getPid(), lwp_to_use, &data);
   else   // lwp == 0, means get data for the entire process (ie. all lwps)
      ret = pm_get_data_group(proc_->getPid(), lwp_to_use, &data);
   if (ret) {    
       pm_error("dyn_lwp::getRawCpuTime_hw: pm_get_data_thread", ret);
       fprintf(stderr, "Attempted pm_get_data(%d, %d, %d)\n",
               proc_->getPid(), lwp_, lwp_to_use);
       
   }
   rawTime64 result = data.accu[get_hwctr_binding(PM_CYC_EVENT)];

   // Continue the process
   if(needToCont) {
      proc_->continueProc();
   }

   //if(pos_junk != 101)
   //  ct_record(pos_junk, result, hw_previous_, lwp_, lwp_to_use);

   if(result < hw_previous_) {
      cerr << "rollback in dyn_lwp::getRawCpuTime_hw, lwp_to_use: " 
           << lwp_to_use << ", lwp: " << lwp_ << ", result: " << result 
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
  
  if (lwp_ > 0) {
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
  if ((frame.getPC() < instFunc->addr()) || (frame.getPC() > instFunc->addr() + instFunc->size()))
    return true;

  const vector <instPoint *>exitPoints = instFunc->funcExits(this);
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
      if (P_ptrace(PTT_READ_SPRS, frame.getLWP()->get_lwp(), (int *)&spr_contents, 0, 0) == -1) {
	perror("Failed to read SPRS in catchupSideEffect");
	return false;
      }
      oldReturnAddr = spr_contents.pt_lr;
      spr_contents.pt_lr = (unsigned) exitTrampAddr;
      if (P_ptrace(PTT_WRITE_SPRS, frame.getLWP()->get_lwp(), (int *)&spr_contents, 0, 0) == -1) {
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

bool dyn_lwp::openFD()
{
  // Umm... no file descriptors on AIX
  return true;
}

void dyn_lwp::closeFD()
{
  return;
}
