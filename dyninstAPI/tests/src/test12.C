//  This program tests the basic features of the dyninst API.
//      The mutatee that goes with this file is test1.mutatee.c
//
//  Naming conventions:
//      All functions, variables, etc are name funcXX_YY, exprXX_YY, etc.
//          XX is the test number
//          YY is the instance withing the test
//          func1_2 is the second function used in test case #1.
//

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>
#include <stdarg.h>
#ifdef i386_unknown_nt4_0
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <winsock2.h>
#include <winbase.h>
#else
#include <unistd.h>
#endif

#include <iostream>
using namespace std;

#include "BPatch.h"
#include "BPatch_Vector.h"
#include "BPatch_thread.h"
#include "BPatch_snippet.h"
#include "test_util.h"

/* include spinlock function from RT lib as if in a header file */
/* note:  testing assembly code this way is only really accurate on */
/* gnu-compiled systems */

#define EXPORT_SPINLOCKS_AS_HEADER 1
#if defined (os_solaris) 
#include "dyninstAPI_RT/src/RTsolaris.c"
#elif defined (os_linux) 
#include "dyninstAPI_RT/src/RTlinux.c"
#elif defined (os_windows)
#include "dyninstAPI_RT/src/RTwinnt.c"
#elif defined (os_irix)
#define GNU_TO_ASS 1
#include "dyninstAPI_RT/src/RTirix.c"
#elif defined (os_osf)
#include "dyninstAPI_RT/src/RTosf.c"
#elif defined (os_aix)
#include "dyninstAPI_RT/src/RTaix.c"
#endif

int debugPrint = 0; // internal "mutator" tracing
int errorPrint = 1; // external "dyninst" tracing (via errorFunc)

bool forceRelocation = false; // force relocation of functions

int mutateeCplusplus = 0;
int mutateeFortran = 0;
int mutateeXLC = 0;
int mutateeF77 = 0;
bool runAllTests = true;
const unsigned int MAX_TEST = 4;
bool runTest[MAX_TEST+1];
bool passedTest[MAX_TEST+1];

BPatch *bpatch;

static const char *mutateeNameRoot = "test12.mutatee";
static const char *libNameAroot = "libtestA";
static const char *libNameBroot = "libtestB";
char libNameA[64], libNameB[64];

//  Globals for BPatch_thread and BPatch_image, just makes it easier
BPatch_thread *appThread;
BPatch_image *appImage;
char err_str[1024];

// control debug printf statements
void dprintf(const char *fmt, ...) {
   va_list args;
   va_start(args, fmt);

   if(debugPrint)
      vfprintf(stderr, fmt, args);

   va_end(args);

   fflush(stderr);
}

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

  ts.tv_nsec = (ms % 1000) * 1000 * 1000;
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
            else {
                printf("%s", params[0]);
            }
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

BPatch_function *findFunction(const char *fname, int testno, const char *testname)
{
  BPatch_Vector<BPatch_function *> bpfv;
  if (NULL == appImage->findFunction(fname, bpfv) || (bpfv.size() != 1)) {

      fprintf(stderr, "**Failed test #%d (%s)\n", testno, testname);
      fprintf(stderr, "  Expected 1 functions matching %s, got %d\n",
              fname, bpfv.size());
         exit(1);
  }
  return bpfv[0];
}

void setVar(const char *vname, void *addr, int testno, const char *testname)
{
   BPatch_variableExpr *v;
   void *buf = addr;
   if (NULL == (v = appImage->findVariable(vname))) {
      fprintf(stderr, "**Failed test #%d (%s)\n", testno, testname);
      fprintf(stderr, "  cannot find variable %s\n", vname);
         exit(1);
   }

   if (! v->writeValue(buf, sizeof(int),true)) {
      fprintf(stderr, "**Failed test #%d (%s)\n", testno, testname);
      fprintf(stderr, "  failed to write call site var to mutatee\n");
      exit(1);
   }
}



#define FAIL(x,y) fprintf(stdout, "**Failed test #%d (%s)\n", x,y);
#define PASS(x,y) fprintf(stdout, "Passed test #%d (%s)\n", x,y);
#define SKIP(x,y) fprintf(stdout, "Skipped test #%d (%s)\n", x,y);
#define SLEEP_INTERVAL 100 /*ms*/
#define TIMEOUT 7000 /*ms*/

