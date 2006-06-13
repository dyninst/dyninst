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
#include <BPatch_function.h>
#include "test_lib.h"

#define NUM_THREADS 5
#define TIMEOUT 20

BPatch *bpatch;
BPatch_process *proc;
unsigned error13 = 0;
unsigned thread_count;
static char dyn_tids[NUM_THREADS];
static char deleted_tids[NUM_THREADS];
// We can get extra threads; add a layer of indirection. Yay.
static int our_tid_max = 0;
static char thread_mapping[NUM_THREADS];
static int deleted_threads;
bool create_proc = true;

bool debug_flag = false;
#define dprintf if (debug_flag) fprintf

#define NUM_FUNCS 6
char initial_funcs[NUM_FUNCS][25] = {"init_func", "main", "_start", "__start", "__libc_start_main", "mainCRTStartup"};

int bpindex_to_myindex(int index) {
    for (unsigned i = 0; i < our_tid_max; i++) {
        if (thread_mapping[i] == index) return i;
    }
    return -1;
}

void deadthr(BPatch_process *my_proc, BPatch_thread *thr)
{
   dprintf(stderr, "%s[%d]:  welcome to deadthr\n", __FILE__, __LINE__);
   if (!thr) {
     dprintf(stderr, "%s[%d]:  deadthr called without valid ptr to thr\n",
            __FILE__, __LINE__);
     return;
   }
   unsigned my_dyn_id = bpindex_to_myindex(thr->getBPatchID());
   if (-1 == my_dyn_id) {
      return;
   }

   if (my_proc != proc)
   {
      fprintf(stderr, "[%s:%u] - Got invalid process: %p vs %p\n", __FILE__,
	      __LINE__, my_proc, proc);
      error13 = 1;
   }
   deleted_tids[my_dyn_id] = 1;
   deleted_threads++;
   dprintf(stderr, "%s[%d]:  leaving to deadthr, %d is dead\n", __FILE__, __LINE__, my_dyn_id);
}

void newthr(BPatch_process *my_proc, BPatch_thread *thr)
{
   dprintf(stderr, "%s[%d]:  welcome to newthr, error13 = %d\n", __FILE__, __LINE__, error13);

   if (my_proc != proc)
   {
      fprintf(stderr, "[%s:%u] - Got invalid process: %p vs %p\n", 
              __FILE__, __LINE__, my_proc, proc);
      error13 = 1;
   }

   if (thr->isDeadOnArrival()) {
      fprintf(stderr, "[%s:%u] - Got a dead on arival thread\n", 
              __FILE__, __LINE__);
      error13 = 1;
      return;
   }

   dprintf(stderr, "%s[%d]:  newthr: BPatchID = %d\n", __FILE__, __LINE__, thr->getBPatchID());
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

   //Initial thread function detection is proving VERY difficult on Windows,
   //currently leaving disabled.
   if (!found_name)
   {
       // We can get unexpected threads with different initial functions; do not include
       // them (but don't consider it an error). If we don't walk the stack right, then
       // we won't have enough expected threads and so check it later.
      fprintf(stderr, "[%s:%d] - Thread %d has unexpected initial function '%s'; ignoring\n",
              __FILE__, __LINE__, thr->getBPatchID(), name);
      //      error13 = 1; // This shouldn't be an error, according to the comment above.
      return;
   }

   if (bpindex_to_myindex(thr->getBPatchID()) != -1) {
      fprintf(stderr, "[%s:%d] - WARNING: Thread %d called in callback twice\n",
              __FILE__, __LINE__, thr->getBPatchID());
      return;
   }

   unsigned my_dyn_id = our_tid_max; our_tid_max++;

   thread_mapping[my_dyn_id] = thr->getBPatchID();

   //Stacks should be unique and non-zero
   static unsigned long stack_addrs[NUM_THREADS];
   unsigned long my_stack = thr->getStackTopAddr();
   if (!my_stack)
   {
      fprintf(stderr, "[%s:%d] - WARNING: Thread %d has no stack\n",
              __FILE__, __LINE__, my_dyn_id);
      error13 = 1;
   }
   else
   {
      for (unsigned i=0; i<NUM_THREADS; i++)
         if (stack_addrs[i] == my_stack)
         {
            fprintf(stderr, "[%s:%d] - WARNING: Thread %d and %d share a stack at %lx\n",
                    __FILE__, __LINE__, my_dyn_id, i, my_stack);
            error13 = 1;
         }
   }
   stack_addrs[my_dyn_id] = my_stack;

   //Thread IDs should be unique
   static long pthread_ids[NUM_THREADS];
   long mytid = thr->getTid();
   if (mytid == -1)
   {
      fprintf(stderr, "[%s:%d] - WARNING: Thread %d has a tid of -1\n", 
              __FILE__, __LINE__, my_dyn_id);
   }
   dprintf(stderr, "%s[%d]:  newthr: tid = %lu\n", 
           __FILE__, __LINE__,  (unsigned long)mytid);
   for (unsigned i=0; i<NUM_THREADS; i++)
      if (i != my_dyn_id && dyn_tids[i] && mytid == pthread_ids[i])
      {
            fprintf(stderr, "[%s:%d] - WARNING: Thread %d and %d share a tid of %lu\n",
                    __FILE__, __LINE__, my_dyn_id, i, mytid);
            error13 = 1;
      }
   pthread_ids[my_dyn_id] = mytid;

   thread_count++;
   dyn_tids[my_dyn_id] = 1;
   dprintf(stderr, "%s[%d]:  leaving newthr: error13 = %d\n", __FILE__, __LINE__, error13);
}

