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
 * $Log: solaris.C,v $
 * Revision 1.11  1996/08/16 21:19:49  tamches
 * updated copyright for release 1.1
 *
 * Revision 1.10  1996/05/08 23:55:07  mjrg
 * added support for handling fork and exec by an application
 * use /proc instead of ptrace on solaris
 * removed warnings
 *
 * Revision 1.9  1996/04/03 14:27:56  naim
 * Implementation of deallocation of instrumentation for solaris and sunos - naim
 *
 * Revision 1.8  1996/03/12  20:48:39  mjrg
 * Improved handling of process termination
 * New version of aggregateSample to support adding and removing components
 * dynamically
 * Added error messages
 *
 * Revision 1.7  1996/02/12 16:46:18  naim
 * Updating the way we compute number_of_cpus. On solaris we will return the
 * number of cpus; on sunos, hp, aix 1 and on the CM-5 the number of processes,
 * which should be equal to the number of cpus - naim
 *
 * Revision 1.6  1996/02/09  23:53:46  naim
 * Adding new internal metric number_of_nodes - naim
 *
 * Revision 1.5  1995/09/26  20:17:52  naim
 * Adding error messages using showErrorCallback function for paradynd
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
#include <unistd.h>
#include "showerror.h"

#include <sys/procfs.h>
#include <poll.h>
#include <limits.h>

extern "C" {
extern int ioctl(int, int, ...);
extern long sysconf(int);
};

extern bool isValidAddress(process *proc, Address where);

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
  praddset(&exitSet, SYS_exec);
  praddset(&exitSet, SYS_execve);
  if (ioctl(fd, PIOCSEXIT, &exitSet) < 0) {
    fprintf(stderr, "osTraceMe: PIOCSEXIT failed: %s\n", sys_errlist[errno]); 
    fflush(stderr);
    P__exit(-1); // must use _exit here.
  }

  errno = 0;
  close(fd);
  return;
}


// already setup on this FD.
// disconnect from controlling terminal 
void OS::osDisconnect(void) {
  int ttyfd = open ("/dev/tty", O_RDONLY);
  ioctl (ttyfd, TIOCNOTTY, NULL); 
  P_close (ttyfd);
}

bool OS::osAttach(pid_t) { assert(0); }

bool OS::osStop(pid_t) { assert(0); }

bool OS::osDumpCore(pid_t, const string) { return false; }

bool OS::osForwardSignal (pid_t, int) { assert(0); }

bool OS::osDumpImage(const string &, pid_t , const Address) { return false; }


// return the result of an exec system call - true if succesful.
// On sparc, if an exec is succesful, the C condition code is clear.
static inline bool execResult(prstatus_t stat) {
  return ((stat.pr_reg[R_PSR] & 0x100000) == 0);
}

/*
   wait for inferior processes to terminate or stop.
*/
int process::waitProcs(int *status) {

   static struct pollfd fds[OPEN_MAX];  // argument for poll
   static int selected_fds;             // number of selected
   static int curr;                     // the current element of fds

   /* Each call to poll may return many selected fds. Since we only report the status
      of one process per each call to waitProcs, we keep the result of the last
      poll buffered, and simply process an element from the buffer until all of
      the selected fds in the last poll have been processed.
   */

   if (selected_fds == 0) {
     for (unsigned u = 0; u < processVec.size(); u++) {
       if (processVec[u]->status() == running || processVec[u]->status() == neonatal)
	 fds[u].fd = processVec[u]->proc_fd;
       else
	 fds[u].fd = -1;
       fds[u].events = POLLPRI;
       fds[u].revents = 0;
     }

     selected_fds = poll(fds, processVec.size(), 0);
     if (selected_fds < 0) {
       fprintf(stderr, "waitProcs: poll failed: %s\n", sys_errlist[errno]);
       selected_fds = 0;
       return 0;
     }

     curr = 0;
   }
   
   if (selected_fds > 0) {
     while (fds[curr].revents == 0)
       ++curr;

     // fds[curr] has an event of interest
     prstatus_t stat;
     int ret = 0;

     if (ioctl(fds[curr].fd, PIOCSTATUS, &stat) != -1 
	 && (stat.pr_flags & PR_STOPPED || stat.pr_flags & PR_ISTOP)) {
       switch (stat.pr_why) {
       case PR_SIGNALLED:
	 // return the signal number
	 *status = stat.pr_what << 8 | 0177;
	 ret = processVec[curr]->getPid();
	 break;
       case PR_SYSEXIT:
	 // exit of exec
	 if (!execResult(stat)) {
	   // a failed exec. continue the process
           processVec[curr]->continueProc_();
           break;
         }	    

	 *status = SIGTRAP << 8 | 0177;
	 ret = processVec[curr]->getPid();
	 break;
       case PR_REQUESTED:
       case PR_JOBCONTROL:
	 assert(0);
	 break;
       }	
      }

     --selected_fds;
     ++curr;

     if (ret > 0) {
       return ret;
     }
   }

   return waitpid(0, status, WNOHANG);
}


