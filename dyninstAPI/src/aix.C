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

// $Id: aix.C,v 1.82 2001/08/20 22:23:46 bernat Exp $

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
#include <scnhdr.h>
#include <sys/time.h>
#include <sys/reg.h>
#include <sys/user.h>
#include <sys/wait.h>
#include <sys/ptrace.h>
#include <procinfo.h> // struct procsinfo
#include <sys/types.h>
#include <signal.h>
#include <dlfcn.h> // dlopen constants

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

#define DLOPEN_MODE (RTLD_NOW | RTLD_GLOBAL)

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
   /*
   if (!executeDummyTrap(this)) {
      cerr << "changePC failed because executeDummyTrap failed" << endl;
      return false;
   }
   */

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
	if( sig == SIGTRAP && ( !p->reachedVeryFirstTrap || p->inExec ) )
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
	  }
	}
      }
      } while ( ignore );

  // This is really a hack. Shouldn't whoever is calling
  // waitProcs handle the dyninst trap/new shared object?
  if( result > 0 ) {
    if( WIFSTOPPED(*status) ) {
      process *curr = findProcess( result );
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
    }
  }// else if( errno )
  //perror( "process::waitProcs - waitpid" );
  return result;
}


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

/* Note:
 * Writes are not forced to be synchronous with instruction fetch on the
 * PowerPC architecture. So this means that if we write to memory and then
 * try to execute those instructions, our writes may not have propagated
 * through. I've seen this happen with inferiorRPCs, and occasionally with
 * base/mini tramps, though less often. So we're going to explicitly
 * flush. Ref: PowerPC architecture, "PowerPC virtual environment arch",
 * p344
 */
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
  if (!ptraceKludge::deliverPtrace(this, PT_WRITE_BLOCK, inTraced,
				   amount, const_cast<void*>(inSelf)))
    return false;

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
	fprintf(stderr, ":  %s", (func->prettyName()).string_of());
      }
      if ((instr_temp > 0xd0000000) && (instr_temp < 0xe0000000)) {
	pd_Function *func;
	if (shared_objects)
	  for(u_int j=0; j < shared_objects->size(); j++) {
	    const image *img = ((*shared_objects)[j])->getImage();
	    func = img->findFuncByAddr((Address) instr_temp, this);
	    if (func)
	      fprintf(stderr, ":   %s", (func->prettyName()).string_of());
	  }
      }
      fprintf(stderr, "\n");
    }
#endif

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

