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

// $Id: irix.C,v 1.82 2004/05/13 12:20:18 bernat Exp $

#include <sys/types.h>    // procfs
#include <sys/signal.h>   // procfs
#include <sys/fault.h>    // procfs
#include <sys/syscall.h>  // procfs
#include <sys/procfs.h>   // procfs
#include <unistd.h>       // getpid()
#include <sys/ucontext.h> // gregset_t

// #include "common/h/Types.h"

#include "dyninstAPI/src/arch-mips.h"
#include "dyninstAPI/src/inst-mips.h"
#include "dyninstAPI/src/symtab.h" // pd_Function
#include "dyninstAPI/src/instPoint.h"
#include "dyninstAPI/src/process.h"
#include "dyninstAPI/src/dyn_lwp.h"
#include "dyninstAPI/src/frame.h"
#include "dyninstAPI/src/instP.h"
#include "dyninstAPI/src/stats.h" // ptrace{Ops,Bytes}
#include "dyninstAPI/src/dyn_thread.h"
#include "common/h/pathName.h" // expand_tilde_pathname, exists_executable
#include "common/h/irixKludges.h" // PDYN_XXX
#include "common/h/Time.h"
#include "common/h/timing.h"
#include "dyninstAPI/src/signalhandler.h"
#include <limits.h>       // poll()
#include <stropts.h>      // poll()
#include <poll.h>         // poll()
#include <errno.h>        // errno
#include <stdio.h>        // perror()
#include <sys/sysmp.h>    // sysmp()
#include <sys/sysinfo.h>  // sysmp()
#include <string.h>       // strncmp()
#include <stdlib.h>       // getenv()
#include <termio.h>       // TIOCNOTTY
#include <sys/timers.h>   // PTIMER macros
#include <sys/hwperfmacros.h> // r10k_counter macros 
#include <sys/hwperftypes.h>  // r10k_counter types
#include <dlfcn.h>


extern unsigned enable_pd_inferior_rpc_debug;

#if ENABLE_DEBUG_CERR == 1
#define inferiorrpc_cerr if (enable_pd_inferior_rpc_debug) cerr
#else
#define inferiorrpc_cerr if (0) cerr
#endif /* ENABLE_DEBUG_CERR == 1 */

extern char *Bool[];
#ifndef BPATCH_LIBRARY
extern pdstring osName;
#endif

char* irixMPIappName;
bool attachToIrixMPIprocess(const pdstring &progpath, int pid, int afterAttach);
int masterMPIfd;
int masterMPIpid;

// constants derived from "tramp-mips.s" (must remain consistent)
// size of basetramp stack frame
// offset of save insns from basetramp "daddiu sp,sp,-512"
static const int bt_ra_save_off = (64 * INSN_SIZE); 
static const int bt_fp_save_off = (63 * INSN_SIZE); 
// basetramp stack frame slots
static const int bt_ra_slot = -512;
static const int bt_fp_slot = -504;
static const int bt_frame_size = 512;

void print_proc_flags(int fd)
{
  prstatus stat;
  ioctl(fd, PIOCSTATUS, &stat);
  bperr( "flags: ");

  if (stat.pr_flags & PR_STOPPED) bperr( "PR_STOPPED ");
  if (stat.pr_flags & PR_ISTOP) bperr( "PR_ISTOP ");
  if (stat.pr_flags & PR_DSTOP) bperr( "PR_DSTOP ");
  if (stat.pr_flags & PR_STEP) bperr( "PR_STEP ");
  if (stat.pr_flags & PR_ASLEEP) bperr( "PR_ASLEEP ");
  if (stat.pr_flags & PR_PCINVAL) bperr( "PR_PCINVAL ");
  //if (stat.pr_flags & PR_ISSYS) bperr( "PR_ISSYS ");
  if (stat.pr_flags & PR_FORK) bperr( "PR_FORK ");
  if (stat.pr_flags & PR_RLC) bperr( "PR_RLC ");
  if (stat.pr_flags & PR_KLC) bperr( "PR_KLC ");
  if (stat.pr_flags & PR_PTRACE) bperr( "PR_PTRACE ");

  if (stat.pr_flags & PR_ISKTHREAD) bperr( "PR_ISKTHREAD ");
  if (stat.pr_flags & PR_JOINTSTOP) bperr( "PR_JOINTSTOP ");
  if (stat.pr_flags & PR_JOINTPOLL) bperr( "PR_JOINTPOLL ");
  if (stat.pr_flags & PR_RETURN) bperr( "PR_RETURN ");
  if (stat.pr_flags & PR_CKF) bperr( "PR_CKF ");

  bperr( "\n");
}

void print_proc_regs(int fd)
{
  gregset_t regs;
  if (ioctl (fd, PIOCGREG, &regs) == -1) {
    perror("ioctl(PIOCGREG)");
    return;
  }
  char buf[32];
  for (int i = 0; i < 32; i++) {
    sprintf(buf, "$%s", reg_names[i]);
    bperr( "%5s: %#10x %s", buf, (unsigned)regs[i],
	    (i % 4 == 3) ? ("\n") : (","));
  }
  bperr( "%5s: %#10x\n", "$pc", (unsigned)regs[CTX_EPC]);
}

void print_proc_pc(int fd)
{
  gregset_t regs;
  if (ioctl (fd, PIOCGREG, &regs) == -1) {
    perror("ioctl(PIOCGREG)");
    return;
  }
  bperr( "%5s: %#10x\n", "$pc", (unsigned)regs[CTX_EPC]);
}

bool dyn_lwp::readDataSpace(const void *inTraced, u_int nbytes, void *inSelf)
{
   //bperr( ">>> process::readDataSpace_(%d@0x%016lx)\n", 
   //        nbytes, inTraced);
   ptraceOps++; 
   ptraceBytes += nbytes;

   if(lseek(get_fd(), (off_t)inTraced, SEEK_SET) == -1)
   {
      perror("process::readDataSpace_(lseek)");
      return false;
   }
   
   // TODO: check for infinite loop if read returns 0?
   char *dst = (char *)inSelf;
   for (int last, left = nbytes; left > 0; left -= last) {
      if ((last = read(get_fd(), dst + nbytes - left, left)) == -1)
      {
         perror("process::readDataSpace_(read)");
         return false;
      } else if (last == 0) {
         bperr( "process::readDataSpace_(read=%d@0x%016lx,"
                 "left=%d/%d\n", last, inTraced, left, nbytes);
         return false;
      }
   }
   return true;
}

bool dyn_lwp::writeDataSpace(void *inTraced, u_int nbytes, const void *inSelf)
{
   //bperr( ">>> process::writeDataSpace_(%d@0x%016lx)\n", 
   //        nbytes, inTraced);
   ptraceOps++; 
   ptraceBytes += nbytes;

   if(lseek(get_fd(), (off_t)inTraced, SEEK_SET) == -1)
   {
      perror("process::writeDataSpace_(lseek)");
      return false;
   }
   
   // TODO: check for infinite loop if write returns 0?
   char *src = (char *)const_cast<void*>(inSelf);
   for (int last, left = nbytes; left > 0; left -= last) {
      if ((last = write(get_fd(), src + nbytes - left, left)) == -1)
      {
         perror("process::writeDataSpace_(write)");
         return false;
      }
   }
   return true;
}

