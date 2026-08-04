#include "Architecture.h"
#include "Instruction.h"
#include "InstructionDecoder.h"
#include "Register.h"
#include "registers/ppc64_regs.h"

#include <boost/make_shared.hpp>
#include <boost/optional.hpp>
#include <boost/range/iterator_range.hpp>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <vector>

/*
 *  PowerPC control-flow target (CFT) tests
 *
 *  These verify the control-flow targets produced by the decoder for the
 *  branch instruction families (I-form, B-form, and XL-form). Each target
 *  expression is evaluated by binding the PC, count, and link registers
 *  (or the target itself for absolute branches) to known values, and both
 *  the resulting address and the CFT properties (call, conditional,
 *  indirect, fallthrough) are checked. A conditional branch has several
 *  targets with different properties (e.g., the taken target is
 *  conditional while the fallthrough target is not), so this test checks
 *  each target individually.
 */

namespace di = Dyninst::InstructionAPI;

namespace {

struct cft_expected {
  bool is_call;
  bool is_conditional;
  bool is_indirect;
  bool is_fallthrough;
};

struct test_cft {
  uint32_t value;
  cft_expected cft;
};

struct test_insn {
  uint32_t opcode;
  bool is_branch;
  bool is_return;
  boost::optional<test_cft> pc;   // program counter
  boost::optional<test_cft> ctr;  // count register
  boost::optional<test_cft> lr;   // link register
  boost::optional<test_cft> abs;  // absolute value (AA=1 instructions)
  boost::optional<test_cft> ft;   // fallthrough target
};

constexpr auto pc_value = uint32_t{0x400};
constexpr auto ctr_value = uint32_t{44};
constexpr auto lr_value = uint32_t{0x200};

constexpr auto result_val_t = di::u64;

std::vector<test_insn> make_tests();

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

bool verifyTargetValue(di::Expression::Ptr target, uint32_t expected_value) {
  auto result = target->eval();
  if(!result.defined) {
    std::cerr << "Target '" << target->format(Dyninst::Arch_ppc64) << "' did not evaluate to a defined value\n";
    return false;
  }
  if(result.type != result_val_t) {
    std::cerr << "Expected result type " << result_val_t << ", got '" << result.type << "'\n";
    return false;
  }
  if(result.convert<uint64_t>() != expected_value) {
    std::cerr << std::hex << "Expected target value 0x" << expected_value << ", got '0x" << result.convert<uint64_t>()
              << "'\n"
              << std::dec;
    return false;
  }
  return true;
}

bool check_cft(di::Instruction::CFT const &cur_cft, boost::optional<test_cft> const &test_reg) {
  if(!test_reg) {
    std::cerr << "Test does not use register, but should\n";
    return false;
  }
  if(!verifyTargetValue(cur_cft.target, test_reg->value)) {
    return false;
  }
  return verifyTargetType(cur_cft, test_reg->cft);
}

} // namespace

