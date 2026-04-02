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
  auto w0 = Dyninst::aarch64::w0;
  auto w4 = Dyninst::aarch64::w4;
  auto w8 = Dyninst::aarch64::w8;
  auto w10 = Dyninst::aarch64::w10;
  auto w12 = Dyninst::aarch64::w12;
  auto w20 = Dyninst::aarch64::w20;
  auto w30 = Dyninst::aarch64::w30;

  auto x0 = Dyninst::aarch64::x0;
  auto x1 = Dyninst::aarch64::x1;
  auto x8 = Dyninst::aarch64::x8;
  auto x10 = Dyninst::aarch64::x10;
  auto x16 = Dyninst::aarch64::x16;
  auto x20 = Dyninst::aarch64::x20;

  using reg_set = Dyninst::register_set;

  return {
    { // extr w10, w20, w30, #5
      {0x8a, 0x16, 0x9e, 0x13},
      di::opcode_test(aarch64_op_extr, "extr w10, w20, w30, #5"),
      di::register_rw_test {
        reg_set{w30, w20},
        reg_set{w10},
      },
    },
    { // extr x0, x8, x16, #63
      {0x00, 0xfd, 0xd0, 0x93},
      di::opcode_test(aarch64_op_extr, "extr x0, x8, x16, #3"), // WRONG: should be 'extr x0, x8, x16, #63'
      di::register_rw_test {
        reg_set{x16, x8},
        reg_set{x0},
      },
    },
    { // movk x1, #256, lsl #48
      {0x01, 0x20, 0xe0, 0xf2},
      di::opcode_test(aarch64_op_movk, "movk x1, #100, lsl #30"), // WRONG: should be 'movk x1, #256, lsl #48'
      di::register_rw_test {
        reg_set{},
        reg_set{x1},
      },
    },
    { // movn w4, #23, lsl #16
      {0xe4, 0x02, 0xa0, 0x12},
      di::opcode_test(aarch64_op_mov_movn, "mov w4, #17, lsl #10"), // WRONG: should be 'movn w4, #23, lsl #16'
      di::register_rw_test {
        reg_set{},
        reg_set{w4},
      },
    },
    { // movn w8, #8, lsl #0
      {0x08, 0x01, 0x80, 0x12},
      di::opcode_test(aarch64_op_mov_movn, "mov w8, #8, lsl #0"), // WRONG: should be 'movn w8, #8, lsl #0'
      di::register_rw_test {
        reg_set{},
        reg_set{w8},
      },
    },
    { // movz x20, #0x12, lsl #32
      {0x54, 0x02, 0xc0, 0xd2},
      di::opcode_test(aarch64_op_mov_movz, "mov x20, #12, lsl #20"),  // WRONG: should be 'movz x20, #0x12, lsl #32'
      di::register_rw_test {
        reg_set{},
        reg_set{x20},
      },
    },

    /*** Reverse bytes **/

    { // rev x10, x20
      {0x8a, 0x0e, 0xc0, 0xda},
      di::opcode_test(aarch64_op_rev, "rev x10, x20"),
      di::register_rw_test {
        reg_set{x20},
        reg_set{x10},
      },
    },
    { // rev16 w0, w12
      {0x80, 0x05, 0xc0, 0x5a},
      di::opcode_test(aarch64_op_rev16_int, "rev16 w0, w12"),
      di::register_rw_test {
        reg_set{w12},
        reg_set{w0},
      },
    },
  };
}
