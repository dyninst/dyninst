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

#include "util/h/headers.h"
#include "dyninstAPI/src/os.h"
#include "dyninstAPI/src/process.h"
#include "dyninstAPI/src/stats.h"
#include "util/h/Types.h"
#include <fcntl.h>
#include "paradynd/src/showerror.h"
#include "paradynd/src/main.h"
#include "dyninstAPI/src/symtab.h"
#include <machine/save_state.h>
#include "util/h/pathName.h"

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


// Search the unwind table to find the entry correspoding to the pc 
// return the pointer to that entry
struct unwind_table_entry *
findUnwindEntry(image* symbols,int pc)
{
    int middleEntry;
    int firstEntry = 0;
    int lastEntry = (symbols->unwind).size() - 1 ;

    while (firstEntry <= lastEntry)
    {
	middleEntry = (firstEntry + lastEntry) / 2;
	if (pc >= symbols->unwind[middleEntry].region_start
	    && pc <= symbols->unwind[middleEntry].region_end)
	{
	    return &(symbols->unwind[middleEntry]);
	}
	
	if (pc < symbols->unwind[middleEntry].region_start)
            lastEntry = middleEntry - 1;
	else
            firstEntry = middleEntry + 1;
    }

    return NULL;
}              

// To see if this frame is valid to determine if this frame is the 
// innermost frame. Return true if it is, otherwise return false. 
bool 
frameChainValid(process *proc, unsigned pc)
{
    function_base *funcStart = proc->findOneFunction("_start");

    if (funcStart) {
	if ((pc >= funcStart->getAddress(proc)) && 
	    (pc <= (funcStart->getAddress(proc) + funcStart->size()))) {   
	    return false;
	}
    } else {
	sprintf(errorLine,"func _start is not found");
	logLine(errorLine);
    }
    
    funcStart = proc->getImage()->findOneFunctionFromAll("$START$");

    if (funcStart) {
	if ((pc >= funcStart->getAddress(proc)) && 
	     (pc <= (funcStart->getAddress(proc) + funcStart->size()))) {   
	    return false;
	}
    } else {
	sprintf(errorLine,"func $start$ is not found");
	logLine(errorLine);
    }

    return true;
}

static unsigned readCurrPC(int pid);

bool process::getActiveFrame(int *fp, int *pc)
{
    struct unwind_table_entry *unwind;
    char buf[4];
    bool err = true;
    int reg_sp;
    
    freeNotOK = false;
    
    // Get the value of PC from the reg no.33 which is ss_pcoq_head in
    // the structure save_state. </usr/include/machine/save_state.h>   
    *pc = readCurrPC(getPid());
#ifdef DEBUG_STACK
	sprintf(errorLine, "getActiveFrame: PC = %x, ", *pc); 
	logLine(errorLine);
#endif
    if (*pc == -1) assert(0);

/*    if (ptraceKludge::deliverPtrace(this,PT_RUREGS,(char *)132,0, buf)) {
        buf[3] &= ~0x3;
	*pc = *(int *)&buf[0];
#ifdef DEBUG_STACK
	sprintf(errorLine, "getAF: PC = %x, ", *pc); 
	logLine(errorLine);
#endif
    }
    else err = false;*/
    
    // We get the value of frame pointer by finding out the frame size
    // of current frame. Then move the SP down to the head of next
    // frame.
    unwind = findUnwindEntry (symbols, *pc);
    if (!unwind) {
#ifdef DEBUG_STACK
	sprintf(errorLine, "Not getting any frame, check that!\n");
	logLine(errorLine);
#endif	
	freeNotOK = true;
	err = false;
	/* *fp = 1; /* set it to non-zero value. which is to say this
		    is not the last frame although the entry is not found */
	return err;
    }
    
    if (ptraceKludge::deliverPtrace(this,PT_RUREGS,(char *)120,0, buf)) {
        buf[3] &= ~0x3;
	reg_sp = *(int *)&buf[0];
#ifdef DEBUG_STACK
	sprintf(errorLine, "SP  = %x, ", reg_sp);
	logLine(errorLine);
#endif
    }
    
    if (unwind -> HP_UX_interrupt_marker) {
#ifdef DEBUG_STACK
	sprintf(errorLine, "Interrupt frame, exit.\n");
	logLine(errorLine);
#endif
	freeNotOK = true;
	err = false;
    } else {
	*fp = reg_sp - ((unwind->Total_frame_size)<<3);
#ifdef DEBUG_STACK
	sprintf(errorLine, "FP = %x.\n", *fp);
	logLine(errorLine);
#endif
    }
    
    return err;

}

