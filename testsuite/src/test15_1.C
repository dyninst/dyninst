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

#include <BPatch.h>
#include <BPatch_process.h>
#include <BPatch_thread.h>
#include <stdio.h>
#include <stdarg.h>
#include "test_lib.h"

#if defined(os_windows)
#include <windows.h>
#else
#include <unistd.h>
#endif
#define NUM_THREADS 5 // one controller, four workers
#define TIMEOUT 20

BPatch *bpatch;
BPatch_process *proc;
unsigned thread_count;
static char dyn_tids[NUM_THREADS];
static long pthread_ids[NUM_THREADS];
static char deleted_tids[NUM_THREADS];
static int deleted_threads;

unsigned error15 = 0;

bool debug_flag = false;
#define dprintf if (debug_flag) fprintf

#define NUM_FUNCS 6
char initial_funcs[NUM_FUNCS][25] = {"init_func", "main", "_start", "__start", "__libc_start_main", "mainCRTStartup"};

void newthr(BPatch_process *my_proc, BPatch_thread *thr)
{
   dprintf(stderr, "%s[%d]:  welcome to newthr, error15 = %d\n", __FILE__, __LINE__, error15);
   unsigned my_dyn_id = thr->getBPatchID();

   if (my_proc != proc)
   {
      fprintf(stderr, "[%s:%u] - Got invalid process\n", __FILE__, __LINE__);
      error15 = 1;
   }

   dprintf(stderr, "%s[%d]:  newthr: BPatchID = %d\n", __FILE__, __LINE__, my_dyn_id);
   //Check initial function
   static char name[1024];
   BPatch_function *f = thr->getInitialFunc();   
   if (f) f->getName(name, 1024);
   else strcpy(name, "<NONE>");

   int found_name = 0;
   for (unsigned i=0; i<NUM_FUNCS; i++)
      if (!strcmp(name, initial_funcs[i]))
      {
         found_name = 1;
         break;
      }
   dprintf(stderr, "%s[%d]:  newthr: %s\n", __FILE__, __LINE__, name);
#if !defined(os_windows)
   //Initial thread function detection is proving VERY difficult on Windows,
   //currently leaving disabled.
   if (!found_name)
   {
      fprintf(stderr, "[%s:%d] - Thread %d has '%s' as initial function\n",
              __FILE__, __LINE__, my_dyn_id, name);
      error15 = 1;
   }
#endif

   //Check that thread_id is unique
   if (my_dyn_id >= NUM_THREADS)
   {
      fprintf(stderr, "[%s:%d] - Thread ID %d out of range\n",
              __FILE__, __LINE__, my_dyn_id);
   }
   if (dyn_tids[my_dyn_id])
   {
      fprintf(stderr, "[%s:%d] - Thread %d called in callback twice\n",
              __FILE__, __LINE__, my_dyn_id);
      error15 = 1;
   }
   dyn_tids[my_dyn_id] = 1;

   //Stacks should be unique and non-zero
   static unsigned long stack_addrs[NUM_THREADS];
   unsigned long my_stack = thr->getStackTopAddr();
   if (!my_stack)
   {
      fprintf(stderr, "[%s:%d] - Thread %d has no stack\n",
              __FILE__, __LINE__, my_dyn_id);
      error15 = 1;
   }
   else
   {
      for (unsigned i=0; i<NUM_THREADS; i++)
         if (stack_addrs[i] == my_stack)
         {
            fprintf(stderr, "[%s:%d] - Thread %d and %d share a stack at %x\n",
                    __FILE__, __LINE__, my_dyn_id, i, my_stack);
            error15 = 1;
         }
   }
   stack_addrs[my_dyn_id] = my_stack;

   //Thread IDs should be unique
   long mytid = thr->getTid();
   if (mytid == -1)
   {
      fprintf(stderr, "[%s:%d] - Thread %d has a tid of -1\n", 
              __FILE__, __LINE__, my_dyn_id);
   }
   dprintf(stderr, "%s[%d]:  newthr: tid = %lu\n", 
           __FILE__, __LINE__,  (unsigned long)mytid);
   for (unsigned i=0; i<NUM_THREADS; i++)
      if (i != my_dyn_id && dyn_tids[i] && mytid == pthread_ids[i])
      {
            fprintf(stderr, "[%s:%d] - Thread %d and %d share a tid of %u\n",
                    __FILE__, __LINE__, my_dyn_id, i, mytid);
            error15 = 1;
      }
   pthread_ids[my_dyn_id] = mytid;

   thread_count++;
   dprintf(stderr, "%s[%d]:  leaving newthr: error15 = %d\n", __FILE__, __LINE__, error15);
}