void inferiorMallocConstraints(Address near, Address &lo, 
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

/*************************************************************************/
/***  Code to handle dlopen()ing the runtime library                   ***/
/***                                                                   ***/
/***  get_dlopen_addr() -- return the address of the dlopen function   ***/
/***  Address dyninstlib_brk_addr -- address of the breakpoint at the  ***/
/***                                 end of the RT init function       ***/
/***  Address main_brk_addr -- address when we switch to dlopen()ing   ***/
/***                           the RT lib                              ***/
/***  dlopenDYNINSTlib() -- Write the (string) name of the RT lib,     ***/
/***                        set up and execute a call to dlopen()      ***/
/***  trapDueToDyninstLib() -- returns true if trap addr is at         ***/
/***                          dyninstlib_brk_addr                      ***/
/***  trapAtEntryPointOfMain() -- see above                            ***/
/***  handleIfDueToDyninstLib -- cleanup function                      ***/
/***  handleTrapAtEntryPointOfMain -- cleanup function                 ***/
/***  insertTrapAtEntryPointOfMain -- insert a breakpoint at the start ***/
/***                                  of main                          ***/
/*************************************************************************/

/*
 * return true if current PC is equal to the dyninstlib_brk_addr
 * variable
 */

/*
 * return true if current PC is equal to the main_brk_addr
 * variable
 */

bool checkAllThreadsForBreakpoint(int pid, Address break_addr)
{
  // get the current PC. Ptrace call PTT_READ_SPRS, which requires a
  // kernel thread ID. Sheesh.

  // hey. Should we move the getPC part to somewhere else?
  
  // Get the list of kernel threads
  struct procsinfo pb;
  int targpid = pid;

  if ( getprocs (&pb, sizeof(struct procsinfo), 0, 0, &targpid, 1) != 1 )
    perror("trapAtEntryPointOfMain: unable to get process info");

  int num_thrds = pb.pi_thcount;

  // Get kernel thread(s)

  struct thrdsinfo thrd_buf[pb.pi_thcount];
  getthrds(pid, thrd_buf, sizeof(struct thrdsinfo),
	   0, // Start at first thread
	   num_thrds);

  // Now that we have the correct thread ID, ptrace the sucker
  struct ptsprs spr_contents;
  for ( int i; i < num_thrds; i++ )
    {
      int kernel_thread = thrd_buf[i].ti_tid;
      
      if (ptrace(PTT_READ_SPRS, kernel_thread, (int *)&spr_contents,
		 0, 0) != -1) 
	{
	  Address prog_counter = (Address) spr_contents.pt_iar;
	  
	  if ((Address) prog_counter == (Address) break_addr)
	    return true;
	}
    }

  return false;
}

bool process::trapDueToDyninstLib()
{
  // Since this call requires a PTRACE, optimize it slightly
  if (dyninstlib_brk_addr == 0x0) return false;
  return checkAllThreadsForBreakpoint(pid, dyninstlib_brk_addr);
}

bool process::trapAtEntryPointOfMain()
{
  // Since this call requires a PTRACE, optimize it slightly
  // This won't trigger (ever) if we are attaching, btw.
  if (main_brk_addr == 0x0) return false;

  return checkAllThreadsForBreakpoint(pid, main_brk_addr);
}

/*
 * Cleanup after dlopen()ing the runtime library. Since we don't overwrite
 * any existing functions, just restore saved registers. Cool, eh?
 */

void process::handleIfDueToDyninstLib()
{
  restoreRegisters(savedRegs);
  delete[] (char *)savedRegs;
  savedRegs = NULL;
  // We was never here.... 
  
  // But before we go, reset the dyninstlib_brk_addr so we don't
  // accidentally trigger it, eh?
  dyninstlib_brk_addr = 0x0;

}

/*
 * Restore "the original instruction" written into main so that
 * we can proceed after the trap. Saved in "savedCodeBuffer",
 * which is a chunk of space we use for dlopening the RT library.
 */

void process::handleTrapAtEntryPointOfMain()
{
  function_base *f_main = findOneFunction("main");
  assert(f_main);

  Address addr = f_main->addr();
  // Put back the original insn
  writeDataSpace((void *)addr, sizeof(instruction), 
		 (char *)savedCodeBuffer);

  // And zero out the main_brk_addr so we don't accidentally
  // trigger on it.
  main_brk_addr = 0x0;
}

/*
 * Stick a trap at the entry point of main. At this point,
 * libraries are mapped into the proc's address space, and
 * we can dlopen the RT library.
 */

void process::insertTrapAtEntryPointOfMain()
{
  function_base *f_main = findOneFunction("main");
  if (!f_main) {
    // we can't instrument main - naim
    showErrorCallback(108,"main() uninstrumentable");
    extern void cleanUpAndExit(int);
    cleanUpAndExit(-1); 
    return;
  }
  assert(f_main);
  Address addr = f_main->addr();
  
  // save original instruction first
  readDataSpace((void *)addr, sizeof(instruction), savedCodeBuffer, true);

  // and now, insert trap
  instruction insnTrap;
  generateBreakPoint(insnTrap);
  writeDataSpace((void *)addr, sizeof(instruction), (char *)&insnTrap);  
  main_brk_addr = addr;
}

/*
 * getRTLibraryName()
 *
 * Return the name of the runtime library, grabbed from the environment
 * or the command line, as appropriate.
 *
 * Updates process::dyninstName
 */

bool getRTLibraryName(string &dyninstName, int pid)
{
  // get library name... 
  // Get the name of the appropriate runtime library
#ifdef BPATCH_LIBRARY  /* dyninst API loads a different run-time library */
  const char DyninstEnvVar[]="DYNINSTAPI_RT_LIB";
#else
  const char DyninstEnvVar[]="PARADYN_LIB";
#endif

  if (dyninstName.length()) {
    // use the library name specified on the start-up command-line
  } else {
    // check the environment variable
    if (getenv(DyninstEnvVar) != NULL) {
      dyninstName = getenv(DyninstEnvVar);
    } else {
      string msg = string("Environment variable " + string(DyninstEnvVar)
                   + " has not been defined for process ") + string(pid);
      showErrorCallback(101, msg);
      return false;
    }
  }

  // Check to see if the library given exists.
  if (access(dyninstName.string_of(), R_OK)) {
    string msg = string("Runtime library ") + dyninstName
        + string(" does not exist or cannot be accessed!");
    showErrorCallback(101, msg);
    return false;
  }
  return true;
}

/*
 * dlopenDYNINSTlib()
 *
 * The evil black magic function. What we want: for the runtime
 * library to show up in the process' address space. Magically.
 * No such luck. What we do: patch in a call to dlopen(RTLIB)
 * at the entry of main, then restore the original instruction
 * and continue.
 */

bool process::dlopenDYNINSTlib()
{
  // This is actually much easier than on other platforms, since
  // by now we have the DYNINSTstaticHeap symbol to work with.
  // Basically, we can ptrace() anywhere in the text heap we want to,
  // so go somewhere untouched. Unfortunately, we haven't initialized
  // the tramp space yet (no point except on AIX) so we can't simply
  // call inferiorMalloc(). 

  // However, if we can get code_len_ + code_off_ from the object file,
  // then we can use the area above that point freely.

  // Steps: Get the library name (command line or ENV)
  //        Get the address for dlopen()
  //        Write in a call to dlopen()
  //        Write in a trap after the call
  //        Write the library name somewhere where dlopen can find it.
  // Actually, why not write the library name first?

  const Object binaryFile = symbols->getObject();
  Address codeBase = binaryFile.code_off() + binaryFile.code_len();
  // Round it up to the nearest instruction. 
  codeBase += sizeof(instruction) - (codeBase % sizeof(instruction));


  int count = 0; // how much we've written
  unsigned char scratchCodeBuffer[BYTES_TO_SAVE]; // space
  Address dyninstlib_addr;
  Address dlopencall_addr;
  Address dlopentrap_addr;

  // Do we want to save whatever is there? Can't see a reason why...

  // Write out the name of the library to the codeBase area
  if (!getRTLibraryName(dyninstName, pid))
    return false;
  
  // write library name...
  dyninstlib_addr = (Address) (codeBase + count);
  writeDataSpace((void *)(codeBase + count), dyninstName.length()+1,
		 (caddr_t)const_cast<char*>(dyninstName.string_of()));
  count += dyninstName.length()+sizeof(instruction); // a little padding

  // Actually, we need to bump count up to a multiple of insnsize
  count += sizeof(instruction) - (count % sizeof(instruction));

  // Need a register space
  // make sure this syncs with inst-power.C
  Register liveRegList[] = {10, 9, 8, 7, 6, 5, 4, 3};
  Register deadRegList[] = {11, 12};
  unsigned liveRegListSize = sizeof(liveRegList)/sizeof(Register);
  unsigned deadRegListSize = sizeof(deadRegList)/sizeof(Register);

  registerSpace *dlopenRegSpace = new registerSpace(deadRegListSize, deadRegList, 
						    liveRegListSize, liveRegList);
  dlopenRegSpace->resetSpace();

  Address dyninst_count = 0; // size of generated code
  vector<AstNode*> dlopenAstArgs(2);
  AstNode *dlopenAst;

  // at this time, we know the offset for the library name, so we fix the
  // call to dlopen and we just write the code again! This is probably not
  // very elegant, but it is easy and it works - naim
  dlopenAstArgs[0] = new AstNode(AstNode::Constant, (void *)(dyninstlib_addr));
  dlopenAstArgs[1] = new AstNode(AstNode::Constant, (void*)DLOPEN_MODE);
  dlopenAst = new AstNode("dlopen", dlopenAstArgs);
  removeAst(dlopenAstArgs[0]);
  removeAst(dlopenAstArgs[1]);
  dlopenAst->generateCode(this, dlopenRegSpace, (char *)scratchCodeBuffer,
			  dyninst_count, true, true);
  dlopencall_addr = codeBase + count;
  writeDataSpace((void *)dlopencall_addr, dyninst_count, 
		 (char *)scratchCodeBuffer);
  removeAst(dlopenAst);
  count += dyninst_count;

  instruction insnTrap;
  generateBreakPoint(insnTrap);
  dlopentrap_addr = codeBase + count;
  writeDataSpace((void *)dlopentrap_addr, sizeof(instruction),
		 (void *)(&insnTrap.raw));
  count += sizeof(instruction);

  dyninstlib_brk_addr = dlopentrap_addr;

  // save registers
  savedRegs = getRegisters();
  assert((savedRegs!=NULL) && (savedRegs!=(void *)-1));

  isLoadingDyninstLib = true;
  if (!changePC(dlopencall_addr)) {
    logLine("WARNING: changePC failed in dlopenDYNINSTlib\n");
    assert(0);
  }

  return true;
}

/*
 * Return the entry point of dlopen(). Used (I think) when we generate
 * a call to dlopen in an AST
 */

Address process::get_dlopen_addr() const {

  function_base *pdf = findOneFunction("dlopen");

  if (pdf)
    return pdf->addr();

  return 0;

}
