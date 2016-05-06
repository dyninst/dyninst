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
#include "solo_mutatee_boilerplate.h"


static volatile int global = 3;

#define NUM_FUNCS 5

typedef int (*func_t)(int);
static func_t funcs[NUM_FUNCS];
static int myerror = 0;

int func1(int counter) {
   int i;
   for (i = 0; i < global; i++) {
      counter += i;
   }
   return counter;
}

int func2(int counter) {
   int i;
   for (i = 3; i < global+3; i++) {
      counter += i;
   }
   return counter;
}

int func3(int counter) {
   int i;
   for (i = 6; i < global+6; i++) {
      counter += i;
   }
   return counter;
}

int func4(int counter) {
   int i;
   for (i = 9; i < global+9; i++) {
      counter += i;
   }
   return counter;
}

int func5(int counter) {
   int i;
   for (i = 12; i < global+12; i++) {
      counter += i;
   }
   return counter;
}

void run_all_funcs()
{
   int result = 0;
   result = func1(result);
   result = func2(result);
   result = func3(result);
   result = func4(result);
   result = func5(result);
   if (result != 105) {
      output->log(STDERR, "Computation failed\n");
      myerror = 1;
   } else {
	   //fprintf(stderr, "run_all_funcs OK\n");
   }
}

static testlock_t init_lock;

static int threadFunc(int myid, void *data)
{
   data = NULL;

   testLock(&init_lock);
   testUnlock(&init_lock);

   run_all_funcs();

   return 0;
}

//Basic test for create/attach and exit.
int pc_singlestep_mutatee()
{
   int result;
   unsigned i;
   send_addr addr_msg;
   syncloc msg;
   myerror = 0;
   initLock(&init_lock);
   testLock(&init_lock);

   result = initProcControlTest(threadFunc, NULL);
   if (result != 0) {
      output->log(STDERR, "Initialization failed\n");
      testUnlock(&init_lock);
      return -1;
   }

   funcs[0] = func1;
   funcs[1] = func2;
   funcs[2] = func3;
   funcs[3] = func4;
   funcs[4] = func5;

   addr_msg.code = SENDADDR_CODE;

   addr_msg.addr = getFunctionPtr((intptr_t *)run_all_funcs);
   result = send_message((unsigned char *) &addr_msg, sizeof(addr_msg));
   if (result == -1) {
	   output->log(STDERR, "Failed to send addr message for initial breakpoint func\n");
	   testUnlock(&init_lock);
	   return -1;
   }

   for (i = 0; i < NUM_FUNCS; i++) {
      addr_msg.addr = getFunctionPtr((intptr_t *)funcs[i]);
      result = send_message((unsigned char *) &addr_msg, sizeof(addr_msg));
      if (result == -1) {
         output->log(STDERR, "Failed to send addr message\n");
		 testUnlock(&init_lock);
         return -1;
      }
   }

   //mutatee stops here
   //this is the dead code in aarch64
   result = recv_message((unsigned char *) &msg, sizeof(syncloc));
   if (result == -1) {
      output->log(STDERR, "Failed to recv sync message\n");
      testUnlock(&init_lock);
      return -1;
   }

   if (msg.code != SYNCLOC_CODE) {
      output->log(STDERR, "Received unexpected sync message (singlestep)\n");
      testUnlock(&init_lock);
      return -1;
   }

   testUnlock(&init_lock);
   run_all_funcs();

   result = finiProcControlTest(0);
   if (result != 0) {
      output->log(STDERR, "Finalization failed\n");
      return -1;
   }

   msg.code = SYNCLOC_CODE;
   result = send_message((unsigned char *) &msg, sizeof(syncloc));
   if (result == -1) {
      output->log(STDERR, "Failed to send sync message\n");
      return -1;
   }

   if (myerror == 0) {
      test_passes(testname);
      return 0;
   }
   return -1;
}
