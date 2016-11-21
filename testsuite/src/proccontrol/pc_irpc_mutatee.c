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

static int myerror = 0;
static int done = 0;
static int busywait = 0;

static testlock_t val_lock;
volatile uint32_t val = 0;
int irpc_calltarg()
{
   testLock(&val_lock);
   val++;
   testUnlock(&val_lock);
   return 4;
}

static testlock_t init_lock;

static int threadFunc(int myid, void *data)
{
   data = NULL;
   myid = 0;

#if defined(os_windows_test)
   while (!done) ;
#endif

   testLock(&init_lock);
   testUnlock(&init_lock);

   return 0;
}

//Basic test for create/attach and exit.
int pc_irpc_mutatee()
{
   int result;
   send_addr addr_msg;
   syncloc msg;
   myerror = 0;
   initLock(&init_lock);
   testLock(&init_lock);

   initLock(&val_lock);

   result = initProcControlTest(threadFunc, NULL);
   if (result != 0) {
      output->log(STDERR, "Initialization failed\n");
      testUnlock(&init_lock);
      return -1;
   }

   addr_msg.code = SENDADDR_CODE;
   addr_msg.addr = getFunctionPtr((intptr_t *)irpc_calltarg);
   result = send_message((unsigned char *) &addr_msg, sizeof(addr_msg));
   if (result == -1) {
      output->log(STDERR, "Failed to send func addr message\n");
      return -1;
   }

   // Only needed on ppc64 (ABIv1), nop on other architectures (including ppc64le/ABIv2)
   addr_msg.addr = getTOCValue((unsigned long *)irpc_calltarg);
   result = send_message((unsigned char *) &addr_msg, sizeof(addr_msg));
   if (result == -1) {
       output->log(STDERR, "Failed to send func addr message\n");
       return -1;
   }

   addr_msg.addr = (intptr_t) &val;
   result = send_message((unsigned char *) &addr_msg, sizeof(addr_msg));
   if (result == -1) {
      output->log(STDERR, "Failed to send val addr message\n");
      return -1;
   }

   addr_msg.addr = (intptr_t) &busywait;
   result = send_message((unsigned char *) &addr_msg, sizeof(addr_msg));
   if (result == -1) {
	   output->log(STDERR, "Failed to send busywait addr message\n");
	   return -1;
   }

#if defined(os_windows_test)
	while (!busywait) ;
#endif
   result = recv_message((unsigned char *) &msg, sizeof(syncloc));
   if (result == -1) {
      output->log(STDERR, "Failed to recv sync message\n");
      testUnlock(&init_lock);
      return -1;
   }

   if (msg.code != SYNCLOC_CODE) {
      output->log(STDERR, "Recieved unexpected sync message\n");
      testUnlock(&init_lock);
      return -1;
   }

   done = 1;
   testUnlock(&init_lock);

   result = finiProcControlTest(0);
   if (result != 0) {
      output->log(STDERR, "Finalization failed\n");
      return -1;
   }

   if (myerror == 0) {
      test_passes(testname);
      return 0;
   }
   return -1;
}
