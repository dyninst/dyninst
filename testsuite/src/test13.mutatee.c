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

#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#define NTHRD 4

volatile int done;
volatile int proc_current_state;


void *init_func(void *arg)
{
   while (!done)
   {
      /*      sleep(1);*/
   }
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

int main(int argc, char *argv[])
{
   unsigned i;
   char c = 'T';
   pthread_t threads[NTHRD];
   pthread_attr_t attr;
   void *ret_val;

   pthread_attr_init(&attr);
   parse_args(argc, argv);

   for (i=0; i<NTHRD; i++)
   {
      pthread_create(&threads[i], &attr, init_func, (void *) i);
   }
   if (attached_fd)
      write(attached_fd, &c, sizeof(char));
   fprintf(stderr, "stage 1\n");
   while (proc_current_state == 0);
   fprintf(stderr, "stage 2\n");
   done = 1;
   for (i=0; i<NTHRD; i++)
   {
      pthread_join(threads[i], &ret_val);
   }
   fprintf(stderr, "stage 3\n");
   while (proc_current_state == 1);
   fprintf(stderr, "stage 4\n");
   return 0;
}
