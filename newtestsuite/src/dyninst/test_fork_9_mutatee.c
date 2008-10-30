#include <sys/types.h>
#include <unistd.h>

#include "mutatee_util.h"

/* Externally accessed function prototypes.  These must have globally unique
 * names.  I suggest following the pattern <testname>_<function>
 */

void test_fork_9_func1();

/* Global variables accessed by the mutator.  These must have globally unique
 * names.
 */

int test_fork_9_global1 = 7;

/* Internally used function prototypes.  These should be declared with the
 * keyword static so they don't interfere with other mutatees in the group.
 */

/* Global variables used internally by the mutatee.  These should be declared
 * with the keyword static so they don't interfere with other mutatees in the
 * group.
 */

static int dummyVal = 0;

/* Function definitions follow */

void test_fork_9_func1() { 
  dummyVal += 10;
}

int test_fork_9_mutatee() {
#if defined(i386_unknown_nt4_0)
  return 0;
#endif
  int pid;
  /* dprintf("mutatee:  starting fork\n"); */
  pid = fork();
  /* dprintf("mutatee:  stopping fork\n"); */

  /* mutatee will get paused here, temporarily, when the mutator receives
     the postForkCallback */

  if (pid == 0) {   /* child */
    dprintf("Child: starting tests\n");
    test_fork_9_func1();
    dprintf("Child: done with tests, exiting\n");
  } else if(pid > 0) {
    registerPID(pid); /* Register for cleanup */
    dprintf("Parent: starting tests\n");
    test_fork_9_func1();
    dprintf("Parent: done with tests, exiting\n");
  } else if(pid < 0) {
    logerror("error on fork\n");
    return -1;  /* error case */
  }

  return 0; /* Mutatee return code is not checked for this test */
}
