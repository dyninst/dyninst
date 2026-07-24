#include "Architecture.h"
#include "Instruction.h"
#include "InstructionDecoder.h"
#include "Register.h"
#include "registers/aarch64_regs.h"

#include <array>
#include <boost/make_shared.hpp>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <vector>

/*
 *  aarch64 control-flow target (CFT) tests
 *
 *  These verify the control-flow targets produced by the decoder for
 *  branch instructions. Each target expression is evaluated by binding
 *  the PC and the target register to known values, and both the
 *  resulting address and the CFT properties (call, conditional,
 *  indirect, fallthrough) are checked. A conditional branch has several
 *  targets with different properties (e.g., the taken target is not a
 *  fallthrough while the fallthrough target is), so each target is
 *  checked individually against its own expected properties.
 */

namespace di = Dyninst::InstructionAPI;

namespace {

// The instruction bytes below are written in big-endian order for
// readability; the decoder expects little-endian words.
void reverseBuffer(unsigned char *buffer, int bufferSize) {
  int elementCount = bufferSize / 4;

  for(int loop_index = 0; loop_index < elementCount; loop_index++) {
    std::swap(buffer[0], buffer[3]);
    std::swap(buffer[1], buffer[2]);
    buffer += 4;
  }
}

struct cft_expected {
  bool defined;
  unsigned long long int expected;
  bool is_call;
  bool is_conditional;
  bool is_indirect;
  bool is_fallthrough;
};

struct test_insn {
  std::array<uint8_t, 4> bytes;
  std::vector<cft_expected> expected_targets;
};

bool verifyTargetType(di::Instruction::CFT const &actual, cft_expected const &expected) {
  bool failed = false;
  if(actual.isCall != expected.is_call) {
    std::cerr << std::boolalpha << "Expected isCall = " << expected.is_call << ", got '" << actual.isCall << "'\n";
    failed = true;
  }
  if(actual.isIndirect != expected.is_indirect) {
    std::cerr << std::boolalpha << "Expected isIndirect = " << expected.is_indirect << ", got '" << actual.isIndirect
              << "'\n";
    failed = true;
  }
  if(actual.isConditional != expected.is_conditional) {
    std::cerr << std::boolalpha << "Expected isConditional = " << expected.is_conditional << ", got '"
              << actual.isConditional << "'\n";
    failed = true;
  }
  if(actual.isFallthrough != expected.is_fallthrough) {
    std::cerr << std::boolalpha << "Expected isFallthrough = " << expected.is_fallthrough << ", got '"
              << actual.isFallthrough << "'\n";
    failed = true;
  }
  return !failed;
}

bool verifyTargetValue(di::Expression::Ptr target, cft_expected const &expected) {
  auto result = target->eval();
  if(result.defined != expected.defined) {
    std::cerr << std::boolalpha << "Target '" << target->format(Dyninst::Arch_aarch64) << "': expected defined = "
              << expected.defined << ", got '" << result.defined << "'\n";
    return false;
  }
  if(!expected.defined) {
    return true;
  }
  if(result.type != di::u64) {
    std::cerr << "Expected result type " << di::u64 << ", got '" << result.type << "'\n";
    return false;
  }
  if(result.convert<unsigned long long int>() != expected.expected) {
    std::cerr << std::hex << "Expected target value 0x" << expected.expected << ", got '0x"
              << result.convert<unsigned long long int>() << "'\n"
              << std::dec;
    return false;
  }
  return true;
}

std::vector<test_insn> make_tests() {
  constexpr auto is_call = true;
  constexpr auto is_conditional = true;
  constexpr auto is_indirect = true;
  constexpr auto is_fallthrough = true;
  constexpr auto is_defined = true;

  // clang-format off
  return {
    {
      // B #-1
      {{0x17, 0xFF, 0xFF, 0xFF}},
      {
        {is_defined, 0x3FC, !is_call, !is_conditional, !is_indirect, !is_fallthrough}
      }
    },
    {
      // BR X12
      {{0xD6, 0x1F, 0x01, 0x80}},
      {
        {is_defined, 0x90, !is_call, !is_conditional, is_indirect, !is_fallthrough},
      }
    },
    {
      // RET X12
      {{0xD6, 0x5F, 0x01, 0x80}},
      {
        {is_defined, 0x90, !is_call, !is_conditional, is_indirect, !is_fallthrough},
      }
    },
    {
      // CBZ W15, #-1
      {{0x34, 0xFF, 0xFF, 0xEF}},
      {
        {is_defined, 0x3FC, !is_call, is_conditional, !is_indirect, !is_fallthrough},
        {is_defined, 0x404, !is_call, is_conditional, !is_indirect, is_fallthrough},
      }
    },
    {
      // B.NE #-1
      {{0x54, 0xFF, 0xFF, 0xE1}},
      {
        {is_defined, 0x3FC, !is_call, is_conditional, !is_indirect, !is_fallthrough},
        {is_defined, 0x404, !is_call, is_conditional, !is_indirect, is_fallthrough},
      }
    },
    {
      // TBZ W4, #30, #-1
      {{0x36, 0xF7, 0xFF, 0xE4}},
      {
        {is_defined, 0x3FC, !is_call, is_conditional, !is_indirect, !is_fallthrough},
        {is_defined, 0x404, !is_call, is_conditional, !is_indirect, is_fallthrough},
      }
    },
    {
      // TBNZ X25, #0, #16
      {{0xB7, 0x80, 0x02, 0x19}},
      {
        {is_defined, 0x440, !is_call, is_conditional, !is_indirect, !is_fallthrough},
        {is_defined, 0x404, !is_call, is_conditional, !is_indirect, is_fallthrough},
      }
    },
    {
      // BL PC + 0x14
      {{0x94, 0x00, 0x00, 0x05}},
      {
        {is_defined, 0x414, is_call, !is_conditional, !is_indirect, !is_fallthrough},
        {is_defined, 0x404, !is_call, !is_conditional, !is_indirect, is_fallthrough},
      }
    },
    {
      // BLR X12
      {{0xD6, 0x3F, 0x01, 0x80}},
      {
        {is_defined, 0x90, is_call, !is_conditional, is_indirect, !is_fallthrough},
        {is_defined, 0x404, !is_call, !is_conditional, !is_indirect, is_fallthrough},
      }
    }
  };
  // clang-format on
}

} // namespace