void upgrade_mutatee_state()
{
   dprintf(stderr, "%s[%d]:  welcome to upgrade_mutatee_state\n", __FILE__, __LINE__);
   BPatch_variableExpr *var;
   BPatch_constExpr *one;
   BPatch_arithExpr *inc_var;
   BPatch_arithExpr *inc_var_assign;

   BPatch_image *img = proc->getImage();
   var = img->findVariable("proc_current_state");
   one = new BPatch_constExpr(1);
   inc_var = new BPatch_arithExpr(BPatch_plus, *var, *one);
   inc_var_assign = new BPatch_arithExpr(BPatch_assign, *var, *inc_var);
   dprintf(stderr, "%s[%d]: going into oneTimecode...\n", __FILE__, __LINE__);
   proc->oneTimeCode(*inc_var_assign);
   dprintf(stderr, "%s[%d]:  upgrade_mutatee_state: after oneTimeCode\n", __FILE__, __LINE__);
}

#define MAX_ARGS 32
char *filename = "test13.mutatee_gcc";
char *args[MAX_ARGS];
char *create_arg = "-create";
unsigned num_args = 0; 

static BPatch_process *getProcess()
{
   args[0] = filename;

   BPatch_process *proc;
   if (create_proc) {
      args[1] = create_arg; // I don't think this does anything.
      proc = bpatch->processCreate(filename, (const char **) args);
      if(proc == NULL) {
         fprintf(stderr, "%s[%d]: processCreate(%s) failed\n", 
                 __FILE__, __LINE__, filename);
         return NULL;
      }
   }
   else
   {
      dprintf(stderr, "%s[%d]: starting process for attach\n", __FILE__, __LINE__);
      int pid = startNewProcessForAttach(filename, (const char **) args);
      if (pid < 0)
      {
         fprintf(stderr, "%s ", filename);
         perror("couldn't be started");
         return NULL;
      }
#if defined(os_windows)
      P_sleep(1);
#endif
      dprintf(stderr, "%s[%d]: started process, now attaching\n", __FILE__, __LINE__);
      proc = bpatch->processAttach(filename, pid);  
      if(proc == NULL) {
         fprintf(stderr, "%s[%d]: processAttach(%s, %d) failed\n", 
                 __FILE__, __LINE__, filename, pid);
         return NULL;
      }
      dprintf(stderr, "%s[%d]: attached to process\n", __FILE__, __LINE__);
      BPatch_image *appimg = proc->getImage();
      signalAttached(NULL, appimg);    
   }
   return proc;
}

