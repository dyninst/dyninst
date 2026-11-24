/*
 * far_plt_call rewrites this program so that work() calls the library's
 * far_plt_call_exit(EXIT_CODE) on entry.  Unrewritten, it exits with 1.
 */

/* Must match kExitCode in far_plt_call.cpp. */
#define EXIT_CODE 42

int far_plt_call_status(void);

__attribute__((noinline)) int work(void)
{
  return far_plt_call_status();
}

int main(void)
{
  return work();
}
