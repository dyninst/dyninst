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

// $Id: test10.C,v 1.4 2004/04/20 01:27:55 jaw Exp $
//
// libdyninst validation suite test #10
//    Author: Jeff Hollingsworth Williams (14 aug 2003) 
//

//  This program tests the platform specific code modifications by
//	of DyninstAPI.
//	The mutatee that goes with this file is test10.mutatee.c
//	
//  Naming conventions:
//      All functions, variables, etc are name funcXX_YY, exprXX_YY, etc.
//          XX is the test number
//          YY is the instance withing the test
//	    func1_2 is the second function used in test case #1.
//

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>
#include <unistd.h>
#include <stdarg.h>

#if defined(i386_unknown_linux2_0)
#include <sys/types.h>
#include <sys/wait.h>
#endif

#include "BPatch.h"
#include "BPatch_Vector.h"
#include "BPatch_thread.h"
#include "BPatch_snippet.h"
#include "test_util.h"
#include "test1.h"

#define TEST1 "1"
#define TEST2 "2"
#define TEST3 "3"
#define TEST4 "4"
  
int debugPrint = 0; // internal "mutator" tracing
int errorPrint = 0; // external "dyninst" tracing (via errorFunc)

bool forceRelocation = true; // force relocation of functions

int mutateeCplusplus = 0;
int mutateeFortran = 0;
int mutateeF77 = 0;
bool runAllTests = true;
const unsigned int MAX_TEST = 6;
bool runTest[MAX_TEST+1];
bool passedTest[MAX_TEST+1];

template class BPatch_Vector<BPatch_variableExpr*>;
template class BPatch_Set<int>;

BPatch *bpatch;

static const char *mutateeNameRoot = "test10.mutatee";

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
	  if(num != 112)
	    printf("Error #%d (level %d): %s\n", num, level, line);
        
            // We consider some errors fatal.
            if (num == 101) {
               exit(-1);
            }
        }
    }
}

void createInstPointError(BPatchErrorLevel level, int num, const char **params)
{
    if (num != 117 && num != 118)
	errorFunc(level, num, params);
}

/**************************************************************************
 * Utility functions
 **************************************************************************/
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


void instrumentToCallZeroArg(BPatch_thread *appThread, BPatch_image *appImage, char *instrumentee, char*patch, int testNo, char *testName){

  BPatch_Vector<BPatch_function *> found_funcs;
  if ((NULL == appImage->findFunction(instrumentee, found_funcs)) || !found_funcs.size()) {
    fprintf(stderr, "    Unable to find function %s\n","instrumentee");
    exit(1);
  }
  
  if (1 < found_funcs.size()) {
    fprintf(stderr, "%s[%d]:  WARNING  : found %d functions named %s.  Using the first.\n", 
	    __FILE__, __LINE__, found_funcs.size(), instrumentee);
  }
  
  BPatch_Vector<BPatch_point *> *point1_1 = found_funcs[0]->findPoint(BPatch_entry);


  if (!point1_1 || ((*point1_1).size() == 0)) {
    fprintf(stderr, "**Failed** test #%d (%s)\n", testNo,testName);
    fprintf(stderr, "    Unable to find entry point to \"%s.\"\n",instrumentee);
    exit(1);
  }

  BPatch_Vector<BPatch_function *> bpfv;
  if (NULL == appImage->findFunction(patch, bpfv) || !bpfv.size()
      || NULL == bpfv[0]){
    fprintf(stderr, "**Failed** test #%d (%s)\n", testNo, testName);
    fprintf(stderr, "    Unable to find function %s\n", patch);
    exit(1);
  }
  BPatch_function *call1_func = bpfv[0];
  
  BPatch_Vector<BPatch_snippet *> call1_args;
  BPatch_funcCallExpr call1Expr(*call1_func, call1_args);
  
  dprintf("Inserted snippet2\n");
  appThread->insertSnippet(call1Expr, *point1_1);

}


//
// Start Test Case #1 - ()
//
void mutatorTest1(BPatch_thread *appThread, BPatch_image *appImage)
{
    instrumentToCallZeroArg(appThread, appImage, "func1", "call1", 1, "test10.1");
}

