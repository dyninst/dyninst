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
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "mutatee_util.h"
#include "solo_mutatee_boilerplate.h"

#if !defined(os_windows_test)
#include <unistd.h>
#else
#include <windows.h>
#endif

#define NTHRD 4

volatile int done;
volatile int proc_current_state;
volatile int threads_running[NTHRD];

void *init_func(void *arg)
{
   threads_running[(int) (long) arg] = 1;
   while (!done);
   return arg;
}

/* int main(int argc, char *argv[]) */
int test_thread_6_mutatee() {
   unsigned i;
   thread_t threads[NTHRD];
   int startedall = 0;

   /* initThreads() has an empty function body? */
   initThreads();

   for (i=0; i<NTHRD; i++)  {
      threads[i] = spawnNewThread((void *) init_func, (void *) (long) i);
   }

   while (!startedall) {
      for (i=0; i<NTHRD; i++) {
         startedall = 1;
         if (!threads_running[i]) {
            startedall = 0;
            P_sleep(1);
            break;
         }
      }
   }

   handleAttach();

   logstatus("[%s:%d]: stage 1 - all threads created\n", __FILE__, __LINE__);
   while (proc_current_state == 0) {
     /* Do nothing */
   }
   logstatus("[%s:%d]: stage 2 - allowing threads to exit\n", __FILE__, __LINE__);
   done = 1;
   for (i=0; i<NTHRD; i++)
   {
      joinThread(threads[i]);
   }
   logstatus("[%s:%d]: stage 3 - all threads joined\n", __FILE__, __LINE__);
   /* Is the return value of this mutatee checked?  Doesn't look like it */
   return 0;
}
