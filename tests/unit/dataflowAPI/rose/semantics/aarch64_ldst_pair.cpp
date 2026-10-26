// Load/store pairs.
#include "aarch64_semantics.h"

#include <cstdlib>

int main() {
  using rose_semantics_aarch64::memory;
  using rose_semantics_aarch64::reg;
  using rose_semantics_aarch64::test_case;
  namespace a64 = Dyninst::aarch64;

  // clang-format off
  test_case const cases[] = {
    // 64-bit pairs: both registers, the second 8 bytes after the first.
    {"ldp   x29, x30, [sp], #16",   0xA8C17BFD, reg(a64::x29), true,  "deref:64",   nullptr},
    {"ldp   x29, x30, [sp], #16",   0xA8C17BFD, reg(a64::x30), true,  "<8:",        nullptr},
    // 32-bit pairs: 32-bit reads, the second 4 bytes after the first.
    {"ldp   w0, w1, [x2]",          0x29400440, reg(a64::w1),  true,  "deref:32",   nullptr},
    {"ldp   w0, w1, [x2]",          0x29400440, reg(a64::w1),  true,  "<4:",        nullptr},
    // LDPSW reads 32 bits and sign-extends.
    {"ldpsw x0, x1, [x2]",          0x69400440, reg(a64::x1),  true,  "deref:32",   nullptr},
    {"ldpsw x0, x1, [x2]",          0x69400440, reg(a64::x1),  true,  "signExtend", nullptr},

    // Writeback: post-index (bits 24:23 = 01) and pre-index (11) write the base register,
    // signed offset (10) and no-allocate (00) do not. Each "not written" row is paired
    // with what the instruction does write.
    {"ldp   x29, x30, [sp], #16",   0xA8C17BFD, reg(a64::sp),  true,  "<16:",       nullptr},
    {"ldp   x29, x30, [sp, #16]!",  0xA9C17BFD, reg(a64::sp),  true,  "<add>",      nullptr},
    {"ldp   x29, x30, [sp, #16]!",  0xA9C17BFD, reg(a64::sp),  true,  "<16:64>",    nullptr},
    {"ldp   x29, x30, [sp, #16]!",  0xA9C17BFD, reg(a64::x29), true,  "deref:64",   nullptr},
    {"stp   x29, x30, [sp], #16",   0xA8817BFD, reg(a64::sp),  true,  "<16:",       nullptr},
    {"stp   x29, x30, [sp, #-16]!", 0xA9BF7BFD, reg(a64::sp),  true,  "<add>",      nullptr},
    // -16 as an unsigned 64-bit constant.
    {"stp   x29, x30, [sp, #-16]!", 0xA9BF7BFD, reg(a64::sp),  true,  "<18446744073709551600:64>", nullptr},
    {"stp   x29, x30, [sp, #-16]!", 0xA9BF7BFD, reg(a64::x29), false, nullptr,      nullptr},
    {"stp   x29, x30, [sp, #-16]!", 0xA9BF7BFD, memory(),      true,  "<V([x",      nullptr},
    {"ldp   x0, x1, [x2, #16]",     0xA9410440, reg(a64::x2),  false, nullptr,      nullptr},
    {"ldp   x0, x1, [x2, #16]",     0xA9410440, reg(a64::x0),  true,  "deref:64",   nullptr},
    {"ldnp  x0, x1, [x2, #16]",     0xA8410440, reg(a64::x2),  false, nullptr,      nullptr},
    {"ldnp  x0, x1, [x2, #16]",     0xA8410440, reg(a64::x0),  true,  "deref:64",   nullptr},
    // A store's address is not part of the value SymEval gives for memory, so STNP's
    // address fix (the memory operand, not the second register) is not checked here.
    {"stnp  x0, x1, [x2, #16]",     0xA8010440, reg(a64::x2),  false, nullptr,      nullptr},
    {"stnp  x0, x1, [x2, #16]",     0xA8010440, memory(),      true,  "<V([x",      nullptr},
  };
  // clang-format on

  return rose_semantics_aarch64::check(cases) ? EXIT_SUCCESS : EXIT_FAILURE;
}
