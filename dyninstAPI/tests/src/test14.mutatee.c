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

#ifdef __cplusplus
int mutateeCplusplus = 1;
#else
int mutateeCplusplus = 0;
#endif

#ifndef COMPILER
#define COMPILER ""
#endif
const char *Builder_id=COMPILER; /* defined on compile line */

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
 *  level0
 *  level1
 *  level2
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
      fprintf(stderr, "[%s:%d] - Error, couldn't find thread id %lu\n",
              __FILE__, __LINE__, me);
      exit(1);
   }
   if (thrds[i].is_in_instr)
   {
      fprintf(stderr, "[%s:%d] - Error, thread %lu reentered instrumentation\n",
              __FILE__, __LINE__, me);
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

volatile unsigned ok_to_go = 0;
void *init_func(void *arg)
{
   assert(arg == NULL);
   while(! ok_to_go) sleep(1);
   level0(N_INSTR-1);
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
   
   pthread_mutex_init(&barrier_mutex, NULL);
   pthread_mutex_init(&count_mutex, NULL);

   parse_args(argc, argv);

   for (i=1; i<NTHRD; i++)
   {
      pthread_create(&(thrds[i].tid), &attr, init_func, NULL);
      thrds[i].is_in_instr = 0;
   }
   thrds[0].tid = pthread_self();
   thrds[0].is_in_instr = 0;

   if (attached_fd) {
      if (write(attached_fd, &c, sizeof(char)) != sizeof(char)) {
         fprintf(stderr, "*ERROR*: Writing to pipe\n");
         exit(-1);
      }
      close(attached_fd);
      printf("Waiting for mutator to attach...\n");
      while (!checkIfAttached()) ;
      printf("Mutator attached.  Mutatee continuing.\n");

   }

   ok_to_go = 1;
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
