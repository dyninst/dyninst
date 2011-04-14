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

#include <BPatch.h>
#include <BPatch_process.h>
#include <BPatch_thread.h>
#include <stdio.h>
#include <unistd.h>
#include <stdarg.h>
#include "test_util.h"

#define NUM_THREADS 5 // one controller, four workers
#define TIMEOUT 20

BPatch bpatch;
BPatch_process *proc;
unsigned error;
bool create_proc = true;
unsigned thread_count;
static char dyn_tids[NUM_THREADS];
static long pthread_ids[NUM_THREADS];
static char deleted_tids[NUM_THREADS];

bool debug_flag = false;
#define dprintf if (debug_flag) fprintf

void newthr(BPatch_process *my_proc, BPatch_thread *thr)
{
   unsigned my_dyn_id = thr->getBPatchID();

   if (create_proc && (my_proc != proc))
   {
      fprintf(stderr, "[%s:%u] - Got invalid process\n", __FILE__, __LINE__);
      error = 1;
   }

   dprintf(stderr, "[%s] New Thread: BPatchID = %d\n", __FILE__, my_dyn_id);

   //Check that BPatch id is unique
   if (my_dyn_id >= NUM_THREADS)
   {
      fprintf(stderr, "[%s:%d] - WARNING: Thread ID %d out of range\n",
              __FILE__, __LINE__, my_dyn_id);
      return;
   }
   if (dyn_tids[my_dyn_id])
   {
      fprintf(stderr, "[%s:%d] - WARNING: Thread %d called in callback twice\n",
              __FILE__, __LINE__, my_dyn_id);
      return;
   }
   dyn_tids[my_dyn_id] = 1;

   //Thread IDs should be unique
   long mytid = thr->getTid();
   if (mytid == -1)
   {
      fprintf(stderr, "[%s:%d] - WARNING: Thread %d has a tid of -1\n", 
              __FILE__, __LINE__, my_dyn_id);
   }
   dprintf(stderr, "[%s]           : tid = %lu\n", 
           __FILE__, (unsigned long)mytid);
   for (unsigned i=0; i<NUM_THREADS; i++)
      if (i != my_dyn_id && dyn_tids[i] && mytid == pthread_ids[i])
      {
         unsigned long my_stack = thr->getStackTopAddr();
            fprintf(stderr, "[%s:%d] - WARNING: Thread %d and %d share a tid of %lu, stack is 0x%lx\n",
                    __FILE__, __LINE__, my_dyn_id, i, (unsigned long)mytid, my_stack);
            error = 1;
      }
   pthread_ids[my_dyn_id] = mytid;
   thread_count++;
}

#define MAX_ARGS 32
char *filename = "test15.mutatee_gcc";
char *args[MAX_ARGS];
char *create_arg = "-create";
unsigned num_args = 0; 

static BPatch_process *getProcess()
{
   args[0] = filename;
   BPatch_process *proc;
   if (create_proc) {
      args[1] = create_arg;
      proc = bpatch.processCreate(filename, (const char **) args);
      if(proc == NULL) {
         fprintf(stderr, "%s[%d]: processCreate(%s) failed\n", 
                 __FILE__, __LINE__, filename);
         return NULL;
      }
   }
   else {
      int pid = startNewProcessForAttach(filename, (const char **) args);
      if (pid < 0)
      {
         fprintf(stderr, "%s ", filename);
         perror("couldn't be started");
         return NULL;
      }
      proc = bpatch.processAttach(filename, pid);
      if(proc == NULL) {
         fprintf(stderr, "%s[%d]: processAttach(%s, %d) failed\n", 
                 __FILE__, __LINE__, filename, pid);
         return NULL;
      }
      BPatch_image *appimg = proc->getImage();
      signalAttached(NULL, appimg);
   }
   return proc;
}

char libRTname[256];
static void parse_args(unsigned argc, char *argv[])
{
   unsigned i;
   args[0] = NULL;
   for (i=1; i<argc; i++)
   {
      if (strcmp(argv[i], "-attach") == 0)
      {
         create_proc = false;
      }
      else if (strcmp(argv[i], "-mutatee") == 0)
      {
         if (++i == argc) break;
         filename = argv[i];
      }
      else if (strcmp(argv[i], "-verbose") == 0)
      {
         debug_flag = true;
      }
      else if (!strcmp(argv[i], "-V")) {
         if (libRTname[0]) {
            fprintf (stdout, "DYNINSTAPI_RT_LIB=%s\n", libRTname);
            fflush(stdout);
         }
      }
      else
      {
         fprintf(stderr, "Usage: test15 [-V] [-verbose] [-attach]|[-mutatee <file>]\n");
         exit(-1);
      }
   }
}

unsigned failed_tests = 2;
void error_exit()
{
   printf("**Failed** %d tests\n", failed_tests);
   if(proc && !proc->isTerminated()) 
      proc->terminateExecution();
   exit(-1);
}

