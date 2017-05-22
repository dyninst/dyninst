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
#include <assert.h>
#include "pcontrol_mutatee_tools.h"
#include "mutatee_util.h"
#include "solo_mutatee_boilerplate.h"

int func_counts[4][MAX_POSSIBLE_THREADS];
#define NUM_BREAKPOINT_SPINS 16

static testlock_t init_lock;

void bp0(int tid)
{
  //output->log(STDERR, "Pre-increment in bp0\n");
  func_counts[0][tid]++;
  //output->log(STDERR, "Post-increment in bp0\n");
}

void bp1(int tid)
{
  func_counts[1][tid]++;
}

void bp2(int tid)
{
  func_counts[2][tid]++;
}

void bp3(int tid)
{
  func_counts[3][tid]++;
}


static int threadFunc(int myid, void *data)
{
  unsigned i;

  //output->log(STDERR, "Entered threadFunc OK, myid = %d\n", myid);
  assert(((num_threads == 0) || (myid >= 0 && myid < num_threads)) && !data);

  testLock(&init_lock);
  testUnlock(&init_lock);

  for (i=0; i<NUM_BREAKPOINT_SPINS; i++) {
  //output->log(STDERR, "Entering bp0, myid = %d\n", myid);
    bp0(myid);
  //output->log(STDERR, "Finished bp0 OK, myid = %d\n", myid);
    bp1(myid);
  //output->log(STDERR, "Finished bp1 OK, myid = %d\n", myid);
    bp2(myid);
  //output->log(STDERR, "Finished bp2 OK, myid = %d\n", myid);
    bp3(myid);
  //output->log(STDERR, "Finished bp3 OK, myid = %d\n", myid);
  }
  return 0;
}

//Basic test for create/attach and exit.
int pc_breakpoint_mutatee()
{
   int result, i, actual_threads;
   send_addr bp_addr_msg;
   syncloc syncloc_msg;

   initLock(&init_lock);
   testLock(&init_lock);

   result = initProcControlTest(threadFunc, NULL);

   if (result != 0) {
      output->log(STDERR, "Initialization failed\n");
      return -1;
   }
   //output->log(STDERR, "Initialized OK\n");

   bp_addr_msg.code = (uint32_t) SENDADDR_CODE;
   bp_addr_msg.addr = getFunctionPtr((intptr_t *)bp0);
   result = send_message((unsigned char *) &bp_addr_msg, sizeof(send_addr));
   if (result == 0) {
     bp_addr_msg.addr = getFunctionPtr((intptr_t *)bp1);
     result = send_message((unsigned char *) &bp_addr_msg, sizeof(send_addr));
   }
   if (result == 0) {
     bp_addr_msg.addr = getFunctionPtr((intptr_t *)bp2);
     result = send_message((unsigned char *) &bp_addr_msg, sizeof(send_addr)); 
   }
   if (result == 0) {
     bp_addr_msg.addr = getFunctionPtr((intptr_t *)bp3);
     result = send_message((unsigned char *) &bp_addr_msg, sizeof(send_addr));
   }

   if (result != 0) {
     output->log(STDERR, "Failed to send breakpoint addresses\n");
     testUnlock(&init_lock);
     return -1;
   }
   //output->log(STDERR, "Sent BP addrs OK\n");

   result = recv_message((unsigned char *) &syncloc_msg, sizeof(syncloc));
   if (result != 0) {
     output->log(STDERR, "Failed to recieve sync message\n");
     testUnlock(&init_lock);
     return -1;
   }

   if (syncloc_msg.code != SYNCLOC_CODE) {
     output->log(STDERR, "Recieved unexpected non-sync message\n");
     testUnlock(&init_lock);
     return -1;
   }
   //output->log(STDERR, "Got sync message OK\n");

   testUnlock(&init_lock);
   
   if (num_threads == 0) {
     threadFunc(0, NULL);
     actual_threads = 1;
   }
   else {
     actual_threads = num_threads;
   }
    
   result = finiProcControlTest(0);
   if (result != 0) {
      output->log(STDERR, "Finalization failed\n");
      return -1;
   }
   //output->log(STDERR, "Finalization OK, checking results\n");

   for (i = 0; i < actual_threads; i++) {
     if (func_counts[0][i] != NUM_BREAKPOINT_SPINS) {
       output->log(STDERR, "bp0 does not have enough executions\n");
       return -1;
     }
	   //output->log(STDERR, "bp0 OK\n");
     if (func_counts[1][i] != NUM_BREAKPOINT_SPINS) {
       output->log(STDERR, "bp1 does not have enough executions\n");
       return -1;
     }
	   //output->log(STDERR, "bp1 OK\n");
     if (func_counts[2][i] != NUM_BREAKPOINT_SPINS) {
       output->log(STDERR, "bp2 does not have enough executions\n");
       return -1;
     }
	   //output->log(STDERR, "bp2 OK\n");
     if (func_counts[3][i] != NUM_BREAKPOINT_SPINS) {
       output->log(STDERR, "bp3 does not have enough executions\n");
       return -1;
     }
	   //output->log(STDERR, "bp3 OK\n");
   }

   test_passes(testname);
   //output->log(STDERR, "pc_breakpoint_mutatee all OK\n");
   return 0;
}
