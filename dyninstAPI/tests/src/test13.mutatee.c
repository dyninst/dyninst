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
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
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

#define NTHRD 4
volatile int done;
volatile int proc_current_state;

void *init_func(void *arg)
{
   while (!done);
   return arg;
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
   char c = 'T';
   pthread_t threads[NTHRD];
   pthread_attr_t attr;

   void *ret_val;

   if(argc == 1) {
      printf("Mutatee %s [%s]:\"%s\"\n", argv[0], 
             mutateeCplusplus ? "C++" : "C", Builder_id);
      return 0;
   }

   pthread_attr_init(&attr);
   pthread_attr_setscope(&attr, PTHREAD_SCOPE_SYSTEM);

   parse_args(argc, argv);

   for (i=0; i<NTHRD; i++)
   {
      pthread_create(&threads[i], &attr, init_func, (void *) i);
   }
   
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

   fprintf(stderr, "[%s:%d]: stage 1 - all threads created\n", __FILE__, __LINE__);

   while (proc_current_state == 0) sched_yield();

   fprintf(stderr, "[%s:%d]: stage 2 - allowing threads to exit\n", __FILE__, __LINE__);
   done = 1;

   for (i=0; i<NTHRD; i++)
   {
      pthread_join(threads[i], &ret_val);
   }
   fprintf(stderr, "[%s:%d]: stage 3 - all threads joined\n", __FILE__, __LINE__);
   return 0;
}
