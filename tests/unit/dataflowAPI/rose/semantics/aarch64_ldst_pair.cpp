// Load/store pairs.
#include "aarch64_semantics.h"

#include <cstdlib>

int main() {
  using rose_semantics_aarch64::test_case;
  namespace a64 = Dyninst::aarch64;

  // clang-format off
  std::array<test_case, 8> const cases = {{
    // Both registers, 64 bits each, the second 8 bytes on; the base register is written
    // back only by the pre- and post-index forms.
    {"ldp   x29, x30, [sp], #16",   0xA8C17BFD, a64::x29, true,  "deref:64"},
    {"ldp   x29, x30, [sp], #16",   0xA8C17BFD, a64::x30, true,  "<8:"},
    {"ldp   x29, x30, [sp], #16",   0xA8C17BFD, a64::sp,  true,  "<16:"},
    {"stp   x29, x30, [sp, #-16]!", 0xA9BF7BFD, a64::x29, false, nullptr},
    {"stp   x29, x30, [sp, #-16]!", 0xA9BF7BFD, a64::sp,  true,  "<add>"},
    {"ldp   x0, x1, [x2, #16]",     0xA9410440, a64::x2,  false, nullptr},
    {"ldpsw x0, x1, [x2]",          0x69400440, a64::x1,  true,  "signExtend"},
    {"ldnp  x0, x1, [x2, #16]",     0xA8410440, a64::x2,  false, nullptr},
  }};
  // clang-format on

  return rose_semantics_aarch64::check(cases) ? EXIT_SUCCESS : EXIT_FAILURE;
}
