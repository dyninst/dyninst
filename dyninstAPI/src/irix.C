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

// $Id: irix.C,v 1.6 1999/06/21 22:31:45 csserra Exp $

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
#include "dyninstAPI/src/process.h"
#include "dyninstAPI/src/stats.h" // ptrace{Ops,Bytes}
#include "util/h/pathName.h" // expand_tilde_pathname, exists_executable
#include "util/h/irixKludges.h" // PDYN_XXX
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
#include <sys/hwperftypes.h>  // hardware performance counters
#include <sys/hwperfmacros.h> // hardware performance counters


extern debug_ostream inferiorrpc_cerr;

extern char *Bool[];
void print_proc_flags(int fd)
{
  prstatus stat;
  ioctl(fd, PIOCSTATUS, &stat);
  fprintf(stderr, "flags: ");
  if (stat.pr_flags & PR_STOPPED) fprintf(stderr, "PR_STOPPED ");
  if (stat.pr_flags & PR_ISTOP) fprintf(stderr, "PR_ISTOP ");
  if (stat.pr_flags & PR_DSTOP) fprintf(stderr, "PR_DSTOP ");
  if (stat.pr_flags & PR_ASLEEP) fprintf(stderr, "PR_ASLEEP ");
  if (stat.pr_flags & PR_FORK) fprintf(stderr, "PR_FORK ");
  if (stat.pr_flags & PR_RLC) fprintf(stderr, "PR_RLC ");
  if (stat.pr_flags & PR_PTRACE) fprintf(stderr, "PR_PTRACE ");
  if (stat.pr_flags & PR_PCINVAL) fprintf(stderr, "PR_PCINVAL ");
  if (stat.pr_flags & PR_STEP) fprintf(stderr, "PR_STEP ");
  if (stat.pr_flags & PR_KLC) fprintf(stderr, "PR_KLC ");
  if (stat.pr_flags & PR_ISKTHREAD) fprintf(stderr, "PR_ISKTHREAD ");
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

#ifdef USES_NATIVE_CC
#include <dem.h>
static char cplus_demangle_buf[MAXDBUF];
extern "C" char *cplus_demangle(char *from, int notused) {
  fprintf(stderr, ">>> cplus_demangle(%s)\n", from);
  demangle(from, cplus_demangle_buf);
  return cplus_demangle_buf;
}
#endif /* USES_NATIVE_CC */

bool process::readDataSpace_(const void *inTraced, u_int nbytes, void *inSelf)
{
  //fprintf(stderr, ">>> process::readDataSpace_()\n");

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
    }
  }
  return true;
}

