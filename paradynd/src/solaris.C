
/* 
 * $Log: solaris.C,v $
 * Revision 1.5  1995/09/26 20:17:52  naim
 * Adding error messages using showErrorCallback function for paradynd
 *
 * Revision 1.4  1995/05/25  17:17:27  markc
 * Accept "1" as ok from call to uname()
 * Paradynd compiles on solaris again
 *
 * Revision 1.3  1995/02/16  08:54:15  markc
 * Corrected error in comments -- I put a "star slash" in the comment.
 *
 * Revision 1.2  1995/02/16  08:34:47  markc
 * Changed igen interfaces to use strings/vectors rather than char igen-arrays
 * Changed igen interfaces to use bool, not Boolean.
 * Cleaned up symbol table parsing - favor properly labeled symbol table objects
 * Updated binary search for modules
 * Moved machine dependnent ptrace code to architecture specific files.
 * Moved machine dependent code out of class process.
 * Removed almost all compiler warnings.
 * Use "posix" like library to remove compiler warnings
 *
 * Revision 1.1  1994/11/01  16:49:29  markc
 * Initial files that will provide os support.  This should limit os
 * specific features to these files.
 *
 */

#include "util/h/headers.h"
#include "os.h"
#include "process.h"
#include "stats.h"
#include "util/h/Types.h"
#include <sys/ioctl.h>
#include <fcntl.h>
#include <sys/termios.h>
#include "showerror.h"

extern "C" {
extern int ioctl(int, int, ...);
};

class ptraceKludge {
public:
  static bool haltProcess(process *p);
  static bool deliverPtrace(process *p, int req, int addr, int data);
  static void continueProcess(process *p, const bool wasStopped);
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

bool ptraceKludge::deliverPtrace(process *p, int req, int addr, int data) {
  bool halted = haltProcess(p);
  bool ret;
  if (P_ptrace(req, p->getPid(), addr, data) == -1)
    ret = false;
  else
    ret = true;
  continueProcess(p, halted);
  return ret;
}

void ptraceKludge::continueProcess(process *p, const bool wasStopped) {
  if ((p->status() != neonatal) && (!wasStopped))
    if (P_ptrace(PTRACE_CONT, p->pid, 1, SIGCONT) == -1) {
      cerr << "error in continueProcess\n";
      assert(0);
    }
}

// already setup on this FD.
// disconnect from controlling terminal 
void OS::osDisconnect(void) {
  int ttyfd = open ("/dev/tty", O_RDONLY);
  ioctl (ttyfd, TIOCNOTTY, NULL); 
  P_close (ttyfd);
}

// TODO
bool OS::osAttach(pid_t pid) {
  abort();
  return (P_ptrace(PTRACE_ATTACH, pid, 0, 0) != -1);
}

bool OS::osStop(pid_t pid) { return (P_kill(pid, SIGSTOP) != -1);}

bool OS::osDumpCore(pid_t pid, const string fileTo) {
  logLine("dumpcore not yet available");
  showErrorCallback(47, "");
  return false;
}

// TODO -- this should only be called when the process is stopped
bool OS::osForwardSignal (pid_t pid, int stat) {
  return (P_ptrace(PTRACE_CONT, pid, 1, stat) != -1);
}

bool OS::osDumpImage(const string &imageFileName, pid_t pid, const Address off) {
  logLine("dumpcore not yet available");
  showErrorCallback(47, "");
  return false;
}

void OS::osTraceMe(void) { P_ptrace(PTRACE_TRACEME, 0, 0, 0); }

// TODO I don't need to halt the process, do I?
bool process::continueProc_() {
  if (!checkStatus()) return false;
  ptraceOps++; ptraceOtherOps++;
  return (P_ptrace(PTRACE_CONT, pid, 1, 0) != -1);
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
  // TODO -- does this work for solaris?
  abort();
  // return (ptraceKludge::PCptrace(SIGCONT, 1, this, PTRACE_DETACH));
  return false;
}

// temporarily unimplemented, PTRACE_DUMPCORE is specific to sunos4.1
bool process::dumpCore_(const string coreFile) {
  if (!checkStatus()) 
    return false;
  ptraceOps++; ptraceOtherOps++;
  return false;
  // TODO -- this is not implemented
}

static bool writeHelper(int *tracedAddr, int request, int *localAddr,
			int amount, process *proc) {
  bool halt = ptraceKludge::haltProcess(proc);
  amount /= 4;
  for (unsigned i=0; i<amount; i++) {
    errno = 0;
    P_ptrace(request, proc->pid, (int) tracedAddr, *localAddr);
    assert(errno == 0);
    localAddr++; tracedAddr++;
  }
  ptraceKludge::continueProcess(proc, halt);
  return true;
}

bool process::writeTextWord_(caddr_t inTraced, int data) {
  if (!checkStatus()) return false;
  ptraceBytes += sizeof(int); ptraceOps++;
  return (ptraceKludge::deliverPtrace(this, PTRACE_POKETEXT, (int) ((void*)inTraced), data));
}

bool process::writeTextSpace_(caddr_t inTraced, int amount, caddr_t inSelf) {
  if (!checkStatus()) return false;
  ptraceBytes += amount; ptraceOps++;
  return (writeHelper((int*)((void*)inTraced), PTRACE_POKETEXT,
		      (int*)((void*)inSelf), amount, this));
}

bool process::writeDataSpace_(caddr_t inTraced, int amount, caddr_t inSelf) {
  if (!checkStatus()) return false;
  ptraceOps++; ptraceBytes += amount;
  return (writeHelper((int*)((void*)inTraced), PTRACE_POKEDATA,
		      (int*)((void*)inSelf), amount, this));
}

static bool readHelper(int *tracedAddr, int request, int *localAddr,
		       int amount, process *proc) {
  bool halt = ptraceKludge::haltProcess(proc);
  amount /= 4;
  for (unsigned i = 0; i < amount; i++) {
    errno = 0;
    int retval = P_ptrace(request, proc->pid, (int) tracedAddr, 0);
    assert(errno == 0);
    P_memcpy(localAddr, &retval, sizeof retval);
    localAddr++; tracedAddr++;
  }
  ptraceKludge::continueProcess(proc, halt);
  return true;
}

bool process::readDataSpace_(caddr_t inTraced, int amount, caddr_t inSelf) {
  if (!checkStatus())
    return false;
  ptraceOps++; ptraceBytes += amount;
  return (readHelper((int*) ((void*)inTraced), PTRACE_PEEKDATA,
		     (int*) ((void*)inSelf), amount, this));
}

bool process::loopUntilStopped() {
  /* make sure the process is stopped in the eyes of ptrace */
  OS::osStop(pid);
  bool isStopped = false;
  int waitStatus;
  while (!isStopped) {
    int ret = P_waitpid(pid, &waitStatus, WUNTRACED);
    if ((ret == -1 && errno == ECHILD) || (WIFEXITED(waitStatus))) {
      // the child is gone.
      status_ = exited;
      return(false);
    }
    if (!WIFSTOPPED(waitStatus) && !WIFSIGNALED(waitStatus)) {
      cerr << "problem stopping process\n";
      P_abort();
    }
    int sig = WSTOPSIG(waitStatus);
    if (sig == SIGSTOP) {
      isStopped = true;
    } else {
      if (P_ptrace(PTRACE_CONT, pid, 1, WSTOPSIG(waitStatus)) == -1) {
	cerr << "Ptrace error\n";
	P_abort();
      }
    }
  }
  return true;
}

// TODO -- only call getrusage once per round
static struct rusage *get_usage_data() {
  return NULL;
#ifdef notdef
  static bool init = false;
  static struct rusage *mapped = NULL;
  static struct rusage other;

