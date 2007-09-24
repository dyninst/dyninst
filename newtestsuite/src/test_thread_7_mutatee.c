#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <sys/types.h>
#include <assert.h>

#include "mutatee_util.h"

extern thread_t spawnNewThread(void *initial_func, void *param);
extern void joinThread(thread_t threadid);
extern void initThreads();

#define NTHRD 8
#define TIMEOUT 10
#define N_INSTR 4

typedef struct thrds_t
{
   thread_t tid;
   int is_in_instr;
   int thread_setup;
} thrds_t;

thrds_t thrds[NTHRD];
volatile int threads_running[NTHRD];

testlock_t barrier_mutex;
testlock_t count_mutex;

volatile int times_level1_called;

void my_barrier(volatile int *br)
{
   int n_sleeps = 0;
   testLock(&barrier_mutex);
   (*br)++;
   if (*br == NTHRD)
      *br = 0;
   testUnlock(&barrier_mutex);
   while (*br)
   {
      if (n_sleeps++ == TIMEOUT)
      {
         logerror("[%s:%u] - Not all threads reported.  Perhaps "
                 "tramp guards are incorrectly preventing some threads "
                 "from running\n",
                 __FILE__, __LINE__);
	 /* FIXME Don't call exit()! */
         exit(1);
      }
      P_sleep(1);        
   }
}

int test_thread_7_level3(int volatile count)
{
   static volatile int prevent_optimization;
   if (!count)
      return 0;
   test_thread_7_level3(count-1);
   prevent_optimization++;
   return prevent_optimization + count;
}

void test_thread_7_level2()
{
   test_thread_7_level3(100);
}

/**
 * Instrumentation to call this function is inserted into the following funcs:
 *  level0
 *  level1
 *  level2
 *  level3 
 * Tramp guards should prevent all of these calls except at init_func and the
 * second call to level2
 **/
void test_thread_7_level1()
{
   unsigned i;
   static int bar, bar2;

   thread_t me = threadSelf();
   for (i=0; i<NTHRD; i++) {
      /* dprintf("Comparing %lu to %lu\n", thread_int(thrds[i].tid), thread_int(me)); */
      if (threads_equal(thrds[i].tid, me))
         break;
   }

   if (i == NTHRD)
   {
      logerror("[%s:%d] - Error, couldn't find thread id %lu\n",
              __FILE__, __LINE__, thread_int(me));
      /* FIXME Don't call exit()! */
      exit(1);
   }
   if (thrds[i].is_in_instr)
   {
      logerror("[%s:%d] - Error, thread %lu reentered instrumentation\n",
              __FILE__, __LINE__, thread_int(me));
      /* FIXME Don't call exit()! */
      exit(1);
   }

   thrds[i].is_in_instr = 1;

   testLock(&count_mutex);
   times_level1_called++;
   testUnlock(&count_mutex);

   /**
    * Now try to re-enter this function with the same thread.
    * Dyninst should prevent this
    **/
   my_barrier(&bar);
   
   test_thread_7_level2();      

   my_barrier(&bar2);

   thrds[i].is_in_instr = 0;
}

/* This is building a call stack.  Looks like it might be trouble if we're
 * tail-call optimized.  Yes; tail-call optimization breaks the test.
 */
volatile int dont_optimize = 0;
void test_thread_7_level0(int count)
{
   if (count)
      test_thread_7_level0(count - 1);
   dont_optimize++;
}

volatile unsigned ok_to_go = 0;
void *init_func(void *arg)
{
   threads_running[(int) (long) arg] = 1;
   while(! ok_to_go) P_sleep(1);
   test_thread_7_level0(N_INSTR-1);
   return NULL;
}

/* int main(int argc, char *argv[]) */
int test_thread_7_mutatee() {
   unsigned i;
   int startedall = 0;

#ifndef os_windows
   char c = 'T';
#endif

   initLock(&barrier_mutex);
   initLock(&count_mutex);

   /* parse_args(argc, argv); */

   for (i=1; i<NTHRD; i++)
   {
      thrds[i].is_in_instr = 0;
      thrds[i].tid = spawnNewThread((void *) init_func, (void *) i);
   }
   thrds[0].is_in_instr = 0;
   thrds[0].tid = threadSelf();

   while (!startedall) {
      for (i=1; i<NTHRD; i++) {
         startedall = 1;
         if (!threads_running[i]) {
            startedall = 0;
            P_sleep(1);
            break;
         }
      }
   }

   ok_to_go = 1;
   init_func(NULL);
   for (i=1; i<NTHRD; i++)
   {
      joinThread(thrds[i].tid);
   }
   
   if (times_level1_called != NTHRD*N_INSTR)
   {
      logerror("[%s:%u] - level1 called %u times.  Expected %u\n",
              __FILE__, __LINE__, times_level1_called, NTHRD*N_INSTR);
      return -1;
   }

   test_passes(testname);
   return 0; /* Return value *does* matter for this one */
}