bool process::needToAddALeafFrame(Frame,unsigned int &){
    return false;
}


bool process::readDataFromFrame(int currentFP, int *fp, int *rtn, bool uppermost)
{

    struct unwind_table_entry *unwind; 
    bool readOK=true;
    char buf[20];

    //pdFunction *func;
    int pc = *rtn;

    //XXX: leaf routine might not necessarily the uppermost routine?
    //if (uppermost) {
    unwind = findUnwindEntry(symbols, pc);
    if (unwind) {
	if (unwind->Total_frame_size == 0) {
	    if (!uppermost) return false;

	    if (ptraceKludge::deliverPtrace(this, PT_RUREGS, (char *)8, 
					    0, buf)) {
		buf[3] &= ~0x3;
		*rtn = *(int *)&buf[0];
#ifdef DEBUG_STACK
		sprintf(errorLine, "before leaf: PC = %x.\n", *rtn);
		logLine(errorLine);
#endif		    
		return readOK;
	    }    
	}
    }
    //else
    //	    return false;
    //}

    if (readDataSpace((caddr_t) (currentFP-20),
		      sizeof(int)*6, buf, true)) {
	// this is the previous PC
        buf[3] &= ~0x3;
	*rtn = *(int *)&buf[0];
	//  use the infomatino in the unwind table instead.  
	//  *fp = *(int *)&buf[20];
	
	unwind = findUnwindEntry (symbols, *rtn);
	if (!unwind) {
#ifdef DEBUG_STACK
	    sprintf(errorLine, "Not getting any frame, check that!(pc = %x)\n", *rtn);
	    logLine(errorLine);
#endif
	    freeNotOK = true;
	    return false;
	}
	
	if (unwind -> HP_UX_interrupt_marker) {
#ifdef DEBUG_STACK
	    sprintf(errorLine, "Interrupt frame, exit.\n");
	    logLine(errorLine);
#endif
	    freeNotOK = true;
	    return false;
	}
	
	*fp = currentFP - ((unwind->Total_frame_size)<<3);

#ifdef DEBUG_STACK 
	sprintf(errorLine, "\t return PC = %x, frame pointer = %x.\n", *rtn, *fp);
	logLine(errorLine);
#endif
	// if we are in the outermost frame ,we should stop walking
	// stack by making fp=0.
	if (!frameChainValid(this, *rtn))  {
	    readOK=false;
	    *fp = 0;
	    *rtn = 0;
	}
    }
    else {
	readOK=false;
    }
    
    return(readOK);
}

static save_state save_state_foo;
#define saveStateRegAddr(field) ((char*)&save_state_foo.field - (char*)&save_state_foo.ss_flags)
#define saveStateFPRegAddr(field) ((char*)&save_state_foo.ss_fpblock.fpdbl.field - (char*)&save_state_foo.ss_flags)

static unsigned readCurrPC(int pid) {
   // If IN_SYSCALL then we should return the contents
   // of register 31 (the link register for BLE should contain
   // the return address, in user code); else, return the contents
   // of reg 33 (the PCOQ_HEAD).
   // Note that in any event, we clear the low 2 bits (I think that they
   // have something to do w/ privilege level -- please confirm this)

    const unsigned saveStateFlagAddr = saveStateRegAddr(ss_flags);
    assert(saveStateFlagAddr == 0);
    errno = 0;
    const unsigned saveStateFlags = ptrace(PT_RUREGS, pid, saveStateFlagAddr, 0, \
					   0);
    if (errno != 0) {
	perror("readCurrPC PT_RUREGS");
	cerr << "ReadCurrPC: the process exited" << endl;
	assert(0);
	return (unsigned)-1;
    }

   errno = 0;
   unsigned result;

   if (saveStateFlags & SS_INSYSCALL) {
      result = ptrace(PT_RUREGS, pid, 4 * 31, 0, 0);
   }
   else {
      result = ptrace(PT_RUREGS, pid, 4 * 33, 0, 0);
   }

   if (errno != 0) {
      perror("readCurrPC");
      cerr << "SS_INSYSCALL was " << (saveStateFlags & SS_INSYSCALL) << endl;

      return (unsigned)-1;
   }

   return result & ~0x03;
}

