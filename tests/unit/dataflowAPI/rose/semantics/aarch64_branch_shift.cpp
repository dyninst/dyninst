// Branch-register linking and rotated register operands.
#include "aarch64_semantics.h"

#include <cstdlib>

int main() {
  using rose_semantics_aarch64::test_case;
  namespace a64 = Dyninst::aarch64;

  // clang-format off
  std::array<test_case, 3> const cases = {{
    // BR does not link; BLR does.
    {"br    x17",                0xD61F0220, a64::x30, false, nullptr},
    {"blr   x8",                 0xD63F0100, a64::x30, true,  nullptr},

    // A ROR-shifted register operand is read, not rejected.
    {"eor   x0, x1, x2, ror #8", 0xCAC22020, a64::x0,  true,  "<rotR>"},
  }};
  // clang-format on

  return rose_semantics_aarch64::check(cases) ? EXIT_SUCCESS : EXIT_FAILURE;
}
