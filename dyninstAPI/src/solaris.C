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

#include "dyninstAPI/src/symtab.h"
#include "util/h/headers.h"
#include "dyninstAPI/src/os.h"
#include "dyninstAPI/src/process.h"
#include "dyninstAPI/src/stats.h"
#include "util/h/Types.h"
#include <sys/ioctl.h>
#include <fcntl.h>
#include <sys/termios.h>
#include <unistd.h>
#include "paradynd/src/showerror.h"
#include "util/h/pathName.h" // concat_pathname_components()
#include "util/h/debugOstream.h"
#include "util/h/solarisKludges.h"

#if defined (sparc_sun_solaris2_4)
#include "dyninstAPI/src/inst-sparc.h"
#else
#include "dyninstAPI/src/inst-x86.h"
#endif

#include "instPoint.h"

#include <sys/procfs.h>
#include <poll.h>
#include <limits.h>
#include <link.h>

extern "C" {
extern int ioctl(int, int, ...);
extern long sysconf(int);
};

// The following were defined in process.C
extern debug_ostream attach_cerr;
extern debug_ostream inferiorrpc_cerr;
extern debug_ostream shmsample_cerr;
extern debug_ostream forkexec_cerr;
extern debug_ostream metric_cerr;
extern debug_ostream signal_cerr;

/*
   Define the indices of some registers to be used with pr_reg.
   These values are different on sparc and x86 platforms.
   RETVAL_REG: the registers that holds the return value of calls ($o0 on sparc,
               %eax on x86).
   PC_REG: program counter
   FP_REG: frame pointer (%i7 on sparc, %ebp on x86) 
*/
#ifdef sparc_sun_solaris2_4
#define RETVAL_REG (R_O0)
#define PC_REG (R_PC)
#define FP_REG (R_O6)
#endif
#ifdef i386_unknown_solaris2_5
#define RETVAL_REG (EAX)
#define PC_REG (EIP)
#define FP_REG (EBP)
#endif


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

bool process::continueWithForwardSignal(int) {
   if (-1 == ioctl(proc_fd, PIOCRUN, NULL)) {
      perror("could not forward signal in PIOCRUN");
      return false;
   }

   return true;
}

bool process::dumpImage() {return false;}


/* 
   execResult: return the result of an exec system call - true if succesful.
   The traced processes will stop on exit of an exec system call, just before
   returning to user code. At this point the return value (errno) is already
   written to a register, and we need to check if the return value is zero.
 */
static inline bool execResult(prstatus_t stat) {
  return (stat.pr_reg[RETVAL_REG] == 0);
}

