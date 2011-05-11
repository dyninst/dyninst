/*
 * Copyright (c) 1996-2011 Barton P. Miller
 * 
 * We provide the Paradyn Parallel Performance Tools (below
 * described as "Paradyn") on an AS IS basis, and do not warrant its
 * validity or performance.  We reserve the right to update, modify,
 * or discontinue this software at any time.  We shall have no
 * obligation to supply such updates or modifications or any other
 * form of support to you.
 * 
 * By your use of Paradyn, you understand and agree that we (or any
 * other person or entity with proprietary rights in Paradyn) are
 * under no obligation to provide either maintenance services,
 * update services, notices of latent defects, or correction of
 * defects for Paradyn.
 * 
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */
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
#include <vector>
using namespace std;

#include "BPatch.h"
#include "BPatch_Vector.h"
#include "BPatch_thread.h"
#include "BPatch_snippet.h"
#include "test_util.h"
#include "test12.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>


#define FAIL(x,y) fprintf(stdout, "**Failed test #%d (%s)\n", x,y);
#define PASS(x,y) fprintf(stdout, "Passed test #%d (%s)\n", x,y);
#define SKIP(x,y) fprintf(stdout, "Skipped test #%d (%s)\n", x,y);
#define SLEEP_INTERVAL 100 /*ms*/

/* include spinlock function from RT lib as if in a header file */
/* note:  testing assembly code this way is only really accurate on */
/* gnu-compiled systems */

int debugPrint = 0; // internal "mutator" tracing
int errorPrint = 1; // external "dyninst" tracing (via errorFunc)

bool forceRelocation = false; // force relocation of functions

int mutateeCplusplus = 0;
int mutateeFortran = 0;
int mutateeXLC = 0;
int mutateeF77 = 0;
bool runAllTests = true;
bool runTest[MAX_TEST+1];
bool passedTest[MAX_TEST+1];
char mutateeName[128];
const char *child_argv[MAX_TEST+5];

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
}

/**************************************************************************
 * Error callback
 **************************************************************************/

#define DYNINST_NO_ERROR -1

int expectError = DYNINST_NO_ERROR;

