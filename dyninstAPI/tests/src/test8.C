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

// $Id: test8.C,v 1.16 2005/02/24 10:18:10 rchen Exp $
//

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stdarg.h>
#ifdef i386_unknown_nt4_0
#define WIN32_LEAN_AND_MEAN
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

const char *mutateeNameRoot = "test8.mutatee";

int expectError = 112;
int debugPrint = 1; // internal "mutator" tracing
int errorPrint = 0; // external "dyninst" tracing (via errorFunc)

bool forceRelocation = false;  // Force relocation upon instrumentation

void dprintf(const char *fmt, ...) {
   va_list args;
   va_start(args, fmt);

   if(debugPrint)
      vfprintf(stderr, fmt, args);

   va_end(args);

   fflush(stderr);
}

bool runAllTests = true;
const unsigned int MAX_TEST = 3;
bool runTest[MAX_TEST+1];
bool passedTest[MAX_TEST+1];
bool failedTest[MAX_TEST+1];

BPatch *bpatch;

typedef struct {
    bool             valid;
    bool             optional;
    BPatch_frameType type;
    const char      *function_name;
} frameInfo_t;

void errorFunc(BPatchErrorLevel level, int num, const char **params)
{
    if (num == 0) {
        // conditional reporting of warnings and informational messages
        if (errorPrint) {
            if (level == BPatchInfo)
              { if (errorPrint > 1) fprintf( stderr, "%s\n", params[0]); }
            else
                fprintf( stderr, "%s", params[0]);
        }
    } else {
        // reporting of actual errors
        char line[256];
        const char *msg = bpatch->getEnglishErrorString(num);
        bpatch->formatErrorString(line, sizeof(line), msg, params);
        
        if (num != expectError) {
            fprintf(stderr, "Error #%d (level %d): %s\n", num, level, line);
        
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

void mutatorTest1(BPatch_thread *appThread, BPatch_image *appImage)
{
#if !defined(alpha_dec_osf4_0)
    static const frameInfo_t correct_frame_info[] = {
#if defined( os_linux ) && (defined( arch_x86 ) || defined( arch_x86_64 ))
	{ true, true, BPatch_frameNormal, "_dl_sysinfo_int80" },
#endif
#if !defined(rs6000_ibm_aix4_1)
	{ false, false, BPatch_frameNormal, NULL },
#endif
#if !defined(i386_unknown_nt4_0)
	{ true,  false, BPatch_frameNormal, "stop_process_" },
#endif
	{ true,  false, BPatch_frameNormal, "func1_3" },
	{ true,  false, BPatch_frameNormal, "func1_2" },
	{ true,  false, BPatch_frameNormal, "func1_1" },
	{ true,  false, BPatch_frameNormal, "main" },
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
#if defined(i386_unknown_linux2_0) \
 || defined(x86_64_unknown_linux2_4) /* Blind duplication - Ray */ \
 || defined(sparc_sun_solaris2_4) \
 || defined(ia64_unknown_linux2_4)

    static const frameInfo_t correct_frame_info[] = {

#if defined( os_linux ) && (defined( arch_x86 ) || defined( arch_x86_64 ))
	{ true, true, BPatch_frameNormal, "_dl_sysinfo_int80" },
#endif
#if !defined(rs6000_ibm_aix4_1)
	{ false, false, BPatch_frameNormal, NULL },
#endif	
	{ true,  false, BPatch_frameNormal, "stop_process_" },
	{ true,  false, BPatch_frameNormal, "func2_4" },
	{ true,  false, BPatch_frameNormal, "sigalrm_handler" },
	{ true,  false, BPatch_frameSignal, NULL },
	{ true,  false, BPatch_frameNormal, "func2_3" },
	{ true,  false, BPatch_frameNormal, "func2_2" },
	{ true,  false, BPatch_frameNormal, "func2_1" },
	{ true,  false, BPatch_frameNormal, "main" }
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

#if defined( DEBUG )
#include <sys/ptrace.h>
#endif
void mutatorTest3( BPatch_thread * appThread, BPatch_image * appImage ) {

#if (defined( arch_alpha ) && defined( os_osf )) \
 || (defined( arch_sparc ) && defined( os_solaris ))
	printf("Skipping test #3 (getCallStack through instrumentation)\n");
	printf("    unwinding through base & minitramps not implemented on this platform\n");
	passedTest[3] = true;
#else
	static const frameInfo_t correct_frame_info[] = {
	
#if defined( os_linux ) && (defined( arch_x86 ) || defined( arch_x86_64 ))
	  { true, true, BPatch_frameNormal, "_dl_sysinfo_int80" },
#endif
#if defined( os_aix ) && defined( arch_power )
		/* AIX uses kill(), but the PC of a process in a syscall can
		   not be correctly determined, and appears to be the address
		   to which the syscall function will return. */
#elif defined( os_windows ) && (defined( arch_x86 ) || defined( arch_x86_64 ))
		/* Windows/x86 does not use kill(), so its lowermost frame will be 
		   something unidentifiable in a system DLL. */
		{ false, false, BPatch_frameNormal, NULL },
#else
		{ true, false, BPatch_frameNormal, "kill" },	
#endif
#if ! defined( os_windows )
		/* Windows/x86's stop_process_() calls DebugBreak(); it's 
		   apparently normal to lose this frame. */
		{ true, false, BPatch_frameNormal, "stop_process_" },
#endif
		{ true, false, BPatch_frameNormal, "func3_3" },
		{ true, false, BPatch_frameTrampoline, NULL },
		/* On AIX and x86 (and others), if our instrumentation fires
		   before frame construction or after frame destruction, it's 
		   acceptable to not report the function (since, after all, it
		   doesn't have a frame on the stack. */
		{ true, true, BPatch_frameNormal, "func3_2" },
		{ true, false, BPatch_frameNormal, "func3_1" },
		{ true, false, BPatch_frameNormal, "main" }
		};
	
	/* Wait for the mutatee to stop in func3_1(). */
	waitUntilStopped( bpatch, appThread, 1, "getCallStack through instrumentation" );
	
	/* Instrument func3_2() to call func3_3(), which will trip another breakpoint. */
	BPatch_Vector<BPatch_function *> instrumentedFunctions;	
	appImage->findFunction( "func3_2", instrumentedFunctions );
	assert( instrumentedFunctions.size() == 1 );
	
	BPatch_Vector<BPatch_point *> * functionEntryPoints = instrumentedFunctions[0]->findPoint( BPatch_entry );
	assert( functionEntryPoints->size() == 1 );
	
	BPatch_Vector<BPatch_function *> calledFunctions;
	appImage->findFunction( "func3_3", calledFunctions );
	assert( calledFunctions.size() == 1 );
	
	BPatch_Vector<BPatch_snippet *> functionArguments;
	BPatch_funcCallExpr functionCall( * calledFunctions[0], functionArguments );
	
	appThread->insertSnippet( functionCall, functionEntryPoints[0] );

	/* Repeat for all three types of instpoints. */
	BPatch_Vector<BPatch_point *> * functionCallPoints = instrumentedFunctions[0]->findPoint( BPatch_subroutine );
	assert( functionCallPoints->size() == 1 );
	appThread->insertSnippet( functionCall, functionCallPoints[0] );
	
	BPatch_Vector<BPatch_point *> * functionExitPoints = instrumentedFunctions[0]->findPoint( BPatch_exit );
	assert( functionExitPoints->size() == 1 );
	appThread->insertSnippet( functionCall, functionExitPoints[0] );

#if defined( DEBUG )
	for( int i = 0; i < 80; i++ ) { ptrace( PTRACE_SINGLESTEP, appThread->getPid(), NULL, NULL ); }
	
	for( int i = 80; i < 120; i++ ) {
		ptrace( PTRACE_SINGLESTEP, appThread->getPid(), NULL, NULL );
		
	    BPatch_Vector<BPatch_frame> stack;
	    appThread->getCallStack( stack );
		
		fprintf( stderr, "single-step stack walk, %d instructions after stop for instrumentation.\n", i );
		for( unsigned i = 0; i < stack.size(); i++ ) {
			char name[ 40 ];
			BPatch_function * func = stack[i].findFunction();
		
			if( func == NULL ) { strcpy( name, "[UNKNOWN]" ); }
			else { func->getName( name, 40 ); }
			
			fprintf( stderr, "  %10p: %s, fp = %p\n", stack[i].getPC(), name, stack[i].getFP() );
			} /* end stack walk dumper */
		fprintf( stderr, "end of stack walk.\n" );
		} /* end single-step iterator */
#endif /* defined( DEBUG ) */		

	/* After inserting the instrumentation, let it be called. */
	appThread->continueExecution();
	  
	/* Wait for the mutatee to stop because of the instrumentation we just inserted. */
	waitUntilStopped( bpatch, appThread, 1, "getCallStack through instrumentation (entry)" );

	passedTest[3] = true;
	if( !checkStack( appThread, correct_frame_info,
			 sizeof(correct_frame_info)/sizeof(frameInfo_t),
			 3, "getCallStack through instrumentation (entry)" ) ) {
	    passedTest[3] = false;
    	}

	/* Repeat for other two types of instpoints. */
	appThread->continueExecution();	

	/* Wait for the mutatee to stop because of the instrumentation we just inserted. */
	waitUntilStopped( bpatch, appThread, 1, "getCallStack through instrumentation (call)" );

	if( !checkStack( appThread, correct_frame_info,
			 sizeof(correct_frame_info)/sizeof(frameInfo_t),
			 3, "getCallStack through instrumentation (call)" ) ) {
	    passedTest[3] = false;
    	}
    	
	appThread->continueExecution();	

	/* Wait for the mutatee to stop because of the instrumentation we just inserted. */
	waitUntilStopped( bpatch, appThread, 1, "getCallStack through instrumentation (exit)" );

	if( !checkStack( appThread, correct_frame_info,
			 sizeof(correct_frame_info)/sizeof(frameInfo_t),
			 3, "getCallStack through instrumentation (exit)" ) ) {
	    passedTest[3] = false;
    	}

	if (passedTest[3])
	    printf("Passed test #3 (unwind through base and mini tramps)\n");

	/* Return the mutatee to its normal state. */
	appThread->continueExecution();	

#endif
	} /* end mutatorTest3() */

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

    const char *child_argv[MAX_TEST+5];

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
    if (runTest[3]) mutatorTest3(appThread, appImage);

    // Start of code to continue the process.  All mutations made
    // above will be in place before the mutatee begins its tests.

    while (!appThread->isTerminated()) {
       if (appThread->isStopped()) {
          appThread->continueExecution();
          }
       bpatch->waitForStatusChange();
    }

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

    return retval;
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
#if defined(i386_unknown_nt4_0) \
 || defined(i386_unknown_linux2_0) \
 || defined(x86_64_unknown_linux2_4) /* Blind duplication - Ray */ \
 || defined(sparc_sun_solaris2_4) \
 || defined(ia64_unknown_linux2_4)
	} else if (!strcmp(argv[i], "-relocate")) {
            forceRelocation = true;
#endif
#if defined(mips_sgi_irix6_4)
	} else if (!strcmp(argv[i], "-n32")) {
	    N32ABI=true;
#endif
	} else {
	    fprintf(stderr, "Usage: test8 "
		    "[-V] [-verbose] "
#if defined(mips_sgi_irix6_4)
		    "[-n32] "
#endif
                    "[-mutatee <test8.mutatee>] "
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