//
// Start Test Case #2 
//
void mutatorTest2(BPatch_thread *appThread, BPatch_image *appImage)
{
    instrumentToCallZeroArg(appThread, appImage, "func2", "call2", 2, "test10.2");
}


//
// Start Test Case #3 
//
void mutatorTest3(BPatch_thread *appThread, BPatch_image *appImage)
{
    instrumentToCallZeroArg(appThread, appImage, "func3", "call3", 3, "test10.3");
}

//
// Start Test Case #4 
//
void mutatorTest4(BPatch_thread *appThread, BPatch_image *appImage)
{
    instrumentToCallZeroArg(appThread, appImage, "func4", "call4", 4, "test10.4");
}

int mutatorMAIN(char *pathname)
{

    BPatch_thread *appThread;
    BPatch_image *appImage;

    // Create an instance of the BPatch library
    bpatch = new BPatch;

    // Force functions to be relocated
    bpatch->setForcedRelocation_NP(true);

    // Register a callback function that prints any error messages
    bpatch->registerErrorCallback(errorFunc);

    // Start the mutatee
    printf("Starting \"%s\"\n", pathname);

    const char* child_argv[MAX_TEST+5];

    int n = 0;
    child_argv[n++] = pathname;
    if (debugPrint) child_argv[n++] = const_cast<char*>("-verbose");

    if (runAllTests) {
        child_argv[n++] = const_cast<char*>("-runall"); // signifies all tests
    } else {
        child_argv[n++] = const_cast<char*>("-run");
        for (unsigned int j=1; j <= MAX_TEST; j++) {
            if (runTest[j]) {
        	char str[5];
        	sprintf(str, "%d", j);
        	child_argv[n++] = strdup(str);
            }
        }
    }

    child_argv[n] = NULL;

    appThread = bpatch->createProcess(pathname, child_argv,NULL);
    appImage = appThread->getImage();

    if (runTest[1]) mutatorTest1(appThread, appImage);
    if (runTest[2]) mutatorTest2(appThread, appImage);
    if (runTest[3]) mutatorTest3(appThread, appImage);
    if (runTest[4]) mutatorTest4(appThread, appImage);

    appThread->continueExecution();

    while (!appThread->isTerminated())
        bpatch->waitForStatusChange();

    int retval;
    if(appThread->terminationStatus() == ExitedNormally) {
       int exitCode = appThread->getExitCode();
       if (exitCode || debugPrint)
          printf("Mutatee exited with exit code 0x%x\n", exitCode);
       retval = exitCode;
    } else if(appThread->terminationStatus() == ExitedViaSignal) {
       int signalNum = appThread->getExitSignal();
       if (signalNum || debugPrint)
          printf("Mutatee exited from signal 0x%d\n", signalNum);
       retval = signalNum;
    }

    dprintf("Done.\n");

    return retval;
}

//
// main - decide our role and call the correct "main"
//
int
main(unsigned int argc, char *argv[])
{
    char mutateeName[128];
    char libRTname[256];


    strcpy(mutateeName,mutateeNameRoot);
    libRTname[0]='\0';

    if (!getenv("DYNINSTAPI_RT_LIB")) {
	 fprintf(stderr,"Environment variable DYNINSTAPI_RT_LIB undefined:\n"
	         "    set it to the full pathname of libdyninstAPI_RT\n");
         exit(-1);
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
#if defined(i386_unknown_nt4_0) || defined(i386_unknown_linux2_0) || defined(sparc_sun_solaris2_4)
	} else if (!strcmp(argv[i], "-relocate")) {
            forceRelocation = true;
#endif
	} else {
	    fprintf(stderr, "Usage: test10 "
		    "[-V] [-verbose]  "
#if defined(sparc_sun_solaris2_4) || defined(i386_unknown_linux2_0) || defined(rs6000_ibm_aix4_1)
		    "[-saveworld] "
#endif 
                    "[-mutatee <test1.mutatee>] "
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
    int retVal = mutatorMAIN(mutateeName);

    int testsFailed=0;

    return retVal;
}