/*******************************************************************************/
/*******************************************************************************/
/*******************************************************************************/

#define TESTNO 1
#define TESTNAME "dynamic call site callback"
static const char *expected_fnames[] = {"call1_1","call1_2","call1_3","call1_4"};
int test1done = 0;
int test1err = 0;
template class BPatch_Vector<void *>;
BPatch_Vector<void *> test1handles;
BPatch_Vector<BPatch_point *> dyncalls;

void dynSiteCB(BPatch_point *dyn_site, BPatch_function *called_function)
{
  //fprintf(stderr, "%s[%d]:  dynSiteCB: pt = %p. func = %p.\n",
  //                 __FILE__, __LINE__, dyn_site, called_function);
  static int counter = 0;
  static int counter2 = 0;
  BPatch_point *pt = dyn_site;
  BPatch_function *func = called_function;
  assert(pt);
  assert(func);
  void *callsite_addr = pt->getAddress();
  char buf[2048];
  func->getName(buf, 2048);
  //fprintf(stderr, "%s[%d]:  got func %s, expect func %s\n", __FILE__, __LINE__, buf,
  //        expected_fnames[counter]);
  if (strcmp(expected_fnames[counter], buf)) {
    FAIL(TESTNO, TESTNAME);
    printf("\t%s[%d]:  got func %s, expect func %s\n", __FILE__, __LINE__, buf,
          expected_fnames[counter]);
    appThread->stopExecution();      
    test1done = 1;
  }
  counter++;
  if (counter > 3) {
    counter = 0;
    counter2++;
  }

  if (counter2 >= 2) {
    bool removal_error = false;
    appThread->stopExecution();      
    //  not passed yet, now remove dynamic call monitoring handles
    assert (test1handles.size());
    for (unsigned int i = 0; i < test1handles.size(); ++i) {
      if (!dyncalls[i]->removeDynamicCallCallback(test1handles[i])) {
        removal_error = true;
      }
    }
    if (removal_error) {
      FAIL(TESTNO, TESTNAME);
      test1err = 1;
    }else {
      PASS(TESTNO, TESTNAME);
    }
    test1done = 1;
  }
}


bool mutatorTest1()
{
  int timeout = 0;

  if (mutateeXLC) {
     appThread->continueExecution();
     SKIP(TESTNO, TESTNAME);
     fprintf(stderr, "\txlc optimizes out dynamic call sites for this test\n");
     sleep_ms(100);
     return true;
  }

  BPatch_function *func1_1 = findFunction("call1_dispatch", TESTNO, TESTNAME);
  BPatch_function *targetFunc = func1_1;

  BPatch_Vector<BPatch_point *> *calls = targetFunc->findPoint(BPatch_subroutine);
  if (!calls) {
     FAIL(TESTNO, TESTNAME);
     fprintf(stderr, "  cannot find call points for func1_1\n");
     exit(1);
  }

  for (unsigned int i = 0; i < calls->size(); ++i) {
    BPatch_point *pt = (*calls)[i];
    if (pt->isDynamic()){
      void *handle = NULL;
      handle = pt->registerDynamicCallCallback(dynSiteCB);
      if (!handle) {
        FAIL(TESTNO, TESTNAME);
        fprintf(stderr, "  registerDynamicCallCallback failed\n");
        exit(1);
      } 
      test1handles.push_back(handle);
      dyncalls.push_back(pt);
    }
  }

  if (dyncalls.size() != 3) {
     FAIL(TESTNO, TESTNAME);
     fprintf(stderr, "  wrong number of dynamic points found (%d -- not 3)\n",
             dyncalls.size());
     fprintf(stderr, "  total number of calls found: %d\n", calls->size());
        exit(1);
  }

  appThread->continueExecution();

  //  wait until we have received the desired number of events
  //  (or timeout happens)

  while(!test1done && (timeout < TIMEOUT)) {
    bpatch->pollForStatusChange();
    sleep_ms(SLEEP_INTERVAL/*ms*/);
    timeout += SLEEP_INTERVAL;
  }

  if (timeout >= TIMEOUT) {
    FAIL(TESTNO, TESTNAME);
    fprintf(stderr, "%s[%d]:  test timed out.\n",
           __FILE__, __LINE__);
    test1err = 1;
  }

  return (test1err == 0);
}

#undef  TESTNO
#define TESTNO 2
#undef  TESTNAME
#define TESTNAME "rtlib spinlocks"
#define THREADS 20

