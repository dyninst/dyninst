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

// $Id: linux.C,v 1.83 2003/02/04 14:59:22 bernat Exp $

#include <fstream.h>

#include "dyninstAPI/src/process.h"
#include "dyninstAPI/src/dyn_lwp.h"

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
#ifndef BPATCH_LIBRARY
#include "common/h/Time.h"
#include "common/h/timing.h"
#include "paradynd/src/init.h"
#endif

#if defined(BPATCH_LIBRARY)
#include "dyninstAPI/src/addLibraryLinux.h"
#include "dyninstAPI/src/writeBackElf.h"
// #include "saveSharedLibrary.h" 
#endif

#ifdef HRTIME
#include "rtinst/h/RThwtimer-linux.h"
#endif

#ifdef PAPI
#include "papi.h"
#endif

// The following were defined in process.C
// Shouldn't they be in a header, then? -- TLM
extern debug_ostream attach_cerr;
extern debug_ostream inferiorrpc_cerr;
extern debug_ostream shmsample_cerr;
extern debug_ostream forkexec_cerr;
extern debug_ostream signal_cerr;

extern void generateBreakPoint(instruction &insn);

#if defined(PTRACEDEBUG) && !defined(PTRACEDEBUG_ALWAYS)
static bool debug_ptrace = false;
#endif

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

