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

static testbarrier_t barrier;
static testlock_t init_lock;
static int error = 0;

extern int num_threads;

#if !defined(os_windows_test)
#include "time.h"
static void microsleep()
{
   struct timespec t;
   t.tv_sec = 0;
   t.tv_nsec = 10000000;
   nanosleep(&t, NULL);
}
#else
#include <windows.h>
static void microsleep()
{
	Sleep(1);
}
#endif


static void send_addrs(void *a)
{
   send_addr saddr;
   int result;
   saddr.code = SENDADDR_CODE;
   saddr.addr = (uint64_t) ((unsigned long) a);
   result = send_message((unsigned char *) &saddr, sizeof(send_addr));
   if (result != 0) {
      output->log(STDERR, "Send addr code failed");
      error = 1;
   }
}

static int waitfor_sync(int myid) {
   syncloc msg;
   int result;

   if (myid != -1)
      return 0;

   result = recv_message((unsigned char *) &msg, sizeof(syncloc));
   if (result != 0) {
      output->log(STDERR, "Mutatee failed to send message\n");
      error = 1;
      return -1;
   }
   if (msg.code != SYNCLOC_CODE) {
      output->log(STDERR, "Mutatee received unexpected non-sync message\n");
      error = 1;
      return -1;
   }
   return 0;
}

static int trigger_sync(int myid) {
   syncloc msg;
   int result;

   if (myid != -1)
      return 0;

   msg.code = SYNCLOC_CODE;
   result = send_message((unsigned char *) &msg, sizeof(syncloc));
   if (result == -1)
      error = 1;
   return result;
}

static volatile uint32_t spin_here = 0;

static int threadFunc(int myid, void *data)
{
   unsigned i=0;
   int timeout;
   testLock(&init_lock);
   testUnlock(&init_lock);

   for (i=0; i<10; i++) {
      spin_here = 0;
      waitTestBarrier(&barrier);
      trigger_sync(myid);
      if (error) return 0;
      
      timeout = 6000; //60 seconds
      while (!spin_here) {
         microsleep();
         if (timeout-- <= 0) {
            output->log(STDERR, "Timeout waiting for stackwalk\n");
            error = 1;
            return -1;
         }
      }
      waitTestBarrier(&barrier);
   }
   
   return 0;
}

int pc_stat_mutatee()
{
   int result;

   error = 0;
   initLock(&init_lock);
   testLock(&init_lock);

   result = initProcControlTest(threadFunc, NULL);
   if (result != 0) {
      output->log(STDERR, "Initialization failed\n");
      return -1;
   }

   send_addrs((void *) &spin_here);
   initBarrier(&barrier, num_threads+1);
   testUnlock(&init_lock);

   threadFunc(-1, NULL);

   result = finiProcControlTest(0);
   if (result != 0) {
      output->log(STDERR, "Finalization failed\n");
      return -1;
   }
   
   if (!error) {
      test_passes(testname);
      return 0;
   }
   return -1;
}
