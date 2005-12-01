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

// $Id: procfs.C,v 1.41 2005/12/01 00:56:25 jaw Exp $

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
  sigset_t signalSet;
  char procName[128];

  sprintf(procName,"/proc/%05d", (int)getpid());
  int fd = P_open(procName, O_RDWR, 0);
  if (fd < 0) {
    bperr( "osTraceMe: open failed: %s\n", sys_errlist[errno]); 
    fflush(stderr);
    P__exit(-1); // must use _exit here.
  }

  premptyset(&signalSet);
  praddset(&signalSet, SIGTRAP);
  praddset(&signalSet, SIGSTOP);
  praddset(&signalSet, SIGSEGV);
  if (ioctl(fd, PIOCSTRACE, &signalSet) == -1) {
      sprintf(errorLine, "Cannot trace singals\n");
      logLine(errorLine);
      P_close(fd);
      return;
  }

  long pr_flags = PR_STOPEXEC | PR_KLC;
  if (ioctl(fd, PIOCSET, &pr_flags) == -1) {
      sprintf(errorLine, "Cannot set status\n");
      logLine(errorLine);
      P_close(fd);
      return;
  }

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

bool lwp_isRunning_(int lwp_fd) {
   // determine if a process is running by doing low-level system checks, as
   // opposed to checking the 'status_' member vrble.  May assume that attach()
   // has run, but can't assume anything else.
   prstatus theStatus;
   if (-1 == ioctl(lwp_fd, PIOCSTATUS, &theStatus)) {
      perror("process::isRunning_()");
      assert(false);
   }

   if (theStatus.pr_flags & PR_STOPPED)
      return false;
   else
      return true;
}

bool process::isRunning_() const {
   return lwp_isRunning_(getRepresentativeLWP()->get_fd());
}