bool dyn_lwp::writeTextWord(caddr_t inTraced, int data)
{
   //bperr( ">>> process::writeTextWord_()\n");
   return writeDataSpace(inTraced, INSN_SIZE, &data);
}

bool dyn_lwp::writeTextSpace(void *inTraced, u_int amount, const void *inSelf)
{
   //bperr( ">>> process::writeTextSpace_()\n");
   return writeDataSpace(inTraced, amount, inSelf);
}

#ifdef BPATCH_SET_MUTATIONS_ACTIVE
bool dyn_lwp::readTextSpace(void *inTraced, u_int amount, const void *inSelf)
{
   //bperr( ">>> process::readTextSpace_()\n");
   return readDataSpace(inTraced, amount, (void *)inSelf);
}
#endif

bool dyn_lwp::getRegisters_(struct dyn_saved_regs *regs) 
{
   if (ioctl(fd_, PIOCGREG, &(regs->intRegs)) == -1) {
      perror("dyn_lwp::getRegisters(PIOCGREG)");
      assert(errno != EBUSY); // procfs thinks the process is active
      return false;
   }
   
   if (ioctl(fd_, PIOCGFPREG, &(regs->fpRegs)) == -1) {
      perror("dyn_lwp::getRegisters(PIOCGFPREG)");
      assert(errno != EBUSY);  // procfs thinks the process is active
      assert(errno != EINVAL); // no floating-point hardware
      return false;
   }
   
   return true;
}

bool dyn_lwp::restoreRegisters_(const struct dyn_saved_regs &regs)
{
  if (ioctl(fd_, PIOCSREG, &(regs.intRegs)) == -1) {
    perror("dyn_lwp::restoreRegisters(PIOCSREG)");
    assert(errno != EBUSY); // procfs thinks the process is active
    return false;
  }  
  if (ioctl(fd_, PIOCSFPREG, &(regs.fpRegs)) == -1) {
    perror("dyn_lwp::restoreRegisters(PIOCSFPREG)");
    assert(errno != EBUSY);  // procfs thinks the process is active
    assert(errno != EINVAL); // no floating-point hardware
    return false;
  }

  return true;
}

bool dyn_lwp::changePC(Address addr, struct dyn_saved_regs *regs)
{

  /* copy integer registers from register buffer */
  gregset_t intRegs;

  if (regs) {
    // FUGGLY someone please fix this (as in Solaris)
    memcpy(&intRegs, &(regs->intRegs), sizeof(gregset_t));
  }
  else {
    if (ioctl(fd_, PIOCGREG, &intRegs) == -1) {
      perror("dyn_lwp::changePC(PIOCGREG)");
      assert(errno != EBUSY); // procfs thinks the process is active
      return false;
    }
  }
  intRegs[PROC_REG_PC] = addr; // set PC
  
  if (ioctl(fd_, PIOCSREG, &intRegs) == -1) {
    perror("dyn_lwp::changePC(PIOCSREG)");
    assert(errno != EBUSY); // procfs thinks the process is active
    return false;
  }
  
  return true;
}

bool process::isRunning_() const
{
  //bperr( ">>> process::isRunning_()\n");
  prstatus status;
  if (ioctl(getRepresentativeLWP()->get_fd(), PIOCSTATUS, &status) == -1) {
    perror("process::isRunning_()");
    assert(0);
  }

  // TODO - check for discrepancy with "status_" member?
  if (status.pr_flags & PR_STOPPED) return false;

  return true;
}

void OS::osTraceMe(void)
{
  char procName[128];
  sprintf(procName,"/proc/%05d", (int)getpid());
  //bperr( ">>> OS::osTraceMe(%s)\n", procName);
  int fd = P_open(procName, O_RDWR, 0);
  if (fd < 0) {
    bperr( "osTraceMe: open failed: %s\n", sys_errlist[errno]); 
    fflush(stderr);
    P__exit(-1); // must use _exit here.
  }

  // Procfs on Irix sets the "run-on-last-close" flag (PR_RLC) by default.
  // This flag needs to be unset to avoid the PIOCSEXIT trace (below) from
  // being lost when the child closes the proc file descriptor to itself.
  /* reset "run-on-last-close" flag */
  long flags = PR_RLC;
  if (ioctl(fd, PIOCRESET, &flags) < 0) {
    bperr( "osTraceMe: PIOCRESET failed: %s\n", sys_errlist[errno]); 
    fflush(stderr);
    P__exit(-1); // must use _exit here.
  };

  /* set a breakpoint at the exit of execv/execve */
  sysset_t exitSet;
  premptyset(&exitSet);
  (void)praddset(&exitSet, SYS_execve);
  if (ioctl(fd, PIOCSEXIT, &exitSet) < 0) {
    bperr( "osTraceMe: PIOCSEXIT failed: %s\n", sys_errlist[errno]); 
    fflush(stderr);
    P__exit(-1); // must use _exit here.
  }

  errno = 0;
  close(fd);
}

procSyscall_t decodeSyscall(process *p, procSignalWhat_t syscall)
{
    if (syscall == SYS_fork)
        return procSysFork;
    if (syscall == SYS_execve)
        return procSysExec;
    if (syscall == SYS_exit)
        return procSysExit;
    return procSysOther;
}

