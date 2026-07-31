// EH mutatee: a stack object's destructor must run during unwinding (cleanup
// landing pad) before the handler catches -- exercises cleanup call-sites.
#include <cstdio>
#include <stdexcept>
static bool cleaned = false;
struct Guard { ~Guard() { cleaned = true; } };
__attribute__((noinline)) static void thrower() { throw std::runtime_error("unwind"); }
__attribute__((noinline)) static void middle() {
  Guard g;          // must be destroyed as the exception unwinds through here
  thrower();
}
int main() {
  try {
    middle();
  } catch (const std::exception& e) {
    if (!cleaned) { std::printf("NO-CLEANUP-5\n"); return 2; }
    std::printf("CAUGHT-5 %s\n", e.what());
    return 0;
  }
  std::printf("NOT-CAUGHT-5\n");
  return 2;
}
