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

// $Id: test3.C,v 1.32 2004/03/23 19:11:27 eli Exp $
//
// libdyninst validation suite test #3
//    Author: Jeff Hollingsworth (6/18/99)
//

//  This program tests having multiple active mutatee processes.
//   
//  To run a subset of the tests, enter -run <test nums> on the command
//      line.
//
//  Naming conventions:
//      All functions, variables, etc are name funcXX_YY, exprXX_YY, etc.
//          XX is the test number
//          YY is the instance withing the test
//	    func1_2 is the second function used in test case #1.
//

#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#ifdef i386_unknown_nt4_0
#include <windows.h>
#include <winbase.h>
#define unlink _unlink
#else
#include <unistd.h>
#endif

#include "BPatch.h"
#include "BPatch_Vector.h"
#include "BPatch_thread.h"
#include "BPatch_snippet.h"
#include "test_util.h"


int debugPrint = 0; // internal "mutator" tracing
int errorPrint = 0; // external "dyninst" tracing (via errorFunc)

bool forceRelocation = false;  // Force relocation upon instrumentation

const unsigned int MAX_MUTATEES = 32;
unsigned int Mutatees=3;

bool runAllTests = true;
const unsigned int MAX_TEST = 5;
bool passedTest[MAX_TEST+1];

template class BPatch_Vector<BPatch_variableExpr*>;

BPatch *bpatch;

static const char *mutateeNameRoot = "test3.mutatee";

// control debug printf statements
void dprintf(const char *fmt, ...) {
   va_list args;
   va_start(args, fmt);

   if(debugPrint)
      vfprintf(stderr, fmt, args);

   va_end(args);

   fflush(stderr);
}

/**************************************************************************
 * Error callback
 **************************************************************************/

#define DYNINST_NO_ERROR -1

int expectError = DYNINST_NO_ERROR;

void errorFunc(BPatchErrorLevel level, int num, const char **params)
{
    if (num == 0) {
        // conditional reporting of warnings and informational messages
        if (errorPrint) {
            if (level == BPatchInfo)
              { if (errorPrint > 1) printf("%s\n", params[0]); }
            else
                printf("%s", params[0]);
        }
    } else {
        // reporting of actual errors
        char line[256];
        const char *msg = bpatch->getEnglishErrorString(num);
        bpatch->formatErrorString(line, sizeof(line), msg, params);
        
        if (num != expectError) {
            printf("Error #%d (level %d): %s\n", num, level, line);
        
            // We consider some errors fatal.
            if (num == 101) {
               exit(-1);
            }
        }
    }
}

/**************************************************************************
 * Utility functions
 **************************************************************************/

//
// Return a pointer to a pdstring identifying a BPatch_procedureLocation
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
	BPatch_image *appImage, char *inFunction, BPatch_procedureLocation loc,
	BPatch_snippet &snippet, int testNo, char *testName)
{
    // Find the point(s) we'll be instrumenting

  BPatch_Vector<BPatch_function *> found_funcs;
    if ((NULL == appImage->findFunction(inFunction, found_funcs, 1)) || !found_funcs.size()) {
      fprintf(stderr, "    Unable to find function %s\n",
	      inFunction);
      exit(1);
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
	exit(-1);
    }

    return appThread->insertSnippet(snippet, *points);
}

//
// Create a snippet that calls the function "funcName" with no arguments
//
BPatch_snippet *makeCallSnippet(BPatch_image *appImage, char *funcName,
				int testNo, char *testName)
{
  BPatch_Vector<BPatch_function *> bpfv;
  if (NULL == appImage->findFunction(funcName, bpfv) || !bpfv.size()
      || NULL == bpfv[0]){
    fprintf(stderr, "**Failed** test #%d (%s)\n", testNo, testName);
    fprintf(stderr, "    Unable to find function %s\n", funcName);
    exit(1);
  }

  BPatch_function *call_func = bpfv[0];

    BPatch_Vector<BPatch_snippet *> nullArgs;
    BPatch_snippet *ret = new BPatch_funcCallExpr(*call_func, nullArgs);

    if (ret == NULL) {
	fprintf(stderr, "**Failed** test #%d (%s)\n", testNo, testName);
	fprintf(stderr, "    Unable to create snippet to call %s\n", funcName);
	exit(1);
    }

    return ret;
}

