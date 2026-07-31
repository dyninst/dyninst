// EH mutatee: a deep (9-frame) non-tail-call stack; the exception must unwind
// through every intermediate frame's synthesized CFI to reach the handler.
#include <cstdio>
#include <stdexcept>
static void sink(volatile int* p) { *p = *p + 1; }
#define FRAME(n, next)                                                       \
  __attribute__((noinline)) static void frame##n() {                        \
    volatile int local = n; sink(&local); next(); sink(&local);             \
  }
__attribute__((noinline)) static void bottom() { throw std::runtime_error("deep8"); }
FRAME(1, bottom)
FRAME(2, frame1)
FRAME(3, frame2)
FRAME(4, frame3)
FRAME(5, frame4)
FRAME(6, frame5)
FRAME(7, frame6)
FRAME(8, frame7)
int main() {
  try {
    frame8();
  } catch (const std::exception& e) {
    std::printf("CAUGHT-8 %s\n", e.what());
    return 0;
  }
  std::printf("NOT-CAUGHT-8\n");
  return 2;
}
