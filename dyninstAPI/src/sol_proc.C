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

// Solaris-style /proc support

// $Id: sol_proc.C,v 1.13 2003/03/12 01:49:58 schendel Exp $

#ifdef rs6000_ibm_aix4_1
#include <sys/procfs.h>
#else
#include <procfs.h>
#endif
#include <limits.h>
#include <poll.h>
#include "common/h/headers.h"
#include "dyninstAPI/src/signalhandler.h"
#include "dyninstAPI/src/process.h"
#include "dyninstAPI/src/dyn_lwp.h"
#include "dyninstAPI/src/dyn_thread.h"
#include "dyninstAPI/src/stats.h"
#include "common/h/pathName.h" // for path name manipulation routines

// Function prototypes
bool get_ps_info(int pid, string &argv0, string &cwdenv, string &pathenv);

/*
 * COMPATIBILITY SECTION
 * 
 * Even though aix and solaris are almost entirely the same, there are differences.
 * This section defines macros to deal with this
 */

#if defined(sparc_sun_solaris2_4)
#define GETREG_nPC(regs)      (regs[R_nPC])
#define GETREG_PC(regs)       (regs[R_PC])
#define GETREG_FP(regs)       (regs[R_O6])
#define GETREG_SP(regs)       (GETREG_FP(regs))
#define GETREG_GPR(regs, reg) (regs[reg])
// Solaris uses the same operators on all set datatypes
#define prfillsysset(x)       prfillset(x)
#define premptysysset(x)      premptyset(x)
#define praddsysset(x,y)      praddset(x,y)
#define prdelsysset(x,y)      prdelset(x,y)
#define prissyssetmember(x,y) prismember(x,y)
#define proc_sigset_t         sigset_t
#define SYSSET_MAP(x, pid)  (x)
#define SYSSET_ALLOC(x)     ((sysset_t *)malloc(sizeof(sysset_t)))
#define SYSSET_SIZE(x)      (sizeof(sysset_t))
#endif
#if defined(i386_unknown_solaris2_5)
#define REG_PC(regs) (regs->theIntRegs[EIP])
#define REG_FP(regs) (regs->theIntRegs[EBP])
#define REG_SP(regs) (regs->theIntRegs[UESP])
#endif
#if defined(rs6000_ibm_aix4_1)
#define GETREG_nPC(regs)       (regs.__iar)
#define GETREG_PC(regs)        (regs.__iar)
#define GETREG_FP(regs)        (regs.__gpr[1])
#define GETREG_SP(regs)        (GETREG_FP(regs))
#define GETREG_GPR(regs,reg)   (regs.__gpr[reg])
#define PR_BPTADJ           0 // Not defined on AIX
#define proc_sigset_t          pr_sigset_t
extern int SYSSET_MAP(int, int);
extern int SYSSET_SIZE(sysset_t *);
extern sysset_t *SYSSET_ALLOC(int);
#endif


/*
   osTraceMe is called after we fork a child process to set
   a breakpoint on the exit of the exec system call.
   When /proc is used, this breakpoint **will not** cause a SIGTRAP to 
   be sent to the process. The parent should use PIOCWSTOP to wait for 
   the child.
*/
// Interesting problem: I've seen a race condition occur when
// we set the proc trace flags from the mutator before the
// mutatee. In this case, DON'T reset the flags if there
// is a stop on exec in effect.

void OS::osTraceMe(void) {
    sysset_t *exitSet = SYSSET_ALLOC(getpid());
    int bufsize = SYSSET_SIZE(exitSet) + sizeof(long);
    
    char buf[bufsize];
    long *bufptr = (long *)buf;
    char procName[128];
    
    // Get the current set of syscalls
    pstatus_t status;
    sprintf(procName,"/proc/%d/status", (int) getpid());
    int stat_fd = P_open(procName, O_RDONLY, 0);
    if (pread(stat_fd, (void *)&status, sizeof(pstatus_t), 0) !=
        sizeof(pstatus_t)) {
        perror("osTraceMe::pread");
        return;
    }
    memcpy(exitSet, &(status.pr_sysexit), SYSSET_SIZE(exitSet));
    close(stat_fd);

    sprintf(procName,"/proc/%d/ctl", (int) getpid());
    int fd = P_open(procName, O_WRONLY, 0);
    if (fd < 0) {
        perror("open");
        return;
    }
    
    /* set a breakpoint at the exit of exec/execve */
    if (SYSSET_MAP(SYS_exec, getpid()) != -1) {
        praddsysset(exitSet, SYSSET_MAP(SYS_exec, getpid()));
    }
    if (SYSSET_MAP(SYS_execve, getpid()) != -1) {
        praddsysset(exitSet, SYSSET_MAP(SYS_execve, getpid()));
    }
    // Write out the command
    *bufptr = PCSEXIT; bufptr++;
    memcpy(bufptr, exitSet, SYSSET_SIZE(exitSet));
    if (write(fd, buf, bufsize) != bufsize) {
        perror("osTraceMe: PCSEXIT");
        P__exit(-1); // must use _exit here.
    }
    
    // AIX: do not close file descriptor or all changes are undone (?)
    // My guess is we need to fiddle with the run on close/kill on 
    // close bits. For now, leaving the FD open works.
    //close(fd);    
}
/*
 * DYN_LWP class
 * Operations on LWP file descriptors, or the representative LWP
 */

// Continues the LWP without clearing signal
bool dyn_lwp::continueWithSignal() {
    long buf[1];
    buf[0] = PCRUN;
    return (write(ctl_fd(), buf, sizeof(long)) == sizeof(long));
}

bool process::continueWithForwardSignal(int)
{
    return getDefaultLWP()->continueWithSignal();
}
// determine if a process is running by doing low-level system checks, as
// opposed to checking the 'status_' member vrble.  May assume that attach()
// has run, but can't assume anything else.

bool dyn_lwp::isRunning() const {
   lwpstatus_t theStatus;
   
   if (!get_status(&theStatus)) return false;

   // Don't consider PR_ASLEEP to be stopped
   long stopped_flags = PR_STOPPED | PR_ISTOP;

   if (theStatus.pr_flags & stopped_flags)
      return false;
   else
       return true;

   
}

bool dyn_lwp::clearSignal() {
    long command[2];
    //fprintf(stderr, "Clearing signal from process\n");
    command[0] = PCRUN; command[1] = PRSTOP | PRCSIG;
    if (write(ctl_fd(), command, 2*sizeof(long)) != 2*sizeof(long)) {
        perror("clearSignal: PCRUN");
        return false;
    }
    command[0] = PCWSTOP;
    
    if (write(ctl_fd(), command, sizeof(long)) != sizeof(long)) {
        perror("clearSignal: PCWSTOP");
        return false;
    }
    return true;
}

