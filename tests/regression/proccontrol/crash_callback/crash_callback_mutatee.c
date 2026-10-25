/* Crashes with SIGSEGV after a short sleep, so that the mutator is in its
   polling loop, not inside BPatch's event handling, when ProcControl sees the
   crash. */
#include <unistd.h>

__attribute__((noinline)) static void crash(volatile int *p) { *p = 1; }

int main(void) {
  sleep(2);
  crash((volatile int *)0);
  return 0;
}
