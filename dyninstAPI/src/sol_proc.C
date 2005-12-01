/*
 * Copyright (c) 1996-2004 Barton P. Miller
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

// Solaris-style /proc support

// $Id: sol_proc.C,v 1.74 2005/12/01 00:56:25 jaw Exp $

#ifdef AIX_PROC
#include <sys/procfs.h>
#else
#include <procfs.h>
#endif
#include <limits.h>
#include <poll.h>
#include <sys/types.h>  // for reading lwps out of proc
#include <dirent.h>     // for reading lwps out of proc
#include "common/h/headers.h"
#include "dyninstAPI/src/signalhandler.h"
#include "dyninstAPI/src/process.h"
#include "dyninstAPI/src/dyn_lwp.h"
#include "dyninstAPI/src/dyn_thread.h"
#include "dyninstAPI/src/stats.h"
#include "common/h/pathName.h" // for path name manipulation routines
#include "dyninstAPI/src/sol_proc.h"

#include "function.h"
#include "mapped_object.h"
#include "mapped_module.h"
#include "dynamiclinking.h" //getlowestSO...

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

    char *buf = new char[bufsize];
    long *bufptr = (long *)buf;
    char procName[128];
    
    // Get the current set of syscalls
    pstatus_t status;
    sprintf(procName,"/proc/%d/status", (int) getpid());
    int stat_fd = P_open(procName, O_RDONLY, 0);
    if (pread(stat_fd, (void *)&status, sizeof(pstatus_t), 0) !=
        sizeof(pstatus_t)) {
        perror("osTraceMe::pread");
        SYSSET_FREE(exitSet);
        delete [] buf;
        return;
    }
#if defined(AIX_PROC) 
    if (status.pr_sysexit_offset)
        pread(stat_fd, exitSet, SYSSET_SIZE(exitSet), status.pr_sysexit_offset);
    else // No syscalls are being traced 
        premptysysset(exitSet);
#else
    memcpy(exitSet, &(status.pr_sysexit), SYSSET_SIZE(exitSet));
#endif
    P_close(stat_fd);

    sprintf(procName,"/proc/%d/ctl", (int) getpid());
    int fd = P_open(procName, O_WRONLY, 0);
    if (fd < 0) {
        perror("open");
        SYSSET_FREE(exitSet);
        delete [] buf;
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

    SYSSET_FREE(exitSet);
    delete [] buf;
}
/*
 * DYN_LWP class
 * Operations on LWP file descriptors, or the representative LWP
 */

// determine if a process is running by doing low-level system checks, as
// opposed to checking the 'status_' member vrble.  May assume that attach()
// has run, but can't assume anything else.

