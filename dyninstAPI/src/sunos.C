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
 * Revision 1.34  1997/07/17 16:53:07  buck
 * Eliminated the need to link dyninst API with -lkvm on SunOS,
 * and added a check for the failure of the "attach" constructor for
 * process.
 *
 * Revision 1.33  1997/07/16 19:24:08  naim
 * Minor change to my previous commit - naim
 *
 * Revision 1.32  1997/07/16 19:13:48  naim
 * Fixing fork on sunos - naim
 *
 * Revision 1.31  1997/07/09 19:30:32  tamches
 * isLeafFunc() --> hasNoStackFrame()
 *
 * Revision 1.30  1997/07/08 19:15:18  buck
 * Added support for the x86 Solaris platform and dynamically linked
 * executables to the dyninst API library.
 *
 * Revision 1.29  1997/07/01 16:54:56  tamches
 * dummy set_breakpoint_for_syscall_completion
 *
 * Revision 1.28  1997/06/23 19:16:03  buck
 * Added features to the dyninst API library, including an optional "else"
 * in a BPatch_ifExpr; the BPatch_setMutationsActive call to temporarily
 * disable all snippets; and the replaceFunctionCall and removeFunctionCall
 * member functions of BPatch_thread to retarget or NOOP out a function
 * call.
 *
 * Revision 1.1.1.3  1997/06/11 17:33:02  buck
 * Update Maryland repository with latest changes from Wisconsin.
 *
 * Revision 1.27  1997/05/16 18:48:26  naim
 * Fixing problem when inferiorRPC was launched and the application was in
 * the middle of a system call - naim
 *
 * Revision 1.26  1997/04/14 00:22:26  newhall
 * removed class pdFunction and replaced it with base class function_base and
 * derived class pd_Function
 *
 * Revision 1.25  1997/03/18 19:44:25  buck
 * first commit of dyninst library.  Also includes:
 * 	moving templates from paradynd to dyninstAPI
 * 	converting showError into a function (in showerror.C)
 * 	many ifdefs for BPATCH_LIBRARY in dyinstAPI/src.
 *
 * Revision 1.24  1997/02/26 23:43:03  mjrg
 * First part on WindowsNT port: changes for compiling with Visual C++;
 * moved unix specific code to unix.C
 *
 * Revision 1.23  1997/02/21 20:13:53  naim
 * Moving files from paradynd to dyninstAPI + moving references to dataReqNode
 * out of the ast class. The is the first pre-dyninstAPI commit! - naim
 *
 * Revision 1.22  1997/01/30 18:14:12  tamches
 * skeleton isRunning, tryToFindExecutable, and read_inferiorRPC_result_register
 *
 * Revision 1.21  1996/11/23 22:46:41  lzheng
 * Finished the implementation of inferiorPRC on HPUX platfrom
 *
 * Revision 1.20  1996/11/19 16:28:24  newhall
 * Fix to stack walking on Solaris: find leaf functions in stack (these can occur
 * on top of stack or in middle of stack if the signal handler is on the stack)
 * Fix to relocated functions: new instrumentation points are kept on a per
 * process basis.  Cleaned up some of the code.
 *
 * Revision 1.19  1996/11/05 20:35:34  tamches
 * changed some OS:: methods to process:: methods
 *
 * Revision 1.18  1996/10/31 08:55:11  tamches
 * the shm-sampling commit; routines to do inferiorRPC; removed some warnings.
 *
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


#include "dyninstAPI/src/process.h"

extern "C" {
extern int ioctl(int, int, ...);
extern int getrusage(int, struct rusage*);
#include <a.out.h>
#include <sys/exec.h>
#include <stab.h>
extern struct rusage *mapUarea();

#include <machine/reg.h> // for ptrace_getregs call
};

#include <sys/ioctl.h>
#include <fcntl.h>
#include <machine/reg.h>
#include <sys/user.h> // for u-area stuff

#include "dyninstAPI/src/symtab.h"
#include "util/h/headers.h"
#include "dyninstAPI/src/os.h"
#include "dyninstAPI/src/stats.h"
#include "util/h/Types.h"
#include "paradynd/src/showerror.h"
#include "dyninstAPI/src/util.h" // getCurrWallTime
#include "util/h/pathName.h"

#ifdef SHM_SAMPLING
#include <kvm.h>
#include <sys/user.h>
#endif

// #include <sys/termios.h>

extern bool isValidAddress(process *proc, Address where);

extern struct rusage *mapUarea();

/* ********************************************************************** */

