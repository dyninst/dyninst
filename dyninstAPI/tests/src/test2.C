// $Id: test2.C,v 1.51 2003/01/02 19:52:02 schendel Exp $
//
// libdyninst validation suite test #2
//    Author: Jeff Hollingsworth (7/10/97)
//

//  This program tests the error features of the dyninst API.  
//	The mutatee that goes with this file is test2.mutatee.c
//	
//  Naming conventions:
//      All functions, variables, etc are name funcXX_YY, exprXX_YY, etc.
//          XX is the test number
//          YY is the instance withing the test
//	    func1_2 is the second function used in test case #1.
//

#include <stdio.h>
#include <signal.h>
#include <string.h>
#ifdef i386_unknown_nt4_0
#include <io.h>
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#else
#include <unistd.h>
#include <errno.h>
#include <sys/wait.h>
#endif

#include "BPatch.h"
#include "BPatch_Vector.h"
#include "BPatch_thread.h"
#include "BPatch_snippet.h"
#include "test_util.h"
#include "test2.h"

#ifdef i386_unknown_nt4_0
#define access _access
#define unlink _unlink
#define F_OK 0
#endif

BPatch_thread *mutatorMAIN(const char *path);
void errorFunc(BPatchErrorLevel level, int num, const char **params);
bool useAttach = false;
int debugPrint = 0; // internal "mutator" tracing
int errorPrint = 0; // external "dyninst" tracing (via errorFunc)
bool expectErrors = false;
bool gotError = false;
bool forceRelocation = false;

int mutateeCplusplus = 0;
const unsigned int MAX_TEST = 14;
bool runTest[MAX_TEST+1];
bool passedTest[MAX_TEST+1];

BPatch *bpatch;

static const char *mutateeNameRoot = "test2.mutatee";
char mutateeName[128];

// control debug printf statements
#define dprintf	if (debugPrint) printf

//
// Test #1 - run an executable that does not exist
//	Attempt to run a program file that does not exist.  Should return a
//	null values from createProcess (called via mutatorMAIN)
//

void test1()
{
    if (useAttach) {
	printf("Skipping test #1 (run an executable that does not exist)\n");
	printf("    not relevant with -attach option\n");
	passedTest[1] = true;
    } else {
	// try to run a program that does not exist
	gotError = false;
	BPatch_thread *ret = mutatorMAIN("./noSuchFile");
	if (ret || !gotError) {
	    printf("**Failed** test #1 (run an executable that does not exist)\n");
	    if (ret)
		printf("    created a thread handle for a non-existant file\n");
	    if (!gotError)
		printf("    the error callback should have been called but wasn't\n");
	} else {
	    printf("Passed test #1 (run an executable that does not exist)\n");
	    passedTest[1] = true;
	}
    }
}

//
// Test #2 - try to execute a file that is not a valid program
//	Try to run a createProcess on a file that is not an executable file 
//	(via mutatorMAIN).
//
void test2()
{
    if (useAttach) {
	printf("Skipping test #2 (try to execute a file that is not a valid program)\n");
	printf("    not relevant with -attach option\n");
	passedTest[2] = true;
	return;
    }

    // try to run a files that is not a valid program
    gotError = false;
#ifdef i386_unknown_nt4_0
    BPatch_thread *ret = mutatorMAIN("nul:");
#else
    BPatch_thread *ret = mutatorMAIN("/dev/null");
#endif
    if (ret || !gotError) {
	printf("**Failed** test #2 (try to execute a file that is not a valid program)\n");
	if (ret)
	    printf("    created a thread handle for invalid executable\n");
	if (!gotError)
	    printf("    the error callback should have been called but wasn't\n");
    } else {
	printf("Passed test #2 (try to execute a file that is not a valid program)\n");
	passedTest[2] = true;
    }
}

