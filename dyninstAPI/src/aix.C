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

// $Id: aix.C,v 1.72 2000/11/15 22:56:05 bernat Exp $

#include "common/h/headers.h"
#include "dyninstAPI/src/os.h"
#include "dyninstAPI/src/process.h"
#include "dyninstAPI/src/symtab.h"
#include "dyninstAPI/src/stats.h"
#include "common/h/Types.h"
#include "common/h/Dictionary.h"
#include "dyninstAPI/src/Object.h"
#include "dyninstAPI/src/instP.h" // class instInstance
#include "common/h/pathName.h"

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
#include <xcoff.h>
#include <scnhdr.h>
#include <sys/time.h>
#include <sys/reg.h>
#include <sys/user.h>
#include <sys/wait.h>
#include <sys/ptrace.h>
#include <procinfo.h> // struct procsinfo
#include <sys/types.h>

#define __AR_BIG__
#define __AR_SMALL__
#include <ar.h> // archive file format.

/* Getprocs() should be defined in procinfo.h, but it's not */
extern "C" {
extern int getprocs(struct procsinfo *ProcessBuffer,
		    int ProcessSize,
		    struct fdsinfo *FileBuffer,
		    int FileSize,
		    pid_t *IndexPointer,
		    int Count);
}

#include "dyninstAPI/src/showerror.h"
#include "common/h/debugOstream.h"

extern "C" {
extern int ioctl(int, int, ...);
};

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
void Frame::getActiveFrame(process *p)
{
    errno = 0;
    pc_ = P_ptrace(PT_READ_GPR, p->getPid(), (int *) IAR, 0, 0); // aix 4.1 likes int *
    if (errno != 0) return;

    errno = 0;
    fp_ = P_ptrace(PT_READ_GPR, p->getPid(), (int *) STKP, 0, 0); // aix 4.1 likes int *
    if (errno != 0) return;

    /* Read the first frame from memory.  The first frame pointer is
       in the memory location pointed to by $sp.  However, there is no
       $pc stored there, it's the place to store a $pc if the current
       function makes a call. */
    Frame dummy = getCallerFrame(p);
    fp_ = dummy.fp_;
}

bool process::needToAddALeafFrame(Frame, Address &){
    return false;
}


//
// given the pointer to a frame (currentFP), return 
//     (1) the saved frame pointer (fp)
//            NULL -> that currentFP is the bottom (last) frame.	
//     (2) the return address of the function for that frame (rtn).
//     (3) return true if we are able to read the frame.
//
Frame Frame::getCallerFrameNormal(process *p) const
{
  //
  // define the linkage area of an activation record.
  //    This information is based on the data obtained from the
  //    info system (search for link area). - jkh 4/5/96
  //

  struct {
    unsigned oldFp;
    unsigned savedCR;
    unsigned savedLR;
    unsigned compilerInfo;
    unsigned binderInfo;
    unsigned savedTOC;
  } linkArea;
  
  if (p->readDataSpace((caddr_t)fp_, sizeof(linkArea),
		       (caddr_t)&linkArea, false))
  {
    Frame ret;
    ret.fp_ = linkArea.oldFp;
    ret.pc_ = linkArea.savedLR;

    if (uppermost_) {
      // use the value stored in the link register instead.
      errno = 0;
      ret.pc_ = P_ptrace(PT_READ_GPR, p->getPid(), (int *)LR, 0, 0); // aix 4.1 likes int *
      if (errno != 0) return Frame(); // zero frame
    }

    return ret;
  }

  return Frame(); // zero frame
}

void *process::getRegisters() {
   // assumes the process is stopped (ptrace requires it)
   assert(status_ == stopped);

   const u_int num_bytes = 32 * 4 // 32 general purpose integer registers
                         + 32 * 8 // 32 floating point registers @ 8 bytes each
                         + 9 * 4; // 9 special registers
   // special registers are:
   // IAR (instruction address register)
   // MSR (machine state register)
   // CR (condition register)
   // LR (link register)
   // CTR (count register)
   // XER (fixed point exception)
   // MQ (multiply quotient)
   // TID
   // FPSCR (fp status)
   // FPINFO (fp info) [no, out of range of what ptrace can access (why?)]
   // FPSCRX (fp sreg ext.) [no, out of range, too]
   
   void *buffer = new char[num_bytes];
   assert(buffer);

   unsigned *bufferPtr = (unsigned *)buffer;

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

   for (unsigned i=0; i < 32; i++) {
      errno = 0;
      unsigned value = P_ptrace(PT_READ_GPR, pid, (void *)i, 0, 0);
      if (errno != 0) {
	 perror("ptrace PT_READ_GPR");
	 cerr << "regnum was " << i << endl;
	 return NULL;
      }

      *bufferPtr++ = value;
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

      errno = 0;
      P_ptrace(PT_READ_FPR, pid, &value,
	       FPR0 + i, // see <sys/reg.h>
	       0);
      if (errno != 0) {
	 perror("ptrace PT_READ_FPR");
	 cerr << "regnum was " << FPR0 + i << "; FPR0=" << FPR0 << endl;
	 return NULL;
      }

      memcpy(bufferPtr, &value, sizeof(value));

      unsigned *oldBufferPtr = bufferPtr;
      bufferPtr += 2; // 2 unsigned's --> 8 bytes
      assert((char*)bufferPtr - (char*)oldBufferPtr == 8); // just for fun

      assert(2*sizeof(unsigned) == 8);
   }

   // Finally, the special registers.
   // (See the notes on PT_READ_GPR above: pass reg # as 3d param, 4th & 5th ignored)
   // (Only reg numbered 0-31 or 128-136 are valid)
   const int special_register_codenums [] = {IAR, MSR, CR, LR, CTR, XER, MQ, TID, FPSCR};
      // see <sys/reg.h>; FPINFO and FPSCRX are out of range, so we can't use them!
   const u_int num_special_registers = 9;

   for (unsigned i=0; i < num_special_registers; i++) {
      errno = 0;
      unsigned value = P_ptrace(PT_READ_GPR, pid, (void *)special_register_codenums[i], 0, 0);
      if (errno != 0) {
	 perror("ptrace PT_READ_GPR for a special register");
	 cerr << "regnum was " << special_register_codenums[i] << endl;
	 return NULL;
      }

      *bufferPtr++ = value;
   }

   assert((unsigned)bufferPtr - (unsigned)buffer == num_bytes);

   // Whew, finally done.
   return buffer;
}

