/*
 * Stands in for the test where it cannot be built or run, so that ctest
 * still runs it and reports it as skipped, with the reason.
 */
#include <stdio.h>

int main(void) {
  printf("skipping: %s\n", SKIP_REASON);
  return 77;
}
