/*
 * Copyright (c) 1996-2004 Barton P. Miller
 * 
 * We provide the Paradyn Parallel Performance Tools (below
 * described as "Paradyn") on an AS IS basis, and do not warrant its
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

// $Id: unix.C,v 1.140 2005/08/05 22:23:18 bernat Exp $

#include "common/h/headers.h"
#include "common/h/String.h"
#include "common/h/Vector.h"
#include "dyninstAPI/src/showerror.h"
#include "dyninstAPI/src/os.h"
#include "dyninstAPI/src/util.h"


#ifndef BPATCH_LIBRARY
#include "paradynd/src/main.h"  // for "tp" ?
#include "paradynd/src/pd_process.h" // for class pd_process
#endif

// the following are needed for handleSigChild
#include "dyninstAPI/src/signalhandler.h"
#include "dyninstAPI/src/process.h"
#include "dyninstAPI/src/dyn_lwp.h"
#include "dyninstAPI/src/dyn_thread.h"
#include "dyninstAPI/src/instP.h"
#include "dyninstAPI/src/stats.h"

/*****************************************************************************
 * forkNewProcess: starts a new process, setting up trace and io links between
 *                the new process and the daemon
 * Returns true if succesfull.
 * 
 * Arguments:
 *   file: file to execute
 *   dir: working directory for the new process
 *   argv: arguments to new process
 *   argp: environment variables for new process
 *   inputFile: where to redirect standard input
 *   outputFile: where to redirect standard output
 *   traceLink: handle or file descriptor of trace link (read only)
//removed all ioLink related code for output redirection
 *   ioLink: handle or file descriptor of io link (read only)
 *   pid: process id of new process
 *   tid: thread id for main thread (needed by WindowsNT)
 *   procHandle: handle for new process (needed by WindowsNT)
 *   thrHandle: handle for main thread (needed by WindowsNT)
 ****************************************************************************/