bool process::writeDataSpace_(void *inTraced, u_int nbytes, const void *inSelf)
{
  //fprintf(stderr, ">>> process::writeDataSpace_(0x%08x: %i bytes)\n", inTraced, nbytes);
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
  return readDataSpace_(inTraced, amount, inSelf);
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
  (void)praddset(&exitSet, SYS_execv);
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
  if (ioctl(fd, PIOCSTRACE, &sigs) < 0) {
    perror("process::attach(PIOCSTRACE)");
    close(fd);
    return false;
  }

  // step 3 - /proc PIOC{SET,RESET}:
  // a) turn on the kill-on-last-close flag (kills inferior with SIGKILL when
  //    the last writable /proc fd closes)
  // b) turn on inherit-on-fork flag (tracing flags inherited when child forks).
  // c) turn off run-on-last-close flag (on by default)
  // Also, any child of this process will stop at the exit of an exec call.
  long flags = PR_FORK;
#ifndef BPATCH_LIBRARY
  flags |= PR_KLC;
#endif
  if (ioctl (fd, PIOCSET, &flags) < 0) {
    perror("process::attach(PIOCSET)");
    close(fd);
    return false;
  }
  flags = PR_RLC;
  if (ioctl (fd, PIOCRESET, &flags) < 0) {
    perror("process::attach(PIOCRESET)");
    close(fd);
    return false;
  }

  proc_fd = fd;

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

#ifdef BPATCH_LIBRARY
static int process::waitProcs(int *status, bool block)
#else
/*static*/ 
int process::waitProcs(int *status)
#endif
{
  //fprintf(stderr, ">>> process::waitProcs()\n");

  extern vector<process*> processVec;

  static struct pollfd fds[OPEN_MAX];  // argument for poll
  static int selected_fds;             // number of selected
  static int curr;                     // the current element of fds

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
	fds[i].events = POLLPRI;
	fds[i].revents = 0;
      }
      
#ifdef BPATCH_LIBRARY
      int timeout;
      if (block) timeout = INFTIM;
      else timeout = 0;
      selected_fds = poll(fds, processVec.size(), timeout);
#else
      selected_fds = poll(fds, processVec.size(), 0);
#endif
      if (selected_fds == -1) {
	perror("process::waitProcs(poll)");
	selected_fds = 0;
	return 0;
      }      
      curr = 0;
    }
    
    if (selected_fds > 0) {
      while (fds[curr].revents == 0) curr++;
      // fds[curr] has an event of interest
      //fprintf(stderr, ">>> process::waitProcs(fd %i)\n", curr);

      prstatus_t stat;
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
	    }
	    else if (stat.pr_reg[PROC_REG_RV] != 0) {
	      // a failed exec; continue the process
	      p->continueProc_();
	      break;
	    }          
	    *status = SIGTRAP << 8 | 0177;
	    ret = p->getPid();
	    break;
	  }
	  case PR_REQUESTED:
	    // TODO: this has been reached
	    fprintf(stderr, ">>> process::waitProcs(fd %i): PR_REQUESTED\n", curr);
	    assert(0);
	  case PR_JOBCONTROL:
	    fprintf(stderr, ">>> process::waitProcs(fd %i): PR_JOBCONTROL\n", curr);
	    assert(0);
	    break;
	  }        
	}
      
      --selected_fds;
      ++curr;      
      if (ret > 0) return ret;
    }
#ifdef BPATCH_LIBRARY
  } while (block);
  return 0;
#else
  return waitpid(0, status, WNOHANG);
#endif
}

bool process::detach_()
{
  //fprintf(stderr, ">>> process::detach_()\n");
  if (close(proc_fd) == -1) {
    perror("process::detach_(close)");
    return false;
  }
  return true;
}

