/*
 * The library far_plt_call's mutatee calls.  Each function here has one
 * name, so the rewriter can't bind the instrumentation's call to an alias
 * that the library doesn't export, as it can with libc's _exit.
 */

#include <unistd.h>

int far_plt_call_status(void)
{
  return 1;
}

void far_plt_call_exit(int code)
{
  _exit(code);
}
