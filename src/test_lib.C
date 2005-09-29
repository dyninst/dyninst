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
// $Id: test_lib.C,v 1.1 2005/09/29 20:40:08 bpellin Exp $
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
#include <sys/wait.h>
#endif
#include <stdarg.h>

// Blind inclusion from test9.C
#if defined(i386_unknown_linux2_0) \
 || defined(x86_64_unknown_linux2_4) /* Blind duplication - Ray */ \
 || defined(sparc_sun_solaris2_4)
#include <sys/types.h>
#include <sys/wait.h>
#endif

#if defined(i386_unknown_linux2_0) \
 || defined(x86_64_unknown_linux2_4)
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#endif
// end inclusion from test9.C

#include "BPatch.h"
#include "BPatch_Vector.h"
#include "BPatch_thread.h"
#include "test_lib.h"

int expectError = DYNINST_NO_ERROR;

/* Control Debug printf statements */
int debugPrint = 0;
int mutateeFortran;

//
// Wait for the mutatee to stop.
//
int waitUntilStopped(BPatch *bpatch, BPatch_thread *appThread, int testnum,
                      const char *testname)
{
    while (!appThread->isStopped() && !appThread->isTerminated())
        bpatch->waitForStatusChange();
    
    if (!appThread->isStopped()) {
        printf("**Failed test #%d (%s)\n", testnum, testname);
        printf("    process did not signal mutator via stop\n");
        fprintf(stderr, "thread is not stopped\n");
        return -1;
    }
#if defined(i386_unknown_nt4_0)  || defined(mips_unknown_ce2_11) //ccw 10 apr 2001
    else if (appThread->stopSignal() != EXCEPTION_BREAKPOINT && appThread->stopSignal() != -1) {
	printf("**Failed test #%d (%s)\n", testnum, testname);
	printf("    process stopped on signal %d, not SIGTRAP\n", 
		appThread->stopSignal());
        return -1;
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
        return -1;
    }
#endif
    return 0;
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

void setMutateeFortran(int mutFor)
{
   mutateeFortran = mutFor;
}


void setDebugPrint(int debug) {
   debugPrint = debug;
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


// control debug printf statements
void dprintf(const char *fmt, ...) {
   va_list args;
   va_start(args, fmt);

   if(debugPrint)
      vfprintf(stderr, fmt, args);

   va_end(args);

   fflush(stderr);
}

void checkCost(BPatch_snippet snippet)
{
   float cost;
   BPatch_snippet copy;

   // test copy constructor too.
   copy = snippet;

   cost = snippet.getCost();
   dprintf("Snippet cost=%g\n", cost);
    if (cost < 0.0) {
        printf("*Error*: negative snippet cost\n");
    } else if (cost == 0.0) {
#if !defined(alpha_dec_osf4_0)
        dprintf("*Warning*: zero snippet cost\n");
#endif
    } else if (cost > 0.01) {
        printf("*Error*: snippet cost of %f, exceeds max expected of 0.1",
            cost);
    }
}

// Wrapper function to find variables
// For Fortran, will look for lowercase variable, if mixed case not found
BPatch_variableExpr *findVariable(BPatch_image *appImage, const char* var,
                                  BPatch_Vector <BPatch_point *> *point = NULL)
{
  //BPatch_variableExpr *FortVar = NULL;
    BPatch_variableExpr *ret = NULL;
    int i, numchars = strlen (var);
    char *lowercase = new char [numchars];
    int temp = expectError;

    if (mutateeFortran && point) {
            strcpy (lowercase, var);
            expectError = 100;
            for (i = 0; i < numchars; i++)
                lowercase [i] = tolower (lowercase [i]);
            ret = appImage->findVariable (*(*point) [0], lowercase);
        if (!ret) {
            expectError = temp;
            ret = appImage->findVariable (*(*point) [0], var);
        }
    } else {
        ret = appImage->findVariable (var);
    }

    expectError = temp;
    delete [] lowercase;
    return ret;
}


int functionNameMatch(const char *gotName, const char *targetName) 
{
    if (!strcmp(gotName, targetName)) return 0;

    if (!strncmp(gotName, targetName, strlen(targetName)) &&
	(strlen(targetName) == strlen(gotName)-1) &&
	(gotName[strlen(targetName)] == '_'))
	return 0;

    return 1;
}

//
// Replace all calls in "inFunction" to "callTo" with calls to "replacement."
// If "replacement" is NULL, them use removeFunctionCall instead of
// replaceFunctionCall.
// Returns the number of replacements that were performed.
//
int replaceFunctionCalls(BPatch_thread *appThread, BPatch_image *appImage,
                         const char *inFunction, const char *callTo, 
                         const char *replacement, int testNo, 
                         const char *testName, int callsExpected = -1)
{
    int numReplaced = 0;

    BPatch_Vector<BPatch_function *> found_funcs;
    if ((NULL == appImage->findFunction(inFunction, found_funcs)) || !found_funcs.size()) {
      fprintf(stderr, "**Failed** test #%d (%s)\n", testNo, testName);
      fprintf(stderr, "    Unable to find function %s\n",
	      inFunction);
      return -1;
    }

    if (1 < found_funcs.size()) {
      fprintf(stderr, "%s[%d]:  WARNING  : found %d functions named %s.  Using the first.\n", 
	      __FILE__, __LINE__, found_funcs.size(), inFunction);
    }

    BPatch_Vector<BPatch_point *> *points = found_funcs[0]->findPoint(BPatch_subroutine);

    if (!points || (!points->size() )) {
	fprintf(stderr, "**Failed** test #%d (%s)\n", testNo, testName);
	fprintf(stderr, "    %s[%d]: Unable to find point in %s - subroutine calls: pts = %p\n",
		__FILE__, __LINE__, inFunction,points);
        return -1;
    }

    BPatch_function *call_replacement = NULL;
    if (replacement != NULL) {
      
      BPatch_Vector<BPatch_function *> bpfv;
      if (NULL == appImage->findFunction(replacement, bpfv) || !bpfv.size()
	  || NULL == bpfv[0]){
	fprintf(stderr, "**Failed** test #%d (%s)\n", testNo, testName);
	fprintf(stderr, "    Unable to find function %s\n", replacement);
	exit(1);
      }
      call_replacement = bpfv[0];
    }

    for (unsigned int n = 0; n < points->size(); n++) {
	BPatch_function *func;

	if ((func = (*points)[n]->getCalledFunction()) == NULL) continue;

	char fn[256];
	if (func->getName(fn, 256) == NULL) {
	    fprintf(stderr, "**Failed** test #%d (%s)\n", testNo, testName);
	    fprintf(stderr, "    Can't get name of called function in %s\n",
		    inFunction);
            return -1;
	}
	if (functionNameMatch(fn, callTo) == 0) {
	    if (replacement == NULL)
		appThread->removeFunctionCall(*((*points)[n]));
	    else {
                assert(call_replacement);
		appThread->replaceFunctionCall(*((*points)[n]),
					       *call_replacement);
            }
	    numReplaced++;
	}
    }

    if (callsExpected > 0 && callsExpected != numReplaced) {
	fprintf(stderr, "**Failed** test #%d (%s)\n", testNo, testName);
	fprintf(stderr, "    Expected to find %d %s to %s in %s, found %d\n",
		callsExpected, callsExpected == 1 ? "call" : "calls",
		callTo, inFunction, numReplaced);
        return -1;
    }


    return numReplaced;
}

//
// Return a pointer to a string identifying a BPatch_procedureLocation
//
const char *locationName(BPatch_procedureLocation l)
{
    switch(l) {
      case BPatch_entry:
	return "entry";
      case BPatch_exit:
	return "exit";
      case BPatch_subroutine:
	return "call points";
      case BPatch_longJump:
	return "long jump";
      case BPatch_allLocations:
	return "all";
      default:
	return "<invalid BPatch_procedureLocation>";
    };
}

//
// Insert "snippet" at the location "loc" in the function "inFunction."
// Returns the value returned by BPatch_thread::insertSnippet.
//
BPatchSnippetHandle *insertSnippetAt(BPatch_thread *appThread,
                               BPatch_image *appImage, const char *inFunction, 
                               BPatch_procedureLocation loc, 
                               BPatch_snippet &snippet,
                               int testNo, const char *testName)
{
    // Find the point(s) we'll be instrumenting

    BPatch_Vector<BPatch_function *> found_funcs;
    if ((NULL == appImage->findFunction(inFunction, found_funcs)) || !found_funcs.size()) {
      fprintf(stderr, "**Failed** test #%d (%s)\n", testNo, testName);
      fprintf(stderr, "    Unable to find function %s\n",
	      inFunction);
      return NULL;
    }

    if (1 < found_funcs.size()) {
      fprintf(stderr, "%s[%d]:  WARNING  : found %d functions named %s.  Using the first.\n", 
	      __FILE__, __LINE__, found_funcs.size(), inFunction);
    }

    BPatch_Vector<BPatch_point *> *points = found_funcs[0]->findPoint(loc);

    if (!points) {
	fprintf(stderr, "**Failed** test #%d (%s)\n", testNo, testName);
	fprintf(stderr, "    Unable to find point %s - %s\n",
		inFunction, locationName(loc));
        return NULL;
    }

    checkCost(snippet);
    return appThread->insertSnippet(snippet, *points);
}

//
// Create a snippet that calls the function "funcName" with no arguments
//
BPatch_snippet *makeCallSnippet(BPatch_image *appImage, const char *funcName,
                                int testNo, const char *testName)
{
  BPatch_Vector<BPatch_function *> bpfv;
  if (NULL == appImage->findFunction(funcName, bpfv) || !bpfv.size()
      || NULL == bpfv[0]){
    fprintf(stderr, "**Failed** test #%d (%s)\n", testNo, testName);
    fprintf(stderr, "    Unable to find function %s\n", funcName);
    return NULL;
  }
  BPatch_function *call_func = bpfv[0];
 
    BPatch_Vector<BPatch_snippet *> nullArgs;
    BPatch_snippet *ret = new BPatch_funcCallExpr(*call_func, nullArgs);

    if (ret == NULL) {
	fprintf(stderr, "**Failed** test #%d (%s)\n", testNo, testName);
	fprintf(stderr, "    Unable to create snippet to call %s\n", funcName);
        return NULL;
    }

    return ret;
}


//
// Insert a snippet to call function "funcName" with no arguments into the
// procedure "inFunction" at the points given by "loc."
//
int insertCallSnippetAt(BPatch_thread *appThread,
                            BPatch_image *appImage, const char *inFunction,
                            BPatch_procedureLocation loc, const char *funcName,
                            int testNo, const char *testName)
{
    BPatch_snippet *call_expr =
       makeCallSnippet(appImage, funcName, testNo, testName);
    RETURNONNULL(call_expr);

    BPatchSnippetHandle *ret = insertSnippetAt(appThread, appImage,
					       inFunction, loc, *call_expr,
					       testNo, testName);
    if (ret == NULL) {
	fprintf(stderr, "**Failed** test #%d (%s)\n", testNo, testName);
	fprintf(stderr, "    Unable to insert snippet to call function %s\n",
		funcName);
        return -1;
    }

    delete call_expr;

    return 0;
}

BPatch_Vector<BPatch_snippet *> genLongExpr(BPatch_arithExpr *tail)
{
    BPatch_Vector<BPatch_snippet *> *ret;
    
    ret = new(BPatch_Vector<BPatch_snippet *>);
    for (int i=0; i < 1000; i++) {
	ret->push_back(tail);
    }
    return *ret;
}

// Build Architecture specific libname
void addLibArchExt(char *dest, unsigned int dest_max_len)
{
   int dest_len;

   dest_len = strlen(dest);

   // Patch up alternate ABI filenames

#if defined(mips_sgi_irix6_4)
   strncat(dest,"_n32", dest_max_len - dest_len);
   dest_len += 3;
#endif

#if defined(i386_unknown_nt4_0)
   strncat(dest, ".dll", dest_max_len - dest_len);
   dest_len += 4;
#else
   strncat(dest, ".so", dest_max_len - dest_len);
   dest_len += 3;
#endif
}

int readyTest21or22(BPatch_thread *appThread, char *libNameA, char *libNameB)
{
    char libA[128], libB[128];
    snprintf(libA, 128, "./%s", libNameA);
    snprintf(libB, 128, "./%s", libNameB);
#if !defined(i386_unknown_nt4_0)
    if (!mutateeFortran) {
	if (! appThread->loadLibrary(libA)) {
	     fprintf(stderr, "**Failed test #21 (findFunction in module)\n");
	     fprintf(stderr, "  Mutator couldn't load %s into mutatee\n", libNameA);
             return -1;
	}
	if (! appThread->loadLibrary(libB)) {
	     fprintf(stderr, "**Failed test #21 (findFunction in module)\n");
	     fprintf(stderr, "  Mutator couldn't load %s into mutatee\n", libNameB);
             return -1;
	}
    }
#endif
    return 0;
}

typedef BPatch_Vector<BPatch_point * > point_vector;
// typedef vector<BPatchSnippetHandle * > handle_vector;

void instrument_entry_points( BPatch_thread * app_thread,
			      BPatch_image * ,
			      BPatch_function * func,
			      BPatch_snippet * code )
{
  assert( func != 0 );
  assert( code != 0 );

//   handle_vector * list_of_handles = new handle_vector;

  int null_entry_point_count = 0;
  int failed_snippet_insertion_count = 0;

  point_vector * entries = func->findPoint( BPatch_entry );
  assert( entries != 0 );

  for( unsigned int i = 0; i < entries->size(); i++ )
    {
      BPatch_point * point = ( * entries )[ i ];
      if( point == 0 )
	{
	  null_entry_point_count++;
	}
      else
	{
	  BPatchSnippetHandle * result =
	    app_thread->insertSnippet( * code,
				       * point, BPatch_callBefore, BPatch_firstSnippet );
	  if( result == 0 )
	    {
	      failed_snippet_insertion_count++;
	    }
// 	  else
// 	    {
// 	      list_of_handles->push_back( result );
// 	    }
	}
    }

  delete code;

//   return * list_of_handles;
}


void instrument_exit_points( BPatch_thread * app_thread,
			     BPatch_image * ,
			     BPatch_function * func,
			     BPatch_snippet * code )
{
  assert( func != 0 );
  assert( code != 0 );

//   handle_vector * list_of_handles = new handle_vector;

  int null_exit_point_count = 0;
  int failed_snippet_insertion_count = 0;

  point_vector * exits = func->findPoint( BPatch_exit );
  assert( exits != 0 );

  for( unsigned int i = 0; i < exits->size(); i++ )
    {
      BPatch_point * point = ( * exits )[ i ];
      if( point == 0 )
	{
	  null_exit_point_count++;
	}
      else
	{
	  BPatchSnippetHandle * result =
	    app_thread->insertSnippet( * code,
				       * point, BPatch_callAfter, BPatch_firstSnippet );
	  if( result == 0 )
	    {
	      failed_snippet_insertion_count++;
	    }
// 	  else
// 	    {
// 	      list_of_handles->push_back( result );
// 	    }
	}
    }

  delete code;

//   return * list_of_handles;
}

// NOTE: What are the benefits of this over appThread->terminateProcess?
void killMutatee(BPatch_thread *appThread)
{
    int pid = appThread->getPid();

#ifndef i386_unknown_nt4_0 /* Not yet implemented on NT. */
    dprintf("Detaching from process %d (leaving it running).\n", pid);
    appThread->detach(true);
#else
    printf("[Process detach not yet implemented.]\n");
#endif

    // now kill the process.
#ifdef i386_unknown_nt4_0
    HANDLE h = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);
    if (h != NULL) {
        dprintf("Killing mutatee process %d.\n", pid);
	TerminateProcess(h, 0);
	CloseHandle(h);
    }
#else
    int kret;

    // Alpha seems to take two kills to work - jkh 3/13/00
    while (1) {
        //dprintf("Killing mutatee process %d.\n", pid);
	kret = kill(pid, SIGKILL);
	if (kret) {
	    if (errno == ESRCH) {
	       break;
	    } else {
	       perror("kill");
	       break;
	    }
	}
	kret = waitpid(pid, NULL, WNOHANG);
	if (kret == pid) break;
    }
#endif
    dprintf("Mutatee process %d killed.\n", pid);
}

// Tests to see if the mutatee has defined the mutateeCplusplus flag
int isMutateeCxx(BPatch_image *appImage) {
    // determine whether mutatee is C or C++
    BPatch_variableExpr *isCxx = appImage->findVariable("mutateeCplusplus");
    if (isCxx == NULL) {
	dprintf("  Unable to locate variable \"mutateeCplusplus\""
                 " -- assuming 0!\n");
        return 0;
    } else {
        int mutateeCplusplus;
        isCxx->readValue(&mutateeCplusplus);
        dprintf("Mutatee is %s.\n", mutateeCplusplus ? "C++" : "C");
        return mutateeCplusplus;
    }
}
// Tests to see if the mutatee has defined the mutateeFortran flag
int isMutateeFortran(BPatch_image *appImage) {
    // determine whether mutatee is Fortran
    BPatch_variableExpr *isF = appImage->findVariable("mutateeFortran");
    if (isF == NULL) {
	dprintf("  Unable to locate variable \"mutateeFortran\""
                 " -- assuming 0!\n");
        return 0;
    } else {
        int mutateeFortran;
        isF->readValue(&mutateeFortran);
        dprintf("Mutatee is %s.\n", mutateeFortran ? "Fortran" : "C/C++");
        return mutateeFortran;
    }

}

// Tests to see if the mutatee has defined the mutateeF77 flag
int isMutateeF77(BPatch_image *appImage) {
    // determine whether mutatee is F77
    BPatch_variableExpr *isF77 = appImage->findVariable("mutateeF77");
    if (isF77 == NULL) {
	dprintf("  Unable to locate variable \"mutateeF77\""
                 " -- assuming 0!\n");
        return 0;
    } else {
        int mutateeF77;
        isF77->readValue(&mutateeF77);
        dprintf("Mutatee is %s.\n", mutateeF77 ? "F77" : "not F77");
        return mutateeF77;
    }
}


void MopUpMutatees(const unsigned int mutatees, BPatch_thread *appThread[])
{
    unsigned int n=0;
    dprintf("MopUpMutatees(%d)\n", mutatees);
    for (n=0; n<mutatees; n++) {
        if (appThread[n]) {
            if (appThread[n]->terminateExecution()) {
                assert(appThread[n]->terminationStatus() == ExitedViaSignal);
                int signalNum = appThread[n]->getExitSignal();
                dprintf("Mutatee terminated from signal 0x%x\n", signalNum);
            } else {
                printf("Failed to mop up mutatee %d (pid=%d)!\n",
                        n, appThread[n]->getPid());
            }
        } else {
            printf("Mutatee %d already terminated?\n", n);
        }
    }
    dprintf("MopUpMutatees(%d) done\n", mutatees);
}

void contAndWaitForAllThreads(BPatch *bpatch, BPatch_thread *appThread, 
      BPatch_thread **mythreads, int *threadCount)
{

  dprintf("Thread %d is pointer %p\n", *threadCount, appThread);
  mythreads[(*threadCount)++] = appThread;
   appThread->continueExecution();

   while (1) {
      int i;
      dprintf("Checking %d threads for terminated status\n", *threadCount);
      for (i=0; i < *threadCount; i++) {
	if (!mythreads[i]->isTerminated()) {
	  dprintf("Thread %d is not terminated\n", i);
            break;
         }
      }

      // see if all exited
      if (i== *threadCount) {
	dprintf("All threads terminated\n");
	break;
      }

      bpatch->waitForStatusChange();

      for (i=0; i < *threadCount; i++) {
         if (mythreads[i]->isStopped()) {
	   dprintf("Thread %d marked stopped, continuing\n", i);
            mythreads[i]->continueExecution();
         }
      }
   }
   
   dprintf("All threads terminated, deleting\n");
   for (int i=0; i < *threadCount; i++) {
     delete mythreads[i];
   }
   *threadCount = 0;
}

/*
 * Given a string variable name and an expected value, lookup the varaible
 *    in the child process, and verify that the value matches.
 *
 */
bool verifyChildMemory(BPatch_thread *appThread, 
                       const char *name, int expectedVal)
{
     BPatch_image *appImage = appThread->getImage();

     if (!appImage) {
	 dprintf("unable to locate image for %d\n", appThread->getPid());
	 return false;
     }

     BPatch_variableExpr *var = appImage->findVariable(name);
     if (!var) {
	 dprintf("unable to located variable %s in child\n", name);
	 return false;
     }

     int actualVal;
     var->readValue(&actualVal);

     if (expectedVal != actualVal) {
	 printf("*** for %s, expected val = %d, but actual was %d\n",
		name, expectedVal, actualVal);
	 return false;
     } else {
	 dprintf("verified %s was = %d\n", name, actualVal);
	 return true;
     }
}


void dumpvect(BPatch_Vector<BPatch_point*>* res, const char* msg)
{
  if(!debugPrint)
    return;

  printf("%s: %d\n", msg, res->size());
  for(unsigned int i=0; i<res->size(); ++i) {
    BPatch_point *bpp = (*res)[i];
    const BPatch_memoryAccess* ma = bpp->getMemoryAccess();
    const BPatch_addrSpec_NP& as = ma->getStartAddr_NP();
    const BPatch_countSpec_NP& cs = ma->getByteCount_NP();
    if(ma->getNumberOfAccesses() == 1) {
      if(ma->isConditional_NP())
        printf("%s[%d]: @[r%d+r%d<<%d+%d] #[r%d+r%d+%d] ?[%X]\n", msg, i+1,
               as.getReg(0), as.getReg(1), as.getScale(), as.getImm(),
               cs.getReg(0), cs.getReg(1), cs.getImm(), ma->conditionCode_NP());
        else
          printf("%s[%d]: @[r%d+r%d<<%d+%d] #[r%d+r%d+%d]\n", msg, i+1,
                 as.getReg(0), as.getReg(1), as.getScale(), as.getImm(),
                 cs.getReg(0), cs.getReg(1), cs.getImm());
    }
    else {
      const BPatch_addrSpec_NP& as2 = ma->getStartAddr_NP(1);
      const BPatch_countSpec_NP& cs2 = ma->getByteCount_NP(1);
      printf("%s[%d]: @[r%d+r%d<<%d+%d] #[r%d+r%d+%d] && "
             "@[r%d+r%d<<%d+%d] #[r%d+r%d+%d]\n", msg, i+1,
             as.getReg(0), as.getReg(1), as.getScale(), as.getImm(),
             cs.getReg(0), cs.getReg(1), cs.getImm(),
             as2.getReg(0), as2.getReg(1), as2.getScale(), as2.getImm(),
             cs2.getReg(0), cs2.getReg(1), cs2.getImm());
    }
  }
}

static inline void dumpxpct(BPatch_memoryAccess* exp[], unsigned int size, const char* msg)
{
  if(!debugPrint)
    return;
           
  printf("%s: %d\n", msg, size);

  for(unsigned int i=0; i<size; ++i) {
    const BPatch_memoryAccess* ma = exp[i];
    if(!ma)
      continue;
    const BPatch_addrSpec_NP& as = ma->getStartAddr_NP();
    const BPatch_countSpec_NP& cs = ma->getByteCount_NP();
    if(ma->getNumberOfAccesses() == 1)
      printf("%s[%d]: @[r%d+r%d<<%d+%d] #[r%d+r%d+%d]\n", msg, i+1,
             as.getReg(0), as.getReg(1), as.getScale(), as.getImm(),
             cs.getReg(0), cs.getReg(1), cs.getImm());
    else {
      const BPatch_addrSpec_NP& as2 = ma->getStartAddr_NP(1);
      const BPatch_countSpec_NP& cs2 = ma->getByteCount_NP(1);
      printf("%s[%d]: @[r%d+r%d<<%d+%d] #[r%d+r%d+%d] && "
             "@[r%d+r%d<<%d+%d] #[r%d+r%d+%d]\n", msg, i+1,
             as.getReg(0), as.getReg(1), as.getScale(), as.getImm(),
             cs.getReg(0), cs.getReg(1), cs.getImm(),
             as2.getReg(0), as2.getReg(1), as2.getScale(), as2.getImm(),
             cs2.getReg(0), cs2.getReg(1), cs2.getImm());
    }
  }
}

bool validate(BPatch_Vector<BPatch_point*>* res,
                            BPatch_memoryAccess* acc[], const char* msg)
{
  bool ok = true;

  for(unsigned int i=0; i<res->size(); ++i) {
    BPatch_point* bpoint = (*res)[i];
    ok = (ok && bpoint->getMemoryAccess()->equals(acc[i]));
    if(!ok) {
      printf("Validation failed at %s #%d.\n", msg, i+1);
      dumpxpct(acc, res->size(), "Expected");
      return ok;
    }
  }
  return ok;
}

BPatch_callWhen instrumentWhere(  const BPatch_memoryAccess* memAccess){

	BPatch_callWhen whenToCall;
	if(memAccess != NULL){
		if(memAccess->hasALoad()){
			whenToCall = BPatch_callBefore;
		}else if(memAccess->hasAStore()){
			whenToCall = BPatch_callAfter;
		}else if(memAccess->hasAPrefetch_NP() ){
			whenToCall = BPatch_callBefore;
		}else{
			whenToCall = BPatch_callBefore;
		}
	}else{
		whenToCall = BPatch_callBefore;
	}
	return whenToCall;
}

int instCall(BPatch_thread* bpthr, const char* fname,
              const BPatch_Vector<BPatch_point*>* res)
{
  char buf[30];
	BPatch_callWhen whenToCall = BPatch_callBefore;

  snprintf(buf, 30, "count%s", fname);

  BPatch_Vector<BPatch_snippet*> callArgs;
  BPatch_image *appImage = bpthr->getImage();

  BPatch_Vector<BPatch_function *> bpfv;
  if (NULL == appImage->findFunction(buf, bpfv) || !bpfv.size()
      || NULL == bpfv[0]){
    fprintf(stderr, "    Unable to find function %s\n", buf);
    return -1;
  }
  BPatch_function *countXXXFunc = bpfv[0];  

  BPatch_funcCallExpr countXXXCall(*countXXXFunc, callArgs);

  for(unsigned int i=0;i<(*res).size();i++){

#if defined(rs6000_ibm_aix4_1) && defined(AIX5)
  	const BPatch_memoryAccess* memAccess;
	memAccess = (*res)[i]->getMemoryAccess() ;

	whenToCall = instrumentWhere( memAccess);

#endif
	bpthr->insertSnippet(countXXXCall, *((*res)[i]),whenToCall);
  }

  return 0;
}

int instEffAddr(BPatch_thread* bpthr, const char* fname,
		 const BPatch_Vector<BPatch_point*>* res,
                 bool conditional)
{
  char buf[30];
  snprintf(buf, 30, "list%s%s", fname, (conditional ? "CC" : ""));
  dprintf("CALLING: %s\n", buf);

  BPatch_Vector<BPatch_snippet*> listArgs;
  BPatch_effectiveAddressExpr eae;
  listArgs.push_back(&eae);

  BPatch_image *appImage = bpthr->getImage();

  BPatch_Vector<BPatch_function *> bpfv;
  if (NULL == appImage->findFunction(buf, bpfv) || !bpfv.size()
      || NULL == bpfv[0]){
    fprintf(stderr, "    Unable to find function %s\n", buf);
    return -1;
  }
  BPatch_function *listXXXFunc = bpfv[0];  
  BPatch_funcCallExpr listXXXCall(*listXXXFunc, listArgs);


  BPatch_callWhen whenToCall = BPatch_callBefore;
  for(unsigned int i=0;i<(*res).size();i++){
#if defined(rs6000_ibm_aix4_1) && defined(AIX5) 
  	const BPatch_memoryAccess* memAccess;

	memAccess = (*res)[i]->getMemoryAccess() ;

	whenToCall = instrumentWhere( memAccess);
#endif
  	if(!conditional)
	    bpthr->insertSnippet(listXXXCall, *((*res)[i]), whenToCall, BPatch_lastSnippet);
	  else {
	    BPatch_ifMachineConditionExpr listXXXCallCC(listXXXCall);
	    bpthr->insertSnippet(listXXXCallCC, *((*res)[i]), whenToCall, BPatch_lastSnippet);
	  }
  }

#if defined(i386_unknown_linux2_0) \
 || defined(x86_64_unknown_linux2_4) /* Blind duplication - Ray */ \
 || defined(i386_unknown_nt4_0)
  BPatch_effectiveAddressExpr eae2(1);
  BPatch_Vector<BPatch_snippet*> listArgs2;
  listArgs2.push_back(&eae2);

  BPatch_funcCallExpr listXXXCall2(*listXXXFunc, listArgs2);
  
  const BPatch_Vector<BPatch_point*>* res2 = BPatch_memoryAccess::filterPoints(*res, 2);

  if(!conditional)
    bpthr->insertSnippet(listXXXCall2, *res2, BPatch_lastSnippet);
  else {
    BPatch_ifMachineConditionExpr listXXXCallCC2(listXXXCall2);
    bpthr->insertSnippet(listXXXCallCC2, *res2, BPatch_lastSnippet);    
  }
#endif

  return 0;
}


int instByteCnt(BPatch_thread* bpthr, const char* fname,
		 const BPatch_Vector<BPatch_point*>* res,
                 bool conditional)
{
  char buf[30];
  snprintf(buf, 30, "list%s%s", fname, (conditional ? "CC" : ""));
  dprintf("CALLING: %s\n", buf);

  BPatch_Vector<BPatch_snippet*> listArgs;
  BPatch_bytesAccessedExpr bae;
  listArgs.push_back(&bae);

  BPatch_image *appImage = bpthr->getImage();

  BPatch_Vector<BPatch_function *> bpfv;
  if (NULL == appImage->findFunction(buf, bpfv) || !bpfv.size()
      || NULL == bpfv[0]){
    fprintf(stderr, "    Unable to find function %s\n", buf);
    return -1;
  }
  BPatch_function *listXXXFunc = bpfv[0];  

  BPatch_callWhen whenToCall = BPatch_callBefore;

  for(unsigned int i=0;i<(*res).size();i++){

#if defined(rs6000_ibm_aix4_1) && defined(AIX5)
  	const BPatch_memoryAccess* memAccess;
	memAccess = (*res)[i]->getMemoryAccess() ;

	whenToCall = instrumentWhere( memAccess);

#endif
  	BPatch_funcCallExpr listXXXCall(*listXXXFunc, listArgs);
	  if(!conditional)
	    bpthr->insertSnippet(listXXXCall, *((*res)[i]), whenToCall, BPatch_lastSnippet);
	  else {
	    BPatch_ifMachineConditionExpr listXXXCallCC(listXXXCall);
	    bpthr->insertSnippet(listXXXCallCC, *((*res)[i]), whenToCall, BPatch_lastSnippet);
	  }
  }

#if defined(i386_unknown_linux2_0) \
 || defined(x86_64_unknown_linux2_4) /* Blind duplication - Ray */ \
 || defined(i386_unknown_nt4_0)
  BPatch_bytesAccessedExpr bae2(1);
  BPatch_Vector<BPatch_snippet*> listArgs2;
  listArgs2.push_back(&bae2);

  BPatch_funcCallExpr listXXXCall2(*listXXXFunc, listArgs2);
  
  const BPatch_Vector<BPatch_point*>* res2 = BPatch_memoryAccess::filterPoints(*res, 2);
  if(!conditional)
    bpthr->insertSnippet(listXXXCall2, *res2, BPatch_lastSnippet);
  else {
    BPatch_ifMachineConditionExpr listXXXCallCC2(listXXXCall2);
    bpthr->insertSnippet(listXXXCallCC2, *res2, BPatch_lastSnippet);
  }
#endif

  return 0;
}

// From Test8
const char *frameTypeString(BPatch_frameType frameType)
{
    switch (frameType) {
      case BPatch_frameNormal:
	return "BPatch_frameNormal";
      case BPatch_frameSignal:
	return "BPatch_frameSignal";
      case BPatch_frameTrampoline:
	return "BPatch_frameTrampoline";
      default:
	break;
    };

    return "UNKNOWN";
}

bool hasExtraUnderscores(const char *str)
{
    assert( str );
    int len = strlen(str) - 1;
    return (str[0] == '_' || str[len] == '_');
}

/* WARNING: This function is not thread safe. */
const char *fixUnderscores(const char *str)
{
    static char buf[256];

    assert( str );
    assert( strlen(str) < sizeof(buf) );

    while (*str == '_') ++str;
    strncpy(buf, str, 256);

    char *ptr = buf + strlen(buf) - 1;
    while (ptr > buf && *ptr == '_') *(ptr--) = 0;

    return buf;
}


bool checkStack(BPatch_thread *appThread,
		const frameInfo_t correct_frame_info[],
		unsigned num_correct_names,
		int test_num, const char *test_name)
{
    unsigned i, j;

    const int name_max = 256;
    bool failed = false;

    BPatch_Vector<BPatch_frame> stack;
    appThread->getCallStack(stack);

    if (debugPrint) {
	printf("Stack in test %d (%s):\n", test_num, test_name);
	for( unsigned i = 0; i < stack.size(); i++) {
	    char name[name_max];
	    BPatch_function *func = stack[i].findFunction();
	    if (func == NULL)
		strcpy(name, "[UNKNOWN]");
	    else
		func->getName(name, name_max);
	    printf("  %10p: %s, fp = %p, type %s\n",
               stack[i].getPC(),
               name,
               stack[i].getFP(),
               frameTypeString(stack[i].getFrameType()));
        
	}
	printf("End of stack dump.\n");
    }

    if (stack.size() < num_correct_names) {
	fprintf(stderr, "**Failed** test %d (%s)\n", test_num, test_name);
	fprintf(stderr, "    Stack trace should contain more frames.\n");
	failed = true;
    }

    for (i = 0, j = 0; i < num_correct_names; i++, j++) {
#if !defined(i386_unknown_nt4_0)
	if (j < stack.size()-1 && stack[j].getFP() == 0) {
	    fprintf(stderr, "**Failed** test %d (%s)\n", test_num, test_name);
	    fprintf(stderr, "    A stack frame other than the lowest has a null FP.\n");
	    failed = true;
	    break;
	}
#endif

	if (correct_frame_info[i].valid) {
	    char name[name_max], name2[name_max];

	    BPatch_function *func = stack[j].findFunction();
	    if (func != NULL)
		func->getName(name, name_max);

	    BPatch_function *func2 =
		appThread->findFunctionByAddr(stack[j].getPC());
	    if (func2 != NULL)
		func2->getName(name2, name_max);

	    if ((func == NULL && func2 != NULL) ||
		(func != NULL && func2 == NULL)) {
		fprintf(stderr, "**Failed** test %d (%s)\n", test_num, test_name);
		fprintf(stderr, "    frame->findFunction() disagrees with thread->findFunctionByAddr()\n");
		fprintf(stderr, "    frame->findFunction() returns %s\n",
			name);
		fprintf(stderr, "    thread->findFunctionByAddr() return %s\n",
			name2);
		failed = true;
		break;
	    } else if (func!=NULL && func2!=NULL && strcmp(name, name2)!=0) {
		fprintf(stderr, "**Failed** test %d (%s)\n", test_num, test_name);
		fprintf(stderr, "    BPatch_frame::findFunction disagrees with BPatch_thread::findFunctionByAddr\n");
		failed = true;
		break;
	    }

	    if (correct_frame_info[i].type != stack[j].getFrameType()) {
		fprintf(stderr, "**Failed** test %d (%s)\n", test_num, test_name);
		fprintf(stderr, "    Stack frame #%d has wrong type, is %s, should be %s\n", i+1, frameTypeString(stack[i].getFrameType()), frameTypeString(correct_frame_info[i].type));
		fprintf(stderr, "    Stack frame 0x%lx, 0x%lx\n", stack[i].getPC(), stack[i].getFP() );
		failed = true;
		break;
	    }

	    if (stack[j].getFrameType() == BPatch_frameSignal ||
		stack[j].getFrameType() == BPatch_frameTrampoline) {
		// No further checking for these types right now
	    } else {
		if (func == NULL) {
		    fprintf(stderr, "**Failed** test %d (%s)\n",
			    test_num, test_name);
		    fprintf(stderr, "    Stack frame #%d refers to an unknown function, should refer to %s\n", j+1, correct_frame_info[i].function_name);
		    failed = true;
		    break;
		} else { /* func != NULL */
		    if (!hasExtraUnderscores(correct_frame_info[i].function_name))
			strncpy(name, fixUnderscores(name), name_max);

		    if (strcmp(name, correct_frame_info[i].function_name) != 0) {
		        if (correct_frame_info[i].optional) {
			    j--;
                            continue;
			}
			fprintf(stderr, "**Failed** test %d (%s)\n", test_num, test_name);
			fprintf(stderr, "    Stack frame #%d refers to function %s, should be %s\n", j+1, name, correct_frame_info[i].function_name);
			failed = true;
			break;
		    }
		}
	    }
	}
    }

    return !failed;
}

/* End Test8 Specific */

/* Begin Test9 Specific */
void buildArgs(const char** child_argv, char *pathname, int testNo){
	int n=0;

	child_argv[n++] = pathname;
	if (debugPrint){
		child_argv[n++] = const_cast<char*>("-verbose");
	}
	child_argv[n++] = const_cast<char*>("-orig"); 

        child_argv[n++] = const_cast<char*>("-run");
       	char str[5];
       	sprintf(str, "%d",testNo);
       	child_argv[n++] = strdup(str);
	
	child_argv[n] = NULL;

}


void createNewProcess(BPatch *bpatch, BPatch_thread *&appThread, BPatch_image *&appImage, 
      char *pathname, const char** child_argv)
{


    appThread = bpatch->createProcess(pathname, child_argv,NULL);

    if (appThread == NULL) {
	fprintf(stderr, "Unable to run test program.\n");
	exit(1);
    }
    appThread->enableDumpPatchedImage();

    // Read the program's image and get an associated image object
    appImage = appThread->getImage();

}


int instrumentToCallZeroArg(BPatch_thread *appThread, BPatch_image *appImage, char *instrumentee, char*patch, int testNo, char *testName){

  BPatch_Vector<BPatch_function *> found_funcs;
  if ((NULL == appImage->findFunction(instrumentee, found_funcs)) || !found_funcs.size()) {
    fprintf(stderr, "    Unable to find function %s\n","instrumentee");
    return -1;
  }
  
  if (1 < found_funcs.size()) {
    fprintf(stderr, "%s[%d]:  WARNING  : found %d functions named %s.  Using the first.\n", 
	    __FILE__, __LINE__, found_funcs.size(), instrumentee);
  }
  
  BPatch_Vector<BPatch_point *> *point1_1 = found_funcs[0]->findPoint(BPatch_entry);


  if (!point1_1 || ((*point1_1).size() == 0)) {
    fprintf(stderr, "**Failed** test #%d (%s)\n", testNo,testName);
    fprintf(stderr, "    Unable to find entry point to \"%s.\"\n",instrumentee);
    return -1;
  }

  BPatch_Vector<BPatch_function *> bpfv;
  if (NULL == appImage->findFunction(patch, bpfv) || !bpfv.size()
      || NULL == bpfv[0]){
    fprintf(stderr, "**Failed** test #%d (%s)\n", testNo, testName);
    fprintf(stderr, "    Unable to find function %s\n", patch);
    return -1;
  }
  BPatch_function *call1_func = bpfv[0];
  
  BPatch_Vector<BPatch_snippet *> call1_args;
  BPatch_funcCallExpr call1Expr(*call1_func, call1_args);
  
  dprintf("Inserted snippet2\n");
  checkCost(call1Expr);
  appThread->insertSnippet(call1Expr, *point1_1);

  return 0;
}

char* saveWorld(BPatch_thread *appThread){

	char *mutatedName = new char[strlen("test9_mutated") +1];
	memset(mutatedName, '\0',strlen("test9_mutated") +1);
	strcat(mutatedName, "test9_mutated");
	   char* dirName = appThread->dumpPatchedImage(mutatedName);
	if(!dirName){
		fprintf(stderr,"Error: No directory name returned\n");
	}

	return dirName;
}

int letOriginalMutateeFinish(BPatch_thread *appThread){
	/* finish original mutatee to see if it runs */
	
	/*fprintf(stderr,"\n************************\n");	
	fprintf(stderr,"Running the original mutatee\n\n");*/
	appThread->continueExecution();

	while( !appThread->isTerminated());

	int retVal;

	if(appThread->terminationStatus() == ExitedNormally) {
		retVal = appThread->getExitCode();
	} else if(appThread->terminationStatus() == ExitedViaSignal) {
		int signalNum = appThread->getExitSignal();
		if (signalNum){
			fprintf(stderr,"Mutatee exited from signal 0x%x\n", signalNum);
		}
	       	retVal = signalNum;
    	}


//	fprintf(stderr,"Original mutatee has terminated\n\************************\n\n");

	return retVal;
}


int runMutatedBinary(char *path, char* fileName, char* testID){

   pid_t pid;
   int status, died;
 	char *mutatedBinary;

#if defined(rs6000_ibm_aix4_1) \
 || defined(rs6000_ibm_aix5_1)
	char *aixBinary="dyninst_mutatedBinary";
#endif
	char *realfileName;

	realfileName = fileName;
#if defined(rs6000_ibm_aix4_1) \
 || defined(rs6000_ibm_aix5_1)
	realfileName = aixBinary;
#endif

	mutatedBinary= new char[strlen(path) + strlen(realfileName) + 1];

	memset(mutatedBinary, '\0', strlen(path) + strlen(realfileName) + 1);

	strcat(mutatedBinary, path);
	strcat(mutatedBinary, realfileName);

	switch((pid=fork())){
		case -1: 
		fprintf(stderr,"can't fork\n");
    	    		exit(-1);
		case 0 : 
			//child
			fprintf(stderr," running: %s %s %s\n", mutatedBinary, realfileName, testID);
#if defined(rs6000_ibm_aix5_1) \
 || defined(rs6000_ibm_aix4_1)
			changePath(path);
#endif

			execl(mutatedBinary, realfileName,"-run", testID, 0); 
			fprintf(stderr,"ERROR!\n");
			perror("execl");
			exit(-1);

		default: 
			//parent
			delete [] mutatedBinary;
#if defined(sparc_sun_solaris2_4) \
 || defined(rs6000_ibm_aix4_1) \
 || defined(i386_unknown_linux2_0) \
 || defined(x86_64_unknown_linux2_4) /* Blind duplication - Ray */ \
 || defined(rs6000_ibm_aix5_1)
			died= waitpid(pid, &status, 0); 
#endif
   	}
#if defined(sparc_sun_solaris2_4) \
 || defined(rs6000_ibm_aix4_1) \
 || defined(i386_unknown_linux2_0) \
 || defined(x86_64_unknown_linux2_4) /* Blind duplication - Ray */ \
 || defined(rs6000_ibm_aix5_1)
	if(WIFEXITED(status)){
		int exitStatus = WEXITSTATUS(status);

		if(exitStatus == 0){
			return 1;
		}
	}else if(WIFSIGNALED(status)){
		fprintf(stderr," terminated with signal: %d \n", WTERMSIG(status));
	}
#endif
	return 0; 
	

}

extern char **environ;

void changePath(char *path){

	char  *newPATH;

	newPATH= new char[strlen("PWD=") + strlen(path) +  1];
	newPATH[0] = '\0';
	strcpy(newPATH, "PWD=");
	strcat(newPATH,path); 

	for(int i=0;environ[i]!= '\0';i++){

		if( strstr(environ[i], "PWD=") ){
			environ[i] = newPATH;
		}
	}

}

int runMutatedBinaryLDLIBRARYPATH(char *path, char* fileName, char* testID){

   pid_t pid;
   int status, died;

	char *mutatedBinary;
#if defined(rs6000_ibm_aix4_1) \
 || defined(rs6000_ibm_aix5_1)
	char *aixBinary="dyninst_mutatedBinary";
#endif
	char *realFileName;

	realFileName = fileName;
#if defined(rs6000_ibm_aix4_1) \
 || defined(rs6000_ibm_aix5_1)
	realFileName = aixBinary;
#endif
	char *currLDPATH, *newLDPATH;

	currLDPATH = getenv("LD_LIBRARY_PATH");
	newLDPATH= new char[strlen("LD_LIBRARY_PATH=") + strlen(currLDPATH) + strlen(":") +strlen(path) + 1];
	newLDPATH[0] = '\0';
	strcpy(newLDPATH, "LD_LIBRARY_PATH=");
	strcat(newLDPATH,path); 
	strcat(newLDPATH,":");
	strcat(newLDPATH, currLDPATH);

	mutatedBinary= new char[strlen(path) + strlen(realFileName) + 1];

	memset(mutatedBinary, '\0', strlen(path) + strlen(realFileName) + 1);

	strcat(mutatedBinary, path);
	strcat(mutatedBinary, realFileName);
	char *command = new char[strlen(mutatedBinary)+ strlen(realFileName) + strlen("-run") + strlen(testID)+10];
	sprintf(command,"%s -run %s", mutatedBinary, testID);

	int retVal =0;
	switch((pid=fork())){
		case -1: 
		fprintf(stderr,"can't fork\n");
    	    		exit(-1);
		case 0 : 
			//child
			fprintf(stderr," running: %s %s %s\n", mutatedBinary, realFileName, testID);

#if defined(rs6000_ibm_aix5_1) \
 || defined(rs6000_ibm_aix4_1)
			changePath(path);
#endif
			for(int i=0;environ[i]!= '\0';i++){

				if( strstr(environ[i], "LD_LIBRARY_PATH=") ){
					environ[i] = newLDPATH;
				}
			}
#if  defined(i386_unknown_linux2_0) \
 || defined(x86_64_unknown_linux2_4)
			struct stat buf;
			retVal = stat("/usr/bin/setarch", &buf);
			if(retVal != -1 ){
				execl("/usr/bin/setarch","setarch","i386",mutatedBinary, "-run", testID,0); 
			}else{

				execl(mutatedBinary, realFileName,"-run", testID,0); 
			}
#else

			execl(mutatedBinary, realFileName,"-run", testID,0); 
#endif
			fprintf(stderr,"ERROR!\n");
			perror("execl");
			exit(-1);

		default: 
			//parent
			delete [] command;
			delete [] mutatedBinary;
#if defined(sparc_sun_solaris2_4) \
 || defined(rs6000_ibm_aix4_1) \
 || defined(i386_unknown_linux2_0) \
 || defined(x86_64_unknown_linux2_4) /* Blind duplication - Ray */ \
 || defined(rs6000_ibm_aix5_1)
			died= waitpid(pid, &status, 0); 
#endif
   	}

#if defined(sparc_sun_solaris2_4) \
 || defined(rs6000_ibm_aix4_1) \
 || defined(i386_unknown_linux2_0) \
 || defined(x86_64_unknown_linux2_4) /* Blind duplication - Ray */ \
 || defined(rs6000_ibm_aix5_1)
	if(WIFEXITED(status)){
		int exitStatus = WEXITSTATUS(status);

		if(exitStatus == 0){
			return 1;
		}
	}else if(WIFSIGNALED(status)){
		fprintf(stderr," terminated with signal: %d \n", WTERMSIG(status));
	}
#endif
	return 0; 
}

// Begin Test12 Library functions

void sleep_ms(int ms) 
{
//#if defined(os_solaris) && (os_solaris < 9)
#ifdef NOTDEF
  if (ms < 1000) {
    usleep(ms * 1000);
  }
  else {
    sleep(ms / 1000);
    usleep((ms % 1000) * 1000);
  }
#else
  struct timespec ts,rem;
  if (ms >= 1000) {
    ts.tv_sec = (int) ms / 1000;
  }
  else
    ts.tv_sec = 0;

  ts.tv_nsec = (ms - (ts.tv_sec * 1000)) * 1000 * 1000;
  //fprintf(stderr, "%s[%d]:  sleep_ms (sec = %lu, nsec = %lu)\n",
  //        __FILE__, __LINE__, ts.tv_sec, ts.tv_nsec);

  sleep:

  if (0 != nanosleep(&ts, &rem)) {
    if (errno == EINTR) {
      fprintf(stderr, "%s[%d]:  sleep interrupted\n", __FILE__, __LINE__);
      ts.tv_sec = rem.tv_sec;
      ts.tv_nsec = rem.tv_nsec;
      goto sleep;
    }
    assert(0);
  }
#endif
}

BPatch_function *findFunction(const char *fname, BPatch_image *appImage, int testno, const char *testname)
{
  BPatch_Vector<BPatch_function *> bpfv;
  if (NULL == appImage->findFunction(fname, bpfv) || (bpfv.size() != 1)) {

      fprintf(stderr, "**Failed test #%d (%s)\n", testno, testname);
      fprintf(stderr, "  Expected 1 functions matching %s, got %d\n",
              fname, bpfv.size());
      return NULL;
  }
  return bpfv[0];
}

BPatch_function *findFunction(const char *fname, BPatch_module *inmod, int testno, const char *testname)
{
  BPatch_Vector<BPatch_function *> bpfv;
  if (NULL == inmod->findFunction(fname, bpfv) || (bpfv.size() != 1)) {

      fprintf(stderr, "**Failed test #%d (%s)\n", testno, testname);
      fprintf(stderr, "  Expected 1 functions matching %s, got %d\n",
              fname, bpfv.size());
      return NULL;
  }
  return bpfv[0];
}

// Internal Function for setVar and getVar
void dumpVars(BPatch_image *appImage)
{
  BPatch_Vector<BPatch_variableExpr *> vars;
  appImage->getVariables(vars);
  for (unsigned int i = 0; i < vars.size(); ++i) {
    fprintf(stderr, "\t%s\n", vars[i]->getName());
  }
}

void setVar(BPatch_image *appImage, const char *vname, void *addr, int testno, const char *testname)
{
   BPatch_variableExpr *v;
   void *buf = addr;
   if (NULL == (v = appImage->findVariable(vname))) {
      fprintf(stderr, "**Failed test #%d (%s)\n", testno, testname);
      fprintf(stderr, "  cannot find variable %s, avail vars:\n", vname);
      dumpVars(appImage);
         exit(1);
   }

   if (! v->writeValue(buf, sizeof(int),true)) {
      fprintf(stderr, "**Failed test #%d (%s)\n", testno, testname);
      fprintf(stderr, "  failed to write call site var to mutatee\n");
      exit(1);
   }
}

void getVar(BPatch_image *appImage, const char *vname, void *addr, int testno, const char *testname)
{
   BPatch_variableExpr *v;
   if (NULL == (v = appImage->findVariable(vname))) {
      fprintf(stderr, "**Failed test #%d (%s)\n", testno, testname);
      fprintf(stderr, "  cannot find variable %s: avail vars:\n", vname);
      dumpVars(appImage);
         exit(1);
   }

   if (! v->readValue(addr, sizeof(int))) {
      fprintf(stderr, "**Failed test #%d (%s)\n", testno, testname);
      fprintf(stderr, "  failed to read var in mutatee\n");
      exit(1);
   }
}

// End Test12 Library functions
