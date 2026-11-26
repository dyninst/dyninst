#include "plt_noret.h"

int caller(void) {
  int const r = ok();
  fatal();
  return r + 1;
}

void tail(void) { fatal(); }