//
// Test #3 - attach to an invalid pid
//	Try to attach to an invalid pid number (65539).
//
void test3()
{
#if defined(sparc_sun_sunos4_1_3)
    printf("Skipping test #3 (attach to an invalid pid)\n");
    printf("    attach is not supported on this platform\n");
    passedTest[3] = true;
#else
    // attach to an an invalid pid
    gotError = false;
    BPatch_thread *ret = bpatch->attachProcess(mutateeName, 65539);
    if (ret || !gotError) {
	printf("**Failed** test #3 (attach to an invalid pid)\n");
	if (ret)
    	    printf("    created a thread handle for invalid executable\n");
	if (!gotError)
	    printf("    the error callback should have been called but wasn't\n");
    } else {
	printf("Passed test #3 (attach to an invalid pid)\n");
	passedTest[3] = true;
    }
#endif
}

//
// Test #4 - attach to a protected pid
//	Try to attach to a "protected pid" number.  This should test the
//	procfs attach to a pid that we don't have "rw" access to. The pid
//	selected is pid #1 which is a OS kernel process that user processes
//	can't read or write.
//

void test4()
{
#if defined(sparc_sun_sunos4_1_3)
    printf("Skipping test #4 (attach to a protected pid)\n");
    printf("    attach is not supported on this platform\n");
    passedTest[4] = true;
#else
    // attach to an a protected pid
    gotError = false;
    BPatch_thread *ret = bpatch->attachProcess(mutateeName, 1);
    if (ret || !gotError) {
	printf("**Failed** test #4 (attach to a protected pid)\n");
	if (ret)
    	    printf("    created a thread handle for invalid executable\n");
	if (!gotError)
	    printf("    the error callback should have been called but wasn't\n");
    } else {
	printf("Passed test #4 (attach to a protected pid)\n");
	passedTest[4] = true;
    }
#endif
}

//
// Test #5 - look up nonexistent function)
//	Try to call findFunction on a function that is not defined for the
//	process.
//
void test5(BPatch_image *img)
{
    gotError = false;
    expectErrors = true; // test #5 causes error #100 (Unable to find function)
    BPatch_function *func = img->findFunction("NoSuchFunction");
    expectErrors = false;
    if (func || !gotError) {
	printf("**Failed** test #5 (look up nonexistent function)\n");
	if (func)
    	    printf("    non-null for findFunction on non-existant func\n");
	if (!gotError)
	    printf("    the error callback should have been called but wasn't\n");
    } else {
	printf("Passed test #5 (look up nonexistent function)\n");
	passedTest[5] = true;
    }
}

//
// Test #6 - load a dynamically linked library from the mutatee
//	Have the mutatee use dlopen (or NT loadLibrary) to load a shared library
//	into itself.  We should then be able to see the new functions from the
//	library via getModules.
//

void test6(BPatch_thread *thread, BPatch_image *img)
{
#if !defined(sparc_sun_solaris2_4) \
 && !defined(i386_unknown_solaris2_5) \
 && !defined(i386_unknown_linux2_0) \
 && !defined(mips_sgi_irix6_4) \
 && !defined(alpha_dec_osf4_0) \
 && !defined(rs6000_ibm_aix4_1) \
 && !defined(ia64_unknown_linux2_4) /* Temporary duplication - TLM */

    printf("Skipping test #6 (load a dynamically linked library from the mutatee)\n");
    printf("    feature not implemented on this platform\n");
    passedTest[6] = true;
#else

    bool found = false;

    // see if the dlopen happended.
    char match2[256];
    sprintf(match2, "%s_module", TEST_DYNAMIC_LIB);
    BPatch_Vector<BPatch_module *> *m = img->getModules();
    for (unsigned i=0; i < m->size(); i++) {
	    char name[80];
	    (*m)[i]->getName(name, sizeof(name));
	    if (strcmp(name, TEST_DYNAMIC_LIB) == 0 ||
#ifdef rs6000_ibm_aix4_1
		strcmp(name, TEST_DYNAMIC_LIB_NOPATH) == 0 ||
#endif
		strcmp(name, match2) == 0) {
		found = true;
		break;
	    }
    }
    if (found) {
    	printf("Passed test #6 (load a dynamically linked library from the mutatee)\n");
	passedTest[6] = true;
    } else {
    	printf("**Failed** test #6 (load a dynamically linked library from the mutatee)\n");
	printf("    image::getModules() did not indicate that the library had been loaded\n");
    }
#endif
}

