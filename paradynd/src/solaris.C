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

#include "symtab.h"
#include "util/h/headers.h"
#include "os.h"
#include "process.h"
#include "symtab.h"
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
#include <link.h>

extern "C" {
extern int ioctl(int, int, ...);
extern long sysconf(int);
};

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


/*
   Open the /proc file correspoding to process pid, 
   set the signals to be caught to be only SIGSTOP and SIGTRAP,
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

  /* we don't catch any child signals, except SIGSTOP (and, on sparc, SIGTRAP;
     on x86, SIGILL, to implement inferiorRPC) */
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

  /* turn on the kill-on-last-close and inherit-on-fork flags. This will cause
     the process to be killed when paradynd exits.
     Also, any child of this process will stop at the exit of an exec call.
     The breakpoint trap pc adjustment flag is used for the X86 platform.
  */
  long flags = PR_KLC | PR_FORK | PR_BPTADJ;
  if (ioctl (fd, PIOCSET, &flags) < 0) {
    fprintf(stderr, "attach: PIOCSET failed: %s\n", sys_errlist[errno]);
    close(fd);
    return false;
  }

  proc_fd = fd;
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
      && ((stat.pr_what == SIGSTOP) || (stat.pr_what == SIGINT))) {
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
//  cerr << "writeTextWord @ " << (void *)inTraced << endl; cerr.flush();
  return writeDataSpace_(inTraced, sizeof(int), (caddr_t) &data);
}

bool process::writeTextSpace_(void *inTraced, int amount, const void *inSelf) {
//  cerr << "writeTextSpace pid=" << getPid() << ", @ " << (void *)inTraced << " len=" << amount << endl; cerr.flush();
  return writeDataSpace_(inTraced, amount, inSelf);
}

bool process::writeDataSpace_(void *inTraced, int amount, const void *inSelf) {
  ptraceOps++; ptraceBytes += amount;

//  cerr << "process::writeDataSpace_ pid " << getPid() << " writing " << amount << " bytes at loc " << inTraced << endl;

  if (lseek(proc_fd, (off_t)inTraced, SEEK_SET) != (off_t)inTraced)
    return false;
  return (write(proc_fd, inSelf, amount) == amount);
}

bool process::readDataSpace_(const void *inTraced, int amount, void *inSelf) {
  ptraceOps++; ptraceBytes += amount;
  if (lseek(proc_fd, (off_t)inTraced, SEEK_SET) != (off_t)inTraced) {
    printf("error in lseek \n");
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
  pdFunction *func;
  int pc = *rtn;

  if (uppermost) {
      func = this->findFunctionIn(pc);
      if (func) {
	  if (func ->isLeafFunc()) {
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
unsigned long long process::getInferiorProcessCPUtime() const {
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

   prusage_t theUsage;
   if (ioctl(proc_fd, PIOCUSAGE, &theUsage) == -1) {
      perror("could not read CPU time of inferior PIOCUSAGE");
      return 0;
   }

   timestruc_t &utime = theUsage.pr_utime;
   timestruc_t &stime = theUsage.pr_stime;

   // Note: we can speed up the multiplication and division; see RTsolaris.c in rtinst

   unsigned long long result = utime.tv_sec + stime.tv_sec;
   result *= 1000000; // sec to usec
   result += utime.tv_nsec/1000 + stime.tv_nsec/1000;

   return result;
}
#endif

void *process::getRegisters(bool &) {
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

bool process::changePC(unsigned addr, void *savedRegs) {
   assert(status_ == stopped);

   prgregset_t theIntRegs = *(prgregset_t *)savedRegs; // makes a copy, on purpose

   theIntRegs[R_PC] = addr; // EIP register
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
	      pdFunction *func = findFunctionIn(leaf_pc);
	      if(func && func->isLeafFunc()) {
		  return(true);
	      }
          }
      }
   }
   return false;
}

