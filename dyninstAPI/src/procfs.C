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

// $Id: procfs.C,v 1.21 2003/02/28 22:13:37 bernat Exp $

#include "symtab.h"
#include "common/h/headers.h"
#include "os.h"
#include "process.h"
#include "dyn_lwp.h"
#include "stats.h"
#include "common/h/Types.h"
#include <sys/ioctl.h>
#include <fcntl.h>
#include <sys/termios.h>
#include <unistd.h>
#include "showerror.h"

#include <sys/procfs.h>
#include <sys/syscall.h>
#include <sys/fault.h>
#include <poll.h>
#include <limits.h>

// PC register index into the gregset_t array for the alphas
#define PC_REGNUM 31
#define GP_REGNUM 27

extern "C" {
extern int ioctl(int, int, ...);
extern long sysconf(int);
};

/*
   osTraceMe is called after we fork a child process to set
   a breakpoint on the exit of the exec system call.
   When /proc is used, this breakpoint **will not** cause a SIGTRAP to 
   be sent to the process. The parent should use PIOCWSTOP to wait for 
   the child.
*/
void OS::osTraceMe(void) {
  sysset_t exitSet;
  char procName[128];

  sprintf(procName,"/proc/%05d", (int)getpid());
  int fd = P_open(procName, O_RDWR, 0);
  if (fd < 0) {
    fprintf(stderr, "osTraceMe: open failed: %s\n", sys_errlist[errno]); 
    fflush(stderr);
    P__exit(-1); // must use _exit here.
  }

  /* set a breakpoint at the exit of exec/execve */
  premptyset(&exitSet);
#ifdef SYS_exec
  praddset(&exitSet, SYS_exec);
#endif
#ifdef SYS_execve
  praddset(&exitSet, SYS_execve);
#endif

  /* DIGITAL UNIX USES THIS */
#ifdef SYS_execv
  praddset(&exitSet,SYS_execv);
#endif

  if (ioctl(fd, PIOCSEXIT, &exitSet) < 0) {
    fprintf(stderr, "osTraceMe: PIOCSEXIT failed: %s\n", sys_errlist[errno]); 
    fflush(stderr);
    P__exit(-1); // must use _exit here.
  }

  long pr_flags;
  if (ioctl(fd, PIOCGSPCACT, &pr_flags) < 0) {
      sprintf(errorLine, "Cannot get status\n");
      logLine(errorLine);
      close(fd);
      return;
  }
  pr_flags |= PRFS_STOPEXEC;	/* stop on exec */
  pr_flags |= PRFS_KOLC;	/* add kill on last close flag */
  ioctl(fd, PIOCSSPCACT, &pr_flags);
 
  errno = 0;
  return;
}


// already setup on this FD.
// disconnect from controlling terminal 
void OS::osDisconnect(void) {
  int ttyfd = open ("/dev/tty", O_RDONLY);
  ioctl (ttyfd, TIOCNOTTY, NULL); 
  P_close (ttyfd);
}

bool process::stop_()
{
  assert(false);
}

bool process::isRunning_() const {
   // determine if a process is running by doing low-level system checks, as
   // opposed to checking the 'status_' member vrble.  May assume that attach()
   // has run, but can't assume anything else.
   prstatus theStatus;
   if (-1 == ioctl(getDefaultLWP()->get_fd(), PIOCSTATUS, &theStatus)) {
      perror("process::isRunning_()");
      assert(false);
   }

   if (theStatus.pr_flags & PR_STOPPED)
      return false;
   else
      return true;
}


bool dyn_lwp::restoreRegisters(struct dyn_saved_regs *regs) 
{
#ifdef __alpha 
   prstatus info;
   ioctl(fd_, PIOCSTATUS,  &info); 
   while (!prismember(&info.pr_flags, PR_STOPPED))
   { 
       sleep(1);
       ioctl(fd_, PIOCSTATUS,  &info);
   }
   errno = 0;
#endif
   if (ioctl(fd_, PIOCSREG, &(regs->theIntRegs)) == -1) {
      logLine("dyn_lwp::restoreRegisters PIOCSREG failed");
      if (errno == EBUSY) {
         cerr << "It appears that the process was not stopped in the eyes of /proc" << endl;
	 assert(false);
      }
      return false;
   }

   if (ioctl(fd_, PIOCSFPREG, &(regs->theFpRegs)) == -1) {
      logLine("dyn_lwp::restoreRegisters PIOCSFPREG failed");
      if (errno == EBUSY) {
         cerr << "It appears that the process was not stopped in the eyes of /proc" << endl;
         assert(false);
      }
      return false;
   }
   return true;
}



