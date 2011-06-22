#include "mutatee_util.h"

void patch3_2_call1() {}
void patch3_2_call2() {}
void patch3_2_call3() {}
void patch3_2_call7() {}


/* in libtestpatch1 */
extern void patch3_2_call5A();

/* in libtestpatch2 */
extern void patch3_2_call6();

int patch3_2_func() {
  /* do nothing */
  return 0;
}

int patch3_2_mutatee() {
  patch3_2_func();
  test_passes(testname);
  return 0;
}
