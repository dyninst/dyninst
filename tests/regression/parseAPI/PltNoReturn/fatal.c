#include "plt_noret.h"

int plt_noret_sink;

/* The compiler does not know that fatal_inner never returns, so it emits code after the call. */
void fatal(void) {
  fatal_inner();
  plt_noret_sink = 1;
}
