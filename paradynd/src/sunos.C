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

/* 
 * $Log: sunos.C,v $
 * Revision 1.17  1996/08/20 19:18:02  lzheng
 * Implementation of moving multiple instructions sequence and
 * splitting the instrumentation into two phases
 *
 * Revision 1.16  1996/08/16 21:19:54  tamches
 * updated copyright for release 1.1
 *
 * Revision 1.15  1996/05/08 23:55:09  mjrg
 * added support for handling fork and exec by an application
 * use /proc instead of ptrace on solaris
 * removed warnings
 *
 * Revision 1.14  1996/04/03 14:27:58  naim
 * Implementation of deallocation of instrumentation for solaris and sunos - naim
 *
 * Revision 1.13  1996/03/12  20:48:40  mjrg
 * Improved handling of process termination
 * New version of aggregateSample to support adding and removing components
 * dynamically
 * Added error messages
 *
 * Revision 1.12  1996/02/12 16:46:19  naim
 * Updating the way we compute number_of_cpus. On solaris we will return the
 * number of cpus; on sunos, hp, aix 1 and on the CM-5 the number of processes,
 * which should be equal to the number of cpus - naim
 *
 * Revision 1.11  1996/02/09  23:53:48  naim
 * Adding new internal metric number_of_nodes - naim
 *
 * Revision 1.10  1995/11/22  00:02:23  mjrg
 * Updates for paradyndPVM on solaris
 * Fixed problem with wrong daemon getting connection to paradyn
 * Removed -f and -t arguments to paradyn
 * Added cleanUpAndExit to clean up and exit from pvm before we exit paradynd
 * Fixed bug in my previous commit
 *
 */


/*
 * The performance consultant's ptrace, it calls CM_ptrace and ptrace as needed.
 *
 */

#include "symtab.h"
#include "util/h/headers.h"
#include "os.h"
#include "process.h"
#include "stats.h"
#include "util/h/Types.h"
#include <sys/ioctl.h>
#include <fcntl.h>
#include <machine/reg.h>
#include "showerror.h"
#include "main.h"
// #include <sys/termios.h>

extern "C" {
extern int ioctl(int, int, ...);
extern int getrusage(int, struct rusage*);
#include <a.out.h>
#include <sys/exec.h>
#include <stab.h>
extern struct rusage *mapUarea();
};

extern bool isValidAddress(process *proc, Address where);

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

bool process::getActiveFrame(int *fp, int *pc)
{
  struct regs regs;
  if (ptraceKludge::deliverPtrace(this,PTRACE_GETREGS,(char *)&regs,0,0)) {
    *fp=regs.r_o6;
    *pc=regs.r_pc;
    return(true);
  }
  else return(false);
}