bool forkNewProcess(pdstring &file, pdstring dir, pdvector<pdstring> *argv, 
		    pdvector<pdstring> *envp,
                    pdstring inputFile, pdstring outputFile, int &traceLink,
                    int &pid, int & /*tid*/, int & /*procHandle*/,
                    int & /*thrHandle*/, int stdin_fd, int stdout_fd, int stderr_fd)
{
#ifndef BPATCH_LIBRARY
   // Strange, but using socketpair here doesn't seem to work OK on SunOS.
   // Pipe works fine.
   // r = P_socketpair(AF_UNIX, SOCK_STREAM, (int) NULL, tracePipe);
   int tracePipe[2];
   int r = P_pipe(tracePipe);
   if (r) {
      // P_perror("socketpair");
      pdstring msg = pdstring("Unable to create trace pipe for program '") + file +
         pdstring("': ") + pdstring(strerror(errno));
      showErrorCallback(68, msg);
      return false;
   }

   /* removed for output redirection
   // ioPipe is used to redirect the child's stdout & stderr to a pipe which is in
   // turn read by the parent via the process->ioLink socket.
   int ioPipe[2];

   // r = P_socketpair(AF_UNIX, SOCK_STREAM, (int) NULL, ioPipe);
   r = P_pipe(ioPipe);
   if (r) {
	// P_perror("socketpair");
   pdstring msg = pdstring("Unable to create IO pipe for program '") + file +
   pdstring("': ") + pdstring(sys_errlist[errno]);
	showErrorCallback(68, msg);
	return false;
   }
   */
#endif

   //
   // WARNING This code assumes that vfork is used, and a failed exec will
   //   corectly change failed in the parent process.
   //
    
   errno = 0;
   pid = fork();

   if (pid != 0) {

      // *** parent

#if (defined(BPATCH_LIBRARY) && !defined(alpha_dec_osf4_0))
      /*
       * On Irix, errno sometimes seems to have a non-zero value after
       * the fork even though it succeeded.  For now, if we're using fork
       * and not vfork, we will check the return code of fork to determine
       * if there was error, instead of relying on errno (so make sure the
       * condition for this section of code is the same as the condition for
       * using fork instead of vfork above). - brb
       */
      if (pid == -1)
#else
         if (errno)
#endif
         {
            sprintf(errorLine, "Unable to start %s: %s\n", file.c_str(), 
                    strerror(errno));
            logLine(errorLine);
            showErrorCallback(68, (const char *) errorLine);
            return false;
         }

#ifndef BPATCH_LIBRARY
      close(tracePipe[1]);
	   // parent never writes trace records; it only receives them.

      /* removed for output redirection
         close(ioPipe[1]);
         // parent closes write end of io pipe; child closes its read end.
         // pipe output goes to the parent's read fd (ret->ioLink); pipe input
         // comes from the child's write fd.  In short, when the child writes to
         // its stdout/stderr, it gets sent to the pipe which in turn sends it to
         // the parent's ret->ioLink fd for reading.

         //ioLink = ioPipe[0];
         */

      traceLink = tracePipe[0];
#endif
      return true;

   } else if (pid == 0) {
      // *** child

#ifndef BPATCH_LIBRARY
      // handle stdio.

      /* removed for output redirection We only write to ioPipe.  Hence we
      // close ioPipe[0], the read end.  Then we call dup2() twice to assign
      // our stdout and stderr to the write end of the pipe.
      // close(ioPipe[0]);
   
      //dup2(ioPipe[1], 1);

	
      // assigns fd 1 (stdout) to be a copy of ioPipe[1].  (Since stdout is
      // already in use, dup2 will first close it then reopen it with the
      // characteristics of ioPipe[1].)  In short, stdout gets redirected
      // towards the write end of the pipe.  The read end of the pipe is read
      // by the parent (paradynd), not by us.

      dup2(ioPipe[1], 2); // redirect fd 2 (stderr) to the pipe, like above.

      // We're not using ioPipe[1] anymore; close it.
      if (ioPipe[1] > 2) close (ioPipe[1]);
      */

      //setup output redirection to termWin
      dup2(stdout_fd,1);
      dup2(stdout_fd,2);

      // Now that stdout is going to a pipe, it'll (unfortunately) be block
      // buffered instead of the usual line buffered (when it goes to a tty).
      // In effect the stdio library is being a little too clever for our
      // purposes.  We don't want the "bufferedness" to change.  So we set it
      // back to line-buffered.  The command to do this is setlinebuf(stdout)
      // [stdio.h call] But we don't do it here, since the upcoming execve()
      // would undo our work [execve keeps fd's but resets higher-level stdio
      // information, which is recreated before execution of main()] So when
      // do we do it?  In rtinst's DYNINSTinit (RTposix.c et al.)

      // setup stderr for rest of exec try.
      FILE *childError = P_fdopen(2, "w");

      P_close(tracePipe[0]);

      if (P_dup2(tracePipe[1], 3) != 3) {
         fprintf(childError, "dup2 failed\n");
         fflush(childError);
         P__exit(-1);
      }

      /* close if higher */
      if (tracePipe[1] > 3) close(tracePipe[1]);


      if ((dir.length() > 0) && (P_chdir(dir.c_str()) < 0)) {
         bpfatal("cannot chdir to '%s': %s\n", dir.c_str(), 
                 strerror(errno));
         P__exit(-1);
      }
#endif
#if !defined(BPATCH_LIBRARY)
      /* see if I/O needs to be redirected */
      if (inputFile.length()) {
         int fd = P_open(inputFile.c_str(), O_RDONLY, 0);
         if (fd < 0) {
            fprintf(childError, "stdin open of %s failed\n", inputFile.c_str());
            fflush(childError);
            P__exit(-1);
         } else {
            dup2(fd, 0);
            P_close(fd);
         }
      }

      if (outputFile.length()) {
         int fd = P_open(outputFile.c_str(), O_WRONLY|O_CREAT, 0444);
         if (fd < 0) {
            fprintf(childError, "stdout open of %s failed\n", outputFile.c_str());
            fflush(childError);
            P__exit(-1);
         } else {
            dup2(fd, 1); // redirect fd 1 (stdout) to a copy of descriptor "fd"
            P_close(fd); // not using descriptor fd any more; close it.
         }
      }
#endif

#if defined (BPATCH_LIBRARY)
      // Should unify with (fancier) Paradyn handling
      /* see if we should use alternate file decriptors */
      if (stdin_fd != 0) dup2(stdin_fd, 0);
      if (stdout_fd != 1) dup2(stdout_fd, 1);
      if (stderr_fd != 2) dup2(stderr_fd, 2);
#endif

#ifdef BPATCH_LIBRARY
      // define our own session id so we don't get the mutators signals

#ifndef rs6000_ibm_aix4_1
      setsid();
#endif
#endif

      /* indicate our desire to be traced */
      errno = 0;
      OS::osTraceMe();
      if (errno != 0) {
         fprintf(stderr, 
                 "Could perform set PTRACE_TRACEME on forked process\n");
         fprintf(stderr, 
                 " Perhaps your executable doesn't have the exec bit set?\n");
         P__exit(-1);   // double underscores are correct
      }

      char **envs = NULL;
      if (envp) {
	  envs = new char*[envp->size() + 2]; // Also room for PARADYN_MASTER_INFO
	  for(unsigned ei = 0; ei < envp->size(); ++ei)
	      envs[ei] = P_strdup((*envp)[ei].c_str());
	  envs[envp->size()] = NULL;
      }
      
#ifndef BPATCH_LIBRARY
      // hand off info about how to start a paradynd to the application.
      //   used to catch rexec calls, and poe events.
      //
      char* paradynInfo = new char[1024];
      sprintf(paradynInfo, "PARADYN_MASTER_INFO= ");
      for (unsigned i=0; i < pd_process::arg_list.size(); i++) {
         const char *str;

         str = P_strdup(pd_process::arg_list[i].c_str());
         if (!strcmp(str, "-l1")) {
            strcat(paradynInfo, "-l0");
         } else {
            strcat(paradynInfo, str);
         }
         strcat(paradynInfo, " ");
      }

      if (envp) {
	  envs[envp->size()] = P_strdup(paradynInfo);
	  envs[envp->size() + 1] = NULL;
      } else {
	  P_putenv(paradynInfo);
      }
#endif

      char **args;
      args = new char*[argv->size()+1];
      for (unsigned ai=0; ai<argv->size(); ai++)
         args[ai] = P_strdup((*argv)[ai].c_str());
      args[argv->size()] = NULL;

      if (envp)
	  P_execve(file.c_str(), args, envs);
      else
	  P_execvp(file.c_str(), args);

      sprintf(errorLine, "paradynd: execv failed, errno=%d\n", errno);
      logLine(errorLine);
    
      logLine(strerror(errno));
      {
         int i=0;
         while (args[i]) {
            sprintf(errorLine, "argv %d = %s\n", i, args[i]);
            logLine(errorLine);
            i++;
         }
      }
      {
	  for(unsigned i = 0; envs[i] != NULL; ++i) {
	      sprintf(errorLine, "envp %d = %s\n", i, envs[i]);
	      logLine(errorLine);
	  }
      }	      
      P__exit(-1);
      // not reached
    
      return false;
   } else { // pid == 0 --- error
      sprintf(errorLine, "vfork failed, errno=%d\n", errno);
      logLine(errorLine);
      showErrorCallback(71, (const char *) errorLine);
      return false;
   }

}


