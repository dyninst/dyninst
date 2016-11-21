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
#include <BPatch_function.h>
#include "test_lib.h"

#include "dyninst_comp.h"
class test_thread_6_Mutator : public DyninstMutator {
protected:
  char *logfilename;
  BPatch *bpatch;
  bool create_proc;

  void upgrade_mutatee_state();
  BPatch_process *getProcess();
  test_results_t mutatorTest(BPatch *bpatch);

public:
  test_thread_6_Mutator();
  virtual bool hasCustomExecutionPath() { return true; }
  virtual test_results_t setup(ParameterDict &param);
  virtual test_results_t executeTest();
};
extern "C" DLLEXPORT TestMutator *test_thread_6_factory() {
  return new test_thread_6_Mutator();
}

test_thread_6_Mutator::test_thread_6_Mutator()
  : logfilename(NULL), bpatch(NULL), create_proc(true) {
}

#define NUM_THREADS 5
#define TIMEOUT 20

static BPatch_process *proc;
static unsigned error13 = 0;
static unsigned thread_count;
static char dyn_tids[NUM_THREADS];
static char deleted_tids[NUM_THREADS];
// We can get extra threads; add a layer of indirection. Yay.
static int our_tid_max = 0;
static int thread_mapping[NUM_THREADS];
static int deleted_threads;

static unsigned long stack_addrs[NUM_THREADS];

static bool debug_flag = false;
#define dprintf if (debug_flag) fprintf

#define NUM_FUNCS 6
static char initial_funcs[NUM_FUNCS][25] = {"init_func", "main", "_start", "__start", "__libc_start_main", "mainCRTStartup"};

// Globals: our_tid_max, thread_mapping
static int bpindex_to_myindex(int index) {
    for (unsigned i = 0; i < our_tid_max; i++) {
        if (thread_mapping[i] == index) return i;
    }
    return -1;
}

// Globals: deleted_threads, deleted_tids, error13, proc
static void deadthr(BPatch_process *my_proc, BPatch_thread *thr)
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
      logerror("[%s:%u] - Got invalid process: %p vs %p\n", __FILE__,
	      __LINE__, my_proc, proc);
      error13 = 1;
   }
   deleted_tids[my_dyn_id] = 1;
   deleted_threads++;
   dprintf(stderr, "%s[%d]:  leaving to deadthr, %d is dead, %d total dead threads\n", __FILE__, __LINE__, my_dyn_id, deleted_threads);
}

// Globals: dyn_tids, error13, initial_funcs(?), our_tid_max, proc,
// stack_addrs, thread_count, thread_mapping
static void newthr(BPatch_process *my_proc, BPatch_thread *thr)
{
   dprintf(stderr, "%s[%d]:  welcome to newthr, error13 = %d\n", __FILE__, __LINE__, error13);

   if (my_proc != proc && proc != NULL && my_proc != NULL)
   {
      logerror("[%s:%u] - Got invalid process: %p vs %p\n", 
              __FILE__, __LINE__, my_proc, proc);
      error13 = 1;
   }

   if (thr->isDeadOnArrival()) {
      logerror("[%s:%u] - Got a dead on arival thread\n", 
              __FILE__, __LINE__);
      error13 = 1;
      return;
   }

   unsigned my_dyn_id = our_tid_max; our_tid_max++;
   if (bpindex_to_myindex(thr->getBPatchID()) != -1) {
      logerror("[%s:%d] - WARNING: Thread %d called in callback twice\n",
              __FILE__, __LINE__, thr->getBPatchID());
      error13 = 1;
      return;
   }

   thread_mapping[my_dyn_id] = thr->getBPatchID();
   thread_count++;
   dyn_tids[my_dyn_id] = 1;

   dprintf(stderr, "%s[%d]:  newthr: BPatchID = %d\n", __FILE__, __LINE__, thr->getBPatchID());
   //Check initial function
   // BUG(?) Make sure this variable gets initialized properly!
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
      logerror("[%s:%d] - Thread %d has unexpected initial function '%s'; ignoring\n",
              __FILE__, __LINE__, thr->getBPatchID(), name);
      //      error13 = 1; // This shouldn't be an error, according to the comment above.
      BPatch_Vector<BPatch_frame> stack;
      thr->getCallStack(stack);
   }

   //Stacks should be unique and non-zero
   // Moving this variable to global scope
   //static unsigned long stack_addrs[NUM_THREADS];
   unsigned long my_stack = thr->getStackTopAddr();
   if (!my_stack)
   {
      logerror("[%s:%d] - WARNING: Thread %d has no stack\n",
              __FILE__, __LINE__, my_dyn_id);

        // For debugging, dump the stack
        BPatch_Vector<BPatch_frame> stack;
	thr->getCallStack(stack);

        dprintf(stderr, "Stack dump\n");
        for( unsigned i = 0; i < stack.size(); i++) {
                char name[256];
                BPatch_function *func = stack[i].findFunction();
                if (func == NULL)
                        strcpy(name, "[UNKNOWN]");
                else
                        func->getName(name, 256);
                dprintf(stderr, "  %10p: %s, fp = %p\n",
                                stack[i].getPC(),
                                name,
                                stack[i].getFP());
        }
        dprintf(stderr, "End of stack dump.\n");
   }
   else
   {
      for (unsigned i=0; i<NUM_THREADS; i++)
         if (stack_addrs[i] == my_stack)
         {
            logerror("[%s:%d] - WARNING: Thread %d and %d share a stack at %lx\n",
                    __FILE__, __LINE__, my_dyn_id, i, my_stack);
         }
   }
   stack_addrs[my_dyn_id] = my_stack;

   //Thread IDs should be unique
   // FIXME Make sure this static variable works correctly.  Maybe push it out
   // to a regular global variable..
   static long pthread_ids[NUM_THREADS];
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
            logerror("[%s:%d] - WARNING: Thread %d and %d share a tid of %lu\n",
                    __FILE__, __LINE__, my_dyn_id, i, mytid);
            error13 = 1;
      }
   pthread_ids[my_dyn_id] = mytid;

   dprintf(stderr, "%s[%d]:  leaving newthr: error13 = %d\n", __FILE__, __LINE__, error13);
}

