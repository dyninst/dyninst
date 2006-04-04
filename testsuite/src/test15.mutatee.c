#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/types.h>
#include <assert.h>
#include <limits.h>
#include "mutatee_util.h"

#if defined(os_windows)
#define TVOLATILE
#else
#define TVOLATILE volatile
#endif

#define NTHRD 5
thread_t thrds[NTHRD];

int thr_ids[NTHRD] = {0, 1, 2, 3, 4};
int ok_to_exit[NTHRD] = {0, 0, 0, 0, 0};

/* oneTimeCodes will set these to the tid for the desired thread */
TVOLATILE thread_t sync_test;
TVOLATILE thread_t async_test;
int sync_failure = 0;
int async_failure = 0;

volatile unsigned thr_exits;

void check_sync()
{
   thread_t tid = threadSelf();
   int id = -1, i;
   for(i = 0; i < NTHRD; i++) {
      if (threads_equal(thrds[i], tid)) {
         id = i;
         break;
      }
   }

   if(threads_equal(tid, sync_test)) {
      printf("Thread %d [tid %lu] - oneTimeCode completed successfully\n", 
             id, tid);
      ok_to_exit[id] = 1;
      return;
   }
   else if( thread_int(sync_test) != 0)
      fprintf(stderr, "%s[%d]: ERROR: Thread %d [tid %lu] - mistakenly ran oneTimeCode for thread with tid %lu\n", __FILE__, __LINE__, id, thread_int(tid), thread_int(sync_test));
   else
      fprintf(stderr, "%s[%d]: ERROR: Thread %d [tid %lu] - sync_test is 0\n", __FILE__, __LINE__, id, thread_int(tid));
   sync_failure++;
}

void check_async()
{
   thread_t tid = threadSelf();
   int id = -1, i;
   for(i = 0; i < NTHRD; i++) {
      if(threads_equal(thrds[i], tid)) {
         id = i;
         break;
      }
   }

   if(threads_equal(tid, async_test)) {
      printf("Thread %d [tid %lu] - oneTimeCodeAsync completed successfully\n",
             id, thread_int(tid));
      return;
   }
   else if(thread_int(async_test) != 0)
      fprintf(stderr, 
              "%s[%d]: ERROR: Thread %d [tid %lu] - mistakenly ran oneTimeCodeAsync for thread with tid %lu\n", 
              __FILE__, __LINE__, id, thread_int(tid), thread_int(async_test));
   else
      fprintf(stderr, 
              "%s[%d]: ERROR: Thread %d [tid %lu] - async_test is 0\n", 
              __FILE__, __LINE__, id, thread_int(tid));
   async_failure++;
}

void thr_loop(int id, thread_t tid)
{
   unsigned long timeout = 0;
   while( (! ok_to_exit[id]) && (timeout != 50000000) ) {
      timeout++;
      schedYield();
   }
   if(timeout == 50000000)
      fprintf(stderr, 
              "%s[%d]: ERROR: Thread %d [tid %lu] - timed-out in thr_loop\n", 
              __FILE__, __LINE__, id, thread_int(tid));
}

void thr_func(void *arg)
{
   unsigned busy_work = 0;
   int id = *((int*)arg);
   thread_t tid = threadSelf();
   thr_loop(id, tid);
   /* busy work simulates the fact that we don't expect a thread to immediately
      exit after performing a oneTimeCode */
   while(++busy_work != UINT_MAX/10);
   thr_exits++;
}

void *init_func(void *arg)
{
   assert(arg != NULL);
   thr_func(arg);
   return NULL;
}

int main()
{
   unsigned i;

#if defined(os_osf)
   return 0;
#endif

   thr_exits = 0;

   /* create the workers */
   for (i=1; i<NTHRD; i++)
   {
      thrds[i] = spawnNewThread((void *) init_func, &(thr_ids[i]));
   }
   thrds[0] = threadSelf();
   
   /* give time for workers to run thr_loop */
   while(thr_exits == 0)
      schedYield();

   /* wait for worker exits */
   for (i=1; i<NTHRD; i++)
   {
      joinThread(thrds[i]);
   }
   
   if(sync_failure)
      fprintf(stderr, 
              "%s[%d]: ERROR: oneTimeCode failed for %d threads\n", 
              __FILE__, __LINE__, sync_failure);
   if(async_failure)
      fprintf(stderr, 
              "%s[%d]: ERROR: oneTimeCodeAsync failed for %d threads\n", 
              __FILE__, __LINE__, async_failure);

   return 0;
}
