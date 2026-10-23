// Single-register, literal and acquire/release loads and stores.
#include "aarch64_semantics.h"

#include <cstdlib>

int main() {
  using rose_semantics_aarch64::memory;
  using rose_semantics_aarch64::reg;
  using rose_semantics_aarch64::test_case;
  namespace a64 = Dyninst::aarch64;

  // clang-format off
  test_case const cases[] = {
    // Only the pre- and post-index forms write the address back to the base register;
    // the unscaled (LDUR/STUR) and unprivileged (LDTR/STTR) forms do not. Each "not
    // written" row is paired with what the instruction does write.
    {"ldur  x0, [x1, #8]",   0xF8408020, reg(a64::x1), false, nullptr,      nullptr},
    {"ldur  x0, [x1, #8]",   0xF8408020, reg(a64::x0), true,  "deref:64",   nullptr},
    {"ldurb w0, [x1, #8]",   0x38408020, reg(a64::x1), false, nullptr,      nullptr},
    {"ldurb w0, [x1, #8]",   0x38408020, reg(a64::w0), true,  "deref:8",    nullptr},
    {"stur  x0, [x1, #8]",   0xF8008020, reg(a64::x1), false, nullptr,      nullptr},
    {"stur  x0, [x1, #8]",   0xF8008020, memory(),     true,  "<V([x0]",    nullptr},
    {"sturb w0, [x1, #8]",   0x38008020, reg(a64::x1), false, nullptr,      nullptr},
    {"sturb w0, [x1, #8]",   0x38008020, memory(),     true,  "<V([w0]",    nullptr},
    {"ldtr  x0, [x1, #8]",   0xF8408820, reg(a64::x1), false, nullptr,      nullptr},
    {"ldtr  x0, [x1, #8]",   0xF8408820, reg(a64::x0), true,  "deref:64",   nullptr},
    {"sttr  x0, [x1, #8]",   0xF8008820, reg(a64::x1), false, nullptr,      nullptr},
    {"sttr  x0, [x1, #8]",   0xF8008820, memory(),     true,  "<V([x0]",    nullptr},
    {"ldr   x0, [x1], #8",   0xF8408420, reg(a64::x1), true,  "<add>",      nullptr},
    {"ldr   x0, [x1, #8]!",  0xF8408C20, reg(a64::x1), true,  "<add>",      nullptr},

    // Loads write their destination and stores do not, whatever bits 23:22 hold.
    {"ldrsb w0, [x1]",       0x39C00020, reg(a64::w0), true,  "deref:8",    nullptr},
    {"ldrsh w0, [x1]",       0x79C00020, reg(a64::w0), true,  "deref:16",   nullptr},
    {"ldar  w0, [x1]",       0x88DFFC20, reg(a64::w0), true,  "deref:32",   nullptr},
    // LDAR Xt's width fix is not visible here: SymEval prints the same 64-bit load
    // before and after it. This row checks the direction only.
    {"ldar  x0, [x1]",       0xC8DFFC20, reg(a64::x0), true,  "deref:64",   nullptr},
    {"stlr  x0, [x1]",       0xC89FFC20, reg(a64::x0), false, nullptr,      nullptr},
    {"stlr  x0, [x1]",       0xC89FFC20, memory(),     true,  "<V([x0]",    nullptr},

    // Literal loads: bits 23:22 are offset bits. LDR never sign-extends, also for a
    // backward literal (offset bit 23 set); LDRSW always does.
    {"ldr   w0, pc+8",       0x18000040, reg(a64::w0), true,  "deref:32",   nullptr},
    {"ldr   x0, pc+24",      0x580000C0, reg(a64::x0), true,  "deref:64",   "signExtend"},
    {"ldr   x0, pc-4",       0x58FFFFE0, reg(a64::x0), true,  "deref:64",   "signExtend"},
    {"ldrsw x0, pc+8",       0x98000040, reg(a64::x0), true,  "signExtend", nullptr},
  };
  // clang-format on

  return rose_semantics_aarch64::check(cases) ? EXIT_SUCCESS : EXIT_FAILURE;
}