//
// Test #7 - load a dynamically linked library from the mutator
// 	Have the mutator "force" a load of a library into the mutatee.
//	We then check that the new symbols are found in the thread via the
//	getModules method.
//
void test7(BPatch_thread *thread, BPatch_image *img)
{
#if !defined(sparc_sun_solaris2_4) \
 && !defined(i386_unknown_solaris2_5) \
 && !defined(i386_unknown_linux2_0) \
 && !defined(mips_sgi_irix6_4) \
 && !defined(rs6000_ibm_aix4_1) \
 && !defined(ia64_unknown_linux2_4) /* Temporary duplication - TLM */
    printf("Skipping test #7 (load a dynamically linked library from the mutator)\n");
    printf("    feature not implemented on this platform\n");
    passedTest[7] = true;
#else

    if (!thread->loadLibrary(TEST_DYNAMIC_LIB2)) {
    	printf("**Failed** test #7 (load a dynamically linked library from the mutator)\n");
	printf("    BPatch_thread::loadLibrary returned an error\n");
    } else {
	// see if it worked
	bool found = false;
	char match2[256];
	sprintf(match2, "%s_module", TEST_DYNAMIC_LIB2);
	BPatch_Vector<BPatch_module *> *m = img->getModules();
	for (unsigned int i=0; i < m->size(); i++) {
		char name[80];
		(*m)[i]->getName(name, sizeof(name));
		if (strcmp(name, TEST_DYNAMIC_LIB2) == 0 ||
#ifdef rs6000_ibm_aix4_1
		    strcmp(name, TEST_DYNAMIC_LIB2_NOPATH) == 0 ||
#endif
		    strcmp(name, match2) == 0) {
		    found = true;
		    break;
		}
	}
	if (found) {
	    printf("Passed test #7 (load a dynamically linked library from the mutator)\n");
	    passedTest[7] = true;
	} else {
	    printf("**Failed** test #7 (load a dynamically linked library from the mutator)\n");
	    printf("    image::getModules() did not indicate that the library had been loaded\n");
	}
    }
#endif
}

//
// Start of test #8 - BPatch_breakPointExpr
//
//   There are two parts to the mutator side of this test.  The first part
//     (test8a) inserts a BPatch_breakPointExpr into the entry point of
//     the function test8_1.  The secon pat (test8b) waits for this breakpoint
//     to be reached.  The first part is run before the processes is continued
//     (i.e. just after process creation or attach).
//
void test8a(BPatch_thread *appThread, BPatch_image *appImage)
{
    /*
     * Instrument a function with a BPatch_breakPointExpr.
     */

  BPatch_Vector<BPatch_function *> found_funcs;
    if ((NULL == appImage->findFunction("func8_1", found_funcs, 1)) || (0 == found_funcs.size())) {
      fprintf(stderr, "    Unable to find function %s\n",
	      "func8_1");
      exit(1);
    }

    if (1 < found_funcs.size()) {
      fprintf(stderr, "%s[%d]:  WARNING  : found %d functions named %s.  Using the first.\n", 
	      __FILE__, __LINE__, found_funcs.size(), "func8_1");
    }

    BPatch_Vector<BPatch_point *> *points = found_funcs[0]->findPoint(BPatch_entry);

    if (points == NULL) {
	printf("**Failed** test #8 (BPatch_breakPointExpr)\n");
	printf("    unable to locate function \"func8_1\".\n");
	exit(1);
    }

    BPatch_breakPointExpr bp;

    if (appThread->insertSnippet(bp, *points) == NULL) {
	printf("**Failed** test #8 (BPatch_breakPointExpr)\n");
	printf("    unable to insert breakpoint snippet\n");
	exit(1);
    }
}

