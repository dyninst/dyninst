
/* 
 * $Log: sunos.C,v $
 * Revision 1.8  1995/09/26 20:17:53  naim
 * Adding error messages using showErrorCallback function for paradynd
 *
 * Revision 1.7  1995/09/18  22:42:09  mjrg
 * Fixed ptrace call.
 *
 * Revision 1.6  1995/09/11  19:19:26  mjrg
 * Removed redundant ptrace calls.
 *
 * Revision 1.5  1995/08/24  15:04:34  hollings
 * AIX/SP-2 port (including option for split instruction/data heaps)
 * Tracing of rexec (correctly spawns a paradynd if needed)
 * Added rtinst function to read getrusage stats (can now be used in metrics)
 * Critical Path
 * Improved Error reporting in MDL sematic checks
 * Fixed MDL Function call statement
 * Fixed bugs in TK usage (strings passed where UID expected)
 *
 * Revision 1.4  1995/05/18  10:42:12  markc
 * Added getruage calls
 *
 * Revision 1.3  1995/02/16  08:54:21  markc
 * Corrected error in comments -- I put a "star slash" in the comment.
 *
 * Revision 1.2  1995/02/16  08:34:54  markc
 * Changed igen interfaces to use strings/vectors rather than char igen-arrays
 * Changed igen interfaces to use bool, not Boolean.
 * Cleaned up symbol table parsing - favor properly labeled symbol table objects
 * Updated binary search for modules
 * Moved machine dependnent ptrace code to architecture specific files.
 * Moved machine dependent code out of class process.
 * Removed almost all compiler warnings.
 * Use "posix" like library to remove compiler warnings
 *
 * Revision 1.1  1994/11/01  16:49:27  markc
 * Initial files that will provide os support.  This should limit os
 * specific features to these files.
 *
 */


/*
 * The performance consultant's ptrace, it calls CM_ptrace and ptrace as needed.
 *
 */

#include "util/h/headers.h"
#include "os.h"
#include "process.h"
#include "stats.h"
#include "util/h/Types.h"
#include <sys/ioctl.h>
#include <fcntl.h>
#include "showerror.h"
// #include <sys/termios.h>

extern "C" {
extern int ioctl(int, int, ...);
extern int getrusage(int, struct rusage*);
#include <a.out.h>
#include <sys/exec.h>
#include <stab.h>
extern struct rusage *mapUarea();
};

extern struct rusage *mapUarea();

