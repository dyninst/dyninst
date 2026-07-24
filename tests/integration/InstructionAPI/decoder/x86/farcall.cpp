#include "Architecture.h"
#include "Instruction.h"
#include "InstructionDecoder.h"

#include <array>
#include <cstdint>
#include <cstdlib>
#include <iostream>

/*
 *  x86 far call decoding test
 *
 *  Verifies that the decoder can decode a far call with an immediate
 *  segment:offset operand. This form is only valid on 32-bit x86.
 */

namespace di = Dyninst::InstructionAPI;

int main() {
  constexpr auto num_tests = 1;

  std::array<const uint8_t, 9> buffer = {{
      0x9A, 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0xFF, 0xFE // CALL 0504030201, with FF/FE as fenceposts
  }};

  di::InstructionDecoder d(buffer.data(), buffer.size(), Dyninst::Arch_x86);

  for(int idx = 0; idx < num_tests; idx++) {
    di::Instruction insn = d.decode();
    if(!insn.isValid()) {
      std::cerr << "Failed to decode test " << (idx + 1) << '\n';
      return EXIT_FAILURE;
    }
    std::clog << "Decoded '" << insn.format() << "'\n";
  }
  return EXIT_SUCCESS;
}
