// Branch-register linking and rotated register operands.
#include "aarch64_semantics.h"

#include <cstdlib>

int main() {
  using rose_semantics_aarch64::exact_case;
  using rose_semantics_aarch64::reg;
  namespace a64 = Dyninst::aarch64;

  // Each row gives the whole value SymEval computes for one location, as AST::format()
  // prints it; nullptr means the instruction does not write it. V([r]:1000) is the value
  // of r before the instruction, which the test places at address 0x1000.
  //
  // The shift and rotate amounts and the signExtend widths below print with a size of
  // 2^64-1 (<8:18446744073709551615>): that is the width those operands are read with, the
  // same for LSL, LSR and ASR, and not something these instructions do.

  // clang-format off
  exact_case const cases[] = {
    // BR branches to x17 without linking.
    {"br    x17", 0xD61F0220, reg(a64::x30), nullptr},
    {"br    x17", 0xD61F0220, reg(a64::pc),
     "<extract:64>(<V([x17]:1000)>,<0:64>,<64:64>,)"},

    // BLR and BL link to the next instruction (PC + 4); BLR branches to x8, BL to PC + 8.
    {"blr   x8", 0xD63F0100, reg(a64::x30),
     "<add>(<V([pc]:1000)>,<4:32>,)"},
    {"blr   x8", 0xD63F0100, reg(a64::pc),
     "<extract:64>(<V([x8]:1000)>,<0:64>,<64:64>,)"},
    {"bl    pc+8", 0x94000002, reg(a64::x30),
     "<add>(<V([pc]:1000)>,<4:32>,)"},
    {"bl    pc+8", 0x94000002, reg(a64::pc),
     "<extract:64>(<add>(<signExtend>(<V([pc]:1000)>,<18446744073709551615:64>,),"
     "<signExtend>(<8:18446744073709551615>,<18446744073709551615:64>,),),<0:64>,<64:64>,)"},

    // A ROR-shifted register operand is read, not rejected, for X and W operands:
    // Rd = Rn XOR (Rm ROR #8), and the PC moves on to the next instruction (0x1000 + 4).
    {"eor   x0, x1, x2, ror #8", 0xCAC22020, reg(a64::x0),
     "<xor>(<V([x1]:1000)>,<rotR>(<V([x2]:1000)>,<8:18446744073709551615>,),)"},
    {"eor   w0, w1, w2, ror #8", 0x4AC22020, reg(a64::w0),
     "<xor>(<V([w1]:1000)>,<rotR>(<V([w2]:1000)>,<8:18446744073709551615>,),)"},
    {"eor   w0, w1, w2, ror #8", 0x4AC22020, reg(a64::pc),
     "<add>(<4096:64>,<4:64>,)"},
  };
  // clang-format on

  return rose_semantics_aarch64::check_exact(cases) ? EXIT_SUCCESS : EXIT_FAILURE;
}
