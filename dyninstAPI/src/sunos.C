
/* 
 * $Log: sunos.C,v $
 * Revision 1.3  1995/02/16 08:54:21  markc
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
// #include <sys/termios.h>

extern "C" {
extern int ioctl(int, int, ...);
#include <a.out.h>
#include <sys/exec.h>
#include <stab.h>
};

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
  bool halted = haltProcess(p);
  bool ret;
  if (P_ptrace(req, p->getPid(), addr, data, addr2) == -1)
    ret = false;
  else
    ret = true;
  continueProcess(p, halted);
  return ret;
}

void ptraceKludge::continueProcess(process *p, const bool wasStopped) {
  if ((p->status() != neonatal) && (!wasStopped))
    if (P_ptrace(PTRACE_CONT, p->pid, (caddr_t) 1, SIGCONT, NULL) == -1) {
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

bool OS::osStop(pid_t pid) { return (P_kill(pid, SIGSTOP) != -1); }

// TODO dump core
bool OS::osDumpCore(pid_t pid, const string dumpTo) {
  // return (!P_ptrace(PTRACE_DUMPCORE, pid, dumpTo, 0, 0));
  logLine("dumpcore not available yet");
  return false;
}

bool OS::osForwardSignal (pid_t pid, int stat) {
  return (P_ptrace(PTRACE_CONT, pid, (char*)1, stat, 0) != -1);
}

void OS::osTraceMe(void) { P_ptrace(PTRACE_TRACEME, 0, 0, 0, 0); }

// TODO is this safe here ?
bool process::continueProc_() {
  if (!checkStatus()) 
    return false;
  ptraceOps++; ptraceOtherOps++;
  return (P_ptrace(PTRACE_CONT, pid, (char*)1, 0, (char*)NULL) != -1);
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

  assert(0);
  errno = 0;
  // int ret = P_ptrace(request, pid, coreFile, 0, (char*) NULL);
  int ret = 0;
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
    sprintf(errorLine, "tried to write %d bytes, only %d written\n",
	    length, total);
    logLine(errorLine);
  }

  P_close(ofd);
  P_close(ifd);
  return true;
}