int main() {
  bool failed = false;
  int test_id = 0;

  auto pc = boost::make_shared<di::RegisterAST>(Dyninst::ppc64::pc);
  auto ctr = boost::make_shared<di::RegisterAST>(Dyninst::ppc64::ctr);
  auto lr = boost::make_shared<di::RegisterAST>(Dyninst::ppc64::lr);

  auto bind_val = [](di::Instruction::CFT const &cur_cft, di::Expression::Ptr target, uint32_t value) {
    cur_cft.target->bind(target.get(), di::Result(result_val_t, value));
  };

  for(auto const &t : make_tests()) {
    test_id++;

    di::InstructionDecoder d(&t.opcode, sizeof(t.opcode), Dyninst::Arch_ppc64);
    auto insn = d.decode();

    if(!insn.isValid()) {
      std::cerr << "Failed to decode test " << test_id << '\n';
      failed = true;
      continue;
    }

    std::clog << "Verifying '" << insn.format() << "'\n";

    auto all_cfts = boost::make_iterator_range(insn.cft_begin(), insn.cft_end());
    const auto num_cft = std::distance(insn.cft_begin(), insn.cft_end());

    if(!num_cft) {
      std::cerr << "No control flow targets found\n";
      failed = true;
      continue;
    }

    int const num_targets_expected = [&t]() {
      int n = 0;
      if(t.pc) n++;
      if(t.ctr) n++;
      if(t.lr) n++;
      if(t.abs) n++;
      if(t.ft) n++;
      return n;
    }();

    int num_targets_seen = 0;
    bool abs_was_used = false;
    for(auto const &cur_cft : all_cfts) {
      std::clog << "Target " << (num_targets_seen + 1) << ": '" << cur_cft.target->format(Dyninst::Arch_ppc64)
                << "'\n";

      if(t.abs && !abs_was_used) {
        std::clog << "Checking absolute address\n";
        num_targets_seen++;
        bind_val(cur_cft, cur_cft.target, t.abs->value);
        if(!check_cft(cur_cft, t.abs)) {
          failed = true;
        }
        abs_was_used = true;
      }
      if(cur_cft.target->isUsed(pc)) {
        std::clog << "Checking PC\n";
        num_targets_seen++;
        bind_val(cur_cft, pc, pc_value);
        if(!check_cft(cur_cft, t.pc)) {
          std::clog << "  Trying fallthrough...\n";
          if(!t.ft || !check_cft(cur_cft, t.ft)) {
            failed = true;
          }
        }
      }
      if(cur_cft.target->isUsed(ctr)) {
        std::clog << "Checking count register\n";
        num_targets_seen++;
        bind_val(cur_cft, ctr, ctr_value);
        if(!check_cft(cur_cft, t.ctr)) {
          failed = true;
        }
      }
      if(cur_cft.target->isUsed(lr)) {
        std::clog << "Checking link register\n";
        num_targets_seen++;
        bind_val(cur_cft, lr, lr_value);
        if(!check_cft(cur_cft, t.lr)) {
          failed = true;
        }
      }
    }
    if(num_targets_seen != num_targets_expected) {
      std::cerr << "Instruction has too " << ((num_targets_seen < num_targets_expected) ? "few" : "many")
                << " targets; expected " << num_targets_expected << ", found " << num_targets_seen << '\n';
      failed = true;
    }
    if(insn.isBranch() != t.is_branch) {
      std::cerr << "Instruction should " << (t.is_branch ? "" : "not ") << "be a branch\n";
      failed = true;
    }
    if(insn.isReturn() != t.is_return) {
      std::cerr << "Instruction should " << (t.is_return ? "" : "not ") << "be a return\n";
      failed = true;
    }

    std::clog << '\n';
  }

  return failed ? EXIT_FAILURE : EXIT_SUCCESS;
}

