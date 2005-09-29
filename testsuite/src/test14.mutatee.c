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

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <signal.h>
#include <sys/types.h>
#include <unistd.h>

#define NTHRD 8
#define TIMEOUT 10
#define N_INSTR 4

typedef struct thrds_t
{
   pthread_t tid;
   int is_in_instr;
} thrds_t;
thrds_t thrds[NTHRD];

pthread_mutex_t barrier_mutex;
pthread_mutex_t count_mutex;

volatile int times_level1_called;

void my_barrier(volatile int *br)
{
   int n_sleeps = 0;
   pthread_mutex_lock(&barrier_mutex);
   (*br)++;
   if (*br == NTHRD)
      *br = 0;
   pthread_mutex_unlock(&barrier_mutex);
   while (*br)
   {
      if (n_sleeps++ == TIMEOUT)
      {
         fprintf(stderr, "[%s:%u] - Not all threads reported.  Perhaps "
                 "tramp guards are incorrectly preventing some threads "
                 "from running\n",
                 __FILE__, __LINE__);
         exit(1);
      }
      sleep(1);        
   }
}

int level3(int volatile count)
{
   static volatile int prevent_optimization;
   if (!count)
      return 0;
   level3(count-1);
   prevent_optimization++;
   return prevent_optimization + count;
}

void level2()
{
   level3(100);
}

/**
 * Instrumentation to call this function is inserted into the following funcs:
 *  init_func
 *  level2 (called twice)
 *  level3 
 * Tramp guards should prevent all of these calls except at init_func and the
 * second call to level2
 **/
void level1()
{
   unsigned i;
   static int bar, bar2;
   pthread_t me = pthread_self();
   for (i=0; i<NTHRD; i++)
      if (thrds[i].tid == me)
         break;

   if (i == NTHRD)
   {
      fprintf(stderr, "[%s:%d] - Error, couldn't find thread id %u\n",
              __FILE__, __LINE__, (unsigned) me);
      exit(1);
   }
   if (thrds[i].is_in_instr)
   {
      fprintf(stderr, "[%s:%d] - Error, thread %u reentered instrumentation\n",
              __FILE__, __LINE__, (unsigned) me);
      exit(1);
   }

   thrds[i].is_in_instr = 1;

   pthread_mutex_lock(&count_mutex);
   times_level1_called++;
   pthread_mutex_unlock(&count_mutex);

   /**
    * Now try to re-enter this function with the same thread.
    * Dyninst should prevent this
    **/
   my_barrier(&bar);
   
   level2();      

   my_barrier(&bar2);

   thrds[i].is_in_instr = 0;
}

void level0(int count)
{
   if (count)
      level0(count - 1);
}

void *init_func(void *arg)
{
   level0(N_INSTR-1);
   return NULL;
}

int main(int argc, char *argv[])
{
   unsigned i;
   void *ret_val;

   pthread_attr_t attr;
   pthread_attr_init(&attr);
   
   pthread_mutex_init(&barrier_mutex, NULL);
   pthread_mutex_init(&count_mutex, NULL);

   for (i=1; i<NTHRD; i++)
   {
      pthread_create(&(thrds[i].tid), &attr, init_func, NULL);
      thrds[i].is_in_instr = 0;
   }
   thrds[0].tid = pthread_self();
   thrds[0].is_in_instr = 0;
   init_func(NULL);
   for (i=1; i<NTHRD; i++)
   {
      pthread_join(thrds[i].tid, &ret_val);
   }
   
   if (times_level1_called != NTHRD*N_INSTR)
   {
      fprintf(stderr, "[%s:%u] - level1 called %u times.  Expected %u\n",
              __FILE__, __LINE__, times_level1_called, NTHRD*N_INSTR);
      exit(1);
   }
   return 0;
}