/////////////////////////////////////////////////////////////////////////////
/// Massive amounts of signal handling code
/////////////////////////////////////////////////////////////////////////////


// Turn the return result of waitpid into something we can use
int decodeWaitPidStatus(process * /*p*/,
                        procWaitpidStatus_t status,
                        procSignalWhy_t *why,
                        procSignalWhat_t *what) {
    // Big if-then-else tree
    if (WIFEXITED(status)) {
        (*why) = procExitedNormally;
        (*what) = WEXITSTATUS(status);
        return 1;
    } 
    else if (WIFSIGNALED(status)) {
        (*why) = procExitedViaSignal;
        (*what) = WTERMSIG(status);
        return 1;
    }
    else if (WIFSTOPPED(status)) {
        (*why) = procSignalled;
        (*what) = WSTOPSIG(status);
        return 1;
    }
    else {
        (*why) = procUndefined;
        (*what) = 0;
        return 0;
    }
    return 0;
}

int forwardSigToProcess(const procevent &event) {
    process *proc = event.proc;

    // Pass the signal along to the child
    bool res;
    if(process::IndependentLwpControl()) {
       res = event.lwp->continueLWP(event.what);
    } else {
       res = proc->continueProc(event.what);
    } 
    if (res == false) {
        cerr << "Couldn't forward signal " << event.what << endl;
        logLine("error  in forwarding  signal\n");
        showErrorCallback(38, "Error  in forwarding  signal");
        return 0;
    } 

    return 1;
}

