// $Id: test8.C,v 1.1 2002/12/05 01:38:40 buck Exp $
//

#include <stdio.h>
#include <string.h>
#include <assert.h>
#ifdef i386_unknown_nt4_0
#include <windows.h>
#include <winbase.h>
#else
#include <unistd.h>
#endif

#include "BPatch.h"
#include "BPatch_Vector.h"
#include "BPatch_thread.h"
#include "BPatch_snippet.h"
#include "test_util.h"

char *mutateeNameRoot = "test8.mutatee";

int expectError = 112;
int debugPrint = 0; // internal "mutator" tracing
int errorPrint = 0; // external "dyninst" tracing (via errorFunc)

bool forceRelocation = false;  // Force relocation upon instrumentation

#define dprintf if (debugPrint) printf

bool runAllTests = true;
const unsigned int MAX_TEST = 2;
bool runTest[MAX_TEST+1];
bool passedTest[MAX_TEST+1];
bool failedTest[MAX_TEST+1];

BPatch *bpatch;

typedef struct {
    bool             valid;
    BPatch_frameType type;
    char             *function_name;
} frameInfo_t;

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

bool checkStack(BPatch_thread *appThread,
		const frameInfo_t correct_frame_info[],
		int num_correct_names,
		int test_num, char *test_name)
{
    int i;

    const int name_max = 256;
    bool failed = false;

    BPatch_Vector<BPatch_frame> stack;
    appThread->getCallStack(stack);

    if (debugPrint) {
	printf("Stack in test %d (%s):\n", test_num, test_name);
	for (int i = 0; i < stack.size(); i++) {
	    char name[name_max];
	    BPatch_function *func = stack[i].findFunction();
	    if (func == NULL)
		strcpy(name, "[UNKNOWN]");
	    else
		func->getName(name, name_max);
	    printf("  %10p: %s, fp = %p\n",
		    stack[i].getPC(),
		    name,
		    stack[i].getFP());
	}
	printf("End of stack dump.\n");
    }

    if (stack.size() < num_correct_names) {
	fprintf(stderr, "**Failed** test %d (%s)\n", test_num, test_name);
	fprintf(stderr, "    Stack trace should contain more frames.\n");
	failed = true;
    }

    for (i = 0; i < num_correct_names; i++) {
#if !defined(i386_unknown_nt4_0)
	if (i < stack.size()-1 && stack[i].getFP() == 0) {
	    fprintf(stderr, "**Failed** test %d (%s)\n", test_num, test_name);
	    fprintf(stderr, "    A stack frame other than the lowest has a null FP.\n");
	    failed = true;
	    break;
	}
#endif

	if (correct_frame_info[i].valid) {
	    char name[name_max], name2[name_max];

	    BPatch_function *func = stack[i].findFunction();
	    if (func != NULL)
		func->getName(name, name_max);

	    BPatch_function *func2 =
		appThread->findFunctionByAddr(stack[i].getPC());
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

	    if (correct_frame_info[i].type != stack[i].getFrameType()) {
		fprintf(stderr, "**Failed** test %d (%s)\n", test_num, test_name);
		fprintf(stderr, "    Stack frame #%d has wrong type, is %s, should be %s\n", i+1, frameTypeString(stack[i].getFrameType()), frameTypeString(correct_frame_info[i].type));
		failed = true;
		break;
	    }

	    if (stack[i].getFrameType() == BPatch_frameSignal ||
		stack[i].getFrameType() == BPatch_frameTrampoline) {
		// No further checking for these types right now
	    } else {
		if (func == NULL) {
		    fprintf(stderr, "**Failed** test %d (%s)\n",
			    test_num, test_name);
		    fprintf(stderr, "    Stack frame #%d refers to an unknown function, should refer to %s\n", i+1, correct_frame_info[i].function_name);
		    failed = true;
		    break;
		} else { /* func != NULL */
		    if (strcmp(name, correct_frame_info[i].function_name) != 0) {
			fprintf(stderr, "**Failed** test %d (%s)\n", test_num, test_name);
			fprintf(stderr, "    Stack frame #%d refers to function %s, should be %s\n", i+1, name, correct_frame_info[i].function_name);
			failed = true;
			break;
		    }
		}
	    }
	}
    }

    return !failed;
}