static unsigned readCurrPC(int pid, unsigned flagsreg) {
   // If IN_SYSCALL then we should return the contents
   // of register 31 (the link register for BLE should contain
   // the return address, in user code); else, return the contents
   // of reg 33 (the PCOQ_HEAD).
   // Note that in any event, we clear the low 2 bits (I think that they
   // have something to do w/ privilege level -- please confirm this)

   errno = 0;
   unsigned result;

   if (flagsreg & SS_INSYSCALL) {
      result = ptrace(PT_RUREGS, pid, 4 * 31, 0, 0);
   }
   else {
      result = ptrace(PT_RUREGS, pid, 4 * 33, 0, 0);
   }

   if (errno != 0) {
      perror("readCurrPC");
      cerr << "SS_INSYSCALL was " << (flagsreg & SS_INSYSCALL) << endl;

      return (unsigned)-1;
   }

   return result & ~0x03;
}


static const unsigned getRegistersNumBytes = 
                             4 // for save-state flags (not sure this needs to be saved)
                           + 4 * 31 // for integer regs 1 thru 31 (can skip reg 0)
			   + 8 * 32 // for floating pt regs 0 thru 31
			   + 4 // IPSW
			   + 4 // SAR
			   + 4 // pc, to be restored as PCOQ_HEAD
			   + 4 // pcspace, to be restored as PCSQ_HEAD
			   + 4 // pc+4, to be restored as PCOQ_TAIL
			   + 4 // pcspace, to be restored as PCSQ_TAIL
			   ;

