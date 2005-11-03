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
#include <unistd.h>
#include "test_util.h"

#define NUM_THREADS 5
#define TIMEOUT 20

BPatch bpatch;
BPatch_process *proc;
unsigned error;
unsigned thread_count;
static char dyn_tids[NUM_THREADS];

static char deleted_tids[NUM_THREADS];
static int deleted_threads;

bool debug_flag;
#define dprintf if (debug_flag) fprintf
#define NUM_FUNCS 4
char initial_funcs[NUM_FUNCS][25] = {"init_func", "main", "_start", "__start"};

void deadthr(BPatch_process *my_proc, BPatch_thread *thr)
{
   dprintf(stderr, "%s[%d]:  welcome to deadthr\n", __FILE__, __LINE__);
   if (!thr) {
     dprintf(stderr, "%s[%d]:  deadthr called without valid ptr to thr\n",
            __FILE__, __LINE__);
     return;
   }
   unsigned my_dyn_id = thr->getBPatchID();
   if (my_proc != proc)
   {
      fprintf(stderr, "[%s:%u] - Got invalid process\n", __FILE__, __LINE__);
      error = 1;
   }
   deleted_tids[my_dyn_id] = 1;
   deleted_threads++;
   dprintf(stderr, "%s[%d]:  leaving to deadthr\n", __FILE__, __LINE__);
}

void newthr(BPatch_process *my_proc, BPatch_thread *thr)
{
   dprintf(stderr, "%s[%d]:  welcome to newthr, error = %d\n", __FILE__, __LINE__, error);
   unsigned my_dyn_id = thr->getBPatchID();

   if (my_proc != proc)
   {
      fprintf(stderr, "[%s:%u] - Got invalid process\n", __FILE__, __LINE__);
      error = 1;
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
   if (!found_name)
   {
      fprintf(stderr, "[%s:%d] - Thread %d has '%s' as initial function\n",
              __FILE__, __LINE__, my_dyn_id, name);
      error = 1;
   }

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
      error = 1;
   }
   dyn_tids[my_dyn_id] = 1;

   //Stacks should be unique and non-zero
   static unsigned long stack_addrs[NUM_THREADS];
   unsigned long my_stack = thr->getStackTopAddr();
   if (!my_stack)
   {
      fprintf(stderr, "[%s:%d] - Thread %d has no stack\n",
              __FILE__, __LINE__, my_dyn_id);
      error = 1;
   }
   else
   {
      for (unsigned i=0; i<NUM_THREADS; i++)
         if (stack_addrs[i] == my_stack)
         {
            fprintf(stderr, "[%s:%d] - Thread %d and %d share a stack at %x\n",
                    __FILE__, __LINE__, my_dyn_id, i, my_stack);
            error = 1;
         }
   }
   stack_addrs[my_dyn_id] = my_stack;

   //Thread IDs should be unique
   static unsigned long pthread_ids[NUM_THREADS];
   unsigned mytid = thr->getTid();
   if (mytid == -1)
   {
      fprintf(stderr, "[%s:%d] - Thread %d has a tid of -1\n", 
              __FILE__, __LINE__, my_dyn_id);
   }
   dprintf(stderr, "%s[%d]:  newthr: tid = %lu\n", __FILE__, __LINE__,  mytid);
   for (unsigned i=0; i<NUM_THREADS; i++)
      if (i != my_dyn_id && dyn_tids[i] && mytid == pthread_ids[i])
      {
            fprintf(stderr, "[%s:%d] - Thread %d and %d share a tid of %u\n",
                    __FILE__, __LINE__, my_dyn_id, i, mytid);
            error = 1;
      }
   pthread_ids[my_dyn_id] = mytid;

   thread_count++;
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

   proc->oneTimeCode(*inc_var_assign);
   dprintf(stderr, "%s[%d]:  upgrade_mutatee_state: after oneTimeCode\n", __FILE__, __LINE__);
}

#define MAX_ARGS 32
char *filename = "test13.mutatee_gcc";
bool should_exec = true;
char *args[MAX_ARGS];
unsigned num_args = 0; 