void test_thread_6_Mutator::upgrade_mutatee_state()
{
   dprintf(stderr, "%s[%d]:  welcome to upgrade_mutatee_state\n", __FILE__, __LINE__);
   BPatch_variableExpr *var;
   BPatch_image *img = proc->getImage();
	var = img->findVariable("proc_current_state");
	dprintf(stderr, "%s[%d]: upgrade_mutatee_state: stopping for read...\n", __FILE__, __LINE__);
   proc->stopExecution();
   int val = 0;
   var->readValue(&val);
   val++;
   var->writeValue(&val);
   proc->continueExecution();
   dprintf(stderr, "%s[%d]:  upgrade_mutatee_state: continued after write, val = %d\n", __FILE__, __LINE__, val);
}

#define MAX_ARGS 32
static const char *filename = "test13.mutatee_gcc";
static const char *args[MAX_ARGS];
static const char *create_arg = "-create";
static unsigned num_args = 0; 

// This method creates (or attaches to?) the mutatee process and returns a
// handle for it
BPatch_process *test_thread_6_Mutator::getProcess()
{
   return appProc;
}

test_results_t test_thread_6_Mutator::mutatorTest(BPatch *bpatch)
{
   unsigned num_attempts = 0;
   bool missing_threads = false;

   error13 = 0;
   thread_count = 0;
   memset(dyn_tids, 0, sizeof(dyn_tids));
   memset(deleted_tids, 0, sizeof(deleted_tids));
   our_tid_max = 0;
   memset(thread_mapping, -1, sizeof(thread_mapping));
   deleted_threads = 0;
   memset(stack_addrs, 0, sizeof(stack_addrs));

   proc = NULL;
   proc = getProcess();
   if (!proc)
      return FAILED;

   proc->continueExecution();

   newthr(appProc, appThread);

   // For the attach case, we may already have the threads in existence; if so, 
   // manually trigger them here. 
   std::vector<BPatch_thread *> threads;
   appProc->getThreads(threads);
   for (unsigned i = 0; i < threads.size(); ++i) {
	   if (threads[i] == appThread) continue;
	   newthr(appProc, threads[i]);
   }

   // Wait for NUM_THREADS new thread callbacks to run
   while (thread_count < NUM_THREADS) {
      dprintf(stderr, "Going into waitForStatusChange...\n");
      bpatch->waitForStatusChange();
      dprintf(stderr, "Back from waitForStatusChange...\n");
      if (proc->isTerminated())
      {
         logerror("[%s:%d] - App exited early\n", __FILE__, __LINE__);
         error13 = 1;
         break;
      }
      if (num_attempts++ == TIMEOUT)
      {
         logerror("[%s:%d] - Timed out waiting for threads\n", 
                 __FILE__, __LINE__);
         logerror("[%s:%d] - Only have %u threads, expected %u!\n",
              __FILE__, __LINE__, thread_count, NUM_THREADS);
         return FAILED;
      }
      P_sleep(1);
   }

   dprintf(stderr, "%s[%d]:  done waiting for thread creations, error13 = %d\n", __FILE__, __LINE__, error13);

   BPatch_Vector<BPatch_thread *> thrds;
   proc->getThreads(thrds);
   if (thrds.size() != NUM_THREADS)
   {
      logerror("[%s:%d] - Have %u threads, expected %u!\n",
              __FILE__, __LINE__, thrds.size(), NUM_THREADS);
      error13 = 1;
   }

   for (unsigned i=0; i<NUM_THREADS; i++)
   {
      if (!dyn_tids[i])
      {
         logerror("[%s:%d] - Thread %u was never created!\n",
                 __FILE__, __LINE__, i);
         missing_threads = true;
      }
   }
   if(error13 || missing_threads) {
      logerror("%s[%d]: ERROR during thread create stage, exiting\n", __FILE__, __LINE__);
      logerror("*** Failed test_thread_6 (Threading Callbacks)\n");
      if(proc && !proc->isTerminated())
         proc->terminateExecution();
      return FAILED;
   }

   upgrade_mutatee_state();
   dprintf(stderr, "%s[%d]:  Now waiting for application to exit.\n", __FILE__, __LINE__);

   while (!proc->isTerminated()) {
	   proc->continueExecution();
      bpatch->waitForStatusChange();
   }
   num_attempts = 0;
   while(deleted_threads != NUM_THREADS && num_attempts != TIMEOUT) {
      num_attempts++;
	  std::cerr << "Deleted " << deleted_threads << " and expected " << NUM_THREADS << std::endl;
	  P_sleep(1);

   }

   for (unsigned i=1; i<NUM_THREADS; i++)
   {
      if (!deleted_tids[i])
      {
         logerror("[%s:%d] - Thread %d wasn't deleted\n",
                 __FILE__, __LINE__, i);
         error13 = 1;
      }
   }

   if (deleted_threads != NUM_THREADS || !deleted_tids[0])
   {
      logerror("[%s:%d] - %d threads deleted at termination." 
           "  Expected %d\n", __FILE__, __LINE__, deleted_threads, NUM_THREADS);
      error13 = 1;
   }


   if (error13)
   {
       logerror("*** Failed test_thread_6 (Threading Callbacks)\n");
   } else {
       logerror("Passed test_thread_6 (Threading Callbacks)\n");
       logerror("Test completed without errors\n");
       return PASSED;
   }
   return FAILED;
}