/* 
   continue a process that is stopped 
*/
bool process::continueProc_() {
  ptraceOps++; ptraceOtherOps++;
  prrun_t flags;
  prstatus_t stat;
  cerr << "ContinueProc_" << endl;
  
  memset(&flags, '\0', sizeof(flags));
  flags.pr_flags = PRCFAULT; 

  int ret = ioctl(getDefaultLWP()->get_fd(), PIOCSTATUS, &stat);

  if (ret == -1) {
      perror("status error is ");
      return true;
  }

  if (!(stat.pr_flags & PR_STOPPED)) {
	return true;
  }

  if ((stat.pr_flags & PR_STOPPED) && (stat.pr_why == PR_SIGNALLED)) {
      flags.pr_flags |= PRCSIG; // clear current signal
  }
  cerr << "changedPC at " << getDefaultLWP()->changedPCvalue << endl;
  
  if (getDefaultLWP()->changedPCvalue) {
      // if we are changing the PC, use the new value as the cont addr.
      flags.pr_flags |= PRSVADDR;
      fprintf(stderr, "Continue: PC at 0x%x\n", getDefaultLWP()->changedPCvalue);
      
      flags.pr_vaddr = (char*)getDefaultLWP()->changedPCvalue;
      getDefaultLWP()->changedPCvalue = 0;
  }

  if (ioctl(getDefaultLWP()->get_fd(), PIOCRUN, &flags) == -1) {
    fprintf(stderr, "continueProc_: PIOCRUN failed: %s\n", sys_errlist[errno]);
    return false;
  }

  return true;
}


/*
   pause a process that is running
*/
bool process::pause_() {
  ptraceOps++; ptraceOtherOps++;

  sysset_t scexit, scsavedexit;
  prstatus_t prstatus;
  int ioctl_ret;

  ioctl_ret = ioctl(getDefaultLWP()->get_fd(), PIOCSTOP, &prstatus);
  if (ioctl_ret == -1) {
      sprintf(errorLine,
      "warn : process::pause_ use ioctl to send PICOSTOP returns error : errno = %i\n", errno);
      perror("warn : process::pause_ ioctl PICOSTOP: ");
      logLine(errorLine);
      return 0;
  }

  if (isRunning_()) {
      sprintf(errorLine,
      "warn : process::pause_ PICOSTOP's process, but still running\n");
      logLine(errorLine);
      return 0;
  }

  return 1;
}

/*
   close the file descriptor for the file associated with a process
*/
bool process::detach_() {
  delete getDefaultLWP();
  return true;
}

bool process::continueWithForwardSignal(int) {
   if (-1 == ioctl(getDefaultLWP()->get_fd(), PIOCRUN, NULL)) {
      perror("could not forward signal in PIOCRUN");
      return false;
   }

   return true;
}

struct dyn_saved_regs *dyn_lwp::getRegisters() {
    struct dyn_saved_regs *regs = new dyn_saved_regs();
    
   if (ioctl(fd_, PIOCGREG, &(regs->theIntRegs)) == -1) {
      perror("dyn_lwp::getRegisters PIOCGREG");
      if (errno == EBUSY) {
         cerr << "It appears that the process was not stopped in the eyes of /proc" << endl;
	 assert(false);
      }

      return NULL;
   }

   if (ioctl(fd_, PIOCGFPREG, &(regs->theFpRegs)) == -1) {
      perror("dyn_lwp::getRegisters PIOCGFPREG");
      if (errno == EBUSY)
         cerr << "It appears that the process was not stopped in the eyes of /proc" << endl;
      else if (errno == EINVAL)
	 // what to do in this case?  Probably shouldn't even do a print, right?
	 // And it certainly shouldn't be an error, right?
	 cerr << "It appears that this machine doesn't have floating-point instructions" << endl;

      return NULL;
   }
   return regs;
}

// PC is changed when we continue the process
bool dyn_lwp::changePC(Address addr, struct dyn_saved_regs *savedRegs) 
{
   changedPCvalue = addr;

   return true;
}

bool dyn_lwp::executingSystemCall() 
{
  prstatus theStatus;
  if (ioctl(fd_, PIOCSTATUS, &theStatus) != -1) {
    if ((theStatus.pr_flags & PR_ISSYS) == PR_ISSYS) {
      return(true);
    }
  } else assert(0);
  return(false);
}


