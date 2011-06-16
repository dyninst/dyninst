#include "mutatee_util.h"

void patch1_2_func() {
  // do nothing
}

int patch1_2_mutatee() {
  patch1_2_func();
  test_passes(testname);
  return 0;
}
