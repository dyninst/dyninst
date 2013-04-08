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

Thread_t test4_threads[TEST3_THREADS];
Lock_t test4lock;
int mutateeIdle = 0;

/* Function definitions follow */

void *thread_main4(void *arg)
{
  long x, i;
  arg = NULL; 
  lockLock(&test4lock); 
  x = 0;

  for (i = 0; i < 0xf; ++i) {
    x = x + i;
  }

  unlockLock(&test4lock); 
  dprintf("%s[%d]:  %p exiting...\n", __FILE__, __LINE__, 
	  (void *) pthread_self());
  return (void *) x;
}

void func4_1()
{
  dprintf("%s[%d]:  welcome to func4_1\n", __FILE__, __LINE__);
  createLock(&test4lock);


  lockLock(&test4lock); 
   
  assert (NULL != createThreads(TEST3_THREADS, thread_main4, test4_threads));

  unlockLock(&test4lock); 
  mutateeIdle = 1;
  while (mutateeIdle) {}
}

int test_thread_3_mutatee() {
  func4_1();
  return 0; /* Return code for mutatee is not checked */
}
