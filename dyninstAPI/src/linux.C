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

// $Id: linux.C,v 1.51 2001/10/11 23:57:59 schendel Exp $

#include <fstream.h>

#include "dyninstAPI/src/process.h"

#include <sys/ptrace.h>
#include <asm/ptrace.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <dlfcn.h>
#include <sys/user.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <math.h> // for floor()
#include <unistd.h> // for sysconf()

#include "dyninstAPI/src/symtab.h"
#include "dyninstAPI/src/instPoint.h"
#include "common/h/headers.h"
#include "dyninstAPI/src/os.h"
#include "dyninstAPI/src/stats.h"
#include "common/h/Types.h"
#include "dyninstAPI/src/showerror.h"
#include "dyninstAPI/src/util.h" // getCurrWallTime
#include "common/h/pathName.h"
#include "dyninstAPI/src/inst-x86.h"
#ifndef BPATCH_LIBRARY
#include "common/h/Time.h"
#include "common/h/timing.h"
#include "paradynd/src/init.h"
#endif

#ifdef HRTIME
#include "rtinst/h/RThwtimer-linux.h"
#endif

#define DLOPEN_MODE (RTLD_NOW | RTLD_GLOBAL)

// The following were defined in process.C
extern debug_ostream attach_cerr;
extern debug_ostream inferiorrpc_cerr;
extern debug_ostream shmsample_cerr;
extern debug_ostream forkexec_cerr;
extern debug_ostream signal_cerr;

extern bool isValidAddress(process *proc, Address where);
extern void generateBreakPoint(instruction &insn);


const char DYNINST_LOAD_HIJACK_FUNCTIONS[][15] = {
  "main",
  "_init",
  "_start"
};
const int N_DYNINST_LOAD_HIJACK_FUNCTIONS = 3;

const char DL_OPEN_FUNC_NAME[] = "_dl_open";

const char libc_version_symname[] = "__libc_version";


#if defined(PTRACEDEBUG) && !defined(PTRACEDEBUG_ALWAYS)
static bool debug_ptrace = false;
#endif

#ifndef _SYS_USER_H
struct user_regs_struct
{
  long ebx;
  long ecx;
  long edx;
  long esi;
  long edi;
  long ebp;
  long eax;
  long xds;
  long xes;
  long xfs;
  long xgs;
  long orig_eax;
  long eip;
  long xcs;
  long eflags;
  long esp;
  long xss;
};
#endif

static int regmap[] = 
{
    EBX, ECX, EDX, ESI,
    EDI, EBP, EAX, DS,
    ES, FS, GS, ORIG_EAX,
    EIP, CS, EFL, UESP,
    SS
/*
  EAX, ECX, EDX, EBX,
  UESP, EBP, ESI, EDI,
  EIP, EFL, CS, SS,
  DS, ES, FS, GS,
  ORIG_EAX
*/
};

#define NUM_REGS (17 /*+ NUM_FREGS*/)
#define NUM_FREGS 8
#define FP0_REGNUM NUM_REGS
#define FP7_REGNUM (FP0_REGNUM+7)
#define INTREGSIZE (sizeof(int))
#define FPREGSIZE 10
#define MAX_REGISTER_RAW_SIZE 10

#define REGISTER_RAW_SIZE(N) (((N) < FP0_REGNUM) ? INTREGSIZE : FPREGSIZE)
#define REGS_SIZE ( NUM_REGS * REGISTER_RAW_SIZE(0) + NUM_FREGS * REGISTER_RAW_SIZE(FP0_REGNUM) )
#define REGS_INTS ( REGS_SIZE / INTREGSIZE )

const int GENREGS_STRUCT_SIZE = sizeof( user_regs_struct );
#ifdef _SYS_USER_H 
const int FPREGS_STRUCT_SIZE = sizeof( user_fpregs_struct );
#else
const int FPREGS_STRUCT_SIZE = sizeof( user_i387_struct );
#endif

int register_addr (int regno )
{
  int addr;

  if ( (regno < 0 || regno >= NUM_REGS)
       && (regno < FP0_REGNUM || regno > FP7_REGNUM) )
    {
      fprintf ( stderr, "Invalid register number %d.", regno);
      assert(0);
      return -1;
    }

  if (regno >= FP0_REGNUM && regno <= FP7_REGNUM) 
    {
      int fpstate;
      struct user *u = NULL;
      fpstate = (int)(&u->i387.st_space);
      addr = fpstate + 10 * (regno - FP0_REGNUM);
    }
  else
    addr = INTREGSIZE * regmap[regno];

  return addr;
}

class ptraceKludge {
private:
  static bool haltProcess(process *p);
  static void continueProcess(process *p, const bool halted);

public:
  static bool deliverPtrace(process *p, int req, int addr, int data);
  static int deliverPtraceReturn(process *p, int req, int addr, int data);
};

bool ptraceKludge::haltProcess(process *p) {
  bool wasStopped = (p->status() == stopped);
  if (p->status() != neonatal && !wasStopped) {
    if (!p->loopUntilStopped()) {
      cerr << "error in loopUntilStopped\n";
      assert(0);
    }
  }  return wasStopped;
}

