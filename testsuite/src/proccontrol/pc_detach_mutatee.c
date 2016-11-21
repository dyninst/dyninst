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
#include "solo_mutatee_boilerplate.h"

static testlock_t init_lock;

static int num_signals = 0;

#if !defined(os_windows_test)
static void signal_handler(int sig)
{
   num_signals++;
}

#endif

#if defined(os_linux_test) && !defined(os_bg_test)
#include <unistd.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <signal.h>
#include <unistd.h>
#include <errno.h>

static void self_signal()
{
  static int gettid_not_valid = 0;
  static int has_tkill = 1;
  long int result, pid;

  /* signal(SIGUSR1, signal_handler); */

  if (gettid_not_valid) {
     pid = getpid();
  }
  else {
     pid = syscall(SYS_gettid);
     if (pid == -1 && errno == ENOSYS)
     {
        gettid_not_valid = 1;
        pid = getpid();
     }
  }

  if (has_tkill) {
     result = syscall(SYS_tkill, pid, SIGUSR1);
     if (result == -1 && errno == ENOSYS) {
        has_tkill = 0;
     }
  }
  if (!has_tkill) {
     result = kill(pid, SIGUSR1);
  }
}

#elif defined(os_windows_test)

static HANDLE am_signaled;

static void signal_handler(int sig)
{
WaitForSingleObject(am_signaled, 30000);
   num_signals++;
}


static void self_signal()
{
	SetEvent(am_signaled);
}

#else


#include <sys/types.h>
#include <signal.h>
#include <unistd.h>

static void self_signal()
{
   kill(getpid(), SIGUSR1);
}

#endif

static int threadFunc(int myid, void *data)
{

   testLock(&init_lock);
#if !defined(os_bg_test)
   self_signal();
#endif
   testUnlock(&init_lock);
   return 0;
}

//Basic test for create/attach and exit.
int pc_detach_mutatee()
{
   syncloc syncloc_msg;
   int result;
   int total_num_signals;

#if !defined(os_windows_test)
  struct sigaction action;

  action.sa_handler = signal_handler;
  sigemptyset(&action.sa_mask);
  action.sa_flags = 0;
  sigaction(SIGUSR1, &action, NULL);
#else
   am_signaled = CreateEvent(NULL, FALSE, FALSE, NULL);
#endif

   num_signals = 0;
   syncloc_msg.code = 0;

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
      output->log(STDERR, "Incorrect sync code: %x\n", syncloc_msg.code);
      return -1;
   }
   self_signal();
   testUnlock(&init_lock);

   result = finiProcControlTest(0);
   if (result != 0) {
      output->log(STDERR, "Finalization failed\n");
      return -1;
   }
#if defined(os_bg_test)
   total_num_signals = 1;
#else
   total_num_signals = num_threads+1;
#endif
   if (num_signals != total_num_signals)
   {
      output->log(STDERR, "Incorrect number of signals recieved\n");
      return -1;
   }

   result = send_message((unsigned char *) &syncloc_msg, sizeof(syncloc));
   if (result != 0) {
      output->log(STDERR, "Failed to send sync message\n");
      return -1;
   }

   test_passes(testname);
   return 0;
}
