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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <sys/types.h>
#include <assert.h>
#include <limits.h>
#include "mutatee_util.h"
#include "solo_mutatee_boilerplate.h"

#define NTHRD 5
thread_t thrds[NTHRD];

int thr_ids[NTHRD] = {0, 1, 2, 3, 4};
volatile int ok_to_exit[NTHRD] = {0, 0, 0, 0, 0};
volatile int threads_running[NTHRD];

/* These functions are accessed by the mutator */
void check_sync();
void check_async();

volatile int sync_failure = 0;
volatile int async_failure = 0;
volatile int timeout_failure = 0;

volatile unsigned thr_exits;

void check_sync(thread_t sync_test)
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
      logerror("Thread %d [tid %lu] - oneTimeCode completed successfully\n", 
             id, tid);
      ok_to_exit[id] = 1;
      return;
   }
   else if( thread_int(sync_test) != 0)
      logerror("%s[%d]: ERROR: Thread %d [tid %lu] - mistakenly ran oneTimeCode for thread with tid %lu\n", __FILE__, __LINE__, id, thread_int(tid), thread_int(sync_test));
   else
      logerror("%s[%d]: ERROR: Thread %d [tid %lu] - sync_test is 0\n", __FILE__, __LINE__, id, thread_int(tid));

   // Still let the threads exit, this will allow a faster failure than waiting for a timeout
   ok_to_exit[id] = 1;
   sync_failure++;
}

void check_async(thread_t async_test)
{
   thread_t tid = threadSelf();
   int id = -1, i;
   for(i = 0; i < NTHRD; i++) {
      if(threads_equal(thrds[i], tid)) {
	/* Determine the index of the current thread */
         id = i;
         break;
      }
   }

   if(threads_equal(tid, async_test)) {
     /* Verify that async_test was set to the correct thread id */
      logerror("Thread %d [tid %lu] - oneTimeCodeAsync completed successfully\n",
             id, thread_int(tid));
      return;
   }
   else if(thread_int(async_test) != 0)
      logerror("%s[%d]: ERROR: Thread %d [tid %lu] - mistakenly ran oneTimeCodeAsync for thread with tid %lu\n", 
              __FILE__, __LINE__, id, thread_int(tid), thread_int(async_test));
   else
      logerror("%s[%d]: ERROR: Thread %d [tid %lu] - async_test is 0\n", 
              __FILE__, __LINE__, id, thread_int(tid));
   async_failure++;
}

/* I'm seeing a lot of timeouts.  Need to figure this out */
#define MAX_TIMEOUTS 5
void thr_loop(int id, thread_t tid)
{
   unsigned long timeout = 0;
   unsigned num_timeout = 0;
   while( (! ok_to_exit[id]) && (num_timeout != MAX_TIMEOUTS) ) {
      timeout++;
      if(timeout == ULONG_MAX) {
         timeout = 0;
         num_timeout++;
      }
   }
   if(num_timeout == MAX_TIMEOUTS) {
     logerror("%s[%d]: ERROR: Thread %d [tid %lu] - timed-out in thr_loop\n", 
	     __FILE__, __LINE__, id, thread_int(tid));
     timeout_failure++;
   }
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
   int id = *((int*)arg);
   assert(arg != NULL);

   threads_running[id] = 1;

   thr_func(arg);
   return NULL;
}

int test_thread_8_mutatee() {
   unsigned i;
   int startedall = 0;
   thr_exits = 0;

   /* create the workers */
   for (i=1; i<NTHRD; i++)
   {
      thrds[i] = spawnNewThread((void *) init_func, &(thr_ids[i]));
   }
   thrds[0] = threadSelf();

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
   handleAttach();

   // Allow the mutator to know when initialization has been finished
   stop_process_();

   /* wait for worker exits */
   for (i=1; i<NTHRD; i++)
   {
      joinThread(thrds[i]);
   }
   
   if(sync_failure)
      logerror("%s[%d]: ERROR: oneTimeCode failed for %d threads\n", 
              __FILE__, __LINE__, sync_failure);
   if(async_failure)
      logerror("%s[%d]: ERROR: oneTimeCodeAsync failed for %d threads\n", 
              __FILE__, __LINE__, async_failure);

   /* TODO Check return value for this mutatee! */
   if(sync_failure) return -1;
   if(async_failure) return -2;
   if(timeout_failure) return -3;
   test_passes(testname);
   return 0;
}
