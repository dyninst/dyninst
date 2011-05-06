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
#include "test_util.h"

#define NUM_THREADS 5
#define TIMEOUT 20

BPatch bpatch;
BPatch_process *proc;
unsigned error = 0;
bool create_proc = true;
unsigned thread_count;
static char dyn_tids[NUM_THREADS];
static char deleted_tids[NUM_THREADS];
// We can get extra threads; add a layer of indirection. Yay.
static int our_tid_max = 0;
static char thread_mapping[NUM_THREADS];

static int deleted_threads;

bool debug_flag = false;
#define dprintf if (debug_flag) fprintf
#define NUM_FUNCS 6 
char initial_funcs[NUM_FUNCS][25] = {"init_func", "main", "_start", "__start", "__libc_start_main", "_lwp_start"};

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
   if (my_dyn_id == -1) return;

   if (my_proc != proc)
   {
       fprintf(stderr, "[%s:%u] - Got invalid process: %p vs %p\n", __FILE__, __LINE__, my_proc, proc);
      error = 1;
   }
   deleted_tids[my_dyn_id] = 1;
   deleted_threads++;
   dprintf(stderr, "%s[%d]:  leaving deadthr\n", __FILE__, __LINE__);
}

void newthr(BPatch_process *my_proc, BPatch_thread *thr)
{
   dprintf(stderr, "%s[%d]:  welcome to newthr, error = %d\n", __FILE__, __LINE__, error);

   if (create_proc && proc && (my_proc != proc) )
   {
      fprintf(stderr, "[%s:%u] - Got invalid process: %p vs %p\n", 
              __FILE__, __LINE__, my_proc, proc);
      error = 1;
   }

   if (thr->isDeadOnArrival()) {
      fprintf(stderr, "[%s:%u] - Got a dead on arival thread\n", 
              __FILE__, __LINE__);
      error = 1;
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
   if (!found_name)
   {
       // We can get unexpected threads with different initial functions; do not include
       // them (but don't consider it an error). If we don't walk the stack right, then
       // we won't have enough expected threads and so check it later.
      fprintf(stderr, "[%s:%d] - Thread %d has unexpected initial function '%s'; ignoring\n",
              __FILE__, __LINE__, thr->getBPatchID(), name);
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
      error = 1;
   }
   else
   {
      for (unsigned i=0; i<NUM_THREADS; i++)
         if (stack_addrs[i] == my_stack)
         {
            fprintf(stderr, "[%s:%d] - WARNING: Thread %d and %d share a stack at %lx\n",
                    __FILE__, __LINE__, my_dyn_id, i, my_stack);
            error = 1;
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
            fprintf(stderr, "[%s:%d] - WARNING: Thread %d and %d share a tid of %ld\n",
                    __FILE__, __LINE__, my_dyn_id, i, mytid);
            error = 1;
      }
   pthread_ids[my_dyn_id] = mytid;

   thread_count++;
   dyn_tids[my_dyn_id] = 1;
   dprintf(stderr, "%s[%d]:  leaving newthr: error = %d\n", __FILE__, __LINE__, error);
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
      args[1] = create_arg;
      proc = bpatch.processCreate(filename, (const char **) args);
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
      dprintf(stderr, "%s[%d]: started process, now attaching\n", __FILE__, __LINE__);
      proc = bpatch.processAttach(filename, pid);
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
          debug_flag = true;
      else if (!strcmp(argv[i], "-V")) {
         if (libRTname[0]) {
            fprintf (stdout, "DYNINSTAPI_RT_LIB=%s\n", libRTname);
            fflush(stdout);
         }
      }
      else {
          fprintf(stderr, "Usage: test13 [-V] [-verbose] [-attach]|[-mutatee <file>]\n");
          exit(-1);
      }
   }
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
   bpatch.registerThreadEventCallback(BPatch_threadDestroyEvent, deadthr);

   proc = getProcess();
   if (!proc)
      return -1;

   proc->continueExecution();

   // Wait for NUM_THREADS new thread callbacks to run
   while (thread_count < NUM_THREADS) {
       dprintf(stderr, "Going into waitForStatusChange; thread count %d, NUM_THREADS %d...\n", thread_count, NUM_THREADS);
      bpatch.waitForStatusChange();
      dprintf(stderr, "Back from waitForStatusChange...\n");
      if (proc->isTerminated()) {
         fprintf(stderr, "[%s:%d] - App exited early\n", __FILE__, __LINE__);
         error = 1;
         break;
      }
      if (num_attempts++ == TIMEOUT) {
         fprintf(stderr, "[%s:%d] - Timed out waiting for threads\n", 
                 __FILE__, __LINE__);
         fprintf(stderr, "[%s:%d] - Only have %u threads, expected %u!\n",
                 __FILE__, __LINE__, thread_count, NUM_THREADS);
         error = 1;
         break;
      }
      P_sleep(1);
   }

   dprintf(stderr, "%s[%d]:  done waiting for thread creations, error = %d\n", __FILE__, __LINE__, error);

   BPatch_Vector<BPatch_thread *> thrds;
   proc->getThreads(thrds);

   if (thrds.size() < NUM_THREADS)
   {
      fprintf(stderr, "[%s:%d] - Have %u threads, expected %u!\n",
              __FILE__, __LINE__, thrds.size(), NUM_THREADS);
      error = 1;
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

   if(error || missing_threads) {
      fprintf(stderr, "%s[%d]: ERROR during thread create stage, exiting\n", __FILE__, __LINE__);
      printf("*** Failed test #1 (Threading Callbacks)\n");
      if(proc && !proc->isTerminated())
         proc->terminateExecution();
      return -1;
   }

   // Update state to allow threads to exit
   upgrade_mutatee_state();

   dprintf(stderr, "%s[%d]:  Now waiting for application to terminate.\n", __FILE__, __LINE__);

   while (!proc->isTerminated())
      bpatch.waitForStatusChange();

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
         error = 1;
      }
   }

   if (deleted_threads != NUM_THREADS || !deleted_tids[0])
   {
      fprintf(stderr, "[%s:%d] - %d threads deleted at termination." 
           "  Expected %d\n", __FILE__, __LINE__, deleted_threads, NUM_THREADS);
      error = 1;
   }

   if (error)
   {
       printf("*** Failed test #1 (Threading Callbacks)\n");
   } else {
       printf("Passed test #1 (Threading Callbacks)\n");
       printf("All tests passed.\n");
       return 0;
   }
   return -1;
}