void errorFunc(BPatchErrorLevel level, int num, const char * const *params)
{
    if (num == 0) {
        // conditional reporting of warnings and informational messages
        if (errorPrint) {
            if ((level == BPatchInfo) || (level == BPatchWarning))
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

BPatch_function *findFunction(const char *fname, BPatch_module *inmod, int testno, const char *testname)
{
  BPatch_Vector<BPatch_function *> bpfv;
  if (NULL == inmod->findFunction(fname, bpfv) || (bpfv.size() != 1)) {

      fprintf(stderr, "**Failed test #%d (%s)\n", testno, testname);
      fprintf(stderr, "  Expected 1 functions matching %s, got %d\n",
              fname, bpfv.size());
         exit(1);
  }
  return bpfv[0];
}

BPatch_point *findPoint(BPatch_function *f, BPatch_procedureLocation loc, 
                        int testno, const char *testname)
{
  assert(f);
  BPatch_Vector<BPatch_point *> *pts = f->findPoint(loc);

  if (!pts) {
    FAIL(testno, testname);
    fprintf(stderr, "%s[%d]:  no points matching requested location\n", __FILE__, __LINE__);
    exit(1);
  }

  if (pts->size() != 1) {
    FAIL(testno, testname);
    fprintf(stderr, "%s[%d]:  %d points matching requested location, not 1\n", __FILE__, __LINE__, 
           pts->size());
    exit(1);
  }

  return (*pts)[0];
}

//  at -- simple instrumentation.  As written, only can insert funcs without args -- 
//     -- modify to take snippet vector args if necessary.
BPatchSnippetHandle *at(BPatch_point * pt, BPatch_function *call, 
                        int testno, const char *testname)
{
  BPatch_Vector<BPatch_snippet *> args;
  BPatch_funcCallExpr snip(*call, args);
  BPatch_procedureLocation pttype = pt->getPointType();
  BPatch_callWhen when;
  if (pttype == BPatch_entry) when = BPatch_callBefore;
  else if (pttype == BPatch_exit) when = BPatch_callAfter;
  else if (pttype == BPatch_subroutine) when = BPatch_callBefore;
  else assert(0);

  BPatchSnippetHandle *ret;
  ret = appThread->insertSnippet(snip, *pt,when);

  if (!ret) {
    FAIL(testno, testname);
    fprintf(stderr, "%s[%d]:  could not insert instrumentation\n", __FILE__, __LINE__);
    exit(1);
  }

  return ret;
}

void dumpVars(void)
{
  BPatch_Vector<BPatch_variableExpr *> vars;
  appImage->getVariables(vars);
  for (unsigned int i = 0; i < vars.size(); ++i) {
    fprintf(stderr, "\t%s\n", vars[i]->getName());
  }
}

void setVar(const char *vname, void *addr, int testno, const char *testname)
{
   BPatch_variableExpr *v;
   void *buf = addr;
   if (NULL == (v = appImage->findVariable(vname))) {
      fprintf(stderr, "**Failed test #%d (%s)\n", testno, testname);
      fprintf(stderr, "  cannot find variable %s, avail vars:\n", vname);
      dumpVars();
         exit(1);
   }

   if (! v->writeValue(buf, sizeof(int),true)) {
      fprintf(stderr, "**Failed test #%d (%s)\n", testno, testname);
      fprintf(stderr, "  failed to write call site var to mutatee\n");
      exit(1);
   }
}

void getVar(const char *vname, void *addr, int len, int testno, const char *testname)
{
   BPatch_variableExpr *v;
   if (NULL == (v = appImage->findVariable(vname))) {
      fprintf(stderr, "**Failed test #%d (%s)\n", testno, testname);
      fprintf(stderr, "  cannot find variable %s: avail vars:\n", vname);
      dumpVars();
         exit(1);
   }

   if (! v->readValue(addr, len)) {
      fprintf(stderr, "**Failed test #%d (%s)\n", testno, testname);
      fprintf(stderr, "  failed to read var in mutatee\n");
      exit(1);
   }
}



/*******************************************************************************/
/*******************************************************************************/
/*******************************************************************************/
#undef  TESTNO
#define TESTNO 1
#undef  TESTNAME
#define TESTNAME "rtlib spinlocks"
#define THREADS 10

bool mutatorTest1()
{
  
  const char *local_child_argv[MAX_TEST+5];

  //  not really going to attach, just start and let run to completion,
  //  checking exit code.  

  dprintf("%s[%d]: starting mutatee for attach: %s ", __FILE__, __LINE__, mutateeName);
  int j = 0;
  int i = 0;
  local_child_argv[i++] = child_argv[j++];
  for ( j = 0; j < MAX_TEST +5; ++j) {
    if (child_argv[j]) {
       if (!strcmp(child_argv[j], "-verbose"))
          local_child_argv[i++] = child_argv[j];
    }
    else {
      local_child_argv[i++] = "-run";
      local_child_argv[i++] = "1";
      local_child_argv[i++] = NULL;
      break;
    }
  } 
  for (j = 0; i < MAX_TEST +5; ++j) {
    if (local_child_argv[j]) dprintf(" %s", local_child_argv[j]);
    else break;
  }
  dprintf("\n");

  BPatch_process *proc = bpatch->processCreate(mutateeName, local_child_argv);
  if (!proc)  {
     FAIL(TESTNO, TESTNAME);
     fprintf(stderr, "could not create process %s\n", mutateeName);
  }

  int childpid = proc->getPid();
  dprintf("%s[%d]:  mutatee process: %d\n", __FILE__, __LINE__, childpid);

  proc->continueExecution();

   //sleep (3);
   int timeout = 0;
   while (timeout < TIMEOUT) {
     sleep_ms(SLEEP_INTERVAL/*ms*/);
     timeout += SLEEP_INTERVAL;
     if (proc->isTerminated()) 
        break;
     if (proc->isStopped()) {
        //  This really shouldn't happen
        fprintf(stderr, "%s[%d]:  BAD NEWS:  process is stopped, something is broken\n", 
                __FILE__, __LINE__);
        proc->continueExecution();
     }
     if (proc->isDetached()) {
        fprintf(stderr, "%s[%d]:  BAD NEWS:  process is detached, something is broken\n", 
                __FILE__, __LINE__);
        abort();
     }
   }

   if (proc->isTerminated()) {
     switch(proc->terminationStatus()) {
       case ExitedNormally:
         {
          int code = proc->getExitCode();
          dprintf("%s[%d]:  exited normally with code %d\n", 
                  __FILE__, __LINE__, code);
          if (code != 0) return false;
          break;
         }
       case ExitedViaSignal:
         {
          int code = proc->getExitSignal();
          fprintf(stderr, "%s[%d]:  exited with signal %d\n", 
                  __FILE__, __LINE__, code);
          return false;
          break;
         }
       case NoExit:
       default:
          assert(0);
     };
   }

   if (timeout >= TIMEOUT) {
     FAIL(TESTNO, TESTNAME);
     fprintf(stderr, "%s[%d]:  test timed out.\n",
            __FILE__, __LINE__);
     //test1err = 1;
     return false;
   }

  //sleep_ms(1000/*ms*/);
  PASS(TESTNO,TESTNAME);
  delete(proc);
  return true;
}



#undef TESTNO
#undef TESTNAME
#define TESTNO 2 
#define TESTNAME "dynamic call site callback"
static const char *expected_fnames[] = {"call2_1","call2_2","call2_3","call2_4"};
int test2done = 0;
int test2err = 0;
template class BPatch_Vector<void *>;
BPatch_Vector<BPatch_point *> test2handles;
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
  if (debugPrint) 
    fprintf(stderr, "%s[%d]:  callsite addr = %p\n", __FILE__, __LINE__, callsite_addr);
  char buf[2048];
  func->getName(buf, 2048);
  //fprintf(stderr, "%s[%d]:  got func %s, expect func %s\n", __FILE__, __LINE__, buf,
  //        expected_fnames[counter]);
  if (strcmp(expected_fnames[counter], buf)) {
    FAIL(TESTNO, TESTNAME);
    printf("\t%s[%d]:  got func %s, expect func %s\n", __FILE__, __LINE__, buf,
          expected_fnames[counter]);
    appThread->stopExecution();      
    test2done = 1;
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
    assert (test2handles.size());
    for (unsigned int i = 0; i < test2handles.size(); ++i) {
      if (!test2handles[i]->stopMonitoring()) {
        removal_error = true;
      }
    }
    if (removal_error) {
      FAIL(TESTNO, TESTNAME);
      test2err = 1;
    }else {
      PASS(TESTNO, TESTNAME);
    }
    test2done = 1;
  }
}


bool mutatorTest2()
{
#if !defined(arch_ia64)
  int timeout = 0;

  if (mutateeXLC) {
     appThread->continueExecution();
     SKIP(TESTNO, TESTNAME);
     fprintf(stderr, "\txlc optimizes out dynamic call sites for this test\n");
     sleep_ms(100);
     return true;
  }

  if (!bpatch->registerDynamicCallCallback(dynSiteCB)) {
     FAIL(TESTNO, TESTNAME);
     fprintf(stderr, "  failed to register callsite callback\n");
     exit(1);
  }

  BPatch_function *func2_1 = findFunction("call2_dispatch", TESTNO, TESTNAME);
  BPatch_function *targetFunc = func2_1;

  BPatch_Vector<BPatch_point *> *calls = targetFunc->findPoint(BPatch_subroutine);
  if (!calls) {
     FAIL(TESTNO, TESTNAME);
     fprintf(stderr, "  cannot find call points for func1_1\n");
     exit(1);
  }

  for (unsigned int i = 0; i < calls->size(); ++i) {
    BPatch_point *pt = (*calls)[i];
    if (pt->isDynamic()){
      bool ret;
      ret = pt->monitorCalls();
      if (!ret) {
        FAIL(TESTNO, TESTNAME);
        fprintf(stderr, "  failed monitorCalls\n");
        exit(1);
      } 
      test2handles.push_back(pt);
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

  while(!test2done && (timeout < TIMEOUT)) {
    bpatch->pollForStatusChange();
    sleep_ms(SLEEP_INTERVAL/*ms*/);
    timeout += SLEEP_INTERVAL;
  }

  if (timeout >= TIMEOUT) {
    FAIL(TESTNO, TESTNAME);
    fprintf(stderr, "%s[%d]:  test timed out.\n",
           __FILE__, __LINE__);
    test2err = 1;
  }

  return (test2err == 0);
#else
  SKIP(TESTNO, TESTNAME);
  return true;
#endif
}

#undef  TESTNO
#define TESTNO 3
#undef  TESTNAME
#define TESTNAME "thread create callback"


vector<unsigned long> callback_tids;

int test3_threadCreateCounter = 0;
void threadCreateCB(BPatch_process * proc, BPatch_thread *thr)
{
  assert(thr);
  if (debugPrint)
     fprintf(stderr, "%s[%d]:  thread %lu start event for pid %d\n", __FILE__, __LINE__,
               thr->getTid(), thr->getPid());
  test3_threadCreateCounter++;
  callback_tids.push_back(thr->getTid());
  if (thr->isDeadOnArrival()) {
     dprintf("%s[%d]:  thread %lu is doa \n", __FILE__, __LINE__, thr->getTid());
  }
}

bool mutatorTest3and4(int testno, const char *testname)
{
  test3_threadCreateCounter = 0;
  callback_tids.clear();

  unsigned int timeout = 0; // in ms
  int err = 0;

  BPatchAsyncThreadEventCallback createcb = threadCreateCB;
  if (!bpatch->registerThreadEventCallback(BPatch_threadCreateEvent, createcb)) 
  {
    FAIL(testno, testname);
    fprintf(stderr, "%s[%d]:  failed to register thread callback\n",
           __FILE__, __LINE__);
    return false;
  }

  //  unset mutateeIde to trigger thread (10) spawn.
  int zero = 0;
  setVar("mutateeIdle", (void *) &zero, testno, testname);
  dprintf("%s[%d]:  continue execution for test %d\n", __FILE__, __LINE__, testno);
  appThread->continueExecution();

  //  wait until we have received the desired number of events
  //  (or timeout happens)

  BPatch_Vector<BPatch_thread *> threads;
  BPatch_process *appProc = appThread->getProcess();
  assert(appProc);
  appProc->getThreads(threads);
  int active_threads = 11;
  threads.clear();
  while (((test3_threadCreateCounter < TEST3_THREADS)
         || (active_threads > 1)) 
         && (timeout < TIMEOUT)) {
    dprintf("%s[%d]: waiting for completion for test %d, num active threads = %d\n", 
            __FILE__, __LINE__, testno, active_threads);
    sleep_ms(SLEEP_INTERVAL/*ms*/);
    timeout += SLEEP_INTERVAL;
    if (appThread->isTerminated()) {
       fprintf(stderr, "%s[%d]:  BAD NEWS:  somehow the process died\n", __FILE__, __LINE__);
       err = 1;
       break;
    }
    bpatch->pollForStatusChange();
    if (appThread->isStopped()) {
       //  this should cause the test to fail, but for the moment we're
       //  just gonna continue it and complain
       fprintf(stderr, "%s[%d]:  BAD NEWS:  somehow the process stopped\n", __FILE__, __LINE__);
       appThread->continueExecution();
    }
    appProc->getThreads(threads);
    active_threads = threads.size();
    threads.clear();
  }

  if (timeout >= TIMEOUT) {
    FAIL(testno, testname);
    fprintf(stderr, "%s[%d]:  test timed out. got %d/10 events\n",
           __FILE__, __LINE__, test3_threadCreateCounter);
    err = 1;
  }

  dprintf("%s[%d]: ending test %d, num active threads = %d\n", 
            __FILE__, __LINE__, testno, active_threads);
  //sleep_ms(500);
  dprintf("%s[%d]:  stop execution for test %d\n", __FILE__, __LINE__, testno);
  appThread->stopExecution();

  //   read all tids from the mutatee and verify that we got them all
  unsigned long mutatee_tids[TEST3_THREADS];
  const char *threads_varname = NULL;
  if (testno == 3)
     threads_varname = "test3_threads";
  if (testno == 4)
     threads_varname = "test4_threads";
  assert(threads_varname);
  getVar(threads_varname, (void *) mutatee_tids, 
         (sizeof(unsigned long) * TEST3_THREADS),
         testno, testname);

  if (debugPrint) {
    fprintf(stderr, "%s[%d]:  read following tids for test%d from mutatee\n", __FILE__, __LINE__, testno);
    
    for (unsigned int i = 0; i < TEST3_THREADS; ++i) {
       fprintf(stderr, "\t%lu\n", mutatee_tids[i]);
    }
  }

  for (unsigned int i = 0; i < TEST3_THREADS; ++i) {
     bool found = false;
     for (unsigned int j = 0; j < callback_tids.size(); ++j) {
       if (callback_tids[j] == mutatee_tids[i]) {
         found = true;
         break;
       }
     }

    if (!found) {
      FAIL(testno, testname);
      fprintf(stderr, "%s[%d]:  could not find record for tid %lu: have these:\n",
             __FILE__, __LINE__, mutatee_tids[i]);
       for (unsigned int j = 0; j < callback_tids.size(); ++j) {
          fprintf(stderr, "%lu\n", callback_tids[j]);
       }
      err = true;
      break;
    }
  }

  dprintf("%s[%d]: removing thread callback\n", __FILE__, __LINE__);
  if (!bpatch->removeThreadEventCallback(BPatch_threadCreateEvent, createcb)) {
    FAIL(testno, testname);
    fprintf(stderr, "%s[%d]:  failed to remove thread callback\n",
           __FILE__, __LINE__);
    err = true;
  }

  if (!err)  {
    PASS(testno, testname);
    //sleep(5);
    //sleep_ms(200);
    return true;
  }
  return false;
}

bool mutatorTest3()
{
  return  mutatorTest3and4(TESTNO, TESTNAME);
}

#undef  TESTNO
#define TESTNO 4
#undef  TESTNAME
#define TESTNAME "thread create callback -- doa"

bool mutatorTest4()
{
#if 0
#if defined(os_linux) && defined(arch_x86)
  SKIP(TESTNO, TESTNAME);
#else
  return  mutatorTest3and4(TESTNO, TESTNAME);
#endif
#endif
  return  mutatorTest3and4(TESTNO, TESTNAME);
}

#undef  TESTNO
#define TESTNO 5 
#undef  TESTNAME
#define TESTNAME "thread exit callback"

int test5_threadDestroyCounter = 0;
void threadDestroyCB(BPatch_process * /*proc*/, BPatch_thread *thr)
{
  if (debugPrint)
    fprintf(stderr, "%s[%d]:  thread %lu destroy event for pid %d\n", 
            __FILE__, __LINE__, thr->getTid(), thr->getPid());
  test5_threadDestroyCounter++;
}

bool mutatorTest5and6(int testno, const char *testname)
{
  unsigned int timeout = 0; // in ms
  int err = 0;

  BPatchAsyncThreadEventCallback destroycb = threadDestroyCB;
  if (!bpatch->registerThreadEventCallback(BPatch_threadDestroyEvent, destroycb)) 
  {
    FAIL(testno, testname);
    fprintf(stderr, "%s[%d]:  failed to register thread callback\n",
           __FILE__, __LINE__);
    return false;
  }

  if (debugPrint)
    fprintf(stderr, "%s[%d]:  registered threadDestroy callback\n", 
            __FILE__, __LINE__);

  //  unset mutateeIdle to trigger thread (10) spawn.

  int zero = 0;
  setVar("mutateeIdle", (void *) &zero, testno, testname);
  appThread->continueExecution();

  //  wait until we have received the desired number of events
  //  (or timeout happens)
  while(test5_threadDestroyCounter < TEST5_THREADS && (timeout < TIMEOUT)) {
    sleep_ms(SLEEP_INTERVAL/*ms*/);
    timeout += SLEEP_INTERVAL; 
    fprintf(stderr, "%s[%d]:  polliing\n", __FILE__, __LINE__);
    bpatch->pollForStatusChange();
  }

  if (timeout >= TIMEOUT) {
    FAIL(testno, testname);
    fprintf(stderr, "%s[%d]:  test timed out.\n",
           __FILE__, __LINE__);
    err = 1;
  }

  appThread->stopExecution();

  if (!bpatch->removeThreadEventCallback(BPatch_threadDestroyEvent, destroycb)) {
    FAIL(testno, testname);
    fprintf(stderr, "%s[%d]:  failed to remove thread callback\n",
           __FILE__, __LINE__);
    return false;
  }

  if (!err) {
    PASS(testno, testname);
    return true;
  }
  return false;
}

bool mutatorTest5()
{
#if defined (os_none)
  return mutatorTest5and6(TESTNO, TESTNAME);
#else
  SKIP(TESTNO, TESTNAME);
  return true;
#endif
}

#undef  TESTNO
#define TESTNO 6 
#undef  TESTNAME
#define TESTNAME "thread exit callback -- doa"

bool mutatorTest6()
{
#if defined (os_none)
  return mutatorTest5and6(TESTNO, TESTNAME);
#else
  SKIP(TESTNO, TESTNAME);
  return true;
#endif
}

#undef  TESTNO
#define TESTNO 7 
#undef  TESTNAME
#define TESTNAME "user defined message callback -- st"

bool test7done = false;
bool test7err = false;
unsigned long test7_tids[TEST8_THREADS];

void test7cb(BPatch_process * /* proc*/, void *buf, unsigned int bufsize)
{
  static int callback_counter = 0;
  if (debugPrint)
    fprintf(stderr, "%s[%d]:  inside test7cb\n", __FILE__, __LINE__);

  if (bufsize != sizeof(user_msg_t)) {
    //  something is incredibly wrong
    fprintf(stderr, "%s[%d]:  unexpected message size %d not %d\n", 
            __FILE__, __LINE__, bufsize, sizeof(user_msg_t));
    test7err = true;
    return;
  }

  user_msg_t *msg = (user_msg_t *) buf;
  user_event_t what = msg->what;
  unsigned long tid = msg->tid;

  if (debugPrint)
    fprintf(stderr, "%s[%d]:  thread = %lu, what = %d\n", __FILE__, __LINE__, tid, what);

  if (callback_counter == 0) {
    //  we expect the entry point to be reported first
    if (what != func_entry) {
      fprintf(stderr, "%s[%d]:  unexpected message %d not %d\n", 
            __FILE__, __LINE__, (int) what, (int) func_entry);
      FAIL(TESTNO, TESTNAME);
      test7err = 1;
      return; 
    }
  } else if (callback_counter <= TEST7_NUMCALLS) {
    // we expect to get a bunch of function calls next
    if (what != func_callsite) {
      fprintf(stderr, "%s[%d]:  unexpected message %d not %d\n", 
            __FILE__, __LINE__, (int) what, (int) func_callsite);
      FAIL(TESTNO, TESTNAME);
      test7err = 1;
      return; 
    }
  }
  else if (callback_counter == (TEST7_NUMCALLS +1)) {
    // lastly comes the function exit
    if (what != func_exit) {
      fprintf(stderr, "%s[%d]:  unexpected message %d not %d\n", 
            __FILE__, __LINE__, (int) what, (int) func_exit);
      FAIL(TESTNO, TESTNAME);
      test7err = 1;
      return; 
    }
    // set test7done to end the test
    test7done = true;
  }
  callback_counter++;
}

bool mutatorTest7()
{
  //  a simple single threaded user messagin scenario where we want to send
  //  async messages at function entry/exit and call points.

  BPatchUserEventCallback cb = test7cb;
  if (!bpatch->registerUserEventCallback(cb)) {
    FAIL(TESTNO, TESTNAME);
    fprintf(stderr, "%s[%d]: could not register callback\n", __FILE__, __LINE__);
    return false;
  }

  //  instrument entry and exit of call7_1, as well as call points inside call7_1
  BPatch_function *call7_1 = findFunction("call7_1", TESTNO, TESTNAME);
  BPatch_point *entry = findPoint(call7_1, BPatch_entry,TESTNO, TESTNAME);
  BPatch_point *exit = findPoint(call7_1, BPatch_exit,TESTNO, TESTNAME);
  BPatch_point *callsite = findPoint(call7_1, BPatch_subroutine,TESTNO, TESTNAME);

  //  These are our asynchronous message functions (in libTest12) that we
  //  attach to the "interesting" points
  BPatch_function *reportEntry = findFunction("reportEntry", TESTNO, TESTNAME);
  BPatch_function *reportExit = findFunction("reportExit", TESTNO, TESTNAME);
  BPatch_function *reportCallsite = findFunction("reportCallsite", TESTNO, TESTNAME);

  //  Do the instrumentation
  BPatchSnippetHandle *entryHandle = at(entry, reportEntry, TESTNO, TESTNAME); 
  BPatchSnippetHandle *exitHandle = at(exit, reportExit, TESTNO, TESTNAME); 
  BPatchSnippetHandle *callsiteHandle = at(callsite, reportCallsite, TESTNO, TESTNAME); 


  if (debugPrint) {
     int one = 1;
     setVar("libraryDebug", (void *) &one, TESTNO, TESTNAME);
  }
 //  unset mutateeIdle to trigger mutatee to issue messages.

  int zero = 0;
  int timeout = 0;
  setVar("mutateeIdle", (void *) &zero, TESTNO, TESTNAME);
  appThread->getProcess()->continueExecution();

  //  wait until we have received the desired number of events
  //  (or timeout happens)
  while(!test7err && !test7done && (timeout < TIMEOUT)) {
    sleep_ms(SLEEP_INTERVAL/*ms*/);
    timeout += SLEEP_INTERVAL;
    bpatch->pollForStatusChange();
  }

  if (timeout >= TIMEOUT) {
    FAIL(TESTNO, TESTNAME);
    fprintf(stderr, "%s[%d]:  test timed out.\n",
           __FILE__, __LINE__);
    test7err = true;
  }

  appThread->getProcess()->stopExecution();

  if (!bpatch->removeUserEventCallback(test7cb)) {
    FAIL(TESTNO, TESTNAME);
    fprintf(stderr, "%s[%d]:  failed to remove callback\n",
           __FILE__, __LINE__);
    return false;
  }

  if (!test7err) {
    PASS(TESTNO, TESTNAME);
    return true;
  } 

  FAIL(TESTNO, TESTNAME);
  return false;
}

#undef  TESTNO
#define TESTNO 8 
#undef  TESTNAME
#define TESTNAME "user defined message callback -- mt"

bool test8done = false;
bool test8err = false;
unsigned long tids[TEST8_THREADS];
user_event_t last_event[TEST8_THREADS];

bool findThreadIndex(unsigned long tid, unsigned int &index)
{
 //  find the index with tid <tid>, if it exists, otherwise, the index of
 //  an empty slot.  If no empty slot, return false (fail);
  for (index = 0; index < TEST8_THREADS; ++index) {
    if (0 == tids[index]) {
      tids[index] = tid;
      if (debugPrint)
        fprintf(stderr, "%s[%d]:  giving new slot to thread id %lu\n",
                __FILE__, __LINE__, tid);
      return true;
    }
    if (tid == tids[index]) 
      return true;
  }
  return false;
}

void test8cb(BPatch_process * /*proc*/, void *buf, unsigned int bufsize)
{
  static int destroy_counter = 0;
  if (debugPrint)
    fprintf(stderr, "%s[%d]:  inside test8cb\n", __FILE__, __LINE__);

  if (bufsize != sizeof(user_msg_t)) {
    //  something is incredibly wrong
    fprintf(stderr, "%s[%d]:  unexpected message size %d not %d\n", 
            __FILE__, __LINE__, bufsize, sizeof(user_msg_t));
    test8err = true;
    return;
  }

  user_msg_t *msg = (user_msg_t *) buf;
  user_event_t what = msg->what;
  unsigned long tid = msg->tid;

  if (debugPrint)
    fprintf(stderr, "%s[%d]:  thread = %lu, what = %d\n", __FILE__, __LINE__, tid, what);
  unsigned int index;
  if (!findThreadIndex(tid, index)) {
    test8err = true;
    fprintf(stderr, "%s[%d]:  failed to find record for tid %lu (or empty slot)\n",
            __FILE__, __LINE__,tid);
    return;
  }

  if (debugPrint)
    fprintf(stderr, "%s[%d]:  thread id %lu: index %d\n", __FILE__, __LINE__, tid, index);
  if (last_event[index] != (what - 1)) {
    test8err = true;
    fprintf(stderr, "%s[%d]:  out of order messsage received for thread %lu, last = %d, now = %d\n",
           __FILE__, __LINE__, tid, last_event[index], what);
    return;
  }

  last_event[index] = what;
  
  if (what == mutex_destroy) 
     destroy_counter++;
  if (destroy_counter == TEST8_THREADS)
    test8done = true;
  //sleep_ms(10);
}

bool mutatorTest8()
{
  for (unsigned int i = 0; i < TEST8_THREADS; ++i) {
    tids[i] = 0;
    last_event[i] = null_event;
  }  
  //  instrument events having to do with mutex init, lock, unlock, destroy
  //  with messaging functions in libtest12.so
  BPatch_module *libpthread = appImage->findModule("libpthread",true);
  assert(libpthread);

  BPatch_function *mutInit = findFunction("createLock", TESTNO, TESTNAME);
  BPatch_point *mutInitPt = findPoint(mutInit, BPatch_entry,TESTNO, TESTNAME);
  BPatch_function *reportInit = findFunction("reportMutexInit", TESTNO, TESTNAME);
  BPatchSnippetHandle *initHandle = at(mutInitPt, reportInit, TESTNO, TESTNAME); 
  BPatch_function *mutDestroy = findFunction("destroyLock", TESTNO, TESTNAME);
  BPatch_point *mutDestroyPt = findPoint(mutDestroy, BPatch_entry,TESTNO, TESTNAME);
  BPatch_function *reportDestroy = findFunction("reportMutexDestroy", TESTNO, TESTNAME);
  BPatchSnippetHandle *destroyHandle = at(mutDestroyPt, reportDestroy, TESTNO, TESTNAME); 

  BPatch_function *mutLock = findFunction("lockLock", TESTNO, TESTNAME);
  BPatch_point *mutLockPt = findPoint(mutLock, BPatch_entry,TESTNO,TESTNAME);
  BPatch_function *reportLock = findFunction("reportMutexLock", TESTNO, TESTNAME);
  BPatchSnippetHandle *lockHandle = at(mutLockPt, reportLock, TESTNO, TESTNAME); 
  BPatch_function *mutUnlock = findFunction("unlockLock", TESTNO, TESTNAME);
  BPatch_point *mutUnlockPt = findPoint(mutUnlock, BPatch_entry,TESTNO,TESTNAME);
  BPatch_function *reportUnlock = findFunction("reportMutexUnlock", TESTNO, TESTNAME);
  BPatchSnippetHandle *unlockHandle = at(mutUnlockPt, reportUnlock, TESTNO, TESTNAME); 

  BPatchUserEventCallback cb = test8cb;
  if (!bpatch->registerUserEventCallback(cb)) {
    FAIL(TESTNO, TESTNAME);
    fprintf(stderr, "%s[%d]: could not register callback\n", __FILE__, __LINE__);
    return false;
  }

 //  unset mutateeIdle to trigger mutatee to issue messages.

  int zero = 0;
  int timeout = 0;
  setVar("mutateeIdle", (void *) &zero, TESTNO, TESTNAME);
  appThread->getProcess()->continueExecution();

  //  wait until we have received the desired number of events
  //  (or timeout happens)
  while(!test8err && !test8done && (timeout < TIMEOUT)) {
    sleep_ms(SLEEP_INTERVAL/*ms*/);
    timeout += SLEEP_INTERVAL;
    bpatch->pollForStatusChange();
  }

  if (timeout >= TIMEOUT) {
    FAIL(TESTNO, TESTNAME);
    fprintf(stderr, "%s[%d]:  test timed out. Took longer than %d ms\n",
           __FILE__, __LINE__, TIMEOUT);
    test8err = true;
  }

  appThread->getProcess()->stopExecution();

  if (!bpatch->removeUserEventCallback(test8cb)) {
    FAIL(TESTNO, TESTNAME);
    fprintf(stderr, "%s[%d]:  failed to remove callback\n",
           __FILE__, __LINE__);
    return false;
  }

  //  what happens here if we still have messages coming in, but no handler -- should
  //  check this?

  appThread->getProcess()->deleteSnippet(initHandle);
  appThread->getProcess()->deleteSnippet(destroyHandle);
  appThread->getProcess()->deleteSnippet(lockHandle);
  appThread->getProcess()->deleteSnippet(unlockHandle);
  if (!test8err) {
    PASS(TESTNO, TESTNAME);
    return true;
  }
  return false;

}


/*******************************************************************************/
/*******************************************************************************/
/*******************************************************************************/

int mutatorMAIN(char *pathname, bool useAttach)
{
        //char *dirName;

   // Create an instance of the BPatch library
    bpatch = new BPatch;

    // Force functions to be relocated
    if (forceRelocation) {
      bpatch->setForcedRelocation_NP(true);
    }

    // Register a callback function that prints any error messages
    bpatch->registerErrorCallback(errorFunc);

    //  set up argument vector for mutatee
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


    //  subtest1 does not have a mutatee, exactly, so deal with it here
    //  before we start the mutatee
    child_argv[n] = NULL;
    if (runTest[1]) passedTest[1] = mutatorTest1();

    if ( ! ( runTest[2] ||
             runTest[3] ||
             runTest[4] ||
             runTest[5] ||
             runTest[6] ||
             runTest[7] ||
             runTest[8] )) {
      if (passedTest[1])
         printf("All requested tests passed\n");
      else
         printf("** Failed test1 (rtlib spinlocks)\n");
     return 0;
    }

    // Start the mutatee
    printf("Starting \"%s\"\n", pathname);

    if (useAttach) {
        int pid = startNewProcessForAttach(pathname, child_argv);
        if (pid < 0) {
            printf("*ERROR*: unable to start tests due to error creating mutatee process\n");
            exit(-1);
        } else {
            dprintf("New mutatee process pid %d started; attaching...\n", pid);
        }
        sleep_ms(100); // let the mutatee catch its breath for a moment
        //P_sleep(1); // let the mutatee catch its breath for a moment
        appThread = bpatch->attachProcess(pathname, pid);
    } else {
        child_argv[n++] = const_cast<char *>("-attachrun");
        child_argv[n] = NULL;
        dprintf("%s[%d]:  starting mutatee %s for tests2 and higher\n", __FILE__, __LINE__, pathname);
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

    // load libtest12.so -- currently only used by subtest 5, but make it generally
    // available
    dprintf("%s[%d]:  loading test library: %s\n", __FILE__, __LINE__, TEST12_LIBNAME);
    if (!appThread->loadLibrary(TEST12_LIBNAME)) {
      fprintf(stderr, "%s[%d]:  failed to load library %s, cannot proceed\n", __FILE__, __LINE__,
              TEST12_LIBNAME);
      goto cleanup;
    }
    printf("\n");

    if (runTest[2]) passedTest[2] = mutatorTest2();
    if (runTest[3]) passedTest[3] = mutatorTest3();
    if (runTest[4]) passedTest[4] = mutatorTest4();
    if (runTest[5]) passedTest[5] = mutatorTest5();
    if (runTest[6]) passedTest[6] = mutatorTest6();
    if (runTest[7]) passedTest[7] = mutatorTest7();
    if (runTest[8]) passedTest[8] = mutatorTest8();

    cleanup:

     //  unset mutateeIdle -- should exit gracefully
     int zero = 0;
      setVar("mutateeIdle", (void *) &zero, -1, "general test12 infrastructure");

     if (appThread->isStopped())  {
         appThread->continueExecution();
      }
        

     while (!appThread->isTerminated()) {
        bpatch->waitForStatusChange();
     }

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
main(int argc, char *argv[])
{
    char libRTname[256];

    bool ABI_32 = false;
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

    updateSearchPaths(argv[0]);

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
#if defined(x86_64_unknown_linux2_4)
        } else if (!strcmp(argv[i], "-m32")) {
            ABI_32 = true;
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
    if (ABI_32 || strstr(mutateeName,"_m32")) {
        // patch up file names based on alternate ABI (as necessary)
        if (!strstr(mutateeName, "_m32")) strcat(mutateeName,"_m32");
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