bool process::continueProc_()
{
  //fprintf(stderr, ">>> process::continueProc_()\n");
  ptraceOps++; 
  ptraceOtherOps++;

  prstatus_t stat;
  if (ioctl(proc_fd, PIOCSTATUS, &stat) == -1) {
    perror("process::continueProc_(PIOCSTATUS)");
    return false;
  }

  // debug
  //Address pc = stat.pr_reg[PROC_REG_PC];
  //disDataSpace(this, pc, 1, "  actual: ");
  //if (!(stat.pr_flags & PR_PCINVAL))
  //dis(&stat.pr_instr, pc, 1, "  /proc:  ");
  //fprintf(stderr, "  "); print_proc_flags(proc_fd);
  
  if (!(stat.pr_flags & (PR_STOPPED | PR_ISTOP))) {
    // not stopped
    return false;
  }
  
  prrun_t run;
  run.pr_flags = PRCSIG; // clear current signal
  if (hasNewPC) {        // new PC value
    fprintf(stderr, "  change $pc (0x%p to 0x%p)\n",
	    (void *)stat.pr_reg[PROC_REG_PC], (void *)currentPC_);
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
    Address addr = lookup_fn(this, name);
    if (!addr && findUs[i].must_find) {
      fprintf(stderr, "process::heapIsOk(): failed to find \"%s\"\n", name.string_of());
      return false;
    }
  }

  string heap_name = INFERIOR_HEAP_BASE;
  Address heap_addr = lookup_fn(this, heap_name);
  if (!heap_addr) {
    fprintf(stderr, "process::heapIsOk(): failed to find \"%s\"\n", heap_name.string_of());
    return false;
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
  //fprintf(stderr, ">>> process::read_inferiorRPC_result_register()\n");
  gregset_t regs;
  if (ioctl (proc_fd, PIOCGREG, &regs) == -1) {
    perror("process::_inferiorRPC_result_registerread(PIOCGREG)");
    return 0;
  }
  return regs[retval_reg];
}

static const Address lowest_addr = 0x00400000;
void inferiorMallocConstraints(Address near, Address &lo, Address &hi)
{
  lo = region_lo(near);
  hi = region_hi(near);  
  // avoid mapping the zero page
  if (lo < lowest_addr) lo = lowest_addr;
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
  //fprintf(stderr, ">>> process::continueWithForwardSignal()\n");
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

  Elf *elfp = elf_begin(fd, ELF_C_READ, 0);
  assert(elfp);
  Elf32_Ehdr *ehdrp = elf32_getehdr(elfp);
  assert(ehdrp);
  Elf_Scn *shstrscnp = elf_getscn(elfp, ehdrp->e_shstrndx);
  assert(shstrscnp);
  Elf_Data *shstrdatap = elf_getdata(shstrscnp, 0);
  assert(shstrdatap);
  char *shnames = (char *)shstrdatap->d_buf;

  Address txtAddr = 0;
  int txtLen = 0;
  int txtOff = 0;

  Elf_Scn *scn = 0;
  while ((scn = elf_nextscn(elfp, scn)) != 0) {
    Elf32_Shdr *shdrp = elf32_getshdr(scn);
    char *name = (char *)&shnames[shdrp->sh_name];
    if (strcmp(name, ".text") == 0) {
      txtOff = shdrp->sh_offset;
      txtLen = shdrp->sh_size;
      txtAddr = shdrp->sh_addr;
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

bool process::getActiveFrame(Address *fp, Address *pc)
{
  //fprintf(stderr, ">>> process::getActiveFrame()\n");

  *pc = 0;
  *fp = 0;
  
  gregset_t regs;
  if (ioctl(proc_fd, PIOCGREG, &regs) == -1) {
    perror("process::getActiveFrame(PIOCGREG)");
    return false;
  }

  // $pc value
  *pc = regs[PROC_REG_PC];

  // $fp value (no actual $fp register)
  pd_Function *fn = (pd_Function *)findFunctionIn(*pc);
  if (!fn) {
    //fprintf(stderr, "  $fp=%0#10x, $pc=%0#10x  FAILED\n", *fp, *pc);
    return false;
  }
  *fp = regs[PROC_REG_SP] + fn->frameSize;

  // debug
  //fprintf(stderr, "  $fp=%0#10x, $pc=%0#10x (in %s)\n", *fp, *pc, 
  //fn->prettyName().string_of());

  return true;
}

// "fp"        (in: callee $fp)
// "fp_caller" (out: caller $fp)
// "pc"        (in: callee $pc, out: caller $pc) 
bool process::readDataFromFrame(Address fp, Address *fp_caller, 
				Address *pc, bool uppermost)
{
  //fprintf(stderr, ">>> process::readDataFromFrame(fp=0x%08x, pc=0x%08x)\n", fp, *pc);
  if (fp == 0) return false;

  // find callee
  pd_Function *callee = (pd_Function *)findFunctionIn(*pc);
  if (!callee) return false;

  // find return address ($ra: caller $pc)
  Address ra;
  if (uppermost && *pc <= callee->saveInsn) {
    // read $ra register directly
    // TODO: need dataflow, ($pc < saveInsn) insufficient
    gregset_t regs;
    if (ioctl(proc_fd, PIOCGREG, &regs) == -1) {
      perror("process::readDataFromFrame(PIOCGREG)");
      return false;
    }
    ra = regs[PROC_REG_RA];
  } else {
    // fetch $ra from stack frame
    pd_Function::regSave_t &ra_save = callee->regSaves[REG_RA];
    if (ra_save.slot == -1) return false;
    Address sp = fp - callee->frameSize;
    Address ra_addr = sp + ra_save.slot;
    // TODO: address-in-memory
    if (ra_save.dword) {
      uint64_t raw64;
      readDataSpace((void *)ra_addr, sizeof(uint64_t), &raw64, true);
      ra = (Address)raw64;
    } else {
      uint32_t raw32;
      readDataSpace((void *)(ra_addr), sizeof(uint32_t), &raw32, true);
      ra = (Address)raw32;
    }
  }

  // find caller
  pd_Function *caller = (pd_Function *)findFunctionIn(ra);
  if (!caller) return false;

  // return values
  *pc = ra;
  *fp_caller = fp + caller->frameSize;
  return true;
}

bool process::needToAddALeafFrame(Frame /*current_frame*/, Address &/*leaf_pc*/)
{ // TODO
  //fprintf(stderr, "!!! process::needToAddALeafFrame()\n");
  return false;
}


//
// paradynd-only methods
//


void OS::osDisconnect(void) {
  int fd = open("/dev/tty", O_RDONLY);
  ioctl(fd, TIOCNOTTY, NULL); 
  P_close(fd);
}

#ifdef SHM_SAMPLING
// returns user+sys time from the u or proc area of the inferior process, which in
// turn is presumably obtained by mmapping it (sunos) or by using a /proc ioctl
// to obtain it (solaris).  It must not stop the inferior process in order
// to obtain the result, nor can it assue that the inferior has been stopped.
// The result MUST be "in sync" with rtinst's DYNINSTgetCPUtime().
#define HW_CTR_NUM (0) // must be consistent with RTirix.c
// TODO: "#ifdef PURE_BUILD" support
time64 process::getInferiorProcessCPUtime() {
  //fprintf(stderr, ">>> getInferiorProcessCPUtime()\n");
  time64 ret;
  static time64 ret_prev = 0;
  static bool use_hw_ctrs = false;
  static uint64_t cycles_per_usec = 0;
  static Address gen_num_addr = 0;
  static bool init = true;
  if (init) {
    //fprintf(stderr, ">>> getInferiorProcessCPUtime(init)\n");
    Address use_hw_addr = lookup_fn(this, "DYNINSTos_CPUctr_use");
    char byte = 0;
    readDataSpace_((void *)use_hw_addr, sizeof(char), &byte);
    use_hw_ctrs = byte;
    if (use_hw_ctrs) {
      Address cycles_addr = lookup_fn(this, "DYNINSTos_CPUctr_cycles");
      readDataSpace_((void *)cycles_addr, sizeof(uint64_t), &cycles_per_usec);
      gen_num_addr = lookup_fn(this, "DYNINSTos_CPUctr_gen");
    }
    init = false;
  }

  if (use_hw_ctrs) {
    hwperf_cntr_t count;
    int gen_num;
    if ((gen_num = ioctl(proc_fd, PIOCGETEVCTRS, &count)) == -1) {
      perror("getInferiorProcessCPUtime - PIOCGETEVCTRS");
      return ret_prev;
    }
    ret = count.hwp_evctr[HW_CTR_NUM] / cycles_per_usec;
    // generation numbers
    // TODO: do not check gen num - save readDataSpace() latency
    /*
    int inf_gen_num;
    readDataSpace_((void *)gen_num_addr, sizeof(int), &inf_gen_num);
    if (gen_num != inf_gen_num) {
      fprintf(stderr, "!!! paradynd: hwperf counters generation number mismatch\n");
    }
    */
  } else { // not using hardware performance counters
    prusage_t usage;
    if (ioctl(proc_fd, PIOCUSAGE, &usage) == -1) {
      perror("getInferiorProcessCPUtime - PIOCUSAGE");
      return ret_prev;
    }
    ret = 0;
    ret += PDYN_mulMillion(usage.pu_utime.tv_sec); // sec to usec  (user)
    ret += PDYN_mulMillion(usage.pu_stime.tv_sec); // sec to usec  (sys)
    ret += PDYN_div1000(usage.pu_utime.tv_nsec);   // nsec to usec (user)
    ret += PDYN_div1000(usage.pu_stime.tv_nsec);   // nsec to usec (sys)
  }

  // sanity check: time should not go backwards
  if (ret < ret_prev) {
    logLine("*** time going backwards in paradynd ***\n");
    ret = ret_prev;
  }
  ret_prev = ret;

  return ret;
}
#endif