int handleSigTrap(const procevent &event) {
    process *proc = event.proc;
    
    // SIGTRAP is our workhorse. It's used to stop the process at a specific
    // address, notify the mutator/daemon of an event, and a few other things
    // as well.
    signal_cerr << "welcome to SIGTRAP for pid " << proc->getPid()
                << " status =" << proc->getStatusAsString() << endl;

    /////////////////////////////////////////
    // Init section
    /////////////////////////////////////////

    if (!proc->reachedBootstrapState(bootstrapped_bs)) {
        if (!proc->reachedBootstrapState(begun_bs)) {
            // We've created the process, but haven't reached main. 
            // This would be a perfect place to load the dyninst library,
            // but all the other shared libs aren't initialized yet. 
            // So we wait for main to be entered before working on the process.
            proc->setBootstrapState(begun_bs);
            if (proc->insertTrapAtEntryPointOfMain()) {
                pdstring buffer = pdstring("PID=") + pdstring(proc->getPid());
                buffer += pdstring(", attached to process, stepping to main");       
                statusLine(buffer.c_str());		
                proc->continueProc();
            } else {
                // We couldn't insert the trap... so detach from the process
                // and let it run. 
                cerr << "ERROR: couldn't insert at entry of main, "
                     << "instrumenting process" << endl;
                cerr << "is impossible" << endl;
                // We should actually delete any mention of this
                // process... including (for Paradyn) removing it from the
                // frontend.
                proc->triggerNormalExitCallback(0);
                proc->handleProcessExit();
		proc->continueProc();
            }
            // Now we wait for the entry point trap to be reached
            return 1;
        }
        else if (proc->trapAtEntryPointOfMain(event.lwp)) {
            proc->handleTrapAtEntryPointOfMain(event.lwp);
            proc->setBootstrapState(initialized_bs);
            // If we were execing, we now know we finished
            if (proc->execing()) {
                proc->finishExec();
            }
            return 1;
        }
        else if (proc->trapDueToDyninstLib(event.lwp)) {
            pdstring buffer = pdstring("PID=") + pdstring(proc->getPid());
            buffer += pdstring(", loaded dyninst library");
            statusLine(buffer.c_str());
            signal_cerr << "trapDueToDyninstLib returned true, trying to handle\n";
            proc->loadDYNINSTlibCleanup(event.lwp);
            proc->setBootstrapState(loadedRT_bs);
            return 1;
        }
    }

    
///////////////////////////////////
// Inferior RPC section
///////////////////////////////////

    // New and improved RPC handling, takes care of both
    // an RPC which has reached a breakpoint and whether
    // we're waiting for a syscall to complete
    if (proc->getRpcMgr()->handleSignalIfDueToIRPC(event.lwp)) {
        signal_cerr << "processed RPC response in SIGTRAP" << endl;
        return 1;
    }
    
/////////////////////////////////////////
// dlopen/close section
/////////////////////////////////////////
    
    // check to see if trap is due to dlopen or dlcose event
    if(proc->isDynamicallyLinked()){
        if(proc->handleIfDueToSharedObjectMapping()){
            signal_cerr << "handle TRAP due to dlopen or dlclose event\n";
            proc->continueProc();
	    return 1;
        }
    }

    // MT AIX is getting spurious trap instructions. I can't figure out where
    // they are coming from (and they're not at all deterministic) so 
    // we're ignoring them for now. 

    // On Linux we see a trap when the process execs. However,
    // there is no way to distinguish this trap from any other,
    // and so it is special-cased here.
#if defined(i386_unknown_linux2_0) \
 || defined(x86_64_unknown_linux2_4) /* Blind duplication - Ray */ \
 || defined(ia64_unknown_linux2_4)
    if (proc->nextTrapIsExec) {
        return handleExecExit(event);
    }
#endif

    // Check to see if this is a syscall exit
    if (proc->handleSyscallExit(0, event.lwp)) {
      proc->continueProc();
      return 1;
    }

    return 0;
}

