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

static testlock_t init_lock;

static int num_signals = 0;

static int threadFunc(int myid, void *data)
{
   testLock(&init_lock);
   return 0;
}

//Basic test for create/attach and exit.
int pc_terminate_mutatee()
{
   syncloc syncloc_msg;
   int result;
   num_signals = 0;

   initLock(&init_lock);
   testLock(&init_lock);

   result = initProcControlTest(threadFunc, NULL);
   if (result != 0) {
      output->log(STDERR, "Initialization failed\n");
      return -1;
   }

   syncloc_msg.code = SYNCLOC_CODE;
   result = send_message((unsigned char *) &syncloc_msg, sizeof(syncloc));
   if (result != 0) {
      output->log(STDERR, "Failed to send syncloc_msg\n");
      return -1;
   }

   test_passes(testname);

   /* HACK: block here forever; mutator will time out if terminate() fails */
   while(1) {}

   result = recv_message((unsigned char *) &syncloc_msg, sizeof(syncloc));
   if (result != 0) {
      output->log(STDERR, "Failed to recieve sync message\n");
      return -1;
   }

   /* Should not reach this point, test needs to terminate */
   output->log(STDERR, "Test was not terminated\n");
   return -1;
}