void *process::getRegisters(bool &syscall) {
   // ptrace - GETREGS call
   // assumes the process is stopped (ptrace requires it)
   syscall = false;

   assert(status_ == stopped);

   void *result = new char[getRegistersNumBytes];
   assert(result);

   unsigned *resultPtr = (unsigned *)result;

   // See <machine/save_state.h>, <sys/param.h> and <machine/param.h>

   // The first thing we need to do is read the save-state flags
   const unsigned saveStateFlagAddr = saveStateRegAddr(ss_flags);
   assert(saveStateFlagAddr == 0);
   errno = 0;
   const unsigned saveStateFlags = ptrace(PT_RUREGS, getPid(), saveStateFlagAddr, 0, 0);
   if (errno != 0) {
      perror("process::getRegisters PT_RUREGS");
      cerr << "addr was " << saveStateFlagAddr << endl;
      return NULL;
   }

   if (saveStateFlags & SS_INSYSCALL)
       *resultPtr++ = saveStateFlags & ~0x2;
   else
       *resultPtr++ = saveStateFlags;

   // Now set 'thePCspace' to the value of PCSQ_HEAD_REGNUM,
   // and save registers "normally" (as read from ptrace).
   errno = 0;
   const unsigned thePCspace = ptrace(PT_RUREGS, getPid(), saveStateRegAddr(ss_pcsq_head), 0, 0);
   if (errno != 0) {
      perror("ptrace PT_RUREGS for ss_pcsq_head");
      return NULL;
   }

   // save integer registers now (skip reg 0, which is hardwired to value 0)
   for (unsigned regnum=1; regnum < 32; regnum++) {
      errno = 0;
      const unsigned regvalue = ptrace(PT_RUREGS, getPid(), 4 * regnum, 0, 0);
      if (errno != 0) {
	 perror("ptrace PT_RUREGS for an integer register");
	 return NULL;
      }

      *resultPtr++ = regvalue;
   }

   // save floating-point registers now (32 of them, each 8 bytes)
   const unsigned fpbaseoffset = saveStateFPRegAddr(ss_fp0);
   for (unsigned regnum=0; regnum < 32; regnum++) {
      const unsigned offset = fpbaseoffset + 8 * regnum;

      // 2 ptrace calls are needed (get 4 bytes at a time)
      errno = 0;
      const unsigned part1 = ptrace(PT_RUREGS, getPid(), offset, 0, 0);
      if (errno != 0) {
	 perror("ptrace PT_RUREGS for part 1 of an fp reg");
	 return NULL;
      }

      errno = 0;
      const unsigned part2 = ptrace(PT_RUREGS, getPid(), offset+4, 0, 0);
      if (errno != 0) {
	 perror("ptrace PT_RUREGS for part 2 of an fp reg");
	 return NULL;
      }

      *resultPtr++ = part1;
      *resultPtr++ = part2;
   }

   // save IPSW_REGNUM (#41) and SAR (#32)
   errno = 0;
   const unsigned ipsw = ptrace(PT_RUREGS, getPid(), 4 * 41, 0, 0);
   if (errno != 0) {
      perror("ptrace PT_RUREGS for IPSW (regnum 41)");
      return NULL;
   }
   *resultPtr++ = ipsw;

   errno = 0;
   const unsigned sar = ptrace(PT_RUREGS, getPid(), 4 * 32, 0, 0);
   if (errno != 0) {
      perror("ptrace PT_RUREGS for SAR (regnum 32)");
      return NULL;
   }
   *resultPtr ++ = sar;

   // save pc (to be restored as PCOQ_HEAD),
   // pcspace (to be restored as PCSQ_HEAD),
   // pc+4 (to be restored as PCOQ_TAIL),
   // and pcspace (to be restored as PCSQ_TAIL)

   const unsigned pc_reg = readCurrPC(getPid(), saveStateFlags);
   if (pc_reg == (unsigned)-1) {
      cerr << "getRegisters() failed because readCurrPC() failed" << endl;
      return NULL;
   }
   *resultPtr++ = pc_reg; // to be restored as PCOQ_HEAD

   *resultPtr++ = thePCspace; // to be restored as PCSQ_HEAD

   *resultPtr++ = pc_reg + 4; // to be restored as PCOQ_TAIL

   *resultPtr++ = thePCspace; // to be restored as PCSQ_TAIL

   assert((char *)resultPtr - (char *)result == getRegistersNumBytes);

   if (saveStateFlags & SS_INSYSCALL) {
       // cerr << "hpux getRegisters(): SS_INSYSCALL is true, so deferring..." << endl;
       syscall = true;
       if (saveStateFlags & SS_DORFI) {
	   // cerr << "Delay the inferior RPC for the SS_DORFI." << endl;
	   return (void *)-1;
       }
   }

   return result;
}

static bool changePC_common(int pid, unsigned flagsReg, unsigned loc) {
   // In in a system call then we also set reg #31, setting low 2 bits (privilege)
   // to true
   if (flagsReg & SS_INSYSCALL) {
      unsigned valueToWrite = loc | 0x03;
      errno = 0;
      ptrace(PT_WUREGS, pid, 31 * 4, valueToWrite, 0);
      if (errno != 0) {
	 perror("changePC_common");
	 cerr << "reg num was 31" << endl;
	 return false;
      }

      // fall through, on purpose
   }

   // Now write reg 33 (PCOQ_HEAD) with loc and reg 35 (PCOQ_TAIL) with loc+4
   errno = 0;
   ptrace(PT_WUREGS, pid, 33 * 4, loc, 0);
   if (errno != 0) {
      perror("changePC_common");
      cerr << "reg num was 33 (PCOQ_HEAD)" << endl;
      return false;
   }

   errno = 0;
   ptrace(PT_WUREGS, pid, 35 * 4, loc, 0);
   if (errno != 0) {
      perror("process::changePC");
      cerr << "reg num was 35 (PCOQ_TAIL)" << endl;
   }

   return true;
}

bool process::executingSystemCall() {
   // this is not implemented yet - naim 5/15/97
   return false;
}

bool process::changePC(unsigned loc) {
   // first we need to get the flags register, so we can check to see
   // if we're in the middle of a system call.

   const unsigned saveStateFlagAddr = saveStateRegAddr(ss_flags);
   assert(saveStateFlagAddr == 0);
   errno = 0;
   unsigned saveStateFlags = ptrace(PT_RUREGS, getPid(), saveStateFlagAddr, 0, 0);
   if (errno != 0) {
      perror("process::getRegisters PT_RUREGS");
      cerr << "addr was " << saveStateFlagAddr << endl;
      return NULL;
   }

   if (saveStateFlags & SS_INSYSCALL)
      saveStateFlags &= ~0x2;

   return changePC_common(pid, saveStateFlags, loc);
}

