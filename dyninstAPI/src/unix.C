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

// $Id: unix.C,v 1.92 2003/05/08 23:48:42 bernat Exp $

#include "common/h/headers.h"
#include "common/h/String.h"
#include "common/h/Vector.h"
#include "dyninstAPI/src/showerror.h"
#include "dyninstAPI/src/os.h"
#include "dyninstAPI/src/util.h"

#ifndef BPATCH_LIBRARY
#include "paradynd/src/perfStream.h"
#include "paradynd/src/main.h"
#include "paradynd/src/pd_process.h"
#endif

// the following are needed for handleSigChild
#include "dyninstAPI/src/signalhandler.h"
#include "dyninstAPI/src/process.h"
#include "dyninstAPI/src/dyn_lwp.h"
#include "dyninstAPI/src/dyn_thread.h"
#include "dyninstAPI/src/instP.h"
#include "dyninstAPI/src/stats.h"

// The following were all defined in process.C (for no particular reason)

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

extern unsigned enable_pd_metric_debug;

#if ENABLE_DEBUG_CERR == 1
#define metric_cerr if (enable_pd_metric_debug) cerr
#else
#define metric_cerr if (0) cerr
#endif /* ENABLE_DEBUG_CERR == 1 */

extern unsigned enable_pd_signal_debug;

#if ENABLE_DEBUG_CERR == 1
#define signal_cerr if (enable_pd_signal_debug) cerr
#else
#define signal_cerr if (0) cerr
#endif /* ENABLE_DEBUG_CERR == 1 */

extern unsigned enable_pd_sharedobj_debug;

#if ENABLE_DEBUG_CERR == 1
#define sharedobj_cerr if (enable_pd_sharedobj_debug) cerr
#else
#define sharedobj_cerr if (0) cerr
#endif /* ENABLE_DEBUG_CERR == 1 */

extern "C" {
#ifdef PARADYND_PVM
int pvmputenv (const char *);
int pvmendtask();
#endif
}

/*****************************************************************************
 * forkNewProcess: starts a new process, setting up trace and io links between
 *                the new process and the daemon
 * Returns true if succesfull.
 * 
 * Arguments:
 *   file: file to execute
 *   dir: working directory for the new process
 *   argv: arguments to new process
 *   envp: environment **** not in use
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
#ifdef BPATCH_LIBRARY
bool forkNewProcess(string &file, string dir, pdvector<string> argv, 
                    pdvector<string> /*envp*/, 
                    string /*inputFile*/, string /*outputFile*/,
                    int & /*traceLink*/, 
                    int &pid, int & /*tid*/, 
                    int & /*procHandle*/, int & /*thrHandle*/, 
		    int stdin_fd, int , int )
#else
bool forkNewProcess(string &file, string dir, pdvector<string> argv, 
		    pdvector<string>envp, string inputFile, string outputFile,
		    int &traceLink,
		    int &pid, int & /*tid*/, 
		    int & /*procHandle*/, int & /*thrHandle*/,
		    int stdin_fd, int stdout_fd, int)

