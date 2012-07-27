/*
 * See the dyninst/COPYRIGHT file for copyright information.
 * 
 * We provide the Paradyn Tools (below described as "Paradyn")
 * on an AS IS basis, and do not warrant its validity or performance.
 * We reserve the right to update, modify, or discontinue this
 * software at any time.  We shall have no obligation to supply such
 * updates or modifications or any other form of support to you.
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

#include <BPatch.h>
#include <BPatch_process.h>
#include <BPatch_thread.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include "test_lib.h"

#define MAX_ARGS 32

#include "dyninst_comp.h"
class test_thread_8_Mutator : public DyninstMutator {
protected:
  char *filename;
  const char *args[MAX_ARGS];
  char *logfilename;
  BPatch *bpatch;
  unsigned failed_tests;

  BPatch_process *getProcess();
  int error_exit();
  int mutatorTest(BPatch *bpatch);

public:
  test_thread_8_Mutator();
  virtual bool hasCustomExecutionPath() { return true; }
  virtual test_results_t setup(ParameterDict &param);
  virtual test_results_t executeTest();
};
extern "C" DLLEXPORT TestMutator *test_thread_8_factory() {
  return new test_thread_8_Mutator();
}

test_thread_8_Mutator::test_thread_8_Mutator()
  : filename(NULL), logfilename(NULL), bpatch(NULL) {
}

#if defined(os_windows_test)
#include <windows.h>
#else
#include <unistd.h>
#endif
#define NUM_THREADS 5 // one controller, four workers
#define TIMEOUT 40

// static FILE *outlog = NULL;
// static FILE *errlog = NULL;
static BPatch_process *proc;
static unsigned thread_count;
static char dyn_tids[NUM_THREADS];
static long pthread_ids[NUM_THREADS];

static unsigned error15 = 0;
static bool create_proc = true;

static bool debug_flag = false;
#define dprintf if (debug_flag) fprintf

// Globals: create_proc, dyn_tids, error15, NUM_THREADS, pthread_ids, proc,
// thread_count
static void newthr(BPatch_process *my_proc, BPatch_thread *thr)
{
   dprintf(stderr, "%s[%d]:  welcome to newthr, error15 = %d\n", __FILE__, __LINE__, error15);
   unsigned my_dyn_id = thr->getBPatchID();

   if (create_proc && (my_proc != proc) && proc != NULL && my_proc != NULL)
   {
      logerror("[%s:%u] - Got invalid process\n", __FILE__, __LINE__);
      error15 = 1;
   }

   dprintf(stderr, "%s[%d]:  newthr: BPatchID = %d\n", __FILE__, __LINE__, my_dyn_id);

   //Check that BPatch id is unique
   if (my_dyn_id >= NUM_THREADS)
   {
      logerror("[%s:%d] - WARNING: Thread ID %d out of range\n",
              __FILE__, __LINE__, my_dyn_id);
      return;
   }
   if (dyn_tids[my_dyn_id])
   {
      logerror("[%s:%d] - WARNING: Thread %d called in callback twice\n",
              __FILE__, __LINE__, my_dyn_id);
      return;
   }
   dyn_tids[my_dyn_id] = 1;

   //Thread IDs should be unique
   long mytid = (long)(thr->getTid());
   if (mytid == -1)
   {
      logerror("[%s:%d] - WARNING: Thread %d has a tid of -1\n", 
              __FILE__, __LINE__, my_dyn_id);
   }
   dprintf(stderr, "%s[%d]:  newthr: tid = %lu\n", 
           __FILE__, __LINE__,  (unsigned long)mytid);
   for (unsigned i=0; i<NUM_THREADS; i++)
      if (i != my_dyn_id && dyn_tids[i] && mytid == pthread_ids[i])
      {
            logerror("[%s:%d] - WARNING: Thread %d and %d share a tid of %u\n",
                    __FILE__, __LINE__, my_dyn_id, i, mytid);
            error15 = 1;
      }
   pthread_ids[my_dyn_id] = mytid;

   thread_count++;
   dprintf(stderr, "%s[%d]:  leaving newthr: error15 = %d\n", __FILE__, __LINE__, error15);
}

BPatch_process *test_thread_8_Mutator::getProcess()
{
  int n = 0;
  args[n++] = filename;
  if (logfilename != NULL) {
    args[n++] = "-log";
    args[n++] = logfilename;
  }

  args[n++] = "-run";
  args[n++] = "test_thread_8";

   args[n] = NULL;

   BPatch_process *proc = NULL;
   if (create_proc) {
      proc = bpatch->processCreate(filename, (const char **) args);
      if(proc == NULL) {
         logerror("%s[%d]: processCreate(%s) failed\n", 
                 __FILE__, __LINE__, filename);
         return NULL;
      }
   }
   else {
#if defined(os_windows_test)
	P_sleep(1);
#endif
	// Should be attached by test infrastructure, but
	// we set delayedAttach so signal it here.
	if (!appProc) return NULL;

	proc = appProc;
   }
   return proc;
}

int test_thread_8_Mutator::error_exit()
{
   logerror("**Failed** %d tests\n", failed_tests);
   if(proc && !proc->isTerminated()) 
      proc->terminateExecution();
   return -1;
}

int test_thread_8_Mutator::mutatorTest(BPatch *bpatch)
{
   unsigned num_attempts = 0;
   bool missing_threads = false;
   thread_count = 0;
   memset(dyn_tids, 0, sizeof(dyn_tids));
   memset(pthread_ids, 0, sizeof(pthread_ids));
   failed_tests = 2;
   error15 = 0;

   proc = NULL;
   proc = getProcess();
   if (!proc)
      return error_exit();

   BPatch_image *img = proc->getImage();

   BPatch_Vector<BPatch_function *> syncfuncs;
   img->findFunction("check_sync", syncfuncs);
   if (syncfuncs.size() != 1) {
      logerror("ERROR: Didn't find 1 'check_sync' function\n");
      return error_exit();
   }
   BPatch_function *check_sync = syncfuncs[0];

   BPatch_Vector<BPatch_function *> asyncfuncs;
   img->findFunction("check_async", asyncfuncs);
   if (asyncfuncs.size() != 1) {
      logerror("ERROR: Didn't find 1 'check_async' function\n");
      return error_exit();
   }
   BPatch_function *check_async = asyncfuncs[0];   

  // For the attach case, we may already have the threads in existence; if so, 
   // manually trigger them here. 

   if (!create_proc) {
	   newthr(appProc, appThread);
	   std::vector<BPatch_thread *> threads;
	   appProc->getThreads(threads);
	   for (unsigned i = 0; i < threads.size(); ++i) {
		   if (threads[i] == appThread) continue;
		   newthr(appProc, threads[i]);
	   }
   }
   proc->continueExecution();

   // Wait for NUM_THREADS to be created
   while (thread_count < NUM_THREADS) {
      bpatch->waitForStatusChange();
      if (proc->isTerminated())
      {
         logerror("[%s:%d] - App exited early\n", __FILE__, __LINE__);
         return error_exit();
      }
      if (num_attempts++ == TIMEOUT)
      {
         logerror("[%s:%d] - Timed out waiting for threads\n", 
                 __FILE__, __LINE__);
         logerror("[%s:%d] - Only have %u threads, expected %u!\n",
              __FILE__, __LINE__, thread_count, NUM_THREADS);
         return error_exit();
      }
   }

   if( waitUntilStopped(bpatch, proc, 8, "test_thread_8: oneTimeCode") < 0 ){
       logerror("[%s:%d] - Failed to wait for stop of threads\n",
               __FILE__, __LINE__);
       return error_exit();
   }

   if( !proc->continueExecution() ) {
       logerror("[%s:%d] - failed to continue process after stop\n",
               FILE__, __LINE__);
       return error_exit();
   }

   dprintf(stderr, "%s[%d]:  done waiting for thread creations\n", 
           __FILE__, __LINE__);

   BPatch_Vector<BPatch_thread *> thrds;
   proc->getThreads(thrds);
   if (thrds.size() != NUM_THREADS)
      logerror("[%s:%d] - Have %u threads, expected %u!\n",
              __FILE__, __LINE__, thrds.size(), NUM_THREADS);
   for (unsigned i=0; i<NUM_THREADS; i++)
   {
      if (!dyn_tids[i]) {
         logerror("[%s:%d] - Thread %u was never created!\n",
                 __FILE__, __LINE__, i);
         missing_threads = true;
      }
   }
   if(missing_threads) {
      logerror("%s[%d]: ERROR during thread create stage, can not run test\n", __FILE__, __LINE__);
      return error_exit();
   }

   // asyncOneTimeCode each worker thread
   for (unsigned i=1; i<NUM_THREADS; i++)
   {
      if (dyn_tids[i])
      {
         long tid = pthread_ids[i];
         BPatch_thread *thr = proc->getThread((dynthread_t)(tid));
         if(thr == NULL) {
            logerror("%s[%d]: ERROR - can't find thread with tid %lu\n",
                    __FILE__, __LINE__, (unsigned long)tid);
            error15 = 1;
            continue;
         }
         BPatch_constExpr asyncVarExpr(tid);
         BPatch_Vector<BPatch_snippet *> args;
         args.push_back(&asyncVarExpr);
         BPatch_funcCallExpr call_check_async(*check_async, args);
         BPatch_Vector<BPatch_snippet *> async_code;
         async_code.push_back(&call_check_async);
         BPatch_sequence *code = new BPatch_sequence(async_code);
         dprintf(stderr, "%s[%d]: issuing oneTimeCodeAsync for tid %lu\n", __FILE__, __LINE__, tid);
         thr->oneTimeCodeAsync(*code);
      }
   }
   // FIXME We don't know if the test worked yet!  This printout is premature.
    if(!error15) {
       failed_tests--;
//       logerror("Passed test #1 (thread-specific oneTimeCodeAsync)\n");
    }

   // OneTimeCode each worker thread to allow it to exit
   for (unsigned i=1; i<NUM_THREADS; i++)
   {
      if (dyn_tids[i])
      {
         long tid = pthread_ids[i];
         BPatch_thread *thr = proc->getThread((dynthread_t)(tid));
         if(thr == NULL) {
            logerror("%s[%d]: ERROR - can't find thread with tid %lu\n",
                    __FILE__, __LINE__, (unsigned long)tid);
            error15 = 1;
            continue;
         }
         BPatch_constExpr syncVarExpr(pthread_ids[i]);
         BPatch_Vector<BPatch_snippet *> args;
	 args.push_back(&syncVarExpr);
         BPatch_funcCallExpr call_check_sync(*check_sync, args);
         BPatch_Vector<BPatch_snippet *> sync_code;
         sync_code.push_back(&call_check_sync);
         BPatch_sequence *code = new BPatch_sequence(sync_code);
         dprintf(stderr, "%s[%d]: issuing oneTimeCode for tid %lu\n", __FILE__, __LINE__, tid);
         proc->stopExecution();
         thr->oneTimeCode(*code);
         proc->continueExecution();
         dprintf(stderr, "%s[%d]: finished oneTimeCode for tid %lu\n", __FILE__, __LINE__, tid);
      }
   }
   // FIXME We don't know if the test worked yet!  This printout is premature.
    if(!error15) {
       failed_tests--;
//       logerror("Passed test #2 (thread-specific oneTimeCode)\n");
    }

   dprintf(stderr, "%s[%d]:  Now waiting for threads to die.\n", __FILE__, __LINE__);

   while (!proc->isTerminated())
      bpatch->waitForStatusChange();

   int exitCode = proc->getExitCode();
   // TODO Parse the exit code value so I know what to print..
   if ((0 == exitCode) && (0 == error15) && (0 == failed_tests)) {
     // TODO Print success message for both sync and async one time codes
     logerror("Passed test #1 (thread-specific asyncOneTimeCode)\n");
     logerror("Passed test #2 (thread-specific oneTimeCode)\n");
     logerror("Passed test_thread_8 (thread-specific one time codes)\n");
     return 0;
   } else {
     // TODO Figure out what went wrong and print a relevant error message
     logerror("**Failed test_thread_8 (thread-specific one time codes)\n");
	 if(exitCode) logerror("**Expected exit code = 0, exit code was %d\n", exitCode);
	 if(error15) logerror("**Expected error15 = 0, error15 was %d\n", error15);
	 if(failed_tests) logerror("**Expected failed tests = 0, failed tests was %d\n", failed_tests);

     return -1;
   }

//    if (error15 || failed_tests) 
//       return error_exit();

//    logerror("Test completed without errors\n");
//    return 0;
}

test_results_t test_thread_8_Mutator::executeTest() {
  memset(args, 0, sizeof (args));

   if (!bpatch->registerThreadEventCallback(BPatch_threadCreateEvent, newthr))
   {
      logerror("%s[%d]:  failed to register thread callback\n",
	      __FILE__, __LINE__);
      return FAILED;
   }
   
   int rv = mutatorTest(bpatch);

   if (!bpatch->removeThreadEventCallback(BPatch_threadCreateEvent, newthr))
   {
      logerror("%s[%d]:  failed to remove thread callback\n",
	      __FILE__, __LINE__);
      return FAILED;
   }

   if (0 == rv) {
     return PASSED;
   } else {
     return FAILED;
   }
}

// extern "C" TEST_DLL_EXPORT int test15_1_mutatorMAIN(ParameterDict &param)
test_results_t test_thread_8_Mutator::setup(ParameterDict &param) {
   /* Grab info from param */
   bpatch = (BPatch *)(param["bpatch"]->getPtr());
   filename = param["pathname"]->getString();
    appProc = (BPatch_process *)(param["appProcess"]->getPtr());
   if (appProc) appImage = appProc->getImage();
   // Get log file pointers
   logfilename = param["logfilename"]->getString();

   if ( param["createmode"]->getInt() != CREATE )
   {
      create_proc = false;
   } else {
     create_proc = true;
   }

   if( param["debugPrint"]->getInt() != 0 ) {
       debug_flag = true;
   }

   return DyninstMutator::setup(param);
}