test_results_t test_thread_6_Mutator::executeTest() {

   test_results_t rv = mutatorTest(bpatch);

   if (!bpatch->removeThreadEventCallback(BPatch_threadCreateEvent, newthr) ||
       !bpatch->removeThreadEventCallback(BPatch_threadDestroyEvent, deadthr))
   {
      logerror("%s[%d]:  failed to remove thread callback\n",
	      __FILE__, __LINE__);
      return FAILED;
   }

   return rv;
}

test_results_t test_thread_6_Mutator::setup(ParameterDict &param) {
   /* Grab info from param */
   bpatch = (BPatch *)(param["bpatch"]->getPtr());
   filename = param["pathname"]->getString();
   logfilename = param["logfilename"]->getString();
   
   if ( param["debugPrint"]->getInt() != 0 ) {
       debug_flag = true;
   }
   
   if ( param["createmode"]->getInt() != CREATE )
   {
      create_proc = false;
   }
   if (!bpatch->registerThreadEventCallback(BPatch_threadCreateEvent,
					    newthr) ||
       !bpatch->registerThreadEventCallback(BPatch_threadDestroyEvent,
					    deadthr))
   {
      logerror("%s[%d]:  failed to register thread callback\n",
	      __FILE__, __LINE__);
      return FAILED;
   }

   appProc = (BPatch_process *)(param["appProcess"]->getPtr());
   if (appProc) appImage = appProc->getImage();
   
   return DyninstMutator::setup(param);
}
