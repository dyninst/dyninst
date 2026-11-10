#include <stdint.h>
#include <stdio.h>

// Set by the snippet the mutator inserts at answer's exit.
volatile long seen = -1;
volatile long seen_marker = -1;

// The value the mutator passes as record's second argument.
#define MARKER 0x0040000000000007L

void record(long value, long marker) {
  seen = value;
  seen_marker = marker;
}

// Not foldable into main, so answer keeps a real exit.
volatile long input = 41;

__attribute__((noinline)) long answer(long x) { return x * 3 + 1; }

static uint64_t read_fpcr(void) {
  uint64_t v;
  __asm__ volatile("mrs %0, fpcr" : "=r"(v));
  return v;
}

static void write_fpcr(uint64_t v) { __asm__ volatile("msr fpcr, %0" : : "r"(v)); }

int main(void) {
  // Round toward zero (RMode = 0b11): not the default, so a tramp that
  // restores FPCR from the wrong slot changes it.
  uint64_t const fpcr = read_fpcr() | (UINT64_C(3) << 22);
  write_fpcr(fpcr);

  long const result = answer(input);
  uint64_t const after = read_fpcr();

  int ok = 1;
  if(result != 124) {
    fprintf(stderr, "answer returned %ld, expected 124\n", result);
    ok = 0;
  }
  if(seen != result) {
    fprintf(stderr, "the exit snippet saw %ld, answer returned %ld\n", seen, result);
    ok = 0;
  }
  if(seen_marker != MARKER) {
    fprintf(stderr, "the exit snippet passed marker 0x%lx, expected 0x%lx\n", seen_marker, MARKER);
    ok = 0;
  }
  if(after != fpcr) {
    fprintf(stderr, "FPCR is 0x%llx after the call, was 0x%llx\n", (unsigned long long)after,
            (unsigned long long)fpcr);
    ok = 0;
  }
  return ok ? 0 : 1;
}
