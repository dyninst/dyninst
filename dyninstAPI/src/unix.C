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

// $Id: unix.C,v 1.78 2003/03/04 19:16:05 willb Exp $

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
#include "paradynd/src/pd_process.h"
#endif

// the following are needed for handleSigChild
#include "dyninstAPI/src/process.h"
#include "dyninstAPI/src/dyn_lwp.h"
#include "dyninstAPI/src/instP.h"
#include "dyninstAPI/src/stats.h"
extern process *findProcess(int);

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
bool forkNewProcess(string &file, string /*dir*/, pdvector<string> argv, 
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

/* 
   TODO: cleanup handleSigChild. This function has a lot of code that
   should be moved to a machine independent place (a lot of what is
   going on here will probably have to be moved anyway once we move to
   the dyninstAPI).

   There is a different version of this function for WindowsNT. If any changes
   are made here, they will probably also be needed in the NT version.

   --mjrg
*/

void checkAndDoExecHandling(process *curr) {
   if(curr->inExec == false) return;
   
   // the process has executed a succesful exec, and is now
   // stopped at the exit of the exec call.
   
   forkexec_cerr << "SIGTRAP: inExec is true, so doing "
                 << "process::handleExec()!\n";
   
   string buffer = string("process ") + string(curr->getPid()) +
      " has performed exec() syscall";
   statusLine(buffer.c_str());
   
   // call handleExec to clean our internal data structures, reparse
   // symbol table.  handleExec does not insert instrumentation or do any
   // propagation.  Why not?  Because it wouldn't be safe -- the process
   // hasn't yet been bootstrapped (run DYNINST, etc.)  Only when we get
   // the breakpoint at the end of DYNINSTinit() is it safe to insert
   // instrumentation and allocate timers & counters...
   curr->handleExec();
   
#ifndef BPATCH_LIBRARY
   // need to start resourceBatchMode here because we will call
   //  tryToReadAndProcessBootstrapInfo which
   // calls tp->resourceBatchMode(false)
   tp->resourceBatchMode(true);
#else
   // BPatch::bpatch->registerExec(curr->thread);
#endif
      
   // Since exec will 'remove' our library, we must redo the whole process
   forkexec_cerr <<"About to start exec reconstruction of shared library\n";
   // fall through...
}

void handleSigTrap(process *curr, int pid, int linux_orig_sig) {
   // Note that there are now several uses for SIGTRAPs in paradynd.  The
   // original use was to detect when a ptraced process had started up.
   // Several have been added.  We must be careful to make sure that uses of
   // SIGTRAPs do not conflict.

  signal_cerr << "welcome to SIGTRAP for pid " << curr->getPid()
	       << " status =" << curr->getStatusAsString() << endl;

   const bool wasRunning = (curr->status() == running);
   curr->status_ = stopped;

   checkAndDoExecHandling(curr);

/////////////////////////////////////////
// Init section
/////////////////////////////////////////
   
   if (!curr->reachedBootstrapState(bootstrapped)) {
       
       if (!curr->reachedBootstrapState(begun)) {
           // We've created the process, but haven't reached main. 
           // This would be a perfect place to load the dyninst library,
           // but all the other shared libs aren't initialized yet. 
           // So we wait for main to be entered before working on the process.
           
           curr->setBootstrapState(begun);
           curr->insertTrapAtEntryPointOfMain();
           string buffer = string("PID=") + string(pid);
           buffer += string(", attached to process, stepping to main");       
           statusLine(buffer.c_str());
           if (!curr->continueProc()) {
               assert(0);
           }
           // Now we wait for the entry point trap to be reached
           
           return;
       }
       else if (curr->trapAtEntryPointOfMain()) {
           
           curr->handleTrapAtEntryPointOfMain();
           curr->setBootstrapState(initialized);
           if(curr->wasExeced())
               curr->loadDyninstLib();

           return;
       }
       else if (curr->trapDueToDyninstLib()) {
           
           string buffer = string("PID=") + string(pid);
           buffer += string(", loaded dyninst library");
           statusLine(buffer.c_str());
           //signal_cerr << "trapDueToDyninstLib returned true, trying to handle\n";
           
           curr->loadDYNINSTlibCleanup();
           curr->setBootstrapState(loadedRT);
           return;
       }
   }

///////////////////////////////////
// Inferior RPC section
///////////////////////////////////

   // New and improved RPC handling, takes care of both
   // an RPC which has reached a breakpoint and whether
   // we're waiting for a syscall to complete

   if (curr->handleTrapIfDueToRPC()) {
       signal_cerr << "processed RPC response in SIGTRAP" << endl;
       return;
   }
   

/////////////////////////////////////////
// dlopen/close section
/////////////////////////////////////////

   // check to see if trap is due to dlopen or dlcose event
   if(curr->isDynamicallyLinked()){
       if(curr->handleIfDueToSharedObjectMapping()){
           signal_cerr << "handle TRAP due to dlopen or dlclose event\n";
           if (!curr->continueProc()) {
               assert(0);
           }
           return;
       }
   }
   
#if defined(i386_unknown_linux2_0) || defined(ia64_unknown_linux2_4)
   Address pc = getPC( pid );
   if( linux_orig_sig == SIGTRAP ) {
       //signal_cerr << "SIGTRAP not handled for pid " << pid << " at " 
       //       << (void*)pc << ", so forwarding back to process\n" << flush;
       if( P_ptrace(PTRACE_CONT, pid, 1, SIGTRAP) == -1 )
           cerr << "ERROR -- process::handleSigChild forwarding SIGTRAP -- " 
                << sys_errlist;
   } else {
       signal_cerr << "Signal " << linux_orig_sig << " not handled for pid " 
                   << pid << " at " << (void*)pc 
                   << ", so leaving process in current state\n" << flush;
   }
#endif
   cerr << "NO response to SIGTRAP" << endl;
   return;
}

int paradynForkOccurring(process *proc) {
  bool err = false;
  const char *fork_occurring_var = "paradyn_fork_occurring";
  Address forkOccurVarAddr = proc->findInternalAddress(fork_occurring_var,
						       false, err);
  if(err == true) {
    // ie. the symbol can't be found, eg. if the paradyn rtinst library
    // hasn't been loaded yet
    return false;
  }

  int appAddrWidth = proc->getImage()->getObject().getAddressWidth();
  //cerr << "address of var " << fork_occurring_var << " = " << hex 
  //     << forkOccurVarAddr << ", width is " << appAddrWidth << " bytes\n"
  //     << dec;

  int forkOccurringVal = 0;
  if (!proc->readDataSpace((caddr_t)forkOccurVarAddr, appAddrWidth, 
			   &forkOccurringVal, true)) {
    cerr << "can't read var " << fork_occurring_var <<"\n";
    return false;
  }
  //cerr << "read " << fork_occurring_var << " and has value of "
  //     << forkOccurringVal << "\n";

  return forkOccurringVal;
}

void handleSig_StopAndInt(process *curr, int pid) {
   signal_cerr << "welcome to SIGSTOP/SIGINT for proc pid " << curr->getPid() 
	       << endl;
   
   const processState prevStatus = curr->status_;
   // assert(prevStatus != stopped); (bombs on sunos and AIX, when
   // lots of spurious sigstops are delivered)
   
   curr->status_ = stopped;
   // the following routines expect (and assert) this status.

   int forkCode = paradynForkOccurring(curr);
   bool atChildStop = (forkCode == 1);
   bool atParentStop = (forkCode == 2);
   bool doingFork = (atChildStop || atParentStop);
   int result = 0;
   int stopfromPARADYNinit = 0;

   if (curr->handleTrapIfDueToRPC()) {
       inferiorrpc_cerr << "processed RPC response in SIGSTOP\n";
       // don't want to execute ->Stopped() which changes status line
       return;
   }
#if 0
   /* XXX We have to leave this out of the Dyninst API library for now -- the
    * ptrace calls that handleStopDueExecEntry uses cause wait() to re-report
    * the signal that we're currently stopped on, causing an infinite loop.
    * Exec doesn't work with the Dyninst API yet anyway.  - brb 7/4/98 */
   else if (curr->handleStopDueToExecEntry()) {
       // grabs data from DYNINST_bootstrap_info
       forkexec_cerr << "fork/exec -- handled stop before exec\n";
       string buffer = string("process ") + string(curr->getPid())
       + " performing exec() syscall...";
       statusLine(buffer.c_str());
       
       // note: status will now be 'running', since handleStopDueToExec() did
       // a continueProc() to let the exec() syscall go forward.
       assert(curr->status_ == running);
       // would neonatal be better? or exited?
       
       return; // don't want to change status line in conventional way
   }
#endif /* BPATCH_LIBRARY */
   else {
       signal_cerr << "unhandled SIGSTOP for pid " << curr->getPid() 
                   << " so just leaving process in paused state.\n" 
                   << flush;
   }
   curr->status_ = prevStatus; // so Stopped() below won't be a nop
   curr->Stopped();
   return;
}

void handleSigBus(process *curr, int status) {
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
   if (ptrace(PT_DETACH,pid,(int *) 1, SIGSTOP, NULL) == -1)
#else
      if (ptrace(PT_DETACH, pid, 1, SIGSTOP, NULL) == -1)
#endif
      {
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
   return;
}

void handleSigSegv(process *curr, int status) {
#if (defined(POWER_DEBUG) || defined(HP_DEBUG)) && \
    (defined(rs6000_ibm_aix4_1) || defined(hppa1_1_hp_hpux))
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
#ifdef notdef
   // XXXX for debugging
  case SIGSEGV:	// treadmarks needs this signal
     sprintf(errorLine, "DEBUG: forwarding signal (sig=%d, pid=%d)\n"
	     , WSTOPSIG(status), pid);
     logLine(errorLine);
#endif
   return;
}

// returns 1 if should do break, 0 otherwise
int handleSigAlarm(process *curr) {
   // Due to the DYNINSTin_sample variable, it's safest to launch
   // inferior-RPCs only when we know that the inferior is not in the
   // middle of processing an alarm-expire.  Otherwise, code that
   // does stuff like call DYNINSTstartWallTimer will appear to do
   // nothing (DYNINSTstartWallTimer will be invoked but will see
   // that DYNINSTin_sample is set and so bails out!!!)  Ick.
    if (curr->existsRPCPending()) {
        curr->status_ = stopped;
        (void)curr->launchRPCs(true);
        return 1; // sure, we lose the SIGALARM, but so what.
    }
    else { }  // no break, on purpose
    return 0;
}
      
void handleStopStatus(int pid, int status, process *curr) {
   int sig = WSTOPSIG(status);

   int linux_orig_sig = -1;
#if defined( i386_unknown_linux2_0 ) || defined( ia64_unknown_linux2_4 )
   linux_orig_sig = sig;
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
               || curr->existsRPCWaitingForSyscall()) { //ccw 30 apr 2002 
               signal_cerr << "Changing SIGILL to SIGTRAP" << endl;
               sig = SIGTRAP;
           }
   }
#endif
   
   switch (sig) {
     case SIGTSTP:
	sprintf(errorLine, "process %d got SIGTSTP", pid);
	statusLine(errorLine);
	curr->Stopped();
	break;   
     case SIGTRAP: 
	handleSigTrap(curr, pid, linux_orig_sig);
	break;
#ifdef USE_IRIX_FIXES
     case SIGEMT:
#endif
     case SIGSTOP:
     case SIGINT: 
	handleSig_StopAndInt(curr, pid);
	break;
     case SIGILL:
	signal_cerr << "welcome to SIGILL" << endl << flush;
	curr->status_ = stopped;
	
	if (curr->handleTrapIfDueToRPC()) {
	   inferiorrpc_cerr << "processed RPC response in SIGILL\n";
	   cerr.flush();
	   break; // we don't forward the signal -- on purpose
	}
	else { }  // fall through, on purpose
	
     case SIGIOT:
     case SIGBUS:
	handleSigBus(curr, status);
	break;
     case SIGALRM:
#ifndef SHM_SAMPLING
	if(handleSigAlarm(curr))  break;
	// else fall through, don't call break
#endif
	
     case SIGCHLD:
     case SIGUSR1:
     case SIGUSR2:
     case SIGVTALRM:
     case SIGCONT:
     case SIGSEGV:	// treadmarks needs this signal
	handleSigSegv(curr, status);
	break;
     default:
	if (!curr->continueWithForwardSignal(WSTOPSIG(status))) {
	   logLine("error  in forwarding  signal\n");
	   P_abort();
	}
	break;
   }
   return;
}

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
      forkexec_cerr << "handleSigChild pid " << pid << " is an unknown "
		    << " process.\nWIFSTOPPED=" 
		    << (WIFSTOPPED(status) ? "true" : "false");
      if (WIFSTOPPED(status)) {
	 forkexec_cerr << "WSTOPSIG=" << WSTOPSIG(status);
      }
      forkexec_cerr << endl << flush;
      /*
	This was causing problems with fork handling on AIX.
	Here's the situation which would cause the assert:
	Daemon                               RTinst
                                             DYNINSTfork in child
                                             send trace record
					     kill(self, SIGSTOP)
        busy with handling other process
	handleSigChild, the stop
        this code, ptrace(PT_DETACH, pid)
	                                     goes past breakpoint (bad)
					     (this messes up the order)
        processNewTSConn., copy proc obj
        (tries to continue proc, but it's
	 not at the breakpoint we expect)

	 * May be able to remove this soon
	 Drew said it was added because of some daemon which was launched
	 for mpi applications.  This may not actually occur anymore.

#ifdef rs6000_ibm_aix4_1
      cerr << "handleSigChild:  Detaching process " << pid 
	   << " and continuing." << endl;
      return ptrace(PT_DETACH, pid, (int *)0, SIGCONT, 0);   //Set process free
#endif

      */
      return -1;
   }

   // Normal case, actually.
   if (curr->status_ == exited) {
     return 0;
   }
   if (WIFSTOPPED(status))
   {
       handleStopStatus(pid, status, curr);
   } 
   else if (WIFEXITED(status)) {
      sprintf(errorLine, "Process %d has terminated with code 0x%x\n", 
	      curr->getPid(), WEXITSTATUS(status));
      statusLine(errorLine);
      logLine(errorLine);
      handleProcessExit(curr, WEXITSTATUS(status));
   }
   else if (WIFSIGNALED(status)) {
      sprintf(errorLine, "process %d has terminated on signal %d\n", 
	      curr->getPid(), WTERMSIG(status));
      logLine(errorLine);
      statusLine(errorLine);
      printDyninstStats();
      handleProcessExit(curr, WTERMSIG(status));
   } 
   else {
      sprintf(errorLine, "Unknown state %d from process %d\n", status, 
	      curr->getPid());
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