void test8b(BPatch_thread *thread)
{
    // Wait for process to finish
    waitUntilStopped(bpatch, thread, 8, "BPatch_breakPointExpr");

    // waitUntilStopped would not return is we didn't stop
    printf("Passed test #8 (BPatch_breakPointExpr)\n");
    passedTest[8] = true;
}

//
// Test #9 - dump core but do not terminate
//	This test dumps the core file from the mutatee process, without having
//	the process terminate exuection.  It looks for the creation of the file
//	"mycore" in the current directory.
//      
void test9(BPatch_thread *thread)
{
#if !defined(sparc_sun_sunos4_1_3) \
 && !defined(sparc_sun_solaris2_4) 
    printf("Skipping test #9 (dump core but do not terminate)\n");
    printf("    BPatch_thread::dumpCore not implemented on this platform\n");
    passedTest[9] = true;
#else

    // dump core, but do not terminate.
    // this doesn't seem to do anything - jkh 7/12/97
    if (access("mycore", F_OK) == 0) {
        dprintf("File \"mycore\" exists.  Deleting it.\n");
	if (unlink("mycore") != 0) {
	    printf("Couldn't delete the file \"mycore\".  Exiting.\n");
	    exit(-1);
	}
    }

    gotError = false;
    thread->dumpCore("mycore", true);
    bool coreExists = (access("mycore", F_OK) == 0);
    if (gotError || !coreExists) {
	printf("**Failed** test #9 (dump core but do not terminate)\n");
	if (gotError)
	    printf("    error reported by dumpCore\n");
	if (!coreExists)
	    printf("    the core file wasn't written\n");
    } else {
	unlink("mycore");
    	printf("Passed test #9 (dump core but do not terminate)\n");
	passedTest[9] = true;
    }
#endif
}

//
// Test #10 - dump image
//	This test dumps out the modified program file.  Note: only the 
//      modified executable is written, any shared libraries that have been
//	instrumented are not written.  In addition, the current dyninst
//	shared library is *NOT* written either.  Thus the results image is
//	really only useful for checking the state of instrumentation code
//	via gdb. It will crash if you try to run it.
//
void test10(BPatch_thread *thread)
{
#if !defined(rs6000_ibm_aix4_1) \
 && !defined(sparc_sun_sunos4_1_3) \
 && !defined(sparc_sun_solaris2_4) \
 && !defined(i386_unknown_linux2_0) \
 && !defined(mips_sgi_irix6_4) \
 && !defined(alpha_dec_osf4_0) \
 && !defined(ia64_unknown_linux2_4) /* Temporary duplication - TLM */

    printf("Skipping test #10 (dump image)\n");
    printf("    BPatch_thread::dumpImage not implemented on this platform\n");
    passedTest[10] = true;
#else

  if (thread->isTerminated()) {
    printf( "**Failed** test #10 (dump image)\n" );
    printf("%s[%d]: mutatee in unexpected (terminated) state\n", __FILE__, __LINE__);
    exit(1);
  }

    // dump image
    if (access("myimage", F_OK) == 0) {
	dprintf("File \"myimage\" exists.  Deleting it.\n");
	if (unlink("myimage") != 0) {
	    printf("Couldn't delete the file \"myimage\".  Exiting.\n");
	    exit(-1);
	}
    }

    gotError = false;
    thread->dumpImage("myimage");
    bool imageExists = (access("myimage", F_OK) == 0);
    if (gotError || !imageExists) {
	printf("**Failed** test #10 (dump image)\n");
	if (gotError)
	    printf("    error reported by dumpImage\n");
	if (!imageExists)
	    printf("    the image file wasn't written\n");
    } else {
    	printf("Passed test #10 (dump image)\n");
	unlink("myimage");
	passedTest[10] = true;
    }
#endif
}

