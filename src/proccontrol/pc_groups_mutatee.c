/*
 * Copyright (c) 1996-2011 Barton P. Miller
 * 
 * We provide the Paradyn Parallel Performance Tools (below
 * described as "Paradyn") on an AS IS basis, and do not warrant its
 * validity or performance.  We reserve the right to update, modify,
 * or discontinue this software at any time.  We shall have no
 * obligation to supply such updates or modifications or any other
 * form of support to you.
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

static testbarrier_t barrier;
static testlock_t init_lock;
static int error = 0;

extern int num_threads;

volatile int do_nothing;
uint64_t data;

void bp_func()
{
   do_nothing++;
}

#if defined(os_bgq_test)
static void *findUnallocatedMemory() {
   return (void *) 0x4000;
}
#elif !defined(os_windows_test)
/* Need this for MAP_ANONYMOUS on ppc32-linux */
#define __USE_MISC
#include <sys/mman.h>
#undef __USE_MISC
static void *findUnallocatedMemory() {
   //Return something the mutator can pass to mallocMemory
   void *result;
   int iresult;
   unsigned pagesize = getpagesize();
   result = mmap(NULL, pagesize, PROT_READ|PROT_WRITE,
                 MAP_ANONYMOUS | MAP_PRIVATE, 0, 0);
   if (result == (void *) -1) {
      perror("mmap failure");
      output->log(STDERR, "Failed to mmap memory\n");
      return NULL;
   }
   iresult = munmap(result, pagesize);
   if (iresult == -1) {
      output->log(STDERR, "Failed to unmap memory\n");
      return NULL;
   }
   //This memory in now guarenteed available and unmapped.
   return result;
}
#else
static void *findUnallocatedMemory() {
	// Do the same allocate/deallocate trick as above
	void *result;
	unsigned pagesize;
	SYSTEM_INFO sysinfo;
	GetSystemInfo(&sysinfo);
	pagesize = sysinfo.dwPageSize;

	result = VirtualAlloc(NULL, pagesize, MEM_RESERVE, PAGE_EXECUTE_READWRITE);
	return result;
}

#endif

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

static int threadFunc(int myid, void *data)
{
   unsigned i=0;
   testLock(&init_lock);
   testUnlock(&init_lock);

   waitTestBarrier(&barrier);
   
   trigger_sync(myid);
   if (error) return 0;

   waitfor_sync(myid);
   if (error) return 0;
   waitTestBarrier(&barrier);

   for (i=0; i<2; i++) {
      bp_func();
      
      waitTestBarrier(&barrier);
      trigger_sync(myid);
      if (error) return 0;

      waitfor_sync(myid);
      if (error) return 0;
      waitTestBarrier(&barrier);
   }
   
   return 0;
}

int pc_groups_mutatee()
{
   int result;
   void *fmem;
 
   error = 0;
   initLock(&init_lock);
   testLock(&init_lock);

   result = initProcControlTest(threadFunc, NULL);
   if (result != 0) {
      output->log(STDERR, "Initialization failed\n");
      return -1;
   }

   initBarrier(&barrier, num_threads+1);

   data = 4;

   send_addrs(&data);
   send_addrs((void *) bp_func);
   fmem = findUnallocatedMemory();
   if (!fmem) {
      return -1;
   }
   send_addrs(fmem);
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
