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

// $Id: linux.C,v 1.113 2003/10/21 17:22:18 bernat Exp $

#include <fstream>

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
#include "dyninstAPI/src/signalhandler.h"
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

#include "dyninstAPI/src/addLibraryLinux.h"
#include "dyninstAPI/src/writeBackElf.h"
// #include "saveSharedLibrary.h" 

#ifdef PAPI
#include "papi.h"
#endif

// The following were defined in process.C
// Shouldn't they be in a header, then? -- TLM

extern unsigned enable_pd_attach_detach_debug;

#if ENABLE_DEBUG_CERR == 1
#define attach_cerr if (enable_pd_attach_detach_debug) cerr
#else
#define attach_cerr if (0) cerr
#endif /* ENABLE_DEBUG_CERR == 1 */

extern unsigned enable_pd_inferior_rpc_debug;

#if ENABLE_DEBUG_CERR == 1
#define inferiorrpc_cerr if (enable_pd_inferior_rpc_debug) cerr
#else
#define inferiorrpc_cerr if (0) cerr
#endif /* ENABLE_DEBUG_CERR == 1 */

extern unsigned enable_pd_shm_sampling_debug;

#if ENABLE_DEBUG_CERR == 1
#define shmsample_cerr if (enable_pd_shm_sampling_debug) cerr
#else
#define shmsample_cerr if (0) cerr
#endif /* ENABLE_DEBUG_CERR == 1 */

extern unsigned enable_pd_fork_exec_debug;

#if ENABLE_DEBUG_CERR == 1
#define forkexec_cerr if (enable_pd_fork_exec_debug) cerr
#else
#define forkexec_cerr if (0) cerr
#endif /* ENABLE_DEBUG_CERR == 1 */

extern unsigned enable_pd_signal_debug;

#if ENABLE_DEBUG_CERR == 1
#define signal_cerr if (enable_pd_signal_debug) cerr
#else
#define signal_cerr if (0) cerr
#endif /* ENABLE_DEBUG_CERR == 1 */

extern void generateBreakPoint(instruction &insn);

#if defined(PTRACEDEBUG) && !defined(PTRACEDEBUG_ALWAYS)
static bool debug_ptrace = false;
#endif

bool ptraceKludge::haltProcess(process *p) {
  bool wasStopped = (p->status() == stopped);
  if (p->status() != neonatal && !wasStopped) {
    p->pause();
/*
    if (!p->loopUntilStopped()) {
      assert(0);
    }
*/
  }
  
  return wasStopped;
}

void ptraceKludge::continueProcess(process *p, const bool wasStopped) {
  // First handle the cases where we shouldn't issue a PTRACE_CONT:
  if (p->status() == neonatal) return;
  if (wasStopped) return;

  // Choose either one of the following methods to continue a process.
  // The choice must be consistent with that in process::continueProc_ and stop_

  p->continueProc();
/*
  if (P_ptrace(PTRACE_CONT, p->pid, 1, SIGCONT) == -1) {
      perror("error in continueProcess");
      assert(0);
  }
*/
}

bool ptraceKludge::deliverPtrace(process *p, int req, Address addr,
				 Address data ) {
  bool halted = haltProcess(p);
  bool ret = (P_ptrace(req, p->getPid(), addr, data) != -1);
  continueProcess(p, halted);
  return ret;
}


// Some ptrace requests in Linux return the value rather than storing at the address in data
// (especially PEEK requests)
// - nash
int ptraceKludge::deliverPtraceReturn(process *p, int req, Address addr,
				 Address data ) {
  bool halted = true;

  if (req != PTRACE_DETACH){
	halted = haltProcess(p);
  }

  int ret = P_ptrace(req, p->getPid(), addr, data);

  if (req != PTRACE_DETACH){
	continueProcess(p, halted);
  }

  return ret;
}

/* ********************************************************************** */