bool ptraceKludge::deliverPtrace(process *p, int req, Address addr,
				 Address data ) {
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
int ptraceKludge::deliverPtraceReturn(process *p, int req, Address addr,
				 Address data ) {
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

void printStackWalk( process *p ) {
  Frame theFrame = p->getDefaultLWP()->getActiveFrame();
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

bool process::continueWithForwardSignal( int sig ) {
   // formerly OS::osForwardSignal
   return (P_ptrace(PTRACE_CONT, pid, 1, sig) != -1);
}

void OS::osTraceMe(void) { P_ptrace(PTRACE_TRACEME, 0, 0, 0); }

process *findProcess( int );  // In process.C

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
   the inferior.  In this handler, the inferior signals us (SIG_REATTACH),
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
   SIG_REATTACH, which brings us here.  Here we reattach to the process and
   then help it re-execute the code that caused its SIGILL.  Having
   reattached, we receive the new SIGILL event and dispatch it as
   usual (in handleSigChild). */
static void sigill_handler(int sig, siginfo_t *si, void *unused)
{
     process *p;

     unused = 0; /* Suppress compiler warning of unused parameter */

     assert(sig == SIG_REATTACH);
     /* Determine the process that sent the signal.  On Linux (at
	least upto 2.2), we can only obtain this with the real-time
	signals, those numbered 33 or higher. */
     p = findProcess(si->si_pid);
     if (!p) {
	  fprintf(stderr, "Received SIGILL sent by unregistered or non-inferior process\n");
	  assert(0);
     }

     /* Synchronize with the SIGSTOP sent by inferior sigill handler */
     waitForInferiorSigillStop(p->getPid());

     /* Reattach, which will leave a pending SIGSTOP. */
     p->reattach();
     if (! p->isRunningIRPC())
	  /* If we got this signal when the inferior was not in an RPC,
	     then we need to reattach after we handle it.
	     FIXME: Why have we released the process for RPCs anyway? */
	  p->needsDetach = true;

     /* Resume the process.  We expect it to re-execute the code that
        generated the SIGILL.  Now that we are attached, we'll get the
        SIGILL event and handle it with handleSigChild as usual. */
     /* clear pending stop left by reattach */
     P_ptrace(PTRACE_CONT, p->getPid(), 0, 0);
     if (0 > waitpid(p->getPid(), NULL, 0))
	     perror("waitpid");
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
     if (0 > sigaction(SIG_REATTACH, &act, NULL)) {
	  perror("SIG_REATTACH sigaction");
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


/* The following two routines block SIG_REATTACH to prevent themselves from
   being reentered. */
int process::detach()
{
     int res, ret;
     sigset_t tmp, old;

     ret = 0;
     sigemptyset(&tmp);
     sigaddset(&tmp, SIG_REATTACH);
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
     sigaddset(&tmp, SIG_REATTACH);
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
		 if( sig == SIGTRAP && ( !p->reachedBootstrapState(begun) || p->inExec ) )
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
          // Attach used to be here, now handled in process.C
          ;
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

  // QUESTION: does this attach operation lead to a SIGTRAP being forwarded
  // to paradynd in all cases?  How about when we are attaching to an
  // already-running process?  (Seems that in the latter case, no SIGTRAP
  // is automatically generated)

  // step 1) /proc open: attach to the inferior process memory file
  dyn_lwp *lwp = new dyn_lwp(0, this);
  if (!lwp->openFD()) {
    delete lwp;
    return false;
  }
  lwps[0] = lwp;

  int fd = lwp->get_fd();
  if (fd < 0 ) {
    fprintf(stderr, "attach: open failed on %s: %s\n", procName, sys_errlist[errno]);
    return false;
  }
  // Only if we are really attaching rather than spawning the inferior
  // process ourselves do we need to call PTRACE_ATTACH
  if( createdViaAttach || createdViaFork || createdViaAttachToCreated ) {
	  attach_cerr << "process::attach() doing PTRACE_ATTACH" << endl;
    if( 0 != P_ptrace(PTRACE_ATTACH, getPid(), 0, 0) )
	{
		perror( "process::attach - PTRACE_ATTACH" );
		return false;
	}

    if (0 > waitpid(getPid(), NULL, 0)) {
      perror("process::attach - waitpid");
      exit(1);
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
			perror( "process::attach - continue 1" );
                }
    }
  }

  if (createdViaAttachToCreated)
    {
      // This case is a special situation. The process is stopped
      // in the exec() system call but we have not received the first 
      // TRAP because it has been caught by another process.

      /* lose race */
      sleep(1);
 

      /* continue, clearing pending stop */
      if (0 > P_ptrace(PTRACE_CONT, getPid(), 0, SIGCONT)) {
          perror("process::attach: PTRACE_CONT 1");
          return false;
      }
     

      if (0 > waitpid(getPid(), NULL, 0)) {
          perror("process::attach: WAITPID");
          return false;
      }
     

      /* continue, resending the TRAP to emulate the normal situation*/
      if ( 0 > kill(getPid(), SIGTRAP)){
	  perror("process::attach: KILL");
          return false;
      }

      if (0 > P_ptrace(PTRACE_CONT, getPid(), 0, SIGCONT)) {
          perror("process::attach: PTRACE_CONT 2");
          return false;
      }

      fprintf(stderr, "done attaching\n");
      status_ = neonatal;
      return true;

     } // end - if createdViaAttachToCreated

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

bool process::trapAtEntryPointOfMain(Address)
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
  if( getPC(getPid()) == (Address)dyninstlib_brk_addr){
	  dyninstlib_brk_addr = 0; //ccw 30 apr 2002 : SPLIT3
	  //dyninstlib_brk_addr and paradynlib_brk_addr may be the same
	  //if they are we dont want to get them mixed up. once we
	  //see this trap is due to dyninst, reset the addr so
	  //we can now catch the paradyn trap
    return(true);
  } else{
    return(false);
  }
}

void emitCallRel32(unsigned disp32, unsigned char *&insn);

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
  delete lwps[0];
  return (ptraceKludge::deliverPtrace(this, PTRACE_DETACH, 1, SIGCONT));
}

#ifdef BPATCH_LIBRARY
bool process::API_detach_(const bool cont) {
//  assert(cont);
  if (!checkStatus())
    return false;
  ptraceOps++; ptraceOtherOps++;
  delete lwps[0];
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
     Address w;               /* ptrace I/O buffer */
     unsigned len = sizeof(w); /* address alignment of ptrace I/O requests */
     unsigned cnt;

#if defined(BPATCH_LIBRARY)
#if defined(i386_unknown_linux2_0)
	if(collectSaveWorldData &&  ((Address) inTraced) > getDyn()->getlowestSObaseaddr() ){
		shared_object *sh_obj = NULL;
		bool result = false;
		for(unsigned int i = 0; shared_objects && !result && i<shared_objects->size();i++){
			sh_obj = (*shared_objects)[i];
			result = sh_obj->isinText((Address) inTraced);
		}
		if( result  ){
			if(strcmp(findFuncByAddr((Address)inTraced)->prettyName().c_str(), 
						"__libc_sigaction")){
				//for linux we instrument sigactiont to watch libraries
				//being loaded. dont consider libc.so mutated because of 
				//this	
				/*printf(" write at %lx in %s amount %x insn: %x \n", 
				(off_t)inTraced, sh_obj->getName().c_str(), nbytes,
				 *(unsigned int*) inSelf);
				 */
				sh_obj->setDirty();	
			}
		}
	}
#endif
#endif

     ptraceOps++; ptraceBytes += nbytes;

     if (0 == nbytes)
	  return true;

     if ((cnt = ((Address)ap) % len)) {
	  /* Start of request is not aligned. */
	  unsigned char *p = (unsigned char*) &w;
	  
	  /* Read the segment containing the unaligned portion, edit
	     in the data from DP, and write the segment back. */
	  errno = 0;
	  w = P_ptrace(PTRACE_PEEKTEXT, pid, (Address) (ap-cnt), 0);
	  if (errno)
	       return false;
	  for (unsigned i = 0; i < len-cnt && i < nbytes; i++)
	       p[cnt+i] = dp[i];
	  if (0 > P_ptrace(PTRACE_POKETEXT, pid, (Address) (ap-cnt), w))
	       return false;

	  if (len-cnt >= nbytes) 
	       return true; /* done */
	  
	  dp += len-cnt;
	  ap += len-cnt;
	  nbytes -= len-cnt;
     }	  
	  
     /* Copy aligned portion */
     while (nbytes >= len) {
	  assert(0 == ((Address)ap) % len);
	  memcpy(&w, dp, len);
	  if (0 > P_ptrace(PTRACE_POKETEXT, pid, (Address) ap, w))
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
	  w = P_ptrace(PTRACE_PEEKTEXT, pid, (Address) ap, 0);
	  if (errno)
	       return false;
	  for (unsigned i = 0; i < nbytes; i++)
	       p[i] = dp[i];
	  if (0 > P_ptrace(PTRACE_POKETEXT, pid, (Address) ap, w))
	       return false;
     }

     return true;
}

bool process::readDataSpace_(const void *inTraced, u_int nbytes, void *inSelf) {
     const unsigned char *ap = (const unsigned char*) inTraced;
     unsigned char *dp = (unsigned char*) inSelf;
     int pid = getPid();
     Address w;               /* ptrace I/O buffer */
     unsigned len = sizeof(w); /* address alignment of ptrace I/O requests */
     unsigned cnt;

     ptraceOps++; ptraceBytes += nbytes;

     if (0 == nbytes)
	  return true;

     if ((cnt = ((Address)ap) % len)) {
	  /* Start of request is not aligned. */
	  unsigned char *p = (unsigned char*) &w;

	  /* Read the segment containing the unaligned portion, and
             copy what was requested to DP. */
	  errno = 0;
	  w = P_ptrace(PTRACE_PEEKTEXT, pid, (Address) (ap-cnt), w);
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
	  w = P_ptrace(PTRACE_PEEKTEXT, pid, (Address) ap, 0);
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
	  w = P_ptrace(PTRACE_PEEKTEXT, pid, (Address) ap, 0);
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
       cwd = env_value.c_str() + 4; // skip past "PWD=" or "CWD="
       attach_cerr << "get_ps_stuff: using PWD value of: " << cwd << endl;
       if( path.length() )
	 break;
     } else if (env_value.prefixed_by("PATH=")) {
       path = env_value.c_str() + 5; // skip past the "PATH="
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

#if !defined(BPATCH_LIBRARY)

rawTime64 dyn_lwp::getRawCpuTime_hw()
{
  rawTime64 result = 0;
#ifdef HRTIME
  result = hrtimeGetVtime(hr_cpu_link);
#endif
  
#ifdef PAPI
  result = papi->getCurrentVirtCycles();
#endif
  
  if (result < hw_previous_) {
    logLine("********* time going backwards in paradynd **********\n");
    result = hw_previous_;
  }
  else 
    hw_previous_ = result;
  
  return result;
}

rawTime64 dyn_lwp::getRawCpuTime_sw()
{
  rawTime64 result = 0;
  int bufsize = 150, utime, stime;
  char procfn[bufsize], *buf;

  sprintf( procfn, "/proc/%d/stat", proc_->getPid() );

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
      result = static_cast<rawTime64>(utime) + static_cast<rawTime64>(stime);
      break;
    }

    delete [] buf;
    shmsample_cerr << "Inferior CPU time buffer expansion (" << bufsize << ")" << endl;
    bufsize = bufsize * 2;

    P_close( fd );
  } while ( true );

  delete [] buf;
  P_close(fd);

  if (result < sw_previous_) {
    logLine("********* time going backwards in paradynd **********\n");
    result = sw_previous_;
  }
  else 
    sw_previous_ = result;

  return result;
}

class instReqNode;

bool process::catchupSideEffect( Frame & /* frame */, instReqNode * /* inst */ )
{
  return true;
}
#endif

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
#if defined(ia64_unknown_linux2_4)
/* FIXME: migrate to linux-[ia64|x86].C, or rewrite to handle 64-bit elfs. */
bool process::dumpImage( string /* imageFileName */ ) { return true; }
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
    system(command.c_str());

    // now open the copy
    newFd = open(imageFileName.c_str(), O_RDWR, 0);
    if (newFd < 0) {
	// log error
	return false;
    }

    Elf *elfp = elf_begin(newFd, ELF_C_READ, 0);
    Elf_Scn *scn = 0;
    Address baseAddr = 0;
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
    const pdvector<relocationEntry> *fbt;
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

bool process::isPapiAvail() {
  return isPapiInitialized();
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
#ifdef HRTIME
  cpuTimeMgr->installLevel(cpuTimeMgr_t::LEVEL_ONE, &process::isLibhrtimeAvail,
			   getCyclesPerSecond(), timeBase::bNone(), 
			   &process::getRawCpuTime_hw, "hwCpuTimeFPtrInfo",
			   &process::free_hrtime_link);
#endif

#ifdef PAPI
  cpuTimeMgr->installLevel(cpuTimeMgr_t::LEVEL_ONE, &process::isPapiAvail,
			   getCyclesPerSecond(), timeBase::bNone(), 
			   &process::getRawCpuTime_hw, "hwCpuTimeFPtrInfo");

#endif

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
void process::inferiorMallocConstraints(Address near, Address &lo, Address &hi,
			       inferiorHeapType /* type */ )
{
  if (near)
    {
      lo = region_lo(near);
      hi = region_hi(near);  
    }
}

void process::inferiorMallocAlign(unsigned &size)
{
     /* 32 byte alignment.  Should it be 64? */
  size = (size + 0x1f) & ~0x1f;
}
#endif

bool dyn_lwp::openFD(void)
{
  char procName[128];
  sprintf(procName, "/proc/%d/mem", (int) proc_->getPid());
  fd_ = P_open(procName, O_RDWR, 0);
  if (fd_ < 0) return false;
  return true;
}

void dyn_lwp::closeFD()
{
  if (fd_) close(fd_);
}