static int mutatorTest(BPatch *bpatch)
{
   unsigned num_attempts = 0;
   bool missing_threads = false;

   proc = getProcess();
   if (!proc)
      return -1;

   proc->continueExecution();

   // Wait for NUM_THREADS new thread callbacks to run
   while (thread_count < NUM_THREADS) {
      dprintf(stderr, "Going into waitForStatusChange...\n");
      bpatch->waitForStatusChange();
      dprintf(stderr, "Back from waitForStatusChange...\n");
      if (proc->isTerminated())
      {
         fprintf(stderr, "[%s:%d] - App exited early\n", __FILE__, __LINE__);
         error13 = 1;
         break;
      }
      if (num_attempts++ == TIMEOUT)
      {
         fprintf(stderr, "[%s:%d] - Timed out waiting for threads\n", 
                 __FILE__, __LINE__);
         fprintf(stderr, "[%s:%d] - Only have %u threads, expected %u!\n",
              __FILE__, __LINE__, thread_count, NUM_THREADS);
         return -1;
      }
      P_sleep(1);
   }

   dprintf(stderr, "%s[%d]:  done waiting for thread creations, error13 = %d\n", __FILE__, __LINE__, error13);

   BPatch_Vector<BPatch_thread *> thrds;
   proc->getThreads(thrds);
   if (thrds.size() != NUM_THREADS)
   {
      fprintf(stderr, "[%s:%d] - Have %u threads, expected %u!\n",
              __FILE__, __LINE__, thrds.size(), NUM_THREADS);
      error13 = 1;
   }

   for (unsigned i=0; i<NUM_THREADS; i++)
   {
      if (!dyn_tids[i])
      {
         fprintf(stderr, "[%s:%d] - Thread %u was never created!\n",
                 __FILE__, __LINE__, i);
         missing_threads = true;
      }
   }
   if(error13 || missing_threads) {
      fprintf(stderr, "%s[%d]: ERROR during thread create stage, exiting\n", __FILE__, __LINE__);
      printf("*** Failed test #1 (Threading Callbacks)\n");
      if(proc && !proc->isTerminated())
         proc->terminateExecution();
      return -1;
   }

   upgrade_mutatee_state();
   dprintf(stderr, "%s[%d]:  Now waiting for application to exit.\n", __FILE__, __LINE__);

   while (!proc->isTerminated())
      bpatch->waitForStatusChange();

   num_attempts = 0;
   while(deleted_threads != NUM_THREADS && num_attempts != TIMEOUT) {
      num_attempts++;
      P_sleep(1);
   }

   for (unsigned i=1; i<NUM_THREADS; i++)
   {
      if (!deleted_tids[i])
      {
         fprintf(stderr, "[%s:%d] - Thread %d wasn't deleted\n",
                 __FILE__, __LINE__, i);
         error13 = 1;
      }
   }

   if (deleted_threads != NUM_THREADS || !deleted_tids[0])
   {
      fprintf(stderr, "[%s:%d] - %d threads deleted at termination." 
           "  Expected %d\n", __FILE__, __LINE__, deleted_threads, NUM_THREADS);
      error13 = 1;
   }


   if (error13)
   {
       printf("*** Failed test #1 (Threading Callbacks)\n");
   } else {
       printf("Passed test #1 (Threading Callbacks)\n");
       printf("Test completed without errors\n");
       return 0;
   }
   return -1;
}

extern "C" TEST_DLL_EXPORT int mutatorMAIN(ParameterDict &param)
{
   /* Grab info from param */
   bpatch = (BPatch *)(param["bpatch"]->getPtr());
   filename = param["pathname"]->getString();

#if defined(os_osf)
   printf("Skipped test #1 (Threading Callbacks)\n");
   printf("\t- Not implemented on this platform\n");
   return 0;
#endif

   if ( param["useAttach"]->getInt() != 0 )
   {
      create_proc = false;
   }

   if (!bpatch->registerThreadEventCallback(BPatch_threadCreateEvent,
					    newthr) ||
       !bpatch->registerThreadEventCallback(BPatch_threadDestroyEvent,
					    deadthr))
   {
      fprintf(stderr, "%s[%d]:  failed to register thread callback\n",
	      __FILE__, __LINE__);
      return -1;
   }

   int rv = mutatorTest(bpatch);

   if (!bpatch->removeThreadEventCallback(BPatch_threadCreateEvent, newthr) ||
       !bpatch->removeThreadEventCallback(BPatch_threadDestroyEvent, deadthr))
   {
      fprintf(stderr, "%s[%d]:  failed to remove thread callback\n",
	      __FILE__, __LINE__);
      return -1;
   }

   return rv;
}
