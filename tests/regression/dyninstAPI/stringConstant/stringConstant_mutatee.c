#include <stdlib.h>
#include <string.h>

// The mutator inserts calls to probe(<string constant>) into target. main
// gets the string the calls must pass and the number of calls expected.

static char const *expected;
static int calls;
static int mismatches;

__attribute__((noinline, used)) void probe(char const *name) {
  ++calls;
  if (!name || strcmp(name, expected) != 0) ++mismatches;
}

__attribute__((noinline, used)) int target(int x) { return x + 1; }

int main(int argc, char **argv) {
  if (argc != 3) return EXIT_FAILURE;
  expected = argv[1];
  int const want = atoi(argv[2]);
  if (target(argc) != argc + 1) return EXIT_FAILURE;
  return calls == want && mismatches == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