// Get the process running again. May do one or more of the following:
// 1) Continue twice to clear a signal
// 2) Restart an aborted system call
bool dyn_lwp::continueLWP() {
  lwpstatus_t status;
  long command[2];
  Address pc;  // PC at which we are trying to continue

  // Check the current status
  if (!get_status(&status)) {
      cerr << "Failed to get process status in continueLWP" << endl;
      return false;
  }
  
  if ((0==status.pr_flags & PR_STOPPED) && (0==status.pr_flags & PR_ISTOP)) {
      return false;
  }
  // If the lwp is stopped on a signal we blip it (technical term).
  // The process will re-stop (since we re-run with PRSTOP). At
  // this point we continue it.
  if ((status.pr_flags & PR_STOPPED)
      && (status.pr_why == PR_SIGNALLED)
      && (status.pr_what == SIGSTOP || 
          status.pr_what == SIGINT ||
          status.pr_what == SIGTRAP)) {
      clearSignal();
  }

  command[0] = PCRUN; command[1] = PRCSIG;

  pc = (Address)(GETREG_PC(status.pr_reg));

  if (! (stoppedInSyscall_ && pc == postsyscallpc_)) {
     // Continue the process the easy way
      if (write(ctl_fd(), command, 2*sizeof(long)) != 2*sizeof(long)) {
          perror("continueLWP: PCRUN2");
          return false;
      }
  } else {
      
      // We interrupted a sleeping system call at some previous pause
      // (i.e. stoppedInSyscall is true), we have not restarted that
      // system call yet, and the current PC is the insn following
      // the interrupted call.  It is time to restart the system
      // call.
      
      // Note that when we make the process runnable, we ignore
      // `flags', set if `hasNewPC' was true in the previous block,
      // because we don't want its PC; we want the PC of the system
      // call trap, which was saved in `syscallreg'.

      if (!restoreRegisters(syscallreg_)) return false;
      delete syscallreg_;

      // We are done -- the process is in the kernel for the system
      // call, with the right registers values.  Make the process
      // runnable, restoring its previously blocked signals.
      // The signals were blocked as part of the abort mechanism.
      stoppedInSyscall_ = false;
      int bufsize = sizeof(long) + sizeof(proc_sigset_t);
      char buf[bufsize]; long *bufptr = (long *)buf;
      *bufptr = PCSHOLD; bufptr++;
      memcpy(bufptr, &sighold_, sizeof(proc_sigset_t));

      if (write(ctl_fd(), buf, bufsize) != bufsize) {
          perror("continueLWP: PCSHOLD"); return false;
      }

      long command[2];
      command[0] = PCRUN;
      command[1] = 0; // No arguments to PCRUN
      if (write(ctl_fd(), command, 2*sizeof(long)) != 2*sizeof(long)) {
          perror("continueLWP: PCRUN3"); return false;
      }
      
  }
  

  // Odd occurrence: in MT startup, we first get a SIGTRAP, then
  // a SIGSTOP (after a run/pause pair), and then another SIGSTOP.

  return true;
  
}

// Abort a system call. Place a trap at the exit of the syscall in
// question, and run with the ABORT flag set. Wait for the trap
// to be hit and then return.

bool dyn_lwp::abortSyscall()
{
    sysset_t* scexit = SYSSET_ALLOC(proc_->getPid());
    sysset_t* scsavedexit = SYSSET_ALLOC(proc_->getPid());
    sysset_t* scentry = SYSSET_ALLOC(proc_->getPid());
    sysset_t* scsavedentry = SYSSET_ALLOC(proc_->getPid());
    lwpstatus_t status;

    // MT: aborting syscalls does not work. Maybe someone with better knowledge
    // of Solaris can get it working. 
#if defined(MT_THREAD)
    return false;
#endif
    // We do not expect to recursively interrupt system calls.  We could
    // probably handle it by keeping a stack of system call state.  But
    // we haven't yet seen any reason to have this functionality.
    assert(!stoppedInSyscall_);
    stoppedInSyscall_ = true;
    
    if (!get_status(&status)) return false;
    
    // 1. Save the syscall number, registers, and blocked signals
    stoppedSyscall_ = status.pr_syscall;
    // Copy registers as is getRegisters
    syscallreg_ = new dyn_saved_regs();
    memcpy(&(syscallreg_->theIntRegs), &(status.pr_reg), sizeof(prgregset_t));
    memcpy(&(syscallreg_->theFpRegs), &(status.pr_fpreg), sizeof(prfpregset_t));
    
    memcpy(&sighold_, &status.pr_lwphold, sizeof(proc_sigset_t));
    
#ifdef i386_unknown_solaris2_5
    // From Roger A. Faulkner at Sun (email unknown), 6/29/1997:
    //
    // On Intel and PowerPC machines, the system call trap instruction
    // leaves the PC (program counter, instruction pointer) referring to
    // the instruction that follows the system call trap instruction.
    // On Sparc machines, the system call trap instruction leaves %pc
    // referring to the system call trap instruction itself (the
    // operating system increments %pc on exit from the system call).
    //
    // We have to reset the PC back to the system call trap instruction
    // on Intel and PowerPC machines.
    //
    // This is 7 on Intel, 4 on PowerPC.
    
    // Note: On x86/Linux this should probably be 2 bytes, because Linux
    // uses "int" to trap, not lcall.
    
    syscallreg_->theIntRegs[PC_REG] -= 7;
#endif
    
    // 2. Abort the system call
    
    // Save current syscall exit traps
    pstatus_t proc_status;
    if (!proc_->get_status(&proc_status)) return false;
    
    proc_->get_entry_syscalls(&proc_status, scsavedentry);
    proc_->get_exit_syscalls(&proc_status, scsavedexit);
    
    // Set process to trap on exit from this system call
    premptyset(scentry);
    premptyset(scexit);
    praddset(scexit, stoppedSyscall_);
    
    proc_->set_syscalls(scentry, scexit);
    
    // Continue, aborting this system call and blocking all sigs except
    // those needed by DynInst.
    proc_sigset_t sigs;
    prfillset(&sigs);
    prdelset(&sigs, SIGTRAP);
    prdelset(&sigs, SIGILL);
    
    // Set the signals to "ignore" and run, aborting the syscall
    int bufsize = sizeof(long) + sizeof(proc_sigset_t);
    char buf[bufsize]; long *bufptr = (long *)buf;
    *bufptr = PCSHOLD; bufptr++;
    memcpy(bufptr, &sigs, sizeof(proc_sigset_t));
    
    long command[2];
    command[0] = PCRUN;
    command[1] = PRSABORT;
    
    if (write(ctl_fd(), buf, bufsize) != bufsize) {
        perror("abortSyscall: PCSHOLD"); return false;
    }
    if (write(ctl_fd(), command, 2*sizeof(long)) != 2*sizeof(long)) {
        perror("abortSyscall: PCRUN"); return false;
    }
    
    command[0] = PCWSTOP;
    
    if (write(ctl_fd(), command, sizeof(long)) != sizeof(long)) {
        perror("abortSyscall: PCWSTOP"); return false;
    }
    
    // Note: We assume that is always safe to restart the call after
    // aborting it.  We are wrong if it turns out that some
    // interruptible system call can make partial progress before we
    // abort it.
    // We think this is impossible because the proc manpage says EINTR
    // would be returned to the process if we didn't trap the syscall
    // exit, and the manpages for interruptible system calls say an
    // EINTR return value means no progress was made.
    // If we are wrong, this is probably the place to decide whether
    // and/or how the syscall should be restarted later.
    
    if (!get_status(&status)) return false;
    
    // Verify that we're stopped in the right place
    if (((status.pr_flags & (PR_STOPPED|PR_ISTOP))
         != (PR_STOPPED|PR_ISTOP))
        || status.pr_why != PR_SYSEXIT
        || status.pr_syscall != stoppedSyscall_) {
        sprintf(errorLine,
                "warn: Can't step paused process out of syscall (Verify)\n");
        logLine(errorLine);
        return 0;
    }
    
    proc_->set_syscalls(scsavedentry, scsavedexit);
    
    // Remember the current PC.  When we continue the process at this PC
    // we will restart the system call.
    postsyscallpc_ = (Address) GETREG_PC(status.pr_reg);
    
    return true;
}