bool process::changePC(unsigned loc, const void *savedRegs) {
   unsigned flagsReg = *(unsigned*)savedRegs;
   return changePC_common(pid, flagsReg, loc);
}

bool process::restoreRegisters(void *buffer) {
   // assumes process is stopped (ptrace requires it)
   assert(status_ == stopped);

   const unsigned saveStateFlags = *(unsigned *)buffer;

   // First, restore the PC queue (PC[S/O]Q_HEAD and _TAIL)
   const unsigned *bufferPtr = buffer;
   bufferPtr += 1 + 31 + 64 + 2; // skip past save-state, 31 int regs, 32 fp regs @ 8 bytes each,
                                 // ipsw, and sar
   const unsigned savedPCOQ_HEAD = *bufferPtr++;
   const unsigned savedPCSQ_HEAD = *bufferPtr++;
   const unsigned savedPCOQ_TAIL = *bufferPtr++;
   const unsigned savedPCSQ_TAIL = *bufferPtr++;

   assert(savedPCSQ_TAIL == savedPCSQ_HEAD); // they're always equal when saved
   assert(savedPCOQ_TAIL == savedPCOQ_HEAD + 4); // it's always this way when saved

   // First step, skip the breakpoint by incrementing the PC by 4
   Frame theFrame(this);
   int framePC = theFrame.getPC();

   ptrace(PT_WUREGS, getPid(), 33*4, framePC+4, 0);
   if (errno != 0) {
      perror("PT_WUREGS for PCOQ_HEAD");
      return false;
   }

   ptrace(PT_WUREGS, getPid(), 35*4, framePC+8, 0);
   if (errno != 0) {
      perror("PT_WUREGS for PCOQ_TAIL");
      return false;
   }

   // Second step, assign the correct SR and PC value to the register.
   ptrace(PT_WUREGS, getPid(), 21*4, savedPCSQ_HEAD, 0);
   if (errno != 0) {
      perror("PT_WUREGS for PCSQ_HEAD");
      return false;
   }
   
   ptrace(PT_WUREGS, getPid(), 22*4, savedPCOQ_HEAD, 0);
   if (errno != 0) {
      perror("PT_WUREGS for PCOQ_HEAD");
      return false;
   }

   // Third step, generate any breakpoint instruction at the the new pc
   // and continue the process. After two instructions being executed,
   // the program should reach this trap instruction. then we replace
   // this instruction with the original instruction and restore all
   // the old registers.
   char buf[4];
   if (!readDataSpace((caddr_t)(savedPCOQ_HEAD), sizeof(int), buf, true)) {
       cerr << "Restore Register failed because readDataSpace failed" << endl;
       assert(0);
   }

   instruction bp;
   extern void generateBreakPoint(instruction &);
   generateBreakPoint(bp);
   if (!writeTextSpace((caddr_t)(savedPCOQ_HEAD), sizeof(int), (caddr_t)&bp)) {
       cerr << "Restore Register failed because writeTextSpace failed" <<endl;
       assert(0);
   }
   continueProc();
       
   bool isStopped = false;
   int waitStatus;
   while (!isStopped) {
       int ret = P_waitpid(pid, &waitStatus, WUNTRACED);
       if ((ret == -1 && errno == ECHILD) || (WIFEXITED(waitStatus))) {
	   cerr << "Child process died.\t" << "ret: " << ret <<"\t errno: "
	        << errno << "\tWIFEXITED(waitStatus): " << WIFEXITED(waitStatus) <<endl;
	   // the child is gone.
	   //status_ = exited;
	   assert(0);
       }
       if (!WIFSTOPPED(waitStatus) && !WIFSIGNALED(waitStatus)) {
	   printf("problem stopping process\n");
	   assert(0);
       }
       int sig = WSTOPSIG(waitStatus);
       if (sig == SIGTRAP) {
	   // if (ptrace(PT_DETACH,pid,(int *) 1, SIGSTOP, NULL) == -1) {
           //    logLine("ptrace error\n");
           // }
	   status_ = stopped;
	   isStopped = true;
       } else {
	   unsigned pc = readCurrPC(getPid());
	   cerr << "Currently, the PC is " << (void *)*&pc << endl; 
	   cerr << "Got an unexpected Trap, hmmm. Anyway... " << endl;
	   status_ = stopped;
	   isStopped = true;
	   if (ptrace(PT_DETACH,pid,(int *) 1, SIGSTOP, NULL) == -1) {
	       logLine("ptrace error\n");
	   }
	   /* cerr << "Detaching from the application!" << endl;
	   if (sig > 0) {
	       if (P_ptrace(PT_CONTIN, pid, 1, WSTOPSIG(waitStatus), 0) == -1) {
		   cerr << "Ptrace error\n";
		   assert(0);
	       }
	   }
	   cerr << "Is this correct? " << endl;
	   continueProc();*/
       }
   }

   writeTextSpace((caddr_t)(savedPCOQ_HEAD), sizeof(int), (caddr_t)&buf);

   /* errno = 0;
   ptrace(PT_WUREGS, getPid(), 33*4, savedPCOQ_HEAD, 0); // 33 --> PCOQ_HEAD
   if (errno != 0) {
      perror("PT_WUREGS for PCOQ_HEAD");
      return false;
   }

   errno = 0;
   ptrace(PT_WUREGS, getPid(), 35*4, savedPCOQ_TAIL, 0); // 35 --> PCOQ_TAIL
   if (errno != 0) {
      perror("PT_WUREGS for PCOQ_TAIL");
      return false;
   }

   cerr << "restoreRegisters: restored PCOQ head and tail...now trying for PCSQ" << endl;

   errno = 0;
   ptrace(PT_WUREGS, getPid(), 33*4, savedPCSQ_HEAD, 0); // 33 --> PCSQ_HEAD
   if (errno != 0) {
      perror("PT_WUREGS for PCSQ_HEAD");
      return false;
   }

   errno = 0;
   ptrace(PT_WUREGS, getPid(), 35*4, savedPCSQ_TAIL, 0); // 35 --> PCSQ_TAIL
   if (errno != 0) {
      perror("PT_WUREGS for PCSQ_TAIL");
      return false;
   } */

   // cerr << "restoreRegisters: got past the hard part!" << endl;

   // Now restore the integer registers
   bufferPtr = (unsigned *)buffer;
   bufferPtr++; // skip past saved state flags
   for (unsigned regnum = 1; regnum < 32; regnum++) {
      const unsigned value = *bufferPtr++;

      errno = 0;
      ptrace(PT_WUREGS, getPid(), regnum * 4, value, 0);
      if (errno != 0) {
         perror("PT_WUREGS for an integer register");
	 cerr << "the register num was " << regnum << endl;
	 return false;
      }
   }

   // Now restore the floating point registers 0 thru 31, @ 8 bytes each
   // We are not able to write the floating point register 0-3
   for (unsigned regnum = 64; regnum < 128; regnum+=2) {
      const unsigned part1 = *bufferPtr++;
      const unsigned part2 = *bufferPtr++;

      // The first four register are not allowed to write 
      if (regnum < 64 + 8)
	  continue;

      errno = 0;
      ptrace(PT_WUREGS, getPid(), regnum * 4, part1, 0);
      if (errno != 0) {
	 perror("PT_WUREGS for part 1 of an fp register");
	 cerr << "the fp register num was " << regnum-64 << endl;
	 // return false;
      }

      errno = 0;
      ptrace(PT_WUREGS, getPid(), (regnum * 4) + 4, part2, 0);
      if (errno != 0) {
	 perror("PT_WUREGS for part 2 of an fp register");
	 cerr << "the fp register num was " << regnum-64 << endl;
	 // return false;
      }
   }

   // Lastly, restore IPSW (cr 22, aka reg num 41) and SAR (cr 11, aka reg num 32)
   const unsigned savedIPSW = *bufferPtr++;
   const unsigned savedSAR  = *bufferPtr++;
   // assert(((unsigned)bufferPtr - (unsigned)buffer) == (getRegistersNumBytes-16));

   errno = 0;
   ptrace(PT_WUREGS, getPid(), 4 * 41, savedIPSW, 0);
   if (errno != 0) {
      perror("PT_WUREGS for IPSW (reg num 41)");
      return false;
   }

   errno = 0;
   ptrace(PT_WUREGS, getPid(), 4 * 32, savedSAR, 0);
   if (errno != 0) {
      perror("PT_WUREGS for SAR (reg num 32)");
      return false;
   }

   // cerr << "restoreRegisters: all done!" << endl;

   return true;
}

