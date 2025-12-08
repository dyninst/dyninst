#include "opcode_tests.h"

#include <iostream>

namespace Dyninst {
namespace InstructionAPI {

bool verify(Instruction const &insn, opcode_test const &expected) {

  entryID actual_opcode = insn.getOperation().getID();
  entryID actual_encoded_opcode = insn.getEncodedOperation().getID();
  entryID expected_opcode = expected.opcode;
  entryID expected_encoded_opcode = expected.encoded_opcode;

  bool failed = false;

  if (actual_opcode != expected_opcode) {
    std::cerr << "Mismatched opcode\n";
    std::cerr << "Expected: " << expected.opcode_mnemonic
              << ", Found: " << insn.getOperation().format();
    std::cerr << '\n';
    failed = true;
  }

  if (actual_encoded_opcode != expected_encoded_opcode) {
    std::cerr << "Mismatched encoded opcode\n";
    std::cerr << "Expected: " << expected.encoded_opcode_mnemonic
              << ", Found: " << insn.getEncodedOperation().format();
    std::cerr << '\n';
    failed = true;
  }

  return !failed;
}
} // namespace InstructionAPI
} // namespace Dyninst
