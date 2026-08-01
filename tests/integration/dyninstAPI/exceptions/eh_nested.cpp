// EH mutatee: nested try/catch with type discrimination. The inner catch
// declines (rethrows implicitly via a non-matching type) and the outer,
// correctly-typed handler must fire -- exercises the action-chain / type-table.
#include <cstdio>
#include <stdexcept>
struct Inner {};
struct Outer { const char* w; };
__attribute__((noinline)) static void thrower() { throw Outer{"deep"}; }
int main() {
  try {
    try {
      thrower();
    } catch (const Inner&) {
      std::printf("WRONG-inner\n");
      return 2;
    }
  } catch (const Outer& o) {
    std::printf("CAUGHT-3 %s\n", o.w);
    return 0;
  }
  std::printf("MISSED-3\n");
  return 2;
}
