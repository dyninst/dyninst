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

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>

#include "pcontrol_mutatee_tools.h"
#include "solo_mutatee_boilerplate.h"
#include "communication.h"

extern void *ThreadTrampoline(void *d);
#if !defined(os_windows_test)
__thread int thread_test_tls = 0;
#else
// replace this with TLSAlloced variable
int thread_test_tls = 0;
#endif

testlock_t sendlock;
testlock_t synclock;

#if defined(os_linux_test) || defined(os_bg_test)
#include <sys/types.h>
#include <sys/syscall.h>

int getlwp()
{
  static int gettid_not_valid = 0;
  long int result;

  if (gettid_not_valid)
    return getpid();

  result = syscall(SYS_gettid);
  if (result == -1 && errno == ENOSYS)
  {
    gettid_not_valid = 1;
    return getpid();
  }
  return (int) result;
}
#endif

#if defined(os_freebsd_test)
#include <sys/types.h>
#include <sys/syscall.h>

int getlwp()
{
    static int gettid_not_valid = 0;
    int result;

    if( gettid_not_valid )
        return getpid();

    lwpid_t lwp_id;
    result = syscall(SYS_thr_self, &lwp_id);
    if( result && errno == ENOSYS ) {
        gettid_not_valid = 1;
        return getpid();
    }

    return lwp_id;
}
#endif

#if defined(os_windows_test)

int getlwp()
{
	return GetCurrentThreadId();
}
#endif

int num_msgs_sent = 0;

static int sendThreadMsg(int initial_thrd)
{
   int result;
   threadinfo tinfo;
   tinfo.code = THREADINFO_CODE;
#if !defined(os_windows_test)
   tinfo.pid = (unsigned long) getpid();
   tinfo.lwp = (unsigned long) getlwp();
   tinfo.tid = (unsigned long) pthread_self();
#else
   tinfo.pid = GetCurrentProcessId();
   tinfo.tid = GetCurrentThreadId();
   tinfo.lwp = (HANDLE) GetCurrentThreadId();
#endif
   tinfo.a_stack_addr = (unsigned long) &tinfo;
   if (initial_thrd)
      tinfo.initial_func = (unsigned long) 0x0;
   else
      tinfo.initial_func = (unsigned long) &ThreadTrampoline;
   tinfo.tls_addr = (unsigned long) &thread_test_tls;

   testLock(&sendlock);
   result = send_message((unsigned char *) &tinfo, sizeof(tinfo));
   num_msgs_sent++;
   testUnlock(&sendlock);
   if (result == -1) {
      output->log(STDERR, "Failed to send threadinfo message\n");
      return -1;
   }
   return 0;
}

static int threadFunc(int myid, void *data)
{
   int result;
   
   result = sendThreadMsg(0);

   testLock(&synclock);
   testUnlock(&synclock);

   return result;
}

int pc_thread_mutatee()
{
   syncloc msg;
   int result;
   int ret_code;

   initLock(&synclock);
   initLock(&sendlock);

   testLock(&synclock);

   result = initProcControlTest(threadFunc, NULL);
   if (result != 0) {
      output->log(STDERR, "Initialization failed\n");
      return -1;
   }
   ret_code = sendThreadMsg(1);

#if defined(os_bgq_test)
   //BGQ has issues if we try the below recv while threads
   // are still doing the above send.  This loop waits until 
   // sends are done
   int local_num_msgs_sent = 0;
   do {
      testLock(&sendlock);
      local_num_msgs_sent = num_msgs_sent;
      testUnlock(&sendlock);      
   } while (local_num_msgs_sent != num_threads+1);
#endif

   result = recv_message((unsigned char *) &msg, sizeof(syncloc));
   if (result == -1) {
      output->log(STDERR, "Failed to recv sync message\n");
      return -1;
   }
   if (msg.code != SYNCLOC_CODE) {
      output->log(STDERR, "Recieved unexpected sync message\n");
      return -1;
   }
   testUnlock(&synclock);

   result = finiProcControlTest(0);
   if (result != 0) {
      output->log(STDERR, "Finalization failed\n");
      return -1;
   }

   test_passes(testname);
   return ret_code;
}