#if !defined(os_windows) 
#include <pthread.h>
#include "dyninstAPI_RT/h/dyninstAPI_RT.h"
extern void DYNINSTlock_spinlock(dyninst_spinlock *);
extern void DYNINSTunlock_spinlock(dyninst_spinlock *);

dyninst_spinlock mut;
int current_locks[THREADS];
pthread_t test2threads[THREADS];
pthread_mutex_t real_lock;
int test2counter = 0;
int test2err = 0;

void register_my_lock(unsigned long id, int val) 
{
  unsigned int i;
  int found = 0;
  for (i = 0; i < THREADS; ++i) {
    if (pthread_equal(test2threads[i],(pthread_t)id)) {
      found = 1;
      current_locks[i] = val;
      break;
    }
  }
  if (!found)
    fprintf(stderr, "%s[%d]: FIXME\n", __FILE__, __LINE__);
}
int is_only_one() {
  unsigned int i;
  int foundone = 0;
  for (i = 0; i < THREADS; ++i) {
    if (0 != current_locks[i]) {
      if (foundone) return 0; /*false*/
      foundone++;
    } 
  }
  return 1; /*true */
}

void *thread_main (void *arg)
{
   DYNINSTlock_spinlock(&mut);
   register_my_lock((unsigned long)pthread_self(),1);
   pthread_mutex_lock(&real_lock);
   /*printf("thread %lu got mutex %d\n", (unsigned long)pthread_self(), mut.lock); */
   sleep_ms (1);
   if (!is_only_one()) {
     FAIL(TESTNO, TESTNAME);
     test2err = 1;
   }
   pthread_mutex_unlock(&real_lock);
   /*printf("thread %lu unlocking mutex\n", (unsigned long) pthread_self()); */
   register_my_lock((unsigned long)pthread_self(),0);
   test2counter++;
   DYNINSTunlock_spinlock(&mut);
   return NULL;
}

void DYNINSTunlock_spinlock(dyninst_spinlock*s)
{
  s->lock = 0;
}

#endif
bool mutatorTest2()
{

#if !defined (os_windows) && !defined(os_irix)
  unsigned int timeout = 0; // in ms
  pthread_attr_t attr;
  unsigned int i;
  mut.lock = 0;
  /*pthread_mutex_attr_t mutattr; */
  pthread_mutex_init(&real_lock, NULL);

  pthread_attr_init(&attr);
  pthread_attr_setdetachstate( &attr, PTHREAD_CREATE_DETACHED);
  pthread_attr_setscope(&attr, PTHREAD_SCOPE_SYSTEM);
  DYNINSTlock_spinlock(&mut);

  for (i = 0; i < THREADS; ++i) {
    current_locks[i] = 0;
    int err;
    err = pthread_create(&(test2threads[i]), &attr, thread_main, NULL);
    if (err) {
      fprintf(stderr, "Error creating thread %d: %s[%d]\n",
              i, strerror(errno), errno);
    }
  }

  sleep_ms(1);
  DYNINSTunlock_spinlock(&mut);

  while((test2counter < THREADS) && !test2err && (timeout < TIMEOUT)) {
    sleep_ms(SLEEP_INTERVAL/*ms*/);
    timeout += SLEEP_INTERVAL;
  }

  if (timeout >= TIMEOUT) {
    FAIL(TESTNO, TESTNAME);
    fprintf(stderr, "%s[%d]:  test timed out.\n",
           __FILE__, __LINE__);
    test2err = 1;
  }


  if (test2err) {
    for (i = 0; i < THREADS; ++i) {
     pthread_kill(test2threads[i],9);
    }
  }
  else {
    PASS(TESTNO, TESTNAME);
  }
#else
    SKIP(TESTNO, TESTNAME);
    fprintf(stderr, "%s[%d]:  This test is not supported on this platform\n",
                    __FILE__, __LINE__);
#endif
  return true;
}


#undef  TESTNO
#define TESTNO 3
#undef  TESTNAME
#define TESTNAME "thread create callback"

int test3_threadCreateCounter = 0;
void threadCreateCB(BPatch_thread *thr, unsigned long thread_id)
{
  test3_threadCreateCounter++;
//  fprintf(stderr, "%s[%d]:  got a thread start event: %d\n", __FILE__, __LINE__,
//          test3_threadCreateCounter);
}

