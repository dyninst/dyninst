// Single-register, literal and acquire/release loads and stores.
#include "aarch64_semantics.h"

#include <cstdlib>

int main() {
  using rose_semantics_aarch64::test_case;
  namespace a64 = Dyninst::aarch64;

  // clang-format off
  std::array<test_case, 16> const cases = {{
    // Only the pre- and post-index forms write the address back to the base register;
    // the unscaled (LDUR/STUR) and unprivileged (LDTR/STTR) forms do not.
    {"ldur  x0, [x1, #8]",   0xF8408020, a64::x1, false, nullptr},
    {"ldurb w0, [x1, #8]",   0x38408020, a64::x1, false, nullptr},
    {"stur  x0, [x1, #8]",   0xF8008020, a64::x1, false, nullptr},
    {"sturb w0, [x1, #8]",   0x38008020, a64::x1, false, nullptr},
    {"ldtr  x0, [x1, #8]",   0xF8408820, a64::x1, false, nullptr},
    {"sttr  x0, [x1, #8]",   0xF8008820, a64::x1, false, nullptr},
    {"ldr   x0, [x1], #8",   0xF8408420, a64::x1, true,  "<add>"},
    {"ldr   x0, [x1, #8]!",  0xF8408C20, a64::x1, true,  "<add>"},

    // Loads write their destination and stores do not, whatever bits 23:22 hold.
    {"ldrsb w0, [x1]",       0x39C00020, a64::w0, true,  "deref:8"},
    {"ldrsh w0, [x1]",       0x79C00020, a64::w0, true,  "deref:16"},
    {"ldr   w0, pc+8",       0x18000040, a64::w0, true,  "deref:32"},
    {"ldr   x0, pc+24",      0x580000C0, a64::x0, true,  "deref:64"},
    {"ldrsw x0, pc+8",       0x98000040, a64::x0, true,  "signExtend"},
    {"ldar  x0, [x1]",       0xC8DFFC20, a64::x0, true,  "deref:64"},
    {"ldar  w0, [x1]",       0x88DFFC20, a64::w0, true,  "deref:32"},
    {"stlr  x0, [x1]",       0xC89FFC20, a64::x0, false, nullptr},
  }};
  // clang-format on

  return rose_semantics_aarch64::check(cases) ? EXIT_SUCCESS : EXIT_FAILURE;
}
