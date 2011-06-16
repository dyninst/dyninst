#include "mutatee_util.h"

#define PATCH1_3_C 7
#define PATCH1_3_D 6.4

static volatile int tc = PATCH1_3_C;
static volatile double td = PATCH1_3_D;
static int patch1_3_iter = 50;

int patch1_3_func(int *int_val, double *double_val) {
  int i, ret = 1;
  *int_val = tc+(tc+(tc+(tc+(tc+(tc+(tc+(tc+(tc+(tc+
             (tc+(tc+(tc+(tc+(tc+(tc+(tc+(tc+(tc+(tc+
             (tc+(tc+(tc+(tc+(tc+(tc+(tc+(tc+(tc+(tc+
             (tc+(tc+(tc+(tc+(tc+(tc+(tc+(tc+(tc+(tc+(tc
             ))))))))))))))))))))))))))))))))))))))));

  *double_val = td+(td+(td+(td+(td+(td+(td+(td+(td+(td+
               (td+(td+(td+(td+(td+(td+(td+(td+(td+(td+
               (td+(td+(td+(td+(td+(td+(td+(td+(td+(td+
               (td+(td+(td+(td+(td+(td+(td+(td+(td+(td+(td
               ))))))))))))))))))))))))))))))))))))))));
  for (i = 0; i < patch1_3_iter; i++) {
    ret *= 3;
    if (i % 2 == 1) {
      ret *= 5;
    } else if (i < 10) {
      ret *= 7;
    } else if (i > 20) {
      ret *= 11;
    }
  }

  return ret;
}

int patch1_3_mutatee() {
  int int_val = 0;
  double double_val = 0.0;
  patch1_3_func(&int_val, &double_val);
  test_passes(testname);
  return 0;
}