namespace {

constexpr auto is_call = true;
constexpr auto is_conditional = true;
constexpr auto is_indirect = true;
constexpr auto is_fallthrough = true;
constexpr auto is_branch = true;
constexpr auto is_return = true;

// clang-format off
std::vector<test_insn> make_tests() {
  return {
    /*
     *  --- Branch I-form ---
     */
    { //  b +16
      0x48000010, is_branch, !is_return,
      test_cft{pc_value + 16, {!is_call, !is_conditional, !is_indirect, !is_fallthrough}},
      {}, {}, {}, {}
    },
    { //  ba -48
      0x4bffffd2, is_branch, !is_return,
      {}, {}, {}, {}, {}
    },
    { //  bl 0x100
      0x48000101, !is_branch, !is_return,
      test_cft{pc_value + 0x100, {is_call, !is_conditional, !is_indirect, !is_fallthrough}},
      {}, {}, {}, {}
    },
    { //  bla 0x100
      0x48000103, !is_branch, !is_return,
      {}, {}, {}, {}, {}
    },

    /*
     *  --- Branch Conditional B-form ---
     */
    { //  bc +32
      0x42f00020, is_branch, !is_return,
      test_cft{pc_value + 32, {!is_call, is_conditional, !is_indirect, !is_fallthrough}},
      {},
      {},
      {},
      test_cft{pc_value + 4, {!is_call, !is_conditional, !is_indirect, is_fallthrough}}
    },
    { //  bca 32
      0x42f00022, is_branch, !is_return,
      {},
      {},
      {},
      test_cft{32, {!is_call, is_conditional, !is_indirect, !is_fallthrough}},
      test_cft{pc_value + 4, {!is_call, !is_conditional, !is_indirect, is_fallthrough}},
    },
    { //  bcl +32  (subroutine call to relative address)
      0x42f00021, !is_branch, !is_return,
      test_cft{pc_value + 32, {is_call, is_conditional, !is_indirect, !is_fallthrough}},
      {},
      {},
      {},
      test_cft{pc_value + 4, {!is_call, !is_conditional, !is_indirect, is_fallthrough}},
    },
    { //  bcla 32  (subroutine call to immediate target)
      0x42f00023, !is_branch, !is_return,
      {},
      {},
      {},
      test_cft{pc_value + 32, {is_call, is_conditional, !is_indirect, !is_fallthrough}},
      test_cft{pc_value + 4, {!is_call, !is_conditional, !is_indirect, is_fallthrough}}
    },

    /*
     *  --- Branch Conditional to Link Register XL-form ---
     *
     *  There are a large number of uses for this instruction.
     */
    { //  bclr with BH=0 (subroutine return)
      0x4e800020, !is_branch, is_return,
      {},
      {},
      test_cft{lr_value, {!is_call, !is_conditional, is_indirect, !is_fallthrough}},
      {},
      {}
    },
    { // bclrl  (LK=1)
      0x4e800021, is_branch, !is_return,
      {},
      {},
      test_cft{lr_value, {!is_call, is_conditional, is_indirect, !is_fallthrough}},
      {},
      test_cft{pc_value + 4, {!is_call, !is_conditional, !is_indirect, is_fallthrough}},
    },
    { // bdnzfl gt, 0x100
      0x40010101, !is_branch, !is_return,
      test_cft{pc_value + 0x100, {is_call, is_conditional, !is_indirect, !is_fallthrough}},
      {},
      {},
      {},
      test_cft{pc_value + 4, {!is_call, !is_conditional, !is_indirect, is_fallthrough}}
    },
    { //  bdnzf gt, 0x100   (bdnz cr0)
      0x40010100, is_branch, !is_return,
      test_cft{pc_value + 0x100, {!is_call, is_conditional, !is_indirect, !is_fallthrough}},
      {},
      {},
      {},
      test_cft{pc_value + 4, {!is_call, !is_conditional, !is_indirect, is_fallthrough}},
    },
    { //  bnslr
      0x4ca30020, !is_branch, is_return,
      {},
      {},
      test_cft{lr_value, {!is_call, !is_conditional, is_indirect, !is_fallthrough}},
      {},
      {}
    },

    /*
     *  --- Branch Conditional to Count Register XL-form ---
     */
    { //  bctr  (LK=0)
      0x4e800420, is_branch, !is_return,
      {},
      test_cft{ctr_value, {!is_call, !is_conditional, is_indirect, !is_fallthrough}},
      {}, {}, {}
    },
    { //  bctrl  (LK=1)
      0x4e800421, !is_branch, !is_return,
      {},
      test_cft{ctr_value, {is_call, !is_conditional, is_indirect, !is_fallthrough}},
      {}, {}, {}
    },
    { //  bcctr 0x17, 4*cr4+lt, 0
      0x4ef00420, is_branch, !is_return,
      {},
      test_cft{ctr_value, {!is_call, !is_conditional, is_indirect, !is_fallthrough}},
      {}, {}, {}
    },

    /*
     *  --- Branch Conditional to Branch Target Address Register XL-form ---
     *
     *  Dyninst can't decode these.
     */
//      { // bctar   (LK=0)
//        0x4e800460
//      },
//      { // bctarl  (LK=1)
//        0x4e800461
//      },
  };
  // clang-format on
}

} // namespace
