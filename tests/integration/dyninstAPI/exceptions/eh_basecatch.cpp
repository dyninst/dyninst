// EH mutatee: throw a derived type, catch by base reference (polymorphic
// match through the type table, not an exact-type match).
#include <cstdio>
struct Base { virtual ~Base() {} virtual const char* tag() const { return "base"; } };
struct Derived : Base { const char* tag() const override { return "term"; } };
__attribute__((noinline)) static void thrower() { throw Derived{}; }
int main() {
  try {
    thrower();
  } catch (const Base& b) {
    std::printf("CAUGHT-7 %s\n", b.tag());
    return 0;
  }
  std::printf("MISSED-7\n");
  return 2;
}