// Stop the LWP in question
bool dyn_lwp::pauseLWP() {
  long command[2];
  command[0] = PCSTOP;
  if (write(ctl_fd(), command, sizeof(long)) != sizeof(long)) {
      perror("pauseLWP: PCSTOP");
      return false;
  }

  // We used to abort system calls automatically... but I'm not
  // sure why. We only need to manipulate the process occasionally
  // so we're not automatically aborting anymore. 

  // Testing: re-adding aborting mechanism
  if (executingSystemCall()) abortSyscall();
  

  return true;
}

// Get the active frame (PC, FP/SP)
Frame dyn_lwp::getActiveFrame()
{
  dyn_saved_regs *regs = getRegisters();
  if (!regs) return Frame();
  
  Frame newFrame = Frame(GETREG_PC(regs->theIntRegs),
                         GETREG_FP(regs->theIntRegs), 
                         proc_->getPid(), NULL, this, true);
  delete regs;
  return newFrame;
}

// Get the registers of the stopped thread and return them.
struct dyn_saved_regs *dyn_lwp::getRegisters()
{
    lwpstatus_t status;
    if (!get_status(&status)) return NULL;
    
    struct dyn_saved_regs *regs = new dyn_saved_regs();
    // Process must be stopped for register data to be correct.

    assert((status.pr_flags & PR_STOPPED) || (status.pr_flags & PR_ISTOP)
           || (status.pr_flags & PR_PCINVAL)  // eg. a trap at syscall exit
           );

    memcpy(&(regs->theIntRegs), &(status.pr_reg), sizeof(prgregset_t));
    memcpy(&(regs->theFpRegs), &(status.pr_fpreg), sizeof(prfpregset_t));

    return regs;
}

// Restore registers saved as above.
bool dyn_lwp::restoreRegisters(struct dyn_saved_regs *regs)
{
    // The fact that this routine can be shared between solaris/sparc and
    // solaris/x86 is just really, really cool.  /proc rules!
    int regbufsize = sizeof(long) + sizeof(prgregset_t);
    char regbuf[regbufsize]; long *regbufptr = (long *)regbuf;
    *regbufptr = PCSREG; regbufptr++;
    memcpy(regbufptr, &(regs->theIntRegs), sizeof(prgregset_t));

    if (write(ctl_fd(), regbuf, regbufsize) != regbufsize) {
        perror("restoreRegisters GPR write");
        sleep(10);
        
        return false;
    }
    
    int fpbufsize = sizeof(long) + sizeof(prfpregset_t);
    char fpbuf[fpbufsize]; long *fpbufptr = (long *)fpbuf;
    *fpbufptr = PCSFPREG; fpbufptr++;
    memcpy(fpbufptr, &(regs->theFpRegs), sizeof(prfpregset_t));
    
    if (write(ctl_fd(), fpbuf, fpbufsize) != fpbufsize) {
        perror("restoreRegisters FPR write");
        return false;
    }

    return true;

}

// Determine if the LWP in question is in a system call.
bool dyn_lwp::executingSystemCall()
{
  lwpstatus_t status;

  if (!get_status(&status)) return false;

  // Old-style
  if (status.pr_syscall > 0 && // If we're in a system call
      status.pr_why != PR_SYSEXIT) {

      stoppedSyscall_ = status.pr_syscall;
      if (abortSyscall()) {
          // Not in a syscall any more :)
          return false;
      }
      else 
          return true;
  }
  return false;
}


bool dyn_lwp::changePC(Address addr, struct dyn_saved_regs *regs)
{
    // Don't change the contents of regs if given
    dyn_saved_regs *local;

    if (!regs) {
        local = getRegisters();    
    }
    else {
        local = new dyn_saved_regs();
        memcpy(local, regs, sizeof(struct dyn_saved_regs));
    }
    
    // Compatibility: we don't use PCSVADDR on Solaris because AIX doesn't 
    // support it
    // nPC MUST be set before PC! On platforms that don't have it,
    // nPC will be overwritten by the PC write
    GETREG_nPC(local->theIntRegs) = addr + sizeof(instruction);
    GETREG_PC(local->theIntRegs) = addr;

    
    restoreRegisters(local);
    delete local;
    return true;
}

#if defined(i386_unknown_solaris2_5)
bool process::changeIntReg(int reg, Address val) {
   assert(status_ == stopped); // /proc will require this

   prgregset_t theIntRegs;
   if (ioctl(getDefaultLWP()->get_fd(), PIOCGREG, &theIntRegs) == -1) {
     perror("dyn_lwp::changeIntReg");
      if (errno == EBUSY) {
	 cerr << "It appears that the process wasn't stopped in the eyes of /proc" << endl;
	 assert(false);
      }
      return false;
   }

   theIntRegs[reg] = val;

   if (ioctl(getDefaultLWP()->get_fd(), PIOCSREG, &theIntRegs) == -1) {
      perror("process::changeIntReg PIOCSREG");
      if (errno == EBUSY) {
	 cerr << "It appears that the process wasn't stopped in the eyes of /proc" << endl;
	 assert(false);
      }
      return false;
   }

   return true;
}
#endif