static bool executeDummyTrap(process *theProc) {
   assert(theProc->status_ == stopped);
   
   // Allocate a tempTramp. Assume there is text heap space available,
   // since otherwise we're up a creek.
   unsigned tempTramp = inferiorMalloc(theProc, 8, textHeap);
   assert(tempTramp);

   unsigned theInsns[2];
   theInsns[0] = BREAK_POINT_INSN;
   theInsns[1] = 0; // illegal insn, just to make sure we never exec past the trap
   if (!theProc->writeTextSpace((void *)tempTramp, sizeof(theInsns), &theInsns)) {
      cerr << "executeDummyTrap failed because writeTextSpace failed" << endl;
      return false;
   }

   errno = 0;
   unsigned oldpc = P_ptrace(PT_READ_GPR, theProc->getPid(), (void *)IAR, 0, 0);
   assert(errno == 0);

   errno = 0;
   P_ptrace(PT_WRITE_GPR, theProc->getPid(), (void *)IAR, tempTramp, 0);
   assert(errno == 0);

   if ((unsigned)P_ptrace(PT_READ_GPR, theProc->getPid(), (void *)IAR, 0, 0) != tempTramp) {
      cerr << "executeDummyTrap failed because PT_READ_GPR of IAR register failed" << endl;
      return false;
   }

   // We bypass continueProc() because continueProc() changes theProc->status_, which
   // we don't want to do here
   errno = 0;
   P_ptrace(PT_CONTINUE, theProc->getPid(), (void *)1, 0, 0);
      // what if there are any pending signals?  Don't we lose the chance to forward
      // them now?
   assert(errno == 0);

while (true) {
   int status, pid;
   do {
      pid = waitpid(theProc->getPid(), &status, WUNTRACED);
      if (pid == 0)
	 cerr << "waitpid returned special case of 0 (?)" << endl;
      else if (pid == -1) {
	 if (errno == ECHILD)
	    // the child has died
	    assert(false);
	 else
	    perror("executeDummyTrap waitpid()");
      }
      else {
//	 cerr << "waitpid result was " << pid << endl;
//	 if (pid == theProc->getPid())
//	    cerr << "which was the pid, as expected" << endl;
      }

//      cerr << "did a wait" << endl; cerr.flush();
   } while (pid != theProc->getPid());

   if (WIFEXITED(status))
      // the child died
      assert(false);

   assert(WIFSTOPPED(status));

   int sig = WSTOPSIG(status);
   if (sig == SIGTRAP) {
#ifdef INFERIOR_RPC_DEBUG
      cerr << "executeDummyTrap: got SIGTRAP, as expected!" << endl;
#endif
      break;
   }
   else {
      // handle an 'ordinary' signal, e.g. SIGALRM
#ifdef INFERIOR_RPC_DEBUG
      cerr << "executeDummyTrap: got unexpected signal " << sig << "...ignoring" << endl;
#endif

      // We bypass continueProc() because continueProc() changes theProc->status_, which
      // we don't want to do here
      errno = 0;
      P_ptrace(PT_CONTINUE, theProc->getPid(), (void *)1, 0, 0);
         // what if there are any pending signals?  Don't we lose the chance to forward
         // them now?
      assert(errno == 0);

//      extern int handleSigChild(int, int);
//      (void)handleSigChild(pid, status);
   }
}

   // Restore the old PC register value
   errno = 0;
   P_ptrace(PT_WRITE_GPR, theProc->getPid(), (void *)IAR, oldpc, 0);
   assert(errno == 0);

   // delete the temp tramp now (not yet implemented)

#ifdef INFERIOR_RPC_DEBUG
   cerr << "leaving executeDummyTrap now" << endl;
   cerr.flush();
#endif

   return true;
}

bool process::executingSystemCall() {
   // this is not implemented yet - naim 5/15/97
   return false;
}

bool process::changePC(Address loc) {
   return changePC(loc, NULL);
}

bool process::changePC(Address loc, const void *) {
   // compare to write_pc() of gdb (findvar.c)
   // 2d arg (saved regs) of this routine isn't needed for aix, since it
   // has the option to write just 1 register with a ptrace call.

   // Format of PT_WRITE_GPR call:
   // 3d param ('address'): the register to modify
   // 4th param ('data'): the value to store
   // 5th param ignored
   // Returns -1 on failure; else, returns the 'data' parameter
   // Errors:
   //    EIO: 3d param not a valid register; must be 0-31 or 128-136

#ifdef INFERIOR_RPC_DEBUG
   cerr << "welcome to changePC with loc=" << (void *)loc << endl;
#endif

// gdb hack: execute 1 dummy insn
#ifdef INFERIOR_RPC_DEBUG
   cerr << "changePC: about to exec dummy trap" << endl;
#endif
   if (!executeDummyTrap(this)) {
      cerr << "changePC failed because executeDummyTrap failed" << endl;
      return false;
   }

   errno = 0;
   P_ptrace(PT_WRITE_GPR, pid, (void *)IAR, loc, 0);
   if (errno) {
      perror("changePC (PT_WRITE_GPR) failed");
      return false;
   }

   // Double-check that the change was made by reading the IAR register
   errno = 0;
   if (P_ptrace(PT_READ_GPR, pid, (void *)IAR, 0, 0) != (int)loc) {
      cerr << "changePC failed because couldn't re-read IAR register" << endl;
      return false;
   }
   assert(errno == 0);

   return true;
}

