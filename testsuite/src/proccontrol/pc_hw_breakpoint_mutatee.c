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
#include <string.h>
#include <errno.h>
#include <sys/mman.h>

#include "pcontrol_mutatee_tools.h"
#include "mutatee_util.h"
#include "communication.h"
#include "solo_mutatee_boilerplate.h"

#define NUM_BREAKPOINT_SPINS 8

static testlock_t init_lock;

volatile uint32_t rw_bp;
volatile uint32_t r_bp;
volatile uint32_t w_bp;
volatile uint32_t rwx_bp[512];
#if defined(arch_64bit_test)
volatile uint64_t r2_bp;
volatile uint64_t w2_bp;
#else
volatile uint32_t r2_bp;
volatile uint32_t w2_bp;
#endif

static testbarrier_t *barrier;
static int had_error = 0;

int x_bp()
{
   return 4;
}

int x2_bp()
{
   return 5;
}

typedef int (*x_func_t)();

static void pre_op(int myid) {
   syncloc sync;
   int result;
   if (myid == -1 && !had_error) {
      result = recv_message((unsigned char *) &sync, sizeof(syncloc));
      if (result != 0) {
         output->log(STDERR, "Failed to receive sync message\n");
         had_error = 1;
      }
      if (sync.code != SYNCLOC_CODE) {
         output->log(STDERR, "Received unexpected non-sync message\n");
         had_error = 1;
      }
   }
   waitTestBarrier(barrier);
}

static void post_op(int myid) {
   syncloc sync;
   int result;
   waitTestBarrier(barrier);
   if (myid == -1 && !had_error) {
      sync.code = SYNCLOC_CODE;
      result = send_message((unsigned char *) &sync, sizeof(syncloc));
      if (result != 0) {
         output->log(STDERR, "Could not send sync message\n");
      }
   }
}

static int threadFunc(int myid, void *data)
{
  unsigned i;
  unsigned int local = 0;

  testLock(&init_lock);
  testUnlock(&init_lock);

  pre_op(myid);
  for (i=0; i<NUM_BREAKPOINT_SPINS; i++)
     rw_bp = rw_bp + 1;
  post_op(myid);

  pre_op(myid);
  for (i=0; i<NUM_BREAKPOINT_SPINS; i++)
     local += r_bp;
  post_op(myid);

  pre_op(myid);
  for (i=0; i<NUM_BREAKPOINT_SPINS; i++)
     w_bp = 1;
  post_op(myid);

  pre_op(myid);
  for (i=0; i<NUM_BREAKPOINT_SPINS; i++) {
     x_bp();
  }
  post_op(myid);

  pre_op(myid);
  for (i=0; i<NUM_BREAKPOINT_SPINS; i++) {
     ((x_func_t)(rwx_bp))();
  }
  waitTestBarrier(barrier);
  for (i=0; i<NUM_BREAKPOINT_SPINS; i++) {
     local += rwx_bp[0];
  }
  for (i=0; i<NUM_BREAKPOINT_SPINS; i++) {
     rwx_bp[0] = 1;
  }
  post_op(myid);

  pre_op(myid);
  volatile unsigned char *end_of_r2 = ((unsigned char *) &r2_bp) + sizeof(r2_bp) - 1;
  for (i=0; i<NUM_BREAKPOINT_SPINS; i++) {
     local += *end_of_r2 + 1;
  }
  post_op(myid);

  pre_op(myid);
  volatile unsigned char *end_of_w2 = ((unsigned char *) &w2_bp) + sizeof(w2_bp) - 1;
  for (i=0; i<NUM_BREAKPOINT_SPINS; i++) {
     *end_of_w2 = (unsigned char) 1;
     local += w2_bp;
  }
  post_op(myid);

  pre_op(myid);
  for (i=0; i<NUM_BREAKPOINT_SPINS; i++) {
     x2_bp();
  }
  post_op(myid);
  
  return 0;
}

