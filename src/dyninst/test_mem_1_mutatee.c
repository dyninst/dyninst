#include "test_mem_util.h"
#include "mutatee_util.h"

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

int test_mem_1_passed = 0;

/* Function definitions follow */

void check1()
{
  if (loadCnt == loadExp) {
    logerror("Passed test_mem_1 (load instrumentation)\n");
    test_mem_1_passed = 1;
  } else {
    logerror("**Failed** test_mem_1 (load instrumentation)\n");
    logerror("    load counter seems wrong\n");
  }
}

/* skeleton test doesn't do anything besides say that it passed */
int test_mem_1_mutatee() {
  loadCnt = 0;
  storeCnt = 0;
  prefeCnt = 0;
  accessCnt = 0;

  if (setupFortranOutput()) {
    logstatus("Error redirecting assembly component output to log file\n");
  }

  result_of_loadsnstores = loadsnstores(2,3,4);
  dprintf("\nresult=0x%x loads=%d stores=%d prefetches=%d accesses=%d\n",
          result_of_loadsnstores, loadCnt, storeCnt, prefeCnt, accessCnt);

  init_test_data();

  check1();

  if (cleanupFortranOutput()) {
    logstatus("Error restoring output to stdout\n");
  }
  
  if (test_mem_1_passed) {
    test_passes(testname);
    return 0; /* No error */
  } else {
    return -1; /* Error */
  }
}
