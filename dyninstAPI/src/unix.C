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

// $Id: unix.C,v 1.63 2002/06/14 21:43:32 tlmiller Exp $

#if defined(i386_unknown_solaris2_5)
#include <sys/procfs.h>
#endif
#include "common/h/headers.h"
#include "common/h/String.h"
#include "common/h/Vector.h"
#include "dyninstAPI/src/showerror.h"
#include "dyninstAPI/src/os.h"
#include "dyninstAPI/src/util.h"

#ifndef BPATCH_LIBRARY
#include "paradynd/src/perfStream.h"
#include "paradynd/src/main.h"
#endif

// the following are needed for handleSigChild
#include "dyninstAPI/src/process.h"
#include "dyninstAPI/src/instP.h"
#include "dyninstAPI/src/stats.h"
extern process *findProcess(int);

// The following were all defined in process.C (for no particular reason)
extern debug_ostream attach_cerr;
extern debug_ostream inferiorrpc_cerr;
extern debug_ostream shmsample_cerr;
extern debug_ostream forkexec_cerr;
extern debug_ostream signal_cerr;
extern debug_ostream sharedobj_cerr;

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
bool forkNewProcess(string &file, string /*dir*/, vector<string> argv, 
                    vector<string> /*envp*/, 
                    string /*inputFile*/, string /*outputFile*/,
                    int & /*traceLink*/, 
                    int &pid, int & /*tid*/, 
                    int & /*procHandle*/, int & /*thrHandle*/, 
		    int stdin_fd, int , int )
