//
// $Id: test_util.C,v 1.14 2001/08/01 15:39:59 chadd Exp $
// Utility functions for use by the dyninst API test programs.
//

#include <stdio.h>
#include <signal.h>

#if defined(i386_unknown_nt4_0) || defined(mips_unknown_ce2_11) //ccw 10 apr 2001 
#ifndef mips_unknown_ce2_11 //ccw 10 apr 2001
#define WIN32_LEAN_AND_MEAN
#endif
#include <Windows.h>
#else
#include <unistd.h>
#endif

#include "BPatch.h"
#include "BPatch_Vector.h"
#include "BPatch_thread.h"

//
// Wait for the mutatee to stop.
//
void waitUntilStopped(BPatch *bpatch,
	BPatch_thread *appThread, int testnum, char *testname)
{
    while (!appThread->isStopped() && !appThread->isTerminated())
	bpatch->waitForStatusChange();

    if (!appThread->isStopped()) {
	printf("**Failed test #%d (%s)\n", testnum, testname);
	printf("    process did not signal mutator via stop\n");
	exit(-1);
    }
#if defined(i386_unknown_nt4_0)  || defined(mips_unknown_ce2_11) //ccw 10 apr 2001
    else if (appThread->stopSignal() != SIGTRAP && appThread->stopSignal() != -1) {
	printf("**Failed test #%d (%s)\n", testnum, testname);
	printf("    process stopped on signal %d, not SIGTRAP\n", 
		appThread->stopSignal());
	exit(-1);
    }
#else
#ifdef DETACH_ON_THE_FLY
    /* FIXME: Why add SIGILL here? */
    else if ((appThread->stopSignal() != SIGSTOP) &&
	     (appThread->stopSignal() != SIGHUP) &&
	     (appThread->stopSignal() != SIGILL)) {
#else
    else if ((appThread->stopSignal() != SIGSTOP) &&
#ifdef USE_IRIX_FIXES
	     (appThread->stopSignal() != SIGEMT) &&
#endif
	     (appThread->stopSignal() != SIGHUP)) {
#endif /* DETACH_ON_THE_FLY */
	printf("**Failed test #%d (%s)\n", testnum, testname);
	printf("    process stopped on signal %d, not SIGSTOP\n", 
		appThread->stopSignal());
	exit(-1);
    }
#endif
}


//
// Signal the child that we've attached.  The child contains a function
// "checkIfAttached" which simply returns the value of the global variable
// "isAttached."  We add instrumentation to "checkIfAttached" to set
// "isAttached" to 1.
//
void signalAttached(BPatch_thread* /*appThread*/, BPatch_image *appImage)
{
    BPatch_variableExpr *isAttached = appImage->findVariable("isAttached");
    if (isAttached == NULL) {
	printf("*ERROR*: unable to start tests because variable \"isAttached\""
               " could not be found in the child process\n");
	exit(-1);
    }

    int yes = 1;
    isAttached->writeValue(&yes);
}


//
// Create a new process and return its process id.  If process creation 
// fails, this function returns -1.
//
int startNewProcessForAttach(char *pathname, char *argv[])
{
#if defined(i386_unknown_nt4_0)  || defined(mips_unknown_ce2_11) //ccw 10 apr 2001
    char child_args[1024];
    strcpy(child_args, "");
    if (argv[0] != NULL) {
	strcpy(child_args, pathname);
	for (int i = 1; argv[i] != NULL; i++) {
	    strcat(child_args, " ");
	    strcat(child_args, argv[i]);
	}	    
	strcat(child_args, " -attach");
    }

    STARTUPINFO si;
    memset(&si, 0, sizeof(STARTUPINFO));
    si.cb = sizeof(STARTUPINFO);
    PROCESS_INFORMATION pi;
    if (!CreateProcess(pathname,	// application name
		       child_args,	// command line
		       NULL,		// security attributes
		       NULL,		// thread security attributes
		       FALSE,		// inherit handles
		       0,		// creation flags
		       NULL,		// environment,
		       NULL,		// current directory
		       &si,
		       &pi)) {
	return -1;
    }

    return pi.dwProcessId;
#else
    /* Make a pipe that we will use to signal that the mutatee has started. */
    int fds[2];
    if (pipe(fds) != 0) {
	fprintf(stderr, "*ERROR*: Unable to create pipe.\n");
	exit(-1);
    }

    /* Create the argv string for the child process. */
    char fdstr[32];
    sprintf(fdstr, "%d", fds[1]);

    int i;
    for (i = 0; argv[i] != NULL; i++) ;
    char **attach_argv = (char**)malloc(sizeof(char *) * (i + 3));

    for (i = 0; argv[i] != NULL; i++)
	attach_argv[i] = argv[i];
    attach_argv[i++] = "-attach";
    attach_argv[i++] = fdstr;
    attach_argv[i++] = NULL;

    int pid = fork();
    if (pid == 0) {
	// child
	close(fds[0]); // We don't need the read side
	execv(pathname, attach_argv);
	exit(-1);
    } else if (pid < 0) {
	return -1;
    }

    // parent
    close(fds[1]);  // We don't need the write side

    // Wait for the child to write to the pipe
    char ch;
    if (read(fds[0], &ch, sizeof(char)) != sizeof(char)) {
	perror("read");
	fprintf(stderr, "*ERROR*: Error reading from pipe\n");
	exit(-1);
    }

    if (ch != 'T') {
	fprintf(stderr, "*ERROR*: Child didn't write expected value to pipe.\n");
	exit(-1);
    }

    close(fds[0]);  // We're done with the pipe

    return pid;
#endif
}