bool mutatorTest3()
{
  unsigned int timeout = 0; // in ms
  int err = 0;

  BPatchThreadEventCallback createcb = threadCreateCB;
  if (!appThread->registerThreadEventCallback(BPatch_threadCreateEvent,
                                              createcb)) {
    FAIL(TESTNO, TESTNAME);
    fprintf(stderr, "%s[%d]:  failed to register thread callback\n",
           __FILE__, __LINE__);
    return false;
  }
  //  unset mutateeIde to trigger thread (10) spawn.
  int zero = 0;
  setVar("mutateeIdle", (void *) &zero, TESTNO, TESTNAME);
  appThread->continueExecution();

  //  wait until we have received the desired number of events
  //  (or timeout happens)

  while(test3_threadCreateCounter < 10 && (timeout < TIMEOUT)) {
    sleep_ms(SLEEP_INTERVAL/*ms*/);
    timeout += SLEEP_INTERVAL;
    bpatch->pollForStatusChange();
  }

  if (timeout >= TIMEOUT) {
    FAIL(TESTNO, TESTNAME);
    fprintf(stderr, "%s[%d]:  test timed out.\n",
           __FILE__, __LINE__);
    err = 1;
  }

  appThread->stopExecution();

  if (!appThread->removeThreadEventCallback(BPatch_threadCreateEvent,
                                            createcb)) {
    FAIL(TESTNO, TESTNAME);
    fprintf(stderr, "%s[%d]:  failed to remove thread callback\n",
           __FILE__, __LINE__);
    return false;
  }
  if (!err)  {
    PASS(TESTNO, TESTNAME);
    return true;
  }
  return false;
}

#undef  TESTNO
#define TESTNO 4 
#undef  TESTNAME
#define TESTNAME "thread exit callback"

int test4_threadDestroyCounter = 0;
void threadDestroyCB(BPatch_thread *thr, unsigned long thread_id)
{
  test4_threadDestroyCounter++;
 // fprintf(stderr, "%s[%d]:  got a thread destroy event: %d\n", __FILE__, __LINE__,
  //        test4_threadDestroyCounter);
}

bool mutatorTest4()
{
  unsigned int timeout = 0; // in ms
  int err = 0;

  BPatchThreadEventCallback destroycb = threadDestroyCB;
  if (!appThread->registerThreadEventCallback(BPatch_threadDestroyEvent,
                                              destroycb)) {
    FAIL(TESTNO, TESTNAME);
    fprintf(stderr, "%s[%d]:  failed to register thread callback\n",
           __FILE__, __LINE__);
    return false;
  }

  //  unset mutateeIdle to trigger thread (10) spawn.

  int zero = 0;
  setVar("mutateeIdle", (void *) &zero, TESTNO, TESTNAME);
  appThread->continueExecution();

  //  wait until we have received the desired number of events
  //  (or timeout happens)
  while(test4_threadDestroyCounter < 10 && (timeout < TIMEOUT)) {
    sleep_ms(SLEEP_INTERVAL/*ms*/);
    timeout += SLEEP_INTERVAL; 
    bpatch->pollForStatusChange();
  }

  if (timeout >= TIMEOUT) {
    FAIL(TESTNO, TESTNAME);
    fprintf(stderr, "%s[%d]:  test timed out.\n",
           __FILE__, __LINE__);
    err = 1;
  }

  appThread->stopExecution();

  if (!appThread->removeThreadEventCallback(BPatch_threadDestroyEvent,
                                            destroycb)) {
    FAIL(TESTNO, TESTNAME);
    fprintf(stderr, "%s[%d]:  failed to remove thread callback\n",
           __FILE__, __LINE__);
    return false;
  }

  if (!err) {
    PASS(TESTNO, TESTNAME);
    return true;
  }
  return false;
}


#undef  TESTNO
#define TESTNO 5 
#undef  TESTNAME
#define TESTNAME "thread start callback"

bool mutatorTest5()
{

  return true;
}

#undef  TESTNO
#define TESTNO 6 
#undef  TESTNAME
#define TESTNAME "thread stop callback"

bool mutatorTest6()
{

  return true;
}

/*******************************************************************************/
/*******************************************************************************/
/*******************************************************************************/