bool process::readDataFromFrame(int currentFP, int *fp, int *rtn, bool uppermost)
{
  bool readOK=true;
  struct {
    int fp;
    int rtn;
  } addrs;

  pdFunction *func;
  int pc = *rtn;
  struct regs regs; 

  if (uppermost) {
      func = symbols -> findFunctionIn(pc);
      if (func) {
	  if (func -> leaf) {
	      if (ptraceKludge::deliverPtrace(this,PTRACE_GETREGS,
					      (char *)&regs,0,0)) {
		  *rtn = regs.r_o7 + 8;
		  return readOK;
	      }    
	  }
      }
  }

  //
  // For the sparc, register %i7 is the return address - 8 and the fp is
  // register %i6. These registers can be located in currentFP+14*5 and
  // currentFP+14*4 respectively, but to avoid two calls to readDataSpace,
  // we bring both together (i.e. 8 bytes of memory starting at currentFP+14*4
  // or currentFP+56).
  //

  if (readDataSpace((caddr_t) (currentFP + 56),
                    sizeof(int)*2, (caddr_t) &addrs, true)) {
    // this is the previous frame pointer
    *fp = addrs.fp;
    // return address
    *rtn = addrs.rtn + 8;

    // if pc==0, then we are in the outermost frame and we should stop. We
    // do this by making fp=0.

    if ( (addrs.rtn == 0) || !isValidAddress(this,(Address) addrs.rtn) ) {
      readOK=false;
    }
  }
  else {
    readOK=false;
  }

  return(readOK);
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
#ifndef PTRACE_ATTACH_DETACH
    if (P_ptrace(PTRACE_CONT, p->pid, (caddr_t) 1, SIGCONT, NULL) == -1) {
#else
    if (P_ptrace(PTRACE_DETACH, p->pid, (caddr_t) 1, SIGCONT, NULL) == -1) {
#endif
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
#ifndef PTRACE_ATTACH_DETACH
	return (P_kill(pid, SIGSTOP) != -1); 
#else
	return (osAttach(pid));
#endif
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


// wait for a process to terminate or stop
int process::waitProcs(int *status) {
  return waitpid(0, status, WNOHANG);
}

// attach to an inferior process.
bool process::attach() {
  // we only need to attach to a process that is not our direct children.
  if (parent != 0) {
    return OS::osAttach(pid);
  }
  return true;
}


// TODO is this safe here ?
bool process::continueProc_() {
  int ret;

  if (!checkStatus()) 
    return false;
  ptraceOps++; ptraceOtherOps++;
/* choose either one of the following ptrace calls, but not both. 
 * The choice must be consistent with that in OS::osStop and ptraceKludge::continueProcess.
 */
#ifndef PTRACE_ATTACH_DETACH
  ret = P_ptrace(PTRACE_CONT, pid, (char*)1, 0, (char*)NULL);
#else
  ret = P_ptrace(PTRACE_DETACH, pid, (char*)1, SIGCONT, (char*)NULL);
#endif
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
  //assert(errno == 0);
  //return ret;
  return (ret != -1);
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
      handleProcessExit(this, WEXITSTATUS(waitStatus));
      return(false);
    } else if (WIFSIGNALED(waitStatus)) {
      handleProcessExit(this, WTERMSIG(waitStatus));
      return false;
    } else if (WIFSTOPPED(waitStatus)) {
      int sig = WSTOPSIG(waitStatus);
      if (sig == SIGSTOP) {
	isStopped = true;
      } else {
	extern int handleSigChild(int, int);
	handleSigChild(pid, waitStatus);
      }
    }
    else {
      logLine("Problem stopping process\n");
      abort();
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
    string msg = string("Dump core failed: unable to open file '") + imageFileName 
                 + string("': ") + string(sys_errlist[errno]);
    showErrorCallback(47, msg);
    return false;
    //P_perror("open");
    //cleanUpAndExit(-1);
  }

  rd = P_read(ifd, (void *) &my_exec, sizeof(struct exec));
  if (rd != sizeof(struct exec)) {
    string msg = string("Dump core failed: read failed '") + imageFileName 
                 + string("': ") + string(sys_errlist[errno]);
    showErrorCallback(47, msg);
    P_close(ifd);
    return false;
    //P_perror("read");
    //cleanUpAndExit(-1);
  }

  rd = P_fstat(ifd, &statBuf);
  if (rd != 0) {
    string msg = string("Dump core failed: fstat failed: ") + string(sys_errlist[errno]);
    showErrorCallback(47, msg);
    P_close(ifd);
    return false;
    //P_perror("fstat");
    //cleanUpAndExit(-1);
  }
  length = statBuf.st_size;
  sprintf(outFile, "%s.real", imageFileName.string_of());
  sprintf(errorLine, "saving program to %s\n", outFile);
  logLine(errorLine);

  ofd = P_open(outFile, O_WRONLY|O_CREAT, 0777);
  if (ofd < 0) {
    string msg = string("Dump core failed: unable to open file '") + string(outFile) 
                 + string("': ") + string(sys_errlist[errno]);
    showErrorCallback(47, msg);
    P_close(ifd);
    return false;
    //P_perror("open");
    //cleanUpAndExit(-1);
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
      string msg = string("Dump core failed: ptrace failed: ") 
	           + string(sys_errlist[errno]);
      showErrorCallback(47, msg);
      P_close(ofd); P_close(ifd);
      return false;
      //P_perror("ptrace");
      //assert(0);
    }
    P_write(ofd, buffer, 4096);
  }

  P_ptrace(PTRACE_CONT, pid, (char*) 1, SIGCONT, 0);

  rd = P_lseek(ofd, N_DATOFF(my_exec), SEEK_SET);
  if (rd != N_DATOFF(my_exec)) {
    string msg = string("Dump core failed: lseek failed: ") 
	           + string(sys_errlist[errno]);
    showErrorCallback(47, msg);
    P_close(ofd); P_close(ifd);
    return false;
    //P_perror("lseek");
    //cleanUpAndExit(-1);
  }

  rd = P_lseek(ifd, N_DATOFF(my_exec), SEEK_SET);
  if (rd != N_DATOFF(my_exec)) {
    string msg = string("Dump core failed: lseek failed: ") 
	           + string(sys_errlist[errno]);
    showErrorCallback(47, msg);
    P_close(ofd); P_close(ifd);
    return false;
    //P_perror("lseek");
    //cleanUpAndExit(-1);
  }

  total = N_DATOFF(my_exec);
  for (i=N_DATOFF(my_exec); i < length; i += 4096) {
    rd = P_read(ifd, buffer, 4096);
    P_write(ofd, buffer, rd);
    total += rd;
  }
  if (total != length) {
    string msg = string("Dump core failed: tried to write ") + string(length) +
                 string(" bytes, only ") + string(total) + string("written");
    showErrorCallback(47, msg);
    P_close(ofd); P_close(ifd);
    return false;
    //sprintf(errorLine, "Tried to write %d bytes, only %d written\n",
    //	    length, total);
    //logLine(errorLine);
    //showErrorCallback(57, (const char *) errorLine);
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

int getNumberOfCPUs()
{
  return(1);
}
