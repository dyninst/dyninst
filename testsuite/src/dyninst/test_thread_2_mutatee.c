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

#include "mutatee_util.h"
#include "solo_mutatee_boilerplate.h"
#include "test_thread.h"
#include "test12.h"

/* Externally accessed function prototypes.  These must have globally unique
 * names.  I suggest following the pattern <testname>_<function>
 */

/* Global variables accessed by the mutator.  These must have globally unique
 * names.
 */

/* Internally used function prototypes.  These should be declared with the
 * keyword static so they don't interfere with other mutatees in the group.
 */

/* Global variables used internally by the mutatee.  These should be declared
 * with the keyword static so they don't interfere with other mutatees in the
 * group.
 */

Thread_t test3_threads[TEST3_THREADS];
Lock_t test3lock;
int mutateeIdle = 0;

/* Function definitions follow */

void *thread_main3(void *arg)
{
  long x, i;
  arg = NULL; /*silence warnings*/
  lockLock(&test3lock);
  x = 0;

  for (i = 0; i < 0xffff; ++i) {
    x = x + i;
  }
  /*dprintf("%s[%d]:  PTHREAD_DESTROY\n", __FILE__, __LINE__); */

  unlockLock(&test3lock);
  dprintf("%s[%d]:  %lu exiting...\n", __FILE__, __LINE__, (unsigned long) pthread_self());
  return (void *) x;
}

void func3_1()
{
  createLock(&test3lock);
  mutateeIdle = 1;
  
  lockLock(&test3lock);
  assert (NULL != createThreads(TEST3_THREADS, thread_main3, test3_threads));

  sleep_ms(999);
  unlockLock(&test3lock);
  dprintf("%s[%d]:  func3_1\n", __FILE__, __LINE__);
  while (mutateeIdle) {}

  dprintf("%s[%d]:  leaving func3_1\n", __FILE__, __LINE__);
}

int test_thread_2_mutatee() {
  func3_1();
  fprintf(stderr, "Done with func3_1\n");
  return 0; /* Return code for this mutatee is not checked */
}
