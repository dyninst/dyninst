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
#include <sys/syscall.h>
/* Externally accessed function prototypes.  These must have globally unique
 * names.  I suggest following the pattern <testname>_<function>
 */

/* Global variables accessed by the mutator.  These must have globally unique
 * names.
 */

volatile int test_thread_5_idle = 0;

/* Internally used function prototypes.  These should be declared with the
 * keyword static so they don't interfere with other mutatees in the group.
 */

/* Global variables used internally by the mutatee.  These should be declared
 * with the keyword static so they don't interfere with other mutatees in the
 * group.
 */

/* Function definitions follow */

void *thread_main8(void *arg)
{
  /*  The mutator will patch in messaging primitives to signal events at mutex creation,
      deletion, locking and unlocking.  Thus far, we are only considering reporting of events
      so actual contention is meaningless */
  Lock_t newmutex;
  arg = NULL;

  if (!createLock(&newmutex)) {
     logerror("%s[%d]:  createLock failed\n", __FILE__, __LINE__);
     return NULL;
  }
  sleep_ms(100);
  if (!lockLock(&newmutex)) {
     logerror("%s[%d]:  lockLock failed\n", __FILE__, __LINE__);
     return NULL;
  }
  sleep_ms(100);

  if (!unlockLock(&newmutex)) {
     logerror("%s[%d]:  unlockLock failed\n", __FILE__, __LINE__);
     return NULL;
  }
  sleep_ms(100); 

  if (!destroyLock(&newmutex)) {
     logerror("%s[%d]:  destroyLock failed\n", __FILE__, __LINE__);
     return NULL;
  }

  sleep(1);

  return NULL;
}

Thread_t test8threads[TEST8_THREADS];
void func8_1()
{
  Thread_t *threads = test8threads;

  threads = createThreads(TEST8_THREADS, thread_main8,threads);
  assert (threads);

  while (test_thread_5_idle == 0) {
    /* Do nothing */
  }

}

int test_thread_5_mutatee() {
  func8_1();
  return 0; /* Return code is not checked */
}
