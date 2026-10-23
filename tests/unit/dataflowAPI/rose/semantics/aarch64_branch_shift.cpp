// Branch-register linking and rotated register operands.
#include "aarch64_semantics.h"

#include <cstdlib>

int main() {
  using rose_semantics_aarch64::reg;
  using rose_semantics_aarch64::test_case;
  namespace a64 = Dyninst::aarch64;

  // clang-format off
  test_case const cases[] = {
    // BR branches without linking; BL and BLR link to the next instruction.
    {"br    x17",                0xD61F0220, reg(a64::x30), false, nullptr,  nullptr},
    {"br    x17",                0xD61F0220, reg(a64::pc),  true,  "x17",    nullptr},
    {"blr   x8",                 0xD63F0100, reg(a64::x30), true,  "<add>",  nullptr},
    {"bl    pc+8",               0x94000002, reg(a64::x30), true,  "<add>",  nullptr},

    // A ROR-shifted register operand is read, not rejected, for X and W operands.
    {"eor   x0, x1, x2, ror #8", 0xCAC22020, reg(a64::x0),  true,  "<rotR>", nullptr},
    {"eor   w0, w1, w2, ror #8", 0x4AC22020, reg(a64::w0),  true,  "<rotR>", nullptr},
  };
  // clang-format on

  return rose_semantics_aarch64::check(cases) ? EXIT_SUCCESS : EXIT_FAILURE;
}