// Utility function: get the appropriate lwpstatus_t struct
bool dyn_lwp::get_status(lwpstatus_t *status) const
{
   if (lwp_id_) {
      // We're using an lwp file descriptor, so get directly
      if (pread(status_fd_, 
                (void *)status, 
                sizeof(lwpstatus_t), 0) != sizeof(lwpstatus_t)) {
         perror("dyn_lwp::get_status");            
         return false;
      }
   }
   else {
      // No lwp, so we get the whole thing and pick it out
      pstatus_t procstatus;
      if (!proc_->get_status(&procstatus)) {
         return false;
      }
      memcpy(status, &(procstatus.pr_lwp), sizeof(lwpstatus_t));
   }
   return true;
}

// Read the value of a particular register
Address dyn_lwp::readRegister(Register reg)
{
    Address result;

    dyn_saved_regs *regs = getRegisters();
    result = GETREG_GPR(regs->theIntRegs,reg);
    delete regs;
    return result;
}

// Open all file descriptors corresponding to an LWP
bool dyn_lwp::openFD_()
{
  if (lwp_id_) {
      char temp[128];
      sprintf(temp, "/proc/%d/lwp/%d/lwpctl", (int)proc_->getPid(), lwp_id_);
      ctl_fd_ = P_open(temp, O_WRONLY, 0);
      if (ctl_fd_ < 0) perror("Opening lwpctl");
      sprintf(temp, "/proc/%d/lwp/%d/lwpstatus", (int)proc_->getPid(),lwp_id_);
      status_fd_ = P_open(temp, O_RDONLY, 0);    
      if (status_fd_ < 0) perror("Opening lwpstatus");
#if !defined(rs6000_ibm_aix4_1)
      sprintf(temp, "/proc/%d/lwp/%d/lwpusage", (int)proc_->getPid(), lwp_id_);
      usage_fd_ = P_open(temp, O_RDONLY, 0);
#else
      usage_fd_ = 0;
#endif
  }
  else {
      // No LWP = representative LWP
    char temp[128];
    sprintf(temp, "/proc/%d/ctl", (int)proc_->getPid());
    ctl_fd_ = P_open(temp, O_WRONLY | O_EXCL, 0);
    if (ctl_fd_ < 0) perror("Opening (LWP) ctl");
    //fprintf(stderr, "Opened %d for ctl\n", ctl_fd_);
    sprintf(temp, "/proc/%d/status", (int)proc_->getPid());
    status_fd_ = P_open(temp, O_RDONLY, 0);    
    if (status_fd_ < 0) perror("Opening (LWP) status");
#if !defined(rs6000_ibm_aix4_1)
    sprintf(temp, "/proc/%d/usage", (int)proc_->getPid());
    usage_fd_ = P_open(temp, O_RDONLY, 0);    
    if (usage_fd_ < 0) perror("Opening (LWP) usage");
#else
    usage_fd_ = 0;
#endif
  }
  return true;
}

// Close FDs opened above
void dyn_lwp::closeFD_()
{
    if (ctl_fd_) close(ctl_fd_);
    if (status_fd_) close(status_fd_);
    if (usage_fd_) close(usage_fd_);    
}

/*
 * Process-wide /proc operations
 */

#ifdef BPATCH_LIBRARY
/*
 * Use by dyninst to set events we care about from procfs
 *
 */