void printStackWalk( process *p ) {
  Frame theFrame = p->getProcessLWP()->getActiveFrame();
  while (true) {
    // do we have a match?
    const Address framePC = theFrame.getPC();
    inferiorrpc_cerr << "stack frame pc @ " << (void*)framePC << endl;
    
    if (theFrame.isLastFrame(p))
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
     return (P_kill(getPid(), SIGSTOP) != -1); 
}

bool process::continueWithForwardSignal( int sig ) {
   // formerly OS::osForwardSignal
   return (P_ptrace(PTRACE_CONT, pid, 1, sig) != -1);
}

void OS::osTraceMe(void) { P_ptrace(PTRACE_TRACEME, 0, 0, 0); }

// Wait for a process event to occur, then map it into
// the why/what space (a la /proc status variables)

int decodeRTSignal(process *proc,
                   procSignalWhy_t &why,
                   procSignalWhat_t &what,
                   procSignalInfo_t &info)
{
   // We've received a signal we believe was sent
   // from the runtime library. Check the RT lib's
   // status variable and return it.
   // These should be made constants
   if (!proc) return 0;

   pdstring status_str = pdstring("DYNINST_instSyscallState");
   pdstring arg_str = pdstring("DYNINST_instSyscallArg1");

   int status;
   Address arg;

   bool err = false;
   Address status_addr = proc->findInternalAddress(status_str, false, err);
   if (err) {
      // Couldn't find symbol
      return 0;
   }

   if (!proc->readDataSpace((void *)status_addr, sizeof(int), 
                            &status, true)) {
      return 0;
   }

   if (status == 0) {
      return 0; // Nothing to see here
   }
    
   Address arg_addr = proc->findInternalAddress(arg_str, false, err);
   if (err) {
      return 0;
   }
    
   if (!proc->readDataSpace((void *)arg_addr, sizeof(Address),
                            &arg, true))
      assert(0);
   info = (procSignalInfo_t)arg;
   fprintf(stderr, "Signal from RT: %d, %d\n",
           status, info);
   
   switch(status) {
     case 1:
        /* Entry to fork */
        why = procSyscallEntry;
        what = SYS_fork;
        break;
     case 2:
        why = procSyscallExit;
        what = SYS_fork;
        break;
     case 3:
        /* Entry to exec */
        why = procSyscallEntry;
        what = SYS_exec;
        break;
     case 4:
        /* Exit of exec, unused */
        break;
     case 5:
        /* Entry of exit, used for the callback. We need to trap before
           the process has actually exited as the callback may want to
           read from the process */
        why = procSyscallEntry;
        what = SYS_exit;
        break;
     default:
        assert(0);
        break;
   }
   return 1;

}

process *decodeProcessEvent(int pid, 
                            procSignalWhy_t &why,
                            procSignalWhat_t &what,
                            procSignalInfo_t &info,
                            bool block) {
    int options = 0;
    if (!block) options |= WNOHANG;
    
    int result = 0, status = 0;
    process *proc = NULL;
    result = waitpid( pid, &status, options );
    // Translate the signal into a why/what combo.
    // We can fake results here as well: translate a stop in fork
    // to a (SYSEXIT,fork) pair. Don't do that yet.
    if (result > 0) {
        proc = process::findProcess(result);

        if (proc) {
            // Processes' state is saved in preSignalStatus()
            proc->savePreSignalStatus();
            // Got a signal, process is stopped.
            proc->status_ = stopped;
        }
        
        if (WIFEXITED(status)) {
            // Process exited via signal
            why = procExitedNormally;
            what = WEXITSTATUS(status);
            }
        else if (WIFSIGNALED(status)) {
            why = procExitedViaSignal;
            what = WTERMSIG(status);
        }
        else if (WIFSTOPPED(status)) {
            // More interesting (and common) case
            // This is where return value faking would occur
            // as well. procSignalled is a generic return.
            // For example, we translate SIGILL to SIGTRAP
            // in a few cases
            why = procSignalled;
            what = WSTOPSIG(status);

            if (what == SIGILL) {
                // Check for system call exit and handle
                // We're aborting on Linux, can ignore this
                
            }

            if (what == SIGSTOP) {
                decodeRTSignal(proc, why, what, info);
            }
	    
	    if (what == SIGTRAP) {
	      // We use int03s (traps) to do instrumentation when there
	      // isn't enough room to insert a branch.
	      why = procInstPointTrap;
	    }

	    if (what == SIGCHLD) {
	      // Linux fork() sends a SIGCHLD once the fork has been created
	      why = procForkSigChild;
	    }

            if (what == SIGILL) {
                // The following is more correct, but breaks.
                // Problem is getting the frame requires a ptrace...
                // which calls loopUntilStopped. Which calls us.
                //Frame frame = proc->getProcessLWP()->getActiveFrame();
                //Address pc = frame.getPC();
                Address pc = getPC(proc->getPid());
                
                if (pc == proc->rbrkAddr() ||
                    pc == proc->main_brk_addr ||
                    pc == proc->dyninstlib_brk_addr) {
                    what = SIGTRAP;
                }
            }
        }
    }
    else if (result < 0 && errno == ECHILD)
	    return NULL; /* nothing to wait for */
    else if (result < 0) {
        perror("decodeProcessEvent: waitpid failure");
    }
    return proc;
}


bool process::installSyscallTracing() {
    // We mimic system call tracing via instrumentation
    AstNode *returnVal = new AstNode(AstNode::ReturnVal, (void *)0);
    AstNode *arg0 = new AstNode(AstNode::Param, (void *)0);

    // Pre-fork - is this strictly necessary?
    tracingRequests += new instMapping("__libc_fork", "DYNINST_instForkEntry",
                                       FUNC_ENTRY);
    // Post-fork:
    instMapping *forkExit = new instMapping("__libc_fork", "DYNINST_instForkExit",
                                            FUNC_EXIT|FUNC_ARG,
                                            returnVal);
    forkExit->dontUseTrampGuard();
    tracingRequests += forkExit;

    // Pre-exec: get the exec'ed file name
    tracingRequests += new instMapping("execve", "DYNINST_instExecEntry",
                                       FUNC_ENTRY|FUNC_ARG,
                                       arg0);

    // Post-exec: handled for us by the system

    // Pre-exit: get the return code
    tracingRequests += new instMapping("exit", "DYNINST_instExitEntry",
                                       FUNC_ENTRY|FUNC_ARG,
                                       arg0);

    // Post-exit: handled for us by the system


    removeAst(arg0);
    removeAst(returnVal);
    

    return true;
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

  ret = P_ptrace(PTRACE_CONT, getPid(), 1, 0);

  if (ret == -1)
  {
      perror("continueProc_()");
  }
  
  return ret != -1;
}

bool process::terminateProc_()
{
  if (!checkStatus()) 
    return false;

  if( kill( getPid(), SIGKILL ) != 0 )
    return false;
  else
    return true;
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
   ptraceOps++;
   ptraceOtherOps++;
   return (ptraceKludge::deliverPtrace(this, PTRACE_DETACH, 1, SIGCONT));
}

bool process::API_detach_(const bool cont) {
//  assert(cont);
  if (!checkStatus())
    return false;
  ptraceOps++; ptraceOtherOps++;
  if (!cont) P_kill(pid, SIGSTOP);
  return (ptraceKludge::deliverPtrace(this, PTRACE_DETACH, 1, SIGCONT));
}

bool process::dumpCore_(const pdstring/* coreFile*/) { return false; }

bool process::writeTextWord_(caddr_t inTraced, int data) {
//  cerr << "writeTextWord @ " << (void *)inTraced << endl; cerr.flush();
  return writeDataSpace_(inTraced, sizeof(int), (caddr_t) &data);
}

bool process::writeTextSpace_(void *inTraced, u_int amount, const void *inSelf) {
//  cerr << "writeTextSpace pid=" << getPid() << ", @ " << (void *)inTraced << " len=" << amount << endl; cerr.flush();
  return writeDataSpace_(inTraced, amount, inSelf);
}

bool process::readTextSpace_(void *inTraced, u_int amount, const void *inSelf) {
  return readDataSpace_( inTraced, amount, const_cast<void*>( inSelf ) );
}

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

// You know, /proc/*/exe is a perfectly good link (directly to the inode) to
// the executable file, who cares where the executable really is, we can open
// this link. - nash
pdstring process::tryToFindExecutable(const pdstring & /* iprogpath */, int pid) {
  return pdstring("/proc/") + pdstring(pid) + "/exe";
}

void process::recognize_threads(pdvector<unsigned> * /*completed_lwps*/) {
   // implement when handling forks for linux multi-threaded programs
}

/*
 * The old, ugly version that we don't need but can waste space anyhow
 * /
pdstring process::tryToFindExecutable(const pdstring &iprogpath, int pid) {
   // returns empty string on failure.
   // Otherwise, returns a full-path-name for the file.  Tries every
   // trick to determine the full-path-name, even though "progpath" may be
   // unspecified (empty string).
   
   // Remember, we can always return the empty string...no need to
   // go nuts writing the world's most complex algorithm.

   attach_cerr << "welcome to tryToFindExecutable; progpath=" << iprogpath << ", pid=" << pid << endl;

   const pdstring progpath = expand_tilde_pathname(iprogpath);

   // Trivial case: if "progpath" is specified and the file exists then nothing needed
   if (exists_executable(progpath)) {
     attach_cerr << "tryToFindExecutable succeeded immediately, returning "
		 << progpath << endl;
     return progpath;
   }

   attach_cerr << "tryToFindExecutable failed on filename " << progpath << endl;

   pdstring argv0, path, cwd;

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
     pdstring env_value = extract_pdstring( procfd, (char*)strptr );
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

   argv0 = extract_pdstring( procfd, (char*)0 );
   close( procfd );

   if ( argv0.length() && path.length() && cwd.length() ) {
     // the following routine is implemented in the util lib.
     pdstring result;
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
#ifdef PAPI
papiMgr* dyn_lwp::papi() {

  return proc()->getPapiMgr();

}
#endif
#endif


#if !defined(BPATCH_LIBRARY)

rawTime64 dyn_lwp::getRawCpuTime_hw()
{
  rawTime64 result = 0;
  
#ifdef PAPI
  result = papi()->getCurrentVirtCycles();
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
    /* make sure the process is stopped in the eyes of ptrace */
    bool haveStopped = false;
    stop_();
    
    // Loop handling signals until we receive a sigstop. Handle
    // anything else.
    while (!haveStopped) {
        procSignalWhy_t why;
        procSignalWhat_t what;
        procSignalInfo_t info;
        if(hasExited()) return false;
        process *proc = decodeProcessEvent(getPid(), why, what, info, true);
        assert(proc == NULL ||
               proc == this);
        if (didProcReceiveSignal(why) &&
            what == SIGSTOP) {
            haveStopped = true;
            // Don't call the general handler
        }
        else {
           if(proc)  handleProcessEvent(this, why, what, info);
        }
    }
    return true;
}