// Needs to be fleshed out
int handleSigStopNInt(const procevent &event) {
   process *proc = event.proc;
   int retval = 0;
   signal_cerr << "welcome to SIGSTOP/SIGINT for proc pid " << proc->getPid() 
               << endl;

   if (proc->getRpcMgr()->handleSignalIfDueToIRPC(event.lwp)) {
       inferiorrpc_cerr << "processed RPC response in SIGSTOP\n";
       // don't want to execute ->Stopped() which changes status line
       retval = 1;
   }
   else {
#if defined(os_linux)
      // Linux uses SIGSTOPs for process control.  If the SIGSTOP
      // came during a process::pause (which we would know because
      // suppressEventConts() is set) then we'll handle the signal.
      // If it comes at another time we'll assume it came from something
      // like a Dyninst Breakpoint and not handle it.      
      proc->set_lwp_status(event.lwp, stopped);
      retval = proc->suppressEventConts();
#else
      signal_cerr << "unhandled SIGSTOP for pid " << proc->getPid() 
		  << " so just leaving process in paused state.\n" 
		  << std::flush;
#endif
   }

   // Unlike other signals, don't forward this to the process. It's stopped
   // already, and forwarding a "stop" does odd things on platforms
   // which use ptrace. PT_CONTINUE and SIGSTOP don't mix
   return retval;
}

int handleSigCritical(const procevent &event) {
   process *proc = event.proc;
   signal_cerr << "caught signal, dying...  (sig="
               << (int) event.what << ")" << endl << std::flush;

   signal_printf("Process %d dying on signal %d\n", proc->getPid(), event.what);
   
   for (unsigned thr_iter = 0; thr_iter <  proc->threads.size(); thr_iter++) {
       dyn_lwp *lwp = proc->threads[thr_iter]->get_lwp();
       if (lwp) {
#if defined(arch_alpha)
	 dyn_saved_regs regs;
	 lwp->getRegisters(&regs);
	 
	 fprintf(stderr, "GP: %lx\n", regs.theIntRegs.regs[REG_GP]);
	 fprintf(stderr, "SP: %lx\n", regs.theIntRegs.regs[REG_SP]);
	 fprintf(stderr, "FP: %lx\n", regs.theIntRegs.regs[15]);
	 fprintf(stderr, "RA: %lx\n", regs.theIntRegs.regs[REG_RA]);
#endif

           pdvector<Frame> stackWalk;
           lwp->walkStack(stackWalk);
           
           signal_printf( "TID: %d, LWP: %d\n",
                   proc->threads[thr_iter]->get_tid(),
                   lwp->get_lwp_id());
	   for (unsigned foo = 0; foo < stackWalk.size(); foo++)
	     signal_cerr << "   " << foo << ": " << stackWalk[foo] << endl;


       }
   }

   proc->dumpImage("imagefile");
   forwardSigToProcess(event);
   return 1;
}

