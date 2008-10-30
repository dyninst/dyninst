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

static Thread_t test4_threads[TEST3_THREADS];
static Lock_t test4lock;
static int mutateeIdle = 0;

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
