/*
 * Copyright (c) 1996-2011 Barton P. Miller
 * 
 * We provide the Paradyn Parallel Performance Tools (below
 * described as "Paradyn") on an AS IS basis, and do not warrant its
 * validity or performance.  We reserve the right to update, modify,
 * or discontinue this software at any time.  We shall have no
 * obligation to supply such updates or modifications or any other
 * form of support to you.
 * 
 * By your use of Paradyn, you understand and agree that we (or any
 * other person or entity with proprietary rights in Paradyn) are
 * under no obligation to provide either maintenance services,
 * update services, notices of latent defects, or correction of
 * defects for Paradyn.
 * 
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

// Solaris-style /proc support

// $Id: sol_proc.C,v 1.125 2008/09/03 06:08:45 jaw Exp $

#if defined(os_aix)
#include <sys/procfs.h>
#else
#include <procfs.h>
#endif
#include <string>
#include <limits.h>
#include <poll.h>
#include <sys/types.h>  // for reading lwps out of proc
#include <dirent.h>     // for reading lwps out of proc
#include "common/h/headers.h"
#include "dyninstAPI/src/signalhandler.h"
#include "dyninstAPI/src/signalgenerator.h"
#include "dyninstAPI/src/process.h"
#include "dyninstAPI/src/dyn_lwp.h"
#include "dyninstAPI/src/dyn_thread.h"
#include "common/h/stats.h"
#include "common/h/pathName.h" // for path name manipulation routines
#include "dyninstAPI/src/sol_proc.h"
#include "dyninstAPI/src/os.h"
#include "dyninstAPI_RT/h/dyninstAPI_RT.h" // for DYNINST_BREAKPOINT_SIGNUM

#include "function.h"
#include "mapped_object.h"
#include "mapped_module.h"
#include "dynamiclinking.h" //getlowestSO...

   void safeClose(handleT &fd) {
      if (fd != INVALID_HANDLE_VALUE)
         P_close(fd);
      fd = INVALID_HANDLE_VALUE;
   }

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

void OS::osTraceMe(void) 
{
   sysset_t *exitSet = SYSSET_ALLOC(getpid());
   int bufsize = SYSSET_SIZE(exitSet) + sizeof(int);

   char *buf = new char[bufsize];
   int *bufptr = (int *)buf;
   char procName[128];

   // Get the current set of syscalls
   pstatus_t status;
   sprintf(procName,"/proc/%d/status", (int) getpid());
   int stat_fd = P_open(procName, O_RDONLY, 0);
   if (pread(stat_fd, (void *)&status, sizeof(pstatus_t), 0) !=
         sizeof(pstatus_t)) {
      fprintf(stderr, "%s[%d]:  pread: %s\n", FILE__, __LINE__, strerror(errno));
      perror("osTraceMe::pread");
      SYSSET_FREE(exitSet);
      delete [] buf;
      return;
   }
#if defined(os_aix) 
    if (status.pr_sysexit_offset) {
        if ((ssize_t)SYSSET_SIZE(exitSet) != pread(stat_fd, exitSet, SYSSET_SIZE(exitSet), status.pr_sysexit_offset))
       fprintf(stderr, "%s[%d]:  pread: %s\n", FILE__, __LINE__, strerror(errno));
    }
    else // No syscalls are being traced 
        premptysysset(exitSet);
#else
   memcpy(exitSet, &(status.pr_sysexit), SYSSET_SIZE(exitSet));
#endif
   safeClose(stat_fd);

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

bool dyn_lwp::isRunning() const 
{
   procProcStatus_t theStatus;

   if (!get_status(&theStatus)) return false;

   // Don't consider PR_ASLEEP to be stopped
   uint32_t stopped_flags = PR_STOPPED | PR_ISTOP;

   if (theStatus.pr_flags & stopped_flags)
      return false;
   else
      return true;
}

bool dyn_lwp::clearSignal() 
{
   lwpstatus_t status;
   get_status(&status);

   if (status.pr_why == PR_SIGNALLED) {
      int command[2];
      command[0] = PCRUN; command[1] = PRSTOP | PRCSIG;
      if (write(ctl_fd(), command, 2*sizeof(int)) != 2*sizeof(int)) {
         perror("clearSignal: PCRUN");
         return false;
      }
      command[0] = PCWSTOP;        
      if (write(ctl_fd(), command, sizeof(int)) != sizeof(int)) {
         perror("clearSignal: PCWSTOP");
         return false;
      }
      return true;
   }
   else if (status.pr_why == PR_JOBCONTROL) {
      // Someone else stopped this guy... we can't use PCRUN, we need to 
      // use signals. Did anyone else think of linux?

      // Non-blocking stop...
      int command[2];
      command[0] = PCDSTOP;
      if (write(ctl_fd(), command, sizeof(int)) != sizeof(int)) {
         perror("clearSignal: PCWSTOP");
         return false;
      }

      // SIGCONTINUE...
      kill(proc()->getPid(), SIGCONT);

      command[0] = PCWSTOP;        
      if (write(ctl_fd(), command, sizeof(int)) != sizeof(int)) {
         perror("clearSignal: PCWSTOP");
         return false;
      }
      return true;
   }
   // Uhh...
   assert(0);
   return false;
}

// Get the process running again. May do one or more of the following:
// 1) Continue twice to clear a signal
// 2) Restart an aborted system call
bool dyn_lwp::continueLWP_(int signalToContinueWith) 
{
   procProcStatus_t status;
   int command[2];
   Address pc;  // PC at which we are trying to continue
   // Check the current status
   if (!get_status(&status)) {
      bperr( "Failed to get LWP status\n");
      return false;
   }

   if ((0== (status.pr_flags & PR_STOPPED)) && 
         (0== (status.pr_flags & PR_ISTOP))) {
      // Already running, so catch our state up.
      return true;
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
   if (signalToContinueWith == NoSignal)
      command[1] = PRCSIG;  // clear the signal
   else {
      command[1] = 0;
   }

  // Continue the process the easy way
 busy_retry:
  errno = 0;
  signal_printf("[%s:%u] - LOW LEVEL continue happened\n", FILE__, __LINE__);
	       
  if (write(ctl_fd(), command, 2*sizeof(int)) != 2*sizeof(int)) {
      if (errno == EBUSY) {
         struct timeval slp;
         slp.tv_sec = 0;
         slp.tv_usec = 1 /*ms*/ *1000;
         select(0, NULL, NULL, NULL, &slp);
         fprintf(stderr, "%s[%d]: continueLWP_, ctl_fd is busy, trying again\n", FILE__, __LINE__);
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
   int bufsize = sizeof(int) + sizeof(proc_sigset_t);
   char buf[bufsize]; int *bufptr = (int *)buf;
   *bufptr = PCSHOLD; bufptr++;
   memcpy(bufptr, &sigs, sizeof(proc_sigset_t));

   if (write(ctl_fd(), buf, bufsize) != bufsize) {
      perror("abortSyscall: PCSHOLD"); return false;
   }