int mutatorMAIN(char *pathname, bool useAttach)
{
        char *dirName;

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

    if (useAttach) {
        int pid = startNewProcessForAttach(pathname, child_argv);
        if (pid < 0) {
            printf("*ERROR*: unable to start tests due to error creating mutatee process\n");
            exit(-1);
        } else {
            dprintf("New mutatee process pid %d started; attaching...\n", pid);
        }
        P_sleep(1); // let the mutatee catch its breath for a moment
        appThread = bpatch->attachProcess(pathname, pid);
    } else {
        appThread = bpatch->createProcess(pathname, child_argv,NULL);
    }

    if (appThread == NULL) {
        fprintf(stderr, "Unable to run test program.\n");
        exit(1);
    }

    // Read the program's image and get an associated image object
    appImage = appThread->getImage();

    // Signal the child that we've attached
    if (useAttach) {
        signalAttached(appThread, appImage);
    }

    // determine whether mutatee is C or C++
    BPatch_variableExpr *isCxx = appImage->findVariable("mutateeCplusplus");
    if (isCxx == NULL) {
        fprintf(stderr, "  Unable to locate variable \"mutateeCplusplus\""
                 " -- assuming 0!\n");
    } else {
        isCxx->readValue(&mutateeCplusplus);
        dprintf("Mutatee is %s.\n", mutateeCplusplus ? "C++" : "C");
    }

    printf("\n");

    if (runTest[1]) passedTest[1] = mutatorTest1();
    if (runTest[2]) passedTest[2] = mutatorTest2();
    if (runTest[3]) passedTest[3] = mutatorTest3();
    if (runTest[4]) passedTest[4] = mutatorTest4();
    //if (runTest[5]) passedTest[5] = mutatorTest5();
    //if (runTest[6]) passedTest[6] = mutatorTest6();

    if (appThread && !appThread->isTerminated())
      appThread->terminateExecution();

    printf("\n");

    int failed_tests  = 0;
    for (unsigned int i = 1; i < (MAX_TEST+1); ++i) {
      if ((runTest[i]) && (!passedTest[i])) {
         failed_tests++;
      }
    }
    if (failed_tests) {
      printf("**Failed %d test%s\n", failed_tests, 
                                     failed_tests == 1 ? "." : "s.");
      for (unsigned int i = 1; i < (MAX_TEST + 1); ++i) {
        printf("%s ", passedTest[i] ? "P" : "F");
      }
      printf("\n");
    }
    else {
      if (runAllTests)
        printf("All tests passed.\n");
      else
        printf("All requested tests passed.\n");
    }
    return 0;
}

int
main(unsigned int argc, char *argv[])
{
    char mutateeName[128];
    char libRTname[256];

    bool N32ABI = false;
    bool useAttach = false;

    strcpy(mutateeName,mutateeNameRoot);
    strcpy(libNameA,libNameAroot);
    strcpy(libNameB,libNameBroot);
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
        } else if (!strcmp(argv[i], "-attach")) {
            useAttach = true;
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
#if defined(mips_sgi_irix6_4)
        } else if (!strcmp(argv[i], "-n32")) {
            N32ABI = true;
#endif
        } else {
            fprintf(stderr, "Usage: test12 "
                    "[-V] [-verbose] [-attach] "
#if defined(mips_sgi_irix6_4)
                    "[-n32] "
#endif
#if defined(sparc_sun_solaris2_4) || defined(i386_unknown_linux2_0) || defined(rs6000_ibm_aix4_1)
                    "[-saveworld] "
#endif
                    "[-mutatee <test12.mutatee>] "
                    "[-run <test#> <test#> ...] "
                    "[-skip <test#> <test#> ...]\n");
            fprintf(stderr, "%d subtests\n", MAX_TEST);
            exit(-1);
        }
    }

    //  detect IBM xlC compiler and set flag
    if (strstr(mutateeName, "xlc") || strstr(mutateeName, "xlC"))
      mutateeXLC = true;

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
        strcat(libNameA,"_n32");
        strcat(libNameB,"_n32");
    }

    // patch up the platform-specific filename extensions
#if defined(i386_unknown_nt4_0)
    if (!strstr(mutateeName, ".exe")) strcat(mutateeName,".exe");
    strcat(libNameA,".dll");
    strcat(libNameB,".dll");
#else
    strcat(libNameA,".so");
    strcat(libNameB,".so");
#endif

    int retval = mutatorMAIN(mutateeName, useAttach);

    return retval;
}