bool ptraceKludge::deliverPtrace(process *p, int req, void *addr,
                                 int data, void *addr2) {
  bool halted;
  bool ret;
  int  valu;

  if (req != PT_DETACH) halted = haltProcess(p);
  if (( valu = P_ptrace(req, p->getPid(), (int) addr, data, (int)addr2)) == (-1))
    ret = false;
  else
    ret = true;

  if (req == PT_RUREGS) *(int *)&addr2[0] = valu;

  if (req != PT_DETACH) continueProcess(p, halted);
  return ret;
}

void ptraceKludge::continueProcess(process *p, const bool wasStopped) {
  if ((p->status() != neonatal) && (!wasStopped))
/* Choose either one of the following methods to continue a process.
 * The choice must be consistent with that in process::continueProc_ and stop_
 */
#ifndef PTRACE_ATTACH_DETACH
    if (P_ptrace(PT_CONTIN, p->pid, 1, SIGCONT, 0) == -1) {
#else
    if (P_ptrace(PT_DETACH, p->pid, 1, SIGCONT, 0) == -1) {
#endif
      cerr << "error in continueProcess\n";
      assert(0);
    }
}

// already setup on this FD.
// disconnect from controlling terminal 
void OS::osDisconnect(void) {
  //logLine("OS::osDisconnect not available");
}

bool process::stop_() {
   // formerly OS::osStop()

/* Choose either one of the following methods for stopping a process, but not both. 
 * The choice must be consistent with that in process::continueProc_ 
 * and ptraceKludge::continueProcess
 */
#ifndef PTRACE_ATTACH_DETACH
	return (P_kill(pid, SIGSTOP) != -1); 
#else
	return attach_();
#endif
}

bool process::continueWithForwardSignal(int sig) {
  return (P_ptrace(PT_CONTIN, pid, 1, sig, 0) != -1);
}

void OS::osTraceMe(void) { P_ptrace(PT_SETTRC, 0, 0, 0, 0); }


// wait for a process to terminate or stop
int process::waitProcs(int *status) {
  return waitpid(0, status, WNOHANG);
}

// attach to an inferior process.
bool process::attach() {
  // we only need to attach to a process that is not our direct children.
  if (parent != 0) {
    return attach_();
  }
  else
    return true;
}

bool process::attach_() {
   return (P_ptrace(PT_ATTACH, getPid(), 0, 0, 0) != -1);
}

bool process::isRunning_() const {
   // determine if a process is running by doing low-level system checks, as
   // opposed to checking the 'status_' member vrble.  May assume that attach()
   // has run, but can't assume anything else.

   assert(false); // not yet implemented!   
}


// TODO is this safe here ?
bool process::continueProc_() {
  int ret;

  if (!checkStatus()) 
    return false;
  ptraceOps++; ptraceOtherOps++;
/* choose either one of the following ptrace calls, but not both. 
 * The choice must be consistent with that in stop_ and
 * ptraceKludge::continueProcess.
 */
#ifndef PTRACE_ATTACH_DETACH
  ret = P_ptrace(PT_CONTIN, pid, 1, 0, 0);
#else
  ret = P_ptrace(PT_DETACH, pid, 1, SIGCONT, 0);
#endif
  return ret != -1;
}

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
  if (!checkStatus())
    return false;
  ptraceOps++; ptraceOtherOps++;
  return (ptraceKludge::deliverPtrace(this, PT_DETACH, (char*)1, SIGCONT, NULL));
}