#else
bool forkNewProcess(string &file, string dir, vector<string> argv, 
		    vector<string>envp, string inputFile, string outputFile,
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
	if (pid == -1) {
#else
	if (errno) {
#endif
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
	for (unsigned i=0; i < process::arg_list.size(); i++) {
	    const char *str;

	    str = P_strdup(process::arg_list[i].c_str());
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



/* 
   TODO: cleanup handleSigChild. This function has a lot of code that
   should be moved to a machine independent place (a lot of what is
   going on here will probably have to be moved anyway once we move to
   the dyninstAPI).

   There is a different version of this function for WindowsNT. If any changes
   are made here, they will probably also be needed in the NT version.

   --mjrg
*/

// TODO -- make this a process method
int handleSigChild(int pid, int status)
{
#ifdef rs6000_ibm_aix4_1
    // On AIX, we get sigtraps on fork and load, and must handle
    // these cases specially
    extern bool handleAIXsigTraps(int, int);
    if (handleAIXsigTraps(pid, status)) {
      return 0;
    }

    /* else check for regular traps and signals */
#endif

    // ignore signals from unknown processes
    process *curr = findProcess(pid);

    if (!curr) {
       forkexec_cerr << "handleSigChild pid " << pid << " is an unknown process." << endl;
       forkexec_cerr << "WIFSTOPPED=" << (WIFSTOPPED(status) ? "true" : "false");
       if (WIFSTOPPED(status)) {
	  forkexec_cerr << "WSTOPSIG=" << WSTOPSIG(status);
       }
       forkexec_cerr << endl << flush;
#ifdef rs6000_ibm_aix4_1
       cerr << "handleSigChild:  Detaching process " << pid 
            << " and continuing." << endl;
       return ptrace(PT_DETACH, pid, (int *)0, SIGCONT, 0);   //Set process free
#endif
       return -1;
    }
    
    // Normal case, actually.
    if (curr->status_ == exited) return 0;

    if (WIFSTOPPED(status)) {
	int sig = WSTOPSIG(status);
#if defined( i386_unknown_linux2_0 ) || defined( ia64_unknown_linux2_4 )
	int orig_sig = sig;
#endif
#ifdef DETACH_ON_THE_FLY
	/* Keep track of ILL->STOP events, in case we need to adjust
	   the PC past the illegal instruction. */
        int fix_ill = 0;
#endif

#if defined(i386_unknown_solaris2_5)
        // we put an illegal instead of a trap at the following places, but 
        // the illegal instructions are really for trapping purpose, so the
        // handling should be the same as for trap
        {
          prgregset_t regs;
          int proc_fd = curr->getProcFileDescriptor();
          if ((ioctl (proc_fd, PIOCGREG, &regs) != -1) && (sig == SIGILL)
          && (regs[R_PC]==(int)curr->rbrkAddr()
            ||regs[R_PC]==(int)curr->main_brk_addr
            ||regs[R_PC]==(int)curr->dyninstlib_brk_addr)) {
            sig = SIGTRAP;
          }
        }
#elif defined(i386_unknown_linux2_0) || defined(ia64_unknown_linux2_4)
        // we put an illegal instead of a trap at the following places, but 
        // the illegal instructions are really for trapping purpose, so the
        // handling should be the same as for trap
	{
	     Address pc = getPC( pid );
	     if (sig == SIGILL)
		  if (pc==(Address)curr->rbrkAddr()
		      || pc==(Address)curr->main_brk_addr
		      || pc==(Address)curr->dyninstlib_brk_addr
		      || curr->isRPCwaitingForSysCallToComplete()) {
		       signal_cerr << "Changing SIGILL to SIGTRAP" << endl;
		       sig = SIGTRAP;
		  }
#ifdef DETACH_ON_THE_FLY
		  else {
		       /* FIXME: If the inferior executes a bona fide illegal instruction,
			  the resulting signal is now lost. */
		       signal_cerr << "Changing SIGILL to SIGSTOP" << endl;
		       sig = SIGSTOP;
		       fix_ill = 1;
		  }
#endif /* DETACH_ON_THE_FLY */
	}
#endif

	switch (sig) {
	    case SIGTSTP:
		sprintf(errorLine, "process %d got SIGTSTP", pid);
		statusLine(errorLine);
		curr->Stopped();
		break;

	    case SIGTRAP: {
		// Note that there are now several uses for SIGTRAPs in paradynd.
		// The original use was to detect when a ptraced process had
		// started up.  Several have been added.  We must be careful
		// to make sure that uses of SIGTRAPs do not conflict.
#ifndef rs6000_ibm_aix4_1
	      signal_cerr << "welcome to SIGTRAP for pid " << curr->getPid()
			    << " status =" << curr->getStatusAsString()
		/* FIXME: On AIX this call currentPC causes the Dyninst tests to fail. */ 
			    << " pc = " << curr->currentPC() << endl;
#endif /* rs6000_ibm_aix4_1 */
#ifdef DETACH_ON_THE_FLY
		const bool wasRunning = (curr->status() == running) || curr->juststopped;
#else
		const bool wasRunning = (curr->status() == running);
#endif
		curr->status_ = stopped;

		// Check to see if the TRAP is due to a system call exit which
		// we were waiting for, in order to launch an inferior rpc safely.
		if (curr->isRPCwaitingForSysCallToComplete()) {
		   inferiorrpc_cerr << "got SIGTRAP indicating syscall completion!"
		                    << endl;
		   curr->setRPCwaitingForSysCallToComplete(false);

		   if (curr->launchRPCifAppropriate(false, // not running
						    true   // finishing a syscall
						    )) {
		      inferiorrpc_cerr << "syscall completion: rpc launched ok, as expected " << endl;
		   }
		   else {
		     inferiorrpc_cerr << "syscall completion: failed to launch rpc" << endl;
		     if (!curr->continueProc()) {
		       sprintf(errorLine,"WARNING: failed to continue process with pid=%d\n",curr->getPid());
		       logLine(errorLine);
		     }
		   }
		   break;
		}

                // check to see if trap is due to dlopen or dlcose event
		if(curr->isDynamicallyLinked()){
		    if(curr->handleIfDueToSharedObjectMapping()){
		      inferiorrpc_cerr << "handle TRAP due to dlopen or dlclose event" <<endl;
#ifdef DETACH_ON_THE_FLY
		      if (!wasRunning)
			   break;
		      if (curr->needsDetach) {
			   if (!curr->detachAndContinue())
				assert(0);
			   break;
		      }
		      if (!curr->continueProc()) {
			   assert(0);
		      }
#else
		      if (wasRunning && !curr->continueProc()) {
			assert(0);
		      }
#endif
		      break;
		    }
		}

		// this code has to go after we have handle the trap
		// due to the call to dlopen - naim
		if (curr->trapDueToDyninstLib()) {
			// signal_cerr << "trapDueToDyninstLib returned true, trying to handle" << endl;
		  curr->handleIfDueToDyninstLib();
		  // fall through...
		}

		//If the list is not empty, it means some previous
		//instrumentation has yet need to be finished.
		if (instWList.size() != 0) {
			cerr << "instWList is full" << endl;
		    if(curr -> cleanUpInstrumentation(wasRunning)){
		        inferiorrpc_cerr << "instWList is full, cleanUpInstrumentation" << endl;
		        break; // successfully processed the SIGTRAP
                    }
		}

		if (curr->handleTrapIfDueToRPC()) {
		  inferiorrpc_cerr << "processed RPC response in SIGTRAP" << endl;
		   break;
		}

		if (curr->inExec) {
		   // the process has executed a succesful exec, and is now
		   // stopped at the exit of the exec call.

		   forkexec_cerr << "SIGTRAP: inExec is true, so doing process::handleExec()!" << endl;
		   string buffer = string("process ") + string(curr->getPid()) +
		                   " has performed exec() syscall";
		   statusLine(buffer.c_str());

		   // call handleExec to clean our internal data structures, reparse
		   // symbol table.  handleExec does not insert instrumentation or do
		   // any propagation.  Why not?  Because it wouldn't be safe -- the
		   // process hasn't yet been bootstrapped (run DYNINST, etc.)  Only
		   // when we get the breakpoint at the end of DYNINSTinit() is it safe
		   // to insert instrumentation and allocate timers & counters...
		   curr->handleExec();

#ifndef BPATCH_LIBRARY
		   // need to start resourceBatchMode here because we will call
		   //  tryToReadAndProcessBootstrapInfo which
		   // calls tp->resourceBatchMode(false)
		   tp->resourceBatchMode(true);
#else
		   // BPatch::bpatch->registerExec(curr->thread);
#endif

		   // set reachedFirstBreak to false here, so we execute
		   // the code below, and insert the initial instrumentation
		   // in the new image. (Actually, this is already done by handleExec())
		   curr->reachedFirstBreak = false;

		   // Since exec will 'remove' our library, we must redo the whole process
		   curr->reachedVeryFirstTrap = false;
		   curr->clearDyninstLibLoadFlags();
		   forkexec_cerr << "About to start exec reconstruction of shared library" << endl;

		   // fall through...
		}

		// Now we expect that this TRAP is the initial trap sent when a ptrace'd
		// process completes startup via exec (or when an exec syscall was
		// executed in an already-running process).
		// But we must query 'reachedFirstBreak' because on machines where we
		// attach/detach on pause/continue, a TRAP is generated on each pause!

		if (!curr->reachedVeryFirstTrap) {
		  // we haven't executed main yet, so we can insert a trap
		  // at the entry point of main - naim
		  curr->reachedVeryFirstTrap = true;
		  curr->insertTrapAtEntryPointOfMain();
		  if (!curr->continueProc()) {
		    assert(0);
		  }
		  break;
		} else {
		  if (curr->trapAtEntryPointOfMain() &&
		      !curr->dyninstLibAlreadyLoaded() &&
		      !curr->dyninstLibIsBeingLoaded()) {
		    curr->handleTrapAtEntryPointOfMain();
		    if (curr->handleStartProcess()) {
		      /* handleStartProcess may detect that dyninstlib was
			 linked into the application */
		      if (!curr->dyninstLibAlreadyLoaded())	  
			curr->dlopenDYNINSTlib();
		      // this will set isLoadingDyninstLib to true - naim
		    } else {
		      logLine("WARNING: handleStartProcess failed\n");
		      assert(0);
		    }
		    // at main
		    if (!curr->continueProc()) {
		      assert(0);
		    }
		    signal_cerr << "Process continued for dlopen" << endl;
		    break;
		  }
		  // fall through...
		}
		if (curr->dyninstLibAlreadyLoaded() &&
		    !curr->dyninstLibIsBeingLoaded()) {
		  // we are ready to handle reachedFirstBreak. The first
		  // condition means that dyninstRT.so.1 has been loaded
		  // already. The second condition makes sure that we have
		  // handle the trap after loading dyninstRT.so.1 and we
		  // have restored the original instructions in the place
		  // we use to call dlopen - naim
		  // process the code below yet - naim
		  ;
		} else {
		  // if this is not the case, then we are not ready to
		  // process the code below yet - naim
		  string msg = string("Process ") + string(curr->getPid()) +
		               string(" was unable to load file \"") + 
                               process::dyninstName + string("\"");
		  showErrorCallback(101, msg);
		  break;
		}

		if (!curr->reachedFirstBreak) { // vrble should be renamed 'reachedFirstTrap'
		   string buffer = string("PID=") + string(pid);
		   buffer += string(", passed trap at start of program");
		   statusLine(buffer.c_str());
		   // check for DYNINST symbols and initializes the inferiorHeap
		   // If libdyninst is dynamically linked, this can only be
		   // called after libdyninst is loaded
		   curr->initDyninstLib();

		   curr->reachedFirstBreak = true;

		   buffer=string("PID=") + string(pid) + string(", installing call to DYNINSTinit()");
		   statusLine(buffer.c_str());

		   curr->installBootstrapInst();

		   // now, let main() and then DYNINSTinit() get invoked.  As it
		   // completes, DYNINSTinit() does a DYNINSTbreakPoint, at which time
		   // we read bootstrap information "sent back" (in a global vrble),
		   // propagate metric instances, do tp->newProgramCallbackFunc(), etc.
                   // inferiorRPC is used to do DYNINSTinit, don't want to 
                   // continueProc() because it'll kill the applications.
                   // don't know why this is so. -Chun.
		   if (!curr->continueProc()) {
		      cerr << "continueProc after installBootstrapInst() failed, so DYNINSTinit() isn't being invoked yet, which it should!" << endl;
		   }
		   else 
		   {
		      buffer=string("PID=") + string(pid) + string(", running DYNINSTinit()...");
		      statusLine(buffer.c_str());
		   }
		}
		else {
#if defined(i386_unknown_linux2_0) || defined(ia64_unknown_linux2_4)
			Address pc = getPC( pid );
			if( orig_sig == SIGTRAP )
			{
				//signal_cerr << "SIGTRAP not handled for pid " << pid << " at " << (void*)pc << ", so forwarding back to process" << endl << flush;
				if( P_ptrace(PTRACE_CONT, pid, 1, SIGTRAP) == -1 )
					cerr << "ERROR -- process::handleSigChild forwarding SIGTRAP -- " << sys_errlist;
			}
			else
			{
				signal_cerr << "Signal " << orig_sig << " not handled for pid " << pid << " at " << (void*)pc << ", so leaving process in current state" << endl << flush;
			}
#endif
		}

		break;
	    }

#ifdef USE_IRIX_FIXES
            case SIGEMT:
#endif
	    case SIGSTOP:
	    case SIGINT: {
	        signal_cerr << "welcome to SIGSTOP/SIGINT for proc pid " << curr->getPid() << endl;

		const processState prevStatus = curr->status_;
		// assert(prevStatus != stopped); (bombs on sunos and AIX, when lots of spurious sigstops are delivered)

		curr->status_ = stopped;
		   // the following routines expect (and assert) this status.

#ifdef DETACH_ON_THE_FLY
		/* During rtinst initialization for attachProcess, we
		   convert an RPC trap into a STOP.  We catch it here,
		   otherwise procStopFromDYNINSTinit will mishandle
		   it. */
 	        if (curr->handleTrapIfDueToRPC()) {
		     inferiorrpc_cerr << "processed RPC response in SIGSTOP!" << endl; cerr.flush();
		     break; // we don't forward the signal -- on purpose
		}

		/* If an illegal instruction (assume "ud2") caused
                   this event, but it was not an inferior RPC, advance
                   the PC past the illegal instruction. */
		if (fix_ill) {
		     Address pc = getPC(pid);
#if defined(i386_unknown_linux2_0)
		     if (! curr->changePC(pc + 2))
			  assert(0);
#else
			/* Advance past an illegal instruction on IA-64? */
			assert(0);
#endif		
		}
		fix_ill = 0;
#endif
		int result = curr->procStopFromDYNINSTinit();
		assert(result >=0 && result <= 2);
		if (result != 0) {
		   forkexec_cerr << "processed SIGSTOP from DYNINSTinit for pid " << curr->getPid() << endl << flush;
		   if (result == 1) {
		      assert(curr->status_ == stopped);
		      // DYNINSTinit() after normal startup, after fork, or after exec
		      // syscall was made by a running program; leave paused
		      // (tp->newProgramCallback() to paradyn will result in the process
		      // being continued soon enough, assuming the applic was running,
		      // which is true in all cases except when an applic is just being
		      // started up).  Fall through (we want the status line to change)
		   } else {
		      assert(result == 2);
		      break; // don't fall through...prog is finishing the inferiorRPC
		   }
		}
		else if (curr->handleTrapIfDueToRPC()) {
		   inferiorrpc_cerr << "processed RPC response in SIGSTOP" << endl;
		   break; // don't want to execute ->Stopped() which changes status line
		}
#ifndef BPATCH_LIBRARY
		/* XXX We have to leave this out of the Dyninst API library
		 * for now -- the ptrace calls that handleStopDueExecEntry
		 * uses cause wait() to re-report the signal that we're
		 * currently stopped on, causing an infinite loop.
		 * Exec doesn't work with the Dyninst API yet anyway.
		 * - brb 7/4/98 */
		else if (curr->handleStopDueToExecEntry()) {
		   // grabs data from DYNINST_bootstrap_info
		   forkexec_cerr << "fork/exec -- handled stop before exec" << endl;
		   string buffer = string("process ") + string(curr->getPid()) +
		                   " performing exec() syscall...";
		   statusLine(buffer.c_str());

		   // note: status will now be 'running', since handleStopDueToExec()
		   // did a continueProc() to let the exec() syscall go forward.
		   assert(curr->status_ == running);
		      // would neonatal be better? or exited?

		   break; // don't want to change status line in conventional way
		}
#endif /* BPATCH_LIBRARY */
		else {
		   signal_cerr << "unhandled SIGSTOP for pid " << curr->getPid() << " so just leaving process in paused state." << endl << flush;
		}
		curr->status_ = prevStatus; // so Stopped() below won't be a nop
		curr->Stopped();

		break;
	    }

	    case SIGILL:
	      signal_cerr << "welcome to SIGILL" << endl << flush;
	       curr->status_ = stopped;

	       if (curr->handleTrapIfDueToRPC()) {
		  inferiorrpc_cerr << "processed RPC response in SIGILL" << endl; cerr.flush();

		  break; // we don't forward the signal -- on purpose
	       }
	       else
		  // fall through, on purpose
		  ;

	    case SIGIOT:
	    case SIGBUS:
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
                if (ptrace(PT_DETACH,pid,(int *) 1, SIGSTOP, NULL) == -1) {
#else
                if (ptrace(PT_DETACH, pid, 1, SIGSTOP, NULL) == -1) { 
#endif
                  logLine("ptrace error\n");
                }
#else

		signal_cerr << "caught signal, dying...  (sig="
                            << WSTOPSIG(status) << ")" << endl << flush;

		curr->status_ = stopped;
#ifdef BPATCH_LIBRARY
		curr->dumpImage("imagefile");
#else
		curr->dumpImage();
#endif
		curr->continueWithForwardSignal(WSTOPSIG(status));
#endif
		break;

	    case SIGALRM:
#ifndef SHM_SAMPLING
		// Due to the DYNINSTin_sample variable, it's safest to launch
		// inferior-RPCs only when we know that the inferior is not in the
		// middle of processing an alarm-expire.  Otherwise, code that does
		// stuff like call DYNINSTstartWallTimer will appear to do nothing
		// (DYNINSTstartWallTimer will be invoked but will see that
		//  DYNINSTin_sample is set and so bails out!!!)
		// Ick.
		if (curr->existsRPCreadyToLaunch()) {
		   curr -> status_ = stopped;
		   (void)curr->launchRPCifAppropriate(true, false);
		   break; // sure, we lose the SIGALARM, but so what.
		}
		else
		   ; // no break, on purpose
#endif

	    case SIGCHLD:
	    case SIGUSR1:
	    case SIGUSR2:
	    case SIGVTALRM:
	    case SIGCONT:
	    case SIGSEGV:	// treadmarks needs this signal
#if (defined(POWER_DEBUG) || defined(HP_DEBUG)) && (defined(rs6000_ibm_aix4_1) || defined(hppa1_1_hp_hpux))
                // In this way, we will detach from the application and we
                // will be able to attach again using gdb - naim
                if (sig==SIGSEGV)
		{
#if defined(rs6000_ibm_aix4_1)
                  if (ptrace(PT_DETACH,pid,(int *) 1, SIGSTOP, NULL) == -1)
#else
                  if (ptrace(PT_DETACH,pid, 1, SIGSTOP, NULL) == -1)
#endif
                    logLine("ptrace error\n");
                  break;
                }
#endif
		if (!curr->continueWithForwardSignal(WSTOPSIG(status))) {
                     logLine("error  in forwarding  signal\n");
		     showErrorCallback(38, "Error  in forwarding  signal");
                     //P_abort();
                }

		break;

#ifdef notdef
	    // XXXX for debugging
	    case SIGSEGV:	// treadmarks needs this signal
		sprintf(errorLine, "DEBUG: forwarding signal (sig=%d, pid=%d)\n"
			, WSTOPSIG(status), pid);
		logLine(errorLine);
#endif
	    default:
		if (!curr->continueWithForwardSignal(WSTOPSIG(status))) {
                     logLine("error  in forwarding  signal\n");
                     P_abort();
                }
		break;

	}
    } else if (WIFEXITED(status)) {
#if defined(PARADYND_PVM)
//        if (pvm_running) {
//            PDYN_reportSIGCHLD (pid, WEXITSTATUS(status));
//	}
#endif
	sprintf(errorLine, "Process %d has terminated with code 0x%x\n", 
                curr->getPid(), WEXITSTATUS(status));
	statusLine(errorLine);
	logLine(errorLine);
        handleProcessExit(curr, WEXITSTATUS(status));
    } else if (WIFSIGNALED(status)) {
	sprintf(errorLine, "process %d has terminated on signal %d\n", curr->getPid(), WTERMSIG(status));
	logLine(errorLine);
	statusLine(errorLine);
	printDyninstStats();
	handleProcessExit(curr, WTERMSIG(status));
    } else {
	sprintf(errorLine, "Unknown state %d from process %d\n", status, curr->getPid());
	logLine(errorLine);
	showErrorCallback(39,(const char *) errorLine);
    }
    return(0);
}


void checkProcStatus() {
   /* check for status change on inferior processes, or the arrival of
      a signal. */

   int wait_status;
   int wait_pid = process::waitProcs(&wait_status);
    if (wait_pid > 0) {
      if (handleSigChild(wait_pid, wait_status) < 0) {
	 cerr << "handleSigChild failed for pid " << wait_pid << endl;
      }
   }
}


bool  OS::osKill(int pid) {
  return (P_kill(pid,9)==0);
}