#endif
   int command[2];

   command[0] = PCRUN;
   command[1] = PRSABORT;

   if (write(ctl_fd(), command, 2*sizeof(int)) != 2*sizeof(int)) {
      perror("abortSyscall: PCRUN"); return false;
   }

   command[0] = PCWSTOP;
   if (write(ctl_fd(), command, sizeof(int)) != sizeof(int)) {
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

bool dyn_lwp::restartSyscall() 
{
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
   int bufsize = sizeof(int) + sizeof(proc_sigset_t);
   char buf[bufsize]; int *bufptr = (int *)buf;
   *bufptr = PCSHOLD; bufptr++;
   memcpy(bufptr, &sighold_, sizeof(proc_sigset_t));

   if (write(ctl_fd(), buf, bufsize) != bufsize) {
      perror("continueLWP: PCSHOLD"); return false;
   }
#endif
   return true;
}



dyn_lwp *process::createRepresentativeLWP() 
{
   // don't register the representativeLWP in the lwps since it's not a true
   // lwp
   representativeLWP = createFictionalLWP(0);
   return representativeLWP;
}

// Stop the LWP in question
bool dyn_lwp::stop_() 
{
   int command[2];
   command[0] = PCSTOP;

   signal_printf("%s[%d]: writing stop command\n", FILE__, __LINE__);

   if (write(ctl_fd(), command, sizeof(int)) != sizeof(int)) {
      fprintf(stderr, "%s[%d][%s]: ", FILE__, __LINE__, getThreadStr(getExecThreadID()));
      perror("pauseLWP: PCSTOP");
      return false;
   }

   return true;
}

// Get the registers of the stopped thread and return them.
bool dyn_lwp::getRegisters_(struct dyn_saved_regs *regs, bool /*includeFP*/)
{
   lwpstatus_t stat;

   if (status() == running) {
      fprintf(stderr, "ERROR: status %s (proc state %s)\n",
            processStateAsString(status()), processStateAsString(proc()->status()));
   }

   assert(status() != running);

   assert(!isRunning());

   if (!get_status(&stat)) return false;

   // Process must be stopped for register data to be correct.

   assert((stat.pr_flags & PR_STOPPED) || (stat.pr_flags & PR_ISTOP)
         || (stat.pr_flags & PR_PCINVAL)  // eg. a trap at syscall exit
         );

   memcpy(&(regs->theIntRegs), &(stat.pr_reg), sizeof(prgregset_t));
   memcpy(&(regs->theFpRegs), &(stat.pr_fpreg), sizeof(prfpregset_t));

   return true;
}

void dyn_lwp::dumpRegisters()
{
   dyn_saved_regs regs;
   if (!getRegisters(&regs)) {
      fprintf(stderr, "%s[%d]:  registers unavailable\n", FILE__, __LINE__);
      return;
   }

   fprintf(stderr, "PC:   %lx\n", (unsigned long) GETREG_PC(regs.theIntRegs));
   fprintf(stderr, "FP:   %lx\n", (unsigned long) GETREG_FP(regs.theIntRegs));
   fprintf(stderr, "INFO: %lx\n", (unsigned long) GETREG_INFO(regs.theIntRegs));
   //  plenty more register if we want to print em....
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
bool dyn_lwp::restoreRegisters_(const struct dyn_saved_regs &regs, bool /*includeFP*/)
{
   assert(status() != running);

   lwpstatus_t stat;
   get_status(&stat);

   assert(!isRunning());

   // The fact that this routine can be shared between solaris/sparc and
   // solaris/x86 is just really, really cool.  /proc rules!
   const int regbufsize = sizeof(int) + sizeof(prgregset_t);
   char regbuf[regbufsize]; int *regbufptr = (int *)regbuf;
   *regbufptr = PCSREG; regbufptr++;
   memcpy(regbufptr, &(regs.theIntRegs), sizeof(prgregset_t));
   int writesize;
try_again:
   int timeout = 2 * 1000; /*ms*/
   int elapsed = 0;
   errno = 0;
   writesize = write(ctl_fd(), regbuf, regbufsize);

   if (writesize != regbufsize) {
      if (errno == EBUSY) {
         //fprintf(stderr, "%s[%d]:  busy fd\n", FILE__, __LINE__);

         struct timeval slp;
         slp.tv_sec = 0;
         slp.tv_usec = 1 /*ms*/ *1000;
         select(0, NULL, NULL, NULL, &slp);
		 elapsed += 1;
		 if (elapsed > timeout)
		 {
			 fprintf(stderr, "%s[%d]:  failed to access process ctl fd\n", FILE__, __LINE__);
			 return false;
		 }
         goto try_again;
      }
      //If this fails, we may be attaching to a mutatee in a system call
      perror("restoreRegisters: GPR write");
      return false;
   }
   const int fpbufsize = sizeof(int) + sizeof(prfpregset_t);
   char fpbuf[fpbufsize]; int *fpbufptr = (int *)fpbuf;
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
   if (!get_status(&status)) {
      fprintf(stderr, "ERROR: failed to get LWP status!\n");
      return false;
   }

   // Old-style

   /*
      fprintf(stderr, "LWP syscall value: %d, pr_why is %d, PC at 0x%lx\n",
      status.pr_syscall, 
      status.pr_why,
      GETREG_PC(status.pr_reg));
    */
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

#if defined(os_aix)
   if (status.pr_why == PR_SIGNALLED &&
         status.pr_what == SIGSTOP) {
      // We can't operate on a process in SIGSTOP, so clear it
      proc()->getRepresentativeLWP()->clearSignal();
   }

   // I've seen cases where we're apparently not in a system
   // call, but can't write registers... we'll label this case
   // a syscall for now
   const int regbufsize = sizeof(int) + sizeof(prgregset_t);
   char regbuf[regbufsize]; int *regbufptr = (int *)regbuf;
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
bool process::changeIntReg(int reg, Address val) 
{
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
bool dyn_lwp::get_status(lwpstatus_t *stat) const
{
   if(!is_attached()) {
      return false;
   }

   if (lwp_id_) {
      // We're using an lwp file descriptor, so get directly
      if (pread(status_fd_, 
               (void *)stat, 
               sizeof(lwpstatus_t), 0) != sizeof(lwpstatus_t)) {
         // When we fork a LWP file might disappear.
         fprintf(stderr, "%s[%d]:  pread: %s\n", FILE__, __LINE__, strerror(errno));
         if ((errno != ENOENT) && (errno != EBADF)) {
            fprintf(stderr, "%s[%d]:  dyn_lwp::get_status: %s\n", FILE__, __LINE__, strerror(errno));
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
      memcpy(stat, &(procstatus.pr_lwp), sizeof(lwpstatus_t));
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

bool dyn_lwp::realLWP_attach_() 
{
   char temp[128];
   sprintf(temp, "/proc/%d/lwp/%d/lwpctl", (int)proc_->getPid(), lwp_id_);
   ctl_fd_ = P_open(temp, O_WRONLY, 0);
   if (ctl_fd_ < 0) {
      //fprintf(stderr, "%s[%d]: Opening lwpctl: %s\n", FILE__, __LINE__, strerror(errno));
      return false;
   }

   sprintf(temp, "/proc/%d/lwp/%d/lwpstatus", (int)proc_->getPid(),lwp_id_);
   status_fd_ = P_open(temp, O_RDONLY, 0);    
   if (status_fd_ < 0) {
      fprintf(stderr, "%s[%d]: Opening lwpstatus: %s\n", FILE__, __LINE__, strerror(errno));
      safeClose(ctl_fd_);
      return false;
   }

   return true;
}

bool dyn_lwp::representativeLWP_attach_() 
{
#if defined(os_aix) 
   //usleep(500 * 1000);
   //sleep(3);
   sleep(2);
   //sleep(1);
#endif
   /*
      Open the /proc file corresponding to process pid
    */

   char temp[128];
   // Open the process-wise handles
   sprintf(temp, "/proc/%d/ctl", getPid());

   if (!waitForFileToExist(temp, 10 /*seconds */)) {
      //fprintf(stderr, "%s[%d]:  cannot attach because %s does not exist\n", FILE__, __LINE__, temp);
      return false;
   }
   //ctl_fd_ = openFileWhenNotBusy(temp, O_WRONLY | O_EXCL, 0, 5/*seconds*/);
   ctl_fd_ = P_open(temp, O_WRONLY | O_EXCL, 0);    
   if (ctl_fd_ < 0) {
   //	fprintf(stderr, "%s[%d]: Opening (LWP) ctl: %s", FILE__, __LINE__, strerror(errno));
   }

   sprintf(temp, "/proc/%d/status", getPid());
   if (!waitForFileToExist(temp, 5 /*seconds */)) {
      fprintf(stderr, "%s[%d]:  cannot attach because %s does not exist\n", FILE__, __LINE__, temp);
      return false;
   }
   //status_fd_ = openFileWhenNotBusy(temp, O_RDONLY, 0, 5/*seconds*/);
   status_fd_ = P_open(temp, O_RDONLY, 0);    
   if (status_fd_ < 0) {
   	//fprintf(stderr, "%s[%d]: Opening (LWP) status: %s", FILE__, __LINE__, strerror(errno));
   } 

   as_fd_ = INVALID_HANDLE_VALUE;
   sprintf(temp, "/proc/%d/as", getPid());
   if (!waitForFileToExist(temp, 5 /*seconds */)) {
      fprintf(stderr, "%s[%d]:  cannot attach because %s does not exist\n", FILE__, __LINE__, temp);
      return false;
   }
   //as_fd_ = openFileWhenNotBusy(temp, O_RDWR, 0, 5/*seconds*/);
   as_fd_ = P_open(temp, O_RDWR, 0);
   if (as_fd_ < 0) {
   	//fprintf(stderr, "%s[%d]: Opening as fd: %s", FILE__, __LINE__, strerror(errno));
   }

#if !defined(os_aix)
   sprintf(temp, "/proc/%d/auxv", getPid());
   if (!waitForFileToExist(temp, 5 /*seconds */)) {
      fprintf(stderr, "%s[%d]:  cannot attach because %s does not exist\n", FILE__, __LINE__, temp);
      return false;
   }
   //auxv_fd_ = openFileWhenNotBusy(temp, O_RDONLY, 0, 5/*seconds*/);
   auxv_fd_ = P_open(temp, O_RDONLY, 0);
   if (auxv_fd_ < 0) {
   //	fprintf(stderr, "%s[%d]: Opening auxv fd: %s", FILE__, __LINE__, strerror(errno));
   }
#else
   // AIX doesn't have the auxv file
   auxv_fd_ = INVALID_HANDLE_VALUE;
#endif

   sprintf(temp, "/proc/%d/map", getPid());
   if (!waitForFileToExist(temp, 5 /*seconds */)) {
      //fprintf(stderr, "%s[%d]:  cannot attach because %s does not exist\n", FILE__, __LINE__, temp);
      return false;
   }
   //map_fd_ = openFileWhenNotBusy(temp, O_RDONLY, 0, 5/*seconds*/);
   map_fd_ = P_open(temp, O_RDONLY, 0);
   if (map_fd_ < 0) {
   	//fprintf(stderr, "%s[%d]:  map_fd: %s\n", FILE__, __LINE__, strerror(errno));
  }

   sprintf(temp, "/proc/%d/psinfo", getPid());
   if (!waitForFileToExist(temp, 5 /*seconds */)) {
      //fprintf(stderr, "%s[%d]:  cannot attach because %s does not exist\n", FILE__, __LINE__, temp);
      return false;
   }
   //ps_fd_ = openFileWhenNotBusy(temp, O_RDONLY, 0, 5/*seconds*/);
   ps_fd_ = P_open(temp, O_RDONLY, 0);
   if (ps_fd_ < 0) {
   	//fprintf(stderr, "%s[%d]: Opening ps fd: %s", FILE__, __LINE__, strerror(errno));
   }
   is_attached_ = true;

   if (isRunning()) {
      // Stop the process; we want it paused post-attach
      stop_();
   }

   // special for aix: we grab the status and clear the STOP
   // signal (if any)
   lwpstatus_t stat;
   get_status(&stat);

   if (((stat.pr_why == PR_SIGNALLED) ||
            (stat.pr_why == PR_JOBCONTROL)) &&
         (stat.pr_what == SIGSTOP)) {
      clearSignal();
   }

   //  if we attached to a running process, it might be stuck in a syscall,
   //  try to abort it
   if (proc()->wasCreatedViaAttach()) {
      // Abort a system call if we're in it
      abortSyscall();
   }

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
   safeClose(ctl_fd_);
   safeClose(status_fd_);
   safeClose(as_fd_);
   safeClose(auxv_fd_);
   safeClose(map_fd_);
   safeClose(ps_fd_);

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
   safeClose(ctl_fd_);
   safeClose(status_fd_);
   safeClose(as_fd_);
   safeClose(auxv_fd_);
   safeClose(map_fd_);
   safeClose(ps_fd_);

   is_attached_ = false;
}

/*
 * Process-wide /proc operations
 */

bool process::setProcessFlags()
{
   int command[2];

   // Unset all flags
   command[0] = PCUNSET;
   command[1] = PR_BPTADJ | PR_MSACCT | PR_RLC | PR_KLC | PR_FORK;

   dyn_lwp *replwp = getRepresentativeLWP();
   if (write(replwp->ctl_fd(), command, 2*sizeof(int)) != 2*sizeof(int)) {
//      perror("installProcessFlags: PRUNSET");
      return false;
   }
   command[0] = PCSET;
   command[1] = PR_BPTADJ | PR_MSACCT | PR_FORK;

   if (write(replwp->ctl_fd(), command, 2*sizeof(int)) != 2*sizeof(int)) {
//      perror("installProcessFlags: PCSET");
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
   praddset(&sigs, DYNINST_BREAKPOINT_SIGNUM);

   praddset(&sigs, SIGTRAP);

   praddset(&sigs, SIGCONT);
   praddset(&sigs, SIGBUS);
   praddset(&sigs, SIGSEGV);
   praddset(&sigs, SIGILL);

   const int bufsize = sizeof(int) + sizeof(proc_sigset_t);
   char buf[bufsize];
   int *bufptr = (int *) buf;
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
   int command[2];

   if (!isAttached()) return false;
   // Unset all flags
   command[0] = PCUNSET;
   command[1] = PR_BPTADJ | PR_MSACCT | PR_RLC | PR_KLC | PR_FORK;

   dyn_lwp *replwp = getRepresentativeLWP();
   if (write(replwp->ctl_fd(), command, 2*sizeof(int)) != 2*sizeof(int)) {
      perror("unsetProcessFlags: PRUNSET");
      return false;
   }

   proc_sigset_t sigs;
   premptyset(&sigs);
   const int sigbufsize = sizeof(int) + sizeof(proc_sigset_t);
   char sigbuf[sigbufsize]; int *sigbufptr = (int *)sigbuf;

   sigbufptr = (int *)sigbuf;
   *sigbufptr = PCSTRACE;
   if (write(replwp->ctl_fd(), sigbuf, sigbufsize) != sigbufsize) {
      perror("unsetProcessFlags: PCSTRACE");
      return false;
   }

   return true;
}

// AIX requires us to re-open the process-wide handles after
// an exec call

#if defined(os_aix)
void dyn_lwp::reopen_fds() {
   // Reopen the process-wide handles
   char temp[128];

   P_close(as_fd_);
   sprintf(temp, "/proc/%d/as", getPid());
   as_fd_ = P_open(temp, O_RDWR, 0);
   if (as_fd_ <= 0) perror("Opening as fd");
#if !defined(os_aix)
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

bool process::isRunning_() const 
{
   if (getRepresentativeLWP() &&
         isAttached())
      return getRepresentativeLWP()->isRunning();

   // Let's assume nothing is open....
   char temp[128];
   int status_fd;

   sprintf(temp, "/proc/%d/status", getPid());
   status_fd = P_open(temp, O_RDONLY, 0);
   if (status_fd <= 0) return false;

   pstatus_t procstatus;
   size_t sz_read = pread(status_fd, (void *)&procstatus, sizeof(pstatus_t), 0);
   close(status_fd);

   if (sz_read != sizeof(pstatus_t)) {
      fprintf(stderr, "%s[%d]:  pread: %s\n", FILE__, __LINE__, strerror(errno));
      return false;
   }

   uint32_t stopped_flags = PR_STOPPED | PR_ISTOP;

   if (procstatus.pr_flags & stopped_flags) {
      return false;
   }
   return true;
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

      int command[2];
      command[0] = PCKILL;
      command[1] = SIGKILL;
      dyn_lwp *cntl_lwp = getRepresentativeLWP();
      if (cntl_lwp) {
         if (write(cntl_lwp->ctl_fd(), 
                  command, 2*sizeof(int)) != 2*sizeof(int)) {
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

bool dyn_lwp::waitUntilStopped() 
{
   signal_printf("%s[%d]: waiting until stopped, process status %s\n", FILE__, __LINE__, proc()->getStatusAsString().c_str());
   pdvector<eventType> evts;
   eventType evt;

   // This ensures that we don't start running accidentally...
   processRunState_t oldState = proc()->sh->overrideSyncContinueState(stopRequest);

   while (proc()->status() != stopped) {
      if (proc()->status() == exited) break;
      evts.push_back(evtAnyEvent);
      signal_printf("%s[%d]: waiting for event in waitUntilStopped...\n", FILE__, __LINE__);
      evt = proc()->sh->waitForOneOf(evts);
      signal_printf("%s[%d]: got event in waitUntilStopped, process status %s\n", FILE__, __LINE__, proc()->getStatusAsString().c_str());
   }

   signal_printf("%s[%d]: stopped...\n", FILE__, __LINE__);
   // And now put things back the way they were.
   proc()->sh->overrideSyncContinueState(oldState);

   return true;
}

// I'm not sure this version is ever called...
bool process::waitUntilStopped() 
{
   signal_printf("%s[%d]: waiting until stopped, process status %s\n", FILE__, __LINE__, getStatusAsString().c_str());
   pdvector<eventType> evts;
   eventType evt;

   while (status() != stopped) {
      if (status() == exited) break;
      evts.push_back(evtAnyEvent);
      signal_printf("%s[%d]: waiting for event in waitUntilStopped...\n", FILE__, __LINE__);
      evt = sh->waitForOneOf(evts);
      signal_printf("%s[%d]: got event in waitUntilStopped, process status %s\n", FILE__, __LINE__, getStatusAsString().c_str());
   }

   signal_printf("%s[%d]: stopped...\n", FILE__, __LINE__);

   return true;
}

bool dyn_lwp::writeTextWord(caddr_t inTraced, int data) 
{
   //  cerr << "writeTextWord @ " << (void *)inTraced << endl; cerr.flush();
   bool ret =  writeDataSpace(inTraced, sizeof(int), (caddr_t) &data);
   if (!ret) fprintf(stderr, "%s[%d][%s]:  writeDataSpace failed\n",
         FILE__, __LINE__, getThreadStr(getExecThreadID()));
   assert(ret);
   return ret;
}

bool dyn_lwp::writeTextSpace(void *inTraced, u_int amount, const void *inSelf)
{
   //  cerr << "writeTextSpace pid=" << getPid() << ", @ " << (void *)inTraced
   //       << " len=" << amount << endl; cerr.flush();
   bool ret =  writeDataSpace(inTraced, amount, inSelf);
   if (!ret) fprintf(stderr, "%s[%d][%s]:  writeDataSpace failed\n",
         FILE__, __LINE__, getThreadStr(getExecThreadID()));
   assert(ret);
   return ret;
}

bool dyn_lwp::readTextSpace(const void *inTraced, u_int amount, void *inSelf) 
{
   return readDataSpace(inTraced, amount, inSelf);
}

bool dyn_lwp::writeDataSpace(void *inTraced, u_int amount, const void *inSelf)
{
   //fprintf(stderr, "%s[%d][%s]:  writeDataSpace: %p\n", FILE__, __LINE__, getThreadStr(getExecThreadID()), inTraced);
   ptraceOps++; ptraceBytes += amount;

   //  cerr << "process::writeDataSpace_ pid " << getPid() << " writing "
   //       << amount << " bytes at loc " << inTraced << endl;
   off64_t loc;
   // Problem: we may be getting a address with the high bit
   // set. So how to convince the system that it's not negative?
   loc = (off64_t) ((unsigned long) inTraced);

   errno = 0;

   int written = pwrite64(as_fd(), inSelf, amount, loc);
   if (written != (int)amount) {
      fprintf(stderr, "%s[%d][%s]:  writeDataSpace: %s\n", FILE__, __LINE__, getThreadStr(getExecThreadID()), strerror(errno));
      perror("writeDataSpace");
      //assert(0);
      return false;
   }

   return true;
}

bool dyn_lwp::readDataSpace(const void *inTraced, u_int amount, void *inSelf) 
{
   ptraceOps++; ptraceBytes += amount;

   off64_t loc;
   loc = (off64_t)((unsigned long)inTraced);

   int res = pread64(as_fd(), inSelf, amount, loc);
   //fprintf(stderr, "%s[%d][%s]:  %d = readDataSpace(%p, amt=%d, %p)\n",
   //       FILE__, __LINE__, getThreadStr(getExecThreadID()), res,
   //       inTraced, amount, inSelf);
   if (res != (int) amount) {
      fprintf(stderr, "%s[%d]:  pread: %s\n", FILE__, __LINE__, strerror(errno));
      /*
      // Commented out; the top-level call will print an error if desired.
      perror("readDataSpace");
      bperr( "From 0x%x (mutator) to 0x%x (mutatee), %d bytes, got %d\n",
      (int)inSelf, (int)inTraced, amount, res);
       */
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
/*
      fprintf(stderr, "[%s][%d]: process::get_status: %s\n", FILE__, __LINE__, strerror(errno));
      fprintf(stderr, "[%s][%d]: pread returned %d instead of %d, fd = %d\n", FILE__, __LINE__, sz_read, sizeof(pstatus_t), readfd);
      perror("pread");
*/      
      return false;
   }

   return true;
}

bool process::set_entry_syscalls(sysset_t *entry)
{
   if (!isAttached()) return false;

   int bufentrysize = sizeof(int) + SYSSET_SIZE(entry);
   char *bufentry = new char[bufentrysize]; int *bufptr = (int *)bufentry;

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

   int bufexitsize = sizeof(int) + SYSSET_SIZE(exit);
   char *bufexit = new char[bufexitsize]; int *bufptr = (int *)bufexit;
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

#if defined(os_aix) 
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

      codeGen gen(instruction::size());
      insnCodeGen::generateTrap(gen);

      bool ret = writeDataSpace((void *)trapAddr, 
            gen.used(),
            gen.start_ptr());
      if (!ret) fprintf(stderr, "%s[%d][%s]:  writeDataSpace failed\n",
            FILE__, __LINE__, getThreadStr(getExecThreadID()));
      writeDataSpace((void *)(callerFrame.getFP() + 8), 
            sizeof(Address),
            (void *)&trapAddr);

      signal_printf("%s[%d]: LWP placing syscall trap at addr 0x%lx\n",
            FILE__, __LINE__, trapAddr);

      if (!ret) fprintf(stderr, "%s[%d][%s]:  writeDataSpace failed\n",
            FILE__, __LINE__, getThreadStr(getExecThreadID()));
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
#if 0
      syscallTraps_ += (trappedSyscall);
#endif
      syscallTraps_.push_back(trappedSyscall);

      return trappedSyscall;
   }
   // Should never reach here.
   return NULL;
}

bool process::clearSyscallTrapInternal(syscallTrap *trappedSyscall) 
{
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

#if defined(os_aix)
   inferiorFree(trappedSyscall->trapAddr);
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
#if defined(os_aix)
   // lwps stopped in syscalls are unmodifiable.. so we 'fake' it
   return get_lwp_id();
#endif

   return status.pr_syscall;
}



bool dyn_lwp::stepPastSyscallTrap()
{
#if defined(os_aix)
   // We trap the exit, as lwps stopped in syscalls are unmodifiable
   changePC(trappedSyscall_->origLR, NULL);
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
#if defined(os_aix) 
bool dyn_lwp::decodeSyscallTrap(EventRecord &ev) 
{
   if (!trappedSyscall_) return false;

   Frame frame = getActiveFrame();

   if (frame.getPC() == trappedSyscall_->trapAddr) {
      ev.type = evtSyscallExit;
      ev.what = procSysOther;
      ev.info = trappedSyscall_->syscall_id;
      return true;
   }

   return false;
}
#else
bool dyn_lwp::decodeSyscallTrap(EventRecord & /* ev */) 
{
   return true;
}
#endif

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
      EventRecord &ev,
      EventRecord &matching_ev) 
{
   bool found = false;
   process *proc = ev.proc;
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


bool SignalGenerator::decodeEvents(pdvector<EventRecord> &events)
{
   assert(events.size() > 0);
   signal_printf("%s[%d]: decodeEvents entry, %d queued events\n",
         FILE__, __LINE__, events.size());

   // There'll only ever be one... Highlander!
   for (unsigned i = 0; i < events.size(); i++) {
      EventRecord &ev = events[i];

      if ((ev.info & POLLHUP) || (ev.info & POLLNVAL)) {
         // True if the process exited out from under us
         ev.lwp = ev.proc->getRepresentativeLWP();
         int status;
         int ret; 
         do {
            ret = waitpid(getPid(), &status, 0);
         } while ((ret < 0) && (errno == EINTR));
         if (ret < 0) { 
            //  if we get ECHILD it just means that the child process no longer exists.
            //  just create an exit event and keep quiet
            if (errno != ECHILD) {
               fprintf(stderr, "%s[%d]:  This shouldn't happen\n", FILE__, __LINE__);         
               perror("waitpid");
            }
            //  but really it _should_ be our child since 
            ev.type = evtProcessExit;
            ev.what = 0;
            ev.status = statusNormal; // If he exited via a signal, that gets handled in decodeWaitPidStatus
            status = 0;
            continue;
         }

         if (!decodeWaitPidStatus(status, ev)) {
            fprintf(stderr, "%s[%d]:  failed to decodeWaitPidStatus\n", FILE__, __LINE__);
            continue;
         }
         signal_printf("%s[%d]: after waitPidStatus, event %s\n",
               FILE__, __LINE__, eventType2str(ev.type));

         if (ev.type == evtSignalled && !decodeSignal(ev)) {
            fprintf(stderr, "%s[%d]:  failed to decodeSignal\n", FILE__, __LINE__);
            continue;
         }

         if (ev.type == evtUndefined) {
            fprintf(stderr, "%s[%d]:  undefined event\n", FILE__, __LINE__);
            continue;
         }

         signal_printf("%s[%d]:  new event: %s\n", FILE__, __LINE__, eventType2str(ev.type));
         continue;
      }

      procProcStatus_t procstatus;
      if(! ev.proc->getRepresentativeLWP()->get_status(&procstatus)) {
         if (ev.type == evtUndefined) {
            ev.type = evtProcessExit;
            fprintf(stderr, "%s[%d]:  file desc for process exit not available\n",
                  FILE__, __LINE__);
            continue;
         }
         fprintf(stderr, "%s[%d]:  file desc for %s not available\n",
               FILE__, __LINE__, eventType2str(ev.type));
         continue;
      }

      signal_printf("Thread status flags: 0x%x (STOPPED %d, ISTOP %d, ASLEEP %d)\n",
            procstatus.pr_flags,
            procstatus.pr_flags & PR_STOPPED,
            procstatus.pr_flags & PR_ISTOP,
            procstatus.pr_flags & PR_ASLEEP);
      signal_printf("Current signal: %d, reason for stopping: %d, (REQ %d, SIG %d, ENT %d, EXIT %d), what %d\n",
            procstatus.pr_cursig, procstatus.pr_why,
            procstatus.pr_why == PR_REQUESTED,
            procstatus.pr_why == PR_SIGNALLED,
            procstatus.pr_why == PR_SYSENTRY,
            procstatus.pr_why == PR_SYSEXIT,
            procstatus.pr_what);

      signal_printf("Signal encountered on LWP %d\n", procstatus.pr_lwpid);

      // copied from old code, must not care about events that don't stop proc
      // Actually, this happens if we've requested a stop but didn't wait for it; the process
      // is _actually_ running although we had thought it stopped.
      if ( !(procstatus.pr_flags & PR_STOPPED || procstatus.pr_flags & PR_ISTOP) ) {
         ev.proc->set_status(running);
         ev.type = evtIgnore;
         signal_printf("%s[%d]:  new event: %s\n",
               FILE__, __LINE__, eventType2str(ev.type));
         continue;
      }

      //  find the right dyn_lwp to work with
      /* unsigned target_lwp_id = (unsigned) procstatus.pr_lwpid; */
      dyn_lwp *lwp_to_use = ev.proc->getRepresentativeLWP();
      bool updated_events = false;

      if (ev.proc->real_lwps.size()) {
         if (ev.proc->real_lwps.find((unsigned) procstatus.pr_lwpid))
            lwp_to_use  = ev.proc->real_lwps[procstatus.pr_lwpid];
         else {
            lwp_to_use = ev.proc->getLWP(procstatus.pr_lwpid);
         }
      }

      ev.lwp = lwp_to_use;
      if (!ev.lwp) {
         fprintf(stderr, "%s[%d]:  no lwp, returning NULL event\n", FILE__, __LINE__);
         ev.type = evtNullEvent;
         return false;
      }

      signal_printf("%s[%d]: decodeEvents, calling decodeProcStatus...\n",
            FILE__, __LINE__);

      // Now decode the sucker
      if (!decodeProcStatus(procstatus, ev)) {
         fprintf(stderr, "%s[%d]:  failed to decodeProcStatus\n", FILE__, __LINE__);
         continue;
      }

      char buf[128];
      signal_printf("%s[%d]:  decodeEvent got %s, returning %d\n", FILE__, __LINE__, ev.sprint_event(buf), updated_events);
   }
   return true;
}

std::string process::tryToFindExecutable(const std::string &iprogpath, int pid) 
{
   char buffer[2];
   int result;
   // This is called by exec, so we might have a valid file path. If so,
   // use it... otherwise go to /proc. Helps with exec aliasing problems.
   buffer[0] = buffer[1] = '\0';
   if (iprogpath.c_str()) {
       int filedes = open(iprogpath.c_str(), O_RDONLY);
       if (filedes != -1) {
          result = read(filedes, buffer, 2);
          P_close(filedes);
          if (result != -1 && (buffer[0] != '#' || buffer[1] != '!')) {
             return iprogpath;
          }
       }
   }

  // We need to dereference the /proc link.
  // Case 1: multiple copies of the same file opened with multiple
  // pids will not match (and should)
  // Case 2: an exec'ed program will have the same /proc path,
  // but different program paths
  std::string procpath = std::string("/proc/") + utos(pid) + std::string("/object/a.out");

  // Sure would be nice if we could get the original....
  
  return procpath;
}

