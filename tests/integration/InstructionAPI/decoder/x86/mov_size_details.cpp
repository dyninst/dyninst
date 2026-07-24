#include "Architecture.h"
#include "Instruction.h"
#include "InstructionDecoder.h"

#include <array>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <tuple>
#include <vector>

/*
 *  x86 operand size tests
 *
 *  These verify the sizes of the value expressions computed for both
 *  operands of a few instructions with interesting operand sizes.
 */

namespace di = Dyninst::InstructionAPI;

int main() {
  constexpr auto num_tests = 5;

  // clang-format off
  constexpr std::array<const uint8_t, 16> buffer = {{
    0x66, 0x8c, 0xe8,               // mov ax, gs
    0x89, 0xe8,                     // mov eax, ebp
    0xf2, 0x0f, 0x12, 0xc0,         // movddup xmm0, xmm0
    0x05, 0xef, 0xbe, 0xad, 0xde,   // add eax, 0xdeadbeef
    0xd8, 0xd8,                     // fcomp st0, st0
  }};
  // clang-format on

  std::vector<di::Instruction> decodedInsns;
  decodedInsns.reserve(num_tests);

  di::InstructionDecoder d(buffer.data(), buffer.size(), Dyninst::Arch_x86);
  for(int idx = 0; idx < num_tests; idx++) {
    di::Instruction insn = d.decode();
    if(!insn.isValid()) {
      std::cerr << "Failed to decode x86 test " << (idx + 1) << '\n';
      return EXIT_FAILURE;
    }
    decodedInsns.push_back(insn);
  }

  std::vector<std::tuple<int, int>> expected_sizes;

  // mov ax, gs
  expected_sizes.push_back(std::make_tuple(2, 2));

  // mov eax, ebp
  expected_sizes.push_back(std::make_tuple(4, 4));

  // movddup xmm0, xmm0
  expected_sizes.push_back(std::make_tuple(16, 16));

  // add eax, 0xdeadbeef
  expected_sizes.push_back(std::make_tuple(4, 4));

  // fcomp st0, st0
  expected_sizes.push_back(std::make_tuple(8, 8)); // wrong. should be 80 bits

  if(decodedInsns.size() != expected_sizes.size()) {
    std::cerr << "FATAL: decodedInsns.size() != expected_sizes.size()\n";
    return EXIT_FAILURE;
  }

  bool failed = false;
  for(size_t i = 0; i < decodedInsns.size(); i++) {
    auto const &insn = decodedInsns[i];

    std::clog << "Verifying '" << insn.format() << "'\n";

    const auto lhs_size = std::get<0>(expected_sizes[i]);
    const auto rhs_size = std::get<1>(expected_sizes[i]);

    di::Expression::Ptr lhs = insn.getOperand(0).getValue();
    di::Expression::Ptr rhs = insn.getOperand(1).getValue();

    if(static_cast<int>(lhs->size()) != lhs_size) {
      std::cerr << "LHS expected " << (lhs_size * 8) << "-bit, actual " << (lhs->size() * 8) << "-bit ("
                << lhs->format() << ")\n";
      failed = true;
    }
    if(static_cast<int>(rhs->size()) != rhs_size) {
      std::cerr << "RHS expected " << (rhs_size * 8) << "-bit, actual " << (rhs->size() * 8) << "-bit ("
                << rhs->format() << ")\n";
      failed = true;
    }
  }

  return failed ? EXIT_FAILURE : EXIT_SUCCESS;
}
