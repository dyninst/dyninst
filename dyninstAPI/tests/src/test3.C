// $Id: test3.C,v 1.4 1999/06/20 03:38:28 wylie Exp $
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

extern "C" const char V_libdyninstAPI[];

int debugPrint = 0;

const int MAX_TEST = 4;
bool passedTest[MAX_TEST+1];

template class BPatch_Vector<BPatch_variableExpr*>;

BPatch *bpatch;

// control debug printf statements
#define dprintf	if (debugPrint) printf

/**************************************************************************
 * Error callback
 **************************************************************************/

#define DYNINST_NO_ERROR -1

int expectError = DYNINST_NO_ERROR;

void errorFunc(BPatchErrorLevel level, int num, const char **params)
{
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

/**************************************************************************
 * Utility functions
 **************************************************************************/

//
// Return a pointer to a string identifying a BPatch_procedureLocation
//
char *locationName(BPatch_procedureLocation l)
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
    BPatch_Vector<BPatch_point *> *points =
	appImage->findProcedurePoint(inFunction, loc);

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
    BPatch_function *call_func = appImage->findFunction(funcName);
    if (call_func == NULL) {
	fprintf(stderr, "**Failed** test #%d (%s)\n", testNo, testName);
	fprintf(stderr, "    Unable to find function %s\n", funcName);
	exit(1);
    }

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


/**************************************************************************
 * Tests
 **************************************************************************/

//
// Start Test Case #1 - create two processes and process events from each
//     Just let them run to finish, no instrumentation added.
//
void mutatorTest1(char *pathname, BPatch *bpatch)
{
    char *child_argv[4];

    child_argv[0] = pathname;
    child_argv[1] = "-?";
    child_argv[2] = NULL;
    BPatch_thread *appThread1, *appThread2;

    child_argv[1] = "1";		// run test1 in mutatee
    appThread1 = bpatch->createProcess(pathname, child_argv, NULL);
    child_argv[1] = "1";		// run test1 in mutatee 
    appThread2 = bpatch->createProcess(pathname, child_argv, NULL);

    appThread1->continueExecution();
    appThread2->continueExecution();

    while (!appThread1->isTerminated() || !appThread2->isTerminated())
	bpatch->waitForStatusChange();

    if (appThread1->isTerminated() && appThread2->isTerminated()) {
	printf("Passed Test #1\n");
	passedTest[1] = true;
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
// Start Test Case #2 - create two processes and insert different code into
//     each one.  The code sets a global variable which the mutatee then
//     writes to a file.  After both mutatee exit, the mutator reads the
//     files to verify that the correct code ran in each mutatee.
//     The first mutator should write a 1 to the file and the second a 2.
//     If no code is patched into the mutatees, the value is 0xdeadbeef.
//
void mutatorTest2(char *pathname, BPatch *bpatch)
{
    char *child_argv[4];

    int pid1, pid2;
    child_argv[0] = pathname;
    child_argv[1] = "-?";
    child_argv[2] = NULL;
    BPatch_thread *appThread1, *appThread2;

    child_argv[1] = "2";		// run test2 in mutatee
    appThread1 = bpatch->createProcess(pathname, child_argv, NULL);
    pid1 = appThread1->getPid();
    child_argv[1] = "2";		// run test2 in mutatee 
    appThread2 = bpatch->createProcess(pathname, child_argv, NULL);
    pid2 = appThread2->getPid();

    BPatch_image *img1 = appThread1->getImage();
    BPatch_image *img2 = appThread2->getImage();

    BPatch_variableExpr *var1 = img1->findVariable("test2ret");
    BPatch_arithExpr snip1(BPatch_assign, *var1, BPatch_constExpr(1));
    insertSnippetAt(appThread1, img1, "func2_1", BPatch_entry, snip1, 2,
	"different inst. active");

    BPatch_variableExpr *var2 = img2->findVariable("test2ret");
    BPatch_arithExpr snip2(BPatch_assign, *var2, BPatch_constExpr(2));
    insertSnippetAt(appThread2, img2, "func2_1", BPatch_entry, snip2, 2,
	"different inst. active");

    appThread1->continueExecution();
    appThread2->continueExecution();

    while (!appThread1->isTerminated() || !appThread2->isTerminated())
	bpatch->waitForStatusChange();

    // now read the files to see if the value is what is expected
    int ret1 = readResult(pid1);
    int ret2 = readResult(pid2);

    if ((ret1 != 1) || (ret2 != 2)) {
	printf("**Failed** test case #2\n");
	if (ret1 != 1) printf("    1st process produced %d, not 1\n", ret1);
	if (ret2 != 2) printf("    2nd process produced %d, not 2\n", ret2);
    } else {
	printf("Passed Test #2\n");
	passedTest[2] = true;
    }
}

//
// Start Test Case #3 - create one process, wait for it to exit.  Then 
//     create a second one and wait for it to exit.
//
void mutatorTest3(char *pathname, BPatch *bpatch)
{
    char *child_argv[4];

    child_argv[0] = pathname;
    child_argv[1] = "-?";
    child_argv[2] = NULL;
    BPatch_thread *appThread1, *appThread2;

    child_argv[1] = "1";		// run test1 in mutatee
    appThread1 = bpatch->createProcess(pathname, child_argv, NULL);

    appThread1->continueExecution();

    while (!appThread1->isTerminated())
	bpatch->waitForStatusChange();

    child_argv[1] = "1";		// run test1 in mutatee 
    appThread2 = bpatch->createProcess(pathname, child_argv, NULL);

    appThread2->continueExecution();
    while (!appThread2->isTerminated())
	bpatch->waitForStatusChange();

    printf("Passed Test #3\n");
    passedTest[3] = true;
}


//
// Start Test Case #4 - create one process, wait for it to exit.  Then 
//     create a second one and wait for it to exit.  This differs from test 3
//     in that the mutatee processes terminate with abort rather than exit.
//
void mutatorTest4(char *pathname, BPatch *bpatch)
{
    char *child_argv[4];

    child_argv[0] = pathname;
    child_argv[1] = "-?";
    child_argv[2] = NULL;
    BPatch_thread *appThread1, *appThread2;

    child_argv[1] = "4";		// run test4 in mutatee
    appThread1 = bpatch->createProcess(pathname, child_argv, NULL);

    appThread1->continueExecution();

    while (!appThread1->isTerminated())
	bpatch->waitForStatusChange();

    child_argv[1] = "4";		// run test4 in mutatee 
    appThread2 = bpatch->createProcess(pathname, child_argv, NULL);

    appThread2->continueExecution();
    while (!appThread2->isTerminated())
	bpatch->waitForStatusChange();

    printf("Passed Test #4\n");
    passedTest[4] = true;
}

int main(unsigned int argc, char *argv[])
{
    char libname[256];
    bool runTest[MAX_TEST+1];		// should we run a particular test

    libname[0]='\0';
#if !defined(USES_LIBDYNINSTRT_SO)
    fprintf(stderr,"(Expecting subject application to be statically linked"
                        " with libdyninstAPI_RT.)\n");
#else
    strcpy((char*) libname, (char*) getenv("DYNINSTAPI_RT_LIB"));
    if (strlen(libname) == 0) {
        fprintf(stderr,"Environment variable DYNINSTAPI_RT_LIB undefined:\n"
#if defined(i386_unknown_nt4_0)
            "    using standard search strategy for libdyninstAPI_RT.dll\n");
#else
                "    set it to the full pathname of libdyninstAPI_RT\n");   
        exit(-1);
#endif
    }
#endif

    unsigned int i;
    // by default run all tests
    for (i=0; i <= MAX_TEST; i++) {
	runTest[i] = true;
	passedTest[i] = false;
    }

    for (i=1; i < argc; i++) {
	if (!strcmp(argv[i], "-verbose")) {
	    debugPrint = 1;
	} else if (!strcmp(argv[i], "-V")) {
            fprintf (stdout, "%s\n", V_libdyninstAPI);
            if (libname[0]) fprintf (stdout, "DYNINSTAPI_RT_LIB=%s\n", libname);
            fflush(stdout);
	} else if (!strcmp(argv[i], "-run")) {
	    unsigned int j;
	    for (j=0; j <= MAX_TEST; j++) runTest[j] = false;
	    for (j=i; j < argc; j++) { 
		int testId;
		if (testId = atoi(argv[j])) {
		    if ((testId > 0) && (testId <= MAX_TEST)) {
			runTest[testId] = true;
		    } else {
			printf("invalid test %d requested\n", testId);
			exit(-1);
		    }
		} else {
		    // end of test list
		}
	    }
	    i=j-1;
	} else {
	    fprintf(stderr, "Usage: test3 [-V] [-verbose] [-run #]\n");
	    exit(-1);
        }
    }

    printf("Running Tests: ");
    for (int j=1; j <= MAX_TEST; j++) {
	if (runTest[j]) printf("%d ", j);
    }
    printf("\n");

    // Create an instance of the bpatch library
    bpatch = new BPatch;

    // Register a callback function that prints any error messages
    bpatch->registerErrorCallback(errorFunc);

#ifdef i386_unknown_nt4_0
    char *programToRun = "test3.mutatee.exe";
#else
    char *programToRun = "test3.mutatee";
#endif

    if (runTest[1]) mutatorTest1(programToRun, bpatch);
    if (runTest[2]) mutatorTest2(programToRun, bpatch);
    if (runTest[3]) mutatorTest3(programToRun, bpatch);
    if (runTest[4]) mutatorTest4(programToRun, bpatch);

    bool allPassed = true;
    for (i=1; i <= MAX_TEST; i++) {
	if (runTest[i] && !passedTest[i]) allPassed = false;
    }

    if (allPassed) {
	printf("All requested tests passed\n");
    } else {
	printf("**Some requested tests failed**\n");
    }
    return 0;
}