//
// Test 11 - getDisplacedInstructions
//	This function tests the getDisplacedInstructions instructions methods.
//	Currently this tests is only enabled on AIX platforms.
//
void test11(BPatch_thread * appThread, BPatch_image *appImage)
{

  BPatch_Vector<BPatch_function *> found_funcs;
    if ((NULL == appImage->findFunction("func11_1", found_funcs, 1)) || (0 == found_funcs.size())) {
      fprintf(stderr, "    Unable to find function %s\n",
	      "func11_1");
      exit(1);
    }

    if (1 < found_funcs.size()) {
      fprintf(stderr, "%s[%d]:  WARNING  : found %d functions named %s.  Using the first.\n", 
	      __FILE__, __LINE__, found_funcs.size(), "func11_1");
    }

    BPatch_Vector<BPatch_point *> *points = found_funcs[0]->findPoint(BPatch_entry);

    if (points == NULL) {
	printf("**Failed** test #11 (getDisplacedInstructions)\n");
	printf("    unable to locate function \"func11_1\".\n");
	return;
    }

    char buf[128];
    memset(buf, 128, 0);
    int nbytes = (*points)[0]->getDisplacedInstructions(128, buf);
    if (nbytes < 0 || nbytes > 128) {
	printf("**Failed** test #11 (getDisplacedInstructions)\n");
	printf("    getDisplacedInstructions returned a strange number of bytes (%d)\n", nbytes);
        return;
    }
    int i;
    for (i = 0; i < nbytes; i++) {
	if (buf[i] != 0) break;
    }
    if (i == nbytes) {
	printf("**Failed** test #11 (getDisplacedInstructions)\n");
	printf("    getDisplacedInstructions doesn't seem to have returned any instructions\n");
        return;
    }
    printf("Passed test #11 (getDisplacedInstructions)\n");
    passedTest[11] = true;
}

//
// Test #12 - BPatch_point query funcs
//	This tests the BPatch_point functions that supply information about
//	an inst. point:
//		getAddress - should return the address of the point
//		usesTrap_NP - returns true of the point requires a trap
//			instruction.

void test12(BPatch_thread *appThread, BPatch_image *appImage)
{
   
  BPatch_Vector<BPatch_function *> found_funcs;
    if ((NULL == appImage->findFunction("func12_1", found_funcs, 1)) || (0 == found_funcs.size())) {
      fprintf(stderr, "    Unable to find function %s\n",
	      "func12_1");
      exit(1);
    }

    if (1 < found_funcs.size()) {
      fprintf(stderr, "%s[%d]:  WARNING  : found %d functions named %s.  Using the first.\n", 
	      __FILE__, __LINE__, found_funcs.size(), "func12_1");
    }

    BPatch_Vector<BPatch_point *> *points = found_funcs[0]->findPoint(BPatch_entry);
    
    if (!points) {
	printf("**Failed** test #12 (BPatch_point query funcs)\n");
	printf("    unable to locate function \"func12_1\".\n");
	exit(1);
    }

    //void *addr = (*points)[0]->getAddress();

    //bool trapFlag = 
    (*points)[0]->usesTrap_NP();

    printf("Passed test #12 (BPatch_point query funcs)\n");
    passedTest[12] = true;
}

char loadLibErrStr[256] = "no error";

void llErrorFunc(BPatchErrorLevel level, int num, const char **params)
{

  char line[256];
  const char *msg = bpatch->getEnglishErrorString(num);
  bpatch->formatErrorString(line, sizeof(line), msg, params);

  if (num == 124) {
    strcpy(loadLibErrStr, line);
  }
  else {
    printf("Unexpected Error #%d (level %d): %s\n", num, level, line);
  }

}

