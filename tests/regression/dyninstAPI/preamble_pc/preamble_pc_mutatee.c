/* main reaches its globals and libc through the TOC (r2), which its global
   entry point sets up from r12. The mutator increments 'entered' at main's
   entry. */
#include <stdio.h>
#include <string.h>

int entered;
static char const *name;

__attribute__((noinline)) static void set_name(char const *argv0) {
  char const *slash = strrchr(argv0, '/');
  name = slash ? slash + 1 : argv0;
}

int main(int argc, char **argv) {
  (void)argc;
  set_name(argv[0]);
  printf("%s: entered %d\n", name, entered);
  return entered == 1 ? 0 : 2;
}
