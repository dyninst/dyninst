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

//
// $Id: test_util.C,v 1.19 2004/04/20 01:27:56 jaw Exp $
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
void waitUntilStopped(BPatch *bpatch, BPatch_thread *appThread, int testnum,
                      const char *testname)
{
    while (!appThread->isStopped() && !appThread->isTerminated())
        bpatch->waitForStatusChange();
    
    if (!appThread->isStopped()) {
        printf("**Failed test #%d (%s)\n", testnum, testname);
        printf("    process did not signal mutator via stop\n");
        fprintf(stderr, "thread is not stopped\n");
        exit(-1);
    }
#if defined(i386_unknown_nt4_0)  || defined(mips_unknown_ce2_11) //ccw 10 apr 2001
    else if (appThread->stopSignal() != EXCEPTION_BREAKPOINT && appThread->stopSignal() != -1) {
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
#if defined(bug_irix_broken_sigstop)
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
int startNewProcessForAttach(const char *pathname, const char *argv[])
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
    const char **attach_argv = (const char**)malloc(sizeof(char *) * (i + 3));

    for (i = 0; argv[i] != NULL; i++)
	attach_argv[i] = argv[i];
    attach_argv[i++] = const_cast<char*>("-attach");
    attach_argv[i++] = fdstr;
    attach_argv[i++] = NULL;

    int pid = fork();
    if (pid == 0) {
	// child
	close(fds[0]); // We don't need the read side
	execv(pathname, (char * const *)attach_argv);
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