void ptraceKludge::continueProcess(process *p, const bool wasStopped) {
  // First handle the cases where we shouldn't issue a PTRACE_CONT:
  if (p->status() == neonatal) return;
  if (wasStopped) return;

  // Choose either one of the following methods to continue a process.
  // The choice must be consistent with that in process::continueProc_ and stop_

#ifndef PTRACE_ATTACH_DETACH
  if (P_ptrace(PTRACE_CONT, p->pid, 1, SIGCONT) == -1) {
#else
  if (P_ptrace(PTRACE_DETACH, p->pid, 1, SIGCONT) == -1) {
#endif
      perror("error in continueProcess");
      assert(0);
  }
}

bool ptraceKludge::deliverPtrace(process *p, int req, int addr,
				 int data ) {
  bool halted = true;

//  if (req != PTRACE_DETACH)
     halted = haltProcess(p);

  bool ret = (P_ptrace(req, p->getPid(), addr, data) != -1);

//  if (req != PTRACE_DETACH)
     continueProcess(p, halted);

  return ret;
}


// Some ptrace requests in Linux return the value rather than storing at the address in data
// (especially PEEK requests)
// - nash
int ptraceKludge::deliverPtraceReturn(process *p, int req, int addr,
				 int data ) {
#ifdef DETACH_ON_THE_FLY
  bool detached = p->haveDetached;
  if (detached) {
       p->reattach();
  }
  int ret = P_ptrace(req, p->getPid(), addr, data);
  if (detached) {
       p->detach();
  }
  return ret;
#else
  bool halted = true;

  if (req != PTRACE_DETACH)
     halted = haltProcess(p);

  int ret = P_ptrace(req, p->getPid(), addr, data);

  if (req != PTRACE_DETACH)
     continueProcess(p, halted);

  return ret;
#endif
}

/* ********************************************************************** */

void *process::getRegisters() {
   // assumes the process is stopped (ptrace requires it)
   assert(status_ == stopped);

   // Cycle through all registers, reading each from the
   // process user space with ptrace(PTRACE_PEEKUSER ...

   char *buf = new char [ GENREGS_STRUCT_SIZE + FPREGS_STRUCT_SIZE ];
   int error;
   bool errorFlag = false;
   error = P_ptrace( PTRACE_GETREGS, pid, 0, (int)(buf) );
   if( error ) {
       perror("process::getRegisters PTRACE_GETREGS" );
       errorFlag = true;
   } else {
       error = P_ptrace( PTRACE_GETFPREGS, pid, 0, (int)(buf + GENREGS_STRUCT_SIZE) );
       if( error ) {
	   perror("process::getRegisters PTRACE_GETFPREGS" );
	   errorFlag = true;
       }
   }

   if( errorFlag )
       return NULL;
   else
       return (void*)buf;
}

static bool changePC(int pid, Address loc) {
  Address regaddr = EIP * INTREGSIZE;
  if (0 != P_ptrace (PTRACE_POKEUSER, pid, regaddr, loc )) {
    perror( "process::changePC - PTRACE_POKEUSER" );
    return false;
  }

  return true;
}

static bool changeBP(int pid, Address loc) {
  Address regaddr = EBP * INTREGSIZE;
  if (0 != P_ptrace (PTRACE_POKEUSER, pid, regaddr, loc )) {
    perror( "process::changeBP - PTRACE_POKEUSER" );
    return false;
  }

  return true;
}

void printStackWalk( process *p ) {
	Frame theFrame(p);
	while (true) {
		// do we have a match?
		const Address framePC = theFrame.getPC();
		inferiorrpc_cerr << "stack frame pc @ " << (void*)framePC << endl;
		
		if (theFrame.isLastFrame())
			// well, we've gone as far as we can, with no match.
			break;
		
		// else, backtrace 1 more level
		theFrame = theFrame.getCallerFrame(p);
	}
}
 
void printRegs( void *save ) {
	user_regs_struct *regs = (user_regs_struct*)save;
	cerr
		<< " eax: " << (void*)regs->eax
		<< " ebx: " << (void*)regs->ebx
		<< " ecx: " << (void*)regs->ecx
		<< " edx: " << (void*)regs->edx << endl
		<< " edi: " << (void*)regs->edi
		<< " esi: " << (void*)regs->esi << endl
		<< " xcs: " << (void*)regs->xcs
		<< " xds: " << (void*)regs->xds
		<< " xes: " << (void*)regs->xes
		<< " xfs: " << (void*)regs->xfs
		<< " xgs: " << (void*)regs->xgs
		<< " xss: " << (void*)regs->xss << endl
		<< " eip: " << (void*)regs->eip
		<< " esp: " << (void*)regs->esp
		<< " ebp: " << (void*)regs->ebp << endl
		<< " orig_eax: " << (void*)regs->orig_eax
		<< " eflags: " << (void*)regs->eflags << endl;
}

bool process::executingSystemCall() {
	// From the program strace, it appears that a non-negative number
	// in the ORIG_EAX register of the inferior process indicates
	// that it is in a system call, and is the number of the system
	// call. - nash

	Address regaddr = ORIG_EAX * INTREGSIZE;
	int res;
	res = P_ptrace ( PTRACE_PEEKUSER, getPid(), regaddr, 0 );
	if( res < 0 )
		return false;

	inferiorrpc_cerr << "In system call #" << res << " @ " << (void*)getPC( getPid() ) << endl;

	return true;
}

bool process::changePC(Address loc, const void * /* savedRegs */ ) {
  assert(status_ == stopped);

  return ::changePC(pid, loc);
}

bool process::changePC(Address loc) {
  assert(status_ == stopped);

  return ::changePC(pid, loc);
}


bool process::restoreRegisters(void *buffer) {
   // assumes the process is stopped (ptrace requires it)
   assert(status_ == stopped);

   // Cycle through all registers, writing each from the
   // buffer with ptrace(PTRACE_POKEUSER ...

   char *buf = (char*)buffer;
   bool retVal = true;

   if( P_ptrace( PTRACE_SETREGS, pid, 0, (int)(buf) ) )
   {
       perror("process::restoreRegisters PTRACE_SETREGS" );
       retVal = false;
   }

   if( P_ptrace( PTRACE_SETFPREGS, pid, 0, (int)(buf + GENREGS_STRUCT_SIZE) ) )
   {
       perror("process::restoreRegisters PTRACE_SETFPREGS" );
       retVal = false;
   }

   return retVal;
}

// getActiveFrame(): populate Frame object using toplevel frame
void Frame::getActiveFrame(process *p)
{
  fp_ = ptraceKludge::deliverPtraceReturn(p, PTRACE_PEEKUSER, 0 + EBP * INTREGSIZE, 0);
  if (errno) return;

  pc_ = ptraceKludge::deliverPtraceReturn(p, PTRACE_PEEKUSER, 0 + EIP * INTREGSIZE, 0);
  if (errno) return;
}

// TODO: implement this
bool process::needToAddALeafFrame(Frame,Address &){
    return false;
}

// already setup on this FD.
// disconnect from controlling terminal 
void OS::osDisconnect(void) {
  int ttyfd = open ("/dev/tty", O_RDONLY);
  ioctl (ttyfd, TIOCNOTTY, NULL);
  P_close (ttyfd);
}

bool process::stop_() {
#ifdef DETACH_ON_THE_FLY
     if (haveDetached)
	  return reattach(); /* will stop the process */
#endif
     return (P_kill(getPid(), SIGSTOP) != -1); 
}

bool process::continueWithForwardSignal(int sig) {
   // formerly OS::osForwardSignal
   return (P_ptrace(PTRACE_CONT, pid, 1, sig) != -1);
}

void OS::osTraceMe(void) { P_ptrace(PTRACE_TRACEME, 0, 0, 0); }

process *findProcess(int);  // In process.C

Address getPC(int pid) {
   Address regaddr = EIP * INTREGSIZE;
   int res;
   res = P_ptrace (PTRACE_PEEKUSER, pid, regaddr, 0);
   if( errno ) {
     perror( "getPC" );
     exit(-1);
     return 0; // Shut up the compiler
   } else {
     assert(res);
     return (Address)res;
   }   
}

#ifdef DETACH_ON_THE_FLY
/* Input P points to a buffer containing the contents of
   /proc/PID/stat.  This buffer will be modified.  Output STATUS is
   the status field (field 3), and output SIGPEND is the pending
   signals field (field 31).  As of Linux 2.2, the format of this file
   is defined in /usr/src/linux/fs/proc/array.c (get_stat). */
static void
parse_procstat(char *p, char *status, unsigned long *sigpend)
{
     int i;

     /* Advance past pid and program name */
     p = strchr(p, ')');
     assert(p);

     /* Advance to status character */
     p += 2;
     *status = *p;

     /* Advance to sigpending int */
     p = strtok(p, " ");
     assert(p);
     for (i = 0; i < 28; i++) {
	  p = strtok(NULL, " ");
	  assert(p);
     }
     *sigpend = strtoul(p, NULL, 10);
}


/* The purpose of this is to synchronize with the sigill handler in
   the inferior.  In this handler, the inferior signals us (SIG33),
   and then it stops itself.  This routine does not return until the
   inferior stop has occurred.  In some contexts, the inferior may be
   acquire a pending SIGSTOP as or after it stops.  We clear the
   pending the SIGSTOP here, so the process doesn't inadvertently stop
   when we continue it later.

   We don't understand how the pending SIGSTOP is acquired.  It seems
   to have something to do with the process sending itself the SIGILL
   (i.e., kill(getpid(),SIGILL)).  */
static void
waitForInferiorSigillStop(int pid)
{
     int fd;
     char name[32];
     char buf[512];
     char status;
     unsigned long sigpend;

     sprintf(name, "/proc/%d/stat", pid);

     /* Keep checking the process until it is stopped */
     while (1) {
	  /* Read current contents of /proc/pid/stat */
	  fd = open(name, O_RDONLY);
	  assert(fd >= 0);
	  assert(read(fd, buf, sizeof(buf)) > 0);
	  close(fd);

	  /* Parse status and pending signals */
	  parse_procstat(buf, &status, &sigpend);

	  /* status == T iff the process is stopped */
	  if (status != 'T')
	       continue;
	  /* This is true iff SIGSTOP is set in sigpend */
	  if (0x1 & (sigpend >> (SIGSTOP-1))) {
	       /* The process is stopped, but it has another SIGSTOP
		  pending.  Continue the process to clear the SIGSTOP
		  (the process will stop again immediately). */
	       P_ptrace(PTRACE_CONT, pid, 0, 0);
	       if (0 > waitpid(pid, NULL, 0))
		    perror("waitpid");
	       continue; /* repeat to be sure we've stopped */
	  }

	  /* The process is stopped properly */
	  break;
     }
}

/* When a detached mutatee needs us to reattach and handle an event,
   it sends itself a SIGILL.  Its SIGILL handler in turn sends us
   SIG33, which brings us here.  Here we reattach to the process and
   then help it re-execute the code that caused its SIGILL.  Having
   reattached, we receive the new SIGILL event and dispatch it as
   usual (in handleSigChild). */
static void sigill_handler(int sig, siginfo_t *si, void *unused)
{
     process *p;

     unused = 0; /* Suppress compiler warning of unused parameter */

     assert(sig == 33);
     /* Determine the process that sent the signal.  On Linux (at
	least upto 2.2), we can only obtain this with the real-time
	signals, those numbered 33 or higher. */
     p = findProcess(si->si_pid);
     if (!p) {
	  fprintf(stderr, "Received SIGILL sent by unregistered or non-inferior process\n");
	  assert(0);
     }

     /* Reattach, which should stop the process. */
     p->reattach();
     if (! p->isRunningRPC())
	  /* If we got this signal when the inferior was not in an RPC,
	     then we need to reattach after we handle it.
	     FIXME: Why have we released the process for RPCs anyway? */
	  p->needsDetach = true;

     /* Synchronize with the SIGSTOP sent by inferior sigill handler */
     waitForInferiorSigillStop(p->getPid());

     /* Resume the process.  We expect it to re-execute the code that
        generated the SIGILL.  Now that we are attached, we'll get the
        SIGILL event and handle it with handleSigChild as usual. */
     if (!p->continueProc())
	  assert(0);
}

void initDetachOnTheFly()
{
     struct sigaction act;
     act.sa_handler = NULL;
     act.sa_sigaction = sigill_handler;
     sigemptyset(&act.sa_mask);
     act.sa_flags = SA_SIGINFO;
     /* We need siginfo values to determine the pid of the signal
	sender.  Linux 2.2, bless its open-sourced little heart, does
	not provide siginfo values for non-realtime (<= 32) signals,
	although it will let you register a siginfo handler for them.
	There are apparently no predefined names for the realtime
	signals so we just use their integer value. */
     if (0 > sigaction(33, &act, NULL)) {
	  perror("SIG33 sigaction");
	  assert(0);
     }
}

/* Returns true if we are detached from process PID.  This is global
   (not a process member) because we need to call it from P_ptrace,
   which does not know about process objects. */
bool haveDetachedFromProcess(int pid)
{
     /* This can be called before a process is initialized.  If we
	don't find a process for PID, we assume it hasn't been
	initialized yet, and thus we have not ever detached from it. */
     process *p = findProcess(pid);
     if (! p)
	  return false;
     return p->haveDetached;
}


/* The following two routines block SIG33 to prevent themselves from
   being reentered. */
int process::detach()
{
     int res, ret;
     sigset_t tmp, old;

     ret = 0;
     sigemptyset(&tmp);
     sigaddset(&tmp, 33);
     sigprocmask(SIG_BLOCK, &tmp, &old);

#if 0
     fprintf(stderr, "DEBUG: detach ENTRY\n");
#endif

     if (this->haveDetached) {
	  /* It is safe to call this when already detached.  Silently
             ignore the call.  But the caller really ought to know
             better. */
	  goto out;
     }
     if (this->pendingSig) {
	  res = P_ptrace(PTRACE_DETACH, pid, 0, this->pendingSig);
	  assert(res >= 0);
	  this->pendingSig = 0;
     } else {
	  res = P_ptrace(PTRACE_DETACH, pid, 0, 0);
	  assert(res >= 0);
     }
     this->haveDetached = true;
     this->juststopped = false;
     this->needsDetach = false;
     this->status_ = running;
     ret = 1;
out:
#if 0
     fprintf(stderr, "DEBUG: detach EXIT (%s)\n", ret == 1 ? "success" : "failure");
#endif
     sigprocmask(SIG_SETMASK, &old, NULL);
     return ret;
}

int process::reattach()
{
     int res, ret;
     sigset_t tmp, old;

     ret = 0;
     sigemptyset(&tmp);
     sigaddset(&tmp, 33);
     sigprocmask(SIG_BLOCK, &tmp, &old);

#if 0
     fprintf(stderr, "DEBUG: reattach ENTRY\n");
#endif
     if (! this->haveDetached) {
	  /* It is safe to call this when already attached.  Silently
             ignore the call.  But the caller really ought to know
             better. */
	  goto out;
     }

     /* Here we attach to the process and call waitpid to process the
	associated stop event.  The process may be about to handle a
	trap, in which case waitpid will indicate a SIGTRAP.  We
	detach and reattach to clear this event, but we'll resend
	SIGTRAP when we detach. */
     while (1) {
	  int stat;
	  res = P_ptrace(PTRACE_ATTACH, pid, 0, 0);
	  assert(res >= 0);
	  res = waitpid(pid, &stat, 0);
	  assert(res >= 0);
	  if (WIFSTOPPED(stat) && WSTOPSIG(stat) != SIGSTOP) {
	       assert(!this->pendingSig);
	       assert(WSTOPSIG(stat) == SIGTRAP); /* We only expect traps */
	       this->pendingSig = SIGTRAP;
	       ret = P_ptrace(PTRACE_DETACH, pid, 0, 0);
	       continue;
	  }
	  break;
     }

     this->haveDetached = 0;
     this->juststopped = true;
     this->status_ = stopped;
     ret = 1;
out:
#if 0
     fprintf(stderr, "DEBUG: reattach EXIT (%s)\n", ret == 1 ? "success" : "failure");
#endif
     sigprocmask(SIG_SETMASK, &old, NULL);
     return ret;
}

/* This method replaces continueProc. */
int process::detachAndContinue()
{
     int ret = false;

#ifndef BPATCH_LIBRARY
     timeStamp notValidSentinal = timeStamp::ts1800();
     timeStamp initialStartTime;
#endif

     if (status_ == exited)
	  goto out;

     if (status_ != stopped && status_ != neonatal) {
	  sprintf(errorLine, "Internal error: "
		  "Unexpected process state %d in process::contineProc",
		  (int)status_);
	  showErrorCallback(39, errorLine);
	  goto out;
     }
     
#ifndef BPATCH_LIBRARY
     initialStartTime = getWallTime();
#endif
     // Vic says continuing is a side effect of detaching
     if (! this->detach()) {
	  showErrorCallback(38, "System error: can't continue process");
#ifndef BPATCH_LIBRARY
	  initialStartTime = notValidSentinal;
#endif
	  goto out;
     }

     ret = true;
     status_ = running;
out:
#ifndef BPATCH_LIBRARY
     if(callBeforeContinue != NULL && 
	initialStartTime.isInitialized() && 
	initialStartTime != notValidSentinal) {
       (*callBeforeContinue)(initialStartTime);
     }
#endif
     return ret;
}

/* This method replaces pause. */
int process::reattachAndPause()
{
     this->reattach();
     return this->pause();
}
#endif /* DETACH_ON_THE_FLY */

// wait for a process to terminate or stop
// We only want to catch SIGSTOP and SIGILL
#ifdef BPATCH_LIBRARY
int process::waitProcs(int *status, bool block) {
  int options = 0;
  if( !block )
    options |= WNOHANG;
#else
int process::waitProcs(int *status) {
  int options = WNOHANG;
#endif
  int result = 0, sig = 0;
  bool ignore;

       do {
	    ignore = false;
	    result = waitpid( -1, status, options );
	    
	    // Check for TRAP at the end of a syscall, then
	    // if the signal's not SIGSTOP or SIGILL,
	    // send the signal back and wait for another.
	    if( result > 0 && WIFSTOPPED(*status) ) {
		 process *p = findProcess( result );
		 sig = WSTOPSIG(*status);
		 if( sig == SIGTRAP && ( !p->reachedVeryFirstTrap || p->inExec ) )
		      ; // Report it
		 else if( sig != SIGSTOP && sig != SIGILL ) {
			ignore = true;
			if( sig != SIGTRAP )
			{
//#ifdef notdef
			     Address pc;
			     pc = getPC( result );
			     signal_cerr << "process::waitProcs -- Signal #" << sig << " in " << result << "@" << (void*)pc << ", resignalling the process" << endl;
//#endif
			}
			if( P_ptrace(PTRACE_CONT, result, 1, sig) == -1 ) {
			     if( errno == ESRCH ) {
				  cerr << "WARNING -- process does not exist, constructing exit code" << endl;
				  *status = W_EXITCODE(0,sig);
				  ignore = false;
			     } else
				  cerr << "ERROR -- process::waitProcs forwarding signal " << sig << " -- " << sys_errlist[errno] << endl;
			}
#ifdef notdef
			else
			     signal_cerr << "Signal " << sig << " in " << result << endl;
#endif
		 }
	    }
       } while ( ignore );


  if( result > 0 ) {
	  if( WIFSTOPPED(*status) ) {
		  process *curr = findProcess( result );
		  if (!curr->dyninstLibAlreadyLoaded() && curr->wasCreatedViaAttach())
		  {
		       /* FIXME: Is any of this code ever executed? */
			  // make sure we are stopped in the eyes of paradynd - naim
			  bool wasRunning = (curr->status() == running);
			  if (curr->status() != stopped)
				  curr->Stopped();   
			  if(curr->isDynamicallyLinked()) {
				  curr->handleIfDueToSharedObjectMapping();
			  }
			  if (curr->trapDueToDyninstLib()) {
				  // we need to load libdyninstRT.so.1 - naim
				  curr->handleIfDueToDyninstLib();
			  }
			  if (wasRunning) 
				  if (!curr->continueProc()) assert(0);
		  }
	  }
#ifdef SIGNAL_DEBUG
	  if( WIFSIGNALED(*status) )
	  {
		  sig = WTERMSIG(*status);
		  signal_cerr << "process::waitProcs -- Exit on signal #" << sig << " in " << result << ", resignalling the process" << endl;
	  }
	  else if( WIFEXITED(*status) )
	  {
		  signal_cerr << "process::waitProcs -- Exit from " << result << endl;
	  }
#endif
  }// else if( errno )
    //perror( "process::waitProcs - waitpid" );
  return result;
}

// attach to an inferior process.
bool process::attach() {
  char procName[128];

  bool running = false;
  if( createdViaAttach )
    running = isRunning_();

  proc_fd = -1;

  // QUESTION: does this attach operation lead to a SIGTRAP being forwarded
  // to paradynd in all cases?  How about when we are attaching to an
  // already-running process?  (Seems that in the latter case, no SIGTRAP
  // is automatically generated)

  // step 1) /proc open: attach to the inferior process memory file
  sprintf(procName,"/proc/%d/mem", (int)getPid());
  attach_cerr << "Opening memory space for process #" << getPid() << " at " << procName << endl;
  int fd = P_open(procName, O_RDWR, 0);
  if (fd < 0 ) {
    fprintf(stderr, "attach: open failed on %s: %s\n", procName, sys_errlist[errno]);
    return false;
  }

  proc_fd = fd;

  // Only if we are really attaching rather than spawning the inferior
  // process ourselves do we need to call PTRACE_ATTACH
  if( createdViaAttach || createdViaFork ) {
	  attach_cerr << "process::attach() doing PTRACE_ATTACH" << endl;
    if( 0 != P_ptrace(PTRACE_ATTACH, getPid(), 0, 0) )
	{
		perror( "process::attach - PTRACE_ATTACH" );
		return false;
	}
  }

  if( createdViaAttach )
  {
    // If the process was running, it will need to be restarted, as
    // PTRACE_ATTACH kills it
    // Actually, the attach process contructor assumes that the process is
    // running.  While this is foolish, let's play along for now.
	if( status_ != running || !isRunning_() ) {
		if( 0 != P_ptrace(PTRACE_CONT, getPid(), 0, 0) )
		{
			perror( "process::attach - continue" );
		}
    }
  }

#ifdef notdef
  if( status_ != running && isRunning_() )
  {
	  attach_cerr << "fixing status_ => running" << endl;
	  status_ = running;
  }
  else if( status_ == running && !isRunning_() )
  {
	  attach_cerr << "fixing status_ => stopped" << endl;
	  status_ = stopped;
  }
#endif

  return true;
}

bool process::attach_() {
  return false; // (P_ptrace(PTRACE_ATTACH, getPid(), 0, 0) != -1);
}

bool process::trapAtEntryPointOfMain()
{
  // is the trap instr at main_brk_addr?
  if( getPC(getPid()) == (Address)main_brk_addr)
    return(true);
  else
    return(false);
}

bool process::trapDueToDyninstLib()
{
  // is the trap instr at dyninstlib_brk_addr?
  if( getPC(getPid()) == (Address)dyninstlib_brk_addr)
    return(true);
  else
    return(false);
}

void process::handleIfDueToDyninstLib() 
{
  // rewrite original instructions in the text segment we use for 
  // the inferiorRPC - naim
  unsigned count = sizeof(savedCodeBuffer);
  //Address codeBase = getImage()->codeOffset();

  Address codeBase = 0;
  int i;

  for( i = 0; i < N_DYNINST_LOAD_HIJACK_FUNCTIONS; i++ ) {
	  bool found = false;
	  Symbol s;
	  codeBase = 0;
	  found = symbols->symbol_info(DYNINST_LOAD_HIJACK_FUNCTIONS[i], s);
	  if( found )
		  codeBase = s.addr();
	  if( codeBase )
		  break;
  }
  assert( codeBase );

  writeDataSpace((void *)codeBase, count, (char *)savedCodeBuffer);

  // restore registers
  restoreRegisters(savedRegs); 

  // restore the stack frame of _start()
  user_regs_struct *theIntRegs = (user_regs_struct *)savedRegs;
  RegValue theEBP = theIntRegs->ebp;

  if( !theEBP )
  {
	  theEBP = theIntRegs->esp;
  }

  assert (theEBP);
  // this is pretty kludge. if the stack frame of _start is not the right
  // size, this would break.
  writeDataSpace ((void*)(theEBP-6*sizeof(int)),6*sizeof(int),savedStackFrame);

  if( isRunning_() )
	  cerr << "WARNING -- process is running at trap from dlopenDYNINSTlib" << endl;

  delete [] (char *) savedRegs;
  savedRegs = NULL;
}

void process::handleTrapAtEntryPointOfMain()
{
  function_base *f_main = findOneFunction("main");
  assert(f_main);
  Address addr = f_main->addr();
  // restore original instruction 
  writeDataSpace((void *)addr, 2, (char *)savedCodeBuffer);
}

void process::insertTrapAtEntryPointOfMain()
{
  function_base *f_main = 0;
  f_main = findOneFunction("main");
  if (!f_main) {
    // we can't instrument main - naim
    showErrorCallback(108,"main() uninstrumentable");
    extern void cleanUpAndExit(int);
    cleanUpAndExit(-1); 
    return;
  }
  assert(f_main);
  Address addr = f_main->addr();

  // save original instruction first
  readDataSpace((void *)addr, 2, savedCodeBuffer, true);

  // and now, insert trap
  instruction insnTrap;
  generateBreakPoint(insnTrap);

  // x86. have to use SIGILL instead of SIGTRAP
  writeDataSpace((void *)addr, 2, insnTrap.ptr());  

  main_brk_addr = addr;
}

void emitCallRel32(unsigned disp32, unsigned char *&insn);

bool process::dlopenDYNINSTlib() {
#if false && defined(PTRACEDEBUG)
  debug_ptrace = true;
#endif
  // we will write the following into a buffer and copy it into the
  // application process's address space
  // [....LIBRARY's NAME...|code for DLOPEN]

  // write to the application at codeOffset. This won't work if we
  // attach to a running process.
  //Address codeBase = this->getImage()->codeOffset();
  // ...let's try "_start" instead
  //  Address codeBase = (this->findFuncByName(DYNINST_LOAD_HIJACK_FUNCTION))->getAddress(this);
  Address codeBase = 0;
  int i;

  for( i = 0; i < N_DYNINST_LOAD_HIJACK_FUNCTIONS; i++ ) {
      bool found = false;
      Symbol s;
      codeBase = 0;
      found = symbols->symbol_info(DYNINST_LOAD_HIJACK_FUNCTIONS[i], s);
      if( found )
          codeBase = s.addr();
      if( codeBase )
          break;
  }

  if( !codeBase || i >= N_DYNINST_LOAD_HIJACK_FUNCTIONS )
  {
      attach_cerr << "Couldn't find a point to insert dlopen call" << endl;
      return false;
  }

  attach_cerr << "Inserting dlopen call in " << DYNINST_LOAD_HIJACK_FUNCTIONS[i] << " at "
      << (void*)codeBase << endl;
  attach_cerr << "Process at " << (void*)getPC( getPid() ) << endl;

  bool libc_21 = true;
  Symbol libc_vers;
  if( !getSymbolInfo( libc_version_symname, libc_vers ) ) {
      cerr << "Couldn't find " << libc_version_symname << ", assuming glibc_2.1.x" << endl;
  } else {
      char libc_version[ libc_vers.size() + 1 ];
      libc_version[ libc_vers.size() ] = '\0';
      readDataSpace( (void *)libc_vers.addr(), libc_vers.size(), libc_version, true );
      if( !strncmp( libc_version, "2.1", 3 ) ) {
	  attach_cerr << "Detected glibc version 2.1, (\"" << libc_version << "\")" << endl;
	  libc_21 = true;
      } else if( !strncmp( libc_version, "2.0", 3 ) ) {
	  attach_cerr << "Detected glibc version 2.0, (\"" << libc_version << "\")" << endl;
	  libc_21 = false;
      } else {
	  cerr << "Found " << libc_version_symname << " = \"" << libc_version
	       << "\", which doesn't match any known glibc, assuming glibc 2.1" << endl;
      }
  }

  // Or should this be readText... it seems like they are identical
  // the remaining stuff is thanks to Marcelo's ideas - this is what 
  // he does in NT. The major change here is that we use AST's to 
  // generate code for dlopen.

  // savedCodeBuffer[BYTES_TO_SAVE] is declared in process.h

  readDataSpace((void *)codeBase, sizeof(savedCodeBuffer), savedCodeBuffer, true);

  unsigned char scratchCodeBuffer[BYTES_TO_SAVE];
  vector<AstNode*> dlopenAstArgs( 2 );

  unsigned count = 0;
  Address dyninst_count = 0;

  AstNode *dlopenAst;

  // deadList and deadListSize are defined in inst-sparc.C - naim
  extern Register deadList[];
  extern int deadListSize;
  registerSpace *dlopenRegSpace = new registerSpace(deadListSize/sizeof(int), deadList, 0, NULL);
  dlopenRegSpace->resetSpace();

  // we need to make a call to dlopen to open our runtime library

  if( !libc_21 ) {
      dlopenAstArgs[0] = new AstNode(AstNode::Constant, (void*)0);
      // library name. We use a scratch value first. We will update this parameter
      // later, once we determine the offset to find the string - naim
      dlopenAstArgs[1] = new AstNode(AstNode::Constant, (void*)DLOPEN_MODE); // mode
      dlopenAst = new AstNode(DL_OPEN_FUNC_NAME,dlopenAstArgs);
      removeAst(dlopenAstArgs[0]);
      removeAst(dlopenAstArgs[1]);
      
      dyninst_count = 0;
      dlopenAst->generateCode(this, dlopenRegSpace, (char *)scratchCodeBuffer,
                              dyninst_count, true, true);
  } else {
      // In glibc 2.1.x, _dl_open is optimized for being an internal wrapper function.
      // Instead of using the stack, it passes three parameters in EAX, EDX and ECX.
      // Here we simply make a call with no normal parameters, and below we change
      // the three registers along with EIP to execute the code.
      unsigned char *code_ptr = scratchCodeBuffer;
      Address disp;
      Address addr;
      bool err;
      addr = findInternalAddress(DL_OPEN_FUNC_NAME, false, err);
      if (err) {
	  function_base *func = findOneFunction(DL_OPEN_FUNC_NAME);
	  if (!func) {
	      ostrstream os(errorLine, 1024, ios::out);
	      os << "Internal error: unable to find addr of " << DL_OPEN_FUNC_NAME << endl;
	      logLine(errorLine);
	      showErrorCallback(80, (const char *) errorLine);
	      P_abort();
	  }
	  addr = func->getAddress(0);
      }

      disp = addr - ( codeBase + 5 );
      attach_cerr << DL_OPEN_FUNC_NAME << " @ " << (void*)addr << ", displacement == "
		  << (void*)disp << endl;
      emitCallRel32( disp, code_ptr );
      dyninst_count = 5;
  }

  writeDataSpace((void *)(codeBase+count), dyninst_count, (char *)scratchCodeBuffer);
  count += dyninst_count;

  instruction insnTrap;
  generateBreakPoint(insnTrap);
  writeDataSpace((void *)(codeBase + count), 2, insnTrap.ptr());
  dyninstlib_brk_addr = codeBase + count;
  count += 2;

#ifdef BPATCH_LIBRARY  /* dyninst API loads a different run-time library */
  const char DyninstEnvVar[]="DYNINSTAPI_RT_LIB";
#else
  const char DyninstEnvVar[]="PARADYN_LIB";
#endif

  if (dyninstName.length()) {
    // use the library name specified on the start-up command-line
  } else {
    // check the environment variable
    if (getenv(DyninstEnvVar) != NULL) {
      dyninstName = getenv(DyninstEnvVar);
    } else {
      string msg = string("Environment variable " + string(DyninstEnvVar)
                   + " has not been defined for process ") + string(pid);
      showErrorCallback(101, msg);
      return false;
    }
  }
  if (access(dyninstName.string_of(), R_OK)) {
    string msg = string("Runtime library ") + dyninstName
        + string(" does not exist or cannot be accessed!");
    showErrorCallback(101, msg);
    return false;
  }

  Address dyninstlib_addr = (Address) (codeBase + count);

  writeDataSpace((void *)(codeBase + count), dyninstName.length()+1,
		 (caddr_t)const_cast<char*>(dyninstName.string_of()));
  count += dyninstName.length()+1;
  // we have now written the name of the library after the trap - naim

  assert(count<=BYTES_TO_SAVE);

  if( !libc_21 ) {
      count = 0; // reset count

      // at this time, we know the offset for the library name, so we fix the
      // call to dlopen and we just write the code again! This is probably not
      // very elegant, but it is easy and it works - naim
      removeAst(dlopenAst); // to avoid leaking memory
      dlopenAstArgs[0] = new AstNode(AstNode::Constant, (void *)(dyninstlib_addr));
      dlopenAstArgs[1] = new AstNode(AstNode::Constant, (void*)DLOPEN_MODE);
      dlopenAst = new AstNode(DL_OPEN_FUNC_NAME,dlopenAstArgs);
      removeAst(dlopenAstArgs[0]);
      removeAst(dlopenAstArgs[1]);
      dyninst_count = 0; // reset count
      dlopenAst->generateCode(this, dlopenRegSpace, (char *)scratchCodeBuffer,
                              dyninst_count, true, true);
      writeDataSpace((void *)(codeBase+count), dyninst_count, (char *)scratchCodeBuffer);
      removeAst(dlopenAst);
  }

  // save registers
  savedRegs = getRegisters();
  assert((savedRegs!=NULL) && (savedRegs!=(void *)-1));
  // save the stack frame of _start()
  user_regs_struct *regs = (user_regs_struct*)savedRegs;
  user_regs_struct new_regs = *regs;
  RegValue theEBP = regs->ebp;

  // Under Linux, at the entry point to main, ebp is 0
  // the first thing main usually does is to push ebp and
  // move esp -> ebp, so we'll do that, too
  if( !theEBP )
  {
	  theEBP = regs->esp;
	  attach_cerr << "eBP at 0x0, creating fake stack frame with eSP == "
				  << (void*)theEBP << endl;
	  changeBP( getPid(), theEBP );
  }

  assert( theEBP );
  // this is pretty kludge. if the stack frame of _start is not the right
  // size, this would break.
  readDataSpace((void*)(theEBP-6*sizeof(int)),6*sizeof(int), savedStackFrame, true);

  isLoadingDyninstLib = true;

  attach_cerr << "Changing PC to " << (void*)codeBase << endl;

  if (!libc_21)
  {
      if (!changePC(codeBase,savedRegs))
      {
          logLine("WARNING: changePC failed in dlopenDYNINSTlib\n");
          assert(0);
      }
  }
  else
  {
      new_regs.eip = codeBase;

      if( libc_21 ) {
          new_regs.eax = dyninstlib_addr;
          new_regs.edx = DLOPEN_MODE;
          new_regs.ecx = codeBase;
      }

      if( !restoreRegisters( (void*)(&new_regs) ) )
      {
          logLine("WARNING: changePC failed in dlopenDYNINSTlib\n");
          assert(0);
      }
  }

#if false && defined(PTRACEDEBUG)
  debug_ptrace = false;
#endif

  return true;
}

Address process::get_dlopen_addr() const {
  if (dyn != NULL)
    return(dyn->get_dlopen_addr());
  else 
    return(0);
}

bool process::isRunning_() const {
   // determine if a process is running by doing low-level system checks, as
   // opposed to checking the 'status_' member vrble.  May assume that attach()
   // has run, but can't assume anything else.

  char procName[64];
  char sstat[132];
  char *token = NULL;

  sprintf(procName,"/proc/%d/stat", (int)pid);
  FILE *sfile = P_fopen(procName, "r");
  fread( sstat, 128, 1, sfile );
  fclose( sfile );

  char status;
  if( !( strtok( sstat, " (" ) && strtok( NULL, " )" ) && ( token = strtok( NULL, " " ) ) ) )
    assert( false && "Shouldn't happen" );
  status = token[0];

  if( status == 'T' )
    return false;
  else
    return true;
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
  ret = P_ptrace(PTRACE_CONT, getPid(), 1, 0);

  if (ret == -1)
  {
	  /*if( isRunning_() )
		  ret = 0;
		  else*/
		  perror("continueProc_()");
  }

  return ret != -1;
}

#ifdef BPATCH_LIBRARY
bool process::terminateProc_()
{
  if (!checkStatus()) 
    return false;

  if( kill( getPid(), SIGKILL ) != 0 )
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
  close( proc_fd );
  return (ptraceKludge::deliverPtrace(this, PTRACE_DETACH, 1, SIGCONT));
}

#ifdef BPATCH_LIBRARY
bool process::API_detach_(const bool cont) {
//  assert(cont);
  if (!checkStatus())
    return false;
  ptraceOps++; ptraceOtherOps++;
  close( proc_fd );
  if (!cont) P_kill(pid, SIGSTOP);
  return (ptraceKludge::deliverPtrace(this, PTRACE_DETACH, 1, SIGCONT));
}
#endif

bool process::dumpCore_(const string/* coreFile*/) { return false; }

bool process::writeTextWord_(caddr_t inTraced, int data) {
//  cerr << "writeTextWord @ " << (void *)inTraced << endl; cerr.flush();
  return writeDataSpace_(inTraced, sizeof(int), (caddr_t) &data);
}

bool process::writeTextSpace_(void *inTraced, u_int amount, const void *inSelf) {
//  cerr << "writeTextSpace pid=" << getPid() << ", @ " << (void *)inTraced << " len=" << amount << endl; cerr.flush();
  return writeDataSpace_(inTraced, amount, inSelf);
}

#ifdef BPATCH_SET_MUTATIONS_ACTIVE
bool process::readTextSpace_(void *inTraced, u_int amount, const void *inSelf) {
  return readDataSpace_( inTraced, amount, const_cast<void*>( inSelf ) );
}
#endif

bool process::writeDataSpace_(void *inTraced, u_int nbytes, const void *inSelf)
{
     unsigned char *ap = (unsigned char*) inTraced;
     const unsigned char *dp = (const unsigned char*) inSelf;
     int pid = getPid();
     int w;               /* ptrace I/O buffer */
     unsigned len = sizeof(w); /* address alignment of ptrace I/O requests */
     unsigned cnt;

     ptraceOps++; ptraceBytes += nbytes;

     if (0 == nbytes)
	  return true;

     if ((cnt = (unsigned)ap % len)) {
	  /* Start of request is not aligned. */
	  unsigned char *p = (unsigned char*) &w;
	  
	  /* Read the segment containing the unaligned portion, edit
	     in the data from DP, and write the segment back. */
	  errno = 0;
	  w = P_ptrace(PTRACE_PEEKTEXT, pid, (int) (ap-cnt), 0);
	  if (errno)
	       return false;
	  for (unsigned i = 0; i < len-cnt && i < nbytes; i++)
	       p[cnt+i] = dp[i];
	  if (0 > P_ptrace(PTRACE_POKETEXT, pid, (int) (ap-cnt), w))
	       return false;

	  if (len-cnt >= nbytes) 
	       return true; /* done */
	  
	  dp += len-cnt;
	  ap += len-cnt;
	  nbytes -= len-cnt;
     }	  
	  
     /* Copy aligned portion */
     while (nbytes >= len) {
	  assert(0 == (unsigned)ap % len);
	  memcpy(&w, dp, len);
	  if (0 > P_ptrace(PTRACE_POKETEXT, pid, (int) ap, w))
	       return false;
	  dp += len;
	  ap += len;
	  nbytes -= len;
     }

     if (nbytes > 0) {
	  /* Some unaligned data remains */
	  unsigned char *p = (unsigned char *) &w;
	  
	  /* Read the segment containing the unaligned portion, edit
             in the data from DP, and write it back. */
	  errno = 0;
	  w = P_ptrace(PTRACE_PEEKTEXT, pid, (int) ap, 0);
	  if (errno)
	       return false;
	  for (unsigned i = 0; i < nbytes; i++)
	       p[i] = dp[i];
	  if (0 > P_ptrace(PTRACE_POKETEXT, pid, (int) ap, w))
	       return false;
     }

     return true;
}

bool process::readDataSpace_(const void *inTraced, u_int nbytes, void *inSelf) {
     const unsigned char *ap = (const unsigned char*) inTraced;
     unsigned char *dp = (unsigned char*) inSelf;
     int pid = getPid();
     int w;               /* ptrace I/O buffer */
     unsigned len = sizeof(w); /* address alignment of ptrace I/O requests */
     unsigned cnt;

     ptraceOps++; ptraceBytes += nbytes;

     if (0 == nbytes)
	  return true;

     if ((cnt = (unsigned)ap % len)) {
	  /* Start of request is not aligned. */
	  unsigned char *p = (unsigned char*) &w;

	  /* Read the segment containing the unaligned portion, and
             copy what was requested to DP. */
	  errno = 0;
	  w = P_ptrace(PTRACE_PEEKTEXT, pid, (int) (ap-cnt), w);
	  if (errno)
	       return false;
	  for (unsigned i = 0; i < len-cnt && i < nbytes; i++)
	       dp[i] = p[cnt+i];

	  if (len-cnt >= nbytes)
	       return true; /* done */

	  dp += len-cnt;
	  ap += len-cnt;
	  nbytes -= len-cnt;
     }

     /* Copy aligned portion */
     while (nbytes >= len) {
	  errno = 0;
	  w = P_ptrace(PTRACE_PEEKTEXT, pid, (int) ap, 0);
	  if (errno)
	       return false;
	  memcpy(dp, &w, len);
	  dp += len;
	  ap += len;
	  nbytes -= len;
     }

     if (nbytes > 0) {
	  /* Some unaligned data remains */
	  unsigned char *p = (unsigned char *) &w;
	  
	  /* Read the segment containing the unaligned portion, and
             copy what was requested to DP. */
	  errno = 0;
	  w = P_ptrace(PTRACE_PEEKTEXT, pid, (int) ap, 0);
	  if (errno)
	       return false;
	  for (unsigned i = 0; i < nbytes; i++)
	       dp[i] = p[i];
     }

     return true;
}

/*
bool process::findCallee(instPoint &instr, function_base *&target){
  fprintf( stderr, "findCallee not implemented\n" );
  return false;
}
*/

Frame Frame::getCallerFrameNormal(process *p) const
{
  //
  // for the x86, the frame-pointer (EBP) points to the previous frame-pointer,
  // and the saved return address is in EBP-4.
  //
  struct {
    int fp;
    int rtn;
  } addrs;

  if (p->readDataSpace((caddr_t)(fp_), 2*sizeof(int),
		       (caddr_t) &addrs, true))
  {
    Frame ret;
    ret.fp_ = addrs.fp;
    ret.pc_ = addrs.rtn;
    return ret;
  }

  return Frame(); // zero frame
}

// You know, /proc/*/exe is a perfectly good link (directly to the inode) to
// the executable file, who cares where the executable really is, we can open
// this link. - nash
string process::tryToFindExecutable(const string & /* iprogpath */, int pid) {
  return string("/proc/") + string(pid) + "/exe";
}

/*
 * The old, ugly version that we don't need but can waste space anyhow
 * /
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

   string argv0, path, cwd;

   char buffer[128];
   sprintf(buffer, "/proc/%d/environ", pid);
   int procfd = open(buffer, O_RDONLY, 0);
   if (procfd == -1) {
     attach_cerr << "tryToFindExecutable failed since open of /proc/ * /environ failed" << endl;
     return "";
   }
   attach_cerr << "tryToFindExecutable: opened /proc/ * /environ okay" << endl;

   int strptr = 0;
   while( true ) {
     string env_value = extract_string( procfd, (char*)strptr );
     if( !env_value.length() )
       break;

     if (env_value.prefixed_by("PWD=") || env_value.prefixed_by("CWD=")) {
       cwd = env_value.string_of() + 4; // skip past "PWD=" or "CWD="
       attach_cerr << "get_ps_stuff: using PWD value of: " << cwd << endl;
       if( path.length() )
	 break;
     } else if (env_value.prefixed_by("PATH=")) {
       path = env_value.string_of() + 5; // skip past the "PATH="
       attach_cerr << "get_ps_stuff: using PATH value of: " << path << endl;
       if( cwd.length() )
	 break;
     }

     strptr += env_value.length() + 1;
   }

   close( procfd );

   sprintf(buffer, "/proc/%d/cmdline", pid);
   procfd = open(buffer, O_RDONLY, 0);
   if (procfd == -1) {
     attach_cerr << "tryToFindExecutable failed since open of /proc/ * /cmdline failed" << endl;
     return "";
   }
   attach_cerr << "tryToFindExecutable: opened /proc/ * /cmdline okay" << endl;

   argv0 = extract_string( procfd, (char*)0 );
   close( procfd );

   if ( argv0.length() && path.length() && cwd.length() ) {
     // the following routine is implemented in the util lib.
     string result;
     if (executableFromArgv0AndPathAndCwd(result, argv0, path, cwd)) {
       attach_cerr << "tryToFindExecutable: returning " << result << endl;

       // I feel picky today, let's make certain that /proc agrees that
       // this is the executable by checking the inode number of
       // /proc/ * /exe against the inode number of result

       sprintf(buffer, "/proc/%d/exe", pid);
       struct stat t_stat;
       int t_inode;
       if( stat( buffer, &t_stat ) ) {
	 t_inode = t_stat.st_ino;
	 if( stat( buffer, &t_stat ) && t_inode != t_stat.st_ino )
	   cerr << "tryToFindExecutable: WARNING - found executable does not match /proc" << endl;
       }

       return result;
     }
   }

   attach_cerr << "tryToFindExecutable: giving up" << endl;

   return "";
}
*/

#ifdef SHM_SAMPLING
rawTime64 process::getRawCpuTime_hw(int /*lwp_id*/) {
  rawTime64 val = 0;
#ifdef HRTIME
  val = hrtimeGetVtime(hr_cpu_link);
#endif
  return val;
}

rawTime64 process::getRawCpuTime_sw(int /*lwp_id*/) /* const */ {
  rawTime64 now = 0;
  int bufsize = 150, utime, stime;
  char procfn[bufsize], *buf;

  sprintf( procfn, "/proc/%d/stat", getPid() );

  int fd;

  // The reason for this complicated method of reading and sseekf-ing is
  // to ensure that we read enough of the buffer 'atomically' to make sure
  // the data is consistent.  Is this necessary?  I *think* so. - nash
  do {
    fd = P_open(procfn, O_RDONLY, 0);
    if (fd < 0) {
      shmsample_cerr << "getInferiorProcessCPUtime: open failed: " << sys_errlist[errno] << endl;
      return false;
    }

    buf = new char[ bufsize ];

    if ((int)P_read( fd, buf, bufsize-1 ) < 0) {
      perror("getInferiorProcessCPUtime");
      return false;
    }

    if(2==sscanf(buf,"%*d %*s %*c %*d %*d %*d %*d %*d %*u %*u %*u %*u %*u %d %d "
		 , &utime, &stime ) ) {
      // These numbers are in 'jiffies' or timeslices.
      // Oh, and I'm also assuming that process time includes system time
      now = static_cast<rawTime64>(utime) + static_cast<rawTime64>(stime);
      break;
    }

    delete [] buf;
    shmsample_cerr << "Inferior CPU time buffer expansion (" << bufsize << ")" << endl;
    bufsize = bufsize * 2;

    P_close( fd );
  } while ( true );

  delete [] buf;
  P_close(fd);

  return now;
}

#endif // SHM_SAMPLING

bool process::loopUntilStopped() {
     int flags = WUNTRACED | WNOHANG;
     bool stopSig = false;
     int count = 0;
     /* make sure the process is stopped in the eyes of ptrace */
     
     while (true) {
	  int waitStatus;
	  int ret = P_waitpid( getPid(), &waitStatus, flags );
	  if( ret == 0 ) {
#ifdef DETACH_ON_THE_FLY
	       /* This used to go between the stopSig = true and the stop_ call. */
	       /* FIXME: Maybe we can clean up loopUntilStopped and make this go away? */
	       if (juststopped) {
		    /* The process was already stopped by reattach() */
		    juststopped = false;
		    break;
	       }
#endif /* DETACH_ON_THE_FLY */

	       if( !stopSig ) {
		    stopSig = true;
		    stop_();
	       }
	       else if( ++count > 10 && !isRunning_() )
		    break;
	  } else if ((ret == -1 && errno == ECHILD) || (WIFEXITED(waitStatus))) {
	       // the child is gone.
	       handleProcessExit(this, WEXITSTATUS(waitStatus));
	       return(false);
	  } else if (WIFSIGNALED(waitStatus)) {
	       handleProcessExit(this, WTERMSIG(waitStatus));
	       return false;
	  } else if (WIFSTOPPED(waitStatus)) {
	       int sig = WSTOPSIG(waitStatus);
	       if ( sig == SIGSTOP ) {
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

#ifndef BPATCH_LIBRARY
bool process::dumpImage() {return false;}
#else
bool process::dumpImage(string imageFileName) 
{
    int newFd;
    image *im;
    int length = 0;
    string command;

    im = getImage();
    string origFile = im->file();


    // first copy the entire image file
    command = "cp ";
    command += origFile;
    command += " ";
    command += imageFileName;
    system(command.string_of());

    // now open the copy
    newFd = open(imageFileName.string_of(), O_RDWR, 0);
    if (newFd < 0) {
	// log error
	return false;
    }

    Elf *elfp = elf_begin(newFd, ELF_C_READ, 0);
    Elf_Scn *scn = 0;
    u_int baseAddr = 0;
    int offset = 0;

    Elf32_Ehdr*	ehdrp;
    Elf_Scn* shstrscnp  = 0;
    Elf_Data* shstrdatap = 0;
    Elf32_Shdr* shdrp;

    assert(ehdrp = elf32_getehdr(elfp));
    assert(((shstrscnp = elf_getscn(elfp, ehdrp->e_shstrndx)) != 0) &&
           ((shstrdatap = elf_getdata(shstrscnp, 0)) != 0));
    const char* shnames = (const char *) shstrdatap->d_buf;

    while ((scn = elf_nextscn(elfp, scn)) != 0) {
	const char* name;

	shdrp = elf32_getshdr(scn);
	name = (const char *) &shnames[shdrp->sh_name];
	if (!strcmp(name, ".text")) {
	    offset = shdrp->sh_offset;
	    length = shdrp->sh_size;
	    baseAddr = shdrp->sh_addr;
	    break;
	}
    }


    char tempCode[length];


    bool ret = readTextSpace_((void *) baseAddr, length, tempCode);
    if (!ret) {
       // log error
       return false;
    }

    lseek(newFd, offset, SEEK_SET);
    write(newFd, tempCode, length);
    close(newFd);

    return true;
}
#endif

#ifndef BPATCH_LIBRARY

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

#endif

int getNumberOfCPUs()
{
  return sysconf(_SC_NPROCESSORS_ONLN);
}

Address process::read_inferiorRPC_result_register(Register /*reg*/) {
   // On x86, the result is always stashed in %EAX
   int ret;
   ret = ptraceKludge::deliverPtraceReturn(this, PTRACE_PEEKUSER, EAX*4, 0);
   return (Address)ret;
}

bool process::set_breakpoint_for_syscall_completion() {
	Address codeBase;
	codeBase = getPC( getPid() );
	readDataSpace( (void*)codeBase, 2, savedCodeBuffer, true);

	instruction insnTrap;
	generateBreakPoint(insnTrap);
	writeDataSpace((void *)codeBase, 2, insnTrap.ptr());

	inferiorrpc_cerr << "Set breakpoint at " << (void*)codeBase << endl;

	return true;
}

void process::clear_breakpoint_for_syscall_completion() {
	Address codeBase;
	codeBase = getPC( getPid() );
	writeDataSpace( (void*)codeBase, 2, savedCodeBuffer );

	inferiorrpc_cerr << "Cleared breakpoint at " << (void*)codeBase << endl;
}


void print_read_error_info(const relocationEntry entry, 
      pd_Function *&target_pdf, Address base_addr) {

    sprintf(errorLine, "  entry      : target_addr 0x%x\n",
	    (unsigned)entry.target_addr());
    logLine(errorLine);
    sprintf(errorLine, "               rel_addr 0x%x\n", (unsigned)entry.rel_addr());
    logLine(errorLine);
    sprintf(errorLine, "               name %s\n", (entry.name()).string_of());
    logLine(errorLine);

    sprintf(errorLine, "  target_pdf : symTabName %s\n",
	    (target_pdf->symTabName()).string_of());
    logLine(errorLine);    
    sprintf(errorLine , "              prettyName %s\n",
	    (target_pdf->symTabName()).string_of());
    logLine(errorLine);
    sprintf(errorLine , "              size %i\n",
	    target_pdf->size());
    logLine(errorLine);
    sprintf(errorLine , "              addr 0x%x\n",
	    (unsigned)target_pdf->addr());
    logLine(errorLine);

    sprintf(errorLine, "  base_addr  0x%x\n", (unsigned)base_addr);
    logLine(errorLine);
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
    
    if((target = const_cast<function_base *>(instr.iPgetCallee()))) {
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
//    Address insn_addr = instr.iPgetAddress(); 
    target_addr = instr.getTargetAddress();

    if(!target_addr) {  
	// this is either not a call instruction or an indirect call instr
	// that we can't get the target address
        target = 0;
        return false;
    }

    // see if there is a function in this image at this target address
    // if so return it
    pd_Function *pdf = 0;
    if( (pdf = owner->findFuncByAddr(target_addr,this)) ) {
        target = pdf;
        instr.set_callee(pdf);
	return true; // target found...target is in this image
    }

    // else, get the relocation information for this image
    const Object &obj = owner->getObject();
    const vector<relocationEntry> *fbt;
    if(!obj.get_func_binding_table_ptr(fbt)) {
	target = 0;
	return false; // target cannot be found...it is an indirect call.
    }

    // find the target address in the list of relocationEntries
    for(u_int i=0; i < fbt->size(); i++) {
	if((*fbt)[i].target_addr() == target_addr) {
	    // check to see if this function has been bound yet...if the
	    // PLT entry for this function has been modified by the runtime
	    // linker
	    pd_Function *target_pdf = 0;
	    if(hasBeenBound((*fbt)[i], target_pdf, base_addr)) {
                target = target_pdf;
                instr.set_callee(target_pdf);
	        return true;  // target has been bound
	    } 
	    else {
		// just try to find a function with the same name as entry 
		target = findFuncByName((*fbt)[i].name());
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
		    s += (*fbt)[i].name();
		    target = findFuncByName(s);
		    if(target){
	                return true;
		    }
		    s = string("__");
		    s += (*fbt)[i].name();
		    target = findFuncByName(s);
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


// hasBeenBound: returns true if the runtime linker has bound the
// function symbol corresponding to the relocation entry in at the address
// specified by entry and base_addr.  If it has been bound, then the callee 
// function is returned in "target_pdf", else it returns false.
bool process::hasBeenBound(const relocationEntry entry, 
			   pd_Function *&target_pdf, Address base_addr) {

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
        sprintf(errorLine, "read error in process::hasBeenBound addr 0x%x, pid=%d\n (readDataSpace returns 0)",(unsigned)got_entry,pid);
	logLine(errorLine);
	print_read_error_info(entry, target_pdf, base_addr);
        return false;
    }

    if( !( bound_addr == (entry.target_addr()+6+base_addr)) ) {
        // the callee function has been bound by the runtime linker
	// find the function and return it
        target_pdf = findFuncByAddr(bound_addr);
	if(!target_pdf){
            return false;
	}
        return true;	
    }
    return false;
}

#ifndef BPATCH_LIBRARY

timeUnit calcJiffyUnit() {
  // Determine the number of jiffies/sec by checking the clock idle time in
  // /proc/uptime against the jiffies idle time in /proc/stat

  FILE *tmp = P_fopen( "/proc/uptime", "r" );
  assert( tmp );
  double uptimeReal;
  assert( 1 == fscanf( tmp, "%*f %lf", &uptimeReal ) );
  fclose( tmp );
  tmp = P_fopen( "/proc/stat", "r" );
  assert( tmp );
  int uptimeJiffies;
  assert( 1 == fscanf( tmp, "%*s %*d %*d %*d %d", &uptimeJiffies ) );

  if (sysconf(_SC_NPROCESSORS_CONF) > 1) {
    // on SMP boxes, the first line is cumulative jiffies, the second line
    // is jiffies for cpu0 - on uniprocessors, this fscanf will fail as
    // there is only a single cpu line
    assert (1 == fscanf(tmp, "\ncpu0 %*d %*d %*d %d", &uptimeJiffies));
  }

  fclose( tmp );
  int intJiffiesPerSec = static_cast<int>( static_cast<double>(uptimeJiffies) 
					   / uptimeReal + 0.5 );
  timeUnit jiffy(fraction(1000000000LL, intJiffiesPerSec));
  return jiffy;
}

bool process::isLibhrtimeAvail() {
#ifdef HRTIME
  int result = ::isLibhrtimeAvail(&hr_cpu_link, getPid());
  return (result == 1);
#else
  return false;
#endif
}

void process::free_hrtime_link() {
#ifdef HRTIME
  int error = free_hrtime_struct(hr_cpu_link);
  if(error != 0) {
    cerr << "process::free_hrtime_link- Error in unmapping hrtime_struct for "
      " libhrtime\n";
  }
#endif
}

void process::initCpuTimeMgrPlt() {
  cpuTimeMgr->installLevel(cpuTimeMgr_t::LEVEL_ONE, &process::isLibhrtimeAvail,
			   getCyclesPerSecond(), timeBase::bNone(), 
			   &process::getRawCpuTime_hw, "hwCpuTimeFPtrInfo",
			   &process::free_hrtime_link);
  cpuTimeMgr->installLevel(cpuTimeMgr_t::LEVEL_TWO, &process::yesAvail, 
			   calcJiffyUnit(), timeBase::bNone(), 
			   &process::getRawCpuTime_sw, "swCpuTimeFPtrInfo");
}
#endif

fileDescriptor *getExecFileDescriptor(string filename,
				     int &,
				     bool)
{
  fileDescriptor *desc = new fileDescriptor(filename);
  return desc;
}

#if defined(USES_DYNAMIC_INF_HEAP)
static const Address lowest_addr = 0x0;
void inferiorMallocConstraints(Address near, Address &lo, Address &hi,
			       inferiorHeapType /* type */ )
{
  if (near)
    {
      lo = region_lo(near);
      hi = region_hi(near);  
    }
}

void inferiorMallocAlign(unsigned &size)
{
     /* 32 byte alignment.  Should it be 64? */
  size = (size + 0x1f) & ~0x1f;
}
#endif
