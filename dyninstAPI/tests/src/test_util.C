
//
// Utility functions for use by the dyninst API test programs.
//

#include <stdio.h>
#include <signal.h>
#if defined(sparc_sun_sunos4_1_3) || defined(sparc_sun_solaris2_4)
#include <unistd.h>
#endif
#ifdef i386_unknown_nt4_0
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#endif

#include "BPatch_Vector.h"
#include "BPatch_thread.h"


//
// Wait for the mutatee to stop.
//
void waitUntilStopped(BPatch_thread *appThread, int testnum, char *testname)
{
    // Wait for process to stop
    while (!appThread->isStopped() && !appThread->isTerminated()) ;
    if (!appThread->isStopped()) {
	printf("**Failed test #%d (%s)\n", testnum, testname);
	printf("    process did not signal mutator via stop\n");
	exit(-1);
    }
#ifdef i386_unknown_nt4_0
    else if (appThread->stopSignal() != SIGTRAP) {
	printf("**Failed test #%d (%s)\n", testnum, testname);
	printf("    process stopped on signal %d, not SIGTRAP\n", 
		appThread->stopSignal());
	exit(-1);
    }
#else
    else if (appThread->stopSignal() != SIGSTOP) {
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
void signalAttached(BPatch_thread *appThread, BPatch_image *appImage)
{
    BPatch_variableExpr *isAttached = appImage->findVariable("isAttached");
    if (isAttached == NULL) {
	printf("*ERROR*: unable to start tests because variable \"isAttached\" could not be found in the child process\n");
	exit(-1);
    }

    BPatch_arithExpr setAttached(BPatch_assign, *isAttached,
				 BPatch_constExpr(1));
    
    // Find the point(s) we'll be instrumenting
    BPatch_Vector<BPatch_point *> *points =
	appImage->findProcedurePoint("checkIfAttached", BPatch_entry);
    if (!points) {
	fprintf(stderr, "*ERROR*: unable to start tests because the entry point to the function \"checkIfAttached\" could not be located\n");
	exit(-1);
    }

    if (appThread->insertSnippet(setAttached, *points) == NULL) {
	fprintf(stderr, "*ERROR*: unable to start tests because the entry point to the function \"checkIfAttached\" could not be instrumented\n");
	exit(-1);
    }
}


//
// Create a new process and return its process id.  If process creation 
// fails, this function returns -1.
//
int startNewProcess(char *pathname, char *argv[])
{
#ifdef i386_unknown_nt4_0
    char child_args[1024];
    strcpy(child_args, "");
    if (argv[0] != NULL) {
	strcpy(child_args, pathname);
	for (int i = 1; argv[i] != NULL; i++) {
	    strcat(child_args, " ");
	    strcat(child_args, argv[i]);
	}	    
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
    int pid = fork();
    if (pid == 0) {
	// child - so exec 
	execv(pathname, argv);
	_exit(-1);
    } else if (pid < 0) {
	return -1;
    }
    return pid;
#endif
}