bool process::setProcfsFlags()
{
    long command[2];
    command[0] = PCSET;
    long flags = PR_BPTADJ;
    if (BPatch::bpatch->postForkCallback) {
        // cause the child to inherit trap-on-exit from exec and other traps
        // so we can learn of the child (if the user cares)
       flags = PR_BPTADJ | PR_FORK | PR_ASYNC | PR_RLC | PR_MSACCT;
    }
    command[1] = flags;

    if (write(getDefaultLWP()->ctl_fd(), command, 2*sizeof(long)) != 2*sizeof(long)) {
        perror("setProcfsFlags: PCSET");
        return false;
    }

    // cause a stop on the exit from fork
    sysset_t *entryset = SYSSET_ALLOC(getPid());
    sysset_t *exitset = SYSSET_ALLOC(getPid());
    premptysysset(entryset);
    premptysysset(exitset);
    pstatus_t status;

    // Get traced system calls (entry and exit)
    if (pread(status_fd(), &status, sizeof(status), 0) != sizeof(status))
        return false;

    //if (!get_entry_syscalls(&status, entryset)) return false;
    //if (!get_exit_syscalls(&status, exitset)) return false;
    
    if (BPatch::bpatch->postForkCallback) {
        if (SYSSET_MAP(SYS_fork, getPid()) != -1) {
            praddsysset (exitset, SYSSET_MAP(SYS_fork, getPid()));
        }
        
        if (SYSSET_MAP(SYS_fork1, getPid()) != -1) {
            praddsysset (exitset, SYSSET_MAP(SYS_fork1, getPid()));
        }
        
        if (SYSSET_MAP(SYS_vfork, getPid()) != -1) {
            praddsysset (exitset, SYSSET_MAP(SYS_vfork, getPid()));
        }
    }
    if (BPatch::bpatch->execCallback) {
        if (SYSSET_MAP(SYS_exec, getPid()) != -1)
           praddsysset (exitset, SYSSET_MAP(SYS_exec, getPid()));
        if (SYSSET_MAP(SYS_execve, getPid()) != -1)
           praddsysset (exitset, SYSSET_MAP(SYS_execve, getPid()));
    }
    if (BPatch::bpatch->exitCallback) {
        if (SYSSET_MAP(SYS_exit, getPid()) != -1)
            praddsysset (entryset, SYSSET_MAP(SYS_exit, getPid()));
    }
    
    if (BPatch::bpatch->preForkCallback) {
        if (SYSSET_MAP(SYS_fork, getPid()) != -1)
            praddsysset (entryset, SYSSET_MAP(SYS_fork, getPid()));
        if (SYSSET_MAP(SYS_fork1, getPid()) != -1)
            praddsysset (entryset, SYSSET_MAP(SYS_fork1, getPid()));
        if (SYSSET_MAP(SYS_vfork, getPid()) != -1)
            praddsysset (entryset, SYSSET_MAP(SYS_vfork, getPid()));
    }
    
    return set_syscalls(entryset, exitset);    
}
#endif
/*
   Open the /proc file correspoding to process pid, 
   set the signals to be caught to be only SIGSTOP and SIGTRAP,
   and set the kill-on-last-close and inherit-on-fork flags.
*/
extern string pd_flavor ;
bool process::attach_() {
    // step 1) /proc open: attach to the inferior process

    //cerr << "Attaching... " << endl;
    dyn_lwp *default_lwp = getDefaultLWP();
    if(default_lwp == NULL)
       return false;

    // Open the process-wise handles
    char temp[128];
    as_fd_ = -1;
    sprintf(temp, "/proc/%d/as", getPid());
    as_fd_ = P_open(temp, O_RDWR, 0);
    if (as_fd_ < 0) perror("Opening as fd");
#if !defined(rs6000_ibm_aix4_1)
    sprintf(temp, "/proc/%d/auxv", getPid());
    auxv_fd_ = P_open(temp, O_RDONLY, 0);
    if (auxv_fd_ < 0) perror("Opening auxv fd");
#else
    // AIX doesn't have the auxv file
    auxv_fd_ = 0;
#endif

    sprintf(temp, "/proc/%d/map", getPid());
    map_fd_ = P_open(temp, O_RDONLY, 0);
    if (map_fd_ < 0) perror("map fd");

    sprintf(temp, "/proc/%d/psinfo", getPid());
    ps_fd_ = P_open(temp, O_RDONLY, 0);
    if (ps_fd_ < 0) perror("Opening ps fd");

    sprintf(temp, "/proc/%d/status", getPid());
    status_fd_ = P_open(temp, O_RDONLY, 0);
    if (status_fd_ < 0) perror("Opening status fd");
    
    // step 2) /proc PIOCSTRACE: define which signals should be forwarded to daemon
    //   These are (1) SIGSTOP and (2) either SIGTRAP (sparc) or SIGILL (x86), to
    //   implement inferiorRPC completion detection.
    
    proc_sigset_t sigs;
    
    premptyset(&sigs);
    praddset(&sigs, SIGSTOP);
    
#ifndef i386_unknown_solaris2_5
    praddset(&sigs, SIGTRAP);
#else
    praddset(&sigs, SIGILL);
#endif    
    int bufsize = sizeof(long) + sizeof(proc_sigset_t); char buf[bufsize]; 
    long *bufptr = (long *) buf;
    *bufptr = PCSTRACE; bufptr++;
    memcpy(bufptr, &sigs, sizeof(proc_sigset_t));

    if(write(getDefaultLWP()->ctl_fd(), buf, bufsize) != bufsize) {
        perror("attach: PCSTRACE");
        return false;
    }

    // Step 3) /proc PIOCSET:
    // a) turn on the reset-on-last-close flag (undoes changes when last
    //    proc fd is closed)
    // b) turn on inherit-on-fork flag (tracing flags inherited when
    // child forks) in Paradyn only
    // c) turn on breakpoint trap pc adjustment (x86 only).
    // Also, any child of this process will stop at the exit of an exec call.
    
#ifdef BPATCH_LIBRARY
    setProcfsFlags();
#else
    // Paradyn only uses a limited subset of Dyninst's flags
    long command[2];
    command[0] = PCSET;
  
    if(process::pdFlavor == string("cow") || process::pdFlavor == string("mpi"))
        command[1] = PR_KLC |  PR_BPTADJ | PR_MSACCT;
    else
        command[1] = PR_KLC | PR_FORK | PR_BPTADJ | PR_MSACCT;
    if (write(getDefaultLWP()->ctl_fd(), command, 2*sizeof(long)) != 2*sizeof(long)) {
        perror("setProcfsFlags: PCSET");
        return false;
    }
#endif
    if (!get_ps_info(getPid(), this->argv0, this->cwdenv, this->pathenv)) return false;
    return true;
}

bool process::isRunning_() const {
    // We can key off the representative LWP
    return getDefaultLWP()->isRunning();
}

bool process::continueProc_() {
  ptraceOps++; ptraceOtherOps++;
  return getDefaultLWP()->continueLWP();
#if 0
  // This doesn't work -- dyninst/ST paradyn are okay,
  // since they are equivalent. But MT breaks.
  if (threads.size() == 0) {
      return getDefaultLWP()->continueLWP();
  }
  else {
      bool success = true;
      for (unsigned i = 0; i < threads.size(); i++) {
          dyn_lwp *lwp = threads[i]->get_lwp();
          if (!lwp) continue;
          if (!lwp->continueLWP())
              success = false;
      }
      
      return success;
  }
  return true;
#endif
}

#ifdef BPATCH_LIBRARY
/*
   terminate execution of a process
 */
bool process::terminateProc_()
{
    // Dummy check
    if (status_ == exited) return true;
    long command[2];
    command[0] = PCKILL;
    command[1] = SIGKILL;
    dyn_lwp *cntl_lwp = getDefaultLWP();
    if (cntl_lwp) 
        if (write(cntl_lwp->ctl_fd(), 
                  command, 2*sizeof(long)) != 2*sizeof(long)) {
            perror("terminateProc: PCKILL");
            return false;
        }
    Exited();
    
    return true;
}
#endif

/*
   pause a process that is running
*/

bool process::pause_() {
    ptraceOps++; ptraceOtherOps++;
    return getDefaultLWP()->pauseLWP();
    // This code doesn't work. I'm leaving it here as an example -- bernat
    if (threads.size() == 0) {
        return getDefaultLWP()->pauseLWP();
    }
    else {
        bool success = true;
        for (unsigned i = 0; i < threads.size(); i++) {
            dyn_lwp *lwp = threads[i]->get_lwp();
            if (!lwp) continue;
            
            if (!lwp->pauseLWP())
                success = false;
        }      
        return success;
    }
    return true;
}

/*
   close the file descriptor for the file associated with a process
*/
bool process::detach_() {
   close(procHandle_);
   close(as_fd_);
   close(ps_fd_);
   close(status_fd_);

   return true;
}

#ifdef BPATCH_LIBRARY
/*
   detach from thr process, continuing its execution if the parameter "cont"
   is true.
 */