static BPatch_process *getProcess()
{
   BPatch_process *proc;
   if (should_exec)
      proc = bpatch.processCreate(filename, (const char **) args);
   else
   {
      int pid = startNewProcessForAttach(filename, (const char **) args);
      if (pid < 0)
      {
         fprintf(stderr, "%s ", filename);
         perror("couldn't be started");
         return NULL;
      }
      proc = bpatch.processAttach(filename, pid);      
   }
   return proc;
}

static void parse_args(unsigned argc, char *argv[])
{
   unsigned i;
   args[0] = NULL;
   for (i=0; i<argc; i++)
   {
      if (strcmp(argv[i], "-attach") == 0)
      {
         should_exec = false;
      }
      else if (strcmp(argv[i], "-mutator") == 0)
      {
         if (++i == argc) break;
         filename = argv[i];
      }
      else
      {
         args[num_args++] = argv[i];
         args[num_args] = NULL;
      }
   }
}

int main(int argc, char *argv[])
{
   unsigned num_attempts = 0;
   parse_args(argc, argv);

   bpatch.registerThreadEventCallback(BPatch_threadCreateEvent, newthr);
   bpatch.registerThreadEventCallback(BPatch_threadDestroyEvent, deadthr);

   proc = getProcess();
   if (!proc)
   {
      fprintf(stderr, "Couldn't create process for %s\n", filename);
      return -1;
   }


   BPatch_Vector<BPatch_thread *> orig_thrds;
   proc->getThreads(orig_thrds);
   if (!orig_thrds.size()) abort();
   for (unsigned i=0; i<orig_thrds.size(); i++) {
      newthr(proc, orig_thrds[i]);
   }

   proc->continueExecution();

   do {
      bpatch.waitForStatusChange();
      if (proc->isTerminated())
      {
         fprintf(stderr, "[%s:%d] - App exited early\n", __FILE__, __LINE__);
         error = 1;
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
      sleep(1);
   } while (thread_count < NUM_THREADS);

   BPatch_Vector<BPatch_thread *> thrds;
   proc->getThreads(thrds);
   if (thrds.size() != NUM_THREADS)
   {
      fprintf(stderr, "[%s:%d] - Only have %u threads, expected %u!\n",
              __FILE__, __LINE__, thrds.size(), NUM_THREADS);
      error = 1;
   }

   for (unsigned i=0; i<NUM_THREADS; i++)
   {
      if (!dyn_tids[i])
      {
         fprintf(stderr, "[%s:%d] - Thread %u was never created!\n",
                 __FILE__, __LINE__, i);
         error = 1;
      }
   }

   dprintf(stderr, "%s[%d]:  done waiting for thread creations, error = %d\n", __FILE__, __LINE__, error);
   upgrade_mutatee_state();
   dprintf(stderr, "%s[%d]:  Now waiting for threads to die.\n", __FILE__, __LINE__);

   //Wait for n-1 threads to die
   do {
      bpatch.waitForStatusChange();
      if (proc->isTerminated())
      {
         fprintf(stderr, "[%s:%d] - App exited early\n", __FILE__, __LINE__);
         error = 1;
         return -1;
      }
      if (num_attempts++ == TIMEOUT)
      {
         fprintf(stderr, "[%s:%d] - Timed out while deleting threads\n", 
                 __FILE__, __LINE__);
         break;
      }
      sleep(1);
   } while (deleted_threads < NUM_THREADS-1);

   for (unsigned i=1; i<NUM_THREADS; i++)
   {
      if (!deleted_tids[i])
      {
         fprintf(stderr, "[%s:%d] - Thread %d wasn't deleted\n",
                 __FILE__, __LINE__, i);
         error = 1;
      }
   }
   if (deleted_tids[0])
   {
      fprintf(stderr, "[%s:%d] - Prematurely deleted thread 0\n",
              __FILE__, __LINE__);
      error = 1;
   }

   upgrade_mutatee_state();

   do {
      bpatch.waitForStatusChange();
   } while (!proc->isTerminated());

   if (deleted_threads != NUM_THREADS || !deleted_tids[0])
   {
      fprintf(stderr, "[%s:%d] - %d threads deleted at termination." 
           "  Expected %d\n", __FILE__, __LINE__, deleted_threads, NUM_THREADS);
      error = 1;
   }


   if (!error)
   {
      printf("Test completed without errors\n");
      return 0;
   }
   return -1;
}
