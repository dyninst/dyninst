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
#include "pcontrol_mutatee_tools.h"
#include "mutatee_util.h"
#include "solo_mutatee_boilerplate.h"

static testlock_t init_lock;

static void waitForEventSource() {
    int i;
    for(i = 0; i < 10; ++i) {
        precisionSleep(100); // ms
    }
}

static int threadFunc(int myid, void *data)
{
   testLock(&init_lock);
   testUnlock(&init_lock);

   waitForEventSource();
   return 0;
}

// Basic test for temporary detach 
// 
// The mutator temporarily detaches from the mutatee and the
// mutatee generates events periodically. The mutator re-attaches
// when the mutatee has finished generating events and the mutator
// reads some state.
int pc_temp_detach_mutatee()
{
   syncloc syncloc_msg;
   send_addr addr_msg;
   int result;
   uint64_t eventCount = 0;
   event_source *es = NULL;

   initLock(&init_lock);
   testLock(&init_lock);

   result = initProcControlTest(threadFunc, NULL);
   if (result != 0) {
      output->log(STDERR, "Initialization failed\n");
      return -1;
   }

   result = recv_message((unsigned char *) &syncloc_msg, sizeof(syncloc));
   if (result != 0) {
      output->log(STDERR, "Failed to recieve sync message\n");
      return -1;
   }
   if (syncloc_msg.code != SYNCLOC_CODE) {
      output->log(STDERR, "Incorrect sync code\n");
      return -1;
   }

   // At this point the mutator should be detached //

   // Send the address of the eventCount variable to the mutator
   addr_msg.code = SENDADDR_CODE;
   addr_msg.addr = ((uint64_t)(unsigned long)&eventCount);
   result = send_message((unsigned char *) &addr_msg, sizeof(send_addr));
   if( result != 0 ) {
       output->log(STDERR, "Failed to send addr message\n");
       return -1;
   }

   es = startEventSource();
   if( es == NULL ) {
       output->log(STDERR, "Failed to start event source\n");
       return -1;
   }

   testUnlock(&init_lock);

   waitForEventSource();

   result = finiProcControlTest(0);
   if (result != 0) {
      output->log(STDERR, "Finalization failed\n");
      return -1;
   }

   // The mutator will read this variable after re-attach
   eventCount = getEventCounter();

   if( !stopEventSource(es) ) {
       output->log(STDERR, "Failed to stop event source\n");
       return -1;
   }

   // Initiate sync with mutator to allow it to re-attach and inspect state
   result = send_message((unsigned char *) &syncloc_msg, sizeof(syncloc));
   if (result != 0) {
      output->log(STDERR, "Failed to send sync message\n");
      return -1;
   }

   // Now wait for the mutator's sync message before exiting
   result = recv_message((unsigned char *) &syncloc_msg, sizeof(syncloc));
   if( result != 0) {
       output->log(STDERR, "Failed to recv sync message\n");
       return -1;
   }
   if (syncloc_msg.code != SYNCLOC_CODE) {
       // Make sure eventCount isn't deallocated until mutator reads it
       output->log(STDERR, "Event count: %llu\n", eventCount);

       output->log(STDERR, "Incorrect sync code\n");
       return -1;
   }

   test_passes(testname);
   return 0;
}