class ptraceKludge {
private:
  static bool haltProcess(process *p);
  static void continueProcess(process *p, const bool halted);

public:
  static bool deliverPtrace(process *p, enum ptracereq req, void *addr,
			    int data, void *addr2);
//  static bool deliverPtraceFast(process *p, enum ptracereq req, void *addr,
//				int data, void *addr2);
//      // like deliverPtrace() but assumes process is paused.
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

void ptraceKludge::continueProcess(process *p, const bool wasStopped) {
  // First handle the cases where we shouldn't issue a PTRACE_CONT:
  if (p->status() == neonatal) return;
  if (wasStopped) return;

  // Choose either one of the following methods to continue a process.
  // The choice must be consistent with that in process::continueProc_ and stop_

#ifndef PTRACE_ATTACH_DETACH
  if (P_ptrace(PTRACE_CONT, p->pid, (caddr_t) 1, SIGCONT, NULL) == -1) {
#else
  if (P_ptrace(PTRACE_DETACH, p->pid, (caddr_t) 1, SIGCONT, NULL) == -1) {
#endif
      perror("error in continueProcess");
      assert(0);
  }
}

bool ptraceKludge::deliverPtrace(process *p, enum ptracereq req, void *addr,
				 int data, void *addr2) {
  bool halted;

  if (req != PTRACE_DETACH)
     halted = haltProcess(p);

  bool ret = (P_ptrace(req, p->getPid(), (char*)addr, data, (char*)addr2) != -1);

  if (req != PTRACE_DETACH)
     continueProcess(p, halted);

  return ret;
}

/* ********************************************************************** */

void *process::getRegisters() {
   // ptrace - GETREGS call
   // assumes the process is stopped (ptrace requires it)
   assert(status_ == stopped);

   // See <machine/reg.h> and <machine/fp.h>
   const int numbytesPart1 = sizeof(struct regs);
   const int numbytesPart2 = sizeof(struct fp_status);
   assert(numbytesPart1 % 4 == 0);
   assert(numbytesPart2 % 4 == 0);

   void *buffer = new char[numbytesPart1 + numbytesPart2];
   assert(buffer);

   struct regs theIntRegs;
   int result = P_ptrace(PTRACE_GETREGS, pid, (char*)&theIntRegs, 0, 0);
   assert(result != -1);

   struct fp_status theFpRegs;
   result = P_ptrace(PTRACE_GETFPREGS, pid, (char*)&theFpRegs, 0, 0);
   assert(result != -1);

   memcpy(buffer, &theIntRegs, sizeof(theIntRegs));
   memcpy((char*)buffer + sizeof(theIntRegs), &theFpRegs, sizeof(theFpRegs));

   return buffer;
}

static bool changePC(int pid, struct regs &theIntRegs, unsigned loc) {
   assert(loc % 4 == 0);

   theIntRegs.r_pc = loc;
   theIntRegs.r_npc = loc+4;

   if (0 != P_ptrace(PTRACE_SETREGS, pid, (char*)&theIntRegs, 0, 0)) {
      cerr << "process::changePC failed" << endl;
      return false;
   }

   return true;
}

bool process::executingSystemCall() {
   // this is not implemented yet - naim 5/15/97
   return false;
}

bool process::changePC(unsigned loc, const void *savedRegs) {
   assert(status_ == stopped);
   struct regs theIntRegs = *(const struct regs*)savedRegs; // makes a copy (on purpose)

   return ::changePC(pid, theIntRegs, loc);
}

bool process::changePC(unsigned loc) {
   assert(status_ == stopped);
   struct regs theIntRegs;
   int result = P_ptrace(PTRACE_GETREGS, pid, (char*)&theIntRegs, 0, 0);
   assert(result != -1);

   return ::changePC(pid, theIntRegs, loc);
}


bool process::restoreRegisters(void *buffer) {
   // two ptrace - SETREGS calls
   // assumes process is stopped (ptrace requires it)
   assert(status_ == stopped);

   struct regs theIntRegs = *(struct regs *)buffer;
   struct fp_status theFpRegs = *(struct fp_status *)((char *)buffer + sizeof(struct regs));

   int result = P_ptrace(PTRACE_SETREGS, pid, (char *)&theIntRegs, 0, 0);
   if (result == -1) {
      perror("process::restoreRegisters PTRACE_SETREGS failed");
      return false;
   }

   result = P_ptrace(PTRACE_SETFPREGS, pid, (char *)&theFpRegs, 0, 0);
   if (result == -1) {
      perror("process::restoreRegisters PTRACE_SETFPREGS failed");
      return false;
   }

   return true;
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

// TODO: implement this
bool process::needToAddALeafFrame(Frame,unsigned int &){
    return false;
}

bool process::readDataFromFrame(int currentFP, int *fp, int *rtn, bool uppermost)
{
  bool readOK=true;
  struct {
    int fp;
    int rtn;
  } addrs;

  pd_Function *func;
  int pc = *rtn;
  struct regs regs; 

  if (uppermost) {
      func = symbols -> findFunctionIn(pc,this);
      if (func) {
	  if (func ->hasNoStackFrame()) {
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

// already setup on this FD.
// disconnect from controlling terminal 
void OS::osDisconnect(void) {
  int ttyfd = open ("/dev/tty", O_RDONLY);
  ioctl (ttyfd, TIOCNOTTY, NULL); 
  P_close (ttyfd);
}

bool process::stop_() {
   // formerly OS::osStop()

/* Choose either one of the following methods for stopping a process, but not both. 
 * The choice must be consistent with that in process::continueProc_ 
 * and ptraceKludge::continueProcess
 */

#ifndef PTRACE_ATTACH_DETACH
	return (P_kill(pid, SIGSTOP) != -1); 
#else
	return attach_();
#endif
}

bool process::continueWithForwardSignal(int sig) {
   // formerly OS::osForwardSignal
   return (P_ptrace(PTRACE_CONT, pid, (char*)1, sig, 0) != -1);
}

void OS::osTraceMe(void) { P_ptrace(PTRACE_TRACEME, 0, 0, 0, 0); }


// wait for a process to terminate or stop
int process::waitProcs(int *status) {
  return waitpid(0, status, WNOHANG);
}

// attach to an inferior process.
bool process::attach() {
  // we only need to attach to a process that is not our direct children.
  if (parent != 0)
    return attach_();
  else
    return true;
}

bool process::attach_() {
   return (P_ptrace(PTRACE_ATTACH, getPid(), 0, 0, 0) != -1);
}

bool process::isRunning_() const {
   // determine if a process is running by doing low-level system checks, as
   // opposed to checking the 'status_' member vrble.  May assume that attach()
   // has run, but can't assume anything else.

   assert(false); // not yet implemented!   
}


// TODO is this safe here ?
bool process::continueProc_() {
  int ret;

  if (!checkStatus()) 
    return false;

  ptraceOps++; ptraceOtherOps++;

/* choose either one of the following ptrace calls, but not both. 
 * The choice must be consistent with that in stop_ and
 * ptraceKludge::continueProcess.
 */
#ifndef PTRACE_ATTACH_DETACH
  if (!ptraceKludge::deliverPtrace(this, PTRACE_CONT, (char*)1, 0, NULL))
    ret = -1;
  else
    ret = 0;
  //ret = P_ptrace(PTRACE_CONT, pid, (char*)1, 0, (char*)NULL);
#else
  ret = P_ptrace(PTRACE_DETACH, pid, (char*)1, SIGCONT, (char*)NULL);
#endif

  if (ret == -1)
     perror("continueProc_()");

  return ret != -1;
}

#ifdef BPATCH_LIBRARY
bool process::terminateProc_()
{
  if (!checkStatus()) 
    return false;

  if (P_ptrace(PTRACE_KILL, pid, NULL, NULL, NULL) != 0)
    return false;
  else
    return true;
}
#endif

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

#ifdef BPATCH_LIBRARY
bool process::API_detach_(const bool cont) {
//  assert(cont);
  if (!checkStatus())
    return false;
  ptraceOps++; ptraceOtherOps++;
  if (!cont) P_kill(pid, SIGSTOP);
  return (ptraceKludge::deliverPtrace(this, PTRACE_DETACH, (char*)1, SIGCONT, NULL));
}
#endif

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

  return (ret != -1);
}

bool process::writeTextWord_(caddr_t inTraced, int data) {
  if (!checkStatus()) 
    return false;
  ptraceBytes += sizeof(int); ptraceOps++;

//  cerr << "writeTextWord @ " << (void *)inTraced << endl; cerr.flush();

  return (ptraceKludge::deliverPtrace(this, PTRACE_POKETEXT, inTraced, data, NULL));
}

bool process::writeTextSpace_(void *inTraced, int amount, const void *inSelf) {
  if (!checkStatus()) 
    return false;
  ptraceBytes += amount; ptraceOps++;

//  cerr << "writeTextSpace pid=" << getPid() << ", @ " << (void *)inTraced << " len=" << amount << endl; cerr.flush();

  return (ptraceKludge::deliverPtrace(this, PTRACE_WRITETEXT, inTraced, amount, (void *)inSelf));
}

#ifdef BPATCH_SET_MUTATIONS_ACTIVE
bool process::readTextSpace_(void *inTraced, int amount, const void *inSelf) {
  bool result;
  if (!checkStatus())
    result = false;
  else {
     ptraceOps++; ptraceBytes += amount;
     result = ptraceKludge::deliverPtrace(this, PTRACE_READTEXT, 
					(void *)inTraced, amount, inSelf);
  }

  return result;
}
#endif

bool process::writeDataSpace_(void *inTraced, int amount, const void *inSelf) {
  if (!checkStatus())
    return false;

  ptraceOps++; ptraceBytes += amount;

//  cerr << "process::writeDataSpace_ pid " << getPid() << " writing " << amount << " bytes at loc " << inTraced << endl;

  bool result = ptraceKludge::deliverPtrace(this, PTRACE_WRITEDATA, inTraced, amount, (void*)inSelf);

  return result;
}

bool process::readDataSpace_(const void *inTraced, int amount, void *inSelf) {
  bool result;
  if (!checkStatus())
    result = false;
  else {
     ptraceOps++; ptraceBytes += amount;
     result = ptraceKludge::deliverPtrace(this, PTRACE_READDATA, 
					(void *)inTraced, amount, inSelf);
  }

  return result;
}

#ifdef SHM_SAMPLING
time64 process::getInferiorProcessCPUtime() const {
   // kvm_getproc returns a ptr to a _copy_ of the proc structure
   // in static memory.
//   time64 wall1 = getCurrWallTime();
   proc *p = kvm_getproc(kvmHandle, getPid());
   if (p == NULL) {
      perror("could not getInferiorProcessCPUtime because kvm_getproc failed");
      exit(5);
   }
//   time64 wall2 = getCurrWallTime();
//   time64 difference = wall2-wall1;
//   unsigned long difference_long = difference;
//   cout << "took " << difference_long << " usecs to kvm_getproc" << endl;

   // kvm_getu returns a copy to a _copy_ of the process' uarea
   // in static memory.
   user *u = kvm_getu(kvmHandle, p);
   if (u == NULL) {
      perror("could not kvm_getu()");
      exit(5);
   }

   return userAndSysTime2uSecs(u->u_ru.ru_utime,
			       u->u_ru.ru_stime);

   assert(false);




   
   if (childUareaPtr == NULL) {
      cout << "cannot read inferior proc cpu time since unmapped...try to implement kvm_getproc instead" << endl;
      return 0;
   }

   static time64 prevResult = 0;  // to check for rollback

   while (true) {
      time64 result = userAndSysTime2uSecs(childUareaPtr->u_ru.ru_utime,
                                                       childUareaPtr->u_ru.ru_stime);
      if (result < prevResult) {
         cout << "process::getInferiorProcessCPUtime() retrying due to rollback!" << endl;
         continue;
      }
      else {
cout << "sunos done getting virtual time." << endl; cout.flush();
         prevResult = result;
         return result;
      }
   }
}

user *process::tryToMapChildUarea(int childpid) {
   // a static member fn
   // see DYNINSTprobeUarea of rtinst/src/sunos.c

assert(0);

   kvm_t *kvmfd = kvm_open(0, 0, 0, O_RDONLY, 0);
   if (kvmfd == NULL) {
      perror("could not map child's uarea because kvm_open failed");
      return NULL;
   }

   // kvm_getproc returns a ptr to a _copy_ of the proc structure
   // in static memory.
   time64 wall1 = getCurrWallTime();
   proc *p = kvm_getproc(kvmfd, childpid);
   if (p == NULL) {
      perror("could not map child's uarea because kvm_getproc failed");
      return NULL;
   }
   time64 wall2 = getCurrWallTime();
   time64 difference = wall2-wall1;
   unsigned long difference_long = difference;
   cout << "took " << difference_long << " usecs to kvm_getproc" << endl;

   // kvm_getu returns a copy to a _copy_ of the process' uarea
   // in static memory.
   user *u = kvm_getu(kvmfd, p);
   if (u == NULL) {
      perror("could not map child's uarea because kvm_getu failed");
      return NULL;
   }

   kvm_close(kvmfd);

   void *uareaPtr = p->p_uarea;

   int kmemfd = open("/dev/kmem", O_RDONLY, 0);
   if (kmemfd == -1) {
      perror("could not map child's uarea because could not open /dev/kmem for reading");
      return NULL;
   }

   void *result = P_mmap(NULL, sizeof(user), PROT_READ, MAP_SHARED, kmemfd,
                         (off_t)uareaPtr);
   if (result == (void *)-1) {
      perror("could not map child's uarea because could not mmap /dev/kmem");
      close(kmemfd);
      return NULL;
   }

   cout << "mmap of child's uarea succeeded!" << endl;

   return (user *)result;
}
#endif

//time64 process::getInferiorProcessCPUtime() const {
//   // We get the inferior process's cpu time via a ptrace() of the u-area
//   // of the inferior process, though it should be possible to mmap()
//   // the inferior process's uarea into paradynd if we really want to...
//
//   // UH OH: this will only work if the inferior process has been stopped!!!
//
//   static time64 prevResult = 0;
//
//   while (true) {
//      // assumes child process has been stopped
//      user childUareaPtr; // just a dummy
//      unsigned addrOffset = (void *)&childUareaPtr.u_ru.ru_utime - (void *)&childUareaPtr;
//      unsigned numBytesNeeded = (void *)&childUareaPtr.u_ru.ru_maxrss -
//                                (void *)&childUareaPtr.u_ru.ru_utime;
//      assert(numBytesNeeded % 4 == 0);
//
//      rusage theUsage; // we'll write into the first few bytes of this structure
//      void *ptr = &theUsage.ru_utime;
//
//      cout << "peeking from uarea for pid " << this->getPid() << endl; cout.flush();
//      while (numBytesNeeded) {
//         errno = 0;
//         unsigned result = P_ptrace(PTRACE_PEEKUSER, this->getPid(),
//				    (char*)addrOffset, 0, NULL);
//         if (errno != 0) {
//            perror("could not getChildCPUtimeViaPtraceOfUarea: ptrace()");
//            exit(5);
//         }
//
//         memcpy(ptr, &result, 4);
//         ptr += 4;
//         addrOffset += 4;
//         numBytesNeeded -= 4;
//      }
//
//      time64 result = userAndSysTime2uSecs(theUsage.ru_utime, theUsage.ru_stime);
//      if (result < prevResult) {
//         cout << "process::getInferiorProcessCPUtime() retrying due to rollback!" << endl;
//         continue;
//      }
//      else {
//         prevResult = result;
//         return result;
//      }
//   }
//}

bool process::loopUntilStopped() {
  /* make sure the process is stopped in the eyes of ptrace */
  stop_(); // sends SIGSTOP signal to the process

  while (true) {
    int waitStatus;
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
        break; // success
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

#ifdef BPATCH_LIBRARY
bool process::dumpImage() { return false; }
#else
bool process::dumpImage() {
  const string &imageFileName = symbols->file();
  const Address codeOff = symbols->codeOffset();

  u_int i;
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
  }

  rd = P_read(ifd, (void *) &my_exec, sizeof(struct exec));
  if (rd != sizeof(struct exec)) {
    string msg = string("Dump core failed: read failed '") + imageFileName 
                 + string("': ") + string(sys_errlist[errno]);
    showErrorCallback(47, msg);
    P_close(ifd);
    return false;
  }

  rd = P_fstat(ifd, &statBuf);
  if (rd != 0) {
    string msg = string("Dump core failed: fstat failed: ") + string(sys_errlist[errno]);
    showErrorCallback(47, msg);
    P_close(ifd);
    return false;
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
    stop_();
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
#endif /* BPATCH_LIBRARY */

#ifdef BPATCH_LIBRARY
float OS::compute_rusage_cpu() { return 0; }

float OS::compute_rusage_sys() { return 0; }

float OS::compute_rusage_min() { return 0; }

float OS::compute_rusage_maj() { return 0; }

float OS::compute_rusage_swap() { return 0; }

float OS::compute_rusage_io_in() { return 0; }

float OS::compute_rusage_io_out() { return 0; }

float OS::compute_rusage_msg_send() { return 0; }

float OS::compute_rusage_msg_recv() { return 0; }

float OS::compute_rusage_sigs() { return 0; }

float OS::compute_rusage_vol_cs() { return 0; }

float OS::compute_rusage_inv_cs() { return 0; }

#else /* BPATCH_LIBRARY */
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
#endif /* BPATCH_LIBRARY */

int getNumberOfCPUs()
{
  return(1);
}

string process::tryToFindExecutable(const string &progpath, int pid) {
   // returns empty string on failure

   if (progpath.length() == 0)
      return "";

   if (exists_executable(progpath))
      return progpath;

   return ""; // failure
}

unsigned process::read_inferiorRPC_result_register(reg) {
   // on sparc, the result register is always in o0, so no need to use the input arg.
   struct regs regs;
   if (!ptraceKludge::deliverPtrace(this, PTRACE_GETREGS, (char*)&regs, 0, 0))
      assert(false);
   return regs.r_o0;
}

bool process::set_breakpoint_for_syscall_completion() {
   // SUNos can't do this (as far as I know)
   return false;
}