bool dyn_lwp::restoreRegisters_(const struct dyn_saved_regs &regs) 
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
   if (ioctl(fd_, PIOCSREG, &(regs.theIntRegs)) == -1) {
      logLine("dyn_lwp::restoreRegisters PIOCSREG failed");
      if (errno == EBUSY) {
         cerr << "It appears that the process was not stopped in the eyes of /proc" << endl;
	 assert(false);
      }
      return false;
   }

   if (ioctl(fd_, PIOCSFPREG, &(regs.theFpRegs)) == -1) {
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
bool dyn_lwp::continueLWP_(int signalToContinueWith) {
   prrun_t flags;
   prstatus_t stat;
   memset(&flags, '\0', sizeof(flags));
   flags.pr_flags = PRCFAULT; 

   // we don't want to operate on the process in this state
   ptraceOps++; 
   ptraceOtherOps++;

   int ret = ioctl(get_fd(), PIOCSTATUS, &stat);

   if (ret == -1) {
      perror("status error is ");
      return true;
   }

   if (!(stat.pr_flags & PR_STOPPED)) {
      return true;
   }

   void *arg3 = NULL;
   //prrun_t run;
   if(signalToContinueWith == dyn_lwp::NoSignal &&
      (stat.pr_flags & PR_STOPPED) && (stat.pr_why == PR_SIGNALLED)) {
         flags.pr_flags |= PRCSIG; // clear current signal
         arg3 = &flags;
   }

   if(changedPCvalue) {
      // if we are changing the PC, use the new value as the cont addr.
      flags.pr_flags |= PRSVADDR;
      flags.pr_vaddr = (char*)changedPCvalue;
      changedPCvalue = 0;
      arg3 = &flags;
   }

   if (ioctl(get_fd(), PIOCRUN, arg3) == -1) {
      bperr( "continueProc_: PIOCRUN failed: %s\n",
              sys_errlist[errno]);
      return false;
   }
   
   return true;
}


/*
   pause a process that is running
*/
bool dyn_lwp::stop_() {
  ptraceOps++; ptraceOtherOps++;

  //sysset_t scexit, scsavedexit;
  prstatus_t prstatus;
  int ioctl_ret;

  ioctl_ret = ioctl(get_fd(), PIOCSTOP, &prstatus);
  if (ioctl_ret == -1) {
      sprintf(errorLine,
      "warn : process::pause_ use ioctl to send PICOSTOP returns error : errno = %i\n", errno);
      perror("warn : process::pause_ ioctl PICOSTOP: ");
      logLine(errorLine);
      return 0;
  }

  if(lwp_isRunning_(get_fd())) {
      sprintf(errorLine,
      "warn : process::pause_ PICOSTOP's process, but still running\n");
      logLine(errorLine);
      return 0;
  }

  return 1;
}

bool dyn_lwp::getRegisters_(struct dyn_saved_regs *regs) {
   if (ioctl(fd_, PIOCGREG, &(regs->theIntRegs)) == -1) {
      perror("dyn_lwp::getRegisters PIOCGREG");
      if (errno == EBUSY) {
         cerr << "It appears that the process was not stopped in the eyes "
              << "of /proc" << endl;
         assert(false);
      }      
      return false;
   }
   
   if (ioctl(fd_, PIOCGFPREG, &(regs->theFpRegs)) == -1) {
      perror("dyn_lwp::getRegisters PIOCGFPREG");
      if (errno == EBUSY)
         cerr << "It appears that the process was not stopped in the eyes "
              << "of /proc" << endl;
      else if (errno == EINVAL)
         // what to do in this case?  Probably shouldn't even do a print,
         // right?  And it certainly shouldn't be an error, right?
         cerr << "It appears that this machine doesn't have floating-point "
              << "instructions" << endl;      
      return false;
   }

   return true;
}

// PC is changed when we continue the process
bool dyn_lwp::changePC(Address addr, struct dyn_saved_regs * /*savedRegs*/) 
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
        if (syscallTraps_[iter]->syscall_id == (unsigned int) syscall) {
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
        trappedSyscall = new syscallTrap;
        trappedSyscall->refcount = 1;
        trappedSyscall->syscall_id = (int) syscall;
        sysset_t *save_exitset = new sysset_t;
        
        dyn_lwp *replwp = getRepresentativeLWP();
        if (-1 == ioctl(replwp->get_fd(), PIOCGEXIT, save_exitset))
            return NULL;
        trappedSyscall->saved_data = (void *)save_exitset;

        sysset_t new_exitset;
        prfillset(&new_exitset);
        
        if (-1 == ioctl(replwp->get_fd(), PIOCSEXIT, &new_exitset))
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

    if (-1 == ioctl(getRepresentativeLWP()->get_fd(), PIOCSEXIT, save_exitset))
        return false;
    
    // Now that we've reset the original behavior, remove this
    // entry from the vector
    pdvector<syscallTrap *> newSyscallTraps;
    for (unsigned iter = 0; iter < syscallTraps_.size(); iter++) {
      if (trappedSyscall != syscallTraps_[iter])
	newSyscallTraps.push_back(syscallTraps_[iter]);
    }
    syscallTraps_ = newSyscallTraps;
    /*
    // Now that we've reset the original behavior, remove this
    // entry from the vector
    for (unsigned iter = 0; iter < syscallTraps_.size(); iter++) {
        if (trappedSyscall == syscallTraps_[iter]) {
            syscallTraps_.removeByIndex(iter);
            break;
        }
    }
    */
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

    // Due to lack of information, we assume this was not the
    // syscall we wanted. 
    return 0;
}

bool dyn_lwp::writeTextWord(caddr_t inTraced, int data) {
   return writeDataSpace(inTraced, sizeof(int), (caddr_t) &data);
}

bool dyn_lwp::writeTextSpace(void  *inTracedProcess, u_int amount,
                             const void *inSelf) {
   return writeDataSpace(inTracedProcess, amount, inSelf);
}

#ifdef BPATCH_SET_MUTATIONS_ACTIVE
bool dyn_lwp::readTextSpace(void *inTraced, u_int amount, const void *inSelf) {
   return readDataSpace(inTraced, amount, const_cast<void*>(inSelf));
}
#endif

bool dyn_lwp::writeDataSpace(void *inTracedProcess, u_int amount,
                             const void *inSelf)
{
   ptraceOps++; ptraceBytes += amount;

   off_t loc = (off_t) inTracedProcess;
   unsigned int written = pwrite(get_fd(), inSelf, amount, loc);
   if (written != amount) {
      fprintf(stderr, "%s[%d]:  writeDataSpace: %s\n", FILE__, __LINE__, strerror(errno));
      assert(0);
      return false;
   }
   return true;
}

bool dyn_lwp::readDataSpace(const void *inTracedProcess, u_int amount,
                            void *inSelf)
{
   off_t ret;
   ptraceOps++; ptraceBytes += amount;


  off_t loc = (off_t) inTracedProcess;
  int res = pread(get_fd(), inSelf, amount, loc);
  if (res != (int) amount) {
      perror("readDataSpace");
      bperr( "From %p (mutator) to %p (mutatee), %d bytes, got %d\n",
              inSelf, inTracedProcess, amount, res);
      return false;
  }
  return true;
}

bool dyn_lwp::waitUntilStopped() {
   return true;
}

bool process::waitUntilStopped() {
   return true;
}