bool process::API_detach_(const bool cont)
{
  // Remove the breakpoint that we put in to detect loading and unloading of
  // shared libraries.
  // XXX We might want to move this into some general cleanup routine for the
  //     dynamic_linking object.
#if !defined(rs6000_ibm_aix4_1)
  if (dyn) {
      dyn->unset_r_brk_point(this);
  }
#endif
  // Reset the kill-on-close flag, and the run-on-last-close flag if necessary
  long command[2];
  command[0] = PCUNSET;
  command[1] = PR_KLC; // Get rid of kill-on-close
  if (!cont) command[1] |= PR_RLC; // Don't want run on last close

  if (write(getDefaultLWP()->ctl_fd(), 
            command, 2*sizeof(long)) != 2*sizeof(long)) {
      perror("apiDetach: PCUNSET");
      return false;
  }
  if (cont) {
      command[0] = PCSET;
      command[1] = PR_RLC;
      if (write(getDefaultLWP()->ctl_fd(), command, 2*sizeof(long)) != 2*sizeof(long)) {
          perror("apiDetach: PCSET");
          return false;
      }
  }

  proc_sigset_t sigs;
  premptyset(&sigs);
  int sigbufsize = sizeof(long) + sizeof(proc_sigset_t);
  char sigbuf[sigbufsize]; long *sigbufptr = (long *)sigbuf;
  *sigbufptr = PCSHOLD; sigbufptr++;
  memcpy(sigbufptr, &sigs, sizeof(proc_sigset_t));
  
  if (write(getDefaultLWP()->ctl_fd(), sigbuf, sigbufsize) != sigbufsize) {
      perror("apiDetach: PCSHOLD");
      return false;
  }

  // Same set gets PCSTRACEd
  sigbufptr = (long *)sigbuf;
  *sigbufptr = PCSTRACE;
  if (write(getDefaultLWP()->ctl_fd(), sigbuf, sigbufsize) != sigbufsize) {
      perror("apiDetach: PCSTRACE");
      return false;
  }
  

  fltset_t faults;
  premptyset(&faults);
  int fltbufsize = sizeof(long) + sizeof(fltset_t);
  char fltbuf[fltbufsize]; long *fltbufptr = (long *)fltbuf;
  *fltbufptr = PCSFAULT; fltbufptr++;
  memcpy(fltbufptr, &faults, sizeof(fltset_t));
  
  if (write(getDefaultLWP()->ctl_fd(), fltbuf, fltbufsize) != fltbufsize) {
      perror("apiDetach: PCSFAULT");
      return false;
  }
  
  sysset_t *syscalls = SYSSET_ALLOC(getPid());
  premptyset(syscalls);
  int sysbufsize = sizeof(long) + SYSSET_SIZE(syscalls);
  char sysbuf[sysbufsize]; long *sysbufptr = (long *)sysbuf;
  *sysbufptr = PCSENTRY; sysbufptr++;
  memcpy(sysbufptr, syscalls, SYSSET_SIZE(syscalls));
  
  if (write(getDefaultLWP()->ctl_fd(), sysbuf, sysbufsize) != sysbufsize) {
      perror("apiDetach: PCSENTRY");
      return false;
  }
  
  sysbufptr = (long *)sysbuf;
  *sysbufptr = PCSEXIT;
  if (write(getDefaultLWP()->ctl_fd(), sysbuf, sysbufsize) != sysbufsize) {
      perror("apiDetach: PCSEXIT");
      return false;
  }

  // Close all file descriptors
  detach(false);
  
  return true;
}
#endif

bool process::writeTextWord_(caddr_t inTraced, int data) {
//  cerr << "writeTextWord @ " << (void *)inTraced << endl; cerr.flush();
  return writeDataSpace_(inTraced, sizeof(int), (caddr_t) &data);
}

bool process::writeTextSpace_(void *inTraced, u_int amount, const void *inSelf) {
//  cerr << "writeTextSpace pid=" << getPid() << ", @ " << (void *)inTraced << " len=" << amount << endl; cerr.flush();
  return writeDataSpace_(inTraced, amount, inSelf);
}

bool process::readTextSpace_(void *inTraced, u_int amount, const void *inSelf) {
  return readDataSpace_(inTraced, amount, const_cast<void *>(inSelf));
}

bool process::writeDataSpace_(void *inTraced, u_int amount, const void *inSelf) {
  ptraceOps++; ptraceBytes += amount;

//  cerr << "process::writeDataSpace_ pid " << getPid() << " writing " << amount << " bytes at loc " << inTraced << endl;
#if defined(BPATCH_LIBRARY)
#if defined (sparc_sun_solaris2_4)
	if(collectSaveWorldData &&  ((Address) inTraced) > getDyn()->getlowestSObaseaddr() ){
		shared_object *sh_obj = NULL;
		bool result = false;
		for(unsigned i = 0; shared_objects && !result && i<shared_objects->size();i++){
			sh_obj = (*shared_objects)[i];
			result = sh_obj->isinText((Address) inTraced);
		}
		if( result  ){
		/*	printf(" write at %lx in %s amount %x insn: %x \n", 
				(off_t)inTraced, sh_obj->getName().c_str(), amount,
				 *(unsigned int*) inSelf);
		*/	
			sh_obj->setDirty();	
		}
	}
#endif
#endif
    off64_t loc;
    // Problem: we may be getting a address with the high bit
    // set. So how to convince the system that it's not negative?
    loc = (off64_t) ((unsigned) inTraced);

    if (pwrite64(as_fd(), inSelf, amount, loc) != (int)amount) {
        perror("writeDataSpace");
        return false;
    }
    
    return true;
}

bool process::readDataSpace_(const void *inTraced, u_int amount, void *inSelf) {
    ptraceOps++; ptraceBytes += amount;

    off64_t loc;
    loc = (off64_t) ((unsigned) inTraced);
    
    if (pread64(as_fd(), inSelf, amount, loc) != (int)amount) {
        perror("readDataSpace");
        return false;
    }
    return true;
}


bool process::get_status(pstatus_t *status) const
{
    if (pread(status_fd(), (void *)status,
              sizeof(pstatus_t), 0) != sizeof(pstatus_t)) {
        perror("process::get_status");
        return false;
    }
    return true;
}

bool process::set_syscalls(sysset_t *entry, sysset_t *exit) const
{
    int bufentrysize = sizeof(long) + SYSSET_SIZE(entry);
    char bufentry[bufentrysize]; long *bufptr = (long *)bufentry;

    // Write entry syscalls
    *bufptr = PCSENTRY; bufptr++;
    memcpy(bufptr, entry, SYSSET_SIZE(entry));
    if (write(getDefaultLWP()->ctl_fd(), 
              bufentry, bufentrysize) != bufentrysize) return false;

    int bufexitsize = sizeof(long) + SYSSET_SIZE(exit);
    char bufexit[bufexitsize]; bufptr = (long *)bufexit;
    *bufptr = PCSEXIT; bufptr++;
    memcpy(bufptr, exit, SYSSET_SIZE(exit));
    if (write(getDefaultLWP()->ctl_fd(), 
              bufexit, bufexitsize) != bufexitsize) return false;
    return true;    
}

