#include "mutatee_util.h"

static int patch3_1_var1 = 0;
static int patch3_1_var2 = 0;
static int patch3_1_var3 = 0;

void patch3_1_call1_1() {
  patch3_1_var1 = 1;
}

void patch3_1_call1() {
  patch3_1_var1 = 2;
}

void patch3_1_call2() {
  patch3_1_var2 = 1;
}

void patch3_1_call3(int* var) {
  *var = 3;
}

int patch3_1_func() {
  patch3_1_call1();
  patch3_1_call2();
  patch3_1_call3(&patch3_1_var3);
  return 0;
}

int patch3_1_mutatee() {
  patch3_1_func();
  test_passes(testname);
  return 0;
}
