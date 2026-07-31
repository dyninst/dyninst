// EH mutatee: a single try/catch of a class-type exception.
#include <cstdio>
#include <stdexcept>
__attribute__((noinline)) static void thrower() { throw std::runtime_error("boom"); }
int main() {
  try {
    thrower();
  } catch (const std::exception& e) {
    std::printf("CAUGHT-1 %s\n", e.what());
    return 0;
  }
  std::printf("NOT-CAUGHT-1\n");
  return 2;
}