bool dyn_lwp::isRunning() const {
   procProcStatus_t theStatus;
   
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
bool dyn_lwp::continueLWP_(int signalToContinueWith) {
  procProcStatus_t status;
  long command[2];
  Address pc;  // PC at which we are trying to continue
  // Check the current status
  if (!get_status(&status)) {
      bperr( "Failed to get LWP status\n");
      return false;
  }

  if ((0==status.pr_flags & PR_STOPPED) && (0==status.pr_flags & PR_ISTOP)) {
      bperr( "LWP not stopped\n");
      return false;
  }


  // If the lwp is stopped on a signal we blip it (technical term).
  // The process will re-stop (since we re-run with PRSTOP). At
  // this point we continue it.
  if ((status.pr_flags & PR_STOPPED)
      && (status.pr_why == PR_SIGNALLED)) {
      if (status.pr_what == SIGSTOP || 
          status.pr_what == SIGINT ||
          status.pr_what == SIGTRAP) {
          clearSignal();
      }
  }

  ptraceOps++; 
  ptraceOtherOps++;


  pc = (Address)(GETREG_PC(status.pr_reg));
  if (stoppedInSyscall_ && pc == postsyscallpc_) {
      if (!restartSyscall()) return false;
  }
  
  command[0] = PCRUN;
  if (signalToContinueWith == dyn_lwp::NoSignal)
      command[1] = PRCSIG;  // clear the signal
  else {
      command[1] = 0;
  }

  // Continue the process the easy way
 busy_retry:
  errno = 0;
  if (write(ctl_fd(), command, 2*sizeof(long)) != 2*sizeof(long)) {
      if (errno == EBUSY) {
        struct timeval slp;
        slp.tv_sec = 0;
        slp.tv_usec = 1 /*ms*/ *1000;
        select(0, NULL, NULL, NULL, &slp);
        fprintf(stderr, "continueLWP_, ctl_fd is busy, trying again\n", FILE__, __LINE__);
        goto busy_retry;
      }
      perror("continueLWP: PCRUN2");
      return false;
  }
  
  return true;
  
}

// Abort a system call. Place a trap at the exit of the syscall in
// question, and run with the ABORT flag set. Wait for the trap
// to be hit and then return.

bool dyn_lwp::abortSyscall()
{
    lwpstatus_t status;

    // MT: aborting syscalls does not work. Maybe someone with better knowledge
    // of Solaris can get it working. 
    if(proc_->multithread_capable(true))
       return false;

    // We do not expect to recursively interrupt system calls.  We could
    // probably handle it by keeping a stack of system call state.  But
    // we haven't yet seen any reason to have this functionality.
    assert(!stoppedInSyscall_);
    stoppedInSyscall_ = true;
    
    if (!get_status(&status)) return false;

    if (status.pr_syscall == 0 ||
        status.pr_why == PR_SYSEXIT) {
        // No work to be done
        stoppedInSyscall_ = false;
        return true;
    }

    if (((status.pr_flags & PR_STOPPED) == 0) &&
        ((status.pr_flags & PR_ISTOP) == 0))
        stop_();
    

    sysset_t* scexit = SYSSET_ALLOC(proc_->getPid());
    sysset_t* scsavedexit = SYSSET_ALLOC(proc_->getPid());
    sysset_t* scentry = SYSSET_ALLOC(proc_->getPid());
    sysset_t* scsavedentry = SYSSET_ALLOC(proc_->getPid());
    
    // 1. Save the syscall number, registers, and blocked signals
    stoppedSyscall_ = status.pr_syscall;
/*
    memcpy(&(syscallreg_->theIntRegs), &(status.pr_reg), sizeof(prgregset_t));
    memcpy(&(syscallreg_->theFpRegs), &(status.pr_fpreg), sizeof(prfpregset_t));
*/   
 
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
    
    syscallreg_.theIntRegs[PC_REG] -= 7;
#endif
    
    // 2. Abort the system call
    
    // Save current syscall exit traps
    proc_->get_entry_syscalls(scsavedentry);
    proc_->get_exit_syscalls(scsavedexit);
    
    // Set process to trap on exit from this system call
    premptyset(scentry);
    premptyset(scexit);
    praddset(scexit, stoppedSyscall_);
    
    proc_->set_entry_syscalls(scentry);
    proc_->set_exit_syscalls(scexit);
    
    // Continue, aborting this system call and blocking all sigs except
    // those needed by DynInst.
    proc_sigset_t sigs;
    prfillset(&sigs);
    prdelset(&sigs, SIGTRAP);
    prdelset(&sigs, SIGILL);
    
    // Set the signals to "ignore" and run, aborting the syscall
#if 0
    int bufsize = sizeof(long) + sizeof(proc_sigset_t);
    char buf[bufsize]; long *bufptr = (long *)buf;
    *bufptr = PCSHOLD; bufptr++;
    memcpy(bufptr, &sigs, sizeof(proc_sigset_t));

    if (write(ctl_fd(), buf, bufsize) != bufsize) {
        perror("abortSyscall: PCSHOLD"); return false;
    }
#endif
    long command[2];
    
    command[0] = PCRUN;
    command[1] = PRSABORT;

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


    // Get a copy of the registers and save for when we restart the syscall
    syscallreg_ = new dyn_saved_regs;
    getRegisters(syscallreg_);

    // Remember the current PC.  When we continue the process at this PC
    // we will restart the system call.
    postsyscallpc_ = (Address) GETREG_PC(syscallreg_->theIntRegs);
    proc_->set_entry_syscalls(scsavedentry);
    proc_->set_exit_syscalls(scsavedexit);
    return true;
}

bool dyn_lwp::restartSyscall() {
    if (!stoppedInSyscall_)
        return true;
    
    // We interrupted a sleeping system call at some previous pause
    // (i.e. stoppedInSyscall is true), we have not restarted that
    // system call yet, and the current PC is the insn following
    // the interrupted call.  It is time to restart the system
    // call.
    
    // Note that when we make the process runnable, we ignore
    // `flags', set if `hasNewPC' was true in the previous block,
    // because we don't want its PC; we want the PC of the system
    // call trap, which was saved in `syscallreg'.
    if (!restoreRegisters(*syscallreg_)) return false;
    delete syscallreg_;
    syscallreg_ = NULL;
    
    // We are done -- the process is in the kernel for the system
    // call, with the right registers values.  Make the process
    // runnable, restoring its previously blocked signals.
    // The signals were blocked as part of the abort mechanism.
    stoppedInSyscall_ = false;
#if 0
    int bufsize = sizeof(long) + sizeof(proc_sigset_t);
    char buf[bufsize]; long *bufptr = (long *)buf;
    *bufptr = PCSHOLD; bufptr++;
    memcpy(bufptr, &sighold_, sizeof(proc_sigset_t));
    
    if (write(ctl_fd(), buf, bufsize) != bufsize) {
        perror("continueLWP: PCSHOLD"); return false;
    }
#endif
    return true;
}



dyn_lwp *process::createRepresentativeLWP() {
   // don't register the representativeLWP in the lwps since it's not a true
   // lwp
   representativeLWP = createFictionalLWP(0);
   return representativeLWP;
}

// Stop the LWP in question
bool dyn_lwp::stop_() {
  long command[2];
  command[0] = PCSTOP;

  if (write(ctl_fd(), command, sizeof(long)) != sizeof(long)) {
      fprintf(stderr, "%s[%d][%s]: ", FILE__, __LINE__, getThreadStr(getExecThreadID()));
      perror("pauseLWP: PCSTOP");
      return false;
  }

  return true;
}

// Get the active frame (PC, FP/SP)
Frame dyn_lwp::getActiveFrame()
{
   struct dyn_saved_regs regs;
   bool status = getRegisters(&regs);
   if(status == false)
      return Frame();  

   return Frame(GETREG_PC(regs.theIntRegs),
		GETREG_FP(regs.theIntRegs), 
		0, // SP unused
		proc_->getPid(), proc_, NULL, this, true);
}

// Get the registers of the stopped thread and return them.
bool dyn_lwp::getRegisters_(struct dyn_saved_regs *regs)
{
    lwpstatus_t status;
    if (!get_status(&status)) return false;

    // Process must be stopped for register data to be correct.

    assert((status.pr_flags & PR_STOPPED) || (status.pr_flags & PR_ISTOP)
           || (status.pr_flags & PR_PCINVAL)  // eg. a trap at syscall exit
           );

    memcpy(&(regs->theIntRegs), &(status.pr_reg), sizeof(prgregset_t));
    memcpy(&(regs->theFpRegs), &(status.pr_fpreg), sizeof(prfpregset_t));

    return true;
}

bool process::determineLWPs(pdvector<unsigned > &lwp_ids) {
   char procdir[128];
   snprintf(procdir, 128, "/proc/%d/lwp", getPid());
   DIR *dirhandle = opendir(procdir);
   if (dirhandle)
   {
      struct dirent *direntry;
      while((direntry = readdir(dirhandle)) != NULL) {
         unsigned lwp_id = atoi(direntry->d_name);
         if (!lwp_id) continue;
         lwp_ids.push_back(lwp_id);
      }
      closedir(dirhandle);
   }
   return true;
}

// Restore registers saved as above.
bool dyn_lwp::restoreRegisters_(const struct dyn_saved_regs &regs)
{
    lwpstatus_t status;
    get_status(&status);

    // The fact that this routine can be shared between solaris/sparc and
    // solaris/x86 is just really, really cool.  /proc rules!
    const int regbufsize = sizeof(long) + sizeof(prgregset_t);
    char regbuf[regbufsize]; long *regbufptr = (long *)regbuf;
    *regbufptr = PCSREG; regbufptr++;
    memcpy(regbufptr, &(regs.theIntRegs), sizeof(prgregset_t));
    int writesize;
try_again:
    errno = 0;
    writesize = write(ctl_fd(), regbuf, regbufsize);
    
    if (writesize != regbufsize) {
        if (errno == EBUSY) {
          fprintf(stderr, "%s[%d]:  busy fd\n", FILE__, __LINE__);
          struct timeval slp;
          slp.tv_sec = 0;
          slp.tv_usec = 1 /*ms*/ *1000;
          select(0, NULL, NULL, NULL, &slp);
          goto try_again;
        }
        perror("restoreRegisters: GPR write");
        return false;
    }
    const int fpbufsize = sizeof(long) + sizeof(prfpregset_t);
    char fpbuf[fpbufsize]; long *fpbufptr = (long *)fpbuf;
    *fpbufptr = PCSFPREG; fpbufptr++;
    memcpy(fpbufptr, &(regs.theFpRegs), sizeof(prfpregset_t));
    
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
      else {
          return true;
      }
  }
  
#if defined(AIX_PROC)
  if (status.pr_why == PR_SIGNALLED &&
      status.pr_what == SIGSTOP) {
      // We can't operate on a process in SIGSTOP, so clear it
      proc()->getRepresentativeLWP()->clearSignal();
  }
  
  // I've seen cases where we're apparently not in a system
  // call, but can't write registers... we'll label this case
  // a syscall for now
  const int regbufsize = sizeof(long) + sizeof(prgregset_t);
  char regbuf[regbufsize]; long *regbufptr = (long *)regbuf;
  *regbufptr = PCSREG; regbufptr++;
  memcpy(regbufptr, &(status.pr_reg), sizeof(prgregset_t));
  int writesize = write(ctl_fd(), regbuf, regbufsize);
  if (writesize == -1) {
      return true;
  }
#endif
  return false;
}