int decodeProcStatus(process *proc,
                     procProcStatus_t status,
                     procSignalWhy_t &why,
                     procSignalWhat_t &what,
                     procSignalInfo_t &info) {
    
    switch (status.pr_why) {
  case PR_SIGNALLED:
      why = procSignalled;
      what = status.pr_what;
      break;
  case PR_SYSENTRY:
      why = procSyscallEntry;
      what = status.pr_what;
      // HACK: We need to know the exec'ed file name
      // (on IRIX) so that we can find the binary to parse.
      // We get this by checking the first argument of the exec
      // call and storing it until the post-exec handling.
      // Problem is, we can't just check the first argument:
      // the function call part of the exec appears to be doing
      // some modifications and so the argument to exec() is not the
      // same as the argument to the exec syscall. By experimentation
      // I've discovered register 18 holds the original argument.
      // This is considered a hack and should be fixed by someone more
      // clueful about IRIX than I am.
      if (decodeSyscall(proc, what) == procSysExec)
          info = (procSignalInfo_t) status.pr_reg[18];
      else
          info = (procSignalInfo_t) status.pr_reg[REG_A0];
      break;
  case PR_SYSEXIT:
      why = procSyscallExit;
      what = status.pr_what;
      info = (procSignalInfo_t) status.pr_reg[PROC_REG_RV];
      break;
  case PR_REQUESTED:
      // We don't expect PR_REQUESTED in the signal handler
      assert(0 && "PR_REQUESTED not handled");
      break;
  case PR_JOBCONTROL:
  case PR_FAULTED:
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

bool signalHandler::checkForProcessEvents(pdvector<procevent *> *events,
                                          int wait_arg, bool block)
{
    extern pdvector<process*> processVec;
    static struct pollfd fds[OPEN_MAX];  // argument for poll
    // Number of file descriptors with events pending
    static int selected_fds = 0; 
    // The current FD we're processing.
    static int curr = 0;
    prstatus_t stat;
    bool any_active_procs = false;
    procSignalWhy_t  why  = procUndefined;
    procSignalWhat_t what = 0;
    procSignalInfo_t info = 0;

    if (selected_fds == 0) {
        for (unsigned u = 0; u < processVec.size(); u++) {
            //bperr("checking %d\n", processVec[u]->getPid());
            if (processVec[u] && 
                (processVec[u]->status() == running || 
                 processVec[u]->status() == neonatal)) {
                if (wait_arg == -1 ||
                    processVec[u]->getPid() == wait_arg) {
                    any_active_procs = true;
                    fds[u].fd = processVec[u]->getRepresentativeLWP()->get_fd();
                }
            } else {
                fds[u].fd = -1;
            }	
            // IRIX note:  Dave Anderson at SGI seems to think that it is
            // "ill-defined" what band signals show up in.  He suggests
            // that we do something like
            //    fds[i].events = POLLPRI | POLLRDBAND;
            // This seems to turn up a bunch of "false positives" in
            // polling, though, which is why we're not doing it.
            //  -- willb, 10/25/2000
            fds[u].events = POLLPRI;
            fds[u].revents = 0;
        }
        //  Add file descriptor for MPI master process
        if ( masterMPIfd != -1 )
        {
            any_active_procs = true;
            fds[processVec.size()].fd = masterMPIfd;
            fds[processVec.size()].events = POLLPRI;
            fds[processVec.size()].revents = 0;
        }

        if (!any_active_procs) {
            // Avoid blocking on nothing
            return false;
        }
        
        int timeout;
        if (block) timeout = -1;
        else timeout = 0;
        selected_fds = poll(fds, processVec.size(), timeout);
        
        if (selected_fds <= 0) {
            if (selected_fds < 0) {
                bperr( "decodeProcessEvent: poll failed: %s\n", sys_errlist[errno]);
                selected_fds = 0;
            }
            return false;
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
    prstatus_t procstatus;
    process *currProcess = processVec[curr];
    
    if (fds[curr].revents & POLLHUP) {
        // True if the process exited out from under us
        int status;
        int ret;
        if (fds[curr].fd == masterMPIfd) {
            close(fds[curr].fd);
            masterMPIfd = -1;
        }
        else {
            // Process exited, get its return status
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
            if (!decodeWaitPidStatus(currProcess, status, &why, &what)) {
                cerr << "decodeProcessEvent: failed to decode waitpid return" << endl;
                return false;
            }
        }
    } else {
        // Real return from poll
        if (ioctl(currProcess->getRepresentativeLWP()->get_fd(), 
                  PIOCSTATUS, 
                  &procstatus) != -1) {
            // Check if the process is stopped waiting for us
            if (procstatus.pr_flags & PR_STOPPED ||
                procstatus.pr_flags & PR_ISTOP) {
                if (!decodeProcStatus(currProcess, procstatus, why, what,info))
                {
                   return false;
                }
            }
        }
        else {
            // get_status failed, probably because the process doesn't exist
        }
    }

    if (currProcess) {
        procevent *new_event = new procevent;
        new_event->proc = currProcess;
        new_event->lwp  = currProcess->getRepresentativeLWP();
        new_event->why  = why;
        new_event->what = what;
        new_event->info = info;
        (*events).push_back(new_event);

        // Got a signal, process is stopped.
        currProcess->set_status(stopped);
    }

    // Skip this FD the next time through
    --selected_fds;
    ++curr;    
    return true;
    
} 

// TODO: this ignores the "sig" argument
bool dyn_lwp::continueLWP_(int signalToContinueWith)
{
   ptraceOps++; 
   ptraceOtherOps++;

   prstatus_t stat;
   if (ioctl(get_fd(), PIOCSTATUS, &stat) == -1) {
      perror("dyn_lwp::continueProc_(PIOCSTATUS)");
      return false;
   }
   
   if (!(stat.pr_flags & (PR_STOPPED | PR_ISTOP))) {
      // not stopped
      bperr( "continueProc_(): process not stopped\n");
      print_proc_flags(get_fd());
      return false;
   }
  
   void *arg3;
   prrun_t run;
   if(signalToContinueWith == dyn_lwp::NoSignal) {
      run.pr_flags = PRCSIG; // clear current signal
      arg3 = &run;
   }
   else arg3 = NULL;

   if (ioctl(get_fd(), PIOCRUN, arg3) == -1) {
      perror("dyn_lwp::continueProc_(PIOCRUN)");
      return false;
   }
   
   return true;
}

bool process::heapIsOk(const pdvector<sym_data>&findUs)
{
  if (!(mainFunction = findOnlyOneFunction("main")) &&
      !(mainFunction = findOnlyOneFunction("_main"))) {
    bperr( "process::heapIsOk(): failed to find \"main\"\n");
    return false;
  }

  for (unsigned i = 0; i < findUs.size(); i++) {
    const pdstring &name = findUs[i].name;
    /*
    Address addr = lookup_fn(this, name);
    if (!addr && findUs[i].must_find) {
      bperr( "process::heapIsOk(): failed to find \"%s\"\n", name.c_str());
      return false;
    }
    */
  }


  return true;
}

bool dyn_lwp::executingSystemCall()
{
   bool ret = false;
   prstatus stat;
   if (ioctl(fd_, PIOCSTATUS, &stat) == -1) {
       perror("process::executingSystemCall(PIOCSTATUS)");
       assert(0);
   }
   if (stat.pr_syscall > 0 && 
       stat.pr_why != PR_SYSEXIT) {
       inferiorrpc_cerr << "pr_syscall=" << stat.pr_syscall << endl;
       ret = true;
   }

   return ret;
}

Address dyn_lwp::readRegister(Register retval_reg)
{
  gregset_t regs;

  assert(retval_reg < NGREG);

  if (ioctl (fd_, PIOCGREG, &regs) == -1) {
    perror("process::_inferiorRPC_result_registerread(PIOCGREG)");
    return 0;
  }
  return regs[retval_reg];
}

static const Address lowest_addr = 0x00400000;
void process::inferiorMallocConstraints(Address near, Address &lo, Address &hi,
					inferiorHeapType type)
{
  if (near)
    {
      lo = region_lo(near);
      hi = region_hi(near);  
      // avoid mapping the zero page
      if (lo < lowest_addr) lo = lowest_addr;
    }
}

void process::inferiorMallocAlign(unsigned &size)
{
  // quadword-aligned (stack alignment)
  unsigned align = 16;
  if (size % align) size = ((size/align)+1)*align;
}

bool dyn_lwp::stop_()
{
  //bperr( ">>> process::pause_()\n");
  ptraceOps++; 
  ptraceOtherOps++;

  int ret;
  if ((ret = ioctl(get_fd(), PIOCSTOP, 0)) == -1) {
    perror("process::pause_(PIOCSTOP)");
    sprintf(errorLine, "warning: PIOCSTOP failed in \"pause_\", errno=%i\n", errno);
    logLine(errorLine);
  }

  return (ret != -1);
}


bool dyn_lwp::waitUntilStopped() {
   return true;
}

bool process::waitUntilStopped() {
   return true;
}

int getNumberOfCPUs() 
{
  // see sysmp(2) man page
  int ret = sysmp(MP_NPROCS);
  //bperr( ">>> getNumberOfCPUs(%i)\n", ret);
  return ret;
}


syscallTrap *process::trapSyscallExitInternal(Address syscall) {
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
        trappedSyscall = new syscallTrap;
        trappedSyscall->refcount = 1;
        trappedSyscall->syscall_id = (int) syscall;
        sysset_t save_exitset;
        dyn_lwp *replwp = getRepresentativeLWP();
        if (-1 == ioctl(replwp->get_fd(), PIOCGEXIT, &save_exitset))
            return NULL;

        if (prismember(&save_exitset, trappedSyscall->syscall_id))
            trappedSyscall->orig_setting = 1;
        else
            trappedSyscall->orig_setting = 0;
    
        praddset(&save_exitset, trappedSyscall->syscall_id);
        
        if (-1 == ioctl(replwp->get_fd(), PIOCSEXIT, &save_exitset))
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
    dyn_lwp *replwp = getRepresentativeLWP();
    sysset_t save_exitset;
    if (-1 == ioctl(replwp->get_fd(), PIOCGEXIT, &save_exitset))
        return false;
    
    if (trappedSyscall->orig_setting == 0)
        prdelset(&save_exitset, trappedSyscall->syscall_id);
    
    if (-1 == ioctl(replwp->get_fd(), PIOCSEXIT, &save_exitset))
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
//            syscallTraps_.removeByIndex(iter);
	    syscallTraps_.erase( iter, iter );
            break;
        }
    }
    */
    delete trappedSyscall;
    
    return true;
}

Address dyn_lwp::getCurrentSyscall() {
    
    prstatus theStatus;
    if (ioctl(fd_, PIOCSTATUS, &theStatus) == -1) {
        return 0;
    }
    return theStatus.pr_syscall;
    
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
    Address syscall = theStatus.pr_what;
    
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

bool process::dumpCore_(const pdstring coreFile)
{
  //bperr( ">>> process::dumpCore_()\n");
  bool ret;
#ifdef BPATCH_LIBRARY
  ret = dumpImage(coreFile);
#else
  ret = dumpImage();
#endif
  return ret;
}

dyn_lwp *process::createRepresentativeLWP() {
   // don't register the representativeLWP in real_lwps since it's not a true
   // lwp
   representativeLWP = createFictionalLWP(0);
   return representativeLWP;
}

bool process::terminateProc_()
{
   // these next two lines are a hack used to get the poll call initiated
   // by checkForAndHandleProcessEvents() in process::terminateProc to
   // still check process for events if it was previously stopped
   if(status() == stopped)
      status_ = running;

   kill(getPid(), 9);

   return true;
}

pdstring process::tryToFindExecutable(const pdstring &progpath, int /*pid*/)
{
  //bperr( ">>> process::tryToFindExecutable(%s)\n", progpath.c_str());
  pdstring ret = "";
  
  // attempt #1: expand_tilde_pathname()
  ret = expand_tilde_pathname(progpath);
  //bperr( "  expand_tilde => \"%s\"\n", ret.c_str());
  if (exists_executable(ret)) return ret;
  
  // TODO: any other way to find executable?
  // no procfs info available (argv, cwd, env) so we're stuck
  return "";
}



// HERE BE DRAGONS



// TODO: this is a lousy implementation
#ifdef BPATCH_LIBRARY
bool process::dumpImage(pdstring outFile) {
#else
bool process::dumpImage() {
  char buf[512];
  sprintf(buf, "image.%d", pid);
  pdstring outFile = buf;
#endif
  //bperr( "!!! process::dumpImage(%s)\n", outFile.c_str());
  
  // copy and open file
  image *img = getImage();
  pdstring imgFile = img->file();
  char buf1[1024];
  sprintf(buf1, "cp %s %s", imgFile.c_str(), outFile.c_str());
  system(buf1);
  int fd = open(outFile.c_str(), O_RDWR, 0);
  if (fd < 0) return false;

  // overwrite ".text" section with runtime contents

  bool is_elf64 = img->getObject().is_elf64();
  Elf *elfp = elf_begin(fd, ELF_C_READ, 0);
  assert(elfp);
  int e_shstrndx;
  if (is_elf64) {

    Elf64_Ehdr *ehdrp = elf64_getehdr(elfp);
    assert(ehdrp);
    e_shstrndx = ehdrp->e_shstrndx;

  } else { // 32-bit

    Elf32_Ehdr *ehdrp = elf32_getehdr(elfp);
    assert(ehdrp);
    e_shstrndx = ehdrp->e_shstrndx;

  }
  Elf_Scn *shstrscnp = elf_getscn(elfp, e_shstrndx);
  assert(shstrscnp);
  Elf_Data *shstrdatap = elf_getdata(shstrscnp, 0);
  assert(shstrdatap);
  char *shnames = (char *)shstrdatap->d_buf;

  Address txtAddr = 0;
  int txtLen = 0;
  int txtOff = 0;
  Elf_Scn *scn = 0;
  while ((scn = elf_nextscn(elfp, scn)) != 0) {
    pdElfShdr pd_shdr(scn, is_elf64);
    char *name = (char *)&shnames[pd_shdr.pd_name];
    if (strcmp(name, ".text") == 0) {
      txtOff = pd_shdr.pd_offset;
      txtLen = pd_shdr.pd_size;
      txtAddr = pd_shdr.pd_addr;
      break;
    }
  }
   
  if (txtLen > 0) {
    char *buf2 = new char[txtLen];
    // TODO: readTextSpace_() only defined for BPATCH_MUTATIONS_ACTIVE
    if (!(readDataSpace((void *)txtAddr, txtLen, buf2, false))) {
      delete [] buf2;
      return false;
    }
    lseek(fd, txtOff, SEEK_SET);
    write(fd, buf2, txtLen);
    close(fd);
    delete [] buf2;
  }
  
  return true;
}

#if 0 
// Make emacs happy...
}
#endif

/* 
 * Syscall tracing wrappers
 */
bool process::get_entry_syscalls(sysset_t *entries) {
    dyn_lwp *replwp = getRepresentativeLWP();    
    if (ioctl(replwp->get_fd(), PIOCGENTRY, entries) < 0) {
        perror("get_entry_syscalls");
        return false;
    }
    return true;
}

bool process::set_entry_syscalls(sysset_t *entries) {
    dyn_lwp *replwp = getRepresentativeLWP();    
    if (ioctl(replwp->get_fd(), PIOCSENTRY, entries) < 0) {
        perror("set_entry_syscalls");
        return false;
    }
    return true;
}

bool process::get_exit_syscalls(sysset_t *exits) {
    dyn_lwp *replwp = getRepresentativeLWP();    
    if (ioctl(replwp->get_fd(), PIOCGEXIT, exits) < 0) {
        perror("get_exit_syscalls");
        return false;
    }
    return true;
}

bool process::set_exit_syscalls(sysset_t *exits) {
    dyn_lwp *replwp = getRepresentativeLWP();
    if (ioctl(replwp->get_fd(), PIOCSEXIT, exits) < 0) {
        perror("set_exit_syscalls");
        return false;
    }
    return true;
}

/*
 * Use by dyninst to set events we care about from procfs
 *
 */

bool process::setProcessFlags()
{
    
    long flags = PR_FORK;

    dyn_lwp *replwp = getRepresentativeLWP();    
    if (ioctl(replwp->get_fd(), PIOCSET, &flags) < 0) {
        bperr( "attach: PIOCSET failed: %s\n", sys_errlist[errno]);
        return false;
    }
    
    sigset_t sigs;
    premptyset(&sigs);
    (void)praddset(&sigs, SIGSTOP);
    (void)praddset(&sigs, SIGTRAP);
    (void)praddset(&sigs, SIGILL);
#if defined(bug_irix_broken_sigstop)
    // we need to use SIGEMT for breakpoints in IRIX due to a bug with
    // tracing SIGSTOP in a process and then waitpid()ing for it
    // --wcb 10/4/2000
    (void)praddset(&sigs, SIGEMT);
#endif
    if (ioctl(replwp->get_fd(), PIOCSTRACE, &sigs) < 0) {
        perror("process::attach(PIOCSTRACE)");
        return false;
    }
    
    return true;
}

bool process::unsetProcessFlags() 
{
    bool ret = true;
// signal handling
    sigset_t sigs;
    premptyset(&sigs);
    dyn_lwp *replwp = getRepresentativeLWP();
    if (ioctl(replwp->get_fd(), PIOCSTRACE, &sigs) == -1) {
        perror("process::unsetProcessFlags(PIOCSTRACE)");
        ret = false;
    }
    long flags = PR_FORK;
    if (ioctl(replwp->get_fd(), PIOCRESET, &flags) == -1) {
        perror("process::unsetProcessFlags(PIOCRESET)");
        ret = false;
    }
    return ret;
}


// getActiveFrame(): populate Frame object using toplevel frame
Frame dyn_lwp::getActiveFrame()
{
  Address pc = 0, fp = 0, sp = 0;
  // Get current register values
  gregset_t regs;
  if (ioctl(fd_, PIOCGREG, &regs) == -1) {
    perror("Frame::Frame(PIOCGREG)");
    return Frame();
  }
  
  pc = regs[PROC_REG_PC];
  sp = regs[PROC_REG_SP];
  fp = regs[PROC_REG_FP];

  // sometimes the actual $fp is zero
  // (kludge for stack walk code)
  if (fp == 0) fp = sp;

  return Frame(pc, fp, sp, proc_->getPid(), NULL, this, true);

}
 
static bool basetrampRegSaved(Address pc, Register reg,
                              const instPoint *ip,
                              trampTemplate *bt,
                              miniTrampHandle *mt)
{
  if (!ip) return false;
  if (mt) return true;
  assert(bt);

  Address save_off;
  switch(reg) {
  case REG_RA:
    save_off = bt_ra_save_off;
    break;
  case REG_S8:
    save_off = bt_fp_save_off;
    break;
  default:
    assert(0);
  }

  if (pc >  bt->baseAddr + bt->savePreInsOffset + save_off && 
      pc <= bt->baseAddr + bt->restorePreInsOffset) {
    return true;
  }
  if (pc >  bt->baseAddr + bt->savePostInsOffset + save_off && 
      pc <= bt->baseAddr + bt->restorePostInsOffset) {
    return true;
  }

  return false;
}

// TODO: need dataflow, ($pc < saveInsn) insufficient
Frame Frame::getCallerFrame(process *p) const
{
  // check for active instrumentation
  // (i.e. $pc in native/basetramp/minitramp code)
  
    codeRange *range = p->findCodeRangeByAddress(pc_);
    if (!range) {
        // We have no idea where we are....
        return Frame();
    }
    
    trampTemplate *bt = range->is_basetramp();
    miniTrampHandle  *mt = range->is_minitramp();
    const instPoint     *ip = NULL;
    if (mt) bt = mt->baseTramp;
    if (bt) ip = bt->location;
    pd_Function *callee = range->is_pd_Function();
    Address pc_off;
    if (!callee && ip)
        callee = (pd_Function *) ip->pointFunc();

    // calculate runtime address of callee fn
    if (!callee) {
        bperr( "!!! <0x%016lx:???> unknown callee\n", pc_);
        return Frame(); // zero frame
    }

    if (ip) {
        pc_off = ip->pointAddr() - callee->getEffectiveAddress(p);
    }
    else {
        pc_off = pc_ - callee->getEffectiveAddress(p);
    }
    
    // frame pointers for native and basetramp frames
    Address fp_bt = sp_;
    if (bt) {
        fp_bt += bt_frame_size;
    }
    Address fp_native = fp_bt;

    if (!bt) {
        fp_native += callee->frame_size;
    }
    // override calculated $fp if frame pointer conventions used
    if (callee->uses_fp) fp_native = saved_fp;
    
    // which frames is $ra saved in?
    pd_Function::regSave_t &ra_save = callee->reg_saves[REG_RA];
    bool ra_saved_native = (ra_save.slot != -1 && pc_off > ra_save.insn);
    bool ra_saved_bt = basetrampRegSaved(pc_, REG_RA, ip, bt, mt);
    
    // which frames is $fp saved in?
    pd_Function::regSave_t &fp_save = callee->reg_saves[REG_S8];
    bool fp_saved_native = (fp_save.slot != -1 && pc_off > fp_save.insn);
    bool fp_saved_bt = basetrampRegSaved(pc_, REG_S8, ip, bt, mt);
    
    
    // find caller $pc (callee $ra)
    Address ra;
    Address ra_addr = 0;
    char ra_debug[256];
    sprintf(ra_debug, "<unknown>");
    
    if (!bt && ra_saved_native) {
        // $ra saved in native frame
        ra_addr = fp_native + ra_save.slot;
        ra = readAddressInMemory(p, ra_addr, ra_save.dword);
        sprintf(ra_debug, "[$fp - %i]", -ra_save.slot);
    } else if (bt && ra_saved_bt) {
        // $ra saved in basetramp frame
        ra_addr = fp_bt + bt_ra_slot;
        ra = readAddressInMemory(p, ra_addr, true);
        sprintf(ra_debug, "[$fp - %i]", -bt_ra_slot);

	// The basetramp's caller might have set up a stack frame.
	// Attempt to detect, and remove it if necessary.
	//
	// NOTE: This will not work for tracing through instrumented
	// leaf functions.  More complex code analysis would be needed.
        if (ra_saved_native) {
            Address fp_tmp = fp_bt + callee->frame_size;
            Address ra_addr_tmp = fp_tmp + ra_save.slot;
            Address ra_tmp = readAddressInMemory(p, ra_addr_tmp, true);

            if (ra_tmp == ra) {
                // Stack frame for caller was active.
                // Remove it from consideration.
                fp_native += callee->frame_size;
            }
        }

    } else {
        // $ra not saved in any frame
        // try to read $ra from registers (toplevel only)
        if (uppermost_) {
            // $ra in live register
            gregset_t regs;
            unsigned fd;
            if (lwp_)
                fd = lwp_->get_fd();
            else
                fd = p->getRepresentativeLWP()->get_fd();
            if (ioctl(fd, PIOCGREG, &regs) == -1) {
                perror("process::readDataFromFrame(PIOCGREG)");
                return Frame(); // zero frame
            }
            ra = regs[PROC_REG_RA];
            sprintf(ra_debug, "regs[ra]");
        } else {
            /*
              // debug
              if (callee->prettyName() != "main" &&
              callee->prettyName() != "__start")
              bperr( "!!! <0x%016lx:\"%s\"> $ra not saved\n",
              pc_adj, callee->prettyName().c_str());
            */
            // $ra cannot be found (error)
            return Frame(); // zero frame
        }
    }
    
    // determine location of caller $pc (native code, basetramp, minitramp)
    instPoint *ip2 = NULL;
    trampTemplate *bt2 = NULL;
    miniTrampHandle *mt2 = NULL;
    pd_Function *caller = NULL;
    range = p->findCodeRangeByAddress(ra);
    if (range) {
        mt2 = range->is_minitramp();
        bt2 = range->is_basetramp();
        caller = range->is_pd_Function();
    }
    if (mt2) bt2 = mt2->baseTramp;
    if (bt2) ip2 = (instPoint *) bt2->location;
    if (!caller && ip2)
        caller = (pd_Function *)ip2->pointFunc();
    
    // Check for saved $fp value
    Address fp2;
    Address fp_addr = 0;
    char fp_debug[256];
    sprintf(fp_debug, "<unknown>");
    if (!bt && fp_saved_native) {
        // $fp saved in native frame
        fp_addr = fp_native + fp_save.slot;
        fp2 = readAddressInMemory(p, fp_addr, fp_save.dword);
        sprintf(fp_debug, "[$fp - %i]", -fp_save.slot);
        //bperr( "  read fp_saved_native at %x from fp_native %x and slot %d\n", fp_addr, fp_native, fp_save.slot);
    } else if (bt && fp_saved_bt) {
        // $ra saved in basetramp frame
        fp_addr = fp_bt + bt_fp_slot;
        fp2 = readAddressInMemory(p, fp_addr, true);
        sprintf(fp_debug, "[$fp - %i]", -bt_fp_slot);
        //bperr( "  read fp_saved_bt at %x from fp_native %x and slot %d\n", fp_addr, fp_bt, bt_fp_slot);
    } else {
        // $fp not saved in any frame
        // pass up callee $fp
        fp2 = saved_fp;
        sprintf(fp_debug, "(callee $fp)");
    }
  // sometimes the retrieved $fp is zero
  // (kludge for stack walk code)
  if (fp2 == 0) fp2 = saved_fp;
  
  // Sometimes we have identified functions with a stack frame
  // ra slot that do not store the ra in the slot.  The following
  // code accounts for this situation.
  if (caller == NULL && uppermost_) {
      // $ra in live register
      gregset_t regs;
      unsigned fd;
      if (lwp_) fd = lwp_->get_fd();
      else fd = p->getRepresentativeLWP()->get_fd();
      if (ioctl(fd, PIOCGREG, &regs) == -1) {
          perror("process::readDataFromFrame(PIOCGREG)");
          return Frame(); // zero frame
      }
      ra = regs[PROC_REG_RA];
      caller = p->findFuncByAddr(ra);
  }
  
  // caller frame is invalid if $pc does not resolve to a function
  if (!caller) return Frame(); // zero frame

  // return value
  Frame ret(0, 0, 0, pid_, thread_, lwp_, false);

  // I've gotten the strangest segfaults doing direct assignment
  memcpy(&ret.pc_, &ra, sizeof(Address));
  memcpy(&ret.sp_, &fp_native, sizeof(Address));

  if ( caller )
  {
      if ( caller->uses_fp )
	memcpy(&ret.fp_, &fp2, sizeof(Address));
      else {
	fp_native += caller->frame_size;
	memcpy(&ret.fp_, &fp_native, sizeof(Address));
      }
  }
  else 
    memcpy(&ret.fp_, &fp2, sizeof(Address));
  
  ret.saved_fp = fp2;
  
  return ret;
}


//
// paradynd-only methods
//


void OS::osDisconnect(void) {
  //bperr( ">>> osDisconnect()\n");
  int fd = open("/dev/tty", O_RDONLY);
  ioctl(fd, TIOCNOTTY, NULL); 
  P_close(fd);
}

#if !defined(BPATCH_LIBRARY)

rawTime64 dyn_lwp::getRawCpuTime_hw()
{
  rawTime64 ret = 0;
  hwperf_cntr_t cnts;
  
  if(ioctl(fd_, PIOCGETEVCTRS, (void *)&cnts) < 0) {
    return hw_previous_;
  }
  
  ret = cnts.hwp_evctr[0];
  if(ret < hw_previous_) {
    logLine("*** time going backwards in paradynd ***\n");
    ret = hw_previous_;
  }
  hw_previous_ = ret;

  return ret;
}

/* return unit: nsecs */
rawTime64 dyn_lwp::getRawCpuTime_sw()
{
  //bperr( ">>> getInferiorProcessCPUtime()\n");
  rawTime64 ret;
  
  /*
  pracinfo_t t;
  ioctl(proc_fd, PIOCACINFO, &t);
  ret = PDYN_div1000(t.pr_timers.ac_utime + t.pr_timers.ac_stime);
  */

  timespec_t t[MAX_PROCTIMER];
  if (ioctl(fd_, PIOCGETPTIMER, t) == -1) {
    perror("getInferiorProcessCPUtime - PIOCGETPTIMER");
    return sw_previous_;
  }
  ret = 0;
  ret += t[AS_USR_RUN].tv_sec * I64_C(1000000000); // sec to nsec  (user)
  ret += t[AS_SYS_RUN].tv_sec * I64_C(1000000000); // sec to nsec  (sys)
  ret += t[AS_USR_RUN].tv_nsec;   // add in nsec (user)
  ret += t[AS_SYS_RUN].tv_nsec;   // add in nsec (sys)
  
  // sanity check: time should not go backwards
  if (ret < sw_previous_) {
    logLine("*** time going backwards in paradynd ***\n");
    ret = sw_previous_;
  }
  else {
    sw_previous_ = ret;
  }
  return ret;
}
#endif

//  Here we start the MPI application by fork/exec.
//
//  The application will load libmpi and execute the init section of
//  this library.  Our child becomes the MPI daemon process, which
//  forks the appropriate number of worker processes.
//
//  We attach to the child before exec'ing and set flags
//  to make sure that we can attach to the children of
//  the MPI daemon.
//
//  Attaching to the children is handled in handleSigChild.

bool execIrixMPIProcess(pdvector<pdstring> &argv)
{
  int pipeFlag[2], retval;
  char processFile[64];
  char flag;
	
  if ( pipe(pipeFlag) == -1)
    assert(false);
	
  if ( (masterMPIpid = fork()) )
  {
    // parent
    // attach to child process
    sprintf(processFile, "/proc/%d", masterMPIpid);
    masterMPIfd = open(processFile, O_RDWR);
    irixMPIappName = strdup(argv[0].c_str()); // used to identify MPI app processes
		
    if ( masterMPIfd == -1 )
    {
      perror("startIrixMPIProcess failed to attach to child");
      return(false);
    }

    flag = 'x';
    if ( write(pipeFlag[1], &flag, 1) != 1)
      perror("startIrixMPIProcess:parent pipe flag");
		
    close(pipeFlag[0]);
    close(pipeFlag[1]);
  }
  else
  {
    int proc_fh;
    sysset_t exitCallset;
    premptyset(&exitCallset);
    praddset(&exitCallset, SYS_fork);	

    // child
    // This process becomes the master MPI application process/daemon

    // MPI on IRIX is started using mpirun
    // mpirun forks/execs local copies of the application process
    //   and starts remote processes using the array_services call asremexec().
    // The first copy of the application process on any machine, loads the libmpi.so
    // library and from within the libmpi.so init section,
    // forks the appropriate number of local MPI applications.
    // Note that the original/master application process never leaves
    // the init section (i.e. never reaches main).  Its purpose
    // is to communicate with the controlling mpirun process.

    // make sure parent has attached before proceeding

    int size = read(pipeFlag[0], &flag, 1);
    close(pipeFlag[0]);
    close(pipeFlag[1]);

    if ( size < 0 )
      perror("addIrixMPIprocesses read parent flag");

    //  set trace information so that this process stops at fork exits

    sprintf(processFile, "/proc/%d", getpid());
    proc_fh = open(processFile, O_RDWR);

    retval = ioctl(proc_fh, PIOCSEXIT, &exitCallset);
    if ( retval == -1 )
      perror("PIOCSEXIT");

    close(proc_fh);

    char **args;
    args = new char*[argv.size()+1];
    for (unsigned ai=0; ai<argv.size(); ai++)
      args[ai] = P_strdup(argv[ai].c_str());
    args[argv.size()] = NULL;

    if ( P_execvp(args[0], args) == -1 )
    {
      perror("MPI application exec");
      exit(-1);
    }
  }
	
  return(true);
}

fileDescriptor *getExecFileDescriptor(pdstring filename,
				     int &,
				     bool)
{
  fileDescriptor *desc = new fileDescriptor(filename);
  return desc;
}

bool dyn_lwp::realLWP_attach_() {
   assert( false && "threads not yet supported on IRIX");
   return false;
}

bool dyn_lwp::representativeLWP_attach_() {
   char procName[128];    
   sprintf(procName, "/proc/%05d", getPid());
   fd_ = P_open(procName, O_RDWR, 0);
   if (fd_ == (unsigned) -1) {
      perror("Error opening process file descriptor");
      return false;
   }

   // QUESTION: does this attach operation lead to a SIGTRAP being forwarded
   // to paradynd in all cases?  How about when we are attaching to an
   // already-running process?  (Seems that in the latter case, no SIGTRAP
   // is automatically generated) TODO
   
   // step 1 - /proc open: attach to the inferior process
   // Note: opening /proc is done in the dyn_lwp constructor
   // THIS STEP IS DONE IN THE OS INDEPENDENT FUNCTION process::attach()
   
   // step 2 - /proc PIOCSTRACE: define which signals should be forwarded to
   // daemon these are (1) SIGSTOP and (2) either SIGTRAP (sparc/mips) or
   // SIGILL (x86), to implement inferiorRPC completion detection.
   sigset_t sigs;
   premptyset(&sigs);
   (void)praddset(&sigs, SIGSTOP);
   (void)praddset(&sigs, SIGTRAP);
   (void)praddset(&sigs, SIGILL);
#if defined(bug_irix_broken_sigstop)
   // we need to use SIGEMT for breakpoints in IRIX due to a bug with
   // tracing SIGSTOP in a process and then waitpid()ing for it
   // --wcb 10/4/2000
   (void)praddset(&sigs, SIGEMT);
#endif
   if (ioctl(fd_, PIOCSTRACE, &sigs) < 0) {
      perror("process::attach(PIOCSTRACE)");
      return false;
   }
   return true;
}

void dyn_lwp::realLWP_detach_()
{
   assert(is_attached());  // dyn_lwp::detach() shouldn't call us otherwise
}

void dyn_lwp::representativeLWP_detach_()
{
   assert(is_attached());  // dyn_lwp::detach() shouldn't call us otherwise
   if (fd_) close(fd_);
}

void loadNativeDemangler() {}

Frame dyn_thread::getActiveFrameMT() {
   return Frame();
}  // not used until MT supported


bool process::trapDueToDyninstLib()
{
  Address pc = getRepresentativeLWP()->getActiveFrame().getPC();
  bool ret = (pc == dyninstlib_brk_addr);
  //if (ret) bperr( ">>> process::trapDueToDyninstLib()\n");
  return ret;
}

bool process::trapAtEntryPointOfMain(Address)
{
  Address pc = getRepresentativeLWP()->getActiveFrame().getPC();
  bool ret = (pc == main_brk_addr);
  //if (ret) bperr( ">>> process::trapAtEntryPointOfMain(true)\n");
  return ret;
}

/* insertTrapAtEntryPointOfMain(): For some Fortran apps, main() is
   defined in a shared library.  If main() cannot be found, then we
   check if the executable image contained a call to main().  This is
   usually inside __start(). */
bool process::insertTrapAtEntryPointOfMain()
{
    // insert trap near "main"
    main_brk_addr = lookup_fn(this, "main");
    if (main_brk_addr == 0) {
        main_brk_addr = getImage()->get_main_call_addr();
    }
    if (!main_brk_addr) return false;
    
    // save original instruction
    if (!readDataSpace((void *)main_brk_addr, INSN_SIZE, savedCodeBuffer, true))
        return false;
    
    // insert trap instruction
    instruction trapInsn;
    genTrap(&trapInsn);
    if (!writeDataSpace((void *)main_brk_addr, INSN_SIZE, &trapInsn))
        return false;
    return true;
}

bool process::handleTrapAtEntryPointOfMain()
{
  // restore original instruction to entry point of main()
    if (!writeDataSpace((void *)main_brk_addr, INSN_SIZE, savedCodeBuffer))
        return false;
    return true;
}


bool process::getDyninstRTLibName() {
    // find runtime library
    char *rtlib_var;
    char *rtlib_prefix;
    
    rtlib_var = "DYNINSTAPI_RT_LIB";
    rtlib_prefix = "libdyninstAPI_RT";
    
    if (!dyninstRT_name.length()) {
        dyninstRT_name = getenv(rtlib_var);
        if (!dyninstRT_name.length()) {
            pdstring msg = pdstring("Environment variable ") + pdstring(rtlib_var)
            + pdstring(" has not been defined for process ") + pdstring(pid);
            showErrorCallback(101, msg);
            return false;
        }
    }
    
    const char *rtlib_val = dyninstRT_name.c_str();
    assert(strstr(rtlib_val, rtlib_prefix));
    
    // for 32-bit apps, modify the rtlib environment variable
    char *rtlib_mod = "_n32";
    if (!getImage()->getObject().is_elf64() &&
        !strstr(rtlib_val, rtlib_mod)) 
    {
        char *rtlib_suffix = ".so.1";
        // truncate suffix
        char *ptr_suffix = strstr(rtlib_val, rtlib_suffix);
        assert(ptr_suffix);
        *ptr_suffix = 0;
        // construct environment variable
        char buf[512];
        sprintf(buf, "%s=%s%s%s", rtlib_var, rtlib_val, rtlib_mod, rtlib_suffix);
        dyninstRT_name = pdstring(rtlib_val)+pdstring(rtlib_mod)+pdstring(rtlib_suffix);
    }

    if (access(dyninstRT_name.c_str(), R_OK)) {
        pdstring msg = pdstring("Runtime library ") + dyninstRT_name
        + pdstring(" does not exist or cannot be accessed!");
        showErrorCallback(101, msg);
        assert(0 && "Dyninst RT lib cannot be accessed!");
        return false;
    }
    return true;
}


bool process::loadDYNINSTlib()
{
  //bperr( ">>> loadDYNINSTlib()\n");

  // use "_start" as scratch buffer to invoke dlopen() on DYNINST
  Address baseAddr = lookup_fn(this, "_start");
  char buf_[BYTES_TO_SAVE], *buf = buf_;
  Address bufSize = 0;
  
  // step 0: illegal instruction (code)
  genIll((instruction *)(buf + bufSize));
  bufSize += INSN_SIZE;
  
  // step 1: DYNINST library pdstring (data)
  //Address libStart = bufSize; // debug
  Address libAddr = baseAddr + bufSize;
  if (access(dyninstRT_name.c_str(), R_OK)) {
       pdstring msg = pdstring("Runtime library ") + dyninstRT_name + 
                    pdstring(" does not exist or cannot be accessed");
       showErrorCallback(101, msg);
       return false;
  }
  int libSize = strlen(dyninstRT_name.c_str()) + 1;
  strcpy(buf + bufSize, dyninstRT_name.c_str());
  bufSize += libSize;
  // pad to aligned instruction boundary
  if (!isAligned(baseAddr + bufSize)) {
    int npad = INSN_SIZE - ((baseAddr + bufSize) % INSN_SIZE);
    for (int i = 0; i < npad; i++)
      buf[bufSize + i] = 0;
    bufSize += npad;
    assert(isAligned(baseAddr + bufSize));
  }

  // step 2: inferior dlopen() call (code)
  Address codeAddr = baseAddr + bufSize;
  registerSpace *regs = new registerSpace(nDead, Dead, 0, (Register *)0);
  pdvector<AstNode*> args(2);
  int dlopen_mode = RTLD_NOW | RTLD_GLOBAL;
  AstNode *call;
  pdstring callee = "dlopen";
  // inferior dlopen(): build AstNodes
  args[0] = new AstNode(AstNode::Constant, (void *)libAddr);
  args[1] = new AstNode(AstNode::Constant, (void *)dlopen_mode);
  call = new AstNode(callee, args);
  removeAst(args[0]);
  removeAst(args[1]);
  // inferior dlopen(): generate code
  regs->resetSpace();
  //Address codeStart = bufSize; // debug
  call->generateCode(this, regs, buf, bufSize, true, true);
  removeAst(call);
  
  // step 3: trap instruction (code)
  Address trapAddr = baseAddr + bufSize;
  genTrap((instruction *)(buf + bufSize));
  bufSize += INSN_SIZE;
  //int trapEnd = bufSize; // debug

  // debug
  /*
  bperr( "inferior dlopen code: (%i bytes)\n", bufSize);
  dis(buf, baseAddr);
  bperr( "%0#10x      \"%s\"\n", baseAddr + libStart, buf + libStart);
  for (int i = codeStart; i < bufSize; i += INSN_SIZE) {
    dis(buf + i, baseAddr + i);
  }
  */
  
  // save registers and "_start" code
  readDataSpace((void *)baseAddr, BYTES_TO_SAVE, savedCodeBuffer, true);
  assert(savedRegs == NULL);
  savedRegs = new dyn_saved_regs;
  bool status = getRepresentativeLWP()->getRegisters(savedRegs);
  assert(status == true);

  // write inferior dlopen code and set PC
  assert(bufSize <= BYTES_TO_SAVE);
  //bperr( "writing %i bytes to <0x%08x:_start>, $pc = 0x%08x\n",
  //bufSize, baseAddr, codeAddr);
  //bperr( ">>> loadDYNINSTlib <0x%08x(_start): %i insns>\n",
  //baseAddr, bufSize/INSN_SIZE);
  writeDataSpace((void *)baseAddr, bufSize, (void *)buf);
  bool ret = getRepresentativeLWP()->changePC(codeAddr, savedRegs);
  assert(ret);

  // debug
  /*
  bperr( "inferior dlopen code: (%i bytes)\n", bufSize - codeStart);
  disDataSpace(this, (void *)(baseAddr + codeStart), 
	       (bufSize - codeStart) / INSN_SIZE, "  ");
  */

  dyninstlib_brk_addr = trapAddr;
  setBootstrapState(loadingRT);

  return true;
}