void mutatorTest1(BPatch_thread *appThread, BPatch_image *appImage)
{
#if !defined(alpha_dec_osf4_0)
    static const frameInfo_t correct_frame_info[] = {
#if !defined(rs6000_ibm_aix4_1)
	{ false, BPatch_frameNormal, NULL },
#endif
#if !defined(i386_unknown_nt4_0)
	{ true,  BPatch_frameNormal, "stop_process_" },
#endif
	{ true,  BPatch_frameNormal, "func1_3" },
	{ true,  BPatch_frameNormal, "func1_2" },
	{ true,  BPatch_frameNormal, "func1_1" },
	{ true,  BPatch_frameNormal, "main" },
    };

    waitUntilStopped(bpatch, appThread, 1, "getCallStack");

    if (checkStack(appThread,
		   correct_frame_info,
		   sizeof(correct_frame_info)/sizeof(frameInfo_t),
		   1, "getCallStack")) {
	passedTest[1] = true;
	printf("Passed test #1 (getCallStack)\n");
    }

    appThread->continueExecution();
#else
    printf("Skipping test #1 (getCallStack)\n");
    printf("    feature not implemented on this platform\n");
     passedTest[1] = true;
#endif
}

void mutatorTest2(BPatch_thread *appThread, BPatch_image *appImage)
{
#if defined(i386_unknown_linux2_0) || defined(sparc_sun_solaris2_4)
    static const frameInfo_t correct_frame_info[] = {
#if !defined(rs6000_ibm_aix4_1)
	{ false, BPatch_frameNormal, NULL },
#endif
	{ true,  BPatch_frameNormal, "stop_process_" },
	{ true,  BPatch_frameNormal, "func2_4" },
	{ true,  BPatch_frameNormal, "sigalrm_handler" },
	{ true,  BPatch_frameSignal, NULL },
	{ true,  BPatch_frameNormal, "func2_3" },
	{ true,  BPatch_frameNormal, "func2_2" },
	{ true,  BPatch_frameNormal, "func2_1" },
	{ true,  BPatch_frameNormal, "main" }
    };

    waitUntilStopped(bpatch, appThread, 2, "getCallStack in signal handler");

    if (checkStack(appThread,
		   correct_frame_info,
		   sizeof(correct_frame_info)/sizeof(frameInfo_t),
		   2, "getCallStack in signal handler")) {
	passedTest[2] = true;
	printf("Passed test #2 (getCallStack in signal handler)\n");
    }

    appThread->continueExecution();
#else
    printf("Skipping test #2 (getCallStack in signal handler)\n");
    printf("    feature not implemented on this platform\n");
     passedTest[2] = true;
#endif
}

int mutatorMAIN(char *pathname)
{
    BPatch_thread *appThread;

    // Create an instance of the BPatch library
    bpatch = new BPatch;

    // Force functions to be relocated
    if (forceRelocation) {
      bpatch->setForcedRelocation_NP(true);
    }

    // Register a callback function that prints any error messages
    bpatch->registerErrorCallback(errorFunc);

    // Start the mutatee
    printf("Starting \"%s\"\n", pathname);

    char *child_argv[MAX_TEST+5];

    int n = 0;
    child_argv[n++] = pathname;
    if (debugPrint) child_argv[n++] = "-verbose";

    if (runAllTests) {
        child_argv[n++] = "-runall"; // signifies all tests
    } else {
        child_argv[n++] = "-run";
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

    if (appThread == NULL) {
	fprintf(stderr, "Unable to run test program.\n");
	exit(1);
    }

    // Read the program's image and get an associated image object
    BPatch_image *appImage = appThread->getImage();

    dprintf("starting program execution.\n");
    appThread->continueExecution();

    if (runTest[1]) mutatorTest1(appThread, appImage);
    if (runTest[2]) mutatorTest2(appThread, appImage);

    // Start of code to continue the process.  All mutations made
    // above will be in place before the mutatee begins its tests.

    while (!appThread->isTerminated()) {
	if (appThread->isStopped())
	    appThread->continueExecution();
	bpatch->waitForStatusChange();
    }

    int exitCode = appThread->terminationStatus();
    if (exitCode || debugPrint) printf("Mutatee exit code 0x%x\n", exitCode);

    unsigned int testsFailed = 0;
    for (unsigned int i=1; i <= MAX_TEST; i++) {
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

    dprintf("Done.\n");
    return(exitCode);
}

//
// main
//
int
main(unsigned int argc, char *argv[])
{
    unsigned int i;
    bool N32ABI=false;
    char mutateeName[128];
    char libRTname[256];

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
#if defined(i386_unknown_nt4_0) || defined(i386_unknown_linux2_0) || defined(sparc_sun_solaris2_4) || defined(ia64_unknown_linux2_4)
	} else if (!strcmp(argv[i], "-relocate")) {
            forceRelocation = true;
#endif
#if defined(mips_sgi_irix6_4)
	} else if (!strcmp(argv[i], "-n32")) {
	    N32ABI=true;
#endif
	} else {
	    fprintf(stderr, "Usage: test4 "
		    "[-V] [-verbose] "
#if defined(mips_sgi_irix6_4)
		    "[-n32] "
#endif
                    "[-mutatee <test4a.mutatee>] "
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

    mutatorMAIN(mutateeName);

    return 0;
}
