/*
 * Copyright (c) 1998 Barton P. Miller
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

// $Id: irix.C,v 1.21 2000/11/15 22:56:07 bernat Exp $

#include <sys/types.h>    // procfs
#include <sys/signal.h>   // procfs
#include <sys/fault.h>    // procfs
#include <sys/syscall.h>  // procfs
#include <sys/procfs.h>   // procfs
#include <unistd.h>       // getpid()
#include <sys/ucontext.h> // gregset_t
#include "dyninstAPI/src/arch-mips.h"
#include "dyninstAPI/src/inst-mips.h"
#include "dyninstAPI/src/symtab.h" // pd_Function
#include "dyninstAPI/src/instPoint-mips.h"
#include "dyninstAPI/src/instP.h"
#include "dyninstAPI/src/process.h"
#include "dyninstAPI/src/stats.h" // ptrace{Ops,Bytes}
#include "common/h/pathName.h" // expand_tilde_pathname, exists_executable
#include "common/h/irixKludges.h" // PDYN_XXX
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

extern debug_ostream inferiorrpc_cerr;
extern char *Bool[];
#ifndef BPATCH_LIBRARY
extern string osName;
#endif

char* irixMPIappName;
bool attachToIrixMPIprocess(const string &progpath, int pid, int afterAttach);
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

static Address adjustedPC(Address pc, Address fn_addr,
			  instPoint *ip,
			  trampTemplate *bt,
			  instInstance *mt);
static bool nativFrameActive(instPoint* ip, Address pc_off,
                             pd_Function *callee, process* p);
static bool instrFrameActive(Address pc,
			     instPoint *ip, 
			     trampTemplate *bt, 
			     instInstance *mt);

void print_proc_flags(int fd)
{
  prstatus stat;
  ioctl(fd, PIOCSTATUS, &stat);
  fprintf(stderr, "flags: ");

  if (stat.pr_flags & PR_STOPPED) fprintf(stderr, "PR_STOPPED ");
  if (stat.pr_flags & PR_ISTOP) fprintf(stderr, "PR_ISTOP ");
  if (stat.pr_flags & PR_DSTOP) fprintf(stderr, "PR_DSTOP ");
  if (stat.pr_flags & PR_STEP) fprintf(stderr, "PR_STEP ");
  if (stat.pr_flags & PR_ASLEEP) fprintf(stderr, "PR_ASLEEP ");
  if (stat.pr_flags & PR_PCINVAL) fprintf(stderr, "PR_PCINVAL ");
  //if (stat.pr_flags & PR_ISSYS) fprintf(stderr, "PR_ISSYS ");
  if (stat.pr_flags & PR_FORK) fprintf(stderr, "PR_FORK ");
  if (stat.pr_flags & PR_RLC) fprintf(stderr, "PR_RLC ");
  if (stat.pr_flags & PR_KLC) fprintf(stderr, "PR_KLC ");
  if (stat.pr_flags & PR_PTRACE) fprintf(stderr, "PR_PTRACE ");

  if (stat.pr_flags & PR_ISKTHREAD) fprintf(stderr, "PR_ISKTHREAD ");
  if (stat.pr_flags & PR_JOINTSTOP) fprintf(stderr, "PR_JOINTSTOP ");
  if (stat.pr_flags & PR_JOINTPOLL) fprintf(stderr, "PR_JOINTPOLL ");
  if (stat.pr_flags & PR_RETURN) fprintf(stderr, "PR_RETURN ");
  if (stat.pr_flags & PR_CKF) fprintf(stderr, "PR_CKF ");

  fprintf(stderr, "\n");
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
    fprintf(stderr, "%5s: %#10x %s", buf, (unsigned)regs[i],
	    (i % 4 == 3) ? ("\n") : (","));
  }
  fprintf(stderr, "%5s: %#10x\n", "$pc", (unsigned)regs[CTX_EPC]);
}

void print_proc_pc(int fd)
{
  gregset_t regs;
  if (ioctl (fd, PIOCGREG, &regs) == -1) {
    perror("ioctl(PIOCGREG)");
    return;
  }
  fprintf(stderr, "%5s: %#10x\n", "$pc", (unsigned)regs[CTX_EPC]);
}

bool process::readDataSpace_(const void *inTraced, u_int nbytes, void *inSelf)
{
  //fprintf(stderr, ">>> process::readDataSpace_(%d@0x%016lx)\n", 
  //        nbytes, inTraced);
  ptraceOps++; 
  ptraceBytes += nbytes;

  if(lseek(proc_fd, (off_t)inTraced, SEEK_SET) == -1) {
    perror("process::readDataSpace_(lseek)");
    return false;
  }

  // TODO: check for infinite loop if read returns 0?
  char *dst = (char *)inSelf;
  for (int last, left = nbytes; left > 0; left -= last) {
    if ((last = read(proc_fd, dst + nbytes - left, left)) == -1) {
      perror("process::readDataSpace_(read)");
      return false;
    } else if (last == 0) {
      fprintf(stderr,"process::readDataSpace_(read=%d@0x%016lx,left=%d/%d\n",
              last,inTraced,left,nbytes);
      return false;
    }
  }
  return true;
}

bool process::writeDataSpace_(void *inTraced, u_int nbytes, const void *inSelf)
{
  //fprintf(stderr, ">>> process::writeDataSpace_(%d@0x%016lx)\n", 
  //        nbytes, inTraced);
  ptraceOps++; 
  ptraceBytes += nbytes;

  if(lseek(proc_fd, (off_t)inTraced, SEEK_SET) == -1) {
    perror("process::writeDataSpace_(lseek)");
    return false;
  }

  // TODO: check for infinite loop if write returns 0?
 char *src = (char *)const_cast<void*>(inSelf);
  for (int last, left = nbytes; left > 0; left -= last) {
    if ((last = write(proc_fd, src + nbytes - left, left)) == -1) {
      perror("process::writeDataSpace_(write)");
      return false;
    }
  }
  return true;
}

bool process::writeTextWord_(caddr_t inTraced, int data)
{
  //fprintf(stderr, ">>> process::writeTextWord_()\n");
  return writeDataSpace_(inTraced, INSN_SIZE, &data);
}

bool process::writeTextSpace_(void *inTraced, u_int amount, const void *inSelf)
{
  //fprintf(stderr, ">>> process::writeTextSpace_()\n");
  return writeDataSpace_(inTraced, amount, inSelf);
}

#ifdef BPATCH_SET_MUTATIONS_ACTIVE
bool process::readTextSpace_(void *inTraced, u_int amount, const void *inSelf)
{
  //fprintf(stderr, ">>> process::readTextSpace_()\n");
  return readDataSpace_(inTraced, amount, (void *)inSelf);
}
#endif

void *process::getRegisters() 
{
  //fprintf(stderr, ">>> process::getRegisters()\n");
  assert(status_ == stopped); // process must be stopped (procfs)
  
  gregset_t intRegs;
  if (ioctl(proc_fd, PIOCGREG, &intRegs) == -1) {
    perror("process::getRegisters(PIOCGREG)");
    assert(errno != EBUSY); // procfs thinks the process is active
    return NULL;
  }
  fpregset_t fpRegs;
  if (ioctl(proc_fd, PIOCGFPREG, &fpRegs) == -1) {
    perror("process::getRegisters(PIOCGFPREG)");
    assert(errno != EBUSY);  // procfs thinks the process is active
    assert(errno != EINVAL); // no floating-point hardware
    return NULL;
  }
  
  char *buf = new char[sizeof(intRegs) + sizeof(fpRegs)];
  assert(buf);
  memcpy(buf, &intRegs, sizeof(intRegs));
  memcpy(buf + sizeof(intRegs), &fpRegs, sizeof(fpRegs));
  
  return (void *)buf;
}

bool process::restoreRegisters(void *_buf)
{
  //fprintf(stderr, ">>> process::restoreRegisters()\n");
  assert(status_ == stopped); // process must be stopped (procfs)

  char *buf = (char *)_buf;
  gregset_t *intRegs = (gregset_t *)buf;
  if (ioctl(proc_fd, PIOCSREG, intRegs) == -1) {
    perror("process::restoreRegisters(PIOCSREG)");
    assert(errno != EBUSY); // procfs thinks the process is active
    return false;
  }  
  fpregset_t *fpRegs = (fpregset_t *)(buf + sizeof(gregset_t));
  if (ioctl(proc_fd, PIOCSFPREG, fpRegs) == -1) {
    perror("process::restoreRegisters(PIOCSFPREG)");
    assert(errno != EBUSY);  // procfs thinks the process is active
    assert(errno != EINVAL); // no floating-point hardware
    return false;
  }

  return true;
}

// fetch actual PC value via procfs
// TODO: make "process::" - csserra
Address pcFromProc(int proc_fd)
{
  gregset_t regs;
  if (ioctl (proc_fd, PIOCGREG, &regs) == -1) {
    perror("currentPC(PIOCGREG)");
    assert(errno != EBUSY); // procfs thinks the process is active
    return 0;
  }
  return (Address)regs[PROC_REG_PC];
}

bool process::changePC(Address addr, const void *savedRegs)
{
  //fprintf(stderr, ">>> process::changePC(0x%08x)\n", addr);
  assert(status_ == stopped); // process must be stopped (procfs)

  /* copy integer registers from register buffer */
  gregset_t *savedIntRegs = (gregset_t *)const_cast<void*>(savedRegs); 
  gregset_t intRegs;
  memcpy(&intRegs, savedIntRegs, sizeof(gregset_t));

  intRegs[PROC_REG_PC] = addr; // set PC

  if (ioctl(proc_fd, PIOCSREG, &intRegs) == -1) {
    perror("process::changePC(PIOCSREG)");
    assert(errno != EBUSY); // procfs thinks the process is active
    return false;
  }

   return true;
}

