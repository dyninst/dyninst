#include <stdio.h>

// The number of calls the mutator inserts: two at the entry of each of
// first, second and third.
#define NUM_CALLS 6

volatile int calls = 0;
volatile int bad = 0;

// The mutator passes (id, id + 0x100, id + 0x200, id + 0x300).
void check(long id, long a, long b, long c) {
  calls++;
  if(a != id + 0x100 || b != id + 0x200 || c != id + 0x300) {
    fprintf(stderr, "call %ld got (%ld, 0x%lx, 0x%lx, 0x%lx)\n", id, id, a, b, c);
    bad++;
  }
}

// GCC's interprocedural register allocation lets main keep values in
// caller-saved registers across calls to functions it knows leave them alone.
// The inserted calls clobber those registers, as the ABI allows, so keep GCC
// to the ABI here.
#if defined(__GNUC__) && !defined(__clang__)
#  define NOT_INLINED __attribute__((noipa))
#else
#  define NOT_INLINED __attribute__((noinline))
#endif

NOT_INLINED long first(long x) { return x + 1; }
NOT_INLINED long second(long x, long y) { return x * y; }
NOT_INLINED long third(long x, long y, long z) { return x - y + z; }

// Not foldable into main, so the functions keep real calls.
volatile long input = 5;

int main(void) {
  long const r = first(input) + second(input, input) + third(input, input, input);
  if(r != 6 + 25 + 5) {
    fprintf(stderr, "the instrumented functions returned %ld, expected 36\n", r);
    return 1;
  }
  if(calls != NUM_CALLS || bad != 0) {
    fprintf(stderr, "check ran %d times, expected %d; %d calls had wrong arguments\n", calls,
            NUM_CALLS, bad);
    return 1;
  }
  return 0;
}
