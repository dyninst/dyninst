// EH mutatee: multiple catch clauses; the matching (non-first) one must fire.
#include <cstdio>
#include <stdexcept>
struct Pick { const char* w; };
__attribute__((noinline)) static void thrower() { throw Pick{"pick-me"}; }
int main() {
  try {
    thrower();
  } catch (const std::runtime_error& e) {
    std::printf("WRONG-runtime %s\n", e.what());
    return 2;
  } catch (int) {
    std::printf("WRONG-int\n");
    return 2;
  } catch (const Pick& p) {
    std::printf("CAUGHT-2 %s\n", p.w);
    return 0;
  }
  std::printf("MISSED-2\n");
  return 2;
}