/*
   Open the /proc file correspoding to process pid, 
   set the signals to be caught to be only SIGSTOP,
   and set the kill-on-last-close and inherit-on-fork flags.
*/
bool process::attach() {
  char procName[128];

  sprintf(procName,"/proc/%05d", (int)pid);
  int fd = P_open(procName, O_RDWR, 0);
  if (fd < 0) {
    fprintf(stderr, "attach: open failed: %s\n", sys_errlist[errno]);
    return false;
  }

  /* we don't catch any child signals, except SIGSTOP */
  sigset_t sigs;
  premptyset(&sigs);
  praddset(&sigs, SIGSTOP);
  if (ioctl(fd, PIOCSTRACE, &sigs) < 0) {
    fprintf(stderr, "attach: ioctl failed: %s\n", sys_errlist[errno]);
    close(fd);
    return false;
  }

  /* turn on the kill-on-last-close and inherit-on-fork flags. This will cause
     the process to be killed when paradynd exits.
     Also, any child of this process will stop at the exit of an exec call.
  */
  long flags = PR_KLC | PR_FORK;
  if (ioctl (fd, PIOCSET, &flags) < 0) {
    fprintf(stderr, "attach: PIOCSET failed: %s\n", sys_errlist[errno]);
    close(fd);
    return false;
  }

  proc_fd = fd;
  return true;
}

/* 
   continue a process that is stopped 
*/
bool process::continueProc_() {
  ptraceOps++; ptraceOtherOps++;
  prrun_t flags;
  prstatus_t stat;

  // a process that receives a stop signal stops twice. We need to run the process
  // and wait for the second stop.
  if ((ioctl(proc_fd, PIOCSTATUS, &stat) != -1)
      && (stat.pr_flags & PR_STOPPED)
      && (stat.pr_why == PR_SIGNALLED)
      && (stat.pr_what == SIGSTOP) || (stat.pr_what == SIGINT)) {
    flags.pr_flags = PRSTOP;
    if (ioctl(proc_fd, PIOCRUN, &flags) == -1) {
      fprintf(stderr, "continueProc_: PIOCRUN failed: %s\n", sys_errlist[errno]);
      return false;
    }
    if (ioctl(proc_fd, PIOCWSTOP, 0) == -1) {
      fprintf(stderr, "continueProc_: PIOCWSTOP failed: %s\n", sys_errlist[errno]);
      return false;
    }
  }
  flags.pr_flags = PRCSIG; // clear current signal
  if (ioctl(proc_fd, PIOCRUN, &flags) == -1) {
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
  return (ioctl(proc_fd, PIOCSTOP, 0) != -1);
}

/*
   close the file descriptor for the file associated with a process
*/
bool process::detach_() {
  close(proc_fd);
  return true;
}

bool process::dumpCore_(const string) {
  return false;
}

bool process::writeTextWord_(caddr_t inTraced, int data) {
  return writeDataSpace_(inTraced, sizeof(int), (caddr_t) &data);
}

bool process::writeTextSpace_(caddr_t inTraced, int amount, caddr_t inSelf) {
  return writeDataSpace_(inTraced, amount, inSelf);
}

bool process::writeDataSpace_(caddr_t inTraced, int amount, caddr_t inSelf) {
  ptraceOps++; ptraceBytes += amount;
  if (lseek(proc_fd, (off_t)inTraced, SEEK_SET) != (off_t)inTraced)
    return false;
  return (write(proc_fd, inSelf, amount) == amount);
}

bool process::readDataSpace_(caddr_t inTraced, int amount, caddr_t inSelf) {
  ptraceOps++; ptraceBytes += amount;
  if (lseek(proc_fd, (off_t)inTraced, SEEK_SET) != (off_t)inTraced) {
    return false;
  }
  return (read(proc_fd, inSelf, amount) == amount);
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

int getNumberOfCPUs()
{
  // _SC_NPROCESSORS_CONF is the number of processors configured in the
  // system and _SC_NPROCESSORS_ONLN is the number of those processors that
  // are online.
  int numberOfCPUs;
  numberOfCPUs = (int) sysconf(_SC_NPROCESSORS_ONLN);
  if (numberOfCPUs) 
    return(numberOfCPUs);
  else 
    return(1);
}  

bool process::getActiveFrame(int *fp, int *pc)
{
  prgregset_t regs;
  bool ok=false;

  if (ioctl (proc_fd, PIOCGREG, &regs) != -1) {
      *fp=regs[R_O6];
      *pc=regs[R_PC];
      ok=true;
  }
/*
#ifdef FREEDEBUG
  if (!ok) 
    logLine("--> TEST <-- getActiveFrame is returning FALSE\n");
#endif

*/
  return(ok);
}

bool process::readDataFromFrame(int currentFP, int *fp, int *rtn)
{
  bool readOK=true;
  struct {
    int fp;
    int rtn;
  } addrs;

#ifdef FREEDEBUG
  static int fpT=0,rtnT=0;
#endif

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

#ifdef FREEDEBUG
    fpT=*fp; rtnT=*rtn;
#endif

    // if pc==0, then we are in the outermost frame and we should stop. We
    // do this by making fp=0.

#ifdef FREEDEBUG
if ( (addrs.rtn!=0) && (!isValidAddress(this,(Address) addrs.rtn)) ) {
  sprintf(errorLine,"==> TEST <== In readDataFromFrame, CHECK, pc out of range. pc=%d, fp=%d\n",rtnT,fpT);
  logLine(errorLine);
}
#endif

    if ( (addrs.rtn == 0) || !isValidAddress(this,(Address) addrs.rtn) ) {
      readOK=false;
    }
  }
  else {

#ifdef FREEDEBUG
  sprintf(errorLine,"==> TEST <== In readDataFromFrame, ERROR?, inTrace=%d, amount=%d, inSelf=%d, previousFP=%d, %x(hex) previousPC=%d, %x(hex). Keep going...\n",(currentFP+56),sizeof(int)*2,&addrs,fpT,fpT,rtnT,rtnT);
  logLine(errorLine);
#endif

    readOK=false;
  }

  return(readOK);
}

