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
  auto pc = Dyninst::aarch64::pc;
  auto wzr = Dyninst::aarch64::wzr;

  auto w4 = Dyninst::aarch64::w4;
  auto w15 = Dyninst::aarch64::w15;

  auto x0 = Dyninst::aarch64::x0;
  auto x12 = Dyninst::aarch64::x12;
  auto x25 = Dyninst::aarch64::x25;
  auto x30 = Dyninst::aarch64::x30;

  using reg_set = Dyninst::register_set;

  return {
    { // b 0xfffffffffffffffc
      {0xff, 0xff, 0xff, 0x17},
      di::opcode_test(aarch64_op_b_uncond, "b pc + #0"),  // WRONG: should be 'b 0xfffffffffffffffc'
      di::register_rw_test {
        reg_set{pc},
        reg_set{pc},
      },
    },
    { // b.gt 0xfc
      {0xec, 0x07, 0x00, 0x54},
      di::opcode_test(aarch64_op_b_cond, "b"),  // WRONG: should be 'b.gt 0xfc'
      di::register_rw_test {
        reg_set{pc, nzcv},
        reg_set{pc},
      },
    },
    { // b.ne 0xfffffffffffffffc
      {0xe1, 0xff, 0xff, 0x54},
      di::opcode_test(aarch64_op_b_cond, "b"),  // WRONG: should be 'b.ne 0xfffffffffffffffc'
      di::register_rw_test {
        reg_set{pc, nzcv},
        reg_set{pc},
      },
    },
    { // bl 0x20
      {0x08, 0x00, 0x00, 0x94},
      di::opcode_test(aarch64_op_bl, "bl pc + #20"),  // WRONG: should be 'bl 0x20'
      di::register_rw_test {
        reg_set{pc},
        reg_set{pc},
      },
    },
    { // blr x30
      {0xc0, 0x03, 0x3f, 0xd6},
      di::opcode_test(aarch64_op_blr, "blr x30"),
      di::register_rw_test {
        reg_set{x30},
        reg_set{pc},
      },
    },
    { // br x12
      {0x80, 0x01, 0x1f, 0xd6},
      di::opcode_test(aarch64_op_br, "br x12"),
      di::register_rw_test {
        reg_set{x12},
        reg_set{pc},
      },
    },
    { // cbnz x30, 4
      {0x3e, 0x00, 0x00, 0xb5},
      di::opcode_test(aarch64_op_cbnz, "cbnz x30, pc + #4"),  // WRONG: should be 'cbnz x30, 4'
      di::register_rw_test {
        reg_set{x30, pc},
        reg_set{pc},
      },
    },
    { // cbz w15, 0xfffffffffffffffc
      {0xef, 0xff, 0xff, 0x34},
      di::opcode_test(aarch64_op_cbz, "cbz w15, pc + #0"),  // WRONG: should be 'cbz w15, 0xfffffffffffffffc'
      di::register_rw_test {
        reg_set{w15, pc},
        reg_set{pc},
      },
    },
    { // ret x0
      {0x00, 0x00, 0x5f, 0xd6},
      di::opcode_test(aarch64_op_ret, "ret x0"),
      di::register_rw_test {
        reg_set{x0},
        reg_set{pc},
      },
    },
    { // tbnz wzr, #0xc, 0x30
      {0x9f, 0x01, 0x60, 0x37},
      di::opcode_test(aarch64_op_tbnz, "tbnz wzr, #0, pc + #30"), // WRONG: should be 'tbnz wzr, #0xc, 0x30'
      di::register_rw_test {
        reg_set{wzr, pc},
        reg_set{pc},
      },
    },
    { // tbnz x25, #0x30, 0
      {0x19, 0x00, 0x80, 0xb7},
      di::opcode_test(aarch64_op_tbnz, "tbnz x25, #30, pc + #0"),  // WRONG: should be 'tbnz x25, #0x30, 0'
      di::register_rw_test {
        reg_set{x25, pc},
        reg_set{pc},
      },
    },
    { // tbz w4, #0x1e, 0
      {0xe4, 0xff, 0xf7, 0x36},
      di::opcode_test(aarch64_op_tbz, "tbz w4, #1, pc + #0"),  // WRONG: should be 'tbz w4, #0x1e, 0'
      di::register_rw_test {
        reg_set{w4, pc},
        reg_set{pc},
      },
    },

  };
}