syscallTrap *process::trapSyscallExitInternal(Address syscall)
{
    syscallTrap *trappedSyscall = NULL;

    // First, the cross-platform bit. If we're already trapping
    // on this syscall, then increment the reference counter
    // and return

    for (unsigned iter = 0; iter < syscallTraps_.size(); iter++) {
        if (syscallTraps_[iter]->syscall_id == (int) syscall) {
            trappedSyscall = syscallTraps_[iter];
            break;
        }
    }
    if (trappedSyscall) {
        // That was easy...
        trappedSyscall->refcount++;
        return trappedSyscall;
    }
    else {
        // Okay, we haven't trapped this system call yet.
        // Things to do:
        // 1) Get the original value
        // 2) Place a trap
        // 3) Create a new syscallTrap object and return it

        trappedSyscall = new syscallTrap;
        trappedSyscall->refcount = 1;
        trappedSyscall->syscall_id = (int) syscall;

        sysset_t *cur_syscalls = SYSSET_ALLOC(getPid());
        pstatus_t proc_status;
        if (!get_status(&proc_status)) return NULL;
        if (!get_exit_syscalls(&proc_status, cur_syscalls)) return NULL;
        if (prissyssetmember(cur_syscalls, trappedSyscall->syscall_id))
            // Surprising case... but it's possible as we trap fork and exec
            trappedSyscall->orig_setting = 1;
        else
            trappedSyscall->orig_setting = 0;
        
        // 2) Place a trap
        praddsysset(cur_syscalls, trappedSyscall->syscall_id);
        int bufsize = sizeof(long) + SYSSET_SIZE(cur_syscalls);
        char buf[bufsize]; long *bufptr = (long *)buf;
        *bufptr = PCSEXIT; bufptr++;
        memcpy(bufptr, cur_syscalls, SYSSET_SIZE(cur_syscalls));
        if (write(getDefaultLWP()->ctl_fd(), buf, bufsize) != bufsize) {
            perror("Syscall trap set");
            return NULL;
        }

        // Insert into the list of trapped syscalls
        syscallTraps_ += (trappedSyscall);

        return trappedSyscall;
    }
    // Should never reach here.
    return NULL;
}

bool process::clearSyscallTrapInternal(syscallTrap *trappedSyscall) {
    // Decrement the reference count, and if it's 0 remove the trapped
    // system call
    assert(trappedSyscall->refcount > 0);
    
    trappedSyscall->refcount--;
    if (trappedSyscall->refcount > 0) {
        return true;
    }
    // Erk... it hit 0. Remove the trap at the system call
    sysset_t *cur_syscalls = SYSSET_ALLOC(getPid());
    pstatus_t proc_status;
    if (!get_status(&proc_status)) return false;
    if (!get_exit_syscalls(&proc_status, cur_syscalls)) return false;
    if (!trappedSyscall->orig_setting) 
        prdelsysset(cur_syscalls, trappedSyscall->syscall_id);
    int bufsize = sizeof(long) + SYSSET_SIZE(cur_syscalls);
    char buf[bufsize]; long *bufptr = (long *)buf;
    *bufptr = PCSEXIT; bufptr++;
    memcpy(bufptr, cur_syscalls, SYSSET_SIZE(cur_syscalls));
    if (write(getDefaultLWP()->ctl_fd(), buf, bufsize) != bufsize) {
        perror("Syscall trap clear");
        return false;
    }
    
    // Now that we've reset the original behavior, remove this
    // entry from the vector
    for (unsigned iter = 0; iter < syscallTraps_.size(); iter++) {
        if (trappedSyscall == syscallTraps_[iter]) {
            syscallTraps_.removeByIndex(iter);
            break;
        }
    }
    delete trappedSyscall;
    
    return true;
}

Address dyn_lwp::getCurrentSyscall() {
    // Return the system call we're currently in
    lwpstatus_t status;
    if (!get_status(&status)) {
        fprintf(stderr, "Failed to get status\n");
        return 0;
    }

    return status.pr_syscall;
}



bool dyn_lwp::stepPastSyscallTrap()
{
    // Nice... on /proc systems we don't need to do anything here
    return true;
}

// Check if a LWP has hit a trap we inserted to catch the exit of a
// system call trap.
// Return values:
// 0: did not trap at the exit of a syscall
// 1: Trapped, but did not have a syscall trap placed for this LWP
// 2: Trapped with a syscall trap desired.
int dyn_lwp::hasReachedSyscallTrap() {
    // Get the status of the LWP
    lwpstatus_t status;
    if (!get_status(&status)) {
        fprintf(stderr, "Failed to get thread status\n");
        return false;
    }
    
    if (status.pr_why != PR_SYSEXIT) {
        return false;
    }
    Address syscall = status.pr_what;

    if (trappedSyscall_ && syscall == trappedSyscall_->syscall_id) {
        return 2;
    }
    
    // Unfortunately we can't check a recorded system call trap,
    // since we don't have one saved. So do a scan through all traps the
    // process placed
    if (proc()->checkTrappedSyscallsInternal(syscall))
        return 1;
    
    return 0;
}

procSyscall_t decodeSyscall(process *p, procSignalWhat_t syscall)
{
    if (syscall == SYSSET_MAP(SYS_fork, p->getPid()) ||
        syscall == SYSSET_MAP(SYS_fork1, p->getPid()) ||
        syscall == SYSSET_MAP(SYS_vfork, p->getPid()))
        return procSysFork;
    if (syscall == SYSSET_MAP(SYS_exec, p->getPid()) || 
        syscall == SYSSET_MAP(SYS_execve, p->getPid()))
        return procSysExec;
    if (syscall == SYSSET_MAP(SYS_exit, p->getPid()))
        return procSysExit;
    return procSysOther;
}



int decodeProcStatus(process *proc,
                     lwpstatus_t status,
                     procSignalWhy_t &why,
                     procSignalWhat_t &what,
                     procSignalInfo_t &info) {
    info = status.pr_reg[R_O0];
    
    switch (status.pr_why) {
  case PR_SIGNALLED:
      why = procSignalled;
      what = status.pr_what;
      break;
  case PR_SYSENTRY:
      why = procSyscallEntry;
      what = status.pr_what;
      break;
  case PR_SYSEXIT:
      why = procSyscallExit;
      what = status.pr_what;
      break;
  case PR_REQUESTED:
      // We don't expect PR_REQUESTED in the signal handler
      assert(0 && "PR_REQUESTED not handled");
      break;
  case PR_JOBCONTROL:
  case PR_FAULTED:
  case PR_SUSPENDED:
  default:
      assert(0);
      break;
    }
    return 1;
}

// Get and decode a signal for a process
// We poll /proc for process events, and so it's possible that
// we'll get multiple hits. In this case we return one and queue
// the rest. Further calls of decodeProcessEvent will burn through
// the queue until there are none left, then re-poll.
// Return value: 0 if nothing happened, or process pointer