bool process::restoreRegisters(void *buffer) {
   // assumes process is stopped (ptrace requires it)
   assert(status_ == stopped);

   // WARNING: gdb has indications that one should execute a dummy instr (breakpoint)
   // in order to let the kernel do housekeeping necessary to avoid corruption of
   // the user stack (rs6000-nat.c).  Should we worry about that?
   // gdb's method (exec_one_dummy_insn()) works as follows: put a breakpoint
   // instruction somewhere in the inferior, save the PC, write the PC to
   // this dummy instr location, do a ptrace PT_CONTINUE, wait() for the signal,
   // restore the PC, free up the breakpoint.  But again, why is this needed?

   unsigned *bufferPtr = (unsigned *)buffer;

   // First, the general-purpose registers:
   // Format for PT_WRITE_GPR:
   // 3d param ('address'): specifies the register (must be 0-31 or 128-136)
   // 4th param ('data'): specifies value to store
   // 5th param ignored.
   // Returns 3d param on success else -1 on error.
   // Errors:
   //    EIO: address must be 0-31 or 128-136

   for (unsigned i=GPR0; i <= GPR31; i++) {
      errno = 0;
      P_ptrace(PT_WRITE_GPR, pid, (void *)i, *bufferPtr++, 0);
      if (errno) {
	 perror("ptrace PT_WRITE_GPR");
	 cerr << "regnum was " << i << endl;
	 return false;
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
      errno = 0;
      P_ptrace(PT_WRITE_FPR, pid, (void *)bufferPtr, i, 0);
         // don't ask me why args 3,4 are reversed from the PT_WRITE_GPR case.,
         // or why param 4 is a ptr to data instead of just data.
      if (errno != 0) {
	 perror("ptrace PT_WRITE_FPR");
	 cerr << "regnum was " << i << endl;
	 return false;
      }

      const unsigned *oldBufferPtr = bufferPtr;
      bufferPtr += 2; // 2 unsigned's == 8 bytes
      assert((unsigned)bufferPtr - (unsigned)oldBufferPtr == 8); // just for fun
   } 

   // Finally, special registers:
   // Remember, PT_WRITE_GPR gives an EIO error if the reg num isn't in range 128-136
   const int special_register_codenums [] = {IAR, MSR, CR, LR, CTR, XER, MQ, TID, FPSCR};
      // I'd like to add on FPINFO and FPSCRX, but their code nums in <sys/reg.h> (138, 148)
      // make PT_WRITE_GPR give an EIO error...
   const u_int num_special_registers = 9;

   for (unsigned i=0; i < num_special_registers; i++) {
      errno = 0;
      P_ptrace(PT_WRITE_GPR, pid, (void *)(special_register_codenums[i]), *bufferPtr++, 0);
      if (errno != 0) {
	 perror("ptrace PT_WRITE_GPR for a special register");
	 cerr << "regnum was " << special_register_codenums[i] << endl;
	 return false;
      }
   }

   return true; // success
}


bool process::emitInferiorRPCheader(void *insnPtr, Address &baseBytes) {
   // TODO: write me!
   instruction *insn = (instruction *)insnPtr;
   Address baseInstruc = baseBytes / sizeof(instruction);

//   extern void generateBreakPoint(instruction &);
//   generateBreakPoint(insn[baseInstruc++]);
//   extern void generateIllegalInsn(instruction &);
//   generateIllegalInsn(insn[baseInstruc++]); 

   // MT_AIX: since we don't have a base-trampoline here, we need to save
   // registers before we can continue - naim
   instruction *tmp_insn = (instruction *) (&insn[baseInstruc]);
   extern void saveAllRegistersThatNeedsSaving(instruction *, Address &);
   saveAllRegistersThatNeedsSaving(tmp_insn,baseInstruc);

   // Convert back:
   baseBytes = baseInstruc * sizeof(instruction);

   return true;
}

// note: the following should be moved to inst-power.C, since it's
// specific to an instruction set and not an OS, right?
bool process::emitInferiorRPCtrailer(void *insnPtr, Address &baseBytes,
				     unsigned &breakOffset,
				     bool stopForResult,
				     unsigned &stopForResultOffset,
				     unsigned &justAfter_stopForResultOffset) {
   // The sequence we want is: (restore), trap, illegal,
   // where (restore) undoes anything done in emitInferiorRPCheader(), above.

   instruction *insn = (instruction *)insnPtr;
   Address baseInstruc = baseBytes / sizeof(instruction);

   extern void generateBreakPoint(instruction &);

   if (stopForResult) {
      generateBreakPoint(insn[baseInstruc]);
      stopForResultOffset = baseInstruc * sizeof(instruction);
      baseInstruc++;

      justAfter_stopForResultOffset = baseInstruc * sizeof(instruction);
   }

   // MT_AIX: restoring previously saved registers - naim
   instruction *tmp_insn = (instruction *) (&insn[baseInstruc]);
   extern void restoreAllRegistersThatNeededSaving(instruction *, Address &);
   restoreAllRegistersThatNeededSaving(tmp_insn,baseInstruc);

   // Trap instruction (breakpoint):
   generateBreakPoint(insn[baseInstruc]);
   breakOffset = baseInstruc * sizeof(instruction);
   baseInstruc++;

   // And just to make sure that we don't continue, we put an illegal
   // insn here:
   extern void generateIllegalInsn(instruction &);
   generateIllegalInsn(insn[baseInstruc++]);

   baseBytes = baseInstruc * sizeof(instruction); // convert back

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

#ifdef PTRACE_ATTACH_DETACH
    if (ptrace(PT_CONTINUE, p->pid, (int *) 1, SIGCONT, NULL) == -1) {
#else
    //if (ptrace(PT_DETACH, p->pid, (int *) 1, SIGCONT, NULL) == -1) {
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
  return waitpid(0, status, options);
}
#else
int process::waitProcs(int *status) {
  return waitpid(0, status, WNOHANG);
}
#endif


// attach to an inferior process.
bool process::attach() {
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
  if (parent != 0) {
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

  errno = 0;
  (void) ptrace(PT_CONTINUE, pid, (int*)1, SIGBUS, NULL);
  assert(errno == 0);
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

bool process::writeDataSpace_(void *inTraced, u_int amount, const void *inSelf) {
  if (!checkStatus()) 
    return false;

  ptraceBytes += amount;

  while (amount > 1024) {
    ptraceOps++;
    if (!ptraceKludge::deliverPtrace(this, PT_WRITE_BLOCK, inTraced,
			     1024, const_cast<void*>(inSelf))) return false;
    amount -= 1024;
    inTraced = (char *)inTraced + 1024;
    inSelf = (char *)const_cast<void*>(inSelf) + 1024;
  }

  ptraceOps++;
  return ptraceKludge::deliverPtrace(this, PT_WRITE_BLOCK, inTraced,
			     amount, const_cast<void*>(inSelf));
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
      //status_ = exited;
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
    extern int errno;
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

    ifd = open(imageFileName.string_of(), O_RDONLY, 0);
    if (ifd < 0) {
      sprintf(errorLine, "Unable to open %s\n", imageFileName.string_of());
      logLine(errorLine);
      showErrorCallback(41, (const char *) errorLine);
      perror("open");
      return true;
    }

    rd = fstat(ifd, &statBuf);
    if (rd != 0) {
      perror("fstat");
      sprintf(errorLine, "Unable to stat %s\n", imageFileName.string_of());
      logLine(errorLine);
      showErrorCallback(72, (const char *) errorLine);
      return true;
    }
    length = statBuf.st_size;
#ifdef BPATCH_LIBRARY
    ofd = open(outFile.string_of(), O_WRONLY|O_CREAT, 0777);
#else
    sprintf(outFile, "%s.real", imageFileName.string_of());
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
        errno = 0;
        length = ((i + 1024) < aout.tsize) ? 1024 : aout.tsize -i;
        ptrace(PT_READ_BLOCK, pid, (int*) (baseAddr + i), length, (int *)buffer);
        if (errno) {
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

//
// Seek to the desired offset and read the passed length of the file
//   into dest.  If any errors are detected, log a message and return false.
//
bool seekAndRead(int fd, int offset, void **dest, int length, bool allocate)
{
    int cnt;

    if (allocate) {
	*dest = malloc(length);
    }

    if (!*dest) {
	sprintf(errorLine, "Unable to parse executable file: failed allocation, size %d\n", length);
	logLine(errorLine);
	showErrorCallback(42, (const char *) errorLine);
	return false;
    }

    cnt = lseek(fd, offset, SEEK_SET);
    if (cnt != offset) {
        sprintf(errorLine, "Unable to parse executable file: failed seek\n");
	logLine(errorLine);
	showErrorCallback(42, (const char *) errorLine);
	return false;
    }
    cnt = read(fd, *dest, length);
    if (cnt != length) {
        sprintf(errorLine, "Unable to parse executable file: failed read\n");
	logLine(errorLine);
	showErrorCallback(42, (const char *) errorLine);
	return false;
    }
    return true;
}

unsigned long roundup4(unsigned long val) {
   while (val % 4 != 0)
      val++;
   return val;
}

// Methods to read file and ar header for both small (32-bit) and
// large (64-bit) archive files. This lets us have a single archive
// parsing method.
int Archive_32::read_arhdr()
{
  char tmpstring[13];
  lseek(fd, 0, SEEK_SET);
  int cnt = read(fd, &filehdr, sizeof(struct fl_hdr));
  if (cnt != sizeof(struct fl_hdr))
    return -1;
  if (strncmp(filehdr.fl_magic, AIAMAG, SAIAMAG))
    return -1;
  strncpy(tmpstring, filehdr.fl_fstmoff, 12);
  tmpstring[12] = 0; first_offset = atol(tmpstring);
  if (first_offset % 2) first_offset++;
  strncpy(tmpstring, filehdr.fl_lstmoff, 12);
  tmpstring[12] = 0; last_offset = atol(tmpstring);
  if (last_offset % 2) last_offset++;
  next_offset = first_offset;
  // Offsets are always even
  return 0;
}

int Archive_64::read_arhdr()
{
  char tmpstring[21];
  lseek(fd, 0, SEEK_SET);
  int cnt = read(fd, &filehdr, sizeof(struct fl_hdr_big));
  if (cnt != sizeof(struct fl_hdr_big))
    return -1;
  if (strncmp(filehdr.fl_magic, AIAMAGBIG, SAIAMAG))
    return -1;
  strncpy(tmpstring, filehdr.fl_fstmoff, 21);
  tmpstring[21] = 0; first_offset = atol(tmpstring);
  if (first_offset % 2) first_offset++;
  strncpy(tmpstring, filehdr.fl_lstmoff, 21);
  tmpstring[21] = 0; last_offset = atol(tmpstring);
  if (last_offset % 2) last_offset++;
  next_offset = first_offset;

  return 0;
}

// Archive member header parsing function for 32/64-bit archives
// Pre:  takes an offset into the ar file which is the start point
//       of a member header
// Post: member_name contains the name of the file corresponding to
//       the member header
//       next_offset contains the offset to the next member header
//       or 0 if there are no members
//       aout_offset contains the offset to the file corresponding 
//       to the member name

int Archive_32::read_mbrhdr()
{
  int cnt;
  char tmpstring[13];

  if (next_offset == 0) return -1;
  lseek(fd, next_offset, SEEK_SET);
  // Don't read last two bytes (first two bytes of the name)
  cnt = read(fd, &memberhdr, sizeof(struct ar_hdr) - 2);
  if (cnt != (sizeof(struct ar_hdr) - 2)) return -1;
  strncpy(tmpstring, memberhdr.ar_namlen, 4);
  tmpstring[4] = 0; member_len = atol(tmpstring);
  if (member_name) free(member_name);
  member_name = (char *)malloc(member_len+1);
  cnt = read(fd, member_name, member_len);
  if (cnt != member_len) return -1;
  // Terminating null
  member_name[member_len] = 0;

  // Set the file offset for this member
  aout_offset = next_offset + sizeof(struct ar_hdr) + member_len;
  if (aout_offset % 2) aout_offset += 1;

  // Fix up next_offset
  if (next_offset == last_offset)
    next_offset = 0; // termination condition
  else {
    strncpy(tmpstring, memberhdr.ar_nxtmem, 12);
    tmpstring[12] = 0; next_offset = atol(tmpstring);
    if (next_offset % 2) next_offset++;
  }

  return 0;
}
// 64-bit function. Differences: structure size
// A lot of shared code. 
int Archive_64::read_mbrhdr()
{
  int cnt;
  char tmpstring[21];
  
  if (next_offset == 0) return -1;
  lseek(fd, next_offset, SEEK_SET);
  // Don't read last two bytes (first two bytes of the name)
  cnt = read(fd, &memberhdr, sizeof(struct ar_hdr_big) - 2);
  if (cnt != (sizeof(struct ar_hdr_big) - 2)) return -1;
  strncpy(tmpstring, memberhdr.ar_namlen, 4);
  tmpstring[4] = 0; member_len = atol(tmpstring);
  if (member_name) free(member_name);
  member_name = (char *)malloc(member_len+1);
  cnt = read(fd, member_name, member_len);
  if (cnt != member_len) return -1;
  // Terminating null
  member_name[member_len] = 0;

  // Set the file offset for this member
  aout_offset = next_offset + sizeof(struct ar_hdr_big) + member_len;
  if (aout_offset % 2) aout_offset += 1;

  // Fix up next_offset
  if (next_offset == last_offset)
    next_offset = 0; // termination condition
  else {
    strncpy(tmpstring, memberhdr.ar_nxtmem, 20);
    tmpstring[20] = 0; next_offset = atol(tmpstring);
    if (next_offset % 2) next_offset++;
  }

  return 0;
}

// This function parses a 32-bit XCOFF file, either as part of an
// archive (library) or from an a.out file. It takes an open file
// descriptor from which it reads its data. In addition, it loads the
// data segment from the pid given in AIX_PID_HACK, the pid of the
// running process. We have to do this because the data segment
// in memory is more correct than the one on disk. Specifically, the
// TOC for inter-module function calls is incorrect on disk, because
// offsets are calculated at runtime instead of link time.
//
// int fd: file descriptor for a.out object, not closed
// int offset: offset to begin reading at (for archive libraries)

// File parsing error macro, assumes errorLine defined
#define PARSE_AOUT_DIE(errType, errCode) { \
      sprintf(errorLine, "Error parsing a.out file %s(%s): %s\n", \
              file_.string_of(), member_.string_of(), errType); \
      statusLine(errorLine); \
      showErrorCallback(errCode,(const char *) errorLine); \
      goto cleanup; \
      }

void Object::parse_aout(int fd, int offset)
{
   // all these vrble declarations need to be up here due to the gotos,
   // which mustn't cross vrble initializations.  Too bad.
   long i,j;
   int err;
   int cnt;
   string name;
   unsigned value;
   int poolOffset;
   int poolLength;
   union auxent *aux;
   struct filehdr hdr;
   struct syment *sym;
   struct aouthdr aout;
   union auxent *csect;
   char *stringPool=NULL;
   Symbol::SymbolType type; 
   bool foundDebug = false;

   int *lengthPtr = &poolLength;
   struct syment *symbols = NULL;
   struct scnhdr *sectHdr = NULL;
   Symbol::SymbolLinkage linkage = Symbol::SL_UNKNOWN;
   unsigned toc_offset = 0;
   string modName;

   unsigned int nlines=0;
   int linesfdptr=0;
   struct lineno* lines=NULL;

   // Amounts to relocate symbol addresses
   unsigned text_reloc;
   unsigned data_reloc;

   // For reading process data space.
   unsigned ptrace_amount;
   char *in_self;
   char *in_traced;
   
   // Get to the right place in the file (not necessarily 0)
   lseek(fd, offset, SEEK_SET);

   // Load and check the XCOFF file header
   cnt = read(fd, &hdr, sizeof(struct filehdr));
   if (cnt != sizeof(struct filehdr))
     PARSE_AOUT_DIE("Reading file header", 49);

   if (hdr.f_magic == 0x1ef)
     {
       // XCOFF64 file! We don't handle those yet.
       cerr << "Unhandled XCOFF64 file" << endl;
       return;
     }
   
   if (hdr.f_magic != 0x1df)
     {
       fprintf(stderr, "Possible problem, magic number is %x, should be %x\n",
	       hdr.f_magic, 0x1df);
     }

   // Load and check the a.out (auxiliary) header
   cnt = read(fd, &aout, sizeof(struct aouthdr));
   if (cnt != sizeof(struct aouthdr)) 
     PARSE_AOUT_DIE("Reading a.out header", 49);

   // Load the section headers
   sectHdr = (struct scnhdr *) malloc(sizeof(struct scnhdr) * hdr.f_nscns);
   assert(sectHdr);
   cnt = read(fd, sectHdr, sizeof(struct scnhdr) * hdr.f_nscns);
   if ((unsigned) cnt != sizeof(struct scnhdr)* hdr.f_nscns)
     PARSE_AOUT_DIE("Reading section headers", 49);
   if (!seekAndRead(fd, hdr.f_symptr + offset, (void**) &symbols, 
                    hdr.f_nsyms * SYMESZ, true))
     PARSE_AOUT_DIE("Reading symbol table", 49);

   // Consistency check
   if ((unsigned) aout.text_start != sectHdr[aout.o_sntext-1].s_paddr)
     PARSE_AOUT_DIE("Checking text address", 49);
   if ((unsigned) aout.tsize != sectHdr[aout.o_sntext-1].s_size) 
     PARSE_AOUT_DIE("Checking text size", 49);
   if ((unsigned) aout.data_start != sectHdr[aout.o_sndata-1].s_paddr) 
     PARSE_AOUT_DIE("Checking data address", 49);
   if ((unsigned long) aout.dsize != sectHdr[aout.o_sndata-1].s_size)
     PARSE_AOUT_DIE("Checking data size", 49);

   /*
    * Get the string pool
    */
   poolOffset = hdr.f_symptr + hdr.f_nsyms * SYMESZ;
   /* length is stored in the first 4 bytes of the string pool */
   if (!seekAndRead(fd, poolOffset + offset, (void**) &lengthPtr, sizeof(int), false))
     PARSE_AOUT_DIE("Reading string pool size", 49);
   if (!seekAndRead(fd, poolOffset + offset, (void**) &stringPool, poolLength, true)) 
     PARSE_AOUT_DIE("Reading string pool", 49);

   /* find the text section such that we access the line information */
   for (i=0; i < hdr.f_nscns; i++)
     if (sectHdr[i].s_flags & STYP_TEXT) {
       nlines = sectHdr[i].s_nlnno;
       
       /* Some libraries have shown line numbers of 0 */
       if (nlines == 0)
	 continue;
       /* if there is overflow in the number of lines */
       if (nlines == 65535)
	 for (j=0; j < hdr.f_nscns; j++)
	   if ((sectHdr[j].s_flags & STYP_OVRFLO) &&
	       (sectHdr[j].s_nlnno == (i+1))){
	     nlines = (unsigned int)(sectHdr[j].s_vaddr);
	     break;
	   }
       
       /* read the line information table */
       if (!seekAndRead(fd,sectHdr[i].s_lnnoptr + offset,(void**) &lines,
			nlines*LINESZ,true))
	 PARSE_AOUT_DIE("Reading line information table", 49);

       linesfdptr = sectHdr[i].s_lnnoptr;
       break;
     }

   // Dyninst/Paradyn meanings
   // code_ptr_: location where mutator has the text segment in memory
   // text_reloc: that + value will get you a cup of coffee... the location in
   //               memory where that file's instructions start.
   //           = "text relocation value" = text_org + scnptr - text_start
   text_reloc = text_org_ + sectHdr[aout.o_sntext-1].s_scnptr - aout.text_start;
   // code_off_ is the value in memory such that code_ptr[x] == code_off_ + x
   code_off_ = text_org_ + sectHdr[aout.o_sntext-1].s_scnptr;
   code_len_ = aout.tsize;
   if (!seekAndRead(fd, roundup4(sectHdr[aout.o_sntext-1].s_scnptr) + offset,
		    (void **) &code_ptr_, aout.tsize, true))
     PARSE_AOUT_DIE("Reading text segment", 49);

   fprintf(stderr, "text_org_ = %x, scnptr = %x, text_start = %x\n",
	   (unsigned) text_org_, sectHdr[aout.o_sntext-1].s_scnptr, 
	   (unsigned) aout.text_start);
   fprintf(stderr, "Code pointer: %x, reloc: %x, offset: %x, length: %x\n",
	   (unsigned) code_ptr_, (unsigned) text_reloc,
	   (unsigned) code_off_, (unsigned) code_len_);

   // data_reloc = "relocation value" = data_org_ - aout.data_start
   data_reloc = data_org_ - aout.data_start;

   // We're forced to get the data segment through ptrace. While
   // some of the shared libraries are accessible from both the 
   // mutator and mutatee, all of them are not necessarily mapped. 
   data_ptr_ = (Word *)malloc(aout.dsize);
   ptrace_amount = aout.dsize;
   in_self = (char *)data_ptr_;
   // I've seen the data_org_ value start on a halfword-aligned boundary.
   // Since the first two bytes don't matter that I can tell, we round
   // to word alignment
   in_traced = (char *)(roundup4(data_org_));
   // Maximum ptrace block = 1k
   for (ptrace_amount = aout.dsize ; ptrace_amount > 1024; ptrace_amount -= 1024)
     {
       if (ptrace(PT_READ_BLOCK, pid_, (int *)in_traced,
		  1024, (int *)in_self) == -1)
	   PARSE_AOUT_DIE("Reading data segment", 49);

       in_self += 1024;
       in_traced += 1024;
     }
   if (ptrace(PT_READ_BLOCK, pid_, (int *)in_traced,
	      ptrace_amount, (int *)in_self) == -1)
     PARSE_AOUT_DIE("Reading data segment", 49);

   // data_off_ is the value subtracted from an (absolute) address to
   // give an offset into the mutator's copy of the data
   data_off_ = data_org_;

   fprintf(stderr, "data_org_ = %x, scnptr = %x, data_start = %x\n",
	   (unsigned) data_org_, sectHdr[aout.o_sndata-1].s_scnptr, 
	   (unsigned) aout.data_start);

   data_len_ = aout.dsize;

   fprintf(stderr, "Data pointer: %x, reloc: %x, offset: %x, length: %x\n",
	   (unsigned) data_ptr_, (unsigned) data_reloc, 
	   (unsigned) data_off_, (unsigned) data_len_);

   foundDebug = false;

   // Find the debug symbol table.
   for (i=0; i < hdr.f_nscns; i++)
     if (sectHdr[i].s_flags & STYP_DEBUG) {
	 foundDebug = true;
	 break;
       }

   if (foundDebug) 
     {
       stabs_ = (long unsigned int) symbols;
       nstabs_ = hdr.f_nsyms;
       stringpool_ = (long unsigned int) stringPool;
       if (!seekAndRead(fd, roundup4(sectHdr[i].s_scnptr + offset),
			(void **) &stabstr_, sectHdr[i].s_size, true))
	 PARSE_AOUT_DIE("Reading initialized debug section", 49);
       linesptr_ = (long unsigned int) lines;
       nlines_ = (int)nlines; 
       linesfdptr_ = linesfdptr;
   }
   else
     {
       // Not all files have debug information. Libraries tend not to.
       stabs_ = 0;
       nstabs_ = 0;
       stringpool_ = 0;
       stabstr_ = 0;
       linesptr_ = 0;
       nlines_ = 0;
       linesfdptr_ = 0;
     }

   // Now the symbol table itself:
   for (i=0; i < hdr.f_nsyms; i++) {
     /* do the pointer addition by hand since sizeof(struct syment)
      *   seems to be 20 not 18 as it should be */
     sym = (struct syment *) (((unsigned) symbols) + i * SYMESZ);
     if (sym->n_sclass & DBXMASK)
       continue;

     if ((sym->n_sclass == C_HIDEXT) || 
	 (sym->n_sclass == C_EXT) ||
	 (sym->n_sclass == C_FILE)) {
       if (!sym->n_zeroes) {
	 name = string(&stringPool[sym->n_offset]);
       } else {
	 char tempName[9];
	 memset(tempName, 0, 9);
	 strncpy(tempName, sym->n_name, 8);
	 name = string(tempName);
       }
     }
     
     if ((sym->n_sclass == C_HIDEXT) || (sym->n_sclass == C_EXT)) {
       if (sym->n_sclass == C_HIDEXT) {
	 linkage = Symbol::SL_LOCAL;
       } else {
	 linkage = Symbol::SL_GLOBAL;
       }
       
       if (sym->n_scnum == aout.o_sntext) {
	 type = Symbol::PDST_FUNCTION;
	 value = sym->n_value + text_reloc;
       } else {
	 // bss or data
	 csect = (union auxent *)
	   ((char *) sym + sym->n_numaux * SYMESZ);
	 
	 if (csect->x_csect.x_smclas == XMC_TC0) { 
	   if (toc_offset)
	     logLine("Found more than one XMC_TC0 entry.");
	   toc_offset = sym->n_value + data_reloc;
	   continue;
	 }
	 
	 if ((csect->x_csect.x_smclas == XMC_TC) ||
	     (csect->x_csect.x_smclas == XMC_DS)) {
	   // table of contents related entry not a real symbol.
	   //dump << " toc entry -- ignoring" << endl;
	   continue;
	 }
	 type = Symbol::PDST_OBJECT;
	 value = sym->n_value + data_reloc;
       }
       
       // skip .text entries
       if (name == ".text") continue;
       if (name.prefixed_by(".")) {
	 // XXXX - Hack to make names match assumptions of symtab.C
	 name = string(name.string_of()+1);
       }
       else if (type == Symbol::PDST_FUNCTION) {
	 // text segment without a leading . is a toc item
	 //dump << " (no leading . so assuming toc item & ignoring)" << endl;
	 continue;
       }
       
       unsigned int size = 0;
       if (type == Symbol::PDST_FUNCTION) {
	 // Find address of inst relative to code_ptr_, instead of code_off_
	 Word *inst = (Word *)((char *)code_ptr_ + value - code_off_);
	 while (inst[size] != 0) size++;
	 size *= sizeof(Word);
       }

       // AIX linkage code appears as a function. Since we don't remove it from
       // the whereaxis yet, I append a _linkage tag to each so that they don't
       // appear as duplicate functions
       // Module glink.s is renamed to Global_Linkage below
       if (modName == "Global_Linkage")
	 name += "_linkage";

       Symbol sym(name, modName, type, linkage, value, false, size);
       
       // If we don't want the function size for some reason, comment out
       // the above and use this:
       // Symbol sym(name, modName, type, linkage, value, false);
       
       symbols_[name] = sym;
       
       if (symbols_.defines(modName)) {
	 // Adjust module's address, if necessary, to ensure that it's <= the
	 // address of this new symbol
	 Symbol &mod_symbol = symbols_[modName];
	 if (value < mod_symbol.addr()) {
	   //cerr << "adjusting addr of module " << modName
	   //     << " to " << value << endl;
	   mod_symbol.setAddr(value);
	 }
       }
     } else if (sym->n_sclass == C_FILE) {
       if (!strcmp(name.string_of(), ".file")) {
	 int j;
	 /* has aux record with additional information. */
	 for (j=1; j <= sym->n_numaux; j++) {
	   aux = (union auxent *) ((char *) sym + j * SYMESZ);
	   if (aux->x_file._x.x_ftype == XFT_FN) {
	     // this aux record contains the file name.
	     if (!aux->x_file._x.x_zeroes) {
	       name = 
		 string(&stringPool[aux->x_file._x.x_offset]);
	     } else {
	       // x_fname is 14 bytes
	       char tempName[15];
	       memset(tempName, 0, 15);
	       strncpy(tempName, aux->x_file.x_fname, 14);
	       name = string(tempName);
	     }
	   }
	 }
       }
       //dump << "found module \"" << name << "\"" << endl;

       // Hack time. Break it down
       // Problem: libc and others show up as file names. So if the
       // file being loaded is a .a (it's a hack, remember?) use the
       // .a as the modName instead of the symbol we just found.
       if ((file_.suffixed_by("libc.a")) ||
	   (file_.suffixed_by("libcrypt.a")) ||
	   (file_.suffixed_by("libm.a")) ||
	   // MPI libraries
	   (file_.suffixed_by("libppe.a")) ||
	   (file_.suffixed_by("libmpci.a")) ||
	   (file_.suffixed_by("libmpi.a")) ||
	   (file_.suffixed_by("libvtd.a")) ||
	   // Paradyn/Dyninst runtime libs
	   (file_.suffixed_by("libdyninstAPI_RT.a")) ||
	   (file_.suffixed_by("libdyninstRT.a")))
	 modName = file_;
       else if (name == "glink.s")
	 modName = string("Global_Linkage");
       else
	 modName = name;
       
       const Symbol modSym(modName, modName, 
			   Symbol::PDST_MODULE, linkage,
			   UINT_MAX, // dummy address for now!
			   false);
       symbols_[modName] = modSym;
       
       continue;
     }
   }
   
   toc_offset_ = toc_offset;
      
 cleanup:

   if (sectHdr) free(sectHdr);
   if (stringPool && !foundDebug) free(stringPool);
   if (symbols && !foundDebug) free(symbols);
   if (lines && !foundDebug) free(lines);
   
   return;
}

// Archive parsing
// Libraries on AIX can be archive files. These files are distinguished
// by their magic number, and come in two types: 32-bit (small) and 
// 64-bit (big). The structure of an either archive file is similar:
// <Archive header> (magic number, offsets)
// <Member header>  (member file name)
//   <member file>
// <Member header>
//   <member file> 
// and so on. Given a member name, we scan the archive until we find
// that name, at which point parse_aout is called.
// The only difference between small and big archive is the size of
// the archive/member header variables (12 byte vs. 20 byte).
// Both archives are handled in one parsing function, which keys off
// the magic number of the archive.
// Note: all data in the headers is in ASCII.

// More macros
#define PARSE_AR_DIE(errType, errCode) { \
      sprintf(errorLine, "Error parsing archive file %s: %s\n", \
              file_.string_of(), errType); \
      statusLine(errorLine); \
      showErrorCallback(errCode,(const char *) errorLine); \
      return; \
      }

void Object::load_archive(int fd)
{
  Archive *archive;
  
  // Determine archive type
  lseek(fd, 0, SEEK_SET);
  char magic_number[SAIAMAG];
  int cnt = read(fd, magic_number, SAIAMAG);
  if (cnt != SAIAMAG)
    PARSE_AR_DIE("Reading magic number", 49);
  if (!strncmp(magic_number, AIAMAG, SAIAMAG))
    archive = (Archive *) new Archive_32(fd);
  else if (!strncmp(magic_number, AIAMAGBIG, SAIAMAG))
    archive = (Archive *) new Archive_64(fd);
  else
    PARSE_AR_DIE("Unknown magic number", 49);

  if (archive->read_arhdr())
    PARSE_AR_DIE("Reading file header", 49);

  while (archive->next_offset != 0)
    {
      if (archive->read_mbrhdr())
	PARSE_AR_DIE("Reading member header", 49);
      
      if (!strncmp(archive->member_name, 
		  member_.string_of(), 
		  archive->member_len - 1))
	  break; // Got the right one
    }
  if (archive->next_offset) // found the right member
    {
      // At this point, we should be able to read the a.out 
      // file header. 
      parse_aout(fd, archive->aout_offset);
    }
  else
    fprintf(stderr, "Member name %s not found in archive %s!\n",
	    member_.string_of(), file_.string_of());
  return;
}

// This is our all-purpose-parse-anything function. 
// Takes a file and determines from the first two bytes the
// file type (archive or a.out). Assumes that two bytes are
// enough to identify the file format. 

void Object::load_object()
{
  // Load in an object (archive, object, .so)
  int fd = 0;
  unsigned char magic_number[2];
  int cnt;

  fd = open(file_.string_of(), O_RDONLY, 0);
  if (fd <0) {
    sprintf(errorLine, "Unable to open file %s\n", 
	    file_.string_of());
    statusLine(errorLine);
    showErrorCallback(27,(const char *) errorLine);
    return;
  }
  
  cnt = read(fd, magic_number, 2);
  
  if (cnt != 2) {
    sprintf(errorLine, "Error reading file %s\n", 
	    file_.string_of());
    statusLine(errorLine);
    showErrorCallback(49,(const char *) errorLine);
    close(fd);
    return;
  }
  
  // a.out file: magic number = 0x01df
  // archive file: magic number = 0x3c62 "<b", actually "<bigaf>"
  // or magic number = "<a", actually "<aiaff>"
  if (magic_number[0] == 0x01) {
    if (magic_number[1] == 0xdf)
      parse_aout(fd, 0);
    else 
      //parse_aout_64(fd, 0);
      fprintf(stderr, "Don't handle 64 bit files yet");
  }
  else if (magic_number[0] == '<') // archive of some sort
    load_archive(fd);
  else // Fallthrough
    { 
      sprintf(errorLine, "Bad magic number in file %s\n",
	      file_.string_of());
      statusLine(errorLine);
      showErrorCallback(49,(const char *) errorLine);
    }
  if (fd) close(fd);
  return;
}

// There are three types of "shared" files:
// archives (made with ar, end with .a)
// objects (ld -bM:SRE)
// new-style shared objects (.so)
// load_shared_object determines from magic number which to use
// since static objects are just a.outs, we can use the same
// function for all

Object::Object(const string file, void (*err_func)(const char *))
  : AObject(file, err_func) {
  cerr << "In illegal constructor Object(string, addr, func)" << endl;
  text_org_ = 0;
  data_org_ = 0;
  member_ = 0;
  pid_ = 0;
  load_object();
}

Object::Object(const Object& obj)
    : AObject(obj) {
  // Copy over org data
  // You know, this really should never be called, but be careful.
  text_org_ = obj.text_org_;
  data_org_ = obj.data_org_;
  pid_ = obj.pid_;
  load_object();
}

// For shared object files
Object::Object(const string file,Address addr,void (*err_func)(const char *))
    : AObject(file, err_func) {
  // Okay, interface limitation problems here. We're passed a 
  // library name (file) and a text relocation address (addr),
  // and we want a member name and data relocation address.
  // Tough.

  cerr << "In illegal constructor Object(string, addr, func)" << endl;

  text_org_ = addr;
  data_org_ = 0;
  member_ = 0;
  pid_ = 0;
  load_object();
}

// More general object creation mechanism
Object::Object(fileDescriptor *desc, void (*err_func)(const char *))
  : AObject(desc->file(), err_func) {
  // We're passed a descriptor object that contains everything needed.
  fileDescriptor_AIX *fda = (fileDescriptor_AIX *)desc;
  text_org_ = fda->addr();
  data_org_ = fda->data();
  member_ = fda->member();
  pid_ = fda->pid();
  load_object();
}

Object::~Object() 
{
  fprintf(stderr, "Deleting object 'cause we're done with it\n");
  // Cleanup memory, otherwise we'll have mega-leaks.
  if (code_ptr_) free(code_ptr_);
  if (data_ptr_) free(data_ptr_);

}

Object& Object::operator=(const Object& obj) {
    (void) AObject::operator=(obj);
    return *this;
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

    ret = 0;
    ret = ptrace(PT_LDINFO, pid, 
		 (int *) ptr, 1024 * sizeof(struct ld_info), (int *)ptr);

    if (ret != 0) {
      perror("failed to get loader information about process");
	statusLine("Unable to get loader info about process, application aborted");
	showErrorCallback(43, "Unable to get loader info about process, application aborted");
	return NULL;
    }

    // turn on 'multiprocess debugging', which allows ptracing of both the
    // parent and child after a fork.  In particular, both parent & child will
    // TRAP after a fork.  Also, a process will TRAP after an exec (after the
    // new image has loaded but before it has started to execute at all).
    // Note that turning on multiprocess debugging enables two new values to be
    // returned by wait(): W_SEWTED and W_SFWTED, which indicate stops during
    // execution of exec and fork, respectively.
    ptrace(PT_MULTI, pid, 0, 1, 0);

    // Set up and return the file descriptor. In this case we actually
    // return a fileDescriptor_AIX type (text/data org value, pid)
    string member = "";
    Address text_org = (Address) ptr->ldinfo_textorg;
    Address data_org = (Address) ptr->ldinfo_dataorg;

    fileDescriptor *desc = 
      (fileDescriptor *) new fileDescriptor_AIX(filename, member,
						text_org, data_org,
						pid);

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

// #include "paradynd/src/metric.h"
// #include "paradynd/src/costmetrics.h"

class instInstance;

// the following MUST become process member vrbles, since paradynd can have
// more than one process active doing a fork!!!
static bool seenForkTrapForParent = false;
static bool seenForkTrapForChild  = false;
static pid_t pidForParent;
static pid_t pidForChild;

extern bool completeTheFork(process *, int);
   // in inst-power.C since class instPoint is too

static void process_whenBothForkTrapsReceived() {
   assert(seenForkTrapForParent);
   assert(seenForkTrapForChild);

   forkexec_cerr << "welcome to: process_whenBothForkTrapsReceived" << endl;

   process *parent = findProcess(pidForParent);
   assert(parent);

   // complete the fork!

   forkexec_cerr << "calling completeTheFork" << endl;

   if (!completeTheFork(parent, pidForChild))
      assert(false);

   // let's continue both processes

   if (!parent->continueProc())
      assert(false);

   // Note that we use PT_CONTINUE on the child instead of kill(pid, SIGSTOP).
   // Is this right?  (I think so)

//   int ret = ptrace(PT_CONTINUE, pidForChild, (int*)1, 0, 0);
   int ret = ptrace(PT_CONTINUE, pidForChild, (int*)1, SIGCONT, 0);
   if (ret == -1)
      assert(false);

   seenForkTrapForParent = seenForkTrapForChild = false;

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

  vector<instInstance*> allInstInstances;             //Get all mini trampolines
  getAllInstInstancesForProcess(p, allInstInstances);

  extern void reattachMiniTramps(process *, vector<instInstance*> &);
  reattachMiniTramps(p, allInstInstances);
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
	//                curr->currentPC());
        resurrectBaseTramps(curr);            //Restore base trampolines
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
	forkexec_cerr << "AIX: got fork SIGTRAP from parent process " << pid << endl;

	curr->status_ = stopped;

	seenForkTrapForParent = true;
	pidForParent = curr->getPid();

	if (seenForkTrapForChild)
	   process_whenBothForkTrapsReceived();

	return true;
      } else {
	// child process
	forkexec_cerr << "AIX: got SIGTRAP from forked (child) process " << pid << endl;

	// get process info
	struct procsinfo psinfo;
	pid_t process_pid = pid;
	if (getprocs(&psinfo, sizeof(psinfo), NULL, 0, &process_pid, 1) == -1) {
	  assert(false);
	  return false;
	}

	assert((pid_t)psinfo.pi_pid == pid);

	//string str = string("Parent of process ") + string(pid) + " is " +
	//               string(psinfo.pi_ppid) + "\n";
	//logLine(str.string_of());

	seenForkTrapForChild = true;
	pidForChild = pid;
	if (seenForkTrapForParent) {
	   assert((pid_t) pidForParent == (pid_t) psinfo.pi_ppid);

	   process_whenBothForkTrapsReceived();
	}

        return true;
      } // child process
    } //  W_SFWTED (stopped-on-fork)

    return false;
}

string process::tryToFindExecutable(const string &progpath, int /*pid*/) {
   // returns empty string on failure

   if (progpath.length() == 0)
      return "";

   if (exists_executable(progpath)) // util lib
      return progpath;

   return ""; // failure
}

Address process::read_inferiorRPC_result_register(Register returnValReg) {
   return P_ptrace(PT_READ_GPR, pid, (void *)returnValReg, 0, 0);
}

bool process::set_breakpoint_for_syscall_completion() {
   // We don't know how to do this on AIX
   return false;
}

void process::clear_breakpoint_for_syscall_completion() { return; }

Address process::getTOCoffsetInfo(Address dest) const
{
  // We have an address, and want to find the module the addr is
  // contained in. Given the probabilities, we (probably) want
  // the module dyninst_rt is contained in. 
  // I think this is the right func to use

  if (symbols->findFunctionIn(dest, this))
    return (Address) (symbols->getObject()).getTOCoffset();

  if (shared_objects)
    for(u_int j=0; j < shared_objects->size(); j++)
      if (((*shared_objects)[j])->getImage()->findFunctionIn(dest, this))
	return (Address) (((*shared_objects)[j])->getImage()->getObject()).getTOCoffset();
  // Serious error! Assert?
  return 0;
}


#ifdef SHM_SAMPLING
rawTime64 process::getRawCpuTime_hw(int /*lwp_id*/) {
  return 0;
}

rawTime64 process::getRawCpuTime_sw(int /*lwp_id*/) {

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

  if (result < previous) // Time ran backwards?
    {
      char errLine[150];
      sprintf(errLine,"process::getRawCpuTime_sw - time going backwords in "
	              "daemon - cur: %lld, prev: %lld\n", result, previous);
      cerr << errLine;
      logLine(errLine);
      result = previous;
    }
  else previous=result;

  return result;

}
#endif


#if defined(USES_DYNAMIC_INF_HEAP)
static const Address branch_range = 0x01fffffc;
static const Address lowest_addr = 0x10000000;
// Looks like we can't touch the shared memory segment. I'm guessing also
// that 0xe... (kernel space) would be a bad idea, as would 0xf... (shared data)
static const Address highest_addr = 0xd0000000;

void inferiorMallocConstraints(Address near, Address &lo, 
			       Address &hi, inferiorHeapType type)
{
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
  else
    {
      switch (type)
	{
	case dataHeap:
	  // mmap, preexisting dataheap constraints
	  lo = 0x20000000;
	  hi = 0xcfffff00;
	  break;
	default:
	  // Wide constraints
	  lo = lowest_addr;
	  hi = highest_addr;
	  break;
	}
    }
}

void inferiorMallocAlign(unsigned &size)
{
     /* 32 byte alignment.  Should it be 64? */
  size = (size + 0x1f) & ~0x1f;
}
#endif

#ifndef BPATCH_LIBRARY
void process::initCpuTimeMgrPlt() {
  cpuTimeMgr->installLevel(cpuTimeMgr_t::LEVEL_TWO, &process::yesAvail, 
			   timeUnit::us(), timeBase::bNone(), 
			   &process::getRawCpuTime_sw, "DYNINSTgetCPUtime_sw");
}
#endif