// temporarily unimplemented, PTRACE_DUMPCORE is specific to sunos4.1
bool process::dumpCore_(const string coreFile) {
  if (!checkStatus()) 
    return false;
  ptraceOps++; ptraceOtherOps++;

  assert(0);
  errno = 0;
  // int ret = P_ptrace(request, pid, coreFile, 0, (char*) NULL);
  int ret = 0;
  assert(errno == 0);
  return ret;
}

bool process::writeTextWord_(caddr_t inTraced, int data) {
  if (!checkStatus()) 
    return false;
  ptraceBytes += sizeof(int); ptraceOps++;
  return (ptraceKludge::deliverPtrace(this, PT_WIUSER, inTraced, data, NULL));
}

bool process::writeTextSpace_(void *inTraced, int amount, const void *inSelf) {
  if (!checkStatus()) 
    return false;
  ptraceBytes += amount; ptraceOps++;
  return (ptraceKludge::deliverPtrace(this, PT_WRTEXT, inTraced, amount, inSelf));
  //the amount is number of bytes, so we need to change it to number
  //of words first!!
  // assert((amount % sizeof(int))==0);     // Same changes in write..
  // amount = amount / sizeof(int);       // read.. procedures;
  // for (unsigned i = 0; i < amount; ++i) {
  //  int data; memcpy(&data, inSelf, sizeof data);
  //  if (!ptraceKludge::deliverPtrace(this, PT_WIUSER, inTraced, data, 0)) {
  //    return false;
  //  }
  //  inTraced += sizeof data;
  //  inSelf += sizeof data;
  //}
  //return true;
}

