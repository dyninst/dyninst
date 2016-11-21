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
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "pcontrol_mutatee_tools.h"
#include "solo_mutatee_boilerplate.h"

struct local_data {
    testbarrier_t barrier;
    testlock_t *myLocks;
};

static int threadFunc(int myid, void *data)
{
    struct local_data *localData = (struct local_data *)data;

    // Wait on the barrier
    waitTestBarrier(&localData->barrier);

    // Then wait on the lock
    testLock(&localData->myLocks[myid]);
    testUnlock(&localData->myLocks[myid]);

    return 0;
}


static
void freeLocalData(struct local_data *ld) {
    free(ld->myLocks);
    free(ld);
}

extern int num_threads;

//Basic test for create/attach and exit.
int pc_thread_cont_mutatee()
{
   int result;
   int error = 0;
   int i;
   struct local_data *localData = NULL;
   handshake can_stop;
   allow_exit can_exit;

   num_threads = 0;
   for (i = 0; i < gargc; i++) {
      if (strcmp(gargv[i], "-mt") == 0) {
         num_threads = atoi(gargv[i+1]);
         break;
      }
   }

   if( num_threads > 0 ) {
       // Each thread needs its own mutex and access to the barrier
       localData = (struct local_data *)malloc(sizeof(struct local_data));
       initBarrier(&localData->barrier, num_threads+1);
       localData->myLocks = (testlock_t *)malloc(sizeof(testlock_t)*num_threads);
       for(i = 0; i < num_threads; ++i) {
           initLock(&localData->myLocks[i]);
           testLock(&localData->myLocks[i]);
       }
   }
   result = initProcControlTest(threadFunc, (void *)localData);
   if (result != 0) {
      output->log(STDERR, "Initialization failed\n");
      if( num_threads > 0 ) freeLocalData(localData);
      return -1;
   }
   if( num_threads > 0 ) {
       logstatus("initial thread: waiting on barrier\n");
       waitTestBarrier(&localData->barrier);
       // Alert the mutator that all the threads have gotten through the lock
       // and can safely be stopped now
       can_stop.code = HANDSHAKE_CODE;
       send_message((unsigned char *)&can_stop, sizeof(handshake));
       // Wait for mutator to indicate that the stop finished
       memset(&can_stop, 0, sizeof(handshake));
       recv_message((unsigned char *)&can_stop, sizeof(handshake));
       if (can_stop.code != HANDSHAKE_CODE) {
           output->log(STDERR, "Received event that wasn't handshake\n");
           error = 1;
       }
       logstatus("initial thread: received stop handshake, releasing threads\n");
       // Release all the locks
       for(i = 0; i < num_threads; ++i) {
           testUnlock(&localData->myLocks[i]);
       }
       // Alert mutator that all threads can be continued when ready
       memset(&can_stop, 0, sizeof(handshake));
       can_stop.code = HANDSHAKE_CODE;
       send_message((unsigned char *)&can_stop, sizeof(handshake));
       logstatus("initial thread: all threads can be continued\n");
   }
   recv_message((unsigned char *) &can_exit, sizeof(allow_exit));
   if (can_exit.code != ALLOWEXIT_CODE) {
      output->log(STDERR, "Recieved event that wasn't allow_exit\n");
      error = 1;
   }
   result = finiProcControlTest(0);
   if (result != 0) {
      output->log(STDERR, "Finalization failed\n");
      if( num_threads > 0 ) freeLocalData(localData);
      return -1;
   }
   if (error) {
      if( num_threads > 0 ) freeLocalData(localData);
      return -1;
   }
   if( num_threads > 0 ) freeLocalData(localData);
   test_passes(testname);
   return 0;
}