  if (!init) {
    mapped = mapUarea();
    init = true;
  }
  if (!mapped) {
    mapped = &other;
    if (!getrusage(RUSAGE_SELF, &other))
      return mapped;
    else
      return NULL;
  } else
    return mapped;
#endif
}

float OS::compute_rusage_cpu() {
  return 0;
#ifdef notdef
  struct rusage *ru = get_usage_data();
  if (ru) {
    return ((float)ru->ru_utime.tv_sec + (float)ru->ru_utime.tv_usec / 1000000.0);
  } else
    return 0;
#endif
}

float OS::compute_rusage_sys() {
  return 0;
#ifdef notdef

  struct rusage *ru = get_usage_data();
  if (ru) {
    return ((float)ru->ru_stime.tv_sec + (float)ru->ru_stime.tv_usec / 1000000.0);
  } else
    return 0;
#endif
}

float OS::compute_rusage_min() {
  return 0;
#ifdef notdef

  struct rusage *ru = get_usage_data();
  if (ru) {
    return ((float) ru->ru_minflt);
  } else
    return 0;
#endif
}
float OS::compute_rusage_maj() {
  return 0;
#ifdef notdef

  struct rusage *ru = get_usage_data();
  if (ru) {
    return ((float) ru->ru_majflt);
  } else
    return 0;
#endif
}
float OS::compute_rusage_swap() {
  return 0;
#ifdef notdef

  struct rusage *ru = get_usage_data();
  if (ru) {
    return ((float) ru->ru_nswap);
  } else
    return 0;
#endif
}
float OS::compute_rusage_io_in() {
  return 0;
#ifdef notdef

  struct rusage *ru = get_usage_data();
  if (ru) {
    return ((float) ru->ru_inblock);
  } else
    return 0;
#endif
}
float OS::compute_rusage_io_out() {
  return 0;
#ifdef notdef

  struct rusage *ru = get_usage_data();
  if (ru) {
    return ((float) ru->ru_oublock);
  } else
    return 0;
#endif
}
float OS::compute_rusage_msg_send() {
  return 0;
#ifdef notdef

  struct rusage *ru = get_usage_data();
  if (ru) {
    return ((float) ru->ru_msgsnd);
  } else
    return 0;
#endif
}
float OS::compute_rusage_msg_recv() {
  return 0;
#ifdef notdef

  struct rusage *ru = get_usage_data();
  if (ru) {
    return ((float) ru->ru_msgrcv);
  } else
    return 0;
#endif
}
float OS::compute_rusage_sigs() {
  return 0;
#ifdef notdef

  struct rusage *ru = get_usage_data();
  if (ru) {
    return ((float) ru->ru_nsignals);
  } else
    return 0;
#endif
}
float OS::compute_rusage_vol_cs() {
  return 0;
#ifdef notdef

  struct rusage *ru = get_usage_data();
  if (ru) {
    return ((float) ru->ru_nvcsw);
  } else
    return 0;
#endif
}
float OS::compute_rusage_inv_cs() {
  return 0;
#ifdef notdef

  struct rusage *ru = get_usage_data();
  if (ru) {
    return ((float) ru->ru_nivcsw);
  } else
    return 0;
#endif
}