bool process::changePC(Address addr)
{
  //fprintf(stderr, ">>> process::changePC()\n");
  assert(status_ == stopped); // process must be stopped (procfs)

  gregset_t intRegs;
  if (ioctl(proc_fd, PIOCGREG, &intRegs) == -1) {
    perror("process::changePC(PIOCGREG)");
    assert(errno != EBUSY); // procfs thinks the process is active
    return false;
  }

  intRegs[PROC_REG_PC] = addr; // set PC

  if (ioctl(proc_fd, PIOCSREG, &intRegs) == -1) {
    perror("process::changePC(PIOCSREG)");
    assert(errno != EBUSY); // procfs thinks the process is active
    return false;
  }

   return true;
}

bool process::isRunning_() const
{
  //fprintf(stderr, ">>> process::isRunning_()\n");
  prstatus status;
  if (ioctl(proc_fd, PIOCSTATUS, &status) == -1) {
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
  //fprintf(stderr, ">>> OS::osTraceMe(%s)\n", procName);
  int fd = P_open(procName, O_RDWR, 0);
  if (fd < 0) {
    fprintf(stderr, "osTraceMe: open failed: %s\n", sys_errlist[errno]); 
    fflush(stderr);
    P__exit(-1); // must use _exit here.
  }

  // Procfs on Irix sets the "run-on-last-close" flag (PR_RLC) by default.
  // This flag needs to be unset to avoid the PIOCSEXIT trace (below) from
  // being lost when the child closes the proc file descriptor to itself.
  /* reset "run-on-last-close" flag */
  long flags = PR_RLC;
  if (ioctl(fd, PIOCRESET, &flags) < 0) {
    fprintf(stderr, "osTraceMe: PIOCRESET failed: %s\n", sys_errlist[errno]); 
    fflush(stderr);
    P__exit(-1); // must use _exit here.
  };

  /* set a breakpoint at the exit of execv/execve */
  sysset_t exitSet;
  premptyset(&exitSet);
  (void)praddset(&exitSet, SYS_execve);
  if (ioctl(fd, PIOCSEXIT, &exitSet) < 0) {
    fprintf(stderr, "osTraceMe: PIOCSEXIT failed: %s\n", sys_errlist[errno]); 
    fflush(stderr);
    P__exit(-1); // must use _exit here.
  }

  errno = 0;
  close(fd);
}

bool process::attach()
{
  // QUESTION: does this attach operation lead to a SIGTRAP being forwarded
  // to paradynd in all cases?  How about when we are attaching to an
  // already-running process?  (Seems that in the latter case, no SIGTRAP
  // is automatically generated) TODO

  // step 1 - /proc open: attach to the inferior process
  char procName[128];
  sprintf(procName,"/proc/%05d", (int)pid);
  int fd = P_open(procName, O_RDWR, 0);
  if (fd < 0) {
    // TODO: official error msg for API tests
    //perror("process::attach(open)");
    return false;
  }

  // step 2 - /proc PIOCSTRACE: define which signals should be forwarded to daemon
  // these are (1) SIGSTOP and (2) either SIGTRAP (sparc/mips) or SIGILL (x86),
  // to implement inferiorRPC completion detection.
  sigset_t sigs;
  premptyset(&sigs);
  (void)praddset(&sigs, SIGSTOP);
  (void)praddset(&sigs, SIGTRAP);
  (void)praddset(&sigs, SIGILL);
#ifdef USE_IRIX_FIXES
  // we need to use SIGEMT for breakpoints in IRIX due to a bug with
  // tracing SIGSTOP in a process and then waitpid()ing for it
  // --wcb 10/4/2000
  (void)praddset(&sigs, SIGEMT);
#endif
  if (ioctl(fd, PIOCSTRACE, &sigs) < 0) {
    perror("process::attach(PIOCSTRACE)");
    close(fd);
    return false;
  }

  // step 3 - /proc PIOC{SET,RESET}:
  // a) turn on the kill-on-last-close flag (kills inferior with SIGKILL when
  //    the last writable /proc fd closes)
  // b) turn on inherit-on-fork flag (tracing flags inherited when
  // child forks), Paradyn only.
  // c) turn off run-on-last-close flag (on by default)
  // Also, any child of this process will stop at the exit of an exec
  // call, Paradyn only.

  proc_fd = fd;

#ifdef BPATCH_LIBRARY
  setProcfsFlags();
#else
  long flags = 0;

  flags = PR_KLC | PR_FORK;
  if (ioctl (proc_fd, PIOCSET, &flags) < 0) {
    perror("process::attach(PIOCSET)");
    close(proc_fd);
    return false;
  }
  flags = PR_RLC;
  if (ioctl (proc_fd, PIOCRESET, &flags) < 0) {
    perror("process::attach(PIOCRESET)");
    close(proc_fd);
    return false;
  }
#endif

  // environment variables
  prpsinfo_t info;
  if (ioctl(fd, PIOCPSINFO, &info) < 0) {
    perror("process::attach(PIOCPSINFO)");
    close(fd);
    return false;
  }
  // argv[0]
  char *argv0_start = info.pr_psargs;
  char *argv0_end = strstr(argv0_start, " ");
  if (argv0_end) (*argv0_end) = 0;
  argv0 = argv0_start;
  // unused variables
  pathenv = "";
  cwdenv = "";

  return true;
}

/*
   execResult: return the result of an exec system call - true if succesful.
   The traced processes will stop on exit of an exec system call, just before
   returning to user code. At this point the return value (errno) is already
   written to a register, and we need to check if the return value is zero.
 */
static inline bool execResult(prstatus_t stat) {
  return (stat.pr_reg[PROC_REG_RV] == 0);
}

#ifdef BPATCH_LIBRARY
int process::waitProcs(int *status, bool block)
#else
int process::waitProcs(int *status)
#endif
{
  //fprintf(stderr, ">>> process::waitProcs()\n");

  extern vector<process*> processVec;

  static struct pollfd fds[OPEN_MAX];  // argument for poll
  static int selected_fds;             // number of selected
  static int curr;                     // the current element of fds
  prstatus_t stat;

#ifndef BPATCH_LIBRARY
  int processCount;	               // the current number of fds, > processVec.size (IRIX MPI)

  if ( masterMPIfd != -1 )
    processCount = processVec.size() + 1;
  else
    processCount = processVec.size();
#endif
  
#ifdef BPATCH_LIBRARY
  do {
#endif
    
    /* Each call to poll may return many selected fds. Since we only
       report the status of one process per each call to waitProcs, we
       keep the result of the last poll buffered, and simply process an
       element from the buffer until all of the selected fds in the
       last poll have been processed.  
    */
    
    if (selected_fds == 0) {
      for (unsigned i = 0; i < processVec.size(); i++) {
	if (processVec[i] && 
	    (processVec[i]->status() == running || processVec[i]->status() == neonatal))
	  fds[i].fd = processVec[i]->proc_fd;
	else
	  fds[i].fd = -1;
	// IRIX note:  Dave Anderson at SGI seems to think that it is
	// "ill-defined" what band signals show up in.  He suggests
	// that we do something like
	//    fds[i].events = POLLPRI | POLLRDBAND;
	// This seems to turn up a bunch of "false positives" in
	// polling, though, which is why we're not doing it.
	//  -- willb, 10/25/2000
	fds[i].events = POLLPRI;
	fds[i].revents = 0;
      }
      
	  //  Add file descriptor for MPI master process
      if ( masterMPIfd != -1 )
      {
	fds[processVec.size()].fd = masterMPIfd;
	fds[processVec.size()].events = POLLPRI;
	fds[processVec.size()].revents = 0;
      }
	  
#ifdef BPATCH_LIBRARY
      int timeout;
      if (block) timeout = INFTIM;
      else timeout = 0;
      selected_fds = poll(fds, processVec.size(), timeout);
#else
      selected_fds = poll(fds, processCount, 0);
#endif
      if (selected_fds == -1) {
	perror("process::waitProcs(poll)");
	selected_fds = 0;
	return 0;
      }      
      curr = 0;
    }

    int test;
	
    if (selected_fds > 0) {
      while (fds[curr].revents == 0) curr++;
      // fds[curr] has an event of interest
      //fprintf(stderr, ">>> process::waitProcs(fd %i)\n", curr);

      //  check for activity from IRIX MPI master process
      if (fds[curr].fd == masterMPIfd )
      {
	if (fds[curr].revents & POLLHUP) {
	  close(fds[curr].fd);
	  masterMPIfd = -1;
	}
	else if ( (test = ioctl(fds[curr].fd, PIOCSTATUS, &stat)) != -1 
		  && ((stat.pr_flags & PR_STOPPED) || (stat.pr_flags & PR_ISTOP)))
	{
	  // check if master MPI process forked
	  if ( stat.pr_syscall == SYS_fork && stat.pr_rval1 > 0 )
	  {
	    prpsinfo_t info;
	    // check if new process is an MPI app
	    if ( ioctl(fds[curr].fd, PIOCPSINFO, &info) == -1 )
	      perror("ioctl psinfo");
	    else
	    {
	      if ( irixMPIappName != 0 && strncmp(irixMPIappName, info.pr_psargs, strlen(irixMPIappName)) == 0 )
	      {
		char * progName;

		progName = strdup(info.pr_psargs);

		progName = strtok(progName, " ");

		if ( !attachToIrixMPIprocess(progName, stat.pr_rval1, 1) )
		{
		  cerr << "attachProcess failed when attempting to attach to MPI daemon!" << endl;
		  assert(0);
		}
	      }
	      ioctl(fds[curr].fd, PIOCRUN, 0);
	    }
	  }
	}
	else if ( test == -1 )
	  cerr << ">>> proc failed!!!!!!" << endl;
		  
	--selected_fds;
	return 0;
      }

      int ret = 0;
#ifdef BPATCH_LIBRARY
      if (fds[curr].revents & POLLHUP) {
	//fprintf(stderr, ">>> process::waitProcs(fd %i): POLLHUP\n", curr);
	do {
	  ret = waitpid(processVec[curr]->getPid(), status, 0);
	} while ((ret < 0) && (errno == EINTR));
	if (ret < 0) {
	  // This means that the application exited, but was not our child
	  // so it didn't wait around for us to get it's return code.  In
	  // this case, we can't know why it exited or what it's return
	  // code was.
	  ret = processVec[curr]->getPid();
	  *status = 0;
	}
	assert(ret == processVec[curr]->getPid());
      } else
#else
	// IRIX MPI programs are started by paradynd, but the MPI
	// process created by paradyn is a master process (or mpi
	// daemon) that doesn't ever actually enter main() in the
	// application.  It just forks the appropriate number of local
	// processes.  In order to keep track of when the application
	// processes exit, we must interpret POLLHUPs as application
	// exits.

	if ( fds[curr].revents & POLLHUP && process::pdFlavor == "mpi" 
	     && osName.prefixed_by("IRIX") && fds[curr].fd != masterMPIfd )
	{
	  //fprintf(stderr, ">>> process::waitProcs(fd %i): POLLHUP\n", curr);
	  do {
	    ret = waitpid(processVec[curr]->getPid(), status, 0);
	  } while ((ret < 0) && (errno == EINTR));
	  if (ret < 0) {
	    // This means that the application exited, but was not our child
	    // so it didn't wait around for us to get it's return code.  In
	    // this case, we can't know why it exited or what it's return
	    // code was.
	    ret = processVec[curr]->getPid();
	    *status = 0;
	  }

	  sprintf(errorLine, "Process %d has terminated\n", processVec[curr]->getPid());
	  statusLine(errorLine);
	  logLine(errorLine);
		  
	  handleProcessExit(processVec[curr], 0);
	  // In this case, we don't want handleSigChild to examine the process state
	  ret = 0;
	} else
#endif
	if (ioctl(fds[curr].fd, PIOCSTATUS, &stat) != -1 
	    && ((stat.pr_flags & PR_STOPPED) || (stat.pr_flags & PR_ISTOP))) {
	  //print_proc_flags(fds[curr].fd);
	  process *p = processVec[curr];
	  switch (stat.pr_why) {
	  case PR_SIGNALLED: {
	    // debug
	    //fprintf(stderr, ">>> process::waitProcs(fd %i): $pc(0x%08x), sig(%i)\n", 
	    //curr, stat.pr_reg[PROC_REG_PC], stat.pr_what);

	    // return the signal number
	    *status = stat.pr_what << 8 | 0177;
	    ret = p->getPid();
#if defined(USES_LIBDYNINSTRT_SO)
	    if (!p->dyninstLibAlreadyLoaded() && 
		p->wasCreatedViaAttach()) 
	      {
		// make sure we are stopped in the eyes of paradynd - naim
		bool wasRunning = (p->status() == running);
		if (p->status() != stopped) p->Stopped();   

		// check for dlopen() of libdyninstRT.so.1
		if(p->isDynamicallyLinked()) {
		  bool objectWasMapped = p->handleIfDueToSharedObjectMapping();
		  if (p->dyninstLibAlreadyLoaded()) {
		    // libdyninstRT.so.1 was just loaded so cleanup "_start"
		    assert(objectWasMapped);
		    p->handleIfDueToDyninstLib();
		  }
		}
		if (wasRunning) {
		  assert(p->continueProc());
		}
	      }
#endif
	  } break;
	  case PR_SYSEXIT: {
	    //fprintf(stderr, ">>> process::waitProcs(fd %i): PR_SYSEXIT\n", curr);
	    // exit of a system call
	    if (p->RPCs_waiting_for_syscall_to_complete) {
	      // reset PIOCSEXIT mask
	      //inferiorrpc_cerr << "solaris got PR_SYSEXIT!" << endl;
	      assert(p->save_exitset_ptr != NULL);
	      assert(ioctl(p->proc_fd, PIOCSEXIT, p->save_exitset_ptr) != -1);
	      delete [] p->save_exitset_ptr;
	      p->save_exitset_ptr = NULL;
	      // fall through on purpose (so status, ret get set)
	    } else if ((stat.pr_what == SYS_execve) && !execResult(stat)) {
	      // a failed exec; continue the process
	      p->continueProc_();
	      break;
	    }          

#ifdef BPATCH_LIBRARY
	    if (stat.pr_what == SYS_fork) { 
		int i;

	        int childPid = stat.pr_reg[PROC_REG_RV];

		if (childPid > 0)  {
		  for (i=0; i < processVec.size(); i++) {
		     if (processVec[i]->getPid() == childPid) break;
		  }
		  if (i== processVec.size()) {
		     // this is a new child, register it with dyninst
		     int parentPid = processVec[curr]->getPid();
		     process *theParent = processVec[curr];
		     process *theChild = new process(*theParent, (int)childPid, -1);
		     // the parent loaded it!
		     theChild->hasLoadedDyninstLib = true;

		     processVec += theChild;
		     activeProcesses++;

		     // it's really stopped, but we need to mark it running so
		     //   it can report to us that it is stopped - jkh 1/5/00
		     // or should this be exited???
		     theChild->status_ = neonatal;

		     // parent is stopped too (on exit fork event)
		     theParent->status_ = stopped;

		     theChild->execFilePath = 
			 theChild->tryToFindExecutable("", childPid);
		     theChild->inExec = false;
		     BPatch::bpatch->registerForkedThread(parentPid,
			 childPid, theChild);
		  }
		}
	     } else if ((stat.pr_what == SYS_execve) && execResult(stat)) {
		 process *proc = processVec[curr];
		 proc->execFilePath = 
		     proc->tryToFindExecutable(proc->execPathArg, proc->getPid());

	         // only handle if this is in the child - is this right??? jkh
	         if (proc->reachedFirstBreak) {
		     // mark this for the sig TRAP that will occur soon
		     proc->inExec = true;

		     // leave process stopped until signal handler runs
		     // mark it running so we get the stop signal
		     proc->status_ = stopped;

		     // reset buffer pool to empty (exec clears old mappings)
		     vector<heapItem *> emptyHeap;
		     proc->heap.bufferPool = emptyHeap;
	         }
	     } else {
		 printf("got unexpected PIOCSEXIT\n");
		 printf("  call is return from syscall #%d\n", stat.pr_what);
	     }
#endif
	     *status = SIGTRAP << 8 | 0177;
	     ret = p->getPid();
	     break;
	  }

#ifdef BPATCH_LIBRARY
       case PR_SYSENTRY: {
	 bool alreadyCont = false;
	 process *p = processVec[curr];

	 if (stat.pr_what == SYS_fork) {
	     if (BPatch::bpatch->preForkCallback) {
		 assert(p->thread);
		 p->setProcfsFlags();
		 BPatch::bpatch->preForkCallback(p->thread, NULL);
	     }
	 } else if (stat.pr_what == SYS_exit) {
	     // get the parameter to exit to see the exit code
	     int code = stat.pr_reg[REG_A0];

	     process *proc = processVec[curr];

	     proc->status_ = stopped;

	     BPatch::bpatch->registerExit(proc->thread, code);

	     proc->continueProc();
	     alreadyCont = true;
	 } else if (stat.pr_what == SYS_execve) {
	     process *proc = processVec[curr];

	     Address pathStr = stat.pr_reg[REG_A0];
	     char name[512];

	     if (!(proc->readDataSpace_((void *)pathStr, sizeof(name), name))) {
		 logLine("execve entry: unable to read path argument\n");
		 proc->execPathArg = "";
	     } else {
		 proc->execPathArg = name;
	     }

	     proc->continueProc_();
	     alreadyCont = true;
	 } else {
	     printf("got PR_SYSENTRY\n");
	     printf("    unhandeled sys call #%d\n", stat.pr_what);
	 }
	 if (!alreadyCont) (void) processVec[curr]->continueProc_();
	 break;
       }
#endif

	  case PR_REQUESTED:
	    // TODO: this has been reached
	    //fprintf(stderr, ">>> process::waitProcs(fd %i): PR_REQUESTED\n", curr);
	    assert(0);
	  case PR_JOBCONTROL:
	    //fprintf(stderr, ">>> process::waitProcs(fd %i): PR_JOBCONTROL\n", curr);
	    assert(0);
	    break;
	  }        
	}
	else
	{
	  perror("waitProcs:");
	  cerr << "proc failed in waitProcs!" <<  endl;
	}
      
      --selected_fds;
      ++curr;      
      if (ret > 0)
	  return ret;
    }
#ifdef BPATCH_LIBRARY
  } while (block);
  return 0;
#else
  int ret = waitpid(0, status, WNOHANG); 

  if ( ret == masterMPIpid )
    ret = 0;

  return ret;
#endif
}

bool process::detach_()
{
  //fprintf(stderr, ">>> process::detach_()\n");
  if (close(proc_fd) == -1) {
    logLine("process::detach_(close)\n");
    return false;
  }
  return true;
}

bool process::continueProc_()
{
  ptraceOps++; 
  ptraceOtherOps++;

  prstatus_t stat;
  if (ioctl(proc_fd, PIOCSTATUS, &stat) == -1) {
    perror("process::continueProc_(PIOCSTATUS)");
    return false;
  }

  if (!(stat.pr_flags & (PR_STOPPED | PR_ISTOP))) {
    // not stopped
    fprintf(stderr, "continueProc_(): process not stopped\n");
    print_proc_flags(proc_fd);
    return false;
  }
  
  prrun_t run;
  run.pr_flags = PRCSIG; // clear current signal
  if (hasNewPC) {        // new PC value
    hasNewPC = false;
    run.pr_vaddr = (caddr_t)currentPC_;
    run.pr_flags |= PRSVADDR;
  }
  if (ioctl(proc_fd, PIOCRUN, &run) == -1) {
    perror("process::continueProc_(PIOCRUN)");
    return false;
  }

  return true;
}

bool process::heapIsOk(const vector<sym_data>&findUs)
{
  if (!(mainFunction = findOneFunction("main")) &&
      !(mainFunction = findOneFunction("_main"))) {
    fprintf(stderr, "process::heapIsOk(): failed to find \"main\"\n");
    return false;
  }

  for (unsigned i = 0; i < findUs.size(); i++) {
    const string &name = findUs[i].name;
    cerr << "Looking up symbol " << name << endl;
    /*
    Address addr = lookup_fn(this, name);
    if (!addr && findUs[i].must_find) {
      fprintf(stderr, "process::heapIsOk(): failed to find \"%s\"\n", name.string_of());
      return false;
    }
    */
  }


  return true;
}

bool process::executingSystemCall()
{
   bool ret = false;
   prstatus stat;
   if (ioctl(proc_fd, PIOCSTATUS, &stat) == -1) {
     perror("process::executingSystemCall(PIOCSTATUS)");
     assert(0);
   }
   if (stat.pr_syscall > 0) {
     inferiorrpc_cerr << "pr_syscall=" << stat.pr_syscall << endl;
     ret = true;
   }

   return ret;
}

Address process::read_inferiorRPC_result_register(Register retval_reg)
{
  gregset_t regs;

  assert(retval_reg < NGREG);

  if (ioctl (proc_fd, PIOCGREG, &regs) == -1) {
    perror("process::_inferiorRPC_result_registerread(PIOCGREG)");
    return 0;
  }
  return regs[retval_reg];
}

static const Address lowest_addr = 0x00400000;
void inferiorMallocConstraints(Address near, Address &lo, Address &hi,
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

void inferiorMallocAlign(unsigned &size)
{
  // quadword-aligned (stack alignment)
  unsigned align = 16;
  if (size % align) size = ((size/align)+1)*align;
}

bool process::pause_()
{
  //fprintf(stderr, ">>> process::pause_()\n");
  ptraceOps++; 
  ptraceOtherOps++;

  int ret;
  if ((ret = ioctl(proc_fd, PIOCSTOP, 0)) == -1) {
    perror("process::pause_(PIOCSTOP)");
    sprintf(errorLine, "warning: PIOCSTOP failed in \"pause_\", errno=%i\n", errno);
    logLine(errorLine);
  }

  return (ret != -1);
}

int getNumberOfCPUs() 
{
  // see sysmp(2) man page
  int ret = sysmp(MP_NPROCS);
  //fprintf(stderr, ">>> getNumberOfCPUs(%i)\n", ret);
  return ret;
}

bool process::set_breakpoint_for_syscall_completion()
{
  /* Can assume: (1) process is paused and (2) in a system call.  We
     want to set a TRAP for the syscall exit, and do the inferiorRPC
     at that time.  We'll use /proc PIOCSEXIT.  Returns true iff
     breakpoint was successfully set. */
  //fprintf(stderr, ">>> process::set_breakpoint_for_syscall_completion()\n");
  
  sysset_t save_exitset;
  if (ioctl(proc_fd, PIOCGEXIT, &save_exitset) == -1) {
    return false;
  }
  
  sysset_t new_exitset;
  prfillset(&new_exitset);
  if (ioctl(proc_fd, PIOCSEXIT, &new_exitset) == -1) {
    return false;
  }
  
  assert(save_exitset_ptr == NULL);
  save_exitset_ptr = new sysset_t;
  memcpy(save_exitset_ptr, &save_exitset, sizeof(sysset_t));
  
  return true;
}

void process::clear_breakpoint_for_syscall_completion() { return; }

// TODO: this ignores the "sig" argument
bool process::continueWithForwardSignal(int /*sig*/)
{
  fprintf(stderr, ">>> process::continueWithForwardSignal()\n");
  if (ioctl(proc_fd, PIOCRUN, NULL) == -1) {
    perror("process::continueWithForwardSignal(PIOCRUN)\n");
    return false;
  }
  return true;
}

bool process::dumpCore_(const string coreFile)
{
  //fprintf(stderr, ">>> process::dumpCore_()\n");
  bool ret;
#ifdef BPATCH_LIBRARY
  ret = dumpImage(coreFile);
#else
  ret = dumpImage();
#endif
  return ret;
}

#ifdef BPATCH_LIBRARY
bool process::terminateProc_()
{
  //fprintf(stderr, ">>> process::terminateProc_()\n");
  long flags = PR_KLC;
  if (ioctl(proc_fd, PIOCSET, &flags) == -1) {
    // not an error: proc_fd has probably been close()ed already
    return false;
  }  
  Exited();
  return true;
}
#endif

#ifdef BPATCH_LIBRARY
/* API_detach_: detach from the process (clean up /proc state);
   continue process' execution if "cont" is true. */
bool process::API_detach_(const bool cont)
{
  //fprintf(stderr, ">>> process::API_detach_(%s)\n", (cont) ? ("continue") : ("abort"));
  bool ret = true;

  // remove shared object loading traps
  if (dyn) dyn->unsetMappingHooks(this);

  // signal handling
  sigset_t sigs;
  premptyset(&sigs);
  if (ioctl(proc_fd, PIOCSTRACE, &sigs) == -1) {
    //perror("process::API_detach_(PIOCSTRACE)");
    ret = false;
  }
  if (ioctl(proc_fd, PIOCSHOLD, &sigs) == -1) {
    //perror("process::API_detach_(PIOCSHOLD)");
    ret = false;
  }
  // fault handling
  fltset_t faults;
  premptyset(&faults);
  if (ioctl(proc_fd, PIOCSFAULT, &faults) == -1) {
    //perror("process::API_detach_(PIOCSFAULT)");
    ret = false;
  }
  // system call handling
  sysset_t syscalls;
  premptyset(&syscalls);
  if (ioctl(proc_fd, PIOCSENTRY, &syscalls) == -1) {
    //perror("process::API_detach_(PIOCSENTRY)");
    ret = false;
  }
  if (ioctl(proc_fd, PIOCSEXIT, &syscalls) == -1) {
    //perror("process::API_detach_(PIOCSEXIT)");
    ret = false;
  }
  // operation mode
  long flags = PR_RLC | PR_KLC;
  if (ioctl(proc_fd, PIOCRESET, &flags) == -1) {
    //perror("process::API_detach_(PIOCRESET)");
    ret = false;
  }
  flags = (cont) ? (PR_RLC) : (PR_KLC);
  if (ioctl(proc_fd, PIOCSET, &flags) == -1) {
    //perror("process::API_detach_(PIOCSET)");
    ret = false;
  }    

  close(proc_fd);
  return ret;
}
#endif

string process::tryToFindExecutable(const string &progpath, int /*pid*/)
{
  //fprintf(stderr, ">>> process::tryToFindExecutable(%s)\n", progpath.string_of());
  string ret = "";
  
  // attempt #1: expand_tilde_pathname()
  ret = expand_tilde_pathname(progpath);
  //fprintf(stderr, "  expand_tilde => \"%s\"\n", ret.string_of());
  if (exists_executable(ret)) return ret;
  
  // TODO: any other way to find executable?
  // no procfs info available (argv, cwd, env) so we're stuck
  return "";
}



// HERE BE DRAGONS



// TODO: this is a lousy implementation
#ifdef BPATCH_LIBRARY
bool process::dumpImage(string outFile) {
#else
bool process::dumpImage() {
  char buf[512];
  sprintf(buf, "image.%d", pid);
  string outFile = buf;
#endif
  //fprintf(stderr, "!!! process::dumpImage(%s)\n", outFile.string_of());
  
  // copy and open file
  image *img = getImage();
  string imgFile = img->file();
  char buf1[1024];
  sprintf(buf1, "cp %s %s", imgFile.string_of(), outFile.string_of());
  system(buf1);
  int fd = open(outFile.string_of(), O_RDWR, 0);
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
    if (!(readDataSpace_((void *)txtAddr, txtLen, buf2))) {
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

#ifdef BPATCH_LIBRARY
/*
 * Use by dyninst to set events we care about from procfs
 *
 */
bool process::setProcfsFlags()
{

  long flags = PR_KLC | PR_FORK;
  if (BPatch::bpatch->postForkCallback) {
      // cause the child to inherit trap-on-exit from exec and other traps
      // so we can learn of the child (if the user cares)
      flags = PR_FORK | PR_RLC;
  }

  if (ioctl (proc_fd, PIOCSET, &flags) < 0) {
    fprintf(stderr, "attach: PIOCSET failed: %s\n", sys_errlist[errno]);
    close(proc_fd);
    return false;
  }

  // cause a stop on the exit from fork
  sysset_t sysset;

  if (ioctl(proc_fd, PIOCGEXIT, &sysset) < 0) {
    fprintf(stderr, "attach: ioctl failed: %s\n", sys_errlist[errno]);
    close(proc_fd);
    return false;
  }

  if (BPatch::bpatch->postForkCallback) {
      praddset (&sysset, SYS_fork);
      praddset (&sysset, SYS_execve);
  }
  
  if (ioctl(proc_fd, PIOCSEXIT, &sysset) < 0) {
    fprintf(stderr, "attach: ioctl failed: %s\n", sys_errlist[errno]);
    close(proc_fd);
    return false;
  }

  // now worry about entry too
  if (ioctl(proc_fd, PIOCGENTRY, &sysset) < 0) {
    fprintf(stderr, "attach: ioctl failed: %s\n", sys_errlist[errno]);
    close(proc_fd);
    return false;
  }

  if (BPatch::bpatch->exitCallback) {
      praddset (&sysset, SYS_exit);
  }

  if (BPatch::bpatch->preForkCallback) {
      praddset (&sysset, SYS_fork);
  }

  praddset (&sysset, SYS_execve);

  // should these be for exec callback??
  // prdelset (&sysset, SYS_execve);
  
  if (ioctl(proc_fd, PIOCSENTRY, &sysset) < 0) {
    fprintf(stderr, "attach: ioctl failed: %s\n", sys_errlist[errno]);
    close(proc_fd);
    return false;
  }

  sigset_t sigs;
  premptyset(&sigs);
  (void)praddset(&sigs, SIGSTOP);
  (void)praddset(&sigs, SIGTRAP);
  (void)praddset(&sigs, SIGILL);
#ifdef USE_IRIX_FIXES
  // we need to use SIGEMT for breakpoints in IRIX due to a bug with
  // tracing SIGSTOP in a process and then waitpid()ing for it
  // --wcb 10/4/2000
  (void)praddset(&sigs, SIGEMT);
#endif
  if (ioctl(proc_fd, PIOCSTRACE, &sigs) < 0) {
    perror("process::attach(PIOCSTRACE)");
    close(proc_fd);
    return false;
  }

  return true;
}
#endif

// getActiveFrame(): populate Frame object using toplevel frame
void Frame::getActiveFrame(process *p)
{
  // Get current register values
  gregset_t regs;
  int proc_fd = p->getProcFileDescriptor();
  if (ioctl(proc_fd, PIOCGREG, &regs) == -1) {
    perror("Frame::Frame(PIOCGREG)");
    return;
  }
  
  pc_ = regs[PROC_REG_PC];
  sp_ = regs[PROC_REG_SP];
  saved_fp = regs[PROC_REG_FP];

  //  Determine the value of the conceptual frame pointer
  //  for this function (not necessarily s8).
  instPoint     *ip = NULL;
  trampTemplate *bt = NULL;
  instInstance  *mt = NULL;
  pd_Function *currFunc = findAddressInFuncsAndTramps(p, pc_, ip, bt, mt);

  if ( currFunc )
  {
    Address base_addr;
    p->getBaseAddress(currFunc->file()->exec(), base_addr);
    Address fn_addr = base_addr + currFunc->getAddress(0);
  
    // adjust $pc for active instrumentation 
    Address pc_adj = adjustedPC(pc_, fn_addr, ip, bt, mt);
    Address pc_off = pc_adj - fn_addr;
    bool nativeFrameActive = nativFrameActive(ip, pc_off, currFunc, p);
    bool basetrampFrameActive = instrFrameActive(pc_, ip, bt, mt);

    if ( currFunc->uses_fp )
    {
      if ( basetrampFrameActive ) // use s8 value stored in bt frame
      {
        Address fp_bt = sp_;
        fp_bt += bt_frame_size;
        Address fp_addr = fp_bt + bt_fp_slot;
        fp_ = readAddressInMemory(p, fp_addr, true);
      }
      else if ( nativeFrameActive )
        fp_ = saved_fp;
      else
        fp_ = sp_;
    }
    else
    {
      fp_ = sp_;
      if ( basetrampFrameActive )
        fp_ += bt_frame_size;
      if ( nativeFrameActive )
        fp_ += currFunc->frame_size;
    }
  }
  else
    fp_ = sp_;

  /*
  if ( currFunc )
  {
    fprintf(stderr, "\nin getActiveFrame for function %s\n", currFunc->prettyName().string_of());
    fprintf(stderr, "  pc_ is      %x\n", pc_);
    fprintf(stderr, "  sp_ is      %x\n", sp_);
    fprintf(stderr, "  fp_ is      %x\n", fp_);
    fprintf(stderr, "  saved_fp is %x\n", saved_fp);
    fprintf(stderr, "  uses fp is %s\n\n", currFunc->uses_fp ? "true" : "false");
    }*/

  // sometimes the actual $fp is zero
  // (kludge for stack walk code)
  if (fp_ == 0) fp_ = sp_;
}
 
// determine if the basetramp frame is active
// NOTE: arguments 2-4 are from findAddressInFuncsAndTramps()
// NOTE: if "ip" is non-NULL, one of "bt" and "mt" must also be non-NULL
static bool instrFrameActive(Address pc,
			     instPoint *ip, 
			     trampTemplate *bt, 
			     instInstance *mt)
{
  // "ip" is set if $pc in instrumentation code
  // the basetramp frame is never active in native code
  if (!ip) return false;

  // "bt" is set if $pc is in basetramp
  // the basetramp frame is active in parts of the basetramp
  if (bt) return bt->inSavedRegion(pc);

  // "mt" is set if $pc is in minitramp
  // the basetramp frame is always active in minitramps
  else if (mt) return true;

  // one of "bt" and "mt" must be non-NULL
  assert(0);
  return false;
}

/* nativeFrameActive(): determine if the (relative) $pc is between the
   stack frame save and restore insns.  Functions are parsed to identify
   instructions that pop the stack frame and return.  If the pc is between
   a pair of these instructions, then the frame is not active.
*/
static bool nativFrameActive(instPoint* ip, Address pc_off,
                             pd_Function *callee, process* p)
{
  unsigned int idx;

  if (callee->frame_size == 0) return false;

  if (pc_off > callee->sp_mod) 
  {
    //  check if pc is between pop & return instructions
    for ( idx = 0; idx < callee->inactiveRanges.size(); idx++ )
    {
      //  Although not an issue at the time of this writing, there is the
      //  possibility that inactive-frame groups of instructions will be
      //  relocated to the basetramp in the future.  This will allow
      //  pre-insn instrumentation to execute at exit points with an active
      //  frame, and post-insn instrumentation to execute with the
      //  understanding that the frame has already been popped.
      //
      //  In order to account for this possibility, the current approach to
      //  identify if the pc is in code where the frame is inactive is :
      //    if the pc is within instrumentation, the inst point matches the
      //      frame pop instruction address, and the corrected pc offset
      //      is after the pop insn.
      //    if the pc is within identified ranges of pop/return instructions

      if ( ip &&
           ip->func_->getEffectiveAddress(p) ==
           callee->getEffectiveAddress(p) +
           callee->inactiveRanges[idx].popOffset )
      {
        if ( pc_off > callee->inactiveRanges[idx].popOffset )
          return false;
      }
      else if ( pc_off > callee->inactiveRanges[idx].popOffset &&
                pc_off <= callee->inactiveRanges[idx].retOffset )
      {
      	return false;
      }
    }
    
    return true;
  }

  return false;
}


static bool basetrampRegSaved(Address pc, Register reg,
			      instPoint *ip,
			      trampTemplate *bt,
			      instInstance *mt)
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

// return the corresponding $pc in native code
// (for a $pc in instrumentation code)
static Address adjustedPC(Address pc, Address fn_addr,
			  instPoint *ip,
			  trampTemplate *bt,
			  instInstance *mt)
{
  // if $pc in native code, no adjustment necessary
  if (!ip) return pc;
  
  // runtime address of instrumentation point
  Address pt_addr = fn_addr + ip->offset();

  if (bt) {
    // $pc in basetramp
    assert(bt->inBasetramp(pc));
    // assumption: basetramp has exactly two displaced insns
    assert(bt->skipPostInsOffset == bt->emulateInsOffset + 2*INSN_SIZE);

    int bt_off = pc - bt->baseAddr;
    if (bt_off <= bt->emulateInsOffset) {
      // $pc is at or before first displaced insn
      // $pc' = address of instr pt
      return pt_addr;
    } else if (bt_off == bt->emulateInsOffset + INSN_SIZE) {
      // $pc is at second displaced insn (delay slot)
      // $pc' = address of instr pt delay slot
      return pt_addr + INSN_SIZE;
    } else if (bt_off > bt->emulateInsOffset + INSN_SIZE) {
      // $pc is after second displaced insn
      // $pc' = address of insn after instr pt
      return pt_addr + (2*INSN_SIZE);
    }
  }

  else if (mt) {
    // $pc in minitramp
    if (mt->when == callPreInsn) {
      // $pc in pre-insn instr
      // $pc' = address of instr pt
      return pt_addr;
    } else if (mt->when == callPostInsn) {
      // $pc in post-insn instr
      // $pc' = address of insn after instr pt
      return pt_addr + (2*INSN_SIZE);
    }
  }

  // should not be reached: error
  assert(0);
  return pc;
}

// TODO: need dataflow, ($pc < saveInsn) insufficient
Frame Frame::getCallerFrameNormal(process *p) const
{
  // check for active instrumentation
  // (i.e. $pc in native/basetramp/minitramp code)
  instPoint     *ip = NULL;
  trampTemplate *bt = NULL;
  instInstance  *mt = NULL;
  pd_Function *callee = findAddressInFuncsAndTramps(p, pc_, ip, bt, mt);
  // non-NULL "ip" means that $pc is in instrumentation
  // non-NULL "bt" means that $pc is in basetramp
  // non-NULL "mt" means that $pc is in minitramp
  if (ip) assert(bt || mt);

  // calculate runtime address of callee fn
  if (!callee) {
    fprintf(stderr, "!!! <0x%016lx:???> unknown callee\n", pc_);
    return Frame(); // zero frame
  }

  Address base_addr;
  p->getBaseAddress(callee->file()->exec(), base_addr);
  Address fn_addr = base_addr + callee->getAddress(0);
  
  /* 
  // debug
  if (uppermost_) {
    char *info = "";
    if (ip) info = (bt) ? ("[basetramp]") : ("[minitramp]");
    fprintf(stderr, ">>> toplevel frame => \"%s\" %s\n",
	    callee->prettyName().string_of(), info);
  }
  */

  // adjust $pc for active instrumentation 
  Address pc_adj = adjustedPC(pc_, fn_addr, ip, bt, mt);
  // $pc' (adjusted $pc) should be inside callee
  /*
  if (pc_adj < fn_addr || pc_adj >= fn_addr + callee->size()) {
    fprintf(stderr, "!!! adjusted $pc\n");
    fprintf(stderr, "    0x%016lx $pc\n", pc_);
    fprintf(stderr, "    0x%016lx adjusted $pc\n", pc_adj);
    fprintf(stderr, "    0x%016lx fn start\n", fn_addr);
    fprintf(stderr, "    0x%016lx fn end\n", fn_addr + callee->size());
  }
  assert(pc_adj >= fn_addr && pc_adj < fn_addr + callee->size());
  */

  // which frames (native/basetramp) are active?
  Address pc_off = pc_adj - fn_addr;
  bool nativeFrameActive = nativFrameActive(ip, pc_off, callee, p);
  bool basetrampFrameActive = instrFrameActive(pc_, ip, bt, mt);

  // frame pointers for native and basetramp frames
  Address fp_bt = sp_;
  if (basetrampFrameActive) {
    fp_bt += bt_frame_size;
  }
  Address fp_native = fp_bt;
  if (nativeFrameActive) {
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

  // sanity checks
  if (!uppermost_) {
    /* if this is a non-toplevel stack frame, the basetramp $pc must
       point to just after an emulated call insn */
    if (bt) {
      assert(bt->skipPostInsOffset == bt->emulateInsOffset + (2*INSN_SIZE));
      /*
      if (pc_ != bt->baseAddr + bt->skipPostInsOffset) {
	fprintf(stderr, "!!! emulated call\n");
	fprintf(stderr, "    0x%016lx $pc\n", pc_);
	fprintf(stderr, "    0x%016lx emulated\n", bt->baseAddr + bt->skipPostInsOffset);
      }
      assert(pc_ == bt->baseAddr + bt->skipPostInsOffset);
      */
    }
    // non-toplevel basetramp frames should always be fully saved
    if (basetrampFrameActive) {
      /*
      if (!fp_saved_bt || !ra_saved_bt) 
	fprintf(stderr, "!!! $fp or $ra not saved in basetramp frame\n");
      fprintf(stderr, "    0x%016lx $pc\n", pc_);
	if (bt) { 
	  fprintf(stderr, "     [in basetramp]\n");
	  fprintf(stderr, "     0x%016lx basetramp\n", bt->baseAddr);
	} else if (mt) { fprintf(stderr, "     [in minitramp]\n"); }
      assert(fp_saved_bt && ra_saved_bt);
      */
    }
  }

  // find caller $pc (callee $ra)
  Address ra;
  Address ra_addr = 0;
  char ra_debug[256];
  sprintf(ra_debug, "<unknown>");

  if (nativeFrameActive && ra_saved_native) {
    // $ra saved in native frame
    ra_addr = fp_native + ra_save.slot;
    ra = readAddressInMemory(p, ra_addr, ra_save.dword);
    sprintf(ra_debug, "[$fp - %i]", -ra_save.slot);
  } else if (basetrampFrameActive && ra_saved_bt) {
    // $ra saved in basetramp frame
    ra_addr = fp_bt + bt_ra_slot;
    ra = readAddressInMemory(p, ra_addr, true);
    sprintf(ra_debug, "[$fp - %i]", -bt_ra_slot);
  } else {
    // $ra not saved in any frame
    // try to read $ra from registers (toplevel only)
    if (uppermost_) {
      // $ra in live register
      gregset_t regs;
      int proc_fd = p->getProcFileDescriptor();
      if (ioctl(proc_fd, PIOCGREG, &regs) == -1) {
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
	fprintf(stderr, "!!! <0x%016lx:\"%s\"> $ra not saved\n",
		pc_adj, callee->prettyName().string_of());
      */
      // $ra cannot be found (error)
      return Frame(); // zero frame
    }
  }

  // determine location of caller $pc (native code, basetramp, minitramp)
  instPoint *ip2 = NULL;
  trampTemplate *bt2 = NULL;
  instInstance *mt2 = NULL;
  pd_Function *caller = findAddressInFuncsAndTramps(p, ra, ip2, bt2, mt2);

  // Check for saved $fp value
  Address fp2;
  Address fp_addr = 0;
  char fp_debug[256];
  sprintf(fp_debug, "<unknown>");
  if (nativeFrameActive && fp_saved_native) {
    // $fp saved in native frame
    fp_addr = fp_native + fp_save.slot;
    fp2 = readAddressInMemory(p, fp_addr, fp_save.dword);
    sprintf(fp_debug, "[$fp - %i]", -fp_save.slot);
    //fprintf(stderr, "  read fp_saved_native at %x from fp_native %x and slot %d\n", fp_addr, fp_native, fp_save.slot);
  } else if (basetrampFrameActive && fp_saved_bt) {
    // $ra saved in basetramp frame
    fp_addr = fp_bt + bt_fp_slot;
    fp2 = readAddressInMemory(p, fp_addr, true);
    sprintf(fp_debug, "[$fp - %i]", -bt_fp_slot);
    //fprintf(stderr, "  read fp_saved_bt at %x from fp_native %x and slot %d\n", fp_addr, fp_bt, bt_fp_slot);
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
    int proc_fd = p->getProcFileDescriptor();
    if (ioctl(proc_fd, PIOCGREG, &regs) == -1) {
      perror("process::readDataFromFrame(PIOCGREG)");
      return Frame(); // zero frame
    }
    ra = regs[PROC_REG_RA];
    caller = findAddressInFuncsAndTramps(p, ra, ip2, bt2, mt2);
  }

  /* 
  // debug
  if(!caller) {
    fprintf(stderr, "!!! 0x%016lx unknown caller (callee:\"%s\")\n",
	    ra, callee->prettyName().string_of());    
    const image *owner = callee->file()->exec();
    Address obj_base = 0;
    p->getBaseAddress(owner, obj_base);
    Address addr;
    // frame pointer conventions
    if (callee->uses_fp) {
      fprintf(stderr, "    uses frame pointer\n");
      addr = obj_base + callee->getAddress(0) + callee->fp_mod;
      disDataSpace(p, (void *)addr, 1, "    $fp frame ");
      addr = obj_base + callee->getAddress(0) + ra_save.insn;
      disDataSpace(p, (void *)addr, 1, "    $ra save  ");
    } else {
      fprintf(stderr, "    no frame pointer\n");
      addr = obj_base + callee->getAddress(0) + callee->sp_mod;
      disDataSpace(p, (void *)addr, 1, "    $sp frame ");
      addr = obj_base + callee->getAddress(0) + ra_save.insn;
      disDataSpace(p, (void *)addr, 1, "    $ra save  ");
    }
    // callee $pc
    if (!ip) { 
      fprintf(stderr, "    in native code\n");
    } else if (bt) {
      fprintf(stderr, "    in basetramp\n");
    } else if (mt) {
      fprintf(stderr, "    in minitramp\n");    
    }
    fprintf(stderr, "    0x%016lx $pc\n", pc_);
    fprintf(stderr, "    0x%016lx native $pc\n", pc_adj);
    fprintf(stderr, "    %18s callee\n", callee->prettyName().string_of());
    // stack frames
    fprintf(stderr, "    0x%016lx callee $sp\n", sp_);
    fprintf(stderr, "    0x%016lx callee $fp\n", fp_);
    if (basetrampFrameActive) {
      fprintf(stderr, "    basetramp frame active\n");
      fprintf(stderr, "    0x%016x basetramp framesize\n", bt_frame_size);
      fprintf(stderr, "    0x%016lx basetramp $fp\n", fp_bt);
    } else fprintf(stderr, "    basetramp frame not active\n");
    if (nativeFrameActive) {
      fprintf(stderr, "    native frame active\n");
      fprintf(stderr, "    0x%016x native framesize\n", callee->frame_size);
      fprintf(stderr, "    0x%016lx native $fp\n", fp_native);
    } else fprintf(stderr, "    native frame not active\n");
    fprintf(stderr, "    0x%016lx $fp\n", fp_native);
    // caller $pc
    fprintf(stderr, "    %18s $ra slot\n", ra_debug);
    fprintf(stderr, "    0x%016lx $ra location\n", ra_addr);
    fprintf(stderr, "    0x%016lx $ra\n", ra);    
    // caller $fp
    fprintf(stderr, "    %18s $fp slot\n", fp_debug);
    fprintf(stderr, "    0x%016lx $fp location\n", fp_addr);
    fprintf(stderr, "    0x%016lx caller $fp\n", fp2);    
  }
  */

  // caller frame is invalid if $pc does not resolve to a function
  if (!caller) return Frame(); // zero frame

  // return value
  Frame ret;
  ret.pc_ = ra;
  ret.sp_ = fp_native;
  if ( caller )
  {
    if ( caller->uses_fp )
      ret.fp_ = fp2;
    else
      ret.fp_ = fp_native + caller->frame_size;
  }
  else 
    ret.fp_ = sp_;

  ret.saved_fp = fp2;

  /* 
  // debug
  fprintf(stderr, "    frame $ra(0x%016lx) $sp(0x%016lx) $fp(0x%016lx)", 
	  ret.pc_, ret.sp_, ret.fp_);
  char *info2 = "";
  if (ip2) info2 = (bt2) ? ("[basetramp]") : ("[minitramp]");
  fprintf(stderr, " => \"%s\" %s\n", caller->prettyName().string_of(), info2);
  */

  return ret;
}

// TODO: implement
// do leaf functions exist on Irix (other than syscalls)?
bool process::needToAddALeafFrame(Frame f, Address &leaf_pc) {
  return false;
}


//
// paradynd-only methods
//


void OS::osDisconnect(void) {
  //fprintf(stderr, ">>> osDisconnect()\n");
  int fd = open("/dev/tty", O_RDONLY);
  ioctl(fd, TIOCNOTTY, NULL); 
  P_close(fd);
}

#ifdef SHM_SAMPLING
rawTime64 process::getRawCpuTime_hw(int lwp_id) {
  lwp_id = 0;  // to turn off warning for now
  return 0;
}

/* return unit: nsecs */
rawTime64 process::getRawCpuTime_sw(int lwp_id) {
  //fprintf(stderr, ">>> getInferiorProcessCPUtime()\n");
  rawTime64 ret;

  /*
  pracinfo_t t;
  ioctl(proc_fd, PIOCACINFO, &t);
  ret = PDYN_div1000(t.pr_timers.ac_utime + t.pr_timers.ac_stime);
  */

  timespec_t t[MAX_PROCTIMER];
  if (ioctl(proc_fd, PIOCGETPTIMER, t) == -1) {
    //perror("getInferiorProcessCPUtime - PIOCGETPTIMER");
    return previous;
  }
  ret = 0;
  ret += t[AS_USR_RUN].tv_sec * I64_C(1000000000); // sec to nsec  (user)
  ret += t[AS_SYS_RUN].tv_sec * I64_C(1000000000); // sec to nsec  (sys)
  ret += t[AS_USR_RUN].tv_nsec;   // add in nsec (user)
  ret += t[AS_SYS_RUN].tv_nsec;   // add in nsec (sys)

  // sanity check: time should not go backwards
  if (ret < previous) {
    logLine("*** time going backwards in paradynd ***\n");
    ret = previous;
  }
  previous = ret;

  return ret;
}

#endif // SHM_SAMPLING


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

bool execIrixMPIProcess(vector<string> &argv)
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
    irixMPIappName = strdup(argv[0].string_of()); // used to identify MPI app processes
		
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
      args[ai] = P_strdup(argv[ai].string_of());
    args[argv.size()] = NULL;

    if ( P_execvp(args[0], args) == -1 )
    {
      perror("MPI application exec");
      exit(-1);
    }
  }
	
  return(true);
}

fileDescriptor *getExecFileDescriptor(string filename,
				     int &,
				     bool)
{
  fileDescriptor *desc = new fileDescriptor(filename);
  return desc;
}

#ifndef BPATCH_LIBRARY
void process::initCpuTimeMgrPlt() {
  cpuTimeMgr->installLevel(cpuTimeMgr_t::LEVEL_TWO, &process::yesAvail, 
			   timeUnit::ns(), timeBase::bNone(), 
			   &process::getRawCpuTime_sw, "DYNINSTgetCPUtime_sw");
}
#endif