class ptraceKludge {
public:
  static bool haltProcess(process *p);
  static bool deliverPtrace(process *p, enum ptracereq req, char *addr,
			    int data, char *addr2);
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

bool ptraceKludge::deliverPtrace(process *p, enum ptracereq req, char *addr,
				 int data, char *addr2) {
  bool halted;
  bool ret;

  if (req != PTRACE_DETACH) halted = haltProcess(p);
  if (P_ptrace(req, p->getPid(), addr, data, addr2) == -1)
    ret = false;
  else
    ret = true;
  if (req != PTRACE_DETACH) continueProcess(p, halted);
  return ret;
}

void ptraceKludge::continueProcess(process *p, const bool wasStopped) {
  if ((p->status() != neonatal) && (!wasStopped))
/* Choose either one of the following methods to continue a process.
 * The choice must be consistent with that in process::continueProc_ and OS::osStop.
 */
    if (P_ptrace(PTRACE_DETACH, p->pid, (caddr_t) 1, SIGCONT, NULL) == -1) {
//    if (P_ptrace(PTRACE_CONT, p->pid, (caddr_t) 1, SIGCONT, NULL) == -1) {
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

bool OS::osAttach(pid_t process_id) {
  return (P_ptrace(PTRACE_ATTACH, process_id, 0, 0, 0) != -1);
}

bool OS::osStop(pid_t pid) { 
/* Choose either one of the following methods for stopping a process, but not both. 
 * The choice must be consistent with that in process::continueProc_ 
 * and ptraceKludge::continueProcess
 */
	return (osAttach(pid));
//	return (P_kill(pid, SIGSTOP) != -1); 
}

// TODO dump core
bool OS::osDumpCore(pid_t pid, const string dumpTo) {
  // return (!P_ptrace(PTRACE_DUMPCORE, pid, dumpTo, 0, 0));
  logLine("dumpcore not available yet");
  showErrorCallback(47, "");
  return false;
}

bool OS::osForwardSignal (pid_t pid, int stat) {
  return (P_ptrace(PTRACE_CONT, pid, (char*)1, stat, 0) != -1);
}

void OS::osTraceMe(void) { P_ptrace(PTRACE_TRACEME, 0, 0, 0, 0); }

// TODO is this safe here ?
bool process::continueProc_() {
  int ret;

  if (!checkStatus()) 
    return false;
  ptraceOps++; ptraceOtherOps++;
/* choose either one of the following ptrace calls, but not both. 
 * The choice must be consistent with that in OS::osStop and ptraceKludge::continueProcess.
 */
//  ret = P_ptrace(PTRACE_CONT, pid, (char*)1, 0, (char*)NULL);
  ret = P_ptrace(PTRACE_DETACH, pid, (char*)1, SIGCONT, (char*)NULL);
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
  return (ptraceKludge::deliverPtrace(this, PTRACE_DETACH, (char*)1, SIGCONT, NULL));
}

// temporarily unimplemented, PTRACE_DUMPCORE is specific to sunos4.1
bool process::dumpCore_(const string coreFile) {
  if (!checkStatus()) 
    return false;
  ptraceOps++; ptraceOtherOps++;

  errno = 0;
  int ret;
  if (coreFile.length() > 0)
    ret = P_ptrace(PTRACE_DUMPCORE, pid, P_strdup(coreFile.string_of()), 0, (char*) NULL);
  else
    ret = P_ptrace(PTRACE_DUMPCORE, pid, (char *) NULL, 0 , (char *) NULL);
  // int ret = 0;
  assert(errno == 0);
  return ret;
}

bool process::writeTextWord_(caddr_t inTraced, int data) {
  if (!checkStatus()) 
    return false;
  ptraceBytes += sizeof(int); ptraceOps++;
  return (ptraceKludge::deliverPtrace(this, PTRACE_POKETEXT, inTraced, data, NULL));
}

bool process::writeTextSpace_(caddr_t inTraced, int amount, caddr_t inSelf) {
  if (!checkStatus()) 
    return false;
  ptraceBytes += amount; ptraceOps++;
  return (ptraceKludge::deliverPtrace(this, PTRACE_WRITETEXT, inTraced, amount, inSelf));
}

bool process::writeDataSpace_(caddr_t inTraced, int amount, caddr_t inSelf) {
  if (!checkStatus())
    return false;
  ptraceOps++; ptraceBytes += amount;
  return (ptraceKludge::deliverPtrace(this, PTRACE_WRITEDATA, inTraced, amount, inSelf));
}

bool process::readDataSpace_(caddr_t inTraced, int amount, caddr_t inSelf) {
  if (!checkStatus())
    return false;
  ptraceOps++; ptraceBytes += amount;
  return (ptraceKludge::deliverPtrace(this, PTRACE_READDATA, inTraced, amount, inSelf));
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
      printf("problem stopping process\n");
      assert(0);
    }
    int sig = WSTOPSIG(waitStatus);
    if (sig == SIGSTOP) {
      isStopped = true;
    } else {
      if (P_ptrace(PTRACE_CONT, pid, (char*)1, WSTOPSIG(waitStatus), 0) == -1) {
	cerr << "Ptrace error\n";
	assert(0);
      }
    }
  }

  return true;
}


bool OS::osDumpImage(const string &imageFileName,  int pid, const Address codeOff)
{
  int i;
  int rd;
  int ifd;
  int ofd;
  int total;
  int length;
  struct exec my_exec;
  char buffer[4096];
  char outFile[256];
  extern int errno;
  struct stat statBuf;

  ifd = P_open(imageFileName.string_of(), O_RDONLY, 0);
  if (ifd < 0) {
    P_perror("open");
    P_exit(-1);
  }

  rd = P_read(ifd, (void *) &my_exec, sizeof(struct exec));
  if (rd != sizeof(struct exec)) {
    P_perror("read");
    P_exit(-1);
  }

  rd = P_fstat(ifd, &statBuf);
  if (rd != 0) {
    P_perror("fstat");
    P_exit(-1);
  }
  length = statBuf.st_size;
  sprintf(outFile, "%s.real", imageFileName.string_of());
  sprintf(errorLine, "saving program to %s\n", outFile);
  logLine(errorLine);

  ofd = P_open(outFile, O_WRONLY|O_CREAT, 0777);
  if (ofd < 0) {
    P_perror("open");
    P_exit(-1);
  }
  /* now copy the rest */

  P_lseek(ofd, 0, SEEK_SET);
  P_write(ofd, (void*) &my_exec, sizeof(struct exec));

  if (!stopped) {
    // make sure it is stopped.
    osStop(pid);
    P_waitpid(pid, NULL, WUNTRACED);
  }

  P_lseek(ofd, N_TXTOFF(my_exec), SEEK_SET);
  for (i=0; i < my_exec.a_text; i+= 4096) {
    errno = 0;
    P_ptrace(PTRACE_READTEXT, pid, (char*) (codeOff + i), 4096, buffer);
    if (errno) {
      P_perror("ptrace");
      assert(0);
    }
    P_write(ofd, buffer, 4096);
  }

  P_ptrace(PTRACE_CONT, pid, (char*) 1, SIGCONT, 0);

  rd = P_lseek(ofd, N_DATOFF(my_exec), SEEK_SET);
  if (rd != N_DATOFF(my_exec)) {
    P_perror("lseek");
    P_exit(-1);
  }

  rd = P_lseek(ifd, N_DATOFF(my_exec), SEEK_SET);
  if (rd != N_DATOFF(my_exec)) {
    P_perror("lseek");
    P_exit(-1);
  }

  total = N_DATOFF(my_exec);
  for (i=N_DATOFF(my_exec); i < length; i += 4096) {
    rd = P_read(ifd, buffer, 4096);
    P_write(ofd, buffer, rd);
    total += rd;
  }
  if (total != length) {
    sprintf(errorLine, "Tried to write %d bytes, only %d written\n",
	    length, total);
    logLine(errorLine);
    showErrorCallback(57, (const char *) errorLine);
  }

  P_close(ofd);
  P_close(ifd);
  return true;
}

// TODO -- only call getrusage once per round
static struct rusage *get_usage_data() {
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
}

float OS::compute_rusage_cpu() {
  struct rusage *ru = get_usage_data();
  if (ru) {
    return ((float)ru->ru_utime.tv_sec + (float)ru->ru_utime.tv_usec / 1000000.0);
  } else
    return 0;
}

float OS::compute_rusage_sys() {
  struct rusage *ru = get_usage_data();
  if (ru) {
    return ((float)ru->ru_stime.tv_sec + (float)ru->ru_stime.tv_usec / 1000000.0);
  } else
    return 0;
}

float OS::compute_rusage_min() {
  struct rusage *ru = get_usage_data();
  if (ru) {
    return ((float) ru->ru_minflt);
  } else
    return 0;
}
float OS::compute_rusage_maj() {
  struct rusage *ru = get_usage_data();
  if (ru) {
    return ((float) ru->ru_majflt);
  } else
    return 0;
}
float OS::compute_rusage_swap() {
  struct rusage *ru = get_usage_data();
  if (ru) {
    return ((float) ru->ru_nswap);
  } else
    return 0;
}
float OS::compute_rusage_io_in() {
  struct rusage *ru = get_usage_data();
  if (ru) {
    return ((float) ru->ru_inblock);
  } else
    return 0;
}
float OS::compute_rusage_io_out() {
  struct rusage *ru = get_usage_data();
  if (ru) {
    return ((float) ru->ru_oublock);
  } else
    return 0;
}
float OS::compute_rusage_msg_send() {
  struct rusage *ru = get_usage_data();
  if (ru) {
    return ((float) ru->ru_msgsnd);
  } else
    return 0;
}
float OS::compute_rusage_msg_recv() {
  struct rusage *ru = get_usage_data();
  if (ru) {
    return ((float) ru->ru_msgrcv);
  } else
    return 0;
}
float OS::compute_rusage_sigs() {
  struct rusage *ru = get_usage_data();
  if (ru) {
    return ((float) ru->ru_nsignals);
  } else
    return 0;
}
float OS::compute_rusage_vol_cs() {
  struct rusage *ru = get_usage_data();
  if (ru) {
    return ((float) ru->ru_nvcsw);
  } else
    return 0;
}
float OS::compute_rusage_inv_cs() {
  struct rusage *ru = get_usage_data();
  if (ru) {
    return ((float) ru->ru_nivcsw);
  } else
    return 0;
}
