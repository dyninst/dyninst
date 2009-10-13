#include "mutatee_util.h"

int test_snip_remove_var =  0;
int test_snip_remove_func() {return 5;}

int test_snip_remove_mutatee() {
  test_snip_remove_func();
  if (test_snip_remove_var == 2) {
    test_passes(testname);
    return 0; /* Test passed */
  } else {
    return -1; /* Test failed */
  }
}