bool dyn_lwp::changePC(Address addr, struct dyn_saved_regs *regs)
{
    // Don't change the contents of regs if given
    dyn_saved_regs local;
    if (!regs) {
        getRegisters(&local);
    } else {
        memcpy(&local, regs, sizeof(struct dyn_saved_regs));
    }

    // Compatibility: we don't use PCSVADDR on Solaris because AIX doesn't 
    // support it
    // nPC MUST be set before PC! On platforms that don't have it,
    // nPC will be overwritten by the PC write
    GETREG_nPC(local.theIntRegs) = addr + instruction::size();
    GETREG_PC(local.theIntRegs) = addr;

    if (!restoreRegisters(local))
        return false;
    
    dyn_saved_regs check;
    getRegisters(&check);
    assert(GETREG_PC(local.theIntRegs) == GETREG_PC(check.theIntRegs));

    return true;
}

#if defined(i386_unknown_solaris2_5)
bool process::changeIntReg(int reg, Address val) {
   assert(status_ == stopped); // /proc will require this

   prgregset_t theIntRegs;
   dyn_lwp *replwp = getRepresentativeLWP();
   if (ioctl(replwp->get_fd(), PIOCGREG, &theIntRegs) == -1) {
      perror("dyn_lwp::changeIntReg");
      if (errno == EBUSY) {
         cerr << "It appears that the process wasn't stopped in the eyes of /proc" << endl;
         assert(false);
      }
      return false;
   }

   theIntRegs[reg] = val;

   if (ioctl(replwp->get_fd(), PIOCSREG, &theIntRegs) == -1) {
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
    if(!is_attached()) {
        return false;
    }
    
   if (lwp_id_) {
      // We're using an lwp file descriptor, so get directly
      if (pread(status_fd_, 
                (void *)status, 
                sizeof(lwpstatus_t), 0) != sizeof(lwpstatus_t)) {
          // When we fork a LWP file might disappear.
          if (errno != ENOENT) {
              perror("dyn_lwp::get_status");            
          }
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
    dyn_saved_regs regs;
    getRegisters(&regs);
    result = GETREG_GPR(regs.theIntRegs, reg);
    return result;
}

bool dyn_lwp::realLWP_attach_() {
   char temp[128];
   sprintf(temp, "/proc/%d/lwp/%d/lwpctl", (int)proc_->getPid(), lwp_id_);
   ctl_fd_ = P_open(temp, O_WRONLY, 0);
   if (ctl_fd_ < 0) perror("Opening lwpctl");
   sprintf(temp, "/proc/%d/lwp/%d/lwpstatus", (int)proc_->getPid(),lwp_id_);
   status_fd_ = P_open(temp, O_RDONLY, 0);    
   if (status_fd_ < 0) perror("Opening lwpstatus");

   return true;
}

bool dyn_lwp::representativeLWP_attach_() {
#if defined(os_aix)
    // Wait a sec; we often outrun process creation.
    sleep(2);
#endif
   /*
     Open the /proc file corresponding to process pid
   */

   char temp[128];
   // Open the process-wise handles
   sprintf(temp, "/proc/%d/ctl", getPid());
   ctl_fd_ = P_open(temp, O_WRONLY | O_EXCL, 0);    
   if (ctl_fd_ < 0) perror("Opening (LWP) ctl");
   
   sprintf(temp, "/proc/%d/status", getPid());
   status_fd_ = P_open(temp, O_RDONLY, 0);    
   if (status_fd_ < 0) perror("Opening (LWP) status");

   as_fd_ = INVALID_HANDLE_VALUE;
   sprintf(temp, "/proc/%d/as", getPid());
   as_fd_ = P_open(temp, O_RDWR, 0);
   if (as_fd_ < 0) perror("Opening as fd");

#if !defined(AIX_PROC)
   sprintf(temp, "/proc/%d/auxv", getPid());
   auxv_fd_ = P_open(temp, O_RDONLY, 0);
   if (auxv_fd_ < 0) perror("Opening auxv fd");
#else
   // AIX doesn't have the auxv file
   auxv_fd_ = INVALID_HANDLE_VALUE;
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

   lwpstatus_t status;

   // special for aix: we grab the status and clear the STOP
   // signal (if any)
   is_attached_ = true;
   get_status(&status);


   if (status.pr_why == PR_SIGNALLED &&
       status.pr_what == SIGSTOP) {
       clearSignal();
   }

  // Abort a system call if we're in it
  abortSyscall();

   return true;
}

// Close FDs opened above
void dyn_lwp::realLWP_detach_()
{
   assert(is_attached());  // dyn_lwp::detach() shouldn't call us otherwise
      
   // First we want to clear any signal which might be pending
   // for the process. Otherwise, when we detach the process has
   // a signal sent that it may have no clue about
   // But we can't do that here -- all FDs associated with the process
   // are closed. 
   if (ctl_fd_ != INVALID_HANDLE_VALUE)
      P_close(ctl_fd_);
   if (status_fd_ != INVALID_HANDLE_VALUE)
      P_close(status_fd_);
   is_attached_ = false;
}

// Close FDs opened above
void dyn_lwp::representativeLWP_detach_()
{
   assert(is_attached());  // dyn_lwp::detach() shouldn't call us otherwise
      
   // First we want to clear any signal which might be pending
   // for the process. Otherwise, when we detach the process has
   // a signal sent that it may have no clue about
   // But we can't do that here -- all FDs associated with the process
   // are closed.
   if (ctl_fd_ != INVALID_HANDLE_VALUE)
      P_close(ctl_fd_);
   if (status_fd_ != INVALID_HANDLE_VALUE)
      P_close(status_fd_);
   if (as_fd_ != INVALID_HANDLE_VALUE)
      P_close(as_fd_);
   if (ps_fd_ != INVALID_HANDLE_VALUE)
      P_close(ps_fd_);
   is_attached_ = false;
}

/*
 * Process-wide /proc operations
 */

bool process::setProcessFlags()
{
    long command[2];

    // Unset all flags
    command[0] = PCUNSET;
    command[1] = PR_BPTADJ | PR_MSACCT | PR_RLC | PR_KLC | PR_FORK;

    dyn_lwp *replwp = getRepresentativeLWP();
    if (write(replwp->ctl_fd(), command, 2*sizeof(long)) != 2*sizeof(long)) {
        perror("installProcessFlags: PRUNSET");
        return false;
    }
    command[0] = PCSET;

    long flags = PR_BPTADJ | PR_MSACCT | PR_FORK;

    command[1] = flags;

    if (write(replwp->ctl_fd(), command, 2*sizeof(long)) != 2*sizeof(long)) {
        perror("installProcessFlags: PCSET");
        return false;
    }

   // step 2) /proc PIOCSTRACE: define which signals should be
   // forwarded to daemon These are (1) SIGSTOP and (2) either
   // SIGTRAP (sparc) or SIGILL (x86), to implement inferiorRPC
   // completion detection.
   // Also detect SIGCONT so that we know if a user has restarted
   // an application.
  
   proc_sigset_t sigs;
    
   premptyset(&sigs);
   praddset(&sigs, SIGSTOP);
    
   praddset(&sigs, SIGTRAP);
    
   praddset(&sigs, SIGCONT);
   praddset(&sigs, SIGBUS);
   praddset(&sigs, SIGSEGV);
   praddset(&sigs, SIGILL);
    
   const int bufsize = sizeof(long) + sizeof(proc_sigset_t);
   char buf[bufsize]; 
   long *bufptr = (long *) buf;
   *bufptr = PCSTRACE;
   bufptr++;
   memcpy(bufptr, &sigs, sizeof(proc_sigset_t));

   if(write(replwp->ctl_fd(), buf, bufsize) != bufsize) {
      perror("attach: PCSTRACE");
      return false;
   }


    return true;
}

bool process::unsetProcessFlags()
{
    long command[2];

    if (!isAttached()) return false;
    // Unset all flags
    command[0] = PCUNSET;
    command[1] = PR_BPTADJ | PR_MSACCT | PR_RLC | PR_KLC | PR_FORK;

    dyn_lwp *replwp = getRepresentativeLWP();
    if (write(replwp->ctl_fd(), command, 2*sizeof(long)) != 2*sizeof(long)) {
        perror("unsetProcessFlags: PRUNSET");
        return false;
    }

    proc_sigset_t sigs;
    premptyset(&sigs);
    const int sigbufsize = sizeof(long) + sizeof(proc_sigset_t);
    char sigbuf[sigbufsize]; long *sigbufptr = (long *)sigbuf;

    sigbufptr = (long *)sigbuf;
    *sigbufptr = PCSTRACE;
    if (write(replwp->ctl_fd(), sigbuf, sigbufsize) != sigbufsize) {
        perror("unsetProcessFlags: PCSTRACE");
        return false;
    }
    
    return true;
}

// AIX requires us to re-open the process-wide handles after
// an exec call

#if defined(AIX_PROC)
void dyn_lwp::reopen_fds() {
        // Reopen the process-wide handles
    char temp[128];

    P_close(as_fd_);
    sprintf(temp, "/proc/%d/as", getPid());
    as_fd_ = P_open(temp, O_RDWR, 0);
    if (as_fd_ <= 0) perror("Opening as fd");
#if !defined(AIX_PROC)
    P_close(auxv_fd_);
    sprintf(temp, "/proc/%d/auxv", getPid());
    auxv_fd_ = P_open(temp, O_RDONLY, 0);
    if (auxv_fd_ <= 0) perror("Opening auxv fd");
#else
    // AIX doesn't have the auxv file
    auxv_fd_ = INVALID_HANDLE_VALUE;
#endif

    P_close(map_fd_);
    sprintf(temp, "/proc/%d/map", getPid());
    map_fd_ = P_open(temp, O_RDONLY, 0);
    if (map_fd_ <= 0) perror("map fd");

    P_close(ps_fd_);
    sprintf(temp, "/proc/%d/psinfo", getPid());
    ps_fd_ = P_open(temp, O_RDONLY, 0);
    if (ps_fd_ <= 0) perror("Opening ps fd");

    P_close(status_fd_);
    sprintf(temp, "/proc/%d/status", getPid());
    status_fd_ = P_open(temp, O_RDONLY, 0);
    if (status_fd_ <= 0) perror("Opening status fd");
}
#endif

bool process::isRunning_() const {
    // We can key off the representative LWP
    return getRepresentativeLWP()->isRunning();
}

/*
   terminate execution of a process
 */
terminateProcStatus_t process::terminateProc_()
{
  // Two kill methods: polite and unpolite. Not sure why we use
  // polite, but hey...

  // Polite: we're attached, and use a /proc command.
  // Unpolite: we're not attached, and kill -9

  if (isAttached()) {
    // these next two lines are a hack used to get the poll call initiated
    // by checkForAndHandleProcessEvents() in process::terminateProc to
    // still check process for events if it was previously stopped
    if(status() == stopped)
      status_ = running;

    long command[2];
    command[0] = PCKILL;
    command[1] = SIGKILL;
    dyn_lwp *cntl_lwp = getRepresentativeLWP();
    if (cntl_lwp) {
      if (write(cntl_lwp->ctl_fd(), 
		command, 2*sizeof(long)) != 2*sizeof(long)) {
	perror("terminateProc: PCKILL");
	// TODO: what gets returned if the process is already dead?
	// proc man page doesn't say.
	return terminateFailed;
      }
      return terminateSucceeded;
    }
    else {
      // Uh...
      return terminateFailed;
    }
  }
  else {
    // We may be detached. Go for it the old-fashioned way.
    if (kill( getPid(), SIGKILL )) {
	return terminateFailed;
    }
    else {
      // alreadyTerminated... since we don't want to wait
      // for a "I'm dead" message.
      return alreadyTerminated;
    }
  }
  assert(0 && "Unreachable");
  return terminateFailed;
}

bool dyn_lwp::waitUntilStopped() {
   return true;
}

bool process::waitUntilStopped() {
   return true;
}

bool dyn_lwp::writeTextWord(caddr_t inTraced, int data) {
   //  cerr << "writeTextWord @ " << (void *)inTraced << endl; cerr.flush();
   bool ret =  writeDataSpace(inTraced, sizeof(int), (caddr_t) &data);
   if (!ret) fprintf(stderr, "%s[%d][%s]:  writeDataSpace failed\n",
                     __FILE__, __LINE__, getThreadStr(getExecThreadID()));
        assert(ret);
   return ret;
}

bool dyn_lwp::writeTextSpace(void *inTraced, u_int amount, const void *inSelf)
{
   //  cerr << "writeTextSpace pid=" << getPid() << ", @ " << (void *)inTraced
   //       << " len=" << amount << endl; cerr.flush();
   bool ret =  writeDataSpace(inTraced, amount, inSelf);
   if (!ret) fprintf(stderr, "%s[%d][%s]:  writeDataSpace failed\n",
                     __FILE__, __LINE__, getThreadStr(getExecThreadID()));
        assert(ret);
   return ret;
}

bool dyn_lwp::readTextSpace(void *inTraced, u_int amount, const void *inSelf) {
   return readDataSpace(inTraced, amount, const_cast<void *>(inSelf));
}

bool dyn_lwp::writeDataSpace(void *inTraced, u_int amount, const void *inSelf)
{
   //fprintf(stderr, "%s[%d][%s]:  writeDataSpace: %p\n", __FILE__, __LINE__, getThreadStr(getExecThreadID()), inTraced);
   ptraceOps++; ptraceBytes += amount;

   //  cerr << "process::writeDataSpace_ pid " << getPid() << " writing "
   //       << amount << " bytes at loc " << inTraced << endl;
#if defined(BPATCH_LIBRARY)
#if defined (sparc_sun_solaris2_4)
   if(proc_->collectSaveWorldData &&  ((Address) inTraced) >
      proc_->getDyn()->getlowestSObaseaddr() ) {
       mapped_object *sh_obj = NULL;
       bool result = false;
       const pdvector<mapped_object *> &objs = proc_->mappedObjects();
       for (unsigned i = 0; i < objs.size(); i++) {
           sh_obj = objs[i];
           result = sh_obj->isinText((Address) inTraced);
           if( result  ){
               /*	bperr(" write at %lx in %s amount %x insn: %x \n", 
                        (off_t)inTraced, sh_obj->getName().c_str(), amount,
                        *(unsigned int*) inSelf);
                        */	
               sh_obj->setDirty();	
               break;
           }
       }
   }
#endif
#endif
   off64_t loc;
   // Problem: we may be getting a address with the high bit
   // set. So how to convince the system that it's not negative?
   loc = (off64_t) ((unsigned) inTraced);

   int written = pwrite64(as_fd(), inSelf, amount, loc);
   if(written != (int)amount) {
      fprintf(stderr, "%s[%d][%s]:  writeDataSpace: %s\n", __FILE__, __LINE__, getThreadStr(getExecThreadID()), strerror(errno));
      perror("writeDataSpace");
      assert(0);
      return false;
   }
    
   return true;
}

bool dyn_lwp::readDataSpace(const void *inTraced, u_int amount, void *inSelf) {
    ptraceOps++; ptraceBytes += amount;

    off64_t loc;
    loc = (off64_t) ((unsigned) inTraced);

    int res = pread64(as_fd(), inSelf, amount, loc);
    //fprintf(stderr, "%s[%d][%s]:  %d = readDataSpace(%p, amt=%d, %p)\n",
    //       FILE__, __LINE__, getThreadStr(getExecThreadID()), res,
    //       inTraced, amount, inSelf);
    if (res != (int) amount) {
        perror("readDataSpace");
        bperr( "From 0x%x (mutator) to 0x%x (mutatee), %d bytes, got %d\n",
                (int)inSelf, (int)inTraced, amount, res);
        return false;
    }
    return true;
}


bool process::get_status(pstatus_t *status) const
{
    if (!isAttached()) return false;
    if (!getRepresentativeLWP()->is_attached()) return false;
   
    int readfd = getRepresentativeLWP()->status_fd();
    size_t sz_read = pread(readfd, (void *)status, sizeof(pstatus_t), 0);
    if (sz_read != sizeof(pstatus_t)) {
        fprintf(stderr, "[%s][%d]: process::get_status: %s\n", FILE__, __LINE__, strerror(errno));
        fprintf(stderr, "[%s][%d]: pread returned %d instead of %d, fd = %d\n", FILE__, __LINE__, sz_read, sizeof(pstatus_t), readfd);
        perror("pread");
        return false;
    }
    
    return true;
}

bool process::set_entry_syscalls(sysset_t *entry)
{
    if (!isAttached()) return false;
    
    int bufentrysize = sizeof(long) + SYSSET_SIZE(entry);
    char *bufentry = new char[bufentrysize]; long *bufptr = (long *)bufentry;
    
    // Write entry syscalls
    *bufptr = PCSENTRY; bufptr++;
    memcpy(bufptr, entry, SYSSET_SIZE(entry));

    dyn_lwp *replwp = getRepresentativeLWP();

    if (write(replwp->ctl_fd(), bufentry, bufentrysize) != bufentrysize){
       delete [] bufentry;
       return false;
    }

    delete [] bufentry;
    return true;    
}

bool process::set_exit_syscalls(sysset_t *exit)
{
    if (!isAttached()) return false;
    
    int bufexitsize = sizeof(long) + SYSSET_SIZE(exit);
    char *bufexit = new char[bufexitsize]; long *bufptr = (long *)bufexit;
    *bufptr = PCSEXIT; bufptr++;
    memcpy(bufptr, exit, SYSSET_SIZE(exit));

    dyn_lwp *replwp = getRepresentativeLWP();        
    if (write(replwp->ctl_fd(), bufexit, bufexitsize) != bufexitsize){
       delete [] bufexit;
       return false;
    }
    delete [] bufexit;
    return true;    
}

syscallTrap *process::trapSyscallExitInternal(Address syscall)
{
    syscallTrap *trappedSyscall = NULL;

    // First, the cross-platform bit. If we're already trapping
    // on this syscall, then increment the reference counter
    // and return

    for (unsigned iter = 0; iter < syscallTraps_.size(); iter++) {
        if (syscallTraps_[iter]->syscall_id == syscall) {
            trappedSyscall = syscallTraps_[iter];
            break;
        }
    }

    if (trappedSyscall) {
        // That was easy...
        trappedSyscall->refcount++;
        //bperr( "Bumping refcount for syscall %d to %d\n",
        //        (int)trappedSyscall->syscall_id, trappedSyscall->refcount);
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

        //bperr( "  trapping syscall %d for 1st time (%d)\n",
        //       (int)trappedSyscall->syscall_id, trappedSyscall->refcount);
        
#if defined(AIX_PROC) 
        // AIX does some weird things, as we can't modify a thread
        // at a system call exit -- this means that using /proc
        // doesn't get us anywhere.
        // The "address" of the system call is actually the LWP
        // in the call
        dyn_lwp *syslwp = getLWP((unsigned) syscall);
        Frame frame = syslwp->getActiveFrame();
        Frame callerFrame = frame.getCallerFrame();
        
        Address origLR;
        readDataSpace((void *)(callerFrame.getFP() + 8),
                      sizeof(Address),
                      (void *)&origLR, false);
        trappedSyscall->origLR = origLR;
        
        Address trapAddr = inferiorMalloc(instruction::size());
        trappedSyscall->trapAddr = trapAddr;

        codeGen gen(1);
        instruction::generateTrap(gen);

        bool ret = writeDataSpace((void *)trapAddr, 
                       gen.used(),
                       gen.start_ptr());
        if (!ret) fprintf(stderr, "%s[%d][%s]:  writeDataSpace failed\n",
                          __FILE__, __LINE__, getThreadStr(getExecThreadID()));
        writeDataSpace((void *)(callerFrame.getFP() + 8), 
                       sizeof(Address),
                       (void *)&trapAddr);
        if (!ret) fprintf(stderr, "%s[%d][%s]:  writeDataSpace failed\n",
                          __FILE__, __LINE__, getThreadStr(getExecThreadID()));
        assert(ret);
#else
        sysset_t *cur_syscalls = SYSSET_ALLOC(getPid());

        if (!get_exit_syscalls(cur_syscalls)) return NULL;
        if (prissyssetmember(cur_syscalls, trappedSyscall->syscall_id))
            // Surprising case... but it's possible as we trap fork and exec
            trappedSyscall->orig_setting = 1;
        else
            trappedSyscall->orig_setting = 0;

        // 2) Place a trap
        praddsysset(cur_syscalls, trappedSyscall->syscall_id);
        if (!set_exit_syscalls(cur_syscalls)) return false;
        
        //bperr("PCSexit for %d, orig %d\n", 
        //trappedSyscall->syscall_id, trappedSyscall->orig_setting);
#endif
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
        //bperr( "Refcount on syscall %d reduced to %d\n",
        //        (int)trappedSyscall->syscall_id,
        //        trappedSyscall->refcount);
        return true;
    }
    // Erk... it hit 0. Remove the trap at the system call
#if defined(AIX_PROC) 
    dyn_lwp *lwp = getLWP(trappedSyscall->syscall_id);
    inferiorFree(trappedSyscall->trapAddr);
    lwp->changePC(trappedSyscall->origLR, NULL);
#else
    sysset_t *cur_syscalls = SYSSET_ALLOC(getPid());

    if (!get_exit_syscalls(cur_syscalls)) return false;
    if (!trappedSyscall->orig_setting) {
        prdelsysset(cur_syscalls, trappedSyscall->syscall_id);
    }
    
    if (!set_exit_syscalls(cur_syscalls)) return false;
    

#endif
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

Address dyn_lwp::getCurrentSyscall() {
    
    // Return the system call we're currently in
    lwpstatus_t status;
    if (!get_status(&status)) {
        bperr( "Failed to get status\n");
        return 0;
    }
#if defined(AIX_PROC)
    return get_lwp_id();
#else
    return status.pr_syscall;
#endif
}



bool dyn_lwp::stepPastSyscallTrap()
{
#if defined(AIX_PROC)
    // What is something else doing hitting our trap?
    assert(0);
#endif
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
#if defined(AIX_PROC) 
    if (!trappedSyscall_) return 0;
    
    Frame frame = getActiveFrame();
    
    if (frame.getPC() == trappedSyscall_->trapAddr)
        return 2;
#else
    Address syscall;
    // Get the status of the LWP
    lwpstatus_t status;
    if (!get_status(&status)) {
        bperr( "Failed to get thread status\n");
        return 0;
    }
    
    if (status.pr_why != PR_SYSEXIT) {
        return 0;
    }
    syscall = status.pr_what;
    
    if (trappedSyscall_ && syscall == trappedSyscall_->syscall_id) {
        return 2;
    }
    // Unfortunately we can't check a recorded system call trap,
    // since we don't have one saved. So do a scan through all traps the
    // process placed
    if (proc()->checkTrappedSyscallsInternal(syscall))
        return 1;
#endif

    // This is probably an error case... but I'm not sure.
    return 0;
}


int decodeProcStatus(process *,
                     lwpstatus_t status,
		     EventRecord &ev) {

   ev.info = GETREG_INFO(status.pr_reg);

   switch (status.pr_why) {
     case PR_SIGNALLED:
        ev.type = evtSignalled;
        ev.what = status.pr_what;
        break;
     case PR_SYSENTRY:
        ev.type = evtSyscallEntry;
        ev.what = status.pr_what;
        
#if defined(AIX_PROC)
        // We actually pull from the syscall argument vector
        if (status.pr_nsysarg > 0)
           ev.info = status.pr_sysarg[0];
        else
           ev.info = 0;
#endif
        break;
     case PR_SYSEXIT:
        ev.type = evtSyscallExit;
        ev.what = status.pr_what;
        
#if defined(AIX_PROC)
        // This from the proc header file: system returns are
        // left in pr_sysarg[0]. NOT IN MAN PAGE.
        ev.info = status.pr_sysarg[0];
#endif
        break;
     case PR_REQUESTED:
        // We don't expect PR_REQUESTED in the signal handler
        assert(0 && "PR_REQUESTED not handled");
#if defined(PR_SUSPENDED)
     case PR_SUSPENDED:
        // I'm seeing this state at times with a forking multi-threaded
        // child process, currently handling by just continuing the process
        ev.type = evtSuspended;
        break;
#endif
     case PR_JOBCONTROL:
     case PR_FAULTED:
     default:
        assert(0);
        break;
   }
   
   return 1;
}

int showProcStatus(lwpstatus_t status)
{
   switch (status.pr_why) {
     case PR_SIGNALLED:
        cerr << "PR_SIGNALED, what: " << (int)status.pr_what << endl;
        break;
     case PR_SYSENTRY:
        cerr << "PR_SYSENTRY, what: " << (int)status.pr_what << endl;
        break;
     case PR_SYSEXIT:
        cerr << "PR_SYSEXIT, what: " << (int)status.pr_what << endl;
        break;
     case PR_REQUESTED:
        cerr << "PR_REQUESTED\n";
        break;
#if defined(PR_SUSPENDED)
     case PR_SUSPENDED:
        // I'm seeing this state at times with a forking multi-threaded
        // child process, currently handling by just continuing the process
        cerr << "PR_SUSPENDED\n";
        break;
#endif
     case PR_JOBCONTROL:
     case PR_FAULTED:
     default:
        cerr << "OTHER\n";
        assert(0);
        break;
   }
   return 1;
}

bool find_matching_event(pdvector<EventRecord> &events,
                         process *proc, EventRecord &ev,
                         EventRecord &matching_ev) {
   bool found = false;
   for(unsigned i=0; i<events.size(); i++) {
      EventRecord *cur_event = &(events[i]);
      if(cur_event->proc == proc && cur_event->type == ev.type &&
         cur_event->what == ev.what) {
         // assume that there's at most one matching event
         assert(!found);
         matching_ev = *cur_event;
         found = true;
      }
   }

   return found;
}

//  decodeEvent() is here in lieu of a preexisting
//  function specialHandlingOfEvents(), which fixes up events in special
//  cases.  Not pretty

bool SignalGeneratorUnix::decodeEvent(EventRecord &cur_event) {

#if defined(os_aix)
  if (cur_event.type == evtSignalled && cur_event.what == SIGSTOP) {
    // On AIX we can't manipulate a process stopped on a
    // SIGSTOP... in any case, we clear it.
    // No other signal exhibits this behavior.
    cur_event.proc->getRepresentativeLWP()->clearSignal();
   }
#endif

#if defined(bug_aix_proc_broken_fork)
  if (cur_event.type == evtSignalled && cur_event.what == SIGSTOP) {
    // Possibly a fork stop in the RT library
    getSH()->decodeRTSignal(cur_event);
  }
#endif

  if (cur_event.type == evtSignalled && cur_event.what == SIGTRAP) 
    decodeSigTrap(cur_event);

  if ((cur_event.type == evtSignalled && cur_event.what == SIGSTOP) 
    || (cur_event.type == evtSignalled && cur_event.what == SIGINT)) 
    decodeSigStopNInt(cur_event);
  return true;
}

// returns true if updated events structure for this lwp 
bool SignalGeneratorUnix::updateEventsWithLwpStatus(process *curProc, dyn_lwp *lwp,
                               pdvector<EventRecord> &events)
{

  lwpstatus_t lwpstatus;
  bool res = lwp->get_status(&lwpstatus);

  if(res == false) {
    return false;
  }

  // This is the "why" for the lwps that were stopped because some other
  // lwp stopped for an interesting reason.  We don't care about lwps
  // that stopped for this reason.
  if(lwpstatus.pr_why == PR_REQUESTED) {
     return false;
  }
   
  EventRecord ev;
  ev.type = evtUndefined;
  ev.what = 0;
  ev.info = 0;
  ev.proc = curProc;
  ev.lwp = lwp;

  if(!decodeProcStatus(curProc, lwpstatus, ev))
     return false;
   
  EventRecord matching_event;
  bool found_match = find_matching_event(events, curProc, ev, matching_event);

  if(!found_match) {
     decodeEvent(ev);
     events.push_back(ev);
  } else {
     matching_event.lwp = lwp;
     decodeEvent(matching_event);
  }
   
  return true;
}

bool SignalGeneratorUnix::updateEvents(pdvector<EventRecord> &events, process *p, int lwp_to_use)
{
  //  returns true if events are updated
  dictionary_hash_iter<unsigned, dyn_lwp *> lwp_iter(p->real_lwps);
  dyn_lwp *cur_lwp;
  unsigned index;
  dyn_lwp *replwp = p->getRepresentativeLWP();
  int numreal_lwps = p->real_lwps.size();
  bool updated_events = false;

  if (numreal_lwps == 0)
     updated_events = updateEventsWithLwpStatus(p, replwp, events);
  else {
     while (lwp_iter.next(index, cur_lwp)) {
       if (cur_lwp->get_lwp_id() != (unsigned)lwp_to_use)
         continue;
       if (updateEventsWithLwpStatus(p, cur_lwp, events)) {
         updated_events = true;
         break;
       }
     }
  }
  return updated_events;
}

bool SignalGeneratorUnix::getFDsForPoll(pdvector<unsigned int> &fds)
{
  extern pdvector<process*> processVec;
  for(unsigned u = 0; u < processVec.size(); u++) {
     process *lproc = processVec[u];
     if(lproc && (lproc->status() == running))
       fds.push_back(lproc->getRepresentativeLWP()->status_fd());
  }
#ifdef DEBUG
  if (!fds.size()) {
    fprintf(stderr, "%s[%d]:  No valid proc fds available\n", __FILE__, __LINE__);
    for (unsigned int i = 0; i < processVec.size(); ++i) {
       process *p = processVec[i];
      fprintf(stderr, "\tprocess %d in state %s\n", processVec[i]->getPid(), 
              processVec[i]->getStatusAsString().c_str());
    }
  }
#endif
  return (fds.size() > 0);
}

pdstring process::tryToFindExecutable(const pdstring &iprogpath, int pid) {
  // This is called by exec, so we might have a valid file path. If so,
  // use it... otherwise go to /proc. Helps with exec aliasing problems.

   if (iprogpath.c_str()) {
       int filedes = open(iprogpath.c_str(), O_RDONLY);
       if (filedes != -1) {
          P_close(filedes);
          return iprogpath;
       }
   }

  // We need to dereference the /proc link.
  // Case 1: multiple copies of the same file opened with multiple
  // pids will not match (and should)
  // Case 2: an exec'ed program will have the same /proc path,
  // but different program paths
  pdstring procpath = pdstring("/proc/") + pdstring(pid) + pdstring("/object/a.out");

  // Sure would be nice if we could get the original....

  return procpath;
}




