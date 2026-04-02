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
  auto nzcv = Dyninst::aarch64::nzcv;

  auto w1 = Dyninst::aarch64::w1;
  auto w5 = Dyninst::aarch64::w5;
  auto w7 = Dyninst::aarch64::w7;
  auto w10 = Dyninst::aarch64::w10;
  auto w15 = Dyninst::aarch64::w15;

  auto x0 = Dyninst::aarch64::x0;
  auto x2 = Dyninst::aarch64::x2;
  auto x4 = Dyninst::aarch64::x4;
  auto x20 = Dyninst::aarch64::x20;
  auto x21 = Dyninst::aarch64::x21;
  auto x22 = Dyninst::aarch64::x22;


  using reg_set = Dyninst::register_set;

  return {
    { // ccmn w5, w10, #7, ne
      {0xa7, 0x10, 0x4a, 0x3a},
      di::opcode_test(aarch64_op_ccmn_reg, "ccmn w5, w10, #7, ne"),
      di::register_rw_test {
        reg_set{w10, w5, nzcv},
        reg_set{nzcv},
      },
    },
    { // ccmn w7, #0x1e, #0xb, pl
      {0xeb, 0x58, 0x5e, 0x3a},
      di::opcode_test(aarch64_op_ccmn_imm, "ccmn w7, #1, #0, pl"), // WRONG: should be 'ccmn w7, #0x1e, #0xb, pl'
      di::register_rw_test {
        reg_set{w7, nzcv},
        reg_set{nzcv},
      },
    },
    { // ccmp x4, x2, #4, ge
      {0x84, 0xa0, 0x42, 0xfa},
      di::opcode_test(aarch64_op_ccmp_reg, "ccmp x4, x2, #4, ge"),
      di::register_rw_test {
        reg_set{x2, x4, nzcv},
        reg_set{nzcv},
      },
    },
    { // ccmp x20, #8, #8, nv
      {0x88, 0xfa, 0x48, 0xfa},
      di::opcode_test(aarch64_op_ccmp_imm, "ccmp x20, #8, #8, nv"),
      di::register_rw_test {
        reg_set{x20, nzcv},
        reg_set{nzcv},
      },
    },

    /**** Conditional select ***/

    { // csel w5, w10, w15, ne
      {0x45, 0x11, 0x8f, 0x1a},
      di::opcode_test(aarch64_op_csel, "csel w5, w10, w15, ne"),
      di::register_rw_test {
        reg_set{w15, w10, nzcv},
        reg_set{w5},
      },
    },
    { // csinc x0, x2, x4, pl
      {0x40, 0x54, 0x84, 0x9a},
      di::opcode_test(aarch64_op_csinc, "csinc x0, x2, x4, pl"),
      di::register_rw_test {
        reg_set{x2, x4, nzcv},
        reg_set{x0},
      },
    },
    { // csinv x20, x21, x22, vc
      {0xb4, 0x72, 0x96, 0xda},
      di::opcode_test(aarch64_op_csinv, "csinv x20, x21, x22, vc"),
      di::register_rw_test {
        reg_set{x21, x22, nzcv},
        reg_set{x20},
      },
    },
    { // csneg w1, w5, w10, ge
      {0xa1, 0xa4, 0x8a, 0x5a},
      di::opcode_test(aarch64_op_csneg, "csneg w1, w5, w10, ge"),
      di::register_rw_test {
        reg_set{w5, w10, nzcv},
        reg_set{w1},
      },
    },


  };
}