// The syscall parameter is ignored here. We can't tell the current
// syscall on OSF (as far as I can tell), and so we need to use
// the previous behavior: trap all of them. 
syscallTrap *process::trapSyscallExitInternal(Address syscall) {
    syscallTrap *trappedSyscall = NULL;
    
    // First, the cross-platform bit. If we're already trapping
    // on this syscall, then increment the reference counter
    // and return
    
    for (unsigned iter = 0; iter < syscallTraps_.size(); iter++) {
        if (syscallTraps_[iter]->syscall_id == (int) syscall) {
            trappedSyscall = syscallTraps_[iter];
            cerr << "Found previously trapped syscall at slot " << iter << endl;
            break;
        }
    }
    if (trappedSyscall) {
        // That was easy...
        trappedSyscall->refcount++;
        cerr << "Syscall refcount = " << trappedSyscall->refcount;
        return trappedSyscall;
    }
    else {
        trappedSyscall = new syscallTrap;
        trappedSyscall->refcount = 1;
        trappedSyscall->syscall_id = (int) syscall;
        sysset_t *save_exitset = new sysset_t;
        
        if (-1 == ioctl(getDefaultLWP()->get_fd(), PIOCGEXIT, save_exitset))
            return NULL;
        trappedSyscall->saved_data = (void *)save_exitset;

        sysset_t new_exitset;
        prfillset(&new_exitset);
        
        if (-1 == ioctl(getDefaultLWP()->get_fd(), PIOCSEXIT, &new_exitset))
            return NULL;
        
        syscallTraps_ += trappedSyscall;
        return trappedSyscall;
    }
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
    sysset_t *save_exitset = (sysset_t *) trappedSyscall->saved_data;

    if (-1 == ioctl(getDefaultLWP()->get_fd(), PIOCSEXIT, save_exitset))
        return false;
    
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
    prstatus theStatus;
    if (ioctl(fd_, PIOCSTATUS, &theStatus) != -1) {
        if ((theStatus.pr_flags & PR_ISSYS) == PR_ISSYS) {
            return 1;
        }
    }
    return 0;
}

bool dyn_lwp::stepPastSyscallTrap() {
    // Don't believe we have to do this
    return true;
}

// 0: not reached syscall trap
// 1: lwp that isn't blocking reached syscall trap
// 2: lwp that is blocking reached syscall trap
int dyn_lwp::hasReachedSyscallTrap() {
    prstatus theStatus;
    if (ioctl(fd_, PIOCSTATUS, &theStatus) == -1) {
        return 0;
    }
    if (theStatus.pr_why != PR_SYSEXIT) {
        return 0;
    }

    // Due to lack of information, we assume this was the 
    // correct syscall.
    return 2;
}

#ifdef BPATCH_LIBRARY
/*
   detach from thr process, continuing its execution if the parameter "cont"
   is true.
 */
bool process::API_detach_(const bool cont)
{
  // Reset the kill-on-close flag, and the run-on-last-close flag if necessary
  long pr_flags = 0;
  if (ioctl(getDefaultLWP()->get_fd(), PIOCSSPCACT, &pr_flags) < 0) {
      sprintf(errorLine, "Cannot get status\n");
      logLine(errorLine);
      return false;
  }

  // Set the run-on-last-close-flag if necessary
  if (cont) {
    long pr_flags = 1;
    if (ioctl (getDefaultLWP()->get_fd(),PIOCSRLC, &pr_flags) < 0) {
      fprintf(stderr, "detach: PIOCSET failed: %s\n", sys_errlist[errno]);
      return false;
    }
  }

  sigset_t sigs;
  premptyset(&sigs);
  if (ioctl(getDefaultLWP()->get_fd(), PIOCSTRACE, &sigs) < 0) {
    fprintf(stderr, "detach: PIOCSTRACE failed: %s\n", sys_errlist[errno]);
    return false;
  }
  if (ioctl(getDefaultLWP()->get_fd(), PIOCSHOLD, &sigs) < 0) {
    fprintf(stderr, "detach: PIOCSHOLD failed: %s\n", sys_errlist[errno]);
    return false;
  }

  fltset_t faults;
  premptyset(&faults);
  if (ioctl(getDefaultLWP()->get_fd(), PIOCSFAULT, &faults) < 0) {
    fprintf(stderr, "detach: PIOCSFAULT failed: %s\n", sys_errlist[errno]);
    return false;
  }

  sysset_t syscalls;
  premptyset(&syscalls);
  if (ioctl(getDefaultLWP()->get_fd(), PIOCSENTRY, &syscalls) < 0) {
    fprintf(stderr, "detach: PIOCSENTRY failed: %s\n", sys_errlist[errno]);
    return false;
  }
  if (ioctl(getDefaultLWP()->get_fd(), PIOCSEXIT, &syscalls) < 0) {
    fprintf(stderr, "detach: PIOCSEXIT failed: %s\n", sys_errlist[errno]);
    return false;
  }

  delete getDefaultLWP();
  return true;
}
#endif