int handleSignal(const procevent &event) {
    process *proc = event.proc;
    int ret = 0;

    switch(event.what) {
      case SIGTRAP:
         // Big one's up top. We use traps for most of our process control
         ret = handleSigTrap(event);

/////////////////////////////////////////////
// Trap based instrumentation
/////////////////////////////////////////////
    // We may use traps to instrument points
    // and so we forward the signal to the process
#if defined(os_linux)
         if (!ret)
             return forwardSigToProcess(event);
#endif
         break;
#if defined(bug_irix_broken_sigstop)
      case SIGEMT:
#endif
      case SIGSTOP:
      case SIGINT:
          ret = handleSigStopNInt(event);
          break;
      case SIGILL: 
          // x86 uses SIGILL for various purposes
          if (proc->getRpcMgr()->handleSignalIfDueToIRPC(event.lwp)) {
              ret = 1;
          }
          if (ret == 0)
              ret = handleSigCritical(event);
          break;
      case SIGCHLD:
         // Ignore
         ret = 1;
         proc->continueProc();
         break;
      case SIGIOT:
      case SIGBUS:
      case SIGSEGV:
	ret = handleSigCritical(event);
         break;
      case SIGCONT:
#if defined(os_linux)
         ret = 1;
         break;
#endif
      case SIGALRM:
      case SIGUSR1:
      case SIGUSR2:
      case SIGVTALRM:
         proc->set_lwp_status(event.lwp, stopped);
      default:
         ret = 0;
         break;
    }
    return ret;
 }

 //////////////////////////////////////////////////////////////////
 // Syscall handling
 //////////////////////////////////////////////////////////////////

 // Most of our syscall handling code is shared on all platforms.
 // Unfortunately, there's that 5% difference...

 int handleForkEntry(const procevent &event) {
     signal_printf("Welcome to FORK ENTRY for process %d\n",
                   event.proc->getPid());
     event.proc->handleForkEntry();
     return 1;
 }

 // On AIX I've seen a long string of calls to exec, basically
 // doing a (for i in $path; do exec $i/<progname>
 // This means that the entry will be called multiple times
 // until the exec call gets the path right.
 int handleExecEntry(const procevent &event) {
     event.proc->handleExecEntry((char *)event.info);
     return 1;
 }

 int handleSyscallEntry(const procevent &event) {
    process *proc = event.proc;
    procSyscall_t syscall = decodeSyscall(proc, event.what);
    int ret = 0;
    switch (syscall) {
      case procSysFork:
          ret = handleForkEntry(event);
          break;
      case procSysExec:
         ret = handleExecEntry(event);
         break;
      case procSysExit:
          proc->triggerNormalExitCallback(event.info);
          ret = 1;
          break;
      default:
      // Check process for any other syscall
      // we may have trapped on entry to?
      ret = 0;
      break;
    }
    // Continue the process post-handling
    proc->continueProc();
    return ret;
 }

 /* Only dyninst for now... paradyn should use this soon */
 int handleForkExit(const procevent &event) {
     signal_printf("Welcome to FORK EXIT for process %d\n",
                   event.proc->getPid());

     process *proc = event.proc;
     // Fork handler time
     extern pdvector<process*> processVec;
     int childPid = event.info;
     
     if (childPid == getpid()) {
         // this is a special case where the normal createProcess code
         // has created this process, but the attach routine runs soon
         // enough that the child (of the mutator) gets a fork exit
         // event.  We don't care about this event, so we just continue
         // the process - jkh 1/31/00
         return 1;
     } else if (childPid > 0) {

         unsigned int i;
         for (i=0; i < processVec.size(); i++) {
             if (processVec[i] && 
                 (processVec[i]->getPid() == childPid)) break;
         }
         if (i== processVec.size()) {
             // this is a new child, register it with dyninst
             // Lose a race condition between us and the OS
             sleep(1);

             // We leave the parent paused until the child is finished,
             // so that we can be sure to copy everything correctly.

             process *theChild = new process(proc, (int)childPid, -1);

             if (theChild->setupFork()) {
                 proc->handleForkExit(theChild);

                 // Okay, let 'er rip
		 proc->continueProc();
		 theChild->continueProc();
             }
             else {
                 // Can happen if we're forking something we can't trace
                 delete theChild;
                 proc->continueProc();
             }
        }
    }
    return 1;
}

// the alwaysdosomething argument is to maintain some strange old code
int handleExecExit(const procevent &event) {
    process *proc = event.proc;
    proc->nextTrapIsExec = false;
    if((int)event.info == -1) {
        // Failed exec, do nothing
        return 1;
    }

    proc->execFilePath = proc->tryToFindExecutable(proc->execPathArg, proc->getPid());
    // As of Solaris 2.8, we get multiple exec signals per exec.
    // My best guess is that the daemon reads the trap into the
    // kernel as an exec call, since the process is paused
    // and PR_SYSEXIT is set. We want to ignore all traps 
    // but the last one.
    
    // We also see an exec as the first signal in a process we create. 
    // That's because it... wait for it... execed!
    if (!proc->reachedBootstrapState(begun_bs)) {
        return handleSigTrap(event);
    }
    
    // Unlike fork, handleExecExit doesn't do all processing required.
    // We finish up when the trap at main() is reached.
    proc->handleExecExit();

    return 1;
}