procSyscall_t decodeSyscall(process * /*p*/, procSignalWhat_t what)
{
    switch (what) {
  case SYS_fork:
      return procSysFork;
      break;
  case SYS_exec:
      return procSysExec;
      break;
  case SYS_exit:
      return procSysExit;
      break;
  default:
      return procSysOther;
      break;
    }
    assert(0);
    return procSysOther;
}

bool process::dumpImage( pdstring imageFileName ) {
	/* What we do is duplicate the original file,
	   and replace the copy's .text section with
	   the (presumably instrumented) in-memory
	   executable image.  Note that we don't seem
	   to be concerned with making sure that we
	   actually grab the instrumentation code itself... */
	
	/* Acquire the filename. */
	image * theImage = getImage();
	pdstring originalFileName = theImage->file();
	
	/* Use system() to execute the copy. */
	pdstring copyCommand = "cp " + originalFileName + " " + imageFileName;
    system( copyCommand.c_str() );

	/* Open the copy so we can use libelf to find the .text section. */
	int copyFD = open( imageFileName.c_str(), O_RDWR, 0 );
	if( copyFD < 0 ) { return false; }

	/* Start up the elven widgetry. */
	Elf * elfPointer = elf_begin( copyFD, ELF_C_READ, NULL );
	char * elfIdent = elf_getident( elfPointer, NULL );
	char elfClass = elfIdent[ EI_CLASS ];
	
	bool is64Bits;
	switch( elfClass ) {
		case ELFCLASSNONE:
			/* Shouldn't happen. */
			elf_end( elfPointer );
			close( copyFD );
			return false;

		case ELFCLASS32:
			is64Bits = false;
			break;
			
		case ELFCLASS64:
			is64Bits = true;
			break;
			
		default:
			/* Can't happen. */
			assert( 0 );
			break;
		} /* end elfClass switch */

    /* Acquire the shared names pointer. */
    const char * sharedNames = NULL;
    if( is64Bits ) {
    	Elf64_Ehdr * elfHeader = elf64_getehdr( elfPointer );
    	assert( elfHeader != NULL );
    	
    	Elf_Scn * elfSection = elf_getscn( elfPointer, elfHeader->e_shstrndx );
    	assert( elfSection != NULL );

    	Elf_Data * elfData = elf_getdata( elfSection, 0 );
    	assert( elfData != NULL );
    	    	    	
    	sharedNames = (const char *) elfData->d_buf;
    	}
    else {
    	Elf32_Ehdr * elfHeader = elf32_getehdr( elfPointer );
    	assert( elfHeader != NULL );
    	
    	Elf_Scn * elfSection = elf_getscn( elfPointer, elfHeader->e_shstrndx );
    	assert( elfSection != NULL );

    	Elf_Data * elfData = elf_getdata( elfSection, 0 );
    	assert( elfData != NULL );
    	    	    	
    	sharedNames = (const char *) elfData->d_buf;
		}   
    
	/* Iterate over the sections to find the text section's
	   offset, length, and base address. */
	Address offset = 0;
	Address length = 0;
	Address baseAddr = 0;
	   
	Elf_Scn * elfSection = NULL;
	while( (elfSection = elf_nextscn( elfPointer, elfSection )) != NULL ) {
		if( is64Bits ) {
			Elf64_Shdr * elfSectionHeader = elf64_getshdr( elfSection );
			const char * name = (const char *) & sharedNames[ elfSectionHeader->sh_name ];

			if( strcmp( name, ".text" ) == 0 ) {
				offset = elfSectionHeader->sh_offset;
				length = elfSectionHeader->sh_size;
				baseAddr = elfSectionHeader->sh_addr;
				break;
				} /* end if we've found the text section */
			} else {
			Elf32_Shdr * elfSectionHeader = elf32_getshdr( elfSection );
			const char * name = (const char *) & sharedNames[ elfSectionHeader->sh_name ];

			if( strcmp( name, ".text" ) == 0 ) {
				offset = elfSectionHeader->sh_offset;
				length = elfSectionHeader->sh_size;
				baseAddr = elfSectionHeader->sh_addr;
				break;
				} /* end if we've found the text section */
			}
		} /* end iteration over sections */

	/* Copy the code out of the mutatee. */
	char * codeBuffer = (char *)malloc( length );
	assert( codeBuffer != NULL );

	if( ! readTextSpace_( (void *) baseAddr, length, codeBuffer ) ) {
		free( codeBuffer );
		elf_end( elfPointer );
		close( copyFD );
		return false;
		}

	/* Write that code to the image file. */
    lseek( copyFD, offset, SEEK_SET );
    write( copyFD, codeBuffer, length );
    
    /* Clean up. */
    free( codeBuffer );
    elf_end( elfPointer );
    close( copyFD );
    return true;
}

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
   fprintf(stderr, "this = 0x%x, looking up 0x%x\n",
           this, target_addr);
   
   if( (pdf = this->findFuncByAddr(target_addr))) {
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
	    pdvector<function_base *> pdfv;
	    bool found = findAllFuncsByName((*fbt)[i].name(), pdfv);
	    if(found) {
	       assert(pdfv.size());
#ifdef BPATCH_LIBRARY
	       if(pdfv.size() > 1)
		   cerr << __FILE__ << ":" << __LINE__ 
			<< ": WARNING:  findAllFuncsByName found " 
			<< pdfv.size() << " references to function " 
			<< (*fbt)[i].name() << ".  Using the first.\n";
#endif
	       target = pdfv[0];
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

               pdstring s("_");
	       s += (*fbt)[i].name();
	       found = findAllFuncsByName(s, pdfv);
	       if(found) {
		  assert(pdfv.size());
#ifdef BPATCH_LIBRARY
		  if(pdfv.size() > 1)
		     cerr << __FILE__ << ":" << __LINE__ 
			  << ": WARNING: findAllFuncsByName found " 
			  << pdfv.size() << " references to function " 
			  << s << ".  Using the first.\n";
#endif
		  target = pdfv[0];
		  return true;
	       }
		    
	       s = pdstring("__");
	       s += (*fbt)[i].name();
	       found = findAllFuncsByName(s, pdfv);
	       if(found) {
		  assert(pdfv.size());
#ifdef BPATCH_LIBRARY
		  if(pdfv.size() > 1)
		     cerr << __FILE__ << ":" << __LINE__ 
			  << ": WARNING: findAllFuncsByName found " 
			  << pdfv.size() << " references to function "
			  << s << ".  Using the first.\n";
#endif
		  target = pdfv[0];
		  return true;
	       }
#ifdef BPATCH_LIBRARY
	       else
		  cerr << __FILE__ << ":" << __LINE__
		       << ": WARNING: findAllFuncsByName found no "
		       << "matches for function " << (*fbt)[i].name() 
		       << " or its possible aliases\n";
#endif
            }
         }
         target = 0;
         return false;
      }
   }
   target = 0;
   return false;  
}

