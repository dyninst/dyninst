#include "mutatee_util.h"

int patch2_1_func() {return 5;}

int patch2_1_mutatee() {
  patch2_1_func();
  test_passes(testname);
  return 0;
}