int handleLoadExit(const procevent &event) {
    // AIX: for 4.3.2 and later, load no longer causes the 
    // reinitialization of the process text space, and as
    // such we don't need to fix base tramps.
    event.proc->handleIfDueToSharedObjectMapping();
    return 1;
}


int handleSyscallExit(const procevent &event) {
    process *proc = event.proc;
    procSyscall_t syscall = decodeSyscall(proc, event.what);
    int ret = 0;
    
    // Check to see if a thread we were waiting for exited a
    // syscall
    int wasHandled = proc->handleSyscallExit(event.what, event.lwp);

    // Fall through no matter what since some syscalls have their
    // own handlers.
    switch(syscall) {
      case procSysFork:
         ret = handleForkExit(event);
         break;
      case procSysExec:
         ret = handleExecExit(event);
         break;
      case procSysLoad:
         ret = handleLoadExit(event);
         break;
      default:
         break;
    }

#if defined(rs6000_ibm_aix4_1)
    // When we handle a fork exit on AIX, we need to keep both parent and
    // child stopped until we've seen the fork exit on both.  This is so
    // we can copy the instrumentation from the parent to the child (if we
    // don't keep the parent stopped, it may, for instance, exit before we
    // can do this).  So, don't continue the process here - it will be
    // continued at the appropriate time by handleForkExit.
    if (syscall != procSysFork)
#endif
      proc->continueProc();
    
    if (ret || wasHandled)
        return 1;
    else
        return 0;
}

int signalHandler::handleProcessEvent(const procevent &event) {
   process *proc = event.proc;
   assert(proc);
   signal_cerr << "handleProcessEvent, pid: " << proc->getPid() << ", why: "
	       << event.why << ", what: " << event.what << ", lwps: "
	       << event.lwp->get_lwp_id() << endl;
#if 0
   if (dyn_debug_signal) { 
       Frame activeFrame = event.lwp->getActiveFrame();
       signal_cerr << "Event active frame: " << activeFrame << endl;
   }
#endif

   int ret = 0;
   if(proc->hasExited()) {
       // Yeah, this was handled... ;)
       return 1;
   }

   // One big switch statement
   switch(event.why) {
      // First the platform-independent stuff
      // (/proc and waitpid)
     case procExitedNormally:
        sprintf(errorLine, "Process %d has terminated with code 0x%x\n", 
                proc->getPid(), event.what);
        statusLine(errorLine);
        //logLine(errorLine);
        // triggerNormalExitCallback function gets called from event
        // triggered by exit() entry
        proc->handleProcessExit();
        ret = 1;
        break;
     case procExitedViaSignal:
        sprintf(errorLine, "process %d has terminated on signal %d\n", 
                proc->getPid(), event.what);
        logLine(errorLine);
        statusLine(errorLine);
        printDyninstStats();
        proc->triggerSignalExitCallback(event.what);  
        proc->handleProcessExit();
        ret = 1;
        break;
     case procSignalled:
     case procForkSigChild:
        ret = handleSignal(event);
        break;
        // Now the /proc only
        // AIX clones some of these (because of fork/exec/load notification)
     case procSyscallEntry:
        ret = handleSyscallEntry(event);
        if (!ret)
            cerr << "handleSyscallEntry failed!" << endl;
        break;
     case procSyscallExit:
        ret = handleSyscallExit(event);
        if (!ret)
            cerr << "handlesyscallExit failed! " << event.what <<  endl;
        break;
     case procSuspended:
       proc->continueProc();   // ignoring this signal
       ret = 1;
       break;
     case procInstPointTrap:
         // Linux inst via traps
         event.lwp->changePC(event.info, NULL);
	 proc->continueProc();
         ret = 1;
         break;
     case procUndefined:
        // Do nothing
         cerr << "Undefined event!" << endl;
        break;
     default:
        assert(0 && "Undefined");
   }

   return ret;
}





bool  OS::osKill(int pid) {
  return (P_kill(pid,9)==0);
}
