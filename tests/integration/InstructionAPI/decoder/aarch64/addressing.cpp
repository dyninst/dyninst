#include "Architecture.h"
#include "cft_tests.h"
#include "InstructionDecoder.h"
#include "memory_tests.h"
#include "opcode_tests.h"
#include "Register.h"
#include "register_tests.h"
#include "registers/MachRegister.h"
#include "registers/register_set.h"
#include "registers/aarch64_regs.h"
#include "entryIDs.h"

#include <cstdlib>
#include <iostream>
#include <vector>

namespace di = Dyninst::InstructionAPI;

struct math_test {
  std::vector<unsigned char> bytes;
  di::opcode_test opcode;
  di::register_rw_test regs;
};

static std::vector<math_test> make_tests();

int main() {
  bool failed = false;
  int test_id = 0;
  const auto arch = Dyninst::Arch_aarch64;
  for(auto const &t : make_tests()) {
    test_id++;
    di::InstructionDecoder d(t.bytes.data(), t.bytes.size(), arch);
    auto insn = d.decode();
    if(!insn.isValid()) {
      std::cerr << "Failed to decode test " << test_id << '\n';
      failed = true;
      continue;
    }

    std::clog << "Verifying '" << insn.format() << "'\n";

    if(!di::verify(insn, t.opcode)) {
      failed = true;
    }
    if(!di::verify(insn, t.regs)) {
      failed = true;
    }

    std::clog << "\n";
  }
  return failed ? EXIT_FAILURE : EXIT_SUCCESS;
}

std::vector<math_test> make_tests() {
  auto pc = Dyninst::aarch64::pc;
  auto x0 = Dyninst::aarch64::x0;

  auto x30 = Dyninst::aarch64::x30;

  using reg_set = Dyninst::register_set;

  return {
    { // adr x0, 0xfffffffffff00000
      {0x00, 0x00, 0x80, 0x10},
      di::opcode_test(aarch64_op_adr, "adr x0, pc + #0"),  // WRONG: should be 'adr x0, 0xfffffffffff00000'
      di::register_rw_test {
        reg_set{pc},
        reg_set{x0},
      },
    },
    { // adrp x30, #7000
      {0x3e, 0x00, 0x00, 0xf0},
      di::opcode_test(aarch64_op_adrp, "adrp x30, pc + #7000"), // WRONG: should be 'adrp x30, #7000"'
      di::register_rw_test {
        reg_set{pc},
        reg_set{x30},
      },
    },

  };
}