/*
   wait for inferior processes to terminate or stop.
*/
int process::waitProcs(int *status) {
   extern vector<process*> processVec;

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
       if (processVec[u] && (processVec[u]->status() == running || processVec[u]->status() == neonatal))
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
	 && ((stat.pr_flags & PR_STOPPED) || (stat.pr_flags & PR_ISTOP))) {
       switch (stat.pr_why) {
       case PR_SIGNALLED:
	 // return the signal number
	 *status = stat.pr_what << 8 | 0177;
	 ret = processVec[curr]->getPid();
	 break;
       case PR_SYSEXIT: {
	 // exit of a system call.
	 process *p = processVec[curr];

	 if (p->RPCs_waiting_for_syscall_to_complete) {
 	    // reset PIOCSEXIT mask
	    inferiorrpc_cerr << "solaris got PR_SYSEXIT!" << endl;
	    assert(p->save_exitset_ptr != NULL);
	    if (-1 == ioctl(p->proc_fd, PIOCSEXIT, p->save_exitset_ptr))
	       assert(false);
	    delete [] p->save_exitset_ptr;
	    p->save_exitset_ptr = NULL;

	    // fall through on purpose (so status, ret get set)
	 }
	 else if (!execResult(stat)) {
	   // a failed exec. continue the process
           processVec[curr]->continueProc_();
           break;
         }	    

	 *status = SIGTRAP << 8 | 0177;
	 ret = processVec[curr]->getPid();
	 break;
       }
       case PR_REQUESTED:
         assert(0);
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


static char *extract_string_ptr(int procfd, char **ptr) {
   // we want to return *ptr.

   if (-1 == lseek(procfd, (long)ptr, SEEK_SET))
      assert(false);

   char *result;
   if (-1 == read(procfd, &result, sizeof(result)))
      assert(false);

   return result;   
}

string extract_string(int procfd, const char *inferiorptr) {
   // assuming inferiorptr points to a null-terminated string in the inferior
   // process, extract it and return it.

   if (-1 == lseek(procfd, (long)inferiorptr, SEEK_SET))
      return "";

   string result;
   while (true) {
      char buffer[100];
      if (-1 == read(procfd, &buffer, 80))
	 return "";
      buffer[80] = '\0';
      result += buffer;

      // was there a '\0' anywhere in chars 0 thru 79?  If so then
      // we're done
      for (unsigned lcv=0; lcv < 80; lcv++)
	 if (buffer[lcv] == '\0') {
	    //attach_cerr << "extract_string returning " << result << endl;
	    return result;
	 }
   }
}

void get_ps_stuff(int proc_fd, string &argv0, string &pathenv, string &cwdenv) {
   // Use ps info to obtain argv[0], PATH, and curr working directory of the
   // inferior process designated by proc_fd.  Writes to argv0, pathenv, cwdenv.

   prpsinfo the_psinfo;
   if (-1 == ioctl(proc_fd, PIOCPSINFO, &the_psinfo))
      assert(false);

   // get argv[0].  It's in the_psinfo.pr_argv[0], but that's a ptr in the inferior
   // space, so we need to /proc read() it out.  Also, the_psinfo.pr_argv is a char **
   // not a char* so we even need to /proc read() to get a pointer value.  Ick.
   assert(the_psinfo.pr_argv != NULL);
   char *ptr_to_argv0 = extract_string_ptr(proc_fd, the_psinfo.pr_argv);
   argv0 = extract_string(proc_fd, ptr_to_argv0);

   // Get the PWD and PATH environment variables from the application.
   char **envptr = the_psinfo.pr_envp;
   while (true) {
      // dereference envptr; check for NULL
      char *env = extract_string_ptr(proc_fd, envptr);
      if (env == NULL)
	 break;

      string env_value = extract_string(proc_fd, env);
      if (env_value.prefixed_by("PWD=") || env_value.prefixed_by("CWD=")) {
	 cwdenv = env_value.string_of() + 4; // skip past "PWD=" or "CWD="
	 attach_cerr << "get_ps_stuff: using PWD value of: " << cwdenv << endl;
      }
      else if (env_value.prefixed_by("PATH=")) {
	 pathenv = env_value.string_of() + 5; // skip past the "PATH="
	 attach_cerr << "get_ps_stuff: using PATH value of: " << pathenv << endl;
      }

      envptr++;
   }
}

/*
   Open the /proc file correspoding to process pid, 
   set the signals to be caught to be only SIGSTOP and SIGTRAP,
   and set the kill-on-last-close and inherit-on-fork flags.
*/
extern string pd_flavor ;
bool process::attach() {
  char procName[128];

  // QUESTION: does this attach operation lead to a SIGTRAP being forwarded
  // to paradynd in all cases?  How about when we are attaching to an
  // already-running process?  (Seems that in the latter case, no SIGTRAP
  // is automatically generated)

  // step 1) /proc open: attach to the inferior process
  sprintf(procName,"/proc/%05d", (int)pid);
  int fd = P_open(procName, O_RDWR, 0);
  if (fd < 0) {
    fprintf(stderr, "attach: open failed: %s\n", sys_errlist[errno]);
    return false;
  }

  // step 2) /proc PIOCSTRACE: define which signals should be forwarded to daemon
  //   These are (1) SIGSTOP and (2) either SIGTRAP (sparc) or SIGILL (x86), to
  //   implement inferiorRPC completion detection.
  sigset_t sigs;
  premptyset(&sigs);
  praddset(&sigs, SIGSTOP);

#ifndef i386_unknown_solaris2_5
  praddset(&sigs, SIGTRAP);
#endif

#ifdef i386_unknown_solaris2_5
  praddset(&sigs, SIGILL);
#endif

  if (ioctl(fd, PIOCSTRACE, &sigs) < 0) {
    fprintf(stderr, "attach: ioctl failed: %s\n", sys_errlist[errno]);
    close(fd);
    return false;
  }

  // Step 3) /proc PIOCSET:
  // a) turn on the kill-on-last-close flag (kills inferior with SIGKILL when
  //    the last writable /proc fd closes)
  // b) turn on inherit-on-fork flag (tracing flags inherited when child forks).
  // c) turn on breakpoint trap pc adjustment (x86 only).
  // Also, any child of this process will stop at the exit of an exec call.

  //Tempest, do not need to inherit-on-fork
  long flags ;
  if(process::pdFlavor == string("cow"))
  	flags = PR_KLC |  PR_BPTADJ;
  else
   	flags = PR_KLC | PR_FORK | PR_BPTADJ;
  if (ioctl (fd, PIOCSET, &flags) < 0) {
    fprintf(stderr, "attach: PIOCSET failed: %s\n", sys_errlist[errno]);
    close(fd);
    return false;
  }

  proc_fd = fd;

  get_ps_stuff(proc_fd, this->argv0, this->pathenv, this->cwdenv);

  return true;
}

bool process::isRunning_() const {
   // determine if a process is running by doing low-level system checks, as
   // opposed to checking the 'status_' member vrble.  May assume that attach()
   // has run, but can't assume anything else.
   prstatus theStatus;
   if (-1 == ioctl(proc_fd, PIOCSTATUS, &theStatus)) {
      perror("process::isRunning_()");
      assert(false);
   }

   if (theStatus.pr_flags & PR_STOPPED)
      return false;
   else
      return true;
}

bool process::attach_() {assert(false);}
bool process::stop_() {assert(false);}

/* 
   continue a process that is stopped 
*/
bool process::continueProc_() {
  ptraceOps++; ptraceOtherOps++;
  prrun_t flags;
  prstatus_t stat;

//cerr << "welcome to continueProc_()" << endl;

  // a process that receives a stop signal stops twice. We need to run the process
  // and wait for the second stop. (The first run simply absorbs the stop signal;
  // the second one does the actual continue.)
  if ((ioctl(proc_fd, PIOCSTATUS, &stat) != -1)
      && (stat.pr_flags & PR_STOPPED)
      && (stat.pr_why == PR_SIGNALLED)
      && (stat.pr_what == SIGSTOP || stat.pr_what == SIGINT)) {
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
  if (hasNewPC) {
    // set new program counter
    //cerr << "continueProc_ doing new currentPC: " << (void*)currentPC_ << endl;
    flags.pr_vaddr = (caddr_t) currentPC_;
    flags.pr_flags |= PRSVADDR;
    hasNewPC = false;
  }
  if (ioctl(proc_fd, PIOCRUN, &flags) == -1) {
    fprintf(stderr, "continueProc_: PIOCRUN 2 failed: %s\n", sys_errlist[errno]);
    return false;
  }

  return true;
}

#ifdef BPATCH_LIBRARY
/*
   terminate execution of a process
 */
bool process::terminateProc_()
{
    int sig = SIGKILL;
    if (ioctl(proc_fd, PIOCKILL, &sig) == -1)
	return false;
    else
	return true;
}
#endif

/*
   pause a process that is running
*/
bool process::pause_() {
  ptraceOps++; ptraceOtherOps++;

  // /proc PIOCSTOP: direct all LWPs to stop, _and_ wait for them to stop.
  return (ioctl(proc_fd, PIOCSTOP, 0) != -1);
}

/*
   close the file descriptor for the file associated with a process
*/
bool process::detach_() {
  close(proc_fd);
  return true;
}

#ifdef BPATCH_LIBRARY
/*
   detach from thr process, continuing its execution if the parameter "cont"
   is true.
 */
bool process::API_detach_(const bool cont)
{
  // Reset the kill-on-close flag, and the run-on-last-close flag if necessary
  long flags = PR_KLC;
  if (!cont) flags |= PR_RLC;
  if (ioctl (proc_fd, PIOCRESET, &flags) < 0) {
    fprintf(stderr, "detach: PIOCRESET failed: %s\n", sys_errlist[errno]);
    close(proc_fd);
    return false;
  }
  // Set the run-on-last-close-flag if necessary
  if (cont) {
    flags = PR_RLC;
    if (ioctl (proc_fd, PIOCSET, &flags) < 0) {
      fprintf(stderr, "detach: PIOCSET failed: %s\n", sys_errlist[errno]);
      close(proc_fd);
      return false;
    }
  }

  sigset_t sigs;
  premptyset(&sigs);
  if (ioctl(proc_fd, PIOCSTRACE, &sigs) < 0) {
    fprintf(stderr, "detach: PIOCSTRACE failed: %s\n", sys_errlist[errno]);
    close(proc_fd);
    return false;
  }
  if (ioctl(proc_fd, PIOCSHOLD, &sigs) < 0) {
    fprintf(stderr, "detach: PIOCSHOLD failed: %s\n", sys_errlist[errno]);
    close(proc_fd);
    return false;
  }

  fltset_t faults;
  premptyset(&faults);
  if (ioctl(proc_fd, PIOCSFAULT, &faults) < 0) {
    fprintf(stderr, "detach: PIOCSFAULT failed: %s\n", sys_errlist[errno]);
    close(proc_fd);
    return false;
  }
  
  sysset_t syscalls;
  premptyset(&syscalls);
  if (ioctl(proc_fd, PIOCSENTRY, &syscalls) < 0) {
    fprintf(stderr, "detach: PIOCSENTRY failed: %s\n", sys_errlist[errno]);
    close(proc_fd);
    return false;
  }
  if (ioctl(proc_fd, PIOCSEXIT, &syscalls) < 0) {
    fprintf(stderr, "detach: PIOCSEXIT failed: %s\n", sys_errlist[errno]);
    close(proc_fd);
    return false;
  }

  close(proc_fd);
  return true;
}
#endif

bool process::dumpCore_(const string) {
  return false;
}

bool process::writeTextWord_(caddr_t inTraced, int data) {
//  cerr << "writeTextWord @ " << (void *)inTraced << endl; cerr.flush();
  return writeDataSpace_(inTraced, sizeof(int), (caddr_t) &data);
}

bool process::writeTextSpace_(void *inTraced, int amount, const void *inSelf) {
//  cerr << "writeTextSpace pid=" << getPid() << ", @ " << (void *)inTraced << " len=" << amount << endl; cerr.flush();
  return writeDataSpace_(inTraced, amount, inSelf);
}

#ifdef BPATCH_SET_MUTATIONS_ACTIVE
bool process::readTextSpace_(void *inTraced, int amount, const void *inSelf) {
  return readDataSpace_(inTraced, amount, inSelf);
}
#endif

bool process::writeDataSpace_(void *inTraced, int amount, const void *inSelf) {
  ptraceOps++; ptraceBytes += amount;

//  cerr << "process::writeDataSpace_ pid " << getPid() << " writing " << amount << " bytes at loc " << inTraced << endl;

  if (lseek(proc_fd, (off_t)inTraced, SEEK_SET) != (off_t)inTraced)
    return false;
  return (write(proc_fd, inSelf, amount) == amount);
}

bool process::readDataSpace_(const void *inTraced, int amount, void *inSelf) {
  ptraceOps++; ptraceBytes += amount;
  if((lseek(proc_fd, (off_t)inTraced, SEEK_SET)) != (off_t)inTraced) {
    printf("error in lseek addr = 0x%x amount = %d\n",(u_int)inTraced,amount);
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
      *fp=regs[FP_REG];
      *pc=regs[PC_REG];
      ok=true;
  }
  return(ok);
}

#ifdef sparc_sun_solaris2_4

bool process::readDataFromFrame(int currentFP, int *fp, int *rtn, bool uppermost)
{
  bool readOK=true;
  struct {
    int fp;
    int rtn;
  } addrs;

  prgregset_t regs;
  function_base *func;
  int pc = *rtn;

  if (uppermost) {
      func = this->findFunctionIn(pc);
      if (func) {
	 if (func->hasNoStackFrame()) { // formerly "isLeafFunc()"
	      if (ioctl (proc_fd, PIOCGREG, &regs) != -1) {
		  *rtn = regs[R_O7] + 8;
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
  // These values are copied to the stack when the application is paused,
  // so we are assuming that the application is paused at this point

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

#endif

#ifdef SHM_SAMPLING
time64 process::getInferiorProcessCPUtime() {
   // returns user+sys time from the u or proc area of the inferior process, which in
   // turn is presumably obtained by mmapping it (sunos) or by using a /proc ioctl
   // to obtain it (solaris).  It must not stop the inferior process in order
   // to obtain the result, nor can it assue that the inferior has been stopped.
   // The result MUST be "in sync" with rtinst's DYNINSTgetCPUtime().

   // We use the PIOCUSAGE /proc ioctl

   // Other /proc ioctls that should work too: PIOCPSINFO
   // and the lower-level PIOCGETPR and PIOCGETU which return copies of the proc
   // and u areas, respectively.
   // PIOCSTATUS does _not_ work because its results are not in sync
   // with DYNINSTgetCPUtime

   time64 result;
   prpsinfo_t theUsage;

   if (ioctl(proc_fd, PIOCPSINFO, &theUsage) == -1) {
      perror("could not read CPU time of inferior PIOCPSINFO");
      return 0;
   }
   result = PDYN_mulMillion(theUsage.pr_time.tv_sec); // sec to usec
   result += PDYN_div1000(theUsage.pr_time.tv_nsec);  // nsec to usec

   if (result<previous) {
     // time shouldn't go backwards, but we have seen this happening
     // before, so we better check it just in case - naim 5/30/97
     logLine("********* time going backwards in paradynd **********\n");
     result=previous;
   } else {
     previous=result;
   }

   return result;
}
#endif

void *process::getRegisters() {
   // Astonishingly, this routine can be shared between solaris/sparc and
   // solaris/x86.  All hail /proc!!!

   // assumes the process is stopped (/proc requires it)
   assert(status_ == stopped);

   prgregset_t theIntRegs;
   if (ioctl(proc_fd, PIOCGREG, &theIntRegs) == -1) {
      perror("process::getRegisters PIOCGREG");
      if (errno == EBUSY) {
         cerr << "It appears that the process was not stopped in the eyes of /proc" << endl;
	 assert(false);
      }

      return NULL;
   }

   prfpregset_t theFpRegs;
   if (ioctl(proc_fd, PIOCGFPREG, &theFpRegs) == -1) {
      perror("process::getRegisters PIOCGFPREG");
      if (errno == EBUSY)
         cerr << "It appears that the process was not stopped in the eyes of /proc" << endl;
      else if (errno == EINVAL)
	 // what to do in this case?  Probably shouldn't even do a print, right?
	 // And it certainly shouldn't be an error, right?
	 // But I wonder if any sparcs out there really don't have floating point.
	 cerr << "It appears that this machine doesn't have floating-point instructions" << endl;

      return NULL;
   }

   const int numbytesPart1 = sizeof(prgregset_t);
   const int numbytesPart2 = sizeof(prfpregset_t);
   assert(numbytesPart1 % 4 == 0);
   assert(numbytesPart2 % 4 == 0);

   void *buffer = new char[numbytesPart1 + numbytesPart2];
   assert(buffer);

   memcpy(buffer, &theIntRegs, sizeof(theIntRegs));
   memcpy((char *)buffer + sizeof(theIntRegs), &theFpRegs, sizeof(theFpRegs));

   return buffer;
}

bool process::executingSystemCall() {
   prstatus theStatus;
   if (ioctl(proc_fd, PIOCSTATUS, &theStatus) != -1) {
     if (theStatus.pr_syscall > 0) {
       inferiorrpc_cerr << "pr_syscall=" << theStatus.pr_syscall << endl;
       return(true);
     }
   } else assert(0);
   return(false);
}

bool process::changePC(unsigned addr, const void *savedRegs) {
   assert(status_ == stopped);

   prgregset_t theIntRegs = *(const prgregset_t *)savedRegs; // makes a copy, on purpose

   theIntRegs[R_PC] = addr; // PC (sparc), EIP (x86)
#ifdef R_nPC  // true for sparc, not for x86
   theIntRegs[R_nPC] = addr + 4;
#endif

   if (ioctl(proc_fd, PIOCSREG, &theIntRegs) == -1) {
      perror("process::changePC PIOCSREG failed");
      if (errno == EBUSY)
	 cerr << "It appears that the process wasn't stopped in the eyes of /proc" << endl;
      return false;
   }

   return true;
}

bool process::changePC(unsigned addr) {
   assert(status_ == stopped); // /proc will require this

   prgregset_t theIntRegs;
   if (-1 == ioctl(proc_fd, PIOCGREG, &theIntRegs)) {
      perror("process::changePC PIOCGREG");
      if (errno == EBUSY) {
	 cerr << "It appears that the process wasn't stopped in the eyes of /proc" << endl;
	 assert(false);
      }
      return false;
   }

   theIntRegs[R_PC] = addr;
#ifdef R_nPC
   theIntRegs[R_nPC] = addr + 4;
#endif

   if (-1 == ioctl(proc_fd, PIOCSREG, &theIntRegs)) {
      perror("process::changePC PIOCSREG");
      if (errno == EBUSY) {
	 cerr << "It appears that the process wasn't stopped in the eyes of /proc" << endl;
	 assert(false);
      }
      return false;
   }

   return true;
}

bool process::restoreRegisters(void *buffer) {
   // The fact that this routine can be shared between solaris/sparc and
   // solaris/x86 is just really, really cool.  /proc rules!

   assert(status_ == stopped); // /proc requires it

   prgregset_t theIntRegs = *(prgregset_t *)buffer;
   prfpregset_t theFpRegs = *(prfpregset_t *)((char *)buffer + sizeof(theIntRegs));

   if (ioctl(proc_fd, PIOCSREG, &theIntRegs) == -1) {
      perror("process::restoreRegisters PIOCSREG failed");
      if (errno == EBUSY) {
         cerr << "It appears that the process was not stopped in the eyes of /proc" << endl;
	 assert(false);
      }
      return false;
   }

   if (ioctl(proc_fd, PIOCSFPREG, &theFpRegs) == -1) {
      perror("process::restoreRegisters PIOCSFPREG failed");
      if (errno == EBUSY) {
         cerr << "It appears that the process was not stopped in the eyes of /proc" << endl;
         assert(false);
      }
      return false;
   }

   return true;
}

#ifdef i386_unknown_solaris2_5

bool process::readDataFromFrame(int currentFP, int *fp, int *rtn, bool )
{
  bool readOK=true;
  struct {
    int fp;
    int rtn;
  } addrs;

  //
  // for the x86, the frame-pointer (EBP) points to the previous frame-pointer,
  // and the saved return address is in EBP-4.
  //

  if (readDataSpace((caddr_t) (currentFP),
                    sizeof(int)*2, (caddr_t) &addrs, true)) {
    // this is the previous frame pointer
    *fp = addrs.fp;
    // return address
    *rtn = addrs.rtn;

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

#endif

#ifdef i386_unknown_solaris2_5
// ******** TODO **********
bool process::needToAddALeafFrame(Frame , Address &) {
  return false;
}

#else

// needToAddALeafFrame: returns true if the between the current frame 
// and the next frame there is a leaf function (this occurs when the 
// current frame is the signal handler and the function that was executing
// when the sighandler was called is a leaf function)
bool process::needToAddALeafFrame(Frame current_frame, Address &leaf_pc){

   // check to see if the current frame is the signal handler 
   Address frame_pc = current_frame.getPC();
   Address sig_addr = 0;
   const image *sig_image = (signal_handler->file())->exec();
   if(getBaseAddress(sig_image, sig_addr)){
       sig_addr += signal_handler->getAddress(0);
   } else {
       sig_addr = signal_handler->getAddress(0);
   }
   u_int sig_size = signal_handler->size();
   if(signal_handler&&(frame_pc >= sig_addr)&&(frame_pc < (sig_addr+sig_size))){
       // get the value of the saved PC: this value is stored in the address
       // specified by the value in register i2 + 44. Register i2 must contain
       // the address of some struct that contains, among other things, the 
       // saved PC value.  
       u_int reg_i2;
       int fp = current_frame.getFramePtr();
       if (readDataSpace((caddr_t)(fp+40),sizeof(u_int),(caddr_t)&reg_i2,true)){
          if (readDataSpace((caddr_t) (reg_i2+44), sizeof(int),
			    (caddr_t) &leaf_pc,true)){
	      // if the function is a leaf function return true
	      function_base *func = findFunctionIn(leaf_pc);
	      if(func && func->hasNoStackFrame()) { // formerly "isLeafFunc()"
		  return(true);
	      }
          }
      }
   }
   return false;
}
#endif

string process::tryToFindExecutable(const string &iprogpath, int pid) {
   // returns empty string on failure.
   // Otherwise, returns a full-path-name for the file.  Tries every
   // trick to determine the full-path-name, even though "progpath" may be
   // unspecified (empty string).
   
   // Remember, we can always return the empty string...no need to
   // go nuts writing the world's most complex algorithm.

   attach_cerr << "welcome to tryToFindExecutable; progpath=" << iprogpath << ", pid=" << pid << endl;

   const string progpath = expand_tilde_pathname(iprogpath);

   // Trivial case: if "progpath" is specified and the file exists then nothing needed
   if (exists_executable(progpath)) {
      attach_cerr << "tryToFindExecutable succeeded immediately, returning "
	          << progpath << endl;
      return progpath;
   }

   attach_cerr << "tryToFindExecutable failed on filename " << progpath << endl;

   char buffer[128];
   sprintf(buffer, "/proc/%05d", pid);
   int procfd = open(buffer, O_RDONLY, 0);
   if (procfd == -1) {
      attach_cerr << "tryToFindExecutable failed since open of /proc failed" << endl;
      return "";
   }
   attach_cerr << "tryToFindExecutable: opened /proc okay" << endl;

   string argv0, path, cwd;
   get_ps_stuff(procfd, argv0, path, cwd);

   // the following routine is implemented in the util lib.
   string result;
   if (executableFromArgv0AndPathAndCwd(result, argv0, path, cwd)) {
      (void)close(procfd);
      return result;
   }

   attach_cerr << "tryToFindExecutable: giving up" << endl;

   (void)close(procfd);
   return "";
}

bool process::set_breakpoint_for_syscall_completion() {
   /* Can assume: (1) process is paused and (2) in a system call.
      We want to set a TRAP for the syscall exit, and do the
      inferiorRPC at that time.  We'll use /proc PIOCSEXIT.
      Returns true iff breakpoint was successfully set. */

   sysset_t save_exitset;
   if (-1 == ioctl(proc_fd, PIOCGEXIT, &save_exitset))
      return false;

   sysset_t new_exit_set;
   prfillset(&new_exit_set);
   if (-1 == ioctl(proc_fd, PIOCSEXIT, &new_exit_set))
      return false;

   assert(save_exitset_ptr == NULL);
   save_exitset_ptr = new sysset_t;
   memcpy(save_exitset_ptr, &save_exitset, sizeof(save_exitset));

   return true;
}

unsigned process::read_inferiorRPC_result_register(reg) {
   prgregset_t theIntRegs;
   if (-1 == ioctl(proc_fd, PIOCGREG, &theIntRegs)) {
      perror("process::read_inferiorRPC_result_register PIOCGREG");
      if (errno == EBUSY) {
	 cerr << "It appears that the process was not stopped in the eyes of /proc" << endl;
	 assert(false);
      }
      return 0; // assert(false)?
   }

   // on x86, the result is always stashed in %EAX; on sparc, it's always %o0.  In
   // neither case do we need the argument of this fn.
#ifdef i386_unknown_solaris2_5
   return theIntRegs[EAX];
#else
   return theIntRegs[R_O0];
#endif
}

// hasBeenBound: returns true if the runtime linker has bound the
// function symbol corresponding to the relocation entry in at the address
// specified by entry and base_addr.  If it has been bound, then the callee 
// function is returned in "target_pdf", else it returns false.
bool process::hasBeenBound(const relocationEntry entry, 
			   pd_Function *&target_pdf, Address base_addr) {

// TODO: the x86 and sparc versions should really go in seperate files 
#if defined(i386_unknown_solaris2_5)

    if (status() == exited) return false;

    // if the relocationEntry has not been bound yet, then the value
    // at rel_addr is the address of the instruction immediately following
    // the first instruction in the PLT entry (which is at the target_addr) 
    // The PLT entries are never modified, instead they use an indirrect 
    // jump to an address stored in the _GLOBAL_OFFSET_TABLE_.  When the 
    // function symbol is bound by the runtime linker, it changes the address
    // in the _GLOBAL_OFFSET_TABLE_ corresponding to the PLT entry

    Address got_entry = entry.rel_addr() + base_addr;
    Address bound_addr = 0;
    if(!readDataSpace((const void*)got_entry, sizeof(Address), 
			&bound_addr, true)){
        sprintf(errorLine, "read error in process::hasBeenBound addr 0x%x, pid=%d\n",got_entry,pid);
	logLine(errorLine);
        return false;
    }

    if( !( bound_addr == (entry.target_addr()+6+base_addr)) ) {
        // the callee function has been bound by the runtime linker
	// find the function and return it
        target_pdf = findpdFunctionIn(bound_addr);
	if(!target_pdf){
            return false;
	}
        return true;	
    }
    return false;

#else
    // if the relocationEntry has not been bound yet, then the second instr 
    // in this PLT entry branches to the fist PLT entry.  If it has been   
    // bound, then second two instructions of the PLT entry have been changed 
    // by the runtime linker to jump to the address of the function.  
    // Here is an example:   
    //     before binding			after binding
    //	   --------------			-------------
    //     sethi  %hi(0x15000), %g1		sethi  %hi(0x15000), %g1
    //     b,a  <_PROCEDURE_LINKAGE_TABLE_>     sethi  %hi(0xef5eb000), %g1
    //	   nop					jmp  %g1 + 0xbc ! 0xef5eb0bc

    instruction next_insn;
    Address next_insn_addr = entry.target_addr() + base_addr + 4; 
    if( !(readDataSpace((caddr_t)next_insn_addr, sizeof(next_insn), 
		       (char *)&next_insn, true)) ) {
        sprintf(errorLine, "read error in process::hasBeenBound addr 0x%x\n",
		next_insn_addr);
	logLine(errorLine);
    }
    // if this is a b,a instruction, then the function has not been bound
    if((next_insn.branch.op == FMT2op)  && (next_insn.branch.op2 == BICCop2) 
       && (next_insn.branch.anneal == 1) && (next_insn.branch.cond == BAcond)) {
	return false;
    } 

    // if this is a sethi instruction, then it has been bound...get target_addr
    instruction third_insn;
    Address third_addr = entry.target_addr() + base_addr + 8; 
    if( !(readDataSpace((caddr_t)third_addr, sizeof(third_insn), 
		       (char *)&third_insn, true)) ) {
        sprintf(errorLine, "read error in process::hasBeenBound addr 0x%x\n",
		third_addr);
	logLine(errorLine);
    }

    // get address of bound function, and return the corr. pd_Function
    if((next_insn.sethi.op == FMT2op) && (next_insn.sethi.op2 == SETHIop2)
	&& (third_insn.rest.op == RESTop) && (third_insn.rest.i == 1)
	&& (third_insn.rest.op3 == JMPLop3)) {
        
	Address new_target = (next_insn.sethi.imm22 << 10) & 0xfffffc00; 
	new_target |= third_insn.resti.simm13;

        target_pdf = findpdFunctionIn(new_target);
	if(!target_pdf){
            return false;
	}
	return true;
    }
    // this is a messed up entry
    return false;
#endif

}



// findCallee: finds the function called by the instruction corresponding
// to the instPoint "instr". If the function call has been bound to an
// address, then the callee function is returned in "target" and the 
// instPoint "callee" data member is set to pt to callee's function_base.  
// If the function has not yet been bound, then "target" is set to the 
// function_base associated with the name of the target function (this is 
// obtained by the PLT and relocation entries in the image), and the instPoint
// callee is not set.  If the callee function cannot be found, (ex. function
// pointers, or other indirect calls), it returns false.
// Returns false on error (ex. process doesn't contain this instPoint).
//
// The assumption here is that for all processes sharing the image containing
// this instPoint they are going to bind the call target to the same function. 
// For shared objects this is always true, however this may not be true for
// dynamic executables.  Two a.outs can be identical except for how they are
// linked, so a call to fuction foo in one version of the a.out may be bound
// to function foo in libfoo.so.1, and in the other version it may be bound to 
// function foo in libfoo.so.2.  We are currently not handling this case, since
// it is unlikely to happen in practice.
bool process::findCallee(instPoint &instr, function_base *&target){
    
    if((target = (function_base *)instr.iPgetCallee())) {
 	return true; // callee already set
    }

    // find the corresponding image in this process  
    const image *owner = instr.iPgetOwner();
    bool found_image = false;
    Address base_addr = 0;
    if(symbols == owner) {  found_image = true; } 
    else if(shared_objects){
        for(u_int i=0; i < shared_objects->size(); i++){
            if(owner == ((*shared_objects)[i])->getImage()) {
		found_image = true;
		base_addr = ((*shared_objects)[i])->getBaseAddress();
		break;
            }
	}
    } 
    if(!found_image) {
        target = 0;
        return false; // image not found...this is bad
    }

    // get the target address of this function
    Address target_addr = 0;
    Address insn_addr = instr.iPgetAddress(); 
    target_addr = instr.getTargetAddress();

    if(!target_addr) {  
	// this is either not a call instruction or an indirect call instr
	// that we can't get the target address
        target = 0;
        return false;
    }

#if defined(sparc_sun_solaris2_4)
    // If this instPoint is from a function that was relocated to the heap
    // then need to get the target address relative to this image   
    if(target_addr && instr.relocated_) {
	assert(target_addr > base_addr);
	target_addr -= base_addr;
    }
#endif

    // see if there is a function in this image at this target address
    // if so return it
    pd_Function *pdf = 0;
    if( (pdf = owner->findFunctionIn(target_addr,this)) ) {
        target = pdf;
        instr.set_callee(pdf);
	return true; // target found...target is in this image
    }

    // else, get the relocation information for this image
    const Object &obj = owner->getObject();
    vector<relocationEntry> fbt;
    if(!obj.get_func_binding_table(fbt)) {
	target = 0;
	return false; // target cannot be found...it is an indirect call.
    }

    // find the target address in the list of relocationEntries
    for(u_int i=0; i < fbt.size(); i++) {
	if(fbt[i].target_addr() == target_addr) {
	    // check to see if this function has been bound yet...if the
	    // PLT entry for this function has been modified by the runtime
	    // linker
	    pd_Function *target_pdf = 0;
	    if(hasBeenBound(fbt[i], target_pdf, base_addr)) {
                target = target_pdf;
                instr.set_callee(target_pdf);
	        return true;  // target has been bound
	    } 
	    else {
		// just try to find a function with the same name as entry 
		target = findOneFunctionFromAll(fbt[i].name());
		if(target){
	            return true;
		}
		else {  
		    // KLUDGE: this is because we are not keeping more than
		    // one name for the same function if there is more
		    // than one.  This occurs when there are weak symbols
		    // that alias global symbols (ex. libm.so.1: "sin" 
		    // and "__sin").  In most cases the alias is the same as 
		    // the global symbol minus one or two leading underscores,
		    // so here we add one or two leading underscores to search
		    // for the name to handle the case where this string 
		    // is the name of the weak symbol...this will not fix 
		    // every case, since if the weak symbol and global symbol
		    // differ by more than leading underscores we won't find
		    // it...when we parse the image we should keep multiple
		    // names for pd_Functions

		    string s = string("_");
		    s += fbt[i].name();
		    target = findOneFunctionFromAll(s);
		    if(target){
	                return true;
		    }
		    s = string("__");
		    s += fbt[i].name();
		    target = findOneFunctionFromAll(s);
		    if(target){
	                return true;
		    }
		}
	    }
	    target = 0;
	    return false;
	}
    }
    target = 0;
    return false;  
}