// Start Test Case #13 - (loadLibrary failure test)
void test13( BPatch_thread * appThread, BPatch_image * appImage )
{

#if !defined(i386_unknown_nt4_0) && !defined(alpha_dec_osf4_0)

  if (appThread->isTerminated()) {
    printf( "**Failed** test #13 (dlopen failure reporting test)\n" );
    printf("%s[%d]: mutatee in unexpected (terminated) state\n", __FILE__, __LINE__);
    exit(1);
  }

  bpatch->registerErrorCallback(llErrorFunc);
  
  if (appThread->loadLibrary("adskfoieweadsf")) {
    fprintf(stderr, "**Failed** test #13 (failure reporting for loadLibrary)\n");
    exit(1);
  }
  else {
    if (!strcmp(loadLibErrStr, "no error")) {
      printf( "**Failed** test #13 (dlopen failure reporting test)\n" );
      printf( "\tno error string produced\n" );
      passedTest[13] = false;
    }
    else {
      passedTest[13] = true;
      printf( "Passed test #13 (dlopen failure test: %s)\n", loadLibErrStr);
    }
  }
  bpatch->registerErrorCallback(errorFunc);
#else
  passedTest[13] = true;
  printf( "Skipped test #13 (dlopen failure reporting test)\n" );
  printf( "\t- not implemented on this platform\n" );
#endif

}

//
// Test 14 - Look through the thread list and make sure the thread has
//    been deleted as required.
//
void test14(BPatch_thread *thread)
{
    bool failed_this = false;
    BPatch_Vector<BPatch_thread *> *threads = bpatch->getThreads();
    for (unsigned int i=0; i < threads->size(); i++) {
	if ((*threads)[i] == thread) {
	    printf("**Failed** test #14 (delete thread)\n"); 
	    printf("    thread %d was deleted, but getThreads found it\n",
		thread->getPid());
	    failed_this = true;
	}
    }

    if (!failed_this) {
	printf("Passed test #14 (delete thread)\n");
	passedTest[14] = true;
    }
}


BPatch_thread *mutatorMAIN(const char *pathname)
{
    BPatch_thread *appThread;

    // Start the mutatee
    dprintf("Starting \"%s\"\n", pathname);

    char *child_argv[MAX_TEST+4];
   
    int n = 0;
    child_argv[n++] = const_cast<char*>(pathname);
    if (debugPrint) child_argv[n++] = const_cast<char*>("-verbose");

    child_argv[n++] = const_cast<char*>("-run");
    for (unsigned int j=0; j <= MAX_TEST; j++) {
        if (runTest[j]) {
            char str[5];
            sprintf(str, "%d", j);
            child_argv[n++] = strdup(str);
        }
    }

    child_argv[n] = NULL;

    if (useAttach) {
	int pid = startNewProcessForAttach(pathname, child_argv);
	if (pid < 0 && !expectErrors) {
	    printf("*ERROR*: unable to start tests due to error starting mutatee process\n");
	    exit(-1);
        } else {
            dprintf("New mutatee process pid %d started; attaching...\n", pid);
	}
        P_sleep(1); // let the mutatee catch its breath for a moment
	appThread = bpatch->attachProcess(pathname, pid);
    } else {
	appThread = bpatch->createProcess(pathname, child_argv);
    }

    return appThread;
}

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
        
        gotError = true;

        if (expectErrors) {
            dprintf("Error (expected) #%d (level %d): %s\n", num, level, line);
        } else {
            printf("Error #%d (level %d): %s\n", num, level, line);
        }
    }
}