bool process::writeTextWord_(caddr_t inTraced, int data) {
  return writeDataSpace_(inTraced, sizeof(int), (caddr_t) &data);
}

bool process::writeTextSpace_(void  *inTracedProcess, u_int amount,const void *inSelf) {
  return writeDataSpace_(inTracedProcess, amount, inSelf);
}

#ifdef BPATCH_SET_MUTATIONS_ACTIVE
bool process::readTextSpace_(void *inTraced, u_int amount, const void *inSelf) {
  return readDataSpace_(inTraced, amount, (void*)inSelf);
}
#endif

bool process::writeDataSpace_(void *inTracedProcess, u_int amount,const void *inSelf) {
  off_t ret;
  ptraceOps++; ptraceBytes += amount;

#ifdef __alpha
  errno = 0;
  prmap_t tmp;
  tmp.pr_vaddr = (char*)inTracedProcess;
  ret = lseek(getDefaultLWP()->get_fd(), (off_t) tmp.pr_vaddr, SEEK_SET);
#else
  ret =  lseek(getDefaultLWP()->get_fd(), (off_t)inTracedProcess, SEEK_SET);
#endif  

  if (ret != (off_t)inTracedProcess) {
      perror("lseek");
      fprintf(stderr, "   target address %lx\n", inTracedProcess);
      return false;
  }
  return (write(getDefaultLWP()->get_fd(), inSelf, amount) == (int)amount);
}

bool process::readDataSpace_(const void *inTracedProcess, u_int amount, void *inSelf) {
  off_t ret;
  ptraceOps++; ptraceBytes += amount;
#ifdef __alpha   
  prstatus info;
  ioctl(getDefaultLWP()->get_fd(), PIOCSTATUS,  &info);
  while (!prismember(&info.pr_flags, PR_STOPPED))
  {
     sleep(1);
     ret = ioctl(getDefaultLWP()->get_fd(), PIOCSTATUS,  &info);
     if (ret == -1) return false;
  } 
  errno = 0;
#endif  
  ret = lseek(getDefaultLWP()->get_fd(), reinterpret_cast<off_t>(inTracedProcess), SEEK_SET);

  if (ret != (off_t)inTracedProcess) {
      perror("lseek");
      fprintf(stderr, "   target address %lx\n", inTracedProcess);
      fprintf(stderr, "lseek(%d,%lx,%d)\n", getDefaultLWP()->get_fd(), inTracedProcess, SEEK_SET); 
      fprintf(stderr, "The return address: %lx\n",ret); 
      #ifdef DEBUG
        if (errno == EBADF)
        {
                perror("The fildes argument is not an open file descriptor.\n");
        }
        else if (errno == EINVAL)
        {
                perror("The whence argument is not SEEK_SET, SEEK_CUR...\n");
        }
        else if (errno == ESPIPE)
        {
                perror("ESPIPE error\n");
        }
        else
        {
                perror("Unknown error\n");
        }
     #endif
     return false;
  }
  return (read(getDefaultLWP()->get_fd(), inSelf, amount) == (int)amount);
}

bool process::loopUntilStopped() {
  assert(0);
}

#ifdef notdef
// TODO -- only call getrusage once per round
static struct rusage *get_usage_data() {
  return NULL;
}
#endif

float OS::compute_rusage_cpu() {
  return 0;
}

float OS::compute_rusage_sys() {
  return 0;
}

float OS::compute_rusage_min() {
  return 0;
}
float OS::compute_rusage_maj() {
  return 0;
}

float OS::compute_rusage_swap() {
  return 0;
}
float OS::compute_rusage_io_in() {
  return 0;
}
float OS::compute_rusage_io_out() {
  return 0;
}
float OS::compute_rusage_msg_send() {
  return 0;
}
float OS::compute_rusage_msg_recv() {
  return 0;
}
float OS::compute_rusage_sigs() {
  return 0;
}
float OS::compute_rusage_vol_cs() {
  return 0;
}
float OS::compute_rusage_inv_cs() {
  return 0;
}