process *decodeProcessEvent(int pid,
                            procSignalWhy_t &why,
                            procSignalWhat_t &what,
                            procSignalInfo_t &info,
                            bool block) {
    why = procUndefined;
    what = 0;
    info = 0;

    extern pdvector<process*> processVec;
    static struct pollfd fds[OPEN_MAX];  // argument for poll
    // Number of file descriptors with events pending
    static int selected_fds = 0; 
    // The current FD we're processing.
    static int curr = 0;

    if (selected_fds == 0) {
        for (unsigned u = 0; u < processVec.size(); u++) {
            //printf("checking %d\n", processVec[u]->getPid());
            if (processVec[u] && 
                (processVec[u]->status() == running || 
                 processVec[u]->status() == neonatal)) {
                if (pid == -1 ||
                    processVec[u]->getPid() == pid)
                    fds[u].fd = processVec[u]->status_fd();
            } else {
                fds[u].fd = -1;
            }	
            fds[u].events = POLLPRI;
            fds[u].revents = 0;
        }
        
        
        int timeout;
        if (block) timeout = -1;
        else timeout = 0;
        selected_fds = poll(fds, processVec.size(), timeout);
        
        if (selected_fds <= 0) {
            if (selected_fds < 0) {
                perror("decodeProcessEvent: poll failed");
                selected_fds = 0;
            }
            return NULL;
        }
        
        // Reset the current pointer to the beginning of the poll list
        curr = 0;
    } // if selected_fds == 0
    // We have one or more events to work with.
    while (fds[curr].revents == 0) {
        // skip
        ++curr;
    }

    // fds[curr] has an event of interest
    lwpstatus_t procstatus;
    process *currProcess = processVec[curr];
    
    if (fds[curr].revents & POLLHUP) {
        // True if the process exited out from under us
        int status;
        int ret;
        do {
            ret = waitpid(currProcess->getPid(), &status, 0);
        } while ((ret < 0) && (errno == EINTR));
        if (ret < 0) {
            // This means that the application exited, but was not our child
            // so it didn't wait around for us to get it's return code.  In
            // this case, we can't know why it exited or what it's return
            // code was.
            ret = currProcess->getPid();
            status = 0;
            // is this the bug??
            // processVec[curr]->continueProc_();
        }
        if (!decodeWaitPidStatus(currProcess, status, why, what))
            return NULL;

    } else {
        // Real return from poll
        if (currProcess->getDefaultLWP()->get_status(&procstatus)) {
            // Check if the process is stopped waiting for us
            if (procstatus.pr_flags & PR_STOPPED ||
                procstatus.pr_flags & PR_ISTOP) {
                if (!decodeProcStatus(currProcess, procstatus, why, what, info))
                    return NULL;
            }
        }
        else {
            // get_status failed, probably because the process doesn't exist
        }
    }
    // Skip this FD the next time through
    --selected_fds;
    ++curr;    
    return currProcess;
    
} 

// Utility function: given an address and a file descriptor,
// read and return the string there

string extract_string(int fd, Address addr)
{
    string result;
    char buffer[81];
    lseek(fd, addr, SEEK_SET);
    while (1) {
        read(fd, buffer, 80);
        buffer[80] = 0; // Null-terminate
        result += buffer;
        for (int i = 0; i < 80; i++) 
            if (buffer[i] == 0)
                return result;
    }
    return "";
}

bool get_ps_info(int pid, string &argv0, string &cwdenv, string &pathenv)
{
    char psPath[256];
    sprintf(psPath, "/proc/%d/psinfo", pid);
    int ps_fd = P_open(psPath, O_RDONLY, 0);
    if (ps_fd < 0) return false;
    
    psinfo_t procinfo;
    if (pread(ps_fd, &procinfo, sizeof(procinfo), 0) != sizeof(procinfo)) {
        close(ps_fd);
        return false;
    }
    
    if (procinfo.pr_nlwp == 0) {
        close(ps_fd);
        return false;
    }
    
    assert(procinfo.pr_argv);
    // We can get the argv vector, which will give us the process name
    // Problem is, everything is in that process' address space. So we need
    // the as file descriptor
    char asPath[256];
    sprintf(asPath, "/proc/%d/as", pid);
    int as_fd;
    as_fd = P_open(asPath, O_RDONLY, 0);
    if (as_fd < 0) {
        close(ps_fd);
        return false;
    }   
    char *ptr_to_argv0;
    if (pread(as_fd, &ptr_to_argv0, sizeof(char *), procinfo.pr_argv) != sizeof(char *)) {
        close(ps_fd);
        close(as_fd);
        return false;
    }
    
    // Now the fun begins... we need to read the process name. Problem is, we don't know how
    // big a string to read. So we use the "incremental read and glue" approach.
    // argv0: process member
    argv0 = extract_string(as_fd, (Address) ptr_to_argv0);
    // Now that we have the process, get PWD and PATH
    char **envptr = (char **)procinfo.pr_envp;
    bool need_path = true;
    bool need_pwd = true;
    while(need_path || need_pwd) { //ccw 30 jan 2003
        char *env;
        if (pread(as_fd, &env, sizeof(char *), (Address) envptr) != sizeof(char *)) {
            close(ps_fd);
            close(as_fd);
            return false;
        }
        if (env == NULL)
            break;
        string env_value = extract_string(as_fd, (Address) env);
        //cerr << "Environment string: " << env_value << endl;
        if (env_value.prefixed_by("PWD=") || env_value.prefixed_by("CWD=")) {
            cwdenv = env_value.c_str() + 4; // Skip CWD= or PWD=
            need_pwd = false;
        }
        if (env_value.prefixed_by("PATH=")) {
            pathenv = env_value.c_str() + 5; // Skip PATH=
            need_path = false;
        }
        envptr++;
    }
    
    close(as_fd);
    close(ps_fd);
    return true;
}



string process::tryToFindExecutable(const string &iprogpath, int pid) {
   // returns empty string on failure.
   // Otherwise, returns a full-path-name for the file.  Tries every
   // trick to determine the full-path-name, even though "progpath" may be
   // unspecified (empty string).
   
   // Remember, we can always return the empty string...no need to
   // go nuts writing the world's most complex algorithm.

   const string progpath = expand_tilde_pathname(iprogpath);

   // Trivial case: if "progpath" is specified and the file exists then nothing needed
   if (exists_executable(progpath)) {
      return progpath;
   }

   // Finding by name didn't work, so try option 2: the PID. 
   // We need to open a fresh file pointer, as the PID we were given may not
   // be the PID of this process object (this whole routine is more properly
   // a utility function)
   string result;
   string argv0;
   string cwdenv;
   string pathenv;
   if (!get_ps_info(pid, argv0, cwdenv, pathenv))
       return "";
   //cerr << "Argv 0: " << argv0 << " cwd: " << cwdenv << " path: " << pathenv << endl;
   
   if (executableFromArgv0AndPathAndCwd(result, argv0, pathenv, cwdenv)) {
       return result;
   }
   return "";
}