//
// main - decide our role and call the correct "main"
//
int
main(unsigned int argc, char *argv[])
{
    BPatch_thread *ret;

    bool N32ABI=false;
    char libRTname[256];

    strcpy(mutateeName,mutateeNameRoot);
    libRTname[0]='\0';

    unsigned int i;

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
	} else if (!strcmp(argv[i], "-attach")) {
	    useAttach = true;
        } else if (!strcmp(argv[i], "-skip")) {
            unsigned int j;
            for (j=i+1; j < argc; j++) {
                unsigned int testId;
                if ((testId = atoi(argv[j]))) {
                    if ((testId > 0) && (testId <= MAX_TEST)) {
                        runTest[testId] = false;
                    } else {
                        printf("%s[%d]: invalid test %d requested\n", __FILE__, __LINE__, testId);
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
            for (j=0; j <= MAX_TEST; j++) runTest[j] = false;
            for (j=i+1; j < argc; j++) {
                unsigned int testId;
                if ((testId = atoi(argv[j]))) {
                    if ((testId > 0) && (testId <= MAX_TEST)) {
                        runTest[testId] = true;
                    } else {
                        printf("%s[%d]: invalid test %d requested\n", __FILE__, __LINE__, testId);
                        exit(-1);
                    }
                } else {
                    // end of test list
                    break;
                }
            }
            i = j-1;
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
	    fprintf(stderr, "Usage: test2 "
		    "[-V] [-verbose] [-attach] "
#if defined(mips_sgi_irix6_4)
		    "[-n32] "
#endif
                    "[-mutatee <test2.mutatee>] "
		    "[-run <test#> <test#> ...] "
		    "[-skip <test#> <test#> ...]\n");
            fprintf(stderr, "%d subtests\n", MAX_TEST);
	    exit(-1);
	}
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

    bpatch->registerErrorCallback(errorFunc);

    // Try failure cases
    expectErrors = true;

    if (runTest[1]) test1();
    if (runTest[2]) test2();
    if (runTest[3]) test3();
    if (runTest[4]) test4();

    // Finished trying failure cases
    expectErrors = false;

    // now start a real program
    gotError = false;
    ret = mutatorMAIN(mutateeName);
    if (!ret || gotError) {
	printf("*ERROR*: unable to create handle for executable\n");
        exit(-1);
    }

    BPatch_image *img = ret->getImage();

    // Signal the child that we've attached
    if ( useAttach )
		signalAttached( ret, img );

    // determine whether mutatee is C or C++
    BPatch_variableExpr *isCxx = img->findVariable("mutateeCplusplus");
    if (isCxx == NULL) {
	fprintf(stderr, "  Unable to locate variable \"mutateeCplusplus\""
                 " -- assuming 0!\n");
    } else {
        isCxx->readValue(&mutateeCplusplus);
        dprintf("Mutatee is %s.\n", mutateeCplusplus ? "C++" : "C");
    }

    if (runTest[5]) test5(img);

    if (runTest[8]) test8a(ret, img);

    ret->continueExecution();

    // Tests 6 and 7 need to be run with the thread stopped
    if (runTest[6] || runTest[7]) {
	waitUntilStopped(bpatch, ret, 6, "load a dynamically linked library");

	if (runTest[6]) test6(ret, img);
	if (runTest[7]) test7(ret, img);

	ret->continueExecution();
    }

    if (runTest[8]) test8b(ret);
    if (runTest[9]) test9(ret);
    if (runTest[10]) test10(ret);
    if (runTest[11]) test11(ret, img);
    if (runTest[12]) test12(ret, img);
    if (runTest[13]) test13(ret, img);     

    /**********************************************************************
     * Kill process and make sure it goes away
     **********************************************************************/
    
    int pid = ret->getPid();

#ifndef i386_unknown_nt4_0 /* Not yet implemented on NT. */
    dprintf("Detaching from process %d (leaving it running).\n", pid);
    ret->detach(true);
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
        dprintf("Killing mutatee process %d.\n", pid);
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

    delete (ret);

    if (runTest[14]) test14(ret);

    delete (bpatch);

    unsigned int testsFailed = 0;
    for (i=1; i <= MAX_TEST; i++) {
	if (runTest[i] && !passedTest[i]) testsFailed++;
    }

    if (!testsFailed) {
        printf("All tests passed\n");
    } else {
        printf("**Failed** %d test%c\n",testsFailed,(testsFailed>1)?'s':' ');
    }

    return (testsFailed ? 127 : 0);
}