int main(int argc, char *argv[])
{
   unsigned num_attempts = 0;
   bool missing_threads = false;
   
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

   parse_args(argc, argv);

   bpatch.registerThreadEventCallback(BPatch_threadCreateEvent, newthr);

   proc = getProcess();
   if (!proc)
      error_exit();

   BPatch_image *img = proc->getImage();

   BPatch_Vector<BPatch_function *> syncfuncs;
   img->findFunction("check_sync", syncfuncs);
   if (syncfuncs.size() != 1) {
      fprintf(stderr, "ERROR: Didn't find 1 'check_sync' function\n");
      error_exit();
   }
   BPatch_function *check_sync = syncfuncs[0];

   BPatch_Vector<BPatch_function *> asyncfuncs;
   img->findFunction("check_async", asyncfuncs);
   if (asyncfuncs.size() != 1) {
      fprintf(stderr, "ERROR: Didn't find 1 'check_async' function\n");
      error_exit();
   }
   BPatch_function *check_async = asyncfuncs[0];
   

   BPatch_variableExpr *sync_var, *async_var;
   sync_var = img->findVariable("sync_test");
   if(sync_var == NULL) {
      fprintf(stderr, "ERROR: Didn't find 'sync_test' variable\n");
      error_exit();
   }
   async_var = img->findVariable("async_test");
   if(async_var == NULL) {
      fprintf(stderr, "ERROR: Didn't find 'async_test' variable\n");
      error_exit();
   }

   proc->continueExecution();

   // Wait for NUM_THREADS new thread callbacks to run
   while (thread_count < NUM_THREADS) {
      bpatch.waitForStatusChange();
      if (proc->isTerminated()) {
         fprintf(stderr, "[%s:%d] - App exited early\n", __FILE__, __LINE__);
         error_exit();
      }
      if (num_attempts++ == TIMEOUT) {
         fprintf(stderr, "[%s:%d] - Timed out waiting for threads\n", 
                 __FILE__, __LINE__);
         fprintf(stderr, "[%s:%d] - Only have %u threads, expected %u!\n",
                 __FILE__, __LINE__, thread_count, NUM_THREADS);
         error_exit();
      }
      sleep(1);
   }

   BPatch_Vector<BPatch_thread *> thrds;
   proc->getThreads(thrds);
   if (thrds.size() != NUM_THREADS)
      fprintf(stderr, "[%s:%d] - Have %u threads, expected %u!\n",
              __FILE__, __LINE__, thrds.size(), NUM_THREADS);

   for (unsigned i=0; i<NUM_THREADS; i++)
   {
      if (!dyn_tids[i]) {
         fprintf(stderr, "[%s:%d] - Thread %u was never created!\n",
                 __FILE__, __LINE__, i);
         missing_threads = true;
      }
   }
   if(missing_threads) {
      fprintf(stderr, "ERROR: Can not run test due to missing threads\n");
      error_exit();
   }


   dprintf(stderr, "%s[%d]:  done waiting for thread creations\n", 
           __FILE__, __LINE__);

   // asyncOneTimeCode each worker thread
   for (unsigned i=1; i<NUM_THREADS; i++)
   {
      if (dyn_tids[i] && !deleted_tids[i])
      {
         long tid = pthread_ids[i];
         BPatch_thread *thr = proc->getThread(tid);
         if(thr == NULL) {
            fprintf(stderr, "%s[%d]: ERROR - can't find thread with tid %lu\n",
                    __FILE__, __LINE__, (unsigned long)tid);
            error = 1;
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
         dprintf(stderr, "%s[%d]: issuing oneTimeCodeAsync for tid %lu\n", 
	         __FILE__, __LINE__, (unsigned long)tid);
         thr->oneTimeCodeAsync(*code);
      }
   }
   if(!error) {
      failed_tests--;
      printf("Passed test #1 (thread-specific oneTimeCodeAsync)\n");
   }

   sleep(10);

   // OneTimeCode each worker thread to allow it to exit
   for (unsigned i=1; i<NUM_THREADS; i++)
   {
      if (dyn_tids[i] && !deleted_tids[i])
      {
         long tid = pthread_ids[i];
         BPatch_thread *thr = proc->getThread(tid);
         if(thr == NULL) {
            fprintf(stderr, "%s[%d]: ERROR - can't find thread with tid %lu\n",
                    __FILE__, __LINE__, (unsigned long)tid);
            error = 1;
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
         dprintf(stderr, "%s[%d]: issuing oneTimeCode for tid %lu\n", 
	         __FILE__, __LINE__, (unsigned long)tid);
         thr->oneTimeCode(*code);
         dprintf(stderr, "%s[%d]: finished oneTimeCode for tid %lu\n", 
	         __FILE__, __LINE__, (unsigned long)tid);
      }
   }
   if(!error) {
      failed_tests--;
      printf("Passed test #2 (thread-specific oneTimeCode)\n");
   }

   dprintf(stderr, "%s[%d]:  Now waiting for application to terminate.\n", __FILE__, __LINE__);

   while (!proc->isTerminated())
      bpatch.waitForStatusChange();

   if (error) 
      error_exit();

   printf("All tests passed.\n");
   return 0;
}