bool process::writeDataSpace_(void *inTraced, int amount, const void *inSelf) {
  if (!checkStatus())
    return false;
  ptraceOps++; ptraceBytes += amount;
  return (ptraceKludge::deliverPtrace(this, PT_WRDATA, inTraced, amount, inSelf));

  //assert((amount % sizeof(int))==0);
  //amount = amount / sizeof(int);   
  //for (unsigned i = 0; i < amount; ++i) {
  //  int data; memcpy(&data, inSelf, sizeof data);
  //  if (!ptraceKludge::deliverPtrace(this, PT_WDUSER, inTraced, data, 0)) {
  //    return false;
  //  }
  //  inTraced += sizeof data;
  //  inSelf += sizeof data;
  //}
  //return true;
}

bool process::readDataSpace_(const void *inTraced, int amount, void *inSelf) {
  if (!checkStatus())
    return false;
  ptraceOps++; ptraceBytes += amount;
  return (ptraceKludge::deliverPtrace(this, PT_RDDATA, inTraced, amount, inSelf));


  //int* self_ptr = (int *) ((void *) inSelf);
  //assert((amount % sizeof(int))==0);
  //amount = amount / sizeof(int);   
  //for (unsigned i = 0; i < amount; ++i) {
  //  int data = ptraceKludge::deliverPtrace(this, PT_RDUSER, inTraced, 0, 0);
  //  memcpy(self_ptr, &data, sizeof data);
  //  inTraced += sizeof data;
  //  self_ptr++;
  //}
  //return true;
}

bool process::loopUntilStopped() {
  /* make sure the process is stopped in the eyes of ptrace */
  stop_();

  bool isStopped = false;
  int waitStatus;
  while (!isStopped) {
    int ret = P_waitpid(pid, &waitStatus, WUNTRACED);
    if ((ret == -1 && errno == ECHILD) || (WIFEXITED(waitStatus))) {
      // the child is gone.
      //status_ = exited;
	assert(0);
      handleProcessExit(this, WEXITSTATUS(waitStatus));
      return(false);
    }
    if (!WIFSTOPPED(waitStatus) && !WIFSIGNALED(waitStatus)) {
      printf("problem stopping process\n");
      assert(0);
    }
    int sig = WSTOPSIG(waitStatus);
    // printf("signal is %d\n", sig); fflush(stdout); 
    if (sig == SIGSTOP) {
      isStopped = true;
    } else {
	if (sig > 0) {
	    if (P_ptrace(PT_CONTIN, pid, 1, WSTOPSIG(waitStatus), 0) == -1) {
		cerr << "Ptrace error\n";
		assert(0);
	    }
	}
	stop_();
    }
  }

  return true;
}

bool process::dumpImage() {return false;}


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

string process::tryToFindExecutable(const string &progpath, int pid) {
   // returns empty string on failure

   if (progpath.length() == 0)
      return "";

   if (exists_executable(progpath))
      return progpath;

   return ""; // failure
}

unsigned process::read_inferiorRPC_result_register(reg) {
   assert(false);  // not yet implemented
}