#define MAX_ARGS 32
char *filename = "test15.mutatee_gcc";
bool should_exec = true;
char *args[MAX_ARGS];
unsigned num_args = 0; 

static BPatch_process *getProcess()
{
   args[0] = filename;
   BPatch_process *proc;
   if (should_exec)
      proc = bpatch->processCreate(filename, (const char **) args);
   else
   {
#if !defined(os_windows)
      int pid = startNewProcessForAttach(filename, (const char **) args);
      if (pid < 0)
      {
         fprintf(stderr, "%s ", filename);
         perror("couldn't be started");
         return NULL;
      }
      proc = bpatch->processAttach(filename, pid);      
#endif
   }
   return proc;
}

unsigned failed_tests = 2;
int error_exit()
{
   printf("**Failed** %d tests\n", failed_tests);
   return -1;
}

static int mutatorTest(BPatch *bpatch)
{
   unsigned num_attempts = 0;

   proc = getProcess();
   if (!proc)
   {
      fprintf(stderr, "ERROR: Couldn't create process for %s\n", filename);
      return error_exit();
   }

   BPatch_image *img = proc->getImage();

   BPatch_Vector<BPatch_function *> syncfuncs;
   img->findFunction("check_sync", syncfuncs);
   if (syncfuncs.size() != 1) {
      fprintf(stderr, "ERROR: Didn't find 1 'check_sync' function\n");
      return error_exit();
   }
   BPatch_function *check_sync = syncfuncs[0];

   BPatch_Vector<BPatch_function *> asyncfuncs;
   img->findFunction("check_async", asyncfuncs);
   if (asyncfuncs.size() != 1) {
      fprintf(stderr, "ERROR: Didn't find 1 'check_async' function\n");
      return error_exit();
   }
   BPatch_function *check_async = asyncfuncs[0];
   

   BPatch_variableExpr *sync_var, *async_var;
   sync_var = img->findVariable("sync_test");
   if(sync_var == NULL) {
      fprintf(stderr, "ERROR: Didn't find 'sync_test' variable\n");
      return error_exit();
   }
   async_var = img->findVariable("async_test");
   if(async_var == NULL) {
      fprintf(stderr, "ERROR: Didn't find 'async_test' variable\n");
      return error_exit();
   }

   BPatch_Vector<BPatch_thread *> orig_thrds;
   proc->getThreads(orig_thrds);
   if (!orig_thrds.size()) abort();
   for (unsigned i=0; i<orig_thrds.size(); i++) {
      newthr(proc, orig_thrds[i]);
   }

   proc->continueExecution();

   // Wait for NUM_THREADS to be created
   do {
      bpatch->waitForStatusChange();
      if (proc->isTerminated())
      {
         fprintf(stderr, "[%s:%d] - App exited early\n", __FILE__, __LINE__);
         return error_exit();
      }
      if (num_attempts++ == TIMEOUT)
      {
         fprintf(stderr, "[%s:%d] - Timed out waiting for threads\n", 
                 __FILE__, __LINE__);
         fprintf(stderr, "[%s:%d] - Only have %u threads, expected %u!\n",
              __FILE__, __LINE__, thread_count, NUM_THREADS);
         return error_exit();
      }
      P_sleep(1);
   } while (thread_count < NUM_THREADS);

   BPatch_Vector<BPatch_thread *> thrds;
   proc->getThreads(thrds);
   if (thrds.size() != NUM_THREADS)
   {
      fprintf(stderr, "[%s:%d] - Only have %u threads, expected %u!\n",
              __FILE__, __LINE__, thrds.size(), NUM_THREADS);
      for (unsigned i=0; i<NUM_THREADS; i++)
      {
         if (!dyn_tids[i])
         {
            fprintf(stderr, "[%s:%d] - Thread %u was never created!\n",
                    __FILE__, __LINE__, i);
         }
      }
      fprintf(stderr, "ERROR: Can not run test due to missing threads");
      return error_exit();
   }

   dprintf(stderr, "%s[%d]:  done waiting for thread creations\n", 
           __FILE__, __LINE__);

   // asyncOneTimeCode each worker thread
   for (unsigned i=1; i<NUM_THREADS; i++)
   {
      if (dyn_tids[i])
      {
         long tid = pthread_ids[i];
         BPatch_thread *thr = proc->getThread(tid);
         if(thr == NULL) {
            fprintf(stderr, "%s[%d]: ERROR - can't find thread with tid %lu\n",
                    __FILE__, __LINE__, (unsigned long)tid);
            error15 = 1;
            continue;
         }
         BPatch_constExpr *val = new BPatch_constExpr(tid);
         BPatch_arithExpr *set_async_test = 
            new BPatch_arithExpr(BPatch_assign, *async_var, *val);
         BPatch_Vector<BPatch_snippet *> args;
         BPatch_funcCallExpr call_check_async(*check_async, args);
         BPatch_Vector<BPatch_snippet *> async_code;
         async_code.push_back(set_async_test);
         async_code.push_back(&call_check_async);
         BPatch_sequence *code = new BPatch_sequence(async_code);
         dprintf(stderr, "%s[%d]: issuing oneTimeCodeAsync for tid %lu\n", __FILE__, __LINE__, tid);
         thr->oneTimeCodeAsync(*code);
      }
   }
   if(!error15) {
      failed_tests--;
      printf("Passed test #1 (thread-specific oneTimeCodeAsync)\n");
   }

   P_sleep(10);

   // OneTimeCode each worker thread to allow it to exit
   for (unsigned i=1; i<NUM_THREADS; i++)
   {
      if (dyn_tids[i])
      {
         long tid = pthread_ids[i];
         BPatch_thread *thr = proc->getThread(tid);
         if(thr == NULL) {
            fprintf(stderr, "%s[%d]: ERROR - can't find thread with tid %lu\n",
                    __FILE__, __LINE__, (unsigned long)tid);
            error15 = 1;
            continue;
         }
         BPatch_constExpr *val = new BPatch_constExpr(pthread_ids[i]);
         BPatch_arithExpr *set_sync_test = 
            new BPatch_arithExpr(BPatch_assign, *sync_var, *val);
         BPatch_Vector<BPatch_snippet *> args;
         BPatch_funcCallExpr call_check_sync(*check_sync, args);
         BPatch_Vector<BPatch_snippet *> sync_code;
         sync_code.push_back(set_sync_test);
         sync_code.push_back(&call_check_sync);
         BPatch_sequence *code = new BPatch_sequence(sync_code);
         dprintf(stderr, "%s[%d]: issuing oneTimeCode for tid %lu\n", __FILE__, __LINE__, tid);
         thr->oneTimeCode(*code);
         dprintf(stderr, "%s[%d]: finished oneTimeCode for tid %lu\n", __FILE__, __LINE__, tid);
      }
   }
   if(!error15) {
      failed_tests--;
      printf("Passed test #2 (thread-specific oneTimeCode)\n");
   }

   dprintf(stderr, "%s[%d]:  Now waiting for threads to die.\n", __FILE__, __LINE__);

   while (!proc->isTerminated())
      bpatch->waitForStatusChange();

   if (error15 || failed_tests) 
      return error_exit();

   printf("Test completed without errors\n");
   return 0;
}

extern "C" TEST_DLL_EXPORT int mutatorMAIN(ParameterDict &param)
{
   /* Grab info from param */
   bpatch = (BPatch *)(param["bpatch"]->getPtr());
   filename = param["pathname"]->getString();

#if defined(os_osf)
   printf("Skipped test #1 (thread-specific oneTimeCode)\n");
   printf("\t- Not implemented on this platform\n");
   return 0;
#endif

   if ( param["useAttach"]->getInt() != 0 )
   {
      should_exec = false;
   }

   if (!bpatch->registerThreadEventCallback(BPatch_threadCreateEvent, newthr))
   {
      fprintf(stderr, "%s[%d]:  failed to register thread callback\n",
	      __FILE__, __LINE__);
      return (-1);
   }
   
   int rv = mutatorTest(bpatch);

   if (!bpatch->removeThreadEventCallback(BPatch_threadCreateEvent, newthr))
   {
      fprintf(stderr, "%s[%d]:  failed to remove thread callback\n",
	      __FILE__, __LINE__);
      return (-1);
   }

   return rv;
}
