#include "opcode_tests.h"

#include <iostream>

namespace Dyninst { namespace InstructionAPI {

  bool verify(Instruction const &insn, opcode_test const &expected) {

    Operation actual_opcode = insn.getOperation();
    Operation actual_encoded_opcode = insn.getEncodedOperation();
    Operation expected_opcode = expected.opcode;
    Operation expected_encoded_opcode = expected.encoded_opcode;

    bool failed = false;

    if(actual_opcode.getID() != expected_opcode.getID()) {
      std::cerr << "Mismatched opcode\n";
      std::cerr << "Expected: " << expected_opcode.format() << ", Found: " << actual_opcode.format();
      std::cerr << '\n';
      failed = true;
    }

    if(actual_encoded_opcode.getID() != expected_encoded_opcode.getID()) {
      std::cerr << "Mismatched encoded opcode\n";
      std::cerr << "Expected: " << expected_encoded_opcode.format() << ", Found: " << actual_encoded_opcode.format();
      std::cerr << '\n';
      failed = true;
    }

    return !failed;
  }
}}
