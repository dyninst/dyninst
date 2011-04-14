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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <signal.h>
#include <sys/types.h>
#include <assert.h>
#include <unistd.h>
#include <limits.h>

#ifdef __cplusplus
int mutateeCplusplus = 1;
#else
int mutateeCplusplus = 0;
#endif

#ifndef COMPILER
#define COMPILER ""
#endif
const char *Builder_id=COMPILER; /* defined on compile line */

#define NTHRD 5
pthread_t thrds[NTHRD];
int thr_ids[NTHRD] = {0, 1, 2, 3, 4};
int ok_to_exit[NTHRD] = {0, 0, 0, 0, 0};

/* oneTimeCodes will set these to the tid for the desired thread */
volatile pthread_t sync_test = 0;
volatile pthread_t async_test = 0;
int sync_failure = 0;
int async_failure = 0;

volatile unsigned thr_exits;

void check_sync()
{
   pthread_t tid = pthread_self();
   int id = -1, i;
   for(i = 0; i < NTHRD; i++) {
      if(thrds[i] == tid) {
         id = i;
         break;
      }
   }

   if(tid == sync_test) {
      printf("Thread %d [tid %lu] - oneTimeCode completed successfully\n", 
             id, tid);
      ok_to_exit[id] = 1;
      return;
   }
   else if(sync_test != 0)
      fprintf(stderr, "%s[%d]: ERROR: Thread %d [tid %lu] - mistakenly ran oneTimeCode for thread with tid %lu\n", __FILE__, __LINE__, id, tid, sync_test);
   else
      fprintf(stderr, "%s[%d]: ERROR: Thread %d [tid %lu] - sync_test is 0\n", __FILE__, __LINE__, id, tid);
   sync_failure++;
}

void check_async()
{
   pthread_t tid = pthread_self();
   int id = -1, i;
   for(i = 0; i < NTHRD; i++) {
      if(thrds[i] == tid) {
         id = i;
         break;
      }
   }

   if(tid == async_test) {
      printf("Thread %d [tid %lu] - oneTimeCodeAsync completed successfully\n",
             id, tid);
      return;
   }
   else if(async_test != 0)
      fprintf(stderr, 
              "%s[%d]: ERROR: Thread %d [tid %lu] - mistakenly ran oneTimeCodeAsync for thread with tid %lu\n", 
              __FILE__, __LINE__, id, tid, async_test);
   else
      fprintf(stderr, 
              "%s[%d]: ERROR: Thread %d [tid %lu] - async_test is 0\n", 
              __FILE__, __LINE__, id, tid);
   async_failure++;
}

#define MAX_TIMEOUTS 5
void thr_loop(int id, pthread_t tid)
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
   if(num_timeout == MAX_TIMEOUTS)
      fprintf(stderr, 
              "%s[%d]: ERROR: Thread %d [tid %lu] - timed-out in thr_loop\n", 
              __FILE__, __LINE__, id, (unsigned long)tid);
}

void thr_func(void *arg)
{
   unsigned busy_work = 0;
   int id = *((int*)arg);
   pthread_t tid = pthread_self();
   thr_loop(id, tid);
   while(++busy_work != UINT_MAX/10);
   thr_exits++;
}

void *init_func(void *arg)
{
   assert(arg != NULL);
   thr_func(arg);
   return NULL;
}

int attached_fd;
void parse_args(int argc, char *argv[])
{
   int i;
   for (i=0; i<argc; i++)
   {
      if (strstr(argv[i], "-attach"))
      {
         if (++i == argc) break;
         attached_fd = atoi(argv[i]);
      }
   }
}

int isAttached = 0;
/* Check to see if the mutator has attached to us. */
int checkIfAttached()
{
    return isAttached;
}

int main(int argc, char *argv[])
{
   unsigned i;
   void *ret_val;
   char c = 'T';
   pthread_attr_t attr;

   if(argc == 1) {
      printf("Mutatee %s [%s]:\"%s\"\n", argv[0], 
             mutateeCplusplus ? "C++" : "C", Builder_id);
      return 0;
   }

   pthread_attr_init(&attr);
   pthread_attr_setscope(&attr, PTHREAD_SCOPE_SYSTEM);

   thr_exits = 0;

   parse_args(argc, argv);

   /* create the workers */
   for (i=1; i<NTHRD; i++)
   {
      pthread_create(&(thrds[i]), &attr, init_func, &(thr_ids[i]));
   }
   thrds[0] = pthread_self();

   if (attached_fd) {
      if (write(attached_fd, &c, sizeof(char)) != sizeof(char)) {
         fprintf(stderr, "*ERROR*: Writing to pipe\n");
         exit(-1);
      }
      close(attached_fd);
      printf("Waiting for mutator to attach...\n");
      while(! checkIfAttached()) ;
      printf("Mutator attached.  Mutatee continuing.\n");
   }
   
   /* give time for workers to run thr_loop */
   while(thr_exits == 0)
      sched_yield();

   /* wait for worker exits */
   for (i=1; i<NTHRD; i++)
   {
      pthread_join(thrds[i], &ret_val);
   }
   
   if(sync_failure)
      fprintf(stderr, 
              "%s[%d]: ERROR: oneTimeCode failed for %d threads\n", 
              __FILE__, __LINE__, sync_failure);
   if(async_failure)
      fprintf(stderr, 
              "%s[%d]: ERROR: oneTimeCodeAsync failed for %d threads\n", 
              __FILE__, __LINE__, async_failure);

   /* let mutator do final work after noticing all workers exit */
   sleep(5);

   return 0;
}