fileDescriptor *getExecFileDescriptor(pdstring filename,
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


bool dyn_lwp::threadLWP_attach_() {
   assert( false && "threads not yet supported on Linux");
   return false;
}

bool dyn_lwp::processLWP_attach_() {
   // step 1) /proc open: attach to the inferior process memory file
   char procName[128];
   sprintf(procName, "/proc/%d/mem", (int) proc_->getPid());
   fd_ = P_open(procName, O_RDWR, 0);
   if (fd_ < 0) return false;
   
   bool running = false;
   if( proc_->wasCreatedViaAttach() )
      running = proc_->isRunning_();
   
   // QUESTION: does this attach operation lead to a SIGTRAP being forwarded
   // to paradynd in all cases?  How about when we are attaching to an
   // already-running process?  (Seems that in the latter case, no SIGTRAP
   // is automatically generated)
   

   // Only if we are really attaching rather than spawning the inferior
   // process ourselves do we need to call PTRACE_ATTACH
   if(proc_->wasCreatedViaAttach() || proc_->wasCreatedViaFork() || 
      proc_->wasCreatedViaAttachToCreated())
   {
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

   if(proc_->wasCreatedViaAttach() )
   {
      // If the process was running, it will need to be restarted, as
      // PTRACE_ATTACH kills it
      // Actually, the attach process contructor assumes that the process is
      // running.  While this is foolish, let's play along for now.
      if( proc_->status() != running || !proc_->isRunning_() ) {
         if( 0 != P_ptrace(PTRACE_CONT, getPid(), 0, 0) )
         {
            perror( "process::attach - continue 1" );
         }
      }
   }

   if(proc_->wasCreatedViaAttachToCreated())
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

      proc_->status_ = neonatal;
   } // end - if createdViaAttachToCreated

   return true;
}

void dyn_lwp::detach_()
{
   assert(is_attached());  // dyn_lwp::detach() shouldn't call us otherwise
   if (fd_) close(fd_);
}

void loadNativeDemangler() {}