#endif
{
#ifndef BPATCH_LIBRARY
    // Strange, but using socketpair here doesn't seem to work OK on SunOS.
    // Pipe works fine.
    // r = P_socketpair(AF_UNIX, SOCK_STREAM, (int) NULL, tracePipe);
    int tracePipe[2];
    int r = P_pipe(tracePipe);
    if (r) {
	// P_perror("socketpair");
        string msg = string("Unable to create trace pipe for program '") + file +
	               string("': ") + string(sys_errlist[errno]);
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
        string msg = string("Unable to create IO pipe for program '") + file +
	               string("': ") + string(sys_errlist[errno]);
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
#if defined(PARADYND_PVM) || (defined(BPATCH_LIBRARY) && !defined(alpha_dec_osf4_0))
// must use fork, since pvmendtask will do some writing in the address space
    pid = fork();
    // fprintf(stderr, "FORK: pid=%d\n", pid);
#else
    pid = vfork();
#endif

    if (pid != 0) {

        // *** parent

#if defined(PARADYND_PVM) || (defined(BPATCH_LIBRARY) && !defined(alpha_dec_osf4_0))
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
		    sys_errlist[errno]);
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

#ifdef PARADYND_PVM
	if (pvm_running)
	  pvmendtask(); 
#endif   

#ifndef BPATCH_LIBRARY
	// handle stdio.

	/* removed for output redirection
        // We only write to ioPipe.  Hence we close ioPipe[0], the read end.  Then we
        // call dup2() twice to assign our stdout and stderr to the write end of the
	// pipe.
	close(ioPipe[0]);

	//dup2(ioPipe[1], 1);

	
           // assigns fd 1 (stdout) to be a copy of ioPipe[1].  (Since stdout is already
           // in use, dup2 will first close it then reopen it with the characteristics
	   // of ioPipe[1].)
           // In short, stdout gets redirected towards the write end of the pipe.
           // The read end of the pipe is read by the parent (paradynd), not by us.

	dup2(ioPipe[1], 2); // redirect fd 2 (stderr) to the pipe, like above.

        // We're not using ioPipe[1] anymore; close it.
	if (ioPipe[1] > 2) close (ioPipe[1]);
	*/

	//setup output redirection to termWin
	dup2(stdout_fd,1);
	dup2(stdout_fd,2);

	// Now that stdout is going to a pipe, it'll (unfortunately) be block buffered
        // instead of the usual line buffered (when it goes to a tty).  In effect the
        // stdio library is being a little too clever for our purposes.  We don't want
        // the "bufferedness" to change.  So we set it back to line-buffered.
        // The command to do this is setlinebuf(stdout) [stdio.h call]  But we don't
        // do it here, since the upcoming execve() would undo our work [execve keeps
        // fd's but resets higher-level stdio information, which is recreated before
        // execution of main()]  So when do we do it?  In rtinst's DYNINSTinit
        // (RTposix.c et al.)

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
	  sprintf(errorLine, "cannot chdir to '%s': %s\n", dir.c_str(), 
		  sys_errlist[errno]);
	  logLine(errorLine);
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

        /* see if we should use alternate file decriptors */
	if (stdin_fd != 0) dup2(stdin_fd, 0);
	//removed for output redirection
	//if (stdout_fd != 1) dup2(stdout_fd, 1);
	//if (stderr_fd != 2) dup2(stderr_fd, 2);

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
	  sprintf(errorLine, "ptrace error, exiting, errno=%d\n", errno);
	  logLine(errorLine);
	  logLine(sys_errlist[errno]);
	  showErrorCallback(69, string("Internal error: ") + 
	                        string((const char *) errorLine)); 
	  P__exit(-1);   // double underscores are correct
	}
#ifdef PARADYND_PVM
	if (pvm_running && envp.size())
	  for (int ep=envp.size()-1; ep>=0; ep--) {
	    pvmputenv(envp[ep].c_str());
	  }
#endif
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
	P_putenv(paradynInfo);
#endif

	char **args;
	args = new char*[argv.size()+1];
	for (unsigned ai=0; ai<argv.size(); ai++)
	  args[ai] = P_strdup(argv[ai].c_str());
	args[argv.size()] = NULL;
	P_execvp(file.c_str(), args);
    fprintf(stderr, "Exec failed\n");
    
	sprintf(errorLine, "paradynd: execv failed, errno=%d\n", errno);
	logLine(errorLine);
    
	logLine(sys_errlist[errno]);
    {
        int i=0;
        while (args[i]) {
            sprintf(errorLine, "argv %d = %s\n", i, args[i]);
            logLine(errorLine);
            i++;
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
                        procSignalWhy_t &why,
                        procSignalWhat_t &what) {
    // Big if-then-else tree
    if (WIFEXITED(status)) {
        why = procExitedNormally;
        what = WEXITSTATUS(status);
        return 1;
    } 
    else if (WIFSIGNALED(status)) {
        why = procExitedViaSignal;
        what = WTERMSIG(status);
        return 1;
    }
    else if (WIFSTOPPED(status)) {
        why = procSignalled;
        what = WSTOPSIG(status);
        return 1;
    }
    else {
        why = procUndefined;
        what = 0;
        return 0;
    }
    return 0;
}

int forwardSigToProcess(process *proc, 
                        procSignalWhat_t what,
                        procSignalInfo_t info) {
#if (defined(POWER_DEBUG) || defined(HP_DEBUG)) && \
    (defined(rs6000_ibm_aix4_1) || defined(hppa1_1_hp_hpux))
    // In this way, we will detach from the application and we
    // will be able to attach again using gdb - naim
    if (what==SIGSEGV) {
#if defined(rs6000_ibm_aix4_1)
        if (ptrace(PT_DETACH,proc->getPid(),(int *) 1, SIGSTOP, NULL) == -1)
#else
        if (ptrace(PT_DETACH,proc->getPid(), 1, SIGSTOP, NULL) == -1)
#endif
        {
            
            logLine("ptrace error\n");
            return 0;
        }
    }
#endif
    // Pass the signal along to the child
    if (!proc->continueWithForwardSignal(what)) {
        cerr << "Couldn't forward signal " << what << endl;
        logLine("error  in forwarding  signal\n");
        showErrorCallback(38, "Error  in forwarding  signal");
        return 0;
        //P_abort();
    } 
    if (what != SIGSTOP)
	    proc->status_ = running;
    return 1;
}

int handleSigTrap(process *proc, procSignalInfo_t info) {
    // SIGTRAP is our workhorse. It's used to stop the process at a specific
    // address, notify the mutator/daemon of an event, and a few other things
    // as well.
    signal_cerr << "welcome to SIGTRAP for pid " << proc->getPid()
                << " status =" << proc->getStatusAsString() << endl;
/////////////////////////////////////////
// Init section
/////////////////////////////////////////
    
    if (!proc->reachedBootstrapState(bootstrapped)) {
        
        if (!proc->reachedBootstrapState(begun)) {
            // We've created the process, but haven't reached main. 
            // This would be a perfect place to load the dyninst library,
            // but all the other shared libs aren't initialized yet. 
            // So we wait for main to be entered before working on the process.
            proc->setBootstrapState(begun);
            if (proc->insertTrapAtEntryPointOfMain()) {
                string buffer = string("PID=") + string(proc->getPid());
                buffer += string(", attached to process, stepping to main");       
                statusLine(buffer.c_str());
                if (!proc->continueProc()) {
                    assert(0);
                }
            } else {
                // We couldn't insert the trap... so detach from the process
                // and let it run. 
                cerr << "ERROR: couldn't insert at entry of main, instrumenting process" << endl;
                cerr << "is impossible" << endl;
                // We should actually delete any mention of this process... including
                // (for Paradyn) removing it from the frontend.
                handleProcessExit(proc, 0);
                proc->continueProc();
            }
            // Now we wait for the entry point trap to be reached
            
            return 1;
        }
        else if (proc->trapAtEntryPointOfMain()) {
            proc->handleTrapAtEntryPointOfMain();
            proc->setBootstrapState(initialized);
            // If we're in an exec, this is when we know it is actually finished
            if (proc->wasExeced()) {
                proc->loadDyninstLib();
            }
            
            return 1;
        }
        else if (proc->trapDueToDyninstLib()) {
            string buffer = string("PID=") + string(proc->getPid());
            buffer += string(", loaded dyninst library");
            statusLine(buffer.c_str());
            //signal_cerr << "trapDueToDyninstLib returned true, trying to handle\n";
            proc->loadDYNINSTlibCleanup();
            proc->setBootstrapState(loadedRT);
            return 1;
        }
    }
    
///////////////////////////////////
// Inferior RPC section
///////////////////////////////////
    
    // New and improved RPC handling, takes care of both
    // an RPC which has reached a breakpoint and whether
    // we're waiting for a syscall to complete
    if (proc->getRpcMgr()->handleSignalIfDueToIRPC()) {
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
            if (!proc->continueProc()) {
                assert(0);
            }
            return 1;
        }
    }

/////////////////////////////////////////////
// Trap based instrumentation
/////////////////////////////////////////////
    // We may use traps to instrument points
    // and so we forward the signal to the process
    // MT AIX is getting spurious trap instructions. I can't figure out where
    // they are coming from (and they're not at all deterministic) so 
    // we're ignoring them for now. 
#if defined(rs6000_ibm_aix4_1) && defined(MT_THREAD)
    // Check to see if this is a syscall exit
    proc->handleSyscallExit(0);
    
    proc->continueProc();
    return 1;
#else

    // On Linux we see a trap when the process execs. However,
    // there is no way to distinguish this trap from any other,
    // and so it is special-cased here.
#if defined(i386_unknown_linux2_0)
    if (proc->nextTrapIsExec) {
        return handleExecExit(proc, 0);
    }
#endif
    // Forward the trap to the process

    return 0;
#endif
}

// Needs to be fleshed out
int handleSigStopNInt(process *proc, procSignalInfo_t info) {
   signal_cerr << "welcome to SIGSTOP/SIGINT for proc pid " << proc->getPid() 
               << endl;

   if (proc->getRpcMgr()->handleSignalIfDueToIRPC()) {
       inferiorrpc_cerr << "processed RPC response in SIGSTOP\n";
       // don't want to execute ->Stopped() which changes status line
       return 1;
   }
   else {
       signal_cerr << "unhandled SIGSTOP for pid " << proc->getPid() 
                   << " so just leaving process in paused state.\n" 
                   << flush;
   }
   // Unlike other signals, don't forward this to the process. It's stopped
   // already, and forwarding a "stop" does odd things on platforms
   // which use ptrace. PT_CONTINUE and SIGSTOP don't mix
   return 1;
}

int handleSigCritical(process *proc, procSignalWhat_t what, procSignalInfo_t info) {
#if (defined(POWER_DEBUG) || defined(HP_DEBUG)) && (defined(rs6000_ibm_aix4_1) || defined(hppa1_1_hp_hpux))
    // In this way, we will detach from the application and we
    // will be able to attach again using gdb. We need to send
    // a kill -ILL pid signal to the application in order to
    // get here - naim
#if defined(rs6000_ibm_aix4_1)
   {
      sprintf(errorLine, "***** Detaching from process %d "
	      ", ready for debugging...\n",
	      pid);
      logLine(errorLine);
   }
   if (ptrace(PT_DETACH, proc->getPid(),(int *) 1, SIGSTOP, NULL) == -1)
#else
   if (ptrace(PT_DETACH, proc->getPid(), 1, SIGSTOP, NULL) == -1)
#endif
   {
       logLine("ptrace error\n");
       return 0;
   }
#endif

#ifdef DEBUG
   signal_cerr << "caught signal, dying...  (sig="
               << (int) what << ")" << endl << flush;

   pdvector<pdvector<Frame> > stackwalks;
   proc->walkStacks(stackwalks);
   
   for (unsigned i = 0; i < stackwalks.size(); i++) {
       pdvector<Frame> stackWalk = stackwalks[i];
       fprintf(stderr, "LWP: %d, TID: %d\n",
               stackWalk[0].getLWP()->get_lwp_id(),
               stackWalk[0].getThread()->get_tid());
#if defined(sparc_sun_solaris2_4)
       lwpstatus status;
       stackWalk[0].getLWP()->get_status(&status);
       fprintf(stderr, "why: %d, what: %d\n",
               status.pr_why, 
               status.pr_what);
#endif
       
       for (unsigned j = 0; j < stackWalk.size(); j++) {
           fprintf(stderr, "PC: 0x%x   ", stackWalk[j].getPC());
           pd_Function *func = proc->findFuncByAddr(stackWalk[j].getPC());
           if (func)
               fprintf(stderr, "%s", func->prettyName().c_str());
           fprintf(stderr, "\n");
       }
       fprintf(stderr, "\n\n\n");
   }

#endif
   proc->dumpImage("imagefile");
   
   forwardSigToProcess(proc, what, info);   
   return 1;
}


int handleSignal(process *proc, procSignalWhat_t what, 
                 procSignalInfo_t info) {
   int ret = 0;
    
   switch(what) {
 case SIGTRAP:
     // Big one's up top. We use traps for most of our process control
     ret = handleSigTrap(proc, info); 
     break;
#if defined(USE_IRIX_FIXES)
 case SIGEMT:
#endif
 case SIGSTOP:
 case SIGINT:
     ret = handleSigStopNInt(proc, info);
     break;
 case SIGILL: 
     if (proc->getRpcMgr()->handleSignalIfDueToIRPC())
         ret = 1;
     break;
 case SIGCHLD:
     // Ignore
     ret = 1;
     proc->continueProc();
     break;
     // Else fall through
 case SIGIOT:
 case SIGBUS:
 case SIGSEGV:
     ret = handleSigCritical(proc, what, info);
     break;
 case SIGCONT:
     // Should inform the mutator/daemon that the process is running
 case SIGALRM:
 case SIGUSR1:
 case SIGUSR2:
 case SIGVTALRM:
 default:
     ret = 0;
     break;
   }
   if (!ret) {
       // Signal was not handled
       ret = forwardSigToProcess(proc, what, info);
   }
   return ret;
}

//////////////////////////////////////////////////////////////////
// Syscall handling
//////////////////////////////////////////////////////////////////

// Most of our syscall handling code is shared on all platforms.
// Unfortunately, there's that 5% difference...

int handleForkEntry(process *proc, procSignalInfo_t info) {
    proc->handleForkEntry();
    return 1;
}

// On AIX I've seen a long string of calls to exec, basically
// doing a (for i in $path; do exec $i/<progname>
// This means that the entry will be called multiple times
// until the exec call gets the path right.
int handleExecEntry(process *proc, procSignalInfo_t info) {
    proc->handleExecEntry((char *)info);
    return 1;
}

int handleExitEntry(process *proc, procSignalInfo_t info) {
    // Should probably call handlProcessExit here,
    // but it gets called later as well.
    proc->handleExitEntry(info);
    return 1;
}


int handleSyscallEntry(process *proc, procSignalWhat_t what, 
                       procSignalInfo_t info) {
   procSyscall_t syscall = decodeSyscall(proc, what);
   int ret = 0;

   switch (syscall) {
     case procSysFork:
        ret = handleForkEntry(proc, info);
        break;
     case procSysExec:
        ret = handleExecEntry(proc, info);
        break;
     case procSysExit:
        ret = handleExitEntry(proc, info);
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
int handleForkExit(process *proc, procSignalInfo_t info) {
    proc->nextTrapIsFork = false;
    // Fork handler time
    extern pdvector<process*> processVec;
    int childPid = info;

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
            if (processVec[i]->getPid() == childPid) break;
        }
        if (i== processVec.size()) {
            // this is a new child, register it with dyninst
            int parentPid = proc->getPid();

            process *theChild = new process(*proc, (int)childPid, -1);
            if (theChild) {
                processVec.push_back(theChild);
                activeProcesses++;
                
                theChild->status_ = stopped;
                
#if defined(rs6000_ibm_aix4_1)
                // AIX has interesting fork behavior: the program image is
                // loaded from disk instead of copied over. This means we 
                // need to reinsert all instrumentation.
                extern bool copyInstrumentationToChild(process *p, process *c);
                copyInstrumentationToChild(proc, theChild);
#endif
                proc->handleForkExit(theChild);
                
            }
            else {
                // Can happen if we're forking something we can't trace
                cerr << "Process forked, but unable to initialize child" << endl;
            }
        }
#if defined(rs6000_ibm_aix4_1)
	// On AIX, we don't continue the process elsewhere because we want to
	// wait until we've seen the fork exit in both the parent and the
	// child.  This is so that the copyInstrumentationToChild above will
	// work - without leaving the parent paused, it may, for instance,
	// exit before we copy the instrumentation.  Now that we've handled
	// that, continue both the parent and the child.
	proc->continueProc();
	processVec[i]->continueProc();
#endif
    }

    return 1;
}

int handleExecExit(process *proc, procSignalInfo_t info) {
    proc->nextTrapIsExec = false;
    if (info == -1) {
        // Failed exec, do nothing
        return 1;
    }
    else {
        proc->execFilePath = proc->tryToFindExecutable(proc->execPathArg, proc->getPid());
        // As of Solaris 2.8, we get multiple exec signals per exec.
        // My best guess is that the daemon reads the trap into the
        // kernel as an exec call, since the process is paused
        // and PR_SYSEXIT is set. We want to ignore all traps 
        // but the last one.
        bool isThisAnExecInTheRunningProgram = 
        proc->reachedBootstrapState(initialized);
        bool areWeInTheProcessOfHandlingAnExec = proc->wasExeced();
        if(isThisAnExecInTheRunningProgram || 
           areWeInTheProcessOfHandlingAnExec)
        {
            // since Solaris causes multiple traps associated with trapping
            // on exit of exec syscall, we do proper exec handling
            // (eg. cause process::handleExec to be called) for each trap
            // so when "real" exec trap occurs, will handle correctly.  I'm
            // considering the "real" exec trap as the one that occurs when
            // the execed process has been created and we're paused at the
            // end of "exec".  The other execs seem to occur at some other
            // point in the exec process syscall an the "execed" process
            // hasn't been created yet.
            
            // because of these multiple exec exit notices, the sequence
            // of the process status goes something like this
            // false exec notice:  boostrapped    =>  unstarted (handleExec)
            // handleSigChild:     unstarted      =>  begun (trap at main)
            // false exec notice:  begun          =>  unstarted (handleExec)
            // handleSigChild:     unstarted      =>  begun (trap at main)
            // real exec notice:   begun          =>  unstarted (handleExec)
            // handleSigChild:     unstarted      =>  begun (trap at main)
            // trap at main:       begun          =>  initialized
            proc->inExec = true; 
            proc->status_ = stopped;
            pdvector<heapItem *> emptyHeap;
            proc->heap.bufferPool = emptyHeap;
#ifndef BPATCH_LIBRARY
            // Mimic bump-up in process constructor
            tp->resourceBatchMode(true);
#endif
            // Clean out internal data structures for the process
            // We should have an exec "constructor"

            // Unlike fork, handleExecExit doesn't do all processing required.
            // We finish up when the trap at main() is reached.
            proc->handleExecExit();
            
            // Note: on Solaris this is called multiple times before anything
            // actually happens. Therefore handleExec must handle being called
            // multiple times. We know that we're done when we hit the trap at main.
            // Oh, and install that here.
        }
        proc->setBootstrapState(begun);
        if (!proc->insertTrapAtEntryPointOfMain()) {
            cerr << "Failed to initialize exec'ed process, assuming exit..." << endl;
            proc->continueProc();
            // We should actually delete any mention of this process... including
            // (for Paradyn) removing it from the frontend.
            handleProcessExit(proc, 0);
        }
        else proc->continueProc();
    }
    return 1;
}

int handleLoadExit(process *proc, procSignalInfo_t info) {
    // AIX: for 4.3.2 and later, load no longer causes the 
    // reinitialization of the process text space, and as
    // such we don't need to fix base tramps.
    proc->handleIfDueToSharedObjectMapping();
    return 1;
}


int handleSyscallExit(process *proc,
                      procSignalWhat_t what,
                      procSignalInfo_t info) {
    procSyscall_t syscall = decodeSyscall(proc, what);
    int ret = 0;
    
    // Check to see if a thread we were waiting for exited a
    // syscall
    int wasHandled = proc->handleSyscallExit(what);

    // Fall through no matter what since some syscalls have their
    // own handlers.
    switch(syscall) {
  case procSysFork:
      ret = handleForkExit(proc, info);
      break;
  case procSysExec:
      ret = handleExecExit(proc, info);
      break;
  case procSysLoad:
      ret = handleLoadExit(proc, info);
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

int handleProcessEvent(process *proc,
                       procSignalWhy_t why,
                       procSignalWhat_t what,
                       procSignalInfo_t info) {
   int ret = 0;
   if(proc->hasExited()) {
       return 1;
   }
   
   // One big switch statement
   switch(why) {
      // First the platform-independent stuff
      // (/proc and waitpid)
     case procExitedNormally:
        sprintf(errorLine, "Process %d has terminated with code 0x%x\n", 
                proc->getPid(), what);
        statusLine(errorLine);
        logLine(errorLine);
        handleProcessExit(proc, what);
        ret = 1;
        break;
     case procExitedViaSignal:
        sprintf(errorLine, "process %d has terminated on signal %d\n", 
                proc->getPid(), what);
        logLine(errorLine);
        statusLine(errorLine);
        printDyninstStats();
        handleProcessExit(proc, what);
        ret = 1;
        break;
     case procSignalled:
        ret = handleSignal(proc, what, info);
        if (!ret)
            cerr << "handleSignal failed! " << what << endl;
        break;
        // Now the /proc only
        // AIX clones some of these (because of fork/exec/load notification)
     case procSyscallEntry:
        ret = handleSyscallEntry(proc, what, info);
        if (!ret)
            cerr << "handleSyscallEntry failed!" << endl;
        break;
     case procSyscallExit:
        ret = handleSyscallExit(proc, what, info);
        if (!ret)
            cerr << "handlesyscallExit failed! " << what <<  endl;
        break;
     case procSuspended:
        proc->continueProc();   // ignoring this signal
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

void decodeAndHandleProcessEvent (bool block) {
    procSignalWhy_t why;
    procSignalWhat_t what;
    procSignalInfo_t info;
    process *proc;
    proc = decodeProcessEvent(-1, why, what, info, block);
    if (!proc)
       return;
    
    if (!handleProcessEvent(proc, why, what, info)) 
        fprintf(stderr, "handleProcessEvent failed!\n");
}


bool  OS::osKill(int pid) {
  return (P_kill(pid,9)==0);
}
