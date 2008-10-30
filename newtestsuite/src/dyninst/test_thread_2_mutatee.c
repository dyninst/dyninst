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

/* Internally used function prototypes.  These should be declared with the
 * keyword static so they don't interfere with other mutatees in the group.
 */

/* Global variables used internally by the mutatee.  These should be declared
 * with the keyword static so they don't interfere with other mutatees in the
 * group.
 */

static Thread_t test3_threads[TEST3_THREADS];
static Lock_t test3lock;
static int mutateeIdle = 0;

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
  return 0; /* Return code for this mutatee is not checked */
}
