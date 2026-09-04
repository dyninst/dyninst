// EH mutatee: catch then re-throw; an outer frame must catch the re-thrown
// exception (exercises unwinding resumed from a landing pad).
#include <cstdio>
#include <stdexcept>
__attribute__((noinline)) static void thrower() { throw std::runtime_error("again"); }
__attribute__((noinline)) static void middle() {
  try {
    thrower();
  } catch (const std::exception&) {
    throw;  // re-throw to the caller
  }
}
int main() {
  try {
    middle();
  } catch (const std::exception& e) {
    std::printf("CAUGHT-4 %s\n", e.what());
    return 0;
  }
  std::printf("MISSED-4\n");
  return 2;
}