int make_rwx_bp()
{
   unsigned char *x1 = (unsigned char *) x_bp;
   unsigned char *x2 = (unsigned char *) x2_bp;
   unsigned long pagesize, page, result;
   
   unsigned char *begin, *end;
   if (x1 < x2) {
      begin = x1;
      end = x2;
   }
   else {
      begin = x2;
      end = x1;
   }

   unsigned long size = ((unsigned long) end) - ((unsigned long) begin);
   if (size > sizeof(rwx_bp))
      size = sizeof(rwx_bp);
   memcpy((void *) rwx_bp, begin, size);

   pagesize = (unsigned long) getpagesize();
   page = ((unsigned long) rwx_bp) & ~(pagesize-1);
   result = mprotect((void *) page, pagesize, 07);
   if (result == -1) {
      int error = errno;
      output->log(STDERR, "Failed to mprotect rwx_page: %s\n", strerror(error));
      return -1;
   }
   return 0;
}

//Basic test for create/attach and exit.
int pc_hw_breakpoint_mutatee()
{
   int result;
   send_addr bp_addr_msg;

   had_error = 0;
   initLock(&init_lock);
   testLock(&init_lock);

   result = initProcControlTest(threadFunc, NULL);
   if (result != 0) {
      output->log(STDERR, "Initialization failed\n");
      return -1;
   }

   if (make_rwx_bp() == -1)
      return -1;

   barrier = (testbarrier_t *) malloc(sizeof(testbarrier_t));
   memset(barrier, 0, sizeof(testbarrier_t));
   initBarrier(barrier, num_threads+1);

   bp_addr_msg.code = (uint32_t) SENDADDR_CODE;
   bp_addr_msg.addr = (intptr_t) &rw_bp;
/* bp 0 */
   result = send_message((unsigned char *) &bp_addr_msg, sizeof(send_addr));
   if (result == 0) {
     bp_addr_msg.addr = (intptr_t) &r_bp;
/* bp 1 */
     result = send_message((unsigned char *) &bp_addr_msg, sizeof(send_addr));
   }
   if (result == 0) {
     bp_addr_msg.addr = (intptr_t) &w_bp;
/* bp 2 */
     result = send_message((unsigned char *) &bp_addr_msg, sizeof(send_addr)); 
   }
   if (result == 0) {
     bp_addr_msg.addr = getFunctionPtr((intptr_t *) x_bp);
/* bp 3 */
     result = send_message((unsigned char *) &bp_addr_msg, sizeof(send_addr));
   }
   if (result == 0) {
     bp_addr_msg.addr = getFunctionPtr((intptr_t *) rwx_bp);
/* bp 4 */
     result = send_message((unsigned char *) &bp_addr_msg, sizeof(send_addr));
   }
   if (result == 0) {
     bp_addr_msg.addr = getFunctionPtr((intptr_t *) &r2_bp);
/* bp 5 */
     result = send_message((unsigned char *) &bp_addr_msg, sizeof(send_addr));
   }
   if (result == 0) {
     bp_addr_msg.addr = getFunctionPtr((intptr_t *) &w2_bp);
/* bp 6 */
     result = send_message((unsigned char *) &bp_addr_msg, sizeof(send_addr));
   }
   if (result == 0) {
     bp_addr_msg.addr = getFunctionPtr((intptr_t *) x2_bp);
/* bp 7 */
     result = send_message((unsigned char *) &bp_addr_msg, sizeof(send_addr));
   }
   if (result != 0) {
     output->log(STDERR, "Failed to send breakpoint addresses\n");
     testUnlock(&init_lock);
     return -1;
   }

   testUnlock(&init_lock);
   threadFunc(-1, NULL);
    
   result = finiProcControlTest(0);
   if (result != 0) {
      output->log(STDERR, "Finalization failed\n");
      return -1;
   }

   if (!had_error) {
      test_passes(testname);
      return 0;
   }
   return -1;
}
