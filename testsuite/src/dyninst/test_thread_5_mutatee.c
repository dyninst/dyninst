#include <assert.h>

#include "mutatee_util.h"
#include "test_thread.h"
#include "test12.h"

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