int main() {
  bool failed = false;
  int test_id = 0;

  di::Expression::Ptr theIP = boost::make_shared<di::RegisterAST>(Dyninst::aarch64::pc);
  di::Expression::Ptr x_reg = boost::make_shared<di::RegisterAST>(Dyninst::aarch64::x12);

  for(auto &t : make_tests()) {
    test_id++;

    reverseBuffer(t.bytes.data(), t.bytes.size());
    di::InstructionDecoder d(t.bytes.data(), t.bytes.size(), Dyninst::Arch_aarch64);

    auto insn = d.decode();
    if(!insn.isValid()) {
      std::cerr << "Failed to decode test " << test_id << '\n';
      failed = true;
      continue;
    }

    std::clog << "Verifying '" << insn.format() << "'\n";

    const size_t num_cft = std::distance(insn.cft_begin(), insn.cft_end());
    const auto num_expected_cft = t.expected_targets.size();

    if(num_cft != num_expected_cft) {
      std::cerr << "Number of targets mismatched for test " << test_id << " '" << insn.format() << "'. Found "
                << num_cft << ", expected " << num_expected_cft << '\n';
      failed = true;
      continue;
    }

    size_t target_id = 0;
    for(auto cft_iter = insn.cft_begin(); cft_iter != insn.cft_end(); ++cft_iter, ++target_id) {
      di::Instruction::CFT const &cft_cur = *cft_iter;
      auto target = cft_cur.target;

      if(!target) {
        std::cerr << "No target for '" << insn.format() << "'\n";
        failed = true;
        continue;
      }

      target->bind(theIP.get(), di::Result(di::u64, 0x400));
      target->bind(x_reg.get(), di::Result(di::u64, 0x90));

      cft_expected const &expected = t.expected_targets[target_id];

      if(!verifyTargetValue(target, expected)) {
        failed = true;
      }
      if(!verifyTargetType(cft_cur, expected)) {
        failed = true;
      }
    }

    std::clog << '\n';
  }

  return failed ? EXIT_FAILURE : EXIT_SUCCESS;
}
