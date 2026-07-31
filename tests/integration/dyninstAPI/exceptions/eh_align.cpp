// EH mutatee: the hardest case. alignas(64) stack buffers force the compiler
// to realign the stack (and, at -O2, to hot/cold-split the throwing and
// catching functions). The throw then unwinds through frames whose relocated
// FDE/LSDA must be assembled per-run and whose personality must be the C++
// one -- the combination that regressed on SPEC 620.omnetpp_s.
#include <cstdio>
#include <stdexcept>
__attribute__((noinline)) static void sink(char* p) { p[0] = 1; asm volatile("" ::: "memory"); }
__attribute__((noinline)) static void thrower() {
  alignas(64) char buf[64];
  sink(buf);
  throw std::runtime_error("align");
}
__attribute__((noinline)) static void middle() {
  alignas(64) char buf[128];
  sink(buf);
  thrower();
}
int main() {
  alignas(64) char buf[64];
  sink(buf);
  try {
    middle();
  } catch (const std::exception& e) {
    std::printf("CAUGHT-9 %s\n", e.what());
    return 0;
  }
  std::printf("NOT-CAUGHT-9\n");
  return 2;
}
