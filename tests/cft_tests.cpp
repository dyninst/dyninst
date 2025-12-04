#include "cft_tests.h"

#include "Architecture.h"
#include "InstructionDecoder.h"

#include <algorithm>
#include <boost/range/iterator_range.hpp>
#include <iostream>

namespace di = Dyninst::InstructionAPI;

namespace Dyninst { namespace InstructionAPI {

  bool verify(Instruction const &insn, cft_test const &test) {
    bool failed = false;

    const auto num_cft = std::distance(insn.cft_begin(), insn.cft_end());
    if(!num_cft && test.hasCFT) {
      std::cerr << "No control flow targets found, but at least one expected.\n";
      return false;
    }

    auto all_cfts = boost::make_iterator_range(insn.cft_begin(), insn.cft_end());

    if(num_cft && !test.hasCFT) {
      std::cerr << "Control flow targets found, but none expected.\n";
      for(auto const &actual : all_cfts) {
        std::cerr << "    " << actual.target->format(insn.getArch()) << "\n";
      }
      return false;
    }

    auto const &expected = test.expected;

    // Check the instruction-level properties
    if(insn.isCall() != expected.isCall) {
      std::cerr << std::boolalpha << "Expected isCall() = " << expected.isCall << ", got '" << insn.isCall() << "'\n";
      failed = true;
    }
    if(insn.isReturn() != expected.isReturn) {
      std::cerr << std::boolalpha << "Expected isReturn() = " << expected.isReturn << ", got '" << insn.isReturn()
                << "'\n";
      failed = true;
    }
    if(insn.isBranch() != expected.isBranch) {
      std::cerr << std::boolalpha << "Expected isBranch() = " << expected.isBranch << ", got '" << insn.isBranch()
                << "'\n";
      failed = true;
    }

    // Check the CFT-level properties
    for(auto const &actual : all_cfts) {
      if(actual.isCall != expected.isCall) {
        std::cerr << std::boolalpha << "Expected isCall = " << expected.isCall << ", got '" << actual.isCall << "'\n";
        failed = true;
      }
      if(actual.isIndirect != expected.isIndirect) {
        std::cerr << std::boolalpha << "Expected isIndirect = " << expected.isIndirect << ", got '" << actual.isIndirect
                  << "'\n";
        failed = true;
      }
      if(actual.isConditional != expected.isConditional) {
        std::cerr << std::boolalpha << "Expected isConditional = " << expected.isConditional << ", got '"
                  << actual.isConditional << "'\n";
        failed = true;
      }
      if(actual.isFallthrough != expected.isFallthrough) {
        std::cerr << std::boolalpha << "Expected isFallthrough = " << expected.isFallthrough << ", got '"
                  << actual.isFallthrough << "'\n";
        failed = true;
      }
    }
    return !failed;
  }

}}