//
// Insert a snippet to call function "funcName" with no arguments into the
// procedure "inFunction" at the points given by "loc."
//
BPatchSnippetHandle *insertCallSnippetAt(BPatch_thread *appThread,
	BPatch_image *appImage, char *inFunction, BPatch_procedureLocation loc,
	char *funcName, int testNo, char *testName)
{
    BPatch_snippet *call_expr =
	makeCallSnippet(appImage, funcName, testNo, testName);

    BPatchSnippetHandle *ret = insertSnippetAt(appThread, appImage,
					       inFunction, loc, *call_expr,
					       testNo, testName);
    if (ret == NULL) {
	fprintf(stderr, "**Failed** test #%d (%s)\n", testNo, testName);
	fprintf(stderr, "    Unable to insert snippet to call function %s\n",
		funcName);
	exit(-1);
    }

    delete call_expr;
    
    return ret;
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

/**************************************************************************
 * Tests
 **************************************************************************/

//
// Start Test Case #1 - create processes and process events from each
//     Just let them run a while, then kill them, no instrumentation added.
//
void mutatorTest1(char *pathname, BPatch *bpatch)
{
    unsigned int n=0;
    char *child_argv[5];
    child_argv[n++] = pathname;
    if (debugPrint) child_argv[n++] = const_cast<char*>("-verbose");
    child_argv[n++] = const_cast<char*>("-run");
    child_argv[n++] = const_cast<char*>("1");       // run test1 in mutatee
    child_argv[n++] = NULL;

    BPatch_thread *appThread[MAX_MUTATEES];

    for (n=0; n<MAX_MUTATEES; n++) appThread[n]=NULL;

    // Start the mutatees
    for (n=0; n<Mutatees; n++) {
        dprintf("Starting \"%s\" %d/%d\n", pathname, n, Mutatees);
        appThread[n] = bpatch->createProcess(pathname, child_argv, NULL);
        if (!appThread[n]) {
            printf("*ERROR*: unable to create handle%d for executable\n", n);
            printf("**Failed** test #1 (simultaneous multiple-process management - terminate)\n");
            MopUpMutatees(n-1,appThread);
            return;
        }
        dprintf("Mutatee %d started, pid=%d\n", n, appThread[n]->getPid());
    }

    dprintf("Letting mutatee processes run a short while (10s).\n");
    for (n=0; n<Mutatees; n++) appThread[n]->continueExecution();

    P_sleep(10);
    dprintf("Terminating mutatee processes.\n");

    unsigned int numTerminated=0;
    for (n=0; n<Mutatees; n++) {
        bool dead = appThread[n]->terminateExecution();
        if (!dead || !(appThread[n]->isTerminated())) {
            printf("**Failed** test #1 (simultaneous multiple-process management - terminate)\n");
            printf("    mutatee process [%d] was not terminated\n", n);
            continue;
        }
        if(appThread[n]->terminationStatus() != ExitedViaSignal) {
            printf("**Failed** test #1 (simultaneous multiple-process management - terminate)\n");
            printf("    mutatee process [%d] didn't get notice of termination\n", n);
            continue;
        }
        int signalNum = appThread[n]->getExitSignal();
        dprintf("Terminated mutatee [%d] from signal 0x%x\n", n, signalNum);
        numTerminated++;
    }

    if (numTerminated == Mutatees) {
	printf("Passed Test #1 (simultaneous multiple-process management - terminate)\n");
	passedTest[1] = true;
    }
}

//
// Start Test Case #2 - create processes and process events from each
//     Just let them run to finish, no instrumentation added.
//
void mutatorTest2(char *pathname, BPatch *bpatch)
{
    unsigned int n=0;
    char *child_argv[5];
    child_argv[n++] = pathname;
    if (debugPrint) child_argv[n++] = const_cast<char*>("-verbose");
    child_argv[n++] = const_cast<char*>("-run");
    child_argv[n++] = const_cast<char*>("2");	    // run test2 in mutatee
    child_argv[n++] = NULL;

    BPatch_thread *appThread[MAX_MUTATEES];

    for (n=0; n<MAX_MUTATEES; n++) appThread[n]=NULL;

    // Start the mutatees
    for (n=0; n<Mutatees; n++) {
        dprintf("Starting \"%s\" %d/%d\n", pathname, n, Mutatees);
        appThread[n] = bpatch->createProcess(pathname, child_argv, NULL);
        if (!appThread[n]) {
            printf("*ERROR*: unable to create handle%d for executable\n", n);
            printf("**Failed** test #2 (simultaneous multiple-process management - exit)\n");
            MopUpMutatees(n-1,appThread);
            return;
        }
        dprintf("Mutatee %d started, pid=%d\n", n, appThread[n]->getPid());
    }
    dprintf("Letting %d mutatee processes run.\n", Mutatees);
    for (n=0; n<Mutatees; n++) appThread[n]->continueExecution();

    unsigned int numTerminated=0;
    bool terminated[MAX_MUTATEES];
    for (n=0; n<Mutatees; n++) terminated[n]=false;

    // monitor the mutatee termination reports
    while (numTerminated < Mutatees) {
        bpatch->waitForStatusChange();
        for (n=0; n<Mutatees; n++)
            if (!terminated[n] && (appThread[n]->isTerminated())) {
                if(appThread[n]->terminationStatus() == ExitedNormally) {
                    int exitCode = appThread[n]->getExitCode();
                    if (exitCode || debugPrint)
                        dprintf("Mutatee %d exited with exit code 0x%x\n", n,
                                exitCode);
                }
                else if(appThread[n]->terminationStatus() == ExitedViaSignal) {
                    int signalNum = appThread[n]->getExitSignal();
                    if (signalNum || debugPrint)
                        dprintf("Mutatee %d exited from signal 0x%d\n", n,
                                signalNum);
                }
                terminated[n]=true;
                numTerminated++;
            }
    }

    if (numTerminated == Mutatees) {
	printf("Passed Test #2 (simultaneous multiple-process management - exit)\n");
	passedTest[2] = true;
    }
}

//
// read the result code written to the file test3.out.<pid> and return it
//
int readResult(int pid)
{
    int ret;
    FILE *fp;
    char filename[80];

    sprintf(filename, "test3.out.%d", pid);
    fp = fopen(filename, "r");
    if (!fp) {
	printf("ERROR: unable to open output file %s\n", filename);
	return -1;
    }
    fscanf(fp, "%d\n", &ret);
    fclose(fp);
    // don't need the file any longer so delete it now
    unlink(filename);

    return ret;
}

//
// Start Test Case #3 - create processes and insert different code into
//     each one.  The code sets a global variable which the mutatee then
//     writes to a file.  After all mutatees exit, the mutator reads the
//     files to verify that the correct code ran in each mutatee.
//     The first mutator should write a 1 to the file, the second a 2, etc.
//     If no code is patched into the mutatees, the value is 0xdeadbeef.
//
void mutatorTest3(char *pathname, BPatch *bpatch)
{
    unsigned int n=0;
    char *child_argv[5];
    child_argv[n++] = pathname;
    if (debugPrint) child_argv[n++] = const_cast<char*>("-verbose");
    child_argv[n++] = const_cast<char*>("-run");
    child_argv[n++] = const_cast<char*>("3");	    // run test3 in mutatee
    child_argv[n++] = NULL;

    int pid[MAX_MUTATEES];
    BPatch_thread *appThread[MAX_MUTATEES];

    for (n=0; n<MAX_MUTATEES; n++) appThread[n]=NULL;

    // Start the mutatees
    for (n=0; n<Mutatees; n++) {
        dprintf("Starting \"%s\" %d/%d\n", pathname, n, Mutatees);
        appThread[n] = bpatch->createProcess(pathname, child_argv, NULL);
        if (!appThread[n]) {
            printf("*ERROR*: unable to create handle%d for executable\n", n);
            printf("**Failed** test #3 (instrument multiple processes)\n");
            MopUpMutatees(n-1,appThread);
            return;
        }
        pid[n] = appThread[n]->getPid();
        dprintf("Mutatee %d started, pid=%d\n", n, pid[n]);
    }

    // Instrument mutatees
    for (n=0; n<Mutatees; n++) {
        dprintf("Instrumenting %d/%d\n", n, Mutatees);

        const char *Func="func3_1";
        const char *Var="test3ret";
        const char *Call="call3_1";
        BPatch_image *img = appThread[n]->getImage();

  BPatch_Vector<BPatch_function *> found_funcs;
    if ((NULL == img->findFunction(Func, found_funcs, 1)) || !found_funcs.size()) {
      fprintf(stderr, "    Unable to find function %s\n",
	      Func);
      exit(1);
    }

    if (1 < found_funcs.size()) {
      fprintf(stderr, "%s[%d]:  WARNING  : found %d functions named %s.  Using the first.\n", 
	      __FILE__, __LINE__, found_funcs.size(), Func);
    }

    BPatch_Vector<BPatch_point *> *point = found_funcs[0]->findPoint(BPatch_entry);

        if (!point || (*point).size() == 0) {
            printf("  Unable to find entry point to \"%s\".\n", Func);
            printf("**Failed** test #3 (instrument multiple processes)\n");
            MopUpMutatees(Mutatees,appThread);
            return;
        }
        BPatch_variableExpr *var = img->findVariable(Var);
        if (var == NULL) {
            printf("  Unable to find variable \"%s\".\n", Var);
            printf("**Failed** test #3 (instrument multiple processes)\n");
            MopUpMutatees(Mutatees,appThread);
            return;
        }

	BPatch_Vector<BPatch_function *> bpfv;
	if (NULL == img->findFunction(Call, bpfv) || !bpfv.size()
	    || NULL == bpfv[0]){
	  printf("  Unable to find target function \"%s\".\n", Call);
	  printf("**Failed** test #3 (instrument multiple processes)\n");
	  exit(1);
	}

	BPatch_function *callFunc = bpfv[0];

        // start with a simple snippet
        BPatch_arithExpr snip(BPatch_assign, *var, BPatch_constExpr((int)n));
        BPatchSnippetHandle *inst = appThread[n]->insertSnippet(snip, *point);
        if (inst == NULL) {
            printf("  Failed to insert simple snippet.\n");
            printf("**Failed** test #3 (instrument multiple processes)\n");
            MopUpMutatees(Mutatees,appThread);
            return;
        }

        // now add a call snippet
        BPatch_Vector<BPatch_snippet *> callArgs;
        BPatch_constExpr arg1(2); callArgs.push_back(&arg1);
        BPatch_constExpr arg2((int)n); callArgs.push_back(&arg2);
        BPatch_funcCallExpr callExpr(*callFunc, callArgs);
        BPatchSnippetHandle *call = 
                appThread[n]->insertSnippet(callExpr, *point);
        if (call == NULL) {
            printf("  Failed to insert call snippet.\n");
            printf("**Failed** test #3 (instrument multiple processes)\n");
            MopUpMutatees(Mutatees,appThread);
            return;
        }
    }

    dprintf("Letting %d mutatee processes run.\n", Mutatees);
    for (n=0; n<Mutatees; n++) appThread[n]->continueExecution();

    unsigned int numTerminated=0;
    bool terminated[MAX_MUTATEES];
    for (n=0; n<Mutatees; n++) terminated[n]=false;

    // monitor the mutatee termination reports
    while (numTerminated < Mutatees) {
	bpatch->waitForStatusChange();
        for (n=0; n<Mutatees; n++)
            if (!terminated[n] && (appThread[n]->isTerminated())) {
                if(appThread[n]->terminationStatus() == ExitedNormally) {
                    int exitCode = appThread[n]->getExitCode();
                    if (exitCode || debugPrint)
                        dprintf("Mutatee %d exited with exit code 0x%x\n", n,
                                exitCode);
                }
                else if(appThread[n]->terminationStatus() == ExitedViaSignal) {
                    int signalNum = appThread[n]->getExitSignal();
                    if (signalNum || debugPrint)
                        dprintf("Mutatee %d exited from signal 0x%d\n", n,
                                signalNum);
                }
                terminated[n]=true;
                numTerminated++;
            }
    }

    // now read the files to see if the value is what is expected
    bool allCorrect=true;
    int ret[MAX_MUTATEES];
    for (n=0; n<Mutatees; n++) {
        ret[n]=readResult(pid[n]);
        if (ret[n] != (int)n) {
            printf("    mutatee process %d produced %d, not %d\n",
                pid[n], ret[n], n);
            allCorrect=false;
        } else {
            dprintf("    mutatee process %d produced expected value %d\n", 
                pid[n], ret[n]);
        }
    }

    if (allCorrect) {
        printf("Passed Test #3 (instrument multiple processes)\n");
        passedTest[3] = true;
    } else {
        printf("**Failed** test #3 (instrument multiple processes)\n");
    }
}

//
// Start Test Case #4 - create one process, wait for it to exit.  Then 
//     create a second one and wait for it to exit.  Repeat as required.
//
void mutatorTest4(char *pathname, BPatch *bpatch)
{
    unsigned int n=0;
    char *child_argv[5];
    child_argv[n++] = pathname;
    if (debugPrint) child_argv[n++] = const_cast<char*>("-verbose");
    child_argv[n++] = const_cast<char*>("-run");
    child_argv[n++] = const_cast<char*>("2");     	// run test2 in mutatee
    child_argv[n++] = NULL;

    BPatch_thread *appThread;

    for (n=0; n<Mutatees; n++) {
        // Start the mutatee
        dprintf("Starting \"%s\" %d/%d\n", pathname, n, Mutatees);
        appThread = bpatch->createProcess(pathname, child_argv, NULL);
        if (!appThread) {
            printf("*ERROR*: unable to create handle%d for executable\n", n);
            printf("**Failed** Test #4 (sequential multiple-process management - exit)\n");
            return;
        }
        dprintf("Mutatee %d started, pid=%d\n", n, appThread->getPid());

        appThread->continueExecution();

        while (!appThread->isTerminated())
            bpatch->waitForStatusChange();

        if(appThread->terminationStatus() == ExitedNormally) {
           int exitCode = appThread->getExitCode();
           if (exitCode || debugPrint)
               dprintf("Mutatee %d exited with exit code 0x%x\n", n, exitCode);
        } else if(appThread->terminationStatus() == ExitedViaSignal) {
           int signalNum = appThread->getExitSignal();
           if (signalNum || debugPrint)
               dprintf("Mutatee %d exited from signal 0x%d\n", n, signalNum);
        }
    }

    printf("Passed Test #4 (sequential multiple-process management - exit)\n");
    passedTest[4] = true;
}


//
// Start Test Case #5 - create one process, wait for it to exit.  Then 
//     create a second one and wait for it to exit.  Repeat as required.
//     Differs from test 3 in that the mutatee processes terminate with
//     abort rather than exit.
//
void mutatorTest5(char *pathname, BPatch *bpatch)
{
    unsigned int n=0;
    char *child_argv[5];
    child_argv[n++] = pathname;
    if (debugPrint) child_argv[n++] = const_cast<char*>("-verbose");
    child_argv[n++] = const_cast<char*>("-run");
    child_argv[n++] = const_cast<char*>("5");	  // run test5 in mutatee
    child_argv[n++] = NULL;

    BPatch_thread *appThread;

    for (n=0; n<Mutatees; n++) {
        // Start the mutatee
        dprintf("Starting \"%s\" %d/%d\n", pathname, n, Mutatees);
        appThread = bpatch->createProcess(pathname, child_argv, NULL);
        if (!appThread) {
            printf("*ERROR*: unable to create handle%d for executable\n", n);
            printf("**Failed** Test #5 (sequential multiple-process management - abort)\n");
            return;
        }
        dprintf("Mutatee %d started, pid=%d\n", n, appThread->getPid());

        appThread->continueExecution();

        while (!appThread->isTerminated())
            bpatch->waitForStatusChange();

        if(appThread->terminationStatus() == ExitedNormally) {
           int exitCode = appThread->getExitCode();
           if (exitCode || debugPrint)
               dprintf("Mutatee %d exited with exit code 0x%x\n", n, exitCode);
        } else if(appThread->terminationStatus() == ExitedViaSignal) {
           int signalNum = appThread->getExitSignal();
           if (signalNum || debugPrint)
               dprintf("Mutatee %d exited from signal 0x%d\n", n, signalNum);
        }
    }

    printf("Passed Test #5 (sequential multiple-process management - abort)\n");
    passedTest[5] = true;
}

int main(unsigned int argc, char *argv[])
{
    bool N32ABI=false;
    char mutateeName[128];
    char libRTname[256];
    bool runTest[MAX_TEST+1];		// should we run a particular test

    strcpy(mutateeName,mutateeNameRoot);
    libRTname[0]='\0';

    if (!getenv("DYNINSTAPI_RT_LIB")) {
	 fprintf(stderr,"Environment variable DYNINSTAPI_RT_LIB undefined:\n"
#if defined(i386_unknown_nt4_0)
		 "    using standard search strategy for libdyninstAPI_RT.dll\n");
#else
	         "    set it to the full pathname of libdyninstAPI_RT\n");   
         exit(-1);
#endif
    } else
         strcpy((char *)libRTname, (char *)getenv("DYNINSTAPI_RT_LIB"));

    unsigned int i;
    // by default run all tests
    for (i=1; i <= MAX_TEST; i++) {
	runTest[i] = true;
	passedTest[i] = false;
    }

    for (i=1; i < argc; i++) {
        if (strncmp(argv[i], "-v+", 3) == 0)    errorPrint++;
        if (strncmp(argv[i], "-v++", 4) == 0)   errorPrint++;
	if (strncmp(argv[i], "-verbose", 2) == 0) {
	    debugPrint = 1;
	} else if (!strcmp(argv[i], "-V")) {
            fprintf (stdout, "%s\n", V_libdyninstAPI);
            if (libRTname[0]) 
                fprintf (stdout, "DYNINSTAPI_RT_LIB=%s\n", libRTname);
            fflush(stdout);
        } else if (strncmp(argv[i], "-plurality", 2) == 0) {
            Mutatees = atoi(argv[++i]);
            if (Mutatees > MAX_MUTATEES) {
                printf("Limiting plurality to maximum of %d!\n", MAX_MUTATEES);
                Mutatees = MAX_MUTATEES;
            } else if (Mutatees <= 1) {
                printf("Plurality of at least 2 required for these tests!\n");
                exit(-1);
            }
            dprintf ("%d mutatees to be used for each test.\n", Mutatees);
        } else if (!strcmp(argv[i], "-skip")) {
            unsigned int j;
            runAllTests = false;
            for (j=i+1; j < argc; j++) {
                unsigned int testId;
                if ((testId = atoi(argv[j]))) {
                    if ((testId > 0) && (testId <= MAX_TEST)) {
                        runTest[testId] = false;
                    } else {
                        printf("invalid test %d requested\n", testId);
                        exit(-1);
                    }
                } else {
                    // end of test list
                    break;
                }
            }
            i=j-1;
	} else if (!strcmp(argv[i], "-run")) {
	    unsigned int j;
            runAllTests = false;
	    for (j=0; j <= MAX_TEST; j++) runTest[j] = false;
	    for (j=i+1; j < argc; j++) { 
		unsigned int testId;
		if ((testId = atoi(argv[j]))) {
		    if ((testId > 0) && (testId <= MAX_TEST)) {
			dprintf("test %d requested\n", testId);
			runTest[testId] = true;
		    } else {
			printf("invalid test %d requested\n", testId);
			exit(-1);
		    }
		} else {
		    // end of test list
                    break;
		}
	    }
	    i=j-1;
	} else if (!strcmp(argv[i], "-mutatee")) {
	    i++;
            if (*argv[i]=='_')
                strcat(mutateeName,argv[i]);
            else
                strcpy(mutateeName,argv[i]);
#if defined(i386_unknown_nt4_0) || defined(i386_unknown_linux2_0) || defined(sparc_sun_solaris2_4) || defined(ia64_unknown_linux2_4)
	} else if (!strcmp(argv[i], "-relocate")) {
            forceRelocation = true;
#endif
#if defined(mips_sgi_irix6_4)
	} else if (!strcmp(argv[i], "-n32")) {
	    N32ABI=true;
#endif
	} else {
	    fprintf(stderr, "Usage: test3 "
		    "[-V] [-verbose] "
#if defined(mips_sgi_irix6_4)
		    "[-n32] "
#endif
                    "[-mutatee <test3.mutatee>] "
                    "[-plurality #] "
                    "[-run <test#> <test#> ...] "
                    "[-skip <test#> <test#> ...]\n");
            fprintf(stderr, "%d subtests\n", MAX_TEST);
	    exit(-1);
        }
    }

    if (!runAllTests) {
        printf("Running Tests: ");
	for (unsigned int j=1; j <= MAX_TEST; j++) {
	    if (runTest[j]) printf("%d ", j);
	}
	printf("\n");
    }

    // patch up the default compiler in mutatee name (if necessary)
    if (!strstr(mutateeName, "_"))
#if defined(i386_unknown_nt4_0)
        strcat(mutateeName,"_VC");
#else
        strcat(mutateeName,"_gcc");
#endif
    if (N32ABI || strstr(mutateeName,"_n32")) {
        // patch up file names based on alternate ABI (as necessary)
        if (!strstr(mutateeName, "_n32")) strcat(mutateeName,"_n32");
    }
    // patch up the platform-specific filename extensions
#if defined(i386_unknown_nt4_0)
    if (!strstr(mutateeName, ".exe")) strcat(mutateeName,".exe");
#endif

    // Create an instance of the BPatch library
    bpatch = new BPatch;

    // Force functions to be relocated
    if (forceRelocation) {
      bpatch->setForcedRelocation_NP(true);
    }

#if defined (sparc_sun_solaris2_4)
    // we use some unsafe type operations in the test cases.
    bpatch->setTypeChecking(false);
#endif

    // Register a callback function that prints any error messages
    bpatch->registerErrorCallback(errorFunc);

    if (runTest[1]) mutatorTest1(mutateeName, bpatch);
    if (runTest[2]) mutatorTest2(mutateeName, bpatch);
    if (runTest[3]) mutatorTest3(mutateeName, bpatch);
    if (runTest[4]) mutatorTest4(mutateeName, bpatch);
    if (runTest[5]) mutatorTest5(mutateeName, bpatch);

    unsigned int testsFailed = 0;
    for (i=1; i <= MAX_TEST; i++) {
	if (runTest[i] && !passedTest[i]) testsFailed++;
    }

    if (!testsFailed) {
	if (runAllTests) {
	    printf("All tests passed\n");
	} else {
	    printf("All requested tests passed\n");
	}
    } else {
	printf("**Failed** %d test%c\n",testsFailed,(testsFailed>1)?'s':' ');
    }

    return (testsFailed ? 127 : 0);
}
