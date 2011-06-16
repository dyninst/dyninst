#include "mutatee_util.h"

static int patch1_1_var = 0;

void patch1_1_call() {
    patch1_1_var = 2;
}

void patch1_1_func() {
    patch1_1_var = 1;
    patch1_1_call();
    patch1_1_var = 3;
}

int patch1_1_mutatee() {
  patch1_1_func();
  test_passes(testname);
  return 0;
}
